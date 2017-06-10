#include "selectors.h"
#include "logging.h"
#include "../misc/photon_extra.h"

#include "TString.h"
#include "TFile.h"
#include "TChain.h"
#include "TKey.h"
#include "TError.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TDirectory.h"
#include "TTreeFormula.h"

#include "GoodLumiFilter.h"

#include <vector>
#include <iostream>
#include <stdexcept>
#include <chrono>
typedef std::chrono::steady_clock SClock;

unsigned TIMEOUT(300);

class Skimmer {
public:
  Skimmer() {}
  ~Skimmer();

  void addPath(char const* _path) { paths_.emplace_back(_path); }
  void clearPaths() { paths_.clear(); }
  void addSelector(EventSelectorBase* _sel) { selectors_.push_back(_sel); }
  void setOwnSelectors(bool b) { ownSelectors_ = b; }
  void setCommonSelection(char const* _sel) { commonSelection_ = _sel; }
  void setGoodLumiFilter(GoodLumiFilter* _filt) { goodLumiFilter_ = _filt; }
  void setSkipMissingFiles(bool b) { skipMissingFiles_ = b; }
  void setPrintEvery(unsigned i) { printEvery_ = i; }
  void run(char const* outputDir, char const* sampleName, bool isData, long nEntries = -1);
  void prepareEvent(panda::Event const&, panda::GenParticleCollection const&, panda::EventMonophoton&);
  void setPrintLevel(unsigned l) { printLevel_ = l; }

private:
  std::vector<TString> paths_{};
  std::vector<EventSelectorBase*> selectors_{};
  bool ownSelectors_{true};
  TString commonSelection_{};
  GoodLumiFilter* goodLumiFilter_{};
  bool skipMissingFiles_{false};
  unsigned printEvery_{10000};
  unsigned printLevel_{0};
};

Skimmer::~Skimmer()
{
  if (ownSelectors_) {
    for (auto* sel : selectors_)
      delete sel;
  }
}

