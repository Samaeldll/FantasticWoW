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

#ifndef _CREATURE_RESPAWN_H_
#define _CREATURE_RESPAWN_H_

#include "Define.h"
#include "Duration.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class ChatHandler;
class Creature;
class Player;

struct CreatureRespawnInfoItems
{
    uint32 ItemID{ 0 };
    std::array<uint32, 4/*default + vip levels*/> ItemCounts{};
    Seconds DecreaseSeconds{ 0s };
};

struct CreatureRespawnInfo
{
    uint32 SpawnID{};
    std::vector<CreatureRespawnInfoItems> Items;
    bool IsKillAnnounceEnable{ true };
    bool IsRespawnAnnounceEnable{ true };
    uint32 HelpNpcTextID{};
};

class CreatureRespawnMgr
{
public:
    static CreatureRespawnMgr* instance();

    void Initialize();
    void LoadConfig(bool reload);

    inline bool IsEnable() { return _isEnable; }

    void LoadDataFromDB();

    // Hooks
    void OnGossipHello(Player* player, Creature* creature);
    void OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action);
    void OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, std::string_view code);
    void OnCreatureRespawn(Creature* creature);
    void OnCreatureKill(Player* player, Creature* creature);

    // CC
    void SendCreatureRespawnInfo(ChatHandler* handler, std::size_t page = 1);

private:
    void LoadCreatureListFromDB();
    void LoadCreatureItemsFromDB();

    Creature* FindCreature(uint32 spawnGuid);
    CreatureRespawnInfo* GetInfo(uint32 spawnGuid);
    static std::string GetCreatureName(uint32 spawnID, uint8 localeIndex);
    static void DecreaseRespawnTimeForCreature(Creature* creature, Seconds seconds);

    bool IsWarningPlayer(Player* player);

    bool _isEnable{ false };
    uint32 _vipDiscountLevel1{ 5 };
    uint32 _vipDiscountLevel2{ 20 };
    uint32 _vipDiscountLevel3{ 50 };

    std::unordered_map<uint32, CreatureRespawnInfo> _creatureList;
    std::unordered_map<uint32, Creature*> _creatureCache;
    std::vector<Player*> _warnings;

    CreatureRespawnMgr() = default;
    ~CreatureRespawnMgr() = default;

    CreatureRespawnMgr(CreatureRespawnMgr const&) = delete;
    CreatureRespawnMgr(CreatureRespawnMgr&&) = delete;
    CreatureRespawnMgr& operator=(CreatureRespawnMgr const&) = delete;
    CreatureRespawnMgr& operator=(CreatureRespawnMgr&&) = delete;
};

#define sCreatureRespawnMgr CreatureRespawnMgr::instance()

#endif // _CREATURE_RESPAWN_H_
