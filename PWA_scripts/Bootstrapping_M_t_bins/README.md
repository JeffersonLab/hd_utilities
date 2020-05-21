Most of the scripts and codes in this directory are based on the ones for bootstrapping in M bins that can be found in the 
following Github repository:
https://github.com/JeffersonLab/hd_utilities/tree/master/PWA_scripts/Bootstrap

They have just been adopted to do bootstraping in M (invariant mass) and t (momentum transfer) bins and a new python code called
"Drawing_Bootstrap_errors_M_t_bins.py" has been developed for plotting the uncertainties from bootstraping.

1. divideData_Mass_t_Bins_Bootstrap.py will divide your data, generated and accepted Monte Carlo .root data files into 
different M_t bins and will create a n EtaPi_fit directory with bin_M_t subdirectories to put the .root files in. Inside each bin_M_t 
directory it will also create Bootstrap directories. You should edit the code to change the paths for .root files and the number of bootstraps and M and t bins.
2.To submit one job taht does the fiiting of your bootstrapping samples for a given M_t bin, copy fit_TEMPLATE.csh, fit_TEMPLATE.py and 
script_Fit.py to created  EtaPi_fit/script/ directory. Edit script_Fit.py to do appropriate changes,like number of M, t bins, Bootstraps, name of the workflow,
path of the baseDir, which should point to the directory above EtaPi_fit. Also edit fit_TEMPLATE.csh to set up your envirement.
3. Create your workflow "swif create -workflow WORKFLOW". "WORKFLOW" needs to be the same as the name in script_Fit.py 
4. Run script_Fit.py to submit one job per mass bin to the ifarm. 
5.To run the jobs do "swif run -workflow WORKFLOW".
6. After your jobs are finished running (check status by "swif list").
7. Copy /w/halld-scifs17exp/Mariana/halld_sim/src/programs/AmplitudeAnalysis/Bootstrap_plot_etapi_delta_SPD_mass_t_bins/Bootstrap_plot_etapi_delta_SPD_mass_t_bins.cc
to your version of halld software and compile it to run in the GlueX framework.
This program will create a text files in each mass and t bin that contains the intensity and error from minut of each partial 
wave as well as total intensity and its error. Each of this .txt files has lines equal to the number of bootstraping samples and the first two colomns show the value of M and t for corresponding bin, and the rest of the colomns correspond to the wave intensityies and their uncertainties. The last two colomn correspond to total intensity.
8. Finally copy the Drawing_Bootstrap_errors_M_t_bins.py to the directory above EtaPi_Fit and run it to plot the results from fiitng the original data in different M and t bins, with the uncertainties from bootstraping. It takes 6 arguments, the waveset used in the fitting, number of M bins, number of t bins, the path of the directory that contains directories corresponding to M_t bins, and the path for the text file with fit results from fitting the original data. In later again first two colomns have the M and t values, and the numbr of lines are N_M_bin*N_t_bin. You can run this code the following way. 

python Drawing_Bootstrap_errors_M_t_bins.py "S0mi P0mi P1mi D0mi D1mi P1pl D1pl" 45 4 100 EtaPi_fit/ etapi_fit.txt

This code will also save the intensity distributions from different bootstrapping samples for a given wave and M_t bin in a .root file.
The order of the waves given as an argument to the code should match that with the one that has been used to write the original and bootstrapping fit results into the text files.

