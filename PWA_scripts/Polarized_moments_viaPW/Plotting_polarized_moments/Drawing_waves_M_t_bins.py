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
    if len(argv) == 5:
        #print ('The script is called: '+str(script))
        print ('Your waves are: '+argv[0])
        print ('Number of M bins is: '+argv[1])
        print ('Number of t bins is: '+argv[2])
        print ('Intensity results from fitting the original data are written in: '+argv[3])
        print ('Phase differences: '+argv[4])
        if len(argv[0].split())<2:
            sys.exit("You should specify more than one amplitude")
    else:
        sys.exit("You need to give 4 args..., usage: python script.py \"S0mi P1pl D1pl ...\" Nmassbins Ntbins fitresultfile \"P0P1diff \"")

    amps, NumBins, NumtBins, Fitresultfile, phasedifflist = argv
    N_amps=len(amps.split())
    NumBins=int(NumBins)
    NumtBins=int(NumtBins) 
    Numphasediff=len(phasedifflist.split())

    ## Output file to save the histograms in
    file_out=TFile('Drawing_waves.root','Recreate')
    
    
    ## column number for M value and each of the amplitude intensities and 
    ##corresponding errors
    column_m = 0
    column_t = 1  
    colomn_Waves=np.arange(2,2*N_amps+1,2)
    colomn_Waves_err=np.arange(3,2*N_amps+2,2)
    column_all_Waves=2*N_amps+2;
    column_all_Waves_err=2*N_amps+3;
    column_phasediff=np.arange(column_all_Waves_err+1,column_all_Waves_err+1+2*Numphasediff,2)
    column_phasediff_err=np.arange(column_all_Waves_err+2,column_all_Waves_err+1+2*Numphasediff,2)


    ## Arrays of  intensities from fitting the original data in different M bins
    orig_Wave=np.zeros((N_amps,NumBins))
    orig_all_Waves=np.zeros(NumBins)      
    ##Reading uncertainties from MINUIT
    orig_Wave_err_square=np.zeros((N_amps,NumBins))
    orig_all_Waves_err_square=np.zeros(NumBins)
    mass_bins=np.zeros(NumBins)
    phase_diff=np.zeros((Numphasediff,NumtBins,NumBins))
    phase_diff_err=np.zeros((Numphasediff,NumtBins,NumBins))


    ## Asigning values to arrays of intensities from fitting the original data in
    ##different M bins by reading from a file
    token_origdata_fitresults=open(Fitresultfile,'r')
    linestoken_orig=token_origdata_fitresults.readlines()
    j=0
    fit_res_colomns=0
    for x in linestoken_orig:
        
        fit_res_colomns=len(x.split())
        for waves in range(0,N_amps):

            orig_Wave[waves][int(j/NumtBins)]=orig_Wave[waves][int(j/NumtBins)]+float(x.split()[colomn_Waves[waves]])
            orig_Wave_err_square[waves][j/NumtBins]=orig_Wave_err_square[waves][j/NumtBins]+float(x.split()[colomn_Waves_err[waves]])*float(x.split()[colomn_Waves_err[waves]])
        orig_all_Waves[j/NumtBins]=orig_all_Waves[j/NumtBins]+float(x.split()[column_all_Waves])
        orig_all_Waves_err_square[j/NumtBins]=orig_all_Waves_err_square[j/NumtBins]+float(x.split()[column_all_Waves_err])*float(x.split()[column_all_Waves_err])
        mass_bins[j/NumtBins]=float(x.split()[column_m])

        
        for N_phase in range(0,Numphasediff):

            phase_diff[N_phase][int(j%NumtBins)][int(j/NumtBins)]=float(x.split()[column_phasediff[N_phase]])
            phase_diff_err[N_phase][int(j%NumtBins)][int(j/NumtBins)]=float(x.split()[column_phasediff_err[N_phase]])
            
        
        j=j+1
    if j != NumBins*NumtBins:
        sys.exit("The total number of bins are different from the one provided")



    orig_Wave_err=np.sqrt(orig_Wave_err_square)   
    orig_all_Waves_err=np.sqrt(orig_all_Waves_err_square)



    ##Combined intensities of the same amplitudes from diff. sums
    N_comb_waves=int(N_amps/2)
    orig_Wave_comb=np.zeros((N_comb_waves,NumBins))
    ##Combined uncertainty from MINUIT for the same wave from diff. sums 
    orig_Wave_err_square_comb=np.zeros((N_comb_waves,NumBins))

    for waves_comb in range(0,N_comb_waves):

        for M_bin in range(0,NumBins):

            orig_Wave_comb[waves_comb][M_bin]=orig_Wave[int(2*waves_comb)][M_bin]+orig_Wave[int(2*waves_comb+1)][M_bin]
            orig_Wave_err_square_comb[waves_comb][M_bin]=orig_Wave_err[int(2*waves_comb)][M_bin]*orig_Wave_err[int(2*waves_comb)][M_bin]+orig_Wave_err[int(2*waves_comb+1)][M_bin]*orig_Wave_err[int(2*waves_comb+1)][M_bin]

    orig_Wave_err_comb=np.sqrt(orig_Wave_err_square_comb)

    
        
    print('I am here')
   
   ##The graphs of intensities of different waves 
    error_M=np.zeros(NumBins)   
    grerr_list_Wave=[]
    for waves in range(0,int(N_amps/2)):
        grerr_list_Wave.append(TGraphErrors( NumBins, mass_bins, orig_Wave_comb[waves], error_M, orig_Wave_err_comb[waves]))
    
    c1 = TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    c1.Divide(3,2)
    c1.SetGrid()
    c1.GetFrame().SetFillColor( 21 )
    c1.GetFrame().SetBorderSize( 12 )
 
    for waves in range(0,int(N_amps/2)):
        grerr_list_Wave[waves].SetMarkerSize( .5 )
        grerr_list_Wave[waves].SetMarkerStyle( 20 )
        grerr_list_Wave[waves].SetName(amps.split()[2*waves])
        grerr_list_Wave[waves].SetTitle(amps.split()[2*waves])        
        if waves       <6:
            grerr_list_Wave[waves].SetMaximum(1.2*np.amax(orig_Wave_comb[waves]))
        else:
            grerr_list_Wave[waves].SetMaximum(1.2*np.amax(orig_all_Waves))
