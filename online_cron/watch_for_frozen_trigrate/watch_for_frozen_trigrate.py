#
# Plan: check that these epics PVs are alive: HD:trig:rate:main
#
#
#
# If beam is on and daq is on, collimator is not blocking, and there is a radiator, 
# trigger rate should have nonzero sigma for the last n minutes.
#
# Averaging this over 3h and checking the sigma if the mean >0 
#
#
# Write logbook entry or send email or ask Hovanes for a new epics pv to caput.
#
# Have this code run just once, looking at the last minute.
# Call it repeatedly via cron.
#
# Write time of last warning into a file, so that further warnings can be suppressed for a while.
#

import os
from datetime import datetime,timedelta
from time import sleep

testing = 0

beam_on_current = 10               # minimum nA to consider as beam on
trig_warning_suppression_time = timedelta(minutes=60)   # time between repeated warnings
outage_warning_suppression_time = timedelta(minutes=60)
err_warning_suppression_time = timedelta(minutes=240)

integrationtime_minutes = 3*60   # length of epics recalled stats  NB this is used for mystats and also for the log message

sleeptime = 60   # if one variable is disconnected, try again in 1 minute

# filenames
epicsfile = '_watch_epics.txt'             # mystats printout

radfile = '_watch_epics_radname.txt'       # myget output for radiator name

trigfile = '_watch_epics_trig_warning.txt' # time of last frozen trig-rate warning
errfile = '_watch_epics_err_warning.txt'           # time of last mystats error warning
outagefile = '_watch_epics_outage_warning.txt'     # time of last mystats outage warning


timeformat = "%Y-%m-%d %H:%M:%S"
timenow = datetime.now()

logbook = "HDLOG"
#logbook = "TLOG"


wavetimeformat = "%Y-%m-%dT%H:%M:%S"
wavestart = datetime.strftime(timenow - timedelta(minutes=integrationtime_minutes), wavetimeformat) 
wavestop = datetime.strftime(timenow, wavetimeformat)  
wavestart = wavestart.replace(":","%3A")
wavestop = wavestop.replace(":","%3A")

daq_wave = "pv=IBCAD00CRCUR6&pv=HD%3Atrig%3Arate%3Amain&pv=HD:coda:LiveTime"
daq_wave += "&IBCAD00CRCUR6color=%23ee0000&HD%3Atrig%3Arate%3Amaincolor=%23000000&HD:coda:LiveTimecolor=%2377bb77"
base_url = "https://epicsweb.jlab.org/wave/?myaLimit=100000&windowMinutes=30&title=&fullscreen=false&layoutMode=1&viewerMode=1"
base_url += "&start=" + wavestart + "&end=" + wavestop + "&"

# viewerMode 1=fixed 2=live
# layoutMode 1=separate charts 3=same chart,separate axes 

# live, separate plots
# https://epicsweb.jlab.org/wave/?myaLimit=100000&windowMinutes=30&title=&fullscreen=false&layoutMode=1&viewerMode=2&pv=HD%3Atrig%3Arate%3Amain&start=2026-05-04T17%3A00%3A00&end=2026-05-05T09%3A40%3A00&myaDeployment=&IBCAD00CRCUR6yAxisLabel=&IBCAD00CRCUR6yAxisMin=&IBCAD00CRCUR6yAxisMax=&IBCAD00CRCUR6yAxisLog=&IBCAD00CRCUR6scaler=&HD%3Atrig%3Arate%3Amainlabel=HD%3Atrig%3Arate%3Amain&HD%3Atrig%3Arate%3Amaincolor=%23000000&HD%3Atrig%3Arate%3AmainyAxisLabel=&HD%3Atrig%3Arate%3AmainyAxisMin=&HD%3Atrig%3Arate%3AmainyAxisMax=1e5&HD%3Atrig%3Arate%3AmainyAxisLog&HD%3Atrig%3Arate%3Amainscaler=&HD%3Acoda%3ALiveTimeyAxisLabel=&HD%3Acoda%3ALiveTimeyAxisMin=&HD%3Acoda%3ALiveTimeyAxisMax=&HD%3Acoda%3ALiveTimeyAxisLog=&HD%3Acoda%3ALiveTimescaler=&PS%3Acoinc%3Ascaler%3ArateyAxisLabel=&PS%3Acoinc%3Ascaler%3ArateyAxisMin=&PS%3Acoinc%3Ascaler%3ArateyAxisMax=&PS%3Acoinc%3Ascaler%3ArateyAxisLog=&PS%3Acoinc%3Ascaler%3Aratescaler=&PSC%3Acoinc%3Ascaler%3ArateyAxisLabel=&PSC%3Acoinc%3Ascaler%3ArateyAxisMin=&PSC%3Acoinc%3Ascaler%3ArateyAxisMax=&PSC%3Acoinc%3Ascaler%3ArateyAxisLog=&PSC%3Acoinc%3Ascaler%3Aratescaler=&pv=SOL%3Ai%3ABarPress2

