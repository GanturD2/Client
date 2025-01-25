
#include "stdafx.h"

#include <sstream>

#include "questmanager.h"
#include "questlua.h"
#include "config.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "buffer_manager.h"
#include "db.h"
#include "xmas_event.h"
#include "locale_service.h"
#include "regen.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "sectree_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	using namespace std;

	string ScriptToString(const string& str)
	{
		lua_State* L = CQuestManager::Instance().GetLuaState();
		const int x = lua_gettop(L);

		int errcode = lua_dobuffer(L, ("return " + str).c_str(), str.size() + 7, "ScriptToString");
		string retstr;
		if (!errcode)
		{
			if (lua_isstring(L, -1))
				retstr = lua_tostring(L, -1);
		}
		else
		{
			sys_err("LUA ScriptRunError (code:%d src:[%s])", errcode, str.c_str());
		}
		lua_settop(L, x);
		return retstr;
	}

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	void FUpdateDungeonRanking::operator() (LPCHARACTER ch)
	{
		if (ch->IsPC() && ch->GetDesc())
		{
			ch->UpdateDungeonRanking(m_strQuestName);
		}
	}
#endif

	void FSetWarpLocation::operator() (LPCHARACTER ch)
	{
		if (ch && ch->IsPC())
			ch->SetWarpLocation(map_index, x, y);
	}

	void FDungeonGiveItemAll::operator() (LPCHARACTER ch)
	{
		if (ch->IsPC())
			ch->AutoGiveItem(vnum, count);
	}

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	void FDungeonUpdateAllBattlepassProcess::operator() (LPCHARACTER ch)
	{
		if (ch->IsPC())
			ch->UpdateExtBattlePassMissionProgress(COMPLETE_DUNGEON, 1, dungeon_index);
	}
#endif

	void FSetQuestFlag::operator() (LPCHARACTER ch)
	{
		if (!ch)
			return;

		if (!ch->IsPC())
			return;

		PC* pPC = CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
		if (pPC)
			pPC->SetFlag(flagname, value);
	}

	bool FPartyCheckFlagLt::operator() (LPCHARACTER ch)
	{
		if (!ch)
			return false;

		if (!ch->IsPC())
			return false;

		PC* pPC = CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
		bool returnBool = false;
		if (pPC)
		{
			const int flagValue = pPC->GetFlag(flagname);
			returnBool = value > flagValue ? true : false;
		}

		return returnBool;
	}

	FPartyChat::FPartyChat(int ChatType, const char* str) : iChatType(ChatType), str(str)
	{
	}

	void FPartyChat::operator() (LPCHARACTER ch)
	{
		if (ch)
			ch->ChatPacket(iChatType, "%s", str);
	}

	void FPartyClearReady::operator() (LPCHARACTER ch)
	{
		if (ch)
			ch->RemoveAffect(AFFECT_DUNGEON_READY);
	}

	void FSendPacket::operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			const LPCHARACTER& ch = dynamic_cast<LPCHARACTER>(ent);
			if (ch && ch->GetDesc())
				ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
	}

#ifdef ENABLE_NEWSTUFF
	void FSendChatPacket::operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (ch)
				ch->ChatPacket(m_chat_type, "%s", m_text.c_str());
		}
	}
