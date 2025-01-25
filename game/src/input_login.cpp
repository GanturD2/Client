#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "buffer_manager.h"
#include "protocol.h"
#include "pvp.h"
#include "start_position.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "dungeon.h"
#include "war_map.h"
#include "questmanager.h"
#include "building.h"
#include "wedding.h"
#include "affect.h"
#include "arena.h"
#include "OXEvent.h"
#include "priv_manager.h"
#include "block_country.h"
#include "dev_log.h"
#include "log.h"
#include "horsename_manager.h"
#include "MarkManager.h"
#include "threeway_war.h"
#include "../../common/CommonDefines.h"
#include "item.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#ifdef ENABLE_12ZI
#	include "zodiac_temple.h"
#endif
#ifdef ENABLE_SWITCHBOT
#	include "switchbot.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_MONSTER_BACK
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif
#ifdef ENABLE_DAWNMIST_DUNGEON
# include "DawnMistDungeon.h"
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
extern bool IS_MOUNTABLE_ZONE(int map_index);	//@custom005
#endif

#ifdef ENABLE_WOLFMAN_CHARACTER
#ifdef USE_LYCAN_CREATE_POSITION
uint32_t g_lycan_create_position[4][2] =
{
	{ 0, 0 },
	{ 768000 + 38300, 896000 + 35500 },
	{ 819200 + 38300, 896000 + 35500 },
	{ 870400 + 38300, 896000 + 35500 },
};

inline uint32_t LYCAN_CREATE_START_X(uint8_t e, uint8_t job)
{
	if (1 <= e && e <= 3)
		return (job == JOB_WOLFMAN) ? g_lycan_create_position[e][0] : g_create_position[e][0];
	return 0;
}

inline uint32_t LYCAN_CREATE_START_Y(uint8_t e, uint8_t job)
{
	if (1 <= e && e <= 3)
		return (job == JOB_WOLFMAN) ? g_lycan_create_position[e][1] : g_create_position[e][1];
	return 0;
}
#endif
#endif

static void _send_bonus_info(LPCHARACTER ch)
{
	if (!ch)
		return;

	int item_drop_bonus = 0;
	int gold_drop_bonus = 0;
	int gold10_drop_bonus = 0;
	int exp_bonus = 0;

	item_drop_bonus = CPrivManager::Instance().GetPriv(ch, PRIV_ITEM_DROP);
	gold_drop_bonus = CPrivManager::Instance().GetPriv(ch, PRIV_GOLD_DROP);
	gold10_drop_bonus = CPrivManager::Instance().GetPriv(ch, PRIV_GOLD10_DROP);
	exp_bonus = CPrivManager::Instance().GetPriv(ch, PRIV_EXP_PCT);

	if (item_drop_bonus)
		ch->ChatPacket(CHAT_TYPE_NOTICE, "[LS;871;%d]", item_drop_bonus);
	if (gold_drop_bonus)
		ch->ChatPacket(CHAT_TYPE_NOTICE, "[LS;872;%d]", gold_drop_bonus);
	if (gold10_drop_bonus)
		ch->ChatPacket(CHAT_TYPE_NOTICE, "[LS;873;%d]", gold10_drop_bonus);
	if (exp_bonus)
		ch->ChatPacket(CHAT_TYPE_NOTICE, "[LS;874;%d]", exp_bonus);
}

static bool FN_is_battle_zone(LPCHARACTER ch)
{
	if (!ch)
		return false;

	switch (ch->GetMapIndex())
	{
		case MAP_A1:
		case MAP_A3:	//@fixme415
		case MAP_B1:
		case MAP_B3:
		case MAP_C1:
		case MAP_C3:
		case MAP_OXEVENT:
			return false;

		default:
			break;
	}

	return true;
}

#ifdef ENABLE_MAP_NAME_INFO
const std::map<int, const char*> _SpMapNames
{
	{MAP_A1,						"Rotes Reich Map 1"},
	{MAP_E1_A,						"Wolfman Rotes Portal"},
	{MAP_A3,						"Rotes Reich Map 2"},
	{MAP_GUILD_01,					"Gildenland Rotes Reich"},
	{MAP_MONKEY_DUNGEON_11,			"Affendungeon (Leicht)"},
	{GUILD_VILLAGE_01,				"Gildenland 1"},
	{MAP_B1,						"Gelbes Reich Map 1"},
	{MAP_E1_B,						"Wolfman Gelbes Portal"},
	{MAP_B3,						"Gelbes Reich Map 2"},
	{MAP_GUILD_02,					"Gildenland Gelbes Reich"},
	{MAP_MONKEY_DUNGEON_12,			"Affendungeon (Mittel)"},
	{GUILD_VILLAGE_02,				"Gildenland 2"},
	{MAP_C1,						"Blaues Reich Map 1"},
	{MAP_E1_C,						"Wolfman Blaues Portal"},
	{MAP_C3,						"Blaues Reich Map 2"},
	{MAP_GUILD_03,					"Gildenland Blaues Reich"},
	{MAP_MONKEY_DUNGEON_13,			"Affendungeon (Schwer)"},
	{GUILD_VILLAGE_03,				"Gildenland 3"},
	{MAP_N_SNOWM_01,				"Eisland (Berg Sohan)"},
	{MAP_N_FLAME_01,				"Feuerland"},
	{MAP_N_DESERT_01,				"Yongbi Wüste"},
	{MAP_N_THREEWAY,				"Orktal"},
	{MAP_MILGYO,					"Hwang Tempel"},
	{MAP_DEVILTOWER1,				"Dämonenturm"},
	{MAP_TRENT,						"Geisterwald"},
	{MAP_TRENT02,					"Roter Wald"},
	{MAP_WL_01,						"WL_01"},			//
	{MAP_NUSLUCK01,					"NUSLUCK01"},		//
	{MAP_SPIDERDUNGEON_02,			"Spinnendungeon 2"},
	{MAP_SKIPIA_DUNGEON_01,			"Grotte I"},
	{MAP_SKIPIA_DUNGEON_02,			"Grotte II"},
	{MAP_WEDDING_01,				"Hochzeitsmap"},
	{MAP_T1,						"?5"},
	{MAP_SPIDERDUNGEON,				"Spinnendungeon 1"},
	{MAP_T2,						"?6"},
	{MAP_MONKEY_DUNGEON,			"Affendoungen anfänger"},
	{MAP_MONKEY_DUNGEON2,			"Affendoungen normal"},
	{MAP_MONKEY_DUNGEON3,			"Affendoungen schwer"},
	{MAP_T3,						"?7"},
	{MAP_T4,						"?8"},
	{MAP_DUEL,						"Duellier Map 1gg1"},
	{MAP_OXEVENT,					"Ox-Event Map"},
	{MAP_SUNGZI,					"Reichsschlacht Map (Normal)"},
	{MAP_SUNGZI_FLAME_HILL_01,		"Reichsschlacht Map (Feuer_01)"},
	{MAP_SUNGZI_FLAME_HILL_02,		"Reichsschlacht Map (Feuer_02)"},
	{MAP_SUNGZI_FLAME_HILL_03,		"Reichsschlacht Map (Feuer_03)"},
	{MAP_SUNGZI_SNOW,				"Reichsschlacht Map (Schnee)"},
	{MAP_SUNGZI_SNOW_PASS01,		"Reichsschlacht Map (Schnee_01)"},
	{MAP_SUNGZI_SNOW_PASS02,		"Reichsschlacht Map (Schnee_02)"},
	{MAP_SUNGZI_SNOW_PASS03,		"Reichsschlacht Map (Schnee_03)"},
	{MAP_SUNGZI_DESERT_01,			"Reichsschlacht Map (Wüste)"},
	{MAP_SUNGZI_DESERT_HILL_01,		"Reichsschlacht Map (Wüste Hill_01)"},
	{MAP_SUNGZI_DESERT_HILL_02,		"Reichsschlacht Map (Wüste Hill_02)"},
	{MAP_SUNGZI_DESERT_HILL_03,		"Reichsschlacht Map (Wüste Hill_03)"},
	{MAP_BATTLEFIELD,				"Combat Zone"},
	{MAP_EMPIREWAR01,				"Reichsschlacht 1"},
	{MAP_EMPIREWAR02,				"Reichsschlacht 2"},
	{MAP_EMPIREWAR03,				"Reichsschlacht 3"},
	{MAP_GM_GUILD,					"GM-Map"},
	{MAP_SKIPIA_DUNGEON_BOSS,		"Grotte (Boss)"},
	{MAP_DEVILCATACOMB,				"Teufelskatakomben" },
	{MAP_SPIDERDUNGEON_03,			"Spinnendungeon 3" },
	{MAP_CAPEDRAGONHEAD,			"Kap des Drachenfeuer" },
	{MAP_DAWNMISTWOOD,				"Nephritbucht" },
	{MAP_BAYBLACKSAND,				"Gautamakliff" },
	{MAP_MT_THUNDER,				"Donnerberge" },
	{MAP_N_FLAME_DUNGEON_01,		"Blazing Pugatory"},
	{MAP_N_SNOW_DUNGEON_01,			"Nemeres" },
	{MAP_DEFENSEWAVE,				"Hydra" },
	{MAP_DEFENSEWAVE_PORT,			"Hydra Port" },
	{MAP_12ZI_STAGE,				"Zodiak" },
	{MAP_SNAKEVALLEY,				"Schlange1" },
	{MAP_SNAKE_TEMPLE_01,			"Schlangentempel 1" },
	{MAP_SNAKE_TEMPLE_02,			"Schlangentempel 2" },
	{MAP_ANGLAR_DUNGEON,			"Anglar" }
};

