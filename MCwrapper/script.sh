#!/bin/csh -f

# SET INPUTS
setenv ENVIRONMENT $1
setenv CONFIG_FILE $2
setenv INDIR $3
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

# PRINT INPUTS
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "OUTDIR            = $OUTDIR"
echo "RUN_NUMBER        = $RUN_NUMBER"
echo "FILE_NUMBER       = $FILE_NUMBER"
echo "NUM TO GEN        = $EVT_TO_GEN"
echo "genr8        = $GENR  $CLEANGENR"
echo "Geant        = $GEANT  $CLEANGEANT"
echo "MCsmear        = $SMEAR $CLEANSMEAR"
echo "Recon        = $RECON   $CLEANRECON"
# ENVIRONMENT

source $ENVIRONMENT
echo pwd = $PWD
#printenv
#necessary to run swif, uses local directory if swif=0 is used
if ("$MCSWIF" == "1") then
mkdir $OUTDIR
midir $OUTDIR/log
cp $INDIR/Gcontrol.in ./
cp $INDIR/$CONFIG_FILE.input ./
endif

if ("$GENR" != "0") then
    echo "RUNNING GENR8"
    set RUNNUM = $RUN_NUMBER+$FILE_NUMBER
    # RUN genr8 and convert
    if ( -f $CONFIG_FILE.input ) then
	echo " input file found"
    else
	echo $CONFIG_FILE".input does not exist"
	exit
    endif
    genr8 -r$RUNNUM -M$EVT_TO_GEN -A$CONFIG_FILE\_$RUN_NUMBER\_$FILE_NUMBER.ascii < $CONFIG_FILE.input
    genr8_2_hddm $CONFIG_FILE\_$RUN_NUMBER\_$FILE_NUMBER.ascii


#GEANT/smearing
#modify TEMPIN/TEMPOUT/TEMPTRIG/TOSMEAR in control.in

    if ("$GEANT" != "0") then
	echo "RUNNING GEANT"
	set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
	if ("$colsize" == "B" || "$colsize" == "R" ) then
	set colsize = "34"
	endif

	set inputfile=$CONFIG_FILE\_$RUN_NUMBER\_$FILE_NUMBER
	cp $PWD/Gcontrol.in $PWD/control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPIN/'$inputfile.hddm'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPOUT/'$inputfile'_geant.hddm/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TOSMEAR/'$SMEAR'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPCOLD/'0.00$colsize'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	mv $PWD/control'_'$RUN_NUMBER'_'$FILE_NUMBER.in $PWD/control.in
	hdgeant #$CONFIG_FILE\_$RUN_NUMBER\_$FILE_NUMBER.hddm $PWD/control.in
#run reconstruction
	if ("$CLEANGENR" == "1") then
	rm *.ascii
	rm $CONFIG_FILE\_$RUN_NUMBER\_$FILE_NUMBER.hddm
	endif

	if ("$RECON" != "0") then
	    echo "RUNNING RECONSTRUCTION"
	    hd_root $inputfile'_geant_smeared.hddm' --plugin=danarest -PNTHREADS=$NUMTHREADS
	    mv dana_rest.hddm dana_rest_$CONFIG_FILE\_$RUN_NUMBER\_$FILE_NUMBER.hddm

	    if ("$CLEANGEANT" == "1") then
	    rm *_geant.hddm
	    endif

	    if ("$CLEANSMEAR" == "1") then
	    rm *_smeared.hddm
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