#endif

	void FSendPacketToEmpire::operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			const LPCHARACTER& ch = dynamic_cast<LPCHARACTER>(ent);

			if (ch && ch->GetDesc())
			{
				if (ch->GetEmpire() == bEmpire)
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
		}
	}

	void FWarpEmpire::operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (ch && ch->IsPC() && ch->GetEmpire() == m_bEmpire)
				ch->WarpSet(m_x, m_y, m_lMapIndexTo);
		}
	}

	FBuildLuaGuildWarList::FBuildLuaGuildWarList(lua_State* lua_state) : L(lua_state), m_count(1)
	{
		lua_newtable(lua_state);
	}

	void FBuildLuaGuildWarList::operator() (uint32_t g1, uint32_t g2)
	{
		CGuild* g = CGuildManager::Instance().FindGuild(g1);

		if (!g)
			return;

		if (g->GetGuildWarType(g2) == GUILD_WAR_TYPE_FIELD)
			return;

		if (g->GetGuildWarState(g2) != GUILD_WAR_ON_WAR)
			return;

		lua_newtable(L);
		lua_pushnumber(L, g1);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, g2);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, m_count++);
	}

	bool IsScriptTrue(const char* code, int size)
	{
		if (size == 0)
			return true;

		lua_State* L = CQuestManager::Instance().GetLuaState();
		const int x = lua_gettop(L);
		const int errcode = lua_dobuffer(L, code, size, "IsScriptTrue");
		const int bStart = lua_toboolean(L, -1);
		if (errcode)
		{
			char buf[100];
			snprintf(buf, sizeof(buf), "LUA ScriptRunError (code:%%d src:[%%%ds])", size);
			sys_err(buf, errcode, code);
		}
		lua_settop(L, x);
		return bStart != 0;
	}

	void combine_lua_string(lua_State* L, ostringstream& s)
	{
		char buf[32];

		const int n = lua_gettop(L);
		int i;

		for (i = 1; i <= n; ++i)
		{
			if (lua_isstring(L, i))
				//printf("%s\n",lua_tostring(L,i));
				s << lua_tostring(L, i);
			else if (lua_isnumber(L, i))
			{
				snprintf(buf, sizeof(buf), "%.14g\n", lua_tonumber(L, i));
				s << buf;
			}
		}
	}

	ALUA(highscore_show)
	{
		CQuestManager& q = CQuestManager::Instance();
		const char* pszBoardName = lua_tostring(L, 1);
		const uint32_t mypid = q.GetCurrentCharacterPtr()->GetPlayerID();
		const bool bOrder = static_cast<int>(lua_tonumber(L, 2)) != 0 ? true : false;

		DBManager::Instance().ReturnQuery(QID_HIGHSCORE_SHOW, mypid, nullptr,
			"SELECT h.pid, p.name, h.value FROM highscore%s as h, player%s as p WHERE h.board = '%s' AND h.pid = p.id ORDER BY h.value %s LIMIT 10",
			get_table_postfix(), get_table_postfix(), pszBoardName, bOrder ? "DESC" : "");
		return 0;
	}

	ALUA(highscore_register)
	{
		CQuestManager& q = CQuestManager::Instance();

		THighscoreRegisterQueryInfo* qi = M2_NEW THighscoreRegisterQueryInfo;

		strlcpy(qi->szBoard, lua_tostring(L, 1), sizeof(qi->szBoard));
		qi->dwPID = q.GetCurrentCharacterPtr()->GetPlayerID();
		qi->iValue = static_cast<int>(lua_tonumber(L, 2));
		qi->bOrder = static_cast<int>(lua_tonumber(L, 3));

		DBManager::Instance().ReturnQuery(QID_HIGHSCORE_REGISTER, qi->dwPID, qi,
			"SELECT value FROM highscore%s WHERE board='%s' AND pid=%u", get_table_postfix(), qi->szBoard, qi->dwPID);
		return 1;
	}

	//
	// "member" Lua functions
	//
	ALUA(member_chat)
	{
		ostringstream s;
		combine_lua_string(L, s);
		CQuestManager::Instance().GetCurrentPartyMember()->ChatPacket(CHAT_TYPE_TALKING, "%s", s.str().c_str());
		return 0;
	}

	ALUA(member_clear_ready)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentPartyMember();
		if (ch)
			ch->RemoveAffect(AFFECT_DUNGEON_READY);
		return 0;
	}

	ALUA(member_set_ready)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentPartyMember();
		if (ch)
			ch->AddAffect(AFFECT_DUNGEON_READY, POINT_NONE, 0, AFF_DUNGEON_READY, 65535, 0, true);
		return 0;
	}

	ALUA(mob_spawn)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		const uint32_t mob_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const long local_x = static_cast<long>(lua_tonumber(L, 2)) * 100;
		const long local_y = static_cast<long>(lua_tonumber(L, 3)) * 100;
		const float radius = static_cast<float>(lua_tonumber(L, 4)) * 100;
		const bool bAggressive = lua_toboolean(L, 5);
		uint32_t count = (lua_isnumber(L, 6)) ? (uint32_t)lua_tonumber(L, 6) : 1;
		bool noReward = lua_toboolean(L, 7);	//@fixme000
		const char* mobName = lua_tostring(L, 8);	//@fixme000

		if (count == 0)
			count = 1;
		else if (count > 10)
		{
			sys_err("count bigger than 10");
			count = 10;
		}

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		const SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (pMap == nullptr)
			return 0;

		const uint32_t dwQuestIdx = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = nullptr;

		while (count--)
		{
			for (int loop = 0; loop < 8; ++loop)
			{
				const float angle = number(0, 999) * static_cast<float>(M_PI) * 2 / 1000;
				const float r = number(0, 999) * radius / 1000;

				const long x = local_x + pMap->m_setting.iBaseX + static_cast<long>(r * cos(angle));
				const long y = local_y + pMap->m_setting.iBaseY + static_cast<long>(r * sin(angle));

				mob = CHARACTER_MANAGER::Instance().SpawnMob(mob_vnum, ch->GetMapIndex(), x, y, 0);

				if (mob)
					break;
			}

			if (mob)
			{
				if (bAggressive)
					mob->SetAggressive();

				if (noReward)
					mob->SetNoRewardFlag();

				if (mobName)
					mob->SetName(mobName);

				mob->SetQuestBy(dwQuestIdx);

				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (uint32_t)mob->GetVID());
				}
			}
		}

		if (!ret)
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(mob_spawn_group)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 6))
		{
			sys_err("invalid argument");
			lua_pushnumber(L, 0);
			return 1;
		}

		const uint32_t group_vnum = static_cast<uint32_t>(lua_tonumber(L, 1));
		const long local_x = static_cast<long>(lua_tonumber(L, 2)) * 100;
		const long local_y = static_cast<long>(lua_tonumber(L, 3)) * 100;
		const float radius = static_cast<float>(lua_tonumber(L, 4)) * 100;
		const bool bAggressive = lua_toboolean(L, 5);
		uint32_t count = static_cast<uint32_t>(lua_tonumber(L, 6));

		if (count == 0)
			count = 1;
		else if (count > 10)
		{
			sys_err("count bigger than 10");
			count = 10;
		}

		const LPCHARACTER& ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		const SECTREE_MAP* pMap = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());
		if (pMap == nullptr)
		{
			lua_pushnumber(L, 0);
			return 1;
		}
		const uint32_t dwQuestIdx = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		bool ret = false;
		LPCHARACTER mob = nullptr;

		while (count--)
		{
			for (int loop = 0; loop < 8; ++loop)
			{
				const float angle = number(0, 999) * static_cast<float>(M_PI) * 2 / 1000;
				const float r = number(0, 999) * radius / 1000;

				const long x = local_x + pMap->m_setting.iBaseX + static_cast<long>(r * cos(angle));
				const long y = local_y + pMap->m_setting.iBaseY + static_cast<long>(r * sin(angle));

				mob = CHARACTER_MANAGER::Instance().SpawnGroup(group_vnum, ch->GetMapIndex(), x, y, x, y, nullptr, bAggressive);

				if (mob)
					break;
			}

			if (mob)
			{
				mob->SetQuestBy(dwQuestIdx);

				if (!ret)
				{
					ret = true;
					lua_pushnumber(L, (uint32_t)mob->GetVID());
				}
			}
		}

		if (!ret)
			lua_pushnumber(L, 0);

		return 1;
	}

