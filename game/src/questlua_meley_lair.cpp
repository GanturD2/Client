#include "stdafx.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#include "questlua.h"
#include "questmanager.h"
#include "MeleyLair.h"
#include "char.h"
#include "guild.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
#include "item_manager.h"
#include "party.h"
#endif

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	ALUA(meley_lair_get_requirements)
	{
		lua_pushnumber(L, MELEY_GUILD_LADDER_POINTS_COST);
		lua_pushnumber(L, MELEY_GUILD_LADDER_POINTS_RETURN);
		return 2;
	}

	ALUA(meley_lair_get_participants_limit)
	{
		lua_pushnumber(L, MELEY_PARTECIPANTS_MAX_LIMIT);
		return 1;
	}

	ALUA(meley_lair_get_sub_map_index)
	{
		lua_pushnumber(L, MELEY_SUBMAP_INDEX);
		return 1;
	}

	ALUA(meley_lair_register)
	{
		auto pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pkChar)
			return 0;

		int iRes1, iRes2;
		CMeleyLairManager::Instance().RegisterGuild(pkChar, iRes1, iRes2);
		lua_pushnumber(L, iRes1);
		lua_pushnumber(L, iRes2);
		return 2;
	}

	ALUA(meley_lair_is_registered)
	{
		const auto& pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			int iCH = 0;
			const bool bTry = pkChar->GetGuild() ? CMeleyLairManager::Instance().isGuildRegistered(pkChar->GetGuild(), iCH) : false;
			lua_pushboolean(L, bTry);
			lua_pushnumber(L, iCH);
		}
		else
		{
			lua_pushboolean(L, false);
			lua_pushnumber(L, 0);
		}

		return 2;
	}

	ALUA(meley_lair_enter_guild)
	{
		auto pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			int iLimit = 0;
			const bool bTry = pkChar->GetGuild() ? CMeleyLairManager::Instance().EnterGuild(pkChar, iLimit) : false;
			lua_pushboolean(L, bTry);
			lua_pushnumber(L, iLimit);
		}
		else
		{
			lua_pushboolean(L, false);
			lua_pushnumber(L, 1);
		}

		return 2;
	}

	ALUA(meley_lair_get_dungeon_type)
	{
		auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 0;
		}

		const auto dungeon_type = CMeleyLairManager::Instance().GetCharacterDungeonType(ch);
		lua_pushnumber(L, dungeon_type);

		return 1;
	}

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	ALUA(meley_lair_check_party_member)
	{
		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch || !ch->GetParty())
		{
			lua_pushnumber(L, 0);
			return 0;
		}

		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		int iTotal = 0;
		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			auto pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			const uint16_t bDungeonType = CMeleyLairManager::Instance().GetCharacterDungeonType(pPC);
			if (0 == bDungeonType)
				continue;

			if (bDungeonType != eType::MELEY_TYPE_PARTY_RED_DRAGON_LAIR)
			{
				lua_pushstring(L, pPC->GetName());
				iTotal++;
			}
		}
		return iTotal;
	}

	ALUA(meley_lair_check_party_ticket)
	{
		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch || !ch->GetParty())
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(10033);
		if (!pItemGroup)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const int iTicketCount = pItemGroup->GetGroupSize();

		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		int iTotal = 0;
		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			auto pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			if (CMeleyLairManager::Instance().IsPartyMemberRegistered(pPC))
				continue;

			bool hasTicket = false;
			for (int i = 0; i < iTicketCount; i++)
			{
				if (pPC->CountSpecifyItem(pItemGroup->GetVnum(i)) >= 1)
				{
					hasTicket = true;
					break;
				}
			}

			if (!hasTicket)
			{
				lua_pushstring(L, pPC->GetName());
				iTotal++;
			}

		}

		return iTotal;
	}

	ALUA(meley_lair_check_party_mount)
	{
		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!ch->GetParty())
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		int iTotal = 0;
		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			const auto& pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			if (pPC->IsRiding())
			{
				lua_pushstring(L, pPC->GetName());
				iTotal++;
			}

		}

		return iTotal;
	}

	ALUA(meley_lair_check_party_level)
	{
		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!ch->GetParty())
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		int iTotal = 0;
		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			const auto& pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			if (pPC->GetLevel() < MELEY_PLAYER_PARTY_MIN_LEVEL)
			{
				lua_pushstring(L, pPC->GetName());
				iTotal++;
			}

		}

		return iTotal;
	}

