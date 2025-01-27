#include "stdafx.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "char.h"
#include "db.h"
#include "lzo_manager.h"
#include "desc_client.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "packet.h"
#include "war_map.h"
#include "questmanager.h"
#include "locale_service.h"
#include "guild_manager.h"
#include "MarkManager.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#	include "LocaleNotice.hpp"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

namespace
{
	struct FGuildNameSender
	{
		FGuildNameSender(uint32_t id, const char* guild_name) : id(id), name(guild_name)
		{
			p.header = HEADER_GC_GUILD;
			p.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
			p.size = sizeof(p) + GUILD_NAME_MAX_LEN + sizeof(uint32_t);
		}

		void operator()(LPCHARACTER ch)
		{
			if (!ch)
				return;

			LPDESC d = ch->GetDesc();

			if (d)
			{
				d->BufferedPacket(&p, sizeof(p));
				d->BufferedPacket(&id, sizeof(id));
				d->Packet(name, GUILD_NAME_MAX_LEN);
			}
		}

		uint32_t id;
		const char* name;
		TPacketGCGuild p;
	};

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	struct FGuildWinner
	{
		FGuildWinner() {}

		void operator()(LPCHARACTER ch)
		{
			if (!ch)
				return;

			LPDESC d = ch->GetDesc();

			CAchievementSystem::Instance().OnWinGuildWar(ch);
		}
	};
#endif
}

CGuildManager::CGuildManager()
{
}

CGuildManager::~CGuildManager()
{
	for (TGuildMap::const_iterator iter = m_mapGuild.begin(); iter != m_mapGuild.end(); ++iter)
	{
		M2_DELETE(iter->second);
	}

	m_mapGuild.clear();
}

int CGuildManager::GetDisbandDelay()
{
	return quest::CQuestManager::Instance().GetEventFlag("guild_disband_delay") * (test_server ? 60 : 86400);
}

int CGuildManager::GetWithdrawDelay()
{
	return quest::CQuestManager::Instance().GetEventFlag("guild_withdraw_delay") * (test_server ? 60 : 86400);
}

uint32_t CGuildManager::CreateGuild(TGuildCreateParameter& gcp)
{
	if (!gcp.master)
		return 0;

	if (!check_name(gcp.name))
	{
		gcp.master->ChatPacket(CHAT_TYPE_INFO, "[LS;472]");
		return 0;
	}

	// @fixme143 BEGIN
	static char __guild_name[GUILD_NAME_MAX_LEN * 2 + 1];
	DBManager::Instance().EscapeString(__guild_name, sizeof(__guild_name), gcp.name, strnlen(gcp.name, sizeof(gcp.name)));
	if (strncmp(__guild_name, gcp.name, strnlen(gcp.name, sizeof(gcp.name))))
		return 0;
	// @fixme143 END

	auto pmsg(DBManager::Instance().DirectQuery("SELECT COUNT(*) FROM guild%s WHERE name = '%s'",
		get_table_postfix(), __guild_name));

	if (pmsg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

		if (!(row[0] && row[0][0] == '0'))
		{
			gcp.master->ChatPacket(CHAT_TYPE_INFO, "[LS;473]");
			return 0;
		}
	}
	else
	{
		gcp.master->ChatPacket(CHAT_TYPE_INFO, "[LS;474]");
		return 0;
	}

	// new CGuild(gcp) queries guild tables and tell dbcache to notice other game servers.
	// other game server calls CGuildManager::LoadGuild to load guild.
	CGuild* pg = M2_NEW CGuild(gcp);
	m_mapGuild.insert(std::make_pair(pg->GetID(), pg));
	return pg->GetID();
}

void CGuildManager::Unlink(uint32_t pid)
{
	m_map_pkGuildByPID.erase(pid);
}

CGuild* CGuildManager::GetLinkedGuild(uint32_t pid)
{
	TGuildMap::iterator it = m_map_pkGuildByPID.find(pid);

	if (it == m_map_pkGuildByPID.end())
		return nullptr;

	return it->second;
}

void CGuildManager::Link(uint32_t pid, CGuild* guild)
{
	if (m_map_pkGuildByPID.find(pid) == m_map_pkGuildByPID.end())
		m_map_pkGuildByPID.insert(std::make_pair(pid, guild));
}

