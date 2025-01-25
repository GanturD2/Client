#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "log.h"

#include "char.h"
#include "desc.h"
#include "item.h"

static char __escape_hint[1024];

LogManager::LogManager() : m_bIsConnect(false)
{
}

LogManager::~LogManager()
{
}

bool LogManager::Connect(const char* host, const int port, const char* user, const char* pwd, const char* db)
{
	if (m_sql.Setup(host, user, pwd, db, g_stLocale.c_str(), false, port))
		m_bIsConnect = true;

	return m_bIsConnect;
}

void LogManager::Query(const char* c_pszFormat, ...)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);

	char szQuery[4096];
	va_list args;

	va_start(args, c_pszFormat);
	vsnprintf(szQuery, sizeof(szQuery), c_pszFormat, args);
	va_end(args);

	if (test_server)
		sys_log(0, "LOG: %s", szQuery);

	m_sql.AsyncQuery(szQuery);
}

bool LogManager::IsConnected()
{
	return m_bIsConnect;
}

size_t LogManager::EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize)
{
	return m_sql.EscapeString(dst, dstSize, src, srcSize);
}

void LogManager::ItemLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dwItemID, const char* c_pszText, const char* c_pszHint, const char* c_pszIP, uint32_t dwVnum)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHint, strlen(c_pszHint));

	Query("INSERT DELAYED INTO log%s (type, time, who, x, y, what, how, hint, ip, vnum) VALUES('ITEM', NOW(), %u, %u, %u, %u, '%s', '%s', '%s', %u)",
		get_table_postfix(), dwPID, x, y, dwItemID, c_pszText, __escape_hint, c_pszIP, dwVnum);
}

void LogManager::ItemLog(LPCHARACTER ch, LPITEM item, const char* c_pszText, const char* c_pszHint)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	if (nullptr == ch || nullptr == item)
	{
		sys_err("character or item nil (ch %p item %p text %s)", get_pointer(ch), get_pointer(item), c_pszText);
		return;
	}

	ItemLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), item->GetID(),
		nullptr == c_pszText ? "" : c_pszText,
		c_pszHint, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "",
		item->GetOriginalVnum());
}

void LogManager::ItemLog(LPCHARACTER ch, int itemID, int itemVnum, const char* c_pszText, const char* c_pszHint)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	ItemLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), itemID, c_pszText, c_pszHint, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "", itemVnum);
}

void LogManager::CharLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dwValue, const char* c_pszText, const char* c_pszHint, const char* c_pszIP)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHint, strlen(c_pszHint));

	Query("INSERT DELAYED INTO log%s (type, time, who, x, y, what, how, hint, ip) VALUES('CHARACTER', NOW(), %u, %u, %u, %u, '%s', '%s', '%s')",
		get_table_postfix(), dwPID, x, y, dwValue, c_pszText, __escape_hint, c_pszIP);
}

void LogManager::CharLog(LPCHARACTER ch, uint32_t dw, const char* c_pszText, const char* c_pszHint)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	if (ch)
		CharLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), dw, c_pszText, c_pszHint, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
	else
		CharLog(0, 0, 0, dw, c_pszText, c_pszHint, "");
}

void LogManager::LoginLog(bool isLogin, uint32_t dwAccountID, uint32_t dwPID, uint8_t bLevel, uint8_t bJob, uint32_t dwPlayTime)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	Query("INSERT DELAYED INTO loginlog%s (type, time, channel, account_id, pid, level, job, playtime) VALUES (%s, NOW(), %d, %u, %u, %d, %d, %u)",
		get_table_postfix(), isLogin ? "'LOGIN'" : "'LOGOUT'", g_bChannel, dwAccountID, dwPID, bLevel, bJob, dwPlayTime);
}

void LogManager::MoneyLog(uint8_t type, uint32_t vnum, int gold)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MAX);
	if (type == MONEY_LOG_RESERVED || type >= MONEY_LOG_TYPE_MAX_NUM)
	{
		sys_err("TYPE ERROR: type %d vnum %u gold %d", type, vnum, gold);
		return;
	}

	Query("INSERT DELAYED INTO money_log%s VALUES (NOW(), %d, %d, %d)", get_table_postfix(), type, vnum, gold);
}

