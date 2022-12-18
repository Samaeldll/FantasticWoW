-- DB update 2021_08_06_11 -> 2021_08_06_12
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2021_08_06_11';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2021_08_06_11 2021_08_06_12 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1627842189482644400'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1627842189482644400');

-- Walk to waypoint
UPDATE `smart_scripts` SET `action_param1` = 0  WHERE (`entryorguid` = 9598) AND (`source_type` = 0) AND (`id` IN (0));
-- Make Arei (9598) fight on aggro
UPDATE `smart_scripts` SET `action_type` = 20, `target_type` = 2, `comment` = 'Arei - On Aggro - Start Attacking' WHERE (`entryorguid` = 9598) AND (`source_type` = 0) AND (`id` IN (2));
-- Make Arei (9598) from darnassus and help us fight
UPDATE `creature_template` SET `faction` = 79 WHERE (`entry` = 9598);


--
-- END UPDATING QUERIES
--
UPDATE version_db_world SET date = '2021_08_06_12' WHERE sql_rev = '1627842189482644400';
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