void CGuildManager::P2PLogoutMember(uint32_t pid)
{
	TGuildMap::iterator it = m_map_pkGuildByPID.find(pid);

	if (it != m_map_pkGuildByPID.end())
	{
		it->second->P2PLogoutMember(pid);
	}
}

void CGuildManager::P2PLoginMember(uint32_t pid)
{
	TGuildMap::iterator it = m_map_pkGuildByPID.find(pid);

	if (it != m_map_pkGuildByPID.end())
	{
		it->second->P2PLoginMember(pid);
	}
}

void CGuildManager::LoginMember(LPCHARACTER ch)
{
	TGuildMap::iterator it = m_map_pkGuildByPID.find(ch->GetPlayerID());

	if (it != m_map_pkGuildByPID.end())
	{
		it->second->LoginMember(ch);
	}
}


CGuild* CGuildManager::TouchGuild(uint32_t guild_id)
{
	TGuildMap::iterator it = m_mapGuild.find(guild_id);

	if (it == m_mapGuild.end())
	{
		m_mapGuild.insert(std::make_pair(guild_id, M2_NEW CGuild(guild_id)));
		it = m_mapGuild.find(guild_id);

		CHARACTER_MANAGER::Instance().for_each_pc(FGuildNameSender(guild_id, it->second->GetName()));
	}

	return it->second;
}

CGuild* CGuildManager::FindGuild(uint32_t guild_id)
{
	TGuildMap::iterator it = m_mapGuild.find(guild_id);
	if (it == m_mapGuild.end())
	{
		return nullptr;
	}
	return it->second;
}

CGuild* CGuildManager::FindGuildByName(const std::string guild_name)
{
	for (auto it = m_mapGuild.begin(); it != m_mapGuild.end(); ++it)
	{
		if (it->second->GetName() == guild_name)
			return it->second;
	}
	return nullptr;
}

void CGuildManager::Initialize()
{
	sys_log(0, "Initializing Guild");

	if (g_bAuthServer)
	{
		sys_log(0, " No need for auth server");
		return;
	}

	auto pmsg(DBManager::Instance().DirectQuery("SELECT id FROM guild%s", get_table_postfix()));

	std::vector<uint32_t> vecGuildID;
	vecGuildID.reserve(static_cast<uint32_t>(pmsg->Get()->uiNumRows));

	for (uint i = 0; i < pmsg->Get()->uiNumRows; i++)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		uint32_t guild_id = strtoul(row[0], (char**)nullptr, 10);
		LoadGuild(guild_id);

		vecGuildID.emplace_back(guild_id);
	}

	CGuildMarkManager& rkMarkMgr = CGuildMarkManager::Instance();

	rkMarkMgr.SetMarkPathPrefix("mark");

	extern bool GuildMarkConvert(const std::vector<uint32_t> &vecGuildID);
	GuildMarkConvert(vecGuildID);

	rkMarkMgr.LoadMarkIndex();
	rkMarkMgr.LoadMarkImages();
	rkMarkMgr.LoadSymbol(GUILD_SYMBOL_FILENAME);
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CGuildManager::InitializeDonate()
{
	auto pMsg(DBManager::Instance().DirectQuery("UPDATE guild%s SET guildstoragestate = 0", get_table_postfix()));
}
#endif

void CGuildManager::LoadGuild(uint32_t guild_id)
{
	const TGuildMap::iterator it = m_mapGuild.find(guild_id);

	if (it == m_mapGuild.end())
	{
		m_mapGuild.insert(std::make_pair(guild_id, M2_NEW CGuild(guild_id)));
	}
	else
	{
		//it->second->Load(guild_id);
	}
}

void CGuildManager::DisbandGuild(uint32_t guild_id)
{
	const TGuildMap::iterator it = m_mapGuild.find(guild_id);

	if (it == m_mapGuild.end())
		return;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	CMeleyLairManager::Instance().GuildRemoved(it->second);
#endif

	it->second->Disband();

	M2_DELETE(it->second);
	m_mapGuild.erase(it);

	CGuildMarkManager::Instance().DeleteMark(guild_id);
}

void CGuildManager::SkillRecharge()
{
	for (auto& it : m_mapGuild)
	{
		it.second->SkillRecharge();
	}
}

int CGuildManager::GetRank(CGuild* g)
{
	if (!g)
		return 1;

	const int point = g->GetLadderPoint();
	int rank = 1;

	for (auto& it : m_mapGuild)
	{
		CGuild* pg = it.second;

		if (pg && (pg->GetLadderPoint() > point))
			rank++;
	}

	return rank;
}

