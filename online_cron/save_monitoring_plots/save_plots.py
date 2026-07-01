#
# Make a log entry to save the rootspy plots after the first X events of run Y
#
# Write run number into a file.
#

import os
import subprocess

testing = False

events_required = 50000000 # 50M

runloggedfile = '_runlogged.txt'         # records last run for which plots were saved

#list of plots to include
include_histos = (' -H //BCAL_occupancy -H //CDC_occupancy -H //DIRC_occupancy -H //ECAL_occupancy -H //FCAL_occupancy -H //FDC_occupancy'
                  ' -H //FDC_P1_pseudo_occupancy  -H //FDC_P2_pseudo_occupancy  -H //FDC_P3_pseudo_occupancy  -H //FDC_P4_pseudo_occupancy'
                  ' -H //PS_occupancy -H //RF_TPOL_occupancy -H //ST_occupancy -H //TAGGER_occupancy -H //TOF_occupancy'
                  ' -H //HistMacro_Beam -H //HistMacro_Kinematics -H //HistMacro_NumHighLevelObjects -H //HistMacro_OnlineRF'
                  ' -H //HistMacro_PID -H //HistMacro_Trigger -H //HistMacro_Trigger_EnergyCorrelation -H //HistMacro_Vertex'
                  ' -H //HistMacro_bad_hits -H //HistMacro_fa125_itrig -H //HistMacro_fa125_temp')


#-----------------------------------------------------------------------------------


pvlist = ['HD:coda:daq:status', 'HD:coda:daq:run_number', 'HD:coda:EventNumber']
    
# the dictlist is a list of dicts like {pvname : last_recorded_value} with the value set to -1 for errors
# exit if there are network disconnections

dictlist = {}

for pv in pvlist:
    cmd_arr = ["myget", "-c", pv, "-t0", "-w-"]

    try:
        result = subprocess.check_output(cmd_arr)
        if testing:
            print(pv)
            print(result)

    except subprocess.CalledProcessError as e:
        if testing:
            print("myget command failed with return code: " + str(e.returncode))
            if e.output:
                print(e.output)
        exit()

    if 'Network disconnection' in result or 'fault' in result:
        exit()

    # 2026-07-01 10:24:22 1.5
    result = result.split()[2]

    dict = {pv : result}

    dictlist.update(dict)

#Require daq go
if dictlist['HD:coda:daq:status'] != '2':
    if testing:
        print('DAQ not running')
    exit()

current_events = float(dictlist['HD:coda:EventNumber'])
if current_events < events_required :
    if testing:
        print('Insufficient events ' + str(events_required))
    exit()

current_run = dictlist['HD:coda:daq:run_number']

line = 'xxx'
if os.path.exists(runloggedfile) :
    f = open(runloggedfile,'r')
    line = f.readline()
    f.close()

if current_run in line :
    if testing:
        print('Already logged plots for current run, ' + current_run)
        exit()
    exit()

    
# See https://logbooks.jlab.org/content/api-authentication
# Need this set: export JAVA_HOME=/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/

#cmd = '/gapps/RootSpy/dev/Linux_RHEL9-x86_64-gcc11.5.0/bin/RSelog -L HDMONITOR -R ' + current_run + include_histos

cmd_arr = ['/gapps/RootSpy/dev/Linux_RHEL9-x86_64-gcc11.5.0/bin/RSelog', '-L', 'HDMONITOR', '-R', current_run, include_histos]


if testing:
    print('logentry deactivated')
    print(cmd_arr)
    exit()

    
os.putenv('JAVA_HOME','/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/')  # needed to make the log entry

try:
    subprocess.check_call(cmd_arr)

except subprocess.CalledProcessError as e:
    print("RSelog failed with return code: " + str(e.returncode))
    exit()

f = open(runloggedfile,'w')
f.write(current_run)
f.close()
