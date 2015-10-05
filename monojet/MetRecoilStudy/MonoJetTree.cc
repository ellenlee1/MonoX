#include "MonoJetTree.h"

ClassImp(MonoJetTree)

//--------------------------------------------------------------------------------------------------
MonoJetTree::MonoJetTree(const char *name)
{ 
  t = new TTree(name,name);

  t->Branch("runNum",&runNum,"runNum/I");
  t->Branch("lumiNum",&lumiNum,"lumiNum/I");
  t->Branch("eventNum",&eventNum,"eventNum/I");
  t->Branch("jet1Pt",&jet1Pt,"jet1Pt/F");
  t->Branch("jet1Eta",&jet1Eta,"jet1Eta/F");
  t->Branch("jet1Phi",&jet1Phi,"jet1Phi/F");
  t->Branch("jet1M",&jet1M,"jet1M/F");
  t->Branch("jet1PuId",&jet1PuId,"jet1PuId/F");
  t->Branch("jet1isMonoJetId",&jet1isMonoJetId,"jet1isMonoJetId/I");
  t->Branch("jet1isLooseMonoJetId",&jet1isLooseMonoJetId,"jet1isLooseMonoJetId/I");
  t->Branch("jet1DPhiMet",&jet1DPhiMet,"jet1DPhiMet/F");
  t->Branch("jet1DPhiUZ",&jet1DPhiUZ,"jet1DPhiUZ/F");
  t->Branch("jet1DPhiUPho",&jet1DPhiUPho,"jet1DPhiUPho/F");
  t->Branch("jet2Pt",&jet2Pt,"jet2Pt/F");
  t->Branch("jet2Eta",&jet2Eta,"jet2Eta/F");
  t->Branch("jet2Phi",&jet2Phi,"jet2Phi/F");
  t->Branch("jet2M",&jet2M,"jet2M/F");
  t->Branch("jet2PuId",&jet2PuId,"jet2PuId/F");
  t->Branch("jet2isMonoJetId",&jet2isMonoJetId,"jet2isMonoJetId/I");
  t->Branch("jet2isLooseMonoJetId",&jet2isLooseMonoJetId,"jet2isLooseMonoJetId/I");
  t->Branch("jet2DPhiMet",&jet2DPhiMet,"jet2DPhiMet/F");
  t->Branch("jet2DPhiUZ",&jet2DPhiUZ,"jet2DPhiUZ/F");
  t->Branch("jet2DPhiUPho",&jet2DPhiUPho,"jet2DPhiUPho/F");
  t->Branch("n_jets",&n_jets,"n_jets/I");
  t->Branch("dPhi_j1j2",&dPhi_j1j2,"dPhi_j1j2/F");
  t->Branch("dR_j1j2",&dR_j1j2,"dR_j1j2/F");
  t->Branch("lep1Pt",&lep1Pt,"lep1Pt/F");
  t->Branch("lep1Eta",&lep1Eta,"lep1Eta/F");
  t->Branch("lep1Phi",&lep1Phi,"lep1Phi/F");
  t->Branch("lep1PdgId",&lep1PdgId,"lep1PdgId/I");
  t->Branch("lep1IsTight",&lep1IsTight,"lep1IsTight/I");
  t->Branch("lep1IsMedium",&lep1IsMedium,"lep1IsMedium/I");
  t->Branch("lep2Pt",&lep2Pt,"lep2Pt/F");
  t->Branch("lep2Eta",&lep2Eta,"lep2Eta/F");
  t->Branch("lep2Phi",&lep2Phi,"lep2Phi/F");
  t->Branch("lep2PdgId",&lep2PdgId,"lep2PdgId/I");
  t->Branch("lep2IsTight",&lep2IsTight,"lep2IsTight/I");
  t->Branch("lep2IsMedium",&lep2IsMedium,"lep2IsMedium/I");
  t->Branch("dilep_pt",&dilep_pt,"dilep_pt/F");
  t->Branch("dilep_eta",&dilep_eta,"dilep_eta/F");
  t->Branch("dilep_phi",&dilep_phi,"dilep_phi/F");
  t->Branch("dilep_m",&dilep_m,"dilep_m/F");
  t->Branch("mt",&mt,"mt/F");
  t->Branch("n_tightlep",&n_tightlep,"n_tightlep/I");
  t->Branch("n_mediumlep",&n_mediumlep,"n_mediumlep/I");
  t->Branch("n_looselep",&n_looselep,"n_looselep/I");
  t->Branch("photonPt",&photonPt,"photonPt/F");
  t->Branch("photonEta",&photonEta,"photonEta/F");
  t->Branch("photonPhi",&photonPhi,"photonPhi/F");
  t->Branch("photonIsTight",&photonIsTight,"photonIsTight/I");
  t->Branch("n_tightpho",&n_tightpho,"n_tightpho/I");
  t->Branch("n_loosepho",&n_loosepho,"n_loosepho/I");
  t->Branch("met",&met,"met/F");
  t->Branch("metPhi",&metPhi,"metPhi/F");
  t->Branch("u_perpZ",&u_perpZ,"u_perpZ/F");
  t->Branch("u_paraZ",&u_paraZ,"u_paraZ/F");
  t->Branch("u_magZ",&u_magZ,"u_magZ/F");
  t->Branch("u_phiZ",&u_phiZ,"u_phiZ/F");
  t->Branch("u_perpPho",&u_perpPho,"u_perpPho/F");
  t->Branch("u_paraPho",&u_paraPho,"u_paraPho/F");
  t->Branch("u_magPho",&u_magPho,"u_magPho/F");
  t->Branch("u_phiPho",&u_phiPho,"u_phiPho/F");
  t->Branch("mcWeight",&mcWeight,"mcWeight/F");
  t->Branch("triggerFired",&triggerFired);

  Reset();
}

