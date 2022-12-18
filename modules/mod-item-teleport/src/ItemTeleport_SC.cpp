/*

 */

#include "Player.h"
#include "WorldSession.h"
#include "ScriptObject.h"
#include "ScriptedGossip.h"
#include "ChatTextBuilder.h"
#include "ModuleLocale.h"

constexpr auto EMOTE_COMBAT = "|cffff0000Вы в бою! Чтобы использовать данный предмет, выйдите из боя!|r";
constexpr auto EMOTE_TELEPORT = "|cffF08080Телепортация выполнено успешно, приятной игры.";
constexpr auto EMOTE_QUESTIONSFIRST = "|cffFFC0CBУсловия задания были выполнены, продолжайте ваш путь.";

constexpr auto ETHEREAL_CREDIT = 38186;

class item_teleport : public ItemScript
{
public:
    item_teleport() : ItemScript("item_teleport") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override // Any hook here
    {
        CreateMenu(player, item);
        return false;
    }

    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        if (!player->getAttackers().empty())
        {
            Warhead::Text::SendAreaTriggerMessage(player->GetSession(), EMOTE_COMBAT);
            player->PlayerTalkClass->SendCloseGossip();
            return;
        }

        if (sender == GOSSIP_SENDER_MAIN)
        {
            player->PlayerTalkClass->ClearMenus();
            switch (action)
            {

            case 999: //Хук на создание меню
                CreateMenu(player, item);
                break;
            case 1: //Шатер Деревни
                Warhead::Text::SendAreaTriggerMessage(player->GetSession(), EMOTE_TELEPORT);
                if (player->GetQuestStatus(50001) == QUEST_STATUS_INCOMPLETE)
                {
                    Warhead::Text::SendAreaTriggerMessage(player->GetSession(), EMOTE_QUESTIONSFIRST);
                    player->KilledMonsterCredit(80016);
                }

                CloseGossipMenuFor(player);
                player->TeleportTo(37, 1064.7357f, 6.245868f, 314.92587f, 3.6755364f);
                break;

            case 2: //Башня Драконов
                Warhead::Text::SendAreaTriggerMessage(player->GetSession(), EMOTE_TELEPORT);
                ClearGossipMenuFor(player);
                player->TeleportTo(37, 881.2614f, 15.757152f, 348.14883f, 5.2447586f);
                player->PlayerTalkClass->SendCloseGossip();
                break;
            }
        }
    }

private:
    void CreateMenu(Player* player, Item* item)
    {
        if (player ->IsInCombat() || player ->IsInFlight() || player ->GetMap()->IsBattlegroundOrArena() || player ->HasStealthAura() || player ->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player ->isDead() || player ->GetAreaId() == 616)
        {
            player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, item, nullptr);
            CloseGossipMenuFor(player);
        }
        else
        {
            ClearGossipMenuFor(player);

            AddGossipItemFor(player, 10, "Шатер Деревни", GOSSIP_SENDER_MAIN, 1);

            if (player->GetQuestStatus(50003) == QUEST_STATUS_REWARDED)
                AddGossipItemFor(player, 10, "Башня Драконов", GOSSIP_SENDER_MAIN, 2);

            SendGossipMenuFor(player, 1, item->GetGUID());
        }
    }
};

class npc_information : public CreatureScript
{
public:
    npc_information() : CreatureScript("npc_information") { }

    bool OnGossipHello(Player* player, Creature* creature) override // Any hook here
    {
        CreateMenu(player, creature);
        return true;
    }

    void CreateMenu(Player* player, Creature* creature)
    {
        if (player->IsInCombat() || player->IsInFlight() || player->GetMap()->IsBattlegroundOrArena() || player->HasStealthAura() || player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH) || player->isDead() || player->GetAreaId() == 616)
        {
            Warhead::Text::SendAreaTriggerMessage(player->GetSession(), "Сейчас вы не можете выполнить данное действие");
            CloseGossipMenuFor(player);
        }        
        else
        {    
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, 10, "Информация о Проекте",             GOSSIP_SENDER_MAIN,     1);
            AddGossipItemFor(player, 10, "Информация о Игровом Мире",         GOSSIP_SENDER_MAIN,        2);
            AddGossipItemFor(player, 10, "Как написать администрации?",      GOSSIP_SENDER_MAIN,      3);
            AddGossipItemFor(player, 10, "Где скачать Лаунчер",             GOSSIP_SENDER_MAIN,      4);

            if (player->IsActiveQuest(50000))
                AddGossipItemFor(player, 10, "Информация о Деревне",         GOSSIP_SENDER_MAIN,     20);

            SendGossipMenuFor(player, 100500, creature->GetGUID());
        }
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (!player)
            return true;

        ClearGossipMenuFor(player);

        switch (sender)
        {
            case GOSSIP_SENDER_MAIN:
            {
                switch (action)
                {    
                    case 1:
                        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Вернутся назад",     GOSSIP_SENDER_MAIN, 999);
                        SendGossipMenuFor(player, 80006, creature->GetGUID());
                        if (player->GetQuestStatus(50000) == QUEST_STATUS_INCOMPLETE)
                        {
                            player->KilledMonsterCredit(80015);
                            Warhead::Text::SendAreaTriggerMessage(player->GetSession(), EMOTE_QUESTIONSFIRST);
                        }
                    break;
                    case 2:
                        CloseGossipMenuFor(player);
                        break;
                    case 999:
                    default:
                        CreateMenu(player, creature);
                        break;
                }
            }
            break;
            default:
                break;
        }

        return true;
    }
};

void AddSC_ItemTeleport()
{
    new item_teleport();
    new npc_information();
}
