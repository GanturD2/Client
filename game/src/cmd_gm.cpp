#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "mob_manager.h"
#include "packet.h"
#include "cmd.h"
#include "regen.h"
#include "guild.h"
#include "guild_manager.h"
#include "p2p.h"
#include "buffer_manager.h"
#include "fishing.h"
#include "mining.h"
#include "questmanager.h"
#include "vector.h"
#include "affect.h"
#include "db.h"
#include "priv_manager.h"
#include "building.h"
#include "battle.h"
#include "arena.h"
#include "start_position.h"
#include "party.h"
#include "monarch.h"
#include "castle.h"
#include "BattleArena.h"
#include "xmas_event.h"
#include "log.h"
#include "threeway_war.h"
#include "unique_item.h"
#include "DragonSoul.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_PET_SYSTEM
	#include "PetSystem.h"
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	#include "shop.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_CUBE_RENEWAL
#	include "CubeManager.h"
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "DungeonInfo.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "AchievementSystem.h"
#endif

extern bool ban_force_reason;
extern bool DropEvent_RefineBox_SetValue(const std::string& name, int value);

// ADD_COMMAND_SLOW_STUN
enum
{
	COMMANDAFFECT_STUN,
	COMMANDAFFECT_SLOW,
};

void Command_ApplyAffect(LPCHARACTER ch, const char* argument, const char* affectName, int cmdAffect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	sys_log(0, arg1);

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: %s <name>", affectName);
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s is not in same map", arg1);
		return;
	}

	switch (cmdAffect)
	{
		case COMMANDAFFECT_STUN:
			SkillAttackAffect(tch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, 30, "GM_STUN");
			break;
		case COMMANDAFFECT_SLOW:
			SkillAttackAffect(tch, 1000, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, 30, "GM_SLOW");
			break;
	}

	sys_log(0, "%s %s", arg1, affectName);

	ch->ChatPacket(CHAT_TYPE_INFO, "%s %s", arg1, affectName);
}
// END_OF_ADD_COMMAND_SLOW_STUN

ACMD(do_stun)
{
	Command_ApplyAffect(ch, argument, "stun", COMMANDAFFECT_STUN);
}

ACMD(do_slow)
{
	Command_ApplyAffect(ch, argument, "slow", COMMANDAFFECT_SLOW);
}

struct log_packet_func
{
	const char* m_str;

	log_packet_func(const char* str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetGMLevel() > GM_PLAYER)
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

void SendLog(const char* c_pszBuf)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), log_packet_func(c_pszBuf));
}

//*~~|| Notice ||~~*//
struct notice_packet_func
{
	const char* m_str;
	bool m_bBigFont;
	notice_packet_func(const char* str, bool bBigFont = false) : m_str(str), m_bBigFont(bBigFont) {}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		d->GetCharacter()->ChatPacket((m_bBigFont) ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

void SendNotice(const char* c_pszBuf, bool bBigFont)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func(c_pszBuf, bBigFont));
}
//*~~|| END Notice ||~~*//

//*~~|| NoticeMap ||~~*//
struct notice_map_packet_func
{
	const char* m_str;
	int m_mapIndex;
	bool m_bBigFont;

	notice_map_packet_func(const char* str, int idx, bool bBigFont) : m_str(str), m_mapIndex(idx), m_bBigFont(bBigFont)
	{
	}

	void operator() (LPDESC d)
	{
		if (d->GetCharacter() == nullptr) return;
		if (d->GetCharacter()->GetMapIndex() != m_mapIndex) return;

		d->GetCharacter()->ChatPacket(m_bBigFont ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

void SendNoticeMap(const char* c_pszBuf, int nMapIndex, bool bBigFont)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_map_packet_func(c_pszBuf, nMapIndex, bBigFont));
}
//*~~|| END NoticeMap ||~~*//

//*~~|| Notice Ox ||~~*//
#ifdef ENABLE_OX_RENEWAL
struct notice_ox_map_packet_func
{
	const char* m_str;
	int m_mapIndex;
	notice_ox_map_packet_func(const char* str, int idx) : m_str(str), m_mapIndex(idx)
	{
	}

	void operator() (LPDESC d)
	{
		if (d->GetCharacter() == nullptr) return;
		if (d->GetCharacter()->GetMapIndex() != m_mapIndex) return;

		d->GetCharacter()->ChatPacket(CHAT_TYPE_CONTROL_NOTICE, "%s", m_str);
	}
};

void SendNoticeOxMap(const char* c_pszBuf, int nMapIndex)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_ox_map_packet_func(c_pszBuf, nMapIndex));
}
#endif
//*~~|| END Notice Ox ||~~*//

//*~~|| Broadcast Notice ||~~*//
void BroadcastNotice(const char* c_pszBuf, bool bBigFont)
{
	TPacketGGNotice p{};
	p.bHeader = (bBigFont) ? HEADER_GG_BIG_NOTICE : HEADER_GG_NOTICE;
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size()); // HEADER_GG_NOTICE

	SendNotice(c_pszBuf, bBigFont);
}
//*~~|| END Broadcast Notice ||~~*//

//*~~|| Notice Monarch ||~~*//
struct monarch_notice_packet_func
{
	const char* m_str;
	uint8_t m_bEmpire;

	monarch_notice_packet_func(uint8_t bEmpire, const char* str) : m_str(str), m_bEmpire(bEmpire)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (m_bEmpire == d->GetCharacter()->GetEmpire())
		{
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
		}
	}
};

void SendMonarchNotice(uint8_t bEmpire, const char* c_pszBuf)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), monarch_notice_packet_func(bEmpire, c_pszBuf));
}

void BroadcastMonarchNotice(uint8_t bEmpire, const char* c_pszBuf)
{
	TPacketGGMonarchNotice p{};
	p.bHeader = HEADER_GG_MONARCH_NOTICE;
	p.bEmpire = bEmpire;
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size());

	SendMonarchNotice(bEmpire, c_pszBuf);
}
//*~~|| END Notice Monarch ||~~*//

//*~~|| BigNotice ||~~*//
struct big_notice_packet_func
{
	const char* m_str;
	int mapIndex;

	big_notice_packet_func(const char* str, int mapidx) : m_str(str), mapIndex(mapidx)
	{
	}

	void operator () (LPDESC d)
	{
		LPCHARACTER ch = d->GetCharacter();
		if (!ch)
			return;

		if (mapIndex == -1 || ch->GetMapIndex() == mapIndex)
			ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", m_str);
	}
};

void SendBigNotice(const char* c_pszBuf, int mapIndex)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), big_notice_packet_func(c_pszBuf, mapIndex));
}

void BroadcastBigNotice(const char* c_pszBuf, int mapIndex)
{
	if (mapIndex == -1) //Send the notice through map & cores
	{
		TPacketGGBigNotice p{};
		p.bHeader = HEADER_GG_BIG_NOTICE;
		p.lSize = strlen(c_pszBuf) + 1;

		TEMP_BUFFER buf;
		buf.write(&p, sizeof(p));
		buf.write(c_pszBuf, p.lSize);

		P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size());
	}

	SendBigNotice(c_pszBuf, mapIndex);
}
//*~~|| END BigNotice ||~~*//

#ifdef ENABLE_BATTLE_FIELD
struct command_packet_func
{
	const char* m_str;
	command_packet_func(const char* str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;
		d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "%s", m_str);
	}
};

void SendCommand(const char* c_pszBuf)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), command_packet_func(c_pszBuf));
}

void BroadcastCommand(const char* c_pszBuf, ...)
{
	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, c_pszBuf);
	vsnprintf(chatbuf, sizeof(chatbuf), c_pszBuf, args);
	va_end(args);

	TPacketGGCommand p{};
	p.bHeader = HEADER_GG_COMMAND;
	p.lSize = strlen(chatbuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(chatbuf, p.lSize);

	P2P_MANAGER::Instance().Send(buf.read_peek(), buf.size());

	SendCommand(chatbuf);
}
#endif

#ifdef ENABLE_EVENT_MANAGER
struct send_event_begin_notification
{
	void operator() (LPDESC d)
	{
		if (!d)
			return;

		if (!d->GetCharacter())
			return;

		quest::CQuestManager::Instance().EventBegin(d->GetCharacter()->GetPlayerID());
	}
};

struct send_event_end_notification
{
	void operator() (LPDESC d)
	{
		if (!d)
			return;

		if (!d->GetCharacter())
			return;

		quest::CQuestManager::Instance().EventEnd(d->GetCharacter()->GetPlayerID());
	}
};

void SendEventBeginNotification()
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), send_event_begin_notification());
}

void SendEventEndNotification()
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), send_event_end_notification());
}

struct send_event_reload
{
	void operator() (LPDESC d)
	{
		if (!d)
			return;

		if (!d->GetCharacter())
			return;

		TPacketGCEventReload p{};
		p.bHeader = HEADER_GC_EVENT_RELOAD;

		d->Packet(&p, sizeof(p));
	}
};

void BroadcastEventReload()
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), send_event_reload());
}

void ReloadEvent()
{
	TPacketGGReloadEvent p{};
	p.bHeader = HEADER_GG_EVENT_RELOAD;

	P2P_MANAGER::Instance().Send(&p, sizeof(p));

	BroadcastEventReload();
}
#endif

////////////

// LUA_ADD_GOTO_INFO
struct GotoInfo
{
	std::string 	st_name;

	uint8_t 	empire;
	int 	mapIndex;
	uint32_t 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

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
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

static std::vector<GotoInfo> gs_vec_gotoInfo;

void CHARACTER_AddGotoInfo(const std::string& c_st_name, uint8_t empire, int mapIndex, uint32_t x, uint32_t y)
{
	GotoInfo newGotoInfo;
	newGotoInfo.st_name = c_st_name;
	newGotoInfo.empire = empire;
	newGotoInfo.mapIndex = mapIndex;
	newGotoInfo.x = x;
	newGotoInfo.y = y;
	gs_vec_gotoInfo.emplace_back(newGotoInfo);

	sys_log(0, "AddGotoInfo(name=%s, empire=%d, mapIndex=%d, pos=(%d, %d))", c_st_name.c_str(), empire, mapIndex, x, y);
}

bool FindInString(const char * c_pszFind, const char * c_pszIn)
{
	const char * c = c_pszIn;
	const char * p;

	p = strchr(c, '|');

	if (!p)
		return (0 == strncasecmp(c_pszFind, c_pszIn, strlen(c_pszFind)));
	else
	{
		char sz[64 + 1];

		do
		{
			strlcpy(sz, c, MIN(sizeof(sz), (p - c) + 1));

			if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
				return true;

			c = p + 1;
		} while ((p = strchr(c, '|')));

		strlcpy(sz, c, sizeof(sz));

		if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
			return true;
	}

	return false;
}

bool CHARACTER_GoToName(LPCHARACTER ch, uint8_t empire, int mapIndex, const char* gotoName)
{
	std::vector<GotoInfo>::iterator i;
	for (i = gs_vec_gotoInfo.begin(); i != gs_vec_gotoInfo.end(); ++i)
	{
		const GotoInfo& c_eachGotoInfo = *i;

		if (mapIndex != 0)
		{
			if (mapIndex != c_eachGotoInfo.mapIndex)
				continue;
		}
		else if (!FindInString(gotoName, c_eachGotoInfo.st_name.c_str()))
			continue;

		if (c_eachGotoInfo.empire == 0 || c_eachGotoInfo.empire == empire)
		{
			int x = c_eachGotoInfo.x * 100;
			int y = c_eachGotoInfo.y * 100;

			ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
			ch->WarpSet(x, y);
			ch->Stop();
			return true;
		}
	}
	return false;
}
// END_OF_LUA_ADD_GOTO_INFO

ACMD(do_transfer)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: transfer <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);
	if (!tch)
	{
		CCI* pkCCI = P2P_MANAGER::Instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
				return;
			}

			TPacketGGTransfer pgg{};

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::Instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, "Transfer requested.");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no character(%s) by that name", arg1);

		return;
	}

	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Transfer me?!?");
		return;
	}

	//tch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());
}

ACMD(do_goto)
{
	char arg1[256], arg2[256];
	int x = 0, y = 0, z = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto <x meter> <y meter>");
		return;
	}

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);

		PIXEL_POSITION p;

		if (SECTREE_MANAGER::Instance().GetMapBasePosition(ch->GetX(), ch->GetY(), p))
		{
			x += p.x / 100;
			y += p.y / 100;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "You goto ( %d, %d )", x, y);
	}
	else
	{
		int mapIndex = 0;
		uint8_t empire = 0;

		if (*arg1 == '#')
			str_to_number(mapIndex,  (arg1 + 1));

		if (*arg2 && isnhdigit(*arg2))
		{
			str_to_number(empire, arg2);
			empire = MINMAX(1, empire, 3);
		}
		else
			empire = ch->GetEmpire();

		if (CHARACTER_GoToName(ch, empire, mapIndex, arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map command syntax: /goto <mapname> [empire]");
			return;
		}

		return;

		/*
		   int iMapIndex = 0;
		   for (int i = 0; aWarpInfo[i].c_pszName != nullptr; ++i)
		   {
		   if (iMapIndex != 0)
		   {
		   if (iMapIndex != aWarpInfo[i].iMapIndex)
		   continue;
		   }
		   else if (!FindInString(arg1, aWarpInfo[i].c_pszName))
		   continue;

		   if (aWarpInfo[i].bEmpire == 0 || aWarpInfo[i].bEmpire == bEmpire)
		   {
		   x = aWarpInfo[i].x * 100;
		   y = aWarpInfo[i].y * 100;

		   ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
		   ch->WarpSet(x, y);
		   ch->Stop();
		   return;
		   }
		   }
		 */

	}

	x *= 100;
	y *= 100;

	ch->Show(ch->GetMapIndex(), x, y, z);
	ch->Stop();
}

ACMD(do_warp)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: warp <character name> | <x meter> <y meter>");
		return;
	}

	int x = 0, y = 0;
#ifdef ENABLE_CMD_WARP_IN_DUNGEON
	int mapIndex = 0;
#endif

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);
	}
	else
	{
		LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

		if (nullptr == tch)
		{
			const CCI* pkCCI = P2P_MANAGER::Instance().Find(arg1);

			if (nullptr != pkCCI)
			{
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
					return;
				}

				ch->WarpToPID( pkCCI->dwPID );
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "There is no one(%s) by that name", arg1);
			}

			return;
		}
		else
		{
			x = tch->GetX() / 100;
			y = tch->GetY() / 100;
#ifdef ENABLE_CMD_WARP_IN_DUNGEON
			mapIndex = tch->GetMapIndex();
#endif
		}
	}

	x *= 100;
	y *= 100;

#ifdef ENABLE_CMD_WARP_IN_DUNGEON
	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d, %d )", x, y, mapIndex);
	ch->WarpSet(x, y, mapIndex);
#else
	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
	ch->WarpSet(x, y);
#endif
	ch->Stop();
}

#ifdef ENABLE_NEWSTUFF
ACMD(do_rewarp)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", ch->GetX(), ch->GetY());
	ch->WarpSet(ch->GetX(), ch->GetY());
	ch->Stop();
}
#endif

ACMD(do_check_flag)
{
	if(ch->GetInventoryItem(0))
	{
		LPITEM item = ch->GetInventoryItem(0);
		ch->ChatPacket(7, "Item %d ITEM_FLAG_CONFIRM_WHEN_USE = %d", item->GetVnum(), (item->GetFlag() & ITEM_FLAG_CONFIRM_WHEN_USE ? 1 : 0));
	}
}

ACMD(do_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: item <item vnum>");
		return;
	}

	int iCount = 1;

	if (*arg2)
	{
		str_to_number(iCount, arg2);
		//iCount = MINMAX(1, iCount, g_bItemCountLimit);
		iCount = MAX(1, abs(iCount));
	}

	uint32_t dwVnum;

	if (isnhdigit(*arg1))
		str_to_number(dwVnum, arg1);
	else
	{
		if (!ITEM_MANAGER::Instance().GetVnum(arg1, dwVnum))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
			return;
		}
	}

	ITEM_MANAGER::Instance().CreateItem(dwVnum, iCount, 0, true);

	while (iCount > 0) {
		const auto& item = ITEM_MANAGER::Instance().CreateItem(dwVnum, iCount, 0, true);
		if (!item) {
			ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum.", dwVnum);
			return;
		}

#ifdef ENABLE_SPECIAL_INVENTORY
		const auto iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item);
#else
		const auto iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item->GetSize());
#endif
		if (iEmptyPos == -1) {
			ch->ChatPacket(CHAT_TYPE_INFO, "Not enough %s space.", item->IsDragonSoul() ? "dragonsoul" : "inventory");
			ITEM_MANAGER::Instance().DestroyItem(item);
			return;
		}

		item->AddToCharacter(ch, TItemPos(item->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iEmptyPos));
		LogManager::Instance().ItemLog(ch, item, "GM", item->GetName());
		iCount -= item->IsStackable() ? g_bItemCountLimit : 1;
	}
}

ACMD(do_give_item)
{
	char arg1[256], arg2[256], arg3[256];

	const char* szName;
	uint32_t dwVnum;
	int iCount;

	one_argument(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	szName = arg1;
	dwVnum = atoi(arg2);
	iCount = atoi(arg3);

	LPCHARACTER pkTargetPlayer = CHARACTER_MANAGER::Instance().FindPC(szName);

	// Syntax Error
	if (!* arg1 || !*arg2 || !* arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Invilid Syntax, usage: <playername> <id> <count>");
		return;
	}

	if (iCount <= 0)
	{
		return;
	}

	// Find Player error
	if (!pkTargetPlayer)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Player %s was not found", szName);
		return;
	}

	pkTargetPlayer->AutoGiveItem(dwVnum, iCount);
	ch->ChatPacket(CHAT_TYPE_INFO, "You have %s ein Item mit der ID %d und %dx gegeben.", szName, dwVnum, iCount);
}

ACMD(do_group_random)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: grrandom <group vnum>");
		return;
	}

	uint32_t dwVnum = 0;
	str_to_number(dwVnum, arg1);
	CHARACTER_MANAGER::Instance().SpawnGroupGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: group <group vnum>");
		return;
	}

	uint32_t dwVnum = 0;
	str_to_number(dwVnum, arg1);

	if (test_server)
		sys_log(0, "COMMAND GROUP SPAWN %u at %u %u %u", dwVnum, ch->GetMapIndex(), ch->GetX(), ch->GetY());

	CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_mob_coward)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mc <vnum>");
		return;
	}

	const CMob * pkMob;

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

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::Instance().SpawnMobRange(vnum,
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750),
				ch->GetY() - number(200, 750),
				ch->GetX() + number(200, 750),
				ch->GetY() + number(200, 750),
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetCoward();
	}
}

ACMD(do_mob_map)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: mm <vnum>");
		return;
	}

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRandomPosition(vnum, ch->GetMapIndex());

	if (tch)
		ch->ChatPacket(CHAT_TYPE_INFO, "%s spawned in %dx%d", tch->GetName(), tch->GetX(), tch->GetY());
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Spawn failed.");
}

ACMD(do_mob_aggresive)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob * pkMob;

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

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::Instance().SpawnMobRange(vnum,
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750),
				ch->GetY() - number(200, 750),
				ch->GetX() + number(200, 750),
				ch->GetY() + number(200, 750),
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetAggressive();
	}
}

ACMD(do_mob)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = nullptr;

	if (isnhdigit(*arg1))
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

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	if (test_server)
		iCount = MIN(40, iCount);
	else
		iCount = MIN(20, iCount);

	while (iCount--)
	{
		CHARACTER_MANAGER::Instance().SpawnMobRange(vnum,
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750),
				ch->GetY() - number(200, 750),
				ch->GetX() + number(200, 750),
				ch->GetY() + number(200, 750),
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
	}
}

ACMD(do_mob_ld)
{
	char	arg1[256], arg2[256], arg3[256], arg4[256];
	uint32_t	vnum = 0;

	two_arguments(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = nullptr;

	if (isnhdigit(*arg1))
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

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int dir = 1;
	long x=0,y=0;

	if (*arg2)
		str_to_number(x, arg2);
	if (*arg3)
		str_to_number(y, arg3);
	if (*arg4)
		str_to_number(dir, arg4);


	CHARACTER_MANAGER::Instance().SpawnMob(vnum,
		ch->GetMapIndex(),
		x*100,
		y*100,
		ch->GetZ(),
		pkMob->m_table.bType == CHAR_TYPE_STONE,
		dir);
}

struct FuncPurge
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncPurge(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)	// 10미터 이상에 있는 것들은 purge 하지 않는다.
			return;

		sys_log(0, "PURGE: %s %d", pkChr->GetName(), iDist);

		if (pkChr->IsNPC() && !pkChr->IsPet()
#ifdef ENABLE_GROWTH_PET_SYSTEM
			&& !pkChr->IsGrowthPet()
#endif
#ifdef ENABLE_DEFENSE_WAVE
			&& !pkChr->IsMast()
			&& !pkChr->IsShipSteeringWheel()
#endif
			&& !pkChr->IsHorse() && !pkChr->IsShop())	//@fixme539
		{
			M2_DESTROY_CHARACTER(pkChr);
		}
	}
};

