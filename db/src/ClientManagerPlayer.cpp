
#include "stdafx.h"

#include "ClientManager.h"

#include "Main.h"
#include "QID.h"
#include "ItemAwardManager.h"
#include "HB.h"
#include "Cache.h"

extern bool g_bHotBackup;

extern std::string g_stLocale;
extern int g_test_server;
extern int g_log;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
extern std::map<uint32_t, spShop> shopMap;
#endif

//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!! IMPORTANT !!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Check all SELECT syntax on item table before change this function!!!
//
bool CreateItemTableFromRes(MYSQL_RES* res, std::vector<TPlayerItem>* pVec, uint32_t dwPID)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int rows;

	if ((rows = static_cast<int>(mysql_num_rows(res))) <= 0) // no data
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerItem& item = pVec->at(i);

		int cur = 0;

		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		// Check all SELECT syntax on item table before change this function!!!
		str_to_number(item.id, row[cur++]);
		str_to_number(item.window, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		str_to_number(item.count, row[cur++]);
		str_to_number(item.vnum, row[cur++]);
#ifdef ENABLE_SEALBIND_SYSTEM
		str_to_number(item.nSealDate, row[cur++]);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		str_to_number(item.dwTransmutationVnum, row[cur++]);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
		str_to_number(item.is_basic, row[cur++]);
#endif
#ifdef ENABLE_REFINE_ELEMENT
		str_to_number(item.grade_element, row[cur++]);
		str_to_number(item.attack_element[0], row[cur++]);
		str_to_number(item.attack_element[1], row[cur++]);
		str_to_number(item.attack_element[2], row[cur++]);
		str_to_number(item.element_type_bonus, row[cur++]);
		str_to_number(item.elements_value_bonus[0], row[cur++]);
		str_to_number(item.elements_value_bonus[1], row[cur++]);
		str_to_number(item.elements_value_bonus[2], row[cur++]);
#endif
		str_to_number(item.alSockets[0], row[cur++]);
		str_to_number(item.alSockets[1], row[cur++]);
		str_to_number(item.alSockets[2], row[cur++]);
#ifdef ENABLE_PROTO_RENEWAL
		str_to_number(item.alSockets[3], row[cur++]);
		str_to_number(item.alSockets[4], row[cur++]);
		str_to_number(item.alSockets[5], row[cur++]);
#endif

		for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
		{
			str_to_number(item.aAttr[j].wType, row[cur++]);
			str_to_number(item.aAttr[j].sValue, row[cur++]);
		}

#ifdef ENABLE_YOHARA_SYSTEM
		for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; s++)
		{
			str_to_number(item.aApplyRandom[s].wType, row[cur++]);
			str_to_number(item.aApplyRandom[s].sValue, row[cur++]);
			str_to_number(item.aApplyRandom[s].row, row[cur++]);
		}

		for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; r++)
		{
			str_to_number(item.alRandomValues[r], row[cur++]);
		}
#endif

#ifdef ENABLE_SET_ITEM
		str_to_number(item.set_value, row[cur++]);
#endif

/*#ifdef ENABLE_PET_CACHE_TEST // TEST
		memset(&item.aPetInfo, 0, sizeof(item.aPetInfo));
#endif*/

		item.owner = dwPID;
	}

	return true;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
bool CreateSkillColorTableFromRes(MYSQL_RES* res, uint32_t* dwSkillColor)
{
	if (mysql_num_rows(res) == 0)
		return false;

	MYSQL_ROW row = mysql_fetch_row(res);

	for (int x = 0; x < ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT; ++x)
	{
		for (int i = 0; i < ESkillColorLength::MAX_EFFECT_COUNT; ++i)
		{
			*(dwSkillColor++) = atoi(row[i + (x * ESkillColorLength::MAX_EFFECT_COUNT)]);
		}
	}

	return true;
}
#endif

size_t CreatePlayerSaveQuery(char* pszQuery, size_t querySize, TPlayerTable* pkTab)
{
	size_t queryLen;

	queryLen = snprintf(pszQuery, querySize,
		"UPDATE player%s SET "
		"job = %d, "
		"voice = %d, "
		"dir = %d, "
		"x = %d, "
		"y = %d, "
		"z = %d, "
		"map_index = %d, "
		"exit_x = %ld, "
		"exit_y = %ld, "
		"exit_map_index = %ld, "
		"hp = %d, "
		"mp = %d, "
		"stamina = %d, "
		"random_hp = %d, "
		"random_sp = %d, "
		"playtime = %d, "
		"level = %d, "
		"level_step = %d, "
		"st = %d, "
		"ht = %d, "
		"dx = %d, "
		"iq = %d, "
		"gold = %d, "
#ifdef ENABLE_CHEQUE_SYSTEM
		"cheque = %d, "
#endif
#ifdef ENABLE_GEM_SYSTEM
		"gem = %d, "
#endif
		"exp = %u, "
		"stat_point = %d, "
		"skill_point = %d, "
		"sub_skill_point = %d, "
		"stat_reset_count = %d, "
		"ip = '%s', "
		"part_main = %d, "
		"part_hair = %d, "
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		"part_acce = %d, "
#endif
#ifdef ENABLE_AURA_SYSTEM
		"part_aura = %d, "
#endif
#ifdef ENABLE_HORSE_APPEARANCE
		"horse_appearance = %u, "
#endif
		"last_play = NOW(), "
		"skill_group = %d, "
		"alignment = %ld, "
		"horse_level = %d, "
		"horse_riding = %d, "
		"horse_hp = %d, "
		"horse_hp_droptime = %u, "
		"horse_stamina = %d, "
		"horse_skill_point = %d, "
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		"gem_next_refresh = %d, "
#endif
#ifdef ENABLE_BATTLE_FIELD
		"battle_point = %d, "
		"usable_battle_point = %d, "
		"daily_time_battle_point = %d, "
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
		"medal_honor = %d, "
		"usable_medal_honor = %d, "
		"daily_time_medal_honor = %d, "
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		"conqueror_st = %d, "
		"conqueror_ht = %d, "
		"conqueror_mov = %d, "
		"conqueror_imu = %d, "
		"conqueror_level = %d, "
		"conqueror_level_step = %d, "
		"conqueror_exp = %u, "
		"conqueror_point = %d, "
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
		"biolog_mission = %d, "
		"biolog_collected = %d, "
		"biolog_cooldown_reminder = %d, "
		"biolog_cooldown = %ld, "
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		"inventory_stage = %d, "
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
		"special_stage_1 = %d, "
		"special_stage_2 = %d, "
		"special_stage_3 = %d, "
# endif
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		"soul = %d, "
		"soulre = %d, "
#endif
#ifdef ENABLE_FISH_EVENT
		"fish_use_count = %d, "
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		"battle_pass_premium_id = %d, "
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		"page_equipment = %d, "
		"unlock_page_equipment = %d, "
#endif
		,
		GetTablePostfix(),
		pkTab->job,
		pkTab->voice,
		pkTab->dir,
		pkTab->x,
		pkTab->y,
		pkTab->z,
		pkTab->lMapIndex,
		pkTab->lExitX,
		pkTab->lExitY,
		pkTab->lExitMapIndex,
		pkTab->hp,
		pkTab->sp,
		pkTab->stamina,
		pkTab->sRandomHP,
		pkTab->sRandomSP,
		pkTab->playtime,
		pkTab->level,
		pkTab->level_step,
		pkTab->st,
		pkTab->ht,
		pkTab->dx,
		pkTab->iq,
		pkTab->gold,
#ifdef ENABLE_CHEQUE_SYSTEM
		pkTab->cheque,
#endif
#ifdef ENABLE_GEM_SYSTEM
		pkTab->gem_point,
#endif
		pkTab->exp,
		pkTab->stat_point,
		pkTab->skill_point,
		pkTab->sub_skill_point,
		pkTab->stat_reset_count,
		pkTab->ip,
		pkTab->parts[PART_MAIN],
		pkTab->parts[PART_HAIR],
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		pkTab->parts[PART_ACCE],
#endif
#ifdef ENABLE_AURA_SYSTEM
		pkTab->parts[PART_AURA],
#endif
#ifdef ENABLE_HORSE_APPEARANCE
		pkTab->sHorse_appearance,
#endif
		pkTab->skill_group,
		pkTab->lAlignment,
		pkTab->horse.bLevel,
		pkTab->horse.bRiding,
		pkTab->horse.sHealth,
		pkTab->horse.dwHorseHealthDropTime,
		pkTab->horse.sStamina,
		pkTab->horse_skill_point
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		, pkTab->gem_next_refresh
#endif
#ifdef ENABLE_BATTLE_FIELD
		, pkTab->battle_point
		, pkTab->aiShopExUsablePoint[SHOP_COIN_TYPE_BATTLE_POINT]
		, pkTab->aiShopExDailyUse[SHOP_COIN_TYPE_BATTLE_POINT]
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
		, pkTab->medal_honor
		, pkTab->aiShopExUsablePoint[SHOP_COIN_TYPE_MEDAL_OF_HONOR]
		, pkTab->aiShopExDailyUse[SHOP_COIN_TYPE_MEDAL_OF_HONOR]
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		, pkTab->conqueror_st
		, pkTab->conqueror_ht
		, pkTab->conqueror_mov
		, pkTab->conqueror_imu
		, pkTab->conqueror_level
		, pkTab->conqueror_level_step
		, pkTab->conqueror_exp
		, pkTab->conqueror_point
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
		, pkTab->m_BiologActualMission
		, pkTab->m_BiologCollectedItems
		, pkTab->m_BiologCooldownReminder
		, pkTab->m_BiologCooldown
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		, pkTab->inventory_stage
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
		, pkTab->special_stage[0]
		, pkTab->special_stage[1]
		, pkTab->special_stage[2]
# endif
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		, pkTab->soul
		, pkTab->soulre
#endif
#ifdef ENABLE_FISH_EVENT
		, pkTab->fishEventUseCount
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		, pkTab->battle_pass_premium_id
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		, pkTab->page_equipment
		, pkTab->unlock_page_equipment
#endif
	);

	// Temporary space to change to binary
	static char text[8192 + 1];

	CDBManager::Instance().EscapeString(text, pkTab->skills, sizeof(pkTab->skills));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "skill_level = '%s', ", text);

	CDBManager::Instance().EscapeString(text, pkTab->quickslot, sizeof(pkTab->quickslot));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, "quickslot = '%s' ", text);

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	CDBManager::Instance().EscapeString(text, pkTab->gemItems, sizeof(pkTab->gemItems));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, ", gem_items = '%s' ", text);
#endif

#ifdef ENABLE_FISH_EVENT
	CDBManager::Instance().EscapeString(text, pkTab->fishSlots, sizeof(pkTab->fishSlots));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, ", fish_slots = '%s' ", text);
#endif

#ifdef ENABLE_FLOWER_EVENT
	CDBManager::Instance().EscapeString(text, pkTab->flowerEvent, sizeof(pkTab->flowerEvent));
	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, ", flower_event = '%s' ", text);
#endif

	queryLen += snprintf(pszQuery + queryLen, querySize - queryLen, " WHERE id=%d", pkTab->id);
	return queryLen;
}

