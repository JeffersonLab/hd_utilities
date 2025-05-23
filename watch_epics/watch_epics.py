#
# Plan: check that these epics PVs are alive: HD:trig:rate:main, PS:coinc:scaler:rate, PSC:coinc:scaler:rate
# 
# If beam is on and daq is on, collimator is not blocking, and there is a radiator, 
# trigger rate should have nonzero sigma for the last minute.
# If the PS or TPOL converter is in place, PS rates should also have nonzero sigma.
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

beam_on_current = 10               # minimum nA to consider as beam on
trig_warning_suppression_time = timedelta(minutes=60)   # time between repeated warnings
ps_warning_suppression_time = timedelta(minutes=60)
psc_warning_suppression_time = timedelta(minutes=60)
err_warning_suppression_time = timedelta(minutes=60)


#-----------------------------------------------------------------------------------

def run_mystats(epicsfile) :

    # SOL:i:BarPress2 is included for monitoring EPICS blackouts
    
    pvlist = 'IBCAD00CRCUR6,hd:collimator_at_block,hd:radiator_at_home,hd:polarimeter_at_home,hd:converter_at_home,HD:coda:daq:status,HD:trig:rate:main,PS:coinc:scaler:rate,PSC:coinc:scaler:rate,SOL:i:BarPress2'

    if os.path.exists(epicsfile):
        os.remove(epicsfile)

    cmd = "myStats -b -1m -e 0 -u -f " + epicsfile + " -l " + pvlist

    os.system(cmd)
    
    return

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

        if things[4] == 'N/A' :
            sigma = -1
        elif things[4] == '<undefined>' :
            sigma = -1
        else :
            sigma = float(things[4])        

        dict = {pvname : { "min":min, "mean":mean, "sigma":sigma}}

        dictlist.update(dict)

    return dictlist

#-----------------------------------------------------------------------------------

def check_goni(epicsfile) :
    
    # we have to check the radiator name to distinguish between goni-blank and goni-amo as both have radiator-id 0
    # mystats returns N/A for the name, so use myget to see the most recent change

    if os.path.exists(epicsfile):
        os.remove(epicsfile)
    
    cmd = '/usr/csite/certified/bin/myget -c HD:GONI:RADIATOR_NAME -t0 > ' + epicsfile

    os.system(cmd)
#    print('reading old epics file in check_goni')

    if not os.path.exists(epicsfile):     # create error logentry and assume goni is blank to avoid false alarms
        issue_warning('err')
        return False
    
    f = open(epicsfile,'r')
    line = f.readline()
    f.close()

    if 'BLANK' in line or 'RETRACTED' in line:
        return False
    else :
        return True

    
#-----------------------------------------------------------------------------------    


def issue_warning(pv) :

    # See https://logbooks.jlab.org/content/api-authentication
    # Need this set: export JAVA_HOME=/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/
    
    os.putenv('JAVA_HOME','/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/')  # needed to make the log entry
    
    if pv == 'trig':
        title = 'HD:trig:rate:main looks frozen'
        body = 'msg_restart_trigger_client.txt'
    elif pv == 'ps' :
        title = 'PS:coinc:scaler:rate looks frozen'
        body = 'msg_restart_ps_client.txt'        
    elif pv == 'psc' :
        title = 'PSC:coinc:scaler:rate looks frozen'
        body = 'msg_restart_ps_client.txt'
    elif pv == 'err' :
        title = 'Cron script could not access EPICS archive'
        body = 'msg_err_mystats.txt'
    else :
        title = 'EPICS outage'
        f = open('msg_outage.txt','w')
        f.write("EPICS is not available for the following, please check the IOCs:\n")
        f.write(pv)
        f.write("\n")
        f.close()
        body = 'msg_outage.txt'
        
        
    cmd = '/site/ace/certified/apps/bin/logentry -l HDLOG -g Autolog -t "' + title + '" -b ' + body + ' -n njarvis@jlab.org'

#    print(cmd)
#    print('logbook entry deactivated')
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


testing = 0   #prints diagnostics

# filenames
epicsfile = '_watch_epics.txt'             # mystats printout
epicsfile2 = '_watch_epics_radname.txt'       # myget output for radiator name
psfile = '_watch_epics_ps_warning.txt'     # time of last frozen PS scaler warning
pscfile = '_watch_epics_psc_warning.txt'   # time of last frozen PSC scaler warning
trigfile = '_watch_epics_trig_warning.txt' # time of last frozen trig-rate warning
errfile = '_watch_epics_err.txt'           # time of last mystats error warning

