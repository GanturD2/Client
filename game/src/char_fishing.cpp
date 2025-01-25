#include "stdafx.h"

#ifdef ENABLE_FISHING_RENEWAL
#include "utils.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "unique_item.h"
#include "fishing.h"
#include "vector.h"
#include "packet.h"
#include "sectree.h"
#include "sectree_manager.h"
#include "questmanager.h"
#include "log.h"
#ifdef ENABLE_BATTLE_PASS_SYSTEM
# include "battle_pass.h"
#endif

EVENTFUNC(fishing_event)
{
	fishingnew_event_info * info = dynamic_cast<fishingnew_event_info *>(event->info);
	if (info == NULL)
		return 0;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(info->pid);
	if (!ch)
		return 0;

	if (ch->GetFishCatch() >= FISHING_NEED_CATCH)
	{
		ch->fishing_catch_decision(info->vnum);
		return 0;
	}
	else
	{
		LPITEM rod = ch->GetWear(WEAR_WEAPON);
		if (!(rod && rod->GetType() == ITEM_ROD))
		{
			ch->fishing_new_stop();
			return 0;
		}

		if (info->sec == 1)
		{
			if (ch->FindAffect(AFFECT_FISH_MIND_PILL) || ch->GetPremiumRemainSeconds(PREMIUM_FISH_MIND) > 0 || ch->IsEquipUniqueGroup(UNIQUE_GROUP_FISH_MIND))
			{
				TItemTable* pTable = ITEM_MANAGER::instance().GetTable(info->vnum);
				if (pTable)
					ch->ChatPacket(CHAT_TYPE_INFO, "Fishing %s", pTable->szLocaleName);
			}
		}

		uint32_t failed = ch->GetFishCatchFailed();
		if (failed > 0) {
			info->sec += failed;
			ch->SetFishCatchFailed(0);
		}

		if (info->sec >= 15) {
			ch->fishing_new_stop();
			return 0;
		}

		++info->sec;
		return (PASSES_PER_SEC(1));
	}
};

void CHARACTER::fishing_new_start()
{
	if (m_pkFishingNewEvent)
		return;

	LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());
	if (!pkSectreeMap)
		return;

	int x = GetX(), y = GetY();
	LPSECTREE tree = pkSectreeMap->Find(x, y);
	if (!tree)
		return;

	if (tree->IsAttr(x, y, ATTR_BLOCK))
	{
		//ChatPacket(CHAT_TYPE_INFO, "fishing_new_start 1");
		return;
	}

	LPITEM pItem = ITEM_MANAGER::Instance().CreateItem(50187);
#ifdef ENABLE_SPECIAL_INVENTORY
	if (GetEmptyInventory(pItem) == -1)
#else
	if (GetEmptyInventory(pItem->GetSize()) == -1)
#endif
	{
		//ChatPacket(CHAT_TYPE_INFO, "fishing_new_start 2");
		return;
	}

	LPITEM rod = GetWear(WEAR_WEAPON);
	if (!rod || rod->GetType() != ITEM_ROD)
	{
		//ChatPacket(CHAT_TYPE_INFO, "fishing_new_start 3");
		return;
	}

	if (rod->GetSocket(2) == 0)
	{
		//ChatPacket(CHAT_TYPE_INFO, "fishing_new_start 4");
		return;
	}

	float fx, fy;
	GetDeltaByDegree(GetRotation(), 400.0f, &fx, &fy);

	SetFishCatch(0);
	SetFishCatchFailed(0);
	SetLastCatchTime(0);

	uint32_t dwVnum = rod->GetVnum();
	bool second = dwVnum >= 27400 && dwVnum <= 27490 ? false : true;

	fishingnew_event_info* info = AllocEventInfo<fishingnew_event_info>();
	info->pid = GetPlayerID();
	info->vnum = fishing::GetFishCatchedVnum(100, 15 + GetPoint(POINT_FISHING_RARE) + rod->GetSocket(2), second);
	info->chance = 0;
	info->sec = 1;
	m_pkFishingNewEvent = event_create(fishing_event, info, PASSES_PER_SEC(1));

	TPacketFishingNew p;
	p.header = HEADER_GC_FISHING_NEW;
	p.subheader = FISHING_SUBHEADER_NEW_START;
	p.vid = GetVID();
	p.dir = (uint8_t)(GetRotation() / 5);
	p.need = FISHING_NEED_CATCH;
	p.count = 0;
	PacketAround(&p, sizeof(p));
}

void CHARACTER::fishing_new_stop()
{
	if (!m_pkFishingNewEvent)
		return;

	event_cancel(&m_pkFishingNewEvent);
	m_pkFishingNewEvent = NULL;

	LPITEM rod = GetWear(WEAR_WEAPON);

	if (rod && rod->GetType() == ITEM_ROD)
		rod->SetSocket(2, 0);

	TPacketFishingNew p;
	p.header = HEADER_GC_FISHING_NEW;
	p.subheader = FISHING_SUBHEADER_NEW_STOP;
	p.vid = GetVID();
	p.dir = 0;
	p.need = 0;
	p.count = 0;
	PacketAround(&p, sizeof(p));
}

