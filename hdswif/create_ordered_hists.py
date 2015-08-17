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

from ROOT import gROOT, gStyle, TCanvas, TH1F, TGraph, TH2F, TF1, TLine, TLatex
TLine.DrawClone._creates = False # This will allow use of DrawClone
import ROOT
gStyle.SetOptStat(False)
ROOT.gROOT.SetBatch(True)

def main(argv):
    
    # color scheme
    colors = (ROOT.kBlack, ROOT.kRed, ROOT.kYellow+2, ROOT.kGreen+2, ROOT.kBlue, ROOT.kMagenta, ROOT.kGray+1)

    # XML file to parse
    parser = OptionParser(usage = "\n create_ordered_hists.py [XML file]")
    (options, args) = parser.parse_args(argv)

    filename = args[0]

    # Read in xml file and create tree, root
    tree = ET.parse(filename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    summary = workflow_status.find('summary')
    workflow_name_text  = summary.find('workflow_name').text

    # Create array of unique dispatch times
    # This is kind of hacky, but we extract all 'dispatch_ts' elements
    # in the xml output file, then use sort -u to extract the unique ones.
    # The gawk command will then extract the date part.
    # The separator "X" is set so that the space between the
    # date and time is not used as a separator.
    command = "grep 'dispatch_ts' " + filename + " | sort -u | gawk 'BEGIN { FS =" + ' "X" }; { $1 = substr($1, 22,21)} 1' + "' > ___tmp.txt"
    # print 'command = ', command

    os.system(command)
    # Read in output file
    infile = open ('___tmp.txt','r')
    # Create array of unique submit times
    submit_times = []
    for line in infile:
        line = line.rstrip() # remove newline
        submit_times.append(line)
    os.system("rm -f ___tmp.txt")

    # Print out unique times
    # for i in range(len(submit_times)):
    #     print 'submit_times[', i, '] = ', submit_times[i]

    NSUBMITTIMES = len(submit_times)
    print 'Number of submit times: ', NSUBMITTIMES

    # Create a dictionary between the dispatch times and
    # array of dependency times
    dict_submit_time_dependency_times = {}
    dict_submit_time_pending_times = {}

    # Create an array that contains how many elements
    # each submit time contains
    nelements = []

    # Loop over all elements and grab the
    # - dispatcth_ts    --- dispatch time
    # - prep_wait_sec   --- time that the job spent in dependency
    # and put the prep_wait_sec value into an array
    # for the corresponding dispatch_ts

    nTotal = 0
    for attempt in workflow_status.iter('attempt'):

        dispatch_ts_text   = '-999'
        prep_wait_sec_text = '-999'
        idle_wait_sec_text = '-999'

        for dispatch_ts in attempt.iter('dispatch_ts'):
            dispatch_ts_text = str(dispatch_ts.text)
        for prep_wait_sec in attempt.iter('prep_wait_sec'):
            prep_wait_sec_text = str(prep_wait_sec.text)
        for idle_wait_sec in attempt.iter('idle_wait_sec'):
            idle_wait_sec_text = str(idle_wait_sec.text)
        # print 'dispatch_ts_text = ', dispatch_ts_text, ' prep_wait_sec_text = ', prep_wait_sec_text, ' idle_wait_sec_text = ', idle_wait_sec_text

        # Add the dependency time to the correct dispatch batch
        # Loop over unique submit times
        for time in submit_times:
            # print 'time = ', time

            # If the dispatch time matches the time,
            # add the dependency time into the value array
            # of the dictionary between
            # submit times and array of dependency times
            if dispatch_ts_text == time:
                array_for_dependencyTimes =  dict_submit_time_dependency_times.get(time, [])
                # print 'current array_for_dependencyTimes = ', array_for_dependencyTimes
                array_for_dependencyTimes.append(float(prep_wait_sec_text))
                new_pair = {time : array_for_dependencyTimes}
                dict_submit_time_dependency_times.update(new_pair)
                del new_pair
                # print 'new array_for_dependencyTimes = ', array_for_dependencyTimes

                array_for_pendingTimes =  dict_submit_time_pending_times.get(time, [])
                # print 'current array_for_pendingTimes = ', array_for_pendingTimes
                array_for_pendingTimes.append(float(idle_wait_sec_text))
                new_pair = {time : array_for_pendingTimes}
                dict_submit_time_pending_times.update(new_pair)
                del new_pair
                # print 'new array_for_pendingTimes = ', array_for_pendingTimes

                nTotal += 1

    # print 'dictionary for dependencyTimes = ', dict_submit_time_dependency_times
    print 'nTotal = ', nTotal

    # Create list of histograms
    hDependencyList = []
    hPendingList = []

    # Dictionaries are not sorted, use list submit_times
    # to order the key submit times.
    # The list of dependency times can be sorted.
    nJobs = 0
    timeIndex = 0
    for time in submit_times:
        # --- Create and fill list of dependency hists ---
        # Append new histogram to list of histograms
        h = TH1F(str('hDependency' + time),';job # ;dependency time (hrs)',nTotal,1,nTotal + 1)
        hDependencyList.append(h)
        hDependencyList[timeIndex].SetLineColor(colors[timeIndex % 7])
        array_for_dependencyTime = dict_submit_time_dependency_times.get(time,'NotFound')
        if array_for_dependencyTime == 'NotFound':
            print 'This should not happen for dependencyTime!'
            exit()
        
        array_for_dependencyTime.sort()

        # print 'time = ', time, ' nJobs = ', nJobs

        # Loop within sorted dependency times and fill hists
        for i in range(len(array_for_dependencyTime)):
            hDependencyList[timeIndex].SetBinContent(1 + nJobs + i,float(array_for_dependencyTime[i]) / 3600.)
        # ---------------------------------------------------------

        # --- Create and fill list of pending hists ---
        # Append new histogram to list of histograms
        h = TH1F(str('hPending' + time),';job # ;pending time (hrs)',nTotal,1,nTotal + 1)
        hPendingList.append(h)
        hPendingList[timeIndex].SetLineColor(colors[timeIndex % 7])
        array_for_pendingTime = dict_submit_time_pending_times.get(time,'NotFound')
        if array_for_pendingTime == 'NotFound':
            print 'This should not happen for pendingTime!'
            exit()
        
        array_for_pendingTime.sort()

        # print 'time = ', time, ' nJobs = ', nJobs

        # Loop within sorted pending times and fill hists
        for i in range(len(array_for_pendingTime)):
            hPendingList[timeIndex].SetBinContent(1 + nJobs + i,float(array_for_pendingTime[i]) / 3600.)
        # ---------------------------------------------------------

        nJobs += len(array_for_dependencyTime)
        timeIndex += 1

    # Get maximum height of hists
    MAX_DEPENDENCY = 0
    for i in range(len(hDependencyList)):
        if MAX_DEPENDENCY < hDependencyList[i].GetMaximum():
            MAX_DEPENDENCY = hDependencyList[i].GetMaximum() * 1.05

    MAX_PENDING = 0
    for i in range(len(hPendingList)):
        if MAX_PENDING < hPendingList[i].GetMaximum():
            MAX_PENDING = hPendingList[i].GetMaximum() * 1.05

    latex = TLatex()
    latex.SetTextColor(ROOT.kBlack)
    latex.SetTextSize(0.06)
    latex.SetTextAlign(12)
    latex.SetNDC(1)

    #--------------------------------------------------------------------
    # Draw histograms of dependency times
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1600, 800 )
    for i in range(len(hDependencyList)):
        hDependencyList[i].SetMaximum(MAX_DEPENDENCY);
        if i== 0:
            hDependencyList[i].Draw()
        else:
            hDependencyList[i].Draw("same")

    text = "total submissions: " + str(NSUBMITTIMES)
    latex.DrawLatex(0.40,0.80,text)

    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/hDependency.png')
    c1.Close()

    #--------------------------------------------------------------------
    # Draw histograms of pending times
    c1 = TCanvas( 'c1', 'Example with Formula', 0, 0, 1600, 800 )
    for i in range(len(hPendingList)):
        hPendingList[i].SetMaximum(MAX_PENDING);
        if i== 0:
            hPendingList[i].Draw()
        else:
            hPendingList[i].Draw("same")

    text = "total submissions: " + str(NSUBMITTIMES)
    latex.DrawLatex(0.40,0.80,text)

    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/hPending.png')
    c1.Close()


## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
