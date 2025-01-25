#include "stdafx.h"
#include "../../common/CommonDefines.h"

#include "../../common/building.h"
#include "../../common/VnumHelper.h"
#include "../../libgame/include/grid.h"

#include "ClientManager.h"

#include "Main.h"
#include "Config.h"
#include "DBManager.h"
#include "QID.h"
#include "GuildManager.h"
#include "PrivManager.h"
#include "MoneyLog.h"
#include "ItemAwardManager.h"
#include "Marriage.h"
#include "Monarch.h"
#include "BlockCountry.h"
#include "ItemIDRangeManager.h"
#include "Cache.h"
#include <sstream>
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "Shop.h"
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
# include "LottoManager.h"
#endif

extern int g_iPlayerCacheFlushSeconds;
extern int g_iItemCacheFlushSeconds;
extern int g_test_server;
extern int g_log;
extern std::string g_stLocale;
extern std::string g_stLocaleNameColumn;
bool CreateItemTableFromRes(MYSQL_RES* res, std::vector<TPlayerItem>* pVec, uint32_t dwPID);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
std::map<uint32_t, spShop> shopMap;
#endif

uint32_t g_dwUsageMax = 0;
uint32_t g_dwUsageAvg = 0;

CPacketInfo g_query_info;
CPacketInfo g_item_info;

int g_item_count = 0;
int g_query_count[2];
#ifdef ENABLE_PROTO_FROM_DB
bool g_bMirror2DB = false;
#endif

CClientManager::CClientManager() :
	m_pkAuthPeer(nullptr),
	m_iPlayerIDStart(0),
	m_iPlayerDeleteLevelLimit(0),
	m_iPlayerDeleteLevelLimitLower(0),
	m_bChinaEventServer(false),
	m_iShopTableSize(0),
	m_pShopTable(nullptr),
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	m_iGemShopTableSize(0),
	m_pGemShopTable(nullptr),
#endif
	m_iRefineTableSize(0),
	m_pRefineTable(nullptr),
	m_bShutdowned(false),
	m_iCacheFlushCount(0),
	m_iCacheFlushCountLimit(200)
#ifdef ENABLE_MAILBOX
	, m_iMailBoxBackupSec(3600)
#endif
{
	m_itemRange.dwMin = 0;
	m_itemRange.dwMax = 0;
	m_itemRange.dwUsableItemIDMin = 0;

	memset(g_query_count, 0, sizeof(g_query_count));
#ifdef ENABLE_PROTO_FROM_DB
	bIsProtoReadFromDB = false;
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	m_dwAchievementRanking.clear();
#endif
}

CClientManager::~CClientManager()
{
	Destroy();
}

void CClientManager::SetPlayerIDStart(int iIDStart) noexcept
{
	m_iPlayerIDStart = iIDStart;
}

void CClientManager::GetPeerP2PHostNames(std::string& peerHostNames)
{
	std::ostringstream oss(std::ostringstream::out);

	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* peer = *it;
		if (peer)
			oss << peer->GetHost() << " " << peer->GetP2PPort() << " channel : " << (int)(peer->GetChannel()) << "\n";
	}

	peerHostNames += oss.str();
}

void CClientManager::Destroy()
{
	m_mChannelStatus.clear();
	for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		(*i)->Destroy();
	}

	m_peerList.clear();

	if (m_fdAccept > 0)
	{
		socket_close(m_fdAccept);
		m_fdAccept = -1;
	}
}

#ifdef ENABLE_DEFAULT_PRIV
static bool bCleanOldPriv = true;
static bool __InitializeDefaultPriv()
{
	if (bCleanOldPriv)
	{
		auto pCleanStuff(CDBManager::Instance().DirectQuery("DELETE FROM priv_settings WHERE value <= 0 OR duration <= NOW();", SQL_COMMON));
		printf("DEFAULT_PRIV_EMPIRE: removed %llu expired priv settings.\n", pCleanStuff->Get()->uiAffectedRows);
	}
	auto pMsg(CDBManager::Instance().DirectQuery("SELECT priv_type, id, type, value, UNIX_TIMESTAMP(duration) FROM priv_settings", SQL_COMMON));
	if (pMsg->Get()->uiNumRows == 0)
		return false;
	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		if (!strcmp(row[0], "EMPIRE"))
		{
			// init
			uint8_t empire = 0;
			uint8_t type = 1;
			int value = 0;
			time_t duration_sec = 0;
			// set
			str_to_number(empire, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			str_to_number(duration_sec, row[4]);
			// recalibrate time
			const time_t now_time_sec = CClientManager::Instance().GetCurrentTime();
			if (now_time_sec > duration_sec)
				duration_sec = 0;
			else
				duration_sec -= now_time_sec;
			// send priv
			printf("DEFAULT_PRIV_EMPIRE: set empire(%u), type(%u), value(%d), duration(%u)\n", empire, type, value, duration_sec);
			CPrivManager::Instance().AddEmpirePriv(empire, type, value, duration_sec);
		}
		else if (!strcmp(row[0], "GUILD"))
		{
			// init
			uint32_t guild_id = 0;
			uint8_t type = 1;
			int value = 0;
			time_t duration_sec = 0;
			// set
			str_to_number(guild_id, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			str_to_number(duration_sec, row[4]);
			// recalibrate time
			const time_t now_time_sec = CClientManager::Instance().GetCurrentTime();
			if (now_time_sec > duration_sec)
				duration_sec = 0;
			else
				duration_sec -= now_time_sec;
			// send priv
			if (guild_id)
			{
				printf("DEFAULT_PRIV_GUILD: set guild_id(%u), type(%u), value(%d), duration(%u)\n", guild_id, type, value, duration_sec);
				CPrivManager::Instance().AddGuildPriv(guild_id, type, value, duration_sec);
			}
		}
		else if (!strcmp(row[0], "PLAYER"))
		{
			// init
			uint32_t pid = 0;
			uint8_t type = 1;
			int value = 0;
			// set
			str_to_number(pid, row[1]);
			str_to_number(type, row[2]);
			str_to_number(value, row[3]);
			// send priv
			if (pid)
			{
				printf("DEFAULT_PRIV_PLAYER: set pid(%u), type(%u), value(%d)\n", pid, type, value);
				CPrivManager::Instance().AddCharPriv(pid, type, value);
			}
		}
	}
	return true;
}

static bool __UpdateDefaultPriv(const char* priv_type, uint32_t id, uint8_t type, int value, time_t duration_sec)
{
	char szQuery[1024];
	snprintf(szQuery, 1024,
		"REPLACE INTO priv_settings SET priv_type='%s', id=%u, type=%u, value=%d, duration=DATE_ADD(NOW(), INTERVAL %u SECOND);",
		priv_type, id, type, value, duration_sec
	);
	auto pMsg(CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON));
	return pMsg->Get()->uiAffectedRows;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
TItemTable* CClientManager::GetItemTableByVnum(uint32_t dwVnum)
{
	const auto it = m_map_itemTableByVnum.find(dwVnum);
	if (it != m_map_itemTableByVnum.end())
		return it->second;

	return nullptr;
}
#endif

bool CClientManager::Initialize()
{
	int tmpValue = 0;

	//BOOT_LOCALIZATION
	if (!InitializeLocalization())
	{
		fprintf(stderr, "Failed Localization Infomation so exit\n");
		return false;
	}
#ifdef ENABLE_DEFAULT_PRIV
	if (!__InitializeDefaultPriv())
	{
		// fprintf(stderr, "Failed Default Priv Setting so exit\n");
		// return false;
	}
#endif
	//END_BOOT_LOCALIZATION
	//ITEM_UNIQUE_ID

	if (!InitializeNowItemID())
	{
		fprintf(stderr, " Item range Initialize Failed. Exit DBCache Server\n");
		return false;
	}
	//END_ITEM_UNIQUE_ID

#ifdef ENABLE_PROTO_FROM_DB
	int iTemp;
	if (CConfig::Instance().GetValue("PROTO_FROM_DB", &iTemp))
	{
		bIsProtoReadFromDB = !!iTemp;
		fprintf(stdout, "PROTO_FROM_DB: %s\n", (bIsProtoReadFromDB) ? "Enabled" : "Disabled");
	}
	if (!bIsProtoReadFromDB && CConfig::Instance().GetValue("MIRROR2DB", &iTemp))
	{
		g_bMirror2DB = !!iTemp;
		fprintf(stdout, "MIRROR2DB: %s\n", (g_bMirror2DB) ? "Enabled" : "Disabled");
	}
#endif
	if (!InitializeTables())
	{
		sys_err("Table Initialize FAILED");
		return false;
	}

	CGuildManager::Instance().BootReserveWar();

	if (!CConfig::Instance().GetValue("BIND_PORT", &tmpValue))
		tmpValue = 5300;

	char szBindIP[128];

	if (!CConfig::Instance().GetValue("BIND_IP", szBindIP, 128))
		strlcpy(szBindIP, "0", sizeof(szBindIP));

	m_fdAccept = socket_tcp_bind(szBindIP, tmpValue);

	if (m_fdAccept < 0)
	{
		perror("socket");
		return false;
	}

	sys_log(0, "ACCEPT_HANDLE: %u", m_fdAccept);
	fdwatch_add_fd(m_fdWatcher, m_fdAccept, nullptr, FDW_READ, false);

	if (!CConfig::Instance().GetValue("BACKUP_LIMIT_SEC", &tmpValue))
		tmpValue = 600;

	m_looping = true;

	if (!CConfig::Instance().GetValue("PLAYER_DELETE_LEVEL_LIMIT", &m_iPlayerDeleteLevelLimit))
	{
		sys_err("conf.txt: Cannot find PLAYER_DELETE_LEVEL_LIMIT, use default level %d", PLAYER_MAX_LEVEL_CONST + 1);
		m_iPlayerDeleteLevelLimit = PLAYER_MAX_LEVEL_CONST + 1;
	}

	if (!CConfig::Instance().GetValue("PLAYER_DELETE_LEVEL_LIMIT_LOWER", &m_iPlayerDeleteLevelLimitLower))
	{
		m_iPlayerDeleteLevelLimitLower = 0;
	}

#ifdef ENABLE_MAILBOX
	CConfig::Instance().GetValue("MAILBOX_BACKUP_SEC", &m_iMailBoxBackupSec);
#endif

	sys_log(0, "PLAYER_DELETE_LEVEL_LIMIT set to %d", m_iPlayerDeleteLevelLimit);
	sys_log(0, "PLAYER_DELETE_LEVEL_LIMIT_LOWER set to %d", m_iPlayerDeleteLevelLimitLower);

	LoadEventFlag();

	// force fit database character-set
	if (g_stLocale == "big5" || g_stLocale == "sjis")
		CDBManager::Instance().QueryLocaleSet();

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	UpdateAchievementsRanking();
#endif

	return true;
}

void CClientManager::MainLoop()
{
	SQLMsg* tmp;

	sys_log(0, "ClientManager pointer is %p", this);

	// Main loop
	while (!m_bShutdowned)
	{
		while ((tmp = CDBManager::Instance().PopResult()))
		{
			AnalyzeQueryResult(tmp);
			delete tmp;
		}

		if (!Process())
			break;

		log_rotate();
	}

	//
	// Main loop end processing
	//
	sys_log(0, "MainLoop exited, Starting cache flushing");

#ifdef ENABLE_MAILBOX
	CClientManager::Instance().MAILBOX_BACKUP();
#endif

	signal_timer_disable();

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	for (TAchievementCacheMap::iterator pAchCache = m_map_AchievementCache.begin();
		pAchCache != m_map_AchievementCache.end(); ++pAchCache) {
		pAchCache->second->Flush();
		delete pAchCache->second;
	}
	m_map_AchievementCache.clear();
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	TSkillColorCacheMap::iterator itColor = m_map_SkillColorCache.begin();

	while (itColor != m_map_SkillColorCache.end())
	{
		CSKillColorCache* pCache = itColor->second;
		pCache->Flush();
		m_map_SkillColorCache.erase(itColor++);
	}

	m_map_SkillColorCache.clear();
#endif

	auto it = m_map_playerCache.begin();

	// Player table cache flush
	while (it != m_map_playerCache.end())
	{
		CPlayerTableCache* c = (it++)->second;
		if (c)
		{
			c->Flush();
			delete c;
		}
	}
	m_map_playerCache.clear();


	auto it2 = m_map_itemCache.begin();
	// Item flush
	while (it2 != m_map_itemCache.end())
	{
		CItemCache* c = (it2++)->second;
		if (c)
		{
			c->Flush();
			delete c;
		}
	}
	m_map_itemCache.clear();

	// MYSHOP_PRICE_LIST
	//
	// Private store item price list Flush
	//
	for (const auto& itPriceList : m_mapItemPriceListCache)
	{
		CItemPriceListTableCache* pCache = itPriceList.second;
		if (pCache)
		{
			pCache->Flush();
			delete pCache;
		}
	}

	m_mapItemPriceListCache.clear();
	// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	// Clear shop
	for (const auto& shop_it : m_map_shopCache)
	{
		CShopCache* pCache = shop_it.second;
		if (pCache)
		{
			pCache->Flush();
			delete pCache;
		}
	}

	m_map_shopCache.clear();
#endif
}

void CClientManager::Quit() noexcept
{
	m_bShutdowned = true;
}

void CClientManager::QUERY_BOOT(CPeer* peer, TPacketGDBoot* p)
{
	constexpr uint8_t bPacketVersion = 6; // Whenever the BOOT packet changes, increase the number.

	std::vector<tAdminInfo> vAdmin;
	std::vector<std::string> vHost;

	__GetHostInfo(vHost);
	__GetAdminInfo(p->szIP, vAdmin);

	sys_log(0, "QUERY_BOOT : AdminInfo (Request ServerIp %s) ", p->szIP);

	const uint32_t dwPacketSize =
		sizeof(uint32_t) +
		sizeof(uint8_t) +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TMobTable) * m_vec_mobTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemTable) * m_vec_itemTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TShopTable) * m_iShopTableSize +
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TGemShopTable) * m_iGemShopTableSize +
#endif
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TSkillTable) * m_vec_skillTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TRefineTable) * m_iRefineTableSize +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemAttrTable) * m_vec_itemAttrTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemAttrTable) * m_vec_itemRareTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TBanwordTable) * m_vec_banwordTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(building::TLand) * m_vec_kLandTable.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(building::TObjectProto) * m_vec_kObjectProto.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(building::TObject) * m_map_pkObjectTable.size() +
#ifdef ENABLE_EVENT_MANAGER
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TEventTable) * m_vec_eventTable.size() +
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TBiologMissionsProto) * m_vec_BiologMissions.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TBiologRewardsProto) * m_vec_BiologRewards.size() +
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TSungmaTable) * m_vec_sungmaTable.size() +
#endif
		sizeof(time_t) +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TItemIDRangeTable) * 2 +
		//ADMIN_MANAGER
		sizeof(uint16_t) + sizeof(uint16_t) + 16 * vHost.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(tAdminInfo) * vAdmin.size() +
		//END_ADMIN_MANAGER
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(TMonarchInfo) +
		sizeof(uint16_t) + sizeof(uint16_t) + sizeof(MonarchCandidacy) * CMonarch::Instance().MonarchCandidacySize() +
		sizeof(uint16_t);

	peer->EncodeHeader(HEADER_DG_BOOT, 0, dwPacketSize);
	peer->Encode(&dwPacketSize, sizeof(uint32_t));
	peer->Encode(&bPacketVersion, sizeof(uint8_t));

	sys_log(0, "BOOT: PACKET: %d", dwPacketSize);
	sys_log(0, "BOOT: VERSION: %d", bPacketVersion);

	sys_log(0, "sizeof(TMobTable) = %d", sizeof(TMobTable));
	sys_log(0, "sizeof(TItemTable) = %d", sizeof(TItemTable));
	sys_log(0, "sizeof(TShopTable) = %d", sizeof(TShopTable));
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	sys_log(0, "sizeof(TGemShopTable) = %d", sizeof(TGemShopTable));
#endif
	sys_log(0, "sizeof(TSkillTable) = %d", sizeof(TSkillTable));
	sys_log(0, "sizeof(TRefineTable) = %d", sizeof(TRefineTable));
	sys_log(0, "sizeof(TItemAttrTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TItemRareTable) = %d", sizeof(TItemAttrTable));
	sys_log(0, "sizeof(TBanwordTable) = %d", sizeof(TBanwordTable));
	sys_log(0, "sizeof(TLand) = %d", sizeof(building::TLand));
	sys_log(0, "sizeof(TObjectProto) = %d", sizeof(building::TObjectProto));
	sys_log(0, "sizeof(TObject) = %d", sizeof(building::TObject));
#ifdef ENABLE_EVENT_MANAGER
	sys_log(0, "sizeof(TEventTable) = %d", sizeof(TEventTable));
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	sys_log(0, "sizeof(TBiologMissionsProto) = %d", sizeof(TBiologMissionsProto));
	sys_log(0, "sizeof(TBiologRewardsProto) = %d", sizeof(TBiologRewardsProto));
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	sys_log(0, "sizeof(TSungmaTable) = %d", sizeof(TSungmaTable));
#endif
	//ADMIN_MANAGER
	sys_log(0, "sizeof(tAdminInfo) = %d * %d ", sizeof(tAdminInfo) * vAdmin.size());
	//END_ADMIN_MANAGER
	sys_log(0, "sizeof(TMonarchInfo) = %d * %d", sizeof(TMonarchInfo));

	peer->EncodeWORD(sizeof(TMobTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_mobTable.size()));
	peer->Encode(&m_vec_mobTable[0], sizeof(TMobTable) * m_vec_mobTable.size());

	peer->EncodeWORD(sizeof(TItemTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_itemTable.size()));
	peer->Encode(&m_vec_itemTable[0], sizeof(TItemTable) * m_vec_itemTable.size());

	peer->EncodeWORD(sizeof(TShopTable));
	peer->EncodeWORD(m_iShopTableSize);
	peer->Encode(m_pShopTable, sizeof(TShopTable) * m_iShopTableSize);

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	peer->EncodeWORD(sizeof(TGemShopTable));
	peer->EncodeWORD(m_iGemShopTableSize);
	peer->Encode(m_pGemShopTable, sizeof(TGemShopTable) * m_iGemShopTableSize);
#endif

	peer->EncodeWORD(sizeof(TSkillTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_skillTable.size()));
	peer->Encode(&m_vec_skillTable[0], sizeof(TSkillTable) * m_vec_skillTable.size());

	peer->EncodeWORD(sizeof(TRefineTable));
	peer->EncodeWORD(m_iRefineTableSize);
	peer->Encode(m_pRefineTable, sizeof(TRefineTable) * m_iRefineTableSize);

	peer->EncodeWORD(sizeof(TItemAttrTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_itemAttrTable.size()));
	peer->Encode(&m_vec_itemAttrTable[0], sizeof(TItemAttrTable) * m_vec_itemAttrTable.size());

	peer->EncodeWORD(sizeof(TItemAttrTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_itemRareTable.size()));
	peer->Encode(&m_vec_itemRareTable[0], sizeof(TItemAttrTable) * m_vec_itemRareTable.size());

	peer->EncodeWORD(sizeof(TBanwordTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_banwordTable.size()));
	peer->Encode(&m_vec_banwordTable[0], sizeof(TBanwordTable) * m_vec_banwordTable.size());

	peer->EncodeWORD(sizeof(building::TLand));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_kLandTable.size()));
	peer->Encode(&m_vec_kLandTable[0], sizeof(building::TLand) * m_vec_kLandTable.size());

	peer->EncodeWORD(sizeof(building::TObjectProto));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_kObjectProto.size()));
	peer->Encode(&m_vec_kObjectProto[0], sizeof(building::TObjectProto) * m_vec_kObjectProto.size());

	peer->EncodeWORD(sizeof(building::TObject));
	peer->EncodeWORD(static_cast<uint16_t>(m_map_pkObjectTable.size()));

	auto it = m_map_pkObjectTable.begin();

	while (it != m_map_pkObjectTable.end())
		peer->Encode((it++)->second, sizeof(building::TObject));

#ifdef ENABLE_EVENT_MANAGER
	peer->EncodeWORD(sizeof(TEventTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_eventTable.size()));
	peer->Encode(&m_vec_eventTable[0], sizeof(TEventTable) * m_vec_eventTable.size());
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	peer->EncodeWORD(sizeof(TBiologMissionsProto));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_BiologMissions.size()));
	peer->Encode(&m_vec_BiologMissions[0], sizeof(TBiologMissionsProto) * m_vec_BiologMissions.size());

	peer->EncodeWORD(sizeof(TBiologRewardsProto));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_BiologRewards.size()));
	peer->Encode(&m_vec_BiologRewards[0], sizeof(TBiologRewardsProto) * m_vec_BiologRewards.size());
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	peer->EncodeWORD(sizeof(TSungmaTable));
	peer->EncodeWORD(static_cast<uint16_t>(m_vec_sungmaTable.size()));
	peer->Encode(&m_vec_sungmaTable[0], sizeof(TSungmaTable) * m_vec_sungmaTable.size());
#endif

	const time_t now = time(0);
	peer->Encode(&now, sizeof(time_t));

	const TItemIDRangeTable itemRange = CItemIDRangeManager::Instance().GetRange();
	const TItemIDRangeTable itemRangeSpare = CItemIDRangeManager::Instance().GetRange();

	peer->EncodeWORD(sizeof(TItemIDRangeTable));
	peer->EncodeWORD(1);
	peer->Encode(&itemRange, sizeof(TItemIDRangeTable));
	peer->Encode(&itemRangeSpare, sizeof(TItemIDRangeTable));

	peer->SetItemIDRange(itemRange);
	peer->SetSpareItemIDRange(itemRangeSpare);

	//ADMIN_MANAGER
	peer->EncodeWORD(16);
	peer->EncodeWORD(static_cast<uint16_t>(vHost.size()));

	for (size_t n = 0; n < vHost.size(); ++n)
	{
		peer->Encode(vHost[n].c_str(), 16);
		sys_log(0, "GMHosts %s", vHost[n].c_str());
	}

	peer->EncodeWORD(sizeof(tAdminInfo));
	peer->EncodeWORD(static_cast<uint16_t>(vAdmin.size()));

	for (size_t n = 0; n < vAdmin.size(); ++n)
	{
		peer->Encode(&vAdmin[n], sizeof(tAdminInfo));
		sys_log(0, "Admin name %s ConntactIP %s", vAdmin[n].m_szName, vAdmin[n].m_szContactIP);
	}
	//END_ADMIN_MANAGER

	//MONARCH
	peer->EncodeWORD(sizeof(TMonarchInfo));
	peer->EncodeWORD(1);
	peer->Encode(CMonarch::Instance().GetMonarch(), sizeof(TMonarchInfo));

	CMonarch::VEC_MONARCHCANDIDACY& rVecMonarchCandidacy = CMonarch::Instance().GetVecMonarchCandidacy();

	const size_t num_monarch_candidacy = CMonarch::Instance().MonarchCandidacySize();
	peer->EncodeWORD(sizeof(MonarchCandidacy));
	peer->EncodeWORD(static_cast<uint16_t>(num_monarch_candidacy));
	if (num_monarch_candidacy != 0)
		peer->Encode(&rVecMonarchCandidacy[0], sizeof(MonarchCandidacy) * num_monarch_candidacy);
	//END_MONARCE

	if (g_test_server)
		sys_log(0, "MONARCHCandidacy Size %d", CMonarch::Instance().MonarchCandidacySize());

	peer->EncodeWORD(0xffff);
}