struct FGuildCompare : public std::function<bool(CGuild*, CGuild*)>
{
	bool operator () (CGuild* g1, CGuild* g2) const
	{
		if (!g1 || !g2)
			return false;

		if (g1->GetLadderPoint() < g2->GetLadderPoint())
			return true;
		if (g1->GetLadderPoint() > g2->GetLadderPoint())
			return false;
		if (g1->GetGuildWarWinCount() < g2->GetGuildWarWinCount())
			return true;
		if (g1->GetGuildWarWinCount() > g2->GetGuildWarWinCount())
			return false;
		if (g1->GetGuildWarLossCount() < g2->GetGuildWarLossCount())
			return true;
		if (g1->GetGuildWarLossCount() > g2->GetGuildWarLossCount())
			return false;

		const int c = strcmp(g1->GetName(), g2->GetName());
		if (c > 0)
			return true;

		return false;
	}
};

void CGuildManager::GetHighRankString(uint32_t dwMyGuild, char* buffer, size_t buflen)
{
	using namespace std;
	vector<CGuild*> v;

	for (auto& it : m_mapGuild)
	{
		if (it.second)
			v.emplace_back(it.second);
	}

	std::sort(v.begin(), v.end(), FGuildCompare());
	const int n = v.size();
	int len = 0, len2;
	*buffer = '\0';

	for (int i = 0; i < 8; ++i)
	{
		if (n - i - 1 < 0)
			break;

		CGuild* g = v[n - i - 1];

		if (!g)
			continue;

		if (g->GetLadderPoint() <= 0)
			break;

		if (dwMyGuild == g->GetID())
		{
			len2 = snprintf(buffer + len, buflen - len, "[COLOR r;255|g;255|b;0]");

			if (len2 < 0 || len2 >= (int)buflen - len)
				len += (buflen - len) - 1;
			else
				len += len2;
		}

		if (i)
		{
			len2 = snprintf(buffer + len, buflen - len, "[ENTER]");

			if (len2 < 0 || len2 >= (int)buflen - len)
				len += (buflen - len) - 1;
			else
				len += len2;
		}

		len2 = snprintf(buffer + len, buflen - len, "%3d | %-12s | %-8d | %4d | %4d | %4d",
			GetRank(g),
			g->GetName(),
			g->GetLadderPoint(),
			g->GetGuildWarWinCount(),
			g->GetGuildWarDrawCount(),
			g->GetGuildWarLossCount());

		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;

		if (g->GetID() == dwMyGuild)
		{
			len2 = snprintf(buffer + len, buflen - len, "[/COLOR]");

			if (len2 < 0 || len2 >= (int)buflen - len)
				len += (buflen - len) - 1;
			else
				len += len2;
		}
	}
}

