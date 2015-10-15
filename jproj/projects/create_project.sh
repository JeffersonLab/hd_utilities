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
  # exit
endif
mkdir -p $OUTDIR/analysis

#--------------------------------------------------------------------------------#
#---                Copy files that don't need modification                   ---#
### launch analysis scripts
cp templates/create_jproj_job_table.py          ${OUTDIR}/analysis
cp templates/Makefile                        ${OUTDIR}/analysis
cp templates/format_jobs_data.cc             ${OUTDIR}/analysis
cp templates/analyze.C                       ${OUTDIR}/analysis
cp templates/mystyle.css                     ${OUTDIR}/analysis
#---                                                                          ---#
#--------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------#
#---                   Copy files that need modification                      ---#

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
