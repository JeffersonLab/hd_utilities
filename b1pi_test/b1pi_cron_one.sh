#!/bin/bash
nevents=100000
date_file=/volatile/halld/$USER/b1pi_date.txt
if [ ! -f $date_file ] ; then
    echo date file $date_file not found, exiting
    exit 1
fi
date_token=`cat /volatile/halld/$USER/b1pi_date.txt`
export TODAYS_DATE=$date_token
export BMS_OSNAME=`/group/halld/Software/build_scripts/osrelease.pl`
export BUILD_DIR=/volatile/halld/gluex/nightly/$TODAYS_DATE/$BMS_OSNAME
export B1PI_TEST_DIR=/group/halld/Software/hd_utilities/b1pi_test
export BUILD_SCRIPTS=/group/halld/Software/build_scripts
export SEED=123

# Setup environment based on sim-recon build we're using 
source $BUILD_SCRIPTS/gluex_env_boot_jlab.sh
gxenv $BUILD_DIR/version_${TODAYS_DATE}.xml
#gxenv $HALLD_VERSIONS/version_5.21.1.xml
export JANA_CALIB_CONTEXT="variation=mc"

# do the test
export RUN=11366
export RUN_DIR=/volatile/halld/$USER/b1pi/$TODAYS_DATE/$BMS_OSNAME/$RUN
rm -rfv $RUN_DIR
mkdir -pv $RUN_DIR
cd $RUN_DIR
$B1PI_TEST_DIR/b1pi_test.sh -4 -n $nevents -r $RUN -t 8
echo \#count events
echo \#count b1_pi.hddm `python hddm_counter.py b1_pi.hddm`
echo \#count hdgeant.hddm `python hddm_counter.py hdgeant.hddm`
echo \#count hdgeant_smeared.hddm `python hddm_counter.py hdgeant_smeared.hddm`
echo \#count dana_rest.hddm `python hddm_counter.py dana_rest.hddm`
echo \#count dana_rest_b1pi.hddm `python hddm_counter.py dana_rest_b1pi.hddm`
export PLOTDIR=/group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME/Run$RUN
mkdir -pv -m755 $PLOTDIR
chmod 755 /group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME
cp -v *.pdf *.gif *.html $PLOTDIR
chmod 644 $PLOTDIR/*.*
#
export RUN=30480
export RUN_DIR=/volatile/halld/$USER/b1pi/$TODAYS_DATE/$BMS_OSNAME/$RUN
rm -rfv $RUN_DIR
mkdir -pv $RUN_DIR
cd $RUN_DIR
$B1PI_TEST_DIR/b1pi_test.sh -n $nevents -r $RUN -4 -t 8
echo \#count events
echo \#count b1_pi.hddm `python hddm_counter.py b1_pi.hddm`
echo \#count hdgeant.hddm `python hddm_counter.py hdgeant.hddm`
echo \#count hdgeant_smeared.hddm `python hddm_counter.py hdgeant_smeared.hddm`
echo \#count dana_rest.hddm `python hddm_counter.py dana_rest.hddm`
echo \#count dana_rest_b1pi.hddm `python hddm_counter.py dana_rest_b1pi.hddm`
export PLOTDIR=/group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME/Run$RUN
mkdir -pv -m755 $PLOTDIR
chmod 755 /group/halld/www/halldweb/html/b1pi/$TODAYS_DATE/$BMS_OSNAME
cp -v *.pdf *.gif *.html $PLOTDIR
chmod 644 $PLOTDIR/*.*
#
exit
