#include "stdafx.h"

#ifdef ENABLE_BATTLE_FIELD
#include "../../common/tables.h"
#include "config.h"
#include "p2p.h"
#include "char.h"
#include "desc_client.h"
#include "utils.h"
#include "start_position.h"
#include "char_manager.h"
#include "log.h"
#include "db.h"
#include "party.h"
#include "battle_field.h"

#ifdef ENABLE_RANKING_SYSTEM
#include "ranking_system.h"
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
# include "LocaleNotice.hpp"
#endif

CBattleField::CBattleField() noexcept
{
}

CBattleField::~CBattleField()
{
	//Destroy();
}

bool CBattleField::Initialize()
{
	if (g_bAuthServer)
		return false;

	/* Load Open Time Info from MySQL */
	LoadOpenTimeInfo();

	/* Load Ranking Update Info from MySQL */
	LoadRankingUpdateTimeInfo();

	/* if not channel 99 return */
	if (g_bChannel != BATTLE_FIELD_MAP_CHANNEL)
		return false;

	SetStatus(BATTLEFIELD_CLOSED);
	return true;
}

void CBattleField::Destroy()
{
	CloseEnter();
	SetEventInfo(0, 0);

	//vecCharacter.clear();
}

/* Load Open Time Info from MySQL */
void CBattleField::LoadOpenTimeInfo()
{
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect),
		"SELECT day+0, open_hour, open_minute, close_hour, close_minute "
		"FROM common.battlefield_open_info ORDER BY day, open_hour ASC;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	vecOpenTimeInfo.clear();
	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (size_t i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			if (!row)
				return;

			SOpenTimeInfo openTime{};
			openTime.bDay = (atoi(row[0]) - 1);
			openTime.bOpenHour = static_cast<uint8_t>(atoi(row[1]));
			openTime.bOpenMinute = static_cast<uint8_t>(atoi(row[2]));
			openTime.bCloseHour = static_cast<uint8_t>(atoi(row[3]));
			openTime.bCloseMinute = static_cast<uint8_t>(atoi(row[4]));

			vecOpenTimeInfo.emplace_back(openTime);
		}
	}
}

/* Load Ranking Update Info from MySQL */
void CBattleField::LoadRankingUpdateTimeInfo()
{
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect),
		"SELECT day+0, hour, minute, second "
		"FROM common.battlefield_update_info ORDER BY day, hour ASC;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	vecRankingUpdateInfo.clear();
	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (size_t i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			if (!row)
				return;

			SRankingUpdate updateTime{};
			updateTime.bDay = (atoi(row[0]) - 1);
			updateTime.bHour = static_cast<uint8_t>(atoi(row[1]));
			updateTime.bMinute = static_cast<uint8_t>(atoi(row[2]));
			updateTime.bSecond = static_cast<uint8_t>(atoi(row[3]));

			vecRankingUpdateInfo.emplace_back(updateTime);
		}
	}
}

void CBattleField::PlayerKill(LPCHARACTER pChar, LPCHARACTER pkKiller)
{
	if (nullptr == pChar || nullptr == pkKiller)
		return;

	if (false == pChar->IsPC() || false == pkKiller->IsPC())
		return;

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()) || false == IsBattleZoneMapIndex(pkKiller->GetMapIndex()))
		return;

	if (pChar->m_pkBattleZoneEvent)
	{
		event_cancel(&pChar->m_pkBattleZoneEvent);
	}

	RewardKiller(pkKiller, pChar);
	RewardVictim(pChar);
}

