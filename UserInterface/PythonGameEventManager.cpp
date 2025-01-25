#include "StdAfx.h"

#ifdef ENABLE_EVENT_MANAGER
#include "PythonGameEventManager.h"
#include <time.h>

InGameEventManager::InGameEventManager()
{
	m_bIsRequestEventDataed = false;
	m_map_EventReward.clear();
	m_mapEventName.clear();

	BuildEventMap();
	BuildEventNameMap();
}

InGameEventManager::~InGameEventManager()
{
}

bool InGameEventManager::LoadEventRewardList(const char* c_szFileName)
{
	if (!m_map_EventReward.empty())
		return true;

	CTextFileLoader TextFileLoader;
	if (!TextFileLoader.Load(c_szFileName))
		return false;

	TextFileLoader.SetTop();

	for (uint32_t i = 0; i < TextFileLoader.GetChildNodeCount(); ++i)
	{
		if (!TextFileLoader.SetChildNode(i))
			continue;

		std::string strName;

		if (!TextFileLoader.GetCurrentNodeName(&strName))
			continue;

		TTokenVector* pTok;
		for (int k = 1; k <= REWARD_MAX_NUM; ++k)
		{
			if (TextFileLoader.GetTokenVector(std::to_string(k), &pTok))
			{
				if (pTok->size() == 2)
				{
					TEventReward e{};
					e.dwVnum = atoi(pTok->at(0).c_str());
					e.dwCount = atoi(pTok->at(1).c_str());

					if (e.dwVnum && e.dwCount)
					{
						std::transform(strName.begin(), strName.end(), strName.begin(), toupper);
						const int iEvent = GetEvent(strName);
						if (iEvent)
							m_map_EventReward[iEvent].emplace_back(e);
					}
				}
			}
		}
		TextFileLoader.SetParentNode();
	}
	return true;
}

void InGameEventManager::BuildEventMap()
{
	m_map_Event.clear();

	for (int i = 1; i <= DAY_MAX_NUM; i++)
		m_map_Event[i].clear();
}

