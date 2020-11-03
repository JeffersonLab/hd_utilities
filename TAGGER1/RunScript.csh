#!/usr/bin/tcsh

echo $1
echo $2
echo $3
echo $4

source $1

cp $3 ./localfile.evio
mkdir Run$2_$4
cd Run$2_$4

#setenv JANA_CALIB_URL `$BUILD_SCRIPTS/calib_url_chooser.sh` 
#setenv JANA_CALIB_CONTEXT "variation=beni"

hd_root --nthreads=4 -PPLUGINS=TAGGER1,PS_flux,BEAM_online -PTHREAD_TIMEOUT=100 -PJANA:MAX_RELAUNCH_THREADS=10  ../localfile.evio

echo ls
ls

cd ../
cp -rp Run$2_$4 /w/halld-scifs17exp/halld2/home/zihlmann/HallD/work/tagger/localdir/

