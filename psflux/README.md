Python script for creating histograms of tagged and untagged PS flux: Justin Stevens (jrsteven@jlab.org)

The tagged and untagged pair spectrometer flux and acceptance are stored in CCDB.  The command to obtain histograms of the flux is:

```
python plot_flux_ccdb.py --begin-run beginRun --end-run endRun --num-bins 100 --energy-min 6.0 --energy-max 12.0 --ana-ver analysisLaunchVersion
```

See below for notes on python version if you get an error with this command.

## Command line options:

The begin-run, end-run, and ana-ver options are required. The script accepts these options:

```
-v, --ana-ver           Analysis Launch version (required)
-b, --begin-run         Starting run for output
-e, --end-run           Ending run for output
-p, --pol               Polarization in RCDB
-a, --angle             Polarization angle in RCDB
-n, --num-bins          Number of histogram bins
-m, --energy-min        Minimum energy for flux histogram
-x, --energy-max        Maximum energy for flux histogram
-q, --rcdb-query        RCDB query for run selection
-t, --calib-time        CCDB calibration time (Y-M-D-h-min-s)
-u, --uniform           Uniform option
-l, --target-length     Target length in cm
```

Here beginRun is the beginning of your run range and endRun is the end of your run range. For example, for the RunPeriod-2017-01, beginRun=30274 and endRun=31057.

You must specify `--ana-ver` with the version of the Analysis Launch used for your analysis. This version must match the launch whose TAGM/TAGH energy calibration you need: the script uses it to select the corresponding CCDB context and energy-calibration timestamp. Using a different launch version can therefore give flux histograms with TAGM/TAGH energies that do not match your analysis. Do not pass the REST production version here; the tool obtains the associated REST context from the selected Analysis Launch version. See the corresponding Analysis Launch page for the version:

https://halldweb.jlab.org/wiki-private/index.php/Spring_2016_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Spring_2017_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Spring_2018_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Fall_2018_Analysis_Launch  
https://halldweb.jlab.org/wiki-private/index.php/Spring_2020_Analysis_Launch
https://halldweb.jlab.org/wiki-private/index.php/Spring_2023_Analysis_Launch

## Prerequisites:

To run this python script you'll need access to CCDB, RCDB and the PyROOT module included in your ROOT build.  On the JLab ifarm this simply requires that you have the following evironment variables set:

ROOTSYS

RCDB_HOME

CCDB_HOME

and for simplicity you can use the standard build_scripts procedure to setup your environment

```
source /group/halld/Software/build_scripts/gluex_env_jlab.csh
```

The location of the plot_flux_ccdb.py script will then be located at $HD_UTILITIES_HOME/psflux/plot_flux_ccdb.py in your environment.

## Python Version Notes:

You need to use a version of Python compatible with that used in your ROOT installation. If you get a python error when executing the script run this command to determine what version of python is used in your ROOT build

root-config --python-version

For older ROOT installations (6.08 or earlier) you will probably use python ver2.7, but for newer builds python ver3.6 is the default.  Now use the correct python version to execute the script

python2.7 $HD_UTILITIES_HOME/psflux/plot_flux_ccdb.py ...

or 

python3.6 $HD_UTILITIES_HOME/psflux/plot_flux_ccdb.py ...