static void __SendMapNameInfo(LPCHARACTER ch)
{
	if (!ch /*|| !ch->IsGM()*/)
		return;

	if (_SpMapNames.find(ch->GetMapIndex()) != _SpMapNames.end())
		//ch->ChatPacket(CHAT_TYPE_INFO, "Map Name: %s", _SpMapNames.find(ch->GetMapIndex())->second);
		ch->ChatPacket(CHAT_TYPE_INFO, "Channel: %d, Map Name: %s", g_bChannel, _SpMapNames.find(ch->GetMapIndex())->second);
	else {
		const auto pRegionInfo = SECTREE_MANAGER::Instance().GetMapRegion(ch->GetMapIndex());
		if (pRegionInfo)
			//ch->ChatPacket(CHAT_TYPE_INFO, "Map Name: %s", pRegionInfo->strMapName.c_str());
			ch->ChatPacket(CHAT_TYPE_INFO, "Channel: %d, Map Name: %s", g_bChannel, pRegionInfo->strMapName.c_str());
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Unknown Map Name: %d", ch->GetMapIndex());
	}
}
#endif

void CInputLogin::Login(LPDESC d, const char* data)
{
	if (!d)
		return;

	const TPacketCGLogin* pinfo = (TPacketCGLogin*)data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	sys_log(0, "InputLogin::Login : %s", login);

	TPacketGCLoginFailure failurePacket{};

	if (!test_server)
	{
		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "VERSION", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_bNoMoreClient)
	{
		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int iTotal;
		int* paiEmpireUserCount;
		int iLocal;

		DESC_MANAGER::Instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));
			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	TLoginPacket login_packet{};

	strlcpy(login_packet.login, login, sizeof(login_packet.login));
	strlcpy(login_packet.passwd, pinfo->passwd, sizeof(login_packet.passwd));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN, d->GetHandle(), &login_packet, sizeof(TLoginPacket));
}

void CInputLogin::LoginByKey(LPDESC d, const char* data)
{
	if (!d)
		return;

	const TPacketCGLogin2* pinfo = (TPacketCGLogin2*)data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	// is blocked ip?
	{
		dev_log(LOG_DEB0, "check_blocked_country_start");

		if (!is_block_exception(login) && is_blocked_country_ip(d->GetHostName()))
		{
			sys_log(0, "BLOCK_COUNTRY_IP (%s)", d->GetHostName());
			d->SetPhase(PHASE_CLOSE);
			return;
		}

		dev_log(LOG_DEB0, "check_blocked_country_end");
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket{};

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int iTotal;
		int* paiEmpireUserCount;
		int iLocal;

		DESC_MANAGER::Instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			TPacketGCLoginFailure failurePacket{};

			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));

			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	sys_log(0, "LOGIN_BY_KEY: %s key %u", login, pinfo->dwLoginKey);

	d->SetLoginKey(pinfo->dwLoginKey);
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	d->SetSecurityKey(pinfo->adwClientKey);
#endif

	TPacketGDLoginByKey ptod{};

	strlcpy(ptod.szLogin, login, sizeof(ptod.szLogin));
	ptod.dwLoginKey = pinfo->dwLoginKey;
	thecore_memcpy(ptod.adwClientKey, pinfo->adwClientKey, sizeof(uint32_t) * 4);
	strlcpy(ptod.szIP, d->GetHostName(), sizeof(ptod.szIP));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN_BY_KEY, d->GetHandle(), &ptod, sizeof(TPacketGDLoginByKey));
}

void CInputLogin::ChangeName(LPDESC d, const char* data)
{
	if (!d)
		return;

	const TPacketCGChangeName* p = (TPacketCGChangeName*)data;
	const TAccountTable& c_r = d->GetAccountTable();

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (!c_r.players[p->index].bChangeName)
		return;

	if (!check_name(p->name))
	{
		TPacketGCCreateFailure pack{};
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 0;
		d->Packet(&pack, sizeof(pack));
		return;
	}

	TPacketGDChangeName pdb{};

	pdb.pid = c_r.players[p->index].dwID;
	strlcpy(pdb.name, p->name, sizeof(pdb.name));
	db_clientdesc->DBPacket(HEADER_GD_CHANGE_NAME, d->GetHandle(), &pdb, sizeof(TPacketGDChangeName));
}

void CInputLogin::CharacterSelect(LPDESC d, const char* data)
{
	if (!d)
		return;

	const struct command_player_select* pinfo = (struct command_player_select*)data;
	const TAccountTable& c_r = d->GetAccountTable();

	sys_log(0, "player_select: login: %s index: %d", c_r.login, pinfo->index);

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("index overflow %d, login: %s", pinfo->index, c_r.login);
		return;
	}

	if (c_r.players[pinfo->index].bChangeName)
	{
		sys_err("name must be changed idx %d, login %s, name %s",
			pinfo->index, c_r.login, c_r.players[pinfo->index].szName);
		return;
	}

	TPlayerLoadPacket player_load_packet{};

	player_load_packet.account_id = c_r.id;
	player_load_packet.player_id = c_r.players[pinfo->index].dwID;
	player_load_packet.account_index = pinfo->index;

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, d->GetHandle(), &player_load_packet, sizeof(TPlayerLoadPacket));
}

