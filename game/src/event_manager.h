#pragma once
#include "stdafx.h"

#ifdef ENABLE_EVENT_MANAGER
#include "../../common/tables.h"
#include <unordered_map>

class CEventManager : public singleton<CEventManager>
{
public:
	typedef struct SKingdomWarCount
	{
		uint8_t bKingdom;
		uint16_t wCount;
	} TKingdomWarCount;

	typedef struct SMapPosition
	{
		uint16_t x;
		uint16_t y;
	} TMapPosition;

	typedef struct SHideAndSeek
	{
		uint32_t dwMapIndex;
		std::vector<TMapPosition>	vec_Positions;
		std::vector<std::string>	vec_Annoucments;
	} THideAndSeek;

	enum EEventTypes
	{
		EVENT_TYPE_NONE,

		// Official Events
		EVENT_TYPE_MYSTERY_BOX_DROP,
		EVENT_TYPE_E_BUFF_SPAWN,			// ENABLE_LUCKY_EVENT
		EVENT_TYPE_MINI_GAME_OKEY,			// ENABLE_MINI_GAME_OKEY_NORMAL
		EVENT_TYPE_NEW_XMAS_EVENT,
		EVENT_TYPE_MINI_GAME_YUTNORI,		// ENABLE_MINI_GAME_YUTNORI
		EVENT_TYPE_ATTENDANCE,				// ENABLE_MONSTER_BACK
		EVENT_TYPE_E_MONSTERBACK,			// ENABLE_MONSTER_BACK
		EVENT_TYPE_EASTER_DROP,				// ENABLE_EASTER_EVENT
		EVENT_TYPE_E_SUMMER_EVENT,			// ENABLE_SUMMER_EVENT
		EVENT_TYPE_RAMADAN_DROP,			// ENABLE_2017_RAMADAN
		EVENT_TYPE_HALLOWEEN_BOX,			// ENABLE_NEW_HALLOWEEN_EVENT
		EVENT_TYPE_SOUL_EVENT,				// ENABLE_SOUL_SYSTEM
		EVENT_TYPE_FOOTBALL_DROP,
		EVENT_TYPE_MEDAL_PART_DROP,
		EVENT_TYPE_VALENTINE_DROP,			// ENABLE_2016_VALENTINE
		EVENT_TYPE_FISH_EVENT,				// ENABLE_FISH_EVENT
		EVENT_TYPE_E_FLOWER_DROP,			// ENABLE_FLOWER_EVENT
		EVENT_TYPE_MINI_GAME_CATCHKING,		// ENABLE_MINI_GAME_CATCH_KING
		EVENT_TYPE_MD_START,				// ENABLE_GEM_SYSTEM
		EVENT_TYPE_MINI_GAME_FINDM,			// ENABLE_MINI_GAME_FINDM
		EVENT_TYPE_E_LATE_SUMMER,			// ENABLE_SUMMER_EVENT_ROULETTE
		EVENT_TYPE_MINI_GAME_BNW,			// ENABLE_MINI_GAME_BNW
		EVENT_TYPE_WORLD_BOSS,				// ENABLE_WORLD_BOSS
		EVENT_TYPE_BATTLE_ROYALE,			// ENABLE_BATTLE_ROYALE
		EVENT_TYPE_METINSTONE_RAIN_EVENT,	// ENABLE_METINSTONE_RAIN_EVENT

		// Custom
		EVENT_TYPE_EXPERIENCE,
		EVENT_TYPE_ITEM_DROP,
		EVENT_TYPE_SUPER_METIN,
		EVENT_TYPE_BOSS,
		EVENT_TYPE_OX,
		EVENT_TYPE_MANWOO, // fish
		EVENT_TYPE_MINING,
		EVENT_TYPE_BUDOKAN,
		EVENT_TYPE_SUNGZI_WAR,
		// Custom drop events
		EVENT_TYPE_MOONLIGHT,
		EVENT_TYPE_HEXEGONAL_CHEST,
		EVENT_TYPE_HUNT_YOUR_MOUNT,

		// Unused
		EVENT_TYPE_GOLD_FROG,
		EVENT_TYPE_TANAKA,
		EVENT_TYPE_HIDE_AND_SEEK,

