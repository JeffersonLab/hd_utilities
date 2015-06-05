#!/bin/tcsh

#----------------------------------------------------------------------
#
# This script will create a new project and its
# directories from the template directory.
#
# Usage:
# ./create_project.sh [project name]
#
# 1. [project name] MUST include substring RunPeriod2YYY-MM
#
# There used to be a second option [file type] which was
# to distinguish file names of the form hd_raw and hd_rawdata,
# but since we don't care about the very early runs of 2014-10
# that had the hd_raw form, this has been taken out, and
# hd_rawdata is assumed for the raw file name.
#
# It is assumed that this file was checked out via
# svn co https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/jproj/
# so that the directories ../scripts/ and ./templates/ exists.
#
#----------------------------------------------------------------------

set PROJECT = $1
set PROJHOME = $PWD
echo "PROJHOME =  $PROJHOME"
set USERNAME = $USER

if ( $PROJECT == "" ) then
  echo "Usage:"
  echo "./create_project.sh [project name]"
  echo "project name MUST contain"
  echo "[RunPeriod20YY-MM] and ver[NN] as substrings"
  exit
endif

# Set run period.
# This assumes that the run period is contained
# in the project name as RunPeriod2YYY_MM
set POS = `echo $PROJECT | gawk '{print index($1,"RunPeriod")}'`
set POS = `expr $POS + 9`
set RUNPERIOD = `expr substr $PROJECT $POS 7`

# The reason for RUNPERIOD and RUNPERIOD_HYPHEN
# is because mysql will not allow tables with a "-"
# in them, while the directory names used on tape are
# e.g. 2014-10. Therefore we need to have two similar
# variables.
set RUNPERIOD_HYPHEN = `echo $RUNPERIOD | sed 's/_/-/'`

# echo "POS = $POS RUNPERIOD = $RUNPERIOD"
if ( $POS == 0 ) then
  echo "Usage:"
  echo "./create_project.sh [project name]"
  echo "project name MUST contain"
  echo "[RunPeriod2YYY_MM] and ver[NN] as substrings"
  exit
endif

# Set version name.
# This assumes that the version is contained
# in the project name as verXX
set POS = `echo $PROJECT | gawk '{print index($1,"ver")}'`
set POS = `expr $POS + 3`
set VERSION = `expr substr $PROJECT $POS 2`
# echo "POS = $POS VERSION = $VERSION"
if ( $POS == 0 ) then
  echo "Usage:"
  echo "./create_project.sh [project name]"
  echo "project name MUST contain"
  echo "[RunPeriod2YYY_MM] and ver[NN] as substrings"
  exit
endif

echo "-------------------------------------"
echo "Creating project : ${PROJECT}"
echo "for run period   : ${RUNPERIOD}"
echo "and version      : ${VERSION}"
echo "continue? (y/n)"
echo "-------------------------------------"

set REPLY = $<
while( $REPLY != "y" && $REPLY != "n")
  echo "Please reply y or n"
end

if($REPLY == "n" ) then
  echo "aborting...."
  exit
endif

# Create directory
set OUTDIR = "${PROJHOME}/${PROJECT}"

if ( -e $OUTDIR ) then
  echo "Directory $OUTDIR already exists......"
  exit
endif
mkdir -p $OUTDIR
mkdir -p $OUTDIR/analysis
# Instead of creating directory "processing", check out
# https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/process
cd $OUTDIR
echo "checking out https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/process...................."
svn co https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/process
cd -

# Create xml files
set XMLFILE = "/group/halld/data_monitoring/run_conditions/soft_comm_${RUNPERIOD}_ver${VERSION}.xml"
if ( -e $XMLFILE ) then
  echo "XMLFILE $XMLFILE already exists,"
  echo "conflict with previous launch?"
  echo "Manually delete the above file to create a new version"
  exit
endif

# Get correct environment variables so we can get svn vers.
source templates/setup_jlab-${RUNPERIOD_HYPHEN}.csh

echo
echo

# Get svn revision number to put in output file name
cd $HDDS_HOME
set hdds_ver = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "hdds revision      = ${hdds_ver}"
cd -

cd $HALLD_HOME
set sim_recon_ver = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "sim-recon revision = ${sim_recon_ver}"
cd -

