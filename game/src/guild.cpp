#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "packet.h"
#include "desc_client.h"
#include "buffer_manager.h"
#include "char_manager.h"
#include "db.h"
#include "guild.h"
#include "guild_manager.h"
#include "affect.h"
#include "p2p.h"
#include "questmanager.h"
#include "building.h"
#include "locale_service.h"
#include "log.h"
#include "questmanager.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
#	include "item.h"
#	include "item_manager.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

SGuildMember::SGuildMember(LPCHARACTER ch, uint8_t grade, uint32_t offer_exp
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	, uint32_t join_date, uint8_t donate_limit, uint32_t last_donation, uint8_t daily_donate_count, uint32_t last_daily_donate
#endif
)
	: pid(ch->GetPlayerID()), grade(grade), is_general(0), job(ch->GetJob()), level(static_cast<uint8_t>(ch->GetLevel())), offer_exp(offer_exp), name(ch->GetName())
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	, join_date(join_date), donate_limit(donate_limit), last_donation(last_donation), daily_donate_count(daily_donate_count), last_daily_donate(last_daily_donate)
#endif
{}

SGuildMember::SGuildMember(uint32_t pid, uint8_t grade, uint8_t is_general, uint8_t job, uint8_t level, uint32_t offer_exp, char* name
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	, uint32_t join_date, uint8_t donate_limit, uint32_t last_donation, uint8_t daily_donate_count, uint32_t last_daily_donate
#endif
)
	: pid(pid), grade(grade), is_general(is_general), job(job), level(level), offer_exp(offer_exp), name(name)
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	, join_date(join_date), donate_limit(donate_limit), last_donation(last_donation), daily_donate_count(daily_donate_count), last_daily_donate(last_daily_donate)
#endif
{}

namespace
{
	struct FGuildNameSender
	{
		FGuildNameSender(uint32_t id, const char* guild_name) : id(id), name(guild_name)
		{
			p.header = HEADER_GC_GUILD;
			p.subheader = GUILD_SUBHEADER_GC_GUILD_NAME;
			p.size = sizeof(p) + sizeof(uint32_t) + GUILD_NAME_MAX_LEN;
		}

		void operator() (LPCHARACTER ch)
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

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	struct RefreshInfo
	{
		CGuild * p;
		RefreshInfo(CGuild * p) : p(p){}

		void operator()(LPCHARACTER ch)
		{
			if(ch && p)
				p->SendGuildInfoPacket(ch);
		}
	};
#endif
}

CGuild::CGuild(TGuildCreateParameter& cp)
{
	Initialize();

	m_general_count = 0;

	m_iMemberCountBonus = 0;

	strlcpy(m_data.name, cp.name, sizeof(m_data.name));
	m_data.master_pid = cp.master->GetPlayerID();
	strlcpy(m_data.grade_array[0].grade_name, LC_TEXT("Leader"), sizeof(m_data.grade_array[0].grade_name));
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	m_data.grade_array[0].auth_flag = GUILD_AUTH_ADD_MEMBER | GUILD_AUTH_REMOVE_MEMBER | GUILD_AUTH_NOTICE | GUILD_AUTH_USE_SKILL | GUILD_AUTH_WAR | GUILD_AUTH_BANK;
#else
	m_data.grade_array[0].auth_flag = GUILD_AUTH_ADD_MEMBER | GUILD_AUTH_REMOVE_MEMBER | GUILD_AUTH_NOTICE | GUILD_AUTH_USE_SKILL;
#endif

#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
	strlcpy(m_data.grade_array[1].grade_name, LC_TEXT("CoLeader"), sizeof(m_data.grade_array[1].grade_name));
	for (int i = GUILD_COLEADER_GRADE; i < GUILD_GRADE_COUNT; ++i)
#else
	for (int i = GUILD_LEADER_GRADE; i < GUILD_GRADE_COUNT; ++i)
#endif
	{
		strlcpy(m_data.grade_array[i].grade_name, LC_TEXT("Member"), sizeof(m_data.grade_array[i].grade_name));
		m_data.grade_array[i].auth_flag = 0;
	}

	auto pmsg(DBManager::Instance().DirectQuery(
		"INSERT INTO guild%s(name, master, sp, level, exp, skill_point, skill) "
		"VALUES('%s', %u, 1000, 1, 0, 0, '\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0')",
		get_table_postfix(), m_data.name, m_data.master_pid));

	// TODO if error occur?
	m_data.guild_id = static_cast<uint32_t>(pmsg->Get()->uiInsertID);

	for (int i = 0; i < GUILD_GRADE_COUNT; ++i)
	{
		DBManager::Instance().Query("INSERT INTO guild_grade%s VALUES(%u, %d, '%s', %d)",
			get_table_postfix(),
			m_data.guild_id,
			i + 1,
			m_data.grade_array[i].grade_name,
			m_data.grade_array[i].auth_flag);
	}

	ComputeGuildPoints();
	m_data.power = m_data.max_power;
	m_data.ladder_point = 0;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_CREATE, 0, &m_data.guild_id, sizeof(uint32_t));

	TPacketGuildSkillUpdate guild_skill{};
	guild_skill.guild_id = m_data.guild_id;
	guild_skill.amount = 0;
	guild_skill.skill_point = 0;
	memset(guild_skill.skill_levels, 0, GUILD_SKILL_COUNT);

	db_clientdesc->DBPacket(HEADER_GD_GUILD_SKILL_UPDATE, 0, &guild_skill, sizeof(guild_skill));

	// TODO GUILD_NAME
	CHARACTER_MANAGER::Instance().for_each_pc(FGuildNameSender(GetID(), GetName()));
	/*
	TPacketDGGuildMember p;
	memset(&p, 0, sizeof(p));
	p.dwPID = cp.master->GetPlayerID();
	p.bGrade = 15;
	AddMember(&p);
	*/
	RequestAddMember(cp.master, GUILD_LEADER_GRADE);
}

void CGuild::Initialize()
{
	memset(&m_data, 0, sizeof(m_data));
	m_data.level = 1;

	for (int i = 0; i < GUILD_SKILL_COUNT; ++i)
		abSkillUsable[i] = true;

	m_iMemberCountBonus = 0;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	storageOpen = false;
	storagePidOpen = 0;
#endif
}

CGuild::~CGuild()
{
}

void CGuild::RequestAddMember(LPCHARACTER ch, int grade)
{
	if (!ch)
		return;

	if (ch->GetGuild())
		return;

#ifdef ENABLE_PULSE_MANAGER
	if (!PulseManager::Instance().IncreaseClock(ch->GetPlayerID(), ePulse::GuildPulse, std::chrono::milliseconds(1000))) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
		return;
	}
#endif

	TPacketGDGuildAddMember gd{};

	if (m_member.find(ch->GetPlayerID()) != m_member.end())
	{
		sys_err("Already a member in guild %s[%d]", ch->GetName(), ch->GetPlayerID());
		return;
	}

	gd.dwPID = ch->GetPlayerID();
	gd.dwGuild = GetID();
	gd.bGrade = grade;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_ADD_MEMBER, 0, &gd, sizeof(TPacketGDGuildAddMember));
}

void CGuild::AddMember(TPacketDGGuildMember* p)
{
	TGuildMemberContainer::iterator it;

	if ((it = m_member.find(p->dwPID)) == m_member.end())
	{
		m_member.insert(std::make_pair(p->dwPID, TGuildMember(p->dwPID, p->bGrade, p->isGeneral, p->bJob, p->bLevel, p->dwOffer, p->szName
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
			, p->dwJoinDate, p->bDonateLimit, p->dwLastDonation, p->bDailyDonateCount, p->dwLastDailyDonate
#endif
		)));
	}
	else
	{
		TGuildMember& r_gm = it->second;
		r_gm.pid = p->dwPID;
		r_gm.grade = p->bGrade;
		r_gm.job = p->bJob;
		r_gm.offer_exp = p->dwOffer;
		r_gm.is_general = p->isGeneral;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		r_gm.join_date = p->dwJoinDate;
		r_gm.donate_limit = p->bDonateLimit;
		r_gm.last_donation = p->dwLastDonation;
		r_gm.daily_donate_count = p->bDailyDonateCount;
		r_gm.last_daily_donate = p->dwLastDailyDonate;
#endif
	}

	CGuildManager::Instance().Link(p->dwPID, this);

	SendListOneToAll(p->dwPID);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(p->dwPID);

	sys_log(0, "GUILD: AddMember PID %u, grade %u, job %u, level %u, offer %u, name %s ptr %p",
		p->dwPID, p->bGrade, p->bJob, p->bLevel, p->dwOffer, p->szName, get_pointer(ch));

	if (ch)
		LoginMember(ch);
	else
		P2PLoginMember(p->dwPID);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	if (ch)
		CAchievementSystem::Instance().OnSocial(ch, achievements::ETaskTypes::TYPE_JOIN_GUILD);
#endif
}

bool CGuild::RequestRemoveMember(uint32_t pid)
{
	TGuildMemberContainer::iterator it;

	if ((it = m_member.find(pid)) == m_member.end())
		return false;

	if (it->second.grade == GUILD_LEADER_GRADE)
		return false;

	TPacketGuild gd_guild{};
	gd_guild.dwGuild = GetID();
	gd_guild.dwInfo = pid;

	db_clientdesc->DBPacket(HEADER_GD_GUILD_REMOVE_MEMBER, 0, &gd_guild, sizeof(TPacketGuild));
	return true;
}

bool CGuild::RemoveMember(uint32_t pid)
{
	if (!pid)
		return false;

	sys_log(0, "Receive Guild P2P RemoveMember");
	TGuildMemberContainer::iterator it;

	if ((it = m_member.find(pid)) == m_member.end())
		return false;

	if (it->second.grade == GUILD_LEADER_GRADE)
		return false;

	if (it->second.is_general)
		m_general_count--;

	m_member.erase(it);
	SendOnlineRemoveOnePacket(pid);

	CGuildManager::Instance().Unlink(pid);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);

#ifdef ENABLE_PULSE_MANAGER
	if (!PulseManager::Instance().IncreaseClock(ch->GetPlayerID(), ePulse::GuildPulse, std::chrono::milliseconds(1000))) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
		return false;
	}
#endif

	if (ch)
	{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		CMeleyLairManager::Instance().GuildMemberRemoved(ch, this);
#endif
		//GuildRemoveAffect(ch);
		m_memberOnline.erase(ch);
		ch->SetGuild(nullptr);

		LogManager::Instance().CharLog(ch, 0, "GUILD_LEAVE", GetName());	//@fixme000
	}

	if (g_bGuildInviteLimit)
		DBManager::Instance().Query("REPLACE INTO guild_invite_limit VALUES(%d, %d)", GetID(), get_global_time());

	return true;
}

void CGuild::P2PLoginMember(uint32_t pid)
{
	if (m_member.find(pid) == m_member.end())
	{
		sys_err("GUILD [%d] is not a memeber of guild.", pid);
		return;
	}

	m_memberP2POnline.insert(pid);

	// Login event occur + Send List
	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
		SendLoginPacket(*it, pid);
}

void CGuild::LoginMember(LPCHARACTER ch)
{
	if (!ch)
		return;

	if (m_member.find(ch->GetPlayerID()) == m_member.end())
	{
		sys_err("GUILD %s[%d] is not a memeber of guild.", ch->GetName(), ch->GetPlayerID());
		return;
	}

	ch->SetGuild(this);

	// Login event occur + Send List
	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
		SendLoginPacket(*it, ch);

	m_memberOnline.insert(ch);

	SendAllGradePacket(ch);
	SendGuildInfoPacket(ch);
	SendListPacket(ch);
	SendSkillInfoPacket(ch);
	SendEnemyGuild(ch);
	//GuildUpdateAffect(ch);
}

void CGuild::P2PLogoutMember(uint32_t pid)
{
	if (m_member.find(pid) == m_member.end())
	{
		sys_err("GUILD [%d] is not a memeber of guild.", pid);
		return;
	}

	m_memberP2POnline.erase(pid);

	// Logout event occur
	TGuildMemberOnlineContainer::iterator it;
	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		SendLogoutPacket(*it, pid);
	}
}

