// vim:ts=4 sw=4
#include <map>
#include "stdafx.h"
#include "ClientManager.h"
#include "Main.h"
#include "Monarch.h"
#include "CsvReader.h"
#include "ProtoReader.h"

using namespace std;

extern int g_test_server;
extern std::string g_stLocaleNameColumn;

bool CClientManager::InitializeTables()
{
#ifdef ENABLE_PROTO_FROM_DB
	if (!(bIsProtoReadFromDB ? InitializeMobTableFromDB() : InitializeMobTable()))
#else
	if (!InitializeMobTable())
#endif
	{
		sys_err("InitializeMobTable FAILED");
		return false;
	}
#ifdef ENABLE_PROTO_FROM_DB
	if (!(bIsProtoReadFromDB ? InitializeItemTableFromDB() : InitializeItemTable()))	// false [InitializeItemTable]
#else
	if (!InitializeItemTable())
#endif
	{
		sys_err("InitializeItemTable FAILED");
		return false;
	}

#ifdef ENABLE_PROTO_FROM_DB
	extern bool g_bMirror2DB;
	if (g_bMirror2DB)	// true
	{
		if (!MirrorMobTableIntoDB())
		{
			sys_err("MirrorMobTableIntoDB FAILED");
			return false;
		}
		if (!MirrorItemTableIntoDB())
		{
			sys_err("MirrorItemTableIntoDB FAILED");
			return false;
		}
	}
#endif

	if (!InitializeShopTable())
	{
		sys_err("InitializeShopTable FAILED");
		return false;
	}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	if (!InitializeGemShopTable())
	{
		sys_err("InitializeGemShopTable FAILED");
		return false;
	}
#endif

#ifdef ENABLE_MAILBOX
	if (!InitializeMailBoxTable())
	{
		sys_err("InitializeMailBoxTable FAILED");
		return false;
	}
#endif

	if (!InitializeSkillTable())
	{
		sys_err("InitializeSkillTable FAILED");
		return false;
	}

	if (!InitializeRefineTable())
	{
		sys_err("InitializeRefineTable FAILED");
		return false;
	}

	if (!InitializeItemAttrTable())
	{
		sys_err("InitializeItemAttrTable FAILED");
		return false;
	}

	if (!InitializeItemRareTable())
	{
		sys_err("InitializeItemRareTable FAILED");
		return false;
	}

	if (!InitializeBanwordTable())
	{
		sys_err("InitializeBanwordTable FAILED");
		return false;
	}

	if (!InitializeLandTable())
	{
		sys_err("InitializeLandTable FAILED");
		return false;
	}

	if (!InitializeObjectProto())
	{
		sys_err("InitializeObjectProto FAILED");
		return false;
	}

	if (!InitializeObjectTable())
	{
		sys_err("InitializeObjectTable FAILED");
		return false;
	}

	if (!InitializeMonarch())
	{
		sys_err("InitializeMonarch FAILED");
		return false;
	}

#ifdef ENABLE_EVENT_MANAGER
	if (!InitializeEventTable())
	{
		sys_err("InitializeEventTable FAILED");
		return false;
	}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	if (!InitializeBiologMissions())
	{
		sys_err("InitializeBiologMissions FAILED");
		return false;
	}

	if (!InitializeBiologRewards())
	{
		sys_err("InitializeBiologRewards FAILED");
		return false;
	}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	if (!InitializeSungmaTable())
	{
		sys_err("InitializeSungmaTable FAILED");
		return false;
	}
#endif

	return true;
}

bool CClientManager::InitializeRefineTable()
{
	char query[2048];

	snprintf(query, sizeof(query),
		"SELECT id, cost, prob, vnum0, count0, vnum1, count1, vnum2, count2, vnum3, count3, vnum4, count4 FROM refine_proto%s",
		GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
		return true;

	if (m_pRefineTable)
	{
		sys_log(0, "RELOAD: refine_proto");
		delete[] m_pRefineTable;
		m_pRefineTable = nullptr;
	}

	m_iRefineTableSize = static_cast<int>(pRes->uiNumRows);

	m_pRefineTable = new TRefineTable[m_iRefineTableSize];
	memset(m_pRefineTable, 0, sizeof(TRefineTable) * m_iRefineTableSize);

	TRefineTable* prt = m_pRefineTable;
	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		//const char* s_szQuery = "SELECT src_vnum, result_vnum, cost, prob, "
		//"vnum0, count0, vnum1, count1, vnum2, count2, vnum3, count3, vnum4, count4 "

		int col = 0;
		//prt->src_vnum = atoi(data[col++]);
		//prt->result_vnum = atoi(data[col++]);
		str_to_number(prt->id, data[col++]);
		str_to_number(prt->cost, data[col++]);
		str_to_number(prt->prob, data[col++]);

		prt->material_count = REFINE_MATERIAL_MAX_NUM; // @fixme185
		for (int i = 0; i < REFINE_MATERIAL_MAX_NUM; i++)
		{
			str_to_number(prt->materials[i].vnum, data[col++]);
			str_to_number(prt->materials[i].count, data[col++]);
			if (prt->materials[i].vnum == 0)
			{
				prt->material_count = i;
				break;
			}
		}

		sys_log(0, "REFINE: id %ld cost %d prob %d mat1 %lu cnt1 %d", prt->id, prt->cost, prt->prob, prt->materials[0].vnum, prt->materials[0].count);

		prt++;
	}
	return true;
}

class FCompareVnum
{
public:
	bool operator () (const TEntityTable& a, const TEntityTable& b) const
	{
		return (a.dwVnum < b.dwVnum);
	}
};

bool CClientManager::InitializeMobTable()
{
	/*=================== Function Description ===================
	1. Summary: read files 'mob_proto.txt', 'mob_proto_test.txt', 'mob_names.txt',
		(!)[mob_table] Create a table object. (Type: TMobTable)
	2. order
		1) Read the 'mob_names.txt' file and create a map (a)[localMap](vnum:name).
		2) 'mob_proto_test.txt' file and (a)[localMap] map
		(b)[test_map_mobTableByVnum](vnum:TMobTable) Create a map.
		3) 'mob_proto.txt' file and (a)[localMap] map
		Create the (!)[mob_table] table.
		<Note>
		in each row,
		Rows in both (b)[test_map_mobTableByVnum] and (!)[mob_table] are
		(b) Use the one in [test_map_mobTableByVnum].
		4) Among the rows of (b)[test_map_mobTableByVnum], add the row that is not in (!)[mob_table].
	3. test
		1) Check if 'mob_proto.txt' information is properly entered into mob_table. -> Done
		2) Check if 'mob_names.txt' information is properly entered into mob_table.
		3) Check if the [overlapping] information from 'mob_proto_test.txt' is properly entered into mob_table.
		4) Check if the [new] information from 'mob_proto_test.txt' is properly entered into mob_table.
		5) Make sure it works properly in the (final) game client.
	_______________________________________________*/

	//===============================================//
	// 1) Read the 'mob_names.txt' file and create (a)[localMap] map.
	//<(a)create localMap map>
	map<int, const char*> localMap;
	//bool isNameFile = true;
	//<read file>
	cCsvTable nameData;
	if (!nameData.Load("mob_names.txt", '\t'))
	{
		fprintf(stderr, "Could not load mob_names.txt\n");
	}
	else
	{
		nameData.Next(); // Omit the description row.
		while (nameData.Next()) {
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}
	//________________________________________________//

	cCsvTable data;

	if (!data.Load("mob_proto.txt", '\t'))
	{
		fprintf(stderr, "Could not load mob_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next(); //Except for the top row (the part that describes the item column)
	//2.2 Create mob_table to fit size
	if (!m_vec_mobTable.empty())
	{
		sys_log(0, "RELOAD: mob_proto");
		m_vec_mobTable.clear();
	}
	m_vec_mobTable.resize(data.m_File.GetRowCount() - 1);
	memset(&m_vec_mobTable[0], 0, sizeof(TMobTable) * m_vec_mobTable.size());
	TMobTable* mob_table = &m_vec_mobTable[0];
	//2.3 Fill data
	while (data.Next())
	{
		if (!Set_Proto_Mob_Table(mob_table, data, localMap))
			fprintf(stderr, "Could not process entry.\n");

		sys_log(1, "MOB #%-5d %-24s %-24s level: %-3u rank: %u empire: %d", mob_table->dwVnum, mob_table->szName, mob_table->szLocaleName, mob_table->bLevel, mob_table->bRank, mob_table->bEmpire);
		++mob_table;
	}
	//_____________________________________________________//

	sort(m_vec_mobTable.begin(), m_vec_mobTable.end(), FCompareVnum());
	return true;
}

bool CClientManager::InitializeShopTable()
{
	MYSQL_ROW data;
	int col;

	static const char* s_szQuery =
		"SELECT "
		"shop.vnum, "
		"shop.npc_vnum, "
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		"shop.limited+0, "
#endif
		"shop_item.item_vnum, "
		"shop_item.count "
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		", shop_item.purchase_limit "
#endif
		"FROM shop LEFT JOIN shop_item "
		"ON shop.vnum = shop_item.shop_vnum ORDER BY shop.vnum, shop_item.item_vnum";

	auto pkMsg2(CDBManager::Instance().DirectQuery(s_szQuery));

	// If shop's vnum exists but shop_item does not exist... Please note that it is treated as a failure.
	// part to fix
	SQLResult* pRes2 = pkMsg2->Get();

	if (!pRes2->uiNumRows)
	{
		sys_err("InitializeShopTable : Table count is zero.");
		return false;
	}

	std::map<int, TShopTable*> map_shop;

	if (m_pShopTable)
	{
		delete[](m_pShopTable);
		m_pShopTable = nullptr;
	}

	TShopTable* shop_table = m_pShopTable;

	while ((data = mysql_fetch_row(pRes2->pSQLResult)))
	{
		col = 0;

		int iShopVnum = 0;
		str_to_number(iShopVnum, data[col++]);

		if (map_shop.end() == map_shop.find(iShopVnum))
		{
			shop_table = new TShopTable{};
			shop_table->dwVnum = iShopVnum;

			map_shop[iShopVnum] = shop_table;
		}
		else
			shop_table = map_shop[iShopVnum];

		str_to_number(shop_table->dwNPCVnum, data[col++]);
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		shop_table->is_limited = atoi(data[col++]) - 1;
#endif

		if (!data[col]) // If there is no item, nullptr is returned.
			continue;

		TShopItemTable* pItem = &shop_table->items[shop_table->byItemCount];

		str_to_number(pItem->vnum, data[col++]);
		str_to_number(pItem->count, data[col++]);
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		str_to_number(pItem->purchase_limit, data[col++]);
#endif

		++shop_table->byItemCount;
	}

	m_pShopTable = new TShopTable[map_shop.size()];
	m_iShopTableSize = map_shop.size();

	typeof(map_shop.begin()) it = map_shop.begin();

	int i = 0;

	while (it != map_shop.end())
	{
		thecore_memcpy((m_pShopTable + i), (it++)->second, sizeof(TShopTable));
		sys_log(0, "SHOP: #%d items: %d", (m_pShopTable + i)->dwVnum, (m_pShopTable + i)->byItemCount);
		++i;
	}

	return true;
}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
bool CClientManager::InitializeGemShopTable()
{
	MYSQL_ROW data;
	int col;

	static const char* s_szQuery =
		"SELECT "
		"id, "
		"vnum, "
		"count, "
		"price, "
		"row "
		"FROM gem_shop ORDER BY id";

	auto pkMsg2(CDBManager::Instance().DirectQuery(s_szQuery));

	SQLResult* pRes2 = pkMsg2->Get();

	if (!pRes2->uiNumRows)
	{
		sys_err("InitializeGemShopTable : Table count is zero.");
		return false;
	}

	std::map<int, TGemShopTable*> map_shop;

	if (m_pGemShopTable)
	{
		delete[](m_pGemShopTable);
		m_pGemShopTable = nullptr;
	}

	TGemShopTable* gem_shop_table = m_pGemShopTable;

	while ((data = mysql_fetch_row(pRes2->pSQLResult)))
	{
		col = 0;

		int iShopId = 0;
		str_to_number(iShopId, data[col++]);

		if (map_shop.end() == map_shop.find(iShopId))
		{
			gem_shop_table = new TGemShopTable;
			memset(gem_shop_table, 0, sizeof(TGemShopTable));

			str_to_number(gem_shop_table->dwVnum, data[col++]);
			str_to_number(gem_shop_table->bCount, data[col++]);
			str_to_number(gem_shop_table->dwPrice, data[col++]);
			str_to_number(gem_shop_table->dwRow, data[col++]);

			map_shop[iShopId] = gem_shop_table;
		}
		else
		{
			gem_shop_table = map_shop[iShopId];
		}

		if (!data[col]) // If there is no item, NULL is returned.
			continue;
	}

	m_pGemShopTable = new TGemShopTable[map_shop.size()];
	m_iGemShopTableSize = map_shop.size();

	typeof(map_shop.begin()) it = map_shop.begin();

	int i = 0;

	while (it != map_shop.end())
	{
		thecore_memcpy((m_pGemShopTable + i), (it++)->second, sizeof(TGemShopTable));
		sys_log(0, "GEM_SHOP: #%d items: %d", (m_pGemShopTable + i)->dwVnum, (m_pGemShopTable + i)->bCount);
		++i;
	}

	return true;
}
#endif

#ifdef ENABLE_MAILBOX
#include <cstring>
bool CClientManager::InitializeMailBoxTable()
{
	if (m_map_mailbox.empty())
		return true;

	char s_szQuery[512];
	int len = sprintf(s_szQuery, "SELECT name, who, title, message, gm, confirm, send_time, delete_time, gold, won, ivnum, icount, "
		"socket0, socket1, socket2, "
#ifdef ENABLE_PROTO_RENEWAL
		"socket3, socket4, socket5, "
#endif
		"attrtype0, attrvalue0, "
		"attrtype1, attrvalue1, "
		"attrtype2, attrvalue2, "
		"attrtype3, attrvalue3, "
		"attrtype4, attrvalue4, "
		"attrtype5, attrvalue5, "
		"attrtype6, attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
		", randomtype0, randomvalue0, randomrow0"
		", randomtype1, randomvalue1, randomrow1"
		", randomtype2, randomvalue2, randomrow2"
		", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		", transmutation"
#endif
#ifdef ENABLE_REFINE_ELEMENT
		", grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3"
#endif
#ifdef ENABLE_SET_ITEM
		", setItem"
#endif
		" FROM mailbox%s", GetTablePostfix()
	);

	auto pkMsg(CDBManager::Instance().DirectQuery(s_szQuery));

	const SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
		return true;

	MYSQL_ROW data;
	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		uint8_t col = 0;
		SMailBoxTable mail;

		auto name = data[col++];
		mail.bIsDeleted = false;
		mail.AddData.bHeader = 0;
		mail.AddData.Index = 0;
		std::memcpy(mail.szName, name, sizeof(mail.szName));
		std::memcpy(mail.AddData.szFrom, data[col++], sizeof(mail.AddData.szFrom));
		std::memcpy(mail.Message.szTitle, data[col++], sizeof(mail.Message.szTitle));
		std::memcpy(mail.AddData.szMessage, data[col++], sizeof(mail.AddData.szMessage));
		str_to_number(mail.Message.bIsGMPost, data[col++]);
		str_to_number(mail.Message.bIsConfirm, data[col++]);
		str_to_number(mail.Message.SendTime, data[col++]);
		str_to_number(mail.Message.DeleteTime, data[col++]);
		str_to_number(mail.AddData.iYang, data[col++]);
		str_to_number(mail.AddData.iWon, data[col++]);
		str_to_number(mail.AddData.ItemVnum, data[col++]);
		str_to_number(mail.AddData.ItemCount, data[col++]);
		mail.Message.bIsItemExist = mail.AddData.ItemVnum > 0 || mail.AddData.iYang > 0 || mail.AddData.iWon > 0;

		for (uint8_t i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
			str_to_number(mail.AddData.alSockets[i], data[col++]);

		for (uint8_t a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++) {
			str_to_number(mail.AddData.aAttr[a].wType, data[col++]);
			str_to_number(mail.AddData.aAttr[a].sValue, data[col++]);
		}

#ifdef ENABLE_YOHARA_SYSTEM
		for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; s++)
		{
			str_to_number(mail.AddData.aApplyRandom[s].wType, data[col++]);
			str_to_number(mail.AddData.aApplyRandom[s].sValue, data[col++]);
			str_to_number(mail.AddData.aApplyRandom[s].row, data[col++]);
		}

		for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; r++)
			str_to_number(mail.AddData.alRandomValues[r], data[col++]);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		str_to_number(mail.AddData.dwTransmutationVnum, data[col++]);
#endif

#ifdef ENABLE_REFINE_ELEMENT
		str_to_number(mail.AddData.grade_element, data[col++]);
		for (int e1 = 0; e1 < MAX_ELEMENTS_SPELL; e1++)
		{
			str_to_number(mail.AddData.attack_element[e1], data[col++]);
		}
		str_to_number(mail.AddData.element_type_bonus, data[col++]);
		for (int e2 = 0; e2 < MAX_ELEMENTS_SPELL; e2++)
		{
			str_to_number(mail.AddData.elements_value_bonus[e2], data[col++]);
		}
#endif

#ifdef ENABLE_SET_ITEM
		str_to_number(mail.AddData.set_value, data[col++]);
#endif

		m_map_mailbox[name].emplace_back(mail);
	}
;
	return true;
}
#endif

bool CClientManager::InitializeQuestItemTable()
{
	using namespace std;

	static const char* s_szQuery = "SELECT vnum, name, %s FROM quest_item_proto ORDER BY vnum";

	char query[1024];
	snprintf(query, sizeof(query), s_szQuery, g_stLocaleNameColumn.c_str());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("query error or no rows: %s", query);
		return false;
	}

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		int col = 0;

		TItemTable tbl;
		memset(&tbl, 0, sizeof(tbl));

		str_to_number(tbl.dwVnum, row[col++]);

		if (row[col])
			strlcpy(tbl.szName, row[col], sizeof(tbl.szName));

		col++;

		if (row[col])
			strlcpy(tbl.szLocaleName, row[col], sizeof(tbl.szLocaleName));

		col++;

		if (m_map_itemTableByVnum.find(tbl.dwVnum) != m_map_itemTableByVnum.end())
		{
			sys_err("QUEST_ITEM_ERROR! %lu vnum already exist! (name %s)", tbl.dwVnum, tbl.szLocaleName);
			continue;
		}

		tbl.bType = ITEM_QUEST; // Everything in the quest_item_proto table is of type ITEM_QUEST
		tbl.bSize = 1;

		m_vec_itemTable.emplace_back(tbl);
	}

	return true;
}

