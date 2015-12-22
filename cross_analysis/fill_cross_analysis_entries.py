
#--------------------------------------------------------------------#
#                                                                    #
# 2015/11/30 Kei Moriya                                              #
#                                                                    #
# Fill resources table used for offline monitoring from swif         #
# XML output and stdout files.                                       #
#                                                                    #
# From SWIF XML output, for each run and file, extract               #
# - auger_cpu_sec                                                    #
# - auger_wall_sec                                                   #
# - auger_mem_kb                                                     #
# - auger_vmem_kb                                                    #
# - final state                                                      #
# - problems                                                         #
#                                                                    #
# For same run and file, use stdout file to extract                  #
# - nevents processed                                                #
# - time to copy input file                                          #
# - time to process plugin                                           #
#                                                                    #
# Note:                                                              #
# ***    we want ONLY ONE ENTRY in this table FOR EACH JOB,   ***    #
# ***    NOT for each attempt.                                ***    #
#                                                                    #
#--------------------------------------------------------------------#

from sets import Set
from optparse import OptionParser
import os.path
import sys
import MySQLdb
import xml.etree.ElementTree as ET
import subprocess
import re

def main(argv):

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "fill_cross_analysis_entries.py [run period] [version]\n"
                                      + "example: fill_cross_analysis_entries.py 2015_03 15\n"
                                      + "\n"
                                      + "Input xml file should be created with\n"
                                      + "hdswif.py summary [workflow]"))
    
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 2:
        print "Usage:"
        print "fill_cross_analysis_entries.py [run period] [version]"
        print "example: fill_cross_analysis_entries.py 2015_03 15"
        exit()
    
    RUNPERIOD = ""
    VERSION = ""

    # RUNPERIOD is with underscore
    RUNPERIOD = args[0]
    RUNPERIOD   = RUNPERIOD.replace('-','_')
    RUNPERIOD_HYPHEN = RUNPERIOD.replace('_', '-')
    VERSION   = args[1]
    XMLDIR    = '/home/' + os.environ['USER'] + '/monitoring/hdswif/xml/'

    # Create MySQL connection
    db_conn = MySQLdb.connect(host='hallddb', user='farmer', passwd='', db='farming')
    cursor = db_conn.cursor()

    tablename = 'cross_analysis_table_' + RUNPERIOD + '_ver' + str(VERSION)

    # Read in xml file and create tree, root
    xmlfilename = XMLDIR + '/swif_output_offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + str(VERSION) + '_hd_rawdata.xml'

    if not os.path.isfile(xmlfilename):
        print 'XML output file ' + xmlfilename + ' does not exist'
        print 'Please create the xml file using hdswif.py summary [workflow]'
        exit()

    tree = ET.parse(xmlfilename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    njobs = 0

    # Create set for run and file to keep only
    # unique jobs (first job in XML file will be kept)
    set_run_file = Set()

    for job in workflow_status.iter('job'):

        run_num = ''
        file_num = ''

        # Get run and file
        for user_run in job.iter('user_run'):
            run_num = str(user_run.text)
        for user_file in job.iter('user_file'):
            file_num = str(user_file.text)

        run_file = run_num + '_' + file_num

        # Check if we have already added this run/file combination
        # to our table
        if run_file in set_run_file:
            print run_file + ' has already been accounted for, skipping'
            continue

        set_run_file.add(run_file)

        # Variables to fill from SWIF XML
        cpu_sec     = -999
        wall_sec    = -999
        mem_kb      = -999
        vmem_kb     = -999
        final_state = ""
        problems    = []

        # Iterate over only final attempt and fill stats of job
        for attempt in job.findall('.//*attempt[last()]'):
            for auger_result in attempt.iter('auger_result'):
                final_state = auger_result.text

            for auger_cpu_sec in attempt.iter('auger_cpu_sec'):
                cpu_sec = int(auger_cpu_sec.text)

            for auger_wall_sec in attempt.iter('auger_wall_sec'):
                wall_sec = int(auger_wall_sec.text)

            for auger_mem_kb in attempt.iter('auger_mem_kb'):
                mem_kb = int(auger_mem_kb.text)

            for auger_vmem_kb in attempt.iter('auger_vmem_kb'):
                vmem_kb = int(auger_vmem_kb.text)

        # Iterate over all attempts and get list of problems
        for attempt in job.iter('attempt'):
            for problem in attempt.iter('problem'):
                problems.append(problem.text)
            
        # Get info from (final) stdout file for this job
        stdoutname = '/volatile/halld/offline_monitoring/RunPeriod-' + RUNPERIOD_HYPHEN + '/ver' + str(VERSION) + \
            '/log/' + str(run_num) + '/stdout_' + str(run_num) + '_' + str(file_num) + '.out'

        # Reply on Unix tools to extract
        # - nevents
        # - input_copy_sec
        # - plugin_sec

        nevents        = -999
        input_copy_sec = -999
        plugin_sec     = -999

        if os.path.isfile(stdoutname) is False:
            print 'warning: File ' + stdoutname + ' does not exist'
        else:
            for line in open (stdoutname,'r'):
                if re.search('copy input file', line):
                    input_copy_sec = line.split()[5]

                if re.search('TIMEDIFF', line):
                    plugin_sec = line.split()[1]

                if re.search('Average rate', line):
                    nevents = line.split()[2]

        # Print results for this job
        # print ' run = ' + str(run_num) + ' file = ' + str(file_num) + ' cpu_sec = ' + str(cpu_sec) \
        #     + ' wall_sec = ' + str(wall_sec) + ' mem_kb = ' + str(mem_kb) + ' vmem_kb = ' + str(vmem_kb) \
        #     + ' final_state = ' + final_state + ' problems = ' + ','.join(problems) \
        #     + ' input_copy_sec = ' + str(input_copy_sec) +  'plugin_sec = ' + str(plugin_sec) +  ' nevents = ' + str(nevents)

        if run_num != '' and file_num != '':
            njobs += 1
            db_cmd = 'INSERT INTO ' + tablename \
                + '(run, file, cpu_sec, wall_sec, mem_kb, vmem_kb, nevents, input_copy_sec, plugin_sec, final_state, problems)' \
                + ' VALUES (' \
                + " '" + str(run_num) + "'," \
                + " '" + str(file_num) + "'," \
                + " '" + str(cpu_sec) + "'," \
                + " '" + str( wall_sec) + "'," \
                + " '" + str( mem_kb) + "'," \
                + " '" + str( vmem_kb) + "'," \
                + " '" + str( nevents) + "'," \
                + " '" + str( input_copy_sec) + "'," \
                + " '" + str( plugin_sec) + "'," \
                + " '" + str( final_state) + "'," \
                + " '" + ','.join(problems) + "'" \
                + ')'

            db_conn.cursor().execute(db_cmd)

            if njobs % 1000 == 0:
                print 'processed ' + str(njobs) + ' jobs'

        else:
            print "This shouldn't happen..."
            print 'run_num_text = ', run_num_text, ' , file_num_text = ', file_num_text

    print 'Modified table ' + tablename + ' with ' + str(njobs) + ' entries.'

if __name__ == "__main__":
   main(sys.argv[1:])