void CClientManager::SendPartyOnSetup(CPeer* pkPeer)
{
	if (!pkPeer)
		return;

	TPartyMap& pm = m_map_pkChannelParty[pkPeer->GetChannel()];

	for (auto& it_party : pm)
	{
		sys_log(0, "PARTY SendPartyOnSetup Party [%u]", it_party.first);
		pkPeer->EncodeHeader(HEADER_DG_PARTY_CREATE, 0, sizeof(TPacketPartyCreate));
		pkPeer->Encode(&it_party.first, sizeof(uint32_t));

		for (auto& it_member : it_party.second)
		{
			sys_log(0, "PARTY SendPartyOnSetup Party [%u] Member [%u]", it_party.first, it_member.first);
			pkPeer->EncodeHeader(HEADER_DG_PARTY_ADD, 0, sizeof(TPacketPartyAdd));
			pkPeer->Encode(&it_party.first, sizeof(uint32_t));
			pkPeer->Encode(&it_member.first, sizeof(uint32_t));
			pkPeer->Encode(&it_member.second.bRole, sizeof(uint8_t));

			pkPeer->EncodeHeader(HEADER_DG_PARTY_SET_MEMBER_LEVEL, 0, sizeof(TPacketPartySetMemberLevel));
			pkPeer->Encode(&it_party.first, sizeof(uint32_t));
			pkPeer->Encode(&it_member.first, sizeof(uint32_t));
			pkPeer->Encode(&it_member.second.bLevel, sizeof(uint8_t));
		}
	}
}

#ifdef ENABLE_MAILBOX
void CClientManager::QUERY_MAILBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, TMailBox* p)
{
	if (g_log)
		sys_log(0, "QUERY_MAILBOX (handle: %d ch: %s)", dwHandle, p->szName);

	std::vector<SMailBoxTable>* vec = nullptr;
	auto it = m_map_mailbox.find(p->szName);
	if (it != m_map_mailbox.end())
		vec = &it->second;

	if (vec)
	{
		const time_t now = time(nullptr);

		vec->erase(std::remove_if(vec->begin(), vec->end(),
			[now](const TMailBoxTable& mail) { return mail.bIsDeleted || difftime(mail.Message.DeleteTime, now) <= 0; }), vec->end());

		std::sort(vec->begin(), vec->end(), [](const TMailBoxTable& l, const TMailBoxTable& r) {
			return l.Message.SendTime > r.Message.SendTime;
		});
	}

	const uint16_t size = vec ? static_cast<uint16_t>(vec->size()) : 0;
	const uint32_t dwPacketSize = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(SMailBoxTable) * size;

	pkPeer->EncodeHeader(HEADER_DG_RESPOND_MAILBOX_LOAD, dwHandle, dwPacketSize);
	pkPeer->EncodeWORD(sizeof(SMailBoxTable));
	pkPeer->EncodeWORD(size);

	if (vec && vec->empty() == false)
		pkPeer->Encode(&(*vec)[0], sizeof(SMailBoxTable) * size);
}

#include <unordered_set>
void CClientManager::QUERY_MAILBOX_CHECK_NAME(CPeer* pkPeer, uint32_t dwHandle, TMailBox* p)
{
	TMailBox t;
	std::memcpy(t.szName, "", sizeof(t.szName));
	t.Index = 0; // Index: Mail Count

	static std::unordered_set<std::string> NameSet;
	bool bFound = NameSet.find(p->szName) != NameSet.end();

	if (bFound == false)
	{
		char s_szQuery[128];
		snprintf(s_szQuery, sizeof(s_szQuery), "SELECT * FROM player%s WHERE name='%s' LIMIT 1", GetTablePostfix(), p->szName);
		auto pMsg(CDBManager::Instance().DirectQuery(s_szQuery));
		bFound = pMsg->Get()->uiNumRows > 0;
	}

	if (bFound)
	{
		NameSet.emplace(p->szName); // player exists, next time we will use this to avoid using mysql.
		std::memcpy(t.szName, p->szName, sizeof(t.szName));
		auto it = m_map_mailbox.find(p->szName);
		if (it != m_map_mailbox.end())
		{
			const time_t now = time(nullptr);
			for (const SMailBoxTable& mail : it->second)
			{
				if (mail.bIsDeleted)
					continue;

				if (difftime(mail.Message.DeleteTime, now) <= 0)
					continue;

				t.Index++;
			}
		}
	}

	pkPeer->EncodeHeader(HEADER_DG_RESPOND_MAILBOX_CHECK_NAME, dwHandle, sizeof(TMailBox));
	pkPeer->Encode(&t, sizeof(TMailBox));
}

void CClientManager::QUERY_MAILBOX_WRITE(CPeer* pkPeer, uint32_t dwHandle, TMailBoxTable* p)
{
	m_map_mailbox[p->szName].emplace_back(*p);
}

bool CClientManager::GET_MAIL(const char* name, const uint8_t index, SMailBoxTable** mail)
{
	auto it = m_map_mailbox.find(name);
	if (it == m_map_mailbox.end())
		return false;

	MailVec& mailvec = it->second;
	if (index >= mailvec.size())
		return false;

	*mail = &mailvec.at(index);
	return true;
}

void CClientManager::QUERY_MAILBOX_DELETE(CPeer* pkPeer, uint32_t dwHandle, TMailBox* p)
{
	SMailBoxTable* mail = nullptr;
	if (GET_MAIL(p->szName, p->Index, &mail) == false)
		return;

	mail->bIsDeleted = true;
}

void CClientManager::QUERY_MAILBOX_CONFIRM(CPeer* pkPeer, uint32_t dwHandle, TMailBox* p)
{
	SMailBoxTable* mail = nullptr;
	if (GET_MAIL(p->szName, p->Index, &mail) == false)
		return;

	mail->Message.bIsConfirm = true;
}

void CClientManager::QUERY_MAILBOX_GET(CPeer* pkPeer, uint32_t dwHandle, TMailBox* p)
{
	SMailBoxTable* mail = nullptr;
	if (GET_MAIL(p->szName, p->Index, &mail) == false)
		return;

	mail->AddData.iYang = 0;
	mail->AddData.iWon = 0;
	mail->Message.bIsItemExist = false;
	mail->Message.bIsConfirm = true;
	mail->AddData.ItemVnum = 0;
	mail->AddData.ItemCount = 0;
	memset(mail->AddData.alSockets, 0, sizeof(mail->AddData.alSockets));
	memset(mail->AddData.aAttr, 0, sizeof(mail->AddData.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(mail->AddData.aApplyRandom, 0, sizeof(mail->AddData.aApplyRandom));
	memset(mail->AddData.alRandomValues, 0, sizeof(mail->AddData.alRandomValues));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
	mail->AddData.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
	mail->AddData.grade_element = 0;
	memset(mail->AddData.attack_element, 0, sizeof(mail->AddData.attack_element));
	mail->AddData.element_type_bonus = 0;
	memset(mail->AddData.elements_value_bonus, 0, sizeof(mail->AddData.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
	mail->AddData.set_value = 0;
#endif
}

void CClientManager::QUERY_MAILBOX_UNREAD(CPeer* pkPeer, uint32_t dwHandle, TMailBox* p)
{
	auto it = m_map_mailbox.find(p->szName);
	if (it == m_map_mailbox.end())
		return;

	const MailVec& mailvec = it->second;
	if (mailvec.empty())
		return;

	const time_t now = time(nullptr);
	TMailBoxRespondUnreadData t;

	for (const SMailBoxTable& mail : it->second)
	{
		if (mail.bIsDeleted)
			continue;

		if (mail.Message.bIsConfirm)
			continue;

		if (difftime(mail.Message.DeleteTime, now) <= 0)
			continue;

		if (mail.Message.bIsGMPost)
			t.bGMVisible = true;

		if (mail.Message.bIsItemExist)
			t.bItemMessageCount++;
		else
			t.bCommonMessageCount++;
	}

	if ((t.bItemMessageCount + t.bCommonMessageCount) < 1)
		return;

	pkPeer->EncodeHeader(HEADER_DG_RESPOND_MAILBOX_UNREAD, dwHandle, sizeof(TMailBoxRespondUnreadData));
	pkPeer->Encode(&t, sizeof(TMailBoxRespondUnreadData));
}

void CClientManager::MAILBOX_BACKUP()
{
	CDBManager::Instance().DirectQuery("TRUNCATE TABLE player.mailbox");

	if (m_map_mailbox.empty())
		return;

	char s_szQuery[1024];
	const time_t now = time(nullptr);

	for (auto& p : m_map_mailbox)
	{
		auto& mailvec = p.second;
		if (mailvec.empty())
			continue;

		mailvec.erase(std::remove_if(mailvec.begin(), mailvec.end(),
			[now](const TMailBoxTable& mail) { return mail.bIsDeleted || difftime(mail.Message.DeleteTime, now) <= 0; }), mailvec.end());

		std::sort(mailvec.begin(), mailvec.end(), [](const TMailBoxTable& l, const TMailBoxTable& r) {
			return l.Message.SendTime > r.Message.SendTime;
		});

		for (const auto& mail : mailvec)
		{
			snprintf(s_szQuery, sizeof(s_szQuery), "INSERT INTO mailbox%s (name, who, title, message, gm, confirm, send_time, delete_time, gold, won, ivnum, icount, socket0, socket1, socket2, "
#ifdef ENABLE_PROTO_RENEWAL
					"socket3, socket4, socket5, "
#endif
					"attrtype0, attrvalue0, attrtype1, attrvalue1, attrtype2, attrvalue2, attrtype3, attrvalue3, attrtype4, attrvalue4, attrtype5, attrvalue5, attrtype6, attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
					", randomtype0, randomvalue0, randomrow0"
					", randomtype1, randomvalue1, randomrow1"
					", randomtype2, randomvalue2, randomrow2"
					", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
					", transmutation"
#endif
#ifdef ENABLE_REFINE_ELEMENT
					", grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3"
#endif
#ifdef ENABLE_SET_ITEM
					", setItem"
#endif
				") "
				"VALUES('%s', '%s', '%s', '%s', %d, %d, %ld, %ld, %d, %d, %lu, %lu, %d, %d, %d, "
#ifdef ENABLE_PROTO_RENEWAL
				"%d, %d, %d, "
#endif
				"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d"
#ifdef ENABLE_YOHARA_SYSTEM
				", %d, %d, %d"
				", %d, %d, %d"
				", %d, %d, %d"
				", %d, %d, %d, %d"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				", %u"
#endif
#ifdef ENABLE_REFINE_ELEMENT
				", %d, %d, %d, %d, %d, %d, %d, %d"
#endif
#ifdef ENABLE_SET_ITEM
				", %d"
#endif
				")",
				GetTablePostfix(),
				mail.szName, mail.AddData.szFrom, mail.Message.szTitle, mail.AddData.szMessage,
				mail.Message.bIsGMPost, mail.Message.bIsConfirm, mail.Message.SendTime, mail.Message.DeleteTime,
				mail.AddData.iYang, mail.AddData.iWon, mail.AddData.ItemVnum, mail.AddData.ItemCount,
				mail.AddData.alSockets[0], mail.AddData.alSockets[1], mail.AddData.alSockets[2],
#ifdef ENABLE_PROTO_RENEWAL
				mail.AddData.alSockets[3], mail.AddData.alSockets[4], mail.AddData.alSockets[5],
#endif
				mail.AddData.aAttr[0].wType, mail.AddData.aAttr[0].sValue,
				mail.AddData.aAttr[1].wType, mail.AddData.aAttr[1].sValue,
				mail.AddData.aAttr[2].wType, mail.AddData.aAttr[2].sValue,
				mail.AddData.aAttr[3].wType, mail.AddData.aAttr[3].sValue,
				mail.AddData.aAttr[4].wType, mail.AddData.aAttr[4].sValue,
				mail.AddData.aAttr[5].wType, mail.AddData.aAttr[5].sValue,
				mail.AddData.aAttr[6].wType, mail.AddData.aAttr[6].sValue
#ifdef ENABLE_YOHARA_SYSTEM
				, mail.AddData.aApplyRandom[0].wType, mail.AddData.aApplyRandom[0].sValue, mail.AddData.aApplyRandom[0].row
				, mail.AddData.aApplyRandom[1].wType, mail.AddData.aApplyRandom[1].sValue, mail.AddData.aApplyRandom[1].row
				, mail.AddData.aApplyRandom[2].wType, mail.AddData.aApplyRandom[2].sValue, mail.AddData.aApplyRandom[2].row
				, mail.AddData.alRandomValues[0], mail.AddData.alRandomValues[1], mail.AddData.alRandomValues[2], mail.AddData.alRandomValues[3]
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				, mail.AddData.dwTransmutationVnum
#endif
#ifdef ENABLE_REFINE_ELEMENT
				, mail.AddData.grade_element
				, mail.AddData.attack_element[0]
				, mail.AddData.attack_element[1]
				, mail.AddData.attack_element[2]
				, mail.AddData.element_type_bonus
				, mail.AddData.elements_value_bonus[0]
				, mail.AddData.elements_value_bonus[1]
				, mail.AddData.elements_value_bonus[2]
#endif
#ifdef ENABLE_SET_ITEM
				, mail.AddData.set_value
#endif
			);

			auto pInsert(CDBManager::Instance().DirectQuery(s_szQuery));
		}
	}
}
#endif

void CClientManager::QUERY_PLAYER_COUNT(CPeer* pkPeer, const TPlayerCountPacket* pPacket)
{
	if (pkPeer)
		pkPeer->SetUserCount(pPacket->dwCount);
}

void CClientManager::QUERY_QUEST_SAVE(CPeer* pkPeer, TQuestTable* pTable, uint32_t dwLen)
{
	if (!pkPeer)
		return;

	if (0 != (dwLen % sizeof(TQuestTable)))
	{
		sys_err("invalid packet size %d, sizeof(TQuestTable) == %d", dwLen, sizeof(TQuestTable));
		return;
	}

	const int iSize = dwLen / sizeof(TQuestTable);

	char szQuery[1024] = { 0, };

	for (int i = 0; i < iSize; ++i, ++pTable)
	{
		if (pTable->lValue == 0)
		{
			snprintf(szQuery, sizeof(szQuery),
				"DELETE FROM quest%s WHERE dwPID=%d AND szName='%s' AND szState='%s'",
				GetTablePostfix(), pTable->dwPID, pTable->szName, pTable->szState);
		}
		else
		{
			snprintf(szQuery, sizeof(szQuery),
				"REPLACE INTO quest%s (dwPID, szName, szState, lValue) VALUES(%d, '%s', '%s', %ld)",
				GetTablePostfix(), pTable->dwPID, pTable->szName, pTable->szState, pTable->lValue);
		}

		CDBManager::Instance().ReturnQuery(szQuery, QID_QUEST_SAVE, pkPeer->GetHandle(), nullptr);
	}
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CClientManager::QUERY_SAFEBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxLoadPacket* packet, int bMall)
#else
void CClientManager::QUERY_SAFEBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxLoadPacket* packet, bool bMall)
#endif
{
	if (!pkPeer)
		return;

	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	strlcpy(pi->safebox_password, packet->szPassword, sizeof(pi->safebox_password));
	pi->account_id = packet->dwID;
	pi->account_index = 0;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	pi->ip[0] = bMall == 2 ? 2 : (bMall == 1 ? 1 : 0);
#else
	pi->ip[0] = bMall ? 1 : 0;
#endif
	strlcpy(pi->login, packet->szLogin, sizeof(pi->login));

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT account_id, size, password, gold FROM safebox%s WHERE account_id=%u",
		GetTablePostfix(), packet->dwID);

	if (g_log)
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		sys_log(0, "HEADER_GD_SAFEBOX_LOAD (handle: %d account.id %u is_mall %d)", dwHandle, packet->dwID, bMall==1 ? 1 : (bMall == 2 ? 2: 0));
#else
		sys_log(0, "HEADER_GD_SAFEBOX_LOAD (handle: %d account.id %u is_mall %d)", dwHandle, packet->dwID, bMall ? 1 : 0);
#endif

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_LOAD, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_LOAD(CPeer* pkPeer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* pi = (ClientHandleInfo*)qi->pvData;
	const uint32_t dwHandle = pi->dwHandle;

	// The account_index used here refers to the query order.
	// The query to find the first password is 0
	// The second query to get actual data

	if (pi->account_index == 0)
	{
		char szSafeboxPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
		strlcpy(szSafeboxPassword, pi->safebox_password, sizeof(szSafeboxPassword));

		TSafeboxTable* pSafebox = new TSafeboxTable;
		memset(pSafebox, 0, sizeof(TSafeboxTable));

		SQLResult* res = msg->Get();

		if (res->uiNumRows == 0
/*#ifdef ENABLE_GUILDRENEWAL_SYSTEM
			&& (pi->ip[0] != 2)
#endif*/
		)
		{
			if (strcmp("000000", szSafeboxPassword))
			{
				pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle, 0);
				delete pSafebox; //@fixme523
				delete pi;
				return;
			}
		}
		else
		{
			MYSQL_ROW row = mysql_fetch_row(res->pSQLResult);

			// If the password is wrong...
			if ((((!row[2] || !*row[2]) && strcmp("000000", szSafeboxPassword)) ||
				((row[2] && *row[2]) && strcmp(row[2], szSafeboxPassword)))
/*#ifdef ENABLE_GUILDRENEWAL_SYSTEM
				&& (pi->ip[0] != 2)
#endif*/
			)
			{
				pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_WRONG_PASSWORD, dwHandle, 0);
				delete pSafebox; //@fixme523
				delete pi;
				return;
			}

			if (!row[0])
				pSafebox->dwID = 0;
			else
				str_to_number(pSafebox->dwID, row[0]);

			if (!row[1])
				pSafebox->bSize = 0;
			else
				str_to_number(pSafebox->bSize, row[1]);

#ifdef ENABLE_SAFEBOX_MONEY
			pSafebox->dwGold = (row[3] && pi->ip[0] == 0) ? atol(row[3]) : 0;
#endif

			if (pi->ip[0] == 1)
			{
				pSafebox->bSize = 1;
				sys_log(0, "MALL id[%d] size[%d]", pSafebox->dwID, pSafebox->bSize);
			}
			else
				sys_log(0, "SAFEBOX id[%d] size[%d]", pSafebox->dwID, pSafebox->bSize);
		}

		if (0 == pSafebox->dwID)
			pSafebox->dwID = pi->account_id;

		pi->pSafebox = pSafebox;

		char szQuery[QUERY_MAX_LEN] = { 0, };
		snprintf(szQuery, sizeof(szQuery),
			"SELECT id, window+0, pos, count, vnum, "
#ifdef ENABLE_SEALBIND_SYSTEM
			"seal_date, "
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			"transmutation, "
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			"is_basic, "
#endif
#ifdef ENABLE_REFINE_ELEMENT
			"grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3, "
#endif
			"socket0, socket1, socket2, "
#ifdef ENABLE_PROTO_RENEWAL
			"socket3, socket4, socket5, "
#endif
			"attrtype0, attrvalue0, "
			"attrtype1, attrvalue1, "
			"attrtype2, attrvalue2, "
			"attrtype3, attrvalue3, "
			"attrtype4, attrvalue4, "
			"attrtype5, attrvalue5, "
			"attrtype6, attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
			", randomtype0, randomvalue0, randomrow0"
			", randomtype1, randomvalue1, randomrow1"
			", randomtype2, randomvalue2, randomrow2"
			", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_SET_ITEM
			", setItem"
#endif
			" FROM item%s WHERE owner_id=%d AND `window`='%s'",
			GetTablePostfix(), pi->account_id,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			pi->ip[0] == 0 ? "SAFEBOX" : (pi->ip[0] == 2 ? "GUILDBANK" : "MALL")
#else
			pi->ip[0] == 0 ? "SAFEBOX" : "MALL"
#endif
		);

		pi->account_index = 1;

		CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_LOAD, pkPeer->GetHandle(), pi);
	}
	else
	{
		if (!pi->pSafebox)
		{
			sys_err("null safebox pointer!");
			delete pi;
			return;
		}

		// Since there was an error in the query, if you respond, it is as if the warehouse is empty.
		// It's better not to open the warehouse because it can be seen
		if (!msg->Get()->pSQLResult)
		{
			sys_err("null safebox result");
			delete pi;
			return;
		}

		static std::vector<TPlayerItem> s_items;
		CreateItemTableFromRes(msg->Get()->pSQLResult, &s_items, pi->account_id);

		std::set<TItemAward*>* pSet = ItemAwardManager::Instance().GetByLogin(pi->login);

		if (pSet && !m_vec_itemTable.empty())
		{
			CGrid grid(5, MAX(1, pi->pSafebox->bSize) * 9);
			bool bEscape = false;

			for (uint32_t i = 0; i < s_items.size(); ++i)
			{
				const TPlayerItem& r = s_items[i];

				const auto it = m_map_itemTableByVnum.find(r.vnum);

				if (it == m_map_itemTableByVnum.end())
				{
					bEscape = true;
					sys_err("invalid item vnum %u in safebox: login %s", r.vnum, pi->login);
					break;
				}

				grid.Put(r.pos, 1, it->second->bSize);
			}

			if (!bEscape)
			{
				std::vector<std::pair<uint32_t, uint32_t> > vec_dwFinishedAwardID;

				typeof(pSet->begin()) it = pSet->begin();

				char szQuery[512] = { 0, };

				while (it != pSet->end())
				{
					TItemAward* pItemAward = *(it++);
					const uint32_t& dwItemVnum = pItemAward->dwVnum;

					if (pItemAward->bTaken)
						continue;

					if (pi->ip[0] == 0 && pItemAward->bMall)
						continue;

					if (pi->ip[0] == 1 && !pItemAward->bMall)
						continue;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
					if (pi->ip[0] == 2 && pItemAward->bMall)
						continue;
#endif

					const auto item_it = m_map_itemTableByVnum.find(pItemAward->dwVnum);
					if (item_it == m_map_itemTableByVnum.end())
					{
						sys_err("invalid item vnum %u in item_award: login %s", pItemAward->dwVnum, pi->login);
						continue;
					}

					const TItemTable* pItemTable = item_it->second;
					int iPos;
					if ((iPos = grid.FindBlank(1, item_it->second->bSize)) == -1)
						break;

					TPlayerItem item{};

#ifdef ENABLE_EXTEND_ITEM_AWARD
					uint32_t dwSocket2 = pItemAward->dwSocket2;
#else
					uint32_t dwSocket2 = 0;
#endif

					if (pItemTable->bType == ITEM_UNIQUE || (pItemTable->bType == ITEM_RING))
					{
#ifdef ENABLE_EXTEND_ITEM_AWARD
						// 12.04.2019 - Correction for unique items based on the real time.
						const long lValue0 = pItemTable->alValues[ITEM_SOCKET_REMAIN_SEC];
						const long lValue2 = pItemTable->alValues[ITEM_SOCKET_UNIQUE_REMAIN_TIME];
						const time_t tNow = CClientManager::Instance().GetCurrentTime();
						dwSocket2 = (lValue2 == 0) ? static_cast<uint32_t>(lValue0) : static_cast<uint32_t>(tNow + lValue0);
#else
						if (pItemAward->dwSocket2 != 0)
							dwSocket2 = pItemAward->dwSocket2;
						else
							dwSocket2 = pItemTable->alValues[0];
#endif
					}
#ifndef ENABLE_EXTEND_ITEM_AWARD
					else if ((dwItemVnum == 50300 || dwItemVnum == 70037) && pItemAward->dwSocket0 == 0)
					{
						uint32_t dwSkillIdx = 0;
						uint32_t dwSkillVnum = 0;

						do	//@fixme000
						{
							dwSkillIdx = number(0, m_vec_skillTable.size() - 1);
							dwSkillVnum = m_vec_skillTable[dwSkillIdx].dwVnum;
							break;
						} while (dwSkillVnum > 120);

#ifndef ENABLE_EXTEND_ITEM_AWARD
						pItemAward->dwSocket0 = dwSkillVnum;
#endif
					}
#endif
					else
					{
						switch (dwItemVnum)
						{
							case 72723: case 72724: case 72725: case 72726:
							case 72727: case 72728: case 72729: case 72730:
							case 76004: case 76005: case 76021: case 76022: // It's terrifying, but it's scary to fix what you were doing before... So just hard coding. Automatic potion items for gift boxes.
							case 79012: case 79013:
							{
								if (pItemAward->dwSocket2 == 0)
									dwSocket2 = pItemTable->alValues[0];
								else
									dwSocket2 = pItemAward->dwSocket2;
							}
							break;

							default:
								break;
						}
					}

					if (GetItemID() > m_itemRange.dwMax)
					{
						sys_err("UNIQUE ID OVERFLOW!!");
						break;
					}

					{
						const auto item2_it = m_map_itemTableByVnum.find(dwItemVnum);
						if (item2_it == m_map_itemTableByVnum.end())
						{
							sys_err("Invalid item(vnum : %d). It is not in m_map_itemTableByVnum.", dwItemVnum);
							continue;
						}
						const TItemTable* item_table = item2_it->second;
						if (item_table == nullptr)
						{
							sys_err("Invalid item_table (vnum : %d). It's value is nullptr in m_map_itemTableByVnum.", dwItemVnum);
							continue;
						}
						if (0 == pItemAward->dwSocket0)
						{
							for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
							{
								if (LIMIT_REAL_TIME == item_table->aLimits[i].bType)
								{
									if (0 == item_table->aLimits[i].lValue)
										pItemAward->dwSocket0 = time(0) + 60 * 60 * 24 * 7;
									else
										pItemAward->dwSocket0 = time(0) + item_table->aLimits[i].lValue;

									break;
								}
								else if (LIMIT_REAL_TIME_START_FIRST_USE == item_table->aLimits[i].bType || LIMIT_TIMER_BASED_ON_WEAR == item_table->aLimits[i].bType)
								{
									if (0 == item_table->aLimits[i].lValue)
										pItemAward->dwSocket0 = 60 * 60 * 24 * 7;
									else
										pItemAward->dwSocket0 = item_table->aLimits[i].lValue;

									break;
								}
							}
						}

#ifdef ENABLE_EXTEND_ITEM_AWARD
						ItemAwardManager::Instance().CheckItemCount(*pItemAward, *pItemTable);
						ItemAwardManager::Instance().CheckItemBlend(*pItemAward, *pItemTable);
						ItemAwardManager::Instance().CheckItemAddonType(*pItemAward, *pItemTable);
						ItemAwardManager::Instance().CheckItemSkillBook(*pItemAward, m_vec_skillTable);
#ifdef USE_ITEM_AWARD_CHECK_ATTRIBUTES
						ItemAwardManager::Instance().CheckItemAttributes(*pItemAward, *pItemTable, m_vec_itemAttrTable);
#endif

						// START_OF_AUTO_QUERY
						char szColumns[QUERY_MAX_LEN], szValues[QUERY_MAX_LEN];

						int	iLen = snprintf(szColumns, sizeof(szColumns), "id, owner_id, window, pos, vnum, count");
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
						int	iValueLen = snprintf(szValues, sizeof(szValues), "%u, %u, '%s', %d, %u, %u", GainItemID(), pi->account_id, (pi->ip[0] == 0) ? "SAFEBOX" : ((pi->ip[0] == 2) ? "GUILDBANK" : "MALL"), iPos, pItemAward->dwVnum, pItemAward->dwCount);
#else
						int	iValueLen = snprintf(szValues, sizeof(szValues), "%u, %u, '%s', %d, %u, %u", GainItemID(), pi->account_id, (pi->ip[0] == 0) ? "SAFEBOX" : "MALL", iPos, pItemAward->dwVnum, pItemAward->dwCount);
#endif

						iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", socket0, socket1, socket2");
						iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %u, %u, %u", pItemAward->dwSocket0, pItemAward->dwSocket1, dwSocket2);

						for (size_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
						{
							iLen += snprintf(szColumns + iLen, sizeof(szColumns) - iLen, ", attrtype%d, attrvalue%d", i, i);
							iValueLen += snprintf(szValues + iValueLen, sizeof(szValues) - iValueLen, ", %d, %d", pItemAward->aAttr[i].wType, pItemAward->aAttr[i].sValue);
						}
						// END_OF_AUTO_QUERY

						snprintf(szQuery, sizeof(szQuery), "INSERT INTO item%s (%s) VALUES(%s)", GetTablePostfix(), szColumns, szValues);
#else
						snprintf(szQuery, sizeof(szQuery),
								"INSERT INTO item%s (id, owner_id, `window`, pos, vnum, count, socket0, socket1, socket2) "
								"VALUES(%u, %u, '%s', %d, %u, %u, %u, %u, %u, 0)",
								GetTablePostfix(),
								GainItemID(),
								pi->account_id,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
								pi->ip[0] == 0 ? "SAFEBOX" : (pi->ip[0] == 2 ? "GUILDBANK" : "MALL"),
#else
								pi->ip[0] == 0 ? "SAFEBOX" : "MALL",
#endif
								iPos,
								pItemAward->dwVnum, pItemAward->dwCount, pItemAward->dwSocket0, pItemAward->dwSocket1, dwSocket2);
#endif
					}

					auto pmsg(CDBManager::Instance().DirectQuery(szQuery));
					const SQLResult* pRes = pmsg->Get();
					sys_log(0, "SAFEBOX Query : [%s]", szQuery);

					if (pRes->uiAffectedRows == 0 || pRes->uiInsertID == 0 || pRes->uiAffectedRows == (uint32_t)-1)
						break;

					item.id = static_cast<uint32_t>(pmsg->Get()->uiInsertID);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
					item.window = pi->ip[0] == 0 ? SAFEBOX : (pi->ip[0] == 2 ? GUILDBANK : MALL);
#else
					item.window = pi->ip[0] == 0 ? SAFEBOX : MALL,
#endif
					item.pos = iPos;
					item.count = pItemAward->dwCount;
					item.vnum = pItemAward->dwVnum;
					item.alSockets[0] = pItemAward->dwSocket0;
					item.alSockets[1] = pItemAward->dwSocket1;
					item.alSockets[2] = dwSocket2;
#ifdef ENABLE_EXTEND_ITEM_AWARD
					thecore_memcpy(&item.aAttr, pItemAward->aAttr, sizeof(item.aAttr));
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
					memset(&item.aPetInfo, 0, sizeof(item.aPetInfo));
#endif
					s_items.emplace_back(item);

					vec_dwFinishedAwardID.emplace_back(std::make_pair(pItemAward->dwID, item.id));
					grid.Put(iPos, 1, item_it->second->bSize);
				}

				for (uint32_t i = 0; i < vec_dwFinishedAwardID.size(); ++i)
					ItemAwardManager::Instance().Taken(vec_dwFinishedAwardID[i].first, vec_dwFinishedAwardID[i].second);
			}
		}

		pi->pSafebox->wItemCount = static_cast<uint16_t>(s_items.size());

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		pkPeer->EncodeHeader(pi->ip[0] == 0 ? HEADER_DG_SAFEBOX_LOAD : (pi->ip[0] == 2 ? HEADER_DG_GUILDSTORAGE_LOAD : HEADER_DG_MALL_LOAD), dwHandle, sizeof(TSafeboxTable) + sizeof(TPlayerItem) * s_items.size());
#else
		pkPeer->EncodeHeader(pi->ip[0] == 0 ? HEADER_DG_SAFEBOX_LOAD : HEADER_DG_MALL_LOAD, dwHandle, sizeof(TSafeboxTable) + sizeof(TPlayerItem) * s_items.size());
#endif

		pkPeer->Encode(pi->pSafebox, sizeof(TSafeboxTable));

#ifdef ENABLE_GROWTH_PET_SYSTEM
		if (!s_items.empty())
		{
			//for (auto petItem : s_items)
			for (uint32_t i = 0; i < s_items.size(); ++i)
			{
				const TItemTable* item_table = CClientManager::Instance().GetItemTableByVnum(s_items[i].vnum);
				if (item_table && item_table->bType == ITEM_PET && ((item_table->bSubType == PET_UPBRINGING) || (item_table->bSubType == PET_BAG)))
				{

					char szGrowthPetQuery[QUERY_MAX_LEN], szGrowthPetColumns[QUERY_MAX_LEN];

					snprintf(szGrowthPetColumns, sizeof(szGrowthPetColumns),
						"id, pet_vnum, pet_nick, pet_level, evol_level, pet_type, pet_hp, pet_def, pet_sp, "
						"pet_duration, pet_birthday, exp_monster, exp_item, skill_count, "
						"pet_skill1, pet_skill_level1, pet_skill_spec1, pet_skill_cool1, "
						"pet_skill2, pet_skill_level2, pet_skill_spec2, pet_skill_cool2, "
						"pet_skill3, pet_skill_level3, pet_skill_spec3, pet_skill_cool3");

					snprintf(szGrowthPetQuery, sizeof(szGrowthPetQuery),
						"SELECT %s FROM pet%s WHERE id=%lu;",
						szGrowthPetColumns, GetTablePostfix(), s_items[i].alSockets[2]);

					auto msgGrowthPet(CDBManager::Instance().DirectQuery(szGrowthPetQuery));
					const SQLResult* resGrowthPet = msgGrowthPet->Get();

					if (resGrowthPet && resGrowthPet->uiNumRows > 0)
					{
						for (size_t k = 0; k < resGrowthPet->uiNumRows; ++k)
						{
							const MYSQL_ROW row = mysql_fetch_row(resGrowthPet->pSQLResult);
							if (!row || !row[0])
								continue;

							int col = 0;

							TGrowthPetInfo table = {};
							str_to_number(table.pet_id, row[col++]);
							str_to_number(table.pet_vnum, row[col++]);
							strlcpy(table.pet_nick, row[col++], sizeof(table.pet_nick));
							str_to_number(table.pet_level, row[col++]);
							str_to_number(table.evol_level, row[col++]);
							str_to_number(table.pet_type, row[col++]);
							str_to_number(table.pet_hp, row[col++]);
							str_to_number(table.pet_def, row[col++]);
							str_to_number(table.pet_sp, row[col++]);
							str_to_number(table.pet_max_time, row[col++]);
							str_to_number(table.pet_birthday, row[col++]);
							str_to_number(table.exp_monster, row[col++]);
							str_to_number(table.exp_item, row[col++]);
							str_to_number(table.skill_count, row[col++]);
							str_to_number(table.skill_vnum[0], row[col++]);
							str_to_number(table.skill_level[0], row[col++]);
							str_to_number(table.skill_spec[0], row[col++]);
							str_to_number(table.skill_cool[0], row[col++]);
							str_to_number(table.skill_vnum[1], row[col++]);
							str_to_number(table.skill_level[1], row[col++]);
							str_to_number(table.skill_spec[1], row[col++]);
							str_to_number(table.skill_cool[1], row[col++]);
							str_to_number(table.skill_vnum[2], row[col++]);
							str_to_number(table.skill_level[2], row[col++]);
							str_to_number(table.skill_spec[2], row[col++]);
							str_to_number(table.skill_cool[2], row[col++]);

							thecore_memcpy(&s_items[i].aPetInfo, &table, sizeof(TGrowthPetInfo));
						}
					}
				}
			}
		}
#endif

		if (!s_items.empty())
			pkPeer->Encode(&s_items[0], sizeof(TPlayerItem) * s_items.size());

		delete pi;
	}
}