		EVENT_TYPE_SIEGE_WAR,
		EVENT_TYPE_KINGDOM_WAR,

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		EVENT_BATTLE_PASS_EVENT,
		EVENT_BATTLE_PASS_NORMAL,
		EVENT_BATTLE_PASS_PREMIUM,
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		EVENT_LOTTERY,
#endif

		EVENT_MAX_NUM
	};

	enum EEvent
	{
		CLEAR_ENTITY_STAGE1_ROUND = 1,
		CLEAR_ENTITY_STAGE2_ROUND = 6,

		DYNAMIC_CYCLE_STAGE1 = 5,
		DYNAMIC_CYCLE_STAGE2 = 10,

		STATIC_CYCLE_MAX_ROUND = 3,

		TANAKA_EVENT_MAP = 63,
		SIEGE_MAP_INDEX = 103,
		KINGDOM_WAR_MAP_INDEX = 103,
		EVENT_MAP_INDEX = 230,

		OX_START_WAITING_TIME = 5,
		OX_ROUND_COUNT = 3,
		OX_PLAYER_WIN_COUNT = 5,
		OX_REWARD_VNUM = 50109,
		OX_REWARD_COUNT = 1,

		SIEGE_SHINSOO = 0,
		SIEGE_CHUNJO = 1,
		SIEGE_JINNO = 2,
		SIEGE_EMPIRE_COUNT = 3,

		HIDE_AND_SEEK_ROUNDS = 5,
		HIDE_AND_SEEK_POSITIONS = 4,
		HIDE_AND_SEEK_NPC = 20359,
		HIDE_AND_SEEK_REWARD_VNUM = 50109,
		HIDE_AND_SEEK_REWARD_COUNT = 5,

		EVENT_CHANNEL = 99,
		HIDE_AND_SEEK_CHANNEL = 1,
	};

	enum EQueueType
	{
		QUEUE_TYPE_NONE,
		QUEUE_TYPE_START,
		QUEUE_TYPE_END,
	};

	enum EDropTypes
	{
		DROP_TYPE_NONE = 1,
		DROP_TYPE_GENERAL,
		DROP_TYPE_BOSS_AND_MORE,
		DROP_TYPE_BOSS,
		DROP_TYPE_STONE,
		DROP_TYPE_MAX
	};

	CEventManager();
	~CEventManager();

	void Process();
	void Enqueue(uint8_t bQueue, TEventTable* table);
	void Dequeue(uint8_t bQueue, uint32_t dwID);
	void CancelActiveEvents();

	void Initialize(TEventTable* pTab, int iSize, bool bReload = false);
	void UpdateEventStatus(SEventTable* eventTable);

	uint8_t GetEvent(const std::string& strEventName);
	std::string	GetEventName(int iEvent);
	std::string GetEventString(int iEvent);
	void SendEventInfo(const LPCHARACTER& ch, int iMonth);

	void BuildEventNameMap();
	bool GetEventState(int iEvent);
	void SetEventState(TEventTable* table, bool bState);
	void UpdateGameFlag(const char* c_szFlag, long lValue);

	void SetReloadMode(bool bReload) noexcept { m_bReload = bReload; }
	bool IsReloadMode() noexcept { return m_bReload; }

	void OnDead(LPCHARACTER pVictim, LPCHARACTER pKiller);

