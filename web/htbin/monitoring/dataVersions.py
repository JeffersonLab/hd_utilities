#!/usr/bin/python

import MySQLdb
import sys
import cgi
import cgitb
cgitb.enable()

import os

dbhost = "hallddb.jlab.org"
dbuser = 'datmon'
dbpass = ''
dbname = 'data_monitoring'

conn=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
curs=conn.cursor()

runPeriodList = [ 'RunPeriod-2014-10', 'RunPeriod-2015-01', 'RunPeriod-2015-03',
                  'RunPeriod-2015-06', 'RunPeriod-2015-12', 'RunPeriod-2016-02' ]

# print the HTTP header
def printHTTPheader():
    print "Content-type: text/html\n\n"


# print the HTML head section 
def printHTMLHead(title):
    print "<head>"
    print "    <title>"
    print title
    print "    </title>"

def printCSS():
    print "<style media='screen' type='text/css'>"
    print "th { padding-bottom: 5px; background-color: #aaaaaa;}"
    print "td { padding-bottom: 5px; }"
    print "</style>"

def printJavaScript():
    print "<script type='text/javascript'>"
    print """ function postForm()
              {
                 document.forms[0].submit();
              }  """
    print "</script>"

#return the option passed to the script
def get_options():
    form=cgi.FieldStorage()
    options = {}
    
    # default to latest run period
    options["run_period"] = runPeriodList[-1]
    if "runPeriod" in form:
        options["run_period"] = form["runPeriod"].value

    return options

# print the page header
def printPageHeader(theRunPeriod):
    print "<h1>GlueX Data Versions</h1>"
    print "<br/>"
    print "<b>Run Periods:</b>"  

    #print "<form><select name=\"rplist\" method=\"POST\" action=\"https://halldweb.jlab.org/cgi-bin/data_monitoring/monitoring/dataVersions.py\">"
    #print "<form method='POST' action='https://halldweb.jlab.org/cgi-bin/data_monitoring/monitoring/dataVersions.py'><select name=\"rplist\">"
    print "<form onchange='postForm()' method='POST' action='https://halldweb.jlab.org/cgi-bin/data_monitoring/monitoring/dataVersions.py'><select name=\"runPeriod\">"
    for runPeriod in runPeriodList:
        if runPeriod==theRunPeriod:
            print "  <option value=\"%s\" selected>%s</option>"%(runPeriod,runPeriod)
        else:
            print "  <option value=\"%s\">%s</option>"%(runPeriod,runPeriod)
    print "</select></form>"

    print "<hr>"

def getVersionData(run_period):
    cmd = "SELECT * FROM version_info WHERE run_period=%s ORDER BY data_type ASC, revision ASC"
    curs.execute(cmd, run_period)
    rows=curs.fetchall()
    return rows

#mysql> describe version_info;
#+-------------------+--------------+------+-----+---------+----------------+
#| Field             | Type         | Null | Key | Default | Extra          |
#+-------------------+--------------+------+-----+---------+----------------+
#| version_id        | int(11)      | NO   | PRI | NULL    | auto_increment |
#| data_type         | varchar(64)  | YES  |     | NULL    |                |
#| run_period        | varchar(64)  | YES  |     | NULL    |                |
#| revision          | int(11)      | YES  |     | NULL    |                |
#| software_version  | varchar(255) | YES  |     | NULL    |                |
#| jana_config       | varchar(255) | YES  |     | NULL    |                |
#| ccdb_context      | varchar(255) | YES  |     | NULL    |                |
#| production_time   | varchar(64)  | YES  |     | NULL    |                |
#| dataVersionString | varchar(255) | YES  |     | NULL    |                |
#| skim_name         | varchar(255) | YES  |     | NULL    |                |
#+-------------------+--------------+------+-----+---------+----------------+

def printTableHeader():
    print " <tr>"
    print "  <th>Data Type</th>"
    print "  <th>Revision</th>"
    print "  <th>CCDB Context</th>"
    print "  <th>Production Date</th>"
    print "  <th>Data Version String</th>"
    print "  <th>Software Versions</th>"
    print "  <th>JANA Config</th>"
    print " </tr>"

def printDataTable(run_period,data):
    basehtmldir = "https://halldweb.jlab.org/data_monitoring/run_conditions/"
    for field in data:
        #print "%s ver%d"%(field[1],field[3])

        #print str(field[4])
        print " <tr>"
        print "  <td>%s</td>"%field[1]
        print "  <td>ver%02d</td>"%field[3]
        print "  <td>%s</td>"%field[6]
        print "  <td>%s</td>"%field[7]
        print "  <td>%s</td>"%field[8]
        if field[4]=="<None>":
            field[4] == ""
            print "  <td>%s</td>"%field[4]
        else:
            print "  <td><a href=%s/%s/%s>%s</a></td>"%(basehtmldir,run_period,field[4],field[4])
        if field[5]=="<None>":
            field[5] == ""
            print "  <td>%s</td>"%field[5]
        else:
            print "  <td><a href=%s/%s/%s>%s</a></td>"%(basehtmldir,run_period,field[5],field[5])
        print " </tr>"


def printVersionData(run_period):
    print "<table style=\"width:100%; padding-bottom:25px\">"

    printTableHeader()
    data = getVersionData(run_period)
    printDataTable(run_period,data)

    print "</table>"

def printHelpText():
    #print "<p style='padding-bottom: 2px;'>"
    print "<p>"
    print "Description of above fields:"
    print "<table>"
    print "<tr><th>Field</th><th>Description</th></tr>"
    print "<tr><td>Data Type</td><td>The class of data: raw, reconstructed, simulated, or some special subset.</td></tr>"
    print "<tr><td>Revision</td><td>Integer corresponding to a pass through the data</td></tr>" 
    print "<tr><td>CCDB Context</td><td>The value of JANA_CALIB_CONTEXT, which specifies the version of calibration constants that were used</td></tr>" 
    print "<tr><td>Production Date</td><td>The data at which processing of this data began</td></tr>" 
    print "<tr><td>Data Version String</td><td>A unique string for identifying this data version.  This is used in EventStore</td></tr>" 
    print "<tr><td>Software Version</td><td>The XML file that specifies the software versions used</td></tr>" 
    print "<tr><td>JANA Config</td><td>The text file that specifies which JANA options were used</td></tr>" 

    print "</table>"
    print "</p>"

    print "For detailed information on these values, see GlueX Note 3062"


def main():
    # get options that may have been passed to this script
    options=get_options()

    # print the HTTP header
    printHTTPheader()

    # add some CSS
    printCSS()

    # add some JS
    printJavaScript()

    # start printing the page
    print "<html>"
    # print the head section including the table
    # used by the javascript for the chart
    printHTMLHead("GlueX Data Versions")

    print "</head>"

    # print the page body
    #print "<body style=\"overflow-y: hidden\" >"
    print "<body>"

    printPageHeader(options["run_period"])
    printVersionData(options["run_period"])

    print "<p style='padding-bottom: 45px;' > &nbsp; </p>"

    printHelpText()

    print "</body>"
    print "</html>"

    conn.close()

if __name__=="__main__":
    main()
