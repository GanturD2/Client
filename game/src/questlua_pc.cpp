#include "stdafx.h"

#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "item.h"
#include "item_manager.h"
#include "guild_manager.h"
#include "war_map.h"
#include "start_position.h"
#include "marriage.h"
#include "mining.h"
#include "p2p.h"
#include "polymorph.h"
#include "desc_client.h"
#include "messenger_manager.h"
#include "log.h"
#include "utils.h"
#include "unique_item.h"
#include "mob_manager.h"
#include "battle.h"
#include "party.h"
#ifdef ENABLE_NEWSTUFF
# include "pvp.h"
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
# include "DungeonInfo.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
# include "AchievementSystem.h"
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
# include "minigame_manager.h"
#endif

#include <cctype>
#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
extern bool IS_MOUNTABLE_ZONE(int map_index);	//@custom005
#endif
const int ITEM_BROKEN_METIN_VNUM = 28960;

#ifdef ENABLE_PC_OPENSHOP
#include "shop.h"
#include "shop_manager.h"
#endif

namespace quest
{
	//
	// "pc" Lua functions
	//
	ALUA(pc_has_master_skill)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		bool bHasMasterSkill = false;
		for (int i = 0; i < SKILL_MAX_NUM; i++)
		{
			if (ch->GetSkillMasterType(i) >= SKILL_MASTER && ch->GetSkillLevel(i) >= 21)
			{
				bHasMasterSkill = true;
				break;
			}
		}

		lua_pushboolean(L, bHasMasterSkill);
		return 1;
	}

	ALUA(pc_remove_skill_book_no_delay)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
		return 0;
	}

	ALUA(pc_is_skill_book_no_delay)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch && ch->FindAffect(AFFECT_SKILL_NO_BOOK_DELAY) ? true : false);
		return 1;
	}

	ALUA(pc_learn_grand_master_skill)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong skill index");
			return 0;
		}

		lua_pushboolean(L, ch->LearnGrandMasterSkill(static_cast<long>(lua_tonumber(L, 1))));
		return 1;
	}

	ALUA(pc_set_warp_location)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong coodinate");
			return 0;
		}

		ch->SetWarpLocation(static_cast<long>(lua_tonumber(L, 1)), static_cast<long>(lua_tonumber(L, 2)), static_cast<long>(lua_tonumber(L, 3)));
		return 0;
	}

	ALUA(pc_set_warp_location_local)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong map index");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("wrong coodinate");
			return 0;
		}

		const long lMapIndex = static_cast<long>(lua_tonumber(L, 1));
		const TMapRegion* region = SECTREE_MANAGER::Instance().GetMapRegion(lMapIndex);

		if (!region)
		{
			sys_err("invalid map index %d", lMapIndex);
			return 0;
		}

		const int x = static_cast<int>(lua_tonumber(L, 2));
		const int y = static_cast<int>(lua_tonumber(L, 3));

		if (x > region->ex - region->sx)
		{
			sys_err("x coordinate overflow max: %d input: %d", region->ex - region->sx, x);
			return 0;
		}

		if (y > region->ey - region->sy)
		{
			sys_err("y coordinate overflow max: %d input: %d", region->ey - region->sy, y);
			return 0;
		}

		ch->SetWarpLocation(lMapIndex, x, y);
		return 0;
	}

	ALUA(pc_get_start_location)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			return 3;
		}

		lua_pushnumber(L, g_start_map[ch->GetEmpire()]);
		lua_pushnumber(L, g_start_position[ch->GetEmpire()][0] / 100);
		lua_pushnumber(L, g_start_position[ch->GetEmpire()][1] / 100);
		return 3;
	}

	ALUA(pc_warp)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		long map_index = 0;

		if (lua_isnumber(L, 3))
			map_index = static_cast<int>(lua_tonumber(L, 3));

		//PREVENT_HACK
		if (ch->IsHack())
		{
			lua_pushboolean(L, false);
			return 1;
		}
		//END_PREVENT_HACK

		if (test_server)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "pc_warp %d %d %d", static_cast<int>(lua_tonumber(L, 1)),
				static_cast<int>(lua_tonumber(L, 2)), map_index);
		}

		ch->WarpSet(static_cast<int>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)), map_index);

		lua_pushboolean(L, true);

		return 1;
	}

	ALUA(pc_warp_local)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("no map index argument");
			return 0;
		}

		if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("no coodinate argument");
			return 0;
		}

		const long lMapIndex = static_cast<long>(lua_tonumber(L, 1));
		const TMapRegion* region = SECTREE_MANAGER::Instance().GetMapRegion(lMapIndex);

		if (!region)
		{
			sys_err("invalid map index %d", lMapIndex);
			return 0;
		}

		const int x = static_cast<int>(lua_tonumber(L, 2));
		const int y = static_cast<int>(lua_tonumber(L, 3));

		if (x > region->ex - region->sx)
		{
			sys_err("x coordinate overflow max: %d input: %d", region->ex - region->sx, x);
			return 0;
		}

		if (y > region->ey - region->sy)
		{
			sys_err("y coordinate overflow max: %d input: %d", region->ey - region->sy, y);
			return 0;
		}

		/*
		int iPulse = thecore_pulse();
		if (pkChr->GetOpenedWindow(W_EXCHANGE | W_MYSHOP | W_SHOP_OWNER | W_SAFEBOX))
		{
			pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot move to another location while the trading window or warehouse is open."));

			return;
		}
		//PREVENT_PORTAL_AFTER_EXCHANGE
		//Time check after exchange
		if (iPulse - pkChr->GetExchangeTime() < PASSES_PER_SEC(60))
		{
			pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot move to another area within 1 minute after the transaction."));
			return;
		}
		//END_PREVENT_PORTAL_AFTER_EXCHANGE
		//PREVENT_ITEM_COPY
		{
			if (iPulse - pkChr->GetMyShopTime() < PASSES_PER_SEC(60))
			{
				pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot move to another area within 1 minute after the transaction."));
				return;
			}
		}
		//END_PREVENT_ITEM_COPY
		*/

		CQuestManager::Instance().GetCurrentCharacterPtr()->WarpSet(region->sx + x, region->sy + y);
		return 0;
	}

	ALUA(pc_warp_exit)
	{
		CQuestManager::Instance().GetCurrentCharacterPtr()->ExitToSavedLocation();
		return 0;
	}

	ALUA(pc_in_dungeon)
	{
		/*Rewrite function, now you're able to use argument. Example:
		* 1. pc.in_dungeon(217) --> Returns true if you're in SpiderDungeon
		* 2. pc.in_dungeon() --> Returns true if you're in whatever dungeon. Warning: Not recommended for Meley / Zodiac
		*/
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

	ALUA(pc_hasguild)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch && ch->GetGuild() ? 1 : 0);
		return 1;
	}

	ALUA(pc_getguild)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch && ch->GetGuild() ? ch->GetGuild()->GetID() : 0);
		return 1;
	}

	ALUA(pc_isguildmaster)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const CGuild* g = ch->GetGuild();

		if (g)
			lua_pushboolean(L, (ch->GetPlayerID() == g->GetMasterPID()));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(pc_destroy_guild)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		CGuild* g = ch->GetGuild();
		if (g)
			g->RequestDisband(ch->GetPlayerID());

		return 0;
	}

	ALUA(pc_remove_from_guild)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		CGuild* g = ch->GetGuild();
		if (g)
			g->RequestRemoveMember(ch->GetPlayerID());

		return 0;
	}

	ALUA(pc_give_gold)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		const int iAmount = static_cast<int>(lua_tonumber(L, 1));

		if (iAmount <= 0)
		{
			sys_err("QUEST : gold amount less then zero");
			return 0;
		}

		DBManager::Instance().SendMoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), iAmount);
		ch->PointChange(POINT_GOLD, iAmount, true);
		return 0;
	}

	ALUA(pc_warp_to_guild_war_observer_position)
	{
		luaL_checknumber(L, 1);
		luaL_checknumber(L, 2);

		const uint32_t gid1 = static_cast<uint32_t>(lua_tonumber(L, 1));
		const uint32_t gid2 = static_cast<uint32_t>(lua_tonumber(L, 2));

		CGuild* g1 = CGuildManager::Instance().FindGuild(gid1);
		const CGuild* g2 = CGuildManager::Instance().FindGuild(gid2);

		if (!g1 || !g2)
		{
			luaL_error(L, "no such guild with id %d %d", gid1, gid2);
		}

		PIXEL_POSITION pos;

		const uint32_t dwMapIndex = g1->GetGuildWarMapIndex(gid2);

		if (!CWarMapManager::Instance().GetStartPosition(dwMapIndex, 2, pos))
		{
			luaL_error(L, "not under warp guild war between guild %d %d", gid1, gid2);
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		//PREVENT_HACK
		if (ch->IsHack())
			return 0;
		//END_PREVENT_HACK

		ch->SetQuestFlag("war.is_war_member", 0);
		ch->SaveExitLocation();
		ch->WarpSet(pos.x, pos.y, dwMapIndex);
		return 0;
	}

	ALUA(pc_give_item_from_special_item_group)
	{
		luaL_checknumber(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const uint32_t dwGroupVnum = static_cast<uint32_t>(lua_tonumber(L, 1));

		std::vector <uint32_t> dwVnums;
		std::vector <uint32_t> dwCounts;
		std::vector <LPITEM> item_gets(0);
		int count = 0;

		ch->GiveItemFromSpecialItemGroup(dwGroupVnum, dwVnums, dwCounts, item_gets, count);

		for (int i = 0; i < count; i++)
		{
			if (!item_gets[i])
			{
				if (dwVnums[i] == 1)
				{
#ifdef ENABLE_CHAT_SETTINGS
					ch->ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#endif
				}
				else if (dwVnums[i] == 2)
				{
#ifdef ENABLE_CHAT_SETTINGS
					ch->ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;487]");
					ch->ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1290;%d]", dwCounts[i]);
#else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;487]");
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1290;%d]", dwCounts[i]);
#endif
				}
			}
		}
		return 0;
	}

	ALUA(pc_enough_inventory)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const uint32_t item_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
#ifdef ENABLE_SPECIAL_INVENTORY
		LPITEM item = ITEM_MANAGER::Instance().CreateItem(item_vnum);
		if (!item)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const bool bEnoughInventoryForItem = ch->GetEmptyInventory(item) != -1;
#else
		TItemTable* pTable = ITEM_MANAGER::Instance().GetTable(item_vnum);
		if (!pTable)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const bool bEnoughInventoryForItem = ch->GetEmptyInventory(pTable->bSize) != -1;
#endif
		lua_pushboolean(L, bEnoughInventoryForItem);
		return 1;
	}

	ALUA(pc_give_item)
	{
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
			return 0;

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isstring(L, 1) || !(lua_isstring(L, 2) || lua_isnumber(L, 2)))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		uint32_t dwVnum = 0;

		if (lua_isnumber(L, 2)) // If it is a number, give it by number.
			dwVnum = static_cast<int>(lua_tonumber(L, 2));
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 2), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			return 0;
		}

		int icount = 1;

		if (lua_isnumber(L, 3) && lua_tonumber(L, 3) > 0)
		{
			icount = static_cast<int>(rint(lua_tonumber(L, 3)));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				return 0;
			}
		}

		pPC->GiveItem(lua_tostring(L, 1), dwVnum, icount);

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);
		return 0;
	}

	ALUA(pc_give_or_drop_item)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t dwVnum = 0;

		if (lua_isnumber(L, 1)) // If it is a number, give it by number.
		{
			dwVnum = static_cast<int>(lua_tonumber(L, 1));
		}
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			lua_pushnumber(L, 0);

			return 1;
		}

		uint8_t icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0)
		{
			icount = static_cast<int>(rint(lua_tonumber(L, 2)));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				lua_pushnumber(L, 0);
				return 1;
			}
		}

		sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());

		const PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);

		LPITEM item = ch->AutoGiveItem(dwVnum, icount);
		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

#ifdef ENABLE_12ZI
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ìƒìžì—ì„œ %s ê°€ %d ê°œ ë‚˜ì™”ìŠµë‹ˆë‹¤."), item->GetName(), icount);
#endif

	//	if ( dwVnum >= 80003 && dwVnum <= 80007 )
		if ((dwVnum >= 80003 && dwVnum <= 80007) || (dwVnum >= 80018 && dwVnum <= 80020))	// Rubinum
		{
			LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), QUEST, "quest: give_item2");
		}

		if (nullptr != item)
			lua_pushnumber(L, item->GetID());
		else
			lua_pushnumber(L, 0);
		return 1;
	}

#ifdef ENABLE_DICE_SYSTEM
	ALUA(pc_give_or_drop_item_with_dice)
	{
		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		uint32_t dwVnum = 0;

		if (lua_isnumber(L, 1)) // If it is a number, give it by number.
		{
			dwVnum = static_cast<int>(lua_tonumber(L, 1));
		}
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			lua_pushnumber(L, 0);

			return 1;
		}

		int icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0)
		{
			icount = static_cast<int>(rint(lua_tonumber(L, 2)));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				lua_pushnumber(L, 0);
				return 1;
			}
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(dwVnum, icount);
		if (ch->GetParty())
		{
			FPartyDropDiceRoll f(item, ch);
			f.Process(nullptr);
			f.GetItemOwner()->AutoGiveItem(item);
		}
		else
			ch->AutoGiveItem(item);


		sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());

		LogManager::Instance().QuestRewardLog(CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);

		lua_pushnumber(L, (item) ? item->GetID() : 0);
		return 1;
	}
