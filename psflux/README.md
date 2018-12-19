Python script for creating histograms of tagged and untagged PS flux: Justin Stevens (jrsteven@jlab.org)

The tagged and untagged pair spectrometer flux and acceptance are stored in CCDB.  The command to obtain histograms of the flux is:

```
plot_flux_ccdb.py --begin-run beginRun --end-run endRun --num-bins 100 --energy-min 6.0 --energy-max 12.0
```

## Command line options:

If you run the script without any arguments you'll receive this message with a list of required and optional arguments

```
Usage: plot_flux_ccdb.py --begin-run beginRun --end-run endRun

Options:
  -b BEGIN_RUN, --begin-run=BEGIN_RUN
                        Starting run for output
  -e END_RUN, --end-run=END_RUN
                        Ending run for output
  -p POL, --pol=POL     Polarization in RCDB
  -a ANGLE, --angle=ANGLE
                        Polarization angle in RCDB
  -n NBINS, --num-bins=NBINS
                        Number of histogram bins
  -m EMIN, --energy-min=EMIN
                        Minimum energy for flux histogram
  -x EMAX, --energy-max=EMAX
                        Maximum energy for flux histogram
  -r RCDB, --rcdb-query='query'
                        RCDB query for run selection
```

Here beginRun is the beginning of your run range and endRun is the end of your run range.  For example for RunPeriod-2016-02 "golden period" beginRun=11366 and endRun=11555

## Prerequisites:

To run this python script you'll need access to CCDB, RCDB and the PyROOT module included in your ROOT build.  On the JLab ifarm this simply requires that you have the following evironment variables set:

ROOTSYS

RCDB_HOME

CCDB_HOME

and for simplicity you can use the standard build_scripts procedure to set these for you or one of the nightly builds with this command (with the date replaced)

```
source /group/halld/Software/build_scripts/gluex_env_nightly.csh 2018-11-11
```

## Notes:

The flux values in the MySQL CCDB are from:

RunPeriod-2017-01: REST ver03 production

RunPeriod-2016-02: REST ver06 production
