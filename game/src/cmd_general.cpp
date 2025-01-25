#include "stdafx.h"
#ifdef __FreeBSD__
#include <md5.h>
#else
#include "../../libthecore/include/xmd5.h"
#endif

#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "shop.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "dev_log.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "threeway_war.h"
#include "log.h"
#include "../../common/VnumHelper.h"
#include "gm.h"
#ifdef ENABLE_12ZI
#	include "zodiac_temple.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_RANKING_SYSTEM
#	include "ranking_system.h"
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#	include "LocaleNotice.hpp"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif
#ifdef ENABLE_DAWNMIST_DUNGEON
# include "DawnMistDungeon.h"
#endif

extern bool view_equip_for_all;
extern int view_equip_sp_cost;

ACMD(do_user_horse_ride)
{
	if (!ch)
		return;

	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (!ch->IsHorseRiding())
	{
		// Riding a vehicle other than a horse
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1054]");
			return;
		}

		if (ch->GetHorse() == nullptr)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;501]");
			return;
		}

		if (ch->GetQuestFlag("HORSE.CHECKER") && get_global_time() < ch->GetQuestFlag("HORSE.CHECKER")) {	//@custom017
			ch->ChatPacket(CHAT_TYPE_INFO, "ride_to_do_that_waite_%d_seconds", (ch->GetQuestFlag("HORSE.CHECKER") - get_global_time()) % 180);
			return;
		}

		ch->SetQuestFlag("HORSE.CHECKER", get_global_time() + ride_seconds_to_ride_again);	//@custom017
		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (!ch)
		return;

	if (ch->GetHorse() != nullptr)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;502]");
	}
	else if (ch->IsHorseRiding())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;504]");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;501]");
	}
}

ACMD(do_user_horse_feed)
{
	if (!ch)
		return;

	// You cannot feed horses while the private shop is open.
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == nullptr)
	{
		if (!ch->IsHorseRiding())
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;501]");
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;505]");
		return;
	}

	const uint32_t dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;506;[IN;%d];%s]", dwFood, "");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;507;[IN;%d]]", dwFood);
	}
}

#define MAX_REASON_LEN 128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int subcmd;
	int left_second;
	char szReason[MAX_REASON_LEN];

	TimedEventInfo()
		: ch()
		, subcmd(0)
		, left_second(0)
	{
		::memset(szReason, 0, MAX_REASON_LEN);
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d && d->GetCharacter())
		{
#ifdef FLUSH_AT_SHUTDOWN
			d->GetCharacter()->SaveReal();
			uint32_t pid = d->GetCharacter()->GetPlayerID();
			db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
			db_clientdesc->Packet(&pid, sizeof(uint32_t));
#endif
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (!d)
			return;

		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int seconds;

	shutdown_event_data()
		: seconds(0)
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>(event->info);

	if (info == nullptr)
	{
		sys_err("shutdown_event> <Factor> Null pointer");
		return 0;
	}

	int* pSec = &(info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (-- * pSec == -10)
		{
			const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::Instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET& c_set_desc = DESC_MANAGER::Instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--* pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), "[LS;508;%d]", *pSec);
		SendNotice(buf);

		--* pSec;
		return passes_per_sec;
	}
}

void Shutdown(int iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::Instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), "[LS;509;%d]", iSec);
	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	//@fixme513
	/*if (nullptr == ch)
	{
		sys_err("Accept shutdown command from %s.", ch->GetName());
	}*/

	if (!ch)
		return;

	sys_err("Accept shutdown command from %s.", ch->GetName());
	//@end_fixme513

	TPacketGGShutdown p{};
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo* info = dynamic_cast<TimedEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("timed_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) // <Factor>
		return 0;

	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = nullptr;

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			case SCMD_QUIT:
			case SCMD_PHASE_SELECT:
				{
					TPacketNeedLoginLogInfo acc_info{};
					acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;
					db_clientdesc->DBPacket(HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info));
					LogManager::Instance().DetailLoginLog(false, ch);
				}
				break;

			default:
				break;
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d) {
					//d->SetCloseReason("USER_LOGOUT");	//@fixme000
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;510;%d]", info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	if (!ch)
		return;

	/* RECALL_DELAY
	if (ch->m_pkRecallEvent != nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;511]");
		event_cancel(&ch->m_pkRecallEvent);
		return;
	}
	// END_OF_RECALL_DELAY */

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;511]");
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;512]");
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;513]");
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;515]");
			break;

		default:
			break;
	}

	constexpr int nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()) &&
		(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch = ch;
				info->subcmd = subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent = event_create(timed_event, info, 1);
			}
			break;

		default:
			break;
	}
}

ACMD(do_mount)
{
	/*
	char arg1[256];
	struct action_mount_param param;

	// if already riding
	if (ch->GetMountingChr())
	{
		char arg2[256];
		two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

		if (!*arg1 || !*arg2)
			return;

		param.x = atoi(arg1);
		param.y = atoi(arg2);
		param.vid = ch->GetMountingChr()->GetVID();
		param.is_unmount = true;

		float distance = DISTANCE_SQRT(param.x - (uint32_t) ch->GetX(), param.y - (uint32_t) ch->GetY());

		if (distance > 600.0f)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Get a little closer and get off."));
			return;
		}

		action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
		return;
	}

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(atoi(arg1));

	if (!tch->IsNPC() || !tch->IsMountable())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't ride there."));
		return;
	}

	float distance = DISTANCE_SQRT(tch->GetX() - ch->GetX(), tch->GetY() - ch->GetY());

	if (distance > 600.0f)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Come a little closer and ride."));
		return;
	}

	param.vid = tch->GetVID();
	param.is_unmount = false;

	action_enqueue(ch, ACTION_TYPE_MOUNT, &param, 0.0f, true);
	*/
}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(static_cast<float>(atof(arg1)));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
#ifdef ENABLE_SUNG_MAHI_TOWER
	LPDUNGEON dungeonInstance = ch->GetDungeon();
	if (dungeonInstance && ch->IsSungMahiDungeon(ch->GetMapIndex()) && dungeonInstance->GetFlag("isSungMahiDungeon") > 0)
	{
		ch->WarpSet((9216 * 100), (6144 * 100));
		return;
	}
#endif

	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (nullptr == ch->m_pkDeadEvent)
		return;

	int eFRS_HERESEC = 170;
	int eFRS_TOWNSEC = 173;

	if (ch->IsGM())	//ENABLE_INSTANT_RESTART_HERE
		eFRS_HERESEC += 10;

#ifdef ENABLE_BATTLE_FIELD
	eFRS_HERESEC += 10;
	eFRS_TOWNSEC += 6;
#endif

	const int iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);
#ifdef ENABLE_BATTLE_FIELD
	const bool isBattleMap = CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex());
	const int toWaitSeconds = isBattleMap ? ch->GetBattleDeadLimit() : g_nPortalLimitTime;
#endif

	if (!ch->IsGM())
	{
		if (subcmd != SCMD_RESTART_TOWN &&
#ifdef ENABLE_BATTLE_FIELD
			subcmd != SCMD_RESTART_BATTLE &&
#endif
			(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
		{
			if (!test_server)
			{
				if (ch->IsHack())
				{
					//If it is a holy land map, it is not checked.
					if (false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
					{
#ifdef ENABLE_BATTLE_FIELD
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - (180 - toWaitSeconds));
#else
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - (180 - g_nPortalLimitTime));
#endif
						return;
					}
				}

#ifdef ENABLE_BATTLE_FIELD
				if (ch->GetGMLevel() < GM_IMPLEMENTOR && iTimeToDead > (eFRS_HERESEC - toWaitSeconds))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - (eFRS_HERESEC - toWaitSeconds));
					return;
			}
#else
				if (ch->GetGMLevel() < GM_IMPLEMENTOR && iTimeToDead > eFRS_HERESEC) {	//@fixme408
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - eFRS_HERESEC);
					return;
				}
#endif
			}
		}

		//PREVENT_HACK
		//DESC : It can be used for bugs using the portal after the warehouse and exchange window.
		// add cooldown

		if (subcmd == SCMD_RESTART_TOWN)
		{
			if (ch->IsHack())
			{
				//It is not checked on the guild map and the holy land map.
				if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
					false == CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
				{
#ifdef ENABLE_BATTLE_FIELD
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - (180 - toWaitSeconds));
#else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - (180 - g_nPortalLimitTime));
#endif
					return;
				}
			}

#ifdef ENABLE_BATTLE_FIELD
			if (ch->GetGMLevel() < GM_IMPLEMENTOR && iTimeToDead > (eFRS_TOWNSEC - toWaitSeconds))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;516;%d]", iTimeToDead - (eFRS_TOWNSEC - toWaitSeconds));
				return;
			}
#else
			if (ch->GetGMLevel() < GM_IMPLEMENTOR && iTimeToDead > eFRS_TOWNSEC) {	//@fixme408
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;519;%d]", iTimeToDead - eFRS_TOWNSEC);
				return;
			}
#endif
		}

#ifdef ENABLE_BATTLE_FIELD
		if (subcmd == SCMD_RESTART_BATTLE)
		{
			if (!isBattleMap)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can use instant restart just in battle map."));
				return;
			}

			if (ch->CountSpecifyItem(50289) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;100]");
				return;
			}
		}
#endif
		//END_PREVENT_HACK

#ifdef ENABLE_12ZI
		// For check how many Prism you need.
		if (subcmd == SCMD_RESTART_HERE)
		{
			//if (ch->GetZodiac())
			if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
			{
				sys_log(0, "do_restart: restart here zodiac");

				uint8_t bNeedPrism;
				if (ch->GetQuestFlag("12zi_temple.IsDead") == 1 || ch->GetQuestFlag("12zi_temple.PrismNeed") > 0)
				{
					if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 1)
						bNeedPrism = 1;
					else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 2)
						bNeedPrism = 2;
					else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 3)
						bNeedPrism = 4;
					else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 4)
						bNeedPrism = 8;
					else if (ch->GetQuestFlag("12zi_temple.PrismNeed") >= 5)
						bNeedPrism = 10;
					else
						bNeedPrism = ch->GetQuestFlag("12zi_temple.PrismNeed");
				}
				else
				{
					if (ch->GetDeadCount() == 3)
						bNeedPrism = 4;
					else if (ch->GetDeadCount() == 4)
						bNeedPrism = 8;
					else if (ch->GetDeadCount() >= 5)
						bNeedPrism = 10;
					else
						bNeedPrism = ch->GetDeadCount();
				}

				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %u %u", (uint32_t)ch->GetVID(), bNeedPrism);
				return;
			}
		}
#endif
	}

	ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	ch->GetDesc()->SetPhase(PHASE_GAME);
	ch->SetPosition(POS_STANDING);
#ifdef ENABLE_AFTERDEATH_SHIELD
#	ifdef AFTERDEATH_SHIELD_ONLY_WARMAP
	if (ch->GetWarMap())
#	endif
	{
		ch->SetShieldAffect();
		ch->StartShieldCountdownEvent(ch, AFTERDEATH_SHIELD_DURATION);
	}
#endif
	ch->StartRecoveryEvent();

	if (1 == quest::CQuestManager::Instance().GetEventFlag("threeway_war"))
	{
		if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE)
		{
#ifdef ENABLE_12ZI
			if (subcmd == SCMD_RESTART_HERE)
			{
				if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
				{
					sys_log(0, "do_restart: restart here zodiac");

					uint8_t bNeedPrism;
					if (ch->GetQuestFlag("12zi_temple.IsDead") == 1 || ch->GetQuestFlag("12zi_temple.PrismNeed") > 0)
					{
						if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 1)
							bNeedPrism = 1;
						else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 2)
							bNeedPrism = 2;
						else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 3)
							bNeedPrism = 4;
						else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 4)
							bNeedPrism = 8;
						else if (ch->GetQuestFlag("12zi_temple.PrismNeed") >= 5)
							bNeedPrism = 10;
						else
							bNeedPrism = ch->GetQuestFlag("12zi_temple.PrismNeed");
					}
					else
					{
						if (ch->GetDeadCount() == 3)
							bNeedPrism = 4;
						else if (ch->GetDeadCount() == 4)
							bNeedPrism = 8;
						else if (ch->GetDeadCount() >= 5)
							bNeedPrism = 10;
						else
							bNeedPrism = ch->GetDeadCount();
					}

					ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %u %u", (uint32_t)ch->GetVID(), bNeedPrism);
					return;
				}
			}
#endif

			if (CThreeWayWar::Instance().IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
				!CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->GoHome();

				ch->ReviveInvisible(5);
#ifdef ENABLE_PET_SYSTEM
				ch->CheckPet();
#endif
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				return;
			}

			// holy land
			if (CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				if (CThreeWayWar::Instance().GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("성지에서 부활 기회를 모두 잃었습니다! 마을로 이동합니다!"));
					ch->GoHome();
				}
				else
				{
					ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
				}

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(5);
#ifdef ENABLE_PET_SYSTEM
				ch->CheckPet();
#endif
				return;
			}
		}
#ifdef ENABLE_AUTO_RESTART_EVENT
		if (subcmd == SCMD_RESTART_AUTOHUNT)
		{
			return;
		}
