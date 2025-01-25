#include "StdAfx.h"
#include "../EterPack/EterPackManager.h"
#include "PythonNonPlayer.h"
#include "InstanceBase.h"
#include "PythonCharacterManager.h"

bool CPythonNonPlayer::LoadNonPlayerData(const char* c_szFileName)
{
	static uint32_t s_adwMobProtoKey[4] = { 4813894, 18955, 552631, 6822045 };

	CMappedFile file;
	LPCVOID pvData;

	Tracef("CPythonNonPlayer::LoadNonPlayerData: %s, sizeof(TMobTable)=%u\n", c_szFileName, sizeof(TMobTable));

	if (!CEterPackManager::Instance().Get(file, c_szFileName, &pvData))
		return false;

	uint32_t dwFourCC, dwElements, dwDataSize;

	file.Read(&dwFourCC, sizeof(uint32_t));

	if (dwFourCC != MAKEFOURCC('M', 'M', 'P', 'T'))
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid Mob proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(uint32_t));
	file.Read(&dwDataSize, sizeof(uint32_t));

	std::vector<uint8_t> pbData(dwDataSize);
	file.Read(pbData.data(), dwDataSize);
	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData.data(), s_adwMobProtoKey))
		return false;

	uint32_t structSize = zObj.GetSize() / dwElements;
	uint32_t structDiff = zObj.GetSize() % dwElements;
	if ((zObj.GetSize() % sizeof(TMobTable)) != 0)
	{
		TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid size %u check data format. structSize %u, structDiff %u", zObj.GetSize(), structSize, structDiff);
		return false;
	}

	for (uint32_t i = 0; i < dwElements; ++i)
	{
		CPythonNonPlayer::TMobTable& t = *((CPythonNonPlayer::TMobTable*)zObj.GetBuffer() + i);
#ifdef ENABLE_INGAME_WIKI
		TMobTable * pTable = &t;
		auto ptr = std::make_unique <TMobTable>();
		*ptr = t;
		m_NonPlayerDataMap[t.dwVnum].mobTable = std::move(ptr);
		m_NonPlayerDataMap[t.dwVnum].isSet = false;
		m_NonPlayerDataMap[t.dwVnum].isFiltered = false;
		m_NonPlayerDataMap[t.dwVnum].dropList.clear();
#else
		m_NonPlayerDataMap.emplace(t.dwVnum, t);
#endif
	}

	return true;
}

bool CPythonNonPlayer::GetName(uint32_t dwVnum, const char** c_pszName)
{
	const TMobTable* p = GetTable(dwVnum);

	if (!p)
		return false;

	*c_pszName = p->szLocaleName;

	return true;
}

bool CPythonNonPlayer::GetInstanceType(uint32_t dwVnum, uint8_t* pbType)
{
	const TMobTable* p = GetTable(dwVnum);

	// dwVnum를 찾을 수 없으면 플레이어 캐릭터로 간주 한다. 문제성 코드 -_- [cronan]
	if (!p)
		return false;

	*pbType = p->bType;

	return true;
}

const CPythonNonPlayer::TMobTable* CPythonNonPlayer::GetTable(uint32_t dwVnum)
{
	if (auto itor = m_NonPlayerDataMap.find(dwVnum); itor != m_NonPlayerDataMap.end())
#ifdef ENABLE_INGAME_WIKI
		return itor->second.mobTable.get();
#else
		return &itor->second;
#endif

	return nullptr;
}

uint8_t CPythonNonPlayer::GetEventType(uint32_t dwVnum)
{
	const TMobTable* p = GetTable(dwVnum);

	if (!p)
	{
		//Tracef("CPythonNonPlayer::GetEventType - Failed to find virtual number\n");
		return ON_CLICK_EVENT_NONE;
	}

	return p->bOnClickType;
}

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
bool CPythonNonPlayer::IsAggressive(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return false;

	return (IS_SET(c_pTable->dwAIFlag, AIFLAG_AGGRESSIVE));
}
#endif

