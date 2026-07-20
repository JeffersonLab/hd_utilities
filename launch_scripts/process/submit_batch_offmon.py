import os
from subprocess import call

def AddJobToSWIF(workflow,run_period,version,run):
    # SET THIS
    command = "check_monitoring_data.batch.sh"
    #command = "check_recon_data.batch.csh"
    
    # config
    project = "halld-pro"         
    track = "production"
    #track = "debug"    # for quick execution of merging
    basedir = "/home/gxproj5/hd_utilities/launch_scripts/process"
    disk_space = 50     # GB
    mem_requested = 16  # GB
    time_limit = 12    # hour
    cores = 6

    # create command
    cmd = "swif2 add-job -workflow %s -account %s -partition %s"%(workflow,project,track)
    cmd += " -name %s_%06d"%(workflow,run)
    cmd += " -os el9 "
    #cmd += " -stdout /volatile/halld/home/gxproj5/process/batch_log/%s/log_%06d"%(workflow,run)
    #cmd += " -stderr /volatile/halld/home/gxproj5/process/batch_log/%s/err_%06d"%(workflow,run)
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
    WORKFLOW = "offmon_2022-08_ver18_post"
    #WORKFLOW = "recon_2017-01_ver02_post"
    DATATYPE="mon"
    VERSION="18"
    RUNPERIOD="RunPeriod-2022-08"

    # A directory containing directories for each run, from which we can extract which directories were processed
    # e.g., /cache/halld/offline_monitoring/RunPeriod-2016-02/ver06/hists
    INPUTDIR="/cache/halld/offline_monitoring/%s/ver%s/hists"%(RUNPERIOD,VERSION)
    #INPUTDIR="/cache/halld/%s/%s/ver%s/hists"%(RUNPERIOD,DATATYPE,VERSION)

    # set up the workflow and some directories
    call("swif2 create -workflow %s"%WORKFLOW, shell=True)
    call("mkdir -p /volatile/halld/home/gxproj5/process/batch_log/%s"%(WORKFLOW), shell=True)

    # create one job for each run
    for rundir in sorted(os.listdir(INPUTDIR)):
        try:
            print("run = %d"%(int(rundir)))
            run  = int(rundir)
            #if run > 110924 and run < 112000:
            AddJobToSWIF(WORKFLOW,RUNPERIOD,VERSION,run)
        except:
            print("Invalid directory = " + rundir)