void CClientManager::QUERY_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxChangeSizePacket* p)
{
	if (!pkPeer)
		return;

	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	pi->account_index = p->bSize; // Temporarily use account_index as the size

	char szQuery[QUERY_MAX_LEN] = { 0, };

	if (p->bSize == 1)
		snprintf(szQuery, sizeof(szQuery), "REPLACE INTO safebox%s (account_id, size) VALUES(%u, %u)", GetTablePostfix(), p->dwID, p->bSize);
	else
		snprintf(szQuery, sizeof(szQuery), "UPDATE safebox%s SET size=%u WHERE account_id=%u", GetTablePostfix(), p->bSize, p->dwID);

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_SIZE, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg)
{
	if (!pkPeer || !msg)
		return;

	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	const uint32_t dwHandle = p->dwHandle;
	const uint8_t bSize = p->account_index;

	delete p;

	if (msg->Get()->uiNumRows > 0)
	{
		pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_SIZE, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bSize);
	}
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CClientManager::QUERY_GUILDSTORAGE_CHANGE_SIZE(CPeer* pkPeer, uint32_t dwHandle, TGuildstorageChangeSizePacket* p)
{
	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	pi->account_index = p->bSize;

	char szQuery[QUERY_MAX_LEN];

	if (p->bSize == 1)
		snprintf(szQuery, sizeof(szQuery), "INSERT INTO guild%s (id, guildstorage) VALUES(%u, %u)", GetTablePostfix(), p->dwID, p->bSize);
	else
		snprintf(szQuery, sizeof(szQuery), "UPDATE guild%s SET guildstorage=%u WHERE id=%u", GetTablePostfix(), p->bSize, p->dwID);

	CDBManager::Instance().ReturnQuery(szQuery, QID_GUILDSTORAGE_CHANGE_SIZE, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_GUILDSTORAGE_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	uint32_t dwHandle = p->dwHandle;
	uint8_t bSize = p->account_index;

	delete p;

	if (msg->Get()->uiNumRows > 0)
	{
		pkPeer->EncodeHeader(HEADER_DG_GUILDSTORAGE_CHANGE_SIZE, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bSize);
	}
}
#endif

void CClientManager::QUERY_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxChangePasswordPacket* p)
{
	if (!pkPeer)
		return;

	ClientHandleInfo* pi = new ClientHandleInfo(dwHandle);
	strlcpy(pi->safebox_password, p->szNewPassword, sizeof(pi->safebox_password));
	strlcpy(pi->login, p->szOldPassword, sizeof(pi->login));
	pi->account_id = p->dwID;

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "SELECT password FROM safebox%s WHERE account_id=%u", GetTablePostfix(), p->dwID);

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD, pkPeer->GetHandle(), pi);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, SQLMsg* msg)
{
	if (!pkPeer || !msg)
		return;

	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	const uint32_t dwHandle = p->dwHandle;

	if (msg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(msg->Get()->pSQLResult);

		if ((row[0] && *row[0] && !strcasecmp(row[0], p->login)) || ((!row[0] || !*row[0]) && !strcmp("000000", p->login)))
		{
			char szQuery[QUERY_MAX_LEN];
			char escape_pwd[64];
			CDBManager::Instance().EscapeString(escape_pwd, p->safebox_password, strlen(p->safebox_password));

			snprintf(szQuery, sizeof(szQuery), "UPDATE safebox%s SET password='%s' WHERE account_id=%u", GetTablePostfix(), escape_pwd, p->account_id);

			CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD_SECOND, pkPeer->GetHandle(), p);
			return;
		}
	}
	else
	{
		char szQuery[QUERY_MAX_LEN];
		char escape_pwd[64];
		CDBManager::Instance().EscapeString(escape_pwd, p->safebox_password, strlen(p->safebox_password));

		snprintf(szQuery, sizeof(szQuery), "INSERT INTO safebox%s SET size=1, account_id=%u, password='%s'", GetTablePostfix(), p->account_id, escape_pwd);

		CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_CHANGE_PASSWORD_SECOND, pkPeer->GetHandle(), p);
		return;
	}

	delete p;

	// Wrong old password
	pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle, sizeof(uint8_t));
	pkPeer->EncodeBYTE(0);
}

void CClientManager::RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer* pkPeer, SQLMsg* msg)
{
	if (!pkPeer)
		return;

	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	ClientHandleInfo* p = (ClientHandleInfo*)qi->pvData;
	const uint32_t dwHandle = p->dwHandle;
	delete p;

	pkPeer->EncodeHeader(HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER, dwHandle, sizeof(uint8_t));
	pkPeer->EncodeBYTE(1);
}

#ifdef ENABLE_MOVE_CHANNEL
void CClientManager::QUERY_MOVE_CHANNEL(CPeer* pkPeer, uint32_t dwHandle, TMoveChannel* data)
{
	auto it = std::find_if(m_peerList.begin(), m_peerList.end(), [pkPeer, data](CPeer* p) {
		return (p != pkPeer && p->GetChannel() == data->bChannel && p->CheckMapIndex(data->lMapIndex));
	});

	TRespondMoveChannel t{};

	if (it != m_peerList.end())
	{
		t.lAddr = (*it)->GetAddr();
		t.wPort = (*it)->GetListenPort();
	}

	pkPeer->EncodeHeader(HEADER_DG_RESPOND_MOVE_CHANNEL, dwHandle, sizeof(TRespondMoveChannel));
	pkPeer->Encode(&t, sizeof(t));
}
#endif

// MYSHOP_PRICE_LIST
void CClientManager::RESULT_PRICELIST_LOAD(CPeer* peer, SQLMsg* pMsg)
{
	if (!peer || !pMsg)
		return;

	TItemPricelistReqInfo* pReqInfo = (TItemPricelistReqInfo*)static_cast<CQueryInfo*>(pMsg->pvUserData)->pvData;

	//
	// Save the information loaded from DB in the cache
	//

	TItemPriceListTable table;
	table.dwOwnerID = pReqInfo->second;
	table.byCount = 0;

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		str_to_number(table.aPriceInfo[table.byCount].dwVnum, row[0]);
#ifdef ENABLE_CHEQUE_SYSTEM
		str_to_number(table.aPriceInfo[table.byCount].price.dwPrice, row[1]);
		str_to_number(table.aPriceInfo[table.byCount].price.dwCheque, row[2]);
#else
		str_to_number(table.aPriceInfo[table.byCount].dwPrice, row[1]);
#endif
		table.byCount++;
	}

	PutItemPriceListCache(&table);

	//
	// Transfer the loaded data to the game server
	//

	TPacketMyshopPricelistHeader header{};

	header.dwOwnerID = pReqInfo->second;
	header.byCount = table.byCount;

	const size_t sizePriceListSize = sizeof(TItemPriceInfo) * header.byCount;

	peer->EncodeHeader(HEADER_DG_MYSHOP_PRICELIST_RES, pReqInfo->first, sizeof(header) + sizePriceListSize);
	peer->Encode(&header, sizeof(header));
	peer->Encode(table.aPriceInfo, sizePriceListSize);

	sys_log(0, "Load MyShopPricelist handle[%d] pid[%d] count[%d]", pReqInfo->first, pReqInfo->second, header.byCount);

	delete pReqInfo;
}

void CClientManager::RESULT_PRICELIST_LOAD_FOR_UPDATE(SQLMsg* pMsg)
{
	if (!pMsg)
		return;

	TItemPriceListTable* pUpdateTable = (TItemPriceListTable*)static_cast<CQueryInfo*>(pMsg->pvUserData)->pvData;

	//
	// Save the information loaded from DB in the cache
	//

	TItemPriceListTable table;
	table.dwOwnerID = pUpdateTable->dwOwnerID;
	table.byCount = 0;

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		str_to_number(table.aPriceInfo[table.byCount].dwVnum, row[0]);
#ifdef ENABLE_CHEQUE_SYSTEM
		str_to_number(table.aPriceInfo[table.byCount].price.dwPrice, row[1]);
		str_to_number(table.aPriceInfo[table.byCount].price.dwCheque, row[2]);
#else
		str_to_number(table.aPriceInfo[table.byCount].dwPrice, row[1]);
#endif
		table.byCount++;
	}

	PutItemPriceListCache(&table);

	// Update cache
	GetItemPriceListCache(pUpdateTable->dwOwnerID)->UpdateList(pUpdateTable);

	delete pUpdateTable;
}
// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_SAFEBOX_MONEY
void CClientManager::QUERY_SAFEBOX_SAVE(CPeer* pkPeer, const TSafeboxTable* pTable)
{
	if (!pkPeer)
		return;

	char szQuery[QUERY_MAX_LEN];
	snprintf(szQuery, sizeof(szQuery), "UPDATE safebox%s SET gold='%u' WHERE account_id=%u", GetTablePostfix(), pTable->dwGold, pTable->dwID);

	CDBManager::Instance().ReturnQuery(szQuery, QID_SAFEBOX_SAVE, pkPeer->GetHandle(), nullptr);
}
#endif

