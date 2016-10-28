#!/usr/bin/python

import MySQLdb
import sys
import cgi
import cgitb
cgitb.enable()

import re

import os
import os.path
os.environ["RCDB_HOME"] = "/group/halld/www/halldweb/html/rcdb_home"
import sys
sys.path.append("/group/halld/www/halldweb/html/rcdb_home/python")
import rcdb

#from os import listdir
import glob
#from os.path import isfile, join

db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb")

dbhost = "hallddb.jlab.org"
dbuser = 'datmon'
dbpass = ''
dbname = 'data_monitoring'

conn=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
curs=conn.cursor()

# get data from the database
# if an interval is passed,
# return a list of records from the database
def get_data(options):
    
    revision_str = str(options[3])
    revision = int(re.search(r'\d+', revision_str).group())
    if options[0] == None:
        query = "SELECT distinct r.run_num from run_info r, version_info v, fdc_hits c WHERE c.runid=r.run_num and v.version_id=c.version_id and run_num>0 and start_time>0 and revision=%s and run_period=%s %s ORDER BY r.run_num"
        curs.execute(query, (str(revision), str(options[5]), str(options[4])))
    else:
        query = "SELECT distinct r.run_num from run_info r, version_info v, fdc_hits c WHERE c.runid=r.run_num and v.version_id=c.version_id and run_num>=%s and run_num<=%s and start_time>0 and revision=%s and run_period=%s %s ORDER BY r.run_num"
        curs.execute(query, (str(options[0]), str(options[1]), str(revision), str(options[5]), str(options[4])))
    rows=curs.fetchall()

    return rows

# get list of versions from the DB
def get_versions(options):

    if options == None:
        query = "SELECT revision, data_type, production_time, run_period from version_info ORDER BY version_id DESC"
        curs.execute(query)
    else:
        query = "SELECT revision, data_type, production_time, run_period from version_info where run_period=%s ORDER BY version_id DESC"
        curs.execute(query, (str(options[5]))) 
    rows=curs.fetchall()

    unique=[]
    #unique.append(rows[1])
    #unique.append(rows[2])
    dup = 0
    for row in rows:
	for ent in range(0,len(unique)):
		if unique[ent][0] == row[0] and unique[ent][1]==row[1]:			
			dup=1
			break
	if dup == 1:
		dup=0
		continue
	else:
		unique.append(row)

    rows=unique

    return rows

# get list of periods from the DB
def get_periods(options):

    query = "SELECT DISTINCT run_period from version_info ORDER BY run_period DESC"
    curs.execute(query)
    rows=curs.fetchall()

    return rows

def show_plots(records, plotName, verName, periodName, rcdb_query):
    #print "<table border=\"1\">"
    #print "<tr>"
    
    loc_i = 0
    temp_runs = []

    if periodName == 'RunPeriod-2016-02':
        rcdb_runs = db.select_runs(rcdb_query)
        rcdb_run_numbers = [ run.number for run in rcdb_runs ]

    for run in records:

        # filter out non-production runs
        if periodName == 'RunPeriod-2016-02' and run[0] not in rcdb_run_numbers:
            continue

            #if db.get_condition(run[0], "beam_current") and db.get_condition(run[0], "event_count") and db.get_condition(run[0], "daq_run"):
            #    beam_current = db.get_condition(run[0], "beam_current").value
            #    event_count = db.get_condition(run[0], "event_count").value
            #    daq_run = db.get_condition(run[0], "daq_run").value
            #    if beam_current < 2 or (daq_run not in ['PHYSICS', 'EXPERT']) or event_count < 500000:
            #        continue
        
        temp_runs.append(run[0])

        if (loc_i+1) % 3 == 0:
            print "<table border=\"1\">"
            print "<tr>"
            for temp_run in temp_runs:
		#PATH="/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"  target=\"_blank\"> Run %s </b>" % (temp_run, verName, periodName, temp_run)
                #if not os.path.isfile(PATH):
		#	continue
		print "<td style='text-align:center; font-size:1.5em' >"
                print "<b><a href=\"/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"  target=\"_blank\"> Run %s </b>" % (temp_run, verName, periodName, temp_run)
                print "</td>"
            print "</tr>"
            print "<tr>"
            for temp_run in temp_runs:
		#PATH="/work/halld2/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, verName, temp_run, plotName)
		#if not os.path.isfile(PATH):
		#	continue
                print "<td>"
                web_link = "https://halldweb.jlab.org/work/halld2/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, verName, temp_run, plotName)
                print "<img width=400px src=\"%s\" onclick=\"window.open('%s', '_blank')\" >" % (web_link, web_link)
                print "</td>"
            print "</tr>"
            print "</table>"
            print "<br>"
            print ""

            temp_runs = []

        loc_i = loc_i+1

    # show remaining plots which didn't make a full row of 3
    print "<table border=\"1\">"
    print "<tr>"
    for temp_run in temp_runs:
        print "<td style='text-align:center; font-size:1.5em' >"
        print "<b><a href=\"/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"  target=\"_blank\"> Run %s </b>" % (temp_run, verName, periodName, temp_run)
        print "</td>"
    print "</tr>"
    print "<tr>"
    for temp_run in temp_runs:
        print "<td>"
        web_link = "https://halldweb.jlab.org/work/halld2/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, verName, temp_run, plotName)
        print "<img width=400px src=\"%s\" onclick=\"window.open('%s', '_blank')\" >" % (web_link, web_link)
        print "</td>"
    print "</tr>"
    print "</table>"
    print "<br>"
    print ""

    #print "</table>"


