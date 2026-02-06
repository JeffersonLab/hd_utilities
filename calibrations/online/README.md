# Online calibration scripts


**Driver scripts**


**Helper scripts**
* AdjustTiming.C -





**Database tables**

<pre>
 mysql> describe online_info;                                                                                                                                                                           # +---------------+------------+------+-----+---------+-------+                                                                                                                                          # | Field         | Type       | Null | Key | Default | Extra |                                                                                                                                          # +---------------+------------+------+-----+---------+-------+                                                                                                                                          # | run           | int(11)    | YES  |     | NULL    |       |                                                                                                                                          # | done          | tinyint(1) | YES  |     | NULL    |       |                                                                                                                                          # | rcdb_update   | tinyint(1) | YES  |     | NULL    |       |                                                                                                                                          # | launched_skim | tinyint(1) | YES  |     | NULL    |       |                                                                                                                                            +---------------+------------+------+-----+---------+-------+                                                                                                                                           

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