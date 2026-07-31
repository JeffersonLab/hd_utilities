#
# watch_mya.py checks Hall D rates in mya when there is beam, a radiator and the daq in go.
#
# Usage:
#        python watch_mya.py    
#        python watch_mya.py -t "2025-01-01 10:00:00"
#        python watch_mya.py -f myepicsfile.txt 
#
# Checks that HD:trig:rate:main, PS:coinc:scaler:rate, PSC:coinc:scaler:rate and TAGH and TAGM have sigma > 0
# if the expected sigma (obtained from beam current sigma) is larger than the deadband.
# Skips the PS & PSC checks if the PS and TPOL converters are both retracted.
#
# Checks that CDC temperatures and pressure are not disconnected
#
# Makes log entries warning about disconnections and frozen PVs.
# Records the warning times, so that over-frequent warnings can be suppressed.
#
# This is written for python 2.7
# To modify for python 3, the subprocess calls will need to be updated

# Naomi Jarvis, June 2026


import sys
import os
import json
import subprocess
from datetime import datetime,timedelta
from time import sleep

TESTING = False
LOGBOOK = "HDLOG"
    
MIN_BEAM_CURRENT = 10      # minimum nA to consider as beam on

INTEGRATION_TIME_MINS = 5  # integration time for mystats
DISCONNECT_TOLERANCE_MINS = 10 # complain about PVs disconnected for longer than this
SUPPRESS_WARNINGS_MINS = 60   # time between repeated identical warnings

DATE_FORMAT = "%Y-%m-%d %H:%M:%S"

MSG_RESTART_TRIG = ("Instructions to restart the trig_client monitoring process are in "
                    "https://halldweb.jlab.org/hdops/wiki/index.php/Level-1_Trigger_Shift_PrimEx#Trigger_Monitor \n\n")

MSG_RESTART_PS = ("Instructions to restart the ps_client monitoring process are in "
                  "https://halldweb.jlab.org/hdops/wiki/index.php/Pair_Spectrometer_Shift#Run_PS_Scalers \n\n")

MSG_RESTART_TAGH = ("Please check the scalers in CSS. "
                    "If they have frozen, try to fix this between runs "
                    "by power-cycling the TAGH2 crate (ROCID 74) and then restarting the DAQ.\n\n")

MSG_RESTART_TAGM = ("Please check the scalers in CSS. "
                    "If they have frozen, try to fix this between runs "
                    "by power-cycling the TAGM2 crate (ROCID 72) and then restarting the DAQ.\n\n")

#-----------------------------------------------------------------------------------

def run_mystats(epicsfile, time_now, mya_deployment) :
    
    if os.path.exists(epicsfile):
        os.remove(epicsfile)

    tstart = datetime.strptime(time_now,DATE_FORMAT) - timedelta(minutes=INTEGRATION_TIME_MINS)
    tstart = datetime.strftime(tstart,DATE_FORMAT)
     
    if TESTING:
        print('Getting epics from ' + tstart + ' to ' + time_now)
    
    cmd_arr = ["myStats", "-m", mya_deployment, "-b", tstart, "-e", time_now, "-u", "-f", epicsfile, "-l", "@pv_list.txt"]

    try:
        subprocess.check_call(cmd_arr)

    except subprocess.CalledProcessError as e:
        print("myStats command failed with return code:" + str(e.returncode))
        if (e.output):
            print(e.output)

        return False
    
    return True

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

def check_disconnect_times(pvlist, time_now, DISCONNECT_TOLERANCE_MINS, mya_deployment):

    lines = [] # list of disconnect times
    pvs = []

    for pv in pvlist :
        cmd_arr = ["myget", "-m", mya_deployment, "-c", pv, "-t", time_now, "-w-"]

        try:
            result = subprocess.check_output(cmd_arr)

        except subprocess.CalledProcessError as e:
            if e.returncode == 1:
                # no events found for this pv (not filled or not archived)
                if TESTING:
                    print('myget returned 1 (unknown) for '+pv)
                continue
                
            else:
                print("myget command failed with return code:" + str(e.returncode))
                if e.output:
                    print(e.output)
                return False

        if "Network disconnection" in result:
            time_since = datetime.strptime(time_now,DATE_FORMAT) - datetime.strptime(result.split(" <<")[0],DATE_FORMAT)   #this is a timedelta
            #print('time_since: ' + str(time_since))
            #print('limit:' + str(timedelta(minutes=DISCONNECT_TOLERANCE_MINS)))
                  
            if time_since > timedelta(minutes=DISCONNECT_TOLERANCE_MINS) :
                  lines.append(pv + " " + result)
                  pvs.append(pv)
                  
    return [pvs, lines]

