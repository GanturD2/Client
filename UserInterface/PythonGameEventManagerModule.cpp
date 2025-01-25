#include "StdAfx.h"
#include "../GameLib/GameEventManager.h"

#ifdef ENABLE_EVENT_MANAGER
#	include "PythonGameEventManager.h"
#endif

PyObject* eventMgrUpdate(PyObject* poSelf, PyObject* poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();
	float fz;
	if (!PyTuple_GetFloat(poArgs, 2, &fz))
		return Py_BuildException();

	CGameEventManager::Instance().SetCenterPosition(fx, fy, fz);
	CGameEventManager::Instance().Update();
	return Py_BuildNone();
}

#ifdef ENABLE_EVENT_MANAGER
PyObject* eventMgrGetDayEventCount(PyObject* poSelf, PyObject* poArgs)
{
	int iDay;
	if (!PyTuple_GetInteger(poArgs, 0, &iDay))
		return Py_BuildException();

	return Py_BuildValue("i", InGameEventManager::Instance().GetDayEventCount(iDay));
}

PyObject* eventMgrGetEventData(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int iDay;
	if (!PyTuple_GetInteger(poArgs, 1, &iDay))
		return Py_BuildException();

	InGameEventManager::TEventTable table;
	memset(&table, 0, sizeof(table));

	InGameEventManager::Instance().GetEventData(iIndex, iDay, table);

	return Py_BuildValue("iiiiiiii", table.dwID, table.bType, table.startTime, table.endTime,
		table.dwVnum, table.iPercent, table.iDropType, table.bCompleted);
}

PyObject* eventMgrSetRequestEventData(PyObject* poSelf, PyObject* poArgs)
{
	bool bRequest;
	if (!PyTuple_GetBoolean(poArgs, 0, &bRequest))
		return Py_BadArgument();

	InGameEventManager::Instance().SetRequestEventData(bRequest);

	return Py_BuildNone();
}

PyObject* eventMgrGetRequestEventData(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("b", InGameEventManager::Instance().GetRequestEventData());
}

PyObject* eventMgrGetEventRewardData(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int iPos;
	if (!PyTuple_GetInteger(poArgs, 1, &iPos))
		return Py_BuildException();

	InGameEventManager::TEventReward table;
	memset(&table, 0, sizeof(table));

	InGameEventManager::Instance().GetEventRewardData(iIndex, iPos, table);

	return Py_BuildValue("ii", table.dwVnum, table.dwCount);
}
#endif

