#!/usr/bin/env python

# Standard imports
from optparse import OptionParser
import os.path
import os
import sys
import subprocess
import glob
from array import *
from math import sqrt, exp

#Import ROOT modules
from ROOT import *

TOP_LEVEL_DATA_LOC =  "/w/halld-scshelf2101/home/jzarling/ForPeople/ForMadison/hd_root_files_2017_DATA/"
TOP_LEVEL_MC_LOC   =  "/w/halld-scshelf2101/home/jzarling/ForPeople/ForMadison/hd_root_files_2017_MC/"

def IsTDir(tobject):
    if("TDirectoryFile" in str(type(tobject))): return True
    else: return False

def main(argv):
    #Usage controls from OptionParser                                       
    parser_usage = ""
    parser = OptionParser(usage = parser_usage)
    (options, args) = parser.parse_args(argv)
    if(len(args) != 0):
        parser.print_help()
        return

    gROOT.SetBatch(True)


    #DATA
    loopcount_data = 0 # This variable counts which file number we are on
    filename_list_data = glob.glob(TOP_LEVEL_DATA_LOC+"/hd_root_0*.root") #file name is now an array variable that holds all the files we want
    NumberOfRuns_data = len(filename_list_data) # Number of Runs specific to this data sample
    
    #MC
    loopcount_mc = 0 # This variable counts which file number we are on
    filename_list_mc = glob.glob(TOP_LEVEL_MC_LOC+"/hd_root_0*.root") #file name is now an array variable that holds all the files we want
    NumberOfRuns_mc = len(filename_list_mc) # Number of Runs specific to this data sample

    #DATA
    subsubkey_array_data = []    
    pullname_array_data = []         
    tempnum_data=0
    f_data = TFile.Open(TOP_LEVEL_DATA_LOC+'/hd_root_030597.root')    #this is needed just to get this f bit to run; f will be redefined later
    topdir_keys_data = f_data.GetListOfKeys()
    for key_data in topdir_keys_data:
        key_string_data = key_data.GetTitle()
        print "Keyname: " + key_string_data #gets the keyname (ie. pi0pippim__B4_M7)
        if key_string_data == 'pi0pippim__B4_F1_M7':
            subdir_data = f_data.Get(key_string_data)
            subdir_keys_data = subdir_data.GetListOfKeys()
            for subkey_data in subdir_keys_data:
                subkey_string_data = subkey_data.GetTitle()
                print "Subkey: " + subkey_string_data #gets the subkey (ie. Hist_KinFitResults)
                print "Trying to access: " + key_string_data+"/"+subkey_string_data
                if subkey_string_data == 'Hist_KinFitResults':
                    subsubdir_data = f_data.Get(key_string_data+"/"+subkey_string_data)
                    print "subsubdir type: " +str(type(subsubdir_data))
                    if IsTDir(subsubdir_data): 
                        subsubdir_keys_data = subsubdir_data.GetListOfKeys()
                        for subsubkey_data in subsubdir_keys_data:
                            subsubkey_string_data = subsubkey_data.GetTitle()
                            print "Subsubkey title: " + subsubkey_string_data #gets the subsubkey (ie.Step0__Photon_Proton__Pi0_Pi+_Pi-_Proton)
                            if 'Step' in subsubkey_string_data:
                                tempnum_data=int(subsubkey_string_data[4])
                                subsubkey_array_data.insert(tempnum_data, subkey_string_data+'/'+subsubkey_string_data) ##UP TO HERE WORKS
                                
    #MC
    subsubkey_array_mc = []    
    pullname_array_mc = []         
    tempnum_mc=0
    f_mc = TFile.Open(TOP_LEVEL_MC_LOC+'/hd_root_030597.root')    #this is needed just to get this f bit to run; f will be redefined later
    topdir_keys_mc = f_mc.GetListOfKeys()
    for key_mc in topdir_keys_mc:
        key_string_mc = key_mc.GetTitle()
        print "Keyname: " + key_string_mc #gets the keyname (ie. pi0pippim__B4_M7)
        if key_string_mc == 'pi0pippim__pi0_gg__B4_F1_M7':
            subdir_mc = f_mc.Get(key_string_mc)
            subdir_keys_mc = subdir_mc.GetListOfKeys()
            for subkey_mc in subdir_keys_mc:
                subkey_string_mc = subkey_mc.GetTitle()
                print "Subkey: " + subkey_string_mc #gets the subkey (ie. Hist_KinFitResults)
                print "Trying to access: " + key_string_mc+"/"+subkey_string_mc
                if subkey_string_mc == 'Hist_KinFitResults':
                    subsubdir_mc = f_mc.Get(key_string_mc+"/"+subkey_string_mc)
                    print "subsubdir type: " +str(type(subsubdir_mc))
                    if IsTDir(subsubdir_mc): 
                        subsubdir_keys_mc = subsubdir_mc.GetListOfKeys()
                        for subsubkey_mc in subsubdir_keys_mc:
                            subsubkey_string_mc = subsubkey_mc.GetTitle()
                            print "Subsubkey title: " + subsubkey_string_mc #gets the subsubkey (ie.Step0__Photon_Proton__Pi0_Pi+_Pi-_Proton)
                            if 'Step' in subsubkey_string_mc:
                                tempnum_mc=int(subsubkey_string_mc[4])
                                subsubkey_array_mc.insert(tempnum_mc, subkey_string_mc+'/'+subsubkey_string_mc) ##UP TO HERE WORKS
        

        print 'subsubkey_array_data = ' 
        print subsubkey_array_data
        print 'subsubkey_array_mc = ' 
        print subsubkey_array_mc

    counter = 0

