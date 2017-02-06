# script for pass 1 calibration processing

import os,sys
from optparse import OptionParser
import hdmon_root_utils
from ROOT import TFile,gPad,gROOT

if __name__ == "__main__":
    parser = OptionParser(usage = "process_new_offline_data.py [root_command]")
    parser.add_option("-d","--debug", dest="debug", action="store_true",
                      help="Run ROOT commands with debug options.")
    parser.add_option("-F","--input_file", dest="input_file", 
                      help="Input ROOT file to open.")
    parser.add_option("-O","--output_file", dest="output_image_filename", 
                      help="File to save canvas image.")
    parser.add_option("-T","--output_file_format", dest="output_image_format", 
                      help="Format to save image at, can be any supported by ROOT (e.g. png, pdf).")

    (options, args) = parser.parse_args(sys.argv)
    # defaults
    image_suffix = "png"   # which file format the image should be
    
    if len(args)<2:
        print "Need to specify ROOT command !"
        print ""
        #parser.print_help()
        sys.exit(1)

        #gROOT.ProcessLine(".T")
    if options.debug:
        # debug options - trace enable
        gROOT.ProcessLine(".T")

    # extract the command to run
    #root_command = args[1]
    root_command = ".x " + " ".join(args[1:])

    if options.input_file:
        # try to open file if specified
        input_file = TFile(options.input_file)
        if input_file is None or input_file.IsZombie():
            print "Could not open file: %s"%(options.input_file)
            sys.exit(1)

    # load library functions
    hdmon_root_utils.load_calibration_library()

    # run the actual command
    if "HALLD_HOME" not in os.environ:
        print "HALLD_HOME not set!"
    else:
        print "Running commmand = " + root_command
        hdmon_root_utils.run_root_commands([ root_command ])

    # save canvas
    if options.output_image_filename:
        #if gPad is not None:   # check error management later
        the_canvas = gPad.GetCanvas()
        #    if the_canvas is not None:
        the_canvas.Print(options.output_image_filename+"."+image_suffix)

    # cleanup
    if options.input_file:
        input_file.Close()
        
