#include "stdafx.h"
#include "constants.h"
#include "log.h"
#include "item.h"
#include "char.h"
#include "desc.h"
#include "item_manager.h"
#ifdef ENABLE_NEWSTUFF
#include "config.h"
#endif

constexpr int MAX_NORM_ATTR_NUM = ITEM_MANAGER::MAX_NORM_ATTR_NUM;
constexpr int MAX_RARE_ATTR_NUM = ITEM_MANAGER::MAX_RARE_ATTR_NUM;

int CItem::GetAttributeSetIndex()
{
	if (GetType() == ITEM_WEAPON)
	{
		if (GetSubType() == WEAPON_ARROW)
			return -1;

#ifdef ENABLE_QUIVER_SYSTEM
		if (GetSubType() == WEAPON_QUIVER)
			return -1;
#endif

		return ATTRIBUTE_SET_WEAPON;
	}

	if (GetType() == ITEM_ARMOR)
	{
		switch (GetSubType())
		{
			case ARMOR_BODY:
				return ATTRIBUTE_SET_BODY;

			case ARMOR_WRIST:
				return ATTRIBUTE_SET_WRIST;

			case ARMOR_FOOTS:
				return ATTRIBUTE_SET_FOOTS;

			case ARMOR_NECK:
				return ATTRIBUTE_SET_NECK;

			case ARMOR_HEAD:
				return ATTRIBUTE_SET_HEAD;

			case ARMOR_SHIELD:
				return ATTRIBUTE_SET_SHIELD;

			case ARMOR_EAR:
				return ATTRIBUTE_SET_EAR;

#if defined(ENABLE_PENDANT) && defined(ENABLE_PENDANT_ITEM_ATTR)
			case ARMOR_PENDANT:
				return ATTRIBUTE_SET_PENDANT;
#endif
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_GLOVE_ITEM_ATTR)
			case ARMOR_GLOVE:
				return ATTRIBUTE_SET_GLOVE;
#endif
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		switch (GetSubType())
		{
			case COSTUME_BODY:
#ifdef ENABLE_ITEM_ATTR_COSTUME
				return ATTRIBUTE_SET_COSTUME_BODY;
#else
				return ATTRIBUTE_SET_BODY;
#endif

			case COSTUME_HAIR:
#ifdef ENABLE_ITEM_ATTR_COSTUME
				return ATTRIBUTE_SET_COSTUME_HAIR;
#else
				return ATTRIBUTE_SET_HEAD;
#endif

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			case COSTUME_MOUNT:
				break;
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			case COSTUME_WEAPON:
#ifdef ENABLE_ITEM_ATTR_COSTUME
				return ATTRIBUTE_SET_COSTUME_WEAPON;
#else
				return ATTRIBUTE_SET_WEAPON;
#endif
#endif

#ifdef ENABLE_AURA_SYSTEM
			case COSTUME_AURA:
				break;
#endif

			default:
				break;
		}
	}

	return -1;
}

bool CItem::HasAttr(uint16_t wApply)	//@fixme532
{
	if (attribute_double_change)	//@custom024
	{
		for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
			if (m_pProto->aApplies[i].wType == wApply)
				return true;
	}

	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
		if (GetAttributeType(i) == wApply)
			return true;

	return false;
}

bool CItem::HasApply(uint16_t wApply)	//@fixme532
{
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		if (m_pProto->aApplies[i].wType == wApply)
			return true;

	return false;
}

bool CItem::HasRareAttr(uint16_t wApply)	//@fixme532
{
	for (int i = 0; i < MAX_RARE_ATTR_NUM; ++i)
		if (GetAttributeType(i + 5) == wApply)
			return true;

	return false;
}

void CItem::AddAttribute(uint16_t wApply, int16_t sValue)	//@fixme532
{
	if (HasAttr(wApply))
		return;

	const int i = GetAttributeCount();

	if (i >= MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		if (sValue)
			SetAttribute(i, wApply, sValue);
	}
}

void CItem::AddAttr(uint16_t wApply, uint8_t bLevel) //@fixme532
{
	if (HasAttr(wApply))
		return;

	if (bLevel <= 0)
		return;

	const int i = GetAttributeCount();

	if (i == MAX_NORM_ATTR_NUM)
		sys_err("item attribute overflow!");
	else
	{
		const TItemAttrTable& r = g_map_itemAttr[wApply];
		const int16_t lVal = static_cast<int16_t>(r.lValues[MIN(4, bLevel - 1)]);

		if (lVal)
			SetAttribute(i, wApply, lVal);
	}
}

