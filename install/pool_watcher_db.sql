-- Adminer 4.2.4 MySQL dump

SET NAMES utf8;
SET time_zone = '+00:00';
SET foreign_key_checks = 0;
SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO';

DROP TABLE IF EXISTS `pools`;
CREATE TABLE `pools` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pool_name` varchar(20) NOT NULL,
  `pool_host` varchar(50) NOT NULL,
  `pool_port` int(11) NOT NULL,
  `worker_name` varchar(30) NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `pool_name` (`pool_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `pool_block_notify`;
CREATE TABLE `pool_block_notify` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pool_name` varchar(20) NOT NULL,
  `job_recv_time` timestamp(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
  `block_height` int(11) NOT NULL,
  `block_prev_hash` char(64) NOT NULL,
  `block_time` char(64) NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`id`),
  UNIQUE KEY `pool_name_block_height_block_prev_hash` (`pool_name`,`block_height`,`block_prev_hash`),
  KEY `block_height` (`block_height`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


-- 2016-08-19 11:51:35
