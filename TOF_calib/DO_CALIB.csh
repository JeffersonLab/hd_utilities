#!/bin/tcsh

set RUN = $1

mkdir calibration$RUN

# do walk correction
echo "executing walk1.C"
root -b -q "src/walk1.C+($RUN)" >& calibration$RUN/walkfit.log
echo "cat walkfit.log:"
cat calibration$RUN/walkfit.log

# do mean time determination
echo "executing domeantime.csh in bg"
./domeantime.csh $RUN >& domt.log &

set PID = $!

# calculate time differences
echo "executing domeantime.csh in fg"
./dotimediff.csh $RUN >& deltat.log

while ( `ps -p "$PID" | wc -l` > 1 )
  sleep 60
end

echo "cat domt.log"
cat domt.log
echo
echo "cat deltat.log"
cat deltat.log

# calculate eff. speeds in paddles for both planes
echo "executing tdlook.C for both planes"
root -b -q "src/tdlook.C($RUN,0)"
root -b -q "src/tdlook.C($RUN,1)"

# determined individual PMT offsets from mean time offsets and
# time difference offsets
echo "executing consolidate.C"
root -b -q "src/consolidate.C($RUN)"

# option 10: get TOF TDC calibration for single ended paddles
echo "executing dofitall.C"
root -b -q "src/dofitall.C+($RUN,10)"

# determine adc time offsets so that T_TDC - T_ADC will be peaked at zero.
echo "executing doadctimeoffsets.C"
root -b -q "src/doadctimeoffsets.C+($RUN)"


# determine attenuation length
#root -b -q "src/dofit.C($RUN)"