bool CClientManager::InitializeItemTable()
{
	/*=================== Function Description ===================
	1. Summary: read files 'item_proto.txt', 'item_proto_test.txt', 'item_names.txt',
		Create <item_table>(TItemTable), <m_map_itemTableByVnum> objects.
	2. order
		1) Read the 'item_names.txt' file and create a map (a)[localMap](vnum:name).
		2) 'item_proto_text.txt' file and (a)[localMap] map
		(b)[test_map_itemTableByVnum](vnum:TItemTable) Create a map.
		3) 'item_proto.txt' file and (a)[localMap] map
		Create (!)[item_table], <m_map_itemTableByVnum>.
		<Note>
		in each row,
		Rows in both (b)[test_map_itemTableByVnum] and (!)[mob_table] are
		(b) Use the one in [test_map_itemTableByVnum].
		4) Among the rows of (b)[test_map_itemTableByVnum], add the one that is not in (!)[item_table].
	3. test
		1) Check if the 'item_proto.txt' information is properly entered in item_table. -> Done
		2) Check if the 'item_names.txt' information is properly entered in item_table.
		3) Check whether the [overlapping] information from 'item_proto_test.txt' is properly entered into item_table.
		4) Check if the [new] information from 'item_proto_test.txt' is entered into item_table.
		5) Make sure it works properly in the (final) game client.
	_______________________________________________*/

	//=================================================================================//
	// 1) Read 'item_names.txt' file and make (a)[localMap](vnum:name) map.
	//=================================================================================//
	map<int, const char*> localMap;
	cCsvTable nameData;
	if (!nameData.Load("item_names.txt", '\t'))
	{
		fprintf(stderr, "Could not load item_names.txt.\n");
	}
	else
	{
		nameData.Next();
		while (nameData.Next())
		{
			localMap[atoi(nameData.AsStringByIndex(0))] = nameData.AsStringByIndex(1);
		}
	}
	//_________________________________________________________________//

	// read the file.
	cCsvTable data;
	if (!data.Load("item_proto.txt", '\t'))
	{
		fprintf(stderr, "Could not load item_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next(); // Except for the top line (the part that describes the item column)

	if (!m_vec_itemTable.empty())
	{
		sys_log(0, "RELOAD: item_proto");
		m_vec_itemTable.clear();
		m_map_itemTableByVnum.clear();
	}

	//Move data back to the first line (read again ;;)
	data.Destroy();
	if (!data.Load("item_proto.txt", '\t'))
	{
		fprintf(stderr, "Could not load item_proto.txt. Wrong file format?\n");
		return false;
	}
	data.Next(); // Except for the top line (the part that describes the item column)

	m_vec_itemTable.resize(data.m_File.GetRowCount() - 1);
	memset(&m_vec_itemTable[0], 0, sizeof(TItemTable) * m_vec_itemTable.size());

	TItemTable* item_table = &m_vec_itemTable[0];

	while (data.Next())
	{
		if (!Set_Proto_Item_Table(item_table, data, localMap))
		{
			fprintf(stderr, "Failed to load item_proto table.\n");
		}

		m_map_itemTableByVnum.insert(std::map<uint32_t, TItemTable*>::value_type(item_table->dwVnum, item_table));
		++item_table;
	}
	//_______________________________________________________________________//

	// QUEST_ITEM_PROTO_DISABLE
	// InitializeQuestItemTable();
	// END_OF_QUEST_ITEM_PROTO_DISABLE

	m_map_itemTableByVnum.clear();

	auto it = m_vec_itemTable.begin();

	while (it != m_vec_itemTable.end())
	{
		TItemTable* item_table2 = &(*(it++));

		sys_log(1, "ITEM: #%-5lu %-24s %-24s VAL: %ld %ld %ld %ld %ld %ld WEAR %lu ANTI %lu IMMUNE %lu REFINE %lu REFINE_SET %u MAGIC_PCT %u",
			item_table->dwVnum,
			item_table->szName,
			item_table->szLocaleName,
			item_table->alValues[0],
			item_table->alValues[1],
			item_table->alValues[2],
			item_table->alValues[3],
			item_table->alValues[4],
			item_table->alValues[5],
			item_table->dwWearFlags,
			item_table->dwAntiFlags,
			item_table2->dwImmuneFlag,
			item_table2->dwRefinedVnum,
			item_table2->wRefineSet,
			item_table2->bAlterToMagicItemPct);

		m_map_itemTableByVnum.insert(std::map<uint32_t, TItemTable*>::value_type(item_table2->dwVnum, item_table2));
	}
	sort(m_vec_itemTable.begin(), m_vec_itemTable.end(), FCompareVnum());
	return true;
}

bool CClientManager::InitializeSkillTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT dwVnum, szName, bType, bMaxLevel, dwSplashRange, "
		"szPointOn, szPointPoly, szSPCostPoly, szDurationPoly, szDurationSPCostPoly, "
		"szCooldownPoly, szMasterBonusPoly, setFlag+0, setAffectFlag+0, "
		"szPointOn2, szPointPoly2, szDurationPoly2, setAffectFlag2+0, "
		"szPointOn3, szPointPoly3, szDurationPoly3, "
		"szPointOn4, szPointPoly4, szDurationPoly4, "
		"szGrandMasterAddSPCostPoly, "
		"bLevelStep, bLevelLimit, prerequisiteSkillVnum, prerequisiteSkillLevel, iMaxHit, szSplashAroundDamageAdjustPoly, eSkillType+0, dwTargetRange "
		"FROM skill_proto%s ORDER BY dwVnum",
		GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from skill_proto");
		return false;
	}

	if (!m_vec_skillTable.empty())
	{
		sys_log(0, "RELOAD: skill_proto");
		m_vec_skillTable.clear();
	}

	m_vec_skillTable.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;
	int col;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TSkillTable t;
		memset(&t, 0, sizeof(t));

		col = 0;

		str_to_number(t.dwVnum, data[col++]);
		strlcpy(t.szName, data[col++], sizeof(t.szName));
		str_to_number(t.bType, data[col++]);
		str_to_number(t.bMaxLevel, data[col++]);
		str_to_number(t.dwSplashRange, data[col++]);

		strlcpy(t.szPointOn, data[col++], sizeof(t.szPointOn));
		strlcpy(t.szPointPoly, data[col++], sizeof(t.szPointPoly));
		strlcpy(t.szSPCostPoly, data[col++], sizeof(t.szSPCostPoly));
		strlcpy(t.szDurationPoly, data[col++], sizeof(t.szDurationPoly));
		strlcpy(t.szDurationSPCostPoly, data[col++], sizeof(t.szDurationSPCostPoly));
		strlcpy(t.szCooldownPoly, data[col++], sizeof(t.szCooldownPoly));
		strlcpy(t.szMasterBonusPoly, data[col++], sizeof(t.szMasterBonusPoly));

		str_to_number(t.dwFlag, data[col++]);
		str_to_number(t.dwAffectFlag, data[col++]);

		strlcpy(t.szPointOn2, data[col++], sizeof(t.szPointOn2));
		strlcpy(t.szPointPoly2, data[col++], sizeof(t.szPointPoly2));
		strlcpy(t.szDurationPoly2, data[col++], sizeof(t.szDurationPoly2));
		str_to_number(t.dwAffectFlag2, data[col++]);

		// ADD_GRANDMASTER_SKILL
		strlcpy(t.szPointOn3, data[col++], sizeof(t.szPointOn3));
		strlcpy(t.szPointPoly3, data[col++], sizeof(t.szPointPoly3));
		strlcpy(t.szDurationPoly3, data[col++], sizeof(t.szDurationPoly3));

#ifdef ENABLE_SKILL_TABLE_RENEWAL
		strlcpy(t.szPointOn4, data[col++], sizeof(t.szPointOn4));
		strlcpy(t.szPointPoly4, data[col++], sizeof(t.szPointPoly4));
		strlcpy(t.szDurationPoly4, data[col++], sizeof(t.szDurationPoly4));
#endif

		strlcpy(t.szGrandMasterAddSPCostPoly, data[col++], sizeof(t.szGrandMasterAddSPCostPoly));
		// END_OF_ADD_GRANDMASTER_SKILL

		str_to_number(t.bLevelStep, data[col++]);
		str_to_number(t.bLevelLimit, data[col++]);
		str_to_number(t.preSkillVnum, data[col++]);
		str_to_number(t.preSkillLevel, data[col++]);

		str_to_number(t.lMaxHit, data[col++]);

		strlcpy(t.szSplashAroundDamageAdjustPoly, data[col++], sizeof(t.szSplashAroundDamageAdjustPoly));

		str_to_number(t.bSkillAttrType, data[col++]);
		str_to_number(t.dwTargetRange, data[col++]);

		sys_log(0, "SKILL: #%d %s flag %u point %s affect %u cooldown %s", t.dwVnum, t.szName, t.dwFlag, t.szPointOn, t.dwAffectFlag, t.szCooldownPoly);

		m_vec_skillTable.emplace_back(t);
	}

	return true;
}

