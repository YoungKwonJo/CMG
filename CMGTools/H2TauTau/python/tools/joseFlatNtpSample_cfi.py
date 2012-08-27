import FWCore.ParameterSet.Config as cms

import os 
rootfile_dir = os.environ['CMSSW_BASE'] + '/src/CMGTools/Common/data/metRecoilCorrection/'
httdata_dir = os.environ['CMSSW_BASE'] + '/src/CMGTools/H2TauTau/data/'


flatNtpTauMu = cms.EDAnalyzer(
    "TauMuFlatNtp",
    path = cms.untracked.string(""),
    printSelectionPass = cms.int32(0),
    dataPeriodFlag = cms.int32(2011),
    firstRun =  cms.int32( 0 ),
    lastRun =  cms.int32( 0 ),
    dataType = cms.int32(-1),
    pupWeightName = cms.InputTag(""),
    trigPath1 = cms.InputTag(""),
    trigPath2 = cms.InputTag(""),
    trigPath3 = cms.InputTag(""),
    trigPath4 = cms.InputTag(""),
    trigPath5 = cms.InputTag(""),
    trigPath6 = cms.InputTag(""),
    trigPath7 = cms.InputTag(""),
    trigPath8 = cms.InputTag(""),
    trigPath9 = cms.InputTag(""),
    trigPath10 = cms.InputTag(""),
    diTauTag =  cms.InputTag(""),
    verticesListTag = cms.InputTag("goodOfflinePrimaryVertices"), #note that these need to be created in the FLATNTP step
    genParticlesTag =  cms.InputTag("genParticlesPruned"),
    trigPathsListTag = cms.InputTag("cmgTriggerObjectSel"),
    trigObjsListTag = cms.InputTag("cmgTriggerObjectListSel"),
    pfJetListTag =  cms.InputTag("cmgPFJetSel"),
    diMuonVetoListTag =  cms.InputTag("cmgMuonSel"),
    sampleGenEventType =   cms.int32( 0 ),
    sampleTruthEventType =  cms.int32( 0 ),
    randsigma = cms.double(0.),
    recoilCorrection = cms.int32(0), # 0 =no correction , 1=Z or Higgs , 2=W
    recoiliScale = cms.double(0.),
    fileCorrectTo = cms.string(rootfile_dir + 'recoilfit_wjets_njet.root'), #need to configure this for each sample
    fileZmmData = cms.string(rootfile_dir + 'recoilfit_datamm_njet.root'),
    fileZmmMC = cms.string(rootfile_dir + 'recoilfit_zmm42X_njet.root'),
    metType = cms.int32(1), # 1=PFMET, 2=MVAMET
    mvaMETSigTag = cms.InputTag("mvaMETTauMu"),
    runSVFit = cms.int32(2), # 0 do not run, 1 run old version, 2 run new version
    mvaWeights = cms.string(httdata_dir + 'VBFMVA_BDTG.weights.44X.xml'),
    muPtCut = cms.double(17.),
    tauPtCut = cms.double(20.),
    muEtaCut = cms.double(2.1),
    tauEtaCut = cms.double(2.3),
    signalWeightDir =  cms.string(httdata_dir),
    signalWeightMass =  cms.string('0'),
    )


flatNtpTauEle = cms.EDAnalyzer(
    "TauEleFlatNtp",
    path = cms.untracked.string(""),
    printSelectionPass = cms.int32(0),
    dataPeriodFlag = cms.int32(2011),
    firstRun =  cms.int32( 0 ),
    lastRun =  cms.int32( 0 ),
    dataType =  cms.int32(-1),
    pupWeightName = cms.InputTag(""),
    trigPath1 = cms.InputTag(""),
    trigPath2 = cms.InputTag(""),
    trigPath3 = cms.InputTag(""),
    trigPath4 = cms.InputTag(""),
    trigPath5 = cms.InputTag(""),
    trigPath6 = cms.InputTag(""),
    trigPath7 = cms.InputTag(""),
    trigPath8 = cms.InputTag(""),
    trigPath9 = cms.InputTag(""),
    trigPath10 = cms.InputTag(""),
    diTauTag =  cms.InputTag(""),
    verticesListTag = cms.InputTag("goodOfflinePrimaryVertices"),
    genParticlesTag =  cms.InputTag("genParticlesPruned"),
    trigPathsListTag = cms.InputTag("cmgTriggerObjectSel"),
    trigObjsListTag = cms.InputTag("cmgTriggerObjectListSel"),
    pfJetListTag =  cms.InputTag("cmgPFJetSel"),
    diMuonVetoListTag =  cms.InputTag("cmgElectronSel"),
    sampleGenEventType =   cms.int32( 0 ),
    sampleTruthEventType =  cms.int32( 0 ),
    randsigma = cms.double(0.),
    recoilCorrection = cms.int32(0), # 0 =no correction , 1=Z or Higgs , 2=W
    recoiliScale = cms.double(0.),
    fileCorrectTo = cms.string(rootfile_dir + 'recoilfit_wjets_njet.root'), #need to configure this for each sample
    fileZmmData = cms.string(rootfile_dir + 'recoilfit_datamm_njet.root'),
    fileZmmMC = cms.string(rootfile_dir + 'recoilfit_zmm42X_njet.root'),
    metType = cms.int32(1), # 1=PFMET, 2=MVAMET
    mvaMETSigTag = cms.InputTag("mvaMETTauEle"),
    runSVFit = cms.int32(1), # 0 do not run, 1 run old version, 2 run new version
    mvaWeights = cms.string(httdata_dir + 'VBFMVA_BDTG.weights.44X.xml'),
    muPtCut = cms.double(20.),
    tauPtCut = cms.double(20.),
    muEtaCut = cms.double(2.1),
    tauEtaCut = cms.double(2.3),
    signalWeightDir =  cms.string(httdata_dir),
    signalWeightMass =  cms.string('0'),
    )