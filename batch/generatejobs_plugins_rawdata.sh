#!/bin/tcsh

###############################################################
#
# 2014/10/30 Kei Moriya
#
# Script to run at JLab Auger system.
# Original scripts from Nathan Sparks, who got
# them from Paul Mattione.
#
# This script will run over rawdata files within
# the JLab tape system.
#
# Usage:
# generatejobs_plugins_rawdata.sh [run #]
# Optionally,
# generatejobs_plugins_rawdata.sh [run #] (min file #) (max file #)
# will speed up file #s to look for.
#
# The script will create a file
# ls-evio_RunXXXXXX that contains a list of files
# with the corresponding run #.
# This saves time of querying 1000's of files
# for each run.
#
# The file script.sh runs will set up the environment,
# and run on the specified file. All arguments
# will be set up within this script.
#
# Input parameters to script.sh:
# - setup file to source
# - input file name
# - plugins to run
# - Bfield option
# - nthreads
#
###############################################################

set RUN     = $1
# File #s to iterate over for this run.
set MINFILE =   0
set MAXFILE = 999

set INPUT2 = $2
set INPUT3 = $3

if ( $INPUT2 != "" ) then
  set MINFILE = $INPUT2
endif

if ( $INPUT3 != "" ) then
  set MAXFILE = $INPUT3
endif

if ($RUN == "" ) then
  echo "gen_requests.sh"
  echo "Usage:"
  echo "generatejobs_plugins_rawdata.sh [run]"
  exit
endif

set FORMATTED_RUN = `printf %06d $RUN`
source /home/gluex/setup_jlab_commissioning.csh

# Get svn revision number to put in output file name
cd $HALLD_HOME
set svn = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "svn revision = $svn"
cd -

#---------------------------------------------------------------------------------------
#
# Settings for tape files
set SOURCETYPE        = "mss"
set INPUT_DIR         = "/mss/halld/RunPeriod-2014-10/rawdata/Run${FORMATTED_RUN}"
set INPUT_PREFIX      = "hd_raw_${FORMATTED_RUN}_"
set INPUT_SUFFIX      = ".evio"
set THIS_DIR          = $PWD
set SCRIPT_OUTPUT_DIR = "${THIS_DIR}/${FORMATTED_RUN}/"
set OUTDIR            = "/u/scratch/gluex/offline_monitoring/${FORMATTED_RUN}"
set PLUGINS           = "DAQ,TTab,TAGH_online,TAGM_online,BCAL_online,CDC_online,CDC_expert,FCAL_online,FDC_online,ST_online,TOF_online,monitoring_hists"
set SCRIPTFILE        = "/home/gluex/halld/monitoring/batch/script.sh"
set SOURCEFILE        = "/home/gluex/setup_jlab_commissioning.csh"
set TIME              = 60 # in minutes
set DISKSPACE         =  1 # in GB
set MEMORY            =  3 # in GB
set EMAIL             = "gluex@jlab.org"
set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
if($RUN == 9101) then
  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
else if($RUN == 9102) then
  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#else
#  echo "run must be 9101 or 9102"
#  exit
endif
#---------------------------------------------------------------------------------------

### Echo settings
echo "input dir    : ${INPUT_DIR}" 
echo "plugins      : $PLUGINS"
echo "Bfield option: ${BFIELD_OPTION}"

set RUN_SCRIPT = "${THIS_DIR}/run_rawdata_${FORMATTED_RUN}.sh"
if( -e $RUN_SCRIPT ) then
  rm -f $RUN_SCRIPT
endif

echo "" > $RUN_SCRIPT

mkdir -p ${SCRIPT_OUTPUT_DIR}
mkdir -p ${OUTDIR}

# Create ls file. This is just all the runs within the
# input directory.
cd $INPUT_DIR
set LSFILE = "${THIS_DIR}/ls-evio_${FORMATTED_RUN}"
if ( -e $LSFILE ) then
  rm -f $LSFILE
endif
ls --color=never *.evio > $LSFILE

cat $LSFILE

# Create input list file. This just tracks which
# files were present.
set LISTFILE = ${OUTDIR}/infiles_${FORMATTED_RUN}.txt
if ( -e $LISTFILE ) then
  rm -f $LISTFILE
endif
touch $LISTFILE