timeformat = "%Y-%m-%d %H:%M:%S"
timenow = datetime.now()



run_mystats(epicsfile)
#print('reading old epics file')

if not os.path.exists(epicsfile):

    time_since_warning = find_time_since_warning(errfile,timeformat)
    
    if time_since_warning > err_warning_suppression_time :
        issue_warning('err')
        record_time(timenow,errfile,timeformat)
    else :
        if testing:
            print('Mystats error warning issued recently')
    exit()

    
dictlist = read_mystats(epicsfile)   

# the dictlist is a list of dicts like 
# eg {'IBCAD00CRCUR6': {'mean': 0.275, 'sigma': 0.641775}}
# invalid mean or sigma, eg 'N/A', are set to -1

# if we ever set up new PVs for this, they could be set to
# 0 if the beam on or daq on tests fail
# 1 if beam and daq are on and PV being checked is good
# 0 if beam and daq are on but PV being checked is bad


# check for epics outages. - N/A values will have been set to -1, but the RADIATOR_NAME always returns N/A if not 'BLANK'

outages = ""

for dict in dictlist:
    if dictlist[dict]['min'] < 0  or dictlist[dict]['mean'] < 0  or dictlist[dict]['sigma'] < 0 :
        outages = outages + dict + " "

if outages != "" :
    time_since_warning = find_time_since_warning(errfile,timeformat)
    
    if time_since_warning > err_warning_suppression_time :
        issue_warning(outages)
        record_time(timenow,errfile,timeformat)
    else :
        if testing:
            print('EPICS outage warning issued recently')
    exit()

# no outages were found, so check other data

if dictlist['HD:coda:daq:status']['mean'] != 2 :   # daq status - HD:coda:daq:status = 2 for 'go'
    if testing:
        print('DAQ is not running')
    exit()

    
if dictlist['IBCAD00CRCUR6']['mean'] < beam_on_current :
    if testing:
        print('Mean beam current is < ' + str(beam_on_current) + 'nA')
    exit()

    
if dictlist['hd:collimator_at_block']['mean'] == 1 :
    if testing:
        print('Collimator is blocking')
    exit()


# find out if a radiator is in place.
# amo ladder is retracted if hd:radiator_at_home=1

radiator_in_place = False
if dictlist['hd:radiator_at_home']['min'] == 0 :    # using amo ladder at least part of the time
    radiator_in_place = True
else : 
    radiator_in_place = check_goni(epicsfile2)       # set false if blank or retracted, otherwise true


if not radiator_in_place :
    if testing:
        print('no radiator')
    exit()

# now we have beam on and daq on and radiator in place
    
trig_zero = False
if dictlist['HD:trig:rate:main']['mean'] == 0 :
    trig_zero = True

ps_frozen = False    
if dictlist['PS:coinc:scaler:rate']['sigma'] == 0 :
    ps_frozen = True

psc_frozen = False    
if dictlist['PSC:coinc:scaler:rate']['sigma'] == 0 :
    psc_frozen = True

    
# check converter is in place - use either PS converter or more often the TPOL converter.  At home means not in use.
no_converter = False
if dictlist['hd:converter_at_home']['mean'] == 1 and dictlist['hd:polarimeter_at_home']['mean'] == 1 :
    no_converter = True
    

# here one could do caput to populate epics pv with 0 or 1 -------------


if trig_zero:

    time_since_warning = find_time_since_warning(trigfile,timeformat)
    
    if time_since_warning > trig_warning_suppression_time :
        issue_warning('trig')
        record_time(timenow,trigfile,timeformat)
    else :
        if testing:
            print('Trig PV warning issued recently')


if ps_frozen and not no_converter:

    time_since_warning = find_time_since_warning(psfile,timeformat)
    
    if time_since_warning > ps_warning_suppression_time :
        issue_warning('ps')
        record_time(timenow,psfile,timeformat)
    else :
        if testing:
            print('PS PV warning issued recently')


if psc_frozen and not no_converter:

    time_since_warning = find_time_since_warning(pscfile,timeformat)
    
    if time_since_warning > psc_warning_suppression_time :
        issue_warning('psc')
        record_time(timenow,pscfile,timeformat)
    else :
        if testing:
            print('PSC PV warning issued recently')
        

