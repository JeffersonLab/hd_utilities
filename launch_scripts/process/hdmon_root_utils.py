# Utility classes for monitoring/calibrations

from ROOT import gROOT,gSystem,gPad
import os
from os import listdir
from os.path import isfile, join, isdir


def load_calibration_library():
    """
    Function for loading external ROOT functions into memory
    
    Requires environemental variables CALIB_LIBDIR and BMS_OSNAME to be set, 
    although 'reasonable' defaults are provided

    Will load:
    - all *.C files in CALIB_LIBDIR
    - all *.so files in CALIB_LIBDIR/BMS_OSNAME and CALIB_LIBDIR/BMS_OSNAME/lib  [get rid of lib subdir?]
    - all *.py files in CALIB_LIBDIR  [not yet]
    """
    # preparations
    libdir = os.getenv('CALIB_LIBDIR', 'lib')
    osdir  = os.getenv('BMS_OSNAME', '.')
    solibdir = join(libdir,osdir)

    # debugging output
    VERBOSE = False
    if VERBOSE:
        if 'CALIB_LIBDIR' in os.environ:
            print "CALIB_LIBDIR = %s"%(os.environ['CALIB_LIBDIR'])
        if 'BMS_OSNAME' in os.environ:
            print "BMS_OSNAME   = %s"%(os.environ['BMS_OSNAME'])
        print "libdir       = %s"%(libdir)
        print "osdir        = %s"%(osdir)
        print "solibdir     = %s"%(solibdir)

    # Load shared libraries into ROOT
    if isdir(solibdir):
        solibs  = [ lib for lib in listdir(solibdir) if isfile(join(solibdir,lib)) and lib[-3:] == ".so" ] 
        for lib in solibs:
            if VERBOSE:
                print "Loading shared lib = %s"%join(solibdir,lib)
            gSystem.Load(join(solibdir,lib))
    if isdir(join(solibdir,'lib')):
        solibs = [ lib for lib in listdir(join(solibdir,'lib')) if isfile(join(solibdir,'lib',lib)) and lib[-3:] == ".so" ] 
        for lib in solibs:
            if VERBOSE:
                print "Loading shared lib = %s"%join(solibdir,'lib',lib)
            gSystem.Load(join(solibdir,'lib',lib))

    # Run any Root scripts
    if isdir(libdir):
        scripts = [ f for f in listdir(libdir) if isfile(join(libdir,f)) and f[-2:] == '.C' ] 
        for script in scripts:
            if VERBOSE:
                print "Loading ROOT script = %s"%(join(libdir,f))
            gROOT.ProcessLine(".L %s"%(join(libdir,f)))


# wrapper functions for executing external commands
def run_root_commands(cmds):
    for cmd in cmds:
        gROOT.ProcessLine(cmd)

def run_python_scripts(cmds):
    for cmd in cmds:
        os.system(cmd)

def run_calib_script(infile, cmds, img_outfile=None):
    infile.cd()
    if gPad != None:
        the_canvas = gPad.GetCanvas()
        the_canvas.Clear()
    run_root_commands(cmds)
    if img_outfile is not None:
        the_canvas = gPad.GetCanvas()
        the_canvas.Print(img_outfile)
