
#--------------------------------------------------------------------#
#                                                                    #
# 2015/10/07 Kei Moriya                                              #
#                                                                    #
# Copy hdswif output html pages and figures to                       #
# web-accessible directories.                                        #
#                                                                    #
# Usage:                                                             #
# python publish_offmon_results.py                                   #
#    [run period] [version]                                          #
#                                                                    #
# The script assumes that the user of this script generated          #
# the hdswif output summary xml file, and that it is in              #
# ~/halld/hdswif/                                                    #
#                                                                    #
#--------------------------------------------------------------------#

from optparse import OptionParser
import os.path
import sys
import MySQLdb
import xml.etree.ElementTree as ET
import re

def main(argv):

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "publish_offmon_results.py [run period] [version] [directory hdswif was run in] \n"
                                      + "example: publish_offmon_results.py 2015_03 15 \n"))
                          
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 2:
        print "Usage:"
        print "publish_offmon_results.py [run period] [version]\n"
        print "example: example: publish_offmon_results.py 2015_03 15 /home/gxproj5/monitoring/hdswif/ \n"
        exit()

    RUNPERIOD = args[0]
    VERSION   = args[1]
    XMLDIR    = '/home/' + os.environ['USER'] + '/monitoring/hdswif/xml/'
    if not os.path.isdir(XMLDIR):
        print 'publish_offmon_results.py:'
        print 'XML file directory ' + XMLDIR + ' does not exist...'
        print 'aborting...'
        exit()

    HTMLDIR    = '/home/' + os.environ['USER'] + '/monitoring/hdswif/html/'
    if not os.path.isdir(HTMLDIR):
        print 'publish_offmon_results.py:'
        print 'html file directory ' + HTMLDIR + ' does not exist...'
        print 'aborting...'
        exit()

    OUTDIRBASE  = '/group/halld/www/halldweb/html/data_monitoring/launch_analysis/' + RUNPERIOD + '/launches/'
    DATASETNAME = 'offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + VERSION + '_hd_rawdata'

    # Get the launch date from the XML file
    tree = ET.parse(XMLDIR + 'swif_output_' + DATASETNAME + '.xml')
    workflow_status = tree.getroot()
    summary = workflow_status.find('summary')
    create_ts = summary.find('create_ts')
    create_date = ''
    create_date = (create_ts.text).split(' ')[0]
    print 'create_date = ' + create_date

    HTMLFILE = HTMLDIR + '/summary_swif_output_' + DATASETNAME + '.html'
    if not os.path.isfile(HTMLFILE):
        print 'publish_offmon_results.py:'
        print 'File ' + HTMLFILE + ' does not exist...'
        print 'aborting...'
        exit()

    # Copy html output file
    os.system('mkdir -p ' + OUTDIRBASE + DATASETNAME + '/')
    os.system('mkdir -p ' + OUTDIRBASE + DATASETNAME + '/figures/')
    os.system('cp -v ' + HTMLFILE + ' ' + OUTDIRBASE + DATASETNAME + '/')
    os.system('cp -v ' + HTMLDIR + '../mystyle.css ' + HTMLDIR + '/mystyle.css')
    os.system('cp -v ' + HTMLDIR + '/mystyle.css ' + OUTDIRBASE + DATASETNAME + '/')
    os.system('cp -rv ' + HTMLDIR + '/figures/' + DATASETNAME + ' ' + OUTDIRBASE + DATASETNAME + '/figures/')

    # Create link to the new file
    runperiodfilename = '/group/halld/www/halldweb/html/data_monitoring/launch_analysis/' + RUNPERIOD + '/' + RUNPERIOD + '.html'
    with open(runperiodfilename,'r') as runperiodfile:
        alllines = runperiodfile.readlines()

    # Reverse the lines and find the first occurrence of the hook
    # this is the final (most recent hook)
    alllines.reverse()

    nlines = 0
    pos = 0
    old_hook = ''
    for line in alllines:
        nlines += 1
        if re.search('<!-- ' + RUNPERIOD + ' ver[0-9][0-9] launch',line):
           pos = nlines
           old_hook = line
           break

    if old_hook == '':
        print 'Did not find old hook, make sure that file'
        print runperiodfilename
        print 'has a line containing <!-- [RUNPERIOD] ver[VER] launch'
        exit()
    
    # position to seek is total number of lines in file minus pos + 1
    pos = len(alllines) - pos + 1

    # create link lines
    newlines = []
    newlines.append(' <li style="color:blue;">\n')
    newlines.append('    <h3><a href = "launches/offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + str(VERSION) \
                        + '_hd_rawdata/summary_swif_output_offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + str(VERSION) \
                        + '_hd_rawdata.html">ver' + str(VERSION) + ' - ' + create_date + ' (SWIF)</a></h3> <!-- ' \
                        + RUNPERIOD + ' ver' + str(VERSION) + ' launch-->\n')

    
    # Reopen stats html file and insert newlines into line pos
    with open(runperiodfilename, 'r+') as runperiodfile:
        alllines = runperiodfile.readlines()

    alllines[pos:pos] = newlines

    # Write the new file
    with open(runperiodfilename, 'w') as runperiodfile:
        for line in alllines:
            runperiodfile.write(line)

if __name__ == "__main__":
   main(sys.argv[1:])
