#include "stdafx.h"
#include "config.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "char.h"
#include "affect.h"
#include "db.h"

namespace quest
{
	//
	// "affect" Lua functions
	//
	ALUA(affect_add)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();

		uint16_t applyOn = (uint16_t) lua_tonumber(L, 1);	//@fixme532

		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		if (ch->FindAffect(AFFECT_QUEST_START_IDX, applyOn)) // 퀘스트로 인해 같은 곳에 효과가 걸려있으면 스킵
			return 0;

		long value = (long) lua_tonumber(L, 2);
		long duration = (long) lua_tonumber(L, 3);

		ch->AddAffect(AFFECT_QUEST_START_IDX, aApplyInfo[applyOn].wPointType, value, 0, duration, 0, false);

		return 0;
	}

	ALUA(affect_remove)
	{
		CQuestManager & q = CQuestManager::Instance();
		int iType;

		if (lua_isnumber(L, 1))
		{
			iType = (int) lua_tonumber(L, 1);

			if (iType == 0)
				iType = q.GetCurrentPC()->GetCurrentQuestIndex() + AFFECT_QUEST_START_IDX;
		}
		else
			iType = q.GetCurrentPC()->GetCurrentQuestIndex() + AFFECT_QUEST_START_IDX;

		q.GetCurrentCharacterPtr()->RemoveAffect(iType);

		return 0;
	}

	ALUA(affect_remove_bad) // 나쁜 효과를 없앰
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		ch->RemoveBadAffect();
		return 0;
	}

	ALUA(affect_remove_good) // 좋은 효과를 없앰
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		ch->RemoveGoodAffect();
		return 0;
	}

	ALUA(affect_add_hair)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();

		uint16_t applyOn = (uint16_t) lua_tonumber(L, 1);	//@fixme532

		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		long value = (long) lua_tonumber(L, 2);
		long duration = (long) lua_tonumber(L, 3);

		ch->AddAffect(AFFECT_HAIR, aApplyInfo[applyOn].wPointType, value, 0, duration, 0, false);

		return 0;
	}

	ALUA(affect_remove_hair) // 헤어 효과를 없앤다.
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		CAffect* pkAff = ch->FindAffect( AFFECT_HAIR );

		if ( pkAff != nullptr )
		{
			lua_pushnumber(L, pkAff->lDuration);
			ch->RemoveAffect( pkAff );
		}
		else
		{
			lua_pushnumber(L, 0);
		}

		return 1;
	}

	// 현재 캐릭터가 AFFECT_TYPE affect를 갖고있으면 bApplyOn 값을 반환하고 없으면 nil을 반환하는 함수.
	// usage :	applyOn = affect.get_apply(AFFECT_TYPE)
	ALUA(affect_get_apply_on)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		uint32_t affectType = lua_tonumber(L, 1);

		CAffect* pkAff = ch->FindAffect(affectType);

		if ( pkAff != nullptr )
			lua_pushnumber(L, pkAff->wApplyOn);
		else
			lua_pushnil(L);

		return 1;

	}

	ALUA(affect_add_collect)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();

		uint16_t applyOn = (uint16_t) lua_tonumber(L, 1);	//@fixme532

		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		long value = (long) lua_tonumber(L, 2);
		long duration = (long) lua_tonumber(L, 3);

		ch->AddAffect(AFFECT_COLLECT, aApplyInfo[applyOn].wPointType, value, 0, duration, 0, false);

		return 0;
	}

	ALUA(affect_add_collect_point)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager & q = CQuestManager::Instance();

		uint16_t point_type = (uint16_t) lua_tonumber(L, 1);	//@fixme532

		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (point_type >= POINT_MAX_NUM || point_type < 1)
		{
			sys_err("point is out of range : %d", point_type);
			return 0;
		}

		long value = (long) lua_tonumber(L, 2);
		long duration = (long) lua_tonumber(L, 3);

		ch->AddAffect(AFFECT_COLLECT, point_type, value, 0, duration, 0, false);

		return 0;
	}

	ALUA(affect_remove_collect)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ( ch != nullptr )
		{
			uint16_t bApply = (uint16_t)lua_tonumber(L, 1);	//@fixme532

			if ( bApply >= MAX_APPLY_NUM ) return 0;

			bApply = aApplyInfo[bApply].wPointType;
			long value = (long)lua_tonumber(L, 2);

			const std::list<CAffect*>& rList = ch->GetAffectContainer();
			const CAffect* pAffect = nullptr;

			for ( std::list<CAffect*>::const_iterator iter = rList.begin(); iter != rList.end(); ++iter )
			{
				pAffect = *iter;

				if ( pAffect->dwType == AFFECT_COLLECT )
				{
					if ( pAffect->wApplyOn == bApply && pAffect->lApplyValue == value )
					{
						break;
					}
				}

				pAffect = nullptr;
			}

			if ( pAffect != nullptr )
			{
				ch->RemoveAffect( const_cast<CAffect*>(pAffect) );
			}
		}

		return 0;
	}

	ALUA(affect_remove_all_collect)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();

		if ( ch != nullptr )
		{
			ch->RemoveAffect(AFFECT_COLLECT);
		}

		return 0;
	}

#ifdef ENABLE_SUNG_MAHI_TOWER
	ALUA(affect_add_type)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		uint32_t dwType = (uint32_t)lua_tonumber(L, 1);
		uint16_t applyOn = (uint16_t)lua_tonumber(L, 2);

		if (applyOn >= MAX_APPLY_NUM || applyOn < 1)
		{
			sys_err("apply is out of range : %d", applyOn);
			return 0;
		}

		long value = (long)lua_tonumber(L, 3);
		long duration = (long)lua_tonumber(L, 4);

		ch->AddAffect(dwType, aApplyInfo[applyOn].wPointType, value, 0, duration, 0, true);
		return 0;
	}

	ALUA(affect_remove_type)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (ch)
		{
			uint32_t dwType = (uint32_t)lua_tonumber(L, 1);
			ch->RemoveAffect(dwType);
		}

		return 0;
	}
#endif

	void RegisterAffectFunctionTable()
	{
		luaL_reg affect_functions[] =
		{
			{ "add",		affect_add		},
			{ "remove",		affect_remove		},
			{ "remove_bad",	affect_remove_bad	},
			{ "remove_good",	affect_remove_good	},
			{ "add_hair",		affect_add_hair		},
			{ "remove_hair",	affect_remove_hair		},
			{ "add_collect",		affect_add_collect		},
			{ "add_collect_point",		affect_add_collect_point		},
			{ "remove_collect",		affect_remove_collect	},
			{ "remove_all_collect",	affect_remove_all_collect	},
			{ "get_apply_on",	affect_get_apply_on },
#ifdef ENABLE_SUNG_MAHI_TOWER
			{ "add_type",	affect_add_type },
			{ "remove_type",	affect_remove_type },
#endif

			{ nullptr,		nullptr			}
		};

		CQuestManager::Instance().AddLuaFunctionTable("affect", affect_functions);
	}
};
