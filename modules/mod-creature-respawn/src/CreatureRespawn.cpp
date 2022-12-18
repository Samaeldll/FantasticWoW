/*
 * This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CreatureRespawn.h"
#include "Chat.h"
#include "GameLocale.h"
#include "GameTime.h"
#include "Log.h"
#include "MapMgr.h"
#include "ModuleLocale.h"
#include "ModulesConfig.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "StopWatch.h"
#include "StringConvert.h"
#include "Vip.h"
#include "WorldSession.h"
#include <range/v3/action/sort.hpp>

constexpr auto CHAT_ITEMS_COUNT_MAX = 5;
constexpr auto GOSSIP_SELECT_TYPE_SHOW_BOSS = static_cast<uint32>(GOSSIP_SENDER_MAIN);
constexpr auto GOSSIP_SELECT_TYPE_SHOW_HELP = 2;

CreatureRespawnMgr* CreatureRespawnMgr::instance()
{
    static CreatureRespawnMgr instance;
    return &instance;
}

void CreatureRespawnMgr::LoadConfig(bool /*reload*/)
{
    _isEnable = MOD_CONF_GET_BOOL("CreatureRespawn.Enable");
    if (!_isEnable)
        return;

    _vipDiscountLevel1 = MOD_CONF_GET_UINT("CreatureRespawn.VipDiscount.Level1");
    _vipDiscountLevel2 = MOD_CONF_GET_UINT("CreatureRespawn.VipDiscount.Level2");
    _vipDiscountLevel3 = MOD_CONF_GET_UINT("CreatureRespawn.VipDiscount.Level3");
}

void CreatureRespawnMgr::Initialize()
{
    if (!_isEnable)
        return;

    LoadDataFromDB();
}

void CreatureRespawnMgr::LoadDataFromDB()
{
    if (!_isEnable)
        return;

    LoadCreatureListFromDB();
    LoadCreatureItemsFromDB();
}

void CreatureRespawnMgr::LoadCreatureListFromDB()
{
    if (!_isEnable)
        return;

    LOG_INFO("module", "Loading creatures for respawn...");

    StopWatch sw;
    _creatureList.clear();

    auto result = WorldDatabase.Query("SELECT `SpawnID`, `Enable`, `KillAnnounceEnable`, `RespawnAnnounceEnable`, `HelpNpcTextID` FROM `wh_creature_respawn` ORDER BY `Index` DESC");
    if (!result)
    {
        LOG_ERROR("module", "> Not found data from `wh_creature_respawn`. Disable module");
        LOG_INFO("module", "");
        _isEnable = false;
        return;
    }

    do
    {
        auto const& [spawnID, isEnable, killAnnounceEnable, respawnAnnounceEnable, npcTextID] = result->FetchTuple<uint32, bool, bool, bool, uint32>();
        if (!isEnable)
            continue;

        if (!FindCreature(spawnID))
            continue;

        _creatureList.emplace(spawnID, CreatureRespawnInfo{ spawnID, {}, killAnnounceEnable, respawnAnnounceEnable, npcTextID });
    } while (result->NextRow());

    if (_creatureList.empty())
    {
        LOG_ERROR("module", "> Not found enabled data from `wh_creature_respawn`. Disable module");
        _isEnable = false;
        return;
    }

    LOG_INFO("module", ">> Loaded {} creatures for respawn in {}", _creatureList.size(), sw);
    LOG_INFO("module", "");
}