#endif
	}
	//END_FORKED_LOAD

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap* pMap = ch->GetWarMap();
		const uint32_t dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
				{
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::Instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
#ifdef ENABLE_PET_SYSTEM
					ch->CheckPet();
#endif
				}
				break;

				case SCMD_RESTART_HERE:
				{
#ifdef ENABLE_12ZI
					if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
					{
						sys_log(0, "do_restart: restart here zodiac");

						uint8_t bNeedPrism;
						if (ch->GetQuestFlag("12zi_temple.IsDead") == 1 || ch->GetQuestFlag("12zi_temple.PrismNeed") > 0)
						{
							if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 1)
								bNeedPrism = 1;
							else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 2)
								bNeedPrism = 2;
							else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 3)
								bNeedPrism = 4;
							else if (ch->GetQuestFlag("12zi_temple.PrismNeed") == 4)
								bNeedPrism = 8;
							else if (ch->GetQuestFlag("12zi_temple.PrismNeed") >= 5)
								bNeedPrism = 10;
							else
								bNeedPrism = ch->GetQuestFlag("12zi_temple.PrismNeed");
						}
						else
						{
							if (ch->GetDeadCount() == 3)
								bNeedPrism = 4;
							else if (ch->GetDeadCount() == 4)
								bNeedPrism = 8;
							else if (ch->GetDeadCount() >= 5)
								bNeedPrism = 10;
							else
								bNeedPrism = ch->GetDeadCount();
						}

						ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %u %u", (uint32_t)ch->GetVID(), bNeedPrism);
						return;
					}
#endif
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
#ifdef ENABLE_PET_SYSTEM
					ch->CheckPet();
#endif
				}
				break;

				default:
					break;
			}

			return;
		}
	}

#ifdef ENABLE_QUEEN_NETHIS
	if (SnakeLair::CSnk::instance().IsSnakeMap(ch->GetMapIndex()))
	{
		switch (subcmd)
		{
			case SCMD_RESTART_TOWN:
				sys_log(0, "do_restart: restart town");
				//PIXEL_POSITION pos;

				ch->RestartAtSamePos();

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(3);
				break;

			case SCMD_RESTART_HERE:
				sys_log(0, "do_restart: restart here");
				ch->RestartAtSamePos();
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(3);
				break;

			default:
				break;
		}

		return;
	}
#endif

	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
		{
			sys_log(0, "do_restart: restart town");
#ifdef ENABLE_BATTLE_FIELD
			if (isBattleMap)
			{
				CBattleField::Instance().RestartAtRandomPos(ch);
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_DEVIL_TOWER
			if (ch->IsDevilTowerDungeon(ch->GetMapIndex()))
			{
				//const auto level = ch->GetDungeonForce()->GetFlag("level");
				LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
				if (!pDungeon)
				{
					ch->WarpSet(590800, 111200, 65);
					return;
				}

				const auto level = pDungeon->GetFlag("level");
				const int floorPositions[10][2] =
				{
					{0, 0}, // 0th
					{0, 0}, // 1st
					{2174, 7040}, // 2nd
					{2182, 6803}, // 3rd
					{2417, 7285}, // 4th
					{2417, 7057}, // 5th
					{2422, 6823}, // 6th
					{2638, 7294}, // 7th
					{2638, 7059}, // 8th
					{2638, 6811}, // 9th (boss)
				};

				if (level <= 1)
					ch->WarpSet(590800, 111200, 65);
				else
					ch->WarpSet(floorPositions[level][0] * 100, floorPositions[level][1] * 100, ch->GetMapIndex());

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_DEVIL_CATACOMBS
			if (ch->IsDevilCatacombsDungeon(ch->GetMapIndex()))
			{
				//const auto level = ch->GetDungeonForce()->GetFlag("level");
				LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
				if (!pDungeon)
				{
					ch->WarpSet(592300, 100200, 65);
					return;
				}

				const auto level = pDungeon->GetFlag("level");
				const int floorPositions[8][2] =
				{
					{0, 0}, // 0th
					{3145, 12095}, // 1st
					{3622, 12077}, // 2nd
					{4450, 12281}, // 3rd
					{3918, 12930}, // 4th (deleted floor)
					{3918, 12930}, // 5th
					{4434, 12698}, // 6th
					{3145, 13188}, // 7th
				};

				if (level == 0)
					ch->WarpSet(592300, 100200, 65);
				else
					ch->WarpSet(floorPositions[level][0] * 100, floorPositions[level][1] * 100, ch->GetMapIndex());

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_SPIDER_DUNGEON
			if (ch->IsSpiderDungeon(ch->GetMapIndex()))
			{
				PIXEL_POSITION pos;
				if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
					ch->WarpSet(88100, 614800, ch->GetMapIndex());
				else
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_FLAME_DUNGEON
			if (ch->IsFlameDungeon(ch->GetMapIndex()))
			{
				//const auto level = ch->GetDungeonForce()->GetFlag("level");
				LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
				if (!pDungeon)
				{
					ch->WarpSet(614400, 707000, 62);
					return;
				}

				const auto level = pDungeon->GetFlag("level");
				if (level == 17)
					ch->WarpSet(810900, 686700, ch->GetMapIndex());
				else
					ch->WarpSet(777600, 651600, ch->GetMapIndex());

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_SNOW_DUNGEON
			if (ch->IsSnowDungeon(ch->GetMapIndex()))
			{
				//const auto level = ch->GetDungeonForce()->GetFlag("level");
				LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
				if (!pDungeon)
				{
					ch->WarpSet(431800, 164400, 61);
					return;
				}

				auto level = pDungeon->GetFlag("level");
				const int floorPositions[11][2] =
				{
					{0, 0}, // 0th
					{5293, 1816}, // 1st
					{5540, 1803}, // 2nd
					{5880, 1809}, // 3rd
					{5292, 2067}, // 4th
					{5539, 2072}, // 5th
					{5865, 2074}, // 6th
					{5423, 2251}, // 7th
					{5691, 2238}, // 8th
					{5969, 2229}, // 9th
					{6047, 1927}, // boss
				};
				if (level == 42)
					level = 4;

				if (level == 0)
					ch->WarpSet(431800, 164400, 61);
				else
					ch->WarpSet(floorPositions[level][0] * 100, floorPositions[level][1] * 100, ch->GetMapIndex());

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_DAWNMIST_DUNGEON
			if (CDawnMistDungeon::Instance().IsInDungeonInstance(ch->GetMapIndex()))
			{
				PIXEL_POSITION pos;
				if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
					ch->WarpSet(782500, 1501700, ch->GetMapIndex());
				else
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
			if (ch->GetMeleyLair())
			{
				PIXEL_POSITION pos;
				if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
					ch->WarpSet(321000, 1526900, ch->GetMapIndex());
				else
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_DEFENSE_WAVE
			if (ch->IsDefenseWaveDungeon(ch->GetMapIndex()))
			{
				const LPDUNGEON& pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
				if (!pDungeon)
				{
					ch->WarpSet(1107700, 1782100, 301);
					return;
				}

				ch->WarpSet(166500, 522100, ch->GetMapIndex());

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

#ifdef ENABLE_RESTART_CITY_POS
			if (ch->GetDungeon())
			{
				PIXEL_POSITION pos;
				if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
					ch->WarpSet(pos.x, pos.y, ch->GetMapIndex());
				else
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(1);
				return;
			}
#endif

			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(1);
		}
		break;

		case SCMD_RESTART_HERE:
		{
#ifdef ENABLE_12ZI
			if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
			{
				sys_log(0, "do_restart: restart here zodiac");
				return;
			}
#endif

			sys_log(0, "do_restart: restart here");
			ch->RestartAtSamePos();
			//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
#ifdef ENABLE_BATTLE_FIELD
			if (isBattleMap)
			{
				ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP());
				ch->DeathPenalty(1);
			}
			else
#endif
			{
				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(0);
			}
			ch->ReviveInvisible(5);
#ifdef ENABLE_PET_SYSTEM
			ch->CheckPet();
#endif
		}
		break;

#ifdef ENABLE_BATTLE_FIELD
		case SCMD_RESTART_BATTLE:
		{
			sys_log(0, "do_restart: restart battle");
			ch->RemoveSpecifyItem(50289, 1);
			ch->RestartAtSamePos();
			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			ch->DeathPenalty(1);
			ch->ReviveInvisible(5);
#	ifdef ENABLE_PET_SYSTEM
			ch->CheckPet();
#	endif
		}
		break;
#endif
#ifdef ENABLE_AUTO_RESTART_EVENT
		case SCMD_RESTART_AUTOHUNT:
		{
			/*if (!ch->oto_av_restart)
			{
				return;
			}*/
/*#ifdef ENABLE_12ZI
			if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
			{
				int REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
				int vid = (int)ch->GetVID();
				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %d %d", vid, REVIVE_PRICE);
				return;
			}
#endif*/
			sys_log(1, "do_restart: restart here");
			ch->RestartAtSamePos();
			//ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY());
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(0);
			ch->ReviveInvisible(5);
		}
		break;
#endif

		default:
			break;
	}

#ifdef ENABLE_ANTI_EXP_RING
	for (int i = 0; i < INVENTORY_MAX_NUM; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item && (item->GetVnum() == ITEM_AUTO_EXP_RING_S || item->GetVnum() == ITEM_AUTO_EXP_RING_M || item->GetVnum() == ITEM_AUTO_EXP_RING_G))
		{
			if (item->GetSocket(0))
			{
				EAffectTypes type = AFFECT_EXPRING;
				int bonus = 0;

				if (item->GetVnum() == ITEM_AUTO_EXP_RING_S)
					bonus = 150;
				else if (item->GetVnum() == ITEM_AUTO_EXP_RING_M)
					bonus = 151;
				else if (item->GetVnum() == ITEM_AUTO_EXP_RING_G)
					bonus = 152;

				CAffect* pAffect = ch->FindAffect(AFFECT_EXPRING);
				if (nullptr == pAffect)
				{
					ch->AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);
					item->Lock(true);
					item->SetSocket(0, true);
					break;
				}
			}
		}
	}
#endif
}

#define MAX_STAT g_iStatusPointSetMaxValue

ACMD(do_stat_reset)
{
	if (ch)
		ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
#ifdef ENABLE_SET_STATES
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
#else
	one_argument(argument, arg1, sizeof(arg1));
#endif

	if (!*arg1)
		return;

#ifdef ENABLE_SET_STATES
	int iStatUp = 1;
	if (*arg2)
		iStatUp = atoi(arg2);
#endif

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
		return;
	}

	uint16_t REAL_POINT = POINT_STAT;

#ifdef ENABLE_YOHARA_SYSTEM
	if (!strcmp(arg1, "sstr") || !strcmp(arg1, "shp") || !strcmp(arg1, "smove") || !strcmp(arg1, "simmune"))
		REAL_POINT = POINT_CONQUEROR_POINT;
#endif

#ifdef ENABLE_SET_STATES
	if (ch->GetPoint(REAL_POINT) < iStatUp)
		iStatUp = ch->GetPoint(REAL_POINT);
#else
	if (ch->GetPoint(REAL_POINT) <= 0)
		return;
#endif

	uint16_t idx = 0; //@fixme532

	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
#ifdef ENABLE_YOHARA_SYSTEM
	else if (!strcmp(arg1, "sstr"))
		idx = POINT_SUNGMA_STR;
	else if (!strcmp(arg1, "shp"))
		idx = POINT_SUNGMA_HP;
	else if (!strcmp(arg1, "smove"))
		idx = POINT_SUNGMA_MOVE;
	else if (!strcmp(arg1, "simmune"))
		idx = POINT_SUNGMA_IMMUNE;
#endif
	else
		return;

#ifdef ENABLE_SET_STATES
	if ((ch->GetRealPoint(idx) + iStatUp) > MAX_STAT)
		iStatUp = MAX_STAT - ch->GetRealPoint(idx);

	if (iStatUp < 1)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + iStatUp);
	ch->SetPoint(idx, ch->GetPoint(idx) + iStatUp);
#else
	// ch->ChatPacket(CHAT_TYPE_INFO, "%s GRP(%d) idx(%u), MAX_STAT(%d), expr(%d)", __FUNCTION__, ch->GetRealPoint(idx), idx, MAX_STAT, ch->GetRealPoint(idx) >= MAX_STAT);
	if (ch->GetRealPoint(idx) >= MAX_STAT)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + 1);
	ch->SetPoint(idx, ch->GetPoint(idx) + 1);
#endif

	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

#ifdef ENABLE_YOHARA_SYSTEM
	if (idx == POINT_SUNGMA_STR)
	{
		ch->PointChange(POINT_SUNGMA_STR, 0);
		ch->PointChange(POINT_ST, 0);
	}
	else if (idx == POINT_SUNGMA_HP)
	{
		ch->PointChange(POINT_SUNGMA_HP, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_SUNGMA_MOVE)
	{
		ch->PointChange(POINT_SUNGMA_MOVE, 0);
		ch->PointChange(POINT_MOV_SPEED, 0);
	}
	else if (idx == POINT_SUNGMA_IMMUNE)
		ch->PointChange(POINT_SUNGMA_IMMUNE, 0);
#endif

#ifdef ENABLE_SET_STATES
	ch->PointChange(REAL_POINT, -iStatUp);
#else
	ch->PointChange(REAL_POINT, -1);
#endif
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	if (!ch)
		return;

	if (ch->GetArena() != nullptr || CArenaManager::Instance().IsArenaMap(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::Instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	if (pkVictim->GetArena() != nullptr)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, "[LS;522]");
		return;
	}

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;118]");
		return;
	}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (ch->GetMapIndex() == MAP_PRIVATSHOP)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1763]");
		return;
	}