#DATA
    for partfilename_data in subsubkey_array_data:
        partfilename_mc = subsubkey_array_mc[counter]
        for var2 in ["Proton", "Pi+", "Pi-", "Photon"]:


#DATA
            run_vec_data = array('f')  # Will store all run numbers in an array. we will use this as the x-axis in a graph later
            pull_var_list_data = ["Pull_Px", "Pull_Py", "Pull_Pz", "Pull_E"]
            RMS_array_data = {var1 : array('f') for var1 in pull_var_list_data} # Stores all RMSs in an array in a dictionary. y-axis in graph
            mean_array_data = {var1 : array('f') for var1 in pull_var_list_data}# Stores all means in an array in a dictionary. y-axis in graph
            skewness_array_data = {var1 : array('f') for var1 in pull_var_list_data} 
            kurtosis_array_data = {var1 : array('f') for var1 in pull_var_list_data}
            for loopcount_data, filename_data in enumerate(filename_list_data):  # opening loop that collects all (x, y)
            
                f_data = TFile.Open(filename_data)    
                print("Current file:", filename_data)
                run_vec_data.append(int(filename_data[-10:-5]))
                for var1 in pull_var_list_data:
                    tempfilename_data = "pi0pippim__B4_F1_M7/"+partfilename_data+"/"+var2+"/"+var1
                    print tempfilename_data
                    if tempfilename_data:
                        pathname_data = "pi0pippim__B4_F1_M7/"+partfilename_data+"/"+var2+"/"+var1
                        h_data = f_data.Get("pi0pippim__B4_F1_M7/"+partfilename_data+"/"+var2+"/"+var1)
                        if hasattr(h_data, 'GetEntries'):

                            num_entries_data = h_data.GetEntries() # Number of times a value was entered into the histogram
                            mean_data = h_data.GetMean() # Avarage vlaue of this histogram (we wnat it to be 0)
                            RMS_data = h_data.GetRMS() #standard deviation
                            skewness_data = h_data.GetSkewness()
                            kurtosis_data = h_data.GetKurtosis()
                            mean_array_data[var1].append(float(mean_data))
                            RMS_array_data[var1].append(float(RMS_data))   #these will be used in the graphs
                            skewness_array_data[var1].append(float(skewness_data))
                            kurtosis_array_data[var1].append(float(kurtosis_data))
                            print("Number of entries in histogram", num_entries_data)
                            print("Histogram average:", mean_data)
                            print("Standard Deviation (RMS):", RMS_data)      # Prints out the results for this data file
                            print("Histogram skewness:", skewness_data)
                            print("Histogram kurtosis:", kurtosis_data)
                            print("Current file:", filename_data)
                            print("Current loop count:", loopcount_data)
                            print("The current variable is:", var1)
                            print("The current outter variable is:", var2)    
                            print("The current path name is: ", pathname_data)        
            

#MC
            run_vec_mc = array('f')  # Will store all run numbers in an array. we will use this as the x-axis in a graph later
            pull_var_list_mc = ["Pull_Px", "Pull_Py", "Pull_Pz", "Pull_E"]
            RMS_array_mc = {var1 : array('f') for var1 in pull_var_list_mc} # Stores all RMSs in an array in a dictionary. y-axis in graph
            mean_array_mc = {var1 : array('f') for var1 in pull_var_list_mc}# Stores all means in an array in a dictionary. y-axis in graph
            skewness_array_mc = {var1 : array('f') for var1 in pull_var_list_mc} 
            kurtosis_array_mc = {var1 : array('f') for var1 in pull_var_list_mc}
            for loopcount_mc, filename_mc in enumerate(filename_list_mc):  # opening loop that collects all (x, y)
            
                f_mc = TFile.Open(filename_mc)    
                print("Current file:", filename_mc)
                run_vec_mc.append(int(filename_mc[-10:-5]))
                for var1 in pull_var_list_mc:
                    tempfilename_mc = "pi0pippim__pi0_gg__B4_F1_M7/"+partfilename_mc+"/"+var2+"/"+var1
                    print tempfilename_mc
                    if tempfilename_mc:
                        pathname_mc = "pi0pippim__pi0_gg__B4_F1_M7/"+partfilename_mc+"/"+var2+"/"+var1
                        h_mc = f_mc.Get("pi0pippim__pi0_gg__B4_F1_M7/"+partfilename_mc+"/"+var2+"/"+var1)
                        if hasattr(h_mc, 'GetEntries'):

                            num_entries_mc = h_mc.GetEntries() # Number of times a value was entered into the histogram
                            mean_mc = h_mc.GetMean() # Avarage vlaue of this histogram (we wnat it to be 0)
                            RMS_mc = h_mc.GetRMS() #standard deviation
                            skewness_mc = h_mc.GetSkewness()
                            kurtosis_mc = h_mc.GetKurtosis()
                            mean_array_mc[var1].append(float(mean_mc))
                            RMS_array_mc[var1].append(float(RMS_mc))   #these will be used in the graphs
                            skewness_array_mc[var1].append(float(skewness_mc))
                            kurtosis_array_mc[var1].append(float(kurtosis_mc))
                            print("Number of entries in histogram", num_entries_mc)
                            print("Histogram average:", mean_mc)
                            print("Standard Deviation (RMS):", RMS_mc)      # Prints out the results for this data file
                            print("Histogram skewness:", skewness_mc)
                            print("Histogram kurtosis:", kurtosis_mc)
                            print("Current file:", filename_mc)
                            print("Current loop count:", loopcount_mc)
                            print("The current variable is:", var1)
                            print("The current outter variable is:", var2)    
                            print("The current path name is: ", pathname_mc)     