	// Official Events
	bool SetMisteryBoxDropEvent(bool bActive, const TEventTable* pTable);
#ifdef ENABLE_LUCKY_EVENT
	bool SetBuffSpawnEvent(bool bActive);
#endif
	bool SetMiniGameOkeyEvent(bool bActive, const TEventTable* pTable); // ENABLE_MINI_GAME_OKEY_NORMAL
	bool SetNewXmasEvent(bool bActive, const TEventTable* pTable);
	bool SetMiniGameYutnoriEvent(bool bActive, const TEventTable* pTable); // ENABLE_MINI_GAME_YUTNORI
#ifdef ENABLE_MONSTER_BACK
	bool SetAttendanceEvent(bool bActive);
	bool SetMonsterBackEvent(bool bActive);
#endif
#ifdef ENABLE_EASTER_EVENT
	bool SetEasterEvent(bool bActive, const TEventTable* table);
#endif
	bool SetSummerEvent(bool bActive, const TEventTable* table); // ENABLE_SUMMER_EVENT
	bool SetRamadanEvent(bool bActive, const TEventTable* table); // ENABLE_2017_RAMADAN
	bool SetHalloweenBoxEvent(bool bActive, const TEventTable* table); // ENABLE_NEW_HALLOWEEN_EVENT
#ifdef ENABLE_SOUL_SYSTEM
	bool SetSoulEvent(bool bActive);
#endif
	bool SetFootballEvent(bool bActive, const TEventTable* table);
	bool SetMedalPartEvent(bool bActive, const TEventTable* table);
#ifdef ENABLE_2016_VALENTINE
	bool SetValentineEvent(bool bActive, const TEventTable* table);
#endif
#ifdef ENABLE_FISH_EVENT
	bool SetFishEvent(bool bActive);
#endif
#ifdef ENABLE_FLOWER_EVENT
	bool SetFlowerEvent(bool bActive, const TEventTable* table);
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	bool SetMiniGameCatchKingEvent(bool bActive, const TEventTable* table);
#endif
#ifdef ENABLE_GEM_SYSTEM
	bool SetMDStartEvent(bool bActive);
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	bool SetMiniGameFindMEvent(bool bActive, const TEventTable* table);
#endif
	bool SetLateSummerEvent(bool bActive); // ENABLE_SUMMER_EVENT_ROULETTE
#ifdef ENABLE_MINI_GAME_BNW
	bool SetMiniGameBNWEvent(bool bActive, const TEventTable* table);
#endif
	bool SetWorldBossEvent(bool bActive);
	bool SetBattleRoyaleEvent(bool bActive); // ENABLE_BATTLE_ROYALE
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	bool SetMetinstoneRainEvent(bool bActive);
#endif

	// Custom
	bool SetExperienceEvent(bool bActive, const TEventTable* table);
	bool SetItemDropEvent(bool bActive, TEventTable* table);
	bool SetSuperMetinEvent(bool bActive);
	bool SetBossEvent(bool bActive);
	bool SetOXEvent(bool bActive);
	bool SetManwooEvent(bool bActive); // fish
	bool SetMiningEvent(bool bActive);
	bool SetBudokanEvent(bool bActive);
	bool SetSungziWarEvent(bool bActive);

	// Custom drop events
	bool SetMoonlightEvent(bool bActive, const TEventTable* table);
	bool SetHexegonalEvent(bool bActive, const TEventTable* table);
	bool SetHuntYourMountEvent(bool bActive, const TEventTable* table);

	// Unused
	bool SetGoldFrogEvent(bool bActive);
	bool SetTanakaEvent(bool bActive, TEventTable* table);
	bool SetHideAndSeekEvent(bool bActive);
	bool HideAndSeekNPC(int iPosition, int iRound);
	void SetHideAndSeekRound(int iRound) { m_iCurrentHideAndSeekRound = iRound; }
	int GetHideAndSeekRound() { return m_iCurrentHideAndSeekRound; }

	bool	SetSiegeWarEvent(bool bActive);

	bool	SetKingdomWarEvent(bool bActive);
	void	SendKingdomWarScorePacket(LPCHARACTER ch);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	bool SetBattlePassEvent(bool bActive, const TEventTable* pTable, uint8_t pass_id);
	bool BattlePassData(const TEventTable* table, uint8_t bType, bool bState);
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	bool SetLotteryEvent(bool bActive);
#endif

private:
	std::unordered_map<std::string, int> m_mapEventName;
	std::unordered_map<uint32_t, TEventTable*> m_mapEvent;

	std::deque<TEventTable*> m_dequeEventStart;
	std::deque<TEventTable*> m_dequeEventEnd;

	bool m_bReload;

	LPEVENT		m_pOXEvent;

	uint8_t		m_bSiegeScore;

	TKingdomWarCount m_aKingdomWarScore[EMPIRE_MAX_NUM];

	LPCHARACTER m_pHideAndSeekNPC;
	int m_iCurrentHideAndSeekRound;
};
#endif
