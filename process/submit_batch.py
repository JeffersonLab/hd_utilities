import os
from subprocess import call

def AddJobToSWIF(run_period,version,run):
    # SET THIS
    workflow = "offmon_2016-02_ver08_post"

    # config
    project = "gluex"         
    track = "reconstruction"  
    basedir = "/home/gxproj5/monitoring/process"
    disk_space = 5     # GB
    mem_requested = 6  # GB
    time_limit = 12    # hour
    cores = 6

    # create command
    cmd = "swif add-job -workflow %s -project %s -track %s"%(workflow,project,track)
    cmd += " -name %s_%06d"%(workflow,run)
    cmd += " -os centos65 "
    cmd += " -stdout file:%s/batch_log/%s_%s_log_%06d"%(run_period,version,basedir,run)
    cmd += " -stderr file:%s/batch_log/%s_%s_err_%06d"%(run_period,version,basedir,run)
    cmd += " -disk %dGB"%int(disk_space)
    cmd += " -ram %dGB"%int(mem_requested)
    cmd += " -time %dhours"%int(time_limit)
    cmd += " -cores %d"%int(cores)
    cmd += " -tag run_number %d"%run

    # add command to execute
    cmd += " %s/check_monitoring_data.batch.csh %s %s %d"%(basedir,run_period,version,run)
    call(cmd, shell=True, stdout=None)


if __name__ == "__main__":
    # SET THIS
    DATATYPE="mon"
    VERSION="08"
    RUNPERIOD="RunPeriod-2016-02"
    # /cache/halld/offline_monitoring/RunPeriod-2016-02/ver06/hists
    INPUTDIR="/cache/halld/offline_monitoring/%s/ver%s/hists"%(RUNPERIOD,VERSION)

    for rundir in sorted(os.listdir(INPUTDIR)):
        print "run = %d"%(int(rundir))
        run  = int(rundir)
        AddJobToSWIF(RUNPERIOD,VERSION,run)
