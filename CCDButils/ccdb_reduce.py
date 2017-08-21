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
#   ccdb_reduce.py -input=ccdb.sqlite -output=ccdb_reduced.sqlite -rmin=30000 =rmax=3999 -variation=default -date='2017-08021 12:00:00'
#
# The above example will read from a local file named "ccdb.sqlite" and write the
# reduced DB to a file named "ccdb_reduced.sqlite". It will only write values
# corresponding to the "default" variation for runs 30000-39999 with the specified
# date/time. Note that the input file is first copied to the output file and then
# the output file reduced. If the input filename and output filename are then same
# then the copy does not take place and the input file is reduced. 
#  
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
import sys
import sqlite3

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
	if arg.startswith('-input='    ) : SQLITE_INFILE  = arg[6:]
	if arg.startswith('-output='   ) : SQLITE_OUTFILE = arg[7:]
	if arg.startswith('-rmin='     ) : RUN_MIN        = arg[6:]
	if arg.startswith('-rmax='     ) : RUN_MAX        = arg[6:]
	if arg.startswith('-variation=') : VARIATION      = arg[10:]
	if arg.startswith('-date='     ) : DATE_AND_TIME  = arg[5:]

#---------------------------------------------------------------------------

# Connect to CCDB sqlite file
sqlite_connect_str = "sqlite:///" + SQLITE_INFILE
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
			assignment = provider.get_assignment(str(t.path), run, VARIATION, DATE_AND_TIME)
			constantsets_to_keep.append(assignment.constant_set_id)
			print '%d : %d - %d : %s' % (assignment.id, assignment.run_range.min, assignment.run_range.max, t.path)
			if assignment.run_range.max >= RUN_MAX : break
			run = assignment.run_range.max + 1
		except:
			#print 'skipping ' + t.path
			break

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

