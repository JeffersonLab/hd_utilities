#!/usr/bin/env python2

import os
import time
from epics import caget,caput

start_red_color = '\033[91;1m'
default_color = '\033[0m'
auto_pulsing_stat = "BCAL:pulser:auto_pulsing_stat"


def main():
    print "Beginning pulsing script"
    # this trigger width used to be 100 ns Alex changed it to 50 ns on 2018-Oct-3.
    caput('FCAL:pulser:TRIG:width_w',50) 
    test = False
    Status = -1
    while 1:
        MustPulse = ShouldWePulse()        
        mins = int(time.strftime("%M"))
        # the test allows changes every minutes instead of every 10
        if test:
            val = mins%6
            if mins ==  1:
                Period=1
            elif val == 2: 
                Period=2
            elif val == 3: 
                Period=3
            elif val == 4: 
                Period=4
            elif val == 5: 
                Period=5
            else: 
                Period=0
        else:
            if mins < 10:
                Period=1
            elif mins < 20: 
                Period=2
            elif mins < 30: 
                Period=3
            elif mins < 40: 
                Period=4
            elif mins < 50: 
                Period=5
            else: 
                Period=0
        print "Status=",Status,"Minute=",mins," Period=",Period," Running=",MustPulse

        if MustPulse:
            if (MustPulse and Status!=0 and Period==0):
                Status = 0
                LEDsOff()
            if (MustPulse and Status!=1 and Period==1):
                Status = 1
                LEDsPowerOn()
                RunVioletLow()
            if (MustPulse and Status!=2 and Period==2):
                Status = 2
                LEDsPowerOn()
                RunBlueLow()
            if (MustPulse and Status!=3 and Period==3):
                Status = 3
                LEDsPowerOn()
                RunGreen()
            if (MustPulse and Status!=4 and Period==4):
                Status = 4
                LEDsPowerOn()
                RunVioletHigh()
            if (MustPulse and Status!=5 and Period==5):
                Status = 5
                LEDsPowerOn()
                RunBlueHigh()
        else:
            if (Status!=0):
                Status = 0
                LEDsOff()

        time.sleep(2)


def RunVioletLow():
    print ' 1. Running Violet pulser            '+time.strftime("%y-%m-%d %H:%M")
    cmd_setVioletBias1   = 'caput FCAL:bias:pulser:v_set 12.0'
    cmd_setVioletFreq    = 'caput FCAL:pulser:VIOLET:freq_w 10'
    cmd_violetOn         = 'caput FCAL:pulser:switch 1'
    cmd_setVioletContin  = 'caput FCAL:pulser:VIOLET:npulses_w -1'
    caput('FCAL:pulser:VIOLET:width_w',100)
    os.system(cmd_setVioletFreq)
    os.system(cmd_setVioletContin)
    os.system(cmd_setVioletBias1)
    os.system(cmd_violetOn)

def RunBlueLow():
    print ' 2. Running Blue pulser              '+time.strftime("%y-%m-%d %H:%M")
    cmd_setBlueBias1     = 'caput FCAL:bias:pulser:v_set 10.0'
    cmd_setBlueFreq      = 'caput FCAL:pulser:BLUE:freq_w 10'
    cmd_blueOn           = 'caput FCAL:pulser:switch 2'
    cmd_setBlueContin    = 'caput FCAL:pulser:BLUE:npulses_w -1'
    caput('FCAL:pulser:BLUE:width_w',100)
    os.system(cmd_setBlueFreq)
    os.system(cmd_setBlueContin)
    os.system(cmd_setBlueBias1)
    os.system(cmd_blueOn)

def RunGreen():
    print ' 3. Running Green pulser             '+time.strftime("%y-%m-%d %H:%M")
    cmd_setGreenBias     = 'caput FCAL:bias:pulser:v_set 29.0' # higher than 29.0 does not work
    cmd_setGreenFreq     = 'caput FCAL:pulser:GREEN:freq_w 10'
    cmd_greenOn          = 'caput FCAL:pulser:switch 3'
    cmd_setGreenContin   = 'caput FCAL:pulser:GREEN:npulses_w -1'
    caput('FCAL:pulser:GREEN:width_w',100)
    os.system(cmd_setGreenFreq)
    os.system(cmd_setGreenContin)
    os.system(cmd_setGreenBias)
    os.system(cmd_greenOn)

def RunVioletHigh():
    print ' 4. Running Violet pulser            '+time.strftime("%y-%m-%d %H:%M")
    cmd_setVioletBias2   = 'caput FCAL:bias:pulser:v_set 22.0'
    cmd_setVioletFreq    = 'caput FCAL:pulser:VIOLET:freq_w 10'
    cmd_violetOn         = 'caput FCAL:pulser:switch 1'
    cmd_setVioletContin  = 'caput FCAL:pulser:VIOLET:npulses_w -1'
    caput('FCAL:pulser:VIOLET:width_w',100)
    os.system(cmd_setVioletFreq)
    os.system(cmd_setVioletContin)
    os.system(cmd_setVioletBias2)
    os.system(cmd_violetOn)

def RunBlueHigh():
    print ' 5. Running Blue pulser              '+time.strftime("%y-%m-%d %H:%M")
    cmd_setBlueBias2     = 'caput FCAL:bias:pulser:v_set 15.0'
    cmd_setBlueFreq      = 'caput FCAL:pulser:BLUE:freq_w 10'
    cmd_blueOn           = 'caput FCAL:pulser:switch 2'
    cmd_setBlueContin    = 'caput FCAL:pulser:BLUE:npulses_w -1'
    caput('FCAL:pulser:BLUE:width_w',100)
    os.system(cmd_setBlueFreq)
    os.system(cmd_setBlueContin)
    os.system(cmd_setBlueBias2)
    os.system(cmd_blueOn)

def LEDsOff():
    print ' 0. Turning pulser off              '+time.strftime("%y-%m-%d %H:%M")
    cmd_LEDsOff           = 'caput FCAL:pulser:switch 0'
    cmd_setOffBias        = 'caput FCAL:bias:pulser:v_set 0.0'
    os.system(cmd_OffBias)
    os.system(cmd_LEDsOff)
    
def LEDsPowerOn():
    cmd_LEDsLVOn          = 'caput FCAL:lv:pulser:switch 1'
    cmd_LEDsBiasOn        = 'caput FCAL:bias:pulser:switch 1'
    os.system(cmd_OffBias)
    os.system(cmd_LEDsOff)
    



def ShouldWePulse():
    global start_red_color
    global default_color
    global auto_pulsing_stat
    ret = caget(auto_pulsing_stat)
    if ret==0:
        return False
    elif ret==1:
        return True
    elif ret == None:
        print start_red_color +"check if epics variable %s exist!"%(auto_pulsing_stat)+ default_color
        do_exit()
    elif ret == -1:
        do_exit()

if __name__=="__main__":
    main()
