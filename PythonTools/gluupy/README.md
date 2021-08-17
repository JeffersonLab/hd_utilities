# gluupy: 
A python-based tool for parsing ROOT files, applying cuts, and creating histograms in the GlueX context.

* Author: Jon Zarling (jzarling@jlab.org, Jonathan.Zarling@uregina.ca)

## Preliminary version

Last updated 8/17/2021

## Requirements:
* Python 2.7.X (no python 3 yet, sorry)
* uproot (https://uproot.readthedocs.io/)
* PyROOT (python 2 compatible)
* Additional python modules: numpy, xxhash, and lz4 


## Installation instructions with pip and python 2.7

### Initial setup (for use on JLab ifarm, or similar cluster) 
1. Move ~/.local to desired location, then make symbolic link back to ~/.local
 * This is where pip will put its packages
 * Default home directory can fill up fast, this way we won't contribute to limited storage space there
2. Self update pip (to latest python 2.7.X compatible version)
> pip install --upgrade "pip < 21.0" --user
 * Add this alias in your login script to use this newer pip version
 > alias pip '~/.local/bin/pip'
 *  Verify pip version with: (after sourcing environment again, of course)
    > pip show pip
 *   Version should be 20.3.4 or similar

### Installing/upgrading packages (numpy, lz4, xxhash, and uproot):
> pip install numpy lz4 xxhash uproot --upgrade --user

# Environment setup:
Add PyROOT and gluupy lib to your PYTHONPATH
 * In csh:
   > setenv PYTHONPATH $HD_UTILITIES_HOME/PythonTools/gluupy:/$ROOTSYS/lib:$PYTHONPATH
 * In bash:
   > export PYTHONPATH="$HD_UTILITIES_HOME/PythonTools/gluupy:/$ROOTSYS/lib:$PYTHONPATH"
   
# Testfile location on ifarm work disk

See input file located at /w/halld-sciwork18/home/jzarling/gluupy_testfile/eta_3pi0_SP18_LZ4.root