#endif


#ifdef ENABLE_MESSENGER_BLOCK
	if (MessengerManager::Instance().IsBlocked(ch->GetName(), pkVictim->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), pkVictim->GetName());
		return;
	}
	else if (MessengerManager::Instance().IsBlocked(pkVictim->GetName(), ch->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), pkVictim->GetName());
		return;
	}
#endif


	if (pkVictim->GetArena() != nullptr)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, "[LS;522]");
		return;
	}

	CPVPManager::Instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;523]");
		return;
	}

	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	const TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;524]");
	}
}

ACMD(do_skillup)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch (vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

#ifdef ENABLE_78TH_SKILL
			case SKILL_ANTI_PALBANG:
			case SKILL_ANTI_AMSEOP:
			case SKILL_ANTI_SWAERYUNG:
			case SKILL_ANTI_YONGBI:
			case SKILL_ANTI_GIGONGCHAM:
			case SKILL_ANTI_HWAJO:
			case SKILL_ANTI_MARYUNG:
			case SKILL_ANTI_BYEURAK:
#if defined(ENABLE_78TH_SKILL) && defined(ENABLE_WOLFMAN_CHARACTER)
			case SKILL_ANTI_SALPOONG:
#endif
			case SKILL_HELP_PALBANG:
			case SKILL_HELP_AMSEOP:
			case SKILL_HELP_SWAERYUNG:
			case SKILL_HELP_YONGBI:
			case SKILL_HELP_GIGONGCHAM:
			case SKILL_HELP_HWAJO:
			case SKILL_HELP_MARYUNG:
			case SKILL_HELP_BYEURAK:
#if defined(ENABLE_78TH_SKILL) && defined(ENABLE_WOLFMAN_CHARACTER)
			case SKILL_HELP_SALPOONG:
#endif
#endif
				ch->SkillLevelUp(vnum);
				break;

			default:
				break;
		}
	}
}

//
// @version 05/06/20 Bang2ni - Command handling Delegate to CHARACTER class
//
ACMD(do_safebox_close)
{
	if (ch)
		ch->CloseSafebox();
}

//
// @version 05/06/20 Bang2ni - Command handling Delegate to CHARACTER class
//
ACMD(do_safebox_password)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	if (!ch)
		return;

	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;526]");
		return;
	}

	if (!*arg2 || strlen(arg2) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;526]");
		return;
	}

	TSafeboxChangePasswordPacket p{};

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;526]");
		return;
	}

	const int iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;527]");
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10) // Can only be requested once every 10 seconds
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;528]");
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p{};
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch && ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch)
		return;

	if (!ch->GetParty())
		return;

	if (!CPartyManager::Instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;530]");
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;531]");
		return;
	}

#ifdef ENABLE_12ZI
	if (ch->GetZodiac() || (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex())))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Group] You cannot leave a group while you are in a Zodiac Temple."));
		return;
	}
#endif

#ifdef ENABLE_QUEEN_NETHIS
	if (SnakeLair::CSnk::instance().IsSnakeMap(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;676]");
		return;
	}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	//if (ch->GetMeleyLair() || CMeleyLairManager::Instance().IsMeleyMap(ch->GetMapIndex()))
	if (ch->GetMeleyLair())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;676]");
		return;
	}
#endif

	LPPARTY pParty = ch->GetParty();
	if (!pParty)
		return;

	if (pParty->GetMemberCount() == 2)
	{
		// party disband
		CPartyManager::Instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;532]");
		//pParty->SendPartyRemoveOneToAll(ch);
		pParty->Quit(ch->GetPlayerID());
		//pParty->SendPartyRemoveAllToOne(ch);
	}
}

#ifdef ENABLE_AFFECT_POLYMORPH_REMOVE
ACMD(do_remove_polymorph)
{
	if (!ch)
		return;

	if (!ch->IsPolymorphed())
		return;

	ch->SetPolymorph(0);
	ch->RemoveAffect(AFFECT_POLYMORPH);
}
#endif

#ifdef ENABLE_AFFECT_BUFF_REMOVE
ACMD(do_remove_buff)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch)
		return;

	int affect = 0;
	str_to_number(affect, arg1);
	CAffect* pAffect = ch->FindAffect(affect);

	if (pAffect)
		ch->RemoveAffect(affect);
}
#endif

ACMD(do_close_shop)
{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This function is not implemented!"));
	return;
#else
	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
#endif
}

ACMD(do_set_walk_mode)
{
	if (!ch)
		return;

	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	if (!ch)
		return;

	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	if (!ch)
		return;

	//get my guild information
	CGuild* g = ch->GetGuild();

	if (!g)
		return;

	//Check if there is a war!
	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;533]");
		return;
	}

	//Divide the parameter by two

#ifdef ENABLE_NEW_WAR_OPTIONS
	char arg1[256], arg2[256], arg3[256], arg4[256], arg5[256], arg6[256];
	uint32_t type = GUILD_WAR_TYPE_FIELD; //fixme102 base int modded uint
	uint32_t flag = 0;
	uint32_t round = 0;
	uint32_t points = 0;
	uint32_t time = 0;
	six_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4), arg5, sizeof(arg5), arg6, sizeof(arg6));
#else
	char arg1[256], arg2[256];
	uint32_t type = GUILD_WAR_TYPE_FIELD; //fixme102 base int modded uint
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
#endif

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type < 0) {	//@fixme441
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<error> can't declare war with type less than zero."));
			return;
		}

		if (type >= GUILD_WAR_TYPE_MAX_NUM)	//@fixme441
			type = GUILD_WAR_TYPE_FIELD;
	}

#ifdef ENABLE_NEW_WAR_OPTIONS
	if (*arg3) {	//Flag
		str_to_number(flag, arg3);
	}

	if (*arg4) {	//Runden
		str_to_number(round, arg4);
	}

	if (*arg5) {	//Punkte
		str_to_number(points, arg5);
	}

	if (*arg6) {	//Kampfzeit
		str_to_number(time, arg6);
	}

	sys_err("do_war: type %d flag %d round %d points %d time %d", type, flag, round, points, time);
#endif

#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
	uint32_t leader_pid = g->GetMasterPID();
	uint32_t cl_grades = g->GetMember(ch->GetPlayerID())->grade;
	bool bIsLeaderOnline = g->IsOnlineLeader();

	if (bIsLeaderOnline) {
		if ((leader_pid != ch->GetPlayerID()) && !(cl_grades <= 2)) {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
			return;
		}
		else {
			if (cl_grades == 2) {
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't Do, because Leader is Online!"));
				return;
			}
		}
	}
	else {
		if ((leader_pid != ch->GetPlayerID()) && !(cl_grades <= 2)) {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
			return;
		}
		else {
			if (cl_grades == 2)
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You grant rights of War, because Leader offline!"));
		}
	}
#else
	uint32_t gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 길드전에 대한 권한이 없습니다."));
		return;
	}
#endif


	CGuild * opp_g = CGuildManager::Instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1610]");
		return;
	}

	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;537]");
					return;
				}

				int iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;538]");
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;539]");
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;780]");
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;537]");
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<길드> 이미 전쟁이 예약된 길드 입니다."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;541]");
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{
		// You do not meet the conditions for guild warfare.
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1308]");
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;543;%d]", GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	// Only check before the field and detailed checks are made when the opponent approves.
	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;544]");
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;545]");
		return;
	}

	do
	{
#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
		if (bIsLeaderOnline) {
			if (g->GetMasterCharacter() != nullptr)
				break;
		}
		else {
			if (g->GetMember(ch->GetPlayerID())->grade == 2)
				break;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1048]");
#else
		if (g->GetMasterCharacter() != nullptr)
			break;

		const CCI* pCCI = P2P_MANAGER::Instance().FindByPID(g->GetMasterPID());

		if (pCCI != nullptr)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1048]");
#endif
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
		bool boppIsLeaderOnline = opp_g->IsOnlineLeader();

		if (boppIsLeaderOnline) {
			if (opp_g->GetMasterCharacter() != nullptr)
				break;
		}
		else {
			if (opp_g->GetMember(ch->GetPlayerID())->grade == 2)
				break;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Gildenmeister und CoLeader sind offline2"));
#else
		if (opp_g->GetMasterCharacter() != nullptr)
			break;

		const CCI* pCCI = P2P_MANAGER::Instance().FindByPID(opp_g->GetMasterPID());

		if (pCCI != nullptr)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1048]");
#endif
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

#ifdef ENABLE_NEW_WAR_OPTIONS
	g->RequestDeclareWar(opp_g->GetID(), type, flag, round, points, time);
#else
	g->RequestDeclareWar(opp_g->GetID(), type);
#endif
}

ACMD(do_nowar)
{
	if (!ch)
		return;

	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	const uint32_t gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;534]");
		return;
	}

	const CGuild* opp_g = CGuildManager::Instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;535]");
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	if (ch)
		ch->DetailLog();
}

ACMD(do_monsterlog)
{
	if (ch)
		ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
		return;
#endif

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (!ch)
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	const char answer = LOWER(*arg1);
	// @fixme130 AuthToAdd void -> bool
	const bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::Instance().AuthToAdd(ch->GetName(), arg2, bIsDenied); // DENY
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg2);
		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, "[LS;548;%s]", ch->GetName());
	}
}

ACMD(do_setblockmode)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint8_t flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
	if (!ch)
		return;

	if (ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot dismount because your inventory is full."));
	}
}

ACMD(do_observer_exit)
{
/*	if(ch->IsObserverMode()  && ch->IsGM()) {	//@fixme000
		ch->SetObserverMode(false);
		return;
	}*/

	if (ch->IsObserverMode())
	{
		//@custom037
		if (CThreeWayWar::Instance().IsSungZiMapIndex(ch->GetMapIndex()) && ch->GetLevel() < 50) {
			ch->GoHome();
			return;
		}

		if (ch->GetWarMap())
			ch->SetWarMap(nullptr);

		if (ch->GetArena() != nullptr || ch->GetArenaObserverMode())
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != nullptr)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(nullptr);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else
		{
			//fixme000
			/*if (ch->GetWarpMapIndex() != 0)
				ch->ExitToSavedLocation();*/
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	if (!ch)
		return;

	if ((!view_equip_for_all) && (ch->GetGMLevel() <= GM_PLAYER))
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;

#ifdef ENABLE_MESSENGER_BLOCK
		if (MessengerManager::Instance().IsBlocked(ch->GetName(), tch->GetName()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), tch->GetName());
			return;
		}
		else if (MessengerManager::Instance().IsBlocked(tch->GetName(), ch->GetName()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), tch->GetName());
			return;
		}
#endif

#ifdef __UNIMPLEMENTED__
		if ((tch->IsBlockMode(BLOCK_VIEW_EQUIP) && !ch->IsGM()) 
			|| (!test_server && tch->IsGM()) ) {
			ch->ChatInfoTrans(("The player %s blocked viewing of his equipment!"), tch->GetName());
			return;
#endif
		tch->SendEquipment(ch);
	}
}

ACMD(do_unstuck)
{
	// This command should only work when stuck
	if (!ch->GetSectree()->IsAttr(ch->GetX(), ch->GetY(), ATTR_BLOCK | ATTR_OBJECT))
		return;

	// Try to get a valid location to unstuck the player
	// ReSharper disable once CppPossiblyErroneousEmptyStatements

	PIXEL_POSITION SetPos;
	int TryCount = 0;
	bool FoundPos = false;

	// TODO: Test for a good TryCount
	while (TryCount < 15)
	{
		// Try to get a valid random position
		// TODO: the range needs some fine tuning
		if (SECTREE_MANAGER::Instance().GetRandomLocation(ch->GetMapIndex(), SetPos, ch->GetX(), ch->GetY(), 100))
		{
			// We found a good position
			FoundPos = true;
			break;
		}
		++TryCount;
	};


	// Show the player at the new position
	if (FoundPos)
	{
		if (test_server)
			ch->ChatPacket(CHAT_TYPE_INFO, "Found valid position in %d tries", TryCount);

		ch->Show(ch->GetMapIndex(), SetPos.x, SetPos.y, SetPos.z);
		ch->Stop();

		return;
	}

	// We could not find a valid position notify the player
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("UNSTUCK_COULD_NOT_FIND_VALID_POSITION_ASK_GM"));
}

ACMD(do_party_request)
{
	if (!ch)
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;549]");
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

	if (tch)
	{
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
	}
}

ACMD(do_party_request_accept)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);
	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);
	if (tch)
		ch->DenyToParty(tch);
}