#ifdef ENABLE_GIVE_BASIC_ITEM
static uint32_t GET_BASIC_ARMOR(uint8_t race)
{
	if (race == MAIN_RACE_WARRIOR_M || race == MAIN_RACE_WARRIOR_W)
		return 11209;
	else if (race == MAIN_RACE_ASSASSIN_W || race == MAIN_RACE_ASSASSIN_M)
		return 11409;
	else if (race == MAIN_RACE_SURA_M || race == MAIN_RACE_SURA_W)
		return 11609;
	else if (race == MAIN_RACE_SHAMAN_W || race == MAIN_RACE_SHAMAN_M)
		return 11809;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (race == MAIN_RACE_WOLFMAN_M)
		return 21009;
#endif
}
#endif

bool NewPlayerTable(TPlayerTable* table,
	const char* name,
	uint8_t job,
	uint8_t shape,
	uint8_t bEmpire,
	uint8_t bCon,
	uint8_t bInt,
	uint8_t bStr,
	uint8_t bDex)
{
	if (job >= JOB_MAX_NUM)
		return false;

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level = START_LEVEL;
	table->job = job;
	table->voice = 0;
	table->part_base = shape;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	table->parts[PART_ACCE] = 0;
#endif
#ifdef ENABLE_AURA_SYSTEM
	table->parts[PART_AURA] = 0;
#endif

	table->st = JobInitialPoints[job].st;
	table->dx = JobInitialPoints[job].dx;
	table->ht = JobInitialPoints[job].ht;
	table->iq = JobInitialPoints[job].iq;

	table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina = JobInitialPoints[job].max_stamina;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_LYCAN_CREATE_POSITION)
	table->x = LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y = LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x = CREATE_START_X(bEmpire) + number(-300, 300);
	table->y = CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z = 0;
	table->dir = 0;
	table->playtime = 0;
	table->gold = START_GOLD;
#ifdef ENABLE_CHEQUE_SYSTEM
	table->cheque = START_WON;
#endif
#ifdef ENABLE_BATTLE_FIELD
	table->battle_point = 0;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	table->medal_honor = 0;
#endif
#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
	memset(table->aiShopExUsablePoint, 0, sizeof(table->aiShopExUsablePoint));
	memset(table->aiShopExDailyUse, 0, sizeof(table->aiShopExDailyUse));
#endif
#ifdef ENABLE_GEM_SYSTEM
	table->gem_point = START_GEM;
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	table->conqueror_level = 0;
	table->conqueror_st = 0;
	table->conqueror_mov = 0;
	table->conqueror_ht = 0;
	table->conqueror_imu = 0;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	table->lotto_moneypool = 0;
	table->lotto_totalmoneywin = 0;
#endif

	table->skill_group = 0;

	return true;
}

bool RaceToJob(unsigned race, unsigned* ret_job)
{
	*ret_job = 0;

	if (race >= MAIN_RACE_MAX_NUM)
		return false;

	switch (race)
	{
		case MAIN_RACE_WARRIOR_M:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_WARRIOR_W:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_ASSASSIN_M:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_ASSASSIN_W:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_SURA_M:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SURA_W:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SHAMAN_M:
			*ret_job = JOB_SHAMAN;
			break;

		case MAIN_RACE_SHAMAN_W:
			*ret_job = JOB_SHAMAN;
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case MAIN_RACE_WOLFMAN_M:
			*ret_job = JOB_WOLFMAN;
			break;
#endif

		default:
			return false;
			break;
	}
	return true;
}

// new character support
bool NewPlayerTable2(TPlayerTable* table, const char* name, uint8_t race, uint8_t shape, uint8_t bEmpire)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("NewPlayerTable2.OUT_OF_RACE_RANGE(%d >= max(%d))\n", race, MAIN_RACE_MAX_NUM);
		return false;
	}

	unsigned job;

	if (!RaceToJob(race, &job))
	{
		sys_err("NewPlayerTable2.RACE_TO_JOB_ERROR(%d)\n", race);
		return false;
	}

	sys_log(0, "NewPlayerTable2(name=%s, race=%d, job=%d)", name, race, job);

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level = START_LEVEL;
	table->job = race; // Add race instead of profession
	table->voice = 0;
	table->part_base = shape;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	table->parts[PART_ACCE] = 0;
#endif
#ifdef ENABLE_AURA_SYSTEM
	table->parts[PART_AURA] = 0;
#endif

	table->st = JobInitialPoints[job].st;
	table->dx = JobInitialPoints[job].dx;
	table->ht = JobInitialPoints[job].ht;
	table->iq = JobInitialPoints[job].iq;

	table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina = JobInitialPoints[job].max_stamina;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_LYCAN_CREATE_POSITION)
	table->x = LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y = LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x = CREATE_START_X(bEmpire) + number(-300, 300);
	table->y = CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z = 0;
	table->dir = 0;
	table->playtime = 0;
	table->gold = START_GOLD;
#ifdef ENABLE_CHEQUE_SYSTEM
	table->cheque = START_WON;
#endif
#ifdef ENABLE_BATTLE_FIELD
	table->battle_point = 0;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	table->medal_honor = 0;
#endif
#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
	memset(table->aiShopExUsablePoint, 0, sizeof(table->aiShopExUsablePoint));
	memset(table->aiShopExDailyUse, 0, sizeof(table->aiShopExDailyUse));
#endif
#ifdef ENABLE_GEM_SYSTEM
	table->gem_point = START_GEM;
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	table->conqueror_level = 0;
	table->conqueror_st = 0;
	table->conqueror_mov = 0;
	table->conqueror_ht = 0;
	table->conqueror_imu = 0;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	table->lotto_moneypool = 0;
	table->lotto_totalmoneywin = 0;
#endif

	table->skill_group = 0;

#if defined(ENABLE_GIVE_BASIC_ITEM) && defined(GIVE_BASIC_SKILLS)
	//table->parts[PART_MAIN] = GET_BASIC_ARMOR(race);
	table->gold = 1000000;
	table->horse.bLevel = 11;
	table->horse.bRiding = 1;
	table->horse.sStamina = 30;
	table->horse.sHealth = 18;
	table->horse.dwHorseHealthDropTime = get_global_time();
	table->skills[121].bMasterType = 1;	// Führung
	table->skills[121].bLevel = 20;
	table->skills[126].bMasterType = 1;	// Shinsoo
	table->skills[126].bLevel = 20;
	table->skills[127].bMasterType = 1;	// Chunjo
	table->skills[127].bLevel = 20;
	table->skills[128].bMasterType = 1;	// Jinno
	table->skills[128].bLevel = 20;
	table->skills[124].bMasterType = 1;	// Bergbau
	table->skills[124].bLevel = 20;
	table->skills[129].bMasterType = 1;	// Verwandlung
	table->skills[129].bLevel = 20;
	table->skills[131].bMasterType = 1;	// Pferd rufen
	table->skills[131].bLevel = 20;
	/*if (job == 1) {
		table->skills[140].bMasterType = 1;	// Pfeilhagel
		table->skills[140].bLevel = 20;
	}*/