#ifdef ENABLE_NEWSTUFF_2019
	ALUA(mob_spawn_random)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t mob_vnum = (uint32_t)lua_tonumber(L, 1);
		long map_index = (long)lua_tonumber(L, 2);

		LPCHARACTER mob = CHARACTER_MANAGER::Instance().SpawnMobRandomPosition(mob_vnum, map_index);
		if (!mob)
			return 0;

		return 1;
	}
#endif

	//
	// global Lua functions
	//
	//
	// Registers Lua function table
	//
	void CQuestManager::AddLuaFunctionTable(const char* c_pszName, luaL_reg* preg, bool bCheckIfExists)
	{
#ifdef ENABLE_NEWSTUFF
		bool bIsExists = false;
		if (bCheckIfExists)
		{
			const int x = lua_gettop(L);
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				lua_settop(L, x);
				bIsExists = true;
			}
		}
		if (!bIsExists)
			lua_newtable(L);
#else
		lua_newtable(L);
#endif

		while ((preg->name))
		{
			lua_pushstring(L, preg->name);
			lua_pushcfunction(L, preg->func);
			lua_rawset(L, -3);
			preg++;
		}

		lua_setglobal(L, c_pszName);
	}

	void CQuestManager::AddLuaFunctionSubTable(const char* c_pszName, const char* c_pszSubName, luaL_reg* preg)
	{
		// lua_State* L = CQuestManager::Instance().GetLuaState();
		const int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				sys_err("%s global index not found for %s", c_pszName, c_pszSubName);
				lua_settop(L, x);
				return;
			}
			lua_pushstring(L, c_pszSubName);
			{
				lua_newtable(L);
				while ((preg->name))
				{
					lua_pushstring(L, preg->name);
					lua_pushcfunction(L, preg->func);
					lua_rawset(L, -3);
					preg++;
				}
			}
			lua_rawset(L, -3);
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

