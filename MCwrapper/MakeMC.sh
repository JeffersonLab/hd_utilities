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
export JANA_CALIB_CONTEXT="variation="$1
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
echo "CONTEXT           = $JANA_CALIB_CONTEXT"
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "OUTDIR            = $OUTDIR"
echo "RUN_NUMBER        = $RUN_NUMBER"
echo "FILE_NUMBER       = $FILE_NUMBER"
echo "NUM TO GEN        = $EVT_TO_GEN"
echo "generator         = $GENERATOR"
echo "generation        = $GENR  $CLEANGENR"
echo "Geant             = $GEANT  $CLEANGEANT"
echo "GCONTROL          = $CUSTOM_GCONTROL"
echo "BKG_FOLD          = $BKGFOLDSTR"
echo "MCsmear           = $SMEAR   $CLEANSMEAR"
echo "Recon             = $RECON   $CLEANRECON"

echo "detected bash"

#printenv
#necessary to run swif, uses local directory if swif=0 is used
if [[ "$MCSWIF" == "1" ]]; then
    # ENVIRONMENT
    echo $ENVIRONMENT
    source $ENVIRONMENT
    echo pwd=$PWD
    mkdir -p $OUTDIR
    mkdir -p $OUTDIR/log
fi

if [[ "$CUSTOM_GCONTROL" == "0" ]]; then
    cp $MCWRAPPER_CENTRAL/Gcontrol.in ./temp_Gcontrol.in
    chmod 777 ./temp_Gcontrol.in
else
    cp $CUSTOM_GCONTROL/Gcontrol.in ./temp_Gcontrol.in
fi

length_count=$((`echo $RUN_NUMBER | wc -c` - 1))

formatted_runNumber=""
while [ $length_count -lt 6 ]; do
    formatted_runNumber="0""$formatted_runNumber"
    length_count=$(($length_count + 1))
done

formatted_runNumber=$formatted_runNumber$RUN_NUMBER
flength_count=$((`echo $FILE_NUMBER | wc -c` - 1))

formatted_fileNumber=""
while [ $flength_count -lt 3 ]; do
    formatted_fileNumber="0""$formatted_fileNumber"
    flength_count=$(($flength_count + 1))
done

formatted_fileNumber=$formatted_fileNumber$FILE_NUMBER


set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
if [[ "$colsize" == "B" || "$colsize" == "R" || "$JANA_CALIB_CONTEXT" != "variation=mc" ]]; then
    set colsize="50"
fi

