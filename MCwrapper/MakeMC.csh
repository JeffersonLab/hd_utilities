#!/bin/csh -f
    
# SET INPUTS
setenv ENVIRONMENT $1 
shift
setenv CONFIG_FILE $1
shift
setenv OUTDIR $1
shift
setenv RUN_NUMBER $1
shift
setenv FILE_NUMBER $1
shift
setenv EVT_TO_GEN $1
shift
setenv JANA_CALIB_CONTEXT "variation="$1
shift
setenv GENR $1
shift
setenv GEANT $1
shift
setenv SMEAR $1
shift
setenv RECON $1
shift
setenv CLEANGENR $1
shift
setenv CLEANGEANT $1
shift
setenv CLEANSMEAR $1
shift
setenv CLEANRECON $1
shift
setenv BATCHSYS $1
shift
setenv BATCHRUN $1
shift
setenv NUMTHREADS $1
shift
setenv GENERATOR $1
shift
setenv GEANTVER $1
shift
setenv BKGFOLDSTR $1
shift
setenv CUSTOM_GCONTROL $1
shift
setenv eBEAM_ENERGY $1
shift
setenv COHERENT_PEAK $1
shift
setenv GEN_MIN_ENERGY $1
shift
setenv GEN_MAX_ENERGY $1
shift
setenv TAGSTR $1
shift
setenv CUSTOM_PLUGINS $1
shift
setenv PER_FILE $1

# PRINT INPUTS
echo `date`
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

echo "detected c-shell"

#necessary to run swif, uses local directory if swif=0 is used
if ( "$BATCHRUN" != "0" ) then
# ENVIRONMENT
    echo $ENVIRONMENT
    source $ENVIRONMENT
    echo pwd=$PWD
    mkdir -p $OUTDIR
    mkdir -p $OUTDIR/log
endif

set current_files=`find . -maxdepth 1 -type f`

if ( "$CUSTOM_GCONTROL" == "0" ) then
    cp $MCWRAPPER_CENTRAL/Gcontrol.in ./temp_Gcontrol.in
    chmod 777 ./temp_Gcontrol.in
else
    cp $CUSTOM_GCONTROL ./temp_Gcontrol.in
endif

@ length_count=`echo $RUN_NUMBER | wc -c` - 1

set formatted_runNumber=""
while ( $length_count < 6 )
    set formatted_runNumber="0""$formatted_runNumber"
    @ length_count=$length_count + 1
end
set formatted_runNumber=$formatted_runNumber$RUN_NUMBER

@ flength_count=`echo $FILE_NUMBER | wc -c` - 1

set formatted_fileNumber=""
while ( $flength_count < 3 )
    set formatted_fileNumber="0""$formatted_fileNumber"
    @ flength_count=$flength_count + 1
end
set formatted_fileNumber=$formatted_fileNumber$FILE_NUMBER

set custom_tag=""

if ( "$TAGSTR" != "I_dont_have_one" ) then
    set custom_tag=$TAGSTR\_
endif

set STANDARD_NAME=$custom_tag$formatted_runNumber\_$formatted_fileNumber

set colsize=`rcnd $RUN_NUMBER collimator_diameter | awk '{print $1}' | sed -r 's/.{2}$//' | sed -e 's/\.//g'`
if ( "$colsize" == "B" || "$colsize" == "R" || "$JANA_CALIB_CONTEXT" != "variation=mc" ) then
	set colsize="50"
endif

if ( `echo $eBEAM_ENERGY | grep -o "\." | wc -l` == 0 ) then
    set eBEAM_ENERGY=$eBEAM_ENERGY\.
endif
if ( `echo $COHERENT_PEAK | grep -o "\." | wc -l` == 0 ) then
    set COHERENT_PEAK=$COHERENT_PEAK\.
endif
if ( `echo $GEN_MIN_ENERGY | grep -o "\." | wc -l` == 0 ) then
    set GEN_MIN_ENERGY=$GEN_MIN_ENERGY\.
endif
if ( `echo $GEN_MAX_ENERGY | grep -o "\." | wc -l` == 0 ) then
    set GEN_MAX_ENERGY=$GEN_MAX_ENERGY\.