#endif

	return true;
}

void CInputLogin::CharacterCreate(LPDESC d, const char* data)
{
	if (!d)
		return;

	struct command_player_create* pinfo = (struct command_player_create*)data;
	TPlayerCreatePacket player_create_packet{};

	sys_log(0, "PlayerCreate: name %s pos %d job %d shape %d",
		pinfo->name,
		pinfo->index,
		pinfo->job,
		pinfo->shape);

	TPacketGCLoginFailure packFailure;
	memset(&packFailure, 0, sizeof(packFailure));
	packFailure.header = HEADER_GC_CHARACTER_CREATE_FAILURE;

	if (true == g_BlockCharCreation)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	// If the name is not usable or if it is the wrong casual wear, the creation will fail.
	if (!check_name(pinfo->name) || pinfo->shape > 1)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	const TAccountTable& c_rAccountTable = d->GetAccountTable();

	if (0 == strcmp(c_rAccountTable.login, pinfo->name))
	{
		TPacketGCCreateFailure pack{};
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 1;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	memset(&player_create_packet, 0, sizeof(TPlayerCreatePacket));

	if (!NewPlayerTable2(&player_create_packet.player_table, pinfo->name, pinfo->job, pinfo->shape, d->GetEmpire()))
	{
		sys_err("player_prototype error: job %d face %d ", pinfo->job);
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	trim_and_lower(c_rAccountTable.login, player_create_packet.login, sizeof(player_create_packet.login));
	strlcpy(player_create_packet.passwd, c_rAccountTable.passwd, sizeof(player_create_packet.passwd));

	player_create_packet.account_id = c_rAccountTable.id;
	player_create_packet.account_index = pinfo->index;

	sys_log(0, "PlayerCreate: name %s account_id %d, TPlayerCreatePacketSize(%d), Packet->Gold %d",
		pinfo->name,
		pinfo->index,
		sizeof(TPlayerCreatePacket),
		player_create_packet.player_table.gold);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_CREATE, d->GetHandle(), &player_create_packet, sizeof(TPlayerCreatePacket));
}

void CInputLogin::CharacterDelete(LPDESC d, const char* data)
{
	if (!d)
		return;

	struct command_player_delete* pinfo = (struct command_player_delete*)data;
	const TAccountTable& c_rAccountTable = d->GetAccountTable();

	if (!c_rAccountTable.id)
	{
		sys_err("PlayerDelete: no login data");
		return;
	}

	sys_log(0, "PlayerDelete: login: %s index: %d, social_id %s", c_rAccountTable.login, pinfo->index, pinfo->private_code);

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("PlayerDelete: index overflow %d, login: %s", pinfo->index, c_rAccountTable.login);
		return;
	}

	if (!c_rAccountTable.players[pinfo->index].dwID)
	{
		sys_err("PlayerDelete: Wrong Social ID index %d, login: %s", pinfo->index, c_rAccountTable.login);
		d->Packet(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
		return;
	}

	TPlayerDeletePacket player_delete_packet{};

	trim_and_lower(c_rAccountTable.login, player_delete_packet.login, sizeof(player_delete_packet.login));
	player_delete_packet.player_id = c_rAccountTable.players[pinfo->index].dwID;
	player_delete_packet.account_index = pinfo->index;
	player_delete_packet.account_id = c_rAccountTable.id;
	strlcpy(player_delete_packet.private_code, pinfo->private_code, sizeof(player_delete_packet.private_code));

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_DELETE, d->GetHandle(), &player_delete_packet, sizeof(TPlayerDeletePacket));
}

#pragma pack(1)

#pragma pack()

struct FPartyCHCollector
{
	std::vector <LPCHARACTER> vecCHPtr; FPartyCHCollector() { }

	void operator () (LPCHARACTER ch)
	{
		vecCHPtr.emplace_back(ch);
	}
};

void CInputLogin::Entergame(LPDESC d, const char* data)
{
	if (!d)
	{
		sys_err("EnterGame: No Desc!!");
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	PIXEL_POSITION pos = ch->GetXYZ();

	if (!SECTREE_MANAGER::Instance().GetMovablePosition(ch->GetMapIndex(), pos.x, pos.y, pos))
	{
		PIXEL_POSITION pos2;
		SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos2);

		sys_err("!GetMovablePosition (name %s %dx%d map %d changed to %dx%d)",
			ch->GetName(),
			pos.x, pos.y,
			ch->GetMapIndex(),
			pos2.x, pos2.y);
		pos = pos2;
	}

	CGuildManager::Instance().LoginMember(ch);

	// Adding a character to the map
	ch->Show(ch->GetMapIndex(), pos.x, pos.y, pos.z);
	SECTREE_MANAGER::Instance().SendNPCPosition(ch);

	ch->ReviveInvisible(REVIVEINVISIBLE_TIME);

	d->SetPhase(PHASE_GAME);

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
	if (ch->GetQuestFlag("costume_option.hide_body") != 0)
		ch->SetBodyCostumeHidden(true);
	else
		ch->SetBodyCostumeHidden(false);

	if (ch->GetQuestFlag("costume_option.hide_hair") != 0)
		ch->SetHairCostumeHidden(true);
	else
		ch->SetHairCostumeHidden(false);

# ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (ch->GetQuestFlag("costume_option.hide_acce") != 0)
		ch->SetAcceCostumeHidden(true);
	else
		ch->SetAcceCostumeHidden(false);
# endif

# ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (ch->GetQuestFlag("costume_option.hide_weapon") != 0)
		ch->SetWeaponCostumeHidden(true);
	else
		ch->SetWeaponCostumeHidden(false);
# endif

# ifdef ENABLE_AURA_SYSTEM
	if (ch->GetQuestFlag("costume_option.hide_aura") != 0)
		ch->SetAuraCostumeHidden(true);
	else
		ch->SetAuraCostumeHidden(false);
# endif
#endif

#ifdef __UNIMPLEMENTED__
	if (ch->affectInfo) {	//Load affects
		ch->LoadAffect(ch->affectInfo->count, (TPacketGCAffectElement*)ch->affectInfo->data);
		M2_DELETE_ARRAY(ch->affectInfo->data);
		M2_DELETE(ch->affectInfo);

		ch->affectInfo = nullptr;
	}
#endif

	//GM & invisible, let's set observer!
	if (ch->IsGM() && ch->IsAffectFlag(AFF_INVISIBILITY) && !test_server)	//@fixme421
		ch->SetObserverMode(true);

	if (ch->GetItemAward_cmd()) //When the game phase enters
		quest::CQuestManager::Instance().ItemInformer(ch->GetPlayerID(), ch->GetItemAward_vnum()); //call questmanager

	sys_log(0, "ENTERGAME: %s %dx%dx%d %s map_index %d",
		ch->GetName(), ch->GetX(), ch->GetY(), ch->GetZ(), d->GetHostName(), ch->GetMapIndex());

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
	const auto& MountCostume = ch->GetWear(WEAR_COSTUME_MOUNT);
	if (!MountCostume && ch->GetHorseLevel() > 0)
		ch->EnterHorse();
	else if (MountCostume) // Need Review in Future
		ch->MountVnum(ch->GetPoint(POINT_MOUNT));
#else
	if (ch->GetHorseLevel() > 0)
	{
		ch->EnterHorse();
	}
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	ch->SendExtendInvenInfo();
#endif

#ifdef ENABLE_EXPRESSING_EMOTION
	ch->LoadingInfoEmotions();
	ch->StartCheckTimeEmotion();
#endif

	// Start of playtime recording
	ch->ResetPlayTime();

	// Add auto save event
	ch->StartSaveEvent();
	ch->StartRecoveryEvent();
#ifdef ENABLE_MULTI_FARM_BLOCK
	CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(d->GetHostName(), ch->GetPlayerID(), ch->GetName(), true, false);
	//CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(d->GetAccountTable().hwid, ch->GetPlayerID(), ch->GetName(), true, false);
#endif
	ch->StartCheckSpeedHackEvent();
#ifdef ENABLE_AFK_MODE_SYSTEM
	ch->StartUpdateCharacterEvent();
#endif
#ifdef ENABLE_ELEMENTAL_WORLD
	if (ch->GetMapIndex() == MAP_ELEMENTAL_04)
	{
		ch->StartElementalWorldEvent();
	}
	else
	{
		if (ch->m_pkElementalWorldEvent)
		{
			ch->StopElementalWorldEvent();
			ch->hp_reduce_count = 0;
		}
	}
#endif

	CPVPManager::Instance().Connect(ch);
	CPVPManager::Instance().SendList(d);

#ifdef ENABLE_METINSTONE_RAIN_EVENT
	CMiniGameManager::Instance().StoneInformation(ch);
#endif

	MessengerManager::Instance().Login(ch->GetName());

	CPartyManager::Instance().SetParty(ch);
	CGuildManager::Instance().SendGuildWar(ch);

	building::CManager::Instance().SendLandList(d, ch->GetMapIndex());

	marriage::CManager::Instance().Login(ch);

#ifdef ENABLE_EVENT_MANAGER
	if (ch->GetMapIndex() == CEventManager::KINGDOM_WAR_MAP_INDEX && CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_KINGDOM_WAR))
		CEventManager::Instance().SendKingdomWarScorePacket(ch);
