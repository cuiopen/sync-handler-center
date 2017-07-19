CREATE DATABASE IF NOT EXISTS sync_db CHARSET=utf8;

USE `sync_db`;

--
-- Table structure for table `product_sync_table`
--

CREATE TABLE IF NOT EXISTS `product_sync_table`
(
  `sync_id` int(10) unsigned NOT NULL auto_increment,
  `game_id` int(10) unsigned NOT NULL,
  `status`  tinyint(1) unsigned NOT NULL DEFAULT 0 COMMENT '0 未处理， 1 已经处理',
  `message` binary(255) NOT NULL,
  `sync_time` timestamp DEFAULT CURRENT_TIMESTAMP,
  `finish_time` char(20) NOT NULL DEFAULT 0,
  PRIMARY KEY  (`sync_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Table structure for table `product_last_sync_table`
--

CREATE TABLE IF NOT EXISTS `product_last_sync_table`
(
    `game_id` int(10) unsigned NOT NULL COMMENT '游戏game_id',
    `last_sync_id` int(10) unsigned NOT NULL DEFAULT 0 COMMENT '上一次处理最大的sync_id',
    `reload_flag` tinyint(1) unsigned NOT NULL DEFAULT 0 COMMENT '此游戏是否需要重载的标志',
    PRIMARY KEY (`game_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
