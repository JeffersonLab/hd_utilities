#!/usr/bin/env python

import ROOT

from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
import glob

##################### main function ####################
def main(argv):

	for lut in range(0, 48):
		fileInName = "lut/loopLut/lut_%d.root" % lut
		subprocess.call(["ln", "-s", fileInName, "lut.root"])
		subprocess.call(["root", "-b", "-q", "loadlib.C", "glxlut_avr.C+"])
		fileOutName = "lut/loopLut/lut_avr_%d.root" % lut
		subprocess.call(["mv", "lut_avr.root", fileOutName])
		subprocess.call(["rm", "lut.root"]) 
	
if __name__=="__main__":
    main(sys.argv[1:])

