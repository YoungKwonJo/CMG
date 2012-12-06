#!/bin/env python

def var( tree, varName, type=float ):
    tree.var(varName, type)

def fill( tree, varName, value ):
    tree.fill( varName, value )

#-------------------
# GENERIC PARTICLE
#-------------------

def bookParticle( tree, pName ):
    var(tree, '{pName}_pt'.format(pName=pName))
    var(tree, '{pName}_eta'.format(pName=pName))
    var(tree, '{pName}_phi'.format(pName=pName))
    var(tree, '{pName}_mass'.format(pName=pName))
    var(tree, '{pName}_energy'.format(pName=pName))

def fillParticle( tree, pName, particle ):
    fill(tree, '{pName}_pt'.format(pName=pName), particle.pt() )
    fill(tree, '{pName}_eta'.format(pName=pName), particle.eta() )
    fill(tree, '{pName}_phi'.format(pName=pName), particle.phi() )
    fill(tree, '{pName}_mass'.format(pName=pName), particle.mass() )
    fill(tree, '{pName}_energy'.format(pName=pName), particle.energy() )
    

#----------
# LEPTON
#----------

def bookLepton( tree, pName ):
    bookParticle(tree, pName )
    var(tree, '{pName}_pdgId'.format(pName=pName))
    var(tree, '{pName}_charge'.format(pName=pName))
    var(tree, '{pName}_isGood'.format(pName=pName),bool)
    
def fillLepton( tree, pName, lepton ):
    fillParticle(tree, pName, lepton )
    fill(tree, '{pName}_pdgId'.format(pName=pName), lepton.pdgId() )
    fill(tree, '{pName}_charge'.format(pName=pName), lepton.charge() )
    fill(tree, '{pName}_charge'.format(pName=pName), lepton.isGood() )
    

# jet

##    /// particle types
##     enum ParticleType {
##       X=0,     // undefined
##       h,       // charged hadron
##       e,       // electron 
##       mu,      // muon 
##       gamma,   // photon
##       h0,      // neutral hadron
##       h_HF,        // HF tower identified as a hadron
##       egamma_HF    // HF tower identified as an EM particle
##     };

def bookJet( tree, pName ):
    bookParticle(tree, pName )
    var(tree, '{pName}_btagCSV'.format(pName=pName))
#     var(tree, '{pName}_puMvaFull'.format(pName=pName))
#     var(tree, '{pName}_puMvaSimple'.format(pName=pName))
#     var(tree, '{pName}_puMvaCutBased'.format(pName=pName))
#     var(tree, '{pName}_puJetId'.format(pName=pName))
#     var(tree, '{pName}_looseJetId'.format(pName=pName))
#     var(tree, '{pName}_chFrac'.format(pName=pName))
#     var(tree, '{pName}_eFrac'.format(pName=pName))
#     var(tree, '{pName}_muFrac'.format(pName=pName))
#     var(tree, '{pName}_gammaFrac'.format(pName=pName))
#     var(tree, '{pName}_h0Frac'.format(pName=pName))
#     var(tree, '{pName}_hhfFrac'.format(pName=pName))
#     var(tree, '{pName}_ehfFrac'.format(pName=pName))
    bookParticle(tree, '{pName}_leg'.format(pName=pName))

def fillJet( tree, pName, jet ):
    fillParticle(tree, pName, jet )
    fill(tree, '{pName}_btagCSV'.format(pName=pName), jet.btag('') )
#     fill(tree, '{pName}_puMvaFull'.format(pName=pName), jet.puMva('full') )
#     fill(tree, '{pName}_puMvaSimple'.format(pName=pName), jet.puMva('simple'))
#     fill(tree, '{pName}_puMvaCutBased'.format(pName=pName), jet.puMva('cut-based'))
#     fill(tree, '{pName}_puJetId'.format(pName=pName), jet.puJetIdPassed)
#     fill(tree, '{pName}_looseJetId'.format(pName=pName), jet.pfJetIdPassed)
#     fill(tree, '{pName}_chFrac'.format(pName=pName), jet.component(1).fraction() )
#     fill(tree, '{pName}_eFrac'.format(pName=pName), jet.component(2).fraction() )
#     fill(tree, '{pName}_muFrac'.format(pName=pName), jet.component(3).fraction() )
#     fill(tree, '{pName}_gammaFrac'.format(pName=pName), jet.component(4).fraction() )
#     fill(tree, '{pName}_h0Frac'.format(pName=pName), jet.component(5).fraction() )
#     fill(tree, '{pName}_hhfFrac'.format(pName=pName), jet.component(6).fraction() )
#     fill(tree, '{pName}_ehfFrac'.format(pName=pName), jet.component(7).fraction() )
    if hasattr(jet, 'leg') and jet.leg:
        fillParticle(tree, '{pName}_leg'.format(pName=pName), jet.leg )
    