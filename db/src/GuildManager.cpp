#include "stdafx.h"
#include "GuildManager.h"
#include "Main.h"
#include "ClientManager.h"
#include "QID.h"
#include "Config.h"
#include <math.h>

extern std::string g_stLocale;

constexpr int GUILD_RANK_MAX_NUM = 20;

bool isEurope() noexcept
{
	do
	{
		if (g_stLocale.compare("germany") == 0)
			break;
		if (g_stLocale.compare("france") == 0)
			break;
		if (g_stLocale.compare("italy") == 0)
			break;
		if (g_stLocale.compare("spain") == 0)
			break;
		if (g_stLocale.compare("uk") == 0)
			break;
		if (g_stLocale.compare("turkey") == 0)
			break;
		if (g_stLocale.compare("poland") == 0)
			break;
		if (g_stLocale.compare("portugal") == 0)
			break;
		if (g_stLocale.compare("greek") == 0)
			break;

		return false;
	} while (false);

	return true;
}

uint32_t GetGuildWarWaitStartDuration()
{
	// const int GUILD_WAR_WAIT_START_DURATION = 60;
	// const int GUILD_WAR_WAIT_START_DURATION = 5;

	if (isEurope() == true) return 60;
	else return 5;
}

uint32_t GetGuildWarReserveSeconds()
{
	// const int GUILD_WAR_RESERVE_SECONDS = 180;
	// const int GUILD_WAR_RESERVE_SECONDS = 10;

	if (isEurope() == true) return 180;
	else return 10;
}

namespace
{
	struct FSendPeerWar
	{
#ifdef ENABLE_NEW_WAR_OPTIONS
		FSendPeerWar(uint8_t bType, uint8_t bRound, uint8_t bPoints, uint8_t bTime, uint8_t bWar, uint32_t GID1, uint32_t GID2)
#else
		FSendPeerWar(uint8_t bType, uint8_t bWar, uint32_t GID1, uint32_t GID2)
#endif
		{
			if (number(0, 1))
				std::swap(GID1, GID2);

			memset(&p, 0, sizeof(TPacketGuildWar));

			p.bWar = bWar;
			p.bType = bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
			p.bRound = bRound;
			p.bPoints = bPoints;
			p.bTime = bTime;
#endif
			p.dwGuildFrom = GID1;
			p.dwGuildTo = GID2;
		}

		void operator() (CPeer* peer)
		{
			if (peer->GetChannel() == 0)
				return;

			peer->EncodeHeader(HEADER_DG_GUILD_WAR, 0, sizeof(TPacketGuildWar));
			peer->Encode(&p, sizeof(TPacketGuildWar));
		}

		TPacketGuildWar p;
	};

	struct FSendGuildWarScore
	{
		FSendGuildWarScore(uint32_t guild_gain, uint32_t dwOppGID, int iScore, int iBetScore)
		{
			pck.dwGuildGainPoint = guild_gain;
			pck.dwGuildOpponent = dwOppGID;
			pck.lScore = iScore;
			pck.lBetScore = iBetScore;
		}

		void operator() (CPeer* peer)
		{
			if (peer->GetChannel() == 0)
				return;

			peer->EncodeHeader(HEADER_DG_GUILD_WAR_SCORE, 0, sizeof(pck));
			peer->Encode(&pck, sizeof(pck));
		}

		TPacketGuildWarScore pck;
	};
}

CGuildManager::CGuildManager()
{
}

CGuildManager::~CGuildManager()
{
	while (!m_pqOnWar.empty())
	{
		if (!m_pqOnWar.top().second->bEnd)
			delete m_pqOnWar.top().second;

		m_pqOnWar.pop();
	}
}

TGuild& CGuildManager::TouchGuild(uint32_t GID)
{
	const auto it = m_map_kGuild.find(GID);

	if (it != m_map_kGuild.end())
		return it->second;

	TGuild info;
	m_map_kGuild.insert(std::map<uint32_t, TGuild>::value_type(GID, info));
	return m_map_kGuild[GID];
}

void CGuildManager::ParseResult(SQLResult* pRes)
{
	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		const uint32_t GID = strtoul(row[0], nullptr, 10);

		TGuild& r_info = TouchGuild(GID);

		strlcpy(r_info.szName, row[1], sizeof(r_info.szName));
		str_to_number(r_info.ladder_point, row[2]);
		str_to_number(r_info.win, row[3]);
		str_to_number(r_info.draw, row[4]);
		str_to_number(r_info.loss, row[5]);
		str_to_number(r_info.gold, row[6]);
		str_to_number(r_info.level, row[7]);
#ifdef ENABLE_GUILD_WAR_SCORE
		for (int i = 0; i < 3; ++i) {
			str_to_number(r_info.winNew[i], row[i+8]);
		}
		for (int i = 0; i < 3; ++i) {
			str_to_number(r_info.drawNew[i], row[i + 11]);
		}
		for (int i = 0; i < 3; ++i) {
			str_to_number(r_info.lossNew[i], row[i + 14]);
		}
#endif

		sys_log(0,
			"GuildWar: %-24s ladder %-5d win %-3d draw %-3d loss %-3d",
			r_info.szName,
			r_info.ladder_point,
			r_info.win,
			r_info.draw,
			r_info.loss);
	}
}

void CGuildManager::Initialize()
{
	char szQuery[1024];
#ifdef ENABLE_GUILD_WAR_SCORE
	snprintf(szQuery, sizeof(szQuery), "SELECT id, name, ladder_point, win, draw, loss, gold, level, wintype0, wintype1, wintype2, drawtype0, drawtype1, drawtype2, losstype0, losstype1, losstype2 FROM guild%s", GetTablePostfix());
#else
	snprintf(szQuery, sizeof(szQuery), "SELECT id, name, ladder_point, win, draw, loss, gold, level FROM guild%s", GetTablePostfix());
#endif
	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));

	if (pmsg->Get()->uiNumRows)
		ParseResult(pmsg->Get());

	char str[128 + 1];

	if (!CConfig::Instance().GetValue("POLY_POWER", str, sizeof(str)))
		*str = '\0';

	if (!polyPower.Analyze(str))
		sys_err("cannot set power poly: %s", str);
	else
		sys_log(0, "POWER_POLY: %s", str);

	if (!CConfig::Instance().GetValue("POLY_HANDICAP", str, sizeof(str)))
		*str = '\0';

	if (!polyHandicap.Analyze(str))
		sys_err("cannot set handicap poly: %s", str);
	else
		sys_log(0, "HANDICAP_POLY: %s", str);

	QueryRanking();
}

void CGuildManager::Load(uint32_t dwGuildID)
{
	char szQuery[1024];

#ifdef ENABLE_GUILD_WAR_SCORE
	snprintf(szQuery, sizeof(szQuery), "SELECT id, name, ladder_point, win, draw, loss, gold, level, wintype0, wintype1, wintype2, drawtype0, drawtype1, drawtype2, losstype0, losstype1, losstype2 FROM guild%s WHERE id=%u", GetTablePostfix(), dwGuildID);
#else
	snprintf(szQuery, sizeof(szQuery), "SELECT id, name, ladder_point, win, draw, loss, gold, level FROM guild%s WHERE id=%u", GetTablePostfix(), dwGuildID);
#endif

	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));
	if (pmsg->Get()->uiNumRows)
		ParseResult(pmsg->Get());
}

void CGuildManager::QueryRanking()
{
	char szQuery[256];
	snprintf(szQuery, sizeof(szQuery), "SELECT id,name,ladder_point FROM guild%s ORDER BY ladder_point DESC LIMIT 20", GetTablePostfix());

	CDBManager::Instance().ReturnQuery(szQuery, QID_GUILD_RANKING, 0, 0);
}

