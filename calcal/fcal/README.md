1. fcal-ped

2. fcal-led

Is used to analyze cosmics

a/ hd_root --config=$HD_UTILITIES_HOME/calcal/fcal/fcal-led/jana_data_led_analysis.cfg cosmics.evio -o your-path/tree-cosmics-iter-0.root

b/ Go to $HD_UTILITIES_HOME/calcal/fcal/macro

root -q -b -l 'fcal_cosmics.c("your-path/tree-cosmics-iter-0.root", "old-gain-130000.txt", "new-gain-130000-iter-1.txt")'

3. fcal-cal

4. fcal-time
