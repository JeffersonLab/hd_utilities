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

set MINRUN     = $1
set MAXRUN     = $2
# File #s to iterate over for this run.
set MINFILE =   0
set MAXFILE =  19

set INPUT2 = $3
set INPUT3 = $4

if ( $INPUT2 != "" ) then
  set MINFILE = $INPUT2
endif

if ( $INPUT3 != "" ) then
  set MAXFILE = $INPUT3
endif

if ($1 == "" ) then
  echo "generatejobs_plugins_rawdata.sh"
  echo "Usage:"
  echo "generatejobs_plugins_rawdata.sh [minrun] [maxrun]"
  exit
endif

source /home/gxproj1/setup_jlab.csh

# Get svn revision number to put in output file name
cd $HDDS_HOME
set svn_hdds = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "hdds revision      = ${svn_hdds}"
cd -

cd $HALLD_HOME
set svn_sim_recon = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "sim-recon revision = ${svn_sim_recon}"
cd -

cd /home/gxproj1/builds/online/packages/monitoring/src/plugins
set svn_plugins = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "plugins revision   = ${svn_plugins}"
cd -

# set date
set DATE  = `date +"%Y-%m-%d"`
set YEAR  = `date +"%Y"`
set MONTH = `date +"%m"`
set DAY   = `date +"%d"`

########################################################
#      Generate run condition file in
#      /work/halld/data_monitoring/run_conditions/
########################################################
set XMLFILE = "/work/halld/data_monitoring/run_conditions/soft_comm_${YEAR}_${MONTH}_${DAY}.xml"
if ( -e $XMLFILE ) then
  rm -f $XMLFILE
endif

echo "<gversions>" > $XMLFILE
echo "<package name="\""jana"\"" version="\""0.7.2"\""/>" >> $XMLFILE
echo "<package name="\""sim-recon"\"" version="\""commissioning.${svn_sim_recon}"\""/>" >> $XMLFILE
echo "<package name="\""hdds"\"" version="\""commissioning.${svn_hdds}"\""/>" >> $XMLFILE
echo "<package name="\""evio"\"" version="\""4.3.1"\""/>" >> $XMLFILE
echo "<package name="\""cernlib"\"" version="\""2005"\"" word_length="\""64-bit"\""/>" >> $XMLFILE
echo "<package name="\""xerces-c"\"" version="\""3.1.1"\""/>" >> $XMLFILE
echo "<package name="\""root"\"" version="\""5.34.01"\""/>" >> $XMLFILE
echo "<package name="\""ccdb"\"" version="\""1.03"\""/>" >> $XMLFILE
echo "<package name="\""monitoring"\"" version="\""${svn_plugins}"\""/>" >> $XMLFILE
echo "<!---" >> $XMLFILE
echo "<package name="\""clhep"\"" version="\""2.0.4.5"\""/>" >> $XMLFILE
echo "<package name="\""geant4"\"" version="\""9.4"\""/>" >> $XMLFILE
echo "-->" >> $XMLFILE
echo "</gversions>" >> $XMLFILE

########################################################
#      Generate jana config file in
#      /work/halld/data_monitoring/run_conditions/
########################################################

set RUN = ${MINRUN}

while (${RUN} <= ${MAXRUN})
	set FORMATTED_RUN = `printf %06d $RUN`
	echo "----------    Run = ${FORMATTED_RUN}   ---------------"

	#---------------------------------------------------------------------------------------
	#
	# Settings for tape files
	set SOURCETYPE        = "mss"
	set INPUT_DIR         = "/mss/halld/RunPeriod-2014-10/rawdata/Run${FORMATTED_RUN}"
	set INPUT_PREFIX      = "hd_rawdata_${FORMATTED_RUN}_"
	if(${RUN} >= 800 && ${RUN} <= 980) then
          set INPUT_PREFIX    = "hd_raw_${FORMATTED_RUN}_"
	endif
	if(${RUN} == 984 || ${RUN} == 997) then
          set INPUT_PREFIX    = "hd_raw_${FORMATTED_RUN}_"
	endif
	if(${RUN} >= 1001 && ${RUN} <= 1005) then
          set INPUT_PREFIX    = "hd_raw_${FORMATTED_RUN}_"
	endif
	set INPUT_SUFFIX      = ".evio"
	set THIS_DIR          = /home/gxproj1/halld/monitoring/batch/
	set SCRIPT_OUTPUT_DIR = "${THIS_DIR}/runs/${FORMATTED_RUN}/"
	set OUTDIR            = "/volatile/halld/RunPeriod-2014-10/offline_monitoring/${DATE}"
	set PLUGINS           = "DAQ,TTab,TAGH_online,TAGM_online,BCAL_online,CDC_online,CDC_expert,FCAL_online,FDC_online,ST_online,TOF_online,monitoring_hists,evio_writer,2trackskim,TOF_TDC_shift"
