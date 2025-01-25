#include "StdAfx.h"

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#include "PythonDungeonInfo.h"
#include "PythonNetworkStream.h"
#include "AbstractPlayer.h"

CPythonDungeonInfo::CPythonDungeonInfo()
{
	Clear();
}

CPythonDungeonInfo::~CPythonDungeonInfo()
{
	Clear();
}

bool CPythonDungeonInfo::AddDungeon(uint8_t byIndex, TPacketGCDungeonInfo TData)
{
	bool bCount = true;
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
	{
		m_vecDungeonInfoDataMap[byIndex].clear();
		bCount = false;
	}

	m_vecDungeonInfoDataMap[byIndex].emplace_back(TData);
	if (bCount)
		++m_byDungeonCount;

	m_vecDungeonRankingContainer.reserve(MAX_RANKING_COUNT);
	m_bIsLoaded = true;

	return true;
}

void CPythonDungeonInfo::Clear()
{
	m_bIsOpen = false;
	m_bIsLoaded = false;

	m_vecDungeonInfoDataMap->clear();
	m_byDungeonCount = 0;
	m_vecDungeonRankingContainer.clear();
}

void CPythonDungeonInfo::Open()
{
	//if (IsLoaded())
	//	return;

	m_bIsOpen = true;

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendDungeonInfo(EAction::OPEN);
}

void CPythonDungeonInfo::Warp(uint8_t byIndex)
{
	if (!IsOpen())
		return;

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendDungeonInfo(EAction::WARP, byIndex);
}

void CPythonDungeonInfo::Close()
{
	m_bIsOpen = false;

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendDungeonInfo(EAction::CLOSE);
}

uint8_t CPythonDungeonInfo::GetKey(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->byIndex;
	return 0;
}

uint8_t CPythonDungeonInfo::GetType(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->byType;
	return 0;
}

long CPythonDungeonInfo::GetMapIndex(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->lMapIndex;
	return 0;
}

long CPythonDungeonInfo::GetEntryMapIndex(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->lEntryMapIndex;
	return 0;
}

uint32_t CPythonDungeonInfo::GetBossVnum(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->dwBossVnum;
	return 0;
}

uint32_t CPythonDungeonInfo::GetLevelLimit(uint8_t byIndex, uint8_t byLimit)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return (byLimit ? it->sLevelLimit.iLevelMax : it->sLevelLimit.iLevelMin);
	return 0;
}

uint32_t CPythonDungeonInfo::GetMemberLimit(uint8_t byIndex, uint8_t byLimit)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return (byLimit ? it->sLevelLimit.iMemberMax : it->sLevelLimit.iMemberMin);
	return 0;
}

uint32_t CPythonDungeonInfo::GetRequiredItemVnum(uint8_t byIndex, uint8_t bySlot)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sRequiredItem[bySlot].dwVnum;
	return 0;
}

uint16_t CPythonDungeonInfo::GetRequiredItemCount(uint8_t byIndex, uint8_t bySlot)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sRequiredItem[bySlot].wCount;
	return 0;
}

uint32_t CPythonDungeonInfo::GetDuration(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->dwDuration;
	return 0;
}

uint32_t CPythonDungeonInfo::GetCooldown(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->dwCooldown;
	return 0;
}

uint8_t CPythonDungeonInfo::GetElement(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->byElement;
	return 0;
}

uint16_t CPythonDungeonInfo::GetAttBonusCount(uint16_t byIndex)
{
	uint16_t byCount = 0;

	if (m_vecDungeonInfoDataMap->empty())
		return byCount;

	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
	{
		for (uint16_t byAffect = CItemData::APPLY_NONE + 1; byAffect < CItemData::MAX_APPLY_NUM; ++byAffect)
		{
			if (std::find(it->sBonus.byAttBonus, std::end(it->sBonus.byAttBonus), byAffect)
				!= std::end(it->sBonus.byAttBonus))
			{
				++byCount;
			}

		}
	}
	return byCount;
}

