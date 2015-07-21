#!/usr/bin/python

import cgi
import cgitb
cgitb.enable()

import sys
import MySQLdb

dbhost = "hallddb.jlab.org"
dbuser = 'datmon'
dbpass = ''
dbname = 'data_monitoring'

conn=MySQLdb.connect(host=dbhost, user=dbuser, db=dbname)
curs=conn.cursor()

from detector import *

# variables for which time series to display by default
def display_options():

    # list of possible charts to display
    charts = []

    charts.append(["num_events", "", "Number of recorded events", "NumEvents", 1, "num_events from run_info",[]])

    # CDC
    charts.append(["cdc_o", "CDC_occupancy", "CDC Hits/Event", "Superlayer", 7, "sum(superlayer1_hits)/sum(num_det_events), sum(superlayer2_hits)/sum(num_det_events), sum(superlayer3_hits)/sum(num_det_events), sum(superlayer4_hits)/sum(num_det_events), sum(superlayer5_hits)/sum(num_det_events), sum(superlayer6_hits)/sum(num_det_events), sum(superlayer7_hits)/sum(num_det_events) from cdc_hits JOIN version_info ON cdc_hits",[],"cdc_hits"])

    # FDC
    charts.append(["fdc_ostrip", "__FDC_fdcos", "FDC Strip Hits", "Package", 6, "(sum(plane1_hits)+sum(plane3_hits)+sum(plane4_hits)+sum(plane6_hits)+sum(plane7_hits)+sum(plane9_hits)+sum(plane10_hits)+sum(plane12_hits))/sum(num_det_events), (sum(plane13_hits)+sum(plane15_hits)+sum(plane16_hits)+sum(plane18_hits)+sum(plane19_hits)+sum(plane21_hits)+sum(plane22_hits)+sum(plane24_hits))/sum(num_det_events), (sum(plane25_hits)+sum(plane27_hits)+sum(plane28_hits)+sum(plane30_hits)+sum(plane31_hits)+sum(plane33_hits)+sum(plane34_hits)+sum(plane36_hits))/sum(num_det_events), (sum(plane37_hits)+sum(plane39_hits)+sum(plane40_hits)+sum(plane42_hits)+sum(plane43_hits)+sum(plane45_hits)+sum(plane46_hits)+sum(plane48_hits))/sum(num_det_events), (sum(plane49_hits)+sum(plane51_hits)+sum(plane52_hits)+sum(plane54_hits)+sum(plane55_hits)+sum(plane57_hits)+sum(plane58_hits)+sum(plane60_hits))/sum(num_det_events), (sum(plane61_hits)+sum(plane63_hits)+sum(plane64_hits)+sum(plane66_hits)+sum(plane67_hits)+sum(plane69_hits)+sum(plane70_hits)+sum(plane72_hits))/sum(num_det_events) from fdc_hits JOIN version_info ON fdc_hits",[],"fdc_hits"])
    charts.append(["fdc_owire", "__FDC_fdcos", "FDC Wire Hits", "Package", 6, "(sum(plane2_hits)+sum(plane5_hits)+sum(plane8_hits)+sum(plane11_hits))/sum(num_det_events), (sum(plane14_hits)+sum(plane17_hits)+sum(plane20_hits)+sum(plane23_hits))/sum(num_det_events), (sum(plane26_hits)+sum(plane29_hits)+sum(plane32_hits)+sum(plane35_hits))/sum(num_det_events), (sum(plane38_hits)+sum(plane41_hits)+sum(plane44_hits)+sum(plane47_hits))/sum(num_det_events), (sum(plane50_hits)+sum(plane53_hits)+sum(plane56_hits)+sum(plane59_hits))/sum(num_det_events), (sum(plane62_hits)+sum(plane65_hits)+sum(plane68_hits)+sum(plane71_hits))/sum(num_det_events) from fdc_hits JOIN version_info ON fdc_hits",[],"fdc_hits"])

    # BCAL
    charts.append(["bcal_o_up","bcal_occupancy", "BCAL Upstream Hits/Event", "Quadrant", 4, "(sum(bcalhits_quad1_layer1_up)+sum(bcalhits_quad1_layer2_up)+sum(bcalhits_quad1_layer3_up)+sum(bcalhits_quad1_layer4_up))/sum(num_det_events), (sum(bcalhits_quad2_layer1_up)+sum(bcalhits_quad2_layer2_up)+sum(bcalhits_quad2_layer3_up)+sum(bcalhits_quad2_layer4_up))/sum(num_det_events), (sum(bcalhits_quad3_layer1_up)+sum(bcalhits_quad3_layer2_up)+sum(bcalhits_quad3_layer3_up)+sum(bcalhits_quad3_layer4_up))/sum(num_det_events), (sum(bcalhits_quad4_layer1_up)+sum(bcalhits_quad4_layer2_up)+sum(bcalhits_quad4_layer3_up)+sum(bcalhits_quad4_layer4_up))/sum(num_det_events)  from bcal_hits JOIN version_info ON bcal_hits",[],"bcal_hits"])
    charts.append(["bcal_o_down","bcal_occupancy", "BCAL Downstream Hits/Event", "Quadrant", 4, "(sum(bcalhits_quad1_layer1_down)+sum(bcalhits_quad1_layer2_down)+sum(bcalhits_quad1_layer3_down)+sum(bcalhits_quad1_layer4_down))/sum(num_det_events), (sum(bcalhits_quad2_layer1_down)+sum(bcalhits_quad2_layer2_down)+sum(bcalhits_quad2_layer3_down)+sum(bcalhits_quad2_layer4_down))/sum(num_det_events), (sum(bcalhits_quad3_layer1_down)+sum(bcalhits_quad3_layer2_down)+sum(bcalhits_quad3_layer3_down)+sum(bcalhits_quad3_layer4_down))/sum(num_det_events), (sum(bcalhits_quad4_layer1_down)+sum(bcalhits_quad4_layer2_down)+sum(bcalhits_quad4_layer3_down)+sum(bcalhits_quad4_layer4_down))/sum(num_det_events) from bcal_hits JOIN version_info ON bcal_hits",[],"bcal_hits"])
    charts.append(["bcal_tres","", "BCAL Time Resolution", "", 2, "(sum(BCAL_tresol_mean))/num_files, (sum(BCAL_tresol_sigma))/num_files from bcal_calib JOIN version_info ON bcal_calib",["Mean","Sigma"],"bcal_calib"])
    charts.append(["bcal_effic","bcal_hist_eff", "BCAL Layer Efficiency", "", 8, "(sum(layer1_eff))/num_files, (sum(layer2_eff))/num_files, (sum(layer3_eff))/num_files, (sum(layer4_eff))/num_files, (sum(layer1_enhanced_eff))/num_files, (sum(layer2_enhanced_eff))/num_files, (sum(layer3_enhanced_eff))/num_files, (sum(layer4_enhanced_eff))/num_files from bcal_calib JOIN version_info ON bcal_calib",["Layer1","Layer2","Layer3","Layer4","Layer1Enhanced","Layer2Enhanced","Layer3Enhanced","Layer4Enhanced"],"bcal_calib"])

    # FCAL
    charts.append(["fcal_o_inner","__fcal_digOcc2D", "FCAL Inner Ring Hits/Event", "Inner", 4, "sum(channel1_hits)/sum(num_det_events), sum(channel2_hits)/sum(num_det_events), sum(channel3_hits)/sum(num_det_events), sum(channel4_hits)/sum(num_det_events) from fcal_hits JOIN version_info ON fcal_hits",[],"fcal_hits"])
    charts.append(["fcal_o_mid_outer", "__fcal_digOcc2D", "FCAL Mid and Outer Ring Hits/Event", "", 8, "sum(channel5_hits)/sum(num_det_events), sum(channel6_hits)/sum(num_det_events), sum(channel7_hits)/sum(num_det_events), sum(channel8_hits)/sum(num_det_events), sum(channel9_hits)/sum(num_det_events), sum(channel10_hits)/sum(num_det_events), sum(channel11_hits)/sum(num_det_events), sum(channel12_hits)/sum(num_det_events) from fcal_hits JOIN version_info ON fcal_hits",["Mid1","Mid2","Mid3","Mid4","Outer1","Outer2","Outer3","Outer4"],"fcal_hits"])
    charts.append(["fcal_tres","", "FCAL Time Resolution", "", 2, "(sum(FCAL_tresol_mean))/num_files, (sum(FCAL_tresol_sigma))/num_files from fcal_calib JOIN version_info ON fcal_calib",["Mean","Sigma"],"fcal_calib"])

    # SC
    charts.append(["sc_o", "", "SC/ST Paddle Hits/Event", "Paddle", 10, "sum(pad3_hits)/sum(num_det_events), sum(pad6_hits)/sum(num_det_events), sum(pad9_hits)/sum(num_det_events), sum(pad12_hits)/sum(num_det_events), sum(pad15_hits)/sum(num_det_events), sum(pad18_hits)/sum(num_det_events), sum(pad21_hits)/sum(num_det_events), sum(pad24_hits)/sum(num_det_events), sum(pad27_hits)/sum(num_det_events), sum(pad30_hits)/sum(num_det_events) from sc_hits JOIN version_info ON sc_hits",["Paddle3", "Paddle6", "Paddle9", "Paddle12", "Paddle15", "Paddle18", "Paddle21", "Paddle24", "Paddle27", "Paddle30"],"sc_hits"])

    # TOF
    charts.append(["tof_o", "__tof_tofo1", "TOF Group Hits/Event", "Group", 16, "sum(group1_hits)/sum(num_det_events), sum(group2_hits)/sum(num_det_events), sum(group3_hits)/sum(num_det_events), sum(group4_hits)/sum(num_det_events), sum(group5_hits)/sum(num_det_events), sum(group6_hits)/sum(num_det_events), sum(group7_hits)/sum(num_det_events), sum(group8_hits)/sum(num_det_events), sum(group9_hits)/sum(num_det_events), sum(group10_hits)/sum(num_det_events), sum(group11_hits)/sum(num_det_events), sum(group12_hits)/sum(num_det_events), sum(group13_hits)/sum(num_det_events), sum(group14_hits)/sum(num_det_events), sum(group15_hits)/sum(num_det_events), sum(group16_hits)/sum(num_det_events) from tof_hits JOIN version_info ON tof_hits",[],"tof_hits"])
    charts.append(["tof_tres","", "TOF Time Resolution", "", 2, "(sum(TOF_tresol_mean))/num_files, (sum(TOF_tresol_sigma))/num_files from tof_calib JOIN version_info ON tof_calib",["Mean","Sigma"],"tof_calib"])

    # TAGM
    charts.append(["tagm_o", "", "TAGM Hits/Event", "Column", 8, "sum(column1_hits)/sum(num_det_events), sum(column2_hits)/sum(num_det_events), sum(column3_hits)/sum(num_det_events), sum(column4_hits)/sum(num_det_events), sum(column5_hits)/sum(num_det_events), sum(column6_hits)/sum(num_det_events), sum(column7_hits)/sum(num_det_events), sum(column8_hits)/sum(num_det_events) from tagm_hits JOIN version_info ON tagm_hits",[],"tagm_hits"])
    charts.append(["tagm_tres","", "TAGM Time Resolution", "", 2, "(sum(TAGM_tresol_mean))/num_files, (sum(TAGM_tresol_sigma))/num_files from tagm_calib JOIN version_info ON tagm_calib",["Mean","Sigma"],"tagm_calib"])

    # TAGH
    charts.append(["tagh_o", "", "TAGH Hits/Event", "Sector", 5, "sum(sector1_hits)/sum(num_det_events), sum(sector2_hits)/sum(num_det_events), sum(sector3_hits)/sum(num_det_events), sum(sector4_hits)/sum(num_det_events), sum(sector5_hits)/sum(num_det_events) from tagh_hits JOIN version_info ON tagh_hits",[],"tagh_hits"])
    charts.append(["tagh_tres","", "TAGH Time Resolution", "", 2, "(sum(TAGH_tresol_mean))/num_files, (sum(TAGH_tresol_sigma))/num_files from tagh_calib JOIN version_info ON tagh_calib",["Mean","Sigma"],"tagh_calib"])
    charts.append(["tagh_ADC_TDC","", "TAGH ADC-TDC fraction", "", 2, "(sum(TAGH_frac_ADC_has_TDC_hit))/num_files, (sum(TAGH_frac_TDC_has_ADC_hit))/num_files from tagh_calib JOIN version_info ON tagh_calib",["Fraction_ADC_has_TDC_hit","Fraction_TDC_has_ADC_hit"],"tagh_calib"])

    #RECO (need better event counter...)
    charts.append(["track_mult", "__Independent_Hist_TrackMultiplicity_NumGoodReconstructedParticles", "Track Multiplicity", "", 4, "sum(num_pos_tracks)/sum(num_events), sum(num_neg_tracks)/sum(num_events), sum(num_good_pos_tracks)/sum(num_events), sum(num_good_neg_tracks)/sum(num_events) from analysis_data JOIN version_info ON analysis_data",["Positive Tracks","Negative Tracks","Good Positive Tracks","Good Negative Tracks"],"analysis_data"])
    charts.append(["track_match", "HistMacro_Matching_p1", "Track/Detector Matches", "", 4, "sum(num_bcal_track_matches)/sum(num_events), sum(num_fcal_track_matches)/sum(num_events), sum(num_tof_track_matches)/sum(num_events), sum(num_sc_track_matches)/sum(num_events) from analysis_data JOIN version_info ON analysis_data",["BCAL Match","FCAL Match","TOF Match","SC/ST Match"],"analysis_data"])
    charts.append(["shower_mult", "HistMacro_BCALReconstruction_p1", "Shower Multiplicity", "", 4, "sum(num_neutraL_showers)/sum(num_events), sum(num_good_neutral_showers)/sum(num_events), sum(num_bcal_showers)/sum(num_events), sum(num_fcal_showers)/sum(num_events) from analysis_data JOIN version_info ON analysis_data",["Neutral Showers","Good Neutral Showers","BCAL Showers","FCAL Showers"],"analysis_data"])

    return charts

