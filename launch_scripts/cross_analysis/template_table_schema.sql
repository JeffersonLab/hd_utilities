/* template schema for cross analysis table */

CREATE TABLE `TABLENAME` (
  `id`         int(11) NOT NULL AUTO_INCREMENT,
  `run`        int(8) DEFAULT NULL,
  `file`       int(4) DEFAULT NULL,
  `timeChange` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `cpu_sec`    int(11)   DEFAULT NULL,
  `wall_sec`   int(11)   DEFAULT NULL,
  `mem_kb`     int(11)   DEFAULT NULL,
  `vmem_kb`    int(11)   DEFAULT NULL,
  `nevents`        int(11)   DEFAULT NULL,
  `input_copy_sec` int(11)   DEFAULT NULL,
  `plugin_sec`     int(11)   DEFAULT NULL,
  `final_state`    char(20)  DEFAULT NULL,
  `problems`       char(160) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