uint8_t CPythonNonPlayer::GetEventTypeByVID(uint32_t dwVID)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);

	if (nullptr == pInstance)
	{
		//Tracef("CPythonNonPlayer::GetEventTypeByVID - There is no Virtual Number\n");
		return ON_CLICK_EVENT_NONE;
	}

	uint16_t dwVnum = pInstance->GetVirtualNumber();
	return GetEventType(dwVnum);
}

const char* CPythonNonPlayer::GetMonsterName(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return "";

	return c_pTable->szLocaleName;
}

uint32_t CPythonNonPlayer::GetMonsterColor(uint32_t dwVnum)
{
	const TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwMonsterColor;
}

uint32_t CPythonNonPlayer::GetMobAIFlag(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwAIFlag;
}

uint32_t CPythonNonPlayer::GetMobLevel(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bLevel;
}

uint32_t CPythonNonPlayer::GetMobRank(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRank;
}

uint32_t CPythonNonPlayer::GetMobType(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bType;
}

#ifdef ENABLE_ELEMENT_ADD
uint8_t CPythonNonPlayer::GetMonsterEnchantElement(uint32_t dwVnum, uint8_t bElement)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	if ((bElement < EMobElements::MOB_ELEMENTAL_ELEC) || (bElement >= EMobElements::MOB_ELEMENTAL_MAX_NUM))
		return 0;

	switch (bElement)
	{
		case EMobElements::MOB_ELEMENTAL_ELEC:
			return c_pTable->cElementalFlags[EMobElements::MOB_ELEMENTAL_ELEC];
		case EMobElements::MOB_ELEMENTAL_FIRE:
			return c_pTable->cElementalFlags[EMobElements::MOB_ELEMENTAL_FIRE];
		case EMobElements::MOB_ELEMENTAL_ICE:
			return c_pTable->cElementalFlags[EMobElements::MOB_ELEMENTAL_ICE];
		case EMobElements::MOB_ELEMENTAL_WIND:
			return c_pTable->cElementalFlags[EMobElements::MOB_ELEMENTAL_WIND];
		case EMobElements::MOB_ELEMENTAL_EARTH:
			return c_pTable->cElementalFlags[EMobElements::MOB_ELEMENTAL_EARTH];
		case EMobElements::MOB_ELEMENTAL_DARK:
			return c_pTable->cElementalFlags[EMobElements::MOB_ELEMENTAL_DARK];
		default:
			break;
	}

	return 0;
}
#endif

#ifdef ENABLE_SCALE_SYSTEM
uint8_t CPythonNonPlayer::GetMonsterScalePercent(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 100;

	return MINMAX(50, c_pTable->bScale, 200);
}
#endif

#ifdef ENABLE_NEW_DISTANCE_CALC
float CPythonNonPlayer::GetMonsterHitRange(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 70.0f;

	if (c_pTable->fHitRange)
		return c_pTable->fHitRange;

	return 100.0f;
}
#endif

uint8_t CPythonNonPlayer::GetMonsterRank(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRank;
}

#ifdef ENABLE_SEND_TARGET_INFO
uint32_t CPythonNonPlayer::GetRaceNumByVID(uint32_t iVID)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(iVID);
	if (!pInstance)
		return 0;

	const CPythonNonPlayer::TMobTable* pMobTable = GetTable(pInstance->GetVirtualNumber());
	if (!pMobTable)
		return 0;

	return pMobTable->dwVnum;
}

uint32_t CPythonNonPlayer::GetMonsterMaxHP(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwMaxHP;
}

uint32_t CPythonNonPlayer::GetMonsterRaceFlag(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwRaceFlag;
}

uint32_t CPythonNonPlayer::GetMonsterLevel(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bLevel;
}

uint32_t CPythonNonPlayer::GetMonsterDamage1(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwDamageRange[0];
}

