#include "stdafx.h"
#include "constants.h"
#include "questmanager.h"
#include "questlua.h"
#include "dungeon.h"
#include "char.h"
#include "party.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "packet.h"
#include "desc_client.h"
#include "desc_manager.h"
#ifdef ENABLE_D_NJGUILD
#include "guild.h"
#include "utils.h"
#include "config.h"
#include "guild_manager.h"
#include "../../common/stl.h"
#include "db.h"
#include "affect.h"
#include "p2p.h"
#include "war_map.h"
#include "sectree_manager.h"
#include "locale_service.h"
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
# include "DungeonInfo.h"
#endif

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

template <class Func> Func CDungeon::ForEachMember(Func f)
{
	for (auto it = m_set_pkCharacter.begin(); it != m_set_pkCharacter.end(); ++it)
	{
		sys_log(0, "Dungeon ForEachMember %s", (*it)->GetName());
		f(*it);
	}
	return f;
}

namespace quest
{
	//
	// "dungeon" lua functions
	//
	ALUA(dungeon_notice)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Notice(lua_tostring(L, 1));

		return 0;
	}

#ifdef ENABLE_NEW_CHAT_DUNGEON
	ALUA(dungeon_syschat)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Syschat(lua_tostring(L, 1));
		return 0;
	}

	ALUA(dungeon_bignotice)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();

		if (pDungeon)
			pDungeon->BigNotice(lua_tostring(L, 1));
		return 0;
	}
#endif

	ALUA(dungeon_set_quest_flag)
	{
		CQuestManager& q = CQuestManager::Instance();

		FSetQuestFlag f;

		f.flagname = q.GetCurrentPC()->GetCurrentQuestName() + "." + lua_tostring(L, 1);
		f.value = static_cast<int>(rint(lua_tonumber(L, 2)));

		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ForEachMember(f);

		return 0;
	}

	ALUA(dungeon_set_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong set flag");
		}
		else
		{
			LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
			if (pDungeon)
			{
				const char* sz = lua_tostring(L, 1);
				const int value = static_cast<int>(lua_tonumber(L, 2));
				pDungeon->SetFlag(sz, value);
			}
			else
			{
				sys_err("set_flag: no dungeon !!!");
			}
		}

		return 0;
	}

	ALUA(dungeon_get_flag)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong get flag");
			lua_pushnumber(L, 0);
			return 1;
		}

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
		{
			const char* sz = lua_tostring(L, 1);
			lua_pushnumber(L, pDungeon->GetFlag(sz));
		}
		else
		{
			sys_err("get_flag: no dungeon !!!");
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(dungeon_get_flag_from_map_index)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong get flag");
			lua_pushboolean(L, 0);
			return 1;
		}

		const uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 2));
		if (dwMapIndex)
		{
			LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
			{
				const char* sz = lua_tostring(L, 1);
				lua_pushnumber(L, pDungeon->GetFlag(sz));
			}
			else
			{
				sys_err("get_flag_from_map_index: no dungeon !!!");
				lua_pushnumber(L, 0);
			}
		}
		else
		{
			lua_pushboolean(L, 0);
		}

		return 1;
	}

	ALUA(dungeon_get_map_index)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
		{
			sys_log(0, "Dungeon GetMapIndex %d", pDungeon->GetMapIndex());
			lua_pushnumber(L, pDungeon->GetMapIndex());
		}
		else
		{
			sys_err("get_map_index: no dungeon !!!");
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(dungeon_regen_file)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong filename");
			return 0;
		}

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
			pDungeon->SpawnRegen(lua_tostring(L, 1));

		return 0;
	}

	ALUA(dungeon_set_regen_file)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("wrong filename");
			return 0;
		}

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
			pDungeon->SpawnRegen(lua_tostring(L, 1), false);

		return 0;
	}

	ALUA(dungeon_clear_regen)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
			pDungeon->ClearRegen();

		return 0;
	}

	ALUA(dungeon_check_eliminated)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
			pDungeon->CheckEliminated();

		return 0;
	}

	ALUA(dungeon_set_exit_all_at_eliminate)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong time");
			return 0;
		}

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon)
			pDungeon->SetExitAllAtEliminate(static_cast<long>(lua_tonumber(L, 1)));

		return 0;
	}

	ALUA(dungeon_set_warp_at_eliminate)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong time");
			return 0;
		}

		if (!lua_isnumber(L, 2))
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 3))
		{
			sys_err("wrong X");
			return 0;
		}

		if (!lua_isnumber(L, 4))
		{
			sys_err("wrong Y");
			return 0;
		}

		const char* c_pszRegenFile = nullptr;
		if (lua_gettop(L) >= 5)
			c_pszRegenFile = lua_tostring(L, 5);

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			pDungeon->SetWarpAtEliminate(static_cast<long>(lua_tonumber(L, 1)),
				static_cast<long>(lua_tonumber(L, 2)),
				static_cast<long>(lua_tonumber(L, 3)),
				static_cast<long>(lua_tonumber(L, 4)),
				c_pszRegenFile);
		}
		else
			sys_err("cannot find dungeon");

		return 0;
	}

	ALUA(dungeon_new_jump)
	{
		if (lua_gettop(L) < 3)
		{
			sys_err("not enough argument");
			return 0;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong argument");
			return 0;
		}

		const long lMapIndex = static_cast<long>(lua_tonumber(L, 1));

		LPDUNGEON pDungeon = CDungeonManager::Instance().Create(lMapIndex);

		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
		{
			//ch->WarpSet(pDungeon->GetMapIndex(), (int) lua_tonumber(L, 2), (int)lua_tonumber(L, 3));
			ch->WarpSet(static_cast<int>(lua_tonumber(L, 2)), static_cast<int>(lua_tonumber(L, 3)), pDungeon->GetMapIndex());
		}
		return 0;
	}

#ifdef ENABLE_D_NJGUILD
	ALUA(dungeon_new_jump_guild)
	{
		if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("not enough argument");
			return 0;
		}

		const long lMapIndex = static_cast<long>(lua_tonumber(L, 1));

		LPDUNGEON pDungeon = CDungeonManager::Instance().Create(lMapIndex);

		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (ch->GetGuild() == nullptr)
		{
			sys_err("cannot go to dungeon alone.");
			return 0;
		}

		pDungeon->JumpGuild(ch->GetGuild(), ch->GetMapIndex(), static_cast<int>(lua_tonumber(L, 2)), static_cast<int>(lua_tonumber(L, 3)));

		return 0;
	}
#endif

#ifdef ENABLE_DUNGEON_RENEWAL
	struct FRegisterAllDungeonMap
	{
		long lMapIndex;
		LPDUNGEON lpDungeon;
		FRegisterAllDungeonMap(long lMapIndex, LPDUNGEON lpDungeon)
			: lMapIndex(lMapIndex), lpDungeon(lpDungeon)
		{}

		void operator()(LPENTITY ent)
		{
			if (!ent)
				return;

			if (!ent->IsType(ENTITY_CHARACTER))
				return;

			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (!ch)
				return;

			if (!ch->IsPC())
				return;

			if (ch->GetMapIndex() == lMapIndex)
			{
				lpDungeon->RegisterParticipant(ch);
			}
		}
	};
