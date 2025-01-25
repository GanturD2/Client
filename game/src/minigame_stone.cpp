#include "stdafx.h"

#ifdef ENABLE_METINSTONE_RAIN_EVENT
#include "minigame_manager.h"

#include "../../common/length.h"
#include "../../common/tables.h"
#include "p2p.h"
#include "char.h"
#include "desc_client.h"
#include "char_manager.h"
#include "db.h"

void CMiniGameManager::DeleteStoneEvent()
{
	m_Stone.clear();
	OnePlayer = 0;
	OnePlayerPoint = 0;
}

void CMiniGameManager::RewardItem()
{
	if (!IsStoneEventSystemStatus())
		return;

	int item_vnum = 32142;
	int item_count = 1;

	// if (OnePlayer <= 0)
	// {
		// char winner_notice[256];
		// snprintf(winner_notice, sizeof(winner_notice), LC_TEXT("FAIL_WIN_STONE_EVENT"));
		// SendNotice(winner_notice);
		// return;
	// }

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(OnePlayer);

	if (ch && ch->IsPC())
	{
		DBManager::Instance().DirectQuery("INSERT INTO player.mail_box SET reader_name = '%s', sender_name  = 'Etkinlik Ödülü', title = 'Etkinlik Ödülü', type = 2, time = NOW(), post = 'Metin taþý etkinliðine katýldýgýn için teþekkür ederiz sana bir ödülümüz var', item_count = %d , item_vnum = %d;", ch->GetName(), item_count, item_vnum);
		char winner_notice[256];
		snprintf(winner_notice, sizeof(winner_notice), LC_TEXT("<Metin tasi etkinligi> Metin Tasi etkinligi sona erdi. Etkinliginin kazanani %s olmustur."), ch->GetName());
		SendNotice(winner_notice);

		// if (ch->GetDesc() && ch->IsPC())
			// ch->ChatPacket(CHAT_TYPE_COMMAND, "MailBoxOpen");

		DeleteStoneEvent();
	}
}

void CMiniGameManager::StoneInformation(LPCHARACTER pkChr)
{
	if (!IsStoneEventSystemStatus())
		return;

	if (!pkChr)
		return;

	if (!pkChr->GetDesc())
		return;

	if (!pkChr->IsPC())
		return;

	if (!IsStoneEvent())
	{
		pkChr->ChatPacket(CHAT_TYPE_COMMAND, "IsStoneEvent 0");
		return;
	}
	else
	{
		pkChr->ChatPacket(CHAT_TYPE_COMMAND, "IsStoneEvent 1");
	}

	TPacketGCStoneEvent p{};
	p.header = HEADER_GC_STONE_EVENT;
	p.stone_point = GetStoneEventPoint(pkChr);

	if (pkChr->GetDesc() != nullptr)
		pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCStoneEvent));

	pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_STONE_POINT: %d AMOUNT STONE POINT: %d"), STONE_EVENT_POINT, GetStoneEventPoint(pkChr));
}

void CMiniGameManager::SetStoneKill(uint32_t PID)
{
	if (IsStoneEventSystemStatus())
	{
		if (!PID)
		{
			sys_err("StoneEvent PID Not Found");
			return;
		}

		//std::unordered_map<uint32_t, int16_t>::iterator it = m_Stone.find(PID);
		auto it = m_Stone.find(PID); // c++11
		if (it == m_Stone.end())
		{
			m_Stone.insert(std::make_pair(PID, STONE_EVENT_POINT));
			if (OnePlayer <= 0)
			{
				OnePlayer = PID;
				OnePlayerPoint = STONE_EVENT_POINT;
			}
		}
		else
		{
			it->second += STONE_EVENT_POINT;
			if (it->second > OnePlayerPoint)
			{
				OnePlayer = it->first;
				OnePlayerPoint = it->second;
			}
		}

		// LPCHARACTER ch_one = CHARACTER_MANAGER::Instance().FindByPID(OnePlayer);
		//pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("STONE_EVENT_ONE_PLAYER %s"), ch_one->GetName());
	}
}

void CMiniGameManager::StoneUpdateP2PPacket(uint32_t PID)
{
	if (IsStoneEventSystemStatus())
	{
		if (PID != 0)
		{
			/*** Send P2P Packet Start ***/
			TPacketGGStoneEvent p{};
			p.header = HEADER_GG_STONE_EVENT;
			p.pid = PID;
			P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGStoneEvent));
			/*** Send P2P Packet End ***/
		}
	}
}

int16_t CMiniGameManager::GetStoneEventPoint(LPCHARACTER pkChr)
{
	if (IsStoneEventSystemStatus())
	{
		if (pkChr)
		{
			if (pkChr->GetDesc())
			{
				if (pkChr->IsPC())
				{
					//std::unordered_map<uint32_t, int16_t>::iterator it = m_Stone.find(pkChr->GetPlayerID());
					auto it = m_Stone.find(pkChr->GetPlayerID()); // c++11
					if (it == m_Stone.end())
						return 0;
					else
						return it->second;
				}
			}
		}
	}
}

bool CMiniGameManager::IsStoneEventSystemStatus()
{
	if (quest::CQuestManager::Instance().GetEventFlag("enable_stone_event_system") == 1)
		return false;

	return true;
}

bool CMiniGameManager::IsStoneEvent()
{
	if (quest::CQuestManager::Instance().GetEventFlag("metinstone_rain_event") == 0)
		return false;

	return true;
}
#endif
