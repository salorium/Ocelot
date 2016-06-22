SET FOREIGN_KEY_CHECKS = 0;

CREATE DATABASE ocelot CHARACTER SET utf8 COLLATE utf8_swedish_ci;

USE ocelot;

CREATE TABLE `xbt_files_users` (
  `uid` int(11) NOT NULL,
  `active` tinyint(1) NOT NULL DEFAULT '1',
  `announced` int(11) NOT NULL DEFAULT '0',
  `completed` tinyint(1) NOT NULL DEFAULT '0',
  `downloaded` bigint(20) NOT NULL DEFAULT '0',
  `remaining` bigint(20) NOT NULL DEFAULT '0',
  `uploaded` bigint(20) NOT NULL DEFAULT '0',
  `upspeed` int(10) unsigned NOT NULL DEFAULT '0',
  `downspeed` int(10) unsigned NOT NULL DEFAULT '0',
  `corrupt` bigint(20) NOT NULL DEFAULT '0',
  `timespent` int(10) unsigned NOT NULL DEFAULT '0',
  `useragent` varchar(51) NOT NULL DEFAULT '',
  `connectable` tinyint(4) NOT NULL DEFAULT '1',
  `peer_id` binary(20) NOT NULL DEFAULT '\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0',
  `fid` int(11) NOT NULL,
  `mtime` int(11) NOT NULL DEFAULT '0',
  `ip` varchar(39) NOT NULL DEFAULT '',
  PRIMARY KEY (`peer_id`,`fid`,`uid`),
  KEY `remaining_idx` (`remaining`),
  KEY `fid_idx` (`fid`),
  KEY `mtime_idx` (`mtime`),
  KEY `uid_active` (`uid`,`active`)
) ENGINE=InnoDB CHARSET utf8;

CREATE TABLE `torrents` (
  `ID` int(10) NOT NULL AUTO_INCREMENT,
  `info_hash` blob NOT NULL,
  `Leechers` int(6) NOT NULL DEFAULT '0',
  `Seeders` int(6) NOT NULL DEFAULT '0',
  `FreeTorrent` enum('0','1','2') NOT NULL DEFAULT '0',
  `Snatched` int(10) unsigned NOT NULL DEFAULT '0',
  `balance` bigint(20) NOT NULL DEFAULT '0',
  `last_action` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `InfoHash` (`info_hash`(40)),
  KEY `Seeders` (`Seeders`),
  KEY `Leechers` (`Leechers`),
  KEY `Snatched` (`Snatched`),
  KEY `last_action` (`last_action`),
  KEY `FreeTorrent` (`FreeTorrent`)
) ENGINE=InnoDB CHARSET utf8;

CREATE TABLE `torrents_files` (
  `TorrentID` int(10) NOT NULL,
  `File` mediumblob NOT NULL,
  PRIMARY KEY (`TorrentID`)
) ENGINE=MyISAM CHARSET utf8;

CREATE TABLE `users_main` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `IP` varchar(15) NOT NULL DEFAULT '0.0.0.0',
  `Uploaded` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Downloaded` bigint(20) unsigned NOT NULL DEFAULT '0',
  `Enabled` enum('0','1','2') NOT NULL DEFAULT '0',
  `Visible` enum('1','0') NOT NULL DEFAULT '1',
  `can_leech` tinyint(4) NOT NULL DEFAULT '1',
  `torrent_pass` char(32) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `IP` (`IP`),
  KEY `Uploaded` (`Uploaded`),
  KEY `Downloaded` (`Downloaded`),
  KEY `Enabled` (`Enabled`),
  KEY `torrent_pass` (`torrent_pass`),
) ENGINE=InnoDB CHARSET utf8;

CREATE TABLE `users_freeleeches` (
  `UserID` int(10) NOT NULL,
  `TorrentID` int(10) NOT NULL,
  `Expired` tinyint(1) NOT NULL DEFAULT '0',
  `Downloaded` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`UserID`,`TorrentID`),
  KEY `Expired_Time` (`Expired`,`Time`)
) ENGINE=InnoDB CHARSET utf8;

CREATE TABLE `xbt_client_whitelist` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `peer_id` varchar(20) DEFAULT NULL,
  `vstring` varchar(200) DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `peer_id` (`peer_id`)
) ENGINE=InnoDB CHARSET utf8;

CREATE TABLE `xbt_snatched` (
  `uid` int(11) NOT NULL DEFAULT '0',
  `tstamp` int(11) NOT NULL,
  `fid` int(11) NOT NULL,
  `IP` varchar(15) NOT NULL,
  KEY `fid` (`fid`),
  KEY `tstamp` (`tstamp`),
  KEY `uid_tstamp` (`uid`,`tstamp`)
) ENGINE=InnoDB CHARSET utf8;
