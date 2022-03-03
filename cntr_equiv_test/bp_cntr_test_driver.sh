#!/bin/bash
dist=/group/halld/www/halldweb/html/dist
module use /apps/modulefiles
module load singularity
#
dir=native_native
rm -rf $dir
mkdir $dir
pushd $dir
/u/scratch/marki/bp_cntr_test.sh
popd
#
dir=native_cntr
rm -rf $dir
mkdir $dir
pushd $dir
/u/scratch/marki/bp_cntr_test.sh Linux_CentOS7-x86_64-gcc4.8.5-cntr
unset BMS_OSNAME_OVERRIDE
popd
#
dir=cntr_native
rm -rf $dir
mkdir $dir
pushd $dir
singularity exec --cleanenv --bind /group/halld --bind /u/scratch/marki $dist/gluex_centos-7.7.1908_sng3.8_gxi2.22.sif /u/scratch/marki/bp_cntr_test.sh Linux_CentOS7.7-x86_64-gcc4.8.5
popd
#
dir=cntr_cntr
rm -rf $dir
mkdir $dir
pushd $dir
singularity exec --cleanenv --bind /group/halld --bind /u/scratch/marki $dist/gluex_centos-7.7.1908_sng3.8_gxi2.22.sif /u/scratch/marki/bp_cntr_test.sh
popd
