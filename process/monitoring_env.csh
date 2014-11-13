
# load ROOT python bindings
setenv PYTHONPATH $ROOTSYS/lib

# need python 2.7 for MySQL bindings
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

# setup CCDB
setenv BUILD_SCRIPTS /group/halld/Software/builds/build_scripts/build_scripts_detcom01
setenv BMS_OSNAME `$BUILD_SCRIPTS/osrelease.pl`
setenv CCDB_HOME /group/halld/Software/builds/ccdb/$BMS_OSNAME/ccdb_1.03
source $CCDB_HOME/environment.csh