#-----------------------------------------------------------------------------------

def check_goni(time_now, mya_deployment) :
    
    # we have to check the radiator name to distinguish between goni-blank and goni-amo as both have radiator-id 0
    # mystats returns N/A for the name, so use myget to see the most recent change

    cmd_arr = ["myget", "-m", mya_deployment, "-c", "HD:GONI:RADIATOR_NAME", "-t", time_now, "-w-"]

    try:
        result = subprocess.check_output(cmd_arr)

    except subprocess.CalledProcessError as e:
        print("myget command failed with return code: " + str(e.returncode))
        if e.output:
            print(e.output)
        return [False, False]

    if 'BLANK' in result or 'RETRACTED' in result or 'Network disconnection' in result:
        return [False, False]
    else :
        return [True, result]
    
#-----------------------------------------------------------------------------------

def find_deadband(pvname='IBCAD00CRCUR6', meanval=1) :
    
    # obtain mya deadband expression from archive, return value, return False if an error occurs

    try:
        result = subprocess.check_output(["archive", pvname])

    except subprocess.CalledProcessError as e:
        print("archive command failed with return code:" + str(e.returncode))
        if e.output:
            print(e.output)
        return False
    
    if not 'deadband' in result:
        if TESTING:
            print('no deadband defined')
        return 0

    dbtext = result.split('{')[0].split('deadband=')[1].strip(" '")
    
    #if TESTING:
    #    print('deadband text:',dbtext)
    # dbtext should be like '0.3' or '=$sqrt($abs(x))+0.005*$abs(x)'

    if not '=' in dbtext:
        return float(dbtext.strip("'"))

    dbtext = dbtext.replace('$','')
    dbtext = dbtext.replace('=','')

    from math import sqrt
    dbtext = dbtext.replace('sqrt(abs(x))',str(sqrt(abs(meanval))))
    dbtext = dbtext.replace('abs(x)',str(abs(meanval)))    

    # make sure that the deadband string does not contain letters/functions

    safe = True
    for i in range(0,len(dbtext)):
        if dbtext[i].isalpha():
            safe = False
    
    if safe : 
        try:
            dbval = eval(dbtext)
            return dbval
        except :
            print('Cannot parse deadband function '+dbtext)
            return False
    else :
        print('Cannot parse deadband function '+dbtext)
        return False

#-----------------------------------------------------------------------------------

def check_tagger_scalers(taggername, dictlist, frac_change):

    # Unfortunately python2 dicts cannot be ordered 
    
    leaders = [] # first pv in each group of 8 frozen
    
    # count instances of sigma=0 - need 8+

    n = 0
    for dict in dictlist:
        if dict[:4] == taggername and dictlist[dict]['sigma'] == 0:
            n += 1

    if n < 8:
        return leaders

    ids_unordered = []  # counter id extracted from pv
    
    for dict in dictlist:
        
        mean = dictlist[dict]['mean']
        sigma = dictlist[dict]['sigma']
        
        if dict[:4] == taggername and mean > 0 and sigma == 0:
            db = find_deadband(dict, mean)
            if TESTING:
                print(dict + ' frac_change x mean: ' +  str(round(frac_change*mean, 0)) + ' db: ' + str(db))
            if (db) :
                if frac_change*mean > 2*db: 
                    id = int(dict.split(":")[2])
                    ids_unordered.append(id)
                    
    if len(ids_unordered) == 0:
        return leaders

    ids = sorted(ids_unordered)

    if TESTING:
        print(ids)
    

    
    # look for 8+ consecutive ids
    n_consec = 0
    first = ""
    prev = -999
    
    for i in range(0, len(ids)):
        if ids[i] == prev+1: 
            n_consec +=1
            if n_consec == 7:
                leaders.append(taggername + ":T:" + str(first) + ":scaler_t1")
        else:
            n_consec = 0
            first = ids[i]
        prev = ids[i]

    return leaders


