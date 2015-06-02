#!/bin/tcsh

set VER_INIT = $1
set VER_LAST = $2
set NVERS = `expr ${VER_LAST} - ${VER_INIT} + 1`

if ( $NVERS == "" || ${VER_INIT} == "" ) then
  echo "--------------------------------------------------------------------------------"
  echo "Usage:"
  echo "spitout_timePlugin_mem_vmem.sh [ver # of first launch] [ver # of last launch] "
  echo "--------------------------------------------------------------------------------"
  echo ""
  echo "This script will create a txt file timePlugin_mem_vmem.txt"
  echo "that contains the time to process the plugins, max mem, max vmem"
  echo "across the specified launches for all files"

  exit
endif

set TIMEPLUGIN_STRING = ""
set MEM_STRING = ""
set VMEM_STRING = ""
set NEVENTS_STRING = ""

foreach VER ( `seq ${VER_INIT} ${VER_LAST}` )
  set FORMATTED_VER = `printf %02d $VER`

  # SKIPPED LAUNCHES
  if ( ${FORMATTED_VER} == 14) then
    echo "skipping VER${VER}"
    continue
  endif

  if ( ${VER} == ${VER_INIT} ) then
    set TIMEPLUGIN_STRING = "timePlugin_ver${FORMATTED_VER}"
    # echo "$VER, $TIMEPLUGIN_STRING"

    set MEM_STRING = "mem_ver${FORMATTED_VER}"
    # echo "$VER, $MEM_STRING"

    set VMEM_STRING = "vmem_ver${FORMATTED_VER}"
    # echo "$VER, $VMEM_STRING"

    set NEVENTS_STRING = "nevents_ver${FORMATTED_VER}"
    # echo "$VER, $NEVENTS_STRING"
  else
    set TIMEPLUGIN_STRING = "${TIMEPLUGIN_STRING}, timePlugin_ver${FORMATTED_VER}"
    # echo "$VER, $TIMEPLUGIN_STRING"

    set MEM_STRING = "${MEM_STRING}, mem_ver${FORMATTED_VER}"
    # echo "$VER, $MEM_STRING"

    set VMEM_STRING = "${VMEM_STRING}, vmem_ver${FORMATTED_VER}"
    # echo "$VER, $VMEM_STRING"

    set NEVENTS_STRING = "${NEVENTS_STRING}, nevents_ver${FORMATTED_VER}"
    # echo "$VER, $NEVENTS_STRING"
  endif

end

# "select run, file, ${TIMEPLUGIN_STRING}, ${MEM_STRING}, ${VMEM_STRING}, ${NEVENTS_STRING} FROM cross_analysis_table"
time mysql -hhallddb -ufarmer farming -s -r -e "select run, file, ${TIMEPLUGIN_STRING}, ${MEM_STRING}, ${VMEM_STRING}, ${NEVENTS_STRING} FROM cross_analysis_table" > timePlguin_mem_vmem.txt