void CreatureRespawnMgr::LoadCreatureItemsFromDB()
{
    if (!_isEnable)
        return;

    LOG_INFO("module", "Loading creature respawn items...");

    StopWatch sw;

    for (auto& [spawnId, info] : _creatureList)
        info.Items.clear();

    auto result = WorldDatabase.Query("SELECT `SpawnID`, `ItemID`, `ItemCount`, `DecreaseSeconds` FROM `wh_creature_respawn_items`");
    if (!result)
    {
        LOG_ERROR("module", "> Not found data from `wh_creature_respawn_items`");
        LOG_INFO("module", "");
        return;
    }

    auto GetDiscount = [](uint32 value, uint32 discount)
    {
        uint32 total = value - value * discount / 100;

        if (total <= 0)
            total = 1;

        return total;
    };

    std::size_t count{ 0 };

    do
    {
        auto const& [spawnID, itemID, itemCount, seconds] = result->FetchTuple<uint32, uint32, uint32, Seconds>();

        if (!sObjectMgr->GetItemTemplate(itemID))
        {
            LOG_ERROR("module", "> CreatureRespawnMgr: Not found item with id: {}", itemID);
            continue;
        }

        auto info = GetInfo(spawnID);
        if (!info)
        {
            LOG_ERROR("module", "> CreatureRespawnMgr: Not found info for creature with spawn id: {}", spawnID);
            continue;
        }

        std::array<uint32, 4> itemCounts
        {
            itemCount,
            GetDiscount(itemCount, _vipDiscountLevel1),
            GetDiscount(itemCount, _vipDiscountLevel2),
            GetDiscount(itemCount, _vipDiscountLevel3)
        };

        info->Items.emplace_back(CreatureRespawnInfoItems{ itemID, itemCounts, seconds });
        count++;
    } while (result->NextRow());

    LOG_INFO("module", ">> Loaded {} creature respawn items in {}", count, sw);
    LOG_INFO("module", "");

    for (auto& [spawnId, info] : _creatureList)
        ranges::sort(info.Items, {}, &CreatureRespawnInfoItems::DecreaseSeconds);
}

Creature* CreatureRespawnMgr::FindCreature(uint32 spawnGuid)
{
    // #1 - check cache
    auto const& itr = _creatureCache.find(spawnGuid);
    if (itr != _creatureCache.end())
    {
        auto creature = itr->second;
        if (!creature)
        {
            _creatureCache.erase(spawnGuid);
            LOG_ERROR("module", "> CR: Not found cache creature with spawn guid {}. Erase", spawnGuid);
            return FindCreature(spawnGuid);
        }

        return creature;
    }

    // #2 - not found in cache, try to find default
    auto const& creatureData = sObjectMgr->GetCreatureData(spawnGuid);
    if (!creatureData)
    {
        LOG_ERROR("module", "> CreatureRespawnMgr: Not found creature data with spawn id: {}", spawnGuid);
        return nullptr;
    }

    Map* map = sMapMgr->FindBaseMap(creatureData->mapid);
    if (!map)
    {
        LOG_ERROR("module", "> CreatureRespawnMgr: Not found map in creature data with spawn id: {}. Map id: {}", spawnGuid, creatureData->mapid);
        return nullptr;
    }

    map->LoadGrid(creatureData->posX, creatureData->posY);

    auto const creBounds = map->GetCreatureBySpawnIdStore().equal_range(spawnGuid);
    if (creBounds.first == creBounds.second)
    {
        LOG_ERROR("module", "> CreatureRespawnMgr: Not found creatures in map with spawn id: {}. Map id: {}", spawnGuid, creatureData->mapid);
        return nullptr;
    }

    _creatureCache.emplace(spawnGuid, creBounds.first->second);
    return creBounds.first->second;
}

CreatureRespawnInfo* CreatureRespawnMgr::GetInfo(uint32 spawnGuid)
{
    return Warhead::Containers::MapGetValuePtr(_creatureList, spawnGuid);
}

// Hooks
void CreatureRespawnMgr::OnGossipHello(Player* player, Creature* creature)
{
    if (!_isEnable)
        return;

    ClearGossipMenuFor(player);

    int8 localeIndex = player->GetSession()->GetSessionDbLocaleIndex();
    auto currentTime = GameTime::GetGameTime().count();

    std::size_t count{ 0 };

    for (auto const& [spawnId, info] : _creatureList)
    {
        // Support only 32 creature
        if (count >= GOSSIP_MAX_MENU_ITEMS)
            break;

        auto const& creatureData = sObjectMgr->GetCreatureData(spawnId);
        auto creatureName = sGameLocale->GetCreatureNamelocale(creatureData->id1, localeIndex);
        std::string curRespawnDelayStr{ "<unknown>" };
        uint32 action{ 0 };

        auto const& creature = FindCreature(spawnId);
        if (creature)
        {
            if (creature->IsAlive())
                curRespawnDelayStr = "Alive";
            else
            {
                int64 curRespawnDelay = creature->GetRespawnTimeEx() - currentTime;
                if (curRespawnDelay < 0)
                    curRespawnDelay = 0;

                curRespawnDelayStr = Warhead::Time::ToTimeString(Seconds(curRespawnDelay));
                action = spawnId;
            }
        }

        AddGossipItemFor(player, GOSSIP_ICON_DOT, Warhead::StringFormat("{} | {}", creatureName, curRespawnDelayStr), GOSSIP_SELECT_TYPE_SHOW_BOSS, action);
        count++;
    }

    SendGossipMenuFor(player, 1, creature->GetGUID());
}