void CGuild::LogoutMember(LPCHARACTER ch)
{
	if (!ch)
		return;

	if (m_member.find(ch->GetPlayerID()) == m_member.end())
	{
		sys_err("GUILD %s[%d] is not a memeber of guild.", ch->GetName(), ch->GetPlayerID());
		return;
	}

	//GuildRemoveAffect(ch);

	//ch->SetGuild(nullptr);
	m_memberOnline.erase(ch);

	// Logout event occur
	TGuildMemberOnlineContainer::iterator it;
	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		SendLogoutPacket(*it, ch);
	}
}

void CGuild::SendOnlineRemoveOnePacket(uint32_t pid)
{
	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 4;
	pack.subheader = GUILD_SUBHEADER_GC_REMOVE;

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&pid, sizeof(pid));

	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::SendAllGradePacket(LPCHARACTER ch)
{
	if (!ch)
		return;

	LPDESC d = ch->GetDesc();
	if (!d)
		return;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 1 + GUILD_GRADE_COUNT * (sizeof(TGuildGrade) + 1);
	pack.subheader = GUILD_SUBHEADER_GC_GRADE;

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));
	constexpr uint8_t n = 15;
	buf.write(&n, 1);

	for (int i = 0; i < GUILD_GRADE_COUNT; i++)
	{
		const uint8_t j = i + 1;
		buf.write(&j, 1);
		buf.write(&m_data.grade_array[i], sizeof(TGuildGrade));
	}

	d->Packet(buf.read_peek(), buf.size());
}

void CGuild::SendListOneToAll(LPCHARACTER ch)
{
	if (ch)
		SendListOneToAll(ch->GetPlayerID());
}

void CGuild::SendListOneToAll(uint32_t pid)
{
	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(TPacketGCGuild);
	pack.subheader = GUILD_SUBHEADER_GC_LIST;

	pack.size += sizeof(TGuildMemberPacketData);

	char c[CHARACTER_NAME_MAX_LEN + 1];
	memset(c, 0, sizeof(c));

	const TGuildMemberContainer::iterator cit = m_member.find(pid);
	if (cit == m_member.end())
		return;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (!d)
			continue;

		TEMP_BUFFER buf;

		buf.write(&pack, sizeof(pack));

		cit->second._dummy = 1;

		buf.write(&(cit->second), sizeof(uint32_t) * 3 + 1);
		buf.write(cit->second.name.c_str(), cit->second.name.length());
		buf.write(c, CHARACTER_NAME_MAX_LEN + 1 - cit->second.name.length());
		d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::SendListPacket(LPCHARACTER ch)
{
	if (!ch)
		return;

	/*
	List Packet

	Header
	Count (byte)
	[
	...
	name_flag 1 - Send or not send your name
	name CHARACTER_NAME_MAX_LEN+1
	] * Count

	*/
	LPDESC d;
	if (!(d = ch->GetDesc()))
		return;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(TPacketGCGuild);
	pack.subheader = GUILD_SUBHEADER_GC_LIST;

	pack.size += static_cast<uint16_t>(sizeof(TGuildMemberPacketData) * m_member.size());

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));

	char c[CHARACTER_NAME_MAX_LEN + 1];

	for (auto& it : m_member)
	{
		it.second._dummy = 1;

		buf.write(&(it.second), sizeof(uint32_t) * 3 + 1);

		strlcpy(c, it.second.name.c_str(), MIN(sizeof(c), it.second.name.length() + 1));

		buf.write(c, CHARACTER_NAME_MAX_LEN + 1);

		if (test_server)
			sys_log(0, "name %s job %d ", it.second.name.c_str(), it.second.job);
	}

	d->Packet(buf.read_peek(), buf.size());

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		SendLoginPacket(ch, *it);
	}

	for (TGuildMemberP2POnlineContainer::iterator it = m_memberP2POnline.begin(); it != m_memberP2POnline.end(); ++it)
	{
		SendLoginPacket(ch, *it);
	}

}

void CGuild::SendLoginPacket(LPCHARACTER ch, LPCHARACTER chLogin)
{
	if (chLogin)
		SendLoginPacket(ch, chLogin->GetPlayerID());
}

void CGuild::SendLoginPacket(LPCHARACTER ch, uint32_t pid)
{
	if (!ch)
		return;

	/*
	Login Packet
	header 4
	pid 4
	*/
	if (!ch->GetDesc())
		return;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 4;
	pack.subheader = GUILD_SUBHEADER_GC_LOGIN;

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));

	buf.write(&pid, 4);

	ch->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void CGuild::SendLogoutPacket(LPCHARACTER ch, LPCHARACTER chLogout)
{
	if (chLogout)
		SendLogoutPacket(ch, chLogout->GetPlayerID());
}

void CGuild::SendLogoutPacket(LPCHARACTER ch, uint32_t pid)
{
	if (!ch)
		return;

	/*
	Logout Packet
	header 4
	pid 4
	*/
	if (!ch->GetDesc())
		return;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 4;
	pack.subheader = GUILD_SUBHEADER_GC_LOGOUT;

	TEMP_BUFFER buf;

	buf.write(&pack, sizeof(pack));
	buf.write(&pid, 4);

	ch->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void CGuild::LoadGuildMemberData(SQLMsg* pmsg)
{
	if (pmsg && pmsg->Get()->uiNumRows == 0)
		return;

	m_general_count = 0;

	m_member.clear();

	for (uint i = 0; i < pmsg->Get()->uiNumRows; ++i)
	{
		int column = 0;
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

		uint32_t pid = strtoul(row[column++], (char**)nullptr, 10); // 0
		const uint8_t grade = (uint8_t)strtoul(row[column++], (char**)nullptr, 10); // 1
		uint8_t is_general = 0;

		column++; // 2
		if (row[column] && *row[column] == '1') // 2
			is_general = 1;

		const uint32_t offer = strtoul(row[column++], (char**)nullptr, 10); // 3
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		const uint32_t join_date = static_cast<uint32_t>(strtoul(row[column++], (char**)nullptr, 10)); // 4
		const uint8_t donate_limit = static_cast<uint8_t>(strtoul(row[column++], (char**)nullptr, 10)); // 5
		const uint32_t last_donation = static_cast<uint32_t>(strtoul(row[column++], (char**)nullptr, 10)); // 6
		const uint8_t daily_donate_count = static_cast<uint8_t>(strtoul(row[column++], (char**)nullptr, 10)); // 7
		const uint32_t last_daily_donate = static_cast<uint32_t>(strtoul(row[column++], (char**)nullptr, 10)); // 8
		const uint8_t level = static_cast<uint8_t>(strtoul(row[column++], (char**) nullptr, 10)); // 9
		const uint8_t job = static_cast<uint8_t>(strtoul(row[column++], (char**) nullptr, 10)); // 10
		char* name = row[column++]; // 11
#else
		const uint8_t level = (uint8_t)strtoul(row[column++], (char**)nullptr, 10); // 4
		const uint8_t job = (uint8_t)strtoul(row[column++], (char**)nullptr, 10); // 5
		char* name = row[column++]; // 6
#endif

		if (is_general)
			m_general_count++;

		m_member.insert(std::make_pair(pid, TGuildMember(pid, grade, is_general, job, level, offer, name
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
			, join_date, donate_limit, last_donation, daily_donate_count, last_daily_donate
#endif
		)));
		CGuildManager::Instance().Link(pid, this);
	}
}

void CGuild::LoadGuildGradeData(SQLMsg* pmsg)
{
	/*
	// Possibility of not being 15
	if (pmsg->Get()->iNumRows != 15)
	{
		sys_err("Query failed: getting guild grade data. GuildID(%d)", GetID());
		return;
	}
	*/
	for (uint i = 0; i < pmsg->Get()->uiNumRows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		uint8_t grade = 0;
		str_to_number(grade, row[0]);
		char* name = row[1];
		const uint32_t auth = strtoul(row[2], nullptr, 10);

		if (grade >= 1 && grade <= 15)
		{
			//sys_log(0, "GuildGradeLoad %s", name);
			strlcpy(m_data.grade_array[grade - 1].grade_name, name, sizeof(m_data.grade_array[grade - 1].grade_name));
			m_data.grade_array[grade - 1].auth_flag = auth;
		}
	}
}

void CGuild::LoadGuildData(SQLMsg* pmsg)
{
	if (pmsg && pmsg->Get()->uiNumRows == 0)
	{
		sys_err("Query failed: getting guild data %s", pmsg->stQuery.c_str());
		return;
	}

	MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
	m_data.master_pid = strtoul(row[0], (char**)nullptr, 10);
	m_data.level = (uint8_t)strtoul(row[1], (char**)nullptr, 10);
	m_data.exp = strtoul(row[2], (char**)nullptr, 10);
	strlcpy(m_data.name, row[3], sizeof(m_data.name));

	m_data.skill_point = (uint8_t)strtoul(row[4], (char**)nullptr, 10);
	if (row[5])
		thecore_memcpy(m_data.abySkill, row[5], sizeof(uint8_t) * GUILD_SKILL_COUNT);
	else
		memset(m_data.abySkill, 0, sizeof(uint8_t) * GUILD_SKILL_COUNT);

	m_data.power = MAX(0, strtoul(row[6], (char**)nullptr, 10));

	str_to_number(m_data.ladder_point, row[7]);

	if (m_data.ladder_point < 0)
		m_data.ladder_point = 0;

	str_to_number(m_data.win, row[8]);
	str_to_number(m_data.draw, row[9]);
	str_to_number(m_data.loss, row[10]);
	str_to_number(m_data.gold, row[11]);
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	str_to_number(m_data.dungeon_ch, row[12]);
	str_to_number(m_data.dungeon_map, row[13]);
	str_to_number(m_data.dungeon_cooldown, row[14]);
#endif
#ifdef ENABLE_GUILD_WAR_SCORE
	for (int w = 0; w < 3; ++w)
		str_to_number(m_data.winTypes[w], row[15+w]);
	for (int d = 0; d < 3; ++d)
		str_to_number(m_data.drawTypes[d], row[18+d]);
	for (int l = 0; l < 3; ++l)
		str_to_number(m_data.lossType[l], row[21+l]);
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	str_to_number(m_data.guildstorage, row[24]);
	strlcpy(m_data.lastGold, row[25], sizeof(m_data.lastGold));
	strlcpy(m_data.lastItem, row[26], sizeof(m_data.lastItem));
	str_to_number(m_data.guildstoragestate, row[27]);
	str_to_number(m_data.guildstoragewho, row[28]);
#endif
	ComputeGuildPoints();
}

void CGuild::Load(uint32_t guild_id)
{
	Initialize();

	m_data.guild_id = guild_id;

	DBManager::Instance().FuncQuery(std::bind(&CGuild::LoadGuildData, this, std::placeholders::_1),
		"SELECT master, level, exp, name, skill_point, skill, sp, ladder_point, win, draw, loss, gold"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		", dungeon_ch, dungeon_map, dungeon_cooldown"
#endif
#ifdef ENABLE_GUILD_WAR_SCORE
		", wintype0, wintype1, wintype2, drawtype0, drawtype1, drawtype2, losstype0, losstype1, losstype2"
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		", guildstorage, lastGold, lastItem, guildstoragestate, guildstoragewho"
#endif
		" FROM guild%s WHERE id = %u", get_table_postfix(), m_data.guild_id);

	sys_log(0, "GUILD: loading guild id %12s %u", m_data.name, guild_id);

	DBManager::Instance().FuncQuery(std::bind(&CGuild::LoadGuildGradeData, this, std::placeholders::_1),
		"SELECT grade, name, auth+0 FROM guild_grade%s WHERE guild_id = %u", get_table_postfix(), m_data.guild_id);

	DBManager::Instance().FuncQuery(std::bind(&CGuild::LoadGuildMemberData, this, std::placeholders::_1),
		"SELECT pid, grade, is_general, offer, "
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		"UNIX_TIMESTAMP(join_date), donate_limit, UNIX_TIMESTAMP(last_donation), daily_donate_count, UNIX_TIMESTAMP(last_daily_donate), "
#endif
		"level, job, name FROM guild_member%s, player%s WHERE guild_id = %u and pid = id", get_table_postfix(), get_table_postfix(), guild_id);
}

void CGuild::SaveLevel()
{
	DBManager::Instance().Query("UPDATE guild%s SET level=%d, exp=%u, skill_point=%d WHERE id = %u", get_table_postfix(), m_data.level, m_data.exp, m_data.skill_point, m_data.guild_id);
}

void CGuild::SendDBSkillUpdate(int amount)
{
	TPacketGuildSkillUpdate guild_skill{};
	guild_skill.guild_id = m_data.guild_id;
	guild_skill.amount = amount;
	guild_skill.skill_point = m_data.skill_point;
	thecore_memcpy(guild_skill.skill_levels, m_data.abySkill, sizeof(uint8_t) * GUILD_SKILL_COUNT);

	db_clientdesc->DBPacket(HEADER_GD_GUILD_SKILL_UPDATE, 0, &guild_skill, sizeof(guild_skill));
}

void CGuild::SaveSkill()
{
	char text[GUILD_SKILL_COUNT * 2 + 1];

	DBManager::Instance().EscapeString(text, sizeof(text), (const char*)m_data.abySkill, sizeof(m_data.abySkill));
	DBManager::Instance().Query("UPDATE guild%s SET sp = %d, skill_point=%d, skill='%s' WHERE id = %u",
		get_table_postfix(), m_data.power, m_data.skill_point, text, m_data.guild_id);
}

TGuildMember* CGuild::GetMember(uint32_t pid)
{
	TGuildMemberContainer::iterator it = m_member.find(pid);
	if (it == m_member.end())
		return nullptr;

	return &it->second;
}

uint32_t CGuild::GetMemberPID(const std::string& strName)
{
	for (auto& iter : m_member)
	{
		if (iter.second.name == strName)
			return iter.first;
	}

	return 0;
}

#ifdef ENABLE_SHOW_GUILD_LEADER
uint8_t CGuild::NewIsGuildGeneral(uint32_t pid)
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.is_general;
	}
	return 0;
}

