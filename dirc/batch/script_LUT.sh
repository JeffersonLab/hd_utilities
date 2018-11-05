#!/bin/csh -f

# SET INPUTS
setenv ENVIRONMENT $1
setenv CONFIG_FILE $2
setenv MAC_FILE $3
setenv OUTDIR $4
setenv BAR $5

# PRINT INPUTS
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "CONFIG_FILE       = $CONFIG_FILE"
echo "MAC_FILE          = $MAC_FILE"
echo "OUTDIR            = $OUTDIR"
echo "BAR               = $BAR"

# ENVIRONMENT
source $ENVIRONMENT
echo pwd = $PWD
printenv

ls -l
cp $CONFIG_FILE control.in

# HDGEANT4
hdgeant4 $MAC_FILE

# RUN JANA
#hd_root out.hddm -o lut.root -PPLUGINS=lut_dirc

# RETURN CODE
set RETURN_CODE = $?
echo "Return Code = " $RETURN_CODE
if ($RETURN_CODE != 0) then
	exit $RETURN_CODE
endif

# save output histograms
#mkdir -p -m 775 ${OUTDIR}/root/
#if (-e lut.root) then
#	cp -v lut.root ${OUTDIR}/root/lut_${BAR}.root
#	chmod 664 ${OUTDIR}/root/lut_${BAR}.root
#endif

# save output hddm
mkdir -p -m 775 ${OUTDIR}/hddm/
if (-e out.hddm) then
        cp -v out.hddm ${OUTDIR}/hddm/lut_${BAR}.hddm
	chmod 664 ${OUTDIR}/hddm/lut_${BAR}.hddm
endif

