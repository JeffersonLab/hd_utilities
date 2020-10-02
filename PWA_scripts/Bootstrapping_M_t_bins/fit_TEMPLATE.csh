#!/bin/csh
source /w/halld-scifs17exp/Mariana/PWA_challenge/AmpTools/sample1_mass_t_bins_bootstrap_SPD
pwd
cd DIRECTORY
pwd
cp SCRIPTDIR/fit_BINFIT.py DIRECTORY/EtaPi_fit/BINFIT/fit.py
cd DIRECTORY/EtaPi_fit/BINFIT
pwd
python DIRECTORY/EtaPi_fit/BINFIT/fit.py
