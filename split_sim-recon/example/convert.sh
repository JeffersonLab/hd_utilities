#!/bin/sh
target=$1
replacement=$2
file=$3
echo target = $target
echo replacement = $replacement
echo file = $file
mv -v $file ${file}.tmp
sed s/$target/$replacement/g ${file}.tmp > $file
rm -v ${file}.tmp