bool CGuild::IsCoLeader(uint32_t pid)
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			if (iter.second.grade == GUILD_COLEADER_GRADE)
				return true;
			else
				return false;
	}

	return false;
}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
uint32_t CGuild::GetMemberJoinDate(uint32_t pid) noexcept	// Donationboard - Time Check
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.join_date;
	}

	return 0;
}

int CGuild::GetGuildMemberDonateCount(uint32_t pid) noexcept // Donationboard 0/3 Count
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.donate_limit;
	}
	return 0;
}

uint32_t CGuild::GetMemberLastDonation(uint32_t pid) noexcept // Last Donation time check
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.last_donation;
	}
	return 0;
}

int CGuild::GetDailyGuildDonatePoints(uint32_t pid) noexcept	// Get Player Donation Points
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.daily_donate_count;
	}
	return 0;
}

uint32_t CGuild::GetMemberLastDailyDonate(uint32_t pid) noexcept // Last daily donate time check
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.last_daily_donate;
	}
	return 0;
}

void CGuild::SetGuildAttendanceState(uint32_t pid, int donateCount, uint32_t dwTime) // Lock Donate Button [Main-Page] and add the count
{
	const TGuildMemberContainer::iterator it = m_member.find(pid);

	if (it == m_member.end())
		return;

	it->second.daily_donate_count += donateCount;
	it->second.last_daily_donate = dwTime;
	SaveMember(pid);
}

void CGuild::ResetGuildMemberDonateCount(uint32_t pid)	//Reset Daily Donation
{
	const TGuildMemberContainer::iterator it = m_member.find(pid);

	if (it == m_member.end())
		return;

	it->second.donate_limit = 0;
	SaveMember(pid);
}
#endif

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) || defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
uint8_t CGuild::IsGeneralMember(uint32_t pid) noexcept
{
	for (auto& iter : m_member)
	{
		if (iter.first == pid)
			return iter.second.is_general;
	}
	return 0;
}
#endif

void CGuild::__P2PUpdateGrade(SQLMsg* pmsg)
{
	if (pmsg && pmsg->Get()->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);

		int grade = 0;
		const char* name = row[1];
		int auth = 0;

		str_to_number(grade, row[0]);
		str_to_number(auth, row[2]);

		if (grade <= 0)
			return;

		grade--;

		// If the class name is different from the current one, update it
		if (0 != strcmp(m_data.grade_array[grade].grade_name, name))
		{
			strlcpy(m_data.grade_array[grade].grade_name, name, sizeof(m_data.grade_array[grade].grade_name));

			TPacketGCGuild pack{};

			pack.header = HEADER_GC_GUILD;
			pack.size = sizeof(pack);
			pack.subheader = GUILD_SUBHEADER_GC_GRADE_NAME;

			TOneGradeNamePacket pack2{};

			pack.size += sizeof(pack2);
			pack2.grade = grade + 1;
			strlcpy(pack2.grade_name, name, sizeof(pack2.grade_name));

			TEMP_BUFFER buf;

			buf.write(&pack, sizeof(pack));
			buf.write(&pack2, sizeof(pack2));

			for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
			{
				LPDESC d = (*it)->GetDesc();

				if (d)
					d->Packet(buf.read_peek(), buf.size());
			}
		}

		if (m_data.grade_array[grade].auth_flag != auth)
		{
			m_data.grade_array[grade].auth_flag = auth;

			TPacketGCGuild pack{};
			pack.header = HEADER_GC_GUILD;
			pack.size = sizeof(pack);
			pack.subheader = GUILD_SUBHEADER_GC_GRADE_AUTH;

			TOneGradeAuthPacket pack2{};
			pack.size += sizeof(pack2);
			pack2.grade = grade + 1;
			pack2.auth = auth;

			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(pack));
			buf.write(&pack2, sizeof(pack2));

			for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
			{
				LPDESC d = (*it)->GetDesc();
				if (d)
				{
					d->Packet(buf.read_peek(), buf.size());
				}
			}
		}
	}
}

void CGuild::P2PChangeGrade(uint8_t grade)
{
	DBManager::Instance().FuncQuery(std::bind(&CGuild::__P2PUpdateGrade, this, std::placeholders::_1),
		"SELECT grade, name, auth+0 FROM guild_grade%s WHERE guild_id = %u and grade = %d", get_table_postfix(), m_data.guild_id, grade);
}

namespace
{
	struct FSendChangeGrade
	{
		uint8_t grade;
		TPacketGuild p;

		FSendChangeGrade(uint32_t guild_id, uint8_t grade) : grade(grade)
		{
			p.dwGuild = guild_id;
			p.dwInfo = grade;
		}

		void operator()()
		{
			db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_GRADE, 0, &p, sizeof(p));
		}
	};
}

void CGuild::ChangeGradeName(uint8_t grade, const char* grade_name)
{
	if (grade == 1)
		return;

	if (grade < 1 || grade > 15)
	{
		sys_err("Wrong guild grade value %d", grade);
		return;
	}

	if (strlen(grade_name) > GUILD_NAME_MAX_LEN)
		return;

	if (!*grade_name)
		return;

	char text[GUILD_NAME_MAX_LEN * 2 + 1];

	DBManager::Instance().EscapeString(text, sizeof(text), grade_name, strlen(grade_name));
	DBManager::Instance().FuncAfterQuery(FSendChangeGrade(GetID(), grade), "UPDATE guild_grade%s SET name = '%s' where guild_id = %u and grade = %d", get_table_postfix(), text, m_data.guild_id, grade);

	grade--;
	strlcpy(m_data.grade_array[grade].grade_name, grade_name, sizeof(m_data.grade_array[grade].grade_name));

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack);
	pack.subheader = GUILD_SUBHEADER_GC_GRADE_NAME;

	TOneGradeNamePacket pack2{};
	pack.size += sizeof(pack2);
	pack2.grade = grade + 1;
	strlcpy(pack2.grade_name, grade_name, sizeof(pack2.grade_name));

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::ChangeGradeAuth(uint8_t grade, uint8_t auth)
{
	if (grade == 1)
		return;

	if (grade < 1 || grade > 15)
	{
		sys_err("Wrong guild grade value %d", grade);
		return;
	}

	DBManager::Instance().FuncAfterQuery(FSendChangeGrade(GetID(), grade), "UPDATE guild_grade%s SET auth = %d where guild_id = %u and grade = %d", get_table_postfix(), auth, m_data.guild_id, grade);

	grade--;

	m_data.grade_array[grade].auth_flag = auth;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack);
	pack.subheader = GUILD_SUBHEADER_GC_GRADE_AUTH;

	TOneGradeAuthPacket pack2{};
	pack.size += sizeof(pack2);
	pack2.grade = grade + 1;
	pack2.auth = auth;

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
			d->Packet(buf.read_peek(), buf.size());
	}
}

void CGuild::SendGuildInfoPacket(LPCHARACTER ch)
{
	if (!ch)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(TPacketGCGuild) + sizeof(TPacketGCGuildInfo);
	pack.subheader = GUILD_SUBHEADER_GC_INFO;

	TPacketGCGuildInfo pack_sub;

	memset(&pack_sub, 0, sizeof(TPacketGCGuildInfo));
	pack_sub.member_count = GetMemberCount();
	pack_sub.max_member_count = GetMaxMemberCount();
	pack_sub.guild_id = m_data.guild_id;
	pack_sub.master_pid = m_data.master_pid;
	pack_sub.exp = m_data.exp;
	pack_sub.level = m_data.level;
	strlcpy(pack_sub.name, m_data.name, sizeof(pack_sub.name));
	pack_sub.gold = m_data.gold;
	pack_sub.has_land = HasLand();
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	strlcpy(pack_sub.goldCheckout, GetLastCheckout(GS_GOLD), sizeof(pack_sub.goldCheckout));
	strlcpy(pack_sub.itemCheckout, GetLastCheckout(GS_ITEM), sizeof(pack_sub.itemCheckout));
#endif

	sys_log(0, "GMC guild_name %s", m_data.name);
	sys_log(0, "GMC master %d", m_data.master_pid);

	d->BufferedPacket(&pack, sizeof(TPacketGCGuild));
	d->Packet(&pack_sub, sizeof(TPacketGCGuildInfo));

#ifdef ENABLE_GUILD_WAR_SCORE
	SetWarInfo();
#endif
#ifdef ENABLE_GUILD_LAND_INFO
	SetGuildLandInfo();
#endif
#ifdef ENABLE_GUILDBANK_LOG
	SetBankInfo();
#endif
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	SetGuildDonateInfo();
#endif
}

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
const char* CGuild::GetLastCheckout(int type)
{
	const char* name = "None";

	if (type == 1)
		name = m_data.lastItem;
	else if (type == 2)
		name = m_data.lastGold;

	return name;
}

void CGuild::SetLastCheckoutP2P(const char* szName, int type)
{
	if(type < GS_ITEM || type > GS_GOLD)
		return;

	std::map<uint32_t, const char*> data;
	data[GS_GOLD] = "lastGold";
	data[GS_ITEM] = "lastItem";

	if (type == 1)
		strlcpy(m_data.lastItem, szName, sizeof(m_data.lastItem));
	else if (type == 2)
		strlcpy(m_data.lastGold, szName, sizeof(m_data.lastGold));

	auto pmsg(DBManager::Instance().DirectQuery("UPDATE guild%s SET %s = '%s' WHERE id = %d", get_table_postfix(), data[type], szName, GetID()));
}

void CGuild::RefreshP2P(CGuild * p)
{
	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), RefreshInfo(p));
}

void CGuild::SetLastCheckout(const char* szName, int type)
{
	TPacketGGGuild p1{};
	TPacketGGGuildChat p2{};

	p1.bHeader = HEADER_GG_GUILD;
	p1.bSubHeader = GUILD_SUBHEADER_GG_REFRESH1;
	p1.dwGuild = GetID();
	p2.dwGuild = type;
	strlcpy(p2.szText, szName, sizeof(p2.szText));

	P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
	P2P_MANAGER::Instance().Send(&p2, sizeof(TPacketGGGuildChat));
}
#endif