#ifdef MELEY_ENABLE_COOLDOWN
	ALUA(meley_lair_check_party_duration)
	{
		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 0;
		}

		if (!ch->GetParty())
		{
			lua_pushnumber(L, 0);
			return 0;
		}

		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		int iTotal = 0;
		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			const auto& pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			quest::PC* qPC = quest::CQuestManager::Instance().GetPCForce(pPC->GetPlayerID());
			if (!qPC)
			{
				lua_pushstring(L, pPC->GetName());
				iTotal++;
			}

			int duration_time = qPC->GetFlag(MELEY_PARTY_COOLDOWN_FLAG);
			if (get_global_time() < duration_time)
			{
				lua_pushstring(L, pPC->GetName());
				iTotal++;
			}
		}
		return iTotal;
	}
#endif

	ALUA(meley_lair_enter_party)
	{
		auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
		{
			int iLimit = 0;
			CMeleyLairManager::Instance().EnterParty(ch, iLimit);
			lua_pushnumber(L, iLimit);
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}
#endif

	ALUA(meley_lair_is_meley_map)
	{
		const auto& pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		const bool bTry = pkChar ? CMeleyLairManager::Instance().IsMeleyMap(pkChar->GetMapIndex()) : false;
		lua_pushboolean(L, bTry);
		return 1;
	}

	ALUA(meley_lair_leave)
	{
		auto pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pkChar)
			CMeleyLairManager::Instance().LeaveRequest(pkChar);
		return 0;
	}

	ALUA(meley_lair_can_get_reward)
	{
		auto pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pkChar)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		const bool bTry = pkChar ? CMeleyLairManager::Instance().CanGetReward(pkChar) : false;
		lua_pushboolean(L, bTry);
		return 1;
	}

	ALUA(meley_lair_reward)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		auto pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			const uint16_t bReward = static_cast<uint16_t>(lua_tonumber(L, 1));
			CMeleyLairManager::Instance().Reward(pkChar, bReward);
		}

		return 0;
	}

	ALUA(meley_lair_open_ranking)
	{
		auto pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pkChar)
			CMeleyLairManager::Instance().OpenRanking(pkChar);

		return 0;
	}

	ALUA(meley_lair_is_active)
	{
		auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		const bool bIsActive = CMeleyLairManager::Instance().GetDungeonStep(ch) == 0 ? FALSE : TRUE;
		lua_pushboolean(L, bIsActive);
		return 1;
	}

	ALUA(meley_lair_start_dungeon)
	{
		auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (CMeleyLairManager::Instance().IsMeleyMap(ch->GetMapIndex()))
			CMeleyLairManager::Instance().StartDungeon(ch);

		return 0;
	}

	ALUA(meley_lair_stop_dungeon)
	{
		auto ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (CMeleyLairManager::Instance().GetDungeonStep(ch) == 4)
			return 0;

		if (CMeleyLairManager::Instance().IsMeleyMap(ch->GetMapIndex()))
			CMeleyLairManager::Instance().EndDungeon(ch, false, true);

		return 0;
	}

	void RegisterMeleyLairFunctionTable()
	{
		luaL_reg functions[] =
		{
			{"get_requirements", meley_lair_get_requirements},
			{"get_participants_limit", meley_lair_get_participants_limit},
			{"get_sub_map_index", meley_lair_get_sub_map_index},
			{"register", meley_lair_register},
			{"is_registered", meley_lair_is_registered},
			{"enter_guild", meley_lair_enter_guild},
			{"get_dungeon_type", meley_lair_get_dungeon_type},
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
			{"check_party_member", meley_lair_check_party_member},
			{"check_party_ticket", meley_lair_check_party_ticket},
			{"check_party_mount", meley_lair_check_party_mount},
			{"check_party_level", meley_lair_check_party_level},
#	ifdef MELEY_ENABLE_COOLDOWN
			{"check_party_duration", meley_lair_check_party_duration},
#	endif
			{"enter_party", meley_lair_enter_party},
#endif
			{"is_meley_map", meley_lair_is_meley_map},
			{"leave", meley_lair_leave},
			{"can_get_reward", meley_lair_can_get_reward},
			{"reward", meley_lair_reward},
			{"open_ranking", meley_lair_open_ranking},
			{"is_active", meley_lair_is_active},
			{"start_dungeon", meley_lair_start_dungeon},
			{"stop_dungeon", meley_lair_stop_dungeon},
			{nullptr, nullptr}
		};

		CQuestManager::Instance().AddLuaFunctionTable("meley_lair", &functions[0]);
	}
}
#endif