void CClientManager::QUERY_EMPIRE_SELECT(CPeer* pkPeer, uint32_t dwHandle, const TEmpireSelectPacket* p)
{
	if (!pkPeer)
		return;

	char szQuery[QUERY_MAX_LEN];

	snprintf(szQuery, sizeof(szQuery), "UPDATE player_index%s SET empire=%u WHERE id=%u", GetTablePostfix(), p->bEmpire, p->dwAccountID);
	CDBManager::Instance().DirectQuery(szQuery);

	sys_log(0, "EmpireSelect: %s", szQuery);
	{
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
		snprintf(szQuery, sizeof(szQuery),
			"SELECT pid1, pid2, pid3, pid4, pid5 FROM player_index%s WHERE id=%u", GetTablePostfix(), p->dwAccountID);
#else
		snprintf(szQuery, sizeof(szQuery),
			"SELECT pid1, pid2, pid3, pid4 FROM player_index%s WHERE id=%u", GetTablePostfix(), p->dwAccountID);
#endif

		auto pmsg(CDBManager::Instance().DirectQuery(szQuery));

		SQLResult* pRes = pmsg->Get();

		if (pRes->uiNumRows)
		{
			sys_log(0, "EMPIRE %lu", pRes->uiNumRows);

			MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
			uint32_t pids[3];

			const uint32_t g_start_map[4] =
			{
				0, // reserved
				1, // Shinsoo
				21, // Chunjo
				41 // Jinno
			};

			// FIXME share with game
			const uint32_t g_start_position[4][2] =
			{
				{ 0, 0 },
				{ 469300, 964200 }, // Shinsoo
				{ 55700, 157900 }, // Chunjo
				{ 969600, 278400 } // Jinno
			};

			for (int i = 0; i < 3; ++i)
			{
				str_to_number(pids[i], row[i]);
				sys_log(0, "EMPIRE PIDS[%d]", pids[i]);

				if (pids[i])
				{
					sys_log(0, "EMPIRE move to pid[%d] to villiage of %u, map_index %d",
						pids[i], p->bEmpire, g_start_map[p->bEmpire]);

					snprintf(szQuery, sizeof(szQuery), "UPDATE player%s SET map_index=%u,x=%u,y=%u WHERE id=%u",
						GetTablePostfix(),
						g_start_map[p->bEmpire],
						g_start_position[p->bEmpire][0],
						g_start_position[p->bEmpire][1],
						pids[i]);

					auto pmsg2(CDBManager::Instance().DirectQuery(szQuery));
				}
			}
		}
	}

	pkPeer->EncodeHeader(HEADER_DG_EMPIRE_SELECT, dwHandle, sizeof(uint8_t));
	pkPeer->EncodeBYTE(p->bEmpire);
}

void CClientManager::QUERY_SETUP(CPeer* peer, uint32_t dwHandle, const char* c_pData)
{
	TPacketGDSetup* p = (TPacketGDSetup*)c_pData;
	c_pData += sizeof(TPacketGDSetup);

	if (p->bAuthServer)
	{
		sys_log(0, "AUTH_PEER ptr %p", peer);

		m_pkAuthPeer = peer;
		return;
	}

	peer->SetPublicIP(p->szPublicIP);
#ifdef ENABLE_MOVE_CHANNEL
	peer->SetAddr(inet_addr(p->szPublicIP));
#endif
	peer->SetChannel(p->bChannel);
	peer->SetListenPort(p->wListenPort);
	peer->SetP2PPort(p->wP2PPort);
	peer->SetMaps(p->alMaps);

	//
	// Send which map is on which server
	//
	TMapLocation kMapLocations{};

	strlcpy(kMapLocations.szHost, peer->GetPublicIP(), sizeof(kMapLocations.szHost));
	kMapLocations.wPort = peer->GetListenPort();
	thecore_memcpy(kMapLocations.alMaps, peer->GetMaps(), sizeof(kMapLocations.alMaps));

	uint8_t bMapCount = 0;

	std::vector<TMapLocation> vec_kMapLocations;

	if (peer->GetChannel() == 1)
	{
		for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i)
		{
			CPeer* tmp = *i;

			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == GUILD_WARP_WAR_CHANNEL || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2{};
				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				thecore_memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));
				vec_kMapLocations.emplace_back(kMapLocation2);

				tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation));
				bMapCount = 1;
				tmp->EncodeBYTE(bMapCount);
				tmp->Encode(&kMapLocations, sizeof(TMapLocation));
			}
		}
	}
	else if (peer->GetChannel() == GUILD_WARP_WAR_CHANNEL)
	{
		for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i)
		{
			CPeer* tmp = *i;

			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == 1 || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2{};
				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				thecore_memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));
				vec_kMapLocations.emplace_back(kMapLocation2);
			}

			tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation));
			bMapCount = 1;
			tmp->EncodeBYTE(bMapCount);
			tmp->Encode(&kMapLocations, sizeof(TMapLocation));
		}
	}
	else
	{
		for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i)
		{
			CPeer* tmp = *i;

			if (tmp == peer)
				continue;

			if (!tmp->GetChannel())
				continue;

			if (tmp->GetChannel() == GUILD_WARP_WAR_CHANNEL || tmp->GetChannel() == peer->GetChannel())
			{
				TMapLocation kMapLocation2{};

				strlcpy(kMapLocation2.szHost, tmp->GetPublicIP(), sizeof(kMapLocation2.szHost));
				kMapLocation2.wPort = tmp->GetListenPort();
				thecore_memcpy(kMapLocation2.alMaps, tmp->GetMaps(), sizeof(kMapLocation2.alMaps));

				vec_kMapLocations.emplace_back(kMapLocation2);
			}

			if (tmp->GetChannel() == peer->GetChannel())
			{
				tmp->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation));
				bMapCount = 1;
				tmp->EncodeBYTE(bMapCount);
				tmp->Encode(&kMapLocations, sizeof(TMapLocation));
			}
		}
	}

	vec_kMapLocations.emplace_back(kMapLocations);

	peer->EncodeHeader(HEADER_DG_MAP_LOCATIONS, 0, sizeof(uint8_t) + sizeof(TMapLocation) * vec_kMapLocations.size());
	bMapCount = static_cast<uint8_t>(vec_kMapLocations.size());
	peer->EncodeBYTE(bMapCount);
	peer->Encode(&vec_kMapLocations[0], sizeof(TMapLocation) * vec_kMapLocations.size());

	//
	// Setup: Make other peers connect to the connected peer. (P2P connection creation)
	//
	sys_log(0, "SETUP: channel %u listen %u p2p %u count %u", peer->GetChannel(), p->wListenPort, p->wP2PPort, bMapCount);

	TPacketDGP2P p2pSetupPacket{};
	p2pSetupPacket.wPort = peer->GetP2PPort();
	p2pSetupPacket.bChannel = peer->GetChannel();
	strlcpy(p2pSetupPacket.szHost, peer->GetPublicIP(), sizeof(p2pSetupPacket.szHost));

	for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (tmp == peer)
			continue;

		// If the channel is 0, it can be considered a peer or auth that has not yet received a SETUP packet.
		if (0 == tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_P2P, 0, sizeof(TPacketDGP2P));
		tmp->Encode(&p2pSetupPacket, sizeof(TPacketDGP2P));
	}

	TPacketLoginOnSetup* pck = (TPacketLoginOnSetup*)c_pData;

	for (uint32_t c = 0; c < p->dwLoginCount; ++c, ++pck)
	{
		CLoginData* pkLD = new CLoginData;

		pkLD->SetKey(pck->dwLoginKey);
		pkLD->SetClientKey(pck->adwClientKey);
		pkLD->SetIP(pck->szHost);

		TAccountTable& r = pkLD->GetAccountRef();

		r.id = pck->dwID;
		trim_and_lower(pck->szLogin, r.login, sizeof(r.login));
		strlcpy(r.social_id, pck->szSocialID, sizeof(r.social_id));
		strlcpy(r.passwd, "TEMP", sizeof(r.passwd));
#ifdef ENABLE_HWID_BAN
		strlcpy(r.hwid, "TEMP2", sizeof(r.hwid));
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		r.bLanguage = pck->bLanguage;
#endif

		InsertLoginData(pkLD);

		if (InsertLogonAccount(pck->szLogin, peer->GetHandle(), pck->szHost))
		{
			sys_log(0, "SETUP: login %u %s login_key %u host %s", pck->dwID, pck->szLogin, pck->dwLoginKey, pck->szHost);
			pkLD->SetPlay(true);
		}
		else
			sys_log(0, "SETUP: login_fail %u %s login_key %u", pck->dwID, pck->szLogin, pck->dwLoginKey);
	}

	SendPartyOnSetup(peer);
	CGuildManager::Instance().OnSetup(peer);
	CPrivManager::Instance().SendPrivOnSetup(peer);
	SendEventFlagsOnSetup(peer);
	marriage::CManager::Instance().OnSetup(peer);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	SendShops(peer);
#endif
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CClientManager::QUERY_SKILL_COLOR_SAVE(const char* c_pData)
{
	PutSkillColorCache((TSkillColor*)c_pData);
}
#endif

void CClientManager::QUERY_ITEM_FLUSH(CPeer* pkPeer, const char* c_pData)
{
	const uint32_t dwID = *(uint32_t*)c_pData;

	if (g_log)
		sys_log(0, "HEADER_GD_ITEM_FLUSH: %u", dwID);

	CItemCache* c = GetItemCache(dwID);

	if (c)
		c->Flush();
}

void CClientManager::QUERY_ITEM_SAVE(CPeer* pkPeer, const char* c_pData)
{
	if (!pkPeer)
		return;

	TPlayerItem* p = (TPlayerItem*)c_pData;

	// If you don't cache the warehouse, you have to remove what was in the cache.

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	bool intoCache = true;
#endif
	if (p->window == SAFEBOX || p->window == MALL
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| p->window == GUILDBANK
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| p->window == PREMIUM_PRIVATE_SHOP
#endif
		)
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		//In the case of 'PREMIUM_PRIVATE_SHOP' items we just need them to be available on the database right away.
		//They SHOULD still be in the cache so that the player can fetch them.
		if (p->window != PREMIUM_PRIVATE_SHOP)
#endif
		{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			intoCache = false;
#endif
			CItemCache* c = GetItemCache(p->id);
			if (c)
			{
				const TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

				if (it != m_map_pkItemCacheSetPtr.end())
				{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
					if (g_test_server)
						sys_log(0, "ITEM_CACHE: window %d owner %u id %u", p->window, c->Get()->owner, c->Get()->id);
#else
					if (g_test_server)
						sys_log(0, "ITEM_CACHE: safebox owner %u id %u", c->Get()->owner, c->Get()->id);
#endif

					it->second->erase(c);
				}

				m_map_itemCache.erase(p->id);

				delete c;
			}
		}

		char szQuery[QUERY_MAX_LEN];
		snprintf(szQuery, sizeof(szQuery),
			"REPLACE INTO item%s (id, owner_id, window, pos, count, vnum, "
#ifdef ENABLE_SEALBIND_SYSTEM
			"seal_date, "
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			"transmutation, "
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			"is_basic, "
#endif
#ifdef ENABLE_REFINE_ELEMENT
			"grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3, "
#endif
			"socket0, socket1, socket2, "
#ifdef ENABLE_PROTO_RENEWAL
			"socket3, socket4, socket5, "
#endif
			"attrtype0, attrvalue0, "
			"attrtype1, attrvalue1, "
			"attrtype2, attrvalue2, "
			"attrtype3, attrvalue3, "
			"attrtype4, attrvalue4, "
			"attrtype5, attrvalue5, "
			"attrtype6, attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
			", randomtype0, randomvalue0, randomrow0"
			", randomtype1, randomvalue1, randomrow1"
			", randomtype2, randomvalue2, randomrow2"
			", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_SET_ITEM
			", setItem"
#endif
			") "

			"VALUES(%u, %u, %d, %d, %u, %u, "
#ifdef ENABLE_SEALBIND_SYSTEM
			"%ld, "
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			"%u, "
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			"%d, "
#endif
#ifdef ENABLE_REFINE_ELEMENT
			"%d, %d, %d, %d, %d, %d, %d, %d, "
#endif
			"%ld, %ld, %ld, "	// socket0, socket1, socket2
#ifdef ENABLE_PROTO_RENEWAL
			"%ld, %ld, %ld, "	// socket3, socket4, socket5
#endif
			"%d, %d, "	// attrtype0, attrvalue0
			"%d, %d, "	// attrtype1, attrvalue1
			"%d, %d, "	// attrtype2, attrvalue2
			"%d, %d, "	// attrtype3, attrvalue3
			"%d, %d, "	// attrtype4, attrvalue4
			"%d, %d, "	// attrtype5, attrvalue5
			"%d, %d"	// attrtype6, attrvalue6
#ifdef ENABLE_YOHARA_SYSTEM
			", %d, %d, %d"
			", %d, %d, %d"
			", %d, %d, %d"
			", %d, %d, %d, %d"
#endif
#ifdef ENABLE_SET_ITEM
			", %d"
#endif
			")",
			GetTablePostfix(),
			p->id,
			p->owner,
			p->window,
			p->pos,
			p->count,
			p->vnum,
#ifdef ENABLE_SEALBIND_SYSTEM
			p->nSealDate,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			p->dwTransmutationVnum,
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			p->is_basic,
#endif
#ifdef ENABLE_REFINE_ELEMENT
			p->grade_element,
			p->attack_element[0],
			p->attack_element[1],
			p->attack_element[2],
			p->element_type_bonus,
			p->elements_value_bonus[0],
			p->elements_value_bonus[1],
			p->elements_value_bonus[2],
#endif
			p->alSockets[0],
			p->alSockets[1],
			p->alSockets[2],
#ifdef ENABLE_PROTO_RENEWAL
			p->alSockets[3],
			p->alSockets[4],
			p->alSockets[5],
#endif
			p->aAttr[0].wType, p->aAttr[0].sValue,
			p->aAttr[1].wType, p->aAttr[1].sValue,
			p->aAttr[2].wType, p->aAttr[2].sValue,
			p->aAttr[3].wType, p->aAttr[3].sValue,
			p->aAttr[4].wType, p->aAttr[4].sValue,
			p->aAttr[5].wType, p->aAttr[5].sValue,
			p->aAttr[6].wType, p->aAttr[6].sValue
#ifdef ENABLE_YOHARA_SYSTEM
			, p->aApplyRandom[0].wType, p->aApplyRandom[0].sValue, p->aApplyRandom[0].row
			, p->aApplyRandom[1].wType, p->aApplyRandom[1].sValue, p->aApplyRandom[1].row
			, p->aApplyRandom[2].wType, p->aApplyRandom[2].sValue, p->aApplyRandom[2].row
			, p->alRandomValues[0]
			, p->alRandomValues[1]
			, p->alRandomValues[2]
			, p->alRandomValues[3]
#endif
#ifdef ENABLE_SET_ITEM
			, p->set_value
#endif
		);

		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_SAVE, pkPeer->GetHandle(), nullptr);
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (intoCache)
#else
	else
#endif
	{
		if (g_test_server)
			sys_log(0, "QUERY_ITEM_SAVE => PutItemCache() owner %d id %d vnum %d ", p->owner, p->id, p->vnum);

		PutItemCache(p);
	}
}

CClientManager::TItemCacheSet* CClientManager::GetItemCacheSet(uint32_t pid)
{
	const TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
		return nullptr;

	return it->second;
}

void CClientManager::CreateItemCacheSet(uint32_t pid)
{
	if (m_map_pkItemCacheSetPtr.find(pid) != m_map_pkItemCacheSetPtr.end())
		return;

	TItemCacheSet* pSet = new TItemCacheSet;
	m_map_pkItemCacheSetPtr.insert(TItemCacheSetPtrMap::value_type(pid, pSet));

	if (g_log)
		sys_log(0, "ITEM_CACHE: new cache %u", pid);
}

void CClientManager::FlushItemCacheSet(uint32_t pid)
{
	const TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
	{
		sys_log(0, "FLUSH_ITEMCACHESET : No ItemCacheSet pid(%d)", pid);
		return;
	}

	TItemCacheSet* pSet = it->second;
	if (!pSet)
		return;
	TItemCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CItemCache* c = *it_set++;
		if (c)
		{
			c->Flush();

			m_map_itemCache.erase(c->Get()->id);
			delete c;
		}
	}

	pSet->clear();
	delete pSet;

	m_map_pkItemCacheSetPtr.erase(it);

	if (g_log)
		sys_log(0, "FLUSH_ITEMCACHESET : Deleted pid(%d)", pid);
}

CItemCache* CClientManager::GetItemCache(uint32_t id)
{
	const TItemCacheMap::iterator it = m_map_itemCache.find(id);

	if (it == m_map_itemCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutItemCache(TPlayerItem* pNew, bool bSkipQuery)
{
	CItemCache* c = GetItemCache(pNew->id);

	// Create a new item
	if (c == nullptr)
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: PutItemCache ==> New CItemCache id%d vnum%d new owner%d", pNew->id, pNew->vnum, pNew->owner);

		c = new CItemCache;
		m_map_itemCache.insert(TItemCacheMap::value_type(pNew->id, c));
	}
	// When there is
	else
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: PutItemCache ==> Have Cache");
		// If the owner is wrong
		if (pNew->owner != c->Get()->owner)
		{
			// Delete the item from users who already have this item.
			const TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

			if (it != m_map_pkItemCacheSetPtr.end())
			{
				if (g_log)
					sys_log(0, "ITEM_CACHE: delete owner %u id %u new owner %u", c->Get()->owner, c->Get()->id, pNew->owner);
				it->second->erase(c);
			}
		}
	}

	// New information update
	c->Put(pNew, bSkipQuery);

	const TItemCacheSetPtrMap::iterator it = m_map_pkItemCacheSetPtr.find(c->Get()->owner);

	if (it != m_map_pkItemCacheSetPtr.end())
	{
		if (g_log)
			sys_log(0, "ITEM_CACHE: save %u id %u", c->Get()->owner, c->Get()->id);
		else
			sys_log(1, "ITEM_CACHE: save %u id %u", c->Get()->owner, c->Get()->id);
		it->second->insert(c);
	}
	else
	{
		// As there is no current owner, you need to save it immediately by querying the SQL when the next connection comes.
		// Save it immediately as it can be received.
		if (g_log)
			sys_log(0, "ITEM_CACHE: direct save %u id %u", c->Get()->owner, c->Get()->id);
		else
			sys_log(1, "ITEM_CACHE: direct save %u id %u", c->Get()->owner, c->Get()->id);

		c->OnFlush();
	}
}

bool CClientManager::DeleteItemCache(uint32_t dwID)
{
	CItemCache* c = GetItemCache(dwID);

	if (!c)
		return false;

	c->Delete();
	return true;
}

// MYSHOP_PRICE_LIST
CItemPriceListTableCache* CClientManager::GetItemPriceListCache(uint32_t dwID)
{
	const TItemPriceListCacheMap::iterator it = m_mapItemPriceListCache.find(dwID);

	if (it == m_mapItemPriceListCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutItemPriceListCache(const TItemPriceListTable* pItemPriceList)
{
	CItemPriceListTableCache* pCache = GetItemPriceListCache(pItemPriceList->dwOwnerID);

	if (!pCache)
	{
		pCache = new CItemPriceListTableCache;
		m_mapItemPriceListCache.insert(TItemPriceListCacheMap::value_type(pItemPriceList->dwOwnerID, pCache));
	}

	pCache->Put(const_cast<TItemPriceListTable*>(pItemPriceList), true);
}

void CClientManager::UpdatePlayerCache()
{
	TPlayerTableCacheMap::iterator it = m_map_playerCache.begin();

	while (it != m_map_playerCache.end())
	{
		CPlayerTableCache* c = (it++)->second;
		if (c)
		{
			if (c->CheckTimeout())
			{
				if (g_log)
					sys_log(0, "UPDATE : UpdatePlayerCache() ==> FlushPlayerCache %d %s ", c->Get(false)->id, c->Get(false)->name);

				c->Flush();

				// Item Cache is also updated
				UpdateItemCacheSet(c->Get()->id);
			}
			else if (c->CheckFlushTimeout())
				c->Flush();
		}
	}
}
// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_SKILL_COLOR_SYSTEM
CSKillColorCache* CClientManager::GetSkillColorCache(uint32_t id)
{
	TSkillColorCacheMap::iterator it = m_map_SkillColorCache.find(id);

	if (it == m_map_SkillColorCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutSkillColorCache(const TSkillColor* pNew)
{
	CSKillColorCache* pCache = GetSkillColorCache(pNew->player_id);

	if (!pCache)
	{
		pCache = new CSKillColorCache;
		m_map_SkillColorCache.insert(TSkillColorCacheMap::value_type(pNew->player_id, pCache));
	}

	pCache->Put(const_cast<TSkillColor*>(pNew), false);
}

void CClientManager::UpdateSkillColorCache()
{
	TSkillColorCacheMap::iterator it = m_map_SkillColorCache.begin();

	while (it != m_map_SkillColorCache.end())
	{
		CSKillColorCache* pCache = it->second;

		if (pCache->CheckFlushTimeout())
		{
			pCache->Flush();
			m_map_SkillColorCache.erase(it++);
		}
		else
			++it;
	}
}
#endif

void CClientManager::SetCacheFlushCountLimit(int iLimit) noexcept
{
	m_iCacheFlushCountLimit = MAX(10, iLimit);
	sys_log(0, "CACHE_FLUSH_LIMIT_PER_SECOND: %d", m_iCacheFlushCountLimit);
}

void CClientManager::UpdateItemCache()
{
	if (m_iCacheFlushCount >= m_iCacheFlushCountLimit)
		return;

	TItemCacheMap::iterator it = m_map_itemCache.begin();

	while (it != m_map_itemCache.end())
	{
		CItemCache* c = (it++)->second;
		// Items are only flushed.
		if (c && c->CheckFlushTimeout())
		{
			if (g_test_server)
				sys_log(0, "UpdateItemCache ==> Flush() vnum %d id owner %d", c->Get()->vnum, c->Get()->id, c->Get()->owner);

			c->Flush();

			if (++m_iCacheFlushCount >= m_iCacheFlushCountLimit)
				break;
		}
	}
}

void CClientManager::UpdateItemPriceListCache()
{
	TItemPriceListCacheMap::iterator it = m_mapItemPriceListCache.begin();

	while (it != m_mapItemPriceListCache.end())
	{
		CItemPriceListTableCache* pCache = it->second;
		if (pCache && pCache->CheckFlushTimeout())
		{
			pCache->Flush();
			m_mapItemPriceListCache.erase(it++);
			delete pCache; // prevent item price memory leak
		}
		else
			++it;
	}
}

void CClientManager::QUERY_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData)
{
	if (!pkPeer)
		return;

	const uint32_t dwID = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	const uint32_t dwPID = *(uint32_t*)c_pData;

	if (!DeleteItemCache(dwID))
	{
		char szQuery[64];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM item%s WHERE id=%u", GetTablePostfix(), dwID);

		if (g_log)
			sys_log(0, "HEADER_GD_ITEM_DESTROY: PID %u ID %u", dwPID, dwID);

		if (dwPID == 0) // Asynchronous query, if no one had it
			CDBManager::Instance().AsyncQuery(szQuery);
		else
			CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_DESTROY, pkPeer->GetHandle(), nullptr);
	}
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CClientManager::QUERY_PET_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData)
{
	if (!pkPeer)
		return;

	const uint32_t dwID = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	//if (!DeleteItemCache(dwID))
	{
		char szQuery[64];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM pet%s WHERE id=%u", GetTablePostfix(), dwID);

		if (g_log)
			sys_log(0, "HEADER_GD_PET_ITEM_DESTROY: ID %u", dwID);

		if (dwID == 0)
			CDBManager::Instance().AsyncQuery(szQuery);
		else
			CDBManager::Instance().ReturnQuery(szQuery, QID_ITEM_DESTROY, pkPeer->GetHandle(), nullptr);
	}
}
#endif

void CClientManager::QUERY_FLUSH_CACHE(CPeer* pkPeer, const char* c_pData)
{
	const uint32_t dwPID = *(uint32_t*)c_pData;

	CPlayerTableCache* pkCache = GetPlayerCache(dwPID);

	if (!pkCache)
		return;

	sys_log(0, "FLUSH_CACHE: %u", dwPID);

	pkCache->Flush();
	FlushItemCacheSet(dwPID);

	m_map_playerCache.erase(dwPID);
	delete pkCache;
}

void CClientManager::QUERY_SMS(CPeer* pkPeer, TPacketGDSMS* pack)
{
	char szQuery[QUERY_MAX_LEN];

	char szMsg[256 + 1];
	//unsigned long len = CDBManager::Instance().EscapeString(szMsg, pack->szMsg, strlen(pack->szMsg), SQL_ACCOUNT);
	unsigned long len = CDBManager::Instance().EscapeString(szMsg, pack->szMsg, strlen(pack->szMsg));
	szMsg[len] = '\0';

	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO sms_pool (server, sender, receiver, mobile, msg) VALUES(%d, '%s', '%s', '%s', '%s')",
		(m_iPlayerIDStart + 2) / 3, pack->szFrom, pack->szTo, pack->szMobile, szMsg);

	CDBManager::Instance().AsyncQuery(szQuery);
}

void CClientManager::QUERY_RELOAD_PROTO()
{
	if (!InitializeTables())
	{
		sys_err("QUERY_RELOAD_PROTO: cannot load tables");
		return;
	}

	for (TPeerList::iterator i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;
		if (!tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_RELOAD_PROTO, 0,
			sizeof(uint16_t) + sizeof(TSkillTable) * m_vec_skillTable.size() +
			sizeof(uint16_t) + sizeof(TBanwordTable) * m_vec_banwordTable.size() +
			sizeof(uint16_t) + sizeof(TItemTable) * m_vec_itemTable.size() +
			sizeof(uint16_t) + sizeof(TMobTable) * m_vec_mobTable.size()
#ifdef ENABLE_EXTENDED_RELOAD
			+ sizeof(uint16_t) + sizeof(TRefineTable) * m_iRefineTableSize
#endif
		);

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_skillTable.size()));
		tmp->Encode(&m_vec_skillTable[0], sizeof(TSkillTable) * m_vec_skillTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_banwordTable.size()));
		tmp->Encode(&m_vec_banwordTable[0], sizeof(TBanwordTable) * m_vec_banwordTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_itemTable.size()));
		tmp->Encode(&m_vec_itemTable[0], sizeof(TItemTable) * m_vec_itemTable.size());

		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_mobTable.size()));
		tmp->Encode(&m_vec_mobTable[0], sizeof(TMobTable) * m_vec_mobTable.size());

