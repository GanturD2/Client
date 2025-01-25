#include "stdafx.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "party.h"
#include "messenger_manager.h"
#include "empire_text_convert.h"
#include "unique_item.h"
#include "xmas_event.h"
#include "affect.h"
#include "castle.h"
#include "dev_log.h"
#include "locale_service.h"
#include "questmanager.h"
#include "skill.h"
#include "threeway_war.h"

#include "../../common/CommonDefines.h"
#ifdef ENABLE_SWITCHBOT
#	include "switchbot.h"
#endif
#ifdef ENABLE_RANKING_SYSTEM
#	include "ranking_system.h"
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Input Processor
CInputP2P::CInputP2P()
{
	BindPacketInfo(&m_packetInfoGG);
}

void CInputP2P::Login(LPDESC d, const char* c_pData)
{
	P2P_MANAGER::Instance().Login(d, (TPacketGGLogin*)c_pData);
}

void CInputP2P::Logout(LPDESC d, const char* c_pData)
{
	const TPacketGGLogout* p = (TPacketGGLogout*)c_pData;
	P2P_MANAGER::Instance().Logout(p->szName);
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CInputP2P::StartShopOffline(LPDESC d, const char* c_pData)
{
	P2P_MANAGER::Instance().StartShopOffline(d, (TPacketGGShopStartOffline*)c_pData);
}

void CInputP2P::EndShopOffline(LPDESC d, const char* c_pData)
{
	const TPacketGGShopEndOffline* p = (TPacketGGShopEndOffline*)c_pData;
	P2P_MANAGER::Instance().EndShopOffline(p->dwPID);
}
#endif

#ifdef ENABLE_RANKING_SYSTEM
void CInputP2P::LoadRanking(const char* c_pData)
{
	TPacketGGLoadRanking * p = (TPacketGGLoadRanking*)c_pData;
	CRankingSystem::Instance().LoadRanking(p->bCategory);
}
#endif

#ifdef ENABLE_EVENT_MANAGER
void CInputP2P::Event(const char* c_pData)
{
	TPacketGGEvent* p = (TPacketGGEvent*)c_pData;
	CEventManager::Instance().SetEventState(&p->table, p->bState);
}

void CInputP2P::HideAndSeekEvent(const char* c_pData)
{
	TPacketGGEventHideAndSeek* p = (TPacketGGEventHideAndSeek*)c_pData;

	// Hide and Seek event is only active on one specific channel
	if (g_bChannel != CEventManager::HIDE_AND_SEEK_CHANNEL)
		return;

	CEventManager::Instance().HideAndSeekNPC(p->iPosition, p->iRound);
}

# ifdef ENABLE_BATTLE_PASS_SYSTEM
void CInputP2P::BattlePassEvent(const char* c_pData)
{
	TPacketGGEventBattlePass* p = (TPacketGGEventBattlePass*)c_pData;
	CEventManager::Instance().BattlePassData(&p->table, p->bType, p->bState);
}
# endif
#endif

int CInputP2P::Relay(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (!d)
		return -1;

	TPacketGGRelay* p = (TPacketGGRelay*)c_pData;

	if (uiBytes < sizeof(TPacketGGRelay) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	sys_log(0, "InputP2P::Relay : %s size %d", p->szName, p->lSize);

	const LPCHARACTER& pkChr = CHARACTER_MANAGER::Instance().FindPC(p->szName);

	const uint8_t* c_pbData = (const uint8_t*)(c_pData + sizeof(TPacketGGRelay));

	if (!pkChr)
		return p->lSize;

	if (*c_pbData == HEADER_GC_WHISPER)
	{
		if (pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			// Reject the whisper in the whisper rejection state.
			return p->lSize;
		}

		char buf[1024];
		memcpy(buf, c_pbData, MIN(p->lSize, sizeof(buf)));

		TPacketGCWhisper* p2 = (TPacketGCWhisper*)buf;
		// bType upper 4 bits: Empire number
		// lower 4 bits of bType: EWhisperType
		const uint8_t bToEmpire = (p2->bType >> 4);
		p2->bType = p2->bType & 0x0F;
		if (p2->bType == 0x0F)
		{
			// System message Whisper uses all the upper bits of bType.
			p2->bType = WHISPER_TYPE_SYSTEM;
		}
		else
		{
			if (!pkChr->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
			{
				if (bToEmpire >= 1 && bToEmpire <= 3 && pkChr->GetEmpire() != bToEmpire)
				{
					ConvertEmpireText(bToEmpire,
						buf + sizeof(TPacketGCWhisper),
						p2->wSize - sizeof(TPacketGCWhisper),
						10 + 2 * pkChr->GetSkillPower(SKILL_LANGUAGE1 + bToEmpire - 1));
				}
			}
		}

		pkChr->GetDesc()->Packet(buf, p->lSize);
	}
	else
		pkChr->GetDesc()->Packet(c_pbData, p->lSize);

	return (p->lSize);
}

#ifdef ENABLE_BATTLE_FIELD
int CInputP2P::Command(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (!d)
		return -1;

	const TPacketGGCommand* p = (TPacketGGCommand*)c_pData;

	if (uiBytes < sizeof(TPacketGGCommand) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGNotice), MIN(p->lSize + 1, sizeof(szBuf)));
	SendCommand(szBuf);
	return (p->lSize);
}
#endif

int CInputP2P::Notice(LPDESC d, const char* c_pData, size_t uiBytes, bool bBigFont)
{
	if (!d)
		return -1;

	const TPacketGGNotice* p = (TPacketGGNotice*)c_pData;

	if (uiBytes < sizeof(TPacketGGNotice) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGNotice), MIN(p->lSize + 1, sizeof(szBuf)));
	SendNotice(szBuf, bBigFont);
	return (p->lSize);
}

int CInputP2P::MonarchNotice(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (!d)
		return -1;

	const TPacketGGMonarchNotice* p = (TPacketGGMonarchNotice*)c_pData;

	if (uiBytes < p->lSize + sizeof(TPacketGGMonarchNotice))
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGMonarchNotice), MIN(p->lSize + 1, sizeof(szBuf)));
	SendMonarchNotice(p->bEmpire, szBuf);
	return (p->lSize);
}

