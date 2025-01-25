#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "event.h"
#include "minilzo.h"
#include "packet.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "char.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "motion.h"
#include "sectree_manager.h"
#include "shop_manager.h"
#include "regen.h"
#include "text_file_loader.h"
#include "skill.h"
#include "pvp.h"
#include "party.h"
#include "questmanager.h"
#include "profiler.h"
#include "lzo_manager.h"
#include "messenger_manager.h"
#include "db.h"
#include "log.h"
#include "p2p.h"
#include "guild_manager.h"
#include "dungeon.h"
#include "cmd.h"
#include "refine.h"
#include "banword.h"
#include "priv_manager.h"
#include "war_map.h"
#include "building.h"
#include "login_sim.h"
#include "target.h"
#include "marriage.h"
#include "wedding.h"
#include "fishing.h"
#include "item_addon.h"
#include "TrafficProfiler.h"
#include "locale_service.h"
#include "arena.h"
#include "OXEvent.h"
#include "monarch.h"
#include "polymorph.h"
#include "blend_item.h"
#include "castle.h"
#include "ani.h"
#include "BattleArena.h"
#include "over9refine.h"
#include "horsename_manager.h"
#include "MarkManager.h"
#include "spam.h"
#include "panama.h"
#include "threeway_war.h"
#include "DragonLair.h"
#include "skill_power.h"
#include "SpeedServer.h"
#include "DragonSoul.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_SERVERTIME_PORTAL_SPAWN)
#	include "zodiac_temple.h"
#endif
#ifdef ENABLE_SWITCHBOT
#	include "switchbot.h"
#endif
#ifdef ENABLE_ATTR_6TH_7TH
#	include "Attr6th7th.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_RANKING_SYSTEM
#	include "ranking_system.h"
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "DungeonInfo.h"
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
#	include "SoulRoulette.h"
#endif
#ifdef ENABLE_CUBE_RENEWAL
#	include "CubeManager.h"
#endif
#ifdef ENABLE_TICKET_SYSTEM
#	include "ticket.h"
#endif
#ifdef ENABLE_PARTY_MATCH
#	include "GroupMatchManager.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif
#ifdef ENABLE_MINI_GAME
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
#	include "battle_pass.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif
#ifdef ENABLE_WHITE_DRAGON
#	include "WhiteDragon.h"
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
#	include "BiologSystemManager.h"
#endif

//#define __FILEMONITOR__

#if defined (__FreeBSD__) && defined(__FILEMONITOR__)
	#include "FileMonitor_FreeBSD.h"
#endif

#ifdef USE_STACKTRACE
#include <execinfo.h>
#endif

extern void WriteVersion();

// TRAFFIC_PROFILER
static constexpr uint32_t TRAFFIC_PROFILE_FLUSH_CYCLE = 3600; ///< Flush cycle of TrafficProfiler. 1 hour interval
// END_OF_TRAFFIC_PROFILER

// Socket to connect with game
int num_events_called = 0;
int max_bytes_written = 0;
int current_bytes_written = 0;
int total_bytes_written = 0;
uint8_t g_bLogLevel = 0;

socket_t tcp_socket = 0;
socket_t udp_socket = 0;
socket_t p2p_socket = 0;

LPFDWATCH main_fdw = nullptr;

int io_loop(LPFDWATCH fdw);

int start(int argc, char** argv);
int idle();
void destroy();

void test();

enum EProfile
{
	PROF_EVENT,
	PROF_CHR_UPDATE,
	PROF_IO,
	PROF_HEARTBEAT,
	PROF_MAX_NUM
};

static uint32_t s_dwProfiler[PROF_MAX_NUM];

int g_shutdown_disconnect_pulse;
int g_shutdown_disconnect_force_pulse;
int g_shutdown_core_pulse;
bool g_bShutdown = false;

extern void CancelReloadSpamEvent();

