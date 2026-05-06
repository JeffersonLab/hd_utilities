#
# Plan: check that these epics PVs are alive: HD:trig:rate:main, PS:coinc:scaler:rate, PSC:coinc:scaler:rate
# 
# If beam is on and daq is on, collimator is not blocking, and there is a radiator, 
# trigger rate should have nonzero sigma for the last n minutes.
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
from time import sleep

testing = 0

beam_on_current = 10               # minimum nA to consider as beam on
trig_warning_suppression_time = timedelta(minutes=60)   # time between repeated warnings
ps_warning_suppression_time = timedelta(minutes=60)
psc_warning_suppression_time = timedelta(minutes=60)
outage_warning_suppression_time = timedelta(minutes=60)
err_warning_suppression_time = timedelta(minutes=240)

integrationtime_minutes = 5  # length of epics recalled stats  NB this is used for mystats and also for the log message

sleeptime = 60   # if one variable is disconnected, try again in 1 minute

# filenames
epicsfile = '_watch_epics.txt'             # mystats printout

radfile = '_watch_epics_radname.txt'       # myget output for radiator name

psfile = '_watch_epics_ps_warning.txt'     # time of last frozen PS scaler warning
pscfile = '_watch_epics_psc_warning.txt'   # time of last frozen PSC scaler warning
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

daq_wave = "pv=IBCAD00CRCUR6&pv=HD%3Atrig%3Arate%3Amain&pv=HD:coda:LiveTime&pv=PS%3Acoinc%3Ascaler%3Arate&pv=PSC%3Acoinc%3Ascaler%3Arate"
daq_wave += "&IBCAD00CRCUR6color=%23ee0000&HD%3Atrig%3Arate%3Amaincolor=%23000000&HD:coda:LiveTimecolor=%2377bb77&PS%3Acoinc%3Ascaler%3Aratecolor=%239999ee&PSC%3Acoinc%3Ascaler%3Aratecolor=%237777aa"

cdc_wave = "pv=SOL:i:BarPress2&pv=GAS:i::CDC_Temps-CDC_D1_Temp&pv=GAS:i::CDC_Temps-CDC_D2_Temp&pv=GAS:i::CDC_Temps-CDC_D3_Temp&pv=GAS:i::CDC_Temps-CDC_D4_Temp&pv=GAS:i::CDC_Temps-CDC_D5_Temp"
cdc_wave += "&SOL:i:BarPress2color=%23000000&GAS:i::CDC_Temps-CDC_D1_Tempcolor=%23ee0000&GAS:i::CDC_Temps-CDC_D2_Tempcolor=%23ff8c00&GAS:i::CDC_Temps-CDC_D3_Tempcolor=%2333bb77&GAS:i::CDC_Temps-CDC_D4_Tempcolor=%231188ff&GAS:i::CDC_Temps-CDC_D5_Tempcolor=%239977dd"

base_url = "https://epicsweb.jlab.org/wave/?myaLimit=100000&windowMinutes=30&title=&fullscreen=false&layoutMode=1&viewerMode=1"
base_url += "&start=" + wavestart + "&end=" + wavestop + "&"

# viewerMode 1=fixed 2=live
# layoutMode 1=separate charts 3=same chart,separate axes 


#https://epicsweb.jlab.org/wave/?myaDeployment=ops&myaLimit=100000&title=&fullscreen=false&layoutMode=1&viewerMode=2&pv=IBCAD00CRCUR6&pv=HD%3Acoda%3Adaq%3Astatus&RESET%3Ai%3AGasPanelBarPress1yAxisLabel=&RESET%3Ai%3AGasPanelBarPress1yAxisMin=&RESET%3Ai%3AGasPanelBarPress1yAxisMax=&RESET%3Ai%3AGasPanelBarPress1scaler=&RESET%3Ai%3AGasPanelBarPress1yAxisLog=&windowMinutes=1440&start=2022-10-26+16%3A23%3A49&end=2022-10-26+16%3A28%3A49&IBCAD00CRCUR6label=IBCAD00CRCUR6&IBCAD00CRCUR6color=%23a6cee3&IBCAD00CRCUR6yAxisLabel=&IBCAD00CRCUR6yAxisMin=&IBCAD00CRCUR6yAxisMax=&IBCAD00CRCUR6yAxisLog=&IBCAD00CRCUR6scaler=&HD%3Acoda%3Adaq%3Astatuslabel=HD%3Acoda%3Adaq%3Astatus&HD%3Acoda%3Adaq%3Astatuscolor=%231f78b4&HD%3Acoda%3Adaq%3AstatusyAxisLabel=&HD%3Acoda%3Adaq%3AstatusyAxisMin=&HD%3Acoda%3Adaq%3AstatusyAxisMax=&HD%3Acoda%3Adaq%3AstatusyAxisLog=&HD%3Acoda%3Adaq%3Astatusscaler=&pv=HD%3Acoda%3Adaq%3Arun_number&HD%3Acoda%3Adaq%3Arun_numberlabel=HD%3Acoda%3Adaq%3Arun_number&HD%3Acoda%3Adaq%3Arun_numbercolor=%23b2df8a&HD%3Acoda%3Adaq%3Arun_numberyAxisLabel=&HD%3Acoda%3Adaq%3Arun_numberyAxisMin=&HD%3Acoda%3Adaq%3Arun_numberyAxisMax=&HD%3Acoda%3Adaq%3Arun_numberyAxisLog=&HD%3Acoda%3Adaq%3Arun_numberscaler=