void LogManager::HackLog(const char* c_pszHackName, const char* c_pszLogin, const char* c_pszName, const char* c_pszIP)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), c_pszHackName, strlen(c_pszHackName));

	Query("INSERT INTO hack_log (time, login, name, ip, server, why) VALUES(NOW(), '%s', '%s', '%s', '%s', '%s')", c_pszLogin, c_pszName, c_pszIP, g_stHostname.c_str(), __escape_hint);
}

void LogManager::HackLog(const char* c_pszHackName, LPCHARACTER ch)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	if (ch->GetDesc())
	{
		HackLog(c_pszHackName,
			ch->GetDesc()->GetAccountTable().login,
			ch->GetName(),
			ch->GetDesc()->GetHostName());
	}
}

void LogManager::HackCRCLog(const char* c_pszHackName, const char* c_pszLogin, const char* c_pszName, const char* c_pszIP, uint32_t dwCRC)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	Query("INSERT INTO hack_crc_log (time, login, name, ip, server, why, crc) VALUES(NOW(), '%s', '%s', '%s', '%s', '%s', %u)", c_pszLogin, c_pszName, c_pszIP, g_stHostname.c_str(), c_pszHackName, dwCRC);
}

void LogManager::GoldBarLog(uint32_t dwPID, uint32_t dwItemID, GOLDBAR_HOW eHow, const char* c_pszHint)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	char szHow[32 + 1];

	switch (eHow)
	{
		case PERSONAL_SHOP_BUY:
			snprintf(szHow, sizeof(szHow), "'BUY'");
			break;

		case PERSONAL_SHOP_SELL:
			snprintf(szHow, sizeof(szHow), "'SELL'");
			break;

		case SHOP_BUY:
			snprintf(szHow, sizeof(szHow), "'SHOP_BUY'");
			break;

		case SHOP_SELL:
			snprintf(szHow, sizeof(szHow), "'SHOP_SELL'");
			break;

		case EXCHANGE_TAKE:
			snprintf(szHow, sizeof(szHow), "'EXCHANGE_TAKE'");
			break;

		case EXCHANGE_GIVE:
			snprintf(szHow, sizeof(szHow), "'EXCHANGE_GIVE'");
			break;

		case QUEST:
			snprintf(szHow, sizeof(szHow), "'QUEST'");
			break;

		default:
			snprintf(szHow, sizeof(szHow), "''");
			break;
	}

	Query("INSERT DELAYED INTO goldlog%s (date, time, pid, what, how, hint) VALUES(CURDATE(), CURTIME(), %u, %u, %s, '%s')",
		get_table_postfix(), dwPID, dwItemID, szHow, c_pszHint);
}

void LogManager::CubeLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum, uint32_t item_uid, int item_count, bool success)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	Query("INSERT DELAYED INTO cube%s (pid, time, x, y, item_vnum, item_uid, item_count, success) "
		"VALUES(%u, NOW(), %u, %u, %u, %u, %d, %d)",
		get_table_postfix(), dwPID, x, y, item_vnum, item_uid, item_count, success ? 1 : 0);
}

void LogManager::SpeedHackLog(uint32_t pid, uint32_t x, uint32_t y, int hack_count)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	Query("INSERT INTO speed_hack%s (pid, time, x, y, hack_count) "
		"VALUES(%u, NOW(), %u, %u, %d)",
		get_table_postfix(), pid, x, y, hack_count);
}

void LogManager::ChangeNameLog(uint32_t pid, const char* old_name, const char* new_name, const char* ip)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	Query("INSERT DELAYED INTO change_name%s (pid, old_name, new_name, time, ip) "
		"VALUES(%u, '%s', '%s', NOW(), '%s') ",
		get_table_postfix(), pid, old_name, new_name, ip);
}

void LogManager::GMCommandLog(uint32_t dwPID, const char* szName, const char* szIP, uint8_t byChannel, const char* szCommand)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), szCommand, strlen(szCommand));

	Query("INSERT DELAYED INTO command_log%s (userid, server, ip, port, username, command, date ) "
		"VALUES(%u, 999, '%s', %u, '%s', '%s', NOW()) ",
		get_table_postfix(), dwPID, szIP, byChannel, szName, __escape_hint);
}

