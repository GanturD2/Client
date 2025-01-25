#include "StdAfx.h"
#include "PythonNonPlayer.h"

#include "InstanceBase.h"
#include "PythonCharacterManager.h"

PyObject * nonplayerGetEventType(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualNumber))
		return Py_BuildException();

	uint8_t iType = CPythonNonPlayer::Instance().GetEventType(iVirtualNumber);

	return Py_BuildValue("i", iType);
}

PyObject * nonplayerGetEventTypeByVID(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	uint8_t iType = CPythonNonPlayer::Instance().GetEventTypeByVID(iVirtualID);

	return Py_BuildValue("i", iType);
}

PyObject * nonplayerGetLevelByVID(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

#ifdef WJ_SHOW_MOB_INFO
	uint32_t dwLevel = pInstance->GetLevel();
	return Py_BuildValue("i", dwLevel);
#else
	const CPythonNonPlayer::TMobTable * pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	float fAverageLevel = pMobTable->bLevel; //(float(pMobTable->abLevelRange[0]) + float(pMobTable->abLevelRange[1])) / 2.0f;
	fAverageLevel = floor(fAverageLevel + 0.5f);
	return Py_BuildValue("i", int(fAverageLevel));
#endif
}

PyObject * nonplayerGetGradeByVID(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable * pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pMobTable->bRank);
}


PyObject * nonplayerGetMonsterName(PyObject * poSelf, PyObject * poArgs)
{
	int iVNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
		return Py_BuildException();

	CPythonNonPlayer & rkNonPlayer = CPythonNonPlayer::Instance();
	return Py_BuildValue("s", rkNonPlayer.GetMonsterName(iVNum));
}

PyObject * nonplayerLoadNonPlayerData(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNonPlayer::Instance().LoadNonPlayerData(szFileName);
	return Py_BuildNone();
}

#ifdef ENABLE_SEND_TARGET_INFO
PyObject* nonplayerGetMonsterMaxHP(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterMaxHP(race));
}

PyObject* nonplayerGetRaceNumByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

	if (!pInstance)
		return Py_BuildValue("i", -1);

	const CPythonNonPlayer::TMobTable* pMobTable = CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

	if (!pMobTable)
		return Py_BuildValue("i", -1);

	return Py_BuildValue("i", pMobTable->dwVnum);
}

PyObject* nonplayerGetMonsterRaceFlag(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterRaceFlag(race));
}

PyObject* nonplayerGetMonsterLevel(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterLevel(race));
}

PyObject* nonplayerGetMonsterDamage(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	uint32_t dmg1 = rkNonPlayer.GetMonsterDamage1(race);
	uint32_t dmg2 = rkNonPlayer.GetMonsterDamage2(race);

	return Py_BuildValue("ii", dmg1, dmg2);
}

PyObject* nonplayerGetMonsterExp(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterExp(race));
}

PyObject* nonplayerGetMonsterDamageMultiply(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("f", rkNonPlayer.GetMonsterDamageMultiply(race));
}

PyObject* nonplayerGetMonsterST(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterST(race));
}

PyObject* nonplayerGetMonsterDX(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMonsterDX(race));
}

PyObject* nonplayerIsMonsterStone(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.IsMonsterStone(race) ? 1 : 0);
}

PyObject* nonplayerGetMobRegenCycle(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobRegenCycle(race));
}

PyObject* nonplayerGetMobRegenPercent(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobRegenPercent(race));
}

PyObject* nonplayerGetMobGoldMin(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobGoldMin(race));
}

PyObject* nonplayerGetMobGoldMax(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobGoldMax(race));
}

PyObject* nonplayerGetMobResist(PyObject* poSelf, PyObject* poArgs)
{
	int race;
	if (!PyTuple_GetInteger(poArgs, 0, &race))
		return Py_BuildException();

	uint8_t resistNum;
	if (!PyTuple_GetInteger(poArgs, 1, &resistNum))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();

	return Py_BuildValue("i", rkNonPlayer.GetMobResist(race, resistNum));
}
#endif

