import os
from subprocess import call

def AddJobToSWIF(workflow,run_period,version,run):
    # SET THIS
    command = "check_bggen_data.batch.sh"
    #command = "check_recon_data.batch.csh"
    
    # config
    project = "halld"         
    track = "production"
    #track = "debug"    # for quick execution of merging
    basedir = "/work/halld2/home/aaustreg/Analysis/sw/my_halld_el9/hd_utilities/launch_scripts/process"
    disk_space = 50     # GB
    mem_requested = 16  # GB
    time_limit = 12    # hour
    cores = 6

    # create command
    cmd = "swif2 add-job -workflow %s -account %s -partition %s"%(workflow,project,track)
    cmd += " -name %s_%06d"%(workflow,run)
    cmd += " -os el9 "
    cmd += " -stdout /farm_out/gxproj5/process/batch_log/%s/log_%06d"%(workflow,run)
    cmd += " -stderr /farm_out/gxproj5/process/batch_log/%s/err_%06d"%(workflow,run)
    cmd += " -disk %dGB"%int(disk_space)
    cmd += " -ram %dGB"%int(mem_requested)
    cmd += " -time %dhours"%int(time_limit)
    cmd += " -cores %d"%int(cores)
    cmd += " -tag run_number %d"%run

    # add command to execute
    #cmd += " %s/check_monitoring_data.batch.csh %s %s %d"%(basedir,run_period,version,run)
    cmd += " %s/%s %s %s %d"%(basedir,command,run_period,version,run)
    
    print(cmd)
    call(cmd, shell=True, stdout=None)


if __name__ == "__main__":
    # SET THIS
    WORKFLOW = "bggen_2019-11_ver01_post"
    DATATYPE="mc"
    VERSION="01"
    RUNPERIOD="RunPeriod-2019-11"

    # A directory containing directories for each run, from which we can extract which directories were processed
    # e.g., /cache/halld/offline_monitoring/RunPeriod-2016-02/ver06/hists
    INPUTDIR="/cache/cache/halld/gluex_simulations/REQUESTED_MC/2019-11_bggen2026_batch01_4906/root/monitoring_hists"
    
    # set up the workflow and some directories
    call("swif2 create -workflow %s"%WORKFLOW, shell=True)

    # create one job for each run
    for rundir in sorted(os.listdir(INPUTDIR)):
        try:
            run = int(rundir.split("_")[3].split(".")[0])
            print("run = %d" % run)
            AddJobToSWIF(WORKFLOW,RUNPERIOD,VERSION,run)
        except:
            print("Invalid directory = " + rundir)
