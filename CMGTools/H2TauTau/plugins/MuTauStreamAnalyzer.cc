#include "CMGTools/H2TauTau/plugins/MuTauStreamAnalyzer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"

//COLIN: replacing by CMG::DiTau
// #include "TauAnalysis/CandidateTools/interface/CompositePtrCandidateT1T2MEtProducer.h"
// #include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEt.h"
// #include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEtFwd.h"
#include "AnalysisDataFormats/CMGTools/interface/CompoundTypes.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/MET.h"

#include <DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h>

#include "DataFormats/GeometryVector/interface/VectorUtil.h"

#include "DataFormats/RecoCandidate/interface/IsoDeposit.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositFwd.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositDirection.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositVetos.h"

#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include "DataFormats/PatCandidates/interface/TriggerEvent.h"
#include <DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h>

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

#include <vector>
#include <utility>
#include <map>

using namespace std;
using namespace reco;

typedef std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more>::iterator CImap;

MuTauStreamAnalyzer::MuTauStreamAnalyzer(const edm::ParameterSet & iConfig){

  diTauTag_          = iConfig.getParameter<edm::InputTag>("diTaus");
  jetsTag_           = iConfig.getParameter<edm::InputTag>("jets");
  newJetsTag_        = iConfig.getParameter<edm::InputTag>("newJets");
  metTag_            = iConfig.getParameter<edm::InputTag>("met");
  rawMetTag_         = iConfig.getParameter<edm::InputTag>("rawMet");
  muonsTag_          = iConfig.getParameter<edm::InputTag>("muons");
  muonsRelTag_       = iConfig.getParameter<edm::InputTag>("muonsRel");
  verticesTag_       = iConfig.getParameter<edm::InputTag>("vertices");
  triggerResultsTag_ = iConfig.getParameter<edm::InputTag>("triggerResults"); 
  isMC_              = iConfig.getParameter<bool>("isMC");
  genJetsTag_        = iConfig.getParameter<edm::InputTag>("genJets"); 
  deltaRLegJet_      = iConfig.getUntrackedParameter<double>("deltaRLegJet",0.3);
  minCorrPt_         = iConfig.getUntrackedParameter<double>("minCorrPt",10.);
  minJetID_          = iConfig.getUntrackedParameter<double>("minJetID",0.5);
  verbose_           = iConfig.getUntrackedParameter<bool>("verbose",false);
}

