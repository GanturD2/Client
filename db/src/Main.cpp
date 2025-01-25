#include "stdafx.h"
#include "Config.h"
#include "Peer.h"
#include "DBManager.h"
#include "ClientManager.h"
#include "GuildManager.h"
#include "ItemAwardManager.h"
#include "HB.h"
#include "PrivManager.h"
#include "MoneyLog.h"
#include "Marriage.h"
#include "Monarch.h"
#include "BlockCountry.h"
#include "ItemIDRangeManager.h"
#include <signal.h>
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
# include "LottoManager.h"
#endif
#undef __FreeBSD__

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#include "QID.h"
int g_iShopCacheFlushSeconds = 360;
#endif

void SetPlayerDBName(const char* c_pszPlayerDBName);
void SetTablePostfix(const char* c_pszTablePostfix);
int Start();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void PollShops();
#endif

std::string g_stTablePostfix;
std::string g_stLocaleNameColumn = "name";
std::string g_stLocale = "latin1"; // default: euckr
std::string g_stPlayerDBName = "";

bool g_bHotBackup = false; // default: true
BOOL g_test_server = false;

//단위 초
int g_iPlayerCacheFlushSeconds = 60*7;
int g_iItemCacheFlushSeconds = 60*5;
#ifdef ENABLE_SKILL_COLOR_SYSTEM
int g_iSkillColorCacheFlushSeconds = 60 * 7;
#endif

//g_iLogoutSeconds 수치는 g_iPlayerCacheFlushSeconds 와 g_iItemCacheFlushSeconds 보다 길어야 한다.
int g_iLogoutSeconds = 60*10;

int g_log = 1;


// MYSHOP_PRICE_LIST
int g_iItemPriceListTableCacheFlushSeconds = 540;
// END_OF_MYSHOP_PRICE_LIST

#if defined(__FreeBSD__) && defined(__FreeBSD_version) && __FreeBSD_version<1000000
extern const char * _malloc_options;
#endif

extern void WriteVersion();

void emergency_sig(int sig)
{
	if (sig == SIGSEGV)
		sys_log(0, "SIGNAL: SIGSEGV");
	else if (sig == SIGUSR1)
		sys_log(0, "SIGNAL: SIGUSR1");

	if (sig == SIGSEGV)
		abort();
}

int main()
{
	WriteVersion();

#if defined(__FreeBSD__) && defined(__FreeBSD_version) && __FreeBSD_version<1000000
	_malloc_options = "A";
#endif

	CConfig Config;
	CNetPoller poller;
	CDBManager DBManager;
	CClientManager ClientManager;
	PlayerHB player_hb;
	CGuildManager GuildManager;
	CPrivManager PrivManager;
	CMoneyLog MoneyLog;
	ItemAwardManager ItemAwardManager;
	marriage::CManager MarriageManager;
	CMonarch Monarch;
	CBlockCountry	BlockCountry;
	CItemIDRangeManager ItemIDRangeManager;
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	CLottoManager LottoManager;
#endif

	if (!Start())
		return 1;

	GuildManager.Initialize();
	MarriageManager.Initialize();
	BlockCountry.Load();
	ItemIDRangeManager.Build();
	sys_log(0, "Metin2DBCacheServer Start\n");

	CClientManager::Instance().MainLoop();

	signal_timer_disable();

	//DBManager.Quit();	//@fixme475
	int iCount;

	while (1)
	{
		iCount = 0;

		iCount += CDBManager::Instance().CountReturnQuery(SQL_PLAYER);
		iCount += CDBManager::Instance().CountAsyncQuery(SQL_PLAYER);

		if (iCount == 0)
			break;

		usleep(1000);
		sys_log(0, "WAITING_QUERY_COUNT %d", iCount);
	}

	return 1;
}

void emptybeat(LPHEART heart, int pulse)
{
	if (!(pulse % heart->passes_per_sec))	// 1초에 한번
	{
	}
}

