#!/bin/bash
target=$1
replacement=$2
rm -f commands.tmp
find . -type f -exec grep -l $target {} \; \
    | awk '{print "convert.sh $target $replacement "$1}' \
    > commands.tmp
. commands.tmp
rm commands.tmp