void LogManager::RefineLog(uint32_t pid, const char* item_name, uint32_t item_id, int item_refine_level, int is_success, const char* how)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), item_name, strlen(item_name));

	Query("INSERT INTO refinelog%s (pid, item_name, item_id, step, time, is_success, setType) VALUES(%u, '%s', %u, %d, NOW(), %d, '%s')",
		get_table_postfix(), pid, __escape_hint, item_id, item_refine_level, is_success, how);
}


void LogManager::ShoutLog(uint8_t bChannel, uint8_t bEmpire, const char* pszText)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), pszText, strlen(pszText));

	Query("INSERT INTO shout_log%s VALUES(NOW(), %d, %d,'%s')", get_table_postfix(), bChannel, bEmpire, __escape_hint);
}

void LogManager::LevelLog(LPCHARACTER pChar, uint32_t level, uint32_t playhour)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	uint32_t aid = 0;

	if (nullptr != pChar->GetDesc())
	{
		aid = pChar->GetDesc()->GetAccountTable().id;
	}

	Query("REPLACE INTO levellog%s (name, level, time, account_id, pid, playtime) VALUES('%s', %u, NOW(), %u, %u, %d)",
		get_table_postfix(), pChar->GetName(), level, aid, pChar->GetPlayerID(), playhour);
}

void LogManager::BootLog(const char* c_pszHostName, uint8_t bChannel)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MIN);
	Query("INSERT INTO bootlog (time, hostname, channel) VALUES(NOW(), '%s', %d)",
		c_pszHostName, bChannel);
}

void LogManager::FishLog(uint32_t dwPID, int prob_idx, int fish_id, int fish_level, uint32_t dwMiliseconds, uint32_t dwVnum, uint32_t dwValue)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MAX);
	Query("INSERT INTO fish_log%s VALUES(NOW(), %u, %d, %u, %d, %u, %u, %u)",
		get_table_postfix(),
		dwPID,
		prob_idx,
		fish_id,
		fish_level,
		dwMiliseconds,
		dwVnum,
		dwValue);
}

void LogManager::QuestRewardLog(const char* c_pszQuestName, uint32_t dwPID, uint32_t dwLevel, int iValue1, int iValue2)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MAX);
	Query("INSERT INTO quest_reward_log%s VALUES('%s',%u,%u,2,%u,%u,NOW())",
		get_table_postfix(),
		c_pszQuestName,
		dwPID,
		dwLevel,
		iValue1,
		iValue2);
}

void LogManager::DetailLoginLog(bool isLogin, LPCHARACTER ch)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MID);
	if (nullptr == ch->GetDesc())
		return;

	if (true == isLogin)
	{
		Query("INSERT INTO loginlog2(type, is_gm, login_time, channel, account_id, pid, ip, client_version) "
			"VALUES('INVALID', %s, NOW(), %d, %u, %u, inet_aton('%s'), '%s')",
			ch->IsGM() == true ? "'Y'" : "'N'",
			g_bChannel,
			ch->GetDesc()->GetAccountTable().id,
			ch->GetPlayerID(),
			ch->GetDesc()->GetHostName(),
			ch->GetDesc()->GetClientVersion());
	}
	else
	{
		Query("SET @i = (SELECT MAX(id) FROM loginlog2 WHERE account_id=%u AND pid=%u)",
			ch->GetDesc()->GetAccountTable().id,
			ch->GetPlayerID());

		Query("UPDATE loginlog2 SET type='VALID', logout_time=NOW(), playtime=TIMEDIFF(logout_time,login_time) WHERE id=@i");
	}
}

void LogManager::DragonSlayLog(uint32_t dwGuildID, uint32_t dwDragonVnum, uint32_t dwStartTime, uint32_t dwEndTime)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MAX);
	Query("INSERT INTO dragon_slay_log%s VALUES( %d, %d, FROM_UNIXTIME(%d), FROM_UNIXTIME(%d) )",
		get_table_postfix(),
		dwGuildID, dwDragonVnum, dwStartTime, dwEndTime);
}

void LogManager::HackShieldLog(unsigned long ErrorCode, LPCHARACTER ch)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MAX);
	struct in_addr st_addr;

#ifndef __WIN32__
	if (0 == inet_aton(ch->GetDesc()->GetHostName(), &st_addr))
#else
	unsigned long in_address;
	in_address = inet_addr(ch->GetDesc()->GetHostName());
	st_addr.s_addr = in_address;
	if (INADDR_NONE == in_address)
