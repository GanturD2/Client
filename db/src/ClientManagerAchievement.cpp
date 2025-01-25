#include "stdafx.h"

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include "ClientManager.h"
#include "Cache.h"
#include "HB.h"
#include "Main.h"
#include "QID.h"
#include "../../libachievement/serializer.hpp"
#include "../../libachievement/deserializer.hpp"

void CClientManager::RESULT_ACHIEVEMENT_LOAD(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD pid, TPlayerAchievementInfo& pAchTable)
{
	int iNumRows;
	if ((iNumRows = mysql_num_rows(pRes)) == 0)
	{
		auto achievementsBuffer = SERIALIZE_NAMESPACE::serialize(pAchTable.achievements);
		uint32_t achievementsBufferSize = achievementsBuffer.size();
		uint8_t subHeader = ACHIEVEMENT_SUBHEADER_LOGIN;

		peer->EncodeHeader(HEADER_DG_ACHIEVEMENT, dwHandle,
			sizeof(uint8_t) + sizeof(uint32_t) +
			sizeof(uint32_t) +
			sizeof(uint32_t) + sizeof(uint32_t) +
			achievementsBuffer.size());

		peer->Encode(&subHeader);
		peer->Encode(&pid);
		peer->Encode(&pAchTable.points);
		peer->Encode(&pAchTable.title);
		peer->Encode(&achievementsBufferSize);
		peer->Encode(achievementsBuffer.data(), achievementsBuffer.size());
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(pRes);
	uint32_t dwAchievementPoints = 0, dwSelectedTitle = 0;

	str_to_number(pAchTable.points, row[0]);
	str_to_number(pAchTable.title, row[1]);

	char szQuery[2048];
	snprintf(szQuery, sizeof(szQuery),"SELECT `achievement`, `finished` FROM `achievements` WHERE `pid` = %d;", pid);
	CDBManager::instance().ReturnQuery(szQuery, QID_ACHIEVEMENT_LOAD_ACHIEVEMENTS, peer->GetHandle(), new ClientHandleInfo(dwHandle, pid, 0, pAchTable));
}

void CClientManager::RESULT_ACHIEVEMENT_LOAD_ACHIEVEMENTS(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD pid, TPlayerAchievementInfo& pAchTable)
{
	int iNumRows;
	if ((iNumRows = mysql_num_rows(pRes)) == 0)
	{
		auto achievementsBuffer = SERIALIZE_NAMESPACE::serialize(pAchTable.achievements);
		uint32_t achievementsBufferSize = achievementsBuffer.size();
		uint8_t subHeader = ACHIEVEMENT_SUBHEADER_LOGIN;

		peer->EncodeHeader(HEADER_DG_ACHIEVEMENT, dwHandle,
			sizeof(uint8_t) + sizeof(uint32_t) +
			sizeof(uint32_t) +
			sizeof(uint32_t) + sizeof(uint32_t) +
			achievementsBuffer.size());

		peer->Encode(&subHeader);
		peer->Encode(&pid);
		peer->Encode(&pAchTable.points);
		peer->Encode(&pAchTable.title);
		peer->Encode(&achievementsBufferSize);
		peer->Encode(achievementsBuffer.data(), achievementsBuffer.size());
		return;
	}

	MYSQL_ROW row;
	for (int i = 0; i < iNumRows; ++i)
	{
		row = mysql_fetch_row(pRes);
		uint32_t achievement_id = 0;
		uint64_t finished = 0;
		TTasksList Tasks;
		Tasks.clear();

		str_to_number(achievement_id, row[0]);
		str_to_number(finished, row[1]);

		if (finished > 0)
			Tasks.insert({ 0, finished });

		pAchTable.achievements.insert({ achievement_id, Tasks });
	}

	char szQuery[2048];
	snprintf(szQuery, sizeof(szQuery), "SELECT `achievement_id`, `task_id`, `task_value` FROM `achievement_tasks` WHERE `pid` = %d;", pid);
	CDBManager::instance().ReturnQuery(szQuery, QID_ACHIEVEMENT_LOAD_TASKS, peer->GetHandle(), new ClientHandleInfo(dwHandle, pid, 0, pAchTable));
}

void CClientManager::RESULT_ACHIEVEMENT_LOAD_TASKS(CPeer* peer, MYSQL_RES* pRes, DWORD dwHandle, DWORD pid, TPlayerAchievementInfo& pAchTable)
{
	int iNumRows = mysql_num_rows(pRes);
	MYSQL_ROW row;
	for (int i = 0; i < iNumRows; ++i)
	{
		row = mysql_fetch_row(pRes);
		uint32_t achievement_id;
		uint32_t task_id;
		uint64_t task_value;
		str_to_number(achievement_id, row[0]);
		str_to_number(task_id, row[1]);
		str_to_number(task_value, row[2]);

		const auto& it = pAchTable.achievements.find(achievement_id);
		if (it != pAchTable.achievements.end())
			it->second.insert({ task_id, task_value });
	}

	auto achievementsBuffer = SERIALIZE_NAMESPACE::serialize(pAchTable.achievements);
	uint32_t achievementsBufferSize = achievementsBuffer.size();
	uint8_t subHeader = ACHIEVEMENT_SUBHEADER_LOGIN;

	peer->EncodeHeader(HEADER_DG_ACHIEVEMENT, dwHandle,
		sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) +
		sizeof(uint32_t) +
		sizeof(uint32_t) + achievementsBuffer.size());

	peer->Encode(&subHeader);
	peer->Encode(&pid);
	peer->Encode(&pAchTable.points);
	peer->Encode(&pAchTable.title);
	peer->Encode(&achievementsBufferSize);
	peer->Encode(achievementsBuffer.data(), achievementsBuffer.size());
}

