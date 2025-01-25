#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "char.h"
#include "packet.h"
#include "desc_client.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "db.h"
#include "affect.h"
#include "p2p.h"
#include "war_map.h"
#include "questmanager.h"
#include "sectree_manager.h"
#include "locale_service.h"
#include "guild_manager.h"

enum
{
	GUILD_WAR_WAIT_START_DURATION = 10*60
};

//
// Packet
//
#ifdef ENABLE_NEW_WAR_OPTIONS
void CGuild::GuildWarPacket(uint32_t dwOppGID, uint8_t bWarType, uint8_t bWarState, uint8_t bRound, uint8_t bPoints, uint8_t bTime)
#else
void CGuild::GuildWarPacket(uint32_t dwOppGID, uint8_t bWarType, uint8_t bWarState)
#endif
{
	TPacketGCGuild pack{};
	TPacketGCGuildWar pack2{};

	pack.header		= HEADER_GC_GUILD;
	pack.subheader	= GUILD_SUBHEADER_GC_WAR;
	pack.size		= sizeof(pack) + sizeof(pack2);
	pack2.dwGuildSelf	= GetID();
	pack2.dwGuildOpp	= dwOppGID;
	pack2.bWarState	= bWarState;
	pack2.bType		= bWarType;
#ifdef ENABLE_NEW_WAR_OPTIONS
	pack2.bRound	= bRound;
	pack2.bPoints	= bPoints;
	pack2.dwTime	= bTime;
#endif

	for (itertype(m_memberOnline) it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;

		if (bWarState == GUILD_WAR_ON_WAR)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;558]");

		LPDESC d = ch->GetDesc();

		if (d)
		{
			ch->SendGuildName( dwOppGID );

			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&pack2, sizeof(pack2));
		}
	}
}

void CGuild::SendEnemyGuild(LPCHARACTER ch)
{
	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	TPacketGCGuild pack{};
	TPacketGCGuildWar pack2{};
	pack.header = HEADER_GC_GUILD;
	pack.subheader = GUILD_SUBHEADER_GC_WAR;
	pack.size = sizeof(pack) + sizeof(pack2);
	pack2.dwGuildSelf = GetID();

	TPacketGCGuild p{};
	p.header = HEADER_GC_GUILD;
	p.subheader = GUILD_SUBHEADER_GC_WAR_SCORE;
#ifdef ENABLE_GUILD_RANKING
	p.size = sizeof(p) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(long) + sizeof(long);
#else
	p.size = sizeof(p) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(long);
#endif

	for (auto it : m_EnemyGuild)
	{
		ch->SendGuildName( it.first );

		pack2.dwGuildOpp = it.first;
		pack2.bType = it.second.type;
		pack2.bWarState = it.second.state;
#ifdef ENABLE_NEW_WAR_OPTIONS
		pack2.bRound = it.second.round;
		pack2.bPoints = it.second.points;
		pack2.dwTime = it.second.time;
#endif

		d->BufferedPacket(&pack, sizeof(pack));
		d->Packet(&pack2, sizeof(pack2));

		if (it.second.state == GUILD_WAR_ON_WAR)
		{
			long lScore = GetWarScoreAgainstTo(pack2.dwGuildOpp);
#ifdef ENABLE_GUILD_RANKING
			long lWinPoints = GetWarWinPointAgainstTo(pack2.dwGuildOpp);;
#endif

			d->BufferedPacket(&p, sizeof(p));
			d->BufferedPacket(&pack2.dwGuildSelf, sizeof(uint32_t));
			d->BufferedPacket(&pack2.dwGuildOpp, sizeof(uint32_t));
#ifdef ENABLE_GUILD_RANKING
			d->BufferedPacket(&lScore, sizeof(long));
			d->Packet(&lWinPoints, sizeof(long));
#else
			d->Packet(&lScore, sizeof(long));
#endif

			lScore = CGuildManager::Instance().TouchGuild(pack2.dwGuildOpp)->GetWarScoreAgainstTo(pack2.dwGuildSelf);
#ifdef ENABLE_GUILD_RANKING
			lWinPoints = CGuildManager::Instance().TouchGuild(pack2.dwGuildOpp)->GetWarWinPointAgainstTo(pack2.dwGuildSelf);;
#endif

			d->BufferedPacket(&p, sizeof(p));
			d->BufferedPacket(&pack2.dwGuildOpp, sizeof(uint32_t));
			d->BufferedPacket(&pack2.dwGuildSelf, sizeof(uint32_t));
#ifdef ENABLE_GUILD_RANKING
			d->BufferedPacket(&lScore, sizeof(long));
			d->Packet(&lWinPoints, sizeof(long));
#else
			d->Packet(&lScore, sizeof(long));
#endif
		}
	}
}