//--------------------------------------------------------------------------------------------------
MonoJetTree::~MonoJetTree()
{
  delete t;
}

//--------------------------------------------------------------------------------------------------
void
MonoJetTree::Reset()
{
  runNum = 0;
  lumiNum = 0;
  eventNum = 0;
  jet1Pt = -5;
  jet1Eta = -5;
  jet1Phi = -5;
  jet1M = -5;
  jet1PuId = -2;
  jet1isMonoJetId = -1;
  jet1isLooseMonoJetId = -1;
  jet1DPhiMet = -1;
  jet1DPhiUZ = -1;
  jet1DPhiUPho = -1;
  jet2Pt = -5;
  jet2Eta = -5;
  jet2Phi = -5;
  jet2M = -5;
  jet2PuId = -1;
  jet2isMonoJetId = -1;
  jet2isLooseMonoJetId = -1;
  jet2DPhiMet = -1;
  jet2DPhiUZ = -1;
  jet2DPhiUPho = -1;
  n_jets = 0;
  dPhi_j1j2 = -1;
  dR_j1j2 = -1;
  lep1Pt = -5;
  lep1Eta = -5;
  lep1Phi = -5;
  lep1PdgId = 0;
  lep1IsTight = -1;
  lep1IsMedium = -1;
  lep2Pt = -5;
  lep2Eta = -5;
  lep2Phi = -5;
  lep2PdgId = 0;
  lep2IsTight = -1;
  lep2IsMedium = -1;
  dilep_pt = -5;
  dilep_eta = -5;
  dilep_phi = -5;
  dilep_m = -5;
  mt = -5;
  n_tightlep = 0;
  n_mediumlep = 0;
  n_looselep = 0;
  photonPt = -5;
  photonEta = -5;
  photonPhi = -5;
  photonIsTight = -1;
  n_tightpho = 0;
  n_loosepho = 0;
  met = -5;
  metPhi = -5;
  u_perpZ = -5;
  u_paraZ = -5;
  u_magZ = -5;
  u_phiZ = -5;
  u_perpPho = -5;
  u_paraPho = -5;
  u_magPho = -5;
  u_phiPho = -5;
  mcWeight = 0;
  triggerFired = 0;
}
