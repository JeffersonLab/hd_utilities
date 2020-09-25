#!bin/tcsh -f
set i=1
set imax=4
while ( $i < $imax )
    source igo_ped.csh period_$i pedcal
    echo $i
    @ i++
end

