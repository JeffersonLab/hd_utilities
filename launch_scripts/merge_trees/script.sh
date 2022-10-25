#!/bin/bash

############################################################# SETUP #############################################################

Setup_Script()
{
	# PWD, STATUS OF MOUNTED DISKS
	echo "pwd =" $PWD
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

	echo "LOCAL FILES PRIOR TO INPUT COPY"
	ls -l
}


####################################################### MERGE FILES #############################################################

Merge_Files()
{
    # loop over trees and merge
    for TYPE in `ls $INPUTDIR` ; do 
	if [ "$TYPE" == "log" ] ; then 
	    local TEMP_FILE="log_${RUN}.root"
	    continue
	elif [ "$TYPE" == "hists" ] || [ "$TYPE" == "hd_root" ] || [ "$TYPE" == "flat" ] || [ "$TYPE" == "survey" ] || [ "$TYPE" == "pdf" ] ; then 
	    local TEMP_FILE="hd_root_${RUN}.root"
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}.root"
	fi

	#cp -v $INPUTDIR/$TYPE/$RUN/${TYPE}_${RUN}_*.root .

	LD_PRELOAD=/home/gxproj6/monitoring/merge_trees/startup_C.so hadd $TEMP_FILE $INPUTDIR/$TYPE/$RUN/${TYPE}_${RUN}_*.root
	#hadd $TEMP_FILE $INPUTDIR/$TYPE/$RUN/${TYPE}_${RUN}_*.root
	# RETURN CODE
	RETURN_CODE=$?

	#rm ${TYPE}_${RUN}_*.root
	
	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
	    exit $RETURN_CODE
	fi

	# test from here

	# # setup output dirs
	# local OUTDIR_THIS=${OUTDIR}/$TYPE/merged/
	# mkdir -p -m 755 ${OUTDIR_THIS}

	# local OUTPUT_FILE=$OUTDIR_THIS/$TEMP_FILE
	
	# # save it
	# mv -v $TEMP_FILE $OUTPUT_FILE

	
    done
}


####################################################### FLATTEN FILES #############################################################

Flatten_Files()
{
    # loop over trees and merge
    for TYPE in `ls $INPUTDIR` ; do 
	if [ "$TYPE" == "hists" ] || [ "$TYPE" == "hd_root" ] || [ "$TYPE" == "flat" ] || [ "$TYPE" == "survey" ] || [ "$TYPE" == "pdf" ] ; then 
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}.root"
	fi

	$FLATTEN/flatten \
           -chi2 25 \
           -numUnusedTracks 1 \
           -numUnusedNeutrals 2 \
           -shQuality 0.5 \
           -massWindows 0.05  \
           -in $PWD/$TEMP_FILE \
           -out $PWD/${TYPE}_${RUN}_flat.root

	RETURN_CODE=$?
	
	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
	    exit $RETURN_CODE
	fi

	# setup flat dirs
	local FLATDIR_THIS=$INPUTDIR/flat/$TYPE
	mkdir -p -m 755 ${FLATDIR_THIS}
	cp $PWD/${TYPE}_${RUN}_flat.root ${FLATDIR_THIS}/
	
    done
}


####################################################### CREATE HISTOGRAMS #############################################################

Create_Hists()
{
    # loop over trees and merge
    for TYPE in `ls $INPUTDIR` ; do 
	if [ "$TYPE" == "hists" ] || [ "$TYPE" == "hd_root" ] || [ "$TYPE" == "flat" ] || [ "$TYPE" == "survey" ] || [ "$TYPE" == "pdf" ] ; then 
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}_flat.root"
	fi

	$GLUEX_SURVEY/surveyFSTreeToHist \
           -in $PWD/$TEMP_FILE \
           -out $PWD/${TYPE}_${RUN}_hist.root

	RETURN_CODE=$?

	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
	    exit $RETURN_CODE
	fi

	# setup survey dirs
	local SURVEYDIR_THIS=$INPUTDIR/survey/$TYPE
	mkdir -p -m 755 ${SURVEYDIR_THIS}
	mv $PWD/${TYPE}_${RUN}_hist.root ${SURVEYDIR_THIS}/

	rm $PWD/$TEMP_FILE
	
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
	if [ "$TYPE" == "log" ] ; then 
	    local TEMP_FILE="log_${RUN}.root"
	    continue
	elif [ "$TYPE" == "hists" ] || [ "$TYPE" == "hd_root" ] || [ "$TYPE" == "flat" ] || [ "$TYPE" == "survey" ] || [ "$TYPE" == "pdf" ] ; then 
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
	if [ "$TYPE" == "log" ] ; then 
	    local TEMP_FILE="log_${RUN}.root"
	    continue
	elif [ "$TYPE" == "hists" ] ; then 
	    local TEMP_FILE="hd_root_${RUN}.root"
	    continue
	else
	    local TEMP_FILE="${TYPE}_${RUN}.root"
	fi
	
	# DELETE OR UNPIN INPUTFILES
	echo Delete $TYPE
	rm -rf $INPUTDIR/$TYPE/$RUN/*
	#jcache unpin $INPUTDIR/$TYPE/$RUN/*
	
    done
}

########################################################## MAIN FUNCTION ########################################################

Run_Script()
{
	Setup_Script

	# RUN hadd
	Merge_Files

	#Flatten_Files

	#Create_Hists

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