#endif

	ALUA(dungeon_new_jump_all)
	{
		if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("not enough argument");
			return 0;
		}

		const long lMapIndex = static_cast<long>(lua_tonumber(L, 1));
		LPDUNGEON pDungeon = CDungeonManager::Instance().Create(lMapIndex);
		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

#ifdef ENABLE_DUNGEON_RENEWAL
		// important register player before teleport
		LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);
		if (pMap)
		{
			FRegisterAllDungeonMap f(lMapIndex, pDungeon);
			pMap->for_each(f);
		}
#endif

		const auto posX = static_cast<int>(lua_tonumber(L, 2));
		const auto posY = static_cast<int>(lua_tonumber(L, 3));
		pDungeon->JumpAll(ch->GetMapIndex(), posX, posY);

		return 0;
	}

	ALUA(dungeon_new_jump_party)
	{
		if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("not enough argument");
			return 0;
		}

		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty() == nullptr)
		{
			sys_err("cannot go to dungeon alone.");
			return 0;
		}

		const auto lMapIndex = static_cast<long>(lua_tonumber(L, 1));
		LPDUNGEON pDungeon = CDungeonManager::Instance().Create(lMapIndex);
		if (!pDungeon)
		{
			sys_err("cannot create dungeon %d", lMapIndex);
			return 0;
		}

#ifdef ENABLE_DUNGEON_RENEWAL
		struct FPartyPIDCollector
		{
			std::vector<uint32_t> vecPIDs{};
			FPartyPIDCollector() noexcept {}
			void operator () (LPCHARACTER ch)
			{
				if (ch)
					vecPIDs.emplace_back(ch->GetPlayerID());
			}
		};

		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			const auto pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			pDungeon->RegisterParticipant(pPC);
		}
#endif

		const auto posX = static_cast<int>(lua_tonumber(L, 2));
		const auto posY = static_cast<int>(lua_tonumber(L, 3));
		pDungeon->JumpParty(ch->GetParty(), ch->GetMapIndex(), posX, posY);

		return 0;
	}

	ALUA(dungeon_jump_all)
	{
		if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();

		if (!pDungeon)
			return 0;

		pDungeon->JumpAll(pDungeon->GetMapIndex(), static_cast<int>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)));
		return 0;
	}

	ALUA(dungeon_warp_all)
	{
		if (lua_gettop(L) < 2 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();

		if (!pDungeon)
			return 0;

		pDungeon->WarpAll(pDungeon->GetMapIndex(), static_cast<int>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)));
		return 0;
	}

	ALUA(dungeon_get_kill_stone_count)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}


		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetKillStoneCount());
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_get_kill_mob_count)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetKillMobCount());
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_is_use_potion)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, 1);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUsePotion());
			return 1;
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(dungeon_revived)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, 1);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUseRevive());
			return 1;
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(dungeon_set_dest)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPPARTY pParty = ch->GetParty();
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon && pParty)
			pDungeon->SendDestPositionToParty(pParty, static_cast<int>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)));

		return 0;
	}

	ALUA(dungeon_unique_set_maxhp)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetMaxHP(lua_tostring(L, 1), static_cast<int>(lua_tonumber(L, 2)));

		return 0;
	}

	ALUA(dungeon_unique_set_hp)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetHP(lua_tostring(L, 1), static_cast<int>(lua_tonumber(L, 2)));

		return 0;
	}

	ALUA(dungeon_unique_set_def_grade)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->UniqueSetDefGrade(lua_tostring(L, 1), static_cast<int>(lua_tonumber(L, 2)));

		return 0;
	}

	ALUA(dungeon_unique_get_hp_perc)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetUniqueHpPerc(lua_tostring(L, 1)));
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_is_unique_dead)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushboolean(L, pDungeon->IsUniqueDead(lua_tostring(L, 1)) ? 1 : 0);
			return 1;
		}

		lua_pushboolean(L, 0);
		return 1;
	}

	ALUA(dungeon_purge_unique)
	{
		if (!lua_isstring(L, 1))
			return 0;
		sys_log(0, "QUEST_DUNGEON_PURGE_UNIQUE %s", lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->PurgeUnique(lua_tostring(L, 1));

		return 0;
	}

	struct FPurgeArea
	{
		int x1, y1, x2, y2;
		LPCHARACTER ExceptChar;

		FPurgeArea(int a, int b, int c, int d, LPCHARACTER p) noexcept
			: x1(a), y1(b), x2(c), y2(d),
			ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(ent);

				if (!pChar || (pChar == ExceptChar))
					return;

				if (!pChar->IsPet() &&
#ifdef ENABLE_GROWTH_PET_SYSTEM
					!pChar->IsGrowthPet() &&
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					!pChar->IsHorse() &&
#endif
					(true == pChar->IsMonster() || true == pChar->IsStone()))
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						if (!pChar->IsDead())
							pChar->DeadNoReward(); // @fixme188 from Dead()
					}
				}
			}
		}
	};

	ALUA(dungeon_purge_area)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;
		sys_log(0, "QUEST_DUNGEON_PURGE_AREA");

		const int x1 = static_cast<int>(lua_tonumber(L, 1));
		const int y1 = static_cast<int>(lua_tonumber(L, 2));
		const int x2 = static_cast<int>(lua_tonumber(L, 3));
		const int y2 = static_cast<int>(lua_tonumber(L, 4));

		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();

		const int mapIndex = pDungeon->GetMapIndex();

		if (0 == mapIndex)
		{
			sys_err("_purge_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(mapIndex);

		if (nullptr != pSectree)
		{
			FPurgeArea func(x1, y1, x2, y2, CQuestManager::Instance().GetCurrentNPCCharacterPtr());

			pSectree->for_each(func);
		}

		return 0;
	}

#ifdef ENABLE_NEWSTUFF
	struct FKillArea
	{
		int x1, y1, x2, y2;
		LPCHARACTER ExceptChar;

		FKillArea(int a, int b, int c, int d, LPCHARACTER p)
			: x1(a), y1(b), x2(c), y2(d),
			ExceptChar(p)
		{}

		void operator () (LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(ent);
				if (!pChar)
					return;

				if (pChar == ExceptChar)
					return;

				if (!pChar->IsPet() && (true == pChar->IsMonster() || true == pChar->IsStone())
#ifdef ENABLE_GROWTH_PET_SYSTEM
					&& !pChar->IsGrowthPet()
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					&& !pChar->IsHorse()
#endif
					)
				{
					if (x1 <= pChar->GetX() && pChar->GetX() <= x2 && y1 <= pChar->GetY() && pChar->GetY() <= y2)
					{
						if (!pChar->IsDead())
							pChar->DeadNoReward(); // @fixme188 from Dead()
					}
				}
			}
		}
	};

	ALUA(dungeon_kill_area)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;
		sys_log(0, "QUEST_DUNGEON_KILL_AREA");

		const int x1 = static_cast<int>(lua_tonumber(L, 1));
		const int y1 = static_cast<int>(lua_tonumber(L, 2));
		const int x2 = static_cast<int>(lua_tonumber(L, 3));
		const int y2 = static_cast<int>(lua_tonumber(L, 4));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		const int mapIndex = pDungeon->GetMapIndex();

		if (0 == mapIndex)
		{
			sys_err("_kill_area: cannot get a map index with (%u, %u)", x1, y1);
			return 0;
		}

		LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(mapIndex);

		if (nullptr != pSectree)
		{
			FKillArea func(x1, y1, x2, y2, CQuestManager::Instance().GetCurrentNPCCharacterPtr());

			pSectree->for_each(func);
		}

		return 0;
	}