#ifdef ENABLE_EXTENDED_RELOAD
		tmp->EncodeWORD(m_iRefineTableSize);
		tmp->Encode(m_pRefineTable, sizeof(TRefineTable) * m_iRefineTableSize);
#endif
	}
}

// ADD_GUILD_PRIV_TIME
/**
* @version 05/06/08 Bang2ni - Add duration
*/
void CClientManager::AddGuildPriv(const TPacketGiveGuildPriv* p)
{
	CPrivManager::Instance().AddGuildPriv(p->guild_id, p->type, p->value, p->duration_sec);
#ifdef ENABLE_DEFAULT_PRIV
	__UpdateDefaultPriv("GUILD", p->guild_id, p->type, p->value, p->duration_sec);
#endif
}

void CClientManager::AddEmpirePriv(const TPacketGiveEmpirePriv* p)
{
	CPrivManager::Instance().AddEmpirePriv(p->empire, p->type, p->value, p->duration_sec);
#ifdef ENABLE_DEFAULT_PRIV
	__UpdateDefaultPriv("EMPIRE", p->empire, p->type, p->value, p->duration_sec);
#endif
}
// END_OF_ADD_GUILD_PRIV_TIME

void CClientManager::AddCharacterPriv(const TPacketGiveCharacterPriv* p)
{
	CPrivManager::Instance().AddCharPriv(p->pid, p->type, p->value);
#ifdef ENABLE_DEFAULT_PRIV
	__UpdateDefaultPriv("PLAYER", p->pid, p->type, p->value, 0);
#endif
}

void CClientManager::MoneyLog(const TPacketMoneyLog* p)
{
	CMoneyLog::Instance().AddLog(p->type, p->vnum, p->gold);
}

CLoginData* CClientManager::GetLoginData(uint32_t dwKey)
{
	const TLoginDataByLoginKey::iterator it = m_map_pkLoginData.find(dwKey);

	if (it == m_map_pkLoginData.end())
		return nullptr;

	return it->second;
}

CLoginData* CClientManager::GetLoginDataByLogin(const char* c_pszLogin)
{
	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(c_pszLogin, szLogin, sizeof(szLogin));

	const TLoginDataByLogin::iterator it = m_map_pkLoginDataByLogin.find(szLogin);

	if (it == m_map_pkLoginDataByLogin.end())
		return nullptr;

	return it->second;
}

CLoginData* CClientManager::GetLoginDataByAID(uint32_t dwAID)
{
	const TLoginDataByAID::iterator it = m_map_pkLoginDataByAID.find(dwAID);

	if (it == m_map_pkLoginDataByAID.end())
		return nullptr;

	return it->second;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
//Note: Due to the structure of the system, this function will only
//return a valid login data if the player IS logged in into the character
//with the given PID.
CLoginData* CClientManager::GetLoginDataByPID(uint32_t dwPID)
{
	for (const auto& it : m_map_pkLoginData)
	{
		CLoginData* data = it.second;
		if (data && (data->GetLastPlayerID() == dwPID))
			return data;
	}

	return nullptr;
}
#endif

void CClientManager::InsertLoginData(CLoginData* pkLD)
{
	if (!pkLD)
		return;

	char szLogin[LOGIN_MAX_LEN + 1];
	trim_and_lower(pkLD->GetAccountRef().login, szLogin, sizeof(szLogin));

	m_map_pkLoginData.insert(std::make_pair(pkLD->GetKey(), pkLD));
	m_map_pkLoginDataByLogin.insert(std::make_pair(szLogin, pkLD));
	m_map_pkLoginDataByAID.insert(std::make_pair(pkLD->GetAccountRef().id, pkLD));
}

void CClientManager::DeleteLoginData(CLoginData* pkLD)
{
	if (!pkLD)
		return;

	m_map_pkLoginData.erase(pkLD->GetKey());
	m_map_pkLoginDataByLogin.erase(pkLD->GetAccountRef().login);
	m_map_pkLoginDataByAID.erase(pkLD->GetAccountRef().id);

	if (m_map_kLogonAccount.find(pkLD->GetAccountRef().login) == m_map_kLogonAccount.end())
		delete pkLD;
	else
		pkLD->SetDeleted(true);
}

void CClientManager::QUERY_AUTH_LOGIN(CPeer* pkPeer, uint32_t dwHandle, TPacketGDAuthLogin* p)
{
	if (!pkPeer)
		return;

	if (g_test_server)
		sys_log(0, "QUERY_AUTH_LOGIN %d %d %s", p->dwID, p->dwLoginKey, p->szLogin);
	CLoginData* pkLD = GetLoginDataByLogin(p->szLogin);

	if (pkLD)
		DeleteLoginData(pkLD);

	uint8_t bResult = 0;

	if (GetLoginData(p->dwLoginKey))
	{
		sys_err("LoginData already exist key %u login %s", p->dwLoginKey, p->szLogin);
		bResult = 0;

		pkPeer->EncodeHeader(HEADER_DG_AUTH_LOGIN, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bResult);
	}
	else
	{
		CLoginData* pkLDNew = new CLoginData;

		pkLDNew->SetKey(p->dwLoginKey);
		pkLDNew->SetClientKey(p->adwClientKey);
		pkLDNew->SetPremium(p->iPremiumTimes);

		TAccountTable& r = pkLDNew->GetAccountRef();

		r.id = p->dwID;
		trim_and_lower(p->szLogin, r.login, sizeof(r.login));
		strlcpy(r.social_id, p->szSocialID, sizeof(r.social_id));
		strlcpy(r.passwd, "TEMP", sizeof(r.passwd));
#ifdef ENABLE_HWID_BAN
		strlcpy(r.hwid, p->hwid, sizeof(r.hwid));
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		r.bLanguage = p->bLanguage;
#endif

		sys_log(0, "AUTH_LOGIN id(%u) login(%s) "
#ifdef ENABLE_HWID_BAN
			"hwid(%s) "
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			"language(%d) "
#endif
			"social_id(%s) login_key(%u), client_key(%u %u %u %u)",
			p->dwID, p->szLogin,
#ifdef ENABLE_HWID_BAN
			p->hwid,
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			p->bLanguage,
#endif
			p->szSocialID,
			p->dwLoginKey,
			p->adwClientKey[0], p->adwClientKey[1], p->adwClientKey[2], p->adwClientKey[3]);

		bResult = 1;

		InsertLoginData(pkLDNew);

		pkPeer->EncodeHeader(HEADER_DG_AUTH_LOGIN, dwHandle, sizeof(uint8_t));
		pkPeer->EncodeBYTE(bResult);
	}
}

void CClientManager::GuildDepositMoney(const TPacketGDGuildMoney* p)
{
	CGuildManager::Instance().DepositMoney(p->dwGuild, p->iGold);
}

#ifdef ENABLE_USE_MONEY_FROM_GUILD
void CClientManager::GuildWithdrawMoney(const TPacketGDGuildMoney* p)
{
	CGuildManager::Instance().WithdrawMoney(p->dwGuild, p->iGold);
}
#else
void CClientManager::GuildWithdrawMoney(CPeer* peer, const TPacketGDGuildMoney* p)
{
	CGuildManager::Instance().WithdrawMoney(peer, p->dwGuild, p->iGold);
}
#endif

void CClientManager::GuildWithdrawMoneyGiveReply(const TPacketGDGuildMoneyWithdrawGiveReply* p)
{
	CGuildManager::Instance().WithdrawMoneyReply(p->dwGuild, p->bGiveSuccess, p->iChangeGold);
}

void CClientManager::GuildWarBet(const TPacketGDGuildWarBet* p)
{
	CGuildManager::Instance().Bet(p->dwWarID, p->szLogin, p->dwGold, p->dwGuild);
}

void CClientManager::CreateObject(const TPacketGDCreateObject* p)
{
	using namespace building;

	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery),
		"INSERT INTO object%s (land_id, vnum, map_index, x, y, x_rot, y_rot, z_rot) VALUES(%u, %u, %d, %d, %d, %f, %f, %f)",
		GetTablePostfix(), p->dwLandID, p->dwVnum, p->lMapIndex, p->x, p->y, p->xRot, p->yRot, p->zRot);

	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));
	if (pmsg->Get()->uiInsertID == 0)
	{
		sys_err("cannot insert object");
		return;
	}

	TObject* pkObj = new TObject;

	memset(pkObj, 0, sizeof(TObject));

	pkObj->dwID = static_cast<uint32_t>(pmsg->Get()->uiInsertID);
	pkObj->dwVnum = p->dwVnum;
	pkObj->dwLandID = p->dwLandID;
	pkObj->lMapIndex = p->lMapIndex;
	pkObj->x = p->x;
	pkObj->y = p->y;
	pkObj->xRot = p->xRot;
	pkObj->yRot = p->yRot;
	pkObj->zRot = p->zRot;
	pkObj->lLife = 0;

	ForwardPacket(HEADER_DG_CREATE_OBJECT, pkObj, sizeof(TObject));

	m_map_pkObjectTable.insert(std::make_pair(pkObj->dwID, pkObj));
}

void CClientManager::DeleteObject(uint32_t dwID)
{
	char szQuery[128];
	snprintf(szQuery, sizeof(szQuery), "DELETE FROM object%s WHERE id=%u", GetTablePostfix(), dwID);

	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));

	if (pmsg->Get()->uiAffectedRows == 0 || pmsg->Get()->uiAffectedRows == (uint32_t)-1)
	{
		sys_err("no object by id %u", dwID);
		return;
	}

	const auto it = m_map_pkObjectTable.find(dwID);

	if (it != m_map_pkObjectTable.end())
	{
		delete it->second;
		m_map_pkObjectTable.erase(it);
	}

	ForwardPacket(HEADER_DG_DELETE_OBJECT, &dwID, sizeof(uint32_t));
}

void CClientManager::UpdateLand(const uint32_t* pdw)
{
	const uint32_t dwID = pdw[0];
	const uint32_t dwGuild = pdw[1];

	building::TLand* p = &m_vec_kLandTable[0];

	uint32_t i;

	for (i = 0; i < m_vec_kLandTable.size(); ++i, ++p)
	{
		if (p->dwID == dwID)
		{
			char buf[256];
			snprintf(buf, sizeof(buf), "UPDATE land%s SET guild_id=%u WHERE id=%u", GetTablePostfix(), dwGuild, dwID);
			CDBManager::Instance().AsyncQuery(buf);

			p->dwGuildID = dwGuild;
			break;
		}
	}

	if (i < m_vec_kLandTable.size())
		ForwardPacket(HEADER_DG_UPDATE_LAND, p, sizeof(building::TLand));
}

// BLOCK_CHAT
void CClientManager::BlockChat(TPacketBlockChat* p)
{
	char szQuery[256] = { 0, };
	snprintf(szQuery, sizeof(szQuery), "SELECT id FROM player%s WHERE name = '%s'", GetTablePostfix(), p->szName);

	auto pmsg(CDBManager::Instance().DirectQuery(szQuery));
	SQLResult* pRes = pmsg->Get();

	if (pRes->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
		const uint32_t pid = strtoul(row[0], nullptr, 10);

		TPacketGDAddAffect pa{};
		pa.dwPID = pid;
		pa.elem.dwType = 223;
		pa.elem.wApplyOn = 0;
		pa.elem.lApplyValue = 0;
		pa.elem.dwFlag = 0;
		pa.elem.lDuration = p->lDuration;
		pa.elem.lSPCost = 0;
		QUERY_ADD_AFFECT(nullptr, &pa);
	}
	else
	{
		// cannot find user with that name
	}
}
// END_OF_BLOCK_CHAT

void CClientManager::MarriageAdd(TPacketMarriageAdd* p)
{
	sys_log(0, "MarriageAdd %u %u %s %s", p->dwPID1, p->dwPID2, p->szName1, p->szName2);
	marriage::CManager::Instance().Add(p->dwPID1, p->dwPID2, p->szName1, p->szName2);
}