//
// @version	05/06/13 Bang2ni - 아이템 가격정보 캐시 flush timeout 설정 추가.
//
int Start()
{
	if (!CConfig::Instance().LoadFile("conf.txt"))
	{
		fprintf(stderr, "Loading conf.txt failed.\n");
		return false;
	}

	if (!CConfig::Instance().GetValue("TEST_SERVER", &g_test_server))
	{
		fprintf(stderr, "Real Server\n");
	}
	else
		fprintf(stderr, "Test Server\n");

	if (!CConfig::Instance().GetValue("LOG", &g_log))
	{
		fprintf(stderr, "Log Off");
		g_log= 0;
	}
	else
	{
		g_log = 1;
		fprintf(stderr, "Log On");
	}


	int tmpValue;

	int heart_beat = 50;
	if (!CConfig::Instance().GetValue("CLIENT_HEART_FPS", &heart_beat))
	{
		fprintf(stderr, "Cannot find CLIENT_HEART_FPS configuration.\n");
		return false;
	}

	log_set_expiration_days(3);

	if (CConfig::Instance().GetValue("LOG_KEEP_DAYS", &tmpValue))
	{
		tmpValue = MINMAX(3, tmpValue, 30);
		log_set_expiration_days(tmpValue);
		fprintf(stderr, "Setting log keeping days to %d\n", tmpValue);
	}

	thecore_init(heart_beat, emptybeat);
	signal_timer_enable(60);

	char szBuf[256+1];

	if (CConfig::Instance().GetValue("LOCALE", szBuf, 256))
	{
		g_stLocale = szBuf;
		sys_log(0, "LOCALE set to %s", g_stLocale.c_str());

		// CHINA_DISABLE_HOTBACKUP
		if ("gb2312" == g_stLocale)
		{
			sys_log(0, "CIBN_LOCALE: DISABLE_HOTBACKUP");
			g_bHotBackup = false;
		}
		// END_OF_CHINA_DISABLE_HOTBACKUP
	}

	int iDisableHotBackup;
	if (CConfig::Instance().GetValue("DISABLE_HOTBACKUP", &iDisableHotBackup))
	{
		if (iDisableHotBackup)
		{
			sys_log(0, "CONFIG: DISABLE_HOTBACKUP");
			g_bHotBackup = false;
		}
	}


	if (!CConfig::Instance().GetValue("TABLE_POSTFIX", szBuf, 256))
	{
		sys_log(0, "TABLE_POSTFIX not configured use default"); // @warme012
		szBuf[0] = '\0';
	}

	SetTablePostfix(szBuf);

	if (CConfig::Instance().GetValue("PLAYER_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iPlayerCacheFlushSeconds, szBuf);
		sys_log(0, "PLAYER_CACHE_FLUSH_SECONDS: %d", g_iPlayerCacheFlushSeconds);
	}

	if (CConfig::Instance().GetValue("ITEM_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iItemCacheFlushSeconds, szBuf);
		sys_log(0, "ITEM_CACHE_FLUSH_SECONDS: %d", g_iItemCacheFlushSeconds);
	}

	// MYSHOP_PRICE_LIST
	if (CConfig::Instance().GetValue("ITEM_PRICELIST_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iItemPriceListTableCacheFlushSeconds, szBuf);
		sys_log(0, "ITEM_PRICELIST_CACHE_FLUSH_SECONDS: %d", g_iItemPriceListTableCacheFlushSeconds);
	}
	// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (CConfig::Instance().GetValue("SHOP_CACHE_FLUSH_SECONDS", szBuf, 256))
	{
		str_to_number(g_iShopCacheFlushSeconds, szBuf);
		sys_log(0, "SHOP_CACHE_FLUSH_SECONDS: %d", g_iShopCacheFlushSeconds);
	}
#endif

	if (CConfig::Instance().GetValue("CACHE_FLUSH_LIMIT_PER_SECOND", szBuf, 256))
	{
		uint32_t dwVal = 0; str_to_number(dwVal, szBuf);
		CClientManager::Instance().SetCacheFlushCountLimit(dwVal);
	}

	int iIDStart;
	if (!CConfig::Instance().GetValue("PLAYER_ID_START", &iIDStart))
	{
		sys_err("PLAYER_ID_START not configured");
		return false;
	}

	CClientManager::Instance().SetPlayerIDStart(iIDStart);

	if (CConfig::Instance().GetValue("NAME_COLUMN", szBuf, 256))
	{
		fprintf(stderr, "%s %s", g_stLocaleNameColumn.c_str(), szBuf);
		g_stLocaleNameColumn = szBuf;
	}

	char szAddr[64], szDB[64], szUser[64], szPassword[64];
	int iPort;
	char line[256+1];

	if (CConfig::Instance().GetValue("SQL_PLAYER", line, 256))
	{
		sscanf(line, " %s %s %s %s %d ", szAddr, szDB, szUser, szPassword, &iPort);
		sys_log(0, "connecting to MySQL server (player)");

		int iRetry = 5;

		do
		{
			if (CDBManager::Instance().Connect(SQL_PLAYER, szAddr, iPort, szDB, szUser, szPassword))
			{
				sys_log(0, "   OK");
				break;
			}

			sys_log(0, "   failed, retrying in 5 seconds");
			fprintf(stderr, "   failed, retrying in 5 seconds");
			sleep(5);
		} while (iRetry--);
		fprintf(stderr, "Success PLAYER\n");
		SetPlayerDBName(szDB);
	}
	else
	{
		sys_err("SQL_PLAYER not configured");
		return false;
	}

	if (CConfig::Instance().GetValue("SQL_ACCOUNT", line, 256))
	{
		sscanf(line, " %s %s %s %s %d ", szAddr, szDB, szUser, szPassword, &iPort);
		sys_log(0, "connecting to MySQL server (account)");

		int iRetry = 5;

		do
		{
			if (CDBManager::Instance().Connect(SQL_ACCOUNT, szAddr, iPort, szDB, szUser, szPassword))
			{
				sys_log(0, "   OK");
				break;
			}

			sys_log(0, "   failed, retrying in 5 seconds");
			fprintf(stderr, "   failed, retrying in 5 seconds");
			sleep(5);
		} while (iRetry--);
		fprintf(stderr, "Success ACCOUNT\n");
	}
	else
	{
		sys_err("SQL_ACCOUNT not configured");
		return false;
	}

	if (CConfig::Instance().GetValue("SQL_COMMON", line, 256))
	{
		sscanf(line, " %s %s %s %s %d ", szAddr, szDB, szUser, szPassword, &iPort);
		sys_log(0, "connecting to MySQL server (common)");

		int iRetry = 5;

		do
		{
			if (CDBManager::Instance().Connect(SQL_COMMON, szAddr, iPort, szDB, szUser, szPassword))
			{
				sys_log(0, "   OK");
				break;
			}

			sys_log(0, "   failed, retrying in 5 seconds");
			fprintf(stderr, "   failed, retrying in 5 seconds");
			sleep(5);
		} while (iRetry--);
		fprintf(stderr, "Success COMMON\n");
	}
	else
	{
		sys_err("SQL_COMMON not configured");
		return false;
	}

	if (CConfig::Instance().GetValue("SQL_HOTBACKUP", line, 256))
	{
		sscanf(line, " %s %s %s %s %d ", szAddr, szDB, szUser, szPassword, &iPort);
		sys_log(0, "connecting to MySQL server (hotbackup)");

		int iRetry = 5;

		do
		{
			if (CDBManager::Instance().Connect(SQL_HOTBACKUP, szAddr, iPort, szDB, szUser, szPassword))
			{
				sys_log(0, "   OK");
				break;
			}

			sys_log(0, "   failed, retrying in 5 seconds");
			fprintf(stderr, "   failed, retrying in 5 seconds");
			sleep(5);
		}
		while (iRetry--);

		fprintf(stderr, "Success HOTBACKUP\n");
	}
	else
	{
		sys_err("SQL_HOTBACKUP not configured");
		return false;
	}

	if (!CNetPoller::Instance().Create())
	{
		sys_err("Cannot create network poller");
		return false;
	}

	sys_log(0, "ClientManager initialization.. ");

	if (!CClientManager::Instance().Initialize())
	{
		sys_log(0, "   failed");
		return false;
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	sys_log(0, "Starting async shop polling");
	PollShops();
#endif

	sys_log(0, "   OK");

	if (!PlayerHB::Instance().Initialize())
	{
		sys_err("cannot initialize player hotbackup");
		return false;
	}

#ifndef __WIN32__
	signal(SIGUSR1, emergency_sig);
#endif
	signal(SIGSEGV, emergency_sig);
	return true;
}

void SetTablePostfix(const char* c_pszTablePostfix)
{
	if (!c_pszTablePostfix || !*c_pszTablePostfix)
		g_stTablePostfix = "";
	else
		g_stTablePostfix = c_pszTablePostfix;
}

const char * GetTablePostfix()
{
	return g_stTablePostfix.c_str();
}

void SetPlayerDBName(const char* c_pszPlayerDBName)
{
	if (! c_pszPlayerDBName || ! *c_pszPlayerDBName)
		g_stPlayerDBName = "";
	else
	{
		g_stPlayerDBName = c_pszPlayerDBName;
		g_stPlayerDBName += ".";
	}
}

const char * GetPlayerDBName()
{
	return g_stPlayerDBName.c_str();
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void PollShops()
{
	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT s.pid, s.x, s.y, s.channel, s.mapindex, UNIX_TIMESTAMP(s.open_time), s.offline_left"
		", s.sign, p.name"
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		", p.level, p.conqueror_level, p.alignment, pi.empire, COALESCE(g.guild_id, 0)"	//	", p.level, p.alignment, pi.empire, g.guild_id"
#	endif
		", s.gold_stash"
#	ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
		", s.cheque_stash"
#	endif
#	ifdef ENABLE_MYSHOP_DECO_PREMIUM
		", s.poly_vnum, s.title_type, s.is_deco"
#	endif
		", s.is_closed "
		"FROM private_shop AS s INNER JOIN player AS p ON p.id = s.pid"
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		" INNER JOIN player_index AS pi ON pi.id = p.account_id "
		"LEFT JOIN guild_member AS g ON g.pid = s.pid;"		//	"INNER JOIN guild_member AS g ON g.pid = p.id;"
#	endif
	);

	CDBManager::Instance().ReturnQuery(szQuery, QID_LOAD_PLAYER_SHOPS, 0, nullptr);
}
#endif