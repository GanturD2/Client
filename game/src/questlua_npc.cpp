#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "questmanager.h"
#include "char.h"
#include "party.h"
#include "xmas_event.h"
#include "char_manager.h"
#include "shop_manager.h"
#include "guild.h"
#include "sectree_manager.h"
#ifdef ENABLE_HWID_BAN
#	include "desc.h"
#endif

namespace quest
{
	//
	// "npc" lua functions
	//
	ALUA(npc_open_shop)
	{
		int iShopVnum = 0;

		if (lua_gettop(L) == 1)
		{
			if (lua_isnumber(L, 1))
				iShopVnum = (int) lua_tonumber(L, 1);
		}

		if (CQuestManager::Instance().GetCurrentNPCCharacterPtr())
			CShopManager::Instance().StartShopping(CQuestManager::Instance().GetCurrentCharacterPtr(), CQuestManager::Instance().GetCurrentNPCCharacterPtr(), iShopVnum);

		return 0;
	}

	ALUA(npc_get_name)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if(npc)
			lua_pushstring(L, npc->GetName());
		else 
			lua_pushstring(L, "");

		return 1;
	}

	ALUA(npc_is_pc)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (npc && npc->IsPC())
			lua_pushboolean(L, 1);
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(npc_get_empire)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (npc)
			lua_pushnumber(L, npc->GetEmpire());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(npc_get_race)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentNPCRace());
		return 1;
	}

	ALUA(npc_get_guild)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		CGuild* pGuild = nullptr;
		if (npc)
			pGuild = npc->GetGuild();

		lua_pushnumber(L, pGuild ? pGuild->GetID() : 0);
		return 1;
	}

	ALUA(npc_get_remain_skill_book_count)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, MAX(0, npc->GetPoint(POINT_ATT_GRADE_BONUS)));
		return 1;
	}

	ALUA(npc_dec_remain_skill_book_count)
	{
		auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			return 0;
		}

		npc->SetPoint(POINT_ATT_GRADE_BONUS, MAX(0, npc->GetPoint(POINT_ATT_GRADE_BONUS)-1));
		return 0;
	}

	ALUA(npc_get_remain_hairdye_count)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, MAX(0, npc->GetPoint(POINT_DEF_GRADE_BONUS)));
		return 1;
	}

	ALUA(npc_dec_remain_hairdye_count)
	{
		auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC() || npc->GetRaceNum() != xmas::MOB_SANTA_VNUM)
		{
			return 0;
		}

		npc->SetPoint(POINT_DEF_GRADE_BONUS, MAX(0, npc->GetPoint(POINT_DEF_GRADE_BONUS)-1));
		return 0;
	}

	ALUA(npc_is_quest)
	{
		CQuestManager& q = CQuestManager::Instance();
		const auto npc = q.GetCurrentNPCCharacterPtr();

		if (npc)
		{
			const std::string & r_st = q.GetCurrentQuestName();

			if (q.GetQuestIndexByName(r_st) == npc->GetQuestBy())
			{
				lua_pushboolean(L, 1);
				return 1;
			}
		}

		lua_pushboolean(L, 0);
		return 1;
	}

	ALUA(npc_kill)
	{
		CQuestManager& q = CQuestManager::Instance();
		auto ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		auto npc = q.GetCurrentNPCCharacterPtr();

		ch->SetQuestNPCID(0);
		if (npc)
			npc->DeadNoReward(); // @fixme188 from Dead()

		return 0;
	}

	ALUA(npc_purge)
	{
		CQuestManager& q = CQuestManager::Instance();
		auto ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		auto npc = q.GetCurrentNPCCharacterPtr();

		ch->SetQuestNPCID(0);
		if (npc)
			M2_DESTROY_CHARACTER(npc);

		return 0;
	}

	ALUA(npc_is_near)
	{
		CQuestManager& q = CQuestManager::Instance();
		const auto ch = q.GetCurrentCharacterPtr();
		const auto npc = q.GetCurrentNPCCharacterPtr();

		lua_Number dist = 10;

		if (lua_isnumber(L, 1))
			dist = lua_tonumber(L, 1);

		if (ch == nullptr || npc == nullptr)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist*100);

		return 1;
	}

	ALUA(npc_is_near_vid)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid vid");
			lua_pushboolean(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		const auto ch = CHARACTER_MANAGER::Instance().Find((uint32_t)lua_tonumber(L, 1));
		const auto npc = q.GetCurrentNPCCharacterPtr();

		lua_Number dist = 10;

		if (lua_isnumber(L, 2))
			dist = lua_tonumber(L, 2);

		if (ch == nullptr || npc == nullptr)
			lua_pushboolean(L, false);
		else
			lua_pushboolean(L, DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY()) < dist*100);

		return 1;
	}

	ALUA(npc_unlock)
	{
		CQuestManager& q = CQuestManager::Instance();
		const auto ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		auto npc = q.GetCurrentNPCCharacterPtr();
		if (npc != nullptr)
		{
			if (npc->IsPC())
				return 0;

			if (npc->GetQuestNPCID() == ch->GetPlayerID())
				npc->SetQuestNPCID(0);
		}

		return 0;
	}

	ALUA(npc_lock)
	{
		CQuestManager& q = CQuestManager::Instance();
		const auto ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		auto npc = q.GetCurrentNPCCharacterPtr();
		if (!npc || npc->IsPC())
		{
			lua_pushboolean(L, TRUE);
			return 1;
		}

		if (npc->GetQuestNPCID() == 0 || npc->GetQuestNPCID() == ch->GetPlayerID())
		{
			npc->SetQuestNPCID(ch->GetPlayerID());
			lua_pushboolean(L, TRUE);
		}
		else
		{
			lua_pushboolean(L, FALSE);
		}

		return 1;
	}

	ALUA(npc_get_leader_vid)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		LPPARTY party = npc->GetParty();
		if (!party)
			return 0;

		const auto leader = party->GetLeader();
		if (!party) {	//@fixme524
			sys_err("npc_get_leader_vid: Function triggered without party");
			return 1;
		}

		if (leader)
			lua_pushnumber(L, leader->GetVID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	/*ALUA(npc_get_leader_vid)	//@fixme524	[VegaS Fix - alternative]
	{
		const LPCHARACTER npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		const LPPARTY pkParty = npc ? npc->GetParty() : nullptr;

		lua_pushnumber(L, pkParty && pkParty->GetLeader() ? pkParty->GetLeader()->GetVID() : 0);
		return 1;
	}
	
	if (npc.get_leader_vid() == 0) then
		syschat("You don't have a party or the leader is offline.")
		return
	end
	*/

	ALUA(npc_get_vid)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetVID());
		return 1;
	}

	ALUA(npc_get_vid_attack_mul)
	{
		const lua_Number vid = lua_tonumber(L, 1);
		auto targetChar = CHARACTER_MANAGER::Instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetAttMul());
		else
			lua_pushnumber(L, 0);


		return 1;
	}

	ALUA(npc_set_vid_attack_mul)
	{
		const lua_Number vid = lua_tonumber(L, 1);
		const lua_Number attack_mul = lua_tonumber(L, 2);

		auto targetChar = CHARACTER_MANAGER::Instance().Find(vid);

		if (targetChar)
			targetChar->SetAttMul(attack_mul);

		return 0;
	}

	ALUA(npc_get_vid_damage_mul)
	{
		const lua_Number vid = lua_tonumber(L, 1);
		auto targetChar = CHARACTER_MANAGER::Instance().Find(vid);

		if (targetChar)
			lua_pushnumber(L, targetChar->GetDamMul());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(npc_set_vid_damage_mul)
	{
		const lua_Number vid = lua_tonumber(L, 1);
		const lua_Number damage_mul = lua_tonumber(L, 2);

		auto targetChar = CHARACTER_MANAGER::Instance().Find(vid);

		if (targetChar)
			targetChar->SetDamMul(damage_mul);

		return 0;
	}

#ifdef ENABLE_NEWSTUFF
	ALUA(npc_get_level0)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetLevel());
		return 1;
	}

	ALUA(npc_get_name0)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushstring(L, npc->GetName());
		return 1;
	}

	ALUA(npc_get_pid0)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetPlayerID());
		return 1;
	}

	ALUA(npc_get_vnum0)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetRaceNum());
		return 1;
	}

	ALUA(npc_is_available0)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		lua_pushboolean(L, npc!=nullptr);
		return 1;
	}