void CClientManager::MarriageUpdate(const TPacketMarriageUpdate* p)
{
	sys_log(0, "MarriageUpdate PID:%u %u LP:%d ST:%d", p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
	marriage::CManager::Instance().Update(p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
}

void CClientManager::MarriageRemove(const TPacketMarriageRemove* p)
{
	sys_log(0, "MarriageRemove %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().Remove(p->dwPID1, p->dwPID2);
}

void CClientManager::WeddingRequest(const TPacketWeddingRequest* p)
{
	sys_log(0, "WeddingRequest %u %u", p->dwPID1, p->dwPID2);
	ForwardPacket(HEADER_DG_WEDDING_REQUEST, p, sizeof(TPacketWeddingRequest));
	//marriage::CManager::Instance().RegisterWedding(p->dwPID1, p->szName1, p->dwPID2, p->szName2);
}

void CClientManager::WeddingReady(const TPacketWeddingReady* p)
{
	sys_log(0, "WeddingReady %u %u", p->dwPID1, p->dwPID2);
	ForwardPacket(HEADER_DG_WEDDING_READY, p, sizeof(TPacketWeddingReady));
	marriage::CManager::Instance().ReadyWedding(p->dwMapIndex, p->dwPID1, p->dwPID2);
}

void CClientManager::WeddingEnd(const TPacketWeddingEnd* p)
{
	sys_log(0, "WeddingEnd %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().EndWedding(p->dwPID1, p->dwPID2);
}

//
// If there is price information in the cache, update the cache.
// First, after loading existing data, create a cache with the existing information and update the newly received price information
//
void CClientManager::MyshopPricelistUpdate(const TItemPriceListTable* pPacket) // @fixme403 (TPacketMyshopPricelistHeader to TItemPriceListTable)
{
	if (pPacket->byCount > SHOP_PRICELIST_MAX_NUM)
	{
		sys_err("count overflow!");
		return;
	}

	CItemPriceListTableCache* pCache = GetItemPriceListCache(pPacket->dwOwnerID);

	if (pCache)
	{
		TItemPriceListTable table;

		table.dwOwnerID = pPacket->dwOwnerID;
		table.byCount = pPacket->byCount;

		thecore_memcpy(table.aPriceInfo, pPacket->aPriceInfo, sizeof(TItemPriceInfo) * pPacket->byCount);

		pCache->UpdateList(&table);
	}
	else
	{
		TItemPriceListTable* pUpdateTable = new TItemPriceListTable;

		pUpdateTable->dwOwnerID = pPacket->dwOwnerID;
		pUpdateTable->byCount = pPacket->byCount;

		thecore_memcpy(pUpdateTable->aPriceInfo, pPacket->aPriceInfo, sizeof(TItemPriceInfo) * pPacket->byCount);

		char szQuery[QUERY_MAX_LEN];
#ifdef ENABLE_CHEQUE_SYSTEM
		snprintf(szQuery, sizeof(szQuery), "SELECT item_vnum, price, cheque FROM myshop_pricelist%s WHERE owner_id=%u", GetTablePostfix(), pPacket->dwOwnerID);
#else
		snprintf(szQuery, sizeof(szQuery), "SELECT item_vnum, price FROM myshop_pricelist%s WHERE owner_id=%u", GetTablePostfix(), pPacket->dwOwnerID);
#endif
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_LOAD_FOR_UPDATE, 0, pUpdateTable);
	}
}

// MYSHOP_PRICE_LIST
// If there is cached price information, it reads the cache and sends it immediately, and if there is no information in the cache, it queries the DB.
//
void CClientManager::MyshopPricelistRequest(CPeer* peer, uint32_t dwHandle, uint32_t dwPlayerID)
{
	if (!peer)
		return;

	if (CItemPriceListTableCache* pCache = GetItemPriceListCache(dwPlayerID))
	{
		sys_log(0, "Cache MyShopPricelist handle[%d] pid[%d]", dwHandle, dwPlayerID);

		const TItemPriceListTable* pTable = pCache->Get(false);

		TPacketMyshopPricelistHeader header =
		{
			pTable->dwOwnerID,
			pTable->byCount
		};

		const size_t sizePriceListSize = sizeof(TItemPriceInfo) * pTable->byCount;

		peer->EncodeHeader(HEADER_DG_MYSHOP_PRICELIST_RES, dwHandle, sizeof(header) + sizePriceListSize);
		peer->Encode(&header, sizeof(header));
		peer->Encode(pTable->aPriceInfo, sizePriceListSize);

	}
	else
	{
		sys_log(0, "Query MyShopPricelist handle[%d] pid[%d]", dwHandle, dwPlayerID);

		char szQuery[QUERY_MAX_LEN];
#ifdef ENABLE_CHEQUE_SYSTEM
		snprintf(szQuery, sizeof(szQuery), "SELECT item_vnum, price, cheque FROM myshop_pricelist%s WHERE owner_id=%u", GetTablePostfix(), dwPlayerID);
#else
		snprintf(szQuery, sizeof(szQuery), "SELECT item_vnum, price FROM myshop_pricelist%s WHERE owner_id=%u", GetTablePostfix(), dwPlayerID);
#endif
		CDBManager::Instance().ReturnQuery(szQuery, QID_ITEMPRICE_LOAD, peer->GetHandle(), new TItemPricelistReqInfo(dwHandle, dwPlayerID));
	}
}
// END_OF_MYSHOP_PRICE_LIST

void CPacketInfo::Add(int header)
{
	const auto it = m_map_info.find(header);

	if (it == m_map_info.end())
		m_map_info.insert(std::map<int, int>::value_type(header, 1));
	else
		++it->second;
}

void CPacketInfo::Reset()
{
	m_map_info.clear();
}

void CClientManager::ProcessPackets(CPeer* peer)
{
	uint8_t header = 0;
	uint32_t dwHandle = 0;
	uint32_t dwLength = 0;
	const char* data = nullptr;
	int i = 0;
	int iCount = 0;

	while (peer->PeekPacket(i, header, dwHandle, dwLength, &data))
	{
		// DISABLE_DB_HEADER_LOG
		// sys_log(0, "header %d %p size %d", header, this, dwLength);
		// END_OF_DISABLE_DB_HEADER_LOG
		m_bLastHeader = header;
		++iCount;

		if (g_test_server)
		{
			if (header != 10)
				sys_log(0, " ProcessPacket Header [%d] Handle[%d] Length[%d] iCount[%d]", header, dwHandle, dwLength, iCount);
		}

		// test log by mhh
		{
			if (HEADER_GD_BLOCK_COUNTRY_IP == header)
				sys_log(0, "recved : HEADER_GD_BLOCK_COUNTRY_IP");
		}

		switch (header)
		{
			case HEADER_GD_BOOT:
				QUERY_BOOT(peer, (TPacketGDBoot*)data);
				break;

			case HEADER_GD_HAMMER_OF_TOR:
				break;

			case HEADER_GD_LOGIN_BY_KEY:
				QUERY_LOGIN_BY_KEY(peer, dwHandle, (TPacketGDLoginByKey*)data);
				break;

			case HEADER_GD_LOGOUT:
				//sys_log(0, "HEADER_GD_LOGOUT (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_LOGOUT(peer, dwHandle, data);
				break;

			case HEADER_GD_PLAYER_LOAD:
			{
				sys_log(1, "HEADER_GD_PLAYER_LOAD (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_PLAYER_LOAD(peer, dwHandle, (TPlayerLoadPacket*)data);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				QUERY_SKILL_COLOR_LOAD(peer, dwHandle, (TPlayerLoadPacket*)data);
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				QUERY_PLAYER_ACHIEVEMENTS_LOAD(peer, dwHandle, (TPlayerLoadPacket*)data);
#endif
			}
			break;

			case HEADER_GD_PLAYER_SAVE:
				sys_log(1, "HEADER_GD_PLAYER_SAVE (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_PLAYER_SAVE(peer, dwHandle, (TPlayerTable*)data);
				break;

			case HEADER_GD_PLAYER_CREATE:
				sys_log(0, "HEADER_GD_PLAYER_CREATE (handle: %d length: %d)", dwHandle, dwLength);
				__QUERY_PLAYER_CREATE(peer, dwHandle, (TPlayerCreatePacket*)data);
				sys_log(0, "END");
				break;

			case HEADER_GD_PLAYER_DELETE:
				sys_log(1, "HEADER_GD_PLAYER_DELETE (handle: %d length: %d)", dwHandle, dwLength);
				__QUERY_PLAYER_DELETE(peer, dwHandle, (TPlayerDeletePacket*)data);
				break;

			case HEADER_GD_PLAYER_COUNT:
				QUERY_PLAYER_COUNT(peer, (TPlayerCountPacket*)data);
				break;

			case HEADER_GD_QUEST_SAVE:
				sys_log(1, "HEADER_GD_QUEST_SAVE (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_QUEST_SAVE(peer, (TQuestTable*)data, dwLength);
				break;

			case HEADER_GD_SAFEBOX_LOAD:
				QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 0);
				break;

#ifdef ENABLE_SAFEBOX_MONEY
			case HEADER_GD_SAFEBOX_SAVE:
				sys_log(1, "HEADER_GD_SAFEBOX_SAVE (handle: %d length: %d)", dwHandle, dwLength);
				QUERY_SAFEBOX_SAVE(peer, (TSafeboxTable*)data);
				break;
#endif

			case HEADER_GD_SAFEBOX_CHANGE_SIZE:
				QUERY_SAFEBOX_CHANGE_SIZE(peer, dwHandle, (TSafeboxChangeSizePacket*)data);
				break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			case HEADER_GD_GUILDSTORAGE_CHANGE_SIZE:
				QUERY_GUILDSTORAGE_CHANGE_SIZE(peer, dwHandle, (TGuildstorageChangeSizePacket*)data);
				break;
#endif

			case HEADER_GD_SAFEBOX_CHANGE_PASSWORD:
				QUERY_SAFEBOX_CHANGE_PASSWORD(peer, dwHandle, (TSafeboxChangePasswordPacket*)data);
				break;

			case HEADER_GD_MALL_LOAD:
				QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 1);
				break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			case HEADER_GD_GUILDSTORAGE_LOAD:
				QUERY_SAFEBOX_LOAD(peer, dwHandle, (TSafeboxLoadPacket*)data, 2);
				break;
#endif

#ifdef ENABLE_MOVE_CHANNEL
			case HEADER_GD_MOVE_CHANNEL:
				QUERY_MOVE_CHANNEL(peer, dwHandle, (TMoveChannel*)data);
				break;
#endif

			case HEADER_GD_EMPIRE_SELECT:
				QUERY_EMPIRE_SELECT(peer, dwHandle, (TEmpireSelectPacket*)data);
				break;

			case HEADER_GD_SETUP:
				QUERY_SETUP(peer, dwHandle, data);
				break;

			case HEADER_GD_GUILD_CREATE:
				GuildCreate(peer, *(uint32_t*)data);
				break;

			case HEADER_GD_GUILD_SKILL_UPDATE:
				GuildSkillUpdate(peer, (TPacketGuildSkillUpdate*)data);
				break;

			case HEADER_GD_GUILD_EXP_UPDATE:
				GuildExpUpdate(peer, (TPacketGuildExpUpdate*)data);
				break;

			case HEADER_GD_GUILD_ADD_MEMBER:
				GuildAddMember(peer, (TPacketGDGuildAddMember*)data);
				break;

			case HEADER_GD_GUILD_REMOVE_MEMBER:
				GuildRemoveMember(peer, (TPacketGuild*)data);
				break;

			case HEADER_GD_GUILD_CHANGE_GRADE:
				GuildChangeGrade(peer, (TPacketGuild*)data);
				break;

			case HEADER_GD_GUILD_CHANGE_MEMBER_DATA:
				GuildChangeMemberData(peer, (TPacketGuildChangeMemberData*)data);
				break;

			case HEADER_GD_GUILD_DISBAND:
				GuildDisband(peer, (TPacketGuild*)data);
				break;

			case HEADER_GD_GUILD_WAR:
				GuildWar(peer, (TPacketGuildWar*)data);
				break;

			case HEADER_GD_GUILD_WAR_SCORE:
				GuildWarScore(peer, (TPacketGuildWarScore*)data);
				break;

			case HEADER_GD_GUILD_CHANGE_LADDER_POINT:
				GuildChangeLadderPoint((TPacketGuildLadderPoint*)data);
				break;

			case HEADER_GD_GUILD_USE_SKILL:
				GuildUseSkill((TPacketGuildUseSkill*)data);
				break;

			case HEADER_GD_FLUSH_CACHE:
				QUERY_FLUSH_CACHE(peer, data);
				break;

			case HEADER_GD_ITEM_SAVE:
				QUERY_ITEM_SAVE(peer, data);
				break;

			case HEADER_GD_ITEM_DESTROY:
				QUERY_ITEM_DESTROY(peer, data);
				break;

#ifdef ENABLE_GROWTH_PET_SYSTEM
			case HEADER_GD_PET_ITEM_DESTROY:
				QUERY_PET_ITEM_DESTROY(peer, data);
				break;
#endif

			case HEADER_GD_ITEM_FLUSH:
				QUERY_ITEM_FLUSH(peer, data);
				break;

			case HEADER_GD_ADD_AFFECT:
				sys_log(1, "HEADER_GD_ADD_AFFECT");
				QUERY_ADD_AFFECT(peer, (TPacketGDAddAffect*)data);
				break;

			case HEADER_GD_REMOVE_AFFECT:
				sys_log(1, "HEADER_GD_REMOVE_AFFECT");
				QUERY_REMOVE_AFFECT(peer, (TPacketGDRemoveAffect*)data);
				break;

			case HEADER_GD_HIGHSCORE_REGISTER:
				QUERY_HIGHSCORE_REGISTER(peer, (TPacketGDHighscore*)data);
				break;

			case HEADER_GD_PARTY_CREATE:
				QUERY_PARTY_CREATE(peer, (TPacketPartyCreate*)data);
				break;

			case HEADER_GD_PARTY_DELETE:
				QUERY_PARTY_DELETE(peer, (TPacketPartyDelete*)data);
				break;

			case HEADER_GD_PARTY_ADD:
				QUERY_PARTY_ADD(peer, (TPacketPartyAdd*)data);
				break;

			case HEADER_GD_PARTY_REMOVE:
				QUERY_PARTY_REMOVE(peer, (TPacketPartyRemove*)data);
				break;

			case HEADER_GD_PARTY_STATE_CHANGE:
				QUERY_PARTY_STATE_CHANGE(peer, (TPacketPartyStateChange*)data);
				break;

			case HEADER_GD_PARTY_SET_MEMBER_LEVEL:
				QUERY_PARTY_SET_MEMBER_LEVEL(peer, (TPacketPartySetMemberLevel*)data);
				break;

			case HEADER_GD_RELOAD_PROTO:
				QUERY_RELOAD_PROTO();
				break;

			case HEADER_GD_CHANGE_NAME:
				QUERY_CHANGE_NAME(peer, dwHandle, (TPacketGDChangeName*)data);
				break;

			case HEADER_GD_SMS:
				QUERY_SMS(peer, (TPacketGDSMS*)data);
				break;

			case HEADER_GD_AUTH_LOGIN:
				QUERY_AUTH_LOGIN(peer, dwHandle, (TPacketGDAuthLogin*)data);
				break;

			case HEADER_GD_REQUEST_GUILD_PRIV:
				AddGuildPriv((TPacketGiveGuildPriv*)data);
				break;

			case HEADER_GD_REQUEST_EMPIRE_PRIV:
				AddEmpirePriv((TPacketGiveEmpirePriv*)data);
				break;

			case HEADER_GD_REQUEST_CHARACTER_PRIV:
				AddCharacterPriv((TPacketGiveCharacterPriv*)data);
				break;

			case HEADER_GD_MONEY_LOG:
				MoneyLog((TPacketMoneyLog*)data);
				break;

			case HEADER_GD_GUILD_DEPOSIT_MONEY:
				GuildDepositMoney((TPacketGDGuildMoney*)data);
				break;

			case HEADER_GD_GUILD_WITHDRAW_MONEY:
#ifdef ENABLE_USE_MONEY_FROM_GUILD
				GuildWithdrawMoney((TPacketGDGuildMoney*)data);
#else
				GuildWithdrawMoney(peer, (TPacketGDGuildMoney*)data);
#endif
				break;

			case HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY:
				GuildWithdrawMoneyGiveReply((TPacketGDGuildMoneyWithdrawGiveReply*)data);
				break;

			case HEADER_GD_GUILD_WAR_BET:
				GuildWarBet((TPacketGDGuildWarBet*)data);
				break;

			case HEADER_GD_SET_EVENT_FLAG:
				SetEventFlag((TPacketSetEventFlag*)data);
				break;

			case HEADER_GD_CREATE_OBJECT:
				CreateObject((TPacketGDCreateObject*)data);
				break;

			case HEADER_GD_DELETE_OBJECT:
				DeleteObject(*(uint32_t*)data);
				break;

			case HEADER_GD_UPDATE_LAND:
				UpdateLand((uint32_t*)data);
				break;

			case HEADER_GD_MARRIAGE_ADD:
				MarriageAdd((TPacketMarriageAdd*)data);
				break;

			case HEADER_GD_MARRIAGE_UPDATE:
				MarriageUpdate((TPacketMarriageUpdate*)data);
				break;

			case HEADER_GD_MARRIAGE_REMOVE:
				MarriageRemove((TPacketMarriageRemove*)data);
				break;

			case HEADER_GD_WEDDING_REQUEST:
				WeddingRequest((TPacketWeddingRequest*)data);
				break;

			case HEADER_GD_WEDDING_READY:
				WeddingReady((TPacketWeddingReady*)data);
				break;

			case HEADER_GD_WEDDING_END:
				WeddingEnd((TPacketWeddingEnd*)data);
				break;

				// BLOCK_CHAT
			case HEADER_GD_BLOCK_CHAT:
				BlockChat((TPacketBlockChat*)data);
				break;
				// END_OF_BLOCK_CHAT

				// MYSHOP_PRICE_LIST
			case HEADER_GD_MYSHOP_PRICELIST_UPDATE:
				MyshopPricelistUpdate((TItemPriceListTable*)data); // @fixme403 (TPacketMyshopPricelistHeader to TItemPriceListTable)
				break;

			case HEADER_GD_MYSHOP_PRICELIST_REQ:
				MyshopPricelistRequest(peer, dwHandle, *(uint32_t*)data);
				break;
				// END_OF_MYSHOP_PRICE_LIST

				//RELOAD_ADMIN
			case HEADER_GD_RELOAD_ADMIN:
				ReloadAdmin(peer, (TPacketReloadAdmin*)data);
				break;
				//END_RELOAD_ADMIN

			case HEADER_GD_BREAK_MARRIAGE:
				BreakMarriage(peer, data);
				break;

			//MOANRCH
			case HEADER_GD_ELECT_MONARCH:
				Election(peer, dwHandle, data);
				break;

			case HEADER_GD_CANDIDACY:
				Candidacy(peer, dwHandle, data);
				break;

			case HEADER_GD_ADD_MONARCH_MONEY:
				AddMonarchMoney(peer, dwHandle, data);
				break;

			case HEADER_GD_DEC_MONARCH_MONEY:
				DecMonarchMoney(peer, dwHandle, data);
				break;

			case HEADER_GD_TAKE_MONARCH_MONEY:
				TakeMonarchMoney(peer, dwHandle, data);
				break;

			case HEADER_GD_COME_TO_VOTE:
				ComeToVote(peer, dwHandle, data);
				break;

			case HEADER_GD_RMCANDIDACY: //< Candidate removal (operator)
				RMCandidacy(peer, dwHandle, data);
				break;

			case HEADER_GD_SETMONARCH: ///< Set up monarchy (operator)
				SetMonarch(peer, dwHandle, data);
				break;

			case HEADER_GD_RMMONARCH: ///< Delete the monarch
				RMMonarch(peer, dwHandle, data);
				break;

			case HEADER_GD_CHANGE_MONARCH_LORD:
				ChangeMonarchLord(peer, dwHandle, (TPacketChangeMonarchLord*)data);
				break;
			//END_MONARCH

			case HEADER_GD_BLOCK_COUNTRY_IP:
				sys_log(0, "HEADER_GD_BLOCK_COUNTRY_IP received");
				CBlockCountry::Instance().SendBlockedCountryIp(peer);
				CBlockCountry::Instance().SendBlockException(peer);
				break;

			case HEADER_GD_BLOCK_EXCEPTION:
				sys_log(0, "HEADER_GD_BLOCK_EXCEPTION received");
				BlockException((TPacketBlockException*)data);
				break;

			case HEADER_GD_REQ_SPARE_ITEM_ID_RANGE:
				SendSpareItemIDRange(peer);
				break;

			case HEADER_GD_REQ_CHANGE_GUILD_MASTER:
				GuildChangeMaster((TPacketChangeGuildMaster*)data);
				break;

			case HEADER_GD_UPDATE_HORSE_NAME:
				UpdateHorseName((TPacketUpdateHorseName*)data, peer);
				break;

			case HEADER_GD_REQ_HORSE_NAME:
				AckHorseName(*(uint32_t*)data, peer);
				break;

			case HEADER_GD_DC:
				DeleteLoginKey((TPacketDC*)data);
				break;

			case HEADER_GD_VALID_LOGOUT:
				ResetLastPlayerID((TPacketNeedLoginLogInfo*)data);
				break;

			case HEADER_GD_REQUEST_CHARGE_CASH:
				ChargeCash((TRequestChargeCash*)data);
				break;

			//delete gift notify icon
			case HEADER_GD_DELETE_AWARDID:
				DeleteAwardId((TPacketDeleteAwardID*)data);
				break;

			case HEADER_GD_UPDATE_CHANNELSTATUS:
				UpdateChannelStatus((SChannelStatus*)data);
				break;
			case HEADER_GD_REQUEST_CHANNELSTATUS:
				RequestChannelStatus(peer, dwHandle);
				break;

#ifdef ENABLE_YOHARA_SYSTEM
			case HEADER_GD_REQUEST_SUNGMA_PREMIUM:
				SungmaPremium(peer, dwHandle, *(uint32_t*)data);
				break;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			case HEADER_GD_LANGUAGE_CHANGE:
				LanguageChange(peer, data);
				break;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
			case HEADER_GD_ACHIEVEMENT:
				RecvAchievementPacket(peer, dwHandle, data);
				break;
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
			case HEADER_GD_GUILD_DUNGEON:
				GuildDungeon((TPacketGDGuildDungeon*)data);
				break;
			case HEADER_GD_GUILD_DUNGEON_CD:
				GuildDungeonGD((TPacketGDGuildDungeonCD*)data);
				break;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
			case HEADER_GD_SKILL_COLOR_SAVE:
				QUERY_SKILL_COLOR_SAVE(data);
				break;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
				//Selling from detached shop
				/*case HEADER_GD_SHOP_SALE:
					ShopSaleResult(data);
					break;*/

			case HEADER_GD_SHOP:
				ProcessShopPacket(peer, dwHandle, data);
				break;

			case HEADER_GD_SAVE_SHOP:
				SaveShop(peer, (TPlayerShopTable*)data);
				break;

			case SHOP_SUBHEADER_GD_SAVE_FLUSH:
			{
				SaveShop(peer, (TPlayerShopTable*)data);
				SaveShopFlush(peer, (TPlayerShopTable*)data);
				break;
			}

			case HEADER_GD_WITHDRAW_SHOP_GOLD:
				WithdrawShopGold(peer, dwHandle, data);
				break;

#	ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
			case HEADER_GD_PLAYER_LOGOUT:
				PlayerLogoutEvent(*(uint32_t*)data);
				break;
#	else
			case HEADER_GD_PLAYER_SHOP_TIMER:
				PlayerSetShopTime(*(uint32_t*)data);
				break;
#	endif
#endif

#ifdef ENABLE_MAILBOX
			case HEADER_GD_MAILBOX_LOAD:
				QUERY_MAILBOX_LOAD(peer, dwHandle, (TMailBox*)data);
				break;

			case HEADER_GD_MAILBOX_CHECK_NAME:
				QUERY_MAILBOX_CHECK_NAME(peer, dwHandle, (TMailBox*)data);
				break;

			case HEADER_GD_MAILBOX_WRITE:
				QUERY_MAILBOX_WRITE(peer, dwHandle, (TMailBoxTable*)data);
				break;

			case HEADER_GD_MAILBOX_DELETE:
				QUERY_MAILBOX_DELETE(peer, dwHandle, (TMailBox*)data);
				break;

			case HEADER_GD_MAILBOX_CONFIRM:
				QUERY_MAILBOX_CONFIRM(peer, dwHandle, (TMailBox*)data);
				break;

			case HEADER_GD_MAILBOX_GET:
				QUERY_MAILBOX_GET(peer, dwHandle, (TMailBox*)data);
				break;

			case HEADER_GD_MAILBOX_UNREAD:
				QUERY_MAILBOX_UNREAD(peer, dwHandle, (TMailBox*)data);
				break;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
			case HEADER_GD_SAVE_EXT_BATTLE_PASS:
				QUERY_SAVE_EXT_BATTLE_PASS(peer, dwHandle, (TPlayerExtBattlePassMission*)data);
				break;
#endif

#ifdef ENABLE_EVENT_MANAGER
			case HEADER_GD_UPDATE_EVENT_STATUS:
				UpdateEventStatus(*(uint32_t*)data);
				break;

			case HEADER_GD_EVENT_NOTIFICATION:
				EventNotification((TPacketSetEventFlag*)data);
				break;

			case HEADER_GD_RELOAD_EVENT:
				QUERY_RELOAD_EVENT();
				break;
#endif

			default:
				sys_err("Unknown header (header: %d handle: %d length: %d)", header, dwHandle, dwLength);
				break;
		}
	}

	peer->RecvEnd(i);
}

void CClientManager::AddPeer(socket_t fd)
{
	CPeer* pPeer = new CPeer;

	if (pPeer->Accept(fd))
		m_peerList.push_front(pPeer);
	else
		delete pPeer;
}

void CClientManager::RemovePeer(CPeer* pPeer)
{
	if (!pPeer)
		return;

	if (m_pkAuthPeer == pPeer)
	{
		m_pkAuthPeer = nullptr;
	}
	else
	{
		TLogonAccountMap::iterator it = m_map_kLogonAccount.begin();

		while (it != m_map_kLogonAccount.end())
		{
			CLoginData* pkLD = it->second;
			if (pkLD->GetConnectedPeerHandle() == pPeer->GetHandle())
			{
				if (pkLD->IsPlay())
				{
					pkLD->SetPlay(false);
				}

				if (pkLD->IsDeleted())
				{
					sys_log(0, "DELETING LoginData");
					delete pkLD;
				}

				m_map_kLogonAccount.erase(it++);
			}
			else
				++it;
		}
	}

	m_peerList.remove(pPeer);
	delete pPeer;
}

CPeer* CClientManager::GetPeer(IDENT ident)
{
	for (auto i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;
		if (tmp && (tmp->GetHandle() == ident))
			return tmp;
	}

	return nullptr;
}

CPeer* CClientManager::GetAnyPeer() noexcept
{
	if (m_peerList.empty())
		return nullptr;

	return m_peerList.front();
}

// Processes the result received from the DB manager.
//
// @version 05/06/10 Bang2ni-Add pricing related query (QID_ITEMPRICE_XXX)
int CClientManager::AnalyzeQueryResult(SQLMsg* msg)
{
	CQueryInfo* qi = (CQueryInfo*)msg->pvUserData;
	CPeer* peer = GetPeer(qi->dwIdent);

	switch (qi->iType)
	{
		case QID_ITEM_AWARD_LOAD:
			ItemAwardManager::Instance().Load(msg);
			delete qi;
			return true;

		case QID_GUILD_RANKING:
			CGuildManager::Instance().ResultRanking(msg->Get()->pSQLResult);
			break;

			// MYSHOP_PRICE_LIST
		case QID_ITEMPRICE_LOAD_FOR_UPDATE:
			RESULT_PRICELIST_LOAD_FOR_UPDATE(msg);
			break;
			// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case QID_LOAD_PLAYER_SHOPS:
			RESULT_LOAD_SHOPS(msg);
			break;

		case QID_LOAD_PLAYER_SHOP_ITEMS:
			RESULT_LOAD_PLAYER_SHOP_ITEMS(msg);
			break;
#endif

		default:
			break;
	}

	if (!peer)
	{
		//sys_err("CClientManager::AnalyzeQueryResult: peer not exist anymore. (ident: %d)", qi->dwIdent);
		delete qi;
		return true;
	}

	switch (qi->iType)
	{
		case QID_PLAYER:
		case QID_ITEM:
		case QID_QUEST:
		case QID_AFFECT:
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		case QID_SKILL_COLOR:
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case QID_EXT_BATTLE_PASS:
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case QID_ACHIEVEMENT_LOAD_DATA:
		case QID_ACHIEVEMENT_LOAD_ACHIEVEMENTS:
		case QID_ACHIEVEMENT_LOAD_TASKS:
#endif
			RESULT_COMPOSITE_PLAYER(peer, msg, qi->iType);
			break;

		case QID_LOGIN:
			RESULT_LOGIN(peer, msg);
			break;

		case QID_SAFEBOX_LOAD:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_LOAD");
			RESULT_SAFEBOX_LOAD(peer, msg);
			break;

		case QID_SAFEBOX_CHANGE_SIZE:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_SIZE");
			RESULT_SAFEBOX_CHANGE_SIZE(peer, msg);
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case QID_GUILDSTORAGE_CHANGE_SIZE:
			sys_log(0, "QUERY_RESULT: HEADER_GD_GUILDSTORAGE_CHANGE_SIZE");
			RESULT_GUILDSTORAGE_CHANGE_SIZE(peer, msg);
			break;
#endif

		case QID_SAFEBOX_CHANGE_PASSWORD:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_PASSWORD %p", msg);
			RESULT_SAFEBOX_CHANGE_PASSWORD(peer, msg);
			break;

		case QID_SAFEBOX_CHANGE_PASSWORD_SECOND:
			sys_log(0, "QUERY_RESULT: HEADER_GD_SAFEBOX_CHANGE_PASSWORD %p", msg);
			RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(peer, msg);
			break;

		case QID_HIGHSCORE_REGISTER:
			sys_log(0, "QUERY_RESULT: HEADER_GD_HIGHSCORE_REGISTER %p", msg);
			RESULT_HIGHSCORE_REGISTER(peer, msg);
			break;

		case QID_SAFEBOX_SAVE:
		case QID_ITEM_SAVE:
		case QID_ITEM_DESTROY:
		case QID_QUEST_SAVE:
		case QID_PLAYER_SAVE:
		case QID_ITEM_AWARD_TAKEN:
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		case QID_SKILL_COLOR_SAVE:
#endif
			break;

			// PLAYER_INDEX_CREATE_BUG_FIX
		case QID_PLAYER_INDEX_CREATE:
			RESULT_PLAYER_INDEX_CREATE(peer, msg);
			break;
			// END_PLAYER_INDEX_CREATE_BUG_FIX

		case QID_PLAYER_DELETE:
			__RESULT_PLAYER_DELETE(peer, msg);
			break;

		case QID_LOGIN_BY_KEY:
			RESULT_LOGIN_BY_KEY(peer, msg);
			break;

			// MYSHOP_PRICE_LIST
		case QID_ITEMPRICE_LOAD:
			RESULT_PRICELIST_LOAD(peer, msg);
			break;
			// END_OF_MYSHOP_PRICE_LIST

		default:
			sys_log(0, "CClientManager::AnalyzeQueryResult unknown query result type: %d, str: %s", qi->iType, msg->stQuery.c_str());
			break;
	}

	delete qi;
	return true;
}

void UsageLog()
{
	FILE* fp = nullptr;

	time_t ct;
	char* time_s;
	struct tm lt;

	const int avg = g_dwUsageAvg / 3600; // 60 seconds * 60 minutes

	fp = fopen("usage.txt", "a+");

	if (!fp)
		return;

	ct = time(0);
	lt = *localtime(&ct);
	time_s = asctime(&lt);

	time_s[strlen(time_s) - 1] = '\0';

	fprintf(fp, "| %4d %-15.15s | %5d | %5u |", lt.tm_year + 1900, time_s + 4, avg, g_dwUsageMax);

	fprintf(fp, "\n");
	fclose(fp);

	g_dwUsageMax = g_dwUsageAvg = 0;
}

#ifdef ENABLE_WEATHER_INFO
static uint8_t GetDayMode(int Hour)
{
	if (Hour >= DailyWeather::MIDNIGHT && Hour < DailyWeather::SUNRISE)			//Nacht [Früh]
		return DayMode::NIGHT;
	else if (Hour >= DailyWeather::SUNRISE && Hour < DailyWeather::NOON)		// Morgens
		return DayMode::MORNING;
	else if (Hour >= DailyWeather::NOON && Hour < DailyWeather::SUNSET)			// Mittags
		return DayMode::DAY;
	else if (Hour >= DailyWeather::SUNSET && Hour < DailyWeather::DARKNESS)		// Nachmittags
		return DayMode::EVENING;
	else if (Hour >= DailyWeather::DARKNESS && Hour <= DailyWeather::BEFORE_MID)	// Nacht [Abend]
		return DayMode::NIGHT;
	else
		return DayMode::DAY;

	return DayMode::DAY;
}

static bool IsWinter(int Month)	// 0 = Summer, 1 = Winter
{
	if (Month < 3)
		return false;
	else if (Month >= 3 && Month < 12)
		return true;
	else if (Month == 12)
		return false;

	return true;
}
#endif

int CClientManager::Process()
{
	int pulses;

	if (!(pulses = thecore_idle()))
		return 0;

	while (pulses--)
	{
		++thecore_heart->pulse;

		/*
		//Change every 30 minutes
		if (((thecore_pulse() % (60 * 30 * 10)) == 0))
		{
			g_iPlayerCacheFlushSeconds = MAX(60, rand() % 180);
			g_iItemCacheFlushSeconds = MAX(60, rand() % 180);
			sys_log(0, "[SAVE_TIME]Change saving time item %d player %d", g_iPlayerCacheFlushSeconds, g_iItemCacheFlushSeconds);
		}
		*/

		if (!(thecore_heart->pulse % thecore_heart->passes_per_sec))
		{
			if (g_test_server)
			{
				if (!(thecore_heart->pulse % thecore_heart->passes_per_sec * 10))
				{
					pt_log("[%9d] return %d/%d/%d/%d async %d/%d/%d/%d",
						thecore_heart->pulse,
						CDBManager::Instance().CountReturnQuery(SQL_PLAYER),
						CDBManager::Instance().CountReturnResult(SQL_PLAYER),
						CDBManager::Instance().CountReturnQueryFinished(SQL_PLAYER),
						CDBManager::Instance().CountReturnCopiedQuery(SQL_PLAYER),
						CDBManager::Instance().CountAsyncQuery(SQL_PLAYER),
						CDBManager::Instance().CountAsyncResult(SQL_PLAYER),
						CDBManager::Instance().CountAsyncQueryFinished(SQL_PLAYER),
						CDBManager::Instance().CountAsyncCopiedQuery(SQL_PLAYER));

					if ((thecore_heart->pulse % 50) == 0)
					{
						sys_log(0, "[%9d] return %d/%d/%d async %d/%d/%d",
							thecore_heart->pulse,
							CDBManager::Instance().CountReturnQuery(SQL_PLAYER),
							CDBManager::Instance().CountReturnResult(SQL_PLAYER),
							CDBManager::Instance().CountReturnQueryFinished(SQL_PLAYER),
							CDBManager::Instance().CountAsyncQuery(SQL_PLAYER),
							CDBManager::Instance().CountAsyncResult(SQL_PLAYER),
							CDBManager::Instance().CountAsyncQueryFinished(SQL_PLAYER));
					}
				}
			}
			else
			{
				pt_log("[%9d] return %d/%d/%d/%d async %d/%d/%d%/%d",
					thecore_heart->pulse,
					CDBManager::Instance().CountReturnQuery(SQL_PLAYER),
					CDBManager::Instance().CountReturnResult(SQL_PLAYER),
					CDBManager::Instance().CountReturnQueryFinished(SQL_PLAYER),
					CDBManager::Instance().CountReturnCopiedQuery(SQL_PLAYER),
					CDBManager::Instance().CountAsyncQuery(SQL_PLAYER),
					CDBManager::Instance().CountAsyncResult(SQL_PLAYER),
					CDBManager::Instance().CountAsyncQueryFinished(SQL_PLAYER),
					CDBManager::Instance().CountAsyncCopiedQuery(SQL_PLAYER));

				if ((thecore_heart->pulse % 50) == 0)
				{
					sys_log(0, "[%9d] return %d/%d/%d async %d/%d/%d",
						thecore_heart->pulse,
						CDBManager::Instance().CountReturnQuery(SQL_PLAYER),
						CDBManager::Instance().CountReturnResult(SQL_PLAYER),
						CDBManager::Instance().CountReturnQueryFinished(SQL_PLAYER),
						CDBManager::Instance().CountAsyncQuery(SQL_PLAYER),
						CDBManager::Instance().CountAsyncResult(SQL_PLAYER),
						CDBManager::Instance().CountAsyncQueryFinished(SQL_PLAYER));
				}
			}

			CDBManager::Instance().ResetCounter();

			const uint32_t dwCount = CClientManager::Instance().GetUserCount();

			g_dwUsageAvg += dwCount;
			g_dwUsageMax = MAX(g_dwUsageMax, dwCount);

			memset(&thecore_profiler[0], 0, sizeof(thecore_profiler));

			if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 3600)))
				UsageLog();

			m_iCacheFlushCount = 0;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
			// Skill color fush
			UpdateSkillColorCache();
#endif

			//Player flush
			UpdatePlayerCache();
			//Item flush
			UpdateItemCache();
			//Process on logout-flush the cache set
			UpdateLogoutPlayer();

			// MYSHOP_PRICE_LIST
			UpdateItemPriceListCache();
			// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			UpdateShopCache();
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
			UpdateAchievementsCache();
#endif

			CGuildManager::Instance().Update();
			CPrivManager::Instance().Update();
			marriage::CManager::Instance().Update();
		}

#ifdef ENABLE_ITEMAWARD_REFRESH
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 5))) // Recheck item_award every 5 seconds
		{
			ItemAwardManager::Instance().RequestLoad();
		}
