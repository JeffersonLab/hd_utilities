#!/bin/csh -f

# SET INPUTS
setenv ENVIRONMENT $1
setenv CONFIG_FILE $2
setenv GEN_NAME $3
setenv OUTDIR $4
setenv RUN_NUMBER $5
setenv FILE_NUMBER $6
setenv EVT_TO_GEN $7
setenv JANA_CALIB_CONTEXT "variation="$8
setenv GENR $9
setenv GEANT $10
setenv SMEAR $11
setenv RECON $12
setenv CLEANGENR $13
setenv CLEANGEANT $14
setenv CLEANSMEAR $15
setenv CLEANRECON $16
setenv MCSWIF $17
setenv NUMTHREADS $18
setenv GENERATOR $19
setenv GEANTVER $20
setenv CUSTOM_GCONTROL $21

if ("$GEANTVER" == "3") then
setenv NUMTHREADS 1
endif

# PRINT INPUTS
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "OUTDIR            = $OUTDIR"
echo "RUN_NUMBER        = $RUN_NUMBER"
echo "FILE_NUMBER       = $FILE_NUMBER"
echo "NUM TO GEN        = $EVT_TO_GEN"
echo "generator        = $GENERATOR"
echo "generation        = $GENR  $CLEANGENR"
echo "Geant        = $GEANT  $CLEANGEANT"
echo "MCsmear        = $SMEAR $CLEANSMEAR"
echo "Recon        = $RECON   $CLEANRECON"
# ENVIRONMENT
echo $ENVIRONMENT
source $ENVIRONMENT
echo pwd = $PWD
#printenv
#necessary to run swif, uses local directory if swif=0 is used
if ("$MCSWIF" == "1") then
mkdir $OUTDIR
mkdir $OUTDIR/log

if ("$CUSTOM_GCONTROL"=="") then
    cp $MCWRAPPER_CENTRAL/Gcontrol.in ./
else
    cp $CUSTOM_GCONTROL/Gcontrol.in ./
endif

endif



if ("$GENR" != "0") then
    if ("$GENERATOR" != "genr8" && "$GENERATOR" != "bggen") then
	echo "NO VALID GENERATOR GIVEN"
	exit
    endif

    if ("$GENERATOR" == "genr8") then
	echo "configuring genr8"
	cp $CONFIG_FILE ./
    else if ("$GENERATOR" == "bggen") then
	echo "configuring bggen"
	cp $MCWRAPPER_CENTRAL/Generators/bggen/particle.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia-geant.map ./
	cp $CONFIG_FILE ./bggen\_$RUN_NUMBER\_$FILE_NUMBER.conf
    endif



    if ("$GENERATOR" == "genr8") then
	echo "RUNNING GENR8"
	set RUNNUM = $RUN_NUMBER+$FILE_NUMBER
	# RUN genr8 and convert
	if ( -f $CONFIG_FILE ) then
	    echo " input file found"
	else
	    echo $CONFIG_FILE" does not exist"
	    exit
	endif
	genr8 -r$RUNNUM -M$EVT_TO_GEN -A$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.ascii < $CONFIG_FILE
	genr8_2_hddm $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.ascii
    else if ("$GENERATOR" == "bggen") then

	set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
	if ("$colsize" == "B" || "$colsize" == "R" ) then
	set colsize = "34"
	endif
	set RANDOM=$$
	echo $RANDOM
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' bggen\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPRUNNO/'$RUN_NUMBER'/' bggen\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPCOLD/'0.00$colsize'/' bggen\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPrand/'$RANDOM'/' bggen\_$RUN_NUMBER\_$FILE_NUMBER.conf
	ln -s bggen\_$RUN_NUMBER\_$FILE_NUMBER.conf fort.15
	bggen
	mv bggen.hddm $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm
    endif

#GEANT/smearing
#modify TEMPIN/TEMPOUT/TEMPTRIG/TOSMEAR in control.in

    if ("$GEANT" != "0") then
	echo "RUNNING GEANT"$GEANTVER
	set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
	if ("$colsize" == "B" || "$colsize" == "R" ) then
	set colsize = "34"
	endif

	set inputfile=$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER
	cp $MCWRAPPER_CENTRAL/Gcontrol.in $PWD/control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPIN/'$inputfile.hddm'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPRUNG/'$RUN_NUMBER'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPOUT/'$inputfile'_geant.hddm/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TOSMEAR/'$SMEAR'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPCOLD/'0.00$colsize'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	mv $PWD/control'_'$RUN_NUMBER'_'$FILE_NUMBER.in $PWD/control.in
	
	if ("$GEANTVER" == "3") then
	hdgeant 
	else if ("$GEANTVER" == "4") then
	#make run.mac then call it below
	rm run.mac
	echo /run/beamOn $EVT_TO_GEN >> run.mac
	echo "exit" >> run.mac
	hdgeant4 -t$NUMTHREADS run.mac
	rm run.mac
	else
	echo "INVALID GEANT VERSION"
	exit
	endif

	#run reconstruction
	if ("$CLEANGENR" == "1") then
	    if ("$GENERATOR" == "genr8") then
		rm *.ascii
	    else if ("$GENERATOR" == "bggen") then
		rm particle.dat
		rm pythia.dat
		rm pythia-geant.map
		unlink fort.15
	    endif
	
	    rm $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm
	endif

	if ("$RECON" != "0") then
	    echo "RUNNING RECONSTRUCTION"
	    hd_root $inputfile'_geant_smeared.hddm' -PPLUGINS=danarest,monitoring_hists -PNTHREADS=$NUMTHREADS
	    mv hd_root.root hd_root_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root
	    mv dana_rest.hddm dana_rest_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm

	    if ("$CLEANGEANT" == "1") then
	    rm *_geant.hddm
	    endif

	    if ("$CLEANSMEAR" == "1") then
	    rm *_smeared.hddm
	    rm smear.root
	    endif

	    if ("$CLEANRECON" == "1") then
	    rm dana_rest*
	    endif

	endif
    endif
endif

if ("$MCSWIF" == "1") then
    cp $PWD/*.hddm $OUTDIR
    cp $PWD/*.ascii $OUTDIR
endif