uint16_t CPythonDungeonInfo::GetAttBonus(uint16_t byIndex, uint16_t byType)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sBonus.byAttBonus[byType];
	return 0;
}

uint16_t CPythonDungeonInfo::GetDefBonusCount(uint16_t byIndex)
{
	uint16_t byCount = 0;

	if (m_vecDungeonInfoDataMap->empty())
		return byCount;

	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
	{
		for (uint16_t byAffect = CItemData::APPLY_NONE + 1; byAffect < CItemData::MAX_APPLY_NUM; ++byAffect)
		{
			if (std::find(it->sBonus.byDefBonus, std::end(it->sBonus.byDefBonus), byAffect)
				!= std::end(it->sBonus.byDefBonus))
			{
				++byCount;
			}
		}
	}
	return byCount;
}

uint16_t CPythonDungeonInfo::GetDefBonus(uint16_t byIndex, uint16_t byType)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sBonus.byDefBonus[byType];
	return 0;
}

uint8_t CPythonDungeonInfo::GetBossDropCount(uint8_t byIndex)
{
	uint8_t byCount = 0;

	if (m_vecDungeonInfoDataMap->empty())
		return byCount;

	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
	{
		for (uint8_t bySlot = 0; bySlot < MAX_BOSS_ITEM_SLOTS; ++bySlot)
		{
			if (it->sBossDropItem[bySlot].dwVnum > 0)
				byCount++;
		}
	}
	return byCount;
}

uint32_t CPythonDungeonInfo::GetBossDropItemVnum(uint8_t byIndex, uint8_t bySlot)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sBossDropItem[bySlot].dwVnum;
	return 0;
}

uint16_t CPythonDungeonInfo::GetBossDropItemCount(uint8_t byIndex, uint8_t bySlot)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sBossDropItem[bySlot].wCount;
	return 0;
}

uint32_t CPythonDungeonInfo::GetFinish(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sResults.dwFinish;
	return 0;
}

uint32_t CPythonDungeonInfo::GetFinishTime(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sResults.dwFinishTime;
	return 0;
}

uint32_t CPythonDungeonInfo::GetFinishDamage(uint8_t byIndex)
{
	TDungeonInfoDataMap::const_iterator it = m_vecDungeonInfoDataMap[byIndex].begin();
	if (it != m_vecDungeonInfoDataMap[byIndex].end())
		return it->sResults.dwFinishDamage;
	return 0;
}

//
// Ranking
//

void CPythonDungeonInfo::AddRanking(const char* c_szName, int iLevel, uint32_t dwPoints)
{
	if (c_szName && iLevel > 0)
		m_vecDungeonRankingContainer.emplace_back(std::make_shared<SDungeonRankingData>(c_szName, iLevel, dwPoints));
}

void CPythonDungeonInfo::ClearRankingData()
{
	m_vecDungeonRankingContainer.clear();
	m_vecDungeonRankingContainer.reserve(ERank::MAX_RANKING_COUNT);
}

size_t CPythonDungeonInfo::GetRankingCount() const
{
	return m_vecDungeonRankingContainer.size();
}

CPythonDungeonInfo::SDungeonRankingData* CPythonDungeonInfo::GetRankingByLine(uint32_t dwArrayIndex) const
{
	if (dwArrayIndex >= GetRankingCount())
		return NULL;

	return m_vecDungeonRankingContainer.at(dwArrayIndex).get();
}

uint32_t CPythonDungeonInfo::GetMyRankingLine() const
{
	auto aIter = std::find_if(m_vecDungeonRankingContainer.begin(),
		m_vecDungeonRankingContainer.end(), [](const std::shared_ptr<SDungeonRankingData>& sRef)
		{
			return !sRef->strName.compare(IAbstractPlayer::GetSingleton().GetName());
		}
	);

	if (aIter != m_vecDungeonRankingContainer.end())
		return std::distance(m_vecDungeonRankingContainer.begin(), aIter) + 1;

	return 0;
}
#endif
