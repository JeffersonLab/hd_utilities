# Introduction 
This directory contains scripts for submitting batch jobs with MPI (and GPU).  They are meant to determine how your fit speed improves (or not) by adding additioinal resources
* submit.py -- submits MPI jobs with various # of cores
* submitGPU.py -- submits MPI+GPU jobs with various # of GPUs
* plotBenchmark.C -- plots fit speed for the benchmark jobs

# Required user modifications
* In submit.py and submitGPU.py you should replace the MyEnv, MyConfig and MyOutDir variables with your own environment setup script, AmpTools fit configuration and output directory location
* You can change the MyCPU or MyGPU list to contain different amounts of cores for your benchmark if you want to test with more or fewer cores/GPUs

# Notes:
* These fits require using the MPI compiled version of AmpTools and halld_sim, see https://halldweb.jlab.org/wiki/index.php/HOWTO_use_AmpTools_on_the_JLab_farm_with_MPI for more details
* Only run the GPU version of the fitter if your fit utilizes a GPU accelerated amplitude and you've compiled that amplitude with the GPU (CUDA) libraries on one of the sciml nodes, see https://halldweb.jlab.org/wiki/index.php/HOWTO_use_AmpTools_on_the_JLab_farm_GPUs for more details
* Some of these default benchmarks require many CPUs or GPUs and may take some time for those nodes to become available on the ifarm/sciml nodes, so be patient.