#endif

	TPacketGCTime p{};
	p.bHeader = HEADER_GC_TIME;
	p.time = get_global_time();
	d->Packet(&p, sizeof(p));

	TPacketGCChannel p2{};
	p2.header = HEADER_GC_CHANNEL;
	p2.channel = g_bChannel;
	d->Packet(&p2, sizeof(p2));

#ifdef ENABLE_MOVE_CHANNEL
	ch->ChatPacket(CHAT_TYPE_COMMAND, "server_info %d %ld", g_bChannel, ch->GetMapIndex());
#endif

#ifdef ENABLE_PARTY_MATCH
	if (quest::CQuestManager::Instance().GetEventFlag("party_match") != 0)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "party_match_off %d", 1);
	else
		ch->ChatPacket(CHAT_TYPE_COMMAND, "party_match_off %d", 0);
#endif

	ch->SendGreetMessage();

	_send_bonus_info(ch);

#ifdef ENABLE_MAILBOX
	CMailBox::UnreadData(ch);
#endif
#ifdef ENABLE_BATTLE_FIELD
	CBattleField::Instance().Connect(ch);
#endif
#ifdef ENABLE_MAP_NAME_INFO
	if (ch->IsGM())
		__SendMapNameInfo(ch);
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	CSoulRoulette::StateError(CSoulRoulette::Error::GIVE, ch);
#endif
#ifdef ENABLE_SET_ITEM
	ch->RefreshSetBonus();
#endif

	for (int i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		const int remain = ch->GetPremiumRemainSeconds(i);
		if (remain <= 0)
			continue;

		ch->AddAffect(AFFECT_PREMIUM_START + i, POINT_NONE, 0, 0, remain, 0, true);
		sys_log(0, "PREMIUM: %s type %d %dmin", ch->GetName(), i, remain);
	}

#ifdef ENABLE_YOHARA_SYSTEM
	if (ch->IsSungmaMap())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_SungMaAttr %d %d %d %d",
			ch->GetSungmaMapAttribute(POINT_SUNGMA_STR),
			ch->GetSungmaMapAttribute(POINT_SUNGMA_HP),
			ch->GetSungmaMapAttribute(POINT_SUNGMA_MOVE),
			ch->GetSungmaMapAttribute(POINT_SUNGMA_IMMUNE)
		);
		ch->AddAffect(AFFECT_SUNGMA_MAP_INFO, POINT_NONE, 0, 0, INFINITE_AFFECT_DURATION, 0, true);

		const int remainSungma = ch->GetPremiumRemainSeconds(PREMIUM_SUNGMA);
		for (int i = POINT_SUNGMA_STR; i <= POINT_SUNGMA_IMMUNE; ++i)
		{
			if (remainSungma <= 0)
				continue;

			ch->PointChange(i, 10);
		}

		if (remainSungma > 0)
			ch->ComputePoints();
	}
#endif

#ifdef ENABLE_AUTO_SYSTEM
	if (ch->GetPremiumRemainSeconds(PREMIUM_AUTO_USE) > 0)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "auto_on");
	else
		ch->ChatPacket(CHAT_TYPE_COMMAND, "auto_off");
#endif

	if (g_bCheckClientVersion)
	{
		sys_log(0, "VERSION CHECK %s %s", g_stClientVersion.c_str(), d->GetClientVersion());

		if (!d->GetClientVersion())
		{
			d->DelayedDisconnect(10);
		}
		else
		{
			if (0 != g_stClientVersion.compare(d->GetClientVersion())) // @fixme103 (version > date)
			{
				ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("[875]You do not have the correct client version. Please install the normal patch."));
				d->DelayedDisconnect(0); // @fixme103 (10);
				LogManager::Instance().HackLog("VERSION_CONFLICT", ch);

				sys_log(0, "VERSION : WRONG VERSION USER : account:%s name:%s hostName:%s server_version:%s client_version:%s",
					d->GetAccountTable().login,
					ch->GetName(),
					d->GetHostName(),
					g_stClientVersion.c_str(),
					d->GetClientVersion());
			}
		}
	}
	else
	{
		sys_log(0, "VERSION : NO CHECK");
	}

	if (ch->IsGM())
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");

	if (ch->GetMapIndex() >= 10000)
	{
		if (CWarMapManager::Instance().IsWarMap(ch->GetMapIndex()))
			ch->SetWarMap(CWarMapManager::Instance().Find(ch->GetMapIndex()));
		else if (marriage::WeddingManager::Instance().IsWeddingMap(ch->GetMapIndex()))
			ch->SetWeddingMap(marriage::WeddingManager::Instance().Find(ch->GetMapIndex()));
#ifdef ENABLE_12ZI
		else if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
			ch->SetZodiac(CZodiacManager::Instance().FindByMapIndex(ch->GetMapIndex()));
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		else if (CMeleyLairManager::Instance().IsMeleyMap(ch->GetMapIndex()))
		{
			CMeleyLairManager::Instance().EnterGame(ch);
			ch->Unmount(ch);
		}
#endif
		else
		{
			ch->SetDungeon(CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex()));

#ifdef ENABLE_SNOW_DUNGEON
			if (ch->IsSnowDungeon(ch->GetMapIndex()))
				ch->Unmount(ch);
#endif

#ifdef ENABLE_DAWNMIST_DUNGEON
			if (CDawnMistDungeon::Instance().IsInDungeonInstance(ch->GetMapIndex()))
				ch->Unmount(ch);
#endif

#ifdef ENABLE_DEFENSE_WAVE
			if (ch->IsDefenseWaveDungeon(ch->GetMapIndex()))
				ch->Unmount(ch);
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
			if (ch->IsSungMahiDungeon(ch->GetMapIndex()))
				ch->Unmount(ch);
#endif
		}
	}
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	else if (ch->GetMapIndex() == MapTypes::MAP_N_FLAME_DRAGON)
	{
		CMeleyLairManager::Instance().ExitCharacter(ch);
	}