#endif

	ALUA(pc_give_or_drop_item_and_select)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isstring(L, 1) && !lua_isnumber(L, 1))
		{
			sys_err("QUEST Make item call error : wrong argument");
			return 0;
		}

		uint32_t dwVnum = 0;

		if (lua_isnumber(L, 1))
		{
			dwVnum = static_cast<int>(lua_tonumber(L, 1));
		}
		else if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), dwVnum))
		{
			sys_err("QUEST Make item call error : wrong item name : %s", lua_tostring(L, 1));
			return 0;
		}

		uint8_t icount = 1;
		if (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 0)
		{
			icount = static_cast<int>(rint(lua_tonumber(L, 2)));
			if (icount <= 0)
			{
				sys_err("QUEST Make item call error : wrong item count : %g", lua_tonumber(L, 2));
				return 0;
			}
		}

		sys_log(0, "QUEST [REWARD] item %s to %s", lua_tostring(L, 1), ch->GetName());

		const PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
			return 0;

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), dwVnum, icount);

		LPITEM item = ch->AutoGiveItem(dwVnum, icount);

		if (nullptr != item)
			CQuestManager::Instance().SetCurrentItem(item);

		//if ( dwVnum >= 80003 && dwVnum <= 80007 )
		if ((dwVnum >= 80003 && dwVnum <= 80007) || (dwVnum >= 80018 && dwVnum <= 80020))	// Rubinum
		{
			LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), QUEST, "quest: give_item2");
		}

		return 0;
	}

	ALUA(pc_get_current_map_index)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetMapIndex());
		return 1;
	}

	ALUA(pc_get_x)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, ch->GetX() / 100.0);
		return 1;
	}

	ALUA(pc_get_y)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, ch->GetY() / 100.0);
		return 1;
	}

	ALUA(pc_get_local_x)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (pMap)
			lua_pushnumber(L, (ch->GetX() - pMap->m_setting.iBaseX) / 100.0);
		else
			lua_pushnumber(L, ch->GetX() / 100.0);

		return 1;
	}

	ALUA(pc_get_local_y)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (pMap)
			lua_pushnumber(L, (ch->GetY() - pMap->m_setting.iBaseY) / 100.0);
		else
			lua_pushnumber(L, ch->GetY() / 100.0);

		return 1;
	}

	ALUA(pc_count_item)
	{
		if (lua_isnumber(L, -1))
			lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->CountSpecifyItem(static_cast<uint32_t>(lua_tonumber(L, -1))));
		else if (lua_isstring(L, -1))
		{
			uint32_t item_vnum;

			if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), item_vnum))
			{
				sys_err("QUEST count_item call error : wrong item name : %s", lua_tostring(L, 1));
				lua_pushnumber(L, 0);
			}
			else
			{
				lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->CountSpecifyItem(item_vnum));
			}
		}
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_remove_item)
	{
		if (lua_gettop(L) == 1)
		{
			uint32_t item_vnum = 0;

			if (lua_isnumber(L, 1))
			{
				item_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			}
			else if (lua_isstring(L, 1))
			{
				if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), item_vnum))
				{
					sys_err("QUEST remove_item call error : wrong item name : %s", lua_tostring(L, 1));
					return 0;
				}
			}
			else
			{
				sys_err("QUEST remove_item wrong argument");
				return 0;
			}

			sys_log(0, "QUEST remove a item vnum %d of %s[%d]", item_vnum, CQuestManager::Instance().GetCurrentCharacterPtr()->GetName(), CQuestManager::Instance().GetCurrentCharacterPtr()->GetPlayerID());
			CQuestManager::Instance().GetCurrentCharacterPtr()->RemoveSpecifyItem(item_vnum);
		}
		else if (lua_gettop(L) == 2)
		{
			uint32_t item_vnum = 0;

			if (lua_isnumber(L, 1))
			{
				item_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
			}
			else if (lua_isstring(L, 1))
			{
				if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, 1), item_vnum))
				{
					sys_err("QUEST remove_item call error : wrong item name : %s", lua_tostring(L, 1));
					return 0;
				}
			}
			else
			{
				sys_err("QUEST remove_item wrong argument");
				return 0;
			}

			const uint32_t item_count = static_cast<uint32_t>(lua_tonumber(L, 2));
			sys_log(0, "QUEST remove items(vnum %d) count %d of %s[%d]",
				item_vnum,
				item_count,
				CQuestManager::Instance().GetCurrentCharacterPtr()->GetName(),
				CQuestManager::Instance().GetCurrentCharacterPtr()->GetPlayerID());

			CQuestManager::Instance().GetCurrentCharacterPtr()->RemoveSpecifyItem(item_vnum, item_count);
		}
		return 0;
	}

	ALUA(pc_get_leadership)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetLeadershipSkillLevel());
		return 1;
	}

	ALUA(pc_reset_point)
	{
		CQuestManager::Instance().GetCurrentCharacterPtr()->ResetPoint(CQuestManager::Instance().GetCurrentCharacterPtr()->GetLevel());
		return 0;
	}

	ALUA(pc_get_playtime)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetRealPoint(POINT_PLAYTIME));
		return 1;
	}

	ALUA(pc_get_vid)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetVID());
		return 1;
	}
	ALUA(pc_get_name)
	{
		lua_pushstring(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetName());
		return 1;
	}

	ALUA(pc_get_next_exp)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetNextExp());
		return 1;
	}

	ALUA(pc_get_exp)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetExp());
		return 1;
	}

	ALUA(pc_get_race)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetRaceNum());
		return 1;
	}

	ALUA(pc_change_sex)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->ChangeSex());
		return 1;
	}

	ALUA(pc_get_job)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetJob());
		return 1;
	}

	ALUA(pc_get_max_sp)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetMaxSP());
		return 1;
	}

	ALUA(pc_get_sp)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetSP());
		return 1;
	}

	ALUA(pc_change_sp)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushboolean(L, 0);
			return 1;
		}

		const long val = static_cast<long>(lua_tonumber(L, 1));
		if (val == 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (val > 0) // Incremental, so unconditionally return success
			ch->PointChange(POINT_SP, val);
		else if (val < 0)
		{
			if (ch->GetSP() < -val)
			{
				lua_pushboolean(L, 0);
				return 1;
			}

			ch->PointChange(POINT_SP, val);
		}

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(pc_get_max_hp)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetMaxHP());
		return 1;
	}

	ALUA(pc_get_hp)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetHP());
		return 1;
	}

	ALUA(pc_get_level)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetLevel());
		return 1;
	}

	ALUA(pc_set_level)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}
		else
		{
			const int newLevel = static_cast<int>(lua_tonumber(L, 1));
			LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
			if (!ch)
				return 0;

			sys_log(0, "QUEST [LEVEL] %s jumpint to level %d", ch->GetName(), static_cast<int>(rint(lua_tonumber(L, 1))));

			const PC* pPC = CQuestManager::Instance().GetCurrentPC();
			if (!pPC)
				return 0;

			LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), newLevel, 0);

			//Points: Skill, subskill, stats
			ch->PointChange(POINT_SKILL, newLevel - ch->GetLevel());
			ch->PointChange(POINT_SUB_SKILL, newLevel < 10 ? 0 : newLevel - MAX(ch->GetLevel(), 9));
			ch->PointChange(POINT_STAT, ((MINMAX(1, newLevel, gPlayerMaxLevel) - ch->GetLevel()) * 3) + ch->GetPoint(POINT_LEVEL_STEP)); // @fixme004
			//level
			ch->PointChange(POINT_LEVEL, newLevel - ch->GetLevel());
			//HP, SP
			ch->SetRandomHP((newLevel - 1) * number(JobInitialPoints[ch->GetJob()].hp_per_lv_begin, JobInitialPoints[ch->GetJob()].hp_per_lv_end));
			ch->SetRandomSP((newLevel - 1) * number(JobInitialPoints[ch->GetJob()].sp_per_lv_begin, JobInitialPoints[ch->GetJob()].sp_per_lv_end));


			// recovery
			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

			ch->ComputePoints();
			ch->PointsPacket();
			ch->SkillLevelPacket();

			return 0;
		}
	}

	ALUA(pc_get_weapon)
	{
		const LPITEM& item = CQuestManager::Instance().GetCurrentCharacterPtr()->GetWear(WEAR_WEAPON);
		lua_pushnumber(L, item ? item->GetVnum() : 0);
		return 1;
	}

	ALUA(pc_get_armor)
	{
		const LPITEM& item = CQuestManager::Instance().GetCurrentCharacterPtr()->GetWear(WEAR_BODY);
		lua_pushnumber(L, item ? item->GetVnum() : 0);
		return 1;
	}

	ALUA(pc_get_wear)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}

		const uint8_t bCell = static_cast<uint8_t>(lua_tonumber(L, 1));
		const LPITEM& item = CQuestManager::Instance().GetCurrentCharacterPtr()->GetWear(bCell);

		if (!item)
			lua_pushnil(L);
		else
			lua_pushnumber(L, item->GetVnum());

		return 1;
	}

	ALUA(pc_get_money)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetGold());
		return 1;
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	ALUA(pc_get_cheque)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetCheque() : 0);
		return 1;
	}

	ALUA(pc_set_cheque)
	{
		const uint32_t cheque = static_cast<uint32_t>(lua_tonumber(L, -1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (cheque + ch->GetCheque() < 0)
			sys_err("QUEST wrong GetCheque %d (now %d)", cheque, ch->GetCheque());
		else
		{
			//DBManager::Instance().SendMoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), cheque);
			ch->PointChange(POINT_CHEQUE, cheque, true);

			// Log
			char buf[512];
			snprintf(buf, sizeof(buf), "%d", cheque);
			LogManager::Instance().CharLog(ch, 0, "CHANGE_CHEQUE", buf);
		}

		return 0;
	}
#endif

	// 20050725.myevan. While using hermit's cloak, a bug that doubled the amount of good and bad during mixed stone training occurred.
	// Make the calculations using the actual good and bad values.
	ALUA(pc_get_real_alignment)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetRealAlignment() / static_cast<lua_Number>(10));
		return 1;
	}

	ALUA(pc_get_alignment)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetAlignment() / 10);
		return 1;
	}

	ALUA(pc_change_alignment)
	{
		const int alignment = static_cast<int>((lua_tonumber(L, 1) * 10));
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->UpdateAlignment(alignment);
		return 0;
	}

	ALUA(pc_change_money)
	{
		const int gold = static_cast<int>(lua_tonumber(L, -1));
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (gold + ch->GetGold() < 0)
			sys_err("QUEST wrong ChangeGold %d (now %d)", gold, ch->GetGold());
		else
		{
			DBManager::Instance().SendMoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), gold);
			ch->PointChange(POINT_GOLD, gold, true);
		}

		return 0;
	}

	ALUA(pc_set_another_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("QUEST wrong set flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			const char* sz2 = lua_tostring(L, 2);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
				return 0;
			pPC->SetFlag(string(sz) + "." + sz2, static_cast<int>(rint(lua_tonumber(L, 3))));
			return 0;
		}
	}

	ALUA(pc_get_another_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			const char* sz2 = lua_tostring(L, 2);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
				return 0;

			lua_pushnumber(L, pPC->GetFlag(string(sz) + "." + sz2));
			return 1;
		}
	}

	ALUA(pc_get_flag)
	{
		if (!lua_isstring(L, -1))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, -1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
				return 0;
			lua_pushnumber(L, pPC->GetFlag(sz));
			return 1;
		}
	}

	ALUA(pc_get_quest_flag)
	{
		if (!lua_isstring(L, -1))
		{
			sys_err("QUEST wrong get flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, -1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
				return 0;
			lua_pushnumber(L, pPC->GetFlag(pPC->GetCurrentQuestName() + "." + sz));
			if (test_server)
				sys_log(0, "GetQF ( %s . %s )", pPC->GetCurrentQuestName().c_str(), sz);
		}
		return 1;
	}

	ALUA(pc_set_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("QUEST wrong set flag");
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
				return 0;
			pPC->SetFlag(sz, static_cast<int>(rint(lua_tonumber(L, 2))));
		}
		return 0;
	}

	ALUA(pc_set_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("QUEST wrong set flag");
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			if (!pPC)
				return 0;
			pPC->SetFlag(pPC->GetCurrentQuestName() + "." + sz, static_cast<int>(rint(lua_tonumber(L, 2))));
		}
		return 0;
	}

	ALUA(pc_del_quest_flag)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("argument error");
			return 0;
		}

		const char* sz = lua_tostring(L, 1);
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
			return 0;
		pPC->DeleteFlag(pPC->GetCurrentQuestName() + "." + sz);
		return 0;
	}

	ALUA(pc_give_exp2)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
			return 0;

		sys_log(0, "QUEST [REWARD] %s give exp2 %d", ch->GetName(), static_cast<int>(rint(lua_tonumber(L, 1))));

		const uint32_t exp = static_cast<uint32_t>(rint(lua_tonumber(L, 1)));

		const PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
			return 0;

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), exp, 0);
		ch->PointChange(POINT_EXP, exp);
		return 0;
	}

	ALUA(pc_give_exp)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		const LPCHARACTER& ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		sys_log(0, "QUEST [REWARD] %s give exp %s %d", ch->GetName(), lua_tostring(L, 1), (int)rint(lua_tonumber(L, 2)));

		const uint32_t exp = static_cast<uint32_t>(rint(lua_tonumber(L, 2)));

		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
			return 0;

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), exp, 0);

		pPC->GiveExp(lua_tostring(L, 1), exp);
		return 0;
	}

	ALUA(pc_give_exp_perc)
	{
		CQuestManager& q = CQuestManager::Instance();
		const LPCHARACTER& ch = q.GetCurrentCharacterPtr();

		if (!ch || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
			return 0;

		const int lev = static_cast<int>(rint(lua_tonumber(L, 2)));
		const double proc = (lua_tonumber(L, 3));

		sys_log(0, "QUEST [REWARD] %s give exp %s lev %d percent %g%%", ch->GetName(), lua_tostring(L, 1), lev, proc);

		const uint32_t exp = static_cast<uint32_t>((exp_table[MINMAX(0, lev, PLAYER_MAX_LEVEL_CONST)] * proc) / 100);
		PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
			return 0;

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), exp, 0);

		pPC->GiveExp(lua_tostring(L, 1), exp);
		return 0;
	}

	ALUA(pc_get_exp_perc)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		int lev = (int)rint(lua_tonumber(L, 1));
		double proc = (lua_tonumber(L, 2));
		
		lua_pushnumber(L, (uint32_t)((exp_table[MINMAX(0, lev, PLAYER_EXP_TABLE_MAX)] * proc) / 100));
		return 1;
	}

	ALUA(pc_get_empire)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetEmpire());
		return 1;
	}

	ALUA(pc_get_part)
	{
		CQuestManager& q = CQuestManager::Instance();
		const LPCHARACTER& ch = q.GetCurrentCharacterPtr();
		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		const uint8_t part_idx = static_cast<uint8_t>(lua_tonumber(L, 1));
		lua_pushnumber(L, ch->GetPart(part_idx));
		return 1;
	}

	ALUA(pc_set_part)
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
			return 0;

		const uint8_t part_idx = static_cast<uint8_t>(lua_tonumber(L, 1));
		const uint32_t part_value = static_cast<uint32_t>(lua_tonumber(L, 2));
		ch->SetPart(part_idx, part_value);
		ch->UpdatePacket();
		return 0;
	}

	ALUA(pc_get_skillgroup)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetSkillGroup());
		return 1;
	}

	ALUA(pc_set_skillgroup)
	{
		if (!lua_isnumber(L, 1))
			sys_err("QUEST wrong skillgroup number");
		else
		{
			CQuestManager& q = CQuestManager::Instance();
			LPCHARACTER ch = q.GetCurrentCharacterPtr();
			if (ch)
			{
				ch->RemoveGoodAffect();	//@fixme428
				ch->SetSkillGroup(static_cast<uint8_t>(rint(lua_tonumber(L, 1))));
			}
		}
		return 0;
	}

	ALUA(pc_is_polymorphed)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsPolymorphed() : false);
		return 1;
	}

	ALUA(pc_remove_polymorph)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->RemoveAffect(AFFECT_POLYMORPH);
		ch->SetPolymorph(0);
		return 0;
	}

	ALUA(pc_polymorph)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (CPolymorphUtils::Instance().IsOnPolymorphMapBlacklist(ch->GetMapIndex())) {	//@custom002
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;499]");
			return 0;
		}

		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const int iDuration = static_cast<int>(lua_tonumber(L, 2));
		ch->AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, AFF_POLYMORPH, iDuration, 0, true);
		return 0;
	}

	ALUA(pc_is_mount)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->GetMountVnum() : 0);
		return 1;
	}

	ALUA(pc_mount)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		int length = 60;

		if (lua_isnumber(L, 2))
			length = static_cast<int>(lua_tonumber(L, 2));

		const uint32_t mount_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));

		if (length < 0)
			length = 60;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
		if (!IS_MOUNTABLE_ZONE(ch->GetMapIndex()))	//@custom005
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("RIDING_IS_BLOCKED_HERE"));
			return 0;
		}
#endif

		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);
#ifdef ENABLE_NEWSTUFF
		if (g_NoMountAtGuildWar && ch->GetWarMap())
		{
			if (ch->IsRiding())
				ch->StopRiding();
			return 0;
		}
#endif

		// If the horse is summoned and followed, remove the horse from the beginning.
		if (ch->GetHorse())
			ch->HorseSummon(false);

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
		ch->AddAffect(AFFECT_MOUNT, POINT_MOUNT, mount_vnum, AFF_NONE, length, 0, true);
#else
		if (mount_vnum)
		{
			ch->AddAffect(AFFECT_MOUNT, POINT_MOUNT, mount_vnum, AFF_NONE, length, 0, true);
			switch (mount_vnum)
			{
				case 20201:
				case 20202:
				case 20203:
				case 20204:
				case 20213:
				case 20216:
					ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 30, AFF_NONE, length, 0, true, true);
				break;

				case 20205:
				case 20206:
				case 20207:
				case 20208:
				case 20214:
				case 20217:
					ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 40, AFF_NONE, length, 0, true, true);
				break;

				case 20209:
				case 20210:
				case 20211:
				case 20212:
				case 20215:
				case 20218:
					ch->AddAffect(AFFECT_MOUNT, POINT_MOV_SPEED, 50, AFF_NONE, length, 0, true, true);
				break;
			}
		}