void MuTauStreamAnalyzer::beginJob(){

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("tree","qqH tree");

  tRandom_ = new TRandom3();
 
  //OK (for a good agreement on jet quantities, cut pt>30 && eta>-4.5)
  jetsBtagHE_  = new std::vector< double >();
  jetsBtagHP_  = new std::vector< double >();

  //DIFF: Lorenzo's charged hadron fraction distribution seems skewed
  // (less charged hadrons than we do - are they computed before JEC?)
  jetsChEfraction_  = new std::vector< float >();
  //BAD: PRIO3 can be dropped
  jetsChNfraction_  = new std::vector< float >();
  //BAD: PRIO2 we don't have that yet
  jetMoments_       = new std::vector< float >();

  //OK
  gammadR_       = new std::vector< float >();
  gammadEta_     = new std::vector< float >();
  gammadPhi_     = new std::vector< float >();
  gammaPt_       = new std::vector< float >();

  //BAD: PRIO1 : Jose needs to implement that
  tauXTriggers_= new std::vector< int >();
  triggerBits_ = new std::vector< int >();

  //DIFF: Lorenzo has a bit more uncorrected jets at low pT, and at pT>30. 
  // I think it's due to the lack of pile-up charged hadron subtraction.
  jetsP4_          = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //OK
  jetsIDP4_        = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //BAD: PRIO2 not yet filled
  jetsIDUpP4_      = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  jetsIDDownP4_    = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  jetsIDL1OffsetP4_= new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //BAD: PRIO3 -the JEC will be checked externally. the genjets are available though.
  genJetsIDP4_     = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();

  //OK
  diTauVisP4_   = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >(); 
  //BAD: PRIO3 we don't have collinear approx yet
  diTauCAP4_    = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  diTauICAP4_   = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //DIFF** 
  diTauSVfitP4_ = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >(); 

  //OK
  diTauLegsP4_    = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //? some variables show up, others don't.
  genDiTauLegsP4_ = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //OK 0: raw MET; 1: recoil corrected
  METP4_          = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //OK
  genMETP4_       = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();
  //OK
  genVP4_         = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();

  //OK (note: I have muons below 15 in my tree; for a good agreement, cut pt>15
  extraMuons_   = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();

  //BAD: PRIO3 - but we don't care. 
  pfMuons_      = new std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >();

  std::vector< float > Summer11Lumi ;
  Double_t Summer11Lumi_f[35] = {
    1.45346E-01,
    6.42802E-02,
    6.95255E-02,
    6.96747E-02,
    6.92955E-02,
    6.84997E-02,
    6.69528E-02,
    6.45515E-02,
    6.09865E-02,
    5.63323E-02,
    5.07322E-02,
    4.44681E-02,
    3.79205E-02,
    3.15131E-02,
    2.54220E-02,
    2.00184E-02,
    1.53776E-02,
    1.15387E-02,
    8.47608E-03,
    6.08715E-03,
    4.28255E-03,
    2.97185E-03,
    2.01918E-03,
    1.34490E-03,
    8.81587E-04,
    5.69954E-04,
    3.61493E-04,
    2.28692E-04,
    1.40791E-04,
    8.44606E-05,
    5.10204E-05,
    3.07802E-05,
    1.81401E-05,
    1.00201E-05,
    5.80004E-06
  };

  std::vector< float > Data2011Lumi;
  Double_t Data2011Lumi_f[35] = {
    0.00290212,
    0.0123985,
    0.0294783,
    0.0504491,
    0.0698525,
    0.0836611,
    0.0905799,
    0.0914388,
    0.0879379,
    0.0817086,
    0.073937,
    0.0653785,
    0.0565162,
    0.047707,
    0.0392591,
    0.0314457,
    0.0244864,
    0.018523,
    0.013608,
    0.00970977,
    0.00673162,
    0.00453714,
    0.00297524,
    0.00189981,
    0.00118234,
    0.000717854,
    0.00042561,
    0.000246653,
    0.000139853,
    7.76535E-05,
    4.22607E-05,
    2.25608E-05,
    1.18236E-05,
    6.0874E-06,
    6.04852E-06
  };

  std::vector< float > Data2011LumiExt;
  Double_t Data2011LumiExt_f[50] = {
    0.00290212,
    0.0123985,
    0.0294783,
    0.0504491,
    0.0698525,
    0.0836611,
    0.0905799,
    0.0914388,
    0.0879379,
    0.0817086,
    0.073937,
    0.0653785,
    0.0565162,
    0.047707,
    0.0392591,
    0.0314457,
    0.0244864,
    0.018523,
    0.013608,
    0.00970977,
    0.00673162,
    0.00453714,
    0.00297524,
    0.00189981,
    0.00118234,
    0.000717854,
    0.00042561,
    0.000246653,
    0.000139853,
    7.76535E-05,
    4.22607E-05,
    2.25608E-05,
    1.18236E-05,
    6.0874E-06,
    6.04852E-06,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };

  std::vector< float > Fall11Lumi ;
  Double_t Fall11Lumi_f[50] = {
    0.003388501,
    0.010357558,
    0.024724258,
    0.042348605,
    0.058279812,
    0.068851751,
    0.072914824,
    0.071579609,
    0.066811668,
    0.060672356,
    0.054528356,
    0.04919354,
    0.044886042,
    0.041341896,
    0.0384679,
    0.035871463,
    0.03341952,
    0.030915649,
    0.028395374,
    0.025798107,
    0.023237445,
    0.020602754,
    0.0180688,
    0.015559693,
    0.013211063,
    0.010964293,
    0.008920993,
    0.007080504,
    0.005499239,
    0.004187022,
    0.003096474,
    0.002237361,
    0.001566428,
    0.001074149,
    0.000721755,
    0.000470838,
    0.00030268,
    0.000184665,
    0.000112883,
    6.74043E-05,
    3.82178E-05,
    2.22847E-05,
    1.20933E-05,
    6.96173E-06,
    3.4689E-06,
    1.96172E-06,
    8.49283E-07,
    5.02393E-07,
    2.15311E-07,
    9.56938E-08
  };

  std::vector< float > Data2011TruthLumi;
  Double_t Data2011TruthLumi_f[50] = {
    0, 
    6.48087e-05, 
    0.00122942, 
    0.0107751, 
    0.0572544, 
    0.110336, 
    0.122622, 
    0.113354, 
    0.0991184, 
    0.0907195, 
    0.0813241, 
    0.0748131, 
    0.0703376, 
    0.0625598, 
    0.0487456, 
    0.030941, 
    0.0158153, 
    0.00657034, 
    0.00232069, 
    0.000782941, 
    0.000240306, 
    6.13863e-05, 
    1.36142e-05, 
    4.99913e-07, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };


  for( int i=0; i<35; i++) {
    Data2011Lumi.push_back(Data2011Lumi_f[i]);
    Summer11Lumi.push_back(Summer11Lumi_f[i]);
  }
  for( int i=0; i<50; i++) {
    Data2011TruthLumi.push_back(Data2011TruthLumi_f[i]);
    Data2011LumiExt.push_back(Data2011LumiExt_f[i]);
    Fall11Lumi.push_back(Fall11Lumi_f[i]);
  }
  cout << "MC = " << Fall11Lumi.size() << ", DATA = " << Data2011LumiExt.size() << endl;
  //LumiWeights_ = edm::LumiReWeighting(Summer11Lumi, Data2011Lumi);
  LumiWeights_ = edm::LumiReWeighting(Fall11Lumi, Data2011LumiExt);
  

  tree_->Branch("jetsP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&jetsP4_);
  tree_->Branch("jetsIDP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&jetsIDP4_);
  tree_->Branch("jetsIDUpP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&jetsIDUpP4_);
  tree_->Branch("jetsIDDownP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&jetsIDDownP4_);
  tree_->Branch("jetsIDL1OffsetP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&jetsIDL1OffsetP4_);
  tree_->Branch("genJetsIDP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&genJetsIDP4_);
  
  tree_->Branch("jetsBtagHE","std::vector<double>",&jetsBtagHE_);
  tree_->Branch("jetsBtagHP","std::vector<double>",&jetsBtagHP_);

  tree_->Branch("jetMoments","std::vector<float> ",&jetMoments_);

  tree_->Branch("gammadR","std::vector<float> ",&gammadR_);
  tree_->Branch("gammadEta","std::vector<float> ",&gammadEta_);
  tree_->Branch("gammadPhi","std::vector<float> ",&gammadPhi_);
  tree_->Branch("gammaPt","std::vector<float> ",&gammaPt_);

  tree_->Branch("jetsChEfraction","std::vector<float>",&jetsChEfraction_);
  tree_->Branch("jetsChNfraction","std::vector<float>",&jetsChNfraction_);

  tree_->Branch("tauXTriggers","std::vector<int>",&tauXTriggers_);
  tree_->Branch("triggerBits","std::vector<int>",&triggerBits_);

  tree_->Branch("diTauVisP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",  &diTauVisP4_);
  tree_->Branch("diTauCAP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",   &diTauCAP4_);
  tree_->Branch("diTauICAP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",   &diTauICAP4_);
  tree_->Branch("diTauSVfitP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&diTauSVfitP4_);

  tree_->Branch("diTauNSVfitMass",       &diTauNSVfitMass_,"diTauNSVfitMass/F");
  tree_->Branch("diTauNSVfitMassErrUp",  &diTauNSVfitMassErrUp_,"diTauNSVfitMassErrUp/F");
  tree_->Branch("diTauNSVfitMassErrDown",&diTauNSVfitMassErrDown_,"diTauNSVfitMassErrDown/F");

  tree_->Branch("diTauLegsP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&diTauLegsP4_);
  tree_->Branch("genDiTauLegsP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&genDiTauLegsP4_);

  tree_->Branch("METP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&METP4_);
  tree_->Branch("genMETP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&genMETP4_);
  tree_->Branch("genVP4","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&genVP4_);
  tree_->Branch("genDecay",&genDecay_,"genDecay/I");

  tree_->Branch("extraMuons","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&extraMuons_);
  tree_->Branch("pfMuons","std::vector< ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > >",&pfMuons_);

  //OK
  tree_->Branch("sumEt",&sumEt_,"sumEt/F");
  //N/A
  tree_->Branch("mTauTauMin",&mTauTauMin_,"mTauTauMin/F");
  //DIFF (small difference due to recoil correction, I think)
  tree_->Branch("MtLeg1",&MtLeg1_,"MtLeg1/F");
  //OK
  tree_->Branch("pZeta",&pZeta_,"pZeta/F");
  //OK
  tree_->Branch("pZetaVis",&pZetaVis_,"pZetaVis/F");
  //N/A we don't have that yet
  tree_->Branch("pZetaSig",&pZetaSig_,"pZetaSig/F");

  //COLIN we don't care about v1 (2010 settings)
  tree_->Branch("chIsoLeg1v1",&chIsoLeg1v1_,"chIsoLeg1v1/F");
  tree_->Branch("nhIsoLeg1v1",&nhIsoLeg1v1_,"nhIsoLeg1v1/F");
  tree_->Branch("phIsoLeg1v1",&phIsoLeg1v1_,"phIsoLeg1v1/F");
  tree_->Branch("elecIsoLeg1v1",&elecIsoLeg1v1_,"elecIsoLeg1v1/F");
  tree_->Branch("muIsoLeg1v1",&muIsoLeg1v1_,"muIsoLeg1v1/F");
  tree_->Branch("chIsoPULeg1v1",&chIsoPULeg1v1_,"chIsoPULeg1v1/F");
  tree_->Branch("nhIsoPULeg1v1",&nhIsoPULeg1v1_,"nhIsoPULeg1v1/F");
  tree_->Branch("phIsoPULeg1v1",&phIsoPULeg1v1_,"phIsoPULeg1v1/F");

  //OK
  tree_->Branch("chIsoLeg1v2",&chIsoLeg1v2_,"chIsoLeg1v2/F");
  //OK
  tree_->Branch("nhIsoLeg1v2",&nhIsoLeg1v2_,"nhIsoLeg1v2/F");
  //OK
  tree_->Branch("phIsoLeg1v2",&phIsoLeg1v2_,"phIsoLeg1v2/F");
  //BAD, PRIO2
  tree_->Branch("elecIsoLeg1v2",&elecIsoLeg1v2_,"elecIsoLeg1v2/F");
  //BAD, PRIO2
  tree_->Branch("muIsoLeg1v2",&muIsoLeg1v2_ ,"muIsoLeg1v2/F");
  //DIFF, but I don't know what this is for.
  tree_->Branch("chIsoPULeg1v2",&chIsoPULeg1v2_,"chIsoPULeg1v2/F");
  //OK
  tree_->Branch("nhIsoPULeg1v2",&nhIsoPULeg1v2_,"nhIsoPULeg1v2/F");
  //OK
  tree_->Branch("phIsoPULeg1v2",&phIsoPULeg1v2_,"phIsoPULeg1v2/F");

  //OK
  tree_->Branch("chIsoLeg2",&chIsoLeg2_,"chIsoLeg2/F");
  //OK
  tree_->Branch("nhIsoLeg2",&nhIsoLeg2_,"nhIsoLeg2/F");
  //OK
  tree_->Branch("phIsoLeg2",&phIsoLeg2_,"phIsoLeg2/F");
  //OK
  tree_->Branch("dxy1",&dxy1_,"dxy1/F");
  //DIFF, we have a more spiky plot than Lorenzo
  tree_->Branch("dxy2",&dxy2_,"dxy2/F");
  //OK
  tree_->Branch("dz1",&dz1_,"dz1/F");
  //OK
  tree_->Branch("dz2",&dz2_,"dz2/F");

  //OK
  tree_->Branch("run",&run_,"run/l");
  //OK
  tree_->Branch("event",&event_,"event/l");
  //OK
  tree_->Branch("lumi",&lumi_,"lumi/l");
  //DIFF??
  tree_->Branch("numPV",&numPV_,"numPV/F");
  //DIFF : I guess I have a looser selection upstream
  tree_->Branch("numOfDiTaus",&numOfDiTaus_,"numOfDiTaus/I");
  //OK (once isolation is applied)
  tree_->Branch("numOfLooseIsoDiTaus",&numOfLooseIsoDiTaus_,"numOfLooseIsoDiTaus/I");
  //OK
  tree_->Branch("decayMode",&decayMode_,"decayMode/I");
  //OK
  tree_->Branch("tightestHPSWP",&tightestHPSWP_,"tightestHPSWP/I");
  //OK
  tree_->Branch("tightestHPSDBWP",&tightestHPSDBWP_,"tightestHPSDBWP/I");
  //OK
  tree_->Branch("visibleTauMass",&visibleTauMass_,"visibleTauMass/F");

  //BAD: PRIO2 I don't have that 
  tree_->Branch("leadPFChargedHadrTrackPt",&leadPFChargedHadrTrackPt_,"leadPFChargedHadrTrackPt/F");
  //BAD: PRIO2 I don't have that
  tree_->Branch("leadPFChargedHadrTrackP", &leadPFChargedHadrTrackP_,"leadPFChargedHadrTrackP/F");
  //OK 
  tree_->Branch("leadPFChargedHadrPt",&leadPFChargedHadrPt_,"leadPFChargedHadrPt/F");
  //OK 
  tree_->Branch("leadPFChargedHadrP", &leadPFChargedHadrP_,"leadPFChargedHadrP/F");
  //OK 
  tree_->Branch("leadPFChargedHadrMva",&leadPFChargedHadrMva_,"leadPFChargedHadrMva/F");
  //OK 
  tree_->Branch("leadPFChargedHadrHcalEnergy",&leadPFChargedHadrHcalEnergy_,"leadPFChargedHadrHcalEnergy/F");
  //OK 
  tree_->Branch("leadPFChargedHadrEcalEnergy",&leadPFChargedHadrEcalEnergy_,"leadPFChargedHadrEcalEnergy/F");

  //BAD: PRIO2 
  tree_->Branch("leadPFCandMva",&leadPFCandMva_,"leadPFCandMva/F");
  //BAD: PRIO2 
  tree_->Branch("leadPFCandHcalEnergy",&leadPFCandHcalEnergy_,"leadPFCandHcalEnergy/F");
  //BAD: PRIO2 
  tree_->Branch("leadPFCandEcalEnergy",&leadPFCandEcalEnergy_,"leadPFCandEcalEnergy/F");
  //OK
  tree_->Branch("leadPFCandPt",&leadPFCandPt_,"leadPFCandPt/F");
  //OK
  tree_->Branch("leadPFCandP",&leadPFCandP_,"leadPFCandP/F");
  //BAD: PRIO2 
  tree_->Branch("emFraction",&emFraction_,"emFraction/F");
  //BAD: PRIO2 
  tree_->Branch("hasGsf",&hasGsf_,"hasGsf/F");
  //DIFF (small differences... Lorenzo has a charge selection maybe?)
  tree_->Branch("signalPFChargedHadrCands",&signalPFChargedHadrCands_,"signalPFChargedHadrCands/I");
  //OK
  tree_->Branch("signalPFGammaCands",&signalPFGammaCands_,"signalPFGammaCands/I");


  //OK
  tree_->Branch("isTauLegMatched",&isTauLegMatched_,"isTauLegMatched/I");
  //OK
  tree_->Branch("isMuLegMatched",&isMuLegMatched_,"isMuLegMatched/I");
  //OK
  tree_->Branch("muFlag",&muFlag_,"muFlag/I");
  //BAD: PRIO2 
  tree_->Branch("hasKft",&hasKft_,"hasKft/I");

  //OK
  tree_->Branch("diTauCharge",&diTauCharge_,"diTauCharge/F");
  //DIFF (I guess due to charged hadron pile-up subtraction)
  tree_->Branch("rhoFastJet",&rhoFastJet_,"rhoFastJet/F");
  //BAD we don't have that - PRIO2
  tree_->Branch("rhoNeutralFastJet",&rhoNeutralFastJet_,"rhoNeutralFastJet/F");
  //COLIN what is this? Forget about it
  tree_->Branch("mcPUweight",&mcPUweight_,"mcPUweight/F");
  //OK
  tree_->Branch("nPUVertices",&nPUVertices_,"nPUVertices/I");
  //Not in Lorenzo's tree
  tree_->Branch("nPUVerticesM1",&nPUVerticesM1_,"nPUVerticesM1/I");
  //Not in Lorenzo's tree
  tree_->Branch("nPUVerticesP1",&nPUVerticesP1_,"nPUVerticesP1/I");
  
  //OK
  tree_->Branch("embeddingWeight",&embeddingWeight_,"embeddingWeight/F");
  //does not work in mine, and not in Lorenzo's tree. 
  tree_->Branch("nPUtruth",  &nPUtruth_,  "nPUtruth/I");


}


MuTauStreamAnalyzer::~MuTauStreamAnalyzer(){
  delete jetsP4_; delete jetsIDP4_; delete jetsIDUpP4_; delete jetsIDDownP4_; 
  delete METP4_; delete diTauVisP4_; delete diTauCAP4_; delete diTauICAP4_; 
  delete diTauSVfitP4_; delete genVP4_;
  delete diTauLegsP4_; delete jetsBtagHE_; delete jetsBtagHP_; delete tauXTriggers_; delete triggerBits_;
  delete genJetsIDP4_; delete genDiTauLegsP4_; delete genMETP4_; delete extraMuons_; delete pfMuons_; delete jetsIDL1OffsetP4_;
  delete tRandom_ ; delete jetsChNfraction_; delete jetsChEfraction_;delete jetMoments_;
  delete gammadR_ ; delete gammadPhi_; delete gammadEta_; delete gammaPt_;
}

void MuTauStreamAnalyzer::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup){



  jetsP4_->clear();
  jetsIDP4_->clear();
  jetsIDUpP4_->clear();
  jetsIDDownP4_->clear();
  jetsIDL1OffsetP4_->clear();
  diTauVisP4_->clear();
  diTauCAP4_->clear();
  diTauICAP4_->clear();
  diTauSVfitP4_->clear();
  diTauLegsP4_->clear();
  METP4_->clear();
  genVP4_->clear();
  jetsChNfraction_->clear();
  jetsChEfraction_->clear();
  jetMoments_->clear();
  
  gammadR_->clear();
  gammadEta_->clear();
  gammadPhi_->clear();
  gammaPt_->clear();

  genJetsIDP4_->clear();
  genDiTauLegsP4_->clear();
  genMETP4_->clear();
  extraMuons_->clear();
  pfMuons_->clear();
  jetsBtagHE_->clear();
  jetsBtagHP_->clear();
  tauXTriggers_->clear();
  triggerBits_->clear();
  
  // COLIN1
  //if running on PAT objects:
  //   typedef pat::Tau               Tau;
  //   typedef PATMuTauPairCollection DiTauCollection;
  //   typedef PATMuTauPair           DiTau;
  //   typedef pat::Muon Muon; 
  //if running on CMG objects:
  typedef cmg::TauMu DiTau;
  typedef std::vector<cmg::TauMu> DiTauCollection;
  typedef cmg::Tau Tau;
  typedef cmg::Muon Muon;
  typedef std::vector<cmg::Muon> MuonCollection;  
  typedef cmg::BaseMET MET;
  typedef std::vector<cmg::BaseMET> METCollection;
  typedef cmg::GenJet GenJet;
  typedef std::vector< GenJet > GenJetCollection;

  edm::Handle<DiTauCollection> diTauHandle;
  iEvent.getByLabel(diTauTag_,diTauHandle);
  //   if( !diTauHandle.isValid() )  
  //     edm::LogError("DataNotAvailable")
  //       << "No diTau label available \n";
  const DiTauCollection* diTaus = diTauHandle.product();
  
  edm::Handle<JetCollection> jetsHandle;
  iEvent.getByLabel(jetsTag_,jetsHandle);
  //   if( !jetsHandle.isValid() )  
  //     edm::LogError("DataNotAvailable")
  //       << "No jets label available \n";
  const JetCollection* jets = jetsHandle.product();

  edm::Handle<JetCollection> newJetsHandle;
  const JetCollection* newJets = 0;
  if(newJetsTag_.label()!=""){
    iEvent.getByLabel(newJetsTag_,newJetsHandle);
    if( !newJetsHandle.isValid() )  
      edm::LogError("DataNotAvailable")
	<< "No newJets label available \n";
    newJets = newJetsHandle.product();
  }

  //COLIN: we don't keep the PFCandidate collection in the CMG-tuple
  //used only in the rejection of low mass drell-yan events.
  //I guess that's not necessary at this stage, but could easily be done 
  //by accessing cmg::DiElectrons and cmg::DiMuons.
  //note : there is a pt cut pt>5 in our PF2PAT sequence, but it could 
  //probably be relaxed.
//   edm::Handle<reco::PFCandidateCollection> pfHandle;
//   iEvent.getByLabel("particleFlow",pfHandle);
//   if( !pfHandle.isValid() )  
//     edm::LogError("DataNotAvailable")
//       << "No pf particles label available \n";
//   const reco::PFCandidateCollection* pfCandidates = pfHandle.product();

  edm::Handle<reco::VertexCollection> pvHandle;
  iEvent.getByLabel( verticesTag_ ,pvHandle);
  if( !pvHandle.isValid() )  
    edm::LogError("DataNotAvailable")
      << "No PV label available \n";
  const reco::VertexCollection* vertexes = pvHandle.product();

  std::vector<float> vtxZ;
  for(unsigned int k = 0; k<vertexes->size(); k++){
    vtxZ.push_back(((*vertexes)[k].position()).z());
  }


  if(verbose_){
    cout <<  "Run " << iEvent.run() << ", event " << (iEvent.eventAuxiliary()).event() 
	 << ", lumi " << iEvent.luminosityBlock() << endl;
    cout << "List of vertexes " << endl;
    for(unsigned int k = 0; k<vertexes->size(); k++){
      cout << "Vtx[" << k << "] (x,y,z) = (" << ((*vertexes)[k].position()).x()
	   << "," << ((*vertexes)[k].position()).y() << "," << ((*vertexes)[k].position()).z() << ")"
	   << endl;
    }
  }

  numPV_ = vertexes->size();

  edm::Handle<METCollection> metHandle;
  iEvent.getByLabel( metTag_, metHandle);
  if( !metHandle.isValid() )  
    edm::LogError("DataNotAvailable")
      << "No MET label available \n";
  const METCollection* met = metHandle.product();

  edm::Handle<METCollection> rawMetHandle;
  iEvent.getByLabel( rawMetTag_, rawMetHandle);
  if( !rawMetHandle.isValid() )  
    edm::LogError("DataNotAvailable")
      << "No raw MET label available \n";
  const METCollection* rawMet = rawMetHandle.product();

  //JOSETD : read our trigger branch
//   edm::Handle<pat::TriggerEvent> triggerHandle;
//   iEvent.getByLabel(triggerResultsTag_, triggerHandle);
//   if( !triggerHandle.isValid() )  
//     edm::LogError("DataNotAvailable")
//       << "No Trigger label available \n";
//   const pat::TriggerEvent* trigger = triggerHandle.product();

  //JOSETD : read our trigger object branch
//   edm::Handle<pat::TriggerObjectStandAloneCollection > triggerObjsHandle;
//   iEvent.getByLabel(edm::InputTag("patTrigger"),triggerObjsHandle);
//   if( !triggerObjsHandle.isValid() )  
//     edm::LogError("DataNotAvailable")
//       << "No Trigger Objects label available \n";
//   const pat::TriggerObjectStandAloneCollection* triggerObjs = triggerObjsHandle.product();


  genDecay_ = -99;
  edm::Handle<reco::GenParticleCollection> genHandle;
  const reco::GenParticleCollection* genParticles = 0;
  const reco::GenParticleCollection* genParticlesStatus3 = 0;
  const reco::GenParticleCollection* genLeptonsStatus1 = 0;
  const GenJetCollection* genJets = 0;

  if(isMC_){
    
    edm::Handle<GenJetCollection> genJetsHandle;
    iEvent.getByLabel( genJetsTag_, genJetsHandle);
    genJets = genJetsHandle.product();    

    //COLIN read genParticles of status 3 from CMG-tuple
    edm::Handle<reco::GenParticleCollection> genStatus3Handle;
    iEvent.getByLabel(edm::InputTag("genParticlesStatus3"), genStatus3Handle);
    genParticlesStatus3 = genStatus3Handle.product();
    
    //COLIN read genLeptons of status 1 from CMG-tuple
    edm::Handle<reco::GenParticleCollection> genLeptonsStatus1Handle;
    iEvent.getByLabel(edm::InputTag("genLeptonsStatus1"), genLeptonsStatus1Handle);
    genLeptonsStatus1 = genLeptonsStatus1Handle.product();
    
    // COLIN: the following can be done on status 3 particles, which are available.
//     iEvent.getByLabel(edm::InputTag("genParticles"),genHandle);
//     if( !genHandle.isValid() )  
//       edm::LogError("DataNotAvailable")
// 	<< "No gen particles label available \n";
//     genParticles = genHandle.product();
    genParticles = genParticlesStatus3;
    for(unsigned int k = 0; k < genParticles->size(); k ++){
      if( !( (*genParticles)[k].pdgId() == 23 || abs((*genParticles)[k].pdgId()) == 24 || (*genParticles)[k].pdgId() == 25) || (*genParticles)[k].status()!=3)
	continue;
      if(verbose_) cout << "Boson status, pt,phi " << (*genParticles)[k].status() << "," << (*genParticles)[k].pt() << "," << (*genParticles)[k].phi() << endl;

      genVP4_->push_back( (*genParticles)[k].p4() );
      genDecay_ = (*genParticles)[k].pdgId();

      int breakLoop = 0;
      for(unsigned j = 0; j< ((*genParticles)[k].daughterRefVector()).size() && breakLoop!=1; j++){
	if( abs(((*genParticles)[k].daughterRef(j))->pdgId()) == 11 ){
	  genDecay_ *= 11;
	  breakLoop = 1;
	}
	if( abs(((*genParticles)[k].daughterRef(j))->pdgId()) == 13 ){
	  genDecay_ *= 13;
	  breakLoop = 1;
	}
	if( abs(((*genParticles)[k].daughterRef(j))->pdgId()) == 15 ){
	  genDecay_ *= 15;
	  breakLoop = 1;
	}
      }
      if(verbose_) cout << "Decays to pdgId " << genDecay_/(*genParticles)[k].pdgId()  << endl;
      break;
    }
  }

  nPUVertices_       = -99;
  nPUVerticesP1_     = -99;
  nPUVerticesM1_     = -99;

  nPUtruth_          = -99;

//   const reco::GenJetCollection* tauGenJets = 0;
  if(isMC_){
    //COLIN: tauGenJets not necessary anymore.
    //can be added to CMG-tuple if needed. 
//     // tag gen jets
//     edm::Handle<reco::GenJetCollection> tauGenJetsHandle;
//     iEvent.getByLabel(edm::InputTag("tauGenJetsSelectorAllHadrons"),tauGenJetsHandle);
//     if( !tauGenJetsHandle.isValid() )  
//       edm::LogError("DataNotAvailable")
// 	<< "No gen jet label available \n";
//     tauGenJets = tauGenJetsHandle.product();
    
    // PU infos
    //COLIN: note: the CMG-tuple already contains the weights for all 
    //data-taking periods. would be better to transfer these weights to the 
    //output tree, to make synchronization easier in the future. 
    edm::Handle<std::vector<PileupSummaryInfo> > puInfoH;
    iEvent.getByType(puInfoH);
    if(puInfoH.isValid()){
      for(std::vector<PileupSummaryInfo>::const_iterator it = puInfoH->begin(); it != puInfoH->end(); it++){
	
	//cout << "Bunc crossing " << it->getBunchCrossing() << endl;
	if(it->getBunchCrossing() == 0 ) 
	  nPUVertices_  = it->getPU_NumInteractions();
	else if(it->getBunchCrossing() == -1)  
	  nPUVerticesM1_= it->getPU_NumInteractions();
	else if(it->getBunchCrossing() == +1)  
	  nPUVerticesP1_= it->getPU_NumInteractions();
	
	nPUtruth_ = it->getTrueNumInteractions();	
      }
    }
  }

  if(verbose_){
    cout << "Num of PU = "          << nPUVertices_ << endl;
    cout << "Num of OOT PU = "      << nPUVerticesM1_+nPUVerticesP1_<< endl;
    cout << "Num of true interactions = " << nPUtruth_ << endl;
  }

  mcPUweight_ = LumiWeights_.weight( nPUVertices_ );

  edm::Handle<double> rhoFastJetHandle;
  iEvent.getByLabel(edm::InputTag("kt6PFJetsAK5","rho", ""), rhoFastJetHandle);
  if( !rhoFastJetHandle.isValid() )  
    edm::LogError("DataNotAvailable")
      << "No rho label available \n";
  rhoFastJet_ = (*rhoFastJetHandle);

  edm::Handle<double> embeddingWeightHandle;
  iEvent.getByLabel(edm::InputTag("generator","weight",""), embeddingWeightHandle);
  embeddingWeight_ = embeddingWeightHandle.isValid() ? (*embeddingWeightHandle) : 1.0;

  //COLINTD: Lorenzo : necessary? - PRIO3 
//   edm::Handle<double> rhoNeutralFastJetHandle;
//   iEvent.getByLabel(edm::InputTag("kt6PFJetsNeutral","rho", ""), rhoNeutralFastJetHandle);
//   if( !rhoNeutralFastJetHandle.isValid() )  
//     edm::LogError("DataNotAvailable")
//       << "No rho neutral label available \n";
//   rhoNeutralFastJet_ = (*rhoNeutralFastJetHandle);
  
  edm::Handle<MuonCollection> muonsHandle;
  iEvent.getByLabel( muonsTag_ ,muonsHandle);
  if( !muonsHandle.isValid() )  
    edm::LogError("DataNotAvailable")
      << "No muons label available \n";
  const MuonCollection* muons = muonsHandle.product();
  if(muons->size()<1){
    cout << " No muons !!! " << endl;
    return;
  } else if(muons->size()>1 && verbose_){
    cout << "WARNING: "<< muons->size() << "  muons found in the event !!! We will select only one" << endl;
  }
  edm::Handle<MuonCollection> muonsRelHandle;
  iEvent.getByLabel(muonsRelTag_, muonsRelHandle);
  if( !muonsRelHandle.isValid() )  
    edm::LogError("DataNotAvailable")
      << "No muonsRel label available \n";
  const MuonCollection* muonsRel = muonsRelHandle.product();
  if(muonsRel->size()<1){
    cout << " No muonsRel !!! " << endl;
    return;
  } else if(muonsRel->size()>1 && verbose_){
    cout << "WARNING: "<< muonsRel->size() << "  muonsRel found in the event !!! We will select only one" << endl;
  }
  
  const DiTau *theDiTau = 0;
  if(diTaus->size()<1){
    cout << " No diTau !!! " << endl;
    return;
  } else if(diTaus->size()>1 && verbose_){
    cout << "WARNING: "<< diTaus->size() << "  diTaus found in the event !!! We will select only one" << endl;
  }
  numOfDiTaus_ = diTaus->size();

  unsigned int muIndex = 0;
  muFlag_ = 0;

  bool found = false;
  //COLIN muonsRel is relaxed for the lepton veto (already applied in preselection)
  // muons is the baseline tight muon selection.
  //Lorenzo: which ptCut? now using 15
  double ptCut = 15;
  for(unsigned int i=0; i<muonsRel->size(); i++){

    const Muon& muonRel = (*muonsRel)[i];
    if(!  ( muonRel.pt()>ptCut &&
	    abs( muonRel.eta() ) < 2.5 &&
	    muonRel.isGlobal() &&
	    abs(muonRel.dxy()) < 0.045 &&
	    abs(muonRel.dz()) < 0.2 && 
	    muonRel.relIso(0.5)<0.3 ) ) {
      //COLIN: not a loose muon
      continue;
    }

    double isorel = muonRel.relIso( 0.5 );

    for(unsigned int j=0; j<muons->size(); j++){
      if(found) continue;
      const Muon& muon = (*muons)[j];
      
      if(!  ( muon.pt()>ptCut &&
	      abs( muon.eta() ) < 2.1 &&
	      muon.isGlobal() &&
	      muon.isTracker() &&
	      muon.numberOfValidTrackerHits() > 10 &&
	      muon.numberOfValidPixelHits() > 0 &&
	      muon.numberOfValidMuonHits() > 0 &&
	      muon.numberOfMatches() > 1 &&
	      muon.normalizedChi2() < 10 &&
	      abs(muon.dxy()) < 0.045 &&
	      abs(muon.dz()) < 0.2 &&
	      muon.relIso(0.5)<0.1 ) ) {
	//COLIN: not a tight muon
	continue;
      }
      
      double iso = muon.relIso( 0.5 );
      if( Geom::deltaR( (*muonsRel)[i].p4(),(*muons)[j].p4())>0.3
	  && (*muonsRel)[i].charge()*(*muons)[j].charge()<0
	  && iso<0.3
	  && isorel<0.3 ) {
	muFlag_ = 1;
	if(verbose_) cout<< "Two muons failing diMu veto: flag= " << muFlag_ << endl;
	found=true;
      }
      else if( Geom::deltaR( (*muonsRel)[i].p4(),(*muons)[j].p4())>0.3
	       && (*muonsRel)[i].charge()*(*muons)[j].charge()>0
	       && iso<0.3 
	       && isorel<0.3 ){
	muFlag_ = 2;
	if(verbose_) cout<< "Two muons with SS: flag= " << muFlag_ << endl;
	found=true;
      }
    }
  }
  

  // select highest-pt muon that forms at least one di-tau
  // no ambiguity if "muons" are isolated ==> if extra
  // isolated muon exists the event is vetoed
  // COLIN: keeping track of the index of this muon in the collection. 
  // COLIN: assumes muons are sorted by pT  
  bool found2 = false;
  for(unsigned int j=0; j<muons->size(); j++){
    for(unsigned int i=0; i< diTaus->size(); i++){
      if(found2) continue;

      //COLIN1A: 2 modifications:
      //  TauMu objects have the muon on the second leg
      //  TauMu objects provide their legs as concrete objects, not as pointers
      //       if( Geom::deltaR(((*diTaus)[i].leg1())->p4(),((*muons)[j]).p4())<0.01 ){
      if( Geom::deltaR(((*diTaus)[i].leg2()).p4(),((*muons)[j]).p4())<0.01 ){
	muIndex=j;
	found2 = true;
	//COLIN: could break the loop here. 
      }
    }
  }
  if(verbose_) cout<< "Selected muon with index " << muIndex << endl;

  //COLIN keeping tracks of all other muons far away enough as extra muons. 
  //COLIN why doing a delta R matching? 
  for(unsigned int i=0; i<muonsRel->size(); i++){
    if( Geom::deltaR( (*muonsRel)[i].p4(),(*muons)[muIndex].p4())>0.3) 
      extraMuons_->push_back( (*muonsRel)[i].p4() );
  }

  // COLIN: one could avoid this second loop.
  std::vector<unsigned int> selectedDiTausFromMu;
  for(unsigned int i=0; i< diTaus->size(); i++){
    //     if( Geom::deltaR(((*diTaus)[i].leg1())->p4(),((*muons)[muIndex]).p4())<0.01 ) 
    //See COLIN1A
    if( Geom::deltaR(((*diTaus)[i].leg2()).p4(),((*muons)[muIndex]).p4())<0.01 ) 
      selectedDiTausFromMu.push_back(i);
  }
  if(verbose_) cout<< "Selection on Muons has selected " << selectedDiTausFromMu.size() << " diTaus...check tau leg now..." << endl;
  
  
  // choose the diTau with most isolated tau leg
  double sumIsoTau = 999.;
  unsigned int index = 0;

  std::vector<unsigned int> identifiedTaus;
  std::vector<unsigned int> looseIsoTaus;

  for(unsigned int i=0; i<selectedDiTausFromMu.size(); i++){
    //COLIN1B getting the tau from the di-tau
    // the Tau type changes, and one should remember that for the cmg::DiTau, the tau is 
    // on the first leg, see COLIN1A
    //     const Tau*  tau_i = dynamic_cast<const Tau*>(  ((*diTaus)[selectedDiTausFromMu[i]].leg2()).get() );
    const Tau* tau_i = &((*diTaus)[selectedDiTausFromMu[i]].leg1());
    if(tau_i->tauID("decayModeFinding")<0.5) continue;
    identifiedTaus.push_back(selectedDiTausFromMu[i]);
    bool isIsolatedTau = false;
    isIsolatedTau = tau_i->tauID("byLooseCombinedIsolationDeltaBetaCorr")>0.5;
    if(isIsolatedTau) looseIsoTaus.push_back(selectedDiTausFromMu[i]);
  }

  numOfLooseIsoDiTaus_ = looseIsoTaus.size();
  if(looseIsoTaus.size()>0 ){
    //COLINTD: couldn't test this code yet!!! 
    // need more events, or to access the pat::Tau somewhere else - PRIO1
    identifiedTaus.swap(looseIsoTaus);
    if(verbose_) cout << identifiedTaus.size() << "  isolated taus found..." << endl;
    for(unsigned int i=0; i<identifiedTaus.size(); i++){
      if(verbose_) cout << "Testing isolation of " << i << "th tau" << endl;
      //COLIN same as COLIN1B
      //       const Tau*  tau_i = dynamic_cast<const Tau*>(  ((*diTaus)[ identifiedTaus[i] ].leg2()).get() );
      const Tau* tau_i = &((*diTaus)[ identifiedTaus[i] ].leg1());
      double sumIsoTau_i = 0.;
     
      //COLIN: I don't know what to do with the isolation...
      // with this solution I keep the pat::Tau and 
      // access the pat::Tau through the cmg::Tau
      //       const pat::TauPtr* patTauPtr = tau_i->sourcePtr();
      //       const pat::Tau* tau = patTauPtr->get();
      //COLINTD talk to Jose - OK, all of that is in the cmg::Tau
      //       const pat::Tau* tau = tau_i->sourcePtr()->get();
      const Tau& tau = *tau_i;
      sumIsoTau_i += tau.isolationPFChargedHadrCandsPtSum();
      //sumIsoTau_i += tau.isolationPFGammaCandsEtSum();
      //COLINTD : is the code below really correct? Don't we want dbeta correction?
      sumIsoTau_i += std::max( (tau.isolationPFGammaCandsEtSum() -
				rhoFastJet_*TMath::Pi()*0.5*0.5), 0.0);
      //sumIsoTau_i += tau.isolationPFNeutrHadrCandsEtSum();
      if(sumIsoTau_i<sumIsoTau){
	index = identifiedTaus[i];
	sumIsoTau = sumIsoTau_i;
      } 
    }
  } 
  else if(identifiedTaus.size()>0 ) {
    index = identifiedTaus[tRandom_->Integer( identifiedTaus.size() )];
    if(verbose_) cout << "Random selection has chosen index " << index << endl;   
  }

   
  if(verbose_) cout << "Chosen index " << index << endl;
  identifiedTaus.clear(); looseIsoTaus.clear();

  theDiTau = &(*diTaus)[index];

  
  diTauCharge_ = theDiTau->charge();
  METP4_->push_back((*rawMet)[0].p4()); // raw met
  //COLINTD PRIO1 : read recoil corrected MET (from diTau?)
  METP4_->push_back((*met)[0].p4());    // possibly rescaled met
  if(isMC_) 
    genMETP4_->push_back( (*rawMet)[0].genMET() );
  sumEt_  = (*met)[0].sumEt();
  //   MtLeg1_ =  theDiTau->mt1MET();
  MtLeg1_ =  theDiTau->mTLeg2();
  //   pZeta_     =  theDiTau->pZeta();
  pZeta_     =  theDiTau->pZeta();
  //   pZetaVis_  =  theDiTau->pZetaVis();
  pZetaVis_  =  theDiTau->pZetaVis();
  //   pZetaSig_  =  theDiTau->pZetaSig();
  //COLINTD: what is pZetaSig? - to be implemented in di-object PRIO2
  pZetaSig_  =  0.;
  //   mTauTauMin_=  theDiTau->mTauTauMin(); - to be implemented in di-object
  //COLINTD: what is mTauTauMin? PRIO2
  mTauTauMin_=  0.;

  isMuLegMatched_  = 0;
  isTauLegMatched_ = 0;

  
  //COLIN accessing (and exchanging) the legs of the ditau
  //   const Muon* leg1 = dynamic_cast<const Muon*>( (theDiTau->leg2()).get() );
  const Muon* leg1 = &( theDiTau->leg2() );
  //   const Tau*  leg2 = dynamic_cast<const Tau*>(  (theDiTau->leg1()).get() );
  const Tau* leg2 = &( theDiTau->leg1() );

  vector<string> triggerPaths;
  vector<string> XtriggerPaths;
  vector<string> HLTfiltersMu;
  vector<string> HLTfiltersTau;

  //COLIN: looks like the vectors below could be filled once and for all,
  // and not at every event. 
  if(isMC_){

    // X-triggers 
    XtriggerPaths.push_back("HLT_Mu15_LooseIsoPFTau20_v*");
    XtriggerPaths.push_back("HLT_IsoMu12_LooseIsoPFTau10_v*");
    XtriggerPaths.push_back("HLT_Mu15_v*");
    XtriggerPaths.push_back("HLT_IsoMu12_v*");

    /*
    // for Summer11
    triggerPaths.push_back("HLT_Mu15_LooseIsoPFTau20_v2");
    triggerPaths.push_back("HLT_IsoMu12_LooseIsoPFTau10_v2");
    triggerPaths.push_back("HLT_Mu15_v2");
    triggerPaths.push_back("HLT_IsoMu12_v1");

    HLTfiltersMu.push_back("hltSingleMuIsoL3IsoFiltered12");
    HLTfiltersMu.push_back("hltSingleMuIsoL3IsoFiltered15");

    HLTfiltersTau.push_back("hltOverlapFilterIsoMu12IsoPFTau10");
    */


    // for Fall11
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v9");
    triggerPaths.push_back("HLT_IsoMu15_eta2p1_LooseIsoPFTau20_v1");
    triggerPaths.push_back("HLT_IsoMu15_v14");

    HLTfiltersMu.push_back("hltSingleMuIsoL3IsoFiltered15");
    HLTfiltersMu.push_back("hltSingleMuIsoL1s14L3IsoFiltered15eta2p1");
    HLTfiltersTau.push_back("hltOverlapFilterIsoMu15IsoPFTau15");
    HLTfiltersTau.push_back("hltOverlapFilterIsoMu15IsoPFTau20");
    HLTfiltersTau.push_back("hltPFTau15TrackLooseIso");
    HLTfiltersTau.push_back("hltPFTau20TrackLooseIso");

  }
  else{

    XtriggerPaths.push_back("HLT_IsoMu12_LooseIsoPFTau10_v*");
    XtriggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v*");
    XtriggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau20_v*");
    XtriggerPaths.push_back("HLT_IsoMu12_v*");


    triggerPaths.push_back("HLT_IsoMu12_LooseIsoPFTau10_v1");
    triggerPaths.push_back("HLT_IsoMu12_LooseIsoPFTau10_v2");
    triggerPaths.push_back("HLT_IsoMu12_LooseIsoPFTau10_v4");
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v2");
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v4");
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v5");
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v6");
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v8");
    triggerPaths.push_back("HLT_IsoMu15_LooseIsoPFTau15_v9");
    triggerPaths.push_back("HLT_IsoMu15_eta2p1_LooseIsoPFTau20_v1");
    triggerPaths.push_back("HLT_IsoMu15_eta2p1_LooseIsoPFTau20_v5");
    triggerPaths.push_back("HLT_IsoMu15_eta2p1_LooseIsoPFTau20_v6");

    triggerPaths.push_back("HLT_IsoMu12_v1");
    triggerPaths.push_back("HLT_IsoMu12_v2");

    HLTfiltersMu.push_back("hltSingleMuIsoL3IsoFiltered12");
    HLTfiltersMu.push_back("hltSingleMuIsoL3IsoFiltered15");
    HLTfiltersMu.push_back("hltSingleMuIsoL1s14L3IsoFiltered15eta2p1");

    HLTfiltersTau.push_back("hltOverlapFilterIsoMu12IsoPFTau10");
    HLTfiltersTau.push_back("hltOverlapFilterIsoMu15IsoPFTau15");
    HLTfiltersTau.push_back("hltOverlapFilterIsoMu15IsoPFTau20");
  }

  //COLINTD : need Jose's help here. - PRIO1
//   for(unsigned int i=0;i<triggerPaths.size();i++){
//     if(!trigger){
//       //COLIN this could be checked out of the for loop.
//       continue;
//       cout << "Invalid triggerEvent !" << endl;
//     }
//     const pat::TriggerPath *triggerPath =  trigger->path(triggerPaths[i]);

//     if(verbose_){
//       cout<<  "Testing " << triggerPaths[i] << endl;
//       if(triggerPath) cout << "Is there..." << endl;
//       if(triggerPath && triggerPath->wasRun()) cout << "Was run..." << endl;
//       if(triggerPath && triggerPath->wasRun() && triggerPath->wasAccept()) cout << "Was accepted..." << endl;
//     }

//     if(triggerPath && triggerPath->wasRun() && 
//        triggerPath->wasAccept() && 
//        triggerPath->prescale()==1) triggerBits_->push_back(1);
//     else if (triggerPath && triggerPath->wasRun() && 
// 	     triggerPath->wasAccept() && 
// 	     triggerPath->prescale()!=1) triggerBits_->push_back(2);
//     else triggerBits_->push_back(0);
//   }

 
//   for(unsigned int i=0 ; i< HLTfiltersMu.size() ; i++){
//     bool matched = false;
//     for(pat::TriggerObjectStandAloneCollection::const_iterator it = triggerObjs->begin() ; it !=triggerObjs->end() ; it++){
//       pat::TriggerObjectStandAlone *aObj = const_cast<pat::TriggerObjectStandAlone*>(&(*it));
//       if(verbose_) {
// 	if( Geom::deltaR( aObj->triggerObject().p4(), leg1->p4() )<0.3 ){
// 	  for(unsigned int k =0; k < (aObj->filterLabels()).size() ; k++){
// 	    cout << "Object passing " << (aObj->filterLabels())[k] << " within 0.3 of muon" << endl;
// 	  }
// 	}
//       }
//       if( Geom::deltaR( aObj->triggerObject().p4(), leg1->p4() )<0.3  && aObj->hasFilterLabel(HLTfiltersMu[i]) ){
// 	matched = true;
//       }
//     }
//     if(matched) 
//       tauXTriggers_->push_back(1);
//     else 
//       tauXTriggers_->push_back(0);
//     if(verbose_){
//       if(matched) cout << "Muon matched within dR=0.3 with trigger object passing filter " << HLTfiltersMu[i] << endl;
//       else cout << "!!! Muon is not trigger matched within dR=0.3 !!!" << endl;
//     }
//   }
//   for(unsigned int i=0 ; i< HLTfiltersTau.size() ; i++){
//     bool matched = false;
//     for(pat::TriggerObjectStandAloneCollection::const_iterator it = triggerObjs->begin() ; it !=triggerObjs->end() ; it++){
//       pat::TriggerObjectStandAlone *aObj = const_cast<pat::TriggerObjectStandAlone*>(&(*it));
//       if(verbose_) {
//         if( Geom::deltaR( aObj->triggerObject().p4(), leg2->p4() )<0.3 ){
//           for(unsigned int k =0; k < (aObj->filterLabels()).size() ; k++){
//             cout << "Object passing " << (aObj->filterLabels())[k] << " within 0.3 of tau" << endl;
//           }
//         }
//       }
//       if( Geom::deltaR( aObj->triggerObject().p4(), leg2->p4() )<0.3  && aObj->hasFilterLabel(HLTfiltersTau[i]) ){
// 	matched = true;
//       }
//     }
//     if(matched) 
//       tauXTriggers_->push_back(1);
//     else 
//       tauXTriggers_->push_back(0);
//     if(verbose_){
//       if(matched) cout << "Tau matched within dR=0.3 with trigger object passing filter " << HLTfiltersTau[i] << endl;
//       else cout << "!!! Tau is not trigger matched within dR=0.3 !!!" << endl;
//     }
//   }

//   // triggers Mu
//   if(verbose_){
//     const pat::TriggerObjectStandAloneCollection trColl = leg1->triggerObjectMatchesByType(trigger::TriggerMuon);
//     cout << "Mu triggers" << endl;
//     for(pat::TriggerObjectStandAloneCollection::const_iterator it = trColl.begin(); it != trColl.end(); it++){
//       for(unsigned int k = 0; k < (it->pathNames(false,false)).size(); k++){
// 	cout << (it->pathNames(false,false))[k] << " with filters: " << endl;
// 	for(unsigned int m = 0; m < (it->filterLabels()).size(); m++){
// 	  cout << "  - " << (it->filterLabels())[m] << endl;
// 	}
//       }
//     }
//   }
//   // triggers Tau
//   if(verbose_){
//     const pat::TriggerObjectStandAloneCollection trColl = leg2->triggerObjectMatchesByType(trigger::TriggerTau);
//     cout << "Tau triggers" << endl;
//     for(pat::TriggerObjectStandAloneCollection::const_iterator it = trColl.begin(); it != trColl.end(); it++){
//       for(unsigned int k = 0; k < (it->pathNames(false,false)).size(); k++){
// 	cout << (it->pathNames(false,false))[k]  << " with filters: " << endl;
// 	for(unsigned int m = 0; m < (it->filterLabels()).size(); m++){
// 	  cout << "  - " << (it->filterLabels())[m] << endl;
// 	}
//       }
//     }
//   }
  //!COLINTD

  diTauLegsP4_->push_back(leg1->p4());
  diTauLegsP4_->push_back(leg2->p4());
  
  //COLIN:
  // in the following, according to the PAT documentation, 
  // the muon is matched to a gen muon with the same charge, status is not checked.
  // will now check only status 1 gen leptons from the corresponding CMG-tuple collection 
  if(isMC_){
    const reco::GenParticleCollection& leptons = *genLeptonsStatus1;
    double dR2Min = 9999999;
    int index = -1;
    for(unsigned i=0; i<leptons.size(); ++i){
      if(abs(leptons[i].pdgId()) != 13) continue;
      if(leptons[i].charge() != leg1->charge()) continue;
      double dR2 = reco::deltaR2( *leg1, leptons[i] );
      if(dR2<dR2Min) {
	dR2Min = dR2; 
	index = static_cast<int>(i);
      }
    }
    if( dR2Min<0.01 ) {
      //    if( (leg1->genParticleById(13,0,true)).isNonnull() ){
      genDiTauLegsP4_->push_back( leptons[index].p4() );
      isMuLegMatched_ = 1;
    }
    else{
      // the muon is not matched to a gen muon, filling p4 = 0
      genDiTauLegsP4_->push_back( math::XYZTLorentzVectorD(0,0,0,0) );
      //COLIN should the verbose block below really be here? - PRIO2
//       if(verbose_){
// 	for(unsigned int l = 0; l < leg1->genParticlesSize() ; l++){
// 	  if((leg1->genParticleRefs())[l]->pt() < 0.5 ) continue;
// 	  cout << "Mu leg matchged to particle " << (leg1->genParticleRefs())[l]->pdgId() 
// 	       << " with pt " << (leg1->genParticleRefs())[l]->pt()
// 	       << endl;
// 	}
//   }
    }
    
    //COLIN: access gen leptons from the CMGTuple collection of leptons with status1.
    bool leg2IsFromMu = false;
    math::XYZTLorentzVectorD genMuP4(0,0,0,0);
    for(unsigned int k = 0; k < genLeptonsStatus1->size(); k ++){
      if( abs((*genLeptonsStatus1)[k].pdgId()) != 13  || (*genLeptonsStatus1)[k].status()!=3 )
	continue;
      if(Geom::deltaR( (*genLeptonsStatus1)[k].p4(),leg2->p4())<0.15){
	leg2IsFromMu = true;
	genMuP4 = (*genLeptonsStatus1)[k].p4();
      }
    }
    
    //COLIN : access the genjet information from the cmg::Tau
    std::string genDecMode = leg2->genJetDecayMode();
    if( ! genDecMode.empty() )
      genDiTauLegsP4_->push_back(leg2->genJetp4());
    else if(leg2IsFromMu)
      genDiTauLegsP4_->push_back( genMuP4 );
    else{
      genDiTauLegsP4_->push_back( math::XYZTLorentzVectorD(0,0,0,0) );
      if(verbose_) cout << "WARNING: no genJet matched to the leg2 with eta,phi " << leg2->eta() << ", " << leg2->phi() << endl;
    }

  // to make the difference between DY fake and DY. 
  //COLIN: done because tauGenJets are made only from hadronic tau decay products
  // in Lorenzo's configuration. 
  //COLIN: tauHadMatched can be obtained looking at the genJet in the cmg::Tau
  // -> tauGenJets not necessary anymore, at least for now.
//     bool tauHadMatched = false;
//     for(unsigned int k = 0; k < tauGenJets->size(); k++){
//       if( Geom::deltaR( (*tauGenJets)[k].p4(),leg2->p4() ) < 0.15 ) tauHadMatched = true;
//     }
  bool tauHadMatched = false;
  // cout<<"genDecMode = "<<genDecMode<<endl;
  if(! genDecMode.empty() ) {
    tauHadMatched = true;
    isTauLegMatched_ = 1;
  }
  //COLIN: I think there is no need to also try to match to the true tau, 
  //but it could easily be done, just use genParticlesStatus3 from CMG-tuple
  //Or maybe Lorenzo wants to match to the true tau and then matching to the tau gen jet is not 
  //needed? 
//     if( ( (leg2->genParticleById(15,0,true)).isNonnull() || (leg2->genParticleById(-15,0,true)).isNonnull() ) && tauHadMatched ) isTauLegMatched_ = 1;
//     else if(verbose_){
//       for(unsigned int l = 0; l < leg2->genParticlesSize() ; l++){
// 	if((leg2->genParticleRefs())[l]->pt() < 0.5 ) continue;
// 	cout << "Tau leg matchged to particle " << (leg2->genParticleRefs())[l]->pdgId() 
// 	     << " with pt " << (leg2->genParticleRefs())[l]->pt()
// 	     << endl;
//       }
//     }
  }

  //COLIN: here, only computing the decay mode, can access it directly.
//   if((leg2->signalPFChargedHadrCands()).size()==1 && (leg2->signalPFGammaCands()).size()==0) decayMode_ = 0; 
//   else if((leg2->signalPFChargedHadrCands()).size()==1 && (leg2->signalPFGammaCands()).size()>0)  decayMode_ = 1; 
//   else if((leg2->signalPFChargedHadrCands()).size()==3) decayMode_ = 2; 
//   else  decayMode_ = -99;
  // cout<<"decay mode"<<leg2->decayMode()<<endl;
  switch( leg2->decayMode() ) {
  case 0: 
    decayMode_ = 0;
    break;
  case 1: 
  case 2: 
  case 3: 
  case 4: 
    decayMode_ = 1;
    break;
  case 10: 
  case 11: 
  case 12: 
  case 13: 
  case 14:
    decayMode_ = 2;
    break;
  default:
    decayMode_ = -99;
  }

  //COLIN needed for control. ok added the signal cands to the cmg::Tau in a light format.
  // btw I don't like the if... is there any way to know with respect to what 
  // the deltas have been computed, from the output tree? 
//   for(unsigned int k = 0 ; k < (leg2->signalPFGammaCands()).size() ; k++){
//     reco::PFCandidateRef gamma = (leg2->signalPFGammaCands()).at(k);
//     if( (leg2->leadPFChargedHadrCand()).isNonnull() ){
//       gammadR_->push_back(   Geom::deltaR( gamma->p4(), leg2->leadPFChargedHadrCand()->p4() ) );
//       gammadPhi_->push_back( Geom::deltaPhi( gamma->p4(), leg2->leadPFChargedHadrCand()->p4() ) );
//       gammadEta_->push_back( gamma->eta() - leg2->leadPFChargedHadrCand()->eta() ) ;
//     }
//     else{
//       gammadR_->push_back(   Geom::deltaR( gamma->p4(), leg2->p4() ) );
//       gammadPhi_->push_back( Geom::deltaPhi( gamma->p4(), leg2->p4() ) );
//       gammadEta_->push_back(  gamma->eta() - leg2->eta() ) ;
//     }
//     gammaPt_->push_back(  gamma->pt() );
//   }
  const cmg::SimpleParticle& leadCharged = leg2->leadPFChargedHadrCand();
  for(unsigned int k = 0 ; k < leg2->signalPFCands().size() ; k++){
    const cmg::SimpleParticle& gamma = leg2->signalPFCands()[k];
    if( gamma.type() != 22) continue;    
    if( leadCharged.type() != 0 ){
      gammadR_->push_back(   Geom::deltaR( gamma, leadCharged ) );
      gammadPhi_->push_back( Geom::deltaPhi( gamma, leadCharged ) );
      gammadEta_->push_back( gamma.eta() - leadCharged.eta() ) ;
    }
    else{
      gammadR_->push_back(   Geom::deltaR( gamma, leg2->p4() ) );
      gammadPhi_->push_back( Geom::deltaPhi( gamma, leg2->p4() ) );
      gammadEta_->push_back(  gamma.eta() - leg2->eta() ) ;
    }
    gammaPt_->push_back(  gamma.pt() );
  }

  visibleTauMass_ = leg2->mass();

  //COLIN: we don't keep the tracks in the CMG tuple. 
  //Lorenzo: are these variables really necessary?
  //these are control variables, we could add them to our pat::Tau - PRIO2
//   if((leg2->leadPFChargedHadrCand()->trackRef()).isNonnull()){
//     leadPFChargedHadrTrackPt_ = leg2->leadPFChargedHadrCand()->trackRef()->pt();
//     leadPFChargedHadrTrackP_  = leg2->leadPFChargedHadrCand()->trackRef()->p();
//   } else if((leg2->leadPFChargedHadrCand()->gsfTrackRef()).isNonnull()){
//     leadPFChargedHadrTrackPt_ = leg2->leadPFChargedHadrCand()->gsfTrackRef()->pt();
//     leadPFChargedHadrTrackP_  = leg2->leadPFChargedHadrCand()->gsfTrackRef()->p();
//   } else{
//     leadPFChargedHadrTrackPt_ = -99;
//     leadPFChargedHadrTrackP_  = -99;
//   }
  leadPFChargedHadrTrackPt_ = -99;
  leadPFChargedHadrTrackP_  = -99;
  leadPFChargedHadrPt_  = leadCharged.pt();
  //   leadPFChargedHadrP_  = leg2->leadPFChargedHadrCand()->p();  
  leadPFChargedHadrP_ = leadCharged.energy();

  //COLIN: electronPreIDOutput is mva_e_pi. 
  //   leadPFChargedHadrMva_        =   leg2->electronPreIDOutput() ;	
  leadPFChargedHadrMva_        =   leg2->leadChargedHadrMvaEPi() ;	
  //   leadPFChargedHadrHcalEnergy_ =  (leg2->leadPFChargedHadrCand()).isNonnull() ? (leg2->leadPFChargedHadrCand())->hcalEnergy() : -99 ;
  leadPFChargedHadrHcalEnergy_ =  leg2->leadChargedHadrHcalEnergy();
  //   leadPFChargedHadrEcalEnergy_ =  (leg2->leadPFChargedHadrCand()).isNonnull() ? (leg2->leadPFChargedHadrCand())->ecalEnergy() : -99;
  leadPFChargedHadrEcalEnergy_ =  leg2->leadChargedHadrEcalEnergy();
 
  //COLIN this is the leading PFCandidate in the tau, of any type, e.g. e-, photon, ...
  //   if( (leg2->leadPFCand()).isNonnull() ){
  //
  //     leadPFCandMva_               =  (leg2->leadPFCand())->mva_e_pi() ;	
  //     leadPFCandHcalEnergy_        =  (leg2->leadPFCand())->hcalEnergy() ;
  //     leadPFCandEcalEnergy_        =  (leg2->leadPFCand())->ecalEnergy() ;
  //     leadPFCandPt_                =  (leg2->leadPFCand())->pt();
  //     leadPFCandP_                 =  (leg2->leadPFCand())->p();
  //   }
  // look for lead PF cand:
  
  const Tau::Constituents& signalCands = leg2->signalPFCands();
  unsigned leadIndex = 0;
  double ptMax = -1;
  unsigned nChargedHadr = 0;
  unsigned nGamma = 0;
  for(unsigned i=0; i<signalCands.size(); ++i) {
    const Tau::Constituent& cand = signalCands[i];
    if(cand.pt() > ptMax ) {
      ptMax = cand.pt();
      leadIndex = i; 
    }
    if(cand.type() == 22) nGamma++;
    if( abs(cand.type()) == 211 ) nChargedHadr++;    
  }

  //COLINTD: we don't have the following yet 
  leadPFCandMva_               =  0;
  leadPFCandHcalEnergy_        =  0;
  leadPFCandEcalEnergy_        =  0;

  leadPFCandPt_                =  signalCands[leadIndex].pt();
  leadPFCandP_                 =  signalCands[leadIndex].energy();
 
  //   signalPFChargedHadrCands_ = (leg2->signalPFChargedHadrCands()).size();
  //   signalPFGammaCands_       = (leg2->signalPFGammaCands()).size();
  signalPFChargedHadrCands_ = nChargedHadr; 
  signalPFGammaCands_ = nGamma;
  
  //COLINTD: we don't have the following numbers: 
  //   emFraction_ = leg2->emFraction();
  //   hasGsf_     = ((leg2->leadPFChargedHadrCand())->gsfTrackRef()).isNonnull() ? 1. : 0.;
  

  tightestHPSWP_ = -1;
  if(leg2->tauID("byVLooseIsolation")>0.5) tightestHPSWP_++;
  if(leg2->tauID("byLooseIsolation")>0.5)  tightestHPSWP_++;
  if(leg2->tauID("byMediumIsolation")>0.5) tightestHPSWP_++;
  if(leg2->tauID("byTightIsolation")>0.5)  tightestHPSWP_++;
  tightestHPSDBWP_ = -1;
  if(leg2->tauID("byVLooseCombinedIsolationDeltaBetaCorr")>0.5) tightestHPSDBWP_++;
  if(leg2->tauID("byLooseCombinedIsolationDeltaBetaCorr")>0.5)  tightestHPSDBWP_++;
  if(leg2->tauID("byMediumCombinedIsolationDeltaBetaCorr")>0.5) tightestHPSDBWP_++;
  if(leg2->tauID("byTightCombinedIsolationDeltaBetaCorr")>0.5)  tightestHPSDBWP_++;


  //COLIN : reading dxy and dz from CMG leptons
//   dxy1_ = vertexes->size()!=0 ? leg1->globalTrack()->dxy( (*vertexes)[0].position() ) : -99;
//   dz1_ = vertexes->size()!=0 ? leg1->globalTrack()->dz( (*vertexes)[0].position() ) : -99;
//   dxy2_ = -99;
//   dz2_ = -99;
//   hasKft_ = 0;
//   if( vertexes->size()!=0 && (leg2->leadPFChargedHadrCand()).isNonnull() ){
//     if( (leg2->leadPFChargedHadrCand()->trackRef()).isNonnull() ){
//       dxy2_ = leg2->leadPFChargedHadrCand()->trackRef()->dxy( (*vertexes)[0].position() );
//       dz2_ = leg2->leadPFChargedHadrCand()->trackRef()->dz( (*vertexes)[0].position() );
//       hasKft_ = 1;
//     }
//     if( (leg2->leadPFChargedHadrCand()->gsfTrackRef()).isNonnull() ){
//       dxy2_ = leg2->leadPFChargedHadrCand()->gsfTrackRef()->dxy( (*vertexes)[0].position() );
//       dz2_ = leg2->leadPFChargedHadrCand()->gsfTrackRef()->dz( (*vertexes)[0].position() );
//     }
//   }
  dxy1_ = leg1->dxy();
  dz1_ = leg1->dz();
  dxy2_ = leg2->dxy();
  dz2_ = leg2->dz();
  

  //COLIN isolation is now precomputed
//   // isoDeposit definition: 2010
//   isodeposit::AbsVetos vetos2010ChargedLeg1; 
//   isodeposit::AbsVetos vetos2010NeutralLeg1; 
//   isodeposit::AbsVetos vetos2010PhotonLeg1;
//   // isoDeposit definition: 2011
//   isodeposit::AbsVetos vetos2011ChargedLeg1; 
//   isodeposit::AbsVetos vetos2011NeutralLeg1; 
//   isodeposit::AbsVetos vetos2011PhotonLeg1;
 
//   vetos2010ChargedLeg1.push_back(new isodeposit::ConeVeto(reco::isodeposit::Direction(leg1->eta(),leg1->phi()),0.01));
//   vetos2010ChargedLeg1.push_back(new isodeposit::ThresholdVeto(0.5));
//   vetos2010NeutralLeg1.push_back(new isodeposit::ConeVeto(isodeposit::Direction(leg1->eta(),leg1->phi()),0.08));
//   vetos2010NeutralLeg1.push_back(new isodeposit::ThresholdVeto(1.0));
//   vetos2010PhotonLeg1.push_back( new isodeposit::ConeVeto(isodeposit::Direction(leg1->eta(),leg1->phi()),0.05));
//   vetos2010PhotonLeg1.push_back( new isodeposit::ThresholdVeto(1.0));

//   vetos2011ChargedLeg1.push_back(new isodeposit::ConeVeto(reco::isodeposit::Direction(leg1->eta(),leg1->phi()),0.0001));
//   vetos2011ChargedLeg1.push_back(new isodeposit::ThresholdVeto(0.0));
//   vetos2011NeutralLeg1.push_back(new isodeposit::ConeVeto(isodeposit::Direction(leg1->eta(),leg1->phi()),0.01));
//   vetos2011NeutralLeg1.push_back(new isodeposit::ThresholdVeto(0.5));
//   vetos2011PhotonLeg1.push_back( new isodeposit::ConeVeto(isodeposit::Direction(leg1->eta(),leg1->phi()),0.01));
//   vetos2011PhotonLeg1.push_back( new isodeposit::ThresholdVeto(0.5));
  
  //COLIN replace by our isolation
  //from which particles is the charged hadron iso done? 
  // should use chargedParticleIso (save it also in cmg::Lepton)... 
  // ... but that's not what Lorenzo is using ???
//   chIsoLeg1v1_   = 
//     leg1->isoDeposit(pat::PfChargedHadronIso)->depositAndCountWithin(0.4,vetos2010ChargedLeg1).first;
  chIsoLeg1v2_   = leg1->chargedAllIso();
//   nhIsoLeg1v1_ = 
//     leg1->isoDeposit(pat::PfNeutralHadronIso)->depositAndCountWithin(0.4,vetos2010NeutralLeg1).first;
  nhIsoLeg1v2_   = leg1->neutralHadronIso();
//   phIsoLeg1v1_ = 
//     leg1->isoDeposit(pat::PfGammaIso)->depositAndCountWithin(0.4,vetos2010PhotonLeg1).first;
  phIsoLeg1v2_   = leg1->photonIso();
//   elecIsoLeg1v1_ = 
//     leg1->isoDeposit(pat::User3Iso)->depositAndCountWithin(0.4,vetos2010ChargedLeg1).first;
  elecIsoLeg1v2_ = 0; //COLINTD - PRIO2
//   muIsoLeg1v1_   = 
//     leg1->isoDeposit(pat::User2Iso)->depositAndCountWithin(0.4,vetos2010ChargedLeg1).first;
  muIsoLeg1v2_ = 0; //COLINTD - PRIO2
//   chIsoPULeg1v1_ = 
//     leg1->isoDeposit(pat::PfAllParticleIso)->depositAndCountWithin(0.4,vetos2010ChargedLeg1).first;
  chIsoPULeg1v2_ = leg1->puChargedHadronIso(); //COLIN: not used for dbeta corrections
//   nhIsoPULeg1v1_ = 
//     leg1->isoDeposit(pat::PfAllParticleIso)->depositAndCountWithin(0.4,vetos2010NeutralLeg1).first;
  nhIsoPULeg1v2_ = chIsoPULeg1v2_; //COLINTD - beware vetoes!!! PRIO1
//   phIsoPULeg1v1_ = 
//     leg1->isoDeposit(pat::PfAllParticleIso)->depositAndCountWithin(0.4,vetos2010PhotonLeg1).first;
  phIsoPULeg1v2_ = chIsoPULeg1v2_; //COLINTD - beware vetoes!!! PRIO1
  
  //COLINTD: check isolation definition of muons
  //COLINTD: implement Daniele's settings in PF2PAT
  //COLINTD: after momentum rescaling of physics objects, need to recompute the MET (provide module)

  //COLIN: dumping V2.
//   chIsoLeg1v2_   = 
//     leg1->isoDeposit(pat::PfChargedHadronIso)->depositAndCountWithin(0.4,vetos2011ChargedLeg1).first;
//   nhIsoLeg1v2_ = 
//     leg1->isoDeposit(pat::PfNeutralHadronIso)->depositAndCountWithin(0.4,vetos2011NeutralLeg1).first;
//   phIsoLeg1v2_ = 
//     leg1->isoDeposit(pat::PfGammaIso)->depositAndCountWithin(0.4,vetos2011PhotonLeg1).first;
//   elecIsoLeg1v2_ = 
//    leg1->isoDeposit(pat::User3Iso)->depositAndCountWithin(0.4,vetos2011ChargedLeg1).first;
//   muIsoLeg1v2_   = 
//     leg1->isoDeposit(pat::User2Iso)->depositAndCountWithin(0.4,vetos2011ChargedLeg1).first;
//   chIsoPULeg1v2_ = 
//     leg1->isoDeposit(pat::PfAllParticleIso)->depositAndCountWithin(0.4,vetos2011ChargedLeg1).first;
//   nhIsoPULeg1v2_ = 
//     leg1->isoDeposit(pat::PfAllParticleIso)->depositAndCountWithin(0.4,vetos2011NeutralLeg1).first;
//   phIsoPULeg1v2_ = 
//     leg1->isoDeposit(pat::PfAllParticleIso)->depositAndCountWithin(0.4,vetos2011PhotonLeg1).first;

  //COLIN8: oops, cannot do that... but I think it is not very important. 
  //One could think of keeping all muons and all electrons as cmg::Leptons. Is that what we do
  //already? Lorenzo 
  // loop over pfMuon to make sure we don't have low-mass DY events
//   for(unsigned int i = 0 ; i < pfCandidates->size() ; i++){
//     const reco::PFCandidate cand = (*pfCandidates)[i];
//     if(! (cand.particleId()== PFCandidate::mu && cand.pt()>0.5) ) continue;
//     float dz = (cand.trackRef()).isNonnull() ? (cand.trackRef())->dz( leg1->vertex() ) : 999.;
//     if(dz>0.2) continue;
//     pfMuons_->push_back(cand.p4());
//   }

  //Check the link between iso in cmg::Lepton and iso in cmg::Tau 
  //assume pat::Tau is kept. 
  chIsoLeg2_ = leg2->isolationPFChargedHadrCandsPtSum();
  //   //
  //   if(verbose_){
  //     cout << "Tau z position " << (leg2->vertex()).z() << endl;
  //     for(unsigned int i = 0; i < (leg2->isolationPFChargedHadrCands()).size(); i++){
  //       if( (((leg2->isolationPFChargedHadrCands()).at(i))->trackRef()).isNonnull() )
  // 	cout << "Ch # " << i << " has z position " << (((leg2->isolationPFChargedHadrCands()).at(i))->trackRef()->referencePoint()).z() << " and pt " << ((leg2->isolationPFChargedHadrCands()).at(i))->pt() << endl;
  //     }
  //   }
  //   //
  nhIsoLeg2_ = 0.;
  // COLIN: recomputed in the TauFactory. Why don't we put that in the reco::Tau or pat::Tau 
  // it it's needed? 
  //   for(unsigned int i = 0; i < (leg2->isolationPFNeutrHadrCands()).size(); i++){
  //     nhIsoLeg2_ += (leg2->isolationPFNeutrHadrCands()).at(i)->pt();
  //   }
  nhIsoLeg2_ = leg2->isolationPFNeutralHadrCandsPtSum();
  //   phIsoLeg2_ = leg2->isolationPFGammaCandsEtSum();
  phIsoLeg2_ = leg2->isolationPFGammaCandsEtSum();

  // cleaning
//   for(unsigned int i = 0; i <vetos2010ChargedLeg1.size(); i++){
//     delete vetos2010ChargedLeg1[i];
//   }
//   for(unsigned int i = 0; i <vetos2010NeutralLeg1.size(); i++){
//     delete vetos2010NeutralLeg1[i];
//     delete vetos2010PhotonLeg1[i];
//   }
//   for(unsigned int i = 0; i <vetos2011ChargedLeg1.size(); i++){
//     delete vetos2011ChargedLeg1[i];
//   }
//   for(unsigned int i = 0; i <vetos2011NeutralLeg1.size(); i++){
//     delete vetos2011NeutralLeg1[i];
//     delete vetos2011PhotonLeg1[i];
//   }

  //   diTauVisP4_->push_back( theDiTau->p4Vis() );
  diTauVisP4_->push_back( theDiTau->p4() );

  //COLIN: we don't have collinear approximation. - PRIO2
//   diTauCAP4_->push_back(  theDiTau->p4CollinearApprox() );
//   diTauICAP4_->push_back( theDiTau->p4ImprovedCollinearApprox() );
//   if(verbose_){
//     if(theDiTau->collinearApproxIsValid() && theDiTau->ImprovedCollinearApproxIsValid())
//       cout << "Found CA and ICA valid solutions" << endl;
//     else if(!theDiTau->collinearApproxIsValid() && theDiTau->ImprovedCollinearApproxIsValid())
//       cout << "ICA rescued the event!!!" << endl;
//     else if(theDiTau->collinearApproxIsValid() && !theDiTau->ImprovedCollinearApproxIsValid())
//       cout << "Strange!!! x1=" << theDiTau->x1CollinearApprox() << ", x2=" << theDiTau->x2CollinearApprox() << endl;
//     if((theDiTau->p4CollinearApprox()).M()>0 && (theDiTau->p4ImprovedCollinearApprox()).M()<=0)
//       cout << "Watch out! ICA gave mass=0 and CA mass !=0 ..." << endl;
//   }
  
  //COLINTD why the if? I think we always have a solution. Lorenzo?
  //Lorenzo why don't we store the mass as a double? the other components seem useless!
//   math::XYZTLorentzVectorD nSVfitFitP4(0,0,0,(theDiTau->p4Vis()).M() );
//   if( theDiTau->hasNSVFitSolutions() && theDiTau->nSVfitSolution("psKine_MEt_logM_fit",0)!=0 /*&& theDiTau->nSVfitSolution("psKine_MEt_logM_fit",0)->isValidSolution()*/ ){
//     if(verbose_) cout << "Visible mass ==> " << nSVfitFitP4.E() << endl;
//     nSVfitFitP4.SetPxPyPzE( 0,0,0, theDiTau->nSVfitSolution("psKine_MEt_logM_fit",0)->mass() ) ;
//     if(verbose_) cout << "SVFit fit solution ==> " << nSVfitFitP4.E() << endl;
//   }
//   diTauSVfitP4_->push_back( nSVfitFitP4  );

  math::XYZTLorentzVectorD nSVfitFitP4(0,0,0,theDiTau->mass() );
  //   if( theDiTau->hasNSVFitSolutions() && theDiTau->nSVfitSolution("psKine_MEt_logM_fit",0)!=0 /*&& theDiTau->nSVfitSolution("psKine_MEt_logM_fit",0)->isValidSolution()*/ ){
  if(verbose_) cout << "Visible mass ==> " << nSVfitFitP4.mass() << endl;
  nSVfitFitP4.SetPxPyPzE( 0,0,0, theDiTau->massSVFit() ) ;
  if(verbose_) cout << "SVFit fit solution ==> " << nSVfitFitP4.mass() << endl;
  // }
  diTauSVfitP4_->push_back( nSVfitFitP4  );

  //COLIN: the following is not necessary 
//   int errFlag = 0;
//   diTauNSVfitMass_        = (theDiTau->hasNSVFitSolutions() && theDiTau->nSVfitSolution("psKine_MEt_logM_int",&errFlag)!=0 && theDiTau->nSVfitSolution("psKine_MEt_logM_int",0)->isValidSolution() ) ? theDiTau->nSVfitSolution("psKine_MEt_logM_int",0)->mass()        : -99; 
//   diTauNSVfitMassErrUp_   = (theDiTau->hasNSVFitSolutions() && theDiTau->nSVfitSolution("psKine_MEt_logM_int",&errFlag)!=0 && theDiTau->nSVfitSolution("psKine_MEt_logM_int",0)->isValidSolution() ) ? theDiTau->nSVfitSolution("psKine_MEt_logM_int",0)->massErrUp()   : -99; 
//   diTauNSVfitMassErrDown_ = (theDiTau->hasNSVFitSolutions() && theDiTau->nSVfitSolution("psKine_MEt_logM_int",&errFlag)!=0 && theDiTau->nSVfitSolution("psKine_MEt_logM_int",0)->isValidSolution() ) ? theDiTau->nSVfitSolution("psKine_MEt_logM_int",0)->massErrDown() : -99; 

  run_   = iEvent.run();
  event_ = (iEvent.eventAuxiliary()).event();
  lumi_  = iEvent.luminosityBlock();

  std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more> sortedJets;
  std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more> sortedJetsIDL1Offset;
  std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more> sortedJetsID;
  std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more> sortedJetsIDUp;
  std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more> sortedJetsIDDown;
  std::map<double, math::XYZTLorentzVectorD ,MuTauStreamAnalyzer::more> sortedGenJetsID;
  std::map<double, std::pair<float,float> ,  MuTauStreamAnalyzer::more> bTaggers;
  std::map<double, std::pair<float,float> ,  MuTauStreamAnalyzer::more> sortedJetsIDFracs;
  std::map<double, std::pair<float,float> ,  MuTauStreamAnalyzer::more> jetMoments;

  for(unsigned int it = 0; it < jets->size() ; it++){

    Jet* jet = const_cast<Jet*>(&(*jets)[it]);

    // newJet redone using possibly different JEC ==> it may not contain infos on bTag
    // so I use it together with jet

    Jet* newJet = 0;
    if(newJets!=0) newJetMatched( jet , newJets );
    if(!newJet){
      if(verbose_) cout << "No jet from the new collection can be matched ==> using old one..." << endl;
      newJet = jet;
    }
    
    //COLIN: if there is a matching between the tau and the jet, use the jet p4.
    // otherwise, use the tau p4. for now, use the tau p4, as we don't have the jet->tau matching. 
    //     math::XYZTLorentzVectorD leg2p4 = ( (leg2->pfJetRef()).isNonnull() ) ? leg2->pfJetRef()->p4() : leg2->p4();
    math::XYZTLorentzVectorD leg2p4 = leg2->p4();

    if( Geom::deltaR(jet->p4(), leg1->p4())<deltaRLegJet_ || 
	Geom::deltaR(jet->p4(), leg2p4 )<deltaRLegJet_ ){
      if(verbose_) cout << "The jet at (" <<jet->pt()<<","<<jet->eta()<<") is closer than "<<deltaRLegJet_ << " from one of the legs" << endl;  
      continue;
    }

    /////////////////////////////////////////////////////////////////////////
    //// use JES uncertainties
    //COLIN: using uncertainty from the cmg::Jet itself.
//     edm::ESHandle<JetCorrectorParametersCollection> jetCorrParameters;
//     // get the jet corrector parameters collection from the global tag
//     iSetup.get<JetCorrectionsRecord>().get("AK5PF", jetCorrParameters);
//     // get the uncertainty parameters from the collection
//     JetCorrectorParameters const & param = (*jetCorrParameters)["Uncertainty"];
//     // instantiate the jec uncertainty object
//     JetCorrectionUncertainty* deltaJEC = new JetCorrectionUncertainty(param);
//     deltaJEC->setJetEta(newJet->eta()); deltaJEC->setJetPt(newJet->pt());
//     float shift  = deltaJEC->getUncertainty( true );
    float shift = jet->uncOnFourVectorScale();
    /////////////////////////////////////////////////////////////////////////

    if(verbose_){
      cout << "Analyzing jet with pt " << (newJet->p4()).Pt() 
	   << "and eta " << (newJet->p4()).Eta() << endl;
      cout << " ==> JEC uncertainty is " << shift*100 << " %" << endl;
      //COLINTD: we don't have the following information, but probably not needed.
      //talk to Lorenzo
//       for(unsigned int i = 0; i < (newJet->availableJECSets()).size() ; i++ ){
// 	std::cout << (newJet->availableJECSets())[i] << std::endl;
//       }
//       for(unsigned int i = 0; i < (newJet->availableJECLevels()).size() ; i++ ){
// 	std::cout << (newJet->availableJECLevels())[i] << std::endl;
//       }
//       std::cout << "L1FastJet ========> " << std::endl;
//       std::cout << "Uncorrected " << newJet->correctedJet("Uncorrected","none","patJetCorrFactors").pt() << std::endl;
//       std::cout << "L1FastJet "   << newJet->correctedJet("L1FastJet","none",  "patJetCorrFactors").pt() << std::endl;
//       std::cout << "L2Relative "  << newJet->correctedJet("L2Relative","none", "patJetCorrFactors").pt() << std::endl; 
//       std::cout << "L3Absolute "  << newJet->correctedJet("L3Absolute","none", "patJetCorrFactors").pt() << std::endl; 
//       std::cout << "L1Offset ========> " << std::endl;
//       std::cout << "Uncorrected " << newJet->jecFactor("Uncorrected","none","patJetCorrFactorsL1Offset")*newJet->pt() << std::endl;
//       std::cout << "L1Offset"     << newJet->jecFactor("L1Offset","none",   "patJetCorrFactorsL1Offset")*newJet->pt() << std::endl;
//       std::cout << "L2Relative "  << newJet->jecFactor("L2Relative","none", "patJetCorrFactorsL1Offset")*newJet->pt() << std::endl;
//       std::cout << "L3Absolute"   << newJet->jecFactor("L3Absolute","none", "patJetCorrFactorsL1Offset")*newJet->pt() << std::endl;  
    }

//     std::map<string,float> aMap;

    //COLIN : doing JetID using the cmg::PFJet "components", as the PFCandidate consistituents 
    // are not available anymore
    //     if( jetID( jet , &((*vertexes)[0]), vtxZ, aMap ) < minJetID_ ){
    if( jetID( *jet ) < minJetID_ ){
      if(verbose_){
	cout << "@@@@ Jet does not pass the Id" << endl;
      }
      continue;
    }

    //COLIN storing raw jets
    reco::Candidate::LorentzVector p4uncorrected =   newJet->rawFactor() * newJet->p4();
    sortedJets.insert( make_pair( p4uncorrected.pt() ,
				  p4uncorrected ) );

    if(newJet->p4().Pt() < minCorrPt_) continue;

    // add b-tag info
    bTaggers.insert(         make_pair( newJet->p4().Pt(), make_pair( jet->bDiscriminator("trackCountingHighEffBJetTags"),
								      jet->bDiscriminator("trackCountingHighPurBJetTags")  ) ) );

    //COLIN: pile-up charged hadrons have been removed -> meaningless to do that. 
    // all use of PV association commented. 
    // add pu information
    //     jetPVassociation.insert( make_pair( newJet->p4().Pt(), make_pair(aMap["chFracRawJetE"],
    // 								     aMap["chFracAllChargE"]) ) );

    // add jet moments. Not used at the moment. Used to parametrize the fake rate as a function of these 
    //COLIN: need to store these guys in the cmg::Jet - PRIO2
    //     jetMoments.insert(       make_pair( newJet->p4().Pt(), make_pair( jet->etaetaMoment(),
    // 								      jet->phiphiMoment()) ) );
   
    //COLIN: no access to all levels of correction. 
    // Is it necessary? if yes could re-correct on the fly. - PRIO3
    //     if(isMC_) 
    //       sortedJetsIDL1Offset.insert( make_pair( newJet->jecFactor("L3Absolute","none", "patJetCorrFactorsL1Offset")*newJet->pt() , 
    // 					      newJet->jecFactor("L3Absolute","none", "patJetCorrFactorsL1Offset")*newJet->p4()) );   
    //     else 
    //       sortedJetsIDL1Offset.insert( make_pair( newJet->jecFactor("L2L3Residual","none", "patJetCorrFactorsL1Offset")*newJet->pt() , 
    // 					      newJet->jecFactor("L2L3Residual","none", "patJetCorrFactorsL1Offset")*newJet->p4()) ); 
    
    if(verbose_) 
      cout << "Components: "
	   << "px=" << (newJet->p4()).Px() << " (" << newJet->px() << "), "
	   << "py=" << (newJet->p4()).Py() << " (" << newJet->py() << "), "
	   << "pz=" << (newJet->p4()).Pz() << " (" << newJet->pz() << "), "
	   << "E="  << (newJet->p4()).E()  << " (" << newJet->energy()  << ")"
	   << endl;
    
    sortedJetsID.insert(     make_pair( newJet->p4().Pt() ,  newJet->p4() ) );
    sortedJetsIDFracs.insert(     make_pair( newJet->p4().Pt() ,  
					     make_pair( newJet->component(PFCandidate::h).fraction(), 
							newJet->component(PFCandidate::h0).fraction() ) ) );
    sortedJetsIDUp.insert(   make_pair( newJet->p4().Pt() ,  newJet->p4()*(1+shift) ) );
    sortedJetsIDDown.insert( make_pair( newJet->p4().Pt() ,  newJet->p4()*(1-shift) ) );

    //COLIN we don't have the matched GenJet
    //just to check JEC - can be removed 
    //     if(isMC_){
    //       if(jet->genJet() != 0) sortedGenJetsID.insert( make_pair( newJet->p4().Pt() ,jet->genJet()->p4() ) );
    //       else sortedGenJetsID.insert( make_pair( newJet->p4().Pt() , math::XYZTLorentzVectorD(0,0,0,0) ) );
    //     }
    
  }
  
  //COLIN: below, feeding jet p4. 
  //contains jet p4 for all jets passing jetID
  
  //uncorrected jets, sorting by uncorrected pt.
  for(CImap it = sortedJets.begin(); it != sortedJets.end() ; it++){
    jetsP4_->push_back( it->second );
  }

  //for the following, sorting by corrected pt

  //corrected jets
  for(CImap it = sortedJetsID.begin(); it != sortedJetsID.end() ; it++){
    jetsIDP4_->push_back( it->second );
  }
  
  //up 
  for(CImap it = sortedJetsIDUp.begin(); it != sortedJetsIDUp.end() ; it++){
    jetsIDUpP4_->push_back( it->second );
  }

  //down
  for(CImap it = sortedJetsIDDown.begin(); it != sortedJetsIDDown.end() ; it++){
    jetsIDDownP4_->push_back( it->second );
  }

  //see the use of JECFactor
  for(CImap it = sortedJetsIDL1Offset.begin(); it != sortedJetsIDL1Offset.end() ; it++){
    jetsIDL1OffsetP4_->push_back( it->second );
  }

  // sorted by reco jet pT. contains the matched genjet, or p4 = 0
  for(CImap it = sortedGenJetsID.begin(); it != sortedGenJetsID.end() ; it++){
    genJetsIDP4_->push_back( it->second );
  }
  for(std::map<double, std::pair<float,float> >::iterator it = bTaggers.begin(); it != bTaggers.end() ; it++){
    jetsBtagHE_->push_back( (it->second).first  );
    jetsBtagHP_->push_back( (it->second).second );
  }
  for(std::map<double, std::pair<float,float> >::iterator it = sortedJetsIDFracs.begin(); 
      it != sortedJetsIDFracs.end() ; it++){
    jetsChEfraction_->push_back( (it->second).first  );
    jetsChNfraction_->push_back( 0 );
  }
  for(std::map<double, std::pair<float,float> >::iterator it = jetMoments.begin(); it != jetMoments.end() ; it++){
    jetMoments_->push_back( (it->second).first  );
    jetMoments_->push_back( (it->second).second );
  }

  tree_->Fill();

}





// unsigned int  MuTauStreamAnalyzer::jetID( const Jet* jet, const reco::Vertex* vtx, std::vector<float> vtxZ, std::map<std::string,float>& map_){

//   if( (jet->pt())<10 ) return 99; // always pass jet ID

//   std::vector<reco::PFCandidatePtr> pfCandPtrs = jet->getPFConstituents();

//   int nCharged = 0;
//   int nPhotons = 0;
//   int nNeutral = 0;
//   int nConst = 0;

//   float energyCharged = 0;
//   float energyPhotons = 0;
//   float energyNeutral = 0;
//   float energyElectrons = 0;
 
//   float totalEnergyFromConst = 0;

//   float chEnFractionPV   = 0;
//   float chEnFractionChPV = 0;
//   float chFractionPV     = 0;

//   for(unsigned i=0; i<pfCandPtrs.size(); ++i) {
//     const reco::PFCandidate& cand = *(pfCandPtrs[i]);

//     totalEnergyFromConst +=  cand.energy();
//     nConst += 1;

//     switch( cand.particleId() ) {
//     case reco::PFCandidate::h: 
//       nCharged++;
//       energyCharged += cand.energy(); 

//       if((cand.trackRef()).isNonnull()){
// 	bool isMatched = false;
// 	for(reco::Vertex::trackRef_iterator it = vtx->tracks_begin() ; it!=vtx->tracks_end() && !isMatched; it++){
// 	  if(  (*it).id() == (cand.trackRef()).id() && (*it).key() == (cand.trackRef()).key() ){
// 	    isMatched = true;
// 	    if(verbose_) cout << (*it).id() << ", " << (*it).key() << " is matched!" << endl;
// 	    chEnFractionPV += cand.energy();
// 	    chFractionPV+=1;
// 	  }
// 	}
// 	if(!isMatched){
// 	  float dist = vtxZ.size()>0 ? fabs(vtxZ[0]-((cand.trackRef())->referencePoint()).z()) : 999.;
// 	  float tmpDist = 999.;
// 	  for(unsigned k = 1; k < vtxZ.size(); k++){
// 	    if( fabs(((cand.trackRef())->referencePoint()).z() - vtxZ[k] ) < tmpDist )
// 	      tmpDist = fabs(((cand.trackRef())->referencePoint()).z() - vtxZ[k] );
// 	  }
// 	  if(tmpDist>dist){
// 	    isMatched = true;
// 	    if(verbose_) cout << "Matched by closest vtx in z!!" << endl;
// 	    chEnFractionPV += cand.energy();
// 	    chFractionPV+=1;
// 	  }
// 	}
// 	if(!isMatched && verbose_) {
// 	  cout << "Ch with pt " << cand.pt() << " and eta " << cand.eta() << " is not matched to the PV !!!" << endl;
// 	  cout << "z position of PV " << (vtx->position()).z() << ", z position of track " << ((cand.trackRef())->referencePoint()).z()  << ", x position of track " << ((cand.trackRef())->referencePoint()).x()   << ", y position of track " << ((cand.trackRef())->referencePoint()).y() << endl;
// 	}
//       }

//       break;
//     case reco::PFCandidate::gamma:
//       nPhotons++;
//       energyPhotons += cand.energy();
//       break;
//     case reco::PFCandidate::h0:
//       nNeutral++;
//       energyNeutral += cand.energy();
//       break;
//     case reco::PFCandidate::e: 
//       energyElectrons += cand.energy();

//       if((cand.gsfTrackRef()).isNonnull()){
// 	bool isMatched = false;
// 	float dist = vtxZ.size()>0 ? fabs(vtxZ[0]-((cand.gsfTrackRef())->referencePoint()).z()) : 999.;
// 	float tmpDist = 999.;
// 	for(unsigned k = 1; k < vtxZ.size(); k++){
// 	  if( fabs(((cand.gsfTrackRef())->referencePoint()).z() - vtxZ[k] ) < tmpDist )
// 	    tmpDist = fabs(((cand.gsfTrackRef())->referencePoint()).z() - vtxZ[k] );
// 	}
// 	if(tmpDist>dist){
// 	  isMatched = true;
// 	  if(verbose_) cout << "Matched by closest vtx in z!!" << endl;
// 	  chEnFractionPV += cand.energy();
// 	  chFractionPV+=1;
// 	}
// 	if(!isMatched && verbose_) {
// 	  cout << "Ele with pt " << cand.pt() << " and eta " << cand.eta() << " is not matched to the PV !!!" << endl;
// 	  cout << "z position of PV " << (vtx->position()).z() << ", z position of gsfTrack " << ((cand.gsfTrackRef())->referencePoint()).z()  << ", x position of gsfTrack " << ((cand.gsfTrackRef())->referencePoint()).x()   << ", y position of gsfTrack " << ((cand.gsfTrackRef())->referencePoint()).y() << endl;
// 	}
//       }
      
//       break;
//     case reco::PFCandidate::h_HF: // fill neutral
//       nNeutral++;
//       //energyNeutral += cand.energy();
//       break;
//     case reco::PFCandidate::egamma_HF: // fill e/gamma
//       nPhotons++;
//       energyPhotons += cand.energy();
//       break;
//     default:
//       break;
//     }
//   }

//   chEnFractionChPV = chEnFractionPV;
//   if(energyCharged>0 || energyElectrons>0)
//     chEnFractionChPV /= (energyCharged+energyElectrons); 
//   chEnFractionPV /= jet->correctedJet("Uncorrected").p4().E();
//   chFractionPV   /= nCharged;

//   //if(chFractionPV==0) cout << energyCharged << endl;

//   map_["chMult"]          =  chFractionPV;
//   map_["chFracRawJetE"]   =  chEnFractionPV;
//   map_["chFracAllChargE"] =  chEnFractionChPV;
        
//   bool loose=false;
//   bool medium=false;
//   bool tight=false;


//   if(verbose_){
//     cout << "NeutrFRAC = " << energyNeutral/totalEnergyFromConst << endl;
//     cout << "PhotoFRAC = " << energyPhotons/totalEnergyFromConst << endl;
//     cout << "ChargFRAC = " << energyCharged/totalEnergyFromConst << endl;
//     cout << "nConst = "    << nConst << endl;
//     cout << "nCharged = "  << nCharged << endl;
//     cout << "ElectFRAC ="  << energyElectrons/totalEnergyFromConst << endl;
//   }


//   //loose id
//   if( (TMath::Abs(jet->eta())>2.4 && 
//        energyNeutral/totalEnergyFromConst<0.99 && 
//        energyPhotons/totalEnergyFromConst<0.99 &&
//        nConst > 1) || 
//       (TMath::Abs(jet->eta())<2.4 && 
//        energyNeutral/totalEnergyFromConst<0.99 && 
//        energyPhotons/totalEnergyFromConst<0.99 &&
//        nConst > 1 &&
//        energyCharged/totalEnergyFromConst>0 &&
//        nCharged>0 &&
//        energyElectrons/totalEnergyFromConst<0.99
//        )
//       ) loose = true;
//   // medium id
//   if( (TMath::Abs(jet->eta())>2.4 && 
//        energyNeutral/totalEnergyFromConst<0.95 && 
//        energyPhotons/totalEnergyFromConst<0.95 &&
//        nConst > 1) || 
//       (TMath::Abs(jet->eta())<2.4 && 
//        energyNeutral/totalEnergyFromConst<1 && 
//        energyPhotons/totalEnergyFromConst<1 &&
//        nConst > 1 &&
//        energyCharged/totalEnergyFromConst>0 &&
//        nCharged>0 &&
//        energyElectrons/totalEnergyFromConst<1
//        )
//       ) medium = true;
//   // tight id
//   if( (TMath::Abs(jet->eta())>2.4 && 
//        energyNeutral/totalEnergyFromConst<0.90 && 
//        energyPhotons/totalEnergyFromConst<0.90 &&
//        nConst > 1) || 
//       (TMath::Abs(jet->eta())<2.4 && 
//        energyNeutral/totalEnergyFromConst<1 && 
//        energyPhotons/totalEnergyFromConst<1 &&
//        nConst > 1 &&
//        energyCharged/totalEnergyFromConst>0 &&
//        nCharged>0 &&
//        energyElectrons/totalEnergyFromConst<1
//        )
//       ) tight = true;
  
//   if(loose && !medium && !tight) return 1;
//   if(loose && medium && !tight)  return 2;
//   if(loose && medium && tight)   return 3; 
  
//   return 0;

// }



unsigned int  MuTauStreamAnalyzer::jetID( const Jet& jet ){

  if( (jet.pt())<10 ) return 99; // always pass jet ID

  float fracCharged = jet.component(PFCandidate::h).fraction();
  float fracPhotons = jet.component(PFCandidate::gamma).fraction();
  float fracNeutral = jet.component(PFCandidate::h0).fraction();
  float fracElectrons = jet.component(PFCandidate::h0).fraction();
  
  int nConst = jet.nConstituents();
  int nCharged = jet.component(PFCandidate::h).number();

  bool loose=false;
  bool medium=false;
  bool tight=false;


  if(verbose_){
    cout << "NeutrFRAC = " << fracNeutral << endl;
    cout << "PhotoFRAC = " << fracPhotons << endl;
    cout << "ChargFRAC = " << fracCharged << endl;
    cout << "nConst = "    << nConst << endl;
    cout << "nCharged = "  << nCharged << endl;
    cout << "ElectFRAC ="  << fracElectrons << endl;
  }


  //loose id
  if( (TMath::Abs(jet.eta())>2.4 && 
       fracNeutral<0.99 && 
       fracPhotons<0.99 &&
       nConst > 1) || 
      (TMath::Abs(jet.eta())<2.4 && 
       fracNeutral<0.99 && 
       fracPhotons<0.99 &&
       nConst > 1 &&
       fracCharged>0 &&
       nCharged>0 &&
       fracElectrons<0.99
       )
      ) loose = true;
  // medium id
  if( (TMath::Abs(jet.eta())>2.4 && 
       fracNeutral<0.95 && 
       fracPhotons<0.95 &&
       nConst > 1) || 
      (TMath::Abs(jet.eta())<2.4 && 
       fracNeutral<1 && 
       fracPhotons<1 &&
       nConst > 1 &&
       fracCharged>0 &&
       nCharged>0 &&
       fracElectrons<1
       )
      ) medium = true;
  // tight id
  if( (TMath::Abs(jet.eta())>2.4 && 
       fracNeutral<0.90 && 
       fracPhotons<0.90 &&
       nConst > 1) || 
      (TMath::Abs(jet.eta())<2.4 && 
       fracNeutral<1 && 
       fracPhotons<1 &&
       nConst > 1 &&
       fracCharged>0 &&
       nCharged>0 &&
       fracElectrons<1
       )
      ) tight = true;
  
  if(loose && !medium && !tight) return 1;
  if(loose && medium && !tight)  return 2;
  if(loose && medium && tight)   return 3; 
  
  return 0;

}



MuTauStreamAnalyzer::Jet* MuTauStreamAnalyzer::newJetMatched( const Jet* oldJet , 
							      const JetCollection* newJets ){

  Jet* matchedJet = 0;

  for(unsigned int it = 0; it < newJets->size() ; it++){
    if( Geom::deltaR( (*newJets)[it].p4() , oldJet->p4() )<0.01 ){
      matchedJet = const_cast<Jet*>(&((*newJets)[it]));
      break;
    }
  }

  return matchedJet;
}



void MuTauStreamAnalyzer::endJob(){}


#include "FWCore/Framework/interface/MakerMacros.h"
 
DEFINE_FWK_MODULE(MuTauStreamAnalyzer);