void CreatureRespawnMgr::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (!_isEnable)
        return;

    ClearGossipMenuFor(player);

    auto spawnID = action;
    if (!spawnID)
    {
        OnGossipHello(player, creature);
        return;
    }

    auto const& info = GetInfo(spawnID);

    switch (sender)
    {
        case GOSSIP_SELECT_TYPE_SHOW_BOSS:
        {
            int8 localeIndex = player->GetSession()->GetSessionDbLocaleIndex();
            uint32 itemCountIndex{ 0 };

            if (sVip->IsVip(player))
            {
                auto vipLevel = sVip->GetLevel(player);
                itemCountIndex += vipLevel;
            }

            AddGossipItemFor(player, GOSSIP_ICON_DOT, GetCreatureName(spawnID, localeIndex), GOSSIP_SELECT_TYPE_SHOW_BOSS, 0);

            // Add help for this boss if exist
            if (info && info->HelpNpcTextID)
                AddGossipItemFor(player, GOSSIP_ICON_DOT, "Инструкция", GOSSIP_SELECT_TYPE_SHOW_HELP, spawnID);

            AddGossipItemFor(player, GOSSIP_ICON_DOT, "> Уменьшить время воскрешения:", GOSSIP_SELECT_TYPE_SHOW_BOSS, 0);

            auto info = GetInfo(spawnID);
            if (!info)
                AddGossipItemFor(player, GOSSIP_ICON_DOT, Warhead::StringFormat("> Not found creature with spawnID: {}", spawnID), spawnID, 0);
            else if (info->Items.empty())
                AddGossipItemFor(player, GOSSIP_ICON_DOT, Warhead::StringFormat("> Not items for spawnID: {}", spawnID), spawnID, 0);
            else if (!info->Items.empty())
            {
                for (auto const& [itemID, itemCounts, seconds] : info->Items)
                {
                    std::string itemLink = sGameLocale->GetItemLink(itemID, localeIndex);
                    std::string timeString = Warhead::Time::ToTimeString(seconds);
                    AddGossipItemFor(player, GOSSIP_ICON_DOT, Warhead::StringFormat("{}. {} - {}", timeString, itemCounts.at(itemCountIndex), itemLink), spawnID, itemID, "", 0, true);
                }
            }

            // Time to back
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "<< Назад", GOSSIP_SELECT_TYPE_SHOW_BOSS, 0);
            SendGossipMenuFor(player, 1, creature->GetGUID());
        }
        break;
        case GOSSIP_SELECT_TYPE_SHOW_HELP:
        {
            if (!info || !info->HelpNpcTextID)
            {
                OnGossipHello(player, creature);
                return;
            }

            AddGossipItemFor(player, GOSSIP_ICON_DOT, "<< Назад", GOSSIP_SELECT_TYPE_SHOW_BOSS, spawnID);
            SendGossipMenuFor(player, info->HelpNpcTextID, creature->GetGUID());
        }
        break;
        default:
        {
            OnGossipHello(player, creature);
            return;
        }
    }
}

