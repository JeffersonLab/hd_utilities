import os
import subprocess
import glob
import rcdb
import shutil

testing = True  # just process one file
tolerance = 0.1
setblanks = True

def main():

    dirs = ["eff_each_run", "eff_combined"]
    for d in dirs:
        if not os.path.exists(d):
            os.makedirs(d)

    #rundirlist = subprocess.check_output(["ls", "hists_merged"]).splitlines()
    #for rundir in rundirlist:

    if os.path.exists("hists"):
        histdir = "hists"
    elif os.path.exists("hists_merged"):
        histdir = "hists_merged"
    else:
        exit("Cannot find hists or hists_merged")

    db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb2")

    #filelist = subprocess.check_output(["ls", "hists_merged/" + rundir]).splitlines()
    filelist = subprocess.check_output(["ls", histdir]).splitlines()
    filelist = os.listdir(histdir)
    filelist.sort()

    firstrun = True

    for file in filelist:

        run = file[8:14]

        condition = db.get_condition(run, "status")
        if not condition:
            continue
        if condition.value < 1:
            continue

        #if not os.path.exists(get_filename(run)):
            
        runfile = histdir + "/" + str(file)
        scriptname = "/work/halld/njarvis/gluex26/eff/scripts/dump_eff.C"

        subprocess.call(["root", "-l", "-b", "-q", runfile, scriptname])

        if not os.path.exists("eff.txt") :
            add_to_bad_list(run)
            continue

        if setblanks:
            set_blanks(run)
        else:
            os.rename("eff.txt", get_filename(run))

        if firstrun:
            start = run
            latest = run
            different = False
            firstrun = False
        else:
            previous = latest
            latest = run
            different = test_different(start, latest, tolerance)
            
        if different:
            startfile = get_filename(start)
            shutil.copy(startfile, get_combined_filename(start, previous))
            start = latest
                        
        if testing:  
            break

    # at end of loop output combo file for last group of runs
    if not different:
        startfile = get_filename(start)
        shutil.copy(startfile, get_combined_filename(start, latest))
        
    


def get_filename(run):
    return "eff_each_run/eff_" + run + ".txt"

def get_combined_filename(run1, run2):
    return "eff_combined/eff_" + run1 + "_" + run2 + ".txt"

def add_to_bad_list(run):
    fbad = open("badfiles.txt", "a")
    fbad.write(run)
    fbad.write("\n")
    fbad.close()
    return  

def test_different(run1, run2, tolerance):
    fname1 = get_filename(run1)
    fname2 = get_filename(run2)

    different = False
    
    with open(fname1, "r") as f1, open(fname2, "r") as f2:
        list1 = f1.readlines()
        list2 = f2.readlines()

        for i in range(0, len(list1)):
            n1 = float(list1[i])
            n2 = float(list2[i])

            diff = abs(n1 - n2)
            if diff >= tolerance:
                different = True

    return different


def set_blanks(run) :

    r = int(run)
    eff = []

    B7 = [233, 234, 235, 236, 237, 238, 239, 240]
    B7.extend([300, 301, 302, 303, 304, 305, 306])
    B7.extend([374, 375, 376, 377, 378, 379, 380, 381, 382])
              
    H13 = [2437, 2438, 2439, 2440, 2441, 2442, 2443]
    H13.extend([2620, 2621, 2622, 2623, 2624, 2625, 2626, 2627])
    H13.extend([2809, 2810, 2811, 2812, 2813, 2814, 2815, 2816])

    H14 = [2444, 2445, 2446, 2447, 2448, 2449, 2450, 2451]
    H14.extend([2628, 2629, 2630, 2631, 2632, 2633, 2634, 2635])
    H14.extend([2817, 2818, 2819, 2820, 2821, 2822, 2823, 2824])

    
    with open("eff.txt", "r") as f:
        for line in f:
            eff.append(float(line.strip()))
    '''
    150103-150156 - CDC B7 tripped on July 14
    150135-150145 CDC H13, I14 tripped on July 17 close to the end of 150134
    150137-150145 CDC H15, I16 tripped
    150137 - CDC H14, I15 tripped & restored
    150147 - CDC H13, I14 tripped & restored
    150174-150176 - CDC H13, I14 tripped & restored
    150201-150212 - CDC B7 tripped on July 22
    150216 - CDC B7 tripped on July 22 at 21:15, 7 minutes before the end of 150215, was restored midway through 150216
    150245-150293 - CDC B7 tripped on July 24 at the start of 150245, restored before 150294
    150335- -150346 - CDC B7 tripped on July 31
    '''

    for i in range(0,3522):
        straw = i+1
        
        blank_B7 = False
        if (r >= 150103 and r <= 150156) or (r >= 150201 and r <= 150212) or (r == 150216):
            blank_B7 = True
        if ( r >= 150245 and r <= 150293) or (r >= 150335 and r <= 150346) :
            blank_B7 = True

        if blank_B7:
            if straw in B7:
                eff[i] = 0





        
    with open(get_filename(run), "w") as f:
        for x in eff:
            f.write(f"{x:.2f}")
            f.write("\n")
    return



if __name__ == "__main__":
    main()