bool CClientManager::InitializeBanwordTable()
{
	m_vec_banwordTable.clear();

	auto pkMsg(CDBManager::Instance().DirectQuery("SELECT word FROM banword"));

	SQLResult* pRes = pkMsg->Get();

	if (pRes->uiNumRows == 0)
		return true;

	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TBanwordTable t{};

		if (data[0])
		{
			strlcpy(t.szWord, data[0], sizeof(t.szWord));
			m_vec_banwordTable.emplace_back(t);
		}
	}

	sys_log(0, "BANWORD: total %d", m_vec_banwordTable.size());
	return true;
}

bool CClientManager::InitializeItemAttrTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT apply, apply+0, prob, "
		"lv1, lv2, lv3, lv4, lv5, "
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
		"lv6, lv7, lv8, lv9, lv10, "
#endif
		"weapon, body, wrist, foots, neck, head, shield, ear"
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
		", pendant"
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
		", glove"
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
		", costume_body, costume_hair"
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		", costume_weapon"
#	endif
#endif
		" FROM item_attr%s ORDER BY apply",
		GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from item_attr");
		return false;
	}

	if (!m_vec_itemAttrTable.empty())
	{
		sys_log(0, "RELOAD: item_attr");
		m_vec_itemAttrTable.clear();
	}

	m_vec_itemAttrTable.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TItemAttrTable t{};

		int col = 0;

		strlcpy(t.szApply, data[col++], sizeof(t.szApply));
		str_to_number(t.dwApplyIndex, data[col++]);
		str_to_number(t.dwProb, data[col++]);
		str_to_number(t.lValues[0], data[col++]);
		str_to_number(t.lValues[1], data[col++]);
		str_to_number(t.lValues[2], data[col++]);
		str_to_number(t.lValues[3], data[col++]);
		str_to_number(t.lValues[4], data[col++]);
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
		str_to_number(t.lValues[5], data[col++]);
		str_to_number(t.lValues[6], data[col++]);
		str_to_number(t.lValues[7], data[col++]);
		str_to_number(t.lValues[8], data[col++]);
		str_to_number(t.lValues[9], data[col++]);
#endif
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_NECK], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_EAR], data[col++]);
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT], data[col++]);
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_GLOVE], data[col++]);
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR], data[col++]);
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON], data[col++]);
#	endif
#endif

		sys_log(0, "ITEM_ATTR: %-20s %4lu { %3d %3d %3d %3d %3d } { %d %d %d %d %d %d %d"
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
			" %d"
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
			" %d"
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
			" %d %d"
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			" %d"
#	endif
#endif
			" }",
			t.szApply,
			t.dwProb,
			t.lValues[0],
			t.lValues[1],
			t.lValues[2],
			t.lValues[3],
			t.lValues[4],
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
			t.lValues[5],
			t.lValues[6],
			t.lValues[7],
			t.lValues[8],
			t.lValues[9],
#endif
			t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON],
			t.bMaxLevelBySet[ATTRIBUTE_SET_BODY],
			t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST],
			t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS],
			t.bMaxLevelBySet[ATTRIBUTE_SET_NECK],
			t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_EAR]
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
			, t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT]
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
			, t.bMaxLevelBySet[ATTRIBUTE_SET_GLOVE]
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY]
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR]
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON]
#	endif
#endif
		);

		m_vec_itemAttrTable.emplace_back(t);
	}

	return true;
}

bool CClientManager::InitializeItemRareTable()
{
	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT apply, apply+0, prob, "
		"lv1, lv2, lv3, lv4, lv5, "
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
		"lv6, lv7, lv8, lv9, lv10, "
#endif
		"weapon, body, wrist, foots, neck, head, shield, ear"
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
		", pendant"
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
		", glove"
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
		", costume_body, costume_hair"
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		", costume_weapon"
#	endif
#endif
		" FROM item_attr_rare%s ORDER BY apply",
		GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!pRes->uiNumRows)
	{
		sys_err("no result from item_attr_rare");
		return false;
	}

	if (!m_vec_itemRareTable.empty())
	{
		sys_log(0, "RELOAD: item_attr_rare");
		m_vec_itemRareTable.clear();
	}

	m_vec_itemRareTable.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;

	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		TItemAttrTable t{}; // reduce memset

		int col = 0;

		strlcpy(t.szApply, data[col++], sizeof(t.szApply));
		str_to_number(t.dwApplyIndex, data[col++]);
		str_to_number(t.dwProb, data[col++]);
		str_to_number(t.lValues[0], data[col++]);
		str_to_number(t.lValues[1], data[col++]);
		str_to_number(t.lValues[2], data[col++]);
		str_to_number(t.lValues[3], data[col++]);
		str_to_number(t.lValues[4], data[col++]);
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
		str_to_number(t.lValues[5], data[col++]);
		str_to_number(t.lValues[6], data[col++]);
		str_to_number(t.lValues[7], data[col++]);
		str_to_number(t.lValues[8], data[col++]);
		str_to_number(t.lValues[9], data[col++]);
#endif
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_NECK], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_EAR], data[col++]);
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT], data[col++]);
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_GLOVE], data[col++]);
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY], data[col++]);
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR], data[col++]);
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		str_to_number(t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON], data[col++]);
#	endif
#endif

		sys_log(0, "ITEM_RARE: %-20s %4lu { %3d %3d %3d %3d %3d "
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
			"%3d %3d %3d %3d %3d "
#endif
			"} { %d %d %d %d %d %d %d"
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
			" %d"
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
			" %d"
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
			" %d %d"
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			" %d"
#	endif
#endif
			" }",
			t.szApply,
			t.dwProb,
			t.lValues[0],
			t.lValues[1],
			t.lValues[2],
			t.lValues[3],
			t.lValues[4],
#ifdef ENABLE_ATTR_6TH_7TH_ATTR_TABLE
			t.lValues[5],
			t.lValues[6],
			t.lValues[7],
			t.lValues[8],
			t.lValues[9],
#endif
			t.bMaxLevelBySet[ATTRIBUTE_SET_WEAPON],
			t.bMaxLevelBySet[ATTRIBUTE_SET_BODY],
			t.bMaxLevelBySet[ATTRIBUTE_SET_WRIST],
			t.bMaxLevelBySet[ATTRIBUTE_SET_FOOTS],
			t.bMaxLevelBySet[ATTRIBUTE_SET_NECK],
			t.bMaxLevelBySet[ATTRIBUTE_SET_HEAD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_SHIELD],
			t.bMaxLevelBySet[ATTRIBUTE_SET_EAR]
#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
			, t.bMaxLevelBySet[ATTRIBUTE_SET_PENDANT]
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
			, t.bMaxLevelBySet[ATTRIBUTE_SET_GLOVE]
#endif
#ifdef ENABLE_ITEM_ATTR_COSTUME
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_BODY]
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_HAIR]
#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			, t.bMaxLevelBySet[ATTRIBUTE_SET_COSTUME_WEAPON]
#	endif
#endif
		);

		m_vec_itemRareTable.emplace_back(t);
	}

	return true;
}

bool CClientManager::InitializeLandTable()
{
	using namespace building;

#ifdef ENABLE_GUILDLAND_INACTIVITY_DELETE
	CDBManager::Instance().DirectQuery("UPDATE land "
		"INNER JOIN guild ON land.guild_id = guild.id "
		"INNER JOIN player ON guild.`master` = player.id "
		"SET guild_id=0 "
		"WHERE guild_id > 0 and "
		"DATE_SUB(NOW(), INTERVAL 21 DAY) > last_play; ");

	CDBManager::Instance().DirectQuery("DELETE object "
		"FROM object "
		"INNER JOIN land ON land.id = object.land_id "
		"WHERE land.guild_id=0; ");
#endif

	char query[4096];

	snprintf(query, sizeof(query),
		"SELECT id, map_index, x, y, width, height, guild_id, guild_level_limit, price "
		"FROM land%s WHERE enable='YES' ORDER BY id",
		GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!m_vec_kLandTable.empty())
	{
		sys_log(0, "RELOAD: land");
		m_vec_kLandTable.clear();
	}

	m_vec_kLandTable.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TLand t;

			memset(&t, 0, sizeof(t));

			int col = 0;

			str_to_number(t.dwID, data[col++]);
			str_to_number(t.lMapIndex, data[col++]);
			str_to_number(t.x, data[col++]);
			str_to_number(t.y, data[col++]);
			str_to_number(t.width, data[col++]);
			str_to_number(t.height, data[col++]);
			str_to_number(t.dwGuildID, data[col++]);
			str_to_number(t.bGuildLevelLimit, data[col++]);
			str_to_number(t.dwPrice, data[col++]);

			sys_log(0, "LAND: %lu map %-4ld %7ldx%-7ld w %-4ld h %-4ld", t.dwID, t.lMapIndex, t.x, t.y, t.width, t.height);

			m_vec_kLandTable.emplace_back(t);
		}

	return true;
}

void parse_pair_number_string(const char* c_pszString, std::vector<std::pair<int, int> >& vec)
{
	// format: 10,1/20,3/300,50
	const char* t = c_pszString;
	const char* p = strchr(t, '/');
	std::pair<int, int> k;

	char szNum[32 + 1];
	char* comma;

	while (p)
	{
		if (isnhdigit(*t))
		{
			strlcpy(szNum, t, MIN(sizeof(szNum), (p - t) + 1));

			comma = strchr(szNum, ',');

			if (comma)
			{
				*comma = '\0';
				str_to_number(k.second, comma + 1);
			}
			else
				k.second = 0;

			str_to_number(k.first, szNum);
			vec.emplace_back(k);
		}

		t = p + 1;
		p = strchr(t, '/');
	}

	if (isnhdigit(*t))
	{
		strlcpy(szNum, t, sizeof(szNum));

		comma = strchr(const_cast<char*>(t), ',');

		if (comma)
		{
			*comma = '\0';
			str_to_number(k.second, comma + 1);
		}
		else
			k.second = 0;

		str_to_number(k.first, szNum);
		vec.emplace_back(k);
	}
}

bool CClientManager::InitializeObjectProto()
{
	using namespace building;

	char query[4096];
	snprintf(query, sizeof(query),
		"SELECT vnum, price, materials, upgrade_vnum, upgrade_limit_time, life, reg_1, reg_2, reg_3, reg_4, npc, group_vnum, dependent_group "
		"FROM object_proto%s ORDER BY vnum",
		GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!m_vec_kObjectProto.empty())
	{
		sys_log(0, "RELOAD: object_proto");
		m_vec_kObjectProto.clear();
	}

	m_vec_kObjectProto.reserve(MAX(0, static_cast<int>(pRes->uiNumRows)));

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TObjectProto t;

			memset(&t, 0, sizeof(t));

			int col = 0;

			str_to_number(t.dwVnum, data[col++]);
			str_to_number(t.dwPrice, data[col++]);

			std::vector<std::pair<int, int> > vec;
			parse_pair_number_string(data[col++], vec);

			for (uint32_t i = 0; i < OBJECT_MATERIAL_MAX_NUM && i < vec.size(); ++i)
			{
				std::pair<int, int>& r = vec[i];

				t.kMaterials[i].dwItemVnum = r.first;
				t.kMaterials[i].dwCount = r.second;
			}

			str_to_number(t.dwUpgradeVnum, data[col++]);
			str_to_number(t.dwUpgradeLimitTime, data[col++]);
			str_to_number(t.lLife, data[col++]);
			str_to_number(t.lRegion[0], data[col++]);
			str_to_number(t.lRegion[1], data[col++]);
			str_to_number(t.lRegion[2], data[col++]);
			str_to_number(t.lRegion[3], data[col++]);

			// ADD_BUILDING_NPC
			str_to_number(t.dwNPCVnum, data[col++]);
			str_to_number(t.dwGroupVnum, data[col++]);
			str_to_number(t.dwDependOnGroupVnum, data[col++]);

			t.lNPCX = 0;
			t.lNPCY = MAX(t.lRegion[1], t.lRegion[3]) + 300;
			// END_OF_ADD_BUILDING_NPC

			sys_log(0, "OBJ_PROTO: vnum %lu price %lu mat %lu %lu",
				t.dwVnum, t.dwPrice, t.kMaterials[0].dwItemVnum, t.kMaterials[0].dwCount);

			m_vec_kObjectProto.emplace_back(t);
		}

	return true;
}

