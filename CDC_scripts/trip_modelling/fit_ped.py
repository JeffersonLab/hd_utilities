import os
import sys
from ROOT import TFile, TF1
from ROOT import gROOT



def main():


    if len(sys.argv) < 3:
        exit('Usage: python fit_ped.py <rootfile> <straw_number> [<draw_histo>]')
    
    filename = sys.argv[1]
    straw = int(sys.argv[2])
    if len(sys.argv) == 4:
        drawh = True
        fitoptions = 'rw'
    else:
        drawh = False
        gROOT.SetBatch(False)
        fitoptions = 'rw0q'
            
    with open("/work/halld/njarvis/gluex26/eff/CDC_board_names.txt", "r") as f:
        for i in range(0, straw + 1):
            f.readline()
        things = f.readline().split()
        ring = things[1]
        bin = int(things[2])
    
    try:
        rootfile = TFile(filename)
    except :
        exit()
    
    
    hname = "cdc_ped_ring[" + ring + "]"
    
    rootfile.cd("/CDC_expert_2/rings_pedestal/")
    
    hh = gROOT.FindObject(hname)
    
    if not hh:
        exit('Could not open histogram '+hname)
    
    
    h = hh.ProjectionY("h",bin,bin)
    
    if not h:
        exit('Could not find bin',bin)
    

    ped, sig = fit_pedestal(h, fitoptions)
    #print(filename, ped, sig)
    print(f"{filename[17:23]}  {straw} {ped:.2f} {sig:.2f}")

    
    



def fit_pedestal(h, fitoptions):
    
    nentries = h.GetEntries()
        
    nfirstbin = h.GetBinContent(1)
    nlastbin = h.GetBinContent(h.GetNbinsX())
    
    
    
    if nfirstbin > 0.5 * nentries:
        ped = 0
        sig = 0
    
    elif nlastbin > 0.5 * nentries:
        ped = 255
        sig = 0
    
    else:
    
        n = h.GetEntries() - h.GetBinContent(1) - h.GetBinContent(h.GetNbinsX())
    
        g = TF1("g","gaus",5,250)
    
        g.SetParameters(0.03*n, 100, 20)
        
        fitresult = h.Fit(g, fitoptions)
    
        if int(fitresult) == 0:
            ped = g.GetParameter(1)
            sig = g.GetParameter(2)
    
        else :
           ped = h.GetMean()
           sig = h.GetRMS()

    if not 'q' in fitoptions :
        h.Draw()
        input()

           
           #print("Bad fit",file,straw)
           #print(f"  {straw} {ped:.2f} {sig:.2f}  Bin 1 {nfirstbin/nentries:.2f} Bin 128 {nlastbin/nentries:.2f}")           
    return ped, sig


if __name__ == "__main__":
    main()