#endif

		return 0;
	}

	ALUA(pc_mount_bonus)
	{
		const uint16_t applyOn = static_cast<uint16_t>(lua_tonumber(L, 1));	//@fixme532
		const long value = static_cast<long>(lua_tonumber(L, 2));
		const long duration = static_cast<long>(lua_tonumber(L, 3));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != ch)
		{
			// @fixme134
			if (!ch->GetMountVnum())
				return 0;
			ch->RemoveAffect(AFFECT_MOUNT_BONUS);
			ch->AddAffect(AFFECT_MOUNT_BONUS, aApplyInfo[applyOn].wPointType, value, AFF_NONE, duration, 0, false);
		}

		return 0;
	}

	ALUA(pc_unmount)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);
		if (ch->IsHorseRiding())
			ch->StopRiding();
		return 0;
	}

	ALUA(pc_get_horse_level)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetHorseLevel() : 0);
		return 1;
	}

	ALUA(pc_get_horse_hp)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch && ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseHealth());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_horse_stamina)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch && ch->GetHorseLevel())
			lua_pushnumber(L, ch->GetHorseStamina());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_is_horse_alive)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? (ch->GetHorseLevel() > 0 && ch->GetHorseHealth() > 0) : false);
		return 1;
	}

	ALUA(pc_revive_horse)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->ReviveHorse();
		return 0;
	}

	ALUA(pc_have_map_scroll)
	{
		if (!lua_isstring(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const char* szMapName = lua_tostring(L, 1);
		const TMapRegion* region = SECTREE_MANAGER::Instance().FindRegionByPartialName(szMapName);

		if (!region)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		bool bFind = false;
		for (uint16_t iCell = 0; iCell < INVENTORY_MAX_NUM; iCell++)
		{
			LPITEM item = ch->GetInventoryItem(iCell);
			if (!item)
				continue;

			if (item->GetType() == ITEM_USE &&
				item->GetSubType() == USE_TALISMAN &&
				(item->GetValue(0) == 1 || item->GetValue(0) == 2))
			{
				const int x = item->GetSocket(0);
				const int y = item->GetSocket(1);
				//if ((x-item_x)*(x-item_x)+(y-item_y)*(y-item_y)<r*r)
				if (region->sx <= x && region->sy <= y && x <= region->ex && y <= region->ey)
				{
					bFind = true;
					break;
				}
			}
		}

		lua_pushboolean(L, bFind);
		return 1;
	}

	ALUA(pc_get_war_map)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch && ch->GetWarMap() ? ch->GetWarMap()->GetMapIndex() : 0);
		return 1;
	}

	ALUA(pc_have_pos_scroll)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid x y position");
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 2))
		{
			sys_err("invalid radius");
			lua_pushboolean(L, 0);
			return 1;
		}

		const int x = static_cast<int>(lua_tonumber(L, 1));
		const int y = static_cast<int>(lua_tonumber(L, 2));
		const float r = static_cast<float>(lua_tonumber(L, 3));

		bool bFind = false;
		for (int iCell = 0; iCell < INVENTORY_MAX_NUM; iCell++)
		{
			LPITEM item = ch->GetInventoryItem(iCell);
			if (!item)
				continue;

			if (item->GetType() == ITEM_USE &&
				item->GetSubType() == USE_TALISMAN &&
				(item->GetValue(0) == 1 || item->GetValue(0) == 2))
			{
				const int item_x = item->GetSocket(0);
				const int item_y = item->GetSocket(1);
				if ((x - item_x) * (x - item_x) + (y - item_y) * (y - item_y) < r * r)
				{
					bFind = true;
					break;
				}
			}
		}

		lua_pushboolean(L, bFind);
		return 1;
	}

	ALUA(pc_get_equip_refine_level)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const int cell = static_cast<int>(lua_tonumber(L, 1));
		if (cell < 0 || cell >= WEAR_MAX_NUM)
		{
			sys_err("invalid wear position %d", cell);
			lua_pushnumber(L, 0);
			return 1;
		}

		LPITEM item = ch->GetWear(cell);
		if (!item)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, item->GetRefineLevel());
		return 1;
	}

	ALUA(pc_refine_equip)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument");
			lua_pushboolean(L, 0);
			return 1;
		}

		const int cell = static_cast<int>(lua_tonumber(L, 1));
		const int level_limit = static_cast<int>(lua_tonumber(L, 2));
		const int pct = lua_isnumber(L, 3) ? static_cast<int>(lua_tonumber(L, 3)) : 100;

		LPITEM item = ch->GetWear(cell);
		if (!item)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (item->GetRefinedVnum() == 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (item->GetRefineLevel() > level_limit)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		if (pct == 100 || number(1, 100) <= pct)
		{
			// improvement success
			lua_pushboolean(L, 1);

			LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(item->GetRefinedVnum(), 1, 0, false);

			if (pkNewItem)
			{
				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
					if (!item->GetSocket(i))
						break;
					else
						pkNewItem->SetSocket(i, 1);

				int set = 0;
				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
				{
					const long socket = item->GetSocket(i);
					if (socket > 2 && socket != 28960)
					{
						pkNewItem->SetSocket(set++, socket);
					}
				}

				item->CopyAttributeTo(pkNewItem);

				ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

				// some tuits need here -_- pkNewItem->AddToCharacter(this, bCell);
				pkNewItem->EquipTo(ch, cell);

				ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

				LogManager::Instance().ItemLog(ch, pkNewItem, "REFINE SUCCESS (QUEST)", pkNewItem->GetName());
			}
		}
		else
		{
			// improvement failure
			lua_pushboolean(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_skill_level)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		lua_pushnumber(L, ch->GetSkillLevel(dwVnum));

		return 1;
	}

	ALUA(pc_give_lotto)
	{
		CQuestManager& q = CQuestManager::Instance();
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		sys_log(0, "TRY GIVE LOTTO TO pid %u", ch->GetPlayerID());

		uint32_t* pdw = M2_NEW uint32_t[3];

		pdw[0] = 50001;
		pdw[1] = 1;
		pdw[2] = q.GetEventFlag("lotto_round");

		// lottery set socket
		DBManager::Instance().ReturnQuery(QID_LOTTO, ch->GetPlayerID(), pdw,
			"INSERT INTO lotto_list VALUES(0, 'server%s', %u, NOW())",
			get_table_postfix(), ch->GetPlayerID());

		return 0;
	}

	ALUA(pc_aggregate_monster)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		ch->AggregateMonster();
		return 0;
	}

	ALUA(pc_forget_my_attacker)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		ch->ForgetMyAttacker();
		return 0;
	}

	ALUA(pc_attract_ranger)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;
		ch->AttractRanger();
		return 0;
	}

	ALUA(pc_select_pid)
	{
		const uint32_t pid = static_cast<uint32_t>(lua_tonumber(L, 1));

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const LPCHARACTER& new_ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
		if (new_ch)
		{
			CQuestManager::Instance().GetPC(new_ch->GetPlayerID());
			lua_pushnumber(L, ch->GetPlayerID());
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_select_vid)
	{
		const uint32_t vid = static_cast<uint32_t>(lua_tonumber(L, 1));

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const LPCHARACTER& new_ch = CHARACTER_MANAGER::Instance().Find(vid);
		if (new_ch)
		{
			CQuestManager::Instance().GetPC(new_ch->GetPlayerID());
			lua_pushnumber(L, (uint32_t)ch->GetVID());
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	ALUA(pc_get_sex)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, static_cast<int>(GET_SEX(ch))); /* 0==MALE, 1==FEMALE */
		return 1;
	}

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	ALUA(pc_get_costume)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch->GetWear(WEAR_COSTUME_BODY))
			lua_pushnumber(L, true);
		else
			lua_pushboolean(L, false);
		return 1;
	}
#endif

	ALUA(pc_is_engaged)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? marriage::CManager::Instance().IsEngaged(ch->GetPlayerID()) : false);
		return 1;
	}

	ALUA(pc_is_married)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? marriage::CManager::Instance().IsMarried(ch->GetPlayerID()) : false);
		return 1;
	}

	ALUA(pc_is_engaged_or_married)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? marriage::CManager::Instance().IsEngagedOrMarried(ch->GetPlayerID()) : false);
		return 1;
	}

	ALUA(pc_is_gm)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->GetGMLevel() >= GM_HIGH_WIZARD : false);
		return 1;
	}

	ALUA(pc_get_gm_level)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetGMLevel() : 0);
		return 1;
	}

	ALUA(pc_mining)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPCHARACTER npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		ch->mining(npc);
		return 0;
	}

	ALUA(pc_diamond_refine)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const int cost = static_cast<int>(lua_tonumber(L, 1));
		const int pct = static_cast<int>(lua_tonumber(L, 2));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPCHARACTER npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		LPITEM item = CQuestManager::Instance().GetCurrentItem();

		if (item)
			lua_pushboolean(L, mining::OreRefine(ch, npc, item, cost, pct, nullptr));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(pc_ore_refine)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const int cost = static_cast<int>(lua_tonumber(L, 1));
		const int pct = static_cast<int>(lua_tonumber(L, 2));
		const int metinstone_cell = static_cast<int>(lua_tonumber(L, 3));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		LPCHARACTER npc = CQuestManager::Instance().GetCurrentNPCCharacterPtr();
		LPITEM item = CQuestManager::Instance().GetCurrentItem();

		LPITEM metinstone_item = ch->GetInventoryItem(metinstone_cell);

		if (item && metinstone_item)
			lua_pushboolean(L, mining::OreRefine(ch, npc, item, cost, pct, metinstone_item));
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(pc_clear_skill)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch == nullptr)
			return 0;

		ch->ClearSkill();
		return 0;
	}

	ALUA(pc_clear_sub_skill)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch == nullptr)
			return 0;

		ch->ClearSubSkill();
		return 0;
	}

	ALUA(pc_set_skill_point)
	{
		if (!lua_isnumber(L, 1))
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		ch->SetRealPoint(POINT_SKILL, newPoint);
		ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
		ch->PointChange(POINT_SKILL, 0);
		ch->ComputePoints();
		ch->PointsPacket();

		return 0;
	}

	// RESET_ONE_SKILL
	ALUA(pc_clear_one_skill)
	{
		const int vnum = static_cast<int>(lua_tonumber(L, 1));
		sys_log(0, "%d skill clear", vnum);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch == nullptr)
		{
			sys_log(0, "skill clear fail");
			lua_pushnumber(L, 0);
			return 1;
		}

		sys_log(0, "%d skill clear", vnum);
		ch->ResetOneSkill(vnum);
		return 0;
	}
	// END_RESET_ONE_SKILL

	ALUA(pc_is_clear_skill_group)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->GetQuestFlag("skill_group_clear.clear") == 1 : false);
		return 1;
	}

	ALUA(pc_save_exit_location)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->SaveExitLocation();

		return 0;
	}

	//Teleport
	ALUA(pc_teleport)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		int x = 0, y = 0;
		if (lua_isnumber(L, 1))
		{
			// local name warp
			constexpr int TOWN_NUM = 10;
			struct warp_by_town_name
			{
				const char* name;
				uint32_t x;
				uint32_t y;
			}

			const ws[TOWN_NUM] =
			{
				{"¿µ¾ÈÀ¾¼º", 4743, 9548},
				{"ÀÓÁö°î", 3235, 9086},
				{"ÀÚ¾çÇö", 3531, 8829},
				{"Á¶¾ÈÀ¾¼º", 638, 1664},
				{"½Â·æ°î", 1745, 1909},
				{"º¹Á¤Çö", 1455, 2400},
				{"Æò¹«À¾¼º", 9599, 2692},
				{"¹æ»ê°î", 8036, 2984},
				{"¹Ú¶óÇö", 8639, 2460},
				{"¼­ÇÑ»ê", 4350, 2143},
			};
			const int idx = static_cast<int>(lua_tonumber(L, 1));

			x = ws[idx].x;
			y = ws[idx].y;
			goto teleport_area;
		}

		else
		{
			const char* arg1 = lua_tostring(L, 1);

			const LPCHARACTER& tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

			if (!tch)
			{
				const CCI* pkCCI = P2P_MANAGER::Instance().Find(arg1);

				if (pkCCI)
				{
					if (pkCCI->bChannel != g_bChannel)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Target is in %d channel (my channel %d)"), pkCCI->bChannel, g_bChannel);
					}
					else
					{

						PIXEL_POSITION pos;

						if (!SECTREE_MANAGER::Instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot find map (index %d)"), pkCCI->lMapIndex);
						}
						else
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You warp to ( %d, %d )"), pos.x, pos.y);
							ch->WarpSet(pos.x, pos.y);
							lua_pushnumber(L, 1);
						}
					}
				}
				else if (nullptr == CHARACTER_MANAGER::Instance().FindPC(arg1))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("There is no one by that name"));
				}

				lua_pushnumber(L, 0);

				return 1;
			}
			else
			{
				x = tch->GetX() / 100;
				y = tch->GetY() / 100;
			}
		}

	teleport_area:

		x *= 100;
		y *= 100;

		ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
		ch->WarpSet(x, y);
		ch->Stop();
		lua_pushnumber(L, 1);
		return 1;
	}

	ALUA(pc_set_skill_level)
	{
		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const uint8_t byLev = static_cast<uint8_t>(lua_tonumber(L, 2));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->SetSkillLevel(dwVnum, byLev);
		ch->SkillLevelPacket();
		return 0;
	}

	ALUA(pc_give_polymorph_book)
	{
		if (!lua_isnumber(L, 1) && !lua_isnumber(L, 2) && !lua_isnumber(L, 3) && !lua_isnumber(L, 4))
		{
			sys_err("Wrong Quest Function Arguments: pc_give_polymorph_book");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		CPolymorphUtils::Instance().GiveBook(ch, static_cast<uint32_t>(lua_tonumber(L, 1)), static_cast<uint32_t>(lua_tonumber(L, 2)), static_cast<uint8_t>(lua_tonumber(L, 3)), static_cast<uint8_t>(lua_tonumber(L, 4)));

		return 0;
	}

	ALUA(pc_upgrade_polymorph_book)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPITEM pItem = CQuestManager::Instance().GetCurrentItem();

		const bool ret = CPolymorphUtils::Instance().BookUpgrade(ch, pItem);
		lua_pushboolean(L, ret);
		return 1;
	}

	ALUA(pc_get_premium_remain_sec)
	{
		int remain_seconds = 0;
		int premium_type = 0;
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong premium index (is not number)");
			return 0;
		}

		premium_type = static_cast<int>(lua_tonumber(L, 1));
		switch (premium_type)
		{
			case PREMIUM_EXP:
			case PREMIUM_ITEM:
			case PREMIUM_SAFEBOX:
			case PREMIUM_AUTOLOOT:
			case PREMIUM_FISH_MIND:
			case PREMIUM_MARRIAGE_FAST:
			case PREMIUM_GOLD:
//#ifdef ENABLE_AUTO_SYSTEM
			case PREMIUM_AUTO_USE:
//#endif
//#ifdef ENABLE_YOHARA_SYSTEM
			case PREMIUM_SUNGMA:
//#endif
//#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			case PREMIUM_PREMIUM_PRIVATE_SHOP:
//#endif
				break;

			default:
				sys_err("wrong premium index %d", premium_type);
				return 0;
		}

		remain_seconds = ch->GetPremiumRemainSeconds(premium_type);

		lua_pushnumber(L, remain_seconds);
		return 1;
	}

	ALUA(pc_send_block_mode)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->SetBlockModeForce(static_cast<uint8_t>(lua_tonumber(L, 1)));
		return 0;
	}

	ALUA(pc_change_empire)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPPARTY pParty = ch->GetParty();

		lua_pushnumber(L, ch ? ch->ChangeEmpire(static_cast<uint8_t>(lua_tonumber(L, 1))) : 0);

		if (pParty) {	//@fixme494
			pParty->Quit(ch->GetPlayerID());
			ch->ChatPacket(CHAT_TYPE_INFO, "GROUP_LEAVE_BECAUSE_OF_EMPIRE_CHANGE!");
		}

		ch->GetDesc()->DelayedDisconnect(5);	//@fixme442
		ch->ChatPacket(CHAT_TYPE_INFO, "DISCONNECT_IN_5");

		return 1;
	}

	ALUA(pc_get_change_empire_count)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		lua_pushnumber(L, ch->GetChangeEmpireCount());

		return 1;
	}

	ALUA(pc_set_change_empire_count)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		ch->SetChangeEmpireCount();

		return 0;
	}

	ALUA(pc_change_name)
	{
		// ¸®ÅÏ°ª
		//		0: »õÀÌ¸§À» ¼³Á¤ÇÑ µÚ ·Î±×¾Æ¿ôÀ» ¾ÈÇßÀ½
		//		1: ½ºÅ©¸³Æ®¿¡¼­ ¹®ÀÚ¿­ÀÌ ³Ñ¾î¿ÀÁö ¾Ê¾ÒÀ½
		//		2: check_name À» Åë°úÇÏÁö ¸øÇßÀ½
		//		3: ÀÌ¹Ì °°Àº ÀÌ¸§ÀÌ »ç¿ëÁß
		//		4: ¼º°ø
		//		5: ÇØ´ç ±â´É Áö¿øÇÏÁö ¾ÊÀ½
#ifdef ENABLE_LOCALECHECK_CHANGENAME
		lua_pushnumber(L, 5);
		return 1;
#endif

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ( ch->GetNewName().size() != 0 )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if ( lua_isstring(L, 1) != true )
		{
			lua_pushnumber(L, 1);
			return 1;
		}

		const char * szName = lua_tostring(L, 1);

		if ( check_name(szName) == false )
		{
			lua_pushnumber(L, 2);
			return 1;
		}

		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery), "SELECT COUNT(*) FROM player%s WHERE name='%s'", get_table_postfix(), szName);

		auto pmsg(DBManager::Instance().DirectQuery(szQuery));
		if (pmsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

			int count = 0;
			str_to_number(count, row[0]);

			// A character with that name already exists
			if (count != 0)
			{
				lua_pushnumber(L, 3);
				return 1;
			}
		}

		const uint32_t pid = ch->GetPlayerID();
		db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
		db_clientdesc->Packet(&pid, sizeof(uint32_t));

		/* delete messenger list */
		MessengerManager::Instance().RemoveAllList(ch->GetName());
#ifdef ENABLE_MESSENGER_BLOCK
		MessengerManager::Instance().RemoveAllBlockList(ch->GetName());