ACMD(do_purge)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncPurge func(ch);

	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	LPSECTREE sectree = ch->GetSectree();
	if (sectree) // #431
		sectree->ForEachAround(func);
	else
		sys_err("PURGE_ERROR.NULL_SECTREE(mapIndex=%d, pos=(%d, %d)", ch->GetMapIndex(), ch->GetX(), ch->GetY());
}

ACMD(do_item_purge)
{
#ifdef ENABLE_CMD_IPURGE_EX
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: ipurge <window>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available windows:");
		ch->ChatPacket(CHAT_TYPE_INFO, " all");
		ch->ChatPacket(CHAT_TYPE_INFO, " inventory or inv");
		ch->ChatPacket(CHAT_TYPE_INFO, " equipment or equip");
		ch->ChatPacket(CHAT_TYPE_INFO, " dragonsoul or ds");
		ch->ChatPacket(CHAT_TYPE_INFO, " belt");
		return;
	}

	int         i;
	LPITEM      item;

#ifdef ENABLE_PET_SYSTEM	//@fixme464
	if (ch->GetPetSystem()->IsActivePet()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "You can't do this with pet summoned.");
		return;
	}
#endif

/*#ifdef ENABLE_GROWTH_PET_SYSTEM	//@fixme464
	if (ch->GetGrowthPetSystem()->IsActivePet()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "You can't do this with pet summoned.");
		return;
	}
#endif*/

	std::string strArg(arg1);
	if (!strArg.compare(0, 3, "all"))
	{
		for (i = 0; i < INVENTORY_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
		for (i = 0; i < EQUIPMENT_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetEquipmentItem(i)))
			{
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, QUICKSLOT_MAX_POS);
			}
		}
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 3, "inv"))
	{
		for (i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
	}
	else if (!strArg.compare(0, 5, "equip"))
	{
		for (i = 0; i < EQUIPMENT_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetEquipmentItem(i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, QUICKSLOT_MAX_POS);
			}
		}
	}
	else if (!strArg.compare(0, 6, "dragon") || !strArg.compare(0, 2, "ds"))
	{
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 4, "belt"))
	{
		for (i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetBeltInventoryItem(i)))
			{
				ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, QUICKSLOT_MAX_POS);
			}
		}
	}
#else
	int         i;
	LPITEM      item;

	for (i = 0; i < INVENTORY_SLOT_COUNT; ++i)
	{
		if ((item = ch->GetInventoryItem(i)))
		{
			ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
			ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
		}
	}
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
		{
			ITEM_MANAGER::Instance().RemoveItem(item, "PURGE");
		}
	}
#endif
}

ACMD(do_state)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::Instance().Find(strtoul(arg1+1, nullptr, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);

			if (!d)
				tch = nullptr;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

	if (!tch)
		return;

	char buf[256];

	snprintf(buf, sizeof(buf), "%s's State: ", tch->GetName());

	if (tch->IsPosition(POS_FIGHTING))
		strlcat(buf, "Battle", sizeof(buf));
	else if (tch->IsPosition(POS_DEAD))
		strlcat(buf, "Dead", sizeof(buf));
	else
		strlcat(buf, "Standing", sizeof(buf));

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (tch->GetViewingShop())
		strlcat(buf, ", Shop", sizeof(buf));
#else
	if (tch->GetShop()) //@fixme526
		strlcat(buf, ", Shop", sizeof(buf));
#endif

	if (tch->GetExchange()) //@fixme526
		strlcat(buf, ", Exchange", sizeof(buf));

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	int len;
	len = snprintf(buf, sizeof(buf), "Coordinate %ldx%ld (%ldx%ld)",
			tch->GetX(), tch->GetY(), tch->GetX() / 100, tch->GetY() / 100);

	if (len < 0 || len >= (int) sizeof(buf))
		len = sizeof(buf) - 1;

	LPSECTREE pSec = SECTREE_MANAGER::Instance().Get(tch->GetMapIndex(), tch->GetX(), tch->GetY());

	if (pSec)
	{
		TMapSetting& map_setting = SECTREE_MANAGER::Instance().GetMap(tch->GetMapIndex())->m_setting;
		snprintf(buf + len, sizeof(buf) - len, " MapIndex %ld Attribute %08X Local Position (%ld x %ld)",
			tch->GetMapIndex(), pSec->GetAttribute(tch->GetX(), tch->GetY()), (tch->GetX() - map_setting.iBaseX)/100, (tch->GetY() - map_setting.iBaseY)/100);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket(CHAT_TYPE_INFO, "LEV %d", tch->GetLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "HP %d/%d", tch->GetHP(), tch->GetMaxHP());
	ch->ChatPacket(CHAT_TYPE_INFO, "SP %d/%d", tch->GetSP(), tch->GetMaxSP());
	ch->ChatPacket(CHAT_TYPE_INFO, "MOVE %d", tch->GetPoint(POINT_MOV_SPEED));
	ch->ChatPacket(CHAT_TYPE_INFO, "ATT %d MAGIC_ATT %d SPD %d CRIT %d%% PENE %d%% ATT_BONUS %d%%",
			tch->GetPoint(POINT_ATT_GRADE),
			tch->GetPoint(POINT_MAGIC_ATT_GRADE),
			tch->GetPoint(POINT_ATT_SPEED),
			tch->GetPoint(POINT_CRITICAL_PCT),
			tch->GetPoint(POINT_PENETRATE_PCT),
			tch->GetPoint(POINT_ATT_BONUS));

	ch->ChatPacket(CHAT_TYPE_INFO, "DEF %d MAGIC_DEF %d BLOCK %d%% DODGE %d%% DEF_BONUS %d%%",
			tch->GetPoint(POINT_DEF_GRADE),
			tch->GetPoint(POINT_MAGIC_DEF_GRADE),
			tch->GetPoint(POINT_BLOCK),
			tch->GetPoint(POINT_DODGE),
			tch->GetPoint(POINT_DEF_BONUS));

	ch->ChatPacket(CHAT_TYPE_INFO, "RESISTANCES:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%% WOLF:%3d%% HUMAN:%3d%% FIST:%3d%%",
			tch->GetPoint(POINT_RESIST_WARRIOR),
			tch->GetPoint(POINT_RESIST_ASSASSIN),
			tch->GetPoint(POINT_RESIST_SURA),
			tch->GetPoint(POINT_RESIST_SHAMAN),
			tch->GetPoint(POINT_RESIST_WOLFMAN),
			tch->GetPoint(POINT_RESIST_HUMAN),
			tch->GetPoint(POINT_RESIST_FIST));

	ch->ChatPacket(CHAT_TYPE_INFO, "   SWORD:%3d%% THSWORD:%3d%% DAGGER:%3d%% BELL:%3d%% FAN:%3d%% BOW:%3d%% CLAW:%3d%%",
			tch->GetPoint(POINT_RESIST_SWORD),
			tch->GetPoint(POINT_RESIST_TWOHAND),
			tch->GetPoint(POINT_RESIST_DAGGER),
			tch->GetPoint(POINT_RESIST_BELL),
			tch->GetPoint(POINT_RESIST_FAN),
			tch->GetPoint(POINT_RESIST_BOW),
			tch->GetPoint(POINT_RESIST_CLAW));

	ch->ChatPacket(CHAT_TYPE_INFO, "   FIRE:%3d%% ELEC:%3d%% MAGIC:%3d%% WIND:%3d%% CRIT:%3d%% PENE:%3d%%",
			tch->GetPoint(POINT_RESIST_FIRE),
			tch->GetPoint(POINT_RESIST_ELEC),
			tch->GetPoint(POINT_RESIST_MAGIC),
			tch->GetPoint(POINT_RESIST_WIND),
			tch->GetPoint(POINT_RESIST_CRITICAL),
			tch->GetPoint(POINT_RESIST_PENETRATE));

	ch->ChatPacket(CHAT_TYPE_INFO, "   ICE:%3d%% EARTH:%3d%% DARK:%3d%%",
			tch->GetPoint(POINT_RESIST_ICE),
			tch->GetPoint(POINT_RESIST_EARTH),
			tch->GetPoint(POINT_RESIST_DARK));

	ch->ChatPacket(CHAT_TYPE_INFO, "   MAGICREDUCT:%3d%%", tch->GetPoint(POINT_RESIST_MAGIC_REDUCTION));

	ch->ChatPacket(CHAT_TYPE_INFO, "MALL:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATT:%3d%% DEF:%3d%% EXP:%3d%% ITEMx%d GOLDx%d",
			tch->GetPoint(POINT_MALL_ATTBONUS),
			tch->GetPoint(POINT_MALL_DEFBONUS),
			tch->GetPoint(POINT_MALL_EXPBONUS),
			tch->GetPoint(POINT_MALL_ITEMBONUS) / 10,
			tch->GetPoint(POINT_MALL_GOLDBONUS) / 10);

	ch->ChatPacket(CHAT_TYPE_INFO, "BONUS:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL:%3d%% NORMAL:%3d%% SKILL_DEF:%3d%% NORMAL_DEF:%3d%%",
			tch->GetPoint(POINT_SKILL_DAMAGE_BONUS),
			tch->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS),
			tch->GetPoint(POINT_SKILL_DEFEND_BONUS),
			tch->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS));

	ch->ChatPacket(CHAT_TYPE_INFO, "   HUMAN:%3d%% ANIMAL:%3d%% ORC:%3d%% MILGYO:%3d%% UNDEAD:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_HUMAN),
			tch->GetPoint(POINT_ATTBONUS_ANIMAL),
			tch->GetPoint(POINT_ATTBONUS_ORC),
			tch->GetPoint(POINT_ATTBONUS_MILGYO),
			tch->GetPoint(POINT_ATTBONUS_UNDEAD));

	ch->ChatPacket(CHAT_TYPE_INFO, "   DEVIL:%3d%% INSECT:%3d%% FIRE:%3d%% ICE:%3d%% DESERT:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_DEVIL),
			tch->GetPoint(POINT_ATTBONUS_INSECT),
			tch->GetPoint(POINT_ATTBONUS_FIRE),
			tch->GetPoint(POINT_ATTBONUS_ICE),
			tch->GetPoint(POINT_ATTBONUS_DESERT));

	ch->ChatPacket(CHAT_TYPE_INFO, "   TREE:%3d%% MONSTER:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_TREE),
			tch->GetPoint(POINT_ATTBONUS_MONSTER));

	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%% WOLF:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_WARRIOR),
			tch->GetPoint(POINT_ATTBONUS_ASSASSIN),
			tch->GetPoint(POINT_ATTBONUS_SURA),
			tch->GetPoint(POINT_ATTBONUS_SHAMAN),
			tch->GetPoint(POINT_ATTBONUS_WOLFMAN));

	ch->ChatPacket(CHAT_TYPE_INFO, "ENCHANT:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   FIRE: %d%% ICE: %d%% EARTH: %d%% DARK: %d%% WIND: %d%% ELECT: %d%%",
			tch->GetPoint(POINT_ENCHANT_FIRE),
			tch->GetPoint(POINT_ENCHANT_ICE),
			tch->GetPoint(POINT_ENCHANT_EARTH),
			tch->GetPoint(POINT_ENCHANT_DARK),
			tch->GetPoint(POINT_ENCHANT_WIND),
			tch->GetPoint(POINT_ENCHANT_ELECT));

	ch->ChatPacket(CHAT_TYPE_INFO, "IMMUNE:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   STUN:%d SLOW:%d FALL:%d",
		tch->GetPoint(POINT_IMMUNE_STUN),
		tch->GetPoint(POINT_IMMUNE_SLOW),
		tch->GetPoint(POINT_IMMUNE_FALL));

#ifdef ENABLE_NEWSTUFF
	ch->ChatPacket(CHAT_TYPE_INFO, "MARRIAGE:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   PENE:%d EXP:%d CRIT:%d TRAN:%d ATT:%d DEF:%d",
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_EXP_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_TRANSFER_DAMAGE),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_ATTACK_BONUS),
		tch->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_DEFENSE_BONUS)
	);
#endif

#if defined(ENABLE_PASSIVE_ATTR) && defined(ENABLE_PRECISION)
	ch->ChatPacket(CHAT_TYPE_INFO, "HIT_PCT: %d%%", tch->GetPoint(POINT_HIT_PCT));
#endif

	for (int i = 0; i < MAX_PRIV_NUM; ++i)
		if (CPrivManager::Instance().GetPriv(tch, i))
		{
			int iByEmpire = CPrivManager::Instance().GetPrivByEmpire(tch->GetEmpire(), i);
			int iByGuild = 0;

			if (tch->GetGuild())
				iByGuild = CPrivManager::Instance().GetPrivByGuild(tch->GetGuild()->GetID(), i);

			int iByPlayer = CPrivManager::Instance().GetPrivByCharacter(tch->GetPlayerID(), i);

			if (iByEmpire)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for empire : %d", LC_TEXT(c_apszPrivNames[i]), iByEmpire);

			if (iByGuild)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for guild : %d", LC_TEXT(c_apszPrivNames[i]), iByGuild);

			if (iByPlayer)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for player : %d", LC_TEXT(c_apszPrivNames[i]), iByPlayer);
		}
}



////////////////////////////////////////////////

ACMD(do_notice)
{
	BroadcastNotice(argument);
}

ACMD(do_map_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), false);
}

ACMD(do_big_notice)
{
	BroadcastNotice(argument, true);
}

#ifdef ENABLE_FULL_NOTICE
ACMD(do_map_big_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), true);
}

ACMD(do_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, "%s", argument);
}

ACMD(do_big_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", argument);
}
#endif

ACMD(do_monarch_notice)
{
	if (ch->IsMonarch())
	{
		BroadcastMonarchNotice(ch->GetEmpire(), argument);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;550]");
	}
}

ACMD(do_who)
{
	int iTotal;
	int * paiEmpireUserCount;
	int iLocal;

	DESC_MANAGER::Instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Total [%d] %d / %d / %d (this server %d)"),
		iTotal, paiEmpireUserCount[1], paiEmpireUserCount[2], paiEmpireUserCount[3], iLocal);
}

class user_func
{
	public:
		LPCHARACTER	m_ch;
		static int count;
		static char str[128];
		static int str_len;

		user_func()
			: m_ch(nullptr)
		{}

		void initialize(LPCHARACTER ch)
		{
			m_ch = ch;
			str_len = 0;
			count = 0;
			str[0] = '\0';
		}

		void operator () (LPDESC d)
		{
			if (!d->GetCharacter())
				return;

			//@custom011
			TMapSetting& map_setting = SECTREE_MANAGER::Instance().GetMap(d->GetCharacter()->GetMapIndex())->m_setting;
			int len = snprintf(str + str_len, sizeof(str) - str_len, "%s (%ld,%ld)%-16s", d->GetCharacter()->GetName(), (d->GetCharacter()->GetX() - map_setting.iBaseX) / 100, (d->GetCharacter()->GetY() - map_setting.iBaseY) / 100, "");
			//int len = snprintf(str + str_len, sizeof(str) - str_len, "%-16s ", d->GetCharacter()->GetName());

			if (len < 0 || len >= (int) sizeof(str) - str_len)
				len = (sizeof(str) - str_len) - 1;

			str_len += len;
			++count;

			if (!(count % 4))
			{
				m_ch->ChatPacket(CHAT_TYPE_INFO, str);

				str[0] = '\0';
				str_len = 0;
			}
		}
};

int	user_func::count = 0;
char user_func::str[128] = { 0, };
int	user_func::str_len = 0;

ACMD(do_user)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	user_func func;

	func.initialize(ch);
	std::for_each(c_ref_set.begin(), c_ref_set.end(), func);

	if (func.count % 4)
		ch->ChatPacket(CHAT_TYPE_INFO, func.str);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total %d", func.count);
}

ACMD(do_disconnect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /dc <player name>");
		return;
	}

	LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", arg1);
		return;
	}

	if (tch == ch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
		return;
	}

	DESC_MANAGER::Instance().DestroyDesc(d);
}

ACMD(do_kill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /kill <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->Dead();
}

#ifdef ENABLE_NEWSTUFF
ACMD(do_poison)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /poison <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByPoison(nullptr);
}
#endif
#ifdef ENABLE_WOLFMAN_CHARACTER
ACMD(do_bleeding)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /bleeding <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::Instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByBleeding(nullptr);
}
#endif

#define MISC    0
#define BINARY  1
#define NUMBER  2

namespace DoSetTypes{
typedef enum do_set_types_s {GOLD, 
#ifdef ENABLE_CHEQUE_SYSTEM
	CHEQUE, 
#endif
#ifdef ENABLE_GEM_SYSTEM
	GAYA,
#endif
#ifdef ENABLE_BATTLE_FIELD
	BATTLE,
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	MEDAL, 
#endif
	RACE, SEX, JOB, EXP,
#ifdef ENABLE_YOHARA_SYSTEM
	CONQUEROR_EXP,
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	SOUL,
	SOULRE,
#endif
	MAX_HP, MAX_SP, SKILL, ALIGNMENT, ALIGN} do_set_types_t;
}

const struct set_struct
{
	const char *cmd;
	const char type;
	const char * help;
} set_fields[] = {
	{ "gold",		NUMBER,	nullptr	},
#ifdef ENABLE_CHEQUE_SYSTEM
	{ "cheque",		NUMBER,	nullptr	},
#endif
#ifdef ENABLE_GEM_SYSTEM
	{ "gaya",		NUMBER, nullptr	},
#endif
#ifdef ENABLE_BATTLE_FIELD
	{ "battle",		NUMBER, nullptr	},
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	{ "medal",		NUMBER, nullptr	},
#endif
#ifdef ENABLE_WOLFMAN_CHARACTER
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman, 4. Lycan"		},
#else
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman"		},
#endif
	{ "sex",		NUMBER,	"0. Male, 1. Female"	},
	{ "job",		NUMBER,	"0. None, 1. First, 2. Second"	},
	{ "exp",		NUMBER,	nullptr	},
#ifdef ENABLE_YOHARA_SYSTEM
	{ "cexp",		NUMBER,	nullptr	},
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	{ "soul",		NUMBER,	nullptr	},
	{ "soulre",		NUMBER,	nullptr	},
#endif
	{ "max_hp",		NUMBER,	nullptr	},
	{ "max_sp",		NUMBER,	nullptr	},
	{ "skill",		NUMBER,	nullptr	},
	{ "alignment",	NUMBER,	nullptr	},
	{ "align",		NUMBER,	nullptr	},
	{ "\n",			MISC,	nullptr	}
};

ACMD(do_set)
{
	char arg1[256], arg2[256], arg3[256];

	LPCHARACTER tch = nullptr;

	int i, len;
	const char* line;

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: set <name> <field> <value>");
#ifdef ENABLE_NEWSTUFF
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the fields available:");
		for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, " %d. %s", i+1, set_fields[i].cmd);
			if (set_fields[i].help != nullptr)
				ch->ChatPacket(CHAT_TYPE_INFO, "  Help: %s", set_fields[i].help);
		}
