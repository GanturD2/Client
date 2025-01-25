#include "stdafx.h"
#include <sstream>
#ifndef __WIN32__
#include <ifaddrs.h>
#endif

#include "constants.h"
#include "utils.h"
#include "log.h"
#include "desc.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "p2p.h"
#include "char.h"
#include "ip_ban.h"
#include "war_map.h"
#include "locale_service.h"
#include "config.h"
#include "dev_log.h"
#include "db.h"
#include "skill_power.h"

using std::string;

uint8_t	g_bChannel = 0;
uint16_t	mother_port = 50080;
int		passes_per_sec = 25;
uint16_t	db_port = 0;
uint16_t	p2p_port = 50900;
char	db_addr[ADDRESS_MAX_LEN + 1];
int		save_event_second_cycle = passes_per_sec * 120;
int		ping_event_second_cycle = passes_per_sec * 60;
bool	g_bNoMoreClient = false;
bool	g_bNoRegen = false;
// #ifdef ENABLE_NEWSTUFF
bool	g_bEmpireShopPriceTripleDisable = true;
bool	g_bShoutAddonEnable = false;
bool	g_bGlobalShoutEnable = false;
bool	g_bDisablePrismNeed = false;
bool	g_bDisableEmotionMask = false;
uint8_t	g_bItemCountLimit = ITEM_COUNT_LIMIT;
uint32_t	g_dwItemBonusChangeTime = ITEM_BONUS_CHANGE_TIME;
bool	g_bAllMountAttack = false;
bool	g_bEnableBootaryCheck = false;
bool	g_bGMHostCheck = false;
bool	g_bGuildInviteLimit = false;
bool	g_bGuildInfiniteMembers = false;
bool	g_bChinaIntoxicationCheck = false;
bool	g_bEnableSpeedHackCrash = false;
int		g_iStatusPointGetLevelLimit = STATUSPOINT_GET_LEVELLIMIT;
int		g_iStatusPointSetMaxValue = STATUSPOINT_SET_MAXVALUE;
#ifdef ENABLE_YOHARA_SYSTEM
int		g_iConquerorPointGetLevelLimit = 30;
int		g_iConquerorPointSetMaxValue = 30;
int		gConquerorMaxLevel = CONQUEROR_MAX_LEVEL;

#endif
int		g_iShoutLimitLevel = SHOUT_LIMIT_LEVEL;
// int		g_iShoutLimitTime = 15;
int		g_iDbLogLevel = LOG_LEVEL_MAX;
int		g_iSysLogLevel = LOG_LEVEL_MAX;
int		g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX] = { ITEM_DEST_TIME_AUTOITEM, ITEM_DEST_TIME_DROPITEM, ITEM_DEST_TIME_DROPGOLD }; // autoitem, dropgold, dropitem
bool	g_bDisableEmpireLanguageCheck = false;
uint32_t	g_dwSkillBookNextReadMin = SKILLBOOK_NEXT_READ_MIN;
uint32_t	g_dwSkillBookNextReadMax = SKILLBOOK_NEXT_READ_MAX;
#ifdef ENABLE_PROXY_IP
std::string	g_stProxyIP = "";
#endif
// #endif

// Custom_addons
int g_npcGroupRespawnRange[] = {30, 90}; //min, max. In seconds
bool emotion_without_mask = false;
bool emotion_same_gender = false;
bool g_bBlockPositionsInDuell = false;
bool g_bShoutFrozen = false;

bool attr_always_add = false;	//@custom029
bool attr_always_5_add = false;	//@custom030
bool attr_rare_enable = true;	//@custom026
bool belt_allow_all_items = false;	//@custom009
bool ban_force_reason = true;
bool trade_effect = true;
int trade_effect_exchange_threshold = 500000;
int trade_effect_shop_threshold = 100000000;
bool view_equip_for_all = false;
int view_equip_sp_cost = 0;
bool attribute_double_change = true;
int ride_seconds_to_ride_again = 3;
bool g_bDisableMovspeedHacklog = false;
double g_dMovspeedHackThreshold = 0.75;
bool poison_draw_monsters_aggro = false;	//@fixme435

std::set<int> s_mtr_map_disable;
// END_Custom_addons

// TRAFFIC_PROFILER
bool		g_bTrafficProfileOn = false;
uint32_t		g_dwTrafficProfileFlushCycle = 3600;
// END_OF_TRAFFIC_PROFILER

int			test_server = 0;
int			speed_server = 0;
bool		distribution_test_server = false;
bool		guild_mark_server = true;
uint8_t		guild_mark_min_level = 3;
bool		no_wander = false;
int			g_iUserLimit = 32768;

char		g_szPublicIP[16] = "0";
char		g_szInternalIP[16] = "0";
bool		g_bSkillDisable = false;
int			g_iFullUserCount = 1200;
int			g_iBusyUserCount = 650;
//Canada
//int			g_iFullUserCount = 600;
//int			g_iBusyUserCount = 350;
//Brazil
//int			g_iFullUserCount = 650;
//int			g_iBusyUserCount = 450;
bool		g_bEmpireWhisper = true;
uint8_t		g_bAuthServer = false;

bool		g_bCheckClientVersion = true;
string	g_stClientVersion = "1215955205";

string	g_stAuthMasterIP;
uint16_t		g_wAuthMasterPort = 0;

static std::set<uint32_t> s_set_dwFileCRC;
static std::set<uint32_t> s_set_dwProcessCRC;

string g_stHostname = "";
string g_table_postfix = "";

string g_stQuestDir = "./quest";
//string g_stQuestObjectDir = "./quest/object";
string g_stDefaultQuestObjectDir = "./quest/object";
std::set<string> g_setQuestObjectDir;

std::vector<std::string>	g_stAdminPageIP;
std::string	g_stAdminPagePassword = "SHOWMETHEMONEY";

string g_stBlockDate = "30000705";

extern string g_stLocale;

int SPEEDHACK_LIMIT_COUNT   = 50;
int SPEEDHACK_LIMIT_BONUS   = 80;
int g_iSyncHackLimitCount = 10;

//시야 = VIEW_RANGE + VIEW_BONUS_RANGE
int VIEW_RANGE = 5000;
int VIEW_BONUS_RANGE = 500;

int g_server_id = 0;
#ifdef ENABLE_CMS_ITEMSHOP
#	ifdef __WIN32__
string g_strWebMallURL = "127.0.0.1";
#	else
string g_strWebMallURL = "www.jeexwork.v4y.hu";
#	endif
#else
string g_strWebMallURL = "www.metin2.de";
#endif

unsigned int g_uiSpamBlockDuration = 60 * 15;
unsigned int g_uiSpamBlockScore = 100;
unsigned int g_uiSpamReloadCycle = 60 * 10;

bool		g_bCheckMultiHack = true;

int			g_iSpamBlockMaxLevel = 10;

void		LoadStateUserCount();
void		LoadValidCRCList();
bool		LoadClientVersion();
bool            g_protectNormalPlayer   = false;
bool            g_noticeBattleZone      = false;