//
// War Login
//
uint8_t CGuild::GetGuildWarState(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
		return GUILD_WAR_NONE;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);
	return (it != m_EnemyGuild.end()) ? (it->second.state) : GUILD_WAR_NONE;
}

uint8_t CGuild::GetGuildWarType(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);

	if (git == m_EnemyGuild.end())
		return GUILD_WAR_TYPE_FIELD;

	return git->second.type;
}

#ifdef ENABLE_NEW_WAR_OPTIONS
int CGuild::GetGuildWarSettingRound(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);

	if (git == m_EnemyGuild.end())
		return 1;

	return git->second.round;
}

int CGuild::GetGuildWarSettingPoints(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);

	if (git == m_EnemyGuild.end())
		return 1;

	return git->second.points;
}

int CGuild::GetGuildWarSettingTime(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);

	if (git == m_EnemyGuild.end())
		return 1;

	return git->second.time;
}
#endif

uint32_t CGuild::GetGuildWarMapIndex(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);

	if (git == m_EnemyGuild.end())
		return 0;

	return git->second.map_index;
}

bool CGuild::CanStartWar(uint8_t bGuildWarType)
{
	if (bGuildWarType >= GUILD_WAR_TYPE_MAX_NUM)
		return false;

	if (test_server || quest::CQuestManager::Instance().GetEventFlag("guild_war_test") != 0)
		return GetLadderPoint() > 0;

	return GetLadderPoint() > 0 && GetMemberCount() >= GUILD_WAR_MIN_MEMBER_COUNT;
}

bool CGuild::UnderWar(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
		return false;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);
	return (it != m_EnemyGuild.end()) && (it->second.IsWarBegin());
}

uint32_t CGuild::UnderAnyWar(uint8_t bType)
{
	for (auto it : m_EnemyGuild)
	{
		if (bType < GUILD_WAR_TYPE_MAX_NUM)
			if (it.second.type != bType)
				continue;

		if (it.second.IsWarBegin())
			return it.first;
	}

	return 0;
}

void CGuild::SetWarScoreAgainstTo(uint32_t dwOppGID, int iScore)
{
	uint32_t dwSelfGID = GetID();

	sys_log(0, "GuildWarScore Set %u from %u %d", dwSelfGID, dwOppGID, iScore);
	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it != m_EnemyGuild.end())
	{
		it->second.score = iScore;

		if (it->second.type != GUILD_WAR_TYPE_FIELD)
		{
			CGuild * gOpp = CGuildManager::Instance().TouchGuild(dwOppGID);
			CWarMap * pMap = CWarMapManager::Instance().Find(it->second.map_index);

			if (pMap)
				pMap->UpdateScore(dwSelfGID, iScore, dwOppGID, gOpp->GetWarScoreAgainstTo(dwSelfGID));
		}
		else
		{
			TPacketGCGuild p{};

			p.header = HEADER_GC_GUILD;
			p.subheader = GUILD_SUBHEADER_GC_WAR_SCORE;
			p.size = sizeof(p) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(long);

			TEMP_BUFFER buf;
			buf.write(&p, sizeof(p));

			buf.write(&dwSelfGID, sizeof(uint32_t));
			buf.write(&dwOppGID, sizeof(uint32_t));
			buf.write(&iScore, sizeof(long));

			Packet(buf.read_peek(), buf.size());

			CGuild * gOpp = CGuildManager::Instance().TouchGuild(dwOppGID);

			if (gOpp)
				gOpp->Packet(buf.read_peek(), buf.size());
		}
	}
}

