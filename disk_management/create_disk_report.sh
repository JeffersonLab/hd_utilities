#!/bin/bash
dir=$1
token=$2
temp_output_dir=/tmp/disk_report_$RANDOM
webdir=/group/halld/www/halldweb/html/disk_management

dmdir=/group/halld/Software/hd_utilities/disk_management
$dmdir/disk_database.pl $dir $token
mkdir -pv $temp_output_dir
cd $temp_output_dir
$dmdir/disk_report.pl -n 20 -m 50 $token

cp -pv $temp_output_dir/*.html $webdir/