# The variable ONLINEPLUGINSHOME is set within setup_jlab-[RUNPERIOD_HYPHEN].csh
# echo "ONLINEPLUGINSHOME = ${ONLINEPLUGINSHOME}"
cd ${ONLINEPLUGINSHOME}
set plugins_ver = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "plugins revision   = ${plugins_ver}"
cd -

set jana_ver   = `echo $JANA_HOME   | sed 's/.*jana_//'     | sed 's:/.*::'`
set evio_ver   = `echo $EVIOROOT    | sed 's/.*evio-//'     | sed 's:/.*::'`
set xerces_ver = `echo $XERCESCROOT | sed 's/.*xerces-c-//' | sed 's:\.Linux.*::'`
set root_ver = `echo $ROOTSYS | sed 's/.*root_//'`
set ccdb_ver = `echo $CCDB_HOME | sed 's/.*ccdb_//'`

echo "<gversions>" > $XMLFILE
echo "<package name="\""jana"\"" version="\""${jana_ver}"\""/>" >> $XMLFILE
echo "<package name="\""sim-recon"\"" version="\""${sim_recon_ver}"\""/>" >> $XMLFILE
echo "<package name="\""hdds"\"" version="\""${hdds_ver}"\""/>" >> $XMLFILE
echo "<package name="\""evio"\"" version="\""${evio_ver}"\""/>" >> $XMLFILE
echo "<package name="\""cernlib"\"" version="\""${CERN_LEVEL}"\"" word_length="\""64-bit"\""/>" >> $XMLFILE
echo "<package name="\""xerces-c"\"" version="\""${xerces_ver}"\""/>" >> $XMLFILE
echo "<package name="\""root"\"" version="\""${root_ver}"\""/>" >> $XMLFILE
echo "<package name="\""ccdb"\"" version="\""${ccdb_ver}"\""/>" >> $XMLFILE
echo "<package name="\""monitoring"\"" version="\""${plugins_ver}"\""/>" >> $XMLFILE
echo "<!---" >> $XMLFILE
echo "<package name="\""clhep"\"" version="\""2.0.4.5"\""/>" >> $XMLFILE
echo "<package name="\""geant4"\"" version="\""9.4"\""/>" >> $XMLFILE
echo "-->" >> $XMLFILE

set mem_requested  = `grep 'Memory space' templates/template.jsub | sed 's/.*space="//' | sed 's:".*::'`
echo "<variable name = "\""mem_requested"\"" value="\""${mem_requested}"\""/>" >> $XMLFILE

set diskspace  = `grep 'DiskSpace' templates/template.jsub | sed 's/.*space="//' | sed 's:".*::'`
echo "<variable name = "\""DiskSpace"\"" value="\""${diskspace}"\""/>" >> $XMLFILE

set ncores  = `grep 'CPU core' templates/template.jsub | sed 's/.*="//' | sed 's:".*::'`
echo "<variable name = "\""ncores"\"" value="\""${ncores}"\""/>" >> $XMLFILE

echo "</gversions>" >> $XMLFILE

echo "---------------------------------------------------------------------------------------------------------------"
echo "contents of XMLFILE : $XMLFILE"
echo "---------------------------------------------------------------------------------------------------------------"
cat $XMLFILE

# Create jana file
set JANAFILE = "/group/halld/data_monitoring/run_conditions/jana_rawdata_comm_${RUNPERIOD}_ver${VERSION}.conf"
if ( -e $JANAFILE ) then
  echo "JANAFILE $JANAFILE already exists,"
  echo "conflict with previous launch?"
  echo "Manually delete the above file to create a new version"
  exit
endif

set plugins        = `grep 'name="plugins"' templates/template.jsub     | sed 's/.*value="//'         | sed 's:"/>.*::'`
set nthreads       = `grep 'name="nthreads"' templates/template.jsub    | sed 's/.*value="//'         | sed 's:"/>.*::'`
set thread_timeout = `grep 'PTHREAD_TIMEOUT' templates/script.sh | sed 's/.*THREAD_TIMEOUT=//' | sed 's:-PPRINT_PLUGIN.*::'`
echo "-PPLUGINS=${plugins}" > $JANAFILE
echo "-PNTHREADS=${nthreads}" >> $JANAFILE
echo "-PTHREAD_TIMEOUT=${thread_timeout}" >> $JANAFILE
if ( $?JANA_CALIB_CONTEXT ) then
  echo "${JANA_CALIB_CONTEXT}" >> $JANAFILE
else
  echo "-PCALIB_CONTEXT="\"\" >> $JANAFILE
