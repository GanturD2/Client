#include "stdafx.h"

#ifdef ENABLE_EVENT_MANAGER
#include "event.h"
#include "event_manager.h"
#include "text_file_loader.h"
#include "locale_service.h"
#include "quest.h"
#include "questmanager.h"
#include "priv_manager.h"
#include "sectree_manager.h"
#include "start_position.h"
#include "char.h"
#include "regen.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "utils.h"
#include "buffer_manager.h"
#include "p2p.h"
#include "minigame_manager.h"
#include "OXEvent.h"
#include "char_manager.h"
#ifdef ENABLE_BATTLE_PASS_SYSTEM
# include "battle_pass.h"
#endif

#include <algorithm>

using namespace quest;

static const CEventManager::THideAndSeek aNPCPosition[CEventManager::HIDE_AND_SEEK_POSITIONS] =
{
	/*
		Map Index
		Vector of positions
		Vector of hint annoucement strings
	*/

	{
		61,
		{ {238, 1427}, {819, 121}, {1425, 1011}, {1043, 693}},
		{ {"Mount Sohan", "Mount Sohan 2", "Mount Sohan 3", "Mount Sohan 4"} },
	},

	{
		62,
		{ {1044, 980}, {798, 806}, {1032, 668}, {805, 1168}},
		{ {"Fireland", "Fireland 2", "Fireland 3", "Fireland 4"} },
	},

	{
		63,
		{ {821, 787}, {435, 703}, {698, 428}, {1485, 692}},
		{ {"Sahara Desert", "Sahara Desert 2", "Sahara Desert 3", "Sahara Desert 4"} },
	},

	{
		64,
		{ {184, 121}, {1225, 279}, {130, 1219}, {1438, 1208}},
		{ {"Valley of Seungryong", "Valley of Seungryong 2", "Valley of Seungryong 3", "Valley of Seungryong 4"} },
	},
};

bool SortStartQueue(const TEventTable* a, const TEventTable* b)
{
	return a->startTime < b->startTime;
}

bool SortEndQueue(const TEventTable* a, const TEventTable* b)
{
	return a->endTime < b->endTime;
}

bool SortKingdomWar(const CEventManager::TKingdomWarCount& a, const CEventManager::TKingdomWarCount& b)
{
	return a.wCount < b.wCount;
}

CEventManager::CEventManager()
{
	m_mapEventName.clear();
	m_mapEvent.clear();
	m_dequeEventStart.clear();
	m_dequeEventEnd.clear();

	BuildEventNameMap();

	m_bReload = false;

	m_pOXEvent = nullptr;

	m_bSiegeScore = 0;

	memset(m_aKingdomWarScore, 0, sizeof(m_aKingdomWarScore));

	m_pHideAndSeekNPC = nullptr;
	m_iCurrentHideAndSeekRound = -1;
}

CEventManager::~CEventManager()
{
	for (auto& kv : m_mapEvent)
		M2_DELETE(kv.second);

	m_mapEvent.clear();
	m_dequeEventStart.clear();
	m_dequeEventEnd.clear();
}

struct FSendKingdomWarScore
{
	FSendKingdomWarScore()
	{
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;

			if (ch->IsPC())
				CEventManager::Instance().SendKingdomWarScorePacket(ch);
		}
	}
};

EVENTINFO(warp_all_to_village_event_info)
{
	uint32_t dwWarpMapIndex;
	uint8_t	bEmpire;

	warp_all_to_village_event_info() noexcept
		: dwWarpMapIndex(0), bEmpire(0)
	{
	}
};

struct FWarpAllToVillage
{
	uint8_t m_bEmpire;
	FWarpAllToVillage() noexcept
	{
		m_bEmpire = 0;
	}

	void operator()(LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (ch && ch->IsPC())
			{
				const uint8_t bEmpire = ch->GetEmpire();
				if (bEmpire == 0)
				{
					sys_err("Unkonwn Empire %s %d ", ch->GetName(), ch->GetPlayerID());
					return;
				}

				if (m_bEmpire)
				{
					if (m_bEmpire == bEmpire)
						ch->WarpSet(g_start_position[bEmpire][0], g_start_position[bEmpire][1]);
				}
				else
					ch->WarpSet(g_start_position[bEmpire][0], g_start_position[bEmpire][1]);
			}
		}
	}
};

EVENTFUNC(warp_all_to_village_event)
{
	const warp_all_to_village_event_info* info = dynamic_cast<warp_all_to_village_event_info*>(event->info);

	if (!info)
	{
		sys_err("warp_all_to_village_event> <Factor> Null pointer");
		return 0;
	}

	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(info->dwWarpMapIndex);

	if (pSecMap)
	{
		FWarpAllToVillage f;
		f.m_bEmpire = info->bEmpire;
		pSecMap->for_each(f);
	}

	return 0;
}

struct FKillSectree
{
	void operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (ch->IsMonster() || ch->IsStone() || mining::IsVeinOfOre(ch->GetRaceNum()))
				ch->Dead();
		}
	}
};

EVENTINFO(dynamic_spawn_cycle_event_info)
{
	uint32_t dwWarpMapIndex;
	std::string strRegenPath;
	std::string strRegenPath2;
	uint8_t step;

	dynamic_spawn_cycle_event_info()
		: dwWarpMapIndex(0),
		strRegenPath(""),
		strRegenPath2(""),
		step(0)
	{
	}
};

EVENTFUNC(dynamic_spawn_cycle_event)
{
	dynamic_spawn_cycle_event_info* info = dynamic_cast<dynamic_spawn_cycle_event_info*>(event->info);

	if (!info)
	{
		sys_err("dynamic_spawn_cycle_event <Factor> Null pointer to event");
		return 0;
	}

	if (info->dwWarpMapIndex == 0 || info->strRegenPath == "" || info->strRegenPath2 == "")
	{
		sys_err("dynamic_spawn_cycle_event: Null pointer to event info");
		return 0;
	}

	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(info->dwWarpMapIndex);

	if (pSecMap)
	{
		info->step += 1;

		/*
			Clear each entity on the map on the beggining of 1st and 6th round.
		*/
		if (info->step == CEventManager::CLEAR_ENTITY_STAGE1_ROUND ||
			info->step == CEventManager::CLEAR_ENTITY_STAGE2_ROUND)
		{
			FKillSectree f;
			pSecMap->for_each(f);
		}

		/*
			Round 1~5 -> Stage 1
			Round 6~10 -> Stage 2
			Exit after round 10
		*/
		if (info->step <= CEventManager::DYNAMIC_CYCLE_STAGE1)
		{
			regen_load_in_file(info->strRegenPath.c_str(), info->dwWarpMapIndex, pSecMap->m_setting.iBaseX, pSecMap->m_setting.iBaseY);
		}
		else if (info->step > CEventManager::DYNAMIC_CYCLE_STAGE1 &&
			info->step <= CEventManager::DYNAMIC_CYCLE_STAGE2)
		{
			regen_load_in_file(info->strRegenPath2.c_str(), info->dwWarpMapIndex, pSecMap->m_setting.iBaseX, pSecMap->m_setting.iBaseY);
		}
		else if (info->step > CEventManager::DYNAMIC_CYCLE_STAGE2)
			return 0;
	}

	return PASSES_PER_SEC(5 * 60);
}

EVENTINFO(static_spawn_cycle_event_info)
{
	uint32_t dwWarpMapIndex;
	std::string strRegenPath;
	uint8_t step;

	static_spawn_cycle_event_info()
		: dwWarpMapIndex(0),
		strRegenPath(""),
		step(0)
	{
	}
};

EVENTFUNC(static_spawn_cycle_event)
{
	static_spawn_cycle_event_info* info = dynamic_cast<static_spawn_cycle_event_info*>(event->info);

	if (!info)
	{
		sys_err("static_spawn_cycle_event <Factor> Null pointer to event");
		return 0;
	}

	if (info->dwWarpMapIndex == 0 || info->strRegenPath == "")
	{
		sys_err("static_spawn_cycle_event: Null pointer to event info");
		return 0;
	}

	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(info->dwWarpMapIndex);

	if (pSecMap)
	{
		info->step += 1;

		/*
			Clear each entity on the map on the beggining of 1st.
			Do not clear map during Tanaka Event.
		*/
		if (info->step == CEventManager::CLEAR_ENTITY_STAGE1_ROUND &&
			info->dwWarpMapIndex != CEventManager::TANAKA_EVENT_MAP)
		{
			FKillSectree f;
			pSecMap->for_each(f);
		}

		/*
			Round 1~3 -> Stage 1
			Exit after round 3
		*/
		if (info->step <= CEventManager::STATIC_CYCLE_MAX_ROUND)
		{
			regen_load_in_file(info->strRegenPath.c_str(), info->dwWarpMapIndex, pSecMap->m_setting.iBaseX, pSecMap->m_setting.iBaseY);
		}
		else
			return 0;
	}

	return PASSES_PER_SEC(8 * 60);
}

