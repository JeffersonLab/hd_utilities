#!/bin/tcsh

###########################################################################################
#
# 2014/12/09 Kei Moriya
#
# Script that will set up GlueX environment for user gxproj1
# based on xml file that was created for previous weekly launch.
#
# -----------------------------------------------------------------------
#
# Usage: ./setup.sh [YEAR] [MONTH] [DAY]
#        Dates should be for previous Friday when weekly jobs were run.
#
# -----------------------------------------------------------------------
#
# The environment should be set to the previous weekly run settings.
# The necessary information is within
# /work/halld/data_monitoring/run_conditions/soft_comm_${YEAR}_${MONTH}_${DAY}.xml
#
###########################################################################################

set YEAR  = $1
set MONTH = $2
set DAY   = $3
# echo "DATE = ${YEAR}_${MONTH}_${DAY}"

if ( $YEAR == "" || $MONTH == "" || $DAY == "" ) then
  echo "must specify YEAR, MONTH, DAY of previous weekly run"
  exit
endif

### Set the sqlite file and context
### This will override the date specified in
### /home/gxproj1/setup_jlab.csh
set JANA_CALIB_CONTEXT = "calibtime=${YEAR}-${MONTH}-${DAY}T00:00"
set JANA_CALIB_URL = "sqlite:////group/halld/www/halldweb1/html/dist/ccdb.sqlite"

### Get xml file containing previous svn build info
set XMLFILE = "/work/halld/data_monitoring/run_conditions/soft_comm_${YEAR}_${MONTH}_${DAY}.xml"

set svn_hdds       = `grep 'hdds' ${XMLFILE}  | sed 's/.*commissioning.//' | sed 's:"/>::'`
echo "hdds rev.       = $svn_hdds"

set svn_sim_recon  = `grep 'sim-recon' ${XMLFILE}  | sed 's/.*commissioning.//' | sed 's:"/>::'`
echo "sim-recon rev.  = $svn_sim_recon"

set svn_monitoring = `grep 'monitoring' ${XMLFILE}  | sed 's/.*="//' | sed 's:"/>::'`
echo "monitoring rev. = $svn_monitoring"

### Check out svn and build for each package

# 1. check out HDDS
mkdir -p /home/gxproj1/builds/hdds
cd /home/gxproj1/builds/hdds/
svn co -r ${svn_hdds} https://halldsvn.jlab.org/repos/branches/hdds-commissioning/
cd hdds-commissioning/
echo "installing HDDS......................."
time scons install -j4

# 2. check out sim-recon
mkdir -p /home/gxproj1/builds/sim-recon
cd /home/gxproj1/builds/sim-recon/
svn co -r ${svn_sim_recon} https://halldsvn.jlab.org/repos/branches/sim-recon-commissioning/
cd sim-recon-commissioning/src/
echo "installing sim-recon.................."
# time scons install -j4

# 3. check out monitoring plugins
mkdir -p /home/gxproj1/builds/
cd /home/gxproj1/builds/
svn co -r ${svn_monitoring} https://halldsvn.jlab.org/repos/trunk/online
cd online/packages/monitoring/src/plugins/
echo "installing plugins...................."
time scons -u install -j4