void InGameEventManager::BuildEventNameMap()
{
	m_mapEventName["EVENT_TYPE_NONE"] = EVENT_TYPE_NONE;
	// Official Events
	m_mapEventName["EVENT_TYPE_MYSTERY_BOX_DROP"] = EVENT_TYPE_MYSTERY_BOX_DROP;
	m_mapEventName["EVENT_TYPE_E_BUFF_SPAWN"] = EVENT_TYPE_E_BUFF_SPAWN;					// ENABLE_LUCKY_EVENT	[Beeshido event]
	m_mapEventName["EVENT_TYPE_MINI_GAME_OKEY"] = EVENT_TYPE_MINI_GAME_OKEY;				// ENABLE_MINI_GAME_OKEY_NORMAL
	m_mapEventName["EVENT_TYPE_NEW_XMAS_EVENT"] = EVENT_TYPE_NEW_XMAS_EVENT;
	m_mapEventName["EVENT_TYPE_MINI_GAME_YUTNORI"] = EVENT_TYPE_MINI_GAME_YUTNORI;			// ENABLE_MINI_GAME_YUTNORI
	m_mapEventName["EVENT_TYPE_ATTENDANCE"] = EVENT_TYPE_ATTENDANCE;						// ENABLE_MONSTER_BACK	[attendance]
	m_mapEventName["EVENT_TYPE_E_MONSTERBACK"] = EVENT_TYPE_E_MONSTERBACK;					// ENABLE_MONSTER_BACK	[MonsterHunt]
	m_mapEventName["EVENT_TYPE_EASTER_DROP"] = EVENT_TYPE_EASTER_DROP;						// ENABLE_EASTER_EVENT
	m_mapEventName["EVENT_TYPE_E_SUMMER_EVENT"] = EVENT_TYPE_E_SUMMER_EVENT;				// ENABLE_SUMMER_EVENT
	m_mapEventName["EVENT_TYPE_RAMADAN_DROP"] = EVENT_TYPE_RAMADAN_DROP;					// ENABLE_2017_RAMADAN
	m_mapEventName["EVENT_TYPE_HALLOWEEN_BOX"] = EVENT_TYPE_HALLOWEEN_BOX;					// ENABLE_NEW_HALLOWEEN_EVENT
	m_mapEventName["EVENT_TYPE_SOUL_EVENT"] = EVENT_TYPE_SOUL_EVENT;						// ENABLE_SOUL_SYSTEM
	m_mapEventName["EVENT_TYPE_FOOTBALL_DROP"] = EVENT_TYPE_FOOTBALL_DROP;
	m_mapEventName["EVENT_TYPE_MEDAL_PART_DROP"] = EVENT_TYPE_MEDAL_PART_DROP;
	m_mapEventName["EVENT_TYPE_VALENTINE_DROP"] = EVENT_TYPE_VALENTINE_DROP;				// ENABLE_2016_VALENTINE
	m_mapEventName["EVENT_TYPE_FISH_EVENT"] = EVENT_TYPE_FISH_EVENT;						// ENABLE_FISH_EVENT	[JigSaw]
	m_mapEventName["EVENT_TYPE_E_FLOWER_DROP"] = EVENT_TYPE_E_FLOWER_DROP;					// ENABLE_FLOWER_EVENT
	m_mapEventName["EVENT_TYPE_MINI_GAME_CATCHKING"] = EVENT_TYPE_MINI_GAME_CATCHKING;		// ENABLE_MINI_GAME_CATCH_KING
	m_mapEventName["EVENT_TYPE_MD_START"] = EVENT_TYPE_MD_START;							// ENABLE_GEM_SYSTEM
	m_mapEventName["EVENT_TYPE_MINI_GAME_FINDM"] = EVENT_TYPE_MINI_GAME_FINDM;				// ENABLE_MINI_GAME_FINDM
	m_mapEventName["EVENT_TYPE_E_LATE_SUMMER"] = EVENT_TYPE_E_LATE_SUMMER;					// ENABLE_SUMMER_EVENT_ROULETTE
	m_mapEventName["EVENT_TYPE_MINI_GAME_BNW"] = EVENT_TYPE_MINI_GAME_BNW;					// ENABLE_MINI_GAME_BNW
	m_mapEventName["EVENT_TYPE_WORLD_BOSS"] = EVENT_TYPE_WORLD_BOSS;
	m_mapEventName["EVENT_TYPE_BATTLE_ROYALE"] = EVENT_TYPE_BATTLE_ROYALE;					// ENABLE_BATTLE_ROYALE
	m_mapEventName["EVENT_TYPE_METINSTONE_RAIN_EVENT"] = EVENT_TYPE_METINSTONE_RAIN_EVENT;	// ENABLE_METINSTONE_RAIN_EVENT

	// Custom
	m_mapEventName["EVENT_TYPE_EXPERIENCE"] = EVENT_TYPE_EXPERIENCE;
	m_mapEventName["EVENT_TYPE_ITEM_DROP"] = EVENT_TYPE_ITEM_DROP;
	m_mapEventName["EVENT_TYPE_SUPER_METIN"] = EVENT_TYPE_SUPER_METIN;
	m_mapEventName["EVENT_TYPE_BOSS"] = EVENT_TYPE_BOSS;
	m_mapEventName["EVENT_TYPE_OX"] = EVENT_TYPE_OX;
	m_mapEventName["EVENT_TYPE_MANWOO"] = EVENT_TYPE_MANWOO;
	m_mapEventName["EVENT_TYPE_MINING"] = EVENT_TYPE_MINING;
	m_mapEventName["EVENT_TYPE_BUDOKAN"] = EVENT_TYPE_BUDOKAN;
	m_mapEventName["EVENT_TYPE_SUNGZI_WAR"] = EVENT_TYPE_SUNGZI_WAR;

	// Custom drop events
	m_mapEventName["EVENT_TYPE_MOONLIGHT"] = EVENT_TYPE_MOONLIGHT;
	m_mapEventName["EVENT_TYPE_HEXEGONAL_CHEST"] = EVENT_TYPE_HEXEGONAL_CHEST;
	m_mapEventName["EVENT_TYPE_HUNT_YOUR_MOUNT"] = EVENT_TYPE_HUNT_YOUR_MOUNT;

	// Unused
	m_mapEventName["EVENT_TYPE_GOLD_FROG"] = EVENT_TYPE_GOLD_FROG;
	m_mapEventName["EVENT_TYPE_TANAKA"] = EVENT_TYPE_TANAKA;
	m_mapEventName["EVENT_TYPE_HIDE_AND_SEEK"] = EVENT_TYPE_HIDE_AND_SEEK;

	m_mapEventName["EVENT_TYPE_SIEGE_WAR"] = EVENT_TYPE_SIEGE_WAR;
	m_mapEventName["EVENT_TYPE_KINGDOM_WAR"] = EVENT_TYPE_KINGDOM_WAR;

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	m_mapEventName["EVENT_BATTLE_PASS_EVENT"] = EVENT_BATTLE_PASS_EVENT;
	m_mapEventName["EVENT_BATTLE_PASS_NORMAL"] = EVENT_BATTLE_PASS_NORMAL;
	m_mapEventName["EVENT_BATTLE_PASS_PREMIUM"] = EVENT_BATTLE_PASS_PREMIUM;
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	m_mapEventName["EVENT_LOTTERY"] = EVENT_LOTTERY;
#endif
}

void InGameEventManager::AddEventData(std::vector<TEventTable>& eventVec)
{
	BuildEventMap();

	for (std::vector<TEventTable>::const_iterator itor = eventVec.begin(); itor < eventVec.end(); ++itor)
	{
		// Sort event based on start time[@day]
		const TEventTable eventTable = *itor;
		const tm* t = localtime(&eventTable.startTime);

		m_map_Event[t->tm_mday].emplace_back(eventTable);
	}
}

int InGameEventManager::GetEvent(const std::string strEventName)
{
	if (m_mapEventName.find(strEventName) != m_mapEventName.end())
		return m_mapEventName[strEventName];

	return EVENT_TYPE_NONE;
}

int InGameEventManager::GetDayEventCount(int iDay)
{
	if (iDay > DAY_MAX_NUM)
		return 0;

	return m_map_Event[iDay].size();
}

void InGameEventManager::GetEventData(int eventIdx, int iDay, TEventTable& table)
{
	if (iDay <= DAY_MAX_NUM)
	{
		EventVector vec = m_map_Event[iDay];
		if (eventIdx < vec.size())
		{
			const TEventTable t = vec.at(eventIdx);
			memcpy(&table, &t, sizeof(TEventTable));
		}
	}
}

void InGameEventManager::GetEventRewardData(int eventIdx, int pos, TEventReward& reward)
{
	const auto it = m_map_EventReward.find(eventIdx);
	if (it != m_map_EventReward.end())
	{
		auto& rewardVec = it->second;

		if (pos < rewardVec.size())
		{
			reward.dwVnum = rewardVec.at(pos).dwVnum;
			reward.dwCount = rewardVec.at(pos).dwCount;
			return;
		}
	}
}
#endif