#endif

	ALUA(dungeon_kill_unique)
	{
		if (!lua_isstring(L, 1))
			return 0;
		sys_log(0, "QUEST_DUNGEON_KILL_UNIQUE %s", lua_tostring(L, 1));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillUnique(lua_tostring(L, 1));

		return 0;
	}

	ALUA(dungeon_spawn_stone_door)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_STONE_DOOR %s %s", lua_tostring(L, 1), lua_tostring(L, 2));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnStoneDoor(lua_tostring(L, 1), lua_tostring(L, 2));

		return 0;
	}

	ALUA(dungeon_spawn_wooden_door)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_WOODEN_DOOR %s %s", lua_tostring(L, 1), lua_tostring(L, 2));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnWoodenDoor(lua_tostring(L, 1), lua_tostring(L, 2));

		return 0;
	}

	ALUA(dungeon_spawn_move_group)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_MOVE_GROUP %d %s %s", static_cast<int>(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnMoveGroup(static_cast<uint32_t>(lua_tonumber(L, 1)), lua_tostring(L, 2), lua_tostring(L, 3));

		return 0;
	}

	ALUA(dungeon_spawn_move_unique)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3) || !lua_isstring(L, 4))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_MOVE_UNIQUE %s %d %s %s", lua_tostring(L, 1), static_cast<int>(lua_tonumber(L, 2)), lua_tostring(L, 3), lua_tostring(L, 4));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnMoveUnique(lua_tostring(L, 1), static_cast<uint32_t>(lua_tonumber(L, 2)), lua_tostring(L, 3), lua_tostring(L, 4));

		return 0;
	}

	ALUA(dungeon_spawn_unique)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN_UNIQUE %s %d %s", lua_tostring(L, 1), static_cast<int>(lua_tonumber(L, 2)), lua_tostring(L, 3));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnUnique(lua_tostring(L, 1), static_cast<uint32_t>(lua_tonumber(L, 2)), lua_tostring(L, 3));

		return 0;
	}

	ALUA(dungeon_spawn)
	{
		if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
			return 0;
		sys_log(0, "QUEST_DUNGEON_SPAWN %d %s", static_cast<int>(lua_tonumber(L, 1)), lua_tostring(L, 2));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Spawn(static_cast<uint32_t>(lua_tonumber(L, 1)), lua_tostring(L, 2));

		return 0;
	}

	ALUA(dungeon_set_unique)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

		if (pDungeon)
			pDungeon->SetUnique(lua_tostring(L, 1), vid);
		return 0;
	}

#ifdef NEW_ICEDAMAGE_SYSTEM
	ALUA(dungeon_get_damage_from_race)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 1));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);

		lua_pushnumber(L, (ch) ? ch->GetNoDamageRaceFlag() : 0);
		return 1;
	}

	ALUA(dungeon_get_damage_from_affect)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 1));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);

		lua_newtable(L);
		if (ch)
		{
			const std::set<uint32_t>& tmp_setNDAFlag = ch->GetNoDamageAffectFlag();
			if (tmp_setNDAFlag.size())
			{
				uint32_t dwTmpLuaIdx = 1;
				for (std::set<uint32_t>::iterator it = tmp_setNDAFlag.begin(); it != tmp_setNDAFlag.end(); ++it)
				{
					lua_pushnumber(L, *it);
					lua_rawseti(L, -2, dwTmpLuaIdx++);
				}
			}
		}
		return 1;
	}

	ALUA(dungeon_set_damage_from_race)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch)
			ch->SetNoDamageRaceFlag(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_set_damage_from_affect)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch)
			ch->SetNoDamageAffectFlag(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_unset_damage_from_race)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch)
			ch->UnsetNoDamageRaceFlag(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_unset_damage_from_affect)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 2));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch)
			ch->UnsetNoDamageAffectFlag(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_reset_damage_from_race)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 1));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch)
			ch->ResetNoDamageRaceFlag();
		return 0;
	}

	ALUA(dungeon_reset_damage_from_affect)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		// extra check for d BEGIN
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;
		// extra check for d END

		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 1));

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch)
			ch->ResetNoDamageAffectFlag();
		return 0;
	}
#endif

#ifdef ENABLE_NEWSTUFF
	ALUA(dungeon_is_available0)
	{
		CQuestManager& q = CQuestManager::Instance();
		const LPDUNGEON& pDungeon = q.GetCurrentDungeon();

		lua_pushboolean(L, pDungeon != nullptr);
		return 1;
	}
#endif

	ALUA(dungeon_get_unique_vid)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			lua_pushnumber(L, pDungeon->GetUniqueVid(lua_tostring(L, 1)));
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_spawn_mob)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			const long x = static_cast<long>(lua_tonumber(L, 2));
			const long y = static_cast<long>(lua_tonumber(L, 3));
			const float radius = lua_isnumber(L, 4) ? static_cast<float>(lua_tonumber(L, 4)) : 0;
			uint32_t count = (lua_isnumber(L, 5)) ? static_cast<uint32_t>(lua_tonumber(L, 5)) : 1;

			sys_log(0, "dungeon_spawn_mob %u %d %d", dwVnum, x, y);

			if (count == 0)
				count = 1;

			while (count--)
			{
				if (radius < 1)
				{
					const LPCHARACTER& ch = pDungeon->SpawnMob(dwVnum, x, y);
					if (ch && !vid)
						vid = ch->GetVID();

#ifdef ENABLE_DEFENSE_WAVE
					if (dwVnum == DEFENSE_WAVE_MAST_VNUM)
						pDungeon->SetMast(ch);
#endif
				}
				else
				{
					const float angle = number(0, 999) * static_cast<float>(M_PI) * 2 / 1000;
					const float r = number(0, 999) * radius / 1000;

					const long nx = x + static_cast<long>(r * cos(angle));
					const long ny = y + static_cast<long>(r * sin(angle));

					const LPCHARACTER& ch = pDungeon->SpawnMob(dwVnum, nx, ny);
					if (ch && !vid)
						vid = ch->GetVID();
#ifdef ENABLE_DEFENSE_WAVE
					if (dwVnum == DEFENSE_WAVE_MAST_VNUM)
						pDungeon->SetMast(ch);
#endif
				}
			}
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_mob_dir)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			const long x = static_cast<long>(lua_tonumber(L, 2));
			const long y = static_cast<long>(lua_tonumber(L, 3));
			const uint8_t dir = static_cast<int>(lua_tonumber(L, 4));

			const LPCHARACTER& ch = pDungeon->SpawnMob(dwVnum, x, y, dir);
			if (ch && !vid)
				vid = ch->GetVID();
