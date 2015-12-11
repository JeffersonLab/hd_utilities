#--------------------------------------------------------------------#
#                                                                    #
# 2015/07/23 Kei Moriya                                              #
#                                                                    #
# Parse the xml output of swif.                                      #
#                                                                    #
# The root element of swif output is an element called               #
# <workflow_status>                                                  #
#                                                                    #
# Within <workflow_status>, each job is put into an element called   #
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

import sys, os
from os import mkdir
from optparse import OptionParser
import xml.etree.ElementTree as ET
import re
import calendar
from datetime import datetime
from matplotlib import pyplot as plt

# To use PyROOT, we need environment variable PYTHONPAT,
# for that we need ROOTSYS to be defined
ROOTSYS = str(os.environ.get('ROOTSYS','NOTSET'))
if ROOTSYS == 'NOTSET':
    print 'Environment variable ROOTSYS must be set to a version of ROOT built with PyROOT support'
    exit()

PATH = str(os.environ.get('PATH','NOTSET'))
if PATH == 'NOTSET':
    PATH = ROOTSYS + '/bin'
else:
    PATH = ROOTSYS + '/bin:' + PATH

PYTHONPATH = str(os.environ.get('PYTHONPATH','NOTSET'))
if PYTHONPATH == 'NOTSET':
    PYTHONPATH = ROOTSYS + '/lib'
else:
    PYTHONPATH = ROOTSYS + '/lib:' + PYTHONPATH

os.environ["PYTHONPATH"]      = PYTHONPATH
sys.path.append(str(ROOTSYS + '/lib/'))

from ROOT import gROOT, gStyle, TCanvas, TH1F, TGraph, TH2F, TF1, TLine, TLatex, TLegend
TLine.DrawClone._creates = False # This will allow use of DrawClone
import ROOT
gStyle.SetOptStat(False)
ROOT.gROOT.SetBatch(True)

import create_ordered_hists
import results_by_resources
import create_stacked_times

