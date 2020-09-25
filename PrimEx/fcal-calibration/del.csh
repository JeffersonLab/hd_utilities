#!bin/tcsh -f
set i=1
set imax=49
while ( $i < $imax )
    source  ../Env/delete_swif.csh pedcal-period_$i-ped
    echo $i
    @ i++
end



