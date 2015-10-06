import sys, os
from optparse import OptionParser
from os import mkdir

# ---------------------------------------------------------------
# Utility function that will take in as argument
# config file name and return a dictionary containing
# the run configuration parameters based on this config file.
# ---------------------------------------------------------------

def main(argv):

    # Default arguments
    USERCONFIGFILE = ''
    VERBOSE        = 0

    # Get input parameters
    parser = OptionParser(usage = "\n read_config.py [config file] (verbosity)")
    (options, args) = parser.parse_args(argv)

    if len(args) < 2:
        print 'read_config.py [config file] (verbosity)'
        exit()

    elif len(args) == 2:
        if int(args[1]) == 1:
            VERBOSE = True
    else:
        print 'read_config.py [config file] (verbosity)'
        exit()
        
    USERCONFIGFILE = args[0]

    # Below is default configuration, is updated
    # with user-specified config file
    config_dict = {
        'PROJECT'        : 'gluex',
        'TRACK'          : 'reconstruction',
        'OS'             : 'centos65',
        'JOBNAMEBASE'    : 'offmon_',
        'RUNPERIOD'      : '2015-03',
        'VERSION'        : '99',                  # Used to specify output top directory
        'OUTPUT_TOPDIR'  : '/volatile/halld/home/gxproj5/hdswif_test/RunPeriod-[RUNPERIOD]/ver[VERSION]', # # Needs to be full path
        'NCORES'         : 6,
        'DISK'           : 40,
        'RAM'            : 6,
        'TIMELIMIT'      : 8,
        'SCRIPTFILE'     : '/home/gxproj5/halld/hdswif/script.sh',                # Needs to be full path
        'ENVFILE'        : '/home/gxproj5/halld/hdswif/setup_jlab-2015-03.csh',   # Needs to be full path
        }

    user_dict = {}
    # Check if config file exists
    if (not os.path.isfile(USERCONFIGFILE)) or (not os.path.exists(USERCONFIGFILE)):
        print 'Config file ', USERCONFIGFILE, ' is not a readable file'
        print 'Exiting...'
        exit()

    # Read in user config file
    infile_config = open(USERCONFIGFILE,'r')
        
    for line in infile_config:

        # Ignore empty lines
        # print 'line = ', line, ' split: ', line.split()
        if len(line.split()) == 0:
            continue

        # Do not update if line begins with #
        if line.split()[0][0] == '#':
            continue

        # Add new key/value pair into user_dict
        user_dict[str(line.split()[0])] = line.split()[1]
        
    #  Update all of the values in config_dict
    # with those specified by the user
    config_dict.update(user_dict)

    if VERBOSE == True:
        print 'Updated config_dict with user config file'
        print 'config_dict is: ', config_dict.items()

    # At this stage we have all the key/value combinations
    # that the user specified. Some of these may depend on
    # other configuration parameters, so update the values
    # containing [key] within the values corresponding to
    # those keys.
    #
    # Example:
    # OUTPUT_TOPDIR /volatile/halld/test/RunPeriod-[RUNPERIOD]/ver[VERSION]
    # depends on other config parameters RUNPERIOD and VERSION
    # 
    # NOTE: The method assumes there are no circular dependencies
    # which would not make sense,
        
    # Iterate over key/value pairs in dictionary
    # If we find a replacement, we need to start over.
    # The parameter found keeps track of whether we found
    # a replacement or not.
    found = 1
    while(found):
        for key, value in config_dict.items():
            found = 0
            
            # print '================================================================'
            # print 'key = ', key, ' value = ', value
            # For each one see if any values contain [P] where
            # P is a different value
            for other_key, other_value in config_dict.items():
                # print 'other_key = ', other_key, ' other_value = ', other_value
                # print 'searching for ', str('[' + other_key + ']')
                if str(value).find(str('[' + other_key + ']')) != -1:
                    # Found replacement
                    found = 1
                    new_value = value.replace(str('[' + other_key + ']'),other_value)
                    # print 'key = ', key, ' new value = ', new_value
                    # Replace new key/value pair into config_dict
                    new_pair = {key : new_value}
                    config_dict.update(new_pair)
                    del new_pair
                    # print '--------------------'
                    
                    # Break out of loop over other_key, other_value
                    break
            # Break out of loop over key, value
            if found == 1:
                break
            
            # If we do not find a replacement we will finish the loop

    if VERBOSE == True:
        print 'config_dict is: ', config_dict.items()

    # config_dict has now been updated if config file was specified
    print "+++         configuration             +++"
    print "---   Job configuration parameters:   ---"
    print "PROJECT           = " + config_dict['PROJECT']
    print "TRACK             = " + config_dict['TRACK']
    print "OS                = " + config_dict['OS']
    print "NCORES            = " + str(config_dict['NCORES'])
    print "DISK              = " + str(config_dict['DISK'])
    print "RAM               = " + str(config_dict['RAM'])
    print "TIMELIMIT         = " + str(config_dict['TIMELIMIT'])
    print "JOBNAMEBASE       = " + str(config_dict['JOBNAMEBASE'])
    print "RUNPERIOD         = " + config_dict['RUNPERIOD']
    print "VERSION           = " + config_dict['VERSION']
    print "OUTPUT_TOPDIR     = " + config_dict['OUTPUT_TOPDIR']
    print "SCRIPTFILE        = " + config_dict['SCRIPTFILE']
    print "ENVFILE           = " + config_dict['ENVFILE']
    print ""
    print "-----------------------------------------"

    answer = ''
    while answer != 'y' or answer != 'n':
        answer = raw_input('OK? (y/n)')
        if answer == 'y':
            break
        if answer == 'n':
            exit()

    return config_dict

## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