void CClientManager::QUERY_PLAYER_ACHIEVEMENTS_LOAD(CPeer* peer, DWORD dwHandle, TPlayerLoadPacket* packet)
{
	auto it = m_map_AchievementCache.find(packet->player_id);
	if (it == m_map_AchievementCache.end())
	{
		char szQuery[2048];
		snprintf(szQuery, sizeof(szQuery), "SELECT `points`, `title` FROM `achievement_data` WHERE `pid` = %d LIMIT 1;", packet->player_id);

		TPlayerAchievementInfo pAchTable;
		pAchTable.achievements.clear();
		pAchTable.pid = packet->player_id;
		pAchTable.points = pAchTable.title = 0;

		CDBManager::instance().ReturnQuery(szQuery, QID_ACHIEVEMENT_LOAD_DATA, peer->GetHandle(), new ClientHandleInfo(dwHandle, packet->player_id, packet->account_id, pAchTable));
	}
	else
	{
		const auto& Ach = it->second->Get();
		auto achievementsBuffer = SERIALIZE_NAMESPACE::serialize(Ach->achievements);
		uint32_t achievementsBufferSize = achievementsBuffer.size();
		uint8_t subHeader = ACHIEVEMENT_SUBHEADER_LOGIN;

		peer->EncodeHeader(HEADER_DG_ACHIEVEMENT, dwHandle,
			sizeof(uint8_t) + sizeof(uint32_t) +
			sizeof(uint32_t) + sizeof(uint32_t) +
			sizeof(uint32_t) +
			achievementsBuffer.size());

		peer->Encode(&subHeader);
		peer->Encode(&packet->player_id);
		peer->Encode(&Ach->points);
		peer->Encode(&Ach->title);
		peer->Encode(&achievementsBufferSize);
		peer->Encode(achievementsBuffer.data(), achievementsBuffer.size());
	}
}

