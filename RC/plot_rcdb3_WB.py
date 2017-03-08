"""
rplot_rcdb3_WB.py

python script to plot information in the rcdb, especially integrated triggers vs run number, or time

based on Elton Smith's and Mark Daltons scripts. 

"""

import rcdb
from rcdb.provider import RCDBProvider
import sys
import numpy as np
from matplotlib import pyplot as plt
from datetime import datetime, date, time, timedelta
from matplotlib.backends.backend_pdf import PdfPages

import argparse as AG


def datetime_from_string(s):
    # date time format yyyy-mm-dd"
    f = np.array(s.split('-')).astype(int)
    return datetime( *f )

# get db
db = RCDBProvider("mysql://rcdb@hallddb/rcdb")

#--------------------------------------------------------- general parameters -------------
# default run numbers

# run1_def = 10331  # spring 2016
# run2_def = 13000

# run1_def = 21968  # fall 2016
# run2_def = 22167

#run1 = 30000  # spring 2017
run1_def = 30278  # spring 2017
run2_def = 31000

start_date_def = '2017-2-1'
end_date_def = '2017-3-22'

#--------------------------------------------------------- general parameters -------------




parser = AG.ArgumentParser()

parser.add_argument('-s', '--start_run', type = int,  help="first run number", default = run1_def)
parser.add_argument('-e', '--end_run', type = int, help="last run number ", default = run2_def)
parser.add_argument('-m', '--min_current', type = int, help="minimal current ", default = 2)
parser.add_argument('-S', '--exp_start',type = str, help="date of exp.start (YYY-mm-dd)", default =  start_date_def)
parser.add_argument('-E', '--exp_end', type = str, help="date of exp_end (YYY-mm-dd)", default = end_date_def)

args = parser.parse_args()

run1 = args.start_run
run2 = args.end_run

min_current = args.min_current

date_exp_start = datetime_from_string(args.exp_start)
date_exp_end = datetime_from_string(args.exp_end)


# make sure the run2 is larger than run1
if run2 < run1 :
    smaller = run2
    run2 = run1
    run1 = smaller

xmin = run1
xmax = run2



runs = db.select_runs("@is_production and event_count > 1000000",run1,run2)

if (len(runs) == 0):
    sys.exit("No runs found !")
# default start time
date_run1 = runs[0].start_time

if (len(runs) == 0):
    print "No runs in RCDB between",run1," and ",run2
    sys.exit()
else:
    print "Selected ",len(runs)," runs from ",runs[0]," to",runs[len(runs)-1]

# All conditions of this run by name.
# conditions_by_name = runs[0].get_conditions_by_name()
# conditions = conditions_by_name.keys()
# print "List of conditions=",conditions


exp_duration = date_exp_end - date_exp_start
exp_duration_days = exp_duration.total_seconds()/86400.


select_data = None
init_select_data = True

# loop over the runs and put the interestong data into a dictionary of numpy arrays