void CBattleField::RewardKiller(const LPCHARACTER pkKiller, const LPCHARACTER& pChar)
{
	if (nullptr == pChar || nullptr == pkKiller)
		return;

	if (false == pChar->IsPC() || false == pkKiller->IsPC())
		return;

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()) || false == IsBattleZoneMapIndex(pkKiller->GetMapIndex()))
		return;

	if (!pChar->GetDesc() || !pkKiller->GetDesc())
		return;

	if (strcmp(pkKiller->GetDesc()->GetHostName(), pChar->GetDesc()->GetHostName()) == 0)
	{
		pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;129]");
		return;
	}

	if (!pkKiller->SetBattleKill(pChar->GetPlayerID()))
	{
		pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;129]");
		return;
	}

	const bool IsInLeaveProcess = pChar->IsAffectFlag(AFF_TARGET_VICTIM);
	const int addPointForKiller = GetEventStatus() == true ? (IsInLeaveProcess ? 10 : 2) : (IsInLeaveProcess ? 5 : 1);

	const uint32_t dwCurrentScore = pkKiller->GetBattleFieldPoint() + addPointForKiller;
	const uint32_t dwTotalScore = pkKiller->GetPoint(POINT_BATTLE_FIELD) + dwCurrentScore;

	pkKiller->SetBattleFieldPoint(dwCurrentScore);
	pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;115;%d;%d]", dwCurrentScore, dwTotalScore);
}

void CBattleField::RewardVictim(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()))
		return;

	const uint32_t dwBattleFieldPoint = pChar->GetBattleFieldPoint();
	const uint32_t removeHalfPoints = (dwBattleFieldPoint / 2) + (dwBattleFieldPoint % 2);
	const uint32_t newPoints = dwBattleFieldPoint - removeHalfPoints;

	pChar->SetBattleFieldPoint(newPoints);

	if (removeHalfPoints > 0)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;127;%d;%d]", removeHalfPoints, newPoints);
	}

	const bool IsInLeaveProcess = pChar->IsAffectFlag(AFF_TARGET_VICTIM);
	if (IsInLeaveProcess)
	{
		pChar->ChatPacket(CHAT_TYPE_COMMAND, "ExitBattleFieldOnDead %d", newPoints);
		pChar->RemoveAffect(AFFECT_TARGET_VICTIM);
	}

	pChar->IncreaseBattleDeadLimit();
}

void CBattleField::Connect(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	if (!pChar->GetDesc())
		return;

	SetWeakRankingPosition(pChar);

	if ((get_global_time() - pChar->GetShopExDailyTimePoint(SHOP_COIN_TYPE_BATTLE_POINT)) > 86400)
	{
		pChar->SetShopExUsablePoint(SHOP_COIN_TYPE_BATTLE_POINT, LIMIT_DAILY_MAX_BATTLE);
		pChar->SetShopExDailyTimePoint(SHOP_COIN_TYPE_BATTLE_POINT, get_global_time());
	}

	pChar->ChatPacket(CHAT_TYPE_COMMAND, "battle_field %d", GetStatus());
	pChar->ChatPacket(CHAT_TYPE_COMMAND, "battle_field_open %d", GetEnterStatus());

	if (GetEventStatus())
		pChar->ChatPacket(CHAT_TYPE_COMMAND, "battle_field_event %d %d %d", GetEventStatus(), GetOpenTime(), GetCloseTime());

	if (false == IsBattleZoneMapIndex(pChar->GetMapIndex()))
		return;

	// Set battle mode so you can attack everyone
	pChar->SetPKMode(PK_MODE_BATTLE);

	// If player have a party and party is not null
	if (LPPARTY party = pChar->GetParty())
	{
		// Remove player from party
		party->Quit(pChar->GetPlayerID());
	}

	if (!GetEnterStatus())
	{
		pChar->WarpSet(EMPIRE_START_X(pChar->GetEmpire()), EMPIRE_START_Y(pChar->GetEmpire()));
		return;
	}

	/*if (std::find(vecCharacter.begin(), vecCharacter.end(), pChar->GetPlayerID()) == vecCharacter.end())
	{
		vecCharacter.emplace_back(pChar->GetPlayerID());
	}*/

}

