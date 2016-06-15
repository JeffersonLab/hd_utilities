#!/usr/bin/python

import MySQLdb
import sys
import cgi
import cgitb
cgitb.enable()

import re

import os
os.environ["RCDB_HOME"] = "/group/halld/www/halldweb/html/rcdb_home"
import sys
sys.path.append("/group/halld/www/halldweb/html/rcdb_home/python")
import rcdb

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
                print "<td style='text-align:center; font-size:1.5em' >"
                print "<b><a href=\"/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"  target=\"_blank\"> Run %s </b>" % (temp_run, verName, periodName, temp_run)
                print "</td>"
            print "</tr>"
            print "<tr>"
            for temp_run in temp_runs:
                print "<td>"
                web_link = "https://halldweb.jlab.org/work/halld/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, verName, temp_run, plotName)
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
        web_link = "https://halldweb.jlab.org/work/halld/data_monitoring/%s/%s/Run%06d/%s.png" % (periodName, verName, temp_run, plotName)
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
    print """<form action="/cgi-bin/data_monitoring/monitoring/plotBrowser.py" method="POST">"""

    plotNames = [["CDC_occupancy","CDC Occupancy"]]
    if options[3] == "ver00":
        plotNames.append(["FDC_occupancy","FDC Occupancy"])
	plotNames.append(["FCAL_occupancy","FCAL Occupancy"])
        plotNames.append(["BCAL_occupancy","BCAL Occupancy"])
        plotNames.append(["PS_occupancy","PS Occupancy"])
        plotNames.append(["RF_TPOL_occupancy","RF & TPOL Occupancy"])
        plotNames.append(["ST_occupancy","ST Occupancy"])
        plotNames.append(["TAGGER_occupancy","TAGGER Occupancy"])
        plotNames.append(["TOF_occupancy","TOF Occupancy"])
    else:
        plotNames.append(["__CDC_cdc_raw_intpp","CDC Raw Integral"])
        plotNames.append(["__CDC_cdc_raw_t","CDC Raw Time"])
        plotNames.append(["__CDC_cdc_ped","CDC Ped"])
        plotNames.append(["__CDC_cdc_raw_intpp_vs_n","CDC Raw Integral vs straw"])
        plotNames.append(["__CDC_cdc_raw_t_vs_n","CDC Raw Time vs straw"])
        plotNames.append(["__CDC_cdc_ped_vs_n","CDC Ped vs straw"])
        plotNames.append(["__CDC_cdc_windata_ped_vs_n","CDC WinData Ped vs straw"])
	plotNames.append(["FDC_P1_pseudo_occupancy","FDC Package 1 Pseudo"])
        plotNames.append(["FDC_P2_pseudo_occupancy","FDC Package 2 Pseudo"])
	plotNames.append(["FDC_P3_pseudo_occupancy","FDC Package 3 Pseudo"])
	plotNames.append(["FDC_P4_pseudo_occupancy","FDC Package 4 Pseudo"])
	plotNames.append(["bcal_summary","BCAL Summary"])
        plotNames.append(["bcal_times","BCAL Timing"])
        plotNames.append(["bcal_occupancy","BCAL Occupancy"])
        plotNames.append(["bcal_cluster","BCAL Cluster"])
        plotNames.append(["bcal_shower","BCAL Shower"])
        plotNames.append(["bcal_hist_eff","BCAL Efficiency"])
        plotNames.append(["bcal_inv_mass","BCAL 2-photon mass"])
        plotNames.append(["bcal_fcal_inv_mass","BCAL-FCAL 2-photon mass"])
        plotNames.append(["__fcal_digHitE","FCAL Digi Pulse Integral"])
        plotNames.append(["__fcal_digOcc2D","FCAL Digi Occupancy"])
        plotNames.append(["__fcal_digT","FCAL Digi Time"])
        plotNames.append(["fcal_hit_energy","FCAL Hit Summary"])
        plotNames.append(["fcal_hit_timing","FCAL Hit Time"])
        plotNames.append(["fcal_cluster_et","FCAL Cluster Energy-Time"])
        plotNames.append(["fcal_cluster_space","FCAL Cluster Space"])
        plotNames.append(["__tof_tofe","TOF Energy"])
        plotNames.append(["__tof_toft","TOF Time"])
        plotNames.append(["__tof_tofo1","TOF Occupancy Plane1"])
        plotNames.append(["__tof_tofo2","TOF Occupancy Plane2"])
        plotNames.append(["__st_st_pi_dhit","SC/ST Digi Pulse Integral"])
        plotNames.append(["__st_st_pt_dhit","SC/ST Digi Time"])
        plotNames.append(["__st_st_sec_adc_dhit","SC/ST Digi Occupancy"])
        plotNames.append(["__tagm_tagm_adc_pint","TAGM Digi Pulse Integral"])
        plotNames.append(["__tagm_tagm_adc_mult","TAGM Digi Multiplicity"])
        plotNames.append(["__tagm_tagm_hit_seen","TAGM Hit Occupancy"])
        plotNames.append(["__tagm_tagm_hit_time","TAGM Hit Time"])
        plotNames.append(["__TAGH_DigiHit_PulseIntegral","TAGH Digi Pulse Integral (ver < 12)"])
        plotNames.append(["__TAGH_DigiHit_tdcTime","TAGH Digi TDC Time (ver < 12)"])
        plotNames.append(["__TAGH_DigiHit_PedestalVsSlotID","TAGH Digi Pedestal vs Slot (ver < 12)"])
        plotNames.append(["__TAGH_DigiHit_DigiHit_PulseIntegral","TAGH Digi Pulse Integral (ver == 12)"])
        plotNames.append(["__TAGH_DigiHit_DigiHit_tdcTime","TAGH Digi TDC Time"])
        plotNames.append(["__TAGH_DigiHit_DigiHit_PedestalVsSlotID","TAGH Digi Pedestal vs Slot"])
        plotNames.append(["__TAGH_DigiHit_DigiHit_RawIntegral","TAGH Digi Raw Integral"])
        plotNames.append(["TAGH_hit","TAGH Hit Summary"])
        plotNames.append(["trig_fcalbcal","Trigger Energy Balance FCAL vs BCAL"])
        plotNames.append(["PSC_hit","PSC Hit Summary 1"])
        plotNames.append(["PSC_hit2","PSC Hit Summary 2"])
        plotNames.append(["PSC_hit3","PSC Hit Summary 3"])
        plotNames.append(["PS_hit","PS Hit Summary 1"])
        plotNames.append(["PS_hit2","PS Hit Summary 2"])
        plotNames.append(["__PSPair_PSC_PS_PS_E","PS Pair Energy"])
        plotNames.append(["PS_PSC_coinc","PS/PSC Pair Coincidences"])
        plotNames.append(["PS_eff","PS(fine) Efficiency"])
        plotNames.append(["PS_TAG_energy","PS/Tagger Energy Correlation"])
    #plotNames.append(["TAG_eff","Tagging Efficiency"])
    #plotNames.append(["TAG_2D_eff","Tagging Efficiency 2D"])
        plotNames.append(["HistMacro_RF_p1","RF Summary 1"])
        plotNames.append(["HistMacro_RF_p2","RF Summary 2"])
        plotNames.append(["HistMacro_RF_p3","RF Summary 3"])
        plotNames.append(["HistMacro_TaggerTiming","HLDetectorTiming Tagger Timing"])
        plotNames.append(["HistMacro_TaggerRFAlignment","HLDetectorTiming Tagger-RF"])
        plotNames.append(["HistMacro_TaggerSCAlignment","HLDetectorTiming Tagger-SC"])
        plotNames.append(["HistMacro_CalorimeterTiming","HLDetectorTiming FCAL/BCAL"])
        plotNames.append(["HistMacro_PIDSystemTiming","HLDetectorTiming SC/TOF"])
        plotNames.append(["HistMacro_TrackMatchedTiming","HLDetectorTiming Track Matched Timing"])
        plotNames.append(["HistMacro_EventInfo","Event Info"])
        plotNames.append(["HistMacro_NumLowLevelObjects_p1","Low Level Objects 1"])
        plotNames.append(["HistMacro_NumLowLevelObjects_p2","Low Level Objects 2"])
        plotNames.append(["HistMacro_NumHighLevelObjects","High Level Objects"])
        plotNames.append(["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","Track Multiplicity"])
        plotNames.append(["HistMacro_Tracking_p1","Track Summary 1"])
        plotNames.append(["HistMacro_Tracking_p2","Track Summary 2"])
        plotNames.append(["HistMacro_Tracking_p3","Track Summary 3"])	
        plotNames.append(["HistMacro_Matching_p1","Matching Summary 1 (ver < 12)"])
        plotNames.append(["HistMacro_Matching_p2","Matching Summary 2 (ver < 12)"])
        plotNames.append(["HistMacro_Matching_BCAL","Matching Summary BCAL"])
        plotNames.append(["HistMacro_Matching_FCAL","Matching Summary FCAL"])
        plotNames.append(["HistMacro_Matching_SC","Matching Summary SC/ST"])
        plotNames.append(["HistMacro_Matching_TOF","Matching Summary TOF"])
        plotNames.append(["HistMacro_Kinematics_p1","Reco Kinematics 1"])
        plotNames.append(["HistMacro_Kinematics_p2","Reco Kinematics 2"])
        plotNames.append(["HistMacro_FCALReconstruction_p1","FCAL Shower Energy-Time"])
        plotNames.append(["HistMacro_FCALReconstruction_p2","FCAL Shower Match"])
        plotNames.append(["HistMacro_FCALReconstruction_p3","FCAL &beta; and E/p"])
        plotNames.append(["HistMacro_BCALReconstruction_p1","BCAL Shower Energy-Time"])
        plotNames.append(["HistMacro_BCALReconstruction_p2","BCAL Shower Match"])
        plotNames.append(["HistMacro_BCALReconstruction_p3","BCAL &beta; and E/p"])
        plotNames.append(["HistMacro_SCReconstruction_p1","SC/ST Match 1"])
        plotNames.append(["HistMacro_SCReconstruction_p2","SC/ST Match 2"])
        plotNames.append(["HistMacro_SCReconstruction_p3","SC/ST Match 3"])
        plotNames.append(["HistMacro_TOFReconstruction_p1","TOF Match 1"])
        plotNames.append(["HistMacro_TOFReconstruction_p2","TOF Match 2"])
        plotNames.append(["HistMacro_p2pi_pmiss","&pi;<sup>+</sup>&pi;<sup>-</sup> Missing Proton"])
        plotNames.append(["HistMacro_p2pi_preco","&pi;<sup>+</sup>&pi;<sup>-</sup> Reconstructed Proton"])
        plotNames.append(["HistMacro_p3pi_preco_2FCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup> (2FCAL)"])
        plotNames.append(["HistMacro_p3pi_preco_FCAL-BCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup> (F/BCAL)"])

    

    print "Select Run Period:"
    periods = get_periods(options)
    print "<select id=\"period\" name=\"period\" onChange=\"changePeriod()\">" 
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
        if options != None and full_version_name == options[1]:
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
        print "<input type=\"text\" name=\"run1\" />"
        print "<input type=\"text\" name=\"run2\" />"
    else:
        print "<input type=\"text\" value=\"%s\" name=\"run1\" />" % (options[0])
        print "<input type=\"text\" value=\"%s\" name=\"run2\" />" % (options[1])
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
    rcdb_query = "@is_production"

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
