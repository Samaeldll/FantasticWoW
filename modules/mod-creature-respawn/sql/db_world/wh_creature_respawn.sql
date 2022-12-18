--
-- Table structure for table `wh_creature_respawn`
--

DROP TABLE IF EXISTS `wh_creature_respawn`;
CREATE TABLE `wh_creature_respawn` (
    `Index` int(10) unsigned NOT NULL AUTO_INCREMENT,
    `SpawnID` int(11) NOT NULL,
    `Enable` tinyint(4) NOT NULL DEFAULT 1,
    `KillAnnounceEnable` tinyint(4) NOT NULL DEFAULT 1,
    `RespawnAnnounceEnable` tinyint(4) NOT NULL DEFAULT 1,
    `HelpNpcTextID` int(11) NOT NULL DEFAULT 1,
    `Comment` text DEFAULT NULL,
    PRIMARY KEY (`Index`),
    UNIQUE KEY `idx_spawnid` (`SpawnID`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `wh_creature_respawn`
--

INSERT INTO `wh_creature_respawn` VALUES
  (1,54984,1,1,1,1,'Магистр'),
  (2,55415,1,1,1,1,NULL),
  (3,55054,1,1,1,1,NULL),
  (4,54993,1,1,1,1,NULL),
  (5,54991,1,1,1,1,NULL),
  (6,55431,1,1,1,1,NULL);