int CInputP2P::MonarchTransfer(LPDESC d, const char* c_pData)
{
	const TPacketMonarchGGTransfer* p = (TPacketMonarchGGTransfer*)c_pData;
	LPCHARACTER pTargetChar = CHARACTER_MANAGER::Instance().FindByPID(p->dwTargetPID);

	if (pTargetChar != nullptr)
	{
		uint32_t qIndex = quest::CQuestManager::Instance().GetQuestIndexByName("monarch_transfer");

		if (qIndex != 0)
		{
			pTargetChar->SetQuestFlag("monarch_transfer.x", p->x);
			pTargetChar->SetQuestFlag("monarch_transfer.y", p->y);
			quest::CQuestManager::Instance().Letter(pTargetChar->GetPlayerID(), qIndex, 0);
		}
	}

	return 0;
}

int CInputP2P::Guild(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGGuild* p = (TPacketGGGuild*)c_pData;
	uiBytes -= sizeof(TPacketGGGuild);
	c_pData += sizeof(TPacketGGGuild);

	CGuild* g = CGuildManager::Instance().FindGuild(p->dwGuild);

	switch (p->bSubHeader)
	{
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
		case GUILD_SUBHEADER_GG_REFRESH:
			{
				if (uiBytes < sizeof(TPacketGGGuildChat))
					return -1;

				if (g)
					g->RefreshP2P(g);

				return sizeof(TPacketGGGuildChat);
			}

		case GUILD_SUBHEADER_GG_REFRESH1:
			{
				if (uiBytes < sizeof(TPacketGGGuildChat))
					return -1;

				TPacketGGGuildChat * p2 = (TPacketGGGuildChat *) c_pData;

				if (g)
					g->SetLastCheckoutP2P(p2->szText, p2->dwGuild);

				return sizeof(TPacketGGGuildChat);
			}
#endif

		case GUILD_SUBHEADER_GG_CHAT:
			{
				if (uiBytes < sizeof(TPacketGGGuildChat))
					return -1;

				const TPacketGGGuildChat* p2 = (TPacketGGGuildChat*)c_pData;

				if (g)
					g->P2PChat(p2->szText);

				return sizeof(TPacketGGGuildChat);
			}

		case GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS:
			{
				if (uiBytes < sizeof(int))
					return -1;

				const int iBonus = *((int*)c_pData);
				CGuild* pGuild = CGuildManager::Instance().FindGuild(p->dwGuild);
				if (pGuild)
					pGuild->SetMemberCountBonus(iBonus);

				return sizeof(int);
			}

		default:
			sys_err("UNKNOWN GUILD SUB PACKET");
			break;
	}
	return 0;
}