endif

echo "---------------------------------------------------------------------------------------------------------------"
echo "contents of JANAFILE : $JANAFILE"
echo "---------------------------------------------------------------------------------------------------------------"
cat $JANAFILE

#--------------------------------------------------------------------------------#
#---                Copy files that don't need modification                   ---#

# submission/jproj scripts
cp templates/script.sh                          ${OUTDIR}/
cp templates/setup_jlab-${RUNPERIOD_HYPHEN}.csh ${OUTDIR}/

### Sean's processing scripts
# cp templates/datamon_db.py                   ${OUTDIR}/process
# cp templates/check_new_runs.py               ${OUTDIR}/process
# cp templates/histograms_to_monitor           ${OUTDIR}/process
# cp templates/macros_to_monitor               ${OUTDIR}/process
# cp templates/make_monitoring_plots.py        ${OUTDIR}/process
# cp templates/process_monitoring_data.py      ${OUTDIR}/process
cp templates/run_process.sh                    ${OUTDIR}/process
# cp templates/register_new_version.py         ${OUTDIR}/process

### launch analysis scripts
cp templates/Makefile                        ${OUTDIR}/analysis
cp templates/format_jobs_data.cc             ${OUTDIR}/analysis
cp templates/analyze.C                       ${OUTDIR}/analysis
cp templates/mystyle.css                     ${OUTDIR}/analysis
#---                                                                          ---#
#--------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------#
#---                   Copy files that need modification                      ---#
# set date
set DATE  = `date +"%Y-%m-%d"`
set YEAR  = `date +"%Y"`
set MONTH = `date +"%m"`
set DAY   = `date +"%d"`

cp templates/template_clear.sh ${OUTDIR}/
cat ${OUTDIR}/template_clear.sh | sed "s:PROJHOME:${PROJHOME}:g" > ${OUTDIR}/clear.sh
rm -f ${OUTDIR}/template_clear.sh
chmod u+x ${OUTDIR}/clear.sh

cp templates/template_status.sh ${OUTDIR}/
cat ${OUTDIR}/template_status.sh | sed "s:PROJHOME:${PROJHOME}:g" > ${OUTDIR}/status.sh
rm -f ${OUTDIR}/template_status.sh
chmod u+x ${OUTDIR}/status.sh

cp templates/template_update_files.sh ${OUTDIR}/
cat ${OUTDIR}/template_update_files.sh | sed "s/PROJECT/${PROJECT}/g" | sed "s:PROJHOME:${PROJHOME}:g" > ${OUTDIR}/update_files.sh
rm -f ${OUTDIR}/template_update_files.sh
chmod u+x ${OUTDIR}/update_files.sh

# offline cron job
cp templates/template_cron_newruns ${OUTDIR}/
cat ${OUTDIR}/template_cron_newruns | sed "s/PROJECT/${PROJECT}/g" > ${OUTDIR}/cron_newruns
rm -f ${OUTDIR}/template_cron_newruns

cp templates/template_newruns.sh ${OUTDIR}/
cat ${OUTDIR}/template_newruns.sh | sed "s/PROJECT/${PROJECT}/g" > ${OUTDIR}/newruns.sh
chmod 775 ${OUTDIR}/newruns.sh
rm -f ${OUTDIR}/template_newruns.sh

# Sean's process scripts
rm -f ${OUTDIR}/process/monitoring_env.csh
cat ${OUTDIR}/process/template_monitoring_env.csh | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/PROJECT/${PROJECT}/" | sed "s:PROJHOME:${PROJHOME}:" > ${OUTDIR}/process/monitoring_env.csh
chmod 775 ${OUTDIR}/process/monitoring_env.csh
rm -f ${OUTDIR}/process/template_monitoring_env.csh

cp templates/template.jproj ${OUTDIR}/
cat ${OUTDIR}/template.jproj | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" > ${OUTDIR}/${PROJECT}.jproj
rm -f ${OUTDIR}/template.jproj

cp templates/template.jsub ${OUTDIR}/
cat ${OUTDIR}/template.jsub | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/g" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" | sed "s/USERNAME/${USER}/" | sed "s:PROJHOME:${PROJHOME}:" > ${OUTDIR}/${PROJECT}.jsub
rm -f ${OUTDIR}/template.jsub

