import imp
import math
import copy
import time
import re

#from CMGTools.H2TauTau.proto.HistogramSet import histogramSet
from CMGTools.H2TauTau.proto.plotter.H2TauTauDataMC import H2TauTauDataMC
from CMGTools.H2TauTau.proto.plotter.prepareComponents import prepareComponents
from CMGTools.H2TauTau.proto.plotter.rootutils import buildCanvas, draw
from CMGTools.H2TauTau.proto.plotter.categories_TauEle import *
from CMGTools.H2TauTau.proto.plotter.binning import binning_svfitMass
from CMGTools.H2TauTau.proto.plotter.titles import xtitles
from CMGTools.H2TauTau.proto.plotter.blind import blind
from CMGTools.H2TauTau.proto.plotter.plotmod import *
from CMGTools.H2TauTau.proto.plotter.datacards import *
from CMGTools.H2TauTau.proto.plotter.embed import *
from CMGTools.H2TauTau.proto.plotter.plotinfo import *
from CMGTools.RootTools.Style import *
from ROOT import kPink, TH1, TPaveText, TPad

cp = copy.deepcopy
EWK = 'WJets'

    
NBINS = 100
XMIN  = 0
XMAX  = 200


def replaceShapeInclusive(plot, var, anaDir,
                          comp, weights, 
                          cut, weight,
                          embed, shift=None):
    '''Replace WJets with the shape obtained using a relaxed tau iso'''
    cut = cut.replace('l1_looseMvaIso>0.5', 'l1_rawMvaIso>-0.5')
    print '[INCLUSIVE] estimate',comp.name,'with cut',cut
    plotWithNewShape = cp( plot )
    wjyield = plot.Hist(comp.name).Integral()
    nbins = plot.bins
    xmin = plot.xmin
    xmax = plot.xmax
    wjshape = shape(var, anaDir,
                    comp, weights, nbins, xmin, xmax,
                    cut, weight,
                    embed, shift=shift, treeName = 'H2TauTauTreeProducerTauEle')
    # import pdb; pdb.set_trace()
    wjshape.Scale( wjyield )
    # import pdb; pdb.set_trace()
    plotWithNewShape.Replace(comp.name, wjshape) 
    # plotWithNewShape.Hist(comp.name).on = False 
    return plotWithNewShape

    

def makePlot( var, anaDir, selComps, weights, wJetScaleSS, wJetScaleOS,
              nbins=None, xmin=None, xmax=None,
              cut='', weight='weight', embed=False, shift=None, replaceW=False):
    
    print 'making the plot:', var, 'cut', cut

    oscut = cut+' && diTau_charge==0'
    osign = H2TauTauDataMC(var, anaDir,
                           selComps, weights, nbins, xmin, xmax,
                           cut=oscut, weight=weight,
                           embed=embed, shift=shift, treeName = 'H2TauTauTreeProducerTauEle')
    osign.Hist(EWK).Scale( wJetScaleOS )
    if replaceW:
        osign = replaceShapeInclusive(osign, var, anaDir,
                                      selComps['WJets'], weights, 
                                      oscut, weight,
                                      embed, shift=shift)    
    sscut = cut+' && diTau_charge!=0'
    ssign = H2TauTauDataMC(var, anaDir,
                           selComps, weights, nbins, xmin, xmax,
                           cut=sscut, weight=weight,
                           embed=embed, shift=shift, treeName = 'H2TauTauTreeProducerTauEle')
    ssign.Hist(EWK).Scale( wJetScaleSS ) 
    # import pdb; pdb.set_trace()
    if replaceW:
        ssign = replaceShapeInclusive(ssign, var, anaDir,
                                      selComps['WJets'], weights, 
                                      sscut, weight,
                                      embed, shift=shift)

    from CMGTools.H2TauTau.proto.plotter.plotmod import getQCD
    ssQCD, osQCD = getQCD( ssign, osign, 'Data', 1.06 ) #PG FIXME check the scale 1.06
        