void CreatureRespawnMgr::OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, std::string_view code)
{
    if (!_isEnable)
        return;

    CloseGossipMenuFor(player);

    auto count = Warhead::StringTo<int32>(code);
    if (!count || !*count)
    {
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_INCORRECT_CODE", code);
        return;
    }

    auto spawnID = sender;
    auto enterCount = *count;

    auto toRespawnCreature = FindCreature(spawnID);
    if (!toRespawnCreature)
    {
        LOG_ERROR("module", "> Not found toRespawnCreature with spawn id: {}", spawnID);
        return;
    }

    auto info = GetInfo(spawnID);
    if (!info)
    {
        LOG_ERROR("module", "> Not found creature respawn info. SpawnID: {}", spawnID);
        return;
    }

    if (info->Items.empty())
    {
        LOG_ERROR("module", "> Not found items for creature respawn info. SpawnID: {}", spawnID);
        return;
    }

    auto itemID = action;
    int8 localeIndex = player->GetSession()->GetSessionDbLocaleIndex();
    uint32 itemCount{ 0 };
    Seconds seconds{ 0 };
    uint32 itemCountIndex{ 0 };

    if (sVip->IsVip(player))
    {
        auto vipLevel = sVip->GetLevel(player);
        itemCountIndex += vipLevel;
    }

    for (auto const& [_itemID, itemCounts, _seconds] : info->Items)
    {
        if (itemID == _itemID)
        {
            itemCount = itemCounts.at(itemCountIndex);
            seconds = _seconds;
            break;
        }
    }

    if (!itemCount)
    {
        LOG_ERROR("module", "> Not found item count. Item id: {}", itemID);
        return;
    }

    if (seconds == 0s)
    {
        LOG_ERROR("module", "> Incorrect seconds (= 0). Item id: {}. Item count: {}", itemID, itemCount);
        return;
    }

    auto totalCount{ itemCount * enterCount };
    std::string itemLink{ sGameLocale->GetItemLink(itemID, localeIndex) };
    auto creatureName{ GetCreatureName(spawnID, localeIndex) };

    if (!player->HasItemCount(itemID, totalCount))
    {
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_NOT_ENOUGH", itemLink, totalCount - player->GetItemCount(itemID));
        return;
    }

    auto currentTime = GameTime::GetGameTime();
    auto curRespawnDelay = Seconds(toRespawnCreature->GetRespawnTimeEx()) - currentTime;

    if (toRespawnCreature->IsAlive() || curRespawnDelay == 0s)
    {
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_NPC_IS_ALIVE", creatureName);
        return;
    }

    auto totalDecreaseSeconds{ seconds * enterCount };

    if (totalDecreaseSeconds > curRespawnDelay && enterCount > 1)
    {
        auto decreaseSecondsPrew{ seconds * (enterCount - 1) };
        if (decreaseSecondsPrew > curRespawnDelay)
        {
            sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_MORE_TIME", Warhead::Time::ToTimeString(totalDecreaseSeconds), Warhead::Time::ToTimeString(curRespawnDelay));
            return;
        }
    }

    bool isWarningPlayer{ IsWarningPlayer(player) };

    if (!isWarningPlayer && enterCount == 1 && totalDecreaseSeconds > curRespawnDelay * 2)
    {
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_PLAYER_WARNING", Warhead::Time::ToTimeString(curRespawnDelay), Warhead::Time::ToTimeString(totalDecreaseSeconds));
        _warnings.emplace_back(player);
        return;
    }

    if (isWarningPlayer)
        std::erase(_warnings, player);

    player->DestroyItemCount(itemID, totalCount, true);
    DecreaseRespawnTimeForCreature(toRespawnCreature, totalDecreaseSeconds);
    sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_YOU_DECREASED_TIME", creatureName, Warhead::Time::ToTimeString(totalDecreaseSeconds));
}

void CreatureRespawnMgr::OnCreatureRespawn(Creature* creature)
{
    if (!_isEnable)
        return;

    auto spawnID = creature->GetSpawnId();
    if (!spawnID)
        return;

    auto info = GetInfo(spawnID);
    if (!info || !info->IsRespawnAnnounceEnable)
        return;

    sModuleLocale->SendGlobalMessageFmt(false, [creature](uint8 localeIndex)
    {
        auto creatureName = sGameLocale->GetCreatureNamelocale(creature->GetEntry(), localeIndex);
        return sModuleLocale->GetLocaleMessage("CR_LOCALE_NPC_RESPAWN", localeIndex, creatureName);
    });
}