for i,run in enumerate(runs):
    conditions_by_name = run.get_conditions_by_name()
    conditions = conditions_by_name.keys()
    if init_select_data:
        # setup data array dictionary
        select_data = {}
        for c in conditions:
            select_data[c] = []
        select_data['run_number'] = []
        select_data['elapsed_time'] = []
        select_data['elapsed_seconds'] = []
        select_data['elapsed_days'] = []
        select_data['elapsed_days_exp_start'] = []
        init_select_data = False
    # select_data setup
    save_date = date(2000,1,1)

    # Remember that get_condition() function returns Condition object. Call .value to get the value
    event_count_cnd = run.get_condition('event_count')

    # get_condition returns None if no such condition is written for run
    if event_count_cnd and event_count_cnd.value > 100000:
        select_data['run_number'].append(run.number)

        if  not run.end_time or not run.start_time:
            print "Run=",run.number," ***Skip*** no begin or end run time\n"
            continue     # skip event if start or end times are not available
        elapsedt = run.end_time-run.start_time
        select_data['elapsed_time'].append(elapsedt)
        seconds = float(elapsedt.total_seconds())
        select_data['elapsed_seconds'].append(seconds)
        hours = seconds/3600.
        rate = 0
        if seconds > 0:
            rate = event_count_cnd.value/seconds
        print "-------------------------------------------------------------------------------------------------"
        print "Run=",run.number," start=",run.start_time," end=",run.end_time,"\n",\
                    "Length of run=",elapsedt," seconds=",seconds," hours=",hours,"\n", \
                                    "Average rate (Hz)=",rate," counts=",event_count_cnd.value
        print "-------------------------------------------------------------------------------------------------"
        if (run.number == run1):
            date_run1 = run.start_time
        elapsed_date = run.start_time - date_run1
        total_date = float(elapsed_date.total_seconds())/3600./24.  # date relative to run 1 start
        select_data['elapsed_days'].append(total_date)
        elapsed_date_exp_start = run.start_time - date_exp_start
        total_date_exp_start = float(elapsed_date_exp_start.total_seconds())/3600./24.  # date relative to run 1 start
        select_data['elapsed_days_exp_start'].append(total_date_exp_start)
        
    else:
        print "Run=",run.number," *** Skip***, count= 0\n"
        continue

    
    for condition in conditions:
        condition_object = run.get_condition(condition)
        if (condition):
            if (condition_object.value_type != 'json'):
                # add unknown conditions that have been added after the first un used for condition init.
                try:
                    select_data[condition].append( condition_object.value)
                except:
                    print "cannot store ", condition, condition_object.value, "was not initialized, will add it"
                    select_data[condition] = i*[type(condition_object.value)()]
                    select_data[condition].append( condition_object.value)
        else:
            print "\tCondition={0: <22}".format(condition)

    if select_data['beam_current'] < min_current: 
        print "Run=",run.number," *** Skip***, minimum current\n"
        continue
#
# convert lists to numpy arrays for selecting
#
for k in select_data.keys():
    select_data[k] = np.array(select_data[k])

# data setup

# make selectors for various radiator combinations

rad_type_0_90 = 'JD70-100 58um 0/90 deg'
rad_type_45_135 = 'JD70-100 58um 45/135 deg'
rad_type_AMO = '3X10-4 Al 30um'

rad_key = 'radiator_type'

sel_para_45_135 = (select_data[rad_key] == 'JD70-100 58um 45/135 deg') & (select_data['polarization_direction'] == 'PARA')
sel_perp_45_135 = (select_data[rad_key] == 'JD70-100 58um 45/135 deg') & (select_data['polarization_direction'] == 'PERP')
sel_para_0_90 = (select_data[rad_key] == 'JD70-100 58um 0/90 deg') & (select_data['polarization_direction'] == 'PARA')
sel_perp_0_90 = (select_data[rad_key] == 'JD70-100 58um 0/90 deg') & (select_data['polarization_direction'] == 'PERP')

sel_para = (select_data['polarization_direction'] == 'PARA')
sel_perp = (select_data['polarization_direction'] == 'PERP')

sel_AMO = (select_data[rad_key] == '3X10-4 Al 30um')

# get event total event counts for the various combinations

current = select_data['beam_current']
radiator_i = select_data['radiator_index']

trig_all =  select_data['event_count'] 

trig_para = select_data['event_count'][sel_para]
trig_perp = select_data['event_count'][sel_perp]

trig_para_45_135 =  select_data['event_count'][sel_para_45_135] 
trig_perp_45_135 =  select_data['event_count'][sel_perp_45_135] 
trig_para_0_90 =  select_data['event_count'][sel_para_0_90] 
trig_perp_0_90 =  select_data['event_count'][sel_perp_0_90] 
trig_AMO =  select_data['event_count'][sel_AMO] 


run_all =  select_data['run_number'] 

run_para_45_135 =  select_data['run_number'][sel_para_45_135] 
run_perp_45_135 =  select_data['run_number'][sel_perp_45_135] 
run_para_0_90 =  select_data['run_number'][sel_para_0_90] 
run_perp_0_90 =  select_data['run_number'][sel_perp_0_90] 
run_perp =  select_data['run_number'][sel_perp] 
run_para =  select_data['run_number'][sel_para] 
run_AMO =  select_data['run_number'][sel_AMO] 


total_all = trig_all.sum()
total_para_45_135 = trig_para_45_135.sum()
total_perp_45_135 = trig_perp_45_135.sum()
total_para_0_90 = trig_para_0_90.sum()
total_perp_0_90 = trig_perp_0_90.sum()
total_AMO = trig_AMO.sum()

# setup the times for plotting