bool CClientManager::InitializeObjectTable()
{
	using namespace building;

	char query[4096];
	snprintf(query, sizeof(query), "SELECT id, land_id, vnum, map_index, x, y, x_rot, y_rot, z_rot, life FROM object%s ORDER BY id", GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	if (!m_map_pkObjectTable.empty())
	{
		sys_log(0, "RELOAD: object");
		m_map_pkObjectTable.clear();
	}

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TObject* k = new TObject;

			memset(k, 0, sizeof(TObject));

			int col = 0;

			str_to_number(k->dwID, data[col++]);
			str_to_number(k->dwLandID, data[col++]);
			str_to_number(k->dwVnum, data[col++]);
			str_to_number(k->lMapIndex, data[col++]);
			str_to_number(k->x, data[col++]);
			str_to_number(k->y, data[col++]);
			str_to_number(k->xRot, data[col++]);
			str_to_number(k->yRot, data[col++]);
			str_to_number(k->zRot, data[col++]);
			str_to_number(k->lLife, data[col++]);

			sys_log(0, "OBJ: %lu vnum %lu map %-4ld %7ldx%-7ld life %ld",
				k->dwID, k->dwVnum, k->lMapIndex, k->x, k->y, k->lLife);

			m_map_pkObjectTable.insert(std::make_pair(k->dwID, k));
		}

	return true;
}

bool CClientManager::InitializeMonarch()
{
	CMonarch::Instance().LoadMonarch();

	return true;
}

bool CClientManager::MirrorMobTableIntoDB()
{
	for (auto it = m_vec_mobTable.begin(); it != m_vec_mobTable.end(); it++)
	{
		const TMobTable& t = *it;
		char query[4096];
		if (g_stLocaleNameColumn == "name")
		{
			snprintf(query, sizeof(query),
				"REPLACE INTO mob_proto%s "
				"("
				"vnum, "
				"name, "
				"type, "
				"`rank`, "
				"battle_type, "
				"level, "
				"size, "
#ifdef ENABLE_MOB_SCALE
				"scale, "
#endif
				"ai_flag, "
#ifdef ENABLE_PROTO_RENEWAL
				"ai_flag_ex, "
#endif
				"setRaceFlag, "
				"setImmuneFlag, "
				"on_click, "
				"empire, "
				"drop_item, "
				"resurrection_vnum, "
				"folder, "
				"st, "
				"dx, "
				"ht, "
				"iq, "
#ifdef ENABLE_YOHARA_SYSTEM
				"sungma_st, "
				"sungma_dx, "
				"sungma_ht, "
				"sungma_iq, "
#endif
				"damage_min, "
				"damage_max, "
				"max_hp, "
				"regen_cycle, "
				"regen_percent, "
				"exp, "
#ifdef ENABLE_YOHARA_SYSTEM
				"sungma_exp, "
#endif
				"gold_min, "
				"gold_max, "
				"def, "
				"attack_speed, "
				"move_speed, "
				"aggressive_hp_pct, "
				"aggressive_sight, "
				"attack_range, "
				"polymorph_item, "
				"enchant_curse, "
				"enchant_slow, "
				"enchant_poison, "
				"enchant_stun, "
				"enchant_critical, "
				"enchant_penetrate, "
				"resist_fist, "
				"resist_sword, "
				"resist_twohand, "
				"resist_dagger, "
				"resist_bell, "
				"resist_fan, "
				"resist_bow, "
#ifdef ENABLE_WOLFMAN_CHARACTER
				"resist_claw, "
#endif
				"resist_fire, "
				"resist_elect, "
				"resist_magic, "
				"resist_wind, "
				"resist_poison, "
#ifdef ENABLE_WOLFMAN_CHARACTER
				"resist_bleeding, "
#endif
#ifdef ENABLE_ELEMENT_ADD
				"att_elec, "
				"att_fire, "
				"att_ice, "
				"att_wind, "
				"att_earth, "
				"att_dark, "
#endif
				"resist_dark, "
				"resist_ice, "
				"resist_earth, "
				"dam_multiply, "
				"summon, "
				"drain_sp, "
				"skill_level0, "
				"skill_vnum0, "
				"skill_level1, "
				"skill_vnum1, "
				"skill_level2, "
				"skill_vnum2, "
				"skill_level3, "
				"skill_vnum3, "
				"skill_level4, "
				"skill_vnum4, "
				"sp_berserk, "
				"sp_stoneskin, "
				"sp_godspeed, "
				"sp_deathblow, "
				"sp_revive, "
				"heal_point, "
				"att_speed_point, "
				"cast_speed_point, "
				"hp_regen_point, "
				"hit_range"
				") "
				"VALUES ("

				"%u, "		// vnum
				"\"%s\", "	// name
				"%u, "		// type
				"%u, "		// rank
				"%u, "		// battle_type
				"%u, "		// level
#ifdef ENABLE_MOB_SCALE
				"%u, "		// scale
#endif
				"%u, "		// size
				"%u, "		// ai_flag
#ifdef ENABLE_PROTO_RENEWAL
				"%u, "		// ai_flag_ex
#endif
				"%u, "		// setRaceFlag
				"%u, "		// setImmuneFlag
				"%u, "		// on_click
				"%u, "		// empire
				"%u, "		// drop_item
				"%u, "		// resurrection_vnum
				"'%s', "	// folder
				"%u, "		// st
				"%u, "		// dx
				"%u, "		// ht
				"%u, "		// iq
#ifdef ENABLE_YOHARA_SYSTEM
				"%u, "		// sungma_st
				"%u, "		// sungma_dx
				"%u, "		// sungma_ht
				"%u, "		// sungma_iq
#endif
				"%u, "		// damage_min
				"%u, "		// damage_max
				"%u, "		// max_hp
				"%u, "		// regen_cycle
				"%u, "		// regen_percent
				"%u, "		// exp
#ifdef ENABLE_YOHARA_SYSTEM
				"%u, "		// sungma_exp
#endif
				"%u, "		// gold_min
				"%u, "		// gold_max
				"%d, "		// def
				"%d, "		// attack_speed
				"%d, "		// move_speed
				"%u, "		// aggressive_hp_pct
				"%d, "		// aggressive_sight
				"%d, "		// attack_range
				"%u, "		// polymorph_item
				"%d, "		// enchant_curse
				"%d, "		// enchant_slow
				"%d, "		// enchant_poison
				"%d, "		// enchant_stun
				"%d, "		// enchant_critical
				"%d, "		// enchant_penetrate
				"%d, "		// resist_fist
				"%d, "		// resist_sword
				"%d, "		// resist_twohand
				"%d, "		// resist_dagger
				"%d, "		// resist_bell
				"%d, "		// resist_fan
				"%d, "		// resist_bow
#ifdef ENABLE_WOLFMAN_CHARACTER
				"%d, "		// resist_claw
#endif
				"%d, "		// resist_fire
				"%d, "		// resist_elect
				"%d, "		// resist_magic
				"%d, "		// resist_wind
				"%d, "		// resist_poison
#ifdef ENABLE_WOLFMAN_CHARACTER
				"%d, "		// resist_bleeding
#endif
#ifdef ENABLE_ELEMENT_ADD
				"%d, "		// att_elec
				"%d, "		// att_fire
				"%d, "		// att_ice
				"%d, "		// att_wind
				"%d, "		// att_earth
				"%d, "		// att_dark
#endif
				"%d, "		// resist_dark
				"%d, "		// resist_ice
				"%d, "		// resist_earth
				"%f, "		// dam_multiply
				"%u, "		// summon
				"%u, "		// drain_sp
				"%u, "		// skill_level0
				"%u, "		// skill_vnum0
				"%u, "		// skill_level1
				"%u, "		// skill_vnum1
				"%u, "		// skill_level2
				"%u, "		// skill_vnum2
				"%u, "		// skill_level3
				"%u, "		// skill_vnum3
				"%u, "		// skill_level4
				"%u, "		// skill_vnum4
				"%u, "		// sp_berserk
				"%u, "		// sp_stoneskin
				"%u, "		// sp_godspeed
				"%u, "		// sp_deathblow
				"%u, "		// sp_revive
				"%u, "		// heal_point
				"%u, "		// att_speed_point
				"%u, "		// cast_speed_point
				"%u, "		// hp_regen_point
				"%f"		// hit_range
				")",
				GetTablePostfix(), /*g_stLocaleNameColumn.c_str(),*/

				t.dwVnum,
				t.szName,
				/*t.szLocaleName, */
				t.bType,
				t.bRank,
				t.bBattleType,
				t.bLevel,
#ifdef ENABLE_MOB_SCALE
				t.bScale,
#endif
				t.bSize,
				t.dwAIFlag,
#ifdef ENABLE_PROTO_RENEWAL
				t.dwAIFlagEx,
#endif
				t.dwRaceFlag,
				t.dwImmuneFlag,
				t.bOnClickType,
				t.bEmpire,
				t.dwDropItemVnum,
				t.dwResurrectionVnum,
				t.szFolder,
				t.bStr,
				t.bDex,
				t.bCon,
				t.bInt,
#ifdef ENABLE_YOHARA_SYSTEM
				t.bSungMaSt,
				t.bSungMaDx,
				t.bSungMaHt,
				t.bSungMaIq,
#endif
				t.dwDamageRange[0],
				t.dwDamageRange[1],
				t.dwMaxHP,
				t.bRegenCycle,
				t.bRegenPercent,
				t.dwExp,
#ifdef ENABLE_YOHARA_SYSTEM
				t.dwSungMaExp,
#endif
				t.dwGoldMin,
				t.dwGoldMax,
				t.wDef,
				t.sAttackSpeed,
				t.sMovingSpeed,
				t.bAggresiveHPPct,
				t.wAggressiveSight,
				t.wAttackRange,
				t.dwPolymorphItemVnum,
				t.cEnchants[MOB_ENCHANT_CURSE],
				t.cEnchants[MOB_ENCHANT_SLOW],
				t.cEnchants[MOB_ENCHANT_POISON],
				t.cEnchants[MOB_ENCHANT_STUN],
				t.cEnchants[MOB_ENCHANT_CRITICAL],
				t.cEnchants[MOB_ENCHANT_PENETRATE],
				t.cResists[MOB_RESIST_FIST],
				t.cResists[MOB_RESIST_SWORD],
				t.cResists[MOB_RESIST_TWOHAND],
				t.cResists[MOB_RESIST_DAGGER],
				t.cResists[MOB_RESIST_BELL],
				t.cResists[MOB_RESIST_FAN],
				t.cResists[MOB_RESIST_BOW],
#ifdef ENABLE_WOLFMAN_CHARACTER
				t.cResists[MOB_RESIST_CLAW],
#endif
				t.cResists[MOB_RESIST_FIRE],
				t.cResists[MOB_RESIST_ELECT],
				t.cResists[MOB_RESIST_MAGIC],
				t.cResists[MOB_RESIST_WIND],
				t.cResists[MOB_RESIST_POISON],
#ifdef ENABLE_WOLFMAN_CHARACTER
				t.cResists[MOB_RESIST_BLEEDING],
#endif
#ifdef ENABLE_ELEMENT_ADD
				t.cElementalFlags[MOB_ELEMENTAL_ELEC],	// AttElec
				t.cElementalFlags[MOB_ELEMENTAL_FIRE],	// AttFire
				t.cElementalFlags[MOB_ELEMENTAL_ICE],	// AttIce
				t.cElementalFlags[MOB_ELEMENTAL_WIND],	// AttWind
				t.cElementalFlags[MOB_ELEMENTAL_EARTH],	// AttEarth
				t.cElementalFlags[MOB_ELEMENTAL_DARK],	// AttDark
#endif
				t.cResistDark,
				t.cResistIce,
				t.cResistEarth,
				t.fDamMultiply,
				t.dwSummonVnum,
				t.dwDrainSP,
				t.Skills[0].bLevel,
				t.Skills[0].dwVnum,
				t.Skills[1].bLevel,
				t.Skills[1].dwVnum,
				t.Skills[2].bLevel,
				t.Skills[2].dwVnum,
				t.Skills[3].bLevel,
				t.Skills[3].dwVnum,
				t.Skills[4].bLevel,
				t.Skills[4].dwVnum,
				t.bBerserkPoint,
				t.bStoneSkinPoint,
				t.bGodSpeedPoint,
				t.bDeathBlowPoint,
				t.bRevivePoint,
				t.bHealPoint,
				t.bRAttSpeedPoint,
				t.bRCastSpeedPoint,
				t.bRHPRegenPoint,
				t.fHitRange
			);
		}
		else
		{
			snprintf(query, sizeof(query),
				"REPLACE INTO mob_proto%s "
				"("
				"vnum, "
				"name, "
				"%s, "
				"type, "
				"`rank`, "
				"battle_type, "
				"level, "
#ifdef ENABLE_MOB_SCALE
				"scale, "
#endif
				"size, "
				"ai_flag, "
#ifdef ENABLE_PROTO_RENEWAL
				"ai_flag_ex, "
#endif
				"setRaceFlag, "
				"setImmuneFlag, "
				"on_click, "
				"empire, "
				"drop_item, "
				"resurrection_vnum, "
				"folder, "
				"st, "
				"dx, "
				"ht, "
				"iq, "
#ifdef ENABLE_YOHARA_SYSTEM
				"sungma_st, "
				"sungma_dx, "
				"sungma_ht, "
				"sungma_iq, "
#endif
				"damage_min, "
				"damage_max, "
				"max_hp, "
				"regen_cycle, "
				"regen_percent, "
				"exp, "
#ifdef ENABLE_YOHARA_SYSTEM
				"sungma_exp, "
#endif
				"gold_min, "
				"gold_max, "
				"def, "
				"attack_speed, "
				"move_speed, "
				"aggressive_hp_pct, "
				"aggressive_sight, "
				"attack_range, "
				"polymorph_item, "
				"enchant_curse, "
				"enchant_slow, "
				"enchant_poison, "
				"enchant_stun, "
				"enchant_critical, "
				"enchant_penetrate, "
				"resist_fist, "
				"resist_sword, "
				"resist_twohand, "
				"resist_dagger, "
				"resist_bell, "
				"resist_fan, "
				"resist_bow, "
#ifdef ENABLE_WOLFMAN_CHARACTER
				"resist_claw, "
#endif
				"resist_fire, "
				"resist_elect, "
				"resist_magic, "
				"resist_wind, "
				"resist_poison, "
#ifdef ENABLE_WOLFMAN_CHARACTER
				"resist_bleeding, "
#endif
#ifdef ENABLE_ELEMENT_ADD
				"att_elec, "
				"att_fire, "
				"att_ice, "
				"att_wind, "
				"att_earth, "
				"att_dark, "
#endif
				"resist_dark, "
				"resist_ice, "
				"resist_earth, "
				"dam_multiply, "
				"summon, "
				"drain_sp, "
				"skill_level0, "
				"skill_vnum0, "
				"skill_level1, "
				"skill_vnum1, "
				"skill_level2, "
				"skill_vnum2, "
				"skill_level3, "
				"skill_vnum3, "
				"skill_level4, "
				"skill_vnum4, "
				"sp_berserk, "
				"sp_stoneskin, "
				"sp_godspeed, "
				"sp_deathblow, "
				"sp_revive, "
				"heal_point, "
				"att_speed_point, "
				"cast_speed_point, "
				"hp_regen_point, "
				"hit_range"
				") "
				"VALUES ("

				"%u, "		// vnum
				"\"%s\", "	// name
				"\"%s\", "	// %s
				"%u, "		// type
				"%u, "		// rank
				"%u, "		// battle_type
				"%u, "		// level
#ifdef ENABLE_MOB_SCALE
				"%u, "		// scale
#endif
				"%u, "		// size
				"%u, "		// ai_flag
#ifdef ENABLE_PROTO_RENEWAL
				"%u, "		// ai_flag_ex
#endif
				"%u, "		// setRaceFlag
				"%u, "		// setImmuneFlag
				"%u, "		// on_click
				"%u, "		// empire
				"%u, "		// drop_item
				"%u, "		// resurrection_vnum
				"'%s', "	// folder
				"%u, "		// st
				"%u, "		// dx
				"%u, "		// ht
				"%u, "		// iq
#ifdef ENABLE_YOHARA_SYSTEM
				"%u, "		// sungma_st
				"%u, "		// sungma_dx
				"%u, "		// sungma_ht
				"%u, "		// sungma_iq
#endif
				"%u, "		// damage_min
				"%u, "		// damage_max
				"%u, "		// max_hp
				"%u, "		// regen_cycle
				"%u, "		// regen_percent
				"%u, "		// exp
#ifdef ENABLE_YOHARA_SYSTEM
				"%u, "		// sungma_exp
#endif
				"%u, "		// gold_min
				"%u, "		// gold_max
				"%d, "		// def
				"%d, "		// attack_speed
				"%d, "		// move_speed
				"%u, "		// aggressive_hp_pct
				"%d, "		// aggressive_sight
				"%d, "		// attack_range
				"%u, "		// polymorph_item
				"%d, "		// enchant_curse
				"%d, "		// enchant_slow
				"%d, "		// enchant_poison
				"%d, "		// enchant_stun
				"%d, "		// enchant_critical
				"%d, "		// enchant_penetrate
				"%d, "		// resist_fist
				"%d, "		// resist_sword
				"%d, "		// resist_twohand
				"%d, "		// resist_dagger
				"%d, "		// resist_bell
				"%d, "		// resist_fan
				"%d, "		// resist_bow
#ifdef ENABLE_WOLFMAN_CHARACTER
				"%d, "		// resist_claw
#endif
				"%d, "		// resist_fire
				"%d, "		// resist_elect
				"%d, "		// resist_magic
				"%d, "		// resist_wind
				"%d, "		// resist_poison
#ifdef ENABLE_WOLFMAN_CHARACTER
				"%d, "		// resist_bleeding
#endif
#ifdef ENABLE_ELEMENT_ADD
				"%d, "		// att_elec
				"%d, "		// att_fire
				"%d, "		// att_ice
				"%d, "		// att_wind
				"%d, "		// att_earth
				"%d, "		// att_dark
#endif
				"%d, "		// resist_dark
				"%d, "		// resist_ice
				"%d, "		// resist_earth
				"%f, "		// dam_multiply
				"%u, "		// summon
				"%u, "		// drain_sp
				"%u, "		// skill_level0
				"%u, "		// skill_vnum0
				"%u, "		// skill_level1
				"%u, "		// skill_vnum1
				"%u, "		// skill_level2
				"%u, "		// skill_vnum2
				"%u, "		// skill_level3
				"%u, "		// skill_vnum3
				"%u, "		// skill_level4
				"%u, "		// skill_vnum4
				"%u, "		// sp_berserk
				"%u, "		// sp_stoneskin
				"%u, "		// sp_godspeed
				"%u, "		// sp_deathblow
				"%u, "		// sp_revive
				"%u, "		// heal_point
				"%u, "		// att_speed_point
				"%u, "		// cast_speed_point
				"%u, "		// hp_regen_point
				"%f"		// hit_range
				")",
				GetTablePostfix(),
				g_stLocaleNameColumn.c_str(),
				t.dwVnum,
				t.szName,
				t.szLocaleName,
				t.bType,
				t.bRank,
				t.bBattleType,
				t.bLevel,
#ifdef ENABLE_MOB_SCALE
				t.bScale,
#endif
				t.bSize,
				t.dwAIFlag,
#ifdef ENABLE_PROTO_RENEWAL
				t.dwAIFlagEx,
#endif
				t.dwRaceFlag,
				t.dwImmuneFlag,
				t.bOnClickType,
				t.bEmpire,
				t.dwDropItemVnum,
				t.dwResurrectionVnum,
				t.szFolder,
				t.bStr,
				t.bDex,
				t.bCon,
				t.bInt,
#ifdef ENABLE_YOHARA_SYSTEM
				t.bSungMaSt,
				t.bSungMaDx,
				t.bSungMaHt,
				t.bSungMaIq,
#endif
				t.dwDamageRange[0],
				t.dwDamageRange[1],
				t.dwMaxHP,
				t.bRegenCycle,
				t.bRegenPercent,
				t.dwExp,
#ifdef ENABLE_YOHARA_SYSTEM
				t.dwSungMaExp,
#endif
				t.dwGoldMin,
				t.dwGoldMax,
				t.wDef,
				t.sAttackSpeed,
				t.sMovingSpeed,
				t.bAggresiveHPPct,
				t.wAggressiveSight,
				t.wAttackRange,
				t.dwPolymorphItemVnum,
				t.cEnchants[MOB_ENCHANT_CURSE],
				t.cEnchants[MOB_ENCHANT_SLOW],
				t.cEnchants[MOB_ENCHANT_POISON],
				t.cEnchants[MOB_ENCHANT_STUN],
				t.cEnchants[MOB_ENCHANT_CRITICAL],
				t.cEnchants[MOB_ENCHANT_PENETRATE],
				t.cResists[MOB_RESIST_FIST],
				t.cResists[MOB_RESIST_SWORD],
				t.cResists[MOB_RESIST_TWOHAND],
				t.cResists[MOB_RESIST_DAGGER],
				t.cResists[MOB_RESIST_BELL],
				t.cResists[MOB_RESIST_FAN],
				t.cResists[MOB_RESIST_BOW],
#ifdef ENABLE_WOLFMAN_CHARACTER
				t.cResists[MOB_RESIST_CLAW],
#endif
				t.cResists[MOB_RESIST_FIRE],
				t.cResists[MOB_RESIST_ELECT],
				t.cResists[MOB_RESIST_MAGIC],
				t.cResists[MOB_RESIST_WIND],
				t.cResists[MOB_RESIST_POISON],
#ifdef ENABLE_WOLFMAN_CHARACTER
				t.cResists[MOB_RESIST_BLEEDING],
#endif
#ifdef ENABLE_ELEMENT_ADD
				t.cElementalFlags[MOB_ELEMENTAL_ELEC],	// AttElec
				t.cElementalFlags[MOB_ELEMENTAL_FIRE],	// AttFire
				t.cElementalFlags[MOB_ELEMENTAL_ICE],	// AttIce
				t.cElementalFlags[MOB_ELEMENTAL_WIND],	// AttWind
				t.cElementalFlags[MOB_ELEMENTAL_EARTH],	// AttEarth
				t.cElementalFlags[MOB_ELEMENTAL_DARK],	// AttDark
#endif
				t.cResistDark,
				t.cResistIce,
				t.cResistEarth,
				t.fDamMultiply,
				t.dwSummonVnum,
				t.dwDrainSP,
				t.Skills[0].bLevel,
				t.Skills[0].dwVnum,
				t.Skills[1].bLevel,
				t.Skills[1].dwVnum,
				t.Skills[2].bLevel,
				t.Skills[2].dwVnum,
				t.Skills[3].bLevel,
				t.Skills[3].dwVnum,
				t.Skills[4].bLevel,
				t.Skills[4].dwVnum,
				t.bBerserkPoint,
				t.bStoneSkinPoint,
				t.bGodSpeedPoint,
				t.bDeathBlowPoint,
				t.bRevivePoint,
				t.bHealPoint,
				t.bRAttSpeedPoint,
				t.bRCastSpeedPoint,
				t.bRHPRegenPoint,
				t.fHitRange
			);
		}

		CDBManager::Instance().AsyncQuery(query);
	}
	return true;
}

