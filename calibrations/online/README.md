# Online calibration scripts


**Driver scripts**
* launch_skim_jobs.py, - check DB for new skim file requests, runs file_calib_skim.sh on various nodes (round-robin).  run this somewhere in daemon mode
* run_prompt_calibrations.py, run_online_calibrations.sh - main driver for online calibrations.  looks for new runs, puts in best guesses, updates run conditions, runs most other calibrations
* run_update_skim_files.sh, update_skim_files.py - check the RAID disk for new EVIO files to skim, register them in a DB

**Helper scripts**
* AdjustTiming.C - Determines most timing offsets.  Cribbed from old HLDT scripts, needs to be replaced with python scripts that parallel the procedure used in gluex_demon
* add_consts-adjust.sh - adds new constants to CCDB
* file_calib_check2ns_shift.sh - Runs over events to see if we have some global timing shift due to MO/RF readout calibration, or changes in the injector
* file_calib_pass0.sh - checks RF calibrations
* file_calib_pass1.sh - creates data for timing calibrations
* run_calib_pass1.sh - processes the output of file_calib_pass1.sh
* file_calib_skim.sh - creates calibration skims for one EVIO file
* push_tables_to_production.py - compares calibration values between calib and default variation - copies new constants into default if they change more than some tolerance.  writes report to logbook
* miscellaneous scripts - hdmon_root_utils.py, run_single_root_command.py



**Database tables**

<pre>
 mysql> describe online_info;                                                                                                                                                                           # +---------------+------------+------+-----+---------+-------+                                                                                                                                          # | Field         | Type       | Null | Key | Default | Extra |                                                                                                                                          # +---------------+------------+------+-----+---------+-------+                                                                                                                                          # | run           | int(11)    | YES  |     | NULL    |       |                                                                                                                                          # | done          | tinyint(1) | YES  |     | NULL    |       |                                                                                                                                          # | rcdb_update   | tinyint(1) | YES  |     | NULL    |       |                                                                                                                                          # | launched_skim | tinyint(1) | YES  |     | NULL    |       |                                                                                                                                            +---------------+------------+------+-----+---------+-------+                                                                                                                                           
<pre>
</pre>
 MariaDB [calibInfo]> describe skim_files;
 +----------+---------------+------+-----+---------+-------+
 | Field    | Type          | Null | Key | Default | Extra |
 +----------+---------------+------+-----+---------+-------+
 | run      | int(11)       | YES  |     | NULL    |       |
 | file     | int(11)       | YES  |     | NULL    |       |
 | filepath | varchar(1024) | YES  |     | NULL    |       |
 | done     | int(11)       | YES  |     | NULL    |       |
 +----------+---------------+------+-----+---------+-------+

</pre>