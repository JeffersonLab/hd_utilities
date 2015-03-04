#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table dc_02_4"
../../scripts/jproj.pl dc_02_4 create 9003 1000
