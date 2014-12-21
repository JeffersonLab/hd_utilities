#!/usr/bin/env python
#
# Tool for manipulating the monitoring database
#
# Author: Sean Dobbs (s-dobbs@northwestern.edu), 2014

import sys
from optparse import OptionParser

#import datamon_db
from datamon_db import datamon_db

def query_yes_no(question, default="yes"):
    """Ask a yes/no question via raw_input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.
        It must be "yes" (the default), "no" or None (meaning
        an answer is required of the user).

    The "answer" return value is one of "yes" or "no".
    """
    valid = {"yes": True, "y": True, "ye": True,
             "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "
                             "(or 'y' or 'n').\n")

def print_commands():
    print "The following commands are supported:\n"
    print "  initDB     initialize database tables"
    print "  clearDB    delete all tables in the database (use carefully!!)"
#    print "  AddRun     Add new run information"
#    print "  AddVersion Add new version information"
    print "  Dump       Dump DB data to screen"
    print "  DumpCSV    Dump comma-seperated DB data"
    print "  DumpTSV    Dump tab-seperated DB data"
    print "  help       print this message"


if __name__ == "__main__":
    
    # read in command line args
    parser = OptionParser(usage = "datamon_db_tool.py [options] [command]")
    parser.add_option("-I","--interactive", dest="interactive", action="store_true",
                      help="Interactive mode")

    (options, args) = parser.parse_args()

    if(len(args) < 1):
        parser.print_help()
        print_commands()
        exit(0)

    ## initialize DB
    db = datamon_db()

    ## figure out what we want to do
    cmd = args[0]
    if(cmd == "initDB"):
        print "initializng database..."
        db.DefineTables()
    elif (cmd == "clearDB"):
        print "clearing DB..."
        if(query_yes_no("Are you REALLY sure?? This is dangerous!!", "no")):
            db.ClearDB()
            ## handcraft something here
            #print "doesn't do anything sorry!"
    elif (cmd == "AddRun"):
        # NEEDS TO UPDATE
        # extract info
        if(len(args) < 5 and not options.interactive):
            parser.print_help()
            print_commands()
            exit(0)
        if(options.interactive):
            run_num_in = input('Run number = ')
            num_events_in = input('Number of events = ')
            beam_current_in = input('Beam current = ')
            luminosity_in = input('Luminosity = ')
            run_num = int(run_num_in)
            num_events = int(num_events_in)
            beam_current = float(beam_current_in)
            luminosity = float(luminosity_in)
        else:
            run_num = int(args[1])
            num_events = int(args[2])
            beam_current = float(args[3])
            luminosity = float(args[4])

        print "== READ IN =="
        print "run number = %d\nnum events = %d\nbeam current = %6.3f\nluminosity = %6.3f" % (run_num,num_events,beam_current,luminosity)

        # check to see if the run already exists
        runid = db.GetRunID(run_num)
        if (runid >= 0):
            print "The run " + str(runid) + " already exists!"
            sys.exit(0)
        runid = db.AddRunInfo(run_num, num_events, beam_current, luminosity)
    elif (cmd == "AddVersion"):
        # NEEDS TO UPDATE
        i=1
        ## EMPTY
    elif (cmd == "Dump"):
        if(len(args)<2):
            print "Please specify a table name to dump, or 'all'"
            print "  valid tables are: " + " ".join(db.table_names)
            sys.exit(0)
        if(args[1] == "all"):
            for table in db.table_names:
                db.DumpTable(table)
                print ""
        else:
            db.DumpTable(args[1])
    elif (cmd == "DumpCSV"):
        if(len(args)<2):
            print "Please specify a table name to dump, or 'all'"
            print "  valid tables are: " + " ".join(db.table_names)
            sys.exit(0)
        if(args[1] == "all"):
            for table in db.table_names:
                db.DumpTable(table,"csv")
                print ""
        else:
            db.DumpTable(args[1],"csv")
    elif (cmd == "DumpTSV"):
        if(len(args)<2):
            print "Please specify a table name to dump, or 'all'"
            print "  valid tables are: " + " ".join(db.table_names)
            sys.exit(0)
        if(args[1] == "all"):
            for table in db.table_names:
                db.DumpTable(table,"tsv")
                print ""
        else:
            db.DumpTable(args[1],"tsv")
    elif (cmd == "help"):
        print_commands()
        sys.exit(0)
    else:
        print_commands()
        sys.exit(0)

    db.CloseDB()