int CGuild::GetWarScoreAgainstTo(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it != m_EnemyGuild.end())
	{
		sys_log(0, "GuildWarScore Get %u from %u %d", GetID(), dwOppGID, it->second.score);
		return it->second.score;
	}

	sys_log(0, "GuildWarScore Get %u from %u No data", GetID(), dwOppGID);
	return 0;
}

#ifdef ENABLE_GUILD_RANKING
int CGuild::GetWarWinPointAgainstTo(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it != m_EnemyGuild.end())
	{
		int winPoints = 0;
		if (it->second.score != 0)
		{
			int GuildPointSize = (int)GetGuildWarSettingPoints(dwOppGID) / 4;
			if (!GuildPointSize)
				GuildPointSize = 0;

			if (it->second.score < (int)GuildPointSize)
				winPoints = 0;
			else if (it->second.score >= (int)GuildPointSize)
				winPoints = 1;
			else if (it->second.score >= (int)GuildPointSize * 2)
				winPoints = 2;
			else if (it->second.score >= (int)GuildPointSize * 3)
				winPoints = 3;
			else if (it->second.score >= (int)GuildPointSize * 4)
				winPoints = 4;
			else
				winPoints = 0;
		}

		sys_log(0, "GuildWinPoint Get %u from %u %d", GetID(), dwOppGID, winPoints);
		return winPoints;
	}

	sys_log(0, "GuildWinPoint Get %u from %u No data", GetID(), dwOppGID);
	return 0;
}
#endif

uint32_t CGuild::GetWarStartTime(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
		return 0;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it == m_EnemyGuild.end())
		return 0;

	return it->second.war_start_time;
}

const TGuildWarInfo& GuildWar_GetTypeInfo(unsigned type)
{
	return KOR_aGuildWarInfo[type];
}

unsigned GuildWar_GetTypeMapIndex(unsigned type)
{
	return GuildWar_GetTypeInfo(type).lMapIndex;
}

bool GuildWar_IsWarMap(unsigned type)
{
	if (type == GUILD_WAR_TYPE_FIELD)
		return true;

	unsigned mapIndex = GuildWar_GetTypeMapIndex(type);

	if (SECTREE_MANAGER::Instance().GetMapRegion(mapIndex))
		return true;

	return false;
}

void CGuild::NotifyGuildMaster(const char* msg)
{
	LPCHARACTER ch = GetMasterCharacter();
	if (ch)
		ch->ChatPacket(CHAT_TYPE_INFO, msg);
}

