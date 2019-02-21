#!/bin/csh
#$ -e WORKING_DIRECTORY/output.err
#$ -o WORKING_DIRECTORY/output.log


#Variables requiring redefining: WORKING_DIRECTORY

cd WORKING_DIRECTORY
# mkdir .asymm_moment_plot_dump

if(-e WORKING_DIRECTORY/MakeOmegaHists.C) then
	echo "Making histograms... this will may take a while"
	set command = ("root -b -q" WORKING_DIRECTORY/MakeOmegaHists.C)
	echo "Command to execute: " $command
	`$command` 
else
	echo "Skipping histogram generation. Continuing to histogram"
endif

if(-e WORKING_DIRECTORY/FitOmegaHists.C) then
	echo "Making histograms... this will may take a while"
	set command = ("root -b -q" WORKING_DIRECTORY/FitOmegaHists.C)
	echo "Command to execute: " $command
	`$command` 
else
	echo "Skipping histogram generation. Continuing to histogram"
endif

echo "DONE!"
