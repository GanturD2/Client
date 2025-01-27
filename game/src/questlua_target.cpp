#include "stdafx.h"
#include "questmanager.h"
#include "char.h"
#include "sectree_manager.h"
#include "target.h"

namespace quest
{
	//
	// "target" Lua functions
	//
	ALUA(target_pos)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		int iQuestIndex = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument, name: %s, quest_index %d", ch->GetName(), iQuestIndex);
			return 0;
		}

		PIXEL_POSITION pos;

		if (!SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(ch->GetMapIndex(), pos))
		{
			sys_err("cannot find base position in this map %d", ch->GetMapIndex());
			return 0;
		}

		int x = pos.x + (int) lua_tonumber(L, 2) * 100;
		int y = pos.y + (int) lua_tonumber(L, 3) * 100;

		CTargetManager::Instance().CreateTarget(ch->GetPlayerID(),
				iQuestIndex,
				lua_tostring(L, 1),
				TARGET_TYPE_POS,
				x,
				y,
				(int) lua_tonumber(L, 4),
				lua_isstring(L, 5) ? lua_tostring(L, 5) : nullptr,
				lua_isnumber(L, 6) ? (int)lua_tonumber(L, 6): 1);

		return 0;
	}

	ALUA(target_vid)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		int iQuestIndex = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		if (!lua_isstring(L, 1) || !lua_isnumber(L, 2))
		{
			sys_err("invalid argument, name: %s, quest_index %d", ch->GetName(), iQuestIndex);
			return 0;
		}


		CTargetManager::Instance().CreateTarget(ch->GetPlayerID(),
				iQuestIndex,
				lua_tostring(L, 1),
				TARGET_TYPE_VID,
				(int) lua_tonumber(L, 2),
				0,
				ch->GetMapIndex(),
				lua_isstring(L, 3) ? lua_tostring(L, 3) : nullptr,
				lua_isnumber(L, 4) ? (int)lua_tonumber(L, 4): 1);

		return 0;
	}

	// 현재 퀘스트에 등록된 타겟을 삭제 한다.
	ALUA(target_delete)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		int iQuestIndex = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument, name: %s, quest_index %d", ch->GetName(), iQuestIndex);
			return 0;
		}

		CTargetManager::Instance().DeleteTarget(ch->GetPlayerID(), iQuestIndex, lua_tostring(L, 1));

		return 0;
	}

	// 현재 퀘스트 인덱스로 되어있는 타겟을 모두 삭제한다.
	ALUA(target_clear)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		int iQuestIndex = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		CTargetManager::Instance().DeleteTarget(ch->GetPlayerID(), iQuestIndex, nullptr);

		return 0;
	}

	ALUA(target_id)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		uint32_t dwQuestIndex = CQuestManager::Instance().GetCurrentPC()->GetCurrentQuestIndex();

		if (!lua_isstring(L, 1))
		{
			sys_err("invalid argument, name: %s, quest_index %u", ch->GetName(), dwQuestIndex);
			lua_pushnumber(L, 0);
			return 1;
		}

		LPEVENT pkEvent = CTargetManager::Instance().GetTargetEvent(ch->GetPlayerID(), dwQuestIndex, (const char *) lua_tostring(L, 1));

		if (pkEvent)
		{
			TargetInfo * pInfo = dynamic_cast<TargetInfo *>(pkEvent->info);

			if ( pInfo == nullptr )
			{
				sys_err( "target_id> <Factor> Null pointer" );
				lua_pushnumber(L, 0);
				return 1;
			}

			if (pInfo->iType == TARGET_TYPE_VID)
			{
				lua_pushnumber(L, pInfo->iArg1);
				return 1;
			}
		}

		lua_pushnumber(L, 0);
		return 1;
	}

	void RegisterTargetFunctionTable()
	{
		luaL_reg target_functions[] =
		{
			{ "pos",			target_pos		},
			{ "vid",			target_vid		},
			{ "npc",			target_vid		}, // TODO: delete this
			{ "pc",			target_vid		}, // TODO: delete this
			{ "delete",			target_delete		},
			{ "clear",			target_clear		},
			{ "id",			target_id		},
			{ nullptr,			nullptr			},
		};

		CQuestManager::Instance().AddLuaFunctionTable("target", target_functions);
	}
};