#ifdef ENABLE_ELEMENT_ADD
PyObject* nonplayerGetElementEnchantsByVID(PyObject* poSelf, PyObject* poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BuildException();

	auto& rkNonPlayer = CPythonNonPlayer::Instance();
	auto oElementsDict = PyDict_New();
	if (!oElementsDict)
		Py_BuildNone();

	const auto dwRaceNum = rkNonPlayer.GetRaceNumByVID(iVirtualID);
	for (uint8_t i = 0; i < CPythonNonPlayer::MOB_ELEMENTAL_MAX_NUM; i++)
		PyDict_SetItem(oElementsDict, Py_BuildValue("i", i), Py_BuildValue("i", rkNonPlayer.GetMonsterEnchantElement(dwRaceNum, i)));

	return oElementsDict;
}

PyObject* nonplayerGetMonsterEnchantElement(PyObject* poSelf, PyObject* poArgs)
{
	int iRace;
	if (!PyTuple_GetInteger(poArgs, 0, &iRace))
		return Py_BuildException();

	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 1, &iVirtualID))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	return Py_BuildValue("i", rkNonPlayer.GetMonsterEnchantElement(iRace, iVirtualID));
}
#endif

#ifdef ENABLE_INGAME_WIKI
PyObject* nonplayerGetMonsterDataByNamePart(PyObject* poSelf, PyObject* poArgs)
{
	char* szNamePart;
	if (!PyTuple_GetString(poArgs, 0, &szNamePart))
		return Py_BadArgument();

	std::tuple<const char*, int> mobData = CPythonNonPlayer::Instance().GetMonsterDataByNamePart(szNamePart);
	return Py_BuildValue("(si)", std::get<0>(mobData), std::get<1>(mobData));
}

PyObject* nonplayerBuildWikiSearchList(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNonPlayer::Instance().BuildWikiSearchList();
	return Py_BuildNone();
}

PyObject* nonplayerGetMonsterImmuneFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iVNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	const CPythonNonPlayer::TMobTable* pkTab = rkNonPlayer.GetTable(iVNum);

	return Py_BuildValue("i", pkTab->dwImmuneFlag);
}

PyObject* nonplayerGetMonsterResistValue(PyObject* poSelf, PyObject* poArgs)
{
	int iVNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
		return Py_BuildException();

	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	if (iType < 0 || iType > CPythonNonPlayer::MOB_RESISTS_MAX_NUM)
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	const CPythonNonPlayer::TMobTable* pkTab = rkNonPlayer.GetTable(iVNum);

	return Py_BuildValue("i", pkTab->cResists[iType]);
}

PyObject* nonplayerGetMonsterGold(PyObject* poSelf, PyObject* poArgs)
{
	int iVNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
		return Py_BuildException();

	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	const CPythonNonPlayer::TMobTable* pkTab = rkNonPlayer.GetTable(iVNum);

	return Py_BuildValue("ii", pkTab->dwGoldMin, pkTab->dwGoldMax);
}
#endif

