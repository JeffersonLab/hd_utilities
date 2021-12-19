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
    if len(argv) == 6:
        #print ('The script is called: '+str(script))
        print ('Your waves are: '+argv[0])
        print ('Number of M bins is: '+argv[1])
        print ('Number of t bins is: '+argv[2])
        print ('Number of bootstrap samples is:'+argv[3])
        print ('Directory for bootstrap results for different M bins is: '+argv[4])
        print ('Intensity results from fitting the original data are written in: '+argv[5])
        if len(argv[0].split())<2:
            sys.exit("You should specify more than one amplitude")
    else:
        sys.exit("You need to give 6 args..., usage: python script.py \"S0mi P1pl D1pl ...\" Nmassbins NBootstrapsamples Bindir fitresultfile")

    amps, NumBins, NumtBins, kBootstrapPoints , Binfolderdir, Fitresultfile = argv
    N_amps=len(amps.split())
    NumBins=int(NumBins)
    NumtBins=int(NumtBins) 
    kBootstrapPoints=int(kBootstrapPoints)


    ## Output file to save the histograms in
    file_out=TFile('Drawing_Bootstrap_errors.root','Recreate')
    
    
    ## column number for M value and each of the amplitude intensities and 
    ##corresponding errors
    column_m = 0
    column_t = 1  
    colomn_Waves=np.arange(2,2*N_amps+1,2)
    colomn_Waves_err=np.arange(3,2*N_amps+2,2)
    column_all_Waves=2*N_amps+2;
    column_all_Waves_err=2*N_amps+3;
   
      
    
    ## Arrays of  intensities from fitting the original data in different M bins
    orig_Wave=np.zeros((N_amps,NumBins))
    orig_all_Waves=np.zeros(NumBins)      
    ##Reading uncertainties from MINUIT
    orig_Wave_err_square=np.zeros((N_amps,NumBins))
    orig_all_Waves_err_square=np.zeros(NumBins)
    
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
        j=j+1
    if j != NumBins*NumtBins:
        sys.exit("The total number of bins are different from the one provided")

    orig_Wave_err=np.sqrt(orig_Wave_err_square)   
    orig_all_Waves_err=np.sqrt(orig_all_Waves_err_square)

    
    
    ## Arrays of intensities for given amplitude from
    ##fitting different bootstrapping samples
    mass=np.zeros(kBootstrapPoints)
    Waves=np.zeros((N_amps,kBootstrapPoints))
    Waves_err=np.zeros((N_amps,kBootstrapPoints))
    all_Waves=np.zeros(kBootstrapPoints)
    all_Waves_err=np.zeros(kBootstrapPoints)
     
    
    
    ## Arrays of mean intensities from bootstrapping for different M bins
    mass_bins=np.zeros(NumBins)
    
    ## Arrays of std intensities from bootstrapping for different M bins
    std_Waves_square=np.zeros((N_amps,NumBins))
    std_all_Waves_square=np.zeros(NumBins)
    
    
    
    ## Histograms of intensities for given mass bin and amplitude from different bootstraping samples
    
    h1_list_Waves=[]
    h1_list_all=[]
    i_Mbin=0
    for Mb in range(0,NumBins):
        h1_list_Waves.append([])
        h1_list_all.append([])
        for tb in range(0,NumtBins):
            h1_list_Waves[i_Mbin].append([])
        i_Mbin=i_Mbin+1







    main = []
    #Defining histograms for intensities from different bootstraping samples for a given aplitude and M bin
    for Bin in range(0,NumBins):

        #N_bins=int(300*orig_all_Waves[Bin])
        N_bins=int(80000)
        x_min=int(-500)
        x_max=int(3*orig_all_Waves[Bin])

        outer = []

        
        for Bint in range(0,NumtBins):
            
            inner = [];
            for waves in range(0,N_amps):
                h1_list_Waves[Bin][Bint].append(TH1F('h1_boot_'+str(amps.split()[waves])+'_Mbin'+str(Bin+1)+'_tbin'+str(Bint+1),str(amps.split()[waves]),N_bins,x_min,x_max))
                #inner.append(TH1F('h1_boot_'+str(amps.split()[waves])+'_Mbin'+str(Bin+1)+'_tbin'+str(Bint+1),str(amps.split()[waves]),N_bins,x_min,x_max))
                #hh = TH1F('h1_boot_'+str(amps.split()[waves])+'_Mbin'+str(Bin+1)+'_tbin'+str(Bint+1),str(amps.split()[waves]),N_bins,x_min,x_max)
                #print( "N_bins = " + str(N_bins) + " x_min =  " + str(x_min) + "   x_max = " + str(x_max)  )
                #inner.append(hh)

            h1_list_all[Bin].append(TH1F('h1_boot_All_'+'Mbin'+str(Bin+1)+'_tbin'+str(Bint+1),'All waves',N_bins,x_min,x_max))
            
            #outer.append(inner);
        #main.append(outer)


    
    
    ## Assigning values to arrays of intensities for given amplitude from fitting different bootstrapping samples
    ## by reading the values from files of each of the M bins
    ##(number of lines in the .txt file corresponds to number of bootstraping samples)
    for bin in range(0,NumBins):
        for bint in range(0,NumtBins):
            token = open(Binfolderdir+'/bin_'+str(bin)+'_'+str(bint)+'/etapi_fit.txt','r')
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
                    h1_list_Waves[bin][bint][waves].Fill(Waves[waves][i])
                all_Waves[i]=x.split()[column_all_Waves]
                all_Waves_err[i]=x.split()[column_all_Waves_err]
                h1_list_all[bin][bint].Fill(all_Waves[i])
            

                i=i+1
            token.close()

            mass_bins[bin]=mass[0]
        
            for waves in range(0,N_amps):
                std_Waves_square[waves][bin]=std_Waves_square[waves][bin]+Waves[waves].std()*Waves[waves].std()
            std_all_Waves_square[bin]=std_all_Waves_square[bin]+all_Waves.std()*all_Waves.std()
        
    

    
    std_Waves=np.sqrt(std_Waves_square)
    std_all_Waves=np.sqrt(std_all_Waves_square)
    
    print('I am here')
   
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
    #gr_all.SetMaximum(max(x_max/2.5))
    #gr_all.SetMinimum(min(x_min/5))
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
    #gr_all_minuit .SetMaximum(max(x_max/2.5))                                                                                                                                                        
    #gr_all_minuit .SetMinimum(min(x_min/5))                                                                                                                                                          
    gr_all_minuit.SetMaximum(1.2*np.amax(orig_all_Waves))
    gr_all_minuit.SetMinimum(0.8*np.amin(orig_all_Waves))
    gr_all_minuit.SetTitle('All waves')
    gr_all_minuit.Draw( 'AP' )
    c3.Print("All_waves_minuit.pdf")






    
    
    file_out.Write()
    file_out.Close()


if __name__ == "__main__":

   main(sys.argv[1:])

