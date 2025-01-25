#include "stdafx.h"

#ifdef ENABLE_ATTR_6TH_7TH
#include "questmanager.h"
#include "char.h"
#include "item.h"
#include "Attr6th7th.h"
#include "utils.h"

#undef sys_err
#ifndef __WIN32__
#define sys_err(fmt, args...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::Instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	ALUA(attr67_npcstorage_have_item)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPITEM item = ch->GetAttr67InventoryItem();
		lua_pushboolean(L, item ? true : false);

		return 1;
	}

	ALUA(attr67_get_vnum)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPITEM item = ch->GetAttr67InventoryItem();
		uint32_t dwVnum = item->GetVnum();
		lua_pushnumber(L, item ? dwVnum : 0);

		return 1;
	}

	ALUA(attr67_get_time)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		LPITEM item = ch->GetAttr67InventoryItem();
		const int getTime = CAttr6th7th::Instance().GetAddAttrTime(ch);
		lua_pushnumber(L, item ? getTime : 0);

		return 1;
	}

	ALUA(attr67_set_time)
	{
		if (!lua_isnumber(L, 1))
			return 1;

		const int newTime = (int)lua_tonumber(L, 1);

		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 1;

		CAttr6th7th::Instance().SetAddAttrTime(ch, get_global_time() + newTime);

		return 1;
	}

	ALUA(attr67_enough_inventory)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		lua_pushboolean(L, CAttr6th7th::Instance().GetEnoughInventory(ch));
		return 1;
	}


	ALUA(attr67_get_item_attr)
	{
		LPCHARACTER ch = CQuestManager::Instance().GetCurrentCharacterPtr();
		if (!ch)
			return 0;

		uint8_t bResult = 0;
		uint16_t wSlot = 0;
		CAttr6th7th::Instance().GetItemAttr(ch, &bResult, &wSlot);

		lua_pushnumber(L, bResult);
		lua_pushnumber(L, wSlot);
		return 2;
	}

	void RegisterAttr67FunctionTable()
	{
		luaL_reg attr6th7th_functions[] =
		{
			{ "npcstorage_have_item", attr67_npcstorage_have_item },
			{ "get_vnum", attr67_get_vnum },
			{ "get_time", attr67_get_time },
			{ "set_time", attr67_set_time },
			{ "enough_inventory", attr67_enough_inventory },
			{ "get_item_attr", attr67_get_item_attr },
			{ nullptr, nullptr }
		};

		CQuestManager::Instance().AddLuaFunctionTable("attr6th7th", attr6th7th_functions);
	}
}
#endif