#endif
		return;
	}

	tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	len = strlen(arg2);

	for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		if (!strncmp(arg2, set_fields[i].cmd, len))
			break;

	switch (i)
	{
		case DoSetTypes::GOLD:	// gold
			{
				int gold = 0;
				str_to_number(gold, arg3);
				DBManager::Instance().SendMoneyLog(MONEY_LOG_MISC, 3, gold);
				tch->PointChange(POINT_GOLD, gold, true);
			}
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case DoSetTypes::CHEQUE: // cheque
			{
				int cheque = 0;
				str_to_number(cheque, arg3);
				tch->PointChange(POINT_CHEQUE, cheque, true);
				tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cheque: ADD[%d] TOTAL[%d]"), cheque, tch->GetCheque());
			}
			break;
#endif

#ifdef ENABLE_GEM_SYSTEM
		case DoSetTypes::GAYA: //gaya
			{
				int gaya = 0;
				str_to_number(gaya, arg3);
				tch->PointChange(POINT_GEM, gaya, true);
				tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Gem: ADD[%d] TOTAL[%d]"), gaya, tch->GetGemPoint());
			}
			break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case DoSetTypes::BATTLE: // Battle points
			{
				long battle = 0;
				str_to_number(battle, arg3);
				tch->PointChange(POINT_BATTLE_FIELD, battle, true);
			}
			break;
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
		case DoSetTypes::MEDAL: // Medals of honor
		{
			long medal = 0;
			str_to_number(medal, arg3);
			tch->PointChange(POINT_MEDAL_OF_HONOR, medal, true);
		}
		break;
#endif

		case DoSetTypes::RACE: // race
#ifdef ENABLE_NEWSTUFF
			{
				int amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(0, amount, JOB_MAX_NUM);
				ESex mySex = GET_SEX(tch);
				uint32_t dwRace = MAIN_RACE_WARRIOR_M;
				switch (amount)
				{
					case JOB_WARRIOR:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_WARRIOR_M:MAIN_RACE_WARRIOR_W;
						break;
					case JOB_ASSASSIN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_ASSASSIN_M:MAIN_RACE_ASSASSIN_W;
						break;
					case JOB_SURA:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_SURA_M:MAIN_RACE_SURA_W;
						break;
					case JOB_SHAMAN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_SHAMAN_M:MAIN_RACE_SHAMAN_W;
						break;
#ifdef ENABLE_WOLFMAN_CHARACTER
					case JOB_WOLFMAN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_WOLFMAN_M:MAIN_RACE_WOLFMAN_M;
						break;
#endif
				}
				if (dwRace!=tch->GetRaceNum())
				{
					tch->SetRace(dwRace);
					tch->ClearSkill();
					tch->SetSkillGroup(0);
					// quick mesh change workaround begin
					tch->SetPolymorph(101);
					tch->SetPolymorph(0);
					// quick mesh change workaround end
				}
			}
#endif
			break;

		case DoSetTypes::SEX: // sex
#ifdef ENABLE_NEWSTUFF
			{
				int amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(SEX_MALE, amount, SEX_FEMALE);
				if (amount != GET_SEX(tch))
				{
					tch->ChangeSex();
					// quick mesh change workaround begin
					tch->SetPolymorph(101);
					tch->SetPolymorph(0);
					// quick mesh change workaround end
				}
			}
#endif
			break;

		case DoSetTypes::JOB: // job
#ifdef ENABLE_NEWSTUFF
			{
				int amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(0, amount, 2);
				if (amount != tch->GetSkillGroup())
				{
					tch->ClearSkill();
					tch->SetSkillGroup(amount);
				}
			}
#endif
			break;

		case DoSetTypes::EXP: // exp
			{
				int amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_EXP, amount, true);
			}
			break;

#ifdef ENABLE_YOHARA_SYSTEM
		case DoSetTypes::CONQUEROR_EXP:	//conqueror_exp
			{
				int amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_CONQUEROR_EXP, amount, true);
				tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ConquerorExp: ADD[%d] TOTAL[%d]"), amount, tch->GetConquerorExp());
				tch->PointsPacket();
			}
			break;
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		case DoSetTypes::SOUL:
			{
				int amount = 0;
				str_to_number(amount, arg3);
				if (amount + tch->GetSoulPoint() > SOUL_MAX)
					return;
				tch->PointChange(POINT_SOUL, amount, true);
			}
			break;

		case DoSetTypes::SOULRE:
			{
				int amount = 0;
				str_to_number(amount, arg3);
				if (amount + tch->GetSoulRePoint() > SOUL_RE_MAX)
					return;
				tch->PointChange(POINT_SOUL_RE, amount, true);
			}
			break;
#endif

		case DoSetTypes::MAX_HP: // max_hp
			{
				int amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_MAX_HP, amount, true);
			}
			break;

		case DoSetTypes::MAX_SP: // max_sp
			{
				int amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_MAX_SP, amount, true);
			}
			break;

		case DoSetTypes::SKILL: // active skill point
			{
				int amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_SKILL, amount, true);
			}
			break;

		case DoSetTypes::ALIGN: // alignment
		case DoSetTypes::ALIGNMENT: // alignment
			{
				int	amount = 0;
				str_to_number(amount, arg3);
				tch->UpdateAlignment(amount - tch->GetRealAlignment());	// @fixme402 (ch -> tch)
			}
			break;
	}

	if (set_fields[i].type == NUMBER)
	{
		int	amount = 0;
		str_to_number(amount, arg3);
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s set to [%d]", tch->GetName(), set_fields[i].cmd, amount);
	}
}

ACMD(do_reset)
{
	ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
	ch->Save();
}

ACMD(do_advance)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: advance <name> <level>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	int level = 0;
	str_to_number(level, arg2);

	tch->ResetPoint(MINMAX(0, level, gPlayerMaxLevel));
}

ACMD(do_respawn)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1 && !strcasecmp(arg1, "all"))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw everywhere");
		regen_reset(0, 0);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw around");
		regen_reset(ch->GetX(), ch->GetY());
	}
}

ACMD(do_safebox_size)
{

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int size = 0;

	if (*arg1)
		str_to_number(size, arg1);

	if (size > 3 || size < 0)
		size = 0;

	ch->ChatPacket(CHAT_TYPE_INFO, "Safebox size set to %d", size);
	ch->ChangeSafeboxSize(size);
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
ACMD(do_guildstorage_size)
{

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int size = 0;

	if (*arg1)
		str_to_number(size, arg1);

	if (size > 3 || size < 0)
		size = 0;

	ch->ChatPacket(CHAT_TYPE_INFO, "guildstorage size set to %d", size);
	ch->ChangeGuildstorageSize(size);
}
#endif

ACMD(do_makeguild)
{
	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::Instance();

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, arg1, sizeof(cp.name));

	if (!check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;680]");
		return;
	}

	gm.CreateGuild(cp);
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;741;%s]", cp.name);
}

ACMD(do_deleteguild)
{
	if (ch->GetGuild())
		ch->GetGuild()->RequestDisband(ch->GetPlayerID());
}

ACMD(do_greset)
{
	if (ch->GetGuild())
		ch->GetGuild()->Reset();
}

// REFINE_ROD_HACK_BUG_FIX
ACMD(do_refine_rod)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
		fishing::RealRefineRod(ch, item);
}
// END_OF_REFINE_ROD_HACK_BUG_FIX

// REFINE_PICK
ACMD(do_refine_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
		mining::RealRefinePick(ch, item);
	}
}

ACMD(do_max_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
	}
}
// END_OF_REFINE_PICK


ACMD(do_fishing_simul)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	argument = one_argument(argument, arg1, sizeof(arg1));
	two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));

	int count = 1000;
	int prob_idx = 0;
	int level = 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "Usage: fishing_simul <level> <prob index> <count>");

	if (*arg1)
		str_to_number(level, arg1);

	if (*arg2)
		str_to_number(prob_idx, arg2);

	if (*arg3)
		str_to_number(count, arg3);

	fishing::Simulation(level, count, prob_idx, ch);
}

ACMD(do_invisibility)
{
	if (ch->IsAffectFlag(AFF_INVISIBILITY))
	{
		ch->RemoveAffect(AFFECT_INVISIBILITY);
	}
	else
	{
		ch->AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, INFINITE_AFFECT_DURATION, 0, true);
	}
}

ACMD(do_event_flag)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!(*arg1) || !(*arg2))
		return;

	int value = 0;
	str_to_number(value, arg2);

	if (!strcmp(arg1, "mob_item") ||
			!strcmp(arg1, "mob_exp") ||
			!strcmp(arg1, "mob_gold") ||
			!strcmp(arg1, "mob_dam") ||
			!strcmp(arg1, "mob_gold_pct") ||
			!strcmp(arg1, "mob_item_buyer") ||
			!strcmp(arg1, "mob_exp_buyer") ||
			!strcmp(arg1, "mob_gold_buyer") ||
			!strcmp(arg1, "mob_gold_pct_buyer")
	   )
		value = MINMAX(0, value, EVENT_MOB_RATE_LIMIT);

	//quest::CQuestManager::Instance().SetEventFlag(arg1, atoi(arg2));
	quest::CQuestManager::Instance().RequestSetEventFlag(arg1, value);
	ch->ChatPacket(CHAT_TYPE_INFO, "RequestSetEventFlag %s %d", arg1, value);
	sys_log(0, "RequestSetEventFlag %s %d", arg1, value);
}

ACMD(do_get_event_flag)
{
	quest::CQuestManager::Instance().SendEventFlagList(ch);
}

ACMD(do_private)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: private <map index>");
		return;
	}

	long lMapIndex;
	long map_index = 0;
	str_to_number(map_index, arg1);
	if ((lMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(map_index)))
	{
		ch->SaveExitLocation();

		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);
		ch->WarpSet(pkSectreeMap->m_setting.posSpawn.x, pkSectreeMap->m_setting.posSpawn.y, lMapIndex);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Can't find map by index %d", map_index);
}

ACMD(do_qf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	std::string questname = pPC->GetCurrentQuestName();

	if (!questname.empty())
	{
		int value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, arg1);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		unsigned int questindex = quest::CQuestManager::Instance().GetQuestIndexByName(questname);

		while (it!= pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

LPCHARACTER chHori, chForge, chLib, chTemple, chTraining, chTree, chPortal, chBall;

ACMD(do_b1)
{
	//호리병 478 579
	chHori = CHARACTER_MANAGER::Instance().SpawnMobRange(14017, ch->GetMapIndex(), 304222, 742858, 304222, 742858, true, false);
	chHori->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_CONSTRUCTION_SMALL, 65535, 0, true);
	chHori->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);

	for (int i = 0; i < 30; ++i)
	{
		int rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 800, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(number(701, 706),
				ch->GetMapIndex(),
				304222 + (int)fx,
				742858 + (int)fy,
				304222 + (int)fx,
				742858 + (int)fy,
				true,
				false);
		tch->SetAggressive();
	}

	for (int i = 0; i < 5; ++i)
	{
		int rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 800, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(8009,
				ch->GetMapIndex(),
				304222 + (int)fx,
				742858 + (int)fy,
				304222 + (int)fx,
				742858 + (int)fy,
				true,
				false);
		tch->SetAggressive();
	}
}

ACMD(do_b2)
{
	chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
}

ACMD(do_b3)
{
	// 포지 492 547
	chForge = CHARACTER_MANAGER::Instance().SpawnMobRange(14003, ch->GetMapIndex(), 307500, 746300, 307500, 746300, true, false);
	chForge->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//높은탑 509 589 -> 도서관
	chLib = CHARACTER_MANAGER::Instance().SpawnMobRange(14007, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//욕조 513 606 -> 힘의신전
	chTemple = CHARACTER_MANAGER::Instance().SpawnMobRange(14004, ch->GetMapIndex(), 307700, 741600, 307700, 741600, true, false);
	chTemple->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//권투장 490 625
	chTraining= CHARACTER_MANAGER::Instance().SpawnMobRange(14010, ch->GetMapIndex(), 307100, 739500, 307100, 739500, true, false);
	chTraining->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//나무 466 614
	chTree= CHARACTER_MANAGER::Instance().SpawnMobRange(14013, ch->GetMapIndex(), 300800, 741600, 300800, 741600, true, false);
	chTree->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	//포탈 439 615
	chPortal= CHARACTER_MANAGER::Instance().SpawnMobRange(14001, ch->GetMapIndex(), 300900, 744500, 300900, 744500, true, false);
	chPortal->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	// 구슬 436 600
	chBall = CHARACTER_MANAGER::Instance().SpawnMobRange(14012, ch->GetMapIndex(), 302500, 746600, 302500, 746600, true, false);
	chBall->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_b4)
{
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_UPGRADE, 65535, 0, true);

	for (int i = 0; i < 30; ++i)
	{
		int rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 1200, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(number(701, 706),
				ch->GetMapIndex(),
				307900 + (int)fx,
				744500 + (int)fy,
				307900 + (int)fx,
				744500 + (int)fy,
				true,
				false);
		tch->SetAggressive();
	}

	for (int i = 0; i < 5; ++i)
	{
		int rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 1200, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::Instance().SpawnMobRange(8009,
				ch->GetMapIndex(),
				307900 + (int)fx,
				744500 + (int)fy,
				307900 + (int)fx,
				744500 + (int)fy,
				true,
				false);
		tch->SetAggressive();
	}

}

ACMD(do_b5)
{
	M2_DESTROY_CHARACTER(chLib);
	//chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
	chLib = CHARACTER_MANAGER::Instance().SpawnMobRange(14008, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_b6)
{
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_UPGRADE, 65535, 0, true);
}
ACMD(do_b7)
{
	M2_DESTROY_CHARACTER(chLib);
	//chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
	chLib = CHARACTER_MANAGER::Instance().SpawnMobRange(14009, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

/*ACMD(do_book)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	CSkillProto * pkProto;

	if (isnhdigit(*arg1))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		pkProto = CSkillManager::Instance().Get(vnum);
	}
	else
		pkProto = CSkillManager::Instance().Get(arg1);

	if (!pkProto)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such a skill.");
		return;
	}

	LPITEM item = ch->AutoGiveItem(50300);
	item->SetSocket(0, pkProto->dwVnum);
}*/

ACMD(do_book)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	CSkillProto * pkProto;
	if (*arg1 && isnhdigit(*arg1)) {
		const uint32_t vnum = std::stoul(arg1);
		pkProto = CSkillManager::Instance().Get(vnum);
	}
	else 
		pkProto = CSkillManager::Instance().Get(arg1);

	if (!pkProto) {
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such a skill.");
		return;
	}

	int count = 1;
	if (*arg2 && isnhdigit(*arg2))
		count = std::stoi(arg2);

	while (count > 0) {
		const LPITEM item = ch->AutoGiveItem(50300, count > g_bItemCountLimit ? g_bItemCountLimit : count);
		if (!item)
			return;
		item->SetSocket(0, pkProto->dwVnum);
		count-= g_bItemCountLimit;
	}
}

ACMD(do_setskillother)
{
	char arg1[256], arg2[256], arg3[256];
	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(argument, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3 || !isdigit(*arg3))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskillother <target> <skillname> <lev>");
		return;
	}

	LPCHARACTER tch;

	tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	CSkillProto * pk;

	if (isdigit(*arg2))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg2);
		pk = CSkillManager::Instance().Get(vnum);
	}
	else
		pk = CSkillManager::Instance().Get(arg2);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	uint8_t level = 0;
	str_to_number(level, arg3);
	tch->SetSkillLevel(pk->dwVnum, level);
	tch->ComputePoints();
	tch->SkillLevelPacket();
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnMasterSkill(tch, pk->dwVnum, level);
#endif
}

ACMD(do_setskill)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2 || !isdigit(*arg2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskill <name> <lev>");
		return;
	}

	CSkillProto * pk;

	if (isdigit(*arg1))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		pk = CSkillManager::Instance().Get(vnum);
	}

	else
		pk = CSkillManager::Instance().Get(arg1);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	uint8_t level = 0;
	str_to_number(level, arg2);
	ch->SetSkillLevel(pk->dwVnum, level);
	ch->ComputePoints();
	ch->SkillLevelPacket();
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem::Instance().OnMasterSkill(ch, pk->dwVnum, level);
#endif
}

ACMD(do_set_skill_point)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int skill_point = 0;
	if (*arg1)
		str_to_number(skill_point, arg1);

	ch->SetRealPoint(POINT_SKILL, skill_point);
	ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
	ch->PointChange(POINT_SKILL, 0);
}

ACMD(do_set_skill_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int skill_group = 0;
	if (*arg1)
		str_to_number(skill_group, arg1);

	ch->SetSkillGroup(skill_group);

	ch->ClearSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "skill group to %d.", skill_group);
}

#ifdef ENABLE_EXTENDED_RELOAD
#	include "shop_manager.h"
#endif
ACMD(do_reload)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
#ifdef ENABLE_EXTENDED_RELOAD
	const int FILE_NAME_LEN = 256;
#endif

	if (*arg1)
	{
		switch (LOWER(*arg1))
		{
			// b g h i j k l m n o t v z
			case 'u':
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");
				LoadStateUserCount();
				break;

			case 'p':
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);
				break;

			case 's':
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading notice string.");
				DBManager::Instance().LoadDBString();
				break;

			case 'q':
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading quest.");
				quest::CQuestManager::Instance().Reload();
				break;

			case 'f':
				fishing::Initialize();
				break;

				//RELOAD_ADMIN
			case 'a':
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading Admin infomation.");
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, nullptr, 0);
				sys_log(0, "Reloading admin infomation.");
				break;
				//END_RELOAD_ADMIN

			case 'c': // cube
				// Only local processes are updated.
#ifndef ENABLE_CUBE_RENEWAL
				Cube_init ();
#else
				CCubeManager::Instance().Cube_init();
#endif
				break;

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
			case 'r':
				if (!CSoulRoulette::ReadRouletteData()) {
					ch->ChatPacket(CHAT_TYPE_INFO, "Error Reloading <CSoulRoulette>!");
					CSoulRoulette::ReadRouletteData(true); // reset
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "<CSoulRoulette> Reloaded!");
				break;
#endif

#ifdef ENABLE_EVENT_MANAGER
			case 'e':
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Reloading event infomation."));
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_EVENT, 0, nullptr, 0);
				sys_log(0, "Reloading event infomation.");
				break;
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
			case 'd': // Dungeon Info
				ch->ChatPacket(CHAT_TYPE_INFO, "Reloading dungeon info.");
				CDungeonInfoManager::Instance().Reload();
				break;
#endif

#ifdef ENABLE_EXTENDED_RELOAD
			case 'w':
			{
				char szMobDropItemGroupFileName[FILE_NAME_LEN];
				snprintf(szMobDropItemGroupFileName, sizeof(szMobDropItemGroupFileName), "%s/mob_drop_item.txt", LocaleService_GetBasePath().c_str());

				if (ITEM_MANAGER::Instance().ReloadMobDropItemGroup(szMobDropItemGroupFileName))
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading MobDropItemGroup.");
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "Failed to reload MobDropItemGroup.");

				return;
			}

			case 'x':
			{
				char szSpecialItemGroupFileName[FILE_NAME_LEN];
				snprintf(szSpecialItemGroupFileName, sizeof(szSpecialItemGroupFileName), "%s/special_item_group.txt", LocaleService_GetBasePath().c_str());

				if (ITEM_MANAGER::Instance().ReloadSpecialItemGroup(szSpecialItemGroupFileName))
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading SpecialItemGroup.");
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "Failed to reload SpecialItemGroup.");

				break;
			}


# ifdef ENABLE_RENEWAL_SHOPEX
			case 'y':
			{
				char szShopTableExFileName[FILE_NAME_LEN];
				snprintf(szShopTableExFileName, sizeof(szShopTableExFileName),
					"%s/shop_table_ex.txt", LocaleService_GetBasePath().c_str());

				if (CShopManager::Instance().ReadShopTableEx(szShopTableExFileName))
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading ShopTableEx.");
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "Failed to reload ShopTableEx.");

				break;
			}
# endif
#endif
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");
		LoadStateUserCount();

		ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
		db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);

		ch->ChatPacket(CHAT_TYPE_INFO, "Reloading notice string.");
		DBManager::Instance().LoadDBString();
	}
}

ACMD(do_cooltime)
{
	ch->DisableCooltime();
}

ACMD(do_level)
{
	char arg2[256];
	one_argument(argument, arg2, sizeof(arg2));

	if (!*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: level <level>");
		return;
	}

	int	level = 0;
	str_to_number(level, arg2);

	ch->ResetPoint(MINMAX(1, level, gPlayerMaxLevel));

	ch->ClearSkill();
	ch->ClearSubSkill();
}

ACMD(do_gwlist)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("현재 전쟁중인 길드 입니다"));
	CGuildManager::Instance().ShowGuildWarList(ch);
}

ACMD(do_stop_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int id1 = 0, id2 = 0;

	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (!id1 || !id2)
		return;

	if (id1 > id2)
	{
		std::swap(id1, id2);
	}

	ch->ChatPacket(CHAT_TYPE_TALKING, "%d %d", id1, id2);
	CGuildManager::Instance().RequestEndWar(id1, id2);
}

ACMD(do_cancel_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int id1 = 0, id2 = 0;
	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (id1 > id2)
		std::swap(id1, id2);

	CGuildManager::Instance().RequestCancelWar(id1, id2);
}

ACMD(do_guild_state)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CGuild* pGuild = CGuildManager::Instance().FindGuildByName(arg1);
	if (pGuild != nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildID: %d", pGuild->GetID());
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildMasterPID: %d", pGuild->GetMasterPID());
		ch->ChatPacket(CHAT_TYPE_INFO, "IsInWar: %d", pGuild->UnderAnyWar());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;743;%s]", arg1);
	}
}

struct FuncWeaken
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncWeaken(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)	// 10미터 이상에 있는 것들은 purge 하지 않는다.
			return;

		if (pkChr->IsNPC()
#ifdef ENABLE_DEFENSE_WAVE
			&& !pkChr->IsMast()
#endif
			)
			pkChr->PointChange(POINT_HP, (10 - pkChr->GetHP()));
	}
};

ACMD(do_weaken)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncWeaken func(ch);

	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	ch->GetSectree()->ForEachAround(func);
}

