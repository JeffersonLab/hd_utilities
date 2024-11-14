#!/bin/bash

############################################################# SETUP #############################################################

Setup_Script()
{
	# PWD, STATUS OF MOUNTED DISKS
	echo "pwd =" $PWD
	if [[ $PWD != /scratch/slurm/${SLURM_JOB_ID} ]] ; then
	    echo "LOCAL DIRECTORY " $PWD " NOT SUPPORTED"
	    exit 1
	fi
	echo "df -h:"
	df -h

	# ENVIRONMENT (shell script or xml?)
	ENV_EXT="${ENVIRONMENT##*.}"
	ENV_DIR=`dirname $ENVIRONMENT`
	if [[ $ENV_EXT == "sh" ]] ; then
	    source $ENVIRONMENT
	elif [[ $ENV_EXT == "xml" ]] ; then
	    if [[ $ENV_DIR == "." ]] ; then
		source /group/halld/Software/build_scripts/gluex_env_jlab.sh /group/halld/www/halldweb/html/halld_versions/$ENVIRONMENT
	    else
		source /group/halld/Software/build_scripts/gluex_env_jlab.sh $ENVIRONMENT
	    fi
	else
	    echo "ENVIRONMENT " $ENVIRONMENT " not supported"
	    exit 1
	fi
	printenv
	echo "PERL INCLUDES: "
	perl -e "print qq(@INC)"

	# COPY CCDB SQLITE FILE TO LOCAL DISK
	if [[ $CCDB_CONNECTION  == *"sqlite"* ]] ; then
	    local SCRATCH=/scratch/slurm/${SLURM_JOB_ID}
	    ls -l $SCRATCH
	    local NEW_SQLITE=${SCRATCH}/ccdb.sqlite
            cp -v ${CCDB_CONNECTION:10} $NEW_SQLITE
            export CCDB_CONNECTION=sqlite:///$NEW_SQLITE
            export JANA_CALIB_URL=sqlite:///$NEW_SQLITE
            echo "JANA_CALIB_URL: " $JANA_CALIB_URL
	fi

	if [[ $INPUTFILE  == *"tar" ]] ; then
	    # extract and flatten directories
	    tar xvf $INPUTFILE --transform='s/.*\///'
	    rm $INPUTFILE
	    # update input files
	    export INPUTFILE=${INPUTFILE/.hddm.tar/_*.hddm}
	    echo INPUTFILE = $INPUTFILE
	fi

	# LIST WORKING DIRECTORY
	echo "LOCAL FILES"
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

	# CALL SAVE FUNCTIONS
	Save_Histograms
	Save_REST
	Save_JANADot
	Save_EVIOSkims
	Save_HDDMSkims
	Save_ROOTFiles
}

Save_Histograms()
{
	# SAVE ROOT HISTOGRAMS
	if [ -e hd_root.root ]; then
		echo "Saving histogram file"

		# setup output dirs
		local OUTDIR_THIS=${OUTDIR_LARGE}/hists/${RUN_NUMBER}

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
		local OUTDIR_THIS=${OUTDIR_LARGE}/REST/${RUN_NUMBER}

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
		local OUTDIR_THIS=${OUTDIR_LARGE}/janadot/${RUN_NUMBER}

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/janadot_${RUN_NUMBER}_${FILE_NUMBER}.pdf
		echo "Adding jana.pdf to swif2 output: $OUTPUT_FILE"
		swif2 output jana.pdf $OUTPUT_FILE
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
		local OUTDIR_THIS=${OUTDIR_LARGE}/${SKIM_NAME}/${RUN_NUMBER}

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/${SKIM_NAME}_${RUN_NUMBER}_${FILE_NUMBER}.evio
		echo "Adding $EVIO_FILE to swif2 output: $OUTPUT_FILE"
		swif2 output $EVIO_FILE $OUTPUT_FILE

	done
}

Save_HDDMSkims()
{
	local NUM_FILES=`ls *.hddm 2>/dev/null | wc -l`
	if [ $NUM_FILES -eq 0 ] ; then
		echo "No HDDM skim files produced"
		return
	fi

	# SAVE HDDM SKIMS #assumes REST file already backed up and removed!
	echo "Saving HDDM skim files"
	for HDDM_FILE in `ls *.hddm | grep -v dana_rest.hddm`; do
		Extract_BaseName $HDDM_FILE SKIM_NAME

		# setup output dir
		local OUTDIR_THIS=${OUTDIR_LARGE}/${SKIM_NAME}/${RUN_NUMBER}

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
		local OUTDIR_THIS=${OUTDIR_LARGE}/${BASE_NAME}/${RUN_NUMBER}

		# save it
		local OUTPUT_FILE=${OUTDIR_THIS}/${BASE_NAME}_${RUN_NUMBER}_${FILE_NUMBER}.root
		echo "Adding $ROOT_FILE to swif2 output: $OUTPUT_FILE"
		swif2 output $ROOT_FILE $OUTPUT_FILE

	done
}


########################################################## MAIN FUNCTION ########################################################

Run_Script()
{
	Setup_Script

	# RUN JANA
	hd_root $INPUTFILE --config=$CONFIG_FILE

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

# PRINT INPUTS
echo "HOSTNAME          = $HOSTNAME"
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "INPUTFILE         = $INPUTFILE"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "OUTDIR_LARGE      = $OUTDIR_LARGE"
echo "OUTDIR_SMALL      = $OUTDIR_SMALL"
echo "RUN_NUMBER        = $RUN_NUMBER"
echo "FILE_NUMBER       = $FILE_NUMBER"

# RUN
Run_Script

