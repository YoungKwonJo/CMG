#!/usr/bin/env python
import os,string,sys,commands,time,ConfigParser
from ROOT import *
from array import array

# legenda:
# step 0--> all
# step 1--> after njet cut
# step 2--> after cut on energy
# step 3--> after 2 tau candidate
# step 4--> after testID
# step 5--> after findhz


# format: file,xsec(fb),tag for legenda
mclist=[
    ["htt/Hig125_104/htttreeproducer_httanalyzer/htttreeproducer_httanalyzer_tree.root",200.,"HZ"]
#   [ "zz.root",1446.,"ZZ"],
#   ["ww.root",14080.,"WW"],
#   ["qq.root",16750.,"QQ"]
    ]


# luminosity to normalize (in fb-1)
lumi=500

# step at which the plot should be made
stepplot=4

# define histograms
step_h=[]
genrec_s3_t1=[]
genrec_s3_t2=[]
massh_h=[]
massz_h=[]
mzh_h2=[]

for index in range(0,len(mclist)):
    step_h.append(TH1F("step"+str(index),"step"+str(index),10,0,10))
    step_h[index].SetLineColor(index+2)
    step_h[index].SetMarkerColor(index+2)
    step_h[index].SetFillColor(index+2)
    massh_h.append(TH1F("Hmass"+str(index),"Hmass"+str(index),200,0,400))
    massh_h[index].SetLineColor(index+2)
    massh_h[index].SetMarkerColor(index+2)
    massh_h[index].SetFillColor(index+2)
    massz_h.append(TH1F("Zmass"+str(index),"Zmass"+str(index),200,0,400))
    massz_h[index].SetLineColor(index+2)
    massz_h[index].SetMarkerColor(index+2)
    massz_h[index].SetFillColor(index+2)

    mzh_h2.append(TH2F("mzvsmh"+str(index),"mzvsmh"+str(index),200,0,400,200,0,400))
    mzh_h2[index].SetLineColor(index+2)
    mzh_h2[index].SetMarkerColor(index+2)
    mzh_h2[index].SetFillColor(index+2)


    genrec_s3_t1.append(TH1F("genrec_s3_t1","genrec_s3_t1",100,-1,1))
    genrec_s3_t2.append(TH1F("genrec_s3_t2","genrec_s3_t2",100,-1,1))
                 

# now loop on tree and project
for index,mc in enumerate(mclist):
    rootfile=mc[0]
    xsec=mc[1]
    tag=mc[2]
    treefile=TFile.Open(rootfile)
    tree=treefile.Get("htttreeproducer_httanalyzer")
    nevents=tree.GetEntries()
    # loop on tree entries
    weight=xsec*lumi/nevents
    
    print weight,nevents
    for event  in tree:
        for bin in range(0,int(event.step)+1):
            if index==0:
                if event.g_ishtt==1 and event.g_isHZqq==1:
                    step_h[index].Fill(bin)
            else:
                step_h[index].Fill(bin)
                
        if event.step>=stepplot:
            # here we can put all plots after selection
            if index==0:
                if event.g_ishtt==1 and event.g_isHZqq==1:
#                    print 'ok'
                    genrec_s3_t1[index].Fill(event.genRecDistance1)
                    genrec_s3_t2[index].Fill(event.genRecDistance2)
                    massh_h[index].Fill(event.recHMass)   
                    massz_h[index].Fill(event.recZMass)   
                    mzh_h2[index].Fill(event.recHMass,event.recZMass)
        continue
    treefile.Close()
    # renormalize step_h histo for efficiencies
    norm=step_h[index].GetBinContent(1)
    for bin in range(0,step_h[index].GetNbinsX()):
        step_h[index].SetBinContent(bin,step_h[index].GetBinContent(bin)/norm)
        
    
# now we can plot them
c1=TCanvas("c1","c1",800,600)
c1.SetFillColor(kWhite)
c1.SetFillStyle(1001)
#gStyle.SetOptStat(0)
 
first=True
# legenda
yheaderstart=.95-.023*len(mclist)
leg_hist = TLegend(0.7,yheaderstart,.98,.98);

#c1.Divide(1,2)
for index in range(0,len(mclist)):
    opt="same"
    mc=mclist[index]
    if (first):
        first=False
        opt=""
        step_h[index].Draw(opt)
#    c1.cd(1)
#    genrec_s3_t1[index].Draw(opt)
#    c1.cd(2)
#    genrec_s3_t2[index].Draw(opt)

    leg_hist.AddEntry(step_h[index],mc[2],"l")

leg_hist.SetFillColor(0)# Have a white background
leg_hist.Draw() 
c1.Print("cut_chain.png")

c2=TCanvas("c2","c2",800,600)
stackh_h = THStack("HmassS", "HmassS")
for index in range(0,len(mclist)):
    stackh_h.Add(massh_h[index])
stackh_h.Draw("nostack")
c2.Print("hmass.png")


c3=TCanvas("c3","c3",800,600)
stackz_h = THStack("ZmassS", "ZmassS")
for index in range(0,len(mclist)):
    stackz_h.Add(massz_h[index])
stackz_h.Draw()
c3.Print("zmass.png")

c4=TCanvas("c4","c4",800,600)
c4.cd()
mzh_h2[0].Draw()
c4.Update()
c4.Print("mzmh.png")


#stackmzh_h2 = THStack("mzmh", "mzmh")
#for index in range(0,len(mclist)):
#    stackmzh_h2.Add(mzh_h2[index])
#stackmzh_h2.Draw("box")



a=raw_input("hit a key to exit...")