void CClientManager::UpdateAchievementsCache()
{
	auto it = m_map_AchievementCache.begin();
	while (it != m_map_AchievementCache.end())
	{
		auto pAchCache = *it;
		if (pAchCache.second->CheckFlushTimeout())
		{
			pAchCache.second->Flush();
			delete pAchCache.second;
			it = m_map_AchievementCache.erase(it);
		}

		else ++it;
	}
}

void CClientManager::UpdateAchievementsRanking()
{
	char szQuery[350];
	snprintf(szQuery, sizeof(szQuery), "SELECT `a`.`pid`, `p`.`name`, `p`.`level`, `g`.`name`, COUNT(`a`.`finished`) FROM `achievements` a LEFT JOIN `player` `p` ON `p`.`id` = `a`.`pid` LEFT JOIN guild_member gm on gm.pid = p.id LEFT JOIN guild g on g.id = gm.guild_id WHERE `a`.`finished` > 0 AND `p`.`name` NOT LIKE '[%%' GROUP BY `a`.`pid` ORDER BY COUNT(`a`.`finished`) DESC LIMIT 50;");

	std::unique_ptr<SQLMsg> qAchievements(CDBManager::Instance().DirectQuery(szQuery, SQL_PLAYER));

	m_dwAchievementRanking.clear();

	if (qAchievements->Get()->uiNumRows && qAchievements->Get()->pSQLResult)
	{
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(qAchievements->Get()->pSQLResult)))
		{
			TAchievementRanking rank;

			uint32_t pid = 0;
			str_to_number(pid, row[0]);

			if (row[1])
				strlcpy(rank.szName, row[1], sizeof(rank.szName));
			else
				strlcpy(rank.szName, "Unknown", sizeof(rank.szName));

			str_to_number(rank.level, row[2]);
			if (row[3])
				strlcpy(rank.szGuild, row[3], sizeof(rank.szGuild));
			else
				strlcpy(rank.szGuild, "-", sizeof(rank.szGuild));

			str_to_number(rank.finished, row[4]);

			m_dwAchievementRanking.push_back(rank);
		}
	}

	sys_log(0, "Achievements ranking has been refreshed.");
}

void CClientManager::RecvAchievementPacket(CPeer* peer, DWORD dwHandle, const char* c_pData)
{
	uint8_t bSubHeader = *(uint8_t*)c_pData;
	c_pData += sizeof(uint8_t);

	switch (bSubHeader)
	{
		case ACHIEVEMENT_SUBHEADER_LOGOUT:	// Player logout
		{
			TPlayerAchievementInfo ach;
			ach.pid = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			ach.points = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			ach.title = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			uint32_t achievementsBufferSize = *(uint32_t*)c_pData;
			c_pData += sizeof(uint32_t);

			ach.achievements = SERIALIZE_NAMESPACE::deserialize<TAchievementsMap>(c_pData, achievementsBufferSize);

			auto it = m_map_AchievementCache.find(ach.pid);
			if (it == m_map_AchievementCache.end())
			{
				CAchievementCache* pCache = new CAchievementCache(false);
				pCache->Put(ach, false);
				m_map_AchievementCache.insert({ ach.pid, pCache });
			}
			else
			{
				it->second->Put(ach, false);
			}
		}
		break;

		case ACHIEVEMENT_SUBHEADER_RANKING:
		{
			uint8_t subHeader = ACHIEVEMENT_SUBHEADER_RANKING;
			uint32_t rank_size = m_dwAchievementRanking.size();

			peer->EncodeHeader(HEADER_DG_ACHIEVEMENT, dwHandle,
				sizeof(uint8_t) + sizeof(uint32_t) +
				sizeof(TAchievementRanking) * rank_size);

			peer->Encode(&subHeader, sizeof(uint8_t));
			peer->Encode(&rank_size, sizeof(uint32_t));

			for (const auto& r : m_dwAchievementRanking)
			{
				peer->Encode(&r, sizeof(TAchievementRanking));
			}
		}
		break;

		default:
			break;
	}
}
#endif
