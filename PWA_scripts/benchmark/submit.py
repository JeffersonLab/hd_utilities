#!/usr/bin/env python

import sys
import os
import subprocess
import math
import pwd
from optparse import OptionParser

########################################################## MAIN ##########################################################
def main(argv):

	# SLURM INFO (see options at https://scicomp.jlab.org/scicomp/slurmJob/slurmInfo)
	PARTITION    = "ifarm"   
        CONSTRAINT   = "farm19"   
        TIMELIMIT  = "24:00:00"   # Max walltime
        MyCPUs = [1,2,4,8,16,32,64,96,128,192] # List of CPU cores to use in benchmark fits

        # User provided environment, fit configuration and options
	MyEnv = "/work/halld2/home/jrsteven/analysisGluexI/builds/setup_gluex_scanParam.csh"
        MyConfig = "/work/halld2/home/jrsteven/forBenchmark/benchmark.cfg"
        MyMPIOpt = "--mca btl_openib_allow_ib 1"
        MyFitOpt = "-m 100000 -r 5"
	MyOutDir = "/volatile/halld/home/" + pwd.getpwuid( os.getuid() )[0] + "/benchmark/"

        # LOOP OVER # OF CORES FOR BENCHMARK
        for nCores in MyCPUs:
                # nodes used in fit (for every 64 CPUs allow an additional node)
                nNodes = nCores/64 + 1
                
                # create output directories
                MyRunningDir = MyOutDir + "cpu%03d" % nCores
                MyLogOutDir = MyRunningDir + "/log"
                if not os.path.exists(MyOutDir):
                        os.makedirs(MyOutDir)
                if not os.path.exists(MyRunningDir):
                        os.makedirs(MyRunningDir)
                if not os.path.exists(MyLogOutDir):
                        os.makedirs(MyLogOutDir)

                # create slurm submission script
                slurmOut = open("tempSlurm.txt",'w')
                slurmOut.write("#!/bin/csh \n")
                slurmOut.write("#SBATCH --nodes=%d \n" % nNodes)
                slurmOut.write("#SBATCH --partition=%s \n" % PARTITION) 
                slurmOut.write("#SBATCH --constraint=%s \n" % CONSTRAINT)
                slurmOut.write("#SBATCH --cpus-per-task=1 \n")
                slurmOut.write("#SBATCH --ntasks-per-core=1 \n")
                slurmOut.write("#SBATCH --threads-per-core=1 \n")
                slurmOut.write("#SBATCH --mem=%dGB \n" % nCores) # 1 GB per core
                slurmOut.write("#SBATCH --time=%s \n" % TIMELIMIT)
                slurmOut.write("#SBATCH --ntasks=%d \n" % (nCores+1))

                slurmOut.write("#SBATCH --chdir=%s \n" % MyRunningDir)
                slurmOut.write("#SBATCH --error=%s/fit.err \n" % (MyLogOutDir))
                slurmOut.write("#SBATCH --output=%s/fit.out \n" % (MyLogOutDir))
                slurmOut.write("#SBATCH --job-name=benchfit_%03d \n\n\n" % nCores)

                # commands to execute during job
                slurmOut.write("pwd \n")
                slurmOut.write("source %s \n" % MyEnv)
                slurmOut.write("mpirun %s fitMPI -c %s %s \n" % (MyMPIOpt, MyConfig, MyFitOpt))
                slurmOut.close()

                # submit individual job
                print("Submitting %d core job on %d nodes" % (nCores, nNodes))
                subprocess.call(["sbatch", "tempSlurm.txt"])
                os.remove("tempSlurm.txt")
        

if __name__ == "__main__":
   main(sys.argv[1:])
