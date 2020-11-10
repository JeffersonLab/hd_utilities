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
        print ('Number of M bins is: '+argv[0])
        print ('Number of t bins is: '+argv[1])
        print ('Number of bootstrap samples is:'+argv[2])
        print ('Directory for bootstrap results for different M bins is: '+argv[3])
        print ('Intensity results from fitting the original data are written in: '+argv[4])
       
    else:
        sys.exit("You need to give 5 args..., usage: python script.py Nmassbins Ntbins NBootstrapsamples Bindir fitresultfile")

    NumBins, NumtBins, kBootstrapPoints , Binfolderdir, Fitresultfile = argv
    NumBins=int(NumBins)
    NumtBins=int(NumtBins) 
    kBootstrapPoints=int(kBootstrapPoints)


    ## Output file to save the histograms in
    file_out=TFile('Drawing_Bootstrap_errors_moments.root','Recreate')
    

    with open(Fitresultfile,'r') as f:
        t=f.read()
        l=t.splitlines()
        amps=l[0]
        amps=amps.split()[2::2]   # start at 3rd element and take every other element
        N_amps=(len(l[0].split())-2)/2  #first two arguments in the line are M and t

    print(amps)
    print(N_amps)

    

    if N_amps<1:
        sys.exit("You should specify at least one moment")


    

    ## column number for M value and each of the amplitude intensities and 
    ##corresponding errors
    column_m = 0
    column_t = 1  
    colomn_moments=np.arange(2,2*N_amps+1,2)
    colomn_moments_err=np.arange(3,2*N_amps+2,2)
    mass_bins=np.zeros(NumBins)
        
    
    ## Arrays of  intensities from fitting the original data in different M bins
    orig_moment=np.zeros((N_amps,NumtBins,NumBins))
    ##Reading uncertainties from MINUIT
    orig_moment_err=np.zeros((N_amps,NumtBins,NumBins))
    
    
    ## Asigning values to arrays of moments from fitting the original fit amplitude calculation in
    ##different M and t bins by reading from a file
    token_origdata_fitresults=open(Fitresultfile,'r')
    linestoken_orig=token_origdata_fitresults.readlines()
    j=0
    fit_res_colomns=0
    for x in linestoken_orig[1:]:
        
        fit_res_colomns=len(x.split())
        mass_bins[j/NumtBins]=float(x.split()[column_m])

        for Moms in range(0,N_amps):

            orig_moment[Moms][int(j%NumtBins)][int(j/NumtBins)]=float(x.split()[colomn_moments[Moms]])
            orig_moment_err[Moms][int(j%NumtBins)][j/NumtBins]=float(x.split()[colomn_moments_err[Moms]])
       
        j=j+1
    if j != NumBins*NumtBins:
        sys.exit("The total number of bins are different from the one provided")

    

        
    ## Arrays of values for given moment from
    ##fitting different bootstrapping samples
    moments=np.zeros((N_amps,kBootstrapPoints))

    #uncertainty on the given moment for each M and t bin 
    std_moments=np.zeros((N_amps,NumtBins,NumBins))
     
        
    ## Histograms of moments for given mass and t bin and moment from different bootstraping samples
    
    h1_list_moments=[]
    i_Mbin=0
    for Mb in range(0,NumBins):
        h1_list_moments.append([])
       
        for tb in range(0,NumtBins):
            h1_list_moments[i_Mbin].append([])
        i_Mbin=i_Mbin+1




    
    #Defining histograms for moments from different bootstraping samples for a given aplitude and M bin
    for Bin in range(0,NumBins):

        #N_bins=int(300*orig_all_moments[Bin])
        N_bins=int(80000)
        x_min=int(-500)
                   
        for Bint in range(0,NumtBins):
            
            for Moms in range(0,N_amps):

                x_max=int(3*orig_moment[Moms][Bint][Bin])
                if Moms==11 and Bint==0 and Bin==5:
                    x_max=1000000
                h1_list_moments[Bin][Bint].append(TH1F('h1_boot_'+str(amps[Moms])+'_Mbin'+str(Bin+1)+'_tbin'+str(Bint+1),str(amps[Moms]),N_bins,x_min,x_max))
            

    
    ## Assigning values to arrays of values for given moment from fitting different bootstrapping samples
    ## by reading the values from files of each of the M and t bins
    ##(number of lines in the .txt file corresponds to number of bootstraping samples + 1)
    for bin in range(0,NumBins):
        for bint in range(0,NumtBins):
            token = open(Binfolderdir+'/bin_'+str(bin)+'_'+str(bint)+'/etapi_fit.txt','r')
            linestoken=token.readlines()
            i=0
        
            #Looping through lines corresponding to results from fitting different bottstraping samples
            for x in linestoken[1:]:  #First line has the names of arguments

                if len(x.split()) != fit_res_colomns:
                    sys.exit("Fit results and bootstraping results have different number of moments")
                            
                for Moms in range(0,N_amps):
                    moments[Moms][i]=x.split()[colomn_moments[Moms]]
                    h1_list_moments[bin][bint][Moms].Fill(moments[Moms][i])
               
                i=i+1
            token.close()
                   
            for Moms in range(0,N_amps):
                std_moments[Moms][bint][bin]=moments[Moms].std()
               



    
    
    print('I am here')
   
   ##The graphs of intensities of different Moms 
    error_M=np.zeros(NumBins)   
    grerr_list_moment=[]
    t_bin=0
    for Moms in range(0,N_amps):
        grerr_list_moment.append(TGraphErrors( NumBins, mass_bins, orig_moment[Moms][t_bin], error_M, std_moments[Moms][t_bin]))
    
    c1 = TCanvas( 'c1', 'A Simple Graph with error bars', 200, 10, 700, 500 )
    c1.Divide(3,2)
    c1.SetGrid()
    c1.GetFrame().SetFillColor( 21 )
    c1.GetFrame().SetBorderSize( 12 )
 
    for Moms in range(0,N_amps):
        grerr_list_moment[Moms].SetMarkerSize( .5 )
        grerr_list_moment[Moms].SetMarkerStyle( 20 )
        grerr_list_moment[Moms].SetName(amps[Moms])
        grerr_list_moment[Moms].SetTitle(amps[Moms])        
        grerr_list_moment[Moms].SetMaximum(1.2*np.amax(orig_moment[Moms]))

        grerr_list_moment[Moms].SetMinimum(0.8*np.amin(orig_moment[Moms]))         
        grerr_list_moment[Moms].Draw( 'AP' )
        grerr_list_moment[Moms].SetName(amps[Moms])
        grerr_list_moment[Moms].Write()
        c1.Print('Plots/Moment_'+amps[Moms]+'_boot.pdf')

    
    
   
    
    
      
    file_out.Write()
    file_out.Close()


if __name__ == "__main__":

   main(sys.argv[1:])

