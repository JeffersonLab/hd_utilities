
#--------------------------------------------------------------------#
#                                                                    #
# 2015/08/06 Kei Moriya                                              #
#                                                                    #
# Create "job" table used for offline monitoring from swif           #
# output.                                                            #
#                                                                    #
# From swif output, the run, file, and Auger ID are                  #
# retrieved and inserted into the Job table.                         #
#                                                                    #
# Once this is done, all other information in the table              #
# (hostname, memoryRequested, timePending,walltime, etc.)            #
# can be retrieved from querying Auger using the jproj scripts.      #
#                                                                    #
# The database portions of this code have relied heavily on          #
# the processing scripts for offline monitoring that Sean has        #
# developed in                                                       #
# https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/process   #
#                                                                    #
#--------------------------------------------------------------------#

# TO DO:
# - options to specify table name
# - print out warning if table exists
# - bring in swif xml output
# - check that table is created and filled

from optparse import OptionParser
import os.path
import sys
import MySQLdb
import xml.etree.ElementTree as ET

def main(argv):

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "create_jproj_job_table.py [run period] [version] [directory for xml input file] \n"
                                      + "example: create_jproj_job_table.py 2015_03 15 /home/gxproj5/halld/hdswif/ \n"
                                      + "\n"
                                      + "Input xml file should be created with\n"
                                      + "hdswif.py summary [workflow]"))
    
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 3:
        print "Usage:"
        print "create_jproj_job_table.py [run period] [version] [directory for xml input file]"
        print "example: create_jproj_job_table.py 2015_03 15 /home/gxproj5/halld/hdswif/"
        exit()

    RUNPERIOD = args[0]
    VERSION   = args[1]
    XMLDIR    = args[2]

    # Create MySQL connection
    db_conn = MySQLdb.connect(host='hallddb', user='farmer', passwd='', db='farming')

    # Create table
    # Note that mySQL does not allow '-' in table names, so
    # run periods are expressed as e.g. 2015_03 instead of 2015-03
    table_name = 'offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + VERSION + '_hd_rawdata'

    # Check if the table already exists
    tablename  = "offline_monitoring_RunPeriod" + RUNPERIOD + "_ver" + VERSION + "_hd_rawdataJob"
    sqlcommand = "show tables like '" + tablename + "'"
    command = 'mysql -hhallddb -ufarmer farming -r -s -e "' + sqlcommand + '" > ___tmp_sql.txt'
    print 'sqlcommand = ' + sqlcommand
    print 'command    = ' + command

    os.system(command)
    tmp_sql_file_handler = open("___tmp_sql.txt",'r')
    for line in tmp_sql_file_handler:
        tablename = line.rstrip() # remove newline
        # print 'tablename = ', tablename
    os.system('rm -f ___tmp_sql.txt')

    if tablename != '':
        print 'Table ' + tablename + ' already exists.'
        answer = ''
        while answer != 'y' or answer != 'n':
            answer = raw_input('Recreate table? (y/n) ')
            if answer == 'y':
                # Drop Job table if it exists
                db_cmd = str("DROP TABLE IF EXISTS `" + table_name + "Job`")
                db_conn.cursor().execute(db_cmd)
                print 'dropped table ' + tablename
                break
            if answer == 'n':
                print 'Aborting...'
                exit()

    # Create Job table that holds Auger info
    db_cmd = str(
        "CREATE TABLE `" + table_name + "Job` (" +
        "`id` int(11) NOT NULL AUTO_INCREMENT," + 
        "`run` int(11) DEFAULT NULL," + 
        "`file` int(11) DEFAULT NULL," + 
        "`jobId` int(11) DEFAULT NULL," + 
        "`timeChange` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP," + 
        "`username` varchar(64) DEFAULT NULL," + 
        "`project` varchar(64) DEFAULT NULL," + 
        "`name` varchar(64) DEFAULT NULL," + 
        "`queue` varchar(64) DEFAULT NULL," + 
        "`hostname` varchar(64) DEFAULT NULL," + 
        "`nodeTags` varchar(64) DEFAULT NULL," + 
        "`coresRequested` int(11) DEFAULT NULL," + 
        "`memoryRequested` int(11) DEFAULT NULL," + 
        "`status` varchar(64) DEFAULT NULL," + 
        "`exitCode` int(11) DEFAULT NULL," + 
        "`result` varchar(64) DEFAULT NULL," + 
        "`timeSubmitted` datetime DEFAULT NULL," + 
        "`timeDependency` datetime DEFAULT NULL," + 
        "`timePending` datetime DEFAULT NULL," + 
        "`timeStagingIn` datetime DEFAULT NULL," + 
        "`timeActive` datetime DEFAULT NULL," + 
        "`timeStagingOut` datetime DEFAULT NULL," + 
        "`timeComplete` datetime DEFAULT NULL," + 
        "`walltime` varchar(8) DEFAULT NULL," + 
        "`cput` varchar(8) DEFAULT NULL," + 
        "`mem` varchar(64) DEFAULT NULL," + 
        "`vmem` varchar(64) DEFAULT NULL," + 
        "`script` varchar(1024) DEFAULT NULL," + 
        "`files` varchar(1024) DEFAULT NULL," + 
        "`error` varchar(1024) DEFAULT NULL," + 
        "PRIMARY KEY (`id`)" + 
        ") ENGINE=MyISAM;"
        )

    # print db_cmd

    # Create table
    db_conn.cursor().execute(db_cmd)
    print 'created table ' + tablename

    # Read in xml file and create tree, root
    xmlfilename = XMLDIR + '/swif_output_offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + VERSION + '_hd_rawdata.xml'

    if not os.path.isfile(xmlfilename):
        print 'XML output file ' + xmlfilename + ' does not exist'
        print 'Please create the xml file using hdswif.py summary [workflow]'
        exit()

    tree = ET.parse(xmlfilename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    njobs = 0

    # Get <name>, then <attempts> and info within
    for job in workflow_status.iter('job'):
        auger_id_text = ''
        run_num_text  = ''
        file_num_text = ''

        for user_run in job.iter('user_run'):
            run_num_text = str(user_run.text)
        for user_file in job.iter('user_file'):
            file_num_text = str(user_file.text)

        for auger_id in job.iter('auger_id'):
            # print "auger_id.tag = " + str(auger_id.tag) + " auger_id.text = " + str(auger_id.text)
            auger_id_text = str(auger_id.text)
            # print name_text + "   " + run_num + "   " + file_num + "   " + auger_id_text

        if auger_id_text != '' and run_num_text != '' and file_num_text != '':
            njobs += 1
            db_cmd = 'INSERT INTO ' + table_name + 'Job SET run=' + run_num_text + ", file=" + file_num_text + ", jobId = " + auger_id_text
            # print db_cmd
            db_conn.cursor().execute(db_cmd)
        else:
            print "This shouldn't happen..."
            print 'auger_id = ', auger_id_text, ' , run_num_text = ', run_num_text, ' , file_num_text = ', file_num_text

    print 'Filled table ' + tablename + ' with ' + str(njobs) + ' entries.'

# ------------------
# Below is the original code that was in parse_swif.py if
# the above does not work.
            
### # Get <name>, then <attempts> and info within
### for job in workflow_status:
###     # Find job
###     for name in job.iter('name'):
###     # Find auger_id
###     for auger_id in job.iter('auger_id'):
###         # print "auger_id.tag = " + str(auger_id.tag) + " auger_id.text = " + str(auger_id.text)
###         auger_id_text = str(auger_id.text)
###         # print name_text + "   " + run_num + "   " + file_num + "   " + auger_id_text
### 
###         #db_cmd = 'INSERT INTO ' + table_name + 'Job SET run=' + run_num + ", file=" + file_num + ", jobId = " + auger_id_text
###         # print db_cmd
###         #db_conn.cursor().execute(db_cmd)

if __name__ == "__main__":
   main(sys.argv[1:])