void CItem::PutAttributeWithLevel(uint8_t bLevel) //@fixme532
{
	const int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int> avail;

	int total = 0;

	// Build an array of attachable properties
	for (uint16_t i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemAttr[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasAttr(i))
		{
			avail.emplace_back(i);
			total += r.dwProb;
		}
	}

	// Selecting properties to attach through probability calculation with constructed array
	uint32_t prob = 0;
	if (total != 0)
		prob = number(1, total);

	uint16_t attr_idx = APPLY_NONE;

	for (uint32_t i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable& r = g_map_itemAttr[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable& r = g_map_itemAttr[attr_idx];

	// Limit the maximum value of attribute level by type
	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddAttr(attr_idx, bLevel);
}

void CItem::PutAttribute(const int* aiAttrPercentTable)
{
	int iAttrLevelPercent = number(1, 100);
	int i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutAttributeWithLevel(i + 1);
}

void CItem::ChangeAttribute(const int* aiChangeProb)
{
	const int iAttributeCount = GetAttributeCount();

	ClearAttribute();

	if (iAttributeCount == 0)
		return;

	TItemTable const* pProto = GetProto();

	if (pProto && pProto->sAddonType)
		ApplyAddon(pProto->sAddonType);

	static const int tmpChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		0, 10, 40, 35, 15,
	};

	for (int i = GetAttributeCount(); i < iAttributeCount; ++i)
	{
		if (aiChangeProb == nullptr)
			PutAttribute(tmpChangeProb);
		else
			PutAttribute(aiChangeProb);
	}
}

void CItem::ChangeAttributeValue()
{
	int attrCount = GetAttributeCount();
	if (attrCount == 0)
		return;

	int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	for (int k = 0; k < attrCount; k++)
	{
		int attr_idx = GetAttributeType(k);

		const TItemAttrTable& r = g_map_itemAttr[attr_idx];
		int nLevel = number(1, r.bMaxLevelBySet[iAttributeSet]);
		long lVal = r.lValues[MIN(4, nLevel - 1)];

		if (lVal)
			SetAttribute(k, attr_idx, lVal);
	}
}

void CItem::AddAttribute()
{
	static const int aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		40, 50, 10, 0, 0
	};

	if (GetAttributeCount() < MAX_NORM_ATTR_NUM)
		PutAttribute(aiItemAddAttributePercent);
}

void CItem::ClearAttribute()
{
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		m_aAttr[i].wType = 0;
		m_aAttr[i].sValue = 0;
	}
}

void CItem::ClearAllAttribute()
{
	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		m_aAttr[i].wType = 0;
		m_aAttr[i].sValue = 0;
	}
}

int CItem::GetAttributeCount()
{
	int i;

	for (i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == 0)
			break;
	}

	return i;
}

int CItem::FindAttribute(uint16_t wType) //@fixme532
{
	for (int i = 0; i < MAX_NORM_ATTR_NUM; ++i)
	{
		if (GetAttributeType(i) == wType)
			return i;
	}

	return -1;
}

bool CItem::RemoveAttributeAt(int index)
{
	if (GetAttributeCount() <= index)
		return false;

	for (int i = index; i < MAX_NORM_ATTR_NUM - 1; ++i)
	{
		SetAttribute(i, GetAttributeType(i + 1), GetAttributeValue(i + 1));
	}

	SetAttribute(MAX_NORM_ATTR_NUM - 1, APPLY_NONE, 0);
	return true;
}

bool CItem::RemoveAttributeType(uint16_t wType) //@fixme532
{
	const int index = FindAttribute(wType);
	return index != -1 && RemoveAttributeType(index);
}

void CItem::SetAttributes(const TPlayerItemAttribute* c_pAttribute)
{
	thecore_memcpy(m_aAttr, c_pAttribute, sizeof(m_aAttr));
	Save();
}