ACMD(do_monarch_warpto)
{
	if (!ch)
		return;

	if (!CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1007]");
		return;
	}

	//Lord Cooldown Check
	if (!ch->IsMCOK(CHARACTER::MI_WARP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;551;%d]", ch->GetMCLTime(CHARACTER::MI_WARP));
		return;
	}

	//Lord Mob Summon Cost
	constexpr int WarpPrice = 10000;

	//monarch treasury inspector
	if (!CMonarch::Instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		const int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;552;%d;%d]", NationMoney, WarpPrice);
		return;
	}

	int x = 0, y = 0;
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;553]");
		return;
	}

	const LPCHARACTER& tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		const CCI* pkCCI = P2P_MANAGER::Instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;554]");
				return;
			}

			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;555;%d;%d]", pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Can't go to that area"));
				return;
			}

			//fixme488
			auto gmList = get_gm_list();
			if (std::find(gmList.begin(), gmList.end(), pkCCI->szName) != gmList.end()) {
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't warp to a Game Master."));
				return;
			}

			PIXEL_POSITION pos;

			if (!SECTREE_MANAGER::Instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot find map (index %d)"), pkCCI->lMapIndex);
			else
			{
				//ch->ChatPacket(CHAT_TYPE_INFO, "You warp to (%d, %d)", pos.x, pos.y);
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;556;%d]", arg1);
				ch->WarpSet(pos.x, pos.y);

				//monarch money cut
				CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

				//Cooldown reset
				ch->SetMC(CHARACTER::MI_WARP);
			}
		}
		else if (nullptr == CHARACTER_MANAGER::Instance().FindPC(arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("There is no one by that name"));
		}

		return;
	}
	else
	{
		//fixme488
		if (tch->IsGM()) {
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't warp to a Game Master."));
			return;
		}

		if (tch->GetEmpire() != ch->GetEmpire())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;554]");
			return;
		}
		if (!IsMonarchWarpZone(tch->GetMapIndex()))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Can't go to that area"));
			return;
		}
		x = tch->GetX();
		y = tch->GetY();
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;556;%d]", arg1);
	ch->WarpSet(x, y);
	ch->Stop();

	//monarch money cut
	CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

	//Cooldown reset
	ch->SetMC(CHARACTER::MI_WARP);
}

ACMD(do_monarch_transfer)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1006]");
		return;
	}

	if (!CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1007]");
		return;
	}

	//Lord Cooldown Check
	if (!ch->IsMCOK(CHARACTER::MI_TRANSFER))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;551;%d]", ch->GetMCLTime(CHARACTER::MI_TRANSFER));
		return;
	}

	//lord warp cost
	constexpr int WarpPrice = 10000;

	//monarch treasury inspector
	if (!CMonarch::Instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		const int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;552;%d;%d]", NationMoney, WarpPrice);
		return;
	}


	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		const CCI* pkCCI = P2P_MANAGER::Instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1008]");
				return;
			}
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1009;%d;%d]", arg1, pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Can't go to that area"));
				return;
			}
			if (!IsMonarchWarpZone(ch->GetMapIndex()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Cannot be summoned to that area."));
				return;
			}

			//fixme488
			auto gmList = get_gm_list();
			for (const auto gm : gmList)  {
				if (strcasecmp(gm.c_str(), pkCCI->szName) == 0) {
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't transfer a Game Master."));
					return;
				}
			}

			TPacketGGTransfer pgg{};

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::Instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1010;%d]", arg1);

			//monarch money cut
			CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
			//Cooldown reset
			ch->SetMC(CHARACTER::MI_TRANSFER);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1011]");
		}

		return;
	}

	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1012]");
		return;
	}

	//fixme488
	if (tch->IsGM()) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't transfer a Game Master."));
		return;
	}

	if (tch->GetEmpire() != ch->GetEmpire())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1008]");
		return;
	}
	if (!IsMonarchWarpZone(tch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Can't go to that area"));
		return;
	}
	if (!IsMonarchWarpZone(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Cannot be summoned to that area."));
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());

	//monarch money cut
	CMonarch::Instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
	//Cooldown reset
	ch->SetMC(CHARACTER::MI_TRANSFER);
}

ACMD(do_monarch_info)
{
	if (!ch)
		return;

	if (CMonarch::Instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;560]");
		TMonarchInfo* p = CMonarch::Instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			if (n == ch->GetEmpire())
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;561;%s;%s;%d]", EMPIRE_NAME(n), p->name[n], p->money[n]);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;561;%s;%s]", EMPIRE_NAME(n), p->name[n]);
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;563]");
		TMonarchInfo* p = CMonarch::Instance().GetMonarch();
		for (int n = 1; n < 4; ++n)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;561;%s;%s]", EMPIRE_NAME(n), p->name[n]);
		}
	}
}

ACMD(do_elect)
{
	if (!ch)
		return;

	db_clientdesc->DBPacketHeader(HEADER_GD_COME_TO_VOTE, ch->GetDesc()->GetHandle(), 0);
}

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string st_name;

	uint8_t empire;
	int mapIndex;
	uint32_t x, y;

	GotoInfo()
	{
		st_name = "";
		empire = 0;
		mapIndex = 0;

		x = 0;
		y = 0;
	}

	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}

	void __copy__(const GotoInfo& c_src)
	{
		st_name = c_src.st_name;
		empire = c_src.empire;
		mapIndex = c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

ACMD(do_monarch_tax)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: monarch_tax <1-50>");
		return;
	}

	// monarch prosecutor
	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;564]");
		return;
	}

	// tax setting
	int tax = 0;
	str_to_number(tax, arg1);

	if (tax < 1 || tax > 50)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;565]");

	quest::CQuestManager::Instance().SetEventFlag("trade_tax", tax);

	// a message to the monarch
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;375]");

	// Bulletin
	char szMsg[1024];

	snprintf(szMsg, sizeof(szMsg), "In the name of the monarch, the tax has been changed to %d%%", tax);
	BroadcastNotice(szMsg);

	snprintf(szMsg, sizeof(szMsg), "In the future, %d%% of the transaction amount will go into the national treasury.", tax);
	BroadcastNotice(szMsg);

	// Cooldown reset
	ch->SetMC(CHARACTER::MI_TAX);
}

static const uint32_t cs_dwMonarchMobVnums[] =
{
	191, // Lykos
	192, // Scrofa
	193, // Bera
	194, // Tigris
	391, // Mi-Jung
	392, // Eun-Jung
	393, // Se-Rang
	394, // Jin-Hee
	491, // Mahon
	492, // Bo
	493, // Goo-Pae
	494, // Chuong
	591, // Bestial Captain
	691, // Chief Orc
	791, // Dark Leader
	1304, // Yellow Tiger Ghost
	1901, // Nine Tails
	2091, // Queen Spider
	2191, // Giant Tortoise
	2206, // Flame King
	0,
};

ACMD(do_monarch_mob)
{
	if (!ch)
		return;

	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;564]");
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mmob <mob name>");
		return;
	}

#ifdef ENABLE_MONARCH_MOB_CMD_MAP_CHECK // @warme006
	uint8_t pcEmpire = ch->GetEmpire();
	uint8_t mapEmpire = SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(ch->GetMapIndex());
	if (mapEmpire != pcEmpire && mapEmpire != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]This feature is only available in your own territory."));
		return;
	}
#endif

	// Lord Mob Summon Cost
	constexpr int SummonPrice = 5000000;

	// Lord Cooldown Check
	if (!ch->IsMCOK(CHARACTER::MI_SUMMON))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;551;%d]", ch->GetMCLTime(CHARACTER::MI_SUMMON));
		return;
	}

	// monarch treasury inspector
	if (!CMonarch::Instance().IsMoneyOk(SummonPrice, ch->GetEmpire()))
	{
		const int NationMoney = CMonarch::Instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;552;%d;%d]", NationMoney, SummonPrice);
		return;
	}

	const CMob* pkMob;
	uint32_t vnum = 0;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::Instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	uint32_t count;

	// Summonable Mob Check
	for (count = 0; cs_dwMonarchMobVnums[count] != 0; ++count)
	{
		if (cs_dwMonarchMobVnums[count] == vnum)
			break;
	}

	if (0 == cs_dwMonarchMobVnums[count])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;567]");
		return;
	}

	tch = CHARACTER_MANAGER::Instance().SpawnMobRange(vnum,
		ch->GetMapIndex(),
		ch->GetX() - number(200, 750),
		ch->GetY() - number(200, 750),
		ch->GetX() + number(200, 750),
		ch->GetY() + number(200, 750),
		true,
		pkMob && pkMob->m_table.bType == CHAR_TYPE_STONE ? true : false,
		true);

	if (tch)
	{
		// monarch money cut
		CMonarch::Instance().SendtoDBDecMoney(SummonPrice, ch->GetEmpire(), ch);

		// Cooldown reset
		ch->SetMC(CHARACTER::MI_SUMMON);
	}
}

