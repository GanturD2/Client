#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "packet.h"
#include "protocol.h"
#include "mob_manager.h"
#include "shop_manager.h"
#include "sectree_manager.h"
#include "skill.h"
#include "questmanager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "start_position.h"
#include "party.h"
#include "refine.h"
#include "banword.h"
#include "priv_manager.h"
#include "db.h"
#include "building.h"
#include "login_sim.h"
#include "wedding.h"
#include "login_data.h"
#include "unique_item.h"

#include "monarch.h"
#include "affect.h"
#include "castle.h"
#include "block_country.h"
#include "motion.h"

#include "dev_log.h"

#include "log.h"

#include "horsename_manager.h"
#include "gm.h"
#include "panama.h"
#include "map_location.h"

#include "DragonSoul.h"

#include "shutdown_manager.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "shop.h"
#endif
#ifdef ENABLE_MONSTER_BACK
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
#	include "battle_pass.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
#	include "BiologSystemManager.h"
#endif

#define MAPNAME_DEFAULT "none"

bool GetServerLocation(TAccountTable& rTab, uint8_t bEmpire)
{
	bool bFound = false;

	for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		if (0 == rTab.players[i].dwID)
			continue;

		bFound = true;
		long lIndex = 0;

		if (!CMapLocation::Instance().Get(rTab.players[i].x,
			rTab.players[i].y,
			lIndex,
			rTab.players[i].lAddr,
			rTab.players[i].wPort))
		{
			sys_err("location error name %s mapindex %d %d x %d empire %d",
				rTab.players[i].szName, lIndex, rTab.players[i].x, rTab.players[i].y, rTab.bEmpire);

			rTab.players[i].x = EMPIRE_START_X(rTab.bEmpire);
			rTab.players[i].y = EMPIRE_START_Y(rTab.bEmpire);

			lIndex = 0;

			if (!CMapLocation::Instance().Get(rTab.players[i].x, rTab.players[i].y, lIndex, rTab.players[i].lAddr, rTab.players[i].wPort))
			{
				sys_err("cannot find server for mapindex %d %d x %d (name %s)",
					lIndex,
					rTab.players[i].x,
					rTab.players[i].y,
					rTab.players[i].szName);
#ifdef ENABLE_PROXY_IP
				if (!g_stProxyIP.empty())
					rTab.players[i].lAddr = inet_addr(g_stProxyIP.c_str());
#endif
				continue;
			}
		}
#ifdef ENABLE_PROXY_IP
		if (!g_stProxyIP.empty())
			rTab.players[i].lAddr = inet_addr(g_stProxyIP.c_str());
#endif
		struct in_addr in;
		in.s_addr = rTab.players[i].lAddr;
		sys_log(0, "success to %s:%d", inet_ntoa(in), rTab.players[i].wPort);
	}

	return bFound;
}

extern std::map<uint32_t, CLoginSim*> g_sim;
extern std::map<uint32_t, CLoginSim*> g_simByPID;

void CInputDB::LoginSuccess(uint32_t dwHandle, const char* data)
{
	sys_log(0, "LoginSuccess");

	TAccountTable* pTab = (TAccountTable*)data;
	if (!pTab)
		return;

	const auto it = g_sim.find(pTab->id);
	if (g_sim.end() != it)
	{
		sys_log(0, "CInputDB::LoginSuccess - already exist sim [%s]", pTab->login);
		it->second->SendLoad();
		return;
	}

	LPDESC d = DESC_MANAGER::Instance().FindByHandle(dwHandle);

	if (!d)
	{
		sys_log(0, "CInputDB::LoginSuccess - cannot find handle [%s]", pTab->login);

		TLogoutPacket pack{};

		strlcpy(pack.login, pTab->login, sizeof(pack.login));
		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, dwHandle, &pack, sizeof(pack));
		return;
	}

	if (strcmp(pTab->status, "OK")) // if not OK
	{
		sys_log(0, "CInputDB::LoginSuccess - status[%s] is not OK [%s]", pTab->status, pTab->login);

		TLogoutPacket pack{};

		strlcpy(pack.login, pTab->login, sizeof(pack.login));
		db_clientdesc->DBPacket(HEADER_GD_LOGOUT, dwHandle, &pack, sizeof(pack));

		LoginFailure(d, pTab->status);
		return;
	}

	for (int i = 0; i != PLAYER_PER_ACCOUNT; ++i)
	{
		TSimplePlayer& player = pTab->players[i];
		sys_log(0, "\tplayer(%s).job(%d)", player.szName, player.byJob);
	}

	const bool bFound = GetServerLocation(*pTab, pTab->bEmpire);

	d->BindAccountTable(pTab);


	if (!bFound) // If there is no character, it is sent to a random empire. -_-
	{
		TPacketGCEmpire pe{};
		pe.bHeader = HEADER_GC_EMPIRE;
		pe.bEmpire = static_cast<uint8_t>(number(1, 3));
		d->Packet(&pe, sizeof(pe));
	}
	else
	{
		TPacketGCEmpire pe{};
		pe.bHeader = HEADER_GC_EMPIRE;
		pe.bEmpire = d->GetEmpire();
		d->Packet(&pe, sizeof(pe));
	}

	d->SetPhase(PHASE_SELECT);
	d->SendLoginSuccessPacket();

	// __SHUTDOWN::Shutdown Register
	CShutdownManager::Instance().AddDesc(d);

	sys_log(0, "InputDB::login_success: %s", pTab->login);
}

void CInputDB::PlayerCreateFailure(LPDESC d, uint8_t bType)
{
	if (!d)
		return;

	TPacketGCCreateFailure pack{};

	pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
	pack.bType = bType;

	d->Packet(&pack, sizeof(pack));
}

void CInputDB::PlayerCreateSuccess(LPDESC d, const char* data)
{
	if (!d)
		return;

	TPacketDGCreateSuccess* pPacketDB = (TPacketDGCreateSuccess*)data;

	if (pPacketDB->bAccountCharacterIndex >= PLAYER_PER_ACCOUNT)
	{
		d->Packet(encode_byte(HEADER_GC_CHARACTER_CREATE_FAILURE), 1);
		return;
	}

	long lIndex = 0;

	if (!CMapLocation::Instance().Get(pPacketDB->player.x,
		pPacketDB->player.y,
		lIndex,
		pPacketDB->player.lAddr,
		pPacketDB->player.wPort))
	{
		sys_err("InputDB::PlayerCreateSuccess: cannot find server for mapindex %d %d x %d (name %s)",
			lIndex,
			pPacketDB->player.x,
			pPacketDB->player.y,
			pPacketDB->player.szName);
	}

	TAccountTable& r_Tab = d->GetAccountTable();
	r_Tab.players[pPacketDB->bAccountCharacterIndex] = pPacketDB->player;

	TPacketGCPlayerCreateSuccess pack{};

	pack.header = HEADER_GC_CHARACTER_CREATE_SUCCESS;
	pack.bAccountCharacterIndex = pPacketDB->bAccountCharacterIndex;
	pack.player = pPacketDB->player;
#ifdef ENABLE_PROXY_IP
	if (!g_stProxyIP.empty())
		pack.player.lAddr = inet_addr(g_stProxyIP.c_str());
#endif
	d->Packet(&pack, sizeof(TPacketGCPlayerCreateSuccess));

	LogManager::Instance().CharLog(pack.player.dwID, 0, 0, 0, "CREATE PLAYER", "", d->GetHostName());
}

void CInputDB::PlayerDeleteSuccess(LPDESC d, const char* data)
{
	if (!d)
		return;

	uint8_t account_index;
	account_index = decode_byte(data);
	d->BufferedPacket(encode_byte(HEADER_GC_CHARACTER_DELETE_SUCCESS), 1);
	d->Packet(encode_byte(account_index), 1);

	d->GetAccountTable().players[account_index].dwID = 0;
}