void CGuildManager::GetAroundRankString(uint32_t dwMyGuild, char* buffer, size_t buflen)
{
	using namespace std;
	vector<CGuild*> v;

	for (auto& it : m_mapGuild)
	{
		if (it.second)
			v.emplace_back(it.second);
	}

	std::sort(v.begin(), v.end(), FGuildCompare());
	const int n = v.size();
	int idx;

	for (idx = 0; idx < (int)v.size(); ++idx)
	{
		if (v[idx]->GetID() == dwMyGuild)
			break;
	}

	int len = 0, len2;
	int count = 0;
	*buffer = '\0';

	for (int i = -3; i <= 3; ++i)
	{
		if (idx - i < 0)
			continue;

		if (idx - i >= n)
			continue;

		CGuild* g = v[idx - i];

		if (!g)
			continue;

		if (dwMyGuild == g->GetID())
		{
			len2 = snprintf(buffer + len, buflen - len, "[COLOR r;255|g;255|b;0]");

			if (len2 < 0 || len2 >= (int)buflen - len)
				len += (buflen - len) - 1;
			else
				len += len2;
		}

		if (count)
		{
			len2 = snprintf(buffer + len, buflen - len, "[ENTER]");

			if (len2 < 0 || len2 >= (int)buflen - len)
				len += (buflen - len) - 1;
			else
				len += len2;
		}

		len2 = snprintf(buffer + len, buflen - len, "%3d | %-12s | %-8d | %4d | %4d | %4d",
			GetRank(g),
			g->GetName(),
			g->GetLadderPoint(),
			g->GetGuildWarWinCount(),
			g->GetGuildWarDrawCount(),
			g->GetGuildWarLossCount());

		if (len2 < 0 || len2 >= (int)buflen - len)
			len += (buflen - len) - 1;
		else
			len += len2;

		++count;

		if (g->GetID() == dwMyGuild)
		{
			len2 = snprintf(buffer + len, buflen - len, "[/COLOR]");

			if (len2 < 0 || len2 >= (int)buflen - len)
				len += (buflen - len) - 1;
			else
				len += len2;
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Guild War
/////////////////////////////////////////////////////////////////////
void CGuildManager::RequestCancelWar(uint32_t guild_id1, uint32_t guild_id2)
{
	sys_log(0, "RequestCancelWar %d %d", guild_id1, guild_id2);

	TPacketGuildWar p{};
	p.bWar = GUILD_WAR_CANCEL;
	p.dwGuildFrom = guild_id1;
	p.dwGuildTo = guild_id2;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
}

void CGuildManager::RequestEndWar(uint32_t guild_id1, uint32_t guild_id2)
{
	sys_log(0, "RequestEndWar %d %d", guild_id1, guild_id2);

	TPacketGuildWar p{};
	p.bWar = GUILD_WAR_END;
	p.dwGuildFrom = guild_id1;
	p.dwGuildTo = guild_id2;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
}

void CGuildManager::RequestWarOver(uint32_t dwGuild1, uint32_t dwGuild2, uint32_t dwGuildWinner, long lReward)
{
	CGuild* g1 = TouchGuild(dwGuild1);
	const CGuild* g2 = TouchGuild(dwGuild2);

	if (!g1 || !g2)
		return;

	if (g1->GetGuildWarState(g2->GetID()) != GUILD_WAR_ON_WAR)
	{
		sys_log(0, "RequestWarOver : both guild was not in war %u %u", dwGuild1, dwGuild2);
		RequestEndWar(dwGuild1, dwGuild2);
		return;
	}

	TPacketGuildWar p{};

	p.bWar = GUILD_WAR_OVER;
	// There is no reward even after the guild battle is over.
	//p.lWarPrice = lReward;
	p.lWarPrice = 0;
	p.bType = dwGuildWinner == 0 ? 1 : 0; // bType == 1 means draw for this packet.
#ifdef ENABLE_NEW_WAR_OPTIONS
	p.bRound = 0;
	p.bPoints = 0;
	p.bTime = 0;
#endif

	if (dwGuildWinner == 0)
	{
		p.dwGuildFrom = dwGuild1;
		p.dwGuildTo = dwGuild2;
	}
	else
	{
		p.dwGuildFrom = dwGuildWinner;
		p.dwGuildTo = dwGuildWinner == dwGuild1 ? dwGuild2 : dwGuild1;
	}

	db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR, 0, &p, sizeof(p));
	sys_log(0, "RequestWarOver : winner %u loser %u draw %u betprice %d", p.dwGuildFrom, p.dwGuildTo, p.bType, p.lWarPrice);
}

#ifdef ENABLE_NEW_WAR_OPTIONS
void CGuildManager::DeclareWar(uint32_t guild_id1, uint32_t guild_id2, uint8_t bType, uint8_t bRound, uint8_t bPoints, uint8_t bTime)
#else
void CGuildManager::DeclareWar(uint32_t guild_id1, uint32_t guild_id2, uint8_t bType)
#endif
{
	if (guild_id1 == guild_id2)
		return;

	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	if (!g1 || !g2)
		return;

#ifdef ENABLE_NEW_WAR_OPTIONS
	if (g1->DeclareWar(guild_id2, bType, GUILD_WAR_SEND_DECLARE, bRound, bPoints, bTime) &&
		g2->DeclareWar(guild_id1, bType, GUILD_WAR_RECV_DECLARE, bRound, bPoints, bTime))
#else
	if (g1->DeclareWar(guild_id2, bType, GUILD_WAR_SEND_DECLARE) &&
		g2->DeclareWar(guild_id1, bType, GUILD_WAR_RECV_DECLARE))
#endif
	{
		// @warme005
		{
			char buf[256];
			snprintf(buf, sizeof(buf), "[LS;475;%s;%s]", TouchGuild(guild_id1)->GetName(), TouchGuild(guild_id2)->GetName());
			SendNotice(buf);
		}
	}
}

void CGuildManager::RefuseWar(uint32_t guild_id1, uint32_t guild_id2)
{
	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	if (g1 && g2)
	{
		if (g2->GetMasterCharacter())
			g2->GetMasterCharacter()->ChatPacket(CHAT_TYPE_INFO, "[LS;476;%s]", g1->GetName());
	}

	if (g1 != nullptr)
		g1->RefuseWar(guild_id2);

	if (g2 != nullptr && g1 != nullptr)
		g2->RefuseWar(g1->GetID());
}

void CGuildManager::WaitStartWar(uint32_t guild_id1, uint32_t guild_id2)
{
	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	if (!g1 || !g2)
	{
		sys_log(0, "GuildWar: CGuildManager::WaitStartWar(%d,%d) - something wrong in arg. there is no guild like that.", guild_id1, guild_id2);
		return;
	}

	if (g1->WaitStartWar(guild_id2) || g2->WaitStartWar(guild_id1))
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "[LS;477;%s;%s]", g1->GetName(), g2->GetName());
		SendNotice(buf);
	}
}

