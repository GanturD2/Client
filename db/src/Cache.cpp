
#include "stdafx.h"
#include "Cache.h"

#include "QID.h"
#include "ClientManager.h"
#include "Main.h"

extern CPacketInfo g_item_info;
extern int g_iPlayerCacheFlushSeconds;
extern int g_iItemCacheFlushSeconds;
extern int g_test_server;
// MYSHOP_PRICE_LIST
extern int g_iItemPriceListTableCacheFlushSeconds;
// END_OF_MYSHOP_PRICE_LIST
//
extern int g_item_count;
constexpr int auctionMinFlushSec = 1800;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
extern int g_iShopCacheFlushSeconds;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
extern int g_iSkillColorCacheFlushSeconds;

CSKillColorCache::CSKillColorCache()
{
	m_expireTime = MIN(1800, g_iSkillColorCacheFlushSeconds);
}

CSKillColorCache::~CSKillColorCache()
{
}

void CSKillColorCache::OnFlush()
{
	char query[QUERY_MAX_LEN];
	snprintf(query, sizeof(query),
#ifdef ENABLE_NINETH_SKILL
		"REPLACE INTO skill_color%s (`player_id`, "
		"`s1_col1`, `s1_col2`, `s1_col3`, `s1_col4`, `s1_col5`, "
		"`s2_col1`, `s2_col2`, `s2_col3`, `s2_col4`, `s2_col5`, "
		"`s3_col1`, `s3_col2`, `s3_col3`, `s3_col4`, `s3_col5`, "
		"`s4_col1`, `s4_col2`, `s4_col3`, `s4_col4`, `s4_col5`, "
		"`s5_col1`, `s5_col2`, `s5_col3`, `s5_col4`, `s5_col5`, "
		"`s6_col1`, `s6_col2`, `s6_col3`, `s6_col4`, `s6_col5`, " // end of normal skills
		"`s7_col1`, `s7_col2`, `s7_col3`, `s7_col4`, `s7_col5`, " // begin of new skills
		"`s8_col1`, `s8_col2`, `s8_col3`, `s8_col4`, `s8_col5`, "
		"`s9_col1`, `s9_col2`, `s9_col3`, `s9_col4`, `s9_col5`, " // end of new skills
		"`s10_col1`, `s10_col2`, `s10_col3`, `s10_col4`, `s10_col5`, " // begin of buffs
		"`s11_col1`, `s11_col2`, `s11_col3`, `s11_col4`, `s11_col5`, "
		"`s12_col1`, `s12_col2`, `s12_col3`, `s12_col4`, `s12_col5`, "
		"`s13_col1`, `s13_col2`, `s13_col3`, `s13_col4`, `s13_col5`, "
		"`s14_col1`, `s14_col2`, `s14_col3`, `s14_col4`, `s14_col5`, "
		"`s15_col1`, `s15_col2`, `s15_col3`, `s15_col4`, `s15_col5`" // nine skill shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
		", `s16_col1`, `s16_col2`, `s16_col3`, `s16_col4`, `s16_col5`"
#	endif
		") "
		"VALUES (%d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, " // end of normal skills
		"%d, %d, %d, %d, %d, " // begin of new skills
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, " // end of new skills
		"%d, %d, %d, %d, %d, " // begin of buffs
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d" // nine skill shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
		", %d, %d, %d, %d, %d"
#	endif
		")", GetTablePostfix(), m_data.player_id,
		m_data.dwSkillColor[0][0], m_data.dwSkillColor[0][1], m_data.dwSkillColor[0][2], m_data.dwSkillColor[0][3], m_data.dwSkillColor[0][4],
		m_data.dwSkillColor[1][0], m_data.dwSkillColor[1][1], m_data.dwSkillColor[1][2], m_data.dwSkillColor[1][3], m_data.dwSkillColor[1][4],
		m_data.dwSkillColor[2][0], m_data.dwSkillColor[2][1], m_data.dwSkillColor[2][2], m_data.dwSkillColor[2][3], m_data.dwSkillColor[2][4],
		m_data.dwSkillColor[3][0], m_data.dwSkillColor[3][1], m_data.dwSkillColor[3][2], m_data.dwSkillColor[3][3], m_data.dwSkillColor[3][4],
		m_data.dwSkillColor[4][0], m_data.dwSkillColor[4][1], m_data.dwSkillColor[4][2], m_data.dwSkillColor[4][3], m_data.dwSkillColor[4][4],
		m_data.dwSkillColor[5][0], m_data.dwSkillColor[5][1], m_data.dwSkillColor[5][2], m_data.dwSkillColor[5][3], m_data.dwSkillColor[5][4], // end of skills
		m_data.dwSkillColor[6][0], m_data.dwSkillColor[6][1], m_data.dwSkillColor[6][2], m_data.dwSkillColor[6][3], m_data.dwSkillColor[6][4], // begin of new skills
		m_data.dwSkillColor[7][0], m_data.dwSkillColor[7][1], m_data.dwSkillColor[7][2], m_data.dwSkillColor[7][3], m_data.dwSkillColor[7][4],
		m_data.dwSkillColor[8][0], m_data.dwSkillColor[8][1], m_data.dwSkillColor[8][2], m_data.dwSkillColor[8][3], m_data.dwSkillColor[8][4], // end of new skills
		m_data.dwSkillColor[9][0], m_data.dwSkillColor[9][1], m_data.dwSkillColor[9][2], m_data.dwSkillColor[9][3], m_data.dwSkillColor[9][4], // begin of buffs
		m_data.dwSkillColor[10][0], m_data.dwSkillColor[10][1], m_data.dwSkillColor[10][2], m_data.dwSkillColor[10][3], m_data.dwSkillColor[10][4],
		m_data.dwSkillColor[11][0], m_data.dwSkillColor[11][1], m_data.dwSkillColor[11][2], m_data.dwSkillColor[11][3], m_data.dwSkillColor[11][4],
		m_data.dwSkillColor[12][0], m_data.dwSkillColor[12][1], m_data.dwSkillColor[12][2], m_data.dwSkillColor[12][3], m_data.dwSkillColor[12][4],
		m_data.dwSkillColor[13][0], m_data.dwSkillColor[13][1], m_data.dwSkillColor[13][2], m_data.dwSkillColor[13][3], m_data.dwSkillColor[13][4],
		m_data.dwSkillColor[14][0], m_data.dwSkillColor[14][1], m_data.dwSkillColor[14][2], m_data.dwSkillColor[14][3], m_data.dwSkillColor[14][4] // nine skill shaman
#	ifdef ENABLE_WOLFMAN_CHARACTER
		, m_data.dwSkillColor[15][0], m_data.dwSkillColor[15][1], m_data.dwSkillColor[15][2], m_data.dwSkillColor[15][3], m_data.dwSkillColor[15][4]
#	endif
	);
#else
		"REPLACE INTO skill_color%s (`player_id`, "
		"`s1_col1`, `s1_col2`, `s1_col3`, `s1_col4`, `s1_col5`, "
		"`s2_col1`, `s2_col2`, `s2_col3`, `s2_col4`, `s2_col5`, "
		"`s3_col1`, `s3_col2`, `s3_col3`, `s3_col4`, `s3_col5`, "
		"`s4_col1`, `s4_col2`, `s4_col3`, `s4_col4`, `s4_col5`, "
		"`s5_col1`, `s5_col2`, `s5_col3`, `s5_col4`, `s5_col5`, "
		"`s6_col1`, `s6_col2`, `s6_col3`, `s6_col4`, `s6_col5`, " // end of skills
		"`s7_col1`, `s7_col2`, `s7_col3`, `s7_col4`, `s7_col5`, " // begin buffs
		"`s8_col1`, `s8_col2`, `s8_col3`, `s8_col4`, `s8_col5`, "
		"`s9_col1`, `s9_col2`, `s9_col3`, `s9_col4`, `s9_col5`, "
		"`s10_col1`, `s10_col2`, `s10_col3`, `s10_col4`, `s10_col5`, "
		"`s11_col1`, `s11_col2`, `s11_col3`, `s11_col4`, `s11_col5`"
		#ifdef ENABLE_WOLFMAN_CHARACTER
		", `s12_col1`, `s12_col2`, `s12_col3`, `s12_col4`, `s12_col5`"
		#endif
		") "
		"VALUES (%d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, " // end of skills
		"%d, %d, %d, %d, %d, " // begin buffs
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d, "
		"%d, %d, %d, %d, %d"
		#ifdef ENABLE_WOLFMAN_CHARACTER
		", %d, %d, %d, %d, %d"
		#endif
		")", GetTablePostfix(), m_data.player_id,
		m_data.dwSkillColor[0][0], m_data.dwSkillColor[0][1], m_data.dwSkillColor[0][2], m_data.dwSkillColor[0][3], m_data.dwSkillColor[0][4],
		m_data.dwSkillColor[1][0], m_data.dwSkillColor[1][1], m_data.dwSkillColor[1][2], m_data.dwSkillColor[1][3], m_data.dwSkillColor[1][4],
		m_data.dwSkillColor[2][0], m_data.dwSkillColor[2][1], m_data.dwSkillColor[2][2], m_data.dwSkillColor[2][3], m_data.dwSkillColor[2][4],
		m_data.dwSkillColor[3][0], m_data.dwSkillColor[3][1], m_data.dwSkillColor[3][2], m_data.dwSkillColor[3][3], m_data.dwSkillColor[3][4],
		m_data.dwSkillColor[4][0], m_data.dwSkillColor[4][1], m_data.dwSkillColor[4][2], m_data.dwSkillColor[4][3], m_data.dwSkillColor[4][4],
		m_data.dwSkillColor[5][0], m_data.dwSkillColor[5][1], m_data.dwSkillColor[5][2], m_data.dwSkillColor[5][3], m_data.dwSkillColor[5][4], // end of skills
		m_data.dwSkillColor[6][0], m_data.dwSkillColor[6][1], m_data.dwSkillColor[6][2], m_data.dwSkillColor[6][3], m_data.dwSkillColor[6][4], // begin buffs
		m_data.dwSkillColor[7][0], m_data.dwSkillColor[7][1], m_data.dwSkillColor[7][2], m_data.dwSkillColor[7][3], m_data.dwSkillColor[7][4],
		m_data.dwSkillColor[8][0], m_data.dwSkillColor[8][1], m_data.dwSkillColor[8][2], m_data.dwSkillColor[8][3], m_data.dwSkillColor[8][4],
		m_data.dwSkillColor[9][0], m_data.dwSkillColor[9][1], m_data.dwSkillColor[9][2], m_data.dwSkillColor[9][3], m_data.dwSkillColor[9][4],
		m_data.dwSkillColor[10][0], m_data.dwSkillColor[10][1], m_data.dwSkillColor[10][2], m_data.dwSkillColor[10][3], m_data.dwSkillColor[10][4]
		#ifdef ENABLE_WOLFMAN_CHARACTER
		, m_data.dwSkillColor[11][0], m_data.dwSkillColor[11][1], m_data.dwSkillColor[11][2], m_data.dwSkillColor[11][3], m_data.dwSkillColor[11][4]
		#endif
	);