struct FuncShout
{
	const char* m_str;
	uint8_t m_bEmpire;
#ifdef ENABLE_MESSENGER_BLOCK
	const char* m_szName;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t m_Language;
#endif
#ifdef ENABLE_EMPIRE_FLAG
	bool m_Flag;
#endif

	FuncShout(const char* str, uint8_t bEmpire
#ifdef ENABLE_MESSENGER_BLOCK
		, const char* c_szName
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		, uint8_t m_Language
#endif
#ifdef ENABLE_EMPIRE_FLAG
		, bool bFlag
#endif
		) : m_str(str), m_bEmpire(bEmpire)
#ifdef ENABLE_MESSENGER_BLOCK
		, m_szName(c_szName)
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		, m_Language(m_Language)
#endif
#ifdef ENABLE_EMPIRE_FLAG
		, m_Flag(bFlag)
#endif
		{}

	void operator () (LPDESC d)
	{
		if (!d)
			return;

#ifdef ENABLE_NEWSTUFF
		if (!d->GetCharacter() || (!g_bGlobalShoutEnable && d->GetCharacter()->GetGMLevel() == GM_PLAYER && d->GetEmpire() != m_bEmpire))
			return;
#else
		if (!d->GetCharacter() || (d->GetCharacter()->GetGMLevel() == GM_PLAYER && d->GetEmpire() != m_bEmpire))
			return;
#endif

#ifdef ENABLE_MESSENGER_BLOCK
		if (MessengerManager::Instance().IsBlocked(d->GetCharacter()->GetName(), m_szName))
			return;
#endif

		d->GetCharacter()->ChatPacket(CHAT_TYPE_SHOUT, "%s", m_str);
	}
};

void SendShout(const char* c_szText, uint8_t bEmpire
#ifdef ENABLE_MESSENGER_BLOCK
, const char* c_szName
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
, uint8_t bLanguage
#endif
#ifdef ENABLE_EMPIRE_FLAG
, bool bFlag
#endif
)
{
	const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(),
		FuncShout(c_szText, bEmpire
#ifdef ENABLE_MESSENGER_BLOCK
		, c_szName
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		, bLanguage
#endif
#ifdef ENABLE_EMPIRE_FLAG
		, bFlag
#endif
	));
}

void CInputP2P::Shout(const char* c_pData)
{
	const TPacketGGShout* p = (TPacketGGShout*)c_pData;
	SendShout(p->szText, p->bEmpire
#ifdef ENABLE_MESSENGER_BLOCK
		, p->szName
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		, p->bLanguage
#endif
#ifdef ENABLE_EMPIRE_FLAG
		, p->bFlag
#endif
	);
}

void CInputP2P::Disconnect(const char * c_pData)
{
	TPacketGGDisconnect * p = (TPacketGGDisconnect *) c_pData;

	LPDESC d = DESC_MANAGER::Instance().FindByLoginName(p->szLogin);

	if (!d)
		return;

	if (!d->GetCharacter())
	{
		d->SetPhase(PHASE_CLOSE);
	}
	else
		d->DisconnectOfSameLogin();
}

void CInputP2P::Setup(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	const TPacketGGSetup* p = (TPacketGGSetup*)c_pData;
	sys_log(0, "P2P: Setup %s:%d", d->GetHostName(), p->wPort);
	d->SetP2P(d->GetHostName(), p->wPort, p->bChannel);
}

void CInputP2P::MessengerAdd(const char* c_pData)
{
	TPacketGGMessenger* p = (TPacketGGMessenger*)c_pData;
	sys_log(0, "P2P: Messenger Add %s %s", p->szAccount, p->szCompanion);
	MessengerManager::Instance().__AddToList(p->szAccount, p->szCompanion);
}

void CInputP2P::MessengerRemove(const char* c_pData)
{
	TPacketGGMessenger* p = (TPacketGGMessenger*)c_pData;
	sys_log(0, "P2P: Messenger Remove %s %s", p->szAccount, p->szCompanion);
	MessengerManager::Instance().__RemoveFromList(p->szAccount, p->szCompanion);
}