#endif

	/**
	* Sends an effect to a given npc
	* Arguments: vid, effect_filename
	*/
	ALUA(npc_send_effect)
	{
		const lua_Number vid = lua_tonumber(L, 1);
		const char *fname = lua_tostring(L, 2);

		auto npc = CHARACTER_MANAGER::Instance().Find((uint32_t)vid);
		if (vid == 0 && !vid)
			npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();

		if(fname != "" && npc)
			npc->SpecificEffectPacket (fname);

		return 0;
	}

	/**
	* Get the local x coordinate of the current quest npc
	* Port of the pc function
	*/
	ALUA(npc_get_local_x)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		const LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(npc->GetMapIndex());
		
		if (pMap)
			lua_pushnumber(L, (npc->GetX() - pMap->m_setting.iBaseX) / 100);
		else
			lua_pushnumber(L, npc->GetX() / 100);

		return 1;
	}

	/**
	* Get the local y coordinate of the current quest npc.
	* Port of the pc function
	*/
	ALUA(npc_get_local_y)
	{
		CQuestManager& q = CQuestManager::Instance();
		const auto npc = q.GetCurrentNPCCharacterPtr();
		if (!npc)
			return 0;

		const LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(npc->GetMapIndex());

		if (pMap)
			lua_pushnumber(L, (npc->GetY() - pMap->m_setting.iBaseY) / 100);
		else
			lua_pushnumber(L, npc->GetY() / 100);

		return 1;
	}

#ifdef ENABLE_HWID_BAN
	ALUA(npc_get_hwid)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (npc && npc->IsPC())
		{
			LPDESC d = npc->GetDesc();
			if (!d)
				return 1;

			lua_pushstring(L, npc->GetDesc()->GetAccountTable().hwid);
		}
		else
		{
			lua_pushstring(L, "");
		}
		return 1;
	}
