#!/bin/tcsh
# farm-specific set-up
set nodename=`uname -n`
if ( $nodename =~ farm* || $nodename =~ ifarm* || $nodename =~ qcd* || $nodename =~ gluon* ) then
    setenv http_proxy http://jprox.jlab.org:8081
    setenv https_proxy https://jprox.jlab.org:8081
endif
grep -lq " release 6." /etc/redhat-release
if ( ! $status ) then
    set GCC_HOME=/apps/gcc/4.9.2
    setenv PATH ${GCC_HOME}/bin:${PATH}
    setenv LD_LIBRARY_PATH ${GCC_HOME}/lib64:${GCC_HOME}/lib
endif
echo GCC_HOME = $GCC_HOME
# perl on the cue
setenv PATH /apps/perl/bin:$PATH
# python on the cue
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH
