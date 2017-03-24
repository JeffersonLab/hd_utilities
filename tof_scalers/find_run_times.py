#!/apps/python/PRO/bin/python

# Import nessesary modules
import sys
import rcdb
from rcdb.model import ConditionType, Condition, Run

import inspect
import sqlalchemy

# Initialize run numbers to be used for rcdb search
begin_run = 0
end_run = 0

if len(sys.argv) > 1:
	# -period option
	if sys.argv[1] == "-period":
		if sys.argv[2] == "RunPeriod-2016-02":
			print "Creating RunInfo.txt for RunPeriod-2016-02 ..."
			begin_run = 10000
			end_run =   19999
		if sys.argv[2] == "RunPeriod-2016-10":
			print "Creating RunInfo.txt for RunPeriod-2016-10 ..."
			begin_run = 20000
			end_run =   29999
		if sys.argv[2] == "RunPeriod-2017-01":
			print "Creating RunInfo.txt for RunPeriod-2017-01 ..."
			begin_run = 30000
			end_run =   39999

	# -range option
	elif sys.argv[1] == "-range":
		print "Creating RunInfo.txt for Runs [", sys.argv[2], ",", sys.argv[3], "] ..."
		begin_run = sys.argv[2]
		end_run = sys.argv[3]

	# -number option
	elif sys.argv[1] == "-number":
		print "Creating RunInfo.txt for Run [", sys.argv[2], "] ..."
		begin_run = sys.argv[2]
		end_run = sys.argv[2]

	# -help option
	elif sys.argv[1] == "-help":
		print "\n./scalerVtime.csh -help:\n" \
			"PURPOSE:\n" \
			"    An executable that loads EPICS scaler values for the Time of Flight detector\n" \
			"located in Hall D of at Thomas Jefferson National Accelerator Facility.\n" \
			"ENVIRONMENT:\n" \
			"    This executable is designed to first check and source the correct environment\n" \
			"for the user. A gluon machine that uses /bin/tcsh or /bin/csh shell environment\n" \
			"is required. Additionally, the user should only source /gluex/etc/hdonline.cshrc,\n" \
			"not /group/halld/Software/build_scripts/gluex_env_jlab.csh.\n" \
			"RUNNING:\n" \
			"    A number of messages should be printed to terminal to ensure the program is\n" \
			"running successfully; including an estimated run time and output root file size.\n" \
			"Only run numbers that are both @is_production and @is_status_approved will be fetched.\n" \
			"OUTPUT:\n" \
			"    The output file TOF_scalers.root should appear inside the directory once the\n" \
			"program has completed. Inside the root file will be a directory stucture that\n" \
			"should be self explanatory.\n" \
			"If there are any questions or bugs that need to be addressed, please email bcannon@jlab.org\n\n" \
			"Usage: ./scalerVtime.csh [OPTION] [PERIOD/RANGE/NUMBER]\n" \
			"  -period          Analyize an entire run period.   Example: ./scalerVtime.csh -period RunPeriod-2016-02\n" \
			"  -range           Analyize a range of runs.        Example: ./scalerVtime.csh -range 30600 30700\n" \
			"  -number          Analyize one run.                Example: ./scalerVtime.csh -number 30651\n"
		sys.exit()

	# incorrect option given
	else:
		print "\n./scalerVtime.csh: invalid option", sys.argv[1], \
			"\nTry ./scalerVtime -help' for more information.\n"
		sys.exit()

# no option given
else:
	print "\n./scalerVtime.csh: no option given.\n" \
		"Try './scalerVtime -help' for more information.\n"
	sys.exit()

# Create output file and get rcdb info
output_file = open("RunInfo.txt","w")
db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb")
runs = db.select_runs("@is_production and @status_approved", begin_run, end_run)

# Check to make sure runs > 0
if len(runs) == 0:
	print "Error: Run list empty."
	print "Run(s) given are either not @is_production or @status_approved."
	print "RunInfo.txt will be an empty file and this analysis will FAIL."

# Write run_number, start_time, and stop_time to output file
for run in runs:
  output_file.write('%s %s %s\n' % (run.number, run.start_time, run.end_time))
print "RunInfo.txt complete ..."

