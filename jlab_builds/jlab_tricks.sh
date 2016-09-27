#!/bin/bash
# farm-specific set-up
nodename=`uname -n`
if [[ $nodename =~ ^farm* || $nodename =~ ^ifarm* || $nodename =~ ^qcd* || $nodename =~ ^gluon* ]]
    then
    export http_proxy=http://jprox.jlab.org:8081
    export https_proxy=https://jprox.jlab.org:8081
fi
grep -lq " release 6." /etc/redhat-release
if [ $? -eq 0 ]
    then
    GCC_HOME=/apps/gcc/4.9.2
    export PATH=${GCC_HOME}/bin:${PATH}
    export LD_LIBRARY_PATH=${GCC_HOME}/lib64:${GCC_HOME}/lib
fi
# perl on the cue
export PATH=/apps/perl/bin:$PATH
# python on the cue
export PATH=/apps/python/PRO/bin:$PATH
export LD_LIBRARY_PATH=/apps/python/PRO/lib:$LD_LIBRARY_PATH