#endif

	CDBManager::Instance().ReturnQuery(query, QID_SKILL_COLOR_SAVE, 0, nullptr);

	if (g_test_server)
		sys_log(0, "SkillColorCache::Flush :REPLACE %u (%s)", m_data.player_id, query);

	m_bNeedQuery = false;
}
#endif

CItemCache::CItemCache()
{
	m_expireTime = MIN(1800, g_iItemCacheFlushSeconds);
}

CItemCache::~CItemCache()
{
}

// 이거 이상한데...
// Delete를 했으면, Cache도 해제해야 하는것 아닌가???
// 근데 Cache를 해제하는 부분이 없어.
// 못 찾은 건가?
// 이렇게 해놓으면, 계속 시간이 될 때마다 아이템을 계속 지워...
// 이미 사라진 아이템인데... 확인사살??????
// fixme
// by rtsummit
void CItemCache::Delete()
{
	if (m_data.vnum == 0)
		return;

	//char szQuery[QUERY_MAX_LEN];
	//szQuery[QUERY_MAX_LEN] = '\0';
	if (g_test_server)
		sys_log(0, "ItemCache::Delete : DELETE %u", m_data.id);

	m_data.vnum = 0;
	m_bNeedQuery = true;
	m_lastUpdateTime = time(0);
	OnFlush();

	//m_bNeedQuery = false;
	//m_lastUpdateTime = time(0) - m_expireTime; // 바로 타임아웃 되도록 하자.
}

