
# Naomi's collection of cron jobs

These are running on gluon153.  The first one has to run from the hdops account.

## save\_monitoring\_plots

- This waits until a specified number of events have been accumulated, and then uses the RootSpy macro to save the monitoring plots to the logbook.
- NB the list of plots is hard-coded in the python script. 


## watch\_mya

- This monitors EPICS to find when the daq is on with a radiator in place and beam on. 
- It checks whether rates are zero or frozen for the trigger, tagger scalers, and (if there is a converter) the PS & PSC.
- It also checks PVs used by RoboCDC just to make sure that they have not disconnected for a substantial time.
- It makes a logbook entry if it finds any problems.