PIXEL_POSITION CBattleField::GetRandomPosition() noexcept
{
	const PIXEL_POSITION validPositions[20] =
	{
		{ 284, 204, 0 }, { 267, 180, 0 }, { 248, 184, 0 }, { 222, 192, 0 },
		{ 200, 192, 0 }, { 195, 210, 0 }, { 195, 236, 0 }, { 195, 272, 0 },
		{ 214, 283, 0 }, { 188, 300, 0 }, { 178, 326, 0 }, { 190, 346, 0 },
		{ 235, 341, 0 }, { 265, 330, 0 }, { 292, 330, 0 }, { 338, 290, 0 },
		{ 325, 325, 0 }, { 330, 305, 0 }, { 332, 265, 0 }, { 315, 237, 0 }
	};

	return validPositions[number(0, (sizeof(validPositions) / sizeof(validPositions[0])) - 1)];
}

void CBattleField::RestartAtRandomPos(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	int x = 0;
	int y = 0;

	const PIXEL_POSITION tempPos = GetRandomPosition();

	x = tempPos.x;
	y = tempPos.y;

	PIXEL_POSITION pos;
	if (SECTREE_MANAGER::Instance().GetMapBasePosition(pChar->GetX(), pChar->GetY(), pos))
	{
		x += pos.x / 100;
		y += pos.y / 100;
	}

	x *= 100;
	y *= 100;

	pChar->Show(pChar->GetMapIndex(), x, y);
	pChar->Stop();
}

void CBattleField::SetPlayerCooldown(uint32_t dwPlayerID)
{
	quest::PC* qPC = quest::CQuestManager::Instance().GetPCForce(dwPlayerID);
	if (!qPC)
		return;

	qPC->SetFlag("battlefield.cooldown", get_global_time() + TIME_BACK_COOLDOWN);
}

/*void CBattleField::RemovePlayerFromVector(uint32_t dwPlayerId)
{
	if (std::find(vecCharacter.begin(), vecCharacter.end(), dwPlayerId) != vecCharacter.end())
	{
		vecCharacter.erase(std::remove(vecCharacter.begin(), vecCharacter.end(), dwPlayerId), vecCharacter.end());
	}
}*/

void CBattleField::OpenBattleUI(LPCHARACTER pChar)
{
	if (!pChar)
		return;

	if (!pChar->GetDesc())
		return;

	pChar->ChatPacket(CHAT_TYPE_COMMAND, "battle_field %d", GetStatus());
	pChar->ChatPacket(CHAT_TYPE_COMMAND, "BattleFieldLeftTime %u %u", GetOpenTime(), GetCloseTime());

#ifdef ENABLE_RANKING_SYSTEM
	CRankingSystem::Instance().SendBFRanking(pChar);
#endif
}

int CBattleField::GetOpenTime() noexcept
{
	const time_t iTime = time(nullptr);
	const tm* pTimeInfo = localtime(&iTime);

	int openTime = 0;
	for (auto& e : vecOpenTimeInfo)
	{
		const uint8_t bDay = e.bDay;
		const uint8_t bOpenHour = e.bOpenHour;
		const uint8_t bOpenMinute = e.bOpenMinute;

		if (bDay == pTimeInfo->tm_wday)
		{
			if (pTimeInfo->tm_hour >= bOpenHour && pTimeInfo->tm_min >= bOpenMinute)
				continue;

			const int iHour = (60 * 60 * (bOpenHour - pTimeInfo->tm_hour));
			const int iMin = (60 * (bOpenMinute - pTimeInfo->tm_min));

			openTime = iHour + iMin + pTimeInfo->tm_sec;
			if (openTime < 0)
				continue;

			break;
		}
	}

	if (openTime <= 0)
	{
		const uint8_t nextDay = pTimeInfo->tm_wday == EWeekDays::SATURDAY ? EWeekDays::SUNDAY : pTimeInfo->tm_wday + 1;
		for (auto& e : vecOpenTimeInfo)
		{
			const uint8_t bDay = e.bDay;
			const uint8_t bOpenHour = e.bOpenHour;
			const uint8_t bOpenMinute = e.bOpenMinute;

			if (bDay == nextDay)
			{
				const int iHour = (60 * 60 * ((24 - pTimeInfo->tm_hour - 1) + bOpenHour));
				const int iMin = (60 * ((60 - pTimeInfo->tm_min - 1) + bOpenMinute));

				openTime = iHour + iMin + pTimeInfo->tm_sec;
				break;
			}
		}
	}

	return openTime;
}