int gPlayerMaxLevel = PLAYER_MAX_LEVEL;	//Original: 99

int gShutdownAge = 0;
int gShutdownEnable = 0;

/*
 * NOTE : 핵 체크 On/Off. CheckIn할때 false로 수정했으면 반드시 확인하고 고쳐놓을것!
 * 이걸로 생길수있는 똥은 책임안짐 ~ ity ~
 */
bool gHackCheckEnable = false;

bool g_BlockCharCreation = false;


//OPENID
char	openid_host[256];
char	openid_uri[256];

bool is_string_true(const char * string)
{
	bool	result = 0;
	if (isnhdigit(*string))
	{
		str_to_number(result, string);
		return result > 0 ? true : false;
	}
	else if (LOWER(*string) == 't')
		return true;
	else
		return false;
}

static std::set<int> s_set_map_allows;

bool map_allow_find(int index)
{
	if (g_bAuthServer)
		return false;

	if (s_set_map_allows.find(index) == s_set_map_allows.end())
		return false;

	return true;
}

void map_allow_log()
{
	std::set<int>::iterator i;

	for (i = s_set_map_allows.begin(); i != s_set_map_allows.end(); ++i)
		sys_log(0, "MAP_ALLOW: %d", *i);
}

void map_allow_add(int index)
{
	if (map_allow_find(index) == true)
	{
		fprintf(stdout, "!!! FATAL ERROR !!! multiple MAP_ALLOW setting!!\n");
		exit(1);
	}

	fprintf(stdout, "MAP ALLOW %d\n", index);
	s_set_map_allows.insert(index);
}

void map_allow_copy(long * pl, int size)
{
	int iCount = 0;
	std::set<int>::iterator it = s_set_map_allows.begin();

	while (it != s_set_map_allows.end())
	{
		int i = *(it++);
		*(pl++) = i;

		if (++iCount > size)
			break;
	}
}

bool mtr_map_disable_find(int index)
{

	if (s_mtr_map_disable.find(index) == s_mtr_map_disable.end())
		return false;

	return true;
}
void mtr_map_disable_add(int index)
{
	if (mtr_map_disable_find(index) == true)
	{
		fprintf(stdout, "!!! FATAL ERROR !!! multiple MTR_MAP_DISABLE setting!!\n");
		exit(1);
	}

	fprintf(stdout, "MTR DISABLE %d\n", index);
	s_mtr_map_disable.insert(index);
}

std::set<int> s_closed_empire_reg;

void closed_empire_reg_add(int index)
{
	fprintf(stdout, "Character creation disabled for empire (%d)\n", index);
	s_closed_empire_reg.insert(index);
}

static void FN_add_adminpageIP(char *line)
{
	char	*last;
	const char *delim = " \t\r\n";
	char *v = strtok_r(line, delim, &last);

	while (v)
	{
		g_stAdminPageIP.emplace_back(v);
		v = strtok_r(nullptr, delim, &last);
	}
}

static void FN_log_adminpage()
{
	itertype(g_stAdminPageIP) iter = g_stAdminPageIP.begin();

	while (iter != g_stAdminPageIP.end())
	{
		dev_log(LOG_DEB0, "ADMIN_PAGE_IP = %s", (*iter).c_str());
		++iter;
	}

	dev_log(LOG_DEB0, "ADMIN_PAGE_PASSWORD = %s", g_stAdminPagePassword.c_str());
}

