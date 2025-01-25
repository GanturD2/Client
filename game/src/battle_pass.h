#pragma once

#ifndef __INC_METIN_II_BATTLE_PASS_H__
#define __INC_METIN_II_BATTLE_PASS_H__

#ifdef ENABLE_BATTLE_PASS_SYSTEM
#include "../../common/stl.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "group_text_parse_tree.h"

#include "packet.h"

class CBattlePassManager : public singleton<CBattlePassManager>
{
public:
	CBattlePassManager();
	virtual ~CBattlePassManager();

	bool InitializeBattlePass();

	bool ReadNormalBattlePassGroup();
	bool ReadNormalBattlePassMissions();
	bool ReadPremiumBattlePassGroup();
	bool ReadPremiumBattlePassMissions();
	bool ReadEventBattlePassGroup();
	bool ReadEventBattlePassMissions();

	void CheckBattlePassTimes();
	uint8_t GetMissionIndex(uint8_t bBattlePassType, uint8_t bMissionType, uint32_t dwCondition);
	uint8_t GetMissionTypeByName(std::string stMissionName);
	uint8_t GetMissionTypeByIndex(uint8_t bBattlePassType, uint32_t dwIndex);
	std::string GetMissionNameByType(uint8_t bType);

	std::string GetNormalBattlePassNameByID(uint8_t bID);
	std::string GetPremiumBattlePassNameByID(uint8_t bID);
	std::string GetEventBattlePassNameByID(uint8_t bID);

	void GetMissionSearchName(uint8_t bMissionType, std::string*, std::string*);

	void BattlePassRequestOpen(LPCHARACTER pkChar, bool bAfterOpen = true);
	void BattlePassRewardMission(LPCHARACTER pkChar, uint8_t bBattlePassType, uint32_t bBattlePassId, uint8_t bMissionIndex);
	bool BattlePassMissionGetInfo(uint8_t bBattlePassType, uint8_t bMissionIndex, uint8_t bBattlePassId, uint8_t bMissionType, uint32_t* dwFirstInfo, uint32_t* dwSecondInfo);

	void BattlePassRequestReward(LPCHARACTER pkChar, uint8_t bBattlePassType);
	void BattlePassReward(LPCHARACTER pkChar, uint8_t bBattlePassType, uint8_t bBattlePassID);

	uint32_t	GetNormalBattlePassID() const { return m_dwActiveNormalBattlePassID; }
	uint32_t	GetPremiumBattlePassID() const { return m_dwActivePremiumBattlePassID; }
	uint32_t	GetEventBattlePassID() const { return m_dwActiveEventBattlePassID; }

	uint32_t	GetNormalBattlePassStartTime() const { return m_dwNormalBattlePassStartTime; }
	uint32_t	GetNormalBattlePassEndTime() const { return m_dwNormalBattlePassEndTime; }
	uint32_t	GetPremiumBattlePassStartTime() const { return m_dwPremiumBattlePassStartTime; }
	uint32_t	GetPremiumBattlePassEndTime() const { return m_dwPremiumBattlePassEndTime; }
	uint32_t	GetEventBattlePassStartTime() const { return m_dwEventBattlePassStartTime; }
	uint32_t	GetEventBattlePassEndTime() const { return m_dwEventBattlePassEndTime; }

	void SetBattlePassID(uint32_t bState, uint8_t bType) { m_dwActiveBattlePassID[bType] = bState; }
	void SetBattlePassStartTime(uint32_t start_time, uint8_t bType) { m_dwBattlePassStartTime[bType] = start_time; }
	void SetBattlePassEndTime(uint32_t start_time, uint8_t bType) { m_dwBattlePassEndTime[bType] = start_time; }

	uint32_t GetNormalBattlePassID(uint8_t bType) { return m_dwActiveBattlePassID[bType]; }
	uint32_t GetNormalBattlePassStartTime(uint8_t bType) { return m_dwBattlePassStartTime[bType]; }
	uint32_t GetNormalBattlePassEndTime(uint8_t bType) { return m_dwBattlePassEndTime[bType]; }

protected:
	uint32_t	m_dwNormalBattlePassStartTime;
	uint32_t	m_dwNormalBattlePassEndTime;
	uint32_t	m_dwPremiumBattlePassStartTime;
	uint32_t	m_dwPremiumBattlePassEndTime;
	uint32_t	m_dwEventBattlePassStartTime;
	uint32_t	m_dwEventBattlePassEndTime;

	uint32_t	m_dwActiveNormalBattlePassID;
	uint32_t	m_dwActivePremiumBattlePassID;
	uint32_t	m_dwActiveEventBattlePassID;

	uint32_t m_dwActiveBattlePassID[3];
	uint32_t m_dwBattlePassStartTime[3];
	uint32_t m_dwBattlePassEndTime[3];

private:
	CGroupTextParseTreeLoader* m_pNormalLoader;
	CGroupTextParseTreeLoader* m_pPremiumLoader;
	CGroupTextParseTreeLoader* m_pEventLoader;

	typedef std::map <uint8_t, std::string> TMapNormalBattlePassName;
	typedef std::map <std::string, std::vector<TExtBattlePassRewardItem>> TNormalMapBattlePassReward;
	typedef std::map <std::string, std::vector<TExtBattlePassMissionInfo>> TNormalMapBattleMissionInfo;

	TMapNormalBattlePassName		m_map_normal_battle_pass_name;
	TNormalMapBattlePassReward		m_map_normal_battle_pass_reward;
	TNormalMapBattleMissionInfo		m_map_normal_battle_pass_mission_info;

	typedef std::map <uint8_t, std::string> TMapPremiumBattlePassName;
	typedef std::map <std::string, std::vector<TExtBattlePassRewardItem>> TPremiumMapBattlePassReward;
	typedef std::map <std::string, std::vector<TExtBattlePassMissionInfo>> TPremiumMapBattleMissionInfo;

	TMapPremiumBattlePassName		m_map_premium_battle_pass_name;
	TPremiumMapBattlePassReward	m_map_premium_battle_pass_reward;
	TPremiumMapBattleMissionInfo	m_map_premium_battle_pass_mission_info;

	typedef std::map <uint8_t, std::string> TMapEventBattlePassName;
	typedef std::map <std::string, std::vector<TExtBattlePassRewardItem>> TEventMapBattlePassReward;
	typedef std::map <std::string, std::vector<TExtBattlePassMissionInfo>> TEventMapBattleMissionInfo;

	TMapEventBattlePassName		m_map_event_battle_pass_name;
	TEventMapBattlePassReward		m_map_event_battle_pass_reward;
	TEventMapBattleMissionInfo		m_map_event_battle_pass_mission_info;

	typedef std::map <uint8_t, std::vector<TExtBattlePassTimeTable>> TMapNormalBattlePassTimeTable;
	typedef std::map <uint8_t, std::vector<TExtBattlePassTimeTable>> TMapPremiumBattlePassTimeTable;
	typedef std::map <uint8_t, std::vector<TExtBattlePassTimeTable>> TMapEventBattlePassTimeTable;
	TMapNormalBattlePassTimeTable		m_map_normal_battle_pass_times;
	TMapPremiumBattlePassTimeTable	m_map_premium_battle_pass_times;
	TMapEventBattlePassTimeTable		m_map_event_battle_pass_times;
};
#endif
#endif
