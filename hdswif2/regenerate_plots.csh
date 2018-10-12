#!/bin/tcsh

# This script will regenerate a number of plots for a
# given workflow at NERSC. For it to work properly it
# must be run from an account with passwordless login
# to cori.nersc.gov and passwordless loging to
# gxproj4@ifarm.
#
# Usage:
#
#   ./regenerate_plots.csh [options]
#
#
# options:
#
#  -w,-workflow <workflow>     Specify the swif2 workflow
#
#  -s,-starttime <start time>  Specify the start time for submitted
#                              jobs to slurm. If this is ommitted
#                              then swif2 is run on gxproj4@ifarm
#                              to find the creation time of the given
#                              workflow.
#
#  -p,-post                    Upload the newly generated plots to the
#                              web. Location can be specified with 
#                              -u option.
#
#  -u,-uploaddir <upload dir>  Upload files to the specified directory.
#                              This option implies -p. The upload  dir
#                              specified will be passed to scp and must
#                              be reachable via passwordless login.
#                              (See example below.)
#
#  -d                          Try to open a window to display the
#                              generated plots. On Linux this will
#                              use eog if it is in your path. On Mac
#                              OS X it will just use "open".
#
#
# Example:  Regenerate plots locally for specified workflow
#
#    ./regenerate_plots.csh -w offmon_2018-01_ver17
#
#
# Example:  Regenerate plots and upload them to the web
#
#    ./regenerate_plots.csh -w offmon_2018-01_ver17 -u davidl@ifarm:/group/halld/www/halldweb/html/data_monitoring/launch_analysis/2018_01/launches/offline_monitoring_RunPeriod2018_01_ver17
#
# n.b. for the above to work the directory must already exist!
#
#
#

# A defaults
set workflow="offmon_2018-01_ver18"
set uploaddir="gxproj5@ifarm:/group/halld/www/halldweb/html/data_monitoring/launch_analysis/2018_01/launches/offline_monitoring_RunPeriod2018_01_ver18"
set njobs=0

# Initial testing
#set start_date="2018-07-19T11:00:00" # n.b. California time
#set   plot_end="2018-07-22T12:00:00" # n.b. California time

# Monitoring launch ver 17
#set start_date="2018-08-21T11:00:00" # n.b. California time
#set   plot_end="2018-09-02T12:00:00" # n.b. California time

# Haswell 14TB test
#set start_date="2018-09-10T00:00:00" # n.b. California time
#set   plot_end="2018-09-16T00:00:00" # n.b. California time

# KNL 14TB test
#set start_date="2018-09-20T13:20:00" # n.b. California time
#set   plot_end="2018-09-22T12:00:00" # Use this for "now"

# Dual Haswell + KNL 14TB test
#set start_date="2018-09-26T06:00:00" # n.b. California time
#set   plot_end="" # Use this for "now"

# Quad Haswell + KNL 14TB test
#set start_date="2018-09-27T10:00:00" # n.b. California time
#set   plot_end="2018-10-01T16:00:00" # Use this for "now"

# Hex Haswell + KNL 14TB test
#set start_date="2018-10-01T18:00:00" # n.b. California time
#set   plot_end="" # Use this for "now"

# offmon-2018_01_ver18
set start_date="2018-10-07T06:30:00" # n.b. California time
set   plot_end="" # Use this for "now"


set plot_start=$start_date 

# Loop over command line args
foreach arg ($*)
	switch( "$arg" )
		case '-d':
			set display_plots=1
			breaksw
		case '-s':
		case '-starttime':
			shift
			set start_date=$argv[1]
			breaksw
		case '-w':
		case '-workflow':
			shift
			set workflow=$argv[1]
			breaksw
		case '-n':
		case '-njobs':
			shift
			set njobs=$argv[1]
			breaksw
		case '-N':
			set get_njobs_from_workflow='1'
			breaksw
		case '-p':
		case '-post':
			set post_to_web=1
			breaksw
		case '-u':
		case '-uploaddir':
			shift
			set uploaddir=$argv[1]
			set post_to_web=1
			breaksw
			
	endsw
	if ( $#argv > 0 ) shift
end

# If user did not specify start date/time of the launch
# then get it from the workflow.
if ( ! $?start_date ) then
	set start_date=`ssh gxproj4@ifarm swif2 status -workflow $workflow | grep create_ts | awk '{print $3" "$4}' | sed 's/..$//'`  # sed is to chop off last ".0" that sacct doesn't like
endif

# Optionally get the number of jobs from workflow
if ( $?get_njobs_from_workflow ) then
	set njobs=`ssh gxproj4@ifarm swif2 status -workflow $workflow | grep jobs | awk '{print $3" "$4}'`
endif

echo "  workflow: "$workflow
echo "start time: "$start_date
echo "plot start: "$plot_start
echo "plot  end : "$plot_end
echo "     njobs: "$njobs

# Run sacct on cori to get info from slurm in form of text
# file and copy it back to local directory.
echo "Mining slurm data from NERSC ..."
ssh cori.nersc.gov "cd builds/accounting ; sacct --format=JobID%15,Submit,Start,End,NCPUS,CPUTimeRaw,ResvCPURAW,MaxRSS,JobName%30,ExitCode,MaxDiskRead -S '$start_date' > slurm.dat"
scp cori.nersc.gov:builds/accounting/slurm.dat .

# Convert ascii file to SQLite DB and CSV formats
./slurm2sqlite.py $plot_start $plot_end

# Run all macros to create plots
foreach m ( Njobs_vs_time.C latency_vs_time.C cpu_vs_time.C )
	echo "Running ROOT macro $m ..."
	root -l -q -b $m'("'$plot_start'")'
end

root -l -q -b 'iNjobs_vs_time.C("'$plot_start'",'$njobs')'

# Optionally open window to display plots on local machine
if ( $?display_plots ) then
	if ( $OSTYPE == 'darwin' ) then
		open *.png
	else if ( `where eog` != "" ) then
		eog *.png
	endif
endif

# Optionally upload files to web
if ( $?post_to_web ) then
	echo "uploading to $uploaddir"
	scp index.html *.png $uploaddir
endif


