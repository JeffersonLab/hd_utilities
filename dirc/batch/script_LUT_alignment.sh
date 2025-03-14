#!/bin/csh -f

# SET INPUTS
setenv ENVIRONMENT $1
setenv BUILDDIR $2
setenv OUTDIR $3
setenv VARIATION $4

# PRINT INPUTS
echo "ENVIRONMENT       = $ENVIRONMENT"
echo "BUILDDIR          = $BUILDDIR"
echo "OUTDIR            = $OUTDIR"
echo "VARIATION         = $VARIATION"

# ENVIRONMENT
source $ENVIRONMENT
echo pwd = $PWD
printenv

ls -l

# get local copy of HDDS with modified geometry
cp $BUILDDIR/hdds/ ./ -r
cp $OUTDIR/var$VARIATION/DIRC_HDDS.xml ./
cd hdds/
scons -u install
cd ../

# set geometry to use local version
setenv JANA_GEOMETRY_URL xmlfile://`pwd`/hdds/main_HDDS.xml
cp $HD_UTILITIES_HOME/dirc/batch/run.mac ./

# macros for averaging LUT
ln -s $HD_UTILITIES_HOME/dirc/loadlib.C
ln -s $HD_UTILITIES_HOME/dirc/glxlut_avr.C

# loop over bars to create LUT
foreach BAR (`seq 0 47`)

    # skip all but 2 bars in each bar box (8 bars total)

    cp $HD_UTILITIES_HOME/dirc/batch/control_$BAR.in control.in
    ls -l
    
    # HDGEANT4
    which hdgeant4
    hdgeant4 run.mac

    # RUN JANA
    which hd_root
    hd_root out.hddm -o lut.root -PPLUGINS=lut_dirc

    # RETURN CODE
    set RETURN_CODE = $?
    echo "Return Code = " $RETURN_CODE
    if ($RETURN_CODE != 0) then
	exit $RETURN_CODE
    endif

    # save output histograms
    mkdir -p -m 775 ${OUTDIR}/root/
    if (-e lut.root) then
    	cp -v lut.root ${OUTDIR}/root/lut_${BAR}.root
    	chmod 664 ${OUTDIR}/root/lut_${BAR}.root
    endif

    # save output hddm
    mkdir -p -m 775 ${OUTDIR}/hddm/
    if (-e out.hddm) then
        cp -v out.hddm ${OUTDIR}/hddm/lut_${BAR}.hddm
	chmod 664 ${OUTDIR}/hddm/lut_${BAR}.hddm
    endif

    # average LUT
    root -l -b -q loadlib.C glxlut_avr.C+
    if (-e lut_avr.root) then
    	cp -v lut_avr.root ${OUTDIR}/root/lut_avr_${BAR}.root
    	chmod 664 ${OUTDIR}/root/lut_avr_${BAR}.root
    endif

end
