#!/bin/bash

# SET INPUTS
export BATCHRUN=$1
shift
export ENVIRONMENT=$1 
shift
if [[ "$BATCHRUN" != "0" ]]; then
source $ENVIRONMENT
fi
export CONFIG_FILE=$1
shift
export OUTDIR=$1
shift
export RUN_NUMBER=$1
shift
export FILE_NUMBER=$1
shift
export EVT_TO_GEN=$1
shift
export VERSION=$1
shift
export CALIBTIME=$1
wholecontext=$VERSION
if [[ "$CALIBTIME" != "notime" ]]; then
wholecontext="variation=$VERSION calibtime=$CALIBTIME"
else
wholecontext="variation=$VERSION"
fi
export JANA_CALIB_CONTEXT="$wholecontext"
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
export BATCHSYS=$1
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
shift
export TAGSTR=$1
shift
export CUSTOM_PLUGINS=$1
shift
export PER_FILE=$1
shift
export RUNNING_DIR=$1
shift
export SQLITEPATH=$1
shift
export BGTAGONLY_OPTION=$1
shift
export RADIATOR_THICKNESS=$1

echo ""
echo ""
echo "Detected bash shell"

radthick="50.e-6"
exponentialu=".e-6"

if [[ "$RADIATOR_THICKNESS" != "rcdb" || "$VERSION" != "mc" ]]; then
	radthick=$RADIATOR_THICKNESS
else
	words=`rcnd $RUN_NUMBER radiator_type | sed 's/ / /g' `
	for word in $words;
	do	
		if [[ "$word" != "number" ]]; then	
			removedum=`echo $word | sed 's/um/ /g'`
			if [[ $removedum != $word ]]; then
				radthick=`echo "$removedum.e-6" | tr -d '[:space:]'`
			fi
		fi
	done
fi

polarization_angle=`rcnd $RUN_NUMBER polarization_angle | awk '{print $1}'`
echo polarization angle: $polarization_angle

elecE=0
elecE_text=`rcnd $RUN_NUMBER beam_energy | awk '{print $1}'`

if [[ "$eBEAM_ENERGY" != "rcdb" || "$VERSION" != "mc" ]]; then
    elecE=$eBEAM_ENERGY
elif [[ $elecE_text == "Run" ]]; then
	elecE=12
elif [[ $elecE_text == "-1.0" ]]; then
	elecE=12 #Should never happen
else
	elecE=`echo "$elecE_text / 1000" | bc -l `
fi

copeak=0
copeak_text=`rcnd $RUN_NUMBER coherent_peak | awk '{print $1}'`

if [[ "$COHERENT_PEAK" != "rcdb" || "$VERSION" != "mc" ]]; then
    copeak=$COHERENT_PEAK
elif [[ $copeak_text == "Run" ]]; then
	copeak=9
elif [[ $copeak_text == "-1.0" ]]; then
	copeak=0.0 #for now add a smidge to the max electron e
else
	copeak=`echo "$copeak_text / 1000" | bc -l `
fi

#echo $copeak

#set copeak=`rcnd $RUN_NUMBER coherent_peak | awk '{print $1}' | sed 's/\.//g' #| awk -vFS="" -vOFS="" '{$1=$1"."}1' `

export COHERENT_PEAK=$copeak

if [[ "$VERSION" != "mc" && "$COHERENT_PEAK" == "rcdb" ]]; then
	echo "error in requesting rcdb for the coherent peak while not using variation=mc"
	exit 1
fi

export eBEAM_ENERGY=$elecE

if [[ "$VERSION" != "mc" && "$eBEAM_ENERGY" == "rcdb" ]]; then
	echo "error in requesting rcdb for the electron beam energy and not using variation=mc"
	exit 1
fi

if [[ "$polarization_angle" == "-1.0" ]]; then
	copeak=`echo "$eBEAM_ENERGY + .5" | bc`
	export COHERENT_PEAK=$copeak
fi