//
// War State Relative
//
//
// A Declare -> B Refuse
//           -> B Declare -> StartWar -> EndWar
//
// A Declare -> B Refuse
//           -> B Declare -> WaitStart -> Fail
//                                     -> StartWar -> EndWar
//
#ifdef ENABLE_NEW_WAR_OPTIONS
void CGuild::RequestDeclareWar(uint32_t dwOppGID, uint8_t type, uint8_t flag, uint8_t round, uint8_t points, uint32_t time)
#else
void CGuild::RequestDeclareWar(uint32_t dwOppGID, uint8_t type)
#endif
{
	if (dwOppGID == GetID())
	{
		sys_log(0, "GuildWar.DeclareWar.DECLARE_WAR_SELF id(%d -> %d), type(%d)", GetID(), dwOppGID, type);
		return;
	}

	if (type >= GUILD_WAR_TYPE_MAX_NUM)
	{
		sys_log(0, "GuildWar.DeclareWar.UNKNOWN_WAR_TYPE id(%d -> %d), type(%d)", GetID(), dwOppGID, type);
		return;
	}

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);
	if (it == m_EnemyGuild.end())
	{
		if (!GuildWar_IsWarMap(type))
		{
			sys_err("GuildWar.DeclareWar.NOT_EXIST_MAP id(%d -> %d), type(%d), map(%d)",
					GetID(), dwOppGID, type, GuildWar_GetTypeMapIndex(type));

			map_allow_log();
			NotifyGuildMaster("[LS;669]");
			return;
		}

		// 패킷 보내기 to another server
		TPacketGuildWar p;
		p.bType = type;
		p.bWar = GUILD_WAR_SEND_DECLARE;
#ifdef ENABLE_NEW_WAR_OPTIONS
		p.bRound = round;
		p.bPoints = points;
		p.bTime = time;
#endif
		p.dwGuildFrom = GetID();
		p.dwGuildTo = dwOppGID;
		db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
		sys_log(0, "GuildWar.DeclareWar id(%d -> %d), type(%d)", GetID(), dwOppGID, type);
		return;
	}

	switch (it->second.state)
	{
		case GUILD_WAR_RECV_DECLARE:
			{
				const uint8_t saved_type = it->second.type;
#ifdef ENABLE_NEW_WAR_OPTIONS
				const uint8_t saved_round = it->second.round;
				const uint8_t saved_points = it->second.points;
				const uint8_t saved_time = it->second.time;
#endif

				if (saved_type == GUILD_WAR_TYPE_FIELD)
				{
					TPacketGuildWar p;
					p.bType = saved_type;
					p.bWar = GUILD_WAR_ON_WAR;
#ifdef ENABLE_NEW_WAR_OPTIONS
					p.bRound = saved_round;
					p.bPoints = saved_points;
					p.bTime = saved_time;
#endif
					p.dwGuildFrom = GetID();
					p.dwGuildTo = dwOppGID;
					db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
					sys_log(0, "GuildWar.AcceptWar id(%d -> %d), type(%d)", GetID(), dwOppGID, saved_type);
					return;
				}

				if (!GuildWar_IsWarMap(saved_type))
				{
					sys_err("GuildWar.AcceptWar.NOT_EXIST_MAP id(%d -> %d), type(%d), map(%d)",
							GetID(), dwOppGID, type, GuildWar_GetTypeMapIndex(type));

					map_allow_log();
					NotifyGuildMaster("[LS;669]");
					return;
				}

				const TGuildWarInfo& guildWarInfo = GuildWar_GetTypeInfo(saved_type);

				TPacketGuildWar p;
				p.bType = saved_type;
				p.bWar = GUILD_WAR_WAIT_START;
#ifdef ENABLE_NEW_WAR_OPTIONS
				p.bRound = saved_round;
				p.bPoints = saved_points;
				p.bTime = saved_time;
#endif
				p.dwGuildFrom = GetID();
				p.dwGuildTo = dwOppGID;
				p.lWarPrice = guildWarInfo.iWarPrice;
				p.lInitialScore = guildWarInfo.iInitialScore;

				if (test_server)
					p.lInitialScore /= 10;

				db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));

				sys_log(0, "GuildWar.WaitStartSendToDB id(%d vs %d), type(%d), bet(%d), map_index(%d)",
						GetID(), dwOppGID, saved_type, guildWarInfo.iWarPrice, guildWarInfo.lMapIndex);

			}
			break;
		case GUILD_WAR_SEND_DECLARE:
			{
				NotifyGuildMaster("[LS;780]");
			}
			break;
		default:
			sys_err("GuildWar.DeclareWar.UNKNOWN_STATE[%d]: id(%d vs %d), type(%d), guild(%s:%u)",
					it->second.state, GetID(), dwOppGID, type, GetName(), GetID());
			break;
	}
}

#ifdef ENABLE_NEW_WAR_OPTIONS
bool CGuild::DeclareWar(uint32_t dwOppGID, uint8_t type, uint8_t state, uint8_t round, uint8_t points, uint8_t time)
#else
bool CGuild::DeclareWar(uint32_t dwOppGID, uint8_t type, uint8_t state)
#endif
{
	if (m_EnemyGuild.find(dwOppGID) != m_EnemyGuild.end())
		return false;

	TGuildWar gw(type);
	gw.state = state;
#ifdef ENABLE_NEW_WAR_OPTIONS
	gw.round = round;
	gw.points = points;
	gw.time = time;
#endif

	m_EnemyGuild.insert(std::make_pair(dwOppGID, gw));

#ifdef ENABLE_NEW_WAR_OPTIONS
	GuildWarPacket(dwOppGID, type, state, round, points, time);
#else
	GuildWarPacket(dwOppGID, type, state);
#endif
	return true;
}