set FILENUM = ${MINFILE}
while (${FILENUM} <= ${MAXFILE})
        set FORMATTED_FILE = `printf %03d $FILENUM`
	set OUTPUT_FILE = ${SCRIPT_OUTPUT_DIR}"/jsub_"${FORMATTED_RUN}"_"${FORMATTED_FILE}".xml"
	if( -e $OUTPUT_FILE ) then
	  rm -f $OUTPUT_FILE
	endif

	echo '<Request>' > ${OUTPUT_FILE}
	echo '	<Variable name="formatted_run" value="'${FORMATTED_RUN}'"/>' >> ${OUTPUT_FILE}
	echo '	<Variable name="output_dir_base" value="file:'${OUTDIR}'"/>' >> ${OUTPUT_FILE}
	echo '	<Variable name="plugins" value="'${PLUGINS}'"/>' >> ${OUTPUT_FILE} # monitoring_hists,
	echo '	<Variable name="env" value="setup_jlab_commissioning.csh"/>' >> ${OUTPUT_FILE}
	echo '	<Variable name="svn" value="'$svn'"/>' >> ${OUTPUT_FILE}
	echo '	<Variable name="bfield_option" value="'${BFIELD_OPTION}'"/>' >> ${OUTPUT_FILE}
	echo >> ${OUTPUT_FILE}

	echo '	<Email email="'${EMAIL}'" request="false" job="false"/>' >> ${OUTPUT_FILE}
	echo '	<Project name="gluex"/>' >> ${OUTPUT_FILE}
	echo '	<Track name="simulation"/>' >> ${OUTPUT_FILE}
	echo '	<TimeLimit unit="minutes" time="'${TIME}'"/>' >> ${OUTPUT_FILE}
	echo '	<DiskSpace space="'${DISKSPACE}'" unit="GB"/>' >> ${OUTPUT_FILE}
	echo '	<Memory space="'${MEMORY}'" unit="GB"/>' >> ${OUTPUT_FILE}
	echo '	<CPU core="1"/>' >> ${OUTPUT_FILE}
	echo '	<OS name="centos62"/>' >> ${OUTPUT_FILE}

	echo >> ${OUTPUT_FILE}

	echo '	<Input src="'${SCRIPTFILE}'" dest="script.sh"/>' >> ${OUTPUT_FILE}
	echo '	<Input src="'$SOURCEFILE'" dest="setup_jlab_commissioning.csh"/>' >> ${OUTPUT_FILE}

	echo >> ${OUTPUT_FILE}
	set FOUND = 0
	echo '	<List name="input_file_stubs">' >> ${OUTPUT_FILE}
	foreach FILE (`grep ${INPUT_PREFIX} $LSFILE | grep ${FORMATTED_RUN}_${FORMATTED_FILE}`)
		echo "FILE = $FILE"
		set INFILE_STUB = `echo ${FILE} | sed -e 's/'${INPUT_PREFIX}'//g' -e 's/'${INPUT_SUFFIX}'//g'`
		echo '		'${INFILE_STUB} >> ${OUTPUT_FILE}
		set FOUND = 1
		echo "${INPUT_DIR}/${FILE}" >> $LISTFILE
	end
	echo '	</List>' >> ${OUTPUT_FILE}
	
	if( ${FOUND} == 0 ) then
	    rm -f ${OUTPUT_FILE} 
	    @ FILENUM += 1
	else 
	echo >> ${OUTPUT_FILE}

	echo '	<ForEach list="input_file_stubs">' >> ${OUTPUT_FILE}
	echo '		<Job>' >> ${OUTPUT_FILE}
	echo '			<Name name="rawdata_plugins.'${FORMATTED_RUN}'_${input_file_stubs}"/>' >> ${OUTPUT_FILE}
	echo '			<Input src="'${SOURCETYPE}':'${INPUT_DIR}'/'${INPUT_PREFIX}'${input_file_stubs}'${INPUT_SUFFIX}'" dest="'${INPUT_PREFIX}'${input_file_stubs}'${INPUT_SUFFIX}'"/>' >> ${OUTPUT_FILE}
	echo '			<Command>script.sh ${env} '${INPUT_PREFIX}'${input_file_stubs}'${INPUT_SUFFIX}' ${plugins} ${bfield_option} 1</Command>' >> ${OUTPUT_FILE}
	echo '			<Output src="hd_root.root" dest="${output_dir_base}/hd_root_${formatted_run}_${input_file_stubs}.root"/>' >> ${OUTPUT_FILE}
	echo '			<Stdout dest="${output_dir_base}/log/stdout_${formatted_run}_${input_file_stubs}.out"/>' >> ${OUTPUT_FILE}
	echo '			<Stderr dest="${output_dir_base}/log/stderr_${formatted_run}_${input_file_stubs}.err"/>' >> ${OUTPUT_FILE}
	echo '		</Job>' >> ${OUTPUT_FILE}
	echo '	</ForEach>' >> ${OUTPUT_FILE}

	echo >> ${OUTPUT_FILE}

	echo '</Request>' >> ${OUTPUT_FILE}
	echo ${OUTPUT_FILE} "CREATED"

	# add the xml file to list of jobs to submit
	if( -e $RUN_SCRIPT ) then
	    echo "echo "\"submitting ${OUTPUT_FILE}\" >> ${RUN_SCRIPT}
	    echo "jsub -xml ${OUTPUT_FILE}"  >> ${RUN_SCRIPT}
	else
	    echo "echo "\"submitting ${OUTPUT_FILE}\" > ${RUN_SCRIPT}
	    echo "jsub -xml ${OUTPUT_FILE}"  >> ${RUN_SCRIPT}
	endif

	@ FILENUM += 1
	endif
end

chmod u+x ${RUN_SCRIPT}
