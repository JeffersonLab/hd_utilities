import xml.etree.ElementTree as ET
import re
import MySQLdb

#--------------------------------------------------------------------#
#                                                                    #
# 2015/07/23 Kei Moriya                                              #
#                                                                    #
# Parse the xml output of swif.                                      #
#                                                                    #
# The root element of swif output is an element called               #
# <status>                                                           #
#                                                                    #
# Within <status>, each job is put into an element called            #
#   <job>                                                            #
#                                                                    #
# Each <job> element has child elements                              #
#     <id>                                                           #
#     <name>                                                         #
#     <status>                                                       #
#     <attempts>                                                     #
#                                                                    #
# If a job is dispatched, a child element                            #
#       <attempt>                                                    #
# will be created within <attempts> each time.                       #
#                                                                    #
# Within <attempt>, there will be 39 child elements                  #
# describing the job status and statistics.                          #
#                                                                    #
#--------------------------------------------------------------------#

# Read in xml file and create tree, root
tree = ET.parse('output.xml')
# root element is <status>
status = tree.getroot()

print "status.tag = " + str(status.tag) + "   status.text = " + str(status.text)

# Within <status> there are <job> tags
#for job in status:
#    print "job.tag = " + str(job.tag) + "job.text = " + str(job.text)
#    for job_child in job:
#        print "job_child.tag = " + str(job_child.tag) + "   job_child.text = " + str(job_child.text)

# Can find any grandchild element
nSuccess = 0
for auger_state in status.iter('auger_final_state'):
    # print "auger_state.text = " + auger_state.text
    if(auger_state.text == "SUCCESS"):
        nSuccess+= 1

print "Number of success: " + str(nSuccess)

# Create MySQL connection
db_conn = MySQLdb.connect(host='hallddb', user='farmer', passwd='', db='farming')
table_name = 'offline_monitoring_RunPeriod2015_03_ver90_hd_rawdata'

# Drop Job table if it exists
db_cmd = str("DROP TABLE IF EXISTS `" + table_name + "Job`")
# print db_cmd
# db_conn.cursor().execute(db_cmd)

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
# db_conn.cursor().execute(db_cmd)

# Get <name>, then <attempts> and info within
for job in status:
    # Find job
    for name in job.iter('name'):
        # print "name.tag = " + str(name.tag) + " name.text = " + str(name.text)
        name_text = str(name.text)
        match = re.match(r"offmon_(\d\d\d\d\d\d)_(\d\d\d)",name_text)
        run_num  = match.group(1)
        file_num = match.group(2)

    # Find auger_id
    for auger_id in job.iter('auger_id'):
        # print "auger_id.tag = " + str(auger_id.tag) + " auger_id.text = " + str(auger_id.text)
        auger_id_text = str(auger_id.text)
        # print name_text + "   " + run_num + "   " + file_num + "   " + auger_id_text

        db_cmd = 'INSERT INTO ' + table_name + 'Job SET run=' + run_num + ", file=" + file_num + ", jobId = " + auger_id_text
        # print db_cmd
        # db_conn.cursor().execute(db_cmd)

    # Find rtime
    rtime_text = ""
    for rtime in job.iter('rtime'):
        # print "rtime.tag = " + str(rtime.tag) + " rtime.text = " + str(rtime.text)
        rtime_text = str(rtime.text)

    # Find stime
    stime_text = ""
    for stime in job.iter('stime'):
        # print "stime.tag = " + str(stime.tag) + " stime.text = " + str(stime.text)
        stime_text = str(stime.text)

    # Find utime
    utime_text = ""
    for utime in job.iter('utime'):
        # print "utime.tag = " + str(utime.tag) + " utime.text = " + str(utime.text)
        utime_text = str(utime.text)

    # Find maxrss
    maxrss_text = ""
    for maxrss in job.iter('maxrss'):
        # print "maxrss.tag = " + str(maxrss.tag) + " maxrss.text = " + str(maxrss.text)
        maxrss_text = str(maxrss.text)

    # Find auger_cpu_sec
    auger_cpu_sec_text = ""
    for auger_cpu_sec in job.iter('auger_cpu_sec'):
        # print "auger_cpu_sec.tag = " + str(auger_cpu_sec.tag) + " auger_cpu_sec.text = " + str(auger_cpu_sec.text)
        auger_cpu_sec_text = str(auger_cpu_sec.text)

    # Find auger_mem_kb
    auger_mem_kb_text = ""
    for auger_mem_kb in job.iter('auger_mem_kb'):
        # print "auger_mem_kb.tag = " + str(auger_mem_kb.tag) + " auger_mem_kb.text = " + str(auger_mem_kb.text)
        auger_mem_kb_text = str(auger_mem_kb.text)

    # Find auger_wall_sec
    auger_wall_sec_text = ""
    for auger_wall_sec in job.iter('auger_wall_sec'):
        # print "auger_wall_sec.tag = " + str(auger_wall_sec.tag) + " auger_wall_sec.text = " + str(auger_wall_sec.text)
        auger_wall_sec_text = str(auger_wall_sec.text)

    # Comparison of swif and Auger resource usage.
    # Save this output as txt file and analyze.
    if(rtime_text != ""):
        print str(rtime_text) + "   " + str(stime_text) + "   " + str(utime_text) + "   " + str(maxrss_text) + "   " + str(auger_cpu_sec_text) + "   " + str(auger_mem_kb_text) + "   " + str(auger_wall_sec_text)
