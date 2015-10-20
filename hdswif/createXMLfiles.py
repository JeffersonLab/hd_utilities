import sys, os
import subprocess
from optparse import OptionParser
from os import mkdir
import datetime
import re

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
        PLUGINS = ''
        command = "grep 'set PLUGINS_VALUE = ' " + SCRIPTFILE + " | gawk '{print $4}' > ___tmp_plugins.txt"
        os.system(command)
        tmp_plugins_file_handler = open("___tmp_plugins.txt",'r')
        for line in tmp_plugins_file_handler:
            PLUGINS_VALUE = line.rstrip() # remove newline

        if PLUGINS == '':
            PLUGINS = "\"\""
        
        # print 'PLUGINS_VALUE = ', PLUGINS_VALUE
        os.system('rm -f ___tmp_plugins.txt')
        janaoutfile.write('-PPLUGINS=' + PLUGINS_VALUE + '\n')

        # 2. Get NTHREADS
        janaoutfile.write('-PNTHREADS=' + NCORES + '\n')

        # 3. Get THREAD_TIMEOUT
        THREAD_TIMEOUT = ''
        command = "grep 'set THREAD_TIMEOUT_VALUE = ' " + SCRIPTFILE + " | gawk '{print $4}' > ___tmp_thread_timeout.txt"
        os.system(command)
        tmp_thread_timeout_file_handler = open("___tmp_thread_timeout.txt",'r')
        for line in tmp_thread_timeout_file_handler:
            THREAD_TIMEOUT_VALUE = line.rstrip() # remove newline
    
        if THREAD_TIMEOUT == '':
            THREAD_TIMEOUT = "\"\""
        
        # print 'THREAD_TIMEOUT_VALUE = ', THREAD_TIMEOUT_VALUE
        os.system('rm -f ___tmp_thread_timeout.txt')
        janaoutfile.write('-PTHREAD_TIMEOUT=' + THREAD_TIMEOUT_VALUE + '\n')

        # 4. Get CALIB_CONTEXT
        CALIB_CONTEXT_VALUE = ''
        command = "grep 'set CALIB_CONTEXT_VALUE = ' " + SCRIPTFILE + " | gawk '{print $4}' > ___tmp_calib_context.txt"
        os.system(command)
        tmp_calib_context_file_handler = open("___tmp_calib_context.txt",'r')
        for line in tmp_calib_context_file_handler:
            CALIB_CONTEXT_VALUE = line.rstrip() # remove newline
            print 'CALIB_CONTEXT_VALUE = ', CALIB_CONTEXT_VALUE

        if CALIB_CONTEXT_VALUE == '':
            CALIB_CONTEXT_VALUE = '\"\"'

        # print 'CALIB_CONTEXT_VALUE = ', CALIB_CONTEXT_VALUE
        os.system('rm -f ___tmp_calib_context.txt')
        janaoutfile.write('-PCALIB_CONTEXT=' + CALIB_CONTEXT_VALUE + '\n')

        print 'Created jana config file     ' + janafilename + ' ...'

    # ----------------------------------------------------------------
    # Create software config file
    # This file contains
    # 1. jana version
    # 2. sim-recon version and hash
    # 3. hdds version and hash
    # 4. evio version
    # 5. cernlib version
    # 6. xerces-c version
    # 7. root version
    # 8. ccdb version
    # 9. RAM requested
    # 10. diskspace requested
    # 11. ncores requested
    # 12. timelimit
    # ----------------------------------------------------------------
    softfilename = OUTPUTDIR + 'soft_comm_' + RUNPERIOD_UNDERSCORE + '_ver' + VERSION + '.xml'
    # print 'softfilename = ', softfilename

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
        softoutfile = open(softfilename,'w\n')

        # Get jana version
        janapath = os.environ['JANA_HOME']
        jana_ver = re.sub(r'.*jana_' '|' '/L.*', '', janapath)

        if jana_ver == '':
            print 'JANA VERSION not found, aborting'
            exit()

        # Get ccdb version
        ccdbpath = os.environ['CCDB_HOME']
        ccdb_ver = re.sub(r'.*ccdb_','',ccdbpath)

        if ccdb_ver == '':
            print 'CCDB VERSION not found, aborting'
            exit()

        # Get root version
        rootpath = os.environ['ROOTSYS']
        if rootpath == '':
            print 'ROOT VERSION not found, aborting'
            exit()
        # ROOTSYS may be alias that includes something like 'prod',
        # so use readlink and see what it is
        rootfullpath = subprocess.check_output(['readlink', '-f', rootpath]).rstrip()
        
        if rootfullpath == '':
            print 'ROOT VERSION not found, aborting'
            exit()

        # Get xercesc version
        xercescpath = os.environ['XERCESCROOT']
        # XERCESCROOT may be alias that includes something like 'prod',
        # so use readlink and see what it is
        xercesc_fullpath = subprocess.check_output(['readlink', '-f', xercescpath]).rstrip()
        xercesc_ver = re.sub('.*xerces-c-', '', xercesc_fullpath)

        if xercesc_ver == '':
            print 'XERCESC VERSION not found, aborting'
            exit()

        # Get cern level
        cernlib_ver = os.environ['CERN_LEVEL']
        if cernlib_ver == '':
            print 'CERN_LEVEL not found, aborting'
            exit()

        # Get evio version
        eviopath = os.environ['EVIOROOT']
        if eviopath == '':
            print 'EVIO VERSION not found, aborting'
            exit()
        # EVIOROOT may be alias that includes something like 'prod',
        # so use readlink and see what it is
        evio_fullpath = subprocess.check_output(['readlink', '-f', eviopath]).rstrip()
        evio_ver = re.sub('.*evio-' '|' '/Linux.*','', evio_fullpath)

        if evio_ver == '':
            print 'EVIO VERSION not found, aborting'
            exit()

        # Get sim-recon version and hash
        simreconpath = os.environ['HALLD_HOME']
        if simreconpath == '':
            print 'HALLD_HOME not found, aborting'
            exit()
        original_dir = os.getcwd()
        os.chdir(simreconpath)
        # Get version number
        simrecon_ver = subprocess.check_output(['git', 'rev-list', 'HEAD', '--count']).rstrip()

        # Get hash
        simrecon_hash = str(subprocess.check_output(['git', 'log', '-1'])).split()[1]
        os.chdir(original_dir) # go back to original directory

        # Get hdds version and hash
        hddspath = os.environ['HDDS_HOME']
        if hddspath == '':
            print 'HALLD_HOME not found, aborting'
            exit()
        original_dir = os.getcwd()
        os.chdir(hddspath)
        # Get version number
        hdds_ver = subprocess.check_output(['git', 'rev-list', 'HEAD', '--count']).rstrip()

        # Get hash
        hdds_hash = str(subprocess.check_output(['git', 'log', '-1'])).split()[1]
        os.chdir(original_dir) # go back to original directory

        # Get date for creation of file
        date = datetime.datetime.now()

        softoutfile.write('<gversions>\n')
        softoutfile.write('  <package name="jana" version="' + jana_ver + '"/>\n')
        softoutfile.write('  <package name="sim-recon" version="' + simrecon_ver + '" hash="' + simrecon_hash + '"/>\n')
        softoutfile.write('  <package name="hdds" version="' + hdds_ver + '" hash="' + hdds_hash + '"/>\n')
        softoutfile.write('  <package name="evio" version="' + evio_ver + '"/>\n')
        softoutfile.write('  <package name="cernlib" version="' + cernlib_ver + '" word_length="64-bit"/>\n')
        softoutfile.write('  <package name="xerces-c" version="' + xercesc_ver + '"/>\n')
        softoutfile.write('  <package name="root" version="' + rootfullpath + '"/>\n')
        softoutfile.write('  <package name="ccdb" version="' + ccdb_ver + '"/>\n')
        softoutfile.write('  \n')
        softoutfile.write('  <variable name = "mem_requested" value="' + RAM + '"/>\n')
        softoutfile.write('  <variable name = "DiskSpace" value="' + DISK + '"/>\n')
        softoutfile.write('  <variable name = "ncores" value="' + NCORES + '"/>\n')
        softoutfile.write('  <variable name = "timelimit" value="' + TIMELIMIT + '"/>\n')
        softoutfile.write('  <variable name = "date" value="' + str(date) + '"/>\n')
        softoutfile.write('</gversions>\n')

        print 'Created software config file ' + softfilename + ' ...'

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
        print 'Create tag ' + hdds_tag + ' for sim-recon?'
        while(1):
            answer = raw_input('Create? (y/n)   ')
            if answer == 'n':
                break
            elif answer == 'y':
                original_dir = os.getcwd()
                simreconpath = os.environ['HDDS_HOME']
                os.chdir(simreconpath)
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
   