void CreatureRespawnMgr::OnCreatureKill(Player* player, Creature* creature)
{
    if (!_isEnable)
        return;

    auto spawnID = creature->GetSpawnId();
    if (!spawnID)
        return;

    auto info = GetInfo(spawnID);
    if (!info || !info->IsKillAnnounceEnable)
        return;

    sModuleLocale->SendGlobalMessageFmt(false, [player, creature](uint8 localeIndex)
    {
        auto creatureName = sGameLocale->GetCreatureNamelocale(creature->GetEntry(), localeIndex);
        return sModuleLocale->GetLocaleMessage("CR_LOCALE_NPC_KILL", localeIndex, player->GetName(), creatureName);
    });
}

void CreatureRespawnMgr::SendCreatureRespawnInfo(ChatHandler* handler, std::size_t page /*= 1*/)
{
    auto player = handler->GetPlayer();
    if (!player)
        return;

    int8 localeIndex = handler->GetSessionDbLocaleIndex();
    auto currentTime = GameTime::GetGameTime().count();
    std::size_t count{ 1 };
    std::size_t minCount{ ((page - 1) * CHAT_ITEMS_COUNT_MAX) + 1 };
    std::size_t maxCount{ page * CHAT_ITEMS_COUNT_MAX };
    std::size_t pageCount{ static_cast<std::size_t>(std::ceil(static_cast<double>(_creatureList.size()) / static_cast<double>(CHAT_ITEMS_COUNT_MAX)))};

    if (page > pageCount)
    {
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_CHAT_INFO_INCORRECT_PAGE", page, pageCount);
        return;
    }

    handler->PSendSysMessage("|cFFFF0000#|r --");

    for (auto const& [spawnId, info] : _creatureList)
    {
        if (count < minCount)
        {
            count++;
            continue;
        }

        if (count > maxCount)
            break;

        auto const& creatureData = sObjectMgr->GetCreatureData(spawnId);
        auto creatureName = sGameLocale->GetCreatureNamelocale(creatureData->id1, localeIndex);
        std::string curRespawnDelayStr{ "<unknown>" };

        auto const& creature = FindCreature(spawnId);
        if (creature)
        {
            if (creature->IsAlive())
                curRespawnDelayStr = "Alive";
            else
            {
                int64 curRespawnDelay = creature->GetRespawnTimeEx() - currentTime;
                if (curRespawnDelay < 0)
                    curRespawnDelay = 0;

                curRespawnDelayStr = Warhead::Time::ToTimeString(Seconds(curRespawnDelay));
            }
        }

        // |cFFFF0000#|r {}. |cFFFF000{}|r - |cff7bbef7{}
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_CHAT_INFO", count++, creatureName, curRespawnDelayStr);
    }

    handler->PSendSysMessage("|cFFFF0000#|r --");

    sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_CHAT_INFO_PAGE", page, pageCount);

    if (page < pageCount)
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_CHAT_INFO_WRITE_FOR_DISPLAY_PAGE", page + 1, page + 1);
    else if (pageCount <= page)
        sModuleLocale->SendPlayerMessage(player, "CR_LOCALE_CHAT_INFO_WRITE_FOR_DISPLAY_PAGE", page - 1, page - 1);
}

std::string CreatureRespawnMgr::GetCreatureName(uint32 spawnID, uint8 localeIndex)
{
    auto const& creatureData = sObjectMgr->GetCreatureData(spawnID);
    return sGameLocale->GetCreatureNamelocale(creatureData->id1, localeIndex);
}

void CreatureRespawnMgr::DecreaseRespawnTimeForCreature(Creature* creature, Seconds seconds)
{
    if (!creature || seconds == 0s)
        return;

    auto currentTime = GameTime::GetGameTime();
    auto curRespawnDelay = Seconds(creature->GetRespawnTimeEx()) - currentTime;

    if (curRespawnDelay <= 10s || curRespawnDelay <= seconds)
    {
        creature->Respawn();
        return;
    }

    auto newTime = curRespawnDelay - seconds;
    creature->SetRespawnTime(newTime.count());
}

bool CreatureRespawnMgr::IsWarningPlayer(Player* player)
{
    if (_warnings.empty())
        return false;

    for (auto const& _player : _warnings)
        if (_player == player)
            return true;

    return false;
}
