#/bin/sh
# delete the project and start over
set PROJECT = `basename $PWD`
echo "clearing tables ${PROJECT}, ${PROJECT}Job.................."
mysql -hhallddb -ufarmer farming -e "drop table ${PROJECT}, ${PROJECT}Job"
echo "clearing table ${PROJECT}.................."
jproj.pl ${PROJECT} create
