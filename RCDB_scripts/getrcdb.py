# script to show all RCDB conditions for specified run(s)

import sys
import rcdb

if len(sys.argv) < 2 :
  print("Usage: python getrcdb.py <first_run_number> [<last_run_number>]")
  print("eg python getrcdb.py 81234")
  exit()
  
run1 = sys.argv[1]

if len(sys.argv) > 2:
    run2 = sys.argv[2]
else:
    run2 = run1


db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb2")
String1 = ['AMO', 'PARA', 'PERP', '45DEG', '135DEG']

runs = db.get_runs(run1,run2)

for R in runs:

    conditions_by_name = R.get_conditions_by_name()
    conditions = conditions_by_name.keys()

    print( f"{'Run_number':22s}",R.number)    
    for cond in conditions:
        cond_object = R.get_condition(cond)
        print( f"{cond:22s}" , cond_object.value)

    print("#---------------------")
    print()
        
