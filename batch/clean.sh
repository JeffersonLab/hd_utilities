#!/bin/tcsh

##############################################################
#                                                            #
#   Clean files created by generatejobs_plugins_rawdata.sh   #
#                                                            #
##############################################################

set RUN = $1

if( $RUN == "" ) then
  echo "Usage:"
  echo "clean.sh [run #]"
  echo "Will remove all auxiliary files produced by generatejobs_plugins_rawdata.sh"
endif

set FORMATTED_RUN = `printf %06d $RUN`

rm -f ls-evio_${FORMATTED_RUN}
rm -f run_rawdata_${FORMATTED_RUN}.sh
rm -fr ${FORMATTED_RUN}
