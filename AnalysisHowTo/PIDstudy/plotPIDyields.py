import os
from ROOT import gDirectory,gPad,TFile,TCanvas,TF1,TH1F,TMath

# define function to get list of string keys in a directory
def GetKeyNames( self, dir = "" ):
    self.cd(dir)
    return [key.GetName() for key in gDirectory.GetListOfKeys()]

TFile.GetKeyNames = GetKeyNames

# Canvas for plotting results
cc1D = TCanvas("cc1D","cc1D",800,400)
cc1D.Divide(2,1)

plotDir = "plots/"
if not os.path.exists(plotDir):
    os.mkdir(plotDir)

# Define double gaussian fit function
fPi0 = TF1("fitPi0", "gaus(0) + gaus(3) + pol1(6)", 0.08, 0.19)
fPi0.SetLineColor(2)

# Parser needs
## Files name and label from text input (defines canvas size and labels)

pidPath = "Hist_AnalyzeCutActions_CutActionEffect"

# Insert your input files here for Data and MC
files = []
files.append(TFile.Open("/Users/jrsteven/gluex/p2gamma/histroot/pid_syst/hist_sum_30730_30788.root")) # Data
files.append(TFile.Open("/Users/jrsteven/gluex/p2gamma/histroot/pid_syst/hist_sum_30274_31057_sim_g4.root")) # MC

# Open data file file and get list of keys in AnalyzeCutActions directory
f = files[0]
cuts = f.GetKeyNames(pidPath)
hists = []

# collect list of histogram names (full path)
for cut in cuts:
    print("Cuts in file:", cut)
    hists.append(pidPath+"/"+cut)
#print(cuts)
#print(hists)

nominalYieldData = 0
nominalYieldMC = 0

# crate histograms to store yields
nEmptyBins = 4
yieldHistData = TH1F("yieldData",";", len(cuts)+nEmptyBins, -0.5, -0.5+len(cuts) +nEmptyBins)
yieldHistMC = TH1F("yieldMC",";", len(cuts)+nEmptyBins, -0.5, -0.5+len(cuts) +nEmptyBins)
for cut in cuts: # set histogram bin names
    yieldHistData.Fill(cut, 0)
    yieldHistMC.Fill(cut, 0)
yieldRatioData = yieldHistData.Clone("yieldRatioData")
yieldRatioMC = yieldHistMC.Clone("yieldRatioMC")

# make plots for each histogram in the list
ihist = 0
for hist,cut in zip(hists,cuts):
    ifile = 0

    # fit yield in data and MC for each histogram
    for file in files:
        cc1D.cd(ifile+1)
        h = file.Get(hist)
        
        # setup simple histogram fit with double gaussian and polynomial
        fPi0.SetParameters(h.GetMaximum(), 0.135, 0.007, h.GetMaximum()/10., 0.135, 0.015)
        
        # draw histogram and fit
        h.SetMarkerStyle(20)
        h.Draw("pe")
        fitResult = h.Fit(fPi0,"Q","",0.08,0.19)
        fitYield = fPi0.Integral(0.11,0.16)/h.GetXaxis().GetBinWidth(1)
        fitYieldError = fPi0.IntegralError(0.11,0.16)/h.GetXaxis().GetBinWidth(1) #,fitResult.GetParams(),fitResult.GetCovarianceMatrix().GetMatrixArray(),0.05
            
        if ifile==0: #Data specific values
            if ihist == 0:
                nominalYieldData = fitYield
                nominalYieldErrorData = fitYieldError
            yieldHistData.SetBinContent(ihist+1,fitYield)
            yieldHistData.SetBinError(ihist+1,fitYieldError)
            yieldRatio = nominalYieldData/fitYield
            yieldRatioRelError = TMath.Sqrt(pow(nominalYieldErrorData/nominalYieldData,2) + pow(fitYieldError/fitYield,2))
            yieldRatioData.SetBinContent(ihist+1,nominalYieldData/fitYield)
            yieldRatioData.SetBinError(ihist+1,yieldRatio*yieldRatioRelError)
        else: # MC specific values
            if ihist == 0:
                nominalYieldMC = fitYield
                nominalYieldErrorMC = fitYieldError
            yieldHistMC.SetBinContent(ihist+1,fitYield)
            yieldHistMC.SetBinError(ihist+1,fitYieldError)
            yieldRatio = nominalYieldMC/fitYield
            yieldRatioRelError = TMath.Sqrt(pow(nominalYieldErrorMC/nominalYieldMC,2) + pow(fitYieldError/fitYield,2))
            yieldRatioMC.SetBinContent(ihist+1,yieldRatio)
            yieldRatioMC.SetBinError(ihist+1,yieldRatio*yieldRatioRelError)
        ifile += 1
    ihist += 1
    
    cc1D.Print("%sfit_%s.pdf" % (plotDir,cut.replace("/","_")))
    cc1D.Print("%sfitSummary.pdf(" % plotDir)

# plot yields and take ratio of data/MC efficiency ratio
ccYieldRatio = TCanvas("ccYieldRatio","ccYieldRatio",900,400)
ccYieldRatio.Divide(3,1)

ccYieldRatio.cd(1);
gPad.SetBottomMargin(0.15)
yieldHistData.SetMarkerStyle(20)
yieldHistData.Draw("pe")
ccYieldRatio.cd(2);
gPad.SetBottomMargin(0.15)
yieldHistMC.SetMarkerStyle(20)
yieldHistMC.Draw("pe")

ccYieldRatio.cd(3)
gPad.SetBottomMargin(0.15)
gPad.SetLeftMargin(0.15)
efficRatio = yieldRatioData.Clone("efficRatio")
efficRatio.GetYaxis().SetTitle("Efficiency Ratio Data/MC")
efficRatio.Divide(yieldRatioMC)
efficRatio.SetMinimum(0.9)
efficRatio.SetMaximum(1.1)
efficRatio.SetMarkerStyle(20)
efficRatio.Draw("pe")

ccYieldRatio.Print("%sefficiency.pdf" % plotDir)
ccYieldRatio.Print("%sfitSummary.pdf)" % plotDir)
