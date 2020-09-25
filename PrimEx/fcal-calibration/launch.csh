#!bin/tcsh -f
set i=1
set imax=14
while ( $i < $imax )
    source ilaunch_v1.csh period_$i 1 > & outlog_$i.txt &
    echo $i
    @ i++
end