void ContinueOnFatalError() noexcept
{
#ifdef USE_STACKTRACE
	void* array[200];
	std::size_t size;
	char** symbols;

	size = backtrace(array, 200);
	symbols = backtrace_symbols(array, size);

	std::ostringstream oss;
	oss << std::endl;
	for (std::size_t i = 0; i < size; ++i) {
		oss << "  Stack> " << symbols[i] << std::endl;
	}

	free(symbols);

	sys_err("FatalError on %s", oss.str().c_str());
#else
	sys_err("FatalError");
#endif
}

void ShutdownOnFatalError()
{
	if (!g_bShutdown)
	{
		sys_err("ShutdownOnFatalError!!!!!!!!!!");
		{
			char buf[256];

			strlcpy(buf, LC_TEXT("[738]A critical server error has occurred. The server will restart automatically."), sizeof(buf));
			SendNotice(buf);
			strlcpy(buf, LC_TEXT("[739]You will be disconnected automatically in 10 seconds."), sizeof(buf));
			SendNotice(buf);
			strlcpy(buf, LC_TEXT("[740]You can connect again after 5 minutes."), sizeof(buf));
			SendNotice(buf);
		}

		g_bShutdown = true;
		g_bNoMoreClient = true;

		g_shutdown_disconnect_pulse = thecore_pulse() + PASSES_PER_SEC(10);
		g_shutdown_disconnect_force_pulse = thecore_pulse() + PASSES_PER_SEC(20);
		g_shutdown_core_pulse = thecore_pulse() + PASSES_PER_SEC(30);
	}
}

namespace
{
	struct SendDisconnectFunc
	{
		void operator() (LPDESC d)
		{
			if (d && d->GetCharacter())
			{
				if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
					d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
			}
		}
	};

	struct DisconnectFunc
	{
		void operator() (LPDESC d)
		{
			if (!d)
				return;

			if (d->GetType() == DESC_TYPE_CONNECTOR)
				return;

			if (d->IsPhase(PHASE_P2P))
				return;

			d->SetPhase(PHASE_CLOSE);
		}
	};
}

extern std::map<uint32_t, CLoginSim*> g_sim; // first: AID
extern std::map<uint32_t, CLoginSim*> g_simByPID;
extern std::vector<TPlayerTable> g_vec_save;
uint32_t save_idx = 0;