static const char* FN_point_string(int apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("최대 생명력 +%d");
		case POINT_MAX_SP:	return LC_TEXT("최대 정신력 +%d");
		case POINT_HT:		return LC_TEXT("체력 +%d");
		case POINT_IQ:		return LC_TEXT("지능 +%d");
		case POINT_ST:		return LC_TEXT("근력 +%d");
		case POINT_DX:		return LC_TEXT("민첩 +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("공격속도 +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("이동속도 %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("쿨타임 -%d");
		case POINT_HP_REGEN:	return LC_TEXT("생명력 회복 +%d");
		case POINT_SP_REGEN:	return LC_TEXT("정신력 회복 +%d");
		case POINT_POISON_PCT:	return LC_TEXT("독공격 %d");
		case POINT_BLEEDING_PCT:	return LC_TEXT("독공격 %d");
#if defined(ENABLE_PASSIVE_ATTR) && defined(ENABLE_PRECISION)
		case POINT_HIT_PCT:	return LC_TEXT("독공격 %d");
#endif
		case POINT_STUN_PCT:	return LC_TEXT("스턴 +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("슬로우 +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% 확률로 치명타 공격");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("상대의 치명타 확률 %d%% 감소");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% 확률로 관통 공격");
		case POINT_RESIST_PENETRATE: return LC_TEXT("상대의 관통 공격 확률 %d%% 감소");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("인간류 몬스터 타격치 +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("동물류 몬스터 타격치 +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("웅귀족 타격치 +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("밀교류 타격치 +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("시체류 타격치 +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("악마류 타격치 +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("타격치 %d%% 를 생명력으로 흡수");
		case POINT_STEAL_SP:		return LC_TEXT("타력치 %d%% 를 정신력으로 흡수");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% 확률로 타격시 상대 전신력 소모");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% 확률로 피해시 정신력 회복");
		case POINT_BLOCK:			return LC_TEXT("물리타격시 블럭 확률 %d%%");
		case POINT_DODGE:			return LC_TEXT("활 공격 회피 확률 %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("한손검 방어 %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("양손검 방어 %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("두손검 방어 %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("방울 방어 %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("부채 방어 %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("활공격 저항 %d%%");
		case POINT_RESIST_CLAW:		return LC_TEXT("두손검 방어 %d%%");
		case POINT_RESIST_FIRE:		return LC_TEXT("화염 저항 %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("전기 저항 %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ACCEDRAIN_RATE:	return LC_TEXT("마법 저항 %d%%");
		case POINT_RESIST_MAGIC_REDUCTION:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ATTBONUS_CZ:		return LC_TEXT("악마류 타격치 +%d%%");
		case POINT_ATTBONUS_DESERT:	return LC_TEXT("악마류 타격치 +%d%%");
		case POINT_ATTBONUS_INSECT:	return LC_TEXT("악마류 타격치 +%d%%");
		case POINT_ENCHANT_ELECT:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ENCHANT_ICE:		return LC_TEXT("마법 저항 %d%%");
		case POINT_ENCHANT_DARK:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ENCHANT_FIRE:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ENCHANT_WIND:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ENCHANT_EARTH:	return LC_TEXT("마법 저항 %d%%");
		case POINT_ATTBONUS_SWORD:	return LC_TEXT("Strong against Sword: %d%%");
		case POINT_ATTBONUS_TWOHAND:	return LC_TEXT("Strong against Two Hand: %d%%");
		case POINT_ATTBONUS_DAGGER:	return LC_TEXT("Strong against Dagger: %d%%");
		case POINT_ATTBONUS_BELL:	return LC_TEXT("Strong against Bell: %d%%");
		case POINT_ATTBONUS_FAN:	return LC_TEXT("Strong against Fan: %d%%");
		case POINT_ATTBONUS_BOW:	return LC_TEXT("Strong against Bow: %d%%");
		case POINT_ATTBONUS_CLAW:	return LC_TEXT("Strong against Claw: %d%%");
		case POINT_RESIST_MOUNT_FALL:	return LC_TEXT("Resist Mount Fall: %d%%");
		case POINT_RESIST_HUMAN: return LC_TEXT("마법 저항 %d%%");
		case POINT_RESIST_WIND:		return LC_TEXT("바람 저항 %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("냉기 저항 %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("대지 저항 %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("어둠 저항 %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("직접 타격치 반사 확률 : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("저주 되돌리기 확률 %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("독 저항 %d%%");
		case POINT_BLEEDING_REDUCE:	return LC_TEXT("독 저항 %d%%");
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% 확률로 적퇴치시 정신력 회복");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 경험치 추가 상승");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 돈 2배 드롭");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% 확률로 적퇴치시 아이템 2배 드롭");
		case POINT_POTION_BONUS:	return LC_TEXT("물약 사용시 %d%% 성능 증가");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% 확률로 적퇴치시 생명력 회복");
//		case POINT_IMMUNE_STUN:	return LC_TEXT("기절하지 않음 %d%%");
//		case POINT_IMMUNE_SLOW:	return LC_TEXT("느려지지 않음 %d%%");
//		case POINT_IMMUNE_FALL:	return LC_TEXT("넘어지지 않음 %d%%");
//		case POINT_SKILL:	return LC_TEXT("");
//		case POINT_BOW_DISTANCE:	return LC_TEXT("");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("공격력 +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("방어력 +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("마법 공격력 +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("마법 방어력 +%d");
//		case POINT_CURSE_PCT:	return LC_TEXT("");
		case POINT_MAX_STAMINA:	return LC_TEXT("최대 지구력 +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("무사에게 강함 +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("자객에게 강함 +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("수라에게 강함 +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("무당에게 강함 +%d%%");
		case POINT_ATTBONUS_WOLFMAN:	return LC_TEXT("무당에게 강함 +%d%%");
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("몬스터에게 강함 +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("공격력 +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("방어력 +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("경험치 %d%%");
//		case POINT_MALL_ITEMBONUS:		return LC_TEXT("아이템 드롭율 %.1f배");
//		case POINT_MALL_GOLDBONUS:		return LC_TEXT("돈 드롭율 %.1f배");
		case POINT_MALL_ITEMBONUS:      return LC_TEXT("아이템 드롭율 %d배"); // @fixme180 float to int
		case POINT_MALL_GOLDBONUS:      return LC_TEXT("돈 드롭율 %d배"); // @fixme180 float to int 
		case POINT_MAX_HP_PCT:			return LC_TEXT("최대 생명력 +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("최대 정신력 +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("스킬 데미지 %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("평타 데미지 %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("스킬 데미지 저항 %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("평타 데미지 저항 %d%%");
//		case POINT_PC_BANG_EXP_BONUS:	return LC_TEXT("");
//		case POINT_PC_BANG_DROP_BONUS:	return LC_TEXT("");
//		case POINT_EXTRACT_HP_PCT:	return LC_TEXT("");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("무사공격에 %d%% 저항");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("자객공격에 %d%% 저항");
		case POINT_RESIST_SURA:		return LC_TEXT("수라공격에 %d%% 저항");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("무당공격에 %d%% 저항");
		case POINT_RESIST_WOLFMAN:	return LC_TEXT("무당공격에 %d%% 저항");
//		default:					return nullptr;
		default:                    return "UNK_ID %d%%"; // @fixme180
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char* buf, size_t bufsiz)
{
	if (nullptr == ch || nullptr == buf)
		return false;

	CAffect* aff = nullptr;
	time_t expire = 0;
	struct tm ltm {};
	int year = 0, mon = 0, day = 0;
	int offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (nullptr == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	// set apply string
	offset = snprintf(buf, bufsiz, FN_point_string(aff->wApplyOn), aff->lApplyValue);

	if (offset < 0 || offset >= static_cast<int>(bufsiz))
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year = ltm.tm_year + 1900;
	mon = ltm.tm_mon + 1;
	day = ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT("[651](Procedure: %d y- %d m - %d d)"), year, mon, day);

	return true;
}

ACMD(do_costume)
{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	char buf[768] = { '\0' };
#else
	char buf[512];
#endif
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	CItem* pMount = ch->GetWear(WEAR_COSTUME_MOUNT);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	CItem* pAcce = ch->GetWear(WEAR_COSTUME_ACCE);
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	CItem* pWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
#endif
#ifdef ENABLE_AURA_SYSTEM
	CItem* pAura = ch->GetWear(WEAR_COSTUME_AURA);
#endif

	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.wType)
			{
				snprintf(buf, bufferSize, FN_point_string(attr.wType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (pMount)
	{
		const char* itemName = pMount->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  MOUNT : %s", itemName);

		if (pMount->IsEquipped() && arg1[0] == 'm')
			ch->UnequipItem(pMount);
	}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (pAcce)
	{
		const char* itemName = pAcce->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  ACCE : %s", itemName);
		for (int i = 0; i < pAcce->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pAcce->GetAttribute(i);
			if (attr.wType > 0)
			{
				const char* pAttrName = FN_point_string(attr.wType);
				if (pAttrName == nullptr)
					continue;

				snprintf(buf, sizeof(buf), FN_point_string(attr.wType), attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pAcce->IsEquipped() && arg1[0] == 's')
			ch->UnequipItem(pAcce);
	}
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (pWeapon)
	{
		const char* itemName = pWeapon->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  WEAPON : %s", itemName);

		if (pWeapon->IsEquipped() && arg1[0] == 'w')
			ch->UnequipItem(pWeapon);
	}
#endif

#ifdef ENABLE_AURA_SYSTEM
	if (pAura)
	{
		const char* itemName = pAura->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  AURA : %s", itemName);
		if (pAura->IsEquipped() && arg1[0] == 'a')
			ch->UnequipItem(pAura);
	}
#endif
}

ACMD(do_hair)
{
	if (!ch)
		return;

	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	if (!ch)
		return;

	int index = 0;
	int count = 1;

	char arg1[256];
	char arg2[256];

	LPITEM item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int i = 0; i < count; ++i)
	{
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		if (index >= ch->GetExtendInvenMax())
#else
		if (index >= INVENTORY_MAX_NUM)
#endif
		{
			break;
		}

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
			index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

//gift notify quest command
ACMD(do_gift)
{
	if (ch)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

#ifdef ENABLE_CUBE_RENEWAL
ACMD(do_cube)
{
	if (!ch)
		return;

	ch->SetTempCubeNPC(ch->GetQuestNPC()->GetRaceNum());
	ch->SetCubeNpc(ch->GetQuestNPC());
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
	ch->SetOpenedWindow(W_CUBE, true);
#endif

	TEMP_BUFFER buf;
	TPacketGCCubeRenewal pack;
	TSubPacketGCCubeRenewalOpenClose sub{};
	pack.wSize = sizeof(TPacketGCCubeRenewal) + sizeof(TSubPacketGCCubeRenewalOpenClose);
	pack.bSubHeader = CUBE_RENEWAL_OPEN;
	sub.npc_vnum = ch->GetQuestNPC()->GetRaceNum();
	sub.open_state = true;

	LPDESC desc = ch->GetDesc();
	if (!desc)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", ch->GetName());
		return;
	}

	buf.write(&pack, sizeof(TPacketGCCubeRenewal));
	buf.write(&sub, sizeof(TSubPacketGCCubeRenewalOpenClose));
	desc->Packet(buf.read_peek(), buf.size());
}
#else
ACMD(do_cube)
{
	if (!ch)
		return;

	if (!ch->CanDoCube())
		return;

	dev_log(LOG_DEB0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int cube_index = 0, inven_index = 0;
	const char* line;

	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		// print usage
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	// r_info (request information)
	// /cube r_info ==> (Client -> Server) Request a recipe that the current NPC can make
	// (Server -> Client) /cube r_list npcVNUM resultCOUNT 123,1/125,1/128,1/130,5
	//
	// /cube r_info 3 ==> (Client -> Server) Requests the information needed to make the 3rd item among the recipes that NPCs can make.
	// /cube r_info 3 5 ==> (Client -> Server) Request the material information required to make the 3rd item and the next 5 items among the recipes that the NPC can make now.
	// (Server -> Client) /cube m_info startIndex count 125,1|126,2|127,2|123,5&555,5&555,4/120000@125,1|126,2|127,2|123,5&555,5&555,4/120000
	//
	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o': // open
			Cube_open(ch);
			break;

		case 'c': // close
			Cube_close(ch);
			break;

		case 'l': // list
			Cube_show_list(ch);
			break;

		case 'a': // add cue_index inven_index
			{
				if (0 == arg2[0] || !isdigit(*arg2) ||
					0 == arg3[0] || !isdigit(*arg3))
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
				Cube_add_item(ch, cube_index, inven_index);
			}
			break;

		case 'd': // delete
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item(ch, cube_index);
			}
			break;

		case 'm': // make
			if (0 != arg2[0])
			{
				while (true == Cube_make(ch))
					dev_log(LOG_DEB0, "cube make success");
			}
			else
				Cube_make(ch);
			break;

		default:
			return;
	}
}
#endif

ACMD(do_in_game_mall)
{
	char buf[512 + 1];
	char sas[33];
	MD5_CTX ctx;
	const char sas_key[] = "GF9001";

	char language[3];
	strcpy(language, "de");

	snprintf(buf, sizeof(buf), "%u%u%s", ch->GetPlayerID(), ch->GetAID(), sas_key);

	MD5Init(&ctx);
	MD5Update(&ctx, (const uint8_t*)buf, strlen(buf));
#ifdef __FreeBSD__
	MD5End(&ctx, sas);
#else
	static const char hex[] = "0123456789abcdef";
	uint8_t digest[16];
	MD5Final(digest, &ctx);
	int i;

	for (i = 0; i < 16; ++i)
	{
		sas[i + i] = hex[digest[i] >> 4];
		sas[i + i + 1] = hex[digest[i] & 0x0f];
	}
	sas[i + i] = '\0';
#endif

#ifdef __WIN32__
	snprintf(buf, sizeof(buf), "mall %s/?pid=%u&c=%s&sid=%d&sas=%s",
		g_strWebMallURL.c_str(), ch->GetPlayerID(), language, g_server_id, sas);
#else
	snprintf(buf, sizeof(buf), "mall http://%s/?pid=%u&c=%s&sid=%d&sas=%s",
		g_strWebMallURL.c_str(), ch->GetPlayerID(), language, g_server_id, sas);
#endif

	ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
}

// dice
ACMD(do_dice)
{
	if (!ch)
		return;

	char arg1[256], arg2[256];
	int start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	const int n = number(start, end);

#ifdef ENABLE_DICE_SYSTEM
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "[LS;1046;%s;%d;%d;%d]", ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_DICE_INFO, "[LS;1047;%d;%d;%d]", n, start, end);
#else
	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, "[LS;1046;%s;%d;%d;%d]", ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1047;%d;%d;%d]", n, start, end);
#endif
}

#ifdef ENABLE_NEWSTUFF
ACMD(do_click_safebox)
{
	if (!ch)
		return;

	/*if (!ch->GetQuestFlag("stash.safebox_delivered"))
	{
		constexpr uint32_t dwVnum = 9005;
		const CMob* pkMob = CMobManager::Instance().Get(dwVnum);
		if (pkMob)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("To open the storeroom for the first time, you must speak to %s."), pkMob->m_table.szLocaleName);

		return;
	}*/

	if ((ch->GetGMLevel() <= GM_PLAYER) && (ch->GetDungeon() || ch->GetWarMap()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}

ACMD(do_force_logout)
{
	if (!ch)
		return;

	LPDESC pDesc = DESC_MANAGER::Instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}
#endif

ACMD(do_click_mall)
{
	if (ch)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
ACMD(do_click_guildstorage)
{
	ch->SetGuildstorageOpenPosition();
	ch->ReqGuildstorageLoad();
}

ACMD(do_guildstorage_close)
{
	if (ch->GetGuildstorage())
	{
		CGuild* pGuild = ch->GetGuild();
		if (!pGuild) {
			return;// no guild ptr fix
		}

		ch->CloseGuildstorage();
		ch->Save();
		pGuild->SetStorageState(false, 0);
	}
}

ACMD(do_update_guildstorage)
{
	CGuild* pGuild = ch->GetGuild();
	pGuild->SetGuildstorage(1);
}
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
extern bool IS_MOUNTABLE_ZONE(int map_index);	//@custom005
#endif
ACMD(do_ride)
{
	if (!ch)
		return;

	dev_log(LOG_DEB0, "[DO_RIDE] start");
	if (ch->IsDead() || ch->IsStun())
		return;

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;98]");
		return;
	}
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
	if (!IS_MOUNTABLE_ZONE(ch->GetMapIndex()))	//@custom005
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("RIDING_IS_BLOCKED_HERE"));
		return;
	}
#endif

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot use a mount as long as you are polymorphed."));
		return;
	}

	// get off
	{
		if (ch->IsHorseRiding())
		{
			dev_log(LOG_DEB0, "[DO_RIDE] stop riding");
			ch->StopRiding();
			return;
		}

		if (ch->GetMountVnum())
		{
			dev_log(LOG_DEB0, "[DO_RIDE] unmount");
			do_unmount(ch, nullptr, 0, 0);
			return;
		}
	}

	// ride
	{
		if (ch->GetHorse() != nullptr)
		{
			dev_log(LOG_DEB0, "[DO_RIDE] start riding");
			ch->StartRiding();
			return;
		}

		for (uint8_t i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item = ch->GetInventoryItem(i);
			if (nullptr == item)
				continue;

			// unique mount item
			if (item->IsRideItem())
			{
				if (
					nullptr == ch->GetWear(WEAR_UNIQUE1)
					|| nullptr == ch->GetWear(WEAR_UNIQUE2)
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
					|| nullptr == ch->GetWear(WEAR_COSTUME_MOUNT)
#endif
					)
				{
					dev_log(LOG_DEB0, "[DO_RIDE] USE UNIQUE ITEM");
					//ch->EquipItem(item);
					ch->UseItem(TItemPos(INVENTORY, i));
					return;
				}
			}

			// general mount item
			// TODO: Add SubType for Vehicles
			switch (item->GetVnum())
			{
				case 71114: // Wild Boar (yellow)
				case 71116: // Wolf (yellow)
				case 71118: // Tiger (yellow)
				case 71120: // Lion (yellow)
					dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
					ch->UseItem(TItemPos(INVENTORY, i));
					return;

				default:
					break;
			}

			// GF mantis #113524, mount 52001~52090
			if ((item->GetVnum() > 52000) && (item->GetVnum() < 52091))
			{
				dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
				ch->UseItem(TItemPos(INVENTORY, i));
				return;
			}
		}
	}

	// When you can't get on or off
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;501]");
}

#ifdef ENABLE_RANKING_SYSTEM
void LoadRanking(uint8_t bCategory)
{
	TPacketGGLoadRanking p{};
	p.bHeader = HEADER_GG_LOAD_RANKING;
	p.bCategory = bCategory;
	P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGLoadRanking));
	CRankingSystem::Instance().LoadRanking(bCategory);
}
#endif

#ifdef ENABLE_BATTLE_FIELD
ACMD(do_open_battle_ui)
{
	if (!ch)
		return;

	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ExitBattleField %d", ch->GetBattleFieldPoint());
		return;
	}

	CBattleField::Instance().OpenBattleUI(ch);
}

ACMD(do_goto_battle)
{
	CBattleField::Instance().RequestEnter(ch);
}

ACMD(do_exit_battle_field)
{
	CBattleField::Instance().RequestExit(ch);
}

ACMD(do_exit_battle_field_on_dead)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t arg = 0;
	if (!*arg1 || !str_to_number(arg, arg1))
		return;

	if (!ch)
		return;

	switch (arg)
	{
		case 0:
		{
			if (ch->GetDesc())
			{
				TPacketGCDead pack{};
				pack.header = HEADER_GC_DEAD;
				pack.vid = ch->GetVID();
				pack.bRestart = true;
				pack.lMapIdx = ch->GetMapIndex();
				ch->GetDesc()->Packet(&pack, sizeof(pack));
				ch->ComputePoints();
			}
			break;
		}
		case 1:
		{
			CBattleField::Instance().ExitCharacter(ch);
			break;
		}

		default:
			break;
	}
}
#endif

#ifdef ENABLE_12ZI
ACMD(do_cz_check_box)
{
	const char *line;
	char arg1[256], arg2[256];
	line = two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (0 == arg1[0] || 0 == arg2[0])
		return;

	int color = atoi(arg1);
	int index = atoi(arg2);

	if (color < 0 || color > 1)
		return;

	if (index < 0 || index > 29)
		return;

	ch->ZTT_CHECK_BOX(color,index);
	ch->ZTT_LOAD_INFO();
}

ACMD(do_cz_reward)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
		return;

	int type = atoi(arg1);

	if(type < 1 || type > 3)
		return;

	ch->ZTT_REWARD(type);
}

ACMD(do_revivedialog)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Missing data. Please, contact an administrator."));
		return;
	}

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("There is no one to resuscitate."));
		return;
	}

	if (!tch->IsPC())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The person you are trying to revive is not human."));
		return;
	}

	if (!tch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can not resurrect someone who is alive."));
		return;
	}

	uint8_t bNeedPrism;
	if (tch->GetQuestFlag("12zi_temple.IsDead") == 1 || tch->GetQuestFlag("12zi_temple.PrismNeed") > 0)
	{
		if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 1)
			bNeedPrism = 1;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 2)
			bNeedPrism = 2;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 3)
			bNeedPrism = 4;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 4)
			bNeedPrism = 8;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") >= 5)
			bNeedPrism = 10;
		else
			bNeedPrism = tch->GetQuestFlag("12zi_temple.PrismNeed");
	}
	else
	{
		if (tch->GetDeadCount() == 3)
			bNeedPrism = 4;
		else if (tch->GetDeadCount() == 4)
			bNeedPrism = 8;
		else if (tch->GetDeadCount() >= 5)
			bNeedPrism = 10;
		else
			bNeedPrism = tch->GetDeadCount();
	}

	ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %u %u", (uint32_t)tch->GetVID(), bNeedPrism);
}

ACMD(do_revive)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Missing data. Please, contact an administrator."));
		return;
	}

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().Find(vid);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("There is no one to resuscitate."));
		return;
	}

	if (!tch->IsPC())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The person you are trying to revive is not human."));
		return;
	}

	if (!tch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can not resurrect someone who is alive."));
		return;
	}

	if (!(CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex())) || !(CZodiacManager::Instance().IsZiStageMapIndex(tch->GetMapIndex())))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This action can only be done in the Zodiac Temple."));
		return;
	}

	uint8_t bNeedPrism;
	if (tch->GetQuestFlag("12zi_temple.IsDead") == 1 || tch->GetQuestFlag("12zi_temple.PrismNeed") > 0)
	{
		if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 1)
			bNeedPrism = 1;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 2)
			bNeedPrism = 2;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 3)
			bNeedPrism = 4;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") == 4)
			bNeedPrism = 8;
		else if (tch->GetQuestFlag("12zi_temple.PrismNeed") >= 5)
			bNeedPrism = 10;
		else
			bNeedPrism = tch->GetQuestFlag("12zi_temple.PrismNeed");
	}
	else
	{
		if (tch->GetDeadCount() == 3)
			bNeedPrism = 4;
		else if (tch->GetDeadCount() == 4)
			bNeedPrism = 8;
		else if (tch->GetDeadCount() >= 5)
			bNeedPrism = 10;
		else
			bNeedPrism = tch->GetDeadCount();
	}

	int iPrismCount = (ch->CountSpecifyItem(33025) + ch->CountSpecifyItem(33032));
	if (iPrismCount < (int)bNeedPrism)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "NotEnoughPrism %u", bNeedPrism);
		return;
	}

	int iDelPrism = bNeedPrism - ch->CountSpecifyItem(33032);
	if (iDelPrism <= 0)
	{
		ch->RemoveSpecifyItem(33032, bNeedPrism);
	}
	else
	{
		ch->RemoveSpecifyItem(33025, bNeedPrism - ch->CountSpecifyItem(33032));
		ch->RemoveSpecifyItem(33032, ch->CountSpecifyItem(33032));
	}

	tch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
	tch->GetDesc()->SetPhase(PHASE_GAME);
	tch->SetPosition(POS_STANDING);
	tch->StartRecoveryEvent();
	tch->RestartAtSamePos();
	tch->PointChange(POINT_HP, tch->GetMaxHP() - tch->GetHP());
	tch->PointChange(POINT_SP, tch->GetMaxSP() - tch->GetSP());
	tch->ReviveInvisible(5);