void CItemCache::OnFlush()
{
	if (m_data.vnum == 0) // vnum이 0이면 삭제하라고 표시된 것이다.
	{
		char szQuery[QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM item%s WHERE id=%u", GetTablePostfix(), m_data.id);
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_DESTROY, 0, nullptr);

		if (g_test_server)
			sys_log(0, "ItemCache::Flush : DELETE %u %s", m_data.id, szQuery);
	}
	else
	{
		long alSockets[ITEM_SOCKET_MAX_NUM];
		bool isSocket = false;

		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
		bool isAttr = false;

#ifdef ENABLE_YOHARA_SYSTEM
		TPlayerItemApplyRandom	aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
		bool isRandomAttr = false;

		uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
		bool isRandomValues = false;
#endif

		memset(&alSockets, 0, sizeof(long) * ITEM_SOCKET_MAX_NUM);
		memset(&aAttr, 0, sizeof(TPlayerItemAttribute) * ITEM_ATTRIBUTE_MAX_NUM);
#ifdef ENABLE_YOHARA_SYSTEM
		memset(&aApplyRandom, 0, sizeof(TPlayerItemApplyRandom) * APPLY_RANDOM_SLOT_MAX_NUM);
		memset(&alRandomValues, 0, sizeof(uint16_t) * ITEM_RANDOM_VALUES_MAX_NUM);
#endif

		TPlayerItem * p = &m_data;

		if (memcmp(alSockets, p->alSockets, sizeof(long) * ITEM_SOCKET_MAX_NUM))
			isSocket = true;

		if (memcmp(aAttr, p->aAttr, sizeof(TPlayerItemAttribute) * ITEM_ATTRIBUTE_MAX_NUM))
			isAttr = true;

#ifdef ENABLE_YOHARA_SYSTEM
		if (memcmp(aApplyRandom, p->aApplyRandom, sizeof(TPlayerItemApplyRandom) * APPLY_RANDOM_SLOT_MAX_NUM))
			isRandomAttr = true;

		if (memcmp(alRandomValues, p->alRandomValues, sizeof(uint16_t) * ITEM_RANDOM_VALUES_MAX_NUM))
			isRandomValues = true;
#endif

		char szColumns[QUERY_MAX_LEN];
		char szValues[QUERY_MAX_LEN];
		char szUpdate[QUERY_MAX_LEN];

		int iLen = snprintf(szColumns, sizeof(szColumns), "id, owner_id, `window`, pos, count, vnum");

		int iValueLen = snprintf(szValues, sizeof(szValues), "%u, %u, %d, %d, %u, %u",
				p->id, p->owner, p->window, p->pos, p->count, p->vnum);

		int iUpdateLen = snprintf(szUpdate, sizeof(szUpdate), "owner_id=%u, `window`=%d, pos=%d, count=%u, vnum=%u",
				p->owner, p->window, p->pos, p->count, p->vnum);

#ifdef ENABLE_SEALBIND_SYSTEM
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", seal_date");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %ld", p->nSealDate);
		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen, ", seal_date=%ld", p->nSealDate);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", transmutation");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %u", p->dwTransmutationVnum);
		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen, ", transmutation=%u", p->dwTransmutationVnum);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", is_basic");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %d", p->is_basic);
		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen, ", is_basic=%d", p->is_basic);
