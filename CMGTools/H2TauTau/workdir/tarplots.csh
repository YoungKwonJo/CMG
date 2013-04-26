export path="/data/benitezj/Samples"

##mu-tau 2012
#export tag="muTau201253X_580Dec13"
#export tag="muTau201253X_580Dec21"
#export tag="muTau201253X_580Jan15"
#export tag="muTau201253X_580Jan16"
#export tag="muTau201253X_580Jan29"
#export tag="muTau201253X_580Feb7"
#export tag="muTau201253X_5140Apr5"
#export tag="muTau201253X_5140Apr7"
#export tag="muTau201253X_5140Apr11"
#export tag="muTau201253X_5140Apr14"

###e-tau 2012
#export tag="eTau201253X_580Jan16"
#export tag="eTau201253X_580Feb7"
#export tag="eTau201253X_5140Apr5"
#export tag="eTau201253X_5140Apr7"
#export tag="eTau201253X_5140Apr11"
#export tag="eTau201253X_5140Apr13"
export tag="eTau201253X_5140Apr14"


export dirname=Plots_${tag}_${1}
echo $dirname


############################################
rm -rf $dirname
mkdir $dirname

mkdir $dirname/pdflog
mv $path/$tag/*log.pdf $dirname/pdflog

mkdir $dirname/pdf
mv $path/$tag/*.pdf $dirname/pdf

mkdir $dirname/pnglog
mv $path/$tag/*log.png $dirname/pnglog

mkdir $dirname/png
mv $path/$tag/*.png $dirname/png

mkdir $dirname/C
mv $path/$tag/*.C $dirname/C

mkdir $dirname/root
mv $path/$tag/*.root $dirname/root

mkdir $dirname/ps
mv $path/$tag/*.ps $dirname/ps
###########################################