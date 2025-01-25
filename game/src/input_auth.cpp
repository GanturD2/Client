#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "protocol.h"
#include "locale_service.h"
#include "db.h"
#include "utils.h"

bool FN_IS_VALID_LOGIN_STRING(const char* str)
{
	const char* tmp;

	if (!str || !*str)
		return false;

	if (strlen(str) < 2)
		return false;

	for (tmp = str; *tmp; ++tmp)
	{
		// Only alphanumeric characters allowed
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;

#ifdef ENABLE_ACCOUNT_W_SPECIALCHARS
		// Canada allows some special characters
		switch (*tmp)
		{
			case ' ':
			case '_':
			case '-':
			case '.':
			case '!':
			case '@':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '*':
			case '(':
			case ')':
				continue;
		}
#endif
		return false;
	}

	return true;
}

bool Login_IsInChannelService(const char* c_login)
{
	if (c_login[0] == '[')
		return true;
	return false;
}

CInputAuth::CInputAuth()
{
}

void CInputAuth::Login(LPDESC d, const char* c_pData)
{
	TPacketCGLogin3* pinfo = (TPacketCGLogin3*)c_pData;

	if (!g_bAuthServer)
	{
		sys_err("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return;
	}

	// copy for string integrity
	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	char passwd[PASSWD_MAX_LEN + 1];
	strlcpy(passwd, pinfo->passwd, sizeof(passwd));

#ifdef ENABLE_HWID_BAN
	char hwid[HWID_MAX_LEN + 1];
	strlcpy(hwid, pinfo->hwid, sizeof(hwid));
#endif

	sys_log(0, "InputAuth::Login : %s(%d) desc %p",
		login, strlen(login), get_pointer(d));

	// check login string
	if (false == FN_IS_VALID_LOGIN_STRING(login))
	{
		sys_log(0, "InputAuth::Login : IS_NOT_VALID_LOGIN_STRING(%s) desc %p",
			login, get_pointer(d));
		LoginFailure(d, "NOID");
		return;
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket{};

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));

		d->Packet(&failurePacket, sizeof(failurePacket));
		return;
	}

	if (DESC_MANAGER::Instance().FindByLoginName(login))
	{
		LoginFailure(d, "ALREADY");
		return;
	}

	uint32_t dwKey = DESC_MANAGER::Instance().CreateLoginKey(d);
	uint32_t dwPanamaKey = dwKey ^ pinfo->adwClientKey[0] ^ pinfo->adwClientKey[1] ^ pinfo->adwClientKey[2] ^ pinfo->adwClientKey[3];
	d->SetPanamaKey(dwPanamaKey);

	sys_log(0, "InputAuth::Login : key %u:0x%x login %s", dwKey, dwPanamaKey, login);

	TPacketCGLogin3* p = M2_NEW TPacketCGLogin3;
	thecore_memcpy(p, pinfo, sizeof(TPacketCGLogin3));

	char szPasswd[PASSWD_MAX_LEN * 2 + 1];
	DBManager::Instance().EscapeString(szPasswd, sizeof(szPasswd), passwd, strlen(passwd));

	char szLogin[LOGIN_MAX_LEN * 2 + 1];
	DBManager::Instance().EscapeString(szLogin, sizeof(szLogin), login, strlen(login));

#ifdef ENABLE_HWID_BAN
	char szHWID[HWID_MAX_LEN * 2 + 1];
	DBManager::Instance().EscapeString(szHWID, sizeof(szHWID), hwid, strlen(hwid));

#ifdef ENABLE_HWID_BAN_EXTENDED
	auto pMsg2(DBManager::Instance().DirectQuery("SELECT hwid FROM account.account WHERE login = '%s' LIMIT 1", szLogin));
	if (pMsg2->Get()->uiNumRows == 0)
	{
		LoginFailure(d, "WRONGPWD");
		return;
	}
	MYSQL_ROW row2 = mysql_fetch_row(pMsg2->Get()->pSQLResult);
	if (strcmp(row2[0], "") != 0)
	{
		if (strcmp(row2[0], szHWID) != 0)
		{
			LoginFailure(d, "WRONGHWID");
			return;
		}
	}
#endif
#endif

	// CHANNEL_SERVICE_LOGIN
	if (Login_IsInChannelService(szLogin))
	{
		sys_log(0, "ChannelServiceLogin [%s]", szLogin);

		DBManager::Instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
#ifdef ENABLE_HWID_BAN
			"SELECT '%s',password,IFNULL(hwid_ban.hwid, '') as hwid_check, '%s' as hwid,"
			"account.social_id,account.id,account.status,"
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"account.lang+0,"
#endif
#else
			"SELECT '%s',password,social_id,id,status,"
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"lang+0,"
#endif
#endif
			"availDt - NOW() > 0,"
			"UNIX_TIMESTAMP(silver_expire),"
			"UNIX_TIMESTAMP(gold_expire),"
			"UNIX_TIMESTAMP(safebox_expire),"
			"UNIX_TIMESTAMP(autoloot_expire),"
			"UNIX_TIMESTAMP(fish_mind_expire),"
			"UNIX_TIMESTAMP(marriage_fast_expire),"
			"UNIX_TIMESTAMP(money_drop_rate_expire),"
//#ifdef ENABLE_AUTO_SYSTEM
			"UNIX_TIMESTAMP(auto_use),"
//#endif
//#ifdef ENABLE_YOHARA_SYSTEM
			"UNIX_TIMESTAMP(sungma_expire),"
//#endif
//#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			"UNIX_TIMESTAMP(private_shop_expire),"
//#endif
			"UNIX_TIMESTAMP(create_time)"
#ifdef ENABLE_HWID_BAN
			" FROM account"
			" LEFT JOIN hwid_ban ON hwid_ban.hwid = '%s'"
			" WHERE account.login='%s'",
			szPasswd, szHWID, szHWID, szLogin);
