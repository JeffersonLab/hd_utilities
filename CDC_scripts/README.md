# Finding the hit thresholds 

This requires the run configuration files, either in the RunLog tar file stored on tape with the evio files, or on the gluons, in /gluex/CALIB/ALL/fadc125/.
The scripts are in the subdirectory scripts_mc.

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
