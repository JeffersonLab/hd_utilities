#!/usr/bin/python

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
        print ('Number of bootstrap samples is:'+argv[2])
        print ('Directory for bootstrap results for different M bins is: '+argv[3])
        print ('Intensity results from fitting the original data are written in: '+argv[4])
        if len(argv[0].split())<2:
            sys.exit("You should specify more than one amplitude")
    else:
        sys.exit("You need to give 5 args..., usage: python script.py \"S0mi P1pl D1pl ...\" Nmassbins NBootstrapsamples Bindir fitresultfile")

    amps, NumBins, kBootstrapPoints , Binfolderdir, Fitresultfile = argv
    N_amps=len(amps.split())
    NumBins=int(NumBins)
    kBootstrapPoints=int(kBootstrapPoints)


    ## Output file to save the histograms in
    file_out=TFile('Drawing_Bootstrap_errors.root','Recreate')
    
    
    ## column number for M value and each of the amplitude intensities and 
    ##corresponding errors
    column_m = 0;
    colomn_Waves=np.arange(1,2*N_amps,2)
    colomn_Waves_err=np.arange(2,2*N_amps+1,2)
    column_all_Waves=2*N_amps+1;
    column_all_Waves_err=2*N_amps+2;
   
   
    
    ## Arrays of  intensities from fitting the original data in different M bins
    orig_Wave=np.zeros((N_amps,NumBins))
    orig_all_Waves=np.zeros(NumBins)      
    ##Reading uncertainties from MINUIT
    orig_Wave_err=np.zeros((N_amps,NumBins))
    orig_all_Waves_err=np.zeros(NumBins)    
    
    
    ## Asigning values to arrays of intensities from fitting the original data in d
    ##ifferent M bins by reading from a file
    token_origdata_fitresults=open(Fitresultfile,'r')
    linestoken_orig=token_origdata_fitresults.readlines()
    j=0
    fit_res_colomns=0
    for x in linestoken_orig:
        
        fit_res_colomns=len(x.split())
        for waves in range(0,N_amps):
            orig_Wave[waves][j]=x.split()[colomn_Waves[waves]]
            orig_Wave_err[waves][j]=x.split()[colomn_Waves_err[waves]]
        orig_all_Waves[j]=x.split()[column_all_Waves]
        orig_all_Waves_err[j]=x.split()[column_all_Waves_err]
        j=j+1
    if j != NumBins:
        sys.exit("Mass bins in fit results are different from the one provided")
    
    
    
    
    ## Arrays of intensities for given amplitude from
    ##fitting different bootstrapping samples
    mass=np.zeros(kBootstrapPoints)
    Waves=np.zeros((N_amps,kBootstrapPoints))
    Waves_err=np.zeros((N_amps,kBootstrapPoints))
    all_Waves=np.zeros(kBootstrapPoints)
    all_Waves_err=np.zeros(kBootstrapPoints)
     
    
    
    ## Arrays of mean intensities from bootstrapping for different M bins
    mass_bins=np.zeros(NumBins)
    mean_Waves=np.zeros((N_amps,NumBins))
    mean_all_Waves=np.zeros(NumBins)
    

 
    ## Arrays of std intensities from bootstrapping for different M bins
    std_Waves=np.zeros((N_amps,NumBins))
    std_all_Waves=np.zeros(NumBins)
    
    
    
    ## Histograms of intensities for given mass bin and amplitude from different bootstraping samples
    h1_list_Waves=[]
    for waves in range(0,N_amps):
        h1_list_Waves.append([])
    h1_list_all=[]
   
   
   



    #Defining histograms for intensities from different bootstraping samples for a given aplitude and M bin
    for bin in range(0,NumBins):
        N_bins=int(10000)
        x_min=-500
        x_max=int(3*orig_all_Waves[bin])
        
        for waves in range(0,N_amps):
            h1_list_Waves[waves].append(TH1F('h1_boot_'+amps.split()[waves]+'_Mbin'+str(bin+1),amps.split()[waves],N_bins,x_min,x_max))
        h1_list_all.append(TH1F('h1_boot_All_'+'Mbin'+str(bin+1),'All waves',N_bins,x_min,x_max))



    
    
    ## Assigning values to arrays of intensities for given amplitude from fitting different bootstrapping samples
    ## by reading the values from files of each of the M bins
    ##(number of lines in the .txt file corresponds to number of bootstraping samples)
    for bin in range(0,NumBins):
        token = open(Binfolderdir+'/bin_'+str(bin)+'/etapi_fit.txt','r')
        linestoken=token.readlines()
        i=0
        
        #Looping through lines corresponding to results from fitting different bottstraping samples
        for x in linestoken:
            
            if len(x.split()) != fit_res_colomns:
                sys.exit("Fit results and bootstraping results have different number of waves")
            mass[i]=x.split()[column_m]
            
            for waves in range(0,N_amps):
                Waves[waves][i]=x.split()[colomn_Waves[waves]]
                Waves_err[waves][i]=x.split()[colomn_Waves_err[waves]]
            all_Waves[i]=x.split()[column_all_Waves]
            all_Waves_err[i]=x.split()[column_all_Waves_err]
            
            for waves in range(0,N_amps):
                h1_list_Waves[waves][bin].Fill(Waves[waves][i])
            h1_list_all[bin].Fill(all_Waves[i])
            i=i+1
        token.close()

        mass_bins[bin]=mass[0]
        
        for waves in range(0,N_amps):
            mean_Waves[waves][bin]=Waves[waves].mean()
            std_Waves[waves][bin]=Waves[waves].std()
        mean_all_Waves[bin]=all_Waves.mean()
        std_all_Waves[bin]=all_Waves.std()

        
    

    
    
    
   
   ##The graphs of intensities of different waves 
    error_M=np.zeros(NumBins)   
    grerr_list_Wave=[]
    for waves in range(0,N_amps):
        grerr_list_Wave.append(TGraphErrors( NumBins, mass_bins, orig_Wave[waves], error_M, std_Waves[waves]))
    
    c1 = TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    c1.Divide(3,2)
    c1.SetGrid()
    c1.GetFrame().SetFillColor( 21 )
    c1.GetFrame().SetBorderSize( 12 )
 
    for waves in range(0,N_amps):
        grerr_list_Wave[waves].SetMarkerSize( .5 )
        grerr_list_Wave[waves].SetMarkerStyle( 20 )
        grerr_list_Wave[waves].SetName(amps.split()[waves])
        grerr_list_Wave[waves].SetTitle(amps.split()[waves])        
        grerr_list_Wave[waves].SetMaximum(1.2*np.amax(orig_Wave[waves]))                                                                                          
        grerr_list_Wave[waves].SetMinimum(0.8*np.amin(orig_Wave[waves]))         
        grerr_list_Wave[waves].Draw( 'AP' )
        c1.Print('Wave'+amps.split()[waves]+'.pdf')

    
    
    ##The graph of total intensity 
    c3 = TCanvas( 'c3', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    c3.Divide(3,2)
    c3.SetGrid()
    c3.GetFrame().SetFillColor( 21 )
    c3.GetFrame().SetBorderSize( 12 )
      
    gr_all = TGraphErrors( NumBins, mass_bins, orig_all_Waves, error_M, std_all_Waves)
    gr_all.SetMarkerSize( .5 )
    gr_all.SetMarkerStyle( 20 )
    gr_all.SetMaximum(1.2*np.amax(orig_all_Waves))
    gr_all.SetMinimum(0.8*np.amin(orig_all_Waves))
    gr_all.SetTitle('All waves')
    gr_all.Draw( 'AP' )
    c3.Print("All_waves.pdf")

    
    
    ## Plotting with MINUIT uncertainties



    grerr_list_Wave_minuit=[]
    for waves in range(0,N_amps):
        grerr_list_Wave_minuit.append(TGraphErrors( NumBins, mass_bins, orig_Wave[waves], error_M, orig_Wave_err[waves]))

    c4 = TCanvas( 'c4', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    #c4.Divide(3,2)
    c4.SetGrid()
    c4.GetFrame().SetFillColor( 21 )
    c4.GetFrame().SetBorderSize( 12 )

    for waves in range(0,N_amps):
        grerr_list_Wave_minuit[waves].SetMarkerSize( .5 )
        grerr_list_Wave_minuit[waves].SetMarkerStyle( 20 )
        grerr_list_Wave_minuit[waves].SetName(amps.split()[waves])
        grerr_list_Wave_minuit[waves].SetTitle(amps.split()[waves])
        grerr_list_Wave_minuit[waves].SetMaximum(1.2*np.amax(orig_Wave[waves]))
        grerr_list_Wave_minuit[waves].SetMinimum(0.8*np.amin(orig_Wave[waves]))
        grerr_list_Wave_minuit[waves].Draw( 'AP' )
        c4.Print('Wave'+amps.split()[waves]+'minuit.pdf')




     ##The graph of total intensity                                                                                                                                                            
    c5 = TCanvas( 'c5', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    #c5.Divide(3,2)
    c5.SetGrid()
    c5.GetFrame().SetFillColor( 21 )
    c5.GetFrame().SetBorderSize( 12 )

    gr_all_minuit = TGraphErrors( NumBins, mass_bins, orig_all_Waves, error_M,orig_all_Waves_err)
    gr_all_minuit.SetMarkerSize( .5 )
    gr_all_minuit.SetMarkerStyle( 20 )
    gr_all_minuit.SetMaximum(1.2*np.amax(orig_all_Waves))
    gr_all_minuit.SetMinimum(0.8*np.amin(orig_all_Waves))
    gr_all_minuit.SetTitle('All waves')
    gr_all_minuit.Draw( 'AP' )
    c3.Print("All_waves_minuit.pdf")






    
    
    file_out.Write()
    file_out.Close()


if __name__ == "__main__":
   main(sys.argv[1:])