#endif
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 10))) // output stats every 10 seconds
		{
			/*
			char buf[4096 + 1];
			int len
			itertype(g_query_info.m_map_info) it;

			/////////////////////////////////////////////////////////////////
			buf[0] = '\0';
			len = 0;

			it = g_query_info.m_map_info.begin();

			int count = 0;

			while (it != g_query_info.m_map_info.end())
			{
				len += snprintf(buf + len, sizeof(buf) - len, "%2d %3d\n", it->first, it->second);
				count += it->second;
				it++;
			}

			pt_log("QUERY:\n%s-------------------- MAX : %d\n", buf, count);
			g_query_info.Reset();
			*/
			pt_log("QUERY: MAIN[%d] ASYNC[%d]", g_query_count[0], g_query_count[1]);
			g_query_count[0] = 0;
			g_query_count[1] = 0;
			/////////////////////////////////////////////////////////////////

			/////////////////////////////////////////////////////////////////
			/*
			buf[0] = '\0';
			len = 0;

			it = g_item_info.m_map_info.begin();

			count = 0;
			while (it != g_item_info.m_map_info.end())
			{
				len += snprintf(buf + len, sizeof(buf) - len, "%5d %3d\n", it->first, it->second);
				count += it->second;
				it++;
			}

			pt_log("ITEM:\n%s-------------------- MAX : %d\n", buf, count);
			g_item_info.Reset();
			*/
			pt_log("ITEM:%d\n", g_item_count);
			g_item_count = 0;
			/////////////////////////////////////////////////////////////////
		}

		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 60))) // Once every 60 seconds
		{
			// Spend time for unique items.
			CClientManager::Instance().SendTime();

			// Displays the host and port of the currently connected peer.
			std::string st;
			CClientManager::Instance().GetPeerP2PHostNames(st);
			sys_log(0, "Current Peer host names...\n%s", st.c_str());
		}

		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 3600))) // Once an hour
		{
			CMoneyLog::Instance().Save();
		}

#ifdef ENABLE_MAILBOX
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * m_iMailBoxBackupSec)))
		{
			CClientManager::Instance().MAILBOX_BACKUP();
		}
#endif

#ifdef ENABLE_WEATHER_INFO
		static bool OnSetup = true;
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * WEATHER_REFRESH_TIMER)) || OnSetup)
		{
			time_t CurrentTime = time(nullptr);
			tm* tm = localtime(&CurrentTime);

			TPacketDGWeatherInfo weatherInfo;
			weatherInfo.bDayMode = GetDayMode(tm->tm_hour);
			weatherInfo.bSeason = IsWinter(tm->tm_mon);
			CClientManager::Instance().ForwardPacket(HEADER_DG_WEATHER_INFO, &weatherInfo, sizeof(TPacketDGWeatherInfo));

			OnSetup = false;
		}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 1800)))
		{
			UpdateAchievementsRanking();
		}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		if (!(thecore_heart->pulse % (thecore_heart->passes_per_sec * 10)))
		{
			CLottoManager::instance().CheckRefreshTime();
		}
#endif
	}

	const int num_events = fdwatch(m_fdWatcher, 0);
	int idx;
	CPeer* peer;

	for (idx = 0; idx < num_events; ++idx) // Input
	{
		peer = (CPeer*)fdwatch_get_client_data(m_fdWatcher, idx);

		if (!peer)
		{
			if (fdwatch_check_event(m_fdWatcher, m_fdAccept, idx) == FDW_READ)
			{
				AddPeer(m_fdAccept);
				fdwatch_clear_event(m_fdWatcher, m_fdAccept, idx);
			}
			else
			{
				sys_log(0, "FDWATCH: peer null in event: ident %d", fdwatch_get_ident(m_fdWatcher, idx)); // @warme012
			}

			continue;
		}

		switch (fdwatch_check_event(m_fdWatcher, peer->GetFd(), idx))
		{
			case FDW_READ:
				if (peer->Recv() < 0)
				{
					sys_err("Recv failed");
					RemovePeer(peer);
				}
				else
				{
					if (peer == m_pkAuthPeer)
					{
						if (g_log)
							sys_log(0, "AUTH_PEER_READ: size %d", peer->GetRecvLength());
					}

					ProcessPackets(peer);
				}
				break;

			case FDW_WRITE:
				if (peer == m_pkAuthPeer)
				{
					if (g_log)
						sys_log(0, "AUTH_PEER_WRITE: size %d", peer->GetSendLength());
				}

				if (peer->Send() < 0)
				{
					sys_err("Send failed");
					RemovePeer(peer);
				}

				break;

			case FDW_EOF:
				RemovePeer(peer);
				break;

			default:
				sys_err("fdwatch_check_fd returned unknown result");
				RemovePeer(peer);
				break;
		}
	}

#ifdef __WIN32__
	if (_kbhit())
	{
		const int c = _getch();
		switch (c)
		{
			case 0x1b: // Esc
				return 0; // shutdown
				break;

			default:
				break;
		}
	}
#endif

	return 1;
}

uint32_t CClientManager::GetUserCount() noexcept
{
	// Simply count the login count. --;
	return m_map_kLogonAccount.size();
}

void CClientManager::SendAllGuildSkillRechargePacket()
{
	ForwardPacket(HEADER_DG_GUILD_SKILL_RECHARGE, nullptr, 0);
}

void CClientManager::SendTime()
{
	const time_t now = GetCurrentTime();
	ForwardPacket(HEADER_DG_TIME, &now, sizeof(time_t));
}

void CClientManager::ForwardPacket(uint8_t header, const void* data, int size, uint8_t bChannel, CPeer* except)
{
	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* peer = *it;
		if (!peer)
			continue;

		if (peer == except)
			continue;

		if (!peer->GetChannel())
			continue;

		if (bChannel && peer->GetChannel() != bChannel)
			continue;

		peer->EncodeHeader(header, 0, size);

		if (size > 0 && data)
			peer->Encode(data, size);
	}
}

void CClientManager::SendNotice(const char* c_pszFormat, ...)
{
	char szBuf[255 + 1];
	va_list args;

	va_start(args, c_pszFormat);
	int len = vsnprintf(szBuf, sizeof(szBuf), c_pszFormat, args);
	va_end(args);
	szBuf[len] = '\0';

	ForwardPacket(HEADER_DG_NOTICE, szBuf, len + 1);
}

time_t CClientManager::GetCurrentTime() noexcept
{
	return time(0);
}

// ITEM_UNIQUE_ID
bool CClientManager::InitializeNowItemID()
{
	uint32_t dwMin, dwMax;

	//Initialize the item ID.
	if (!CConfig::Instance().GetTwoValue("ITEM_ID_RANGE", &dwMin, &dwMax))
	{
		sys_err("conf.txt: Cannot find ITEM_ID_RANGE [start_item_id] [end_item_id]");
		return false;
	}

	sys_log(0, "ItemRange From File %u ~ %u ", dwMin, dwMax);

	if (CItemIDRangeManager::Instance().BuildRange(dwMin, dwMax, m_itemRange) == false)
	{
		sys_err("Can not build ITEM_ID_RANGE");
		return false;
	}

	sys_log(0, " Init Success Start %u End %u Now %u\n", m_itemRange.dwMin, m_itemRange.dwMax, m_itemRange.dwUsableItemIDMin);

	return true;
}

uint32_t CClientManager::GainItemID() noexcept
{
	return m_itemRange.dwUsableItemIDMin++;
}

uint32_t CClientManager::GetItemID() noexcept
{
	return m_itemRange.dwUsableItemIDMin;
}
// ITEM_UNIQUE_ID_END
//BOOT_LOCALIZATION

bool CClientManager::InitializeLocalization()
{
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery), "SELECT mValue, mKey FROM locale");
	auto pMsg = CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON);

	if (pMsg->Get()->uiNumRows == 0)
	{
		sys_err("InitializeLocalization() ==> DirectQuery failed(%s)", szQuery);
		return false;
	}

	sys_log(0, "InitializeLocalization() - LoadLocaleTable(count:%d)", pMsg->Get()->uiNumRows);

	m_vec_Locale.clear();

	MYSQL_ROW row = nullptr;

	for (int n = 0; (row = mysql_fetch_row(pMsg->Get()->pSQLResult)) != nullptr; ++n)
	{
		int col = 0;
		tLocale locale;

		strlcpy(locale.szValue, row[col++], sizeof(locale.szValue));
		strlcpy(locale.szKey, row[col++], sizeof(locale.szKey));

		//DB_NAME_COLUMN Setting
		if (strcmp(locale.szKey, "LOCALE") == 0)
		{
			if (strcmp(locale.szValue, "cibn") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "gb2312");

				g_stLocale = "gb2312";
				g_stLocaleNameColumn = "gb2312name";
			}
			else if (strcmp(locale.szValue, "ymir") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "euckr";
				g_stLocaleNameColumn = "name";
			}
			else if (strcmp(locale.szValue, "japan") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "sjis");

				g_stLocale = "sjis";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "english") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "germany") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "france") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "italy") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "spain") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "uk") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "turkey") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin5";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "poland") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin2";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "portugal") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "hongkong") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "big5");

				g_stLocale = "big5";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "newcibn") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "gb2312");

				g_stLocale = "gb2312";
				g_stLocaleNameColumn = "gb2312name";
			}
			else if (strcmp(locale.szValue, "korea") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "euckr";
				g_stLocaleNameColumn = "name";
			}
			else if (strcmp(locale.szValue, "canada") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "gb2312name";
			}
			else if (strcmp(locale.szValue, "brazil") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "greek") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "greek";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "russia") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "cp1251";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "denmark") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "bulgaria") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "cp1251";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "croatia") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "cp1251";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "mexico") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "arabia") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "cp1256";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "czech") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin2";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "hungary") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin2";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "romania") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin2";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "netherlands") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "singapore") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "vietnam") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "thailand") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "usa") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
			else if (strcmp(locale.szValue, "we_korea") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "euckr");

				g_stLocale = "euckr";
				g_stLocaleNameColumn = "name";
			}
			else if (strcmp(locale.szValue, "taiwan") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "big5");
				g_stLocale = "big5";
				g_stLocaleNameColumn = "locale_name";
			}
//#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			else if (strcmp(locale.szValue, "europe") == 0)
			{
				sys_log(0, "locale[LOCALE] = %s", locale.szValue);

				if (g_stLocale != locale.szValue)
					sys_log(0, "Changed g_stLocale %s to %s", g_stLocale.c_str(), "latin1");

				g_stLocale = "latin1";
				g_stLocaleNameColumn = "locale_name";
			}
//#endif
			else
			{
				sys_err("locale[LOCALE] = UNKNOWN(%s)", locale.szValue);
				exit(0);
			}
			// @warme007
			// sys_log(0,"before call SetLocale: %s",g_stLocale.c_str());
			// CDBManager::Instance().SetLocale(g_stLocale.c_str());
			// sys_log(0,"Called SetLocale");
		}
		else if (strcmp(locale.szKey, "DB_NAME_COLUMN") == 0)
		{
			sys_log(0, "locale[DB_NAME_COLUMN] = %s", locale.szValue);
			g_stLocaleNameColumn = locale.szValue;
		}
		else
		{
			sys_log(0, "locale[UNKNOWN_KEY(%s)] = %s", locale.szKey, locale.szValue);
		}
		m_vec_Locale.emplace_back(locale);
	}

	return true;
}
//END_BOOT_LOCALIZATION
//ADMIN_MANAGER

bool CClientManager::__GetAdminInfo(const char* szIP, std::vector<tAdminInfo>& rAdminVec)
{
	//If szIP == nullptr, all servers have operator privileges.
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT mID,mAccount,mName,mContactIP,mServerIP,mAuthority FROM gmlist WHERE mServerIP='ALL' or mServerIP='%s'",
		szIP ? szIP : "ALL");

	auto pMsg = CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON);
	if (!pMsg)
		return false;

	if (pMsg->Get()->uiNumRows == 0)
	{
		// sys_err("__GetAdminInfo() ==> DirectQuery failed(%s)", szQuery); // @warme013
		return false;
	}

	MYSQL_ROW row;
	rAdminVec.reserve(static_cast<uint32_t>(pMsg->Get()->uiNumRows));

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		int idx = 0;
		tAdminInfo Info{};

		str_to_number(Info.m_ID, row[idx++]);
		trim_and_lower(row[idx++], Info.m_szAccount, sizeof(Info.m_szAccount));
		strlcpy(Info.m_szName, row[idx++], sizeof(Info.m_szName));
		strlcpy(Info.m_szContactIP, row[idx++], sizeof(Info.m_szContactIP));
		strlcpy(Info.m_szServerIP, row[idx++], sizeof(Info.m_szServerIP));
		std::string stAuth = row[idx++];

		if (!stAuth.compare("IMPLEMENTOR"))
			Info.m_Authority = GM_IMPLEMENTOR;
		else if (!stAuth.compare("GOD"))
			Info.m_Authority = GM_GOD;
		else if (!stAuth.compare("HIGH_WIZARD"))
			Info.m_Authority = GM_HIGH_WIZARD;
		else if (!stAuth.compare("LOW_WIZARD"))
			Info.m_Authority = GM_LOW_WIZARD;
		else if (!stAuth.compare("WIZARD"))
			Info.m_Authority = GM_WIZARD;
#ifdef ENABLE_VIP_SYSTEM
		else if (!stAuth.compare("VIP"))
			Info.m_Authority = GM_VIP;
#endif
		else
			continue;

		rAdminVec.emplace_back(Info);

		sys_log(0, "GM: PID %u Login %s Character %s ContactIP %s ServerIP %s Authority %d[%s]",
			Info.m_ID, Info.m_szAccount, Info.m_szName, Info.m_szContactIP, Info.m_szServerIP, Info.m_Authority, stAuth.c_str());
	}

	return true;
}

bool CClientManager::__GetHostInfo(std::vector<std::string>& rIPVec)
{
	char szQuery[512];
	snprintf(szQuery, sizeof(szQuery), "SELECT mIP FROM gmhost");
	auto pMsg = CDBManager::Instance().DirectQuery(szQuery, SQL_COMMON);
	if (!pMsg)
		return false;

	if (pMsg->Get()->uiNumRows == 0)
	{
		// sys_err("__GetHostInfo() ==> DirectQuery failed(%s)", szQuery); // @warme013
		return false;
	}

	rIPVec.reserve(static_cast<uint32_t>(pMsg->Get()->uiNumRows));

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		if (row[0] && *row[0])
		{
			rIPVec.emplace_back(row[0]);
			sys_log(0, "GMHOST: %s", row[0]);
		}
	}

	return true;
}
//END_ADMIN_MANAGER

void CClientManager::ReloadAdmin(CPeer*, TPacketReloadAdmin* p)
{
	std::vector<tAdminInfo> vAdmin;
	std::vector<std::string> vHost;

	__GetHostInfo(vHost);
	__GetAdminInfo(p->szIP, vAdmin);

	const uint32_t dwPacketSize = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(tAdminInfo) * vAdmin.size() +
		sizeof(uint16_t) + sizeof(uint16_t) + 16 * vHost.size();

	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* peer = *it;
		if (!peer)
			continue;

		if (!peer->GetChannel())
			continue;

		peer->EncodeHeader(HEADER_DG_RELOAD_ADMIN, 0, dwPacketSize);

		peer->EncodeWORD(16);
		peer->EncodeWORD(static_cast<uint16_t>(vHost.size()));

		for (size_t n = 0; n < vHost.size(); ++n)
			peer->Encode(vHost[n].c_str(), 16);

		peer->EncodeWORD(sizeof(tAdminInfo));
		peer->EncodeWORD(static_cast<uint16_t>(vAdmin.size()));

		for (size_t n = 0; n < vAdmin.size(); ++n)
			peer->Encode(&vAdmin[n], sizeof(tAdminInfo));
	}

	sys_log(0, "ReloadAdmin End %s", p->szIP);
}

