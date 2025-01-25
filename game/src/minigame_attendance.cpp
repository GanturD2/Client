#include "stdafx.h"

#ifdef ENABLE_MONSTER_BACK
#include "config.h"
#include "minigame_manager.h"

#include "../../common/length.h"
#include "../../common/tables.h"
#include "p2p.h"
#include "locale_service.h"
#include "char.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "questmanager.h"
#include "questlua.h"
#include "start_position.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "regen.h"
#include "log.h"
#include "db.h"
#include "target.h"
#include "party.h"

void CMiniGameManager::InitializeAttendance(int iEnable)
{
	const uint32_t bossMaps[4] = { MapTypes::MAP_N_SNOWM_01, MapTypes::MAP_N_FLAME_01, MapTypes::MAP_N_DESERT_01, MapTypes::MAP_N_THREEWAY };

#ifdef ENABLE_10TH_EVENT
	char szFileNameWarrior[256 + 1] = { '\0' };
	snprintf(&szFileNameWarrior[0], sizeof(szFileNameWarrior), "data/event/attendance/shadow_warrior_regen.txt");
#else
	char szFileNameNemere[256 + 1] = { '\0' };
	snprintf(&szFileNameNemere[0], sizeof(szFileNameNemere), "data/event/attendance/nemere_regen.txt");

	char szFileNameRazador[256 + 1] = { '\0' };
	snprintf(&szFileNameRazador[0], sizeof(szFileNameRazador), "data/event/attendance/razador_regen.txt");

	char szFileNameDragon[256 + 1] = { '\0' };
	snprintf(&szFileNameDragon[0], sizeof(szFileNameDragon), "data/event/attendance/red_dragon_regen.txt");

	char szFileNameBeran[256 + 1] = { '\0' };
	snprintf(&szFileNameBeran[0], sizeof(szFileNameBeran), "data/event/attendance/beran_setaou_regen.txt");
#endif

	if (iEnable)
	{
		SpawnEventNPC(ATTENDANCE_NPC);

		for (uint8_t i = 0; i < 4; i++)
		{
			const LPSECTREE_MAP& pkMap = SECTREE_MANAGER::Instance().GetMap(bossMaps[i]);
			if (pkMap && map_allow_find(bossMaps[i]))
			{
				switch (bossMaps[i])
				{
#ifdef ENABLE_10TH_EVENT
					case MapTypes::MAP_N_SNOWM_01:
					case MapTypes::MAP_N_FLAME_01:
					case MapTypes::MAP_N_DESERT_01:
					case MapTypes::MAP_N_THREEWAY:
						regen_load(&szFileNameWarrior[0], bossMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
						break;
#else
					case MapTypes::MAP_N_SNOWM_01:
						regen_load(&szFileNameNemere[0], bossMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
						break;
					case MapTypes::MAP_N_FLAME_01:
						regen_load(&szFileNameRazador[0], bossMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
						break;
					case MapTypes::MAP_N_DESERT_01:
						regen_load(&szFileNameDragon[0], bossMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
						break;
					case MapTypes::MAP_N_THREEWAY:
						regen_load(&szFileNameBeran[0], bossMaps[i], pkMap->m_setting.iBaseX, pkMap->m_setting.iBaseY);
						break;
#endif
					default:
						break;
				}
			}
		}
	}
	else
	{
		const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(ATTENDANCE_NPC);
		for (auto ch : chars)
		{
			M2_DESTROY_CHARACTER(ch);
		}

#ifdef ENABLE_10TH_EVENT
		clear_regen(&szFileNameWarrior[0]);
#else
		clear_regen(&szFileNameNemere[0]);
		clear_regen(&szFileNameRazador[0]);
		clear_regen(&szFileNameDragon[0]);
		clear_regen(&szFileNameBeran[0]);
#endif
	}
}

void CMiniGameManager::AttendanceMonsterAttack(LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
{
	if (pkAttacker == nullptr || pkVictim == nullptr)
		return;

	if (!pkAttacker->GetDesc())
		return;

	if (pkVictim->IsMonsterBackVnum())
	{
		if (pkAttacker->IsPC())
		{
			if (!pkAttacker->FindAffect(AFFECT_ATT_SPEED_SLOW))
				pkAttacker->AddAffect(AFFECT_ATT_SPEED_SLOW, POINT_ATT_SPEED, -20, 0, 60, 0, false);
		}
	}

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	if (pkVictim->IsMonsterBackVnum())
	{
		bool bContain = false;
		uint32_t dwCount = 1;

		if (!pkAttacker->m_hitCount.empty())
		{
			for (uint32_t i = 0; i < pkAttacker->m_hitCount.size(); i++)
			{
				if (pkAttacker->m_hitCount[i].dwVid == pkVictim->GetVID())
				{
					bContain = true;
					pkAttacker->m_hitCount[i].dwCount += 1;
					dwCount = pkAttacker->m_hitCount[i].dwCount;
					break;
				}
			}
		}

		if (!bContain)
			pkAttacker->m_hitCount.emplace_back(THitCountInfo(pkVictim->GetVID(), dwCount));

		TPacketGCHitCountInfo packHitCount{};
		packHitCount.bHeader = HEADER_GC_HIT_COUNT_INFO;
		packHitCount.dwVid = pkVictim->GetVID();
		packHitCount.dwCount = dwCount;
		pkAttacker->GetDesc()->Packet(&packHitCount, sizeof(TPacketGCHitCountInfo));
	}
#endif
}

void CMiniGameManager::AttendanceEventInfo(LPCHARACTER pChar)
{
	if (pChar == nullptr)
		return;

	if (!pChar->GetDesc())
		return;

	time_t iTime;
	time(&iTime);
	const tm* pTimeInfo = localtime(&iTime);
	char szClearName[32 + 1] = { '\0' };
	char szRewardName[32 + 1] = { '\0' };
	snprintf(&szClearName[0], sizeof(szClearName), "attendance.clear_day_%d", pTimeInfo->tm_yday);
	snprintf(&szRewardName[0], sizeof(szRewardName), "attendance.reward_day_%d", pTimeInfo->tm_yday);
#ifdef ENABLE_10TH_EVENT
	bool bIsEnable = quest::CQuestManager::Instance().GetEventFlag("e_monsterback");
#else
	bool bIsEnable = quest::CQuestManager::Instance().GetEventFlag("e_easter_monsterback");
#endif
	uint8_t bCurrentDay = pTimeInfo->tm_yday - quest::CQuestManager::Instance().GetEventFlag("attendance_start_day");

	TPacketGCAttendanceEventInfo packEvent{};
	packEvent.bHeader = HEADER_GC_ATTENDANCE_EVENT_INFO;
	packEvent.wSize = static_cast<uint16_t>(sizeof(packEvent) + (sizeof(TRewardItem) * attendanceRewardVec.size()));
	packEvent.bIsEnable = bIsEnable;
	packEvent.dwDay = bCurrentDay;
	packEvent.bClear = static_cast<uint8_t>(pChar->GetQuestFlag(&szClearName[0]));
	packEvent.bReward = static_cast<uint8_t>(pChar->GetQuestFlag(&szRewardName[0]));

	pChar->GetDesc()->BufferedPacket(&packEvent, sizeof(packEvent));
	pChar->GetDesc()->Packet(&attendanceRewardVec[0], sizeof(TRewardItem) * attendanceRewardVec.size());
}

void CMiniGameManager::AttendanceEventRequestReward(LPCHARACTER pChar)
{
	if (pChar == nullptr)
		return;

	if (!pChar->GetDesc())
		return;

	time_t iTime;
	time(&iTime);
	const tm* pTimeInfo = localtime(&iTime);
	char szClearName[32 + 1] = { '\0' };
	char szRewardName[32 + 1] = { '\0' };
	snprintf(&szClearName[0], sizeof(szClearName), "attendance.clear_day_%d", pTimeInfo->tm_yday);
	snprintf(&szRewardName[0], sizeof(szRewardName), "attendance.reward_day_%d", pTimeInfo->tm_yday);

#ifdef ENABLE_10TH_EVENT
	if (!quest::CQuestManager::Instance().GetEventFlag("e_monsterback"))
#else
	if (!quest::CQuestManager::Instance().GetEventFlag("e_easter_monsterback"))
#endif
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pChar->GetQuestFlag(&szClearName[0]) == 0)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need to earn this reward before you get it."));
		return;
	}

	if (pChar->GetQuestFlag(&szRewardName[0]))
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have already obtained your reward for today."));
		return;
	}

	if (pChar->GetLevel() < 30)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need at least level 30 for this event."));
		return;
	}

	uint32_t rewardVnum = 0;
	uint8_t rewardCount = 0;
	if (!attendanceRewardVec.empty())
	{
		for (size_t i = 0; i < attendanceRewardVec.size(); i++)
		{
			if (attendanceRewardVec[i].dwDay == static_cast<uint32_t>(pTimeInfo->tm_yday - quest::CQuestManager::Instance().GetEventFlag("attendance_start_day")) + 1)
			{
				rewardVnum = attendanceRewardVec[i].dwVnum;
				rewardCount = static_cast<uint8_t>(attendanceRewardVec[i].dwCount);
				break;
			}
		}
	}

	pChar->SetQuestFlag(&szRewardName[0], 1);

	if (rewardVnum && rewardCount)
	{
		pChar->AutoGiveItem(rewardVnum, rewardCount);

		TPacketGCAttendanceEvent packEvent{};
		packEvent.bHeader = HEADER_GC_ATTENDANCE_EVENT;
		packEvent.bType = 3;
		packEvent.bValue = 1;
		pChar->GetDesc()->Packet(&packEvent, sizeof(TPacketGCAttendanceEvent));
	}
}

bool CMiniGameManager::ReadRewardItemFile(const char* c_pszFileName)
{
	FILE* fp = fopen(c_pszFileName, "r");
	if (!fp)
		return false;

	uint32_t dwDay;
	uint32_t dwVnum;
	uint32_t dwCount;

	while (fscanf(fp, "%u\t%u\t%u", &dwDay, &dwVnum, &dwCount) != EOF)
	{
		if (!ITEM_MANAGER::Instance().GetTable(dwVnum))
		{
			sys_err("No such item (Vnum: %lu)", dwVnum);
			return false;
		}

		TRewardItem tempItem{};
		tempItem.dwDay = dwDay;
		tempItem.dwVnum = dwVnum;
		tempItem.dwCount = dwCount;

		attendanceRewardVec.emplace_back(tempItem);
	}

	fclose(fp);
	return true;
}

#endif