#ifdef ENABLE_DELETE_FAILURE_TYPE
void CInputDB::PlayerDeleteFail(LPDESC d, const char* c_pData)
#else
void CInputDB::PlayerDeleteFail(LPDESC d)
#endif
{
#ifdef ENABLE_DELETE_FAILURE_TYPE
	TPacketDeleteFailureInfo* pTab = (TPacketDeleteFailureInfo*)c_pData;
#endif

	if (!d)
		return;

#ifdef ENABLE_DELETE_FAILURE_TYPE
	d->BufferedPacket(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
	d->Packet(encode_byte(pTab->type), 1);
	d->Packet(encode_4bytes(pTab->time), 4);
#else
	d->Packet(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
#endif
	//d->Packet(encode_byte(account_index), 1);

	//d->GetAccountTable().players[account_index].dwID = 0;
}

void CInputDB::ChangeName(LPDESC d, const char* data)
{
	if (!d)
		return;

	const TPacketDGChangeName* p = (TPacketDGChangeName*)data;

	TAccountTable& r = d->GetAccountTable();

	if (!r.id)
		return;

	for (size_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		if (r.players[i].dwID == p->pid)
		{
			strlcpy(r.players[i].szName, p->name, sizeof(r.players[i].szName));
			r.players[i].bChangeName = 0;

			TPacketGCChangeName pgc{};

			pgc.header = HEADER_GC_CHANGE_NAME;
			pgc.pid = p->pid;
			strlcpy(pgc.name, p->name, sizeof(pgc.name));

			d->Packet(&pgc, sizeof(TPacketGCChangeName));
			break;
		}
}

void CInputDB::PlayerLoad(LPDESC d, const char* data)
{
	TPlayerTable* pTab = (TPlayerTable*)data;

	if (!d)
		return;

	long lMapIndex = pTab->lMapIndex;
	PIXEL_POSITION pos;

	if (lMapIndex == 0)
	{
		lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(pTab->x, pTab->y);

		if (lMapIndex == 0) // Coordinates not found
		{
			lMapIndex = EMPIRE_START_MAP(d->GetAccountTable().bEmpire);
			pos.x = EMPIRE_START_X(d->GetAccountTable().bEmpire);
			pos.y = EMPIRE_START_Y(d->GetAccountTable().bEmpire);
		}
		else
		{
			pos.x = pTab->x;
			pos.y = pTab->y;
		}
	}
	pTab->lMapIndex = lMapIndex;

	// If you were in a private map and the private map is gone, you have to go back to the exit.
	// ----
	// But if you say you have to go back to the exit... Why are you looking for the location of the pulic map corresponding to the private map instead of the exit...
	// Since you don't know the history... Hard-coding again.
	// If it is an anglerfish cave, go to the exit...
	// by rtsummit
	if (!SECTREE_MANAGER::Instance().GetValidLocation(pTab->lMapIndex, pTab->x, pTab->y, lMapIndex, pos, d->GetEmpire()))
	{
		sys_err("InputDB::PlayerLoad : cannot find valid location %d x %d (name: %s)", pTab->x, pTab->y, pTab->name);
#ifdef ENABLE_GOHOME_IF_MAP_NOT_EXIST
		lMapIndex = EMPIRE_START_MAP(d->GetAccountTable().bEmpire);
		pos.x = EMPIRE_START_X(d->GetAccountTable().bEmpire);
		pos.y = EMPIRE_START_Y(d->GetAccountTable().bEmpire);
#else
		d->SetPhase(PHASE_CLOSE);
		return;
#endif
	}

	pTab->x = pos.x;
	pTab->y = pos.y;
	pTab->lMapIndex = lMapIndex;

	if (d->GetCharacter() || d->IsPhase(PHASE_GAME))
	{
		LPCHARACTER p = d->GetCharacter();
		sys_err("login state already has main state (character %s %p)", p->GetName(), get_pointer(p));
		return;
	}

	if (nullptr != CHARACTER_MANAGER::Instance().FindPC(pTab->name))
	{
		sys_err("InputDB: PlayerLoad : %s already exist in game", pTab->name);
		return;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().CreateCharacter(pTab->name, pTab->id);

	ch->BindDesc(d);
	ch->SetPlayerProto(pTab);
	ch->SetEmpire(d->GetEmpire());

	d->BindCharacter(ch);

	{
		// P2P Login
		TPacketGGLogin p{};

		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, ch->GetName(), sizeof(p.szName));
		p.dwPID = ch->GetPlayerID();
		p.bEmpire = ch->GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(ch->GetX(), ch->GetY());
		p.bChannel = g_bChannel;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		p.bLanguage = ch->GetDesc()->GetLanguage();
#endif

		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGLogin));

		char buf[55];
		snprintf(buf, sizeof(buf), "%s %d %d "
			"%ld %d"
#ifdef ENABLE_CHEQUE_SYSTEM
			"%d"
#endif
				,
				inet_ntoa(ch->GetDesc()->GetAddr().sin_addr), ch->GetGold(),
				g_bChannel, ch->GetMapIndex(), ch->GetAlignment()
#ifdef ENABLE_CHEQUE_SYSTEM
				, ch->GetCheque()
#endif
		);
		LogManager::Instance().CharLog(ch, 0, "LOGIN", buf);
	}

	d->SetPhase(PHASE_LOADING);
	ch->MainCharacterPacket();

	long lPublicMapIndex = lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex;

	//Send Supplementary Data Block if new map requires security packages in loading this map
	const TMapRegion* rMapRgn = SECTREE_MANAGER::Instance().GetMapRegion(lPublicMapIndex);
	if (rMapRgn)
	{
		DESC_MANAGER::Instance().SendClientPackageSDBToLoadMap(d, rMapRgn->strMapName.c_str());
	}
	//if (!map_allow_find(lMapIndex >= 10000 ? lMapIndex / 10000 : lMapIndex) || !CheckEmpire(ch, lMapIndex))
	if (!map_allow_find(lPublicMapIndex))
	{
		sys_err("InputDB::PlayerLoad : entering %d map is not allowed here (name: %s, empire %u)",
			lMapIndex, pTab->name, d->GetEmpire());

		ch->SetWarpLocation(EMPIRE_START_MAP(d->GetEmpire()),
			EMPIRE_START_X(d->GetEmpire()) / 100,
			EMPIRE_START_Y(d->GetEmpire()) / 100);

		d->SetPhase(PHASE_CLOSE);
		return;
	}

	quest::CQuestManager::Instance().BroadcastEventFlagOnLogin(ch);

	for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
		ch->SetQuickslot(i, pTab->quickslot[i]);

	ch->PointsPacket();
	ch->SkillLevelPacket();

	sys_log(0, "InputDB: player_load %s %dx%dx%d LEVEL %d MOV_SPEED %d JOB %d ATG %d DFG %d GMLv %d",
		pTab->name,
		ch->GetX(), ch->GetY(), ch->GetZ(),
		ch->GetLevel(),
		ch->GetPoint(POINT_MOV_SPEED),
		ch->GetJob(),
		ch->GetPoint(POINT_ATT_GRADE),
		ch->GetPoint(POINT_DEF_GRADE),
		ch->GetGMLevel());

	ch->QuerySafeboxSize();
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (ch->GetGuild())
		ch->QueryGuildstorageSize();	//if core-crash, then delete [useless, just offical Code]
#endif
}

void CInputDB::Boot(const char* data)
{
	signal_timer_disable();

	// Packet size check
	uint32_t dwPacketSize = decode_4bytes(data);
	data += 4;

	// check packet version
	uint8_t bVersion = decode_byte(data);
	data += 1;

	sys_log(0, "BOOT: PACKET: %d", dwPacketSize);
	sys_log(0, "BOOT: VERSION: %d", bVersion);
	if (bVersion != 6)
	{
		sys_err("boot version error");
		thecore_shutdown();
	}

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
	sys_log(0, "sizeof(TAdminManager) = %d", sizeof(TAdminInfo));
	//END_ADMIN_MANAGER

	uint16_t size;

	/*
	* MOB
	*/

	if (decode_2bytes(data) != sizeof(TMobTable))
	{
		sys_err("mob table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: MOB: %d", size);

	if (size)
	{
		CMobManager::Instance().Initialize((TMobTable*)data, size);
		data += size * sizeof(TMobTable);
	}

	/*
	* ITEM
	*/

	if (decode_2bytes(data) != sizeof(TItemTable))
	{
		sys_err("item table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM: %d", size);


	if (size)
	{
		ITEM_MANAGER::Instance().Initialize((TItemTable*)data, size);
		data += size * sizeof(TItemTable);
	}

	/*
	* SHOP
	*/

	if (decode_2bytes(data) != sizeof(TShopTable))
	{
		sys_err("shop table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SHOP: %d", size);


	if (size)
	{
		if (!CShopManager::Instance().Initialize((TShopTable*)data, size))
		{
			sys_err("shop table Initialize error");
			thecore_shutdown();
			return;
		}
		data += size * sizeof(TShopTable);
	}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	/*
	 * GEM_SHOP
	 */

	if (decode_2bytes(data) != sizeof(TGemShopTable))
	{
		sys_err("gem shop table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: GEM_SHOP: %d", size);

	if (size)
	{
		if (!CShopManager::Instance().InitializeGemShop((TGemShopTable*)data, size))
		{
			sys_err("gem shop table Initialize error");
			thecore_shutdown();
			return;
		}
		data += size * sizeof(TGemShopTable);
	}
#endif

	/*
	* SKILL
	*/

	if (decode_2bytes(data) != sizeof(TSkillTable))
	{
		sys_err("skill table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SKILL: %d", size);

	if (size)
	{
		if (!CSkillManager::Instance().Initialize((TSkillTable*)data, size))
		{
			sys_err("cannot initialize skill table");
			thecore_shutdown();
			return;
		}

		data += size * sizeof(TSkillTable);
	}

	/*
	* REFINE RECIPE
	*/
	if (decode_2bytes(data) != sizeof(TRefineTable))
	{
		sys_err("refine table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: REFINE: %d", size);

	if (size)
	{
		CRefineManager::Instance().Initialize((TRefineTable*)data, size);
		data += size * sizeof(TRefineTable);
	}

	/*
	* ITEM ATTR
	*/
	if (decode_2bytes(data) != sizeof(TItemAttrTable))
	{
		sys_err("item attr table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM_ATTR: %d", size);

	if (size)
	{
		TItemAttrTable* p = (TItemAttrTable*)data;

		for (int i = 0; i < size; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;

			g_map_itemAttr[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_ATTR[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
	}

	data += size * sizeof(TItemAttrTable);


	/*
	* ITEM RARE
	*/
	if (decode_2bytes(data) != sizeof(TItemAttrTable))
	{
		sys_err("item rare table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: ITEM_RARE: %d", size);

	if (size)
	{
		TItemAttrTable* p = (TItemAttrTable*)data;

		for (int i = 0; i < size; ++i, ++p)
		{
			if (p->dwApplyIndex >= MAX_APPLY_NUM)
				continue;

			g_map_itemRare[p->dwApplyIndex] = *p;
			sys_log(0, "ITEM_RARE[%d]: %s %u", p->dwApplyIndex, p->szApply, p->dwProb);
		}
	}

	data += size * sizeof(TItemAttrTable);


	/*
	* BANWORDS
	*/

	if (decode_2bytes(data) != sizeof(TBanwordTable))
	{
		sys_err("ban word table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	CBanwordManager::Instance().Initialize((TBanwordTable*)data, size);
	data += size * sizeof(TBanwordTable);

	{
		using namespace building;

		/*
		* LANDS
		*/

		if (decode_2bytes(data) != sizeof(TLand))
		{
			sys_err("land table size error");
			thecore_shutdown();
			return;
		}
		data += 2;

		size = decode_2bytes(data);
		data += 2;

		TLand* kLand = (TLand*)data;
		data += size * sizeof(TLand);

		for (uint16_t i = 0; i < size; ++i, ++kLand)
			CManager::Instance().LoadLand(kLand);

		/*
		* OBJECT PROTO
		*/

		if (decode_2bytes(data) != sizeof(TObjectProto))
		{
			sys_err("object proto table size error");
			thecore_shutdown();
			return;
		}
		data += 2;

		size = decode_2bytes(data);
		data += 2;

		CManager::Instance().LoadObjectProto((TObjectProto*)data, size);
		data += size * sizeof(TObjectProto);

		/*
		* OBJECT
		*/
		if (decode_2bytes(data) != sizeof(TObject))
		{
			sys_err("object table size error");
			thecore_shutdown();
			return;
		}
		data += 2;

		size = decode_2bytes(data);
		data += 2;

		TObject* kObj = (TObject*)data;
		data += size * sizeof(TObject);

		for (uint16_t i = 0; i < size; ++i, ++kObj)
			CManager::Instance().LoadObject(kObj, true);
	}

#ifdef ENABLE_EVENT_MANAGER
	/*
	* EVENT
	*/

	if (decode_2bytes(data) != sizeof(TEventTable))
	{
		sys_err("event table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: EVENT: %d", size);

	if (size)
	{
		CEventManager::Instance().Initialize((TEventTable*)data, size);
		data += size * sizeof(TEventTable);
	}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	// Missions
	if (decode_2bytes(data) != sizeof(TBiologMissionsProto))
	{
		sys_err("TBiologMissionsProto table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	CBiologSystemManager::Instance().InitializeMissions((TBiologMissionsProto*)data, size);
	data += size * sizeof(TBiologMissionsProto);

	// Reward
	if (decode_2bytes(data) != sizeof(TBiologRewardsProto))
	{
		sys_err("TBiologRewardsProto table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	CBiologSystemManager::Instance().InitializeRewards((TBiologRewardsProto*)data, size);
	data += size * sizeof(TBiologRewardsProto);
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	/*
	* SUNGMA_TABLE
	*/

	if (decode_2bytes(data) != sizeof(TSungmaTable))
	{
		sys_err("sungma table size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;
	sys_log(0, "BOOT: SUNGMA_TABLE: %d", size);

	if (size)
	{
		TSungmaTable* p = (TSungmaTable*)data;
		for (size_t i = 0; i < size; ++i, ++p)
		{
			g_map_SungmaTable[p->lMapIdx] = *p;
			sys_log(0, "SUNGMA_MAP[%d]: str %d hp %d move %d immune %d "
#ifdef ENABLE_PRECISION
				"hit_pct %d"
#endif
				, p->lMapIdx, p->bSungmaStr, p->bSungmaHp, p->bSungmaMove, p->bSungmaImmune
#ifdef ENABLE_PRECISION
				, p->bHitPct
#endif
			);
		}

		data += size * sizeof(TSungmaTable);
	}
#endif

	set_global_time(*(time_t*)data);
	data += sizeof(time_t);

	if (decode_2bytes(data) != sizeof(TItemIDRangeTable))
	{
		sys_err("ITEM ID RANGE size error");
		thecore_shutdown();
		return;
	}
	data += 2;

	size = decode_2bytes(data);
	data += 2;

	TItemIDRangeTable* range = (TItemIDRangeTable*)data;
	data += size * sizeof(TItemIDRangeTable);

	TItemIDRangeTable* rangespare = (TItemIDRangeTable*)data;
	data += size * sizeof(TItemIDRangeTable);

	//ADMIN_MANAGER
	// Register as administrator
	int ChunkSize = decode_2bytes(data);
	data += 2;
	int HostSize = decode_2bytes(data);
	data += 2;
	sys_log(0, "GM Value Count %d %d", HostSize, ChunkSize);
	for (int n = 0; n < HostSize; ++n)
	{
		gm_new_host_inert(data);
		sys_log(0, "GM HOST : IP[%s] ", data);
		data += ChunkSize;
	}


	data += 2;
	int adminsize = decode_2bytes(data);
	data += 2;

	for (int n = 0; n < adminsize; ++n)
	{
		tAdminInfo& rAdminInfo = *(tAdminInfo*)data;

		gm_new_insert(rAdminInfo);

		data += sizeof(rAdminInfo);
	}

	//END_ADMIN_MANAGER

	//MONARCH
	data += 2;
	data += 2;

	TMonarchInfo& p = *(TMonarchInfo*)data;
	data += sizeof(TMonarchInfo);

	CMonarch::Instance().SetMonarchInfo(&p);

	for (int n = 1; n < 4; ++n)
	{
		if (p.name[n] && *p.name[n])
			sys_log(0, "[MONARCH] Empire %d Pid %d Money %d %s", n, p.pid[n], p.money[n], p.name[n]);
	}

	int CandidacySize = decode_2bytes(data);
	data += 2;

	int CandidacyCount = decode_2bytes(data);
	data += 2;

	if (test_server)
		sys_log(0, "[MONARCH] Size %d Count %d", CandidacySize, CandidacyCount);

	data += CandidacySize * CandidacyCount;


	//END_MONARCH

	uint16_t endCheck = decode_2bytes(data);
	if (endCheck != 0xffff)
	{
		sys_err("boot packet end check error [%x]!=0xffff", endCheck);
		thecore_shutdown();
		return;
	}
	else
		sys_log(0, "boot packet end check ok [%x]==0xffff", endCheck);
	data += 2;

	if (!ITEM_MANAGER::Instance().SetMaxItemID(*range))
	{
		sys_err("not enough item id contact your administrator!");
		thecore_shutdown();
		return;
	}

	if (!ITEM_MANAGER::Instance().SetMaxSpareItemID(*rangespare))
	{
		sys_err("not enough item id for spare contact your administrator!");
		thecore_shutdown();
		return;
	}

	// LOCALE_SERVICE
	const int FILE_NAME_LEN = 256;
	char szCommonDropItemFileName[FILE_NAME_LEN];
	char szETCDropItemFileName[FILE_NAME_LEN];
	char szMOBDropItemFileName[FILE_NAME_LEN];
	char szDropItemGroupFileName[FILE_NAME_LEN];
	char szSpecialItemGroupFileName[FILE_NAME_LEN];
	char szMapIndexFileName[FILE_NAME_LEN];
	char szItemVnumMaskTableFileName[FILE_NAME_LEN];
	char szDragonSoulTableFileName[FILE_NAME_LEN];
#ifdef ENABLE_MONSTER_BACK
	char szAttendanceRewardFileName[FILE_NAME_LEN];
#endif

	snprintf(szCommonDropItemFileName, sizeof(szCommonDropItemFileName),
		"%s/common_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szETCDropItemFileName, sizeof(szETCDropItemFileName),
		"%s/etc_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szMOBDropItemFileName, sizeof(szMOBDropItemFileName),
		"%s/mob_drop_item.txt", LocaleService_GetBasePath().c_str());
	snprintf(szSpecialItemGroupFileName, sizeof(szSpecialItemGroupFileName),
		"%s/special_item_group.txt", LocaleService_GetBasePath().c_str());
	snprintf(szDropItemGroupFileName, sizeof(szDropItemGroupFileName),
		"%s/drop_item_group.txt", LocaleService_GetBasePath().c_str());
	snprintf(szMapIndexFileName, sizeof(szMapIndexFileName),
		"%s/index", LocaleService_GetMapPath().c_str());
	snprintf(szItemVnumMaskTableFileName, sizeof(szItemVnumMaskTableFileName),
		"%s/ori_to_new_table.txt", LocaleService_GetBasePath().c_str());
	snprintf(szDragonSoulTableFileName, sizeof(szDragonSoulTableFileName),
		"%s/dragon_soul_table.txt", LocaleService_GetBasePath().c_str());
#ifdef ENABLE_MONSTER_BACK
	snprintf(szAttendanceRewardFileName, sizeof(szAttendanceRewardFileName),
		"%s/attendance_reward.txt", LocaleService_GetBasePath().c_str());
#endif

#ifndef ENABLE_CUBE_RENEWAL
	sys_log(0, "Initializing Informations of Cube System");
	Cube_InformationInitialize();
#endif

	sys_log(0, "LoadLocaleFile: CommonDropItem: %s", szCommonDropItemFileName);
	if (!ITEM_MANAGER::Instance().ReadCommonDropItemFile(szCommonDropItemFileName))
	{
		sys_err("cannot load CommonDropItem: %s", szCommonDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: ETCDropItem: %s", szETCDropItemFileName);
	if (!ITEM_MANAGER::Instance().ReadEtcDropItemFile(szETCDropItemFileName))
	{
		sys_err("cannot load ETCDropItem: %s", szETCDropItemFileName);
		thecore_shutdown();
		return;
	}

#ifdef ENABLE_MONSTER_BACK
	sys_log(0, "LoadLocaleFile: AddendanceRewardList: %s", szAttendanceRewardFileName);
	if (!CMiniGameManager::Instance().ReadRewardItemFile(szAttendanceRewardFileName))
	{
		sys_err("Cannot load AddendanceRewardList: %s", szAttendanceRewardFileName);
		thecore_shutdown();
		return;
	}
#endif

	sys_log(0, "LoadLocaleFile: DropItemGroup: %s", szDropItemGroupFileName);
	if (!ITEM_MANAGER::Instance().ReadDropItemGroup(szDropItemGroupFileName))
	{
		sys_err("cannot load DropItemGroup: %s", szDropItemGroupFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: SpecialItemGroup: %s", szSpecialItemGroupFileName);
	if (!ITEM_MANAGER::Instance().ReadSpecialDropItemFile(szSpecialItemGroupFileName))
	{
		sys_err("cannot load SpecialItemGroup: %s", szSpecialItemGroupFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: ItemVnumMaskTable : %s", szItemVnumMaskTableFileName);
	if (!ITEM_MANAGER::Instance().ReadItemVnumMaskTable(szItemVnumMaskTableFileName))
	{
		sys_log(0, "Could not open MaskItemTable");
	}

	sys_log(0, "LoadLocaleFile: MOBDropItemFile: %s", szMOBDropItemFileName);
	if (!ITEM_MANAGER::Instance().ReadMonsterDropItemGroup(szMOBDropItemFileName))
	{
		sys_err("cannot load MOBDropItemFile: %s", szMOBDropItemFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: MapIndex: %s", szMapIndexFileName);
	if (!SECTREE_MANAGER::Instance().Build(szMapIndexFileName, LocaleService_GetMapPath().c_str()))
	{
		sys_err("cannot load MapIndex: %s", szMapIndexFileName);
		thecore_shutdown();
		return;
	}

	sys_log(0, "LoadLocaleFile: DragonSoulTable: %s", szDragonSoulTableFileName);
	if (!DSManager::Instance().ReadDragonSoulTableFile(szDragonSoulTableFileName))
	{
		sys_err("cannot load DragonSoulTable: %s", szDragonSoulTableFileName);
		//thecore_shutdown();
		//return;
	}
	// END_OF_LOCALE_SERVICE

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	if (!CBattlePassManager::instance().InitializeBattlePass())
		sys_err("Failure to Initialize Extended BattlePass!");
#endif

	building::CManager::Instance().FinalizeBoot();

	CMotionManager::Instance().Build();

	signal_timer_enable(30);

	if (test_server)
		CMobManager::Instance().DumpRegenCount("mob_count");

	// castle_boot
	castle_boot();

	// request blocked_country_ip
	{
		db_clientdesc->DBPacket(HEADER_GD_BLOCK_COUNTRY_IP, 0, nullptr, 0);
		dev_log(LOG_DEB0, "<sent HEADER_GD_BLOCK_COUNTRY_IP>");
	}

#ifdef ENABLE_QUEST_BOOT_EVENT
	quest::CQuestManager::Instance().Boot();
#endif
}

EVENTINFO(quest_login_event_info)
{
	uint32_t dwPID;

	quest_login_event_info()
		: dwPID(0)
	{
	}
};

EVENTFUNC(quest_login_event)
{
	quest_login_event_info* info = dynamic_cast<quest_login_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("quest_login_event> <Factor> Null pointer");
		return 0;
	}

	uint32_t dwPID = info->dwPID;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(dwPID);

	if (!ch)
		return 0;

	LPDESC d = ch->GetDesc();

	if (!d)
		return 0;

	if (d->IsPhase(PHASE_HANDSHAKE) ||
		d->IsPhase(PHASE_LOGIN) ||
		d->IsPhase(PHASE_SELECT) ||
		d->IsPhase(PHASE_DEAD) ||
		d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		sys_log(0, "QUEST_LOAD: Login pc %d by event", ch->GetPlayerID());
		quest::CQuestManager::Instance().Login(ch->GetPlayerID());
		return 0;
	}
	else
	{
		sys_err(0, "input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		return 0;
	}
}

void CInputDB::QuestLoad(LPDESC d, const char* c_pData)
{
	if (nullptr == d)
		return;

	LPCHARACTER ch = d->GetCharacter();

	if (nullptr == ch)
		return;

	const uint32_t dwCount = decode_4bytes(c_pData);

	const TQuestTable* pQuestTable = reinterpret_cast<const TQuestTable*>(c_pData + 4);

	if (nullptr != pQuestTable)
	{
		if (dwCount != 0)
		{
			if (ch->GetPlayerID() != pQuestTable[0].dwPID)
			{
				sys_err("PID differs %u %u", ch->GetPlayerID(), pQuestTable[0].dwPID);
				return;
			}
		}

		sys_log(0, "QUEST_LOAD: count %d", dwCount);

		quest::PC* pkPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());

		if (!pkPC)
		{
			sys_err("null quest::PC with id %u", pQuestTable[0].dwPID);
			return;
		}

		if (pkPC->IsLoaded())
			return;

		for (uint32_t i = 0; i < dwCount; ++i)
		{
			std::string st(pQuestTable[i].szName);

			st += ".";
			st += pQuestTable[i].szState;

			sys_log(0, " %s %d", st.c_str(), pQuestTable[i].lValue);
			pkPC->SetFlag(st.c_str(), pQuestTable[i].lValue, false);
		}

		pkPC->SetLoaded();
		pkPC->Build();

		if (ch->GetDesc()->IsPhase(PHASE_GAME))
		{
			sys_log(0, "QUEST_LOAD: Login pc %d", pQuestTable[0].dwPID);
			quest::CQuestManager::Instance().Login(pQuestTable[0].dwPID);
		}
		else
		{
			quest_login_event_info* info = AllocEventInfo<quest_login_event_info>();
			info->dwPID = ch->GetPlayerID();

			event_create(quest_login_event, info, PASSES_PER_SEC(1));
		}
	}
}

void CInputDB::SafeboxLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const TSafeboxTable* p = (TSafeboxTable*)c_pData;

	if (d->GetAccountTable().id != p->dwID)
	{
		sys_err("SafeboxLoad: safebox has different id %u != %u", d->GetAccountTable().id, p->dwID);
		return;
	}

	uint8_t bSize = 1;

	const LPCHARACTER& ch = d->GetCharacter();
	if (!ch)
		return;

	//PREVENT_TRADE_WINDOW
	if (
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetShopOwner() ||
#endif
		ch->GetMyShop() || ch->GetExchange() || ch->IsCubeOpen()
#ifdef ENABLE_AURA_SYSTEM
		|| ch->IsAuraRefineWindowOpen()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| ch->IsChangeLookWindowOpen()
#endif
#ifdef ENABLE_MAILBOX
		|| ch->GetMailBox()
#endif
#ifdef ENABLE_CHANGED_ATTR
		|| ch->IsSelectAttr()
#endif
		)
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, "[LS;773]");
		d->GetCharacter()->CancelSafeboxLoad();
		return;
	}
	//END_PREVENT_TRADE_WINDOW

	// ADD_PREMIUM
	if (d->GetCharacter()->GetPremiumRemainSeconds(PREMIUM_SAFEBOX) > 0 ||
		d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_LARGE_SAFEBOX))
		bSize = 3;
	// END_OF_ADD_PREMIUM

	//if (d->GetCharacter()->IsEquipUniqueItem(UNIQUE_ITEM_SAFEBOX_EXPAND))
	//	bSize = 3; // warehouse expansion right

	//d->GetCharacter()->LoadSafebox(p->bSize * SAFEBOX_PAGE_SIZE, p->dwGold, p->wItemCount, (TPlayerItem *) (c_pData + sizeof(TSafeboxTable)));
#ifdef ENABLE_SAFEBOX_MONEY
	d->GetCharacter()->LoadSafebox(bSize * SAFEBOX_PAGE_SIZE, p->dwGold, p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
#else
	d->GetCharacter()->LoadSafebox(bSize * SAFEBOX_PAGE_SIZE, p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
#endif
}

void CInputDB::SafeboxChangeSize(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const uint8_t bSize = *(uint8_t*)c_pData;

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->ChangeSafeboxSize(bSize);
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CInputDB::GuildstorageChangeSize(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	uint8_t bSize = *(uint8_t*)c_pData;

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->ChangeGuildstorageSize(bSize);
}
#endif

//
// @version 05/06/20 Bang2ni - Cancel ReqSafeboxLoad
//
void CInputDB::SafeboxWrongPassword(LPDESC d)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	TPacketGCSafeboxWrongPassword p{};
	p.bHeader = HEADER_GC_SAFEBOX_WRONG_PASSWORD;
	d->Packet(&p, sizeof(p));

	d->GetCharacter()->CancelSafeboxLoad();
}

void CInputDB::SafeboxChangePasswordAnswer(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	const TSafeboxChangePasswordPacketAnswer* p = (TSafeboxChangePasswordPacketAnswer*)c_pData;
	if (p->flag)
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, "[LS;774]");
	else
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, "[LS;775]");
}

void CInputDB::MallLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const TSafeboxTable* p = (TSafeboxTable*)c_pData;

	if (d->GetAccountTable().id != p->dwID)
	{
		sys_err("safebox has different id %u != %u", d->GetAccountTable().id, p->dwID);
		return;
	}

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->LoadMall(p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CInputDB::GuildstorageLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	TSafeboxTable* p = (TSafeboxTable*)c_pData;

	//uint8_t bSize = 1;

	LPCHARACTER ch = d->GetCharacter();
	if(!ch){
		sys_err("no ch ptr");
		return;
	}

	uint32_t gID = (ch->GetGuild() ? ch->GetGuild()->GetID() : 0);
	if (gID != p->dwID)
	{
		sys_err("Guildsafebox has different id %u != %u", gID, p->dwID);
		return;
	}

	//PREVENT_TRADE_WINDOW


	if (ch->GetExchange() || ch->IsCubeOpen()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| ch->GetViewingShopOwner()
#else
		|| ch->GetShopOwner() || ch->GetMyShop()
#endif
#ifdef ENABLE_MAILBOX
		|| ch->GetMailBox()
#endif
#ifdef ENABLE_CHANGED_ATTR
		|| ch->IsSelectAttr()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| ch->IsChangeLookWindowOpen()
#endif
	)
	{
		d->GetCharacter()->ChatPacket(CHAT_TYPE_INFO, "[LS;773]");
		d->GetCharacter()->CancelSafeboxLoad();
		return;
	}
	//END_PREVENT_TRADE_WINDOW

	uint8_t bSize = ch->GetGuild()->GetGuildstorage();

	if (!d->GetCharacter())
		return;

	d->GetCharacter()->LoadGuildstorage(bSize * SAFEBOX_PAGE_SIZE, p->wItemCount, (TPlayerItem*)(c_pData + sizeof(TSafeboxTable)));
}
#endif

#ifdef ENABLE_MOVE_CHANNEL
void CInputDB::MoveChannelRespond(LPDESC d, const char* c_pData)
{
	if (d == nullptr)
		return;

	const LPCHARACTER ch = d->GetCharacter();
	if (ch == nullptr)
		return;

	ch->MoveChannel(reinterpret_cast<const TRespondMoveChannel*>(c_pData));
}
#endif

void CInputDB::LoginAlready(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	// INTERNATIONAL_VERSION Disconnect if already connected
	{
		const TPacketDGLoginAlready* p = (TPacketDGLoginAlready*)c_pData;

		LPDESC d2 = DESC_MANAGER::Instance().FindByLoginName(p->szLogin);

		if (d2)
			d2->DisconnectOfSameLogin();
		else
		{
			TPacketGGDisconnect pgg{};

			pgg.bHeader = HEADER_GG_DISCONNECT;
			strlcpy(pgg.szLogin, p->szLogin, sizeof(pgg.szLogin));

			P2P_MANAGER::Instance().Send(&pgg, sizeof(TPacketGGDisconnect));
		}
	}
	// END_OF_INTERNATIONAL_VERSION

	LoginFailure(d, "ALREADY");
}

void CInputDB::EmpireSelect(LPDESC d, const char* c_pData)
{
	sys_log(0, "EmpireSelect %p", get_pointer(d));

	if (!d)
		return;

	TAccountTable& rTable = d->GetAccountTable();
	rTable.bEmpire = *(uint8_t*)c_pData;

	TPacketGCEmpire pe{};
	pe.bHeader = HEADER_GC_EMPIRE;
	pe.bEmpire = rTable.bEmpire;
	d->Packet(&pe, sizeof(pe));

	for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		if (rTable.players[i].dwID)
		{
			rTable.players[i].x = EMPIRE_START_X(rTable.bEmpire);
			rTable.players[i].y = EMPIRE_START_Y(rTable.bEmpire);
		}

	GetServerLocation(d->GetAccountTable(), rTable.bEmpire);

	d->SendLoginSuccessPacket();
}

void CInputDB::MapLocations(const char* c_pData)
{
	uint8_t bCount = *(uint8_t*)(c_pData++);

	sys_log(0, "InputDB::MapLocations %d", bCount);

	TMapLocation* pLoc = (TMapLocation*)c_pData;

	while (bCount--)
	{
		for (int i = 0; i < MAP_ALLOW_LIMIT; ++i)
		{
			if (0 == pLoc->alMaps[i])
				break;

			CMapLocation::Instance().Insert(pLoc->alMaps[i], pLoc->szHost, pLoc->wPort);
		}

		pLoc++;
	}
}

void CInputDB::P2P(const char* c_pData)
{
	extern LPFDWATCH main_fdw;

	TPacketDGP2P* p = (TPacketDGP2P*)c_pData;

	P2P_MANAGER& mgr = P2P_MANAGER::Instance();

	if (false == DESC_MANAGER::Instance().IsP2PDescExist(p->szHost, p->wPort))
	{
		LPCLIENT_DESC pkDesc = nullptr;
		sys_log(0, "InputDB:P2P %s:%u", p->szHost, p->wPort);
		pkDesc = DESC_MANAGER::Instance().CreateConnectionDesc(main_fdw, p->szHost, p->wPort, PHASE_P2P, false);
		mgr.RegisterConnector(pkDesc);
		pkDesc->SetP2P(p->szHost, p->wPort, p->bChannel);
	}
}

void CInputDB::GuildLoad(const char* c_pData)
{
	CGuildManager::Instance().LoadGuild(*(uint32_t*)c_pData);
}

void CInputDB::GuildSkillUpdate(const char* c_pData)
{
	TPacketGuildSkillUpdate* p = (TPacketGuildSkillUpdate*)c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->guild_id);

	if (g)
	{
		g->UpdateSkill(p->skill_point, p->skill_levels);
		g->GuildPointChange(POINT_SP, p->amount, p->save ? true : false);
	}
}

void CInputDB::GuildWar(const char* c_pData)
{
	const TPacketGuildWar* p = (TPacketGuildWar*)c_pData;

	sys_log(0, "InputDB::GuildWar %u %u state %d", p->dwGuildFrom, p->dwGuildTo, p->bWar);

	switch (p->bWar)
	{
		case GUILD_WAR_SEND_DECLARE:
		case GUILD_WAR_RECV_DECLARE:
#ifdef ENABLE_NEW_WAR_OPTIONS
			CGuildManager::Instance().DeclareWar(p->dwGuildFrom, p->dwGuildTo, p->bType, p->bRound, p->bPoints, p->bTime);
#else
			CGuildManager::Instance().DeclareWar(p->dwGuildFrom, p->dwGuildTo, p->bType);
#endif
			break;

		case GUILD_WAR_REFUSE:
			CGuildManager::Instance().RefuseWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_WAIT_START:
			CGuildManager::Instance().WaitStartWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_CANCEL:
			CGuildManager::Instance().CancelWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_ON_WAR:
			CGuildManager::Instance().StartWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_END:
			CGuildManager::Instance().EndWar(p->dwGuildFrom, p->dwGuildTo);
			break;

		case GUILD_WAR_OVER:
			CGuildManager::Instance().WarOver(p->dwGuildFrom, p->dwGuildTo, p->bType);
			break;

		case GUILD_WAR_RESERVE:
			CGuildManager::Instance().ReserveWar(p->dwGuildFrom, p->dwGuildTo, p->bType);
			break;

		default:
			sys_err("Unknown guild war state");
			break;
	}
}

void CInputDB::GuildWarScore(const char* c_pData)
{
	const TPacketGuildWarScore* p = (TPacketGuildWarScore*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuildGainPoint);
	if (g)
		g->SetWarScoreAgainstTo(p->dwGuildOpponent, p->lScore);
}

void CInputDB::GuildSkillRecharge()
{
	CGuildManager::Instance().SkillRecharge();
}

#ifdef ENABLE_MAILBOX
#include "MailBox.h"
void CInputDB::MailBoxRespondLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const LPCHARACTER ch = d->GetCharacter();
	if (ch == nullptr)
		return;

	uint16_t size;

	if (decode_2bytes(c_pData) != sizeof(TMailBoxTable))
	{
		sys_err("mailbox table size error");
		return;
	}

	c_pData += 2;
	size = decode_2bytes(c_pData);
	c_pData += 2;

	CMailBox::Create(ch, (TMailBoxTable*)c_pData, size);
}

void CInputDB::MailBoxRespondName(LPDESC d, const char* c_pData)
{
	if (d == nullptr)
		return;

	const LPCHARACTER ch = d->GetCharacter();
	if (ch == nullptr)
		return;

	CMailBox* mail = ch->GetMailBox();
	if (mail == nullptr)
		return;

	mail->CheckPlayerResult((TMailBox*)c_pData);
}

void CInputDB::MailBoxRespondUnreadData(LPDESC d, const char* c_pData)
{
	if (d == nullptr)
		return;

	CMailBox::ResultUnreadData(d->GetCharacter(), (TMailBoxRespondUnreadData*)c_pData);
}
#endif

void CInputDB::GuildExpUpdate(const char* c_pData)
{
	const TPacketGuildSkillUpdate* p = (TPacketGuildSkillUpdate*)c_pData;
	sys_log(1, "GuildExpUpdate %d", p->amount);

	CGuild* g = CGuildManager::Instance().TouchGuild(p->guild_id);

	if (g)
		g->GuildPointChange(POINT_EXP, p->amount);
}

void CInputDB::GuildAddMember(const char* c_pData)
{
	TPacketDGGuildMember* p = (TPacketDGGuildMember*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	if (g)
		g->AddMember(p);
}

void CInputDB::GuildRemoveMember(const char* c_pData)
{
	const TPacketGuild* p = (TPacketGuild*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	if (g)
		g->RemoveMember(p->dwInfo);
}

void CInputDB::GuildChangeGrade(const char* c_pData)
{
	const TPacketGuild* p = (TPacketGuild*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);

	if (g)
		g->P2PChangeGrade((uint8_t)p->dwInfo);
}

void CInputDB::GuildChangeMemberData(const char* c_pData)
{
	sys_log(0, "Recv GuildChangeMemberData");
	const TPacketGuildChangeMemberData* p = (TPacketGuildChangeMemberData*)c_pData;
	CGuild* g = CGuildManager::Instance().TouchGuild(p->guild_id);

	if (g)
	{
		g->ChangeMemberData(p->pid, p->offer, p->level, p->grade
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
			, p->join_date, p->donate_limit, p->last_donation, p->daily_donate_count, p->last_daily_donate
#endif
		);
	}
}

void CInputDB::GuildDisband(const char* c_pData)
{
	const TPacketGuild* p = (TPacketGuild*)c_pData;
	CGuildManager::Instance().DisbandGuild(p->dwGuild);
}

void CInputDB::GuildLadder(const char* c_pData)
{
	TPacketGuildLadder* p = (TPacketGuildLadder*)c_pData;
	sys_log(0, "Recv GuildLadder %u %d / w %d d %d l %d", p->dwGuild, p->lLadderPoint, p->lWin, p->lDraw, p->lLoss);
	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);
	if (g)
	{
		g->SetLadderPoint(p->lLadderPoint);
		g->SetWarData(p->lWin, p->lDraw, p->lLoss);
#ifdef ENABLE_GUILD_WAR_SCORE
		g->SetNewWarData(p->lWinNew, p->lDrawNew, p->lLossNew);
#endif
	}
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CInputDB::SkillColorLoad(LPDESC d, const char* c_pData)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	ch->SetSkillColor((uint32_t*)c_pData);
}
#endif

void CInputDB::ItemLoad(LPDESC d, const char* c_pData)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	if (ch->IsItemLoaded())
		return;

	const uint32_t dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	sys_log(0, "ITEM_LOAD: COUNT %s %u", ch->GetName(), dwCount);

	std::vector<LPITEM> v;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	std::vector<TPlayerItem> shopItems;
#endif

	TPlayerItem* p = (TPlayerItem*)c_pData;
	if (!p)
		return;

	for (uint32_t i = 0; i < dwCount; ++i, ++p)
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		if (p->window == EWindows::PREMIUM_PRIVATE_SHOP)
		{
			shopItems.emplace_back(*p);
			continue;
		}
#endif

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(p->vnum, p->count, p->id);

		if (!item)
		{
			sys_err("cannot create item by vnum %u (name %s id %u)", p->vnum, ch->GetName(), p->id);
			continue;
		}

		item->SetSkipSave(true);
		item->SetSockets(p->alSockets);
		item->SetAttributes(p->aAttr);
#ifdef ENABLE_YOHARA_SYSTEM
		item->SetRandomAttrs(p->aApplyRandom);
		item->SetRandomDefaultAttrs(p->alRandomValues);
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
		item->SetSealDate(p->nSealDate);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		item->SetChangeLookVnum(p->dwTransmutationVnum);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
		item->SetBasic(p->is_basic);
#endif
#ifdef ENABLE_REFINE_ELEMENT
		item->SetElement(p->grade_element, p->attack_element, p->element_type_bonus, p->elements_value_bonus);
#endif
#ifdef ENABLE_SET_ITEM
		item->SetItemSetValue(p->set_value);
#endif
		item->SetLastOwnerPID(p->owner);

#ifdef ENABLE_CHANGE_LOOK_MOUNT
		/*if (item->IsHorseSummonItem() && item->GetChangeLookVnum())
			ch->SetHorseChangeLookVnum(item->GetChangeLookVnum());*/
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		item->SetGrowthPetItemInfo(p->aPetInfo);

		if (item->GetType() == ITEM_PET)
		{
			switch (item->GetSubType())
			{
				case PET_BAG:
				{
					ch->SetGrowthPetInfo(p->aPetInfo);
					if (test_server)
						sys_err("PET_BAG: SetGrowthPetInfo: vnum: %d, owner: %d pet_id: %d", p->vnum, p->owner, p->aPetInfo.pet_id);
				}
				break;

				case PET_UPBRINGING:
				{
					ch->SetGrowthPetInfo(p->aPetInfo);
					if (test_server)
						sys_err("PET_UPBRINGING: SetGrowthPetInfo: vnum: %d, owner: %d pet_id: %d", p->vnum, p->owner, p->aPetInfo.pet_id);

					const long lPetDuration = (item->GetSocket(0) - get_global_time()) / 60;
					if (lPetDuration <= 0)
					{
						if (item->GetSocket(3) == TRUE)
							item->SetSocket(3, FALSE);

						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1149;%s]", item->GetName());
					}

#	ifdef ENABLE_PET_SUMMON_AFTER_REWARP
				else if ((p->window == INVENTORY) && (item->GetSocket(3) == TRUE) && (!ch->GetSummonGrowthPet()))
					ch->SetSummonGrowthPet(item);
#	endif
				}
				break;

#ifdef ENABLE_PET_SUMMON_AFTER_REWARP
				case PET_PAY:
				{
					const long lPetDuration = (item->GetSocket(0) - get_global_time()) / 60;
					if (lPetDuration > 0)
					{
						if ((p->window == INVENTORY) && (item->GetSocket(1) == TRUE) && (!ch->GetSummonPetPay()))
							ch->SetSummonPetPay(item);
					}
				}
				break;
#endif

				default:
					break;
			}
		}
#endif

		if ((p->window == INVENTORY && ch->GetInventoryItem(p->pos)) ||
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			(p->window == EQUIPMENT && ch->GetWearDefault(static_cast<uint8_t>(p->pos)))
#else
			(p->window == EQUIPMENT && ch->GetWear(static_cast<uint8_t>(p->pos)))
#endif
			|| (p->window == BELT_INVENTORY && ch->GetBeltInventoryItem(p->pos))
			)
		{
			sys_log(0, "ITEM_RESTORE: %s %s", ch->GetName(), item->GetName());
			v.emplace_back(item);
		}
		else
		{
			switch (p->window)
			{
				case INVENTORY:
				case DRAGON_SOUL_INVENTORY:
#ifdef ENABLE_BELT_INVENTORY
				case BELT_INVENTORY:
#endif
#ifdef ENABLE_SWITCHBOT
				case SWITCHBOT:
#endif
#ifdef ENABLE_ATTR_6TH_7TH
				case NPC_STORAGE:
#endif
					item->AddToCharacter(ch, TItemPos(p->window, p->pos));
					break;

				case EQUIPMENT:
				{
					if (item->CheckItemUseLevel(ch->GetLevel()) == true)
					{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
						if (!ch->CheckAdditionalEquipment(p->pos))
						{
							if (item->EquipTo(ch, p->pos) == false)
							{
								v.push_back(item);
							}
						}
						else
						{
							if (item->EquipToDB(ch, p->pos) == false)
							{
								v.push_back(item);
							}
						}
#else
						if (item->EquipTo(ch, static_cast<uint8_t>(p->pos)) == false)
						{
							v.emplace_back(item);
						}
#endif
					}
					else
					{
						v.emplace_back(item);
					}
					break;
				}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
				case ADDITIONAL_EQUIPMENT_1:
				{
					item->AddToCharacter(ch, TItemPos(p->window, p->pos));
					if (ch->CheckAdditionalEquipment(p->pos))
					{
						item->RefreshAdditionalEquipmentItems(ch, p->pos, true);
					}
				}
				break;
#endif

				default:
					break;
			}
		}

		if (false == item->OnAfterCreatedItem())
			sys_err("Failed to call ITEM::OnAfterCreatedItem (vnum: %d, id: %d)", item->GetVnum(), item->GetID());

		item->SetSkipSave(false);
	}

	auto it = v.begin();

	while (it != v.end())
	{
		LPITEM item = *(it++);
		if (!item)
			continue;

#ifdef ENABLE_SPECIAL_INVENTORY
		const int pos = ch->GetEmptyInventory(item);
#else
		const int pos = ch->GetEmptyInventory(item->GetSize());
#endif

		if (pos < 0)
		{
			PIXEL_POSITION coord;
			coord.x = ch->GetX();
			coord.y = ch->GetY();

			item->AddToGround(ch->GetMapIndex(), coord);
			item->SetOwnership(ch, 180);
			item->StartDestroyEvent();
		}
		else
		{
			item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
		}
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	ch->SetShopItems(shopItems);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	ch->SendGrowthPetInfoPacket();
#endif

	ch->CheckMaximumPoints();
	ch->PointsPacket();
	ch->SetItemLoaded();
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
/*
* Obtain all the information about the shop (items, name, gold, times and position),
* cache gold locally and send everything to the client.
*/
void CInputDB::MyShopInfoLoad(LPDESC d, const char* c_pData)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	// Amount of items in the shop
	const uint8_t bCount = decode_byte(c_pData);
	c_pData += sizeof(uint8_t);

	// Shop sign
	TPacketPlayerShopSign namePack{};
	namePack.header = HEADER_GC_MY_SHOP_SIGN;
	strlcpy(namePack.sign, c_pData, sizeof(namePack.sign));
	ch->GetDesc()->Packet(&namePack, sizeof(TPacketPlayerShopSign));
	c_pData += sizeof(namePack.sign);

	// Gold stash in the shop
	const int goldStash = (int)decode_4bytes(c_pData);
	c_pData += sizeof(int);

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	const int chequeStash = (int)decode_4bytes(c_pData);
	c_pData += sizeof(int);
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	// Offline Time on our shop
	const int offlineTime = decode_4bytes(c_pData);
	c_pData += sizeof(int);
#endif

	// Channel where our shop is at
	const int channel = decode_4bytes(c_pData);
	c_pData += sizeof(int);

	// X-Cord of our shop
	const int x = decode_4bytes(c_pData);
	c_pData += sizeof(int);

	// Y-Cord of our shop
	const int y = decode_4bytes(c_pData);
	c_pData += sizeof(int);

	// Load shop items
	if (bCount > 0)
	{
		std::vector<TPlayerItem> shopItems = ch->GetShopItems();
		if (shopItems.empty())
		{
			sys_err("Trying to load myshop items without having loaded items! (%d items on shop)", bCount);
			c_pData += sizeof(TMyShopPriceInfo) * bCount;
			return;
		}

#ifdef ENABLE_MYSHOP_DECO
		if (bCount > SHOP_HOST_ITEM_MAX)
#else
		if (bCount > SHOP_HOST_ITEM_MAX_NUM)
#endif
		{
			sys_err("MyShopItemLoad: Cannot load %d items (Over the max)", bCount);
			c_pData += sizeof(TMyShopPriceInfo) * bCount;
			return;
		}

		TMyShopPriceInfo* p = (TMyShopPriceInfo*)c_pData;

#ifdef ENABLE_MYSHOP_DECO
		std::array <TMyShopPriceInfo, SHOP_HOST_ITEM_MAX> info;
#else
		std::array <TMyShopPriceInfo, SHOP_HOST_ITEM_MAX_NUM> info;
#endif
		for (uint8_t i = 0; i < bCount; ++i, ++p)
		{
			info[p->pos] = *p;
		}

		for (const auto& item : shopItems)
		{
			if (info[item.pos].price == 0
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
				&& info[item.pos].cheque == 0
#endif
				)
			{
				sys_err("Position %d has no price when loading items for shop inventory of #%u", item.pos, ch->GetPlayerID());
				continue;
			}

			TPacketPlayerShopSet pack{};
			pack.header = HEADER_GC_PLAYER_SHOP_SET;
			pack.pos = (uint8_t)item.pos;

			pack.count = (uint8_t)item.count;
			pack.vnum = item.vnum;
			pack.price = info[item.pos].price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			pack.cheque = info[item.pos].cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			pack.dwTransmutationVnum = item.dwTransmutationVnum;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
			pack.grade_element = item.grade_element;
			pack.element_type_bonus = item.element_type_bonus;
			thecore_memcpy(pack.attack_element, item.attack_element, sizeof(pack.attack_element));
			thecore_memcpy(pack.elements_value_bonus, item.elements_value_bonus, sizeof(pack.elements_value_bonus));
#endif

			thecore_memcpy(pack.alSockets, item.alSockets, sizeof(pack.alSockets));
			thecore_memcpy(pack.aAttr, item.aAttr, sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(pack.aApplyRandom, item.aApplyRandom, sizeof(pack.aApplyRandom));
			thecore_memcpy(pack.alRandomValues, item.alRandomValues, sizeof(pack.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
			pack.set_value = item.set_value;
#endif

			ch->GetDesc()->Packet(&pack, sizeof(TPacketPlayerShopSet));
		}
	}

	// Set the gold and cheque our shop has
	ch->AlterShopGoldStash(goldStash);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	ch->AlterShopChequeStash(chequeStash);
#endif

	// Sync gold stash on client
	TPacketGCShopStashSync stash{};
	stash.bHeader = HEADER_GC_SYNC_SHOP_STASH;
	stash.value = goldStash;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	stash.cheque = chequeStash;
#endif
	ch->GetDesc()->Packet(&stash, sizeof(TPacketGCShopStashSync));

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	// Sync offline time on client
	TPacketGCShopOffTimeSync pack{};
	pack.bHeader = HEADER_GC_SYNC_SHOP_OFFTIME;
	pack.value = offlineTime;
	ch->GetDesc()->Packet(&pack, sizeof(TPacketGCShopOffTimeSync));
#endif

	// Sync position on client
	TPacketGCShopSyncPos pos{};
	pos.bHeader = HEADER_GC_SYNC_SHOP_POSITION;
	pos.channel = channel;
	pos.xGlobal = x;
	pos.yGlobal = y;
	ch->GetDesc()->Packet(&pos, sizeof(TPacketGCShopSyncPos));

	// If player has been disconnected for over an hour, and they have gold in
	// the shop stash, remember them so!
#ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
	if (ch->GetLogOffInterval() > 3600 && (goldStash > 0 || chequeStash > 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1717]");
		if (goldStash > 0 && chequeStash > 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang and %d Won."), goldStash, chequeStash);
		else if (goldStash > 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang."), goldStash);
		else if (chequeStash > 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Won."), chequeStash);
	}
#else
	if (ch->GetLogOffInterval() > 3600 && goldStash > 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1717][Personal Premium Shop] You can collect money in your personal shop."));
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total earnings: %d Yang."), goldStash);
	}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	if (offlineTime > 0 && CShop::CanOpenShopHere(ch->GetMapIndex()))
		ch->SyncPrivateShopTailInfo();
#endif
}

void CInputDB::CloseShop(const char* c_pData)
{
	const uint32_t pid = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	LPCHARACTER shopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!shopChar)
		return;

	M2_DESTROY_CHARACTER(shopChar);
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
void CInputDB::ExtBattlePassLoad(LPDESC d, const char* c_pData)
{
	if (!d || !d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();
	if (!ch)
		return;

	uint32_t dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	uint32_t dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	if (ch->GetPlayerID() != dwPID)
		return;

	ch->LoadExtBattlePass(dwCount, (TPlayerExtBattlePassMission*)c_pData);
}
#endif

void CInputDB::AffectLoad(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	if (!d->GetCharacter())
		return;

	LPCHARACTER ch = d->GetCharacter();
	if (!ch)
		return;

	const uint32_t dwPID = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	const uint32_t dwCount = decode_4bytes(c_pData);
	c_pData += sizeof(uint32_t);

	if (ch->GetPlayerID() != dwPID)
		return;

	ch->LoadAffect(dwCount, (TPacketGDAffectElement*)c_pData);
#ifdef ENABLE_SET_ITEM
	ch->RefreshSetBonus();
#endif
}

void CInputDB::PartyCreate(const char* c_pData)
{
	const TPacketPartyCreate* p = (TPacketPartyCreate*)c_pData;
	CPartyManager::Instance().P2PCreateParty(p->dwLeaderPID);
}

void CInputDB::PartyDelete(const char* c_pData)
{
	const TPacketPartyDelete* p = (TPacketPartyDelete*)c_pData;
	CPartyManager::Instance().P2PDeleteParty(p->dwLeaderPID);
}

void CInputDB::PartyAdd(const char* c_pData)
{
	const TPacketPartyAdd* p = (TPacketPartyAdd*)c_pData;
	CPartyManager::Instance().P2PJoinParty(p->dwLeaderPID, p->dwPID, p->bState);
}

void CInputDB::PartyRemove(const char* c_pData)
{
	const TPacketPartyRemove* p = (TPacketPartyRemove*)c_pData;
	CPartyManager::Instance().P2PQuitParty(p->dwPID);
}

void CInputDB::PartyStateChange(const char* c_pData)
{
	const TPacketPartyStateChange* p = (TPacketPartyStateChange*)c_pData;
	LPPARTY pParty = CPartyManager::Instance().P2PCreateParty(p->dwLeaderPID);

	if (!pParty)
		return;

	pParty->SetRole(p->dwPID, p->bRole, p->bFlag);
}

void CInputDB::PartySetMemberLevel(const char* c_pData)
{
	const TPacketPartySetMemberLevel* p = (TPacketPartySetMemberLevel*)c_pData;
	LPPARTY pParty = CPartyManager::Instance().P2PCreateParty(p->dwLeaderPID);

	if (!pParty)
		return;

	pParty->P2PSetMemberLevel(p->dwPID, p->bLevel);
}

void CInputDB::Time(const char* c_pData)
{
	set_global_time(*(time_t*)c_pData);
}

void CInputDB::ReloadProto(const char* c_pData)
{
	uint16_t wSize;

	/*
	* Skill
	*/
	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(uint16_t);
	if (wSize) CSkillManager::Instance().Initialize((TSkillTable*)c_pData, wSize);
	c_pData += sizeof(TSkillTable) * wSize;

	/*
	* Banwords
	*/

	wSize = decode_2bytes(c_pData);
	c_pData += sizeof(uint16_t);
	CBanwordManager::Instance().Initialize((TBanwordTable*)c_pData, wSize);
	c_pData += sizeof(TBanwordTable) * wSize;

	/*
	* ITEM
	*/
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: ITEM: %d", wSize);

	if (wSize)
	{
		ITEM_MANAGER::Instance().Initialize((TItemTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TItemTable);
	}

	/*
	* MONSTER
	*/
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: MOB: %d", wSize);

	if (wSize)
	{
		CMobManager::Instance().Initialize((TMobTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TMobTable);
	}

#ifdef ENABLE_EXTENDED_RELOAD
	/*
	* REFINE
	*/
	wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: REFINE: %d", wSize);

	if (wSize)
	{
		CRefineManager::instance().Initialize((TRefineTable*)c_pData, wSize);
		c_pData += wSize * sizeof(TRefineTable);
	}
#endif

	CMotionManager::Instance().Build();

	CHARACTER_MANAGER::Instance().for_each_pc(std::mem_fn(&CHARACTER::ComputePoints));
}

void CInputDB::GuildSkillUsableChange(const char* c_pData)
{
	const TPacketGuildSkillUsableChange* p = (TPacketGuildSkillUsableChange*)c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);
	if (g)
		g->SkillUsableChange(p->dwSkillVnum, p->bUsable ? true : false);
}

void CInputDB::AuthLogin(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const uint8_t bResult = *(uint8_t*)c_pData;

	TPacketGCAuthSuccess ptoc{};

	ptoc.bHeader = HEADER_GC_AUTH_SUCCESS;

	if (bResult)
	{
		// Send the required key to the Panama encryption pack
		SendPanamaList(d);
		ptoc.dwLoginKey = d->GetLoginKey();

		//NOTE: Must be sent before AuthSucess, otherwise it will be PHASE Closed and not sent.-_-
		//Send Client Package CryptKey
		{
			DESC_MANAGER::Instance().SendClientPackageCryptKey(d);
			DESC_MANAGER::Instance().SendClientPackageSDBToLoadMap(d, MAPNAME_DEFAULT);
		}
	}
	else
	{
		ptoc.dwLoginKey = 0;
	}

	ptoc.bResult = bResult;

	d->Packet(&ptoc, sizeof(TPacketGCAuthSuccess));
	sys_log(0, "AuthLogin result %u key %u", bResult, d->GetLoginKey());
}

void CInputDB::ChangeEmpirePriv(const char* c_pData)
{
	const TPacketDGChangeEmpirePriv* p = (TPacketDGChangeEmpirePriv*)c_pData;

	// ADD_EMPIRE_PRIV_TIME
	CPrivManager::Instance().GiveEmpirePriv(p->empire, p->type, p->value, p->bLog, p->end_time_sec);
	// END_OF_ADD_EMPIRE_PRIV_TIME
}

/**
* @version 05/06/08 Bang2ni - added duration
*/
void CInputDB::ChangeGuildPriv(const char* c_pData)
{
	const TPacketDGChangeGuildPriv* p = (TPacketDGChangeGuildPriv*)c_pData;

	// ADD_GUILD_PRIV_TIME
	CPrivManager::Instance().GiveGuildPriv(p->guild_id, p->type, p->value, p->bLog, p->end_time_sec);
	// END_OF_ADD_GUILD_PRIV_TIME
}

void CInputDB::ChangeCharacterPriv(const char* c_pData)
{
	const TPacketDGChangeCharacterPriv* p = (TPacketDGChangeCharacterPriv*)c_pData;
	CPrivManager::Instance().GiveCharacterPriv(p->pid, p->type, p->value, p->bLog);
}

void CInputDB::MoneyLog(const char* c_pData)
{
	const TPacketMoneyLog* p = (TPacketMoneyLog*)c_pData;

	if (p->type == 4) // QUEST_MONEY_LOG_SKIP
		return;

	if (g_bAuthServer)
		return;

	LogManager::Instance().MoneyLog(p->type, p->vnum, p->gold);
}

void CInputDB::GuildMoneyChange(const char* c_pData)
{
	const TPacketDGGuildMoneyChange* p = (TPacketDGGuildMoneyChange*)c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);
	if (g)
		g->RecvMoneyChange(p->iTotalGold);
}

#ifndef ENABLE_USE_MONEY_FROM_GUILD
void CInputDB::GuildWithdrawMoney(const char* c_pData)
{
	const TPacketDGGuildMoneyWithdraw* p = (TPacketDGGuildMoneyWithdraw*)c_pData;

	CGuild* g = CGuildManager::Instance().TouchGuild(p->dwGuild);
	if (g)
		g->RecvWithdrawMoneyGive(p->iChangeGold);
}
#endif

void CInputDB::SetEventFlag(const char* c_pData)
{
	const TPacketSetEventFlag* p = (TPacketSetEventFlag*)c_pData;
	quest::CQuestManager::Instance().SetEventFlag(p->szFlagName, p->lValue);
}

void CInputDB::CreateObject(const char* c_pData)
{
	using namespace building;
	CManager::Instance().LoadObject((TObject*)c_pData);
}

void CInputDB::DeleteObject(const char* c_pData)
{
	using namespace building;
	CManager::Instance().DeleteObject(*(uint32_t*)c_pData);
}

void CInputDB::UpdateLand(const char* c_pData)
{
	using namespace building;
	CManager::Instance().UpdateLand((TLand*)c_pData);
}

void CInputDB::Notice(const char* c_pData)
{
	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData, sizeof(szBuf));

	sys_log(0, "InputDB:: Notice: %s", szBuf);

	SendNotice(szBuf);
}

void CInputDB::GuildWarReserveAdd(TGuildWarReserve* p)
{
	CGuildManager::Instance().ReserveWarAdd(p);
}

void CInputDB::GuildWarReserveDelete(uint32_t dwID)
{
	CGuildManager::Instance().ReserveWarDelete(dwID);
}

void CInputDB::GuildWarBet(TPacketGDGuildWarBet* p)
{
	CGuildManager::Instance().ReserveWarBet(p);
}

void CInputDB::MarriageAdd(TPacketMarriageAdd* p)
{
	sys_log(0, "MarriageAdd %u %u %u %s %s", p->dwPID1, p->dwPID2, (uint32_t)p->tMarryTime, p->szName1, p->szName2);
	marriage::CManager::Instance().Add(p->dwPID1, p->dwPID2, p->tMarryTime, p->szName1, p->szName2);
}

void CInputDB::MarriageUpdate(TPacketMarriageUpdate* p)
{
	sys_log(0, "MarriageUpdate %u %u %d %d", p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
	marriage::CManager::Instance().Update(p->dwPID1, p->dwPID2, p->iLovePoint, p->byMarried);
}

void CInputDB::MarriageRemove(TPacketMarriageRemove* p)
{
	sys_log(0, "MarriageRemove %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().Remove(p->dwPID1, p->dwPID2);
}

void CInputDB::WeddingRequest(TPacketWeddingRequest* p)
{
	marriage::WeddingManager::Instance().Request(p->dwPID1, p->dwPID2);
}

void CInputDB::WeddingReady(TPacketWeddingReady* p)
{
	sys_log(0, "WeddingReady %u %u %u", p->dwPID1, p->dwPID2, p->dwMapIndex);
	marriage::CManager::Instance().WeddingReady(p->dwPID1, p->dwPID2, p->dwMapIndex);
}

void CInputDB::WeddingStart(TPacketWeddingStart* p)
{
	sys_log(0, "WeddingStart %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().WeddingStart(p->dwPID1, p->dwPID2);
}

void CInputDB::WeddingEnd(TPacketWeddingEnd* p)
{
	sys_log(0, "WeddingEnd %u %u", p->dwPID1, p->dwPID2);
	marriage::CManager::Instance().WeddingEnd(p->dwPID1, p->dwPID2);
}

// MYSHOP_PRICE_LIST
void CInputDB::MyshopPricelistRes(LPDESC d, const TPacketMyshopPricelistHeader* p)
{
	LPCHARACTER ch;

	if (!d || !(ch = d->GetCharacter()))
		return;

	sys_log(0, "RecvMyshopPricelistRes name[%s]", ch->GetName());
	ch->UseSilkBotaryReal(p);

}
// END_OF_MYSHOP_PRICE_LIST


//RELOAD_ADMIN
void CInputDB::ReloadAdmin(const char* c_pData)
{
	gm_new_clear();
	const int ChunkSize = decode_2bytes(c_pData);
	c_pData += 2;
	const int HostSize = decode_2bytes(c_pData);
	c_pData += 2;

	for (int n = 0; n < HostSize; ++n)
	{
		gm_new_host_inert(c_pData);
		c_pData += ChunkSize;
	}


	c_pData += 2;
	const int size = decode_2bytes(c_pData);
	c_pData += 2;

	for (int n = 0; n < size; ++n)
	{
		const tAdminInfo& rAdminInfo = *(tAdminInfo*)c_pData;

		gm_new_insert(rAdminInfo);

		c_pData += sizeof(tAdminInfo);

		LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindPC(rAdminInfo.m_szName);
		if (pChar)
		{
			pChar->SetGMLevel();
		}
	}

}
//END_RELOAD_ADMIN

////////////////////////////////////////////////////////////////////
// Analyze
// @version 05/06/10 Bang2ni - Added item price information list packet (HEADER_DG_MYSHOP_PRICELIST_RES) processing routine.
////////////////////////////////////////////////////////////////////
int CInputDB::Analyze(LPDESC d, uint8_t bHeader, const char* c_pData)
{
	switch (bHeader)
	{
		case HEADER_DG_BOOT:
			Boot(c_pData);
			break;

		case HEADER_DG_LOGIN_SUCCESS:
			LoginSuccess(m_dwHandle, c_pData);
			break;

		case HEADER_DG_LOGIN_NOT_EXIST:
			LoginFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), "NOID");
			break;

		case HEADER_DG_LOGIN_WRONG_PASSWD:
			LoginFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), "WRONGPWD");
			break;

		case HEADER_DG_LOGIN_ALREADY:
			LoginAlready(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_PLAYER_LOAD_SUCCESS:
			PlayerLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_PLAYER_CREATE_SUCCESS:
			PlayerCreateSuccess(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_PLAYER_CREATE_FAILED:
			PlayerCreateFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), 0);
			break;

		case HEADER_DG_PLAYER_CREATE_ALREADY:
			PlayerCreateFailure(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), 1);
			break;

		case HEADER_DG_PLAYER_DELETE_SUCCESS:
			PlayerDeleteSuccess(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_PLAYER_LOAD_FAILED:
			//sys_log(0, "PLAYER_LOAD_FAILED");
			break;

		case HEADER_DG_PLAYER_DELETE_FAILED:
			//sys_log(0, "PLAYER_DELETE_FAILED");
#ifdef ENABLE_DELETE_FAILURE_TYPE
			PlayerDeleteFail(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
#else
			PlayerDeleteFail(DESC_MANAGER::Instance().FindByHandle(m_dwHandle));
#endif
			break;

		case HEADER_DG_ITEM_LOAD:
			ItemLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_QUEST_LOAD:
			QuestLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_AFFECT_LOAD:
			AffectLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_SAFEBOX_LOAD:
			SafeboxLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_SAFEBOX_CHANGE_SIZE:
			SafeboxChangeSize(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case HEADER_DG_GUILDSTORAGE_CHANGE_SIZE:
			GuildstorageChangeSize(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

		case HEADER_DG_SAFEBOX_WRONG_PASSWORD:
			SafeboxWrongPassword(DESC_MANAGER::Instance().FindByHandle(m_dwHandle));
			break;

		case HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER:
			SafeboxChangePasswordAnswer(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_MALL_LOAD:
			MallLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case HEADER_DG_GUILDSTORAGE_LOAD:
			GuildstorageLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

#ifdef ENABLE_MOVE_CHANNEL
		case HEADER_DG_RESPOND_MOVE_CHANNEL:
			MoveChannelRespond(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

		case HEADER_DG_EMPIRE_SELECT:
			EmpireSelect(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_MAP_LOCATIONS:
			MapLocations(c_pData);
			break;

		case HEADER_DG_P2P:
			P2P(c_pData);
			break;

		case HEADER_DG_GUILD_SKILL_UPDATE:
			GuildSkillUpdate(c_pData);
			break;

		case HEADER_DG_GUILD_LOAD:
			GuildLoad(c_pData);
			break;

		case HEADER_DG_GUILD_SKILL_RECHARGE:
			GuildSkillRecharge();
			break;

		case HEADER_DG_GUILD_EXP_UPDATE:
			GuildExpUpdate(c_pData);
			break;

		case HEADER_DG_PARTY_CREATE:
			PartyCreate(c_pData);
			break;

		case HEADER_DG_PARTY_DELETE:
			PartyDelete(c_pData);
			break;

		case HEADER_DG_PARTY_ADD:
			PartyAdd(c_pData);
			break;

		case HEADER_DG_PARTY_REMOVE:
			PartyRemove(c_pData);
			break;

		case HEADER_DG_PARTY_STATE_CHANGE:
			PartyStateChange(c_pData);
			break;

		case HEADER_DG_PARTY_SET_MEMBER_LEVEL:
			PartySetMemberLevel(c_pData);
			break;

		case HEADER_DG_TIME:
			Time(c_pData);
			break;

		case HEADER_DG_GUILD_ADD_MEMBER:
			GuildAddMember(c_pData);
			break;

		case HEADER_DG_GUILD_REMOVE_MEMBER:
			GuildRemoveMember(c_pData);
			break;

		case HEADER_DG_GUILD_CHANGE_GRADE:
			GuildChangeGrade(c_pData);
			break;

		case HEADER_DG_GUILD_CHANGE_MEMBER_DATA:
			GuildChangeMemberData(c_pData);
			break;

		case HEADER_DG_GUILD_DISBAND:
			GuildDisband(c_pData);
			break;

		case HEADER_DG_RELOAD_PROTO:
			ReloadProto(c_pData);
			break;

		case HEADER_DG_GUILD_WAR:
			GuildWar(c_pData);
			break;

		case HEADER_DG_GUILD_WAR_SCORE:
			GuildWarScore(c_pData);
			break;

		case HEADER_DG_GUILD_LADDER:
			GuildLadder(c_pData);
			break;

		case HEADER_DG_GUILD_SKILL_USABLE_CHANGE:
			GuildSkillUsableChange(c_pData);
			break;

		case HEADER_DG_CHANGE_NAME:
			ChangeName(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_AUTH_LOGIN:
			AuthLogin(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_CHANGE_EMPIRE_PRIV:
			ChangeEmpirePriv(c_pData);
			break;

		case HEADER_DG_CHANGE_GUILD_PRIV:
			ChangeGuildPriv(c_pData);
			break;

		case HEADER_DG_CHANGE_CHARACTER_PRIV:
			ChangeCharacterPriv(c_pData);
			break;

		case HEADER_DG_MONEY_LOG:
			MoneyLog(c_pData);
			break;

#ifndef ENABLE_USE_MONEY_FROM_GUILD
		case HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE:
			GuildWithdrawMoney(c_pData);
			break;
#endif

		case HEADER_DG_GUILD_MONEY_CHANGE:
			GuildMoneyChange(c_pData);
			break;

		case HEADER_DG_SET_EVENT_FLAG:
			SetEventFlag(c_pData);
			break;

		case HEADER_DG_CREATE_OBJECT:
			CreateObject(c_pData);
			break;

		case HEADER_DG_DELETE_OBJECT:
			DeleteObject(c_pData);
			break;

		case HEADER_DG_UPDATE_LAND:
			UpdateLand(c_pData);
			break;

		case HEADER_DG_NOTICE:
			Notice(c_pData);
			break;

		case HEADER_DG_GUILD_WAR_RESERVE_ADD:
			GuildWarReserveAdd((TGuildWarReserve *) c_pData);
			break;

		case HEADER_DG_GUILD_WAR_RESERVE_DEL:
			GuildWarReserveDelete(*(uint32_t *) c_pData);
			break;

		case HEADER_DG_GUILD_WAR_BET:
			GuildWarBet((TPacketGDGuildWarBet *) c_pData);
			break;

		case HEADER_DG_MARRIAGE_ADD:
			MarriageAdd((TPacketMarriageAdd*) c_pData);
			break;

		case HEADER_DG_MARRIAGE_UPDATE:
			MarriageUpdate((TPacketMarriageUpdate*) c_pData);
			break;

		case HEADER_DG_MARRIAGE_REMOVE:
			MarriageRemove((TPacketMarriageRemove*) c_pData);
			break;

		case HEADER_DG_WEDDING_REQUEST:
			WeddingRequest((TPacketWeddingRequest*) c_pData);
			break;

		case HEADER_DG_WEDDING_READY:
			WeddingReady((TPacketWeddingReady*) c_pData);
			break;

		case HEADER_DG_WEDDING_START:
			WeddingStart((TPacketWeddingStart*) c_pData);
			break;

		case HEADER_DG_WEDDING_END:
			WeddingEnd((TPacketWeddingEnd*) c_pData);
			break;

			// MYSHOP_PRICE_LIST
		case HEADER_DG_MYSHOP_PRICELIST_RES:
			MyshopPricelistRes(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), (TPacketMyshopPricelistHeader*) c_pData );
			break;
			// END_OF_MYSHOP_PRICE_LIST

		// RELOAD_ADMIN
		case HEADER_DG_RELOAD_ADMIN:
			ReloadAdmin(c_pData );
			break;
		//END_RELOAD_ADMIN

		case HEADER_DG_ADD_MONARCH_MONEY:
			AddMonarchMoney(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData );
			break;

		case HEADER_DG_DEC_MONARCH_MONEY:
			DecMonarchMoney(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData );
			break;

		case HEADER_DG_TAKE_MONARCH_MONEY:
			TakeMonarchMoney(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData );
			break;

		case HEADER_DG_CHANGE_MONARCH_LORD_ACK :
			ChangeMonarchLord((TPacketChangeMonarchLordACK*)c_pData);
			break;

		case HEADER_DG_UPDATE_MONARCH_INFO :
			UpdateMonarchInfo((TMonarchInfo*)c_pData);
			break;

		case HEADER_DG_BLOCK_COUNTRY_IP:
			this->AddBlockCountryIp((TPacketBlockCountryIp *) c_pData);
			break;
		case HEADER_DG_BLOCK_EXCEPTION:
			this->BlockException((TPacketBlockException *) c_pData);
			break;

		case HEADER_DG_ACK_CHANGE_GUILD_MASTER :
			this->GuildChangeMaster((TPacketChangeGuildMaster*) c_pData);
			break;
		case HEADER_DG_ACK_SPARE_ITEM_ID_RANGE :
			ITEM_MANAGER::Instance().SetMaxSpareItemID(*((TItemIDRangeTable*)c_pData) );
			break;

		case HEADER_DG_UPDATE_HORSE_NAME :
		case HEADER_DG_ACK_HORSE_NAME :
			CHorseNameManager::Instance().UpdateHorseName(
					((TPacketUpdateHorseName*)c_pData)->dwPlayerID,
					((TPacketUpdateHorseName*)c_pData)->szHorseName);
			break;

		case HEADER_DG_NEED_LOGIN_LOG:
			DetailLog( (TPacketNeedLoginLogInfo*) c_pData );
			break;

		case HEADER_DG_ITEMAWARD_INFORMER:
			ItemAwardInformer((TPacketItemAwardInfromer*) c_pData);
			break;

		case HEADER_DG_RESPOND_CHANNELSTATUS:
			RespondChannelStatus(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		case HEADER_DG_GUILD_DUNGEON:
			GuildDungeon(c_pData);
			break;

		case HEADER_DG_GUILD_DUNGEON_CD:
			GuildDungeonCD(c_pData);
			break;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
		case HEADER_DG_SKILL_COLOR_LOAD:
			SkillColorLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case HEADER_DG_ACHIEVEMENT:
			CAchievementSystem::Instance().ProcessDBPackets(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		//SHOP
		case HEADER_DG_SHOP_MYINFO_LOAD:
			MyShopInfoLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_SHOP_SALE_INFO:
			ReceiveShopSaleInfo(c_pData);
			break;

		case HEADER_DG_PLAYER_SPAWN_SHOP:
			SpawnPlayerShops(c_pData);
			break;

		case HEADER_DG_SHOP_TIMER:
			SpinPlayerShopTimer(c_pData);
			break;

#ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
		case HEADER_DG_SHOP_STOP_OFFLINE:
			StopPlayerShopTimer(c_pData);
			break;
#endif

		case HEADER_DG_SHOP_WITHDRAW_RESULT:
			WithdrawGoldResult(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), (TPacketGoldWithdrawResult*)c_pData);
			break;

		case HEADER_DG_SHOP_CLOSE:
			MyShopClose(c_pData);
			break;

		case HEADER_DG_SHOP_UPDATE_NAME:
			UpdateMyShopName(c_pData);
			break;
#endif

#ifdef ENABLE_MAILBOX
		case HEADER_DG_RESPOND_MAILBOX_LOAD:
			MailBoxRespondLoad(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_RESPOND_MAILBOX_CHECK_NAME:
			MailBoxRespondName(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;

		case HEADER_DG_RESPOND_MAILBOX_UNREAD:
			MailBoxRespondUnreadData(DESC_MANAGER::Instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case HEADER_DG_EXT_BATTLE_PASS_LOAD:
			ExtBattlePassLoad(DESC_MANAGER::instance().FindByHandle(m_dwHandle), c_pData);
			break;
#endif

#ifdef ENABLE_EVENT_MANAGER
		case HEADER_DG_UPDATE_EVENT_STATUS:
			CEventManager::Instance().UpdateEventStatus((SEventTable*)c_pData);
			break;

		case HEADER_DG_EVENT_NOTIFICATION:
			EventNotification(c_pData);
			break;

		case HEADER_DG_RELOAD_EVENT:
			ReloadEvent(c_pData);
			break;
#endif

#ifdef ENABLE_WEATHER_INFO
		case HEADER_DG_WEATHER_INFO:
			ReloadWeatherInfo((TPacketDGWeatherInfo*)c_pData);
			break;
#endif

		default:
			return (-1);
	}

	return 0;
}

bool CInputDB::Process(LPDESC d, const void* orig, int bytes, int& r_iBytesProceed)
{
	const char* c_pData = (const char*)orig;
	uint8_t bHeader, bLastHeader = 0;
	int iSize;
	int iLastPacketLen = 0;

	for (m_iBufferLeft = bytes; m_iBufferLeft > 0;)
	{
		if (m_iBufferLeft < 9)
			return true;

		bHeader = *((uint8_t*)(c_pData)); // 1
		m_dwHandle = *((uint32_t*)(c_pData + 1)); // 4
		iSize = *((uint32_t*)(c_pData + 5)); // 4

		sys_log(1, "DBCLIENT: header %d handle %d size %d bytes %d", bHeader, m_dwHandle, iSize, bytes);

		if (m_iBufferLeft - 9 < iSize)
			return true;

		const char* pRealData = (c_pData + 9);

		if (Analyze(d, bHeader, pRealData) < 0)
		{
			sys_err("in InputDB: UNKNOWN HEADER: %d, LAST HEADER: %d(%d), REMAIN BYTES: %d, DESC: %d",
				bHeader, bLastHeader, iLastPacketLen, m_iBufferLeft, d->GetSocket());

			//printdata((uint8_t*) orig, bytes);
			//d->SetPhase(PHASE_CLOSE);
		}

		c_pData += 9 + iSize;
		m_iBufferLeft -= 9 + iSize;
		r_iBytesProceed += 9 + iSize;

		iLastPacketLen = 9 + iSize;
		bLastHeader = bHeader;
	}

	return true;
}

void CInputDB::AddMonarchMoney(LPDESC d, const char* data)
{
	const int Empire = *(int*)data;
	data += sizeof(int);

	const int Money = *(int*)data;
	data += sizeof(int);

	CMonarch::Instance().AddMoney(Money, Empire);

	uint32_t pid = CMonarch::Instance().GetMonarchPID(Empire);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);

	if (ch)
	{
		if (number(1, 100) > 95)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]%s treasury currently has %u of money."), EMPIRE_NAME(Empire), CMonarch::Instance().GetMoney(Empire));
	}
}

void CInputDB::DecMonarchMoney(LPDESC d, const char* data)
{
	const int Empire = *(int*)data;
	data += sizeof(int);

	const int Money = *(int*)data;
	data += sizeof(int);

	CMonarch::Instance().DecMoney(Money, Empire);

	const uint32_t pid = CMonarch::Instance().GetMonarchPID(Empire);

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
	if (ch)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;784;%s;%d]", EMPIRE_NAME(Empire), CMonarch::Instance().GetMoney(Empire));
}

void CInputDB::TakeMonarchMoney(LPDESC d, const char* data)
{
	const int Empire = *(int*)data;
	data += sizeof(int);

	const int Money = *(int*)data;
	data += sizeof(int);

	if (!CMonarch::Instance().DecMoney(Money, Empire))
	{
		if (!d)
			return;

		if (!d->GetCharacter())
			return;

		LPCHARACTER ch = d->GetCharacter();
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;785]");
	}
}

void CInputDB::ChangeMonarchLord(TPacketChangeMonarchLordACK* info)
{
	char notice[256];
	snprintf(notice, sizeof(notice), LC_TEXT("[1015]The emperor of %s has changed to %s."), EMPIRE_NAME(info->bEmpire), info->szName);
	SendNotice(notice);
}

void CInputDB::UpdateMonarchInfo(TMonarchInfo* info)
{
	CMonarch::Instance().SetMonarchInfo(info);
	sys_log(0, "MONARCH INFO UPDATED");
}

void CInputDB::AddBlockCountryIp(TPacketBlockCountryIp* data)
{
	add_blocked_country_ip(data);
}

void CInputDB::BlockException(TPacketBlockException* data)
{
	block_exception(data);
}

void CInputDB::GuildChangeMaster(TPacketChangeGuildMaster* p)
{
	CGuildManager::Instance().ChangeMaster(p->dwGuildID);
}

void CInputDB::DetailLog(const TPacketNeedLoginLogInfo* info)
{
	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindByPID(info->dwPlayerID);

	if (nullptr != pChar)
	{
		LogManager::Instance().DetailLoginLog(true, pChar);
	}
}

void CInputDB::ItemAwardInformer(TPacketItemAwardInfromer* data)
{
	LPDESC d = DESC_MANAGER::Instance().FindByLoginName(data->login); //login information

	if (d == nullptr)
		return;
	else
	{
		if (d->GetCharacter())
		{
			LPCHARACTER ch = d->GetCharacter();
			if (!ch)
				return;

			ch->SetItemAward_vnum(data->vnum); // It is temporarily stored in ch and processed in the QuestLoad function.
			ch->SetItemAward_cmd(data->command);

			if (d->IsPhase(PHASE_GAME)) //during game phase
			{
				quest::CQuestManager::Instance().ItemInformer(ch->GetPlayerID(), ch->GetItemAward_vnum()); //call questmanager
			}
		}
	}
}

void CInputDB::RespondChannelStatus(LPDESC desc, const char* pcData)
{
	if (!desc)
		return;

	const int nSize = decode_4bytes(pcData);
	pcData += sizeof(nSize);

	constexpr uint8_t bHeader = HEADER_GC_RESPOND_CHANNELSTATUS;
	desc->BufferedPacket(&bHeader, sizeof(uint8_t));
	desc->BufferedPacket(&nSize, sizeof(nSize));
	if (0 < nSize)
		desc->BufferedPacket(pcData, sizeof(TChannelStatus) * nSize);

	constexpr uint8_t bSuccess = 1;
	desc->Packet(&bSuccess, sizeof(bSuccess));
	desc->SetChannelStatusRequested(false);
}

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
void CInputDB::GuildDungeon(const char* c_pData)
{
	const TPacketDGGuildDungeon* sPacket = (TPacketDGGuildDungeon*)c_pData;
	CGuild* pkGuild = CGuildManager::Instance().TouchGuild(sPacket->dwGuildID);
	if (pkGuild)
		pkGuild->RecvDungeon(sPacket->bChannel, sPacket->lMapIndex);
}

void CInputDB::GuildDungeonCD(const char* c_pData)
{
	const TPacketDGGuildDungeonCD* sPacket = (TPacketDGGuildDungeonCD*)c_pData;
	CGuild* pkGuild = CGuildManager::Instance().TouchGuild(sPacket->dwGuildID);
	if (pkGuild)
		pkGuild->RecvDungeonCD(sPacket->dwTime);
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CInputDB::ReceiveShopSaleInfo(const char* c_pData)
{
	const uint32_t pid = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	const TPacketShopSaleResult* p = (TPacketShopSaleResult*)c_pData;
	c_pData += sizeof(TPacketShopSaleResult);

	//The princess is not in this castle
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(pid);
	if (!ch)
	{
		sys_err("Character by pid %u could not be delivered a shop sale info", pid);
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	ch->ShopSellResult(p->itemVnum, p->amount, p->gold, p->cheque, p->pos);
#else
	ch->ShopSellResult(p->itemVnum, p->amount, p->gold, p->pos);
#endif
}

/*
* Receive information about one or more shops and spawn them.
*
* This function will only do something if there's no shop spawned already in the core
* associated with the given PID.
*
* A fake CHARACTER instance is created and destroyed to allow better code reuse in
* other parts of the source.
*/
void CInputDB::SpawnPlayerShops(const char* c_pData)
{
	// Fetch how many shops to spawn and then loop until all are.
	const int spawnAmount = decode_4bytes(c_pData);
	c_pData += sizeof(int);

	// Create a fake char
	LPCHARACTER fake = CHARACTER_MANAGER::Instance().CreateCharacter("fake");

	// Initialize
	LPITEM item;

	/*struct ReadShopTable : TPlayerShopTable {
		int offlineMinutes;
	};

	ReadShopTable* p = (ReadShopTable*)c_pData;*/
	TPlayerShopTableCache* p = (TPlayerShopTableCache*)c_pData;

	for (int i = 0; i < spawnAmount; ++i, ++p)
	{
		if (CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(p->pid)) // Shop already exists
			continue;

		std::string positionList = "";
#ifdef ENABLE_MYSHOP_DECO
		for (int j = 0; j < SHOP_HOST_ITEM_MAX; ++j)
#else
		for (int j = 0; j < SHOP_HOST_ITEM_MAX_NUM; ++j)
#endif
		{
			if (p->items[j].vnum == 0)
				continue;

			if (positionList != "")
				positionList += ", ";

			positionList += std::to_string(p->items[j].display_pos);
		}

		if (positionList.empty())
		{
			sys_err("Empty position list (empty item table?) for pid %lu", p->pid);
			continue;
		}

		fake->SetName(p->playerName);

		// Set the char's position
		const PIXEL_POSITION pos = { p->x, p->y, 0 };
		const long mapIndex = SECTREE_MANAGER::Instance().GetMapIndex(p->x, p->y);

		fake->SetXYZ(pos);
		fake->SetMapIndex(mapIndex);

		// Set the PID (always after creating it so that it doesn't get into the real character PID lists)
		fake->SetPlayerID(p->pid);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		if(p->clevel)
			fake->SetConquerorLevel(p->clevel);
		else
			fake->SetLevel(p->level);
		fake->SetAlignment(p->alignment);
		fake->SetEmpire(p->empire);
		//
		if (p->empire < 1 || p->empire > 3)
			sys_err("Invalid fake shop empire for pid: %u", p->pid);
		//

		CGuild* pGuild = CGuildManager::Instance().FindGuild(p->guild);
		if (pGuild)
			fake->SetGuild(pGuild);
#endif

		// Load the items (synchronously)
		// TODO/OPTIMIZE: Possible optimization if items are loaded someplace else/asynchronously.
		char queryStr[QUERY_MAX_LEN];
		snprintf(queryStr, sizeof(queryStr),
			"SELECT id,window+0,pos,count,vnum,"
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			"transmutation, "
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			"seal_date,"
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
			"grade_elements, attackelement1, attackelement2, attackelement3, elementstype, elementsvalue1, elementsvalue2, elementsvalue3,"
#endif
			"socket0,socket1,socket2,"
#ifdef ENABLE_PROTO_RENEWAL
			"socket3, socket4, socket5,"
#endif
			"attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6"
#ifdef ENABLE_YOHARA_SYSTEM
			",randomtype0,randomvalue0,randomrow0"
			",randomtype1,randomvalue1,randomrow1"
			",randomtype2,randomvalue2,randomrow2"
			", rValue0, rValue1, rValue2, rValue3"
#endif
#ifdef ENABLE_SET_ITEM
			",setItem"
#endif
			" FROM item WHERE owner_id=%d AND window = %d AND pos IN (%s)",
			p->pid, PREMIUM_PRIVATE_SHOP, positionList.c_str());

		auto upMsg(DBManager::Instance().DirectQuery(queryStr));
		SQLResult* res = upMsg->Get();

		if (!res || res->uiNumRows < 1)
		{
			sys_err("Query yielded no results! %s", queryStr);
			continue;
		}

		if (res->uiNumRows > 255)
		{
			sys_err("Max number of shop items exceeded.");
			continue;
		}

		std::vector<TPlayerItem> itemVec;
		itemVec.resize((uint32_t)res->uiNumRows);

		uint8_t count = 0;
		for (size_t k = 0; k < res->uiNumRows; ++k)
		{
			MYSQL_ROW row = mysql_fetch_row(upMsg->Get()->pSQLResult);

			//Store the data
			TPlayerItem& info = itemVec.at(k);

			int cur = 0;
			str_to_number(info.id, row[cur++]);
			str_to_number(info.window, row[cur++]);
			str_to_number(info.pos, row[cur++]);
			str_to_number(info.count, row[cur++]);
			str_to_number(info.vnum, row[cur++]);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			str_to_number(info.dwTransmutationVnum, row[cur++]);
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			str_to_number(info.nSealDate, row[cur++]);
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
			str_to_number(info.grade_element, row[cur++]);
			str_to_number(info.attack_element[0], row[cur++]);
			str_to_number(info.attack_element[1], row[cur++]);
			str_to_number(info.attack_element[2], row[cur++]);
			str_to_number(info.element_type_bonus, row[cur++]);
			str_to_number(info.elements_value_bonus[0], row[cur++]);
			str_to_number(info.elements_value_bonus[1], row[cur++]);
			str_to_number(info.elements_value_bonus[2], row[cur++]);
#endif
			str_to_number(info.alSockets[0], row[cur++]);
			str_to_number(info.alSockets[1], row[cur++]);
			str_to_number(info.alSockets[2], row[cur++]);
#ifdef ENABLE_PROTO_RENEWAL
			str_to_number(info.alSockets[3], row[cur++]);
			str_to_number(info.alSockets[4], row[cur++]);
			str_to_number(info.alSockets[5], row[cur++]);
#endif

			for (int j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
			{
				str_to_number(info.aAttr[j].wType, row[cur++]);
				str_to_number(info.aAttr[j].sValue, row[cur++]);
			}

#ifdef ENABLE_YOHARA_SYSTEM
			for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; s++)
			{
				str_to_number(info.aApplyRandom[s].wType, row[cur++]);
				str_to_number(info.aApplyRandom[s].sValue, row[cur++]);
				str_to_number(info.aApplyRandom[s].row, row[cur++]);
			}

			for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; r++)
			{
				str_to_number(info.alRandomValues[r], row[cur++]);
			}
#endif

#ifdef ENABLE_SET_ITEM
			str_to_number(info.set_value, row[cur++]);
#endif

			info.owner = p->pid;

			// Sanity check
			if (info.window != PREMIUM_PRIVATE_SHOP)
			{
				sys_err("Not shop window while loading shop item %u!", info.id);
				continue;
			}

			// Create the item
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item = ITEM_MANAGER::Instance().CreateItem(info.vnum, info.count, info.id, info.dwTransmutationVnum);
#else
			item = ITEM_MANAGER::Instance().CreateItem(info.vnum, info.count, info.id);
#endif

			if (!item)
			{
				sys_err("cannot create shop item by vnum %u (pid %u id %u)", info.vnum, p->pid, info.id);
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(info.alSockets);
			item->SetAttributes(info.aAttr);
#ifdef ENABLE_YOHARA_SYSTEM
			item->SetRandomAttrs(info.aApplyRandom);
			item->SetRandomDefaultAttrs(info.alRandomValues);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item->SetChangeLookVnum(info.dwTransmutationVnum);
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			item->SetSealDate(info.nSealDate);
#endif
#ifdef ENABLE_REFINE_ELEMENT
			item->SetElement(info.grade_element, info.attack_element, info.element_type_bonus, info.elements_value_bonus);
#endif
#ifdef ENABLE_SET_ITEM
			item->SetItemSetValue(info.set_value);
#endif
			item->SetGrowthPetItemInfo(info.aPetInfo);
			item->AddToCharacter(fake, TItemPos(info.window, info.pos));
			item->SetSkipSave(false);

			++count;
		}

		if (count < 1)
		{
			sys_err("Boot SpawnPlayerShop: Shop %s (pid #%u) had no items / an error!", p->shopName, p->pid);
			continue;
		}

		// Finally, spawn the shop if there are any items in it
		LPCHARACTER shopChar = CHARACTER_MANAGER::Instance().SpawnShop(fake, p->shopName, p->items, count, p->openTime
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
			, p->dwPolyVnum, p->bTitleType, p->isShopDeco
#endif
		);

		if (shopChar && shopChar->GetMyShop())
		{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
			if(p->clevel)
				shopChar->SetConquerorLevel(p->clevel);
			else
				shopChar->SetLevel(p->level);
			shopChar->SetAlignment(p->alignment);
			shopChar->SetEmpire(p->empire);

			CGuild* pGuild = CGuildManager::Instance().FindGuild(p->guild);
			if (pGuild)
				shopChar->SetGuild(pGuild);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
			if (p->offlineMinutesLeft == 0 && !shopChar->IsAffectFlag(AFF_INVISIBILITY))
			{
				shopChar->AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, INFINITE_AFFECT_DURATION, 0, true);
				shopChar->ViewReencode();
			}
#endif
			shopChar->GetMyShop()->SetOfflineMinutes(p->offlineMinutesLeft);
			/*
			shopChar->GetMyShop()->SetOfflineMinutes(p->offlineMinutes);
			*/
		}
	}

	M2_DESTROY_CHARACTER(fake);
}

/*
* Start a 5 minutes timer after which the shop will start to consume offline time.
* Before doing so, a check against P2P logins is ran to prevent a slow packet from
* interfering with normal functioning.
*/
void CInputDB::SpinPlayerShopTimer(const char* c_pData)
{
	const uint32_t pid = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	const int offMinutes = *(int*)c_pData;
	c_pData += sizeof(int);

	if (P2P_MANAGER::Instance().FindByPID(pid)) // We found the player in p2p, packet probably delayed!
		return;

	LPCHARACTER owner = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!owner)
	{
		sys_err("No shop to spin timer on (PID %u)", pid);
		return;
	}

	// Set offline minutes based on the activity value sent by DB
	if (!owner->GetMyShop())
		return;

	owner->GetMyShop()->SetOfflineMinutes(offMinutes);
	owner->StartShopOfflineEvent();
}

#ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
/*
* Cancels the shop timer in the event that there is one running
*/
void CInputDB::StopPlayerShopTimer(const char* c_pData)
{
	uint32_t pid = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	LPCHARACTER owner = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!owner)
	{
		sys_err("No shop to stop for (PID %u)", pid);
		return;
	}

	if (!owner->GetMyShop())
		return;

	owner->CancelShopOfflineEvent();
}
#endif

void CInputDB::WithdrawGoldResult(LPDESC desc, TPacketGoldWithdrawResult* p)
{
	LPCHARACTER ch = desc ? desc->GetCharacter() : nullptr;

	if (!desc || !ch)
	{

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		sys_err("Desc%s gold: %u, cheque %u, pid: %u", !desc ? " did not exist" : "->GetCharacter() was nullptr", p->goldAmount, p->chequeAmount, p->shopPid);
#else
		sys_err("Desc%s gold: %u, pid: %u", !desc ? " did not exist" : "->GetCharacter() was nullptr", p->goldAmount, p->shopPid);
#endif

		if (p->success)
		{
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			sys_log(0, "Bouncing rollback for shop withdraw transaction worth %u gold %u cheque on shop %u back to db", p->goldAmount, p->chequeAmount, p->shopPid);
#else
			sys_log(0, "Bouncing rollback for shop withdraw transaction worth %u gold on shop %u back to db", p->goldAmount, p->shopPid);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
#else
			db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t));
#endif
			constexpr uint8_t subheader = SHOP_SUBHEADER_GD_WITHDRAW_ROLLBACK;
			db_clientdesc->Packet(&subheader, sizeof(uint8_t));
			db_clientdesc->Packet(&p->shopPid, sizeof(uint32_t));
			db_clientdesc->Packet(&p->goldAmount, sizeof(uint32_t));
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			db_clientdesc->Packet(&p->chequeAmount, sizeof(uint32_t));
#endif
		}

		return;
	}

	if (!p->success)
	{
		// Gold removal did not go right
		sys_err("Unknown error withdrawing an amount of %u gold %u cheque for #%u", p->goldAmount, ch->GetPlayerID());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UNKNOWN_ERROR_WITHDRAWING_FROM_SHOP"));
	}
	else
	{
		// Log
		char buf[512];
		snprintf(buf, sizeof(buf), "%d %d %d %d %d %d",
			// Current Gold / Cheque
			ch->GetGold(), ch->GetCheque(),
			// Current Gold / Cheque in Shop
			ch->GetShopGoldStash(), ch->GetShopChequeStash(),
			// Gold / Cheque Withdraw
			p->goldAmount, p->chequeAmount);

		LogManager::Instance().CharLog(ch, 0, "SHOP_WITHDRAW", buf);

		//All worked fine. Increase player's gold (reduce the gold stash) + tell them
		ch->AlterShopGoldStash(-p->goldAmount);
		ch->PointChange(POINT_GOLD, p->goldAmount);
		if (p->goldAmount > 0)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", p->goldAmount);

#ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
		ch->AlterShopChequeStash(-p->chequeAmount);
		ch->PointChange(POINT_CHEQUE, p->chequeAmount);
		if (p->chequeAmount > 0)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;337;%d]", p->chequeAmount);
#endif

		//Sync gold stash on client
		TPacketGCShopStashSync stash{};
		stash.bHeader = HEADER_GC_SYNC_SHOP_STASH;
		stash.value = ch->GetShopGoldStash();
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		stash.cheque = ch->GetShopChequeStash();
#endif
		ch->GetDesc()->Packet(&stash, sizeof(TPacketGCShopStashSync));
	}
}

void CInputDB::MyShopClose(const char* c_pData)
{
	const uint32_t pid = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	const bool isDeleteShop = *(bool*)c_pData;
	c_pData += sizeof(bool);

	LPCHARACTER shopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!shopChar)
		return;

	LPSHOP shop = shopChar->GetMyShop();
	if (shop)
	{
		if (!isDeleteShop)
		{
			shop->SetClosed(true);
			if (shopChar->IsShopOfflineEventRunning())
				shop->SaveOffline();
			shop->Save();
		}
		else
		{
			// We are deleting - avoid flushes
			shopChar->SetShopOfflineEventRunning(false);
		}

		M2_DESTROY_CHARACTER(shopChar);
	}
}

void CInputDB::UpdateMyShopName(const char* c_pData)
{
	const uint32_t pid = *(uint32_t*)c_pData;
	c_pData += sizeof(uint32_t);

	char shopName[SHOP_SIGN_MAX_LEN + 1];
	strlcpy(shopName, c_pData, sizeof(shopName));
	c_pData += sizeof(shopName);

	LPCHARACTER shopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!shopChar)
		return;

	shopChar->SetShopSign(shopName);
	shopChar->GetMyShop()->SetShopSign(shopName);
	shopChar->GetMyShop()->SetNextRenamePulse(thecore_pulse() + PASSES_PER_SEC(60 * 60));

	TPacketGCShopSign p{};
	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = shopChar->GetVID();
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	p.bType = shopChar->GetMyShop()->GetTitleType();
#endif
	strlcpy(p.szSign, shopName, sizeof(p.szSign));
	shopChar->PacketAround(&p, sizeof(TPacketGCShopSign));

	const LPCHARACTER& owner = CHARACTER_MANAGER::Instance().FindByPID(pid); // Try on the fly update if possible
	if (!owner)
		return;

	LPDESC d = owner->GetDesc();
	if (!d)
		return;

	TPacketPlayerShopSign namePack{};
	namePack.header = HEADER_GC_MY_SHOP_SIGN;
	strlcpy(namePack.sign, shopName, sizeof(namePack.sign));

	d->Packet(&namePack, sizeof(TPacketPlayerShopSign));
}
#endif

#ifdef ENABLE_EVENT_MANAGER
void CInputDB::EventNotification(const char* c_pData)
{
	const TPacketSetEventFlag* p = (TPacketSetEventFlag*)c_pData;
	quest::CQuestManager::Instance().SetEventFlag(p->szFlagName, p->lValue);

	if (p->lValue)
		SendEventBeginNotification();
	else
		SendEventEndNotification();
}

void CInputDB::ReloadEvent(const char* c_pData)
{
	const uint16_t wSize = decode_2bytes(c_pData);
	c_pData += 2;
	sys_log(0, "RELOAD: Event: %d", wSize);

	if (wSize)
	{
		CEventManager::Instance().Initialize((TEventTable*)c_pData, wSize, true);
		c_pData += wSize * sizeof(TEventTable);
	}
}
#endif

#ifdef ENABLE_WEATHER_INFO
void CInputDB::ReloadWeatherInfo(TPacketDGWeatherInfo* p)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();

	for (itertype(c_ref_set) it = c_ref_set.begin(); it != c_ref_set.end(); ++it)
	{
		LPCHARACTER ch = (*it)->GetCharacter();

		if (!ch)
			continue;

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		if (quest::CQuestManager::Instance().GetEventFlag("xmas_soul") != 0 || quest::CQuestManager::Instance().GetEventFlag("e_late_summer_event") != 0)
			return;
#endif

		uint8_t bDayMode = p->bDayMode;
		uint8_t rainActive = 0;

		if (number(1, 100) <= WEATHER_RAIN_PERC && !byRainState)
			byRainState = 1;

		if (byRainState)
		{
			byRainTimer += 1;
			rainActive = 1;
			if (bDayMode == DayMode::DAY && p->bSeason)
				bDayMode = DayMode::RAIN;
		}
			
		if (byRainState && rainActive && (byRainTimer > RAIN_SNOW_ACTIVE_TIME))
		{
			byRainState = 0;
			rainActive = 0;
		}

		ch->ChatPacket(CHAT_TYPE_COMMAND, "WeatherRefresh %d %d %d", bDayMode, p->bSeason, rainActive);
	}
}
#endif