CPlayerTableCache* CClientManager::GetPlayerCache(uint32_t id)
{
	const TPlayerTableCacheMap::iterator it = m_map_playerCache.find(id);

	if (it == m_map_playerCache.end())
		return nullptr;

	TPlayerTable* pTable = it->second->Get(false);
	pTable->logoff_interval = GetCurrentTime() - it->second->GetLastUpdateTime();
	return it->second;
}

void CClientManager::PutPlayerCache(TPlayerTable* pNew)
{
	CPlayerTableCache* c = GetPlayerCache(pNew->id);

	if (!c)
	{
		c = new CPlayerTableCache;
		m_map_playerCache.insert(TPlayerTableCacheMap::value_type(pNew->id, c));
	}

	if (g_bHotBackup)
		PlayerHB::Instance().Put(pNew->id);

	c->Put(pNew);
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CClientManager::QUERY_SKILL_COLOR_LOAD(CPeer* peer, uint32_t dwHandle, TPlayerLoadPacket* packet)
{
	CSKillColorCache* c;
	if (c = GetSkillColorCache(packet->player_id))
	{
		TSkillColor* p;
		p = c->Get();
		peer->EncodeHeader(HEADER_DG_SKILL_COLOR_LOAD, dwHandle, sizeof(p->dwSkillColor));
		peer->Encode(p->dwSkillColor, sizeof(p->dwSkillColor));
	}
	else
	{
		char szQuery[QUERY_MAX_LEN];
#ifdef ENABLE_NINETH_SKILL
		snprintf(szQuery, sizeof(szQuery), "SELECT "
			"s1_col1, s1_col2, s1_col3, s1_col4, s1_col5, "
			"s2_col1, s2_col2, s2_col3, s2_col4, s2_col5, "
			"s3_col1, s3_col2, s3_col3, s3_col4, s3_col5, "
			"s4_col1, s4_col2, s4_col3, s4_col4, s4_col5, "
			"s5_col1, s5_col2, s5_col3, s5_col4, s5_col5, "
			"s6_col1, s6_col2, s6_col3, s6_col4, s6_col5, " // end of normal skills
			"s7_col1, s7_col2, s7_col3, s7_col4, s7_col5, " // begin of new skills
			"s8_col1, s8_col2, s8_col3, s8_col4, s8_col5, "
			"s9_col1, s9_col2, s9_col3, s9_col4, s9_col5, " // end of new skills
			"s10_col1, s10_col2, s10_col3, s10_col4, s10_col5, " // begin of buffs
			"s11_col1, s11_col2, s11_col3, s11_col4, s11_col5, "
			"s12_col1, s12_col2, s12_col3, s12_col4, s12_col5, "
			"s13_col1, s13_col2, s13_col3, s13_col4, s13_col5, "
			"s14_col1, s14_col2, s14_col3, s14_col4, s14_col5, "
			"s15_col1, s15_col2, s15_col3, s15_col4, s15_col5 " // nine skill shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
			", s16_col1, s16_col2, s16_col3, s16_col4, s16_col5 "
#	endif
			"FROM skill_color%s WHERE player_id=%d",
			GetTablePostfix(), packet->player_id);
#else
		snprintf(szQuery, sizeof(szQuery), "SELECT "
			"s1_col1, s1_col2, s1_col3, s1_col4, s1_col5, "
			"s2_col1, s2_col2, s2_col3, s2_col4, s2_col5, "
			"s3_col1, s3_col2, s3_col3, s3_col4, s3_col5, "
			"s4_col1, s4_col2, s4_col3, s4_col4, s4_col5, "
			"s5_col1, s5_col2, s5_col3, s5_col4, s5_col5, "
			"s6_col1, s6_col2, s6_col3, s6_col4, s6_col5, " // end of skills
			"s7_col1, s7_col2, s7_col3, s7_col4, s7_col5, " // begin of buffs
			"s8_col1, s8_col2, s8_col3, s8_col4, s8_col5, "
			"s9_col1, s9_col2, s9_col3, s9_col4, s9_col5, "
			"s10_col1, s10_col2, s10_col3, s10_col4, s10_col5, "
			"s11_col1, s11_col2, s11_col3, s11_col4, s11_col5 "
			#ifdef ENABLE_WOLFMAN_CHARACTER
			", s12_col1, s12_col2, s12_col3, s12_col4, s12_col5 "
			#endif
			"FROM skill_color%s WHERE player_id=%d",
			GetTablePostfix(), packet->player_id);
#endif
		CDBManager::Instance().ReturnQuery(szQuery, QID_SKILL_COLOR, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));
	}
}
#endif

/*
* PLAYER LOAD
*/
void CClientManager::QUERY_PLAYER_LOAD(CPeer* peer, uint32_t dwHandle, const TPlayerLoadPacket* packet)
{
	CPlayerTableCache* c;
	TPlayerTable* pTab;

	//
	// Cache all characters belonging to one account
	//
	CLoginData* pLoginData = GetLoginDataByAID(packet->account_id);

	if (pLoginData)
	{
		for (int n = 0; n < PLAYER_PER_ACCOUNT; ++n)
		{
			if (pLoginData->GetAccountRef().players[n].dwID != 0)
				DeleteLogoutPlayer(pLoginData->GetAccountRef().players[n].dwID);
		}
	}

	//----------------------------------------------------------------
	// 1. User information exists in DBCache: In DBCache
	// 2. User information is not in DBCache: in DB
	// ---------------------------------------------------------------

	//----------------------------------
	// 1. User information exists in DBCache: In DBCache
	//----------------------------------
	if ((c = GetPlayerCache(packet->player_id)))
	{
		CLoginData* pkLD = GetLoginDataByAID(packet->account_id);

		if (!pkLD || pkLD->IsPlay())
		{
			sys_log(0, "PLAYER_LOAD_ERROR: LoginData %p IsPlay %d", pkLD, pkLD ? pkLD->IsPlay() : 0);
			peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, dwHandle, 0);
			return;
		}

		pTab = c->Get();

		pkLD->SetPlay(true);
		thecore_memcpy(pTab->aiPremiumTimes, pkLD->GetPremiumPtr(), sizeof(pTab->aiPremiumTimes));

		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, dwHandle, sizeof(TPlayerTable));
		peer->Encode(pTab, sizeof(TPlayerTable));

		if (packet->player_id != pkLD->GetLastPlayerID())
		{
			TPacketNeedLoginLogInfo logInfo{};
			logInfo.dwPlayerID = packet->player_id;

			pkLD->SetLastPlayerID(packet->player_id);

			peer->EncodeHeader(HEADER_DG_NEED_LOGIN_LOG, dwHandle, sizeof(TPacketNeedLoginLogInfo));
			peer->Encode(&logInfo, sizeof(TPacketNeedLoginLogInfo));
		}

		char szQuery[1024] = { 0, };

		TItemCacheSet* pSet = GetItemCacheSet(pTab->id);

#ifdef ENABLE_CHEQUE_SYSTEM
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %d cheque %d ", pTab->name, pTab->id, pTab->gold, pTab->cheque);
#else
		sys_log(0, "[PLAYER_LOAD] ID %s pid %d gold %d ", pTab->name, pTab->id, pTab->gold);
#endif

		//--------------------------------------------
		// Item & AFFECT & QUEST loading:
		//--------------------------------------------
		// 1) Item exists in DBCache: fetched from DBCache
		// 2) Item not found in DBCache: fetched from DB

		/////////////////////////////////////////////
		// 1) Item exists in DBCache: fetched from DBCache
		/////////////////////////////////////////////
		if (pSet)
		{
			static std::vector<TPlayerItem> s_items;
			s_items.resize(pSet->size());

			uint32_t dwCount = 0;
			TItemCacheSet::iterator it = pSet->begin();

			while (it != pSet->end())
			{
				CItemCache* c2 = *it++;
				const TPlayerItem* p = c2->Get();

				if (p->vnum) // If there is no vnum, it is a deleted item.
					thecore_memcpy(&s_items[dwCount++], p, sizeof(TPlayerItem));
			}

			if (g_test_server)
				sys_log(0, "ITEM_CACHE: HIT! %s count: %u", pTab->name, dwCount);

			peer->EncodeHeader(HEADER_DG_ITEM_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TPlayerItem) * dwCount);
			peer->EncodeDWORD(dwCount);

			if (dwCount)
				peer->Encode(&s_items[0], sizeof(TPlayerItem) * dwCount);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			// Private shop item pricing (if it exists)
			SendMyShopInfo(peer, dwHandle, pTab->id);
