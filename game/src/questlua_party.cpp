#include "stdafx.h"
#include <sstream>

#include "desc.h"
#include "party.h"
#include "char.h"
#include "questlua.h"
#include "questmanager.h"
#include "packet.h"
#include "char_manager.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	using namespace std;

	//
	// "party" Lua functions
	//
	ALUA(party_clear_ready)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
		{
			FPartyClearReady f;
			ch->GetParty()->ForEachNearMember(f);
		}
		return 0;
	}

	ALUA(party_get_max_level)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			lua_pushnumber(L,ch->GetParty()->GetMemberMaxLevel());
		else
			lua_pushnumber(L, 1);

		return 1;
	}

#ifdef ENABLE_NEWSTUFF
	ALUA(party_get_min_level)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			lua_pushnumber(L,ch->GetParty()->GetMemberMinLevel());
		else
			lua_pushnumber(L, 1);

		return 1;
	}

	ALUA(party_leave_party)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		// if (!ch->GetParty()&&!CPartyManager::Instance().IsEnablePCParty()&&ch->GetDungeon())
			// return 0;

		LPPARTY pParty = ch->GetParty();
		if (pParty)
		{
			if (pParty->GetMemberCount() == 2)
				CPartyManager::Instance().DeleteParty(pParty);
			else
				pParty->Quit(ch->GetPlayerID());
		}

		lua_pushboolean(L, ch->GetParty()==nullptr);
		return 1;
	}

	ALUA(party_delete_party)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		// if (!ch->GetParty()&&!CPartyManager::Instance().IsEnablePCParty()&&ch->GetDungeon()&&!ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			// return 0;

		if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			CPartyManager::Instance().DeleteParty(ch->GetParty());

		lua_pushboolean(L, ch->GetParty()==nullptr);
		return 1;
	}
#endif

    struct FRunCinematicSender
    {
        std::string data;
		TPacketGCScript pack;

        FRunCinematicSender(const char* str)
        {
            data = "[RUN_CINEMA value;";
            data += str;
            data += "]";

            pack.header = HEADER_GC_SCRIPT;
            pack.skin = CQuestManager::QUEST_SKIN_CINEMATIC;
            //pack.skin = CQuestManager::QUEST_SKIN_NOWINDOW;
            pack.src_size = data.size();
            pack.size = pack.src_size + sizeof(TPacketGCScript);
        }

        void operator()(LPCHARACTER ch)
        {
            sys_log(0, "CINEMASEND_TRY %s", ch->GetName());

            if (ch->GetDesc())
            {
                sys_log(0, "CINEMASEND %s", ch->GetName());
                ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCScript));
                ch->GetDesc()->Packet(data.c_str(),data.size());
            }
        }
    };

	ALUA(party_run_cinematic)
	{
		if (!lua_isstring(L, 1))
			return 0;

		sys_log(0, "RUN_CINEMA %s", lua_tostring(L, 1));
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
		{
			FRunCinematicSender f(lua_tostring(L, 1));

			ch->GetParty()->Update();
			ch->GetParty()->ForEachNearMember(f);
		}

		return 0;
	}

	struct FCinematicSender
	{
		const char* str;
		TPacketGCScript packet_script;
		int len;

		FCinematicSender(const char* str)
			: str(str)
		{
			len = strlen(str);

			packet_script.header = HEADER_GC_SCRIPT;
			packet_script.skin = CQuestManager::QUEST_SKIN_CINEMATIC;
			packet_script.src_size = len;
			packet_script.size = packet_script.src_size + sizeof(TPacketGCScript);
		}

		void operator()(LPCHARACTER ch)
		{
			sys_log(0, "CINEMASEND_TRY %s", ch->GetName());

			if (ch->GetDesc())
			{
				sys_log(0, "CINEMASEND %s", ch->GetName());
				ch->GetDesc()->BufferedPacket(&packet_script, sizeof(TPacketGCScript));
				ch->GetDesc()->Packet(str,len);
			}
		}
	};

	ALUA(party_show_cinematic)
	{
		if (!lua_isstring(L, 1))
			return 0;

		sys_log(0, "CINEMA %s", lua_tostring(L, 1));
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
		{
			FCinematicSender f(lua_tostring(L, 1));

			ch->GetParty()->Update();
			ch->GetParty()->ForEachNearMember(f);
		}
		return 0;
	}

	ALUA(party_get_near_count)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			lua_pushnumber(L, ch->GetParty()->GetNearMemberCount());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(party_syschat)
	{
		LPPARTY pParty = CQuestManager::Instance().GetCurrentCharacterPtr()->GetParty();
		if (pParty)
		{
			ostringstream s;
			combine_lua_string(L, s);

			FPartyChat f(CHAT_TYPE_INFO, s.str().c_str());

			pParty->ForEachOnlineMember(f);
		}

		return 0;
	}

	ALUA(party_is_leader)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())
			lua_pushboolean(L, 1);
		else
			lua_pushboolean(L, 0);

		return 1;
	}

	ALUA(party_is_party)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		lua_pushboolean(L, ch->GetParty() ? 1 : 0);
		return 1;
	}

	ALUA(party_get_leader_pid)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			lua_pushnumber(L, ch->GetParty()->GetLeaderPID());
		else
			lua_pushnumber(L, -1);

		return 1;
	}

