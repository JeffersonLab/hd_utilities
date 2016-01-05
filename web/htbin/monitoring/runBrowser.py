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
    
    revision_str = str(options[1])
    revision_str = revision_str.replace("ver","")
    revision = int(float(revision_str))
    query = "SELECT distinct r.run_num, r.start_time, r.num_events from run_info r, version_info v, bcal_hits b WHERE b.runid=r.run_num and v.version_id=b.version_id and run_num>0 and revision=%s and run_period=%s ORDER BY r.run_num"
    curs.execute(query, (revision, str(options[2])))    
    rows=curs.fetchall()

    return rows

# get data from the database for single run
def get_data_singlerun(options):

    revision_str = str(options[1])
    revision_str = revision_str.replace("ver","")
    revision = int(float(revision_str))
    query = "SELECT distinct r.run_num, r.start_time, r.num_events, r.beam_current, r.radiator_type, r.solenoid_current, r.trigger_config_file from run_info r, version_info v, bcal_hits b WHERE b.runid=r.run_num and v.version_id=b.version_id and run_num>0 and revision=%s and r.run_num=%s ORDER BY r.run_num"
    curs.execute(query, (revision, options[0]))
    rows=curs.fetchall()

    return rows

# get list of versions from the DB
def get_versions(options):

    query = "SELECT revision, dataVersionString, run_period from version_info where run_period=%s ORDER BY revision DESC"
    curs.execute(query, (str(options[2])))
    rows=curs.fetchall()

    return rows

# get list of periods from the DB
def get_periods(options):

    query = "SELECT DISTINCT run_period from version_info ORDER BY run_period DESC"
    curs.execute(query)
    rows=curs.fetchall()

    return rows