#endif
	else if (CArenaManager::Instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		const int memberFlag = CArenaManager::Instance().IsMember(ch->GetMapIndex(), ch->GetPlayerID());
		if (memberFlag == MEMBER_OBSERVER)
		{
			ch->SetObserverMode(true);
			ch->SetArenaObserverMode(true);
			if (CArenaManager::Instance().RegisterObserverPtr(ch, ch->GetMapIndex(), static_cast<uint16_t>(ch->GetX() / 100), static_cast<uint16_t>(ch->GetY() / 100)))
				sys_log(0, "ARENA : Observer add failed");

			ch->Unmount(ch);
		}
		else if (memberFlag == MEMBER_DUELIST)
		{
			TPacketGCDuelStart duelStart{};
			duelStart.header = HEADER_GC_DUEL_START;
			duelStart.wSize = sizeof(TPacketGCDuelStart);

			ch->GetDesc()->Packet(&duelStart, sizeof(TPacketGCDuelStart));
			ch->Unmount(ch);

			LPPARTY pParty = ch->GetParty();
			if (pParty != nullptr)
			{
				if (pParty->GetMemberCount() == 2)
					CPartyManager::Instance().DeleteParty(pParty);
				else
					pParty->Quit(ch->GetPlayerID());
			}
		}
		else if (memberFlag == MEMBER_NO)
		{
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->GoHome();
		}
		else
		{
			// wtf
		}
	}
#ifdef ENABLE_MOUNT_CHECK
	else if (ch->GetMapIndex() == MAP_PVP_ARENA) // Budokan
	{
		ch->Unmount(ch);
	}
#endif
	else if (COXEventManager::IsEventMap(ch->GetMapIndex()))	//@custom033
	{
		// ox event map
		if (COXEventManager::Instance().Enter(ch) == false)
		{
			// No permission to enter the ox map. If you're a player, let's send it to the village
			if (!ch->IsGM())
				ch->GoHome();
		}

#ifdef ENABLE_MULTI_FARM_BLOCK
		if (!ch->GetMultiStatus())
			ch->GoHome();
#endif

		ch->Unmount(ch);
	}
#ifdef ENABLE_BATTLE_FIELD
	else if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->Unmount(ch);
	}
#endif
	else
	{
		if (CWarMapManager::Instance().IsWarMap(ch->GetMapIndex()) ||
			marriage::WeddingManager::Instance().IsWeddingMap(ch->GetMapIndex()))
		{
			if (!test_server)
				ch->GoHome();
		}
	}

	if (ch->GetHorseLevel() > 0)
	{
		const uint32_t pid = ch->GetPlayerID();

		if (pid != 0 && CHorseNameManager::Instance().GetHorseName(pid) == nullptr)
			db_clientdesc->DBPacket(HEADER_GD_REQ_HORSE_NAME, 0, &pid, sizeof(uint32_t));

		// @fixme182 BEGIN
		ch->SetHorseLevel(ch->GetHorseLevel());
		ch->SkillLevelPacket();
		// @fixme182 END
	}

#ifdef ENABLE_12ZI
	ch->BeadTime();
	ch->MarkTime();

	if (ch && ch->GetMapIndex() == 358)
	{
		if (DISTANCE_APPROX(ch->GetX() - 333300, ch->GetY() - 1434400) > 7000)
			ch->WarpSet(333200, 1431000, 358);
	}

	if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
	{
		if (ch->GetQuestFlag("12zi_temple.IsDead") == 1)
			ch->Dead();

		LPZODIAC pZodiac = CZodiacManager::Instance().FindByMapIndex(ch->GetMapIndex());
		if (pZodiac)
		{
			uint8_t Floor = pZodiac->GetFloor();
			if (Floor != 0)
			{
				uint32_t time = 0;

				if (Floor == 0)
				{
					ch->SetQuestFlag("12zi_temple.PrismNeed", 0);
					ch->SetQuestFlag("12zi_temple.IsDead", 0);
				}

				if (Floor == 7 || Floor == 14 || Floor == 21 || Floor == 28 || Floor == 35 || Floor == 36 || Floor == 37 || Floor == 38 || Floor == 39 || Floor == 40) //Bonus floor
					time = 5 * 60;
				else //Normal floor
					time = 10 * 60;

				ch->ZodiacFloorMessage(Floor);

				uint32_t Time_1 = (get_global_time() + 1) - pZodiac->GetTime();

				if (Floor == 40)
					ch->ChatPacket(CHAT_TYPE_COMMAND, "ZodiacTime %d %d %d %d", Floor, 0, time, Time_1);
				else
					ch->ChatPacket(CHAT_TYPE_COMMAND, "ZodiacTime %d %d %d %d", Floor, 1, time, Time_1);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_COMMAND, "ZodiacTime %d %d %d %d", 0, 1, 10, 0);
			}
		}
		else
		{
			ch->WarpSet(333200, 1431000, 358);
		}
	}
#endif

	// Guidance when entering the neutral map
	if (g_noticeBattleZone)
	{
		if (FN_is_battle_zone(ch))
		{
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("[#Unk]There may be mandatory matches on this map."));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("[#Unk]If you do not agree with this provision"));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("[#Unk]Please go back to your principal and parent."));
		}
	}

#ifdef ENABLE_MONSTER_BACK
	CMiniGameManager::Instance().AttendanceEventInfo(ch);
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	CMiniGameManager::Instance().MiniGameCatchKingEventInfo(ch);
#endif
#ifdef ENABLE_FISH_EVENT
	ch->FishEventGeneralInfo();
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
	if (!IS_MOUNTABLE_ZONE(ch->GetMapIndex()))	//@custom005
	{
		//ch->UnEquipCostumeMoudeItem(false);
		ch->Unmount(ch);
	}
#endif

#ifdef ENABLE_ANTI_EXP_RING
	CAffect* pAffect = ch->FindAffect(AFFECT_EXPRING);
	if (pAffect) {
		LPITEM expitem = ch->FindItemByID(pAffect->dwFlag);
		if (expitem && expitem->GetOwner() == ch) {
			expitem->Lock(true);
			expitem->SetSocket(0, true);
		}
		else if (!expitem) {
			ch->RemoveAffect(pAffect);
		}
	}