#ifdef ENABLE_MESSENGER_BLOCK
void CInputP2P::MessengerBlockAdd(const char* c_pData)
{
	TPacketGGMessenger* p = (TPacketGGMessenger*)c_pData;
	sys_log(0, "P2P: Messenger Block Add %s %s", p->szAccount, p->szCompanion);
	MessengerManager::Instance().__AddToBlockList(p->szAccount, p->szCompanion);
}

void CInputP2P::MessengerBlockRemove(const char* c_pData)
{
	TPacketGGMessenger* p = (TPacketGGMessenger*)c_pData;
	sys_log(0, "P2P: Messenger Block Remove %s %s", p->szAccount, p->szCompanion);
	MessengerManager::Instance().__RemoveFromBlockList(p->szAccount, p->szCompanion);
}
#endif

void CInputP2P::FindPosition(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	TPacketGGFindPosition* p = (TPacketGGFindPosition*)c_pData;
	const LPCHARACTER& ch = CHARACTER_MANAGER::Instance().FindByPID(p->dwTargetPID);
#ifdef ENABLE_CMD_WARP_IN_DUNGEON
	if (ch)
#else
	if (ch && ch->GetMapIndex() < 10000)
#endif
	{
		TPacketGGWarpCharacter pw{};
		pw.header = HEADER_GG_WARP_CHARACTER;
		pw.pid = p->dwFromPID;
		pw.x = ch->GetX();
		pw.y = ch->GetY();
#ifdef ENABLE_CMD_WARP_IN_DUNGEON
		pw.mapIndex = (ch->GetMapIndex() < 10000) ? 0 : ch->GetMapIndex();
#endif
		d->Packet(&pw, sizeof(pw));
	}
}

void CInputP2P::WarpCharacter(const char* c_pData)
{
	const TPacketGGWarpCharacter* p = (TPacketGGWarpCharacter*)c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(p->pid);
#ifdef ENABLE_CMD_WARP_IN_DUNGEON
	if (ch)
		ch->WarpSet(p->x, p->y, p->mapIndex);
#else
	if (ch)
		ch->WarpSet(p->x, p->y);
#endif
}

void CInputP2P::GuildWarZoneMapIndex(const char* c_pData)
{
	const TPacketGGGuildWarMapIndex* p = (TPacketGGGuildWarMapIndex*)c_pData;
	CGuildManager& gm = CGuildManager::Instance();

	sys_log(0, "P2P: GuildWarZoneMapIndex g1(%u) vs g2(%u), mapIndex(%d)", p->dwGuildID1, p->dwGuildID2, p->lMapIndex);

	CGuild* g1 = gm.FindGuild(p->dwGuildID1);
	CGuild* g2 = gm.FindGuild(p->dwGuildID2);

	if (g1 && g2)
	{
		g1->SetGuildWarMapIndex(p->dwGuildID2, p->lMapIndex);
		g2->SetGuildWarMapIndex(p->dwGuildID1, p->lMapIndex);
	}
}

void CInputP2P::Transfer(const char* c_pData)
{
	const TPacketGGTransfer * p = (TPacketGGTransfer*)c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(p->szName);

	if (ch)
		ch->WarpSet(p->lX, p->lY);
}

int CInputP2P::BigNotice(LPDESC d, const char * c_pData, size_t uiBytes)
{
	TPacketGGBigNotice * p = (TPacketGGBigNotice *)c_pData;

	if (uiBytes < sizeof(TPacketGGBigNotice) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256 + 1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGBigNotice), MIN(p->lSize + 1, sizeof(szBuf)));
	SendBigNotice(szBuf, -1); //If it went through P2P, it's a notice for everyone

	return (p->lSize);
}

void CInputP2P::XmasWarpSanta(const char* c_pData)
{
	const TPacketGGXmasWarpSanta* p = (TPacketGGXmasWarpSanta*)c_pData;

	if (p->bChannel == g_bChannel && map_allow_find(p->lMapIndex))
	{
		constexpr int iNextSpawnDelay = 50 * 60;

		xmas::SpawnSanta(p->lMapIndex, iNextSpawnDelay); // After 50 minutes, a new Santa appears (20 minutes in Korea)

		TPacketGGXmasWarpSantaReply pack_reply{};
		pack_reply.bHeader = HEADER_GG_XMAS_WARP_SANTA_REPLY;
		pack_reply.bChannel = g_bChannel;
		P2P_MANAGER::Instance().Send(&pack_reply, sizeof(pack_reply));
	}
}

