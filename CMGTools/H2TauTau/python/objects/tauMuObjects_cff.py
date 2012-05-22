import FWCore.ParameterSet.Config as cms

from CMGTools.Common.diTau_cff import *
from CMGTools.H2TauTau.objects.tauMuCuts_cff import * 

from CMGTools.Common.generator.metRecoilCorrection.metRecoilCorrection_cff import *
from CMGTools.Common.factories.cmgTauScaler_cfi import  cmgTauScaler
from CMGTools.Common.factories.cmgTauMuCor_cfi import cmgTauMuCor 
from CMGTools.H2TauTau.objects.tauMuSVFit_cfi import tauMuSVFit 



# no correction, no svfit ---------------------------------------------------

# attaching the cuts defined in this module
# to the di-tau factory
cmgTauMu.cuts = tauMuCuts.clone()
cmgTauMu.cfg.leg1Collection = 'cmgTauScaler'

# preselection 
cmgTauMuPreSel = cmgTauMuSel.clone( cut = 'getSelection("cuts_baseline")')

# full selection
## cmgTauMuFullSel = cmgTauMuSel.clone( src = 'cmgTauMuPreSel',
##                                      cut = 'getSelection("cuts_baseline")' )

tauMuStdSequence = cms.Sequence( cmgTauScaler +
                                 cmgTauMu +
                                 cmgTauMuPreSel
                                 # +
                                 # cmgTauMuFullSel
                                 )


# correction and svfit ------------------------------------------------------

# this is done for preselected di-taus

# mva MET

from CMGTools.Common.eventCleaning.goodPVFilter_cfi import goodPVFilter
from CMGTools.Common.miscProducers.mvaMET.mvaMET_cff import *
from CMGTools.Common.factories.cmgBaseMETFromPFMET_cfi import cmgBaseMETFromPFMET
mvaMETTauMu.recBosonSrc = 'cmgTauMuPreSel'
#WARNING: jet collections have to be redone if correction wrong

mvaBaseMETTauMu = cmgBaseMETFromPFMET.clone()
mvaBaseMETTauMu.cfg.inputCollection = 'mvaMETTauMu'

cmgTauMuMVAPreSel = cmgTauMuCor.clone()
cmgTauMuMVAPreSel.cfg.metCollection = 'mvaBaseMETTauMu'
cmgTauMuMVAPreSel.cfg.diObjectCollection = 'cmgTauMuPreSel'

mvaMETSequence = cms.Sequence(
    goodPVFilter + 
    mvaMETTauMu +
    mvaBaseMETTauMu
    # +
    # cmgTauMuMVAPreSel
    )

# recoil correction
## doMVAMet = True
## metForRecoil = 'cmgPFMETRaw'
## diTausForRecoil = 'cmgTauMuPreSel'
## if doMVAMet:
##     # in this case the MET will be taken from the di-tau
##     metForRecoil = 'mvaBaseMETTauMu'
##     diTausForRecoil = 'cmgTauMuMVAPreSel'

## recoilCorMETTauMu =  recoilCorrectedMETTauMu.clone(
##     recBosonSrc = diTausForRecoil,
##     metSrc = metForRecoil
##     )

cmgTauMuCorPreSel = cmgTauMuCor.clone()
cmgTauMuCorPreSel.cfg.metCollection = 'mvaBaseMETTauMu'
cmgTauMuCorPreSel.cfg.diObjectCollection = 'cmgTauMuPreSel'

# SVFit

cmgTauMuCorSVFitPreSel = tauMuSVFit.clone()
cmgTauMuCorSVFitPreSel.diTauSrc = 'cmgTauMuCorPreSel'
cmgTauMuCorSVFitFullSel = cmgTauMuSel.clone( src = 'cmgTauMuCorSVFitPreSel',
                                             cut = 'getSelection("cuts_baseline")' ) 
#WARNING here, need to read MET significance from MVA!

tauMuCorSVFitSequence = cms.Sequence( # recoilCorMETTauMu +
    mvaMETSequence + 
    cmgTauMuCorPreSel +
    cmgTauMuCorSVFitPreSel +
    cmgTauMuCorSVFitFullSel
    )

## if doMVAMet:
##     tauMuCorSVFitSequence.insert(0, mvaMETSequence)

tauMuSequence = cms.Sequence( tauMuStdSequence +
                              tauMuCorSVFitSequence
                              )