#endif
	{
		Query( "INSERT INTO hackshield_log(time, account_id, login, pid, name, reason, ip) "
				"VALUES(NOW(), %u, '%s', %u, '%s', %u, 0)",
				ch->GetDesc()->GetAccountTable().id, ch->GetDesc()->GetAccountTable().login,
				ch->GetPlayerID(), ch->GetName(),
				ErrorCode);
	}
	else
	{
		Query( "INSERT INTO hackshield_log(time, account_id, login, pid, name, reason, ip) "
				"VALUES(NOW(), %u, '%s', %u, '%s', %u, inet_aton('%s'))",
				ch->GetDesc()->GetAccountTable().id, ch->GetDesc()->GetAccountTable().login,
				ch->GetPlayerID(), ch->GetName(),
				ErrorCode,
				ch->GetDesc()->GetHostName());
	}
}

void LogManager::ChatLog(uint32_t where, uint32_t who_id, const char* who_name, uint32_t whom_id, const char* whom_name, const char* type, const char* msg, const char* ip)
{
	Query("INSERT DELAYED INTO `chat_log%s` (`where`, `who_id`, `who_name`, `whom_id`, `whom_name`, `type`, `msg`, `when`, `ip`) "
		"VALUES (%u, %u, '%s', %u, '%s', '%s', '%s', NOW(), '%s');",
		get_table_postfix(),
		where, who_id, who_name, whom_id, whom_name, type, msg, ip);
}

void LogManager::InvalidServerLog(enum eLocalization eLocaleType, const char* pcszIP, const char* pszRevision)
{
	LOG_LEVEL_CHECK_N_RET(LOG_LEVEL_MAX);
	// revision sql inject exploit fix
	m_sql.EscapeString(__escape_hint, sizeof(__escape_hint), pszRevision, strlen(pszRevision));
	// @fixme101 last %s to '%s'
	Query("INSERT INTO invalid_server_log(locale_type, log_date, ip, revision) VALUES(%d, NOW(), '%s', '%s')", eLocaleType, pcszIP, __escape_hint);
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
void LogManager::AcceLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum, uint32_t item_uid, int item_count, int abs_chance, bool success)
{
	Query("INSERT DELAYED INTO acce%s (pid, time, x, y, item_vnum, item_uid, item_count, item_abs_chance, success) VALUES(%u, NOW(), %u, %u, %u, %u, %d, %d, %d)", get_table_postfix(), dwPID, x, y, item_vnum, item_uid, item_count, abs_chance, success ? 1 : 0);
}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
void LogManager::MoveCostumeAttrLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum)
{
	Query("INSERT DELAYED INTO move_costume_attr%s (pid, time, x, y, item_vnum) VALUES(%u, NOW(), %u, %u, %u)", get_table_postfix(), dwPID, x, y, item_vnum);
}
#endif

