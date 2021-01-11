#!/bin/bash
dir=$1
token=$2
temp_output_dir=/tmp/disk_report_$RANDOM
webdir=/group/halld/www/halldweb/html/disk_management

dmdir=/group/halld/Software/hd_utilities/disk_management
$dmdir/disk_database.pl $dir $token
mkdir -pv $temp_output_dir
$dmdir/disk_report.pl $token > $temp_output_dir/${token}_report.html
$dmdir/disk_report_users.pl $token $temp_output_dir

cp -pv $temp_output_dir/*.html $webdir/