#endif

			// Quest
			snprintf(szQuery, sizeof(szQuery),
				"SELECT dwPID,szName,szState,lValue FROM quest%s WHERE dwPID=%d AND lValue<>0",
				GetTablePostfix(), pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery, QID_QUEST, peer->GetHandle(), new ClientHandleInfo(dwHandle, 0, packet->account_id));

			// Affect
			snprintf(szQuery, sizeof(szQuery),
				"SELECT dwPID,bType,bApplyOn,lApplyValue,dwFlag,lDuration,lSPCost"
#ifdef ENABLE_AFFECT_RENEWAL
				",UNIX_TIMESTAMP(expireTime)"
#endif
				" FROM affect%s WHERE dwPID=%d",
				GetTablePostfix(), pTab->id);
			// @fixme402 ClientHandleInfo+pTab->id
			CDBManager::Instance().ReturnQuery(szQuery, QID_AFFECT, peer->GetHandle(), new ClientHandleInfo(dwHandle, pTab->id));
		}
		/////////////////////////////////////////////
		// 2) Item not found in DBCache: fetched from DB
		/////////////////////////////////////////////
		else
		{
			//Items
			snprintf(szQuery, sizeof(szQuery),
				"SELECT id,window+0,pos,count,vnum,"
#ifdef ENABLE_SEALBIND_SYSTEM
				"seal_date,"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				"transmutation,"
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
				"is_basic,"
#endif
#ifdef ENABLE_REFINE_ELEMENT
				"grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3,"
#endif
				"socket0,socket1,socket2,"
#ifdef ENABLE_PROTO_RENEWAL
				"socket3,socket4,socket5,"
#endif
				"attrtype0,attrvalue0,"
				"attrtype1,attrvalue1,"
				"attrtype2,attrvalue2,"
				"attrtype3,attrvalue3,"
				"attrtype4,attrvalue4,"
				"attrtype5,attrvalue5,"
				"attrtype6,attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
				", randomtype0, randomvalue0, randomrow0"
				", randomtype1, randomvalue1, randomrow1"
				", randomtype2, randomvalue2, randomrow2"
				", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_SET_ITEM
				", setItem"
#endif

				//"WHERE owner_id=%d AND (`window` < %d or window = %d or window = %d) ",
				" FROM item%s WHERE owner_id=%d AND (window in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'"
#ifdef ENABLE_SWITCHBOT
				",'SWITCHBOT'"
#endif
#ifdef ENABLE_ATTR_6TH_7TH
				",'NPC_STORAGE'"
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
				",'PREMIUM_PRIVATE_SHOP'"
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
				",'ADDITIONAL_EQUIPMENT_1'"
#endif
				"))",
				GetTablePostfix(), pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery,
				QID_ITEM,
				peer->GetHandle(),
				new ClientHandleInfo(dwHandle, pTab->id));

			//Quest
			snprintf(szQuery, sizeof(szQuery),
				"SELECT dwPID, szName, szState, lValue FROM quest%s WHERE dwPID=%d",
				GetTablePostfix(), pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery,
				QID_QUEST,
				peer->GetHandle(),
				new ClientHandleInfo(dwHandle, pTab->id));

			//Affect
			snprintf(szQuery, sizeof(szQuery),
				"SELECT dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost"
#ifdef ENABLE_AFFECT_RENEWAL
				", UNIX_TIMESTAMP(expireTime)"
#endif
				" FROM affect%s WHERE dwPID=%d",
				GetTablePostfix(), pTab->id);

			CDBManager::Instance().ReturnQuery(szQuery,
				QID_AFFECT,
				peer->GetHandle(),
				new ClientHandleInfo(dwHandle, pTab->id));
		}
		//ljw
		//return;
	}
	//----------------------------------
	// 2. User information is not in DBCache: In DB
	//----------------------------------
	else
	{
		sys_log(0, "[PLAYER_LOAD] Load from PlayerDB pid[%d]", packet->player_id);

		char queryStr[QUERY_MAX_LEN];

		//--------------------------------------------------------------
		// Getting character information: unconditionally from DB
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
			"SELECT "
			"id,name,job,voice,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,random_hp,random_sp,"
			"playtime, gold,"
#ifdef ENABLE_CHEQUE_SYSTEM
			"cheque,"
#endif
#ifdef ENABLE_GEM_SYSTEM
			"gem,"
#endif
			"level,level_step,st,ht,dx,iq,exp,"
			"stat_point,skill_point,sub_skill_point,stat_reset_count,part_base,part_hair,"
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			"part_acce, "
#endif
#ifdef ENABLE_AURA_SYSTEM
			"part_aura, "
#endif
#ifdef ENABLE_HORSE_APPEARANCE
			"horse_appearance,"
#endif
			"skill_level,quickslot,skill_group,alignment,"
			"mobile,horse_level,horse_riding,horse_hp,horse_hp_droptime,horse_stamina,"
			"UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_play),horse_skill_point"
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
			",gem_items,gem_next_refresh"
#endif
#ifdef ENABLE_BATTLE_FIELD
			", battle_point "
			", usable_battle_point "
			", daily_time_battle_point "
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
			", medal_honor "
			", usable_medal_honor "
			", daily_time_medal_honor "
#endif
#ifdef ENABLE_YOHARA_SYSTEM
			", conqueror_st "
			", conqueror_ht "
			", conqueror_mov "
			", conqueror_imu "
			", conqueror_level "
			", conqueror_level_step "
			", conqueror_exp "
			", conqueror_point "
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
			", biolog_mission"
			", biolog_collected"
			", biolog_cooldown_reminder"
			", biolog_cooldown"
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
			", inventory_stage"
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
			", special_stage_1"
			", special_stage_2"
			", special_stage_3"
# endif
#endif
#ifdef NEW_SELECT_CHARACTER
			", last_play "
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
			", soul"
			", soulre"
#endif
#ifdef ENABLE_FISH_EVENT
			", fish_use_count"
			", fish_slots"
#endif
#ifdef ENABLE_FLOWER_EVENT
			", flower_event"
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
			", battle_pass_premium_id"
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			", page_equipment"
			", unlock_page_equipment"
#endif
			" FROM player%s WHERE id=%d",
			GetTablePostfix(), packet->player_id);

		ClientHandleInfo* pkInfo = new ClientHandleInfo(dwHandle, packet->player_id);
		pkInfo->account_id = packet->account_id;
		CDBManager::Instance().ReturnQuery(queryStr, QID_PLAYER, peer->GetHandle(), pkInfo);

		//--------------------------------------------------------------
		// Load up items
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
			"SELECT id,`window`+0,pos,count,vnum,"
#ifdef ENABLE_SEALBIND_SYSTEM
			"seal_date,"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			"transmutation,"
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			"is_basic,"
#endif
#ifdef ENABLE_REFINE_ELEMENT
			"grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3,"
#endif
			"socket0,socket1,socket2,"
#ifdef ENABLE_PROTO_RENEWAL
			"socket3,socket4,socket5,"
#endif
			"attrtype0,attrvalue0,"
			"attrtype1,attrvalue1,"
			"attrtype2,attrvalue2,"
			"attrtype3,attrvalue3,"
			"attrtype4,attrvalue4,"
			"attrtype5,attrvalue5,"
			"attrtype6,attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
			", randomtype0, randomvalue0, randomrow0"
			", randomtype1, randomvalue1, randomrow1"
			", randomtype2, randomvalue2, randomrow2"
			", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_SET_ITEM
			", setItem"
#endif
			//"WHERE owner_id=%d AND (window < %d or window = %d or window = %d) ",
			" FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'"
#ifdef ENABLE_SWITCHBOT
			",'SWITCHBOT'"
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			",'NPC_STORAGE'"
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			",'PREMIUM_PRIVATE_SHOP'"
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			",'ADDITIONAL_EQUIPMENT_1'"
#endif
			"))",
			GetTablePostfix(), packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_ITEM, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));

		//--------------------------------------------------------------
		// Get QUEST
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
			"SELECT dwPID,szName,szState,lValue FROM quest%s WHERE dwPID=%d",
			GetTablePostfix(), packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_QUEST, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id, packet->account_id));
		//In the German futures function, the account id is also passed to get login information from the item_award table.
		//--------------------------------------------------------------
		// Get AFFECT
		//--------------------------------------------------------------
		snprintf(queryStr, sizeof(queryStr),
			"SELECT dwPID,bType,bApplyOn,lApplyValue,dwFlag,lDuration,lSPCost"
#ifdef ENABLE_AFFECT_RENEWAL
			",UNIX_TIMESTAMP(expireTime)"
#endif
			" FROM affect%s WHERE dwPID=%d",
			GetTablePostfix(), packet->player_id);
		CDBManager::Instance().ReturnQuery(queryStr, QID_AFFECT, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//Spawn a shop on the game if needed
	PlayerLoginEvent(packet->player_id);
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	// Load all missions from table
	char queryStrBP[QUERY_MAX_LEN];
	snprintf(queryStrBP, sizeof(queryStrBP),
		"SELECT player_id, battlepass_type+0, mission_index, mission_type+0, battle_pass_id, extra_info, completed FROM battlepass_missions WHERE player_id = %d", packet->player_id);
	CDBManager::instance().ReturnQuery(queryStrBP, QID_EXT_BATTLE_PASS, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id));
#endif
}

void CClientManager::ItemAward(CPeer* peer, const char* login)
{
	char login_t[LOGIN_MAX_LEN + 1] = "";
	strlcpy(login_t, login, LOGIN_MAX_LEN + 1);
	std::set<TItemAward*>* pSet = ItemAwardManager::Instance().GetByLogin(login_t);
	if (pSet == nullptr)
		return;
	typeof(pSet->begin()) it = pSet->begin(); //Reads those whose taken_time is nullptr
	while (it != pSet->end())
	{
		TItemAward* pItemAward = *(it++);
		const char* whyStr = pItemAward->szWhy; //why colum read
		char cmdStr[100] = ""; //Copy the value read from the why Colum into a temporary string
		strcpy(cmdStr, whyStr); //If you write a token in the process of obtaining the command, the original is also tokenized.
		char command[20] = "";
		// @fixme203 directly GetCommand instead of strcpy
		GetCommand(cmdStr, command); // Get command
		if (!(strcmp(command, "GIFT"))) // If command is GIFT
		{
			TPacketItemAwardInfromer giftData{};
			strcpy(giftData.login, pItemAward->szLogin); //Login ID Copy
			strcpy(giftData.command, command); //Command copy
			giftData.vnum = pItemAward->dwVnum; //Copy item vnum too
			ForwardPacket(HEADER_DG_ITEMAWARD_INFORMER, &giftData, sizeof(TPacketItemAwardInfromer));
		}
	}
}

char* CClientManager::GetCommand(char* str, char* command) noexcept // @fixme203
{
	char* tok;

	if (str[0] == '[')
	{
		tok = strtok(str, "]");
		strcat(command, &tok[1]);
	}

	return command;
}

