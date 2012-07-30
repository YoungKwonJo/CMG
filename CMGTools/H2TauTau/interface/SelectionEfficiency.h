#ifndef CMGTools_H2TauTau_SelectionEfficiency_H
#define CMGTools_H2TauTau_SelectionEfficiency_H

#include <math.h> 
#include "TMath.h" 
#include <limits>



class SelectionEfficiency {
public:
  SelectionEfficiency(){} ;

  /*
    Factors correcting for id and isolation differences between MC and Data
  */  
  //****************
  //parameters taken from AN-11-390 v8, should be for Fall11 MC 
  //*****************  
  double effCorrMu2011AB(double pt,double eta){
    if(fabs(eta)<1.479) {//Barrel
      if(10.0<pt&&pt<=15.0) return 0.92;
      if(15.0<pt&&pt<=20.0) return 0.948;
      if(20.0<pt) return 0.9933;
      return 0;
    }else if(1.479<fabs(eta)&&fabs(eta)<2.1){//Endcap
      if(10.0<pt&&pt<=15.0) return 0.98;
      if(15.0<pt&&pt<=20.0) return 0.962;
      if(20.0<pt) return 0.9982;
      return 0;
    }
    return 0;
  }


  ///for e-Tau channel the electron id+iso corrections taken from twiki:
  //https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsToTauTauWorking2012
//   ID
// 20 < pT < 30 Barrel 	0.955 � 0.002
// 20 < pT < 30 Endcap 	0.938 � 0.007
// pT > 30 Barrel 	1.044 � 0.001
// pT > 30 Endcap 	0.977 � 0.001
// Iso
// 20 < pT < 30 Barrel 	0.980 � 0.003
// 20 < pT < 30 Endcap 	0.967 � 0.006
// pT > 30 Barrel 	0.984 � 0.001
// pT > 30 Endcap 	0.989 � 0.001 
  double effCorrEle2011AB(double pt,double eta){
    if(fabs(eta)<1.479) {//Barrel
      if(20.0<pt&&pt<=30.0) return 0.955*0.980;
      if(30.0<pt) return 1.044*0.984;
      return 0;
    }else if(1.479<fabs(eta)&&fabs(eta)<2.1){//Endcap
      if(20.0<pt&&pt<=30.0) return 0.938*0.967;
      if(30.0<pt) return 0.977*0.989;
      return 0;
    }
    return 0;
  }


  //eTau 2012 //Upto ICHEP
// 2012
// pT /  eta bin 	MC Efficiency 	Data Efficiency 	Scale Factor
// ID
// 20 < pT < 30 Barrel 	0.795 � 0.005 	0.733 � 0.003 	0.922 � 0.007
// 20 < pT < 30 Endcap 	0.433 � 0.009 	0.389 � 0.006 	0.944 � 0.026
// pT > 30 Barrel 	0.908 � 0.001 	0.876 � 0.001 	0.964 � 0.001
// pT > 30 Endcap 	0.617 � 0.004 	0.592 � 0.010 	0.958 � 0.007
// Iso
// 20 < pT < 30 Barrel 	0.734 � 0.006 	0.715 � 0.001 	0.974 � 0.008
// 20 < pT < 30 Endcap 	0.740 � 0.013 	0.745 � 0.008 	1.008 � 0.021
// pT > 30 Barrel 	0.896 � 0.001 	0.893 � 0.001 	0.997 � 0.001
// pT > 30 Endcap 	0.912 � 0.003 	0.896 � 0.001 	0.983 � 0.004 

  double effCorrEle2012AB(double pt,double eta){
    if(fabs(eta)<1.479) {//Barrel
      if(20.0<pt&&pt<=30.0) return 0.922*0.974 ;
      if(30.0<pt) return 0.964*0.997;
      return 0;
    }else if(1.479<fabs(eta)&&fabs(eta)<2.1){//Endcap
      if(20.0<pt&&pt<=30.0) return 0.944*1.008;
      if(30.0<pt) return 0.958*0.983;
      return 0;
    }
    return 0;
  }



private:
  
} ;
#endif 

