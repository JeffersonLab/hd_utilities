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

#--------------------------------------------------------------------------------#
#---                Copy files that don't need modification                   ---#

# submission/jproj scripts
cp /home/gxproj1/halld/jproj/projects/templates/clear.sh                        ${OUTDIR}/
cp /home/gxproj1/halld/jproj/projects/templates/script.sh                       ${OUTDIR}/
cp /home/gxproj1/halld/jproj/projects/templates/setup_jlab.csh                  ${OUTDIR}/
cp /home/gxproj1/halld/jproj/projects/templates/status.sh                       ${OUTDIR}/

# Sean's processing scripts
cp /home/gxproj1/halld/jproj/projects/templates/monitoring_env.csh              ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/process_new_offline_data.py     ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/datamon_db.py                   ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/histograms_to_monitor           ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/macros_to_monitor               ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/make_monitoring_plots.py        ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/process_monitoring_data.py      ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/process_run_conditions.py       ${OUTDIR}/processing
cp /home/gxproj1/halld/jproj/projects/templates/run_processing.sh               ${OUTDIR}/processing

# launch analysis scripts
cp /home/gxproj1/halld/jproj/projects/templates/format_jobs_data.C              ${OUTDIR}/analysis
cp /home/gxproj1/halld/jproj/projects/templates/analyze.C                       ${OUTDIR}/analysis
cp /home/gxproj1/halld/jproj/projects/templates/run_analysis.sh                 ${OUTDIR}/analysis
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
cp /home/gxproj1/halld/jproj/projects/templates/template.jproj ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template.jproj | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/FILETYPE/${FILETYPE}/" > ${OUTDIR}/processing/${PROJECT}.jproj
rm -f ${OUTDIR}/processing/template.jproj

cp /home/gxproj1/halld/jproj/projects/templates/template.jsub ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template.jsub | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/FILETYPE/${FILETYPE}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" > ${OUTDIR}/processing/${PROJECT}.jsub
rm -f ${OUTDIR}/processing/template.jsub

cp /home/gxproj1/halld/jproj/projects/templates/template_check_monitoring_data.csh ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template_check_monitoring_data.csh | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/FILETYPE/${FILETYPE}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" > ${OUTDIR}/processing/check_monitoring_data.csh
chmod 775 ${OUTDIR}/processing/check_monitoring_data.csh
rm -f ${OUTDIR}/processing/template_check_monitoring_data.csh

cp /home/gxproj1/halld/jproj/projects/templates/template_version_file.txt ${OUTDIR}/processing/
cat ${OUTDIR}/processing/template_version_file.txt | sed "s/RUNPERIOD/${RUNPERIOD_HYPHEN}/" | sed "s/FILETYPE/${FILETYPE}/" | sed "s/PROJECT/${PROJECT}/" | sed "s/VERSION/${VERSION}/" | sed "s/YEAR/${YEAR}/" | sed "s/MONTH/${MONTH}/" | sed "s/DAY/${DAY}/" > ${OUTDIR}/processing/version_file.txt
chmod 775 ${OUTDIR}/processing/version_file.txt
rm -f ${OUTDIR}/processing/template_version_file.txt

# launch analysis scripts
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
#---                                                                          ---#
#--------------------------------------------------------------------------------#
