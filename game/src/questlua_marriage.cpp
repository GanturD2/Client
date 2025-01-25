#include "stdafx.h"
#include "char.h"
#include "char_manager.h"
#include "wedding.h"
#include "questmanager.h"
#include "utils.h"
#include "config.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	ALUA(marriage_engage_to)
	{
		uint32_t vid = (uint32_t) lua_tonumber(L, 1);
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		LPCHARACTER ch_you = CHARACTER_MANAGER::Instance().Find(vid);
		if (ch_you)
		{
			marriage::CManager::Instance().RequestAdd(ch->GetPlayerID(), ch_you->GetPlayerID(), ch->GetName(), ch_you->GetName());
		}
		return 0;
	}

	ALUA(marriage_remove)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		marriage::CManager::Instance().RequestRemove(ch->GetPlayerID(), pMarriage->GetOther(ch->GetPlayerID()));
		return 0;
	}

	ALUA(marriage_set_to_marriage)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		pMarriage->SetMarried();
		return 0;
	}

	ALUA(marriage_find_married_vid)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		uint32_t vid = 0;
		if (pMarriage)
		{
			LPCHARACTER you = CHARACTER_MANAGER::Instance().FindByPID(pMarriage->GetOther(ch->GetPlayerID()));
			if (you)
				vid = you->GetVID();
		}

		lua_pushnumber(L, vid);

		return 1;
	}

	struct FBuildLuaWeddingMapList
	{
		lua_State * L;
		int m_count;
		FBuildLuaWeddingMapList(lua_State * L) : L(L), m_count(1)
		{
			lua_newtable(L);
		}

		void operator() (marriage::TMarriage* pMarriage)
		{
			if (!pMarriage->pWeddingInfo)
				return;

			lua_newtable(L);
			lua_pushnumber(L, pMarriage->m_pid1);
			lua_rawseti(L, -2, 1);
			lua_pushnumber(L, pMarriage->m_pid2);
			lua_rawseti(L, -2, 2);
			lua_pushstring(L, pMarriage->name1.c_str());
			lua_rawseti(L, -2, 3);
			lua_pushstring(L, pMarriage->name2.c_str());
			lua_rawseti(L, -2, 4);
			lua_rawseti(L, -2, m_count++);
		}
	};

	ALUA(marriage_get_wedding_list)
	{
		marriage::CManager::Instance().for_each_wedding(FBuildLuaWeddingMapList(L));
		return 1;
	}

	ALUA(marriage_join_wedding)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid player id for wedding map");
			return 0;
		}

		uint32_t pid1 = (uint32_t) lua_tonumber(L, 1);
		uint32_t pid2 = (uint32_t) lua_tonumber(L, 2);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(pid1);
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->GetOther(pid1) != pid2)
		{
			sys_err("not married %u %u", pid1, pid2);
			return 0;
		}
		//PREVENT_HACK
		if ( ch->IsHack() )
			return 0;
		//END_PREVENT_HACK

		pMarriage->WarpToWeddingMap(ch->GetPlayerID());
		return 0;
	}

	ALUA(marriage_warp_to_my_marriage_map)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}


		//PREVENT_HACK
		if ( ch->IsHack() )
			return 0;
		//END_PREVENT_HACK

		pMarriage->WarpToWeddingMap(ch->GetPlayerID());
		return 0;
	}

	ALUA(marriage_end_wedding)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->pWeddingInfo)
		{
			// ��ȥ�� ������ ��û
			pMarriage->RequestEndWedding();
		}
		return 0;
	}

	//@fixme533
	ALUA(marriage_wedding_is_dark)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage) {
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->pWeddingInfo) {
			marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
			lua_pushboolean(L, pWedding->IsDark());
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}

	ALUA(marriage_wedding_is_snow)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage) {
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->pWeddingInfo) {
			marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
			lua_pushboolean(L, pWedding->IsSnow());
			return 1;
		}

		lua_pushboolean(L, false);
		return 1;
	}
	//@end_fixme533

	ALUA(marriage_wedding_dark)
	{
		if (!lua_isboolean(L, 1))
		{
			sys_err("invalid argument 1 : must be boolean");
			return 0;
		}
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());

		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->pWeddingInfo)
		{
			marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
			pWedding->SetDark(lua_toboolean(L, 1));
		}

		return 0;
	}

	ALUA(marriage_wedding_client_command)
	{
		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument 1 : must be string");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage && pMarriage->pWeddingInfo)	//@fixme533
		{
			marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
			pWedding->ShoutInMap(CHAT_TYPE_COMMAND, lua_tostring(L, 1));
		}
		return 0;

	}

	ALUA(marriage_wedding_is_playing_music)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}

		marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
		lua_pushboolean(L, pWedding->IsPlayingMusic());
		return 1;
	}
	ALUA(marriage_wedding_music)
	{
		if (!lua_isboolean(L, 1))
		{
			sys_err("invalid argument 1 : must be boolean");
			return 0;
		}
		if (!lua_isstring(L, 2))
		{
			sys_err("invalid argument 2 : must be string");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->pWeddingInfo)
		{
			marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
			pWedding->SetMusic(
					lua_toboolean(L, 1),
					lua_tostring(L, 2)
					);
		}
		return 0;
	}
	ALUA(marriage_wedding_snow)
	{
		if (!lua_isboolean(L, 1))
		{
			sys_err("invalid argument 1 : must be boolean");
			return 0;
		}
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (!pMarriage)
		{
			sys_err("pid[%d:%s] is not exist couple", ch->GetPlayerID(), ch->GetName());
			return 0;
		}
		if (pMarriage->pWeddingInfo)
		{
			marriage::WeddingMap* pWedding = marriage::WeddingManager::Instance().Find(pMarriage->pWeddingInfo->dwMapIndex);
			pWedding->SetSnow(lua_toboolean(L, 1));
		}
		return 0;
	}

	ALUA(marriage_in_my_wedding)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());
		if (pMarriage->pWeddingInfo)
		{
			lua_pushboolean(L, (uint32_t)ch->GetMapIndex() == pMarriage->pWeddingInfo->dwMapIndex);
		}
		else
		{
			lua_pushboolean(L, 0);
		}
		return 1;
	}

	ALUA(marriage_get_married_time)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(ch->GetPlayerID());

		if (!pMarriage)
		{
			sys_err("trying to get time for not married character");
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, get_global_time() - pMarriage->marry_time);
		return 1;
	}

	void RegisterMarriageFunctionTable()
	{
		luaL_reg marriage_functions[] =
		{
			{ "engage_to",		marriage_engage_to	    },
			{ "remove",			marriage_remove		    },
			{ "find_married_vid",	marriage_find_married_vid   },
			{ "get_wedding_list",	marriage_get_wedding_list   },
			{ "join_wedding",		marriage_join_wedding	    },
			{ "set_to_marriage",	marriage_set_to_marriage    },
			{ "end_wedding",		marriage_end_wedding	    },
			{ "wedding_is_dark",	marriage_wedding_is_dark	},	
			{ "wedding_is_snow",	marriage_wedding_is_snow	},	//@fixme533
			{ "wedding_dark",		marriage_wedding_dark	    },	//@fixme533
			{ "wedding_snow",		marriage_wedding_snow	    },
			{ "wedding_music",		marriage_wedding_music	    },
			{ "wedding_is_playing_music", marriage_wedding_is_playing_music },
			{ "wedding_client_command",	marriage_wedding_client_command    },
			{ "in_my_wedding",		marriage_in_my_wedding	    },
			{ "warp_to_my_marriage_map",marriage_warp_to_my_marriage_map},
			{ "get_married_time",	marriage_get_married_time   },
			{ nullptr,			nullptr			    }
		};
		CQuestManager::Instance().AddLuaFunctionTable("marriage", marriage_functions);
	}
}