bool CGuild::OfferExp(LPCHARACTER ch, int amount)
{
	if (!ch)
		return false;

	const TGuildMemberContainer::iterator cit = m_member.find(ch->GetPlayerID());

	if (cit == m_member.end())
		return false;

	if (m_data.exp + amount < m_data.exp)
		return false;

	if (amount <= 0)
		return false;

	if (ch->GetExp() < static_cast<uint32_t>(amount))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;929]");
		return false;
	}

	if (ch->GetExp() - static_cast<uint32_t>(amount) > ch->GetExp())
	{
		sys_err("Wrong guild offer amount %d by %s[%u]", amount, ch->GetName(), ch->GetPlayerID());
		return false;
	}

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	// +5% EXP to guild donations [Lv. 5]

	const int necessary_exp = (guild_exp_table2[GetLevel()] - m_data.exp) * 100;

	if (GetLevel() >= 5)
	{
		if (amount > necessary_exp)
			amount = necessary_exp - static_cast<int>(necessary_exp * 0.05f);
		else
			amount = amount - static_cast<int>(amount * 0.05f);
	}
	else
	{
		if (amount > necessary_exp)
			amount = necessary_exp;
	}
#endif

	ch->PointChange(POINT_EXP, -amount);

	TPacketGuildExpUpdate guild_exp{};
	guild_exp.guild_id = GetID();
	guild_exp.amount = amount / 100;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_EXP_UPDATE, 0, &guild_exp, sizeof(guild_exp));
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	// +5% EXP to guild donations
	GuildPointChange(POINT_EXP, ((GetLevel() >= 5) ? static_cast<int>((amount + (amount * 0.05)) / 95) : static_cast<int>(amount / 100.0)), true);
#else
	GuildPointChange(POINT_EXP, amount / 100, true);
#endif

	cit->second.offer_exp += amount / 100;
	cit->second._dummy = 0;

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}

	SaveMember(ch->GetPlayerID());

	TPacketGuildChangeMemberData gd_guild{};

	gd_guild.guild_id = GetID();
	gd_guild.pid = ch->GetPlayerID();
	gd_guild.offer = cit->second.offer_exp;
	gd_guild.level = ch->GetLevel();
	gd_guild.grade = cit->second.grade;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	gd_guild.join_date = cit->second.join_date;
	gd_guild.donate_limit = cit->second.donate_limit;
	gd_guild.last_donation = cit->second.last_donation;
	gd_guild.daily_donate_count = cit->second.daily_donate_count;
	gd_guild.last_daily_donate = cit->second.last_daily_donate;
#endif

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));
	return true;
}

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
bool CGuild::OfferExpNew(LPCHARACTER ch, int amount)
{
	if (!ch)
		return false;

#ifdef ENABLE_PULSE_MANAGER
	if (!PulseManager::Instance().IncreaseClock(ch->GetPlayerID(), ePulse::GuildPulse, std::chrono::milliseconds(1000))) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
		return false;
	}
#endif

	const TGuildMemberContainer::iterator cit = m_member.find(ch->GetPlayerID());
	if (cit == m_member.end())
		return false;

	if (amount < 0 || amount >= 3)
		return false;

	if (ch->GetLevel() < 30)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1927]");
		return false;
	}

	const time_t waitJoinTime = (cit->second.join_date + (60 * 60 * 24));
	if (waitJoinTime > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1926]");
		return false;
	}

	const time_t waitLastDonationTime = (cit->second.last_donation + (60 * 60 * 24));
	if (waitLastDonationTime > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't donate today again - you reached the max!")); // Custom Text
		return false;
	}

	int iEmptyPos;
	const int experience[] = { 1000, 10000, 30000 };
	const int donation_required[] = { 120000, 56011, 56012 };
	const int medal_reward[] = { 3, 10, 30 };
	const int reward_donation[] = { 90010, 90011, 90012 };
	const int reward_count[] = { 3, 5, 15 };
	const int random_reward = number(1, 3);

	switch (amount)
	{
		case 0: // Gold
		{
			if (ch->GetGold() < donation_required[amount]) {
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;785]");
				return false;
			}
		}
		break;

		case 1: // Medium
		{
			if (ch->CountSpecifyItem(donation_required[1]) < 1) {
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1925]");
				return false;
			}
		}
		break;

		case 2: // Large
		{
			if (ch->CountSpecifyItem(donation_required[2]) < 1) {
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1924]");
				return false;
			}
		}
		break;

		default:
			break;
	}

	if (cit->second.donate_limit == MAX_GUILD_DONATION_PER_DAY - 1) // max donations
		cit->second.last_donation = get_global_time();

	if (cit->second.donate_limit >= MAX_GUILD_DONATION_PER_DAY) // max donations
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't donate today again - you reached the max!"));
		return false;
	}

	const uint32_t dwVnumReward = reward_donation[random_reward - 1];
	LPITEM item = ITEM_MANAGER::Instance().CreateItem(dwVnumReward);
	if (!item)
		return false;

#ifdef ENABLE_SPECIAL_INVENTORY
	iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item);
#else
	iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item->GetSize());
#endif
	if (iEmptyPos < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");// Not enough space in inventory
		return false;
	}

	if (amount == 0)
		ch->PointChange(POINT_GOLD, -(donation_required[amount]));
	else
		ch->RemoveSpecifyItem((donation_required[amount]), 1);

#ifdef ENABLE_MEDAL_OF_HONOR
	ch->PointChange(POINT_MEDAL_OF_HONOR, medal_reward[amount], true);
#endif

	ch->AutoGiveItem(dwVnumReward, reward_count[amount]);

	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1485;%d]", experience[amount]);
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;24;[IN;%d];%d]", item->GetVnum(), reward_count[amount]);
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1923;%d]", medal_reward[amount]);

	TPacketGuildExpUpdate guild_exp{};
	guild_exp.guild_id = GetID();
	guild_exp.amount = experience[amount] / 100;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_EXP_UPDATE, 0, &guild_exp, sizeof(guild_exp));
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	// +5% EXP to guild donations
	GuildPointChange(POINT_EXP, ((GetLevel() >= 5) ? static_cast<int>((experience[amount] + (experience[amount] * 0.05)) / 95) : static_cast<int>(experience[amount] / 100.0)), true);
#else
	GuildPointChange(POINT_EXP, experience[amount] / 100, true);
#endif

	cit->second.offer_exp += experience[amount] / 100;
	cit->second._dummy = 0;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	cit->second.donate_limit += 1;
#endif

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}

	SaveMember(ch->GetPlayerID());
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	SetGuildDonateInfo();
#endif

	TPacketGuildChangeMemberData gd_guild{};

	gd_guild.guild_id = GetID();
	gd_guild.pid = ch->GetPlayerID();
	gd_guild.offer = cit->second.offer_exp;
	gd_guild.level = ch->GetLevel();
	gd_guild.grade = cit->second.grade;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	gd_guild.join_date = cit->second.join_date;
	gd_guild.donate_limit = cit->second.donate_limit;
	gd_guild.last_donation = cit->second.last_donation;
	gd_guild.daily_donate_count = cit->second.daily_donate_count;
	gd_guild.last_daily_donate = cit->second.last_daily_donate;
#endif

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));
	M2_DESTROY_ITEM(item);
	return true;
}
#endif

void CGuild::Disband()
{
	sys_log(0, "GUILD: Disband %s:%u", GetName(), GetID());

	//building::CLand* pLand = building::CManager::Instance().FindLandByGuild(GetID());
	//if (pLand)
	//	pLand->SetOwner(0);

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		if (ch)
		{
			ch->SetGuild(nullptr);
			SendOnlineRemoveOnePacket(ch->GetPlayerID());
			// @fixme401
			ch->SetQuestFlag("guild_manage.new_disband_time", get_global_time());
		}
	}

	for (TGuildMemberContainer::iterator it = m_member.begin(); it != m_member.end(); ++it)
	{
		CGuildManager::Instance().Unlink(it->first);
	}
}

void CGuild::RequestDisband(uint32_t pid)
{
	if (m_data.master_pid != pid)
		return;

	TPacketGuild gd_guild{};
	gd_guild.dwGuild = GetID();
	gd_guild.dwInfo = 0;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_DISBAND, 0, &gd_guild, sizeof(TPacketGuild));

	// LAND_CLEAR
	building::CManager::Instance().ClearLandByGuildID(GetID());
	// END_LAND_CLEAR

	//ADD_DELETE_FUNCTION_FOR_GUILD_ITEMS_IN_STORAGE
}

void CGuild::AddComment(LPCHARACTER ch, const std::string& str)
{
	if (!ch)
		return;

	if (str.length() > GUILD_COMMENT_MAX_LEN || str.length() == 0) // Added string null verification
		return;

#ifdef ENABLE_PULSE_MANAGER
	if (!PulseManager::Instance().IncreaseClock(ch->GetPlayerID(), ePulse::GuildPulse, std::chrono::milliseconds(1000))) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
		return;
	}
#endif

	char text[GUILD_COMMENT_MAX_LEN * 2 + 1];
	DBManager::Instance().EscapeString(text, sizeof(text), str.c_str(), str.length());

	DBManager::Instance().FuncAfterQuery(std::bind(&CGuild::RefreshCommentForce, this, ch->GetPlayerID()),
		"INSERT INTO guild_comment%s(guild_id, name, notice, content, time) VALUES(%u, '%s', %d, '%s', NOW())",
		get_table_postfix(), m_data.guild_id, ch->GetName(), (str[0] == '!') ? 1 : 0, text);
}

void CGuild::DeleteComment(LPCHARACTER ch, uint32_t comment_id)
{
	if (!ch)
		return;

	std::unique_ptr<SQLMsg> pmsg;

	if (GetMember(ch->GetPlayerID())->grade == GUILD_LEADER_GRADE)
		pmsg = DBManager::Instance().DirectQuery("DELETE FROM guild_comment%s WHERE id = %u AND guild_id = %u", get_table_postfix(), comment_id, m_data.guild_id);
	else
		pmsg = DBManager::Instance().DirectQuery("DELETE FROM guild_comment%s WHERE id = %u AND guild_id = %u AND name = '%s'", get_table_postfix(), comment_id, m_data.guild_id, ch->GetName());

	if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (uint32_t)-1)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;930]");
	else
		RefreshCommentForce(ch->GetPlayerID());
}

void CGuild::RefreshComment(LPCHARACTER ch)
{
	if (ch)
		RefreshCommentForce(ch->GetPlayerID());
}

void CGuild::RefreshCommentForce(uint32_t player_id)
{
	const LPCHARACTER& ch = CHARACTER_MANAGER::Instance().FindByPID(player_id);
	if (ch == nullptr)
		return;

	auto pmsg(DBManager::Instance().DirectQuery("SELECT id, name, content FROM guild_comment%s WHERE guild_id = %u ORDER BY notice DESC, id DESC LIMIT %d", get_table_postfix(), m_data.guild_id, GUILD_COMMENT_MAX_COUNT));

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 1;
	pack.subheader = GUILD_SUBHEADER_GC_COMMENTS;

	const uint8_t count = static_cast<uint8_t>(pmsg->Get()->uiNumRows);

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	pack.size += (sizeof(uint32_t) + CHARACTER_NAME_MAX_LEN + 1 + GUILD_COMMENT_MAX_LEN + 1) * (uint16_t)count;
	d->BufferedPacket(&pack, sizeof(pack));
	d->BufferedPacket(&count, 1);
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	char szContent[GUILD_COMMENT_MAX_LEN + 1];
	memset(szName, 0, sizeof(szName));
	memset(szContent, 0, sizeof(szContent));

	for (uint i = 0; i < pmsg->Get()->uiNumRows; i++)
	{
		MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
		uint32_t id = strtoul(row[0], nullptr, 10);

		strlcpy(szName, row[1], sizeof(szName));
		strlcpy(szContent, row[2], sizeof(szContent));

		d->BufferedPacket(&id, sizeof(id));
		d->BufferedPacket(szName, sizeof(szName));

		if (i == pmsg->Get()->uiNumRows - 1)
			d->Packet(szContent, sizeof(szContent)); // Send if last line
		else
			d->BufferedPacket(szContent, sizeof(szContent));
	}
}