bool CGuild::CheckStartWar(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it == m_EnemyGuild.end())
		return false;

	TGuildWar & gw(it->second);

	if (gw.state == GUILD_WAR_ON_WAR)
		return false;

	return true;
}

void CGuild::StartWar(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it == m_EnemyGuild.end())
		return;

	TGuildWar & gw(it->second);

	if (gw.state == GUILD_WAR_ON_WAR)
		return;

	gw.state = GUILD_WAR_ON_WAR;
	gw.war_start_time = get_global_time();

#ifdef ENABLE_NEW_WAR_OPTIONS
	GuildWarPacket(dwOppGID, gw.type, GUILD_WAR_ON_WAR, gw.round, gw.points, gw.time);
#else
	GuildWarPacket(dwOppGID, gw.type, GUILD_WAR_ON_WAR);
#endif

	if (gw.type != GUILD_WAR_TYPE_FIELD)
		GuildWarEntryAsk(dwOppGID);
}

bool CGuild::WaitStartWar(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
	{
		sys_log(0 ,"GuildWar.WaitStartWar.DECLARE_WAR_SELF id(%u -> %u)", GetID(), dwOppGID);
		return false;
	}

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);
	if (it == m_EnemyGuild.end())
	{
		sys_log(0 ,"GuildWar.WaitStartWar.UNKNOWN_ENEMY id(%u -> %u)", GetID(), dwOppGID);
		return false;
	}

	TGuildWar & gw(it->second);

	if (gw.state == GUILD_WAR_WAIT_START)
	{
		sys_log(0 ,"GuildWar.WaitStartWar.UNKNOWN_STATE id(%u -> %u), state(%d)", GetID(), dwOppGID, gw.state);
		return false;
	}

	gw.state = GUILD_WAR_WAIT_START;

	CGuild* g = CGuildManager::Instance().FindGuild(dwOppGID);
	if (!g)
	{
		sys_log(0 ,"GuildWar.WaitStartWar.NOT_EXIST_GUILD id(%u -> %u)", GetID(), dwOppGID);
		return false;
	}

	// GUILDWAR_INFO
	const TGuildWarInfo& rkGuildWarInfo = GuildWar_GetTypeInfo(gw.type);
	// END_OF_GUILDWAR_INFO

	if (gw.type == GUILD_WAR_TYPE_FIELD)
	{
		sys_log(0 ,"GuildWar.WaitStartWar.FIELD_TYPE id(%u -> %u)", GetID(), dwOppGID);
		return true;
	}

	sys_log(0 ,"GuildWar.WaitStartWar.CheckWarServer id(%u -> %u), type(%d), map(%d)",
			GetID(), dwOppGID, gw.type, rkGuildWarInfo.lMapIndex);

	if (!map_allow_find(rkGuildWarInfo.lMapIndex))
	{
		sys_log(0 ,"GuildWar.WaitStartWar.SKIP_WAR_MAP id(%u -> %u)", GetID(), dwOppGID);
		return true;
	}


	uint32_t id1 = GetID();
	uint32_t id2 = dwOppGID;

	if (id1 > id2)
		std::swap(id1, id2);

	uint32_t lMapIndex = CWarMapManager::Instance().CreateWarMap(rkGuildWarInfo, id1, id2);
	if (!lMapIndex)
	{
		sys_err("GuildWar.WaitStartWar.CREATE_WARMAP_ERROR id(%u vs %u), type(%u), map(%d)", id1, id2, gw.type, rkGuildWarInfo.lMapIndex);
		CGuildManager::Instance().RequestEndWar(GetID(), dwOppGID);
		return false;
	}

	sys_log(0, "GuildWar.WaitStartWar.CreateMap id(%u vs %u), type(%u), map(%d) -> map_inst(%u)", id1, id2, gw.type, rkGuildWarInfo.lMapIndex, lMapIndex);

	gw.map_index = lMapIndex;

	SetGuildWarMapIndex(dwOppGID, lMapIndex);
	g->SetGuildWarMapIndex(GetID(), lMapIndex);

	///////////////////////////////////////////////////////
	TPacketGGGuildWarMapIndex p{};

	p.bHeader	= HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX;
	p.dwGuildID1	= id1;
	p.dwGuildID2 	= id2;
	p.lMapIndex	= lMapIndex;

	P2P_MANAGER::Instance().Send(&p, sizeof(p));
	///////////////////////////////////////////////////////

	return true;
}