#    if 0:
#        qcd_yield = osQCD.Hist('QCD').Integral()
#        
#        sscut_qcdshape = cut.replace('l2_relIso05<0.1','l2_relIso05>0.2').replace('l1_looseMvaIso>0.5', 'l1_rawMvaIso>-0.75') + ' && diTau_charge!=0' 
#        ssign_qcdshape = H2TauTauDataMC(var, anaDir,
#                                        selComps, weights, nbins, xmin, xmax,
#                                        cut=sscut_qcdshape, weight=weight,
#                                        embed=embed, shift=shift, treeName = 'H2TauTauTreeProducerTauEle')
#        qcd_shape = copy.deepcopy( ssign_qcdshape.Hist('Data') )    
#        qcd_shape.Normalize()
#        qcd_shape.Scale(qcd_yield)
#        # qcd_shape.Scale( qcd_yield )
#        osQCD.Replace('QCD', qcd_shape)

    osQCD.Group('VV', ['WW','WZ','ZZ'])
    osQCD.Group('EWK', ['WJets', 'Ztt_ZL', 'Ztt_ZJ','VV'])
    osQCD.Group('Higgs 125', ['HiggsVBF125', 'HiggsGGH125', 'HiggsVH125'])
    return ssign, osign, ssQCD, osQCD


def drawAll(cut, plots, embed, selComps, weights, fwss, fwos):
    '''See plotinfo for more information'''
    for plot in plots.values():
        print plot.var
        ss, os, ssQ, osQ = makePlot( plot.var, anaDir,
                                     selComps, weights, fwss, fwos,
                                     plot.nbins, plot.xmin, plot.xmax,
                                     cut, weight=weight, embed=embed)

        osQ.legendOn = False
        print 'drawing ', plot.var
        draw(osQ, False, 'TauEle')
        plot.ssign = cp(ss)
        plot.osign = cp(os)
        plot.ssQCD = cp(ssQ)
        plot.osQCD = cp(osQ)
        time.sleep(1)




if __name__ == '__main__':

    import copy
    from optparse import OptionParser
    from CMGTools.RootTools.RootInit import *

    parser = OptionParser()
    parser.usage = '''
    %prog <anaDir> <cfgFile>

    cfgFile: analysis configuration file, see CMGTools.H2TauTau.macros.MultiLoop
    anaDir: analysis directory containing all components, see CMGTools.H2TauTau.macros.MultiLoop.
    hist: histogram you want to plot
    '''
    parser.add_option("-H", "--hist", 
                      dest="hist", 
                      help="histogram list",
                      default=None)
    parser.add_option("-C", "--cut", 
                      dest="cut", 
                      help="cut to apply in TTree::Draw",
                      default='1')
    parser.add_option("-E", "--embed", 
                      dest="embed", 
                      help="Use embedd samples.",
                      action="store_true",
                      default=False)
    parser.add_option("-B", "--blind", 
                      dest="blind", 
                      help="Blind.",
                      action="store_true",
                      default=False)
    parser.add_option("-n", "--nbins", 
                      dest="nbins", 
                      help="Number of bins",
                      default=None)
    parser.add_option("-m", "--min", 
                      dest="xmin", 
                      help="xmin",
                      default=None)
    parser.add_option("-M", "--max", 
                      dest="xmax", 
                      help="xmax",
                      default=None)
    parser.add_option("-g", "--higgs", 
                      dest="higgs", 
                      help="Higgs mass: 125, 130,... or dummy",
                      default=None)
    parser.add_option("-p", "--plots", 
                      dest="plots", 
                      help="plots: set it to true to make control plots",
                      default=False)
    
    (options,args) = parser.parse_args()
    if len(args) != 2:
        parser.print_help()
        sys.exit(1)

    if options.nbins is None:
        NBINS = binning_svfitMass
        XMIN = None
        XMAX = None
    else:
        NBINS = int(options.nbins)
        XMIN = float(options.xmin)
        XMAX = float(options.xmax)
        
    cutstring = options.cut
    options.cut = replaceCategories(options.cut, categories) 

    print 'CUT APPLIED:', options.cut
    
    # TH1.AddDirectory(False)
    dataName = 'Data'
    weight='weight'
    replaceW = False
    useW11 = False
    
    anaDir = args[0].rstrip('/')
    shift = None
    if anaDir.endswith('_Down'):
        shift = 'Down'
    elif anaDir.endswith('_Up'):
        shift = 'Up'
        
    cfgFileName = args[1]
    file = open( cfgFileName, 'r' )
    cfg = imp.load_source( 'cfg', cfgFileName, file)
    embed = options.embed

    origComps = copy.deepcopy(cfg.config.components)
    
    # WJet normalization
    comps = []
    for comp in cfg.config.components:
        if comp.name == 'W3Jets': continue
        if comp.name == 'W2Jets': continue
        if comp.name == 'TTJets11': continue
        if useW11:
            if comp.name == 'WJets': continue
        else:
            if comp.name == 'WJets11': continue
        comps.append( comp )
    aliases = None
    if useW11:
        aliases = {'WJets11':'WJets'}

    cfg.config.components = comps

    selComps, weights, zComps = prepareComponents(anaDir, cfg.config, None, 
                                                  options.embed, 'TauEle', options.higgs)
    #import pdb ; pdb.set_trace()
    #print 'SELCOMPS:',selComps

    cutw = options.cut.replace('mt<40', '1')
    fwss, fwos, ss, os = plot_W( anaDir, selComps, weights,
                                 24, 70, 130, cutw,
                                 weight=weight, embed=options.embed,
                                 treeName='H2TauTauTreeProducerTauEle', replaceW=replaceW)

    can0 = TCanvas('can0','',100,100,600,600)

    #PG compare the MC-subtracted data to the WJets MC only for SS
    #PG ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

    W_ss = copy.deepcopy( ss )
    W_ss_WJets = W_ss.Hist('WJets').weighted
    W_ss_Data = W_ss.Hist('Data - DY - TT').weighted
    W_ss_WJets.SetFillColor (0)
    W_ss_WJets.Draw ('hist')
    W_ss_Data.Draw ('same')
    can0.Print ('compare_W_ss.png','png')
    W_ss_Data.Divide (W_ss_WJets)
    W_ss_WJets.Divide (W_ss_WJets)
