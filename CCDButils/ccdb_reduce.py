#!/usr/bin/env python
#
#
# This script will try and shrink a CCDB SQLite file by removing constant sets
# that do not correspond to a specific run range, variation, and date. 
#
# Usage:
#
#   ccdb_reduce.py [options]
#
#   options:
#       -input=ccdb_input.sqlite
#       -output=ccdb_output.sqlite
#       -rmin=run_min
#       -rmax=run_max
#       -variation=variation
#       -date='date_time'
#
#  All options are in form of single string with format "-key=value". 
#
# Example:
#
#   ccdb_reduce.py -input=ccdb.sqlite -output=ccdb_reduced.sqlite -rmin=30000 -rmax=3999 -variation=default -date='2017-08-21 12:00:00'
#
# The above example will read from a local file named "ccdb.sqlite" and write the
# reduced DB to a file named "ccdb_reduced.sqlite". It will only write values
# corresponding to the "default" variation for runs 30000-39999 with the specified
# date/time. Note that the input file is first copied to the output file and then
# the output file reduced. If the input filename and output filename are the same
# then the copy does not take place and the input file is reduced.
# 
# This script should do the right thing regarding variations. It uses the CCDB API
# to get a list of constant set ids for each item using the specified variation.
# This means all constant sets saved in the reduced file will be what would be
# returned by querying the full DB. However, if you save variation "A" to the 
# reduced file and then query it using the "default" variation, some constant sets
# will not be correct.
#
# This will not make the absolute smallest possible SQLite file since there are
# entries in things like the runRange tables that are not removed. This will
# remove the bulk of the extra data though making a much more compact SQLite file.
#
# WARNING:
#  This tries to capture all constants in the specified run range, but it is
# possible that some can be missed. Specifically, this situation:
#
# set 2 :          |-----------------------|
# set 1 :   |--------------------------------------|
#        RUN_MIN                                RUN_MAX
#
# In the above case, set 1 will be saved, but set 2 will not. This is because the
# algorithm used in this script relies on checking only at run range boundaries
# of the constants it finds. To capture set 2, every run number would need to be
# checked for every type table making this script much slower.
# Note that a similar condition would occur if set 1 did not exist and no other
# sets include RUN_MIN. 
#


import os
import re
import sys
import sqlite3
import datetime
from subprocess import call

import ccdb
from ccdb import Directory, TypeTable, Assignment, ConstantSet

SQLITE_INFILE  = os.path.join(os.getcwd(), "ccdb_sparse.sqlite")
SQLITE_OUTFILE = os.path.join(os.getcwd(), "ccdb_sparse.sqlite")
RUN_MIN = 30000
RUN_MAX = 39999
VARIATION = 'default'
DATE_AND_TIME = None
USER = os.getenv('USER', 'anonymous')

#---------------------------------------------------------------------------

for arg in sys.argv:
	if arg.startswith('-input='    ) : SQLITE_INFILE  = arg[7:]
	if arg.startswith('-output='   ) : SQLITE_OUTFILE = arg[8:]
	if arg.startswith('-rmin='     ) : RUN_MIN        = arg[6:]
	if arg.startswith('-rmax='     ) : RUN_MAX        = arg[6:]
	if arg.startswith('-variation=') : VARIATION      = arg[11:]
	if arg.startswith('-date='     ) : DATE_AND_TIME  = arg[6:]

# Datetime string must be handled specially
DATETIME = None
if DATE_AND_TIME :
	tok = []
	for t in filter(None, re.split(r'(\d+)', DATE_AND_TIME)):
		try: tok.append(int(t))
		except: pass
	if   len(tok) == 6 : DATETIME = datetime.datetime(tok[0], tok[1], tok[2], tok[3], tok[4], tok[5])
	elif len(tok) == 3 : DATETIME = datetime.datetime(tok[0], tok[1], tok[2])
	else: print 'Unable to parse date/time string properly!'

sqlite_connect_str = "sqlite:///" + SQLITE_INFILE
print ''
print '--------------------------------------------------'
print '    input: ' + sqlite_connect_str
print '   output: ' + SQLITE_OUTFILE
print '     rmin: ' + RUN_MIN
print '     rmax: ' + RUN_MAX
print 'variation: ' + VARIATION
if DATETIME : print '     date: \'' + str(DATETIME) + '\''
print '--------------------------------------------------'
print ''

#---------------------------------------------------------------------------



# Connect to CCDB sqlite file
provider = ccdb.AlchemyProvider()                        # this class has all CCDB manipulation functions
provider.connect(sqlite_connect_str)                     # use usual connection string to connect to database
provider.authentication.current_user_name = USER         # to have a name in logs

# Get list of constant set id's for the specified RUN, VARIATION, and DATE_AND_TIME
print 'Getting list of ids for specified run, variation, and date ...'
type_tables = provider.search_type_tables('*')
constantsets_to_keep = []
for t in type_tables :

	# automatically skip old B-field maps that are obsolete
	if t.name == 'radlen': continue
	if 'solenoid' in t.name and '_2009' in t.name: continue
	if t.name.startswith( 'solenoid_1400' ): continue
	if t.name.startswith( 'solenoid_1500' ): continue
	if t.name.startswith( 'solenoid_1600' ): continue

	# Loop until we find all ids in specified run range
	run = RUN_MIN
	while run <= RUN_MAX:
		try:
			assignment = provider.get_assignment(str(t.path), run, VARIATION, DATETIME)
			constantsets_to_keep.append(assignment.constant_set_id)
			# print 'found id:%d   runs:%d - %d  %s' % (assignment.id, assignment.run_range.min, assignment.run_range.max, t.path)
			if assignment.run_range.max >= RUN_MAX : break
			run = assignment.run_range.max + 1
		except:
			#print 'skipping ' + t.path
			break
print 'Found %d constants in %d type tables for specfied parameters' % (len(constantsets_to_keep), len(type_tables))


# Copy input to output if different files
if SQLITE_OUTFILE != SQLITE_INFILE:
	print 'Copying '+SQLITE_INFILE+' to '+SQLITE_OUTFILE+' ...'
	call(['cp', SQLITE_INFILE, SQLITE_OUTFILE])

# Close CCDB connection and re-open as straight sqlite connection
provider.disconnect()
conn = sqlite3.connect(SQLITE_OUTFILE)
c = conn.cursor()

# Get list of all constant set ids
print 'Getting list of all constants ..'
c.execute('SELECT id FROM constantSets')
rows = c.fetchall()
constantsets_all = []
for row in rows : constantsets_all.append(row[0])
print 'Found %d total constant sets in DB' % len(constantsets_all)

# Form list of constant sets to delete and delete them
constantsets_to_delete = []
for id in constantsets_all :
	if id not in constantsets_to_keep : constantsets_to_delete.append(id)

print 'deleting %d/%d constant sets (keeping %d for %d type tables) ...' % (len(constantsets_to_delete), len(constantsets_all), len(constantsets_to_keep), len(type_tables))
for id in constantsets_to_delete:
	c.execute('DELETE from assignments WHERE constantSetId=%d' % id)
	c.execute('DELETE from constantSets WHERE id=%d' % id)

# Delete entries in variations table that are not this one
c.execute('DELETE from variations WHERE name!="%s"' % VARIATION)

# Delete all entries in logs table (this can be half the file size!)
c.execute('DELETE from logs')

# Make sqlite actually shorten file size
print 'Vacuuming sqlite file ...'
c.execute('REINDEX')
c.execute('VACUUM')

print 'done\n'