# PRINT INPUTS
echo "Job started: " `date`
echo "Producing file number: "$FILE_NUMBER
echo "Containing: " $EVT_TO_GEN"/""$PER_FILE"" events"
echo "Running location:" $RUNNING_DIR
echo "Output location: "$OUTDIR
echo "Environment file: " $ENVIRONMENT
echo "Context: "$JANA_CALIB_CONTEXT
echo "Run Number: "$RUN_NUMBER
echo "Electron beam energy to use: "$eBEAM_ENERGY" GeV"
echo "Radiator Thickness to use: "$radthick" m"
echo "Photon Energy between "$GEN_MIN_ENERGY" and "$GEN_MAX_ENERGY" GeV"
echo "Coherent Peak position: "$COHERENT_PEAK
echo "----------------------------------------------"
echo "Run generation step? "$GENR"  Will be cleaned?" $CLEANGENR
echo "Using "$GENERATOR"  with config: "$CONFIG_FILE
echo "----------------------------------------------"
echo "Run geant step? "$GEANT"  Will be cleaned?" $CLEANGEANT
echo "Using geant"$GEANTVER
echo "Background to use: "$BKGFOLDSTR
echo "Run mcsmear ? "$SMEAR"  Will be cleaned?" $CLEANSMEAR
echo "----------------------------------------------"
echo "Run reconstruction? "$RECON"  Will be cleaned?" $CLEANRECON
echo "With additional plugins: "$CUSTOM_PLUGINS
echo "=============================================="
echo ""
echo ""

if [[ ! -d $RUNNING_DIR ]]; then
mkdir $RUNNING_DIR
fi

cd $RUNNING_DIR

if [[ ! -d $RUNNING_DIR/${RUN_NUMBER}_${FILE_NUMBER} ]]; then
mkdir $RUNNING_DIR/${RUN_NUMBER}_${FILE_NUMBER}
fi

cd $RUNNING_DIR/${RUN_NUMBER}_${FILE_NUMBER}

#printenv
#necessary to run swif, uses local directory if swif=0 is used
if [[ "$BATCHRUN" != "0" ]]; then
    # ENVIRONMENT
    echo $ENVIRONMENT
	if [[ "$BATCHSYS" == "QSUB" ]]; then
		cd $RUNNING_DIR
	fi
    
    if [[ "$SQLITEPATH" != "no_sqlite" ]]; then
        cp $SQLITEPATH .
        export CCDB_CONNECTION sqlite:///$RUNNING_DIR/ccdb.sqlite
        export JANA_CALIB_URL $CCDB_CONNECTION
    fi
    echo pwd=$PWD
    mkdir -p $OUTDIR
    mkdir -p $OUTDIR/log
fi

current_files=`find . -maxdepth 1 -type f`

if [[ "$CUSTOM_GCONTROL" == "0" ]]; then
    cp $MCWRAPPER_CENTRAL/Gcontrol.in ./temp_Gcontrol.in
    chmod 777 ./temp_Gcontrol.in
else
    cp $CUSTOM_GCONTROL ./temp_Gcontrol.in
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

custom_tag=""

if [[ "$TAGSTR" != "I_dont_have_one" ]]; then
    custom_tag=$TAGSTR\_
fi

STANDARD_NAME=$custom_tag$formatted_runNumber\_$formatted_fileNumber

colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
if [[ "$colsize" == "B" || "$colsize" == "R" || "$JANA_CALIB_CONTEXT" != "variation=mc" ]]; then
    colsize="50"
fi


