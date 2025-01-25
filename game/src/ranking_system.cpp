#include "stdafx.h"

#ifdef ENABLE_RANKING_SYSTEM
#include "char.h"
#include "desc_client.h"
#include "db.h"

#include "ranking_system.h"

CRankingSystem::CRankingSystem()
{
}

CRankingSystem::~CRankingSystem()
{
	Destroy();
}

void CRankingSystem::Initialize()
{
#ifdef ENABLE_BATTLE_FIELD
	LoadBFRanking();
#endif
}

void CRankingSystem::Destroy()
{
#ifdef ENABLE_BATTLE_FIELD
	vecBattleFieldRanking.clear();
#endif
}

void CRankingSystem::LoadRanking(uint8_t bCategory)
{
	switch (bCategory)
	{
#ifdef ENABLE_BATTLE_FIELD
	case RK_CATEGORY_BF:
		LoadBFRanking();
		break;
#endif
	default:
		break;
	}
}

#ifdef ENABLE_BATTLE_FIELD
void CRankingSystem::LoadBFRanking()
{
	vecBattleFieldRanking.clear();

	LoadRankingBFWeek();
	LoadRankingBFTotal();
	LoadRankingWeekWinners();
}

void CRankingSystem::LoadRankingBFWeek()
{
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect),
		"SELECT p.name, e.empire, r.week_score FROM log.battle_score AS r INNER JOIN player.player AS p ON (r.pid = p.id) "
		"INNER JOIN player.player_index AS e ON (e.id = p.account_id) "
		"WHERE r.week_score > 0 "
		"ORDER BY r.week_score DESC LIMIT 10;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		TBattleRankingMember BattleRanking{};

		for (uint8_t i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			int col = 0;

			BattleRanking.bPosition = i;
			BattleRanking.bCategory = SOLO_RK_CATEGORY_BF_WEAK;

			strlcpy(BattleRanking.szName, row[col++], sizeof(BattleRanking.szName));
			str_to_number(BattleRanking.bEmpire, row[col++]);
			str_to_number(BattleRanking.lScore, row[col++]);

			vecBattleFieldRanking.emplace_back(BattleRanking);
		}
	}
}

void CRankingSystem::LoadRankingBFTotal()
{
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect),
		"SELECT p.name, e.empire, (r.total_score + r.week_score) FROM log.battle_score AS r INNER JOIN player.player AS p ON (r.pid = p.id) "
		"INNER JOIN player.player_index AS e ON (e.id = p.account_id) "
		"ORDER BY (r.total_score + r.week_score) DESC LIMIT 10;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		TBattleRankingMember BattleRanking{};

		for (uint8_t i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			int col = 0;

			BattleRanking.bPosition = i;
			BattleRanking.bCategory = SOLO_RK_CATEGORY_BF_TOTAL;

			strlcpy(BattleRanking.szName, row[col++], sizeof(BattleRanking.szName));
			str_to_number(BattleRanking.bEmpire, row[col++]);
			str_to_number(BattleRanking.lScore, row[col++]);

			vecBattleFieldRanking.emplace_back(BattleRanking);
		}
	}
}

void CRankingSystem::LoadRankingWeekWinners()
{
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect),
		"SELECT p.id FROM log.battle_week AS r INNER JOIN player.player AS p ON (r.pid = p.id) "
		"INNER JOIN player.player_index AS e ON (e.id = p.account_id) "
		"ORDER BY r.score DESC LIMIT 3;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	vecBattleFieldWeekRankingWinners.clear();
	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (uint i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			vecBattleFieldWeekRankingWinners.emplace_back(atoi(row[0]));
		}
	}
}

void CRankingSystem::SendBFRanking(LPCHARACTER pChar)
{
	if (!pChar)
		return;

	if (!pChar->GetDesc())
		return;

	TPacketGCBattleInfo packet{};
	packet.bHeader = HEADER_GC_BATTLE_ZONE_INFO;
	packet.wSize = static_cast<uint16_t>(sizeof(packet) + sizeof(TBattleRankingMember) * vecBattleFieldRanking.size());

	pChar->GetDesc()->BufferedPacket(&packet, sizeof(packet));
	pChar->GetDesc()->Packet(&vecBattleFieldRanking[0], sizeof(TBattleRankingMember) * vecBattleFieldRanking.size());
}

int8_t CRankingSystem::GetBFRankingPosition(uint32_t dwPlayerID)
{
	int8_t bPosition = 0;
	for (auto& e : vecBattleFieldWeekRankingWinners)
	{
		if (e == dwPlayerID)
			return bPosition;
		bPosition++;
	}
	return -1;
}
#endif


#endif