def main(argv):

    # XML file to parse
    parser = OptionParser(usage = "\n parse_swif.py [XML file]")
    (options, args) = parser.parse_args(argv)

    filename = args[0]
    basename = os.path.basename(filename)
    print 'basename = ' + basename
    filename_base = basename.rsplit('.xml')[0]
    print 'filename_base = ' + filename_base

    # Read in xml file and create tree, root
    tree = ET.parse(filename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    #--------------------------------------------------------------------
    # Get summary information
    summary = workflow_status.find('summary')
    workflow_name_text  = summary.find('workflow_name').text
    suspended_text      = summary.find('suspended').text    if (summary.find('suspended')    is not None) else 'No'
    job_limit           = summary.find('job_limit').text    if (summary.find('job_limit')    is not None) else 0
    nTotal              = summary.find('jobs').text         if (summary.find('jobs')         is not None) else 0
    nUndispatched       = summary.find('undispatched').text if (summary.find('undispatched') is not None) else 0
    nDispatched         = summary.find('dispatched').text   if (summary.find('dispatched')   is not None) else 0
    nProblems           = summary.find('problems').text     if (summary.find('problems')     is not None) else 0
    nSucceeded          = summary.find('succeeded').text    if (summary.find('succeeded')    is not None) else 0
    nFailed             = summary.find('failed').text       if (summary.find('failed')       is not None) else 0
    nCanceled           = summary.find('canceled').text     if (summary.find('canceled')     is not None) else 0
    nAttempts           = summary.find('attempts').text     if (summary.find('attempts')     is not None) else 0

    nDependency = summary.find('auger_depend').text       if (summary.find('auger_depend')      is not None) else 0
    nPending    = summary.find('auger_pending').text      if (summary.find('auger_pending')     is not None) else 0
    nStagingIn  = summary.find('auger_staging_in').text   if (summary.find('auger_staging_in')  is not None) else 0
    nActive     = summary.find('auger_active').text       if (summary.find('auger_active')      is not None) else 0
    nStagingOut = summary.find('auger_staging_out').text  if (summary.find('auger_staging_out') is not None) else 0
    nFinishing  = summary.find('auger_finishing').text    if (summary.find('auger_finishing')   is not None) else 0
    
    current_time = summary.find('current_ts').text

    # output file
    if not os.path.exists('./html'):
        os.makedirs('./html')

    print 'outfile name = ' + "html/summary_" + filename_base + ".html"
    outfile = open("html/summary_" + filename_base + ".html","w+")
    outfile.write('<html>\n')
    outfile.write('  <head>\n')
    outfile.write('  <link rel="stylesheet" type="text/css" href="mystyle.css">\n')
    outfile.write('  <meta http-equiv="content-style-type" content="text/css">\n')
    outfile.write('  <title>Summary of ' + workflow_name_text + '</title>\n')
    outfile.write('  </head>\n')
    outfile.write('  <body>\n')
    outfile.write('  <h1>' + workflow_name_text + '</h1>\n')
    outfile.write('  <hr>\n')
    outfile.write('  <h3 style="color:red; text-align:right; font-size:1em;">Click on each figure to show larger version</h3>\n')
            
    #--------------------------------------------------------------------
    # Print stats to screen
    print "------------------------------------------------------------"
    print "Undispatched        : " + str(nUndispatched)
    print "Currently dispatched: " + str(nDispatched)
    print "Number of succeeded : " + str(nSucceeded)
    print "Number of failed    : " + str(nFailed)
    print "Number of problems  : " + str(nProblems)
    print "Number of canceled  : " + str(nCanceled)
    print "Total               : " + str(nTotal)
    print "Total attempts      : " + str(nAttempts)

    #--------------------------------------------------------------------
    # Print Auger stats to screen
    print "------------------------------------------------------------"
    print "Currently running jobs:"
    print "Dependency        : " + str(nDependency)
    print "Pending           : " + str(nPending)
    print "StagingIn         : " + str(nStagingIn)
    print "Active            : " + str(nActive)
    print "Staging Out       : " + str(nStagingOut)
    print "Finishing         : " + str(nFinishing)
    print "Dispatched Total  : " + str(nDispatched)

    #--------------------------------------------------------------------
    # Create table of current job status
    outfile.write('  <h2>Status</h2>\n')
    outfile.write('  <h3>Info retrieved: ' + current_time + '</h3>\n')
    if not job_limit is '':
        outfile.write('  <h3>Job Limit: ' + str(job_limit) + '  Total attempts: ' + str(nAttempts) + '</h3>\n')
        outfile.write('  <table style="border: 0px; table-layout: fixed;">\n')

        outfile.write('  <h2>Status from SWIF Summary</h2>\n')
        outfile.write('  <h3>Numbers are for individual registered jobs, final status.</h3>\n')
    
    # Header for dispatched, undispatched, total
        outfile.write('    <tr style="background: #99CCFF;">\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Undispatched</th>\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Dispatched</th>\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Succeeded</th>\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Failed</th>\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Problems</th>\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Canceled</th>\n')
        outfile.write('      <th style="border: 0px; height:10px; width:100px;">Total</th>\n')
        outfile.write('    </tr>\n')

    # Row for dispatched, undispatched, total
        outfile.write('    <tr>\n')
        output_text = '      <td>' + str(nUndispatched) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nDispatched) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nSucceeded) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nFailed) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nProblems) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nCanceled) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td><b>' + str(nTotal) + '</b></td>\n'
        outfile.write(output_text)
        outfile.write('    </tr>\n')
        outfile.write('  </table>\n')
        
        outfile.write('  <br><br>\n')

    # Table showing Auger states:
    # - depend
    # - pending
    # - staging in
    # - active
    # - staging out
    # - finishing
    outfile.write('  <h2>Auger Status</h2>\n')
    outfile.write('  <table style="border: 0px; table-layout: fixed;">\n')
    
    # Header for dependency, pending, active, complete, undispatched, total
    outfile.write('    <tr style="background: #99CCFF;">\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Dependency</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Pending</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Staging In</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Active</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Staging Out</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Finishing</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Dispatched Total</th>\n')
    outfile.write('    </tr>\n')
    
    # Row for dependency, pending, active, complete, undispatched, total
    outfile.write('    <tr>\n')
    output_text = '      <td>' + str(nDependency) + '</td>\n'
    outfile.write(output_text)
    output_text = '      <td>' + str(nPending) + '</td>\n'
    outfile.write(output_text)
    output_text = '      <td>' + str(nStagingIn) + '</td>\n'
    outfile.write(output_text)
    output_text = '      <td>' + str(nActive) + '</td>\n'
    outfile.write(output_text)
    output_text = '      <td>' + str(nStagingOut) + '</td>\n'
    outfile.write(output_text)
    output_text = '      <td>' + str(nFinishing) + '</td>\n'
    outfile.write(output_text)
    output_text = '      <td><b>' + str(nDispatched) + '</b></td>\n'
    outfile.write(output_text)
    outfile.write('    </tr>\n')
    
    outfile.write('  </table>\n')
    outfile.close()
    
    # Call results_by_resources
    results_by_resources.main([filename])

    # Reopen
    outfile = open("html/summary_" + filename_base + ".html","a+")
    outfile.write('  <hr>\n')

    #--------------------------------------------------------------------
    # Make histogram of number of attempts
    # This can be done by looking at the <num_attempts>
    # element for each job.
    hnum_attempts = TH1F("hnum_attemps", ";# attempts", 10, -0.5, 9.5)
    for num_attempts in workflow_status.iter("num_attempts"):
        hnum_attempts.Fill(int(num_attempts.text))
    
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1200, 600 )
    # c1.find_all_primitives()
    # c1.SetMinimum(0.5)
    c1.SetLogy(1)
    
    latex = TLatex()
    latex.SetTextColor(ROOT.kBlack)
    latex.SetTextSize(0.06)
    latex.SetTextAlign(12)
    latex.SetNDC(1)
    
    hnum_attempts.SetBarWidth(0.20)
    hnum_attempts.SetBarOffset(0.50 - hnum_attempts.GetBarWidth()/2.)
    hnum_attempts.Draw("bar,textsame")
    
    # Total # of jobs is given by jobs tag within summary
    njobs = 0
    for summary in workflow_status.iter('summary'):
        for jobs in summary.iter('jobs'):
            njobs = jobs.text
    
    text = "total jobs: " + str(njobs)
    latex.DrawLatex(0.50,0.80,text)
    
    text = "total attempts: " + str(nAttempts)
    latex.DrawLatex(0.50,0.70,text)
    c1.Update()
    local_figureDir = 'html/figures/' + workflow_name_text
    web_figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(local_figureDir): os.makedirs(local_figureDir)
    c1.SaveAs(local_figureDir + '/hnum_attempts.png')
    c1.SetLogy(0)
    c1.Close()
    
    outfile.write('    <h2>Number of Attempts For Each Job</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/hnum_attempts.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/hnum_attempts.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')
    
    #--------------------------------------------------------------------
    # Find max ram_bytes
    max_ram_requested = 0
    min_ram_requested = 100
    
    for job in workflow_status.iter('job'):
        for user_run in job.iter('user_run'):
            run_name = user_run.text
        for user_file in job.iter('user_file'):
            file_name = user_file.text
        attempts = job.find('attempts')
    
        for attempt in attempts.iter('attempt'):
            # Get requested RAM
            ram_bytes_name = ""
            for ram_bytes in attempt.findall('ram_bytes'):
                ram_bytes_name = int(int(ram_bytes.text) / 1000 / 1000/ 1000)
                if max_ram_requested < ram_bytes_name:
                    max_ram_requested = int(ram_bytes_name)
    
                if min_ram_requested > ram_bytes_name:
                    min_ram_requested = int(ram_bytes_name)

    # Get the first dispatch time, which sets when the job
    # submissions began
    MIN_DISPATCH_TS_POSIX = 9999999999
    LAUNCH_TIME = ''
    time_text = ''
    # Also get the max time which is the final
    # auger_ts_complete time
    MAXTIME = 0
    # Get <name>, then <attempts> and info within
    for job in workflow_status:
        for dispatch_ts in job.iter('dispatch_ts'):
            # rpartition will strip off the final '.0' if it exists, then
            # return the 3-tuple before the separator, the separator, and after the separator
            dispatch_ts_text = str(dispatch_ts.text).rpartition(".0")[0]
            time_text = str(dispatch_ts.text).rpartition(".0")[0]
            dispatch_ts_posix = calendar.timegm(datetime.strptime(dispatch_ts_text, '%Y-%m-%d %H:%M:%S').timetuple())
            if dispatch_ts_posix < MIN_DISPATCH_TS_POSIX:
                MIN_DISPATCH_TS_POSIX = dispatch_ts_posix
                LAUNCH_TIME = time_text

        for auger_ts_complete in job.iter('auger_ts_complete'):
            auger_complete_ts_text = str(auger_ts_complete.text).rpartition(".0")[0]
            auger_complete_ts_posix = calendar.timegm(datetime.strptime(auger_complete_ts_text, '%Y-%m-%d %H:%M:%S').timetuple())
            if auger_complete_ts_posix > MAXTIME:
                MAXTIME = auger_complete_ts_posix

    # Histograms for time since start of submission
    # maximum time for jobs to finish since launch
    # Range of time histogram
    TOTALTIME = int(float(MAXTIME - MIN_DISPATCH_TS_POSIX) / 3600.) + 1.0
    print 'TOTALTIME = ', TOTALTIME, ' hrs'
    htimeSinceLaunch_submitted = TH1F("htimeSinceLaunch_submitted", ";submit time since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)
    htimeSinceLaunch_dependency = TH1F("htimeSinceLaunch_dependency", ";dependency time since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)
    htimeSinceLaunch_pending = TH1F("htimeSinceLaunch_pending", ";pending time since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)
    htimeSinceLaunch_stagingIn = TH1F("htimeSinceLaunch_stagingIn", ";staging in time since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)
    htimeSinceLaunch_active = TH1F("htimeSinceLaunch_active", ";active since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)
    htimeSinceLaunch_stagingOut = TH1F("htimeSinceLaunch_stagingOut", ";staging out time since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)
    htimeSinceLaunch_complete = TH1F("htimeSinceLaunch_complete", ";complete time since launch (hrs)", int(TOTALTIME * 10), 0,TOTALTIME)

    # Histograms for duration of dependency, pending and active
    hDurationDependency = TH1F("hDurationDependency",";time spent in dependency (hrs)",240,0,24)
    hDurationPending = TH1F("hDurationPending",";time spent in pending (hrs)",240,0,8)
    hDurationActive = TH1F("hDurationActive",";time spent in active (hrs)",240,0,8)
    
    hmaxrss = TH1F("hmaxrss", ";maxrss (GB)", 200, 0,max_ram_requested + 1)
    hauger_mem = TH1F("hauger_mem", ";mem (GB)", 200, 0,max_ram_requested + 1)
    hauger_vmem = TH1F("hauger_vmem", ";vmem (GB)", 200, 0,max_ram_requested + 1)
    hwalltime = TH1F("hwalltime", ";wall time (hrs)", 200, 0,8)
    gcput_walltime = TGraph()
    gcput_walltime.SetName("gcput_walltime")
    gcput_walltime.SetTitle("");
    gcput_walltime.SetMarkerStyle(20);
    gcput_walltime.SetMarkerSize(0.3);
    gcput_walltime.SetMarkerColor(ROOT.kBlue);
    
    # Get <name>, then <attempts> and info within
    for job in workflow_status:
        # Find job
        for name in job.iter('name'):
            name_text = str(name.text)
            match = re.match(r'offmon.*_(\d\d\d\d\d\d)_(\d\d\d)',name_text)
            run_num  = match.group(1)
            file_num = match.group(2)

        for attempt in job.iter('attempt'):
    
            # Find auger_id
            for auger_id in attempt.iter('auger_id'):
                auger_id_text = str(auger_id.text)
        
            # Find rtime
            rtime_text = ""
            for rtime in attempt.iter('rtime'):
                rtime_text = str(rtime.text)
        
            # Find stime
            stime_text = ""
            for stime in attempt.iter('stime'):
                stime_text = str(stime.text)
        
            # Find utime
            utime_text = ""
            for utime in attempt.iter('utime'):
                utime_text = str(utime.text)
        
            # Find maxrss
            maxrss_text = ""
            for maxrss in attempt.iter('maxrss'):
                maxrss_text = str(maxrss.text)
                hmaxrss.Fill(float(maxrss_text) / 1000. / 1000.)
        
            # Find auger_cpu_sec
            auger_cpu_sec_text = ""
            for auger_cpu_sec in attempt.iter('auger_cpu_sec'):
                auger_cpu_sec_text = str(auger_cpu_sec.text)
        
            # Find auger_mem_kb
            auger_mem_kb_text = ""
            for auger_mem_kb in attempt.iter('auger_mem_kb'):
                auger_mem_kb_text = str(auger_mem_kb.text)
                hauger_mem.Fill(float(auger_mem_kb_text) / 1000. / 1000.)

            # Find auger_vmem_kb
            auger_vmem_kb_text = ""
            for auger_vmem_kb in attempt.iter('auger_vmem_kb'):
                auger_vmem_kb_text = str(auger_vmem_kb.text)
                hauger_vmem.Fill(float(auger_vmem_kb_text) / 1000. / 1000.)
        
            # Find auger_wall_sec
            auger_wall_sec_text = ""
            for auger_wall_sec in attempt.iter('auger_wall_sec'):
                auger_wall_sec_text = str(auger_wall_sec.text)
                hwalltime.Fill(float(auger_wall_sec_text) / 3600.)
                gcput_walltime.SetPoint(gcput_walltime.GetN(),float(auger_wall_sec_text) / 3600.,float(auger_cpu_sec_text) / 3600.)
            
            # Get ts_submitted
            auger_ts_submitted = ""
            ts_submitted_posix = 0
            for auger_ts_submitted in attempt.iter('auger_ts_submitted'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_submitted_text = str(auger_ts_submitted.text).rpartition(".0")[0]
                ts_submitted_posix = calendar.timegm(datetime.strptime(auger_ts_submitted_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of submit time since launch
                htimeSinceLaunch_submitted.Fill(float(ts_submitted_posix - MIN_DISPATCH_TS_POSIX) / 3600.)
        
            # Get ts_dependency
            auger_ts_dependency = ""
            ts_dependency_posix = 0
            for auger_ts_dependency in attempt.iter('auger_ts_dependency'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_dependency_text = str(auger_ts_dependency.text).rpartition(".0")[0]
                ts_dependency_posix = calendar.timegm(datetime.strptime(auger_ts_dependency_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of dependency time since launch
                htimeSinceLaunch_dependency.Fill(float(ts_dependency_posix - MIN_DISPATCH_TS_POSIX) / 3600.)

            # Get ts_pending
            auger_ts_pending = ""
            ts_pending_posix = 0
            for auger_ts_pending in attempt.iter('auger_ts_pending'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_pending_text = str(auger_ts_pending.text).rpartition(".0")[0]
                ts_pending_posix = calendar.timegm(datetime.strptime(auger_ts_pending_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of pending time since launch
                htimeSinceLaunch_pending.Fill(float(ts_pending_posix - MIN_DISPATCH_TS_POSIX) / 3600.)
        
                # Fill hist of duration for dependency
                hDurationDependency.Fill(float(ts_pending_posix - ts_dependency_posix) / 3600.)
        
            # Get ts_stagingIn
            auger_ts_stagingIn = ""
            ts_stagingIn_posix = 0
            for auger_ts_stagingIn in attempt.iter('auger_ts_staging_in'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_stagingIn_text = str(auger_ts_stagingIn.text).rpartition(".0")[0]
                ts_stagingIn_posix = calendar.timegm(datetime.strptime(auger_ts_stagingIn_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of staging in time since launch
                htimeSinceLaunch_stagingIn.Fill(float(ts_stagingIn_posix - MIN_DISPATCH_TS_POSIX) / 3600.)

                # Fill hist of duration for pending
                hDurationPending.Fill(float(ts_stagingIn_posix - ts_pending_posix) / 3600.)
        
            # Get ts_active
            auger_ts_active = ""
            ts_active_posix = 0
            for auger_ts_active in attempt.iter('auger_ts_active'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_active_text = str(auger_ts_active.text).rpartition(".0")[0]
                ts_active_posix = calendar.timegm(datetime.strptime(auger_ts_active_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of active time since launch
                htimeSinceLaunch_active.Fill(float(ts_active_posix - MIN_DISPATCH_TS_POSIX) / 3600.)
        
            # Get ts_stagingOut
            auger_ts_stagingOut = ""
            ts_stagingOut_posix = 0
            for auger_ts_stagingOut in attempt.iter('auger_ts_staging_out'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_stagingOut_text = str(auger_ts_stagingOut.text).rpartition(".0")[0]
                ts_stagingOut_posix = calendar.timegm(datetime.strptime(auger_ts_stagingOut_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of staging out time since launch
                htimeSinceLaunch_stagingOut.Fill(float(ts_stagingOut_posix - MIN_DISPATCH_TS_POSIX) / 3600.)
        
                # Fill hist of duration for active
                hDurationActive.Fill(float(ts_stagingOut_posix - ts_active_posix) / 3600.)

            # Get ts_complete
            auger_ts_complete = ""
            ts_complete_posix = 0
            for auger_ts_complete in attempt.iter('auger_ts_complete'):
                # rpartition will strip off the final '.0' if it exists, then
                # return the 3-tuple before the separator, the separator, and after the separator
                auger_ts_complete_text = str(auger_ts_complete.text).rpartition(".0")[0]
                ts_complete_posix = calendar.timegm(datetime.strptime(auger_ts_complete_text, '%Y-%m-%d %H:%M:%S').timetuple())
                # Fill hist of complete time since launch
                htimeSinceLaunch_complete.Fill(float(ts_complete_posix - MIN_DISPATCH_TS_POSIX) / 3600.)
        
    # Create histograms of cumulative jobs at each stage since launch
    hCumulativeTimeSinceLaunch_submitted = htimeSinceLaunch_submitted.Clone("hCumulativeTimeSinceLaunch_submitted")
    hCumulativeTimeSinceLaunch_submitted.Clear()
    hCumulativeTimeSinceLaunch_dependency = htimeSinceLaunch_dependency.Clone("hCumulativeTimeSinceLaunch_dependency")
    hCumulativeTimeSinceLaunch_dependency.Clear()
    hCumulativeTimeSinceLaunch_pending = htimeSinceLaunch_pending.Clone("hCumulativeTimeSinceLaunch_pending")
    hCumulativeTimeSinceLaunch_pending.Clear()
    hCumulativeTimeSinceLaunch_stagingIn = htimeSinceLaunch_stagingIn.Clone("hCumulativeTimeSinceLaunch_stagingIn")
    hCumulativeTimeSinceLaunch_stagingIn.Clear()
    hCumulativeTimeSinceLaunch_active = htimeSinceLaunch_active.Clone("hCumulativeTimeSinceLaunch_active")
    hCumulativeTimeSinceLaunch_active.Clear()
    hCumulativeTimeSinceLaunch_stagingOut = htimeSinceLaunch_stagingOut.Clone("hCumulativeTimeSinceLaunch_stagingOut")
    hCumulativeTimeSinceLaunch_stagingOut.Clear()
    hCumulativeTimeSinceLaunch_complete = htimeSinceLaunch_complete.Clone("hCumulativeTimeSinceLaunch_complete")
    hCumulativeTimeSinceLaunch_complete.Clear()
    
    # submitted
    for i in range(0,hCumulativeTimeSinceLaunch_submitted.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_submitted.GetBinContent(j)
        hCumulativeTimeSinceLaunch_submitted.SetBinContent(i,total)
    
    # dependency
    for i in range(0,hCumulativeTimeSinceLaunch_dependency.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_dependency.GetBinContent(j)
        hCumulativeTimeSinceLaunch_dependency.SetBinContent(i,total)
    
    # pending
    for i in range(0,hCumulativeTimeSinceLaunch_pending.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_pending.GetBinContent(j)
        hCumulativeTimeSinceLaunch_pending.SetBinContent(i,total)
    
    # stagingIn
    for i in range(0,hCumulativeTimeSinceLaunch_stagingIn.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_stagingIn.GetBinContent(j)
        hCumulativeTimeSinceLaunch_stagingIn.SetBinContent(i,total)
    
    # active
    for i in range(0,hCumulativeTimeSinceLaunch_active.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_active.GetBinContent(j)
        hCumulativeTimeSinceLaunch_active.SetBinContent(i,total)
    
    # stagingOut
    for i in range(0,hCumulativeTimeSinceLaunch_stagingOut.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_stagingOut.GetBinContent(j)
        hCumulativeTimeSinceLaunch_stagingOut.SetBinContent(i,total)
    
    # complete
    for i in range(0,hCumulativeTimeSinceLaunch_complete.GetNbinsX()+2):
        total = 0
        for j in range(0,i+1):
            total += htimeSinceLaunch_complete.GetBinContent(j)
        hCumulativeTimeSinceLaunch_complete.SetBinContent(i,total)
    
    #--------------------------------------------------------------------
    # Draw # of jobs reaching each stage since
    # beginning of launch
    
    # Colors of each stage
    colors = (ROOT.kBlack, ROOT.kRed, ROOT.kYellow+2, ROOT.kGreen+2, ROOT.kBlue, ROOT.kMagenta, ROOT.kGray+1)
    
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1200, 600 )
    hCumulativeTimeSinceLaunch_submitted.SetLineColor(colors[0])
    hCumulativeTimeSinceLaunch_submitted.SetMinimum(0)
    hCumulativeTimeSinceLaunch_submitted.Draw()
    hCumulativeTimeSinceLaunch_dependency.SetLineColor(colors[1])
    hCumulativeTimeSinceLaunch_dependency.Draw("same")
    hCumulativeTimeSinceLaunch_pending.SetLineColor(colors[2])
    hCumulativeTimeSinceLaunch_pending.Draw("same")
    hCumulativeTimeSinceLaunch_stagingIn.SetLineColor(colors[3])
    hCumulativeTimeSinceLaunch_stagingIn.Draw("same")
    hCumulativeTimeSinceLaunch_active.SetLineColor(colors[4])
    hCumulativeTimeSinceLaunch_active.Draw("same")
    hCumulativeTimeSinceLaunch_stagingOut.SetLineColor(colors[5])
    hCumulativeTimeSinceLaunch_stagingOut.Draw("same")
    hCumulativeTimeSinceLaunch_complete.SetLineColor(colors[6])
    hCumulativeTimeSinceLaunch_complete.Draw("same")

    text = "launch: " + LAUNCH_TIME
    latex.DrawLatex(0.40,0.25,text)

    text = 'time to complete: ' + str(format((MAXTIME - MIN_DISPATCH_TS_POSIX)/3600., '.2f')) + ' hrs'
    latex.DrawLatex(0.40,0.20,text)
    
    legend = TLegend(0.20,0.60,0.90,0.85)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetNColumns(3)
    legend.AddEntry(hCumulativeTimeSinceLaunch_submitted,'submitted','L')
    legend.AddEntry(hCumulativeTimeSinceLaunch_dependency,'dependency','L')
    legend.AddEntry(hCumulativeTimeSinceLaunch_pending,'pending','L')
    legend.AddEntry(hCumulativeTimeSinceLaunch_stagingIn,'staging in','L')
    legend.AddEntry(hCumulativeTimeSinceLaunch_active,'active','L')
    legend.AddEntry(hCumulativeTimeSinceLaunch_stagingOut,'staging out','L')
    legend.AddEntry(hCumulativeTimeSinceLaunch_complete,'complete','L')
    legend.Draw("same")

    c1.Update()
    c1.SaveAs(local_figureDir + '/cumulativeNumsSinceLaunch.png')
    c1.Close()
    
    outfile.write('    <h2>Number of jobs reaching each stage since launch</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/cumulativeNumsSinceLaunch.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/cumulativeNumsSinceLaunch.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')
    

    #--------------------------------------------------------------------
    # Create histograms of dependency and pending times
    # ordered by submission time.
    # This is done in the module create_ordered_hists.py
    create_ordered_hists.main([filename])

    outfile.write('    <h2>Time Spent in Dependency</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/hDependency.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/hDependency.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

    outfile.write('    <h2>Time Spent in Pending</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/hPending.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/hPending.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

###     #--------------------------------------------------------------------
###     # Draw maxrss histogram
###     c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1200, 600 )
###     hmaxrss.Draw()
###     c1.Update()
###     c1.SaveAs(local_figureDir + '/hmaxrss.png')
###     c1.Close()
###     
###     outfile.write('    <h2>MAX RSS reported by SWIF</h2>\n')
###     outfile.write('    <a href = "' + web_figureDir + '/hmaxrss.png">\n')
###     outfile.write('      <img src = "' + web_figureDir + '/hmaxrss.png" width = "70%">\n')
###     outfile.write('    </a>\n')
###     outfile.write('    <hr>\n')
    
    #--------------------------------------------------------------------
    # Draw duration of dependency, pending, active
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 2400, 600 )
    c1.Divide(3,1,.002,.002)
    c1.cd(1)
    c1.cd(1).SetBottomMargin(0.15)
    c1.cd(1).SetRightMargin(0.02)
    hDurationDependency.GetXaxis().SetTitleSize(0.08);
    hDurationDependency.GetXaxis().SetLabelSize(0.06);
    hDurationDependency.GetXaxis().SetTitleOffset(0.750);
    hDurationDependency.GetYaxis().SetLabelSize(0.06);
    hDurationDependency.Draw()
    overflow = int(hDurationDependency.GetBinContent(hDurationDependency.GetNbinsX()+1))
    text = "overflow: " + str(overflow)
    latex.SetTextColor(ROOT.kRed)
    latex.DrawLatex(0.50,0.85,text)
    latex.SetTextColor(ROOT.kBlack)

    c1.cd(2)
    c1.cd(2).SetBottomMargin(0.15)
    c1.cd(2).SetRightMargin(0.02)
    c1.cd(2).SetLogy(1)
    hDurationPending.GetXaxis().SetTitleSize(0.08);
    hDurationPending.GetXaxis().SetLabelSize(0.06);
    hDurationPending.GetXaxis().SetTitleOffset(0.750);
    hDurationPending.GetYaxis().SetLabelSize(0.06);
    hDurationPending.Draw()
    overflow = int(hDurationPending.GetBinContent(hDurationPending.GetNbinsX()+1))
    text = "overflow: " + str(overflow)
    latex.SetTextColor(ROOT.kRed)
    latex.DrawLatex(0.50,0.85,text)
    latex.SetTextColor(ROOT.kBlack)

    c1.cd(3)
    c1.cd(3).SetBottomMargin(0.15)
    c1.cd(3).SetRightMargin(0.02)
    hDurationActive.GetXaxis().SetTitleSize(0.08);
    hDurationActive.GetXaxis().SetLabelSize(0.06);
    hDurationActive.GetXaxis().SetTitleOffset(0.750);
    hDurationActive.GetYaxis().SetLabelSize(0.06);
    hDurationActive.Draw()
    overflow = int(hDurationActive.GetBinContent(hDurationActive.GetNbinsX()+1))
    text = "overflow: " + str(overflow)
    latex.SetTextColor(ROOT.kRed)
    latex.DrawLatex(0.50,0.85,text)
    latex.SetTextColor(ROOT.kBlack)
    
    c1.Update()
    c1.SaveAs(local_figureDir + '/duration.png')
    c1.Close()
    
    outfile.write('    <h2>Duration of Each Stage</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/duration.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/duration.png" width = "100%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

    #--------------------------------------------------------------------
    # Create stacked histograms of job times ordered by
    # - Auger job ID
    # - total job time
    create_stacked_times.main([filename])

    outfile.write('    <h2>Total Job Time in Order of Auger ID</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/hstack_times_auger_id.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/hstack_times_auger_id.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

    outfile.write('    <h2>Total Job Time in Order of Total Time</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/hstack_times_totaltime.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/hstack_times_totaltime.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

    #--------------------------------------------------------------------
    # Draw wall time, cpu time vs wall time
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1800, 600 )
    c1.Divide(2,1,.002,.002)
    c1.cd(1)
    c1.cd(1).SetRightMargin(0.02)
    c1.cd(1).SetTopMargin(0.03)
    hwalltime.Draw()
    c1.cd(2)
    c1.cd(2).SetRightMargin(0.02)
    c1.cd(2).SetTopMargin(0.03)
    gcput_walltime.GetXaxis().SetLimits(0,8);
    gcput_walltime.SetMinimum(0);
    gcput_walltime.SetMaximum(48);
    gcput_walltime.GetXaxis().SetTitleOffset(0.900);
    gcput_walltime.GetXaxis().SetTitle("wall time (hrs)");
    gcput_walltime.GetYaxis().SetTitleOffset(0.900);
    gcput_walltime.GetYaxis().SetTitle("cpu time (hrs)");
    gcput_walltime.Draw("AP")
    
    flinear = []
    line_colors = (ROOT.kBlack, ROOT.kRed, ROOT.kYellow+2, ROOT.kGreen+2, ROOT.kBlue, ROOT.kMagenta)
    for i in range(0,6):
        fname  = 'flinear' + '_' + str(i)
        f = TF1(fname,"[0]*x",0,8)
        flinear.append(f)
        flinear[i].SetParameter(0,i+1.)
        # print 'line_colors[', i, '] = ', line_colors[i]
        flinear[i].SetLineColor(line_colors[i])
        flinear[i].SetLineStyle(3)
        flinear[i].SetNpx(1000)
        flinear[i].DrawClone("same")

    c1.Update()
    c1.SaveAs(local_figureDir + '/walltime.png')
    c1.Close()
    
    outfile.write('    <h2>Wall Time, CPU Time vs Wall Time</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/walltime.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/walltime.png" width = "80%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

    #--------------------------------------------------------------------
    # Draw auger_mem histogram
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1800, 600 )
    c1.Divide(2,1,.002,.002)
    c1.cd(1)
    hauger_mem.Draw()

    line_auger_mem = TLine(min_ram_requested,0,min_ram_requested,hauger_mem.GetMaximum() * 1.05)
    line_auger_mem.SetLineColor(ROOT.kRed)
    line_auger_mem.SetLineStyle(2)
    line_auger_mem.SetLineWidth(1)
    line_auger_mem.DrawClone("same")

    text = "min. RAM requested: " + str(min_ram_requested) + " GB"
    latex.SetTextColor(ROOT.kRed)
    latex.DrawLatex(0.50,0.85,text)
    latex.SetTextColor(ROOT.kBlack)
    
    c1.cd(2)
    hauger_vmem.Draw()

    line_auger_mem.SetY2(hauger_vmem.GetMaximum() * 1.05)
    line_auger_mem.DrawClone("same")

    text = "min. RAM requested: " + str(min_ram_requested) + " GB"
    latex.SetTextColor(ROOT.kRed)
    latex.DrawLatex(0.50,0.85,text)
    latex.SetTextColor(ROOT.kBlack)
    
    c1.Update()
    c1.SaveAs(local_figureDir + '/hauger_mem.png')
    c1.Close()
    
    outfile.write('    <h2>MAX Memory reported by AUGER</h2>\n')
    outfile.write('    <a href = "' + web_figureDir + '/hauger_mem.png">\n')
    outfile.write('      <img src = "' + web_figureDir + '/hauger_mem.png" width = "70%">\n')
    outfile.write('    </a>\n')
    outfile.write('    <hr>\n')

    #--------------------------------------------------------------------
    # Find jobs that had problems, make table of
    # run, file, problem, resolution
    
    outfile.write('  <h2>Problem Jobs</h2>\n')
    outfile.write('  <table style="border: 0px; table-layout: fixed;">\n')
    outfile.write('    <tr style="background: #99CCFF;">\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">Run</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">File</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:50px;">RAM (GB)</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:50px;">Attempt</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:200px;">Problem</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:140px;">Resolution</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:160px;">SWIF Job ID</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:160px;">Auger Job ID</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:200px;">Submit Time</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:200px;">Complete Time</th>\n')
    outfile.write('    </tr>\n')
    
    for job in workflow_status.iter('job'):
        for user_run in job.iter('user_run'):
            run_name = user_run.text
        for user_file in job.iter('user_file'):
            file_name = user_file.text

        attempts = job.find('attempts')
        total_attempts = len(list(attempts.iter('attempt')))
        nattempts = 0

        for attempt in attempts.iter('attempt'):
            nattempts += 1
            # Iterate over all problems in a single job
            hadProblem = False
            problem_name    = ""
            resolution_name = ""

            job_id_name = ""
            for job_id in attempt.iter('job_id'):
                job_id_name = job_id.text

            auger_id_name = ""
            for auger_id in attempt.iter('auger_id'):
                auger_id_name = auger_id.text

            auger_ts_submitted_name = ''
            for auger_ts_submitted in attempt.iter('auger_ts_submitted'):
                auger_ts_submitted_name = auger_ts_submitted.text

            auger_ts_complete_name = ''
            for auger_ts_complete in attempt.iter('auger_ts_complete'):
                auger_ts_complete_name = auger_ts_complete.text
    
            # Get requested RAM
            ram_bytes_name = ""
            for ram_bytes in attempt.iter('ram_bytes'):
                ram_bytes_name = int(int(ram_bytes.text) / 1000 / 1000/ 1000)

            for problem in attempt.findall('problem'):
                problem_name = problem.text
                hadProblem = True
                if hadProblem == True:
                    for resolution in attempt.iter('resolution'):
                        resolution_name = resolution.text
                    outfile.write('    <tr>\n')
                    output_text = '      <td>' + run_name + '</td>\n'
                    outfile.write(output_text)
                    output_text = ('      <td>' + file_name + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + str(ram_bytes_name) + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + str(nattempts) + ' / ' + str(total_attempts) + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + problem_name + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + resolution_name + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + job_id_name + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + auger_id_name + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + auger_ts_submitted_name + '</td>\n')
                    outfile.write(output_text)
                    output_text = ('      <td>' + auger_ts_complete_name + '</td>\n')
                    outfile.write(output_text)
                    outfile.write('    </tr>\n')

                    # if problem_name == 'SWIF-USER-NON-ZERO':
                    #     print 'run = ' + run_name + ' file = ' + file_name + ' attempt ' + str(nattempts) + ' / ' + str(total_attempts) + ' complete time : ' + auger_ts_complete_name

    outfile.write('    </table>\n')
    outfile.write('    <hr>\n')

    #--------------------------------------------------------------------
    # Close output file
    outfile.write('  </body>\n')
    outfile.write('</html>\n')

#------------------------------         end of main function          ---------------------------------#
    
## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
    