void CHARACTER::fishing_new_catch()
{
	if (!m_pkFishingNewEvent)
		return;

	if (GetLastCatchTime() > get_global_time())
		return;

	uint8_t v = GetFishCatch() + 1;
	SetLastCatchTime(get_global_time() + 1);
	SetFishCatch(v);

	TPacketFishingNew p;
	p.header = HEADER_GC_FISHING_NEW;
	p.subheader = FISHING_SUBHEADER_NEW_CATCH;
	p.vid = GetVID();
	p.dir = 0;
	p.need = 0;
	p.count = v;
	PacketAround(&p, sizeof(p));
}

void CHARACTER::fishing_new_catch_failed()
{
	if (!m_pkFishingNewEvent)
		return;

	SetFishCatchFailed(GetFishCatchFailed() + 1);

	TPacketFishingNew p;
	p.header = HEADER_GC_FISHING_NEW;
	p.subheader = FISHING_SUBHEADER_NEW_CATCH_FAILED;
	p.vid = GetVID();
	p.dir = 0;
	p.need = 0;
	p.count = 0;
	PacketAround(&p, sizeof(p));
}

void CHARACTER::fishing_catch_decision(uint32_t itemVnum)
{
	if (!m_pkFishingNewEvent)
		return;

	event_cancel(&m_pkFishingNewEvent);
	m_pkFishingNewEvent = NULL;

	LPITEM rod = GetWear(WEAR_WEAPON);
	if (!rod)
		return;

	if (rod->GetType() == ITEM_ROD)
	{
		if (rod->GetRefinedVnum()> 0 && rod->GetSocket(0) < rod->GetValue(2) && number(1, rod->GetValue(1)) == 1)
		{
			rod->SetSocket(0, rod->GetSocket(0) + 1);
			//ChatPacket(CHAT_TYPE_INFO, "fishing_catch_decision %d#%d", rod->GetSocket(0), rod->GetValue(2));

			if (rod->GetSocket(0) == rod->GetValue(2))
			{
				//ChatPacket(CHAT_TYPE_INFO, "fishing_catch_decision");
			}
		}

		rod->SetSocket(2, 0);
	}

	uint8_t chance;
	switch (itemVnum) {
		case 27803:
		case 27806:
		case 27816:
		case 27807:
		case 27818:
		case 27805:
		case 27822:
		case 27823:
		case 27824:
		case 27825:
			{
				chance = 10;
			}
			break;
		case 27804:
		case 27811:
		case 27810:
		case 27809:
		case 27814:
		case 27812:
		case 27808:
		case 27826:
		case 27827:
		case 27813:
		case 27815:
		case 27819:
		case 27820:
		case 27821:
			{
				chance = 3;
			}
			break;
		default:
			{
				chance = 0;
			}
			break;
	}

	if (GetPoint(POINT_FISHING_RARE) > 0 && chance == 3)
		chance += 20;

	uint32_t dwVnum = rod->GetVnum();

	if (dwVnum == 27591 && dwVnum >= 27400 && dwVnum <= 27490)
		chance += (rod->GetValue(0) / 10) * 2;
	else
		chance += rod->GetValue(0) / 10;

	//NEW_FISHING_LOGS
	bool second = dwVnum >= 27400 && dwVnum <= 27490 ? false : true;
	//NEW_FISHING_LOGS

	TPacketFishingNew p;
	p.header = HEADER_GC_FISHING_NEW;
	if (number(1, 100) >= chance)
	{
		p.subheader = FISHING_SUBHEADER_NEW_CATCH_FAIL;
		//NEW_FISHING_LOGS
		LogManager::instance().FishNewLog(GetPlayerID(), fishing::GetFishCatchedVnum(100, 15 + GetPoint(POINT_FISHING_RARE) + rod->GetSocket(2), second), 0);
		//NEW_FISHING_LOGS
	}
	else
	{
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		UpdateExtBattlePassMissionProgress(FISH_CATCH, 1, itemVnum);
#endif

		p.subheader = FISHING_SUBHEADER_NEW_CATCH_SUCCESS;
		AutoGiveItem(itemVnum, 1, -1, false);

#ifdef ENABLE_FISH_EVENT
		if(quest::CQuestManager::instance().GetEventFlag("enable_fish_event"))
		{
			if (IsEquipUniqueItem(UNIQUE_ITEM_FISH_MIND))
			{
				int r = number(1, 100);
				if(r <= 10)
					AutoGiveItem(ITEM_FISH_EVENT_BOX_SPECIAL, 1, -1, false);
				else
					AutoGiveItem(ITEM_FISH_EVENT_BOX, 5, -1, false);
			}
			else
			{
				AutoGiveItem(ITEM_FISH_EVENT_BOX, 5, -1, false);
			}
		}
#endif
		//NEW_FISHING_LOGS
		LogManager::instance().FishNewLog(GetPlayerID(), fishing::GetFishCatchedVnum(100, 15 + GetPoint(POINT_FISHING_RARE) + rod->GetSocket(2), second), 1);
		//NEW_FISHING_LOGS
	}

	p.vid = GetVID();
	p.dir = 0;
	p.need = 0;
	p.count = 0;
	PacketAround(&p, sizeof(p));
}
#endif