#else
			" FROM account WHERE login='%s'",
			szPasswd, szLogin);
#endif
	}
	// END_OF_CHANNEL_SERVICE_LOGIN
	else
	{
		// @fixme138 1. PASSWORD('%s') -> %s 2. szPasswd wrapped inside mysql_hash_password(%s).c_str()
#ifdef __WIN32__
		DBManager::Instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
#	ifdef ENABLE_HWID_BAN
			"SELECT PASSWORD('%s'),password,IFNULL(hwid_ban.hwid, '') as hwid_check, '%s' as hwid,"
			"account.social_id,account.id,account.status,"
#	ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"account.lang+0,"
#	endif
#	else
			"SELECT PASSWORD('%s'),password,social_id,id,status,"
#	ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"lang+0,"
#	endif
#	endif
			"availDt - NOW() > 0,"
			"UNIX_TIMESTAMP(silver_expire),"
			"UNIX_TIMESTAMP(gold_expire),"
			"UNIX_TIMESTAMP(safebox_expire),"
			"UNIX_TIMESTAMP(autoloot_expire),"
			"UNIX_TIMESTAMP(fish_mind_expire),"
			"UNIX_TIMESTAMP(marriage_fast_expire),"
			"UNIX_TIMESTAMP(money_drop_rate_expire),"
//#	ifdef ENABLE_AUTO_SYSTEM
			"UNIX_TIMESTAMP(auto_use),"
//#	endif
//#	ifdef ENABLE_YOHARA_SYSTEM
			"UNIX_TIMESTAMP(sungma_expire),"
//#	endif
//#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			"UNIX_TIMESTAMP(private_shop_expire),"
//#	endif
			"UNIX_TIMESTAMP(create_time)"
#	ifdef ENABLE_HWID_BAN
			" FROM account"
			" LEFT JOIN hwid_ban ON hwid_ban.hwid = '%s'"
			" WHERE account.login='%s'",
			szPasswd, szHWID, szHWID, szLogin);
#	else
			" FROM account WHERE login='%s'", szPasswd, szLogin);
#	endif
#else
		// @fixme138 1. PASSWORD('%s') -> %s 2. szPasswd wrapped inside mysql_hash_password(%s).c_str()
		DBManager::Instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
#	ifdef ENABLE_HWID_BAN
			"SELECT '%s',account.password,IFNULL(hwid_ban.hwid, '') as hwid_check, '%s' as hwid,"
			"account.social_id,account.id,account.status,"
#	ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"account.lang+0,"
#	endif
#	else
			"SELECT '%s',password,social_id,id,status,"
#	ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"lang+0,"
#	endif
#	endif
			"availDt - NOW() > 0,"
			"UNIX_TIMESTAMP(silver_expire),"
			"UNIX_TIMESTAMP(gold_expire),"
			"UNIX_TIMESTAMP(safebox_expire),"
			"UNIX_TIMESTAMP(autoloot_expire),"
			"UNIX_TIMESTAMP(fish_mind_expire),"
			"UNIX_TIMESTAMP(marriage_fast_expire),"
			"UNIX_TIMESTAMP(money_drop_rate_expire),"
//#	ifdef ENABLE_AUTO_SYSTEM
			"UNIX_TIMESTAMP(auto_use),"
//#	endif
//#	ifdef ENABLE_YOHARA_SYSTEM
			"UNIX_TIMESTAMP(sungma_expire),"
//#	endif
//#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			"UNIX_TIMESTAMP(private_shop_expire),"
//#	endif
			"UNIX_TIMESTAMP(create_time)"
#	ifdef ENABLE_HWID_BAN
			" FROM account"
			" LEFT JOIN hwid_ban ON hwid_ban.hwid = '%s'"
			" WHERE account.login='%s'",
			mysql_hash_password(szPasswd).c_str(), szHWID, szHWID, szLogin);
#	else
			" FROM account WHERE login='%s'",
			mysql_hash_password(szPasswd).c_str(), szLogin);
#	endif
#endif
	}
}

extern void socket_timeout(socket_t s, long sec, long usec);

int CInputAuth::Analyze(LPDESC d, uint8_t bHeader, const char* c_pData)
{

	if (!g_bAuthServer)
	{
		sys_err("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return 0;
	}

	int iExtraLen = 0;

	if (test_server)
		sys_log(0, " InputAuth Analyze Header[%d] ", bHeader);

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_LOGIN3:
			Login(d, c_pData);
			break;

		case HEADER_CG_HANDSHAKE:
			break;

		default:
			sys_err("This phase does not handle this header %d (0x%x)(phase: AUTH)", bHeader, bHeader);
			break;
	}

	return iExtraLen;
}
