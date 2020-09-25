#!/bin/csh
# demoloop.csh - Sample loop script
set j = 1
while ( $j <= 5 )
  echo "Welcome $j times"
  @ j++
end
