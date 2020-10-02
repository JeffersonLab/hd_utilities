
#--------------------------------------------------------------------#
#                                                                    #
# 2015/12/01 Kei Moriya                                              #
#                                                                    #
# Read out values from MySQL table and create history for            #
# each file                                                          #
#                                                                    #
#--------------------------------------------------------------------#

from sets import Set
from optparse import OptionParser
import os.path
import sys
import MySQLdb
import xml.etree.ElementTree as ET
import subprocess
import glob
import re

def main(argv):
    VERBOSE = False

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "create_stats_for_each_file.py [run period] [minversion] [maxversion]\n"
                                      + "example: create_stats_for_each_file.py 2015_03 15 20\n"
                                      + "\n"
                                      + "Input xml file should be created with\n"
                                      + "hdswif.py summary [workflow]"))
    
    parser.add_option("-V","--verbose",dest="verbose",
                      help="verbose output")

    (options, args) = parser.parse_args(argv)

    if(options.verbose):
        VERBOSE = True
    
    if len(args) != 3:
        print "Usage:"
        print "create_stats_for_each_file.py [run period] [minversion] [maxversion]"
        print "example: create_stats_for_each_file.py 2015_03 15 20"
        exit()
    
    RUNPERIOD = ""
    VERSION = ""

    # RUNPERIOD is with underscore
    RUNPERIOD = args[0]
    RUNPERIOD   = RUNPERIOD.replace('-','_')
    RUNPERIOD_HYPHEN = RUNPERIOD.replace('_', '-')
    MINVERSION   = int(args[1])
    MAXVERSION   = int(args[2])

    # Create MySQL connection
    db_conn = MySQLdb.connect(host='hallddb', user='farmer', passwd='', db='farming')
    cursor = db_conn.cursor()

    # Grab all ***valid*** files in the tape system from /mss
    # Files ***must***
    # - have 6-digit, number only run numbers
    # - same run number in directory name and file name

    # Below is the same method used in hdswif.py to register jobs
    pathstring = "/mss/halld/RunPeriod-" + RUNPERIOD_HYPHEN + "/rawdata/Run*/hd_rawdata_*.evio"
    file_list = glob.glob(pathstring)

    # List of all valid launch vers
    launchlist = []
    # Check which launches between MINVERSION and MAXVERSION have a DB table,
    # if not, skip that launch
    for ver in range(MINVERSION,MAXVERSION+1):
        formatted_vernum = "%02d" % ver
        tablename = 'cross_analysis_table_' + RUNPERIOD + '_ver' + formatted_vernum
        cursor.execute("SHOW TABLES LIKE '" + tablename + "'")
        results = cursor.fetchall()

        if len(results) == 0:
            print 'table ' + tablename + ' does NOT exist, skipping...'
        else:
            print 'table ' + tablename + ' exists'
            launchlist.append(ver)

    print 'Launch vers to run over: ' + str(launchlist)

    # List of all [run]_[file]
    allfiles = []

    for eviofile in file_list:

        # Make sure that run number and file format is valid
        match = ""
        rundir   = 0
        run_num  = 0
        file_num = 0
        try:
            match = re.search(r"Run(\d\d\d\d\d\d)/hd_rawdata_(\d\d\d\d\d\d)_(\d\d\d)",eviofile) # _\d\d\d
            rundir   = match.group(1)
            run_num  = match.group(2)
            file_num = match.group(3)
        except AttributeError:
            "could not find regexp for " + eviofile
            continue

        # Check that run # in directory and file name agree
        if rundir != run_num:
            print 'skipping file ' + eviofile + ' (run number in directory and file name do not agree)'
            continue

        if(rundir == 0 or run_num == 0 or file_num == 0):
            print "couldn't find run and file number in " + eviofile
            continue

        allfiles.append(str(run_num) + '_' + str(file_num))

    # Sort all [run]_[file] combinations
    # All files can be processed in order
    allfiles.sort()

    run_previous = -999;
    hundreds_previous = -100;
    is_new_run = False

    # Dictionaries between [version] : [counts]
    ntotal = {}
    nsuccess = {}
    nsegfault = {}
    ncancelled = {}
    nover_rlimit = {}
    ntimeout = {}
    nsystem = {}
    nnull = {}
    for ver in launchlist:
        ntotal[ver] = 0
        nsegfault[ver] = 0
        nsuccess[ver] = 0
        ncancelled[ver] = 0
        nover_rlimit[ver] = 0
        ntimeout[ver] = 0
        nsystem[ver] = 0
        nnull[ver] = 0

    # need to declare outfile as a file handler
    # before using
    outfile = open('___tmp.txt','w')
    outfile.close()
    os.remove('___tmp.txt')
    outfilename = ''

    for run_file in allfiles:
        match = re.search(r"(\d\d\d\d\d\d)_(\d\d\d)",run_file)
        run_num = match.group(1)
        file_num = match.group(2)
        if VERBOSE:
            print '-------------------------------------------------------------------'
            print 'run = ' + str(run_num) + ' file = ' + str(file_num)

        # Grab results from DB for each version for this file,
        # fill into list of final_states and problems for this file
        dict_final_states = {ver : 'NULL' for ver in launchlist}
        dict_problems     = {ver : 'NULL' for ver in launchlist}

        for ver in launchlist:
            formatted_vernum = "%02d" % ver
            tablename = 'cross_analysis_table_' + RUNPERIOD + '_ver' + formatted_vernum
            cursor.execute('SELECT final_state, problems FROM ' + tablename + ' WHERE run = ' + str(run_num) + ' AND file = ' + str(file_num))
            results = cursor.fetchall()

            for row in results:
                dict_final_states[ver] = row[0]
                dict_problems[ver]     = row[1]

        # Start filling in tables for this run and file
        if run_num != run_previous:
            is_new_run = True

            if (int(run_num) // 100) != hundreds_previous:
                if VERBOSE:
                    print 'start of new hundreds'
                    print 'previous hundreds: ' + str(hundreds_previous)
                    print 'new hundreds     : ' + str(int(run_num) // 100)
                    print 'run_num = ' + str(run_num)

                # Getting into a new hundreds
                if not outfile.closed:
                    # Close file table for previous hundreds
                    outfile.write('  </table>\n')
                    outfile.write('\n')
                    outfile.write('  <hr style="width:80%;align:center;height:3px;color:#ff0000;border-color:#ff0000;background-color:#ff0000;">\n')
                    outfile.write('\n')
                
                    # Create summary table for previous hundreds
                    outfile.write('  <table style="border: 1px solid black; table-layout: fixed;">\n')
                    outfile.write('    <tr>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Status for ' \
                                      + "{:0>6d}".format(hundreds_previous * 100) + ' Runs</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Total (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Success (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Segfault (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Cancelled (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">NULL (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Over Limit (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Timeout (%)</th>\n')
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">System Error (%)</th>\n')
                    outfile.write('    </tr>\n')

                    # Fill in summary table for previous hundreds
                    for ver in launchlist:
                        # launch ver
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">VER' \
                                          + "{:0>2d}".format(ver) + '</td>\n')
                        # ntotal
                        value = 0 if ntotal[ver] == 0 else 100. * ntotal[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(ntotal[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # nsuccess
                        value = 0 if ntotal[ver] == 0 else 100. * nsuccess[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(nsuccess[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # nsegfault
                        value = 0 if ntotal[ver] == 0 else 100. * nsegfault[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(nsegfault[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # ncancelled
                        value = 0 if ntotal[ver] == 0 else 100. * ncancelled[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(ncancelled[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # nnull
                        value = 0 if ntotal[ver] == 0 else 100. * nnull[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(nnull[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # nover_rlimit
                        value = 0 if ntotal[ver] == 0 else 100. * nover_rlimit[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(nover_rlimit[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # ntimeout
                        value = 0 if ntotal[ver] == 0 else 100. * ntimeout[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(ntimeout[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        # nsystem
                        value = 0 if ntotal[ver] == 0 else 100. * nsystem[ver] / ntotal[ver]
                        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                                          + "{:>4d}".format(nsystem[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

                        outfile.write('    </tr>\n')
                    # end of looping over ver in launchlist
                    outfile.write('  </table>\n')

                    # Finish summary table for previous hundreds
                    outfile.write('<h2><a href = "../index.html">back to main</a></h2>\n')
                    outfile.write('</body>\n')
                    outfile.close()

                    # Reset counters for each hundreds of runs
                    for ver in launchlist:
                        ntotal[ver] = 0
                        nsegfault[ver] = 0
                        nsuccess[ver] = 0
                        ncancelled[ver] = 0
                        nover_rlimit[ver] = 0
                        ntimeout[ver] = 0
                        nsystem[ver] = 0
                        nnull[ver] = 0

                # end of not outfile.closed
            
                # We are still at the beginning of a new hundreds

                # Create new hundreds html file
                dirname = 'swif_rap_sheet_' + RUNPERIOD
                if not os.path.exists(dirname):
                    os.makedirs(dirname)

                outfilename = dirname + '/results_' + "{:06d}".format(int(run_num) // 100 * 100) + '.html'
                outfile = open(outfilename,'w')
                outfile.write('<head>\n')
                outfile.write('<link rel="stylesheet" type="text/css" href="../mystyle.css">\n')
                outfile.write('<meta http-equiv="content-style-type" content="text/css">\n')
                
                outfile.write('  <title> runs ' + str(int(run_num) // 100 * 100) + '  </title>\n')
                outfile.write('</head>\n')
                outfile.write('<body>\n')
                outfile.write('<h2>results for ' + str(int(run_num) // 100 * 100) + ' runs</h2>\n')

                # Example output
                outfile.write('  <h3>legend</h3>\n')
                outfile.write('  <table style="border: 1px solid black; table-layout: fixed;">\n')
                outfile.write('    <tr>\n')
                outfile.write('      <th style="border: 1px solid black; height:15px; width:100px;">state</th>\n')
                outfile.write('      <th style="border: 1px solid black; height:15px; width:450px;">explanation</th>\n')
                outfile.write('      <th style="border: 1px solid black; height:15px; width:100px;">state</th>\n')
                outfile.write('      <th style="border: 1px solid black; height:15px; width:450px;">explanation</th>\n')
                outfile.write('    </tr>\n')

                # SUCCESS
                outfile.write('    <tr>\n')
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + '#66FF99' + '; color:' + 'black' + ';">' + 'SUCCESS' + '</td>\n')
                outfile.write('      <td>' + 'SUCCESS, no problems' + '</td>\n')

                # SUCCESS with TIMEOUT only
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'orange' + '; color:' + 'black' + ';">' + 'SUCCESS' + '</td>\n')
                outfile.write('      <td>' + 'final state is SUCCESS, problems were TIMEOUT only ' + '</td>\n')
                outfile.write('    </tr>\n')

                # SUCCESS with OVER_RLIMIT only
                outfile.write('    <tr>\n')
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'yellow' + '; color:' + 'black' + ';">' + 'SUCCESS' + '</td>\n')
                outfile.write('      <td>' + 'final state is SUCCESS, problems were OVER_RLIMIT only ' + '</td>\n')

                # SUCCESS with SYSTEM only
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'gray' + '; color:' + 'black' + ';">' + 'SUCCESS' + '</td>\n')
                outfile.write('      <td>' + 'final state is SUCCESS, problems were SYSTEM only ' + '</td>\n')
                outfile.write('    </tr>\n')

                # SUCCESS with multiple problems
                outfile.write('    <tr>\n')
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'purple' + '; color:' + 'black' + ';">' + 'SUCCESS' + '</td>\n')
                outfile.write('      <td>' + 'final state is SUCCESS, multiple types of problems' + '</td>\n')

                # SEGFAULT
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'red' + '; color:' + 'black' + ';">' + 'SEGFAULT' + '</td>\n')
                outfile.write('      <td>' + 'final state is SEGFAULT' + '</td>\n')
                outfile.write('    </tr>\n')

                # NULL
                outfile.write('    <tr>\n')
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'gray' + '; color:' + 'white' + ';">' + 'NULL' + '</td>\n')
                outfile.write('      <td>' + 'NULL (this run/file was not registered)' + '</td>\n')

                # CANCELLED
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'gray' + '; color:' + 'white' + ';">' + 'CANCELLED' + '</td>\n')
                outfile.write('      <td>' + 'CANCELLED (all other final states)' + '</td>\n')
                outfile.write('    </tr>\n')

                # OVER_RLIMIT
                outfile.write('    <tr>\n')
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'yellow' + '; color:' + 'red' + ';">' + 'OVER_RLIMIT' + '</td>\n')
                outfile.write('      <td>' + 'final state was OVER_RLIMIT and no other problems' + '</td>\n')

                # TIMEOUT
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'orange' + '; color:' + 'white' + ';">' + 'TIMEOUT' + '</td>\n')
                outfile.write('      <td>' + 'final state was TIMEOUT and no other problems' + '</td>\n')
                outfile.write('    </tr>\n')

                # SYSTEM
                outfile.write('    <tr>\n')
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + '#E5E4E2' + '; color:' + 'red' + ';">' + 'SYSTEM' + '</td>\n')
                outfile.write('      <td>' + 'final state was SYSTEM and no other problems' + '</td>\n')

                # multiple problems
                outfile.write('      <td style=" padding:15px; height:20px; width:100px; border:1px solid black; background-color:' \
                                  + 'purple' + '; color:' + 'black' + ';">' + 'TIMEOUT/OVER_RLIMIT/SYSTEM' + '</td>\n')
                outfile.write('      <td>' + 'multiple problems and final state is not SUCCESS' + '</td>\n')
                outfile.write('    </tr>\n')
                outfile.write('  </table>\n')
                outfile.write('<br><br><br>\n')
                
                # Start of table
                outfile.write('  <table style="border: 1px solid black; table-layout: fixed;">\n')
                outfile.write('    <tr>\n')
                outfile.write('      <th style="border: 1px solid black; height:15px; width:100px;">run</th>\n')
                outfile.write('      <th style="border: 1px solid black; height:15px; width:100px;">file</th>\n')
                
                for ver in launchlist:
                    outfile.write('      <th style="border: 1px solid black; height:15px; width:100px;">VER' + "{:02d}".format(ver) + '</th>\n')

                outfile.write('    </tr>\n')
           
                hundreds_previous = int(run_num) // 100
            
            # end of int(run_num) // 100 != hundreds_previous
        # end of run_num != run_previous
        if is_new_run == True:
            #If new run, add a single line in between
            outfile.write('    <tr style="bgcolor:red; padding:25px blue;">\n')
            outfile.write('      <td style="padding:0px; height:8px;" colspan = "' + str(MAXVERSION+1-MINVERSION) + '"></td>\n')
            outfile.write('    </tr> \n')
            outfile.write('    <tr> \n')
            # reset flag
            is_new_run = False
        else:
            outfile.write('    <tr>\n')

        outfile.write('      <td style=" padding:15px; height:50px; width:100px; border:1px solid black;">' \
                          + "{:06d}".format(int(run_num)) + '</td>\n')
            
        outfile.write('      <td style=" padding:15px; height:50px; width:100px; border:1px solid black;">' \
                          + "{:03d}".format(int(file_num)) + '</td>\n')

        for ver in launchlist:
            formatted_vernum = "%02d" % ver
            if VERBOSE:
                print 'ver = ' + formatted_vernum + ' dict_final_states[' + formatted_vernum + '] = ' + dict_final_states[ver]

            loc_final_state = dict_final_states[ver]
            loc_problems    = dict_problems[ver]

            # determine text and bg color
            text      = ''
            textcolor = ''
            bgcolor   = ''
            ntotal[ver] += 1

            # Check how many types of problems there were
            problem_list = loc_problems.split(',')
            nprobs = 0
            if 'AUGER-OVER_RLIMIT' in problem_list:
                nprobs += 1
            if 'AUGER-TIMEOUT' in problem_list:
                nprobs += 1
            if any(re.match(regex_strs, loc_problems) for \
                       regex_strs in ['AUGER-SUBMIT', \
                                          'AUGER-FAILED', \
                                          'AUGER-INPUT-FAIL', \
                                          'AUGER-OUTPUT-FAIL', \
                                          'SWIF-SYSTEM-ERROR', \
                                          'SWIF-MISSING-OUTPUT']):
                nprobs += 1

            # --- 1. Loop over final states
            # SEGFAULT and NULL are indepedent of problems

            # SEGFAULT
            if loc_final_state == "SEGFAULT":
                text      = 'SEGFAULT'
                textcolor = 'black'
                bgcolor   = 'red'
                nsegfault[ver] += 1
            
            # NULL
            elif loc_final_state == "NULL":
                text      = 'NULL'
                textcolor = 'white'
                bgcolor = 'gray'
                nnull[ver] += 1

            # SUCCESS
            elif loc_final_state == "SUCCESS":
                text      = 'SUCCESS'
                textcolor = 'black'
                bgcolor   = '#66FF99'
                nsuccess[ver] += 1

            # ANYTHING ELSE IS CANCELLED
            # Depending on the problem  this will be modified later
            else :
                text      = 'CANCELLED'
                textcolor = 'yellow'
                bgcolor   = 'black'
                ncancelled[ver] += 1

            # --- 2. count number for each problem
            if loc_problems.find('AUGER-OVER_RLIMIT') > 0:
                nover_rlimit[ver] += 1
            if loc_problems.find('AUGER-TIMEOUT') > 0:
                ntimeout[ver] += 1
            if any(re.match(regex_strs, loc_problems) for \
                       regex_strs in ['AUGER-SUBMIT', \
                                          'AUGER-FAILED', \
                                          'AUGER-INPUT-FAIL', \
                                          'AUGER-OUTPUT-FAIL', \
                                          'SWIF-SYSTEM-ERROR', \
                                          'SWIF-MISSING-OUTPUT']):
                nsystem[ver] += 1

            # --- 3. Modify text if only one problem
            if nprobs == 1:
                # If final state was success, just change bgcolor
                if loc_final_state == 'SUCCESS':
                    if 'AUGER-OVER_RLIMIT' in loc_problems:
                        bgcolor   = 'yellow'
                    if 'AUGER-TIMEOUT' in loc_problems:
                        bgcolor   = 'orange'
                    if any(re.match(regex_strs, loc_problems) for \
                               regex_strs in ['AUGER-SUBMIT', \
                                                  'AUGER-FAILED', \
                                                  'AUGER-INPUT-FAIL', \
                                                  'AUGER-OUTPUT-FAIL', \
                                                  'SWIF-SYSTEM-ERROR', \
                                                  'SWIF-MISSING-OUTPUT']):
                        bgcolor   = '#E5E4E2'
                # If final state was the problem, show
                else:
                    if 'AUGER-OVER_RLIMIT' in loc_problems:
                        text      = 'OVER_RLIMIT'
                        bgcolor   = 'yellow'
                        textcolor = 'red'
                    if 'AUGER-TIMEOUT' in loc_problems:
                        text      = 'TIMEOUT'
                        bgcolor   = 'orange'
                        textcolor = 'white'
                    if any(re.match(regex_strs, loc_problems) for \
                               regex_strs in ['AUGER-SUBMIT', \
                                                  'AUGER-FAILED', \
                                                  'AUGER-INPUT-FAIL', \
                                                  'AUGER-OUTPUT-FAIL', \
                                                  'SWIF-SYSTEM-ERROR', \
                                                  'SWIF-MISSING-OUTPUT']):
                        text      = 'SYSTEM'
                        bgcolor   = '#E5E4E2'
                        textcolor = 'red'

            # --- 4. If there were multiple problems change the text color
            # and show all problems
            if nprobs > 1:
                text      = ''
                bgcolor = 'purple'
                # If the final state was success, change bgcolor
                if loc_final_state == 'SUCCESS':
                    text = 'SUCCESS'
                else:
                    if loc_problems.find('AUGER-OVER_RLIMIT'):
                        text += 'OVER_RLIMIT'
                    if loc_problems.find('AUGER-TIMEOUT'):
                        text += 'TIMEOUT'
                    if any(re.match(regex_strs, loc_problems) for \
                               regex_strs in ['AUGER-SUBMIT', \
                                                  'AUGER-FAILED', \
                                                  'AUGER-INPUT-FAIL', \
                                                  'AUGER-OUTPUT-FAIL', \
                                                  'SWIF-SYSTEM-ERROR', \
                                                  'SWIF-MISSING-OUTPUT']):
                        text += 'SYSTEM'
        
            if text == '' or textcolor == '' or bgcolor == '':
                print 'unknown result for final_state = ' + loc_final_state + ' problems = ' + loc_problems
                print 'text = ' + text + ' textcolor = ' + textcolor + ' bgcolor = ' + bgcolor
                print 'nprobs = ' + str(nprobs)
                exit()

            if VERBOSE:
                print 'final_state = ' + loc_final_state + ' problems = ' + loc_problems
                print 'text = ' + text + ' textcolor = ' + textcolor + ' bgcolor = ' + bgcolor

            # write out entry for this run and file
            outfile.write('      <td style=" padding:15px; height:50px; width:100px; border:1px solid black; background-color:' \
                              + bgcolor + '; color:' + textcolor + ';">' + text + '</td>\n')

        # end of loop over vers
        
        outfile.write('    </tr>\n')

    # end of loop over run and files
    
    # Finish table for each file fo final hundreds
    outfile.write('  </table>')
    outfile.write('\n')
    outfile.write('  <hr style="width:80%;align:center;height:3px;color:#ff0000;border-color:#ff0000;background-color:#ff0000;">\n')
    outfile.write('\n')
        
    if VERBOSE:
        for ver in launchlist:
            print 'ntotal['       + str(ver) + ' ] = ' + str(ntotal[ver])
            print 'nsuccess['     + str(ver) + ' ] = ' + str(nsuccess[ver])
            print 'nsegfault['    + str(ver) + ' ] = ' + str(nsegfault[ver])
            print 'nnull['        + str(ver) + ' ] = ' + str(nnull[ver])
            print 'ncancelled['     + str(ver) + ' ] = ' + str(ncancelled[ver])
            print 'nover_rlimit[' + str(ver) + ' ] = ' + str(nover_rlimit[ver])
            print 'ntimeout['     + str(ver) + ' ] = ' + str(ntimeout[ver])
            print 'nsystem['      + str(ver) + ' ] = ' + str(nsystem[ver])

    # Create summary table for final hundreds
    outfile.write('  <table style="border: 1px solid black; table-layout: fixed;">\n')
    outfile.write('    <tr>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Status for ' \
                      + "{:0>6d}".format(hundreds_previous * 100) + ' Runs</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Total (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Success (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Segfault (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Cancelled (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">NULL (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Over Limit (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">Timeout (%)</th>\n')
    outfile.write('      <th style="border: 1px solid black; height:15px; width:150px;">System Error (%)</th>\n')
    outfile.write('    </tr>\n')
    
    # Fill in summary table for final hundreds
    for ver in launchlist:
        outfile = open(outfilename,'a')

        outfile.write('    <tr>\n')
        # launch ver
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">VER' \
          + "{:0>2d}".format(ver) + '</td>\n')
        # ntotal
        value = 0 if ntotal[ver] == 0 else 100. * ntotal[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(ntotal[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # nsuccess
        value = 0 if ntotal[ver] == 0 else 100. * nsuccess[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(nsuccess[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # nsegfault
        value = 0 if ntotal[ver] == 0 else 100. * nsegfault[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(nsegfault[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # ncancelled
        value = 0 if ntotal[ver] == 0 else 100. * ncancelled[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(ncancelled[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # nnull
        value = 0 if ntotal[ver] == 0 else 100. * nnull[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
                          + "{:>4d}".format(nnull[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # nover_rlimit
        value = 0 if ntotal[ver] == 0 else 100. * nover_rlimit[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(nover_rlimit[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # ntimeout
        value = 0 if ntotal[ver] == 0 else 100. * ntimeout[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(ntimeout[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        # nsystem
        value = 0 if ntotal[ver] == 0 else 100. * nsystem[ver] / ntotal[ver]
        outfile.write('      <td style=" padding:15px; height:50px; width:150px; border:1px solid black;">' \
          + "{:>4d}".format(nsystem[ver]) + ' (' + "{:5.2f}".format(value) + ')</td>\n')

        outfile.write('    </tr>\n')

    outfile.write('</table>\n')
    # Finish summary table for previous hundreds
    outfile.write('<h2><a href = "../index.html">back to main</a></h2>\n')
    outfile.write('</body>\n')
    outfile.close()

    exit()

if __name__ == "__main__":
   main(sys.argv[1:])