ACMD(do_getqf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	LPCHARACTER tch;

	if (!*arg1)
		tch = ch;
	else
	{
		tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}

	quest::PC* pPC = quest::CQuestManager::Instance().GetPC(tch->GetPlayerID());

	if (pPC)
		pPC->SendFlagList(ch);
}

ACMD(do_set_state)
{
	char arg1[256];
	char arg2[256];

	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO,
			"Syntax: set_state <questname> <statename>"
#ifdef ENABLE_SET_STATE_WITH_TARGET
			" [<character name>]"
#endif
		);
		return;
	}

#ifdef ENABLE_SET_STATE_WITH_TARGET
	LPCHARACTER tch = ch;
	char arg3[256];
	argument = one_argument(argument, arg3, sizeof(arg3));
	if (*arg3)
	{
		tch = CHARACTER_MANAGER::Instance().FindPC(arg3);
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}
	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(tch->GetPlayerID());
#else
	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
#endif
	std::string questname = arg1;
	std::string statename = arg2;

	if (!questname.empty())
	{
		int value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, statename);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		unsigned int questindex = quest::CQuestManager::Instance().GetQuestIndexByName(questname);

		while (it!= pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

ACMD(do_setqf)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];

	one_argument(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setqf <flagname> <value> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg3)
		tch = CHARACTER_MANAGER::Instance().FindPC(arg3);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::Instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		int value = 0;
		str_to_number(value, arg2);
		pPC->SetFlag(arg1, value);
		ch->ChatPacket(CHAT_TYPE_INFO, "Quest flag set: %s %d", arg1, value);
	}
}

ACMD(do_delqf)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: delqf <flagname> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg2)
		tch = CHARACTER_MANAGER::Instance().FindPC(arg2);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::Instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		if (pPC->DeleteFlag(arg1))
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete success.");
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete failed. Quest flag does not exist.");
	}
}

ACMD(do_forgetme)
{
	ch->ForgetMyAttacker();
}

ACMD(do_aggregate)
{
	ch->AggregateMonster();
}

ACMD(do_attract_ranger)
{
	ch->AttractRanger();
}

ACMD(do_pull_monster)
{
	ch->PullMonster();
}

ACMD(do_polymorph)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);
		bool bMaintainStat = false;
		if (*arg2)
		{
			int value = 0;
			str_to_number(value, arg2);
			bMaintainStat = (value>0);
		}

		ch->SetPolymorph(dwVnum, bMaintainStat);
	}
}

ACMD(do_polymorph_item)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);

		LPITEM item = ITEM_MANAGER::Instance().CreateItem(70104, 1, 0, true);
		if (item)
		{
			item->SetSocket(0, dwVnum);
#ifdef ENABLE_SPECIAL_INVENTORY
			int iEmptyPos = ch->GetEmptyInventory(item);
#else
			int iEmptyPos = ch->GetEmptyInventory(item->GetSize());
#endif

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
				LogManager::Instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", 70103);
		}
		//ch->SetPolymorph(dwVnum, bMaintainStat);
	}
}

ACMD(do_priv_empire)
{
	char arg1[256] = {0};
	char arg2[256] = {0};
	char arg3[256] = {0};
	char arg4[256] = {0};
	int empire = 0;
	int type = 0;
	int value = 0;
	int duration = 0;

	const char* line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		goto USAGE;

	if (!line)
		goto USAGE;

	two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg3 || !*arg4)
		goto USAGE;

	str_to_number(empire, arg1);
	str_to_number(type,	arg2);
	str_to_number(value,	arg3);
	value = MINMAX(0, value, PRIV_EMPIRE_RATE_LIMIT);
	str_to_number(duration, arg4);

	if (empire < 0 || 3 < empire)
		goto USAGE;

	if (type < 1 || 4 < type)
		goto USAGE;

	if (value < 0)
		goto USAGE;

	if (duration < 0)
		goto USAGE;

	// 시간 단위로 변경
	duration = duration * (60*60);

	sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, duration=%d) by command",
			empire, type, value, duration);
	CPrivManager::Instance().RequestGiveEmpirePriv(empire, type, value, duration);
	return;

USAGE:
	ch->ChatPacket(CHAT_TYPE_INFO, "usage : priv_empire <empire> <type> <value> <duration>");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <empire>    0 - 3 (0==all)");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <type>      1:item_drop, 2:gold_drop, 3:gold10_drop, 4:exp");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <value>     percent");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <duration>  hour");
}

/**
 * @version 05/06/08	Bang2ni - 길드 보너스 퀘스트 진행 안되는 문제 수정.(스크립트가 작성안됨.)
 * 			          quest/priv_guild.quest 로 부터 스크립트 읽어오게 수정됨
 */
ACMD(do_priv_guild)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		CGuild * g = CGuildManager::Instance().FindGuildByName(arg1);

		if (!g)
		{
			uint32_t guild_id = 0;
			str_to_number(guild_id, arg1);
			g = CGuildManager::Instance().FindGuild(guild_id);
		}

		if (!g)
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;744]");
		else
		{
			char buf[1024+1];
			snprintf(buf, sizeof(buf), "%d", g->GetID()); // @fixme177

			using namespace quest;
			PC * pc = CQuestManager::Instance().GetPC(ch->GetPlayerID());
			QuestState qs = CQuestManager::Instance().OpenState("ADMIN_QUEST", QUEST_FISH_REFINE_STATE_INDEX);
			luaL_loadbuffer(qs.co, buf, strlen(buf), "ADMIN_QUEST");
			pc->SetQuest("ADMIN_QUEST", qs);

			QuestState & rqs = *pc->GetRunningQuestState();

			if (!CQuestManager::Instance().RunState(rqs))
			{
				CQuestManager::Instance().CloseState(rqs);
				pc->EndRunning();
				return;
			}
		}
	}
}

ACMD(do_mount_test)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		ch->MountVnum(vnum);
	}
}

ACMD(do_observer)
{
	ch->SetObserverMode(!ch->IsObserverMode());
}

ACMD(do_socket_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);

		int iSocketCount = 0;
		str_to_number(iSocketCount, arg2);

		if (!iSocketCount || iSocketCount >= ITEM_SOCKET_MAX_NUM)
			iSocketCount = 3;

		if (!dwVnum)
		{
			if (!ITEM_MANAGER::Instance().GetVnum(arg1, dwVnum))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", dwVnum);
				return;
			}
		}

		LPITEM item = ch->AutoGiveItem(dwVnum);

		if (item)
		{
			for (int i = 0; i < iSocketCount; ++i)
				item->SetSocket(i, 1);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d cannot create item.", dwVnum);
		}
	}
}

ACMD(do_xmas)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int flag = 0;

	if (*arg1)
		str_to_number(flag, arg1);

	switch (subcmd)
	{
		case SCMD_XMAS_SNOW:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_snow", flag);
			break;

		case SCMD_XMAS_BOOM:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_boom", flag);
			break;

		case SCMD_XMAS_SANTA:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_santa", flag);
			break;

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		case SCMD_XMAS_SOUL:
			quest::CQuestManager::Instance().RequestSetEventFlag("xmas_soul", flag);
			break;
#endif
	}
}


// BLOCK_CHAT
ACMD(do_block_chat_list)
{
	// GM이 아니거나 block_chat_privilege가 없는 사람은 명령어 사용 불가
	if (!ch || (ch->GetGMLevel() < GM_HIGH_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;921]");
		return;
	}

	DBManager::Instance().ReturnQuery(QID_BLOCK_CHAT_LIST, ch->GetPlayerID(), nullptr,
			"SELECT p.name, a.lDuration FROM affect%s as a, player%s as p WHERE a.bType = %d AND a.dwPID = p.id",
			get_table_postfix(), get_table_postfix(), AFFECT_BLOCK_CHAT);
}

ACMD(do_vote_block_chat)
{
	return;

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: vote_block_chat <name>");
		return;
	}

	const char* name = arg1;
	long lBlockDuration = 10;
	sys_log(0, "vote_block_chat %s %d", name, lBlockDuration);

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(name);

		if (pkCCI)
		{
			TPacketGGBlockChat p{};

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, name, sizeof(p.szName));
			p.lBlockDuration = lBlockDuration;
			P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGBlockChat));
		}
		else
		{
			TPacketBlockChat p{};

			strlcpy(p.szName, name, sizeof(p.szName));
			p.lDuration = lBlockDuration;
			db_clientdesc->DBPacket(HEADER_GD_BLOCK_CHAT, ch ? ch->GetDesc()->GetHandle() : 0, &p, sizeof(p));

		}

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

		return;
	}

	if (tch && ch != tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, lBlockDuration, 0, true);
}

ACMD(do_block_chat)
{
	// GM이 아니거나 block_chat_privilege가 없는 사람은 명령어 사용 불가
	if (ch && (ch->GetGMLevel() < GM_HIGH_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;921]");
		return;
	}

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: block_chat <name> <time> (0 to off)");

		return;
	}

	const char* name = arg1;
	long lBlockDuration = parse_time_str(argument);

	if (lBlockDuration < 0)
	{
		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "잘못된 형식의 시간입니다. h, m, s를 붙여서 지정해 주십시오.");
			ch->ChatPacket(CHAT_TYPE_INFO, "예) 10s, 10m, 1m 30s");
		}
		return;
	}

	sys_log(0, "BLOCK CHAT %s %d", name, lBlockDuration);

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(name);

		if (pkCCI)
		{
			TPacketGGBlockChat p{};

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, name, sizeof(p.szName));
			p.lBlockDuration = lBlockDuration;
			P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGBlockChat));
		}
		else
		{
			TPacketBlockChat p{};

			strlcpy(p.szName, name, sizeof(p.szName));
			p.lDuration = lBlockDuration;
			db_clientdesc->DBPacket(HEADER_GD_BLOCK_CHAT, ch ? ch->GetDesc()->GetHandle() : 0, &p, sizeof(p));
		}

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

		return;
	}

	if (tch && ch != tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, lBlockDuration, 0, true);
}
// END_OF_BLOCK_CHAT

// BUILD_BUILDING
ACMD(do_build)
{
	using namespace building;

	char arg1[256], arg2[256], arg3[256], arg4[256];
	const char * line = one_argument(argument, arg1, sizeof(arg1));
	uint8_t GMLevel = ch->GetGMLevel();

	CLand * pkLand = CManager::Instance().FindLand(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	// NOTE: 조건 체크들은 클라이언트와 서버가 함께 하기 때문에 문제가 있을 때는
	//       메세지를 전송하지 않고 에러를 출력한다.
	if (!pkLand)
	{
		sys_err("%s trying to build on not buildable area.", ch->GetName());
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax: no command");
		return;
	}

	// 건설 권한 체크
	if (GMLevel == GM_PLAYER)
	{
		// 플레이어가 집을 지을 때는 땅이 내껀지 확인해야 한다.
		if ((!ch->GetGuild() || ch->GetGuild()->GetID() != pkLand->GetOwner()))
		{
			sys_err("%s trying to build on not owned land.", ch->GetName());
			return;
		}

		// 내가 길마인가?
		if (ch->GetGuild()->GetMasterPID() != ch->GetPlayerID())
		{
			sys_err("%s trying to build while not the guild master.", ch->GetName());
			return;
		}
	}

	switch (LOWER(*arg1))
	{
		case 'c':
			{
				// /build c vnum x y x_rot y_rot z_rot
				char arg5[256], arg6[256];
				line = one_argument(two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3)); // vnum x y
				one_argument(two_arguments(line, arg4, sizeof(arg4), arg5, sizeof(arg5)), arg6, sizeof(arg6)); // x_rot y_rot z_rot

				if (!*arg1 || !*arg2 || !*arg3 || !*arg4 || !*arg5 || !*arg6)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
					return;
				}

				uint32_t dwVnum = 0;
				str_to_number(dwVnum,  arg1);

				using namespace building;

				const TObjectProto * t = CManager::Instance().GetObjectProto(dwVnum);
				if (!t)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;745]");
					return;
				}

				const uint32_t BUILDING_MAX_PRICE = 100000000;

				if (t->dwGroupVnum)
				{
					if (pkLand->FindObjectByGroup(t->dwGroupVnum))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;746]");
						return;
					}
				}

				// 건물 종속성 체크 (이 건물이 지어져 있어야함)
				if (t->dwDependOnGroupVnum)
				{
					//		const TObjectProto * dependent = CManager::Instance().GetObjectProto(dwVnum);
					//		if (dependent)
					{
						// 지어져있는가?
						if (!pkLand->FindObjectByGroup(t->dwDependOnGroupVnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "[LS;748]");
							return;
						}
					}
				}

				if (test_server || GMLevel == GM_PLAYER)
				{
					// GM이 아닐경우만 (테섭에서는 GM도 소모)
					// 건설 비용 체크
					if (t->dwPrice > BUILDING_MAX_PRICE)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;749]");
						return;
					}

#ifdef ENABLE_USE_MONEY_FROM_GUILD
					CGuild* g = ch->GetGuild();
					if (g->GetGuildMoney() < (int)t->dwPrice)
#else
					if (ch->GetGold() < (int)t->dwPrice)
#endif
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;750]");
						return;
					}

					// 아이템 자재 개수 체크

					int i;
					for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
					{
						uint32_t dwItemVnum = t->kMaterials[i].dwItemVnum;
						uint32_t dwItemCount = t->kMaterials[i].dwCount;

						if (dwItemVnum == 0)
							break;

						if ((int) dwItemCount > ch->CountSpecifyItem(dwItemVnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, "[LS;751]");
							return;
						}
					}
				}

				float x_rot = atof(arg4);
				float y_rot = atof(arg5);
				float z_rot = atof(arg6);
				// 20050811.myevan.건물 회전 기능 봉인 해제
				/*
				   if (x_rot != 0.0f || y_rot != 0.0f || z_rot != 0.0f)
				   {
				   ch->ChatPacket(CHAT_TYPE_INFO, "건물 회전 기능은 아직 제공되지 않습니다");
				   return;
				   }
				 */

				long map_x = 0;
				str_to_number(map_x, arg2);
				long map_y = 0;
				str_to_number(map_y, arg3);

				bool isSuccess = pkLand->RequestCreateObject(dwVnum,
						ch->GetMapIndex(),
						map_x,
						map_y,
						x_rot,
						y_rot,
						z_rot, true);

				if (!isSuccess)
				{
					if (test_server)
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;752]");
					return;
				}

				//if (test_server || GMLevel == GM_PLAYER)
				if (!test_server)	//@fixme529
				{
#ifdef ENABLE_USE_MONEY_FROM_GUILD
					CGuild* g = ch->GetGuild();
					if (!g)
						return;
					g->RequestWithdrawMoney(ch, t->dwPrice, false);
					ch->PointChange(POINT_GOLD, -static_cast<int>(t->dwPrice));	//@fixme501
#else
					//ch->PointChange(POINT_GOLD, -t->dwPrice);
					ch->PointChange(POINT_GOLD, -static_cast<int>(t->dwPrice));	//@fixme501
#endif

					// 아이템 자재 사용하기
					{
						int i;
						for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
						{
							uint32_t dwItemVnum = t->kMaterials[i].dwItemVnum;
							uint32_t dwItemCount = t->kMaterials[i].dwCount;

							if (dwItemVnum == 0)
								break;

							sys_log(0, "BUILD: material %d %u %u", i, dwItemVnum, dwItemCount);
							ch->RemoveSpecifyItem(dwItemVnum, dwItemCount);
						}
					}
				}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
				if (dwVnum == 14071) {
					if (!ch->GetGuild())
						return;

					ch->GetGuild()->SetGuildstorage(1);
					//auto msgUpdate(DBManager::Instance().DirectQuery("UPDATE player.guild SET guildstorage = 1 WHERE id = %d", ch->GetGuild()->GetID()));
					sys_log(0, "GUILDSTORAGE_BUILD_DONE!");
				}
#endif
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_EXTENDED_LOGS)
				if (dwVnum != 14062 || dwVnum != 14063 || dwVnum != 14072 || dwVnum != 14073) {
					if (t->dwGroupVnum == 1 || t->dwGroupVnum == 2 || t->dwGroupVnum == 3 || t->dwGroupVnum == 4 /*14062, 14063*/ || t->dwGroupVnum == 6 /*14072, 14073*/) {
						LogManager::Instance().GuildLog(ch, ch->GetGuild()->GetID(), dwVnum, "Object", GUILD_GOLD_TYPE_OBJECT_CREATE, t->dwPrice, 1);
						sys_log(0, "BUILD_LOG_OBJECT: %d", dwVnum);
					}
				}
#endif
			}
			break;

		case 'd' :
			// build (d)elete ObjectID
			{
				one_argument(line, arg1, sizeof(arg1));

				if (!*arg1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
					return;
				}

				uint32_t vid = 0;
				str_to_number(vid, arg1);
				pkLand->RequestDeleteObjectByVID(vid);
			}
			break;

			// BUILD_WALL

			// build w n/e/w/s
		case 'w' :
			if (GMLevel > GM_PLAYER)
			{
				int mapIndex = ch->GetMapIndex();

				one_argument(line, arg1, sizeof(arg1));

				sys_log(0, "guild.wall.build map[%d] direction[%s]", mapIndex, arg1);

				switch (arg1[0])
				{
					case 's':
						pkLand->RequestCreateWall(mapIndex,   0.0f);
						break;
					case 'n':
						pkLand->RequestCreateWall(mapIndex, 180.0f);
						break;
					case 'e':
						pkLand->RequestCreateWall(mapIndex,  90.0f);
						break;
					case 'w':
						pkLand->RequestCreateWall(mapIndex, 270.0f);
						break;
					default:
						ch->ChatPacket(CHAT_TYPE_INFO, "guild.wall.build unknown_direction[%s]", arg1);
						sys_err("guild.wall.build unknown_direction[%s]", arg1);
						break;
				}

			}
			break;

		case 'e':
			if (GMLevel > GM_PLAYER)
			{
				pkLand->RequestDeleteWall();
			}
			break;

		case 'W' :
			// 담장 세우기
			// build (w)all 담장번호 담장크기 대문동 대문서 대문남 대문북

			if (GMLevel > GM_PLAYER)
			{
				int setID = 0, wallSize = 0;
				char arg5[256], arg6[256];
				line = two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2));
				line = two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));
				two_arguments(line, arg5, sizeof(arg5), arg6, sizeof(arg6));

				str_to_number(setID, arg1);
				str_to_number(wallSize, arg2);

				if (setID != 14105 && setID != 14115 && setID != 14125)
				{
					sys_log(0, "BUILD_WALL: wrong wall set id %d", setID);
					break;
				}
				else
				{
					bool door_east = false;
					str_to_number(door_east, arg3);
					bool door_west = false;
					str_to_number(door_west, arg4);
					bool door_south = false;
					str_to_number(door_south, arg5);
					bool door_north = false;
					str_to_number(door_north, arg6);
					pkLand->RequestCreateWallBlocks(setID, ch->GetMapIndex(), wallSize, door_east, door_west, door_south, door_north);
				}
			}
			break;

		case 'E' :
			// 담장 지우기
			// build (e)rase 담장셋ID
			if (GMLevel > GM_PLAYER)
			{
				one_argument(line, arg1, sizeof(arg1));
				uint32_t id = 0;
				str_to_number(id, arg1);
				pkLand->RequestDeleteWallBlocks(id);
			}
			break;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, "Invalid command %s", arg1);
			break;
	}
}
// END_OF_BUILD_BUILDING

ACMD(do_clear_quest)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	pPC->ClearQuest(arg1);
}

ACMD(do_horse_state)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Horse Information:");
	ch->ChatPacket(CHAT_TYPE_INFO, "    Level  %d", ch->GetHorseLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Health %d/%d (%d%%)", ch->GetHorseHealth(), ch->GetHorseMaxHealth(), ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Stam   %d/%d (%d%%)", ch->GetHorseStamina(), ch->GetHorseMaxStamina(), ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina());
}

ACMD(do_horse_level)
{
	char arg1[256] = {0};
	char arg2[256] = {0};
	LPCHARACTER victim;
	int	level = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /horse_level <name> <level>");
		return;
	}

	victim = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (nullptr == victim)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;753]");
		return;
	}

	str_to_number(level, arg2);
	level = MINMAX(0, level, HORSE_MAX_LEVEL);

	ch->ChatPacket(CHAT_TYPE_INFO, "horse level set (%s: %d)", victim->GetName(), level);

	victim->SetHorseLevel(level);
	victim->ComputePoints();
	victim->SkillLevelPacket();
	return;

/*-----
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int level = MINMAX(0, atoi(arg1), HORSE_MAX_LEVEL);

	ch->ChatPacket(CHAT_TYPE_INFO, "horse level set to %d.", level);
	ch->SetHorseLevel(level);
	ch->ComputePoints();
	ch->SkillLevelPacket();
	return;
-----*/
}

