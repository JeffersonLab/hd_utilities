# Locating the Data
At the time of writing, the CDC_TimeToDistance plugin is included in the offline monitoring at JLab. The general location is

/cache/halld/offline_monitoring/RunPeriod-YYYY-MM/verXX/hists/hists_merged

The instructions below assume that the scripts are run at JLab where the offline_monitoring was run with the plugin.

# Setting up the directory
Use a directory outside of sim-recon for checking the results of the CDC_TimeToDistance plugin. Inside this directory, make a symlink to the merged histograms from the offline monitoring:

ln -s /cache/halld/offline_monitoring/RunPeriod-YYYY-MM/verXX/hists/hists_merged hists_merged

This will make it easy to access the files and will work properly with the script.

Additionally, making symlinks to the scripts will also make life easier.

ln -s $GLUEX_RECON_HOME/src/plugins/Calibration/CDC_TimeToDistance/FitScripts/run.py run.py

ln -s $GLUEX_RECON_HOME/src/plugins/Calibration/CDC_TimeToDistance/FitScripts/FitTimeToDistance FitTimeToDistance.C

ln -s $GLUEX_RECON_HOME/src/plugins/Calibration/CDC_TimeToDistance/FitScripts/display.py display.py

# Running the scripts
With everything prepared as described above, run the python script `run.py`

python run.py

This will create a set of output directories for various PNG images that will be useful to check the calibration. It will also create a directory containing the text files needed for updating CCDB. Once the directories are created, the script finds all of the runs in the hists_merged directory and executes FitTimeToDistance.C on each of the files.

Once this completes, each new directory should be filled with images or text files. To view the relevant plots on a single canvas for all runs, use `display.py`

python display.py

This script assumes that the command `display <image>.png` works, which should be true on JLab machines.

A single canvas will appear with 5 plots. Starting on the left of the top row, this is the result with the initial CCDB constants. Moving to the right, this is what it will look like with the newly calculated constants. The final plot in this row shows the difference between the before and after. The bottom row shows the residuals as a function of drift time with the final plot being a projection onto the residual axis.

To advance to the next run, close the window and hit enter in the terminal.