# fixed

# https://epicsweb.jlab.org/wave/?myaLimit=100000&windowMinutes=30&title=&fullscreen=false&layoutMode=1&viewerMode=1&pv=HD%3Atrig%3Arate%3Amain&pv=SOL%3Ai%3ABarPress2&start=2026-05-05T11%3A56%3A24&end=2026-05-05T12%3A26%3A24&myaDeployment=&IBCAD00CRCUR6yAxisLabel=&IBCAD00CRCUR6yAxisMin=&IBCAD00CRCUR6yAxisMax=&IBCAD00CRCUR6yAxisLog=&IBCAD00CRCUR6scaler=&HD%3Atrig%3Arate%3Amainlabel=HD%3Atrig%3Arate%3Amain&HD%3Atrig%3Arate%3Amaincolor=%23000000&HD%3Atrig%3Arate%3AmainyAxisLabel=&HD%3Atrig%3Arate%3AmainyAxisMin=&HD%3Atrig%3Arate%3AmainyAxisMax=1e5&HD%3Atrig%3Arate%3AmainyAxisLog&HD%3Atrig%3Arate%3Amainscaler=&HD%3Acoda%3ALiveTimeyAxisLabel=&HD%3Acoda%3ALiveTimeyAxisMin=&HD%3Acoda%3ALiveTimeyAxisMax=&HD%3Acoda%3ALiveTimeyAxisLog=&HD%3Acoda%3ALiveTimescaler=&PS%3Acoinc%3Ascaler%3ArateyAxisLabel=&PS%3Acoinc%3Ascaler%3ArateyAxisMin=&PS%3Acoinc%3Ascaler%3ArateyAxisMax=&PS%3Acoinc%3Ascaler%3ArateyAxisLog=&PS%3Acoinc%3Ascaler%3Aratescaler=&PSC%3Acoinc%3Ascaler%3ArateyAxisLabel=&PSC%3Acoinc%3Ascaler%3ArateyAxisMin=&PSC%3Acoinc%3Ascaler%3ArateyAxisMax=&PSC%3Acoinc%3Ascaler%3ArateyAxisLog=&PSC%3Acoinc%3Ascaler%3Aratescaler=&SOL%3Ai%3ABarPress2label=SOL%3Ai%3ABarPress2&SOL%3Ai%3ABarPress2color=%231f78b4&SOL%3Ai%3ABarPress2yAxisLabel=&SOL%3Ai%3ABarPress2yAxisMin=&SOL%3Ai%3ABarPress2yAxisMax=&SOL%3Ai%3ABarPress2yAxisLog=&SOL%3Ai%3ABarPress2scaler=


#-----------------------------------------------------------------------------------

def run_mystats(epicsfile) :
    
    pvlist = 'IBCAD00CRCUR6,HD:coda:daq:status,HD:coda:LiveTime,HD:trig:rate:main'

    if os.path.exists(epicsfile):
        os.remove(epicsfile)

    cmd = "myStats -b -" + str(integrationtime_minutes) + "m -e 0 -u -f " + epicsfile + " -l " + pvlist

    os.system(cmd)
    
    return

#-----------------------------------------------------------------------------------

def check_output_exists(thisfile):
    
    if os.path.exists(thisfile):
        return True
    
    time_since_warning = find_time_since_warning(errfile,timeformat)
    
    if time_since_warning > err_warning_suppression_time :
        issue_warning('err')
        record_time(timenow,errfile,timeformat)
    else :
        if testing:
            print('EPICS archive access error warning issued recently')

    return False


#-----------------------------------------------------------------------------------

def read_mystats(epicsfile) :

    f = open(epicsfile,'r')
    lines = f.readlines()
    f.close()

    dictlist = {}    
    # Name     Min   Mean   Max   Sigma
    
    for line in lines:
 
        things = line.strip().split(" ")
        pvname = things[0]
            
        if things[1] == 'N/A' :
            min = -1
        elif things[1] == '<undefined>' :
            min = -1
        else :
            min = float(things[1])        
        
        if things[2] == 'N/A' :
            mean = -1
        elif things[2] == '<undefined>' :
            mean = -1
        else :
            mean = float(things[2])

        if things[3] == 'N/A' :
            max = -1
        elif things[3] == '<undefined>' :
            max = -1
        else :
            max = float(things[3])

        if things[4] == 'N/A' :
            sigma = -1
        elif things[4] == '<undefined>' :
            sigma = -1
        else :
            sigma = float(things[4])        

        dict = {pvname : { "min":min, "mean":mean, "max":max, "sigma":sigma}}
        dictlist.update(dict)
        
    return dictlist

#-----------------------------------------------------------------------------------

    
#-----------------------------------------------------------------------------------    


