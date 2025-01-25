
#include "stdafx.h"

#include "questmanager.h"
#include "DragonLair.h"
#include "char.h"
#include "guild.h"

namespace quest
{
	ALUA(dl_startRaid)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		long baseMapIndex = lua_tonumber(L, -1);

		CDragonLairManager::Instance().Start(ch->GetMapIndex(), baseMapIndex, ch->GetGuild()->GetID());

		return 0;
	}

	void RegisterDragonLairFunctionTable()
	{
		luaL_reg dl_functions[] =
		{
			{	"startRaid",	dl_startRaid	},

			{	nullptr,			nullptr			}
		};

		CQuestManager::Instance(). AddLuaFunctionTable("DragonLair", dl_functions);
	}
}

