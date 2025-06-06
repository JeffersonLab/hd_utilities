#!/bin/bash

############################################################# SETUP #############################################################

Setup_Script()
{
	# PWD, STATUS OF MOUNTED DISKS
	echo "pwd =" $PWD
	echo "df -h:"
	df -h

	# ENVIRONMENT
	source $ENVIRONMENT
	printenv
	echo "PERL INCLUDES: "
	perl -e "print qq(@INC)"

	# COPY INPUT FILE TO WORKING DIRECTORY
	# This step is necessary since the cache files will be created as soft links in the current directory, and we want to avoid large I/O processes.
	# We first copy the input file to the current directory, then remove the link.
	echo "LOCAL FILES PRIOR TO INPUT COPY"
	ls -l
	cp $INPUTFILE ./tmp_file
	rm -f $INPUTFILE
	mv tmp_file $INPUTFILE
	echo "LOCAL FILES AFTER INPUT COPY"
	ls -l
}

####################################################### UTILITY FUNCTIONS #######################################################

Extract_SkimName()
{
	# to extract the skim name, first extract the locations of the last two periods in the file name
	local LAST_INDEX=0
	local SECOND_TO_LAST_INDEX=0
	local INPUT_FILE=$1
	for INDEX in `echo $INPUT_FILE | grep -bo '\.' | awk 'BEGIN {FS=":"}{print $1}'`; do
		SECOND_TO_LAST_INDEX=$LAST_INDEX
		LAST_INDEX=$INDEX
	done

	# extract the skim name: awk & grep use different location #'ing, so must convert
	local LENGTH=$[$LAST_INDEX - $SECOND_TO_LAST_INDEX - 1]
	local START=$[$SECOND_TO_LAST_INDEX + 2]
	local LOCAL_SKIM_NAME=`echo $INPUT_FILE | awk -v size="$LENGTH" -v start="$START" '{print substr($0,start,size)}'`
	echo "SKIM_NAME:" $LOCAL_SKIM_NAME
	
	#return the result "by reference"
	local __result=$2
	eval $__result="'$LOCAL_SKIM_NAME'"
}

Extract_BaseName()
{
	# base name is everything before the last period
	local INPUT_FILE=$1
	local LENGTH=`echo $INPUT_FILE | awk '{print index($0,".")}'`
	let LENGTH-=1
	local LOCAL_BASE_NAME=`echo $INPUT_FILE | awk -v size="$LENGTH" '{print substr($0,1,size)}'`
	echo "BASE_NAME: " $LOCAL_BASE_NAME

	#return the result "by reference"
	local __result=$2
	eval $__result="'$LOCAL_BASE_NAME'"
}

####################################################### SAVE OUTPUT FILES #######################################################

Save_OutputFiles()
{
	# SEE WHAT FILES ARE PRESENT
	echo "FILES PRESENT PRIOR TO SAVE:"
	ls -l

	# REMOVE INPUT FILE: so that it's easier to determine which remaining files are skims
	rm -f $INPUTFILE

	# BUILD TAPEDIR, IF $OUTDIR_LARGE STARTS WITH "/cache/"
	# AND CACHE_PIN_DAYS WAS GIVEN AND GREATER THAN 0  && [ "$CACHE_PIN_DAYS" -gt "0" ]
	# If so, output files are pinned & jcache put.  If not, then they aren't. 
	local TAPEDIR=""
	local OUTDIR_LARGE_BASE=`echo $OUTDIR_LARGE | awk '{print substr($0,1,7)}'`
	# first strip /cache/, then insert /mss/
	if [ "$OUTDIR_LARGE_BASE" == "/cache/" ]; then
		local OUTPATH=`echo $OUTDIR_LARGE | awk '{print substr($0,8)}'`
		TAPEDIR=/mss/${OUTPATH}/
	fi

	# CALL SAVE FUNCTIONS
	Save_Histograms
	Save_REST
	Save_JANADot
	Save_EVIOSkims
	Save_HDDMSkims
	Save_ROOTFiles

	# SEE WHAT FILES ARE LEFT
	echo "FILES REMAINING AFTER SAVING:"
	ls -l
}

Save_Histograms()
{
	# SAVE ROOT HISTOGRAMS
	if [ -e hd_root.root ]; then
		echo "Saving histogram file"

		# setup output dirs
		local OUTDIR_THIS=${OUTDIR_LARGE}/hists/${RUN_NUMBER}/

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root
		echo "Adding hd_root.root to swif2 output: $OUTPUT_FILE"
		swif2 output hd_root.root $OUTPUT_FILE

	fi
}

Save_REST()
{
	# SAVE REST FILE
	if [ -e dana_rest.hddm ]; then
		echo "Saving REST file"

		# setup output dirs
		local OUTDIR_THIS=${OUTDIR_LARGE}/REST/${RUN_NUMBER}/

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/dana_rest_${RUN_NUMBER}_${FILE_NUMBER}.hddm
		echo "Adding dana_rest.hddm to swif2 output: $OUTPUT_FILE"
		swif2 output dana_rest.hddm $OUTPUT_FILE

	fi
}

Save_JANADot()
{
	# SAVE JANADOT FILE
	if [ -e jana.dot ]; then
		echo "Saving JANADOT file"
		dot -Tps2 jana.dot -o jana.ps
		ps2pdf jana.ps

		# setup output dir
		local OUTDIR_THIS=${OUTDIR_SMALL}/log/${RUN_NUMBER}/
		mkdir -p -m 755 $OUTDIR_THIS

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/janadot_${RUN_NUMBER}_${FILE_NUMBER}.pdf
		mv -v jana.pdf $OUTPUT_FILE
		chmod 644 $OUTPUT_FILE
	fi
}