void CInputP2P::XmasWarpSantaReply(const char* c_pData)
{
	const TPacketGGXmasWarpSantaReply* p = (TPacketGGXmasWarpSantaReply*)c_pData;

	if (p->bChannel == g_bChannel)
	{
		auto snapshot = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(xmas::MOB_SANTA_VNUM);
		if (!snapshot.empty())
		{
			auto it = snapshot.begin();
			while (it != snapshot.end())
			{
				M2_DESTROY_CHARACTER(*it++);
			}
		}
	}
}

void CInputP2P::LoginPing(LPDESC d, const char* c_pData)
{
	const TPacketGGLoginPing* p = (TPacketGGLoginPing*)c_pData;

	if (!g_pkAuthMasterDesc) // If I am master, I have to broadcast
		P2P_MANAGER::Instance().Send(p, sizeof(TPacketGGLoginPing), d);
}

// BLOCK_CHAT
void CInputP2P::BlockChat(const char* c_pData)
{
	TPacketGGBlockChat* p = (TPacketGGBlockChat*)c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindPC(p->szName);

	if (ch)
	{
		sys_log(0, "BLOCK CHAT apply name %s dur %d", p->szName, p->lBlockDuration);
		ch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, p->lBlockDuration, 0, true);
	}
	else
	{
		sys_log(0, "BLOCK CHAT fail name %s dur %d", p->szName, p->lBlockDuration);
	}
}
// END_OF_BLOCK_CHAT
//

void CInputP2P::IamAwake(LPDESC d, const char* c_pData)
{
	if (!d)
		return;

	std::string hostNames;
	P2P_MANAGER::Instance().GetP2PHostNames(hostNames);
	sys_log(0, "P2P Awakeness check from %s. My P2P connection number is %d. and details...\n%s", d->GetHostName(), P2P_MANAGER::Instance().GetDescCount(), hostNames.c_str());
}