#endif

		/* change_name_log */
		LogManager::Instance().ChangeNameLog(pid, ch->GetName(), szName, ch->GetDesc()->GetHostName());

		snprintf(szQuery, sizeof(szQuery), "UPDATE player%s SET name='%s' WHERE id=%u", get_table_postfix(), szName, pid);
		auto msg = DBManager::Instance().DirectQuery(szQuery);

		ch->SetNewName(szName);
		lua_pushnumber(L, 4);
		return 1;
	}

	ALUA(pc_is_dead)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch != nullptr)
		{
			lua_pushboolean(L, ch->IsDead());
			return 1;
		}

		lua_pushboolean(L, true);

		return 1;
	}

	ALUA(pc_reset_status)
	{
		if (lua_isnumber(L, 1))
		{
			const int idx = static_cast<int>(lua_tonumber(L, 1));

			if (idx >= 0 && idx < 4)
			{
				LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
				if (!ch)
				{
					lua_pushboolean(L, false);
					return 1;
				}

				int point = POINT_NONE;
				char buf[128];

				switch (idx)
				{
				case 0:
					point = POINT_HT;
					break;
				case 1:
					point = POINT_IQ;
					break;
				case 2:
					point = POINT_ST;
					break;
				case 3:
					point = POINT_DX;
					break;
				default:
					lua_pushboolean(L, false);
					return 1;
				}

				const int old_val = ch->GetRealPoint(point);
				const int old_stat = ch->GetRealPoint(POINT_STAT);

				ch->SetRealPoint(point, 1);
				ch->SetPoint(point, ch->GetRealPoint(point));

				ch->PointChange(POINT_STAT, old_val);	//@fixme536

				if (point == POINT_HT)
				{
					const uint8_t job = ch->GetJob();
					ch->SetRandomHP((ch->GetLevel() - 1) * number(JobInitialPoints[job].hp_per_lv_begin, JobInitialPoints[job].hp_per_lv_end));
				}
				else if (point == POINT_IQ)
				{
					const uint8_t job = ch->GetJob();
					ch->SetRandomSP((ch->GetLevel() - 1) * number(JobInitialPoints[job].sp_per_lv_begin, JobInitialPoints[job].sp_per_lv_end));
				}

				ch->ComputePoints();
				ch->PointsPacket();

				if (point == POINT_HT)
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				else if (point == POINT_IQ)
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				switch (idx)
				{
				case 0:
					snprintf(buf, sizeof(buf), "reset ht(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 1:
					snprintf(buf, sizeof(buf), "reset iq(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 2:
					snprintf(buf, sizeof(buf), "reset st(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				case 3:
					snprintf(buf, sizeof(buf), "reset dx(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_STAT));
					break;
				default:
					break;
				}

				LogManager::Instance().CharLog(ch, 0, "RESET_ONE_STATUS", buf);

				lua_pushboolean(L, true);
				return 1;
			}
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(pc_get_ht)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_HT) : 0);
		return 1;
	}

	ALUA(pc_set_ht)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_HT);
		ch->SetRealPoint(POINT_HT, newPoint);
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_iq)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_IQ) : 0);
		return 1;
	}

	ALUA(pc_set_iq)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_IQ);
		ch->SetRealPoint(POINT_IQ, newPoint);
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_st)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_ST) : 0);
		return 1;
	}

	ALUA(pc_set_st)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_ST);
		ch->SetRealPoint(POINT_ST, newPoint);
		ch->PointChange(POINT_ST, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_dx)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_DX) : 0);
		return 1;
	}

	ALUA(pc_set_dx)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_DX);
		ch->SetRealPoint(POINT_DX, newPoint);
		ch->PointChange(POINT_DX, 0);
		ch->PointChange(POINT_STAT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_is_near_vid)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushboolean(L, false);
		}
		else
		{
			const LPCHARACTER& pMe = CQuestManager::Instance().GetCurrentCharacterPtr();
			const LPCHARACTER& pOther = CHARACTER_MANAGER::Instance().Find(static_cast<uint32_t>(lua_tonumber(L, 1)));

			if (pMe != nullptr && pOther != nullptr)
				lua_pushboolean(L, (DISTANCE_APPROX(pMe->GetX() - pOther->GetX(), pMe->GetY() - pOther->GetY()) < static_cast<int>(lua_tonumber(L, 2) * 100)));
			else
				lua_pushboolean(L, false);
		}

		return 1;
	}

	ALUA(pc_get_socket_items)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		lua_newtable(L);

		if (pChar == nullptr)
			return 1;

		int idx = 1;

		/*
		I don't think there's any need for the Dragon Soul Stone slot.
		This function seems to be a function for evacuation.
		*/
		for (int i = 0; i < WEAR_MAX_NUM; i++)
		{
			LPITEM pItem = pChar->GetInventoryItem(i);

			if (pItem != nullptr)
			{
				if (pItem->IsEquipped() == false)
				{
					int j = 0;
					for (; j < ITEM_SOCKET_MAX_NUM; j++)
					{
						const long socket = pItem->GetSocket(j);

						if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
						{
							const TItemTable* pItemInfo = ITEM_MANAGER::Instance().GetTable(socket);
							if (pItemInfo != nullptr)
							{
								if (pItemInfo->bType == ITEM_METIN)
									break;
							}
						}
					}

					if (j >= ITEM_SOCKET_MAX_NUM)
						continue;

					lua_newtable(L);

					{
						lua_pushstring(L, pItem->GetName());
						lua_rawseti(L, -2, 1);

						lua_pushnumber(L, i);
						lua_rawseti(L, -2, 2);
					}

					lua_rawseti(L, -2, idx++);
				}
			}
		}

		return 1;
	}

	ALUA(pc_get_empty_inventory_count)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
			lua_pushnumber(L, pChar->CountEmptyInventory());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_logoff_interval)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
			lua_pushnumber(L, pChar->GetLogOffInterval());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_player_id)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
			lua_pushnumber(L, pChar->GetPlayerID());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_account_id)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
		{
			if (pChar->GetDesc() != nullptr)
			{
				lua_pushnumber(L, pChar->GetDesc()->GetAccountTable().id);
				return 1;
			}
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(pc_get_account)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
		{
			if (nullptr != pChar->GetDesc())
			{
				lua_pushstring(L, pChar->GetDesc()->GetAccountTable().login);
				return 1;
			}
		}

		lua_pushstring(L, "");
		return 1;
	}

	ALUA(pc_is_riding)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
		{
			const bool is_riding = pChar->IsRiding();

			lua_pushboolean(L, is_riding);

			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(pc_get_special_ride_vnum)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
		{
			LPITEM Unique1 = pChar->GetWear(WEAR_UNIQUE1);
			LPITEM Unique2 = pChar->GetWear(WEAR_UNIQUE2);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			LPITEM MountCostume = pChar->GetWear(WEAR_COSTUME_MOUNT);
#endif

			if (nullptr != Unique1)
			{
				if (UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup())
				{
					lua_pushnumber(L, Unique1->GetVnum());
					lua_pushnumber(L, Unique1->GetSocket(0)); // @fixme152 (2->0)
					return 2;
				}
			}

			if (nullptr != Unique2)
			{
				if (UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup())
				{
					lua_pushnumber(L, Unique2->GetVnum());
					lua_pushnumber(L, Unique2->GetSocket(0)); // @fixme152 (2->0)
					return 2;
				}
			}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			if (MountCostume)
			{
				lua_pushnumber(L, MountCostume->GetVnum());
				lua_pushnumber(L, MountCostume->GetSocket(0)); // @fixme152 (2->0)
				return 2;
			}
#endif
		}

		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);

		return 2;
	}

	ALUA(pc_can_warp)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
			lua_pushboolean(L, pChar->CanWarp());
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(pc_dec_skill_point)
	{
		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
			pChar->PointChange(POINT_SKILL, -1);

		return 0;
	}

	ALUA(pc_get_skill_point)
	{
		const LPCHARACTER& pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (nullptr != pChar)
			lua_pushnumber(L, pChar->GetPoint(POINT_SKILL));
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_channel_id)
	{
		lua_pushnumber(L, g_bChannel);

		return 1;
	}

	ALUA(pc_give_poly_marble)
	{
		const int dwVnum = static_cast<int>(lua_tonumber(L, 1));

		const CMob* MobInfo = CMobManager::Instance().Get(dwVnum);

		if (nullptr == MobInfo)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		if (0 == MobInfo->m_table.dwPolymorphItemVnum)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(MobInfo->m_table.dwPolymorphItemVnum);

		if (nullptr == item)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		item->SetSocket(0, dwVnum);

		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

#ifdef ENABLE_SPECIAL_INVENTORY
		const int iEmptyCell = ch->GetEmptyInventory(item);
#else
		const int iEmptyCell = ch->GetEmptyInventory(item->GetSize());
#endif

		if (-1 == iEmptyCell)
		{
			M2_DESTROY_ITEM(item);
			lua_pushboolean(L, false);
			return 1;
		}

		item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyCell));

		const PC* pPC = CQuestManager::Instance().GetCurrentPC();
		if (!pPC)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LogManager::Instance().QuestRewardLog(pPC->GetCurrentQuestName().c_str(), ch->GetPlayerID(), ch->GetLevel(), MobInfo->m_table.dwPolymorphItemVnum, dwVnum);

		lua_pushboolean(L, true);

		return 1;
	}

	ALUA(pc_get_sig_items)
	{
		const uint32_t group_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		int count = 0;
		for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if (ch->GetInventoryItem(i) != nullptr && ch->GetInventoryItem(i)->GetSIGVnum() == group_vnum)
			{
				lua_pushnumber(L, ch->GetInventoryItem(i)->GetID());
				count++;
			}
		}

		return count;
	}

	ALUA(pc_charge_cash)
	{
		TRequestChargeCash packet{};

		const uint32_t amount = lua_isnumber(L, 1) ? static_cast<int>(lua_tonumber(L, 1)) : 0;
		std::string strChargeType = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ((nullptr == ch) || (nullptr == ch->GetDesc()) || (1 > amount) || (50000 < amount))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		packet.dwAID = ch->GetDesc()->GetAccountTable().id;
		packet.dwAmount = amount;
		packet.eChargeType = ERequestCharge_Cash;

		if (0 < strChargeType.length())
			std::transform(strChargeType.begin(), strChargeType.end(), strChargeType.begin(), (int(*)(int))std::tolower);

		if ("mileage" == strChargeType)
			packet.eChargeType = ERequestCharge_Mileage;

		db_clientdesc->DBPacketHeader(HEADER_GD_REQUEST_CHARGE_CASH, 0, sizeof(TRequestChargeCash));
		db_clientdesc->Packet(&packet, sizeof(packet));

		lua_pushboolean(L, 1);
		return 1;
	}

	ALUA(pc_give_award)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3))
		{
			sys_err("QUEST give award call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const int icount = static_cast<int>(lua_tonumber(L, 2));

		sys_log(0, "QUEST [award] item %d to login %s", dwVnum, ch->GetDesc()->GetAccountTable().login);

#ifdef ENABLE_SUNG_MAHI_TOWER
		DBManager::instance().Query("INSERT INTO item_award (login, vnum, count, given_time, why, mall) VALUES ('%s', %d, %d, now(), '%s', 1);", ch->GetDesc()->GetAccountTable().login, dwVnum, icount, lua_tostring(L, 3));
#else
		DBManager::Instance().Query("INSERT INTO item_award (login, vnum, count, given_time, why, mall)select '%s', %d, %d, now(), '%s', 1 from DUAL where not exists (select login, why from item_award where login = '%s' and why  = '%s') ;",
			ch->GetDesc()->GetAccountTable().login,
			dwVnum,
			icount,
			lua_tostring(L, 3),
			ch->GetDesc()->GetAccountTable().login,
			lua_tostring(L, 3));
#endif

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(pc_give_award_socket)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isstring(L, 3) || !lua_isstring(L, 4) || !lua_isstring(L, 5) || !lua_isstring(L, 6))
		{
			sys_err("QUEST give award call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const int icount = static_cast<int>(lua_tonumber(L, 2));

		sys_log(0, "QUEST [award] item %d to login %s", dwVnum, ch->GetDesc()->GetAccountTable().login);

		DBManager::Instance().Query("INSERT INTO item_award (login, vnum, count, given_time, why, mall, socket0, socket1, socket2)select '%s', %d, %d, now(), '%s', 1, %s, %s, %s from DUAL where not exists (select login, why from item_award where login = '%s' and why = '%s') ;",
			ch->GetDesc()->GetAccountTable().login,
			dwVnum,
			icount,
			lua_tostring(L, 3),
			lua_tostring(L, 4),
			lua_tostring(L, 5),
			lua_tostring(L, 6),
			ch->GetDesc()->GetAccountTable().login,
			lua_tostring(L, 3));

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(pc_get_informer_type) //Germany Futures Features
	{
		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
		{
			//sys_err("quest cmd test %s", pChar->GetItemAward_cmd() );
			lua_pushstring(L, pChar->GetItemAward_cmd());
		}
		else
			lua_pushstring(L, "");

		return 1;
	}

	ALUA(pc_get_informer_item)
	{
		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (pChar != nullptr)
			lua_pushnumber(L, pChar->GetItemAward_vnum());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(pc_get_killee_drop_pct)
	{
		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);
			return 2;
		}

		LPCHARACTER pKillee = pChar->GetQuestNPC();

		int iDeltaPercent, iRandRange;
		if (nullptr == pKillee || !ITEM_MANAGER::Instance().GetDropPct(pKillee, pChar, iDeltaPercent, iRandRange))
		{
			sys_err("killee is null");
			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);

			return 2;
		}

		lua_pushnumber(L, iDeltaPercent);
		lua_pushnumber(L, iRandRange);

		return 2;
	}

#ifdef ENABLE_NEWSTUFF

	#define PC_MI0L_ARG1 2		// 1: vnum or locale_name, 2: count
	#define PC_MI0L_ARG2 3		// socket 1-2-3
	#define PC_MI0L_ARG3 7*2	// (type, value)*7
	enum eMakeItemType
	{
		PCMI0_GIVE,
		PCMI0_DROP,
		PCMI0_DROPWP,
		PCMI0_MAX
	};

	ALUA(pc_make_item0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_istable(L, 1) && !lua_istable(L, 2) && !lua_istable(L, 3) && !lua_isnumber(L, 4))
			return 0;

		int m_idx = 0;
		// config arg1
		uint32_t m_vnum = 0;
		int m_count = 0;
		// start arg1
		lua_pushnil(L);
		while (lua_next(L, 1))
		{
			switch (m_idx)
			{
				case 0:
					if (lua_isnumber(L, -1))
					{
						if ((m_vnum = static_cast<uint32_t>(lua_tonumber(L, -1))) <= 0)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not valid by that vnum.", m_vnum);
							return 0;
						}
					}
					else if (lua_isstring(L, -1))
					{
						if (!ITEM_MANAGER::Instance().GetVnum(lua_tostring(L, -1), m_vnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum.", m_vnum);
							return 0;
						}
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that unknown vnum.");
						return 0;
					}
					break;

				case 1:
					if (lua_isnumber(L, -1))
					{
						// if ((m_count = MINMAX(1, lua_tonumber(L, -1), g_bItemCountLimit))<=0)
						if ((m_count = static_cast<int>(lua_tonumber(L, -1))) <= 0)
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not valid by that count.", m_count);
							return 0;
						}
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that unknown count.");
						return 0;
					}
					break;

				default:
					ch->ChatPacket(CHAT_TYPE_INFO, "arg1(%d) index found", m_idx);
					break;
			}
			m_idx++;
			lua_pop(L, 1);
		}
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg1 is %u %d", m_vnum, m_count);
		// end arg1
		// config arg2
		int m_socket[ITEM_SOCKET_MAX_NUM] = { 0 };
		// start arg2
		// ch->ChatPacket(CHAT_TYPE_INFO, "%d: %s", lua_type(L, 1), lua_typename(L, lua_type(L, 1)));
		m_idx = 0;
		lua_pushnil(L);
		while (lua_next(L, 2) && m_idx < ITEM_SOCKET_MAX_NUM)
		{
			if (!lua_isnumber(L, -1))
				return 0;
			m_socket[m_idx++] = static_cast<int>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg2 is %d %d %d", m_socket[0], m_socket[1], m_socket[2]);
		// end arg2
		// config arg3
		int m_attr[ITEM_ATTRIBUTE_MAX_NUM * 2] = { 0 };
		// start arg3
		m_idx = 0;
		lua_pushnil(L);
		while (lua_next(L, 3) && m_idx < (ITEM_ATTRIBUTE_MAX_NUM * 2))
		{
			if (!lua_isnumber(L, -1))
				return 0;
			m_attr[m_idx++] = static_cast<int>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg3 is %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		// m_attr[0], m_attr[1], m_attr[2], m_attr[3], m_attr[4], m_attr[5],
		// m_attr[6], m_attr[7], m_attr[8], m_attr[9], m_attr[10], m_attr[11],
		// m_attr[12], m_attr[13]
		// );
		// end arg3
		// config arg4
		uint32_t m_state = 0;
		// start arg4
		if ((m_state = static_cast<uint32_t>(lua_tonumber(L, 4))) >= PCMI0_MAX)
			return 0;
		// ch->ChatPacket(CHAT_TYPE_INFO, "arg4 is %d", m_state);
		// end arg4

		// create item
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(m_vnum, 1, 0, false);
		if (pkNewItem)
		{
			// socket
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
				pkNewItem->SetSocket(i, m_socket[i]);
			// attr
			for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
				pkNewItem->SetForceAttribute(i, m_attr[(i * 2) + 0], m_attr[(i * 2) + 1]);
			// state
			int iEmptyCell = -1;
			int m_sec = 0;
			PIXEL_POSITION pos{};
			switch (m_state)
			{
				case PCMI0_GIVE:
#ifdef ENABLE_SPECIAL_INVENTORY
					iEmptyCell = ch->GetEmptyInventory(pkNewItem);
#else
					iEmptyCell = ch->GetEmptyInventory(pkNewItem->GetSize());
#endif
					if (-1 == iEmptyCell)
					{
						M2_DESTROY_ITEM(pkNewItem);
						lua_pushboolean(L, false);
						return 1;
					}
					pkNewItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyCell));
					break;
				case PCMI0_DROPWP:
					if (lua_isnumber(L, 5) && (m_sec = static_cast<int>(lua_tonumber(L, 5))))
						pkNewItem->SetOwnership(ch, m_sec <= 0 ? 1 : m_sec); //, ch->ChatPacket(CHAT_TYPE_INFO, "arg5 is %d", m_sec<=0?1:m_sec);
					else
						pkNewItem->SetOwnership(ch);
				case PCMI0_DROP:
					pos.x = ch->GetX() + number(-200, 200);
					pos.y = ch->GetY() + number(-200, 200);

					pkNewItem->AddToGround(ch->GetMapIndex(), pos);
					pkNewItem->StartDestroyEvent();
					break;
				default:
					lua_pushboolean(L, false);
					return 1;
			}
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);

		return 1;
	}

	ALUA(pc_set_race0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const int amount = MINMAX(0, static_cast<int>(lua_tonumber(L, 1)), JOB_MAX_NUM);
		const ESex mySex = GET_SEX(ch);
		uint32_t dwRace = MAIN_RACE_WARRIOR_M;
		switch (amount)
		{
			case JOB_WARRIOR:
				dwRace = (mySex == ESex::SEX_MALE) ? MAIN_RACE_WARRIOR_M : MAIN_RACE_WARRIOR_W;
				break;
			case JOB_ASSASSIN:
				dwRace = (mySex == ESex::SEX_MALE) ? MAIN_RACE_ASSASSIN_M : MAIN_RACE_ASSASSIN_W;
				break;
			case JOB_SURA:
				dwRace = (mySex == ESex::SEX_MALE) ? MAIN_RACE_SURA_M : MAIN_RACE_SURA_W;
				break;
			case JOB_SHAMAN:
				dwRace = (mySex == ESex::SEX_MALE) ? MAIN_RACE_SHAMAN_M : MAIN_RACE_SHAMAN_W;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case JOB_WOLFMAN:
				dwRace = MAIN_RACE_WOLFMAN_M;
				break;
#endif
			default:
				break;
		}
		if (dwRace != ch->GetRaceNum())
		{
			ch->SetRace(dwRace);
			ch->ClearSkill();
			ch->SetSkillGroup(0);
			// quick mesh change workaround begin
			ch->SetPolymorph(101);
			ch->SetPolymorph(0);
			// quick mesh change workaround end
		}
		return 0;
	}

	ALUA(pc_del_another_quest_flag)
	{
		if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		{
			sys_err("QUEST wrong del flag");
			return 0;
		}
		else
		{
			const char* sz = lua_tostring(L, 1);
			const char* sz2 = lua_tostring(L, 2);
			CQuestManager& q = CQuestManager::Instance();
			PC* pPC = q.GetCurrentPC();
			lua_pushboolean(L, pPC ? pPC->DeleteFlag(string(sz) + "." + sz2) : false);
			return 1;
		}
	}

	ALUA(pc_pointchange)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->PointChange(static_cast<uint16_t>(lua_tonumber(L, 1)), static_cast<int>(lua_tonumber(L, 2)), lua_toboolean(L, 3), lua_toboolean(L, 4));
		return 0;
	}

	ALUA(pc_pullmob)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->PullMonster();
		return 0;
	}

	ALUA(pc_set_level0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->ResetPoint(static_cast<int>(lua_tonumber(L, 1)));
		ch->ClearSkill();
		ch->ClearSubSkill();
		return 0;
	}

	ALUA(pc_set_gm_level)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->SetGMLevel();
		return 0;
	}

	ALUA(pc_if_fire)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsAffectFlag(AFF_FIRE) : false);
		return 1;
	}

	ALUA(pc_if_invisible)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsAffectFlag(AFF_INVISIBILITY) : false);
		return 1;
	}

	ALUA(pc_if_poison)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsAffectFlag(AFF_POISON) : false);
		return 1;
	}

