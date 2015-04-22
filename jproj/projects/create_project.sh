#!/bin/tcsh

#----------------------------------------------------------------------
#
# This script will create a new project and its
# directories from the template directory.
#
# Usage:
# ./create_project.sh [project name] [file type]
#
# 1. [project name] MUST include substring RunPeriod2YYY-MM
# 2. [file type] is prefix of raw data files to be searched for
#
#----------------------------------------------------------------------

set PROJECT = $1
set FILETYPE = $2

if ( $PROJECT == "" || $FILETYPE == "" ) then
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
echo "POS = $POS RUNPERIOD = $RUNPERIOD"
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
echo "POS = $POS VERSION = $VERSION"
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
set OUTDIR = "/home/gxproj1/halld/jproj/projects/${PROJECT}"

if ( -e $OUTDIR ) then
  echo "Directory $OUTDIR already exists......"
  exit
endif
mkdir -p $OUTDIR
mkdir -p $OUTDIR/processing
mkdir -p $OUTDIR/analysis

# Need to have GLUEX variables set to get info
# on versions of software
source /home/gxproj1/setup_jlab.csh

# Create xml files
set XMLFILE = "/group/halld/data_monitoring/run_conditions/soft_comm_${RUNPERIOD}_ver${VERSION}.xml"
if ( -e $XMLFILE ) then
  rm -f $XMLFILE
endif

# Get svn revision number to put in output file name
cd $HDDS_HOME
set hdds_ver = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "hdds revision      = ${hdds_ver}"
cd -

cd $HALLD_HOME
set sim_recon_ver = `svn info | grep 'Revision' | gawk '{print $2}'`
echo "sim-recon revision = ${sim_recon_ver}"
cd -

cd /home/gxproj1/builds/online/packages/monitoring/src/plugins
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



# Create jana file
set JANAFILE = "/group/halld/data_monitoring/run_conditions/jana_rawdata_comm_${RUNPERIOD}_ver${VERSION}.conf" # ${YEAR}_${MONTH}_${DAY}
echo $JANAFILE
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

#--------------------------------------------------------------------------------#
#---                Copy files that don't need modification                   ---#

# submission/jproj scripts
cp /home/gxproj1/halld/jproj/projects/templates/clear.sh                        ${OUTDIR}/
cp /home/gxproj1/halld/jproj/projects/templates/script.sh                       ${OUTDIR}/
cp /home/gxproj1/halld/jproj/projects/templates/status.sh                       ${OUTDIR}/
# note: this is not from templates directory, but the one in home directory.
# This ensures consistency with what gxproj1 sees.
cp /home/gxproj1/setup_jlab.csh                                                 ${OUTDIR}/

# Sean's processing scripts
cp /home/gxproj1/halld/jproj/projects/templates/monitoring_env.csh              ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/datamon_db.py                   ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/histograms_to_monitor           ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/macros_to_monitor               ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/make_monitoring_plots.py        ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/process_monitoring_data.py      ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/run_processing.sh               ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/register_new_version.py         ${OUTDIR}/processing
# launch analysis scripts
cp /home/gxproj1/halld/jproj/projects/templates/Makefile                        ${OUTDIR}/analysis
cp /home/gxproj1/halld/jproj/projects/templates/format_jobs_data.cc             ${OUTDIR}/analysis
cp /home/gxproj1/halld/jproj/projects/templates/analyze.C                       ${OUTDIR}/analysis
cp /home/gxproj1/halld/jproj/projects/templates/mystyle.css                     ${OUTDIR}/analysis
#---                                                                          ---#
#--------------------------------------------------------------------------------#



#--------------------------------------------------------------------------------#
#---                   Copy files that need modification                      ---#
# set date
set DATE  = `date +"%Y-%m-%d"`
set YEAR  = `date +"%Y"`
set MONTH = `date +"%m"`
set DAY   = `date +"%d"`
# Since the jproj file is looking for data files, we need to have the
# run period be 2YYY-MM instead of 2YYY_MM
set RUNPERIOD_HYPHEN = `echo $RUNPERIOD | sed 's/_/-/'`

# Sean's processing scripts
cp /home/gxproj1/halld/jproj/projects/templates/template.jproj ${OUTDIR}/
cat ${OUTDIR}/template.jproj | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/FILETYPE/${FILETYPE}/" > ${OUTDIR}/${PROJECT}.jproj
rm -f ${OUTDIR}/template.jproj

cp /home/gxproj1/halld/jproj/projects/templates/template.jsub ${OUTDIR}/
cat ${OUTDIR}/template.jsub | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" > ${OUTDIR}/${PROJECT}.jsub
rm -f ${OUTDIR}/template.jsub

