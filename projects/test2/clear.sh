#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table test2, test2Job"
jproj.pl test2 create