bool CGuild::ChangeMemberGeneral(uint32_t pid, uint8_t is_general)
{
#ifndef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if (is_general && GetGeneralCount() >= GetMaxGeneralCount())
		return false;
#endif

	const TGuildMemberContainer::iterator it = m_member.find(pid);
	if (it == m_member.end())
		return true;

	is_general = is_general ? 1 : 0;

	if (it->second.is_general == is_general)
		return true;

	if (is_general)
		++m_general_count;
	else
		--m_general_count;

	it->second.is_general = is_general;

	TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 5;
	pack.subheader = GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL;

	while (itOnline != m_memberOnline.end())
	{
		LPDESC d = (*(itOnline++))->GetDesc();

		if (!d)
			continue;

		d->BufferedPacket(&pack, sizeof(pack));
		d->BufferedPacket(&pid, sizeof(pid));
		d->Packet(&is_general, sizeof(is_general));
	}

	SaveMember(pid);
	return true;
}

void CGuild::ChangeMemberGrade(uint32_t pid, uint8_t grade)
{
	if (grade == 1)
		return;

#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
	if (grade == 2) {
		char Query[1024];
		snprintf(Query, sizeof(Query), "SELECT COUNT(*) FROM guild_member WHERE guild_id=%d and grade=2", GetID());
		auto pmsg(DBManager::Instance().DirectQuery(Query));

		int count = 0;

		if (pmsg->Get()->uiNumRows == 0) {
			count = 0;
			sys_err("not found query");
		}
		else {
			MYSQL_ROW row = mysql_fetch_row(pmsg->Get()->pSQLResult);
			count = atoi(row[0]);
		}

		LPCHARACTER ch = GetMasterCharacter();
		if (count == 1) {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Only one CoLeader possible!!!"));
			return;
		}
	}
#endif

	const TGuildMemberContainer::iterator it = m_member.find(pid);

	if (it == m_member.end())
		return;

	it->second.grade = grade;

	TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 5;
	pack.subheader = GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE;

	while (itOnline != m_memberOnline.end())
	{
		LPDESC d = (*(itOnline++))->GetDesc();

		if (!d)
			continue;

		d->BufferedPacket(&pack, sizeof(pack));
		d->BufferedPacket(&pid, sizeof(pid));
		d->Packet(&grade, sizeof(grade));
	}

	SaveMember(pid);

	TPacketGuildChangeMemberData gd_guild{};
	gd_guild.guild_id = GetID();
	gd_guild.pid = pid;
	gd_guild.offer = it->second.offer_exp;
	gd_guild.level = it->second.level;
	gd_guild.grade = grade;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	gd_guild.join_date = it->second.join_date;
	gd_guild.donate_limit = it->second.donate_limit;
	gd_guild.last_donation = it->second.last_donation;
	gd_guild.daily_donate_count = it->second.daily_donate_count;
	gd_guild.last_daily_donate = it->second.last_daily_donate;
#endif

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));
}

void CGuild::SkillLevelUp(uint32_t dwVnum)
{
	uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such guild skill by number %u", dwVnum);
		return;
	}

	if (m_data.abySkill[dwRealVnum] >= pkSk->bMaxLevel)
		return;

	if (m_data.skill_point <= 0)
		return;
	m_data.skill_point--;

	m_data.abySkill[dwRealVnum] ++;

	ComputeGuildPoints();
	SaveSkill();
	SendDBSkillUpdate();

	/*switch (dwVnum)
	* {
	*	 case GUILD_SKILL_GAHO:
	*	{
	*		TGuildMemberOnlineContainer::iterator it;
	*
	*		for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	*		(*it)->PointChange(POINT_DEF_GRADE, 1);
	* }
	* break;
	* 
	*	case GUILD_SKILL_HIM:
	*	{
	*		TGuildMemberOnlineContainer::iterator it;
	* 
	*		for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	*		(*it)->PointChange(POINT_ATT_GRADE, 1);
	*	}
	*	break;
	* }*/

	for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, *this, std::placeholders::_1));

	sys_log(0, "Guild SkillUp: %s %d level %d type %u", GetName(), pkSk->dwVnum, m_data.abySkill[dwRealVnum], pkSk->dwType);
}

void CGuild::UseSkill(uint32_t dwVnum, LPCHARACTER ch, uint32_t pid)
{
	if (!ch)
		return;

	LPCHARACTER victim = nullptr;

	if (!GetMember(ch->GetPlayerID()) || !HasGradeAuth(GetMember(ch->GetPlayerID())->grade, GUILD_AUTH_USE_SKILL))
		return;

	sys_log(0, "GUILD_USE_SKILL : cname(%s), skill(%d)", ch ? ch->GetName() : "", dwVnum);

	uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

	if (!ch->CanMove())
		return;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such guild skill by number %u", dwVnum);
		return;
	}

	if (m_data.abySkill[dwRealVnum] == 0)
		return;

	if ((pkSk->dwFlag & SKILL_FLAG_SELFONLY))
	{
		// Don't use it because it's already hung.
		if (ch->FindAffect(pkSk->dwVnum))
			return;

		victim = ch;
	}

	if (ch->IsAffectFlag(AFF_REVIVE_INVISIBLE))
		ch->RemoveAffect(AFFECT_REVIVE_INVISIBLE);

	if (ch->IsAffectFlag(AFF_EUNHYUNG))
		ch->RemoveAffect(SKILL_EUNHYUNG);

	const double k = 1.0 * m_data.abySkill[dwRealVnum] / pkSk->bMaxLevel;
	pkSk->kSPCostPoly.SetVar("k", k);
	const int iNeededSP = (int)pkSk->kSPCostPoly.Eval();

	if (GetSP() < iNeededSP)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;931;%d;%d]", GetSP(), iNeededSP);
		return;
	}

	pkSk->kCooldownPoly.SetVar("k", k);
	const int iCooltime = (int)pkSk->kCooldownPoly.Eval();

	if (!abSkillUsable[dwRealVnum])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;932]");
		return;
	}

	{
		TPacketGuildUseSkill p{};
		p.dwGuild = GetID();
		p.dwSkillVnum = pkSk->dwVnum;
		p.dwCooltime = iCooltime;
		db_clientdesc->DBPacket(HEADER_GD_GUILD_USE_SKILL, 0, &p, sizeof(p));
	}
	abSkillUsable[dwRealVnum] = false;
	//abSkillUsed[dwRealVnum] = true;
	//adwSkillNextUseTime[dwRealVnum] = get_dword_time() + iCooltime * 1000;

	//PointChange(POINT_SP, -iNeededSP);
	//GuildPointChange(POINT_SP, -iNeededSP);

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;933;%d;%d;%d;%u]", dwVnum, GetSP(), iNeededSP, pid);

	switch (dwVnum)
	{
		case GUILD_SKILL_TELEPORT:
		// First try who is currently on the server.
		{
			SendDBSkillUpdate(-iNeededSP);
			if ((victim = (CHARACTER_MANAGER::Instance().FindByPID(pid))))
				ch->WarpSet(victim->GetX(), victim->GetY());
			else
			{
				if (m_memberP2POnline.find(pid) != m_memberP2POnline.end())
				{
					// There is someone logged in to another server -> Let's get the coordinates by sending a message
					// 1. Spray A.pid, B.pid
					// 2. The server with B.pid sends A.pid and coordinates to the rooted server.
					// 3. Warp
					CCI* pcci = P2P_MANAGER::Instance().FindByPID(pid);

					if (pcci->bChannel != g_bChannel)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;934;%d;%d]", pcci->bChannel, g_bChannel);
					}
					else
					{
						TPacketGGFindPosition p{};
						p.header = HEADER_GG_FIND_POSITION;
						p.dwFromPID = ch->GetPlayerID();
						p.dwTargetPID = pid;
						pcci->pkDesc->Packet(&p, sizeof(TPacketGGFindPosition));
						if (test_server)
							ch->ChatPacket(CHAT_TYPE_PARTY, "sent find position packet for guild teleport");
					}
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;935]");
			}
		}
		break;

		/*case GUILD_SKILL_ACCEL:
		* 	ch->RemoveAffect(dwVnum);
		* 	ch->AddAffect(dwVnum, POINT_MOV_SPEED, m_data.abySkill[dwRealVnum]*3, pkSk->dwAffectFlag, (int)pkSk->kDurationPoly.Eval(), 0, false);
		* 	ch->AddAffect(dwVnum, POINT_ATT_SPEED, m_data.abySkill[dwRealVnum]*3, pkSk->dwAffectFlag, (int)pkSk->kDurationPoly.Eval(), 0, false);
		* 	break;*/

		default:
			{
				/*if (ch->GetPlayerID() != GetMasterPID())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[17][Guild] You do not have permission to use guild skills."));
					return;
				}*/

				if (!UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;937]");
					return;
				}

				SendDBSkillUpdate(-iNeededSP);

				for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
				{
					LPCHARACTER victim2 = *it;
					victim2->RemoveAffect(dwVnum);
					ch->ComputeSkill(dwVnum, victim2, m_data.abySkill[dwRealVnum]);
				}
			}
			break;

			/*if (!victim)
				return;

			ch->ComputeSkill(dwVnum, victim, m_data.abySkill[dwRealVnum]);*/
	}
}

void CGuild::SendSkillInfoPacket(LPCHARACTER ch) const
{
	if (!ch)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	TPacketGCGuild pack{};

	pack.header = HEADER_GC_GUILD;
	pack.size = sizeof(pack) + 6 + GUILD_SKILL_COUNT;
	pack.subheader = GUILD_SUBHEADER_GC_SKILL_INFO;

	d->BufferedPacket(&pack, sizeof(pack));
	d->BufferedPacket(&m_data.skill_point, 1);
	d->BufferedPacket(&m_data.abySkill, GUILD_SKILL_COUNT);
	d->BufferedPacket(&m_data.power, 2);
	d->Packet(&m_data.max_power, 2);
}

void CGuild::ComputeGuildPoints()
{
	m_data.max_power = GUILD_BASE_POWER + (m_data.level - 1) * GUILD_POWER_PER_LEVEL;

	m_data.power = MINMAX(0, m_data.power, m_data.max_power);
}

int CGuild::GetSkillLevel(uint32_t vnum)
{
	const uint32_t dwRealVnum = vnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return 0;

	return m_data.abySkill[dwRealVnum];
}

/*void CGuild::GuildUpdateAffect(LPCHARACTER ch)
{
	if (GetSkillLevel(GUILD_SKILL_GAHO))
		ch->PointChange(POINT_DEF_GRADE, GetSkillLevel(GUILD_SKILL_GAHO));

	if (GetSkillLevel(GUILD_SKILL_HIM))
		ch->PointChange(POINT_ATT_GRADE, GetSkillLevel(GUILD_SKILL_HIM));
}*/

/*void CGuild::GuildRemoveAffect(LPCHARACTER ch)
{
	if (GetSkillLevel(GUILD_SKILL_GAHO))
		ch->PointChange(POINT_DEF_GRADE, -(int) GetSkillLevel(GUILD_SKILL_GAHO));

	if (GetSkillLevel(GUILD_SKILL_HIM))
		ch->PointChange(POINT_ATT_GRADE, -(int) GetSkillLevel(GUILD_SKILL_HIM));
}*/

void CGuild::UpdateSkill(uint8_t skill_point, uint8_t* skill_levels)
{
	//int iDefMoreBonus = 0;
	//int iAttMoreBonus = 0;

	m_data.skill_point = skill_point;
	/*if (skill_levels[GUILD_SKILL_GAHO - GUILD_SKILL_START]!=GetSkillLevel(GUILD_SKILL_GAHO))
	{
		iDefMoreBonus = skill_levels[GUILD_SKILL_GAHO - GUILD_SKILL_START]-GetSkillLevel(GUILD_SKILL_GAHO);
	}
	if (skill_levels[GUILD_SKILL_HIM - GUILD_SKILL_START]!=GetSkillLevel(GUILD_SKILL_HIM))
	{
		iAttMoreBonus = skill_levels[GUILD_SKILL_HIM - GUILD_SKILL_START]-GetSkillLevel(GUILD_SKILL_HIM);
	}

	if (iDefMoreBonus || iAttMoreBonus)
	{
		for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
		{
			(*it)->PointChange(POINT_ATT_GRADE, iAttMoreBonus);
			(*it)->PointChange(POINT_DEF_GRADE, iDefMoreBonus);
		}
	}*/

	thecore_memcpy(m_data.abySkill, skill_levels, sizeof(uint8_t) * GUILD_SKILL_COUNT);
	ComputeGuildPoints();
}

