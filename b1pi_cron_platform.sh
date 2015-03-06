#!/bin/bash
date_token=`cat /u/scratch/$USER/b1pi_date.txt`
export TODAYS_DATE=$date_token
export BUILD_DIR=/u/scratch/gluex/nightly/$TODAYS_DATE
echo DEBUG MODE
export SCRIPTS=/group/halld/Software/scripts
export BMS_OSNAME=`$SCRIPTS/build_scripts/osrelease.pl`
export RUN_DIR=/u/scratch/$USER/b1pi/$TODAYS_DATE/$BMS_OSNAME

# Setup environment based on sim-recon build we're using 
source ${BUILD_DIR}/sim-recon/$BMS_OSNAME/setenv.sh
export JANA_CALIB_CONTEXT="variation=mc"

# do the test
rm -rf $RUN_DIR
mkdir -pv $RUN_DIR
cd $RUN_DIR
cp -pr $SCRIPTS/b1pi_test/* .
cp -pr $SCRIPTS/b1pi_test/macros/* .
./mkevents.sh
export PLOTDIR=/group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME
mkdir -pv $PLOTDIR
cp -v *.pdf *.gif *.html $PLOTDIR
cp -v ../../../b1pi*.log $PLOTDIR/../
exit