# print the HTTP header
def printHTTPheader():
    print "Content-type: text/html\n\n"


# print the HTML head section with java script for handling histogram display
def printHTMLHead(title):
    print "<head>"
    print "    <title>"
    print title
    print "    </title>"
    print """
	<style>
        figcaption {
        text-align: center;
        font-size: 175%;
        }
	</style>
    """

    print "<script src=\"https://halldweb.jlab.org/data_monitoring/js/jquery.js\" type=\"text/javascript\"></script>"

    print """
	<script>
        function changePeriod()
	{
           $("#ver").load("/data_monitoring/textdata/" + $(this.period).val() + ".txt");

         var chosenPeriod=$(this.period).val()

	var runlow=0;
	var runhigh=999999999;	
   
	if(chosenPeriod == "RunPeriod-2014-10")
	{
            runlow=1;
            runhigh=9000;
        
	}else if(chosenPeriod == "RunPeriod-2015-03")
         {
	    runlow=2600;
            runhigh=9000;
	}else if(chosenPeriod == "RunPeriod-2015-06")
	{	    
	    runlow=3400;
            runhigh=9000;
	}else if( chosenPeriod == "RunPeriod-2015-12")
         {
	    runlow=4000;
            runhigh=9000;
	}else if(chosenPeriod == "RunPeriod-2016-02")
	{
	    runlow=10000;
            runhigh=19999;
	}
	
	document.getElementById("runlow").value=runlow;
	document.getElementById("runhigh").value=runhigh;
	
	}
        </script>

      """

def print_option_selector(options):
    print """<form action="/cgi-bin/data_monitoring/monitoring/plotBrowser.py" method="POST">"""



 #   mypath = "/work/halld2/data_monitoring/RunPeriod-2016-02/recon_ver02/Run011366/"
    plotNames = [["CDC_occupancy","CDC Occupancy"]]
    plotNames.pop(0)
    #os.chdir(mypath)
    #for file in glob.glob("*.png"):
    #    name = file[:-4]
    
    with open('./figure_titles','r') as f:
        for line in f:
            words=line.split(',', 3)
            filename = words[0][:-4]
            dispname = words[1]#line.split(',', 2)[1]
            #print filename
            #print "   "
            #print dispname
            #print "<br>"
            plotNames.append([filename[0:],dispname[0:]])
            
    plotNames.sort(key=lambda x: x[1])
    print "Select Run Period:"
    periods = get_periods(options)
    print "<select id=\"period\" name=\"period\" onChange=\"changePeriod();\">" 
    for period in periods:
        if period[0] == "RunPeriod-2015-01":
            continue;
        print "<option value=\"%s\" " % (period[0])
        if period[0] == options[5]:
            print "selected"
        print "> %s</option>" % (period[0])
    print "</select>"

    print "and Recon. Version:"
    versions = get_versions(options)
    print "<select id=\"ver\" name=\"ver\">"
    for version in versions:
        revision = ("ver%02d" % version[0])
        data_type = version[1]
        production_time = version[2]
        full_version_name = "%s_%s" % (data_type, revision)
        print "<option value=\"%s\" " % full_version_name
	print str(full_version_name)
	print str(options[3])        
	if options != None and str(full_version_name) == str(options[3]):
            print "selected"
        version_name = ""
        if version[0] == 0 and data_type == "rawdata":
            version_name = "RootSpy"
        elif data_type == "mon":
            if version[0] == 1:
                version_name = "Incoming Data"
            else:
                version_name = "Monitoring Launch "
                version_name += production_time
        elif data_type == "recon":
            version_name = "Recon Launch "
            version_name += production_time
        elif data_type == "mc":
            version_name = "MC Production "
            version_name += production_time
        print "> %s %s</option>" % (revision, version_name)
    print "</select>"
    print "<br>"

    print "Select plot to display:"
    print "<select name=\"plot\">"
    for plotName in plotNames:
        print "<option value=\"%s\" " % (plotName[0])
        if options != None and plotName[0] == options[2]:
            print "selected"
        print ">%s</option>" % (plotName[1])
    print "</select>"
    print """ and run number range to query:"""
    if options == None:
        print "<input type=\"text\" id=\"runlow\" name=\"run1\" />"
        print "<input type=\"text\" id=\"runhigh\" name=\"run2\" />"
    else:
        print "<input type=\"text\" id=\"runlow\" value=\"%s\" name=\"run1\" />" % (options[0])
        print "<input type=\"text\" id=\"runhigh\" value=\"%s\" name=\"run2\" />" % (options[1])
    print "<input type=\"submit\" value=\"Display\" />"

    print "<br>"
    print """Add additional MYSQL query requirements as string:"""
    if options == None:
        print "<input type=\"text\" name=\"query\" size=\"40\" />"
    else:
        print "<input type=\"text\" name=\"query\" size=\"40\" value=\"%s\" />" % (options[4])
    print "eg. and beam_current>20 and solenoid_current>1190"

    print "<br>"
    print """Add additional RCDB query requirements as string:"""
    if options == None:
        print "<input type=\"text\" name=\"query\" size=\"40\" />"
    else:
        print "<input type=\"text\" name=\"rcdb_query\" size=\"40\" value=\"%s\" />" % (options[6])
    print "eg. @is_production"

    print "<br>"
    print "<b> Note: </b> Click on figure to open larger image in new tab, or click on Run # to open runBrowser page for that Run."
    print "</form>"


