#ifndef __INC_METIN_II_BATTLE_FIELD_H__
#define __INC_METIN_II_BATTLE_FIELD_H__

#include "../../common/CommonDefines.h"

#ifdef ENABLE_BATTLE_FIELD
#include "packet.h"
#include "questmanager.h"

enum
{
	BATTLE_FIELD_MAP_CHANNEL = 99,	// map channel
	BATTLE_FIELD_MAP_INDEX = 357,	// map index
	MIN_LEVEL_FOR_ENTER = 50,		// level min
	TIME_BETWEEN_KILLS = BATTLE_FIELD_KILL_TIME,		// seconds
	TIME_BACK_COOLDOWN = 600,		// seconds
};

enum
{
	BATTLEFIELD_DISABLED,
	BATTLEFIELD_ACTIVE,
	BATTLEFIELD_OPEN,
	BATTLEFIELD_CLOSED,
};

/* Open Time Info Struct */
typedef struct SOpenTimeInfo
{
	uint8_t bDay;
	uint8_t bOpenHour;
	uint8_t bOpenMinute;
	uint8_t bCloseHour;
	uint8_t bCloseMinute;
} TOpenTimeInfo;

/* Ranking Update Struct */
typedef struct SRankingUpdate
{
	uint8_t bDay;
	uint8_t bHour;
	uint8_t bMinute;
	uint8_t bSecond;
} TRankingUpdate;

class CBattleField : public singleton<CBattleField>
{
/*
* Standard functions
*/
public:
	CBattleField() noexcept;
	~CBattleField();

	bool Initialize();
	void Destroy();

	bool IsBattleZoneMapIndex(int iMapIndex) const noexcept { return iMapIndex == BATTLE_FIELD_MAP_INDEX; };

	void PlayerKill(LPCHARACTER pChar, LPCHARACTER pKiller);
	void RewardKiller(const LPCHARACTER pKiller, const LPCHARACTER& pChar);
	void RewardVictim(LPCHARACTER pChar);

	void Connect(LPCHARACTER pChar);
	void RestartAtRandomPos(LPCHARACTER pChar);
	PIXEL_POSITION GetRandomPosition() noexcept;

	void SetPlayerCooldown(uint32_t dwPlayerID);
	//void RemovePlayerFromVector(uint32_t dwPlayerId);

	void OpenBattleUI(LPCHARACTER pChar);

//private:
//	std::vector<uint32_t> vecCharacter{};

/*
* Ranking functions
*/
public:
	void LoadRankingUpdateTimeInfo();

	void UpdateWeekRanking();
	void SetWeakRankingPosition(LPCHARACTER pChar);
	void RegisterBattleRanking(LPCHARACTER& pChar, int iTempPoints);

private:
	std::vector<SRankingUpdate> vecRankingUpdateInfo{};

/*
* Open time functions
*/
public:
	void LoadOpenTimeInfo();

	int GetOpenTime() noexcept;
	int GetCloseTime() noexcept;

	void Update();
//private:
public:
	std::vector<TOpenTimeInfo> vecOpenTimeInfo{};

/*
* Enter functions
*/
public:
	uint8_t GetStatus() { return static_cast<uint8_t>(quest::CQuestManager::Instance().GetEventFlag("battlefield_status")); };
	void SetStatus(uint8_t bFlag) { quest::CQuestManager::Instance().RequestSetEventFlag("battlefield_status", bFlag); }

	bool GetEnterStatus() const { return quest::CQuestManager::Instance().GetEventFlag("battlefield_status") == BATTLEFIELD_OPEN ? true : false; };

	void SetEventStatus(bool bFlag) noexcept { bEventStatus = bFlag; };
	bool GetEventStatus() const noexcept { return bEventStatus; };

	void SetEventInfo(uint8_t bMonth, uint8_t bDay) noexcept { bEventMonth = bMonth; bEventDay = bDay; };
	uint8_t GetEventMonth() const noexcept { return bEventMonth; };
	uint8_t GetEventDay() const noexcept { return bEventDay; };

	void SetForceOpen(bool isForceOpen) noexcept { bIsForceOpen = isForceOpen; };
	bool IsForceOpen() noexcept { return bIsForceOpen; };

	void OpenEnter(bool isEvent = false, bool isForce = false);
	void CloseEnter();

	void ExitCharacter(LPCHARACTER pChar);
	void RequestEnter(LPCHARACTER pChar);
	void RequestExit(LPCHARACTER pChar);
private:
	bool bIsForceOpen{ false };
	bool bEventStatus{ false };
	uint8_t bEventMonth{ 0 };
	uint8_t bEventDay{ 0 };
};

#endif
#endif