# print the HTTP header
def printHTTPheader():
    print "Content-type: text/html\n\n"

# get data from the database
# if an interval is passed,
# return a list of records from the database
def get_data(options, view, name):

    revision_str = str(options[2])
    revision_str = revision_str.replace("ver","")
    revision = int(float(revision_str))
    if options == None or options[0] == None:
        #curs.execute("select runid, version_id, %s WHERE num_det_events>0 and runid>0 GROUP BY runid" % (view))
        curs.execute("SELECT run_num, revision, %s.version_id=version_info.version_id JOIN run_info on run_info.run_num=%s.runid WHERE (num_det_events>0 or num_det_events=-1) and num_events>0 and run_num>0 and revision=%d %s GROUP BY run_num, revision" % (view, name, revision, options[3]))
    else:
        curs.execute("SELECT run_num, revision, %s.version_id=version_info.version_id JOIN run_info on run_info.run_num=%s.runid WHERE (num_det_events>0 or num_det_events=-1) and num_events>0 and run_num>=%d and run_num<=%d and revision=%d %s GROUP BY run_num, revision" % (view, name, options[0], options[1], revision, options[3]))

    rows=curs.fetchall()

    return rows


# get data from the database
# if an interval is passed,
# return a list of records from the database
def get_data_calib(options, view, name):

    revision_str = str(options[2])
    revision_str = revision_str.replace("ver","")
    revision = int(float(revision_str))
    if options == None or options[0] == None:
        curs.execute("SELECT run_num, revision, %s.version_id=version_info.version_id JOIN run_info on run_info.run_num=%s.runid WHERE num_files>0 and run_num>0 and revision=%d %s GROUP BY run_num, revision" % (view, name, revision, options[3]))
    else:
        curs.execute("SELECT run_num, revision, %s.version_id=version_info.version_id JOIN run_info on run_info.run_num=%s.runid WHERE num_files>0 and run_num>=%d and run_num<=%d and revision=%d %s GROUP BY run_num, revision" % (view, name, options[0], options[1], revision, options[3]))

    rows=curs.fetchall()

    return rows