#ifdef ENABLE_WOLFMAN_CHARACTER
	ALUA(pc_if_bleeding)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsAffectFlag(AFF_BLEEDING) : false);
		return 1;
	}
#endif

	ALUA(pc_if_slow)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsAffectFlag(AFF_SLOW) : false);
		return 1;
	}

	ALUA(pc_if_stun)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->IsAffectFlag(AFF_STUN) : false);
		return 1;
	}

	ALUA(pc_sf_fire)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_FIRE, 0, 0, AFF_FIRE, 3 * 5 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_FIRE);
		return 0;
	}

	ALUA(pc_sf_invisible)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_INVISIBILITY, 0, 0, AFF_INVISIBILITY, 60 * 60 * 24 * 365 * 60 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_INVISIBILITY);
		return 0;
	}

	ALUA(pc_sf_poison)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_POISON, 0, 0, AFF_POISON, 30 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_POISON);
		return 0;
	}

#ifdef ENABLE_WOLFMAN_CHARACTER
	ALUA(pc_sf_bleeding)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_BLEEDING, 0, 0, AFF_BLEEDING, 30 + 1, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_BLEEDING);
		return 0;
	}
#endif

	ALUA(pc_find_first_item)
	{
		if (!lua_isnumber(L, -1)) {
			lua_pushnumber(L, -1);
			return 1;
		}
		
		LPITEM item = CQuestManager::Instance().GetCurrentCharacterPtr()->FindSpecifyItem((uint32_t)lua_tonumber(L, -1));
		if (!item) {
			lua_pushnumber(L, -1);
			return 1;
		}

		lua_pushnumber(L, item->GetCell());
		return 1;
	}

	ALUA(pc_specific_effect)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (lua_isstring(L, 1))
		{
			ch->SpecificEffectPacket (lua_tostring(L, 1));
		}
		return 0;
	}

	ALUA(pc_start_duel)
	{
		uint32_t vid = (int)lua_tonumber(L, 1);
		if (!vid) {
			lua_pushnumber(L, 0);
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPCHARACTER pkVictim = CHARACTER_MANAGER::Instance().Find(vid);

		if (!pkVictim) {
			lua_pushnumber(L, 0);
			return 0;
		}

		if (pkVictim->IsNPC()) {
			lua_pushnumber(L, 0);
			return 0;
		}

		CPVPManager::Instance().Insert(ch, pkVictim);
		CPVPManager::Instance().Insert(pkVictim, ch);

		lua_pushnumber(L, 0);
		return 1;
	}


	/*
		@desc Stuns the current CurrentCharacter.
		@usage pc.stun((time as int in seconds [2 seconds]), (reason as string)[QUEST_STUN]), (forced as boolean)[false])
	*/
	ALUA(pc_stun_player)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		
		if (!ch){
			sys_err("no valid Character found [GetCurrentCharacterPtr() == nullptr");
			return 0;
		}

		uint32_t time = (lua_isnumber(L,1) && (int)lua_tonumber(L, 1) > 0) ? (int)lua_tonumber(L, 1) : 2; //2 seconds by default
		const char * reason = (lua_isstring(L, 2)) ? lua_tostring(L, 2) : "QUEST_STUN";
		int IMMUNE_FLAG = (lua_isboolean(L, 3) && lua_toboolean(L, 3)) ? 0 : IMMUNE_STUN;

		SkillAttackAffect(ch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, time, reason);

		return 0;
	}

	ALUA(pc_reencode_view)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch) {
			sys_err("No character on a pc function!");
			return 0;
		}

		ch->ViewReencode();
		return 0;
	}

	ALUA(pc_sf_slow)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_SLOW, 19, -30, AFF_SLOW, 30, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_SLOW);
		return 0;
	}

	ALUA(pc_sf_stun)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_STUN, 0, 0, AFF_STUN, 30, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_STUN);
		return 0;
	}

	ALUA(pc_sf_kill)
	{
		LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(lua_tostring(L, 1));
		if (ch)
			ch->Dead(0, 0);
		return 0;
	}

	ALUA(pc_sf_dead)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->Dead(0, 0);
		return 0;
	}

	ALUA(pc_get_exp_level)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("arg1 must be a number");
			return 0;
		}
		lua_pushnumber(L, static_cast<uint32_t>(exp_table[MINMAX(0, static_cast<int>(lua_tonumber(L, 1)), PLAYER_MAX_LEVEL_CONST)] / 100));
		return 1;
	}

	ALUA(pc_get_exp_level0)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("arg1 and arg2 must be numbers");
			return 0;
		}
		lua_pushnumber(L, static_cast<uint32_t>((exp_table[MINMAX(0, static_cast<int>(lua_tonumber(L, 1)), PLAYER_MAX_LEVEL_CONST)] / 100) * MINMAX(1, static_cast<int>(lua_tonumber(L, 2)), 100)));
		return 1;
	}

	ALUA(pc_set_max_health)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
		ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
		return 0;
	}

	ALUA(pc_get_ip0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushstring(L, ch ? ch->GetDesc()->GetHostName() : nullptr);
		return 1;
	}

	ALUA(pc_get_client_version0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushstring(L, ch ? ch->GetDesc()->GetClientVersion() : nullptr);
		return 1;
	}

	ALUA(pc_dc_delayed0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		const bool bRet = ch->GetDesc()->DelayedDisconnect(MINMAX(0, static_cast<int>(lua_tonumber(L, 1)), 60 * 5));
		lua_pushboolean(L, bRet);
		return 1;
	}

	ALUA(pc_dc_direct0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->Disconnect(lua_tostring(L, 1));
		return 0;
	}

	ALUA(pc_is_trade0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, ch->GetExchange() != nullptr);
		return 1;
	}

	ALUA(pc_is_busy0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, (ch->GetExchange() || ch->GetMyShop() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			ch->GetViewingShopOwner() || 
#else
			ch->GetShopOwner() || 
#endif
			ch->IsOpenSafebox() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			|| ch->IsOpenGuildstorage()
#endif
		));
		return 1;
	}

	ALUA(pc_is_arena0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, ch->GetArena() != nullptr);
		return 1;
	}

	ALUA(pc_is_arena_observer0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch ? ch->GetArenaObserverMode() : 0);
		return 1;
	}

	ALUA(pc_equip_slot0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LPITEM item = ch->GetInventoryItem(static_cast<uint16_t>(lua_tonumber(L, 1)));
		lua_pushboolean(L, (item) ? ch->EquipItem(item) : false);
		return 1;
	}

	ALUA(pc_unequip_slot0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LPITEM item = ch->GetWear(static_cast<uint8_t>(lua_tonumber(L, 1)));
		lua_pushboolean(L, (item) ? ch->UnequipItem(item) : false);
		return 1;
	}

	ALUA(pc_is_available0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch != nullptr);
		return 1;
	}

	ALUA(pc_give_random_book0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		LPITEM item = ch->AutoGiveItem(50300);
		if (item)
		{
			if (lua_isnumber(L, 1))
				item->SetSocket(0, ::GetRandomSkillVnum(static_cast<uint8_t>(lua_tonumber(L, 1))));
			else
				item->SetSocket(0, ::GetRandomSkillVnum());
		}
		lua_pushboolean(L, item != nullptr);
		return 1;
	}

	ALUA(pc_is_pvp0)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		lua_pushboolean(L, (ch) ? CPVPManager::Instance().IsFighting(ch->GetPlayerID()) : false);
		return 1;
	}

	ALUA(pc_set_safebox_delivered)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->SetQuestFlag("stash.safebox_delivered", 1);

		return 1;
	}
#endif

#ifdef ENABLE_PC_OPENSHOP
	ALUA(pc_open_shop0)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		//PREVENT_TRADE_WINDOW
		if (ch->IsOpenSafebox() || ch->GetExchange() || ch->GetMyShop() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			|| ch->IsOpenGuildstorage()
#endif
		)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[876]You cannot trade in the shop while another window is open."));
			return 0;
		}
		//END_PREVENT_TRADE_WINDOW

		LPSHOP sh = CShopManager::Instance().Get(lua_tonumber(L, 1));
		if (sh)
		{
			sh->AddGuest(ch, 0, false);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			ch->SetViewingShopOwner(nullptr);
#else
			ch->SetShopOwner(nullptr);
#endif
		}
		return 0;
	}
#endif

#ifdef ENABLE_NEWGUILDMAKE
	enum MKGLD { MKGLD_INVALID_NAME_LENGTH = -2, MKGLD_INVALID_NAME_INPUT = -1, MKGLD_GUILD_NOT_CREATED = 0, MKGLD_GUILD_CREATED = 1, MKGLD_ALREADY_GUILDED = 2, MKGLD_ALREADY_MASTER_GUILD = 3 };
	ALUA(pc_make_guild0)
	{
		/*
		-2 guild name is invalid (strlen <2 or >11!)
		-1 guild name is invalid (special chars found!)
		0 guild not created (guild name already present or already member of a guild)
		1 guild created
		2 player already part of a guild
		3 player already guild master
		*/

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (ch->GetGuild())
		{
			lua_pushnumber(L, (ch->GetPlayerID() == ch->GetGuild()->GetMasterPID()) ? MKGLD_ALREADY_MASTER_GUILD : MKGLD_ALREADY_GUILDED);
			return 1;
		}
		const char* guild_name = lua_tostring(L, 1);
		const size_t guild_lname = strlen(guild_name);
		if (guild_lname < 2 || 11 < guild_lname)
		{
			lua_pushnumber(L, MKGLD_INVALID_NAME_LENGTH);
			return 1;
		}

		TGuildCreateParameter cp;
		memset(&cp, 0, sizeof(cp));

		cp.master = ch;
		strlcpy(cp.name, guild_name, sizeof(cp.name));

		int ret_type = MKGLD_GUILD_NOT_CREATED;
		if (check_name(cp.name))
		{
			if (CGuildManager::Instance().CreateGuild(cp))
				ret_type = MKGLD_GUILD_CREATED;
			else
				ret_type = MKGLD_GUILD_NOT_CREATED;
		}
		else ret_type = MKGLD_INVALID_NAME_INPUT;
		lua_pushnumber(L, ret_type);
		return 1;
	}
#endif

#ifdef ENABLE_NEWSTUFF_2017
	// Nyx - 2016.06.20
	ALUA(pc_give_and_equip_item)
	{
		LPCHARACTER pkCharacter = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!pkCharacter || !lua_isnumber(L, 1))
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		LPITEM pkItem = ITEM_MANAGER::Instance().CreateItem(static_cast<uint32_t>(lua_tonumber(L, 1)));
		if (pkItem && pkCharacter->EquipItem(pkItem))
		{
			lua_pushboolean(L, TRUE);
		}
		else
		{
			M2_DESTROY_ITEM(pkItem);
			lua_pushboolean(L, FALSE);
		}

		return 1;
	}

	// Nyx 05.09.2017
	ALUA(pc_reset_change_empire)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		if (ch->GetChangeEmpireCount() == 0)
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		DBManager::Instance().Query("DELETE FROM log.change_empire%s WHERE account_id=%d",
			get_table_postfix(), ch->GetAID());

		lua_pushboolean(L, TRUE);

		return 1;
	}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	ALUA(pc_open_acce_absorption)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->OpenAcceAbsorption();
		return 1;
	}

	ALUA(pc_open_acce_combination)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		ch->OpenAcceCombination();
		return 1;
	}
#endif

#ifdef ENABLE_678TH_SKILL
	ALUA(pc_get_678th_skill_points)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetQuestFlag("678th_skill.points") : 0);
		return 1;
	}

	ALUA(pc_set_678th_skill_points)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch || !lua_isnumber(L, 1))
		{
			sys_err("arg1 must be a number");
			return 0;
		}

		const uint32_t dwPoints = static_cast<uint32_t>(lua_tonumber(L, 1));
		ch->SetQuestFlag("678th_skill.points", dwPoints);

		return 1;
	}

	ALUA(pc_give_skill_points)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch || !lua_isnumber(L, 1))
		{
			sys_err("arg1 must be a number");
			return 0;
		}

		const uint32_t dwPoints = static_cast<uint32_t>(lua_tonumber(L, 1));
		ch->PointChange(POINT_SKILL, +dwPoints);
		ch->ComputePoints();
		ch->PointsPacket();

		return 1;
	}
#endif

