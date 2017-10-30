#!/usr/bin/env python

import sys
import os
from subprocess import call

ccdb_connection = os.environ['CCDB_CONNECTION']

if 'mysql' in ccdb_connection:
   print "CONNECTED TO JLAB MYSQL DATABASE === ABORTING" 
   quit()

if 'sqlite' not in ccdb_connection:
   print "UNKNOWN DATABASE TYPE %s" % ccdb_connection
   quit()

print 'Committing constants to %s' % ccdb_connection
call('ccdb add /CDC/global_alignment zeros/CDC_global_alignment.txt')
call('ccdb add /CDC/timing_offsets zeros/CDC_timing.txt')
call('ccdb add /CDC/wire_alignment zeros/CDC_wire_alignment.txt')
call('ccdb add /FDC/cathode_alignment zeros/FDC_cathode_alignment.txt')
call('ccdb add /FDC/cell_offsets zeros/FDC_cell_offsets.txt')
call('ccdb add /FDC/cell_rotations zeros/FDC_cell_rotations.txt')
call('ccdb add /FDC/strip_pitches_v2 zeros/FDC_strip_pitches_v2.txt')
call('ccdb add /FDC/wire_alignment zeros/FDC_wire_alignment.txt')
call('ccdb add /FDC/package1/wire_timing_offsets zeros/FDC_wire_timing_offsets.txt')
call('ccdb add /FDC/package1/strip_gains_v2 zeros/FDC_strip_gains_v2.txt')
call('ccdb add /FDC/package2/wire_timing_offsets zeros/FDC_wire_timing_offsets.txt')
call('ccdb add /FDC/package2/strip_gains_v2 zeros/FDC_strip_gains_v2.txt')
call('ccdb add /FDC/package3/wire_timing_offsets zeros/FDC_wire_timing_offsets.txt')
call('ccdb add /FDC/package3/strip_gains_v2 zeros/FDC_strip_gains_v2.txt')
call('ccdb add /FDC/package4/wire_timing_offsets zeros/FDC_wire_timing_offsets.txt')
call('ccdb add /FDC/package4/strip_gains_v2 zeros/FDC_strip_gains_v2.txt')
