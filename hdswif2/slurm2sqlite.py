#!/usr/bin/env python
#
# This currently reads from a file created by running the following
# commands on Cori:
#
#  > cd ~/builds/accounting
#  > sacct --format=JobID%15,Submit,Start,End,NCPUS,CPUTimeRaw,ResvCPURAW,MaxRSS,JobName%30,ExitCode,MaxDiskRead -S "2018-08-21 11:00" > slurm.dat
#
# Then running this to copy to the local machine:
#
#  local.jlab> scp cori.nersc.gov:builds/accounting/slurm.dat .
#
#
# If an argument is given it is taken as the date/time for which all of the
# times written to the CSV file are relative. The format should be something
# like:  2018-09-11T10:37:23
# If no argument is given then the earliest submit time is used. 
#
# If a second argument is given then it is considered the maximum submit time to
# be written to the CSV file. The ROOT macros will use the latest time
# to determine the right-and edge of the time axis. Note that times earlier
# than the start time (first argument) will end up with negative values in
# the CSV file and therefore be outside of the plot range. Time later than
# the end time will not show up in the CSV file at all.
#
# n.b. the sqlite file will contain the actual date/time. Only the csv file
# uses relative times (in seconds) to make it easier to handle in ROOT. It
# is necessary then to tell the ROOT macros the start time used for the
# time axis to be displayed correctly.
#
#
#  Field definitions:
#
#  JobID%15    - JobID. The "%15" part is so it will print up to 15 characters of the
#                id. Each job actually has 3 steps and each shows up as a line with the
#                same numerical jobid, but with the second two having ".batch" and
#                ".extern" appended.
#
# Submit       - Submit time in YYY-MM-DDTHH:MM:SS format. Note that the "T" is always
#                there and seems to separate the Date and Time sections.
#
# Start        - Start time of the job in same format as Submit
#
# End          - End time of the job in same format as Submit
#
# NCPUS        - Number of CPUs assigned to the job. This looks to always be 64. This
#                counts both full cores and hyperthreads since the nodes have only 32
#                full cores.
#
# CPUTimeRaw   - Amount of time taken by the job in CPU-seconds. Divide this by NCPUS
#                to get the amount of time a single core was in use.
#
# ResvCPURaw   - The amount of CPU-seconds the job was waiting to run. This is a funny
#                unit to use here since there is not any actual CPU usage going on.
#                Divide this by NCPUS to get actual seconds the job was waiting
#                between Submit and Start.
#
# MaxRSS       - Maximum Resident Set Size or memory used. This is printed with a "K"
#                suffix which I assume is kB.
#
# JobName%30   - Job name. This should be something like "GLUEX_offmon_041008_005".
#                It is the only way I can see to get at the run/file numbers of
#                the job. This may actually be a potential issue since some jobs
#                may get retried after failure and they will have the same name.
#                The "%30" is just to make the number of printed characters large
#                enough to hold the whole name. Note also that the "batch" and "extern"
#                steps will have names that are just that.
#
# ExitCode     - This holds both the exit code and the signal type that caused the
#                exit like this:   exit_code:signal_type
#


import sqlite3
import csv
import sys

start_time = None
end_time   = None

if len(sys.argv)>1: start_time = sys.argv[1]
if len(sys.argv)>2:   end_time = sys.argv[2]

