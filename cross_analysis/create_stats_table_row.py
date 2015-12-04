
#--------------------------------------------------------------------#
#                                                                    #
# 2015/11/30 Kei Moriya                                              #
#                                                                    #
# Read out values from MySQL table and create a row in the           #
# stats table for each launch.                                       #
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
                                      + "create_stats_table_row.py [run period] [version]\n"
                                      + "example: create_stats_table_row.py 2015_03 15\n"
                                      + "\n"
                                      + "Input xml file should be created with\n"
                                      + "hdswif.py summary [workflow]"))
    
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 2:
        print "Usage:"
        print "create_stats_table_row.py [run period] [version]"
        print "example: create_stats_table_row.py 2015_03 15"
        exit()
    
    RUNPERIOD = ""
    VERSION = ""

    # RUNPERIOD is with underscore
    RUNPERIOD = args[0]
    RUNPERIOD   = RUNPERIOD.replace('-','_')
    RUNPERIOD_HYPHEN = RUNPERIOD.replace('_', '-')
    VERSION   = args[1]
    XMLDIR    = '/home/' + os.environ['USER'] + '/halld/hdswif/'

    # Variables of interest
    ntotal       = 0 # total number of run/files recorded in table; each run/file is unique
    nsuccess     = 0 # number of success in final attempt for that job
    nover_rlimit = 0 # number of jobs that had over_rlimit occur at some stage
    ntimeout     = 0 # number of jobs that had timeout occur at some stage
    nsystem      = 0 # number of jobs that had system issue occur at some stage
    nsegfault    = 0 # number of jobs that had segfault

    # Create MySQL connection
    db_conn = MySQLdb.connect(host='hallddb', user='farmer', passwd='', db='farming')
    cursor = db_conn.cursor()
    tablename = 'cross_analysis_table_' + RUNPERIOD + '_ver' + str(VERSION)
    print 'tablename = ' + tablename
    
    cursor.execute('SELECT run, file, final_state, problems FROM ' + tablename)
    results = cursor.fetchall()
    for row in results:
        run_num = row[0]
        file_num = row[1]
        final_state = row[2]
        problems = row[3]

        ntotal += 1

        if final_state == 'SUCCESS':
            nsuccess += 1
        if re.search('AUGER-OVER_RLIMIT', problems):
            nover_rlimit += 1
        if re.search('AUGER-TIMEOUT', problems):
            ntimeout += 1
        if re.search('SWIF-USER-NON-ZERO', problems):
            nsegfault += 1

        if any(re.match(regex_strs, problems) for regex_strs in ['AUGER-SUBMIT', 'AUGER-FAILED', 'AUGER-INPUT-FAIL', 'AUGER-OUTPUT-FAIL', 'SWIF-SYSTEM-ERROR', 'SWIF-MISSING-OUTPUT']):
            nsystem += 1
        
#        print 'run = ' + str(run_num) + ' file = ' + str(file_num) + ' cpu_sec = ' + str(cpu_sec) \
#            + ' wall_sec = ' + str(wall_sec) + ' mem_kb = ' + str(mem_kb) + ' vmem_kb = ' + str(vmem_kb) \
#            + ' nevents = ' + str(nevents) + ' input_copy_sec = ' + str(input_copy_sec) + ' plugin_sec = ' + str(plugin_sec) \
#            + ' final_state = ' + str(final_state) + ' problems = ' + str(problems)


    db_conn.close()

    # Segfaults are included in SUCCESS, so need to be subtracted off
    nsuccess = nsuccess - nsegfault

    print 'ntotal        = ' + str(ntotal)
    print 'nsuccess      = ' + str(nsuccess)
    print 'nover_rlimit  = ' + str(nover_rlimit)
    print 'ntimeout      = ' + str(ntimeout)
    print 'nsystem       = ' + str(nsystem)
    print 'nsegfault     = ' + str(nsegfault)

    # Create html table row from these
    outfilename = 'stats_swif_' + RUNPERIOD + '_' + 'ver' + str(VERSION) + '.html'
    outfile = open(outfilename,'w')

    outfile.write('  <tr>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">VER' \
                      + str(VERSION) + '</td>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                      + str(ntotal) + ' (' + "{:05.2f}".format(ntotal/ntotal * 100.) + ')</td>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                      + str(nsuccess) + ' (' + "{:05.2f}".format(100. * nsuccess/ntotal) + ')</td>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                      + str(nsegfault) + ' (' + "{:05.2f}".format(100. * nsegfault/ntotal) + ')</td>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                      + str(nover_rlimit) + ' (' + "{:05.2f}".format(100. * nover_rlimit/ntotal) + ')</td>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                      + str(ntimeout) + ' (' + "{:05.2f}".format(100. * ntimeout/ntotal) + ')</td>\n')
    outfile.write('    <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                      + str(nsystem) + ' (' + "{:05.2f}".format(100. * nsystem/ntotal) + ')</td>\n')
    # add hook to end so new results can be added in
    outfile.write('  </tr> <!--' + RUNPERIOD + '_' + 'ver' + str(VERSION) + '--> \n')
    outfile.close()

    # Add the newest row into the html table using the hook
    # that is added at the end of each launch version.

    # Open the stats html file and read in the contents
    stats_table_filename = '/group/halld/www/halldweb/html/data_monitoring/launch_analysis/' \
        + RUNPERIOD + '/stats_swif_' + RUNPERIOD + '.html'
    with open(stats_table_filename,'r') as stats_table_file:
        alllines = stats_table_file.readlines()

    # Reverse the lines and find the first occurrence of the hook
    # this is the final (most recent hook)
    alllines.reverse()

    nlines = 0
    pos = 0
    old_hook = ''
    for line in alllines:
        nlines += 1
        if re.search('<!--' + RUNPERIOD + '_' + 'ver',line):
           pos = nlines
           old_hook = line
           break

    if old_hook == '':
        print 'Did not find old hook, make sure that file'
        print stats_table_filename
        print 'has a line containing <!--[RUNPERIOD]_ver'
        exit()
    
    # position to seek is total number of lines in file minus pos + 1
    pos = len(alllines) - pos + 1

    # reopen outfile now as input
    with open(outfilename,'r') as newfile:
        newlines = newfile.readlines()
    
    # Reopen stats html file and insert newlines into line pos
    with open(stats_table_filename, 'r+') as stats_table_file:
        alllines = stats_table_file.readlines()

    alllines[pos:pos] = newlines

    # Write the new file
    with open(stats_table_filename, 'w') as stats_table_file:
        for line in alllines:
            stats_table_file.write(line)

if __name__ == "__main__":
   main(sys.argv[1:])
