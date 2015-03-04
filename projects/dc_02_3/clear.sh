#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table dc_02_3"
../../scripts/jproj.pl dc_02_3 create 9003 1000