# get list of versions from the DB
def get_versions(options):

    curs.execute("SELECT revision, dataVersionString from version_info where revision>0 ORDER BY revision DESC")
    rows=curs.fetchall()

    return rows


def get_num_events(options, view):

    if options == None or options[0] == None:
        curs.execute("select run_num, runid, %s WHERE run_num>0 and num_events>0 %s ORDER BY run_num" % (view, options[3]))
    else:
        curs.execute("select run_num, runid, %s WHERE run_num>=%d and run_num<=%d and num_events>0 %s ORDER BY run_num" % (view, options[0], options[1], options[3]))

    rows=curs.fetchall()

    return rows


# convert rows from database into a javascript table
def create_table(rows, pngFilename, options):
    chart_table=""
    run_number=""

    # set proper format for arbitrary row
    for row in rows: 
        chart_table+="["

        i=0
        for column in row:
            if i == 0:
                run_number=("%d" % (column))
                chart_table+=("'%s'" % (column))
            if i == 1:
                i=i+1
                continue
            if i > 1 : 
                chart_table+=(", ") # separate with commas
                chart_table+=("%s" % (column))
                chart_table+=(", '<img width=300px src=\"https://halldweb.jlab.org/work/halld/data_monitoring/RunPeriod-2014-10/%s/Run%06d/%s.png\">'" % (options[2],int(float(run_number)),pngFilename))
	    i=i+1
        chart_table+=("],\n")

    return chart_table


