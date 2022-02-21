Python script for creating histograms of tagged and untagged PS flux: Justin Stevens (jrsteven@jlab.org)

The tagged and untagged pair spectrometer flux and acceptance are stored in CCDB.  The command to obtain histograms of the flux is:

```
python plot_flux_ccdb.py --begin-run beginRun --end-run endRun --num-bins 100 --energy-min 6.0 --energy-max 12.0 --rest-ver 3
```

See below for notes on python version if you get an error with this command.

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
  -q RCDB, --rcdb-query='query'
                        RCDB query for run selection
  -t CALIBTIME, --calib-time='T-M-D-h-min-s'
                        CCDB calibtime
  -u UNIFORM, --uniform=UNIFORM
                        Uniform option
  -r REST_VER, --rest-ver=REST_VER
                        REST version option
```

Here beginRun is the beginning of your run range and endRun is the end of your run range.  For example for RunPeriod-2016-02 "golden period" beginRun=11366 and endRun=11555.  

You must also match the REST production version you're analysing by setting the --rest-ver option to the REST version listed on the corresponding TTree in the analysis launch.  See examples at

https://halldweb.jlab.org/wiki-private/index.php/Spring_2016_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Spring_2017_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Spring_2018_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Fall_2018_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Spring_2020_Analysis_Launch

## Prerequisites:

To run this python script you'll need access to CCDB, RCDB and the PyROOT module included in your ROOT build.  On the JLab ifarm this simply requires that you have the following evironment variables set:

ROOTSYS

RCDB_HOME

CCDB_HOME

and for simplicity you can use the standard build_scripts procedure to set these for you or one of the nightly builds with this command (with the date replaced)

```
source /group/halld/Software/build_scripts/gluex_env_nightly.csh 2019-10-08
```

## Python Version Notes:

You need to use a version of Python compatible with that used in your ROOT installation. If you get a python error when executing the script run this command to determine what version of python is used in your ROOT build

root-config --python-version

For older ROOT installations (6.08 or earlier) you will probably use python ver2.7, but for newer builds python ver3.6 is the default.  Now use the correct python version to execute the script

python2.7 plot_flux_ccdb.py ...

or 

python3.6 plot_flux_ccdb.py ...

## The flux values in the MySQL CCDB are from:

RunPeriod-2019-11: REST ver01 production  
RunPeriod-2018-08: REST ver02 production  
RunPeriod-2018-01: REST ver02 production   
RunPeriod-2017-01: REST ver03 production  
RunPeriod-2016-02: REST ver06 production  
