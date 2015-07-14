/* 2015/03/24 Kei Moriya                         */
/*                                               */
/* Create a table for cross-launch analysis.     */
/* This table needs to be created only once,     */
/* and for each launch new columns will be added */
/*                                               */
-- DROP TABLE IF EXISTS `cross_analysis_table`;
CREATE TABLE `cross_analysis_table_2015_03` (
  `id`   int(11) NOT NULL AUTO_INCREMENT,
  `run`  int(11) DEFAULT NULL,
  `file` int(11) DEFAULT NULL,
  `timeChange` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
