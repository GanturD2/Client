#include "stdafx.h"

#include "config.h"
#include "questmanager.h"
#include "char.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	ALUA(ds_open_refine_window)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err ("nullptr POINT ERROR");
			return 0;
		}
		if (ch->DragonSoul_IsQualified())
			ch->DragonSoul_RefineWindow_Open(CQuestManager::Instance().GetCurrentNPCCharacterPtr());

		return 0;
	}

	ALUA(ds_give_qualification)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err ("nullptr POINT ERROR");
			return 0;
		}
		ch->DragonSoul_GiveQualification();

		return 0;
	}

	ALUA(ds_is_qualified)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err ("nullptr POINT ERROR");
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, ch->DragonSoul_IsQualified());
		return 1;
	}

#ifdef ENABLE_DS_CHANGE_ATTR
	ALUA(ds_open_change_attr_window)
	{
		const LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("nullptr POINT ERROR");
			return 0;
		}
		if (ch->DragonSoul_IsQualified())
			ch->DragonSoul_ChangeAttrWindow_Open(CQuestManager::Instance().GetCurrentNPCCharacterPtr());

		return 0;
	}
#endif

	void RegisterDragonSoulFunctionTable()
	{
		luaL_reg ds_functions[] =
		{
			{ "open_refine_window"	, ds_open_refine_window },
			{ "give_qualification"	, ds_give_qualification },
			{ "is_qualified"		, ds_is_qualified		},
#ifdef ENABLE_DS_CHANGE_ATTR
			{ "open_attr_change_window", ds_open_change_attr_window		},
#endif
			{ nullptr					, nullptr					}
		};

		CQuestManager::Instance().AddLuaFunctionTable("ds", ds_functions);
	}
};
