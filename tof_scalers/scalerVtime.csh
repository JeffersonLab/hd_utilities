#!/bin/tcsh

# Check to make sure the correct $SHELL is being used
if (`echo $SHELL | grep /bin/tcsh | wc -l` == 0) then
	echo "Error: incorrect shell environment"
	echo "This software requires: /bin/tcsh"
	echo "You are currently using: $SHELL"
	echo "Try using gluon31"
	exit
endif

# Source correct environment
echo "Sourcing /gluex/etc/hdonline.cshrc environment..."
source /gluex/etc/hdonline.cshrc
echo "Setting up python environment for rcdb..."
setenv BUILD_SCRIPTS /group/halld/Software/build_scripts
setenv PATH $BUILD_SCRIPTS/patches/jlab_extras/rh6:/apps/python/PRO/bin:$PATH
setenv LD_LIBRARY_PATH /apps/python/PRO/lib:$LD_LIBRARY_PATH
setenv RCDB_HOME /gluonfs1/home/bcannon/rcdb
source /group/halld/Software/build_scripts/rcdb_env.csh

# Check to make sure the correct g++ compiler is being used
if (`g++ --version | grep g++ | cut -c "11","12","13"` != "4.4") then
	echo "Error: incorrect g++ compiler"
	echo "This software requires: g++ (GCC) 4.4.7 20120313 (Red Hat 4.4.7-4)"
	echo "You are currently using: `g++ --version | grep g++`"
	echo "Try using gluon31"
	exit
endif

# Compile c++ code ..
echo "Running Makefile ..."
make

# Find the options inputted by the user, then make the file RunInfo.txt to get start and stop times
./find_run_times.py $*
echo "Removing unwanted files..."
rm -f hdr.ctl
rm -f #scalerVtime.csh#

# Run C++ code
echo "Fetching TOF scalers..."
./get_TOF_EPICS_scalers

# Remove RunInfo.txt
rm -f ./RunInfo.txt
exit