void CItem::SetAttribute(int i, uint16_t wType, int16_t sValue) //@fixme532
{
	assert(i < MAX_NORM_ATTR_NUM);

	m_aAttr[i].wType = wType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();

	if (wType)
	{
		const char* pszIP = nullptr;

		if (GetOwner() && GetOwner()->GetDesc())
			pszIP = GetOwner()->GetDesc()->GetHostName();

		LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().ItemLog(i, wType, sValue, GetID(), "SET_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum()));
	}
}

void CItem::SetForceAttribute(int i, uint16_t wType, int16_t sValue) //@fixme532
{
	assert(i < ITEM_ATTRIBUTE_MAX_NUM);

	m_aAttr[i].wType = wType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();

	if (wType)
	{
		const char* pszIP = nullptr;

		if (GetOwner() && GetOwner()->GetDesc())
			pszIP = GetOwner()->GetDesc()->GetHostName();

		LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().ItemLog(i, wType, sValue, GetID(), "SET_FORCE_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum()));
	}
}


void CItem::CopyAttributeTo(LPITEM pItem)
{
	if (pItem)
		pItem->SetAttributes(m_aAttr);
}

int CItem::GetRareAttrCount()
{
	int ret = 0;

	for (uint32_t dwIdx = ITEM_ATTRIBUTE_RARE_START; dwIdx < ITEM_ATTRIBUTE_RARE_END; dwIdx++)
	{
		if (m_aAttr[dwIdx].wType != 0)
			ret++;
	}

	return ret;
}

bool CItem::ChangeRareAttribute()
{
	if (GetRareAttrCount() == 0)
		return false;

	const int cnt = GetRareAttrCount();

	for (int i = 0; i < cnt; ++i)
	{
		m_aAttr[i + ITEM_ATTRIBUTE_RARE_START].wType = 0;
		m_aAttr[i + ITEM_ATTRIBUTE_RARE_START].sValue = 0;
	}

	if (GetOwner() && GetOwner()->GetDesc())
		LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().ItemLog(GetOwner(), this, "SET_RARE_CHANGE", ""))
	else
		LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().ItemLog(0, 0, 0, GetID(), "SET_RARE_CHANGE", "", "", GetOriginalVnum()))

	for (int i = 0; i < cnt; ++i)
	{
		AddRareAttribute();
	}

	return true;
}

bool CItem::AddRareAttribute()
{
	const int count = GetRareAttrCount();

	if (count >= ITEM_ATTRIBUTE_RARE_NUM)
		return false;

	int pos = count + ITEM_ATTRIBUTE_RARE_START;
	TPlayerItemAttribute& attr = m_aAttr[pos];

	const int nAttrSet = GetAttributeSetIndex();
	std::vector<int> avail;

	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemRare[i];

		if (r.dwApplyIndex != 0 && r.bMaxLevelBySet[nAttrSet] > 0 && HasRareAttr(i) != true)
		{
			avail.emplace_back(i);
		}
	}

	if (avail.empty())	//@fixme542
	{
		sys_err("Couldn't add a rare bonus - item_attr_rare has incorrect values!");
		return false;
	}

	const TItemAttrTable& r = g_map_itemRare[avail[number(0, avail.size() - 1)]];
	int nAttrLevel = number(1, ITEM_ATTRIBUTE_MAX_LEVEL); // Nyx: 06052021 - Fixed for get random

	if (nAttrLevel > r.bMaxLevelBySet[nAttrSet])
		nAttrLevel = r.bMaxLevelBySet[nAttrSet];

	attr.wType = r.dwApplyIndex;
	attr.sValue = static_cast<int16_t>(r.lValues[nAttrLevel - 1]);

	UpdatePacket();

	Save();

	const char* pszIP = nullptr;

	if (GetOwner() && GetOwner()->GetDesc())
		pszIP = GetOwner()->GetDesc()->GetHostName();

	LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().ItemLog(pos, attr.wType, attr.sValue, GetID(), "SET_RARE", "", pszIP ? pszIP : "", GetOriginalVnum()));
	return true;
}

void CItem::AddRareAttribute2(const int* aiAttrPercentTable)
{
	static const int aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] =
	{
		40, 50, 10, 0, 0
	};
	if (aiAttrPercentTable == nullptr)
		aiAttrPercentTable = aiItemAddAttributePercent;

	if (GetRareAttrCount() < MAX_RARE_ATTR_NUM)
		PutRareAttribute(aiAttrPercentTable);
}