Save_EVIOSkims()
{
	# SAVE EVIO SKIMS
        local NUM_FILES=`ls *.evio 2>/dev/null | wc -l`
        if [ $NUM_FILES -eq 0 ] ; then
                echo "No EVIO skim files produced"
                return
        fi

	echo "Saving EVIO skim files"
	for EVIO_FILE in `ls *.evio`; do
		Extract_SkimName $EVIO_FILE SKIM_NAME

		# setup output dir
		local OUTDIR_THIS=${OUTDIR_LARGE}/${SKIM_NAME}/${RUN_NUMBER}/

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/${SKIM_NAME}_${RUN_NUMBER}_${FILE_NUMBER}.evio
		echo "Adding $EVIO_FILE to swif2 output: $OUTPUT_FILE"
		swif2 output $EVIO_FILE $OUTPUT_FILE

	done
}

Save_HDDMSkims()
{
	local NUM_FILES=`ls *.hddm | grep -v dana_rest.hddm 2>/dev/null | wc -l`
	if [ $NUM_FILES -eq 0 ] ; then
		echo "No HDDM skim files produced"
		return
	fi

	# SAVE HDDM SKIMS #assumes REST file already backed up and removed!
	echo "Saving HDDM skim files"
	for HDDM_FILE in `ls *.hddm | grep -v dana_rest.hddm`; do
		Extract_SkimName $HDDM_FILE SKIM_NAME

		# setup output dir
		local OUTDIR_THIS=${OUTDIR_LARGE}/${SKIM_NAME}/${RUN_NUMBER}/

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/${SKIM_NAME}_${RUN_NUMBER}_${FILE_NUMBER}.hddm
		echo "Adding $HDDM_FILE to output: $OUTPUT_FILE"
		swif2 output $HDDM_FILE $OUTPUT_FILE

	done
}

Save_ROOTFiles()
{
	# SAVE OTHER ROOT FILES
        local NUM_FILES=`ls *.root | grep -v hd_root.root 2>/dev/null | wc -l`
        if [ $NUM_FILES -eq 0 ] ; then
                echo "No additional ROOT files produced"
                return
        fi

	echo "Saving other ROOT files"
	for ROOT_FILE in `ls *.root | grep -v hd_root.root`; do
		Extract_BaseName $ROOT_FILE BASE_NAME

		# setup output dir
		local OUTDIR_THIS=${OUTDIR_LARGE}/${BASE_NAME}/${RUN_NUMBER}/

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/${BASE_NAME}_${RUN_NUMBER}_${FILE_NUMBER}.root
		echo "Adding $ROOT_FILE to swif2 output: $OUTPUT_FILE"
		swif2 output $ROOT_FILE $OUTPUT_FILE

	done
}

########################################################### CCDB SQLITE #########################################################

Create_SQLite()
{
	echo "Creating ccdb.sqlite"
	#$CCDB_HOME/scripts/mysql2sqlite/mysql2sqlite.sh -hhallddb.jlab.org -uccdb_user ccdb | sqlite3 ccdb.sqlite
	mysqldump --skip-tz-utc -u ccdb_user -h hallddb.jlab.org ccdb > dump.mysql.sql
	$CCDB_HOME/scripts/mysql2sqlite/mysql2sqlite dump.mysql.sql | sqlite3 ccdb.sqlite
	rm dump.mysql.sql
	ls -lrt
	export JANA_CALIB_URL="sqlite:///${PWD}/ccdb.sqlite"
	echo "JANA_CALIB_URL = " $JANA_CALIB_URL
}

########################################################## MAIN FUNCTION ########################################################

Run_Script()
{
	Setup_Script
	Create_SQLite

	# RUN JANA(2)
	local JANA_MAJOR_VERSION=`jana -v | head -n 1 | awk '{print $3}' | cut -d'.' -f1`
	if [ $JANA_MAJOR_VERSION -ge 2 ]; then
	    hd_root $INPUTFILE --loadconfigs $CONFIG_FILE
	else
	    hd_root $INPUTFILE --config=$CONFIG_FILE
	fi

	# RETURN CODE
	RETURN_CODE=$?
	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
		exit $RETURN_CODE
	fi

	# SAVE OUTPUTS
	Save_OutputFiles
}

######################################################### EXECUTE SCRIPT ########################################################

# SET INPUTS
ENVIRONMENT=$1
INPUTFILE=$2
CONFIG_FILE=$3
OUTDIR_LARGE=$4
OUTDIR_SMALL=$5
RUN_NUMBER=$6
FILE_NUMBER=$7
CACHE_PIN_DAYS=$8

# PRINT INPUTS
echo "HOSTNAME          = $HOSTNAME"
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "INPUTFILE         = $INPUTFILE"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "OUTDIR_LARGE      = $OUTDIR_LARGE"
echo "OUTDIR_SMALL      = $OUTDIR_SMALL"
echo "RUN_NUMBER        = $RUN_NUMBER"
echo "FILE_NUMBER       = $FILE_NUMBER"
echo "CACHE_PIN_DAYS    = $CACHE_PIN_DAYS"

# RUN
Run_Script

