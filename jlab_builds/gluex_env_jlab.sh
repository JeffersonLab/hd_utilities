#!/bin/bash
if [ -z $1 ]
    then
    VERSION_XML=/group/halld/www/halldweb/html/dist/version_jlab.xml
else
    VERSION_XML=$1
fi
# farm-specific set-up
export JLAB_BUILDS=/group/halld/Software/hd_utilities/jlab_builds
. $JLAB_BUILDS/jlab_tricks.sh
export BUILD_SCRIPTS=/group/halld/Software/build_scripts
export BMS_OSNAME=`$BUILD_SCRIPTS/osrelease.pl`
export GLUEX_TOP=/group/halld/Software/builds/$BMS_OSNAME
# finish the rest of the environment
. $BUILD_SCRIPTS/gluex_env_version.sh $VERSION_XML
export JANA_CALIB_URL=$CCDB_CONNECTION
export JANA_RESOURCE_DIR=/group/halld/www/halldweb/html/resources