#endif
#ifdef ENABLE_REFINE_ELEMENT
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
			", %d, %d, %d, %d, %d, %d ,%d, %d", p->grade_element, p->attack_element[0], p->attack_element[1], p->attack_element[2], p->element_type_bonus, p->elements_value_bonus[0], p->elements_value_bonus[1], p->elements_value_bonus[2]);
		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
			", grade_elements=%d, attackelement1=%d, attackelement2=%d, attackelement3=%d, elementstype=%d, elementsvalue1=%d, elementsvalue2=%d, elementsvalue3=%d", p->grade_element, p->attack_element[0], p->attack_element[1], p->attack_element[2], p->element_type_bonus, p->elements_value_bonus[0], p->elements_value_bonus[1], p->elements_value_bonus[2]);
#endif

		if (isSocket)
		{
#ifdef ENABLE_PROTO_RENEWAL
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", socket0, socket1, socket2, socket3, socket4, socket5");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
				", %lu, %lu, %lu, %lu, %lu, %lu", p->alSockets[0], p->alSockets[1], p->alSockets[2], p->alSockets[3], p->alSockets[4], p->alSockets[5]);
			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
				", socket0=%lu, socket1=%lu, socket2=%lu, socket3=%lu, socket4=%lu, socket5=%lu", p->alSockets[0], p->alSockets[1], p->alSockets[2], p->alSockets[3], p->alSockets[4], p->alSockets[5]);
#else
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", socket0, socket1, socket2");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
					", %lu, %lu, %lu", p->alSockets[0], p->alSockets[1], p->alSockets[2]);
			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
					", socket0=%lu, socket1=%lu, socket2=%lu", p->alSockets[0], p->alSockets[1], p->alSockets[2]);