endif

if ( ! -d "$OUTDIR" ) then
    mkdir $OUTDIR
endif
if ( ! -d "$OUTDIR/configurations/" ) then
    mkdir $OUTDIR/configurations/
endif
if ( ! -d "$OUTDIR/configurations/generation/" ) then
    mkdir $OUTDIR/configurations/generation/
endif
if ( ! -d "$OUTDIR/configurations/geant/" ) then
    mkdir $OUTDIR/configurations/geant/
endif
if ( ! -d "$OUTDIR/hddm/" ) then
    mkdir $OUTDIR/hddm/
endif
if ( ! -d "$OUTDIR/root/" ) then
    mkdir $OUTDIR/root/
endif

set bkglocstring=""
set bkgloc_pre=`echo $BKGFOLDSTR | cut -c 1-4`

if ( "$BKGFOLDSTR" == "DEFAULT" || "$bkgloc_pre" == "loc:" ) then
   #find file and run:1
    echo "Finding the right file to fold in during MCsmear step"
    set runperiod="RunPeriod-2017-01"

    if ( $RUN_NUMBER > 40000 ) then
	echo
	#set runperiod="RunPeriod-2017-10"
    endif

    if ( $RUN_NUMBER < 30000 ) then
	echo "Warning: random triggers did not exist by this point"
    endif
	
	if ( "$bkgloc_pre" == "loc:" ) then
	set rand_bkg_loc=`echo $BKGFOLDSTR | cut -c 5-`
    set bkglocstring=$rand_bkg_loc"/run$formatted_runNumber""_random.hddm"
	else
    set bkglocstring="/cache/halld/""$runperiod""/sim/random_triggers/""run$formatted_runNumber""_random.hddm"
    #set bkglocstring="/w/halld-scifs1a/home/tbritton/converted.hddm"
	endif

    if ( ! -f $bkglocstring ) then
	echo "Could not find mix-in file "$bkglocstring
	exit
    endif
endif


set recon_pre=`echo $CUSTOM_PLUGINS | cut -c1-4`
set jana_config_file=`echo $CUSTOM_PLUGINS | sed -r 's/^.{5}//'`

if ( -f $jana_config_file ) then
    cp $jana_config_file ./jana_config.cfg
endif

set gen_pre=""