#endif

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	FPartyCHCollector f;

	LPCHARACTER pLeader = ch;
	LPPARTY pParty = ch->GetParty();

	if (pParty) {
		/*pLeader->ChatPacket(7, "Login-Group-Check GetParty");*/
		if (pLeader && pLeader->GetParty()->GetLeaderCharacter())
		{
			pLeader->GetParty()->ForEachOnMapMember(f, pLeader->GetParty()->GetLeaderCharacter()->GetMapIndex());
			// if (pLeader->GetParty() && pLeader->GetParty()->GetLeaderCharacter() && pLeader->GetParty()->GetLeaderCharacter()->GetMapIndex())
				//pLeader->GetParty()->ForEachOnMapMember(f, pLeader->GetParty()->GetLeaderCharacter()->GetMapIndex());
			for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); it++) {
				/*pLeader->ChatPacket(7, "Login-Group-Check PartyCollector");*/
				LPCHARACTER partyMember = *it;
				if (partyMember) {
					/*pLeader->ChatPacket(7, "Login-Group-Check Partymember");*/
					LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
					if (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON)) {
						/*pLeader->ChatPacket(7, "[Login-Group-Check] ITEM_FLAG_GROUP_WEAPON WORKS!");*/
				//	if (eqpdweapon && eqpdweapon->GetVnum() == 149) {
					//	pLeader->ChatPacket(7, "Login-Group-Check Vnum 149");
						if (eqpdweapon->GetAttributeValue(5) >= 1) {
							/*pLeader->ChatPacket(7, "Login-Group-Check Attributecheck");*/
							eqpdweapon->SetForceAttribute(5, 0, 0);
							partyMember->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Login-Group-Check success, Points status: %d"), 0);
						}
						/*else {
							pLeader->ChatPacket(7, "Login-Group-Check No Attributes");
						}*/
					}
					/*else {
						pLeader->ChatPacket(7, "Login-Group-Check No Vnum 149");
					}*/
				}
				/*else {
					pLeader->ChatPacket(7, "Login-Group-Check No Party Member");
				}*/
			}
		}
	}
	/*else {
		pLeader->ChatPacket(7, "Login-Group-Check No Party");
	}*/

	if (!pParty) {
		LPITEM eqpdweapon = ch->GetWear(WEAR_WEAPON);
		if (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON)) {
			/*ch->ChatPacket(7, "[Login-No-Group-Check] ITEM_FLAG_GROUP_WEAPON WORKS!");*/
	//	if (eqpdweapon && eqpdweapon->GetVnum() == 149) {
			if (eqpdweapon->GetAttributeValue(5) >= 1) {
				eqpdweapon->SetForceAttribute(5, 0, 0);	//reset points
				/*ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Login-No-Group-Check success, Points reset to %d"), 0);*/
			}
		}
	}
#endif
#ifdef ENABLE_SWITCHBOT
	CSwitchbotManager::Instance().EnterGame(ch);
#endif
#ifdef ENABLE_PET_SYSTEM
	ch->CheckPet();
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	ch->CheckMedals();
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	LPITEM costumeAcce = ch->GetWear(WEAR_COSTUME_ACCE);
	if (costumeAcce != nullptr)
	{
		if (costumeAcce->GetSocket(1) > 18)
			ch->EffectPacket(SE_ACCE_BACK);
	}
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	if (ch->GetPoint(POINT_SUNGMA_MOVE))
		ch->PointChange(POINT_MOV_SPEED, 0);
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	if (ch->GetQuestFlag("first_time.items") <= 0) {
		ch->EquipItems(ch);
		ch->SetQuestFlag("first_time.items", 1);
	}
#endif

	ch->SkillLevelPacket();

#ifdef ENABLE_AUTO_SYSTEM
	if (ch->IsAffectFlag(AFF_AUTO_USE))
		ch->RemoveAffect(AFFECT_AUTO);
#endif

#ifdef ENABLE_HUNTING_SYSTEM
	ch->CheckHunting();
#endif
}

void CInputLogin::Empire(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const TPacketCGEmpire* p = reinterpret_cast<const TPacketCGEmpire*>(c_pData);

	if (EMPIRE_MAX_NUM <= p->bEmpire)
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	const TAccountTable& r = d->GetAccountTable();

	if (r.bEmpire != 0)
	{
		for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (0 != r.players[i].dwID)
			{
				sys_err("EmpireSelectFailed %d", r.players[i].dwID);
				return;
			}
		}
	}

#ifdef __UNIMPLEMENTED__
	bool empireEnabled = true;
	for (auto it = s_closed_empire_reg.begin(); it != s_closed_empire_reg.end(); ++it){
		if (*it == p->bEmpire) {
			empireEnabled = false;
			break;
		}
	}
	
	TPacketGCVerifyEmpire verify;
	verify.bHeader = HEADER_GC_VERIFY_EMPIRE;
	verify.empireEnabled = empireEnabled;
	d->Packet(&verify, sizeof(verify));

	if (!empireEnabled) //We already told the client, do nothing
		return;
#endif

	TEmpireSelectPacket pd{};
	pd.dwAccountID = r.id;
	pd.bEmpire = p->bEmpire;

	db_clientdesc->DBPacket(HEADER_GD_EMPIRE_SELECT, d->GetHandle(), &pd, sizeof(pd));
}

int CInputLogin::GuildSymbolUpload(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (!d)
		return -1;

	if (uiBytes < sizeof(TPacketCGGuildSymbolUpload))
		return -1;

	sys_log(0, "GuildSymbolUpload uiBytes %u", uiBytes);

	const TPacketCGGuildSymbolUpload* p = (TPacketCGGuildSymbolUpload*)c_pData;

	if (uiBytes < p->size)
		return -1;

	const int iSymbolSize = p->size - sizeof(TPacketCGGuildSymbolUpload);

	if (iSymbolSize <= 0 || iSymbolSize > 64 * 1024)
	{
		// Guild symbols larger than 64k cannot be uploaded
		// disconnect and ignore
		d->SetPhase(PHASE_CLOSE);
		return 0;
	}

	// If the guild does not own the land.
	if (!test_server)
	{
		if (!building::CManager::Instance().FindLandByGuild(p->guild_id))
		{
			d->SetPhase(PHASE_CLOSE);
			return 0;
		}
	}

	sys_log(0, "GuildSymbolUpload Do Upload %02X%02X%02X%02X %d", c_pData[7], c_pData[8], c_pData[9], c_pData[10], sizeof(*p));

	CGuildMarkManager::Instance().UploadSymbol(p->guild_id, iSymbolSize, (const uint8_t*)(c_pData + sizeof(*p)));
	CGuildMarkManager::Instance().SaveSymbol(GUILD_SYMBOL_FILENAME);
	return iSymbolSize;
}

