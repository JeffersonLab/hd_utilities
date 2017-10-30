#!/usr/bin/env python

import sys
import os
from subprocess import call

path = (os.path.dirname(os.path.realpath(__file__))) 
ccdb_connection = os.environ['CCDB_CONNECTION']

if 'mysql' in ccdb_connection:
   print "CONNECTED TO JLAB MYSQL DATABASE === ABORTING" 
   quit()

if 'sqlite' not in ccdb_connection:
   print "UNKNOWN DATABASE TYPE %s" % ccdb_connection
   quit()

print 'Committing constants to %s' % ccdb_connection
call('ccdb add /CDC/global_alignment %s/zeros/CDC_global_alignment.txt' % path, shell=True)
call('ccdb add /CDC/timing_offsets %s/zeros/CDC_timing.txt' % path, shell=True )
call('ccdb add /CDC/wire_alignment %s/zeros/CDC_wire_alignment.txt' % path, shell=True)
call('ccdb add /FDC/cathode_alignment %s/zeros/FDC_cathode_alignment.txt' % path, shell=True)
call('ccdb add /FDC/cell_offsets %s/zeros/FDC_cell_offsets.txt' % path, shell=True)
call('ccdb add /FDC/cell_rotations %s/zeros/FDC_cell_rotations.txt' % path, shell=True)
call('ccdb add /FDC/strip_pitches_v2 %s/zeros/FDC_strip_pitches_v2.txt' % path, shell=True)
call('ccdb add /FDC/wire_alignment %s/zeros/FDC_wire_alignment.txt' % path, shell=True)
call('ccdb add /FDC/package1/wire_timing_offsets %s/zeros/FDC_wire_timing_offsets.txt' % path, shell=True)
call('ccdb add /FDC/package1/strip_gains_v2 %s/zeros/FDC_strip_gains_v2.txt' % path, shell=True)
call('ccdb add /FDC/package2/wire_timing_offsets %s/zeros/FDC_wire_timing_offsets.txt' % path, shell=True)
call('ccdb add /FDC/package2/strip_gains_v2 %s/zeros/FDC_strip_gains_v2.txt' % path, shell=True)
call('ccdb add /FDC/package3/wire_timing_offsets %s/zeros/FDC_wire_timing_offsets.txt' % path, shell=True)
call('ccdb add /FDC/package3/strip_gains_v2 %s/zeros/FDC_strip_gains_v2.txt' % path, shell=True)
call('ccdb add /FDC/package4/wire_timing_offsets %s/zeros/FDC_wire_timing_offsets.txt' % path, shell=True)
call('ccdb add /FDC/package4/strip_gains_v2 %s/zeros/FDC_strip_gains_v2.txt' % path, shell=True)