#ifdef ENABLE_NEWSTUFF //ENABLE_ORC_MAZE
	ALUA(pc_get_move_pos_x)
	{
		CQuestManager& q = CQuestManager::Instance();
		PC* pPC = q.GetCurrentPC();

		lua_pushnumber(L, pPC->GetMoveTargetX());
		return 1;
	}

	ALUA(pc_get_move_pos_y)
	{
		CQuestManager& q = CQuestManager::Instance();
		PC* pPC = q.GetCurrentPC();

		lua_pushnumber(L, pPC->GetMoveTargetY());
		return 1;
	}

	ALUA(pc_move_to)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("no coodinate argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		int x = static_cast<int>(lua_tonumber(L, 1));
		int y = static_cast<int>(lua_tonumber(L, 2));

		PIXEL_POSITION p;
		if (SECTREE_MANAGER::Instance().GetMapBasePosition(ch->GetX(), ch->GetY(), p))
		{
			x += p.x / 100;
			y += p.y / 100;
		}

		x *= 100;
		y *= 100;

		ch->Show(ch->GetMapIndex(), x, y, 0);
		ch->Stop();
		return 1;
	}

	// Nyx 02.07.2018
	ALUA(pc_is_equip_unique_item)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, FALSE);
			return 1;
		}

		const uint32_t dwItemVnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		if (ch->IsEquipUniqueItem(dwItemVnum))
			lua_pushboolean(L, TRUE);

		return 1;
	}

	// Nyx 02.07.2019
	ALUA(pc_send_command)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (ch)
			ch->ChatPacket(CHAT_TYPE_COMMAND, "%s", lua_tostring(L, 1));

		return 0;
	}
#endif

#ifdef ENABLE_12ZI
	ALUA(pc_set_animasphere)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const int animasphere = (int)lua_tonumber(L, 1);
		ch->SetAnimaSphere(animasphere);
		lua_pushnumber(L, animasphere);
		return 1;
	}

	ALUA(pc_get_animasphere)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetAnimaSphere() : 0);
		return 1;
	}

	ALUA(pc_delete_animasphere)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetAnimaSphere() < 12)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You haven't enough animasphere."));
			return 0;
		}

		ch->SetAnimaSphere(-12);
		return 1;
	}

	ALUA(pc_if_cz_unlimit_enter)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushboolean(L, ch->IsAffectFlag(AFF_CZ_UNLIMIT_ENTER));
		return 1;
	}

	ALUA(pc_sf_cz_unlimit_enter)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (lua_toboolean(L, 1))
			ch->AddAffect(AFFECT_CZ_UNLIMIT_ENTER, 0, 0, AFF_CZ_UNLIMIT_ENTER, 10800, 0, 1, 0);
		else
			ch->RemoveAffect(AFFECT_CZ_UNLIMIT_ENTER);
		return 0;
	}
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	ALUA(pc_update_dungeon_rank)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch || !ch->GetDesc())
			return 0;

		CQuestManager& q = CQuestManager::Instance();
		const std::string c_szQuestName = q.GetCurrentPC()->GetCurrentQuestName();

		ch->UpdateDungeonRanking(c_szQuestName);

		return 0;
	}
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	ALUA(pc_open_soul_roulette)
	{
		LPCHARACTER pkChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (pkChar && !pkChar->GetSoulRoulette())
			pkChar->SetSoulRoulette(M2_NEW CSoulRoulette(pkChar));
		return 0;
	}

	ALUA(pc_change_soul)
	{
		int soul = (int)lua_tonumber(L, -1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (soul + ch->GetSoulPoint() < 0)
			sys_err("QUEST wrong ChangeSoul %d (now %d)", soul, ch->GetSoulPoint());
		else
		{
			ch->PointChange(POINT_SOUL, soul, true);
		}

		return 0;
	}

	ALUA(pc_get_soul)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetSoulPoint());
		return 1;
	}

	ALUA(pc_change_soul_re)
	{
		int soulre = (int)lua_tonumber(L, -1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (soulre + ch->GetSoulRePoint() < 0)
			sys_err("QUEST wrong ChangeSoulRe %d (now %d)", soulre, ch->GetSoulRePoint());
		else
		{
			ch->PointChange(POINT_SOUL_RE, soulre, true);
		}

		return 0;
	}

	ALUA(pc_get_soul_re)
	{
		lua_pushnumber(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetSoulRePoint());
		return 1;
	}

	ALUA(pc_get_soul_global_rank)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;


		char szBuf[4096 + 1];
		ch->GetGlobalRankRoulette(szBuf, sizeof(szBuf));
		lua_pushstring(L, szBuf);
		return 1;
	}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	ALUA(pc_conqueror_reset_status)
	{
		if (lua_isnumber(L, 1))
		{
			const int idx = static_cast<int>(lua_tonumber(L, 1));

			if (idx >= 0 && idx < 4)
			{
				LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
				if (!ch)
				{
					lua_pushboolean(L, 0);
					return 1;
				}

				int point = POINT_NONE;
				char buf[128];

				switch (idx)
				{
					case 0:
						point = POINT_SUNGMA_STR;
						break;
					case 1:
						point = POINT_SUNGMA_HP;
						break;
					case 2:
						point = POINT_SUNGMA_MOVE;
						break;
					case 3:
						point = POINT_SUNGMA_IMMUNE;
						break;
					default:
						lua_pushboolean(L, false);
						return 1;
				}

				const int old_val = ch->GetRealPoint(point);
				const int old_stat = ch->GetRealPoint(POINT_CONQUEROR_POINT);

				ch->SetRealPoint(point, 0);
				ch->SetPoint(point, ch->GetRealPoint(point));
				ch->PointChange(POINT_CONQUEROR_POINT, old_val); //@fixme536

				ch->ComputePoints();
				ch->PointsPacket();

				switch (idx)
				{
					case 0:
						snprintf(buf, sizeof(buf), "reset cst(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_CONQUEROR_POINT));
						break;
					case 1:
						snprintf(buf, sizeof(buf), "reset cht(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_CONQUEROR_POINT));
						break;
					case 2:
						snprintf(buf, sizeof(buf), "reset move(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_CONQUEROR_POINT));
						break;
					case 3:
						snprintf(buf, sizeof(buf), "reset immu(%d)->1 stat_point(%d)->(%d)", old_val, old_stat, ch->GetRealPoint(POINT_CONQUEROR_POINT));
						break;
					default:
						break;
				}

				LogManager::Instance().CharLog(ch, 0, "RESET_SUNGMA_ONE_STATUS", buf);

				lua_pushboolean(L, true);
				return 1;
			}
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(pc_get_cht)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_SUNGMA_HP) : 0);
		return 1;
	}

	ALUA(pc_set_cht)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_SUNGMA_HP);
		ch->SetRealPoint(POINT_SUNGMA_HP, newPoint);
		ch->PointChange(POINT_SUNGMA_HP, 0);
		ch->PointChange(POINT_CONQUEROR_POINT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_cimmun)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_SUNGMA_IMMUNE) : 0);
		return 1;
	}

	ALUA(pc_set_cimmun)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_SUNGMA_IMMUNE);
		ch->SetRealPoint(POINT_SUNGMA_IMMUNE, newPoint);
		ch->PointChange(POINT_SUNGMA_IMMUNE, 0);
		ch->PointChange(POINT_CONQUEROR_POINT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_cst)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_SUNGMA_STR) : 0);
		return 1;
	}

	ALUA(pc_set_cst)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_SUNGMA_STR);
		ch->SetRealPoint(POINT_SUNGMA_STR, newPoint);
		ch->PointChange(POINT_SUNGMA_STR, 0);
		ch->PointChange(POINT_CONQUEROR_POINT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_get_cmove)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetRealPoint(POINT_SUNGMA_MOVE) : 0);
		return 1;
	}

	ALUA(pc_set_cmove)
	{
		if (lua_isnumber(L, 1) == false)
			return 1;

		const int newPoint = static_cast<int>(lua_tonumber(L, 1));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		const int usedPoint = newPoint - ch->GetRealPoint(POINT_SUNGMA_MOVE);
		ch->SetRealPoint(POINT_SUNGMA_MOVE, newPoint);
		ch->PointChange(POINT_SUNGMA_MOVE, 0);
		ch->PointChange(POINT_CONQUEROR_POINT, -usedPoint);
		ch->ComputePoints();
		ch->PointsPacket();
		return 1;
	}

	ALUA(pc_set_conqueror_level)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if ((ch->GetLevel() != gPlayerMaxLevel) || (ch->GetLevel() == gPlayerMaxLevel && ch->GetExp() < (exp_table_common[120] / 4)))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1752]");
			return 0;
		}

#	ifdef ENABLE_DEFENSE_WAVE
		if (ch->GetQuestFlag("defense_wave.is_completed") == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1753]");
			return 0;
		}
#	endif

		if (ch->GetConquerorLevel() > 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1754]");
			return 0;
		}

		ch->PointChange(POINT_CONQUEROR_LEVEL, 1, false, true);

		const auto skill_group = ch->GetSkillGroup();
		if (skill_group == 0)
			return 0;

		const auto job = ch->GetJob();
		if (job == 0)
			ch->SetSkillLevel(176, 1);
		else if (job == 1)
			ch->SetSkillLevel(176 + skill_group, 1);
		else if (job == 2)
			ch->SetSkillLevel(178 + skill_group, 1);
		else if (job == 3)
			ch->SetSkillLevel(180 + skill_group, 1);
		else if (job == 4)
			ch->SetSkillLevel(183, 1);
		ch->SkillLevelPacket();

		return 0;
	}

	ALUA(pc_get_conqueror_level)
	{
		lua_pushnumber(L, CQuestManager::instance().GetCurrentCharacterPtr()->GetConquerorLevel());
		return 1;
	}
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	ALUA(pc_open_gem_shop)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->IsOpenSafebox() || ch->GetExchange() || ch->GetMyShop() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			|| ch->IsOpenGuildstorage()
#endif
			)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;186]");
			return 0;
		}

		bool bAllSlotUnlocked = lua_isboolean(L, 1) ? lua_toboolean(L, 1) : false;
		ch->OpenGemShop(bAllSlotUnlocked);
		return 0;
	}

	ALUA(pc_create_gaya)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		const int cost = static_cast<int>(lua_tonumber(L, 1));
		const int pct = static_cast<int>(lua_tonumber(L, 2));
		const int metinstone_cell = static_cast<int>(lua_tonumber(L, 3));
		const int glimmerstone_count = static_cast<int>(lua_tonumber(L, 4));

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		LPITEM metinstone_item = ch->GetInventoryItem(metinstone_cell);

		if (metinstone_item)
			lua_pushboolean(L, ch->CreateGem(glimmerstone_count, metinstone_item, cost, pct));
		else
			lua_pushboolean(L, 0);

		return 1;
	}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	ALUA(pc_is_blocked)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong::pc_is_blocked");
			return 0;
		}

		const char* arg1 = lua_tostring(L, 1);
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
		if (!tch || !tch->IsPC())
		{
			sys_err("wrong::pc_is_blocked-2");
			return 0;
		}

		lua_pushboolean(L, MessengerManager::Instance().IsBlocked(ch->GetName(), tch->GetName()));
		return 1;
	}

	ALUA(pc_is_friend)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("wrong::pc_is_blocked");
			return 0;
		}

		const char* arg1 = lua_tostring(L, 1);
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
		if (!tch || !tch->IsPC())
		{
			sys_err("wrong::pc_is_blocked-2");
			return 0;
		}

		lua_pushboolean(L, MessengerManager::Instance().IsInList(ch->GetName(), tch->GetName()));
		return 1;
	}
#endif

#ifdef ENABLE_HWID_BAN
	ALUA(pc_get_hwid)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if(!ch) return 0;

		lua_pushstring(L,ch->GetDesc()->GetAccountTable().hwid);
		return 1;
	}
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	ALUA(pc_get_okay_global_rank)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		char szBuf[4096 + 1];
		ch->GetGlobalRank(szBuf, sizeof(szBuf));
		lua_pushstring(L, szBuf);
		return 1;
	}

	ALUA(pc_get_okay_rund_rank)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		char szBuf[4096 + 1];
		ch->GetRundRank(szBuf, sizeof(szBuf));
		lua_pushstring(L, szBuf);
		return 1;
	}
#endif

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
	ALUA(pc_hide_costume)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("pc_hide_costume::wrong part value (1-4)");
			return 0;
		}

		if (!lua_isboolean(L, 2))
		{
			sys_err("Invalid argument: arg2 must be boolean.");
			return 0;
		}

		/*
		* 1 : WEAR_COSTUME_BODY
		* 2 : WEAR_COSTUME_HAIR
		* 3 : WEAR_COSTUME_ACCE
		* 4 : WEAR_COSTUME_WEAPON
		*/
		uint8_t bPartPos = (uint8_t)lua_tonumber(L, 1);
		bool hidePart = lua_toboolean(L, 2);

		if (bPartPos == 1)
			ch->SetBodyCostumeHidden(hidePart);
		else if (bPartPos == 2)
			ch->SetHairCostumeHidden(hidePart);
# ifdef ENABLE_ACCE_COSTUME_SYSTEM
		else if (bPartPos == 3)
			ch->SetAcceCostumeHidden(hidePart);
# endif
# ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		else if (bPartPos == 4)
			ch->SetWeaponCostumeHidden(hidePart);
# endif
# ifdef ENABLE_AURA_SYSTEM
		else if (bPartPos == 5)
			ch->SetAuraCostumeHidden(hidePart);
# endif
		else
		{
			sys_err("Invalid part");
			return 0;
		}

		ch->UpdatePacket();
		return 0;
	}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	ALUA(pc_is_achievement_finished)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1)) {
			sys_err("wrong achievement id");
			return 0;
		}

		lua_pushboolean(L, CAchievementSystem::Instance().IsAchievementFinished(ch, (uint32_t)lua_tonumber(L, 1)) == true);

		return 1;
	}

	ALUA(pc_finish_achievement)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1)) {
			sys_err("wrong achievement id");
			return 0;
		}

		CAchievementSystem::Instance().FinishAchievement(
			ch, (uint32_t)lua_tonumber(L, 1));

		return 0;
	}

	ALUA(pc_finish_achievement_task)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
			sys_err("wrong achievement id or task id");
			return 0;
		}

		CAchievementSystem::Instance().FinishAchievementTask(ch, (uint32_t)lua_tonumber(L, 1), (uint32_t)lua_tonumber(L, 2));

		return 0;
	}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	ALUA(pc_biolog_has_mission)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(ch->GetBiologMissions());
		lua_pushboolean(L, pMission ? true : false);
		return 1;
	}

	ALUA(pc_biolog_get_mission)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetBiologMissions() : -1);

		return 1;
	}

	ALUA(pc_biolog_get_sub_mission)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(ch->GetBiologMissions());
		if (!pMission)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, pMission->bSubMission ? true : false);
		return 1;
	}

	ALUA(pc_biolog_set_mission)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		if (!lua_isnumber(L, 1))
			return 1;

		const auto nextMission = static_cast<uint8_t>(lua_tonumber(L, 1));

		if (ch->GetBiologManager())
		{
			ch->GetBiologManager()->ResetMission();
			ch->SetBiologMissions(nextMission);
		}

		return 1;
	}

	ALUA(pc_biolog_get_cooldown)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, ch ? ch->GetBiologCooldown() - get_global_time() : 0);

		return 1;
	}

	ALUA(pc_biolog_set_cooldown)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		if (!lua_isnumber(L, 1))
			return 1;

		const int newTime = static_cast<int>(lua_tonumber(L, 1));

		ch->SetBiologCooldown(get_global_time() + newTime);
		//CBiologSystem::Instance().SendBiologInformation(true);
		return 1;
	}

	ALUA(pc_biolog_get_mission_npc)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 20084);
			return 1;
		}

		const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(ch->GetBiologMissions());
		if (!pMission)
		{
			lua_pushnumber(L, 20084);
			return 1;
		}

		if (pMission->dwNpcVnum != 0)
		{
			lua_pushnumber(L, pMission->dwNpcVnum);
			return 1;
		}

		lua_pushnumber(L, 20084);
		return 1;
	}

	ALUA(pc_biolog_get_mission_item)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(ch->GetBiologMissions());
		lua_pushnumber(L, pMission ? pMission->iRequiredItem : 0);
		return 1;
	}

	ALUA(pc_biolog_get_sub_mission_item)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(ch->GetBiologMissions());
		if (!pMission)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		if (pMission->bSubMission == true)
		{
			lua_pushnumber(L, pMission->dwRequiredSubItem);
			return 1;
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	ALUA(pc_biolog_get_reward_item)
	{
		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			return 2;
		}

		const TBiologRewardsProto* pReward = CBiologSystemManager::Instance().GetReward(ch->GetBiologMissions());
		if (!pReward)
		{
			sys_err("GiveRewardByMission -> cannot get reward %u by %s", ch->GetBiologMissions(), ch->GetName());
			lua_pushnumber(L, 0);
			lua_pushnumber(L, 0);
			return 2;
		}

		if (pReward->dRewardItem != 0 && pReward->wRewardItemCount != 0)
		{
			lua_pushnumber(L, pReward->dRewardItem);
			lua_pushnumber(L, pReward->wRewardItemCount);
			return 2;
		}

		lua_pushnumber(L, 0);
		lua_pushnumber(L, 0);
		return 2;
	}

	ALUA(pc_biolog_set_reward_bonus)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		const TBiologRewardsProto* pReward = CBiologSystemManager::Instance().GetReward(ch->GetBiologMissions());
		if (!pReward)
		{
			sys_err("SetRewardBonus -> cannot get reward %u by %s", ch->GetBiologMissions(), ch->GetName());
			return 1;
		}

		for (uint8_t i = 0; i < MAX_BONUSES_LENGTH; i++)
		{
			const uint16_t wApply = static_cast<uint16_t>(pReward->wApplyType[i] - 1);
			const int16_t sValue = static_cast<int16_t>(pReward->lApplyValue[i]);
			if (wApply != 0 && sValue != 0)
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[wApply].wPointType, sValue, 0, INFINITE_AFFECT_DURATION, 0, false);
		}

		return 1;
	}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	ALUA(pc_mini_game_catch_king_get_my_score)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		bool isTotal = false;
		if (lua_isboolean(L, 1))
			isTotal = lua_toboolean(L, 1);

		const int iMyScore = CMiniGameManager::Instance().MiniGameCatchKingGetMyScore(ch, isTotal);
		lua_pushnumber(L, iMyScore);
		return 1;
	}
