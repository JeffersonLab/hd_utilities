#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table dc_02_1"
../../scripts/jproj.pl dc_02_1 create 9003 1000
