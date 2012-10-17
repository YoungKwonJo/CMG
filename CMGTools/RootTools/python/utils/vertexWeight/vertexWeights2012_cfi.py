import FWCore.ParameterSet.Config as cms

import os 
rootfile_dir = os.environ['CMSSW_BASE'] + '/src/CMGTools/RootTools/data/vertexWeight'
#centraldir = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions12/8TeV/PileUp'


#for 52X MC to 2012 ICHEP data set
vertexWeightSummer12MCICHEPData = cms.EDProducer(
    "PileUpWeightProducer",
    verbose = cms.untracked.bool( False ),
    src = cms.InputTag('addPileupInfo'),
    type = cms.int32(2), # 1 = observed , 2= true 
    inputHistMC = cms.string( rootfile_dir + '/Pileup_Summer12MC52X.true.root'),
    inputHistData = cms.string( rootfile_dir + '/Pileup_2012ICHEP_start_196509.true.root' ),
    )

#for 53X MC to 2012 HCP data set
vertexWeightSummer12MC53XHCPData = cms.EDProducer(
    "PileUpWeightProducer",
    verbose = cms.untracked.bool( False ),
    src = cms.InputTag('addPileupInfo'),
    type = cms.int32(2), # 1 = observed , 2= true 
    inputHistMC = cms.string( rootfile_dir + '/Pileup_Summer12MC53X.true.root'),
    inputHistData = cms.string( rootfile_dir + '/Pileup_2012HCP_190456_203002.true.root' ),
    )