bool GetIPInfo()
{
#ifndef __WIN32__
	struct ifaddrs* ifaddrp = nullptr;

	if (0 != getifaddrs(&ifaddrp))
		return false;

	for( struct ifaddrs* ifap=ifaddrp ; nullptr != ifap ; ifap = ifap->ifa_next )
	{
		struct sockaddr_in * sai = (struct sockaddr_in *) ifap->ifa_addr;

		if (!ifap->ifa_netmask ||  // ignore if no netmask
				sai->sin_addr.s_addr == 0 || // ignore if address is 0.0.0.0
				sai->sin_addr.s_addr == 16777343) // ignore if address is 127.0.0.1
			continue;
#else
	WSADATA wsa_data;
	char host_name[100];
	HOSTENT* host_ent;
	int n = 0;

	if (WSAStartup(0x0101, &wsa_data)) {
		return false;
	}

	gethostname(host_name, sizeof(host_name));
	host_ent = gethostbyname(host_name);
	if (host_ent == nullptr) {
		return false;
	}
	for ( ; host_ent->h_addr_list[n] != nullptr; ++n) {
		struct sockaddr_in addr;
		struct sockaddr_in* sai = &addr;
		memcpy(&sai->sin_addr.s_addr, host_ent->h_addr_list[n], host_ent->h_length);
#endif

		char * netip = inet_ntoa(sai->sin_addr);

		if (!strncmp(netip, "192.168", 7)) // ignore if address is starting with 192
		{
			strlcpy(g_szInternalIP, netip, sizeof(g_szInternalIP));
#ifndef __WIN32__
			fprintf(stderr, "INTERNAL_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "INTERNAL_IP: %s\n", netip);
#endif
		}
		else if (!strncmp(netip, "10.", 3))
		{
			strlcpy(g_szInternalIP, netip, sizeof(g_szInternalIP));
#ifndef __WIN32__
			fprintf(stderr, "INTERNAL_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "INTERNAL_IP: %s\n", netip);
#endif
		}
		else if (g_szPublicIP[0] == '0')
		{
			strlcpy(g_szPublicIP, netip, sizeof(g_szPublicIP));
#ifndef __WIN32__
			fprintf(stderr, "PUBLIC_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "PUBLIC_IP: %s\n", netip);
#endif
		}
	}

#ifndef __WIN32__
	freeifaddrs( ifaddrp );
#else
	WSACleanup();
#endif

	if (g_szPublicIP[0] != '0')
		return true;
	else
	{
#ifdef ENABLE_AUTODETECT_INTERNAL_IP
		if (g_szInternalIP[0] == '0')
			return false;
		else
		{
			strlcpy(g_szPublicIP, g_szInternalIP, sizeof(g_szPublicIP));
			fprintf(stderr, "INTERNAL_IP -> PUBLIC_IP: %s\n", g_szPublicIP);
			return true;
		}
#else
		return false;
#endif
	}
}

static bool __LoadConnectConfigFile(const char* configName)
	{
#ifdef WIN32
	char	buf[256] = { '\0' };
	char	token_string[256] = { '\0' };
	char	value_string[256] = { '\0' };
#else
	char	buf[256];
	char	token_string[256];
	char	value_string[256];
#endif

	char db_host[2][64], db_user[2][64], db_pwd[2][64], db_db[2][64];
	int mysql_db_port[2];

	for (int n = 0; n < 2; ++n)
	{
		*db_host[n]	= '\0';
		*db_user[n] = '\0';
		*db_pwd[n]= '\0';
		*db_db[n]= '\0';
		mysql_db_port[n] = 0;
	}

	char log_host[64], log_user[64], log_pwd[64], log_db[64];
	int log_port = 0;

	*log_host = '\0';
	*log_user = '\0';
	*log_pwd = '\0';
	*log_db = '\0';

	bool isCommonSQL = false;
	bool isPlayerSQL = false;

	FILE* fpOnlyForDB;

	if (!(fpOnlyForDB = fopen(configName, "r")))
	{
		fprintf(stderr, "Can not open [%s]\n", configName);
		exit(1);
	}

	while (fgets(buf, 256, fpOnlyForDB))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("hostname")
		{
			g_stHostname = value_string;
			fprintf(stdout, "HOSTNAME: %s\n", g_stHostname.c_str());
			continue;
		}

		TOKEN("channel")
		{
			str_to_number(g_bChannel, value_string);
			continue;
		}

		TOKEN("player_sql")
		{
			const char * line = two_arguments(value_string, db_host[0], sizeof(db_host[0]), db_user[0], sizeof(db_user[0]));
			line = two_arguments(line, db_pwd[0], sizeof(db_pwd[0]), db_db[0], sizeof(db_db[0]));

			if (line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[0], buf);
			}

			if (!*db_host[0] || !*db_user[0] || !*db_pwd[0] || !*db_db[0])
			{
				fprintf(stderr, "PLAYER_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "PLAYER_SQL: %s %s %s %s %d", db_host[0], db_user[0], db_pwd[0], db_db[0], mysql_db_port[0]);
			isPlayerSQL = true;
			continue;
		}

		TOKEN("common_sql")
		{
			const char * line = two_arguments(value_string, db_host[1], sizeof(db_host[1]), db_user[1], sizeof(db_user[1]));
			line = two_arguments(line, db_pwd[1], sizeof(db_pwd[1]), db_db[1], sizeof(db_db[1]));

			if (line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[1], buf);
			}

			if (!*db_host[1] || !*db_user[1] || !*db_pwd[1] || !*db_db[1])
			{
				fprintf(stderr, "COMMON_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "COMMON_SQL: %s %s %s %s %d", db_host[1], db_user[1], db_pwd[1], db_db[1], mysql_db_port[1]);
			isCommonSQL = true;
			continue;
		}

		TOKEN("log_sql")
		{
			const char * line = two_arguments(value_string, log_host, sizeof(log_host), log_user, sizeof(log_user));
			line = two_arguments(line, log_pwd, sizeof(log_pwd), log_db, sizeof(log_db));

			if (line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(log_port, buf);
			}

			if (!*log_host || !*log_user || !*log_pwd || !*log_db)
			{
				fprintf(stderr, "LOG_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "LOG_SQL: %s %s %s %s %d", log_host, log_user, log_pwd, log_db, log_port);
			continue;
		}
	}

	fclose(fpOnlyForDB);

	// CONFIG_SQL_INFO_ERROR
	if (!isCommonSQL)
	{
		puts("LOAD_COMMON_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("COMMON_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	if (!isPlayerSQL)
	{
		puts("LOAD_PLAYER_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("PLAYER_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	AccountDB::Instance().Connect(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1]);

	if (false == AccountDB::Instance().IsConnected())
	{
		fprintf(stderr, "cannot start server while no common sql connected\n");
		exit(1);
	}

	fprintf(stdout, "CommonSQL connected\n");

	{
		char szQuery[512];
		snprintf(szQuery, sizeof(szQuery), "SELECT mKey, mValue FROM locale");

		auto pMsg(AccountDB::Instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			fprintf(stderr, "COMMON_SQL: DirectQuery failed : %s\n", szQuery);
			exit(1);
		}

		MYSQL_ROW row;

		while (nullptr != (row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
		{
			if (strcasecmp(row[0], "LOCALE") == 0)
			{
				if (LocaleService_Init(row[1]) == false)
				{
					fprintf(stderr, "COMMON_SQL: invalid locale key %s\n", row[1]);
					exit(1);
				}
			}
		}
	}

	fprintf(stdout, "Setting DB to locale %s\n", g_stLocale.c_str());

	AccountDB::Instance().SetLocale(g_stLocale);

	AccountDB::Instance().ConnectAsync(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1], g_stLocale.c_str());

	// Player DB 접속
	DBManager::Instance().Connect(db_host[0], mysql_db_port[0], db_user[0], db_pwd[0], db_db[0]);

	if (!DBManager::Instance().IsConnected())
	{
		fprintf(stderr, "PlayerSQL.ConnectError\n");
		exit(1);
	}

	fprintf(stdout, "PlayerSQL connected\n");

	if (false == g_bAuthServer)
	{
		// Log DB 접속
		LogManager::Instance().Connect(log_host, log_port, log_user, log_pwd, log_db);

		if (!LogManager::Instance().IsConnected())
		{
			fprintf(stderr, "LogSQL.ConnectError\n");
			exit(1);
		}

		fprintf(stdout, "LogSQL connected\n");

		LogManager::Instance().BootLog(g_stHostname.c_str(), g_bChannel);
	}

	// SKILL_POWER_BY_LEVEL
	{
		char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT mValue FROM locale WHERE mKey='SKILL_POWER_BY_LEVEL'");
		auto pMsg(AccountDB::Instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			fprintf(stderr, "[SKILL_PERCENT] Query failed: %s", szQuery);
			exit(1);
		}

		MYSQL_ROW row;

		row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		const char * p = row[0];
		int cnt = 0;
		char num[128];
		int aiBaseSkillPowerByLevelTable[SKILL_MAX_LEVEL+1];

		fprintf(stdout, "SKILL_POWER_BY_LEVEL %s\n", p);
		while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
		{
			p = one_argument(p, num, sizeof(num));
			aiBaseSkillPowerByLevelTable[cnt++] = atoi(num);

			//fprintf(stdout, "%d %d\n", cnt - 1, aiBaseSkillPowerByLevelTable[cnt - 1]);
			if (*p == '\0')
			{
				if (cnt != (SKILL_MAX_LEVEL + 1))
				{
					fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
					exit(1);
				}

				fprintf(stdout, "SKILL_POWER_BY_LEVEL: Done! (count %d)\n", cnt);
				break;
			}
		}

		for (int job = 0; job < JOB_MAX_NUM * 2; ++job)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT mValue from locale where mKey='SKILL_POWER_BY_LEVEL_TYPE%d' ORDER BY CAST(mValue AS unsigned)", job);
			auto pMsg(AccountDB::Instance().DirectQuery(szQuery));

			if (pMsg->Get()->uiNumRows == 0)
			{
				CTableBySkill::Instance().SetSkillPowerByLevelFromType(job, aiBaseSkillPowerByLevelTable);
				continue;
			}

			row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			cnt = 0;
			p = row[0];
			int aiSkillTable[SKILL_MAX_LEVEL + 1];

			fprintf(stdout, "SKILL_POWER_BY_JOB %d %s\n", job, p);
			while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
			{
				p = one_argument(p, num, sizeof(num));
				aiSkillTable[cnt++] = atoi(num);

				//fprintf(stdout, "%d %d\n", cnt - 1, aiBaseSkillPowerByLevelTable[cnt - 1]);
				if (*p == '\0')
				{
					if (cnt != (SKILL_MAX_LEVEL + 1))
					{
						fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
						exit(1);
					}

					fprintf(stdout, "SKILL_POWER_BY_JOB: Done! (job: %d count: %d)\n", job, cnt);
					break;
				}
			}

			CTableBySkill::Instance().SetSkillPowerByLevelFromType(job, aiSkillTable);
		}
	}
	// END_SKILL_POWER_BY_LEVEL

	// LOG_KEEP_DAYS_EXTEND
	log_set_expiration_days(2);
	// END_OF_LOG_KEEP_DAYS_EXTEND
	return true;
}

static bool __LoadDefaultConfigFile(const char* configName)
{
	FILE	*fp;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("port")
		{
			str_to_number(mother_port, value_string);
			continue;
		}

		TOKEN("p2p_port")
		{
			str_to_number(p2p_port, value_string);
			continue;
		}

		TOKEN("map_allow")
		{
			char * p = value_string;
			string stNum;

			for (; *p; p++)
			{
				if (isnhspace(*p))
				{
					if (stNum.length())
					{
						int	index = 0;
						str_to_number(index, stNum.c_str());
						map_allow_add(index);
						stNum.clear();
					}
				}
				else
					stNum += *p;
			}

			if (stNum.length())
		{
				int	index = 0;
				str_to_number(index, stNum.c_str());
				map_allow_add(index);
			}

			continue;
		}

		TOKEN("auth_server")
		{
			char szIP[32];
			char szPort[32];

			two_arguments(value_string, szIP, sizeof(szIP), szPort, sizeof(szPort));

			if (!*szIP || (!*szPort && strcasecmp(szIP, "master")))
			{
				fprintf(stderr, "AUTH_SERVER: syntax error: <ip|master> <port>\n");
				exit(1);
			}

			g_bAuthServer = true;

			LoadBanIP("BANIP");

			if (!strcasecmp(szIP, "master"))
				fprintf(stdout, "AUTH_SERVER: I am the master\n");
			else
		{
				g_stAuthMasterIP = szIP;
				str_to_number(g_wAuthMasterPort, szPort);

				fprintf(stdout, "AUTH_SERVER: master %s %u\n", g_stAuthMasterIP.c_str(), g_wAuthMasterPort);
			}
			continue;
		}
	}

	fclose(fp);
	return true;
}

static bool __LoadGeneralConfigFile(const char* configName)
{
	FILE	*fp;

	char	buf[256];
	char	token_string[256];
	char	value_string[256];

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		//OPENID
		TOKEN("WEB_AUTH")
		{
			two_arguments(value_string, openid_host, sizeof(openid_host), openid_uri, sizeof(openid_uri));

			if (!*openid_host || !*openid_uri)
			{
				fprintf(stderr, "WEB_AUTH syntax error (ex: WEB_AUTH <host(metin2.co.kr) uri(/kyw/gameauth.php)>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "WEB_AUTH: %s %s", openid_host, openid_uri);
			continue;
		}

		// DB_ONLY_BEGIN
		TOKEN("BLOCK_LOGIN")
		{
			g_stBlockDate = value_string;
		}

		TOKEN("adminpage_ip")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_ip1")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_ip2")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_ip3")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_password")
		{
			g_stAdminPagePassword = value_string;
		}
		// DB_ONLY_END

		// CONNECTION_BEGIN
		TOKEN("db_port")
		{
			str_to_number(db_port, value_string);
			continue;
		}

		TOKEN("db_addr")
		{
			strlcpy(db_addr, value_string, sizeof(db_addr));

			for (int n =0; n < ADDRESS_MAX_LEN; ++n)
			{
				if (db_addr[n] == ' ')
					db_addr[n] = '\0';
			}

			continue;
		}
		// CONNECTION_END

		TOKEN("empire_whisper")
		{
			bool b_value = 0;
			str_to_number(b_value, value_string);
			g_bEmpireWhisper = !!b_value;
			continue;
		}

		TOKEN("mark_server")
		{
			guild_mark_server = is_string_true(value_string);
			continue;
		}

		TOKEN("mark_min_level")
		{
			str_to_number(guild_mark_min_level, value_string);
			guild_mark_min_level = MINMAX(0, guild_mark_min_level, GUILD_MAX_LEVEL);
			continue;
		}

		TOKEN("log_keep_days")
		{
			int i = 0;
			str_to_number(i, value_string);
			log_set_expiration_days(MINMAX(1, i, 90));
			continue;
		}

		TOKEN("passes_per_sec")
		{
			str_to_number(passes_per_sec, value_string);
			continue;
		}

		TOKEN("save_event_second_cycle")
		{
			int	cycle = 0;
			str_to_number(cycle, value_string);
			save_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("ping_event_second_cycle")
		{
			int	cycle = 0;
			str_to_number(cycle, value_string);
			ping_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("table_postfix")
		{
			g_table_postfix = value_string;
			continue;
		}

		TOKEN("test_server")
		{
			printf("-----------------------------------------------\n");
			printf("TEST_SERVER\n");
			printf("-----------------------------------------------\n");
			str_to_number(test_server, value_string);
			continue;
		}

		TOKEN("speed_server")
		{
			printf("-----------------------------------------------\n");
			printf("SPEED_SERVER\n");
			printf("-----------------------------------------------\n");
			str_to_number(speed_server, value_string);
			continue;
		}

		TOKEN("distribution_test_server")
		{
			str_to_number(distribution_test_server, value_string);
			continue;
		}

		TOKEN("shutdowned")
		{
			g_bNoMoreClient = true;
			continue;
		}

		TOKEN("no_regen")
		{
			g_bNoRegen = true;
			continue;
		}

#ifdef ENABLE_NEWSTUFF
		TOKEN("item_count_limit")
		{
			str_to_number(g_bItemCountLimit, value_string);
			fprintf(stdout, "ITEM_COUNT_LIMIT: %d\n", g_bItemCountLimit);
			continue;
		}

		TOKEN("disable_shop_price_3x")
		{
			g_bEmpireShopPriceTripleDisable = true;
			fprintf(stdout, "EMPIRE_SHOP_PRICE_3x: DISABLED\n");
			continue;
		}

		TOKEN("shop_price_3x_tax") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bEmpireShopPriceTripleDisable = !flag;
			fprintf(stdout, "SHOP_PRICE_3X_TAX: %s\n", (!g_bEmpireShopPriceTripleDisable)?"ENABLED":"DISABLED");
			continue;
		}

		//unused
		TOKEN("enable_shout_addon")
		{
			g_bShoutAddonEnable = true;
			continue;
		}

		//unused
		TOKEN("enable_all_mount_attack")
		{
			g_bAllMountAttack = true;
			continue;
		}

		TOKEN("disable_change_attr_time")
		{
			g_dwItemBonusChangeTime = 0;
			fprintf(stdout, "CHANGE_ATTR_TIME_LIMIT: DISABLED\n");
			continue;
		}

		TOKEN("change_attr_time_limit") //alternative
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwItemBonusChangeTime = flag;
			fprintf(stdout, "CHANGE_ATTR_TIME_LIMIT: %u\n", g_dwItemBonusChangeTime);
			continue;
		}

		TOKEN("disable_prism_item")
		{
			g_bDisablePrismNeed = true;
			fprintf(stdout, "PRISM_ITEM_REQUIREMENT: DISABLED\n");
			continue;
		}

		TOKEN("prism_item_require") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bDisablePrismNeed = !flag;
			fprintf(stdout, "PRISM_ITEM_REQUIRE: %s\n", (!g_bDisablePrismNeed)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("enable_global_shout")
		{
			g_bGlobalShoutEnable = true;
			fprintf(stdout, "GLOBAL_SHOUT: ENABLED\n");
			continue;
		}

		TOKEN("global_shout") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bGlobalShoutEnable = !!flag;
			fprintf(stdout, "GLOBAL_SHOUT: %s\n", (g_bGlobalShoutEnable)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("disable_emotion_mask")
		{
			g_bDisableEmotionMask = true;
			fprintf(stdout, "EMOTION_MASK_REQUIREMENT: DISABLED\n");
			continue;
		}

		TOKEN("emotion_mask_require") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bDisableEmotionMask = !flag;
			fprintf(stdout, "EMOTION_MASK_REQUIRE: %s\n", (g_bDisableEmotionMask)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("enable_bootary_check")
		{
			g_bEnableBootaryCheck = true;
			fprintf(stdout, "ENABLE_BOOTARY_CHECK: ENABLED\n");
			continue;
		}

		TOKEN("bootary_check") //alternative
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bEnableBootaryCheck = !!flag;
			fprintf(stdout, "BOOTARY_CHECK: %s\n", (g_bEnableBootaryCheck)?"ENABLED":"DISABLED");
			continue;
		}

		TOKEN("status_point_get_level_limit")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointGetLevelLimit = MINMAX(0, flag, PLAYER_MAX_LEVEL_CONST);
			fprintf(stdout, "STATUS_POINT_GET_LEVEL_LIMIT: %d\n", g_iStatusPointGetLevelLimit);
			continue;
		}

		TOKEN("status_point_set_max_value")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointSetMaxValue = flag;
			fprintf(stdout, "STATUS_POINT_SET_MAX_VALUE: %d\n", g_iStatusPointSetMaxValue);
			continue;
		}

		TOKEN("shout_limit_level")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iShoutLimitLevel = flag;
			fprintf(stdout, "SHOUT_LIMIT_LEVEL: %d\n", g_iShoutLimitLevel);
			continue;
		}

		TOKEN("db_log_level")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_iDbLogLevel = flag;
			fprintf(stdout, "DB_LOG_LEVEL: %d\n", g_iDbLogLevel);
			continue;
		}

		TOKEN("sys_log_level")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_iSysLogLevel = flag;
			fprintf(stdout, "SYS_LOG_LEVEL: %d\n", g_iSysLogLevel);
			continue;
		}

		TOKEN("item_destroy_time_autogive")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_AUTOGIVE: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
			continue;
		}

		TOKEN("item_destroy_time_dropgold")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_DROPGOLD: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD]);
			continue;
		}

		TOKEN("item_destroy_time_dropitem")
		{
			int flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM] = flag;
			fprintf(stdout, "ITEM_DESTROY_TIME_DROPITEM: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]);
			continue;
		}

		// TOKEN("shout_limit_time")
		// {
			// int flag = 0;
			// str_to_number(flag, value_string);
			// if (flag <= 0) continue;

			// g_iShoutLimitTime = flag;
			// fprintf(stdout, "SHOUT_LIMIT_TIME: %d\n", g_iShoutLimitTime);
			// continue;
		// }

		TOKEN("check_version_server")
		{
			int flag = 0;

			str_to_number(flag, value_string);
			g_bCheckClientVersion = !!flag;
			fprintf(stdout, "CHECK_VERSION_SERVER: %d\n", g_bCheckClientVersion);
			continue;
		}

		TOKEN("check_version_value")
		{
			g_stClientVersion = value_string;
			fprintf(stdout, "CHECK_VERSION_VALUE: %s\n", g_stClientVersion.c_str());
			continue;
		}

		TOKEN("enable_hack_check")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			gHackCheckEnable = !!flag;
			fprintf(stdout, "ENABLE_HACK_CHECK: %d\n", gHackCheckEnable);
			continue;
		}

		TOKEN("gm_host_check")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGMHostCheck = !!flag;
			fprintf(stdout, "GM_HOST_CHECK: %d\n", g_bGMHostCheck);
			continue;
		}

		TOKEN("guild_invite_limit")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInviteLimit = !!flag;
			fprintf(stdout, "GUILD_INVITE_LIMIT: %d\n", g_bGuildInviteLimit);
			continue;
		}

		TOKEN("guild_infinite_members")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInfiniteMembers = !!flag;
			fprintf(stdout, "GUILD_INFINITE_MEMBERS: %d\n", g_bGuildInfiniteMembers);
			continue;
		}

		TOKEN("empire_language_check")
		{
			int flag = 0;
			str_to_number(flag, value_string);
			g_bDisableEmpireLanguageCheck = !flag;
			fprintf(stdout, "EMPIRE_LANGUAGE_CHECK: %s\n", (g_bDisableEmpireLanguageCheck)?"DISABLED":"ENABLED");
			continue;
		}

		TOKEN("skillbook_nextread_min")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMin = flag;
			fprintf(stdout, "SKILLBOOK_NEXTREAD_MIN: %u\n", g_dwSkillBookNextReadMin);
			continue;
		}

		TOKEN("skillbook_nextread_max")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMax = flag;
			fprintf(stdout, "SKILLBOOK_NEXTREAD_MAX: %u\n", g_dwSkillBookNextReadMax);
			continue;
		}