# print the HTML head section
# arguments are the page title and the table for the chart
def printHTMLHead(title):
    print "<head>"
    print "    <title>"
    print title
    print "    </title>"
    print """
     <style>
          div.link-list {
          width: 15.0em;
          height: 87%;
          position:absolute;
          padding-left:1%;
          padding-right:1%;
          margin-left:0;
          margin-right:0;
        }
        #main {
          height: 87%;
          margin-left:15.0em;
          padding-left:2em;
          padding-right:2em;
          overflow-y: scroll;
        }
        #nav {
          left:0;
          overflow-y: scroll;
        }
      </style>"""

    print "<script type=\"text/javascript\" src=\"https://www.google.com/jsapi\"></script>"


# print the javascript to generate the chart
# pass the table generated from the database info
def print_graph_script(table, chart, form_num):

    # generic chart preamble before data table
    pre_dataTable="""
<script type="text/javascript">
   google.load("visualization", "1", {packages:["corechart"]});
   google.setOnLoadCallback(init);""" 
    print pre_dataTable

    # unique plot descripter to give proper labels and format columns/rows
    detector_o(table, chart[2], chart[3], chart[0], chart[4], chart[6], form_num)

    # generic chart postamble after data table
    post_dataTable="""
      var selectHandler = function(e) {
         var sel = chart.getSelection();
         sel = sel[0];
	 if(sel != null){
	    var url = dataTable.getValue(sel['row'], sel['column'] +1);
            url = url.replace(/<img width=300px src=\"/,"");
            url = url.replace(/\">/, "");
            window.open(url);
         }
      }

      google.visualization.events.addListener(chart, 'select', selectHandler);

      chart.draw(dataTable, options);
   }

</script>

    """
    print post_dataTable