#endif
		}

		if (isAttr)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen,
					", attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3"
					", attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6"
			);

			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
					", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d"
					,
					p->aAttr[0].wType, p->aAttr[0].sValue,
					p->aAttr[1].wType, p->aAttr[1].sValue,
					p->aAttr[2].wType, p->aAttr[2].sValue,
					p->aAttr[3].wType, p->aAttr[3].sValue,
					p->aAttr[4].wType, p->aAttr[4].sValue,
					p->aAttr[5].wType, p->aAttr[5].sValue,
					p->aAttr[6].wType, p->aAttr[6].sValue
			);

			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
					", attrtype0=%d, attrvalue0=%d"
					", attrtype1=%d, attrvalue1=%d"
					", attrtype2=%d, attrvalue2=%d"
					", attrtype3=%d, attrvalue3=%d"
					", attrtype4=%d, attrvalue4=%d"
					", attrtype5=%d, attrvalue5=%d"
					", attrtype6=%d, attrvalue6=%d"
					,
					p->aAttr[0].wType, p->aAttr[0].sValue,
					p->aAttr[1].wType, p->aAttr[1].sValue,
					p->aAttr[2].wType, p->aAttr[2].sValue,
					p->aAttr[3].wType, p->aAttr[3].sValue,
					p->aAttr[4].wType, p->aAttr[4].sValue,
					p->aAttr[5].wType, p->aAttr[5].sValue,
					p->aAttr[6].wType, p->aAttr[6].sValue
			);
		}

#ifdef ENABLE_YOHARA_SYSTEM
		if (isRandomAttr)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", randomtype0, randomvalue0, randomrow0, randomtype1, randomvalue1, randomrow1, randomtype2, randomvalue2, randomrow2");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %d, %d, %d, %d, %d, %d, %d, %d, %d", p->aApplyRandom[0].wType, p->aApplyRandom[0].sValue, p->aApplyRandom[0].row, p->aApplyRandom[1].wType, p->aApplyRandom[1].sValue, p->aApplyRandom[1].row, p->aApplyRandom[2].wType, p->aApplyRandom[2].sValue, p->aApplyRandom[2].row);

			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
				", randomtype0=%d, randomvalue0=%d, randomrow0=%d"
				", randomtype1=%d, randomvalue1=%d, randomrow1=%d"
				", randomtype2=%d, randomvalue2=%d, randomrow2=%d",
				p->aApplyRandom[0].wType, p->aApplyRandom[0].sValue, p->aApplyRandom[0].row,
				p->aApplyRandom[1].wType, p->aApplyRandom[1].sValue, p->aApplyRandom[1].row,
				p->aApplyRandom[2].wType, p->aApplyRandom[2].sValue, p->aApplyRandom[2].row
			);
		}

		if (isRandomValues)
		{
			iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", rValue0, rValue1, rValue2, rValue3");
			iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen,
				", %d, %d, %d, %d", p->alRandomValues[0], p->alRandomValues[1], p->alRandomValues[2], p->alRandomValues[3]);
			iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen,
				", rValue0=%d, rValue1=%d, rValue2=%d, rValue3=%d", p->alRandomValues[0], p->alRandomValues[1], p->alRandomValues[2], p->alRandomValues[3]);
		}
#endif

