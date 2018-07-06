########## Instructions to setup DIRC environment ###########

Use 2018-dirc-commissioning branch for: 
	-HDDS (https://github.com/JeffersonLab/hdds)
	-sim-recon (https://github.com/JeffersonLab/sim-recon)
	-HDGeant4 (https://github.com/JeffersonLab/HDGeant4)

To setup your own builds of with these branches use the following commands:

> setenv BUILD_SCRIPTS /group/halld/Software/build_scripts
> mkdir builds
> cd builds
> $BUILD_SCRIPTS/my_sim-recon_jlab.sh

This will pull the master of each repository and build it, after this you'll need to switch to the proper branch

> cd hdds/
> git checkout dirc_commissioning_2018
> scons -u install
> cd ../sim-recon/src/
> git checkout dirc_commissioning_2018
> scons -u install -j8
> cd ../../hdgeant4/
> git checkout dirc_commissioning_2018
> make

Finally, you're environment can be set using the builds/setup_gluex.csh file creted by the build scripts.  In order to use the local xml geometry with the DIRC you'll need to add the line below to your setup_gluex.csh script so the local geometry definition is used instead of CCDB

setenv JANA_GEOMETRY_URL xmlfile://${HDDS_HOME}/main_HDDS.xml

Now your environment is set to run DIRC simulations!

-------------------------------------------------------------------------

####### Example DIRC simulation workflow #######

#1) Set particle generator/source by using control.in file and run.mac files
# Set KINE and SCAP cards for single particle MC in control.in
#	KINE controls momentum (GeV/c) and angle (degrees) of paticle gun
#	SCAP controls vertex postion of particle gun: should be (0., 0., 65.) 
# Set number of events to generate using run.mac
#	/run/beamOn 100: will generate 100 events

#2) Generate single track events and smear hits
hdgeant4 run.mac
mcsmear out.hddm

#3) Analyze smeared .hddm output with several plugins (Note: use -PTRIG:BYPASS=1 to skip trigger simulation)
#	pid_dirc: convert DIRC hits into custom .root format for LUT algorithm
#	truth_dirc: make diagnostic truth histograms from DIRC hits
#	monitoring_hists: make diagnostic histograms for other detectors (eg. check tracking is working)
hd_root -o drc.root -PPLUGINS=pid_dirc,truth_dirc,monitoring_hists -PTRIG:BYPASS=1 out_smeared.hddm

#4) Plot DIRC's hit pattern:
root loadlib.C drawHP.C'("drc.root")'

For a GUI with 3D visualization one need to have OpenGl and Qt libraries installed.
Also -DG4UI_USE_EXECUTIVE flag needs to be added in GNUmakefile

####### LUT creation #######

#1) Generate photons from the end of a single bar for (1M events in run.mac)
#	DIRCLUT card in control.in controls which bar the LUT will be generated for
hdgeant4 run.mac

#2) Create ROOT tree with LUT from lut_dirc plugin for a single bar:
hd_root -nthreads=10 -o lut.root --plugin=lut_dirc out.hddm

#3) Averege LUT directions for a single bar:
root loadlib.C glxlut_avr.C+'("lut.root")'

#3.5) Add step for generating LUT for all bars via batch system

#4) Combine averaged LUT tables from many bars into single file:
root loadlib.C glxlut_add.C+'("lut/loopLut/lut_avr_*.root")'

####### LUT reconstruction (standalone macro) #######

#1) Simulate pion and kaon samples with fixed momentum and anglues (see example simulation workflow above).  Then combine into a single ROOT file with hadd.

#2) Run the LUT reconstruction macro to show the pi/K separation power
root loadlib.C reco_lut.C'("drc_kaon_pion.root")'

####### LUT reconstruction (JANA factory) #######

Work in progress...