bool CClientManager::MirrorItemTableIntoDB()
{
	for (auto it = m_vec_itemTable.begin(); it != m_vec_itemTable.end(); it++)
	{
		if (g_stLocaleNameColumn != "name")
		{
			const TItemTable& t = *it;
			char query[4096];
			snprintf(query, sizeof(query),
				"REPLACE INTO item_proto%s ("
				"vnum, "
				"type, "
				"subtype, "
				"name, "
				"%s, "
				"shop_buy_price, "
				"shop_sell_price, "
				"weight, "
				"size, "
				"flag, "
				"wearflag, "
				"antiflag, "
				"immuneflag, "
				"refine_element_apply_type, "
				"refine_element_grade, "
				"refine_element_value, "
				"refine_element_bonus, "
				"refined_vnum, "
				"refine_set, "
#ifdef ENABLE_ATTR_6TH_7TH
				"material67, "
#endif
				"magic_pct, "
				"socket_pct, "
				"addon_type, "
				"limittype0, "
				"limitvalue0, "
				"limittype1, "
				"limitvalue1, "
				"applytype0, "
				"applyvalue0, "
				"applytype1, "
				"applyvalue1, "
				"applytype2, "
				"applyvalue2, "
				"applytype3, "
				"applyvalue3, "
				"value0, "
				"value1, "
				"value2, "
				"value3, "
				"value4, "
				"value5, "
				"socket0, "
				"socket1, "
				"socket2, "
				"socket3, "
				"socket4, "
				"socket5, "
				"masktype, "
				"masksubtype "
				") "
				"VALUES ("
				"%u, "		// vnum
				"%u, "		// type
				"%u, "		// subtype
				"\"%s\", "	// name
				"\"%s\", "	// %s
				"%u, "		// shop_buy_price
				"%u, "		// shop_sell_price
				"%u, "		// weight
				"%u, "		// size
				"%u, "		// flag
				"%u, "		// wearflag
				"%u, "		// antiflag
				"%u, "		// immuneflag
				"%u, "		// refine_element_apply_type
				"%u, "		// refine_element_grade
				"%u, "		// refine_element_value
				"%u, "		// refine_element_bonus
				"%u, "		// refined_vnum
				"%d, "		// refine_set
#ifdef ENABLE_ATTR_6TH_7TH
				"%u, "		// material67
#endif
				"%u, "		// magic_pct
				"%u, "		// socket_pct
				"%d, "		// addon_type
				"%u, "		// limittype0
				"%ld, "		// limitvalue0
				"%u, "		// limittype1
				"%ld, "		// limitvalue1
				"%u, "		// applytype0
				"%ld, "		// applyvalue0
				"%u, "		// applytype1
				"%ld, "		// applyvalue1
				"%u, "		// applytype2
				"%ld, "		// applyvalue2
				"%u, "		// applytype3
				"%ld, "		// applyvalue3
				"%ld, "		// value0
				"%ld, "		// value1
				"%ld, "		// value2
				"%ld, "		// value3
				"%ld, "		// value4
				"%ld, "		// value5
				"%ld, "		// socket0
				"%ld, "		// socket1
				"%ld, "		// socket2
				"%ld, "		// socket3
				"%ld, "		// socket4
				"%ld, "		// socket5
				"%d, "		// masktype
				"%d "		// masksubtype
				")",
				GetTablePostfix(),
				g_stLocaleNameColumn.c_str(),
				t.dwVnum,
				t.bType,
				t.bSubType,
				t.szName,
				t.szLocaleName,
				t.dwShopBuyPrice,
				t.dwShopSellPrice,
				t.bWeight,
				t.bSize,
				t.dwFlags,
				t.dwWearFlags,
				t.dwAntiFlags,
				t.dwImmuneFlag,
				t.dwRefineElementApplyType,
				t.dwRefineElementGrade,
				t.dwRefineElementValue,
				t.dwRefineElementBonus,
				t.dwRefinedVnum,
				t.wRefineSet,
#ifdef ENABLE_ATTR_6TH_7TH
				t.dwMaterial67,
#endif
				t.bAlterToMagicItemPct,
				t.bGainSocketPct,
				t.sAddonType,
				t.aLimits[0].bType,
				t.aLimits[0].lValue,
				t.aLimits[1].bType,
				t.aLimits[1].lValue,
				t.aApplies[0].wType,
				t.aApplies[0].lValue,
				t.aApplies[1].wType,
				t.aApplies[1].lValue,
				t.aApplies[2].wType,
				t.aApplies[2].lValue,
				t.aApplies[3].wType,
				t.aApplies[3].lValue,
				t.alValues[0],
				t.alValues[1],
				t.alValues[2],
				t.alValues[3],
				t.alValues[4],
				t.alValues[5],
				t.alSockets[0],
				t.alSockets[1],
				t.alSockets[2],
				t.alSockets[3],
				t.alSockets[4],
				t.alSockets[5],
				t.bMaskType,
				t.bMaskSubType
			);
			CDBManager::Instance().AsyncQuery(query);
		}
		else
		{
			const TItemTable& t = *it;
			char query[4096];
			snprintf(query, sizeof(query),
				"REPLACE INTO item_proto%s ("
				"vnum, "
				"type, "
				"subtype, "
				"name, "
				"shop_buy_price, "
				"shop_sell_price, "
				"weight, "
				"size, "
				"flag, "
				"wearflag, "
				"antiflag, "
				"immuneflag, "
				"refine_element_apply_type, "
				"refine_element_grade, "
				"refine_element_value, "
				"refine_element_bonus, "
				"refined_vnum, "
				"refine_set, "
#ifdef ENABLE_ATTR_6TH_7TH
				"material67, "
#endif
				"magic_pct, "
				"socket_pct, "
				"addon_type, "
				"limittype0, "
				"limitvalue0, "
				"limittype1, "
				"limitvalue1, "
				"applytype0, "
				"applyvalue0, "
				"applytype1, "
				"applyvalue1, "
				"applytype2, "
				"applyvalue2, "
				"applytype3, "
				"applyvalue3, "
				"value0, "
				"value1, "
				"value2, "
				"value3, "
				"value4, "
				"value5, "
				"socket0, "
				"socket1, "
				"socket2, "
				"socket3, "
				"socket4, "
				"socket5, "
				"masktype, "
				"masksubtype "
				") "
				"VALUES ("
				"%d, "		// vnum
				"%d, "		// type
				"%d, "		// subtype
				"\"%s\", "	// name
				"%d, "		// shop_buy_price
				"%d, "		// shop_sell_price
				"%d, "		// weight
				"%d, "		// size
				"%d, "		// flag
				"%d, "		// wearflag
				"%d, "		// antiflag
				"%d, "		// immuneflag
				"%d, "		// refined_vnum
				"%u, "		// refine_element_apply_type
				"%u, "		// refine_element_grade
				"%u, "		// refine_element_value
				"%u, "		// refine_element_bonus
				"%d, "		// refine_set
#ifdef ENABLE_ATTR_6TH_7TH
				"%u, "		// material67
#endif
				"%d, "		// magic_pct
				"%d, "		// socket_pct
				"%d, "		// addon_type
				"%d, "		// limittype0
				"%ld, "		// limitvalue0
				"%d, "		// limittype1
				"%ld, "		// limitvalue1
				"%d, "		// applytype0
				"%ld, "		// applyvalue0
				"%d, "		// applytype1
				"%ld, "		// applyvalue1
				"%d, "		// applytype2
				"%ld, "		// applyvalue2
				"%d, "		// applytype3
				"%ld, "		// applyvalue3
				"%ld, "		// value0
				"%ld, "		// value1
				"%ld, "		// value2
				"%ld, "		// value3
				"%ld, "		// value4
				"%ld, "		// value5
				"%ld, "		// socket0
				"%ld, "		// socket1
				"%ld, "		// socket2
				"%ld, "		// socket3
				"%ld, "		// socket4
				"%ld, "		// socket5
				"%d, "		// masktype
				"%d "		// masksubtype
				")",
				GetTablePostfix(),
				t.dwVnum,
				t.bType,
				t.bSubType,
				t.szName,
				t.dwShopBuyPrice,
				t.dwShopSellPrice,
				t.bWeight,
				t.bSize,
				t.dwFlags,
				t.dwWearFlags,
				t.dwAntiFlags,
				t.dwImmuneFlag,
				t.dwRefineElementApplyType,
				t.dwRefineElementGrade,
				t.dwRefineElementValue,
				t.dwRefineElementBonus,
				t.dwRefinedVnum,
				t.wRefineSet,
#ifdef ENABLE_ATTR_6TH_7TH
				t.dwMaterial67,
#endif
				t.bAlterToMagicItemPct,
				t.bGainSocketPct,
				t.sAddonType,
				t.aLimits[0].bType,
				t.aLimits[0].lValue,
				t.aLimits[1].bType,
				t.aLimits[1].lValue,
				t.aApplies[0].wType,
				t.aApplies[0].lValue,
				t.aApplies[1].wType,
				t.aApplies[1].lValue,
				t.aApplies[2].wType,
				t.aApplies[2].lValue,
				t.aApplies[3].wType,
				t.aApplies[3].lValue,
				t.alValues[0],
				t.alValues[1],
				t.alValues[2],
				t.alValues[3],
				t.alValues[4],
				t.alValues[5],
				t.alSockets[0],
				t.alSockets[1],
				t.alSockets[2],
				t.alSockets[3],
				t.alSockets[4],
				t.alSockets[5],
				t.bMaskType,
				t.bMaskSubType
			);
			CDBManager::Instance().AsyncQuery(query);
		}
	}
	return true;
}

