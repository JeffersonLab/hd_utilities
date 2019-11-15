import os
import time

count = 1

while (count):
     cmd_red = 'caput FCAL:pulser:switch 1'
     print 'Running Violet pulser'
     os.system(cmd_red)
     time.sleep(3600)
     cmd_blue = 'caput FCAL:pulser:switch 2'
     print 'Running Blue Pulser'
     os.system(cmd_blue)
     time.sleep(3600)
     cmd_green = 'caput FCAL:pulser:switch 3'
     print 'Running Green Pulser'
     os.system(cmd_green)
     time.sleep(3600)
     
