Python script for creating histograms of tagged and untagged PS flux: Justin Stevens (jrsteven@jlab.org)

The tagged and untagged pair spectrometer flux and acceptance are stored in CCDB.  The command to obtain histograms of the flux is:

python plot_flux_ccdb.py -b beginRun -e endRun

where beginRun is the beginning of your run range and endRun is the end of your run range.  For example for RunPeriod-2016-02 "golden period" beginRun=11366 and endRun=11555

Prerequisites:

To run this python script you'll need access to CCDB, RCDB and the PyROOT module included in your ROOT build.  On the JLab ifarm this simply requires that you have the following evironment variables set:

ROOTSYS
RCDB_HOME
CCDB_HOME

and for simplicity you can use the standard build_scripts procedure to set these for you.

Notes:

The flux values in the MySQL CCDB are from:

RunPeriod-2017-01: REST ver03 production
RunPeriod-2016-02: REST ver06 production