#-----------------------------------------------------------------------------------

def warning_needed(err, time_now, logfile, suppression_timedelta) :

    # read previous times from logfile
    warnings = {}
     
    if os.path.exists(logfile):
        with open(logfile,"r") as wfile:
            warnings = json.load(wfile)

    do_warn = True

    if TESTING:
        print('Warning for ' + err)
    
    if err in warnings :
        datetime_now = datetime.strptime(time_now, DATE_FORMAT)
        datetime_then = datetime.strptime(warnings[err], DATE_FORMAT)
        if TESTING:
            print('Last warned at ' + warnings[err])
            #print('compare times, now:', datetime_now , 'time of last warning:',warnings[err], 'warning suppression expires:',datetime_then + suppression_timedelta)
        if datetime_now < datetime_then + suppression_timedelta :
            do_warn = False
            if TESTING:
                print('Warning suppressed')
        '''
        if err == 'disconnect' :
            print(pvs,warnings['pvs'])
            if pvs != warnings['pvs'] :
                do_warn = True
        '''

    if do_warn:
        warnings[err] = time_now
        with open(logfile,"w") as wfile:
            json.dump(warnings, wfile)
        
    return do_warn


#-----------------------------------------------------------------------------------

def make_wave_url(pvs, time_now, mya_deployment):


    # viewerMode 1=fixed 2=live
    # layoutMode 1=separate charts 3=same chart,separate axes 

    base_url = "https://epicsweb.jlab.org/wave/?myaDeployment=" + mya_deployment + "&myaLimit=100000&windowMinutes=30&title=&fullscreen=false&layoutMode=1&viewerMode=1"

    wavetimeformat = "%Y-%m-%dT%H:%M:%S"
    datetime_now = datetime.strptime(time_now, DATE_FORMAT)
    wavestart = datetime.strftime(datetime_now - timedelta(minutes=2*INTEGRATION_TIME_MINS), wavetimeformat) 
    wavestop = datetime.strftime(datetime_now + timedelta(minutes=INTEGRATION_TIME_MINS), wavetimeformat) 

    url = "&start=" + wavestart + "&end=" + wavestop

    colours = ['%23ee0000','%2377bb77', '%23000000', '%239999ee', '%237777aa']

    count = 0
    for pv in pvs:
        count +=1
        url += "&pv=" + pv

        if count <= len(colours):
            url += "&" + pv + "color=" + colours[count-1]

    url = base_url + url.replace(":","%3A")

            
    return url
    
    # https://epicsweb.jlab.org/wave/?myaDeployment=ops&myaLimit=100000&windowMinutes=30&title=&fullscreen=false&layoutMode=1&viewerMode=1&start=2026-05-18T05%3A33%3A00&end=2026-05-18T05%3A48%3A00&pv=IBCAD00CRCUR6&IBCAD00CRCUR6color=%23ee0000&pv=HD%3Acoda%3ALiveTime&HD%3Acoda%3ALiveTimecolor=%2377bb77&pv=TAGH%3AT%3A145%3Ascaler_t1&TAGH%3AT%3A145%3Ascaler_t1color=%23000000&pv=TAGH%3AT%3A225%3Ascaler_t1&TAGH%3AT%3A225%3Ascaler_t1color=%239999ee


#-----------------------------------------------------------------------------------