bool CreatePlayerTableFromRes(MYSQL_RES* res, TPlayerTable* pkTab) noexcept
{
	if (mysql_num_rows(res) == 0) // no data
		return false;

	memset(pkTab, 0, sizeof(TPlayerTable));

	MYSQL_ROW row = mysql_fetch_row(res);
	const auto row_lenght = mysql_fetch_lengths(res);

	int col = 0;

	// "id,name,job,voice,dir,x,y,z,map_index,exit_x,exit_y,exit_map_index,hp,mp,stamina,random_hp,random_sp,playtime,"
	// "gold,level,level_step,st,ht,dx,iq,exp,"
	// "stat_point,skill_point,sub_skill_point,stat_reset_count,part_base,part_hair,"
	// "skill_level,quickslot,skill_group,alignment,mobile,horse_level,horse_riding,horse_hp,horse_stamina FROM player%s WHERE id=%d",
	str_to_number(pkTab->id, row[col++]);
	strlcpy(pkTab->name, row[col++], sizeof(pkTab->name));
	str_to_number(pkTab->job, row[col++]);
	str_to_number(pkTab->voice, row[col++]);
	str_to_number(pkTab->dir, row[col++]);
	str_to_number(pkTab->x, row[col++]);
	str_to_number(pkTab->y, row[col++]);
	str_to_number(pkTab->z, row[col++]);
	str_to_number(pkTab->lMapIndex, row[col++]);
	str_to_number(pkTab->lExitX, row[col++]);
	str_to_number(pkTab->lExitY, row[col++]);
	str_to_number(pkTab->lExitMapIndex, row[col++]);
	str_to_number(pkTab->hp, row[col++]);
	str_to_number(pkTab->sp, row[col++]);
	str_to_number(pkTab->stamina, row[col++]);
	str_to_number(pkTab->sRandomHP, row[col++]);
	str_to_number(pkTab->sRandomSP, row[col++]);
	str_to_number(pkTab->playtime, row[col++]);
	str_to_number(pkTab->gold, row[col++]);
#ifdef ENABLE_CHEQUE_SYSTEM
	str_to_number(pkTab->cheque, row[col++]);
#endif
#ifdef ENABLE_GEM_SYSTEM
	str_to_number(pkTab->gem_point, row[col++]);
#endif
	str_to_number(pkTab->level, row[col++]);
	str_to_number(pkTab->level_step, row[col++]);
	str_to_number(pkTab->st, row[col++]);
	str_to_number(pkTab->ht, row[col++]);
	str_to_number(pkTab->dx, row[col++]);
	str_to_number(pkTab->iq, row[col++]);
	str_to_number(pkTab->exp, row[col++]);
	str_to_number(pkTab->stat_point, row[col++]);
	str_to_number(pkTab->skill_point, row[col++]);
	str_to_number(pkTab->sub_skill_point, row[col++]);
	str_to_number(pkTab->stat_reset_count, row[col++]);
	str_to_number(pkTab->part_base, row[col++]);
	str_to_number(pkTab->parts[PART_HAIR], row[col++]);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	str_to_number(pkTab->parts[PART_ACCE], row[col++]);
#endif
#ifdef ENABLE_AURA_SYSTEM
	str_to_number(pkTab->parts[PART_AURA], row[col++]);
#endif
#ifdef ENABLE_HORSE_APPEARANCE
	str_to_number(pkTab->sHorse_appearance, row[col++]);
#endif

	if (row[col])
		thecore_memcpy(pkTab->skills, row[col], sizeof(pkTab->skills));
	else
		memset(&pkTab->skills, 0, sizeof(pkTab->skills));

	col++;

	if (row[col])
		thecore_memcpy(pkTab->quickslot, row[col], sizeof(pkTab->quickslot));
	else
		memset(pkTab->quickslot, 0, sizeof(pkTab->quickslot));

	col++;

	str_to_number(pkTab->skill_group, row[col++]);
	str_to_number(pkTab->lAlignment, row[col++]);

	if (row[col])
	{
		strlcpy(pkTab->szMobile, row[col], sizeof(pkTab->szMobile));
	}

	col++;

	str_to_number(pkTab->horse.bLevel, row[col++]);
	str_to_number(pkTab->horse.bRiding, row[col++]);
	str_to_number(pkTab->horse.sHealth, row[col++]);
	str_to_number(pkTab->horse.dwHorseHealthDropTime, row[col++]);
	str_to_number(pkTab->horse.sStamina, row[col++]);
	str_to_number(pkTab->logoff_interval, row[col++]);
	str_to_number(pkTab->horse_skill_point, row[col++]);
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	if (row_lenght[col])
		thecore_memcpy(pkTab->gemItems, row[col], sizeof(pkTab->gemItems));
	else
		memset(pkTab->gemItems, 0, sizeof(pkTab->gemItems));
	col++;

	str_to_number(pkTab->gem_next_refresh, row[col++]);
#endif
#ifdef ENABLE_BATTLE_FIELD
	str_to_number(pkTab->battle_point, row[col++]);
	str_to_number(pkTab->aiShopExUsablePoint[SHOP_COIN_TYPE_BATTLE_POINT], row[col++]);
	str_to_number(pkTab->aiShopExDailyUse[SHOP_COIN_TYPE_BATTLE_POINT], row[col++]);
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	str_to_number(pkTab->medal_honor, row[col++]);
	str_to_number(pkTab->aiShopExUsablePoint[SHOP_COIN_TYPE_MEDAL_OF_HONOR], row[col++]);
	str_to_number(pkTab->aiShopExDailyUse[SHOP_COIN_TYPE_MEDAL_OF_HONOR], row[col++]);
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	str_to_number(pkTab->conqueror_st, row[col++]);
	str_to_number(pkTab->conqueror_ht, row[col++]);
	str_to_number(pkTab->conqueror_mov, row[col++]);
	str_to_number(pkTab->conqueror_imu, row[col++]);
	str_to_number(pkTab->conqueror_level, row[col++]);
	str_to_number(pkTab->conqueror_level_step, row[col++]);
	str_to_number(pkTab->conqueror_exp, row[col++]);
	str_to_number(pkTab->conqueror_point, row[col++]);
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	str_to_number(pkTab->m_BiologActualMission, row[col++]);
	str_to_number(pkTab->m_BiologCollectedItems, row[col++]);
	str_to_number(pkTab->m_BiologCooldownReminder, row[col++]);
	str_to_number(pkTab->m_BiologCooldown, row[col++]);
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	str_to_number(pkTab->inventory_stage, row[col++]);
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	str_to_number(pkTab->special_stage[0], row[col++]);
	str_to_number(pkTab->special_stage[1], row[col++]);
	str_to_number(pkTab->special_stage[2], row[col++]);
# endif
#endif
#ifdef NEW_SELECT_CHARACTER
	str_to_number(pkTab->last_playtime, row[col++]);
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	str_to_number(pkTab->soul, row[col++]);
	str_to_number(pkTab->soulre, row[col++]);
#endif
#ifdef ENABLE_FISH_EVENT
	str_to_number(pkTab->fishEventUseCount, row[col++]);

	if (row_lenght[col])
		thecore_memcpy(pkTab->fishSlots, row[col], sizeof(pkTab->fishSlots));
	else
		memset(pkTab->fishSlots, 0, sizeof(pkTab->fishSlots));

	col++;
#endif
#ifdef ENABLE_FLOWER_EVENT
	if (row_lenght[col])
		thecore_memcpy(pkTab->flowerEvent, row[col], sizeof(pkTab->flowerEvent));
	else
		memset(pkTab->flowerEvent, 0, sizeof(pkTab->flowerEvent));
	col++;
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	str_to_number(pkTab->battle_pass_premium_id, row[col++]);
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	str_to_number(pkTab->page_equipment, row[col++]);
	str_to_number(pkTab->unlock_page_equipment, row[col++]);
#endif

	// reset sub_skill_point
	{
#ifdef ENABLE_REVERSED_FUNCTIONS
		/* 130 == SKILL_HORSE*/
		if (pkTab->skills[130].bMasterType != 0)
			pkTab->skills[130].bMasterType = SKILL_NORMAL;
#endif
		pkTab->skills[123].bLevel = 0; // SKILL_CREATE

		if (pkTab->level > 9)
		{
			const int max_point = pkTab->level - 9;

			const int skill_point =
				MIN(20, pkTab->skills[121].bLevel) +	// SKILL_LEADERSHIP
				MIN(20, pkTab->skills[124].bLevel) +	// SKILL_MINING
				MIN(10, pkTab->skills[131].bLevel) +	// SKILL_HORSE_SUMMON
				MIN(20, pkTab->skills[141].bLevel) +	// SKILL_ADD_HP
				MIN(20, pkTab->skills[142].bLevel);		// SKILL_RESIST_PENETRATE

			pkTab->sub_skill_point = max_point - skill_point;
		}
		else
			pkTab->sub_skill_point = 0;
	}

	return true;
}

void CClientManager::RESULT_COMPOSITE_PLAYER(CPeer* peer, SQLMsg* pMsg, uint32_t dwQID)
{
	if (!pMsg)
		return;

	CQueryInfo* qi = (CQueryInfo*)pMsg->pvUserData;
	std::unique_ptr<ClientHandleInfo> info((ClientHandleInfo*)qi->pvData);

	MYSQL_RES* pSQLResult = pMsg->Get()->pSQLResult;
	if (!pSQLResult)
	{
		sys_err("null MYSQL_RES QID %u", dwQID);
		return;
	}

	switch (dwQID)
	{
		case QID_PLAYER:
			sys_log(0, "QID_PLAYER %u %u", info->dwHandle, info->player_id);
			RESULT_PLAYER_LOAD(peer, pSQLResult, info.get());
			break;

		case QID_ITEM:
			sys_log(0, "QID_ITEM %u", info->dwHandle);
			RESULT_ITEM_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			break;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
		case QID_SKILL_COLOR:
			sys_log(0, "QID_SKILL_COLOR %u %u", info->dwHandle, info->player_id);
			RESULT_SKILL_COLOR_LOAD(peer, pSQLResult, info->dwHandle);
			break;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case QID_EXT_BATTLE_PASS:
			sys_log(0, "QID_EXT_BATTLE_PASS %u", info->dwHandle);
			RESULT_EXT_BATTLE_PASS_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			break;
#endif

		case QID_QUEST:
		{
			sys_log(0, "QID_QUEST %u", info->dwHandle);
			RESULT_QUEST_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			//get aid
			const ClientHandleInfo* temp1 = info.get();
			if (temp1 == nullptr)
				break;

			CLoginData* pLoginData1 = GetLoginDataByAID(temp1->account_id); //
			//Germany futures function
			if (pLoginData1->GetAccountRef().login == nullptr)
				break;
			if (pLoginData1 == nullptr)
				break;
			sys_log(0, "info of pLoginData1 before call ItemAwardfunction %d", pLoginData1);
			ItemAward(peer, pLoginData1->GetAccountRef().login);
		}
		break;

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case QID_ACHIEVEMENT_LOAD_DATA:
		{
			RESULT_ACHIEVEMENT_LOAD(peer, pSQLResult, info->dwHandle, info->player_id, info->pAchievementTable);
		}
		break;
		case QID_ACHIEVEMENT_LOAD_ACHIEVEMENTS:
		{
			RESULT_ACHIEVEMENT_LOAD_ACHIEVEMENTS(peer, pSQLResult, info->dwHandle, info->player_id, info->pAchievementTable);
		}
		break;
		case QID_ACHIEVEMENT_LOAD_TASKS:
		{
			RESULT_ACHIEVEMENT_LOAD_TASKS(peer, pSQLResult, info->dwHandle, info->player_id, info->pAchievementTable);
		}
		break;
#endif

		case QID_AFFECT:
			sys_log(0, "QID_AFFECT %u", info->dwHandle);
			// @fixme402 RESULT_AFFECT_LOAD+info->player_id
			RESULT_AFFECT_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);
			break;

/*		case QID_PLAYER_ITEM_QUEST_AFFECT:
			sys_log(0, "QID_PLAYER_ITEM_QUEST_AFFECT %u", info->dwHandle);
			RESULT_PLAYER_LOAD(peer, pSQLResult, info->dwHandle);

			if (!pMsg->Next())
			{
				sys_err("RESULT_COMPOSITE_PLAYER: QID_PLAYER_ITEM_QUEST_AFFECT: ITEM FAILED");
				return;
			}

		case QID_ITEM_QUEST_AFFECT:
			sys_log(0, "QID_ITEM_QUEST_AFFECT %u", info->dwHandle);
			RESULT_ITEM_LOAD(peer, pSQLResult, info->dwHandle, info->player_id);

			if (!pMsg->Next())
			{
				sys_err("RESULT_COMPOSITE_PLAYER: QID_PLAYER_ITEM_QUEST_AFFECT: QUEST FAILED");
				return;
			}

		case QID_QUEST_AFFECT:
			sys_log(0, "QID_QUEST_AFFECT %u", info->dwHandle);
			RESULT_QUEST_LOAD(peer, pSQLResult, info->dwHandle);

			if (!pMsg->Next())
				sys_err("RESULT_COMPOSITE_PLAYER: QID_PLAYER_ITEM_QUEST_AFFECT: AFFECT FAILED");
			else
				RESULT_AFFECT_LOAD(peer, pSQLResult, info->dwHandle);
			break;*/

		default:
			break;
	}
}

