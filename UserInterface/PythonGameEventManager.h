#pragma once

#ifdef ENABLE_EVENT_MANAGER
#include <unordered_map>

class InGameEventManager : public CSingleton<InGameEventManager>
{
public:
	struct TEventTable
	{
		uint32_t dwID;
		uint8_t bType;
		long startTime;
		long endTime;
		uint32_t dwVnum;
		int iPercent;
		int iDropType;
		uint8_t bCompleted;
	};

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
		EVENT_TYPE_WORLD_BOSS,
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

	enum
	{
		MONTH_MAX_NUM = 11,
		DAY_MAX_NUM = 31,
		REWARD_MAX_NUM = 3,
	};

	InGameEventManager();
	~InGameEventManager();

	typedef struct SEventReward
	{
		uint32_t dwVnum;
		uint32_t dwCount;
	} TEventReward;

	bool LoadEventRewardList(const char* c_szFileName);

	void BuildEventMap();
	void BuildEventNameMap();
	void AddEventData(std::vector<TEventTable>& eventVec);

	int GetEvent(const std::string strEventName);

	void SetRequestEventData(bool isRequested) noexcept { m_bIsRequestEventDataed = isRequested; };
	bool GetRequestEventData() noexcept { return m_bIsRequestEventDataed; };

	int GetDayEventCount(int iDay);
	void GetEventData(int eventIdx, int iDay, TEventTable& table);
	void GetEventRewardData(int eventIdx, int pos, TEventReward& reward);
private:
	typedef std::vector<TEventTable> EventVector;
	typedef std::map<int, EventVector> EventMap;

	EventVector m_vec_EventData;
	EventMap m_map_Event;

	std::unordered_map<int, std::vector<TEventReward>> m_map_EventReward;

	std::unordered_map<std::string, int> m_mapEventName;

	bool m_bIsRequestEventDataed;
};
#endif