def issue_warning(pv) :

    # See https://logbooks.jlab.org/content/api-authentication
    # Need this set: export JAVA_HOME=/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/
    
    os.putenv('JAVA_HOME','/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/')  # needed to make the log entry
    
    if pv == 'trig':
        title = 'HD:trig:rate:main looks frozen'
        body = 'msg_restart_trigger_client.txt'
    else :
        title = 'EPICS outage'
        f = open('_msg_outage.txt','w')
        f.write("EPICS is not available for the following, please check the IOCs:\n")
        f.write(pv)
        f.write("\n\n")
        f.close()
        body = '_msg_outage.txt'
        
    f = open(body,'r')
    lines = f.readlines()
    f.close()
    
    f2 = open('logmsg.txt','w')
    for line in lines:
        f2.write(line)
        
    f = open(epicsfile,'r')
    lines = f.readlines()
    f.close()

    header = "\nEPICS output for the last " + str(integrationtime_minutes) + " minutes:\n\n"
    f2.write(header)
    f2.write("Name                   Min       Mean      Max       Sigma\n")

    for line in lines:
        things = line.strip().split(" ")
        newline = '{:<23}'.format(things[0])
        for i in range(1,len(things)):
            newline = newline + '{:14}'.format(things[i])
        f2.write(newline + "\n")


    f2.write("\n\n")


    wave_url = base_url + daq_wave

    f2.write("EPICS wave url:\n")
    
    f2.write(wave_url)
    f2.write("\n\n")
    
    import platform

    f2.write("This message was generated by Naomi's watch_for_frozen_trigrate script, running as a cron job on " + platform.node() + "\n")
    f2.close()

    cmd = '/site/ace/certified/apps/bin/logentry -l ' + logbook + ' -g Autolog -t "' + title + '" -b logmsg.txt -n njarvis@jlab.org'

    if testing == 1:
        print(cmd)
        print('logbook entry deactivated')
    else :
        os.system(cmd)
    return

#-----------------------------------------------------------------------------------

def find_time_since_warning(filename,timeformat) :
    
    if not os.path.exists(filename):
        time_since_warning = timedelta(days=1000)     # no previous warnings were made 
    else :
        lastwarning = read_time(filename)
        time_since_warning = timenow - datetime.strptime(lastwarning,timeformat)   #this is a timedelta
    return time_since_warning

#-----------------------------------------------------------------------------------

def read_time(filename) :

    f = open(filename,'r')
    time_recorded = f.readline()
    f.close()
    return time_recorded

#-----------------------------------------------------------------------------------

def record_time(timenow,filename,timeformat) :

    f = open(filename,'w')
    f.write(timenow.strftime(timeformat))
    f.close()       
    return

#-----------------------------------------------------------------------------------
#-----------------------------------------------------------------------------------


run_mystats(epicsfile)

allok = check_output_exists(epicsfile)   # this issues a warning if the file is not there

if not allok:
    exit()

dictlist = read_mystats(epicsfile)   

# the dictlist is a list of dicts like 
# eg {'IBCAD00CRCUR6': {'mean': 0.275, 'sigma': 0.641775}}
# invalid mean or sigma, eg 'N/A', are set to -1

outages = ""

for dict in dictlist:
    if dictlist[dict]['min'] < 0  or dictlist[dict]['mean'] < 0  or dictlist[dict]['sigma'] < 0 :
        outages = outages + dict + " "

if outages != "" :
    time_since_warning = find_time_since_warning(outagefile,timeformat)
    
    if time_since_warning > outage_warning_suppression_time :
        issue_warning(outages)
        record_time(timenow,outagefile,timeformat)
    else :
        if testing:
            print('EPICS outage warning issued recently')
    exit()

# no outages were found, so check other data


if dictlist['HD:coda:daq:status']['max'] != 2 :   # daq status - HD:coda:daq:status = 2 for 'go'
    if testing:
        print('DAQ was not in GO')
    exit()

    
if dictlist['IBCAD00CRCUR6']['sigma'] == 0 :
    if testing:
        print('Beam current sigma was 0')
    exit()

# at some point the daq was on, and beam current was varying

# the daq status PV get stuck when it crashes
# don't complain about the trigger rates if the daq livetime is zero.
# 

daq_live = True
if dictlist['HD:coda:LiveTime']['mean'] == 0 :
    daq_live = False


# now we have beam on and daq on (and alive) 

trig_zero = False
if dictlist['HD:trig:rate:main']['sigma'] == 0 and dictlist['HD:trig:rate:main']['mean'] > 0 :
    trig_zero = True
    if testing:
        print('trig rate sigma =0 and mean > 0')
        

# here one could do caput to populate epics pv with 0 or 1 -------------


if daq_live and trig_zero:

    time_since_warning = find_time_since_warning(trigfile,timeformat)
    
    if time_since_warning > trig_warning_suppression_time :
        issue_warning('trig')
        record_time(timenow,trigfile,timeformat)
    else :
        if testing:
            print('Trig PV warning issued recently')