void CClientManager::RESULT_PLAYER_LOAD(CPeer* peer, MYSQL_RES* pRes, const ClientHandleInfo* pkInfo)
{
	if (!peer)
		return;

	TPlayerTable tab;

	if (!CreatePlayerTableFromRes(pRes, &tab))
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, pkInfo->dwHandle, 0);
		return;
	}

	CLoginData* pkLD = GetLoginDataByAID(pkInfo->account_id);

	if (!pkLD || pkLD->IsPlay())
	{
		sys_log(0, "PLAYER_LOAD_ERROR: LoginData %p IsPlay %d", pkLD, pkLD ? pkLD->IsPlay() : 0);
		peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_FAILED, pkInfo->dwHandle, 0);
		return;
	}

	pkLD->SetPlay(true);
	thecore_memcpy(tab.aiPremiumTimes, pkLD->GetPremiumPtr(), sizeof(tab.aiPremiumTimes));

	peer->EncodeHeader(HEADER_DG_PLAYER_LOAD_SUCCESS, pkInfo->dwHandle, sizeof(TPlayerTable));
	peer->Encode(&tab, sizeof(TPlayerTable));

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	PlayerLoginEvent(tab.id);
#endif

	if (tab.id != pkLD->GetLastPlayerID())
	{
		TPacketNeedLoginLogInfo logInfo{};
		logInfo.dwPlayerID = tab.id;

		pkLD->SetLastPlayerID(tab.id);

		peer->EncodeHeader(HEADER_DG_NEED_LOGIN_LOG, pkInfo->dwHandle, sizeof(TPacketNeedLoginLogInfo));
		peer->Encode(&logInfo, sizeof(TPacketNeedLoginLogInfo));
	}
}

void CClientManager::RESULT_ITEM_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwPID)
{
	if (!peer)
		return;

	static std::vector<TPlayerItem> s_items;
	// Read item information from DB.
	CreateItemTableFromRes(pRes, &s_items, dwPID);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	uint32_t dwShopCount = 0;
	for (auto it = s_items.begin(); it != s_items.end();)
	{
		const TPlayerItem item = *it;
		if (item.window == PREMIUM_PRIVATE_SHOP)
		{
			CItemCache* c = GetItemCache(item.id);
			if (c && c->Get() && c->Get()->owner != dwPID) //Shop items that we've sold are not loaded
			{
				it = s_items.erase(it);
				continue;
			}

			++dwShopCount;
		}

		++it;
	}
#endif

	const uint32_t dwCount = s_items.size();

	peer->EncodeHeader(HEADER_DG_ITEM_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TPlayerItem) * dwCount);
	peer->EncodeDWORD(dwCount);

	// Create a CacheSet
	CreateItemCacheSet(dwPID);

	// ITEM_LOAD_LOG_ATTACH_PID
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	sys_log(0, "ITEM_LOAD: count %u, shop %u, pid %u", dwCount, dwShopCount, dwPID);
#else
	sys_log(0, "ITEM_LOAD: count %u pid %u", dwCount, dwPID);
#endif
	// END_OF_ITEM_LOAD_LOG_ATTACH_PID

	if (dwCount)
	{
#ifdef ENABLE_GROWTH_PET_SYSTEM
		//
		//for (auto petItem : s_items)
		for (uint32_t i = 0; i < dwCount; ++i)
		{
			const auto it = m_map_itemTableByVnum.find(s_items[i].vnum);

			if (it == m_map_itemTableByVnum.end())
				continue;

			if (it->second->bType == ITEM_PET && ((it->second->bSubType == PET_UPBRINGING) || (it->second->bSubType == PET_BAG)))
			{
				char szGrowthPetQuery[QUERY_MAX_LEN], szGrowthPetColumns[QUERY_MAX_LEN];

				snprintf(szGrowthPetColumns, sizeof(szGrowthPetColumns),
					"id, pet_vnum, pet_nick, pet_level, evol_level, pet_type, pet_hp, pet_def, pet_sp, "
					"pet_duration, pet_birthday, exp_monster, exp_item, skill_count, "
					"pet_skill1, pet_skill_level1, pet_skill_spec1, pet_skill_cool1, "
					"pet_skill2, pet_skill_level2, pet_skill_spec2, pet_skill_cool2, "
					"pet_skill3, pet_skill_level3, pet_skill_spec3, pet_skill_cool3");

				snprintf(szGrowthPetQuery, sizeof(szGrowthPetQuery),
					"SELECT %s FROM pet%s WHERE id=%lu",
					szGrowthPetColumns, GetTablePostfix(), s_items[i].alSockets[2]);

				auto msgGrowthPet(CDBManager::Instance().DirectQuery(szGrowthPetQuery));
				const SQLResult* resGrowthPet = msgGrowthPet->Get();

				if (resGrowthPet && resGrowthPet->uiNumRows > 0)
				{
					for (size_t k = 0; k < resGrowthPet->uiNumRows; ++k)
					{
						const MYSQL_ROW row = mysql_fetch_row(resGrowthPet->pSQLResult);
						if (!row || !row[0])
							continue;

						int col = 0;

						TGrowthPetInfo table = {};
						str_to_number(table.pet_id, row[col++]);
						str_to_number(table.pet_vnum, row[col++]);
						strlcpy(table.pet_nick, row[col++], sizeof(table.pet_nick));
						str_to_number(table.pet_level, row[col++]);
						str_to_number(table.evol_level, row[col++]);
						str_to_number(table.pet_type, row[col++]);
						str_to_number(table.pet_hp, row[col++]);
						str_to_number(table.pet_def, row[col++]);
						str_to_number(table.pet_sp, row[col++]);
						str_to_number(table.pet_max_time, row[col++]);
						str_to_number(table.pet_birthday, row[col++]);
						str_to_number(table.exp_monster, row[col++]);
						str_to_number(table.exp_item, row[col++]);
						str_to_number(table.skill_count, row[col++]);
						str_to_number(table.skill_vnum[0], row[col++]);
						str_to_number(table.skill_level[0], row[col++]);
						str_to_number(table.skill_spec[0], row[col++]);
						str_to_number(table.skill_cool[0], row[col++]);
						str_to_number(table.skill_vnum[1], row[col++]);
						str_to_number(table.skill_level[1], row[col++]);
						str_to_number(table.skill_spec[1], row[col++]);
						str_to_number(table.skill_cool[1], row[col++]);
						str_to_number(table.skill_vnum[2], row[col++]);
						str_to_number(table.skill_level[2], row[col++]);
						str_to_number(table.skill_spec[2], row[col++]);
						str_to_number(table.skill_cool[2], row[col++]);

						if (g_test_server)
							sys_err("RESULT_PET_ITEM_LOAD: %d %d %d", table.pet_id, table.exp_monster, table.exp_item);

						thecore_memcpy(&s_items[i].aPetInfo, &table, sizeof(TGrowthPetInfo));
					}
				}
			}
		}
		//
#endif

		peer->Encode(&s_items[0], sizeof(TPlayerItem) * dwCount);

		for (uint32_t i = 0; i < dwCount; ++i)
			PutItemCache(&s_items[i], true); // Since there is no need to save the loaded one, put true in the argument bSkipQuery.
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//Send the shop information (if any)
	SendMyShopInfo(peer, dwHandle, dwPID);
#endif
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CClientManager::RESULT_SKILL_COLOR_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle)
{
	if (!peer)
		return;

	uint32_t dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
	memset(dwSkillColor, 0, sizeof(dwSkillColor));

	CreateSkillColorTableFromRes(pRes, *dwSkillColor);
	sys_log(0, "SKILL_COLOR_LOAD %i, %i, %i, %i,", dwSkillColor[0][0], dwSkillColor[1][1], dwSkillColor[2][2], dwSkillColor[3][3]);
	peer->EncodeHeader(HEADER_DG_SKILL_COLOR_LOAD, dwHandle, sizeof(dwSkillColor));
	peer->Encode(&dwSkillColor, sizeof(dwSkillColor));
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
void CClientManager::RESULT_EXT_BATTLE_PASS_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwRealPID)
{
	int iNumRows;

	if ((iNumRows = mysql_num_rows(pRes)) == 0)
	{
		uint32_t dwCount = 0;
		TPlayerExtBattlePassMission pbpTable = { 0 };

		sys_log(0, "EXT_BATTLE_PASS_LOAD: count %u PID %u", dwCount, dwRealPID);

		peer->EncodeHeader(HEADER_DG_EXT_BATTLE_PASS_LOAD, dwHandle, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(TPlayerExtBattlePassMission) * dwCount);
		peer->Encode(&dwRealPID, sizeof(uint32_t));
		peer->Encode(&dwCount, sizeof(uint32_t));
		peer->Encode(&pbpTable, sizeof(TPlayerExtBattlePassMission) * dwCount);
		return;
	}

	static std::vector<TPlayerExtBattlePassMission> s_mission;
	s_mission.resize(iNumRows);

	MYSQL_ROW row;

	for (int i = 0; i < iNumRows; ++i)
	{
		int col = 0;
		TPlayerExtBattlePassMission& r = s_mission[i];
		row = mysql_fetch_row(pRes);

		str_to_number(r.dwPlayerId, row[col++]);
		str_to_number(r.dwBattlePassType, row[col++]);
		str_to_number(r.dwMissionIndex, row[col++]);
		str_to_number(r.dwMissionType, row[col++]);
		str_to_number(r.dwBattlePassId, row[col++]);
		str_to_number(r.dwExtraInfo, row[col++]);
		str_to_number(r.bCompleted, row[col++]);

		r.bIsUpdated = 0;
	}

	sys_log(0, "EXT_BATTLE_PASS_LOAD: count %d PID %u", s_mission.size(), dwRealPID);

	uint32_t dwCount = s_mission.size();

	peer->EncodeHeader(HEADER_DG_EXT_BATTLE_PASS_LOAD, dwHandle, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(TPlayerExtBattlePassMission) * dwCount);
	peer->Encode(&dwRealPID, sizeof(uint32_t));
	peer->Encode(&dwCount, sizeof(uint32_t));
	peer->Encode(&s_mission[0], sizeof(TPlayerExtBattlePassMission) * dwCount);
}

void CClientManager::QUERY_SAVE_EXT_BATTLE_PASS(CPeer* peer, uint32_t dwHandle, TPlayerExtBattlePassMission* battlePass)
{
	if (g_test_server)
		sys_log(0, "QUERY_SAVE_EXT_BATTLE_PASS: %lu", battlePass->dwPlayerId);

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"REPLACE INTO battlepass_missions (player_id, battlepass_type, mission_index, mission_type, battle_pass_id, extra_info, completed) VALUES (%lu, %d, %d, %d, %d, %d, %d)",
		battlePass->dwPlayerId,
		battlePass->dwBattlePassType,
		battlePass->dwMissionIndex,
		battlePass->dwMissionType,
		battlePass->dwBattlePassId,
		battlePass->dwExtraInfo,
		battlePass->bCompleted ? 1 : 0);
	CDBManager::instance().AsyncQuery(szQuery);
}
#endif