#ifdef ENABLE_SET_ITEM
		iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", setItem");
		iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %d", p->set_value);
		iUpdateLen += snprintf(szUpdate + iUpdateLen, sizeof(szUpdate) - iUpdateLen, ", setItem=%d", p->set_value);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		const TItemTable* item_table = CClientManager::Instance().GetItemTableByVnum(m_data.vnum);
		if (item_table && item_table->bType == ITEM_PET && ((item_table->bSubType == PET_UPBRINGING) || (item_table->bSubType == PET_BAG)))
			//if (item_table)
		{
			if (p->aPetInfo.pet_id)
			{
				char szGrowthPetQuery[QUERY_MAX_LEN], szGrowthPetColumns[QUERY_MAX_LEN], szGrowthPetValues[QUERY_MAX_LEN];

				snprintf(szGrowthPetColumns, sizeof(szGrowthPetColumns),
					"id, pet_vnum, pet_nick, pet_level, evol_level, pet_type, pet_hp, pet_def, pet_sp, "
					"pet_duration, pet_birthday, exp_monster, exp_item, skill_count, "
					"pet_skill1, pet_skill_level1, pet_skill_spec1, pet_skill_cool1, "
					"pet_skill2, pet_skill_level2, pet_skill_spec2, pet_skill_cool2, "
					"pet_skill3, pet_skill_level3, pet_skill_spec3, pet_skill_cool3");

				snprintf(szGrowthPetValues, sizeof(szGrowthPetValues),
					"%d, %d, '%s', %d, %d, %d, %.1f, %.1f, %.1f, %d, %d, %d, %d, %d, "
					"%d, %d, %.1f, %d, %d, %d, %.1f, %d, %d, %d, %.1f, %d",
					p->aPetInfo.pet_id, p->aPetInfo.pet_vnum, p->aPetInfo.pet_nick, p->aPetInfo.pet_level, p->aPetInfo.evol_level, p->aPetInfo.pet_type,
					p->aPetInfo.pet_hp, p->aPetInfo.pet_def, p->aPetInfo.pet_sp, p->aPetInfo.pet_max_time,
					p->aPetInfo.pet_birthday, p->aPetInfo.exp_monster, p->aPetInfo.exp_item, p->aPetInfo.skill_count,
					p->aPetInfo.skill_vnum[0], p->aPetInfo.skill_level[0], p->aPetInfo.skill_spec[0], p->aPetInfo.skill_cool[0],
					p->aPetInfo.skill_vnum[1], p->aPetInfo.skill_level[1], p->aPetInfo.skill_spec[1], p->aPetInfo.skill_cool[1],
					p->aPetInfo.skill_vnum[2], p->aPetInfo.skill_level[2], p->aPetInfo.skill_spec[2], p->aPetInfo.skill_cool[2]
				);

				snprintf(szGrowthPetQuery, sizeof(szGrowthPetQuery),
					"REPLACE INTO pet%s (%s) VALUES(%s);", GetTablePostfix(), szGrowthPetColumns, szGrowthPetValues);

				//sys_err("Cache: |%s|", szGrowthPetQuery);
				//sys_err("Cache type: |%d|", item_table->bType);
				//sys_err("Cache subtype: |%d|", item_table->bSubType);

				CDBManager::Instance().ReturnQuery(szGrowthPetQuery, QID_ITEM_SAVE, 0, nullptr);
			}
		}
#endif

		char szItemQuery[QUERY_MAX_LEN + QUERY_MAX_LEN];
		snprintf(szItemQuery, sizeof(szItemQuery), "REPLACE INTO item%s (%s) VALUES(%s)", GetTablePostfix(), szColumns, szValues);

		if (g_test_server)
			sys_log(0, "ItemCache::Flush :REPLACE  (%s)", szItemQuery);

		CDBManager::Instance().ReturnQuery(szItemQuery, QID_ITEM_SAVE, 0, nullptr);

		//g_item_info.Add(p->vnum);
		++g_item_count;
	}

	m_bNeedQuery = false;
}

//
// CPlayerTableCache
//
CPlayerTableCache::CPlayerTableCache()
{
	m_expireTime = MIN(1800, g_iPlayerCacheFlushSeconds);
}

CPlayerTableCache::~CPlayerTableCache()
{
}

void CPlayerTableCache::OnFlush()
{
	if (g_test_server)
		sys_log(0, "PlayerTableCache::Flush : %s", m_data.name);

	char szQuery[QUERY_MAX_LEN];
	CreatePlayerSaveQuery(szQuery, sizeof(szQuery), &m_data);
	CDBManager::Instance().ReturnQuery(szQuery, QID_PLAYER_SAVE, 0, nullptr);
}

// MYSHOP_PRICE_LIST
//
// CItemPriceListTableCache class implementation
//

const int CItemPriceListTableCache::s_nMinFlushSec = 1800;

CItemPriceListTableCache::CItemPriceListTableCache()
{
	m_expireTime = MIN(s_nMinFlushSec, g_iItemPriceListTableCacheFlushSeconds);
}

