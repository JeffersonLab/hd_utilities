#!/bin/bash

#source /gluex/etc/hdonline.cshrc
source /gluex/software/gluex_RHEL95_top/gluex_env_boot.sh
#gxenv                                                                                                                                                                                                                                
#gxenv /gluonwork1/Users/sdobbs/version.xml
#source /gluex/software/gluex_RHEL95_top/gluex_env_local.sh /gluonwork1/Users/sdobbs/version.xml

export BUILD_SCRIPTS=/gluex/software/gluex_RHEL95_top/build_scripts
export BMS_OSNAME=`$BUILD_SCRIPTS/osrelease.pl`
export BUILD_SCRIPTS_CONSISTENCY_CHECK=false
export GLUEX_TOP=/gluex/software/gluex_RHEL95_top

export HALLD_VERSIONS=$GLUEX_TOP/halld_versions
source /gluex/software/gluex_RHEL95_top/build_scripts/gluex_env_version.sh /gluonwork1/Users/sdobbs/version.xml

unset HAVE_ET
unset ETROOT

export HALLD_MY=/gluonwork1/Users/sdobbs/Software/plugins

export JANA_RESOURCE_DIR=/gluonwork1/Users/sdobbs/work/resources
export JANA_CALIB_URL=mysql://ccdb_user@gluondb1.jlab.org/ccdb
export CCDB_CONNECTION=mysql://ccdb_user@gluondb1.jlab.org/ccdb

# for pyepics
export  PYTHONPATH=/gapps/pyepics/pyepics-3.2.1/lib/python2.7/site-packages:$PYTHONPATH

# for MYA
export PATH=/cs/certified//rhel-9-x86_64/bin:$PATH
export LD_LIBRARY_PATH=/cs/certified//rhel-9-x86_64/libexec:$LD_LIBRARY_PATH


# Read geometry from ccdb database by default
export JANA_GEOMETRY_URL=ccdb:///GEOMETRY/main_HDDS.xml

# calibration stuff
export NTHREADS=64
export CALIBRATION_TRAIN=/gluonwork1/Users/sdobbs/calibration_train
