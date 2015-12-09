from optparse import OptionParser
import xml.etree.ElementTree as ET
import sys, os
import subprocess

def is_number(string):
    try:
        int(string)
        return True
    except ValueError:
        return False

def main(argv):

    # Default parameters
    WORKFLOW = ''
    RUN  = ''
    FILE = ''

    # Get input parameters
    parser = OptionParser(usage = "\n output_job_details.py [workflow] [run] [file]")
    (options, args) = parser.parse_args(argv)
    
    if len(args) != 3:
        print 'output_job_details.py [workflow] [run] [file]'
        exit()

    WORKFLOW = args[0]
    RUN      = args[1]
    FILE     = args[2]

    if(is_number(RUN) == True):
        FORMATTED_RUN = "{:0>6d}".format(int(RUN))
    else:
        print 'output_job_details.py [workflow] [run] [file]'
        print 'Run and file must be numbers'
        exit()

    FORMATTED_RUN  = ""
    FORMATTED_FILE = ""

    if(is_number(RUN) == True):
        FORMATTED_RUN = "{:0>6d}".format(int(RUN))
    else:
        print 'output_job_details.py [workflow] [run] [file]'
        print 'Run and file must be numbers'
        exit()

    if(is_number(FILE) == True):
        FORMATTED_FILE = "{:0>3d}".format(int(FILE))
    else:
        print 'output_job_details.py [workflow] [run] [file]'
        print 'Run and file must be numbers'
        exit()

    # Get the XML output for this workflow
    # It would be nice if we could do this without writing out
    # the XML file to disk
    filename = str('___tmp_' + WORKFLOW + '.xml')
    print 'Creating XML output file........'
    os.system("swif status " + WORKFLOW + " -runs -summary -display xml > " + filename)

    tree = ET.parse(filename)
    workflow_status = tree.getroot()

    # Loop over jobs and find specified run and file
    nFound = 0
    for job in workflow_status.iter('job'):
        run_text  = ''
        file_text = ''
        id_text   = ''

        for user_run in job.iter('user_run'):
            run_text = user_run.text
        for user_file in job.iter('user_file'):
            file_text = user_file.text
        for id in job.iter('id'):
            id_text = id.text

        if run_text == FORMATTED_RUN and file_text == FORMATTED_FILE:
            nFound += 1
            print '--- Found match #' + str(nFound) + ' ---'
            url = 'http://farmpbs:6080/job/' + id_text + '/spec' # use "status" for full job info
            # Don't output progress meter with --silent
            output = subprocess.check_output(['curl', '--silent', url]).rstrip()
            print output

    if nFound == 0:
        print 'No jobs found for run = ' + str(RUN) + ' file = ' + str(FILE)
    else:
        print '========================================'
        print 'Total of ' + str(nFound) + ' jobs found that matched'
    
    # Delete XML file
    os.system("rm -f " + filename)

## main function 
if __name__ == "__main__":
    main(sys.argv[1:])
   
