#!/bin/tcsh

# need python 2.7 for MySQL bindings
# setenv PATH /apps/python/PRO/bin:$PATH
# setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

source /group/halld/Software/build_scripts/gluex_env_jlab.csh

# load ROOT python bindings
setenv PYTHONPATH $ROOTSYS/lib:$PYTHONPATH