ACMD(do_horse_ride)
{
	if (ch->IsHorseRiding())
		ch->StopRiding();
	else
		ch->StartRiding();
}

ACMD(do_horse_summon)
{
	ch->HorseSummon(true, true);
}

ACMD(do_horse_unsummon)
{
	ch->HorseSummon(false, true);
}

ACMD(do_horse_set_stat)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		int hp = 0;
		str_to_number(hp, arg1);
		int stam = 0;
		str_to_number(stam, arg2);
		ch->UpdateHorseHealth(hp - ch->GetHorseHealth());
		ch->UpdateHorseStamina(stam - ch->GetHorseStamina());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage : /horse_set_stat <hp> <stamina>");
	}
}

ACMD(do_save_attribute_to_image) // command "/saveati" for alias
{
	char szFileName[256];
	char szMapIndex[256];

	two_arguments(argument, szMapIndex, sizeof(szMapIndex), szFileName, sizeof(szFileName));

	if (!*szMapIndex || !*szFileName)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /saveati <map_index> <filename>");
		return;
	}

	long lMapIndex = 0;
	str_to_number(lMapIndex, szMapIndex);

	if (SECTREE_MANAGER::Instance().SaveAttributeToImage(lMapIndex, szFileName))
		ch->ChatPacket(CHAT_TYPE_INFO, "Save done.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Save failed.");
}

ACMD(do_affect_remove)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <player name>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <type> <point>");

		LPCHARACTER tch = ch;

		if (*arg1)
			if (!(tch = CHARACTER_MANAGER::Instance().FindPC(arg1)))
				tch = ch;

		ch->ChatPacket(CHAT_TYPE_INFO, "-- Affect List of %s -------------------------------", tch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, "Type Point Modif Duration Flag");

		const std::list<CAffect *> & cont = tch->GetAffectContainer();

		itertype(cont) it = cont.begin();

		while (it != cont.end())
		{
			CAffect * pkAff = *it++;

			ch->ChatPacket(CHAT_TYPE_INFO, "%4d %5d %5d %8d %u",
					pkAff->dwType, pkAff->wApplyOn, pkAff->lApplyValue, pkAff->lDuration, pkAff->dwFlag);
		}
		return;
	}

	bool removed = false;

	CAffect * af;

	uint32_t	type = 0;
	str_to_number(type, arg1);
	uint16_t	point = 0;	//@fixme532
	str_to_number(point, arg2);
	while ((af = ch->FindAffect(type, point)))
	{
		ch->RemoveAffect(af);
		removed = true;
	}

	if (removed)
		ch->ChatPacket(CHAT_TYPE_INFO, "Affect successfully removed.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Not affected by that type and point.");
}

ACMD(do_change_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeAttribute();
}

ACMD(do_add_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddAttribute();
}

ACMD(do_add_socket)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddSocket();
}

#ifdef ENABLE_NEWSTUFF
ACMD(do_change_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeRareAttribute();
}

ACMD(do_add_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddRareAttribute();
}
#endif

ACMD(do_show_arena_list)
{
	CArenaManager::Instance().SendArenaMapListTo(ch);
}

ACMD(do_end_all_duel)
{
	CArenaManager::Instance().EndAllDuel();
}

ACMD(do_end_duel)
{
	char szName[256];

	one_argument(argument, szName, sizeof(szName));

	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindPC(szName);
	if (pChar == nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;753]");
		return;
	}

	if (CArenaManager::Instance().EndDuel(pChar->GetPlayerID()) == false)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;754]");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;755]");
	}
}

ACMD(do_duel)
{
	char szName1[256];
	char szName2[256];
	char szSet[256];
	char szMinute[256];
	int set = 0;
	int minute = 0;

	argument = two_arguments(argument, szName1, sizeof(szName1), szName2, sizeof(szName2));
	two_arguments(argument, szSet, sizeof(szSet), szMinute, sizeof(szMinute));

	str_to_number(set, szSet);

	if (set < 0) set = 1;
	if (set > 5) set = 5;

	if (!str_to_number(minute, szMinute))
		minute = 5;

	if (minute < 5)
		minute = 5;

	LPCHARACTER pChar1 = CHARACTER_MANAGER::Instance().FindPC(szName1);
	LPCHARACTER pChar2 = CHARACTER_MANAGER::Instance().FindPC(szName2);

	if (pChar1 != nullptr && pChar2 != nullptr)
	{
		pChar1->RemoveGoodAffect();
		pChar2->RemoveGoodAffect();

		pChar1->RemoveBadAffect();
		pChar2->RemoveBadAffect();

		LPPARTY pParty = pChar1->GetParty();
		if (pParty != nullptr)
		{
			if (pParty->GetMemberCount() == 2)
			{
				CPartyManager::Instance().DeleteParty(pParty);
			}
			else
			{
				pChar1->ChatPacket(CHAT_TYPE_INFO, "[LS;532]");
				pParty->Quit(pChar1->GetPlayerID());
			}
		}

		pParty = pChar2->GetParty();
		if (pParty != nullptr)
		{
			if (pParty->GetMemberCount() == 2)
			{
				CPartyManager::Instance().DeleteParty(pParty);
			}
			else
			{
				pChar2->ChatPacket(CHAT_TYPE_INFO, "[LS;532]");
				pParty->Quit(pChar2->GetPlayerID());
			}
		}

		if (CArenaManager::Instance().StartDuel(pChar1, pChar2, set, minute))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;756]");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;757]");
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;759]");
	}
}

ACMD(do_stat_plus_amount)
{
	char szPoint[256];

	one_argument(argument, szPoint, sizeof(szPoint));

	if (*szPoint == '\0')
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
		return;
	}

	int nRemainPoint = ch->GetPoint(POINT_STAT);

	if (nRemainPoint <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;760]");
		return;
	}

	int nPoint = 0;
	str_to_number(nPoint, szPoint);

	if (nRemainPoint < nPoint)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;761]");
		return;
	}

	if (nPoint < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;762]");
		return;
	}

#ifndef ENABLE_STATPLUS_NOLIMIT
	switch (subcmd)
	{
		case POINT_HT : // 체력
			if (nPoint + ch->GetPoint(POINT_HT) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_HT);
			}
			break;

		case POINT_IQ : // 지능
			if (nPoint + ch->GetPoint(POINT_IQ) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_IQ);
			}
			break;

		case POINT_ST : // 근력
			if (nPoint + ch->GetPoint(POINT_ST) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_ST);
			}
			break;

		case POINT_DX : // 민첩
			if (nPoint + ch->GetPoint(POINT_DX) > 90)
			{
				nPoint = 90 - ch->GetPoint(POINT_DX);
			}
			break;

		default :
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;763]");
			return;
			break;
	}
#endif

	if (nPoint != 0)
	{
		ch->SetRealPoint(subcmd, ch->GetRealPoint(subcmd) + nPoint);
		ch->SetPoint(subcmd, ch->GetPoint(subcmd) + nPoint);
		ch->ComputePoints();
		ch->PointChange(subcmd, 0);

		ch->PointChange(POINT_STAT, -nPoint);
		ch->ComputePoints();
	}
}

struct tTwoPID
{
	int pid1;
	int pid2;
};

ACMD(do_break_marriage)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	tTwoPID pids = { 0, 0 };

	str_to_number(pids.pid1, arg1);
	str_to_number(pids.pid2, arg2);

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("플레이어 %d 와 플레이어  %d를 파혼시킵니다.."), pids.pid1, pids.pid2);
	db_clientdesc->DBPacket(HEADER_GD_BREAK_MARRIAGE, 0, &pids, sizeof(pids));
}

ACMD(do_effect)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	int	effect_type = 0;
	str_to_number(effect_type, arg1);
	ch->EffectPacket(effect_type);
}


struct FCountInMap
{
	int m_Count[4];
	FCountInMap() { memset(m_Count, 0, sizeof(int) * 4); }
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch && ch->IsPC())
				++m_Count[ch->GetEmpire()];
		}
	}
	int GetCount(uint8_t bEmpire) { return m_Count[bEmpire]; }
};

ACMD(do_threeway_war_info)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;767]");
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;768;%d;%d;%d;%d]", GetSungziMapIndex(), GetPassMapIndex(1), GetPassMapIndex(2), GetPassMapIndex(3));
	ch->ChatPacket(CHAT_TYPE_INFO, "ThreewayPhase %d", CThreeWayWar::Instance().GetRegenFlag());

	for (int n = 1; n < 4; ++n)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(GetSungziMapIndex());

		FCountInMap c;

		if (pSecMap)
		{
			pSecMap->for_each(c);
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "%s killscore %d usercount %d",
				EMPIRE_NAME(n),
			   	CThreeWayWar::Instance().GetKillScore(n),
				c.GetCount(n));
	}
}

ACMD(do_threeway_war_myinfo)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;771]");
	ch->ChatPacket(CHAT_TYPE_INFO, "Deadcount %d",
			CThreeWayWar::Instance().GetReviveTokenForPlayer(ch->GetPlayerID()));
}

ACMD(do_rmcandidacy)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: rmcandidacy <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target is in %d channel (my channel %d)", pkCCI->bChannel, g_bChannel);
				return;
			}
		}
	}

	db_clientdesc->DBPacket(HEADER_GD_RMCANDIDACY, 0, nullptr, 32);
	db_clientdesc->Packet(arg1, 32);
}

ACMD(do_setmonarch)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: setmonarch <name>");
		return;
	}

	db_clientdesc->DBPacket(HEADER_GD_SETMONARCH, 0, nullptr, 32);
	db_clientdesc->Packet(arg1, 32);
}

ACMD(do_rmmonarch)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: rmmonarch <name>");
		return;
	}

	db_clientdesc->DBPacket(HEADER_GD_RMMONARCH, 0, nullptr, 32);
	db_clientdesc->Packet(arg1, 32);
}

ACMD(do_check_monarch_money)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int empire = 0;
	str_to_number(empire, arg1);
	int NationMoney = CMonarch::Instance().GetMoney(empire);

	ch->ChatPacket(CHAT_TYPE_INFO, "국고: %d 원", NationMoney);
}

ACMD(do_reset_subskill)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: reset_subskill <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (tch == nullptr)
		return;

	tch->ClearSubSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "Subskill of [%s] was reset", tch->GetName());
}

ACMD(do_siege)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int	empire = strtol(arg1, nullptr, 10);
	int tower_count = strtol(arg2, nullptr, 10);

	if (empire == 0) empire = number(1, 3);
	if (tower_count < 5 || tower_count > 10) tower_count = number(5, 10);

	TPacketGGSiege packet{};
	packet.bHeader = HEADER_GG_SIEGE;
	packet.bEmpire = empire;
	packet.bTowerCount = tower_count;

	P2P_MANAGER::Instance().Send(&packet, sizeof(TPacketGGSiege));

	switch (castle_siege(empire, tower_count))
	{
		case 0 :
			ch->ChatPacket(CHAT_TYPE_INFO, "SIEGE FAILED");
			break;
		case 1 :
			ch->ChatPacket(CHAT_TYPE_INFO, "SIEGE START Empire(%d) Tower(%d)", empire, tower_count);
			break;
		case 2 :
			ch->ChatPacket(CHAT_TYPE_INFO, "SIEGE END");
			break;
	}
}

ACMD(do_temp)
{
	if (false == test_server)
		return;

	char	arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (0 == arg1[0] || 0 == arg2[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: empire money");
		return;
	}

	int	empire = 0;
	str_to_number(empire, arg1);
	int	money = 0;
	str_to_number(money, arg2);

	CMonarch::Instance().SendtoDBAddMoney(money, empire, ch);
}

ACMD(do_frog)
{
	char	arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: empire(1, 2, 3)");
		return;
	}

	int	empire = 0;
	str_to_number(empire, arg1);

	switch (empire)
	{
		case 1:
		case 2:
		case 3:
			if (IS_CASTLE_MAP(ch->GetMapIndex()))
			{
				castle_spawn_frog(empire);
				castle_save();
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "You must spawn frog in castle");
			break;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: empire(1, 2, 3)");
			break;
	}

}

/*ACMD(do_flush)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /flush player_id");
		return;
	}

	uint32_t pid = (uint32_t) strtoul(arg1, nullptr, 10);

	db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
}*/

ACMD(do_flush)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	uint32_t pid = 0;
	if (0 == arg1[0]) {
		pid = ch->GetPlayerID();
	}
	else {
		pid = (uint32_t)strtoul(arg1, nullptr, 10);
	}

	if (pid != 0) {
		ch->SaveReal();
		db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
		db_clientdesc->Packet(&pid, sizeof(uint32_t));
	}
}

ACMD(do_eclipse)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (strtol(arg1, nullptr, 10) == 1)
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("eclipse", DayMode::NIGHT);
	}
	else
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("eclipse", DayMode::DAY);
	}
}

ACMD(do_weeklyevent)
{
	char arg1[256];
	int empire = 0;

	if (CBattleArena::Instance().IsRunning() == false)
	{
		one_argument(argument, arg1, sizeof(arg1));

		empire = strtol(arg1, nullptr, 10);

		if (empire == 1 || empire == 2 || empire == 3)
		{
			CBattleArena::Instance().Start(empire);
		}
		else
		{
			CBattleArena::Instance().Start(rand()%3 + 1);
		}
		ch->ChatPacket(CHAT_TYPE_INFO, "Weekly Event Start");
	}
	else
	{
		CBattleArena::Instance().ForceEnd();
		ch->ChatPacket(CHAT_TYPE_INFO, "Weekly Event End");
	}
}

ACMD(do_event_helper)
{
	char arg1[256];
	int mode = 0;

	one_argument(argument, arg1, sizeof(arg1));
	str_to_number(mode, arg1);

	if (mode == 1)
	{
		xmas::SpawnEventHelper(true);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Spawn");
	}
	else
	{
		xmas::SpawnEventHelper(false);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Delete");
	}
}

struct FMobCounter
{
	int nCount;

	void operator () (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (pChar->IsMonster() || pChar->IsStone())
			{
				nCount++;
			}
		}
	}
};

ACMD(do_get_mob_count)
{
	LPSECTREE_MAP pSectree = SECTREE_MANAGER::Instance().GetMap(ch->GetMapIndex());

	if (pSectree == nullptr)
		return;

	FMobCounter f;
	f.nCount = 0;

	pSectree->for_each(f);

	ch->ChatPacket(CHAT_TYPE_INFO, "MapIndex: %d MobCount %d", ch->GetMapIndex(), f.nCount);
}

ACMD(do_clear_land)
{
	const building::CLand* pLand = building::CManager::Instance().FindLand(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	if( nullptr == pLand )
	{
		return;
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "Guild Land(%d) Cleared", pLand->GetID());

	building::CManager::Instance().ClearLand(pLand->GetID());
}

ACMD(do_special_item)
{
    ITEM_MANAGER::Instance().ConvSpecialDropItemFile();
}

ACMD(do_set_stat)
{
	char szName [256];
	char szChangeAmount[256];

	two_arguments (argument, szName, sizeof (szName), szChangeAmount, sizeof(szChangeAmount));

	if (!*szName || *szChangeAmount == '\0')
	{
		ch->ChatPacket (CHAT_TYPE_INFO, "Invalid argument.");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(szName);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(szName);

		if (pkCCI)
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). %s is not in your game server.", szName, szName);
			return;
		}
		else
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). Perhaps %s doesn't login or exist.", szName, szName);
			return;
		}
	}
	else
	{
		if (tch->IsPolymorphed())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
			return;
		}

		if (subcmd != POINT_HT && subcmd != POINT_IQ && subcmd != POINT_ST && subcmd != POINT_DX)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;763]");
			return;
		}
		int nRemainPoint = tch->GetPoint(POINT_STAT);
		int nCurPoint = tch->GetRealPoint(subcmd);
		int nChangeAmount = 0;
		str_to_number(nChangeAmount, szChangeAmount);
		int nPoint = nCurPoint + nChangeAmount;

		int n = -1;
		switch (subcmd)
		{
			case POINT_HT:
				if (nPoint < JobInitialPoints[tch->GetJob()].ht)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 0;
				break;

			case POINT_IQ:
				if (nPoint < JobInitialPoints[tch->GetJob()].iq)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 1;
				break;

			case POINT_ST:
				if (nPoint < JobInitialPoints[tch->GetJob()].st)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 2;
				break;

			case POINT_DX:
				if (nPoint < JobInitialPoints[tch->GetJob()].dx)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 3;
				break;
		}

		if (nPoint > 90)
		{
			nChangeAmount -= nPoint - 90;
			nPoint = 90;
		}

		if (nRemainPoint < nChangeAmount)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;761]");
			return;
		}

		tch->SetRealPoint(subcmd, nPoint);
		tch->SetPoint(subcmd, tch->GetPoint(subcmd) + nChangeAmount);
		tch->ComputePoints();
		tch->PointChange(subcmd, 0);

		tch->PointChange(POINT_STAT, -nChangeAmount);
		tch->ComputePoints();

		const char* stat_name[4] = {"con", "int", "str", "dex"};
		if (-1 == n)
			return;
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s change %d to %d", szName, stat_name[n], nCurPoint, nPoint);
	}
}

ACMD(do_get_item_id_list)
{
	for (int i = 0; i < INVENTORY_SLOT_COUNT; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

ACMD(do_set_socket)
{
	char arg1 [256];
	char arg2 [256];
	char arg3 [256];

	one_argument (two_arguments (argument, arg1, sizeof (arg1), arg2, sizeof(arg2)), arg3, sizeof (arg3));

	int item_id, socket_num, value;
	if (!str_to_number (item_id, arg1) || !str_to_number (socket_num, arg2) || !str_to_number (value, arg3))
		return;

	LPITEM item = ITEM_MANAGER::Instance().Find (item_id);
	if (item)
		item->SetSocket (socket_num, value);
}

ACMD (do_can_dead)
{
	if (subcmd)
		ch->SetArmada();
	else
		ch->ResetArmada();
}

ACMD (do_all_skill_master)
{
	for (int i = 0; i < SKILL_MAX_NUM; i++) {
		if (ch->CanUseSkill(i)) {
			switch(i) {
				// @fixme154 BEGIN
				// taking out the it->second->bMaxLevel from map_pkSkillProto (&& 1==40|SKILL_MAX_LEVEL) will be very resource-wasting, so we go full ugly so far
				case SKILL_COMBO:
					ch->SetSkillLevel(i, 2);
					break;

				case SKILL_LANGUAGE1:
				case SKILL_LANGUAGE2:
				case SKILL_LANGUAGE3:
					ch->SetSkillLevel(i, 20);
					break;

				case SKILL_HORSE_SUMMON:
					ch->SetSkillLevel(i, 10);
					break;

				case SKILL_HORSE:
					ch->SetSkillLevel(i, HORSE_MAX_LEVEL);
					break;

				// CanUseSkill will be true for skill_horse_skills if riding
				case SKILL_HORSE_WILDATTACK:
				case SKILL_HORSE_CHARGE:
				case SKILL_HORSE_ESCAPE:
				case SKILL_HORSE_WILDATTACK_RANGE:
					ch->SetSkillLevel(i, 20);
					break;

				case SKILL_ADD_HP:
				case SKILL_RESIST_PENETRATE:
					ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
					break;

				// @fixme154 END
				default:
					ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
					break;
			}
		}
		else {
			switch(i) {
				case SKILL_HORSE_WILDATTACK:
				case SKILL_HORSE_CHARGE:
				case SKILL_HORSE_ESCAPE:
				case SKILL_HORSE_WILDATTACK_RANGE:
					ch->SetSkillLevel(i, 20); // @fixme154 40 -> 20
					break;
			}
		}
	}

	ch->SetHorseLevel(HORSE_MAX_LEVEL);
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD (do_item_full_set)
{
	uint8_t job = ch->GetJob();
	LPITEM item;
	for (int i = 0; i < 6; i++)
	{
		item = ch->GetWear(i);
		if (item != nullptr)
			ch->UnequipItem(item);
	}
	item = ch->GetWear(WEAR_SHIELD);
	if (item != nullptr)
		ch->UnequipItem(item);

	// Schuhe Feuerschuhe +9
	item = ITEM_MANAGER::Instance().CreateItem(15449);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	// Armband Himmelstr?enarmband+9
	item = ITEM_MANAGER::Instance().CreateItem(14209);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	// Halskette Himmelstr?enhalskette+9
	item = ITEM_MANAGER::Instance().CreateItem(16209);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	// Schild Titanenschild+9
	item = ITEM_MANAGER::Instance().CreateItem(13149);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	//G?tel Seeleng?tel+9
	item = ITEM_MANAGER::Instance().CreateItem(18089);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	// Ring1 Ring der Willenskraft
	item = ITEM_MANAGER::Instance().CreateItem(71148);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	//Ring2 Ring der t?lichen Macht
	item = ITEM_MANAGER::Instance().CreateItem(71149);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	//Unique1 Windschuhe+ (handelbar)
	item = ITEM_MANAGER::Instance().CreateItem(72702);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);

	if (SEX_MALE == GET_SEX(ch))
	{
		// C_Body S. Dokkaebi-Gewand+ (m)
		item = ITEM_MANAGER::Instance().CreateItem(41818);
		if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
			M2_DESTROY_ITEM(item);

		// C_Hair S. Dokkaebi-H?ner+ (m)
		item = ITEM_MANAGER::Instance().CreateItem(45558);
		if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
			M2_DESTROY_ITEM(item);
	}
	if (SEX_FEMALE == GET_SEX(ch))
	{
		// C_Body S. Dokkaebi-Gewand+ (w)
		item = ITEM_MANAGER::Instance().CreateItem(41820);
		if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
			M2_DESTROY_ITEM(item);

		// C_Hair S. Dokkaebi-H?ner+ (w)
		item = ITEM_MANAGER::Instance().CreateItem(45560);
		if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
			M2_DESTROY_ITEM(item);
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	// C_Mount Manni
	item = ITEM_MANAGER::Instance().CreateItem(71224);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	// C_Acce Herrscherband (ma?ef.)
	item = ITEM_MANAGER::Instance().CreateItem(85004);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);
#endif
#ifdef ENABLE_PENDANT
	// Pendant Feuertalisman+200
	item = ITEM_MANAGER::Instance().CreateItem(9800);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);
#endif
#ifdef ENABLE_AURA_SYSTEM
	// C_Aura Strahlendes Auragewand
	item = ITEM_MANAGER::Instance().CreateItem(49006);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);
#endif
#ifdef ENABLE_PET_SYSTEM
	// C_Pet_Slot Griffy
	item = ITEM_MANAGER::Instance().CreateItem(53263);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);
