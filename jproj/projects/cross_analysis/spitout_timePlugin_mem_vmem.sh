#!/bin/tcsh

set NVERS = $1
set VER_INIT = $2

if ( $NVERS == "" || ${VER_INIT} == "" ) then
  echo "--------------------------------------------------------------------------------"
  echo "Usage:"
  echo "spitout_timePlugin_mem_vmem.sh [# of versions] [ver # of first launch]"
  echo "--------------------------------------------------------------------------------"
  echo ""
  echo "This script will create a txt file timePlugin_mem_vmem.txt"
  echo "that contains the time to process the plugins, max mem, max vmem"
  echo "across the specified launches for all files"

  exit
endif

set VER_FINAL = `expr ${VER_INIT} + ${NVERS} - 1`

set TIMEPLUGIN_STRING = ""
set MEM_STRING = ""
set VMEM_STRING = ""

foreach VER ( `seq ${VER_INIT} ${VER_FINAL}` )
  set FORMATTED_VER = `printf %02d $VER`

  if ( ${VER} == ${VER_INIT} ) then
    set TIMEPLUGIN_STRING = "timePlugin_ver${FORMATTED_VER}"
    # echo "$VER, $TIMEPLUGIN_STRING"

    set MEM_STRING = "mem_ver${FORMATTED_VER}"
    # echo "$VER, $MEM_STRING"

    set VMEM_STRING = "vmem_ver${FORMATTED_VER}"
    # echo "$VER, $VMEM_STRING"
  else
    set TIMEPLUGIN_STRING = "${TIMEPLUGIN_STRING}, timePlugin_ver${FORMATTED_VER}"
    # echo "$VER, $TIMEPLUGIN_STRING"

    set MEM_STRING = "${MEM_STRING}, mem_ver${FORMATTED_VER}"
    # echo "$VER, $MEM_STRING"

    set VMEM_STRING = "${VMEM_STRING}, vmem_ver${FORMATTED_VER}"
    # echo "$VER, $VMEM_STRING"
  endif

end

# "select run, file, ${TIMEPLUGIN_STRING}, ${MEM_STRING}, ${VMEM_STRING} FROM cross_analysis_table"
time mysql -hhallddb -ufarmer farming -s -r -e "select run, file, ${TIMEPLUGIN_STRING}, ${MEM_STRING}, ${VMEM_STRING} FROM cross_analysis_table" > timePlguin_mem_vmem.txt