//BREAK_MARRIAGE
void CClientManager::BreakMarriage(CPeer* peer, const char* data)
{
	uint32_t pid1, pid2;

	pid1 = *(int*)data;
	data += sizeof(int);

	pid2 = *(int*)data;
	data += sizeof(int);

	sys_log(0, "Breaking off a marriage engagement! pid %d and pid %d", pid1, pid2);
	marriage::CManager::Instance().Remove(pid1, pid2);
}
//END_BREAK_MARIIAGE

void CClientManager::UpdateItemCacheSet(uint32_t pid)
{
	const auto it = m_map_pkItemCacheSetPtr.find(pid);

	if (it == m_map_pkItemCacheSetPtr.end())
	{
		if (g_test_server)
			sys_log(0, "UPDATE_ITEMCACHESET : UpdateItemCacheSet ==> No ItemCacheSet pid(%d)", pid);
		return;
	}

	TItemCacheSet* pSet = it->second;
	if (!pSet)
		return;

	TItemCacheSet::iterator it_set = pSet->begin();

	while (it_set != pSet->end())
	{
		CItemCache* c = *it_set++;
		if (c)
			c->Flush();
	}

	if (g_log)
		sys_log(0, "UPDATE_ITEMCACHESET : UpdateItemCachsSet pid(%d)", pid);
}

void CClientManager::Election(CPeer* peer, uint32_t dwHandle, const char* data)
{
	if (!peer)
		return;

	uint32_t idx;
	uint32_t selectingpid;

	idx = *(uint32_t*)data;
	data += sizeof(uint32_t);

	selectingpid = *(uint32_t*)data;
	data += sizeof(uint32_t);

	int Success = 0;

	if (!(Success = CMonarch::Instance().VoteMonarch(selectingpid, idx)))
	{
		if (g_test_server)
			sys_log(0, "[MONARCH_VOTE] Failed %d %d", idx, selectingpid);
		peer->EncodeHeader(HEADER_DG_ELECT_MONARCH, dwHandle, sizeof(int));
		peer->Encode(&Success, sizeof(int));
		return;
	}
	else
	{
		if (g_test_server)
			sys_log(0, "[MONARCH_VOTE] Success %d %d", idx, selectingpid);
		peer->EncodeHeader(HEADER_DG_ELECT_MONARCH, dwHandle, sizeof(int));
		peer->Encode(&Success, sizeof(int));
		return;
	}

}
void CClientManager::Candidacy(CPeer* peer, uint32_t dwHandle, const char* data)
{
	if (!peer)
		return;

	uint32_t pid;

	pid = *(uint32_t*)data;
	data += sizeof(uint32_t);

	if (!CMonarch::Instance().AddCandidacy(pid, data))
	{
		if (g_test_server)
			sys_log(0, "[MONARCH_CANDIDACY] Failed %d %s", pid, data);

		peer->EncodeHeader(HEADER_DG_CANDIDACY, dwHandle, sizeof(int) + 32);
		peer->Encode(0, sizeof(int));
		peer->Encode(data, 32);
		return;
	}
	else
	{
		if (g_test_server)
			sys_log(0, "[MONARCH_CANDIDACY] Success %d %s", pid, data);

		for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
		{
			CPeer* p = *it;
			if (!p)
				continue;

			if (!p->GetChannel())
				continue;

			/*if (0 && p->GetChannel() != 0)
				continue;*/

			if (p == peer)
			{
				p->EncodeHeader(HEADER_DG_CANDIDACY, dwHandle, sizeof(int) + 32);
				p->Encode(&pid, sizeof(int));
				p->Encode(data, 32);
			}
			else
			{
				p->EncodeHeader(HEADER_DG_CANDIDACY, 0, sizeof(int) + 32);
				p->Encode(&pid, sizeof(int));
				p->Encode(data, 32);
			}
		}
	}
}

void CClientManager::AddMonarchMoney(const CPeer* peer, uint32_t dwHandle, const char* data)
{
	int Empire = *(int*)data;
	data += sizeof(int);

	int Money = *(int*)data;
	data += sizeof(int);

	if (g_test_server)
		sys_log(0, "[MONARCH] Add money Empire(%d) Money(%d)", Empire, Money);

	CMonarch::Instance().AddMoney(Empire, Money);

	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* p = *it;
		if (!p)
			continue;

		if (!p->GetChannel())
			continue;

		if (p == peer)
		{
			p->EncodeHeader(HEADER_DG_ADD_MONARCH_MONEY, dwHandle, sizeof(int) + sizeof(int));
			p->Encode(&Empire, sizeof(int));
			p->Encode(&Money, sizeof(int));
		}
		else
		{
			p->EncodeHeader(HEADER_DG_ADD_MONARCH_MONEY, 0, sizeof(int) + sizeof(int));
			p->Encode(&Empire, sizeof(int));
			p->Encode(&Money, sizeof(int));
		}

	}
}

void CClientManager::DecMonarchMoney(const CPeer* peer, uint32_t dwHandle, const char* data)
{
	int Empire = *(int*)data;
	data += sizeof(int);

	int Money = *(int*)data;
	data += sizeof(int);

	if (g_test_server)
		sys_log(0, "[MONARCH] Dec money Empire(%d) Money(%d)", Empire, Money);

	CMonarch::Instance().DecMoney(Empire, Money);

	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* p = *it;
		if (!p)
			continue;

		if (!p->GetChannel())
			continue;

		if (p == peer)
		{
			p->EncodeHeader(HEADER_DG_DEC_MONARCH_MONEY, dwHandle, sizeof(int) + sizeof(int));
			p->Encode(&Empire, sizeof(int));
			p->Encode(&Money, sizeof(int));
		}
		else
		{
			p->EncodeHeader(HEADER_DG_DEC_MONARCH_MONEY, 0, sizeof(int) + sizeof(int));
			p->Encode(&Empire, sizeof(int));
			p->Encode(&Money, sizeof(int));
		}
	}
}

void CClientManager::TakeMonarchMoney(CPeer* peer, uint32_t dwHandle, const char* data)
{
	if (!peer)
		return;

	int Empire = *(int*)data;
	data += sizeof(int);

	uint32_t pid = *(uint32_t*)data;
	data += sizeof(int);

	int Money = *(int*)data;
	data += sizeof(int);

	if (g_test_server)
		sys_log(0, "[MONARCH] Take money Empire(%d) Money(%d)", Empire, Money);

	if (CMonarch::Instance().TakeMoney(Empire, pid, Money) == true)
	{
		peer->EncodeHeader(HEADER_DG_TAKE_MONARCH_MONEY, dwHandle, sizeof(int) + sizeof(int));
		peer->Encode(&Empire, sizeof(int));
		peer->Encode(&Money, sizeof(int));
	}
	else
	{
		Money = 0;
		peer->EncodeHeader(HEADER_DG_TAKE_MONARCH_MONEY, dwHandle, sizeof(int) + sizeof(int));
		peer->Encode(&Empire, sizeof(int));
		peer->Encode(&Money, sizeof(int));
	}
}

void CClientManager::ComeToVote(CPeer* peer, uint32_t dwHandle, const char* data)
{
	CMonarch::Instance().ElectMonarch();
}

void CClientManager::RMCandidacy(CPeer* peer, uint32_t dwHandle, const char* data)
{
	char szName[32];

	strlcpy(szName, data, sizeof(szName));
	sys_log(0, "[MONARCH_GM] Remove candidacy name(%s)", szName);

	const int iRet = CMonarch::Instance().DelCandidacy(szName) ? 1 : 0;

	if (1 == iRet)
	{
		for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
		{
			CPeer* p = *it;
			if (!p)
				continue;

			if (!p->GetChannel())
				continue;

			if (p == peer)
			{
				p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
				p->Encode(&iRet, sizeof(int));
				p->Encode(szName, sizeof(szName));
			}
			else
			{
				p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
				p->Encode(&iRet, sizeof(int));
				p->Encode(szName, sizeof(szName));
			}
		}
	}
	else
	{
		CPeer* p = peer;
		if (p)
		{
			p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
			p->Encode(&iRet, sizeof(int));
			p->Encode(szName, sizeof(szName));
		}
	}
}

void CClientManager::SetMonarch(CPeer* peer, uint32_t dwHandle, const char* data)
{
	char szName[32];

	strlcpy(szName, data, sizeof(szName));

	if (g_test_server)
		sys_log(0, "[MONARCH_GM] Set Monarch name(%s)", szName);

	const int iRet = CMonarch::Instance().SetMonarch(szName) ? 1 : 0;

	if (1 == iRet)
	{
		for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
		{
			CPeer* p = *it;
			if (!p)
				continue;

			if (!p->GetChannel())
				continue;

			if (p == peer)
			{
				p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
				p->Encode(&iRet, sizeof(int));
				p->Encode(szName, sizeof(szName));
			}
			else
			{
				p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
				p->Encode(&iRet, sizeof(int));
				p->Encode(szName, sizeof(szName));
			}
		}
	}
	else
	{
		CPeer* p = peer;
		if (p)
		{
			p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
			p->Encode(&iRet, sizeof(int));
			p->Encode(szName, sizeof(szName));
		}
	}
}

void CClientManager::RMMonarch(CPeer* peer, uint32_t dwHandle, const char* data)
{
	char szName[32];

	strlcpy(szName, data, sizeof(szName));

	if (g_test_server)
		sys_log(0, "[MONARCH_GM] Remove Monarch name(%s)", szName);

	CMonarch::Instance().DelMonarch(szName);

	const int iRet = CMonarch::Instance().DelMonarch(szName) ? 1 : 0;

	if (1 == iRet)
	{
		for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
		{
			CPeer* p = *it;
			if (!p)
				continue;

			if (!p->GetChannel())
				continue;

			if (p == peer)
			{
				p->EncodeHeader(HEADER_DG_RMMONARCH, dwHandle, sizeof(int) + sizeof(szName));
				p->Encode(&iRet, sizeof(int));
				p->Encode(szName, sizeof(szName));
			}
			else
			{
				p->EncodeHeader(HEADER_DG_RMMONARCH, dwHandle, sizeof(int) + sizeof(szName));
				p->Encode(&iRet, sizeof(int));
				p->Encode(szName, sizeof(szName));
			}
		}
	}
	else
	{
		CPeer* p = peer;
		if (p)
		{
			p->EncodeHeader(HEADER_DG_RMCANDIDACY, dwHandle, sizeof(int) + sizeof(szName));
			p->Encode(&iRet, sizeof(int));
			p->Encode(szName, sizeof(szName));
		}
	}
}

void CClientManager::ChangeMonarchLord(CPeer* peer, uint32_t dwHandle, TPacketChangeMonarchLord* info)
{
	char szQuery[1024];
	snprintf(szQuery, sizeof(szQuery),
		"SELECT a.name, NOW() FROM player%s AS a, player_index%s AS b WHERE (a.account_id=b.id AND a.id=%u AND b.empire=%u) AND "
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
		"(b.pid1=%u OR b.pid2=%u OR b.pid3=%u OR b.pid4=%u OR b.pid5=%u)",
#else
		"(b.pid1=%u OR b.pid2=%u OR b.pid3=%u OR b.pid4=%u)",
#endif
		GetTablePostfix(), GetTablePostfix(), info->dwPID, info->bEmpire,
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
		info->dwPID, info->dwPID, info->dwPID, info->dwPID, info->dwPID);
#else
		info->dwPID, info->dwPID, info->dwPID, info->dwPID);
#endif

	auto pMsg = CDBManager::Instance().DirectQuery(szQuery, SQL_PLAYER);
	if (!pMsg)
		return;

	if (pMsg->Get()->uiNumRows != 0)
	{
		TPacketChangeMonarchLordACK ack{};
		ack.bEmpire = info->bEmpire;
		ack.dwPID = info->dwPID;

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		strlcpy(ack.szName, row[0], sizeof(ack.szName));
		strlcpy(ack.szDate, row[1], sizeof(ack.szDate));

		snprintf(szQuery, sizeof(szQuery), "UPDATE monarch SET pid=%u, windate=NOW() WHERE empire=%d", ack.dwPID, ack.bEmpire);

		auto pMsg2 = CDBManager::Instance().DirectQuery(szQuery, SQL_PLAYER);
		if (pMsg2 && pMsg2->Get()->uiAffectedRows > 0)
		{
			CMonarch::Instance().LoadMonarch();

			const TMonarchInfo* newInfo = CMonarch::Instance().GetMonarch();

			for (auto it = m_peerList.begin(); it != m_peerList.end(); it++)
			{
				CPeer* client = *it;
				if (client)
				{
					client->EncodeHeader(HEADER_DG_CHANGE_MONARCH_LORD_ACK, 0, sizeof(TPacketChangeMonarchLordACK));
					client->Encode(&ack, sizeof(TPacketChangeMonarchLordACK));

					client->EncodeHeader(HEADER_DG_UPDATE_MONARCH_INFO, 0, sizeof(TMonarchInfo));
					client->Encode(newInfo, sizeof(TMonarchInfo));
				}
			}
		}

	}

}

void CClientManager::BlockException(TPacketBlockException* data)
{
	sys_log(0, "[BLOCK_EXCEPTION] CMD(%d) login(%s)", data->cmd, data->login);

	// save sql
	{
		char buf[1024];

		switch (data->cmd)
		{
			case BLOCK_EXCEPTION_CMD_ADD:
				snprintf(buf, sizeof(buf), "INSERT INTO block_exception VALUES('%s')", data->login);
				CDBManager::Instance().AsyncQuery(buf, SQL_ACCOUNT);
				CBlockCountry::Instance().AddBlockException(data->login);
				break;
			case BLOCK_EXCEPTION_CMD_DEL:
				snprintf(buf, sizeof(buf), "DELETE FROM block_exception VALUES('%s')", data->login);
				CDBManager::Instance().AsyncQuery(buf, SQL_ACCOUNT);
				CBlockCountry::Instance().DelBlockException(data->login);
				break;
			default:
				return;
		}

	}

	for (auto it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		CPeer* peer = *it;
		if (!peer)
			continue;

		if (!peer->GetChannel())
			continue;

		CBlockCountry::Instance().SendBlockExceptionOne(peer, data->login, data->cmd);
	}
}

void CClientManager::SendSpareItemIDRange(CPeer* peer)
{
	if (peer)
		peer->SendSpareItemIDRange();
}

//
// Only the login key is deleted from the map.
//
void CClientManager::DeleteLoginKey(const TPacketDC* data)
{
	char login[LOGIN_MAX_LEN + 1] = { 0 };
	trim_and_lower(data->login, login, sizeof(login));

	CLoginData* pkLD = GetLoginDataByLogin(login);
	if (pkLD)
	{
		const TLoginDataByLoginKey::iterator it = m_map_pkLoginData.find(pkLD->GetKey());

		if (it != m_map_pkLoginData.end())
			m_map_pkLoginData.erase(it);
	}
}

// delete gift notify icon
void CClientManager::DeleteAwardId(const TPacketDeleteAwardID* data)
{
	//sys_log(0,"data from game server arrived %d",data->dwID);
	std::map<uint32_t, TItemAward*>::iterator it;
	it = ItemAwardManager::Instance().GetMapAward().find(data->dwID);
	if (it != ItemAwardManager::Instance().GetMapAward().end())
	{
		std::set<TItemAward*>& kSet = ItemAwardManager::Instance().GetMapkSetAwardByLogin()[it->second->szLogin];
		if (kSet.erase(it->second))
			sys_log(0, "erase ItemAward id: %d from cache", data->dwID);
		ItemAwardManager::Instance().GetMapAward().erase(data->dwID);
	}
	else
	{
		sys_log(0, "DELETE_AWARDID : could not find the id: %d", data->dwID);
	}
}

void CClientManager::UpdateChannelStatus(TChannelStatus* pData)
{
	const TChannelStatusMap::iterator it = m_mChannelStatus.find(pData->nPort);
	if (it != m_mChannelStatus.end())
		it->second = pData->bStatus;
	else
		m_mChannelStatus.insert(TChannelStatusMap::value_type(pData->nPort, pData->bStatus));
}

void CClientManager::RequestChannelStatus(CPeer* peer, uint32_t dwHandle)
{
	if (!peer)
		return;

	const int nSize = m_mChannelStatus.size();
	peer->EncodeHeader(HEADER_DG_RESPOND_CHANNELSTATUS, dwHandle, sizeof(TChannelStatus) * nSize + sizeof(int));
	peer->Encode(&nSize, sizeof(int));
	for (TChannelStatusMap::iterator it = m_mChannelStatus.begin(); it != m_mChannelStatus.end(); it++)
	{
		peer->Encode(&it->first, sizeof(int16_t));
		peer->Encode(&it->second, sizeof(uint8_t));
	}
}

void CClientManager::ResetLastPlayerID(const TPacketNeedLoginLogInfo* data)
{
	CLoginData* pkLD = GetLoginDataByAID(data->dwPlayerID);

	if (nullptr != pkLD)
	{
		pkLD->SetLastPlayerID(0);
	}
}

void CClientManager::ChargeCash(const TRequestChargeCash* packet)
{
	char szQuery[512];

	if (ERequestCharge_Cash == packet->eChargeType)
		sprintf(szQuery, "UPDATE account SET cash = cash + %d WHERE id = %d LIMIT 1", packet->dwAmount, packet->dwAID);
	else if (ERequestCharge_Mileage == packet->eChargeType)
		sprintf(szQuery, "UPDATE account SET mileage = mileage + %d WHERE id = %d LIMIT 1", packet->dwAmount, packet->dwAID);
	else
	{
		sys_err("Invalid request charge type (type : %d, amount : %d, aid : %d)", packet->eChargeType, packet->dwAmount, packet->dwAID);
		return;
	}

	sys_log(0, "Request Charge (type : %d, amount : %d, aid : %d)", packet->eChargeType, packet->dwAmount, packet->dwAID);

	CDBManager::Instance().AsyncQuery(szQuery, SQL_ACCOUNT);
}

#ifdef ENABLE_YOHARA_SYSTEM
void CClientManager::SungmaPremium(CPeer* peer, uint32_t dwHandle, uint32_t dwAccID)
{
	if (!peer)
		return;

	const time_t tNow = CClientManager::Instance().GetCurrentTime();

	char szQuery[512];
	sprintf(szQuery, "UPDATE account SET sungma_expire %u WHERE id = %d", tNow + 864000, dwAccID);
	CDBManager::Instance().AsyncQuery(szQuery, SQL_ACCOUNT);
}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
void CClientManager::GuildDungeon(const TPacketGDGuildDungeon* sPacket)
{
	CGuildManager::Instance().Dungeon(sPacket->dwGuildID, sPacket->bChannel, sPacket->lMapIndex);
}

void CClientManager::GuildDungeonGD(const TPacketGDGuildDungeonCD* sPacket)
{
	CGuildManager::Instance().DungeonCooldown(sPacket->dwGuildID, sPacket->dwTime);
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
void CClientManager::LanguageChange(CPeer* pPeer, const char* c_szData)
{
	TPacketGDLangaugeChange* p = (TPacketGDLangaugeChange*)c_szData;

	CLoginData* pLoginData = GetLoginDataByAID(p->dwAID);

	if (p->bState)
	{
		if (!pLoginData)
			return;

		if (pLoginData->GetAccountRef().bLanguage == p->bLanguage)
			return;

		pLoginData->GetAccountRef().bLanguage = p->bLanguage;
	}

	char szQuery[512];
	sprintf(szQuery, "UPDATE account set `lang` = %u WHERE id = %d LIMIT 1", p->bLanguage, p->dwAID);

	CDBManager::Instance().AsyncQuery(szQuery, SQL_ACCOUNT);
}
#endif

#ifdef ENABLE_EVENT_MANAGER
void CClientManager::UpdateEventStatus(uint32_t dwID)
{
	char buf[128] = { '\0' };
	snprintf(&buf[0], sizeof(buf), "UPDATE event%s SET completed = 1 WHERE id=%u", GetTablePostfix(), dwID);
	CDBManager::Instance().AsyncQuery(&buf[0]);

	for (auto it = m_vec_eventTable.begin(); it != m_vec_eventTable.end(); ++it)
	{
		if (it->dwID == dwID)
			it->bCompleted = true;
	}

	ForwardPacket(HEADER_DG_UPDATE_EVENT_STATUS, &dwID, sizeof(uint32_t));
}

void CClientManager::EventNotification(TPacketSetEventFlag* p)
{
	ForwardPacket(HEADER_DG_EVENT_NOTIFICATION, p, sizeof(TPacketSetEventFlag));

	bool bChanged = false;

	const auto it = m_map_lEventFlag.find(p->szFlagName);
	if (it == m_map_lEventFlag.end())
	{
		bChanged = true;
		m_map_lEventFlag.insert(std::make_pair(std::string(p->szFlagName), p->lValue));
	}
	else if (it->second != p->lValue)
	{
		bChanged = true;
		it->second = p->lValue;
	}

	if (bChanged)
	{
		char szQuery[1024];
		snprintf(szQuery, sizeof(szQuery),
			"REPLACE INTO quest%s (dwPID, szName, szState, lValue) VALUES(0, '%s', '', %ld)",
			GetTablePostfix(), p->szFlagName, p->lValue);
		szQuery[1023] = '\0';

		CDBManager::Instance().AsyncQuery(szQuery);
		sys_log(0, "HEADER_GD_SET_EVENT_FLAG : Changed CClientmanager::SetEventFlag(%s %d) ", p->szFlagName, p->lValue);
		return;
	}
	sys_log(0, "HEADER_GD_SET_EVENT_FLAG : No Changed CClientmanager::SetEventFlag(%s %d) ", p->szFlagName, p->lValue);
}

void CClientManager::QUERY_RELOAD_EVENT()
{
	if (!InitializeEventTable())
	{
		sys_err("QUERY_RELOAD_EVENT: cannot load tables");
		return;
	}

	for (TPeerList::iterator i = m_peerList.begin(); i != m_peerList.end(); ++i)
	{
		CPeer* tmp = *i;

		if (!tmp->GetChannel())
			continue;

		tmp->EncodeHeader(HEADER_DG_RELOAD_EVENT, 0, sizeof(uint16_t) + sizeof(TEventTable) * m_vec_eventTable.size());
		tmp->EncodeWORD(static_cast<uint16_t>(m_vec_eventTable.size()));
		tmp->Encode(&m_vec_eventTable[0], sizeof(TEventTable) * m_vec_eventTable.size());
	}
}
#endif
