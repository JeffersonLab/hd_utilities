#!/usr/bin/python

import MySQLdb
import cgi
import cgitb
cgitb.enable()

import datetime
import re

import os
os.environ["RCDB_HOME"] = "/group/halld/www/halldweb/html/rcdb_home"
import sys
sys.path.append("/group/halld/www/halldweb/html/rcdb_home/python")
import rcdb
import glob

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
    
    revision_str = str(options[1])
    revision = int(re.search(r'\d+', revision_str).group())
    query = "SELECT distinct r.run_num, r.start_time, r.num_events from run_info r, version_info v, bcal_hits b WHERE b.runid=r.run_num and v.version_id=b.version_id and run_num>0 and revision=%s and run_period=%s ORDER BY r.run_num"
    #if revision_str == "ver00":
    #print "ver00 query"
    #query = "SELECT distinct r.run_num, r.start_time, r.num_events from run_info r, version_info v WHERE run_num>0 and revision=%s and run_period=%s ORDER BY r.run_num"
    curs.execute(query, (revision, str(options[2])))    
    rows=curs.fetchall()

    return rows

# get data from the database for single run
def get_data_singlerun(options):

    revision_str = str(options[1])
    revision = int(re.search(r'\d+', revision_str).group())
    #revision_str = revision_str.replace("ver","")
    #revision = int(float(revision_str))
    query = "SELECT distinct r.run_num, r.start_time, r.num_events, r.beam_current, r.radiator_type, r.solenoid_current, r.trigger_config_file from run_info r, version_info v, bcal_hits b WHERE b.runid=r.run_num and v.version_id=b.version_id and run_num>0 and revision=%s and r.run_num=%s ORDER BY r.run_num"
    #if revision == 0:
	#query = "SELECT distinct r.run_num, r.start_time, r.num_events, r.beam_current, r.radiator_type, r.solenoid_current, r.trigger_config_file from run_info r, version_info v WHERE run_num>0 and revision=%s and r.run_num=%s ORDER BY r.run_num"
    curs.execute(query, (revision, options[0]))
    rows=curs.fetchall()

    return rows

# get list of versions from the DB
def get_versions(options):

    query = "SELECT revision, data_type, production_time, run_period from version_info where run_period=%s ORDER BY version_id DESC"
    curs.execute(query, (str(options[2])))
    rows=curs.fetchall()

    return rows

# get list of periods from the DB
def get_periods(options):

    query = "SELECT DISTINCT run_period from version_info ORDER BY run_period DESC"
    curs.execute(query)
    rows=curs.fetchall()

    return rows

# get period for given run_number from the DB
def get_periods_run_number(options):

    query = "SELECT DISTINCT run_period from version_info v, run_info r, bcal_hits b WHERE b.runid=r.run_num and v.version_id=b.version_id and r.run_num=%s ORDER BY run_period DESC"
    curs.execute(query, (str(options[0])))
    rows=curs.fetchall()

    return rows