#endif

//Custom
		TOKEN("emotion_without_mask") {
			int flag = 0;
			str_to_number(flag, value_string);
			if (1 == flag) {
				emotion_without_mask = true;
				fprintf(stderr, "EMOTION_WITHOUT_MASK: %i\n", flag);
			}
		}

		TOKEN("emotion_same_gender") {
			int flag = 0;
			str_to_number(flag, value_string);
			if (1 == flag) {
				emotion_same_gender = true;
				fprintf(stderr, "EMOTION_SAME_GENDER: %i\n", flag);
			}
		}

		TOKEN("block_potions_in_duell") {
			g_bBlockPositionsInDuell = true;
			fprintf(stdout, "POTIONS_IN_DUELL: DISABLED\n");
			continue;
		}

		//@custom029
		TOKEN("attr_always_add") {
			int flag = 0;
			str_to_number(flag, value_string);
			fprintf(stderr, "ATTR_ALWAYS_ADD: %d\n", flag);
			if (1 == flag) {
				attr_always_add = true;
			}
		}

		//@custom030
		TOKEN("attr_always_5_add") {
			int flag = 0;
			str_to_number(flag, value_string);
			fprintf(stderr, "ATTR_ALWAYS_5_ADD: %d\n", flag);
			if (1 == flag) {
				attr_always_5_add = true;
			}
		}

		//@custom026
		TOKEN("attr_rare_enable") {
			int flag = 0;
			str_to_number(flag, value_string);
			if (0 == flag) {
				attr_rare_enable = false;
				fprintf(stderr, "ATTR_RARE_ENABLE: %i\n", flag);
			}
		}

		//@custom009
		TOKEN("belt_allow_all_items") {
			int flag = 0;
			str_to_number(flag, value_string);
			if (1 == flag) {
				belt_allow_all_items = true;
				fprintf(stderr, "BELT_ALLOW_ALL_ITEMS: %i\n", flag);
			}
		}

		TOKEN("ban_force_reason") {
			int flag = 0;
			str_to_number(flag, value_string);
			fprintf(stderr, "BAN_FORCE_REASON: %d\n", flag);
			if (0 == flag) {
				ban_force_reason = false;
			}
		}

		TOKEN("trade_effect") {
			int flag = 0;
			str_to_number(flag, value_string);
			fprintf(stderr, "TRADE_EFFECT: %d\n", flag);
			if (0 == flag) {
				trade_effect = false;
			}
		}

		TOKEN("trade_effect_shop_threshold") {
			str_to_number(trade_effect_shop_threshold, value_string);
			trade_effect_shop_threshold = MINMAX(0, trade_effect_shop_threshold, 1000000000);
			fprintf(stderr, "TRADE_EFFECT_SHOP_THRESHOLD: %d\n", trade_effect_shop_threshold);
		}

		TOKEN("trade_effect_exchange_threshold") {
			str_to_number(trade_effect_exchange_threshold, value_string);
			trade_effect_exchange_threshold = MINMAX(0, trade_effect_exchange_threshold, 1000000000);
			fprintf(stderr, "TRADE_EFFECT_EXCHANGE_THRESHOLD: %d\n", trade_effect_exchange_threshold);
		}

		TOKEN("view_equip_for_all") {
			int flag = 0;
			str_to_number(flag, value_string);
			fprintf(stderr, "VIEW_EQUIP_FOR_ALL: %d\n", flag);
			if (1 == flag) {
				view_equip_for_all = true;
			}
		}

		TOKEN("view_equip_sp_cost") {
			str_to_number(view_equip_sp_cost, value_string);
			view_equip_sp_cost = MINMAX(0, view_equip_sp_cost, 100);
			fprintf(stderr, "VIEW_EQUIP_SP_COST: %d\n", view_equip_sp_cost);
		}

		TOKEN("attribute_double_change") {
			int flag = 0;
			str_to_number(flag, value_string);
			if (1 == flag) {
				attribute_double_change = false;
				fprintf(stderr, "ATTRIBUTE_DOUBLE_CHANGE: %i\n", flag);
			}
		}

		TOKEN("RIDE_SECONDS_TO_RIDE_AGAIN") {
			str_to_number(ride_seconds_to_ride_again, value_string);
			ride_seconds_to_ride_again = MINMAX(0, ride_seconds_to_ride_again, 5);
			fprintf(stderr, "RIDE_SECONDS_TO_RIDE_AGAIN: %d\n", ride_seconds_to_ride_again);
		}

		TOKEN("disable_movspeed_hacklog") {
			str_to_number(g_bDisableMovspeedHacklog, value_string);
			continue;
		}

		TOKEN("movspeed_hack_threshold") {
			str_to_number(g_dMovspeedHackThreshold, value_string);
			continue;
		}

		TOKEN("POISON_DRAW_AGGRO") {	//@fixme435
			int flag = 0;
			str_to_number(flag, value_string);
			if (0 == flag) {
				poison_draw_monsters_aggro = false;
				fprintf(stderr, "POISON_DRAW_AGGRO: %i\n", flag);
			}
		}
