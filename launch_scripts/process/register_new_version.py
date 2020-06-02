#!/usr/bin/env python
#
# 2014/11/13  Sean Dobbs (s-dobbs@northwestern.edu)
#
# <notes>
#

import sys,os
from os import listdir
from os.path import isfile, join
from math import fabs
from datetime import datetime

from datamon_db import datamon_db

VALID_COMMANDS = [ "add", "update" ] 
VALID_DATA_TYPES = [ "rawdata", "recon", "root", "mc", "mon" ]


# set default values
def init_property_mapping():
    version_properties = {
        'data_type'          : '<None>',
        'run_period'         : '<None>',
        'revision'           : '<None>',
        'software_version'   : '<None>',
        'jana_config'        : '<None>',
        'ccdb_context'       : '<None>',
        'production_time'    : '<None>',
        'dataVersionString'  : '<None>'
    }
    return version_properties

# parse files containing version information
# be fairly lenient about the contents
# ignore lines that begin with '#'
# look for lines that look like:   KEY = VALUE
def parse_version_file(fname):
    properties = {}
    try:
        infile = open(fname)
        for line in infile:
            tokens = line.strip().split()
            if len(tokens)==0 or tokens[0][0] == '#':
                continue
            if len(tokens)<3:
                continue
            if tokens[1] == "=":
                properties[ tokens[0] ] = " ".join(tokens[2:])
        infile.close()
    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
        return None
        #sys.exit(0)
#    except:
#        print "Unexpected error:", sys.exc_info()[0]
#        return None
        #sys.exit(0)
    return properties


def print_usage():
    print "usage: register_new_version.py add filename"
    print "       register_new_version.py update version_id filename"

def main(argv):
    if len(argv) <= 1:
        print_usage()
        return
    cmd = argv[0]
    if cmd not in VALID_COMMANDS:
        print "Invalud command = " + cmd
        print_usage()
        return
        
    if cmd == "add":
        if len(argv) < 2:
            print_usage()
            return
        filename = argv[1]
    elif cmd == "update":
        if len(argv) < 3:
            print_usage()
            return
        try:
            version_id = int(argv[1])
        except ValueError:
            print "Bad version ID = " + argv[1]
            return
        filename = argv[2]

    # set up defaults
    version_properties = init_property_mapping()
    # read data from file
    input_properties = parse_version_file(filename)

    # do some sanity checking
    if input_properties['data_type'] not in VALID_DATA_TYPES:
        print "Invalid data_type specified = " + input_properties['data_type']
        print "Valid data types = " + " ".join(VALID_DATA_TYPES)
        return
    try:
        revision = int(input_properties['revision'])
    except ValueError:
        print "Bad revision value = " + input_properties['revision']
        return


    # build final version info
    for key in version_properties:
        if key in input_properties:
            version_properties[key] = input_properties[key]

    
    # input to database
    db = datamon_db()
    if cmd == "add":
        version_id = db.AddVersionInfo(version_properties)
        print "Your new version number is:  " + str(int(version_id[0]))
    elif cmd == "update":
        db.UpdateVersionInfo(version_id, version_properties)

if __name__ == "__main__":
    main(sys.argv[1:])