static uint32_t __guild_levelup_exp(int level)
{
	return guild_exp_table2[level];
}

void CGuild::GuildPointChange(uint16_t type, int amount, bool save)	//@fixme532
{
	switch (type)
	{
		case POINT_SP:
		{
			m_data.power += amount;

			m_data.power = MINMAX(0, m_data.power, m_data.max_power);

			if (save)
			{
				SaveSkill();
			}

			for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, *this, std::placeholders::_1));
		}
		break;

		case POINT_EXP:
		{
			if (amount < 0 && m_data.exp < (uint32_t)-amount)
			{
				m_data.exp = 0;
			}
			else
			{
				m_data.exp += amount;

				while (m_data.exp >= __guild_levelup_exp(m_data.level))
				{

					if (m_data.level < GUILD_MAX_LEVEL)
					{
						m_data.exp -= __guild_levelup_exp(m_data.level);
						++m_data.level;
						++m_data.skill_point;

						if (m_data.level > GUILD_MAX_LEVEL)
							m_data.level = GUILD_MAX_LEVEL;

						ComputeGuildPoints();
						GuildPointChange(POINT_SP, m_data.max_power - m_data.power);

						if (save)
							ChangeLadderPoint(GUILD_LADDER_POINT_PER_LEVEL);

						// NOTIFY_GUILD_EXP_CHANGE
						for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendGuildInfoPacket, this, std::placeholders::_1));
						// END_OF_NOTIFY_GUILD_EXP_CHANGE
					}

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
					if (m_data.level == 20)
#else
					if (m_data.level == GUILD_MAX_LEVEL)
#endif
					{
						m_data.exp = 0;
					}
				}
			}

			TPacketGCGuild pack{};
			pack.header = HEADER_GC_GUILD;
			pack.size = sizeof(pack) + 5;
			pack.subheader = GUILD_SUBHEADER_GC_CHANGE_EXP;

			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(pack));
			buf.write(&m_data.level, 1);
			buf.write(&m_data.exp, 4);

			for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
			{
				LPDESC d = (*it)->GetDesc();

				if (d)
					d->Packet(buf.read_peek(), buf.size());
			}

			if (save)
				SaveLevel();
		}
		break;

		default:
			break;
	}
}

void CGuild::SkillRecharge()
{
	//GuildPointChange(POINT_SP, m_data.max_power / 2);
	//GuildPointChange(POINT_SP, 10);
}

void CGuild::SaveMember(uint32_t pid)
{
	const TGuildMemberContainer::iterator it = m_member.find(pid);

	if (it == m_member.end())
		return;

	DBManager::Instance().Query("UPDATE guild_member%s SET grade = %d, offer = %u, is_general = %d"
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		", join_date = FROM_UNIXTIME(%d), donate_limit = %d, last_donation = FROM_UNIXTIME(%d), daily_donate_count = %d, last_daily_donate = FROM_UNIXTIME(%d)"
#endif
		" WHERE pid = %u and guild_id = %u",
		get_table_postfix(), it->second.grade, it->second.offer_exp, it->second.is_general,
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		it->second.join_date, it->second.donate_limit, it->second.last_donation, it->second.daily_donate_count, it->second.last_daily_donate,
#endif
		pid, m_data.guild_id);
}

void CGuild::LevelChange(uint32_t pid, uint8_t level)
{
	const TGuildMemberContainer::iterator cit = m_member.find(pid);

	if (cit == m_member.end())
		return;

	cit->second.level = level;

	TPacketGuildChangeMemberData gd_guild{};

	gd_guild.guild_id = GetID();
	gd_guild.pid = pid;
	gd_guild.offer = cit->second.offer_exp;
	gd_guild.grade = cit->second.grade;
	gd_guild.level = level;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	gd_guild.join_date = cit->second.join_date;
	gd_guild.donate_limit = cit->second.donate_limit;
	gd_guild.last_donation = cit->second.last_donation;
	gd_guild.daily_donate_count = cit->second.daily_donate_count;
	gd_guild.last_daily_donate = cit->second.last_daily_donate;
#endif

	db_clientdesc->DBPacket(HEADER_GD_GUILD_CHANGE_MEMBER_DATA, 0, &gd_guild, sizeof(gd_guild));

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;
	cit->second._dummy = 0;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();

		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}
}

void CGuild::ChangeMemberData(uint32_t pid, uint32_t offer, uint8_t level, uint8_t grade
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	, uint32_t join_date, uint8_t donate_limit, uint32_t last_donation, uint8_t daily_donate_count, uint32_t last_daily_donate
#endif
)
{
	const TGuildMemberContainer::iterator cit = m_member.find(pid);

	if (cit == m_member.end())
		return;

	cit->second.offer_exp = offer;
	cit->second.level = level;
	cit->second.grade = grade;
	cit->second._dummy = 0;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	cit->second.join_date = join_date;
	cit->second.donate_limit = donate_limit;
	cit->second.last_donation = last_donation;
	cit->second.daily_donate_count = daily_donate_count;
	cit->second.last_daily_donate = last_daily_donate;
#endif

	TPacketGCGuild pack{};
	pack.header = HEADER_GC_GUILD;

	for (TGuildMemberOnlineContainer::iterator it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (d)
		{
			pack.subheader = GUILD_SUBHEADER_GC_LIST;
			pack.size = sizeof(pack) + 13;
			d->BufferedPacket(&pack, sizeof(pack));
			d->Packet(&(cit->second), sizeof(uint32_t) * 3 + 1);
		}
	}
}

namespace
{
	struct FGuildChat
	{
		const char* c_pszText;

		FGuildChat(const char* c_pszText)
			: c_pszText(c_pszText)
			{}

		void operator()(LPCHARACTER ch)
		{
			ch->ChatPacket(CHAT_TYPE_GUILD, "%s", c_pszText);
		}
	};
}

void CGuild::P2PChat(const char* c_pszText)
{
	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), FGuildChat(c_pszText));
}

void CGuild::Chat(const char* c_pszText)
{
	std::for_each(m_memberOnline.begin(), m_memberOnline.end(), FGuildChat(c_pszText));

	TPacketGGGuild p1{};
	TPacketGGGuildChat p2{};

	p1.bHeader = HEADER_GG_GUILD;
	p1.bSubHeader = GUILD_SUBHEADER_GG_CHAT;
	p1.dwGuild = GetID();
	strlcpy(p2.szText, c_pszText, sizeof(p2.szText));

	P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
	P2P_MANAGER::Instance().Send(&p2, sizeof(TPacketGGGuildChat));
}

LPCHARACTER CGuild::GetMasterCharacter()
{
	return CHARACTER_MANAGER::Instance().FindByPID(GetMasterPID());
}

#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
bool CGuild::IsOnlineLeader()
{
	for (const auto& pkChr : m_memberOnline)
	{
		const auto& rkMember = GetMember(pkChr->GetPlayerID());
		if (rkMember && rkMember->grade == GUILD_LEADER_GRADE)
			return true;
	}

	for (const auto& dwPID : m_memberP2POnline)
	{
		const auto& rkMember = GetMember(dwPID);
		if (rkMember && rkMember->grade == GUILD_LEADER_GRADE)
			return true;
	}

	return false;
}
#endif

void CGuild::Packet(const void* buf, int size)
{
	for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPDESC d = (*it)->GetDesc();
		if (d)
			d->Packet(buf, size);
	}
}

int CGuild::GetTotalLevel() const
{
	int total = 0;

	for (auto& it : m_member)
	{
		total += it.second.level;
	}

	return total;
}

bool CGuild::ChargeSP(LPCHARACTER ch, int iSP)
{
	if (!ch)
		return false;

	int gold_percent = 100;

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	// Cost red.: Dragon God Power [Lv. 15]
	if (GetLevel() >= 15)
		gold_percent = 90;
#endif

	int gold = iSP * gold_percent;

	if (gold < iSP || ch->GetGold() < gold)
		return false;

	const int iRemainSP = m_data.max_power - m_data.power;

	if (iSP > iRemainSP)
	{
		iSP = iRemainSP;
		gold = iSP * gold_percent;
	}

	ch->PointChange(POINT_GOLD, -gold);

	DBManager::Instance().SendMoneyLog(MONEY_LOG_GUILD, 1, -gold);

	SendDBSkillUpdate(iSP);
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;938;%d]", iSP);
	}
	return true;
}

void CGuild::SkillUsableChange(uint32_t dwSkillVnum, bool bUsable)
{
	uint32_t dwRealVnum = dwSkillVnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	abSkillUsable[dwRealVnum] = bUsable;

	// GUILD_SKILL_COOLTIME_BUG_FIX
	sys_log(0, "CGuild::SkillUsableChange(guild=%s, skill=%d, usable=%d)", GetName(), dwSkillVnum, bUsable);
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
}

// GUILD_MEMBER_COUNT_BONUS
void CGuild::SetMemberCountBonus(int iBonus)
{
	m_iMemberCountBonus = iBonus;
	sys_log(0, "GUILD_IS_FULL_BUG : Bonus set to %d(val:%d)", iBonus, m_iMemberCountBonus);
}

void CGuild::BroadcastMemberCountBonus()
{
	TPacketGGGuild p1{};

	p1.bHeader = HEADER_GG_GUILD;
	p1.bSubHeader = GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS;
	p1.dwGuild = GetID();

	P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
	P2P_MANAGER::Instance().Send(&m_iMemberCountBonus, sizeof(int));
}

int CGuild::GetMaxMemberCount()
{
	// GUILD_IS_FULL_BUG_FIX
	if (m_iMemberCountBonus < 0 || m_iMemberCountBonus > 18)
		m_iMemberCountBonus = 0;
	// END_GUILD_IS_FULL_BUG_FIX

	if (g_bGuildInfiniteMembers)
		return INT_MAX;

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	// Increased max. member count [Lv 10]
	const uint8_t iGuildmemberCountBonus = GetLevel() >= 10 ? MEMBER_COUNT_BONUS : 0;
	const int MemberCountResult = 32 + iGuildmemberCountBonus + (2 * (m_data.level - 1)) + m_iMemberCountBonus;
	return (MemberCountResult > 90) ? 90 : MemberCountResult;
#else
	return 32 + 2 * (m_data.level - 1) + m_iMemberCountBonus;
#endif
}
// END_OF_GUILD_MEMBER_COUNT_BONUS

void CGuild::AdvanceLevel(int iLevel)
{
	if (m_data.level == iLevel)
		return;

	m_data.level = MIN(GUILD_MAX_LEVEL, iLevel);
}

void CGuild::RequestDepositMoney(LPCHARACTER ch, int iGold)
{
	if (!ch)
		return;

	if (!ch->CanDeposit())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;994]");
		return;
	}

	if (ch->GetGold() < iGold)
		return;

#ifdef ENABLE_GROWTH_PET_SYSTEM
	CGrowthPetSystem* petSystem = ch->GetGrowthPetSystem();
	if (petSystem && petSystem->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;151]");
		return;
	}
#endif

	ch->PointChange(POINT_GOLD, -iGold);

	TPacketGDGuildMoney p{};
	p.dwGuild = GetID();
	p.iGold = iGold;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_DEPOSIT_MONEY, 0, &p, sizeof(p));

	char buf[64 + 1];
	snprintf(buf, sizeof(buf), "%u %s", GetID(), GetName());
	LogManager::Instance().CharLog(ch, iGold, "GUILD_DEPOSIT", buf);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_EXTENDED_LOGS)
	LogManager::Instance().GuildLog(ch, ch->GetGuild()->GetID(), 0, "Yang", GUILD_GOLD_TYPE_IN, iGold, 1);
#endif

	ch->UpdateDepositPulse();
	sys_log(0, "GUILD: DEPOSIT %s:%u player %s[%u] gold %d", GetName(), GetID(), ch->GetName(), ch->GetPlayerID(), iGold);
}

