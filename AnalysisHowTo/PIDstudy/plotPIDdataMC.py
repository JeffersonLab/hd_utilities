import os
from ROOT import gDirectory,gStyle,TFile,TCanvas,TF1,TLegend

# define function to get list of string keys in a directory
def GetKeyNames( self, dir = "" ):
    self.cd(dir)
    return [key.GetName() for key in gDirectory.GetListOfKeys()]

TFile.GetKeyNames = GetKeyNames

# some basic setup for canvases and legends
gStyle.SetOptStat(0)
leg = TLegend(0.6,0.7,0.9,0.9)

cc2D = TCanvas("cc2D","cc2D",800,400)
cc2D.Divide(2,1)
cc1D = TCanvas("cc1D","cc1D",600,400)

plotDir = "plots/"
if not os.path.exists(plotDir):
    os.mkdir(plotDir)

# default dE/dx cut for plotting on 2D distribution
fMinProton_dEdx = TF1("fMinProton_dEdx", "exp(-1.*[0]*x + [1]) + [2]", 0., 10.) # cut for dEdx curve
fMinProton_dEdx.SetParameters(5.2, 3.0, 0.9)
fMinProton_dEdx.SetLineColor(2)


# Parser needs
## Files name and label from text input (defines canvas size and labels)
## Momentum range for given particle?
## DeltaT range to zoom in

pidPath = "Hist_ParticleID_Initial"
maxDeltaT = 2.5
minP = 0.0
maxP = 2.5
minSliceP = 0.8
maxSliceP = 1.0

# Insert your input files here for Data and MC
files = []
files.append(TFile.Open("/Users/jrsteven/gluex/p2gamma/histroot/pid_syst/hist_sum_30730_30788.root")) # Data
files.append(TFile.Open("/Users/jrsteven/gluex/p2gamma/histroot/pid_syst/hist_sum_30274_31057_sim_g4.root")) #MC

# Open file and get list of keys
f = files[0]
stepKeyList = f.GetKeyNames(pidPath)

# collect list of histogram names (full path)
hists = []
particles = []
for step in stepKeyList:
    print("Keys in file:", step)
    particleKeyList = f.GetKeyNames(pidPath+"/"+step)
   
    for particle in particleKeyList:
        print("Particle in step:", particle)
        histKeyList = f.GetKeyNames(pidPath+"/"+step+"/"+particle)
        
        for hist in histKeyList:
            print("Histogram for particle:", hist)
            if "DIRC" in hist or "Beta" in hist: # skip DIRC and Best histograms for now
                continue
            
            particles.append(particle)
            hists.append(pidPath+"/"+step+"/"+particle+"/"+hist)

# make plots for each histogram in the list
ihist = 0
for hist,particle in zip(hists,particles):
    print(hist)
    ifile = 0
    projMinBin = 0
    projMaxBin = 999
    norm = 0
    
    
    for file in files:
        h = file.Get(hist)
        
        if "DeltaT" in hist:
            h.GetYaxis().SetRangeUser(-1.*maxDeltaT,maxDeltaT)
            
        # make 1D slices in momentum for comparison
        plot1D = "p (GeV/c)" in h.GetXaxis().GetTitle()
        if plot1D:
        
            # 1D projection
            cc1D.cd()
            if ifile == 0: #Data specific values
                projMinBin = h.ProjectionX().FindBin(minSliceP);
                projMaxBin = h.ProjectionX().FindBin(maxSliceP);
                h1D = h.ProjectionY("%s" % hist,projMinBin,projMaxBin)
                h1D.SetTitle(h.GetTitle())
                h1D.SetLineColor(ifile+1)
                h1D.SetMarkerColor(ifile+1)
                h1D.SetMarkerStyle(20)
                norm = h1D.GetMaximum()
                h1D.Draw("pe")
                
                if ihist == 0:
                    leg.AddEntry(h1D,"Data","p")
                    
            else: # MC specific values
                h1D = h.ProjectionY("%d" % ifile,projMinBin,projMaxBin)
                if h1D.GetMaximum() > 0:
                    h1D.Scale(norm/h1D.GetMaximum())
                h1D.SetLineColor(ifile+1)
                h1D.SetMarkerColor(ifile+1)
                h1D.SetMarkerStyle(20)
                h1D.Draw("pe same")
                
                if ihist == 0:
                    leg.AddEntry(h1D,"MC","p")
                    
                leg.Draw("same")
                
            if "Proton" in particle:
                h.GetXaxis().SetRangeUser(minP,maxP)
        
        cc2D.cd(ifile+1)
        h.Draw("colz")
        
        if "CDC dE/dx" in h.GetYaxis().GetTitle():
            fMinProton_dEdx.Draw("same")
        
        ifile += 1
    ihist += 1
    
    # print plots
    cc2D.Print("%s%s.pdf" % (plotDir,hist.replace("/","_")))
    if plot1D:
        cc1D.Print("%spSlice_%s.pdf" % (plotDir,hist.replace("/","_")))
