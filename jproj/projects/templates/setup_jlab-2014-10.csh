###############################################################
#
# 2014/12/09 Kei Moriya
#
# This file was copied from the file alias
# /home/gluex/setup_jlab_commissioning.csh
#
# Changes from original are:
# - HALLD_HOME and HDDS_HOME to refer to local builds.
# - JANA_CALIB_URL is set to /home/gxproj1/ccdb.sqlite, which
#    should be a soft link to the desired version.
# - JANA_PLUGIN_PATH is set to reflect gxproj1's build.
#
###############################################################

# Set versions of software to use here, they will
# be used to modify PATH and LD_LIBRARY_PATH when
# $BUILD_SCRIPTS/gluex_env.csh is sourced.
#
# Use common build for these
setenv BUILD_SCRIPTS /group/halld/Software/builds/build_scripts/build_scripts
setenv BMS_OSNAME `$BUILD_SCRIPTS/osrelease.pl`
setenv ROOTSYS     /group/halld/Software/builds/$BMS_OSNAME/root/root_5.34.26
setenv EVIOROOT    /group/halld/Software/builds/$BMS_OSNAME/evio/evio-4.3.1/Linux-x86_64
setenv XERCESCROOT /group/halld/Software/builds/$BMS_OSNAME/xerces-c/xerces-c-3.1.1

# I build these myself
setenv HALLD_HOME  /home/gxproj1/builds/sim-recon/sim-recon
setenv HDDS_HOME   /home/gxproj1/builds/hdds/hdds-commissioning
# CCDB must be built before JANA
# To get CCDB via svn, use
# svn co https://phys12svn.jlab.org/repos/trunk/ccdb
setenv CCDB_HOME   /home/gxproj1/builds/ccdb/ccdb_1.05
setenv JANA_HOME   /home/gxproj1/builds/jana/jana_0.7.2/$BMS_OSNAME
# setenv JANA_HOME   /group/halld/Software/builds/$BMS_OSNAME/jana/jana_0.7.2/$BMS_OSNAME

# `$BUILD_SCRIPTS/cue_root.pl`
setenv CERN_CUE `$BUILD_SCRIPTS/cue_cernlib.pl`
setenv CERN `$BUILD_SCRIPTS/cue_cernlib.pl`
setenv CERN_LEVEL 2005

setenv GLUEX_TOP /home/gxproj1/builds
setenv HALLD_MY $HOME/halld_my
# -----------------------------------------------------------------
# THIS SCRIPT WILL ADD MODIFY PATH AND LD_LIBRARY_PATH TO INCLUDE
#  ENVIRONMENT VARIABLES SET ABOVE
source $BUILD_SCRIPTS/gluex_env.csh
# -----------------------------------------------------------------

# Add 
# setenv LD_LIBRARY_PATH $EVIOROOT/lib:$LD_LIBRARY_PATH

# make sure to set the correct calibtime
setenv JANA_CALIB_URL  sqlite:////home/gxproj1/builds/ccdb.sqlite # mysql://ccdb_user@hallddb.jlab.org/ccdb
setenv CCDB_CONNECTION sqlite:////home/gxproj1/builds/ccdb.sqlite # mysql://ccdb_user@hallddb.jlab.org/ccdb
# /group/halld/www/halldweb/html/dist/ccdb.sqlite
# setenv JANA_CALIB_CONTEXT "calibtime=2015-02-11T00:00"
setenv JANA_RESOURCE_DIR /group/halld/www/halldweb/html/resources
if ( ! $?JANA_PLUGIN_PATH ) then
	setenv JANA_PLUGIN_PATH 
endif

# Set ONLINEPLUGINSHOME directory so that we can pick out the svn rev.
setenv ONLINEPLUGINSHOME ${HOME}/builds/online/packages/monitoring/src/plugins

if ( ! -e $ONLINEPLUGINSHOME ) then
  echo "Could not set ONLINEPLUGINSHOME to ${HOME}/builds/online/packages/monitoring/${BMS_OSNAME}/plugins"
  echo "Edit file setup_jlab-2014-10.csh so that directory with online plugins is set correctly"
  exit
endif

setenv JANA_PLUGIN_PATH ${ONLINEPLUGINSHOME}/../../${BMS_OSNAME}\:${HALLD_HOME}/${BMS_OSNAME}/plugins\:${JANA_HOME}/plugins/\:${HALLD_MY}/${BMS_OSNAME}/plugins

# setenv JANA_CALIB_CONTEXT "variation=mc"
# calibtime=2014-09-30"

# python2.7 needed for CCDB command line tool
setenv PATH /apps/python/PRO/bin\:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib\:$LD_LIBRARY_PATH

unsetenv HALLD_MY
