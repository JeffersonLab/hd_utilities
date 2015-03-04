#/bin/sh
# delete the project and start over
mysql -hhalldweb1 -ufarmer farming -e "drop table offmon_rp2014m10_v10_type1, offmon_rp2014m10_v10_type1Job"
jproj.pl offmon_rp2014m10_v10_type1 create
