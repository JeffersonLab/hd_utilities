import rcdb
import sys

db = rcdb.RCDBProvider("mysql://rcdb@hallddb.jlab.org/rcdb")
runs = db.select_runs("@is_dirc_production", int(sys.argv[1]), int(sys.argv[2]))
#runs = db.select_runs("", int(sys.argv[1]), int(sys.argv[2]))

print len(runs), "runs"

total = 0
# this is the slow way to do it, see the RCDB wiki for a faster way
for run in runs:
    print run.number,db.get_condition(run, "event_count").value
    total += int(db.get_condition(run, "event_count").value)

print "TOTAL = " + str(total)
