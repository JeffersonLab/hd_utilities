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
	PARTITION  = "gpu"
        GPUTYPE    = "TitanRTX"
        TIMELIMIT  = "24:00:00"   # Max walltime
        MyGPUs = [1,2,3,4]     # List of GPU cards to use in benchmark fits

        # User provided environment, fit configuration and options
	MyEnv = "/work/halld2/home/jrsteven/2021-amptools/builds_gpu/setup_gluex_dev.csh"
        MyConfig = "/work/halld2/home/jrsteven/forBenchmark/benchmark.cfg"
        MyMPIOpt = "--mca btl_openib_allow_ib 1"
        MyFitOpt = "-m 100000 -r 5"
	MyOutDir = "/volatile/halld/home/" + pwd.getpwuid( os.getuid() )[0] + "/benchmark/"

        # LOOP OVER # OF GPUs FOR BENCHMARK
        for nGPUs in MyGPUs:
                
                # Two types of nodes/GPUs (sciml19 and sciml21), both with 3 each  
                nNodes = 1
                if GPUTYPE=="T4": 
                        if nGPUs > 8: nNodes=2
                        if nGPUs > 16: nNodes=3
                if GPUTYPE=="TitanRTX": 
                        if nGPUs > 4: nNodes=2
                        if nGPUs > 8: nNodes=3

                # create output directories
                MyRunningDir = MyOutDir + "gpu%s%03d" % (GPUTYPE,nGPUs)
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
                slurmOut.write("#SBATCH --gres=gpu:%s:%d \n" % (GPUTYPE,nGPUs))
                slurmOut.write("#SBATCH --cpus-per-task=1 \n")
                slurmOut.write("#SBATCH --ntasks-per-core=1 \n")
                slurmOut.write("#SBATCH --threads-per-core=1 \n")
                slurmOut.write("#SBATCH --mem=20GB \n") # multiplied by nGPUs in slurm?
                slurmOut.write("#SBATCH --time=%s \n" % TIMELIMIT)
                slurmOut.write("#SBATCH --ntasks=%d \n" % (nGPUs+1))

                slurmOut.write("#SBATCH --chdir=%s \n" % MyRunningDir)
                slurmOut.write("#SBATCH --error=%s/fit.err \n" % (MyLogOutDir))
                slurmOut.write("#SBATCH --output=%s/fit.out \n" % (MyLogOutDir))
                slurmOut.write("#SBATCH --job-name=benchfitgpu_%03d \n\n\n" % nGPUs)

                # commands to execute during job
                slurmOut.write("pwd \n")
                slurmOut.write("source %s \n" % MyEnv)
                slurmOut.write("mpirun %s fitMPI -c %s %s \n" % (MyMPIOpt, MyConfig, MyFitOpt))
                slurmOut.close()

                # submit individual job
                print("Submitting %d GPU job on %d %s nodes" % (nGPUs, nNodes, GPUTYPE))
                subprocess.call(["sbatch", "tempSlurm.txt"])
                os.remove("tempSlurm.txt")
        

if __name__ == "__main__":
   main(sys.argv[1:])