void initNonPlayer()
{
	static PyMethodDef s_methods[] = {{"GetEventType", nonplayerGetEventType, METH_VARARGS},
									  {"GetEventTypeByVID", nonplayerGetEventTypeByVID, METH_VARARGS},
									  {"GetLevelByVID", nonplayerGetLevelByVID, METH_VARARGS},
									  {"GetGradeByVID", nonplayerGetGradeByVID, METH_VARARGS},
									  {"GetMonsterName", nonplayerGetMonsterName, METH_VARARGS},

#ifdef ENABLE_SEND_TARGET_INFO
		// TARGET_INFO
		{ "GetRaceNumByVID",			nonplayerGetRaceNumByVID,			METH_VARARGS },
		{ "GetMonsterMaxHP",			nonplayerGetMonsterMaxHP,			METH_VARARGS },
		{ "GetMonsterRaceFlag",			nonplayerGetMonsterRaceFlag,		METH_VARARGS },
		{ "GetMonsterLevel",			nonplayerGetMonsterLevel,			METH_VARARGS },
		{ "GetMonsterDamage",			nonplayerGetMonsterDamage,			METH_VARARGS },
		{ "GetMonsterExp",				nonplayerGetMonsterExp,				METH_VARARGS },
		{ "GetMonsterDamageMultiply",	nonplayerGetMonsterDamageMultiply,	METH_VARARGS },
		{ "GetMonsterST",				nonplayerGetMonsterST,				METH_VARARGS },
		{ "GetMonsterDX",				nonplayerGetMonsterDX,				METH_VARARGS },
		{ "IsMonsterStone",				nonplayerIsMonsterStone,			METH_VARARGS },
		{ "GetMobRegenCycle", nonplayerGetMobRegenCycle, METH_VARARGS },
		{ "GetMobRegenPercent", nonplayerGetMobRegenPercent, METH_VARARGS },
		{ "GetMobGoldMin", nonplayerGetMobGoldMin, METH_VARARGS },
		{ "GetMobGoldMax", nonplayerGetMobGoldMax, METH_VARARGS },
		{ "GetResist", nonplayerGetMobResist, METH_VARARGS },
#endif

									  {"LoadNonPlayerData", nonplayerLoadNonPlayerData, METH_VARARGS},

#ifdef ENABLE_ELEMENT_ADD
		{"GetElementEnchantsByVID", nonplayerGetElementEnchantsByVID, METH_VARARGS},
		{"GetMonsterEnchantElement", nonplayerGetMonsterEnchantElement, METH_VARARGS},
#endif

#ifdef ENABLE_INGAME_WIKI
		{ "GetMonsterDataByNamePart",	nonplayerGetMonsterDataByNamePart,	METH_VARARGS },
		{ "BuildWikiSearchList",		nonplayerBuildWikiSearchList,		METH_VARARGS },

		{ "GetMonsterImmuneFlag",		nonplayerGetMonsterImmuneFlag,		METH_VARARGS },
		{ "GetMonsterResistValue",		nonplayerGetMonsterResistValue,		METH_VARARGS },
		{ "GetMonsterGold",				nonplayerGetMonsterGold,			METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("nonplayer", s_methods);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_NONE",		CPythonNonPlayer::ON_CLICK_EVENT_NONE);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_BATTLE",		CPythonNonPlayer::ON_CLICK_EVENT_BATTLE);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_SHOP",		CPythonNonPlayer::ON_CLICK_EVENT_SHOP);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_TALK",		CPythonNonPlayer::ON_CLICK_EVENT_TALK);
	PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_VEHICLE",		CPythonNonPlayer::ON_CLICK_EVENT_VEHICLE);

	PyModule_AddIntConstant(poModule, "PAWN",			CPythonNonPlayer::MOB_RANK_PAWN);
	PyModule_AddIntConstant(poModule, "S_PAWN",			CPythonNonPlayer::MOB_RANK_S_PAWN);
	PyModule_AddIntConstant(poModule, "KNIGHT",			CPythonNonPlayer::MOB_RANK_KNIGHT);
	PyModule_AddIntConstant(poModule, "S_KNIGHT",		CPythonNonPlayer::MOB_RANK_S_KNIGHT);
	PyModule_AddIntConstant(poModule, "BOSS",			CPythonNonPlayer::MOB_RANK_BOSS);
	PyModule_AddIntConstant(poModule, "KING",			CPythonNonPlayer::MOB_RANK_KING);
#ifdef ENABLE_SEND_TARGET_INFO
#	ifdef ENABLE_PROTO_RENEWAL_CUSTOM
	PyModule_AddIntConstant(poModule, "MOB_RESIST_FIST", CPythonNonPlayer::MOB_RESIST_FIST);
#	endif
	PyModule_AddIntConstant(poModule, "MOB_RESIST_SWORD", CPythonNonPlayer::MOB_RESIST_SWORD);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_TWOHAND", CPythonNonPlayer::MOB_RESIST_TWOHAND);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_DAGGER", CPythonNonPlayer::MOB_RESIST_DAGGER);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_BELL", CPythonNonPlayer::MOB_RESIST_BELL);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_FAN", CPythonNonPlayer::MOB_RESIST_FAN);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_BOW", CPythonNonPlayer::MOB_RESIST_BOW);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_FIRE", CPythonNonPlayer::MOB_RESIST_FIRE);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_ELECT", CPythonNonPlayer::MOB_RESIST_ELECT);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_MAGIC", CPythonNonPlayer::MOB_RESIST_MAGIC);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_WIND", CPythonNonPlayer::MOB_RESIST_WIND);
	PyModule_AddIntConstant(poModule, "MOB_RESIST_POISON", CPythonNonPlayer::MOB_RESIST_POISON);
