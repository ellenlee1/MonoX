#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TF2.h"

#include "fitFunctions.h"

void doFit() {

  TFile *file = new TFile("/Users/dabercro/GradSchool/Winter15/flatTrees/monojet_DYJetsToLL_M-50.root");
  TTree *tree = (TTree*) file->Get("events");

  TH2D *hist = new TH2D("test","test",200,15,1000,300,-150,150);

  tree->Draw("u_paraZ + dilep_pt:dilep_pt>>test","((lep1PdgId*lep2PdgId == -169) && abs(dilep_m - 91) < 15 && n_looselep == 2 && n_tightlep == 2 && n_loosepho == 0 && n_tau == 0 && lep2Pt > 30)&&jet1isMonoJetId == 1");

  // Double_t params[13] = {0,0.1,30,0.01,0,30,0.01,0,60,0.01,0,1000,0};
  Double_t params[5] = {0,0.1,30,20,50};
  TF2 *fitA = new TF2("fittums",fitFunc,-150,150,0,1000,5);

  TF2 *fit1 = new TF2("fit1",singleFunc,-150,150,0,1000,5);

  fit1->SetParameters(params);

  hist->Fit(fit1,"MLE");
  // fitA->SetParLimits(0,-50,50);

  // fitA->SetParLimits(3,0,2);
  // fitA->SetParLimits(6,0,2);
  // fitA->SetParLimits(9,0,2);
  // fitA->SetParLimits(4,-5,5);
  // fitA->SetParLimits(7,-5,5);
  // fitA->SetParLimits(10,-5,5);

  // TF2 *fitA = new TF2("fittums",fitFunc,-150,150,0,1000,11);

  // fitA->SetParameter(0,fit1->GetParameter(0));
  // fitA->SetParameter(1,fit1->GetParameter(1));
  // fitA->SetParameter(2,fit1->GetParameter(2));
  // fitA->SetParameter(3,fit1->GetParameter(3));
  // fitA->SetParameter(4,fit1->GetParameter(4));
  // fitA->SetParameter(5,fit1->GetParameter(2)-3);
  // fitA->SetParameter(6,fit1->GetParameter(3));
  // fitA->SetParameter(7,fit1->GetParameter(4));
  // fitA->SetParameter(8,fit1->GetParameter(2)+20);
  // fitA->SetParameter(9,fit1->GetParameter(3));
  // fitA->SetParameter(10,fit1->GetParameter(4));

  // hist->Fit(fitA,"MLE");

  TFile *results = new TFile("fitTest.root","RECREATE");

  TF1 *aFunc = new TF1("linfits",lin,15,1000,2);
  // TF1 *bFunc = new TF1("quadfits",quad,15,1000,3);

  aFunc->SetParameter(0,fitA->GetParameter(0));
  aFunc->SetParameter(1,fitA->GetParameter(1));
  results->WriteTObject(aFunc->Clone("mu"),"mu");

  // bFunc->SetParameter(0,fitA->GetParameter(2));
  // bFunc->SetParameter(1,fitA->GetParameter(3));
  // bFunc->SetParameter(2,fitA->GetParameter(4));
  // results->WriteTObject(bFunc->Clone("sigma3"),"sigma3");

  // bFunc->SetParameter(0,fitA->GetParameter(5));
  // bFunc->SetParameter(1,fitA->GetParameter(6));
  // bFunc->SetParameter(2,fitA->GetParameter(7));
  // results->WriteTObject(bFunc->Clone("sigma1"),"sigma1");

  // bFunc->SetParameter(0,fitA->GetParameter(8));
  // bFunc->SetParameter(1,fitA->GetParameter(9));
  // bFunc->SetParameter(2,fitA->GetParameter(10));
  // results->WriteTObject(bFunc->Clone("sigma2"),"sigma2");

  results->Close();

}