#ifdef ENABLE_NEWSTUFF
	void CQuestManager::AppendLuaFunctionTable(const char* c_pszName, luaL_reg* preg, bool bForceCreation)
	{
		const int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				sys_err("%s global index not found (force=%d)", c_pszName, bForceCreation);
				lua_settop(L, x);
				if (bForceCreation)
					AddLuaFunctionTable(c_pszName, preg);
				return;
			}

			while ((preg->name))
			{
				lua_pushstring(L, preg->name);
				lua_pushcfunction(L, preg->func);
				lua_rawset(L, -3);
				preg++;
			}

			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantGlobal(const char* c_pszName, lua_Number lNumber, bool bOverwrite)
	{
		const int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (lua_isnumber(L, -1))
			{
				if (!bOverwrite)
				{
					sys_err("%s global index already defined", c_pszName);
					lua_settop(L, x);
					return;
				}
			}
			lua_pushnumber(L, lNumber);
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantInTable(const char* c_pszName, const char* c_pszSubName, lua_Number lNumber, bool bForceCreation)
	{
		const int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				if (!bForceCreation)
				{
					sys_err("%s global index for %s already defined", c_pszName, c_pszSubName);
					lua_settop(L, x);
					return;
				}
				lua_newtable(L);
			}
			{
				lua_pushstring(L, c_pszSubName);
				lua_pushnumber(L, lNumber);
				lua_rawset(L, -3);
			}
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantInTable(const char* c_pszName, const char* c_pszSubName, const char* szString, bool bForceCreation)
	{
		const int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				if (!bForceCreation)
				{
					sys_err("%s global index for %s already defined", c_pszName, c_pszSubName);
					lua_settop(L, x);
					return;
				}
				lua_newtable(L);
			}
			{
				lua_pushstring(L, c_pszSubName);
				lua_pushstring(L, szString);
				lua_rawset(L, -3);
			}
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}

	void CQuestManager::AddLuaConstantSubTable(const char* c_pszName, const char* c_pszSubName, luaC_tab* preg)
	{
		// lua_State* L = CQuestManager::Instance().GetLuaState();
		const int x = lua_gettop(L);
		{
			lua_getglobal(L, c_pszName);
			if (!lua_istable(L, -1))
			{
				sys_err("%s global index not found for %s", c_pszName, c_pszSubName);
				lua_settop(L, x);
				return;
			}
			lua_pushstring(L, c_pszSubName);
			{
				lua_newtable(L);
				while ((preg->name))
				{
					lua_pushstring(L, preg->name);
					switch (preg->val.type)
					{
						case ETL_CFUN:
							lua_pushcfunction(L, preg->val.cfVal);
							break;
						case ETL_LNUM:
							lua_pushnumber(L, preg->val.lnVal);
							break;
						case ETL_LSTR:
							lua_pushstring(L, preg->val.lsVal);
							break;
						case ETL_NIL:
							lua_pushnil(L);
							break;
					}
					lua_rawset(L, -3);
					preg++;
				}
			}
			lua_rawset(L, -3);
			lua_setglobal(L, c_pszName);
		}
		lua_settop(L, x);
	}