void heartbeat(LPHEART ht, int pulse)
{
	uint32_t t;

	t = get_dword_time();
	num_events_called += event_process(pulse);
	s_dwProfiler[PROF_EVENT] += (get_dword_time() - t);

	t = get_dword_time();

	// Every second
	if (!(pulse % ht->passes_per_sec))
	{
		if (!g_bAuthServer)
		{
			TPlayerCountPacket pack{};
			pack.dwCount = DESC_MANAGER::Instance().GetLocalUserCount();
			db_clientdesc->DBPacket(HEADER_GD_PLAYER_COUNT, 0, &pack, sizeof(TPlayerCountPacket));
		}
		else
		{
			DESC_MANAGER::Instance().ProcessExpiredLoginKey();
		}

		{
			int count = 0;
			auto it = g_sim.begin();

			while (it != g_sim.end())
			{
				if (!it->second->IsCheck())
				{
					it->second->SendLogin();

					if (++count > 50)
					{
						sys_log(0, "FLUSH_SENT");
						break;
					}
				}

				it++;
			}

			if (save_idx < g_vec_save.size())
			{
				count = MIN(100, g_vec_save.size() - save_idx);

				for (int i = 0; i < count; ++i, ++save_idx)
					db_clientdesc->DBPacket(HEADER_GD_PLAYER_SAVE, 0, &g_vec_save[save_idx], sizeof(TPlayerTable));

				sys_log(0, "SAVE_FLUSH %d", count);
			}
		}
	}

	//
	// Assuming 25 Pulse per second (PPS)
	//

	// About every 1.16 seconds
	if (!(pulse % (passes_per_sec + 4)))
		CHARACTER_MANAGER::Instance().ProcessDelayedSave();

	//Every 4 seconds
#if defined (__FreeBSD__) && defined(__FILEMONITOR__)
	if (!(pulse % (passes_per_sec * 5)))
	{
		FileMonitorFreeBSD::Instance().Update(pulse);
	}
#endif

	// About every 5.08 seconds
	if (!(pulse % (passes_per_sec * 5 + 2)))
	{
		ITEM_MANAGER::Instance().Update();
		DESC_MANAGER::Instance().UpdateLocalUserCount();
	}

#if defined(ENABLE_BATTLE_PASS_SYSTEM)// && !defined(ENABLE_EVENT_MANAGER)
	if (!(pulse % (passes_per_sec * 1)))
	{
		CBattlePassManager::instance().CheckBattlePassTimes();
	}
#endif

	if (!g_bAuthServer && g_bChannel == CHANNEL_99)
	{
		if (!(pulse % (passes_per_sec)))
		{
#ifdef ENABLE_BATTLE_FIELD
			CBattleField::Instance().Update();
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
			CMiniGameManager::Instance().MiniGameCatchKingCheckEnd();
#endif
#ifdef ENABLE_MINI_GAME_FINDM
			CMiniGameManager::Instance().MiniGameFindMCheckEnd();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
			CMiniGameManager::instance().MiniGameYutNoriCheckEnd();
#endif
		}
	}

	s_dwProfiler[PROF_HEARTBEAT] += (get_dword_time() - t);

	DBManager::Instance().Process();
	AccountDB::Instance().Process();
	CPVPManager::Instance().Process();
#ifdef ENABLE_EVENT_MANAGER
	if (!(pulse % passes_per_sec))
		CEventManager::Instance().Process();
#endif

	if (g_bShutdown)
	{
		if (thecore_pulse() > g_shutdown_disconnect_pulse)
		{
			const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::Instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), ::SendDisconnectFunc());
			g_shutdown_disconnect_pulse = INT_MAX;
		}
		else if (thecore_pulse() > g_shutdown_disconnect_force_pulse)
		{
			const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::Instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), ::DisconnectFunc());
		}
		else if (thecore_pulse() > g_shutdown_disconnect_force_pulse + PASSES_PER_SEC(5))
		{
			thecore_shutdown();
		}
	}
}

static void CleanUpForEarlyExit()
{
	CancelReloadSpamEvent();
}

int main(int argc, char** argv)
{
	ilInit(); // DevIL Initialize

	WriteVersion();

	SECTREE_MANAGER sectree_manager;
	CHARACTER_MANAGER char_manager;
	ITEM_MANAGER item_manager;
	CShopManager shop_manager;
	CMobManager mob_manager;
	CMotionManager motion_manager;
	CPartyManager party_manager;
	CSkillManager skill_manager;
	CPVPManager pvp_manager;
	LZOManager lzo_manager;
	DBManager db_manager;
	AccountDB account_db;

	LogManager log_manager;
	MessengerManager messenger_manager;
	P2P_MANAGER p2p_manager;
	CGuildManager guild_manager;
	CGuildMarkManager mark_manager;
	CDungeonManager dungeon_manager;
	CRefineManager refine_manager;
	CBanwordManager banword_manager;
	CPrivManager priv_manager;
	CWarMapManager war_map_manager;
	building::CManager building_manager;
	CTargetManager target_manager;
	marriage::CManager marriage_manager;
	marriage::WeddingManager wedding_manager;
	CItemAddonManager item_addon_manager;
	CArenaManager arena_manager;
	COXEventManager OXEvent_manager;
	CMonarch Monarch;
	CHorseNameManager horsename_manager;

	DESC_MANAGER desc_manager;

	TrafficProfiler trafficProfiler;
	CTableBySkill SkillPowerByLevel;
	CPolymorphUtils polymorph_utils;
	CProfiler profiler;
	CBattleArena ba;
	COver9RefineManager o9r;
	SpamManager spam_mgr;
	CThreeWayWar threeway_war;
	CDragonLairManager dl_manager;

	CSpeedServerManager SSManager;
	DSManager dsManager;

#ifdef ENABLE_CUBE_RENEWAL
	CCubeManager cube_manager;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_SERVERTIME_PORTAL_SPAWN)
	CZodiacManager	zodiac_manager;
