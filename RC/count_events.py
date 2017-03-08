
# simple script to count the number of events in a range of production runs
# not working right now, something in RCDB is broken?
import rcdb
import sys

db = rcdb.RCDBProvider("mysql://rcdb@hallddb.jlab.org/rcdb")
#runs = db.select_runs("@is_production", sys.argv[1], 900000)
runs = db.select_runs("", sys.argv[1], 900000)

print len(runs), "runs"

total = 0
# this is the slow way to do it, see the RCDB wiki for a faster way
for run in runs:
    print run.number,run.get_condition("event_count")
    total += int(run.get_condition("event_count"))

print "TOTAL = " + str(total)