#ifdef ENABLE_PROTO_FROM_DB
bool CClientManager::InitializeMobTableFromDB()
{
	char query[2048];
	fprintf(stdout, "Loading mob_proto from MySQL\n");
	snprintf(query, sizeof(query),
		"SELECT "
		"vnum, "
		"name, "
		"%s, "
		"rank, "
		"type, "
		"battle_type, "
		"level, "
		"scale, "
		"size+0, "
		"ai_flag+0, "
#ifdef ENABLE_PROTO_RENEWAL
		"ai_flag_ex+0, "
#endif
		"mount_capacity, "
		"setRaceFlag+0, "
		"setImmuneFlag+0, "
		"empire, "
		"folder, "
		"on_click, "
		"st, "
		"dx, "
		"ht, "
		"iq, "
#ifdef ENABLE_YOHARA_SYSTEM
		"sungma_st, "
		"sungma_dx, "
		"sungma_ht, "
		"sungma_iq, "
#endif
		"damage_min, "
		"damage_max, "
		"max_hp, "
		"regen_cycle, "
		"regen_percent, "
		"gold_min, "
		"gold_max, "
		"exp, "
#ifdef ENABLE_YOHARA_SYSTEM
		"sungma_exp, "
#endif
		"def, "
		"attack_speed, "
		"move_speed, "
		"aggressive_hp_pct, "
		"aggressive_sight, "
		"attack_range, "
		"drop_item, "
		"resurrection_vnum, "
		"enchant_curse, "
		"enchant_slow, "
		"enchant_poison, "
		"enchant_stun, "
		"enchant_critical, "
		"enchant_penetrate, "
		"resist_fist, "
		"resist_sword, "
		"resist_twohand, "
		"resist_dagger, "
		"resist_bell, "
		"resist_fan, "
		"resist_bow, "
		"resist_claw, "
		"resist_fire, "
		"resist_elect, "
		"resist_magic, "
		"resist_wind, "
		"resist_poison, "
		"resist_bleeding, "
		"att_elec, "
		"att_fire, "
		"att_ice, "
		"att_wind, "
		"att_earth, "
		"att_dark, "
		"resist_dark, "
		"resist_ice, "
		"resist_earth, "
		"dam_multiply, "
		"summon, "
		"drain_sp, "
		"mob_color, "
		"polymorph_item, "
		"skill_level0, "
		"skill_vnum0, "
		"skill_level1, "
		"skill_vnum1, "
		"skill_level2, "
		"skill_vnum2, "
		"skill_level3, "
		"skill_vnum3, "
		"skill_level4, "
		"skill_vnum4, "
		"sp_berserk, "
		"sp_stoneskin, "
		"sp_godspeed, "
		"sp_deathblow, "
		"sp_revive, "
		"heal_point, "
		"att_speed_point, "
		"cast_speed_point, "
		"hp_regen_point, "
		"hit_range "
		"FROM mob_proto%s ORDER BY vnum;",
		g_stLocaleNameColumn.c_str(),
		GetTablePostfix()
	);

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	const auto addNumber = static_cast<uint32_t>(pRes->uiNumRows);
	if (addNumber == 0)
		return false;

	if (!m_vec_mobTable.empty())
	{
		sys_log(0, "RELOAD: mob_proto");
		m_vec_mobTable.clear();
	}

	m_vec_mobTable.resize(addNumber);
	memset(&m_vec_mobTable[0], 0, sizeof(TMobTable) * m_vec_mobTable.size());
	TMobTable* mob_table = &m_vec_mobTable[0];

	MYSQL_ROW data = nullptr;
	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		int col = 0;
		// check whether or not the field is nullptr or that contains an empty string
		// General
		mob_table->dwVnum = atoi(data[col++]);
		strlcpy(mob_table->szName, data[col++], sizeof(mob_table->szName));
		strlcpy(mob_table->szLocaleName, data[col++], sizeof(mob_table->szLocaleName));
		mob_table->bRank = get_Mob_Rank_Value(data[col++]);
		mob_table->bType = get_Mob_Type_Value(data[col++]);
		mob_table->bBattleType = get_Mob_BattleType_Value(data[col++]);
		mob_table->bLevel = atoi(data[col++]);
		col++; //scale
		mob_table->bSize = atoi(data[col++]);
		mob_table->dwAIFlag = atoi(data[col++]);
#ifdef ENABLE_PROTO_RENEWAL
		mob_table->dwAIFlagEx = atoi(data[col++]);
#endif
		col++; // mount_capacity
		mob_table->dwRaceFlag = atoi(data[col++]);
		mob_table->dwImmuneFlag = atoi(data[col++]);

		// Others
		mob_table->bEmpire = atoi(data[col++]);
		strlcpy(mob_table->szFolder, data[col++], sizeof(mob_table->szFolder));
		mob_table->bOnClickType = atoi(data[col++]);
		mob_table->bStr = atoi(data[col++]);
		mob_table->bDex = atoi(data[col++]);
		mob_table->bCon = atoi(data[col++]);
		mob_table->bInt = atoi(data[col++]);
#ifdef ENABLE_YOHARA_SYSTEM
		mob_table->bSungMaSt = atoi(data[col++]);
		mob_table->bSungMaDx = atoi(data[col++]);
		mob_table->bSungMaHt = atoi(data[col++]);
		mob_table->bSungMaIq = atoi(data[col++]);
#endif
		mob_table->dwDamageRange[0] = atoi(data[col++]);
		mob_table->dwDamageRange[1] = atoi(data[col++]);
		mob_table->dwMaxHP = atoi(data[col++]);
		mob_table->bRegenCycle = atoi(data[col++]);
		mob_table->bRegenPercent = atoi(data[col++]);
		mob_table->dwGoldMin = atoi(data[col++]);
		mob_table->dwGoldMax = atoi(data[col++]);
		mob_table->dwExp = atoi(data[col++]);
#ifdef ENABLE_YOHARA_SYSTEM
		mob_table->dwSungMaExp = atoi(data[col++]);
#endif
		mob_table->wDef = atoi(data[col++]);
		mob_table->sAttackSpeed = atoi(data[col++]);
		mob_table->sMovingSpeed = atoi(data[col++]);
		mob_table->bAggresiveHPPct = atoi(data[col++]);
		mob_table->wAggressiveSight = atoi(data[col++]);
		mob_table->wAttackRange = atoi(data[col++]);
		mob_table->dwDropItemVnum = atoi(data[col++]);
		mob_table->dwResurrectionVnum = atoi(data[col++]);

		// Enchant
		mob_table->cEnchants[MOB_ENCHANT_CURSE] = atoi(data[col++]);
		mob_table->cEnchants[MOB_ENCHANT_SLOW] = atoi(data[col++]);
		mob_table->cEnchants[MOB_ENCHANT_POISON] = atoi(data[col++]);
		mob_table->cEnchants[MOB_ENCHANT_STUN] = atoi(data[col++]);
		mob_table->cEnchants[MOB_ENCHANT_CRITICAL] = atoi(data[col++]);
		mob_table->cEnchants[MOB_ENCHANT_PENETRATE] = atoi(data[col++]);

		// Resist
		mob_table->cResists[MOB_RESIST_FIST] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_SWORD] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_TWOHAND] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_DAGGER] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_BELL] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_FAN] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_BOW] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_CLAW] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_FIRE] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_ELECT] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_MAGIC] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_WIND] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_POISON] = atoi(data[col++]);
		mob_table->cResists[MOB_RESIST_BLEEDING] = atoi(data[col++]);

		// Element
		mob_table->cElementalFlags[MOB_ELEMENTAL_ELEC] = atoi(data[col++]);
		mob_table->cElementalFlags[MOB_ELEMENTAL_FIRE] = atoi(data[col++]);
		mob_table->cElementalFlags[MOB_ELEMENTAL_ICE] = atoi(data[col++]);
		mob_table->cElementalFlags[MOB_ELEMENTAL_WIND] = atoi(data[col++]);
		mob_table->cElementalFlags[MOB_ELEMENTAL_EARTH] = atoi(data[col++]);
		mob_table->cElementalFlags[MOB_ELEMENTAL_DARK] = atoi(data[col++]);

		// Resists
		mob_table->cResistDark = atoi(data[col++]);
		mob_table->cResistIce = atoi(data[col++]);
		mob_table->cResistEarth = atoi(data[col++]);

		// ## OTHERS #2
		mob_table->fDamMultiply = static_cast<float>(atof(data[col++]));
		mob_table->dwSummonVnum = atoi(data[col++]);
		mob_table->dwDrainSP = atoi(data[col++]);
		mob_table->dwMobColor = atoi(data[col++]);
		mob_table->dwPolymorphItemVnum = atoi(data[col++]);

		mob_table->Skills[0].bLevel = atoi(data[col++]);
		mob_table->Skills[0].dwVnum = atoi(data[col++]);
		mob_table->Skills[1].bLevel = atoi(data[col++]);
		mob_table->Skills[1].dwVnum = atoi(data[col++]);
		mob_table->Skills[2].bLevel = atoi(data[col++]);
		mob_table->Skills[2].dwVnum = atoi(data[col++]);
		mob_table->Skills[3].bLevel = atoi(data[col++]);
		mob_table->Skills[3].dwVnum = atoi(data[col++]);
		mob_table->Skills[4].bLevel = atoi(data[col++]);
		mob_table->Skills[4].dwVnum = atoi(data[col++]);

		// ## SPECIAL
		mob_table->bBerserkPoint = atoi(data[col++]);
		mob_table->bStoneSkinPoint = atoi(data[col++]);
		mob_table->bGodSpeedPoint = atoi(data[col++]);
		mob_table->bDeathBlowPoint = atoi(data[col++]);
		mob_table->bRevivePoint = atoi(data[col++]);

		// incoming
		col++; // heal_point
		col++; // att_speed_point
		col++; // cast_speed_point
		col++; // hp_regen_point
		mob_table->fHitRange = static_cast<float>(atof(data[col++])); // hit_range

		sys_log(0,
			"MOB #%-5d\t"
			"NAME: %-24s\t"
			"LOCALE_NAME: %-24s\t"
			"RANK: %u\t"
			"TYPE: %u\t"
			"BATTLE_TYPE: %u\t"
			"LEVEL: %u\t"
			"SIZE: %u\t"
			"AI_FLAG: %u\t"
#ifdef ENABLE_PROTO_RENEWAL
			"AI_FLAG_EX: %u\t"
#endif
			"RACE_FLAG: %u\t"
			"IMMUNE: %u\t"
			"EMPIRE: %u\t"
			"FOLDER: %-24s\t"
			"ONCLICK: %u\t"
			"STR: %u\t"
			"DEX: %u\t"
			"CON: %u\t"
			"INT: %u\t"
#ifdef ENABLE_YOHARA_SYSTEM
			"SUNGMA_STR: %u\t"
			"SUNGMA_DEX: %u\t"
			"SUNGMA_CON: %u\t"
			"SUNGMA_INT: %u\t"
#endif
			"DAMAGE_MIN: %u\t"
			"DAMAGE_MAX: %u\t"
			"MAX_HP: %u\t"
			"REGEN_CICLE: %u\t"
			"REGEN_PERCENT: %u\t"
			"GOLD_MIN: %u\t"
			"GOLD_MAX: %u\t"
			"EXP: %u\t"
#ifdef ENABLE_YOHARA_SYSTEM
			"SUNGMA_EXP: %u\t"
#endif
			"DEF: %u\t"
			"ATTACK_SPEED: %u\t"
			"MOVE_SPEED: %u\t"
			"AGRESSIVE_HP_PCT: %u\t"
			"AGRESSIVE_SIGHT: %u\t"
			"ATTACK_RANGE: %u\t"
			"DROP_ITEM_VNUM: %u\t"
			"RESSURECTION_VNUM: %u\t"
			"ENCHANTS: %d %d %d %d %d %d\t"
			"RESISTS: %d %d %d %d %d %d %d %d %d %d %d %d %d %d\t"
			"ELEMENTS: %d %d %d %d %d %d\t"
			"RESIST_DARK: %d\t"
			"RESIST_ICE: %d\t"
			"RESIST_EARTH: %d\t"
			"DAMAGE_MULTIPLY: %0.1f\t"
			"SUMMON_VNUM: %d\t"
			"DRAIN_SP: %d\t"
			"MOB_COLOR: %d\t"
			"POLYMORPH_VNUM: %d\t"
			"SKILLS: %d %d %d %d %d %d %d %d %d %d\t"
			"BERSERK_POINT: %d\t"
			"STONE_SKIN_POINT: %d\t"
			"GOD_SPEED_POINT: %d\t"
			"DEATH_BLOW_POINT: %d\t"
			"REVIVE_POINT: %d\t"
			"HIT_RANGE: %0.1f",
			mob_table->dwVnum,
			mob_table->szName,
			mob_table->szLocaleName,
			mob_table->bRank,
			mob_table->bType,
			mob_table->bBattleType,
			mob_table->bLevel,
			mob_table->bSize,
			mob_table->dwAIFlag,
#ifdef ENABLE_PROTO_RENEWAL
			mob_table->dwAIFlagEx,
#endif
			mob_table->dwRaceFlag,
			mob_table->dwImmuneFlag,
			mob_table->bEmpire,
			mob_table->szFolder,
			mob_table->bOnClickType,
			mob_table->bStr,
			mob_table->bDex,
			mob_table->bCon,
			mob_table->bInt,
#ifdef ENABLE_YOHARA_SYSTEM
			mob_table->bSungMaSt,
			mob_table->bSungMaDx,
			mob_table->bSungMaHt,
			mob_table->bSungMaIq,
#endif
			mob_table->dwDamageRange[0],
			mob_table->dwDamageRange[1],
			mob_table->dwMaxHP,
			mob_table->bRegenCycle,
			mob_table->bRegenPercent,
			mob_table->dwGoldMin,
			mob_table->dwGoldMax,
			mob_table->dwExp,
#ifdef ENABLE_YOHARA_SYSTEM
			mob_table->dwSungMaExp,
#endif
			mob_table->wDef,
			mob_table->sAttackSpeed,
			mob_table->sMovingSpeed,
			mob_table->bAggresiveHPPct,
			mob_table->wAggressiveSight,
			mob_table->wAttackRange,
			mob_table->dwDropItemVnum,
			mob_table->dwResurrectionVnum,
			mob_table->cEnchants[MOB_ENCHANT_CURSE], mob_table->cEnchants[MOB_ENCHANT_SLOW], mob_table->cEnchants[MOB_ENCHANT_POISON],
			mob_table->cEnchants[MOB_ENCHANT_STUN], mob_table->cEnchants[MOB_ENCHANT_CRITICAL], mob_table->cEnchants[MOB_ENCHANT_PENETRATE],
			mob_table->cResists[MOB_RESIST_FIST], mob_table->cResists[MOB_RESIST_SWORD], mob_table->cResists[MOB_RESIST_TWOHAND], mob_table->cResists[MOB_RESIST_DAGGER],
			mob_table->cResists[MOB_RESIST_BELL], mob_table->cResists[MOB_RESIST_FAN], mob_table->cResists[MOB_RESIST_BOW], mob_table->cResists[MOB_RESIST_CLAW],
			mob_table->cResists[MOB_RESIST_FIRE], mob_table->cResists[MOB_RESIST_ELECT], mob_table->cResists[MOB_RESIST_MAGIC], mob_table->cResists[MOB_RESIST_WIND],
			mob_table->cResists[MOB_RESIST_POISON], mob_table->cResists[MOB_RESIST_BLEEDING],
			mob_table->cElementalFlags[MOB_ELEMENTAL_ELEC], mob_table->cElementalFlags[MOB_ELEMENTAL_FIRE], mob_table->cElementalFlags[MOB_ELEMENTAL_ICE],
			mob_table->cElementalFlags[MOB_ELEMENTAL_WIND], mob_table->cElementalFlags[MOB_ELEMENTAL_EARTH], mob_table->cElementalFlags[MOB_ELEMENTAL_DARK],
			mob_table->cResistDark, mob_table->cResistIce, mob_table->cResistEarth,
			mob_table->fDamMultiply,
			mob_table->dwSummonVnum,
			mob_table->dwDrainSP,
			mob_table->dwMobColor,
			mob_table->dwPolymorphItemVnum,
			mob_table->Skills[0].bLevel, mob_table->Skills[0].dwVnum,
			mob_table->Skills[1].bLevel, mob_table->Skills[1].dwVnum,
			mob_table->Skills[2].bLevel, mob_table->Skills[2].dwVnum,
			mob_table->Skills[3].bLevel, mob_table->Skills[3].dwVnum,
			mob_table->Skills[4].bLevel, mob_table->Skills[4].dwVnum,
			mob_table->bBerserkPoint,
			mob_table->bStoneSkinPoint,
			mob_table->bGodSpeedPoint,
			mob_table->bDeathBlowPoint,
			mob_table->bRevivePoint,
			mob_table->fHitRange);

		++mob_table;
	}
	sort(m_vec_mobTable.begin(), m_vec_mobTable.end(), FCompareVnum());

	fprintf(stdout, "Complete! %u Mobs loaded.\n", addNumber);
	return true;
}