#ifdef ENABLE_USE_MONEY_FROM_GUILD
void CGuild::RequestWithdrawMoney(LPCHARACTER ch, int iGold, bool buildPrice)
#else
void CGuild::RequestWithdrawMoney(LPCHARACTER ch, int iGold)
#endif
{
	if (!ch)
		return;

	if (!ch->CanDeposit())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;82]");
		return;
	}

#ifndef ENABLE_GUILDSTORAGE_SYSTEM
	if (ch->GetPlayerID() != GetMasterPID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;939]");
		return;
	}
#endif

	if (m_data.gold < iGold)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;687]");
		return;
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	CGrowthPetSystem* petSystem = ch->GetGrowthPetSystem();
	if (petSystem && petSystem->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;151]");
		return;
	}
#endif

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_USE_MONEY_FROM_GUILD)
	if (buildPrice)
	{
		int iChangeGoldTax = iGold;
		if (GetLevel() < 20) { // Keine Gildenbanksteuer [Bonus 4]
			uint32_t dwTax = 0;
			int iVal = 15;

			dwTax = iChangeGoldTax * iVal / 100;
			iChangeGoldTax -= dwTax;
		}

		ch->PointChange(POINT_GOLD, iChangeGoldTax);
		//if (test_server)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PAYED OUT %d, GIVE %d"), iGold, iChangeGoldTax);
		}
	}
#endif

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	SetLastCheckout(ch->GetName(), GS_GOLD);
	SetLastCheckoutP2P(ch->GetName(), GS_GOLD);
	SendGuildInfoPacket(ch);
	TPacketGGGuild p1{};
	TPacketGGGuildChat p2{};

	p1.bHeader = HEADER_GG_GUILD;
	p1.bSubHeader = GUILD_SUBHEADER_GG_REFRESH;
	p1.dwGuild = GetID();
	strlcpy(p2.szText, "none", sizeof(p2.szText));

	P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
	P2P_MANAGER::Instance().Send(&p2, sizeof(TPacketGGGuildChat));
#endif

	TPacketGDGuildMoney p{};
	p.dwGuild = GetID();
	p.iGold = iGold;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_WITHDRAW_MONEY, 0, &p, sizeof(p));

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_USE_MONEY_FROM_GUILD)
	char buf[64 + 1];
	snprintf(buf, sizeof(buf), "%u %s", GetID(), GetName());
	LogManager::Instance().CharLog(ch, iGold, "GUILD_WITHDRAW", buf);
#endif

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_EXTENDED_LOGS)
#	ifdef ENABLE_USE_MONEY_FROM_GUILD
	if (buildPrice)
#	endif
	{
		LogManager::Instance().GuildLog(ch, ch->GetGuild()->GetID(), 0, "Yang", GUILD_GOLD_TYPE_OUT, iGold, 1);
	}
#endif

	ch->UpdateDepositPulse();
	sys_log(0, "GUILD: WITHDRAW %s:%u player %s[%u] gold %d", GetName(), GetID(), ch->GetName(), ch->GetPlayerID(), iGold);
}

void CGuild::RecvMoneyChange(int iGold)
{
	m_data.gold = iGold;

	TPacketGCGuild p{};
	p.header = HEADER_GC_GUILD;
	p.size = sizeof(p) + sizeof(int);
	p.subheader = GUILD_SUBHEADER_GC_MONEY_CHANGE;

	for (auto it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		const LPCHARACTER& ch = *it;
		if (ch)
		{
			LPDESC d = ch->GetDesc();
			if (d)
			{
				d->BufferedPacket(&p, sizeof(p));
				d->Packet(&iGold, sizeof(int));
			}
		}
	}
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CGuild::SetStorageState(bool val, uint32_t pid)
{
	m_data.guildstoragestate = val ? 1 : 0;
	m_data.guildstoragewho = pid;
	auto pmsg(DBManager::Instance().DirectQuery("UPDATE guild%s SET guildstoragestate=%d, guildstoragewho = %d WHERE id = %d", get_table_postfix(), m_data.guildstoragestate, pid, GetID()));
};

void CGuild::SetGuildstorage(int val)
{
	m_data.guildstorage = val;
	auto pmsg(DBManager::Instance().DirectQuery("UPDATE guild%s SET guildstorage=%d WHERE id = %d", get_table_postfix(), m_data.guildstorage, GetID()));
}
#endif

#ifdef ENABLE_GUILD_WAR_SCORE
void CGuild::SetWarInfo()
{
	TPacketGCGuild p{};
	p.header = HEADER_GC_GUILD;
	p.size = sizeof(p) + sizeof(int);
	p.subheader = GUILD_SUBHEADER_GC_WAR_INFO;

	for (itertype(m_memberOnline) it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		LPDESC d = ch->GetDesc();
		d->BufferedPacket(&p, sizeof(p));

		for (int i = 0; i < 3; ++i) {
			d->Packet(&m_data.winTypes[i], sizeof(int));
		}
		for (int i = 0; i < 3; ++i) {
			d->Packet(&m_data.lossType[i], sizeof(int));
		}
		for (int i = 0; i < 3; ++i) {
			d->Packet(&m_data.drawTypes[i], sizeof(int));
		}
		d->Packet(&m_data.ladder_point, sizeof(int));
		int rank = CGuildManager::Instance().GetRank(this);
		d->Packet(&rank, sizeof(int));
	}
}
#endif

#ifdef ENABLE_GUILD_LAND_INFO
int CGuild::GetGuildLandIndex()
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT map_index FROM player.land WHERE guild_id = %d", GetID()));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	int maxIndex = 0;

	if (row[0] == 0)
		str_to_number(maxIndex, 0);
	else
		str_to_number(maxIndex, row[0]);

	sys_log(0, "GetGuildLandIndex: %u", maxIndex);
	return maxIndex;
}

int CGuild::GetGuildLandID()
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT id FROM player.land WHERE guild_id = %d", GetID()));
	if (pMsg->Get()->uiNumRows == 0)
		return 0;

	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	int LandID = 0;

	if (row[0] == 0)
		str_to_number(LandID, 0);
	else
		str_to_number(LandID, row[0]);

	sys_log(0, "GetGuildLandID: %u", LandID);
	return LandID;
}

void CGuild::SetGuildLandInfo()
{
	TPacketGCGuild p{};
	p.header = HEADER_GC_GUILD;
	p.size = sizeof(p) + sizeof(int);
	p.subheader = GUILD_SUBHEADER_GC_LAND_INFO;

	std::vector<int> landAltars = { 14061, 14062, 14063 };
	std::vector<int> Alchemists = { 14043, 14045, 14046, 14047, 14048, 14049, 14050, 14051, 14052, 14053, 14054, 14055, 14074, 14075, 14076, 14077, 14078, 14079 };
	std::vector<int> Blacksmiths = { 14013, 14014, 14015 };
	
	auto pkMsg(DBManager::Instance().DirectQuery("SELECT vnum FROM player.object WHERE land_id = %d", GetGuildLandID()));

	MYSQL_ROW row = nullptr;
	int guildAltar = 0;
	int guildAlchemist = 0;
	int guildBlacksmith = 0;
	while ((row = mysql_fetch_row(pkMsg->Get()->pSQLResult)))
	{
		int vnum = atoi(row[0]);

		std::vector<int>::iterator it = std::find(landAltars.begin(), landAltars.end(), vnum);
		if (it != landAltars.end())  {
			int pos = std::distance(landAltars.begin(), it);
			guildAltar=pos+1;
		}

		it = std::find(Alchemists.begin(), Alchemists.end(), vnum);
		if (it != Alchemists.end())  {
			int pos = std::distance(Alchemists.begin(), it);
			guildAlchemist=pos+1;
		}

		it = std::find(Blacksmiths.begin(), Blacksmiths.end(), vnum);
		if (it != Blacksmiths.end())  {
			int pos = std::distance(Blacksmiths.begin(), it);
			guildBlacksmith=pos+1;
		}
	}

	int index = GetGuildLandIndex();

	for (itertype(m_memberOnline) it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it) {
		LPCHARACTER ch = *it;
		LPDESC d = ch->GetDesc();
		d->BufferedPacket(&p, sizeof(p));
		d->Packet(&index, sizeof(int));				//Baseinfo
		d->Packet(&guildAltar, sizeof(int));		//Altar der Kraft LV.
		d->Packet(&guildAlchemist, sizeof(int));	//Gildenalchemist
		d->Packet(&guildBlacksmith, sizeof(int));	//Gildenschmied
		d->Packet(&m_data.guildstorage, sizeof(int));//Gildenlager LV.
	}
}
#endif

#ifdef ENABLE_GUILDBANK_LOG
void CGuild::SetBankInfo()
{
	for (itertype(m_memberOnline) it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		LPDESC d = ch->GetDesc();

		TPacketGCGuild p_del{};
		p_del.header = HEADER_GC_GUILD;
		p_del.size = sizeof(p_del);
		p_del.subheader = GUILD_SUBHEADER_GC_BANK_DELETE;
		d->Packet(&p_del, sizeof(p_del));

		auto pkMsg(DBManager::Instance().DirectQuery("SELECT chrname,itemname,iteminout,itemcount,datatype FROM log.guildstorage WHERE guild_id = %d ORDER by time DESC", GetID()));

		MYSQL_ROW row = nullptr;
		while ((row = mysql_fetch_row(pkMsg->Get()->pSQLResult)))
		{
			char szName[24];
			strlcpy(szName, row[0], sizeof(szName));
			char szItemName[24];
			strlcpy(szItemName, row[1], sizeof(szItemName));

			int action = atoi(row[2]);
			int count = atoi(row[3]);
			int datatype = atoi(row[4]);

			TPacketGCGuild p{};
			p.header = HEADER_GC_GUILD;
			p.size = sizeof(p) + sizeof(szName) + sizeof(szItemName) + sizeof(action) + sizeof(count) + sizeof(datatype);
			p.subheader = GUILD_SUBHEADER_GC_BANK_INFO;

			TEMP_BUFFER buf;
			buf.write(&p, sizeof(p));
			buf.write(szName, sizeof(szName));
			buf.write(szItemName, sizeof(szItemName));
			buf.write(&action, sizeof(action));
			buf.write(&count, sizeof(count));
			buf.write(&datatype, sizeof(datatype));

			d->Packet(buf.read_peek(), buf.size());
		}
	}
}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
void CGuild::SetGuildDonateInfo()
{
	for (itertype(m_memberOnline) it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		const LPCHARACTER& ch = *it;
		if (!ch)
			continue;
		LPDESC d = ch->GetDesc();
		if (!d)
			continue;
		const int medals = ch->GetMedalHonor();

		int donateCount = GetGuildMemberDonateCount(ch->GetPlayerID());
		if (!donateCount)
			donateCount = 0;

		const time_t waitTime = (GetMemberLastDonation(ch->GetPlayerID()) + (60 * 60 * 24));
		if ((donateCount >= MAX_GUILD_DONATION_PER_DAY) && (waitTime < get_global_time()))
		{
			ResetGuildMemberDonateCount(ch->GetPlayerID());
			donateCount = 0;
		}

		TPacketGCGuild p{};
		p.header = HEADER_GC_GUILD;
		p.size = sizeof(p) + sizeof(medals);
		p.subheader = GUILD_SUBHEADER_GC_DONATE;

		d->BufferedPacket(&p, sizeof(p));
		d->Packet(&medals, sizeof(medals));
		d->Packet(&donateCount, sizeof(donateCount));
	}
}
#endif

#ifndef ENABLE_USE_MONEY_FROM_GUILD
void CGuild::RecvWithdrawMoneyGive(int iChangeGold)
{
	LPCHARACTER ch = GetMasterCharacter();

	if (ch) {
		ch->PointChange(POINT_GOLD, iChangeGold);
		sys_log(0, "GUILD: WITHDRAW %s:%u player %s[%u] gold %d", GetName(), GetID(), ch->GetName(), ch->GetPlayerID(), iChangeGold);
	}

	TPacketGDGuildMoneyWithdrawGiveReply p;
	p.dwGuild = GetID();
	p.iChangeGold = iChangeGold;
	p.bGiveSuccess = ch ? 1 : 0;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY, 0, &p, sizeof(p));
}
#endif