// @fixme402 (RESULT_AFFECT_LOAD +dwRealPID)
void CClientManager::RESULT_AFFECT_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwRealPID)
{
	if (!peer)
		return;

	int iNumRows;

	if ((iNumRows = static_cast<int>(mysql_num_rows(pRes))) == 0) // no data
	{
		// @fixme402 begin
		static uint32_t dwPID;
		static uint32_t dwCount = 0; //1;
		static TPacketGDAffectElement paeTable = { 0 };

		dwPID = dwRealPID;
		sys_log(0, "AFFECT_LOAD: count %u PID %u RealPID %u", dwCount, dwPID, dwRealPID);

		peer->EncodeHeader(HEADER_DG_AFFECT_LOAD, dwHandle, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(TPacketGDAffectElement) * dwCount);
		peer->Encode(&dwPID, sizeof(uint32_t));
		peer->Encode(&dwCount, sizeof(uint32_t));
		peer->Encode(&paeTable, sizeof(TPacketGDAffectElement) * dwCount);
		// @fixme402 end
		return;
	}

	static std::vector<TPacketGDAffectElement> s_elements;
	s_elements.resize(iNumRows);

	uint32_t dwPID = 0;

	MYSQL_ROW row;

	for (int i = 0; i < iNumRows; ++i)
	{
		TPacketGDAffectElement& r = s_elements[i];
		row = mysql_fetch_row(pRes);

		if (dwPID == 0)
			str_to_number(dwPID, row[0]);

		str_to_number(r.dwType, row[1]);
		str_to_number(r.wApplyOn, row[2]);
		str_to_number(r.lApplyValue, row[3]);
		str_to_number(r.dwFlag, row[4]);
		str_to_number(r.lDuration, row[5]);
		str_to_number(r.lSPCost, row[6]);
#ifdef ENABLE_AFFECT_RENEWAL
		str_to_number(r.dwExpireTime, row[7]);
#endif
	}

	sys_log(0, "AFFECT_LOAD: count %d PID %u", s_elements.size(), dwPID);

	const uint32_t dwCount = s_elements.size();

	peer->EncodeHeader(HEADER_DG_AFFECT_LOAD, dwHandle, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(TPacketGDAffectElement) * dwCount);
	peer->Encode(&dwPID, sizeof(uint32_t));
	peer->Encode(&dwCount, sizeof(uint32_t));
	peer->Encode(&s_elements[0], sizeof(TPacketGDAffectElement) * dwCount);
}

void CClientManager::RESULT_QUEST_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t pid)
{
	if (!peer)
		return;

	int iNumRows;

	if ((iNumRows = static_cast<int>(mysql_num_rows(pRes))) == 0)
	{
		const uint32_t dwCount = 0;
		peer->EncodeHeader(HEADER_DG_QUEST_LOAD, dwHandle, sizeof(uint32_t));
		peer->Encode(&dwCount, sizeof(uint32_t));
		return;
	}

	static std::vector<TQuestTable> s_table;
	s_table.resize(iNumRows);

	MYSQL_ROW row;

	for (int i = 0; i < iNumRows; ++i)
	{
		TQuestTable& r = s_table[i];

		row = mysql_fetch_row(pRes);

		str_to_number(r.dwPID, row[0]);
		strlcpy(r.szName, row[1], sizeof(r.szName));
		strlcpy(r.szState, row[2], sizeof(r.szState));
		str_to_number(r.lValue, row[3]);
	}

	sys_log(0, "QUEST_LOAD: count %d PID %u", s_table.size(), s_table[0].dwPID);

	const uint32_t dwCount = s_table.size();

	peer->EncodeHeader(HEADER_DG_QUEST_LOAD, dwHandle, sizeof(uint32_t) + sizeof(TQuestTable) * dwCount);
	peer->Encode(&dwCount, sizeof(uint32_t));
	peer->Encode(&s_table[0], sizeof(TQuestTable) * dwCount);
}

/*
* PLAYER SAVE
*/
void CClientManager::QUERY_PLAYER_SAVE(CPeer* peer, uint32_t dwHandle, TPlayerTable* pkTab)
{
	if (g_test_server)
		sys_log(0, "PLAYER_SAVE: %s", pkTab->name);

	PutPlayerCache(pkTab);
}

typedef std::map<uint32_t, time_t> time_by_id_map_t;
static time_by_id_map_t s_createTimeByAccountID;

/*
* PLAYER CREATE
*/
void CClientManager::__QUERY_PLAYER_CREATE(CPeer* peer, uint32_t dwHandle, TPlayerCreatePacket* packet)
{
	if (!peer)
		return;

	char queryStr[QUERY_MAX_LEN];
	int queryLen;
	int player_id;

	// You cannot create a character within X seconds of one account.
	time_by_id_map_t::iterator it = s_createTimeByAccountID.find(packet->account_id);

	if (it != s_createTimeByAccountID.end())
	{
		const time_t curtime = time(0);

		if (curtime - it->second < 30)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}
	}

	queryLen = snprintf(queryStr, sizeof(queryStr),
		"SELECT pid%u FROM player_index%s WHERE id=%d", packet->account_index + 1, GetTablePostfix(), packet->account_id);

	auto pMsg0(CDBManager::Instance().DirectQuery(queryStr));
	if (pMsg0->Get()->uiNumRows != 0)
	{
		if (!pMsg0->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg0->Get()->pSQLResult);

		uint32_t dwPID = 0; str_to_number(dwPID, row[0]);
		if (row[0] && dwPID > 0)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			sys_log(0, "ALREADY EXIST AccountChrIdx %d ID %d", packet->account_index, dwPID);
			return;
		}
	}
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	snprintf(queryStr, sizeof(queryStr), "SELECT COUNT(*) as count FROM player%s WHERE name='%s'", GetTablePostfix(), packet->player_table.name);

	auto pMsg1(CDBManager::Instance().DirectQuery(queryStr));
	if (pMsg1->Get()->uiNumRows)
	{
		if (!pMsg1->Get()->pSQLResult)
		{
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg1->Get()->pSQLResult);

		if (*row[0] != '0')
		{
			sys_log(0, "ALREADY EXIST name %s, row[0] %s query %s", packet->player_table.name, row[0], queryStr);
			peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
			return;
		}
	}
	else
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	char szColumns[QUERY_MAX_LEN], szValues[QUERY_MAX_LEN]{};

	int queryColumnsLen = snprintf(szColumns, sizeof(szColumns),
		"id, account_id, name, level, st, ht, dx, iq, job, voice, dir, " // 11
		"x, y, z, hp, mp, random_hp, random_sp, stat_point, stamina, " // 9
		"part_base"); // 1

	int queryValuesLen = snprintf(szValues, sizeof(szValues),
		"0, %u, '%s', %d, %d, %d, %d, %d, %d, %d, %d, " // 11
		"%d, %d, %d, %d, %d, %d, %d, %d, %d, " // 9
		"%d",
		packet->account_id,
		packet->player_table.name,
		packet->player_table.level,
		packet->player_table.st,
		packet->player_table.ht,
		packet->player_table.dx,
		packet->player_table.iq,
		packet->player_table.job,
		packet->player_table.voice,
		packet->player_table.dir,
		packet->player_table.x,
		packet->player_table.y,
		packet->player_table.z,
		packet->player_table.hp,
		packet->player_table.sp,
		packet->player_table.sRandomHP,
		packet->player_table.sRandomSP,
		packet->player_table.stat_point,
		packet->player_table.stamina,
		packet->player_table.part_base);

	static char text[4096 + 1];
	CDBManager::Instance().EscapeString(text, packet->player_table.skills, sizeof(packet->player_table.skills));
	queryColumnsLen += snprintf(szColumns + queryColumnsLen, sizeof(szColumns) - queryColumnsLen, ", skill_level");
	queryValuesLen += snprintf(szValues + queryValuesLen, sizeof(szValues) - queryValuesLen, ", '%s'", text);

	CDBManager::Instance().EscapeString(text, packet->player_table.quickslot, sizeof(packet->player_table.quickslot));
	queryColumnsLen += snprintf(szColumns + queryColumnsLen, sizeof(szColumns) - queryColumnsLen, ", quickslot");
	queryValuesLen += snprintf(szValues + queryValuesLen, sizeof(szValues) - queryValuesLen, ", '%s'", text);

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	CDBManager::Instance().EscapeString(text, packet->player_table.gemItems, sizeof(packet->player_table.gemItems));
	queryColumnsLen += snprintf(szColumns + queryColumnsLen, sizeof(szColumns) - queryColumnsLen, ", gem_items");
	queryValuesLen += snprintf(szValues + queryValuesLen, sizeof(szValues) - queryValuesLen, ", '%s'", text);
#endif

#ifdef ENABLE_FISH_EVENT
	CDBManager::Instance().EscapeString(text, packet->player_table.fishSlots, sizeof(packet->player_table.fishSlots));
	queryColumnsLen += snprintf(szColumns + queryColumnsLen, sizeof(szColumns) - queryColumnsLen, ", fish_slots");
	queryValuesLen += snprintf(szValues + queryValuesLen, sizeof(szValues) - queryValuesLen, ", '%s'", text);
#endif

#ifdef ENABLE_FLOWER_EVENT
	CDBManager::Instance().EscapeString(text, packet->player_table.flowerEvent, sizeof(packet->player_table.flowerEvent));
	queryColumnsLen += snprintf(szColumns + queryColumnsLen, sizeof(szColumns) - queryColumnsLen, ", flower_event");
	queryValuesLen += snprintf(szValues + queryValuesLen, sizeof(szValues) - queryValuesLen, ", '%s'", text);
#endif

	queryLen = snprintf(queryStr, sizeof(queryStr),
		"INSERT INTO player%s (%s) VALUES(%s)",
		GetTablePostfix(), szColumns, szValues);

	sys_log(0,
		"PlayerCreate accountid %d name %s level %d gold %d, st %d ht %d job %d",
		packet->account_id,
		packet->player_table.name,
		packet->player_table.level,
		packet->player_table.gold,
		packet->player_table.st,
		packet->player_table.ht,
		packet->player_table.job);

	auto pMsg2(CDBManager::Instance().DirectQuery(queryStr));
	if (g_test_server)
		sys_log(0, "Create_Player queryLen[%d] TEXT[%s]", queryLen, text);

	if (pMsg2->Get()->uiAffectedRows <= 0)
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_ALREADY, dwHandle, 0);
		sys_log(0, "ALREADY EXIST3 query: %s AffectedRows %lu", queryStr, pMsg2->Get()->uiAffectedRows);
		return;
	}

	player_id = static_cast<int>(pMsg2->Get()->uiInsertID);

	snprintf(queryStr, sizeof(queryStr), "UPDATE player_index%s SET pid%d=%d WHERE id=%d",
		GetTablePostfix(), packet->account_index + 1, player_id, packet->account_id);

	auto pMsg3(CDBManager::Instance().DirectQuery(queryStr));
	if (pMsg3->Get()->uiAffectedRows <= 0)
	{
		sys_err("QUERY_ERROR: %s", queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM player%s WHERE id=%d", GetTablePostfix(), player_id);
		CDBManager::Instance().DirectQuery(queryStr);

		peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_FAILED, dwHandle, 0);
		return;
	}

	TPacketDGCreateSuccess pack;
	memset(&pack, 0, sizeof(pack));

	pack.bAccountCharacterIndex = packet->account_index;

	pack.player.dwID = player_id;
	strlcpy(pack.player.szName, packet->player_table.name, sizeof(pack.player.szName));
	pack.player.byJob = packet->player_table.job;
	pack.player.byLevel = 1;
	pack.player.dwPlayMinutes = 0;
	pack.player.byST = packet->player_table.st;
	pack.player.byHT = packet->player_table.ht;
	pack.player.byDX = packet->player_table.dx;
	pack.player.byIQ = packet->player_table.iq;
	pack.player.wMainPart = packet->player_table.part_base;
