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

	echo "LOCAL FILES PRIOR TO INPUT COPY"
	ls -l
}


####################################################### MERGE FILES #############################################################

Merge_Files()
{
    # loop over trees and merge
    for TYPE in `ls $INPUTDIR` ; do 
	if [ "$TYPE" == "hists" ] ; then 
	    local TEMP_FILE="hd_root_${RUN}.root"
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}.root"
	fi
	hadd $TEMP_FILE $INPUTDIR/$TYPE/$RUN/*
	
	# RETURN CODE
	RETURN_CODE=$?
	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
	    exit $RETURN_CODE
	fi
	
    done
}


####################################################### SAVE OUTPUT FILES #######################################################

Save_OutputFiles()
{
	# SEE WHAT FILES ARE PRESENT
	echo "FILES PRESENT PRIOR TO SAVE:"
	ls -l

	# BUILD TAPEDIR, IF $OUTDIR STARTS WITH "/cache/"
	# AND CACHE_PIN_DAYS WAS GIVEN AND GREATER THAN 0 
	# If so, output files are pinned & jcache put.  If not, then they aren't. 
	local TAPEDIR=""
	local OUTDIR_BASE=`echo $OUTDIR | awk '{print substr($0,1,7)}'`
	# first strip /cache/, then insert /mss/
	if [ "$OUTDIR_BASE" == "/cache/" ] && [ $CACHE_PIN_DAYS -gt 0 ] ; then
	    local OUTPATH=`echo $OUTDIR | awk '{print substr($0,8)}'`
	    TAPEDIR=/mss/${OUTPATH}/
	fi

	# CALL SAVE FUNCTION
	Save_Files

	# ONLY IF SUCCESSFULLY SAVED, CALL DELETE FUNCTION
	Delete_InputFiles
		
	# SEE WHAT FILES ARE LEFT
	echo "FILES REMAINING AFTER SAVING:"
	ls -l
}

Save_Files()
{
    echo "Saving merged tree file"
    
    # SAVE MERGED TREES
    for TYPE in `ls $INPUTDIR` ; do 
	if [ "$TYPE" == "hists" ] ; then 
	    local TEMP_FILE="hd_root_${RUN}.root"
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}.root"
	fi
	
	# setup output dirs
	local OUTDIR_THIS=${OUTDIR}/$TYPE/merged/
	mkdir -p -m 755 ${OUTDIR_THIS}

	local OUTPUT_FILE=$OUTDIR_THIS/$TEMP_FILE
	
	# save it
	mv -v $TEMP_FILE $OUTPUT_FILE
	# if save failed, do not go on and delete anything
	RETURN_CODE=$?
	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
	    exit $RETURN_CODE
	fi
	
	chmod 644 $OUTPUT_FILE

	# force save to tape & pin
	if [ "$TAPEDIR" != "" ]; then
	    echo jcache pin $OUTPUT_FILE -D $CACHE_PIN_DAYS
	    jcache pin $OUTPUT_FILE -D $CACHE_PIN_DAYS
	    echo jcache put $OUTPUT_FILE
	    jcache put $OUTPUT_FILE
	fi
	
    done
}

Delete_InputFiles()
{
    echo "Deleting input tree files"
    
    # SAVE MERGED TREES
    for TYPE in `ls $INPUTDIR` ; do 
	if [ "$TYPE" == "hists" ] ; then 
	    local TEMP_FILE="hd_root_${RUN}.root"
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}.root"
	fi
	
	# DELETE INPUTFILES
	rm -rf $INPUTDIR/$TYPE/$RUN/*
	
    done
}

########################################################## MAIN FUNCTION ########################################################

Run_Script()
{
	Setup_Script

	# RUN hadd
	Merge_Files

	#Merge_Hists

	Save_OutputFiles
}

######################################################### EXECUTE SCRIPT ########################################################

# SET INPUTS
ENVIRONMENT=$1
INPUTDIR=$2
OUTDIR=$3
RUN=$4
CACHE_PIN_DAYS=$5

# PRINT INPUTS
echo "HOSTNAME          = $HOSTNAME"
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "INPUTDIR          = $INPUTDIR"
echo "OUTDIR            = $OUTDIR"
echo "RUN               = $RUN"
echo "CACHE_PIN_DAYS    = $CACHE_PIN_DAYS"

# RUN
Run_Script