int CBattleField::GetCloseTime() noexcept
{
	const time_t iTime = time(nullptr);
	const tm* pTimeInfo = localtime(&iTime);

	int closeTime = 0;
	for (auto& e : vecOpenTimeInfo)
	{
		const uint8_t bDay = e.bDay;
		const uint8_t bCloseHour = e.bCloseHour;
		const uint8_t bCloseMinute = e.bCloseMinute;

		if (bDay == pTimeInfo->tm_wday && bCloseHour >= pTimeInfo->tm_hour)
		{
			if (pTimeInfo->tm_hour >= bCloseHour && pTimeInfo->tm_min >= bCloseMinute)
				continue;

			const int iHour = (60 * 60 * (bCloseHour - pTimeInfo->tm_hour));
			const int iMin = (60 * (bCloseMinute - pTimeInfo->tm_min));

			closeTime = iHour + iMin + pTimeInfo->tm_sec;
			if (closeTime < 0)
				continue;

			break;
		}
	}

	if (closeTime <= 0)
	{
		const uint8_t nextDay = pTimeInfo->tm_wday == EWeekDays::SATURDAY ? EWeekDays::SUNDAY : pTimeInfo->tm_wday + 1;
		for (auto& e : vecOpenTimeInfo)
		{
			const uint8_t bDay = e.bDay;
			const uint8_t bCloseHour = e.bCloseHour;
			const uint8_t bCloseMinute = e.bCloseMinute;

			if (bDay == nextDay)
			{
				const int iHour = (60 * 60 * ((24 - pTimeInfo->tm_hour - 1) + bCloseHour));
				const int iMin = (60 * ((60 - pTimeInfo->tm_min - 1) + bCloseMinute));

				closeTime = iHour + iMin + pTimeInfo->tm_sec;
				break;
			}
		}
	}

	return closeTime;
}

static LPEVENT s_pkNoticeEvent;

EVENTINFO(notice_event_data)
{
	int seconds;
	bool isClose;
	notice_event_data() noexcept :
		seconds(0), isClose(false)
	{}
};

EVENTFUNC(notice_event)
{
	notice_event_data* info = dynamic_cast<notice_event_data*>(event->info);

	if (info == nullptr)
	{
		sys_err("<notice_event_data> <Factor> Null pointer");
		return 0;
	}

	char buf[1024] = { 0 };
	int& pSec = info->seconds;
	if (pSec >= 0)
	{
		if (pSec > 50 && pSec % 60 == 0)
		{
			const int remain_time = (pSec / 60);
			if (info->isClose)
			{
				snprintf(buf, sizeof(buf), LC_TEXT("[104]Combat Zone will close in %d minute(s)."), remain_time);
				BroadcastNotice(buf);
			}
			else
			{
				snprintf(buf, sizeof(buf), LC_TEXT("[103]Combat Zone will open in %d minute(s)."), remain_time);
				BroadcastNotice(buf);
			}
		}

		--pSec;
		return PASSES_PER_SEC(1);
	}

	return 0;
}

void CBattleField::OpenEnter(bool isEvent, bool isForce)
{
	if (s_pkNoticeEvent)
		event_cancel(&s_pkNoticeEvent);

	if (isForce)
		bIsForceOpen = true;

	SetStatus(BATTLEFIELD_OPEN);
	BroadcastNotice(LC_TEXT("[101]Combat Zone is open."));
	BroadcastCommand("battle_field_open %d", true);

	if (isEvent)
		SetEventStatus(true);
}

