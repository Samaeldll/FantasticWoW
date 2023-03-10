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

#include "Errors.h"
#include "ScriptMgr.h"
#include "ScriptMgrMacros.h"
#include "ScriptObject.h"

void ScriptMgr::OnCreatureAddWorld(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([creature](AllCreatureScript* script)
    {
        script->OnCreatureAddWorld(creature);
    });
}

void ScriptMgr::OnCreatureRemoveWorld(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([creature](AllCreatureScript* script)
    {
        script->OnCreatureRemoveWorld(creature);
    });
}

void ScriptMgr::OnCreatureSaveToDB(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([creature](AllCreatureScript* script)
    {
        script->OnCreatureSaveToDB(creature);
    });
}

void ScriptMgr::Creature_SelectLevel(const CreatureTemplate* cinfo, Creature* creature)
{
    ExecuteScript<AllCreatureScript>([cinfo, creature](AllCreatureScript* script)
    {
        script->Creature_SelectLevel(cinfo, creature);
    });
}

bool ScriptMgr::CanCreatureSendListInventory(Player* player, Creature* creature, uint32 vendorEntry)
{
    auto ret = IsValidBoolScript<AllCreatureScript>([player, creature, vendorEntry](AllCreatureScript* script)
    {
        return !script->CanCreatureSendListInventory(player, creature, vendorEntry);
    });

    return ReturnValidBool(ret);
}

void ScriptMgr::OnCreatureRespawn(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([creature](AllCreatureScript* script)
    {
        script->OnCreatureRespawn(creature);
    });
}
