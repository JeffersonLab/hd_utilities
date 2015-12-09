
#--------------------------------------------------------------------#
#                                                                    #
# 2015/12/02 Kei Moriya                                              #
#                                                                    #
# Create plots showing correlation of resource usage for files       #
# across different launches.                                         #
#                                                                    #
# Correlations are made for only files included in the launch        #
# of interest.                                                       #
#                                                                    #
#--------------------------------------------------------------------#

from sets import Set
from optparse import OptionParser
import os.path
import sys
import MySQLdb
import xml.etree.ElementTree as ET
import subprocess
import glob
import re
import shutil

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

# note: formatted version is string, not number
def format (ver, n = 2):
    form = "{:0>" + str(n) + "}"
    return form.format(ver)

def main(argv):
    VERBOSE = False

    # Read in command line args
    parser = OptionParser(usage = str("\n"
                                      + "create_resource_correlation_plots.py [run period] [minversion] [version]\n"
                                      + "example: create_resource_correlation_plots.py 2015_03 15 20\n"
                                      + "\n"
                                      + "Input xml file should be created with\n"
                                      + "hdswif.py summary [workflow]"))
    
    parser.add_option("-V","--verbose",dest="verbose",
                      help="verbose output")

    (options, args) = parser.parse_args(argv)

    if(options.verbose):
        VERBOSE = True
    
    if len(args) != 3:
        print "Usage:"
        print "create_resource_correlation_plots.py [run period] [minversion] [version]"
        print "example: create_resource_correlation_plots.py 2015_03 15 20"
        exit()
    
    RUNPERIOD = ""
    VERSION = ""

    # RUNPERIOD is with underscore
    RUNPERIOD = args[0]
    RUNPERIOD   = RUNPERIOD.replace('-','_')
    RUNPERIOD_HYPHEN = RUNPERIOD.replace('_', '-')
    MINVERSION = int(args[1])
    VERSION = int(args[2])
    assert MINVERSION < VERSION

    # Dictionary between ver to be filled and TGraph
    dict_cpu_sec = {}
    dict_wall_sec = {}
    dict_mem_kb = {}
    dict_vmem_kb = {}
    dict_nevents = {}
    dict_input_copy_sec = {}
    dict_plugin_sec = {}
    # This one is a TH1F
    dict_nevents_diff = {}

    for ver in range(MINVERSION,VERSION):
        dict_cpu_sec[ver] = TGraph()
        dict_wall_sec[ver] = TGraph()
        dict_mem_kb[ver] = TGraph()
        dict_vmem_kb[ver] = TGraph()
        dict_nevents[ver] = TGraph()
        dict_input_copy_sec[ver] = TGraph()
        dict_plugin_sec[ver] = TGraph()
        hname = 'dict_nevents_diff_' + format(ver,2)
        dict_nevents_diff[ver] = TH1F(hname,';#events ver' + str(VERSION) + ' - ver' + str(ver) + ';',400,-200,200)

    # Create MySQL connection
    db_conn = MySQLdb.connect(host='hallddb', user='farmer', passwd='', db='farming')
    cursor = db_conn.cursor()

    # Get all runs and files for the launch of interest (ver VERSION)
    tablename = 'cross_analysis_table_' + RUNPERIOD + '_ver' + str(VERSION)
    cursor.execute('SELECT run, file FROM ' + tablename)
    results = cursor.fetchall()

    # Keep the run/file combination in a set. The entries in the DB
    # should be unique, but this acts as a guard.
    set_run_file = Set()
    for row in results:
        run_file = format(row[0],6) + '_' + format(row[1],3)
        if run_file in set_run_file:
            print run_file + ' has already been accounted for, skipping'
            continue

        set_run_file.add(run_file)

    npoints = 0
    for run_file in set_run_file:
        npoints += 1
        if npoints % 500 == 0:
            print 'processing ' + format(npoints,4) + ' / ' + str(len(set_run_file)) + '...'

        match = re.search(r"(\d\d\d\d\d\d)_(\d\d\d)",run_file)
        run_num = match.group(1)
        file_num = match.group(2)
        if VERBOSE:
            print '-------------------------------------------------------------------'
            print 'run = ' + str(run_num) + ' file = ' + str(file_num)

        # Grab results from DB for each version for this file,
        # fill into dicts

        # Dictionaries between [version] : [resources]
        cpu_sec = {}
        wall_sec = {}
        mem_kb = {}
        vmem_kb = {}
        nevents = {}
        input_copy_sec = {}
        plugin_sec = {}

        # Start iterating from VERSION since we always
        # plot the resources for ver VERSION on the x-axis
        noValue = -999
        for ver in range(VERSION,MINVERSION-1,-1):
            cpu_sec[ver] = noValue
            wall_sec[ver] = noValue
            mem_kb[ver] = noValue
            vmem_kb[ver] = noValue
            nevents[ver] = noValue
            input_copy_sec[ver] = noValue
            plugin_sec[ver] = noValue

            tablename = 'cross_analysis_table_' + RUNPERIOD + '_ver' + str(ver)
            cursor.execute('SELECT cpu_sec, wall_sec, mem_kb, vmem_kb, nevents, input_copy_sec, plugin_sec FROM ' \
                               + tablename + ' WHERE run = ' + str(run_num) + ' AND file = ' + str(file_num))
            results = cursor.fetchall()

            for row in results:
                cpu_sec[ver]        = row[0]
                wall_sec[ver]       = row[1]
                mem_kb[ver]         = row[2]
                vmem_kb[ver]        = row[3]
                nevents[ver]        = row[4]
                input_copy_sec[ver] = row[5]
                plugin_sec[ver]     = row[6]

            if ver == VERSION:
                # Nothing to fill for ver VERSION
                continue

            # Fill correlations plots
            if cpu_sec[VERSION] != noValue and cpu_sec[ver] != noValue:
                dict_cpu_sec[ver].SetPoint(dict_cpu_sec[ver].GetN(),float(cpu_sec[VERSION]) / 60,float(cpu_sec[ver]) / 60)
            if wall_sec[VERSION] != noValue and wall_sec[ver] != noValue:
                dict_wall_sec[ver].SetPoint(dict_wall_sec[ver].GetN(),float(wall_sec[VERSION]) / 60,float(wall_sec[ver]) / 60)
            if mem_kb[VERSION] != noValue and mem_kb[ver] != noValue:
                dict_mem_kb[ver].SetPoint(dict_mem_kb[ver].GetN(),float(mem_kb[VERSION]) / 1024 / 1024,float(mem_kb[ver]) / 1024 / 1024)
            if vmem_kb[VERSION] != noValue and vmem_kb[ver] != noValue:
                dict_vmem_kb[ver].SetPoint(dict_vmem_kb[ver].GetN(),float(vmem_kb[VERSION]) / 1024 / 1024,float(vmem_kb[ver]) / 1024 / 1024)
            if nevents[VERSION] != noValue and nevents[ver] != noValue:
                dict_nevents[ver].SetPoint(dict_nevents[ver].GetN(),float(nevents[VERSION]),float(nevents[ver]))
                dict_nevents_diff[ver].Fill(float(nevents[VERSION]) - float(nevents[ver]))
            if input_copy_sec[VERSION] != noValue and input_copy_sec[ver] != noValue:
                dict_input_copy_sec[ver].SetPoint(dict_input_copy_sec[ver].GetN(),float(input_copy_sec[VERSION]) / 60,float(input_copy_sec[ver]) / 60)
            if plugin_sec[VERSION] != noValue and plugin_sec[ver] != noValue:
                dict_plugin_sec[ver].SetPoint(dict_plugin_sec[ver].GetN(),float(plugin_sec[VERSION]) / 60,float(plugin_sec[ver]) / 60)

        # end of loop over vers
    # end of loop over run and files

    # Create html page
    htmlDir = 'resource_use'
    if not os.path.exists(htmlDir):
        os.makedirs(htmlDir)
    htmlfilename = htmlDir + '/resource_use_' + RUNPERIOD + '_ver' + format(VERSION,2) + '.html'
    htmlfile = open(htmlfilename,'w')

    htmlfile.write('<!-- launch ver ' + format(VERSION,2) + '-->\n')
    htmlfile.write('<html>\n')
    htmlfile.write('\n')
    htmlfile.write('<head>\n')
    htmlfile.write('<link rel="stylesheet" type="text/css" href="mystyle.css">\n')
    htmlfile.write('<meta http-equiv="content-style-type" content="text/css">\n')
    htmlfile.write('<title>Offline Monitoring Results 2015-03</title>\n')
    htmlfile.write('\n')
    htmlfile.write('<style>\n')
    htmlfile.write(' #content iframe{\n')
    htmlfile.write('        overflow: hidden;\n')
    htmlfile.write('        width:100%;\n')
    htmlfile.write('        border:none;\n')
    htmlfile.write('        margin: 0;\n')
    htmlfile.write('        padding: 0;\n')
    htmlfile.write('    }\n')
    htmlfile.write('</style>\n')
    htmlfile.write('\n')
    htmlfile.write('<script language="javascript" type="text/javascript">\n')
    htmlfile.write('  function resizeIframe(obj) {\n')
    htmlfile.write("    obj.style.height = obj.contentWindow.document.body.scrollHeight + 'px';\n")
    htmlfile.write('  }\n')
    htmlfile.write('</script>\n')
    htmlfile.write('\n')
    htmlfile.write('</head>\n')
    htmlfile.write('\n')
    htmlfile.write('<body>\n')

    htmlfile.write('<h2>Comparison of ver' + format(VERSION,2) + ' and older</h2>\n')
    tablewidth = (VERSION - MINVERSION) * 20
    htmlfile.write('<table style="width:' + str(tablewidth) + '%">\n')
    # cpu_sec
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> CPU time (min) </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 01___cpu_sec ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/01___cpu_sec_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/01___cpu_sec_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # wall_sec
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> wall time (min) </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 02___wall_sec ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/02___wall_sec_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/02___wall_sec_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # mem_kb
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> Memory (GB) </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 03___mem_kb ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/03___mem_kb_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/03___mem_kb_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # vmem_kb
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> Virtual memory (GB) </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 04___vmem_kb ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/04___vmem_kb_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/04___vmem_kb_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # nevents
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> #events </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 05___nevents ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/05___nevents_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/05___nevents_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # nevents_diff
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> &Delta; events </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 06___nevents_diff ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/06___nevents_diff_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/06___nevents_diff_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # input_copy_sec
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> input copy time (min) </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 07___input_copy_sec ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/07___input_copy_sec_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/07___input_copy_sec_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    # plugin_time
    htmlfile.write('  <tr>\n')
    htmlfile.write('    <td colspan="' + str(VERSION - MINVERSION) + '"> plugin time (min) </td>\n')
    htmlfile.write('  </tr>\n')
    htmlfile.write('  <tr>\n')
    for ver in range(MINVERSION,VERSION):
        htmlfile.write('    <!-- 08___plugin_time ver' + format(ver,2) + '-->\n')
        htmlfile.write('    <td style="width:20%">\n')
        htmlfile.write('      <a href="./figures/SWIF/ver' + format(VERSION,2) + '/08___plugin_time_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png">\n')
        htmlfile.write('        <img src="./figures/SWIF/ver' + format(VERSION,2) + '/08___plugin_time_ver' \
                           + format(VERSION,2) + '_ver' + format(ver,2) + '.png" width="100%">\n')
        htmlfile.write('    </a>\n')
        htmlfile.write('    </td>\n')
    htmlfile.write('  </tr>\n')

    htmlfile.write('</table>\n')
    htmlfile.write('</body>\n')
    htmlfile.write('</html>\n')
    htmlfile.close()

    line = TLine(0,0,0,0)

    latex = TLatex()
    latex.SetTextColor(ROOT.kBlack)
    latex.SetTextSize(0.06)
    latex.SetTextAlign(12)
    latex.SetNDC(1)

    figureDir = htmlDir + '/figures/SWIF/ver' + format(VERSION,2)
    if not os.path.exists(figureDir):
        os.makedirs(figureDir)

    for ver in range(MINVERSION,VERSION):
        #--------------------------------------------------------------------
        #                                cpu_sec
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.02)
        c1.SetRightMargin(0.02)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_cpu_sec[ver].GetXaxis().SetTitleSize(0.06)
        dict_cpu_sec[ver].GetXaxis().SetTitleOffset(0.900)
        dict_cpu_sec[ver].GetXaxis().SetLabelSize(0.05)
        dict_cpu_sec[ver].GetYaxis().SetTitleSize(0.06)
        dict_cpu_sec[ver].GetYaxis().SetTitleOffset(1.200)
        dict_cpu_sec[ver].GetYaxis().CenterTitle()
        dict_cpu_sec[ver].GetYaxis().SetLabelSize(0.05)
        dict_cpu_sec[ver].SetMarkerColor(ROOT.kRed)
        dict_cpu_sec[ver].SetMarkerStyle(20)
        dict_cpu_sec[ver].SetMarkerSize(1.2)
        dict_cpu_sec[ver].SetLineColor(ROOT.kRed)
        dict_cpu_sec[ver].SetLineStyle(1)
        dict_cpu_sec[ver].SetLineWidth(1)
        dict_cpu_sec[ver].GetXaxis().SetTitle('CPU time (min) for ver' + format(VERSION,2))
        dict_cpu_sec[ver].GetYaxis().SetTitle('CPU time (min) for ver' + format(ver,2))

        X = dict_cpu_sec[ver].GetXaxis().GetXmax()
        Y = dict_cpu_sec[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_cpu_sec[ver].GetXaxis().SetLimits(0,larger)
        dict_cpu_sec[ver].SetMinimum(0)
        dict_cpu_sec[ver].SetMaximum(larger)

        dict_cpu_sec[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_cpu_sec[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/01___cpu_sec_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                wall_sec
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.02)
        c1.SetRightMargin(0.02)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_wall_sec[ver].GetXaxis().SetTitleSize(0.06)
        dict_wall_sec[ver].GetXaxis().SetTitleOffset(0.900)
        dict_wall_sec[ver].GetXaxis().SetLabelSize(0.05)
        dict_wall_sec[ver].GetYaxis().SetTitleSize(0.06)
        dict_wall_sec[ver].GetYaxis().SetTitleOffset(1.200)
        dict_wall_sec[ver].GetYaxis().CenterTitle()
        dict_wall_sec[ver].GetYaxis().SetLabelSize(0.05)
        dict_wall_sec[ver].SetMarkerColor(ROOT.kRed)
        dict_wall_sec[ver].SetMarkerStyle(20)
        dict_wall_sec[ver].SetMarkerSize(1.2)
        dict_wall_sec[ver].SetLineColor(ROOT.kRed)
        dict_wall_sec[ver].SetLineStyle(1)
        dict_wall_sec[ver].SetLineWidth(1)
        dict_wall_sec[ver].GetXaxis().SetTitle('wall time (min) for ver' + format(VERSION,2))
        dict_wall_sec[ver].GetYaxis().SetTitle('wall time (min) for ver' + format(ver,2))

        X = dict_wall_sec[ver].GetXaxis().GetXmax()
        Y = dict_wall_sec[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_wall_sec[ver].GetXaxis().SetLimits(0,larger)
        dict_wall_sec[ver].SetMinimum(0)
        dict_wall_sec[ver].SetMaximum(larger)

        dict_wall_sec[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_wall_sec[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/02___wall_sec_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                mem_kb
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.02)
        c1.SetRightMargin(0.02)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_mem_kb[ver].GetXaxis().SetTitleSize(0.06)
        dict_mem_kb[ver].GetXaxis().SetTitleOffset(0.900)
        dict_mem_kb[ver].GetXaxis().SetLabelSize(0.05)
        dict_mem_kb[ver].GetYaxis().SetTitleSize(0.06)
        dict_mem_kb[ver].GetYaxis().SetTitleOffset(1.200)
        dict_mem_kb[ver].GetYaxis().CenterTitle()
        dict_mem_kb[ver].GetYaxis().SetLabelSize(0.05)
        dict_mem_kb[ver].SetMarkerColor(ROOT.kRed)
        dict_mem_kb[ver].SetMarkerStyle(20)
        dict_mem_kb[ver].SetMarkerSize(1.2)
        dict_mem_kb[ver].SetLineColor(ROOT.kRed)
        dict_mem_kb[ver].SetLineStyle(1)
        dict_mem_kb[ver].SetLineWidth(1)
        dict_mem_kb[ver].GetXaxis().SetTitle('mem (GB) for ver' + format(VERSION,2))
        dict_mem_kb[ver].GetYaxis().SetTitle('mem (GB) for ver' + format(ver,2))

        X = dict_mem_kb[ver].GetXaxis().GetXmax()
        Y = dict_mem_kb[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_mem_kb[ver].GetXaxis().SetLimits(0,larger)
        dict_mem_kb[ver].SetMinimum(0)
        dict_mem_kb[ver].SetMaximum(larger)

        dict_mem_kb[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_mem_kb[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/03___mem_kb_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                vmem_kb
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.02)
        c1.SetRightMargin(0.02)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_vmem_kb[ver].GetXaxis().SetTitleSize(0.06)
        dict_vmem_kb[ver].GetXaxis().SetTitleOffset(0.900)
        dict_vmem_kb[ver].GetXaxis().SetLabelSize(0.05)
        dict_vmem_kb[ver].GetYaxis().SetTitleSize(0.06)
        dict_vmem_kb[ver].GetYaxis().SetTitleOffset(1.200)
        dict_vmem_kb[ver].GetYaxis().CenterTitle()
        dict_vmem_kb[ver].GetYaxis().SetLabelSize(0.05)
        dict_vmem_kb[ver].SetMarkerColor(ROOT.kRed)
        dict_vmem_kb[ver].SetMarkerStyle(20)
        dict_vmem_kb[ver].SetMarkerSize(1.2)
        dict_vmem_kb[ver].SetLineColor(ROOT.kRed)
        dict_vmem_kb[ver].SetLineStyle(1)
        dict_vmem_kb[ver].SetLineWidth(1)
        dict_vmem_kb[ver].GetXaxis().SetTitle('vmem (GB) for ver' + format(VERSION,2))
        dict_vmem_kb[ver].GetYaxis().SetTitle('vmem (GB) for ver' + format(ver,2))

        X = dict_vmem_kb[ver].GetXaxis().GetXmax()
        Y = dict_vmem_kb[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_vmem_kb[ver].GetXaxis().SetLimits(0,larger)
        dict_vmem_kb[ver].SetMinimum(0)
        dict_vmem_kb[ver].SetMaximum(larger)

        dict_vmem_kb[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_vmem_kb[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/04___vmem_kb_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                nevents
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.04)
        c1.SetRightMargin(0.04)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_nevents[ver].GetXaxis().SetTitleSize(0.06)
        dict_nevents[ver].GetXaxis().SetTitleOffset(0.900)
        dict_nevents[ver].GetXaxis().SetLabelSize(0.05)
        dict_nevents[ver].GetYaxis().SetTitleSize(0.06)
        dict_nevents[ver].GetYaxis().SetTitleOffset(1.200)
        dict_nevents[ver].GetYaxis().CenterTitle()
        dict_nevents[ver].GetYaxis().SetLabelSize(0.05)
        dict_nevents[ver].SetMarkerColor(ROOT.kRed)
        dict_nevents[ver].SetMarkerStyle(20)
        dict_nevents[ver].SetMarkerSize(1.2)
        dict_nevents[ver].SetLineColor(ROOT.kRed)
        dict_nevents[ver].SetLineStyle(1)
        dict_nevents[ver].SetLineWidth(1)
        dict_nevents[ver].GetXaxis().SetTitle('#events for ver' + format(VERSION,2))
        dict_nevents[ver].GetYaxis().SetTitle('#events for ver' + format(ver,2))

        X = dict_nevents[ver].GetXaxis().GetXmax()
        Y = dict_nevents[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_nevents[ver].GetXaxis().SetLimits(0,larger)
        dict_nevents[ver].SetMinimum(0)
        dict_nevents[ver].SetMaximum(larger)

        dict_nevents[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_nevents[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/05___nevents_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                nevents_diff
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.04)
        c1.SetRightMargin(0.04)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_nevents_diff[ver].GetXaxis().SetTitleSize(0.06)
        dict_nevents_diff[ver].GetXaxis().SetTitleOffset(0.900)
        dict_nevents_diff[ver].GetXaxis().SetLabelSize(0.05)
        dict_nevents_diff[ver].GetYaxis().SetTitleSize(0.06)
        dict_nevents_diff[ver].GetYaxis().SetTitleOffset(1.200)
        dict_nevents_diff[ver].GetYaxis().CenterTitle()
        dict_nevents_diff[ver].GetYaxis().SetLabelSize(0.05)
        dict_nevents_diff[ver].SetLineColor(ROOT.kRed)
        dict_nevents_diff[ver].SetLineStyle(1)
        dict_nevents_diff[ver].SetLineWidth(1)

        c1.SetLogy(1)
        dict_nevents_diff[ver].Draw()

        text = "total: " + str(int(dict_nevents_diff[ver].GetEntries()))
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/06___nevents_diff_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                input_copy_sec
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.02)
        c1.SetRightMargin(0.02)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_input_copy_sec[ver].GetXaxis().SetTitleSize(0.06)
        dict_input_copy_sec[ver].GetXaxis().SetTitleOffset(0.900)
        dict_input_copy_sec[ver].GetXaxis().SetLabelSize(0.05)
        dict_input_copy_sec[ver].GetYaxis().SetTitleSize(0.06)
        dict_input_copy_sec[ver].GetYaxis().SetTitleOffset(1.200)
        dict_input_copy_sec[ver].GetYaxis().CenterTitle()
        dict_input_copy_sec[ver].GetYaxis().SetLabelSize(0.05)
        dict_input_copy_sec[ver].SetMarkerColor(ROOT.kRed)
        dict_input_copy_sec[ver].SetMarkerStyle(20)
        dict_input_copy_sec[ver].SetMarkerSize(1.2)
        dict_input_copy_sec[ver].SetLineColor(ROOT.kRed)
        dict_input_copy_sec[ver].SetLineStyle(1)
        dict_input_copy_sec[ver].SetLineWidth(1)
        dict_input_copy_sec[ver].GetXaxis().SetTitle('input copy time (min) for ver' + format(VERSION,2))
        dict_input_copy_sec[ver].GetYaxis().SetTitle('input copy time (min) for ver' + format(ver,2))

        X = dict_input_copy_sec[ver].GetXaxis().GetXmax()
        Y = dict_input_copy_sec[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_input_copy_sec[ver].GetXaxis().SetLimits(0,larger)
        dict_input_copy_sec[ver].SetMinimum(0)
        dict_input_copy_sec[ver].SetMaximum(larger)

        dict_input_copy_sec[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_input_copy_sec[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/07___input_copy_sec_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

        #--------------------------------------------------------------------
        #                                plugin_sec
        #--------------------------------------------------------------------
        c1 = TCanvas( 'c1', 'canvas', 0, 0, 1600, 1200 )
        c1.SetTopMargin(0.02)
        c1.SetRightMargin(0.02)
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.14)

        dict_plugin_sec[ver].GetXaxis().SetTitleSize(0.06)
        dict_plugin_sec[ver].GetXaxis().SetTitleOffset(0.900)
        dict_plugin_sec[ver].GetXaxis().SetLabelSize(0.05)
        dict_plugin_sec[ver].GetYaxis().SetTitleSize(0.06)
        dict_plugin_sec[ver].GetYaxis().SetTitleOffset(1.200)
        dict_plugin_sec[ver].GetYaxis().CenterTitle()
        dict_plugin_sec[ver].GetYaxis().SetLabelSize(0.05)
        dict_plugin_sec[ver].SetMarkerColor(ROOT.kRed)
        dict_plugin_sec[ver].SetMarkerStyle(20)
        dict_plugin_sec[ver].SetMarkerSize(1.2)
        dict_plugin_sec[ver].SetLineColor(ROOT.kRed)
        dict_plugin_sec[ver].SetLineStyle(1)
        dict_plugin_sec[ver].SetLineWidth(1)
        dict_plugin_sec[ver].GetXaxis().SetTitle('plugin time (min) for ver' + format(VERSION,2))
        dict_plugin_sec[ver].GetYaxis().SetTitle('plugin time (min) for ver' + format(ver,2))

        X = dict_plugin_sec[ver].GetXaxis().GetXmax()
        Y = dict_plugin_sec[ver].GetYaxis().GetXmax()
        larger = Y if X < Y else X

        # don't show negative values (should not exist)
        dict_plugin_sec[ver].GetXaxis().SetLimits(0,larger)
        dict_plugin_sec[ver].SetMinimum(0)
        dict_plugin_sec[ver].SetMaximum(larger)

        dict_plugin_sec[ver].Draw('AP')

        line.SetX1(0)
        line.SetY1(0)
        line.SetX2(larger)
        line.SetY2(larger)
        line.SetLineColor(ROOT.kMagenta)
        line.SetLineStyle(2)
        line.DrawClone("same")

        text = "total: " + str(dict_plugin_sec[ver].GetN())
        latex.SetTextColor(ROOT.kBlue)
        latex.DrawLatex(0.75,0.92,text)

        c1.SaveAs(figureDir + '/08___plugin_time_ver' + format(VERSION,2) + '_ver' + format(ver,2) + '.png')
        c1.Close()

    # Copy all files to web-accessible directory
    outDir = '/group/halld/www/halldweb/html/data_monitoring/launch_analysis/' + RUNPERIOD + '/resource_use/'
    shutil.copy(htmlfilename,outDir)
    outputfigureDir = outDir + '/figures/SWIF/ver' + format(VERSION,2)
    # delete directory if it already exists (necessary for shutil.copytree)
    if os.path.exists(outputfigureDir):
        shutil.rmtree(outputfigureDir)
    shutil.copytree(figureDir,outputfigureDir + '/')

    # Create link from main [RUNPERIOD].html file

    # Add the newest row into the html table using the hook
    # that is added at the end of each launch version.

    runperiodfilename = '/group/halld/www/halldweb/html/data_monitoring/launch_analysis/' + RUNPERIOD + '/' + RUNPERIOD + '.html'
    with open(runperiodfilename,'r') as runperiodfile:
        alllines = runperiodfile.readlines()

    # Reverse the lines and find the first occurrence of the hook
    # this is the final (most recent hook)
    alllines.reverse()

    nlines = 0
    pos = 0
    old_hook = ''
    for line in alllines:
        nlines += 1
        if re.search('<!--2015_03_ver[0-9][0-9] resource_use-->',line):
           pos = nlines
           old_hook = line
           break

    if old_hook == '':
        print 'Did not find old hook, make sure that file'
        print runperiodfilename
        print 'has a line containing <!--[RUNPERIOD]_ver[VERSION] resource_use-->'
        exit()
    
    # position to seek is total number of lines in file minus pos + 1
    pos = len(alllines) - pos + 1

    # lines for link
    linklines = []
    linklines.append('  <li style="color:blue;">\n')
    linklines.append('    <h3><a href = "resource_use/resource_use_2015_03_ver' + format(VERSION,2) + '.html">' \
                         + 'ver' + format(VERSION,2) + '</a></h3> <!--' + RUNPERIOD + '_ver' \
                         + format(VERSION,2) + ' resource_use-->\n')
    
    # Reopen stats html file and insert newlines into line pos
    with open(runperiodfilename, 'r+') as runperiodfile:
        alllines = runperiodfile.readlines()

    alllines[pos:pos] = linklines

    # Write the new file
    with open(runperiodfilename, 'w') as runperiodfile:
        for line in alllines:
            runperiodfile.write(line)

    exit()

if __name__ == "__main__":
   main(sys.argv[1:])

