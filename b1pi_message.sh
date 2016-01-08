#!/bin/sh
simple_dir=/group/halld/Software/scripts/simple_email_list/lists/b1pi
cd $simple_dir
rm -f message.txt
touch message.txt
date_token=`cat /u/scratch/$USER/b1pi_date.txt`
echo https://halldweb.jlab.org/b1pi/$date_token/ >> message.txt
ls /group/halld/www/halldweb/html/b1pi/$date_token/b1pi_*.log | awk ' split($0, t, "b1pi_|.log") { system("echo " $0 ";" "grep \"#count\" " $0)} '
../../scripts/simple_email_list.pl