uint32_t CPythonNonPlayer::GetMonsterDamage2(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwDamageRange[1];
}

uint32_t CPythonNonPlayer::GetMonsterExp(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwExp;
}

float CPythonNonPlayer::GetMonsterDamageMultiply(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0.0f;

	return c_pTable->fDamMultiply;
}

uint32_t CPythonNonPlayer::GetMonsterST(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bStr;
}

uint32_t CPythonNonPlayer::GetMonsterDX(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bDex;
}

bool CPythonNonPlayer::IsMonsterStone(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return false;

	if (c_pTable->bType == 2)
		return true;

	return false;
}

uint8_t CPythonNonPlayer::GetMobRegenCycle(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRegenCycle;
}

uint8_t CPythonNonPlayer::GetMobRegenPercent(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->bRegenPercent;
}

uint32_t CPythonNonPlayer::GetMobGoldMin(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwGoldMin;
}

uint32_t CPythonNonPlayer::GetMobGoldMax(uint32_t dwVnum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	return c_pTable->dwGoldMax;
}

uint32_t CPythonNonPlayer::GetMobResist(uint32_t dwVnum, uint8_t bResistNum)
{
	const CPythonNonPlayer::TMobTable* c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return 0;

	if (bResistNum >= MOB_RESISTS_MAX_NUM)
		return 0;

	return c_pTable->cResists[bResistNum];
}
#endif

bool CPythonNonPlayer::IsImmuneFlagByVnum(uint32_t dwVnum, uint32_t dwImmuneFlag)
{
	const CPythonNonPlayer::TMobTable * c_pTable = GetTable(dwVnum);
	if (!c_pTable)
		return false;

	return IS_SET(c_pTable->dwImmuneFlag, dwImmuneFlag) != 0;
}

#ifdef ENABLE_INGAME_WIKI
#include "../GameLib/RaceManager.h"
bool CPythonNonPlayer::CanRenderMonsterModel(uint32_t dwMonsterVnum)
{
	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(dwMonsterVnum, &pRaceData, false))
		return false;

	return true;
}

size_t CPythonNonPlayer::WikiLoadClassMobs(uint8_t bType, uint16_t fromLvl, uint16_t toLvl)
{
	m_vecTempMob.clear();
	for (auto it = m_NonPlayerDataMap.begin(); it != m_NonPlayerDataMap.end(); ++it)
	{
		if (!it->second.isFiltered && it->second.mobTable->bLevel >= fromLvl &&
			it->second.mobTable->bLevel < toLvl && CanRenderMonsterModel(it->second.mobTable->dwVnum))
		{
			if (bType == 0 && it->second.mobTable->bType == MONSTER && it->second.mobTable->bRank >= 4)
				m_vecTempMob.emplace_back(it->first);
			else if (bType == 1 && it->second.mobTable->bType == MONSTER && it->second.mobTable->bRank < 4)
				m_vecTempMob.emplace_back(it->first);
			else if (bType == 2 && it->second.mobTable->bType == STONE)
				m_vecTempMob.emplace_back(it->first);
		}
	}

	return m_vecTempMob.size();
}

void CPythonNonPlayer::WikiSetBlacklisted(uint32_t vnum)
{
	auto it = m_NonPlayerDataMap.find(vnum);
		if (it != m_NonPlayerDataMap.end())
			it->second.isFiltered = true;
}