int CGuildManager::GetRanking(uint32_t dwGID)
{
	const auto it = map_kLadderPointRankingByGID.find(dwGID);

	if (it == map_kLadderPointRankingByGID.end())
		return GUILD_RANK_MAX_NUM;

	return MINMAX(0, it->second, GUILD_RANK_MAX_NUM);
}

void CGuildManager::ResultRanking(MYSQL_RES* pRes)
{
	if (!pRes)
		return;

	const int iLastLadderPoint = -1;
	int iRank = 0;

	map_kLadderPointRankingByGID.clear();

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pRes)))
	{
		uint32_t dwGID = 0;
		str_to_number(dwGID, row[0]);
		int iLadderPoint = 0;
		str_to_number(iLadderPoint, row[2]);

		if (iLadderPoint != iLastLadderPoint)
			++iRank;

		sys_log(0, "GUILD_RANK: %-24s %2d %d", row[1], iRank, iLadderPoint);

		map_kLadderPointRankingByGID.insert(std::make_pair(dwGID, iRank));
	}
}

void CGuildManager::Update()
{
	ProcessReserveWar(); // Reserve War Handling

	const time_t now = CClientManager::Instance().GetCurrentTime();

	if (!m_pqOnWar.empty())
	{
		// UNKNOWN_GUILD_MANAGE_UPDATE_LOG
		/*
		sys_log(0, "GuildManager::Update size %d now %d top %d, %s(%u) vs %s(%u)",
		m_WarMap.size(),
		now,
		m_pqOnWar.top().first,
		m_map_kGuild[m_pqOnWar.top().second->GID[0]].szName,
		m_pqOnWar.top().second->GID[0],
		m_map_kGuild[m_pqOnWar.top().second->GID[1]].szName,
		m_pqOnWar.top().second->GID[1]);
		*/
		// END_OF_UNKNOWN_GUILD_MANAGE_UPDATE_LOG

		while (!m_pqOnWar.empty() && (m_pqOnWar.top().first <= now || (m_pqOnWar.top().second && m_pqOnWar.top().second->bEnd)))
		{
			TGuildWarPQElement* e = m_pqOnWar.top().second;

			m_pqOnWar.pop();

			if (e)
			{
				if (!e->bEnd)
					WarEnd(e->GID[0], e->GID[1], false);

				delete e;
			}
		}
	}

	// GUILD_SKILL_COOLTIME_BUG_FIX
	while (!m_pqSkill.empty() && m_pqSkill.top().first <= now)
	{
		const TGuildSkillUsed& s = m_pqSkill.top().second;
		CClientManager::Instance().SendGuildSkillUsable(s.GID, s.dwSkillVnum, true);
		m_pqSkill.pop();
	}
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX

	while (!m_pqWaitStart.empty() && m_pqWaitStart.top().first <= now)
	{
		const TGuildWaitStartInfo& ws = m_pqWaitStart.top().second;
		m_pqWaitStart.pop();

#ifdef ENABLE_NEW_WAR_OPTIONS
		StartWar(ws.bType, ws.GID[0], ws.GID[1], ws.bRound, ws.bPoints, ws.bTime, ws.pkReserve); // insert new element to m_WarMap and m_pqOnWar
#else
		StartWar(ws.bType, ws.GID[0], ws.GID[1], ws.pkReserve); // insert new element to m_WarMap and m_pqOnWar
#endif

		if (ws.lInitialScore)
		{
			UpdateScore(ws.GID[0], ws.GID[1], ws.lInitialScore, 0);
			UpdateScore(ws.GID[1], ws.GID[0], ws.lInitialScore, 0);
		}

		TPacketGuildWar p{};

		p.bType		= ws.bType;
		p.bWar		= GUILD_WAR_ON_WAR;
#ifdef ENABLE_NEW_WAR_OPTIONS
		p.bRound	= ws.bRound;
		p.bPoints	= ws.bPoints;
		p.bTime		= ws.bTime;
#endif
		p.dwGuildFrom	= ws.GID[0];
		p.dwGuildTo	= ws.GID[1];

		CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_WAR, &p, sizeof(p));
		sys_log(0, "GuildWar: GUILD sending start of wait start war %d %d", ws.GID[0], ws.GID[1]);
	}
}

#define for_all(cont, it) for (typeof((cont).begin()) it = (cont).begin(); it != (cont).end(); ++it)

void CGuildManager::OnSetup(CPeer* peer)
{
	for_all(m_WarMap, it_cont)
		for_all(it_cont->second, it)
	{
		const uint32_t g1 = it_cont->first;
		const uint32_t g2 = it->first;
		const TGuildWarPQElement* p = it->second.pElement;

		if (!p || p->bEnd)
			continue;

#ifdef ENABLE_NEW_WAR_OPTIONS
		FSendPeerWar(p->bType, p->bRound, p->bPoints, p->bTime, GUILD_WAR_ON_WAR, g1, g2) (peer);
#else
		FSendPeerWar(p->bType, GUILD_WAR_ON_WAR, g1, g2) (peer);
#endif
		FSendGuildWarScore(p->GID[0], p->GID[1], p->iScore[0], p->iBetScore[0]);
		FSendGuildWarScore(p->GID[1], p->GID[0], p->iScore[1], p->iBetScore[1]);
	}

	for_all(m_DeclareMap, it)
	{
#ifdef ENABLE_NEW_WAR_OPTIONS
		FSendPeerWar(it->bType, it->bRound, it->bPoints, it->bTime, GUILD_WAR_SEND_DECLARE, it->dwGuildID[0], it->dwGuildID[1]) (peer);
#else
		FSendPeerWar(it->bType, GUILD_WAR_SEND_DECLARE, it->dwGuildID[0], it->dwGuildID[1]) (peer);
#endif
	}

	for_all(m_map_kWarReserve, it)
	{
		it->second->OnSetup(peer);
	}
}