#endif
#ifdef ENABLE_SWITCHBOT
	CSwitchbotManager switchbot;
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	CMeleyLairManager MeleyLair_manager;
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	CAttr6th7th Attr67_manager;
#endif
#ifdef ENABLE_BATTLE_FIELD
	CBattleField BattleField_manager;
#endif
#ifdef ENABLE_RANKING_SYSTEM
	CRankingSystem Ranking_manager;
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	CDungeonInfoManager DungeonInfoManager;
#endif
#ifdef ENABLE_TICKET_SYSTEM
	CTicketSystem ticket;
#endif
#ifdef ENABLE_MINI_GAME
	CMiniGameManager minigame_manager;
#endif
#ifdef ENABLE_PARTY_MATCH
	CGroupMatchManager groupmatch_manager;
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	CBattlePassManager battle_pass;
#endif
#ifdef ENABLE_EVENT_MANAGER
	CEventManager event_manager;
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem achievement;
#endif
#ifdef ENABLE_QUEEN_NETHIS
	SnakeLair::CSnk SnakeLair_manager;
#endif
#ifdef ENABLE_WHITE_DRAGON
	WhiteDragon::CWhDr WhiteDragon_manager;
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	CBiologSystemManager biologSystem;
#endif

	if (!start(argc, argv))
	{
		CleanUpForEarlyExit();
		return 0;
	}

	quest::CQuestManager quest_manager;

	if (!quest_manager.Initialize())
	{
		CleanUpForEarlyExit();
		return 0;
	}

	MessengerManager::Instance().Initialize();
	CGuildManager::Instance().Initialize();
	fishing::Initialize();
	OXEvent_manager.Initialize();

	if (speed_server)
		CSpeedServerManager::Instance().Initialize();

#ifndef ENABLE_CUBE_RENEWAL
	Cube_init();
#else
	CCubeManager::Instance().Cube_init();
#endif
	Blend_Item_init();
	ani_init();
	PanamaLoad();

#if defined(ENABLE_12ZI) && defined(ENABLE_SERVERTIME_PORTAL_SPAWN)
	zodiac_manager.Initialize();
#endif
#ifdef ENABLE_RANKING_SYSTEM
	Ranking_manager.Initialize();
#endif
#ifdef ENABLE_BATTLE_FIELD
	BattleField_manager.Initialize();
#endif
#ifdef ENABLE_QUEEN_NETHIS
	SnakeLair_manager.Initialize();
#endif
#ifdef ENABLE_WHITE_DRAGON
	WhiteDragon_manager.Initialize();
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	MeleyLair_manager.Initialize();
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	DungeonInfoManager.Initialize();
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	achievement.Initialize();
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (!g_bAuthServer)
	{
		guild_manager.InitializeDonate();
	}
#endif

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	char szPortalLevelLimitFileName[256];
	snprintf(szPortalLevelLimitFileName, sizeof(szPortalLevelLimitFileName), "%s/portal.txt", LocaleService_GetBasePath().c_str());
	char_manager.InitPortalLevelLimit(szPortalLevelLimitFileName);
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	CSoulRoulette::ReadRouletteData();
	CSoulRoulette::StateError(CSoulRoulette::Error::LOAD, nullptr);