#ifdef ENABLE_DEFENSE_WAVE
			if (dwVnum == DEFENSE_WAVE_MAST_VNUM)
				pDungeon->SetMast(ch);
#endif
		}
		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_mob_ac_dir)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			const long x = static_cast<long>(lua_tonumber(L, 2));
			const long y = static_cast<long>(lua_tonumber(L, 3));
			const uint8_t dir = static_cast<int>(lua_tonumber(L, 4));

			const LPCHARACTER& ch = pDungeon->SpawnMob_ac_dir(dwVnum, x, y, dir);
			if (ch && !vid)
				vid = ch->GetVID();

#ifdef ENABLE_DEFENSE_WAVE
			if (dwVnum == DEFENSE_WAVE_MAST_VNUM)
				pDungeon->SetMast(ch);
#endif
		}
		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_goto_mob)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;

		const long lFromX = static_cast<long>(lua_tonumber(L, 1));
		const long lFromY = static_cast<long>(lua_tonumber(L, 2));
		const long lToX = static_cast<long>(lua_tonumber(L, 3));
		const long lToY = static_cast<long>(lua_tonumber(L, 4));

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SpawnGotoMob(lFromX, lFromY, lToX, lToY);

		return 0;
	}

	ALUA(dungeon_spawn_name_mob)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isstring(L, 4))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			const long x = static_cast<long>(lua_tonumber(L, 2));
			const long y = static_cast<long>(lua_tonumber(L, 3));
			pDungeon->SpawnNameMob(dwVnum, x, y, lua_tostring(L, 4));
		}
		return 0;
	}

	ALUA(dungeon_spawn_group)
	{
		//
		// argument: vnum,x,y,radius,aggressive,count
		//
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t vid = 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			const uint32_t group_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			const long local_x = static_cast<long>(lua_tonumber(L, 2)) * 100;
			const long local_y = static_cast<long>(lua_tonumber(L, 3)) * 100;
			const float radius = static_cast<float>(lua_tonumber(L, 4)) * 100;
			const bool bAggressive = lua_toboolean(L, 5);
			const uint32_t count = static_cast<uint32_t>(lua_tonumber(L, 6));

			const LPCHARACTER& chRet = pDungeon->SpawnGroup(group_vnum, local_x, local_y, radius, bAggressive, count);
			if (chRet)
				vid = chRet->GetVID();
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_join)
	{
		if (lua_gettop(L) < 1 || !lua_isnumber(L, 1))
			return 0;

		const long lMapIndex = static_cast<long>(lua_tonumber(L, 1));
		LPDUNGEON pDungeon = CDungeonManager::Instance().Create(lMapIndex);

		if (!pDungeon)
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

#ifdef D_JOIN_AS_JUMP_PARTY
		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);
		if (!ch->GetParty())
		{
			sys_err("cannot go to dungeon alone.");
			return 0;
		}
		pDungeon->JumpParty(ch->GetParty(), ch->GetMapIndex(), pkSectreeMap->m_setting.posSpawn.x, pkSectreeMap->m_setting.posSpawn.y);
#else
		if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			pDungeon->JoinParty(ch->GetParty());
		else if (!ch->GetParty())
			pDungeon->Join(ch);
