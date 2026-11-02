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
    #cmd += " -stdout /volatile/halld/home/gxproj5/process/batch_log/%s/log_%06d"%(workflow,run)
    #cmd += " -stderr /volatile/halld/home/gxproj5/process/batch_log/%s/err_%06d"%(workflow,run)
    cmd += " -stdout /farm_out/aaustreg/process/batch_log/%s/log_%06d"%(workflow,run)
    cmd += " -stderr /farm_out/aaustreg/process/batch_log/%s/err_%06d"%(workflow,run)
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
    #WORKFLOW = "recon_2017-01_ver02_post"
    DATATYPE="mc"
    VERSION="01"
    RUNPERIOD="RunPeriod-2019-11"

    # A directory containing directories for each run, from which we can extract which directories were processed
    # e.g., /cache/halld/offline_monitoring/RunPeriod-2016-02/ver06/hists
    #INPUTDIR="/cache/halld/offline_monitoring/%s/ver%s/hists"%(RUNPERIOD,VERSION)
    #INPUTDIR="/cache/halld/%s/%s/ver%s/hists"%(RUNPERIOD,DATATYPE,VERSION)
    INPUTDIR="/cache/cache/halld/gluex_simulations/REQUESTED_MC/2019-11_bggen2026_batch01_4906/root/monitoring_hists"
    
    # set up the workflow and some directories
    call("swif2 create -workflow %s"%WORKFLOW, shell=True)
    #call("mkdir -p /volatile/halld/home/gxproj5/process/batch_log/%s"%(WORKFLOW), shell=True)

    runs=[71491,71494,71544,71670,71721,71724,71725,71760,71761,71762,71850,71868,71994,72059,72068,72070,72082,72132,72159,72164,72175,72185,72188,72189,72193,72392,72731,72736,72761,72770,72785,72790,72792,72795,72815,72850,72872,72878,72879,72880,72881,72882,72909,72911,72920,72924,72926,72927,73043,73044,73050,73063,73066,73077,73083,73085,73095,73102,73115,73123,73129,73144,73146,73147,73148,73161,73166,73177,73191,73211,73212,73215,73219,73220,73224,73225,73227]
    # create one job for each run
    for rundir in sorted(os.listdir(INPUTDIR)):
        try:
            run = int(rundir.split("_")[3].split(".")[0])
            print("run = %d" % run)
            if run in runs:
                AddJobToSWIF(WORKFLOW,RUNPERIOD,VERSION,run)
        except:
            print("Invalid directory = " + rundir)