def print_chart_selector_script():
    print """
<script>
    onload = function () {
        chart_select();
        chart_select();
    }
</script>

<script>
    function toggle(source) {
	var checkboxes = document.getElementsByName('chart');
	for (var i=0; i<checkboxes.length; i++) {
	    checkboxes[i].checked = source.checked;
	}
    }
</script>

<script type="text/javascript">

    function chart_select() {
        
        var button = document.getElementById('chart_selector');
        button.onclick = function() {
            var charts = document.forms[1].chart;
            for (i = 0; i < charts.length; i++) {
                var chart = document.getElementById(charts[i].value);
                var form_name = charts[i].value;
                form_name += "_form";
                var form = document.getElementById(form_name);
                if (charts[i].checked) {
                   chart.style.display = 'block';
                   form.style.display = 'block';
                }
                else {
                   chart.style.display = 'none';
                   form.style.display = 'none';
                }
            }
        }
    }

</script>
"""


def print_option_selector(options, options2):
    print """<form action="/cgi-bin/data_monitoring/monitoring/timeSeries.py" method="POST">"""
    print """Select run number range to query:"""
    if options == None or options[0] == None:
        print "<input type=\"text\" name=\"run_number1\" />"
        print "<input type=\"text\" name=\"run_number2\" />"
    else:
        print "<input type=\"text\" value=\"%s\" name=\"run_number1\" />" % (options[0])
        print "<input type=\"text\" value=\"%s\" name=\"run_number2\" />" % (options[1])

    versions = []
    for version in options2:
        revision = ("ver%02d" % version[0])
        label = version[1]
        versions.append([revision,label])

    print "and Recon. Version:"
    print "<select name=\"ver\">"
    for version in versions:
        print "<option value=\"%s\" " % (version[0])
        if options != None and version[0] == options[2]:
            print "selected"
        print "> %s</option>" % (version[1])
    print "</select>"
    
    print "<br>"
    print """Add additional MYSQL query requirements as string:"""
    if options == None:
        print "<input type=\"text\" name=\"query\" size=\"60\" />"
    else:
        print "<input type=\"text\" name=\"query\" size=\"60\" value=\"%s\" />" % (options[3])
    print "eg. and beam_current>20 and solenoid_current>1190"

    print "<input type=\"submit\" value=\"Query\" />"
    print "</form>"


