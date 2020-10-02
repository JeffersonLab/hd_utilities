import os
import time

count = 1

waitTime = 30
resetEachCycle = 1

cmd_setVioletBias1   = 'caput FCAL:bias:pulser:v_set 12.0'
cmd_setBlueBias1     = 'caput FCAL:bias:pulser:v_set 10.0'
cmd_setGreenBias     = 'caput FCAL:bias:pulser:v_set 29.0' # higher than 29.0 does not work
cmd_setVioletBias2   = 'caput FCAL:bias:pulser:v_set 22.0'
cmd_setBlueBias2     = 'caput FCAL:bias:pulser:v_set 15.0'

cmd_setVioletFreq    = 'caput FCAL:pulser:VIOLET:freq_w 2000'
cmd_setBlueFreq      = 'caput FCAL:pulser:BLUE:freq_w 2000'
cmd_setGreenFreq     = 'caput FCAL:pulser:GREEN:freq_w 2000'

cmd_violetOn         = 'caput FCAL:pulser:switch 1'
cmd_blueOn           = 'caput FCAL:pulser:switch 2'
cmd_greenOn          = 'caput FCAL:pulser:switch 3'

cmd_setVioletContin  = 'caput FCAL:pulser:VIOLET:npulses_w -1'
cmd_setBlueContin    = 'caput FCAL:pulser:BLUE:npulses_w -1'
cmd_setGreenContin   = 'caput FCAL:pulser:GREEN:npulses_w -1'

os.system(cmd_setVioletFreq)
os.system(cmd_setBlueFreq)
os.system(cmd_setGreenFreq)

os.system(cmd_setVioletContin)
os.system(cmd_setBlueContin)
os.system(cmd_setGreenContin)

while (count):
     print ' 1. Running Violet pulser            '+time.strftime("%y-%m-%d %H:%M:%S")
     #print time.strftime()+' 1. Running Violet pulser'
     if resetEachCycle:
          os.system(cmd_setVioletFreq)
          os.system(cmd_setVioletContin)
     os.system(cmd_setVioletBias1)
     os.system(cmd_violetOn)
     time.sleep(waitTime)

     print ' 2. Running Blue pulser              '+time.strftime("%y-%m-%d %H:%M:%S")
     if resetEachCycle:
          os.system(cmd_setBlueFreq)
          os.system(cmd_setBlueContin)
     os.system(cmd_setBlueBias1)
     os.system(cmd_blueOn)
     time.sleep(waitTime)

     print ' 3. Running Green pulser             '+time.strftime("%y-%m-%d %H:%M:%S")
     if resetEachCycle:
          os.system(cmd_setGreenFreq)
          os.system(cmd_setGreenContin)
     os.system(cmd_setGreenBias)
     os.system(cmd_greenOn)
     time.sleep(waitTime)

     print ' 4. Running Violet pulser            '+time.strftime("%y-%m-%d %H:%M:%S")
     if resetEachCycle:
          os.system(cmd_setVioletFreq)
          os.system(cmd_setVioletContin)
     os.system(cmd_setVioletBias2)
     os.system(cmd_violetOn)
     time.sleep(waitTime)

     print ' 5. Running Blue pulser              '+time.strftime("%y-%m-%d %H:%M:%S")
     if resetEachCycle:
          os.system(cmd_setBlueFreq)
          os.system(cmd_setBlueContin)
     os.system(cmd_setBlueBias2)
     os.system(cmd_blueOn)
     time.sleep(waitTime)