#ifdef ENABLE_GUILD_WAR_SCORE
int CGuildManager::GetGuildWarType(int GID)
{
	char Query[1024];
	snprintf(Query, sizeof(Query), "SELECT type FROM player.guild_war_reservation WHERE guild1 = %d or guild2 = %d", GID, GID);
	auto pmsg(CDBManager::Instance().DirectQuery(Query));

	if (pmsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
	int warType = 0;

	if (row[0] == 0)
		str_to_number(warType, 0);
	else
		str_to_number(warType, row[0]);

	sys_log(0, "GetGuildWarType: %d", warType);
	return warType;
}
#endif

void CGuildManager::GuildWarWin(uint32_t GID)
{
	const auto it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	++it->second.win;
#ifdef ENABLE_GUILD_WAR_SCORE
	char winType[1024];
	int _type = GetGuildWarType(GID);
	++it->second.winNew[_type];
	snprintf(winType, sizeof(winType), "wintype%d", _type);
#endif

	char buf[1024];
#ifdef ENABLE_GUILD_WAR_SCORE
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET win=%d, wintype%d = %d WHERE id=%u", GetTablePostfix(), it->second.win, _type, it->second.winNew[_type], GID);
#else
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET win=%d WHERE id=%u", GetTablePostfix(), it->second.win, GID);
#endif
	CDBManager::Instance().AsyncQuery(buf);
}

void CGuildManager::GuildWarLose(uint32_t GID)
{
	const auto it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	++it->second.loss;
#ifdef ENABLE_GUILD_WAR_SCORE
	char lossType[1024];
	int _type = GetGuildWarType(GID);
	++it->second.lossNew[_type];
	snprintf(lossType, sizeof(lossType), "losstype%d", _type);
#endif

	char buf[1024];
#ifdef ENABLE_GUILD_WAR_SCORE
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET loss=%d, losstype%d = %d WHERE id=%u", GetTablePostfix(), it->second.loss, _type, it->second.lossNew[_type], GID);
#else
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET loss=%d WHERE id=%u", GetTablePostfix(), it->second.loss, GID);
#endif
	CDBManager::Instance().AsyncQuery(buf);
}

void CGuildManager::GuildWarDraw(uint32_t GID)
{
	const auto it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	++it->second.draw;
#ifdef ENABLE_GUILD_WAR_SCORE
	char drawType[1024];
	int _type = GetGuildWarType(GID);
	++it->second.drawNew[_type];
	snprintf(drawType, sizeof(drawType), "drawtype%d", _type);
#endif

	char buf[1024];
#ifdef ENABLE_GUILD_WAR_SCORE
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET draw=%d, drawtype%d = %d WHERE id=%u", GetTablePostfix(), it->second.draw, _type, it->second.drawNew[_type], GID);
#else
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET draw=%d WHERE id=%u", GetTablePostfix(), it->second.draw, GID);
#endif
	CDBManager::Instance().AsyncQuery(buf);
}

bool CGuildManager::IsHalfWinLadderPoint(uint32_t dwGuildWinner, uint32_t dwGuildLoser)
{
	uint32_t GID1 = dwGuildWinner;
	uint32_t GID2 = dwGuildLoser;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	const auto it = m_mapGuildWarEndTime[GID1].find(GID2);

	if (it != m_mapGuildWarEndTime[GID1].end() &&
		it->second + GUILD_WAR_LADDER_HALF_PENALTY_TIME > CClientManager::Instance().GetCurrentTime())
		return true;

	return false;
}

void CGuildManager::ProcessDraw(uint32_t dwGuildID1, uint32_t dwGuildID2)
{
	sys_log(0, "GuildWar: \tThe war between %d and %d is ended in draw", dwGuildID1, dwGuildID2);

	GuildWarDraw(dwGuildID1);
	GuildWarDraw(dwGuildID2);
	ChangeLadderPoint(dwGuildID1, 0);
	ChangeLadderPoint(dwGuildID2, 0);

	QueryRanking();
}

void CGuildManager::ProcessWinLose(uint32_t dwGuildWinner, uint32_t dwGuildLoser)
{
	GuildWarWin(dwGuildWinner);
	GuildWarLose(dwGuildLoser);
	sys_log(0, "GuildWar: \tWinner : %d Loser : %d", dwGuildWinner, dwGuildLoser);

	const int iPoint = GetLadderPoint(dwGuildLoser);
	int gain = static_cast<int>(iPoint * 0.05);
	const int loss = static_cast<int>(iPoint * 0.07);

	if (IsHalfWinLadderPoint(dwGuildWinner, dwGuildLoser))
		gain /= 2;

	sys_log(0, "GuildWar: \tgain : %d loss : %d", gain, loss);

	ChangeLadderPoint(dwGuildWinner, gain);
	ChangeLadderPoint(dwGuildLoser, -loss);

	QueryRanking();
}

void CGuildManager::RemoveWar(uint32_t GID1, uint32_t GID2)
{
	sys_log(0, "GuildWar: RemoveWar(%d, %d)", GID1, GID2);

	if (GID1 > GID2)
		std::swap(GID2, GID1);

	const auto it = m_WarMap[GID1].find(GID2);

	if (it == m_WarMap[GID1].end())
	{
		if (m_WarMap[GID1].empty())
			m_WarMap.erase(GID1);

		return;
	}

	if (it->second.pElement)
		it->second.pElement->bEnd = true;

	m_mapGuildWarEndTime[GID1][GID2] = CClientManager::Instance().GetCurrentTime();

	m_WarMap[GID1].erase(it);

	if (m_WarMap[GID1].empty())
		m_WarMap.erase(GID1);
}

//
// Abnormal end of guild battle and end of field battle
//
void CGuildManager::WarEnd(uint32_t GID1, uint32_t GID2, bool bForceDraw)
{
	if (GID1 > GID2)
		std::swap(GID2, GID1);

	sys_log(0, "GuildWar: WarEnd %d %d", GID1, GID2);

	const auto itWarMap = m_WarMap[GID1].find(GID2);

	if (itWarMap == m_WarMap[GID1].end())
	{
		sys_err("GuildWar: war not exist or already ended. [1]");
		return;
	}

	const TGuildWarInfo gwi = itWarMap->second;
	const TGuildWarPQElement* pData = gwi.pElement;

	if (!pData || pData->bEnd)
	{
		sys_err("GuildWar: war not exist or already ended. [2]");
		return;
	}

	uint32_t win_guild = pData->GID[0];
	uint32_t lose_guild = pData->GID[1];

	bool bDraw = false;

	if (!bForceDraw) // If it is not a forced draw, the score is checked.
	{
		if (pData->iScore[0] > pData->iScore[1])
		{
			win_guild = pData->GID[0];
			lose_guild = pData->GID[1];
		}
		else if (pData->iScore[1] > pData->iScore[0])
		{
			win_guild = pData->GID[1];
			lose_guild = pData->GID[0];
		}
		else
			bDraw = true;
	}
	else // In case of forced draw, unconditional draw
		bDraw = true;

	if (bDraw)
		ProcessDraw(win_guild, lose_guild);
	else
		ProcessWinLose(win_guild, lose_guild);

	// Since the DB server may end itself by itself, a separate packet must be sent.
#ifdef ENABLE_NEW_WAR_OPTIONS
	CClientManager::Instance().for_each_peer(FSendPeerWar(0, 0, 0, 0, GUILD_WAR_END, GID1, GID2));
#else
	CClientManager::Instance().for_each_peer(FSendPeerWar(0, GUILD_WAR_END, GID1, GID2));
#endif

	RemoveWar(GID1, GID2);
}

//
// Guild War ends normally
//
void CGuildManager::RecvWarOver(uint32_t dwGuildWinner, uint32_t dwGuildLoser, bool bDraw, long lWarPrice)
{
	sys_log(0, "GuildWar: RecvWarOver : winner %u vs %u draw? %d war_price %d", dwGuildWinner, dwGuildLoser, bDraw ? 1 : 0, lWarPrice);

	uint32_t GID1 = dwGuildWinner;
	uint32_t GID2 = dwGuildLoser;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	const auto it = m_WarMap[GID1].find(GID2);

	if (it == m_WarMap[GID1].end())
		return;

	TGuildWarInfo& gw = it->second;

	// Award
	if (bDraw)
	{
		// give bet money / 2 to both guild
		DepositMoney(dwGuildWinner, lWarPrice / 2);
		DepositMoney(dwGuildLoser, lWarPrice / 2);
		ProcessDraw(dwGuildWinner, dwGuildLoser);
	}
	else
	{
		// give bet money to winner guild
		DepositMoney(dwGuildWinner, lWarPrice);
		ProcessWinLose(dwGuildWinner, dwGuildLoser);
	}

	if (gw.pkReserve)
	{
		if (bDraw || !gw.pElement)
			gw.pkReserve->Draw();
		else if (gw.pElement->bType == GUILD_WAR_TYPE_BATTLE)
			gw.pkReserve->End(gw.pElement->iBetScore[0], gw.pElement->iBetScore[1]);
	}

	RemoveWar(GID1, GID2);
}

void CGuildManager::RecvWarEnd(uint32_t GID1, uint32_t GID2)
{
	sys_log(0, "GuildWar: RecvWarEnded : %u vs %u", GID1, GID2);
	WarEnd(GID1, GID2, true); // It must be terminated unconditionally.
}

#ifdef ENABLE_NEW_WAR_OPTIONS
void CGuildManager::StartWar(uint8_t bType, uint32_t GID1, uint32_t GID2, uint8_t bRound, uint8_t bPoints, uint8_t bTime, CGuildWarReserve* pkReserve)
#else
void CGuildManager::StartWar(uint8_t bType, uint32_t GID1, uint32_t GID2, CGuildWarReserve* pkReserve)
#endif
{
	sys_log(0, "GuildWar: StartWar(%d,%d,%d)", bType, GID1, GID2);

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	TGuildWarInfo& gw = m_WarMap[GID1][GID2]; // map insert

#ifdef ENABLE_NEW_WAR_OPTIONS
	int wtime;
	if (bTime == 1)
		wtime = 600;	//10min
	else if (bTime == 2)
		wtime = 1800;	//30min
	else if (bTime == 3)
		wtime = 3600;	//1h
	else
		wtime = 123;
		//wtime = GUILD_WAR_DURATION;

	gw.tEndTime = CClientManager::Instance().GetCurrentTime() + wtime;
	gw.pElement = new TGuildWarPQElement(bType, bRound, bPoints, bTime, GID1, GID2);
#else
	if (bType == GUILD_WAR_TYPE_FIELD)
		gw.tEndTime = CClientManager::Instance().GetCurrentTime() + GUILD_WAR_DURATION;
	else
		gw.tEndTime = CClientManager::Instance().GetCurrentTime() + 172800;

	gw.pElement = new TGuildWarPQElement(bType, GID1, GID2);
#endif
	gw.pkReserve = pkReserve;

	m_pqOnWar.push(std::make_pair(gw.tEndTime, gw.pElement));
}

void CGuildManager::UpdateScore(uint32_t dwGainGID, uint32_t dwOppGID, int iScoreDelta, int iBetScoreDelta)
{
	uint32_t GID1 = dwGainGID;
	uint32_t GID2 = dwOppGID;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	const auto it = m_WarMap[GID1].find(GID2);

	if (it != m_WarMap[GID1].end())
	{
		TGuildWarPQElement* p = it->second.pElement;

		if (!p || p->bEnd)
		{
			sys_err("GuildWar: war not exist or already ended.");
			return;
		}

		int iNewScore = 0;
		int iNewBetScore = 0;

		if (p->GID[0] == dwGainGID)
		{
			p->iScore[0] += iScoreDelta;
			p->iBetScore[0] += iBetScoreDelta;

			iNewScore = p->iScore[0];
			iNewBetScore = p->iBetScore[0];
		}
		else
		{
			p->iScore[1] += iScoreDelta;
			p->iBetScore[1] += iBetScoreDelta;

			iNewScore = p->iScore[1];
			iNewBetScore = p->iBetScore[1];
		}

		sys_log(0, "GuildWar: SendGuildWarScore guild %u wartype %u score_delta %d betscore_delta %d result %u, %u",
			dwGainGID, p->bType, iScoreDelta, iBetScoreDelta, iNewScore, iNewBetScore);

		CClientManager::Instance().for_each_peer(FSendGuildWarScore(dwGainGID, dwOppGID, iNewScore, iNewBetScore));
	}
}

#ifdef ENABLE_NEW_WAR_OPTIONS
void CGuildManager::AddDeclare(uint8_t bType, uint32_t guild_from, uint32_t guild_to, uint8_t bRound, uint8_t bPoints, uint8_t bTime)
#else
void CGuildManager::AddDeclare(uint8_t bType, uint32_t guild_from, uint32_t guild_to)
#endif
{
#ifdef ENABLE_NEW_WAR_OPTIONS
	const TGuildDeclareInfo di(bType, bRound, bPoints, bTime, guild_from, guild_to);
#else
	const TGuildDeclareInfo di(bType, guild_from, guild_to);
#endif

	if (m_DeclareMap.find(di) == m_DeclareMap.end())
		m_DeclareMap.insert(di);

	sys_log(0, "GuildWar: AddDeclare(Type:%d,from:%d,to:%d)", bType, guild_from, guild_to);
}

void CGuildManager::RemoveDeclare(uint32_t guild_from, uint32_t guild_to)
{
#ifdef ENABLE_NEW_WAR_OPTIONS
	typeof(m_DeclareMap.begin()) it = m_DeclareMap.find(TGuildDeclareInfo(0, 0, 0, 0, guild_from, guild_to));
#else
	typeof(m_DeclareMap.begin()) it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_from, guild_to));