# get dates to list on runBrowser page
def get_dates(options):
   
    revision_str = str(options[1])
    revision_str = revision_str.replace("ver","")
    revision = int(float(revision_str)) 
    query = "SELECT DISTINCT DATE(r.start_time) FROM run_info r, version_info v, bcal_hits b WHERE b.runid=r.run_num and v.version_id=b.version_id and run_num>0 and start_time>'2014-11-01' and revision=%s  and run_period=%s GROUP BY start_time"
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

        function showPlot(run, ver, name, period)
	{
	  var imgsrc = "https://halldweb.jlab.org/work/halld/data_monitoring/"
          imgsrc += period;
          imgsrc += "/";
          imgsrc += ver;
          imgsrc += "/Run";
	  imgsrc += run;
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
        function changePeriod()
	{
           $("#ver").load("/data_monitoring/textdata/" + $(this.period).val() + ".txt");
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

    versions = get_versions(options)
    print "<select id=\"ver\" name=\"ver\">" 
    for version in versions:
        revision = ("ver%02d" % version[0])
        print "<option value=\"%s\" " % (revision)
        if options != None and revision == options[1]:
            print "selected"
        print "> %s</option>" % (revision)
    
    print "</select>"
    print "<input type=\"submit\" value=\"Display\" />"
    print "</form>"
    print "Link displays plots or <button type=\"button\"> ROOT </button> opens file"
    print "<br><br>"  

def print_run_selector(records, options):
    months = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"]
    print "<ul id=\"runList\">"
    dates = get_dates(options)
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
            rundate = str(row[1])[:10]
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
                rundate = str(row[1])[:10]
                if rundate == fulldate:
                    print "<li>"
                    print "<a href=\"/cgi-bin/data_monitoring/monitoring/runBrowser.py?run_number=%s&ver=%s&period=%s\"> %s (%s events) </a>  " % (row[0], options[1], options[2], row[0], row[2])
                    print ("<a href=\"/cgi-bin/data_monitoring/monitoring/browseJSRoot.py?run_number=%s&ver=%s&period=%s\" target=\"_blank\"><button type=\"button\"> ROOT </button></a>" % (row[0], options[1], options[2]))
                    print "</li>"
                    
            print "</ul>"
            print "</li>"
    
    print "</ul>"

    print "<script type=\"text/javascript\">make_tree_menu('runList');</script>"


#print row of table to display histograms
def print_row(options, charts):
    for chart in charts:
        print "<td style='text-align:center' bgcolor='#A9E2F3' onclick=\"freezeIt(); showPlot(\'%06d\', \'%s\', \'%s\', \'%s\');\" onmouseover=\"showPlot(\'%06d\', \'%s\', \'%s\', \'%s\'); this.style.backgroundColor='#F78181';\" onmouseout=\"this.style.backgroundColor='#A9E2F3';\">%s</td>" % (options[0], options[1], chart[0], options[2], options[0], options[1], chart[0], options[2], chart[1])
    print "</tr>" 


#return the option passed to the script
def get_options():
    form=cgi.FieldStorage()
    run_number_str = []
    run_number = []
    
    verName = "ver01"
    periodName = "RunPeriod-2015-12"

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

    # print version selector
    print """<div id="nav" class="link-list">"""
    print_version_selector(options)

    # print run selector form
    if options[1] != None:
        records=get_data(options)
        print_run_selector(records, options)
    print "</div class=\"link-list\">"

    # print main page with plots if run number selected
    print """<div id="main">"""

    if options[0] == None:
	print "Select run number link to show histograms or click <button type=\"button\"> ROOT </button> to open ROOT file in browser."
        print "</div>"
        print "</body>"
        print "</html>"
	sys.exit() 

    # get revision number 
    revision_str = str(options[1])
    revision_str = revision_str.replace("ver","")
    revision = int(float(revision_str))

    # Fall 2014 run
    if options[2] == 'RunPeriod-2014-10':
        if revision < 3: 
            cdc_charts = [["__CDC_cdc_e","Charge"],["__CDC_cdc_t","Time"],["CDC_occupancy","Occupancy"]]
        elif revision == 3:
            cdc_charts = [["__CDC_expert_cdc_e","Charge"],["__CDC_expert_cdc_t","Time"],["CDC_occupancy","Occupancy"]]
        elif revision == 4:
            cdc_charts = [["__CDC_expert_cdc_e","Charge"],["__CDC_cdc_raw_t","Time"],["CDC_occupancy","Occupancy"],["__CDC_cdc_ped","Pedestal"],["__CDC_cdc_windata_ped_mean","WinDataPedMean"],["__CDC_cdc_windata_ped_width","WinDataPedWidth"],["__CDC_cdc_raw_intpp_vs_n","RawIntVsN"],["__CDC_cdc_raw_t_vs_n","RawTimeVsN"],["__CDC_cdc_ped_vs_n","PedVsN"]]
        else:
            cdc_charts = [["__CDC_cdc_raw_intpp","RawInt"],["__CDC_cdc_raw_t","Time"],["CDC_occupancy","Occupancy"],["__CDC_cdc_ped","Pedestal"],["__CDC_cdc_raw_intpp_vs_n","RawIntVsN"],["__CDC_cdc_raw_t_vs_n","RawTimeVsN"],["__CDC_cdc_ped_vs_n","PedVsN"],["__CDC_cdc_windata_ped_vs_n","WinDataPedVsN"]]
            
        fdc_charts = [["__FDC_fdcos","FdcStripOcc"],["__FDC_fdcow","FdcWireOcc"]]
    
        if revision < 6:
            bcal_charts = [["bcal_summary","DigiSummary"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["__bcal_bcal_shower_plane","ShowerPosition"]]
        elif revision < 12:
            bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"]]
        elif revision < 14: 
            bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"],["bcal_hist_eff","Effic"]]
        else:
            bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"],["bcal_hist_eff","Effic"],["bcal_inv_mass","BCALInvMass"],["bcal_fcal_inv_mass","B/FCALInvMass"]]

        fcal_charts = [["__fcal_digHitE","DigiPulseInt"],["__fcal_digOcc2D","DigiOccupancy"],["__fcal_digT","DigiTime"],["fcal_hit_energy","HitSummary"],["fcal_hit_timing","HitTime"],["fcal_cluster_et","ClusterEnergyTime"],["fcal_cluster_space","ClusterSpace"]]
        tof_charts = [["__tof_tofe","Energy"],["__tof_toft","Time"],["__tof_tofo1","OccupancyPlane1"],["__tof_tofo2","OccupancyPlane2"]]
        st_charts = [["__st_st_pi_dhit","DigiPulseInt"],["__st_st_pt_dhit","DigiTime"],["__st_st_sec_adc_dhit","DigiOccupancy"]]
        tagm_charts = [["__tagm_tagm_adc_pint","DigiPulseInt"],["__tagm_tagm_adc_mult","DigiMultiplicity"],["__tagm_tagm_hit_seen","HitOccupancy"],["__tagm_tagm_hit_time","HitTime"]]
        
        if revision < 5:
            tagh_charts = [["__TAGH_DigiHit_PulseIntegral","DigiPulseInt"],["__TAGH_DigiHit_tdcTime","DigiTDCTime"],["TAGH_hit","HitSummary"]]
        elif revision < 12:
            tagh_charts = [["__TAGH_DigiHit_PulseIntegral","DigiPulseInt"],["__TAGH_DigiHit_tdcTime","DigiTDCTime"],["__TAGH_DigiHit_PedestalVsSlotID","DigiPedVsSlot"],["TAGH_hit","HitSummary"]]
        elif revision < 13:
            tagh_charts = [["__TAGH_DigiHit_DigiHit_PulseIntegral","DigiPulseInt"],["__TAGH_DigiHit_DigiHit_tdcTime","DigiTDCTime"],["__TAGH_DigiHit_DigiHit_PedestalVsSlotID","DigiPedVsSlot"],["TAGH_hit","HitSummary"]]
        else:
            tagh_charts = [["__TAGH_DigiHit_DigiHit_RawIntegral","DigiRawInt"],["__TAGH_DigiHit_DigiHit_tdcTime","DigiTDCTime"],["__TAGH_DigiHit_DigiHit_PedestalVsSlotID","DigiPedVsSlot"],["TAGH_hit","HitSummary"]]

        if revision > 15:
            hldetectortiming_charts = [["HistMacro_TaggerTiming","Tagger Timing"],["HistMacro_TaggerRFAlignment","Tagger-RF"],["HistMacro_TaggerSCAlignment","Tagger-SC"],["HistMacro_CalorimeterTiming","FCAL/BCAL"],["HistMacro_PIDSystemTiming","SC/TOF"],["HistMacro_TrackMatchedTiming","Track Matched Timing"]]

        if revision < 4:
            ana_charts1 = [["HistMacro_EventInfo","EventInfo"],["HistMacro_NumLowLevelObjects_p1","LLObjects1"],["HistMacro_NumLowLevelObjects_p2","LLObjects2"],["HistMacro_NumHighLevelObjects","HLObjects"],["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","TrackMult"],["HistMacro_Tracking","TrackSummary"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
        elif revision < 5:
            ana_charts1 = [["HistMacro_EventInfo","EventInfo"],["HistMacro_NumLowLevelObjects_p1","LLObjects1"],["HistMacro_NumLowLevelObjects_p2","LLObjects2"],["HistMacro_NumHighLevelObjects","HLObjects"],["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","TrackMult"],["HistMacro_Tracking_p1","Tracking1"],["HistMacro_Tracking_p2","Tracking2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
        elif revision < 12:
            ana_charts1 = [["HistMacro_EventInfo","EventInfo"],["HistMacro_NumLowLevelObjects_p1","LLObjects1"],["HistMacro_NumLowLevelObjects_p2","LLObjects2"],["HistMacro_NumHighLevelObjects","HLObjects"],["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","TrackMult"],["HistMacro_Tracking_p1","Tracking1"],["HistMacro_Tracking_p2","Tracking2"],["HistMacro_Tracking_p3","Tracking3"],["HistMacro_Matching_p1","Matching1"],["HistMacro_Matching_p2","Matching2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
        else:
            ana_charts1 = [["HistMacro_EventInfo","EventInfo"],["HistMacro_NumLowLevelObjects_p1","LLObjects1"],["HistMacro_NumLowLevelObjects_p2","LLObjects2"],["HistMacro_NumHighLevelObjects","HLObjects"],["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","TrackMult"],["HistMacro_Tracking_p1","Tracking1"],["HistMacro_Tracking_p2","Tracking2"],["HistMacro_Tracking_p3","Tracking3"],["HistMacro_Matching_BCAL","MatchBCAL"],["HistMacro_Matching_FCAL","MatchFCAL"],["HistMacro_Matching_SC","MatchSC/ST"],["HistMacro_Matching_TOF","MatchTOF"]]
            
        if revision < 12:
            ana_charts2 = [["HistMacro_FCALReconstruction_p1","FCAL1"],["HistMacro_FCALReconstruction_p2","FCAL2"],["HistMacro_BCALReconstruction_p1","BCAL1"],["HistMacro_BCALReconstruction_p2","BCAL2"],["HistMacro_SCReconstruction_p1","SC/ST1"],["HistMacro_SCReconstruction_p2","SC/ST2"],["HistMacro_SCReconstruction_p3","SC/ST3"],["HistMacro_TOFReconstruction_p1","TOF1"],["HistMacro_TOFReconstruction_p2","TOF2"]]
        else:
            ana_charts2 = [["HistMacro_FCALReconstruction_p1","FCAL1"],["HistMacro_FCALReconstruction_p2","FCAL2"],["HistMacro_BCALReconstruction_p1","BCAL1"],["HistMacro_BCALReconstruction_p2","BCAL2"],["HistMacro_SCReconstruction_p1","SC/ST1"],["HistMacro_SCReconstruction_p2","SC/ST2"],["HistMacro_SCReconstruction_p3","SC/ST3"],["HistMacro_TOFReconstruction_p1","TOF1"],["HistMacro_TOFReconstruction_p2","TOF2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]

        ana_charts3 = [["HistMacro_p2pi_pmiss","&pi;<sup>+</sup>&pi;<sup>-</sup>"],["HistMacro_p3pi_pmiss_2FCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(2FCAL)"],["HistMacro_p3pi_pmiss_FCAL-BCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(F/BCAL)"]]    
    # Spring 2015 run
    elif options[2] == 'RunPeriod-2015-03' or options[2] == 'detcom_02' or options[2] == 'RunPeriod-2015-06':
        cdc_charts = [["__CDC_cdc_raw_intpp","RawInt"],["__CDC_cdc_raw_t","Time"],["CDC_occupancy","Occupancy"],["__CDC_cdc_ped","Pedestal"],["__CDC_cdc_raw_intpp_vs_n","RawIntVsN"],["__CDC_cdc_raw_t_vs_n","RawTimeVsN"],["__CDC_cdc_ped_vs_n","PedVsN"],["__CDC_cdc_windata_ped_vs_n","WinDataPedVsN"]]
        fdc_charts = [["__FDC_fdcos","FdcStripOcc"],["__FDC_fdcow","FdcWireOcc"]]
        if revision < 4:
                bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"],["bcal_hist_eff","Effic"]]
	elif revision < 5:
		bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"],["bcal_hist_eff","Effic"],["bcal_inv_mass","InvMass"],["trig_fcalbcal","Trigger"]]
        else:
               	bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"],["bcal_hist_eff","Effic"],["bcal_inv_mass","BCALInvMass"],["bcal_fcal_inv_mass","B/FCALInvMass"],["trig_fcalbcal","Trigger"]]
   
        fcal_charts = [["__fcal_digHitE","DigiPulseInt"],["__fcal_digOcc2D","DigiOccupancy"],["__fcal_digT","DigiTime"],["fcal_hit_energy","HitSummary"],["fcal_hit_timing","HitTime"],["fcal_cluster_et","ClusterEnergyTime"],["fcal_cluster_space","ClusterSpace"]]
        tof_charts = [["__tof_tofe","Energy"],["__tof_toft","Time"],["__tof_tofo1","OccupancyPlane1"],["__tof_tofo2","OccupancyPlane2"]]
        if revision < 9:
		st_charts = [["__st_st_pi_dhit","DigiPulseInt"],["__st_st_pt_dhit","DigiTime"],["__st_st_sec_adc_dhit","DigiOccupancy"]]
	else:
		st_charts = [["ST_Monitoring_Waveform_ch4","LowWaveform"],["ST_Monitoring_Multi","LowMulti"],["ST_Monitoring_Pid","TrackingPID"],["ST_Monitoring_Eff","TrackingEff"]]
        tagm_charts = [["__tagm_tagm_adc_pint","DigiPulseInt"],["__tagm_tagm_adc_mult","DigiMultiplicity"],["__tagm_tagm_hit_seen","HitOccupancy"],["__tagm_tagm_hit_time","HitTime"]]
        if revision < 4:
		tagh_charts = [["__TAGH_DigiHit_DigiHit_RawIntegral","DigiRawInt"],["__TAGH_DigiHit_DigiHit_tdcTime","DigiTDCTime"],["__TAGH_DigiHit_DigiHit_PedestalVsSlotID","DigiPedVsSlot"],["TAGH_hit","HitSummary"]]
	else:
		tagh_charts = [["__TAGH_DigiHit_DigiHit_RawIntegral","DigiRawInt"],["__TAGH_DigiHit_DigiHit_tdcTime","DigiTDCTime"],["__TAGH_DigiHit_DigiHit_PedestalVsSlotID","DigiPedVsSlot"],["TAGH_hit","HitSummary"],["TAGH_hit2","HitSummary2"]]
	if revision > 3:
		ps_charts = [["PSC_hit","PSC1"],["PSC_hit2","PSC2"],["PSC_hit3","PSC3"],["PS_hit","PS1"],["PS_hit2","PS2"],["__PSPair_PSC_PS_PS_E","PS_E"],["PS_PSC_coinc","PairCoinc"],["PS_eff","PairEff"],["PS_TAG_energy","PairTagEnergy"]] #,["TAG_eff","TagEff"],["TAG_2D_eff","Tag2DEff"]]
        if revision > 4:
		rf_charts = [["HistMacro_RF_p1","RF1"],["HistMacro_RF_p2","RF2"],["HistMacro_RF_p3","RF3"]]
        if revision > 5:
		hldetectortiming_charts = [["HistMacro_TaggerTiming","Tagger Timing"],["HistMacro_TaggerRFAlignment","Tagger-RF"],["HistMacro_TaggerSCAlignment","Tagger-SC"],["HistMacro_CalorimeterTiming","FCAL/BCAL"],["HistMacro_PIDSystemTiming","SC/TOF"],["HistMacro_TrackMatchedTiming","Track Matched Timing"]]
        ana_charts1 = [["HistMacro_EventInfo","EventInfo"],["HistMacro_NumLowLevelObjects_p1","LLObjects1"],["HistMacro_NumLowLevelObjects_p2","LLObjects2"],["HistMacro_NumHighLevelObjects","HLObjects"],["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","TrackMult"],["HistMacro_Tracking_p1","Tracking1"],["HistMacro_Tracking_p2","Tracking2"],["HistMacro_Tracking_p3","Tracking3"],["HistMacro_Matching_BCAL","MatchBCAL"],["HistMacro_Matching_FCAL","MatchFCAL"],["HistMacro_Matching_SC","MatchSC/ST"],["HistMacro_Matching_TOF","MatchTOF"]]
        if revision < 4: 
            ana_charts2 = [["HistMacro_FCALReconstruction_p1","FCAL1"],["HistMacro_FCALReconstruction_p2","FCAL2"],["HistMacro_BCALReconstruction_p1","BCAL1"],["HistMacro_BCALReconstruction_p2","BCAL2"],["HistMacro_SCReconstruction_p1","SC/ST1"],["HistMacro_SCReconstruction_p2","SC/ST2"],["HistMacro_SCReconstruction_p3","SC/ST3"],["HistMacro_TOFReconstruction_p1","TOF1"],["HistMacro_TOFReconstruction_p2","TOF2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
        if revision < 8:
            ana_charts2 = [["HistMacro_FCALReconstruction_p1","FCAL1"],["HistMacro_FCALReconstruction_p2","FCAL2"],["HistMacro_FCALReconstruction_p3","FCAL3"],["HistMacro_BCALReconstruction_p1","BCAL1"],["HistMacro_BCALReconstruction_p2","BCAL2"],["HistMacro_BCALReconstruction_p3","BCAL3"],["HistMacro_SCReconstruction_p1","SC/ST1"],["HistMacro_SCReconstruction_p2","SC/ST2"],["HistMacro_SCReconstruction_p3","SC/ST3"],["HistMacro_TOFReconstruction_p1","TOF1"],["HistMacro_TOFReconstruction_p2","TOF2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
	else:
            ana_charts2 = [["HistMacro_FCALReconstruction_p1","FCAL1"],["HistMacro_FCALReconstruction_p2","FCAL2"],["HistMacro_FCALReconstruction_p3","FCAL3"],["HistMacro_BCALReconstruction_p1","BCAL1"],["HistMacro_BCALReconstruction_p2","BCAL2"],["HistMacro_BCALReconstruction_p3","BCAL3"],["HistMacro_SCReconstruction_p1","SC/ST1"],["HistMacro_SCReconstruction_p2","SC/ST2"],["HistMacro_TOFReconstruction_p1","TOF1"],["HistMacro_TOFReconstruction_p2","TOF2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
        if revision < 10: 
            ana_charts3 = [["HistMacro_p2pi_pmiss","&pi;<sup>+</sup>&pi;<sup>-</sup>1"],["HistMacro_p2pi_preco1","&pi;<sup>+</sup>&pi;<sup>-</sup>2"],["HistMacro_p3pi_pmiss_2FCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(2FCAL)"],["HistMacro_p3pi_pmiss_FCAL-BCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(F/BCAL)"]]
        else:
            ana_charts3 = [["HistMacro_p2pi_pmiss","&pi;<sup>+</sup>&pi;<sup>-</sup>1"],["HistMacro_p2pi_preco1","&pi;<sup>+</sup>&pi;<sup>-</sup>2"],["HistMacro_p3pi_preco_2FCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(2FCAL)"],["HistMacro_p3pi_preco_FCAL-BCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(F/BCAL)"]]
    # Fall 2015 run
    elif options[2] == 'RunPeriod-2015-12':
        cdc_charts = [["__CDC_cdc_raw_intpp","RawInt"],["__CDC_cdc_raw_t","Time"],["CDC_occupancy","Occupancy"],["__CDC_cdc_ped","Pedestal"],["__CDC_cdc_raw_intpp_vs_n","RawIntVsN"],["__CDC_cdc_raw_t_vs_n","RawTimeVsN"],["__CDC_cdc_ped_vs_n","PedVsN"],["__CDC_cdc_windata_ped_vs_n","WinDataPedVsN"]]
        fdc_charts = [["__FDC_fdcos","FdcStripOcc"],["__FDC_fdcow","FdcWireOcc"]]
        bcal_charts = [["bcal_summary","DigiSummary"],["bcal_times","DigiTime"],["bcal_occupancy","DigiOccupancy"],["bcal_cluster","Cluster"],["bcal_shower","Shower"],["bcal_hist_eff","Effic"],["bcal_inv_mass","BCALInvMass"],["bcal_fcal_inv_mass","B/FCALInvMass"],["trig_fcalbcal","Trigger"]]
   
        fcal_charts = [["__fcal_digHitE","DigiPulseInt"],["__fcal_digOcc2D","DigiOccupancy"],["__fcal_digT","DigiTime"],["fcal_hit_energy","HitSummary"],["fcal_hit_timing","HitTime"],["fcal_cluster_et","ClusterEnergyTime"],["fcal_cluster_space","ClusterSpace"]]
        tof_charts = [["__tof_tofe","Energy"],["__tof_toft","Time"],["__tof_tofo1","OccupancyPlane1"],["__tof_tofo2","OccupancyPlane2"]]
        st_charts = [["ST_Monitoring_Waveform_ch4","LowWaveform"],["ST_Monitoring_Multi","LowMulti"],["ST_Monitoring_Pid","TrackingPID"],["ST_Monitoring_Eff","TrackingEff"]]
        tagm_charts = [["__tagm_tagm_adc_pint","DigiPulseInt"],["__tagm_tagm_adc_mult","DigiMultiplicity"],["__tagm_tagm_hit_seen","HitOccupancy"],["__tagm_tagm_hit_time","HitTime"]]
        tagh_charts = [["__TAGH_DigiHit_DigiHit_RawIntegral","DigiRawInt"],["__TAGH_DigiHit_DigiHit_tdcTime","DigiTDCTime"],["__TAGH_DigiHit_DigiHit_PedestalVsSlotID","DigiPedVsSlot"],["TAGH_hit","HitSummary"],["TAGH_hit2","HitSummary2"]]
        ps_charts = [["PSC_hit","PSC1"],["PSC_hit2","PSC2"],["PSC_hit3","PSC3"],["PS_hit","PS1"],["PS_hit2","PS2"],["__PSPair_PSC_PS_PS_E","PS_E"],["PS_PSC_coinc","PairCoinc"],["PS_eff","PairEff"],["PS_TAG_energy","PairTagEnergy"]] #,["TAG_eff","TagEff"],["TAG_2D_eff","Tag2DEff"]]
        rf_charts = [["HistMacro_RF_p1","RF1"],["HistMacro_RF_p2","RF2"],["HistMacro_RF_p3","RF3"]]
        hldetectortiming_charts = [["HistMacro_TaggerTiming","Tagger Timing"],["HistMacro_TaggerRFAlignment","Tagger-RF"],["HistMacro_TaggerSCAlignment","Tagger-SC"],["HistMacro_CalorimeterTiming","FCAL/BCAL"],["HistMacro_PIDSystemTiming","SC/TOF"],["HistMacro_TrackMatchedTiming","Track Matched Timing"]]
        ana_charts1 = [["HistMacro_EventInfo","EventInfo"],["HistMacro_NumLowLevelObjects_p1","LLObjects1"],["HistMacro_NumLowLevelObjects_p2","LLObjects2"],["HistMacro_NumHighLevelObjects","HLObjects"],["__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles","TrackMult"],["HistMacro_Tracking_p1","Tracking1"],["HistMacro_Tracking_p2","Tracking2"],["HistMacro_Tracking_p3","Tracking3"],["HistMacro_Matching_BCAL","MatchBCAL"],["HistMacro_Matching_FCAL","MatchFCAL"],["HistMacro_Matching_SC","MatchSC/ST"],["HistMacro_Matching_TOF","MatchTOF"]]
        ana_charts2 = [["HistMacro_FCALReconstruction_p1","FCAL1"],["HistMacro_FCALReconstruction_p2","FCAL2"],["HistMacro_FCALReconstruction_p3","FCAL3"],["HistMacro_BCALReconstruction_p1","BCAL1"],["HistMacro_BCALReconstruction_p2","BCAL2"],["HistMacro_BCALReconstruction_p3","BCAL3"],["HistMacro_SCReconstruction_p1","SC/ST1"],["HistMacro_SCReconstruction_p2","SC/ST2"],["HistMacro_TOFReconstruction_p1","TOF1"],["HistMacro_TOFReconstruction_p2","TOF2"],["HistMacro_Kinematics_p1","Kinematics1"],["HistMacro_Kinematics_p2","Kinematics2"]]
        ana_charts3 = [["HistMacro_p2pi_pmiss","&pi;<sup>+</sup>&pi;<sup>-</sup>1"],["HistMacro_p2pi_preco1","&pi;<sup>+</sup>&pi;<sup>-</sup>2"],["HistMacro_p3pi_preco_2FCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(2FCAL)"],["HistMacro_p3pi_preco_FCAL-BCAL","&pi;<sup>+</sup>&pi;<sup>-</sup>&pi;<sup>0</sup>(F/BCAL)"]]

    # set names to "rootspy" if these are online histograms 
    if revision == 0:
        for chart in cdc_charts:
            chart[0] = chart[0].replace("__CDC_cdc","_rootspy__rootspy_CDC_cdc")
        for chart in fdc_charts:
            chart[0] = chart[0].replace("__FDC","_rootspy__rootspy_FDC")
        for chart in fcal_charts:
            chart[0] = chart[0].replace("__fcal","_rootspy__rootspy_fcal")
        for chart in tof_charts:
            chart[0] = chart[0].replace("__tof","_rootspy__rootspy_tof")
        for chart in st_charts:
            chart[0] = chart[0].replace("__st_st","_rootspy__rootspy_st_st")
        for chart in tagm_charts:
            chart[0] = chart[0].replace("__tagm_tagm","_rootspy__rootspy_tagm_tagm")
        for chart in tagh_charts:
            chart[0] = chart[0].replace("__TAGH","_rootspy__rootspy_TAGH")
        for chart in ana_charts1:
            chart[0] = chart[0].replace("__Independent","_rootspy__rootspy_Independent")

    # display all possible charts in table for selection
    print """Mouse over <font style="background-color: #A9E2F3">light blue</font> entries in table to view histograms, and <b>click</b>  on an entry to freeze/unfreeze a specific historgram. <br>"""
    print """<table style="width:200px; font-size:0.8em">
      <tr>"""

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

    if revision > 3 and options[2] == 'RunPeriod-2015-03' or options[2] == 'RunPeriod-2015-06' or options[2] == 'RunPeriod-2015-12':
	print """<table style="width:200px; font-size:0.8em">
	   <tr>"""
	print "<td>PS:</td>"
	print_row(options, ps_charts)
    	print "</table>"

    if revision > 4 and options[2] == 'RunPeriod-2015-03' or options[2] == 'RunPeriod-2015-06' or options[2] == 'RunPeriod-2015-12':
	print """<table style="width:200px; font-size:0.8em">
	   <tr>"""
	print "<td>RF:</td>"
	print_row(options, rf_charts)
    	print "</table>"

    if (revision > 5 and options[2] == 'RunPeriod-2015-03') or (revision > 15 and options[2] == 'RunPeriod-2014-10') or options[2] == 'RunPeriod-2015-06' or options[2] == 'RunPeriod-2015-12':
	print """<table style="font-size:0.8em">
	   <tr>"""
	print "<td>HLDetectorTiming:</td>"
	print_row(options, hldetectortiming_charts)
    	print "</table>"

    print """<table style="width:200px; font-size:0.8em">
      <tr>"""
    print "<td>RECO:  </td>" 
    print_row(options, ana_charts1)
    print "<td></td>"
    print_row(options, ana_charts2)
    print "</table>"
    
    print """<table style="width=200px; font-size:0.8em">
      <tr>"""
    print "<td>ANA:</td>" 
    print_row(options, ana_charts3)
    print "</table>" 

    record_singlerun=get_data_singlerun(options)
    #print "<br> Run %s" % (options[0]) # record_singlerun[3])
    for row in record_singlerun:
       print "<br> <b> Run %s: </b> Beam current = %s nA, Radiator = %s, Solenoid current = %s A, Trigger = %s" % (options[0], row[3], row[4], row[5], row[6]) 

    # display histogram
    print """<img width=700px src="" id="imageshow" style="display:none">"""
    print "</div>"

    print "</body>"
    print "</html>"

    conn.close()

if __name__=="__main__":
    main()