#ifdef ENABLE_PET_SYSTEM
	tch->CheckPet();
#endif
	tch->SetQuestFlag("12zi_temple.IsDead", 0);
	sys_log(0, "do_restart: restart here zodiac");
}

ACMD(do_jump_floor)
{
	if (ch)
	{
		if ((ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID()) || !ch->GetParty())
		{
			LPZODIAC pkZodiac = CZodiacManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (pkZodiac && pkZodiac->IsNextFloor() == true)
			{
				pkZodiac->NewFloor(pkZodiac->GetNextFloor());
			}
		}
	}
}

ACMD(do_next_floor)
{
	if (ch)
	{
		if ((ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID()) || !ch->GetParty())
		{
			LPZODIAC pkZodiac = CZodiacManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (pkZodiac && pkZodiac->IsNextFloor() == true)
			{
				pkZodiac->NewFloor(pkZodiac->GetFloor() + 1);
			}
		}
	}
}

ACMD(do_cz_complete_reward)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cz_complete_reward <color>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available colors:");
		ch->ChatPacket(CHAT_TYPE_INFO, " yellow");
		ch->ChatPacket(CHAT_TYPE_INFO, " green");
		ch->ChatPacket(CHAT_TYPE_INFO, " all");
		return;
	}

	std::string strArg(arg1);
	if (!strArg.compare(0, 6, "yellow"))
	{
		if (ch->IsGM())
		{
			ch->SetQuestFlag("12zi_temple.zt_color_0", 1073741823);
			ch->ZTT_CHECK_REWARD();
			ch->ZTT_LOAD_INFO();
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Only GM have access to this command"));
	}
	else if (!strArg.compare(0, 5, "green"))
	{
		if (ch->IsGM())
		{
			ch->SetQuestFlag("12zi_temple.zt_color_1", 1073741823);
			ch->ZTT_CHECK_REWARD();
			ch->ZTT_LOAD_INFO();
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Only GM have access to this command"));
	}
	else if (!strArg.compare(0, 3, "all"))
	{
		if (ch->IsGM())
		{
			ch->SetQuestFlag("12zi_temple.zt_color_0", 1073741823);
			ch->SetQuestFlag("12zi_temple.zt_color_1", 1073741823);
			ch->ZTT_CHECK_REWARD();
			ch->ZTT_LOAD_INFO();
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Only GM have access to this command"));
	}
}
#endif
/*--------------------------------------------------*/

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#include "shop_manager.h"
#include "shop.h"
ACMD(do_shop_manage)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	const uint32_t pid = ch->GetPlayerID();
	if (!pid)
		return;

	LPCHARACTER myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!myShopChar)
		return;

	bool isEditing = false;
	str_to_bool(isEditing, arg1);

	myShopChar->SetEditingShop(isEditing);
	myShopChar->GetMyShop()->CloseShopBecauseEditing();
}

#ifdef ENABLE_OPEN_SHOP_WITHOUT_BAG
extern bool IS_BOTARYABLE_ZONE(int nMapIndex); // char_item.cpp
ACMD(do_shop_open)
{
	if (!ch)
		return;

	if (ch->GetExchange() || ch->IsShop() || ch->GetViewingShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen()
#	ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| ch->IsOpenGuildstorage()
#	endif
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래창,창고 등을 연 상태에서는 보따리,비단보따리를 사용할수 없습니다."));
		return;
	}

	if (g_bEnableBootaryCheck && IS_BOTARYABLE_ZONE(ch->GetMapIndex()) == false)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("개인 상점을 열 수 없는 지역입니다"));
		return;
	}

	ch->OpenPrivateShop();
}
#endif

#ifdef ENABLE_OPEN_SHOP_WITH_PASSWORD
ACMD(do_click_shop_editor)
{
	if (!ch)
		return;

	//ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMePremiumShopPassword");
}

ACMD(do_shop_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!ch)
		return;

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;526]");
		return;
	}

	const int iPulse = thecore_pulse();
	if (iPulse - ch->GetPremiumShopLoadTime() < passes_per_sec * 10) // Can only be requested once every 10 seconds
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1719;%d]", 10);
		return;
	}
	ch->SetPremiumShopLoadTime(iPulse);

	char defaultPass[256] = "000000";
	char noPass[256] = "";
	char qGetSafeboxPass[256];
	snprintf(qGetSafeboxPass, sizeof(qGetSafeboxPass), "SELECT password FROM player.safebox WHERE account_id = %u;", ch->GetDesc()->GetAccountTable().id);
	auto pMsg(DBManager::Instance().DirectQuery(qGetSafeboxPass));
	SQLResult* res = pMsg->Get();

	if (res && res->uiNumRows == 0)
	{
		if (*arg1 == *defaultPass)
			ch->OpenShopEditorWithPassword(ch, true);
		else
			ch->OpenShopEditorWithPassword(ch, false);
		return;
	}

	MYSQL_ROW row;
	row = mysql_fetch_row(res->pSQLResult);
	if ((*arg1 == *row[0]) || ((*arg1 == *defaultPass) && (*row[0] == *noPass)))
		ch->OpenShopEditorWithPassword(ch, true);
	else
		ch->OpenShopEditorWithPassword(ch, false);
}
#endif
#endif

#ifdef ENABLE_DS_REFINE_WINDOW
ACMD(do_open_refine_ds)
{
	if (ch)
		ch->DragonSoul_RefineWindow_Open(ch);
}
#endif

#ifdef ENABLE_GUILDWAR_BUTTON
ACMD(do_guildwar_request_enter)
{
	if (!ch)
		return;

	if (ch->GetGuild())
		ch->GetGuild()->GuildWarEntryAccept(ch->GetGuild()->UnderAnyWar(), ch);
}
#endif

#ifdef ENABLE_SOCCER_BALL_EVENT
ACMD(do_top_ver)
{
	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHack())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetShopOwner() ||
#endif
		ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("kelime_event_kpt"));
		return;
	}

	if (ch->CountSpecifyItem(50096) < 20)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("kelime_event_yk"));
		return;
	}
	ch->RemoveSpecifyItem(50096, 20);
	ch->AutoGiveItem(50265, 1);
}
#endif

#ifdef ENABLE_WORD_GAME_EVENT
ACMD(do_word_game)
{
	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHack())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetShopOwner() ||