# times per run
time_s_all =  select_data['elapsed_seconds'] 
time_s_perp_45_135 =  select_data['elapsed_seconds'][sel_perp_45_135] 
time_s_para_45_135 =  select_data['elapsed_seconds'][sel_para_45_135] 
time_s_perp_0_90 =  select_data['elapsed_seconds'][sel_perp_0_90] 
time_s_para_0_90 =  select_data['elapsed_seconds'][sel_para_0_90] 
time_s_AMO =  select_data['elapsed_seconds'][sel_AMO] 


# 
time_days_all = select_data['elapsed_days'] 
time_days_perp_45_135 =  select_data['elapsed_days'][sel_perp_45_135] 
time_days_para_45_135 =  select_data['elapsed_days'][sel_para_45_135] 
time_days_perp_0_90 =  select_data['elapsed_days'][sel_perp_0_90] 
time_days_para_0_90 =  select_data['elapsed_days'][sel_para_0_90] 
time_days_AMO =  select_data['elapsed_days'][sel_AMO] 

# 
time_days_exp_start_all = select_data['elapsed_days_exp_start'] 
time_days_exp_start_perp_45_135 =  select_data['elapsed_days_exp_start'][sel_perp_45_135] 
time_days_exp_start_para_45_135 =  select_data['elapsed_days_exp_start'][sel_para_45_135] 
time_days_exp_start_perp_0_90 =  select_data['elapsed_days_exp_start'][sel_perp_0_90] 
time_days_exp_start_para_0_90 =  select_data['elapsed_days_exp_start'][sel_para_0_90] 
time_days_exp_start_AMO =  select_data['elapsed_days_exp_start'][sel_AMO] 


# use start time of last run
exp_time_used = time_days_exp_start_all[-1]

# prepare for plotting


print "\n"
print "Today is",date.today()

print "\n"
print "Para sum={:.3e}\n".format(total_para_0_90),\
    "Perp sum={:.3e}\n".format(total_perp_0_90),\
    "135 sum={:.3e}\n".format(total_perp_45_135),\
    "45 sum={:.3e}\n".format(total_para_45_135),\
    "Amo sum={:.3e}\n".format(total_AMO),\
    "Tot sum={:.3e}\n".format(total_all)

# fraction of total time used
frac_time = exp_time_used/exp_duration_days*100.
print "Fraction of time used : {:.1f}%".format(frac_time)

# make labels for plotting

strtot = '%.1f' % (total_all/1e9) +" B"
strpara = '%.1f' % (total_para_0_90/1e9)+" B"
strperp = '%.1f' % (total_perp_0_90/1e9)+" B"
str135 = '%.1f' % (total_perp_45_135/1e9)+" B"
str45 = '%.1f' % (total_para_45_135/1e9)+" B"
stramo = '%.1f' % (total_AMO/1e9)+" B"

strparafrac = '%.0f' % (100*total_para_0_90/total_all)
strperpfrac = '%.0f' % (100*total_perp_0_90/total_all)
str135frac = '%.0f' % (100*total_perp_45_135/total_all)
str45frac = '%.0f' % (100*total_para_45_135/total_all)
stramofrac = '%.0f' % (100*total_AMO/total_all)

np.set_printoptions(threshold='nan')

figs = []
fig = plt.figure(1)
figs.append(fig)

plt.plot(run_all,         trig_all.cumsum(),'b.',label='  Tot trigs = '+strtot)
plt.plot(run_para_0_90  , trig_para_0_90.cumsum(),'r.',label='     0 trigs = '+strpara+" ("+strparafrac+" %)")
plt.plot(run_para_45_135, trig_para_45_135.cumsum(),'c.',label='   45 trigs = '+str45+" ("+str45frac+" %)")
plt.plot(run_perp_0_90,   trig_perp_0_90.cumsum(),'g.',label='   90 trigs = '+strperp+" ("+strperpfrac+" %)")
plt.plot(run_perp_45_135, trig_perp_45_135.cumsum(),'m.',label=' 135 trigs = '+str135+" ("+str135frac+" %)")
plt.plot(run_AMO,         trig_AMO.cumsum(),'k.',label='Amo trigs = '+stramo+" ("+stramofrac+" %)")
plt.title("Date printed: "+str(date.today()))
plt.ylabel("Int Trig Count")
plt.xlabel("Run Number")
ymin, ymax = plt.ylim()
plt.legend(bbox_to_anchor=(0.56,0.99))
plt.locator_params(axis = 'x', nbins=5)