def issue_warnings(title, message, time_now, epicsfile) :
    
    if message == "" :
        return

    time_start = datetime.strptime(time_now,DATE_FORMAT) - timedelta(minutes=INTEGRATION_TIME_MINS)
    time_start = datetime.strftime(time_start,DATE_FORMAT)
    
    import platform
    
    f = open("_logmsg.txt","w")
    f.write("Date: " + time_now)
    f.write("\n")
    f.write(message)
    f.write("\n")
    f.write("This message was generated by Naomi's watch_mya script, running as a cron job on " + platform.node() + "\n\n")

    if os.path.exists(epicsfile):
        f.write("MyStats data from " + time_start + " to " + time_now + " follow.\n\n")

        with open(epicsfile,"r") as f2:
            for line in f2:
                f.write(line)

    f.close()
    
    if TESTING:
        print("\nLogentries are disabled. To see the full message, cat _logmsg.txt")
        print('Title: '+title)
 
        with open("_logmsg.txt","r") as f:
            for line in f:
                if "follow" in line:
                    break
                print(line)

    else :

        # See https://logbooks.jlab.org/content/api-authentication
        # Need this set: export JAVA_HOME=/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/
    
        os.putenv('JAVA_HOME','/gapps/Java/jdk/23.0.1/x64/jdk23.0.1/')  # needed to make the log entry
        cmd_arr = ["/site/ace/certified/apps/bin/logentry", "-l", LOGBOOK, "-g", "Autolog", "-t", title, "-b", "_logmsg.txt", "-n", "njarvis@jlab.org"]

        try:
            subprocess.check_call(cmd_arr)
        except subprocess.CalledProcessError as e:
            print("Logentry command failed with return code:", e.returncode)
        
    return

#-----------------------------------------------------------------------------------
#-- main ---------------------------------------------------------------------------