#endif
		return 0;
	}

	ALUA(dungeon_exit) // Sent to the location before entering the dungeon
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->ExitToSavedLocation();
		return 0;
	}

	ALUA(dungeon_exit_all) // Sends everyone in the dungeon to the location before entering the dungeon
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ExitAll();

		return 0;
	}

	struct FSayDungeonByItemGroup
	{
		const CDungeon::ItemGroup* item_group{};
		std::string can_enter_ment;
		std::string cant_enter_ment;
		void operator()(LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				const LPCHARACTER& ch = dynamic_cast<LPCHARACTER>(ent);

				if (ch && ch->IsPC())
				{
					TPacketGCScript packet_script{};
					TEMP_BUFFER buf;

					for (CDungeon::ItemGroup::const_iterator it = item_group->begin(); it != item_group->end(); ++it)	//@fixme541
					{
						if (ch->CountSpecifyItem(it->first) >= it->second)
						{
							packet_script.header = HEADER_GC_SCRIPT;
							packet_script.skin = quest::CQuestManager::QUEST_SKIN_NORMAL;
							packet_script.src_size = static_cast<uint16_t>(can_enter_ment.size());
							packet_script.size = packet_script.src_size + sizeof(TPacketGCScript);

							buf.write(&packet_script, sizeof(TPacketGCScript));
							buf.write(&can_enter_ment[0], can_enter_ment.size());
							ch->GetDesc()->Packet(buf.read_peek(), buf.size());
							return;
						}
					}

					packet_script.header = HEADER_GC_SCRIPT;
					packet_script.skin = quest::CQuestManager::QUEST_SKIN_NORMAL;
					packet_script.src_size = static_cast<uint16_t>(cant_enter_ment.size());
					packet_script.size = packet_script.src_size + sizeof(TPacketGCScript);

					buf.write(&packet_script, sizeof(TPacketGCScript));
					buf.write(&cant_enter_ment[0], cant_enter_ment.size());
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
				}
			}
		}
	};

	ALUA(dungeon_say_diff_by_item_group)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isstring(L, 3))
		{
			sys_log(0, "QUEST wrong set flag");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		CDungeon* pDungeon = q.GetCurrentDungeon();

		if (!pDungeon)
		{
			sys_err("QUEST : no dungeon");
			return 0;
		}

		SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(pDungeon->GetMapIndex());

		if (!pMap)
		{
			sys_err("cannot find map by index %d", pDungeon->GetMapIndex());
			return 0;
		}
		FSayDungeonByItemGroup f;
		sys_log(0, "diff_by_item");

		std::string group_name(lua_tostring(L, 1));
		f.item_group = pDungeon->GetItemGroup(group_name);

		if (f.item_group == nullptr)
		{
			sys_err("invalid item group");
			return 0;
		}

		f.can_enter_ment = lua_tostring(L, 2);
		f.can_enter_ment += "[ENTER][ENTER][ENTER][ENTER][DONE]";
		f.cant_enter_ment = lua_tostring(L, 3);
		f.cant_enter_ment += "[ENTER][ENTER][ENTER][ENTER][DONE]";

		pMap->for_each(f);

		return 0;
	}

	struct FExitDungeonByItemGroup
	{
		const CDungeon::ItemGroup* item_group;

		void operator()(LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);

				if (ch && ch->IsPC())
				{
					for (CDungeon::ItemGroup::const_iterator it = item_group->begin(); it != item_group->end(); it++)
					{
						if (ch->CountSpecifyItem(it->first) >= it->second)
						{
							return;
						}
					}
					ch->ExitToSavedLocation();
					// Nyx: Quit or remove party for prevent join again with the group
					LPPARTY pParty = ch->GetParty();
					if (pParty)
					{
						if (pParty->GetMemberCount() == 2)
							CPartyManager::Instance().DeleteParty(pParty);
						else
							pParty->Quit(ch->GetPlayerID());
					}
				}
			}
		}
	};

	ALUA(dungeon_exit_all_by_item_group) // Anyone who does not have an item belonging to a specific item group will be kicked out
	{
		if (!lua_isstring(L, 1))
		{
			sys_log(0, "QUEST wrong set flag");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		CDungeon* pDungeon = q.GetCurrentDungeon();

		if (!pDungeon)
		{
			sys_err("QUEST : no dungeon");
			return 0;
		}

		SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(pDungeon->GetMapIndex());

		if (!pMap)
		{
			sys_err("cannot find map by index %d", pDungeon->GetMapIndex());
			return 0;
		}
		FExitDungeonByItemGroup f{};

		std::string group_name(lua_tostring(L, 1));
		f.item_group = pDungeon->GetItemGroup(group_name);

		if (f.item_group == nullptr)
		{
			sys_err("invalid item group");
			return 0;
		}

		pMap->for_each(f);

		return 0;
	}

	struct FDeleteItemInItemGroup
	{
		const CDungeon::ItemGroup* item_group;

		void operator()(LPENTITY ent)
		{
			if (ent && ent->IsType(ENTITY_CHARACTER))
			{
				LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);

				if (ch && ch->IsPC())
				{
					for (CDungeon::ItemGroup::const_iterator it = item_group->begin(); it != item_group->end(); it++)
					{
						if (ch->CountSpecifyItem(it->first) >= it->second)
						{
							ch->RemoveSpecifyItem(it->first, it->second);
							return;
						}
					}
				}
			}
		}
	};

	ALUA(dungeon_delete_item_in_item_group_from_all) // Delete a specific item from the PC in the dungeon.
	{
		if (!lua_isstring(L, 1))
		{
			sys_log(0, "QUEST wrong set flag");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		CDungeon* pDungeon = q.GetCurrentDungeon();

		if (!pDungeon)
		{
			sys_err("QUEST : no dungeon");
			return 0;
		}

		SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(pDungeon->GetMapIndex());

		if (!pMap)
		{
			sys_err("cannot find map by index %d", pDungeon->GetMapIndex());
			return 0;
		}
		FDeleteItemInItemGroup f{};

		std::string group_name(lua_tostring(L, 1));
		f.item_group = pDungeon->GetItemGroup(group_name);

		if (f.item_group == nullptr)
		{
			sys_err("invalid item group");
			return 0;
		}

		pMap->for_each(f);

		return 0;
	}


	ALUA(dungeon_kill_all)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillAll();

		return 0;
	}

	ALUA(dungeon_purge)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->Purge();

		return 0;
	}

	ALUA(dungeon_exit_all_to_start_position)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ExitAllToStartPosition();

		return 0;
	}

	ALUA(dungeon_count_monster)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			lua_pushnumber(L, pDungeon->CountMonster());
		else
		{
			sys_err("not in a dungeon");
			lua_pushnumber(L, LONG_MAX);
		}

		return 1;
	}

	ALUA(dungeon_select)
	{
		const uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 1));
		if (dwMapIndex)
		{
			LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
			{
				CQuestManager::Instance().SelectDungeon(pDungeon);
				lua_pushboolean(L, 1);
			}
			else
			{
				CQuestManager::Instance().SelectDungeon(nullptr);
				lua_pushboolean(L, 0);
			}
		}
		else
		{
			CQuestManager::Instance().SelectDungeon(nullptr);
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	ALUA(dungeon_find)
	{
		const uint32_t dwMapIndex = static_cast<uint32_t>(lua_tonumber(L, 1));
		if (dwMapIndex)
		{
			const LPDUNGEON& pDungeon = CDungeonManager::Instance().FindByMapIndex(dwMapIndex);
			if (pDungeon)
				lua_pushboolean(L, 1);
			else
				lua_pushboolean(L, 0);
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	ALUA(dungeon_all_near_to)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();

		if (pDungeon != nullptr)
			lua_pushboolean(L, pDungeon->IsAllPCNearTo(static_cast<int>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)), 30));
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(dungeon_set_warp_location)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon == nullptr)
			return 0;

		if (lua_gettop(L) < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
			return 0;

		const FSetWarpLocation f(static_cast<int>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)), static_cast<int>(lua_tonumber(L, 3)));
		pDungeon->ForEachMember(f);

		return 0;
	}

	ALUA(dungeon_set_item_group)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		std::string group_name(lua_tostring(L, 1));
		const int size = static_cast<int>(lua_tonumber(L, 2));

		CDungeon::ItemGroup item_group;

		for (int i = 0; i < size; i++)
		{
			if (!lua_isnumber(L, i * 2 + 3) || !lua_isnumber(L, i * 2 + 4))
				return 0;

			item_group.emplace_back(std::pair <uint32_t, int>(lua_tonumber(L, i * 2 + 3), lua_tonumber(L, i * 2 + 4)));
		}
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (pDungeon == nullptr)
			return 0;

		pDungeon->CreateItemGroup(group_name, item_group);
		return 0;
	}

	ALUA(dungeon_set_quest_flag2)
	{
		CQuestManager& q = CQuestManager::Instance();

		FSetQuestFlag f;

		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("Invalid Argument");
		}

		f.flagname = string(lua_tostring(L, 1)) + "." + lua_tostring(L, 2);
		f.value = static_cast<int>(rint(lua_tonumber(L, 3)));

		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ForEachMember(f);

		return 0;
	}

#ifdef ENABLE_NEWSTUFF
	ALUA(dungeon_kill_monsters)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->KillMonsters();

		return 0;
	}
#endif

#ifdef ENABLE_NEWSTUFF	//ENABLE_ORC_MAZE
	ALUA(dungeon_spawn_goto_mob0)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
			return 0;

		long lFromX = (long)lua_tonumber(L, 1);
		long lFromY = (long)lua_tonumber(L, 2);
		const long lToX = (long)lua_tonumber(L, 3);
		const long lToY = (long)lua_tonumber(L, 4);

		const int MOB_GOTO_VNUM = 20039;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			const long mapIndex = pDungeon->GetMapIndex();
			LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(mapIndex);
			if (pkSectreeMap == nullptr)
				return 0;

			sys_log(0, "SpawnGotoMob %d %d to %d %d", lFromX, lFromY, lToX, lToY);

			lFromX = pkSectreeMap->m_setting.iBaseX + lFromX * 100;
			lFromY = pkSectreeMap->m_setting.iBaseY + lFromY * 100;

			LPCHARACTER mob = CHARACTER_MANAGER::Instance().SpawnMob(MOB_GOTO_VNUM, mapIndex, lFromX, lFromY, 0, false, -1);

			if (mob)
			{
				char buf[30 + 1];
				snprintf(buf, sizeof(buf), ". %ld %ld", lToX, lToY);

				mob->SetName(buf);
				mob->SetDungeon(pDungeon);

				lua_pushnumber(L, (mob) ? mob->GetVID() : 0);
			}
		}

		return 1;
	}
#endif

#ifdef ENABLE_DUNGEON_DEAD_TIME_RENEWAL
	ALUA(dungeon_set_dead_time)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		long lDeadTime = static_cast<long>(lua_tonumber(L, 1));

		if (lDeadTime <= 0)
			lDeadTime = 10;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			pDungeon->SetDeadTime(lDeadTime);
			return 1;
		}

		return 0;
	}
