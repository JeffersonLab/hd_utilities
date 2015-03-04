-- DROP TABLE IF EXISTS `offline_monAux`;
CREATE TABLE `offline_monitoring_RunPeriod2014_10_ver11_hd_rawdata_aux` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `jobId` int(11) DEFAULT NULL,
  `timeChange` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `nevents` int(11) DEFAULT NULL,
  `timeCopy` int(11) DEFAULT NULL,
  `timePlugin` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
