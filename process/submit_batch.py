import os
from subprocess import call

def AddJobToSWIF(run_period,run):
    # config
    project = "gluex"         
    track = "reconstruction"  
    workflow = "offmon_2016-02_ver08_post"
    basedir = "/home/gxproj5/monitoring/process"
    disk_space = 5
    mem_requested = 6
    time_limit = 24
    cores = 6

    # create command
    #inputfile="/mss/halld/%s/rawdata/Run%06d/hd_rawdata_%06d_%03d.evio"%(run_period,run,run,filenum)
    cmd = "swif add-job -workflow %s -project %s -track %s"%(workflow,project,track)
    cmd += " -name %s_%06d"%(workflow,run)
    cmd += " -os centos65 "
    # stage file from tape
    #cmd += " -input data.evio mss:%s"%inputfile   
    cmd += " -stdout file:%s/batch_log/log_%06d"%(basedir,run)
    cmd += " -stderr file:%s/batch_log/err_%06d"%(basedir,run)
    cmd += " -disk %dGB"%int(disk_space)
    cmd += " -ram %dGB"%int(mem_requested)
    cmd += " -time %dhours"%int(time_limit)
    cmd += " -cores %d"%int(cores)
    cmd += " -tag run_number %d"%run

    # add command to execute
    cmd += " %s/check_monitoring_data.batch.csh %d"%(basedir,run)
    call(cmd, shell=True, stdout=None)


if __name__ == "__main__":
    # configuration
    DATATYPE="mon"
    VERSION="08"
    RUNPERIOD="RunPeriod-2016-02"
    # /cache/halld/offline_monitoring/RunPeriod-2016-02/ver06/hists
    INPUTDIR="/cache/halld/offline_monitoring/%s/ver%s/hists"%(RUNPERIOD,VERSION)

    for rundir in sorted(os.listdir(INPUTDIR)):
        print "run = %d"%(int(rundir))
        run  = int(rundir)
        AddJobToSWIF(RUNPERIOD,run)
