/*

 */

#include "Player.h"
#include "WorldSession.h"
#include "ScriptObject.h"
#include "Log.h"
#include "World.h"
#include "ModulesConfig.h"

class start_quest : public PlayerScript
{
public:
    start_quest() : PlayerScript("start_quest") { }

    void OnFirstLogin(Player* player) override
    {
        if (!MOD_CONF_GET_BOOL("QuestStart.Enable"))
            return;

        if (Quest const* quest = sObjectMgr->GetQuestTemplate(MOD_CONF_GET_INT("QuestStart.QuestID")))
        {
            if (player->CanAddQuest(quest, true))
                player->AddQuest(quest, nullptr);
        }
    }
};

void AddSC_StartQuest()
{
    new start_quest();
}