fig = plt.figure(2)
figs.append(fig)

plt.plot(time_days_all,         trig_all.cumsum(),'b.',label='  Tot trigs = '+strtot)
plt.plot(time_days_para_0_90  , trig_para_0_90.cumsum(),'r.',label='     0 trigs = '+strpara+" ("+strparafrac+" %)")
plt.plot(time_days_para_45_135, trig_para_45_135.cumsum(),'c.',label='   45 trigs = '+str45+" ("+str45frac+" %)")
plt.plot(time_days_perp_0_90,   trig_perp_0_90.cumsum(),'g.',label='   90 trigs = '+strperp+" ("+strperpfrac+" %)")
plt.plot(time_days_perp_45_135, trig_perp_45_135.cumsum(),'m.',label=' 135 trigs = '+str135+" ("+str135frac+" %)")
plt.plot(time_days_AMO,         trig_AMO.cumsum(),'k.',label='Amo trigs = '+stramo+" ("+stramofrac+" %)")

plt.title("Date printed: "+str(date.today()))
plt.ylabel("Int Trig Count")
plt.xlabel("Days from start")
ymin, ymax = plt.ylim()
plt.legend(bbox_to_anchor=(0.56,1.0))
plt.locator_params(axis = 'x', nbins = 5)


fig = plt.figure(3)
figs.append(fig)

plt.plot(time_days_exp_start_all,         trig_all.cumsum(),'b.',label='  Tot trigs = '+strtot)
plt.plot(time_days_exp_start_para_0_90  , trig_para_0_90.cumsum(),'r.',label='     0 trigs = '+strpara+" ("+strparafrac+" %)")
plt.plot(time_days_exp_start_para_45_135, trig_para_45_135.cumsum(),'c.',label='   45 trigs = '+str45+" ("+str45frac+" %)")
plt.plot(time_days_exp_start_perp_0_90,   trig_perp_0_90.cumsum(),'g.',label='   90 trigs = '+strperp+" ("+strperpfrac+" %)")
plt.plot(time_days_exp_start_perp_45_135, trig_perp_45_135.cumsum(),'m.',label=' 135 trigs = '+str135+" ("+str135frac+" %)")
plt.plot(time_days_exp_start_AMO,         trig_AMO.cumsum(),'k.',label='Amo trigs = '+stramo+" ("+stramofrac+" %)")

plt.title("Date printed: "+str(date.today()))
plt.ylabel("Int Trig Count")
plt.xlabel("Running time (days)")
ymin, ymax = plt.ylim()
plt.legend(bbox_to_anchor=(0.56,0.99))
plt.locator_params(nbins=5)


fig = plt.figure(4)
figs.append(fig)

nsub = 1
plt.subplot(2,2,nsub)

plt.plot(run_all,trig_all,'r.')
plt.ylabel("Events per run")
plt.xlabel("Run Number")
# plt.title("Vover="+str(Vover)+" V")
ymin, ymax = plt.ylim()
plt.axis([xmin,xmax,0,1.2*ymax])
plt.locator_params(nbins=5)

nsub = 2
plt.subplot(2,2,nsub)
plt.plot(run_all,current,'r.')
plt.ylabel("Average Current (nA)")
plt.xlabel("Run Number")
ymin, ymax = plt.ylim()
plt.axis([xmin,xmax,0,1.2*ymax])
plt.locator_params(nbins=5)

nsub = 3
plt.subplot(2,2,nsub)
plt.plot(run_all,radiator_i,'r.')
plt.ylabel("Radiator Tag")
plt.xlabel("Run Number")
ymin, ymax = plt.ylim()
plt.axis([xmin,xmax,-1,15])
plt.locator_params(nbins=5)

nsub = 4
plt.subplot(2,2,nsub)
plt.plot(run_all,trig_all.cumsum(),'b.')
plt.plot(run_para,trig_para.cumsum(),'r.')
plt.plot(run_perp,trig_perp.cumsum(),'k.')
plt.plot(run_AMO,trig_AMO.cumsum(),'g.')
plt.ylabel("Int Trig Count")
plt.xlabel("Run Number")
ymin, ymax = plt.ylim()
plt.locator_params(nbins=4)

plt.tight_layout()

figs[0].savefig("fig1.png");
figs[1].savefig("fig2.png");
figs[2].savefig("fig3.png");
figs[3].savefig("fig4.png");

plt.show()