#endif
		ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("kelime_event_kpt"));
		return;
	}

	if (ch->CountSpecifyItem(30216) == 0 || ch->CountSpecifyItem(30213) == 0 || ch->CountSpecifyItem(30219) == 0 || ch->CountSpecifyItem(30214) == 0 || ch->CountSpecifyItem(30217) == 0 || ch->CountSpecifyItem(30210) == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("kelime_event_yk"));
		return;
	}
	ch->RemoveSpecifyItem(30216, 1);
	ch->RemoveSpecifyItem(30213, 1);
	ch->RemoveSpecifyItem(30219, 1);
	ch->RemoveSpecifyItem(30214, 1);
	ch->RemoveSpecifyItem(30217, 1);
	ch->RemoveSpecifyItem(30210, 1);
	ch->AutoGiveItem(50128, 1);
}
#endif

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
ACMD(do_hide_costume)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	bool hidden = true;
	uint8_t bPartPos = 0;
	uint8_t bHidden = 0;

	str_to_number(bPartPos, arg1);

	if (*arg2)
	{
		str_to_number(bHidden, arg2);

		if (bHidden == 0)
			hidden = false;
	}

	if (ch->IsDead())
		return;

	bool bAttacking = (get_dword_time() - ch->GetLastAttackTime()) < 1500;
	bool bMoving = (get_dword_time() - ch->GetLastMoveTime()) < 1500;
	bool bDelayedCMD = false;

	if (ch->IsStateMove() || bAttacking || bMoving)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have to stand still to hide your costume."));
		return;
	}

	if (bDelayedCMD)
	{
		int iPulse = thecore_pulse();
		if (iPulse - ch->GetHideCostumePulse() < passes_per_sec * 3)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have to wait 3 seconds before you can hide your costume again."));
			return;
		}
		ch->SetHideCostumePulse(iPulse);
	}

	if (bPartPos == 1)
		ch->SetBodyCostumeHidden(hidden);
	else if (bPartPos == 2)
		ch->SetHairCostumeHidden(hidden);
# ifdef ENABLE_ACCE_COSTUME_SYSTEM
	else if (bPartPos == 3)
		ch->SetAcceCostumeHidden(hidden);
# endif
# ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	else if (bPartPos == 4)
		ch->SetWeaponCostumeHidden(hidden);
# endif
#ifdef ENABLE_AURA_SYSTEM
	else if (bPartPos == 5)
		ch->SetAuraCostumeHidden(hidden);
# endif
	else
		return;

	ch->UpdatePacket();
}
#endif

#ifdef NEW_SELECT_CHARACTER
ACMD(do_set_play_time)
{
	if (!ch)
		return;

	if (ch->GetRealPoint(POINT_PLAYTIME) >= 1)
		return;

	ch->SetRealPoint(POINT_PLAYTIME, ch->GetRealPoint(POINT_PLAYTIME) + 1);
	ch->SetPoint(POINT_PLAYTIME, ch->GetPoint(POINT_PLAYTIME) + 1);
	//ch->ComputePoints(); // Not necessary
}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
ACMD(do_gdonate)
{
	if (!ch)
		return;

	const int iPulse = thecore_pulse();
	if (iPulse - ch->GetLastOfferNewExpTime() < PASSES_PER_SEC(10))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
		return;
	}

	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	const uint32_t g_id = g->GetID();
	const time_t waitTime = (g->GetMemberLastDailyDonate(ch->GetPlayerID()) + (60 * 60 * 24));
	if (waitTime > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have already attended today."));
		return;
	}

	const int donateCount = g->GetDailyGuildDonatePoints(g_id);

	int donateBonus = 0;
	if (donateCount <= 19)
		donateBonus = 1;
	else if (donateCount <= 29)
		donateBonus = 2;
	else if (donateCount <= 39)
		donateBonus = 4;
	else if (donateCount <= 49)
		donateBonus = 6;
	else if (donateCount <= 59)
		donateBonus = 8;
	else if (donateCount <= 69)
		donateBonus = 10;
	else if (donateCount <= 100)
		donateBonus = 12;

	ch->PointChange(POINT_MEDAL_OF_HONOR, donateBonus, true);
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d Donate points earned!"), donateBonus);

	g->SetGuildAttendanceState(ch->GetPlayerID(), donateBonus, get_global_time());
	ch->SetLastOfferNewExpTime();
}
#endif

#ifdef ENABLE_INVENTORY_SORT
bool sortByType(CItem* a, CItem* b)
{
	if (!a || !b)
		return false;

	return (a->GetType() < b->GetType());
}

bool sortBySubType(CItem* a, CItem* b)
{
	if (!a || !b)
		return false;

	return (a->GetSubType() < b->GetSubType());
}

bool sortByVnum(CItem* a, CItem* b)
{
	if (!a || !b)
		return false;

	return (a->GetVnum() < b->GetVnum());
}

bool sortBySocket(CItem* a, CItem* b)
{
	if (!a || !b)
		return false;

	return (a->GetSocket(0) < b->GetSocket(0));
}

ACMD(do_sort_inventory)
{
	if (!ch)
		return;

	if (!ch->CanHandleItem())
		return;

	int iPulse = thecore_pulse();	//@custom038
	if (iPulse - ch->GetInventorySortPulse() < PASSES_PER_SEC(10)) {	//if (ch->GetInventorySortPulse() + 100 >thecore_pulse())
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_HAVE_TO_WAITE_BEFORE_YOU_CAN_SORT_AGAIN"));
		return;
	}

	std::vector<CItem*> collectItems;
	int totalSize = 0;

	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item)
		{
#ifdef ENABLE_ANTI_EXP_RING
			if ((item->GetVnum() == ITEM_AUTO_EXP_RING_S || item->GetVnum() == ITEM_AUTO_EXP_RING_M || item->GetVnum() == ITEM_AUTO_EXP_RING_G) && item->GetSocket(0) == 1) {
				if (item->GetSocket(0) == 1) {
					CAffect* pAffect = ch->FindAffect(AFFECT_EXPRING);
					item->SetSocket(0, 0);
					item->SetSocket(2, 1);
					item->Lock(false);
					if (pAffect) {
						ch->RemoveAffect(pAffect);
					}
				}
			}
#endif

			totalSize += item->GetSize();
			collectItems.emplace_back(item);
		}
	}

	//@fixme490
	if (ch->IsDead() || ch->IsStun()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "CANNOT_SORT_IF_PC_IS_DEAD_OR_STUNNED");
		return;
	}

	if (ch->GetExchange() || ch->GetMyShop() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetShopOwner() || 
#endif
		ch->IsOpenSafebox() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| ch->IsOpenGuildstorage()
#endif
	) {
		ch->ChatPacket(CHAT_TYPE_INFO, "CANNOT_SORT_IF_OTHER_WINDOWS_ARE_OPENED");
		return;
	}
	//@end_fixme490

	if (totalSize - 3 >= INVENTORY_MAX_NUM)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("INVENTORY_FULL_CANNOT_SORT"));
		return;
	}

	for (std::vector<CItem*>::iterator it = collectItems.begin(); it != collectItems.end(); ++it)
	{
		LPITEM item = *it;
		item->RemoveFromCharacter();
	}

	std::sort(collectItems.begin(), collectItems.end(), sortByType);
	std::sort(collectItems.begin(), collectItems.end(), sortBySubType);
	std::sort(collectItems.begin(), collectItems.end(), sortByVnum);
	std::sort(collectItems.begin(), collectItems.end(), sortBySocket);

	for (std::vector<CItem*>::iterator iit = collectItems.begin(); iit < collectItems.end(); ++iit)
	{
		LPITEM sortedItem = *iit;
		if (sortedItem)
		{
#ifdef ENABLE_ANTI_EXP_RING
			if ((sortedItem->GetVnum() == ITEM_AUTO_EXP_RING_S || sortedItem->GetVnum() == ITEM_AUTO_EXP_RING_M || sortedItem->GetVnum() == ITEM_AUTO_EXP_RING_G) && sortedItem->GetSocket(2) == 1) {
				sortedItem->SetSocket(0, 1);
				sortedItem->SetSocket(2, 0);
				sortedItem->Lock(true);
			}
#endif

			uint32_t dwCount = sortedItem->GetCount();

			if (sortedItem->IsStackable() && !IS_SET(sortedItem->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item2 = ch->GetInventoryItem(i);
					if (!item2)
						continue;

					if (item2->GetVnum() == sortedItem->GetVnum())
					{
						int j;

						for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
						{
							if (item2->GetSocket(j) != sortedItem->GetSocket(j))
								break;
						}

						if (j != ITEM_SOCKET_MAX_NUM)
						{
							continue;
						}

						uint32_t dwCount2 = MIN(g_bItemCountLimit - item2->GetCount(), dwCount);
						dwCount -= dwCount2;

						item2->SetCount(item2->GetCount() + dwCount2);

						if (dwCount == 0)
						{
							M2_DESTROY_ITEM(sortedItem);
							break;
						}
						else
						{
							sortedItem->SetCount(dwCount);
						}
					}
				}
			}

			if (dwCount > 0)
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				int cell = ch->GetEmptyInventory(sortedItem);
#else
				int cell = ch->GetEmptyInventory(sortedItem->GetSize());
#endif
				sortedItem->AddToCharacter(ch, TItemPos(INVENTORY, cell));
		}
	}
}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SUCCESFULLY_SORTED_INVENTORY"));
	ch->SetInventorySortPulse(thecore_pulse());	//@custom038
}

#ifdef ENABLE_SPECIAL_INVENTORY
ACMD(do_sort_special_inventory)
{
	if (!ch)
		return;

	if (!ch->CanHandleItem())
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	const uint8_t specialInventoryType = atoi(arg1);
	if (specialInventoryType < 0 || specialInventoryType > 2)
		return;

	// Wait Time
	char sort_wait_time[1024];
	snprintf(sort_wait_time, sizeof(sort_wait_time), "special_inventory_sort.time_to_wait_%d", specialInventoryType);

	const time_t waitTime = (ch->GetQuestFlag(sort_wait_time) + (60)); // 1 min
	if (waitTime > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
		return;
	}

	ch->SetQuestFlag(sort_wait_time, get_global_time());
	// Wait Time

	std::vector<CItem*> collectItems;

	const int startIndex = SPECIAL_INVENTORY_SLOT_START + (SPECIAL_INVENTORY_PAGE_SIZE * INVENTORY_PAGE_COUNT) * specialInventoryType;
	const int endIndex = SPECIAL_INVENTORY_SLOT_START + (SPECIAL_INVENTORY_PAGE_SIZE * INVENTORY_PAGE_COUNT) * (specialInventoryType + 1);

	for (int i = startIndex; i < endIndex; ++i)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item)
			collectItems.emplace_back(item);
	}

	for (std::vector<CItem*>::iterator it = collectItems.begin(); it != collectItems.end(); ++it)
	{
		LPITEM item = *it;
		if (item)
			item->RemoveFromCharacter();
	}

	std::sort(collectItems.begin(), collectItems.end(), sortByType);
	std::sort(collectItems.begin(), collectItems.end(), sortBySubType);
	std::sort(collectItems.begin(), collectItems.end(), sortByVnum);
	std::sort(collectItems.begin(), collectItems.end(), sortBySocket);

	for (std::vector<CItem*>::iterator iit = collectItems.begin(); iit < collectItems.end(); ++iit)
	{
		LPITEM sortedItem = *iit;
		if (sortedItem)
		{
			uint32_t dwCount = sortedItem->GetCount();

			if (sortedItem->IsStackable() && !IS_SET(sortedItem->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				for (int i = startIndex; i < endIndex; ++i)
				{
					LPITEM item2 = ch->GetInventoryItem(i);
					if (!item2)
						continue;

					if (item2->GetVnum() == sortedItem->GetVnum())
					{
						int j;

						for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
						{
							if (item2->GetSocket(j) != sortedItem->GetSocket(j))
								break;
						}

						if (j != ITEM_SOCKET_MAX_NUM)
							continue;

						const uint32_t dwCount2 = MIN(g_bItemCountLimit - item2->GetCount(), dwCount);
						dwCount -= dwCount2;

						item2->SetCount(item2->GetCount() + dwCount2);

						if (dwCount == 0)
						{
							M2_DESTROY_ITEM(sortedItem);
							break;
						}
						else
						{
							sortedItem->SetCount(dwCount);
						}
					}
				}
			}

			if (dwCount > 0)
			{
				const int cell = ch->GetEmptyInventory(sortedItem);
				sortedItem->AddToCharacter(ch, TItemPos(INVENTORY, cell));
			}
		}
	}
}
#endif
#endif

ACMD(do_skillup2)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	int iCount = 1;

	if (*arg2)
	{
		str_to_number(iCount, arg2);
		iCount = MINMAX(1, iCount, 20);	//Countlimit: 1-20
	}

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);

	if (ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum, iCount);
	}
	else
	{
		switch (vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
				ch->SkillLevelUp(vnum, iCount);
				break;
		}
	}
}

