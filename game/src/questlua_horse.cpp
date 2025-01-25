#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "horsename_manager.h"
#include "char.h"
#include "affect.h"
#include "config.h"
#include "utils.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern int (*check_name) (const char * str);
#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
extern bool IS_MOUNTABLE_ZONE(int map_index);	//@custom005
#endif

namespace quest
{
	//
	// "horse" Lua functions
	//
	ALUA(horse_is_riding)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch->IsHorseRiding())
			lua_pushnumber(L, 1);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(horse_is_summon)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != ch)
		{
			lua_pushboolean(L, (ch->GetHorse() != nullptr) ? true : false);
		}
		else
		{
			lua_pushboolean(L, false);
		}

		return 1;
	}

	ALUA(horse_ride)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		ch->StartRiding();
		return 0;
	}

	ALUA(horse_unride)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		ch->StopRiding();
		return 0;
	}

	ALUA(horse_summon)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
		if (!IS_MOUNTABLE_ZONE(ch->GetMapIndex())) {	//@custom005
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("RIDING_IS_BLOCKED_HERE"));
			return 0;
		}
#endif

		// 소환하면 멀리서부터 달려오는지 여부
		bool bFromFar = lua_isboolean(L, 1) ? lua_toboolean(L, 1) : false;

		// 소환수의 vnum
#ifdef ENABLE_HORSE_APPEARANCE
		uint32_t horseVnum = ch->GetHorseAppearance() > 0 ? ch->GetHorseAppearance() : lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0;
#else
		uint32_t horseVnum = lua_isnumber(L, 2) ? lua_tonumber(L, 2) : 0;
#endif

		const char* name = lua_isstring(L, 3) ? lua_tostring(L, 3) : 0;
		ch->HorseSummon(true, bFromFar, horseVnum, name);
		return 0;
	}

	ALUA(horse_unsummon)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		ch->HorseSummon(false);
		return 0;
	}

	ALUA(horse_is_mine)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPCHARACTER horse = CQuestManager::Instance().GetCurrentNPCCharacterPtr();

		lua_pushboolean(L, horse && horse->GetRider() == ch);
		return 1;
	}

	ALUA(horse_in_summonable_area)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
		lua_pushboolean(L, IS_MOUNTABLE_ZONE(ch->GetMapIndex()));	//@custom005
