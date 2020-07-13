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
source ~/MC_environment.csh
unset run

if( $#argv == 0 ) then
#     set run = "030804"
#     set run1 = "0304"
#    set run1 = "03045"
#     set run1 = "05144"
     set run1 = "030274_000"
#     set run1 = "030800"
else if ( $#argv == 1) then
#    set run1 = "030274"
#    set run1 = "30796-030895"
    set run1 = $1
else
    echo "Too many arguments=" $1 $2 
endif
# set run = "0"${run1}
set run = ${run1}

# set Gversion = "G4"

set dir = "/w/halld-scifs17exp/home/elton/PhotonCostheEff/Anal2020/"
#if (${run1} == 30345) then
#    set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piWideMass_t4_30345_20190418033817pm/hddm/"
#else if (${run1} == 30346) then
#    set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piWideMass_t4_30346_20190418034019pm/hddm/"
#else if (${run1} == 30351) then
#    set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piWideMass_t4_30351_20190418033913pm/hddm/"
#else if (${run1} == 30823) then
#    set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piWideMass_t4_30823_20190418034120pm/hddm/"
#else if (${run1} == 30824) then
#    set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piWideMass_t4_30824_20190418034132pm/hddm/"
#else if (${run1} == 30830) then
#    set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piWideMass_t4_30830_20190418034145pm/hddm/"
#else
#    echo "*** Unknown run=" ${run}
#endif

set MCPeriod = "2017"
# set MCPeriod = "2018Spring"
# set MCPeriod = "2018Fall"
# set MCPeriod = "2018LowEnergy"
set indir = "/cache/halld/gluex_simulations/REQUESTED_MC/gen_omega_3pi_efficiency_2017_01_20200525020622pm/hddm/"
# set indir = "/cache/halld/gluex_simulations/REQUESTED_MC/gen_omega_3pi_efficiency_2018_01_20200525023506pm/hddm/"
# set indir = "/cache/halld/gluex_simulations/REQUESTED_MC/gen_omega_3pi_efficiency_2018_08_20200603092642pm/hddm/"
#
# set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp__3piHighMass_t2.5__G4_20190501054440pm/hddm/"
# set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_3piHighMass_t2_20190401055042pm/hddm/"
# set indir = "/w/halld-scifs17exp/home/elton/PhotonCostheEff/MCTrees"${Gversion}"/"
# set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp__omega3pi_t2_${Gversion}_20190322051445pm/root/trees/"
# set indir = "/cache/halld/halld-scratch/REQUESTED_MC/LowE/gen_amp_omega_3pi_20190201020108pm/root/"
# set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_omega_3pi_t2_20190206045522pm/root/"
# set indir = "/cache/halld/halld-scratch/REQUESTED_MC/2017/gen_amp_omega_3pi_20190206034547pm/root/"
# set indir = "/w/halld-scifs17exp/home/elton/PhotonCostheEff/"

echo " indir =" $indir
echo " run =" $run " run1=" $run1

echo hd_root ${indir}dana_rest_gen_amp_${run}.hddm --config=jana.config
hd_root ${indir}dana_rest_gen_amp_${run}.hddm --config=jana.config
cp tree_pi0pippim__B4.root  ${dir}MCTree${MCPeriod}/tree_pi0pippim__B4_gen_amp_${run}.root
echo root -b -q tree_pi0pippim__B4_gen_amp_${run}.root 'call_DSelector.C("DSelector_pi0pippim__B4_ver21.C+")' >! ${dir}MCList/DSelector_pi0pippim__B4_${run}.list
root -b -q tree_pi0pippim__B4.root 'call_DSelector.C("DSelector_pi0pippim__B4_ver21.C+")' >! ${dir}MCList/DSelector_pi0pippim__B4_${run}.list
echo cp DSelector_pi0pippim__B4.root ${dir}MCRoot${MCPeriod}/DSelector_pi0pippim__B4_gen_amp_${run}.root
cp DSelector_pi0pippim__B4.root ${dir}MCRoot${MCPeriod}/DSelector_pi0pippim__B4_gen_amp_${run}.root


unset echo