#endif

#ifdef ENABLE_NEWSTUFF //LEVEL_RANGE_EXTEND
	ALUA(dungeon_spawn_group_level_range)
	{
		//
		// argument: vnum,x,y,radius,aggressive,count,min_level,max_level
		//
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) ||
			!lua_isnumber(L, 6) || !lua_isnumber(L, 7) || !lua_isnumber(L, 8) ||
			!lua_isboolean(L, 9) || !lua_isboolean(L, 10))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t vid = 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			const uint32_t group_vnum = (uint32_t)lua_tonumber(L, 1);
			const long local_x = (long)lua_tonumber(L, 2) * 100;
			const long local_y = (long)lua_tonumber(L, 3) * 100;
			const float radius = (float)lua_tonumber(L, 4) * 100;
			const bool bAggressive = lua_toboolean(L, 5);
			const uint32_t count = (uint32_t)lua_tonumber(L, 6);
			const uint8_t min_level = (uint8_t)lua_tonumber(L, 7);
			const uint8_t max_level = (uint8_t)lua_tonumber(L, 8);
			const bool bIncreaseHP = lua_toboolean(L, 9);
			const bool bIncreaseDamage = lua_toboolean(L, 10);

			LPCHARACTER chRet = pDungeon->SpawnGroupLevelRange(group_vnum, local_x, local_y, radius, bAggressive, count, min_level, max_level, bIncreaseHP, bIncreaseDamage);
			if (chRet)
				vid = chRet->GetVID();
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_spawn_mob_level_range)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			const long x = static_cast<long>(lua_tonumber(L, 2));
			const long y = static_cast<long>(lua_tonumber(L, 3));
			const uint8_t min_level = static_cast<uint8_t>(lua_tonumber(L, 4));
			const uint8_t max_level = static_cast<uint8_t>(lua_tonumber(L, 5));
			const float radius = lua_isnumber(L, 6) ? static_cast<float>(lua_tonumber(L, 6)) : 0;
			uint32_t count = (lua_isnumber(L, 7)) ? static_cast<uint32_t>(lua_tonumber(L, 7)) : 1;

			sys_log(0, "dungeon_spawn_mob %u %d %d", dwVnum, x, y);

			if (count == 0)
				count = 1;

			while (count--)
			{
				const uint8_t level = static_cast<uint8_t>(number(min_level, max_level));
				const float fAttMul = static_cast<float>(level / 10);
				const float fDamMul = 1.0f + ((gPlayerMaxLevel - level) / fAttMul / 2);

				long posX = x;
				long posY = y;

				if (radius > 1)
				{
					const float angle = number(0, 999) * static_cast<float>(M_PI) * 2 / 1000;
					const float r = number(0, 999) * radius / 1000;

					posX = x + static_cast<long>(r * cos(angle));
					posY = y + static_cast<long>(r * sin(angle));
				}

				LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, posX, posY);
				if (ch)
				{
					vid = ch->GetVID();
					ch->SetLevel(level);
					ch->SetMaxHP(ch->GetMaxHP() * level);
					ch->SetHP(ch->GetMaxHP());

					ch->SetAttMul(fAttMul);
					ch->SetDamMul(fDamMul);

					ch->UpdatePacket();
				}
			}
		}

		lua_pushnumber(L, vid);
		return 1;
	}
#endif

#ifdef ENABLE_DUNGEON_RENEWAL
	// Clear Participants
	ALUA(dungeon_clear_register)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		pDungeon->ClearParticipants();
		return 0;
	}

	// Register Participant
	ALUA(dungeon_register_participant)
	{
		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		pDungeon->RegisterParticipant(ch);
		return 0;
	}

	// Register Party Participants
	ALUA(dungeon_register_party_participants)
	{
		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch || !ch->GetParty())
		{
			lua_pushnumber(L, 0);
			return 0;
		}

		LPDUNGEON pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		struct FPartyPIDCollector
		{
			std::vector<uint32_t> vecPIDs;
			FPartyPIDCollector() noexcept {}
			void operator () (LPCHARACTER ch)
			{
				if (ch)
					vecPIDs.emplace_back(ch->GetPlayerID());
			}
		};
		FPartyPIDCollector f;
		ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());

		for (std::vector<uint32_t>::iterator pid = f.vecPIDs.begin(); pid != f.vecPIDs.end(); pid++)
		{
			const auto pPC = CHARACTER_MANAGER::Instance().FindByPID(*pid);
			if (!pPC)
				continue;

			pDungeon->RegisterParticipant(pPC);
		}
		return 0;
	}

	// Check if is Registered
	ALUA(dungeon_is_registered)
	{
		// Check Char pointer
		const auto& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

#ifdef ENABLE_GM_BLOCK
		// If is GM always return true
		if (pChar->IsGM() && pChar->GetStaffPermissions())
		{
			lua_pushboolean(L, 1);
			return 1;
		}
#endif

		// Check Dungeon Pointer
		auto pDungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!pDungeon)
		{
			const auto lMapIndex = static_cast<long>(lua_tonumber(L, 1));
			if (lMapIndex)
				pDungeon = CDungeonManager::Instance().FindByMapIndex(lMapIndex);

			if (!pDungeon)
			{
				lua_pushboolean(L, 0);
				return 1;
			}
		}

		// Check if is registered
		if (pDungeon->IsParticipantRegistered(pChar->GetPlayerID()))
		{
			lua_pushboolean(L, 1);
			return 1;
		}

		lua_pushboolean(L, 0);
		return 1;
	}
#endif

#ifdef ENABLE_DEFENSE_WAVE
	ALUA(dungeon_set_victim_mast)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
			return 0;

		const auto dwVID = static_cast<uint32_t>(lua_tonumber(L, 1));
		if (dwVID == 0)
		{
			sys_err("Trying to set dwVID, but dwVID not exist");
			return 0;
		}

		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(dwVID);
		if (!ch)
			return 0;

		if (!pDungeon->GetMast())
		{
			sys_err("Trying to set victim mast, but mast not exist");
			return 0;
		}

		if (static_cast<int>(lua_tonumber(L, 2)) == 1)
			ch->SetVictim(pDungeon->GetMast());
		else
			ch->SetVictim(nullptr);

		return 0;
	}

	ALUA(dungeon_get_mast)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (!pDungeon)
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		if (!pDungeon->GetMast())
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		lua_pushboolean(L, pDungeon->GetMast()->IsDead() ? FALSE : TRUE);
		return 1;
	}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	ALUA(dungeon_battlepass_update_progress)
	{
		CQuestManager& q = CQuestManager::instance();

		FDungeonUpdateAllBattlepassProcess f;

		if (!lua_isnumber(L, 1)) {
			sys_err("arg1 must be number (dungeon_index)");
			return 0;
		}

		f.dungeon_index = (int)lua_tonumber(L, 1);

		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ForEachMember(f);

		return 0;
	}
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
	ALUA(dungeon_spawn_mob_dir_nomove)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			const uint32_t dwVnum = (uint32_t)lua_tonumber(L, 1);
			const long x = (long)lua_tonumber(L, 2);
			const long y = (long)lua_tonumber(L, 3);
			const uint8_t dir = (int)lua_tonumber(L, 4);

			LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, x, y, dir, true);
			if (ch && !vid)
				vid = ch->GetVID();
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_set_unique_master)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			pDungeon->SetUniqueMaster(lua_tostring(L, 1));
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_clear_dungeon_flags)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
		{
			pDungeon->ClearDungeonFlags();
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(dungeon_set_dungeon_difficulty)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->SetDungeonDifficulty((uint8_t)lua_tonumber(L, 1));

		lua_pushnumber(L, 0);
		return 1;
	}