#,PS_online,PSC_online"
	set SCRIPTFILE        = "/home/gxproj1/halld/monitoring/batch/script.sh"
	set SOURCEFILE        = "/home/gxproj1/setup_jlab.csh"
	set TIME              = 1440 # in minutes
	set DISKSPACE         =  8 # in GB
	set MEMORY            =  3 # in GB
	set EMAIL             = "gxproj1@jlab.org"
#	set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
	set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
	set JANAFILE_SUFFIX   = "_1000A"
        set ADDITIONAL_OPTION = ""

	# Get magnetic field setting from Sean's database
        set BFIELD_MEAS = `mysql -sN -h hallddb -u datmon -e "select solenoid_current from run_info where run_num=$RUN" data_monitoring`
	if ( ${BFIELD_MEAS} != "NULL") then
          set BFIELD_MEAS_INT = `python -c "from math import ceil; print int(ceil($BFIELD_MEAS))"`
        else
          set BFIELD_MEAS_INT = "-999"
        endif

	if( ${BFIELD_MEAS_INT} < 25 ) then
	  set BFIELD_OPTION     = "-PBFIELD_TYPE=NoField"
          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
	  set JANAFILE_SUFFIX   = "_0000A"
	else if ( ${BFIELD_MEAS_INT} >= 25 && ${BFIELD_MEAS_INT} < 75) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_50A_poisson_20140819"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_0050A"
	else if ( ${BFIELD_MEAS_INT} >= 75 && ${BFIELD_MEAS_INT} < 150) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_100A_poisson_20140819"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_0100A"
	else if ( ${BFIELD_MEAS_INT} >= 150 && ${BFIELD_MEAS_INT} < 250) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_200A_poisson_20140819"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_0200A"
	else if ( ${BFIELD_MEAS_INT} >= 250 && ${BFIELD_MEAS_INT} < 450) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_0300A"
	else if ( ${BFIELD_MEAS_INT} >= 450 && ${BFIELD_MEAS_INT} < 675) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_600A_poisson_20141118"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_0600A"
	else if ( ${BFIELD_MEAS_INT} >= 675 && ${BFIELD_MEAS_INT} < 875) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_750A_poisson_20140314"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_0750A"
	else if ( ${BFIELD_MEAS_INT} >= 875 && ${BFIELD_MEAS_INT} < 1100) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_1000A"
	else if ( ${BFIELD_MEAS_INT} >= 1100 && ${BFIELD_MEAS_INT} < 1275) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_1200A"
	else if ( ${BFIELD_MEAS_INT} >= 1275 && ${BFIELD_MEAS_INT} < 1425) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1350_poisson_20130925"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_1350A"
	else if ( ${BFIELD_MEAS_INT} >= 1425) then
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1500A_poisson_20140520"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_1350A"
	else if ( ${BFIELD_MEAS_INT} == -999 ) then
	  echo "NO MAGNETIC FIELD INFO IN DATABASE... SETTING TO DEFAULT 1200 A"
	  set BFIELD_OPTION     = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
          set ADDITIONAL_OPTION = ""
	  set JANAFILE_SUFFIX   = "_1200A"
	else
	  echo "VALUE UNKNOWN"
	  echo "RUN = $RUN, BFIELD_MEAS = ${BFIELD_MEAS}"
	  exit
        endif