#-----------------------------------------------------------------------------------

def run_mystats(epicsfile) :

    # SOL:i:BarPress2 is included for monitoring EPICS blackouts
    
    pvlist = 'IBCAD00CRCUR6,hd:collimator_at_block,hd:radiator_at_home,hd:polarimeter_at_home,hd:converter_at_home,HD:coda:daq:status,HD:coda:LiveTime,HD:trig:rate:main,PS:coinc:scaler:rate,PSC:coinc:scaler:rate,SOL:i:BarPress2,GAS:i::CDC_Temps-CDC_D1_Temp,GAS:i::CDC_Temps-CDC_D2_Temp,GAS:i::CDC_Temps-CDC_D3_Temp,GAS:i::CDC_Temps-CDC_D4_Temp,GAS:i::CDC_Temps-CDC_D5_Temp'

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

def check_goni(radfile) :
    
    # we have to check the radiator name to distinguish between goni-blank and goni-amo as both have radiator-id 0
    # mystats returns N/A for the name, so use myget to see the most recent change

    if os.path.exists(radfile):
        os.remove(radfile)
    
    cmd = '/usr/csite/certified/bin/myget -c HD:GONI:RADIATOR_NAME -t0 > ' + radfile

    os.system(cmd)
    #    print('reading old epics file in check_goni')

    allok = check_output_exists(radfile)

    if not allok:              # create error logentry and assume goni is blank to avoid false alarms
        issue_warning('err')
        return False
    
    f = open(radfile,'r')
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
        title = 'HD:trig:rate:main is zero'
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
    f2.write("Name                          Min           Mean          Max           Sigma\n")

    for line in lines:
        things = line.strip().split(" ")
        #newline = f'{things[0]:22}' + ' '
        newline = '{:<30}'.format(things[0])
        for i in range(1,len(things)):
            #newline = newline + f'{ things[i]:10}'
            newline = newline + '{:14}'.format(things[i])
        f2.write(newline + "\n")

    f2.write("\n")

    f2.write("Goniometer setting:\n\n")        
   
    f = open(radfile,'r')
    lines = f.readlines()
    f.close()

    for line in lines:
        f2.write(line)

    f2.write("\n\n")


    if "CDC" in pv or "SOL" in pv :
        wave_url = base_url + cdc_wave
    else :
        wave_url = base_url + daq_wave
    f2.write("EPICS wave url:\n")
    
    f2.write(wave_url)
    f2.write("\n\n")
    
    import platform

    f2.write("This message was generated by Naomi's watch_epics script, running as a cron job on " + platform.node() + "\n")
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

# if we ever set up new PVs for this, they could be set to
# 0 if the beam on or daq on tests fail
# 1 if beam and daq are on and PV being checked is good
# 0 if beam and daq are on but PV being checked is bad


# check for epics outages. - N/A values will have been set to -1   (HD:GONI:RADIATOR_NAME not used as it always returns N/A if not 'BLANK')

outages = ""

for dict in dictlist:
    if dictlist[dict]['min'] < 0  or dictlist[dict]['mean'] < 0  or dictlist[dict]['sigma'] < 0 :
        outages = outages + dict + " "

# if there is an outage, repeat the above <sleeptime> seconds later, in case the outaage is temporary 
if outages != "" :
    time.sleep(sleeptime)
    run_mystats(epicsfile)
    allok = check_output_exists(epicsfile)   # this issues a warning if the file is not there
    if not allok:
        exit()
    dictlist = read_mystats(epicsfile)   
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
    radiator_in_place = check_goni(radfile)       # set false if blank or retracted, otherwise true


if not radiator_in_place :
    if testing:
        print('no radiator')
    exit()

# now we have beam on and daq on and radiator in place

# the daq status PV get stuck when it crashes
# don't complain about the trigger rates if the daq livetime is zero.

daq_live = True
if dictlist['HD:coda:LiveTime']['min'] == 0 :
    daq_live = False

# now we have beam on and daq on (and alive) and radiator in place

# trig rate is static over longish times May 2026, use a separate process to check sigma with mean>0
# here just check nonzero

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


if daq_live and trig_zero:

    time_since_warning = find_time_since_warning(trigfile,timeformat)
    
    if time_since_warning > trig_warning_suppression_time :
        issue_warning('trig')
        record_time(timenow,trigfile,timeformat)
    else :
        if testing:
            print('Trig PV warning issued recently')

            
if no_converter:
    exit()

            
if daq_live and (ps_frozen or psc_frozen):
            
    time_since_warning = find_time_since_warning(psfile,timeformat)
    
    if time_since_warning > ps_warning_suppression_time :
        if ps_frozen :
            issue_warning('ps')
        else :
            issue_warning('psc')
        record_time(timenow,psfile,timeformat)
    else :
        if testing:
            print('PS PV warning issued recently')