#endif

	if (it != m_DeclareMap.end())
		m_DeclareMap.erase(it);

#ifdef ENABLE_NEW_WAR_OPTIONS
	it = m_DeclareMap.find(TGuildDeclareInfo(0, 0, 0, 0, guild_to, guild_from));
#else
	it = m_DeclareMap.find(TGuildDeclareInfo(0, guild_to, guild_from));
#endif

	if (it != m_DeclareMap.end())
		m_DeclareMap.erase(it);

	sys_log(0, "GuildWar: RemoveDeclare(from:%d,to:%d)", guild_from, guild_to);
}

bool CGuildManager::TakeBetPrice(uint32_t dwGuildTo, uint32_t dwGuildFrom, long lWarPrice)
{
	const auto it_from = m_map_kGuild.find(dwGuildFrom);
	const auto it_to = m_map_kGuild.find(dwGuildTo);

	if (it_from == m_map_kGuild.end() || it_to == m_map_kGuild.end())
	{
		sys_log(0, "TakeBetPrice: guild not exist %u %u",
			dwGuildFrom, dwGuildTo);
		return false;
	}

	if (it_from->second.gold < lWarPrice || it_to->second.gold < lWarPrice)
	{
		sys_log(0, "TakeBetPrice: not enough gold %u %d to %u %d",
			dwGuildFrom, it_from->second.gold, dwGuildTo, it_to->second.gold);
		return false;
	}

	it_from->second.gold -= lWarPrice;
	it_to->second.gold -= lWarPrice;

	MoneyChange(dwGuildFrom, it_from->second.gold);
	MoneyChange(dwGuildTo, it_to->second.gold);
	return true;
}

bool CGuildManager::WaitStart(TPacketGuildWar* p)
{
	if (!p)
		return false;

	if (p->lWarPrice > 0)
	{
		if (!TakeBetPrice(p->dwGuildFrom, p->dwGuildTo, p->lWarPrice))
			return false;
	}

	const uint32_t dwCurTime = CClientManager::Instance().GetCurrentTime();

#ifdef ENABLE_NEW_WAR_OPTIONS
	TGuildWaitStartInfo info(p->bType, p->bRound, p->bPoints, p->bTime, p->dwGuildFrom, p->dwGuildTo, p->lWarPrice, p->lInitialScore, nullptr);
#else
	TGuildWaitStartInfo info(p->bType, p->dwGuildFrom, p->dwGuildTo, p->lWarPrice, p->lInitialScore, nullptr);
#endif
	m_pqWaitStart.push(std::make_pair(dwCurTime + GetGuildWarWaitStartDuration(), info));

	sys_log(0,
		"GuildWar: WaitStart g1 %d g2 %d price %d start at %u",
		p->dwGuildFrom,
		p->dwGuildTo,
		p->lWarPrice,
		dwCurTime + GetGuildWarWaitStartDuration());

	return true;
}

int CGuildManager::GetLadderPoint(uint32_t GID)
{
	const auto it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return 0;

	return it->second.ladder_point;
}

void CGuildManager::ChangeLadderPoint(uint32_t GID, int change)
{
	const auto it = m_map_kGuild.find(GID);

	if (it == m_map_kGuild.end())
		return;

	TGuild& r = it->second;

	r.ladder_point += change;

	if (r.ladder_point < 0)
		r.ladder_point = 0;

	char buf[1024];
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET ladder_point=%d WHERE id=%u", GetTablePostfix(), r.ladder_point, GID);
	CDBManager::Instance().AsyncQuery(buf);

	sys_log(0, "GuildManager::ChangeLadderPoint %u %d", GID, r.ladder_point);
	sys_log(0, "%s", buf);

	// send packet
	TPacketGuildLadder p{};

	p.dwGuild = GID;
	p.lLadderPoint = r.ladder_point;
	p.lWin = r.win;
	p.lDraw = r.draw;
	p.lLoss = r.loss;
#ifdef ENABLE_GUILD_WAR_SCORE
	for (int i = 0; i < 3; ++i) {
		p.lWinNew[i] = r.winNew[i];
		p.lDrawNew[i] = r.drawNew[i];
		p.lLossNew[i] = r.lossNew[i];
	}
#endif

	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_LADDER, &p, sizeof(TPacketGuildLadder));
}