// END Custom

#ifdef ENABLE_PROXY_IP
		TOKEN("proxy_ip")
		{
			g_stProxyIP = value_string;
		}
#endif

		TOKEN("traffic_profile")
		{
			g_bTrafficProfileOn = true;
			continue;
		}

		TOKEN("no_wander")
		{
			no_wander = true;
			continue;
		}

		TOKEN("user_limit")
		{
			str_to_number(g_iUserLimit, value_string);
			continue;
		}

		TOKEN("skill_disable")
		{
			str_to_number(g_bSkillDisable, value_string);
			continue;
		}

		TOKEN("quest_dir")
		{
			sys_log(0, "QUEST_DIR SETTING : %s", value_string);
			g_stQuestDir = value_string;
		}

		TOKEN("quest_object_dir")
		{
			//g_stQuestObjectDir = value_string;
			std::istringstream is(value_string);
			sys_log(0, "QUEST_OBJECT_DIR SETTING : %s", value_string);
			string dir;
			while (!is.eof())
			{
				is >> dir;
				if (is.fail())
					break;
				g_setQuestObjectDir.insert(dir);
				sys_log(0, "QUEST_OBJECT_DIR INSERT : %s", dir .c_str());
			}
		}

		TOKEN("synchack_limit_count")
		{
			str_to_number(g_iSyncHackLimitCount, value_string);
		}

		TOKEN("speedhack_limit_count")
		{
			str_to_number(SPEEDHACK_LIMIT_COUNT, value_string);
		}

		TOKEN("speedhack_limit_bonus")
		{
			str_to_number(SPEEDHACK_LIMIT_BONUS, value_string);
		}

		TOKEN("server_id")
		{
			str_to_number(g_server_id, value_string);
		}

		TOKEN("mall_url")
		{
			g_strWebMallURL = value_string;
		}

		TOKEN("bind_ip")
		{
			strlcpy(g_szPublicIP, value_string, sizeof(g_szPublicIP));
		}

		TOKEN("view_range")
		{
			str_to_number(VIEW_RANGE, value_string);
		}

		TOKEN("spam_block_duration")
		{
			str_to_number(g_uiSpamBlockDuration, value_string);
		}

		TOKEN("spam_block_score")
		{
			str_to_number(g_uiSpamBlockScore, value_string);
			g_uiSpamBlockScore = MAX(1, g_uiSpamBlockScore);
		}

		TOKEN("spam_block_reload_cycle")
		{
			str_to_number(g_uiSpamReloadCycle, value_string);
			g_uiSpamReloadCycle = MAX(60, g_uiSpamReloadCycle);
		}

		TOKEN("check_multihack")
		{
			str_to_number(g_bCheckMultiHack, value_string);
		}

		TOKEN("spam_block_max_level")
		{
			str_to_number(g_iSpamBlockMaxLevel, value_string);
		}
		TOKEN("protect_normal_player")
		{
			str_to_number(g_protectNormalPlayer, value_string);
		}
		TOKEN("notice_battle_zone")
		{
			str_to_number(g_noticeBattleZone, value_string);
		}

		TOKEN("pk_protect_level")
		{
		    str_to_number(PK_PROTECT_LEVEL, value_string);
		    fprintf(stderr, "PK_PROTECT_LEVEL: %d", PK_PROTECT_LEVEL);
		}

		TOKEN("max_level")
		{
			str_to_number(gPlayerMaxLevel, value_string);

			gPlayerMaxLevel = MINMAX(1, gPlayerMaxLevel, PLAYER_MAX_LEVEL_CONST);

			fprintf(stderr, "PLAYER_MAX_LEVEL: %d\n", gPlayerMaxLevel);
		}

		TOKEN("shutdown_age")
		{
			str_to_number(gShutdownAge, value_string);
			fprintf(stderr, "SHUTDOWN_AGE: %d\n", gShutdownAge);

		}

		TOKEN("shutdown_enable")
		{
			str_to_number(gShutdownEnable, value_string);
			fprintf(stderr, "SHUTDOWN_ENABLE: %d\n", gShutdownEnable);
		}

		TOKEN("block_char_creation")
		{
			int tmp = 0;

			str_to_number(tmp, value_string);

			if (0 == tmp)
				g_BlockCharCreation = false;
			else
				g_BlockCharCreation = true;

			continue;
		}
	}

	fclose(fp);
	return true;
}