#endif

#ifdef ENABLE_NEWSTUFF
	ALUA(pc_get_killer_drop_pct_custom)
	{
		LPCHARACTER pChar = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!pChar)
		{
			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);
			return 2;
		}

		LPCHARACTER pKiller = pChar->GetQuestNPC();

		int iDeltaPercent, iRandRange;
		if (nullptr == pKiller || !ITEM_MANAGER::Instance().GetDropPctCustom(pKiller, pChar, iDeltaPercent, iRandRange))
		{
			sys_err("pKiller is null");
			lua_pushnumber(L, -1);
			lua_pushnumber(L, -1);

			return 2;
		}

		lua_pushnumber(L, iDeltaPercent);
		lua_pushnumber(L, iRandRange);
		return 2;
	}
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
	ALUA(pc_metinstone_rain_spawn)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, -1);
			return 1;
		}

		const auto dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));

		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, -2);
			return 1;
		}

		const auto x = ch->GetX() + number(-500, 500);
		const auto y = ch->GetY() + number(-500, 500);
		auto pMob = CHARACTER_MANAGER::Instance().SpawnMob(dwVnum, ch->GetMapIndex(), x, y, 0, true, -1);
		if (pMob)
			pMob->SetAggressive();

		lua_pushnumber(L, (pMob) ? pMob->GetVID() : 0);

		return 1;
	}

	ALUA(pc_metinstone_rain_chance)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushboolean(L, false);
			return 1;
		}

		const auto dwVnum = static_cast<uint32_t>(lua_tonumber(L, 1));

		const auto& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		const auto* pkMob = CMobManager::Instance().Get(dwVnum);
		if (!pkMob)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		bool isBeginnerMap = false, isMiddleMap = false, isAdvancedMap = false, isExpertMap = false, isExpertMap2 = false, isExpertMap3 = false, isYoharaMap = false;
		switch (ch->GetMapIndex())
		{
			case 1: // metin2_map_a1
			case 3: // metin2_map_a3
			case 4: // metin2_map_guild_01
			case 5: // metin2_map_monkey_dungeon_11
			case 6: // metin2_guild_village_01
			case 7: // metin2_map_wolf_a1
			case 21: // metin2_map_b1
			case 23: // metin2_map_b3
			case 24: // metin2_map_guild_02
			case 25: // metin2_map_monkey_dungeon_12
			case 26: // metin2_guild_village_02
			case 27: // metin2_map_wolf_b1
			case 41: // metin2_map_c1
			case 43: // metin2_map_c3
			case 44: // metin2_map_guild_03
			case 45: // metin2_map_monkey_dungeon_13
			case 46: // metin2_guild_village_03
			case 47: // metin2_map_wolf_c1
			case 107: // metin2_map_monkey_dungeon
			{
				isBeginnerMap = true;
			}
			break;

			case 63: // metin2_map_n_desert_01
			case 64: // map_n_threeway
			case 65: // metin2_map_milgyo
			case 66: // metin2_map_deviltower1
			case 69: // metin2_map_WL_01
			case 70: // metin2_map_nusluck01
			case 108: // metin2_map_monkey_dungeon2
			{
				isBeginnerMap = true;
				isMiddleMap = true;
			}
			break;

			case 61: // map_n_snowm_01
			case 62: // metin2_map_n_flame_01
			case 67: // metin2_map_trent
			case 68: // metin2_map_trent02
			case 104: // metin2_map_spiderdungeon
			case 71: // metin2_map_spiderdungeon_02
			case 109: // metin2_map_monkey_dungeon3
			{
				isBeginnerMap = true;
				isMiddleMap = true;
				isAdvancedMap = true;
			}
			break;

			case 72: // metin2_map_skipia_dungeon_01
			case 73: // metin2_map_skipia_dungeon_02
			case 208: // metin2_map_skipia_dungeon_boss
			case 216: // metin2_map_devilcatacomb
			case 217: // metin2_map_spiderdungeon_03
			{
				isBeginnerMap = true;
				isMiddleMap = true;
				isAdvancedMap = true;
				isExpertMap = true;
			}
			break;

			case 301: // Metin2_map_CapeDragonHead
			case 302: // metin2_map_dawnmistwood
			case 303: // metin2_map_BayBlackSand
			case 304: // metin2_map_Mt_Thunder
			{
				isBeginnerMap = true;
				isMiddleMap = true;
				isAdvancedMap = true;
				isExpertMap = true;
				isExpertMap2 = true;
			}
			break;

			case 351: // metin2_map_n_flame_dungeon_01
			case 352: // metin2_map_n_snow_dungeon_01
			case 353: // metin2_map_dawnmist_dungeon_01
			case 354: // metin2_map_mt_th_dungeon_01
			case 356: // metin2_map_n_flame_dragon
			case 358: // metin2_map_defensewave
			case 359: // metin2_map_defensewave_port
			{
				isBeginnerMap = true;
				isMiddleMap = true;
				isAdvancedMap = true;
				isExpertMap = true;
				isExpertMap2 = true;
				isExpertMap3 = true;
			}
			break;

			case 373: // metin2_map_eastplain_01
			case 374: // metin2_map_empirecastle
			case 376: // metin2_map_eastplain_02
			case 377: // metin2_map_eastplain_03
			case 382: // metin2_map_maze_dungeon1
			case 323: // metin2_map_maze_dungeon2
			case 384: // metin2_map_maze_dungeon3
			case 386: // metin2_map_smhdungeon_01
			case 387: // metin2_map_smhdungeon_02
			{
				isBeginnerMap = true;
				isMiddleMap = true;
				isAdvancedMap = true;
				isExpertMap = true;
				isExpertMap2 = true;
				isExpertMap3 = true;
				isYoharaMap = true;
			}
			break;

			default:
				break;
		}

		uint8_t bChance = 0;
		switch (dwVnum)
		{
			case 6430: // Bestial Captain
			{
				bChance = isBeginnerMap ? 100 : 30;
			}
			break;

			case 6431: // Chief Orc
			case 6432: // Nine Tails
			{
				bChance = isMiddleMap ? 90 : 30;
			}
			break;
		
			case 6433: // Mighty Ice Witch
			case 6434: // General Huashin
			{
				bChance = isAdvancedMap ? 80 : 30;
			}
			break;

			case 6436: // Beran - Setaou
			case 6437: // Razador
			{
				bChance = isExpertMap ? 65 : 30;
			}
			break;

			case 6435: // Nemere
			{
				if (isExpertMap2)
					bChance = 60;
				else if (isExpertMap3)
					bChance = 65;
				else
					bChance = 30;
			}
			break;

			case 6438: // Jotun Thrym
			{
				bChance = isYoharaMap ? 70 : 0;
			}
			break;

			default:
				break;
		}

		bool bCanSpawn = false;
		const auto ranNumber = static_cast<uint8_t>(number(1, 100));
		if (ranNumber <= bChance)
			bCanSpawn = true;

		lua_pushboolean(L, bCanSpawn);
		return 1;
	}
#endif

#ifdef ENABLE_MOUNT_CHECK
	ALUA(pc_unride)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
			ch->HorseSummon(false);
		}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);
		if (pMount)
		{
			if (pMount->IsEquipped())
				ch->UnequipItem(pMount);
		}
#endif
		return 0;
	}
#endif

#ifdef ENABLE_VIP_SYSTEM
	ALUA(pc_is_vip)
	{
		lua_pushboolean(L, CQuestManager::Instance().GetCurrentCharacterPtr()->GetGMLevel() == GM_VIP);
		return 1;
	}

	ALUA(pc_delete_vip)
	{
		char szQueryDeleteVip[QUERY_MAX_LEN];
		snprintf(szQueryDeleteVip, sizeof(szQueryDeleteVip), "DELETE FROM common.gmlist WHERE mName = '%s'", CQuestManager::Instance().GetCurrentCharacterPtr()->GetName());
		auto pMsg(DBManager::Instance().DirectQuery(szQueryDeleteVip));
		return 0;
	}

	ALUA(pc_set_vip)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (ch->GetGMLevel() >= GM_LOW_WIZARD) {	//@fixme440
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GM_CANT_BE_VIPS"));
			return false;
		}

		char szQueryInsertVip[QUERY_MAX_LEN];
		snprintf(szQueryInsertVip, sizeof(szQueryInsertVip), "INSERT INTO common.gmlist (mAccount, mName, mContactIP, mServerIP, mAuthority) VALUES('%s', '%s', '%s', '%s', '%s')", ch->GetDesc()->GetAccountTable().login, ch->GetName(), "ALL", "ALL", "VIP");
		DBManager::Instance().DirectQuery(szQueryInsertVip);

		db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, nullptr, 0);
		ch->RefreshVip();

		return 0;
	}
#endif

#ifdef ENABLE_AUTO_SYSTEM
	ALUA(pc_autohunt)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		auto msg(DBManager::instance().DirectQuery("UPDATE account.account SET auto_use = DATE_ADD(CURRENT_TIMESTAMP(), INTERVAL 30 DAY) WHERE id = %d;", ch->GetAID()));
		if (msg->uiSQLErrno != 0)
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, 1);

		return 0;
	}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	ALUA(pc_open_lottery)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (!ch || !ch->GetDesc())
			return 0;

		ch->OpenLottoWindow();
		return 1;
	}
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
	ALUA(pc_sung_mahi_curse_hp)
	{
		if (lua_isnumber(L, 1))
		{
			CQuestManager& q = CQuestManager::instance();
			LPCHARACTER ch = q.GetCurrentCharacterPtr();

			if (ch)
			{
				uint8_t bCurseAmount = (uint8_t)lua_tonumber(L, 1);
				if (bCurseAmount < 1 or bCurseAmount > 10)
					return 0;

				ch->PointChange(POINT_HP, -(ch->GetMaxHP() * (10 * bCurseAmount) / 100));
			}
		}
		else
			sys_err("Invalid argument: arg1 must be number.");

		return 0;
	}
#endif

#ifdef ENABLE_MAILBOX
	ALUA(pc_mailbox_reward)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("QUEST give mailbox award call error : wrong argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t dwVnum = (int)lua_tonumber(L, 1);
		const int icount = (int)lua_tonumber(L, 2);

		/*
		 * Lua Args:
		 * 1) Item-Vnum
		 * 2) Item-Count
		 * 3) Stage ID (1)

		 * WriteFromLua Arg:
		 * 1) Playername (who is getting)
		 * 2) Title, Message, ItemVnum, ItemCount
		 * 3) Message
		 * 4) From x Name
		 * 4) ItemVnum
		 * 5) ItemCount
		*/

		stringstream ss;
		ss << (int)lua_tonumber(L, 3);
		std::string floor = ss.str();

		std::string title = /*locale_string = 1653*/"Sung Mahi Tower Level ";
		title += floor;

		CMailBox* mail = ch->GetMailBox();
		mail->WriteFromLua(ch->GetName(), title.c_str(), "", "Sung Mahi Tower", dwVnum, icount);
		sys_log(0, "QUEST [mailbox award] item %d to login %s", dwVnum, ch->GetDesc()->GetAccountTable().login);
		return 1;
	}
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
	ALUA(pc_give_medal)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("QUEST : wrong argument");
			return 0;
		}

		int iAmount = (int)lua_tonumber(L, 1);

		if (iAmount <= 0)
		{
			sys_err("QUEST : medal amount less then zero");
			return 0;
		}

		ch->PointChange(POINT_MEDAL_OF_HONOR, iAmount, true);
		return 0;
	}