#endif

	void CQuestManager::BuildStateIndexToName(const char* questName)
	{
		const int x = lua_gettop(L);
		lua_getglobal(L, questName);

		if (lua_isnil(L, -1))
		{
			sys_err("QUEST wrong quest state file for quest %s", questName);
			lua_settop(L, x);
			return;
		}

		for (lua_pushnil(L); lua_next(L, -2);)
		{
			if (lua_isstring(L, -2) && lua_isnumber(L, -1))
			{
				lua_pushvalue(L, -2);
				lua_rawset(L, -4);
			}
			else
			{
				lua_pop(L, 1);
			}
		}

		lua_settop(L, x);
	}

	/**
	* @version 05/06/08 Bang2ni - Register __get_guildid_byname script function
	*/
	bool CQuestManager::InitializeLua()
	{
#if LUA_V == 503
		L = lua_open();

		luaopen_base(L);
		luaopen_table(L);
		luaopen_string(L);
		luaopen_math(L);
		//TEMP
		luaopen_io(L);
		luaopen_debug(L);
#elif LUA_V == 523
		L = luaL_newstate();

		luaL_openlibs(L);
		//luaopen_debug(L);
#else
#error "lua version not found"
#endif

		RegisterAffectFunctionTable();
		RegisterBuildingFunctionTable();
		RegisterDungeonFunctionTable();
		RegisterGameFunctionTable();
		RegisterGuildFunctionTable();
		RegisterHorseFunctionTable();
#ifdef __PET_SYSTEM__
		RegisterPetFunctionTable();
#endif
		RegisterITEMFunctionTable();
		RegisterMarriageFunctionTable();
		RegisterNPCFunctionTable();
		RegisterPartyFunctionTable();
		RegisterPCFunctionTable();
		RegisterQuestFunctionTable();
		RegisterTargetFunctionTable();
		RegisterArenaFunctionTable();
		RegisterForkedFunctionTable();
		RegisterMonarchFunctionTable();
		RegisterOXEventFunctionTable();
		RegisterMgmtFunctionTable();
		RegisterBattleArenaFunctionTable();
		RegisterDanceEventFunctionTable();
		RegisterDragonLairFunctionTable();
		RegisterSpeedServerFunctionTable();
		RegisterDragonSoulFunctionTable();
#ifdef ENABLE_QUEST_DND_EVENT
		RegisterDNDFunctionTable();
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		RegisterMeleyLairFunctionTable();
#endif
#ifdef ENABLE_12ZI
		RegisterZodiacTempleFunctionTable();
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		RegisterAttr67FunctionTable();
#endif
#ifdef ENABLE_QUEEN_NETHIS
		RegisterSnakeLairFunctionTable();
#endif
#ifdef ENABLE_WHITE_DRAGON
		RegisterWhiteLairFunctionTable();
#endif

		{
			luaL_reg member_functions[] =
			{
				{ "chat", member_chat },
				{ "set_ready", member_set_ready },
				{ "clear_ready", member_clear_ready },
				{ nullptr, nullptr }
			};

			AddLuaFunctionTable("member", member_functions);
		}

		{
			luaL_reg highscore_functions[] =
			{
				{ "register", highscore_register },
				{ "show", highscore_show },
				{ nullptr, nullptr }
			};

			AddLuaFunctionTable("highscore", highscore_functions);
		}

		{
			luaL_reg mob_functions[] =
			{
				{ "spawn", mob_spawn },
				{ "spawn_group", mob_spawn_group },
#ifdef ENABLE_NEWSTUFF_2019
				{ "spawn_random", mob_spawn_random},
#endif
				{ nullptr, nullptr }
			};

			AddLuaFunctionTable("mob", mob_functions);
		}

		//
		// global namespace functions
		//
		RegisterGlobalFunctionTable(L);

		// LUA_INIT_ERROR_MESSAGE
		{
			char settingsFileName[256];
			snprintf(settingsFileName, sizeof(settingsFileName), "%s/settings.lua", LocaleService_GetBasePath().c_str());

			const int settingsLoadingResult = lua_dofile(L, settingsFileName);
			sys_log(0, "LoadSettings(%s), returns %d", settingsFileName, settingsLoadingResult);
			if (settingsLoadingResult != 0)
			{
				sys_err("LOAD_SETTINS_FAILURE(%s)", settingsFileName);
				return false;
			}
		}

		{
			char questlibFileName[256];
			snprintf(questlibFileName, sizeof(questlibFileName), "%s/questlib.lua", LocaleService_GetQuestPath().c_str());

			const int questlibLoadingResult = lua_dofile(L, questlibFileName);
			sys_log(0, "LoadQuestlib(%s), returns %d", questlibFileName, questlibLoadingResult);
			if (questlibLoadingResult != 0)
			{
				sys_err("LOAD_QUESTLIB_FAILURE(%s)", questlibFileName);
				return false;
			}
		}

#ifdef ENABLE_TRANSLATE_LUA
		{
			char translateFileName[256];
			snprintf(translateFileName, sizeof(translateFileName), "%s/translate.lua", LocaleService_GetBasePath().c_str());

			const int translateLoadingResult = lua_dofile(L, translateFileName);
			sys_log(0, "LoadTranslate(%s), returns %d", translateFileName, translateLoadingResult);
			if (translateLoadingResult != 0)
			{
				sys_err("LOAD_TRANSLATE_ERROR(%s)", translateFileName);
				return false;
			}
		}
#endif

#ifdef ENABLE_QUESTLIB_EXTRA_LUA
		{
			char questlibExtraFileName[256];
			snprintf(questlibExtraFileName, sizeof(questlibExtraFileName), "%s/questlib_extra.lua", LocaleService_GetQuestPath().c_str());

			const int questlibLoadingResult = lua_dofile(L, questlibExtraFileName);
			sys_log(0, "LoadQuestlib(%s), returns %d", questlibExtraFileName, questlibLoadingResult);
		}
#endif

		{
			char questLocaleFileName[256];
			snprintf(questLocaleFileName, sizeof(questLocaleFileName), "%s/locale.lua", g_stQuestDir.c_str());

			const int questLocaleLoadingResult = lua_dofile(L, questLocaleFileName);
			sys_log(0, "LoadQuestLocale(%s), returns %d", questLocaleFileName, questLocaleLoadingResult);
			if (questLocaleLoadingResult != 0)
			{
				sys_err("LoadQuestLocale(%s) FAILURE", questLocaleFileName);
				return false;
			}
		}
		// END_OF_LUA_INIT_ERROR_MESSAGE

		for (auto it = g_setQuestObjectDir.begin(); it != g_setQuestObjectDir.end(); ++it)
		{
			const string& stQuestObjectDir = *it;
			char buf[PATH_MAX];
			snprintf(buf, sizeof(buf), "%s/state/", stQuestObjectDir.c_str());
			DIR* pdir = opendir(buf);
			int iQuestIdx = 0;

			if (pdir)
			{
				dirent* pde;

				while ((pde = readdir(pdir)))
				{
					if (pde->d_name[0] == '.')
						continue;

					snprintf(buf + 11, sizeof(buf) - 11, "%s", pde->d_name);

					RegisterQuest(pde->d_name, ++iQuestIdx);
					int ret = lua_dofile(L, (stQuestObjectDir + "/state/" + pde->d_name).c_str());
					sys_log(0, "QUEST: loading %s, returns %d", (stQuestObjectDir + "/state/" + pde->d_name).c_str(), ret);

					BuildStateIndexToName(pde->d_name);
				}

				closedir(pdir);
			}
		}

#if LUA_V == 503
		lua_setgcthreshold(L, 0);
#endif
		lua_newtable(L);
		lua_setglobal(L, "__codecache");
		return true;
	}

	void CQuestManager::GotoSelectState(QuestState& qs)
	{
		lua_checkstack(qs.co, 1);

		//int n = lua_gettop(L);
		const int n = luaL_getn(qs.co, -1);
		qs.args = n;
		//cout << "select here (1-" << qs.args << ")" << endl;
		//

		ostringstream os;
		os << "[QUESTION ";

		for (int i = 1; i <= n; i++)
		{
			lua_rawgeti(qs.co, -1, i);
			if (lua_isstring(qs.co, -1))
			{
				//printf("%d\t%s\n",i,lua_tostring(qs.co,-1));
				if (i != 1)
					os << "|";
				os << i << ";" << lua_tostring(qs.co, -1);
			}
			else
			{
				sys_err("SELECT wrong data %s", lua_typename(qs.co, -1));
				sys_err("here");
			}
			lua_pop(qs.co, 1);
		}
		os << "]";


		AddScript(os.str());
		qs.suspend_state = SUSPEND_STATE_SELECT;
		if (test_server)
			sys_log(0, "%s", m_strScript.c_str());
		SendScript();
	}

	EVENTINFO(confirm_timeout_event_info)
	{
		uint32_t dwWaitPID;
		uint32_t dwReplyPID;

		confirm_timeout_event_info() noexcept
			: dwWaitPID(0)
			, dwReplyPID(0)
		{
		}
	};

	EVENTFUNC(confirm_timeout_event)
	{
		const confirm_timeout_event_info* info = dynamic_cast<confirm_timeout_event_info*>(event->info);

		if (info == nullptr)
		{
			sys_err("confirm_timeout_event> <Factor> Null pointer");
			return 0;
		}

		const LPCHARACTER& chWait = CHARACTER_MANAGER::Instance().FindByPID(info->dwWaitPID);
		const LPCHARACTER& chReply = nullptr; //CHARACTER_MANAGER::info().FindByPID(info->dwReplyPID);

		if (chReply)
		{
			// Closes on its own after time
		}

		if (chWait)
		{
			CQuestManager::Instance().Confirm(info->dwWaitPID, CONFIRM_TIMEOUT);
		}

		return 0;
	}

	void CQuestManager::GotoConfirmState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_CONFIRM;
		const uint32_t dwVID = static_cast<uint32_t>(lua_tonumber(qs.co, -3));
		const char* szMsg = lua_tostring(qs.co, -2);
		const int iTimeout = static_cast<int>(lua_tonumber(qs.co, -1));

		sys_log(0, "GotoConfirmState vid %u msg '%s', timeout %d", dwVID, szMsg, iTimeout);

		// 1. Open a confirmation window to the other party
		// 2. Show me a window to wait for confirmation
		// 3. Timeout setting (If the timeout occurs, close the other party's window and send me to close the window as well)

		// 1
		// If there is no other person, do not send it to the other person. Passed by timeout
		LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(dwVID);
		if (ch && ch->IsPC())
			ch->ConfirmWithMsg(szMsg, iTimeout, GetCurrentCharacterPtr()->GetPlayerID());

		// 2
		GetCurrentPC()->SetConfirmWait((ch && ch->IsPC()) ? ch->GetPlayerID() : 0);
		ostringstream os;
		os << "[CONFIRM_WAIT timeout;" << iTimeout << "]";
		AddScript(os.str());
		SendScript();

		// 3
		confirm_timeout_event_info* info = AllocEventInfo<confirm_timeout_event_info>();

		info->dwWaitPID = GetCurrentCharacterPtr()->GetPlayerID();
		info->dwReplyPID = (ch && ch->IsPC()) ? ch->GetPlayerID() : 0;

		event_create(confirm_timeout_event, info, PASSES_PER_SEC(iTimeout));
	}

	void CQuestManager::GotoSelectItemState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_SELECT_ITEM;
		AddScript("[SELECT_ITEM]");
		SendScript();
	}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	void CQuestManager::GotoSelectItemExState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_SELECT_ITEM_EX;
		AddScript("[SELECT_ITEM_EX]");
		SendScript();
	}