#return the option passed to the script
def get_options():
    form=cgi.FieldStorage()
    run_number_str = []
    run_number = []

    plotName = "CDC_occupancy"
    verName = "recon_ver01"
    periodName = "RunPeriod-2016-02"
    query = ""
    rcdb_query = "@is_production and @status_approved"

    if "plot" in form:
        plotName = str(form["plot"].value)
    if "ver" in form:
        verName = str(form["ver"].value)
    if "period" in form:
        periodName = str(form["period"].value)
    if "query" in form:
        query = str(form["query"].value)
    if "rcdb_query" in form:
        rcdb_query = str(form["rcdb_query"].value)
    if "run1" in form:
        run_number_str.append(form["run1"].value)
    if "run2" in form:
        run_number_str.append(form["run2"].value)
    if len(run_number_str) == 2 and run_number_str[0].isalnum() and run_number_str[1].isalnum():
        if int(run_number_str[1]) > int(run_number_str[0]):
            for run in run_number_str:
                run_number.append(int(run))
            run_number.append(plotName)
            run_number.append(verName)
            run_number.append(query)
            run_number.append(periodName)
            run_number.append(rcdb_query)
            return run_number
        else:
            return None
    else:
        if periodName == "RunPeriod-2014-10":
            run_number.append(1)
            run_number.append(9000)
        elif periodName == "RunPeriod-2015-03":
            run_number.append(2600)
            run_number.append(9000)
	elif periodName == "RunPeriod-2015-06":
	    run_number.append(3400)
            run_number.append(9000)
        elif periodName == "RunPeriod-2015-12":
            run_number.append(4000)
            run_number.append(9000)
	else:
	    run_number.append(10000)
            run_number.append(19999)
        run_number.append(plotName)
        run_number.append(verName)
        run_number.append(query)
        run_number.append(periodName)
        run_number.append(rcdb_query)
        return run_number


# main function
# This is where the program starts
def main():

    # get options that may have been passed to this script
    options=get_options()

    # print the HTTP header
    printHTTPheader()

    # start printing the page
    print "<html>"
    # print the head section including the table
    # used by the javascript for the chart
    printHTMLHead("Offline Data Monitoring: Plot Browser")

    print "</head>"

    # print the page body
    print "<body style=\"overflow-y: hidden\" >"
    print "<h1>Offline Data Monitoring: Plot Browser</h1>"
    
    # print option selector form
    print_option_selector(options)

    print "<hr>"

    print """<div style="height: 75%; overflow-y: scroll;">"""

    if options[0] == None:
	print "</div>"
        print "</body>"
        print "</html>"
	sys.exit() 

    # print run selector form
    records=get_data(options)

    # display histograms
    show_plots(records, options[2], options[3], options[5], options[6])

    print "</div>"
    print "</body>"
    print "</html>"

    conn.close()

if __name__=="__main__":
    main()