if ( "$GENR" != "0" ) then
    set gen_pre=`echo $GENERATOR | cut -c1-4`
    if ( "$gen_pre" != "file" && "$GENERATOR" != "genr8" && "$GENERATOR" != "bggen" && "$GENERATOR" != "genEtaRegge" && "$GENERATOR" != "gen_2pi_amp" && "$GENERATOR" != "gen_pi0" && "$GENERATOR" != "gen_2pi_primakoff" && "$GENERATOR" != "gen_omega_3pi" ) then
	echo "NO VALID GENERATOR GIVEN"
	echo "only [genr8, bggen, genEtaRegge, gen_2pi_amp, gen_pi0, gen_omega_3pi] are supported"
	exit
    endif

    if ( "$gen_pre" == "file" ) then
	set gen_in_file=`echo $GENERATOR | sed -r 's/^.{5}//'`
	echo "bypassing generation"
	if ( -f $gen_in_file ) then
	    echo "using pre-generated file: "$gen_in_file
	    cp $gen_in_file ./$STANDARD_NAME.hddm
	else
	    echo "cannot find file: "$gen_in_file
	    exit
	endif
				
    else 
	if ( -f $CONFIG_FILE ) then
	    echo "input file found"
	else
	    echo $CONFIG_FILE" does not exist"
	    exit
    	endif
	
    endif

    if ( "$GENERATOR" == "genr8" ) then
	echo "configuring genr8"
	set STANDARD_NAME="genr8_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    else if ( "$GENERATOR" == "bggen" ) then
	echo "configuring bggen"
	set STANDARD_NAME="bggen_"$STANDARD_NAME
	cp $MCWRAPPER_CENTRAL/Generators/bggen/particle.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia.dat ./
	cp $MCWRAPPER_CENTRAL/Generators/bggen/pythia-geant.map ./
	cp $CONFIG_FILE ./$STANDARD_NAME.conf	
    else if ( "$GENERATOR" == "genEtaRegge" ) then
	echo "configuring genEtaRegge"
	set STANDARD_NAME="genEtaRegge_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    else if ( "$GENERATOR" == "gen_2pi_amp" ) then
	echo "configuring gen_2pi_amp"
	set STANDARD_NAME="gen_2pi_amp_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
	else if ( "$GENERATOR" == "gen_omega_3pi" ) then
	echo "configuring gen_omega_3pi"
	set STANDARD_NAME="gen_omega_3pi_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    else if ( "$GENERATOR" == "gen_2pi_primakoff" ) then
	echo "configuring gen_2pi_primakoff"
	set STANDARD_NAME="gen_2pi_primakoff_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    else if ( "$GENERATOR" == "gen_pi0" ) then
	echo "configuring gen_pi0"
	set STANDARD_NAME="genr_pi0_"$STANDARD_NAME
	cp $CONFIG_FILE ./$STANDARD_NAME.conf
    endif

    if ( "$gen_pre" != "file" ) then
	set config_file_name=`basename "$CONFIG_FILE"`
	echo $config_file_name
    endif
    
    if ( "$GENERATOR" == "genr8" ) then
	echo "RUNNING GENR8"
	set RUNNUM=$formatted_runNumber+$formatted_fileNumber
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' $STANDARD_NAME.conf
	# RUN genr8 and convert
	genr8 -r$formatted_runNumber -M$EVT_TO_GEN -A$STANDARD_NAME.ascii < $STANDARD_NAME.conf #$config_file_name
	genr8_2_hddm $STANDARD_NAME.ascii
    else if ( "$GENERATOR" == "bggen" ) then
	set RANDOMnum=`bash -c 'echo $RANDOM'`
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
    else if ( "$GENERATOR" == "genEtaRegge" ) then
	echo "RUNNING GENETAREGGE" 	
	sed -i 's/TEMPCOLD/'0.00$colsize'/' $STANDARD_NAME.conf
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' $STANDARD_NAME.conf
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' $STANDARD_NAME.conf
	sed -i 's/TEMPMINGENE/'$GEN_MIN_ENERGY'/' $STANDARD_NAME.conf
	sed -i 's/TEMPMAXGENE/'$GEN_MAX_ENERGY'/' $STANDARD_NAME.conf
	genEtaRegge -N$EVT_TO_GEN -O$STANDARD_NAME.hddm -I$STANDARD_NAME.conf
    else if ( "$GENERATOR" == "gen_2pi_amp" ) then
	echo "RUNNING GEN_2PI_AMP" 
    set optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	#set RANDOMnum=`bash -c 'echo $RANDOM'`
	echo $optionals_line
	echo gen_2pi_amp -c $STANDARD_NAME.conf -o $STANDARD_NAME.hddm -hd $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_2pi_amp -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	else if ( "$GENERATOR" == "gen_omega_3pi" ) then
	echo "RUNNING GEN_OMEGA_3PI" 
    set optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	echo gen_omega_3pi -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_omega_3pi -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
    else if ( "$GENERATOR" == "gen_2pi_primakoff" ) then
	echo "RUNNING GEN_2PI_PRIMAKOFF" 
        set optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	echo gen_2pi_primakoff -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER  -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
	gen_2pi_primakoff -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY $optionals_line
    else if ( "$GENERATOR" == "gen_pi0" ) then
	echo "RUNNING GEN_PI0" 
        set optionals_line=`head -n 1 $STANDARD_NAME.conf | sed -r 's/.//'`
	echo $optionals_line
	gen_pi0 -c $STANDARD_NAME.conf -hd $STANDARD_NAME.hddm -o $STANDARD_NAME.root -n $EVT_TO_GEN -r $RUN_NUMBER -a $GEN_MIN_ENERGY -b $GEN_MAX_ENERGY -p $COHERENT_PEAK  -s $formatted_fileNumber $optionals_line -m $eBEAM_ENERGY
    endif

   set RETURN_CODE=$?
   #echo "Return Code = " $RETURN_CODE

