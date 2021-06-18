This directory contains scripts useful for extracting the hit thresholds from configuration files and for calculating the correction to Garfield's drift time tables due to the magnetic field.

# Finding the hit thresholds 

This requires the run configuration files, either in the RunLog tar file stored on tape with the evio files, or on the gluons, in /gluex/CALIB/ALL/fadc125/.  Their filenames follow the pattern roccdc1something.cnf.

The scripts are in the subdirectory hit_thresholds.

geth.C extracts the thresholds from the configuration files and writes them into a simple table suitable for CCDB.
CDC\_straw\_numbers\_run\_3221.txt is a reference table used by geth.C

geth.C creates the following file:
- cdc_h.txt  This is a list of the thresholds, ordered by straw number


**To extract the thresholds:**

1. Make a new directory and cd into it
2. Copy geth.C, CDC\_straw\_numbers\_run\_3221.txt and the configuration files into this directory.  
3. Edit the configuration filename in the script, line 65.
4. Run the script 
```sh
root -q geth.C 
```
5. Load the data into CCDB, eg
```sh
ccdb add /CDC/hit_thresholds -r 71860-72435 cdc_h.txt
```


# Calculating the correction to Garfield's drift time tables for the magnetic field

The process is documented in GlueX-Docs 2512 and 2593.
The script is in the subdirectory Bfield\_drift\_correction.

**To find the correction function:**

1. Obtain the magnetic field map as a root tree, using the program bfield2root in src/programs/Utilities

2. Scan the tree to find the range of Bz in the CDC volume 

```sh
Bfield->Scan("Bz:r:z","Bz<1.25&&z>=15&&z<=170&&r>=5&&r<=65")
Bfield->Scan("Bz:r:z","Bz>1.92&&z>=15&&z<=170&&r>=5&&r<=65")
```

3. Generate Garfield time-to-distance tables to cover the range of Bz, at 0.05T intervals 

4. Edit the script Bfield_dt2.C as necessary and run it.  
```sh
root -q Bfield_dt2.C
```

5. Add the fit parameters obtained to the CCDB table /CDC/cdc_drift_parms.