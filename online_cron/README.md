
# Naomi's collection of cron jobs

These are running on gluon153.  The first one has to run from the hdops account.

# save\_monitoring\_plots

- This waits until a specified number of events have been accumulated, and then uses the RootSpy macro to save the monitoring plots to the logbook.
- NB the list of plots is hard-coded in the python script. 


# watch\_epics

- This monitors EPICS to find when the daq is on with a radiator in place and beam on. 
- If so, it makes a logbook entry if there is a PS converter and the PS rates are not changing over a short period (minutes).
- Also, it makes a logbook entry if the trigger rate is 0.
- Regardless of the above, it makes a logbook entry if any of the EPICS PVs used by RoboCDC are unavailable.


# watch\_for\_frozen\_trigger

- This works like watch\_epics, but warns if the trigger rate is non-zero and not changing over a long time period (hours).
- When running at low rates the trigger rate does apparently not change enough to update EPICS.
- It was easier to make a second cron job to handle the different integration period than to adapt watch\_epics to do both.
