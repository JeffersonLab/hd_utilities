#!/bin/tcsh

ssh cori.nersc.gov 'cd builds/accounting ; sacct --format=JobID%15,Submit,Start,End,NCPUS,CPUTimeRaw,ResvCPURAW,MaxRSS,JobName%30,ExitCode,MaxDiskRead -S "2018-08-21 11:00" > slurm.dat'

scp cori.nersc.gov:builds/accounting/slurm.dat .

./slurm2sqlite.py

foreach m ( Njobs_vs_time.C latency_vs_time.C cpu_vs_time.C )
	root -l -q $m
end

# If on Mac OS X then open the PNG files automatically
if ( $OSTYPE == 'darwin' ) then
	open *.png
endif
