#!/bin/sh
hosts="jlabl1 ifarm1102 jlabl3 ifarm1401"
date_token=`date +%F`
date_file=/u/scratch/$USER/b1pi_date.txt
rm -fv $date_file
echo $date_token > $date_file
RUN_DIR=/u/scratch/$USER/b1pi/$date_token
mkdir -p $RUN_DIR
for host in $hosts
do
    echo host = $host
    logfile=/u/scratch/$USER/b1pi_$host.log
    echo logfile = $logfile
    rm -fv $logfile
    # the following ssh executes the command associated with the
    # /home/gluex/.ssh/b1pi_cron.pub key in
    # /home/gluex/.ssh/authorized_keys. That command should be
    # /group/halld/Software/scripts/b1pi_macros/b1pi_one.csh
    env -u SSH_AUTH_SOCK ssh -i ~/.ssh/b1pi_cron $host > $logfile 2>&1
    mv -v $logfile $RUN_DIR/
done
/group/halld/Software/scripts/b1pi_test/b1pi_message.sh
exit