struct FSendWarList
{
	FSendWarList(uint8_t subheader, uint32_t guild_id1, uint32_t guild_id2)
	{
		gid1 = guild_id1;
		gid2 = guild_id2;

		p.header = HEADER_GC_GUILD;
		p.size = sizeof(p) + sizeof(uint32_t) * 2;
		p.subheader = subheader;
	}

	void operator() (LPCHARACTER ch)
	{
		if (!ch)
			return;

		LPDESC d = ch->GetDesc();
		if (d)
		{
			d->BufferedPacket(&p, sizeof(p));
			d->BufferedPacket(&gid1, sizeof(uint32_t));
			d->Packet(&gid2, sizeof(uint32_t));
		}
	}

	uint32_t gid1, gid2;
	TPacketGCGuild p;
};

void CGuildManager::StartWar(uint32_t guild_id1, uint32_t guild_id2)
{
	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	if (!g1 || !g2)
		return;

	if (!g1->CheckStartWar(guild_id2) || !g2->CheckStartWar(guild_id1))
		return;

	g1->StartWar(guild_id2);
	g2->StartWar(guild_id1);

	char buf[256];
	snprintf(buf, sizeof(buf), "[LS;477;%s;%s]", g1->GetName(), g2->GetName());
	SendNotice(buf);

	if (guild_id1 > guild_id2)
		std::swap(guild_id1, guild_id2);

	CHARACTER_MANAGER::Instance().for_each_pc(FSendWarList(GUILD_SUBHEADER_GC_GUILD_WAR_LIST, guild_id1, guild_id2));
	m_GuildWar.insert(std::make_pair(guild_id1, guild_id2));
}

void SendGuildWarOverNotice(CGuild* g1, CGuild* g2, bool bDraw)
{
	if (g1 && g2)
	{
		char buf[256];

		if (bDraw)
		{
			snprintf(buf, sizeof(buf), "[LS;479;%s;%s]", g1->GetName(), g2->GetName());
		}
		else
		{
			if (g1->GetWarScoreAgainstTo(g2->GetID()) > g2->GetWarScoreAgainstTo(g1->GetID()))
			{
				snprintf(buf, sizeof(buf), "[LS;480;%s;%s]", g1->GetName(), g2->GetName());
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				FGuildWinner f;
				g1->ForEachOnMapMember(f, g1->GetGuildWarMapIndex(g2->GetID()));
#endif
			}
			else
			{
				snprintf(buf, sizeof(buf), "[LS;480;%s;%s]", g2->GetName(), g1->GetName());
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				FGuildWinner f;
				g2->ForEachOnMapMember(f, g2->GetGuildWarMapIndex(g1->GetID()));
#endif
			}

			SendNotice(buf);
		}
	}

#ifdef ENABLE_GUILDWAR_BUTTON
	g1->SendAllMembersButtonState(false);
	g2->SendAllMembersButtonState(false);
#endif
}