/*
	Execeute event state at both queues for the 
	first element in list. First element's time is always the
	closest one to the current time. (Sorted by time at initialization)

	Send an announcement before the start of the event.
*/
void CEventManager::Process()
{
	if (IsReloadMode())
		return;

	if (g_bChannel == EVENT_CHANNEL)
	{
		if (!m_dequeEventStart.empty())
		{
			TEventTable* pStartEvent = m_dequeEventStart.front();
			if (pStartEvent->startTime <= get_global_time())
			{

				SetEventState(pStartEvent, true);
				Dequeue(QUEUE_TYPE_START, pStartEvent->dwID);
			}

			// Send an announcment for the 5 closest upcoming events
			for (int i = 0; i < 5; ++i)
			{
				if (i >= m_dequeEventStart.size())
					break;

				TEventTable* pStartEvent = m_dequeEventStart.at(i);
				time_t lSecondsLeft = pStartEvent->startTime - get_global_time();

				if (lSecondsLeft <= 3600)
				{
					char szBuf[256 + 1];
					switch (lSecondsLeft)
					{
						case 3600:
							snprintf(szBuf, sizeof(szBuf), "%s will be starting in 1 hour.", GetEventName(GetEvent(pStartEvent->szType)).c_str());
							BroadcastNotice(szBuf);
							break;

						case 2700:
							snprintf(szBuf, sizeof(szBuf), "%s will be starting in 45 minutes.", GetEventName(GetEvent(pStartEvent->szType)).c_str());
							BroadcastNotice(szBuf);
							break;

						case 1800:
							snprintf(szBuf, sizeof(szBuf), "%s will be starting in 30 minutes.", GetEventName(GetEvent(pStartEvent->szType)).c_str());
							BroadcastNotice(szBuf);
							break;

						case 900:
							snprintf(szBuf, sizeof(szBuf), "%s will be starting in 15 minutes.", GetEventName(GetEvent(pStartEvent->szType)).c_str());
							BroadcastNotice(szBuf);
							break;

						case 300:
							snprintf(szBuf, sizeof(szBuf), "%s will be starting in 5 minutes.", GetEventName(GetEvent(pStartEvent->szType)).c_str());
							BroadcastNotice(szBuf);
							break;
					}
				}

			}
		}

		if (!m_dequeEventEnd.empty())
		{
			TEventTable* pEndEvent = m_dequeEventEnd.front();
			if (pEndEvent->endTime <= get_global_time())
			{
				SetEventState(pEndEvent, false);
				Dequeue(QUEUE_TYPE_END, pEndEvent->dwID);
			}
		}
	}
}

/*
	Function will be called at the initialization of events.
	Each game event will be added to start-event queue and
	end-event queue (dependant on bQueue value).
*/
void CEventManager::Enqueue(uint8_t bQueue, TEventTable* table)
{
	if (bQueue == QUEUE_TYPE_START)
	{
		if (table->startTime <= get_global_time())
			SetEventState(table, true);
		else
			m_dequeEventStart.push_back(table);
	}
	else if (bQueue == QUEUE_TYPE_END)
	{
		m_dequeEventEnd.push_back(table);
	}
}

/*
	Events are dequeued from their respective deque on start/end.
	At the end of an event, a packet is sent to database to update
	event completion status. Database then sents a packet back
	to update information on every game core.
*/
void CEventManager::Dequeue(uint8_t bQueue, uint32_t dwID)
{
	if (bQueue == QUEUE_TYPE_START)
	{
		m_dequeEventStart.pop_front();
	}
	else if (bQueue == QUEUE_TYPE_END)
	{
		m_dequeEventEnd.pop_front();

		auto eventIt = m_mapEvent.find(dwID);

		if (eventIt != m_mapEvent.end())
			db_clientdesc->DBPacket(HEADER_GD_UPDATE_EVENT_STATUS, 0, &eventIt->second->dwID, sizeof(uint32_t));
	}
}

void CEventManager::CancelActiveEvents()
{
	for (auto& kv : m_mapEvent)
	{
		const uint32_t dwIndex = kv.first;
		TEventTable* pEvent = kv.second;

		bool bStartFound = false;
		for (auto it = m_dequeEventStart.begin(); it != m_dequeEventStart.end(); ++it)
		{
			TEventTable* pEventQueue = *it;
			if (pEventQueue->dwID == dwIndex)
			{
				bStartFound = true;
				break;
			}
		}

		bool bEndFound = false;
		for (auto it = m_dequeEventEnd.begin(); it != m_dequeEventEnd.end(); ++it)
		{
			TEventTable* pEventQueue = *it;
			if (pEventQueue->dwID == dwIndex)
			{
				bEndFound = true;
				break;
			}
		}

		/*
			If an event is only present in the end queue, it means it has already started.
			Event must be ended to reset the game flag for future auto-start.
		*/
		if (!bStartFound && bEndFound)
		{
			int iEvent = GetEvent(kv.second->szType);

			SetEventState(kv.second, false);
		}
	}

	m_mapEvent.clear();
	m_dequeEventStart.clear();
	m_dequeEventEnd.clear();
}

void CEventManager::Initialize(TEventTable* pTab, int iSize, bool bReload /* = false */)
{
	if (g_bChannel == EVENT_CHANNEL)
		regen_free_map(EVENT_MAP_INDEX);

	if (bReload)
	{
		SetReloadMode(true);
		CancelActiveEvents();
	}

	for (int i = 0; i < iSize; ++i, ++pTab)
	{
		if (!GetEvent(pTab->szType))
		{
			sys_err("Skipping event table id %d, unknown event type %s", pTab->dwID, pTab->szType);
			continue;
		}

		TEventTable* t = M2_NEW TEventTable;
		t->dwID = pTab->dwID;
		strlcpy(t->szType, pTab->szType, sizeof(t->szType));
		t->startTime = pTab->startTime;
		t->endTime = pTab->endTime;
		t->dwVnum = pTab->dwVnum;
		t->iPercent = pTab->iPercent;
		t->iDropType = pTab->iDropType;
		t->bCompleted = pTab->bCompleted;

		m_mapEvent.emplace(std::make_pair(t->dwID, t));

		// Event queues are initialized on only one specific game channel
		if (g_bChannel == EVENT_CHANNEL && !t->bCompleted)
		{
			// Add event to the start queue
			Enqueue(QUEUE_TYPE_START, t);

			// Add event to the end queue
			Enqueue(QUEUE_TYPE_END, t);
		}
	}

	std::sort(m_dequeEventStart.begin(), m_dequeEventStart.end(), SortStartQueue);
	std::sort(m_dequeEventEnd.begin(), m_dequeEventEnd.end(), SortEndQueue);

	// Send a packet to reload event board on client-side incase it was a reload
	if ((g_bChannel == EVENT_CHANNEL) && bReload)
		ReloadEvent();

	SetReloadMode(false);
}

void CEventManager::UpdateEventStatus(SEventTable* eventTable)
{
	const auto eventIt = m_mapEvent.find(eventTable->dwID);

	if (eventIt != m_mapEvent.end())
		eventIt->second->bCompleted = true;
}

uint8_t CEventManager::GetEvent(const std::string& strEventName)
{
	if (m_mapEventName.find(strEventName) != m_mapEventName.end())
		return static_cast<uint8_t>(m_mapEventName[strEventName]);

	return EVENT_TYPE_NONE;
}

std::string CEventManager::GetEventName(int iEvent)
{
	const static std::map<int, std::string> mapEventName =
	{
		{ EVENT_TYPE_EXPERIENCE,		"Experience Event"},
		{ EVENT_TYPE_ITEM_DROP,			"Item - Drop Boost Event"},
		{ EVENT_TYPE_BOSS,				"Boss Event"},
		{ EVENT_TYPE_MINING,			"Mining Event"},
		{ EVENT_TYPE_GOLD_FROG,			"Golden Frog Event"},
		{ EVENT_TYPE_MOONLIGHT,			"Moonlight Chest Event"},
		{ EVENT_TYPE_HEXEGONAL_CHEST,	"Hexegonal Box Event"},
		{ EVENT_TYPE_HIDE_AND_SEEK,		"Hide and Seek Event" },
		{ EVENT_TYPE_OX,				"OX Event" },
		{ EVENT_TYPE_TANAKA,			"Tanka Event" },
		{ EVENT_TYPE_SIEGE_WAR,			"Siege War Event" },
		{ EVENT_TYPE_KINGDOM_WAR,		"Kingdom War Event" },
	};

	return mapEventName.at(iEvent);
}

std::string CEventManager::GetEventString(int iEvent)
{
	for (auto& kv : m_mapEventName)
	{
		if (kv.second == iEvent)
			return kv.first;
	}

	return "EVENT_TYPE_NONE";
}