#endif

#ifdef ENABLE_12ZI
	ALUA(dungeon_zodiac_clear)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ZodiacMessageClear();

		return 0;
	}

	ALUA(dungeon_zodiac_notice)
	{
		if (!lua_isstring(L, 1))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ZodiacMessage(lua_tostring(L, 1));

		return 0;
	}

	ALUA(dungeon_zodiac_time)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
			return 0;

		const uint8_t current = (uint8_t)lua_tonumber(L, 1);
		const uint8_t next = (uint8_t)lua_tonumber(L, 2);
		const int time = (int)lua_tonumber(L, 3);

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ZodiacTime(current, next, time);

		return 0;
	}

	ALUA(dungeon_zodiac_time_clear)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		if (pDungeon)
			pDungeon->ZodiacTimeClear();

		return 0;
	}
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	ALUA(dungeon_update_ranking)
	{
		CQuestManager& q = CQuestManager::Instance();
		const std::string c_szQuestName = q.GetCurrentPC()->GetCurrentQuestName();

		LPDUNGEON pDungeon = q.GetCurrentDungeon();
		if (pDungeon)
		{
			FUpdateDungeonRanking f(c_szQuestName);
			pDungeon->ForEachMember(f);
		}

		return 0;
	}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	ALUA(dungeon_set_start_time)
	{
		if (!lua_isnumber(L, 1)) {
			sys_err("Invalid syntax: d.set_start_time(number: time)");
			return 0;
		}

		auto dungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!dungeon) {
			sys_err("No dungeon selected.");
			return 0;
		}

		dungeon->SetStartTime(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_set_end_time)
	{
		if (!lua_isnumber(L, 1)) {
			sys_err("Invalid syntax: d.set_end_time(number: time)");
			return 0;
		}

		auto dungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!dungeon) {
			sys_err("No dungeon selected.");
			return 0;
		}

		dungeon->SetEndTime(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_set_dungeon_id)
	{
		if (!lua_isnumber(L, 1)) {
			sys_err("Invalid syntax: d.set_dungeon_id(number: id)");
			return 0;
		}

		auto dungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!dungeon) {
			sys_err("No dungeon selected.");
			return 0;
		}

		dungeon->SetDungeonID(static_cast<uint32_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(dungeon_get_finish_time)
	{
		auto dungeon = CQuestManager::Instance().GetCurrentDungeon();
		if (!dungeon) {
			sys_err("No dungeon selected.");
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, dungeon->GetFinishTime());
		return 1;
	}
#endif

#ifdef ENABLE_DUNGEON_RENEWAL
	ALUA(dungeon_spawn_mob_healer)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPDUNGEON pDungeon = q.GetCurrentDungeon();

		uint32_t vid = 0;

		if (pDungeon)
		{
			const uint32_t dwVnum = (uint32_t) lua_tonumber(L, 1);
			const long x = (long) lua_tonumber(L, 2);
			const long y = (long) lua_tonumber(L, 3);
			const uint32_t dwVnum2 = (uint32_t)lua_tonumber(L, 4);
			const float radius = lua_isnumber(L, 5) ? (float) lua_tonumber(L, 5) : 0;
			uint32_t count = (lua_isnumber(L, 6)) ? (uint32_t) lua_tonumber(L, 6) : 1;

			sys_log(0, "dungeon_spawn_mob %u %d %d", dwVnum, x, y);

			if (count == 0)
				count = 1;

			while (count --)
			{
				if (radius<1)
				{
					LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, x, y);
					pDungeon->SpawnMob(dwVnum2, x, y);

					if (ch && !vid)
						vid = ch->GetVID();
				}
				else
				{
					const float angle = number(0, 999) * M_PI * 2 / 1000;
					const float r = number(0, 999) * radius / 1000;

					const long nx = x + (long)(r * cos(angle));
					const long ny = y + (long)(r * sin(angle));

					LPCHARACTER ch = pDungeon->SpawnMob(dwVnum, nx, ny);
					pDungeon->SpawnMob(dwVnum2, nx, ny);

					if (ch && !vid)
						vid = ch->GetVID();
				}
			}
		}

		lua_pushnumber(L, vid);
		return 1;
	}

	ALUA(dungeon_set_dead_warp_location)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("wrong argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		ch->WarpSet((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2), ch->GetMapIndex());
		return 0;
	}

	ALUA(dungeon_set_restart_city_location)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		ch->WarpSet((int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2), (int)lua_tonumber(L, 3));
		return 0;
	}

	ALUA(dungeon_in_dungeon)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		if (!ch->GetDungeon())
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		if (lua_isnumber(L, 1))
		{
			const int iMapIndex = static_cast<int>(lua_tonumber(L, 1));
			const bool bMapIndexIsDungeon = ((ch->GetMapIndex() >= iMapIndex * 10000) && (ch->GetMapIndex() < (iMapIndex + 1) * 10000));
			lua_pushboolean(L, bMapIndexIsDungeon);
		}
		else
			lua_pushboolean(L, ch->GetDungeon() ? TRUE : FALSE);

		return 1;
	}