#ifdef ENABLE_PARTY_FEATURES
	ALUA(party_get_leader_name)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			lua_pushstring(L, ch->GetParty()->GetLeaderName());
		else
			lua_pushstring(L, ch->GetName());
		return 1;
	}
#endif

	ALUA(party_chat)
	{
		LPPARTY pParty = CQuestManager::Instance().GetCurrentCharacterPtr()->GetParty();
		if (pParty)
		{
			ostringstream s;
			combine_lua_string(L, s);

			FPartyChat f(CHAT_TYPE_TALKING, s.str().c_str());

			pParty->ForEachOnlineMember(f);
		}

		return 0;
	}


	ALUA(party_is_map_member_flag_lt)
	{
		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CQuestManager& q = CQuestManager::Instance();
		LPPARTY pParty = q.GetCurrentCharacterPtr()->GetParty();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		PC* pPC = q.GetCurrentPC();

		const char* sz = lua_tostring(L,1);

		if (pParty)
		{
			FPartyCheckFlagLt f;
			f.flagname = pPC->GetCurrentQuestName() + "."+sz;
			f.value = (int) rint(lua_tonumber(L, 2));

			bool returnBool = pParty->ForEachOnMapMemberBool(f, ch->GetMapIndex());
			lua_pushboolean(L, returnBool);
		}

		return 1;
	}

	ALUA(party_set_flag)
	{
		if (!lua_isstring(L, 1) && !lua_isnumber(L, 2))
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!ch->GetParty())
			return 0;

		LPPARTY pParty = ch->GetParty();
		pParty->SetFlag(lua_tostring(L, 1), (int)lua_tonumber(L, 2));
		return 0;
	}

	ALUA(party_get_flag)
	{
		if (!lua_isstring(L, 1))
			return 0;

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!ch->GetParty())
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, ch->GetParty()->GetFlag(lua_tostring(L, 1)));

		return 1;
	}

	ALUA(party_set_quest_flag)
	{
		CQuestManager & q = CQuestManager::Instance();

		FSetQuestFlag f;

		f.flagname = q.GetCurrentPC()->GetCurrentQuestName() + "." + lua_tostring(L, 1);
		f.value = (int) rint(lua_tonumber(L, 2));

		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			ch->GetParty()->ForEachOnlineMember(f);
		else
			f(ch);

		return 0;
	}

	ALUA(party_is_in_dungeon)
	{
		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPPARTY pParty = ch->GetParty();
		if (pParty != nullptr)
		{
			lua_pushboolean (L, pParty->GetDungeon() ? true : false);
			return 1;
		}

		lua_pushboolean (L, false);
		return 1;
	}

	struct FGiveBuff
	{
		uint32_t dwType;
		uint16_t wApplyOn;	//@fixme532
		long lApplyValue;
		uint32_t dwFlag;
		long lDuration;
		long lSPCost;
		bool bOverride;
		bool IsCube;

		FGiveBuff (uint32_t _dwType, uint16_t _wApplyOn, long _lApplyValue, uint32_t _dwFlag, long _lDuration,	//@fixme532
					long _lSPCost, bool _bOverride, bool _IsCube = false)
			: dwType (_dwType), wApplyOn(_wApplyOn), lApplyValue (_lApplyValue), dwFlag(_dwFlag), lDuration(_lDuration),
				lSPCost(_lSPCost), bOverride(_bOverride), IsCube(_IsCube)
		{}
		void operator () (LPCHARACTER ch)
		{
			ch->AddAffect(dwType, wApplyOn, lApplyValue, dwFlag, lDuration, lSPCost, bOverride, IsCube);
		}
	};

	// 파티 단위로 버프 주는 함수.
	// 같은 맵에 있는 파티원만 영향을 받는다.
	ALUA(party_give_buff)
	{
		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) ||
			!lua_isnumber(L, 5) || !lua_isnumber(L, 6) || !lua_isboolean(L, 7) || !lua_isboolean(L, 8))
		{
			lua_pushboolean (L, false);
			return 1;
		}

		const uint32_t dwType = lua_tonumber(L, 1);
		const uint16_t wApplyOn = lua_tonumber(L, 2);	//@fixme532
		const long lApplyValue = lua_tonumber(L, 3);
		const uint32_t dwFlag = lua_tonumber(L, 4);
		const long lDuration = lua_tonumber(L, 5);
		const long lSPCost = lua_tonumber(L, 6);
		const bool bOverride = lua_toboolean(L, 7);
		const bool IsCube = lua_toboolean(L, 8);

		FGiveBuff f (dwType, wApplyOn, lApplyValue, dwFlag, lDuration, lSPCost, bOverride, IsCube);
		if (ch->GetParty())
			ch->GetParty()->ForEachOnMapMember(f, ch->GetMapIndex());
		else
			f(ch);

		lua_pushboolean (L, true);
		return 1;
	}

	struct FPartyPIDCollector
	{
		std::vector <uint32_t> vecPIDs;
		FPartyPIDCollector()
		{
		}
		void operator () (LPCHARACTER ch)
		{
			vecPIDs.emplace_back(ch->GetPlayerID());
		}
	};
	ALUA(party_get_member_pids)
	{
		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPPARTY pParty = ch->GetParty();
		if (nullptr == pParty)
		{
			return 0;
		}

		FPartyPIDCollector f;
		pParty->ForEachOnMapMember(f, ch->GetMapIndex());
		for (std::vector <uint32_t>::iterator it = f.vecPIDs.begin(); it != f.vecPIDs.end(); ++it)	//@fixme541
		{
			lua_pushnumber(L, *it);
		}

		return f.vecPIDs.size();
	}

	struct FPartyVIDCollector
	{
		std::vector <uint32_t> vecVIDs;
		FPartyVIDCollector()
		{
		}

		void operator () (LPCHARACTER ch)
		{
			vecVIDs.emplace_back(ch->GetVID());
		}
	};

	ALUA(party_get_member_vids)
	{
		CQuestManager & q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPPARTY pParty = ch->GetParty();
		if (nullptr == pParty)
		{
			return 0;
		}

		FPartyVIDCollector f;
		pParty->ForEachOnMapMember(f, ch->GetMapIndex());
		for (std::vector <uint32_t>::iterator it = f.vecVIDs.begin(); it != f.vecVIDs.end(); ++it)
		{
			lua_pushnumber(L, *it);
		}

		return f.vecVIDs.size();
	}


	/* Removes a player from the group also deletes the group if members = 2 or user is leader. */
	ALUA(party_remove_player)
	{
		LPCHARACTER ch;
		CQuestManager & q = CQuestManager::Instance();
		if (lua_isnumber(L, 1))
		{
			uint32_t pid = (uint32_t)lua_tonumber(L, 1);
			ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
		}
		else
			ch = q.GetCurrentCharacterPtr();

		if (!ch)
			return 0; // Supplied pid doesn? exist or GetCurrentCharacterPTR failed (unlikely)

		LPPARTY pParty = ch->GetParty();
		if (!pParty) // Player has no party
			return 0;

		if (pParty->GetMemberCount() == 2 || pParty->GetLeaderPID() == ch->GetPlayerID())
			CPartyManager::Instance().DeleteParty(pParty);
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;675]");
			pParty->Quit(ch->GetPlayerID());
		}

		return 1;
	}