#endif

	if (g_bTrafficProfileOn)
		TrafficProfiler::Instance().Initialize(TRAFFIC_PROFILE_FLUSH_CYCLE, "ProfileLog");

	// Client PackageCrypt

	//TODO : make it config
	const std::string strPackageCryptInfoDir = "package/";
	if (!desc_manager.LoadClientPackageCryptInfo(strPackageCryptInfoDir.c_str()))
	{
		sys_err("Failed to Load ClientPackageCryptInfo File(%s)", strPackageCryptInfoDir.c_str());
	}

#if defined (__FreeBSD__) && defined(__FILEMONITOR__)
	PFN_FileChangeListener pPackageNotifyFunc = &(DESC_MANAGER::NotifyClientPackageFileChanged);
	//FileMonitorFreeBSD::Instance().AddWatch(strPackageCryptInfoName, pPackageNotifyFunc);
#endif

	while (idle());

	sys_log(0, "<shutdown> Starting...");
	g_bShutdown = true;
	g_bNoMoreClient = true;

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	CSoulRoulette::ReadRouletteData(true);
	CSoulRoulette::StateError(CSoulRoulette::Error::SHUTDOWN, nullptr);
#endif

	if (g_bAuthServer)
	{
		const int iLimit = DBManager::Instance().CountQuery() / 50;
		int i = 0;

		do
		{
			const uint32_t dwCount = DBManager::Instance().CountQuery();
			sys_log(0, "Queries %u", dwCount);

			if (dwCount == 0)
				break;

			usleep(500000);

			if (++i >= iLimit)
				if (dwCount == DBManager::Instance().CountQuery())
					break;
		} while (1);
	}

	sys_log(0, "<shutdown> Destroying CArenaManager...");
	arena_manager.Destroy();
	sys_log(0, "<shutdown> Destroying COXEventManager...");
	OXEvent_manager.Destroy();

	sys_log(0, "<shutdown> Disabling signal timer...");
	signal_timer_disable();

	sys_log(0, "<shutdown> Shutting down CHARACTER_MANAGER...");
	char_manager.GracefulShutdown();
	sys_log(0, "<shutdown> Shutting down ITEM_MANAGER...");
	item_manager.GracefulShutdown();

	sys_log(0, "<shutdown> Flushing db_clientdesc...");
	db_clientdesc->FlushOutput();
	sys_log(0, "<shutdown> Flushing p2p_manager...");
	p2p_manager.FlushOutput();

	sys_log(0, "<shutdown> Destroying CShopManager...");
	shop_manager.Destroy();
	sys_log(0, "<shutdown> Destroying CHARACTER_MANAGER...");
	char_manager.Destroy();
	sys_log(0, "<shutdown> Destroying ITEM_MANAGER...");
	item_manager.Destroy();
	sys_log(0, "<shutdown> Destroying DESC_MANAGER...");
	desc_manager.Destroy();
	sys_log(0, "<shutdown> Destroying quest::CQuestManager...");
	quest_manager.Destroy();
	sys_log(0, "<shutdown> Destroying building::CManager...");
	building_manager.Destroy();
#ifdef ENABLE_MINI_GAME
	sys_log(0, "<shutdown> Destroying minigame_manager...");
	minigame_manager.Destroy();
#endif
#ifdef ENABLE_BATTLE_FIELD
	sys_log(0, "<shutdown> Destroying BattleField_manager...");
	BattleField_manager.Destroy();
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	sys_log(0, "<shutdown> Destroying MeleyLair_manager.");
	MeleyLair_manager.Destroy();
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	sys_log(0, "<shutdown> Destroying CDungeonInfoManager...");
	DungeonInfoManager.Destroy();
#endif
#ifdef ENABLE_QUEEN_NETHIS
	sys_log(0, "<shutdown> Destroying SnakeLair_manager.");
	SnakeLair_manager.Destroy();
#endif
#ifdef ENABLE_WHITE_DRAGON
	sys_log(0, "<shutdown> Destroying WhiteDragon_manager.");
	WhiteDragon_manager.Destroy();
