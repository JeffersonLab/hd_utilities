from ROOT import gROOT, TFile, TH1, TH2, TF1, TGraphErrors, TCanvas
import os
import sys
import rcdb
from array import array


def main():

    # Suppress warnings and info messages (only show Errors and higher)
    #ROOT.gErrorIgnoreLevel = ROOT.kError  # kError is 3000
    gROOT.SetBatch(True)
    
    dirs = ["pedestals", "problems"]
    for d in dirs:
        if not os.path.exists(d):
            os.makedirs(d)

    filedir = "hists"
    filelist = os.listdir(filedir)
    filelist.sort()

    firstrun = True

    g = TF1('g','gaus',0,255)

    runs = []

    peds = []
    sigs = []

    problemstraws = []
    
    for i in range(0,3522):
        peds.append([])
        sigs.append([])
        problemstraws.append(0)        


        
    db = rcdb.RCDBProvider("mysql://rcdb@hallddb/rcdb2")        
    
    for filename in filelist:

        run = filename[8:14]


        condition = db.get_condition(run, "status")
        if not condition:
            continue
        if condition.value != 1:
            continue        

        print(run)
        
        rootfile = TFile("hists/" + filename)

        rootfile.cd("/CDC_expert_2/rings_pedestal")

        runs.append(int(run))
        
        straw = 0
        for ring in range (1,29):
            histoname = "cdc_ped_ring[" + str(ring) + "]"
            hh = gROOT.FindObject(histoname)
            if not hh:
                print(run,'Histo missing')
                break

            for x in range(1, 1+hh.GetNbinsX()):
                straw = straw + 1

                h = hh.ProjectionY(str(x),x,x)

                ped, sig = fit_pedestal(h)

                peds[straw-1].append(ped)
                sigs[straw-1].append(sig)                

                if ped <= 2.5*sig :
                    problemstraws[straw-1] = 1


    x = array('d')
    ex = array('d')
    for r in runs:
        x.append(float(r))
        ex.append(0)


    c = TCanvas("c","Eff",900,500)   
    
    #print('Problem straws:')
    for i in range(0,3522):

        with open("pedestals/ped_" + str(i+1) + ".txt", "w") as f:
            for j in range(0,len(runs)):
                f.write(f"{runs[j]:.0f} {peds[i][j]:.1f} {sigs[i][j]:.1f}\n")

        
        y = array('d')
        ey = array('d')            
        for ped in peds[i]:
            y.append(float(ped))
        for sig in sigs[i]:
            ey.append(float(sig))                

        g = TGraphErrors( len(x), x, y, ex, ey )
        g.SetTitle("Straw " + str(i+1) + " pedestal")
        g.GetXaxis().SetNoExponent(True)
        g.GetXaxis().SetTitle( 'Run number' )
        g.GetYaxis().SetTitle( 'Pedestal' )
        g.SetMarkerStyle( 20 )
        g.SetMarkerSize( 0.7 )
        g.SetMarkerColor(9)
            

        g.Draw("ap")

        c.SaveAs("pedestals/n" + str(i+1) + ".png")

        if problemstraws[i] == 1:
            c.SaveAs("problems/n" + str(i+1) + ".png")
            
    
'''
        with open(get_filename(run), "w") as f:
            for x in eff:
                f.write(f"{x:.2f}")
                f.write("\n")
'''

#gROOT.SetBatch(True)

def fit_pedestal(h):
    
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
        
        fitresult = h.Fit(g,"rwq0")
    
        if int(fitresult) == 0:
            ped = g.GetParameter(1)
            sig = g.GetParameter(2)

            if ped < 0:
                ped = 0
                sig = 0
            
        else :
           ped = h.GetMean()
           sig = h.GetRMS()
    
           #print("Bad fit",file,straw)
           #print(f"  {straw} {ped:.2f} {sig:.2f}  Bin 1 {nfirstbin/nentries:.2f} Bin 128 {nlastbin/nentries:.2f}")           
    return ped, sig



if __name__ == "__main__":
    main()