#endif

#ifdef ENABLE_NEWSTUFF
	ALUA(npc_purge_vid0)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("purge_vid0: invalid vid");
			return 0;
		}

		const uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		auto npc = CHARACTER_MANAGER::instance().Find(vid);

		if (npc)
			npc->Dead(); // M2_DESTROY_CHARACTER(npc); (Change by Marty)

		return 0;
	}

	ALUA(npc_is_available_vid0)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("is_available_vid0: invalid vid");
			return 0;
		}

		const uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		const auto npc = CHARACTER_MANAGER::instance().Find(vid);

		lua_pushboolean(L, npc != nullptr);
		return 1;
	}

	ALUA(npc_kill_vid0)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("kill_vid0: invalid vid");
			return 0;
		}

		const uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		auto npc = CHARACTER_MANAGER::instance().Find(vid);

		if (npc)
			npc->Dead();

		return 0;
	}

	ALUA(npc_get_x_vid0)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("kill_vid0: invalid vid");
			return 0;
		}

		const uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		const auto npc = CHARACTER_MANAGER::instance().Find(vid);
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetX());
		return 1;
	}

	ALUA(npc_get_y_vid0)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("kill_vid0: invalid vid");
			return 0;
		}

		const uint32_t vid = (uint32_t)lua_tonumber(L, 1);
		const auto npc = CHARACTER_MANAGER::instance().Find(vid);
		if (!npc)
			return 0;

		lua_pushnumber(L, npc->GetY());
		return 1;
	}

	ALUA(npc_get_rank)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetMobRank());
		return 1;
	}

	ALUA(npc_get_level)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetLevel());
		return 1;
	}

	ALUA(npc_get_type)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentNPCCharacterPtr()->GetType());
		return 1;
	}

	ALUA(npc_is_stone)
	{
		const auto npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		if (npc && npc->IsStone())
			lua_pushboolean(L, 1);
		else
			lua_pushboolean(L, 0);

		return 1;
	}
#endif

	void RegisterNPCFunctionTable()
	{
		luaL_reg npc_functions[] =
		{
			{ "getrace",			npc_get_race			},
			{ "get_race",			npc_get_race			},
			{ "open_shop",			npc_open_shop			},
			{ "get_empire",			npc_get_empire			},
			{ "is_pc",				npc_is_pc			},
			{ "get_name",			npc_get_name		},
			{ "is_quest",			npc_is_quest			},
			{ "kill",				npc_kill			},
			{ "purge",				npc_purge			},
			{ "is_near",			npc_is_near			},
			{ "is_near_vid",			npc_is_near_vid			},
			{ "lock",				npc_lock			},
			{ "unlock",				npc_unlock			},
			{ "get_guild",			npc_get_guild			},
			{ "get_leader_vid",		npc_get_leader_vid	},
			{ "get_vid",			npc_get_vid	},
			{ "get_vid_attack_mul",		npc_get_vid_attack_mul	},
			{ "set_vid_attack_mul",		npc_set_vid_attack_mul	},
			{ "get_vid_damage_mul",		npc_get_vid_damage_mul	},
			{ "set_vid_damage_mul",		npc_set_vid_damage_mul	},

			// X-mas santa special
			{ "get_remain_skill_book_count",	npc_get_remain_skill_book_count },
			{ "dec_remain_skill_book_count",	npc_dec_remain_skill_book_count },
			{ "get_remain_hairdye_count",	npc_get_remain_hairdye_count	},
			{ "dec_remain_hairdye_count",	npc_dec_remain_hairdye_count	},
#ifdef ENABLE_NEWSTUFF
			{ "get_level0",			npc_get_level0},	// [return lua number]
			{ "get_name0",			npc_get_name0},		// [return lua string]
			{ "get_pid0",			npc_get_pid0},		// [return lua number]
			{ "get_vnum0",			npc_get_vnum0},		// [return lua number]
			{ "is_available0",		npc_is_available0},	// [return lua boolean]
#endif
			{ "send_effect",			npc_send_effect			},
			{ "get_local_x",			npc_get_local_x			},
			{ "get_local_y",			npc_get_local_y			},

#ifdef ENABLE_HWID_BAN
			{ "get_hwid",					npc_get_hwid				},
#endif

#ifdef ENABLE_NEWSTUFF
			{ "purge_vid0",					npc_purge_vid0				},
			{ "is_available_vid0",			npc_is_available_vid0		},
			{ "get_x_vid0",					npc_get_x_vid0				},
			{ "get_y_vid0",					npc_get_y_vid0				},
			{ "kill_vid0",					npc_kill_vid0				},
			{ "get_rank",					npc_get_rank				},
			{ "get_level",					npc_get_level				},
			{ "get_type",					npc_get_type				},
			{ "is_stone",					npc_is_stone				},
#endif

			{ nullptr,				nullptr			    	}
		};

		CQuestManager::Instance().AddLuaFunctionTable("npc", npc_functions);
	}
};
