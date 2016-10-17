#!/usr/bin/python

import MySQLdb
import sys
import cgi
import cgitb
cgitb.enable()

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
    
    query = "SELECT distinct revision, data_type, production_time from run_info r, version_info v, fdc_hits c WHERE c.runid=r.run_num and v.version_id=c.version_id and start_time>0 and run_num=%s and run_period=%s ORDER BY v.version_id desc"
    curs.execute(query, (str(options[0]), str(options[2])))
    rows=curs.fetchall()

    return rows

# get list of versions from the DB
def get_versions(options):

    if options == None:
        query = "SELECT revision, data_type, production_time, run_period from version_info ORDER BY version_id DESC"
        curs.execute(query)
    else:
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

def show_plots(records, runName, plotName, periodName):
    #print "<table border=\"1\">"
    #print "<tr>"

    loc_i = 0
    temp_vers = []

    for ver in records:
        temp_vers.append(ver)

        if (loc_i+1) % 3 == 0:
            print "<table border=\"1\">"
            print "<tr>"
            for temp_ver in temp_vers:
                revision = ("ver%02d" % temp_ver[0])
                full_version_name = "%s_%s" % (temp_ver[1], revision)
                print "<td style='text-align:center; font-size:1.5em' >"
                print "<b><a href=\"/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"  target=\"_blank\"> %s (%s)</b>" % (runName, full_version_name, periodName, full_version_name, temp_ver[2])
                print "</td>"
            print "</tr>"
            print "<tr>"
            for temp_ver in temp_vers:
                print "<td>"
                revision = ("ver%02d" % temp_ver[0])
                full_version_name = "%s_%s" % (temp_ver[1], revision)
                web_link = "https://halldweb.jlab.org/work/halld2/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, full_version_name, runName, plotName)
                print "<img width=400px src=\"%s\" onclick=\"window.open('%s', '_blank')\" >" % (web_link, web_link)
                print "</td>"
            print "</tr>"
            print "</table>"
            print "<br>"
            print ""

            temp_vers = []

        loc_i = loc_i+1

    # show remaining plots which didn't make a full row of 3
    print "<table border=\"1\">"
    print "<tr>"
    for temp_ver in temp_vers:
        revision = ("ver%02d" % temp_ver[0])
        full_version_name = "%s_%s" % (temp_ver[1], revision)
        print "<td style='text-align:center; font-size:1.5em' >"
        print "<b><a href=\"/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"  target=\"_blank\"> %s (%s)</b>" % (runName, full_version_name, periodName, full_version_name, temp_ver[2])
        print "</td>"
    print "</tr>"
    print "<tr>"
    for temp_ver in temp_vers:
        print "<td>"
        revision = ("ver%02d" % temp_ver[0])
        full_version_name = "%s_%s" % (temp_ver[1], revision)
        web_link = "https://halldweb.jlab.org/work/halld2/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, full_version_name, runName, plotName)
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
        }
        </script>
    """

def print_option_selector(options):
    print """<form action="/cgi-bin/data_monitoring/monitoring/versionBrowser.py" method="POST">"""
    
    plotNames = [["CDC_occupancy","CDC Occupancy"]]
    plotNames.pop(0)
    #os.chdir(mypath)                                                                                                                                                                                             
    #for file in glob.glob("*.png"):                                                                                                                                                                              
    #    name = file[:-4]                                                                                                                                                                                         

    with open('./figure_titles','r') as f:
        for line in f:
            filename = line.split(',', 2)[0][:-4]
            dispname = line.split(',', 2)[1]
            #print filename                                                                                                                                                                                       
            #print "   "                                                                                                                                                                                          
            #print dispname                                                                                                                                                                                       
            #print "<br>"                                                                                                                                                                                         
            plotNames.append([filename[0:],dispname[0:]])

    print "Select Run Period:"
    periods = get_periods(options)
    print "<select id=\"period\" name=\"period\" onChange=\"changePeriod()\">" 
    for period in periods:
        if period[0] == "RunPeriod-2015-01":
            continue;
        print "<option value=\"%s\" " % (period[0])
        if period[0] == options[2]:
            print "selected"
        print "> %s</option>" % (period[0])
    print "</select><br>"

    print "Select plot to display:"
    print "<select name=\"plot\">"
    for plotName in plotNames:
        print "<option value=\"%s\" " % (plotName[0])
        if options != None and plotName[0] == options[1]:
            print "selected"
        print ">%s</option>" % (plotName[1])
    print "</select>"
    print """ and run number:"""
    if options == None:
        print "<input type=\"text\" name=\"run_number\" />"
    else:
        print "<input type=\"text\" value=\"%s\" name=\"run_number\" />" % (options[0])
    print "<input type=\"submit\" value=\"Display\" />"


#return the option passed to the script
def get_options():
    form=cgi.FieldStorage()
    run_number_str = []
    run_number = []

    plotName = "HistMacro_BCALReconstruction_p3"
    periodName = "RunPeriod-2016-02"

    if "plot" in form:
        plotName = str(form["plot"].value)
    if "period" in form:
        periodName = str(form["period"].value)
    if "run_number" in form:
        run_number_str.append(form["run_number"].value)
    if len(run_number_str) == 1 and run_number_str[0].isalnum() :
        run_number.append(int(run_number_str[0]))
        run_number.append(plotName)
        run_number.append(periodName)
        return run_number
    else:
        if periodName == "RunPeriod-2014-10":
            run_number.append(1)
        elif periodName == "RunPeriod-2015-03":
            run_number.append(2600)
        elif periodName == "RunPeriod-2015-06":
            run_number.append(3400)
        else:
            run_number.append(10391)
        run_number.append(plotName)
        run_number.append(periodName)
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
    printHTMLHead("Offline Data Monitoring: Version Browser")

    print "</head>"

    # print the page body
    print "<body style=\"overflow-y: hidden\" >"
    print "<h1>Offline Data Monitoring: Version Browser</h1>"

    # print option selector form
    print_option_selector(options)
    print "<hr>"

    print """<div style="height: 83%; overflow-y: scroll;">"""

    if options[0] == None:
	print "</div>"
        print "</body>"
        print "</html>"
	sys.exit() 

    # print run selector form
    records=get_versions(options)

    # display histograms
    show_plots(records, options[0], options[1], options[2])

    print "</div>"
    print "</body>"
    print "</html>"

    conn.close()

if __name__=="__main__":
    main()
