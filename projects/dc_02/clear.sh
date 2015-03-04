#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table dc_02"
../../scripts/jproj.pl dc_02 create
../../scripts/jproj.pl dc_02 update
