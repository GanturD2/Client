#pragma once

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#include "Packet.h"

class CPythonDungeonInfo : public CSingleton<CPythonDungeonInfo>
{
public:
	typedef std::vector<TPacketGCDungeonInfo> TDungeonInfoDataMap;

	enum EAction
	{
		CLOSE,
		OPEN,
		WARP,
		RANK
	};

	enum ERank
	{
		MAX_RANKING_LINES = 5,
		MAX_RANKING_COUNT = 100,
	};

	enum ERankTypes
	{
		RANKING_TYPE_COMPLETED = 1,
		RANKING_TYPE_TIME = 2,
		RANKING_TYPE_DAMAGE = 3,

		MAX_RANKING_TYPE,
	};

	struct SDungeonRankingData
	{
		SDungeonRankingData(const char* c_szName, int iLevel, uint32_t dwPoints)
			: strName(c_szName), iLevel(iLevel), dwPoints(dwPoints) {}
		std::string strName;
		int iLevel;
		uint32_t dwPoints;
	};
	std::vector<std::shared_ptr<SDungeonRankingData>> m_vecDungeonRankingContainer;

	CPythonDungeonInfo();
	virtual ~CPythonDungeonInfo();

	bool AddDungeon(uint8_t byIndex, TPacketGCDungeonInfo TData);
	void Clear();

	bool IsLoaded() { return m_bIsLoaded; }
	void Unload() { m_bIsLoaded = false; }

	bool IsOpen() { return m_bIsOpen; }
	void Open();
	void Warp(uint8_t byIndex);
	void Close();

	uint8_t GetCount() { return m_byDungeonCount; }

	uint8_t GetKey(uint8_t byIndex);
	uint8_t GetType(uint8_t byIndex);

	long GetMapIndex(uint8_t byIndex);
	long GetEntryMapIndex(uint8_t byIndex);

	uint32_t GetBossVnum(uint8_t byIndex);

	// Limits
	uint32_t GetLevelLimit(uint8_t byIndex, uint8_t byLimit = 0);
	uint32_t GetMemberLimit(uint8_t byIndex, uint8_t byLimit = 0);

	// Required Item
	uint32_t GetRequiredItemVnum(uint8_t byIndex, uint8_t bySlot);
	uint16_t GetRequiredItemCount(uint8_t byIndex, uint8_t bySlot);

	uint32_t GetDuration(uint8_t byIndex);
	uint32_t GetCooldown(uint8_t byIndex);
	uint8_t GetElement(uint8_t byIndex);

	// Attack Bonus
	uint16_t GetAttBonusCount(uint16_t byIndex);
	uint16_t GetAttBonus(uint16_t byIndex, uint16_t byType);

	// Defense Bonus
	uint16_t GetDefBonusCount(uint16_t byIndex);
	uint16_t GetDefBonus(uint16_t byIndex, uint16_t byType);

	// Boss Drop Item
	uint8_t GetBossDropCount(uint8_t byIndex);
	uint32_t GetBossDropItemVnum(uint8_t byIndex, uint8_t bySlot);
	uint16_t GetBossDropItemCount(uint8_t byIndex, uint8_t bySlot);

	// Results
	uint32_t GetFinish(uint8_t byIndex);
	uint32_t GetFinishTime(uint8_t byIndex);
	uint32_t GetFinishDamage(uint8_t byIndex);

	void AddRanking(const char* c_szName, int iLevel, uint32_t dwPoints);
	void ClearRankingData();

	size_t GetRankingCount() const;
	SDungeonRankingData* GetRankingByLine(uint32_t dwArrayIndex) const;
	uint32_t GetMyRankingLine() const;

protected:
	TDungeonInfoDataMap m_vecDungeonInfoDataMap[255];
	uint8_t m_byDungeonCount;
	bool m_bIsOpen;
	bool m_bIsLoaded;
};
#endif