int CInputP2P::Analyze(LPDESC d, uint8_t bHeader, const char* c_pData)
{
	if (test_server)
		sys_log(0, "CInputP2P::Anlayze[Header %d]", bHeader);

	int iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_GG_SETUP:
			Setup(d, c_pData);
			break;

		case HEADER_GG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_GG_LOGOUT:
			Logout(d, c_pData);
			break;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case HEADER_GG_SHOP_OFFLINE_START:
			StartShopOffline(d, c_pData);
			break;

		case HEADER_GG_SHOP_OFFLINE_END:
			EndShopOffline(d, c_pData);
			break;
#endif

		case HEADER_GG_RELAY:
			if ((iExtraLen = Relay(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#ifdef ENABLE_FULL_NOTICE
		case HEADER_GG_BIG_NOTICE:
			if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft, true)) < 0)
				return -1;
			break;
#endif
		case HEADER_GG_NOTICE:
			if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_SHUTDOWN:
			sys_err("Accept shutdown p2p command from %s.", d->GetHostName());
			Shutdown(10);
			break;

		case HEADER_GG_GUILD:
			if ((iExtraLen = Guild(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_SHOUT:
			Shout(c_pData);
			break;

		case HEADER_GG_DISCONNECT:
			Disconnect(c_pData);
			break;

		case HEADER_GG_MESSENGER_ADD:
			MessengerAdd(c_pData);
			break;

		case HEADER_GG_MESSENGER_REMOVE:
			MessengerRemove(c_pData);
			break;
			
#ifdef ENABLE_MESSENGER_BLOCK
		case HEADER_GG_MESSENGER_BLOCK_ADD:
			MessengerBlockAdd(c_pData);
			break;

		case HEADER_GG_MESSENGER_BLOCK_REMOVE:
			MessengerBlockRemove(c_pData);
			break;
#endif

		case HEADER_GG_FIND_POSITION:
			FindPosition(d, c_pData);
			break;

		case HEADER_GG_WARP_CHARACTER:
			WarpCharacter(c_pData);
			break;

		case HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX:
			GuildWarZoneMapIndex(c_pData);
			break;

		case HEADER_GG_TRANSFER:
			Transfer(c_pData);
			break;

		case HEADER_GG_XMAS_WARP_SANTA:
			XmasWarpSanta(c_pData);
			break;

		case HEADER_GG_XMAS_WARP_SANTA_REPLY:
			XmasWarpSantaReply(c_pData);
			break;

		case HEADER_GG_RELOAD_CRC_LIST:
			LoadValidCRCList();
			break;

		case HEADER_GG_CHECK_CLIENT_VERSION:
			CheckClientVersion();
			break;

		case HEADER_GG_LOGIN_PING:
			LoginPing(d, c_pData);
			break;

		case HEADER_GG_BLOCK_CHAT:
			BlockChat(c_pData);
			break;

		case HEADER_GG_SIEGE:
			{
				TPacketGGSiege* pSiege = (TPacketGGSiege*)c_pData;
				castle_siege(pSiege->bEmpire, pSiege->bTowerCount);
			}
			break;

		case HEADER_GG_MONARCH_NOTICE:
			if ((iExtraLen = MonarchNotice(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_MONARCH_TRANSFER :
			MonarchTransfer(d, c_pData);
			break;

		case HEADER_GG_CHECK_AWAKENESS:
			IamAwake(d, c_pData);
			break;

#ifdef ENABLE_MULTI_FARM_BLOCK
		case HEADER_GG_MULTI_FARM:
			MultiFarm(c_pData);
			break;
#endif

#ifdef ENABLE_SWITCHBOT
		case HEADER_GG_SWITCHBOT:
			Switchbot(d, c_pData);
			break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case HEADER_GG_COMMAND:
			if ((iExtraLen = Command(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_RANKING_SYSTEM
		case HEADER_GG_LOAD_RANKING:
			LoadRanking(c_pData);
			break;
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
		case HEADER_GG_STONE_EVENT:
			StoneEvent(d, c_pData);
			break;
#endif

#ifdef ENABLE_EVENT_MANAGER
		case HEADER_GG_EVENT_RELOAD:
			BroadcastEventReload();
			break;
		case HEADER_GG_EVENT:
			Event(c_pData);
			break;

		case HEADER_GG_EVENT_HIDE_AND_SEEK:
			HideAndSeekEvent(c_pData);
			break;

# ifdef ENABLE_BATTLE_PASS_SYSTEM
		case HEADER_GG_EVENT_BATTLE_PASS:
			BattlePassEvent(c_pData);
			break;
# endif
#endif

#ifdef ENABLE_WORLD_BOSS
		case HEADER_GG_WORLD_BOSS:
			WorldBoss(d, c_pData);
			break;
#endif
	}

	return (iExtraLen);
}

#ifdef ENABLE_SWITCHBOT
void CInputP2P::Switchbot(LPDESC d, const char* c_pData)
{
	const TPacketGGSwitchbot* p = reinterpret_cast<const TPacketGGSwitchbot*>(c_pData);
	if (p->wPort != mother_port)
	{
		return;
	}

	CSwitchbotManager::Instance().P2PReceiveSwitchbot(p->table);
}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
void CInputP2P::MultiFarm(const char* c_pData)
{
	TPacketGGMultiFarm* p = (TPacketGGMultiFarm*)c_pData;
	if (p->subHeader == MULTI_FARM_SET)
		CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(p->playerIP, p->playerID, p->playerName, p->farmStatus, p->affectType, p->affectTime, true);
	else if (p->subHeader == MULTI_FARM_REMOVE)
		CHARACTER_MANAGER::Instance().RemoveMultiFarm(p->playerIP, p->playerID, true);
}
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
void CInputP2P::StoneEvent(LPDESC d, const char* c_pData)
{
	TPacketGGStoneEvent* p = (TPacketGGStoneEvent*)c_pData;
	CMiniGameManager::Instance().SetStoneKill(p->pid);
}
#endif

#ifdef ENABLE_WORLD_BOSS
void CInputP2P::WorldBoss(LPDESC d, const char* c_pData)
{
	TPacketGGSendWorldBossStates* pack = (TPacketGGSendWorldBossStates*)c_pData;

	auto func = [&pack](auto& ch) -> void
	{
		if (ch && ch->IsPC())
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "worldboss update|%d|%d|%d", pack->WBState, pack->WBTimer, pack->WBCooldown);
		}
	};

	CHARACTER_MANAGER::instance().for_each_pc(func);
}
#endif
