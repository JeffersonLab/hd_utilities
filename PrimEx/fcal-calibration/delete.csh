set i=0
set imax=48
while ( $i < $imax )
    #set j=1
    #set jmax=6
    #while ( $j < $jmax )
    #	source ../Env/delete_swif.csh primex-fcal-gains-matching-period_$i-$j
    #	@ j++
    #end
    source ../Env/delete_swif.csh pedcal-period_$i-ped
    @ i++
end
