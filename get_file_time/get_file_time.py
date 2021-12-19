# Usage: python get_file_time.py <evio_file> [<output_file>]
#
# This script attempts to find the time when the first data in the supplied evio file were recorded, using information stored in the supplied file and the first file for the same run, if that is available, or RCDB if it is not.  Using RCDB gives less accurate results. 
#
#
# The prestart event starts a 250 MHz clock which is recorded for each event in DCODAROCInfo. Physics data recording starts after the go event.
#
# Both prestart and go events are usually in file 000.
#
# The run start time is recorded in RCDB a few seconds after the go event. 
#
# The script uses hd_dump to look for prestart and go event times and the timestamp of the first event, all in file 000.
#
# If file 000 is not found, it uses the run start time from RCDB
#
# It then reads the timestamp for the first event in the supplied file, converts the difference in timestamps into difference in seconds, and adds that to the start time.
#
# If an output file is specified as the 2nd command line argument, the prestart time and file time are written to that file, followed by 'RCDB' if RCDB was used to estimate the prestart time. 
#
# Naomi Jarvis 1 Dec 2021


import sys
import os
import subprocess
import glob
import rcdb
import re
import time
from datetime import datetime


def parsefilename(eviofile):
    # deconstruct the evio filename assuming it takes the form of 
    # base_dir/hd_rawdata_run_file.evio or base_dir/hd_rawdata_run_file.skim_name.evio

    loc = eviofile.find('hd_rawdata_') + 18   # start of the file number
    base = eviofile[0:loc]
    filenum = eviofile[loc:loc+3]
    ext = eviofile[loc+3:]
    
    if not filenum.isdigit():
        print('Could not extract the file number from the file name, expected to find hd_rawdata_<run>_<file>')
        eviofile0 = ''
        filenum = 0
    else: 
        eviofile0 = base + '000' + ext
    
    return eviofile0,filenum


def getrunnumber(filename):

    dumpf=open(filename,'r')

    line='start                           '
    while line[:4] != 'Run:' and len(line)>0:
        line=dumpf.readline()
        if not line:
          break

    if len(line) == 0:  # eof
       print('Run number was not found in the file.')
       return 0

    return line[4:len(line)-1]     # exclude the newline

    dumpf.close()


def getprestarttime(filename):

    dumpf=open(filename,'r')

    line='start                           '
    while line[:18] != 'DCODAControlEvent:' and len(line)>0:
        line=dumpf.readline()
        if not line:
          break

    if len(line) == 0:  # eof
       print('DCODAControlEvent was not found in the file.')
       return 0

    dumpf.readline()  #  event_type:  unix_time: Nwords:
    dumpf.readline() #---------------------------------------------------------------
    line=dumpf.readline() #        ffd2  1630144027       5 

    event_type=line.split()[0]
    unixtime=line.split()[1]

#    if event_type != 'ffd2' :
#      print event_type
#      print 'Second control event was not GO!'
#      unixtime=0

    return unixtime

    dumpf.close()



def geteventtime(filename):

    dumpf=open(filename,'r')

    line='start                        '
    while line[:13] != 'DCODAROCInfo:':
        line=dumpf.readline()
        if not line:
            break

    if len(line) == 0:  # eof
       print('DCODAROCInfo was not found in the file.')
       return 0

    dumpf.readline()      #  rocid:    timestamp: Nmisc:
    dumpf.readline()      #-----------------------------
    line=dumpf.readline() #    34  620160547174      0 

    timestamp=line.split()[1]

    return timestamp

    dumpf.close()


#-------------------------------

if not sys.argv[1]:

    exit('Usage: python getfiletime.py <evio_file> [<output_file>]')

eviofile = sys.argv[1]


if len(sys.argv) > 2:
    outputfile = sys.argv[2]
    # Open output file now, so that it is empty if the time is not found
    outf = open(outputfile,'w')
else:
    outf = False


if not os.path.exists(eviofile):
    exit('File not found: '+eviofile)


eviofile0,filenum = parsefilename(eviofile)


if os.path.exists(eviofile0):
    print('The start time for the run will be determined from '+eviofile0)
    usedrcdb = False
else:
    print('The start time for a file can be obtained from the start time for the run plus the difference between the clock times at prestart and at the start of the file. The start time for this run will be estimated from RCDB, because file 000 was not found. The estimated start time for this file will be delayed by the (unknown) interval between the prestart and go events. For a more accurate time, provide file 000. '+eviofile0)
    usedrcdb = True


# dump times from supplied file

times_thisfile='_temp1.txt'    

outputfile=open(times_thisfile,"w")
subprocess.call(["hd_dump", eviofile, "-q3","-DCODAControlEvent", "-DCODAROCInfo"],stdout=outputfile,stderr=subprocess.STDOUT) 
outputfile.close()


# extract run number and first event time and also prestart time if it's file 0.

run = getrunnumber(times_thisfile)


if (filenum == '000'):
    start_unixtime = getprestarttime(times_thisfile)
    #print 'prestart unix time is ',start_unixtime
    if start_unixtime == 0 :
        exit('Could not find the prestart event.')

    dt_runstart = datetime.fromtimestamp(float(start_unixtime))

else:

    if os.path.exists(eviofile0):
        #print 'Looking for time of prestart event'
        # get prestart time 
        times_file0='_temp0.txt'
    
        outputfile=open(times_file0,"w")
        subprocess.call(["hd_dump", eviofile0, "-q3","-DCODAControlEvent", "-DCODAROCInfo"],stdout=outputfile,stderr=subprocess.STDOUT) 
        outputfile.close()

        start_unixtime = getprestarttime(times_file0)
        #        print 'prestart unix time is ',start_unixtime

        if start_unixtime == 0 :
            exit('Could not find the prestart event.')


        dt_runstart = datetime.fromtimestamp(float(start_unixtime))

    else:
        #  get start time from rcdb
        if run == 0 : 
            exit('RCDB cannot provide a start time for run 0.')
        else : 
            db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb")
            intrun = int(run)
            runs = db.select_runs("", intrun, intrun)
    
            dt_runstart = runs[0].start_time
            start_unixtime = time.mktime(dt_runstart.timetuple()) - 2   #subtract 2 seconds as rcdb is always a bit late
            # no idea what to subtract to estimate for the prestart to go interval
    

# find first event time from current file

clocktime = geteventtime(times_thisfile)

if clocktime == 0 :
    exit('Could not find any DCODAROCInfo from which to obtain a timestamp.')
  

timediff = float(clocktime)/2.5e8

thisfile_timestamp = float(start_unixtime) + timediff

dt_thisfile = datetime.fromtimestamp(thisfile_timestamp)

print('prestart      '+dt_runstart.strftime("%Y-%m-%d %H:%M:%S"))
print('start of file '+dt_thisfile.strftime("%Y-%m-%d %H:%M:%S"))

if outf:
    outf.write(dt_thisfile.strftime("%Y-%m-%d %H:%M:%S")+'\n')
    if usedrcdb:
        outf.write('RCDB\n')
    outf.close()




