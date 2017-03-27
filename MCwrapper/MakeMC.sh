#!/bin/bash

# SET INPUTS
export ENVIRONMENT=$1 
shift
export CONFIG_FILE=$1
shift
export GEN_NAME=$1
shift
export OUTDIR=$1
shift
export RUN_NUMBER=$1
shift
export FILE_NUMBER=$1
shift
export EVT_TO_GEN=$1
shift
export JANA_CALIB_CONTEXT "variation="$1
shift
export GENR=$1
shift
export GEANT=$1
shift
export SMEAR=$1
shift
export RECON=$1
shift
export CLEANGENR=$1
shift
export CLEANGEANT=$1
shift
export CLEANSMEAR=$1
shift
export CLEANRECON=$1
shift
export MCSWIF=$1
shift
export NUMTHREADS=$1
shift
export GENERATOR=$1
shift
export GEANTVER=$1
shift
export BKGFOLDSTR=$1
shift
export CUSTOM_GCONTROL=$1
shift
export eBEAM_ENERGY=$1
shift
export COHERENT_PEAK=$1
shift
export GEN_MIN_ENERGY=$1
shift
export GEN_MAX_ENERGY=$1

if [[ "$GEANTVER" == "3" ]]; then
    export NUMTHREADS=1
fi

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
echo "GCONTROL        = $CUSTOM_GCONTROL"
echo "BKG_FOLD      = $BKGFOLDSTR"
echo "MCsmear        = $SMEAR $CLEANSMEAR"
echo "Recon        = $RECON   $CLEANRECON"

echo "detected bash"
#printenv
#necessary to run swif, uses local directory if swif=0 is used
if [[ "$MCSWIF" -eq "1" ]]; then
    # ENVIRONMENT
    echo $ENVIRONMENT
    source $ENVIRONMENT
    echo pwd = $PWD
    mkdir -p $OUTDIR
    mkdir -p $OUTDIR/log
fi

if [[ "$CUSTOM_GCONTROL" -eq "0"  ]]; then
    cp $MCWRAPPER_CENTRAL/Gcontrol.in ./
else
    cp $CUSTOM_GCONTROL/Gcontrol.in ./
fi