void
Skimmer::run(char const* _outputDir, char const* _sampleName, bool isData, long _nEntries/* = -1*/)
{
  // check all input exists
  for (auto&& pItr(paths_.begin()); pItr != paths_.end(); ++pItr) {
    TFile* source(0);

    auto originalErrorIgnoreLevel(gErrorIgnoreLevel);
    gErrorIgnoreLevel = kError + 1;

    unsigned const tryEvery(30);
    for (unsigned iAtt(0); iAtt <= TIMEOUT / tryEvery; ++iAtt) {
      source = TFile::Open(*pItr);
      if (source) {
        if (!source->IsZombie())
          break;
        delete source;
      }

      if (skipMissingFiles_)
        break;

      gSystem->Sleep(tryEvery * 1000.);
    }

    gErrorIgnoreLevel = originalErrorIgnoreLevel;

    if ((!source || source->IsZombie())) {
      if (skipMissingFiles_) {
        std::cerr << "Skipping missing file " << *pItr << std::endl;
        auto pos(pItr);
        --pItr;
        paths_.erase(pos);
      }
      else {
        std::cerr << "Cannot open file " << *pItr << std::endl;
        delete source;
        throw std::runtime_error("source");
      }
    }
    else {
      auto* inputKey(static_cast<TKey*>(source->GetListOfKeys()->FindObject("events")));
      if (!inputKey) {
        std::cerr << "Events tree missing from " << source->GetName() << std::endl;
        delete source;
        throw std::runtime_error("source");
      }
    }

    delete source;
  }

  TString outputDir(_outputDir);
  TString sampleName(_sampleName);

  std::ostream* stream(&std::cout);
  std::ofstream debugFile;
  if (printLevel_ > 0 && printLevel_ <= DEBUG) {
    TString debugPath("debug_" + sampleName + ".txt");
    debugFile.open(debugPath.Data());
    stream = &debugFile;
  }

  panda::Event event;
  panda::GenParticleCollection genParticles("genParticles");
  genParticles.data.parentContainer_ = &genParticles;
  panda::EventMonophoton skimmedEvent;

  // will get updated by individual operators
  panda::utils::BranchList branchList = {
    "!*",
    "runNumber",
    "lumiNumber",
    "eventNumber",
    "isData",
    "weight",
    "npv",
    "rho",
    "rhoCentralCalo",
    "triggers",
    "vertices",
    "superClusters",
    "electrons",
    "muons",
    "taus",
    "photons",
    "chsAK4Jets",
    "!chsAK4Jets.constituents_",
    "pfMet",
    "metFilters"
  };

  if (!isData)
    branchList += {"npvTrue", "genReweight", "genVertex", "partons"};

  if (printLevel_ > 0 && printLevel_ <= DEBUG)
    branchList.setVerbosity(1);

  bool doPreskim(commonSelection_.Length() != 0);

  for (auto* sel : selectors_) {
    sel->setPrintLevel(printLevel_, stream);

    TString outputPath(outputDir + "/" + sampleName + "_" + sel->name() + ".root");
    sel->initialize(outputPath, skimmedEvent, branchList, !isData);

    if (!sel->getCanPhotonSkim())
      doPreskim = false;
  }

  // if the selectors register triggers, make sure the information is passed to the actual input event
  event.run = skimmedEvent.run;

  if (goodLumiFilter_)
    *stream << "Applying good lumi filter." << std::endl;

  TChain preInput("events");
  TChain mainInput("events");
  TChain genInput("events");

  for (auto& path : paths_) {
    preInput.Add(path);
    mainInput.Add(path);
    genInput.Add(path);
  }

  TTreeFormula* preselection(0);
  int treeNumber(-1);
  if (doPreskim) {
    *stream << "Applying baseline selection \"" << commonSelection_ << "\"" << std::endl;

    preselection = new TTreeFormula("preselection", commonSelection_, &preInput);
  }

  event.setStatus(mainInput, branchList);
  event.setAddress(mainInput, {"*"}, false);

  genInput.SetBranchStatus("*", false);
  genParticles.setAddress(genInput);

  event.electrons.data.matchedGenContainer_ = &genParticles;
  event.muons.data.matchedGenContainer_ = &genParticles;
  event.taus.data.matchedGenContainer_ = &genParticles;
  event.photons.data.matchedGenContainer_ = &genParticles;

  auto now(SClock::now());
  auto start(now);

  long iEntry(0);
  while (iEntry++ != _nEntries) {
    if (iEntry % printEvery_ == 1 && printLevel_ > 0) {
      auto past = now;
      now = SClock::now();
      *stream << " " << iEntry << " (took " << std::chrono::duration_cast<std::chrono::milliseconds>(now - past).count() / 1000. << " s)" << std::endl;
    }

    if (preselection) {
      if (preInput.LoadTree(iEntry - 1) < 0)
        break;

      if (treeNumber != preInput.GetTreeNumber()) {
        treeNumber = preInput.GetTreeNumber();
        preselection->UpdateFormulaLeaves();
      }

      int nD(preselection->GetNdata());
      int iD(0);
      for (; iD != nD; ++iD) {
        if (preselection->EvalInstance(iD) != 0.)
          break;
      }
      if (iD == nD)
        continue;
    }

    if (event.getEntry(mainInput, iEntry - 1) <= 0)
      break;

    if (goodLumiFilter_ && !goodLumiFilter_->isGoodLumi(event.runNumber, event.lumiNumber))
      continue;

    if (!event.isData)
      genParticles.getEntry(genInput, iEntry - 1);

    prepareEvent(event, genParticles, skimmedEvent);

    if (printLevel_ > 0 && printLevel_ <= INFO) {
      debugFile << std::endl << ">>>>> Printing event " << iEntry <<" !!! <<<<<" << std::endl;
      debugFile << skimmedEvent.runNumber << ":" << skimmedEvent.lumiNumber << ":" << skimmedEvent.eventNumber << std::endl;
      skimmedEvent.print(debugFile, 2);
      debugFile << std::endl;
      skimmedEvent.photons.print(debugFile, 2);
      // debugFile << "photons.size() = " << skimmedEvent.photons.size() << std::endl;
      debugFile << std::endl;
      skimmedEvent.muons.print(debugFile, 2);
      // debugFile << "muons.size() = " << skimmedEvent.muons.size() << std::endl;
      debugFile << std::endl;
      skimmedEvent.electrons.print(debugFile, 2);
      // debugFile << "electrons.size() = " << skimmedEvent.electrons.size() << std::endl;
      debugFile << std::endl;
      skimmedEvent.jets.print(debugFile, 2);
      // debugFile << "jets.size() = " << skimmedEvent.jets.size() << std::endl;
      debugFile << std::endl;
      skimmedEvent.t1Met.print(debugFile, 2);
      // debugFile << std::endl;
      skimmedEvent.metMuOnlyFix.print(debugFile, 2);
      debugFile << std::endl;
      skimmedEvent.metNoFix.print(debugFile, 2);
      debugFile << std::endl;
      debugFile << ">>>>> Event " << iEntry << " done!!! <<<<<" << std::endl << std::endl;
    }

    for (auto* sel : selectors_)
      sel->selectEvent(skimmedEvent);
  }

  delete preselection;

  for (auto* sel : selectors_)
    sel->finalize();

  if (printLevel_ > 0 && printLevel_ <= INFO) {
    debugFile.close();
  }

  if (printLevel_ > 0) {
    now = SClock::now();
    *stream << "Finished. Took " << std::chrono::duration_cast<std::chrono::seconds>(now - start).count() / 60. << " minutes in total. " << std::endl;
  }
}

void
Skimmer::prepareEvent(panda::Event const& _event, panda::GenParticleCollection const& _genParticles, panda::EventMonophoton& _outEvent)
{
  // copy most of the event content (special operator= of EventMonophoton that takes Event as RHS)
  _outEvent.copy(_event);

  _outEvent.copyGenParticles(_genParticles);

  if (_outEvent.run.runNumber != _event.run.runNumber)
    _outEvent.run = _event.run;

  for (unsigned iPh(0); iPh != _event.photons.size(); ++iPh)
    panda::photon_extra(_outEvent.photons[iPh], _event.photons[iPh], _event.rho);
}