struct FTeleportCity
{
	void operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(ent);
			CBattleField::Instance().ExitCharacter(pChar);
			//if ((pChar->IsPC()) && (pChar->GetGMLevel() == GM_PLAYER))
			//	pChar->WarpSet(EMPIRE_START_X(pChar->GetEmpire()), EMPIRE_START_Y(pChar->GetEmpire()));
		}
	}
};

void CBattleField::CloseEnter()
{
	if (s_pkNoticeEvent)
		event_cancel(&s_pkNoticeEvent);

	if (GetEventStatus())
		SetEventStatus(false);

	if (IsForceOpen())
		SetForceOpen(false);

	SetStatus(BATTLEFIELD_CLOSED);
	BroadcastNotice(LC_TEXT("[102]Combat Zone is closed."));
	BroadcastCommand("battle_field_open %d", false);
	BroadcastCommand("battle_field_event %d %d %d", GetEventStatus(), GetOpenTime(), GetCloseTime());

#ifdef ENABLE_RANKING_SYSTEM
	LoadRanking(RK_CATEGORY_BF);
#endif

	/*for (auto& e : vecCharacter)
	{
		LPCHARACTER pkChar = CHARACTER_MANAGER::Instance().FindByPID(e);
		if (pkChar != nullptr)
		{
			ExitCharacter(pkChar);
			//pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
		}
	}*/

	LPSECTREE_MAP pSectreeMap = SECTREE_MANAGER::Instance().GetMap(BATTLE_FIELD_MAP_INDEX);
	if (pSectreeMap != nullptr)
	{
		FTeleportCity pChar;
		pSectreeMap->for_each(pChar);
	}

	//vecCharacter.clear();
}

EVENTINFO(BattleZoneEventInfo)
{
	BattleZoneEventInfo() = default;
	DynamicCharacterPtr ch{};
	int left_second{ 0 };
	bool isExit{ 0 };
};

EVENTFUNC(battle_zone_event)
{
	BattleZoneEventInfo* info = dynamic_cast<BattleZoneEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("battle_field_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr)
		return 0;

	if (info->left_second <= 0)
	{
		ch->m_pkBattleZoneEvent = nullptr;

		if (info->isExit)
		{
			CBattleField::Instance().ExitCharacter(ch);
		}
		else
		{
			int x = 0;
			int y = 0;

			const PIXEL_POSITION tempPos = CBattleField::Instance().GetRandomPosition();

			x = tempPos.x;
			y = tempPos.y;

			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::Instance().GetMapBasePositionByMapIndex(BATTLE_FIELD_MAP_INDEX, pos))
			{
				x += pos.x / 100;
				y += pos.y / 100;
			}

			x *= 100;
			y *= 100;

			ch->WarpSet(x, y);
		}

		return 0;
	}
	else
	{
		if (info->left_second <= 10 && info->left_second > 0)
		{
			if (info->isExit)
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;125;%d]", info->left_second);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;126;%d]", info->left_second);
		}

		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

void CBattleField::ExitCharacter(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	const uint32_t dwBattleFieldPoints = pChar->GetBattleFieldPoint();
	if (dwBattleFieldPoints > 0)
	{
		pChar->PointChange(POINT_BATTLE_FIELD, dwBattleFieldPoints);
		pChar->SetBattleFieldPoint(0); // this is really necessary?
		RegisterBattleRanking(pChar, dwBattleFieldPoints);
	}

	SetPlayerCooldown(pChar->GetPlayerID());
	//RemovePlayerFromVector(pChar->GetPlayerID());

	pChar->WarpSet(EMPIRE_START_X(pChar->GetEmpire()), EMPIRE_START_Y(pChar->GetEmpire()));
}

