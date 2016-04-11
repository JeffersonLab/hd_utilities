# load standard environment
##source $HOME/setup_jlab.csh
#source $HOME/setup_jlab_commissioning.csh
#source /home/gxproj5/halld/jproj/projects/offline_monitoring_RunPeriod2015_03_ver09_hd_rawdata/setup_jlab-2015-03.csh

# load ROOT python bindings
setenv PYTHONPATH $ROOTSYS/lib

# need python 2.7 for MySQL bindings
setenv PATH /apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH

# load RCDB
source /work/halld2/home/gxproj1/builds/rcdb/environment.csh

# setup CCDB
#setenv CCDB_HOME /group/halld/Software/builds/ccdb/$BMS_OSNAME/ccdb_1.04
#source $CCDB_HOME/environment.csh
