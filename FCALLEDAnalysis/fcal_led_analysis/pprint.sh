#!/bin/sh

[ "$#" -lt "2" ] && exit 1

comm="$1"
shift

red="\033[1;31m"
green="\033[1;32m"
cyan="\033[1;36m"
rst="\033[0m"

case "$comm" in
  info)
    printf "[${cyan}i${rst}] %s\n" "$*"
    ;;
  err)
    printf "[${red}e${rst}] %s\n" "$*"
    ;;
  succ)
    printf "[${green}s${rst}] %s\n" "$*"
    ;;
  log)
    printf "[.] %s\n" "$*"
    ;;
  *)
    exit 1
esac