#GEANT/smearing

    if ( "$GEANT" != "0" ) then
	echo "RUNNING GEANT"$GEANTVER

	if ( `echo $eBEAM_ENERGY | grep -o "\." | wc -l` == 0 ) then
	    set eBEAM_ENERGY=$eBEAM_ENERGY\.
	endif
	
	if ( `echo $COHERENT_PEAK | grep -o "\." | wc -l` == 0 ) then
	    set COHERENT_PEAK=$COHERENT_PEAK\.
	endif

	cp temp_Gcontrol.in $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	chmod 777 $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPELECE/'$eBEAM_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPCOHERENT/'$COHERENT_PEAK'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPIN/'$STANDARD_NAME.hddm'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPRUNG/'$RUN_NUMBER'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPOUT/'$STANDARD_NAME'_geant'$GEANTVER'.hddm/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPTRIG/'$EVT_TO_GEN'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	sed -i 's/TEMPCOLD/'0.00$colsize'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in

	if ( "$gen_pre" == "file" ) then
		@ skip_num = $FILE_NUMBER * $PER_FILE
	    sed -i 's/TEMPSKIP/'$skip_num'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	else
	    sed -i 's/TEMPSKIP/'0'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	endif

	if ( "$BKGFOLDSTR" != "BeamPhotons" ) then
	    echo "removing Beam Photon background from geant simulation"
	    sed -i 's/BGRATE/cBGRATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/BGGATE/cBGGATE/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	    sed -i 's/TEMPMINE/'$GEN_MIN_ENERGY'/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	else
	    sed -i 's/TEMPMINE/0.0012/' control'_'$formatted_runNumber'_'$formatted_fileNumber.in
	endif

	cp $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in $OUTDIR/configurations/geant/

	mv $PWD/control'_'$formatted_runNumber'_'$formatted_fileNumber.in $PWD/control.in
	
	if ( "$GEANTVER" == "3" ) then
	    hdgeant 
	else if ( "$GEANTVER" == "4" ) then
	    #make run.mac then call it below
	    rm -f run.mac
	    echo "/run/beamOn $EVT_TO_GEN" > run.mac
	    echo "exit" >>! run.mac
	    hdgeant4 -t$NUMTHREADS run.mac
	    rm run.mac
	else
	    echo "INVALID GEANT VERSION"
	    exit
	endif
	set RETURN_CODE=$?
	#echo "Return Code = " $RETURN_CODE
	
	if ( "$SMEAR" != "0" ) then
	    echo "RUNNING MCSMEAR"
	    
	    if ( "$BKGFOLDSTR" == "BeamPhotons" || "$BKGFOLDSTR" == "None" ) then
		echo "running MCsmear without folding in random background"
		mcsmear -o$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' $STANDARD_NAME'_geant'$GEANTVER'.hddm'
	    else if ( "$BKGFOLDSTR" == "DEFAULT" ) then
		echo "mcsmear -o$STANDARD_NAME"\_"geant$GEANTVER"\_"smeared.hddm $STANDARD_NAME"\_"geant$GEANTVER.hddm $bkglocstring"\:"1"
		mcsmear -o$STANDARD_NAME\_geant$GEANTVER\_smeared.hddm $STANDARD_NAME\_geant$GEANTVER.hddm $bkglocstring\:1
		else if ( "$bkgloc_pre" == "loc:" ) then
		echo "mcsmear -o$STANDARD_NAME"\_"geant$GEANTVER"\_"smeared.hddm $STANDARD_NAME"\_"geant$GEANTVER.hddm $bkglocstring"\:"1"
		mcsmear -o$STANDARD_NAME\_geant$GEANTVER\_smeared.hddm $STANDARD_NAME\_geant$GEANTVER.hddm $bkglocstring\:1
	    else
		#trust the user and use their string
		echo 'mcsmear -o'$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm'' '$STANDARD_NAME'_geant'$GEANTVER'.hddm'' '$BKGFOLDSTR
		mcsmear -o$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' $STANDARD_NAME'_geant'$GEANTVER'.hddm' $BKGFOLDSTR
	    endif
	
	    #run reconstruction
	    if ( "$CLEANGENR" == "1" ) then
		if ( "$GENERATOR" == "genr8" ) then
		    rm *.ascii
		else if ( "$GENERATOR" == "bggen" ) then
		    rm particle.dat
		    rm pythia.dat
		    rm pythia-geant.map
		    unlink fort.15
		endif		
		rm $STANDARD_NAME.hddm
	    endif
	    
	    set RETURN_CODE=$?
	    #echo "Return Code = " $RETURN_CODE
    
	    if ( "$RECON" != "0" ) then
		echo "RUNNING RECONSTRUCTION"
		
		if ( "$recon_pre" == "file" ) then
		    echo "using config file: "$jana_config_file
		    hd_root ./$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' --config=jana_config.cfg -PNTHREADS=$NUMTHREADS
			rm jana_config.cfg
		else
		    set pluginlist=("danarest" "monitoring_hists")
	     
		    if ( "$CUSTOM_PLUGINS" != "None" ) then
			set pluginlist=( "$pluginlist" "$CUSTOM_PLUGINS" )
		    endif	
		    set PluginStr=""
	       
		    foreach plugin ($pluginlist)
			set PluginStr="$PluginStr""$plugin"","
		    end
		
		    set PluginStr=`echo $PluginStr | sed -r 's/.{1}$//'`
		    echo "Running hd_root with:""$PluginStr"
		    echo "hd_root ""$STANDARD_NAME"'_geant'"$GEANTVER"'_smeared.hddm'" -PPLUGINS=""$PluginStr ""-PNTHREADS=""$NUMTHREADS"
		    hd_root ./$STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm' -PPLUGINS=$PluginStr -PNTHREADS=$NUMTHREADS
		    
		endif
		
		set RETURN_CODE=$?
		#echo "Return Code = " $RETURN_CODE
		if ( -f dana_rest.hddm ) then
		    mv dana_rest.hddm dana_rest_$STANDARD_NAME.hddm
		endif

		if ( "$CLEANGEANT" == "1" ) then
		    rm $STANDARD_NAME'_geant'$GEANTVER'.hddm'
		    rm control.in
		    rm -f geant.hbook
		    rm -f hdgeant.rz
		    if ( "$PWD" != "$MCWRAPPER_CENTRAL" ) then
			rm temp_Gcontrol.in	
		    endif
		endif
		
		if ( "$CLEANSMEAR" == "1" ) then
		    rm $STANDARD_NAME'_geant'$GEANTVER'_smeared.hddm'
		    rm smear.root
		endif
		
		if ( "$CLEANRECON" == "1" ) then
		    rm dana_rest*
		endif
		
		set rootfiles=`ls *.root`
		set filename_root=""

		foreach rootfile ($rootfiles)
	    	set filename_root=`echo $rootfile | sed -r 's/.{5}$//'`
			set filetomv="$rootfile"
			set filecheck=`echo $current_files | grep -c $filetomv`

			if ( "$filecheck" == "0" ) then
		    	mv $filetomv $filename_root\_$STANDARD_NAME.root
		    	mv $PWD/$filename_root\_$STANDARD_NAME.root $OUTDIR/root/
			endif
		end
		
		
	    endif
	endif
    endif
endif

if ( "$gen_pre" != "file" ) then
    mv $PWD/*.conf $OUTDIR/configurations/generation/
endif

set hddmfiles=`ls | grep .hddm`

if ( "$hddmfiles" != "" ) then
	foreach hddmfile ($hddmfiles)
		set filetomv="$hddmfile" 
		set filecheck=`echo $current_files | grep -c $filetomv`
		if ( "$filecheck" == "0" ) then
    		mv $hddmfile $OUTDIR/hddm/
		endif
	end
endif

#    mv $PWD/*.root $OUTDIR/root/ #just in case
echo `date`
