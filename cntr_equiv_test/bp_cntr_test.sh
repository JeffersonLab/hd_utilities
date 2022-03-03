#!/bin/bash
if [ ! -z $1 ]
then
    export BMS_OSNAME_OVERRIDE=$1
fi
source /group/halld/Software/build_scripts/gluex_env_boot_jlab.sh
gxenv $HALLD_VERSIONS/version_5.4.1.xml
which hdgeant4 > which_hdgeant4.txt
bpt=$HD_UTILITIES_HOME/b1pi_test
$bpt/my_b1pi_test.sh -d $bpt -n 1000 -4 -s 123