if [[ "$GENR" != "0" ]]; then
    if [[ "$GENERATOR" != "genr8" && "$GENERATOR" != "bggen" && "$GENERATOR" != "genEtaRegge" && "$GENERATOR" != "gen_2pi_amp" && "$GENERATOR" != "gen_pi0" && "$GENERATOR" != "gen_2pi_primakoff"  ]]; then
	echo "NO VALID GENERATOR GIVEN"
	echo "only [genr8, bggen, genEtaRegge, gen_2pi_amp, gen_pi0] are supported"
	exit
    fi
    
    if [[ -f $CONFIG_FILE  ]]; then
	echo " input file found"
    else
	echo $CONFIG_FILE" does not exist"
	exit
    fi
    
    if [[ "$GENERATOR" -eq "genr8" ]]; then
	echo "configuring genr8"
	cp $CONFIG_FILE ./genr8\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
    else if [[ "$GENERATOR" -eq "bggen" ]]; then
	echo "configuring bggen"
	cp $MCWRAPPER_CENTRAL/Generators/bggen/particle.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia-geant.map ./
	cp $CONFIG_FILE ./bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	if [[ `echo $eBEAM_ENERGY | grep -o "\." | wc -l` -eq 0 ]]; then
	    set eBEAM_ENERGY = $eBEAM_ENERGY\.
	fi
	if [[ `echo $COHERENT_PEAK | grep -o "\." | wc -l` -eq 0 ]]; then
	    set COHERENT_PEAK = $COHERENT_PEAK\.
	fi
	if [[ `echo $GEN_MIN_ENERGY | grep -o "\." | wc -l` -eq 0 ]]; then
	    set GEN_MIN_ENERGY = $GEN_MIN_ENERGY\.
	fi
	if [[ `echo $GEN_MAX_ENERGY | grep -o "\." | wc -l` -eq 0 ]]; then
	    set GEN_MAX_ENERGY = $GEN_MAX_ENERGY\.
	fi
	
    else if [[ "$GENERATOR" -eq "genEtaRegge" ]]; then
	echo "configuring genEtaRegge"
	cp $CONFIG_FILE ./genEtaRegge\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
    else if [[ "$GENERATOR" -eq "gen_2pi_amp" ]]; then
	echo "configuring gen_2pi_amp"
	cp $CONFIG_FILE ./gen_2pi_amp\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
    else if [[ "$GENERATOR" -eq "gen_2pi_primakoff" ]]; then
	echo "configuring gen_2pi_primakoff"
	cp $CONFIG_FILE ./gen_2pi_primakoff\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
    else if [[ "$GENERATOR" -eq "gen_pi0" ]]; then
	echo "configuring gen_pi0"
	cp $CONFIG_FILE ./gen_pi0\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
    fi
    set config_file_name=`basename "$CONFIG_FILE"`
    echo $config_file_name
    
    if [[ "$GENERATOR" -eq "genr8" ]]; then
	echo "RUNNING GENR8"
	set RUNNUM = $RUN_NUMBER+$FILE_NUMBER
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' genr8\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	# RUN genr8 and convert
	genr8 -r$RUN_NUMBER -M$EVT_TO_GEN -A$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.ascii < genr8\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf #$config_file_name
	genr8_2_hddm $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.ascii
    else if [[ "$GENERATOR" -eq "bggen" ]]; then
	set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
	if [[ "$colsize" -eq "B" || "$colsize" -eq "R"  ]]; then
	    set colsize = "34"
	fi
	set RANDOM=$$
	echo $RANDOM
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPRUNNO/'$RUN_NUMBER'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPCOLD/'0.00$colsize'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPRAND/'$RANDOM'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	
	ln -s bggen\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf fort.15
	bggen
	mv bggen.hddm $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm
    else if [[ "$GENERATOR" -eq "genEtaRegge" ]]; then
	echo "RUNNING GENETAREGGE" 
	set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
	if [[ "$colsize" -eq "B" || "$colsize" -eq "R"  ]]; then
	    set colsize = "34"
	fi
	sed -i 's/TEMPCOLD/'0.00$colsize'/' genEtaRegge\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' genEtaRegge\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' genEtaRegge\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' genEtaRegge\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' genEtaRegge\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
	genEtaRegge -N$EVT_TO_GEN -O$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm -I'genEtaRegge'\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf
    else if [[ "$GENERATOR" -eq "gen_2pi_amp" ]]; then
	echo "RUNNING GEN_2PI_AMP" 
        set optionals_line = `head -n 1 $config_file_name | sed -r 's/.//'`
	echo $optionals_line
	echo gen_2pi_amp -c gen_2pi_amp\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf -o $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm -hd $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root -n $EVT_TO_GEN -r $RUN_NUMBER  -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_2pi_amp -c gen_2pi_amp\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf -hd $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm -o $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY - b $GEN_MAX_ENERGY $optionals_line
    else if [[ "$GENERATOR" -eq "gen_2pi_primakoff" ]]; then
	echo "RUNNING GEN_2PI_PRIMAKOFF" 
        set optionals_line = `head -n 1 $config_file_name | sed -r 's/.//'`
	echo $optionals_line
	echo gen_2pi_primakoff -c gen_2pi_primakoff\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf -o $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm -hd $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root -n $EVT_TO_GEN -r $RUN_NUMBER  -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_2pi_primakoff -c gen_2pi_primakoff\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf -hd $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm -o $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY - b $GEN_MAX_ENERGY $optionals_line
    else if [[ "$GENERATOR" -eq "gen_pi0" ]]; then
	echo "RUNNING GEN_PI0" 
        set optionals_line = `head -n 1 $config_file_name | sed -r 's/.//'`
	echo $optionals_line
	gen_pi0 -c gen_pi0\_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.conf -hd $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm -o $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK  -s $FILE_NUMBER $optionals_line -m $eBEAM_ENERGY
    fi
    
    #GEANT/smearing
    #modify TEMPIN/TEMPOUT/TEMPTRIG/TOSMEAR in control.in
    
    if [[ "$GEANT" != "0" ]]; then
	echo "RUNNING GEANT"$GEANTVER
	set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
	if [[ "$colsize" -eq "B" || "$colsize" -eq "R"  ]]; then
	    set colsize = "34"
	fi
	
	set inputfile=$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER
	cp Gcontrol.in $PWD/control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPIN/'$inputfile.hddm'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPRUNG/'$RUN_NUMBER'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPOUT/'$inputfile'_geant.hddm/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	#sed -i 's/TOSMEAR/'$SMEAR'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	sed -i 's/TEMPCOLD/'0.00$colsize'/' control'_'$RUN_NUMBER'_'$FILE_NUMBER.in
	mv $PWD/control'_'$RUN_NUMBER'_'$FILE_NUMBER.in $PWD/control.in
	
	if [[ "$GEANTVER" -eq "3" ]]; then
	    hdgeant 
	else if [[ "$GEANTVER" -eq "4" ]]; then
	    #make run.mac then call it below
	    rm run.mac
	    echo /run/beamOn $EVT_TO_GEN >> run.mac
	    echo "exit" >> run.mac
	    hdgeant4 -t$NUMTHREADS run.mac
	    rm run.mac
	else
	    echo "INVALID GEANT VERSION"
	    exit
	fi
	
	if [[ "$SMEAR" != "0" ]]; then
	    echo "RUNNING MCSMEAR"
	    
	    if [[ "$BKGFOLDSTR" -eq "0" ]]; then
		mcsmear -o$inputfile'_geant_smeared.hddm' $inputfile'_geant.hddm'
	    else
		echo 'mcsmear -o'$inputfile'_geant_smeared.hddm'' '$inputfile'_geant.hddm'' '$BKGFOLDSTR
		mcsmear -o$inputfile'_geant_smeared.hddm' $inputfile'_geant.hddm' $BKGFOLDSTR
	    fi
	    #run reconstruction
	    if [[ "$CLEANGENR" -eq "1" ]]; then
		if [[ "$GENERATOR" -eq "genr8" ]]; then
		    rm *.ascii
		else if [[ "$GENERATOR" -eq "bggen" ]]; then
		    rm particle.dat
		    rm pythia.dat
		    rm pythia-geant.map
		    unlink fort.15
		fi
		
		rm $GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm
	    fi
	    
	    if [[ "$RECON" != "0" ]]; then
		echo "RUNNING RECONSTRUCTION"
		hd_root $inputfile'_geant_smeared.hddm' -PPLUGINS=danarest,monitoring_hists -PNTHREADS=$NUMTHREADS
		mv hd_root.root hd_root_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.root
		mv dana_rest.hddm dana_rest_$GEN_NAME\_$RUN_NUMBER\_$FILE_NUMBER.hddm
		
		if [[ "$CLEANGEANT" -eq "1" ]]; then
		    rm *_geant.hddm
		    rm Gcontrol.in
		    
		fi
		
		if [[ "$CLEANSMEAR" -eq "1" ]]; then
		    rm *_smeared.hddm
		    rm smear.root
		fi
		
		if [[ "$CLEANRECON" -eq "1" ]]; then
		    rm dana_rest*
		fi
	    fi
	fi
    fi
fi

if [[ ! -d "$OUTDIR" ]]; then
    mkdir $OUTDIR
fi
if [[ ! -d "$OUTDIR/configurations/" ]]; then
    mkdir $OUTDIR/configurations/
fi
if [[ ! -d "$OUTDIR/hddm/" ]]; then
    mkdir $OUTDIR/hddm/
fi
if [[ ! -d "$OUTDIR/root/" ]]; then
    mkdir $OUTDIR/root/
fi
mv $PWD/*.conf $OUTDIR/configurations/
mv $PWD/*.hddm $OUTDIR/hddm/
mv $PWD/*.root $OUTDIR/root/