std::tuple<const char*, int> CPythonNonPlayer::GetMonsterDataByNamePart(const char* namePart)
{
	char searchName[CHARACTER_NAME_MAX_LEN + 1];
	memcpy(searchName, namePart, sizeof(searchName));
	for (int j = 0; j < sizeof(searchName); j++)
		searchName[j] = tolower(searchName[j]);
	std::string tempSearchName = searchName;

	TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin();
	for (; itor != m_NonPlayerDataMap.end(); ++itor)
	{
		TMobTable* pMobTable = itor->second.mobTable.get();

		if (itor->second.isFiltered)
			continue;

		char mobName[CHARACTER_NAME_MAX_LEN + 1];
		memcpy(mobName, pMobTable->szLocaleName, sizeof(mobName));
		for (int j = 0; j < sizeof(mobName); j++)
			mobName[j] = tolower(mobName[j]);
		std::string tempName = mobName;

		const size_t tempSearchNameLenght = tempSearchName.length();
		if (tempName.length() < tempSearchNameLenght)
			continue;

		if (!tempName.substr(0, tempSearchNameLenght).compare(tempSearchName))
			return std::make_tuple(pMobTable->szLocaleName, pMobTable->dwVnum);
	}

	return std::make_tuple("", -1);
}

void CPythonNonPlayer::BuildWikiSearchList()
{
	m_vecWikiNameSort.clear();
	for (auto it = m_NonPlayerDataMap.begin(); it != m_NonPlayerDataMap.end(); ++it)
		if (!it->second.isFiltered)
			m_vecWikiNameSort.emplace_back(it->second.mobTable.get());

	SortMobDataName();
}

void CPythonNonPlayer::SortMobDataName()
{
	std::qsort(&m_vecWikiNameSort[0], m_vecWikiNameSort.size(), sizeof(m_vecWikiNameSort[0]), [](const void* a, const void* b)
	{
		TMobTable* pItem1 = *(TMobTable**)(static_cast<const TMobTable*>(a));
		std::string stRealName1 = pItem1->szLocaleName;
		std::transform(stRealName1.begin(), stRealName1.end(), stRealName1.begin(), ::tolower);

		TMobTable* pItem2 = *(TMobTable**)(static_cast<const TMobTable*>(b));
		std::string stRealName2 = pItem2->szLocaleName;
		std::transform(stRealName2.begin(), stRealName2.end(), stRealName2.begin(), ::tolower);

		int iSmallLen = std::min(stRealName1.length(), stRealName2.length());
		int iRetCompare = stRealName1.compare(0, iSmallLen, stRealName2, 0, iSmallLen);

		if (iRetCompare != 0)
			return iRetCompare;

		if (stRealName1.length() < stRealName2.length())
			return -1;
		else if (stRealName2.length() < stRealName1.length())
			return 1;

		return 0;
	});
}

CPythonNonPlayer::TWikiInfoTable* CPythonNonPlayer::GetWikiTable(uint32_t dwVnum)
{
	TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.find(dwVnum);

	if (itor == m_NonPlayerDataMap.end())
		return nullptr;

	return &(itor->second);
}
#endif

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
void CPythonNonPlayer::AddPortalLevelLimit(uint32_t dwRace, int iMinLevel, int iMaxLevel)
{
	if (iMinLevel > 0 && iMaxLevel > 0)
		m_NonPlayerPortalLevelLimit[dwRace] = std::make_pair(iMinLevel, iMaxLevel);
}

const std::pair<int, int>* CPythonNonPlayer::GetPortalLevelLimit(uint32_t dwRace) const
{
	auto it = m_NonPlayerPortalLevelLimit.find(dwRace);
	if (it != m_NonPlayerPortalLevelLimit.end())
		return &it->second;

	return nullptr;
}
#endif

void CPythonNonPlayer::Clear() const {}

void CPythonNonPlayer::Destroy()
{
/*
	for (TNonPlayerDataMap::iterator itor=m_NonPlayerDataMap.begin(); itor!=m_NonPlayerDataMap.end(); ++itor)
	{
#ifdef ENABLE_INGAME_WIKI
		m_NonPlayerDataMap.erase(itor);
#else
		delete itor->second;
#endif
	}
*/
	m_NonPlayerDataMap.clear();
}

CPythonNonPlayer::CPythonNonPlayer()
{
	Clear();
}

CPythonNonPlayer::~CPythonNonPlayer()
{
	Destroy();
}