#!/bin/bash
script_dir=/u/scratch/marki/hd_utilities/cntr_equiv_test
dist=/group/halld/www/halldweb/html/dist
singularity_container=$dist/gluex_centos-7.7.1908_sng3.8_gxi2.30.sif
module use /apps/modulefiles
module load singularity
#
dir=native_native
rm -rf $dir
mkdir $dir
pushd $dir
$script_dir/bp_cntr_test.sh
popd
#
dir=native_cntr
rm -rf $dir
mkdir $dir
pushd $dir
$script_dir/bp_cntr_test.sh Linux_CentOS7-x86_64-gcc4.8.5-cntr
unset BMS_OSNAME_OVERRIDE
popd
#
dir=cntr_native
rm -rf $dir
mkdir $dir
pushd $dir
singularity exec --cleanenv --bind /group/halld --bind $script_dir $singularity_container $script_dir/bp_cntr_test.sh Linux_CentOS7.7-x86_64-gcc4.8.5
popd
#
dir=cntr_cntr
rm -rf $dir
mkdir $dir
pushd $dir
singularity exec --cleanenv --bind /group/halld --bind $script_dir $singularity_container $script_dir/bp_cntr_test.sh
popd