void CItem::PutRareAttribute(const int* aiAttrPercentTable)
{
	int iAttrLevelPercent = number(1, 100);
	int i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
	{
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutRareAttributeWithLevel(i + 1);
}

void CItem::PutRareAttributeWithLevel(uint8_t bLevel)	//@fixme532
{
	const int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int> avail;

	int total = 0;

	// Build an array of attachable properties
	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemRare[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasRareAttr(i))
		{
			avail.emplace_back(i);
			total += r.dwProb;
		}
	}

	// Selecting properties to attach through probability calculation with constructed array
	uint32_t prob = number(1, total);
	int attr_idx = APPLY_NONE;

	for (uint32_t i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable& r = g_map_itemRare[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item rare attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable& r = g_map_itemRare[attr_idx];

	// Limit the maximum value of attribute level by type
	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddRareAttr(attr_idx, bLevel);
}

void CItem::AddRareAttr(uint16_t wApply, uint8_t bLevel) //@fixme532
{
	if (HasRareAttr(wApply))
		return;

	if (bLevel <= 0)
		return;

	const int i = ITEM_ATTRIBUTE_RARE_START + GetRareAttrCount();

	if (i == ITEM_ATTRIBUTE_RARE_END)
		sys_err("item rare attribute overflow!");
	else
	{
		const TItemAttrTable& r = g_map_itemRare[wApply];
		const int16_t lVal = static_cast<int16_t>(r.lValues[MIN(4, bLevel - 1)]);

		if (lVal)
			SetForceAttribute(i, wApply, lVal);
	}
}

#ifdef ENABLE_YOHARA_SYSTEM
// Defaultfunctions
int CItem::GetRandomAttrCount()
{
	int ret = 0;

	for (uint32_t dwIdx = 0; dwIdx < APPLY_RANDOM_SLOT_MAX_NUM; dwIdx++)
		if (m_aApplyRandom[dwIdx].wType != 0)
			ret++;

	return ret;
}

void CItem::ClearRandomAttribute()
{
	for (int i = 0; i < APPLY_RANDOM_SLOT_MAX_NUM; ++i) {
		m_aApplyRandom[i].wType = 0;
		m_aApplyRandom[i].sValue = 0;
		m_aApplyRandom[i].row = 0;
	}
}

void CItem::SetRandomAttrs(const TPlayerItemApplyRandom* c_pAttribute)
{
	thecore_memcpy(m_aApplyRandom, c_pAttribute, sizeof(m_aApplyRandom));
	Save();
}

void CItem::CopyApplyRandomTo(LPITEM pItem)
{
	pItem->SetRandomAttrs(m_aApplyRandom);
}

////////////////////////////////////////////////////////////////////
// Refine
void CItem::SetItemApplyRandom(int i, uint16_t wType, int16_t sValue, int8_t bRow)	//@fixme532
{
	assert(i < APPLY_RANDOM_SLOT_MAX_NUM);

	m_aApplyRandom[i].wType = wType;
	m_aApplyRandom[i].sValue = sValue;
	m_aApplyRandom[i].row = bRow;
	UpdatePacket();
	Save();

	if (wType)
	{
		const char* pszIP = nullptr;

		if (GetOwner() && GetOwner()->GetDesc())
			pszIP = GetOwner()->GetDesc()->GetHostName();

		LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().ItemLog(i, wType, sValue, GetID(), "SET_SUNGMA_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum()));
	}
}

void CItem::PutRandomAttributeWithLevel(uint16_t bLevel)	//@fixme532
{
	int iAttributeSet = GetAttributeSetIndex();
	if (iAttributeSet < 0)
		return;

	if (bLevel > APPLY_RANDOM_SLOT_MAX_NUM)
		return;

	std::vector<int> avail;

	int total = 0;

	for (int i = 0; i < MAX_APPLY_NUM; ++i)
	{
		const TItemAttrTable& r = g_map_itemAttr[i];

		if (r.bMaxLevelBySet[iAttributeSet] && !HasRandomAttr(i))
		{
			avail.emplace_back(i);
			total += r.dwProb;
		}
	}

	total = (total == 0 ? 1 : total);
	unsigned int prob = number(1, total);
	int attr_idx = APPLY_NONE;

	for (uint32_t i = 0; i < avail.size(); ++i)
	{
		const TItemAttrTable& r = g_map_itemAttr[avail[i]];

		if (prob <= r.dwProb)
		{
			attr_idx = avail[i];
			break;
		}

		prob -= r.dwProb;
	}

	if (!attr_idx)
	{
		sys_err("Cannot put item random attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const TItemAttrTable& r = g_map_itemAttr[attr_idx];

	if (bLevel > r.bMaxLevelBySet[iAttributeSet])
		bLevel = r.bMaxLevelBySet[iAttributeSet];

	AddRandomAttr(attr_idx, bLevel);
}

void CItem::AddRandomAttr(uint16_t wApply, uint16_t bLevel)	//@fixme532
{
	if (HasRandomAttr(wApply))
		return;

	if (bLevel <= 0)
		return;

	int i = GetRandomAttrCount();

	if (i == APPLY_RANDOM_SLOT_MAX_NUM)
		sys_err("item random attribute overflow!");
	else
	{
		const TItemAttrTable& r = g_map_itemAttr[wApply];
		long lVal = r.lValues[MIN(4, bLevel - 1)];

		if (lVal)
			SetItemApplyRandom(i, wApply, lVal, 0);
	}
}

bool CItem::HasRandomAttr(uint16_t wApply)	//@fixme532
{
	for (int i = 0; i < APPLY_RANDOM_SLOT_MAX_NUM; ++i)
		if (GetApplyRandomType(i) == wApply)
			return true;

	return false;
}
#endif

#ifdef ENABLE_CHANGED_ATTR
#include "utils.h"
void CItem::GetSelectAttr(TPlayerItemAttribute(&arr)[ITEM_ATTRIBUTE_MAX_NUM])
{
	auto __GetAttributeCount = [&arr]() -> int
	{
		auto c = std::count_if(std::begin(arr), std::end(arr),
			[](const TPlayerItemAttribute& _attr) { return _attr.wType != 0 && _attr.sValue != 0; });

		return c;
	};

	auto __HasAttr = [this, &arr](const uint8_t bApply) -> bool
	{
		if (m_pProto)
		{
			for (size_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
				if (m_pProto->aApplies[i].wType == bApply)
					return true;
		}

		for (size_t i = 0; i < MAX_NORM_ATTR_NUM; ++i)
			if (arr[i].wType == bApply)
				return true;

		return false;
	};

	auto __PutAttributeWithLevel = [this, &__HasAttr, &__GetAttributeCount, &arr](uint8_t bLevel) -> void
	{
		const int iAttributeSet = GetAttributeSetIndex();

		if (iAttributeSet < 0)
			return;

		if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
			return;

		std::vector<int> avail;

		int total = 0;

		for (size_t i = 0; i < MAX_APPLY_NUM; ++i)
		{
			const TItemAttrTable& r = g_map_itemAttr[i];

			if (r.bMaxLevelBySet[iAttributeSet] && !__HasAttr(i))
			{
				avail.emplace_back(i);
				total += r.dwProb;
			}
		}

		unsigned int prob = number(1, total);
		int attr_idx = APPLY_NONE;

		for (size_t i = 0; i < avail.size(); ++i)
		{
			const TItemAttrTable& r = g_map_itemAttr[avail[i]];

			if (prob <= r.dwProb)
			{
				attr_idx = avail[i];
				break;
			}

			prob -= r.dwProb;
		}

		if (!attr_idx)
		{
			sys_err("Cannot put item attribute %d %d", iAttributeSet, bLevel);
			return;
		}

		const TItemAttrTable& r = g_map_itemAttr[attr_idx];

		if (bLevel > r.bMaxLevelBySet[iAttributeSet])
			bLevel = r.bMaxLevelBySet[iAttributeSet];

		const long lVal = r.lValues[MIN(4, bLevel - 1)];
		arr[__GetAttributeCount()] = { static_cast<uint8_t>(attr_idx), static_cast<int16_t>(lVal) };
	};

	if (m_pProto && m_pProto->sAddonType)
	{
		int iSkillBonus = MINMAX(-30, static_cast<int>(gauss_random(0, 5) + 0.5f), 30);
		int iNormalHitBonus = 0;
		if (abs(iSkillBonus) <= 20)
			iNormalHitBonus = -2 * iSkillBonus + abs(number(-8, 8) + number(-8, 8)) + number(1, 4);
		else
			iNormalHitBonus = -2 * iSkillBonus + number(1, 5);

		arr[__GetAttributeCount()] = { static_cast<uint8_t>(APPLY_NORMAL_HIT_DAMAGE_BONUS), static_cast<int16_t>(iNormalHitBonus) };
		arr[__GetAttributeCount()] = { static_cast<uint8_t>(APPLY_SKILL_DAMAGE_BONUS), static_cast<int16_t>(iSkillBonus) };
	}

	static constexpr uint8_t aiAttrPercentTable[ITEM_ATTRIBUTE_MAX_LEVEL] = { 0, 10, 40, 35, 15 };
	for (uint8_t c = __GetAttributeCount(); c < GetAttributeCount(); c++)
	{
		int iAttrLevelPercent = number(1, 100);
		size_t i;

		for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i)
		{
			if (iAttrLevelPercent <= aiAttrPercentTable[i])
				break;

			iAttrLevelPercent -= aiAttrPercentTable[i];
		}

		__PutAttributeWithLevel(i + 1);
	}

	std::copy(std::begin(m_aAttr) + MAX_NORM_ATTR_NUM, std::end(m_aAttr), std::begin(arr) + MAX_NORM_ATTR_NUM);
}
#endif
