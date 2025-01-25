
#include "stdafx.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "db.h"
#include "guild_manager.h"
#include "marriage.h"
#include "../../common/CommonDefines.h"

/*
   Return Value
		0 : 알 수 없는 에러 or 쿼리 에러
		1 : 동일한 제국으로 바꾸려고함
		2 : 길드 가입한 캐릭터가 있음
		3 : 결혼한 캐릭터가 있음

		999 : 제국 이동 성공
*/
int CHARACTER::ChangeEmpire(uint8_t empire)
{
	if (GetEmpire() == empire)
		return 1;

	char szQuery[1024+1];
	uint32_t dwAID;
	uint32_t dwPID[PLAYER_PER_ACCOUNT];
	memset(dwPID, 0, sizeof(dwPID));

	{
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
		snprintf(szQuery, sizeof(szQuery),
				"SELECT id, pid1, pid2, pid3, pid4, pid5 FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
				get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#else
		snprintf(szQuery, sizeof(szQuery),
				"SELECT id, pid1, pid2, pid3, pid4 FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u AND empire=%u",
				get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#endif
		auto msg(DBManager::Instance().DirectQuery(szQuery));

		if (msg->Get()->uiNumRows == 0)
		{
			return 0;
		}

		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		str_to_number(dwAID, row[0]);
		str_to_number(dwPID[0], row[1]);
		str_to_number(dwPID[1], row[2]);
		str_to_number(dwPID[2], row[3]);
		str_to_number(dwPID[3], row[4]);
	}

	const int loop = PLAYER_PER_ACCOUNT;

	{
		uint32_t dwGuildID[4];
		CGuild * pGuild[4];
		std::unique_ptr<SQLMsg> pMsg;

		for (int i = 0; i < loop; ++i)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT guild_id FROM guild_member%s WHERE pid=%u", get_table_postfix(), dwPID[i]);

			pMsg = DBManager::Instance().DirectQuery(szQuery);

			if (pMsg != nullptr)
			{
				if (pMsg->Get()->uiNumRows > 0)
				{
					MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

					str_to_number(dwGuildID[i], row[0]);

					pGuild[i] = CGuildManager::Instance().FindGuild(dwGuildID[i]);

					if (pGuild[i] != nullptr)
						return 2;
				}
				else
				{
					dwGuildID[i] = 0;
					pGuild[i] = nullptr;
				}
			}
		}
	}

	{
		for (int i = 0; i < loop; ++i)
		{
			if (marriage::CManager::Instance().IsEngagedOrMarried(dwPID[i]) == true)
				return 3;
		}
	}

	{
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
		snprintf(szQuery, sizeof(szQuery), "UPDATE player_index%s SET empire=%u WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
				get_table_postfix(), empire, GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#else
		snprintf(szQuery, sizeof(szQuery), "UPDATE player_index%s SET empire=%u WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u AND empire=%u",
				get_table_postfix(), empire, GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#endif
		auto msg(DBManager::Instance().DirectQuery(szQuery));

		if (msg->Get()->uiAffectedRows > 0)
		{
			SetChangeEmpireCount();
			//@fixme439
			SetEmpire(empire);
			UpdatePacket();
			//@end_fixme439
			return 999;
		}
	}

	return 0;
}

int CHARACTER::GetChangeEmpireCount() const
{
	char szQuery[1024+1];
	uint32_t dwAID = GetAID();

	if (dwAID == 0)
		return 0;

	snprintf(szQuery, sizeof(szQuery), "SELECT change_count FROM change_empire WHERE account_id = %u", dwAID);

	auto pMsg = DBManager::Instance().DirectQuery(szQuery);

	if (pMsg != nullptr)
	{
		if (pMsg->Get()->uiNumRows == 0)
			return 0;

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		uint32_t count = 0;
		str_to_number(count, row[0]);
		return count;
	}

	return 0;
}

void CHARACTER::SetChangeEmpireCount()
{
	char szQuery[1024+1];

	uint32_t dwAID = GetAID();

	if (dwAID == 0) return;

	int count = GetChangeEmpireCount();

	if (count == 0)
	{
		count++;
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO change_empire VALUES(%u, %d, NOW())", dwAID, count);
	}
	else
	{
		count++;
		snprintf(szQuery, sizeof(szQuery), "UPDATE change_empire SET change_count=%d WHERE account_id=%u", count, dwAID);
	}

	auto pmsg(DBManager::Instance().DirectQuery(szQuery));
}

uint32_t CHARACTER::GetAID() const
{
	char szQuery[1024+1];
	uint32_t dwAID = 0;
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u OR pid5=%u AND empire=%u",
			get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#else
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player_index%s WHERE pid1=%u OR pid2=%u OR pid3=%u OR pid4=%u AND empire=%u",
			get_table_postfix(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetPlayerID(), GetEmpire());
#endif
	auto pMsg = DBManager::Instance().DirectQuery(szQuery);

	if (pMsg != nullptr)
	{
		if (pMsg->Get()->uiNumRows == 0)
			return 0;

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		str_to_number(dwAID, row[0]);
		return dwAID;
	}
	else
	{
		return 0;
	}
}

