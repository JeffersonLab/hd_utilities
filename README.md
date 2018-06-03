
## GlueX Computing Resource Model

This directory contains files used to estimate comoputing resource requirements
for GlueX. Originally this was done via spreadsheet. That has been changed to
use a python script to read the values from an input file. This makes it easier to
manage input parameters for various RunPeriods separately from the model
calculations. The older spreadsheets can be found in the ''Spreadsheets'' directory.

To get a summary of the computing resources run the comp_mod.py script, giving
it an input XML file. (See example below) 

```
>./comp_mod.py RunPeriod-2017-01.xml 

           GlueX Computing Model
           RunPeriod-2017-01.xml
==========================================
         Trigger Rate: 40.0 kHz
             PAC Time: 2.9 weeks
         Running Time: 5.7 weeks
   Running Efficiency: 48%
  --------------------------------------
  Reconstruction Rate: 5.0 Hz/core
       Available CPUs: 4500 cores (full)
      Time to process: 10.2 weeks (all passes)
     Number of passes: 2.1
   Reconstruction CPU: 7.7 Mcore-hr
  --------------------------------------
 Raw Data Num. Events:  66.4 billion
 Raw Data compression: 1.00
  Raw Data Event Size: 12.7 kB 
    Max Raw Data Rate: 0.52 GB/s 
      Raw Data Volume: 0.863 PB 
 Bandwidth to offsite: 328 MB/s (all raw data in 1 month)
  REST/Raw size frac.: 14.60%
     REST Data Volume: 0.265 PB (for 2.1 passes)
  --------------------------------------
   MC generation Rate: 25.0 Hz/core
  MC Number of passes: 2.0
  MC events/raw event: 0.60
       MC data volume: 0.151 PB  (REST only)
    MC Generation CPU: 0.9 Mcore-hr
MC Reconstruction CPU: 4.4 Mcore-hr
               MC CPU: 5.3 Mcore-hr
  --------------------------------------
               TOTALS:
                  CPU: 13.0 Mcore-hr
                 TAPE: 1.3 PB
```

