#!/usr/bin/env python
#
#
# Extract the minimum and maximum run number from a given
# swif2 workflow.
#
# This works by reading the .json file for the workflow
# and extracting the first number in each job name. It
# prints the smallest and largest number found.
#
# The jobs are read from the file workflow.json where
# workflow is the actual workflow name. If the file
# does not exist in the current directory, swif2 is
# run to create it.

import os
import re
import sys
import json
import subprocess

if len(sys.argv) != 2:
	print('\nUsage:\n\n  extract_run_range.py workflow\n\n')
	sys.exit(0)

workflow = sys.argv[1]
fname = workflow + '.json'

# Create json file if needed
if not os.path.exists( fname ):
	cmd = ['swif2', 'status', '-workflow', workflow, '-jobs', '-display', 'json']
	json = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()[0]
	f = open(fname, 'w')
	f. write( json )
	f.close()

# Read in json file
RunMin = 999999
RunMax = 0
Njobs = 0
with open( fname ) as json_file:
	data = json.load( json_file )
	for job in data['jobs']:
		Njobs += 1
		nums = re.findall(r'\d+', job['job_name'] )
		if len(nums) > 0:
			run = int(nums[0])
			if run>RunMax: RunMax = run
			if run<RunMin: RunMin = run


print('%d %d' % (RunMin, RunMax))
#print('Njobs = ' + str(Njobs))
