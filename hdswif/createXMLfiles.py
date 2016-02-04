import sys, os
import subprocess
from optparse import OptionParser
from os import mkdir
import datetime
import re
from subprocess import Popen, PIPE

import read_config

def main(argv):

    # Default parameters
    USERCONFIGFILE = ''
    VERBOSE        = 0

    # Get input parameters
    parser = OptionParser(usage = "\n createXMLfiles.py [config file] (verbosity)")
    (options, args) = parser.parse_args(argv)
    
    if len(args) < 1:
        print 'createXMLfiles [config file] (verbosity)'
        print '  If username contains "gxproj" file is output to /group/halld/data_monitoring/run_conditions/'
        print '  Otherwise output goes to current directory'
        exit()

    USERCONFIGFILE = args[0]
    if len(args) > 1 and int(args[1]) == 1:
        VERBOSE = 1

    OUTPUTDIR = '.'
    USER = os.environ['USER']
    if USER.find('gxproj') != -1:
        OUTPUTDIR = '/group/halld/data_monitoring/run_conditions/'

    config_dict = read_config.main([USERCONFIGFILE,str(VERBOSE)])

    RUNPERIOD     = config_dict['RUNPERIOD']
    OUTPUT_TOPDIR = config_dict['OUTPUT_TOPDIR']
    RUNPERIOD     = config_dict['RUNPERIOD']
    VERSION       = config_dict['VERSION']

    NCORES        = config_dict['NCORES']
    DISK          = config_dict['DISK']
    RAM           = config_dict['RAM']
    TIMELIMIT     = config_dict['TIMELIMIT']
    SCRIPTFILE    = config_dict['SCRIPTFILE']
    PLUGINS       = config_dict['PLUGINS']

    if VERBOSE:
        print 'RUNPERIOD     = ', RUNPERIOD
        print 'OUTPUT_TOPDIR = ', OUTPUT_TOPDIR
        print 'RUNPERIOD     = ', RUNPERIOD
        print 'VERSION       = ', VERSION

        print 'NCORES        = ', NCORES
        print 'DISK          = ', DISK
        print 'RAM           = ', RAM
        print 'TIMELIMIT     = ', TIMELIMIT
        print 'SCRIPTFILE    = ', SCRIPTFILE
        print 'PLUGINS       = ', PLUGINS

    # ----------------------------------------------------------------
    # Create jana config file
    # This file contains
    # 1. -PPLUGINS
    # 2. -PNTHREADS
    # 3. -PTHREAD_TIMEOUT
    # 4. -PCALIB_CONTEXT
    # NTHREADS is in the config file (NCORES), while the other
    # variables are defined within the SCRIPTFILE
    # ----------------------------------------------------------------
    RUNPERIOD_UNDERSCORE = RUNPERIOD.replace('-', '_')

    janafilename = OUTPUTDIR + '/jana_rawdata_comm_' + RUNPERIOD_UNDERSCORE + '_ver' + VERSION + '.conf'
    # print 'janafilename = ', janafilename

    create_janafile = True

    if os.path.isfile(janafilename):
        print 'File ', janafilename, ' already exists'
            
        while(1):
            answer = raw_input('Overwrite? (y/n)   ')
            if answer == 'n':
                print 'Not creating jana config file...'
                create_janafile = False
                break
            elif answer == 'y':
                break

    if create_janafile == True:

        janaoutfile = open(janafilename,'w')

        # 1. Get PLUGINS
        if PLUGINS == '':
            PLUGINS = "\"\""
        
        janaoutfile.write('-PPLUGINS=' + PLUGINS + '\n')

        # 2. Get NTHREADS
        num_threads = str(NCORES)
        if(num_threads == "24"):
            num_threads = "Ncores"
        janaoutfile.write('-PNTHREADS=' + num_threads + '\n')

        # 3. Get THREAD_TIMEOUT
        THREAD_TIMEOUT = ''
        THREAD_TIMEOUT = subprocess.check_output(['grep', 'set THREAD_TIMEOUT_VALUE', SCRIPTFILE]).rstrip().split()[3]
    
        if THREAD_TIMEOUT == '':
            THREAD_TIMEOUT = "\"\""
        
        janaoutfile.write('-PTHREAD_TIMEOUT=' + THREAD_TIMEOUT + '\n')

        # 4. Get CALIB_CONTEXT
        CALIB_CONTEXT = ''
        try:
            CALIB_CONTEXT = subprocess.check_output(['grep', 'set CALIB_CONTEXT_VALUE', SCRIPTFILE])
        except subprocess.CalledProcessError as e:
            print e.output

        if CALIB_CONTEXT:
            CALIB_CONTEXT = CALIB_CONTEXT.rstrip().split()[3]

        if CALIB_CONTEXT == '':
            CALIB_CONTEXT = '\"\"'

        janaoutfile.write('-PCALIB_CONTEXT=' + CALIB_CONTEXT + '\n')

        print 'Created jana config file     ' + janafilename + ' ...'




    softfilename = OUTPUTDIR + 'soft_comm_' + RUNPERIOD_UNDERSCORE + '_ver' + VERSION + '.xml'
    create_softfile = True

    if os.path.isfile(softfilename):
        print 'File ', softfilename, ' already exists'
            
        while(1):
            answer = raw_input('Overwrite? (y/n)   ')
            if answer == 'n':
                print 'Not creating software xml file...'
                create_softfile = False
                break
            elif answer == 'y':
                break

    if create_softfile == True:
        xmlfile = os.environ['GLUEX_VERSION_XML']
        command = "cp " + xmlfile + " " + softfilename
        process = Popen(command.split(), stdout=PIPE)
		  output = process.communicate()[0] # is stdout. [1] is stderr

    # Create tags for git repositories if user is gxproj
    create_tags = False
    USER = os.environ['USER']
    if USER.find('gxproj') != -1:
        create_tags = True

    if create_tags == True:

        sim_recon_tag = 'offmon-' + RUNPERIOD_UNDERSCORE + '-ver' + VERSION
        print 'Create tag ' + sim_recon_tag + ' for sim-recon?'
        while(1):
            answer = raw_input('Create? (y/n)   ')
            if answer == 'n':
                break
            elif answer == 'y':
                original_dir = os.getcwd()
                simreconpath = os.environ['HALLD_HOME']
                os.chdir(simreconpath)
                today = datetime.datetime.today().strftime("%Y-%m-%d")
                comment = '"Used for offline monitoring ' + RUNPERIOD + ' ver' + VERSION + ' started on ' + today + '"'
                os.system('git tag -a ' + sim_recon_tag + ' -m ' + comment)
                os.system('git push origin ' + sim_recon_tag)
                print 'Created tag ' + sim_recon_tag
                os.chdir(original_dir) # go back to original directory
                break

        hdds_tag = 'offmon-' + RUNPERIOD_UNDERSCORE + '-ver' + VERSION
        print 'Create tag ' + hdds_tag + ' for hdds?'
        while(1):
            answer = raw_input('Create? (y/n)   ')
            if answer == 'n':
                break
            elif answer == 'y':
                original_dir = os.getcwd()
                hddspath = os.environ['HDDS_HOME']
                os.chdir(hddspath)
                today = datetime.datetime.today().strftime("%Y-%m-%d")
                comment = '"Used for offline monitoring ' + RUNPERIOD + ' ver' + VERSION + ' started on ' + today + '"'
                os.system('git tag -a ' + hdds_tag + ' -m ' + comment)
                os.system('git push origin ' + hdds_tag)
                print 'Created tag ' + hdds_tag
                os.chdir(original_dir) # go back to original directory
                break
    

## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
   
