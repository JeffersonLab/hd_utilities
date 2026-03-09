#!/bin/bash
# skim out several types of data from EVIO files

RUN=$1
FILE=$2
EVIOFILE=$3

WORKDIR=$4


# probably should check other arguments too
if [ ! -z "$WORKDIR" ]; then
    echo changing working directory to $WORKDIR
    #
    cd $WORKDIR
else
    echo did not set WORKDIR, exiting...
    exit 1
fi


source /gluonwork1/Users/sdobbs/calibration_train/online/setup_gluex.sh
RUN_PERIOD=RunPeriod-2025-01
HOSTNAME=`hostname -s`

# register job
#mysql -h hallddb -u calibInformer calibInfo -e 'INSERT INTO running_jobs (run,file,host) VALUES ($RUN,$FILE,"$HOSTNAME")'

echo ==START SKIMMING RUN $RUN and FILE $FILE ==
date

RUNNUM=`echo ${RUN} | awk '{printf "%06d\n",$0;}'`
FILENUM=`echo ${FILE} | awk '{printf "%03d\n",$0;}'`


# config
#CALIB_PLUGINS=HLDetectorTiming,evio_writer,pi0fcaltofskim,pi0bcalskim,ps_skim,TOF_calib,omega_skim,TAGH_timewalk
CALIB_PLUGINS=HLDetectorTiming,evio_writer,pi0fcaltofskim,pi0bcalskim,TOF_calib,omega_skim,TAGH_timewalk,il_clb_skim
#CALIB_OPTIONS="  -Pjana:nevents=10000  "
CALIB_OPTIONS="  -PFCAL:LOAD_NONLIN_CCDB=0 -PFCAL:expfit_param1=2 -PFCAL:expfit_param2=0 -PFCAL:expfit_param3=0 -PPI0FCALTOFSKIM:WRITE_EVIO=1 -PPI0FCALTOFSKIM:WRITE_HDDM=0 -PPI0FCALTOFSKIM:TURN_OFF_TRACK_MATCH=1 -PPI0FCALTOFSKIM:SAVE_BEAM_PHOTON=1 -PPI0FCALTOFSKIM:SAVE_TOF=1 -PPI0FCALTOFSKIM:GET_IP=1  "
#PASS2_OUTPUT_FILENAME=hd_calib_pass1_Run${RUN}_${FILE}.root
OUTPUT_FILENAME=hd_skim_Run${RUNNUM}_File${FILENUM}.root
# run
echo ==skimming==
echo Running these plugins: $CALIB_PLUGINS
echo cmd: hd_root -Pnthreads=$NTHREADS  -PEVIO:RUN_NUMBER=${RUNNUM} -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$OUTPUT_FILENAME -PPLUGINS=$CALIB_PLUGINS $CALIB_OPTIONS $EVIOFILE
timeout 7200 hd_root -Pnthreads=$NTHREADS  -PEVIO:RUN_NUMBER=${RUNNUM} -PPRINT_PLUGIN_PATHS=1 -PTHREAD_TIMEOUT=300 -POUTPUT_FILENAME=$OUTPUT_FILENAME -PPLUGINS=$CALIB_PLUGINS $CALIB_OPTIONS $EVIOFILE
retval=$?

echo ==copying skims==
date


# remove job
mysql -h hallddb -u calibInformer calibInfo -e "DELETE FROM running_jobs WHERE run=$RUN and file=$FILE"


# save the results

SKIM_DIR=/gluonraid4/data1/online_skimming/${RUN_PERIOD}/
mkdir -p $SKIM_DIR


mkdir -p ${SKIM_DIR}/hists/Run${RUN}
cp -v $OUTPUT_FILENAME  ${SKIM_DIR}/hists/Run${RUN}/$OUTPUT_FILENAME
chmod g+wx ${SKIM_DIR}/hists/Run${RUN}/
chmod g+w ${SKIM_DIR}/hists/Run${RUN}/$OUTPUT_FILENAME
mkdir -p ${SKIM_DIR}/BCAL_pi0/Run${RUN}
cp -v  hd_rawdata_${RUNNUM}_${FILENUM}.pi0bcalskim.evio ${SKIM_DIR}/BCAL_pi0/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.pi0bcalskim.evio
chmod g+wx ${SKIM_DIR}/BCAL_pi0/Run${RUN}/
chmod g+w ${SKIM_DIR}/BCAL_pi0/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.pi0bcalskim.evio
mkdir -p ${SKIM_DIR}/FCAL_pi0/Run${RUN}
cp -v  hd_rawdata_${RUNNUM}_${FILENUM}.pi0fcaltofskim.evio ${SKIM_DIR}/FCAL_pi0/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.pi0fcaltofskim.evio
chmod g+wx ${SKIM_DIR}/FCAL_pi0/Run${RUN}/
chmod g+w ${SKIM_DIR}/FCAL_pi0/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.pi0fcaltofskim.evio
mkdir -p ${SKIM_DIR}/FCAL_pi0_outer/Run${RUN}
cp -v  hd_rawdata_${RUNNUM}_${FILENUM}.fcal_outer.evio ${SKIM_DIR}/FCAL_pi0_outer/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.fcal_outer.evio
chmod g+wx ${SKIM_DIR}/FCAL_pi0_outer/
chmod g+w ${SKIM_DIR}/FCAL_pi0_outer/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.fcal_outer.evio
mkdir -p ${SKIM_DIR}/trd/Run${RUN}
cp -v  hd_rawdata_${RUNNUM}_${FILENUM}.trd.evio ${SKIM_DIR}/trd/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.trd.evio
chmod g+wx  ${SKIM_DIR}/trd/Run${RUN}/
chmod g+w  ${SKIM_DIR}/trd/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.trd.evio
mkdir -p ${SKIM_DIR}/omega/Run${RUN}
cp -v  hd_rawdata_${RUNNUM}_${FILENUM}.omega.evio ${SKIM_DIR}/omega/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.omega.evio
chmod g+wx ${SKIM_DIR}/omega/Run${RUN}/
chmod g+w ${SKIM_DIR}/omega/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.omega.evio
#mkdir -p ${SKIM_DIR}/PS/Run${RUN}
#cp -v  data.ps.evio ${SKIM_DIR}/PS/Run${RUN}/hd_rawdata_${RUNNUM}_${FILENUM}.ps.evio
#mkdir -p ${SKIM_DIR}/TOF/Run${RUN}
#cp -v  hd_root_tofcalib.root ${SKIM_DIR}/TOF/Run${RUN}/hd_root_tofcalib_${RUNNUM}_${FILENUM}.root


echo ==done==

exit $retval
