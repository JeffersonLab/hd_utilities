#!/bin/csh

setenv PATH /bin/
setenv HOME /home/gxproj7/
echo $PATH
echo $HOME

# SET INPUTS
setenv ENV_FILE $1
setenv INPUT_FILE $2
setenv LUT_FILE $3
setenv CODE_DIR $4
setenv OUT_DIR $5
setenv BAR $6
setenv BIN $7

# PRINT INPUTS
echo "ENV_FILE          = $ENV_FILE"
echo "INPUT_FILE        = $INPUT_FILE"
echo "LUT_FILE          = $LUT_FILE"
echo "CODE_DIR          = $CODE_DIR"
echo "OUT_DIR           = $OUT_DIR"
echo "BAR               = $BAR"
echo "BIN               = $BIN"

# ENVIRONMENT
source $ENV_FILE
echo pwd = $PWD
printenv

ls -l

# INPUT FILES
setenv LOCAL_FILE `basename ${INPUT_FILE}`
ln -sf $INPUT_FILE $LOCAL_FILE
ln -sf $LUT_FILE ./lut_all_avr.root

# LINK SCRIPTS
ln -sf $CODE_DIR/loadlib.C
ln -sf $CODE_DIR/reco_lut_02.C
ln -sf $CODE_DIR/makeEnv.sh
ln -sf $CODE_DIR/compileLib.C
ln -sf $CODE_DIR/glxtools.C
source makeEnv.sh

mkdir -p data/
setenv BAR_PAD `printf "%02d" ${BAR}`
setenv BIN_PAD `printf "%02d" ${BIN}`

# loop over trials until 2 corrections are made
@ trial = 0
while( ! -e "${LOCAL_FILE}.corr_${BAR_PAD}_${BIN_PAD}_level2.root" )
    echo $trial
    ls -l ./*.root
    ls -l data/

    # cor_level = 0 (no corrections)
    # cor_level = 1 (initial time correction)
    root -l -b -q loadlib.C reco_lut_02.C\(\"${LOCAL_FILE}\",\"lut_all_avr.root\",$BAR,$BIN,4.0,0.0,0,0,2\)

    @ trial += 1
    if ( $trial > 4 ) then
	break
    endif
end

ls -l

# cor_level = 2 (full correction, check performance)
# root -l -b -q loadlib.C reco_lut_02.C\(\"${LOCAL_FILE}\",\"lut_all_avr.root\",$BAR,$BIN,3.0,0.0,0,0,2\)
    
#ls -l

# save output histograms
mkdir -p -m 775 ${OUT_DIR}/corr/
mv *corr*.root ${OUT_DIR}/corr/
mkdir -p -m 775 ${OUT_DIR}/data/
mv data/*/* ${OUT_DIR}/data/
mkdir -p -m 775 ${OUT_DIR}/pmt/
mv data/*.pdf ${OUT_DIR}/pmt/

rm ./*.C
rm ./Drc*
rm ./makeEnv.sh

# RETURN CODE
set RETURN_CODE = $?
echo "Return Code = " $RETURN_CODE
if ($RETURN_CODE != 0) then
    exit $RETURN_CODE
endif