static bool __LoadDefaultCMDFile(const char* cmdName)
	{
	FILE	*fp;
	char	buf[256];

	if ((fp = fopen(cmdName, "r")))
	{
		while (fgets(buf, 256, fp))
		{
			char cmd[32], levelname[32];
			int level;

			two_arguments(buf, cmd, sizeof(cmd), levelname, sizeof(levelname));

			if (!*cmd || !*levelname)
			{
#ifdef ENABLE_CMD_PLAYER
				fprintf(stderr, "CMD syntax error: <cmd> <PLAYER | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#else
				fprintf(stderr, "CMD syntax error: <cmd> <LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#endif
				exit(1);
			}

			if (!strcasecmp(levelname, "LOW_WIZARD"))
				level = GM_LOW_WIZARD;
			else if (!strcasecmp(levelname, "WIZARD"))
				level = GM_WIZARD;
			else if (!strcasecmp(levelname, "HIGH_WIZARD"))
				level = GM_HIGH_WIZARD;
			else if (!strcasecmp(levelname, "GOD"))
				level = GM_GOD;
			else if (!strcasecmp(levelname, "IMPLEMENTOR"))
				level = GM_IMPLEMENTOR;
#ifdef ENABLE_VIP_SYSTEM
			else if (!strcasecmp(levelname, "VIP"))
				level = GM_VIP;
#endif
#ifdef ENABLE_CMD_PLAYER
			else if (!strcasecmp(levelname, "PLAYER"))
				level = GM_PLAYER;
#endif
			else if (!strcasecmp(levelname, "DISABLE"))
				level = GM_DISABLE;
			else
			{
#ifdef ENABLE_CMD_PLAYER
				fprintf(stderr, "CMD syntax error: <cmd> <PLAYER | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#else
				fprintf(stderr, "CMD syntax error: <cmd> <LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#endif
				exit(1);
			}

			if (test_server)
				fprintf(stdout, "CMD_REWRITE: [%s] [%s:%d]\n", cmd, levelname, level);
			// interpreter_set_privilege(cmd, level);	//@fixme420
		}

		fclose(fp);
		return true;
	}
	return false;
}

#ifdef ENABLE_EXPTABLE_FROMDB
static bool __LoadExpTableFromDB(void)
{
	auto pMsg(AccountDB::Instance().DirectQuery("SELECT level, exp FROM exp_table"));
	if (pMsg->Get()->uiNumRows == 0)
		return false;

	static uint32_t new_exp_table[PLAYER_MAX_LEVEL_CONST+1];
	if (exp_table != nullptr)
		memcpy(new_exp_table, exp_table, (PLAYER_MAX_LEVEL_CONST+1)*sizeof(uint32_t));

	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		uint32_t level = 0;
		uint32_t exp = 0;
		str_to_number(level, row[0]);
		str_to_number(exp, row[1]);
		if (level > PLAYER_MAX_LEVEL_CONST)
			continue;
		new_exp_table[level] = exp;
		// printf("new_exp_table[%u] = %u;\n", level, exp);
	}
	exp_table = new_exp_table;
	return true;
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
static bool __LoadGrowthPetExpTableFromDB(void)
{
	auto pMsg(AccountDB::Instance().DirectQuery("SELECT growthpet_level, growthpet_exp FROM exp_growthpet_table"));
	if (pMsg->Get()->uiNumRows == 0)
		return false;

	static uint32_t new_growthpet_exp_table[GROWTHPET_MAX_LEVEL_CONST + 1];
	if (exp_pet_table != nullptr)
		memcpy(new_growthpet_exp_table, exp_pet_table, (GROWTHPET_MAX_LEVEL_CONST + 1) * sizeof(uint32_t));

	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		uint32_t clevel = 0;
		uint32_t cexp = 0;
		str_to_number(clevel, row[0]);
		str_to_number(cexp, row[1]);
		if (clevel > GROWTHPET_MAX_LEVEL_CONST)
			continue;
		new_growthpet_exp_table[clevel] = cexp;
		// printf("new_growthpet_exp_table[%u] = %u;\n", clevel, cexp);
	}
	exp_pet_table = new_growthpet_exp_table;
	return true;
}
#endif
#endif

void config_init(const string& st_localeServiceName)
{
	// LOCALE_SERVICE
	string	st_configFileName;

	st_configFileName.reserve(32);
	st_configFileName = "CONFIG";

	if (!st_localeServiceName.empty())
	{
		st_configFileName += ".";
		st_configFileName += st_localeServiceName;
	}
	// END_OF_LOCALE_SERVICE

	if (!GetIPInfo())
	{
	//	fprintf(stderr, "Can not get public ip address\n");
	//	exit(1);
	}

	// default config load (REQUIRED)
	if (!__LoadConnectConfigFile(st_configFileName.c_str()) ||
		!__LoadDefaultConfigFile(st_configFileName.c_str()) ||
		!__LoadGeneralConfigFile(st_configFileName.c_str())
	)
	{
		fprintf(stderr, "Can not open [%s]\n", st_configFileName.c_str());
		exit(1);
	}
#ifdef ENABLE_GENERAL_CONFIG
	// general config - locale based
	{
		char szFileName[256];
		snprintf(szFileName, sizeof(szFileName), "%s/conf/GENERAL_%s", LocaleService_GetBasePath().c_str(), st_configFileName.c_str());
		if (__LoadGeneralConfigFile(szFileName))
			fprintf(stderr, "GENERAL CONFIG LOAD OK [%s]\n", szFileName);
	}
	// general config - locale n channel based
	{
		char szFileName[256];
		snprintf(szFileName, sizeof(szFileName), "%s/conf/GENERAL_%s_CHANNEL_%d", LocaleService_GetBasePath().c_str(), st_configFileName.c_str(), g_bChannel);
		if (__LoadGeneralConfigFile(szFileName))
			fprintf(stderr, "GENERAL CONFIG LOAD OK [%s]\n", szFileName);
	}
	// general config - locale n channel n hostname based
	{
		char szFileName[256];
		snprintf(szFileName, sizeof(szFileName), "%s/conf/GENERAL_%s_CHANNEL_%d_HOSTNAME_%s", LocaleService_GetBasePath().c_str(), st_configFileName.c_str(), g_bChannel, g_stHostname.c_str());
		if (__LoadGeneralConfigFile(szFileName))
			fprintf(stderr, "GENERAL CONFIG LOAD OK [%s]\n", szFileName);
	}
#endif

	if (g_setQuestObjectDir.empty())
		g_setQuestObjectDir.insert(g_stDefaultQuestObjectDir);

	if (0 == db_port)
	{
		fprintf(stderr, "DB_PORT not configured\n");
		exit(1);
	}

	if (0 == g_bChannel)
	{
		fprintf(stderr, "CHANNEL not configured\n");
		exit(1);
	}

	if (g_stHostname.empty())
	{
		fprintf(stderr, "HOSTNAME must be configured.\n");
		exit(1);
	}

	// LOCALE_SERVICE
#ifndef ENABLE_MULTI_LANGUAGE_SYSTEM
	LocaleService_LoadLocaleStringFile();
#endif
	LocaleService_TransferDefaultSetting();
	LocaleService_LoadEmpireTextConvertTables();
	// END_OF_LOCALE_SERVICE

#ifdef ENABLE_EXPTABLE_FROMDB
	if (!__LoadExpTableFromDB())
	{
		// do as you please to manage this
		fprintf(stderr, "Failed to Load ExpTable from DB so exit\n");
		// exit(1);
	}

#	ifdef ENABLE_GROWTH_PET_SYSTEM
	if (!__LoadGrowthPetExpTableFromDB())
	{
		// do as you please to manage this
		fprintf(stderr, "Failed to Load GrowthPetExpTable from DB so exit\n");
		// exit(1);
	}
#	endif
#endif

	std::string st_cmdFileName("CMD");
	__LoadDefaultCMDFile(st_cmdFileName.c_str());
#ifdef ENABLE_GENERAL_CMD
	// general cmd - locale based
	{
		char szFileName[256];
		snprintf(szFileName, sizeof(szFileName), "%s/conf/GENERAL_%s", LocaleService_GetBasePath().c_str(), st_cmdFileName.c_str());
		if (__LoadDefaultCMDFile(szFileName))
			fprintf(stdout, "GENERAL CMD LOAD OK [%s]\n", szFileName);
	}
	// general cmd - locale n channel based
	{
		char szFileName[256];
		snprintf(szFileName, sizeof(szFileName), "%s/conf/GENERAL_%s_CHANNEL_%d", LocaleService_GetBasePath().c_str(), st_cmdFileName.c_str(), g_bChannel);
		if (__LoadDefaultCMDFile(szFileName))
			fprintf(stdout, "GENERAL CMD LOAD OK [%s]\n", szFileName);
	}
	// general cmd - locale n channel n hostname based
	{
		char szFileName[256];
		snprintf(szFileName, sizeof(szFileName), "%s/conf/GENERAL_%s_CHANNEL_%d_HOSTNAME_%s", LocaleService_GetBasePath().c_str(), st_cmdFileName.c_str(), g_bChannel, g_stHostname.c_str());
		if (__LoadDefaultCMDFile(szFileName))
			fprintf(stdout, "GENERAL CMD LOAD OK [%s]\n", szFileName);
	}
#endif

#ifndef WIN32
	if(!gHackCheckEnable)
	{
		assert(test_server);
	}
#endif

	LoadValidCRCList();
	LoadStateUserCount();

	CWarMapManager::Instance().LoadWarMapInfo(nullptr);

	FN_log_adminpage();
	if (g_szPublicIP[0] == '0')
	{
		fprintf(stderr, "Can not get public ip address\n");
		exit(1);
	}
}

const char* get_table_postfix()
{
	return g_table_postfix.c_str();
}

void LoadValidCRCList()
{
	s_set_dwProcessCRC.clear();
	s_set_dwFileCRC.clear();

	FILE * fp;
	char buf[256];

	if ((fp = fopen("CRC", "r")))
	{
		while (fgets(buf, 256, fp))
		{
			if (!*buf)
				continue;

			uint32_t dwValidClientProcessCRC;
			uint32_t dwValidClientFileCRC;

			sscanf(buf, " %u %u ", &dwValidClientProcessCRC, &dwValidClientFileCRC);

			s_set_dwProcessCRC.insert(dwValidClientProcessCRC);
			s_set_dwFileCRC.insert(dwValidClientFileCRC);

			fprintf(stderr, "CLIENT_CRC: %u %u\n", dwValidClientProcessCRC, dwValidClientFileCRC);
		}

		fclose(fp);
	}
}

bool LoadClientVersion()
{
	FILE * fp = fopen("VERSION", "r");

	if (!fp)
		return false;

	char buf[256];
	fgets(buf, 256, fp);

	char * p = strchr(buf, '\n');
	if (p) *p = '\0';

	fprintf(stderr, "VERSION: \"%s\"\n", buf);

	g_stClientVersion = buf;
	fclose(fp);
	return true;
}

void CheckClientVersion()
{
	const DESC_MANAGER::DESC_SET & set = DESC_MANAGER::Instance().GetClientSet();
	DESC_MANAGER::DESC_SET::const_iterator it = set.begin();

	while (it != set.end())
	{
		LPDESC d = *(it++);

		if (!d->GetCharacter())
			continue;

		if (0 != g_stClientVersion.compare(d->GetClientVersion())) // @fixme103 (version > date)
		{
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "[LS;875]");
			d->DelayedDisconnect(10);
		}
	}
}

void LoadStateUserCount()
{
	FILE * fp = fopen("state_user_count", "r");

	if (!fp)
		return;

	fscanf(fp, " %d %d ", &g_iFullUserCount, &g_iBusyUserCount);

	fclose(fp);
}

bool IsValidProcessCRC(uint32_t dwCRC)
{
	return s_set_dwProcessCRC.find(dwCRC) != s_set_dwProcessCRC.end();
}

bool IsValidFileCRC(uint32_t dwCRC)
{
	return s_set_dwFileCRC.find(dwCRC) != s_set_dwFileCRC.end();
}


