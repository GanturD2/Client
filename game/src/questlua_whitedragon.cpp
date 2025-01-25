#include "stdafx.h"

#ifdef ENABLE_WHITE_DRAGON
#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

#include "questlua.h"
#include "questmanager.h"
#include "WhiteDragon.h"
#include "char.h"
#include "party.h"

namespace quest
{
	ALUA(WhiteDragon_Access_Easy)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			WhiteDragon::CWhDr::instance().Access(pkChar, false);
			lua_pushboolean(L, true);
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 2;
	}

	ALUA(WhiteDragon_Access_Hard)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			WhiteDragon::CWhDr::instance().Access(pkChar, true);
			lua_pushboolean(L, true);
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 2;
	}

	ALUA(WhiteDragon_StartDungeon)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!pkChar)
			return 0;

		WhiteDragon::CWhDr::instance().StartDungeon(pkChar);

		return 1;
	}

	void RegisterWhiteLairFunctionTable()
	{
		luaL_reg functions[] =
		{
			{"Access", WhiteDragon_Access_Easy},
			{"Access2", WhiteDragon_Access_Hard},
			{"StartDungeon", WhiteDragon_StartDungeon},
			{NULL, NULL}
		};

		CQuestManager::instance().AddLuaFunctionTable("WhiteLair", functions);
	}
};
#endif