#endif
#ifdef ENABLE_GLOVE_SYSTEM
	// C_Glove_Slot Schlangenhands.(STR) +9
	item = ITEM_MANAGER::Instance().CreateItem(23059);
	if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
		M2_DESTROY_ITEM(item);
#endif

	switch (job)
	{
		case JOB_SURA:
			{
				// Waffe Schlangenklinge+15
				item = ITEM_MANAGER::Instance().CreateItem(395);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// R?tung B?er Schlangenmantel+15
				item = ITEM_MANAGER::Instance().CreateItem(21365);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Helm Drachenpanzerhelm+9
				item = ITEM_MANAGER::Instance().CreateItem(12719);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Ohrringe Rubinohrringe+9
				item = ITEM_MANAGER::Instance().CreateItem(17509);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
				// C_Weapon Schwert des Dokkaebi+
				item = ITEM_MANAGER::Instance().CreateItem(40150);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);
#endif
			}
			break;

		case JOB_WARRIOR:
			{
				// Waffe Schlangenschwert+15
				item = ITEM_MANAGER::Instance().CreateItem(375);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// R?tung Schlangenmantel+15
				item = ITEM_MANAGER::Instance().CreateItem(21325);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Helm Drachenstahlhelm+9
				item = ITEM_MANAGER::Instance().CreateItem(12699);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Ohrringe Smaragdohrringe+9
				item = ITEM_MANAGER::Instance().CreateItem(17549);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
				// C_Weapon Schwert des Dokkaebi+
				item = ITEM_MANAGER::Instance().CreateItem(40150);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);
#endif
			}
			break;

		case JOB_SHAMAN:
			{
				// Waffe Schlangenglocke+15
				item = ITEM_MANAGER::Instance().CreateItem(5215);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// R?tung Rein. Schlangenmantel+15
				item = ITEM_MANAGER::Instance().CreateItem(21385);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Helm Drachenhut+9
				item = ITEM_MANAGER::Instance().CreateItem(12729);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Ohrringe Rubinohrringe+9
				item = ITEM_MANAGER::Instance().CreateItem(17509);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
				// C_Weapon Glocke des Dokkaebi+
				item = ITEM_MANAGER::Instance().CreateItem(40154);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);
#endif
			}
			break;

		case JOB_ASSASSIN:
			{
				// Waffe Schlangendolch+15
				item = ITEM_MANAGER::Instance().CreateItem(1225);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// R?tung Schlangenwams+15
				item = ITEM_MANAGER::Instance().CreateItem(21345);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Helm Drachenkapuze+9
				item = ITEM_MANAGER::Instance().CreateItem(12709);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Ohrringe Granatohrringe+9
				item = ITEM_MANAGER::Instance().CreateItem(17529);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
				// C_Weapon Dolch des Dokkaebi+
				item = ITEM_MANAGER::Instance().CreateItem(40151);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);
#endif
			}
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case JOB_WOLFMAN:
			{
				// Waffe Schlangenkralle+15
				item = ITEM_MANAGER::Instance().CreateItem(6165);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// R?tung Him. Schlangenmantel+15
				item = ITEM_MANAGER::Instance().CreateItem(21405);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Helm Drachenhornhelm+9
				item = ITEM_MANAGER::Instance().CreateItem(21559);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

				// Ohrringe Rubinohrringe+9
				item = ITEM_MANAGER::Instance().CreateItem(17509);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);

#	ifdef ENABLE_WEAPON_COSTUME_SYSTEM
				// C_Weapon Krallen des Dokkaebi+
				item = ITEM_MANAGER::Instance().CreateItem(40156);
				if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
					M2_DESTROY_ITEM(item);
#	endif
			}
			break;
#endif
	}
}

ACMD (do_attr_full_set)
{
	uint8_t job = ch->GetJob();
	LPITEM item;

	switch (job)
	{
		case JOB_WARRIOR:
		case JOB_ASSASSIN:
		case JOB_SURA:
		case JOB_SHAMAN:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case JOB_WOLFMAN:
#endif
		{
			item = ch->GetWear(WEAR_BODY);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetSocket(0, 28442);
				item->SetSocket(1, 28438);
				item->SetSocket(2, 28439);
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute(1, APPLY_RESIST_SWORD, 15);
				item->SetForceAttribute(2, APPLY_STEAL_HP, 10);
				item->SetForceAttribute(3, APPLY_ATT_GRADE_BONUS, 50);
				item->SetForceAttribute(4, APPLY_RESIST_BOW, 15);
			}

			item = ch->GetWear(WEAR_HEAD);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(1, APPLY_HP_REGEN, 30);
				item->SetForceAttribute(2, APPLY_ATTBONUS_HUMAN, 10);
				item->SetForceAttribute(3, APPLY_RESIST_MAGIC, 15);
				item->SetForceAttribute(4, APPLY_RESIST_WIND, 15);
			}

			item = ch->GetWear(WEAR_FOOTS);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute(1, APPLY_RESIST_SWORD, 15);
				item->SetForceAttribute(2, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute(3, APPLY_DODGE, 10);
				item->SetForceAttribute(4, APPLY_RESIST_BOW, 15);
			}

			item = ch->GetWear(WEAR_WRIST);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetSocket(0, 3);
				item->SetSocket(1, 3);
				item->SetSocket(2, 21600);
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute(1, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute(3, APPLY_STEAL_HP, 10);
				item->SetForceAttribute(4, APPLY_RESIST_MAGIC, 15);
			}

			item = ch->GetWear(WEAR_WEAPON);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetSocket(0, 28430);
				item->SetSocket(1, 28431);
				item->SetSocket(2, 28437);
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_STR, 12);
				item->SetForceAttribute(1, APPLY_DEX, 12);
				item->SetForceAttribute(2, APPLY_ATTBONUS_HUMAN, 10);
				item->SetForceAttribute(3, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute(4, APPLY_ATTBONUS_DEVIL, 20);
			}

			item = ch->GetWear(WEAR_NECK);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetSocket(0, 3);
				item->SetSocket(1, 3);
				item->SetSocket(2, 21600);
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute(1, APPLY_RESIST_SWORD, 15);
				item->SetForceAttribute(2, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute(3, APPLY_HP_REGEN, 30);
				item->SetForceAttribute(4, APPLY_RESIST_BOW, 15);
			}

			item = ch->GetWear(WEAR_EAR);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetSocket(0, 3);
				item->SetSocket(1, 3);
				item->SetSocket(2, 21600);
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_ATTBONUS_HUMAN, 10);
				item->SetForceAttribute(1, APPLY_RESIST_SWORD, 15);
				item->SetForceAttribute(2, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(3, APPLY_MANA_BURN_PCT, 10);
				item->SetForceAttribute(4, APPLY_RESIST_BOW, 15);
			}

			item = ch->GetWear(WEAR_SHIELD);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetForceAttribute(0, APPLY_STR, 12);
				item->SetForceAttribute(1, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(2, APPLY_BLOCK, 15);
				item->SetForceAttribute(3, APPLY_IMMUNE_STUN, 1);
				item->SetForceAttribute(4, APPLY_DEX, 12);
			}

			item = ch->GetWear(WEAR_BELT);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(0, 3);
				item->SetSocket(1, 3);
				item->SetSocket(2, 21600);
			}

#ifdef ENABLE_PENDANT
			item = ch->GetWear(WEAR_PENDANT);
			if (item != nullptr)
			{
				item->ClearAttribute();
#	ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#	endif
				item->SetForceAttribute(0, APPLY_STUN_PCT, 8);
				item->SetForceAttribute(1, APPLY_ATTBONUS_HUMAN, 10);
				item->SetForceAttribute(2, APPLY_RESIST_HUMAN, 10);
				item->SetForceAttribute(3, APPLY_ATTBONUS_CZ, 30);
				item->SetForceAttribute(4, APPLY_ATTBONUS_SWORD, 5);
			}
#endif

#ifdef ENABLE_GLOVE_SYSTEM
			item = ch->GetWear(WEAR_GLOVE);
			if (item != nullptr)
			{
				item->ClearAttribute();
#	ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#	endif
				item->SetForceAttribute(0, APPLY_STUN_PCT, 8);
				item->SetForceAttribute(1, APPLY_HIT_PCT, 12);
				item->SetForceAttribute(2, APPLY_RESIST_HUMAN, 10);
				item->SetForceAttribute(3, APPLY_REFLECT_MELEE, 15);
				item->SetForceAttribute(4, APPLY_STEAL_HP, 10);
			}
#endif

			item = ch->GetWear(WEAR_UNIQUE1);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(1, 0);
				item->SetSocket(2, 0);
				item->SetSocket(2, 520000);
			}

			item = ch->GetWear(WEAR_UNIQUE2);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(0, get_global_time() + 31536000);
				item->SetSocket(1, 2);
				item->SetSocket(2, 0);
			}

			item = ch->GetWear(WEAR_RING1);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(0, get_global_time() + 31536000);
				item->SetSocket(1, 2);
				item->SetSocket(2, 0);
			}

			item = ch->GetWear(WEAR_RING2);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(0, get_global_time() + 31536000);
				item->SetSocket(1, 2);
				item->SetSocket(2, 0);
			}

			item = ch->GetWear(WEAR_COSTUME_BODY);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(0, get_global_time() + 31536000);
				item->SetForceAttribute(0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute(1, APPLY_ATT_GRADE_BONUS, 50);
				item->SetForceAttribute(2, APPLY_STEAL_HP, 10);
			}

			item = ch->GetWear(WEAR_COSTUME_HAIR);
			if (item != nullptr)
			{
				item->ClearAttribute();
#ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#endif
				item->SetSocket(0, get_global_time() + 31536000);
				item->SetForceAttribute(0, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(1, APPLY_ATTBONUS_HUMAN, 10);
				item->SetForceAttribute(2, APPLY_POISON_PCT, 8);
			}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			item = ch->GetWear(WEAR_COSTUME_ACCE);
			if (item != nullptr)
			{
				item->ClearAttribute();
#	ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#	endif
				item->SetSocket(0, 2225);
				item->SetSocket(1, 25);
				item->SetSocket(2, 0);
			}
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			item = ch->GetWear(WEAR_COSTUME_WEAPON);
			if (item != nullptr)
			{
				item->ClearAttribute();
#	ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#	endif
				item->SetSocket(0, get_global_time() + 31536000);
				item->SetForceAttribute(0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute(1, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(2, APPLY_STEAL_HP, 10);
			}
#endif

#ifdef ENABLE_AURA_SYSTEM
			item = ch->GetWear(WEAR_COSTUME_AURA);
			if (item != nullptr)
			{
				item->ClearAttribute();
#	ifdef ENABLE_SEALBIND_SYSTEM
				if (ch->GetGMLevel() < GM_IMPLEMENTOR)
					item->SetSealDate(time(0) + 31536000);
#	endif
				//item->SetSocket(0, 250);
				//item->SetSocket(1, 13149);
				item->SetSocket(1, 125000000);
				//item->SetSocket(2, 0);
				item->SetForceAttribute(0, APPLY_STR, 12);
				item->SetForceAttribute(1, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute(2, APPLY_BLOCK, 15);
				item->SetForceAttribute(3, APPLY_IMMUNE_STUN, 1);
				item->SetForceAttribute(4, APPLY_DEX, 12);
			}
#endif
		}
		break;
	}
}

ACMD (do_full_set)
{
	do_all_skill_master(ch, nullptr, 0, 0);
	do_item_full_set(ch, nullptr, 0, 0);
	do_attr_full_set(ch, nullptr, 0, 0);
}

ACMD (do_use_item)
{
	char arg1 [256];

	one_argument (argument, arg1, sizeof (arg1));

	int cell = 0;
	str_to_number(cell, arg1);

	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		ch->UseItem(TItemPos (INVENTORY, cell));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "아이템이 없어서 착용할 수 없어.");
	}
}

ACMD (do_clear_affect)
{
	ch->ClearAffect(true);
}

ACMD (do_player_ban)
{
	if (ch && (ch->GetGMLevel() < GM_HIGH_WIZARD))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GM"));
		return;
	}

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: player_ban <name> <reason>");

		return;
	}

	const char* name = arg1;
	
	char arg2[256];
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (!*arg2 && ban_force_reason)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: player_ban <name> <reason>");

		return;
	}
	char* reason;

	if(!*arg2)
		reason = (char*) "";
	else
		reason = arg2;

	sys_log(0, "BAN %s", name);

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "can not find the target!");
		return;
	}
	
	if (ch)
		ch->ChatPacket(CHAT_TYPE_INFO, "ban requested.");

	if (tch && ch != tch)
	{
		if(tch->GetGMLevel() == GM_IMPLEMENTOR)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Can not ban/unban Administrator!");
		} else {
			tch->ban(reason, ch->GetPlayerID());
		}
	}
}

ACMD (do_player_unban)
{
	if (ch && (ch->GetGMLevel() < GM_HIGH_WIZARD))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GM"));
		return;
	}

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: player_unban <name>");

		return;
	}

	char* name = arg1;

	sys_log(0, "UNBAN %s", name);
	
	ch->ChatPacket(CHAT_TYPE_INFO, "player_unban requested.");
	
	ch->unban(name);
}

ACMD (do_dragon_soul)
{
	char arg1[512];
	const char* rest = one_argument (argument, arg1, sizeof(arg1));
	switch (arg1[0])
	{
		case 'a':
			{
				one_argument (rest, arg1, sizeof(arg1));
				int deck_idx;
				if (str_to_number(deck_idx, arg1) == false)
				{
					return;
				}
				ch->DragonSoul_ActivateDeck(deck_idx);
			}
			break;

		case 'd':
			{
				ch->DragonSoul_DeactivateAll();
			}
			break;
	}
}

ACMD (do_ds_list)
{
	for (int i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; i++)
	{
		TItemPos cell(DRAGON_SOUL_INVENTORY, i);

		LPITEM item = ch->GetItem(cell);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %d, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

ACMD(do_ds_qualify)
{
	if (!ch || !ch->IsGM())
		return;

	ch->DragonSoul_GiveQualification();
}

#ifdef ENABLE_BATTLE_FIELD
ACMD(do_battle_force_close)
{
	if (g_bChannel != BATTLE_FIELD_MAP_CHANNEL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Cant close in this channel, use channel %d.", BATTLE_FIELD_MAP_CHANNEL);
		return;
	}

	CBattleField::Instance().CloseEnter();
}

ACMD(do_battle_force_open)
{
	if (g_bChannel != BATTLE_FIELD_MAP_CHANNEL)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Cant open in this channel, use channel %d.", BATTLE_FIELD_MAP_CHANNEL);
		return;
	}

	CBattleField::Instance().OpenEnter(false, true);
}

ACMD(do_battle_set_event)
{
	char arg1[256]{};
	char arg2[256]{};

	int month = 0;
	int day = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: battle_set_event <month> <day>");
		return;
	}

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(month, arg1);
		str_to_number(day, arg2);
	}

	if (month <= 0 || month > 12)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Open event month need to be bettween 1 and 12."));
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("	1 = January."));
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("	12 = December."));
		return;
	}

	if (day <= 0 || day > 31)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Open event day need to be bettween 1 and 31."));
		return;
	}

	month = month - 1;

	CBattleField::Instance().SetEventInfo(month, day);
}
#endif

#ifdef ENABLE_CLEAN_BANNED_PLAYER_DATA
const std::map<const char*, const char*> _Deleter
{
	{"log.log", "who"},
	{"log.cube", "pid"},
	{"log.goldlog", "pid"},
	{"log.levellog", "pid"},
	{"log.speedhack", "pid"},
	{"log.refinelog", "pid"},
	{"log.quest_reward_log", "2"},
	{"log.loginlog", "pid"},
	{"log.loginlog2", "pid"},
	{"log.goldlog", "pid"},
	{"player.item", "owner_id"},
	{"player.player", "id"},
	{"player.guild_member", "pid"},
	{"player.affect", "dwPID"}
};