#	if defined(ENABLE_PROTO_RENEWAL) && defined(ENABLE_WOLFMAN_CHARACTER)
	PyModule_AddIntConstant(poModule, "MOB_RESIST_CLAW", CPythonNonPlayer::MOB_RESIST_CLAW);		//ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "MOB_RESIST_BLEEDING", CPythonNonPlayer::MOB_RESIST_BLEEDING);	//ENABLE_WOLFMAN_CHARACTER
#	endif
	PyModule_AddIntConstant(poModule, "MOB_RESIST_MAX_NUM", CPythonNonPlayer::MOB_RESISTS_MAX_NUM);
#endif

#ifdef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_ELECT", CPythonNonPlayer::MOB_ELEMENTAL_ELEC);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_FIRE", CPythonNonPlayer::MOB_ELEMENTAL_FIRE);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_ICE", CPythonNonPlayer::MOB_ELEMENTAL_ICE);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_WIND", CPythonNonPlayer::MOB_ELEMENTAL_WIND);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_EARTH", CPythonNonPlayer::MOB_ELEMENTAL_EARTH);
	PyModule_AddIntConstant(poModule, "MOB_ENCHANT_DARK", CPythonNonPlayer::MOB_ELEMENTAL_DARK);
	PyModule_AddIntConstant(poModule, "MOB_ELEMENTAL_MAX_NUM", CPythonNonPlayer::MOB_ELEMENTAL_MAX_NUM);
#endif

#ifdef ENABLE_INGAME_WIKI
	PyModule_AddIntConstant(poModule, "IMMUNE_STUN", CItemData::IMMUNE_STUN);
	PyModule_AddIntConstant(poModule, "IMMUNE_SLOW", CItemData::IMMUNE_SLOW);
	PyModule_AddIntConstant(poModule, "IMMUNE_CURSE", CItemData::IMMUNE_CURSE);
	PyModule_AddIntConstant(poModule, "IMMUNE_POISON", CItemData::IMMUNE_POISON);
	PyModule_AddIntConstant(poModule, "IMMUNE_TERROR", CItemData::IMMUNE_TERROR);
	PyModule_AddIntConstant(poModule, "IMMUNE_FLAG_MAX_NUM", CItemData::IMMUNE_FLAG_MAX_NUM);

	PyModule_AddIntConstant(poModule, "RACE_FLAG_ANIMAL", CPythonNonPlayer::RACE_FLAG_ANIMAL);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_UNDEAD", CPythonNonPlayer::RACE_FLAG_UNDEAD);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_DEVIL", CPythonNonPlayer::RACE_FLAG_DEVIL);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_HUMAN", CPythonNonPlayer::RACE_FLAG_HUMAN);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_ORC", CPythonNonPlayer::RACE_FLAG_ORC);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_MILGYO", CPythonNonPlayer::RACE_FLAG_MILGYO);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_INSECT", CPythonNonPlayer::RACE_FLAG_INSECT);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_DESERT", CPythonNonPlayer::RACE_FLAG_DESERT);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_TREE", CPythonNonPlayer::RACE_FLAG_TREE);
	PyModule_AddIntConstant(poModule, "RACE_FLAG_MAX_NUM", CPythonNonPlayer::RACE_FLAG_MAX_NUM);
#endif
}