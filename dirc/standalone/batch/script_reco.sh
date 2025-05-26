#!/bin/csh

setenv PATH /bin/
setenv HOME /home/gxproj7/
echo $PATH
echo $HOME

# SET INPUTS
setenv ENVIRONMENT $1
setenv INPUT_FILE $2
setenv LUT_FILE $3
setenv CODE_DIR $4
setenv OUTDIR $5
setenv BAR $6
setenv BIN $7

# PRINT INPUTS
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "INPUT_FILE        = $INPUT_FILE"
echo "LUT_FILE          = $LUT_FILE"
echo "CODE_DIR          = $CODE_DIR"
echo "OUTDIR            = $OUTDIR"
echo "BAR               = $BAR"
echo "BIN               = $BIN"

# ENVIRONMENT
source $ENVIRONMENT
echo pwd = $PWD
printenv

ls -l

# INPUT FILES
#ln -sf $INPUT_FILE ./hd_root.root
ln -sf $LUT_FILE ./lut_all_avr.root

# LINK SCRIPTS
ln -sf $CODE_DIR/loadlib.C
ln -sf $CODE_DIR/reco_lut_02.C
ln -sf $CODE_DIR/makeEnv.sh
ln -sf $CODE_DIR/compileLib.C
ln -sf $CODE_DIR/glxtools.C
source makeEnv.sh

ls -l

ls ${INPUT_FILE}*.root

# cor_level = 0 (no corrections)
root -l -b -q loadlib.C reco_lut_02.C\(\"${INPUT_FILE}\",\"lut_all_avr.root\",$BAR,$BIN,4.0,0.0,0,0,2\)
cp hd_root.corr_${BAR}_${BIN}.root hd_root.corr_${BAR}_${BIN}_level1.root
    
# cor_level = 1 (initial time correction)
root -l -b -q loadlib.C reco_lut_02.C\(\"${INPUT_FILE}\",\"lut_all_avr.root\",$BAR,$BIN,4.0,0.0,0,0,2\)
cp hd_root.corr_${BAR}_${BIN}.root hd_root.corr_${BAR}_${BIN}_level2.root
    
# cor_level = 2 (full correction, check performance)
root -l -b -q loadlib.C reco_lut_02.C\(\"${INPUT_FILE}\",\"lut_all_avr.root\",$BAR,$BIN,3.5,0.0,0,0,2\)
    
# RETURN CODE
set RETURN_CODE = $?
echo "Return Code = " $RETURN_CODE
if ($RETURN_CODE != 0) then
    exit $RETURN_CODE
endif

# save output histograms
mkdir -p -m 775 ${OUTDIR}/corr/
mv *corr*.root ${OUTDIR}/corr/
mkdir -p -m 775 ${OUTDIR}/data/
mv data/*/* ${OUTDIR}/data/

rm ./*.C
rm ./Drc*
rm ./makeEnv.sh
