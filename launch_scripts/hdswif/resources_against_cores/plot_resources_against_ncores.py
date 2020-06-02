import sys, os
from os import mkdir
from optparse import OptionParser
import xml.etree.ElementTree as ET
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

    parser = OptionParser(usage = "\n plot_resources_against_ncores.py [RUNPERIOD] [VERSION]")
    (options, args) = parser.parse_args(argv)

    if len(args) != 2:
        print "Usage:"
        print "plot_resources_against_ncores.py [RUNPERIOD] [VERSION]"
        print "example: plot_resources_against_ncores.py 2015_03 15"
        exit()
    
    RUNPERIOD = ""
    VERSION = ""

    # RUNPERIOD is with underscore
    RUNPERIOD = args[0]
    RUNPERIOD   = RUNPERIOD.replace('-','_')
    RUNPERIOD_HYPHEN = RUNPERIOD.replace('_', '-')
    VERSION   = args[1]
    XMLDIR    = '/home/' + os.environ['USER'] + '/halld/hdswif/xml/'

    VERBOSE = False
    
    # color scheme
    colors = (ROOT.kRed, ROOT.kSpring-2, ROOT.kAzure+10, ROOT.kMagenta-2, ROOT.kBlack)
    sp_colors = (ROOT.kBlack, ROOT.kRed, ROOT.kRed-9, ROOT.kOrange-2, ROOT.kYellow-7,
                 ROOT.kYellow-3, ROOT.kSpring-2, ROOT.kGreen+2, ROOT.kCyan-7, ROOT.kCyan-3,
                 ROOT.kAzure+10, ROOT.kBlue-7, ROOT.kBlue-10, ROOT.kViolet, ROOT.kViolet-9,
                 ROOT.kMagenta-2, ROOT.kMagenta-7, ROOT.kPink+10, ROOT.kPink-7, ROOT.kGray,
                 ROOT.kGray+3)

    # Dictionary for #cores and color index
    # The #cores should match what the jobs ran with.
    dict_cpu_color = {1  : 0,
                      2  : 1,
                      3  : 2,
                      4  : 3,
                      5  : 4,
                      6  : 5,
                      7  : 6,
                      8  : 7,
                      9  : 8,
                      12 : 9,
                      15 : 10,
                      18 : 11,
                      20 : 12,
                      24 : 13,
                      32 : 14,
                      42 : 15}
              
    # File nevents_resources.txt contains
    # against [run] [file] [ncores] [#events]
    # #events is set to -999 if no stdout file was found.
    dict_run_file_ncores_events = {}
    dict_run_file_ncores_plugintime = {}

    nevents_resources_file = open('nevents_resources_' + RUNPERIOD + '_ver' + VERSION + '.txt', 'r')
    for line in nevents_resources_file:
        runnum  = str(line.split()[0])
        filenum = str(line.split()[1])
        ncores  = int(line.split()[2])
        nevents = int(line.split()[3])
        plugintime = int(line.split()[4])

        # Create dict of dict_run_file_ncores_events[runnum][filenum][ncores] = nevents

        if not dict_run_file_ncores_events.has_key(runnum):
            # print 'runnum = ' + runnum
            dict_run_file_ncores_events[runnum] = {}

        if not dict_run_file_ncores_events[runnum].has_key(filenum):
            # print 'filenum = ' + filenum
            dict_run_file_ncores_events[runnum][filenum] = {}

        dict_run_file_ncores_events[runnum][filenum][ncores] = nevents

        # Create dict of dict_run_file_ncores_plugintime[runnum][filenum][ncores] = plugintime

        if not dict_run_file_ncores_plugintime.has_key(runnum):
            # print 'runnum = ' + runnum
            dict_run_file_ncores_plugintime[runnum] = {}

        if not dict_run_file_ncores_plugintime[runnum].has_key(filenum):
            # print 'filenum = ' + filenum
            dict_run_file_ncores_plugintime[runnum][filenum] = {}

        dict_run_file_ncores_plugintime[runnum][filenum][ncores] = plugintime

                
    # print dict_run_file_ncores_events

    # XML file to parse
    filename = XMLDIR + 'swif_output_offline_monitoring_RunPeriod' + RUNPERIOD + '_ver' + VERSION + '_hd_rawdata.xml'

    # Read in xml file and create tree, root
    tree = ET.parse(filename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    summary = workflow_status.find('summary')
    workflow_name_text  = summary.find('workflow_name').text

    # Create a list of run/file numbers.
    # For each entry we will create a TGraph of
    # RAM usage vs ncores.
    list_runfiles = []

    # Create dictionary of run/file numbers and TGraphs
    dict_runfiles_mem   = {}
    dict_runfiles_vmem  = {}
    dict_runfiles_rtime = {}
    dict_runfiles_rtime_times_ncores = {}
    dict_runfiles_walltime = {}
    dict_runfiles_walltime_times_ncores = {}
    dict_runfiles_eventrate = {}
    
    # For TGraph of rtime vs wall time, we want to
    # separate them into [runfiles][cores]
    # so we have two dicts,
    # runfiles -> cores,
    # cores    -> grtime_walltime
    dict_runfiles_dict_ncores_cputime_walltime = {}

    # Loop over all job elements and grab
    # - user_run, user_file
    # Create a TGraph for the run/file combination if necessary
    #   Loop over all attempt elements and grab
    #   - cpu_cores
    #   - auger_mem_kb
    #   - auger_vmem_kb
    #   - walltime
    #   - rtime
    # Add a point to the corresponding TGraph

    for job in workflow_status.iter('job'):
        # Get user_run, user_file
        user_run_text                  = 'None'
        user_file_text                 = 'None'

        for tags in job.iter('tags'):
            user_run = tags.find('user_run')
            if user_run != None:
                user_run_text = str(user_run.text)
            user_file = tags.find('user_file')
            if user_file != None:
                user_file_text = str(user_file.text)

        # print 'user_run_text = ' + user_run_text + ' user_file_text = ' + user_file_text

        runfile = ''
        if (user_run_text is not 'None') and (user_file_text is not 'None'):
            runfile = user_run_text + '_' + user_file_text
        # print 'runfile = ' + runfile

        # Add this runfile to list of list_runfiles
        # if it is not already included
        if not runfile in list_runfiles:
            list_runfiles.append(runfile)
            # Creat TGraphs corresponding to this run/file number
            gmem   = TGraph()
            gmem.SetName('gmem_' + runfile)
            new_pair = {runfile : gmem}
            dict_runfiles_mem.update(new_pair)

            gvmem   = TGraph()
            gvmem.SetName('gvmem_' + runfile)
            new_pair = {runfile : gvmem}
            dict_runfiles_vmem.update(new_pair)

            grtime   = TGraph()
            grtime.SetName('grtime_' + runfile)
            new_pair = {runfile : grtime}
            dict_runfiles_rtime.update(new_pair)

            grtime_times_ncores   = TGraph()
            grtime_times_ncores.SetName('grtime_times_ncores_' + runfile)
            new_pair = {runfile : grtime_times_ncores}
            dict_runfiles_rtime_times_ncores.update(new_pair)

            gwalltime   = TGraph()
            gwalltime.SetName('gwalltime_' + runfile)
            new_pair = {runfile : gwalltime}
            dict_runfiles_walltime.update(new_pair)

            gwalltime_times_ncores   = TGraph()
            gwalltime_times_ncores.SetName('gwalltime_times_ncores_' + runfile)
            new_pair = {runfile : gwalltime_times_ncores}
            dict_runfiles_walltime_times_ncores.update(new_pair)

            geventrate   = TGraph()
            geventrate.SetName('geventrate_' + runfile)
            new_pair = {runfile : geventrate}
            dict_runfiles_eventrate.update(new_pair)

            # Create new dict for {ncores : gcputime_walltime}
            dict_ncores_cputime_walltime = {}
            new_pair = {runfile : dict_ncores_cputime_walltime}
            dict_runfiles_dict_ncores_cputime_walltime.update(new_pair)
            

        # print list_runfiles

        # sort
        list_runfiles.sort()

        # For each attempt, get cpu_cores, auger_mem_kb, auger_vmem_kb
        for attempt in job.findall('.//*attempt[last()]'):
        # for attempt in job.iter('attempt'):
            cpu_cores_int     = 0
            auger_mem_kb_int  = 0
            auger_vmem_kb_int = 0
            rtime_float       = 0
            rtime_times_ncores_float = 0
            walltime_float       = 0
            walltime_times_ncores_float = 0
            auger_result_text = ''
            problem_text = ''
            cputime_float     = 0

            REMOVETHIS = False

            cpu_cores = attempt.find('cpu_cores')
            if cpu_cores != None:
                cpu_cores_int = int(cpu_cores.text)
            
            auger_mem_kb = attempt.find('auger_mem_kb')
            if auger_mem_kb != None:
                auger_mem_kb_int = int(auger_mem_kb.text)

            auger_vmem_kb = attempt.find('auger_vmem_kb')
            if auger_vmem_kb != None:
                auger_vmem_kb_int = int(auger_vmem_kb.text)

            walltime = attempt.find('auger_wall_sec')
            if walltime != None:
                walltime_float = float(walltime.text)

            if cpu_cores_int is not 0 and walltime_float is not 0:
                walltime_times_ncores_float = walltime_float * cpu_cores_int

            rtime = attempt.find('rtime')
            if rtime != None:
                rtime_float = float(rtime.text)

                # Bad event 003079_001 with 4 cores
                # used only 0.25 s of rtime.
                # FLAG AS BAD EVENT
                if rtime_float < 1:
                    REMOVETHIS = True
                    print 'removing this event with rtime = ' + str(rtime_float)

            cputime = attempt.find('auger_cpu_sec')
            if cputime != None:
                cputime_float = float(cputime.text)

            if cpu_cores_int is not 0 and rtime_float is not 0:
                rtime_times_ncores_float = rtime_float * cpu_cores_int

            auger_result = attempt.find('auger_result')
            if auger_result != None:
                auger_result_text = auger_result.text

            problem = attempt.find('problem')
            if problem != None:
                problem_text = problem.text

            # print 'cpu_cores = ' + str(cpu_cores_int) + ' auger_mem_kb_int = ' + str(auger_mem_kb_int) + ' auger_vmem_kb_int = ' + str(auger_vmem_kb_int) + ' walltime_float = ' + str(walltime_float)

            # Add entries to graphs only if result was success

            if auger_result_text == 'SUCCESS'and problem_text == '' and REMOVETHIS == False:
                if auger_mem_kb_int is not 0:
                    dict_runfiles_mem[runfile].SetPoint(dict_runfiles_mem[runfile].GetN(),float(cpu_cores_int),auger_mem_kb_int / 1000. / 1000.)
                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' mem = ' + str(auger_mem_kb_int)

                if auger_vmem_kb_int is not 0:
                    dict_runfiles_vmem[runfile].SetPoint(dict_runfiles_vmem[runfile].GetN(),float(cpu_cores_int),auger_vmem_kb_int / 1000. / 1000.)
                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' vmem = ' + str(auger_vmem_kb_int)

                if walltime_float is not 0:
                    dict_runfiles_walltime[runfile].SetPoint(dict_runfiles_walltime[runfile].GetN(),float(cpu_cores_int),walltime_float / 3600.)
                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' walltime = ' + str(walltime_float)

                if walltime_times_ncores_float is not 0:
                    dict_runfiles_walltime_times_ncores[runfile].SetPoint(dict_runfiles_walltime_times_ncores[runfile].GetN(),float(cpu_cores_int),walltime_times_ncores_float / 3600.)
                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' walltime_times_ncores = ' + str(walltime_times_ncores_float)

                if rtime_float is not 0:
                    dict_runfiles_rtime[runfile].SetPoint(dict_runfiles_rtime[runfile].GetN(),float(cpu_cores_int),rtime_float / 3600.)
                    # print str(rtime_float) + '     ' + str(dict_run_file_ncores_plugintime[user_run_text][user_file_text][cpu_cores_int]) + '     ' + str((rtime_float - float(dict_run_file_ncores_plugintime[user_run_text][user_file_text][cpu_cores_int])) / rtime_float)
                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' rtime = ' + str(rtime_float)

                if rtime_float is not 0 and dict_run_file_ncores_events[user_run_text][user_file_text][cpu_cores_int] is not -999:
                    nevents = dict_run_file_ncores_events[user_run_text][user_file_text][cpu_cores_int]
                    time    = float(dict_run_file_ncores_plugintime[user_run_text][user_file_text][cpu_cores_int])
                    y       =  nevents / time
                    dict_runfiles_eventrate[runfile].SetPoint(dict_runfiles_eventrate[runfile].GetN(),float(cpu_cores_int),y)

                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' rtime = ' + str(rtime_float)

                if rtime_times_ncores_float is not 0:
                    dict_runfiles_rtime_times_ncores[runfile].SetPoint(dict_runfiles_rtime_times_ncores[runfile].GetN(),float(cpu_cores_int),rtime_times_ncores_float / 3600.)
                else:
                    print 'runfile = ' + runfile + ' cpu = ' + str(cpu_cores_int) + ' rtime_times_ncores = ' + str(rtime_times_ncores_float)

                # Loop over keys in dict_runfiles_dict_ncores_cputime_walltime[runfile]
                # and check if there is an entry for this # of cores.
                # If not, create a TGraph.
                if not dict_runfiles_dict_ncores_cputime_walltime[runfile].has_key(cpu_cores_int):
                    # print 'creating new dict for runfile = ' + runfile + ' cpu_cores = ' + str(cpu_cores_int)
                    gcputime_walltime = TGraph()
                    gcputime_walltime.SetName('gcputime_walltime_' + runfile + '_' + str(cpu_cores_int))
                    new_pair = {cpu_cores_int : gcputime_walltime}
                    dict_runfiles_dict_ncores_cputime_walltime[runfile].update(new_pair)
                # Update the TGraph point
                if (cputime_float is not 0) and (walltime_float is not 0):
                    dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores_int].SetPoint(dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores_int].GetN(),walltime_float / 3600., cputime_float / 3600.)

            else:
                print 'result ws not SUCCESS for ' + user_run_text + ' ' + user_file_text + ' ' + str(cpu_cores_int)

    # Draw graphs
    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend = TLegend(0.13,0.77,0.93,0.97)
    legend.SetBorderSize(0)
    legend.SetTextSize(0.045)
    legend.SetFillStyle(0)
    legend.SetNColumns(5)

    ndraw = 0
    for runfile in list_runfiles:

        dict_runfiles_mem[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_mem[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_mem[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_mem[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_mem[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_mem[runfile].GetYaxis().CenterTitle()
        dict_runfiles_mem[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_mem[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_mem[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_mem[runfile].SetMarkerSize(1.5)
        dict_runfiles_mem[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_mem[runfile].SetLineStyle(1)
        dict_runfiles_mem[runfile].SetLineWidth(1)
        dict_runfiles_mem[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_mem[runfile].GetYaxis().SetTitle('mem (GB)')

        dict_runfiles_mem[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_mem[runfile].SetMinimum(3.5) # 0
        dict_runfiles_mem[runfile].SetMaximum(14.0) # 9

        dict_runfiles_mem[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_mem[runfile].Draw('AP')
        else:
            dict_runfiles_mem[runfile].Draw('P')

        legend.AddEntry(dict_runfiles_mem[runfile],runfile,'P')
        # legend.AddEntry(dict_runfiles_mem[runfile],runfile + ' (' + str(dict_runfiles_mem[runfile].GetN()) + ')','P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/01___mem.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_mem[runfile].Draw('ALP')
        else:
            dict_runfiles_mem[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/01___mem_lines.pdf')

    c1.Close()

    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.Clear()

    ndraw = 0
    for runfile in list_runfiles:
        dict_runfiles_vmem[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_vmem[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_vmem[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_vmem[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_vmem[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_vmem[runfile].GetYaxis().CenterTitle()
        dict_runfiles_vmem[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_vmem[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_vmem[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_vmem[runfile].SetMarkerSize(1.5)
        dict_runfiles_vmem[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_vmem[runfile].SetLineStyle(1)
        dict_runfiles_vmem[runfile].SetLineWidth(1)
        dict_runfiles_vmem[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_vmem[runfile].GetYaxis().SetTitle('vmem (GB)')

        dict_runfiles_vmem[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_vmem[runfile].SetMinimum(5.0) # 0.0
        dict_runfiles_vmem[runfile].SetMaximum(18.0) # 9.0

        dict_runfiles_vmem[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_vmem[runfile].Draw('AP')
        else:
            dict_runfiles_vmem[runfile].Draw('P')

        # legend.AddEntry(dict_runfiles_vmem[runfile],runfile + ' (' + str(dict_runfiles_vmem[runfile].GetN()) + ')','P')
        legend.AddEntry(dict_runfiles_vmem[runfile],runfile,'P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/02___vmem.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_vmem[runfile].Draw('ALP')
        else:
            dict_runfiles_vmem[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/02___vmem_lines.pdf')
    c1.Close()

    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.Clear()

    ndraw = 0
    for runfile in list_runfiles:
        dict_runfiles_walltime[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_walltime[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_walltime[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_walltime[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_walltime[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_walltime[runfile].GetYaxis().CenterTitle()
        dict_runfiles_walltime[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_walltime[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_walltime[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_walltime[runfile].SetMarkerSize(1.5)
        dict_runfiles_walltime[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_walltime[runfile].SetLineStyle(1)
        dict_runfiles_walltime[runfile].SetLineWidth(1)
        dict_runfiles_walltime[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_walltime[runfile].GetYaxis().SetTitle('walltime (hrs)')

        dict_runfiles_walltime[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_walltime[runfile].SetMinimum(0)
        dict_runfiles_walltime[runfile].SetMaximum(22) # 8

        dict_runfiles_walltime[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_walltime[runfile].Draw('AP')
        else:
            dict_runfiles_walltime[runfile].Draw('P')

        # legend.AddEntry(dict_runfiles_walltime[runfile],runfile + ' (' + str(dict_runfiles_walltime[runfile].GetN()) + ')','P')
        legend.AddEntry(dict_runfiles_walltime[runfile],runfile,'P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/03___walltime.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_walltime[runfile].Draw('ALP')
        else:
            dict_runfiles_walltime[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/03___walltime_lines.pdf')

    c1.Close()

    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.Clear()

    ndraw = 0
    for runfile in list_runfiles:
        dict_runfiles_walltime_times_ncores[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_walltime_times_ncores[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_walltime_times_ncores[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_walltime_times_ncores[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_walltime_times_ncores[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_walltime_times_ncores[runfile].GetYaxis().CenterTitle()
        dict_runfiles_walltime_times_ncores[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_walltime_times_ncores[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_walltime_times_ncores[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_walltime_times_ncores[runfile].SetMarkerSize(1.5)
        dict_runfiles_walltime_times_ncores[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_walltime_times_ncores[runfile].SetLineStyle(1)
        dict_runfiles_walltime_times_ncores[runfile].SetLineWidth(1)
        dict_runfiles_walltime_times_ncores[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_walltime_times_ncores[runfile].GetYaxis().SetTitle('walltime #times ncores (hrs)')

        dict_runfiles_walltime_times_ncores[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_walltime_times_ncores[runfile].SetMinimum(0)
        dict_runfiles_walltime_times_ncores[runfile].SetMaximum(90) # 30

        dict_runfiles_walltime_times_ncores[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_walltime_times_ncores[runfile].Draw('AP')
        else:
            dict_runfiles_walltime_times_ncores[runfile].Draw('P')

        # legend.AddEntry(dict_runfiles_walltime_times_ncores[runfile],runfile + ' (' + str(dict_runfiles_walltime_times_ncores[runfile].GetN()) + ')','P')
        legend.AddEntry(dict_runfiles_walltime_times_ncores[runfile],runfile,'P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/04___walltime_times_ncores.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_walltime_times_ncores[runfile].Draw('ALP')
        else:
            dict_runfiles_walltime_times_ncores[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/04___walltime_times_ncores_lines.pdf')

    c1.Close()

    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.Clear()

    ndraw = 0
    for runfile in list_runfiles:
        dict_runfiles_rtime[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_rtime[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_rtime[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_rtime[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_rtime[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_rtime[runfile].GetYaxis().CenterTitle()
        dict_runfiles_rtime[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_rtime[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_rtime[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_rtime[runfile].SetMarkerSize(1.5)
        dict_runfiles_rtime[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_rtime[runfile].SetLineStyle(1)
        dict_runfiles_rtime[runfile].SetLineWidth(1)
        dict_runfiles_rtime[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_rtime[runfile].GetYaxis().SetTitle('rtime (hrs)')

        dict_runfiles_rtime[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_rtime[runfile].SetMinimum(0)
        dict_runfiles_rtime[runfile].SetMaximum(22) # 8

        dict_runfiles_rtime[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_rtime[runfile].Draw('AP')
        else:
            dict_runfiles_rtime[runfile].Draw('P')

        # legend.AddEntry(dict_runfiles_rtime[runfile],runfile + ' (' + str(dict_runfiles_rtime[runfile].GetN()) + ')','P')
        legend.AddEntry(dict_runfiles_rtime[runfile],runfile,'P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/05___rtime.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_rtime[runfile].Draw('ALP')
        else:
            dict_runfiles_rtime[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/05___rtime_lines.pdf')

    c1.Close()

    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.Clear()

    ndraw = 0
    for runfile in list_runfiles:
        dict_runfiles_rtime_times_ncores[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_rtime_times_ncores[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_rtime_times_ncores[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_rtime_times_ncores[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_rtime_times_ncores[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_rtime_times_ncores[runfile].GetYaxis().CenterTitle()
        dict_runfiles_rtime_times_ncores[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_rtime_times_ncores[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_rtime_times_ncores[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_rtime_times_ncores[runfile].SetMarkerSize(1.5)
        dict_runfiles_rtime_times_ncores[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_rtime_times_ncores[runfile].SetLineStyle(1)
        dict_runfiles_rtime_times_ncores[runfile].SetLineWidth(1)
        dict_runfiles_rtime_times_ncores[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_rtime_times_ncores[runfile].GetYaxis().SetTitle('rtime #times ncores (hrs)')

        dict_runfiles_rtime_times_ncores[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_rtime_times_ncores[runfile].SetMinimum(0)
        dict_runfiles_rtime_times_ncores[runfile].SetMaximum(90) # 30

        dict_runfiles_rtime_times_ncores[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_rtime_times_ncores[runfile].Draw('AP')
        else:
            dict_runfiles_rtime_times_ncores[runfile].Draw('P')

        # legend.AddEntry(dict_runfiles_rtime_times_ncores[runfile],runfile + ' (' + str(dict_runfiles_rtime_times_ncores[runfile].GetN()) + ')','P')
        legend.AddEntry(dict_runfiles_rtime_times_ncores[runfile],runfile,'P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/06___rtime_times_ncores.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_rtime_times_ncores[runfile].Draw('ALP')
        else:
            dict_runfiles_rtime_times_ncores[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/06___rtime_times_ncores_lines.pdf')

    c1.Close()

    #--------------------------------------------------------------------
    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.Clear()

    ndraw = 0
    for runfile in list_runfiles:
        dict_runfiles_eventrate[runfile].GetXaxis().SetTitleSize(0.06)
        dict_runfiles_eventrate[runfile].GetXaxis().SetTitleOffset(0.800)
        dict_runfiles_eventrate[runfile].GetXaxis().SetLabelSize(0.05)
        dict_runfiles_eventrate[runfile].GetYaxis().SetTitleSize(0.06)
        dict_runfiles_eventrate[runfile].GetYaxis().SetTitleOffset(0.700)
        dict_runfiles_eventrate[runfile].GetYaxis().CenterTitle()
        dict_runfiles_eventrate[runfile].GetYaxis().SetLabelSize(0.05)
        dict_runfiles_eventrate[runfile].SetMarkerColor(colors[ndraw//5])
        dict_runfiles_eventrate[runfile].SetMarkerStyle(20 + (ndraw % 5))
        dict_runfiles_eventrate[runfile].SetMarkerSize(1.5)
        dict_runfiles_eventrate[runfile].SetLineColor(colors[ndraw//5])
        dict_runfiles_eventrate[runfile].SetLineStyle(1)
        dict_runfiles_eventrate[runfile].SetLineWidth(1)
        dict_runfiles_eventrate[runfile].GetXaxis().SetTitle('# cores')
        dict_runfiles_eventrate[runfile].GetYaxis().SetTitle('event rate (Hz)')

        dict_runfiles_eventrate[runfile].GetXaxis().SetLimits(0,42.5)
        dict_runfiles_eventrate[runfile].SetMinimum(0)
        dict_runfiles_eventrate[runfile].SetMaximum(300.) # 800

        dict_runfiles_eventrate[runfile].Sort()

        if ndraw == 0:
            dict_runfiles_eventrate[runfile].Draw('AP')
        else:
            dict_runfiles_eventrate[runfile].Draw('P')

        # legend.AddEntry(dict_runfiles_eventrate[runfile],runfile + ' (' + str(dict_runfiles_eventrate[runfile].GetN()) + ')','P')
        legend.AddEntry(dict_runfiles_eventrate[runfile],runfile,'P')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/08___eventrate.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        if ndraw == 0:
            dict_runfiles_eventrate[runfile].Draw('ALP')
        else:
            dict_runfiles_eventrate[runfile].Draw('LP')
        ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/08___eventrate_lines.pdf')

    c1.Close()

















    #--------------------------------------------------------------------
    # For rtime vs walltime, TGraphs have two indicies,
    # runfiles and # cores
    nrunfile = 0
    ndraw = 0

    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
    c1.SetTopMargin(0.02)
    c1.SetRightMargin(0.02)
    c1.SetLeftMargin(0.10)
    c1.SetBottomMargin(0.12)

    legend.SetNColumns(4)
    legend.Clear()

    for runfile in list_runfiles:
        # print '---------------- runfile = ' + runfile + ' --------------------'

        nrunfile += 1
        for cpu_cores in dict_runfiles_dict_ncores_cputime_walltime[runfile].keys():
            # print 'cpu_cores = ' + str(cpu_cores)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetXaxis().SetTitleSize(0.06)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetXaxis().SetTitleOffset(0.800)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetXaxis().SetLabelSize(0.05)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetYaxis().SetTitleSize(0.06)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetYaxis().SetTitleOffset(0.700)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetYaxis().CenterTitle()
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetYaxis().SetLabelSize(0.05)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetMarkerColor(sp_colors[dict_cpu_color[cpu_cores]])
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetMarkerStyle(20 + (nrunfile % 5))
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetMarkerSize(1.5)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetLineColor(sp_colors[dict_cpu_color[cpu_cores]])
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetLineStyle(1)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetLineWidth(1)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetXaxis().SetTitle('walltime (hrs)')
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetYaxis().SetTitle('cpu time (hrs)')

            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].GetXaxis().SetLimits(0,8)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetMinimum(0)
            dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].SetMaximum(30) # 15

            # dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].Sort()

            if ndraw == 0:
                dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].Draw('AP')
            else:
                dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].Draw('P')

            if runfile == '003185_000':
                legend.AddEntry(dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores],runfile + ' (' + str(cpu_cores) + ')','P')

            # Draw lines (for this we need to choose a
            # runfile combination where all jobs succeeded)
            if runfile == '003185_000':
                flinear = []
                nkeys = 0
                for cpu_cores in dict_runfiles_dict_ncores_cputime_walltime[runfile].keys():
                    fname  = 'flinear' + '_' + str(cpu_cores)
                    f = TF1(fname,"[0]*x",0,8)
                    flinear.append(f)
                    flinear[nkeys].SetParameter(0,cpu_cores)
                    flinear[nkeys].SetLineColor(sp_colors[dict_cpu_color[cpu_cores]])
                    flinear[nkeys].SetLineStyle(3)
                    flinear[nkeys].SetNpx(1000)
                    flinear[nkeys].DrawClone("same")
                    nkeys += 1
            ndraw += 1
        # end of loop over cores

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/07___cputime_walltime_' + runfile + '.pdf')

    # redraw with lines
    ndraw = 0
    for runfile in list_runfiles:
        for cpu_cores in dict_runfiles_dict_ncores_cputime_walltime[runfile].keys():
            if ndraw == 0:
                dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].Draw('ALP')
            else:
                dict_runfiles_dict_ncores_cputime_walltime[runfile][cpu_cores].Draw('LP')
            ndraw += 1

    legend.Draw('same')
    figureDir = 'figures/' + workflow_name_text
    if not os.path.exists(figureDir): os.makedirs(figureDir)
    c1.SaveAs(figureDir + '/07___cputime_walltime_' + runfile + '_lines.pdf')
    
    c1.Close()
    # end of loop over runfiles


#    # if VERBOSE:
#    #     print dict_totaltime_times.keys()
#
#
#    # Sort list of totaltime, fill histograms
#    list_totaltime.sort()
#
#    # Histograms for each stage
#    hsubmitted_totaltime   = TH1F('hsubmitted_totaltime',';Auger ID;',nTotal,1,nTotal+1)
#    hsubmitted_totaltime.SetFillColor(colors[0])
#    hsubmitted_totaltime.SetLineColor(colors[0])
#    hdependency_totaltime  = TH1F('hdependency_totaltime',';Auger ID;',nTotal,1,nTotal+1)
#    hdependency_totaltime.SetFillColor(colors[1]);
#    hdependency_totaltime.SetLineColor(colors[1]);
#    hpending_totaltime     = TH1F('hpending_totaltime',';Auger ID;',nTotal,1,nTotal+1)
#    hpending_totaltime.SetFillColor(colors[2]);
#    hpending_totaltime.SetLineColor(colors[2]);
#    hstaging_in_totaltime  = TH1F('hstaging_in_totaltime',';Auger ID;',nTotal,1,nTotal+1)
#    hstaging_in_totaltime.SetFillColor(colors[3]);
#    hstaging_in_totaltime.SetLineColor(colors[3]);
#    hactive_totaltime      = TH1F('hactive_totaltime',';Auger ID;',nTotal,1,nTotal+1)
#    hactive_totaltime.SetFillColor(colors[4]);
#    hactive_totaltime.SetLineColor(colors[4]);
#    hstaging_out_totaltime = TH1F('hstaging_out_totaltime',';Auger ID;',nTotal,1,nTotal+1)
#    hstaging_out_totaltime.SetFillColor(colors[5]);
#    hstaging_out_totaltime.SetLineColor(colors[5]);
#
#    bin = 1
#    for totaltime in list_totaltime:
#        duration_submitted   = 0
#        duration_dependency  = 0
#        duration_pending     = 0
#        duration_staging_in  = 0
#        duration_active      = 0
#        duration_staging_out = 0
#
#        if dict_totaltime_times[totaltime]['dependency'] != 0 and dict_totaltime_times[totaltime]['submitted'] != 0:
#            duration_submitted   = float(dict_totaltime_times[totaltime]['dependency']  - dict_totaltime_times[totaltime]['submitted'])     / 3600.
#        if dict_totaltime_times[totaltime]['pending'] != 0 and dict_totaltime_times[totaltime]['dependency'] != 0:
#            duration_dependency  = float(dict_totaltime_times[totaltime]['pending']     - dict_totaltime_times[totaltime]['dependency'])    / 3600.
#        if dict_totaltime_times[totaltime]['staging_in'] != 0 and dict_totaltime_times[totaltime]['pending'] != 0:
#            duration_pending     = float(dict_totaltime_times[totaltime]['staging_in']  - dict_totaltime_times[totaltime]['pending'])       / 3600.
#        if dict_totaltime_times[totaltime]['active'] != 0 and dict_totaltime_times[totaltime]['staging_in'] != 0:
#            duration_staging_in  = float(dict_totaltime_times[totaltime]['active']      - dict_totaltime_times[totaltime]['staging_in'])    / 3600.
#        if dict_totaltime_times[totaltime]['staging_out'] != 0 and dict_totaltime_times[totaltime]['active'] != 0:
#            duration_active      = float(dict_totaltime_times[totaltime]['staging_out'] - dict_totaltime_times[totaltime]['active'])        / 3600.
#        if dict_totaltime_times[totaltime]['complete'] != 0 and dict_totaltime_times[totaltime]['staging_out'] != 0:
#            duration_staging_out = float(dict_totaltime_times[totaltime]['complete']    - dict_totaltime_times[totaltime]['staging_out'])   / 3600.
#
#        hsubmitted_totaltime.SetBinContent(bin,duration_submitted)
#        hdependency_totaltime.SetBinContent(bin,duration_dependency)
#        hpending_totaltime.SetBinContent(bin,duration_pending)
#        hstaging_in_totaltime.SetBinContent(bin,duration_staging_in)
#        hactive_totaltime.SetBinContent(bin,duration_active)
#        hstaging_out_totaltime.SetBinContent(bin,duration_staging_out)
#
#        if VERBOSE:
#            print 'bin       = ' + str(bin)       + '\n' \
#                + 'totaltime = ' + str(totaltime) + '\n' \
#                + 'submitted      = ' + str(dict_totaltime_times[totaltime]['submitted'])   + '\n' \
#                + 'dependency     = ' + str(dict_totaltime_times[totaltime]['dependency'])  + '\n' \
#                + 'pending        = ' + str(dict_totaltime_times[totaltime]['pending'])     + '\n' \
#                + 'staging_in     = ' + str(dict_totaltime_times[totaltime]['staging_in'])  + '\n' \
#                + 'active         = ' + str(dict_totaltime_times[totaltime]['active'])      + '\n' \
#                + 'staging_out    = ' + str(dict_totaltime_times[totaltime]['staging_out']) + '\n' \
#                + 'complete       = ' + str(dict_totaltime_times[totaltime]['complete'])    + '\n' \
#                + '   submitted   = ' + str(duration_submitted)    + '\n' \
#                + '   dependency  = ' + str(duration_dependency)   + '\n' \
#                + '   pending     = ' + str(duration_pending)      + '\n' \
#                + '   staging_in  = ' + str(duration_staging_in)   + '\n' \
#                + '   active      = ' + str(duration_active)       + '\n' \
#                + '   staging_out = ' + str(duration_staging_out)  + '\n'
#        
#        bin += 1
#
#    # Create stacked hist and add in all hists
#    hstack_times_totaltime = THStack("hstack_times_totaltime",";job # ordered by total time; total time (hrs)")
#    hstack_times_totaltime.Add(hsubmitted_totaltime)
#    hstack_times_totaltime.Add(hdependency_totaltime)
#    hstack_times_totaltime.Add(hpending_totaltime)
#    hstack_times_totaltime.Add(hstaging_in_totaltime)
#    hstack_times_totaltime.Add(hactive_totaltime)
#    hstack_times_totaltime.Add(hstaging_out_totaltime)
#
#    legend.Clear()
#
#    legend.AddEntry(hsubmitted_totaltime,'submitted','F')
#    legend.AddEntry(hdependency_totaltime,'dependency','F')
#    legend.AddEntry(hpending_totaltime,'pending','F')
#    legend.AddEntry(hstaging_in_totaltime,'staging in','F')
#    legend.AddEntry(hactive_totaltime,'active','F')
#    legend.AddEntry(hstaging_out_totaltime,'staging out','F')
#
#    #--------------------------------------------------------------------
#    # Draw histograms of stacked times against total time order
#    c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 800 )
#    c1.SetTopMargin(0.02)
#    c1.SetRightMargin(0.02)
#    c1.SetLeftMargin(0.12)
#    c1.SetBottomMargin(0.12)
#
#    hstack_times_totaltime.Draw()
#    hstack_times_totaltime.GetXaxis().SetTitleSize(0.06)
#    hstack_times_totaltime.GetXaxis().SetTitleOffset(0.800)
#    hstack_times_totaltime.GetXaxis().SetLabelSize(0.05)
#    hstack_times_totaltime.GetYaxis().SetTitleSize(0.06)
#    hstack_times_totaltime.GetYaxis().SetTitleOffset(0.800)
#    hstack_times_totaltime.GetYaxis().CenterTitle()
#    hstack_times_totaltime.GetYaxis().SetLabelSize(0.05)
#    hstack_times_totaltime.Draw() # need to redraw
#
#    text = "total submissions: " + str(nTotal)
#    latex.DrawLatex(0.40,0.95,text)
#
#    # Variable nTotal is total number of 'attempt's in XML file.
#    # Compare this to the number of unique totaltimes:
#    if len(list_totaltime) != nTotal:
#        print '# of attempt in XML file:      ' + str(nTotal) + '\n'
#        print '# of unique total times in list: ' + str(len(list_totaltime)) + '\n'
#    else:
#        print 'Total number of unique total times matches number of attempts'
#
#    legend.Draw('same')
#
#    figureDir = 'figures/' + workflow_name_text
#    if not os.path.exists(figureDir): os.makedirs(figureDir)
#    c1.SaveAs(figureDir + '/hstack_times_totaltime.png')
#    c1.Close()
#    
#

# main function 
if __name__ == "__main__":
    main(sys.argv[1:])

