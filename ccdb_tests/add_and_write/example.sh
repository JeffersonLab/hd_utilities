#!/bin/bash
#
# set up the gluex environment
#
source /group/halld/Software/build_scripts/gluex_env_boot_jlab.sh
gxenv
export CCDB_CONNECTION=sqlite:////local/scratch/ccdb_test/ccdb.sqlite
#
# make a list of commands to execute
#
c[1]="ccdb ls /test"
c[2]="ccdb mkdir /test/mydir"
c[3]="ccdb ls /test"
c[4]="ccdb ls /test/mydir"
c[5]="ccdb mktbl /test/mydir/mytable alpha beta gamma"
c[6]="ccdb ls /test/mydir"
c[7]="cat data.txt"
c[8]="ccdb add /test/mydir/mytable -v test -r 1000-2000 data.txt"
c[9]="ccdb ls /test/mydir"
c[10]="ccdb cat /test/mydir/mytable:999:test"
c[11]="ccdb cat /test/mydir/mytable:1000:test"
c[12]="ccdb cat /test/mydir/mytable:1500:test"
c[13]="ccdb cat /test/mydir/mytable:2000:test"
c[14]="ccdb cat /test/mydir/mytable:2001:test"
#
# execute the commands
#
for i in {1..14}
do
    echo ----- command $i = \"${c[$i]}\" -----
    ${c[$i]}
done
