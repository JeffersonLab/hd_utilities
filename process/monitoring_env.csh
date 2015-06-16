# load standard environment
source $HOME/setup_jlab.csh
#source $HOME/setup_jlab_commissioning.csh

# load ROOT python bindings
setenv PYTHONPATH $ROOTSYS/lib

# need python 2.7 for MySQL bindings
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

# setup CCDB
setenv CCDB_HOME /group/halld/Software/builds/ccdb/$BMS_OSNAME/ccdb_1.04
source $CCDB_HOME/environment.csh