void CGuildManager::UseSkill(uint32_t GID, uint32_t dwSkillVnum, uint32_t dwCooltime)
{
	// GUILD_SKILL_COOLTIME_BUG_FIX
	sys_log(0, "UseSkill(gid=%d, skill=%d) CoolTime(%d:%d)", GID, dwSkillVnum, dwCooltime, CClientManager::Instance().GetCurrentTime() + dwCooltime);
	m_pqSkill.push(std::make_pair(CClientManager::Instance().GetCurrentTime() + dwCooltime, TGuildSkillUsed(GID, dwSkillVnum)));
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
}

void CGuildManager::MoneyChange(uint32_t dwGuild, uint32_t dwGold)
{
	sys_log(0, "GuildManager::MoneyChange %d %d", dwGuild, dwGold);

	TPacketDGGuildMoneyChange p{};
	p.dwGuild = dwGuild;
	p.iTotalGold = dwGold;
	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_MONEY_CHANGE, &p, sizeof(p));

	char buf[1024];
	snprintf(buf, sizeof(buf), "UPDATE guild%s SET gold=%u WHERE id = %u", GetTablePostfix(), dwGold, dwGuild);
	CDBManager::Instance().AsyncQuery(buf);
}

void CGuildManager::DepositMoney(uint32_t dwGuild, INT iGold)
{
	if (iGold <= 0)
		return;

	const auto it = m_map_kGuild.find(dwGuild);

	if (it == m_map_kGuild.end())
	{
		sys_err("No guild by id %u", dwGuild);
		return;
	}

	it->second.gold += iGold;
	sys_log(0, "GUILD: %u Deposit %u Total %d", dwGuild, iGold, it->second.gold);

	MoneyChange(dwGuild, it->second.gold);
}

#ifdef ENABLE_USE_MONEY_FROM_GUILD
void CGuildManager::WithdrawMoney(uint32_t dwGuild, INT iGold)
{
	if (iGold <= 0)
		return;

	const auto it = m_map_kGuild.find(dwGuild);

	if (it == m_map_kGuild.end())
	{
		sys_err("No guild by id %u", dwGuild);
		return;
	}

	it->second.gold -= iGold;
	sys_log(0, "GUILD: %u Withdraw %u Total %d", dwGuild, iGold, it->second.gold);

	MoneyChange(dwGuild, it->second.gold);
}
#else
void CGuildManager::WithdrawMoney(CPeer* peer, uint32_t dwGuild, INT iGold)
{
	const auto it = m_map_kGuild.find(dwGuild);

	if (it == m_map_kGuild.end())
	{
		sys_err("No guild by id %u", dwGuild);
		return;
	}

	// If you have money, withdraw it and upload it
	if (it->second.gold >= iGold)
	{
		it->second.gold -= iGold;
		sys_log(0, "GUILD: %u Withdraw %d Total %d", dwGuild, iGold, it->second.gold);

		TPacketDGGuildMoneyWithdraw p{};
		p.dwGuild = dwGuild;
		p.iChangeGold = iGold;

		peer->EncodeHeader(HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE, 0, sizeof(TPacketDGGuildMoneyWithdraw));
		peer->Encode(&p, sizeof(TPacketDGGuildMoneyWithdraw));
	}
}
#endif

void CGuildManager::WithdrawMoneyReply(uint32_t dwGuild, uint8_t bGiveSuccess, INT iGold)
{
	const auto it = m_map_kGuild.find(dwGuild);

	if (it == m_map_kGuild.end())
		return;

	sys_log(0, "GuildManager::WithdrawMoneyReply : guild %u success %d gold %d", dwGuild, bGiveSuccess, iGold);

	if (!bGiveSuccess)
		it->second.gold += iGold;
	else
		MoneyChange(dwGuild, it->second.gold);
}

//
// Reserved Guild Battle (Spectators can bet)
//
const int c_aiScoreByLevel[GUILD_MAX_LEVEL + 1] =
{
	500,	// level 0 = 500 probably error
	500,	// 1
	1000,
	2000,
	3000,
	4000,
	6000,
	8000,
	10000,
	12000,
	15000,	// 10
	18000,
	21000,
	24000,
	28000,
	32000,
	36000,
	40000,
	45000,
	50000,
	55000,
};

const int c_aiScoreByRanking[GUILD_RANK_MAX_NUM + 1] =
{
	0,
	55000,	// 1st
	50000,
	45000,
	40000,
	36000,
	32000,
	28000,
	24000,
	21000,
	18000,	// 10th
	15000,
	12000,
	10000,
	8000,
	6000,
	4000,
	3000,
	2000,
	1000,
	500		// 20th
};

void CGuildManager::BootReserveWar()
{
	const char* c_apszQuery[2] =
	{
#ifdef ENABLE_NEW_WAR_OPTIONS
		"SELECT id, guild1, guild2, UNIX_TIMESTAMP(time), type, round, points, xtime, warprice, initscore, bet_from, bet_to, power1, power2, handicap FROM guild_war_reservation WHERE started=1 AND winner=-1",
		"SELECT id, guild1, guild2, UNIX_TIMESTAMP(time), type, round, points, xtime, warprice, initscore, bet_from, bet_to, power1, power2, handicap FROM guild_war_reservation WHERE started=0"
#else
		"SELECT id, guild1, guild2, UNIX_TIMESTAMP(time), type, warprice, initscore, bet_from, bet_to, power1, power2, handicap FROM guild_war_reservation WHERE started=1 AND winner=-1",
		"SELECT id, guild1, guild2, UNIX_TIMESTAMP(time), type, warprice, initscore, bet_from, bet_to, power1, power2, handicap FROM guild_war_reservation WHERE started=0"
#endif
	};

	for (int i = 0; i < 2; ++i)
	{
		auto pmsg(CDBManager::Instance().DirectQuery(c_apszQuery[i]));

		if (pmsg->Get()->uiNumRows == 0)
			continue;

		MYSQL_ROW row;

		while ((row = mysql_fetch_row(pmsg->Get()->pSQLResult)))
		{
			int col = 0;

			TGuildWarReserve t;

			str_to_number(t.dwID, row[col++]);
			str_to_number(t.dwGuildFrom, row[col++]);
			str_to_number(t.dwGuildTo, row[col++]);
			str_to_number(t.dwTime, row[col++]);
			str_to_number(t.bType, row[col++]);
#ifdef ENABLE_NEW_WAR_OPTIONS
			str_to_number(t.bRound, row[col++]);
			str_to_number(t.bPoints, row[col++]);
			str_to_number(t.bTime, row[col++]);
#endif
			str_to_number(t.lWarPrice, row[col++]);
			str_to_number(t.lInitialScore, row[col++]);
			str_to_number(t.dwBetFrom, row[col++]);
			str_to_number(t.dwBetTo, row[col++]);
			str_to_number(t.lPowerFrom, row[col++]);
			str_to_number(t.lPowerTo, row[col++]);
			str_to_number(t.lHandicap, row[col++]);
			t.bStarted = 0;

			CGuildWarReserve* pkReserve = new CGuildWarReserve(t);

			char buf[512];
			snprintf(buf, sizeof(buf), "GuildWar: BootReserveWar : step %d id %u GID1 %u GID2 %u", i, t.dwID, t.dwGuildFrom, t.dwGuildTo);
			// If i == 0, the DB bounced during the guild battle, so it is treated as a draw.
			// Alternatively, the remaining reservation guild battles with less than 5 minutes remaining will be treated as a draw. (They return their bets)
			//if (i == 0 || (int) t.dwTime - CClientManager::Instance().GetCurrentTime() < 60 * 5)
			if (i == 0 || (int)t.dwTime - CClientManager::Instance().GetCurrentTime() < 0)
			{
				if (i == 0)
					sys_log(0, "%s : DB was shutdowned while war is being.", buf);
				else
					sys_log(0, "%s : left time lower than 5 minutes, will be canceled", buf);

				pkReserve->Draw();
				delete pkReserve;
			}
			else
			{
				sys_log(0, "%s : OK", buf);
				m_map_kWarReserve.insert(std::make_pair(t.dwID, pkReserve));
			}
		}
	}
}