ACMD(do_clean)
{
	std::array<MYSQL_ROW, 2> row;
	auto msg(DBManager::Instance().DirectQuery("SELECT id FROM account.account WHERE availDt - NOW() > 0"));
	while ((row[0] = mysql_fetch_row(msg->Get()->pSQLResult))) {
		DBManager::Instance().DirectQuery("DELETE FROM player.player_index WHERE id = %d", std::atoi(row[0][0]));
		auto msg2(DBManager::Instance().DirectQuery("SELECT id FROM player.player WHERE account_id='%d'", std::atoi(row[0][0])));
		while ((row[1] = mysql_fetch_row(msg2->Get()->pSQLResult)))
			for (const auto&[table, where] : _Deleter)
				DBManager::Instance().DirectQuery("DELETE FROM %s WHERE %s = %d", table, where, std::atoi(row[1][0]));
	}
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
ACMD(do_shop_withdraw)
{
	char arg1[56];
	one_argument(argument, arg1, sizeof(arg1));

	uint32_t gold;
	if (!*arg1 || !str_to_number(gold, arg1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Usage: shop_withdraw <amount>"));
		return;
	}

	if (gold < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The amount of gold to withdraw should be above zero."));
		return;
	}

	if ((uint32_t)gold > ch->GetShopGoldStash())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You are trying to withdraw more gold than the %u you have!"), ch->GetShopGoldStash());
		return;
	}

	uint32_t pid = ch->GetPlayerID();
	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, ch->GetDesc()->GetHandle(), sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t));
	uint8_t subheader = SHOP_SUBHEADER_GD_WITHDRAW;
	db_clientdesc->Packet(&subheader, sizeof(uint8_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
	db_clientdesc->Packet(&gold, sizeof(uint32_t));
}

ACMD(do_shop_close)
{
	char arg1[56];
	uint32_t pid = 0;
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || !str_to_number(pid, arg1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Usage: shop_close <pid>"));
		return;
	}

	if (pid <= 0)
		return;

	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t));
	uint8_t subheader = SHOP_SUBHEADER_GD_CLOSE_REQUEST;
	db_clientdesc->Packet(&subheader, sizeof(uint8_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Shop closing requested for pid #%u"), pid);
}

ACMD(do_shop_moderate)
{
	char arg1[56];
	uint32_t pid = 0;
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || !str_to_number(pid, arg1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Usage: shop_moderate <pid>"));
		return;
	}

	if (pid <= 0)
		return;

	char newName[SHOP_SIGN_MAX_LEN + 1];
	strlcpy(newName, "Private shop", sizeof(newName));

	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(newName));
	uint8_t subheader = SHOP_SUBHEADER_GD_RENAME;
	db_clientdesc->Packet(&subheader, sizeof(uint8_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
	db_clientdesc->Packet(newName, sizeof(newName));

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Renaming shop %u to 'Private shop'"), pid);
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
ACMD(do_state_attr)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::Instance().Find(strtoul(arg1 + 1, nullptr, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);

			if (!d)
				tch = nullptr;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

	if (!tch)
		return;

	char buf[256];

	snprintf(buf, sizeof(buf), "%s's State: ", tch->GetName());
	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket(CHAT_TYPE_INFO, "6th-7hh ATTR:");

	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_SAMYEON:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_SAMYEON));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_TANHWAN:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_TANHWAN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_PALBANG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_PALBANG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_GIGONGCHAM:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_GIGONGCHAM));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_GYOKSAN:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_GYOKSAN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_GEOMPUNG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_GEOMPUNG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_AMSEOP:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_AMSEOP));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_GUNGSIN:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_GUNGSIN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_CHARYUN:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_CHARYUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_SANGONG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_SANGONG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_YEONSA:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_YEONSA));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_KWANKYEOK:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_KWANKYEOK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_GIGUNG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_GIGUNG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_HWAJO:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_HWAJO));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_SWAERYUNG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_SWAERYUNG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_YONGKWON:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_YONGKWON));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_PABEOB:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_PABEOB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_MARYUNG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_MARYUNG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_HWAYEOMPOK:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_HWAYEOMPOK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_MAHWAN:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_MAHWAN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_BIPABU:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_BIPABU));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_YONGBI:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_YONGBI));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_PAERYONG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_PAERYONG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_NOEJEON:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_NOEJEON));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_BYEURAK:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_BYEURAK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_CHAIN:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_CHAIN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_CHAYEOL:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_CHAYEOL));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_SALPOONG:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_SALPOONG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_GONGDAB:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_GONGDAB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_PASWAE:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_PASWAE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE:%3d%%", tch->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DEFEND_BONUS_BOSS_OR_MORE:%3d%%", tch->GetPoint(POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE:%3d%%", tch->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DAMAGE_BONUS_BOSS_OR_MORE:%3d%%", tch->GetPoint(POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_ENCHANT_FIRE:%3d%%", tch->GetPoint(POINT_HIT_BUFF_ENCHANT_FIRE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_ENCHANT_ICE:%3d%%", tch->GetPoint(POINT_HIT_BUFF_ENCHANT_ICE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_ENCHANT_ELEC:%3d%%", tch->GetPoint(POINT_HIT_BUFF_ENCHANT_ELEC));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_ENCHANT_WIND:%3d%%", tch->GetPoint(POINT_HIT_BUFF_ENCHANT_WIND));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_ENCHANT_DARK:%3d%%", tch->GetPoint(POINT_HIT_BUFF_ENCHANT_DARK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_ENCHANT_EARTH:%3d%%", tch->GetPoint(POINT_HIT_BUFF_ENCHANT_EARTH));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_RESIST_FIRE:%3d%%", tch->GetPoint(POINT_HIT_BUFF_RESIST_FIRE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_RESIST_ICE:%3d%%", tch->GetPoint(POINT_HIT_BUFF_RESIST_ICE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_RESIST_ELEC:%3d%%", tch->GetPoint(POINT_HIT_BUFF_RESIST_ELEC));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_RESIST_WIND:%3d%%", tch->GetPoint(POINT_HIT_BUFF_RESIST_WIND));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_RESIST_DARK:%3d%%", tch->GetPoint(POINT_HIT_BUFF_RESIST_DARK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HIT_BUFF_RESIST_EARTH:%3d%%", tch->GetPoint(POINT_HIT_BUFF_RESIST_EARTH));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHEONGRANG_MOV_SPEED:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHEONGRANG_MOV_SPEED));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHEONGRANG_CASTING_SPEED:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHEONGRANG_CASTING_SPEED));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHAYEOL_CRITICAL_PCT:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHAYEOL_CRITICAL_PCT));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SANGONG_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_SANGONG_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GIGUNG_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_GIGUNG_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_JEOKRANG_DEF_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_JEOKRANG_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GWIGEOM_DEF_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_GWIGEOM_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_TERROR_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_TERROR_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_MUYEONG_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_MUYEONG_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_MANASHILED_CASTING_SPEED:%3d%%", tch->GetPoint(POINT_USE_SKILL_MANASHILED_CASTING_SPEED));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_HOSIN_DEF_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_HOSIN_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GICHEON_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_GICHEON_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_JEONGEOP_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_JEUNGRYEOK_DEF_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_JEUNGRYEOK_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GIHYEOL_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHUNKEON_CASTING_SPEED:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHUNKEON_CASTING_SPEED));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_NOEGEOM_ATT_GRADE_BONUS:%3d%%", tch->GetPoint(POINT_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DURATION_INCREASE_EUNHYUNG:%3d%%", tch->GetPoint(POINT_SKILL_DURATION_INCREASE_EUNHYUNG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DURATION_INCREASE_GYEONGGONG:%3d%%", tch->GetPoint(POINT_SKILL_DURATION_INCREASE_GYEONGGONG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DURATION_INCREASE_GEOMKYUNG:%3d%%", tch->GetPoint(POINT_SKILL_DURATION_INCREASE_GEOMKYUNG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_DURATION_INCREASE_JEOKRANG:%3d%%", tch->GetPoint(POINT_SKILL_DURATION_INCREASE_JEOKRANG));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_PALBANG_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_PALBANG_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_AMSEOP_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_AMSEOP_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_YEONSA_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_YEONSA_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_YONGBI_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_YONGBI_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHAIN_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHAIN_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_PASWAE_SP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_PASWAE_SP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GIGONGCHAM_STUN:%3d%%", tch->GetPoint(POINT_USE_SKILL_GIGONGCHAM_STUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHARYUN_STUN:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHARYUN_STUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_PABEOB_STUN:%3d%%", tch->GetPoint(POINT_USE_SKILL_PABEOB_STUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_MAHWAN_STUN:%3d%%", tch->GetPoint(POINT_USE_SKILL_MAHWAN_STUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GONGDAB_STUN:%3d%%", tch->GetPoint(POINT_USE_SKILL_GONGDAB_STUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SAMYEON_STUN:%3d%%", tch->GetPoint(POINT_USE_SKILL_SAMYEON_STUN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GYOKSAN_KNOCKBACK:%3d%%", tch->GetPoint(POINT_USE_SKILL_GYOKSAN_KNOCKBACK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SEOMJEON_KNOCKBACK:%3d%%", tch->GetPoint(POINT_USE_SKILL_SEOMJEON_KNOCKBACK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SWAERYUNG_KNOCKBACK:%3d%%", tch->GetPoint(POINT_USE_SKILL_SWAERYUNG_KNOCKBACK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_HWAYEOMPOK_KNOCKBACK:%3d%%", tch->GetPoint(POINT_USE_SKILL_HWAYEOMPOK_KNOCKBACK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GONGDAB_KNOCKBACK:%3d%%", tch->GetPoint(POINT_USE_SKILL_GONGDAB_KNOCKBACK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_KWANKYEOK_KNOCKBACK:%3d%%", tch->GetPoint(POINT_USE_SKILL_KWANKYEOK_KNOCKBACK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_STONE:%3d%%", tch->GetPoint(POINT_ATTBONUS_STONE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   DAMAGE_HP_RECOVERY:%3d%%", tch->GetPoint(POINT_DAMAGE_HP_RECOVERY));
	ch->ChatPacket(CHAT_TYPE_INFO, "   DAMAGE_SP_RECOVERY:%3d%%", tch->GetPoint(POINT_DAMAGE_SP_RECOVERY));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ALIGNMENT_DAMAGE_BONUS:%3d%%", tch->GetPoint(POINT_ALIGNMENT_DAMAGE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   NORMAL_DAMAGE_GUARD:%3d%%", tch->GetPoint(POINT_NORMAL_DAMAGE_GUARD));
	ch->ChatPacket(CHAT_TYPE_INFO, "   MORE_THEN_HP90_DAMAGE_REDUCE:%3d%%", tch->GetPoint(POINT_MORE_THEN_HP90_DAMAGE_REDUCE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_TUSOK_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_TUSOK_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_PAERYONG_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_PAERYONG_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_BYEURAK_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_BYEURAK_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   FIRST_ATTRIBUTE_BONUS:%3d%%", tch->GetPoint(POINT_FIRST_ATTRIBUTE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SECOND_ATTRIBUTE_BONUS:%3d%%", tch->GetPoint(POINT_SECOND_ATTRIBUTE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   THIRD_ATTRIBUTE_BONUS:%3d%%", tch->GetPoint(POINT_THIRD_ATTRIBUTE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   FOURTH_ATTRIBUTE_BONUS:%3d%%", tch->GetPoint(POINT_FOURTH_ATTRIBUTE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   FIFTH_ATTRIBUTE_BONUS:%3d%%", tch->GetPoint(POINT_FIFTH_ATTRIBUTE_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER:%3d%%", tch->GetPoint(POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   USE_SKILL_CHAYEOL_HP_ABSORB:%3d%%", tch->GetPoint(POINT_USE_SKILL_CHAYEOL_HP_ABSORB));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_HUMAN:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_HUMAN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_ANIMAL:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_ANIMAL));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_ORC:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_ORC));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_MILGYO:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_MILGYO));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_UNDEAD:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_UNDEAD));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_DEVIL:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_DEVIL));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ENCHANT_PER_ELECT:%3d%%", tch->GetPoint(POINT_ENCHANT_PER_ELECT));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ENCHANT_PER_FIRE:%3d%%", tch->GetPoint(POINT_ENCHANT_PER_FIRE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ENCHANT_PER_ICE:%3d%%", tch->GetPoint(POINT_ENCHANT_PER_ICE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ENCHANT_PER_WIND:%3d%%", tch->GetPoint(POINT_ENCHANT_PER_WIND));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ENCHANT_PER_EARTH:%3d%%", tch->GetPoint(POINT_ENCHANT_PER_EARTH));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ENCHANT_PER_DARK:%3d%%", tch->GetPoint(POINT_ENCHANT_PER_DARK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_CZ:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_CZ));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_INSECT:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_INSECT));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_DESERT:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_DESERT));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_STONE:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_STONE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTBONUS_PER_MONSTER:%3d%%", tch->GetPoint(POINT_ATTBONUS_PER_MONSTER));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_HUMAN:%3d%%", tch->GetPoint(POINT_RESIST_PER_HUMAN));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_ICE:%3d%%", tch->GetPoint(POINT_RESIST_PER_ICE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_DARK:%3d%%", tch->GetPoint(POINT_RESIST_PER_DARK));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_EARTH:%3d%%", tch->GetPoint(POINT_RESIST_PER_EARTH));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_FIRE:%3d%%", tch->GetPoint(POINT_RESIST_PER_FIRE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_ELEC:%3d%%", tch->GetPoint(POINT_RESIST_PER_ELEC));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_MAGIC:%3d%%", tch->GetPoint(POINT_RESIST_PER_MAGIC));
	ch->ChatPacket(CHAT_TYPE_INFO, "   RESIST_PER_WIND:%3d%%", tch->GetPoint(POINT_RESIST_PER_WIND));
}
#endif

#ifdef ENABLE_PASSIVE_ATTR
ACMD(do_state_party)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::Instance().Find(strtoul(arg1 + 1, nullptr, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);

			if (!d)
				tch = nullptr;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

	if (!tch)
		return;

	char buf[256];

	snprintf(buf, sizeof(buf), "%s's State: ", tch->GetName());
	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket(CHAT_TYPE_INFO, "Party-Bonus:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATTACKER:%3d%%", tch->GetPoint(POINT_PARTY_ATTACKER_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   TANKER:%3d%%", tch->GetPoint(POINT_PARTY_TANKER_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   BUFFER:%3d%%", tch->GetPoint(POINT_PARTY_BUFFER_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL_MASTER:%3d%%", tch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   DEFENDER:%3d%%", tch->GetPoint(POINT_PARTY_DEFENDER_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "   HASTE:%3d%%", tch->GetPoint(POINT_PARTY_HASTE_BONUS));
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
ACMD(do_clevel)
{
	char arg2[256];
	one_argument(argument, arg2, sizeof(arg2));

	if (!*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: clevel <clevel>");
		return;
	}

	int	clevel = 0;
	str_to_number(clevel, arg2);

	ch->ResetConquerorPoint(MINMAX(0, clevel, gConquerorMaxLevel));

	//ch->ClearSkill();
	//ch->ClearSubSkill();
}

ACMD(do_conqueror_plus_amount)
{
	char szPoint[256];

	one_argument(argument, szPoint, sizeof(szPoint));

	if (*szPoint == '\0')
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
		return;
	}

	int nRemainPoint = ch->GetPoint(POINT_CONQUEROR_POINT);

	if (nRemainPoint <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;760]");
		return;
	}

	int nPoint = 0;
	str_to_number(nPoint, szPoint);

	if (nRemainPoint < nPoint)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;761]");
		return;
	}

	if (nPoint < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;762]");
		return;
	}

	switch (subcmd)
	{
		case POINT_SUNGMA_HP:
			if (nPoint + ch->GetPoint(POINT_SUNGMA_HP) > 30)
			{
				nPoint = 30 - ch->GetPoint(POINT_SUNGMA_HP);
			}
			break;

		case POINT_SUNGMA_IMMUNE:
			if (nPoint + ch->GetPoint(POINT_SUNGMA_IMMUNE) > 30)
			{
				nPoint = 30 - ch->GetPoint(POINT_SUNGMA_IMMUNE);
			}
			break;

		case POINT_SUNGMA_STR:
			if (nPoint + ch->GetPoint(POINT_SUNGMA_STR) > 30)
			{
				nPoint = 30 - ch->GetPoint(POINT_SUNGMA_STR);
			}
			break;

		case POINT_SUNGMA_MOVE:
			if (nPoint + ch->GetPoint(POINT_SUNGMA_MOVE) > 30)
			{
				nPoint = 30 - ch->GetPoint(POINT_SUNGMA_MOVE);
			}
			break;

		default :
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;763]");
			return;
			break;
	}

	if (nPoint != 0)
	{
		ch->SetRealPoint(subcmd, ch->GetRealPoint(subcmd) + nPoint);
		ch->SetPoint(subcmd, ch->GetPoint(subcmd) + nPoint);
		ch->ComputePoints();
		ch->PointChange(subcmd, 0);

		ch->PointChange(POINT_CONQUEROR_POINT, -nPoint);
		ch->ComputePoints();
	}
}

ACMD(do_set_conqueror)
{
	char szName [256];
	char szChangeAmount[256];

	two_arguments (argument, szName, sizeof (szName), szChangeAmount, sizeof(szChangeAmount));

	if (*szName == '\0' || *szChangeAmount == '\0')
	{
		ch->ChatPacket (CHAT_TYPE_INFO, "Invalid argument.");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(szName);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::Instance().Find(szName);

		if (pkCCI)
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). %s is not in your game server.", szName, szName);
			return;
		}
		else
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). Perhaps %s doesn't login or exist.", szName, szName);
			return;
		}
	}
	else
	{
		if (tch->IsPolymorphed())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
			return;
		}

		if (subcmd != POINT_SUNGMA_HP && subcmd != POINT_SUNGMA_IMMUNE && subcmd != POINT_SUNGMA_STR && subcmd != POINT_SUNGMA_MOVE)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;763]");
			return;
		}
		int nRemainPoint = tch->GetPoint(POINT_CONQUEROR_POINT);
		int nCurPoint = tch->GetRealPoint(subcmd);
		int nChangeAmount = 0;
		str_to_number(nChangeAmount, szChangeAmount);
		int nPoint = nCurPoint + nChangeAmount;

		int n = -1;
		switch (subcmd)
		{
			case POINT_SUNGMA_HP:
				if (nPoint < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 0;
				break;

			case POINT_SUNGMA_IMMUNE:
				if (nPoint < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 1;
				break;

			case POINT_SUNGMA_STR:
				if (nPoint < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 2;
				break;

			case POINT_SUNGMA_MOVE:
				if (nPoint < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
					return;
				}
				n = 3;
				break;
		}

		if (nPoint > 30)
		{
			nChangeAmount -= nPoint - 30;
			nPoint = 30;
		}

		if (nRemainPoint < nChangeAmount)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;761]");
			return;
		}

		tch->SetRealPoint(subcmd, nPoint);
		tch->SetPoint(subcmd, tch->GetPoint(subcmd) + nChangeAmount);
		tch->ComputePoints();
		tch->PointChange(subcmd, 0);

		tch->PointChange(POINT_CONQUEROR_POINT, -nChangeAmount);
		tch->ComputePoints();

		const char* stat_name[4] = {"ccon", "cimu", "cstr", "cmov"};
		if (-1 == n)
			return;
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s change %d to %d", szName, stat_name[n], nCurPoint, nPoint);
	}
}

ACMD(do_state_sungma)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::Instance().Find(strtoul(arg1 + 1, nullptr, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(arg1);

			if (!d)
				tch = nullptr;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

	if (!tch)
		return;

	char buf[256];

	snprintf(buf, sizeof(buf), "%s's State: ", tch->GetName());

	if (tch->IsPosition(POS_FIGHTING))
		strlcat(buf, "Battle", sizeof(buf));
	else if (tch->IsPosition(POS_DEAD))
		strlcat(buf, "Dead", sizeof(buf));
	else
		strlcat(buf, "Standing", sizeof(buf));

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (tch->GetViewingShop())
		strlcat(buf, ", Shop", sizeof(buf));
#else
	if (tch->GetShop()) //@fixme526
		strlcat(buf, ", Shop", sizeof(buf));
#endif

	if (tch->GetExchange()) //@fixme526
		strlcat(buf, ", Exchange", sizeof(buf));

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	int len;
	len = snprintf(buf, sizeof(buf), "Coordinate %ldx%ld (%ldx%ld)",
		tch->GetX(), tch->GetY(), tch->GetX() / 100, tch->GetY() / 100);

	if (len < 0 || len >= (int)sizeof(buf))
		len = sizeof(buf) - 1;

	LPSECTREE pSec = SECTREE_MANAGER::Instance().Get(tch->GetMapIndex(), tch->GetX(), tch->GetY());

	if (pSec)
	{
		TMapSetting& map_setting = SECTREE_MANAGER::Instance().GetMap(tch->GetMapIndex())->m_setting;
		snprintf(buf + len, sizeof(buf) - len, " MapIndex %ld Attribute %08X Local Position (%ld x %ld)",
			tch->GetMapIndex(), pSec->GetAttribute(tch->GetX(), tch->GetY()), (tch->GetX() - map_setting.iBaseX) / 100, (tch->GetY() - map_setting.iBaseY) / 100);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket(CHAT_TYPE_INFO, "C_LEV %d", tch->GetConquerorLevel());

	ch->ChatPacket(CHAT_TYPE_INFO, "SUNGMA:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   SUNGMA_HP:%d SUNGMA_STR:%d SUNGMA_MOVE:%d SUNGMA_IMMUN:%d",
		tch->GetPoint(POINT_SUNGMA_HP),
		tch->GetPoint(POINT_SUNGMA_STR),
		tch->GetPoint(POINT_SUNGMA_MOVE),
		tch->GetPoint(POINT_SUNGMA_IMMUNE)
	);

	for (int i = 0; i < MAX_PRIV_NUM; ++i)
		if (CPrivManager::Instance().GetPriv(tch, i))
		{
			int iByEmpire = CPrivManager::Instance().GetPrivByEmpire(tch->GetEmpire(), i);
			int iByGuild = 0;

			if (tch->GetGuild())
				iByGuild = CPrivManager::Instance().GetPrivByGuild(tch->GetGuild()->GetID(), i);

			int iByPlayer = CPrivManager::Instance().GetPrivByCharacter(tch->GetPlayerID(), i);

			if (iByEmpire)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for empire : %d", LC_TEXT(c_apszPrivNames[i]), iByEmpire);

			if (iByGuild)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for guild : %d", LC_TEXT(c_apszPrivNames[i]), iByGuild);

			if (iByPlayer)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for player : %d", LC_TEXT(c_apszPrivNames[i]), iByPlayer);
		}
}

#endif

#ifdef ENABLE_EVENT_BANNER_FLAG
#include "minigame_manager.h"
ACMD(do_banner)
{
	char arg1[256], arg2[256];
	int iEnable = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2 || !isnhdigit(*arg1))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: /banner <1:0> <banner_name>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Example: /banner 1 easter");
		return;
	}

	str_to_number(iEnable, arg1);

	int bType = 0;
	if (arg2 == "easter")
		bType = EASTER_EVENT;
	else if (arg2 == "halloween")
		bType = HALLOWEEN_EVENT;
	else if (arg2 == "ramadan")
		bType = RAMADAN_EVENT;
	else if (arg2 == "xmas")
		bType = XMAS_EVENT;

	CMiniGameManager::Instance().InitializeEvent(bType, iEnable);
	return;
}
#endif

