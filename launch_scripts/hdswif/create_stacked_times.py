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

from ROOT import gROOT, gStyle, TCanvas, TH1F, THStack, TGraph, TH2F, TF1, TLine, TLatex, TLegend
TLine.DrawClone._creates = False # This will allow use of DrawClone
import ROOT
gStyle.SetOptStat(False)
ROOT.gROOT.SetBatch(True)

def main(argv):

    VERBOSE = False
    
    # color scheme
    colors = (ROOT.kBlack, ROOT.kRed, ROOT.kYellow+2, ROOT.kGreen+2, ROOT.kBlue, ROOT.kMagenta, ROOT.kGray+1)

    # XML file to parse
    parser = OptionParser(usage = "\n create_stacked_times.py [XML file]")
    (options, args) = parser.parse_args(argv)

    filename = args[0]

    # Read in xml file and create tree, root
    tree = ET.parse(filename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    summary = workflow_status.find('summary')
    workflow_name_text  = summary.find('workflow_name').text

    # Create a dictionary between the auger_id
    # and a dictionary of  times for that job
    dict_auger_id_times = {}
    # Create a list of auger_ids
    list_auger_ids = []

    # Create a dictionary between the total time for the job
    # and a dictionary of  times for that job
    dict_totaltime_times = {}
    # Create a list of total time
    list_totaltime = []

    # Loop over all elements and grab the
    # - auger_id
    # - timestamps for each stage of job
    # and put the auger_id and times into a dictionary
    nTotal = 0
    for attempt in workflow_status.iter('attempt'):
        # defaults
        auger_id_text                  = None
        auger_ts_submitted_text        = None
        auger_ts_dependency_text       = None
        auger_ts_pending_text          = None
        auger_ts_staging_in_text       = None
        auger_ts_active_text           = None
        auger_ts_staging_out_text      = None
        auger_ts_complete_text         = None

        submitted_time   = 0
        dependency_time  = 0
        pending_time     = 0
        staging_in_time  = 0
        active_time      = 0
        staging_out_time = 0
        complete_time    = 0

        auger_id = attempt.find('auger_id')
        if auger_id != None:
            auger_id_text = str(auger_id.text)

        if VERBOSE:
            print 'auger_id_text = ' + auger_id_text

        # submitted
        auger_ts_submitted = attempt.find('auger_ts_submitted')
        if auger_ts_submitted != None:
            auger_ts_submitted_text = str(auger_ts_submitted.text.rpartition(".0")[0])
            submitted_time = calendar.timegm(datetime.strptime(auger_ts_submitted_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'submitted_time = ' + str(submitted_time)

        # dependency
        auger_ts_dependency = attempt.find('auger_ts_dependency')
        if auger_ts_dependency != None:
            auger_ts_dependency_text = str(auger_ts_dependency.text.rpartition(".0")[0])
            dependency_time = calendar.timegm(datetime.strptime(auger_ts_dependency_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'dependency_time = ' + str(dependency_time)

        # pending
        auger_ts_pending = attempt.find('auger_ts_pending')
        if auger_ts_pending != None:
            auger_ts_pending_text = str(auger_ts_pending.text.rpartition(".0")[0])
            pending_time = calendar.timegm(datetime.strptime(auger_ts_pending_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'pending_time = ' + str(pending_time)

        # staging_in
        auger_ts_staging_in = attempt.find('auger_ts_staging_in')
        if auger_ts_staging_in != None:
            auger_ts_staging_in_text = str(auger_ts_staging_in.text.rpartition(".0")[0])
            staging_in_time = calendar.timegm(datetime.strptime(auger_ts_staging_in_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'staging_in_time = ' + str(staging_in_time)

        # active
        auger_ts_active = attempt.find('auger_ts_active')
        if auger_ts_active != None:
            auger_ts_active_text = str(auger_ts_active.text.rpartition(".0")[0])
            active_time = calendar.timegm(datetime.strptime(auger_ts_active_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'active_time = ' + str(active_time)

        # staging_out
        auger_ts_staging_out = attempt.find('auger_ts_staging_out')
        if auger_ts_staging_out != None:
            auger_ts_staging_out_text = str(auger_ts_staging_out.text.rpartition(".0")[0])
            staging_out_time = calendar.timegm(datetime.strptime(auger_ts_staging_out_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'staging_out_time = ' + str(staging_out_time)

        # complete
        auger_ts_complete = attempt.find('auger_ts_complete')
        if auger_ts_complete != None:
            auger_ts_complete_text = str(auger_ts_complete.text.rpartition(".0")[0])
            complete_time = calendar.timegm(datetime.strptime(auger_ts_complete_text,'%Y-%m-%d %H:%M:%S').timetuple())

        if VERBOSE:
            print 'complete_time = ' + str(complete_time)

        # Append the auger_id to the list
        if auger_id_text != None:
            list_auger_ids.append(int(auger_id_text))

            # Update the dictionary of auger_ids and dictionary of times
            dict_times = {'submitted'   : submitted_time,
                          'dependency'  : dependency_time,
                          'pending'     : pending_time,
                          'staging_in'  : staging_in_time,
                          'active'      : active_time,
                          'staging_out' : staging_out_time,
                          'complete'    : complete_time }
            new_pair = {int(auger_id_text) : dict_times}
            dict_auger_id_times.update(new_pair)

        
        if complete_time != 0 and submitted_time != 0:
            # Append the total time to the list
            list_totaltime.append(float(complete_time - submitted_time))

            # Update the dictionary of totaltimes and dictionary of times
            new_pair = {float(complete_time - submitted_time) : dict_times}
            dict_totaltime_times.update(new_pair)

        nTotal += 1

    if VERBOSE:
        print 'nTotal = ', nTotal

    # Sort list of auger_ids, fill histograms
    list_auger_ids.sort()

    # Histograms for each stage
    hsubmitted   = TH1F('hsubmitted',';Auger ID;',nTotal,1,nTotal+1)
    hsubmitted.SetFillColor(colors[0])
    hsubmitted.SetLineColor(colors[0])
    hdependency  = TH1F('hdependency',';Auger ID;',nTotal,1,nTotal+1)
    hdependency.SetFillColor(colors[1]);
    hdependency.SetLineColor(colors[1]);
    hpending     = TH1F('hpending',';Auger ID;',nTotal,1,nTotal+1)
    hpending.SetFillColor(colors[2]);
    hpending.SetLineColor(colors[2]);
    hstaging_in  = TH1F('hstaging_in',';Auger ID;',nTotal,1,nTotal+1)
    hstaging_in.SetFillColor(colors[3]);
    hstaging_in.SetLineColor(colors[3]);
    hactive      = TH1F('hactive',';Auger ID;',nTotal,1,nTotal+1)
    hactive.SetFillColor(colors[4]);
    hactive.SetLineColor(colors[4]);
    hstaging_out = TH1F('hstaging_out',';Auger ID;',nTotal,1,nTotal+1)
    hstaging_out.SetFillColor(colors[5]);
    hstaging_out.SetLineColor(colors[5]);

    bin = 1
    for auger_id_num in list_auger_ids:
        duration_submitted   = 0
        duration_dependency  = 0
        duration_pending     = 0
        duration_staging_in  = 0
        duration_active      = 0
        duration_staging_out = 0

        if dict_auger_id_times[auger_id_num]['dependency'] != 0 and dict_auger_id_times[auger_id_num]['submitted'] != 0:
            duration_submitted   = float(dict_auger_id_times[auger_id_num]['dependency']  - dict_auger_id_times[auger_id_num]['submitted'])     / 3600.
        if dict_auger_id_times[auger_id_num]['pending'] != 0 and dict_auger_id_times[auger_id_num]['dependency'] != 0:
            duration_dependency  = float(dict_auger_id_times[auger_id_num]['pending']     - dict_auger_id_times[auger_id_num]['dependency'])    / 3600.
        if dict_auger_id_times[auger_id_num]['staging_in'] != 0 and dict_auger_id_times[auger_id_num]['pending'] != 0:
            duration_pending     = float(dict_auger_id_times[auger_id_num]['staging_in']  - dict_auger_id_times[auger_id_num]['pending'])       / 3600.
        if dict_auger_id_times[auger_id_num]['active'] != 0 and dict_auger_id_times[auger_id_num]['staging_in'] != 0:
            duration_staging_in  = float(dict_auger_id_times[auger_id_num]['active']      - dict_auger_id_times[auger_id_num]['staging_in'])    / 3600.
        if dict_auger_id_times[auger_id_num]['staging_out'] != 0 and dict_auger_id_times[auger_id_num]['active'] != 0:
            duration_active      = float(dict_auger_id_times[auger_id_num]['staging_out'] - dict_auger_id_times[auger_id_num]['active'])        / 3600.
        if dict_auger_id_times[auger_id_num]['complete'] != 0 and dict_auger_id_times[auger_id_num]['staging_out'] != 0:
            duration_staging_out = float(dict_auger_id_times[auger_id_num]['complete']    - dict_auger_id_times[auger_id_num]['staging_out'])   / 3600.

        hsubmitted.SetBinContent(bin,duration_submitted)
        hdependency.SetBinContent(bin,duration_dependency)
        hpending.SetBinContent(bin,duration_pending)
        hstaging_in.SetBinContent(bin,duration_staging_in)
        hactive.SetBinContent(bin,duration_active)
        hstaging_out.SetBinContent(bin,duration_staging_out)

        if VERBOSE:
            print 'bin    = ' + str(bin)                       + '\n' \
                + 'auger_id   = ' + str(auger_id_num)              + '\n' \
                + 'submitted      = ' + str(dict_auger_id_times[auger_id_num]['submitted'])   + '\n' \
                + 'dependency     = ' + str(dict_auger_id_times[auger_id_num]['dependency'])  + '\n' \
                + 'pending        = ' + str(dict_auger_id_times[auger_id_num]['pending'])     + '\n' \
                + 'staging_in     = ' + str(dict_auger_id_times[auger_id_num]['staging_in'])  + '\n' \
                + 'active         = ' + str(dict_auger_id_times[auger_id_num]['active'])      + '\n' \
                + 'staging_out    = ' + str(dict_auger_id_times[auger_id_num]['staging_out']) + '\n' \
                + 'complete       = ' + str(dict_auger_id_times[auger_id_num]['complete'])    + '\n' \
                + '   submitted   = ' + str(duration_submitted)    + '\n' \
                + '   dependency  = ' + str(duration_dependency)   + '\n' \
                + '   pending     = ' + str(duration_pending)      + '\n' \
                + '   staging_in  = ' + str(duration_staging_in)   + '\n' \
                + '   active      = ' + str(duration_active)       + '\n' \
                + '   staging_out = ' + str(duration_staging_out)  + '\n'
        
        bin += 1

    # Create stacked hist and add in all hists
    hstack_times_auger_id = THStack("hstack_times_auger_id",";job # ordered by Auger ID; total time (hrs)")
    hstack_times_auger_id.Add(hsubmitted)
    hstack_times_auger_id.Add(hdependency)
    hstack_times_auger_id.Add(hpending)
    hstack_times_auger_id.Add(hstaging_in)
    hstack_times_auger_id.Add(hactive)
    hstack_times_auger_id.Add(hstaging_out)

    latex = TLatex()
    latex.SetTextColor(ROOT.kBlack)
    latex.SetTextSize(0.05)
    latex.SetTextAlign(12)
    latex.SetNDC(1)

    legend = TLegend(0.20,0.70,0.80,0.90)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetNColumns(3)

    legend.AddEntry(hsubmitted,'submitted','F')
    legend.AddEntry(hdependency,'dependency','F')
    legend.AddEntry(hpending,'pending','F')
    legend.AddEntry(hstaging_in,'staging in','F')
    legend.AddEntry(hactive,'active','F')
    legend.AddEntry(hstaging_out,'staging out','F')

    #--------------------------------------------------------------------
    # Draw histograms of stacked times against Auger ID order
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.12)
    c1.SetBottomMargin(0.12)

    hstack_times_auger_id.Draw()
    hstack_times_auger_id.GetXaxis().SetTitleSize(0.06)
    hstack_times_auger_id.GetXaxis().SetTitleOffset(0.800)
    hstack_times_auger_id.GetXaxis().SetLabelSize(0.05)
    hstack_times_auger_id.GetYaxis().SetTitleSize(0.06)
    hstack_times_auger_id.GetYaxis().SetTitleOffset(0.500)
    hstack_times_auger_id.GetYaxis().CenterTitle()
    hstack_times_auger_id.GetYaxis().SetLabelSize(0.05)
    hstack_times_auger_id.Draw() # need to redraw

    # Variable nTotal is total number of 'attempt's in XML file.
    # Compare this to the number of unique auger IDs:
    if len(list_auger_ids) != nTotal:
        print '# of attempt in XML file:      ' + str(nTotal) + '\n'
        print '# of unique Auger IDs in list: ' + str(len(list_auger_ids)) + '\n'
    else:
        print 'Total number of unique Auger IDs matches number of attempts'

    text = "total submissions: " + str(nTotal)
    latex.DrawLatex(0.40,0.95,text)

    legend.Draw('same')

    figureDir = 'html/figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/hstack_times_auger_id.png')
    c1.Close()

    # if VERBOSE:
    #     print dict_totaltime_times.keys()


    # Sort list of totaltime, fill histograms
    list_totaltime.sort()

    # Histograms for each stage
    hsubmitted_totaltime   = TH1F('hsubmitted_totaltime',';Auger ID;',nTotal,1,nTotal+1)
    hsubmitted_totaltime.SetFillColor(colors[0])
    hsubmitted_totaltime.SetLineColor(colors[0])
    hdependency_totaltime  = TH1F('hdependency_totaltime',';Auger ID;',nTotal,1,nTotal+1)
    hdependency_totaltime.SetFillColor(colors[1]);
    hdependency_totaltime.SetLineColor(colors[1]);
    hpending_totaltime     = TH1F('hpending_totaltime',';Auger ID;',nTotal,1,nTotal+1)
    hpending_totaltime.SetFillColor(colors[2]);
    hpending_totaltime.SetLineColor(colors[2]);
    hstaging_in_totaltime  = TH1F('hstaging_in_totaltime',';Auger ID;',nTotal,1,nTotal+1)
    hstaging_in_totaltime.SetFillColor(colors[3]);
    hstaging_in_totaltime.SetLineColor(colors[3]);
    hactive_totaltime      = TH1F('hactive_totaltime',';Auger ID;',nTotal,1,nTotal+1)
    hactive_totaltime.SetFillColor(colors[4]);
    hactive_totaltime.SetLineColor(colors[4]);
    hstaging_out_totaltime = TH1F('hstaging_out_totaltime',';Auger ID;',nTotal,1,nTotal+1)
    hstaging_out_totaltime.SetFillColor(colors[5]);
    hstaging_out_totaltime.SetLineColor(colors[5]);

    bin = 1
    for totaltime in list_totaltime:
        duration_submitted   = 0
        duration_dependency  = 0
        duration_pending     = 0
        duration_staging_in  = 0
        duration_active      = 0
        duration_staging_out = 0

        if dict_totaltime_times[totaltime]['dependency'] != 0 and dict_totaltime_times[totaltime]['submitted'] != 0:
            duration_submitted   = float(dict_totaltime_times[totaltime]['dependency']  - dict_totaltime_times[totaltime]['submitted'])     / 3600.
        if dict_totaltime_times[totaltime]['pending'] != 0 and dict_totaltime_times[totaltime]['dependency'] != 0:
            duration_dependency  = float(dict_totaltime_times[totaltime]['pending']     - dict_totaltime_times[totaltime]['dependency'])    / 3600.
        if dict_totaltime_times[totaltime]['staging_in'] != 0 and dict_totaltime_times[totaltime]['pending'] != 0:
            duration_pending     = float(dict_totaltime_times[totaltime]['staging_in']  - dict_totaltime_times[totaltime]['pending'])       / 3600.
        if dict_totaltime_times[totaltime]['active'] != 0 and dict_totaltime_times[totaltime]['staging_in'] != 0:
            duration_staging_in  = float(dict_totaltime_times[totaltime]['active']      - dict_totaltime_times[totaltime]['staging_in'])    / 3600.
        if dict_totaltime_times[totaltime]['staging_out'] != 0 and dict_totaltime_times[totaltime]['active'] != 0:
            duration_active      = float(dict_totaltime_times[totaltime]['staging_out'] - dict_totaltime_times[totaltime]['active'])        / 3600.
        if dict_totaltime_times[totaltime]['complete'] != 0 and dict_totaltime_times[totaltime]['staging_out'] != 0:
            duration_staging_out = float(dict_totaltime_times[totaltime]['complete']    - dict_totaltime_times[totaltime]['staging_out'])   / 3600.

        hsubmitted_totaltime.SetBinContent(bin,duration_submitted)
        hdependency_totaltime.SetBinContent(bin,duration_dependency)
        hpending_totaltime.SetBinContent(bin,duration_pending)
        hstaging_in_totaltime.SetBinContent(bin,duration_staging_in)
        hactive_totaltime.SetBinContent(bin,duration_active)
        hstaging_out_totaltime.SetBinContent(bin,duration_staging_out)

        if VERBOSE:
            print 'bin       = ' + str(bin)       + '\n' \
                + 'totaltime = ' + str(totaltime) + '\n' \
                + 'submitted      = ' + str(dict_totaltime_times[totaltime]['submitted'])   + '\n' \
                + 'dependency     = ' + str(dict_totaltime_times[totaltime]['dependency'])  + '\n' \
                + 'pending        = ' + str(dict_totaltime_times[totaltime]['pending'])     + '\n' \
                + 'staging_in     = ' + str(dict_totaltime_times[totaltime]['staging_in'])  + '\n' \
                + 'active         = ' + str(dict_totaltime_times[totaltime]['active'])      + '\n' \
                + 'staging_out    = ' + str(dict_totaltime_times[totaltime]['staging_out']) + '\n' \
                + 'complete       = ' + str(dict_totaltime_times[totaltime]['complete'])    + '\n' \
                + '   submitted   = ' + str(duration_submitted)    + '\n' \
                + '   dependency  = ' + str(duration_dependency)   + '\n' \
                + '   pending     = ' + str(duration_pending)      + '\n' \
                + '   staging_in  = ' + str(duration_staging_in)   + '\n' \
                + '   active      = ' + str(duration_active)       + '\n' \
                + '   staging_out = ' + str(duration_staging_out)  + '\n'
        
        bin += 1

    # Create stacked hist and add in all hists
    hstack_times_totaltime = THStack("hstack_times_totaltime",";job # ordered by total time; total time (hrs)")
    hstack_times_totaltime.Add(hsubmitted_totaltime)
    hstack_times_totaltime.Add(hdependency_totaltime)
    hstack_times_totaltime.Add(hpending_totaltime)
    hstack_times_totaltime.Add(hstaging_in_totaltime)
    hstack_times_totaltime.Add(hactive_totaltime)
    hstack_times_totaltime.Add(hstaging_out_totaltime)

    legend.Clear()

    legend.AddEntry(hsubmitted_totaltime,'submitted','F')
    legend.AddEntry(hdependency_totaltime,'dependency','F')
    legend.AddEntry(hpending_totaltime,'pending','F')
    legend.AddEntry(hstaging_in_totaltime,'staging in','F')
    legend.AddEntry(hactive_totaltime,'active','F')
    legend.AddEntry(hstaging_out_totaltime,'staging out','F')

    #--------------------------------------------------------------------
    # Draw histograms of stacked times against total time order
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.12)
    c1.SetBottomMargin(0.12)

    hstack_times_totaltime.Draw()
    hstack_times_totaltime.GetXaxis().SetTitleSize(0.06)
    hstack_times_totaltime.GetXaxis().SetTitleOffset(0.800)
    hstack_times_totaltime.GetXaxis().SetLabelSize(0.05)
    hstack_times_totaltime.GetYaxis().SetTitleSize(0.06)
    hstack_times_totaltime.GetYaxis().SetTitleOffset(0.500)
    hstack_times_totaltime.GetYaxis().CenterTitle()
    hstack_times_totaltime.GetYaxis().SetLabelSize(0.05)
    hstack_times_totaltime.Draw() # need to redraw

    text = "total submissions: " + str(nTotal)
    latex.DrawLatex(0.40,0.95,text)

    # Variable nTotal is total number of 'attempt's in XML file.
    # Compare this to the number of unique totaltimes:
    if len(list_totaltime) != nTotal:
        print '# of attempt in XML file:      ' + str(nTotal) + '\n'
        print '# of unique total times in list: ' + str(len(list_totaltime)) + '\n'
    else:
        print 'Total number of unique total times matches number of attempts'

    legend.Draw('same')

    figureDir = 'html/figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/hstack_times_totaltime.png')
    c1.Close()
    

## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