void CInputLogin::GuildSymbolCRC(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const TPacketCGSymbolCRC& CGPacket = *((TPacketCGSymbolCRC*)c_pData);

	sys_log(0, "GuildSymbolCRC %u %u %u", CGPacket.guild_id, CGPacket.crc, CGPacket.size);

	const CGuildMarkManager::TGuildSymbol* pkGS = CGuildMarkManager::Instance().GetGuildSymbol(CGPacket.guild_id);

	if (!pkGS)
		return;

	sys_log(0, " Server %u %u", pkGS->crc, pkGS->raw.size());

	if (pkGS->raw.size() != CGPacket.size || pkGS->crc != CGPacket.crc)
	{
		TPacketGCGuildSymbolData GCPacket{};

		GCPacket.header = HEADER_GC_GUILD_SYMBOL_DATA;
		GCPacket.size = static_cast<uint16_t>(sizeof(GCPacket) + pkGS->raw.size());
		GCPacket.guild_id = CGPacket.guild_id;

		d->BufferedPacket(&GCPacket, sizeof(GCPacket));
		d->Packet(&pkGS->raw[0], pkGS->raw.size());

		sys_log(0, "SendGuildSymbolHead %02X%02X%02X%02X Size %d",
			pkGS->raw[0], pkGS->raw[1], pkGS->raw[2], pkGS->raw[3], pkGS->raw.size());
	}
}

void CInputLogin::GuildMarkUpload(LPDESC d, const char* c_pData)
{
	TPacketCGMarkUpload* p = (TPacketCGMarkUpload*)c_pData;
	CGuildManager& rkGuildMgr = CGuildManager::Instance();
	CGuild* pkGuild;

	if (!(pkGuild = rkGuildMgr.FindGuild(p->gid)))
	{
		sys_err("MARK_SERVER: GuildMarkUpload: no guild. gid %u", p->gid);
		return;
	}

	if (pkGuild->GetLevel() < guild_mark_min_level)
	{
		sys_log(0, "MARK_SERVER: GuildMarkUpload: level < %u (%u)", guild_mark_min_level, pkGuild->GetLevel());
		return;
	}

	CGuildMarkManager& rkMarkMgr = CGuildMarkManager::Instance();

	sys_log(0, "MARK_SERVER: GuildMarkUpload: gid %u", p->gid);

	bool isEmpty = true;

	for (uint32_t iPixel = 0; iPixel < SGuildMark::SIZE; ++iPixel)
	{
		if (*((uint32_t*)p->image + iPixel) != 0x00000000)
			isEmpty = false;
	}

	if (isEmpty)
		rkMarkMgr.DeleteMark(p->gid);
	else
		rkMarkMgr.SaveMark(p->gid, p->image);
}

void CInputLogin::GuildMarkIDXList(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const CGuildMarkManager& rkMarkMgr = CGuildMarkManager::Instance();

	const uint32_t bufSize = sizeof(uint16_t) * 2 * rkMarkMgr.GetMarkCount();
	char* buf = nullptr;

	if (bufSize > 0)
	{
		buf = (char*)malloc(bufSize);
		rkMarkMgr.CopyMarkIdx(buf);
	}

	TPacketGCMarkIDXList p{};
	p.header = HEADER_GC_MARK_IDXLIST;
	p.bufSize = sizeof(p) + bufSize;
	p.count = rkMarkMgr.GetMarkCount();

	if (buf)
	{
		d->BufferedPacket(&p, sizeof(p));
		d->LargePacket(buf, bufSize);
		free(buf);
	}
	else
		d->Packet(&p, sizeof(p));

	sys_log(0, "MARK_SERVER: GuildMarkIDXList %d bytes sent.", p.bufSize);
}

void CInputLogin::GuildMarkCRCList(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const TPacketCGMarkCRCList* pCG = (TPacketCGMarkCRCList*)c_pData;

	std::map<uint8_t, const SGuildMarkBlock*> mapDiffBlocks;
	CGuildMarkManager::Instance().GetDiffBlocks(pCG->imgIdx, pCG->crclist, mapDiffBlocks);

	uint32_t blockCount = 0;
	TEMP_BUFFER buf(1024 * 1024); // 1M buffer

	for (auto& it : mapDiffBlocks)
	{
		const uint8_t posBlock = it.first;
		const SGuildMarkBlock& rkBlock = *it.second;

		buf.write(&posBlock, sizeof(uint8_t));
		buf.write(&rkBlock.m_sizeCompBuf, sizeof(uint32_t));
		buf.write(rkBlock.m_abCompBuf, rkBlock.m_sizeCompBuf);

		++blockCount;
	}

	TPacketGCMarkBlock pGC{};

	pGC.header = HEADER_GC_MARK_BLOCK;
	pGC.imgIdx = pCG->imgIdx;
	pGC.bufSize = buf.size() + sizeof(TPacketGCMarkBlock);
	pGC.count = blockCount;

	sys_log(0, "MARK_SERVER: Sending blocks. (imgIdx %u diff %u size %u)", pCG->imgIdx, mapDiffBlocks.size(), pGC.bufSize);

	if (buf.size() > 0)
	{
		d->BufferedPacket(&pGC, sizeof(TPacketGCMarkBlock));
		d->LargePacket(buf.read_peek(), buf.size());
	}
	else
		d->Packet(&pGC, sizeof(TPacketGCMarkBlock));
}

int CInputLogin::Analyze(LPDESC d, uint8_t bHeader, const char* c_pData)
{
	int iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_CG_LOGIN2:
			LoginByKey(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_SELECT:
			CharacterSelect(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_CREATE:
			CharacterCreate(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_DELETE:
			CharacterDelete(d, c_pData);
			break;

		case HEADER_CG_ENTERGAME:
			Entergame(d, c_pData);
			break;

		case HEADER_CG_EMPIRE:
			Empire(d, c_pData);
			break;

		case HEADER_CG_MOVE:
			break;

			///////////////////////////////////////
			// Guild Mark
			/////////////////////////////////////
		case HEADER_CG_MARK_CRCLIST:
			GuildMarkCRCList(d, c_pData);
			break;

		case HEADER_CG_MARK_IDXLIST:
			GuildMarkIDXList(d, c_pData);
			break;

		case HEADER_CG_MARK_UPLOAD:
			GuildMarkUpload(d, c_pData);
			break;

			//////////////////////////////////////
			// Guild Symbol
			/////////////////////////////////////
		case HEADER_CG_GUILD_SYMBOL_UPLOAD:
			if ((iExtraLen = GuildSymbolUpload(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_GUILD_SYMBOL_CRC:
			GuildSymbolCRC(d, c_pData);
			break;
			/////////////////////////////////////

		case HEADER_CG_HACK:
			break;

		case HEADER_CG_CHANGE_NAME:
			ChangeName(d, c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION:
			Version(d->GetCharacter(), c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION2:
			Version(d->GetCharacter(), c_pData);
			break;

		// @fixme120
		case HEADER_CG_ITEM_USE:
		case HEADER_CG_TARGET:
		// @Nyx new login phase fixes
		// this happened because sometimes when player disconnect or login and
		// disconnect, the phase will return 0, so 0 = auth, and the packet will be send
		// to auth...
		case HEADER_CG_MARK_LOGIN: // 20170513 - @fixme473
		case HEADER_CG_CHAT: // 20170514
#ifdef ENABLE_CUBE_RENEWAL
		case HEADER_CG_CUBE_RENEWAL:
#endif
			break;

		default:
			sys_err("login phase does not handle this packet! header %d", bHeader);
			//d->SetPhase(PHASE_CLOSE);
			return (0);
	}

	return (iExtraLen);
}
