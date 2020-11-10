#!/bin/csh
source /w/halld-scifs17exp/Mariana/PWA_challenge/Pol_beam/eta_pi0_posMepsilon_SPD/fit_ampmatch_m_t_bins_bootsrapping_SPDG/Mar_env.csh
pwd
cd DIRECTORY
pwd
cp SCRIPTDIR/fit_BINFIT.py DIRECTORY/EtaPi_fit/BINFIT/fit.py
cd DIRECTORY/EtaPi_fit/BINFIT
pwd
python DIRECTORY/EtaPi_fit/BINFIT/fit.py