#endif

	void RegisterPCFunctionTable()
	{
		luaL_reg pc_functions[] =
		{
			{ "get_wear",		pc_get_wear			},
			{ "get_player_id",	pc_get_player_id	},
			{ "get_account_id", pc_get_account_id	},
			{ "get_account",	pc_get_account		},
			{ "get_level",		pc_get_level		},
			{ "set_level",		pc_set_level		},
			{ "get_next_exp",		pc_get_next_exp		},
			{ "get_exp",		pc_get_exp		},
			{ "get_job",		pc_get_job		},
			{ "get_race",		pc_get_race		},
			{ "change_sex",		pc_change_sex	},
			{ "gethp",			pc_get_hp		},
			{ "get_hp",			pc_get_hp		},
			{ "getmaxhp",		pc_get_max_hp		},
			{ "get_max_hp",		pc_get_max_hp		},
			{ "getsp",			pc_get_sp		},
			{ "get_sp",			pc_get_sp		},
			{ "getmaxsp",		pc_get_max_sp		},
			{ "get_max_sp",		pc_get_max_sp		},
			{ "change_sp",		pc_change_sp		},
			{ "getmoney",		pc_get_money		},
			{ "get_money",		pc_get_money		},
			{ "get_real_alignment",	pc_get_real_alignment	},
			{ "get_alignment",		pc_get_alignment	},
			{ "getweapon",		pc_get_weapon		},
			{ "get_weapon",		pc_get_weapon		},
			{ "getarmor",		pc_get_armor		},
			{ "get_armor",		pc_get_armor		},
			{ "getgold",		pc_get_money		},
			{ "get_gold",		pc_get_money		},
#ifdef ENABLE_CHEQUE_SYSTEM
			{ "set_cheque",		pc_set_cheque		},
			{ "get_cheque",		pc_get_cheque		},
#endif
			{ "changegold",		pc_change_money		},
			{ "changemoney",		pc_change_money		},
			{ "changealignment",	pc_change_alignment	},
			{ "change_gold",		pc_change_money		},
			{ "change_money",		pc_change_money		},
			{ "change_alignment",	pc_change_alignment	},
			{ "getname",		pc_get_name		},
			{ "get_name",		pc_get_name		},
			{ "get_vid",		pc_get_vid		},
			{ "getplaytime",		pc_get_playtime		},
			{ "get_playtime",		pc_get_playtime		},
			{ "getleadership",		pc_get_leadership	},
			{ "get_leadership",		pc_get_leadership	},
#ifdef ENABLE_NEWSTUFF
			{ "getf2",			pc_get_flag	},
			{ "setf2",			pc_set_flag	},
#endif
			{ "getqf",			pc_get_quest_flag	},
			{ "setqf",			pc_set_quest_flag	},
			{ "delqf",			pc_del_quest_flag	},
			{ "getf",			pc_get_another_quest_flag},
			{ "setf",			pc_set_another_quest_flag},
			{ "get_x",			pc_get_x		},
			{ "get_y",			pc_get_y		},
			{ "getx",			pc_get_x		},
			{ "gety",			pc_get_y		},
			{ "get_local_x",		pc_get_local_x		},
			{ "get_local_y",		pc_get_local_y		},
			{ "getcurrentmapindex",	pc_get_current_map_index},
			{ "get_map_index",		pc_get_current_map_index},
			{ "give_exp",		pc_give_exp		},
			{ "give_exp_perc",		pc_give_exp_perc	},
			{ "get_exp_perc",	pc_get_exp_perc		},
			{ "give_exp2",		pc_give_exp2		},
			{ "give_item",		pc_give_item		},
			{ "give_item2",		pc_give_or_drop_item	},
#ifdef ENABLE_DICE_SYSTEM
			{ "give_item2_with_dice",	pc_give_or_drop_item_with_dice	},
#endif
			{ "give_item2_select",		pc_give_or_drop_item_and_select	},
			{ "give_gold",		pc_give_gold		},
			{ "count_item",		pc_count_item		},
			{ "remove_item",		pc_remove_item		},
			{ "countitem",		pc_count_item		},
			{ "removeitem",		pc_remove_item		},
			{ "find_first_item", pc_find_first_item },
			{ "reset_point",		pc_reset_point		},
			{ "has_guild",		pc_hasguild		},
			{ "hasguild",		pc_hasguild		},
			{ "get_guild",		pc_getguild		},
			{ "getguild",		pc_getguild		},
			{ "isguildmaster",		pc_isguildmaster	},
			{ "is_guild_master",	pc_isguildmaster	},
			{ "destroy_guild",		pc_destroy_guild	},
			{ "remove_from_guild",	pc_remove_from_guild	},
			{ "in_dungeon",		pc_in_dungeon		},
			{ "getempire",		pc_get_empire		},
			{ "get_empire",		pc_get_empire		},
			{ "get_skill_group",	pc_get_skillgroup	},
			{ "set_skill_group",	pc_set_skillgroup	},
			{ "warp",			pc_warp			},
			{ "warp_local",		pc_warp_local		},
			{ "warp_exit",		pc_warp_exit		},
			{ "set_warp_location",	pc_set_warp_location	},
			{ "set_warp_location_local",pc_set_warp_location_local },
			{ "get_start_location",	pc_get_start_location	},
			{ "has_master_skill",	pc_has_master_skill	},
			{ "set_part",		pc_set_part		},
			{ "get_part",		pc_get_part		},
			{ "is_polymorphed",		pc_is_polymorphed	},
			{ "remove_polymorph",	pc_remove_polymorph	},
			{ "is_mount",		pc_is_mount		},
			{ "polymorph",		pc_polymorph		},
			{ "mount",			pc_mount		},
			{ "mount_bonus",	pc_mount_bonus	},
			{ "unmount",		pc_unmount		},
			{ "warp_to_guild_war_observer_position", pc_warp_to_guild_war_observer_position	},
			{ "give_item_from_special_item_group", pc_give_item_from_special_item_group	},
			{ "learn_grand_master_skill", pc_learn_grand_master_skill	},
			{ "is_skill_book_no_delay",	pc_is_skill_book_no_delay},
			{ "remove_skill_book_no_delay",	pc_remove_skill_book_no_delay},

			{ "enough_inventory",	pc_enough_inventory	},
			{ "get_horse_level",	pc_get_horse_level	}, // TO BE DELETED XXX
			{ "is_horse_alive",		pc_is_horse_alive	}, // TO BE DELETED XXX
			{ "revive_horse",		pc_revive_horse		}, // TO BE DELETED XXX
			{ "have_pos_scroll",	pc_have_pos_scroll	},
			{ "have_map_scroll",	pc_have_map_scroll	},
			{ "get_war_map",		pc_get_war_map		},
			{ "get_equip_refine_level",	pc_get_equip_refine_level },
			{ "refine_equip",		pc_refine_equip		},
			{ "get_skill_level",	pc_get_skill_level	},
			{ "give_lotto",		pc_give_lotto		},
			{ "aggregate_monster",	pc_aggregate_monster	},
			{ "forget_my_attacker",	pc_forget_my_attacker	},
			{ "pc_attract_ranger",	pc_attract_ranger	},
			{ "select",			pc_select_vid		},
			{ "get_sex",		pc_get_sex		},
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			{ "get_costume",	pc_get_costume },
#endif
			{ "is_married",		pc_is_married		},
			{ "is_engaged",		pc_is_engaged		},
			{ "is_engaged_or_married",	pc_is_engaged_or_married},
			{ "is_gm",			pc_is_gm		},
			{ "get_gm_level",		pc_get_gm_level		},
			{ "mining",			pc_mining		},
			{ "ore_refine",		pc_ore_refine		},
			{ "diamond_refine",		pc_diamond_refine	},

			// RESET_ONE_SKILL
			{ "clear_one_skill",        pc_clear_one_skill      },
			// END_RESET_ONE_SKILL

			{ "clear_skill",                pc_clear_skill          },
			{ "clear_sub_skill",    pc_clear_sub_skill      },
			{ "set_skill_point",    pc_set_skill_point      },

			{ "is_clear_skill_group",	pc_is_clear_skill_group		},

			{ "save_exit_location",		pc_save_exit_location		},
			{ "teleport",				pc_teleport },

			{ "set_skill_level",        pc_set_skill_level      },

            { "give_polymorph_book",    pc_give_polymorph_book  },
            { "upgrade_polymorph_book", pc_upgrade_polymorph_book },
            { "get_premium_remain_sec", pc_get_premium_remain_sec },

			{ "send_block_mode",		pc_send_block_mode	},

			{ "change_empire",			pc_change_empire	},
			{ "get_change_empire_count",	pc_get_change_empire_count	},
			{ "set_change_empire_count",	pc_set_change_empire_count	},

			{ "change_name",			pc_change_name },

			{ "is_dead",				pc_is_dead	},

			{ "reset_status",		pc_reset_status	},

			{ "get_ht",				pc_get_ht	},
			{ "set_ht",				pc_set_ht	},
			{ "get_iq",				pc_get_iq	},
			{ "set_iq",				pc_set_iq	},
			{ "get_st",				pc_get_st	},
			{ "set_st",				pc_set_st	},
			{ "get_dx",				pc_get_dx	},
			{ "set_dx",				pc_set_dx	},

			{ "is_near_vid",		pc_is_near_vid	},

			{ "get_socket_items",	pc_get_socket_items	},
			{ "get_empty_inventory_count",	pc_get_empty_inventory_count	},

			{ "get_logoff_interval",	pc_get_logoff_interval	},

			{ "is_riding",			pc_is_riding	},
			{ "get_special_ride_vnum",	pc_get_special_ride_vnum	},

			{ "can_warp",			pc_can_warp		},

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
			{ "open_gaya_shop",			pc_open_gem_shop },
			{ "create_gaya",			pc_create_gaya },
#endif

			{ "dec_skill_point",	pc_dec_skill_point	},
			{ "get_skill_point",	pc_get_skill_point	},

			{ "get_channel_id",		pc_get_channel_id	},

			{ "give_poly_marble",	pc_give_poly_marble	},
			{ "get_sig_items",		pc_get_sig_items	},

			{ "charge_cash",		pc_charge_cash		},

			{ "get_informer_type",	pc_get_informer_type	},	//µ¶ÀÏ ¼±¹° ±â´É
			{ "get_informer_item",  pc_get_informer_item	},

			{ "give_award",			pc_give_award			},	//ÀÏº» °èÁ¤´ç ÇÑ¹ø¾¿ ±Ý±« Áö±Þ
			{ "give_award_socket",	pc_give_award_socket	},	//¸ô ÀÎº¥Åä¸®¿¡ ¾ÆÀÌÅÛ Áö±Þ. ¼ÒÄÏ ¼³Á¤À» À§ÇÑ ÇÔ¼ö.

			{ "get_killee_drop_pct",	pc_get_killee_drop_pct	}, /* mob_vnum.kill ÀÌº¥Æ®¿¡¼­ killee¿Í pc¿ÍÀÇ level Â÷ÀÌ, pcÀÇ ÇÁ¸®¹Ì¾ö µå¶ø·ü µîµîÀ» °í·ÁÇÑ ¾ÆÀÌÅÛ µå¶ø È®·ü.
																    * return °ªÀº (ºÐÀÚ, ºÐ¸ð).
																    * (¸»ÀÌ º¹ÀâÇÑµ¥, CreateDropItemÀÇ GetDropPctÀÇ iDeltaPercent, iRandRange¸¦ returnÇÑ´Ù°í º¸¸é µÊ.)
																	* (ÀÌ ¸»ÀÌ ´õ ¾î·Á¿ï¶ó³ª ¤Ð¤Ð)
																	* ÁÖÀÇ»çÇ× : kill event¿¡¼­¸¸ »ç¿ëÇÒ °Í!
																	*/

#ifdef ENABLE_NEWSTUFF
			//pc.set_race0(race=[0. Warrior, 1. Ninja, 2. Sura, 3. Shaman, 4. Lycan])
			{ "set_race0",			pc_set_race0			},
			//if pc.delf("game_option", "block_cocks") then syschat("now you are unsafe") end
			{ "delf",				pc_del_another_quest_flag},	// delete quest flag [return lua boolean: successfulness]
			//pc.make_item0({vnum|locale_name, count}, {socket1,2,3}, {type1, value1, ... , type7, value7}, gstate(=0: giveitem, 1: dropitem, 2: drop_item_with_leadership)[, countdown_in_secs_before_ownership_vanish(=if <=10 would be 30; default=10->30)])
			{ "make_item0",			pc_make_item0		},	// [return lua boolean: successfulness]
			//pc.pointchange(uint type, int amount, bool bAmount, bool bBroadcast)
			{ "pointchange",		pc_pointchange		},	// [return nothing]
			//pc.pullmob()
			{ "pullmob",			pc_pullmob			},	// [return nothing]
			//pc.select_pid(pc.get_player_id())
			{ "select_pid",			pc_select_pid		},	// [return lua number: old pid]
			//pc.select renamed in pc.select_vid
			{ "select_vid",			pc_select_vid		},	// [return lua number: old vid]
			//pc.set_level0(level)
			{ "set_level0",			pc_set_level0		},	// [return nothing]
			//pc.set_gm_level(); instead of `/reload a` (note: this only refresh gm privileges if you already are gm on common.gm[host|list])
			{ "set_gm_level",		pc_set_gm_level		},	// [return nothing]
			//is_flags (void) [return lua boolean]
			{ "is_flag_fire",			pc_if_fire			},
			{ "is_flag_invisible",		pc_if_invisible		},
			{ "is_flag_poison",			pc_if_poison		},
#ifdef ENABLE_WOLFMAN_CHARACTER
			{ "is_flag_bleeding",		pc_if_bleeding		},
#endif
			{ "is_flag_slow",			pc_if_slow			},
			{ "is_flag_stun",			pc_if_stun			},
			//set_flags (bool) [return nothing]
			{ "set_flag_fire",			pc_sf_fire			},
			{ "set_flag_invisible",		pc_sf_invisible		},
			{ "set_flag_poison",		pc_sf_poison		},
#ifdef ENABLE_WOLFMAN_CHARACTER
			{ "set_flag_bleeding",		pc_sf_bleeding		},
#endif
			{ "set_flag_slow",			pc_sf_slow			},
			{ "set_flag_stun",			pc_sf_stun			},
			//pc.set_flag_kill(char_name) [return nothing]
			{ "set_flag_kill",			pc_sf_kill			},
			//pc.set_flag_dead()
			{ "set_flag_dead",			pc_sf_dead			},	// kill themselves [return nothing]
			//pc.get_exp_level(level) [return: lua number]
			{ "get_exp_level",		pc_get_exp_level	},	// get needed exp for <level> level [return: lua number]
			//pc.get_exp_level(level, perc)
			{ "get_exp_level0",		pc_get_exp_level0	},	// get needed exp for <level> level / 100 * perc [return: lua number]
			//pc.set_max_health()
			{ "set_max_health",		pc_set_max_health	},	// [return nothing]
			{ "get_ip0",			pc_get_ip0			},	// [return lua string]
			{ "get_client_version0",	pc_get_client_version0	},	// get player client version [return lua string]
			{ "dc_delayed0",		pc_dc_delayed0	},	// crash a player after x secs [return lua boolean: successfulness]
			{ "dc_direct0",			pc_dc_direct0	},	// crash the <nick> player [return nothing]
			{ "is_trade0",			pc_is_trade0	},	// get if player is trading [return lua boolean]
			{ "is_busy0",			pc_is_busy0		},	// get if player is "busy" (if trade, safebox, npc/myshop, cube are open) [return lua boolean]
			{ "is_arena0",			pc_is_arena0		},	// get if player is in arena [return lua boolean]
			{ "is_arena_observer0",	pc_is_arena_observer0		},	// get if player is in arena as observer [return lua boolean]
			// pc.equip_slot0(cell)
			{ "equip_slot0",		pc_equip_slot0		},	// [return lua boolean: successfulness]
			// pc.unequip_slot0(cell)
			{ "unequip_slot0",		pc_unequip_slot0	},	// [return lua boolean: successfulness]
			{ "is_available0",		pc_is_available0	},	// [return lua boolean]
			{ "give_random_book0",	pc_give_random_book0},	// [return lua boolean]
			{ "is_pvp0",			pc_is_pvp0			},	// [return lua boolean]
			{ "set_safebox_delivered", pc_set_safebox_delivered },
#endif
#ifdef ENABLE_PC_OPENSHOP
			//pc.open_shop0(id_shop)
			{ "open_shop0",			pc_open_shop0		},	// buy/sell won't work on it [return nothing]
#endif
#ifdef ENABLE_NEWGUILDMAKE
			//pc.make_guild0(guild_name)
			{ "make_guild0",			pc_make_guild0	},	// it returns few state values which you can manage via lua [return lua number]
#endif
			{ "send_effect",		pc_specific_effect		},
			{ "start_duel",			pc_start_duel			}, // Force a duel with a player 
			{ "stun", pc_stun_player}, // Stun player for more details read the comments above.
			{ "reencode_view", pc_reencode_view },
#ifdef ENABLE_VIP_SYSTEM
			{ "is_vip", pc_is_vip },
			{ "delete_vip", pc_delete_vip },
			{ "set_vip", pc_set_vip },
#endif
#ifdef ENABLE_12ZI
			{ "set_animasphere",			pc_set_animasphere },
			{ "get_animasphere",			pc_get_animasphere },
			{ "delete_animasphere",			pc_delete_animasphere },
			{ "is_flag_cz_ulimit_enter",	pc_if_cz_unlimit_enter },
			{ "set_flag_cz_ulimit_enter",	pc_sf_cz_unlimit_enter },
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
			{ "update_dungeon_rank",		pc_update_dungeon_rank },
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
			{ "open_soul_roulette",		pc_open_soul_roulette },
			{ "get_soul",				pc_get_soul },
			{ "change_soul",			pc_change_soul },
			{ "get_soul_re",			pc_get_soul_re },
			{ "change_soul_re",			pc_change_soul_re },
			{ "get_soul_global_rank",	pc_get_soul_global_rank },
#endif

#ifdef ENABLE_YOHARA_SYSTEM
			{ "conqueror_reset_status",				pc_conqueror_reset_status },

			{ "get_cht",			pc_get_cht },
			{ "set_cht",			pc_set_cht },
			{ "get_cimmun",			pc_get_cimmun },
			{ "set_cimmun",			pc_set_cimmun },
			{ "get_cst",			pc_get_cst },
			{ "set_cst",			pc_set_cst },
			{ "get_cmove",			pc_get_cmove },
			{ "set_cmove",			pc_set_cmove },

			{ "set_conqueror_level",	pc_set_conqueror_level },
			{ "get_conqueror_level",	pc_get_conqueror_level },
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			{ "open_acce_absorption", pc_open_acce_absorption },
			{ "open_acce_combination", pc_open_acce_combination },
#endif
#ifdef ENABLE_678TH_SKILL
			{ "get_678th_skill_points", pc_get_678th_skill_points },
			{ "set_678th_skill_points", pc_set_678th_skill_points },
			{ "give_skill_points", pc_give_skill_points },
#endif
#ifdef ENABLE_MESSENGER_BLOCK
			{ "is_blocked", 		pc_is_blocked },
			{ "is_friend", 			pc_is_friend },
#endif

#ifdef ENABLE_HWID_BAN
			{ "get_hwid",			pc_get_hwid },
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
			{ "get_okay_global_rank",	pc_get_okay_global_rank },
			{ "get_okay_rund_rank",	pc_get_okay_rund_rank },
#endif

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
			{ "hide_costume", pc_hide_costume },
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
			{ "is_achievement_finished", pc_is_achievement_finished },
			{ "finish_achievement", pc_finish_achievement },
			{ "finish_achievement_task", pc_finish_achievement_task },
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
			{ "biolog_has_mission", pc_biolog_has_mission },
			{ "biolog_get_mission", pc_biolog_get_mission },
			{ "biolog_get_sub_mission", pc_biolog_get_sub_mission },
			{ "biolog_set_mission", pc_biolog_set_mission },
			{ "biolog_get_cooldown", pc_biolog_get_cooldown },
			{ "biolog_set_cooldown", pc_biolog_set_cooldown },
			{ "biolog_get_mission_npc", pc_biolog_get_mission_npc },
			{ "biolog_get_mission_item", pc_biolog_get_mission_item },
			{ "biolog_get_sub_mission_item", pc_biolog_get_sub_mission_item },
			{ "biolog_get_reward_item", pc_biolog_get_reward_item },
			{ "biolog_set_reward_bonus", pc_biolog_set_reward_bonus },
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
			{ "mini_game_catch_king_get_my_score", pc_mini_game_catch_king_get_my_score },
#endif
#ifdef ENABLE_NEWSTUFF
			{ "get_killer_drop_pct_custom", pc_get_killer_drop_pct_custom },
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
			{ "metinstone_rain_spawn", pc_metinstone_rain_spawn },
			{ "metinstone_rain_chance", pc_metinstone_rain_chance },
#endif
#ifdef ENABLE_AUTO_SYSTEM
			{ "autohunt",pc_autohunt },
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
			{ "open_lottery",			pc_open_lottery },
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
			{ "sung_mahi_curse_hp", pc_sung_mahi_curse_hp },
#endif

#ifdef ENABLE_MAILBOX
			{ "mailbox_reward", pc_mailbox_reward },
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
			{ "give_medal", pc_give_medal },
#endif

#ifdef ENABLE_NEWSTUFF_2017
			{ "give_and_equip0", pc_give_and_equip_item },
			{ "reset_change_empire", pc_reset_change_empire },
			{ "move_to", pc_move_to },

				// Nyx 02.07.2018
			{ "is_equip_unique_item", pc_is_equip_unique_item },

			{ "send_command", pc_send_command },

			{ "get_move_pos_x", pc_get_move_pos_x },
			{ "get_move_pos_y", pc_get_move_pos_y },
#endif

			{ nullptr,			nullptr			}
		};

		CQuestManager::Instance().AddLuaFunctionTable("pc", pc_functions);
	}
};