cp /home/gxproj1/halld/jproj/projects/templates/template_check_monitoring_data.csh ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template_check_monitoring_data.csh | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" > ${OUTDIR}/processing/check_monitoring_data.csh
chmod 775 ${OUTDIR}/processing/check_monitoring_data.csh
rm -f ${OUTDIR}/processing/template_check_monitoring_data.csh

cp /home/gxproj1/halld/jproj/projects/templates/template_version_file.txt ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template_version_file.txt | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" | sed "s/YEAR/${YEAR}/" | sed "s/MONTH/${MONTH}/" | sed "s/DAY/${DAY}/" > ${OUTDIR}/processing/version_file.txt
chmod 775 ${OUTDIR}/processing/version_file.txt
rm -f ${OUTDIR}/processing/template_version_file.txt

cp /home/gxproj1/halld/jproj/projects/templates/template_cron_processing.txt ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template_cron_processing.txt | sed "s/PROJECT/${PROJECT}/"  > ${OUTDIR}/processing/cron_processing.txt
rm -f ${OUTDIR}/processing/template_cron_processing.txt

cp /home/gxproj1/halld/jproj/projects/templates/template_process_run_conditions.py ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template_process_run_conditions.py | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/"  > ${OUTDIR}/processing/process_run_conditions.py
rm -f ${OUTDIR}/processing/template_process_run_conditions.py

cp /home/gxproj1/halld/jproj/projects/templates/template_process_new_offline_data.py     ${OUTDIR}/processing
cat ${OUTDIR}/processing/template_process_new_offline_data.py | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/"  > ${OUTDIR}/processing/process_new_offline_data.py
rm -f ${OUTDIR}/processing/template_process_new_offline_data.py

# launch analysis scripts
cp /home/gxproj1/halld/jproj/projects/templates/template_fill_jobIds_monAux.sql ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_fill_jobIds_monAux.sql | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/fill_jobIds_monAux.sql
chmod 775 ${OUTDIR}/analysis/fill_jobIds_monAux.sql
rm -f ${OUTDIR}/analysis/template_fill_jobIds_monAux.sql

cp /home/gxproj1/halld/jproj/projects/templates/template_get_processed_job_info_from_stdout.sh ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_get_processed_job_info_from_stdout.sh | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" > ${OUTDIR}/analysis/get_processed_job_info_from_stdout.sh
chmod 775 ${OUTDIR}/analysis/get_processed_job_info_from_stdout.sh
rm -f ${OUTDIR}/analysis/template_get_processed_job_info_from_stdout.sh

cp /home/gxproj1/halld/jproj/projects/templates/template_create_offline_monAux.sql ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_create_offline_monAux.sql | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/create_offline_monAux.sql
chmod 775 ${OUTDIR}/analysis/create_offline_monAux.sql
rm -f ${OUTDIR}/analysis/template_create_offline_monAux.sql

cp /home/gxproj1/halld/jproj/projects/templates/template_write_inserts_aux.pl ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_write_inserts_aux.pl | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/write_inserts_aux.pl
chmod 775 ${OUTDIR}/analysis/write_inserts_aux.pl
rm -f ${OUTDIR}/analysis/template_write_inserts_aux.pl

cp /home/gxproj1/halld/jproj/projects/templates/template_create_jobs_data_from_db.csh ${OUTDIR}/analysis/
cat ${OUTDIR}/analysis/template_create_jobs_data_from_db.csh | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/create_jobs_data_from_db.csh
chmod 775 ${OUTDIR}/analysis/create_jobs_data_from_db.csh
rm -f ${OUTDIR}/analysis/template_create_jobs_data_from_db.csh

cp /home/gxproj1/halld/jproj/projects/templates/template_run_analysis.sh ${OUTDIR}/analysis
cat ${OUTDIR}/analysis/template_run_analysis.sh | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/run_analysis.sh
chmod 775 ${OUTDIR}/analysis/run_analysis.sh
rm -f ${OUTDIR}/analysis/template_run_analysis.sh

cp /home/gxproj1/halld/jproj/projects/templates/template_results.html ${OUTDIR}/analysis
cat ${OUTDIR}/analysis/template_results.html | sed "s/PROJECT/${PROJECT}/" > ${OUTDIR}/analysis/results.html
chmod 775 ${OUTDIR}/analysis/results.html
rm -f ${OUTDIR}/analysis/template_results.html
#---                                                                          ---#
#--------------------------------------------------------------------------------#
