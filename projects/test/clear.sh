#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table test; drop table testJob"
../../scripts/jproj.pl test create 1234 10