with open('slurm.dat') as f:

	conn = sqlite3.connect('slurm.db')
	c = conn.cursor()
	try:
		c.execute('''DROP TABLE IF EXISTS jobs''')  # force recreation of table to avoid duplicate entries
		c.execute('''CREATE TABLE jobs (Submit text, Start text, End text, NCPUS integer, CPUTIME integer, Latency real, JobName text, job_type text, Run integer, file integer, ExitCode integer)''')
	except:
		print 'Error creating table'

	for line in f :
		# Split line into tokens and extract numerical id
		vals = line.split();
		id = vals[0].split('.')[0]
		
		# Skip header lines and unfinished jobs
		#if vals[3] == 'Unknown': continue
		if vals[0] == 'JobID' or vals[0].startswith('---'): continue

		# Each job is spread over 3 lines with much of the info redundant amoung them.
		# Extract certain values from certain lines and assume the "extern" line is
		# the last.
		if vals[0].endswith('.batch'):
			if 'K' in vals[6]:
				MaxRSS_GB = float(vals[6].split('K')[0])/1000000.0  # Non-batch line skips the MaxRSS field so JobName is 6th value
			if 'M' in vals[6]:
				MaxRSS_GB = float(vals[6].split('M')[0])/1000.0  # Non-batch line skips the MaxRSS field so JobName is 6th value
		elif vals[0].endswith('.extern'):
			for i in range(0,len(vals)):
				if vals[i] == 'extern':
					if len(vals)>(i+1): ExitCode = int(vals[i+1].split(':')[0])  # "extern" line skips ResvCPURAW field so ExitCode is 8th value
					myvals =                          (Submit , Start , End , NCPUS , CPUTIME , Latency , JobName , job_type , run , file , ExitCode)
					c.execute('INSERT INTO jobs VALUES ( ?    ,   ?   ,  ?  ,   ?   ,   ?     ,    ?    ,    ?    ,     ?    ,  ?  ,   ?  ,    ?    )', myvals)
					break
		elif vals[0].endswith('.0'):
			pass # early test jobs seem to have this with NCPUS=1
		else:
			Submit   = vals[1]
			Start    = vals[2]
			End      = vals[3] if vals[3]!='Unknown' else '2018-01-01'
			NCPUS    = int(vals[4])
			CPUTIME  = vals[5]  # cpu-seconds
			Latency  = float(vals[6])/float(NCPUS)  # wait time between submit and start in seconds
			JobName  = vals[7]  # Non-batch line skips the MaxRSS field so JobName is 7th value
			nvals    = JobName.split('_')
			if len(nvals)>=4:
				job_type = nvals[1]
				run      = int(nvals[-2])
				file     = int(nvals[-1])
			else:
				job_type='test'
				run = 12345
				file = 0
			ExitCode = 0
			if len(vals)>8 : ExitCode = int(vals[8].split(':')[0])

			# Go ahead and make an entry for jobs that have not started but are queued so they
			# can be plotted
			if Start == 'Unknown':
				Start = '2018-01-01'
				End   = '2018-01-01'
				ExitCode = 0
				myvals =                          (Submit , Start , End , NCPUS , CPUTIME , Latency , JobName , job_type , run , file , ExitCode)
				c.execute('INSERT INTO jobs VALUES ( ?    ,   ?   ,  ?  ,   ?   ,   ?     ,    ?    ,    ?    ,     ?    ,  ?  ,   ?  ,    ?    )', myvals)

	# Flush date to sqlite file and close it
	conn.commit()
	conn.close()

	# Open the SQLite file back up and read data from it
	conn = sqlite3.connect('slurm.db')
	c = conn.cursor()
	
	# Use either earliest submit time or user specified start time
	sql = 'SELECT MIN(strftime("%s",Submit))  FROM jobs'
	if start_time != None: sql = 'SELECT strftime("%s","' + start_time + '")'
	c.execute(sql)
	tmin = str(c.fetchone()[0])

	# Use either current time or user specified end time
	sql = 'SELECT strftime("%s")'  # i.e. now
	if end_time != None: sql = 'SELECT strftime("%s","' + end_time + '")'
	print sql
	c.execute(sql)
	tmax = str(c.fetchone()[0])
	print 'tmax='+tmax
	
	sql = 'SELECT strftime("%s",Submit)-' + tmin
	sql += ',strftime("%s",Start)-' + tmin
	sql += ',strftime("%s",End)-' + tmin
	sql += ',CPUTIME'
	#sql += ',LATENCY'
	sql += ',CAST(strftime("%s",Start) as integer) - CAST(strftime("%s",Submit) as integer)'
	sql += ',NCPUS'
	sql += ',Run'
	sql += ',file'
	sql += ' FROM jobs'
	sql += ' WHERE CAST(strftime("%s",Submit) as integer)<' + tmax
	print sql

	ofile = open('slurm.csv', 'w')
	csvwriter = csv.writer(ofile)
	for row in c.execute(sql):
		csvwriter.writerow(row)
	ofile.close()
