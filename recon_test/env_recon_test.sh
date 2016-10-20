#!/bin/bash

export MONITORING_HOME=/group/halld/Software/scripts/monitoring/

# SET /apps/bin AT FRONT OF PATH (to use working version of git)
export PATH=/apps/bin:$PATH

# PYTHON
export PATH=/apps/python/PRO/bin:$PATH
export LD_LIBRARY_PATH=/apps/python/PRO/lib:$LD_LIBRARY_PATH

# SET PATH FOR PERL MODULES (Does not seem to be automatically present when running jobs from cron)
export PERL5LIB=/usr/local/lib64/perl5:/usr/local/share/perl5:/usr/lib64/perl5/vendor_perl:/usr/share/perl5/vendor_perl:/usr/lib64/perl5:/usr/share/perl5

# INITIALIZE
export BUILD_SCRIPTS=/group/halld/Software/build_scripts/
export BMS_OSNAME=`${BUILD_SCRIPTS}/osrelease.pl`
export EDITOR=vim

# SET SOFTWARE VERSIONS/PATHS (e.g. $ROOTSYS, $CCDB_HOME, etc.)
export GLUEX_TOP=/group/halld/Software/builds/$BMS_OSNAME/
export DATE=`date +%Y-%m-%d`
export GLUEX_VERSION_XML=/u/scratch/gluex/nightly/${DATE}/${BMS_OSNAME}/version_${DATE}.xml
eval `$BUILD_SCRIPTS/version.pl -s bash $GLUEX_VERSION_XML`

# SET FULL ENVIRONMENT
source $BUILD_SCRIPTS/gluex_env_jlab.sh

# SETUP SQLITE & RESOURCES
export SQLITE_PATH=ccdb.sqlite #only used locally
export CCDB_CONNECTION=sqlite://${SQLITE_PATH}
export JANA_CALIB_URL=sqlite://${SQLITE_PATH}
export JANA_RESOURCE_DIR=/group/halld/www/halldweb/html/resources

# quick fix for rootcling compilation
unset CPLUS_INCLUDE_PATH