rm -f ${OUTDIR}/process/check_monitoring_data.csh
cat ${OUTDIR}/process/template_check_monitoring_data.csh | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s:PROJHOME:${PROJHOME}:" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" > ${OUTDIR}/process/check_monitoring_data.csh
chmod 775 ${OUTDIR}/process/check_monitoring_data.csh
rm -f ${OUTDIR}/process/template_check_monitoring_data.csh

cat ${OUTDIR}/process/template_version_file.txt | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/RUN_PERIOD_HYPHEN/${RUNPERIOD}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" | sed "s/YEAR/${YEAR}/" | sed "s/MONTH/${MONTH}/" | sed "s/DAY/${DAY}/" > ${OUTDIR}/process/version_file.txt
chmod 775 ${OUTDIR}/process/version_file.txt
rm -f ${OUTDIR}/process/template_version_file.txt

cat ${OUTDIR}/process/template_cron_process.txt | sed "s/PROJECT/${PROJECT}/" | sed "s:PROJHOME:${PROJHOME}:" | sed "s/USERNAME/${USERNAME}/"  > ${OUTDIR}/process/cron_process.txt
rm -f ${OUTDIR}/process/template_cron_process.txt

# cp templates/template_process_run_conditions.py ${OUTDIR}/process/
# cat ${OUTDIR}/process/template_process_run_conditions.py | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/"  > ${OUTDIR}/process/process_run_conditions.py
# rm -f ${OUTDIR}/process/template_process_run_conditions.py

# cp templates/template_process_new_offline_data.py     ${OUTDIR}/process
# cat ${OUTDIR}/process/template_process_new_offline_data.py | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/"  > ${OUTDIR}/process/process_new_offline_data.py
# chmod 775 ${OUTDIR}/process/process_new_offline_data.py
# rm -f ${OUTDIR}/process/template_process_new_offline_data.py

# cp templates/template_check_new_runs.csh ${OUTDIR}/process/
# cat ${OUTDIR}/process/template_check_new_runs.csh | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s:PROJHOME:${PROJHOME}:" | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/process/check_new_runs.csh
# chmod 775 ${OUTDIR}/process/check_new_runs.csh
# rm -f ${OUTDIR}/process/template_check_new_runs.csh

# launch analysis scripts
cp templates/template_fill_jobIds_monAux.sql ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_fill_jobIds_monAux.sql | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/fill_jobIds_monAux.sql
chmod 775 ${OUTDIR}/analysis/fill_jobIds_monAux.sql
rm -f ${OUTDIR}/analysis/template_fill_jobIds_monAux.sql

cp templates/template_get_processed_job_info_from_stdout.sh ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_get_processed_job_info_from_stdout.sh | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s:PROJHOME:${PROJHOME}:" > ${OUTDIR}/analysis/get_processed_job_info_from_stdout.sh
chmod 775 ${OUTDIR}/analysis/get_processed_job_info_from_stdout.sh
rm -f ${OUTDIR}/analysis/template_get_processed_job_info_from_stdout.sh

cp templates/template_create_offline_monAux.sql ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_create_offline_monAux.sql | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/create_offline_monAux.sql
chmod 775 ${OUTDIR}/analysis/create_offline_monAux.sql
rm -f ${OUTDIR}/analysis/template_create_offline_monAux.sql

cp templates/template_write_inserts_aux.pl ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_write_inserts_aux.pl | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/write_inserts_aux.pl
chmod 775 ${OUTDIR}/analysis/write_inserts_aux.pl
rm -f ${OUTDIR}/analysis/template_write_inserts_aux.pl

cp templates/template_create_jobs_data_from_db.csh ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_create_jobs_data_from_db.csh | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/create_jobs_data_from_db.csh
chmod 775 ${OUTDIR}/analysis/create_jobs_data_from_db.csh
rm -f ${OUTDIR}/analysis/template_create_jobs_data_from_db.csh

cp templates/template_run_analysis.sh ${OUTDIR}/analysis
cat ${OUTDIR}/analysis/template_run_analysis.sh | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/run_analysis.sh
chmod 775 ${OUTDIR}/analysis/run_analysis.sh
rm -f ${OUTDIR}/analysis/template_run_analysis.sh

cp templates/template_results.html ${OUTDIR}/analysis
cat ${OUTDIR}/analysis/template_results.html | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/results.html
chmod 775 ${OUTDIR}/analysis/results.html
rm -f ${OUTDIR}/analysis/template_results.html
#---                                                                          ---#
#--------------------------------------------------------------------------------#