#        grerr_list_Wave[waves].SetMinimum(0.8*np.amin(orig_Wave_comb[waves]))         

        grerr_list_Wave[waves].SetMinimum(0.8*np.amin(orig_all_Waves))
        grerr_list_Wave[waves].Draw( 'AP' )
        c1.Print('Plots/Wave'+amps.split()[2*waves]+'.pdf')

    

    ##The graph of total intensity 
    c3 = TCanvas( 'c3', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    c3.Divide(3,2)
    c3.SetGrid()
    c3.GetFrame().SetFillColor( 21 )
    c3.GetFrame().SetBorderSize( 12 )
      
    gr_all = TGraphErrors( NumBins, mass_bins, orig_all_Waves, error_M, orig_all_Waves_err)
    gr_all.SetMarkerSize( .5 )
    gr_all.SetMarkerStyle( 20 )
    #gr_all.SetMaximum(max(x_max/2.5))
    #gr_all.SetMinimum(min(x_min/5))
    gr_all.SetMaximum(1.2*np.amax(orig_all_Waves))
    gr_all.SetMinimum(0.8*np.amin(orig_all_Waves))
    gr_all.SetTitle('All waves')
    gr_all.Draw( 'AP' )
    c3.Print("Plots/All_waves.pdf")

    
    #plotting phase differences

    grerr_list_phasediff=[]
    t_bin=0
    for N_phasediff in range(0,Numphasediff):

        grerr_list_phasediff.append(TGraphErrors( NumBins, mass_bins,phase_diff[N_phasediff][t_bin], error_M,phase_diff_err[N_phasediff][t_bin]))

    c4 = TCanvas( 'c4', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    
    c4.SetGrid()
    c4.GetFrame().SetFillColor( 21 )
    c4.GetFrame().SetBorderSize( 12 )

    for N_phasediff in range(0,Numphasediff):
        grerr_list_phasediff[N_phasediff].SetMarkerSize( .5 )
        grerr_list_phasediff[N_phasediff].SetMarkerStyle( 20 )
        grerr_list_phasediff[N_phasediff].SetName(phasedifflist.split()[N_phasediff])
        grerr_list_phasediff[N_phasediff].SetTitle(phasedifflist.split()[N_phasediff])

        grerr_list_phasediff[N_phasediff].SetMaximum(1.2*np.amax(phase_diff[N_phasediff][t_bin]))
        grerr_list_phasediff[N_phasediff].SetMinimum(0.8*np.amin(phase_diff[N_phasediff][t_bin]))
        grerr_list_phasediff[N_phasediff].Draw( 'AP' )
        c4.Print('Plots/Phasediff'+phasedifflist.split()[N_phasediff]+'.pdf')





    

    

    
    file_out.Write()
    file_out.Close()


if __name__ == "__main__":

   main(sys.argv[1:])