void CEventManager::SendEventInfo(const LPCHARACTER& ch, int iMonth)
{
	if (!ch)
		return;

	TEMP_BUFFER buf;

	for (const auto& kv : m_mapEvent)
	{
		const time_t startTime = kv.second->startTime;
		const struct tm* tStart = localtime(&startTime);

		// Send current month's events and events from previous months which are still active
		if (((tStart->tm_mon < iMonth) && !kv.second->bCompleted) || (tStart->tm_mon == iMonth))
		{
			TPacketEventData eventData;
			eventData.dwID = kv.second->dwID;
			eventData.bType = GetEvent(kv.second->szType);
			eventData.startTime = kv.second->startTime;
			eventData.endTime = kv.second->endTime;
			eventData.dwVnum = kv.second->dwVnum;
			eventData.iPercent = kv.second->iPercent;
			eventData.iDropType = kv.second->iDropType;
			eventData.bCompleted = kv.second->bCompleted;

			buf.write(&eventData, sizeof(TPacketEventData));
		}
	}

	TPacketGCEventInfo p{};
	p.bHeader = HEADER_GC_EVENT_INFO;
	p.wSize = sizeof(TPacketGCEventInfo) + buf.size();

	ch->GetDesc()->Packet(&p, sizeof(TPacketGCEventInfo));

	if (buf.size())
		ch->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void CEventManager::BuildEventNameMap()
{
	m_mapEventName["EVENT_TYPE_NONE"] = EVENT_TYPE_NONE;
	// Official Events
	m_mapEventName["EVENT_TYPE_MYSTERY_BOX_DROP"] = EVENT_TYPE_MYSTERY_BOX_DROP;
	m_mapEventName["EVENT_TYPE_E_BUFF_SPAWN"] = EVENT_TYPE_E_BUFF_SPAWN;					// ENABLE_LUCKY_EVENT	[Beeshido event]
	m_mapEventName["EVENT_TYPE_MINI_GAME_OKEY"] = EVENT_TYPE_MINI_GAME_OKEY;				// ENABLE_MINI_GAME_OKEY_NORMAL
	m_mapEventName["EVENT_TYPE_NEW_XMAS_EVENT"] = EVENT_TYPE_NEW_XMAS_EVENT;
	m_mapEventName["EVENT_TYPE_MINI_GAME_YUTNORI"] = EVENT_TYPE_MINI_GAME_YUTNORI;			// ENABLE_MINI_GAME_YUTNORI
	m_mapEventName["EVENT_TYPE_ATTENDANCE"] = EVENT_TYPE_ATTENDANCE;						// ENABLE_MONSTER_BACK	[Attendance]
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

bool CEventManager::GetEventState(int iEventType)
{
	switch (iEventType)
	{
		// Official Events
		case EVENT_TYPE_MYSTERY_BOX_DROP:
			return CQuestManager::Instance().GetEventFlag("mystery_box_drop_event");
			break;

		case EVENT_TYPE_E_BUFF_SPAWN:	// ENABLE_LUCKY_EVENT	[Beeshido event]
			return CQuestManager::Instance().GetEventFlag("e_buff_spawn_event");
			break;

		case EVENT_TYPE_MINI_GAME_OKEY: // ENABLE_MINI_GAME_OKEY_NORMAL
			return CQuestManager::Instance().GetEventFlag("mini_game_okey_event");
			break;

		case EVENT_TYPE_NEW_XMAS_EVENT:
			return CQuestManager::Instance().GetEventFlag("new_xmas_event_event");
			break;

		case EVENT_TYPE_MINI_GAME_YUTNORI: // ENABLE_MINI_GAME_YUTNORI
			return CQuestManager::Instance().GetEventFlag("mini_game_yutnori_event");
			break;

		case EVENT_TYPE_ATTENDANCE:	// ENABLE_MONSTER_BACK	[Attendance]
			return CQuestManager::Instance().GetEventFlag("enable_attendance_event");
			break;

		case EVENT_TYPE_E_MONSTERBACK:	// ENABLE_MONSTER_BACK[MonsterHunt]
#	ifdef ENABLE_10TH_EVENT
			return CQuestManager::Instance().GetEventFlag("e_monsterback");
#	else
			return CQuestManager::Instance().GetEventFlag("e_easter_monsterback");
#	endif
			break;

		case EVENT_TYPE_EASTER_DROP:// ENABLE_EASTER_EVENT
			return CQuestManager::Instance().GetEventFlag("enable_easter_event");
			break;

		case EVENT_TYPE_E_SUMMER_EVENT: // ENABLE_SUMMER_EVENT
			return CQuestManager::Instance().GetEventFlag("e_summer_event");
			break;

		case EVENT_TYPE_RAMADAN_DROP: // ENABLE_2017_RAMADAN
			return CQuestManager::Instance().GetEventFlag("ramadan_drop_event");
			break;

		case EVENT_TYPE_HALLOWEEN_BOX: // ENABLE_NEW_HALLOWEEN_EVENT
			return CQuestManager::Instance().GetEventFlag("halloween_box_event");
			break;

		case EVENT_TYPE_SOUL_EVENT:	// ENABLE_SOUL_SYSTEM
			return CQuestManager::Instance().GetEventFlag("soul_event");
			break;

		case EVENT_TYPE_FOOTBALL_DROP:
			return CQuestManager::Instance().GetEventFlag("football_drop_event");
			break;

		case EVENT_TYPE_MEDAL_PART_DROP:
			return CQuestManager::Instance().GetEventFlag("medal_part_drop_event");
			break;

		case EVENT_TYPE_VALENTINE_DROP:	// ENABLE_2016_VALENTINE
			return CQuestManager::Instance().GetEventFlag("valentine_drop_event");
			break;

		case EVENT_TYPE_FISH_EVENT:	// ENABLE_FISH_EVENT	[JigSaw]
			return CQuestManager::Instance().GetEventFlag("fish_event");
			break;

		case EVENT_TYPE_E_FLOWER_DROP:	// ENABLE_FLOWER_EVENT
			return CQuestManager::Instance().GetEventFlag("e_flower_drop_event");
			break;

		case EVENT_TYPE_MINI_GAME_CATCHKING:	// ENABLE_MINI_GAME_CATCH_KING
			return CQuestManager::Instance().GetEventFlag("mini_game_catchking_event");
			break;

		case EVENT_TYPE_MD_START:	// ENABLE_GEM_SYSTEM
			return CQuestManager::Instance().GetEventFlag("md_start_event");
			break;

		case EVENT_TYPE_MINI_GAME_FINDM:	// ENABLE_MINI_GAME_FINDM
			return CQuestManager::Instance().GetEventFlag("mini_game_findm_event");
			break;

		case EVENT_TYPE_E_LATE_SUMMER: // ENABLE_SUMMER_EVENT_ROULETTE
			return CQuestManager::Instance().GetEventFlag("e_late_summer_event");
			break;

		case EVENT_TYPE_MINI_GAME_BNW:	// ENABLE_MINI_GAME_BNW
			return CQuestManager::Instance().GetEventFlag("mini_game_bnw_event");
			break;

		case EVENT_TYPE_WORLD_BOSS:
			return CQuestManager::Instance().GetEventFlag("world_boss_event");
			break;

		case EVENT_TYPE_BATTLE_ROYALE: // ENABLE_BATTLE_ROYALE
			return CQuestManager::Instance().GetEventFlag("battle_royale_event");
			break;

		case EVENT_TYPE_METINSTONE_RAIN_EVENT:	// ENABLE_METINSTONE_RAIN_EVENT
			return CQuestManager::Instance().GetEventFlag("metinstone_rain_event");
			break;

		// Custom
		case EVENT_TYPE_EXPERIENCE:
			return CQuestManager::Instance().GetEventFlag("exp_event");
			break;

		case EVENT_TYPE_ITEM_DROP:
			return CQuestManager::Instance().GetEventFlag("item_drop_event");
			break;

		case EVENT_TYPE_SUPER_METIN:
			return CQuestManager::Instance().GetEventFlag("superstone_rain_event");
			break;

		case EVENT_TYPE_BOSS:
			return CQuestManager::Instance().GetEventFlag("boss_event");
			break;

		case EVENT_TYPE_OX:
			return CQuestManager::Instance().GetEventFlag("oxevent_status");
			break;

		case EVENT_TYPE_MANWOO: // fish
			return CQuestManager::Instance().GetEventFlag("manwoo_event");
			break;

		case EVENT_TYPE_MINING:
			return CQuestManager::Instance().GetEventFlag("mining_event");
			break;

		case EVENT_TYPE_BUDOKAN:
			return CQuestManager::Instance().GetEventFlag("budokan_event");
			break;

		case EVENT_TYPE_SUNGZI_WAR:
			return CQuestManager::Instance().GetEventFlag("sungzi_event");
			break;

		// Custom drop events
		case EVENT_TYPE_MOONLIGHT:
			return CQuestManager::Instance().GetEventFlag("moon_drop");
			break;

		case EVENT_TYPE_HEXEGONAL_CHEST:
			return CQuestManager::Instance().GetEventFlag("hexegonal_drop");
			break;

		case EVENT_TYPE_HUNT_YOUR_MOUNT:
			return CQuestManager::Instance().GetEventFlag("hunt_your_mount_drop");
			break;

		// Unused
		case EVENT_TYPE_GOLD_FROG:
			return CQuestManager::Instance().GetEventFlag("golden_frog_event");
			break;

		case EVENT_TYPE_TANAKA:
			return CQuestManager::Instance().GetEventFlag("tanaka_event");
			break;

		case EVENT_TYPE_HIDE_AND_SEEK:
			return CQuestManager::Instance().GetEventFlag("hide_seek_event");
			break;

		case EVENT_TYPE_SIEGE_WAR:
			return CQuestManager::instance().GetEventFlag("siege_war_event");
			break;

		case EVENT_TYPE_KINGDOM_WAR:
			return CQuestManager::instance().GetEventFlag("kingdom_war_event");
			break;

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case EVENT_BATTLE_PASS_EVENT:
			return CQuestManager::instance().GetEventFlag("battle_pass_event");
			break;

		case EVENT_BATTLE_PASS_NORMAL:
			return CQuestManager::instance().GetEventFlag("battle_pass_normal");
			break;

		case EVENT_BATTLE_PASS_PREMIUM:
			return CQuestManager::instance().GetEventFlag("battle_pass_premium");
			break;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		case EVENT_LOTTERY:
			return CQuestManager::instance().GetEventFlag("lottery_event");
			break;
#endif

		default:
			return false;
			break;
	}
}

void CEventManager::SetEventState(TEventTable* pTable, bool bState)
{
	const int iEvent = GetEvent(pTable->szType);

	/*
		Return if the event state is already set, ignore
		if it is not a reload -> game boot
	*/
	if ((GetEventState(iEvent) == bState) && !IsReloadMode())
		return;

	bool ret = false;

	switch (iEvent)
	{
		// Official Events
		case EVENT_TYPE_MYSTERY_BOX_DROP:
			ret = SetMisteryBoxDropEvent(bState, pTable);
			break;

#ifdef ENABLE_LUCKY_EVENT
		case EVENT_TYPE_E_BUFF_SPAWN:
			ret = SetBuffSpawnEvent(bState);
			break;
#endif

		case EVENT_TYPE_MINI_GAME_OKEY: // ENABLE_MINI_GAME_OKEY_NORMAL
			ret = SetMiniGameOkeyEvent(bState, pTable);
			break;

		case EVENT_TYPE_NEW_XMAS_EVENT:
			ret = SetNewXmasEvent(bState, pTable);
			break;

		case EVENT_TYPE_MINI_GAME_YUTNORI: // ENABLE_MINI_GAME_YUTNORI
			ret = SetMiniGameYutnoriEvent(bState, pTable);
			break;

#ifdef ENABLE_MONSTER_BACK
		case EVENT_TYPE_ATTENDANCE:
			ret = SetAttendanceEvent(bState);
			break;

		case EVENT_TYPE_E_MONSTERBACK:
			ret = SetMonsterBackEvent(bState);
			break;
#endif

#ifdef ENABLE_EASTER_EVENT
		case EVENT_TYPE_EASTER_DROP:
			ret = SetEasterEvent(bState, pTable);
			break;
#endif

		case EVENT_TYPE_E_SUMMER_EVENT: // ENABLE_SUMMER_EVENT
			ret = SetSummerEvent(bState, pTable);
			break;

		case EVENT_TYPE_RAMADAN_DROP: // ENABLE_2017_RAMADAN
			ret = SetRamadanEvent(bState, pTable);
			break;

		case EVENT_TYPE_HALLOWEEN_BOX: // ENABLE_NEW_HALLOWEEN_EVENT
			ret = SetHalloweenBoxEvent(bState, pTable);
			break;

#ifdef ENABLE_SOUL_SYSTEM
		case EVENT_TYPE_SOUL_EVENT:
			ret = SetSoulEvent(bState);
			break;
#endif

		case EVENT_TYPE_FOOTBALL_DROP:
			ret = SetFootballEvent(bState, pTable);
			break;

		case EVENT_TYPE_MEDAL_PART_DROP:
			ret = SetMedalPartEvent(bState, pTable);
			break;

#ifdef ENABLE_2016_VALENTINE
		case EVENT_TYPE_VALENTINE_DROP:
			ret = SetValentineEvent(bState, pTable);
			break;
#endif

#ifdef ENABLE_FISH_EVENT
		case EVENT_TYPE_FISH_EVENT: // JigSaw
			ret = SetFishEvent(bState);
			break;
#endif

#ifdef ENABLE_FLOWER_EVENT
		case EVENT_TYPE_E_FLOWER_DROP:
			ret = SetFlowerEvent(bState, pTable);
			break;
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
		case EVENT_TYPE_MINI_GAME_CATCHKING:
			ret = SetMiniGameCatchKingEvent(bState, pTable);
			break;
#endif

#ifdef ENABLE_GEM_SYSTEM
		case EVENT_TYPE_MD_START: // gem_point
			ret = SetMDStartEvent(bState);
			break;
#endif

#ifdef ENABLE_MINI_GAME_FINDM
		case EVENT_TYPE_MINI_GAME_FINDM:
			ret = SetMiniGameFindMEvent(bState, pTable);
			break;
#endif

		case EVENT_TYPE_E_LATE_SUMMER: // ENABLE_SUMMER_EVENT_ROULETTE
			ret = SetLateSummerEvent(bState);
			break;

#ifdef ENABLE_MINI_GAME_BNW
		case EVENT_TYPE_MINI_GAME_BNW:
			ret = SetMiniGameBNWEvent(bState, pTable);
			break;
#endif

		case EVENT_TYPE_WORLD_BOSS:
			ret = SetWorldBossEvent(bState);
			break;

		case EVENT_TYPE_BATTLE_ROYALE: // ENABLE_BATTLE_ROYALE
			ret = SetBattleRoyaleEvent(bState);
			break;

#ifdef ENABLE_METINSTONE_RAIN_EVENT
		case EVENT_TYPE_METINSTONE_RAIN_EVENT:
			ret = SetMetinstoneRainEvent(bState);
			break;
#endif

		// Custom
		case EVENT_TYPE_EXPERIENCE:
			ret = SetExperienceEvent(bState, pTable);
			break;

		case EVENT_TYPE_ITEM_DROP:
			ret = SetItemDropEvent(bState, pTable);
			break;

		case EVENT_TYPE_SUPER_METIN:
			ret = SetSuperMetinEvent(bState);
			break;

		case EVENT_TYPE_BOSS:
			ret = SetBossEvent(bState);
			break;

		case EVENT_TYPE_OX:
			ret = SetOXEvent(bState);
			break;

		case EVENT_TYPE_MANWOO: // fish
			ret = SetManwooEvent(bState);
			break;

		case EVENT_TYPE_MINING:
			ret = SetMiningEvent(bState);
			break;

		case EVENT_TYPE_BUDOKAN:
			ret = SetBudokanEvent(bState);
			break;

		case EVENT_TYPE_SUNGZI_WAR:
			ret = SetSungziWarEvent(bState);
			break;

		// Custom drop events
		case EVENT_TYPE_MOONLIGHT:
			ret = SetMoonlightEvent(bState, pTable);
			break;

		case EVENT_TYPE_HEXEGONAL_CHEST:
			ret = SetHexegonalEvent(bState, pTable);
			break;

		case EVENT_TYPE_HUNT_YOUR_MOUNT:
			ret = SetHuntYourMountEvent(bState, pTable);
			break;

		// Unused
		case EVENT_TYPE_GOLD_FROG:
			ret = SetGoldFrogEvent(bState);
			break;

		case EVENT_TYPE_TANAKA:
			ret = SetTanakaEvent(bState, pTable);
			break;

		case EVENT_TYPE_HIDE_AND_SEEK:
			ret = SetHideAndSeekEvent(bState);
			break;

		case EVENT_TYPE_SIEGE_WAR:
			ret = SetSiegeWarEvent(bState);
			break;

		case EVENT_TYPE_KINGDOM_WAR:
			ret = SetKingdomWarEvent(bState);
			break;

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case EVENT_BATTLE_PASS_EVENT:
			ret = SetBattlePassEvent(bState, pTable, BATTLEPASS_EVENT);
			break;

		case EVENT_BATTLE_PASS_NORMAL:
			ret = SetBattlePassEvent(bState, pTable, BATTLEPASS_NORMAL);
			break;

		case EVENT_BATTLE_PASS_PREMIUM:
			ret = SetBattlePassEvent(bState, pTable, BATTLEPASS_PREMIUM);
			break;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		case EVENT_LOTTERY:
			ret = SetLotteryEvent(bState);
			break;
#endif

		default:
			break;
	}

	if (!ret)
		sys_err("Event %s could not be started/stopped, state %d", pTable->szType, bState);
}

/*
	When reloading, game requires instant information about
	game flags. Waiting for it to retrieve from database is not an
	option so we are updating this core's game flag directly while
	also sending a packet to update db.
*/
void CEventManager::UpdateGameFlag(const char* c_szFlag, long lValue)
{
	CQuestManager::Instance().SetEventFlag(c_szFlag, (int)lValue);

	TPacketSetEventFlag p{};
	strlcpy(p.szFlagName, c_szFlag, sizeof(p.szFlagName));
	p.lValue = lValue;
	db_clientdesc->DBPacket(HEADER_GD_EVENT_NOTIFICATION, 0, &p, sizeof(TPacketSetEventFlag));

	if (lValue)
		SendEventBeginNotification();
	else
		SendEventEndNotification();
}

void CEventManager::OnDead(LPCHARACTER pVictim, LPCHARACTER pKiller)
{
	if (!pKiller || !pKiller->IsPC())
		return;

	if(GetEventState(EVENT_TYPE_KINGDOM_WAR))
	{
		++m_aKingdomWarScore[pKiller->GetEmpire()].wCount;

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(KINGDOM_WAR_MAP_INDEX);

		FSendKingdomWarScore f;
		pSecMap->for_each(f);

		/*
		// End the event on specific kill count

		if (m_aKingdomWarScore[pKiller->GetEmpire()].wCount >= 100)
			SetKingdomWarEvent(false);
		*/
	}
	else if (pVictim->IsStone())
	{
		switch (pVictim->GetRaceNum())
		{
			case 8020:
			{
				BroadcastNotice("Shinsoo Stone has been destroyed. Shinsoo players will be teleported to their villages.");
				++m_bSiegeScore;

				// Teleport players from the map
				LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(SIEGE_MAP_INDEX);

				if (pSecMap)
				{
					warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();
					info->dwWarpMapIndex = SIEGE_MAP_INDEX;
					info->bEmpire = SHINSOO_EMPIRE;

					event_create(warp_all_to_village_event, info, PASSES_PER_SEC(3));
				}

				// End the event if the 2nd Kingdom has been defeated
				if (m_bSiegeScore == 2)
				{
					// Update the flag immidiately without P2P procedure
					CQuestManager::Instance().SetEventFlag("siege_shinsoo_score", m_bSiegeScore);
					SetSiegeWarEvent(false);
				}
				else
				{
					CQuestManager::Instance().RequestSetEventFlag("siege_shinsoo_score", m_bSiegeScore);
				}

			} break;

			case 8021:
			{
				BroadcastNotice("Chunjo Stone has been destroyed. Chunjo players will be teleported to their villages.");
				++m_bSiegeScore;

				// Teleport players from the map
				LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(SIEGE_MAP_INDEX);

				if (pSecMap)
				{
					warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();
					info->dwWarpMapIndex = SIEGE_MAP_INDEX;
					info->bEmpire = CHUNJO_EMPIRE;

					event_create(warp_all_to_village_event, info, PASSES_PER_SEC(3));
				}

				// End the event if the 2nd Kingdom has been defeated
				if (m_bSiegeScore == 2)
				{
					// Additional set-event-flag is required to update the flag immidiately without P2P procedure
					CQuestManager::Instance().SetEventFlag("siege_chunjo_score", m_bSiegeScore);
					SetSiegeWarEvent(false);
				}
				else
				{
					CQuestManager::Instance().RequestSetEventFlag("siege_chunjo_score", m_bSiegeScore);
				}
			} break;

			case 8022:
			{
				BroadcastNotice("Jinno Stone has been destroyed. Jinno players will be teleported to their villages.");
				++m_bSiegeScore;

				// Teleport players from the map
				LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(SIEGE_MAP_INDEX);

				if (pSecMap)
				{
					warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();
					info->dwWarpMapIndex = SIEGE_MAP_INDEX;
					info->bEmpire = JINNO_EMPIRE;

					event_create(warp_all_to_village_event, info, PASSES_PER_SEC(3));
				}

				// End the event if the 2nd Kingdom has been defeated
				if (m_bSiegeScore == 2)
				{
					// Additional set-event-flag is required to update the flag immidiately without P2P procedure
					CQuestManager::Instance().SetEventFlag("siege_jinno_score", m_bSiegeScore);
					SetSiegeWarEvent(false);
				}
				else
				{
					CQuestManager::Instance().RequestSetEventFlag("siege_jinno_score", m_bSiegeScore);
				}
			} break;
		}
	}
}

// Official Events
bool CEventManager::SetMisteryBoxDropEvent(bool bActive, const TEventTable* pTable)
{
	UpdateGameFlag("mystery_box_drop_event", bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("e_random_drop_vnum", pTable->dwVnum);
	quest::CQuestManager::Instance().RequestSetEventFlag("e_random_drop_perc", pTable->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("e_random_drop_type", pTable->iDropType);

	if (bActive)
		BroadcastNotice("The Mistery Box event has started.");
	else
		BroadcastNotice("The Mistery Box event has ended.");

	return true;
}

#ifdef ENABLE_LUCKY_EVENT
bool CEventManager::SetBuffSpawnEvent(bool bActive)
{
	UpdateGameFlag("e_buff_spawn_event", bActive);

	if (bActive)
		BroadcastNotice("The Buff Spawn event has started.");
	else
		BroadcastNotice("The Buff Spawn event has ended.");

	return true;
}
#endif

bool CEventManager::SetMiniGameOkeyEvent(bool bActive, const TEventTable* table) // ENABLE_MINI_GAME_OKEY_NORMAL
{
	UpdateGameFlag("mini_game_okey_event", bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("okey_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("okey_event_end_day", lEndTime);

	if (bActive)
		BroadcastNotice("[1246]The Okey Event has started!");
	else
		BroadcastNotice("[1247]The Okey Event is over!");

	return true;
}

bool CEventManager::SetNewXmasEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("new_xmas_event_event", bActive);
	quest::CQuestManager::Instance().SetEventFlag("xmas_snow", bActive);
	quest::CQuestManager::Instance().RequestSetEventFlag("xmas_perc", table->iPercent);
	CMiniGameManager::Instance().InitializeEvent(XMAS_EVENT, bActive);

	if (bActive)
		BroadcastNotice("The Xmas event has started.");
	else
		BroadcastNotice("The Xmas event has ended.");

	return true;
}

bool CEventManager::SetMiniGameYutnoriEvent(bool bActive, const TEventTable* table) // ENABLE_MINI_GAME_YUTNORI
{
	UpdateGameFlag("mini_game_yutnori_event", bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("yutnori_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("yutnori_event_end_day", lEndTime);

	if (bActive)
	{
		BroadcastNotice("[287]Yut Nori has started!");
		BroadcastNotice("[288]Collect Birch Branches, craft Yut Nori Boards from them and play the exciting minigame.");
		BroadcastNotice("[289]Fantastic prizes await!");
	}
	else
		BroadcastNotice("[290]Yut Nori has finished.");

	return true;
}

#ifdef ENABLE_MONSTER_BACK
bool CEventManager::SetAttendanceEvent(bool bActive)
{
	UpdateGameFlag("enable_attendance_event", bActive);
	return true;
}

bool CEventManager::SetMonsterBackEvent(bool bActive)
{
#ifdef ENABLE_10TH_EVENT
	UpdateGameFlag("e_monsterback", bActive);
#else
	UpdateGameFlag("e_easter_monsterback", bActive);
#endif

	time_t iTime;
	time(&iTime);
	const tm* pTimeInfo = localtime(&iTime);
	quest::CQuestManager::Instance().RequestSetEventFlag("attendance_start_day", pTimeInfo->tm_yday);

	if (bActive)
	{
		BroadcastNotice("[1249]Monsters have appeared in the Valley of Seungryong, Mount Sohan, Doyyumhwan and in the Yongbi Desert!");
		BroadcastNotice("[1250]Comb through the areas and eliminate them. Gather hit points and exchange them for chests.");
	}
	else
	{
#ifdef ENABLE_10TH_EVENT
		//BroadcastNotice("[1251]The Boss Reaper Event ends in %d minute(s).");
		BroadcastNotice("[1251-X]The Boss Reaper Event has finished."); // Custom (has finished)
#else
		//BroadcastNotice("[1252]Mr B and the boss monsters disappear in %d minute(s).");
		BroadcastNotice("[1252-X]Mr B and the boss monsters has disappeared"); // Custom (has disappeared)
#endif
	}

	return true;
}
#endif

#ifdef ENABLE_EASTER_EVENT
bool CEventManager::SetEasterEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("enable_easter_event", bActive);
	CMiniGameManager::Instance().InitializeEasterEvent(bActive);
	CMiniGameManager::Instance().InitializeEvent(EASTER_EVENT, bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("easter_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice("The Easter event has started.");
	else
		BroadcastNotice("The Easter event has ended.");

	return true;
}
#endif

bool CEventManager::SetSummerEvent(bool bActive, const TEventTable* table) // ENABLE_SUMMER_EVENT
{
	UpdateGameFlag("e_summer_event", bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("e_summer_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("e_summer_event_end_day", lEndTime);

	if (bActive)
		BroadcastNotice("[59]The summer event has started. Visit Summer in town. She'll give you some tasty refreshments.");
	else
		BroadcastNotice("[60]The summer event will end soon. Summer will keep the towns supplied with tasty refreshments for 7 more days.");

	return true;
}

bool CEventManager::SetRamadanEvent(bool bActive, const TEventTable* table) // ENABLE_2017_RAMADAN
{
	UpdateGameFlag("ramadan_drop_event", bActive);
	CMiniGameManager::Instance().InitializeEvent(RAMADAN_EVENT, bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("ramadan_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice("[158]The Ramadan Event has started!");
	else
		BroadcastNotice("[159]The Ramadan Event is over.");

	return true;
}

bool CEventManager::SetHalloweenBoxEvent(bool bActive, const TEventTable* table) // ENABLE_NEW_HALLOWEEN_EVENT
{
	UpdateGameFlag("halloween_box_event", bActive);
	CMiniGameManager::Instance().InitializeEvent(HALLOWEEN_EVENT, bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("halloween_box_perc", table->iPercent);

	if (bActive)
	{
		BroadcastNotice("[1211]The Halloween Event has begun!");
		BroadcastNotice("[1212]Go and talk to Sam Pumpkin!");
	}
	else
		BroadcastNotice("[1213]The Halloween Event has finished. Don't forget to collect your rewards!");

	return true;
}

#ifdef ENABLE_SOUL_SYSTEM
bool CEventManager::SetSoulEvent(bool bActive)
{
	UpdateGameFlag("soul_event", bActive);

	if (bActive)
		BroadcastNotice("The Soul event has started.");
	else
		BroadcastNotice("The Soul event has ended.");

	return true;
}
#endif

bool CEventManager::SetFootballEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("football_drop_event", bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("football_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice("The Football event has started.");
	else
		BroadcastNotice("The Football event has ended.");

	return true;
}

bool CEventManager::SetMedalPartEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("medal_part_drop_event", bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("medal_part_drop_perc", table->iPercent);

	if (bActive)
	{
		BroadcastNotice("The Hero Event has started! Adventurers of level 30 or higher,");
		BroadcastNotice("click the 'E' button next to the minimap for more information.");
	}
	else
		BroadcastNotice("The Hero Event has ended.");

	return true;
}

#ifdef ENABLE_2016_VALENTINE
bool CEventManager::SetValentineEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("valentine_drop_event", bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("valentine_drop_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("valentine_drop_end_day", lEndTime);

	if (bActive)
		BroadcastNotice("The Valentine event has started.");
	else
		BroadcastNotice("The Valentine event has ended.");

	return true;
}
#endif

#ifdef ENABLE_FISH_EVENT
bool CEventManager::SetFishEvent(bool bActive)
{
	UpdateGameFlag("fish_event", bActive);

	if (bActive)
		BroadcastNotice("The JigSaw Fish event has started.");
	else
		BroadcastNotice("The JigSaw Fish event has ended.");

	return true;
}
#endif

#ifdef ENABLE_FLOWER_EVENT
bool CEventManager::SetFlowerEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("e_flower_drop_event", bActive);

	quest::CQuestManager::Instance().RequestSetEventFlag("e_flower_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice(LC_TEXT("[326]The Flower Power Event has started."));
	else
		BroadcastNotice(LC_TEXT("[329]The Flower Power Event has ended."));

	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
bool CEventManager::SetMiniGameCatchKingEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("mini_game_catchking_event", bActive);
	CMiniGameManager::Instance().InitializeMiniGameCatchKing(bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("catch_king_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("catch_king_event_end_day", lEndTime);
	CMiniGameManager::Instance().InitializeMiniGameCatchKingEndTime(lEndTime);

	if (bActive)
	{
		BroadcastNotice("[1421][Catch the King] Defeat monsters to collect King Cards. You need 25 cards for a King Deck.");
		BroadcastNotice("[1422][Catch the King] Loads of great prizes await!");
		BroadcastNotice("[1423][Catch the King] If you make it into the top rankings, you'll receive your prize from the game table.");
	}
	else
		BroadcastNotice("[346][Catch the King] Catch the King has ended.");

	return true;
}
#endif

#ifdef ENABLE_GEM_SYSTEM
bool CEventManager::SetMDStartEvent(bool bActive)
{
	UpdateGameFlag("md_start_event", bActive);

	if (bActive)
		BroadcastNotice("The Gem event has started.");
	else
		BroadcastNotice("The Gem event has ended.");

	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
bool CEventManager::SetMiniGameFindMEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("mini_game_findm_event", bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("findm_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("findm_event_end_day", lEndTime);

	if (bActive)
	{
		BroadcastNotice("[1445]Tri-Monster Memo has started!");
		BroadcastNotice("[1446][Tri-Monster Memo] Hunt monsters to collect cards for the game.");
		BroadcastNotice("[1447][Tri-Monster Memo] Rich rewards await!");
	}
	else
	{
		BroadcastNotice("[1448]Tri-Monster Memo has finished.");
		BroadcastNotice("[1449][Tri-Monster Memo] Visit the event NPC to discover your ranking and receive your rewards.");
	}

	return true;
}
#endif

bool CEventManager::SetLateSummerEvent(bool bActive)  // ENABLE_SUMMER_EVENT_ROULETTE
{
	UpdateGameFlag("e_late_summer_event", bActive);
	//quest::CQuestManager::Instance().SetEventFlag("xmas_soul", bActive);

	if (bActive)
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("xmas_soul", 1);
		BroadcastNotice("[1457]The Ritual of Blood has started. Buy special items to get better rewards.");
	}
	else
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("xmas_soul", 0);
		BroadcastNotice("[1458]The Ritual of Blood is over. Try again another time.");
	}

	return true;
}

#ifdef ENABLE_MINI_GAME_BNW
bool CEventManager::SetMiniGameBNWEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("mini_game_bnw_event", bActive);

	const auto lEndTime = table->endTime;

	quest::CQuestManager::Instance().RequestSetEventFlag("bnw_perc", table->iPercent);
	quest::CQuestManager::Instance().RequestSetEventFlag("bnw_event_end_day", lEndTime);

	if (bActive)
	{
		BroadcastNotice("[1513][Duel of the Seers] The Duel of the Seers has begun!]");
		BroadcastNotice("[1514][Duel of the Seers] You receive a quest for the minigame.");
		BroadcastNotice("[1515][Duel of the Seers] Rich treasures await!");
	}
	else
	{
		BroadcastNotice("[1516][Duel of the Seers] The Duel of the Seers is over.");
		BroadcastNotice("[1517][Duel of the Seers] Check your rank and claim your reward.");
	}

	return true;
}
#endif

bool CEventManager::SetWorldBossEvent(bool bActive)
{
	UpdateGameFlag("world_boss_event", bActive);

	if (bActive)
		BroadcastNotice("[1622][World boss] Event started");
	else
		BroadcastNotice("[1623][World boss] Event completed");

	return true;
}

bool CEventManager::SetBattleRoyaleEvent(bool bActive) // ENABLE_BATTLE_ROYALE
{
	UpdateGameFlag("battle_royale_event", bActive);

	if (bActive)
		BroadcastNotice("The Battle Royale event has started.");
	else
		BroadcastNotice("The Battle Royale event has ended.");

	return true;
}

#ifdef ENABLE_METINSTONE_RAIN_EVENT
bool CEventManager::SetMetinstoneRainEvent (bool bActive)
{
	UpdateGameFlag("metinstone_rain_event", bActive);

	if (bActive)
	{
		BroadcastNotice("[1813]The Metin Fever event has started!");
		BroadcastNotice("[1814]Complete quests on the event quest list, destroy Metin stones and claim your rewards.");
	}
	else
		BroadcastNotice("[1815]Metin Fever is not currently active.");

	return true;
}
#endif

// Custom
bool CEventManager::SetExperienceEvent(bool bActive, const TEventTable* table)
{
	if (bActive && (table->iPercent == 0))
	{
		sys_err("CEventManager::SetExperienceEvent: rate is set to null.");
		return false;
	}

	UpdateGameFlag("exp_event", bActive);

	if (bActive)
	{
		const long time = table->endTime - table->startTime;
		if (!time)
		{
			sys_err("CEventManager::SetExperienceEvent: time is set to null.");
			return false;
		}

		BroadcastNotice("Experience Boost Event has started. Enjoy the leveling.");
		CPrivManager::Instance().RequestGiveEmpirePriv(ALL_EMPIRES, PRIV_EXP_PCT, table->iPercent, time);
	}
	else
	{
		BroadcastNotice("Experience Boost Event has ended.");
	}

	return true;
}

bool CEventManager::SetItemDropEvent(bool bActive, TEventTable* table)
{
	if (bActive && (table->iPercent == 0))
	{
		sys_err("CEventManager::SetItemDropEvent: rate is set to null.");
		return false;
	}

	UpdateGameFlag("item_drop_event", bActive);

	if (bActive)
	{
		const long time = table->endTime - table->startTime;
		if (!time)
		{
			sys_err("CEventManager::SetExperienceEvent: time is set to null.");
			return false;
		}

		BroadcastNotice("Item-Drop Boost Event has started. Enjoy the grinding.");
		CPrivManager::Instance().RequestGiveEmpirePriv(ALL_EMPIRES, PRIV_ITEM_DROP, table->iPercent, time);
	}
	else
		BroadcastNotice("Item-Drop Boost Event has ended.");

	return true;
}

bool CEventManager::SetSuperMetinEvent(bool bActive)
{
	UpdateGameFlag("superstone_rain_event", bActive);

	if (bActive)
	{
		const LPSECTREE_MAP& pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		dynamic_spawn_cycle_event_info* info = AllocEventInfo<dynamic_spawn_cycle_event_info>();
		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		info->strRegenPath = "data/event/metin_event_regen_type_1.txt";
		info->strRegenPath2 = "data/event/metin_event_regen_type_2.txt";

		event_create(dynamic_spawn_cycle_event, info, PASSES_PER_SEC(1));

		BroadcastNotice("Metin Shower Event has started. Open the Quest Scroll to join.");
	}
	else
	{
		BroadcastNotice("Metin Shower Event has ended. Thank you for participating.");
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();

		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(15));

		SendNoticeMap(LC_TEXT("[1]Everyone will be teleported into the city shortly."), EVENT_MAP_INDEX, false);

		FKillSectree f;
		pSecMap->for_each(f);
	}

	return true;
}

bool CEventManager::SetBossEvent(bool bActive)
{
	UpdateGameFlag("boss_event", bActive);

	if (bActive)
	{
		const LPSECTREE_MAP& pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		dynamic_spawn_cycle_event_info* info = AllocEventInfo<dynamic_spawn_cycle_event_info>();
		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		info->strRegenPath = "data/event/boss_event_regen_type_1.txt";
		info->strRegenPath2 = "data/event/boss_event_regen_type_2.txt";

		event_create(dynamic_spawn_cycle_event, info, PASSES_PER_SEC(1));

		BroadcastNotice("Boss Hunt Event has started. Open the Quest Scroll to join.");
	}
	else
	{
		BroadcastNotice("Boss Hunt Event has ended. Thank you for participating.");

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();

		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(60));

		SendNoticeMap(LC_TEXT("[1]Everyone will be teleported into the city shortly."), EVENT_MAP_INDEX, false);

		FKillSectree f;
		pSecMap->for_each(f);
	}
	return true;
}

bool CEventManager::SetManwooEvent(bool bActive)
{
	UpdateGameFlag("manwoo_event", bActive);

	if (bActive)
	{
		BroadcastNotice("Fishing Event (Manwoo) has started. Bring Fishbones to the Fisherman and receive your reward.");
	}
	else
	{
		BroadcastNotice("Fishing Event (Manwoo) has finished.");
	}

	return true;
}

bool CEventManager::SetMiningEvent(bool bActive)
{
	UpdateGameFlag("mining_event", bActive);

	if (bActive)
	{
		const LPSECTREE_MAP& pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		regen_free_map(EVENT_MAP_INDEX);

		if (!regen_load("data/event/mining_event_regen_type_0.txt", EVENT_MAP_INDEX, pSecMap->m_setting.iBaseX, pSecMap->m_setting.iBaseY))
			return false;

		BroadcastNotice("The mining event has started");

	}
	else
	{
		BroadcastNotice("The mining event has finished");
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();

		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(15));

		SendNoticeMap(LC_TEXT("[1]Everyone will be teleported into the city shortly."), EVENT_MAP_INDEX, false);

		regen_free_map(EVENT_MAP_INDEX);

		FKillSectree f;
		pSecMap->for_each(f);
	}

	return true;
}

bool CEventManager::SetBudokanEvent(bool bActive)
{
	//UpdateGameFlag("oxevent_status", bActive);

	if (bActive)
	{
		BroadcastNotice("Budokan will be starting soon, prepare yourselves.");
	}
	else
	{
		BroadcastNotice("Budokan has ended.");
	}

	return true;
}

bool CEventManager::SetSungziWarEvent(bool bActive)
{
	//UpdateGameFlag("oxevent_status", bActive);

	if (bActive)
	{
		BroadcastNotice("SungZi War will be starting soon, prepare yourselves.");
	}
	else
	{
		BroadcastNotice("SungZi War has ended.");
	}

	return true;
}

// Custom drop events
bool CEventManager::SetMoonlightEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("moon_drop", bActive);
	quest::CQuestManager::Instance().RequestSetEventFlag("moon_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice("Moonlight Chest Event has started! Defeat enemies and grab your rewards!");
	else
		BroadcastNotice("Moonlight Chest Event has ended!");

	return true;
}

bool CEventManager::SetHexegonalEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("hexegonal_drop", bActive);
	quest::CQuestManager::Instance().RequestSetEventFlag("hexegonal_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice("Hexegonal Box Event has started! Defeat enemies and grab your rewards!");
	else
		BroadcastNotice("Hexegonal Box Event has ended!");

	return true;
}

bool CEventManager::SetHuntYourMountEvent(bool bActive, const TEventTable* table)
{
	UpdateGameFlag("hunt_your_mount_drop", bActive);
	quest::CQuestManager::Instance().RequestSetEventFlag("hunt_your_mount_drop_perc", table->iPercent);

	if (bActive)
		BroadcastNotice("Hunt your mount has started! Defeat enemies and grab your rewards!");
	else
		BroadcastNotice("Hunt your mount Event has ended!");

	return true;
}

// Unused
bool CEventManager::SetGoldFrogEvent(bool bActive)
{
	UpdateGameFlag("golden_frog_event", bActive);

	if (bActive)
	{
		const LPSECTREE_MAP& pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		static_spawn_cycle_event_info* info = AllocEventInfo<static_spawn_cycle_event_info>();
		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		info->strRegenPath = "data/event/golden_frog_event_regen_type_0.txt";

		event_create(static_spawn_cycle_event, info, PASSES_PER_SEC(1));
		BroadcastNotice("Golden Frog Event has started. Open the Quest Scroll to join.");

	}
	else
	{
		BroadcastNotice("Golden Frog Event has ended.Thank you for participating.");

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(EVENT_MAP_INDEX);

		if (!pSecMap)
			return false;

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();

		info->dwWarpMapIndex = EVENT_MAP_INDEX;
		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(15));

		SendNoticeMap(LC_TEXT("[1]Everyone will be teleported into the city shortly."), EVENT_MAP_INDEX, false);

		FKillSectree f;
		pSecMap->for_each(f);
	}

	return true;
}

bool CEventManager::SetTanakaEvent(bool bActive, TEventTable* pTable)
{
	if (g_bChannel == EVENT_CHANNEL)
	{
		UpdateGameFlag("tanaka_event", bActive);

		if (bActive)
		{
			TPacketGGEvent packet;
			packet.bHeader = HEADER_GG_EVENT;
			packet.table = *pTable;
			packet.bState = bActive;

			P2P_MANAGER::Instance().Send(&packet, sizeof(packet));

			BroadcastNotice("Pirate Tanaka Event has started. Tanakas took over the desert, hunt them down!");
		}
		else
			BroadcastNotice("Pirate Tanaka Event has ended. Thank you for participating!");

		return true;
	}

	if (!map_allow_find(TANAKA_EVENT_MAP))
		return true;

	if (bActive)
	{

		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(TANAKA_EVENT_MAP);

		if (!pSecMap)
			return false;

		static_spawn_cycle_event_info* info = AllocEventInfo<static_spawn_cycle_event_info>();
		info->dwWarpMapIndex = TANAKA_EVENT_MAP;
		info->strRegenPath = "data/event/tanaka_event_regen_type_0.txt";

		event_create(static_spawn_cycle_event, info, PASSES_PER_SEC(1));
	}

	return true;
}

bool CEventManager::SetHideAndSeekEvent(bool bActive)
{
	UpdateGameFlag("hide_seek_event", bActive);

	if (bActive)
	{
		BroadcastNotice("Hide and Seek Event has started. Search for the Old Box to receive a reward!");
		SetHideAndSeekRound(HIDE_AND_SEEK_ROUNDS);

		TPacketGGEventHideAndSeek packet;
		packet.bHeader = HEADER_GG_EVENT_HIDE_AND_SEEK;
		packet.iPosition = number(0, HIDE_AND_SEEK_POSITIONS - 1);
		packet.iRound = HIDE_AND_SEEK_ROUNDS;

		P2P_MANAGER::Instance().Send(&packet, sizeof(packet));
	}
	else
	{
		BroadcastNotice("Hide and Seek Event has ended. Thank you for participating!");

		CQuestManager::Instance().RequestSetEventFlag("hide_seek_vid", 0);
		SetHideAndSeekRound(0);

		// Send a packet to other game cores to remove the NPC in case it was not found by players
		TPacketGGEventHideAndSeek packet;
		packet.bHeader = HEADER_GG_EVENT_HIDE_AND_SEEK;
		packet.iPosition = -1;
		packet.iRound = -1;
		P2P_MANAGER::Instance().Send(&packet, sizeof(packet));
	}

	return true;
}

/*
	If iPosition is equal to -1, it will only try to find an unfound NPC
	and purge it from the game. If iPosition is greater than -1, an NPC
	will be spawned if the map data that corresponds to NPCPosition[iPosition]
	is valid.
*/
bool CEventManager::HideAndSeekNPC(int iPosition, int iRound)
{
	// Delete the spawned NPC
	{
		if (m_pHideAndSeekNPC)
		{
			M2_DESTROY_CHARACTER(m_pHideAndSeekNPC);
			m_pHideAndSeekNPC = nullptr;
		}

		// If position is negative, we only need to remove the NPC
		if (iPosition < 0)
			return true;
	}

	// Close the event in case there are no rounds left
	if (iRound <= 0)
	{
		TEventTable eventTable;
		memset(&eventTable, 0, sizeof(eventTable));
		strlcpy(eventTable.szType, "EVENT_TYPE_HIDE_AND_SEEK", sizeof(eventTable.szType));

		SetEventState(&eventTable, false);
		return true;
	}

	// Decrease the round count
	SetHideAndSeekRound(--iRound);

	const THideAndSeek table = aNPCPosition[iPosition];
	if (!table.dwMapIndex)
	{
		sys_err("could not find map data on position %d", iPosition);
		return false;
	}

	// Return if there is no map in the game core
	if (!map_allow_find(table.dwMapIndex))
		return false;

	LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(table.dwMapIndex);

	if (!pSecMap)
		return false;

	// Get a random coordinates index for the NPC
	const uint8_t bLocationIndex = number(0, table.vec_Positions.size() - 1);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().SpawnMob(HIDE_AND_SEEK_NPC,
		table.dwMapIndex,
		pSecMap->m_setting.iBaseX + table.vec_Positions[bLocationIndex].x * 100,
		pSecMap->m_setting.iBaseY + table.vec_Positions[bLocationIndex].y * 100,
		0);

	if (!ch)
	{
		sys_err("could not spawn NPC on map %d pos %d %d", table.dwMapIndex, table.vec_Positions[bLocationIndex].x, table.vec_Positions[bLocationIndex].y);
		return false;
	}

	// Assign the hidden NPC and save it's VID
	m_pHideAndSeekNPC = ch;
	CQuestManager::Instance().RequestSetEventFlag("hide_seek_vid", ch->GetVID());

	// Select a random hint announcement and display it to players
	std::string strHintAnnouncement = table.vec_Annoucments.at(bLocationIndex);  //number(0, table.vec_Annoucments.size() - 1) 
	BroadcastNotice(strHintAnnouncement.c_str());

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
//  OX Event
///////////////////////////////////////////////////////////////////////////////////////
EVENTINFO(ox_event_process_info)
{
	uint8_t bState;
	uint8_t bRegistrationTimeLeft;
	uint8_t bRound;
	ox_event_process_info()
		: bState(0), bRegistrationTimeLeft(0), bRound(0)
	{
	}
};

EVENTFUNC(ox_event_process)
{
	ox_event_process_info* info = dynamic_cast<ox_event_process_info*>(event->info);

	if (!info)
	{
		sys_err("ox_event_process <Factor> Null pointer to event");
		return 0;
	}

	switch (info->bState)
	{
		// Start Phase
		case OXEVENT_OPEN:
		{
			--info->bRegistrationTimeLeft;

			if (info->bRegistrationTimeLeft)
			{
				char buf[128 + 1];
				snprintf(buf, sizeof(buf), "OX Event will be starting in %d minute(s).", info->bRegistrationTimeLeft);

				BroadcastNotice(buf);

				return PASSES_PER_SEC(60);
			}
			else
			{
				info->bState = OXEVENT_CLOSE;
				--info->bRound;	// Decrease available rounds count on each start

				// Restrict players from entering the event
				COXEventManager::Instance().SetStatus(OXEVENT_CLOSE);

				// Start Quiz phase in 15 secs
				return PASSES_PER_SEC(15);
			}
		}
		break;

		// Quiz Phase
		case OXEVENT_CLOSE:
		{
			if (COXEventManager::Instance().GetAttenderCount() <= CEventManager::OX_PLAYER_WIN_COUNT)
			{
				COXEventManager::Instance().GiveItemToAttender(CEventManager::OX_REWARD_VNUM, CEventManager::OX_REWARD_COUNT);
				info->bState = OXEVENT_FINISH;

				SendNoticeMap("Participants will now be teleported to the city.", MAP_OXEVENT, true);

				return PASSES_PER_SEC(15);
			}

			if (!COXEventManager::Instance().Quiz(1, 30))
			{
				sys_err("Could not start an OX quiz.");

				COXEventManager::instance().CloseEvent();
				CEventManager::Instance().SetOXEvent(false);
			}

			/*
				Question Time:			15 Secs
				Result Time:			10 Secs
				Player Cleanup Time:	5 Secs
				-------------------------------
				Question Interval:		35 Secs
			*/
			return PASSES_PER_SEC(35);
		}
		break;

		// End Phase
		case OXEVENT_FINISH:
		{
			/*
				Continue the event until there are no available rounds left.
				End the event in case there are no rounds left.
			*/
			if (info->bRound)
			{
				BroadcastNotice("The next round of OX will start again shortly.");

				// Clear the event map
				COXEventManager::instance().CloseEvent();

				info->bState = OXEVENT_OPEN;
				info->bRegistrationTimeLeft = CEventManager::OX_START_WAITING_TIME;

				COXEventManager::Instance().SetStatus(OXEVENT_OPEN);

				// Back to the Start Phase..
				return PASSES_PER_SEC(60);
			}
			else
			{

				// End the event
				CEventManager::Instance().SetOXEvent(false);
			}

			return 0;
		}
		break;
	}

	return 0;
}

bool CEventManager::SetOXEvent(bool bActive)
{
	if (bActive)
	{
		// Return if an OX Event is still in process
		if ((COXEventManager::Instance().GetStatus() != OXEVENT_FINISH) || m_pOXEvent)
		{
			sys_err("OX Event is already running");
			return false;
		}

		COXEventManager::instance().ClearQuiz();

		char szFilePath[256];
		snprintf(szFilePath, sizeof(szFilePath), "%s/oxquiz.lua", LocaleService_GetBasePath().c_str());
		int result = lua_dofile(quest::CQuestManager::instance().GetLuaState(), szFilePath);
		if (result != 0)
		{
			sys_err("Could not load quiz file %s", szFilePath);
			return false;
		}

		// Start OX Event
		BroadcastNotice("OX Event will be starting soon, prepare yourselves.");
		UpdateGameFlag("oxevent_status", bActive);

		ox_event_process_info* info = AllocEventInfo<ox_event_process_info>();
		info->bState = OXEVENT_OPEN;
		info->bRegistrationTimeLeft = OX_START_WAITING_TIME;
		info->bRound = OX_ROUND_COUNT;
		m_pOXEvent = event_create(ox_event_process, info, PASSES_PER_SEC(60));

		COXEventManager::Instance().SetStatus(OXEVENT_OPEN);
	}
	else
	{
		BroadcastNotice("OX Event has ended. Thank you for participating!");
		UpdateGameFlag("oxevent_status", bActive);

		COXEventManager::Instance().SetStatus(OXEVENT_FINISH);
		COXEventManager::instance().CloseEvent();

		event_cancel(&m_pOXEvent);
		m_pOXEvent = nullptr;
	}

	return true;
}

bool CEventManager::SetSiegeWarEvent(bool bActive)
{
	if (bActive)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(SIEGE_MAP_INDEX);

		if (!pSecMap)
			return false;

		const static std::pair<long, long> s_aStonePos[SIEGE_EMPIRE_COUNT] =
		{
			{224, 302},
			{467, 216},
			{433, 566}
		};

		// Spawn Kingdom Stones
		for (int i = 0; i < SIEGE_EMPIRE_COUNT; ++i)
		{
			LPCHARACTER pMetinStone = CHARACTER_MANAGER::Instance().SpawnMob(8020 + i, SIEGE_MAP_INDEX,
				pSecMap->m_setting.iBaseX + s_aStonePos[i].first * 100, pSecMap->m_setting.iBaseY + s_aStonePos[i].second * 100, 0);

			if (!pMetinStone)
				return false;
		}

		// Clean the event data
		m_bSiegeScore = 0;
		CQuestManager::instance().RequestSetEventFlag("siege_shinsoo_score", 0);
		CQuestManager::instance().RequestSetEventFlag("siege_chunjo_score", 0);
		CQuestManager::instance().RequestSetEventFlag("siege_jinno_score", 0);

		BroadcastNotice("Siege War Event has started. Coordinate with the kingdom to become victorious.");
		UpdateGameFlag("siege_war_event", bActive);
	}
	else
	{
		int iShinsooScore = CQuestManager::instance().GetEventFlag("siege_shinsoo_score");
		int iChunjoScore = CQuestManager::instance().GetEventFlag("siege_chunjo_score");
		int iJinnoScore = CQuestManager::instance().GetEventFlag("siege_jinno_score");

		/*
			Score 1 -> Rank 3
			Score 2 -> Rank 2
			Score 3 -> Rank 1

			Status is NULL if the empire's Metin has not been destroyed.
			In this case remaining kingdoms share the next reward.
		*/

		if (iShinsooScore <= 0)
			iShinsooScore = m_bSiegeScore + 1;

		if (iChunjoScore <= 0)
			iChunjoScore = m_bSiegeScore + 1;

		if (iJinnoScore <= 0)
			iJinnoScore = m_bSiegeScore + 1;

		switch (iShinsooScore)
		{
			case 1:
				CPrivManager::instance().RequestGiveEmpirePriv(SHINSOO_EMPIRE, PRIV_EXP_PCT, 25, 60 * 60 * 48);
				break;
			case 2:
				CPrivManager::instance().RequestGiveEmpirePriv(SHINSOO_EMPIRE, PRIV_EXP_PCT, 50, 60 * 60 * 48);
				break;
			case 3:
				CPrivManager::instance().RequestGiveEmpirePriv(SHINSOO_EMPIRE, PRIV_EXP_PCT, 100, 60 * 60 * 48);
				CPrivManager::instance().RequestGiveEmpirePriv(SHINSOO_EMPIRE, PRIV_ITEM_DROP, 50, 60 * 60 * 48);
				break;
			default:
				break;
		}

		switch (iChunjoScore)
		{
			case 1:
				CPrivManager::instance().RequestGiveEmpirePriv(CHUNJO_EMPIRE, PRIV_EXP_PCT, 25, 60 * 60 * 48);
				break;
			case 2:
				CPrivManager::instance().RequestGiveEmpirePriv(CHUNJO_EMPIRE, PRIV_EXP_PCT, 50, 60 * 60 * 48);
				break;
			case 3:
				CPrivManager::instance().RequestGiveEmpirePriv(CHUNJO_EMPIRE, PRIV_EXP_PCT, 100, 60 * 60 * 48);
				CPrivManager::instance().RequestGiveEmpirePriv(CHUNJO_EMPIRE, PRIV_ITEM_DROP, 50, 60 * 60 * 48);
				break;
			default:
				break;
		}

		switch (iJinnoScore)
		{
			case 1:
				CPrivManager::instance().RequestGiveEmpirePriv(JINNO_EMPIRE, PRIV_EXP_PCT, 25, 60 * 60 * 48);
				break;
			case 2:
				CPrivManager::instance().RequestGiveEmpirePriv(JINNO_EMPIRE, PRIV_EXP_PCT, 50, 60 * 60 * 48);
				break;
			case 3:
				CPrivManager::instance().RequestGiveEmpirePriv(JINNO_EMPIRE, PRIV_EXP_PCT, 100, 60 * 60 * 48);
				CPrivManager::instance().RequestGiveEmpirePriv(JINNO_EMPIRE, PRIV_ITEM_DROP, 50, 60 * 60 * 48);
				break;
			default:
				break;
		}


		// Reset event flags at the end of the event 
		m_bSiegeScore = 0;
		CQuestManager::instance().RequestSetEventFlag("siege_shinsoo_score", 0);
		CQuestManager::instance().RequestSetEventFlag("siege_chunjo_score", 0);
		CQuestManager::instance().RequestSetEventFlag("siege_jinno_score", 0);

		UpdateGameFlag("siege_war_event", bActive);
		BroadcastNotice("Siege War Event has ended. Thank you for participating!");

		// Teleport players to the city
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(SIEGE_MAP_INDEX);

		if (!pSecMap)
			return false;

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();
		info->dwWarpMapIndex = SIEGE_MAP_INDEX;

		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(60));

		FKillSectree f;
		pSecMap->for_each(f);

		SendNoticeMap(LC_TEXT("Everyone will be teleported into the city shortly."), SIEGE_MAP_INDEX, false);
	}

	return true;
}

bool CEventManager::SetKingdomWarEvent(bool bActive)
{
	if (bActive)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(KINGDOM_WAR_MAP_INDEX);

		if (!pSecMap)
			return false;

		// Initialize kingdom score array
		for (int i = 0; i < EMPIRE_MAX_NUM; ++i)
		{
			m_aKingdomWarScore[i].bKingdom = i;
			m_aKingdomWarScore[i].wCount = 0;
		}

		BroadcastNotice("Kingdom War Event has started. Sharpen your weapons and join the battlefield.");
		UpdateGameFlag("kingdom_war_event", bActive);
	}
	else
	{
		// Sort the kingdoms from lowest to highest
		std::sort(m_aKingdomWarScore, m_aKingdomWarScore + EMPIRE_MAX_NUM, SortKingdomWar);

		/*
			Position 1 -> Rank 3
			Position 2 -> Rank 2
			Position 3 -> Rank 1
		*/

		// m_aKingdomWarScore[0] is ignored, its there just for the correct kingdom's position in array
		for (int i = 1; i < EMPIRE_MAX_NUM; ++i)
		{
			switch (i)
			{
				case 1:
					CPrivManager::instance().RequestGiveEmpirePriv(m_aKingdomWarScore[i].bKingdom, PRIV_EXP_PCT, 25, 60 * 60 * 48);
					break;
				case 2:
					CPrivManager::instance().RequestGiveEmpirePriv(m_aKingdomWarScore[i].bKingdom, PRIV_EXP_PCT, 50, 60 * 60 * 48);
					break;
				case 3:
					CPrivManager::instance().RequestGiveEmpirePriv(m_aKingdomWarScore[i].bKingdom, PRIV_EXP_PCT, 100, 60 * 60 * 48);
					CPrivManager::instance().RequestGiveEmpirePriv(m_aKingdomWarScore[i].bKingdom, PRIV_ITEM_DROP, 50, 60 * 60 * 48);
					break;

				default:
					break;
			}
		}

		memset(m_aKingdomWarScore, 0, sizeof(m_aKingdomWarScore));
		UpdateGameFlag("kingdom_war_event", bActive);

		BroadcastNotice("Kingdom War Event has ended. Thank you for participating!");

		// Teleport players to the city
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(KINGDOM_WAR_MAP_INDEX);

		if (!pSecMap)
			return false;

		warp_all_to_village_event_info* info = AllocEventInfo<warp_all_to_village_event_info>();
		info->dwWarpMapIndex = KINGDOM_WAR_MAP_INDEX;

		event_create(warp_all_to_village_event, info, PASSES_PER_SEC(60));

		SendNoticeMap(LC_TEXT("Everyone will be teleported into the city shortly."), KINGDOM_WAR_MAP_INDEX, false);
	}

	return true;
}

void CEventManager::SendKingdomWarScorePacket(LPCHARACTER ch)
{
	if (ch->GetDesc())
	{
		TPacketGCEventKWScore packet;
		packet.bHeader = HEADER_GC_EVENT_KW_SCORE;
		packet.wKingdomScores[0] = m_aKingdomWarScore[SHINSOO_EMPIRE].wCount;
		packet.wKingdomScores[1] = m_aKingdomWarScore[CHUNJO_EMPIRE].wCount;
		packet.wKingdomScores[2] = m_aKingdomWarScore[JINNO_EMPIRE].wCount;

		ch->GetDesc()->Packet(&packet, sizeof(packet));
	}
}

# ifdef ENABLE_BATTLE_PASS_SYSTEM
bool CEventManager::SetBattlePassEvent(bool bActive, const TEventTable* pTable, uint8_t pass_id)
{
	switch (pass_id)
	{
		case BATTLEPASS_NORMAL:
		{
			UpdateGameFlag("battle_pass_normal", bActive);

			TPacketGGEventBattlePass packet;
			packet.bHeader = HEADER_GG_EVENT_BATTLE_PASS;
			packet.table = *pTable;
			packet.bState = bActive;
			packet.bType = BATTLEPASS_NORMAL;

			P2P_MANAGER::Instance().Send(&packet, sizeof(packet));
		}
		break;

		case BATTLEPASS_PREMIUM:
		{
			UpdateGameFlag("battle_pass_premium", bActive);

			TPacketGGEventBattlePass packet;
			packet.bHeader = HEADER_GG_EVENT_BATTLE_PASS;
			packet.table = *pTable;
			packet.bState = bActive;
			packet.bType = BATTLEPASS_PREMIUM;

			P2P_MANAGER::Instance().Send(&packet, sizeof(packet));
		}
		break;

		case BATTLEPASS_EVENT:
		{
			UpdateGameFlag("battle_pass_event", bActive);

			TPacketGGEventBattlePass packet;
			packet.bHeader = HEADER_GG_EVENT_BATTLE_PASS;
			packet.table = *pTable;
			packet.bState = bActive;
			packet.bType = BATTLEPASS_EVENT;

			P2P_MANAGER::Instance().Send(&packet, sizeof(packet));
		}
		break;

		default:
			break;
	}

	return true;
}

bool CEventManager::BattlePassData(const TEventTable* table, uint8_t bType, bool bState)
{
	CBattlePassManager::Instance().SetBattlePassID(bState, bType);
	CBattlePassManager::Instance().SetBattlePassStartTime(table->startTime, bType);
	CBattlePassManager::Instance().SetBattlePassEndTime(table->endTime, bType);

	CBattlePassManager::Instance().CheckBattlePassTimes();
	return true;
}
# endif

# ifdef ENABLE_WORLD_LOTTERY_SYSTEM
bool CEventManager::SetLotteryEvent(bool bActive)
{
	UpdateGameFlag("lottery_event", bActive);

	if (bActive)
		BroadcastNotice("Lottery Started");
	else
		BroadcastNotice("Lottery has ended.");

	return true;
}
# endif
#endif