int GetAverageGuildMemberLevel(uint32_t dwGID)
{
	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery),
		"SELECT AVG(level) FROM guild_member%s, player%s AS p WHERE guild_id=%u AND guild_member%s.pid=p.id",
		GetTablePostfix(), GetTablePostfix(), dwGID, GetTablePostfix());

	auto msg(CDBManager::Instance().DirectQuery(szQuery));

	MYSQL_ROW row;
	row = mysql_fetch_row(msg->Get()->pSQLResult);

	int nAverageLevel = 0;
	str_to_number(nAverageLevel, row[0]);
	return nAverageLevel;
}

int GetGuildMemberCount(uint32_t dwGID)
{
	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery), "SELECT COUNT(*) FROM guild_member%s WHERE guild_id=%u", GetTablePostfix(), dwGID);

	auto msg(CDBManager::Instance().DirectQuery(szQuery));

	MYSQL_ROW row;
	row = mysql_fetch_row(msg->Get()->pSQLResult);

	uint32_t dwCount = 0;
	str_to_number(dwCount, row[0]);
	return dwCount;
}

bool CGuildManager::ReserveWar(TPacketGuildWar* p)
{
	if (!p)
		return false;

	uint32_t GID1 = p->dwGuildFrom;
	uint32_t GID2 = p->dwGuildTo;

	if (GID1 > GID2)
		std::swap(GID1, GID2);

	if (p->lWarPrice > 0)
		if (!TakeBetPrice(GID1, GID2, p->lWarPrice))
			return false;

	TGuildWarReserve t;
	memset(&t, 0, sizeof(TGuildWarReserve));

	t.dwGuildFrom = GID1;
	t.dwGuildTo = GID2;
	t.dwTime = CClientManager::Instance().GetCurrentTime() + GetGuildWarReserveSeconds();
	t.bType = p->bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
	t.bRound = p->bRound;
	t.bPoints = p->bPoints;
	t.bTime = p->bTime;
#endif
	t.lWarPrice = p->lWarPrice;
	t.lInitialScore = p->lInitialScore;

	int lvp = 0, rkp = 0, alv = 0, mc = 0;

	// calculate power
	TGuild& k1 = TouchGuild(GID1);

	lvp = c_aiScoreByLevel[MIN(GUILD_MAX_LEVEL, k1.level)];
	rkp = c_aiScoreByRanking[GetRanking(GID1)];
	alv = GetAverageGuildMemberLevel(GID1);
	mc = GetGuildMemberCount(GID1);

	polyPower.SetVar("lvp", lvp);
	polyPower.SetVar("rkp", rkp);
	polyPower.SetVar("alv", alv);
	polyPower.SetVar("mc", mc);

	t.lPowerFrom = (long)polyPower.Eval();
	sys_log(0, "GuildWar: %u lvp %d rkp %d alv %d mc %d power %d", GID1, lvp, rkp, alv, mc, t.lPowerFrom);

	// calculate power
	const TGuild& k2 = TouchGuild(GID2);

	lvp = c_aiScoreByLevel[MIN(GUILD_MAX_LEVEL, k2.level)];
	rkp = c_aiScoreByRanking[GetRanking(GID2)];
	alv = GetAverageGuildMemberLevel(GID2);
	mc = GetGuildMemberCount(GID2);

	polyPower.SetVar("lvp", lvp);
	polyPower.SetVar("rkp", rkp);
	polyPower.SetVar("alv", alv);
	polyPower.SetVar("mc", mc);

	t.lPowerTo = (long)polyPower.Eval();
	sys_log(0, "GuildWar: %u lvp %d rkp %d alv %d mc %d power %d", GID2, lvp, rkp, alv, mc, t.lPowerTo);

	// calculate handicap
	if (t.lPowerTo > t.lPowerFrom)
	{
		polyHandicap.SetVar("pA", t.lPowerTo);
		polyHandicap.SetVar("pB", t.lPowerFrom);
	}
	else
	{
		polyHandicap.SetVar("pA", t.lPowerFrom);
		polyHandicap.SetVar("pB", t.lPowerTo);
	}

	t.lHandicap = (long)polyHandicap.Eval();
	sys_log(0, "GuildWar: handicap %d", t.lHandicap);

	// Queries
	char szQuery[512];

#ifdef ENABLE_NEW_WAR_OPTIONS
	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO guild_war_reservation (guild1, guild2, time, type, round, points, xtime, warprice, initscore, power1, power2, handicap) "
		"VALUES(%u, %u, DATE_ADD(NOW(), INTERVAL 180 SECOND), %u, %u, %u, %u, %ld, %ld, %ld, %ld, %ld)",
		GID1, GID2, p->bType, p->bRound, p->bPoints, p->bTime, p->lWarPrice, p->lInitialScore, t.lPowerFrom, t.lPowerTo, t.lHandicap);
#else
	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO guild_war_reservation (guild1, guild2, time, type, warprice, initscore, power1, power2, handicap) "
		"VALUES(%u, %u, DATE_ADD(NOW(), INTERVAL 180 SECOND), %u, %ld, %ld, %ld, %ld, %ld)",
		GID1, GID2, p->bType, p->lWarPrice, p->lInitialScore, t.lPowerFrom, t.lPowerTo, t.lHandicap);
#endif

	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));

	if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiInsertID == 0 || pmsg->Get()->uiAffectedRows == (uint32_t)-1)
	{
		sys_err("GuildWar: Cannot insert row");
		return false;
	}

	t.dwID = static_cast<uint32_t>(pmsg->Get()->uiInsertID);

	m_map_kWarReserve.insert(std::make_pair(t.dwID, new CGuildWarReserve(t)));

	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_WAR_RESERVE_ADD, &t, sizeof(TGuildWarReserve));
	return true;
}

