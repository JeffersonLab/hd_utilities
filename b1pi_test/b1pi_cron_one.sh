#!/bin/bash
date_file=/u/scratch/$USER/b1pi_date.txt
if [ ! -f $date_file ] ; then
    echo date file $date_file not found, exiting
    exit 1
fi
date_token=`cat /u/scratch/$USER/b1pi_date.txt`
export TODAYS_DATE=$date_token
export BMS_OSNAME=`/group/halld/Software/build_scripts/osrelease.pl`
export BUILD_DIR=/u/scratch/gluex/nightly/$TODAYS_DATE/$BMS_OSNAME
export B1PI_TEST_DIR=/group/halld/Software/scripts/b1pi_test
export BUILD_SCRIPTS=/group/halld/Software/build_scripts

# Setup environment based on sim-recon build we're using 
source $BUILD_SCRIPTS/gluex_env_boot_jlab.sh
gxenv $BUILD_DIR/version_${TODAYS_DATE}.xml
export JANA_CALIB_CONTEXT="variation=mc"

# do the test
export RUN=11366
export RUN_DIR=/u/scratch/$USER/b1pi/$TODAYS_DATE/$BMS_OSNAME/$RUN
rm -rfv $RUN_DIR
mkdir -pv $RUN_DIR
cd $RUN_DIR
$B1PI_TEST_DIR/b1pi_test.sh -n 10000 -r $RUN
echo \#count events
echo \#count b1_pi.hddm `hddm_counter.pl b1_pi.hddm physicsEvent`
echo \#count hdgeant.hddm `hddm_counter.pl hdgeant.hddm physicsEvent`
echo \#count hdgeant_smeared.hddm `hddm_counter.pl hdgeant_smeared.hddm physicsEvent`
echo \#count dana_rest.hddm `hddm_counter.pl dana_rest.hddm reconstructedPhysicsEvent`
echo \#count dana_rest_b1pi.hddm `hddm_counter.pl dana_rest_b1pi.hddm reconstructedPhysicsEvent`
export PLOTDIR=/group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME/Run$RUN
mkdir -pv $PLOTDIR
cp -v *.pdf *.gif *.html $PLOTDIR
#
export RUN=30480
export RUN_DIR=/u/scratch/$USER/b1pi/$TODAYS_DATE/$BMS_OSNAME/$RUN
rm -rfv $RUN_DIR
mkdir -pv $RUN_DIR
cd $RUN_DIR
$B1PI_TEST_DIR/b1pi_test.sh -n 10000 -r $RUN
echo \#count events
echo \#count b1_pi.hddm `hddm_counter.pl b1_pi.hddm physicsEvent`
echo \#count hdgeant.hddm `hddm_counter.pl hdgeant.hddm physicsEvent`
echo \#count hdgeant_smeared.hddm `hddm_counter.pl hdgeant_smeared.hddm physicsEvent`
echo \#count dana_rest.hddm `hddm_counter.pl dana_rest.hddm reconstructedPhysicsEvent`
echo \#count dana_rest_b1pi.hddm `hddm_counter.pl dana_rest_b1pi.hddm reconstructedPhysicsEvent`
export PLOTDIR=/group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME/Run$RUN
mkdir -pv $PLOTDIR
cp -v *.pdf *.gif *.html $PLOTDIR

exit