void CBattleField::RequestEnter(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	if (pChar->IsDead())
		return;

	if (pChar->m_pkBattleZoneEvent)
		return;

	if (GetEnterStatus() == false)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;113]");
		return;
	}

	if (pChar->GetLevel() < MIN_LEVEL_FOR_ENTER)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;89;%d]", MIN_LEVEL_FOR_ENTER);
		return;
	}

	if ((g_bChannel == 99) || (pChar->GetMapIndex() >= 10000))
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;90]");
		return;
	}

	if (pChar->IsRiding())
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;312]");
		return;
	}

	if (!pChar->CanWarp())
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;91]");
		return;
	}

	// check cooldown
	quest::PC* qPC = quest::CQuestManager::Instance().GetPCForce(pChar->GetPlayerID());
	if (qPC)
	{
		time_t duration_time = qPC->GetFlag("battlefield.cooldown");
		if (get_global_time() < duration_time)
		{
			const uint32_t remainTime = (duration_time - get_global_time()) / 60;
			pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;95;%d]", remainTime);
			return;
		}
	}

	pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;93]");

	BattleZoneEventInfo* info = AllocEventInfo<BattleZoneEventInfo>();

	info->left_second = 0;
	info->ch = pChar;
	info->isExit = false;

	pChar->m_pkBattleZoneEvent = event_create(battle_zone_event, info, 1);
}

void CBattleField::RequestExit(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	if (pChar->m_pkBattleZoneEvent)
		return;

	if (!pChar->CanWarp())
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;113]");
		return;
	}

	const uint32_t dwBattleFieldPoint = pChar->GetBattleFieldPoint();
	const bool bCanExitDead = dwBattleFieldPoint == 0 ? true : false;
	int iExitSeconds = 3;

	if (!bCanExitDead)
		iExitSeconds = 120;

	if (pChar->IsDead() && bCanExitDead == true)
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;92]");
		return;
	}

	if (iExitSeconds > 15)
	{
		pChar->AddAffect(AFFECT_TARGET_VICTIM, POINT_NONE, 0, AFF_TARGET_VICTIM, iExitSeconds, 0, true);
		pChar->ComputePoints();

		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;107;%d]", (iExitSeconds / 60));
		SendNoticeMap("[LS;108]", BATTLE_FIELD_MAP_INDEX, true);
	}
	else
	{
		pChar->ChatPacket(CHAT_TYPE_INFO, "[LS;94]");
	}

	BattleZoneEventInfo* info = AllocEventInfo<BattleZoneEventInfo>();

	info->left_second = iExitSeconds;
	info->ch = pChar;
	info->isExit = true;

	pChar->m_pkBattleZoneEvent = event_create(battle_zone_event, info, 1);
}

void CBattleField::UpdateWeekRanking()
{
	/* get weekly winners */
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect),
		"SELECT pid, week_score FROM log.battle_score WHERE week_score > 0 ORDER BY week_score DESC LIMIT 3;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (uint8_t pos = 1; pos <= resSelect->uiNumRows; pos++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			if (!row)
				return;

			DBManager::Instance().DirectQuery("REPLACE INTO log.battle_week (pos, pid, score, last_update) VALUES (%d, %d, %d, NOW());",
				pos, atoi(row[0]), atoi(row[1]));
		}
	}

	/* update weekly points to total points */
	DBManager::Instance().DirectQuery("UPDATE log.battle_score SET total_score = total_score + week_score, week_score = 0;");
}

void CBattleField::SetWeakRankingPosition(LPCHARACTER pChar)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	if (true == IsBattleZoneMapIndex(pChar->GetMapIndex()))
		return;

	const uint8_t eBattleFieldEffect[3] =
	{
		/*AFFECT_BATTLE_RANKER_1,
		AFFECT_BATTLE_RANKER_2,
		AFFECT_BATTLE_RANKER_3*/
		AFF_BATTLE_RANKER_1,
		AFF_BATTLE_RANKER_2,
		AFF_BATTLE_RANKER_3
	};

	const uint32_t dwPlayerID = pChar->GetPlayerID();
	const int8_t bPosition = CRankingSystem::Instance().GetBFRankingPosition(dwPlayerID);
	if (bPosition >= 0)
		pChar->SetAffectFlag(eBattleFieldEffect[bPosition]);
}

