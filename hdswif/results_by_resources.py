#--------------------------------------------------------------------#
#                                                                    #
# 2015/09/09 Kei Moriya                                              #
#                                                                    #
# Parse the xml output of swif and create a dictionary               #
# that uses the resources as key values and has values               #
# that are dictionaries with Auger results as keys and               #
# number of counts as values.                                        #
#                                                                    #
# Example:                                                           #
#                                                                    #
# resource_dict['4000000000_480'] = {                                #
# 'SUCCESS'     : 150,                                               #
# 'FAILED'      :  10,                                               #
# 'TIMEOUT'     :   5,                                               #
# 'OVER_RLIMIT' :  30                                                #
# }                                                                  #
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

from ROOT import gROOT, gStyle, TCanvas, TH1F, TGraph, TH2F, TF1, TLine, TLatex
TLine.DrawClone._creates = False # This will allow use of DrawClone
import ROOT
gStyle.SetOptStat(False)
ROOT.gROOT.SetBatch(True)

import create_ordered_hists

def main(argv):

    # XML file to parse
    parser = OptionParser(usage = "\n results_by_resources.py [XML file]")
    (options, args) = parser.parse_args(argv)

    filename = args[0]

    filename_base = filename.rsplit('.xml')[0]

    outfile = open("summary_" + filename_base + ".html","a+") # append
    outfile.write('  <h2>Status by Resources</h2>\n')

    # Header for results for different resources
    outfile.write('  <table style="border: 0px; table-layout: fixed;">\n')
    outfile.write('    <tr style="background: #99CCFF;">\n')
    outfile.write('      <th colspan = "2" style="border: 0px; height:10px; width:300px; bgcolor = #ff0000;">Resources</th>\n')
    outfile.write('      <th colspan = "6" style="border: 0px; height:10px; width:500px; bgcolor = #00ff00;">Results</th>\n')
    outfile.write('      <th rowspan="2"><b>TOTAL</b></th>\n')
    outfile.write('    </tr>\n')
    outfile.write('    <tr style="background: #99CCFF;">\n')
    outfile.write('      <th style="border: 0px; height:10px; width:150px;">RAM (GB)</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:150px;">time limit (hrs)</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">SUCCESS</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">FAILED</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">TIMEOUT</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">OVER_RLIMIT</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">UNDISPATCHED</th>\n')
    outfile.write('      <th style="border: 0px; height:10px; width:100px;">CANCELLED</th>\n')
    outfile.write('    </tr>\n')

    # Read in xml file and create tree, root
    tree = ET.parse(filename)
    # root element is <workflow_status>
    workflow_status = tree.getroot()

    #--------------------------------------------------------------------
    # Create dictionary of resources and results
    resources_results_dict = {}

    #--------------------------------------------------------------------
    # Iterate over attempts and if the resources are already in
    # the dictionary increment the counts; if not, create a new entry.

    for attempt in workflow_status.iter('attempt'):
        ram = ''
        time = ''
        result = ''

        for ram_bytes in attempt.iter('ram_bytes'):
            ram = ram_bytes.text
        for time_secs in attempt.iter('time_secs'):
            time = time_secs.text
        for auger_result in attempt.iter('auger_result'):
            result = auger_result.text
        # Sometimes result is empty if the job has not run yet
        if result is '':
            result = 'UNDISPATCHED'
        

        # Create key for dictionary from ram and time
        resources = ram + '_' + time
        # print 'resources = ', resources, ' result = ', result

        # If key does not exist, create entry and set
        # value to empty dictionary
        if not resources_results_dict.has_key(resources):

            # Create dictionary that will be values of resources_results_dict
            empty_results_dict = {'SUCCESS'      : 0,
                                  'FAILED'       : 0,
                                  'TIMEOUT'      : 0,
                                  'OVER_RLIMIT'  : 0,
                                  'UNDISPATCHED' : 0,
                                  'CANCELLED' : 0
                                  }

            resources_results_dict[resources] = empty_results_dict

        # Increment for current result
        resources_results_dict[resources][result] += 1
    
    # totals for each type of result
    nTotal_SUCCESS      = 0
    nTotal_FAILED       = 0
    nTotal_TIMEOUT      = 0
    nTotal_OVER_RLIMIT  = 0
    nTotal_UNDISPATCHED = 0
    nTotal_CANCELLED    = 0
    nTotal_ALL          = 0

    # Print all values of dictionary
    for key, value in resources_results_dict.items():
        underscore_pos = key.find('_')
        ram_GB  = '{:5.2f}'.format(float(key[:underscore_pos]) / 1000 / 1000 / 1000)
        time_hr = '{:5.2f}'.format(float(key[underscore_pos+1:]) / 3600)
        # print 'ram_GB = ', ram_GB, ' time_hr = ', time_hr
        # print 'key = ', key, ' value = ', value
        output_text = '    <tr>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(ram_GB) + '</td>     <!-- RAM -->\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(time_hr) + '</td>    <!-- time -->\n'
        outfile.write(output_text)

        # Dictionaries are not sorted, save number of jobs for each result
        # in a temporary variable
        nSUCCESS      = 0
        nFAILED       = 0
        nTIMEOUT      = 0
        nOVER_RLIMIT  = 0
        nUNDISPATCHED = 0
        nCANCELLED = 0

        for result_key, result_value in value.items():
            # print 'result_key = ', result_key, ' result_value = ', result_value
            if result_key == 'SUCCESS':
                nSUCCESS = result_value
            elif result_key == 'FAILED':
                nFAILED = result_value
            elif result_key == 'TIMEOUT':
                nTIMEOUT = result_value
            elif result_key == 'OVER_RLIMIT':
                nOVER_RLIMIT = result_value
            elif result_key == 'UNDISPATCHED':
                nUNDISPATCHED = result_value
            elif result_key == 'CANCELLED':
                nCANCELLED = result_value
            else:
                print 'This should not happen!!'
                exit()

        # Total number of jobs for this resource
        nTotal_thisResource = nSUCCESS + nFAILED + nTIMEOUT + nOVER_RLIMIT + nUNDISPATCHED + nCANCELLED

        # Add job numbers to totals for each type of result
        nTotal_SUCCESS      += nSUCCESS
        nTotal_FAILED       += nFAILED
        nTotal_TIMEOUT      += nTIMEOUT
        nTotal_OVER_RLIMIT  += nOVER_RLIMIT
        nTotal_UNDISPATCHED += nUNDISPATCHED
        nTotal_CANCELLED    += nCANCELLED
        nTotal_ALL          += nTotal_thisResource

        output_text = '      <td>' + str(nSUCCESS) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nFAILED) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nTIMEOUT) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nOVER_RLIMIT) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nUNDISPATCHED) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nCANCELLED) + '</td>\n'
        outfile.write(output_text)
        output_text = '      <td>' + str(nTotal_thisResource) + '</td>\n'
        outfile.write(output_text)

        output_text = '      </tr>\n'
        outfile.write(output_text)

    # Row for totals of each result
    outfile.write('    <tr>\n')
    outfile.write('      <td colspan="2"><b>TOTAL</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_SUCCESS)      + '</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_FAILED)       + '</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_TIMEOUT)      + '</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_OVER_RLIMIT)  + '</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_UNDISPATCHED) + '</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_CANCELLED)    + '</b></td>\n')
    outfile.write('      <td><b>' + str(nTotal_ALL)          + '</b></td>\n')
    outfile.write('    </tr>\n')

    outfile.write('  </table>\n')

#------------------------------         end of main function          ---------------------------------#
    
## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
    