bool CClientManager::InitializeItemTableFromDB()
{
	char query[2048];
	fprintf(stdout, "Loading item_proto from MySQL\n");
	snprintf(query, sizeof(query),
		"SELECT "
		"vnum, "
		"name, "
		"%s, "
		"type, "
		"subtype, "
		"weight, "
		"size, "
		"antiflag+0, "
		"flag+0, "
		"wearflag+0, "
		"immuneflag+0, "
		"shop_buy_price, "
		"shop_sell_price, "
		"refine_element_apply_type, "
		"refine_element_grade, "
		"refine_element_value, "
		"refine_element_bonus, "
		"refined_vnum, "
		"refine_set, "
		"material67, "
		"magic_pct, "
		"limittype0, "
		"limitvalue0, "
		"limittype1, "
		"limitvalue1, "
		"applytype0, "
		"applyvalue0, "
		"applytype1, "
		"applyvalue1, "
		"applytype2, "
		"applyvalue2, "
		"applytype3, "
		"applyvalue3, "
		"value0, "
		"value1, "
		"value2, "
		"value3, "
		"value4, "
		"value5, "
		"socket0, "
		"socket1, "
		"socket2, "
		"socket3, "
		"socket4, "
		"socket5, "
		"specular, "
		"socket_pct, "
		"addon_type, "
		"masktype, "
		"masksubtype "
		"FROM item_proto%s ORDER BY vnum;",
		g_stLocaleNameColumn.c_str(),
		GetTablePostfix()
	);

	auto pkMsg(CDBManager::Instance().DirectQuery(query));
	SQLResult* pRes = pkMsg->Get();

	const auto addNumber = static_cast<uint32_t>(pRes->uiNumRows);
	if (addNumber == 0)
		return false;

	if (!m_vec_itemTable.empty())
	{
		sys_log(0, "RELOAD: item_proto");
		m_vec_itemTable.clear();
		m_map_itemTableByVnum.clear();
	}

	m_vec_itemTable.resize(addNumber);
	memset(&m_vec_itemTable[0], 0, sizeof(TItemTable) * m_vec_itemTable.size());
	TItemTable* item_table = &m_vec_itemTable[0];

	MYSQL_ROW data = nullptr;
	while ((data = mysql_fetch_row(pRes->pSQLResult)))
	{
		int col = 0;
		// check whether or not the field is nullptr or that contains an empty string
		// General
		item_table->dwVnum = atoi(data[col++]);
		strlcpy(item_table->szName, data[col++], sizeof(item_table->szName));
		strlcpy(item_table->szLocaleName, data[col++], sizeof(item_table->szLocaleName));
		item_table->bType = get_Item_Type_Value(data[col++]);
		item_table->bSubType = get_Item_SubType_Value(item_table->bType, data[col++]);
		item_table->bWeight = atoi(data[col++]);
		item_table->bSize = atoi(data[col++]);
		item_table->dwAntiFlags = atoi(data[col++]);
		item_table->dwFlags = atoi(data[col++]);
		item_table->dwWearFlags = atoi(data[col++]);
		item_table->dwImmuneFlag = atoi(data[col++]);
		item_table->dwShopBuyPrice = atoi(data[col++]);
		item_table->dwShopSellPrice = atoi(data[col++]);

		// Refine Element
		item_table->dwRefineElementApplyType = atoi(data[col++]);
		item_table->dwRefineElementGrade = atoi(data[col++]);
		item_table->dwRefineElementValue = atoi(data[col++]);
		item_table->dwRefineElementBonus = atoi(data[col++]);

		// Refine
		item_table->dwRefinedVnum = atoi(data[col++]);
		item_table->wRefineSet = atoi(data[col++]);
		item_table->dwMaterial67 = atoi(data[col++]);
		item_table->bAlterToMagicItemPct = atoi(data[col++]);

		// Limit
		item_table->cLimitRealTimeFirstUseIndex = -1;
		item_table->cLimitTimerBasedOnWearIndex = -1;

		const uint8_t bType0 = get_Item_LimitType_Value(data[col++]);
		item_table->aLimits[0].bType = bType0;
		item_table->aLimits[0].lValue = atoi(data[col++]);

		if (LIMIT_REAL_TIME_START_FIRST_USE == bType0)
			item_table->cLimitRealTimeFirstUseIndex = 0;
		else if (LIMIT_TIMER_BASED_ON_WEAR == bType0)
			item_table->cLimitTimerBasedOnWearIndex = 0;

		const uint8_t bType1 = get_Item_LimitType_Value(data[col++]);
		item_table->aLimits[1].bType = bType1;
		item_table->aLimits[1].lValue = atoi(data[col++]);

		if (LIMIT_REAL_TIME_START_FIRST_USE == bType1)
			item_table->cLimitRealTimeFirstUseIndex = 1;
		else if (LIMIT_TIMER_BASED_ON_WEAR == bType1)
			item_table->cLimitTimerBasedOnWearIndex = 1;

		if ((LIMIT_NONE != bType0) && (bType0 == bType1)) // just checking the first limit one is enough
			sys_log(0, "vnum(%u): limittype0(%u)==limittype1(%u)", item_table->dwVnum, bType0, bType1); // @warme012

		// Apply
		item_table->aApplies[0].wType = get_Item_ApplyType_Value(data[col++]);
		item_table->aApplies[0].lValue = atoi(data[col++]);
		item_table->aApplies[1].wType = get_Item_ApplyType_Value(data[col++]);
		item_table->aApplies[1].lValue = atoi(data[col++]);
		item_table->aApplies[2].wType = get_Item_ApplyType_Value(data[col++]);
		item_table->aApplies[2].lValue = atoi(data[col++]);
		item_table->aApplies[3].wType = get_Item_ApplyType_Value(data[col++]);
		item_table->aApplies[3].lValue = atoi(data[col++]);

		// Value
		item_table->alValues[0] = atoi(data[col++]);
		item_table->alValues[1] = atoi(data[col++]);
		item_table->alValues[2] = atoi(data[col++]);
		item_table->alValues[3] = atoi(data[col++]);
		item_table->alValues[4] = atoi(data[col++]);
		item_table->alValues[5] = atoi(data[col++]);

		// Socket
		item_table->alSockets[0] = atoi(data[col++]);
		item_table->alSockets[1] = atoi(data[col++]);
		item_table->alSockets[2] = atoi(data[col++]);
		item_table->alSockets[3] = atoi(data[col++]);
		item_table->alSockets[4] = atoi(data[col++]);
		item_table->alSockets[5] = atoi(data[col++]);

		// Others
		item_table->bSpecular = atoi(data[col++]);
		item_table->bGainSocketPct = atoi(data[col++]);
		item_table->sAddonType = atoi(data[col++]);

		// MaskType
		item_table->bMaskType = get_Mask_Type_Value(data[col++]);
		//MaskSubType
		item_table->bMaskSubType = get_Mask_SubType_Value(item_table->bMaskType, data[col++]);

#if !defined(ENABLE_AUTODETECT_VNUMRANGE)
		VERIFY_IFIELD(IProto::vnum_range, item_table->dwVnumRange);
#else
		if (item_table->bType == ITEM_DS)
			item_table->dwVnumRange = 99;
#endif

		m_map_itemTableByVnum.insert(std::map<uint32_t, TItemTable*>::value_type(item_table->dwVnum, item_table));
		sys_log(0,
			"ITEM: #%-5lu\t"
			"NAME: %-24s\t"
			"LOCALE_NAME: %-24s\t"
			"TYPE: %d\t"
			"SUBTYPE: %d\t"
			"WEIGHT: %d\t"
			"SIZE: %d\t"
			"ANTI: %d\t"
			"FLAGS: %d\t"
			"WEAR: %d\t"
			"IMMUNE: %d\t"
			"SHOP_BUY: %d\t"
			"SHOP_SELL: %d\t"
			"ELEMENT_TYPE: %u\t"
			"ELEMENT_GRADE: %u\t"
			"ELEMENT_VALUE: %u\t"
			"ELEMENT_BONUS: %u\t"
			"REFINED_VNUM: %d\t"
			"REFINE_SET: %d\t"
			"MATERIAL67: %d\t"
			"MAGIC_PCT: %d\t"
			"LIMIT: %d %ld %d %ld\t"
			"APPLY: %u %ld %u %ld %u %ld %u %ld\t"
			"VAL: %ld %ld %ld %ld %ld %ld\t"
			"SOCKET: %ld %ld %ld %ld %ld %ld\t"
			"SPECULAR %d\t"
			"SOCKET_PCT %d\t"
			"ADDON %d",
			item_table->dwVnum,
			item_table->szName,
			item_table->szLocaleName,
			item_table->bType,
			item_table->bSubType,
			item_table->bWeight,
			item_table->bSize,
			item_table->dwAntiFlags,
			item_table->dwFlags,
			item_table->dwWearFlags,
			item_table->dwImmuneFlag,
			item_table->dwShopBuyPrice,
			item_table->dwShopSellPrice,
			item_table->dwRefineElementApplyType,
			item_table->dwRefineElementGrade,
			item_table->dwRefineElementValue,
			item_table->dwRefineElementBonus,
			item_table->dwRefinedVnum,
			item_table->wRefineSet,
			item_table->dwMaterial67,
			item_table->bAlterToMagicItemPct,
			item_table->aLimits[0].bType,
			item_table->aLimits[0].lValue,
			item_table->aLimits[1].bType,
			item_table->aLimits[1].lValue,
			item_table->aApplies[0].wType,
			item_table->aApplies[0].lValue,
			item_table->aApplies[1].wType,
			item_table->aApplies[1].lValue,
			item_table->aApplies[2].wType,
			item_table->aApplies[2].lValue,
			item_table->aApplies[3].wType,
			item_table->aApplies[3].lValue,
			item_table->alValues[0],
			item_table->alValues[1],
			item_table->alValues[2],
			item_table->alValues[3],
			item_table->alValues[4],
			item_table->alValues[5],
			item_table->alSockets[0],
			item_table->alSockets[1],
			item_table->alSockets[2],
			item_table->alSockets[3],
			item_table->alSockets[4],
			item_table->alSockets[5],
			item_table->bSpecular,
			item_table->bGainSocketPct,
			item_table->sAddonType
		);
		item_table++;
	}
	sort(m_vec_itemTable.begin(), m_vec_itemTable.end(), FCompareVnum());

	fprintf(stdout, "Complete! %u Items loaded.\n", addNumber);
	return true;
}
#endif