void CItemPriceListTableCache::UpdateList(const TItemPriceListTable* pUpdateList)
{
	//
	// 이미 캐싱된 아이템과 중복된 아이템을 찾고 중복되지 않는 이전 정보는 tmpvec 에 넣는다.
	//

	std::vector<TItemPriceInfo> tmpvec;

	for (uint idx = 0; idx < m_data.byCount; ++idx)
	{
		const TItemPriceInfo* pos = pUpdateList->aPriceInfo;
		for (; pos != pUpdateList->aPriceInfo + pUpdateList->byCount && m_data.aPriceInfo[idx].dwVnum != pos->dwVnum; ++pos)
			;

		if (pos == pUpdateList->aPriceInfo + pUpdateList->byCount)
			tmpvec.emplace_back(m_data.aPriceInfo[idx]);
	}

	//
	// pUpdateList 를 m_data 에 복사하고 남은 공간을 tmpvec 의 앞에서 부터 남은 만큼 복사한다.
	//

	if (pUpdateList->byCount > SHOP_PRICELIST_MAX_NUM)
	{
		sys_err("Count overflow!");
		return;
	}

	m_data.byCount = pUpdateList->byCount;

	thecore_memcpy(m_data.aPriceInfo, pUpdateList->aPriceInfo, sizeof(TItemPriceInfo) * pUpdateList->byCount);

	int nDeletedNum;	// 삭제된 가격정보의 갯수

	if (pUpdateList->byCount < SHOP_PRICELIST_MAX_NUM)
	{
		size_t sizeAddOldDataSize = SHOP_PRICELIST_MAX_NUM - pUpdateList->byCount;

		if (tmpvec.size() < sizeAddOldDataSize)
			sizeAddOldDataSize = tmpvec.size();
		if (tmpvec.size() != 0)
		{
			thecore_memcpy(m_data.aPriceInfo + pUpdateList->byCount, &tmpvec[0], sizeof(TItemPriceInfo) * sizeAddOldDataSize);
			m_data.byCount += sizeAddOldDataSize;
		}
		nDeletedNum = tmpvec.size() - sizeAddOldDataSize;
	}
	else
		nDeletedNum = tmpvec.size();

	m_bNeedQuery = true;

	sys_log(0,
		"ItemPriceListTableCache::UpdateList : OwnerID[%u] Update [%u] Items, Delete [%u] Items, Total [%u] Items",
		m_data.dwOwnerID, pUpdateList->byCount, nDeletedNum, m_data.byCount);
}

void CItemPriceListTableCache::OnFlush()
{
	char szQuery[QUERY_MAX_LEN];

	//
	// 이 캐시의 소유자에 대한 기존에 DB 에 저장된 아이템 가격정보를 모두 삭제한다.
	//

	snprintf(szQuery, sizeof(szQuery), "DELETE FROM myshop_pricelist%s WHERE owner_id = %u", GetTablePostfix(), m_data.dwOwnerID);
	CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_DESTROY, 0, nullptr);

	//
	// 캐시의 내용을 모두 DB 에 쓴다.
	//

	for (int idx = 0; idx < m_data.byCount; ++idx)
	{
		snprintf(szQuery, sizeof(szQuery),
#ifdef ENABLE_CHEQUE_SYSTEM
			"REPLACE myshop_pricelist%s(owner_id, item_vnum, price, cheque) VALUES(%u, %u, %u, %u)", // @fixme204 (INSERT INTO -> REPLACE)
			GetTablePostfix(), m_data.dwOwnerID, m_data.aPriceInfo[idx].dwVnum, m_data.aPriceInfo[idx].price.dwPrice, m_data.aPriceInfo[idx].price.dwCheque);
#else
			"REPLACE myshop_pricelist%s(owner_id, item_vnum, price) VALUES(%u, %u, %u)", // @fixme204 (INSERT INTO -> REPLACE)
			GetTablePostfix(), m_data.dwOwnerID, m_data.aPriceInfo[idx].dwVnum, m_data.aPriceInfo[idx].dwPrice);
#endif
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_SAVE, 0, nullptr);
	}

	sys_log(0, "ItemPriceListTableCache::Flush : OwnerID[%u] Update [%u]Items", m_data.dwOwnerID, m_data.byCount);

	m_bNeedQuery = false;
}

CItemPriceListTableCache::~CItemPriceListTableCache()
{
}
// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
//
// CShopCache
//
CShopCache::CShopCache()
{
	m_expireTime = MIN(360, g_iShopCacheFlushSeconds);
}

CShopCache::~CShopCache()
{
}

