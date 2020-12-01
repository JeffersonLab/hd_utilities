#!/usr/bin/env python
import os,sys
from optparse import OptionParser
from ROOT import gDirectory,gStyle,gROOT,TFile,TCanvas,TF1,TLegend

# define function to get list of string keys in a directory
def GetKeyNames( self, dir = "" ):
    self.cd(dir)
    return [key.GetName() for key in gDirectory.GetListOfKeys()]

TFile.GetKeyNames = GetKeyNames

def main():

    # some basic setup for canvases and legends
    gROOT.SetBatch(1)
    gStyle.SetOptStat(0)
    leg = TLegend(0.6,0.7,0.9,0.9)
    
    plotDir = "plots/"
    if not os.path.exists(plotDir):
        os.mkdir(plotDir)
    
    # default dE/dx cut for plotting on 2D distribution
    fMinProton_dEdx = TF1("fMinProton_dEdx", "exp(-1.*[0]*x + [1]) + [2]", 0., 10.) # cut for dEdx curve
    fMinProton_dEdx.SetParameters(4.0, 2.25, 1.0)
    fMinProton_dEdx.SetLineColor(2)
    
    fMaxPion_dEdx = TF1("fMaxPion_dEdx", "exp(-1.*[0]*x + [1]) + [2]", 0., 10.) # cut for dEdx curve
    fMaxPion_dEdx.SetParameters(7.0, 3.0, 6.2)
    fMaxPion_dEdx.SetLineColor(2)
    
    fMaxElectron_dEdx = TF1("fMaxElectron_dEdx", "[0]", 0., 10.) # cut for dEdx curve
    fMaxElectron_dEdx.SetParameter(0,5.5)
    fMaxElectron_dEdx.SetLineColor(2)
    
    # default
    pidPath = "Hist_ParticleID_Initial"
    maxDeltaT = 2.5
    minProtonPrange = 0.0
    maxProtonPrange = 2.5
    #minSliceP = 0.8
    #maxSliceP = 1.0
    #numSlices = 1
    minSliceP = 0.6
    maxSliceP = 2.
    numSlices = 7

    # Parser could set these from command line:
    parser = OptionParser(usage = "plotPIDdataMC.py --path pidPath --proton-plotmin plotMin --proton-plotmax plotMax --min-slice minSliceP --max-slice maxSliceP --timerange maxDeltaT")
    parser.add_option("-p","--path", dest="path", help="Path to PID histogram directory")
    parser.add_option("-a","--proton-plotmin", dest="plotmin", help="Minimum proton plot momentum")
    parser.add_option("-b","--proton-plotmax", dest="plotmax", help="Maximum proton plot momentum")
    parser.add_option("-c","--min-slice", dest="minslice", help="Minimum momentum for 1D projection")
    parser.add_option("-d","--max-slice", dest="maxslice", help="Maximum momentum for 1D projection")
    parser.add_option("-n","--num-slices", dest="numslices", help="Number of 1D projection slices to make")
    parser.add_option("-t","--trange", dest="trange", help="Timing |DeltaT| maximum")
    parser.add_option("-v","--verbose", action="store_true", default=False, dest="verbose", help="Verbose mode (see canvases produced)")

    # set parser options if available
    (options, args) = parser.parse_args(sys.argv)
    #parser.print_help()

    if options.path:
        pidPath = options.path
    if options.plotmin:
        minProtonPrange = float(options.plotmin)
    if options.plotmax:
        maxProtonPrange = float(options.plotmax)
    if options.minslice:
        minSliceP = float(options.minslice)
    if options.maxslice:
        maxSliceP = float(options.maxslice)
    if options.trange:
        maxDeltaT = float(options.trange)
    if options.verbose:
        print("verbose")
        gROOT.SetBatch(0)

    # Get list of ROOT histogram files (and labels) to include from input text file
    files = []
    labels = []
    finput = open("input_file_labels.txt")
    inputlines = finput.read().splitlines()
    for line in inputlines:
        input_list = line.split(",")
        #print input_list
        files.append(TFile.Open(input_list[0]))
        labels.append(input_list[1])
        
    nfiles = len(files)
    if nfiles == 0 or nfiles > 6:
        return

    # Setup canvases
    cc2D = TCanvas("cc2D","cc2D",800,400)
    cc1Ds = []
    for x in xrange(numSlices):
	    cc1D = TCanvas("cc1D_%d"%(x+1),"cc1D_%d"%(x+1),600,400)
	    cc1Ds.append(cc1D)
    if nfiles < 4: # single row of panels
        cc2D.Divide(nfiles,1)
    else: # double row of panels
        cc2D.SetWindowSize(800, 800)
        cc2D.Divide(nfiles/2,2)
        
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

	# numSlices

    # make plots for each histogram in the list
    ihist = 0
    for hist,particle in zip(hists,particles):
        print(hist)
        ifile = 0
        projMinBin = 0
        projMaxBin = 999
        norm = 0
        
        for file,label in zip(files,labels):
            h = file.Get(hist)
            
            if "DeltaT" in hist:
                h.GetYaxis().SetRangeUser(-1.*maxDeltaT,maxDeltaT)
            
            # make 1D slices in momentum for comparison
            plot1D = "p (GeV/c)" in h.GetXaxis().GetTitle()
            if plot1D:
                
                # 1D projection
                #cc1D.cd()
                if ifile == 0: #Data specific values
                    projMinBin = h.ProjectionX().FindBin(minSliceP)
                    projMaxBin = h.ProjectionX().FindBin(maxSliceP)
                    binStep = (projMaxBin-projMinBin) / numSlices
                    pStep = (maxSliceP-minSliceP) / numSlices
                    
                    minBin = projMinBin
                    minSlice = minSliceP
                    for slice in xrange(numSlices):
						cc1Ds[slice].cd()
						h1D = h.ProjectionY("%s_%d" % (hist,slice+1),minBin,minBin+binStep)
						h1D.SetTitle(h.GetTitle()+": %0.2f < p < %0.2f GeV" % (minSlice,minSlice+pStep))
						h1D.SetLineColor(ifile+1)
						h1D.SetMarkerColor(ifile+1)
						h1D.SetMarkerStyle(20)
						norm = h1D.GetMaximum()
						h1D.Draw("pe")
					
						if ihist == 0:
							leg.AddEntry(h1D,label,"p")
						
						minBin = minBin+binStep
						minSlice = minSlice+pStep
                    
                else: # MC specific values
                    minBin = projMinBin
                    minSlice = minSliceP
                    for slice in xrange(numSlices):
						cc1Ds[slice].cd()
						h1D = h.ProjectionY("%s_%d" % (ifile,slice+1),minBin,minBin+binStep)
						if h1D.GetMaximum() > 0:
							h1D.Scale(norm/h1D.GetMaximum())
						h1D.SetLineColor(ifile+1)
						h1D.SetMarkerColor(ifile+1)
						h1D.SetMarkerStyle(20)
						h1D.Draw("pe same")
				
						if ihist == 0:
							leg.AddEntry(h1D,label,"p")
					
							leg.Draw("same")
                
            if "Proton" in particle:
                h.GetXaxis().SetRangeUser(minProtonPrange,maxProtonPrange)
                
            cc2D.cd(ifile+1)
            h.Draw("colz")

            if "CDC dE/dx" in h.GetYaxis().GetTitle():
                if h.GetTitle() == "p":
                    fMinProton_dEdx.Draw("same")
                elif "e^" in h.GetTitle():
                    fMaxElectron_dEdx.Draw("same")
                else:
                    fMaxPion_dEdx.Draw("same")
                
            h.SetTitle(label+": "+h.GetTitle())

            ifile += 1
        ihist += 1
    
        # print plots
        cc2D.Print("%s%s.pdf" % (plotDir,hist.replace("/","_")))
        if plot1D:
			minSlice = minSliceP
			for slice in xrange(numSlices):
				cc1Ds[slice].Print("%spSlice%0.2f-%0.2f_%s.pdf" % (plotDir,minSlice,minSlice+pStep,hist.replace("/","_")))
				minSlice = minSlice+pStep

## main function 
if __name__ == "__main__":
    main()