#ifdef ENABLE_EVENT_MANAGER
bool CClientManager::InitializeEventTable()
{
	char query[4096] = { '\0' };
	snprintf(&query[0], sizeof(query), "SELECT id, type, UNIX_TIMESTAMP(start), UNIX_TIMESTAMP(end), "
		"vnum, percent, drop_type+0, completed "
		"FROM event%s ORDER BY start", GetTablePostfix());

	auto pkMsg(CDBManager::Instance().DirectQuery(&query[0]));
	SQLResult* pRes = pkMsg->Get();

	if (!m_vec_eventTable.empty())
	{
		sys_log(0, "RELOAD: event");
		m_vec_eventTable.clear();
	}

	m_vec_eventTable.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
	{
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TEventTable t;
			memset(&t, 0, sizeof(t));

			int col = 0;

			str_to_number(t.dwID, data[col++]);
			strlcpy(t.szType, data[col++], sizeof(t.szType));
			str_to_number(t.startTime, data[col++]);
			str_to_number(t.endTime, data[col++]);
			str_to_number(t.dwVnum, data[col++]);
			str_to_number(t.iPercent, data[col++]);
			str_to_number(t.iDropType, data[col++]);
			str_to_number(t.bCompleted, data[col++]);

			sys_log(0, "EVENT: %s start %lu end %lu", t.szType, t.startTime, t.endTime);

			m_vec_eventTable.emplace_back(t);
		}
	}

	return true;
}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
bool CClientManager::InitializeBiologMissions()
{
	char szQuery[4096];
	snprintf(szQuery, sizeof(szQuery), "SELECT `mission`, `sub_mission`, `npc_mission`,"
		"`required_lvl`, `required_item`, `required_item_count`, `required_sub_item`,"
		"`cooldown`, `chance` "
		"FROM biolog_missions%s ORDER BY mission;", GetTablePostfix());

	auto pMsg(CDBManager::Instance().DirectQuery(szQuery));
	SQLResult* pRes = pMsg->Get();

	if (pMsg->uiSQLErrno != 0)
	{
		sys_err("CANNOT LOAD biolog_missions TABLE , errorcode %d ", pMsg->uiSQLErrno);
		return false;
	}

	if (!m_vec_BiologMissions.empty())
	{
		sys_log(0, "RELOAD: biolog_missions upgrade");
		m_vec_BiologMissions.clear();
	}

	m_vec_BiologMissions.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
	{
		sys_log(0, "BIOLOG_MISSIONS: Table size is %u", pRes->uiNumRows);

		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TBiologMissionsProto b;
			memset(&b, 0, sizeof(TBiologMissionsProto));

			int col = 0;

			str_to_number(b.bMission, data[col++]);
			str_to_number(b.bSubMission, data[col++]);
			str_to_number(b.dwNpcVnum, data[col++]);
			str_to_number(b.bRequiredLevel, data[col++]);
			str_to_number(b.iRequiredItem, data[col++]);
			str_to_number(b.wRequiredItemCount, data[col++]);
			str_to_number(b.dwRequiredSubItem, data[col++]);
			str_to_number(b.iCooldown, data[col++]);
			str_to_number(b.bChance, data[col++]);

			sys_log(0, "BIOLOG_MISSIONS: Added to vector: mission %u required_level: %u required_item: %u required_count: %u chance: %u",
				b.bMission, b.bRequiredLevel, b.iRequiredItem, b.wRequiredItemCount, b.bChance);

			m_vec_BiologMissions.emplace_back(b);
		}
	}

	return true;
}

bool CClientManager::InitializeBiologRewards()
{
	char szQuery[4096];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT mission,"
		"reward_item, reward_item_count,"
		"apply_type0+0, apply_value0,"
		"apply_type1+0, apply_value1,"
		"apply_type2+0, apply_value2,"
		"apply_type3+0, apply_value3,"
		"apply_type4+0, apply_value4"
		" FROM biolog_rewards%s ORDER by mission;", GetTablePostfix());

	auto pMsg(CDBManager::Instance().DirectQuery(szQuery));
	SQLResult* pRes = pMsg->Get();

	if (pMsg->uiSQLErrno != 0)
	{
		sys_err("CANNOT LOAD biolog_rewards TABLE , errorcode %d ", pMsg->uiSQLErrno);
		return false;
	}

	if (!m_vec_BiologRewards.empty())
	{
		sys_log(0, "RELOAD: biolog_missions upgrade");
		m_vec_BiologRewards.clear();
	}

	m_vec_BiologRewards.reserve(static_cast<uint32_t>(pRes->uiNumRows));

	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
	{
		sys_log(0, "BIOLOG_REWARDS: Table size is %u", pRes->uiNumRows);

		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TBiologRewardsProto b;
			memset(&b, 0, sizeof(TBiologRewardsProto));

			int col = 0;

			str_to_number(b.bMission, data[col++]);
			str_to_number(b.dRewardItem, data[col++]);
			str_to_number(b.wRewardItemCount, data[col++]);

			for (uint8_t i = 0; i < MAX_BONUSES_LENGTH; i++)
			{
				str_to_number(b.wApplyType[i], data[col++]);
				str_to_number(b.lApplyValue[i], data[col++]);
			}

			sys_log(0, "BIOLOG_REWARDS: Added to vector: mission %u"
				"apply_type0: %u aply_value0: %u,"
				"apply_type1: %u aply_value1: %u,"
				"apply_type2: %u aply_value2: %u"
				"apply_type3: %u aply_value3: %u"
				"apply_type4: %u aply_value4: %u"
				,
				b.bMission,
				b.wApplyType[0], b.lApplyValue[0],
				b.wApplyType[1], b.lApplyValue[1],
				b.wApplyType[2], b.lApplyValue[2],
				b.wApplyType[3], b.lApplyValue[3],
				b.wApplyType[4], b.lApplyValue[4]
			);

			m_vec_BiologRewards.emplace_back(b);
		}
	}

	return true;
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
bool CClientManager::InitializeSungmaTable()
{
	char query[4096] = { '\0' };
	snprintf(&query[0], sizeof(query), "SELECT mapidx, sungma_str, sungma_hp, sungma_move, sungma_immune, hit_pct "
	"FROM common.sungma_table ORDER BY mapidx");

	auto pkMsg(CDBManager::Instance().DirectQuery(&query[0]));
	SQLResult* pRes = pkMsg->Get();

	if (!m_vec_sungmaTable.empty())
	{
		sys_log(0, "RELOAD: sungma_table");
		m_vec_sungmaTable.clear();
	}

	m_vec_sungmaTable.reserve(static_cast<uint32_t>(pRes->uiNumRows));
	MYSQL_ROW data;

	if (pRes->uiNumRows > 0)
	{
		while ((data = mysql_fetch_row(pRes->pSQLResult)))
		{
			TSungmaTable t{};

			int col = 0;

			str_to_number(t.lMapIdx, data[col++]);
			str_to_number(t.bSungmaStr, data[col++]);
			str_to_number(t.bSungmaHp, data[col++]);
			str_to_number(t.bSungmaMove, data[col++]);
			str_to_number(t.bSungmaImmune, data[col++]);
			str_to_number(t.bHitPct, data[col++]);

			sys_log(0, "SUNGMA: mapidx %d str %d hp %d move %d immune %d hit_pct %d", t.lMapIdx, t.bSungmaStr, t.bSungmaHp, t.bSungmaMove, t.bSungmaImmune, t.bHitPct);

			m_vec_sungmaTable.emplace_back(t);
		}
	}

	return true;
}
#endif