#endif

	sys_log(0, "<shutdown> Flushing TrafficProfiler...");
	trafficProfiler.Flush();

	destroy();

	return 1;
}

void usage()
{
	printf("Option list\n"
		"-p <port> : bind port number (port must be over 1024)\n"
		"-l <level> : sets log level\n"
		"-n <locale> : sets locale name\n"
#ifdef ENABLE_NEWSTUFF
		"-C <on-off> : checkpointing check on/off\n"
#endif
		"-v : log to stdout\n"
		"-r : do not load regen tables\n"
		"-t : traffic profile on\n");
}

int start(int argc, char** argv)
{
	std::string st_localeServiceName;

	bool bVerbose = false;
	char ch;

	//_malloc_options = "A";

#ifdef ENABLE_NEWSTUFF
	char optstring[] = "npverltIC";
#else
	char optstring[] = "npverltI";
#endif
	while ((ch = getopt(argc, argv, &optstring[0])) != -1)
	{
		char* ep = nullptr;

		switch (ch)
		{
			case 'I': // IP
			{
				strlcpy(g_szPublicIP, argv[optind], sizeof(g_szPublicIP));

				printf("IP %s\n", g_szPublicIP);

				optind++;
				optreset = 1;
			}
			break;

			case 'p': // port
			{
				mother_port = strtol(argv[optind], &ep, 10);

				if (mother_port <= 1024)
				{
					usage();
					return 0;
				}

				printf("port %d\n", mother_port);

				optind++;
				optreset = 1;
			}
			break;

			case 'l':
			{
				const long l = strtol(argv[optind], &ep, 10);

				log_set_level(l);

				optind++;
				optreset = 1;
			}
			break;

			// LOCALE_SERVICE
			case 'n':
			{
				if (optind < argc)
				{
					st_localeServiceName = argv[optind++];
					optreset = 1;
				}
			}
			break;
			// END_OF_LOCALE_SERVICE

#ifdef ENABLE_NEWSTUFF
			case 'C': // checkpoint check
			{
				bCheckpointCheck = strtol(argv[optind], &ep, 10);
				printf("CHECKPOINT_CHECK %d\n", bCheckpointCheck);

				optind++;
				optreset = 1;
			}
			break;
#endif

			case 'v': // verbose
				bVerbose = true;
				break;

			case 'r':
				g_bNoRegen = true;
				break;

				// TRAFFIC_PROFILER
			case 't':
				g_bTrafficProfileOn = true;
				break;
				// END_OF_TRAFFIC_PROFILER

			default:
				break;
		}
	}

	// LOCALE_SERVICE
	config_init(st_localeServiceName);
	// END_OF_LOCALE_SERVICE

#ifdef __WIN32__
	// In Windows dev mode, "verbose" option is [on] by default.
	bVerbose = true;
#endif
	if (!bVerbose)
		freopen("stdout", "a", stdout);

	const bool is_thecore_initialized = thecore_init(25, heartbeat);
	if (!is_thecore_initialized)
		exit(0);

	if (false == CThreeWayWar::Instance().LoadSetting("forkedmapindex.txt"))
	{
		if (!g_bAuthServer)
		{
			fprintf(stderr, "Could not Load ThreeWayWar Setting file");
			exit(0);
		}
	}

	signal_timer_disable();

	main_fdw = fdwatch_new(4096);

	if ((tcp_socket = socket_tcp_bind(g_szPublicIP, mother_port)) == INVALID_SOCKET)
	{
		perror("socket_tcp_bind: tcp_socket");
		return 0;
	}


#ifndef __UDP_BLOCK__
	if ((udp_socket = socket_udp_bind(g_szPublicIP, mother_port)) == INVALID_SOCKET)
	{
		perror("socket_udp_bind: udp_socket");
		return 0;
	}
#endif

	// if internal ip exists, p2p socket uses internal ip, if not use public ip
	//if ((p2p_socket = socket_tcp_bind(*g_szInternalIP ? g_szInternalIP : g_szPublicIP, p2p_port)) == INVALID_SOCKET)
	if ((p2p_socket = socket_tcp_bind(g_szPublicIP, p2p_port)) == INVALID_SOCKET)
	{
		perror("socket_tcp_bind: p2p_socket");
		return 0;
	}

	fdwatch_add_fd(main_fdw, tcp_socket, nullptr, FDW_READ, false);
#ifndef __UDP_BLOCK__
	fdwatch_add_fd(main_fdw, udp_socket, nullptr, FDW_READ, false);
#endif
	fdwatch_add_fd(main_fdw, p2p_socket, nullptr, FDW_READ, false);

	db_clientdesc = DESC_MANAGER::Instance().CreateConnectionDesc(main_fdw, db_addr, db_port, PHASE_DBCLIENT, true);
	if (!g_bAuthServer)
		db_clientdesc->UpdateChannelStatus(0, true);

	if (g_bAuthServer)
	{
		if (g_stAuthMasterIP.length() != 0)
		{
			fprintf(stderr, "SlaveAuth");
			g_pkAuthMasterDesc = DESC_MANAGER::Instance().CreateConnectionDesc(main_fdw, g_stAuthMasterIP.c_str(), g_wAuthMasterPort, PHASE_P2P, true);
			P2P_MANAGER::Instance().RegisterConnector(g_pkAuthMasterDesc);
			g_pkAuthMasterDesc->SetP2P(g_stAuthMasterIP.c_str(), g_wAuthMasterPort, g_bChannel);

		}
		else
		{
			fprintf(stderr, "MasterAuth %d\n", LC_GetLocalType());
		}
	}

	signal_timer_enable(30);
	return 1;
}

