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

# preliminaries
setenv BUILD_SCRIPTS /group/halld/Software/builds/build_scripts/build_scripts_detcom01
setenv BMS_OSNAME `$BUILD_SCRIPTS/osrelease.pl`
# gluex software versions
setenv CCDB_HOME /group/halld/Software/builds/ccdb/$BMS_OSNAME/ccdb_1.03
setenv HALLD_HOME /home/gxproj1/builds/sim-recon/sim-recon-commissioning
setenv HDDS_HOME  /home/gxproj1/builds/hdds/hdds-commissioning
setenv JANA_HOME //group/halld/Software/builds/jana/jana_0.7.2/$BMS_OSNAME
setenv ROOTSYS `$BUILD_SCRIPTS/cue_root.pl`
setenv CERN_CUE `$BUILD_SCRIPTS/cue_cernlib.pl`
setenv CERN `$BUILD_SCRIPTS/cue_cernlib.pl`
setenv CERN_LEVEL 2005
setenv EVIOROOT /group/halld/Software/builds/evio/evio-4.3.1/Linux-x86_64
setenv XERCESCROOT /group/halld/Software/ExternalPackages/xerces-c-3.1.1.$BMS_OSNAME
source $BUILD_SCRIPTS/gluex_env.csh
setenv LD_LIBRARY_PATH $EVIOROOT/lib:$LD_LIBRARY_PATH

# make sure to set the correct calibtime
setenv JANA_CALIB_URL sqlite:////home/gxproj1/ccdb.sqlite # group/halld/www/halldweb1/html/dist/ccdb.sqlite
setenv CCDB_CONNECTION sqlite:////home/gxproj1/ccdb.sqlite
#setenv JANA_CALIB_CONTEXT "calibtime=2015-01-30T00:00"
setenv JANA_RESOURCE_DIR /group/halld/www/halldweb1/html/resources
if ( ! $?JANA_PLUGIN_PATH ) then
	setenv JANA_PLUGIN_PATH 
endif
setenv JANA_PLUGIN_PATH /home/gxproj1/builds/online/packages/monitoring/${BMS_OSNAME}/plugins\:${HALLD_HOME}/${BMS_OSNAME}/plugins\:${JANA_HOME}/plugins/

# setenv JANA_CALIB_CONTEXT "variation=mc"
# calibtime=2014-09-30"

# python2.7 needed for CCDB command line tool
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

unsetenv HALLD_MY