bool CGuild::HasLand()
{
	return building::CManager::Instance().FindLandByGuild(GetID()) != nullptr;
}

// GUILD_JOIN_BUG_FIX
/// Guild invitation event information
EVENTINFO(TInviteGuildEventInfo)
{
	uint32_t dwInviteePID; ///< PID of the invited character
	uint32_t dwGuildID; ///< ID of the guild you invited

	TInviteGuildEventInfo()
		: dwInviteePID(0)
		, dwGuildID(0)
	{
	}
};

/**
* Guild invitation event callback function.
* When an event is triggered, it is treated as rejection of the invitation.
*/
EVENTFUNC(GuildInviteEvent)
{
	const TInviteGuildEventInfo* pInfo = dynamic_cast<TInviteGuildEventInfo*>(event->info);

	if (pInfo == nullptr)
	{
		sys_err("GuildInviteEvent> <Factor> Null pointer");
		return 0;
	}

	CGuild* pGuild = CGuildManager::Instance().FindGuild(pInfo->dwGuildID);

	if (pGuild)
	{
		sys_log(0, "GuildInviteEvent %s", pGuild->GetName());
		pGuild->InviteDeny(pInfo->dwInviteePID);
	}

	return 0;
}

void CGuild::Invite(LPCHARACTER pchInviter, LPCHARACTER pchInvitee)
{
	if (!pchInviter || !pchInvitee)
		return;

	if (quest::CQuestManager::Instance().GetPCForce(pchInviter->GetPlayerID())->IsRunning() == true)
	{
		pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;1058]");
		return;
	}

	if (quest::CQuestManager::Instance().GetPCForce(pchInvitee->GetPlayerID())->IsRunning() == true)
		return;

	if (pchInvitee->IsBlockMode(BLOCK_GUILD_INVITE))
	{
		pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;940]");
		return;
	}
	else if (!HasGradeAuth(GetMember(pchInviter->GetPlayerID())->grade, GUILD_AUTH_ADD_MEMBER))
	{
		pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;941]");
		return;
	}
	else if (pchInvitee->GetEmpire() != pchInviter->GetEmpire())
	{
		pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;942]");
		return;
	}

	const GuildJoinErrCode errcode = VerifyGuildJoinableCondition(pchInvitee);
	switch (errcode)
	{
		case GERR_NONE:
			break;

		case GERR_WITHDRAWPENALTY:
			pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;943;%d]", quest::CQuestManager::Instance().GetEventFlag( "guild_withdraw_delay" ) );
			return;

		case GERR_COMMISSIONPENALTY:
			pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;944;%d]", quest::CQuestManager::Instance().GetEventFlag( "guild_disband_delay") );
			return;

		case GERR_ALREADYJOIN:
			pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;945]");
			return;

		case GERR_GUILDISFULL:
			pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;946]");
			return;

		case GERR_GUILD_IS_IN_WAR:
			pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;537]");
			return;

		case GERR_INVITE_LIMIT:
			pchInviter->ChatPacket(CHAT_TYPE_INFO, "[LS;946]");
			return;

		default:
			sys_err("ignore guild join error(%d)", errcode);
			return;
	}

	if (m_GuildInviteEventMap.end() != m_GuildInviteEventMap.find(pchInvitee->GetPlayerID()))
		return;

	//
	// Event creation
	//
	TInviteGuildEventInfo* pInfo = AllocEventInfo<TInviteGuildEventInfo>();
	pInfo->dwInviteePID = pchInvitee->GetPlayerID();
	pInfo->dwGuildID = GetID();

	m_GuildInviteEventMap.insert(EventMap::value_type(pchInvitee->GetPlayerID(), event_create(GuildInviteEvent, pInfo, PASSES_PER_SEC(10))));

	//
	// Send an invitation packet to the invited character
	//
	const uint32_t gid = GetID();

	TPacketGCGuild p{};
	p.header = HEADER_GC_GUILD;
	p.size = sizeof(p) + sizeof(uint32_t) + GUILD_NAME_MAX_LEN + 1;
	p.subheader = GUILD_SUBHEADER_GC_GUILD_INVITE;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(&gid, sizeof(uint32_t));
	buf.write(GetName(), GUILD_NAME_MAX_LEN + 1);

	pchInvitee->GetDesc()->Packet(buf.read_peek(), buf.size());
}

void CGuild::InviteAccept(LPCHARACTER pchInvitee)
{
	if (!pchInvitee)
		return;

	const EventMap::iterator itFind = m_GuildInviteEventMap.find(pchInvitee->GetPlayerID());
	if (itFind == m_GuildInviteEventMap.end())
	{
		sys_log(0, "GuildInviteAccept from not invited character(invite guild: %s, invitee: %s)", GetName(), pchInvitee->GetName());
		return;
	}

	event_cancel(&itFind->second);
	m_GuildInviteEventMap.erase(itFind);

	const GuildJoinErrCode errcode = VerifyGuildJoinableCondition(pchInvitee);
	switch (errcode)
	{
		case GERR_NONE:
			break;
		case GERR_WITHDRAWPENALTY:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;943;%d]", quest::CQuestManager::Instance().GetEventFlag( "guild_withdraw_delay" ) );
			return;

		case GERR_COMMISSIONPENALTY:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;944;%d]", quest::CQuestManager::Instance().GetEventFlag( "guild_disband_delay") );
			return;

		case GERR_ALREADYJOIN:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;945]");
			return;

		case GERR_GUILDISFULL:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;946]");
			return;

		case GERR_GUILD_IS_IN_WAR:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;537]");
			return;

		case GERR_INVITE_LIMIT:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, "[LS;946]");
			return;

		default:
			sys_err("ignore guild join error(%d)", errcode);
			return;
	}

	RequestAddMember(pchInvitee, 15);
}

void CGuild::InviteDeny(uint32_t dwPID)
{
	EventMap::iterator itFind = m_GuildInviteEventMap.find(dwPID);
	if (itFind == m_GuildInviteEventMap.end())
	{
		sys_log(0, "GuildInviteDeny from not invited character(invite guild: %s, invitee PID: %d)", GetName(), dwPID);
		return;
	}

	event_cancel(&itFind->second);
	m_GuildInviteEventMap.erase(itFind);
}

CGuild::GuildJoinErrCode CGuild::VerifyGuildJoinableCondition(const LPCHARACTER pchInvitee)
{
	if (!pchInvitee)
		return GERR_NONE;

	if (get_global_time() - pchInvitee->GetQuestFlag("guild_manage.new_withdraw_time") < CGuildManager::Instance().GetWithdrawDelay() )
		return GERR_WITHDRAWPENALTY;
	else if (get_global_time() - pchInvitee->GetQuestFlag("guild_manage.new_disband_time") < CGuildManager::Instance().GetDisbandDelay())
		return GERR_COMMISSIONPENALTY;
	else if (pchInvitee->GetGuild())
		return GERR_ALREADYJOIN;
	else if (GetMemberCount() >= GetMaxMemberCount())
	{
		sys_log(1, "GuildName = %s, GetMemberCount() = %d, GetMaxMemberCount() = %d (32 + MAX(level(%d)-10, 0) * 2 + bonus(%d)", GetName(), GetMemberCount(), GetMaxMemberCount(), m_data.level, m_iMemberCountBonus);
		return GERR_GUILDISFULL;
	}
	else if (UnderAnyWar() != 0)
	{
		return GERR_GUILD_IS_IN_WAR;
	}
	else if (g_bGuildInviteLimit)
	{
		auto pMsg(DBManager::Instance().DirectQuery("SELECT value FROM guild_invite_limit WHERE id=%d", GetID()));
		if (pMsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			time_t limit_time = 0;
			str_to_number(limit_time, row[0]);

			if (test_server)
				limit_time += quest::CQuestManager::Instance().GetEventFlag("guild_invite_limit") * 60;
			else
				limit_time += quest::CQuestManager::Instance().GetEventFlag("guild_invite_limit") * 24 * 60 * 60;

			if (get_global_time() < limit_time)
				return GERR_INVITE_LIMIT;
		}
	}

	return GERR_NONE;
}
// END_OF_GUILD_JOIN_BUG_FIX

bool CGuild::ChangeMasterTo(uint32_t dwPID)
{
	if (GetMember(dwPID) == nullptr)
		return false;

	TPacketChangeGuildMaster p{};
	p.dwGuildID = GetID();
	p.idFrom = GetMasterPID();
	p.idTo = dwPID;

	db_clientdesc->DBPacket(HEADER_GD_REQ_CHANGE_GUILD_MASTER, 0, &p, sizeof(p));
	return true;
}

void CGuild::SendGuildDataUpdateToAllMember(SQLMsg* pmsg)
{
	TGuildMemberOnlineContainer::iterator iter = m_memberOnline.begin();

	for (; iter != m_memberOnline.end(); ++iter)	//@fixme541
	{
		SendGuildInfoPacket(*iter);
		SendAllGradePacket(*iter);
	}
}

#ifdef ENABLE_GUILDWAR_BUTTON
void CGuild::SendAllMembersButtonState(bool show)
{
	TGuildMemberOnlineContainer::iterator itOnline = m_memberOnline.begin();

	while (itOnline != m_memberOnline.end())
	{
		LPDESC d = (*(itOnline++))->GetDesc();

		if (!d || !d->GetCharacter())
			continue;

		d->GetCharacter()->SendWarTeleportButton(show);
	}
}
#endif

#ifdef ENABLE_D_NJGUILD
void CGuild::SetDungeon_for_Only_guild(LPDUNGEON pDungeon)
{
	m_pkDungeon_for_Only_guild = pDungeon;
}

LPDUNGEON CGuild::GetDungeon_for_Only_guild()
{
	return m_pkDungeon_for_Only_guild;
}
#endif

#ifdef ENABLE_NEWSTUFF
void CGuild::SetSkillLevel(uint32_t dwVnum, uint8_t level, uint8_t point)
{
	uint32_t dwRealVnum = dwVnum - GUILD_SKILL_START;

	if (dwRealVnum >= GUILD_SKILL_COUNT)
		return;

	const CSkillProto* pkSk = CSkillManager::Instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such guild skill by number %u", dwVnum);
		return;
	}

	if (level > pkSk->bMaxLevel)
		return;

	if (point)
	{
		if (m_data.skill_point < point)
			return;
		m_data.skill_point -= point;
	}

	m_data.abySkill[dwRealVnum] = level;

	ComputeGuildPoints();
	SaveSkill();
	SendDBSkillUpdate();

	for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, *this, std::placeholders::_1));

	sys_log(0, "Guild SetSkillLevel: %s %d level %d type %u", GetName(), pkSk->dwVnum, m_data.abySkill[dwRealVnum], pkSk->dwType);
}

uint32_t CGuild::GetSkillPoint()
{
	return m_data.skill_point;
}

void CGuild::SetSkillPoint(uint8_t point)
{
	m_data.skill_point = point;
	SendDBSkillUpdate();
	for_each(m_memberOnline.begin(), m_memberOnline.end(), std::bind(&CGuild::SendSkillInfoPacket, *this, std::placeholders::_1));
}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
bool CGuild::RequestDungeon(uint8_t bChannel, long lMapIndex)
{
	TPacketGDGuildDungeon sPacket{};
	sPacket.dwGuildID = GetID();
	sPacket.bChannel = bChannel;
	sPacket.lMapIndex = lMapIndex;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_DUNGEON, 0, &sPacket, sizeof(sPacket));
	return true;
}

void CGuild::RecvDungeon(uint8_t bChannel, long lMapIndex)
{
	m_data.dungeon_ch = bChannel;
	m_data.dungeon_map = lMapIndex;
}

bool CGuild::SetDungeonCooldown(uint32_t dwTime)
{
	TPacketGDGuildDungeonCD sPacket{};
	sPacket.dwGuildID = GetID();
	sPacket.dwTime = dwTime;
	db_clientdesc->DBPacket(HEADER_GD_GUILD_DUNGEON_CD, 0, &sPacket, sizeof(sPacket));
	return true;
}

void CGuild::RecvDungeonCD(uint32_t dwTime)
{
	m_data.dungeon_cooldown = dwTime;
}
#endif
