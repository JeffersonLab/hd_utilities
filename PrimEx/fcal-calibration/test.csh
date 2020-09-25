set list = one,two,three
foreach i ( $list:as/,/ / )
  echo $i
end

set split = ($list:as/,/ /)
#echo 'test' $split[2] # returns two