#ifdef ENABLE_HWID_BAN
ACMD(do_banpanel)
{
	char arg1[256], arg2[256];
	int account_id = 0;
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage: banpanel <type( unban | ban | hwidban | deletehwidban | totalhwidban )> <name>");
		ch->ChatPacket(CHAT_TYPE_INFO, "");
		ch->ChatPacket(CHAT_TYPE_INFO, "unban: unban player (BLOCK to OK)");
		ch->ChatPacket(CHAT_TYPE_INFO, "ban: ban player (OK to BLOCK)");
		ch->ChatPacket(CHAT_TYPE_INFO, "hwidban: ban player hwid (hwid to hwid_ban)");
		ch->ChatPacket(CHAT_TYPE_INFO, "deletehwidban: delete player banned hwid (delete from hwid_ban)");
		ch->ChatPacket(CHAT_TYPE_INFO, "totalhwidban: ban player hwid and all associated accs (OK to BLOCK with same hwid and hwid to hwid_ban)");
		return;
	}

	if(arg1[0]=='u')
	{
		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), arg2, strlen(arg2));
		auto pMsg(DBManager::Instance().DirectQuery("SELECT player.account_id FROM player WHERE LOWER(name) like LOWER('%s') LIMIT 1", szEscapeName));
		if (pMsg->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s doesnt exist, search for playername", szEscapeName);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		str_to_number(account_id, row[0]);
		DBManager::Instance().DirectQuery("UPDATE account.account set status='OK', availDt='0000-00-00 00:00:00' where id='%d'",account_id);
		ch->ChatPacket(CHAT_TYPE_INFO,"%s unbanned", szEscapeName);
		return;	
	}
	else if(arg1[0]=='b')
	{
		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), arg2, strlen(arg2));
		auto pMsg(DBManager::Instance().DirectQuery("SELECT player.account_id FROM player WHERE LOWER(name) like LOWER('%s') LIMIT 1", szEscapeName));
		if (pMsg->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s doesnt exist, search for playername", szEscapeName);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		str_to_number(account_id, row[0]);
		DBManager::Instance().DirectQuery("UPDATE account.account set status='BLOCK' where id='%d'", account_id);
		ch->ChatPacket(CHAT_TYPE_INFO,"%s banned", szEscapeName);
		LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(szEscapeName);
		LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", szEscapeName);
			return;
		}

		if (tch == ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
			return;
		}

		DESC_MANAGER::Instance().DestroyDesc(d);
		return;	
	}
	else if(arg1[0]=='h')
	{
		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), arg2, strlen(arg2));	
		auto pMsg(DBManager::Instance().DirectQuery("SELECT player.account_id FROM player WHERE LOWER(name) like LOWER('%s') LIMIT 1", szEscapeName));
		if (pMsg->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s doesnt exist, search for playername", szEscapeName);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		str_to_number(account_id, row[0]);
		auto pMsg2(DBManager::Instance().DirectQuery("SELECT hwid FROM account.account WHERE id = '%d' LIMIT 1", account_id));
		if (pMsg2->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "cant get hwid from %s", szEscapeName);
			return;
		}

		MYSQL_ROW row2 = mysql_fetch_row(pMsg2->Get()->pSQLResult);
		if (strcmp(row2[0], "") == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s has no hwid", szEscapeName);
			return;
		}

		DBManager::Instance().DirectQuery("INSERT INTO account.hwid_ban (hwid, player, gm) VALUES ('%s', '%s', '%s')", row2[0], szEscapeName, ch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO,"%s HWID banned (%s)", szEscapeName, row2[0]);
		LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(szEscapeName);
		LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", szEscapeName);
			return;
		}

		if (tch == ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
			return;
		}

		DESC_MANAGER::Instance().DestroyDesc(d);
		return;	
	}
	else if(arg1[0]=='d')
	{
		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), arg2, strlen(arg2));
		auto pMsg(DBManager::Instance().DirectQuery("SELECT player.account_id FROM player WHERE LOWER(name) like LOWER('%s') LIMIT 1", szEscapeName));
		if (pMsg->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s doesnt exist, search for playername", szEscapeName);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		str_to_number(account_id, row[0]);
		auto pMsg2(DBManager::Instance().DirectQuery("SELECT hwid FROM account.account WHERE id = '%d' LIMIT 1", account_id));
		if (pMsg2->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "cant get hwid from %s", szEscapeName);
			return;
		}

		MYSQL_ROW row2 = mysql_fetch_row(pMsg2->Get()->pSQLResult);
		if (strcmp(row2[0], "") == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s has no hwid", szEscapeName);
			return;
		}

		DBManager::Instance().DirectQuery("DELETE FROM account.hwid_ban WHERE hwid = '%s'", row2[0]);
		ch->ChatPacket(CHAT_TYPE_INFO,"%s HWID unbanned (%s)", szEscapeName, row2[0]);
		return;	
	}
	else if(arg1[0]=='t')
	{
		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), arg2, strlen(arg2));	
		auto pMsg(DBManager::Instance().DirectQuery("SELECT player.account_id FROM player WHERE LOWER(name) like LOWER('%s') LIMIT 1", szEscapeName));
		if (pMsg->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s doesnt exist, search for playername", szEscapeName);
			return;
		}

		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		str_to_number(account_id, row[0]);
		auto pMsg2(DBManager::Instance().DirectQuery("SELECT hwid FROM account.account WHERE id = '%d' LIMIT 1", account_id));
		if (pMsg2->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "cant get hwid from %s", szEscapeName);
			return;
		}

		MYSQL_ROW row2 = mysql_fetch_row(pMsg2->Get()->pSQLResult);
		if (strcmp(row2[0], "") == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s has no hwid", szEscapeName);
			return;
		}

		DBManager::Instance().DirectQuery("UPDATE account.account set status = 'BLOCK' WHERE hwid = '%s'", row2[0]);
		DBManager::Instance().DirectQuery("INSERT INTO account.hwid_ban (hwid, player, gm) VALUES ('%s', '%s', '%s')", row2[0], szEscapeName, ch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO,"%s HWID and all associated accounts banned (%s)", szEscapeName, row2[0]);
		LPDESC d = DESC_MANAGER::Instance().FindByCharacterName(szEscapeName);
		LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", szEscapeName);
			return;
		}

		if (tch == ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
			return;
		}

		DESC_MANAGER::Instance().DestroyDesc(d);
		return;	
	}
}
#endif

#ifdef ENABLE_TICKET_SYSTEM
ACMD(do_delete_tickets_closed)
{
	if (!ch)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Wrong syntax> Use /delete_tickets_closed true");
		return;
	}

	if (!strcmp(arg1, "true"))
	{
		auto dwExtract(DBManager::Instance().DirectQuery("SELECT COUNT(*) FROM ticket.list WHERE status = 2"));
		MYSQL_ROW row = mysql_fetch_row(dwExtract->Get()->pSQLResult);

		int	iRowsDeleted = 0;
		str_to_number(iRowsDeleted, row[0]);

		if (iRowsDeleted == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<Ticket-Query> Not exist query's (row's) with status closed.");
			return;
		}

		char szQuery[QUERY_MAX_LEN + 1];
		snprintf(szQuery, sizeof(szQuery), "DELETE FROM ticket.list WHERE status = 2;");
		auto pMsg(DBManager::Instance().DirectQuery(szQuery));

		ch->ChatPacket(CHAT_TYPE_INFO, "<Ticket-Query> Delete with successfully %d query's (row's) with status closed.", iRowsDeleted);
	}
}
#endif

#ifdef ENABLE_EASTER_EVENT
ACMD(do_easter_event)
{
	char arg1[256];
	int iCommand = 1;
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: easter_event <command>");
		ch->ChatPacket(CHAT_TYPE_INFO, "	0 = Deaktif.");
		ch->ChatPacket(CHAT_TYPE_INFO, "	1 = Aktif.");
		return;
	}

	if (isnhdigit(*arg1))
	{
		str_to_number(iCommand, arg1);
	}

	if (iCommand == 1)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("enable_easter_event") == 0)
		{
			quest::CQuestManager::Instance().RequestSetEventFlag("enable_easter_event", 1);
			//SendNotice("Paskalya eventi aktif.");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Zaten acik.");
		}
	}
	else
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("enable_easter_event", 0);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Kapandi.");
	}
}
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
ACMD(do_catch_king_event)
{
	char arg1[256], arg2[256];
	int iCommand = 1;
	int iDays = 1;
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: catch_king_event <command> <days_number>");
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
		if (quest::CQuestManager::Instance().GetEventFlag("enable_catch_king_event") == 0)
		{
			int iEndTime = time(0) + 60 * 60 * 24 * iDays;

			quest::CQuestManager::Instance().RequestSetEventFlag("enable_catch_king_event", 1);
			quest::CQuestManager::Instance().RequestSetEventFlag("enable_catch_king_event_drop", 1);
			quest::CQuestManager::Instance().RequestSetEventFlag("catch_king_event_end_day", iEndTime);

			SendNotice("Catch king event is now active. Relog and check the icon beside minimap.");
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "This event is already opened.");
		}
	}
	else
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("enable_catch_king_event", 0);
		quest::CQuestManager::Instance().RequestSetEventFlag("enable_catch_king_event_drop", 0);
		ch->ChatPacket(CHAT_TYPE_INFO, "You deactivated catch king event.");
	}
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
ACMD(do_growth_pet)
{
	if (!ch)
		return;

	char arg1[256] = {};
	char arg2[256] = {};
	int value = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: /set_pet type value");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available types:");
		ch->ChatPacket(CHAT_TYPE_INFO, " level");
		ch->ChatPacket(CHAT_TYPE_INFO, " age");
		ch->ChatPacket(CHAT_TYPE_INFO, " exp_monster");
		ch->ChatPacket(CHAT_TYPE_INFO, " exp_item");
		return;
	}

	CGrowthPetSystem* petSystem = ch->GetGrowthPetSystem();
	if (!petSystem)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Cannot get pet system.");
		return;
	}

	if (!petSystem->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1343]");
		return;
	}

	std::string strArg(arg1);
	str_to_number(value, arg2);

	if (!strArg.compare(0, 5, "level"))
		petSystem->SetLevel(value);
	else if (!strArg.compare(0, 8, "age"))
		petSystem->SetBirthday(value);
	else if (!strArg.compare(0, 11, "exp_monster"))
		petSystem->SetExp(value, PET_EXP_FROM_MOB);
	else if (!strArg.compare(0, 8, "exp_item"))
		petSystem->SetExp(value, PET_EXP_FROM_ITEM);
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: /set_pet type value");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available types:");
		ch->ChatPacket(CHAT_TYPE_INFO, " level");
		ch->ChatPacket(CHAT_TYPE_INFO, " age");
		ch->ChatPacket(CHAT_TYPE_INFO, " exp_monster");
		ch->ChatPacket(CHAT_TYPE_INFO, " exp_item");
		return;
	}
}
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
ACMD(do_force_finish_achievement)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	uint32_t achievement_id = 0;

	if (*arg1 == '\0' || *arg2 == '\0') {
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: /force_finish_achievement <player> <achievement id>");
		return;
	}

	str_to_number(achievement_id, arg2);

	LPCHARACTER target = CHARACTER_MANAGER::Instance().FindPC(arg1);

	if (!target) {
		ch->ChatPacket(CHAT_TYPE_INFO, "Player not connected.");
		return;
	}

	CAchievementSystem::Instance().FinishAchievement(target, achievement_id);
	ch->ChatPacket(CHAT_TYPE_INFO, "Achievement %d for player %s is now finished.", achievement_id, arg1);
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
#include "battle_pass.h"
ACMD(do_battlepass_get_info)
{
	if (CBattlePassManager::instance().GetNormalBattlePassID() == 0)
		ch->ChatPacket(CHAT_TYPE_INFO, "No normal Battlepass is currently active");
	else
	{
		std::unique_ptr<SQLMsg> pMsgRegistred(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM `battlepass_playerindex` WHERE battlepass_type = 1 and battlepass_id = %d", CBattlePassManager::instance().GetNormalBattlePassID()));
		std::unique_ptr<SQLMsg> pMsgCompledet(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM `battlepass_playerindex` WHERE battlepass_type = 1 and battlepass_id = %d and battlepass_completed = 1", CBattlePassManager::instance().GetNormalBattlePassID()));
		if (!pMsgRegistred->uiSQLErrno and !pMsgCompledet->uiSQLErrno)
		{
			MYSQL_ROW row_registred = mysql_fetch_row(pMsgRegistred->Get()->pSQLResult);
			MYSQL_ROW row_compledet = mysql_fetch_row(pMsgCompledet->Get()->pSQLResult);

			ch->ChatPacket(CHAT_TYPE_INFO, "---------------------------------------------------------------");
			ch->ChatPacket(CHAT_TYPE_INFO, "Actual Normal Battlepass ID = %d", CBattlePassManager::instance().GetNormalBattlePassID());
			ch->ChatPacket(CHAT_TYPE_INFO, "Registred Player for Normal Battlepass = %d", std::atoi(row_registred[0]));
			ch->ChatPacket(CHAT_TYPE_INFO, "Finish Player for Normal Battlepass = %d / %d", std::atoi(row_compledet[0]), std::atoi(row_registred[0]));
			ch->ChatPacket(CHAT_TYPE_INFO, "---------------------------------------------------------------");
		}
	}

	if (CBattlePassManager::instance().GetPremiumBattlePassID() == 0)
		ch->ChatPacket(CHAT_TYPE_INFO, "No premium Battlepass is currently active");
	else
	{
		std::unique_ptr<SQLMsg> pMsgRegistred(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM `battlepass_playerindex` WHERE battlepass_type = 2 and battlepass_id = %d", CBattlePassManager::instance().GetPremiumBattlePassID()));
		std::unique_ptr<SQLMsg> pMsgCompledet(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM `battlepass_playerindex` WHERE battlepass_type = 2 and battlepass_id = %d and battlepass_completed = 1", CBattlePassManager::instance().GetPremiumBattlePassID()));
		if (!pMsgRegistred->uiSQLErrno and !pMsgCompledet->uiSQLErrno)
		{
			MYSQL_ROW row_registred = mysql_fetch_row(pMsgRegistred->Get()->pSQLResult);
			MYSQL_ROW row_compledet = mysql_fetch_row(pMsgCompledet->Get()->pSQLResult);

			ch->ChatPacket(CHAT_TYPE_INFO, "---------------------------------------------------------------");
			ch->ChatPacket(CHAT_TYPE_INFO, "Actual Premium Battlepass ID = %d", CBattlePassManager::instance().GetPremiumBattlePassID());
			ch->ChatPacket(CHAT_TYPE_INFO, "Registred Player for Premium Battlepass = %d", std::atoi(row_registred[0]));
			ch->ChatPacket(CHAT_TYPE_INFO, "Finish Player for Premium Battlepass = %d / %d", std::atoi(row_compledet[0]), std::atoi(row_registred[0]));
			ch->ChatPacket(CHAT_TYPE_INFO, "---------------------------------------------------------------");
		}
	}

	if (CBattlePassManager::instance().GetEventBattlePassID() == 0)
		ch->ChatPacket(CHAT_TYPE_INFO, "No event Battlepass is currently active");
	else
	{
		std::unique_ptr<SQLMsg> pMsgRegistred(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM `battlepass_playerindex` WHERE battlepass_type = 3 and battlepass_id = %d", CBattlePassManager::instance().GetEventBattlePassID()));
		std::unique_ptr<SQLMsg> pMsgCompledet(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM `battlepass_playerindex` WHERE battlepass_type = 3 and battlepass_id = %d and battlepass_completed = 1", CBattlePassManager::instance().GetEventBattlePassID()));
		if (!pMsgRegistred->uiSQLErrno and !pMsgCompledet->uiSQLErrno)
		{
			MYSQL_ROW row_registred = mysql_fetch_row(pMsgRegistred->Get()->pSQLResult);
			MYSQL_ROW row_compledet = mysql_fetch_row(pMsgCompledet->Get()->pSQLResult);

			ch->ChatPacket(CHAT_TYPE_INFO, "---------------------------------------------------------------");
			ch->ChatPacket(CHAT_TYPE_INFO, "Actual Event Battlepass ID = %d", CBattlePassManager::instance().GetEventBattlePassID());
			ch->ChatPacket(CHAT_TYPE_INFO, "Registred Player for Event Battlepass = %d", std::atoi(row_registred[0]));
			ch->ChatPacket(CHAT_TYPE_INFO, "Finish Player for Event Battlepass = %d / %d", std::atoi(row_compledet[0]), std::atoi(row_registred[0]));
			ch->ChatPacket(CHAT_TYPE_INFO, "---------------------------------------------------------------");
		}
	}
}

ACMD(do_battlepass_set_mission)
{
	char arg1[256], arg2[256], arg3[256], arg4[256];
	four_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1 || !*arg2 || !*arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: battlepass_set_mission <battlepass_type> <mission_index> <value> (<playername>)");
		ch->ChatPacket(CHAT_TYPE_INFO, "battlepass_type: 1 = NORMAL | 2 = PREMIUM | 3 = EVENT");
		ch->ChatPacket(CHAT_TYPE_INFO, "mission_index: mission index means the number of the mission counted from the top starting with 1");
		ch->ChatPacket(CHAT_TYPE_INFO, "value: input the value what you will override");
		return;
	}

	int battlepass_type, mission_index, value;
	str_to_number(battlepass_type, arg1);
	str_to_number(mission_index, arg2);
	str_to_number(value, arg3);

	value = MAX(0, value);

	if (battlepass_type == 1 and CBattlePassManager::instance().GetNormalBattlePassID() == 0) {
		ch->ChatPacket(CHAT_TYPE_INFO, "No normal Battlepass is currently active");
		return;
	}
	if (battlepass_type == 2 and CBattlePassManager::instance().GetPremiumBattlePassID() == 0) {
		ch->ChatPacket(CHAT_TYPE_INFO, "No premium Battlepass is currently active");
		return;
	}
	if (battlepass_type == 3 and CBattlePassManager::instance().GetEventBattlePassID() == 0) {
		ch->ChatPacket(CHAT_TYPE_INFO, "No event Battlepass is currently active");
		return;
	}

	LPCHARACTER tch;

	if (*arg4 && ch->GetName() != arg4)
		tch = CHARACTER_MANAGER::instance().FindPC(arg4);
	else
		tch = CHARACTER_MANAGER::instance().FindPC(ch->GetName());

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "This player is not online or does not exist.");
		return;
	}
	if (battlepass_type == 2 and CBattlePassManager::instance().GetPremiumBattlePassID() != tch->GetExtBattlePassPremiumID()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "This player does not have access to the current Premium Battle Pass.");
		return;
	}
	uint32_t mission_type = CBattlePassManager::instance().GetMissionTypeByIndex(battlepass_type, mission_index);
	if (mission_type == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no mission index %d in battlepass-typ %d", mission_index, battlepass_type);
		return;
	}

	tch->SetExtBattlePassMissionProgress(battlepass_type, mission_index, mission_type, value);
}

ACMD(do_battlepass_premium_activate)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int value;
	str_to_number(value, arg2);

	if (!*arg1 || !*arg2 || value > 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: battlepass_premium_activate <playername> <activate = 1 / deactivate = 0>");
		return;
	}

	if (CBattlePassManager::instance().GetPremiumBattlePassID() == 0) {
		ch->ChatPacket(CHAT_TYPE_INFO, "No premium Battlepass is currently active");
		return;
	}

	if (ch->GetName() != arg1) {
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "This player is not online or does not exist.");
			return;
		}

		if (value == 1)
		{
			tch->PointChange(POINT_BATTLE_PASS_PREMIUM_ID, CBattlePassManager::instance().GetPremiumBattlePassID());
			CBattlePassManager::instance().BattlePassRequestOpen(tch, false);
			tch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_NOW_IS_ACTIVATED_PREMIUM_BATTLEPASS");
			ch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_CMDGM_ACTIVATE_PREMIUM_TO_PLAYER", tch->GetName());
		}
		if (value == 0)
		{
			tch->PointChange(POINT_BATTLE_PASS_PREMIUM_ID, 0);
			tch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_CMDGM_DEACTIVATE_PREMIUM_PLAYER");
			ch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_CMDGM_DEACTIVATE_PREMIUM_TO_PLAYER", tch->GetName());
		}
	}
	else
	{
		if (value == 1)
		{
			ch->PointChange(POINT_BATTLE_PASS_PREMIUM_ID, CBattlePassManager::instance().GetPremiumBattlePassID());
			CBattlePassManager::instance().BattlePassRequestOpen(ch, false);
			ch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_NOW_IS_ACTIVATED_PREMIUM_BATTLEPASS_OWN");
		}
		if (value == 0)
		{
			ch->PointChange(POINT_BATTLE_PASS_PREMIUM_ID, 0);
			CBattlePassManager::instance().BattlePassRequestOpen(ch, false);
			ch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_CMDGM_DEACTIVATE_PREMIUM_OWN");
		}
	}
}
#endif