bool CGuildManager::EndWar(uint32_t guild_id1, uint32_t guild_id2)
{
	if (guild_id1 > guild_id2)
		std::swap(guild_id1, guild_id2);

	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	const std::pair<uint32_t, uint32_t> k = std::make_pair(guild_id1, guild_id2);

	const TGuildWarContainer::iterator it = m_GuildWar.find(k);

	if (m_GuildWar.end() == it)
	{
		sys_log(0, "EndWar(%d,%d) - EndWar request but guild is not in list", guild_id1, guild_id2);
		return false;
	}

	if (g1 && g2)
	{
		if (g1->GetGuildWarType(guild_id2) == GUILD_WAR_TYPE_FIELD)
		{
			SendGuildWarOverNotice(g1, g2, g1->GetWarScoreAgainstTo(guild_id2) == g2->GetWarScoreAgainstTo(guild_id1));
		}
	}
	else
	{
		return false;
	}

	if (g1)
		g1->EndWar(guild_id2);

	if (g2)
		g2->EndWar(guild_id1);

	m_GuildWarEndTime[k] = get_global_time();
	CHARACTER_MANAGER::Instance().for_each_pc(FSendWarList(GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST, guild_id1, guild_id2));
	m_GuildWar.erase(it);

	return true;
}

void CGuildManager::WarOver(uint32_t guild_id1, uint32_t guild_id2, bool bDraw)
{
	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	if (guild_id1 > guild_id2)
		std::swap(guild_id1, guild_id2);

	std::pair<uint32_t, uint32_t> k = std::make_pair(guild_id1, guild_id2);

	TGuildWarContainer::iterator it = m_GuildWar.find(k);

	if (m_GuildWar.end() == it)
		return;

	SendGuildWarOverNotice(g1, g2, bDraw);

	EndWar(guild_id1, guild_id2);
}

void CGuildManager::CancelWar(uint32_t guild_id1, uint32_t guild_id2)
{
	if (!EndWar(guild_id1, guild_id2))
		return;

	CGuild* g1 = FindGuild(guild_id1);
	CGuild* g2 = FindGuild(guild_id2);

	if (g1)
	{
		LPCHARACTER master1 = g1->GetMasterCharacter();

		if (master1)
			master1->ChatPacket(CHAT_TYPE_INFO, "[LS;482]");
	}

	if (g2)
	{
		LPCHARACTER master2 = g2->GetMasterCharacter();

		if (master2)
			master2->ChatPacket(CHAT_TYPE_INFO, "[LS;482]");
	}

	if (g1 && g2)
	{
		char buf[256 + 1];
		snprintf(buf, sizeof(buf), "[LS;483;%s;%s]", g1->GetName(), g2->GetName());
		SendNotice(buf);
	}
}

void CGuildManager::ReserveWar(uint32_t dwGuild1, uint32_t dwGuild2, uint8_t bType) // from DB
{
	sys_log(0, "GuildManager::ReserveWar %u %u", dwGuild1, dwGuild2);

	CGuild* g1 = FindGuild(dwGuild1);
	CGuild* g2 = FindGuild(dwGuild2);

	if (!g1 || !g2)
		return;

	g1->ReserveWar(dwGuild2, bType);
	g2->ReserveWar(dwGuild1, bType);
}

void CGuildManager::ShowGuildWarList(LPCHARACTER ch)
{
	if (!ch)
		return;

	for (auto& it : m_GuildWar)
	{
		CGuild* A = TouchGuild(it.first);
		const CGuild* B = TouchGuild(it.second);

		if (A && B)
		{
			ch->ChatPacket(CHAT_TYPE_NOTICE, "%s[%d] vs %s[%d] time %u sec.",
				A->GetName(), A->GetID(),
				B->GetName(), B->GetID(),
				get_global_time() - A->GetWarStartTime(B->GetID()));
		}
	}
}

void CGuildManager::SendGuildWar(LPCHARACTER ch)
{
	if (!ch)
		return;

	if (!ch->GetDesc())
		return;

	TEMP_BUFFER buf;
	TPacketGCGuild p{};
	p.header = HEADER_GC_GUILD;
	p.subheader = GUILD_SUBHEADER_GC_GUILD_WAR_LIST;
	p.size = static_cast<uint16_t>(sizeof(p) + (sizeof(uint32_t) * 2) * m_GuildWar.size());
	buf.write(&p, sizeof(p));

	for (auto& it : m_GuildWar)
	{
		buf.write(&it.first, sizeof(uint32_t));
		buf.write(&it.second, sizeof(uint32_t));
	}

#ifdef ENABLE_GUILDWAR_BUTTON
	ch->SendWarTeleportButton(m_GuildWar.size() <= 0 ? false : true);
#endif

	ch->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void SendGuildWarScore(uint32_t dwGuild, uint32_t dwGuildOpp, int iDelta, int iBetScoreDelta)
{
	TPacketGuildWarScore p{};

	p.dwGuildGainPoint = dwGuild;
	p.dwGuildOpponent = dwGuildOpp;
	p.lScore = iDelta;
	p.lBetScore = iBetScoreDelta;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR_SCORE, 0, &p, sizeof(TPacketGuildWarScore));
	sys_log(0, "SendGuildWarScore %u %u %d", dwGuild, dwGuildOpp, iDelta);
}