if [[ `echo $eBEAM_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
    eBEAM_ENERGY=$eBEAM_ENERGY\.
fi
if [[ `echo $COHERENT_PEAK | grep -o "\." | wc -l` == 0 ]]; then
    COHERENT_PEAK=$COHERENT_PEAK\.
fi
if [[ `echo $GEN_MIN_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
    GEN_MIN_ENERGY=$GEN_MIN_ENERGY\.
fi
if [[ `echo $GEN_MAX_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
    GEN_MAX_ENERGY=$GEN_MAX_ENERGY\.
fi

if [[ ! -d "$OUTDIR" ]]; then
    mkdir $OUTDIR
fi
if [[ ! -d "$OUTDIR/configurations/" ]]; then
    mkdir $OUTDIR/configurations/
fi
if [[ ! -d "$OUTDIR/configurations/generation/" ]]; then
    mkdir $OUTDIR/configurations/generation/
fi
if [[ ! -d "$OUTDIR/configurations/geant/" ]]; then
    mkdir $OUTDIR/configurations/geant/
fi
if [[ ! -d "$OUTDIR/hddm/" ]]; then
    mkdir $OUTDIR/hddm/
fi
if [[ ! -d "$OUTDIR/root/" ]]; then
    mkdir $OUTDIR/root/
fi

bkglocstring=""
bkgloc_pre=`echo $BKGFOLDSTR | cut -c 1-4`
if [[ "$BKGFOLDSTR" == "DEFAULT" || "$bkgloc_pre" == "loc:" || "$BKGFOLDSTR" == "Random" ]]; then
		    #find file and run:1
		    echo "Finding the right file to fold in during MCsmear step"
		    runperiod="RunPeriod-2017-01"

		    if [[ $RUN_NUMBER > 40000 ]]; then
			echo
			#set runperiod="RunPeriod-2017-10"
		    fi

		    if [[ $RUN_NUMBER < 30000 ]]; then
			echo "Warning: random triggers do not exist for this run"
			exit
		    fi

			if [[ "$bkgloc_pre" == "loc:" ]]; then
			rand_bkg_loc=`echo $BKGFOLDSTR | cut -c 5-`
 		   	bkglocstring=$rand_bkg_loc"/run$formatted_runNumber""_random.hddm"
			else
		    bkglocstring="/cache/halld/""$runperiod""/sim/random_triggers/""run$formatted_runNumber""_random.hddm"
		    fi
			#set bkglocstring="/w/halld-scifs1a/home/tbritton/converted.hddm"
		    
		    if [[ ! -f $bkglocstring ]]; then
			echo "Could not find mix-in file "$bkglocstring
			exit
		    fi
fi


recon_pre=`echo $CUSTOM_PLUGINS | cut -c1-4`
jana_config_file=`echo $CUSTOM_PLUGINS | sed -r 's/^.{5}//'`

if [[ -f $jana_config_file ]]; then
echo "gathering jana config file"
cp $jana_config_file ./jana_config.cfg
fi

gen_pre=""

if [[ "$GENR" != "0" ]]; then
	gen_pre=`echo $GENERATOR | cut -c1-4`
    if [[ "$gen_pre" != "file" && "$GENERATOR" != "genr8" && "$GENERATOR" != "bggen" && "$GENERATOR" != "genEtaRegge" && "$GENERATOR" != "gen_2pi_amp" && "$GENERATOR" != "gen_pi0" && "$GENERATOR" != "gen_2pi_primakoff" && "$GENERATOR" != "gen_omega_3pi" && "$GENERATOR" != "gen_2k" ]]; then
	echo "NO VALID GENERATOR GIVEN"
	echo "only [genr8, bggen, genEtaRegge, gen_2pi_amp, gen_pi0, gen_omega_3pi, gen_2k] are supported"
	exit
    fi
    
	if [[ "$gen_pre" == "file" ]]; then
		gen_in_file=`echo $GENERATOR | sed -r 's/^.{5}//'`
		echo "bypassing generation"
		if [[ -f $gen_in_file ]]; then
			echo "using pre-generated file: "$gen_in_file
			cp $gen_in_file ./$STANDARD_NAME.hddm
		else
			echo "cannot find file: "$gen_in_file
			exit
		fi
				
	else 
		if [[ -f $CONFIG_FILE ]]; then
	    	echo "input file found"
		else
	    	echo $CONFIG_FILE" does not exist"
	    	exit
    	fi

	fi
    
    if [[ "$GENERATOR" == "genr8" ]]; then
	echo "configuring genr8"
	STANDARD_NAME="genr8_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    elif [[ "$GENERATOR" == "bggen" ]]; then
	echo "configuring bggen"
	STANDARD_NAME="bggen_"$STANDARD_NAME
	cp $MCWRAPPER_CENTRAL/Generators/bggen/particle.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia-geant.map ./
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
	
    elif [[ "$GENERATOR" == "genEtaRegge" ]]; then
	echo "configuring genEtaRegge"
	STANDARD_NAME="genEtaRegge_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    elif [[ "$GENERATOR" == "gen_2pi_amp" ]]; then
	echo "configuring gen_2pi_amp"
	STANDARD_NAME="gen_2pi_amp_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
	elif [[ "$GENERATOR" == "gen_omega_3pi" ]]; then
	echo "configuring gen_omega_3pi"
	STANDARD_NAME="gen_omega_3pi_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    elif [[ "$GENERATOR" == "gen_2pi_primakoff" ]]; then
	echo "configuring gen_2pi_primakoff"
	STANDARD_NAME="gen_2pi_primakoff_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    elif [[ "$GENERATOR" == "gen_pi0" ]]; then
	echo "configuring gen_pi0"
	STANDARD_NAME="genr_pi0_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
	elif [[ "$GENERATOR" == "gen_2k" ]]; then
	echo "configuring gen_2k"
	set STANDARD_NAME="gen_2k_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    fi
	
	if [[ "$gen_pre" != "file" ]]; then
    config_file_name=`basename "$CONFIG_FILE"`
    echo $config_file_name
    fi

    if [[ "$GENERATOR" == "genr8" ]]; then
	echo "RUNNING GENR8"
	RUNNUM=$formatted_runNumber+$formatted_fileNumber
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' $STANDARD_NAME.conf
	# RUN genr8 and convert
	genr8 -r$formatted_runNumber -M$EVT_TO_GEN -A$STANDARD_NAME.ascii < $STANDARD_NAME.conf #$config_file_name
	genr8_2_hddm $STANDARD_NAME.ascii
    elif [[ "$GENERATOR" == "bggen" ]]; then
	RANDOMnum=`bash -c 'echo $RANDOM'`
	echo $RANDOMnum
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' $STANDARD_NAME.conf
	sed -i 's/TEMPRUNNO/'$RUN_NUMBER'/' $STANDARD_NAME.conf
	sed -i 's/TEMPCOLD/'0.00$colsize'/' $STANDARD_NAME.conf
	sed -i 's/TEMPRAND/'$RANDOMnum'/' $STANDARD_NAME.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' $STANDARD_NAME.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' $STANDARD_NAME.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' $STANDARD_NAME.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' $STANDARD_NAME.conf
	
	ln -s $STANDARD_NAME.conf fort.15
	bggen
	mv bggen.hddm $STANDARD_NAME.hddm
    elif [[ "$GENERATOR" == "genEtaRegge" ]]; then
	echo "RUNNING GENETAREGGE" 
	
	sed -i 's/TEMPCOLD/'0.00$colsize'/' $STANDARD_NAME.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' $STANDARD_NAME.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' $STANDARD_NAME.conf
	sed -i 's/TEMPRADTHICK/'"$radthick"'/' $STANDARD_NAME.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' $STANDARD_NAME.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' $STANDARD_NAME.conf
	genEtaRegge -N$EVT_TO_GEN -O$STANDARD_NAME.hddm -I$STANDARD_NAME.conf
    elif [[ "$GENERATOR" == "gen_2pi_amp" ]]; then
	echo "RUNNING GEN_2PI_AMP" 
    optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	#RANDOMnum=`bash -c 'echo $RANDOM'`
	echo gen_2pi_amp -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK -m $eBEAM_ENERGY $optionals_line
	gen_2pi_amp -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK -m $eBEAM_ENERGY $optionals_line
	elif [[ "$GENERATOR" == "gen_omega_3pi" ]]; then
	echo "RUNNING GEN_OMEGA_3PI" 
        optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	echo gen_omega_3pi -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -m $eBEAM_ENERGY $optionals_line
	gen_omega_3pi -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -m $eBEAM_ENERGY $optionals_line
    elif [[ "$GENERATOR" == "gen_2pi_primakoff" ]]; then
	echo "RUNNING GEN_2PI_PRIMAKOFF" 
        optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	echo gen_2pi_primakoff -c $STANDARD_NAME.conf -o  $STANDARD_NAME.hddm -hd  $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK -m $eBEAM_ENERGY $optionals_line
	gen_2pi_primakoff -c $STANDARD_NAME.conf -hd  $STANDARD_NAME.hddm -o  $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK -m $eBEAM_ENERGY $optionals_line
    elif [[ "$GENERATOR" == "gen_pi0" ]]; then
	echo "RUNNING GEN_PI0" 
        optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	gen_pi0 -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK  -s $formatted_fileNumber -m $eBEAM_ENERGY $optionals_line
    elif [[ "$GENERATOR" == "gen_2k" ]]; then
	echo "RUNNING GEN_2K" 
    set optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	#set RANDOMnum=`bash -c 'echo $RANDOM'`
	echo $optionals_line
	echo gen_2k -c $STANDARD_NAME.conf -o $STANDARD_NAME.hddm -hd $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK -m $eBEAM_ENERGY $optionals_line
	gen_2k -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK -m $eBEAM_ENERGY $optionals_line
	fi
    
    #GEANT/smearing
    
    if [[ "$GEANT" != "0" ]]; then
	echo "RUNNING GEANT"$GEANTVER
	
	if [[ `echo $eBEAM_ENERGY | grep -o "\." | wc -l` == 0 ]]; then
	    eBEAM_ENERGY=$eBEAM_ENERGY\.
	fi
	if [[ `echo $COHERENT_PEAK | grep -o "\." | wc -l` == 0 ]]; then
	    COHERENT_PEAK=$COHERENT_PEAK\.
	fi
	
	cp temp_Gcontrol.in $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	chmod 777 $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPIN/'$STANDARD_NAME.hddm'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPRUNG/'$RUN_NUMBER'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPOUT/'$STANDARD_NAME'_geant'$GEANTVER'.hddm/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPCOLD/'0.00$colsize'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPRADTHICK/'"$radthick"'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPBGTAGONLY/'$BGTAGONLY_OPTION'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in

	if [[ "$gen_pre" == "file" ]]; then
			skip_num=$((FILE_NUMBER * PER_FILE))
            sed -i 's/TEMPSKIP/'$skip_num'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
        else
	    sed -i 's/TEMPSKIP/'0'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
        fi
	
	if [[ "$BKGFOLDSTR" == "None" ]]; then
	    echo "removing Beam Photon background from geant simulation"
	    sed -i 's/BGRATE/cBGRATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/BGGATE/cBGGATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/TEMPMINE/'$GEN_MIN_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	elif [[ "$BKGFOLDSTR" == "BeamPhotons" ]]; then
		sed -i 's/TEMPMINE/0.0012/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	elif [[ "$BKGFOLDSTR" == "DEFAULT" || "$BKGFOLDSTR" == "Random" && "$BGTAGONLY_OPTION" == "0" ]]; then
	    sed -i 's/BGRATE/cBGRATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/BGGATE/cBGGATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/TEMPMINE/'$GEN_MIN_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	else
	    sed -i 's/TEMPMINE/'$GEN_MIN_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	fi
	
	cp $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in $OUTDIR/configurations/geant/
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
		mcsmear -PTHREAD_TIMEOUT=300 -o$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' $STANDARD_NAME'_geant'$GEANTVER'.hddm'
	    elif [[ "$BKGFOLDSTR" == "DEFAULT" || "$BKGFOLDSTR" == "Random" ]]; then
		echo "mcsmear -PTHREAD_TIMEOUT=300 -o$STANDARD_NAME"\_"geant$GEANTVER"\_"smeared.hddm $STANDARD_NAME"\_"geant$GEANTVER.hddm $bkglocstring"\:"1"
		mcsmear -PTHREAD_TIMEOUT=300 -o$STANDARD_NAME\_geant$GEANTVER\_smeared.hddm $STANDARD_NAME\_geant$GEANTVER.hddm $bkglocstring\:1
		elif [[ "$bkgloc_pre" == "loc:" ]]; then
		echo "mcsmear -PTHREAD_TIMEOUT=300 -o$STANDARD_NAME"\_"geant$GEANTVER"\_"smeared.hddm $STANDARD_NAME"\_"geant$GEANTVER.hddm $bkglocstring"\:"1"
		mcsmear -PTHREAD_TIMEOUT=300 -o$STANDARD_NAME\_geant$GEANTVER\_smeared.hddm $STANDARD_NAME\_geant$GEANTVER.hddm $bkglocstring\:1
		else
		    #trust the user and use their string
		    echo 'mcsmear -PTHREAD_TIMEOUT=300 -o'$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm'' '$STANDARD_NAME'_geant'$GEANTVER'.hddm'' '$BKGFOLDSTR
		    mcsmear -PTHREAD_TIMEOUT=300 -o$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' $STANDARD_NAME'_geant'$GEANTVER'.hddm' $BKGFOLDSTR

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
		
		rm $STANDARD_NAME.hddm
	    fi
	    
	    if [[ "$RECON" != "0" ]]; then
		echo "RUNNING RECONSTRUCTION"

		if [[ "$recon_pre" == "file" ]]; then
			echo "using config file: "$jana_config_file
			hd_root ./$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' --config=jana_config.cfg -PNTHREADS=$NUMTHREADS
			rm jana_config.cfg
		else
		
			declare -a pluginlist=("danarest" "monitoring_hists")
			echo ${pluginlist[@]}
			echo $CUSTOM_PLUGINS
            if [[ "$CUSTOM_PLUGINS" != "None" ]]; then
				pluginlist=("${pluginlist[@]}" $CUSTOM_PLUGINS)
            fi
			echo ${pluginlist[@]}

			PluginStr=""
		
            for plugin in "${pluginlist[@]}"; do
				PluginStr="$PluginStr""$plugin"","
            done
		
			PluginStr=`echo $PluginStr | sed -r 's/.{1}$//'`
            echo "Running hd_root with:""$PluginStr"
			echo "hd_root ""$STANDARD_NAME"'_geant'"$GEANTVER"'_smeared.hddm'" -PPLUGINS=""$PluginStr ""-PNTHREADS=""$NUMTHREADS"
			hd_root ./$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' -PPLUGINS=$PluginStr -PNTHREADS=$NUMTHREADS
		fi

		if [[ -f dana_rest.hddm ]]; then
                    mv dana_rest.hddm dana_rest_$STANDARD_NAME.hddm
        fi


		if [[ "$CLEANGEANT" == "1" ]]; then
		    rm $STANDARD_NAME'_geant'$GEANTVER'.hddm'
		    rm control.in
		    rm -f geant.hbook
		    rm -f hdgeant.rz
		    if [[ "$PWD" != "$MCWRAPPER_CENTRAL" ]]; then
			rm temp_Gcontrol.in	
		    fi
		    
		fi
		
		if [[ "$CLEANSMEAR" == "1" ]]; then
		    rm $STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm'
		    rm smear.root
		fi
		
		if [[ "$CLEANRECON" == "1" ]]; then
		    rm dana_rest*
		fi
		
		rootfiles=$(ls *.root)
		filename_root=""
		for rootfile in $rootfiles; do
		    filename_root=`echo $rootfile | sed -r 's/.{5}$//'`
		    filetomv="$rootfile"
			filecheck=`echo $current_files | grep -c $filetomv`
			
			if [[ "$filecheck" == "0" ]]; then
			    mv $filetomv $filename_root\_$STANDARD_NAME.root
			    mv $PWD/$filename_root\_$STANDARD_NAME.root $OUTDIR/root/
			fi
		done
	    fi
	fi
  fi
fi
if [[ "$gen_pre" != "file" ]]; then
	mv $PWD/*.conf $OUTDIR/configurations/generation/
fi
hddmfiles=$(ls | grep .hddm)
if [[ "$hddmfiles" != "" ]]; then
	for hddmfile in $hddmfiles; do
		filetomv="$hddmfile" 
		filecheck=`echo $current_files | grep -c $filetomv`
		if [[ "$filecheck" == "0" ]]; then
    		mv $hddmfile $OUTDIR/hddm/
		fi
	done
fi

if [[ `ls $RUNNING_DIR/${RUN_NUMBER}_${FILE_NUMBER} | wc -l` == 0 ]]; then
	rm -rf $RUNNING_DIR/${RUN_NUMBER}_${FILE_NUMBER}
else
	echo "MOVING AND/OR CLEANUP FAILED"
	echo `ls $RUNNING_DIR/${RUN_NUMBER}_${FILE_NUMBER}`
fi

#mv $PWD/*.root $OUTDIR/root/ #just in case
