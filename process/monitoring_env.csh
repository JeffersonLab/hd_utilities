#!/bin/tcsh
# load standard environment
##source $HOME/setup_jlab.csh
#source $HOME/setup_jlab_commissioning.csh
#source /home/gxproj5/halld/jproj/projects/offline_monitoring_RunPeriod2015_03_ver09_hd_rawdata/setup_jlab-2015-03.csh

# need python 2.7 for MySQL bindings
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

# SET SOFTWARE VERSIONS/PATHS (e.g. $ROOTSYS, $CCDB_HOME, etc.)
setenv GLUEX_VERSION_XML $GLUEX_MYTOP/version_monitoring_launch_root6.xml
eval `$BUILD_SCRIPTS/version.pl $GLUEX_VERSION_XML`

# SET FULL ENVIRONMENT
source $BUILD_SCRIPTS/gluex_env.csh -v

# load ROOT python bindings
setenv PYTHONPATH $ROOTSYS/lib:$PYTHONPATH

# RCDB
#setenv RCDB_HOME $GLUEX_MYTOP/rcdb/$PROJECT_NAME/
#source $RCDB_HOME/environment.csh