ACMD(do_switchbot)
{
#ifdef ENABLE_SWITCHBOT_FOR_VIP
	if (ch->GetGMLevel() != GM_VIP) {
		ch->ChatPacket(CHAT_TYPE_INFO, "NO_VIP_PERMISSIONS");
		return;
	}

	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSwitchbot");
	/*	if (ch->GetGMLevel() == GM_VIP) {
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSwitchbot");
		}
		else {
			ch->ChatPacket(CHAT_TYPE_INFO, "NO_VIP_PERMISSIONS");
		}*/
#else
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSwitchbot");
#endif
}

#ifdef ENABLE_MINI_GAME_BNW
ACMD(do_bnw_event)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int iCommand = 1;

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: bnw_event <command>");
		ch->ChatPacket(CHAT_TYPE_INFO, "	0 = Disable.");
		ch->ChatPacket(CHAT_TYPE_INFO, "	1 = Enable.");
		return;
	}

	if (isnhdigit(*arg1))
		str_to_number(iCommand, arg1);

	if (iCommand == 1)
	{
		if (quest::CQuestManager::instance().GetEventFlag("mini_game_bnw_event") == 0)
			quest::CQuestManager::instance().RequestSetEventFlag("mini_game_bnw_event", 1);
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "This event is already opened.");
	}
	else
		quest::CQuestManager::instance().RequestSetEventFlag("mini_game_bnw_event", 0);
}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
ACMD(do_find_m_event)
{
	char arg1[256], arg2[256];
	int iCommand = 1;
	int iDays = 1;
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: find_m_event <command> <days_number>");
		ch->ChatPacket(CHAT_TYPE_INFO, "	0 = Disable.");
		ch->ChatPacket(CHAT_TYPE_INFO, "	1 = Enable.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<days_number> Is the number of days that event is on.");
		return;
	}

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(iCommand, arg1);
		str_to_number(iDays, arg2);
	}

	if (iDays <= 0)
		return;

	if (iCommand == 1)
	{
		if (quest::CQuestManager::instance().GetEventFlag("mini_game_findm_event") == 0)
		{
			int iEndTime = time(0) + 60 * 60 * 24 * iDays;

			quest::CQuestManager::instance().RequestSetEventFlag("findm_event_end_day", iEndTime);

			quest::CQuestManager::instance().RequestSetEventFlag("mini_game_findm_event", 1);
			quest::CQuestManager::instance().RequestSetEventFlag("enable_find_m_event_drop", 1);

			ch->ChatPacket(CHAT_TYPE_COMMAND, "is_active_event");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "This event is already opened.");
		}
	}
	else
	{
		quest::CQuestManager::instance().RequestSetEventFlag("mini_game_findm_event", 0);
		quest::CQuestManager::instance().RequestSetEventFlag("enable_find_m_event_drop", 0);
		ch->ChatPacket(CHAT_TYPE_INFO, "You deactivated find monster event.");
	}
}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
ACMD(do_yut_nori_event)
{
	char arg1[256], arg2[256];
	int iCommand = 1;
	int iDays = 1;
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: yut_nori_event <command> <days_number>");
		ch->ChatPacket(CHAT_TYPE_INFO, "	0 = Disable.");
		ch->ChatPacket(CHAT_TYPE_INFO, "	1 = Enable.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<days_number> Is the number of days that event is on.");
		return;
	}

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(iCommand, arg1);
		str_to_number(iDays, arg2);
	}

	if (iDays <= 0)
		return;

	if (iCommand == 1)
	{
		if (quest::CQuestManager::instance().GetEventFlag("mini_game_yutnori_event") == 0)
		{
			int iEndTime = time(0) + 60 * 60 * 24 * iDays;

			quest::CQuestManager::instance().RequestSetEventFlag("yut_nori_event_end_day", iEndTime);

			quest::CQuestManager::instance().RequestSetEventFlag("mini_game_yutnori_event", 1);
			quest::CQuestManager::instance().RequestSetEventFlag("enable_yut_nori_event_drop", 1);

			//quest::CQuestManager::instance().RequestSetEventFlag("enable_yut_nori_event_get_reward", 0);
			quest::CQuestManager::instance().RequestSetEventFlag("yut_nori_event_get_reward_end_day", 0);

			SendNotice("Yut nori event is now active. Relog and check the icon beside minimap.");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "This event is already opened.");
		}
	}
	else
	{
		quest::CQuestManager::instance().RequestSetEventFlag("mini_game_yutnori_event", 0);
		quest::CQuestManager::instance().RequestSetEventFlag("enable_yut_nori_event_drop", 0);
		ch->ChatPacket(CHAT_TYPE_INFO, "You deactivated yut nori event.");
	}
}
#endif

#ifdef ENABLE_AUTO_SYSTEM
ACMD(do_autohunt)
{
	/*if (quest::CQuestManager::instance().GetEventFlag("DISABLE_AUTO_HUNT") == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "AutoHunt Disabled");
		return;
	}*/

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	switch (LOWER(arg1[0]))
	{
		case 'b':
		{
			if (ch->IsRiding() || ch->GetHorse())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Mount unsummon!");
			}
			else
			{
				if (ch->GetPremiumRemainSeconds(PREMIUM_AUTO_USE) > 0)
				{
					if (!ch->IsAffectFlag(AFF_AUTO_USE)) {
						ch->AddAffect(AFFECT_AUTO, POINT_NONE, 0, AFF_AUTO_USE, INFINITE_AFFECT_DURATION, 0, false);
					}
				}
			}
		}
		break;

		case 'd':
		{
			if (ch->IsAffectFlag(AFF_AUTO_USE))
				ch->RemoveAffect(AFFECT_AUTO);
		}
		break;

		default:
			break;
	}
}
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
ACMD(do_additional_equipment)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
		return;

	int page_index = atoi(arg1);

	if (page_index <= 0 || page_index > NEW_EQUIPMENT_MAX_PAGE)
		return;

	if ((page_index - 1) > ch->GetPageTotalEquipment())
		return;

	if (!ch->CanHandleItem())
		return;

	if (ch->IsDead())
		return;

	if (ch->IsStun())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetShopOwner() ||
#endif
		ch->IsOpenSafebox() || ch->IsCubeOpen())
		return;

	uint32_t dwCurTime = get_dword_time();

	if (dwCurTime - ch->GetLastAttackTime() <= 1500 || dwCurTime - ch->GetLastSkillTime() <= 1500)
		return;

	int get_time_change_equip = ch->GetTimeChangeEquip();
	int currentPulse = thecore_pulse();

	if (get_time_change_equip > currentPulse)
	{
		int deltaInSeconds = ((get_time_change_equip / PASSES_PER_SEC(1)) - (currentPulse / PASSES_PER_SEC(1)));
		int minutes = deltaInSeconds / 60;
		int seconds = (deltaInSeconds - (minutes * 60));

		ch->ChatPacket(CHAT_TYPE_INFO, "you have to wait %02d seconds to change equip .", seconds);
		return;
	}

	LPITEM item_change_equip;
	LPITEM item_inv;

	item_inv = ch->GetWearDefault(WEAR_COSTUME_WEAPON);
	if (item_inv)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Este cambio de equipo no se puede realizar, tiene que desequipar la skin de arma.");
		return;
	}

#ifdef ENABLE_PET_SYSTEM
	if (ch->GetWearDefault(WEAR_PET))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Unsummon your Pet first!");
		return;
	}
#endif

	if (ch->GetWearDefault(WEAR_COSTUME_MOUNT))
	{
		ch->Unmount(ch);
	}

	if (page_index == 1 && ch->GetPageEquipment() == 1)
	{
		for (int i = 0; i < ADDITIONAL_EQUIPMENT_MAX_SLOT; ++i)
		{
			uint8_t cell = i;
			if (ch->GetWearNotChange(cell))
			{
				item_change_equip = ch->GetAdditionalEquipmentItem(i);
				if (item_change_equip)
				{
					item_change_equip->RefreshAdditionalEquipmentItems(ch, cell, false);
				}
			}
		}

		ch->PointChange(POINT_NEW_EQUIPMENT_ACTUAL, 0);

		for (int i = 0; i < ADDITIONAL_EQUIPMENT_MAX_SLOT; ++i)
		{
			uint8_t cell = i;
			if (ch->GetWearNotChange(cell))
			{
				item_inv = ch->GetWearDefault(cell);
				if (item_inv)
				{
					item_inv->RefreshAdditionalEquipmentItems(ch, cell, true);
				}
			}
		}
	}
	else if (page_index == 2 && ch->GetPageEquipment() == 0)
	{
		for (int i = 0; i < ADDITIONAL_EQUIPMENT_MAX_SLOT; ++i)
		{
			uint8_t cell = i;
			if (ch->GetWearNotChange(cell))
			{
				item_inv = ch->GetWearDefault(cell);
				if (item_inv)
				{
					item_inv->RefreshAdditionalEquipmentItems(ch, cell, false);
				}
			}
		}

		ch->PointChange(POINT_NEW_EQUIPMENT_ACTUAL, 1);

		for (int i = 0; i < ADDITIONAL_EQUIPMENT_MAX_SLOT; ++i)
		{
			uint8_t cell = i;

			if (ch->GetWearNotChange(cell))
			{
				item_change_equip = ch->GetAdditionalEquipmentItem(i);
				if (item_change_equip)
				{
					item_change_equip->RefreshAdditionalEquipmentItems(ch, cell, true);
				}
			}
		}
	}

#ifdef ENABLE_SET_ITEM
	ch->RefreshSetBonus();
#endif
	ch->UpdatePacket();
	ch->SetTimeChangeEquip(thecore_pulse() + PASSES_PER_SEC(3));
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
ACMD(do_multi_farm)
{
	if (!ch->GetDesc())
		return;

	if (ch->GetProtectTime("multi-farm") > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You need be slow! You can try after %d second.", ch->GetProtectTime("multi-farm") - get_global_time());
		return;
	}

	ch->SetProtectTime("multi-farm", get_global_time() + 10);
	CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(ch->GetDesc()->GetHostName(), ch->GetPlayerID(), ch->GetName(), !ch->GetMultiStatus());
}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
ACMD(do_lottery_open)
{
	if (!ch || !ch->GetDesc())
		return;

	ch->OpenLottoWindow();
}
#endif

#ifdef ENABLE_AUTO_RESTART_EVENT
ACMD(do_auto_restart)
{
	if (!ch)
	{
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t type;
	str_to_number(type, arg1);

	if (type)
	{
		ch->autohunt_restart = true;
	}
	else
	{
		ch->autohunt_restart = false;
	}
}
#endif

#ifdef ENABLE_WORLD_BOSS
ACMD(do_get_wb_reward)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->GotWBRewards() == 1)
		return;

	if (ch->GetTier() == 0)
		return;

	std::vector<int> WB_Tier_1_Items = { 19, 29, 39 };
	std::vector<int> WB_Tier_2_Items = { 19, 29, 39 };
	std::vector<int> WB_Tier_3_Items = { 19, 29, 39 };
	std::vector<int> WB_Tier_4_Items = { 19, 29, 39 };

	if (ch->GetTier() == 1)
	{
		for (const auto& item : WB_Tier_1_Items)
		{
			LPITEM pReward = ITEM_MANAGER::Instance().CreateItem(item, 1, 0, false);
			if (pReward)
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				const int pos = ch->GetEmptyInventory(pReward);
#else
				const int pos = ch->GetEmptyInventory(pReward->bSize);
#endif

				if (pos == -1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "CANNOT GET");
					M2_DELETE(pReward);
					return;
				}

				ch->AutoGiveItem(pReward, 1);
			}
		}
	}
	if (ch->GetTier() == 2)
	{
		for (const auto& item : WB_Tier_2_Items)
		{
			LPITEM pReward = ITEM_MANAGER::Instance().CreateItem(item, 1, 0, false);
			if (pReward)
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				const int pos = ch->GetEmptyInventory(pReward);
#else
				const int pos = ch->GetEmptyInventory(pReward->bSize);
#endif

				if (pos == -1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "CANNOT GET");
					M2_DELETE(pReward);
					return;
				}

				ch->AutoGiveItem(pReward, 1);
			}
		}
	}
	if (ch->GetTier() == 3)
	{
		for (const auto& item : WB_Tier_3_Items)
		{
			LPITEM pReward = ITEM_MANAGER::Instance().CreateItem(item, 1, 0, false);
			if (pReward)
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				const int pos = ch->GetEmptyInventory(pReward);
#else
				const int pos = ch->GetEmptyInventory(pReward->bSize);
#endif

				if (pos == -1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "CANNOT GET");
					M2_DELETE(pReward);
					return;
				}

				ch->AutoGiveItem(pReward, 1);
			}
		}
	}
	if (ch->GetTier() == 4)
	{
		for (const auto& item : WB_Tier_4_Items)
		{
			LPITEM pReward = ITEM_MANAGER::Instance().CreateItem(item, 1, 0, false);
			if (pReward)
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				const int pos = ch->GetEmptyInventory(pReward);
#else
				const int pos = ch->GetEmptyInventory(pReward->bSize);
#endif

				if (pos == -1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "CANNOT GET");
					M2_DELETE(pReward);
					return;
				}

				ch->AutoGiveItem(pReward, 1);
			}
		}
	}

	ch->SetWBRewards(true);
}
#endif