###Graphs

            if var2 == "Proton" or var2 == "Pi+" or var2 == "Pi-":
                if partfilename_data == 'Hist_KinFitResults/Step0__Photon_Proton__Pi0_Pi+_Pi-_Proton': #might need to change
                    var4 = ["Pull_Px"]#, "Pull_Xx"]
                    var5 = ["Pull_Py"]#, "Pull_Xy"]
                    var9 = ["Pull_Pz"]#, "Pull_Xz"]
                    for var6 in [0]:#,1]:
                    #this prints the graphs for x,y,z mean

                        if var2 == "Pi+" : graphsymbol = "#pi^{+}"
                        if var2 == "Pi-" : graphsymbol = "#pi^{-}"
                        if var2 == "Photon" : graphsymbol = "#gamma"
                        if var2 == "Proton" : graphsymbol = 'P^{+}'
                        if var6 == 0: graphname = 'Momentum'

                  
                        tempvar2=var2+graphname
                        c3 = TCanvas("c3","c3",2400,900)
                        c3.Divide(2,1)
                        ###Data
                        c3.cd(1)
                        gr1 = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data[var4[var6]]) # Size of arrays, followed     by x,y-axis
                        gr1.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                        gr1.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr1.SetTitle("Data: Mean Versus Run Number of "+ graphname + " " + graphsymbol)
                        gr1.GetXaxis().SetTitle("Run Number")
                        gr1.GetYaxis().SetTitle("Mean")
                        gr1.SetMarkerColor(kBlue)
                        gr1.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        gr2 = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        gr2.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr2.SetMarkerColor(kRed)
                        gr2.SetMarkerStyle(kFullCircle)
                        gr3 = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        gr3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr3.SetMarkerColor(kGreen+3)
                        gr3.SetMarkerStyle(kFullCircle)
                        ## Defining TLegend for graphs gr1, gr2, ...
                        legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legend.AddEntry(gr1,"Mean X","pl")
                        legend.AddEntry(gr2,"Mean Y","pl")
                        legend.AddEntry(gr3,"Mean Z","pl")
                        gr1.Draw("AP")
                        gr2.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        gr3.Draw("PSAME")
                        legend.Draw() # Doesn't need any arguments
                        ###mc
                        c3.cd(2)
                        gr_1 = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc[var4[var6]]) # Size of arrays, followed     by x,y-axis
                        gr_1.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                        gr_1.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr_1.SetTitle("MC: Mean Versus Run Number of "+ graphname + " " + graphsymbol)
                        gr_1.GetXaxis().SetTitle("Run Number")
                        gr_1.GetYaxis().SetTitle("Mean")
                        gr_1.SetMarkerColor(kBlue)
                        gr_1.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        gr_2 = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        gr_2.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr_2.SetMarkerColor(kRed)
                        gr_2.SetMarkerStyle(kFullCircle)
                        gr_3 = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        gr_3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr_3.SetMarkerColor(kGreen+3)
                        gr_3.SetMarkerStyle(kFullCircle)
                        ## Defining TLegend for graphs gr1, gr2, ...
                        legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legendd.AddEntry(gr1,"Mean X","pl")
                        legendd.AddEntry(gr2,"Mean Y","pl")
                        legendd.AddEntry(gr3,"Mean Z","pl")
                        gr_1.Draw("AP")
                        gr_2.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        gr_3.Draw("PSAME")
                        legendd.Draw() # Doesn't need any arguments
                        c3.SaveAs("splitplotMEAN%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png

                        #this prints the graphs for x,y,z RMS
                        c4 = TCanvas("c3","c3",2400,900)
                        c4.Divide(2,1)
                        ##DATA
                        c4.cd(1)
                        gr3 = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data[var4[var6]]) # Size of arrays, followed by x,y-axis
                        gr3.GetYaxis().SetRangeUser(0.7,1.3) #this is the y-axis range
                        gr3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr3.SetTitle("Data: RMS Versus Run Number of "+graphname+" "+ graphsymbol)
                        gr3.GetXaxis().SetTitle("Run Number")
                        gr3.GetYaxis().SetTitle("RMS")
                        gr3.SetMarkerColor(kBlue)
                        gr3.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        gr4 = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        gr4.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr4.SetMarkerColor(kRed)
                        gr4.SetMarkerStyle(kFullCircle)
                        gr5 = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        gr5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr5.SetMarkerColor(kGreen+3)
                        gr5.SetMarkerStyle(kFullCircle)
                        legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legend.AddEntry(gr3,"RMS X","pl")
                        legend.AddEntry(gr4,"RMS Y","pl")
                        legend.AddEntry(gr5,"RMS Z","pl")
                        gr3.Draw("AP")
                        gr4.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        gr5.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        legend.Draw() # Doesn't need any arguments
                        ##MC
                        c4.cd(2)
                        gr_3 = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc[var4[var6]]) # Size of arrays, followed by x,y-axis
                        gr_3.GetYaxis().SetRangeUser(0.7,1.3) #this is the y-axis range
                        gr_3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr_3.SetTitle("MC: RMS Versus Run Number of "+graphname+" "+ graphsymbol)
                        gr_3.GetXaxis().SetTitle("Run Number")
                        gr_3.GetYaxis().SetTitle("RMS")
                        gr_3.SetMarkerColor(kBlue)
                        gr_3.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        gr_4 = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        gr_4.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr_4.SetMarkerColor(kRed)
                        gr_4.SetMarkerStyle(kFullCircle)
                        gr_5 = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        gr_5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        gr_5.SetMarkerColor(kGreen+3)
                        gr_5.SetMarkerStyle(kFullCircle)
                        legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legendd.AddEntry(gr_3,"RMS X","pl")
                        legendd.AddEntry(gr_4,"RMS Y","pl")
                        legendd.AddEntry(gr_5,"RMS Z","pl")
                        gr_3.Draw("AP")
                        gr_4.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        gr_5.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        legendd.Draw() # Doesn't need any arguments
                        c4.SaveAs("splitplotRMS%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png

                        c6 = TCanvas("c3","c3",2400,900)
                        c6.Divide(2,1)
                        ##DATA
                        c6.cd(1)
                        grap1 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data[var4[var6]]) # Size of arrays, followed     by x,y-axis
                        grap1.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                        grap1.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap1.SetTitle("Data: Skewness Versus Run Number of "+ graphname + " " + graphsymbol)
                        grap1.GetXaxis().SetTitle("Run Number")
                        grap1.GetYaxis().SetTitle("Skewness")
                        grap1.SetMarkerColor(kBlue)
                        grap1.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        grap2 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        grap2.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap2.SetMarkerColor(kRed)
                        grap2.SetMarkerStyle(kFullCircle)
                        grap3 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        grap3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap3.SetMarkerColor(kGreen+3)
                        grap3.SetMarkerStyle(kFullCircle)
                        ## Defining TLegend for graphs gr1, gr2, ...
                        legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legend.AddEntry(grap1,"Skewness X","pl")
                        legend.AddEntry(grap2,"Skewness Y","pl")
                        legend.AddEntry(grap3,"Skewness Z","pl")
                        grap1.Draw("AP")
                        grap2.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        grap3.Draw("PSAME")
                        legend.Draw() # Doesn't need any arguments
                        ##mc
                        c6.cd(2)
                        grap_1 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc[var4[var6]]) # Size of arrays, followed     by x,y-axis
                        grap_1.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                        grap_1.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap_1.SetTitle("MC: Skewness Versus Run Number of "+ graphname + " " + graphsymbol)
                        grap_1.GetXaxis().SetTitle("Run Number")
                        grap_1.GetYaxis().SetTitle("Skewness")
                        grap_1.SetMarkerColor(kBlue)
                        grap_1.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        grap_2 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        grap_2.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap_2.SetMarkerColor(kRed)
                        grap_2.SetMarkerStyle(kFullCircle)
                        grap_3 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        grap_3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap_3.SetMarkerColor(kGreen+3)
                        grap_3.SetMarkerStyle(kFullCircle)
                        ## Defining TLegend for graphs gr1, gr2, ...
                        legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legendd.AddEntry(grap_1,"Skewness X","pl")
                        legendd.AddEntry(grap_2,"Skewness Y","pl")
                        legendd.AddEntry(grap_3,"Skewness Z","pl")
                        grap_1.Draw("AP")
                        grap_2.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        grap_3.Draw("PSAME")
                        legendd.Draw() # Doesn't need any arguments
                        c6.SaveAs("splitplotSKEWNESS%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png

                        c7 = TCanvas("c3","c3",2400,900)
                        c7.Divide(2,1)
                        ##DATA
                        c7.cd(1)
                        grap4 = TGraph(NumberOfRuns_data, run_vec_data, kurtosis_array_data[var4[var6]]) # Size of arrays, followed     by x,y-axis
                        grap4.GetYaxis().SetRangeUser(-1,0.3) #this is the y-axis range
                        grap4.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap4.SetTitle("Data: Kurtosis Versus Run Number of "+ graphname + " " + graphsymbol)
                        grap4.GetXaxis().SetTitle("Run Number")
                        grap4.GetYaxis().SetTitle("Kurtosis")
                        grap4.SetMarkerColor(kBlue)
                        grap4.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        grap5 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        grap5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap5.SetMarkerColor(kRed)
                        grap5.SetMarkerStyle(kFullCircle)
                        grap6 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        grap6.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap6.SetMarkerColor(kGreen+3)
                        grap6.SetMarkerStyle(kFullCircle)
                        ## Defining TLegend for graphs gr1, gr2, ...
                        legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legend.AddEntry(grap4,"Kurtosis X","pl")
                        legend.AddEntry(grap5,"Kurtosis Y","pl")
                        legend.AddEntry(grap6,"Kurtosis Z","pl")
                        grap4.Draw("AP")
                        grap5.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        grap6.Draw("PSAME")
                        legend.Draw() # Doesn't need any arguments
                        ##MC
                        c7.cd(2)
                        grap_4 = TGraph(NumberOfRuns_mc, run_vec_mc, kurtosis_array_mc[var4[var6]]) # Size of arrays, followed     by x,y-axis
                        grap_4.GetYaxis().SetRangeUser(-1,0.3) #this is the y-axis range
                        grap_4.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap_4.SetTitle("MC: Kurtosis Versus Run Number of "+ graphname + " " + graphsymbol)
                        grap_4.GetXaxis().SetTitle("Run Number")
                        grap_4.GetYaxis().SetTitle("Kurtosis")
                        grap_4.SetMarkerColor(kBlue)
                        grap_4.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                        grap_5 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc[var5[var6]]) # Size of arrays, followed     by x,y-axis
                        grap_5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap_5.SetMarkerColor(kRed)
                        grap_5.SetMarkerStyle(kFullCircle)
                        grap_6 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc[var9[var6]]) # Size of arrays, followed     by x,y-axis
                        grap_6.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                        grap_6.SetMarkerColor(kGreen+3)
                        grap_6.SetMarkerStyle(kFullCircle)
                        ## Defining TLegend for graphs gr1, gr2, ...
                        legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                        legendd.AddEntry(grap_4,"Kurtosis X","pl")
                        legendd.AddEntry(grap_5,"Kurtosis Y","pl")
                        legendd.AddEntry(grap_6,"Kurtosis Z","pl")
                        grap_4.Draw("AP")
                        grap_5.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                        grap_6.Draw("PSAME")
                        legendd.Draw() # Doesn't need any arguments
                        c7.SaveAs("splitplotKURTOSIS%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png



            if partfilename_mc == 'Hist_KinFitResults/Step1__Pi0__Photon_Photon' and var2 == "Photon": #might need to change
                tempvar = "Pull_E"+var2
                # the next bit makes the Mean Energy graph

                if var2 == "Pi+" : graphsymbol = "#pi^{+}"
                if var2 == "Pi-" : graphsymbol = "#pi^{-}"
                if var2 == "Photon" : graphsymbol = "#gamma"
                if var2 == "Proton" : graphsymbol = 'P^{+}'

                try:
                    c1 = TCanvas("c1","c1",2400,900) # creates a 1200x900 pixel drawing pad called c1. not sure if I have to add 'new' somewhere
                    c1.Divide(2,1)
                    ##Data
                    c1.cd(1)
                    gr = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data["Pull_E"]) # Size of arrays, followed     by x,y-axis
                    gr.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gr.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr.SetTitle("Data: Mean Versus Run Number of Pull_E "+ graphsymbol)
                    gr.GetXaxis().SetTitle("Run Number")
                    gr.GetYaxis().SetTitle("Mean")
                    gr.SetMarkerColor(kBlue)
                    gr.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gr,"Mean E","pl")
                    gr.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    legend.Draw() # Doesn't need any arguments
                    ##MC
                    c1.cd(2)
                    gr_ = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc["Pull_E"]) # Size of arrays, followed     by x,y-axis
                    gr_.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gr_.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr_.SetTitle("MC: Mean Versus Run Number of Pull_E "+ graphsymbol)
                    gr_.GetXaxis().SetTitle("Run Number")
                    gr_.GetYaxis().SetTitle("Mean")
                    gr_.SetMarkerColor(kBlue)
                    gr_.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gr_,"Mean E","pl")
                    gr_.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    legendd.Draw() # Doesn't need any arguments
                    c1.SaveAs("splitplotMEAN%s.pdf" % tempvar) #Saves in current directory, recommend saving asa pdf or png

                    # the next bit makes the Energy RMS graph
                    c2 = TCanvas("c2","c2",2400,900) # creates a 1200x900 pixel drawing pad called c1; not sure if I have to add 'new' somewhere
                    c2.Divide(2,1)
                    ##DATA
                    c2.cd(1)
                    gra = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data["Pull_E"]) # Size of arrays, followed by x,y-axis
                    gra.GetYaxis().SetRangeUser(0.7,1.3) #this is the y-axis range
                    gra.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra.SetTitle("Data: RMS Versus Run Number of Pull_E "+ graphsymbol)
                    gra.GetXaxis().SetTitle("Run Number")
                    gra.GetYaxis().SetTitle("RMS")
                    gra.SetMarkerColor(kBlue)
                    gra.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gra,"RMS E","pl")
                    gra.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instea    d of lines between points) Draws on Canvas c1
                    legend.Draw() # Doesn't need any arguments
                    ##MC
                    c2.cd(2)
                    gra_ = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc["Pull_E"]) # Size of arrays, followed by x,y-axis
                    gra_.GetYaxis().SetRangeUser(0.7,1.3) #this is the y-axis range
                    gra_.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_.SetTitle("MC: RMS Versus Run Number of Pull_E "+ graphsymbol)
                    gra_.GetXaxis().SetTitle("Run Number")
                    gra_.GetYaxis().SetTitle("RMS")
                    gra_.SetMarkerColor(kBlue)
                    gra_.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gra_,"RMS E","pl")
                    gra_.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instea    d of lines between points) Draws on Canvas c1
                    legendd.Draw() # Doesn't need any arguments
                    c2.SaveAs("splitplotRMS%s.pdf" % tempvar) #Saves in current directory, recommend saving as a pdf or png

                    # the next bit makes the Energy RMS graph
                    c8 = TCanvas("c2","c2",2400,900) # creates a 1200x900 pixel drawing pad called c1; not sure if I have to add 'new' somewhere
                    c8.Divide(2,1)
                    ##DATA
                    c8.cd(1)
                    gra4 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data["Pull_E"]) # Size of arrays, followed by x,y-axis
                    gra4.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gra4.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra4.SetTitle("Data: Skewness Versus Run Number of Pull_E "+ graphsymbol)
                    gra4.GetXaxis().SetTitle("Run Number")
                    gra4.GetYaxis().SetTitle("Skewness")
                    gra4.SetMarkerColor(kBlue)
                    gra4.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gra4,"Skewness E","pl")
                    gra4.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instea    d of lines between points) Draws on Canvas c1
                    legend.Draw() # Doesn't need any arguments
                    ##MC
                    c8.cd(2)
                    gra_4 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc["Pull_E"]) # Size of arrays, followed by x,y-axis
                    gra_4.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gra_4.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_4.SetTitle("MC: Skewness Versus Run Number of Pull_E "+ graphsymbol)
                    gra_4.GetXaxis().SetTitle("Run Number")
                    gra_4.GetYaxis().SetTitle("Skewness")
                    gra_4.SetMarkerColor(kBlue)
                    gra_4.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gra_4,"Skewness E","pl")
                    gra_4.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instea    d of lines between points) Draws on Canvas c1
                    legendd.Draw() # Doesn't need any arguments
                    c8.SaveAs("splitplotSKEWNESS%s.pdf" % tempvar) #Saves in current directory, recommend saving as a pdf or png
                
                    # the next bit makes the Energy RMS graph
                    c9 = TCanvas("c2","c2",2400,900) # creates a 1200x900 pixel drawing pad called c1; not sure if I have to add 'new' somewhere
                    c9.Divide(2,1)
                    ##Data
                    c9.cd(1)
                    gra5 = TGraph(NumberOfRuns_data, run_vec_data, kurtosis_array_data["Pull_E"]) # Size of arrays, followed by x,y-axis
                    gra5.GetYaxis().SetRangeUser(-0.7,0) #this is the y-axis range
                    gra5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0                    
                    gra5.SetTitle("Data: Kurtosis Versus Run Number of Pull_E "+ graphsymbol)
                    gra5.GetXaxis().SetTitle("Run Number")
                    gra5.GetYaxis().SetTitle("Kurtosis")
                    gra5.SetMarkerColor(kBlue)
                    gra5.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)                    
                    legend.AddEntry(gra5,"Kurtosis E","pl")
                    gra5.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instea    d of lines between points) Draws on Canvas c1
                    legend.Draw() # Doesn't need any arguments
                    ##Data
                    c9.cd(2)
                    gra_5 = TGraph(NumberOfRuns_mc, run_vec_mc, kurtosis_array_mc["Pull_E"]) # Size of arrays, followed by x,y-axis
                    gra_5.GetYaxis().SetRangeUser(-0.7,0) #this is the y-axis range
                    gra_5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0                    
                    gra_5.SetTitle("MC: Kurtosis Versus Run Number of Pull_E "+ graphsymbol)
                    gra_5.GetXaxis().SetTitle("Run Number")
                    gra_5.GetYaxis().SetTitle("Kurtosis")
                    gra_5.SetMarkerColor(kBlue)
                    gra_5.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)                    
                    legendd.AddEntry(gra_5,"Kurtosis E","pl")
                    gra_5.Draw("AP") # AP are options while drawing.  A = show axis, P = markers at points (instea    d of lines between points) Draws on Canvas c1
                    legendd.Draw() # Doesn't need any arguments
                    c9.SaveAs("splitplotKURTOSIS%s.pdf" % tempvar) #Saves in current directory, recommend saving as a pdf or png
                except: print "Photon energy couldn't be graphed"

                try:
                    #makes the photon Xxyz mean graph
                    tempvar2=var2+"Position"
                    c4 = TCanvas("c3","c3",2400,900)
                    c4.Divide(2,1)
                    ##DATA
                    c4.cd(1)
                    gr6 = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gr6.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gr6.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr6.SetTitle("Data: Mean Versus Run Number of Position " + graphsymbol)
                    gr6.GetXaxis().SetTitle("Run Number")
                    gr6.GetYaxis().SetTitle("Mean")
                    gr6.SetMarkerColor(kBlue)
                    gr6.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gr7 = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gr7.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr7.SetMarkerColor(kRed)
                    gr7.SetMarkerStyle(kFullCircle)
                    gr8 = TGraph(NumberOfRuns_data, run_vec_data, mean_array_data["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gr8.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr8.SetMarkerColor(kGreen+3)
                    gr8.SetMarkerStyle(kFullCircle)
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gr6,"Mean X","pl")
                    legend.AddEntry(gr7,"Mean Y","pl")
                    legend.AddEntry(gr8,"Mean Z","pl")
                    gr6.Draw("AP")
                    gr7.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gr8.Draw("PSAME")
                    legend.Draw() # Doesn't need any arguments
                    ##DATA
                    c4.cd(2)
                    gr_6 = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gr_6.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gr_6.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr_6.SetTitle("MC: Mean Versus Run Number of Position " + graphsymbol)
                    gr_6.GetXaxis().SetTitle("Run Number")
                    gr_6.GetYaxis().SetTitle("Mean")
                    gr_6.SetMarkerColor(kBlue)
                    gr_6.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gr_7 = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gr_7.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr_7.SetMarkerColor(kRed)
                    gr_7.SetMarkerStyle(kFullCircle)
                    gr_8 = TGraph(NumberOfRuns_mc, run_vec_mc, mean_array_mc["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gr_8.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gr_8.SetMarkerColor(kGreen+3)
                    gr_8.SetMarkerStyle(kFullCircle)
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gr_6,"Mean X","pl")
                    legendd.AddEntry(gr_7,"Mean Y","pl")
                    legendd.AddEntry(gr_8,"Mean Z","pl")
                    gr_6.Draw("AP")
                    gr_7.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gr_8.Draw("PSAME")
                    legendd.Draw() # Doesn't need any arguments
                    c4.SaveAs("splitplotMEAN%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png

                    #makes the photon Xxyz RMS graph
                    c5 = TCanvas("c3","c3",2400,900)
                    c5.Divide(2,1)
                    ##DATA
                    c5.cd(1)
                    gra1 = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gra1.GetYaxis().SetRangeUser(0.7,1.3) #this is the y-axis range
                    gra1.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra1.SetTitle("Data: RMS Versus Run Number of Position " + graphsymbol)
                    gra1.GetXaxis().SetTitle("Run Number")
                    gra1.GetYaxis().SetTitle("RMS")
                    gra1.SetMarkerColor(kBlue)
                    gra1.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gra2 = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gra2.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra2.SetMarkerColor(kRed)
                    gra2.SetMarkerStyle(kFullCircle)
                    gra3 = TGraph(NumberOfRuns_data, run_vec_data, RMS_array_data["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gra3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra3.SetMarkerColor(kGreen+3)
                    gra3.SetMarkerStyle(kFullCircle)
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gra1,"RMS X","pl")
                    legend.AddEntry(gra2,"RMS Y","pl")
                    legend.AddEntry(gra3,"RMS Z","pl")
                    gra1.Draw("AP")
                    gra2.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gra3.Draw("PSAME")
                    legend.Draw() # Doesn't need any arguments
                    ##MC
                    c5.cd(2)
                    gra_1 = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gra_1.GetYaxis().SetRangeUser(0.7,1.3) #this is the y-axis range
                    gra_1.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_1.SetTitle("MC: RMS Versus Run Number of Position " + graphsymbol)
                    gra_1.GetXaxis().SetTitle("Run Number")
                    gra_1.GetYaxis().SetTitle("RMS")
                    gra_1.SetMarkerColor(kBlue)
                    gra_1.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gra_2 = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gra_2.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_2.SetMarkerColor(kRed)
                    gra_2.SetMarkerStyle(kFullCircle)
                    gra_3 = TGraph(NumberOfRuns_mc, run_vec_mc, RMS_array_mc["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gra_3.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_3.SetMarkerColor(kGreen+3)
                    gra_3.SetMarkerStyle(kFullCircle)
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gra_1,"RMS X","pl")
                    legendd.AddEntry(gra_2,"RMS Y","pl")
                    legendd.AddEntry(gra_3,"RMS Z","pl")
                    gra_1.Draw("AP")
                    gra_2.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gra_3.Draw("PSAME")
                    legendd.Draw() # Doesn't need any arguments
                    c5.SaveAs("splitplotRMS%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png

                    #makes the photon Xxyz RMS graph
                    c10 = TCanvas("c3","c3",2400,900)
                    c10.Divide(2,1)
                    ##DATA
                    c10.cd(1)
                    gra5 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gra5.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gra5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra5.SetTitle("Data: Skewness Versus Run Number of Position " + graphsymbol)
                    gra5.GetXaxis().SetTitle("Run Number")
                    gra5.GetYaxis().SetTitle("Skewness")
                    gra5.SetMarkerColor(kBlue)
                    gra5.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gra6 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gra6.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra6.SetMarkerColor(kRed)
                    gra6.SetMarkerStyle(kFullCircle)
                    gra7 = TGraph(NumberOfRuns_data, run_vec_data, skewness_array_data["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gra7.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra7.SetMarkerColor(kGreen+3)
                    gra7.SetMarkerStyle(kFullCircle)
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gra5,"Skewness X","pl")
                    legend.AddEntry(gra6,"Skewness Y","pl")
                    legend.AddEntry(gra7,"Skewness Z","pl")
                    gra5.Draw("AP")
                    gra6.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gra7.Draw("PSAME")
                    legend.Draw() # Doesn't need any arguments
                    ##DATA
                    c10.cd(2)
                    gra_5 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gra_5.GetYaxis().SetRangeUser(-0.3,0.3) #this is the y-axis range
                    gra_5.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_5.SetTitle("MC: Skewness Versus Run Number of Position " + graphsymbol)
                    gra_5.GetXaxis().SetTitle("Run Number")
                    gra_5.GetYaxis().SetTitle("Skewness")
                    gra_5.SetMarkerColor(kBlue)
                    gra_5.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gra_6 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gra_6.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_6.SetMarkerColor(kRed)
                    gra_6.SetMarkerStyle(kFullCircle)
                    gra_7 = TGraph(NumberOfRuns_mc, run_vec_mc, skewness_array_mc["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gra_7.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_7.SetMarkerColor(kGreen+3)
                    gra_7.SetMarkerStyle(kFullCircle)
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gra_5,"Skewness X","pl")
                    legendd.AddEntry(gra_6,"Skewness Y","pl")
                    legendd.AddEntry(gra_7,"Skewness Z","pl")
                    gra_5.Draw("AP")
                    gra_6.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gra_7.Draw("PSAME")
                    legendd.Draw() # Doesn't need any arguments
                    c10.SaveAs("splitplotSKEWNESS%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png

                    #makes the photon Xxyz RMS graph
                    c11 = TCanvas("c3","c3",2400,900)
                    c11.Divide(2,1)
                    ##Data
                    c11.cd(1)
                    gra8 = TGraph(NumberOfRuns_data, run_vec_data, kurtosis_array_data["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gra8.GetYaxis().SetRangeUser(-0.6,-0.2) #this is the y-axis range
                    gra8.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra8.SetTitle("Data: Kurtosis Versus Run Number of Position " + graphsymbol)
                    gra8.GetXaxis().SetTitle("Run Number")
                    gra8.GetYaxis().SetTitle("Kurtosis")
                    gra8.SetMarkerColor(kBlue)
                    gra8.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gra9 = TGraph(NumberOfRuns_data, run_vec_data, kurtosis_array_data["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gra9.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra9.SetMarkerColor(kRed)
                    gra9.SetMarkerStyle(kFullCircle)
                    gra10 = TGraph(NumberOfRuns_data, run_vec_data, kurtosis_array_data["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gra10.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra10.SetMarkerColor(kGreen+3)
                    gra10.SetMarkerStyle(kFullCircle)
                    legend = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legend.AddEntry(gra8,"Kurtosis X","pl")
                    legend.AddEntry(gra9,"Kurtosis Y","pl")
                    legend.AddEntry(gra10,"Kurtosis Z","pl")
                    gra8.Draw("AP")
                    gra9.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gra10.Draw("PSAME")
                    legend.Draw() # Doesn't need any arguments
                    ##Data
                    c11.cd(2)
                    gra_8 = TGraph(NumberOfRuns_mc, run_vec_mc, kurtosis_array_mc["Pull_Xx"]) # Size of arrays, followed     by x,y-axis
                    gra_8.GetYaxis().SetRangeUser(-0.6,-0.2) #this is the y-axis range
                    gra_8.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_8.SetTitle("MC: Kurtosis Versus Run Number of Position " + graphsymbol)
                    gra_8.GetXaxis().SetTitle("Run Number")
                    gra_8.GetYaxis().SetTitle("Kurtosis")
                    gra_8.SetMarkerColor(kBlue)
                    gra_8.SetMarkerStyle(kFullCircle) # See https://root.cern.ch/doc/masterclassTAttMarker.html
                    gra_9 = TGraph(NumberOfRuns_mc, run_vec_mc, kurtosis_array_mc["Pull_Xy"]) # Size of arrays, followed     by x,y-axis
                    gra_9.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_9.SetMarkerColor(kRed)
                    gra_9.SetMarkerStyle(kFullCircle)
                    gra_10 = TGraph(NumberOfRuns_mc, run_vec_mc, kurtosis_array_mc["Pull_Xz"]) # Size of arrays, followed     by x,y-axis
                    gra_10.SetMarkerSize(1.2) # Makes marker size a bit bigger; standard is 1.0
                    gra_10.SetMarkerColor(kGreen+3)
                    gra_10.SetMarkerStyle(kFullCircle)
                    legendd = TLegend(0.7,0.7,0.9,0.9) #0.1 is lower limit of plot, 0.9 is upper limit (beyond on either side is labeling+whitespace)
                    legendd.AddEntry(gra_8,"Kurtosis X","pl")
                    legendd.AddEntry(gra_9,"Kurtosis Y","pl")
                    legendd.AddEntry(gra_10,"Kurtosis Z","pl")
                    gra_8.Draw("AP")
                    gra_9.Draw("PSAME") # AP are options while drawing.  A = show axis, P = markers at points (instead of lines between points) Draws on Canvas c1
                    gra_10.Draw("PSAME")
                    legendd.Draw() # Doesn't need any arguments
                    c11.SaveAs("splitplotKURTOSIS%s.pdf" % tempvar2) #Saves in current directory, recommend saving asa pdf or png
                except: print "Photon position couldn't be graphed"
        counter = counter +1

            
if __name__ == "__main__":
    main(sys.argv[1:])