#!/usr/bin/env python

import ROOT
from ROOT import TMath

from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
import glob
import math

##################### RCDB ENVIRONMENT ####################
os.environ["RCDB_HOME"] = "/group/halld/www/halldweb/html/rcdb_home"
sys.path.append("/group/halld/www/halldweb/html/rcdb_home/python")
import rcdb
db = rcdb.RCDBProvider("mysql://rcdb@hallddb.jlab.org/rcdb2")    

##################### main function ####################
def main(argv):

    ROOT.gStyle.SetOptStat(0)

    parser_usage = "loopRunsRCDB.py minrun maxrun label"
    parser = OptionParser(usage = parser_usage)
    (options, args) = parser.parse_args(argv)
    
    if(len(args) != 3):
        parser.print_help()
        return
        
    # GET ARGUMENTS
    MINRUN = int(args[0])
    MAXRUN = int(args[1])
    label = args[2]

    hPol_0 = ROOT.TH1F("polarization_0", "; Run Number; Polarization;", MAXRUN-MINRUN, MINRUN, MAXRUN)
    hPol_90 = ROOT.TH1F("polarization_90", "; Run Number; Polarization;", MAXRUN-MINRUN, MINRUN, MAXRUN)
    hPol_45 = ROOT.TH1F("polarization_45", "; Run Number; Polarization;", MAXRUN-MINRUN, MINRUN, MAXRUN)
    hPol_135 = ROOT.TH1F("polarization_135", "; Run Number; Polarization;", MAXRUN-MINRUN, MINRUN, MAXRUN)
    
    hEnhancement_0 = hPol_0.Clone("enhancement_0")
    hEnhancement_0.SetTitle("; Run Number; Enhancment Diamond/Amorph.")
    hEnhancement_90 = hPol_0.Clone("enhancement_90")
    hEnhancement_90.SetTitle("; Run Number; Enhancment Diamond/Amorph.")
    hEnhancement_45 = hPol_0.Clone("enhancement_45")
    hEnhancement_45.SetTitle("; Run Number; Enhancment Diamond/Amorph.")
    hEnhancement_135 = hPol_0.Clone("enhancement_135")
    hEnhancement_135.SetTitle("; Run Number; Enhancment Diamond/Amorph.")

    hSlope_0 = hPol_0.Clone("slope_0")
    hSlope_0.SetTitle("; Run Number; Enhancement Slope")
    hSlope_90 = hPol_0.Clone("slope_90")
    hSlope_90.SetTitle("; Run Number; Enhancement Slope")
    hSlope_45 = hPol_0.Clone("slope_45")
    hSlope_45.SetTitle("; Run Number; Enhancement Slope")
    hSlope_135 = hPol_0.Clone("slope_135")
    hSlope_135.SetTitle("; Run Number; Enhancement Slope")

    hEpeak_0 = hPol_0.Clone("Epeak_0")
    hEpeak_0.SetTitle("; Run Number; Enhancement Epeak")
    hEpeak_90 = hPol_0.Clone("Epeak_90")
    hEpeak_90.SetTitle("; Run Number; Enhancement Epeak")
    hEpeak_45 = hPol_0.Clone("Epeak_45")
    hEpeak_45.SetTitle("; Run Number; Enhancement Epeak")
    hEpeak_135 = hPol_0.Clone("Epeak_135")
    hEpeak_135.SetTitle("; Run Number; Enhancement Epeak")

    hPol_0.SetMarkerColor(1)
    hPol_0.SetMarkerStyle(20)
    hEnhancement_0.SetMarkerColor(1)
    hEnhancement_0.SetMarkerStyle(20)
    hSlope_0.SetMarkerColor(1)
    hSlope_0.SetMarkerStyle(20)
    hEpeak_0.SetMarkerColor(1)
    hEpeak_0.SetMarkerStyle(20)
    
    hPol_90.SetMarkerColor(2)
    hPol_90.SetMarkerStyle(20)
    hEnhancement_90.SetMarkerColor(2)
    hEnhancement_90.SetMarkerStyle(20)
    hSlope_90.SetMarkerColor(2)
    hSlope_90.SetMarkerStyle(20)
    hEpeak_90.SetMarkerColor(2)
    hEpeak_90.SetMarkerStyle(20)

    hPol_45.SetMarkerColor(4)
    hPol_45.SetMarkerStyle(20)
    hEnhancement_45.SetMarkerColor(4)
    hEnhancement_45.SetMarkerStyle(20)
    hSlope_45.SetMarkerColor(4)
    hSlope_45.SetMarkerStyle(20)
    hEpeak_45.SetMarkerColor(4)
    hEpeak_45.SetMarkerStyle(20)

    hPol_135.SetMarkerColor(6)
    hPol_135.SetMarkerStyle(20)
    hEnhancement_135.SetMarkerColor(6)
    hEnhancement_135.SetMarkerStyle(20)
    hSlope_135.SetMarkerColor(6)
    hSlope_135.SetMarkerStyle(20)
    hEpeak_135.SetMarkerColor(6)
    hEpeak_135.SetMarkerStyle(20)

    leg = ROOT.TLegend(0.6, 0.8, 0.8, 0.85)
    leg.SetMargin(0.2)
    leg.SetNColumns(4)
    leg.SetLineColor(0)
    leg.AddEntry(hPol_0, "0#circ")
    leg.AddEntry(hPol_90, "90#circ")
    leg.AddEntry(hPol_45, "45#circ")
    leg.AddEntry(hPol_135, "135#circ")

    # GET LISTS OF GOOD RUNS TO CHOOSE FROM
    RCDB_RUNS = db.select_runs("@is_dirc_production", run_min=MINRUN, run_max=MAXRUN+1)
    RCDB_RUN_NUMBERS = [ run.number for run in RCDB_RUNS ]

    # define paths for input
    MyInDataDir="polarization/spring2025/"

    a = ROOT.TCanvas("aa","aa",600,400)

    for RCDB_RUN in RCDB_RUNS:
        RUN = RCDB_RUN.number
        print(RUN)
        polAngle = RCDB_RUN.get_condition('polarization_angle').value

        filename = "%s/enhancementFit_%d.root" % (MyInDataDir, RUN)
        if 'RL' in str(RCDB_RUN.get_condition('radiator_type')):
            continue
        elif os.path.exists(filename):
            f = ROOT.TFile(filename)
            g = f.Get("Graph")
            h = f.Get("EnhancementData")
            if not g:
                continue

            h.SetMarkerStyle(20)
            if polAngle == 0.:
                h.SetMarkerColor(1);
            elif polAngle == 90.:
                h.SetMarkerColor(2);
            elif polAngle == 45.:
                h.SetMarkerColor(4);
            elif polAngle == 135.:
                h.SetMarkerColor(6);

            x = g.GetX()
            y = g.GetY()
            maxPol = ROOT.TMath.MaxElement(g.GetN(),y)
            maxPolPoint = ROOT.TMath.LocMax(g.GetN(),y)
            maxPolEgamma = x[maxPolPoint]
            maxEnhance = h.GetBinContent(h.GetMaximumBin())
            maxEnhanceErr = h.GetBinError(h.GetMaximumBin())
            print(maxEnhance)
            #if maxPol < 0.01:
            #    continue
            binID = hPol_0.FindBin(RUN)

            # fit slope
            maxEnhanceEgamma = maxPolEgamma*1000 #h.GetXaxis().GetBinCenter(h.GetMaximumBin())
            if maxEnhanceEgamma < 8825:
                maxEnhanceEgamma = 8825
            fitslope = ROOT.TF1("slopefit","pol1", maxEnhanceEgamma, maxEnhanceEgamma+250.)
            #print "%f %f" % (maxEnhanceEgamma, maxEnhanceEgamma+150.)
            h.Draw()
            h.Fit(fitslope,"Q","Q",maxEnhanceEgamma+40., maxEnhanceEgamma+220.)
            h.Fit(fitslope,"Q","Q",maxEnhanceEgamma+50., maxEnhanceEgamma+110.)
            if fitslope.GetParameter(1) > -0.05:
                print("%f %f" % (maxEnhanceEgamma, fitslope.GetParameter(1)))
		#g.Fit(fitslope,"","",maxEnhanceEgamma+50.,maxEnhanceEgamma+90.)

            slopePointMin = 999
            for i in range(0, h.GetXaxis().GetNbins()):
                if h.GetXaxis().GetBinCenter(i) < maxEnhanceEgamma+40. or h.GetXaxis().GetBinCenter(i) > maxEnhanceEgamma+200.:
                    continue
                slopeTemp = (h.GetBinContent(i+1) - h.GetBinContent(i-1))/(2*h.GetBinWidth(i));
                slopeTempErr = math.sqrt(h.GetBinError(i+1)*h.GetBinError(i+1) + h.GetBinError(i-1)*h.GetBinError(i-1))/(2*h.GetBinWidth(i));
                if slopeTemp < slopePointMin and slopeTemp < -0.01:
                    slopePointMin = slopeTemp
                    slopePointMinErr = slopeTempErr
            #slope = fitslope.GetParameter(1)
            #slopeErr = fitslope.GetParError(1)
            slope = slopePointMin
            slopeErr = slopePointMinErr

            t = ROOT.TText(10000, h.GetMaximum()*0.8,"Run %d" % RUN)
            t.Draw("same")
            a.Print("slopeFit/slopeFit_%d.ps" % RUN)

            if polAngle == 0.: 
                hPol_0.SetBinContent(binID, maxPol) 
                hEnhancement_0.SetBinContent(binID, maxEnhance)
                hEnhancement_0.SetBinError(binID, maxEnhanceErr)
                hSlope_0.SetBinContent(binID, slope)
                hSlope_0.SetBinError(binID, slopeErr)
                hEpeak_0.SetBinContent(binID, maxEnhanceEgamma/1000.)
            if polAngle == 90.: 
                hPol_90.SetBinContent(binID, maxPol) 
                hEnhancement_90.SetBinContent(binID, maxEnhance)
                hEnhancement_90.SetBinError(binID, maxEnhanceErr)
                hSlope_90.SetBinContent(binID, slope)
                hSlope_90.SetBinError(binID, slopeErr)
                hEpeak_90.SetBinContent(binID, maxEnhanceEgamma/1000.)
            if polAngle == 45.: 
                hPol_45.SetBinContent(binID, maxPol) 
                hEnhancement_45.SetBinContent(binID, maxEnhance)
                hEnhancement_45.SetBinError(binID, maxEnhanceErr)
                hSlope_45.SetBinContent(binID, slope)
                hSlope_45.SetBinError(binID, slopeErr)
                hEpeak_45.SetBinContent(binID, maxEnhanceEgamma/1000.)
            if polAngle == 135.: 
                hPol_135.SetBinContent(binID, maxPol) 
                hEnhancement_135.SetBinContent(binID, maxEnhance)
                hEnhancement_135.SetBinError(binID, maxEnhanceErr)
                hSlope_135.SetBinContent(binID, slope)
                hSlope_135.SetBinError(binID, slopeErr)
                hEpeak_135.SetBinContent(binID, maxEnhanceEgamma/1000.)

    # sum over enhancement and fit
    hEnhancementSum = hEnhancement_0.Clone()
    hEnhancementSum.Add(hEnhancement_90)
    hEnhancementSum.Add(hEnhancement_45)
    hEnhancementSum.Add(hEnhancement_135)

    fTune1 = ROOT.TF1("tune1","[0]", MINRUN, 41008)
    fTune2 = ROOT.TF1("tune2","[0]", 41040, 41172)
    fTune3 = ROOT.TF1("tune3","[0]", 41173, MAXRUN)
    hEnhancementSum.Fit(fTune1,"","",MINRUN,41008);
    hEnhancementSum.Fit(fTune2,"","",41040,41172);
    hEnhancementSum.Fit(fTune3,"","",41173,MAXRUN);
    
    c = ROOT.TCanvas("cc","cc",1000,400)
    ymin = 0.0
    ymax = 0.5
    if(label == "zoomIn"):
        ymin = 0.3
        ymax = 0.4
    hPol_0.SetMinimum(ymin)
    hPol_0.SetMaximum(ymax)
    hPol_0.Draw("p");
    hPol_90.Draw("p same")
    hPol_45.Draw("p same")
    hPol_135.Draw("p same")

    tune1 = ROOT.TLine(40934, ymin, 40934, ymax)
    tune1.Draw("same")
    tune2 = ROOT.TLine(41040, ymin, 41040, ymax)
    tune2.Draw("same")
    tune3 = ROOT.TLine(41172, ymin, 41172, ymax)
    tune3.Draw("same")
    tune4 = ROOT.TLine(41286, ymin, 41286, ymax)
    tune4.Draw("same")
    tune5 = ROOT.TLine(41837, ymin, 41837, ymax)
    tune5.Draw("same")
    coll2 = ROOT.TLine(42190, ymin, 42190, ymax)
    coll2.Draw("same")
    conv = ROOT.TLine(42202, ymin, 42202, ymax)
    conv.SetLineStyle(2)
    conv.Draw("same")
    coll3 = ROOT.TLine(42228, ymin, 42228, ymax)
    coll3.Draw("same")

    leg.Draw("same")
    c.Print("polVsRunNumber_%s.png" % label)

    d = ROOT.TCanvas("dd","dd",1000,400)
    ymin_enh = 0.0;
    ymax_enh = 8.0;
    if(label == "zoomIn"):
        ymin_enh = 3.0;
        ymax_enh = 7.0;
    hEnhancement_0.SetMinimum(ymin_enh)
    hEnhancement_0.SetMaximum(ymax_enh)
    hEnhancement_0.Draw("p")
    hEnhancement_90.Draw("same p")
    hEnhancement_45.Draw("same p")
    hEnhancement_135.Draw("same p")

    #fTune1.Draw("same")
    #fTune2.Draw("same")
    #fTune3.Draw("same")

    tune1_enh = ROOT.TLine(40934, ymin_enh, 40934, ymax_enh)
    tune1_enh.Draw("same")
    tune2_enh = ROOT.TLine(41040, ymin_enh, 41040, ymax_enh)
    tune2_enh.Draw("same")
    tune3_enh = ROOT.TLine(41172, ymin_enh, 41172, ymax_enh)
    tune3_enh.Draw("same")
    tune4_enh = ROOT.TLine(41286, ymin_enh, 41286, ymax_enh)
    tune4_enh.Draw("same")
    tune5_enh = ROOT.TLine(41837, ymin_enh, 41837, ymax_enh)
    tune5_enh.Draw("same")
    coll2_enh = ROOT.TLine(42190, ymin_enh, 42190, ymax_enh)
    coll2_enh.Draw("same")
    conv_enh = ROOT.TLine(42202, ymin_enh, 42202, ymax_enh)
    conv_enh.SetLineStyle(2)
    conv_enh.Draw("same")
    coll3_enh = ROOT.TLine(42228, ymin_enh, 42228, ymax_enh)
    coll3_enh.Draw("same")
    fall18_1_enh = ROOT.TLine(50720, ymin_enh, 50720, ymax_enh)
    fall18_1_enh.Draw("same")
    fall18_2_enh = ROOT.TLine(50726, ymin_enh, 50726, ymax_enh)
    fall18_2_enh.Draw("same")
    fall18_tune2_enh = ROOT.TLine(50990, ymin_enh, 50990, ymax_enh)
    fall18_tune2_enh.Draw("same")
    fall18_50um_enh = ROOT.TLine(51036, ymin_enh, 51036, ymax_enh)
    fall18_50um_enh.Draw("same")

    d.SetTicky(2);

    leg.Draw("same")
    d.Print("enhVsRunNumber_%s.png" % label)

    e = ROOT.TCanvas("ee","ee",1000,400)
    ymin_slope = -0.1;
    ymax_slope = -0.; #0.005;
    if(label == "zoomIn"):
        ymin_slope = -0.08;
        ymax_slope = -0.005;
    hSlope_0.SetMinimum(ymin_slope)
    hSlope_0.SetMaximum(ymax_slope)
    hSlope_0.Draw("p")
    hSlope_90.Draw("same p")
    hSlope_45.Draw("same p")
    hSlope_135.Draw("same p")

    tune1_slope = ROOT.TLine(40934, ymin_slope, 40934, ymax_slope)
    tune1_slope.Draw("same")
    tune2_slope = ROOT.TLine(41040, ymin_slope, 41040, ymax_slope)
    tune2_slope.Draw("same")
    tune3_slope = ROOT.TLine(41172, ymin_slope, 41172, ymax_slope)
    tune3_slope.Draw("same")
    tune4_slope = ROOT.TLine(41286, ymin_slope, 41286, ymax_slope)
    tune4_slope.Draw("same")
    tune5_slope = ROOT.TLine(41837, ymin_slope, 41837, ymax_slope)
    tune5_slope.Draw("same")
    coll2_slope = ROOT.TLine(42190, ymin_slope, 42190, ymax_slope)
    coll2_slope.Draw("same")
    conv_slope = ROOT.TLine(42202, ymin_slope, 42202, ymax_slope)
    conv_slope.SetLineStyle(2)
    conv_slope.Draw("same")
    coll3_slope = ROOT.TLine(42228, ymin_slope, 42228, ymax_slope)
    coll3_slope.Draw("same")
    fall18_1_slope = ROOT.TLine(50720, ymin_slope, 50720, ymax_slope)
    fall18_1_slope.Draw("same")
    fall18_2_slope = ROOT.TLine(50726, ymin_slope, 50726, ymax_slope)
    fall18_2_slope.Draw("same")
    fall18_tune2_slope = ROOT.TLine(50990, ymin_slope, 50990, ymax_slope)
    fall18_tune2_slope.Draw("same")
    fall18_50um_slope = ROOT.TLine(51036, ymin_slope, 51036, ymax_slope)
    fall18_50um_slope.Draw("same")

    e.SetTicky(2);

    leg.Draw("same")
    e.Print("slopeVsRunNumber_%s.png" % label)

    p = ROOT.TCanvas("pp","pp",1000,400)
    ymin_epeak = 0.0;
    ymax_epeak = 12.0;
    if(label == "zoomIn"):
        ymin_epeak = 8.5;
        ymax_epeak = 9.1;
    hEpeak_0.SetMinimum(ymin_epeak)
    hEpeak_0.SetMaximum(ymax_epeak)
    hEpeak_0.Draw("p")
    hEpeak_90.Draw("same p")
    hEpeak_45.Draw("same p")
    hEpeak_135.Draw("same p")

    tune1_epeak = ROOT.TLine(40934, ymin_epeak, 40934, ymax_epeak)
    tune1_epeak.Draw("same")
    tune2_epeak = ROOT.TLine(41040, ymin_epeak, 41040, ymax_epeak)
    tune2_epeak.Draw("same")
    tune3_epeak = ROOT.TLine(41172, ymin_epeak, 41172, ymax_epeak)
    tune3_epeak.Draw("same")
    tune4_epeak = ROOT.TLine(41286, ymin_epeak, 41286, ymax_epeak)
    tune4_epeak.Draw("same")
    tune5_epeak = ROOT.TLine(41837, ymin_epeak, 41837, ymax_epeak)
    tune5_epeak.Draw("same")
    coll2_epeak = ROOT.TLine(42190, ymin_epeak, 42190, ymax_epeak)
    coll2_epeak.Draw("same")
    conv_epeak = ROOT.TLine(42202, ymin_epeak, 42202, ymax_epeak)
    conv_epeak.SetLineStyle(2)
    conv_epeak.Draw("same")
    coll3_epeak = ROOT.TLine(42228, ymin_epeak, 42228, ymax_epeak)
    coll3_epeak.Draw("same")

    leg.Draw("same")
    p.Print("epeakVsRunNumber_%s.png" % label)

if __name__=="__main__":
    main(sys.argv[1:])
 
