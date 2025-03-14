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

	#subprocess.call(["sleep", "1800"])
	subprocess.call(["mkdir", "-p", "root/"])
	for lut in range(0, 48):
		fileInName = "hddm/lut_%d.hddm" % lut
		fileOutName = "root/lut_%d.root" % lut
		subprocess.call(["hd_root", "-PNTHREADS=8",  "-o", fileOutName, "-PPLUGINS=lut_dirc", fileInName])

	
if __name__=="__main__":
    main(sys.argv[1:])

