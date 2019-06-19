#!/bin/csh
source /u/home/gleasonc/myenv_gluex
pwd
cd DIRECTORY
pwd
cp SCRIPTDIR/fit_BINFIT.py DIRECTORY/EtaPi_fit/BINFIT/fit.py
cd DIRECTORY/EtaPi_fit/BINFIT
pwd
python DIRECTORY/EtaPi_fit/BINFIT/fit.py
