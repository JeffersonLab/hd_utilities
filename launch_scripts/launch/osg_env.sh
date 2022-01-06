#!/bin/bash

# python2.7 needed for CCDB command line tool - this is the version needed for the CentOS7 nodes
# export PATH=/apps/anaconda/PRO/bin:$PATH
# export LD_LIBRARY_PATH=/apps/anaconda/PRO/lib/python2.7:$LD_LIBRARY_PATH

# ALTERNATIVE HOME FOLDER
export HALLD_MY=/work/halld2/home/$USER/

# UNIQUE PROJECT NAME (used to prevent env overlaps)
export PROJECT_NAME=monitoring_launch #only used locally

# SET PATH FOR PERL MODULES (Does not seem to be automatically present when running jobs from cron)
#export PERL5LIB=/usr/local/lib64/perl5:/usr/local/share/perl5:/usr/lib64/perl5/vendor_perl:/usr/share/perl5/vendor_perl:/usr/lib64/perl5:/usr/share/perl5

# SET SOFTWARE VERSIONS/PATHS (e.g. $ROOTSYS, $CCDB_HOME, etc.)
export GLUEX_VERSION_XML=/work/halld2/home/$USER/builds/version_${PROJECT_NAME}.xml

# SET FULL ENVIRONMENT
export EDITOR=vim
export BUILD_SCRIPTS=/group/halld/Software/build_scripts/
source $BUILD_SCRIPTS/gluex_env_jlab.sh $GLUEX_VERSION_XML

# SETUP SQLITE & RESOURCES
export SQLITE_PATH=/work/halld2/home/$USER/sqlite/ccdb_${PROJECT_NAME}.sqlite #only used locally
export CCDB_CONNECTION=sqlite:///${SQLITE_PATH}
export JANA_CALIB_URL=sqlite:///${SQLITE_PATH}
export JANA_RESOURCE_DIR=/group/halld/www/halldweb/html/resources

# quick fix for rootcling compilation
unset CPLUS_INCLUDE_PATH

#try geometry from ccdb
export JANA_GEOMETRY_URL="ccdb:///GEOMETRY/main_HDDS.xml"
