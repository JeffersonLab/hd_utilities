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
	export PATH=/site/bin:${PATH} #because .login isn't executed, and need this path for SWIF
	printenv
	echo "PERL INCLUDES: "
	perl -e "print qq(@INC)"
	echo ""

	# LIST WORKING DIRECTORY
	echo "LOCAL FILES"
	ls -l
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

		# save it to web dir
		mkdir -p -m 755 ${WEBDIR_LARGE}
		cp hd_root.root ${WEBDIR_LARGE}/hd_root_${RUN_NUMBER}_${FILE_NUMBER}.root

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

########################################################### MAKE PLOTS ##########################################################

Make_Plots()
{
	cp $MONITORING_HOME/recon_test/Make_Plots.C .
	root -b -q hd_root.root 'Make_Plots.C("$HALLD_RECON_HOME/src/plugins/Analysis/monitoring_hists/HistMacro_NumHighLevelObjects.C", "HistMacro_NumHighLevelObjects.png")'
	root -b -q hd_root.root 'Make_Plots.C("$HALLD_RECON_HOME/src/plugins/Analysis/monitoring_hists/HistMacro_EventInfo.C", "HistMacro_EventInfo.png")'
	root -b -q hd_root.root 'Make_Plots.C("$HALLD_RECON_HOME/src/plugins/monitoring/highlevel_online/HistMacro_Kinematics.C", "HistMacro_Kinematics.png")'
	root -b -q hd_root.root 'Make_Plots.C("$HALLD_RECON_HOME/src/plugins/Analysis/p2pi_hists/HistMacro_p2pi.C", "HistMacro_p2pi.png")'
	root -b -q hd_root.root 'Make_Plots.C("$HALLD_RECON_HOME/src/plugins/Analysis/p3pi_hists/HistMacro_p3pi.C", "HistMacro_p3pi.png")'

	# setup output dir
	local OUTDIR_THIS=${WEBDIR_SMALL}/${RUN_NUMBER}/
	mkdir -p -m 755 $OUTDIR_THIS

	# save it
	cp *.png ${OUTDIR_THIS}
	chmod 644 ${OUTDIR_THIS}/*.png

	# copy html
	cp $MONITORING_HOME/recon_test/index.html ${OUTDIR_THIS}
}

########################################################### CCDB SQLITE #########################################################

Create_SQLite()
{
	mysqldump -u ccdb_user -h hallddb.jlab.org ccdb > dump.mysql.sql
	$CCDB_HOME/scripts/mysql2sqlite/mysql2sqlite dump.mysql.sql | sqlite3 ccdb.sqlite
	ls -l
	export CCDB_CONNECTION=sqlite:///$PWD/ccdb.sqlite
        export JANA_CALIB_URL=sqlite:///$PWD/ccdb.sqlite
        echo "JANA_CALIB_URL: " $JANA_CALIB_URL
}

########################################################## MAIN FUNCTION ########################################################

Run_Script()
{
	Setup_Script
	Create_SQLite

	# RUN JANA
	hd_root $INPUTFILE --config=$CONFIG_FILE

	# RETURN CODE
	RETURN_CODE=$?
	echo "Return Code = " $RETURN_CODE
	if [ $RETURN_CODE -ne 0 ]; then
		exit $RETURN_CODE
	fi

	# SAVE OUTPUTS
	Make_Plots
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
WEBDIR_SMALL=$8
WEBDIR_LARGE=$9

# PRINT INPUTS
echo "HOSTNAME          = $HOSTNAME"
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "INPUTFILE         = $INPUTFILE"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "OUTDIR_LARGE      = $OUTDIR_LARGE"
echo "OUTDIR_SMALL      = $OUTDIR_SMALL"
echo "RUN_NUMBER        = $RUN_NUMBER"
echo "FILE_NUMBER       = $FILE_NUMBER"
echo "WEBDIR_SMALL      = $WEBDIR_SMALL"
echo "WEBDIR_LARGE      = $WEBDIR_LARGE"

# RUN
Run_Script

