PERIOD=RunPeriod-2017-01
VER=ver03

for file in `ls $PERIOD/$VER/tagmTaggedFluxRun*.txt`
do

run=${file:41:5}
echo $run

ccdb add PHOTON_BEAM/pair_spectrometer/lumi/tagm/tagged -r $run-$run /work/halld2/home/jrsteven/tables/$PERIOD/$VER/tagmTaggedFluxRun$run.txt
ccdb add PHOTON_BEAM/pair_spectrometer/lumi/tagm/untagged -r $run-$run /work/halld2/home/jrsteven/tables/$PERIOD/$VER/tagmUntaggedFluxRun$run.txt

ccdb add PHOTON_BEAM/pair_spectrometer/lumi/tagh/tagged -r $run-$run /work/halld2/home/jrsteven/tables/$PERIOD/$VER/taghTaggedFluxRun$run.txt
ccdb add PHOTON_BEAM/pair_spectrometer/lumi/tagh/untagged -r $run-$run /work/halld2/home/jrsteven/tables/$PERIOD/$VER/taghUntaggedFluxRun$run.txt

ccdb add PHOTON_BEAM/pair_spectrometer/lumi/trig_live -r $run-$run /work/halld2/home/jrsteven/tables/$PERIOD/$VER/livetimeRun$run.txt

done
