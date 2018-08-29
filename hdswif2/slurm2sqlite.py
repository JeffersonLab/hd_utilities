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
			MaxRSS_GB = float(vals[6].split('K')[0])/1000000.0  # Non-batch line skips the MaxRSS field so JobName is 6th value
		elif vals[0].endswith('.extern'):
			for i in range(0,len(vals)):
				if vals[i] == 'extern' and len(vals)>(i+1):
					ExitCode = int(vals[i+1].split(':')[0])  # "extern" line skips ResvCPURAW field so ExitCode is 8th value
					myvals =                          (Submit , Start , End , NCPUS , CPUTIME , Latency , JobName , job_type , run , file , ExitCode)
					c.execute('INSERT INTO jobs VALUES ( ?    ,   ?   ,  ?  ,   ?   ,   ?     ,    ?    ,    ?    ,     ?    ,  ?  ,   ?  ,    ?    )', myvals)
					break

		else:
			Submit   = vals[1]
			Start    = vals[2]
			End      = vals[3] if vals[3]!='Unknown' else '2018-01-01'
			NCPUS    = int(vals[4])
			CPUTIME  = vals[5]  # cpu-seconds
			Latency  = float(vals[6])/float(NCPUS)  # wait time between submit and start in seconds
			JobName  = vals[7]  # Non-batch line skips the MaxRSS field so JobName is 7th value
			nvals    = JobName.split('_')
			job_type = nvals[1]
			run      = int(nvals[2])
			file     = int(nvals[3])

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
	
	c.execute('SELECT MIN(strftime("%s",Submit))  FROM jobs')
	tmin = c.fetchone()[0]
	
	sql = 'SELECT strftime("%s",Submit)-' + tmin
	sql += ',strftime("%s",Start)-' + tmin
	sql += ',strftime("%s",End)-' + tmin
	sql += ',CPUTIME,Latency'
	sql += ' FROM jobs'

	ofile = open('slurm.csv', 'w')
	csvwriter = csv.writer(ofile)
	for row in c.execute(sql):
		csvwriter.writerow(row)
	ofile.close()