#ifdef ENABLE_DESTROY_SYSTEM
void LogManager::ItemDestroyLog(LPCHARACTER ch, LPITEM item)
{
	Query("INSERT INTO destroy_log" \
		"(`owner`,`old_item_id`, `vnum`, count, socket0, socket1, socket2," \
		" socket3, socket4, socket5," \
		" attrtype0, attrtype1, attrtype2, attrtype3, attrtype4, attrtype5, attrtype6," \
		" attrvalue0, attrvalue1, attrvalue2, attrvalue3, attrvalue4, attrvalue5, attrvalue6,"
		" date)" \
		"VALUES ("\
		"%lu, %lu,%lu,%lu,"\
		"%ld,%ld,%ld,"\
		"%ld,%ld,%ld,"\
		"%d,%d,%d,%d,"\
		"%d,%d,%d,%d,%d,"\
		"%d,%d,%d,%d,%d,"\
		"NOW())"
		, item->GetLastOwnerPID(), item->GetID(), item->GetVnum(), item->GetCount()
		, item->GetSocket(0), item->GetSocket(1), item->GetSocket(2)
		, item->GetSocket(3), item->GetSocket(4), item->GetSocket(5)
		, item->GetAttributeType(0), item->GetAttributeType(1), item->GetAttributeType(2), item->GetAttributeType(3)
		, item->GetAttributeType(4), item->GetAttributeType(5), item->GetAttributeType(6), item->GetAttributeValue(0), item->GetAttributeValue(1)
		, item->GetAttributeValue(2), item->GetAttributeValue(3), item->GetAttributeValue(4), item->GetAttributeValue(5), item->GetAttributeValue(6)
	);
}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#include "db.h"
void LogManager::MeleyGuildLog(uint32_t dwGuildID, const char* szParticipantsList, uint32_t dwParticipantsCount, uint32_t dwTime)
{
	auto pMsg(DBManager::Instance().DirectQuery("SELECT participants_count, time FROM log.meley_lair_guild%s WHERE guild_id=%u;", get_table_postfix(), dwGuildID));
	if (pMsg->Get()->uiNumRows == 0)
		Query("INSERT INTO meley_lair_guild%s (guild_id, participants, participants_count, time, date) VALUES(%u, '%s', %u, %u, NOW())", get_table_postfix(), dwGuildID, szParticipantsList, dwParticipantsCount, dwTime);
	else
	{
		uint32_t dwParticipantsR = 0;
		uint32_t dwTimeR = 0;
		MYSQL_ROW mRow;
		while (nullptr != (mRow = mysql_fetch_row(pMsg->Get()->pSQLResult)))
		{
			int iCur = 0;
			str_to_number(dwParticipantsR, mRow[iCur++]);
			str_to_number(dwTimeR, mRow[iCur]);
		}

		if ((dwTimeR == dwTime) && (dwParticipantsR < dwParticipantsCount))
			Query("UPDATE meley_lair_guild%s SET participants='%s', participants_count=%u, time=%u, date=NOW() WHERE guild_id=%u;", get_table_postfix(), szParticipantsList, dwParticipantsCount, dwTime, dwGuildID);
		else if (dwTimeR > dwTime)
			Query("UPDATE meley_lair_guild%s SET participants='%s', participants_count=%u, time=%u, date=NOW() WHERE guild_id=%u;", get_table_postfix(), szParticipantsList, dwParticipantsCount, dwTime, dwGuildID);
	}
}

void LogManager::MeleyPartyLog(const char* szLeaderName, const char* szParticipantsList, uint32_t dwParticipantsCount, uint32_t dwTime)
{
	Query("INSERT INTO meley_lair_party%s (master, participants, participants_count, time, date) VALUES('%s', '%s', %u, %u, NOW())",
		get_table_postfix(), szLeaderName, szParticipantsList, dwParticipantsCount, dwTime);
}
#endif

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_LOG)
void LogManager::GuildLog(LPCHARACTER ch, int guildID, uint32_t vnum, const char* typeName, int iteminout, int count, uint32_t datatype)
{
	//ch
	//guildID	= GildenID
	//vnum		= ItemVnum
	//typeName	= "Yang" / "Object" / (names)
	//iteminout	= 0=In / 1=Out
	//count		= Anzahl
	//stage		= Datentyp (Yang = 1 / Items = 0 / Objecte = )

	if (!ch)
		return;

	Query("INSERT INTO guildstorage%s (guild_id, time, playerid, chrname, itemvnum, itemname, iteminout, itemcount, datatype) VALUES(%u, NOW(), %u, '%s', %d, '%s', %d, %u, %d)",
		get_table_postfix(), guildID, ch->GetPlayerID(), ch->GetName(), vnum, typeName, iteminout, count, datatype);
}
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
void LogManager::SoulRouletteLog(const char* table, const char* Name, const int vnum, const int count, const bool state)
{
	Query("INSERT INTO %s%s (name, vnum, count, state, date) VALUES('%s', '%d', '%d', '%s', NOW())", table, get_table_postfix(), Name, vnum, count, (state ? "OK" : "ERROR"));
}
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
void LogManager::OkayEventLog(int dwPID, const char* c_pszText, int points)
{
	Query("INSERT INTO okay_event%s (pid, name, points) VALUES(%d, '%s', %d)", get_table_postfix(), dwPID, c_pszText, points);
}
#endif

#ifdef ENABLE_FISHING_RENEWAL
void LogManager::FishNewLog(uint32_t pid, uint32_t vnum, uint32_t success)
{
	Query("INSERT INTO fish_new_log%s VALUES(NOW(), %u, %u, %u)", get_table_postfix(), pid, vnum, success);
}
#endif