#endif

	void CQuestManager::GotoInputState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_INPUT;
		AddScript("[INPUT]");
		SendScript();

		// Sword time limit
		//event_create(input_timeout_event, dwEI, PASSES_PER_SEC(iTimeout));
	}

	void CQuestManager::GotoPauseState(QuestState& qs)
	{
		qs.suspend_state = SUSPEND_STATE_PAUSE;
		AddScript("[NEXT]");
		SendScript();
	}

	void CQuestManager::GotoEndState(QuestState& qs)
	{
		AddScript("[DONE]");
		SendScript();
	}

	//
	// * OpenState
	//
	// The beginning of script
	//

	QuestState CQuestManager::OpenState(const string& quest_name, int state_index)
	{
		QuestState qs;
		qs.args = 0;
		qs.st = state_index;
		qs.co = lua_newthread(L);
		qs.ico = lua_ref(L, 1/*qs.co*/);
		return qs;
	}

	//
	// * RunState
	//
	// decides script to wait for user input, or finish
	//
	bool CQuestManager::RunState(QuestState& qs)
	{
		ClearError();

		m_CurrentRunningState = &qs;
		const int ret = lua_resume(qs.co, qs.args);

		if (ret == 0)
		{
			if (lua_gettop(qs.co) == 0)
			{
				// end of quest
				GotoEndState(qs);
				return false;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "select"))
			{
				GotoSelectState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "wait"))
			{
				GotoPauseState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "input"))
			{
				GotoInputState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "confirm"))
			{
				GotoConfirmState(qs);
				return true;
			}

			if (!strcmp(lua_tostring(qs.co, 1), "select_item"))
			{
				GotoSelectItemState(qs);
				return true;
			}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
			if (!strcmp(lua_tostring(qs.co, 1), "select_item_ex"))
			{
				GotoSelectItemExState(qs);
				return true;
			}
#endif
		}
		else
		{
			sys_err("LUA_ERROR: %s", lua_tostring(qs.co, 1));
		}

		WriteRunningStateToSyserr();
		SetError();

		GotoEndState(qs);
		return false;
	}

	//
	// * CloseState
	//
	// makes script end
	//
	void CQuestManager::CloseState(QuestState& qs)
	{
		if (qs.co)
		{
			//cerr << "ICO "<<qs.ico <<endl;
			lua_unref(L, qs.ico);
			qs.co = 0;
		}
	}
}