void CGuildManager::Kill(LPCHARACTER killer, LPCHARACTER victim)
{
	if (!killer || !victim)
		return;

	if (!killer->IsPC())
		return;

	if (!victim->IsPC())
		return;

	if (killer->GetWarMap())
	{
		killer->GetWarMap()->OnKill(killer, victim);
		return;
	}

	CGuild* gAttack = killer->GetGuild();
	const CGuild* gDefend = victim->GetGuild();

	if (!gAttack || !gDefend)
		return;

	if (gAttack->GetGuildWarType(gDefend->GetID()) != GUILD_WAR_TYPE_FIELD)
		return;

	if (!gAttack->UnderWar(gDefend->GetID()))
		return;

	SendGuildWarScore(gAttack->GetID(), gDefend->GetID(), victim->GetLevel());
}

void CGuildManager::StopAllGuildWar()
{
	for (auto& it : m_GuildWar)
	{
		CGuild* g = CGuildManager::Instance().TouchGuild(it.first);
		CGuild* pg = CGuildManager::Instance().TouchGuild(it.second);
		if (g && pg)
		{
			g->EndWar(it.second);
			pg->EndWar(it.first);
		}
	}

	m_GuildWar.clear();
}

void CGuildManager::ReserveWarAdd(TGuildWarReserve* p)
{
	const auto it = m_map_kReserveWar.find(p->dwID);

	CGuildWarReserveForGame* pkReserve;

	if (it != m_map_kReserveWar.end())
		pkReserve = it->second;
	else
	{
		pkReserve = M2_NEW CGuildWarReserveForGame;

		m_map_kReserveWar.insert(std::make_pair(p->dwID, pkReserve));
		m_vec_kReserveWar.emplace_back(pkReserve);
	}

	thecore_memcpy(&pkReserve->data, p, sizeof(TGuildWarReserve));

	sys_log(0, "ReserveWarAdd %u gid1 %u power %d gid2 %u power %d handicap %d",
		pkReserve->data.dwID, p->dwGuildFrom, p->lPowerFrom, p->dwGuildTo, p->lPowerTo, p->lHandicap);
}

void CGuildManager::ReserveWarBet(TPacketGDGuildWarBet* p)
{
	const auto it = m_map_kReserveWar.find(p->dwWarID);

	if (it == m_map_kReserveWar.end())
		return;

	it->second->mapBet.insert(std::make_pair(p->szLogin, std::make_pair(p->dwGuild, p->dwGold)));
}

bool CGuildManager::IsBet(uint32_t dwID, const char* c_pszLogin)
{
	const auto it = m_map_kReserveWar.find(dwID);

	if (it == m_map_kReserveWar.end())
		return true;

	return it->second->mapBet.end() != it->second->mapBet.find(c_pszLogin);
}

void CGuildManager::ReserveWarDelete(uint32_t dwID)
{
	sys_log(0, "ReserveWarDelete %u", dwID);
	const auto it = m_map_kReserveWar.find(dwID);

	if (it == m_map_kReserveWar.end())
		return;

	auto it_vec = m_vec_kReserveWar.begin();

	while (it_vec != m_vec_kReserveWar.end())
	{
		if (*it_vec == it->second)
		{
			it_vec = m_vec_kReserveWar.erase(it_vec);
			break;
		}
		else
			++it_vec;
	}

	M2_DELETE(it->second);
	m_map_kReserveWar.erase(it);
}

std::vector<CGuildWarReserveForGame*>& CGuildManager::GetReserveWarRef()
{
	return m_vec_kReserveWar;
}

//
// End of Guild War
//

void CGuildManager::ChangeMaster(uint32_t dwGID)
{
	const TGuildMap::iterator iter = m_mapGuild.find(dwGID);

	if (iter != m_mapGuild.end())
		iter->second->Load(dwGID);

	// Send updated information
	DBManager::Instance().FuncQuery(std::bind(&CGuild::SendGuildDataUpdateToAllMember, iter->second, std::placeholders::_1), "SELECT 1");
}
