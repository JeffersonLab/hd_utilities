# already ran get_all_eff to dump efficiency histo contents into eff_from_histo directory

import os
import subprocess
import glob
import rcdb
import shutil


def main():

    # original wire gains
    wg = []
    with open("/work/halld/njarvis/gluex26/wiregains/cdc_new_wiregains_sum_150468_150477_plus_sum_150519_150526.txt","r") as file:
        for line in file:
            wg.append(float(line))


    
    dirs = ["wg_combined"]
    for d in dirs:
        if not os.path.exists(d):
            os.makedirs(d)

            
    filedir = "eff_ts_combined"
    filelist = os.listdir(filedir)
    filelist.sort()


    fadd = open("add_to_ccdb", "w") 
    
    
    for efile in filelist:

        # eff_150097_150102.txt

        newfile = "wg_combined/wg_" + efile[4:]

        eff = []
        with open("eff_ts_combined/" + efile, "r") as f:
            for line in f:
                eff.append(float(line))

        new = []
        for w, e in zip(wg, eff):
            x = w
            if e == 0:
                x = 0
            new.append(x)

        with open(newfile, "w") as ff:
            for x in new:
                print(f"{x:.3f}", file=ff)

        range = efile[4:10] + "-" + efile[11:17]
                
        print("ccdb add /CDC/wire_gains -r " + range + " " + newfile, file=fadd)
        print("ccdb add /CDC/wire_mc_efficiency -r " + range + ' -v "mc" eff_ts_combined/' + efile, file=fadd)        

    fadd.close()
        
if __name__ == "__main__":
    main()