#ifdef ENABLE_DUNGEON_RENEWAL
	ALUA(party_get_member_count)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		if (ch->GetParty())
			lua_pushnumber(L, ch->GetParty()->GetMemberCount());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	ALUA(party_get_near_member_pids)	// Near Check missing!
	{
		CQuestManager& q = CQuestManager::Instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPPARTY pParty = ch->GetParty();
		if (nullptr == pParty)
		{
			return 0;
		}

		FPartyPIDCollector f;
		pParty->ForEachOnMapMember(f, ch->GetMapIndex());
		for (std::vector <uint32_t>::iterator it = f.vecPIDs.begin(); it != f.vecPIDs.end(); ++it)	//@fixme541
		{
			lua_pushnumber(L, *it);
		}

		return f.vecPIDs.size();
	}
#endif

	void RegisterPartyFunctionTable()
	{
		luaL_reg party_functions[] =
		{
			{ "is_leader",		party_is_leader		},
			{ "is_party",		party_is_party		},
			{ "get_leader_pid",	party_get_leader_pid},
#ifdef ENABLE_PARTY_FEATURES
			{"get_leader_name",	party_get_leader_name},
#endif
			{ "setf",			party_set_flag		},
			{ "getf",			party_get_flag		},
			{ "setqf",			party_set_quest_flag},
			{ "chat",			party_chat			},
			{ "syschat",		party_syschat		},
			{ "get_near_count",	party_get_near_count},
			{ "show_cinematic",	party_show_cinematic},
			{ "run_cinematic",	party_run_cinematic	},
			{ "get_max_level",	party_get_max_level	},
#ifdef ENABLE_NEWSTUFF
			{ "get_min_level",	party_get_min_level	},	// [return lua number]
			{ "leave_party",	party_leave_party	},	// [return lua boolean=successfulness]
			{ "delete_party",	party_delete_party	},	// [return lua boolean=successfulness]
#endif
			{ "clear_ready",	party_clear_ready	},
			{ "is_in_dungeon",	party_is_in_dungeon	},
			{ "give_buff",		party_give_buff		},
			{ "is_map_member_flag_lt",	party_is_map_member_flag_lt	},
			{ "get_member_pids",		party_get_member_pids	}, // 파티원들의 pid를 return
			{ "get_member_vids",	party_get_member_vids },
			{ "remove_player",		party_remove_player },

#ifdef ENABLE_DUNGEON_RENEWAL
			{ "get_member_count",		party_get_member_count },
			{ "get_near_member_pids",	party_get_near_member_pids },

#endif

			{ nullptr,				nullptr				}
		};

		CQuestManager::Instance().AddLuaFunctionTable("party", party_functions);
	}
}