void CGuild::RequestRefuseWar(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
		return;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it != m_EnemyGuild.end() && it->second.state == GUILD_WAR_RECV_DECLARE)
	{
		TPacketGuildWar p;
		p.bWar = GUILD_WAR_REFUSE;
		p.dwGuildFrom = GetID();
		p.dwGuildTo = dwOppGID;

		db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
	}
}

void CGuild::RefuseWar(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
		return;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it != m_EnemyGuild.end() && (it->second.state == GUILD_WAR_SEND_DECLARE || it->second.state == GUILD_WAR_RECV_DECLARE))
	{
		uint8_t type = it->second.type;
#ifdef ENABLE_NEW_WAR_OPTIONS
		uint8_t round = it->second.round;
		uint8_t point = it->second.points;
		uint8_t time = it->second.time;
#endif
		m_EnemyGuild.erase(dwOppGID);

#ifdef ENABLE_NEW_WAR_OPTIONS
		GuildWarPacket(dwOppGID, type, GUILD_WAR_END, round, point, time);
#else
		GuildWarPacket(dwOppGID, type, GUILD_WAR_END);
#endif
	}
}

void CGuild::ReserveWar(uint32_t dwOppGID, uint8_t type)
{
	if (dwOppGID == GetID())
		return;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it == m_EnemyGuild.end())
	{
		TGuildWar gw(type);
		gw.state = GUILD_WAR_RESERVE;
		m_EnemyGuild.insert(std::make_pair(dwOppGID, gw));
	}
	else
		it->second.state = GUILD_WAR_RESERVE;

	sys_log(0, "Guild::ReserveWar %u", dwOppGID);
}

void CGuild::EndWar(uint32_t dwOppGID)
{
	if (dwOppGID == GetID())
		return;

	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it != m_EnemyGuild.end())
	{
		CWarMap * pMap = CWarMapManager::Instance().Find(it->second.map_index);

		if (pMap)
			pMap->SetEnded();

#ifdef ENABLE_NEW_WAR_OPTIONS
		GuildWarPacket(dwOppGID, it->second.type, GUILD_WAR_END, it->second.round, it->second.points, it->second.time);
#else
		GuildWarPacket(dwOppGID, it->second.type, GUILD_WAR_END);
#endif
		m_EnemyGuild.erase(it);

		if (!UnderAnyWar())
		{
/*#ifdef ENABLE_BATTLE_PASS_SYSTEM
			for (itertype(m_memberOnline) it_char = m_memberOnline.begin(); it_char != m_memberOnline.end(); ++it_char)
			{
				LPCHARACTER ch = *it_char;
				TGuildWar& gw(it->second);
				ch->UpdateExtBattlePassMissionProgress(GUILD_PLAY_GUILDWAR, 1, gw.type + 1);
#else*/
			for (itertype(m_memberOnline) it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
			{
				LPCHARACTER ch = *it;
//#endif
				ch->RemoveAffect(GUILD_SKILL_BLOOD);
				ch->RemoveAffect(GUILD_SKILL_BLESS);
				ch->RemoveAffect(GUILD_SKILL_SEONGHWI);
				ch->RemoveAffect(GUILD_SKILL_ACCEL);
				ch->RemoveAffect(GUILD_SKILL_BUNNO);
				ch->RemoveAffect(GUILD_SKILL_JUMUN);

				ch->RemoveBadAffect();
			}
		}
	}
#ifdef ENABLE_NEW_WAR_OPTIONS
	for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, this, std::placeholders::_1));
#endif
}

void CGuild::SetGuildWarMapIndex(uint32_t dwOppGID, long lMapIndex)
{
	itertype(m_EnemyGuild) it = m_EnemyGuild.find(dwOppGID);

	if (it == m_EnemyGuild.end())
		return;

	it->second.map_index = lMapIndex;
	sys_log(0, "GuildWar.SetGuildWarMapIndex id(%d -> %d), map(%d)", GetID(), dwOppGID, lMapIndex);
}

