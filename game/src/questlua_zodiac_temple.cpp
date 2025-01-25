#include "stdafx.h"
#ifdef ENABLE_12ZI
#include "questlua.h"
#include "questmanager.h"
#include "zodiac_temple.h"
#include "war_map.h"
#include "char.h"

#include "../../common/CommonDefines.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	ALUA(zodiac_temple_starttemple)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		CZodiacManager::Instance().StartTemple(ch, (uint8_t)lua_tonumber(L, 1));
		return 0;
	}

	ALUA(zodiac_temple_new_floor)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			return 0;
		}

		quest::CQuestManager& q = quest::CQuestManager::Instance();
		LPZODIAC pZodiac = q.GetCurrentZodiac();
		if (!pZodiac)
			return 0;

		uint8_t Floor = (uint8_t)lua_tonumber(L, 1);
		pZodiac->NewFloor(Floor);
		return 0;
	}

	ALUA(zodiac_temple_setflag)
	{
		if (!lua_isstring(L,1) || !lua_isnumber(L,2))
		{
			sys_err("wrong set flag");
		}
		else
		{
			CQuestManager& q = CQuestManager::Instance();
			LPZODIAC pZodiac = q.GetCurrentZodiac();

			if (pZodiac)
			{
				const char* sz = lua_tostring(L,1);
				int value = int(lua_tonumber(L, 2));
				pZodiac->SetFlag(sz, value);
			}
			else
			{
				sys_err("no zodiac !!!");
			}
		}
		return 0;
	}

	ALUA(zodiac_temple_getflag)
	{
		if (!lua_isstring(L,1))
		{
			sys_err("wrong get flag");
		}

		CQuestManager& q = CQuestManager::Instance();
		LPZODIAC pZodiac = q.GetCurrentZodiac();

		if (pZodiac)
		{
			const char* sz = lua_tostring(L,1);
			lua_pushnumber(L, pZodiac->GetFlag(sz));
		}
		else
		{
			sys_err("no zodiac !!!");
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(zodiac_temple_spawn_portals)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		CZodiacManager::Instance().SpawnPortals();
		return 0;
	}

	void RegisterZodiacTempleFunctionTable()
	{
		luaL_reg zodiac_temple_functions[] =
		{
			{"starttemple", zodiac_temple_starttemple},
			{"new_floor", zodiac_temple_new_floor},
			{"setflag", zodiac_temple_setflag},
			{"getflag", zodiac_temple_getflag},
			{"spawn_portals", zodiac_temple_spawn_portals},

			{nullptr, nullptr}
		};

		CQuestManager::Instance().AddLuaFunctionTable("zodiac_temple", zodiac_temple_functions);
	}
}
#endif
