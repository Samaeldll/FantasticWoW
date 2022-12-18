-- -------------------------
-- Records of string_module
-- -------------------------

DELETE FROM `string_module`
WHERE `Entry` IN ('CR_LOCALE_NPC_RESPAWN',
    'CR_LOCALE_NOT_ENOUGH',
    'CR_LOCALE_NPC_IS_ALIVE',
    'CR_LOCALE_NPC_KILL',
    'CR_LOCALE_MORE_TIME',
    'CR_LOCALE_YOU_DECREASED_TIME',
    'CR_LOCALE_PLAYER_WARNING',
    'CR_LOCALE_CHAT_INFO',
    'CR_LOCALE_CHAT_INFO_PAGE',
    'CR_LOCALE_CHAT_INFO_INCORRECT_PAGE',
    'CR_LOCALE_CHAT_INFO_WRITE_FOR_DISPLAY_PAGE',
    'CR_LOCALE_INCORRECT_CODE');

INSERT INTO `string_module`
    VALUES
    ('CR_LOCALE_NPC_RESPAWN', 'enUS', '|cFFFF0000[CR]:|r {} |cff6C8CD5respawned'),
    ('CR_LOCALE_NOT_ENOUGH', 'enUS', '|cFFFF0000[CR]:|r|cff6C8CD5 You dont have enough|r {} х{}'),
    ('CR_LOCALE_NPC_IS_ALIVE', 'enUS', '|cFFFF0000#|r {} |cff6C8CD5is alive'),
    ('CR_LOCALE_NPC_KILL', 'enUS', '|cFFFF0000[CR]: |cff7bbef7{}|r and his group killed |cffff0000{}|r'),
    ('CR_LOCALE_MORE_TIME', 'enUS', '|cFFFF0000[CR]: |cff7bbef7Entered time|r {} |cff7bbef7much more than it needs to be|r {}'),
    ('CR_LOCALE_YOU_DECREASED_TIME', 'enUS', '|cFFFF0000[CR]:|cff7bbef7 You decreased respawn time for |cFFFF0000{}|r |cff7bbef7by|r {}'),
    ('CR_LOCALE_PLAYER_WARNING', 'enUS', '|cFFFFFF0000[CR]:|cff7bbef7 The creature has |cFFFF0000{}|r |cff7bbef7left to resurrect, if you agree to use |cFFFF0000{}|r |cff7bbef7resurrection reduction, repeat'),
    ('CR_LOCALE_CHAT_INFO', 'enUS', '|cFFFF0000#|r {}. |cFFFF0000{}|r - |cff7bbef7{}'),
    ('CR_LOCALE_CHAT_INFO_PAGE', 'enUS', '|cFFFF0000#|r |cff7bbef7Page|r {} |cff7bbef7of|r {}'),
    ('CR_LOCALE_CHAT_INFO_INCORRECT_PAGE', 'enUS', '|cFFFF0000#|r |cff7bbef7Page|r {} |cff7bbef7incorrect. Max pages:|r {}'),
    ('CR_LOCALE_CHAT_INFO_WRITE_FOR_DISPLAY_PAGE', 'enUS', '|cFFFF0000#|r |cff7bbef7Write|r [.boss info {}] |cff7bbef7for display page|r {}'),
    ('CR_LOCALE_INCORRECT_CODE', 'enUS', '|cFFFF0000#|r |cff7bbef7You entered the code incorrectly|r {}'),

    ('CR_LOCALE_NPC_RESPAWN', 'ruRU', '|cFFFF0000[CR]:|r {} |cff6C8CD5возродился'),
    ('CR_LOCALE_NOT_ENOUGH', 'ruRU', '|cFFFF0000[CR]:|r|cff6C8CD5 Вам не хватает|r {} х{}'),
    ('CR_LOCALE_NPC_IS_ALIVE', 'ruRU', '|cFFFF0000[CR]:|r {} |cff6C8CD5жив'),
    ('CR_LOCALE_NPC_KILL', 'ruRU', '|cFFFF0000[CR]: |cff7bbef7{}|r и его группа убили |cffff0000{}|r'),
    ('CR_LOCALE_MORE_TIME', 'ruRU', '|cFFFF0000[CR]: |cff7bbef7Введёное время|r ({}) |cff7bbef7намного больше нужного|r {}'),
    ('CR_LOCALE_YOU_DECREASED_TIME', 'ruRU', '|cFFFF0000[CR]:|cff7bbef7 Вы уменьшили время возрождения для |cFFFF0000{}|r |cff7bbef7на|r {}'),
    ('CR_LOCALE_PLAYER_WARNING', 'ruRU', '|cFFFF0000[CR]:|cff7bbef7 У существа осталось |cFFFF0000{}|r |cff7bbef7до воскрешения, если вы согласны использовать |cFFFF0000{}|r |cff7bbef7уменьшение воскрешения, повторите'),
    ('CR_LOCALE_CHAT_INFO', 'ruRU', '|cFFFF0000#|r {}. |cFFFF0000{}|r - |cff7bbef7{}'),
    ('CR_LOCALE_CHAT_INFO_PAGE', 'ruRU', '|cFFFF0000#|r |cff7bbef7Страница|r {} |cff7bbef7из|r {}'),
    ('CR_LOCALE_CHAT_INFO_INCORRECT_PAGE', 'ruRU', '|cFFFF0000#|r |cff7bbef7Страницы|r {} |cff7bbef7не существует. Максимум страниц:|r {}'),
    ('CR_LOCALE_CHAT_INFO_WRITE_FOR_DISPLAY_PAGE', 'ruRU', '|cFFFF0000#|r |cff7bbef7Введите|r [.boss info {}] |cff7bbef7для отображения страницы|r {}'),
    ('CR_LOCALE_INCORRECT_CODE', 'ruRU', '|cFFFF0000#|r |cff7bbef7Вы ввели код некорректно|r {}');