void CGuild::GuildWarEntryAccept(uint32_t dwOppGID, LPCHARACTER ch)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);

	if (git == m_EnemyGuild.end())
		return;

	TGuildWar & gw(git->second);

	if (gw.type == GUILD_WAR_TYPE_FIELD)
		return;

	if (gw.state != GUILD_WAR_ON_WAR)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;891]");
		return;
	}

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	if (!HasGradeAuth(GetMember(ch->GetPlayerID())->grade, GUILD_AUTH_WAR)) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("guild_war_permit"));
		return;
	}
#endif

	if (!gw.map_index)
		return;

	PIXEL_POSITION pos;

	if (!CWarMapManager::Instance().GetStartPosition(gw.map_index, GetID() < dwOppGID ? 0 : 1, pos))
		return;
#ifdef ENABLE_NEWSTUFF
	if (g_NoMountAtGuildWar)
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);
		if (ch->IsRiding())
			ch->StopRiding();
	}
#endif
	quest::PC * pPC = quest::CQuestManager::Instance().GetPC(ch->GetPlayerID());
	pPC->SetFlag("war.is_war_member", 1);

	ch->SaveExitLocation();
	ch->WarpSet(pos.x, pos.y, gw.map_index);
}

void CGuild::GuildWarEntryAsk(uint32_t dwOppGID)
{
	itertype(m_EnemyGuild) git = m_EnemyGuild.find(dwOppGID);
	if (git == m_EnemyGuild.end())
	{
		sys_err("GuildWar.GuildWarEntryAsk.UNKNOWN_ENEMY(%d)", dwOppGID);
		return;
	}

	TGuildWar & gw(git->second);

	sys_log(0, "GuildWar.GuildWarEntryAsk id(%d vs %d), map(%d)", GetID(), dwOppGID, gw.map_index);
	if (!gw.map_index)
	{
		sys_err("GuildWar.GuildWarEntryAsk.NOT_EXIST_MAP id(%d vs %d)", GetID(), dwOppGID);
		return;
	}

	PIXEL_POSITION pos;
	if (!CWarMapManager::Instance().GetStartPosition(gw.map_index, GetID() < dwOppGID ? 0 : 1, pos))
	{
		sys_err("GuildWar.GuildWarEntryAsk.START_POSITION_ERROR id(%d vs %d), pos(%d, %d)", GetID(), dwOppGID, pos.x, pos.y);
		return;
	}

	sys_log(0, "GuildWar.GuildWarEntryAsk.OnlineMemberCount(%d)", m_memberOnline.size());

	itertype(m_memberOnline) it = m_memberOnline.begin();

	while (it != m_memberOnline.end())
	{
		LPCHARACTER ch = *it++;

#ifdef ENABLE_GUILDWAR_BUTTON
		ch->SendWarTeleportButton(true);
#else
		using namespace quest;
		unsigned int questIndex=CQuestManager::Instance().GetQuestIndexByName("guild_war_join");
		if (questIndex)
		{
			sys_log(0, "GuildWar.GuildWarEntryAsk.SendLetterToMember pid(%d), qid(%d)", ch->GetPlayerID(), questIndex);
			CQuestManager::Instance().Letter(ch->GetPlayerID(), questIndex, 0);
		}
		else
		{
			sys_err("GuildWar.GuildWarEntryAsk.SendLetterToMember.QUEST_ERROR pid(%d), quest_name('guild_war_join.quest')",
					ch->GetPlayerID(), questIndex);
			break;
		}
#endif
	}
}

//
// LADDER POINT
//
void CGuild::SetLadderPoint(int point)
{
	if (m_data.ladder_point != point)
	{
		for (itertype(m_memberOnline) it = m_memberOnline.begin(); it!=m_memberOnline.end();++it)
		{
			LPCHARACTER ch = (*it);
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1001]");
		}
	}
	m_data.ladder_point = point;
}

void CGuild::ChangeLadderPoint(int iChange)
{
	TPacketGuildLadderPoint p;
	p.dwGuild = GetID();
	p.lChange = iChange;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_LADDER_POINT, 0, &p, sizeof(p));
}