#	if($RUN == 9101) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
#	  set JANAFILE_SUFFIX   = "_1200A"
#	else if($RUN >= 940) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
#          set JANAFILE_SUFFIX   = "_1000A"
#	else if($RUN >= 1036 && $RUN <= 1053) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#	else if($RUN >= 1065 && $RUN <= 1121) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#	else if($RUN >= 1122 && $RUN <= 1211) then
#	  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#	else if($RUN >= 1212 && $RUN <= 1254) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#	else if($RUN >= 1255 && $RUN <= 1308) then
#	  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#	else if($RUN >= 1309 && $RUN <= 1318) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#	else if($RUN >= 1319 && $RUN <= 1448) then
#	  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#	else if($RUN >= 1449 && $RUN <= 1643) then
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
#          set JANAFILE_SUFFIX   = "_1200A"
#        else if($RUN >= 1644 && $RUN <= 1645) then
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#        else if($RUN >= 1646 && $RUN <= 1650) then
#          # actually between 150 - 260
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#        else if($RUN >= 1651 && $RUN <= 1655) then
#          set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#        else if($RUN == 1656) then
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#        else if($RUN >= 1657 && $RUN <= 1675) then
#          set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#        else if($RUN == 1676) then
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_100A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0100A"
#        else if($RUN >= 1677 && $RUN <= 1696) then
#          # actually 800 A
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_750A_poisson_20140314"
#          set JANAFILE_SUFFIX   = "_0750A"
#        else if($RUN >= 1697 && $RUN <= 1698) then
#          # actually 950 A
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
#          set JANAFILE_SUFFIX   = "_1000A"
#        else if($RUN >= 1699 && $RUN <= 1810) then
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
#          set JANAFILE_SUFFIX   = "_1200A"
#        else if($RUN >= 1811 && $RUN <= 1823) then
#          set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#        else if($RUN >= 1824 && $RUN <= 1845) then
#          # ramping from 0 A to 950 A
#          set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#        else if($RUN >= 1846 && $RUN <= 1876) then
#          # actually 150 A
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_200A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0200A"
#        else if($RUN >= 1877 && $RUN <= 1904) then
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#        else if($RUN == 1905) then
#          # actually 800 A
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
#          set JANAFILE_SUFFIX   = "_1000A"
#	else if($RUN >= 1906 && $RUN <= 2016) then
#	  # ramping from 0 A to 950 A
#	  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#	else if($RUN >= 2017 && $RUN <= 2022) then
#	  # actually ramping from 230 A - 800 A
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_750A_poisson_20140314"
#          set JANAFILE_SUFFIX   = "_0750A"
#	else if($RUN >= 2023 && $RUN <= 2029) then
#	  # actually ramping from 1000 A - 1200 A
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
#          set JANAFILE_SUFFIX   = "_1000A"
#	else if($RUN >= 2030 && $RUN <= 2223) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
#          set JANAFILE_SUFFIX   = "_1200A"
#	else if($RUN >= 2224 && $RUN <= 2229) then
#	  # actually ramping down from 1000 A - 800 A
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
#          set JANAFILE_SUFFIX   = "_1000A"
#        else if($RUN == 2230) then
#	  # during ramp down
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_300A_poisson_20140819"
#          set JANAFILE_SUFFIX   = "_0300A"
#	else if($RUN >= 2231 && $RUN <= 2246) then
#	  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#	else if($RUN >= 2247 && $RUN <= 2250) then
#	  # actually ramping from 650 A - 845 A
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_750A_poisson_20140314"
#          set JANAFILE_SUFFIX   = "_0750A"
#	else if($RUN >= 2251 && $RUN <= 2261) then
#	  # actually ramping down from 900 A - 1078 A
#          set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1000A_poisson_20141104"
#          set JANAFILE_SUFFIX   = "_1000A"
#	else if($RUN >= 2262) then
#	  set BFIELD_OPTION   = "-PBFIELD_MAP=Magnets/Solenoid/solenoid_1200A_poisson_20140520"
#          set JANAFILE_SUFFIX   = "_1200A"
#	else
#	  echo "-----------------------------------------------"
#	  echo "run $RUN does not have magnetic field set"
#	  echo "using 0 A setting..."
#	  echo "-----------------------------------------------"
#	  set BFIELD_OPTION   = "-PBFIELD_TYPE=NoField"
#          set ADDITIONAL_OPTION = "-PDEFTAG:DTrackCandidate=StraightLine"
#          set JANAFILE_SUFFIX   = "_0000A"
#	endif
#	#---------------------------------------------------------------------------------------

	# Create jana config file
	set JANAFILE = "/work/halld/data_monitoring/run_conditions/jana_rawdata_comm_${YEAR}_${MONTH}_${DAY}.conf"
	set JANAFILE_COPY = ${JANAFILE}${JANAFILE_SUFFIX}
	if ( -e $JANAFILE ) then
	rm -f $JANAFILE
	endif

	echo "-PPLUGINS=DAQ,TTab,TAGH_online,TAGM_online,BCAL_online,CDC_online,CDC_expert,FCAL_online,FDC_online,ST_online,TOF_online,monitoring_hists,evio_writer,2trackskim,TOF_TDC_shift" > $JANAFILE
	echo ${BFIELD_OPTION} ${ADDITIONAL_OPTION} >> $JANAFILE
	echo "-PNTHREADS=1" >> $JANAFILE
	echo "-PTHREAD_TIMEOUT=300" >> $JANAFILE
	# Grab date of JANA_CALIB_URL
	set JANA_CALIB_URL_DATE = `echo $JANA_CALIB_URL | sed 's/sqlite:\/\/\/\/group\/halld\/www\/halldweb1\/html\/dist\/ccdb_//' | sed s/.sqlite//`
	echo "-PCALIB_CONTEXT="\""calibtime=${JANA_CALIB_URL_DATE}"\" >> $JANAFILE

	# Make a copy of the JANAFILE for this B field setting
	cp $JANAFILE $JANAFILE_COPY

	### Echo settings
	echo "input dir         : ${INPUT_DIR}" 
	echo "plugins           : $PLUGINS"
	echo "Bfield option     : ${BFIELD_OPTION}"
	echo "additional option : ${ADDITIONAL_OPTION}"     

	mkdir -p ${THIS_DIR}/runscripts
	set RUN_SCRIPT = "${THIS_DIR}/runscripts/run_rawdata_${FORMATTED_RUN}.sh"
	if( -e $RUN_SCRIPT ) then
	  rm -f $RUN_SCRIPT
	endif

	echo "" > $RUN_SCRIPT
	ls -lh $RUN_SCRIPT

	mkdir -p ${SCRIPT_OUTPUT_DIR}
	mkdir -p ${OUTDIR}
	mkdir -p ${OUTDIR}/ROOT/${FORMATTED_RUN}
	mkdir -p ${OUTDIR}/skims/${FORMATTED_RUN}
	mkdir -p ${OUTDIR}/misc/${FORMATTED_RUN}
	mkdir -p ${OUTDIR}/log/${FORMATTED_RUN}
	# This is so other gluex group users can read/write
	chmod g+rw ${OUTDIR} ${OUTDIR}/ROOT ${OUTDIR}/skims ${OUTDIR}/misc ${OUTDIR}/log ${OUTDIR}/ROOT/${FORMATTED_RUN} ${OUTDIR}/skims/${FORMATTED_RUN} ${OUTDIR}/misc/${FORMATTED_RUN} ${OUTDIR}/log/${FORMATTED_RUN}

	# Create ls file. This is just all the runs within the
	# input directory.
	if( -e ${INPUT_DIR} ) then
          echo "${INPUT_DIR} exists"
     	  cd $INPUT_DIR
        else
	  echo "${INPUT_DIR} doesn't exist"
	  @ RUN += 1
          continue
        endif

	mkdir -p ${THIS_DIR}/lsfiles/
	set LSFILE = "${THIS_DIR}/lsfiles/ls-evio_${FORMATTED_RUN}"
	if ( -e $LSFILE ) then
	  rm -f $LSFILE
	endif

        ls --color=never *.evio > $LSFILE
        cat $LSFILE

	# Create input list file. This just tracks which
	# files were present.
	set LISTFILE = ${OUTDIR}/misc/${FORMATTED_RUN}/infiles_${FORMATTED_RUN}.txt
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
		echo '	<Variable name="env" value="setup_jlab.csh"/>' >> ${OUTPUT_FILE}
		echo '	<Variable name="svn" value="'${svn_sim_recon}'"/>' >> ${OUTPUT_FILE}
		echo '	<Variable name="bfield_option" value="'${BFIELD_OPTION}'"/>' >> ${OUTPUT_FILE}
		echo '	<Variable name="additional_option" value="'${ADDITIONAL_OPTION}'"/>' >> ${OUTPUT_FILE}
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
		echo '			<Command>./script.sh ${env} '${INPUT_PREFIX}'${input_file_stubs}'${INPUT_SUFFIX}' ${plugins} ${bfield_option} ${additional_option} ${formatted_run}</Command>' >> ${OUTPUT_FILE}
		echo '			<Output src="hd_root.root" dest="${output_dir_base}/ROOT/${formatted_run}/hd_root_${formatted_run}_${input_file_stubs}.root"/>' >> ${OUTPUT_FILE}
		echo '			<Output src="'${INPUT_PREFIX}'${input_file_stubs}.2tracks.evio" dest="${output_dir_base}/skims/${formatted_run}/'${INPUT_PREFIX}'${input_file_stubs}.2tracks.evio"/>' >> ${OUTPUT_FILE}
		echo '			<Output src="TOF_TDC_shift_${formatted_run}.txt" dest="${output_dir_base}/misc/${formatted_run}/TOF_TDC_shift_${formatted_run}_${input_file_stubs}.txt"/>' >> ${OUTPUT_FILE}
		echo '			<Stdout dest="${output_dir_base}/log/${formatted_run}/stdout_${formatted_run}_${input_file_stubs}.out"/>' >> ${OUTPUT_FILE}
		echo '			<Stderr dest="${output_dir_base}/log/${formatted_run}/stderr_${formatted_run}_${input_file_stubs}.err"/>' >> ${OUTPUT_FILE}
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
	@ RUN += 1
	cd -

end
