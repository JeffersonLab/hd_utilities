import os
import subprocess
import rcdb

testing = False  # just process one file

def main():

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

    max_eff = []
    this_eff = []
    best_run = []
    
    for file in filelist:

        run = file[8:14]

        condition = db.get_condition(run, "status")
        if not condition:
            continue
        if condition.value < 1:
            continue

        #if not os.path.exists(get_filename(run)):
        '''    
        runfile = histdir + "/" + str(file)
        scriptname = "/work/halld/njarvis/gluex26/eff/scripts/dump_eff.C"

        subprocess.call(["root", "-l", "-b", "-q", runfile, scriptname])
        fname= 'eff.txt'
        '''
        fname = get_filename(run)
        
        if not os.path.exists(fname) :
            add_to_bad_list(run)
            continue
      
        if firstrun:
            firstrun = False
            max_eff = read_eff(fname)
            for i in range(0, 3522):
                best_run.append(run)
        else:
            this_eff = read_eff(fname)

            #print(run, this_eff[1], max_eff[1])
            for i in range(0, 3522):
                if this_eff[i] > max_eff[i]:
                    max_eff[i] = this_eff[i]
                    best_run[i] = run
                    
            
        if testing:  
            break

    with open("max_eff.txt", "w") as f:
        for x in max_eff:
            f.write(f"{x:.2f}")
            f.write("\n")

    with open("best_run.txt", "w") as f:
        for x in best_run:
            f.write(x)
            f.write("\n")


def get_filename(run):
    x = 'eff_from_histo/eff_' + str(run) + '.txt'
    return x
            
def add_to_bad_list(run):
    fbad = open("badfiles.txt", "a")
    fbad.write(run)
    fbad.write("\n")
    fbad.close()
    return  

def read_eff(filename):

    eff = []
    with open(filename, "r") as f1:
        list = f1.readlines()

        for x in list:
            eff.append(float(x))

    return eff



if __name__ == "__main__":
    main()