void CGuildManager::ProcessReserveWar()
{
	const uint32_t dwCurTime = CClientManager::Instance().GetCurrentTime();

	auto it = m_map_kWarReserve.begin();

	while (it != m_map_kWarReserve.end())
	{
		const auto it2 = it++;

		CGuildWarReserve* pk = it2->second;
		if (!pk)
			return;

		TGuildWarReserve& r = pk->GetDataRef();

		if (!r.bStarted && r.dwTime - 1800 <= dwCurTime) // notify 30 minutes in advance.
		{
			const int iMin = static_cast<int>(ceil(static_cast<int>(r.dwTime - dwCurTime) / 60.0));

			/*
			// Nyx: The text that show in GameForge is in CGuildManager::WaitStartWar from guild_manager.cpp (in game_src)
			const TGuild& r_1 = m_map_kGuild[r.dwGuildFrom];
			const TGuild& r_2 = m_map_kGuild[r.dwGuildTo];
			*/

			sys_log(0, "GuildWar: started GID1 %u GID2 %u %d time %d min %d", r.dwGuildFrom, r.dwGuildTo, r.bStarted, dwCurTime - r.dwTime, iMin);

			if (iMin <= 0)
			{
				char szQuery[128];
				snprintf(szQuery, sizeof(szQuery), "UPDATE guild_war_reservation SET started=1 WHERE id=%u", r.dwID);
				CDBManager::Instance().AsyncQuery(szQuery);

				CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_WAR_RESERVE_DEL, &r.dwID, sizeof(uint32_t));

				r.bStarted = true;

#ifdef ENABLE_NEW_WAR_OPTIONS
				TGuildWaitStartInfo info(r.bType, r.bRound, r.bPoints, r.bTime, r.dwGuildFrom, r.dwGuildTo, r.lWarPrice, r.lInitialScore, pk);
#else
				TGuildWaitStartInfo info(r.bType, r.dwGuildFrom, r.dwGuildTo, r.lWarPrice, r.lInitialScore, pk);
#endif
				m_pqWaitStart.push(std::make_pair(dwCurTime + GetGuildWarWaitStartDuration(), info));

				TPacketGuildWar pck{};

				pck.bType = r.bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
				pck.bRound = r.bRound;
				pck.bPoints = r.bPoints;
				pck.bTime = r.bTime;
#endif
				pck.bWar = GUILD_WAR_WAIT_START;
				pck.dwGuildFrom = r.dwGuildFrom;
				pck.dwGuildTo = r.dwGuildTo;
				pck.lWarPrice = r.lWarPrice;
				pck.lInitialScore = r.lInitialScore;

				CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_WAR, &pck, sizeof(TPacketGuildWar));
				//m_map_kWarReserve.erase(it2);
			}
			/*
			// Nyx: The text that show in GameForge is in CGuildManager::WaitStartWar from guild_manager.cpp (in game_src)
			else
			{
				if (iMin != pk->GetLastNoticeMin())
				{
					pk->SetLastNoticeMin(iMin);
					CClientManager::Instance().SendNotice("The war between %s and %s will start after %d minutes!", r_1.szName, r_2.szName, iMin);
				}
			}
			*/
		}
	}
}

bool CGuildManager::Bet(uint32_t dwID, const char* c_pszLogin, uint32_t dwGold, uint32_t dwGuild)
{
	const auto it = m_map_kWarReserve.find(dwID);

	char szQuery[1024];

	if (it == m_map_kWarReserve.end())
	{
		sys_log(0, "WAR_RESERVE: Bet: cannot find reserve war by id %u", dwID);
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO item_award (login, vnum, socket0, given_time) VALUES('%s', %d, %u, NOW())",
			c_pszLogin, ITEM_ELK_VNUM, dwGold);
		CDBManager::Instance().AsyncQuery(szQuery);
		return false;
	}

	if (!it->second->Bet(c_pszLogin, dwGold, dwGuild))
	{
		sys_log(0, "WAR_RESERVE: Bet: cannot bet id %u, login %s, gold %u, guild %u", dwID, c_pszLogin, dwGold, dwGuild);
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO item_award (login, vnum, socket0, given_time) VALUES('%s', %d, %u, NOW())",
			c_pszLogin, ITEM_ELK_VNUM, dwGold);
		CDBManager::Instance().AsyncQuery(szQuery);
		return false;
	}

	return true;
}

void CGuildManager::CancelWar(uint32_t GID1, uint32_t GID2)
{
	RemoveDeclare(GID1, GID2);
	RemoveWar(GID1, GID2);
}

bool CGuildManager::ChangeMaster(uint32_t dwGID, uint32_t dwFrom, uint32_t dwTo)
{
	const auto iter = m_map_kGuild.find(dwGID);

	if (iter == m_map_kGuild.end())
		return false;

	char szQuery[1024];

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild%s SET master=%u WHERE id=%u", GetTablePostfix(), dwTo, dwGID);
	CDBManager::Instance().DirectQuery(szQuery);

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_member%s SET grade=1 WHERE pid=%u", GetTablePostfix(), dwTo);
	CDBManager::Instance().DirectQuery(szQuery);

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_member%s SET grade=15 WHERE pid=%u", GetTablePostfix(), dwFrom);
	CDBManager::Instance().DirectQuery(szQuery);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Guild War Reserve Class
//////////////////////////////////////////////////////////////////////////////////////////
CGuildWarReserve::CGuildWarReserve(const TGuildWarReserve& rTable)
{
	thecore_memcpy(&m_data, &rTable, sizeof(TGuildWarReserve));
	m_iLastNoticeMin = -1;

	Initialize();
}

void CGuildWarReserve::Initialize()
{
	char szQuery[256];
	snprintf(szQuery, sizeof(szQuery), "SELECT login, guild, gold FROM guild_war_bet WHERE war_id=%u", m_data.dwID);

	auto msgbet(CDBManager::Instance().DirectQuery(szQuery));
	if (msgbet->Get()->uiNumRows)
	{
		MYSQL_RES* res = msgbet->Get()->pSQLResult;
		MYSQL_ROW row;

		char szLogin[LOGIN_MAX_LEN + 1];
		uint32_t dwGuild = 0;
		uint32_t dwGold = 0;

		while ((row = mysql_fetch_row(res)))
		{
			dwGuild = dwGold = 0;
			strlcpy(szLogin, row[0], sizeof(szLogin));
			str_to_number(dwGuild, row[1]);
			str_to_number(dwGold, row[2]);

			mapBet.insert(std::make_pair(szLogin, std::make_pair(dwGuild, dwGold)));
		}
	}
}

void CGuildWarReserve::OnSetup(CPeer* peer)
{
	if (!peer)
		return;

	if (m_data.bStarted) // Don't send what has already started.
		return;

#ifdef ENABLE_NEW_WAR_OPTIONS
	FSendPeerWar(m_data.bType, m_data.bRound, m_data.bPoints, m_data.bTime, GUILD_WAR_RESERVE, m_data.dwGuildFrom, m_data.dwGuildTo) (peer);
#else
	FSendPeerWar(m_data.bType, GUILD_WAR_RESERVE, m_data.dwGuildFrom, m_data.dwGuildTo) (peer);
#endif

	peer->EncodeHeader(HEADER_DG_GUILD_WAR_RESERVE_ADD, 0, sizeof(TGuildWarReserve));
	peer->Encode(&m_data, sizeof(TGuildWarReserve));

	TPacketGDGuildWarBet pckBet{};
	pckBet.dwWarID = m_data.dwID;

	auto it = mapBet.begin();

	while (it != mapBet.end())
	{
		strlcpy(pckBet.szLogin, it->first.c_str(), sizeof(pckBet.szLogin));
		pckBet.dwGuild = it->second.first;
		pckBet.dwGold = it->second.second;

		peer->EncodeHeader(HEADER_DG_GUILD_WAR_BET, 0, sizeof(TPacketGDGuildWarBet));
		peer->Encode(&pckBet, sizeof(TPacketGDGuildWarBet));

		++it;
	}
}

bool CGuildWarReserve::Bet(const char* pszLogin, uint32_t dwGold, uint32_t dwGuild)
{
	char szQuery[1024];

	if (m_data.dwGuildFrom != dwGuild && m_data.dwGuildTo != dwGuild)
	{
		sys_log(0, "GuildWarReserve::Bet: invalid guild id");
		return false;
	}

	if (m_data.bStarted)
	{
		sys_log(0, "GuildWarReserve::Bet: war is already started");
		return false;
	}

	if (mapBet.find(pszLogin) != mapBet.end())
	{
		sys_log(0, "GuildWarReserve::Bet: failed. already bet");
		return false;
	}

	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO guild_war_bet (war_id, login, gold, guild) VALUES(%u, '%s', %u, %u)",
		m_data.dwID, pszLogin, dwGold, dwGuild);

	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));
	if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (uint32_t)-1)
	{
		sys_log(0, "GuildWarReserve::Bet: failed. cannot insert row to guild_war_bet table");
		return false;
	}

	if (m_data.dwGuildFrom == dwGuild)
		m_data.dwBetFrom += dwGold;
	else
		m_data.dwBetTo += dwGold;

	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_WAR_RESERVE_ADD, &m_data, sizeof(TGuildWarReserve));

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_war_reservation SET bet_from=%u,bet_to=%u WHERE id=%u",
		m_data.dwBetFrom, m_data.dwBetTo, m_data.dwID);

	CDBManager::Instance().AsyncQuery(szQuery);

	sys_log(0, "GuildWarReserve::Bet: success. %s %u war_id %u bet %u : %u", pszLogin, dwGuild, m_data.dwID, m_data.dwBetFrom, m_data.dwBetTo);
	mapBet.insert(std::make_pair(pszLogin, std::make_pair(dwGuild, dwGold)));

	TPacketGDGuildWarBet pckBet{};
	pckBet.dwWarID = m_data.dwID;
	strlcpy(pckBet.szLogin, pszLogin, sizeof(pckBet.szLogin));
	pckBet.dwGuild = dwGuild;
	pckBet.dwGold = dwGold;

	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_WAR_BET, &pckBet, sizeof(TPacketGDGuildWarBet));
	return true;
}