void initeventmgr() noexcept
{
	static PyMethodDef s_methods[] = { {"Update", eventMgrUpdate, METH_VARARGS},
#ifdef ENABLE_EVENT_MANAGER
									  {"GetDayEventCount", eventMgrGetDayEventCount, METH_VARARGS},
									  {"GetEventData", eventMgrGetEventData, METH_VARARGS},
									  {"SetRequestEventData", eventMgrSetRequestEventData, METH_VARARGS},
									  {"GetRequestEventData", eventMgrGetRequestEventData, METH_VARARGS},
									  {"GetEventRewardData", eventMgrGetEventRewardData, METH_VARARGS},
#endif
									  {nullptr, nullptr, 0} };

	PyObject* poModule = Py_InitModule("eventMgr", s_methods);

#ifdef ENABLE_EVENT_MANAGER
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_NONE",					InGameEventManager::EVENT_TYPE_NONE);
	// Official Events
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MYSTERY_BOX_DROP",		InGameEventManager::EVENT_TYPE_MYSTERY_BOX_DROP);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_E_BUFF_SPAWN",			InGameEventManager::EVENT_TYPE_E_BUFF_SPAWN);			// ENABLE_LUCKY_EVENT	[Beeshido event]
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MINI_GAME_OKEY",			InGameEventManager::EVENT_TYPE_MINI_GAME_OKEY);			// ENABLE_MINI_GAME_OKEY_NORMAL
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_NEW_XMAS_EVENT",			InGameEventManager::EVENT_TYPE_NEW_XMAS_EVENT);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MINI_GAME_YUTNORI",		InGameEventManager::EVENT_TYPE_MINI_GAME_YUTNORI);		// ENABLE_MINI_GAME_YUTNORI
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_ATTENDANCE",				InGameEventManager::EVENT_TYPE_ATTENDANCE);				// ENABLE_MONSTER_BACK	[attendance]
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_E_MONSTERBACK",			InGameEventManager::EVENT_TYPE_E_MONSTERBACK);			// ENABLE_MONSTER_BACK	[MonsterHunt]
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_EASTER_DROP",				InGameEventManager::EVENT_TYPE_EASTER_DROP);			// ENABLE_EASTER_EVENT
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_E_SUMMER_EVENT",			InGameEventManager::EVENT_TYPE_E_SUMMER_EVENT);			// ENABLE_SUMMER_EVENT
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_RAMADAN_DROP",			InGameEventManager::EVENT_TYPE_RAMADAN_DROP);			// ENABLE_2017_RAMADAN
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_HALLOWEEN_BOX",			InGameEventManager::EVENT_TYPE_HALLOWEEN_BOX);			// ENABLE_NEW_HALLOWEEN_EVENT
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_SOUL_EVENT",				InGameEventManager::EVENT_TYPE_SOUL_EVENT);				// ENABLE_SOUL_SYSTEM
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_FOOTBALL_DROP",			InGameEventManager::EVENT_TYPE_FOOTBALL_DROP);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MEDAL_PART_DROP",			InGameEventManager::EVENT_TYPE_MEDAL_PART_DROP);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_VALENTINE_DROP",			InGameEventManager::EVENT_TYPE_VALENTINE_DROP);			// ENABLE_2016_VALENTINE
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_FISH_EVENT",				InGameEventManager::EVENT_TYPE_FISH_EVENT);				// ENABLE_FISH_EVENT
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_E_FLOWER_DROP",			InGameEventManager::EVENT_TYPE_E_FLOWER_DROP);			// ENABLE_FLOWER_EVENT
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MINI_GAME_CATCHKING",		InGameEventManager::EVENT_TYPE_MINI_GAME_CATCHKING);	// ENABLE_MINI_GAME_CATCH_KING
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MD_START",				InGameEventManager::EVENT_TYPE_MD_START);				// ENABLE_GEM_SYSTEM
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MINI_GAME_FINDM",			InGameEventManager::EVENT_TYPE_MINI_GAME_FINDM);		// ENABLE_MINI_GAME_FINDM
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_E_LATE_SUMMER",			InGameEventManager::EVENT_TYPE_E_LATE_SUMMER);			// ENABLE_SUMMER_EVENT_ROULETTE
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MINI_GAME_BNW",			InGameEventManager::EVENT_TYPE_MINI_GAME_BNW);			// ENABLE_MINI_GAME_BNW
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_WORLD_BOSS",				InGameEventManager::EVENT_TYPE_WORLD_BOSS);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_BATTLE_ROYALE",			InGameEventManager::EVENT_TYPE_BATTLE_ROYALE);			// ENABLE_BATTLE_ROYALE
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_METINSTONE_RAIN_EVENT",	InGameEventManager::EVENT_TYPE_METINSTONE_RAIN_EVENT);	// ENABLE_METINSTONE_RAIN_EVENT

	// Custom
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_EXPERIENCE",				InGameEventManager::EVENT_TYPE_EXPERIENCE);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_ITEM_DROP",				InGameEventManager::EVENT_TYPE_ITEM_DROP);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_SUPER_METIN",				InGameEventManager::EVENT_TYPE_SUPER_METIN);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_BOSS",					InGameEventManager::EVENT_TYPE_BOSS);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_OX",						InGameEventManager::EVENT_TYPE_OX);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MANWOO",					InGameEventManager::EVENT_TYPE_MANWOO); // fish
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MINING",					InGameEventManager::EVENT_TYPE_MINING);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_BUDOKAN",					InGameEventManager::EVENT_TYPE_BUDOKAN);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_SUNGZI_WAR",				InGameEventManager::EVENT_TYPE_SUNGZI_WAR);
	// Custom drop events
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_MOONLIGHT",				InGameEventManager::EVENT_TYPE_MOONLIGHT);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_HEXEGONAL_CHEST",			InGameEventManager::EVENT_TYPE_HEXEGONAL_CHEST);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_HUNT_YOUR_MOUNT",			InGameEventManager::EVENT_TYPE_HUNT_YOUR_MOUNT);

	// Unused
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_GOLD_FROG",				InGameEventManager::EVENT_TYPE_GOLD_FROG);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_TANAKA",					InGameEventManager::EVENT_TYPE_TANAKA);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_HIDE_AND_SEEK",			InGameEventManager::EVENT_TYPE_HIDE_AND_SEEK);

	PyModule_AddIntConstant(poModule, "EVENT_TYPE_SIEGE_WAR",				InGameEventManager::EVENT_TYPE_SIEGE_WAR);
	PyModule_AddIntConstant(poModule, "EVENT_TYPE_KINGDOM_WAR",				InGameEventManager::EVENT_TYPE_KINGDOM_WAR);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	PyModule_AddIntConstant(poModule, "EVENT_BATTLE_PASS_EVENT",			InGameEventManager::EVENT_BATTLE_PASS_EVENT);
	PyModule_AddIntConstant(poModule, "EVENT_BATTLE_PASS_NORMAL",			InGameEventManager::EVENT_BATTLE_PASS_NORMAL);
	PyModule_AddIntConstant(poModule, "EVENT_BATTLE_PASS_PREMIUM",			InGameEventManager::EVENT_BATTLE_PASS_PREMIUM);
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	PyModule_AddIntConstant(poModule, "EVENT_LOTTERY",						InGameEventManager::EVENT_LOTTERY);
#endif

	PyModule_AddIntConstant(poModule, "EVENT_MAX_NUM",						InGameEventManager::EVENT_MAX_NUM);
	PyModule_AddIntConstant(poModule, "DAY_MAX_NUM",						InGameEventManager::DAY_MAX_NUM);
	PyModule_AddIntConstant(poModule, "REWARD_MAX_NUM",						InGameEventManager::REWARD_MAX_NUM);
#endif
}
