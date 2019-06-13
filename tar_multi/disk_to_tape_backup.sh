#!/bin/bash
source_dir=$1 # directory name to be archive with full path
tar_multi=$2 # script to guide tar multi-volume archive making, extraction
size=$3 # maximum size of each tar file volume (suffix: G, M, or k)
cache_dir=/cache/halld/home/backups$source_dir
mkdir -pv $cache_dir
cp -v $tar_multi $cache_dir
tarname=$(basename $source_dir)
multiname=$(basename $tar_multi)
cd $source_dir
echo pwd = `pwd`
cd ..
echo pwd = `pwd`
command="tar cvf $cache_dir/${tarname}.tar -F $tar_multi --multi-volume -L $size $tarname"
echo $command
$command
cd $cache_dir
echo pwd = `pwd`
date > README
echo "To restore files:" >> README
echo "tar xvf $cache_dir/${tarname}.tar -F $cache_dir/$multiname --multi-volume" >> README
date > MANIFEST
echo $cache_dir >> MANIFEST
ls -l $cache_dir >> MANIFEST
echo tar file contents: >> MANIFEST
command="tar tvf $cache_dir/${tarname}.tar -F $tar_multi --multi-volume"
echo $command
$command >> MANIFEST
echo Listing of $cache_dir:
ls -l $cache_dir
