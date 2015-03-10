#!/bin/tcsh -f

set NEVENTS=10000
set VERTEX="0 0 50 80"  # x y zmin zmax
set NTHREADS=1
set b1pi_input=${HALLD_HOME}/src/programs/Simulation/genr8/InputFiles/b1_pi.input

if ( ! -e ${b1pi_input} ) then
	echo " "
	echo "No file: ${b1pi_input}"
	echo "Please make sure your HALLD_HOME env. var. is set."
	echo " "
	exit -1
endif

echo "Copying ${b1pi_input} ..."
cp $b1pi_input b1_pi.input

echo "Running genr8 ..."
genr8 -r1501 -M${NEVENTS} -Ab1_pi.ascii < b1_pi.input

echo "Converting generated events to HDDM ..."
genr8_2_hddm -V"${VERTEX}" b1_pi.ascii 
mv output.hddm b1_pi.hddm

echo "Creating control.in file ..."
cat - << EOF > control.in

INFILE 'b1_pi.hddm'
TRIG ${NEVENTS}
OUTFILE 'hdgeant.hddm'
RNDM 123
HADR 1

EOF

echo "Running hdgeant ..."
hdgeant

echo "Running mcsmear ..."
mcsmear -PJANA:BATCH_MODE=1 hdgeant.hddm

echo "Running hd_root with danarest ..."
#nice hd_root --nthreads=Ncores -PPLUGINS=phys_tree hdgeant_smeared.hddm
nice hd_root -PJANA:BATCH_MODE=1 --nthreads=Ncores -PPLUGINS=danarest -PBFIELD_MAP=Magnets/Solenoid/solenoid_1350_poisson_20130925 hdgeant_smeared.hddm

echo "Running hd_root with b1pi_hists & monitoring_hists ..."
#nice hd_root --nthreads=Ncores -PPLUGINS=phys_tree hdgeant_smeared.hddm
nice hd_root -PJANA:BATCH_MODE=1 --nthreads=Ncores -PPLUGINS=b1pi_hists,monitoring_hists dana_rest.hddm

#echo "Analyzing ntuple and make histograms"
#root -b -q anal1.C

echo "Create plots"
#root -b -q makeallplots.C
root -b -q mk_pics.C