def main() :

    if MIN_BEAM_CURRENT < 10:
        print('Minimum beam current required is ' + str(MIN_BEAM_CURRENT))
    
    # filenames
    epicsfile = '_epics.txt'             # mystats printout
    radfile = '_radname.txt'       # myget output for radiator name
    warningfile = '_warningtimes.json'
    
    time_now = datetime.now().strftime(DATE_FORMAT)
    
    suppression_timedelta = timedelta(minutes=SUPPRESS_WARNINGS_MINS)
    
    # watch_mya.py -t "2025-01-01 10:00:00 or watch_mya.py -f myepicsfile.txt or watch_mya.py

    mya_deployment = "ops"    
    override_mystats_file = ""
    
    while len(sys.argv) > 0 :
        x = sys.argv.pop(0)
        
        if x == "-f" :
            TESTING = True
            override_mystats_file = sys.argv.pop(0)
            if not os.path.exists(override_mystats_file) :
                exit('File not found: ' + override_mystats_file)
                    
        elif x == "-t" :
            TESTING = True
            time_now = sys.argv.pop(0)
            try:
                datetime.strptime(time_now, DATE_FORMAT)
                print('Running as if at ' + time_now)
    
                time_diff = datetime.now() - datetime.strptime(time_now, DATE_FORMAT)

                if time_diff > timedelta(days=365) :
                    mya_deployment = "history"

            except ValueError:
                exit('Invalid date format')
                
    logbook_title = ""
    logbook_message = ""
    wave_pvs = ['IBCAD00CRCUR6', 'HD:coda:LiveTime']

    # run mystats unless override file provided
    if override_mystats_file == "" :
        ok = run_mystats(epicsfile, time_now, mya_deployment)
        if not ok:
            if warning_needed('myStats', time_now, warningfile, suppression_timedelta) :    
                issue_warnings('watch_mya cron: myStats failed', 'myStats command failed, cron script exited.', time_now, epicsfile)
            exit()
    else:
        epicsfile = override_mystats_file

    dictlist = read_mystats(epicsfile)   
    
    # the dictlist is a list of dicts, eg {'IBCAD00CRCUR6': {'mean': 0.275, 'sigma': 0.641775}}
    # invalid mean or sigma, eg 'N/A', are set to -1
    # beam current doesn't experience disconnects often. This happened on May 25 2026 at 8:15am
    
    if dictlist['IBCAD00CRCUR6']['mean'] < MIN_BEAM_CURRENT :
        if TESTING:
            print('Mean beam current ' + str(dictlist['IBCAD00CRCUR6']['mean']))
        exit()
    
    
    # check for epics outages. - N/A values will have been set to -1 
    # Stockpile the logbook messages until exiting
    
    list_of_disconnects = []
    
    for dict in dictlist:
        if dictlist[dict]['min'] < 0  or dictlist[dict]['mean'] < 0  or dictlist[dict]['sigma'] < 0 :
            list_of_disconnects.append(dict)

    # this returns [pvs, lines] or False if there was an error
    result = check_disconnect_times(list_of_disconnects, time_now, DISCONNECT_TOLERANCE_MINS, mya_deployment)

    if result == False:
        if warning_needed('myget', time_now, warningfile, suppression_timedelta) :    
            issue_warnings('watch_mya cron: myget failed', 'myget command failed, cron script exited.', time_now, epicsfile)
        exit()

    pvs = result[0]
    lines = result[1]
    
    if len(pvs) > 0 :    
        if TESTING:
            print('Disconnected: ' + str(pvs))
                
        if warning_needed('disconnect', time_now, warningfile, suppression_timedelta) :
            wave_pvs.extend(pvs)
            logbook_title = "Some EPICS PVs disconnected from MYA. "
            logbook_message += "The following PVs have been disconnected from MYA for at least " + str(DISCONNECT_TOLERANCE_MINS) + " minutes:\n"
            for line in lines :
                logbook_message += line + "\n"
            logbook_message += "\n"
    
    # Continue only if it looks like we are taking data and DAQ livetime > 0
    # HD:coda:daq:status = 2 for 'go'
    # The daq status PV get stuck when it crashes

    if dictlist['HD:coda:daq:status']['mean'] != 2 or dictlist['hd:collimator_at_block']['mean'] != 0 or dictlist['HD:coda:LiveTime']['min'] == 0 :
        if TESTING:
            print('Collimator status: ' + str(dictlist['hd:collimator_at_block']['mean']))
            print('DAQ status: ' + str(dictlist['HD:coda:daq:status']['mean']))
            print('Mean beam current: ' +  str(dictlist['IBCAD00CRCUR6']['mean']))
            print('Mean DAQ livetime: ' + str(dictlist['HD:coda:LiveTime']['min']))
    
        # inform about any long disconnects before exiting    
        issue_warnings(logbook_title, logbook_message, time_now, epicsfile)
        exit()
    
    
    # Find out if a radiator is in place.  amo ladder is retracted if hd:radiator_at_home=1
    # HD:GONI:RADIATOR_NAME was not included in earlier myStats call because it returns N/A if not 'BLANK'
    
    radiator_in_place = False
    radiator_name = ""
    
    if dictlist['hd:radiator_at_home']['min'] == 0 :    # using amo ladder at least part of the time
        radiator_in_place = True
    else : 
        radiator_in_place, radiator_name = check_goni(time_now, mya_deployment)       # set false if blank or retracted, otherwise true
        
    if not radiator_in_place :
        if TESTING:
            print('no radiator')
            
        # inform about any disconnects before exiting            
        issue_warnings(logbook_title, logbook_message, time_now, epicsfile)
        exit()
    
    # beam current sigma should not be 0.  Need it for later calcs.
        
    if dictlist['IBCAD00CRCUR6']['sigma'] == 0 :
        if TESTING:
            print('beam current sigma = 0')
            
        # inform about any disconnects before exiting            
        issue_warnings(logbook_title, logbook_message, time_now, epicsfile)
        exit()
    
    # now we have beam on and daq on (and alive) and radiator in place
    
    # calculate expected sigma for pvs with sigma=0 from beam current    
    frac_change = dictlist['IBCAD00CRCUR6']['sigma']/dictlist['IBCAD00CRCUR6']['mean']

    pvname = 'HD:trig:rate:main'
    new_message = ""
    if dictlist[pvname]['mean'] == 0 :
        if TESTING:
            print('trigger rate mean = 0')
            
        if warning_needed('trigrate_0', time_now, warningfile, suppression_timedelta) :
            wave_pvs.append(pvname)        
            logbook_title = "The main trigger rate is zero in MYA. " + logbook_title
            new_message = "The main trigger rate is zero in MYA.\n"
    
    elif dictlist[pvname]['sigma'] == 0 :
        if TESTING:
            print('trigger rate sigma = 0')

        db = find_deadband(pvname,dictlist[pvname]['mean'])
        if (db) :
            if TESTING:
                print('db: ' + str(db) + " frac_change*mean: " + str(frac_change*dictlist[pvname]['mean']))
                
            if frac_change*dictlist[pvname]['mean'] > 2*db:
                if warning_needed('trigrate_frozen', time_now, warningfile, suppression_timedelta) :
                    wave_pvs.append(pvname)
                    logbook_title = "The main trigger rate looks frozen in MYA. " + logbook_title
                    new_message = "The main trigger rate looks frozen in MYA.\n"

    if new_message != "":
        new_message += MSG_RESTART_TRIG
        logbook_message = new_message + logbook_message
    
        
    # PS & PSC - check converter is in place - use either PS converter or more often the TPOL converter. At home means not in use.
    if dictlist['hd:converter_at_home']['mean'] == 0 or dictlist['hd:polarimeter_at_home']['mean'] == 0 :
        new_message = ""
        
        for pv in ['PS', 'PSC'] :
            pvname = pv + ':coinc:scaler:rate'

            if dictlist[pvname]['mean'] == 0 :
                if warning_needed(pv+'coincrate_0', time_now, warningfile, suppression_timedelta) :
                    if TESTING:
                        print(pv + 'rate mean = 0')                    
                    wave_pvs.append(pvname)        
                    logbook_title += pv + " coincidence rate is zero in MYA. " 
                    new_message += pv + " coincidence rate is zero in MYA.\n"

            elif dictlist[pvname]['sigma'] == 0 :
                db = find_deadband(pvname,dictlist[pvname]['mean'])
                if (db) :
                    if TESTING:
                        print('db: ' + str(db) + " frac_change*mean: " + str(frac_change*dictlist[pvname]['mean']))                                                
                    if frac_change*dictlist[pvname]['mean'] > 2*db:
                        if TESTING:
                            print(pv + 'coinc rate frozen')
                        if warning_needed(pv+'coincrate_frozen', time_now, warningfile, suppression_timedelta) :
                            wave_pvs.append(pvname)
                            logbook_title += "The " + pv + " coincidence rate looks frozen in MYA. "
                            new_message += "The " + pv + " coincidence rate looks frozen in MYA.\n"
    
        if new_message != "" :
            new_message += MSG_RESTART_PS
            logbook_message = new_message + logbook_message

            
    # TAGH - look for 8+ consecutive 0s in tagger rates, eg 2026-05-18 05:45:00
    tagh_leaders = check_tagger_scalers("TAGH", dictlist, frac_change)
    
    if len(tagh_leaders) > 0 :
        if warning_needed('tagh_frozen', time_now, warningfile, suppression_timedelta) :
            logbook_title += "Some TAGH scalers look frozen in MYA. "

            if len(tagh_leaders) == 1:
                new_message = "The rates reported by a group of 8 or more TAGH scalers appear to have frozen.\n"
                new_message += "The group of channels starts with " + tagh_leaders[0] + "\n"
                wave_pvs.append(tagh_leaders[0])
            else:
                new_message = "The rates reported by groups of 8 or more TAGH scalers appear to have frozen.\n"
                new_message += "The groups of channels start with: \n" 
                for tagh in tagh_leaders:
                    new_message += tagh + "\n"
                    wave_pvs.append(tagh)

            new_message += MSG_RESTART_TAGH
            logbook_message += new_message

    
    # TAGM - look for 8+ consecutive 0s in tagger rates, eg 2018-01-27 21:15:00    
    tagm_leaders = check_tagger_scalers("TAGM", dictlist, frac_change)
    
    if len(tagm_leaders) > 0 :
        if warning_needed('tagm_frozen', time_now, warningfile, suppression_timedelta) :
            logbook_title += "Some TAGM scalers look frozen in MYA. "
            if len(tagm_leaders) == 1: 
                new_message = "The rates reported by a group of 8 or more TAGM scalers appear to have frozen.\n"
                new_message += "The group of channels starts with " + tagm_leaders[0] + "\n"
                wave_pvs.append(tagm_leaders[0])            
            else:
                new_message = "The rates reported by groups of 8 or more TAGM scalers appear to have frozen.\n "
                new_message += "The groups of channels start with:\n" 
                for tagm in tagm_leaders:
                    new_message += tagm + "\n"
                    wave_pvs.append(tagm)
                    
            new_message += MSG_RESTART_TAGM
            logbook_message += new_message
    

    if len(wave_pvs) > 2:
        wave_url = make_wave_url(wave_pvs, time_now, mya_deployment)
        logbook_message += "EPICS wave link: \n" + wave_url + "\n"
    
    issue_warnings(logbook_title, logbook_message, time_now, epicsfile)
    exit()


#-----------------------------------------------------------------------------------
    
if __name__ == '__main__':
    main()