# get dates to list on runBrowser page
def get_dates(options):
   
    revision_str = str(options[1])
    revision = int(re.search(r'\d+', revision_str).group())
    #revision_str = revision_str.replace("ver","")
    #revision = int(float(revision_str)) 
    query = "SELECT DISTINCT DATE(r.start_time) FROM run_info r, version_info v WHERE run_num>0 and start_time>'2014-11-01' and revision=%s and run_period=%s ORDER BY DATE(start_time)"
    curs.execute(query, (revision, str(options[2])))
    rows=curs.fetchall()

    return rows


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
	  div.link-list {
          width:20.0em;
	  height: 98%;
	  position:absolute;
          padding-left:1%;
          padding-right:1%;
          margin-left:0;
          margin-right:0;
	}
	#main {
	  height: 101%;
          margin-left:21.0em;
          padding-left:1em;
          padding-right:2em;
	  overflow-y: scroll;
	}
	#nav {
          left:0;
	  overflow-y: scroll;
	}

        /* Menu styles */
        .link-list ul
        {
        margin:0px;
        padding:0px;
        }
        .link-list li
        {
        margin:0px 0px 0px 5px;
        padding:0px;
        list-style-type:none;
        text-align:left;
        font-weight:normal;
        }

        /* Symbol styles */
        .link-list .symbol-item,
        .link-list .symbol-open,
        .link-list .symbol-close
        {
        float:left;
        width:16px;
        height:1em;
        background-position:left center;
        background-repeat:no-repeat;
        }
        .link-list .symbol-open  { background-image:url(https://halldweb.jlab.org/data_monitoring/js/runBrowser/icons/minus.png); }
        .link-list .symbol-close { background-image:url(https://halldweb.jlab.org/data_monitoring/js/runBrowser/icons/plus.png);}

        /* Menu line styles */
        .link-list li.open  { font-weight:bold; }
        .link-list li.close { font-weight:normal; }
      </style>
"""

    print "<script src=\"https://halldweb.jlab.org/data_monitoring/js/runBrowser/TreeMenu.js\" type=\"text/javascript\"></script>"
    print "<script src=\"https://halldweb.jlab.org/data_monitoring/js/jquery.js\" type=\"text/javascript\"></script>"

    print """
      <script type="text/javascript">
        var freeze = false;

        function showPlot(ver, name, period)
	{
	  var imgsrc = "https://halldweb.jlab.org/work/halld2/data_monitoring/"
          imgsrc += period;
          imgsrc += "/";
          imgsrc += ver;
          imgsrc += "/Run";
	  imgsrc += $(this.run_number).val()
	  imgsrc += "/";
	  imgsrc += name;
	  imgsrc += ".png";
          if(!freeze) {
	    document.getElementById('imageshow').src=imgsrc;
            document.getElementById('imageshow').style.display='block';
	  }
        }

        function freezeIt()
        {
          if(freeze) freeze = false;
          else freeze = true;
        }
      </script>

      <script>
      function changeText(id,text)
      {
       document.getElementById(id).innerHTML = text;
      }
      </script>
      
      <script>
        function changePeriod()
	{
           $("#ver").load("/data_monitoring/textdata/" + $(this.period).val() + ".txt");
        }
      </script>

      <script>
        function changeRun(run, runNumber)
	{
           document.getElementById('run_number').value = "0"
           document.getElementById('run_number').value += run
           myVar = "Run "+runNumber+":"
           changeText("RunLine",myVar)
        }
      </script>
"""
    #window.alert(document.getElementById("period").value);
    
def print_version_selector(options):
    print """<form action="/cgi-bin/data_monitoring/monitoring/runBrowser.py" method="POST">"""
    
    periods = get_periods(options)
    print "<select id=\"period\" name=\"period\" onChange=\"changePeriod()\">" 
    for period in periods:
        if period[0] == "RunPeriod-2015-01":
            continue;
        print "<option value=\"%s\" " % (period[0])
        if options != None and period[0] == options[2]:
            print "selected"
        print "> %s</option>" % (period[0])
    print "</select>"

    recon_versions = []

    versions = get_versions(options)
    print "<select id=\"ver\" name=\"ver\">" 
    for version in versions:
        revision = ("ver%02d" % version[0])
        data_type = version[1]
        production_time = version[2]
        full_version_name = "%s_%s" % (data_type, revision)
        if version[0] not in recon_versions or data_type != "recon":
            print "<option value=\"%s\" " % full_version_name
        if options != None and full_version_name == options[1]:
            if data_type != "recon":
                print "selected"
            elif version[0] not in recon_versions:
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
            if version[0] in recon_versions:
                print "> %s %s</option>" % (revision, version_name)
                continue
            version_name = "Recon Launch "
            version_name += production_time
            recon_versions.append(version[0])
        elif data_type == "mc":
            version_name = "MC Production "
            version_name += production_time
        print "> %s %s</option>" % (revision, version_name)
    
    print "</select>"
    print "<input type=\"submit\" value=\"Display\" />"
    print "Link displays plots or <button type=\"button\"> ROOT </button> opens file"
    print "<br><br>"  

def print_run_selector(records, options, row):
    months = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"]
    print "<ul id=\"runList\">"
    dates = get_dates(options)
    rcdb_runs = db.select_runs("@status_approved and @is_production")
    rcdb_run_numbers = [ run.number for run in rcdb_runs ]

    if "mc_ver01" in options:
        newText="MC"
        print ("<label><input type=\"radio\" id=\"run_number\" name=\"run_number\" value=\"%s\" onclick=\"changeRun(%s,%s);\"> %s</label>" % (10000, 10000, "MC_run", 10000))
        #print ("<a href=\"/cgi-bin/data_monitoring/monitoring/browseJSRoot.py?run_number=%s&ver=%s&period=%s\" target=\"_blank\"><button type=\"button\"> ROOT </button></a>" % (row[0], options[1], options[2]))
        #print ("<a href=\"https://halldweb.jlab.org/rcdb/runs/info/%s\" target=\"_blank\"><button type=\"button\"> RCDB </button></a>" % (row[0]))
        return

    #print dates
    for date in dates:
        if date[0] == None: 
            continue;

        # format date (must be a better way)
        fulldate = str(date[0]) 
        month = fulldate[5:7]
        day = fulldate[8:10]
        namedate = "%s %s" % (months[int(month)-1], day)      
 
        # get run range
        minRun = 9e9
        maxRun = 0
        for row in records:
            if row[1] == None or row[1] == '0':
                continue

            if "recon" in options[1] and row[0] not in rcdb_run_numbers:
                continue

	    rundate_obj = None
	    try:
            	rundate_obj = datetime.datetime.strptime(row[1], "%Y-%m-%d %H:%M:%S")
	    except ValueError:
		pass
	    if rundate_obj == None:
		continue
            #print rundate_obj

	    try:
            	rundate = rundate_obj.strftime("%Y-%m-%d")
            except ValueError:
		pass
	    #print rundate
	    
            if rundate == fulldate:
                if row[0] < minRun:
                    minRun = row[0]
                if row[0] > maxRun:
                    maxRun = row[0]
                
        if minRun != 9e9 and maxRun != 0:
            print "<li>"
            print "<b>%s</b> (Run %s-%s)" % (namedate, minRun, maxRun)
            print "<ul>"
            
            # print runs for given date
            for row in records:
                if row[1] == None or row[1] == '0':
                    continue
		if "recon" in options[1] and row[0] not in rcdb_run_numbers:
                    continue

		rundate_obj = None
                try:
    	            rundate_obj = datetime.datetime.strptime(row[1], "%Y-%m-%d %H:%M:%S")
                except ValueError:
            	    pass
                if rundate_obj == None:
                    continue
                #print rundate_obj

                try:
                    rundate = rundate_obj.strftime("%Y-%m-%d")
                except ValueError:
                    pass
                #print rundate

                numevents = row[2]

                if rundate == fulldate:
                    if options[2] == 'RunPeriod-2016-02':
                        if db.get_condition(row[0], "event_count"):
                            numevents = db.get_condition(row[0], "event_count").value
                    print "<li>"
                    #print records
                    print ("<label><input type=\"radio\" id=\"run_number\" name=\"run_number\" value=\"%s\" onclick=\"changeRun(%s,%s)\"> %s (%s eve)</label>" % (row[0], row[0], row[0], row[0], numevents))
                    print ("<a href=\"/cgi-bin/data_monitoring/monitoring/browseJSRoot.py?run_number=%s&ver=%s&period=%s\" target=\"_blank\"><button type=\"button\"> ROOT </button></a>" % (row[0], options[1], options[2]))
                    print ("<a href=\"https://halldweb.jlab.org/rcdb/runs/info/%s\" target=\"_blank\"><button type=\"button\"> RCDB </button></a>" % (row[0]))
                    print "</li>"
                    
            print "</ul>"
            print "</li>"
    
    print "</ul>"

    print "<script type=\"text/javascript\">make_tree_menu('runList');</script>"


#print row of table to display histograms
def print_row(options, charts):
    for chart in charts:
        print "<td style='text-align:center' bgcolor='#A9E2F3' onclick=\"freezeIt(); showPlot(\'%s\', \'%s\', \'%s\');\" onmouseover=\"showPlot(\'%s\', \'%s\', \'%s\'); this.style.backgroundColor='#F78181';\" onmouseout=\"this.style.backgroundColor='#A9E2F3';\">%s</td>" % (options[1], chart[0], options[2], options[1], chart[0], options[2], chart[1])
    print "</tr>" 


#return the option passed to the script
def get_options():
    form=cgi.FieldStorage()
    run_number_str = []
    run_number = []
    
    verName = "recon_ver01"
    periodName = "RunPeriod-2016-02"

    if "ver" in form:
        verName = str(form["ver"].value)
    if "period" in form:
        periodName = str(form["period"].value)
    if "run_number" in form:
        run_number_str.append(form["run_number"].value)
    if len(run_number_str) == 1 and run_number_str[0].isalnum():
	run_number.append(int(run_number_str[0]))
        run_number.append(verName)
        run_number.append(periodName)
        return run_number
    else:
        run_number.append(None)
        run_number.append(verName)
        run_number.append(periodName)
    
    return run_number


def get_path(options):
    runpth="./"
    if options[0] != None:
        runpth = "/work/halld2/data_monitoring/"
        runpth += options[2]
        runpth += "/"
        runpth += options[1]
        runpth += "/Run"
        length = len(str(options[0]))
        for i in range(6-length):
            runpth += "0"
        runpth += str(options[0])
        runpth += "/"
    return runpth


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
    printHTMLHead("Offline Data Monitoring: Run Browser")

    print "</head>"

    # print the page body
    print "<body style=\"overflow-y: hidden\" >"

    #print "<b id=test>Test Text"
    #print "</b>"
    #print "<br>"
    #myvar = "test"
    #print "<input type=\"radio\" name=\"group1\" value=\"blah\" onclick=changeText(\"test\",\"%s\")> My button" % (myvar)
    #print "<br>"
    
   #set period and version if only run_number is given
    if options[0] != None:
        # set period first
        period=get_periods_run_number(options)
        options[2]=period[0][0]
        # set version
        if options[1] == None:
            versions=get_versions(options)
            revision = ("%s_ver%d" % (versions[0][1], versions[0][0]))
            options[1]=revision


    
    # print version selector
    print """<div id="nav" class="link-list">"""
    print_version_selector(options)
    
    # print run selector form
    records=get_data(options)
    #if records == None: 
    #    records.append([10267, "", 10])
    print_run_selector(records, options, records)
    print "</form>"
    print "</div class=\"link-list\">"

    # print main page with plots if run number selected
    print """<div id="main">"""

    #if options[0] == None:
    #print "Select run number link to show histograms or click <button type=\"button\"> ROOT </button> to open ROOT file in browser."
    #print "</div>"
    #print "</body>"
    #print "</html>"
    #sys.exit() 

    # get revision number 
    revision_str = str(options[1])
    revision = int(re.search(r'\d+', revision_str).group())
    #revision_str = revision_str.replace("ver","")
    #revision = int(float(revision_str))

    isRecon = "recon" in revision_str
#######################################################################3    
    #define and initialize charts
    occupancy_charts = [["OCCUPANCY","OCCUPANCY"]]
    occupancy_charts.pop(0)
    cdc_charts = [["CDC","CDC"]]
    cdc_charts.pop(0)
    bcal_charts = [["BCAL","BCAL"]]
    bcal_charts.pop(0)
    fcal_charts = [["FCAL","FCAL"]]
    fcal_charts.pop(0)
    tof_charts = [["TOF","TOF"]]
    tof_charts.pop(0)
    fdc_charts = [["FDC","FDC"]]
    fdc_charts.pop(0)
    st_charts = [["ST","ST"]]
    st_charts.pop(0)
    tagm_charts = [["TAGM","TAGM"]]
    tagm_charts.pop(0)
    tagh_charts = [["TAGH","TAGH"]]
    tagh_charts.pop(0)
    hldetectortiming_charts = [["HLTIMING","HLTIMING"]]
    hldetectortiming_charts.pop(0)
    ps_charts = [["PS","PS"]]
    ps_charts.pop(0)
    l1_charts = [["L1","L1"]]
    l1_charts.pop(0)
    rf_charts = [["RF","RF"]]
    rf_charts.pop(0)
    ana_charts1 = [["ANA1","ANA1"]]
    ana_charts1.pop(0)
    ana_charts2 = [["ANA2","ANA2"]]
    ana_charts2.pop(0)
    ana_charts3 = [["ANA3","ANA3"]]
    ana_charts3.pop(0)
    tpol_charts = [["TPOL","TPOL"]]
    tpol_charts.pop(0)

    folder_path = get_path(options)
    print folder_path
    print "<br>"
    #for each file in the folder find the corresponding textfile entry and sort into the subdetector table
    if os.path.isdir(folder_path):
        os.chdir(folder_path)
        for file in glob.glob("*.png"):
            is_found = False
            #print file
            #print "<br>"
            filename = ""
            dispname = ""
            with open('/u/group/halld/www/halldweb/htbin/data_monitoring/monitoring/figure_titles','r') as f:
                for line in f:
                    filename = line.split(',', 2)[0]
                    dispname = line.split(',', 2)[1]
                    if filename == file:
                        is_found = True
                        break
                    
            if is_found == False:
                #print file
                #print "================================"
                #print "<br>"
                continue
            else:
                if filename.find("p2pi")>-1 or filename.find("p3pi")>-1:
#                    print "   --found ANA1 <br>"
                    ana_charts3.append([filename[:-4],dispname])
                    continue

                if filename.find("Reconstruction")>-1 or filename.find("Num")>-1 or filename.find("EventInfo")>-1 or filename.find("Matching")>-1 or filename.find("Track")>-1:
#                    print "   --found ANA1 <br>"
                    ana_charts1.append([filename[:-4],dispname])
                    continue

                if filename.find("_p1")>-1 or filename.find("_p2")>-1:
                #    print "   --found ANA1 <br>"
                    ana_charts2.append([filename[:-4],dispname])
                    continue

                if filename.find("_occupancy")>-1 or filename.find("Occupancy")>-1:
#                    print "   --found Occupancy <br>"
                    occupancy_charts.append([filename[:-4],dispname])
                    continue

                if filename.find("L1")>-1 or filename.find("l1")>-1:
#                    print "   --found L1 <br>"
                    l1_charts.append([filename[:-4],dispname])
                    continue

                if filename.find("CDC")>-1 or filename.find("cdc")>-1:
#                    print "   --found cdc <br>"
                    cdc_charts.append([filename[:-4],dispname])
                    continue
                
                if (filename.find("BCAL")>-1 or filename.find("bcal")>-1) and filename.find("Reconstruction") == -1:
#                    print "   --found bcal <br>"
                    bcal_charts.append([filename[:-4],dispname])
                    continue
                
                if (filename.find("FCAL")>-1 or filename.find("fcal")>-1) and (filename.find("Reconstruction")<0 and filename.find("Matching")<0):
#                    print "   --found fcal <br>"
                    fcal_charts.append([filename[:-4],dispname])
                    continue
                
                if filename.find("TOF")>-1 or filename.find("tof")>-1 and (filename.find("Reconstruction")<0 and filename.find("occupancy")<0):
#                    print "   --found tof <br>"
                    tof_charts.append([filename[:-4],dispname])
                    continue
                
                if (filename.find("FDC")>-1 or filename.find("fdc")>-1) and filename.find("occupancy")<0:
#                    print "   --found fdc <br>"
                    fdc_charts.append([filename[:-4],dispname])
                    continue
               
                if (filename.find("ST_")>-1 or filename.find("st_")>-1) and filename.find("occupancy")<0:
#                    print "   --found st <br>"
                    st_charts.append([filename[:-4],dispname])
                    continue
                
                if filename.find("TAGM")>-1 or filename.find("tagm")>-1:
#                    print "   --found tagm <br>"
                    tagm_charts.append([filename[:-4],dispname])
                    continue
                
                if filename.find("TAGH")>-1 or filename.find("tagh")>-1:
#                    print "   --found tagh <br>"
                    tagh_charts.append([filename[:-4],dispname])
                    continue
                
                if filename.find("Alignment")>-1 or filename.find("Timing")>-1:
#                    print "   --found hldetect <br>"
                    hldetectortiming_charts.append([filename[:-4],dispname])
                    continue

                if filename.find("PS")>-1 or filename.find("ps_")>-1 or filename.find("TAG_")>-1:
#                    print "   --found ps <br>"
                    ps_charts.append([filename[:-4],dispname])
                    continue

                if filename.find("RF_")>-1:
#                    print "   --found RF <br>"
                    rf_charts.append([filename[:-4],dispname])
                    continue

                if filename.find("TPOL")>-1:
#                    print "   --found RF <br>"
                    tpol_charts.append([filename[:-4],dispname])
                    continue


            print filename
            print "<br>"

            #print name
            #print "<br>"
    else:
        print "This run number does not exist for the given run period/version number!!!!"
        print "<br>"
        print "<br>"
        print "<br>"
        print "<br>"
####################################################################################
    # set names to "rootspy" if these are online histograms 
    #if revision == 0:
        #for chart in cdc_charts:
        #    chart[0] = chart[0].replace("__CDC_cdc","_rootspy__rootspy_CDC_cdc")
        #for chart in fdc_charts:
        #    chart[0] = chart[0].replace("__FDC","_rootspy__rootspy_FDC")
        #for chart in fcal_charts:
        #    chart[0] = chart[0].replace("__fcal","_rootspy__rootspy_fcal")
        #for chart in tof_charts:
        #    chart[0] = chart[0].replace("__tof","_rootspy__rootspy_tof")
        #for chart in st_charts:
        #    chart[0] = chart[0].replace("__st_st","_rootspy__rootspy_st_st")
        #for chart in tagm_charts:
        #    chart[0] = chart[0].replace("__tagm_tagm","_rootspy__rootspy_tagm_tagm")
        #for chart in tagh_charts:
        #    chart[0] = chart[0].replace("__TAGH","_rootspy__rootspy_TAGH")
        #for chart in ana_charts1:
        #    chart[0] = chart[0].replace("__Independent","_rootspy__rootspy_Independent")

    # display all possible charts in table for selection
    print """Mouse over <font style="background-color: #A9E2F3">light blue</font> entries in table to view histograms, and <b>click</b>  on an entry to freeze/unfreeze a specific historgram. <br>"""
    print """<table style="width:200px; font-size:0.8em">
      <tr>"""

    #if revision == 0:

    print "<td>Online Occupancies: </td>"
    print_row(options, occupancy_charts)

    if not isRecon:
        print "<td>CDC: </td>"
        print_row(options, cdc_charts)
        print "<td>FDC: </td>"
        print_row(options, fdc_charts)
        print "<td>BCAL: </td>" 
        print_row(options, bcal_charts)
        print "<td>FCAL: </td>" 
        print_row(options, fcal_charts)
        print "<td>TOF: </td>" 
        print_row(options, tof_charts)
        print "<td>SC/ST: </td>" 
        print_row(options, st_charts)
        print "<td>TAGM:</td>"
        print_row(options, tagm_charts)
        print "<td>TAGH:</td>"
        print_row(options, tagh_charts)
        print "</table>"
        
        if revision > 4 and options[2] == 'RunPeriod-2015-03' or options[2] == 'RunPeriod-2015-06' or options[2] == 'RunPeriod-2015-12' or options[2] == 'RunPeriod-2016-02':
            print """<table style="width:200px; font-size:0.8em">
	   <tr>"""
            print "<td>RF:</td>"
            print_row(options, rf_charts)
            print "</table>"
            
        if (revision > 5 and options[2] == 'RunPeriod-2015-03') or (revision > 15 and options[2] == 'RunPeriod-2014-10') or options[2] == 'RunPeriod-2015-06' or options[2] == 'RunPeriod-2015-12' or options[2] == 'RunPeriod-2016-02':
            print """<table style="font-size:0.8em">
	   <tr>"""
            print "<td>HLDetectorTiming:</td>"
            print_row(options, hldetectortiming_charts)
            print "</table>"
        
    if revision > 3 and options[2] == 'RunPeriod-2015-03' or options[2] == 'RunPeriod-2015-06' or options[2] == 'RunPeriod-2015-12' or options[2] == 'RunPeriod-2016-02':
        print """<table style="width:200px; font-size:0.8em">
	   <tr>"""
        print "<td>PS:</td>"
        print_row(options, ps_charts)
        print "<td>L1:</td>"
        print_row(options, l1_charts)
        print "</table>"
        
    print """<table style="width:200px; font-size:0.8em">
       <tr>"""
    print "<td>RECO:  </td>" 
    print_row(options, ana_charts1)
    print "<td></td>"
    print_row(options, ana_charts2)
    print "</table>"
    
    if not isRecon:
        print """<table style="width=200px; font-size:0.8em">
      <tr>"""
        print "<td>ANA:</td>" 
        print_row(options, ana_charts3)
             
    print "</table>"

    record_singlerun=get_data_singlerun(options)
    #print "<br> Run %s" % (options[0]) # record_singlerun[3])
    
    print "<br>"
    print "<b id='RunLine'> <b>Run %s:</b> </b>" % (options[0])
 
    # display histogram
    print """<img width=700px src="" id="imageshow" style="display:none">"""
    print "</div>"

    print "</body>"
    print "</html>"

    conn.close()

if __name__=="__main__":
    main()