#endif
		return 1;
	}

	ALUA(horse_set_level)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
			return 0;

		int newlevel = MINMAX(0, (int)lua_tonumber(L, 1), HORSE_MAX_LEVEL);
		ch->SetHorseLevel(newlevel);
		ch->ComputePoints();
		ch->SkillLevelPacket();
		return 0;
	}

	ALUA(horse_get_level)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetHorseLevel());
		return 1;
	}

	ALUA(horse_advance)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch->GetHorseLevel() >= HORSE_MAX_LEVEL)
			return 0;

		ch->SetHorseLevel(ch->GetHorseLevel() + 1);
		ch->ComputePoints();
		ch->SkillLevelPacket();
		return 0;
	}

	ALUA(horse_get_health)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseHealth());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(horse_get_health_pct)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		int pct = MINMAX(0, ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth(), 100);
		sys_log(1, "horse.get_health_pct %d", pct);

		if (ch->GetHorseLevel())
			lua_pushnumber(L, pct);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(horse_get_stamina)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseStamina());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(horse_get_stamina_pct)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		int pct = MINMAX(0, ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina(), 100);
		sys_log(1, "horse.get_stamina_pct %d", pct);

		if (ch->GetHorseLevel())
			lua_pushnumber(L, pct);
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(horse_get_grade)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch->GetHorseGrade());
		return 1;
	}

	ALUA(horse_is_dead)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->GetHorseHealth()<=0);
		return 1;
	}

	ALUA(horse_revive)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch->GetHorseLevel() > 0 && ch->GetHorseHealth() <= 0)
		{
			ch->ReviveHorse();
		}
		return 0;
	}

	ALUA(horse_feed)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		//uint32_t dwHorseFood = ch->GetHorseLevel() + ITEM_HORSE_FOOD_1 - 1;
		if (ch->GetHorseLevel() > 0 && ch->GetHorseHealth() > 0)
		{
			ch->FeedHorse();
		}
		return 0;
	}

	ALUA(horse_set_name)
	{
		// 리턴값
		// 0 : 소유한 말이 없다
		// 1 : 잘못된 이름이다
		// 2 : 이름 바꾸기 성공

		if ( lua_isstring(L, -1) != true ) return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ( ch->GetHorseLevel() > 0 )
		{
			const char* pHorseName = lua_tostring(L, -1);

			if ( pHorseName == nullptr || check_name(pHorseName) == 0 )
			{
				lua_pushnumber(L, 1);
			}
			else
			{
				int nHorseNameDuration = test_server == true ? 60*5 : 60*60*24*30;

				ch->SetQuestFlag("horse_name.valid_till", get_global_time() + nHorseNameDuration);
				ch->AddAffect(AFFECT_HORSE_NAME, 0, 0, 0, PASSES_PER_SEC(nHorseNameDuration), 0, true);

				CHorseNameManager::Instance().UpdateHorseName(ch->GetPlayerID(), lua_tostring(L, -1), true);

				ch->HorseSummon(false, true);
				ch->HorseSummon(true, true);

				lua_pushnumber(L, 2);
			}
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(horse_get_name)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ( ch != nullptr )
		{
			const char* pHorseName = CHorseNameManager::Instance().GetHorseName(ch->GetPlayerID());

			if ( pHorseName != nullptr )
			{
				lua_pushstring(L, pHorseName);
				return 1;
			}
		}

		lua_pushstring(L, "");

		return 1;
	}

#ifdef ENABLE_NEWSTUFF
	ALUA(horse_set_stat0)
	{
		int iHealth = MINMAX(0, lua_tonumber(L, 1), 50);
		int iStamina = MINMAX(0, lua_tonumber(L, 2), 200);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		ch->UpdateHorseHealth(iHealth - ch->GetHorseHealth());
		ch->UpdateHorseStamina(iStamina - ch->GetHorseStamina());

		return 0;
	}
#endif

#ifdef ENABLE_HORSE_APPEARANCE
	ALUA(horse_set_appearance)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong horse_appearance vnum");
			return 0;
		}
		ch->SetHorseAppearance((uint32_t)lua_tonumber(L, 1));
		return 0;
	}

	ALUA(horse_get_appearance)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		lua_pushnumber(L, ch->GetHorseAppearance());
		return 1;
	}
#endif

	void RegisterHorseFunctionTable()
	{
		luaL_reg horse_functions[] =
		{
			{ "is_mine",		horse_is_mine			},
			{ "is_riding",		horse_is_riding			},
			{ "is_summon",		horse_is_summon			},
			{ "in_summonable_area", horse_in_summonable_area},
			{ "ride",			horse_ride				},
			{ "unride",			horse_unride			},
			{ "summon",			horse_summon			},
			{ "unsummon",		horse_unsummon			},
			{ "advance",		horse_advance			},
			{ "get_level",		horse_get_level			},
			{ "set_level",		horse_set_level			},
			{ "get_health",		horse_get_health		},
			{ "get_health_pct",	horse_get_health_pct	},
			{ "get_stamina",	horse_get_stamina		},
			{ "get_stamina_pct",horse_get_stamina_pct	},
			{ "get_grade",      horse_get_grade         },
			{ "is_dead",		horse_is_dead			},
			{ "revive",			horse_revive			},
			{ "feed",			horse_feed				},
			{ "set_name",		horse_set_name			},
			{ "get_name",		horse_get_name			},
#ifdef ENABLE_NEWSTUFF
			// horse.set_stat0(health, stamina) -- /do_horse_set_stat
			{ "set_stat0",		horse_set_stat0			},	// [return nothing]
#endif
#ifdef ENABLE_HORSE_APPEARANCE
			{ "set_appearance",	horse_set_appearance	},
			{ "get_appearance",	horse_get_appearance	},
#endif
			{ nullptr,				nullptr					}
		};

		CQuestManager::Instance().AddLuaFunctionTable("horse", horse_functions);
	}
}




