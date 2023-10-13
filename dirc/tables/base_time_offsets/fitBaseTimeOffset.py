#!/usr/bin/env python

import sys
import os
import subprocess
import time
import ROOT

# main function
def main():

    ROOT.gStyle.SetOptStat(0)
    
    # define paths for input
    MyInDataDir="/volatile/halld/home/gxproj7/RunPeriod-2019-01/dirc_production/ver00/hists/"
    #MyInDataDir="/volatile/halld/home/gxproj7/RunPeriod-2019-01/dirc_monitoring/analysis_REST/ver08_pass09/hists/"
    MyInDataDir="/w/halld-scifs17exp/halld2/home/gxproj7/"
    minRun = 60700
    maxRun = 60883
    
    cc = ROOT.TCanvas("cc","cc",600,400)
    
    cc.cd()
    cc.SetBottomMargin(0.2)
    
    for file in sorted(os.listdir(MyInDataDir)):
        if file.startswith("060830") or file.startswith("060841"):
            continue

        if file.startswith("060"):
            #print(file)
            runNumber = file.replace("hd_root_", "")
            #runNumber = runNumber.replace(".root", "")
        
            filename = MyInDataDir+file+"/hd_root_"+file+"_000.root"
            if not os.path.exists(filename):
                continue
            f = ROOT.TFile(filename,"r")
            h = f.Get("DIRC/Pi+/hDiff_Pi+")
            center = h.GetXaxis().GetBinCenter(h.GetMaximumBin());
            min = center - 2.;
            max = center + 2.;
            f1 = ROOT.TF1("gaus","gaus",min,max);
            h.Fit(f1,"Q","",min,max);
            print("Run %s: offset = %f" % (runNumber,f1.GetParameter(1)))
        
    return

if __name__=="__main__":
    main()
 