void CShopCache::OnFlush()
{
	if (g_test_server)
		sys_log(0, "ShopCache::Flush: %u", m_data.pid);

	char shopNameEscaped[SHOP_SIGN_MAX_LEN * 2 + 1];
	CDBManager::Instance().EscapeString(shopNameEscaped, m_data.shopName, strlen(m_data.shopName));

	//Save the main shop info
	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO private_shop (pid, x, y, mapindex, channel, sign, open_time, is_closed, offline_left"
		", gold_stash"
#	ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
		", cheque_stash"
#	endif
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		", poly_vnum, title_type, is_deco"
#	endif
		") "
		"VALUES (%u, %ld, %ld, %ld, %d, '%s', FROM_UNIXTIME(%u), %d, %d"
		", %u"
#	ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
		", %u"
#	endif
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		", %d, %d, %d"
#	endif
		") "

		"ON DUPLICATE KEY UPDATE x = VALUES(x), y = VALUES(y), channel = VALUES(channel), mapindex = VALUES(mapindex), "
		"open_time = VALUES(open_time), sign = VALUES(sign), is_closed = VALUES(is_closed), offline_left = VALUES(offline_left), "
		"gold_stash = VALUES(gold_stash)"
#	ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
		", cheque_stash = VALUES(cheque_stash)"
#	endif
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		", poly_vnum = VALUES(poly_vnum)"
		", title_type = VALUES(title_type)"
		", is_deco = VALUES(is_deco)"
#	endif
		,
		m_data.pid,
		m_data.x,
		m_data.y,
		m_data.mapIndex,
		m_data.channel,
		shopNameEscaped,
		m_data.openTime,
		m_data.closed ? 1 : 0,
		m_data.offlineMinutesLeft,
		m_data.goldStash
#	ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
		, m_data.chequeStash
#	endif
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		, m_data.dwPolyVnum
		, m_data.bTitleType
		, m_data.isShopDeco ? 1 : 0
#	endif
	);

	CDBManager::Instance().AsyncQuery(szQuery);

	//Delete old items
	snprintf(szQuery, sizeof(szQuery),
		"DELETE FROM private_shop_items WHERE pid = %d", m_data.pid);

	CDBManager::Instance().AsyncQuery(szQuery);

#	ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	size_t len = snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO private_shop_items (pid, pos, price, cheque) VALUES ");
#	else
	size_t len = snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO private_shop_items (pid, pos, price) VALUES ");
#	endif

	bool cancel = true;
	for (const TShopItemTable sitem : m_data.items)
	{
		if (sitem.vnum == 0)
			continue;

		if (cancel)
			cancel = false;

		len += snprintf(szQuery + len, sizeof(szQuery) - len,
#	ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
			"(%u, %u, %u, %u),",
#	else
			"(%u, %u, %u),",
#	endif
			m_data.pid,
			sitem.display_pos,
			sitem.price
#	ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
			, sitem.cheque
#	endif
		);
	}

	if (cancel)
		return;

	szQuery[len - 1] = '\0'; //Remove the last comma
	CDBManager::Instance().AsyncQuery(szQuery);
}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
CAchievementCache::CAchievementCache(bool bMemSet) : cache(bMemSet) {
	m_expireTime = 1800; // 30 min
}

CAchievementCache::~CAchievementCache() {}

void CAchievementCache::OnFlush()
{
	char query[QUERY_MAX_LEN];

	snprintf(query, QUERY_MAX_LEN,
		"DELETE FROM `achievement_tasks` WHERE `pid` = %d; DELETE FROM `achievements` WHERE `pid` = %d;", m_data.pid, m_data.pid);
	CDBManager::instance().DirectQuery(query, SQL_PLAYER);

	snprintf(query, QUERY_MAX_LEN,
		"REPLACE INTO `achievement_data`(`pid`, `points`, `title`) VALUES(%d, %d, %d);", m_data.pid, m_data.points, m_data.title);
	CDBManager::instance().DirectQuery(query, SQL_PLAYER);

	for (const auto& achievement : m_data.achievements)
	{
		const auto& finished_it = achievement.second.find(0);
		if (finished_it != achievement.second.end())
		{
			snprintf(
				query, QUERY_MAX_LEN,
				"INSERT INTO `achievements`(`pid`, `achievement`, `finished`) "
				"VALUES(%d, %d, %llu);",
				m_data.pid, achievement.first, finished_it->second);
			CDBManager::instance().DirectQuery(query, SQL_PLAYER);
			continue;
		}
		else
		{
			snprintf(
				query, QUERY_MAX_LEN,
				"INSERT INTO `achievements`(`pid`, `achievement`, `finished`) "
				"VALUES(%d, %d, 0);",
				m_data.pid, achievement.first);
			CDBManager::instance().DirectQuery(query, SQL_PLAYER);

			for (const auto& task : achievement.second)
			{
				snprintf(query, QUERY_MAX_LEN,
					"INSERT INTO `achievement_tasks`(`pid`, `achievement_id`, "
					"`task_id`, `task_value`) "
					"VALUES(%d, %d, %d, %llu);",
					m_data.pid, achievement.first, task.first, task.second);
				CDBManager::instance().DirectQuery(query, SQL_PLAYER);
			}
		}

	}

	m_bNeedQuery = false;
}
#endif
