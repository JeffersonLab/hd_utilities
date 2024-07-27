#!/bin/sh
hosts="ifarm1901 sandd1 ifarm9"
date_token=`date +%F`
date_file=/volatile/halld/$USER/b1pi_date.txt
rm -fv $date_file
echo $date_token > $date_file
RUN_DIR=/volatile/halld/$USER/b1pi/$date_token
mkdir -pv $RUN_DIR
webdir=/group/halld/www/halldweb/html/b1pi/$date_token
mkdir -pv -m755 $webdir
for host in $hosts
do
    echo host = $host
    logfile=/volatile/halld/$USER/b1pi_$host.log
    echo logfile = $logfile \(https://halldweb.jlab.org/b1pi/$date_token/b1pi_${host}.log\)
    rm -fv $logfile
    # the following ssh executes the command associated with the
    # /home/gluex/.ssh/b1pi_cron_rsa.pub key in
    # /home/gluex/.ssh/authorized_keys. That command should be
    # /group/halld/Software/scripts/b1pi_test/b1pi_cron_one.sh
    ( \
    env -u SSH_AUTH_SOCK ssh -i ~/.ssh/b1pi_cron_alma9 $host > $logfile 2>&1 \
    ; \
    mv -v $logfile $webdir \
    ) &
done
nprocs=999
while [ $nprocs -ne 0 ]
    do
    nprocs=`ps aux | grep /home/gluex/.ssh/b1pi_cron | grep -v grep | wc -l`
    sleep 300
done
/group/halld/Software/hd_utilities/b1pi_test/b1pi_message.sh
source /group/halld/Software/build_scripts/gluex_env_jlab.sh
/group/halld/Software/time-history-plots/plots/b1pi_test/b1pi_history_data.sh $date_token >> /group/halld/time_history_data/b1pi_test.txt
exit