def print_chart_selector():

    print """<div id="nav" class="link-list">"""
    print "<form>"
    print "<b>Select plots to display:</b>"
    print "<input type=\"button\" id=\"chart_selector\" onclick=\"chart_select()\" value=\"Display\">"
    print "<br> <b>Check All</b> <input type=\"checkbox\" onclick=\"toggle(this)\" >"
    print "<br><br>" # <b>Detector Summaries:</b> <br>"
    print "<b>CDC:</b> Hit <input type=\"checkbox\" name=\"chart\" value=\"cdc_o\" checked>"
    print "<br><br>"
    print "<b>FDC:</b> Hit Strip <input type=\"checkbox\" name=\"chart\" value=\"fdc_ostrip\" checked>"
    print "Wire <input type=\"checkbox\" name=\"chart\" value=\"fdc_owire\" checked>"
    print "<br><br>"
    print "<b>BCAL:</b>"
    print "<br>"
    print "Hit Up <input type=\"checkbox\" name=\"chart\" value=\"bcal_o_up\" checked>"
    print "Down <input type=\"checkbox\" name=\"chart\" value=\"bcal_o_down\" checked>"
    print "<br>"
    print "Time Resolution <input type=\"checkbox\" name=\"chart\" value=\"bcal_tres\" checked>"
    print "<br>"
    print "Layer Efficiency <input type=\"checkbox\" name=\"chart\" value=\"bcal_effic\" checked>"
    print "<br><br>"
    print "<b>FCAL:</b>"
    print "<br>"
    print "Hit Inner <input type=\"checkbox\" name=\"chart\" value=\"fcal_o_inner\" checked>"
    print "Hit Mid-Outer <input type=\"checkbox\" name=\"chart\" value=\"fcal_o_mid_outer\" checked>"
    print "<br>"
    print "Time Resolution <input type=\"checkbox\" name=\"chart\" value=\"fcal_tres\" checked>"
    print "<br><br>"
    print "<b>SC/ST:</b> Hit <input type=\"checkbox\" name=\"chart\" value=\"sc_o\" checked>"
    print "<br><br>"
    print "<b>TOF:</b>"
    print "Hit <input type=\"checkbox\" name=\"chart\" value=\"tof_o\" checked>"
    print "<br>"
    print "Time Resolution <input type=\"checkbox\" name=\"chart\" value=\"tof_tres\" checked>"
    print "<br><br>"
    print "<b>TAGM:</b>"
    print "Hit <input type=\"checkbox\" name=\"chart\" value=\"tagm_o\" checked>"
    print "<br>"
    print "Time Resolution <input type=\"checkbox\" name=\"chart\" value=\"tagm_tres\" checked>"
    print "<br><br>"
    print "<b>TAGH:</b>"
    print "Hit <input type=\"checkbox\" name=\"chart\" value=\"tagh_o\" checked>"
    print "<br>"
    print "Time Resolution <input type=\"checkbox\" name=\"chart\" value=\"tagh_tres\" checked>"
    print "ADC-TDC Fraction <input type=\"checkbox\" name=\"chart\" value=\"tagh_ADC_TDC\" checked>"

    print "<br><br> <b>Reconstruction Summaries:</b> <br>"
    print "Track: Multiplicity <input type=\"checkbox\" name=\"chart\" value=\"track_mult\" checked>"
    print "Match <input type=\"checkbox\" name=\"chart\" value=\"track_match\" checked> "
    print "<br>"
    print "Shower: Multiplicity <input type=\"checkbox\" name=\"chart\" value=\"shower_mult\" checked>"

    print "</form>"
    
    print "</div>"