if [[ "$GENR" != "0" ]]; then
    if [["$GENERATOR" != "genr8" && "$GENERATOR" != "bggen" && "$GENERATOR" != "genEtaRegge" && "$GENERATOR" != "gen_2pi_amp" && "$GENERATOR" != "gen_pi0" && "$GENERATOR" != "gen_2pi_primakoff" ]]; then
	echo "NO VALID GENERATOR GIVEN"
	echo "only [genr8, bggen, genEtaRegge, gen_2pi_amp, gen_pi0] are supported"
	exit
    fi
    
    if [[ -f $CONFIG_FILE ]]; then
	echo " input file found"
    else
	echo $CONFIG_FILE" does not exist"
	exit
    fi
    
    if [[ "$GENERATOR" == "genr8" ]]; then
	echo "configuring genr8"
	cp $CONFIG_FILE ./genr8\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
    elif [[ "$GENERATOR" == "bggen" ]]; then
	echo "configuring bggen"
	cp $MCWRAPPER_CENTRAL/Generators/bggen/particle.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia-geant.map ./
	cp $CONFIG_FILE ./bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	if [[ `echo $eBEAM_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
	    set eBEAM_ENERGY=$eBEAM_ENERGY\.
	fi
	if [[ `echo $COHERENT_PEAK | grep -o "\." | wc -l` == 0 ]]; then
	    set COHERENT_PEAK=$COHERENT_PEAK\.
	fi
	if [[ `echo $GEN_MIN_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
	    set GEN_MIN_ENERGY=$GEN_MIN_ENERGY\.
	fi
	if [[ `echo $GEN_MAX_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
	    set GEN_MAX_ENERGY=$GEN_MAX_ENERGY\.
	fi
	
    elif [[ "$GENERATOR" == "genEtaRegge" ]]; then
	echo "configuring genEtaRegge"
	cp $CONFIG_FILE ./genEtaRegge\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
    elif [[ "$GENERATOR" == "gen_2pi_amp" ]]; then
	echo "configuring gen_2pi_amp"
	cp $CONFIG_FILE ./gen_2pi_amp\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
    elif [[ "$GENERATOR" == "gen_2pi_primakoff" ]]; then
	echo "configuring gen_2pi_primakoff"
	cp $CONFIG_FILE ./gen_2pi_primakoff\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
    elif [[ "$GENERATOR" == "gen_pi0" ]]; then
	echo "configuring gen_pi0"
	cp $CONFIG_FILE ./gen_pi0\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
    fi
    set config_file_name=`basename "$CONFIG_FILE"`
    echo $config_file_name
    
    if [[ "$GENERATOR" == "genr8" ]]; then
	echo "RUNNING GENR8"
	set RUNNUM=$formatted_runNumber+$formatted_fileNumber
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' genr8\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	# RUN genr8 and convert
	genr8 -r$formatted_runNumber -M$EVT_TO_GEN -A$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.ascii < genr8\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf #$config_file_name
	genr8_2_hddm $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.ascii
    elif [[ "$GENERATOR" == "bggen" ]]; then
	set RANDOM=$$
	echo $RANDOM
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPRUNNO/'$RUN_NUMBER'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPCOLD/'0.00$colsize'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPRAND/'$RANDOM'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	
	ln -s bggen\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf fort.15
	bggen
	mv bggen.hddm $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm
    elif [[ "$GENERATOR" == "genEtaRegge" ]]; then
	echo "RUNNING GENETAREGGE" 
	
	sed -i 's/TEMPCOLD/'0.00$colsize'/' genEtaRegge\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' genEtaRegge\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' genEtaRegge\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' genEtaRegge\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' genEtaRegge\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
	genEtaRegge -N$EVT_TO_GEN -O$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm -I'genEtaRegge'\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf
    elif [[ "$GENERATOR" == "gen_2pi_amp" ]]; then
	echo "RUNNING GEN_2PI_AMP" 
        set optionals_line=`head -n 1 $config_file_name | sed -r 's/.//'`
	echo $optionals_line
	echo gen_2pi_amp -c gen_2pi_amp\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf -o $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm -hd $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.root -n $EVT_TO_GEN -r $RUN_NUMBER  -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_2pi_amp -c gen_2pi_amp\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf -hd $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm -o $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY - b $GEN_MAX_ENERGY $optionals_line
    elif [[ "$GENERATOR" == "gen_2pi_primakoff" ]]; then
	echo "RUNNING GEN_2PI_PRIMAKOFF" 
        set optionals_line=`head -n 1 $config_file_name | sed -r 's/.//'`
	echo $optionals_line
	echo gen_2pi_primakoff -c gen_2pi_primakoff\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf -o $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm -hd $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.root -n $EVT_TO_GEN -r $RUN_NUMBER  -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_2pi_primakoff -c gen_2pi_primakoff\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf -hd $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm -o $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY - b $GEN_MAX_ENERGY $optionals_line
    elif [[ "$GENERATOR" == "gen_pi0" ]]; then
	echo "RUNNING GEN_PI0" 
        set optionals_line=`head -n 1 $config_file_name | sed -r 's/.//'`
	echo $optionals_line
	gen_pi0 -c gen_pi0\_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.conf -hd $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm -o $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK  -s $formatted_fileNumber $optionals_line -m $eBEAM_ENERGY
    fi
    
    #GEANT/smearing
    
    if [[ "$GEANT" != "0" ]]; then
	echo "RUNNING GEANT"$GEANTVER
	
	if [[ `echo $eBEAM_ENERGY | grep -o "\." | wc -l` == 0]]; then
	    set eBEAM_ENERGY=$eBEAM_ENERGY\.
	fi
	if [[ `echo $COHERENT_PEAK | grep -o "\." | wc -l` == 0]]; then
	    set COHERENT_PEAK=$COHERENT_PEAK\.
	fi
	
	set inputfile=$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber
	cp temp_Gcontrol.in $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	chmod 777 $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPIN/'$inputfile.hddm'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPRUNG/'$RUN_NUMBER'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPOUT/'$inputfile'_geant.hddm/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPCOLD/'0.00$colsize'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	
	if [[ "$BKGFOLDSTR" != "BeamPhotons" ]]; then
	    echo "removing Beam Photon background from geant simulation"
	    sed -i 's/BGRATE/cBGRATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/BGGATE/cBGGATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/TEMPMINE/'$GEN_MIN_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	else
	    sed -i 's/TEMPMINE/0.0012/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	fi
	
	mv $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in $PWD/control.in
	
	if [[ "$GEANTVER" == "3" ]]; then
	    hdgeant 
	elif [[ "$GEANTVER" == "4" ]]; then
	    #make run.mac then call it below
	    rm -f run.mac
	    echo "/run/beamOn $EVT_TO_GEN" > run.mac
	    echo "exit" >>! run.mac
	    hdgeant4 -t$NUMTHREADS run.mac
	    rm run.mac
	else
	    echo "INVALID GEANT VERSION"
	    exit
	fi
	
	if [[ "$SMEAR" != "0" ]]; then
	    echo "RUNNING MCSMEAR"
	    
	    if [[ "$BKGFOLDSTR" == "BeamPhotons" || "$BKGFOLDSTR" == "None" ]]; then
		echo "running MCsmear without folding in random background"
		mcsmear -o$inputfile'_geant_smeared.hddm' $inputfile'_geant.hddm'
	    else
		if( "$BKGFOLDSTR" == "DEFAULT" ]]; then
		    #find file and run:1
		    echo "Finding the right file to fold in"
		    set runperiod="RunPeriod-2017-01"
		    
		    if [[ $RUN_NUMBER > 40000 ]]; then
			#set runperiod="RunPeriod-2017-10"
			echo
		    fi
		    
		    if [[ $RUN_NUMBER < 30000 ]]; then
			echo "Warning: random triggers did not exist by this point"
		    fi
		    set bkglocstring="/cache/halld/""$runperiod""/sim/random_triggers/""$formatted_runNumber"".hddm"
		    #set bkglocstring="/w/halld-scifs1a/home/tbritton/converted.hddm"
		    
		    if [[ ! -f $bkglocstring ]]; then
			echo "Could not find mix-in file "$bkglocstring
			exit
		    else
			echo "mcsmear -o$inputfile"\_"geant"\_"smeared.hddm $inputfile"\_"geant.hddm $bkglocstring"\:"1"
			mcsmear -o$inputfile\_geant\_smeared.hddm $inputfile\_geant.hddm $bkglocstring\:1
		    fi
		    
		else
		    #trust the user and use their string
		    echo 'mcsmear -o'$inputfile'_geant_smeared.hddm'' '$inputfile'_geant.hddm'' '$BKGFOLDSTR
		    mcsmear -o$inputfile'_geant_smeared.hddm' $inputfile'_geant.hddm' $BKGFOLDSTR
		fi
	    fi
	    #run reconstruction
	    if [[ "$CLEANGENR" == "1" ]]; then
		if [[ "$GENERATOR" == "genr8" ]]; then
		    rm *.ascii
		elif [[ "$GENERATOR" == "bggen" ]]; then
		    rm particle.dat
		    rm pythia.dat
		    rm pythia-geant.map
		    unlink fort.15
		fi
		
		rm $GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm
	    fi
	    
	    if [[ "$RECON" != "0" ]]; then
		echo "RUNNING RECONSTRUCTION"
		hd_root $inputfile'_geant_smeared.hddm' -PPLUGINS=danarest,monitoring_hists -PNTHREADS=$NUMTHREADS
		mv hd_root.root hd_root_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.root
		mv dana_rest.hddm dana_rest_$GEN_NAME\_$formatted_runNumber\_$formatted_fileNumber.hddm
		
		if [[ "$CLEANGEANT" == "1" ]]; then
		    rm *_geant.hddm
		    if [[ "$PWD" != "$MCWRAPPER_CENTRAL" ]]; then
			rm temp_Gcontrol.in	
		    fi
		fi
		
		if [[ "$CLEANSMEAR" == "1" ]]; then
		    rm *_smeared.hddm
		    rm smear.root
		fi
		
		if [[ "$CLEANRECON" == "1" ]]; then
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
