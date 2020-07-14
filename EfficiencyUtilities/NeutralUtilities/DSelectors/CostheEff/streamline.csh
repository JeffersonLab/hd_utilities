#!/bin/csh -f
set echo
#
# streamline.csh
# Elton Smith. Sep 18, 2017. 
# Streamline instructions to process the output of mcsmear output files through amplitude analysis.
# Update Feb 13, 2018: Modify script to use Paul's new ReactionFilter Plugin (See https://halldweb.jlab.org/doc-private/DocDB/ShowDocument?docid=3407)
# Update Jan 11, 2019: update pippim -> pi0pi0, 2pi -> 2pi0
# Update Feb 13, 2019: modify for PhotonCostheEff 
#
setenv JANA_CALIB_CONTEXT "variation=default"     # change to access variation=mc

unset run

if( $#argv == 0 ) then
#    set run = "031055"
     set run = "051384"
     set run1 = "051384"
else if ( $#argv == 1) then
     set run = $1
else
    echo "Too many arguments=" $1 $2 
endif
# set run = ${run1}"0"

set dir = "/work/halld/home/elton/PhotonCostheEff/Anal2020"
# set indir = "/w/halld-scifs17exp/home/elton/PhotonCostheEff/"
set indir = "/cache/halld/RunPeriod-2017-01/analysis/ver27/tree_pi0pippim__B4/merged/"
# set indir = "/cache/halld/RunPeriod-2018-01/analysis/ver02/tree_pi0pippim__B4/merged/"
# set dataset = "2018Spring"
set dataset = "2017"

echo " indir =" $indir " dir=" $dir
echo " run =" $run

# rm -f tree_pi0pippim__B4_gen_amp.root
# echo hadd tree_pi0pippim__B4_gen_amp_${run}.root ${indir}tree_pi0pippim__B4_gen_amp_${run1}*.root
# hadd tree_pi0pippim__B4_gen_amp_${run}.root ${indir}tree_pi0pippim__B4_gen_amp_${run1}*.root

echo root -b -q ${indir}tree_pi0pippim__B4_${run}.root 'call_DSelector.C("DSelector_pi0pippim__B4_ver21.C+")' >! ${dir}/DataList/DSelector_pi0pippim__B4_${run}.list
root -b -q ${indir}tree_pi0pippim__B4_${run}.root 'call_DSelector.C("DSelector_pi0pippim__B4_ver21.C+")' >! ${dir}/DataList/DSelector_pi0pippim__B4_${run}.list
# echo root -b -q  plot_pi0pippim.C\(\"DSelector_pi0pippim__B4\"\)
# root -b -q  plot_pi0pippim.C\(\"DSelector_pi0pippim__B4\"\)

echo cp DSelector_pi0pippim__B4.root ${dir}/DataRoot${dataset}/DSelector_pi0pippim__B4_${run}.root
cp DSelector_pi0pippim__B4.root ${dir}/DataRoot${dataset}/DSelector_pi0pippim__B4_${run}.root
cp tree_DSelector_pi0pippim__B4.root ${dir}/DataTree${dataset}/tree_DSelector_pi0pippim__B4_${run}.root

unset echo