#endif

	ALUA(dungeon_give_item_all)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("Invalid Argument");
			return 0;
		}

		FDungeonGiveItemAll f;
		f.vnum = (uint32_t)lua_tonumber(L, 1);
		f.count = (int)lua_tonumber(L, 2);

		LPDUNGEON pDungeon = CQuestManager::instance().GetCurrentDungeon();
		if (pDungeon)
			pDungeon->ForEachMember(f);

		return 0;
	}

	void RegisterDungeonFunctionTable()
	{
		luaL_reg dungeon_functions[] =
		{
			{ "join", dungeon_join },
			{ "exit", dungeon_exit },
			{ "exit_all", dungeon_exit_all },
			{ "set_item_group", dungeon_set_item_group },
			{ "exit_all_by_item_group", dungeon_exit_all_by_item_group},
			{ "say_diff_by_item_group", dungeon_say_diff_by_item_group},
			{ "delete_item_in_item_group_from_all", dungeon_delete_item_in_item_group_from_all},
			{ "purge", dungeon_purge },
			{ "kill_all", dungeon_kill_all },
			{ "spawn", dungeon_spawn },
			{ "spawn_mob", dungeon_spawn_mob },
			{ "spawn_mob_dir", dungeon_spawn_mob_dir },
			{ "spawn_mob_ac_dir", dungeon_spawn_mob_ac_dir },
			{ "spawn_name_mob", dungeon_spawn_name_mob },
			{ "spawn_goto_mob", dungeon_spawn_goto_mob },
			{ "spawn_group", dungeon_spawn_group },
			{ "spawn_unique", dungeon_spawn_unique },
			{ "spawn_move_unique", dungeon_spawn_move_unique},
			{ "spawn_move_group", dungeon_spawn_move_group},
			{ "spawn_stone_door", dungeon_spawn_stone_door},
			{ "spawn_wooden_door", dungeon_spawn_wooden_door},
			{ "purge_unique", dungeon_purge_unique },
			{ "purge_area", dungeon_purge_area },
			{ "kill_unique", dungeon_kill_unique },
#ifdef ENABLE_NEWSTUFF
			{ "kill_area", dungeon_kill_area },
#endif
			{ "is_unique_dead", dungeon_is_unique_dead },
			{ "unique_get_hp_perc", dungeon_unique_get_hp_perc},
			{ "unique_set_def_grade", dungeon_unique_set_def_grade},
			{ "unique_set_hp", dungeon_unique_set_hp },
			{ "unique_set_maxhp", dungeon_unique_set_maxhp},
			{ "get_unique_vid", dungeon_get_unique_vid},
			{ "get_kill_stone_count", dungeon_get_kill_stone_count},
			{ "get_kill_mob_count", dungeon_get_kill_mob_count},
			{ "is_use_potion", dungeon_is_use_potion },
			{ "revived", dungeon_revived },
			{ "set_dest", dungeon_set_dest },
			{ "jump_all", dungeon_jump_all },
			{ "warp_all", dungeon_warp_all },
			{ "new_jump_all", dungeon_new_jump_all },
#ifdef ENABLE_D_NJGUILD
			// d.new_jump_guild(map_index, x, y)
			{ "new_jump_all_guild", dungeon_new_jump_guild }, // [return nothing]
			{ "new_jump_guild", dungeon_new_jump_guild }, // [return nothing]
#endif
			{ "new_jump_party", dungeon_new_jump_party },
			{ "new_jump", dungeon_new_jump },
			{ "regen_file", dungeon_regen_file },
			{ "set_regen_file", dungeon_set_regen_file },
			{ "clear_regen", dungeon_clear_regen },
			{ "set_exit_all_at_eliminate", dungeon_set_exit_all_at_eliminate},
			{ "set_warp_at_eliminate", dungeon_set_warp_at_eliminate},
			{ "get_map_index", dungeon_get_map_index },
			{ "check_eliminated", dungeon_check_eliminated},
			{ "exit_all_to_start_position", dungeon_exit_all_to_start_position },
			{ "count_monster", dungeon_count_monster },
			{ "setf", dungeon_set_flag },
			{ "getf", dungeon_get_flag },
			{ "getf_from_map_index", dungeon_get_flag_from_map_index },
			{ "set_unique", dungeon_set_unique },
#ifdef NEW_ICEDAMAGE_SYSTEM
			{ "get_damage_from_race", dungeon_get_damage_from_race }, // [return lua number]
			{ "get_damage_from_affect", dungeon_get_damage_from_affect }, // [return lua table]
			{ "set_damage_from_race", dungeon_set_damage_from_race }, // [return nothing]
			{ "set_damage_from_affect", dungeon_set_damage_from_affect }, // [return nothing]
			{ "unset_damage_from_race", dungeon_unset_damage_from_race }, // [return nothing]
			{ "unset_damage_from_affect", dungeon_unset_damage_from_affect}, // [return nothing]
			{ "reset_damage_from_race", dungeon_reset_damage_from_race }, // [return nothing]
			{ "reset_damage_from_affect", dungeon_reset_damage_from_affect}, // [return nothing]
#endif
#ifdef ENABLE_NEWSTUFF
			{ "is_available0", dungeon_is_available0 }, // [return lua boolean]
#endif
			{ "select", dungeon_select },
			{ "find", dungeon_find },
			{ "notice", dungeon_notice },
			{ "setqf", dungeon_set_quest_flag },
			{ "all_near_to", dungeon_all_near_to },
			{ "set_warp_location", dungeon_set_warp_location },
			{ "setqf2", dungeon_set_quest_flag2 },
#ifdef ENABLE_NEW_CHAT_DUNGEON
			{ "syschat", dungeon_syschat },
			{ "bignotice", dungeon_bignotice },
#endif
#ifdef ENABLE_NEWSTUFF
			{ "kill_monsters", dungeon_kill_monsters },
#endif
#ifdef ENABLE_NEWSTUFF	//ENABLE_ORC_MAZE
			{ "spawn_goto_mob0", dungeon_spawn_goto_mob0 },
#endif
#ifdef ENABLE_DUNGEON_DEAD_TIME_RENEWAL
			{ "set_dead_time", dungeon_set_dead_time },
#endif
#ifdef ENABLE_NEWSTUFF //LEVEL_RANGE_EXTEND
			{"spawn_group_level_range", dungeon_spawn_group_level_range},
			{"spawn_mob_level_range", dungeon_spawn_mob_level_range},
#endif
#ifdef ENABLE_DUNGEON_RENEWAL
			{"clear_participants", dungeon_clear_register},
			{"register_participant", dungeon_register_participant},
			{"register_party_participants", dungeon_register_party_participants},
			{"is_registered", dungeon_is_registered},
#endif
#ifdef ENABLE_DEFENSE_WAVE
			{ "get_mast", dungeon_get_mast },
			{ "set_victim_mast", dungeon_set_victim_mast },
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
			{ "update_ranking", dungeon_update_ranking },
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
			{ "set_dungeon_id", dungeon_set_dungeon_id},
			{ "set_start_time", dungeon_set_start_time },
			{ "set_end_time", dungeon_set_end_time },
			{ "get_finish_time", dungeon_get_finish_time },
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
			{ "battlepass_update_progress", dungeon_battlepass_update_progress },
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			{ "spawn_mob_dir_nomove",	dungeon_spawn_mob_dir_nomove },
			{ "set_unique_master",		dungeon_set_unique_master },
			{ "clear_dungeon_flags",	dungeon_clear_dungeon_flags },
			{ "set_dungeon_difficulty",	dungeon_set_dungeon_difficulty },
#endif
#ifdef ENABLE_12ZI
			{ "zodiac_notice_clear",	dungeon_zodiac_clear },
			{ "zodiac_notice",			dungeon_zodiac_notice },

			{ "zodiac_time",			dungeon_zodiac_time },
			{ "zodiac_time_clear",		dungeon_zodiac_time_clear },
#endif
#ifdef ENABLE_DUNGEON_RENEWAL
			{ "spawn_mob_healer",			dungeon_spawn_mob_healer },
			{ "set_dead_warp_location",		dungeon_set_dead_warp_location },
			{ "set_restart_city_location",	dungeon_set_restart_city_location },
			{ "in_dungeon",	dungeon_in_dungeon },
			{ "spawn_goto_mob0",		dungeon_spawn_goto_mob },
#endif
			{ "give_item_all",        dungeon_give_item_all },

			{ nullptr, nullptr }
		};

		CQuestManager::Instance().AddLuaFunctionTable("d", dungeon_functions);
	}
}