#return the option passed to the script
def get_options():
    form=cgi.FieldStorage()
    run_number_str = []
    run_number = []

    verName = "ver16"
    query = ""

    if "ver" in form:
        verName = str(form["ver"].value)
    if "query" in form:
        query = str(form["query"].value)
    if "run_number1" in form:
        run_number_str.append(form["run_number1"].value)
    if "run_number2" in form:
        run_number_str.append(form["run_number2"].value)
    if len(run_number_str) == 2 and run_number_str[0].isalnum() and run_number_str[1].isalnum():
        if int(run_number_str[1]) > int(run_number_str[0]):
            for run in run_number_str:
                run_number.append(int(run))
            run_number.append(verName)
            run_number.append(query)
            return run_number
        else:
            return None
    else:
        run_number.append(None)
        run_number.append(None)
        run_number.append(verName)
        run_number.append(query)
        return run_number


# main function
# This is where the program starts
def main():

    # get options that may have been passed to this script
    options=get_options()

    # set which charts to display
    charts = display_options()

    # print the HTTP header
    printHTTPheader()

    # start printing the page
    print "<html>"
    # print the head section including the table
    # used by the javascript for the chart
    printHTMLHead("Offline Data Monitoring: Time Series")

    form_num = 2
    
    # loop over possible charts and produce javascripts to display
    for chart in charts:

        # pull info from data_monitoring DB
        if form_num == 2:
            records=get_num_events(options, chart[5])
        elif "calib" in chart[7]:
            records=get_data_calib(options, chart[5], chart[7])
        else:
            records=get_data(options, chart[5], chart[7])

        if len(records) != 0:
            # convert the data into a table
            table=create_table(records,chart[1],options)
        else:
            continue
            #print "No data found"
            #return

        print_graph_script(table, chart, form_num)
	form_num = form_num+1

    # print chart selector script
    print_chart_selector_script()

    print "</head>"

    # print the page body
    print "<body style=\"overflow-y: hidden\" >"
    print "<h1>Offline Data Monitoring: Time Series</h1>"

    # print option selector form
    versions = get_versions(options)
    print_option_selector(options, versions)
    print "<hr>"

    print_chart_selector()
    #print "<hr>"

    print """<div id="main">"""

    # print form to select data rows and display chart
    for chart in charts:
	
        print '<div id="%s_form">' % (chart[0])
        print "<h2>%s</h2>" % chart[2]
        detector_o_form(chart[3], chart[4], chart[6], chart[0])
        print '</div>'
	print '<div id="%s" style="width: 900px; height: 500px;"></div>' % (chart[0])
        #break

    print "</div>"

    print "</body>"
    print "</html>"

    conn.close()

if __name__=="__main__":
    main()
