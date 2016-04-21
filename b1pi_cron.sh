#!/bin/sh
hosts="jlabl1 jlabl3 ifarm1401 jlabl5"
date_token=`date +%F`
date_file=/u/scratch/$USER/b1pi_date.txt
rm -fv $date_file
echo $date_token > $date_file
RUN_DIR=/u/scratch/$USER/b1pi/$date_token
mkdir -pv $RUN_DIR
webdir=/group/halld/www/halldweb/html/b1pi/$date_token
mkdir -pv $webdir
for host in $hosts
do
    echo host = $host
    logfile=/u/scratch/$USER/b1pi_$host.log
    echo logfile = $logfile
    rm -fv $logfile
    # the following ssh executes the command associated with the
    # /home/gluex/.ssh/b1pi_cron.pub key in
    # /home/gluex/.ssh/authorized_keys. That command should be
    # /group/halld/Software/scripts/b1pi_test/b1pi_cron_one.sh
    ( \
    env -u SSH_AUTH_SOCK ssh -i ~/.ssh/b1pi_cron $host > $logfile 2>&1 \
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
/group/halld/Software/scripts/b1pi_test/b1pi_message.sh
exit
