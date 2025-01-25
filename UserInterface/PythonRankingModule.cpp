#include "StdAfx.h"
#include "PythonRanking.h"

#ifdef ENABLE_RANKING_SYSTEM
enum __RE_20200724_1__
{
	TYPE_RK_SOLO,
	TYPE_RK_PARTY,
	TYPE_RK_MAX,
};

enum __RE_20200724_2__
{
	SOLO_RK_CATEGORY_BF_WEAK = 0,
	SOLO_RK_CATEGORY_BF_TOTAL = 1,
	SOLO_RK_CATEGORY_MD_RED = 2,
	SOLO_RK_CATEGORY_MD_BLUE = 3,
	// UNK_4
	// UNK_5
	SOLO_RK_CATEGORY_BNW = 6,
	SOLO_RK_CATEGORY_WORLD_BOSS = 7,

	SOLO_RK_CATEGORY_MAX,
};

enum __RE_20200724_3__
{
	// ENABLE_GUILD_DRAGONLAIR_SYSTEM
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL = 0,
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK = 1,
	PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK = 2,
	// ENABLE_12ZI
	PARTY_RK_CATEGORY_CZ_MOUSE = 3,
	PARTY_RK_CATEGORY_CZ_COW = 4,
	PARTY_RK_CATEGORY_CZ_TIGER = 5,
	PARTY_RK_CATEGORY_CZ_RABBIT = 6,
	PARTY_RK_CATEGORY_CZ_DRAGON = 7,
	PARTY_RK_CATEGORY_CZ_SNAKE = 8,
	PARTY_RK_CATEGORY_CZ_HORSE = 9,
	PARTY_RK_CATEGORY_CZ_SHEEP = 10,
	PARTY_RK_CATEGORY_CZ_MONKEY = 11,
	PARTY_RK_CATEGORY_CZ_CHICKEN = 12,
	PARTY_RK_CATEGORY_CZ_DOG = 13,
	PARTY_RK_CATEGORY_CZ_PIG = 14,
	// ENABLE_DEFENSE_WAVE
	PARTY_RK_CATEGORY_DEFENSE_WAVE = 15,
	//
	PARTY_RK_CATEGORY_MAX, // 16
};

enum __RE_20200724_UNK__
{
	HIGH_RANK_VIEW_MAX = 10,
};

PyObject* rankingGetHighRankingInfoSolo(PyObject* poSelf, PyObject* poArgs)
{
	int iCategory;
	if (!PyTuple_GetInteger(poArgs, 0, &iCategory))
		return Py_BadArgument();

	int iLine;
	if (!PyTuple_GetInteger(poArgs, 1, &iLine))
		return Py_BadArgument();

	if (iLine > 10)
		return Py_BadArgument();

	const CPythonRanking::TBattleFieldRanking& cVec = CPythonRanking::Instance().GetBattleFieldRanking();

	for (auto& e : cVec)
	{
		if (e.bType != iCategory)
			continue;

		if (e.bPosition != iLine)
			continue;

		return Py_BuildValue("siiii", e.szName, e.lScore, 0, 0, e.bEmpire);
	}

	// charname, record0, record1, time, empire
	return Py_BuildValue("siiii", "", 0, 0, 0, 0);
}

PyObject* rankingGetMyRankingInfoSolo(PyObject* poSelf, PyObject* poArgs)
{
	//TraceError("rankingGetMyRankingInfoSolo");
	// charname, record0, record1, time, empire
	return Py_BuildValue("isiiii", 0, "", 0, 0, 0, 0);
}

void initRanking() noexcept
{
	static PyMethodDef s_methods[] = {/*{"GetHighRankingInfoParty", rankingGetHighRankingInfoParty, METH_VARARGS},
									  {"GetHighRankingInfoSolo", rankingGetHighRankingInfoSolo, METH_VARARGS},
									  {"GetHighRankingInfoSoloWithGuildName", rankingGetHighRankingInfoSoloWithGuildName, METH_VARARGS},
									  {"GetMyPartyMemberName", rankingGetMyPartyMemberName, METH_VARARGS},
									  {"GetMyRankingInfoParty", rankingGetMyRankingInfoParty, METH_VARARGS},
									  {"GetMyRankingInfoSolo", rankingGetMyRankingInfoSolo, METH_VARARGS},
									  {"GetMyRankingInfoSoloWithGuildName", rankingGetMyRankingInfoSoloWithGuildName, METH_VARARGS},
									  {"GetPartyMemberName", rankingGetPartyMemberName, METH_VARARGS},
									  {"GetRankingInfoSoloSize", rankingGetRankingInfoSoloSize, METH_VARARGS},*/

									  {"GetHighRankingInfoSolo", rankingGetHighRankingInfoSolo, METH_VARARGS},
									  {"GetMyRankingInfoSolo", rankingGetMyRankingInfoSolo, METH_VARARGS},
									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("ranking", s_methods);

	PyModule_AddIntConstant(poModule, "TYPE_RK_SOLO", TYPE_RK_SOLO);
	PyModule_AddIntConstant(poModule, "TYPE_RK_PARTY", TYPE_RK_PARTY);
	PyModule_AddIntConstant(poModule, "TYPE_RK_MAX", TYPE_RK_MAX);

	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_BF_WEAK", SOLO_RK_CATEGORY_BF_WEAK);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_BF_TOTAL", SOLO_RK_CATEGORY_BF_TOTAL);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_MD_RED", SOLO_RK_CATEGORY_MD_RED);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_MD_BLUE", SOLO_RK_CATEGORY_MD_BLUE);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_BNW", SOLO_RK_CATEGORY_BNW);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_WORLD_BOSS", SOLO_RK_CATEGORY_WORLD_BOSS);
	PyModule_AddIntConstant(poModule, "SOLO_RK_CATEGORY_MAX", SOLO_RK_CATEGORY_MAX);

	// ENABLE_GUILD_DRAGONLAIR_SYSTEM
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL", PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_ALL);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK", PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_NOW_WEEK);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK", PARTY_RK_CATEGORY_GUILD_DRAGONLAIR_RED_PAST_WEEK);
	// ENABLE_12ZI
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_MOUSE", PARTY_RK_CATEGORY_CZ_MOUSE);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_COW", PARTY_RK_CATEGORY_CZ_COW);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_TIGER", PARTY_RK_CATEGORY_CZ_TIGER);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_RABBIT", PARTY_RK_CATEGORY_CZ_RABBIT);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_DRAGON", PARTY_RK_CATEGORY_CZ_DRAGON);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_SNAKE", PARTY_RK_CATEGORY_CZ_SNAKE);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_HORSE", PARTY_RK_CATEGORY_CZ_HORSE);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_SHEEP", PARTY_RK_CATEGORY_CZ_SHEEP);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_MONKEY", PARTY_RK_CATEGORY_CZ_MONKEY);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_CHICKEN", PARTY_RK_CATEGORY_CZ_CHICKEN);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_DOG", PARTY_RK_CATEGORY_CZ_DOG);
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_CZ_PIG", PARTY_RK_CATEGORY_CZ_PIG);
	// ENABLE_DEFENSE_WAVE
	PyModule_AddIntConstant(poModule, "PARTY_RK_CATEGORY_DEFENSE_WAVE", PARTY_RK_CATEGORY_DEFENSE_WAVE);
}
#endif