#ifdef ENABLE_YOHARA_SYSTEM
	pack.player.bycLevel = 0;
	pack.player.bySungST = packet->player_table.conqueror_st;
	pack.player.bySungHT = packet->player_table.conqueror_ht;
	pack.player.bySungDX = packet->player_table.conqueror_mov;
	pack.player.bySungIQ = packet->player_table.conqueror_imu;
#endif
	pack.player.x = packet->player_table.x;
	pack.player.y = packet->player_table.y;

	peer->EncodeHeader(HEADER_DG_PLAYER_CREATE_SUCCESS, dwHandle, sizeof(TPacketDGCreateSuccess));
	peer->Encode(&pack, sizeof(TPacketDGCreateSuccess));

	sys_log(0, "7 name %s job %d", pack.player.szName, pack.player.byJob);

	s_createTimeByAccountID[packet->account_id] = time(0);
}

/*
* PLAYER DELETE
*/

#ifdef ENABLE_DELETE_FAILURE_TYPE
static int DeleteChecks(int pID, int accID, int* waitTime)
{
	char szQuery[256];
	snprintf(szQuery, sizeof(szQuery), "SELECT * FROM item%s WHERE owner_id = %d and seal_date != 0", GetTablePostfix(), pID);
	auto pMsg = CDBManager::Instance().DirectQuery(szQuery);
	if (pMsg->Get()->uiNumRows)
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_HAVE_SEALED_ITEM);
	
	snprintf(szQuery, sizeof(szQuery), "SELECT * FROM guild_member%s WHERE pid=%d", GetTablePostfix(), pID);
	pMsg = CDBManager::Instance().DirectQuery(szQuery);
	if (pMsg->Get()->uiNumRows)
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_GUILD_MEMBER);

	snprintf(szQuery, sizeof(szQuery), "SELECT * FROM marriage%s WHERE pid1=%d or pid2=%d", GetTablePostfix(), pID, pID);
	pMsg = CDBManager::Instance().DirectQuery(szQuery);
	if (pMsg->Get()->uiNumRows)
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_MARRIAGE);

	snprintf(szQuery, sizeof(szQuery), "SELECT * FROM item%s WHERE owner_id=%d and window='SAFEBOX'", GetTablePostfix(), accID);
	pMsg = CDBManager::Instance().DirectQuery(szQuery);
	if (pMsg->Get()->uiNumRows)
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_LAST_CHAR_SAFEBOX);

	snprintf(szQuery, sizeof(szQuery), "SELECT * FROM item%s WHERE owner_id=%d and window='NPC_STORAGE'", GetTablePostfix(), pID);
	pMsg = CDBManager::Instance().DirectQuery(szQuery);
	if (pMsg->Get()->uiNumRows)
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_ATTR67);

	snprintf(szQuery, sizeof(szQuery), "SELECT * FROM private_shop%s WHERE pid=%d", GetTablePostfix(), pID);
	pMsg = CDBManager::Instance().DirectQuery(szQuery);
	if (pMsg->Get()->uiNumRows)
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_PREMIUM_PRIVATE_SHOP);

	CPlayerTableCache* pc = CClientManager::Instance().GetPlayerCache(pID);
	int playtime = pc ? pc->Get(false)->playtime : 0;
	if (playtime < 10) {
		*waitTime = 10 - playtime;
		return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_REMAIN_TIME);
	}

	return static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_NORMAL);
}
#endif

void CClientManager::__QUERY_PLAYER_DELETE(CPeer* peer, uint32_t dwHandle, TPlayerDeletePacket* packet)
{
	if (!peer)
		return;

	if (!packet->login[0] || !packet->player_id || packet->account_index >= PLAYER_PER_ACCOUNT)
		return;

	CLoginData* ld = GetLoginDataByLogin(packet->login);

	if (!ld)
	{
#ifdef ENABLE_DELETE_FAILURE_TYPE
		TPacketDeleteFailureInfo table;
		table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_NORMAL);
		table.time = 0;
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, sizeof(TPacketDeleteFailureInfo));
		peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
		peer->EncodeBYTE(packet->account_index);
#endif
		return;
	}

#ifdef ENABLE_DELETE_FAILURE_TYPE
	int waitTime = 0;
	const uint8_t delChecks = DeleteChecks(packet->player_id, packet->account_id, &waitTime);
	if (delChecks) {
		TPacketDeleteFailureInfo table;
		table.type = delChecks;
		table.time = waitTime;
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, sizeof(TPacketDeleteFailureInfo));
		peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
		return;
	}
#endif

	const TAccountTable& r = ld->GetAccountRef();

	// block for japan
	if (g_stLocale != "sjis")
	{
		if (!IsChinaEventServer())
		{
			if (strlen(r.social_id) < 7 || strncmp(packet->private_code, r.social_id + strlen(r.social_id) - 7, 7))
			{
				sys_log(0, "PLAYER_DELETE FAILED len(%d)", strlen(r.social_id));
#ifdef ENABLE_DELETE_FAILURE_TYPE
				TPacketDeleteFailureInfo table;
				table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_PRIVATE_CODE_ERROR);
				table.time = 0;
				peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, sizeof(TPacketDeleteFailureInfo));
				peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
				peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
				peer->EncodeBYTE(packet->account_index);
#endif
				return;
			}

			CPlayerTableCache* pkPlayerCache = GetPlayerCache(packet->player_id);
			if (pkPlayerCache)
			{
				const TPlayerTable* pTab = pkPlayerCache->Get();

				if (pTab->level >= m_iPlayerDeleteLevelLimit)
				{
					sys_log(0, "PLAYER_DELETE FAILED LEVEL %u >= DELETE LIMIT %d", pTab->level, m_iPlayerDeleteLevelLimit);
#ifdef ENABLE_DELETE_FAILURE_TYPE
					TPacketDeleteFailureInfo table;
					table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_LIMITE_LEVEL_HIGHER);
					table.time = 0;
					peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, sizeof(TPacketDeleteFailureInfo));
					peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
					peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
					peer->EncodeBYTE(packet->account_index);
#endif
					return;
				}

				if (pTab->level < m_iPlayerDeleteLevelLimitLower)
				{
					sys_log(0, "PLAYER_DELETE FAILED LEVEL %u < DELETE LIMIT %d", pTab->level, m_iPlayerDeleteLevelLimitLower);
#ifdef ENABLE_DELETE_FAILURE_TYPE
					TPacketDeleteFailureInfo table;
					table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_LIMITE_LEVEL_LOWER);
					table.time = 0;
					peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, sizeof(TPacketDeleteFailureInfo));
					peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
					peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, dwHandle, 1);
					peer->EncodeBYTE(packet->account_index);
#endif
					return;
				}
			}
		}
	}

	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "SELECT p.id, p.level, p.name FROM player_index%s AS i, player%s AS p WHERE pid%u=%u AND pid%u=p.id",
		GetTablePostfix(), GetTablePostfix(), packet->account_index + 1, packet->player_id, packet->account_index + 1);

	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle, packet->player_id);
	pi->account_index = packet->account_index;

	sys_log(0, "PLAYER_DELETE TRY: %s %d pid%d", packet->login, packet->player_id, packet->account_index + 1);
	CDBManager::Instance().ReturnQuery(szQuery, QID_PLAYER_DELETE, peer->GetHandle(), pi);
}

//
// @version 05/06/10 Bang2ni - Added deletion of price information list when deleting player.
//
void CClientManager::__RESULT_PLAYER_DELETE(CPeer* peer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;

	if (msg->Get() && msg->Get()->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		uint32_t dwPID = 0;
		str_to_number(dwPID, row[0]);

		int deletedLevelLimit = 0;
		str_to_number(deletedLevelLimit, row[1]);

		char szName[64];
		strlcpy(szName, row[2], sizeof(szName));

		if (deletedLevelLimit >= m_iPlayerDeleteLevelLimit && !IsChinaEventServer())
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u >= DELETE LIMIT %d", deletedLevelLimit, m_iPlayerDeleteLevelLimit);
#ifdef ENABLE_DELETE_FAILURE_TYPE
			TPacketDeleteFailureInfo table;
			table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_LIMITE_LEVEL_HIGHER);
			table.time = 0;
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, sizeof(TPacketDeleteFailureInfo));
			peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
#endif
			return;
		}

		if (deletedLevelLimit < m_iPlayerDeleteLevelLimitLower)
		{
			sys_log(0, "PLAYER_DELETE FAILED LEVEL %u < DELETE LIMIT %d", deletedLevelLimit, m_iPlayerDeleteLevelLimitLower);
#ifdef ENABLE_DELETE_FAILURE_TYPE
			TPacketDeleteFailureInfo table;
			table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_LIMITE_LEVEL_LOWER);
			table.time = 0;
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, sizeof(TPacketDeleteFailureInfo));
			peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
#endif
			return;
		}

		char queryStr[QUERY_MAX_LEN];

		snprintf(queryStr, sizeof(queryStr), "INSERT INTO player%s_deleted SELECT * FROM player%s WHERE id=%d",
			GetTablePostfix(), GetTablePostfix(), pi->player_id);

		auto pIns(CDBManager::Instance().DirectQuery(queryStr));
		if (pIns->Get()->uiAffectedRows == 0 || pIns->Get()->uiAffectedRows == (uint32_t)-1)
		{
			sys_log(0, "PLAYER_DELETE FAILED %u CANNOT INSERT TO player%s_deleted", dwPID, GetTablePostfix());
#ifdef ENABLE_DELETE_FAILURE_TYPE
			TPacketDeleteFailureInfo table;
			table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_NORMAL);//zero rows
			table.time = 0;
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, sizeof(TPacketDeleteFailureInfo));
			peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
#endif
			return;
		}

		// successful deletion
		sys_log(0, "PLAYER_DELETE SUCCESS %u", dwPID);

		char account_index_string[16];

		snprintf(account_index_string, sizeof(account_index_string), "player_id%d", m_iPlayerIDStart + pi->account_index);

		// Delete the player table from the cache.
		CPlayerTableCache* pkPlayerCache = GetPlayerCache(pi->player_id);

		if (pkPlayerCache)
		{
			m_map_playerCache.erase(pi->player_id);
			delete pkPlayerCache;
		}

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		{
			auto it = m_map_AchievementCache.find(pi->player_id);
			if (it != m_map_AchievementCache.end()) {
				it->second->Flush();
				delete it->second;
				m_map_AchievementCache.erase(it);
			}
		}
