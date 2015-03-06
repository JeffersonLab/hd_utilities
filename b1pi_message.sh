#!/bin/sh
simple_dir=/group/halld/Software/scripts/simple_email_list/lists/b1pi
cd $simple_dir
rm -f message.txt
touch message.txt
date_token=`cat /u/scratch/$USER/b1pi_date.txt`
echo https://halldweb.jlab.org/b1pi/$date_token/ >> message.txt
../../scripts/simple_email_list.pl
