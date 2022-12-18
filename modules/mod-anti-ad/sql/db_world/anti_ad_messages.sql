-- ----------------------------
-- Table structure for anti_ad_patterns
-- ----------------------------

DROP TABLE IF EXISTS `anti_ad_patterns`;
CREATE TABLE `anti_ad_patterns` (
    `Pattern` varchar(255) CHARACTER
    SET utf8 COLLATE utf8_general_ci NOT NULL,
    PRIMARY KEY (`Pattern`)
    USING BTREE) ENGINE = InnoDB CHARACTER
SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Compact;

-- ----------------------------
-- Records of anti_ad_patterns
-- ----------------------------
INSERT INTO `anti_ad_patterns` VALUES ('[-a-zA-Z0-9@:%._ \\+~#=]{1,256}\\.[a-zA-Z0-9 ()]{1,6}\\b(?:[-a-zA-Z0-9()@:%_\\+.~#?&\\/=]*)');

-- ----------------------------
-- Records of string_module
-- ----------------------------

DELETE FROM `string_module`
WHERE `Entry` IN ('ANTIAD_LOCALE_SEND_GM_TEXT', 'ANTIAD_LOCALE_SEND_SELF', 'ANTIAD_LOCALE_SEND_GM_TEXT', 'ANTIAD_LOCALE_SEND_GM_TEXT', 'ANTIAD_LOCALE_SEND_SELF');

INSERT INTO `string_module`
        VALUES
        ('ANTIAD_LOCALE_SEND_GM_TEXT', 'enUS', '|cFFFF0000[AntiAD]:|r {} |cff6C8CD5wanted to say:|r {}'),
        ('ANTIAD_LOCALE_SEND_SELF', 'enUS', '|cFFFF0000[AntiAD]:|cff6C8CD5 You chat muted on |r {} |cff6C8CD5minutes.'),
        ('ANTIAD_LOCALE_SEND_GM_TEXT', 'ruRU', '|cFFFF0000[Антиреклама]:|r {} |cff6C8CD5хотел сказать:|r {}'),
        ('ANTIAD_LOCALE_SEND_SELF', 'ruRU', '|cFFFF0000[Антиреклама]:|cff6C8CD5 Ваш чат заблокирован на|r {} |cff6C8CD5минут.');