//
// Draw handling: In most cases, it is normal to win, but in certain situations, such as server problems,
// There should be draw handling.
//
void CGuildWarReserve::Draw()
{
	char szQuery[1024];

	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_war_reservation SET started=1,winner=0 WHERE id=%u", m_data.dwID);
	CDBManager::Instance().AsyncQuery(szQuery);

	if (mapBet.empty())
		return;

	sys_log(0, "WAR_REWARD: Draw. war_id %u", m_data.dwID);

	auto it = mapBet.begin();

	while (1)
	{
		int iLen = 0;
		int iRow = 0;

		iLen += snprintf(szQuery, sizeof(szQuery) - iLen, "INSERT INTO item_award (login, vnum, socket0, given_time) VALUES");

		while (it != mapBet.end())
		{
			if (iRow == 0)
			{
				iLen += snprintf(szQuery + iLen, sizeof(szQuery) - iLen, "('%s', %d, %u, NOW())",
					it->first.c_str(), ITEM_ELK_VNUM, it->second.second);
			}
			else
			{
				iLen += snprintf(szQuery + iLen, sizeof(szQuery) - iLen, ",('%s', %d, %u, NOW())",
					it->first.c_str(), ITEM_ELK_VNUM, it->second.second);
			}

			it++;

			if (iLen > 384)
				break;

			++iRow;
		}

		if (iRow > 0)
		{
			sys_log(0, "WAR_REWARD: QUERY: %s", szQuery);
			CDBManager::Instance().AsyncQuery(szQuery);
		}

		if (it == mapBet.end())
			break;
	}
}

void CGuildWarReserve::End(int iScoreFrom, int iScoreTo)
{
	uint32_t dwWinner = 0;

	sys_log(0, "WAR_REWARD: End: From %u %d To %u %d", m_data.dwGuildFrom, iScoreFrom, m_data.dwGuildTo, iScoreTo);

	if (m_data.lPowerFrom > m_data.lPowerTo)
	{
		if (m_data.lHandicap > iScoreFrom - iScoreTo)
		{
			sys_log(0, "WAR_REWARD: End: failed to overcome handicap, From is strong but To won");
			dwWinner = m_data.dwGuildTo;
		}
		else
		{
			sys_log(0, "WAR_REWARD: End: success to overcome handicap, From win!");
			dwWinner = m_data.dwGuildFrom;
		}
	}
	else
	{
		if (m_data.lHandicap > iScoreTo - iScoreFrom)
		{
			sys_log(0, "WAR_REWARD: End: failed to overcome handicap, To is strong but From won");
			dwWinner = m_data.dwGuildFrom;
		}
		else
		{
			sys_log(0, "WAR_REWARD: End: success to overcome handicap, To win!");
			dwWinner = m_data.dwGuildTo;
		}
	}

	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery), "UPDATE guild_war_reservation SET started=1,winner=%u,result1=%d,result2=%d WHERE id=%u",
		dwWinner, iScoreFrom, iScoreTo, m_data.dwID);
	CDBManager::Instance().AsyncQuery(szQuery);

	if (mapBet.empty())
		return;

	const uint32_t dwTotalBet = m_data.dwBetFrom + m_data.dwBetTo;
	uint32_t dwWinnerBet = 0;

	if (dwWinner == m_data.dwGuildFrom)
		dwWinnerBet = m_data.dwBetFrom;
	else if (dwWinner == m_data.dwGuildTo)
		dwWinnerBet = m_data.dwBetTo;
	else
	{
		sys_err("WAR_REWARD: fatal error, winner does not exist!");
		return;
	}

	if (dwWinnerBet == 0)
	{
		sys_err("WAR_REWARD: total bet money on winner is zero");
		return;
	}

	sys_log(0, "WAR_REWARD: End: Total bet: %u, Winner bet: %u", dwTotalBet, dwWinnerBet);

	auto it = mapBet.begin();

	while (1)
	{
		int iLen = 0;
		int iRow = 0;

		iLen += snprintf(szQuery, sizeof(szQuery) - iLen, "INSERT INTO item_award (login, vnum, socket0, given_time) VALUES");

		while (it != mapBet.end())
		{
			if (it->second.first != dwWinner)
			{
				++it;
				continue;
			}

			const double ratio = (double)it->second.second / dwWinnerBet;

			// distribution after 10% tax
			sys_log(0, "WAR_REWARD: %s %u ratio %f", it->first.c_str(), it->second.second, ratio);

			const uint32_t dwGold = (uint32_t)(dwTotalBet * ratio * 0.9);

			if (iRow == 0)
			{
				iLen += snprintf(szQuery + iLen, sizeof(szQuery) - iLen, "('%s', %d, %u, NOW())",
					it->first.c_str(), ITEM_ELK_VNUM, dwGold);
			}
			else
			{
				iLen += snprintf(szQuery + iLen, sizeof(szQuery) - iLen, ",('%s', %d, %u, NOW())",
					it->first.c_str(), ITEM_ELK_VNUM, dwGold);
			}

			++it;

			if (iLen > 384)
				break;

			++iRow;
		}

		if (iRow > 0)
		{
			sys_log(0, "WAR_REWARD: query: %s", szQuery);
			CDBManager::Instance().AsyncQuery(szQuery);
		}

		if (it == mapBet.end())
			break;
	}
}

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
void CGuildManager::Dungeon(uint32_t dwGuildID, uint8_t bChannel, long lMapIndex)
{
	TPacketDGGuildDungeon sPacket{};
	sPacket.dwGuildID = dwGuildID;
	sPacket.bChannel = bChannel;
	sPacket.lMapIndex = lMapIndex;
	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_DUNGEON, &sPacket, sizeof(sPacket));

	char szBuf[512];
	snprintf(szBuf, sizeof(szBuf), "UPDATE guild%s SET dungeon_ch = %d, dungeon_map = %ld WHERE id = %u;", GetTablePostfix(), bChannel, lMapIndex, dwGuildID);
	CDBManager::Instance().AsyncQuery(szBuf);
}

void CGuildManager::DungeonCooldown(uint32_t dwGuildID, uint32_t dwTime)
{
	TPacketDGGuildDungeonCD sPacket{};
	sPacket.dwGuildID = dwGuildID;
	sPacket.dwTime = dwTime;
	CClientManager::Instance().ForwardPacket(HEADER_DG_GUILD_DUNGEON_CD, &sPacket, sizeof(sPacket));

	char szBuf[512];
	snprintf(szBuf, sizeof(szBuf), "UPDATE guild%s SET dungeon_cooldown = %u WHERE id = %u;", GetTablePostfix(), dwTime, dwGuildID);
	CDBManager::Instance().AsyncQuery(szBuf);
}
#endif