#endif

		// Delete the items from the cache.
		TItemCacheSet* pSet = GetItemCacheSet(pi->player_id);

		if (pSet)
		{
			TItemCacheSet::iterator it = pSet->begin();

			while (it != pSet->end())
			{
				CItemCache* pkItemCache = *it++;
				DeleteItemCache(pkItemCache->Get()->id);
			}

			pSet->clear();
			delete pSet;

			m_map_pkItemCacheSetPtr.erase(pi->player_id);
		}

		snprintf(queryStr, sizeof(queryStr), "UPDATE player_index%s SET pid%u=0 WHERE pid%u=%d",
			GetTablePostfix(),
			pi->account_index + 1,
			pi->account_index + 1,
			pi->player_id);

		auto pMsg(CDBManager::Instance().DirectQuery(queryStr));
		if (pMsg->Get()->uiAffectedRows == 0 || pMsg->Get()->uiAffectedRows == (uint32_t)-1)
		{
			sys_log(0, "PLAYER_DELETE FAIL WHEN UPDATE account table");
#ifdef ENABLE_DELETE_FAILURE_TYPE
			TPacketDeleteFailureInfo table;
			table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_NORMAL);//zero rows
			table.time = 0;
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, sizeof(TPacketDeleteFailureInfo));
			peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
			peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
			peer->EncodeBYTE(pi->account_index);
#endif
			return;
		}

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM player%s WHERE id=%d", GetTablePostfix(), pi->player_id);
		CDBManager::Instance().DirectQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM item%s WHERE owner_id=%d AND (`window` in ('INVENTORY','EQUIPMENT','DRAGON_SOUL_INVENTORY','BELT_INVENTORY'"
#ifdef ENABLE_SWITCHBOT
			",'SWITCHBOT'"
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			",'NPC_STORAGE'"
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			",'PREMIUM_PRIVATE_SHOP'"
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			",'ADDITIONAL_EQUIPMENT_1'"
#endif
			"))", GetTablePostfix(), pi->player_id);

		CDBManager::Instance().DirectQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM quest%s WHERE dwPID=%d", GetTablePostfix(), pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM affect%s WHERE dwPID=%d", GetTablePostfix(), pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM guild_member%s WHERE pid=%d", GetTablePostfix(), pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);

		// MYSHOP_PRICE_LIST
		snprintf(queryStr, sizeof(queryStr), "DELETE FROM myshop_pricelist%s WHERE owner_id=%d", GetTablePostfix(), pi->player_id);
		CDBManager::Instance().AsyncQuery(queryStr);
		// END_OF_MYSHOP_PRICE_LIST

		snprintf(queryStr, sizeof(queryStr), "DELETE FROM messenger_list%s WHERE account='%s' OR companion='%s'", GetTablePostfix(), szName, szName);
		CDBManager::Instance().AsyncQuery(queryStr);

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && !defined(ENABLE_DELETE_FAILURE_TYPE)
		// Find player's shop, if it exists, close it and then delete everything about it
		// (Items in item table have already been deleted)
		spShop shop = GetShop(pi->player_id);
		if (shop)
		{
			RequestCloseShop(shop, true);
			shopMap.erase(pi->player_id);

			snprintf(queryStr, sizeof(queryStr), "DELETE FROM private_shop WHERE pid=%u", pi->player_id);
			CDBManager::Instance().AsyncQuery(queryStr);

			snprintf(queryStr, sizeof(queryStr), "DELETE FROM private_shop_items WHERE pid=%u", pi->player_id);
			CDBManager::Instance().AsyncQuery(queryStr);
		}
#endif

		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_SUCCESS, pi->dwHandle, 1);
		peer->EncodeBYTE(pi->account_index);
	}
	else
	{
		// delete failed
		sys_log(0, "PLAYER_DELETE FAIL NO ROW");
#ifdef ENABLE_DELETE_FAILURE_TYPE
		TPacketDeleteFailureInfo table;
		table.type = static_cast<uint8_t>(EDeleteTypes::DELETE_FAILURE_NORMAL);//unknown error
		table.time = 0;
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, sizeof(TPacketDeleteFailureInfo));
		peer->Encode(&table, sizeof(TPacketDeleteFailureInfo));
#else
		peer->EncodeHeader(HEADER_DG_PLAYER_DELETE_FAILED, pi->dwHandle, 1);
		peer->EncodeBYTE(pi->account_index);
#endif
	}
}

void CClientManager::QUERY_ADD_AFFECT(CPeer* peer, const TPacketGDAddAffect* p)
{
	char queryStr[QUERY_MAX_LEN];
	/*
	snprintf(queryStr, sizeof(queryStr),
	"INSERT INTO affect%s (dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost) "
	"VALUES(%u, %u, %u, %d, %u, %d, %d) "
	"ON DUPLICATE KEY UPDATE lApplyValue=%d, dwFlag=%u, lDuration=%d, lSPCost=%d",
	GetTablePostfix(),
	p->dwPID,
	p->elem.dwType,
	p->elem.bApplyOn,
	p->elem.lApplyValue,
	p->elem.dwFlag,
	p->elem.lDuration,
	p->elem.lSPCost,
	p->elem.lApplyValue,
	p->elem.dwFlag,
	p->elem.lDuration,
	p->elem.lSPCost);
	*/
	snprintf(queryStr, sizeof(queryStr),
		"REPLACE INTO affect%s (dwPID, bType, bApplyOn, lApplyValue, dwFlag, lDuration, lSPCost"
#ifdef ENABLE_AFFECT_RENEWAL
		", expireTime"
#endif
		") "
		"VALUES(%u, %u, %u, %ld, %u, %ld, %ld"
#ifdef ENABLE_AFFECT_RENEWAL
		", IF(%u > 0, FROM_UNIXTIME(%u), '0000-00-00 00:00:00')"
#endif
		")",
		GetTablePostfix(),
		p->dwPID,
		p->elem.dwType,
		p->elem.wApplyOn,
		p->elem.lApplyValue,
		p->elem.dwFlag,
		p->elem.lDuration,
		p->elem.lSPCost
#ifdef ENABLE_AFFECT_RENEWAL
		, p->elem.dwExpireTime
		, p->elem.dwExpireTime
#endif
	);

	CDBManager::Instance().AsyncQuery(queryStr);
}

void CClientManager::QUERY_REMOVE_AFFECT(CPeer* peer, const TPacketGDRemoveAffect* p)
{
	char queryStr[QUERY_MAX_LEN];

	snprintf(queryStr, sizeof(queryStr),
		"DELETE FROM affect%s WHERE dwPID=%u AND bType=%u AND bApplyOn=%u",
		GetTablePostfix(), p->dwPID, p->dwType, p->wApplyOn);

	CDBManager::Instance().AsyncQuery(queryStr);
}


void CClientManager::QUERY_HIGHSCORE_REGISTER(CPeer* peer, TPacketGDHighscore* data)
{
	if (!peer)
		return;

	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "SELECT value FROM highscore%s WHERE board='%s' AND pid = %u", GetTablePostfix(), data->szBoard, data->dwPID);

	sys_log(0, "HEADER_GD_HIGHSCORE_REGISTER: PID %u", data->dwPID);

	ClientHandleInfo* pi = new ClientHandleInfo(0);
	strlcpy(pi->login, data->szBoard, sizeof(pi->login));
	pi->account_id = (uint32_t)data->lValue;
	pi->player_id = data->dwPID;
	pi->account_index = (data->cDir > 0);

	CDBManager::Instance().ReturnQuery(szQuery, QID_HIGHSCORE_REGISTER, peer->GetHandle(), pi);
}

void CClientManager::RESULT_HIGHSCORE_REGISTER(CPeer* pkPeer, SQLMsg* msg)
{
	if (!msg)
		return;

	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;
	//uint32_t dwHandle = pi->dwHandle;

	char szBoard[21];
	strlcpy(szBoard, pi->login, sizeof(szBoard));
	int value = (int)pi->account_id;

	SQLResult* res = msg->Get();

	if (res->uiNumRows == 0)
	{
		// insert new high score
		char buf[256];
		snprintf(buf, sizeof(buf), "INSERT INTO highscore%s VALUES('%s', %u, %d)", GetTablePostfix(), szBoard, pi->player_id, value);
		CDBManager::Instance().AsyncQuery(buf);
	}
	else
	{
		if (!res->pSQLResult)
		{
			delete pi;
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(res->pSQLResult);
		if (row && row[0])
		{
			int current_value = 0; str_to_number(current_value, row[0]);
			if (((pi->account_index) && (current_value >= value)) || ((!pi->account_index) && (current_value <= value)))
			{
				value = current_value;
			}
			else
			{
				char buf[256];
				snprintf(buf, sizeof(buf), "REPLACE INTO highscore%s VALUES('%s', %u, %d)", GetTablePostfix(), szBoard, pi->player_id, value);
				CDBManager::Instance().AsyncQuery(buf);
			}
		}
		else
		{
			char buf[256];
			snprintf(buf, sizeof(buf), "INSERT INTO highscore%s VALUES('%s', %u, %d)", GetTablePostfix(), szBoard, pi->player_id, value);
			CDBManager::Instance().AsyncQuery(buf);
		}
	}
	// TODO: Check if the high score has been updated here and post a notice.
	delete pi;
}

void CClientManager::InsertLogoutPlayer(uint32_t pid)
{
	const TLogoutPlayerMap::iterator it = m_map_logout.find(pid);

	// If it doesn't exist, add it
	if (it != m_map_logout.end())
	{
		// update time only if it exists
		if (g_log)
			sys_log(0, "LOGOUT: Update player time pid(%d)", pid);

		it->second->time = time(0);
		return;
	}

	TLogoutPlayer* pLogout = new TLogoutPlayer;
	pLogout->pid = pid;
	pLogout->time = time(0);
	m_map_logout.insert(std::make_pair(pid, pLogout));

	if (g_log)
		sys_log(0, "LOGOUT: Insert player pid(%d)", pid);
}

void CClientManager::DeleteLogoutPlayer(uint32_t pid)
{
	const TLogoutPlayerMap::iterator it = m_map_logout.find(pid);

	if (it != m_map_logout.end())
	{
		delete it->second;
		m_map_logout.erase(it);
	}
}

extern int g_iLogoutSeconds;

void CClientManager::UpdateLogoutPlayer()
{
	const time_t now = time(0);

	TLogoutPlayerMap::iterator it = m_map_logout.begin();

	while (it != m_map_logout.end())
	{
		TLogoutPlayer* pLogout = it->second;

		if (now - g_iLogoutSeconds > pLogout->time)
		{
			FlushItemCacheSet(pLogout->pid);
			FlushPlayerCacheSet(pLogout->pid);

			delete pLogout;
			m_map_logout.erase(it++);
		}
		else
			++it;
	}
}

void CClientManager::FlushPlayerCacheSet(uint32_t pid)
{
	const TPlayerTableCacheMap::iterator it = m_map_playerCache.find(pid);

	if (it != m_map_playerCache.end())
	{
		CPlayerTableCache* c = it->second;
		m_map_playerCache.erase(it);

		c->Flush();
		delete c;
	}
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
void CClientManager::PlayerLogoutEvent(uint32_t pid)
{
	NotifyShopLogout(pid);
}
#endif

void CClientManager::PlayerLoginEvent(uint32_t pid)
{
	SendSingleShop(pid);
#	ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
	CancelOpenShopOffline(pid);
#	endif
}

#ifdef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
void CClientManager::PlayerSetShopTime(uint32_t pid)
{
	NotifyShopTime(pid);
}
#endif
#endif