#    W_ss_WJets.Divide (W_ss_Data)
    can0.DrawFrame(W_ss_WJets.GetXaxis().GetXmin(), 0.5,W_ss_WJets.GetXaxis().GetXmax(), 2)
    W_ss_Data.Draw ('same')
    W_ss_WJets.SetFillStyle (4001)
    W_ss_WJets.SetFillColor (2)
    W_ss_WJets.SetMarkerStyle (9)
    W_ss_WJets.Draw ('sameE3')
    can0.Print ('compare_W_ss_ratio.png','png')

    #import pdb ; pdb.set_trace()

    #PG compare the MC-subtracted data to the WJets MC only for OS
    #PG ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

    W_os = copy.deepcopy( os )
    W_os_WJets = W_os.Hist('WJets').weighted
    W_os_Data = W_os.Hist('Data - DY - TT').weighted
    W_os_WJets.SetFillColor (0)
    W_os_WJets.Draw ('hist')
    W_os_Data.Draw ('same')
    can0.Print ('compare_W_os.png','png')
    W_os_Data.Divide (W_os_WJets)
    W_os_WJets.Divide (W_os_WJets)
#    W_os_WJets.Divide (W_os_Data)
    can0.DrawFrame(W_os_WJets.GetXaxis().GetXmin(), 0.5,W_os_WJets.GetXaxis().GetXmax(), 2)
    W_os_Data.Draw ('same')
    W_os_WJets.SetFillStyle (4001)
    W_os_WJets.SetFillColor (2)
    W_os_WJets.SetMarkerStyle (9)
    W_os_WJets.Draw ('sameE3')
    can0.Print ('compare_W_os_ratio.png','png')

    can, pad, padr = buildCanvas()
    draw(ss, False, 'TauEle', plotprefix = 'W_ss')
    draw(os, False, 'TauEle', plotprefix = 'W_os')

    if (options.plots == 'True') :
        drawAll(options.cut, plots_TauEle, options.embed, selComps, weights, fwss, fwos)
        #PG this does not work yet, it does not get the DY right
    else :
    
        ssign, osign, ssQCD, osQCD = makePlot( options.hist, anaDir, selComps, weights, 
                                               fwss, fwos, NBINS, XMIN, XMAX, 
                                               options.cut, weight=weight, embed=options.embed, 
                                               replaceW=replaceW )
        draw(ssign, False, 'TauEle', 'QCD_ss')
        draw(osign, False, 'TauEle', 'QCD_os')
#        osQCD.legendOn = False
        draw(osQCD, options.blind, 'TauEle')
        datacards(osQCD, cutstring, shift, 'TauEle')
