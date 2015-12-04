
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

def main(argv):

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "publish_offmon_results.py [run period] [version] [directory hdswif was run in] \n"
                                      + "example: publish_offmon_results.py 2015_03 15 \n"))
                          
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 2:
        print "Usage:"
        print "publish_offmon_results.py [run period] [version]\n"
        print "example: example: publish_offmon_results.py 2015_03 15 /home/gxproj5/halld/hdswif/ \n"
        exit()

    RUNPERIOD = args[0]
    VERSION   = args[1]
    XMLDIR    = '/home/' + os.environ['USER'] + '/halld/hdswif/'
    if not os.path.isdir(XMLDIR):
        print 'publish_offmon_results.py:'
        print 'Directory ' + XMLDIR + ' does not exist...'
        print 'aborting...'
        exit()

    OUTDIRBASE  = '/group/halld/www/halldweb/html/data_monitoring/launch_analysis/' + RUNPERIOD + '/launches/'
    DATASETNAME = 'offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + VERSION + '_hd_rawdata'

    HTMLFILE = XMLDIR + '/summary_swif_output_' + DATASETNAME + '.html'
    if not os.path.isfile(HTMLFILE):
        print 'publish_offmon_results.py:'
        print 'File ' + HTMLFILE + ' does not exist...'
        print 'aborting...'
        exit()

    # Copy html output file
    os.system('mkdir -p ' + OUTDIRBASE + DATASETNAME + '/')
    os.system('mkdir -p ' + OUTDIRBASE + DATASETNAME + '/figures/')
    os.system('cp -v ' + HTMLFILE + ' ' + OUTDIRBASE + DATASETNAME + '/')
    os.system('cp -v ' + XMLDIR + '/mystyle.css ' + OUTDIRBASE + DATASETNAME + '/')
    os.system('cp -rv ' + XMLDIR + '/figures/' + DATASETNAME + ' ' + OUTDIRBASE + DATASETNAME + '/figures/')

if __name__ == "__main__":
   main(sys.argv[1:])
