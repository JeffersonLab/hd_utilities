#!bin/tcsh -f
set i=1
set imax=49
while ( $i < $imax )
    source ../Env/retry_swif.csh pedcal-period_$i-ped AUGER-TIMEOUT
    source ../Env/retry_swif.csh pedcal-period_$i-ped AUGER-FAILED
    source ../Env/retry_swif.csh pedcal-period_$i-ped SWIF-USER-NON-ZERO
    source ../Env/retry_swif.csh pedcal-period_$i-ped SWIF-SYSTEM-ERROR
    echo $i
    @ i++
end