void CBattleField::RegisterBattleRanking(LPCHARACTER& pChar, int iTempPoints)
{
	if (nullptr == pChar)
		return;

	if (false == pChar->IsPC())
		return;

	char querySelectTotal[256];

	snprintf(querySelectTotal, sizeof(querySelectTotal),
		"SELECT * FROM log.battle_score WHERE pid = %d;", pChar->GetPlayerID());

	auto pSelectTotalMsg(DBManager::Instance().DirectQuery(querySelectTotal));
	const SQLResult* resTotalSelect = pSelectTotalMsg->Get();

	if (resTotalSelect && resTotalSelect->uiNumRows > 0)
		DBManager::Instance().DirectQuery("UPDATE log.battle_score SET week_score = week_score + %d, last_update = NOW() WHERE pid = %d;", iTempPoints, pChar->GetPlayerID());
	else
		DBManager::Instance().DirectQuery("REPLACE INTO log.battle_score (pid, week_score, last_update) VALUES (%d, %d, NOW());", pChar->GetPlayerID(), iTempPoints);
}

void CBattleField::Update()
{
	if (GetStatus() == BATTLEFIELD_DISABLED)
		return;

	time_t iTime;
	time(&iTime);
	const tm* pTimeInfo = localtime(&iTime);

	/* Update Ranking */
	for (auto& e : vecRankingUpdateInfo)
	{
		const uint8_t bDay = e.bDay;
		const uint8_t bHour = e.bHour;
		const uint8_t bMinute = e.bMinute;
		const uint8_t bSecond = e.bSecond;

		if (bDay == pTimeInfo->tm_wday && bHour == pTimeInfo->tm_hour && bMinute == pTimeInfo->tm_min && bSecond == pTimeInfo->tm_sec)
		{
			UpdateWeekRanking();
#ifdef ENABLE_RANKING_SYSTEM
			LoadRanking(RK_CATEGORY_BF);
#endif
			break;
		}
	}

	/* Update Info */
	for (auto& e : vecOpenTimeInfo)
	{
		const uint8_t bDay = e.bDay;
		const uint8_t bOpenHour = e.bOpenHour;
		const uint8_t bOpenMinute = e.bOpenMinute;
		const uint8_t bCloseHour = e.bCloseHour;
		const uint8_t bCloseMinute = e.bCloseMinute;

		if (bDay == pTimeInfo->tm_wday)
		{
			if (GetOpenTime() > GetCloseTime() && !GetEnterStatus() && !IsForceOpen())
			{
				const bool isEvent = (GetEventMonth() == pTimeInfo->tm_mon && GetEventDay() == pTimeInfo->tm_mday) ? true : false;
				OpenEnter(isEvent);
				break;
			}

			if (GetOpenTime() < GetCloseTime() && GetEnterStatus() && !IsForceOpen())
			{
				CloseEnter();
				break;
			}
		}

		if (bDay == pTimeInfo->tm_wday && !s_pkNoticeEvent)
		{
			const int tempHour = GetEnterStatus() == false ? bOpenHour : bCloseHour;
			const int tempMinute = GetEnterStatus() == false ? bOpenMinute : bCloseMinute;

			const int iHour = tempMinute < 15 ? tempHour - 1 : tempHour;
			const int iMinute = tempMinute < 15 ? 60 + tempMinute - 15 : tempMinute - 15;

			if (pTimeInfo->tm_hour == iHour && pTimeInfo->tm_min == iMinute)
			{
				notice_event_data* info = AllocEventInfo<notice_event_data>();
				info->seconds = 15 * 60;
				info->isClose = GetEnterStatus();

				s_pkNoticeEvent = event_create(notice_event, info, 1);
				break;
			}
		}
	}
}
#endif