void destroy()
{
	sys_log(0, "<shutdown> Canceling ReloadSpamEvent...");
	CancelReloadSpamEvent();

	sys_log(0, "<shutdown> regen_free()...");
	regen_free();

	sys_log(0, "<shutdown> Closing sockets...");
	socket_close(tcp_socket);
#ifndef __UDP_BLOCK__
	socket_close(udp_socket);
#endif
	socket_close(p2p_socket);

	sys_log(0, "<shutdown> fdwatch_delete()...");
	fdwatch_delete(main_fdw);

	sys_log(0, "<shutdown> event_destroy()...");
	event_destroy();

	sys_log(0, "<shutdown> CTextFileLoader::DestroySystem()...");
	CTextFileLoader::DestroySystem();

	sys_log(0, "<shutdown> thecore_destroy()...");
	thecore_destroy();
}

int idle()
{
	static struct timeval pta = { 0, 0 };
	static int process_time_count = 0;
	struct timeval now {};

	if (pta.tv_sec == 0)
		gettimeofday(&pta, (struct timezone*)0);

	int passed_pulses;

	if (!(passed_pulses = thecore_idle()))
		return 0;

	assert(passed_pulses > 0);

	uint32_t t;

	while (passed_pulses--)
	{
		heartbeat(thecore_heart, ++thecore_heart->pulse);

		// To reduce the possibility of abort() in checkpointing
		thecore_tick();
	}

	t = get_dword_time();
	CHARACTER_MANAGER::Instance().Update(thecore_heart->pulse);
	db_clientdesc->Update(t);
	s_dwProfiler[PROF_CHR_UPDATE] += (get_dword_time() - t);

	t = get_dword_time();
	if (!io_loop(main_fdw))
		return 0; // shutdown

	s_dwProfiler[PROF_IO] += (get_dword_time() - t);

	log_rotate();

	gettimeofday(&now, (struct timezone*)0);
	++process_time_count;

	if (now.tv_sec - pta.tv_sec > 0)
	{
		pt_log("[%3d] event %5d/%-5d idle %-4ld event %-4ld heartbeat %-4ld I/O %-4ld chrUpate %-4ld | WRITE: %-7d | PULSE: %d",
			process_time_count,
			num_events_called,
			event_count(),
			thecore_profiler[PF_IDLE],
			s_dwProfiler[PROF_EVENT],
			s_dwProfiler[PROF_HEARTBEAT],
			s_dwProfiler[PROF_IO],
			s_dwProfiler[PROF_CHR_UPDATE],
			current_bytes_written,
			thecore_pulse());

		num_events_called = 0;
		current_bytes_written = 0;

		process_time_count = 0;
		gettimeofday(&pta, (struct timezone*)0);

		memset(&thecore_profiler[0], 0, sizeof(thecore_profiler));
		memset(&s_dwProfiler[0], 0, sizeof(s_dwProfiler));
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

int io_loop(LPFDWATCH fdw)
{
	LPDESC d;
	int num_events = 0, event_idx = 0;

	DESC_MANAGER::Instance().DestroyClosed(); // Disconnects PHASE_CLOSE connections.
	DESC_MANAGER::Instance().TryConnect();

	if ((num_events = fdwatch(fdw, 0)) < 0)
		return 0;

	for (event_idx = 0; event_idx < num_events; ++event_idx)
	{
		d = static_cast<LPDESC>(fdwatch_get_client_data(fdw, event_idx));

		if (!d)
		{
			if (FDW_READ == fdwatch_check_event(fdw, tcp_socket, event_idx))
			{
				DESC_MANAGER::Instance().AcceptDesc(fdw, tcp_socket);
				fdwatch_clear_event(fdw, tcp_socket, event_idx);
			}
			else if (FDW_READ == fdwatch_check_event(fdw, p2p_socket, event_idx))
			{
				DESC_MANAGER::Instance().AcceptP2PDesc(fdw, p2p_socket);
				fdwatch_clear_event(fdw, p2p_socket, event_idx);
			}
			/*
			else if (FDW_READ == fdwatch_check_event(fdw, udp_socket, event_idx))
			{
				char buf[256];
				struct sockaddr_in cliaddr;
				socklen_t socklen = sizeof(cliaddr);

				int iBytesRead;

				if ((iBytesRead = socket_udp_read(udp_socket, buf, 256, (struct sockaddr *) &cliaddr, &socklen)) > 0)
				{
					static CInputUDP s_inputUDP;

					s_inputUDP.SetSockAddr(cliaddr);

					int iBytesProceed;
					s_inputUDP.Process(nullptr, buf, iBytesRead, iBytesProceed);
				}

				fdwatch_clear_event(fdw, udp_socket, event_idx);
			}
			*/
			continue;
		}

		const int iRet = fdwatch_check_event(fdw, d->GetSocket(), event_idx);

		switch (iRet)
		{
			case FDW_READ:
				if (db_clientdesc == d)
				{
					const int size = d->ProcessInput();

					if (size)
						sys_log(1, "DB_BYTES_READ: %d", size);

					if (size < 0)
						d->SetPhase(PHASE_CLOSE);
				}
				else if (d->ProcessInput() < 0)
				{
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			case FDW_WRITE:
				if (db_clientdesc == d)
				{
					const int buf_size = buffer_size(d->GetOutputBuffer());
					const int sock_buf_size = fdwatch_get_buffer_size(fdw, d->GetSocket());

					const int ret = d->ProcessOutput();

					if (ret < 0)
						d->SetPhase(PHASE_CLOSE);

					if (buf_size)
						sys_log(1, "DB_BYTES_WRITE: size %d sock_buf %d ret %d", buf_size, sock_buf_size, ret);
				}
				else if (d->ProcessOutput() < 0)
				{
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			case FDW_EOF:
				{
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			default:
				sys_err("fdwatch_check_event returned unknown %d", iRet);
				d->SetPhase(PHASE_CLOSE);
				break;
		}
	}

	return 1;
}
