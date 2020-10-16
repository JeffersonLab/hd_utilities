#!/usr/bin/python


# Mariana Khachatryan
# marianak@jlab.org


import math
import sys
from ROOT import TCanvas, TGraphErrors
from ROOT import TH1F, TFile
import numpy as np
import array





def main(argv):
    
    print(len(argv))
    if len(argv) == 3:
        #print ('The script is called: '+str(script))
        print ('Number of M bins is: '+argv[0])
        print ('Number of t bins is: '+argv[1])
        print ('Moments are written in: '+argv[2])
        
    else:
        sys.exit("You need to give 3 args..., usage: python script.py  Nmassbins Ntbins fitresultfile")

    NumBins, NumtBins, Fitresultfile = argv
    NumBins=int(NumBins)
    NumtBins=int(NumtBins) 
   
    ## Output file to save the histograms in
    file_out=TFile('Drawing_moments.root','Recreate')
    

    with open(Fitresultfile,'r') as f:
        t=f.read()
        l=t.splitlines()
        momentlist=l[0]
        momentlist=momentlist[3:]
        momentlist=momentlist.split()[0::2]
        Nummoment=(len(l[0].split())-2)/2

    print(momentlist)
    print(Nummoment)
    
    ## column number for M value and each of the amplitude intensities and 
    ##corresponding errors
    column_m = 0
    column_t = 1  
    column_moment=np.arange(column_t+1,column_t+1+2*Nummoment,2)
    column_moment_err=np.arange(column_t+2,column_t+1+2*Nummoment,2)


    ##Moments  in different M and t bins
    mass_bins=np.zeros(NumBins)
    Mom=np.zeros((Nummoment,NumtBins,NumBins))
    Mom_err=np.zeros((Nummoment,NumtBins,NumBins))


    ## Asigning values to arrays of intensities from fitting the original data in
    ##different M bins by reading from a file
    token_origdata_fitresults=open(Fitresultfile,'r')
    linestoken_orig=token_origdata_fitresults.readlines()
    j=0
    fit_res_colomns=0
    for x in linestoken_orig[1:]:
        
        fit_res_colomns=len(x.split())
        mass_bins[j/NumtBins]=float(x.split()[column_m])
        
        for N_phase in range(0,Nummoment):

            Mom[N_phase][int(j%NumtBins)][int(j/NumtBins)]=float(x.split()[column_moment[N_phase]])
            Mom_err[N_phase][int(j%NumtBins)][int(j/NumtBins)]=float(x.split()[column_moment_err[N_phase]])
            
        
        j=j+1
    if j != NumBins*NumtBins:
        sys.exit("The total number of bins are different from the one provided")

    print('I am here')
   
   ##The graphs of intensities of different waves 
    error_M=np.zeros(NumBins)   
   
    
    #plotting phase differences

    grerr_list_moment=[]
    t_bin=0
    for N_moment in range(0,Nummoment):

        grerr_list_moment.append(TGraphErrors( NumBins, mass_bins,Mom[N_moment][t_bin], error_M,Mom_err[N_moment][t_bin]))
#        print(Mom_err[N_moment][t_bin])
    c4 = TCanvas( 'c4', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    
    c4.SetGrid()
    c4.GetFrame().SetFillColor( 21 )
    c4.GetFrame().SetBorderSize( 12 )

    for N_moment in range(0,Nummoment):
        grerr_list_moment[N_moment].SetMarkerSize( .5 )
        grerr_list_moment[N_moment].SetMarkerStyle( 20 )
        grerr_list_moment[N_moment].SetName(momentlist[N_moment])
        grerr_list_moment[N_moment].SetTitle(momentlist[N_moment])

        grerr_list_moment[N_moment].SetMaximum(1.2*np.amax(Mom[N_moment][t_bin]))
        grerr_list_moment[N_moment].SetMinimum(0.8*np.amin(Mom[N_moment][t_bin]))
        grerr_list_moment[N_moment].Draw( 'AP' )
        grerr_list_moment[N_moment].SetName(momentlist[N_moment])
        grerr_list_moment[N_moment].Write()
        print(momentlist[N_moment])
        c4.Print('Plots/Moments'+momentlist[N_moment]+'.pdf')





    

    

    
    file_out.Write()
    file_out.Close()


if __name__ == "__main__":

   main(sys.argv[1:])

