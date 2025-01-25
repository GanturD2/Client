#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "protocol.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "cmd.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "battle.h"
#include "exchange.h"
#include "questmanager.h"
#include "profiler.h"
#include "messenger_manager.h"
#include "party.h"
#include "p2p.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "log.h"
#include "banword.h"
#include "empire_text_convert.h"
#include "unique_item.h"
#include "building.h"
#include "locale_service.h"
#include "gm.h"
#include "spam.h"
#include "ani.h"
#include "motion.h"
#include "OXEvent.h"
#include "locale_service.h"
#include "DragonSoul.h"
#include "belt_inventory_helper.h" // @fixme119
#include "../../common/CommonDefines.h"
#include "input.h"
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "DungeonInfo.h"
#endif
#ifdef ENABLE_SWITCHBOT
#	include "switchbot.h"
#endif
#ifdef ENABLE_ATTR_6TH_7TH
#	include "Attr6th7th.h"
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "../../libgame/include/grid.h"
#	include "shop.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_INGAME_WIKI
#	include "mob_manager.h"
#endif
#ifdef ENABLE_TICKET_SYSTEM
#	include "ticket.h"
#endif
#ifdef ENABLE_MINI_GAME
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
#	include "battle_pass.h"
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
#	include "entity.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
#	include "BiologSystemManager.h"
#endif

#ifdef ENABLE_CHAT_LOGGING
static char __escape_string[1024];
static char __escape_string2[1024];
#endif

static int __deposit_limit() noexcept
{
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	return (GOLD_MAX);
#else
	return (1000 * 10000); // 10 million
#endif
}

#ifdef ENABLE_SEND_TARGET_INFO
void CInputMain::TargetInfoLoad(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGTargetInfoLoad* p = (TPacketCGTargetInfoLoad*)c_pData;
	TPacketGCTargetInfo pInfo{};
	pInfo.header = HEADER_GC_TARGET_INFO;
	static std::vector<LPITEM> s_vec_item;
	s_vec_item.clear();
	LPITEM pkInfoItem = 0;
	LPCHARACTER m_pkChrTarget = CHARACTER_MANAGER::Instance().Find(p->dwVID);

	// if (m_pkChrTarget && (m_pkChrTarget->IsMonster() || m_pkChrTarget->IsStone()))
	// {
		// if (thecore_heart->pulse - (int) ch->GetLastTargetInfoPulse() < passes_per_sec * 3)
			// return;

		// ch->SetLastTargetInfoPulse(thecore_heart->pulse);

	if (ITEM_MANAGER::Instance().CreateDropItemVector(m_pkChrTarget, ch, s_vec_item) && (m_pkChrTarget->IsMonster() || m_pkChrTarget->IsStone()))
	{
		if (s_vec_item.size() == 0);
		else if (s_vec_item.size() == 1)
		{
			pkInfoItem = s_vec_item[0];
			pInfo.dwVID = m_pkChrTarget->GetVID();
			pInfo.race = m_pkChrTarget->GetRaceNum();
			pInfo.dwVnum = pkInfoItem->GetVnum();
			pInfo.count = pkInfoItem->GetCount();
			ch->GetDesc()->Packet(&pInfo, sizeof(TPacketGCTargetInfo));
		}
		else
		{
			int iItemIdx = s_vec_item.size() - 1;
			while (iItemIdx >= 0)
			{
				pkInfoItem = s_vec_item[iItemIdx--];

				if (!pkInfoItem)
				{
					sys_err("pkInfoItem null in vector idx %d", iItemIdx + 1);
					continue;
				}

				pInfo.dwVID = m_pkChrTarget->GetVID();
				pInfo.race = m_pkChrTarget->GetRaceNum();
				pInfo.dwVnum = pkInfoItem->GetVnum();
				pInfo.count = pkInfoItem->GetCount();
				ch->GetDesc()->Packet(&pInfo, sizeof(TPacketGCTargetInfo));
			}
		}
	}
	// }
}
#endif

static const char * GetEmpireName(int empire)
{
	return c_apszEmpireNamesAlt[empire];
}

void SendBlockChatInfo(LPCHARACTER ch, int sec)
{
	if (!ch)
		return;

	if (sec <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;652]");
		return;
	}

	const long hour = sec / 3600;
	sec -= hour * 3600;

	const long min = (sec / 60);
	sec -= min * 60;

	char buf[128 + 1];

	if (hour > 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("[1028]%d hours %d minutes %d seconds left on your chat block"), hour, min, sec);
	else if (hour > 0 && min == 0)
		snprintf(buf, sizeof(buf), LC_TEXT("[1029]%d hours %d seconds left on your chat block"), hour, sec);
	else if (hour == 0 && min > 0)
		snprintf(buf, sizeof(buf), LC_TEXT("[1030]%d minutes %d seconds left on your chat block"), min, sec);
	else
		snprintf(buf, sizeof(buf), LC_TEXT("[1042]%d seconds left on your chat block"), sec);

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

EVENTINFO(spam_event_info)
{
	char host[MAX_HOST_LENGTH + 1];

	spam_event_info()
	{
		::memset(host, 0, MAX_HOST_LENGTH + 1);
	}
};

typedef std::unordered_map<std::string, std::pair<uint32_t, LPEVENT> > spam_score_of_ip_t;
spam_score_of_ip_t spam_score_of_ip;

EVENTFUNC(block_chat_by_ip_event)
{
	const spam_event_info* info = dynamic_cast<spam_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("block_chat_by_ip_event> <Factor> Null pointer");
		return 0;
	}

	const char* host = info->host;

	const spam_score_of_ip_t::iterator it = spam_score_of_ip.find(host);

	if (it != spam_score_of_ip.end())
	{
		it->second.first = 0;
		it->second.second = nullptr;
	}

	return 0;
}

bool SpamBlockCheck(LPCHARACTER ch, const char* const buf, const size_t buflen)
{
	if (!ch)
		return false;

	if (ch->GetLevel() < g_iSpamBlockMaxLevel)
	{
		spam_score_of_ip_t::iterator it = spam_score_of_ip.find(ch->GetDesc()->GetHostName());

		if (it == spam_score_of_ip.end())
		{
			spam_score_of_ip.insert(std::make_pair(ch->GetDesc()->GetHostName(), std::make_pair(0, (LPEVENT)nullptr)));
			it = spam_score_of_ip.find(ch->GetDesc()->GetHostName());
		}

		if (it->second.second)
		{
			SendBlockChatInfo(ch, event_time(it->second.second) / passes_per_sec);
			return true;
		}

		uint32_t score;
		const char* word = SpamManager::Instance().GetSpamScore(buf, buflen, score);

		it->second.first += score;

		if (word)
			sys_log(0, "SPAM_SCORE: %s text: %s score: %u total: %u word: %s", ch->GetName(), buf, score, it->second.first, word);

		if (it->second.first >= g_uiSpamBlockScore)
		{
			spam_event_info* info = AllocEventInfo<spam_event_info>();
			strlcpy(info->host, ch->GetDesc()->GetHostName(), sizeof(info->host));

			it->second.second = event_create(block_chat_by_ip_event, info, PASSES_PER_SEC(g_uiSpamBlockDuration));
			sys_log(0, "SPAM_IP: %s for %u seconds", info->host, g_uiSpamBlockDuration);

			LogManager::Instance().CharLog(ch, 0, "SPAM", word);

			SendBlockChatInfo(ch, event_time(it->second.second) / passes_per_sec);

			return true;
		}
	}

	return false;
}

enum
{
	TEXT_TAG_PLAIN,
	TEXT_TAG_TAG, // ||
	TEXT_TAG_COLOR, // |cffffffff
	TEXT_TAG_HYPERLINK_START, // |H
	TEXT_TAG_HYPERLINK_END, // |h ex) |Hitem:1234:1:1:1|h
	TEXT_TAG_RESTORE_COLOR,
};

int GetTextTag(const char* src, int maxLen, int& tagLen, std::string& extraInfo)
{
	tagLen = 1;

	if (maxLen < 2 || *src != '|')
		return TEXT_TAG_PLAIN;

	const char* cur = ++src;

	if (*cur == '|') // ||Is denoted by |.
	{
		tagLen = 2;
		return TEXT_TAG_TAG;
	}
	else if (*cur == 'c') // color |cffffffffblahblah|r
	{
		tagLen = 2;
		return TEXT_TAG_COLOR;
	}
	else if (*cur == 'H') // hyperlink |Hitem:10000:0:0:0:0|h[name]|h
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_START;
	}
	else if (*cur == 'h') // end of hyperlink
	{
		tagLen = 2;
		return TEXT_TAG_HYPERLINK_END;
	}

	return TEXT_TAG_PLAIN;
}

void GetTextTagInfo(const char* src, int src_len, int& hyperlinks, bool& colored)
{
	colored = false;
	hyperlinks = 0;

	int len;
	std::string extraInfo;

	for (int i = 0; i < src_len;)
	{
		const int tag = GetTextTag(&src[i], src_len - i, len, extraInfo);

		if (tag == TEXT_TAG_HYPERLINK_START)
			++hyperlinks;

		if (tag == TEXT_TAG_COLOR)
			colored = true;

		i += len;
	}
}

int ProcessTextTag(LPCHARACTER ch, const char* c_pszText, size_t len)
{
	//In case of using the Geumgangkyung in private shop
	//0 : Use normally
	//1 : Lack of sutra
	//2 : There is a Sutra, but it is being used in a private shop
	//3 : Etc
	//4 : Error
	int hyperlinks;
	bool colored;

	GetTextTagInfo(c_pszText, len, hyperlinks, colored);

	if (colored && hyperlinks == 0)
		return 4;

#ifdef ENABLE_NEWSTUFF
	if (g_bDisablePrismNeed)
		return 0;
#endif
	const int nPrismCount = ch->CountSpecifyItem(ITEM_PRISM);
	if (nPrismCount < hyperlinks)
		return 1;

	if (!ch->GetMyShop())
	{
		ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
		return 0;
	}
	else
	{
		const int sellingNumber = ch->GetMyShop()->GetNumberByVnum(ITEM_PRISM);
		if (nPrismCount - sellingNumber < hyperlinks)
		{
			return 2;
		}
		else
		{
			ch->RemoveSpecifyItem(ITEM_PRISM, hyperlinks);
			return 0;
		}
	}

	return 4;
}

int CInputMain::Whisper(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGWhisper* pinfo = reinterpret_cast<const TPacketCGWhisper*>(data);

	if (uiBytes < pinfo->wSize)
		return -1;

	const int iExtraLen = pinfo->wSize - sizeof(TPacketCGWhisper);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

#ifdef ENABLE_WHISPER_CHAT_SPAMLIMIT
	if (ch->IncreaseChatCounter() >= 10)
	{
		ch->GetDesc()->DelayedDisconnect(0);
		return (iExtraLen);
	}
#endif

	if (ch->FindAffect(AFFECT_BLOCK_CHAT))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;652]");
		return (iExtraLen);
	}

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;118]");
		return (iExtraLen);
	}
#endif

	LPCHARACTER pkChr = CHARACTER_MANAGER::Instance().FindPC(pinfo->szNameTo);
	if (pkChr == ch)
		return (iExtraLen);

	LPDESC pkDesc = nullptr;

	uint8_t bOpponentEmpire = 0;

	if (test_server)
	{
		if (!pkChr)
			sys_log(0, "Whisper to %s(%s) from %s", "Null", pinfo->szNameTo, ch->GetName());
		else
			sys_log(0, "Whisper to %s(%s) from %s", pkChr->GetName(), pinfo->szNameTo, ch->GetName());
	}

	if (ch->IsBlockMode(BLOCK_WHISPER))
	{
		if (ch->GetDesc())
		{
			TPacketGCWhisper pack{};
			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
			pack.wSize = sizeof(TPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(pack));
		}
		return iExtraLen;
	}

	if (!pkChr)
	{
		CCI* pkCCI = P2P_MANAGER::Instance().Find(pinfo->szNameTo);

		if (pkCCI)
		{
			pkDesc = pkCCI->pkDesc;
			pkDesc->SetRelay(pinfo->szNameTo);
			bOpponentEmpire = pkCCI->bEmpire;

			if (test_server)
				sys_log(0, "Whisper to %s from %s (Channel %d Mapindex %d)", "Null", ch->GetName(), pkCCI->bChannel, pkCCI->lMapIndex);
		}
	}
	else
	{
		pkDesc = pkChr->GetDesc();
		bOpponentEmpire = pkChr->GetEmpire();
	}

	if (!pkDesc)
	{
		if (ch->GetDesc())
		{
			TPacketGCWhisper pack{};
			pack.bHeader = HEADER_GC_WHISPER;
			pack.bType = WHISPER_TYPE_NOT_EXIST;
			pack.wSize = sizeof(TPacketGCWhisper);
			strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
			ch->GetDesc()->Packet(&pack, sizeof(TPacketGCWhisper));
			sys_log(0, "WHISPER: no player");
		}
	}
	else
	{
		if (ch->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack{};
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_SENDER_BLOCKED;
				pack.wSize = sizeof(TPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		else if (pkChr && pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack{};
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_TARGET_BLOCKED;
				pack.wSize = sizeof(TPacketGCWhisper);
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
#ifdef ENABLE_MESSENGER_BLOCK
		else if (pkChr && MessengerManager::Instance().IsBlocked(ch->GetName(), pkChr->GetName()))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack{};

				char msg[CHAT_MAX_LEN + 1];
				snprintf(msg, sizeof(msg), LC_TEXT("Unblock %s to continue."), pkChr->GetName());
				int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);

				pack.bHeader = HEADER_GC_WHISPER;
				pack.wSize = sizeof(TPacketGCWhisper) + len;
				pack.bType = WHISPER_TYPE_SYSTEM;
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));

				TEMP_BUFFER buf;

				buf.write(&pack, sizeof(TPacketGCWhisper));
				buf.write(msg, len);
				ch->GetDesc()->Packet(buf.read_peek(), buf.size());

				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), pkChr->GetName());
			}
		}
		else if (pkChr && MessengerManager::Instance().IsBlocked(pkChr->GetName(), ch->GetName()))
		{
			if (ch->GetDesc())
			{
				TPacketGCWhisper pack{};

				char msg[CHAT_MAX_LEN + 1];
				snprintf(msg, sizeof(msg), LC_TEXT("%s has blocked you."), pkChr->GetName());
				int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);

				pack.bHeader = HEADER_GC_WHISPER;
				pack.wSize = sizeof(TPacketGCWhisper) + len;
				pack.bType = WHISPER_TYPE_SYSTEM;
				strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));

				TEMP_BUFFER buf;

				buf.write(&pack, sizeof(TPacketGCWhisper));
				buf.write(msg, len);
				ch->GetDesc()->Packet(buf.read_peek(), buf.size());

				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), pkChr->GetName());
			}
		}
#endif
		else
		{
			uint8_t bType = WHISPER_TYPE_NORMAL;

			char buf[CHAT_MAX_LEN + 1];
			strlcpy(buf, data + sizeof(TPacketCGWhisper), MIN(iExtraLen + 1, sizeof(buf)));
			const size_t buflen = strlen(buf);

			if (true == SpamBlockCheck(ch, buf, buflen))
			{
				if (!pkChr)
				{
					const CCI* pkCCI = P2P_MANAGER::Instance().Find(pinfo->szNameTo);
					if (pkCCI)
						pkDesc->SetRelay("");
				}
				return iExtraLen;
			}

			CBanwordManager::Instance().ConvertString(buf, buflen);

			if (g_bEmpireWhisper)
			{
				if (!ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
				{
					if (!(pkChr && pkChr->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)))
					{
						if (bOpponentEmpire != ch->GetEmpire() && ch->GetEmpire() && bOpponentEmpire // With different empires
							&& ch->GetGMLevel() == GM_PLAYER && gm_get_level(pinfo->szNameTo) == GM_PLAYER) // If both are regular players
							// I only know the name, so I use the gm_get_level function
						{
							if (!pkChr)
							{
								// It is on a different server, so it only displays the Empire. The upper 4 bits of bType are used as the Empire number.
								bType = ch->GetEmpire() << 4;
							}
							else
							{
								ConvertEmpireText(ch->GetEmpire(), buf, buflen, 10 + 2 * pkChr->GetSkillPower(SKILL_LANGUAGE1 + ch->GetEmpire() - 1)/*Conversion probability*/);
							}
						}
					}
				}
			}

			const int processReturn = ProcessTextTag(ch, buf, buflen);
			if (0 != processReturn)
			{
				if (ch->GetDesc())
				{
					TItemTable* pTable = ITEM_MANAGER::Instance().GetTable(ITEM_PRISM);

					if (pTable)
					{
						char buf[128];
						int len;
						if (3 == processReturn) //In exchange
							len = snprintf(buf, sizeof(buf), LC_TEXT("[432]This function is not available right now."));
						else
						{
							len = snprintf(buf, sizeof(buf), LC_TEXT("[#Unk]%s is required."), pTable->szLocaleName);
						}

						if (len < 0 || len >= (int)sizeof(buf))
							len = sizeof(buf) - 1;

						++len; // \0 Contain characters

						TPacketGCWhisper pack{};
						pack.bHeader = HEADER_GC_WHISPER;
						pack.bType = WHISPER_TYPE_ERROR;
						pack.wSize = sizeof(TPacketGCWhisper) + len;
						strlcpy(pack.szNameFrom, pinfo->szNameTo, sizeof(pack.szNameFrom));

						ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
						ch->GetDesc()->Packet(buf, len);

						sys_log(0, "WHISPER: not enough %s: char: %s", pTable->szLocaleName, ch->GetName());
					}
				}

				// It may be in a relay state, so release the relay.
				pkDesc->SetRelay("");
				return (iExtraLen);
			}

			if (ch->IsGM())
				bType = (bType & 0xF0) | WHISPER_TYPE_GM;

			if (buflen > 0)
			{
				TPacketGCWhisper pack{};
				pack.bHeader = HEADER_GC_WHISPER;
				pack.wSize = static_cast<uint16_t>(sizeof(TPacketGCWhisper) + buflen);
				pack.bType = bType;
				strlcpy(pack.szNameFrom, ch->GetName(), sizeof(pack.szNameFrom));
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				pack.bCanFormat = false;
#endif

				// The reason to write to the buffer without desc->BufferedPacket is
				// This is because packets can be encapsulated by being P2P relayed.
				TEMP_BUFFER tmpbuf;

				tmpbuf.write(&pack, sizeof(pack));
				tmpbuf.write(buf, buflen);

				pkDesc->Packet(tmpbuf.read_peek(), tmpbuf.size());

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				if (ch)
					CAchievementSystem::Instance().OnSocial(ch, achievements::ETaskTypes::TYPE_WHISPER);
#endif

				// @warme006
				// sys_log(0, "WHISPER: %s -> %s : %s", ch->GetName(), pinfo->szNameTo, buf);
#ifdef ENABLE_CHAT_LOGGING
				//if (ch->IsGM())
				{
					LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), buf, buflen);
					LogManager::Instance().EscapeString(__escape_string2, sizeof(__escape_string2), pinfo->szNameTo, sizeof(pack.szNameFrom));
					LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, __escape_string2, "WHISPER", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
				}
#endif
			}
		}
	}
	if (pkDesc)
		pkDesc->SetRelay("");

	return (iExtraLen);
}

struct RawPacketToCharacterFunc
{
	const void* m_buf;
	int m_buf_len;

	RawPacketToCharacterFunc(const void* buf, int buf_len) : m_buf(buf), m_buf_len(buf_len)
	{
	}

	void operator () (LPCHARACTER c)
	{
		if (!c)
			return;

		if (!c->GetDesc())
			return;

		c->GetDesc()->Packet(m_buf, m_buf_len);
	}
};

struct FEmpireChatPacket
{
	packet_chat& p;
	const char* orig_msg;
	int orig_len;
	char converted_msg[CHAT_MAX_LEN + 1];

	uint8_t bEmpire;
	int iMapIndex;
	int namelen;

	FEmpireChatPacket(packet_chat& p, const char* chat_msg, int len, uint8_t bEmpire, int iMapIndex, int iNameLen)
		: p(p), orig_msg(chat_msg), orig_len(len), bEmpire(bEmpire), iMapIndex(iMapIndex), namelen(iNameLen)
	{
		memset(converted_msg, 0, sizeof(converted_msg));
	}

	void operator () (LPDESC d)
	{
		if (!d)
			return;

		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetMapIndex() != iMapIndex)
			return;

		d->BufferedPacket(&p, sizeof(packet_chat));

		if (d->GetEmpire() == bEmpire ||
			bEmpire == 0 ||
			d->GetCharacter()->GetGMLevel() > GM_PLAYER ||
			d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
		{
			d->Packet(orig_msg, orig_len);
		}
		else
		{
			// Each person has a different skill level, so you have to do it every time
			size_t len = strlcpy(converted_msg, orig_msg, sizeof(converted_msg));

			if (len >= sizeof(converted_msg))
				len = sizeof(converted_msg) - 1;

			ConvertEmpireText(bEmpire, converted_msg + namelen, len - namelen, 10 + 2 * d->GetCharacter()->GetSkillPower(SKILL_LANGUAGE1 + bEmpire - 1));
			d->Packet(converted_msg, orig_len);
		}
	}
};

struct FYmirChatPacket
{
	packet_chat& packet;
	const char* m_szChat;
	size_t m_lenChat;
	const char* m_szName;

	int m_iMapIndex;
	uint8_t m_bEmpire;
	bool m_ring;

	char m_orig_msg[CHAT_MAX_LEN + 1];
	int m_len_orig_msg;
	char m_conv_msg[CHAT_MAX_LEN + 1];
	int m_len_conv_msg;

	FYmirChatPacket(packet_chat& p, const char* chat, size_t len_chat, const char* name, size_t len_name, int iMapIndex, uint8_t empire, bool ring)
		: packet(p),
		m_szChat(chat), m_lenChat(len_chat),
		m_szName(name),
		m_iMapIndex(iMapIndex), m_bEmpire(empire),
		m_ring(ring)
	{
		m_len_orig_msg = snprintf(m_orig_msg, sizeof(m_orig_msg), "%s : %s", m_szName, m_szChat) + 1; // Contains null character

		if (m_len_orig_msg < 0 || m_len_orig_msg >= (int)sizeof(m_orig_msg))
			m_len_orig_msg = sizeof(m_orig_msg) - 1;

		m_len_conv_msg = snprintf(m_conv_msg, sizeof(m_conv_msg), "??? : %s", m_szChat) + 1; // Does not contain null characters

		if (m_len_conv_msg < 0 || m_len_conv_msg >= (int)sizeof(m_conv_msg))
			m_len_conv_msg = sizeof(m_conv_msg) - 1;

		ConvertEmpireText(m_bEmpire, m_conv_msg + 6, m_len_conv_msg - 6, 10); // 6 is the length of "???:"
	}

	void operator() (LPDESC d)
	{
		if (!d)
			return;

		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetMapIndex() != m_iMapIndex)
			return;

		if (m_ring ||
			d->GetEmpire() == m_bEmpire ||
			d->GetCharacter()->GetGMLevel() > GM_PLAYER ||
			d->GetCharacter()->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
		{
			packet.size = m_len_orig_msg + sizeof(TPacketGCChat);

			d->BufferedPacket(&packet, sizeof(packet_chat));
			d->Packet(m_orig_msg, m_len_orig_msg);
		}
		else
		{
			packet.size = m_len_conv_msg + sizeof(TPacketGCChat);

			d->BufferedPacket(&packet, sizeof(packet_chat));
			d->Packet(m_conv_msg, m_len_conv_msg);
		}
	}
};

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
void CInputMain::DungeonInfo(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGDungeonInfo* p = (TPacketCGDungeonInfo*)c_pData;

	switch (p->bySubHeader)
	{
		case EAction::CLOSE:
		{
			ch->SetDungeonInfoOpen(false);
			ch->StopDungeonInfoReloadEvent();
		}
		break;

		case EAction::OPEN:
			if (!ch->IsDungeonInfoOpen())
				CDungeonInfoManager::Instance().SendInfo(ch);
			break;

		case EAction::WARP:
			if (ch->IsDungeonInfoOpen())
				CDungeonInfoManager::Instance().Warp(ch, p->byIndex);
			break;

		case EAction::RANK:
			if (ch->IsDungeonInfoOpen())
				CDungeonInfoManager::Instance().Ranking(ch, p->byIndex, p->byRankType);
			break;

		default:
			return;
	}
}
#endif

int CInputMain::Chat(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGChat* pinfo = reinterpret_cast<const TPacketCGChat*>(data);

	if (uiBytes < pinfo->size)
		return -1;

	const int iExtraLen = pinfo->size - sizeof(TPacketCGChat);

	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->size, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char buf[CHAT_MAX_LEN - (CHARACTER_NAME_MAX_LEN + 3) + 1];
	strlcpy(buf, data + sizeof(TPacketCGChat), MIN(iExtraLen + 1, sizeof(buf)));
	const size_t buflen = strlen(buf);

	if (buflen > 1 && *buf == '/')
	{
		interpret_command(ch, buf + 1, buflen - 1);
		return iExtraLen;
	}
#ifdef ENABLE_CHAT_SPAMLIMIT
	ch->IncreaseChatCounter();
	if (ch->GetChatCounter() >= 10)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Do not spam the chat!"));
		sys_log(0, "CHAT_SPAM: %s", ch->GetName());
		ch->GetDesc()->DelayedDisconnect(0);
		return iExtraLen;
	}

/*	if (ch->IncreaseChatCounter() >= 4)
{
		if (ch->GetChatCounter() == 10)
			ch->GetDesc()->DelayedDisconnect(0);
		return iExtraLen;
	}*/
#else
	if (ch->IncreaseChatCounter() >= 10)
	{
		if (ch->GetChatCounter() == 10)
		{
			sys_log(0, "CHAT_HACK: %s", ch->GetName());
			ch->GetDesc()->DelayedDisconnect(5);
		}

		return iExtraLen;
	}
#endif

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;118]");
		return iExtraLen;
	}
#endif

	// Chat Ban Affect Handling
	const CAffect* pAffect = ch->FindAffect(AFFECT_BLOCK_CHAT);

	if (pAffect != nullptr)
	{
		SendBlockChatInfo(ch, pAffect->lDuration);
		return iExtraLen;
	}

	if (SpamBlockCheck(ch, buf, buflen))
	{
		return iExtraLen;
	}

	if (CHAT_TYPE_SHOUT == pinfo->type && g_bShoutFrozen && !ch->IsGM()) {	//@custom022
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The Global Chat is frozen by an Admin."));
		return iExtraLen;
	}

	// @fixme133 begin
	CBanwordManager::Instance().ConvertString(buf, buflen);

	const int processReturn = ProcessTextTag(ch, buf, buflen);
	if (0 != processReturn)
	{
		const TItemTable* pTable = ITEM_MANAGER::Instance().GetTable(ITEM_PRISM);

		if (nullptr != pTable)
		{
			if (3 == processReturn) //In exchange
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1801]");
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]%s is required."), pTable->szLocaleName);
			}
		}

		return iExtraLen;
	}
	// @fixme133 end

	char chatbuf[CHAT_MAX_LEN + 1];
	int len = snprintf(chatbuf, sizeof(chatbuf), "%s : %s", ch->GetName(), buf);

	CBanwordManager::Instance().ConvertString(buf, buflen);	//@fixme000

	if (len < 0 || len >= (int)sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	if (pinfo->type == CHAT_TYPE_SHOUT)
	{
		if (ch->GetLevel() < g_iShoutLimitLevel)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;654;%d]", g_iShoutLimitLevel);
			return (iExtraLen);
		}
		
		if (thecore_heart->pulse - (int)ch->GetLastShoutPulse() < passes_per_sec * 15 && !ch->IsGM())
			return (iExtraLen);

		ch->SetLastShoutPulse(thecore_heart->pulse);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (ch)
			CAchievementSystem::Instance().OnSocial(ch, achievements::ETaskTypes::TYPE_SHOUTS);
#endif

		// Global Chat Start
		char szGlobalChat[CHAT_MAX_LEN + 1];

		if (ch->GetGMLevel() >= GM_LOW_WIZARD)
		{
#ifdef ENABLE_CHAT_COLOR_SYSTEM
			std::string strColor = "|cFFffa200|H|h[";
			switch (ch->GetGMLevel())
			{
				case GM_LOW_WIZARD:
					strColor += "TGM";
					break;
				case GM_WIZARD:
				case GM_HIGH_WIZARD:
				case GM_GOD:
					strColor += "GM";
					break;
				case GM_IMPLEMENTOR:
					strColor += "Staff";
					break;
			}

			strColor += "]|h|r";
			sprintf(szGlobalChat, "%s %s", strColor.c_str(), chatbuf);
#else
			sprintf(szGlobalChat, "%s", chatbuf);
#endif
		}
		else
		{
# ifdef ENABLE_EMPIRE_FLAG
			sprintf(szGlobalChat, "%s", chatbuf);
# else
			sprintf(szGlobalChat, "%s %s", GetEmpireName(ch->GetEmpire()), chatbuf);
# endif
		}
		sprintf(chatbuf, "%s", szGlobalChat);
		// Global Chat Ends

		TPacketGGShout p{};
		p.bHeader = HEADER_GG_SHOUT;
		p.bEmpire = ch->GetEmpire();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		p.bLanguage = ch->GetDesc()->GetLanguage();
#endif
#ifdef ENABLE_EMPIRE_FLAG
		p.bFlag = true;
#endif
#ifdef ENABLE_MESSENGER_BLOCK
		strlcpy(p.szName, ch->GetName(), sizeof(p.szName));
#endif
		strlcpy(p.szText, chatbuf, sizeof(p.szText));

		P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGShout));

		SendShout(chatbuf, ch->GetEmpire()
#ifdef ENABLE_MESSENGER_BLOCK
			, ch->GetName()
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			, ch->GetDesc()->GetLanguage()
#endif
#ifdef ENABLE_EMPIRE_FLAG
			, true
#endif
		);

		return (iExtraLen);	//Memory leak: timeBuf [memleak]
	}

	TPacketGCChat pack_chat{};
	pack_chat.header = HEADER_GC_CHAT;
	pack_chat.size = sizeof(TPacketGCChat) + len;
	pack_chat.type = pinfo->type;
	pack_chat.id = ch->GetVID();
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pack_chat.bCanFormat = false;
	pack_chat.bLanguage = ch->GetDesc()->GetLanguage();
#endif
#ifdef ENABLE_EMPIRE_FLAG
	pack_chat.bFlag = false;
#endif

	switch (pinfo->type)
	{
		case CHAT_TYPE_TALKING:
			{
				const DESC_MANAGER::DESC_SET& c_ref_set = DESC_MANAGER::Instance().GetClientSet();

				if (false)
				{
					std::for_each(c_ref_set.begin(), c_ref_set.end(),
						FYmirChatPacket(pack_chat,
							buf,
							strlen(buf),
							ch->GetName(),
							strlen(ch->GetName()),
							ch->GetMapIndex(),
							ch->GetEmpire(),
							ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)));
				}
				else
				{
					std::for_each(c_ref_set.begin(), c_ref_set.end(),
						FEmpireChatPacket(pack_chat,
							chatbuf,
							len,
							(ch->GetGMLevel() > GM_PLAYER || ch->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE)) ? 0 : ch->GetEmpire(),
							ch->GetMapIndex(), strlen(ch->GetName())));
#ifdef ENABLE_CHAT_LOGGING
					//if (ch->IsGM())
					{
						LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), 0, "", "NORMAL", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
#endif
				}
			}
			break;

		case CHAT_TYPE_PARTY:
			{
				if (!ch->GetParty())
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;655]");
				else
				{
					TEMP_BUFFER tbuf;

					tbuf.write(&pack_chat, sizeof(pack_chat));
					tbuf.write(chatbuf, len);

					RawPacketToCharacterFunc f(tbuf.read_peek(), tbuf.size());
					ch->GetParty()->ForEachOnlineMember(f);
#ifdef ENABLE_CHAT_LOGGING
					//if (ch->IsGM())
					{
						LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetParty()->GetLeaderPID(), "", "PARTY", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
#endif
				}
			}
			break;

		case CHAT_TYPE_GUILD:
			{
				if (!ch->GetGuild())
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;656]");
				else
				{
					ch->GetGuild()->Chat(chatbuf);
#ifdef ENABLE_CHAT_LOGGING
					//if (ch->IsGM())
					{
						LogManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), chatbuf, len);
						LogManager::Instance().ChatLog(ch->GetMapIndex(), ch->GetPlayerID(), ch->GetName(), ch->GetGuild()->GetID(), ch->GetGuild()->GetName(), "GUILD", __escape_string, ch->GetDesc() ? ch->GetDesc()->GetHostName() : "");
					}
#endif
				}
			}
			break;

		default:
			sys_err("Unknown chat type %d", pinfo->type);
			break;
	}

	return (iExtraLen);
}

void CInputMain::ItemUse(LPCHARACTER ch, const char* data)
{
	if (ch)
		ch->UseItem(((struct command_item_use*)data)->Cell);
}

void CInputMain::ItemToItem(LPCHARACTER ch, const char* pcData)
{
	const TPacketCGItemUseToItem* p = (TPacketCGItemUseToItem*)pcData;
	if (ch)
		ch->UseItem(p->Cell, p->TargetCell);
}

void CInputMain::ItemDrop(LPCHARACTER ch, const char* data)
{
	const struct command_item_drop* pinfo = (struct command_item_drop*)data;

	//MONARCH_LIMIT
	//if (ch->IsMonarch())
	//	return;
	//END_MONARCH_LIMIT
	if (!ch)
		return;

	// If the elk is greater than 0, the elk is discarded.
	if (pinfo->gold > 0)
		ch->DropGold(pinfo->gold);
	else
		ch->DropItem(pinfo->Cell);
}

void CInputMain::ItemDrop2(LPCHARACTER ch, const char* data)
{
	//MONARCH_LIMIT
	//if (ch->IsMonarch())
	//	return;
	//END_MONARCH_LIMIT

	const TPacketCGItemDrop2* pinfo = (TPacketCGItemDrop2*)data;

	// If the elk is greater than 0, the elk is discarded.

	if (!ch)
		return;

	if (pinfo->gold > 0)
		ch->DropGold(pinfo->gold);
	else
		ch->DropItem(pinfo->Cell, pinfo->count);
}

#ifdef ENABLE_DESTROY_SYSTEM
void CInputMain::ItemDestroy(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	//MONARCH_LIMIT
	//if (ch->IsMonarch())
	//	return;
	//END_MONARCH_LIMIT

	TPacketCGItemDestroy* pinfo = (TPacketCGItemDestroy*)data;

	if (pinfo->gold > 0)
		ch->PointChange(POINT_GOLD, -pinfo->gold);
	else
		ch->RemoveItem(pinfo->Cell, pinfo->count);
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
bool CompareItemVnumAcPriceAC(CShop::SHOP_ITEM i, CShop::SHOP_ITEM j)
{
	return (i.vnum < j.vnum) && (i.price < j.price);
}

void CInputMain::PrivateShopSearchSearch(LPCHARACTER ch, const char* data)
{
	const TPacketCGPrivateShopSearch* pinfo = (TPacketCGPrivateShopSearch*)data;

	if (!ch)
		return;

	if (!data)
		return;

	if ((ch->GetExchange() || ch->IsOpenSafebox()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| ch->GetViewingShopOwner())
#else
		|| ch->GetShopOwner())
#endif
		|| ch->IsCubeOpen()
#ifdef ENABLE_CHANGED_ATTR
		|| ch->IsSelectAttr()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| ch->IsChangeLookWindowOpen()
#endif
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_DO_THIS_BECAUSE_OTHER_WINDOW_OPEN"));
		return;
	}

	if (ch->IsDead())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("privateshop_search_disable") == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_SEARCH_SYSTEM_DISABLED"));
		return;
	}

	const uint8_t bRace = pinfo->bRace;
	const uint8_t bMaskType = pinfo->bMaskType;
	const uint8_t bMaskSubType = pinfo->bMaskSubType;
	const uint8_t bMinRefine = pinfo->bMinRefine;
	const uint8_t bMaxRefine = pinfo->bMaxRefine;
	const uint8_t bMinLevel = pinfo->bMinLevel;
	const uint8_t bMaxLevel = pinfo->bMaxLevel;
	const int iMinGold = pinfo->iMinGold;
	const int iMaxGold = pinfo->iMaxGold;
	std::string itemName = "";
#ifdef ENABLE_CHEQUE_SYSTEM
	const uint32_t dwMinCheque = pinfo->dwMinCheque;
	const uint32_t dwMaxCheque = pinfo->dwMaxCheque;
#endif

	//Checks
	if (bRace < JOB_WARRIOR
#ifdef ENABLE_WOLFMAN_CHARACTER
		|| bRace > JOB_WOLFMAN
#else
		|| bRace > JOB_SURA
#endif
		)
	{
		return;
	}

	switch (bMaskType)
	{
		case MASK_ITEM_TYPE_MOUNT_PET:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_MOUNT_PET_MOUNT || bMaskSubType > MASK_ITEM_SUBTYPE_MOUNT_PET_EGG)
				return;
		}
		break;

		case MASK_ITEM_TYPE_EQUIPMENT_WEAPON:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_WEAPON_WEAPON_SWORD || bMaskSubType > MASK_ITEM_SUBTYPE_WEAPON_WEAPON_QUIVER)
				return;
		}
		break;

		case MASK_ITEM_TYPE_EQUIPMENT_ARMOR:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_ARMOR_ARMOR_BODY
#ifdef ENABLE_GLOVE_SYSTEM
				|| bMaskSubType > MASK_ITEM_SUBTYPE_ARMOR_ARMOR_GLOVE
#else
				|| bMaskSubType > MASK_ITEM_SUBTYPE_ARMOR_ARMOR_SHIELD
#endif
				)
			{
				return;
			}
		}
		break;

		case MASK_ITEM_TYPE_EQUIPMENT_JEWELRY:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_WRIST
#ifdef ENABLE_PENDANT
				|| bMaskSubType > MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_PENDANT
#else
				|| bMaskSubType > MASK_ITEM_SUBTYPE_JEWELRY_ITEM_BELT
#endif
				)
			{
				return;
			}
		}
		break;

		case MASK_ITEM_TYPE_TUNING:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_TUNING_RESOURCE || bMaskSubType > MASK_ITEM_SUBTYPE_TUNING_ETC)
				return;
		}
		break;

		case MASK_ITEM_TYPE_POTION:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_POTION_ABILITY || bMaskSubType > MASK_ITEM_SUBTYPE_POTION_ETC)
				return;
		}
		break;

		case MASK_ITEM_TYPE_FISHING_PICK:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_FISHING_PICK_FISHING_POLE || bMaskSubType > MASK_ITEM_SUBTYPE_FISHING_PICK_ETC)
				return;
		}
		break;

		case MASK_ITEM_TYPE_DRAGON_STONE:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_DIAMOND || bMaskSubType > MASK_ITEM_SUBTYPE_DRAGON_STONE_ETC)
				return;
		}
		break;

		case MASK_ITEM_TYPE_COSTUMES:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_WEAPON
#ifdef ENABLE_AURA_SYSTEM
				|| bMaskSubType > MASK_ITEM_SUBTYPE_COSTUMES_AURA
#else
				|| bMaskSubType > MASK_ITEM_SUBTYPE_COSTUMES_ETC
#endif
				)
			{
				return;
			}
		}
		break;

		case MASK_ITEM_TYPE_SKILL:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_SKILL_PAHAE || bMaskSubType > MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK_HORSE)
				return;
		}
		break;

		case MASK_ITEM_TYPE_UNIQUE:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_UNIQUE_ABILITY || bMaskSubType > MASK_ITEM_SUBTYPE_UNIQUE_ETC)
				return;
		}
		break;

		case MASK_ITEM_TYPE_ETC:
		{
			if (bMaskSubType < MASK_ITEM_SUBTYPE_ETC_GIFTBOX || bMaskSubType > MASK_ITEM_SUBTYPE_ETC_ETC)
				return;
		}
		break;

		default:
			return;
	}

	if ((bMinRefine < 0 || bMinRefine > 15) || (bMaxRefine < 0 || bMaxRefine > 15))
		return;

	if ((bMinLevel < 0 || bMinLevel > PLAYER_MAX_LEVEL_CONST) || (bMaxLevel < 0 || bMaxLevel > PLAYER_MAX_LEVEL_CONST))
		return;

	if ((bMinLevel > bMaxLevel) || (bMinRefine > bMaxRefine))
		return;

	if ((iMinGold < 0 || iMinGold > GOLD_MAX) || (iMaxGold < 0 || iMaxGold > GOLD_MAX) || (iMinGold > iMaxGold))
		return;

#ifdef ENABLE_CHEQUE_SYSTEM
	if ((dwMinCheque < 0 || dwMinCheque > CHEQUE_MAX) || (dwMaxCheque < 0 || dwMaxCheque > CHEQUE_MAX) || (dwMinCheque > dwMaxCheque))
		return;
#endif

	bool bNameInput = false;
	itemName = pinfo->itemName;
	if (itemName.length() > 2)
		bNameInput = true;

	if (bNameInput)
		std::replace(itemName.begin(), itemName.end(), '_', ' ');

	if (bNameInput)
	{
		sys_log(0, "PRIVATE_SHOP_SEARCH (search by name): player_name: %s race: %d"
			"mask: %d sub_mask: %d"
			"min_level: %d max_level: %d min_refine: %d max_refine: %d min_gold: %d max_gold: %d"
#	ifdef ENABLE_CHEQUE_SYSTEM
			"min_cheque: %d max_cheque: %d"
#	endif
			"itemName: %s",
			ch->GetName(), bRace,
			bMaskType, bMaskSubType,
			bMinLevel, bMaxLevel, bMinRefine, bMaxRefine, iMinGold, iMaxGold,
#	ifdef ENABLE_CHEQUE_SYSTEM
			dwMinCheque, dwMaxCheque,
#	endif
			itemName.c_str());
	}
	else
	{
		sys_log(0, "PRIVATE_SHOP_SEARCH: player_name: %s race: %d"
			"mask: %d sub_mask: %d"
			"min_level: %d max_level: %d min_refine: %d max_refine: %d min_gold: %d max_gold: %d"
#	ifdef ENABLE_CHEQUE_SYSTEM
			"min_cheque: %d max_cheque: %d"
#	endif
			,
			ch->GetName(), bRace,
			bMaskType, bMaskSubType,
			bMinLevel, bMaxLevel, bMinRefine, bMaxRefine, iMinGold, iMaxGold
#	ifdef ENABLE_CHEQUE_SYSTEM
			, dwMinCheque, dwMaxCheque
#	endif
		);
	}

	//CEntity::ENTITY_MAP viewmap = ch->GetViewMap();
	//CEntity::ENTITY_MAP::iterator it = viewmap.begin();
	const auto shops = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(30000);
	std::vector<CShop::SHOP_ITEM> sendItems;

	//While (it != viewmap.end())
	for (auto tch : shops)
	{
		//if (it->first->GetType() != ENTITY_CHARACTER)
		if (tch->GetType() != ENTITY_CHARACTER)
		{
			//++it;
			continue;
		}

		//LPCHARACTER tch = static_cast<LPCHARACTER>((it++)->first);

		if (!tch)
			continue;

		if (!tch->IsShop() || !tch->GetMyShop() || tch == ch)
			continue;

		if (tch->IsAffectFlag(AFF_INVISIBILITY))
			continue;

		if (tch->IsEditingShop())
		{
			//sys_log(0, "Player %lu trying to search from editing shop.", ch->GetPlayerID());
			continue;
		}

		auto shop_items = tch->GetMyShop()->GetItemVector();

		for (auto& item : shop_items)
		{
			LPITEM curItem = item.pkItem;
			if (!curItem)
				continue;

			if ((curItem && (curItem->GetMaskType() == bMaskType) && (curItem->GetMaskSubType() == bMaskSubType)))
			{
				if (!(curItem->GetRefineLevel() >= bMinRefine && curItem->GetRefineLevel() <= bMaxRefine)
					&& (curItem->GetMaskType() == MASK_ITEM_TYPE_EQUIPMENT_WEAPON || curItem->GetMaskType() == MASK_ITEM_TYPE_EQUIPMENT_ARMOR))
				{
					continue;
				}

				if (bNameInput && itemName.length() > 2)
				{
					std::string foundName = curItem->GetNameString();
					if (foundName.find(itemName) == std::string::npos)
						continue;
				}

				if (!((curItem->GetLevel() >= bMinLevel && curItem->GetLevel() <= bMaxLevel) || curItem->GetLevel() == 0))
					continue;

				if (!((item.price >= iMinGold) && (item.price <= iMaxGold)))
					continue;

#ifdef ENABLE_CHEQUE_SYSTEM
				if (!((item.cheque >= dwMinCheque) && (item.cheque <= dwMaxCheque)))
					continue;
#endif

				bool cont = false;
				if (curItem->GetMaskType() == MASK_ITEM_TYPE_SKILL && (curItem->GetMaskSubType() == MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK || curItem->GetMaskSubType() == MASK_ITEM_SUBTYPE_SKILL_BOOK_OF_OBLIVION))
				{
					if (bRace == JOB_WARRIOR && !(curItem->GetSocket(0) >= 1 && curItem->GetSocket(0) <= 21))
						cont = true;
					else if (bRace == JOB_ASSASSIN && !(curItem->GetSocket(0) >= 31 && curItem->GetSocket(0) <= 51))
						cont = true;
					else if (bRace == JOB_SURA && !(curItem->GetSocket(0) >= 61 && curItem->GetSocket(0) <= 81))
						cont = true;
					else if (bRace == JOB_SHAMAN && !(curItem->GetSocket(0) >= 91 && curItem->GetSocket(0) <= 111))
						cont = true;
#ifdef ENABLE_WOLFMAN_CHARACTER
					else if (bRace == JOB_WOLFMAN && !(curItem->GetSocket(0) >= 170 && curItem->GetSocket(0) <= 175))
						cont = true;
#endif
				}

				switch (bRace)
				{
					case JOB_WARRIOR:
						if (curItem->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
							cont = true;
						break;

					case JOB_ASSASSIN:
						if (curItem->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
							cont = true;
						break;

					case JOB_SHAMAN:
						if (curItem->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
							cont = true;
						break;

					case JOB_SURA:
						if (curItem->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
							cont = true;
						break;

#ifdef ENABLE_WOLFMAN_CHARACTER
					case JOB_WOLFMAN:
						if (curItem->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
							cont = true;
						break;
#endif
					default:
						break;
				}

				if (cont)
					continue;

				sendItems.emplace_back(item);
			}
		}
	}

	std::stable_sort(sendItems.begin(), sendItems.end(), CompareItemVnumAcPriceAC);

	int real_position = 0;
	std::vector<TPrivateShopSearchItem> vecPrivateShopSearchItem;
	for (auto& item : sendItems)
	{
#ifdef ENABLE_PRIVATESHOP_SEARCH_PACKETFLOOD_TEMP_FIX
		if (real_position >= 800)
			break;
#endif

		LPITEM curItem = item.pkItem;
		if (!curItem)
			continue;

		if (curItem && curItem->GetOwner())
		{
			TPrivateShopSearchItem itemResult;
			itemResult.real_position = real_position++;
			itemResult.vid = curItem->GetOwner()->GetVID();
			thecore_memcpy(itemResult.char_name, curItem->GetOwner()->GetName(), sizeof(itemResult.char_name));
			itemResult.price = item.price;
#ifdef ENABLE_CHEQUE_SYSTEM
			itemResult.cheque = item.cheque;
#endif
			itemResult.Cell = TItemPos(PREMIUM_PRIVATE_SHOP, curItem->GetCell());

			itemResult.Item.vnum = curItem->GetVnum();
			itemResult.Item.count = static_cast<uint8_t>(curItem->GetCount());
			itemResult.Item.flags = curItem->GetFlag();
			itemResult.Item.anti_flags = curItem->GetAntiFlag();
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //SEARCHSHOP_CHANGELOOK
			itemResult.Item.dwTransmutationVnum = curItem->GetChangeLookVnum();
#endif
#ifdef ENABLE_REFINE_ELEMENT //SEARCHSHOP_ELEMENT
			itemResult.Item.grade_element = curItem->GetElementGrade();
			thecore_memcpy(itemResult.Item.attack_element, curItem->GetElementAttacks(), sizeof(itemResult.Item.attack_element));
			itemResult.Item.element_type_bonus = curItem->GetElementsType();
			thecore_memcpy(itemResult.Item.elements_value_bonus, curItem->GetElementsValues(), sizeof(itemResult.Item.elements_value_bonus));
#endif
			thecore_memcpy(itemResult.Item.alSockets, curItem->GetSockets(), sizeof(itemResult.Item.alSockets));
			thecore_memcpy(itemResult.Item.aAttr, curItem->GetAttributes(), sizeof(itemResult.Item.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(itemResult.Item.aApplyRandom, curItem->GetApplysRandom(), sizeof(itemResult.Item.aApplyRandom));
			thecore_memcpy(itemResult.Item.alRandomValues, curItem->GetRandomDefaultAttrs(), sizeof(itemResult.Item.alRandomValues));
#endif

			vecPrivateShopSearchItem.emplace_back(itemResult);
		}
	}

	if (ch && ch->GetDesc())
	{
		TPacketGCPrivateShopSearchItemInfo packet{};
		packet.bHeader = HEADER_GC_PRIVATE_SHOP_SEARCH_SET;
		packet.wSize = sizeof(packet) + sizeof(TPrivateShopSearchItem) * vecPrivateShopSearchItem.size();

		ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
		ch->GetDesc()->Packet(&vecPrivateShopSearchItem[0], sizeof(TPrivateShopSearchItem) * vecPrivateShopSearchItem.size());
	}

}

void CInputMain::PrivateShopSearchBuy(LPCHARACTER ch, const char* data)
{
	const TPacketCGPrivateShopSearchBuy* pinfo = (TPacketCGPrivateShopSearchBuy*)data;

	const uint32_t shopVid = pinfo->shopVid;
	const uint8_t shopItemPos = pinfo->shopItemPos;

	if (!ch)
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("privateshop_search_disable") == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_SEARCH_SYSTEM_DISABLED"));
		return;
	}

	if ((ch->GetExchange() || ch->IsOpenSafebox()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| ch->GetViewingShopOwner())
#else
		|| ch->GetShopOwner())
#endif
		|| ch->IsCubeOpen()
#ifdef ENABLE_CHANGED_ATTR
		|| ch->IsSelectAttr()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| ch->IsChangeLookWindowOpen()
#endif
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_DO_THIS_BECAUSE_OTHER_WINDOW_OPEN"));
		return;
	}

	if (ch->IsDead())
		return;

	LPCHARACTER pkChrShop = CHARACTER_MANAGER::Instance().Find(shopVid);
	if (!pkChrShop)
		return;

	LPSHOP pkShop = pkChrShop->GetMyShop();
	if (!pkShop)
		return;

	if (pkChrShop->IsAffectFlag(AFF_INVISIBILITY))
		return;

	if (pkChrShop->IsEditingShop())
	{
		sys_err("Player %lu trying to buy from editing shop.", ch->GetPlayerID());
		return;
	}

	if (pkShop->IsClosed())
	{
		sys_err("Player %lu trying to buy from closed shop.", ch->GetPlayerID());
		return;
	}

	if (DISTANCE_APPROX(ch->GetX() - pkChrShop->GetX(), ch->GetY() - pkChrShop->GetY()) > VIEW_RANGE)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;877]");
		return;
	}

	if (ch->GetMapIndex() != pkChrShop->GetMapIndex())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_BUY_THIS_BECAUSE_NOT_IN_SAME_MAP"));
		return;
	}

	const int returnHeader = pkShop->Buy(ch, shopItemPos, true);

	if (SHOP_SUBHEADER_GC_OK != returnHeader)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShopSearchError %d", returnHeader);

	//If, after buying, the shop is closed, destroy it (through its owner char)
	if (pkShop->IsClosed() && pkShop->IsPCShop())
	{
		M2_DESTROY_CHARACTER(pkShop->GetOwner());
	}
}

void CInputMain::PrivateShopSearchClose(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	ch->SetPrivateShopSearchOpen(false);
}
#endif

void CInputMain::ItemMove(LPCHARACTER ch, const char* data)
{
	const struct command_item_move* pinfo = (struct command_item_move*)data;
	if (ch)
		ch->MoveItem(pinfo->Cell, pinfo->CellTo, pinfo->count);
}

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
void CInputMain::SoulRoulette(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const TPacketCGSoulRoulette* pinfo = reinterpret_cast<const TPacketCGSoulRoulette*>(data);
	enum { CLOSE, TURN, GIVE };

	switch (pinfo->option)
	{
		case CLOSE:
			if (ch->GetSoulRoulette())
			{
				if (ch->GetSoulRoulette()->GetGiftVnum())
					ch->ChatPacket(CHAT_TYPE_INFO, "Please wait, <Soul Roulette> is active now.");
				else
				{
					ch->GetSoulRoulette()->SendPacket(CSoulRoulette::Packet::CLOSE);
					ch->SetSoulRoulette(nullptr);
				}
			}
			break;

		case TURN:
			if (ch->GetSoulRoulette())
				ch->GetSoulRoulette()->TurnWheel();
			break;

		case GIVE:
			if (ch->GetSoulRoulette())
				ch->GetSoulRoulette()->GiveGift();
			break;

		default:
			sys_err("CInputMain::SoulRoulette : Unknown option %d : %s", pinfo->option, ch->GetName());
			break;
	}
}
#endif

void CInputMain::ItemPickup(LPCHARACTER ch, const char* data)
{
	const TPacketCGItemPickup* pinfo = (TPacketCGItemPickup*)data;
	if (ch)
	{
#ifdef ENABLE_CHECK_PICKUP_HACK
		if (ch->PickupItem(pinfo->vid))
		{
			uint32_t d_difference = get_dword_time() - ch->GetLastPickupTime();
			if (d_difference < ENABLE_CHECK_PICKUP_HACK && d_difference > 3)
			{
				LPDESC d = ch->GetDesc();
				if (d)
				{
					if (d->DelayedDisconnect(3))
					{
						LogManager::Instance().HackLog("Pickup Hack", ch);
					}
				}
			}
			ch->SetLastPickupTime();
		}
#else
		ch->PickupItem(pinfo->vid);
#endif
	}
}

void CInputMain::QuickslotAdd(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	TPacketCGQuickslotAdd* pinfo = (TPacketCGQuickslotAdd*)data;

	if (pinfo->slot.type == QUICKSLOT_TYPE_ITEM)	//@fixme528
	{
		LPITEM item = nullptr;
		const TItemPos srcCell(INVENTORY, pinfo->slot.pos);

		if (!(item = ch->GetItem(srcCell)))
			return;

		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_QUICKSLOT))
			return;

		switch (item->GetType())
		{
			case ITEM_USE:
			case ITEM_QUEST:
			case ITEM_PET:
#ifdef ENABLE_SOUL_SYSTEM
			case ITEM_SOUL:
#endif
				break;

			default:
				return;
		}
	}

	ch->SetQuickslot(pinfo->pos, pinfo->slot);
}

void CInputMain::QuickslotDelete(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const TPacketCGQuickslotDel* pinfo = (TPacketCGQuickslotDel*)data;
	ch->DelQuickslot(pinfo->pos);
}

void CInputMain::QuickslotSwap(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const TPacketCGQuickslotSwap* pinfo = (TPacketCGQuickslotSwap*)data;
	ch->SwapQuickslot(pinfo->pos, pinfo->change_pos);
}

int CInputMain::Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	if (!ch)
		return -1;

	const TPacketCGMessenger* p = (TPacketCGMessenger*)c_pData;

	if (uiBytes < sizeof(TPacketCGMessenger))
		return -1;

	c_pData += sizeof(TPacketCGMessenger);
	uiBytes -= sizeof(TPacketCGMessenger);

	switch (p->subheader)
	{
		case MESSENGER_SUBHEADER_CG_ADD_BY_VID:
			{
				if (uiBytes < sizeof(TPacketCGMessengerAddByVID))
					return -1;

				const TPacketCGMessengerAddByVID* p2 = (TPacketCGMessengerAddByVID*)c_pData;
				LPCHARACTER ch_companion = CHARACTER_MANAGER::Instance().Find(p2->vid);

				if (!ch_companion)
					return sizeof(TPacketCGMessengerAddByVID);

				if (ch->IsObserverMode())
					return sizeof(TPacketCGMessengerAddByVID);

				if (ch_companion->IsBlockMode(BLOCK_MESSENGER_INVITE))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;657]");
					return sizeof(TPacketCGMessengerAddByVID);
				}

				const DESC* d = ch_companion->GetDesc();

				if (!d)
					return sizeof(TPacketCGMessengerAddByVID);

				if (ch->GetGMLevel() == GM_PLAYER && ch_companion->GetGMLevel() != GM_PLAYER)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;659]");
					return sizeof(TPacketCGMessengerAddByVID);
				}

				if (ch->GetDesc() == d) // You cannot add yourself.
					return sizeof(TPacketCGMessengerAddByVID);

#ifdef ENABLE_MESSENGER_BLOCK
				if (MessengerManager::Instance().IsBlocked(ch->GetName(), ch_companion->GetName()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;138;%s]", ch_companion->GetName());
					return sizeof(TPacketCGMessengerAddByVID);
				}

				if (MessengerManager::Instance().IsBlocked(ch_companion->GetName(), ch->GetName()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;139;%s]", ch_companion->GetName());
					return sizeof(TPacketCGMessengerAddByVID);
				}
#endif
				
				MessengerManager::Instance().RequestToAdd(ch, ch_companion);
				//MessengerManager::Instance().AddToList(ch->GetName(), ch_companion->GetName());
			}
			return sizeof(TPacketCGMessengerAddByVID);

		case MESSENGER_SUBHEADER_CG_ADD_BY_NAME:
			{
				if (uiBytes < CHARACTER_NAME_MAX_LEN)
					return -1;

				char name[CHARACTER_NAME_MAX_LEN + 1];
				strlcpy(name, c_pData, sizeof(name));

				if (ch->GetGMLevel() == GM_PLAYER && gm_get_level(name) != GM_PLAYER)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;659]");
					return CHARACTER_NAME_MAX_LEN;
				}

#ifdef ENABLE_BATTLE_FIELD
				if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_ZONE_CANT_ADD_FRIEND"));
					return CHARACTER_NAME_MAX_LEN;
				}
#endif

				LPCHARACTER tch = CHARACTER_MANAGER::Instance().FindPC(name);

				if (!tch)
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;660;%s]", name);
				else
				{
					if (tch == ch) // You cannot add yourself.
						return CHARACTER_NAME_MAX_LEN;

					if (tch->IsBlockMode(BLOCK_MESSENGER_INVITE) == true)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;657]");
					}
					else
					{
#ifdef ENABLE_MESSENGER_BLOCK
						if (MessengerManager::Instance().IsBlocked(ch->GetName(), tch->GetName()))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), tch->GetName());
							return CHARACTER_NAME_MAX_LEN;
						}

						if (MessengerManager::Instance().IsBlocked(tch->GetName(), ch->GetName()))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), tch->GetName());
							return CHARACTER_NAME_MAX_LEN;
						}
#endif
						MessengerManager::Instance().RequestToAdd(ch, tch);
						//MessengerManager::Instance().AddToList(ch->GetName(), tch->GetName());
					}
				}
			}
			return CHARACTER_NAME_MAX_LEN;

		case MESSENGER_SUBHEADER_CG_REMOVE:
			{
				if (uiBytes < CHARACTER_NAME_MAX_LEN)
					return -1;

				char char_name[CHARACTER_NAME_MAX_LEN + 1];
				strlcpy(char_name, c_pData, sizeof(char_name));
				MessengerManager::Instance().RemoveFromList(ch->GetName(), char_name);
				MessengerManager::Instance().RemoveFromList(char_name, ch->GetName());	//@fixme183
			}
			return CHARACTER_NAME_MAX_LEN;

#ifdef ENABLE_MESSENGER_BLOCK
		case MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_VID:
		{
			if (uiBytes < sizeof(TPacketCGMessengerAddBlockByVID))
				return -1;

			const TPacketCGMessengerAddBlockByVID* p2 = (TPacketCGMessengerAddBlockByVID*)c_pData;
			const LPCHARACTER& ch_companion = CHARACTER_MANAGER::Instance().Find(p2->vid);

			if (!ch_companion)
				return sizeof(TPacketCGMessengerAddBlockByVID);

			if (ch->IsObserverMode())
				return sizeof(TPacketCGMessengerAddBlockByVID);

			const LPDESC& d = ch_companion->GetDesc();

			if (!d)
				return sizeof(TPacketCGMessengerAddByVID);

			LPCHARACTER pkPartner = ch->GetMarryPartner();
			if (pkPartner)
			{
				if (ch_companion->GetName() == pkPartner->GetName())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot block your spouse."));
					return sizeof(TPacketCGMessengerAddBlockByVID);
				}
			}

			if (MessengerManager::Instance().IsBlocked(ch->GetName(), ch_companion->GetName()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Remove %s from your friends list to continue."), ch_companion->GetName());
				return sizeof(TPacketCGMessengerAddBlockByVID);
			}

			if (MessengerManager::Instance().IsBlocked(ch->GetName(), ch_companion->GetName()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s is already being blocked."), ch_companion->GetName());
				return sizeof(TPacketCGMessengerAddBlockByVID);
			}

			if (ch->GetGMLevel() == GM_PLAYER && ch_companion->GetGMLevel() != GM_PLAYER && !test_server)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot block this player."));
				return sizeof(TPacketCGMessengerAddByVID);
			}

			if (ch->GetDesc() == d)
				return sizeof(TPacketCGMessengerAddBlockByVID);

			MessengerManager::Instance().AddToBlockList(ch->GetName(), ch_companion->GetName());
		}
		return sizeof(TPacketCGMessengerAddBlockByVID);

		case MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_NAME:
		{
			if (uiBytes < CHARACTER_NAME_MAX_LEN)
				return -1;

			char name[CHARACTER_NAME_MAX_LEN + 1];
			strlcpy(name, c_pData, sizeof(name));

			if (gm_get_level(name) != GM_PLAYER && !test_server)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot block this player."));
				return CHARACTER_NAME_MAX_LEN;
			}

			const LPCHARACTER& tch = CHARACTER_MANAGER::Instance().FindPC(name);

			if (!tch)
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s is not online."), name);
			else
			{
				if (tch == ch)
					return CHARACTER_NAME_MAX_LEN;

				LPCHARACTER partner = ch->GetMarryPartner();
				if (partner)
				{
					if (tch->GetName() == partner->GetName())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot block your spouse."));
						return CHARACTER_NAME_MAX_LEN;
					}
				}

				if (MessengerManager::Instance().IsBlocked(ch->GetName(), tch->GetName()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Remove %s from your friends list to continue."), tch->GetName());
					return CHARACTER_NAME_MAX_LEN;
				}

				if (MessengerManager::Instance().IsBlocked(ch->GetName(), tch->GetName()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s is already being blocked."), tch->GetName());
					return CHARACTER_NAME_MAX_LEN;
				}

				MessengerManager::Instance().AddToBlockList(ch->GetName(), tch->GetName());
			}
		}
		return CHARACTER_NAME_MAX_LEN;

		case MESSENGER_SUBHEADER_CG_BLOCK_REMOVE_BLOCK:
		{
			if (uiBytes < CHARACTER_NAME_MAX_LEN)
				return -1;

			char char_name[CHARACTER_NAME_MAX_LEN + 1];
			strlcpy(char_name, c_pData, sizeof(char_name));

			if (!MessengerManager::Instance().IsBlocked(ch->GetName(), char_name))
				return CHARACTER_NAME_MAX_LEN;

			MessengerManager::Instance().RemoveFromBlockList(ch->GetName(), char_name);
		}
		return CHARACTER_NAME_MAX_LEN;
#endif

		default:
			sys_err("CInputMain::Messenger : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

#ifdef ENABLE_BATTLE_PASS_SYSTEM
int CInputMain::ReciveExtBattlePassActions(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGExtBattlePassAction* p = (TPacketCGExtBattlePassAction*)data;

	if (uiBytes < sizeof(TPacketCGExtBattlePassAction))
		return -1;

	const char* c_pData = data + sizeof(TPacketCGExtBattlePassAction);
	uiBytes -= sizeof(TPacketCGExtBattlePassAction);

	switch (p->bAction)
	{
		case 1:
			CBattlePassManager::instance().BattlePassRequestOpen(ch);
			return 0;

		case 2:
			if(get_dword_time() < ch->GetLastReciveExtBattlePassOpenRanking()) {
				ch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_NEXT_REFRESH_RANKLIST_TIME %d", ((ch->GetLastReciveExtBattlePassOpenRanking() - get_dword_time()) / 1000) + 1 );
				return 0;
			}
			ch->SetLastReciveExtBattlePassOpenRanking(get_dword_time() + 10000);
			
			for (uint8_t bBattlePassType = 1; bBattlePassType <= 3 ; ++bBattlePassType)
			{
				uint8_t bBattlePassID;
				if (bBattlePassType == 1)
					bBattlePassID = CBattlePassManager::instance().GetNormalBattlePassID();
				if (bBattlePassType == 2){
					bBattlePassID = CBattlePassManager::instance().GetPremiumBattlePassID();
					if (bBattlePassID != ch->GetExtBattlePassPremiumID())
						continue;
				}
				if (bBattlePassType == 3)
					bBattlePassID = CBattlePassManager::instance().GetEventBattlePassID();

				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT player_name, battlepass_type+0, battlepass_id, UNIX_TIMESTAMP(start_time), UNIX_TIMESTAMP(end_time) FROM player.battlepass_playerindex WHERE battlepass_type = %d and battlepass_id = %d and battlepass_completed = 1 and not player_name LIKE '[%%' ORDER BY (UNIX_TIMESTAMP(end_time)-UNIX_TIMESTAMP(start_time)) ASC LIMIT 40", bBattlePassType, bBattlePassID));
				if (pMsg->uiSQLErrno)
					return 0;

				MYSQL_ROW row;

				while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
				{
					TPacketGCExtBattlePassRanking pack;
					pack.bHeader = HEADER_GC_EXT_BATTLE_PASS_SEND_RANKING;
					strlcpy(pack.szPlayerName, row[0], sizeof(pack.szPlayerName));
					pack.bBattlePassType = std::atoi(row[1]);
					pack.bBattlePassID = atoll(row[2]);
					pack.dwStartTime = atoll(row[3]);
					pack.dwEndTime = atoll(row[4]);

					ch->GetDesc()->Packet(&pack, sizeof(pack));
				}
			}
			break;

		case 10:
			CBattlePassManager::instance().BattlePassRequestReward(ch, 1);
			return 0;
			
		case 11:
			CBattlePassManager::instance().BattlePassRequestReward(ch, 2);
			return 0;
			
		case 12:
			CBattlePassManager::instance().BattlePassRequestReward(ch, 3);
			return 0;


		default:
			break;
	}

	return 0;
}

int CInputMain::ReciveExtBattlePassPremium(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	TPacketCGExtBattlePassSendPremium* p = (TPacketCGExtBattlePassSendPremium*)data;

	if (uiBytes < sizeof(TPacketCGExtBattlePassSendPremium))
		return -1;

	const char* c_pData = data + sizeof(TPacketCGExtBattlePassSendPremium);
	uiBytes -= sizeof(TPacketCGExtBattlePassSendPremium);

	if (p->premium && (ch->IsGM() || ch->IsVIP()))
	{
		ch->PointChange(POINT_BATTLE_PASS_PREMIUM_ID, CBattlePassManager::instance().GetPremiumBattlePassID());
		CBattlePassManager::instance().BattlePassRequestOpen(ch);
		ch->ChatPacket(CHAT_TYPE_INFO, "BATTLEPASS_NOW_IS_ACTIVATED_PREMIUM_BATTLEPASS_OWN");
	}
	return 0;
}
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
int CInputMain::Shop(LPCHARACTER ch, const char* data)
#else
int CInputMain::Shop(LPCHARACTER ch, const char* data, size_t uiBytes)
#endif
{
	if (!ch)
		return 0;

	const TPacketCGShop* p = (TPacketCGShop*)data;

#ifndef ENABLE_SPECIAL_INVENTORY
	if (uiBytes < sizeof(TPacketCGShop))
		return -1;
#endif

	if (test_server)
		sys_log(0, "CInputMain::Shop() ==> SubHeader %d", p->subheader);

#ifndef ENABLE_SPECIAL_INVENTORY
	const char* c_pData = data + sizeof(TPacketCGShop);
	uiBytes -= sizeof(TPacketCGShop);
#endif

	switch (p->subheader)
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		case SHOP_SUBHEADER_CG_END:
		{
			sys_log(1, "INPUT: %s SHOP: END", ch->GetName());
			CShopManager::Instance().StopShopping(ch);
		}
		break;

		case SHOP_SUBHEADER_CG_BUY:
		{
			sys_log(1, "INPUT: %s SHOP: BUY %d", ch->GetName(), static_cast<uint8_t>(p->wPos));
			CShopManager::Instance().Buy(ch, static_cast<uint8_t>(p->wPos));
		}
		break;

		case SHOP_SUBHEADER_CG_SELL:
		{
			sys_log(0, "INPUT: %s pos %d SHOP: SELL", ch->GetName(), p->wPos);
			CShopManager::Instance().Sell(ch, p->wPos);
		}
		break;

		case SHOP_SUBHEADER_CG_SELL2:
		{
			sys_log(0, "INPUT: %s pos %d count %d SHOP: SELL2", ch->GetName(), p->wPos, p->bCount);
			CShopManager::Instance().Sell(ch, p->wPos, p->bCount);
		}
		break;
#else
		case SHOP_SUBHEADER_CG_END:
			sys_log(1, "INPUT: %s SHOP: END", ch->GetName());
			CShopManager::Instance().StopShopping(ch);
			return 0;

		case SHOP_SUBHEADER_CG_BUY:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint8_t))
					return -1;

				uint8_t bPos = *(c_pData + 1);
				sys_log(1, "INPUT: %s SHOP: BUY %d", ch->GetName(), bPos);
				CShopManager::Instance().Buy(ch, bPos);
				return (sizeof(uint8_t) + sizeof(uint8_t));
			}

		case SHOP_SUBHEADER_CG_SELL:
			{
				if (uiBytes < sizeof(uint8_t))
					return -1;

				uint8_t pos = *c_pData;

				sys_log(0, "INPUT: %s SHOP: SELL", ch->GetName());
				CShopManager::Instance().Sell(ch, pos);
				return sizeof(uint8_t);
			}

		case SHOP_SUBHEADER_CG_SELL2:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint8_t))
					return -1;

				uint8_t pos = *(c_pData++);
				uint8_t count = *(c_pData);

				sys_log(0, "INPUT: %s SHOP: SELL2", ch->GetName());

				CShopManager::Instance().Sell(ch, pos, count);
				return sizeof(uint8_t) + sizeof(uint8_t);
			}
#endif

		default:
			sys_err("CInputMain::Shop : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

void CInputMain::OnClick(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const struct command_on_click* pinfo = (struct command_on_click*)data;
	LPCHARACTER victim = CHARACTER_MANAGER::Instance().Find(pinfo->vid);
	if (!victim)
	{
		sys_err("CInputMain::OnClick %s.Click.NOT_EXIST_VID[%d]", ch->GetName(), pinfo->vid);
		return;
	}

	victim->OnClick(ch);
}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
int CInputMain::GemShop(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (!ch)
		return 0;

	const TPacketCGGemShop* p = (TPacketCGGemShop*)data;

	if (uiBytes < sizeof(TPacketCGGemShop))
		return -1;

	if (test_server)
		sys_log(0, "CInputMain::GemShop() ==> SubHeader %d", p->subheader);

	const char* c_pData = data + sizeof(TPacketCGGemShop);
	uiBytes -= sizeof(TPacketCGGemShop);

	switch (p->subheader)
	{
		case GEM_SHOP_SUBHEADER_CG_BUY:
		{
			if (uiBytes < sizeof(uint8_t))
				return -1;

			const uint8_t bPos = *c_pData;

			sys_log(1, "INPUT: %s GEM_SHOP: BUY %d", ch->GetName(), bPos);
			ch->GemShopBuy(bPos);
			return sizeof(uint8_t);
		}

		case GEM_SHOP_SUBHEADER_CG_ADD:
		{
			//if (uiBytes < sizeof(uint8_t))
			//	return -1;

			//uint8_t bPos = *c_pData;

			sys_log(1, "INPUT: %s GEM_SHOP: ADD", ch->GetName());
			//sys_log(1, "INPUT: %s GEM_SHOP: ADD %d", ch->GetName(), bPos);
			ch->GemShopAdd();
			//return sizeof(uint8_t);
			return 0;
		}

		case GEM_SHOP_SUBHEADER_CG_REFRESH:
		{
			sys_log(1, "INPUT: %s GEM_SHOP: REFRESH", ch->GetName());
			ch->RefreshGemShopWithItem();
			return 0;
		}

		default:
			sys_err("CInputMain::GemShop() : Unknown subheader %d : %s", p->subheader, ch->GetName());
			break;
	}

	return 0;
}

void CInputMain::ScriptSelectItemEx(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	const TPacketCGScriptSelectItemEx* p = (TPacketCGScriptSelectItemEx*)c_pData;
	sys_log(0, "QUEST ScriptSelectItemEx pid %d answer %d", ch->GetPlayerID(), p->selection);
	quest::CQuestManager::Instance().SelectItemEx(ch->GetPlayerID(), p->selection);
}
#endif

void CInputMain::Exchange(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	TPacketCGExchange* pinfo = (TPacketCGExchange*)data;
	LPCHARACTER to_ch = nullptr;

	if (!ch->CanHandleItem())
		return;

	const int iPulse = thecore_pulse();

	if ((to_ch = CHARACTER_MANAGER::Instance().Find(pinfo->arg1)))
	{
		if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			to_ch->ChatPacket(CHAT_TYPE_INFO, "[LS;661;%d]", g_nPortalLimitTime);
			return;
		}

		if (true == to_ch->IsDead())
			return;
	}

	sys_log(0, "CInputMain()::Exchange() SubHeader %d ", pinfo->sub_header);

	if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;661;%d]", g_nPortalLimitTime);
		return;
	}


	switch (pinfo->sub_header)
	{
		case EXCHANGE_SUBHEADER_CG_START: // arg1 == vid of target character
		{
			if (!ch->GetExchange())
			{
				if ((to_ch = CHARACTER_MANAGER::Instance().Find(pinfo->arg1)))
				{
					//MONARCH_LIMIT
					/*
					if (to_ch->IsMonarch() || ch->IsMonarch())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¿Í´Â °Å·¡¸¦ ÇÒ¼ö°¡ ¾ø½À´Ï´Ù"), g_nPortalLimitTime);
						return;
					}
					//END_MONARCH_LIMIT
					*/
					if (iPulse - ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;662;%d]", g_nPortalLimitTime);

						if (test_server)
							ch->ChatPacket(CHAT_TYPE_INFO, "[Safebox]Pulse %d LoadTime %d PASS %d", iPulse, ch->GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
						return;
					}

					if (iPulse - to_ch->GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
					{
						to_ch->ChatPacket(CHAT_TYPE_INFO, "[LS;662;%d]", g_nPortalLimitTime);


						if (test_server)
							to_ch->ChatPacket(CHAT_TYPE_INFO, "[Safebox]Pulse %d LoadTime %d PASS %d", iPulse, to_ch->GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
						return;
					}

					if (ch->GetGold() >= GOLD_MAX)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;663]");

						sys_err("[OVERFLOG_GOLD] START (%u) id %u name %s ", ch->GetGold(), ch->GetPlayerID(), ch->GetName());
						return;
					}

					if (to_ch->IsPC())
					{
						if (quest::CQuestManager::Instance().GiveItemToPC(ch->GetPlayerID(), to_ch))
						{
							sys_log(0, "Exchange canceled by quest %s %s", ch->GetName(), to_ch->GetName());
							return;
						}
					}

					if (
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
						ch->GetViewingShopOwner() ||
#else
						ch->GetMyShop() || ch->GetShopOwner() || 
#endif
						ch->IsOpenSafebox() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
						|| ch->IsOpenGuildstorage()
#endif
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
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1115]");
						return;
					}

#ifdef ENABLE_BATTLE_FIELD
					if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_ZONE_CANT_EXCHANGE"));
						return;
					}
#endif

					ch->ExchangeStart(to_ch);
				}
			}
		}
		break;

		case EXCHANGE_SUBHEADER_CG_ITEM_ADD: // arg1 == position of item, arg2 == position in exchange window
		{
			if (ch->GetExchange())
			{
				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->AddItem(pinfo->Pos, pinfo->arg2);
			}
		}
		break;

		case EXCHANGE_SUBHEADER_CG_ITEM_DEL: // arg1 == position of item
		{
			if (ch->GetExchange())
			{
				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
					ch->GetExchange()->RemoveItem(pinfo->arg1);
			}
		}
		break;

		case EXCHANGE_SUBHEADER_CG_ELK_ADD: // arg1 == amount of gold
		{
			if (ch->GetExchange())
			{
				const int64_t nTotalGold = static_cast<int64_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetGold()) + static_cast<int64_t>(pinfo->arg1);
#ifdef ENABLE_CHEQUE_SYSTEM
				const int64_t nTotalCheque = static_cast<int64_t>(ch->GetExchange()->GetCompany()->GetOwner()->GetCheque()) + static_cast<int64_t>(pinfo->cheque);
#endif

				if (GOLD_MAX <= nTotalGold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;665]");

					sys_err("[OVERFLOW_GOLD] ELK_ADD (%u) id %u name %s ",
							ch->GetExchange()->GetCompany()->GetOwner()->GetGold(),
							ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
						   	ch->GetExchange()->GetCompany()->GetOwner()->GetName());

					return;
				}

#ifdef ENABLE_CHEQUE_SYSTEM
				if (CHEQUE_MAX <= nTotalCheque)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The trade can't be done because the other character exceeds %d won"), CHEQUE_MAX - 1);

					sys_err("[OVERFLOW_CHEQUE] CHEQUE_ADD (%u) id %u name %s ",
						ch->GetExchange()->GetCompany()->GetOwner()->GetCheque(),
						ch->GetExchange()->GetCompany()->GetOwner()->GetPlayerID(),
						ch->GetExchange()->GetCompany()->GetOwner()->GetName());

					return;
				}
#endif

				if (ch->GetExchange()->GetCompany()->GetAcceptStatus() != true)
				{
#ifdef ENABLE_CHEQUE_SYSTEM
					ch->GetExchange()->AddGold(pinfo->arg1, pinfo->cheque);
#else
					ch->GetExchange()->AddGold(pinfo->arg1);
#endif
				}
			}
		}
		break;

		case EXCHANGE_SUBHEADER_CG_ACCEPT: // arg1 == not used
		{
			if (ch->GetExchange())
			{
				sys_log(0, "CInputMain()::Exchange() ==> ACCEPT ");
				ch->GetExchange()->Accept(true);
			}
		}
		break;

		case EXCHANGE_SUBHEADER_CG_CANCEL: // arg1 == not used
		{
			if (ch->GetExchange())
				ch->GetExchange()->Cancel();
		}
		break;

		default:
			break;
	}
}

void CInputMain::Position(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const struct command_position* pinfo = (struct command_position*)data;

	switch (pinfo->position)
	{
		case POSITION_GENERAL:
			ch->Standup();
			break;

		case POSITION_SITTING_CHAIR:
			ch->Sitdown(0);
			break;

		case POSITION_SITTING_GROUND:
			ch->Sitdown(1);
			break;

		default:
			break;
	}
}

static const int ComboSequenceBySkillLevel[3][8] =
{
	// 0 1 2 3 4 5 6 7
	{ 14, 15, 16, 17, 0, 0, 0, 0 },
	{ 14, 15, 16, 18, 20, 0, 0, 0 },
	{ 14, 15, 16, 18, 19, 17, 0, 0 },
};

bool CheckComboHack(LPCHARACTER ch, uint8_t bArg, uint32_t dwTime, bool CheckSpeedHack)
{
	if (!gHackCheckEnable)
		return false;

	/*
	You cannot attack while dead or stunned, so skip.
	Don't do this, in CHRACTER::CanMove()
	if (IsStun() || IsDead())
		return false;
	I think it's right to add
	Already in other parts CanMove() is used with IsStun(), IsDead()
	Because it is checked independently, the effect of correction
	In order to minimize it, I write the code like this.
	*/
	if (ch->IsStun() || ch->IsDead())
		return false;

	const int ComboInterval = dwTime - ch->GetLastComboTime();
	int HackScalar = 0; // Basic scalar unit 1
#if 0
	sys_log(0, "COMBO: %s arg:%u seq:%u delta:%d checkspeedhack:%d",
		ch->GetName(), bArg, ch->GetComboSequence(), ComboInterval - ch->GetValidComboInterval(), CheckSpeedHack);
#endif

#ifdef ENABLE_WOLFMAN_CHARACTER
	int COMBO_HACK_ALLOWABLE_MS;	//@fixme457
	if (ch->GetJob() == JOB_WOLFMAN)
		COMBO_HACK_ALLOWABLE_MS = 500;
	else
		COMBO_HACK_ALLOWABLE_MS = 100;
#else
	int COMBO_HACK_ALLOWABLE_MS = 100;
#endif

	// A total of 8 combos are possible from 14 to 21 bArg
	// 1. The first combo (14) can be repeated after a certain period of time.
	// 2. 15 ~ 21 times cannot be repeated
	// 3. It increases in sequence.
	if (bArg == 14)
	{
		if (CheckSpeedHack && ComboInterval > 0 && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
		{
			// The FIXME first combo can come strangely quickly, so divide it by 300 -_-;

			// If you attack while being downed by multiple monsters
			// The first combo comes in at very few intervals.
			// As a result, the following code is disabled because it may bounce as a combo hack.
			//HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 300;

			//sys_log(0, "COMBO_HACK: 2 %s arg:%u interval:%d valid:%u atkspd:%u riding:%s",
			// ch->GetName(),
			// bArg,
			// ComboInterval,
			// ch->GetValidComboInterval(),
			// ch->GetPoint(POINT_ATT_SPEED),
			// ch->IsRiding() ? "yes" : "no");
		}

		ch->SetComboSequence(1);
		ch->SetValidComboInterval((int)(ani_combo_speed(ch, 1) / (ch->GetPoint(POINT_ATT_SPEED) / 100.f)));
		ch->SetLastComboTime(dwTime);
	}
	else if (bArg > 14 && bArg < 22)
	{
		const int idx = MIN(2, ch->GetComboIndex());

		if (ch->GetComboSequence() > 5) // There are currently no more than 6 combos.
		{
			HackScalar = 1;
			ch->SetValidComboInterval(300);
			sys_log(0, "COMBO_HACK: 5 %s combo_seq:%d", ch->GetName(), ch->GetComboSequence());
		}
		// Assassin dual number combo exception handling
		else if (bArg == 21 &&
			idx == 2 &&
			ch->GetComboSequence() == 5 &&
			ch->GetJob() == JOB_ASSASSIN &&
			ch->GetWear(WEAR_WEAPON) &&
			ch->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
			ch->SetValidComboInterval(300);
#ifdef ENABLE_WOLFMAN_CHARACTER
		else if (bArg == 21 && idx == 2 && ch->GetComboSequence() == 5 && ch->GetJob() == JOB_WOLFMAN && ch->GetWear(WEAR_WEAPON) && ch->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_CLAW)
			ch->SetValidComboInterval(300);
#endif
		else if (ComboSequenceBySkillLevel[idx][ch->GetComboSequence()] != bArg)
		{
			HackScalar = 1;
			ch->SetValidComboInterval(300);

			sys_log(0, "COMBO_HACK: 3 %s arg:%u valid:%u combo_idx:%d combo_seq:%d",
				ch->GetName(),
				bArg,
				ComboSequenceBySkillLevel[idx][ch->GetComboSequence()],
				idx,
				ch->GetComboSequence());
		}
		else
		{
			if (CheckSpeedHack && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
			{
				HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 100;

				sys_log(0, "COMBO_HACK: 2 %s arg:%u interval:%d valid:%u atkspd:%u riding:%s",
					ch->GetName(),
					bArg,
					ComboInterval,
					ch->GetValidComboInterval(),
					ch->GetPoint(POINT_ATT_SPEED),
					ch->IsRiding() ? "yes" : "no");
			}

			// When riding a horse, repeat 15 to 16 times
			//if (ch->IsHorseRiding())
			if (ch->IsRiding())
				ch->SetComboSequence(ch->GetComboSequence() == 1 ? 2 : 1);
			else
				ch->SetComboSequence(ch->GetComboSequence() + 1);

			ch->SetValidComboInterval((int)(ani_combo_speed(ch, bArg - 13) / (ch->GetPoint(POINT_ATT_SPEED) / 100.f)));
			ch->SetLastComboTime(dwTime);
		}
	}
	else if (bArg == 13) // Basic Attack (Comes when Polymorph)
	{
		if (CheckSpeedHack && ComboInterval > 0 && ComboInterval < ch->GetValidComboInterval() - COMBO_HACK_ALLOWABLE_MS)
		{
			// If you attack while being down by multiple monsters
			// The first combo comes in at very few intervals.
			// This causes the next code to be deactivated as it may bounce as a combo hack.
			//HackScalar = 1 + (ch->GetValidComboInterval() - ComboInterval) / 100;

			//sys_log(0, "COMBO_HACK: 6 %s arg:%u interval:%d valid:%u atkspd:%u",
			// ch->GetName(),
			// bArg,
			// ComboInterval,
			// ch->GetValidComboInterval(),
			// ch->GetPoint(POINT_ATT_SPEED));
		}

		if (ch->GetRaceNum() >= MAIN_RACE_MAX_NUM)
		{
			// POLYMORPH_BUG_FIX

			// DELETEME
			/*
			const CMotion * pkMotion = CMotionManager::Instance().GetMotion(ch->GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK));

			if (!pkMotion)
				sys_err("cannot find motion by race %u", ch->GetRaceNum());
			else
			{
				// If this is a normal calculation, it should be multiplied by 1000.f, but the client
				// Multiply by 900.f as it allows blending of the next animation.
				int k = (int) (pkMotion->GetDuration() / ((float) ch->GetPoint(POINT_ATT_SPEED) / 100.f) * 900.f);
				ch->SetValidComboInterval(k);
				ch->SetLastComboTime(dwTime);
			}
			*/
			const float normalAttackDuration = CMotionManager::Instance().GetNormalAttackDuration(ch->GetRaceNum());
			const int k = (int)(normalAttackDuration / ((float)ch->GetPoint(POINT_ATT_SPEED) / 100.f) * 900.f);
			ch->SetValidComboInterval(k);
			ch->SetLastComboTime(dwTime);
			// END_OF_POLYMORPH_BUG_FIX
		}
		else
		{
			/*
			There is a nonsense combo. Possible hackers?
			if (ch->GetDesc()->DelayedDisconnect(number(2, 9)))
			{
				LogManager::Instance().HackLog("Hacker", ch);
				sys_log(0, "HACKER: %s arg %u", ch->GetName(), bArg);
			}

			With the code above, if you attack while unraveling the polymorph,
			Sometimes it is recognized as a nucleus.

			In detail,
			The server handled poly 0, but
			Before receiving that packet from the claret, attack the mob. <- That is, attack while being a mob.

			Then, the clar sends a command to the server that it attacked in a mob state (arg == 13)

			On the server, race is human, but the attack type is a mob! He said he had a nuclear check.

			In fact, the attack pattern is not judged by the client and sent,
			The server would have to judge... why did he do this?
			by rtsummit
			*/
		}
	}
	else
	{
		// There is a nonsense combo. Possible hackers?
		if (ch->GetDesc()->DelayedDisconnect(number(2, 9)))
		{
			LogManager::Instance().HackLog("Hacker", ch);
			sys_log(0, "HACKER: %s arg %u", ch->GetName(), bArg);
		}

		HackScalar = 10;
		ch->SetValidComboInterval(300);
	}

	if (HackScalar > 0)	//@fixme429
	{
		// When riding or landing on a horse, an attack for 1.5 seconds is not considered nuclear, but has no attack power.
		if (get_dword_time() - ch->GetLastMountTime() > 1500)
			ch->IncreaseComboHackCount(1 + HackScalar);

		ch->SkipComboAttackByTime(ch->GetValidComboInterval());
	}

	return HackScalar != 0;	//@fixme429
}

void CInputMain::Move(LPCHARACTER ch, const char* data)
{
	if (!ch->CanMove())
		return;

	const struct command_move* pinfo = (struct command_move*)data;

	if (pinfo->bFunc >= FUNC_MAX_NUM && !(pinfo->bFunc & 0x80))
	{
		sys_err("invalid move type: %s", ch->GetName());
		return;
	}

#ifdef ENABLE_CHECK_GHOSTMODE
	if (ch->IsPC() && ch->IsDead())
	{
		if (ch->GetGhostmodeCount() <= 10)
		{
			ch->AddGhostmodeCount();
		}
		else
		{
			LPDESC d = ch->GetDesc();
			if (d)
			{
				if (d->DelayedDisconnect(3))
				{
					LogManager::Instance().HackLog("Ghostmode", ch);
					ch->ResetGhostmodeCount();
				}
			}
			return;
		}
	}
	else
	{
		ch->ResetGhostmodeCount();
	}
#endif
#ifdef ENABLE_CHECK_WALLHACK
	if (ch->IsPC())
	{
		if (!SECTREE_MANAGER::Instance().IsMovablePosition(ch->GetMapIndex(), pinfo->lX, pinfo->lY))
		{
			if (ch->GetWallhackCount() <= 5)
			{
				ch->AddWallhackCount();
			}
			else
			{
				LPDESC d = ch->GetDesc();
				if (d)
				{
					if (d->DelayedDisconnect(3))
					{
						LogManager::Instance().HackLog("Wallhack", ch);
						ch->ResetWallhackCount();
					}
				}
				return;
			}
		}
		else
		{
			ch->ResetWallhackCount();
		}
	}
#endif

	/*enum EMoveFuncType
	{
		FUNC_WAIT,
		FUNC_MOVE,
		FUNC_ATTACK,
		FUNC_COMBO,
		FUNC_MOB_SKILL,
		_FUNC_SKILL,
		FUNC_MAX_NUM,
		FUNC_SKILL = 0x80,
	};*/

	// Teleport hack check

	// if (!test_server)
#ifdef ENABLE_AUTO_SYSTEM
	if (!ch->IsAffectFlag(AFF_AUTO_USE))
#endif
	{
		const float fDist = DISTANCE_SQRT((ch->GetX() - pinfo->lX) / 100, (ch->GetY() - pinfo->lY) / 100);
		// @fixme106 (changed 40 to 80)
		if (((false == ch->IsRiding() && fDist > 25) || fDist > 80) && MAP_OXEVENT != ch->GetMapIndex())
		{
#ifdef ENABLE_HACK_TELEPORT_LOG // @warme006
			{
				const PIXEL_POSITION& warpPos = ch->GetWarpPosition();

				if (warpPos.x == 0 && warpPos.y == 0)
					LogManager::Instance().HackLog("Teleport", ch); // May be inaccurate
			}
#endif
			sys_log(0, "MOVE: %s trying to move too far (dist: %.1fm) Riding(%d)", ch->GetName(), fDist, ch->IsRiding());

			ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
			ch->Stop();
			return;
		}
#ifdef ENABLE_CHECK_GHOSTMODE
		if (ch->IsPC() && ch->IsDead())
		{
			sys_log(0, "MOVE: %s trying to move as dead", ch->GetName());

			ch->Show(ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ());
			ch->Stop();
			return;
		}
#endif
		//
		// Speed Hack (SPEEDHACK) Check
		//
		const uint32_t dwCurTime = get_dword_time();
		// Sync the time and check from 7 seconds later. (Before 20090702, it was 5 seconds)
		const bool CheckSpeedHack = (false == ch->GetDesc()->IsHandshaking() && dwCurTime - ch->GetDesc()->GetClientTime() > 7000);

		if (CheckSpeedHack)
		{
			int iDelta = (int)(pinfo->dwTime - ch->GetDesc()->GetClientTime());
			const int iServerDelta = (int)(dwCurTime - ch->GetDesc()->GetClientTime());

			iDelta = (int)(dwCurTime - pinfo->dwTime);

			// Time is running late. First, log only. You have to check if there are really many people like this. TODO
			if (iDelta >= 30000)
			{
				sys_log(0, "SPEEDHACK: slow timer name %s delta %d", ch->GetName(), iDelta);
				ch->GetDesc()->DelayedDisconnect(3);
			}
			// I understand that it goes 20msec faster in 1 second.
			else if (iDelta < -(iServerDelta / 50))
			{
				sys_log(0, "SPEEDHACK: DETECTED! %s (delta %d %d)", ch->GetName(), iDelta, iServerDelta);
				ch->GetDesc()->DelayedDisconnect(3);
			}
		}

		//
		// Combo hack and speed hack check
		//
		if (pinfo->bFunc == FUNC_COMBO && g_bCheckMultiHack)
		{
			CheckComboHack(ch, pinfo->bArg, pinfo->dwTime, CheckSpeedHack); // Combo check
		}
	}

	if (pinfo->bFunc == FUNC_MOVE)
	{
		if (ch->GetLimitPoint(POINT_MOV_SPEED) == 0)
			return;

		ch->SetRotation(static_cast<float>(pinfo->bRot * 5)); // Duplicate code
		ch->ResetStopTime(); // ""

		ch->Goto(pinfo->lX, pinfo->lY);

#ifdef ENABLE_AFK_MODE_SYSTEM
		if (ch->IsAway()) {
			ch->SetAway(false);
			if (ch->IsAffectFlag(AFF_AFK))
				ch->RemoveAffect(AFFECT_AFK);
		}
#endif
	}
	else
	{
		if (pinfo->bFunc == FUNC_ATTACK || pinfo->bFunc == FUNC_COMBO)
		{
			ch->OnMove(true);
		}
		else if (pinfo->bFunc & FUNC_SKILL)
		{
			const int MASK_SKILL_MOTION = 0x7F;
			const uint32_t motion = pinfo->bFunc & MASK_SKILL_MOTION;

			if (!ch->IsUsableSkillMotion(motion))
			{
				const char* name = ch->GetName();
				const uint32_t job = ch->GetJob();
				const uint32_t group = ch->GetSkillGroup();

				char szBuf[256];
				snprintf(szBuf, sizeof(szBuf), "SKILL_HACK: name=%s, job=%d, group=%d, motion=%d", name, job, group, motion);
				LogManager::Instance().HackLog(szBuf, ch->GetDesc()->GetAccountTable().login, ch->GetName(), ch->GetDesc()->GetHostName());
				sys_log(0, "%s", szBuf);

				if (test_server)
				{
					ch->GetDesc()->DelayedDisconnect(number(2, 8));
					ch->ChatPacket(CHAT_TYPE_INFO, szBuf);
				}
				else
				{
					ch->GetDesc()->DelayedDisconnect(number(150, 500));
				}
			}

			ch->OnMove();
		}

		ch->SetRotation(static_cast<float>(pinfo->bRot * 5)); // Duplicate code
		ch->ResetStopTime(); // ""

		ch->Move(pinfo->lX, pinfo->lY);
		ch->Stop();
		ch->StopStaminaConsume();

#ifdef ENABLE_AFK_MODE_SYSTEM
		if (ch->IsAway()) {
			ch->SetAway(false);
			if (ch->IsAffectFlag(AFF_AFK))
				ch->RemoveAffect(AFFECT_AFK);
		}
#endif
	}

	TPacketGCMove pack{};
	pack.bHeader = HEADER_GC_MOVE;
	pack.bFunc = pinfo->bFunc;
	pack.bArg = pinfo->bArg;
	pack.bRot = pinfo->bRot;
	pack.dwVID = ch->GetVID();
	pack.lX = pinfo->lX;
	pack.lY = pinfo->lY;
	pack.dwTime = pinfo->dwTime;
	pack.dwDuration = (pinfo->bFunc == FUNC_MOVE) ? ch->GetCurrentMoveDuration() : 0;

	ch->PacketAround(&pack, sizeof(TPacketGCMove), ch);
	/*
	if (pinfo->dwTime == 10653691) // Debugger found
	{
		if (ch->GetDesc()->DelayedDisconnect(number(15, 30)))
			LogManager::Instance().HackLog("Debugger", ch);
	}
	else if (pinfo->dwTime == 10653971) // Softice discovery
	{
		if (ch->GetDesc()->DelayedDisconnect(number(15, 30)))
			LogManager::Instance().HackLog("Softice", ch);
	}
	*/
	/*
	sys_log(0,
	"MOVE: %s Func:%u Arg:%u Pos:%dx%d Time:%u Dist:%.1f",
	ch->GetName(),
	pinfo->bFunc,
	pinfo->bArg,
	pinfo->lX / 100,
	pinfo->lY / 100,
	pinfo->dwTime,
	fDist);
	*/
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CInputMain::SetSkillColor(LPCHARACTER ch, const char* pcData)
{
	TPacketCGSkillColor* p = (TPacketCGSkillColor*)pcData;

	if (p->skill >= ESkillColorLength::MAX_SKILL_COUNT)
		return;

	uint32_t data[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
	memcpy(data, ch->GetSkillColor(), sizeof(data));

	data[p->skill][0] = p->col1;
	data[p->skill][1] = p->col2;
	data[p->skill][2] = p->col3;
	data[p->skill][3] = p->col4;
	data[p->skill][4] = p->col5;

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have changed the color of your skill."));

	ch->SetSkillColor(data[0]);

	TSkillColor db_pack;
	memcpy(db_pack.dwSkillColor, data, sizeof(data));
	db_pack.player_id = ch->GetPlayerID();
	db_clientdesc->DBPacketHeader(HEADER_GD_SKILL_COLOR_SAVE, 0, sizeof(TSkillColor));
	db_clientdesc->Packet(&db_pack, sizeof(TSkillColor));
}
#endif

void CInputMain::Attack(LPCHARACTER ch, const uint8_t header, const char* data)
{
	if (nullptr == ch)
		return;

	struct type_identifier
	{
		uint8_t header;
		uint8_t type;
	};

	const struct type_identifier* const type = reinterpret_cast<const struct type_identifier*>(data);

	if (type->type > 0)
	{
		if (false == ch->CanUseSkill(type->type))
		{
			return;
		}

		switch (type->type)
		{
			case SKILL_GEOMPUNG:
			case SKILL_SANGONG:
			case SKILL_YEONSA:
			case SKILL_KWANKYEOK:
			case SKILL_HWAJO:
			case SKILL_GIGUNG:
			case SKILL_PABEOB:
			case SKILL_MARYUNG:
			case SKILL_TUSOK:
			case SKILL_MAHWAN:
			case SKILL_BIPABU:
			case SKILL_NOEJEON:
			case SKILL_CHAIN:
#ifdef ENABLE_NINETH_SKILL
			case SKILL_ILGWANGPYO: // 177 ninja job 1
			case SKILL_PUNGLOEPO: // 178 sura job 2
			case SKILL_MABEOBAGGWI: // 180 sura job 2
			case SKILL_METEO: // 181 shaman job 1
#endif
			case SKILL_HORSE_WILDATTACK_RANGE:
				if (HEADER_CG_SHOOT != type->header)
				{
					if (test_server)
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Attack :name[%s] Vnum[%d] can't use skill by attack(warning)"), type->type);
					return;
				}
				break;

			default:
				break;
		}
	}

	switch (header)
	{
		case HEADER_CG_ATTACK:
		{
			if (nullptr == ch->GetDesc())
				return;

			const TPacketCGAttack* const packMelee = reinterpret_cast<const TPacketCGAttack*>(data);

			ch->GetDesc()->AssembleCRCMagicCube(packMelee->bCRCMagicCubeProcPiece, packMelee->bCRCMagicCubeFilePiece);

			LPCHARACTER victim = CHARACTER_MANAGER::Instance().Find(packMelee->dwVID);

			if (nullptr == victim || ch == victim)
				return;

			switch (victim->GetCharType())
			{
				case CHAR_TYPE_NPC:
				case CHAR_TYPE_WARP:
				case CHAR_TYPE_GOTO:
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
				case CHAR_TYPE_SHOP:
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
				case CHAR_TYPE_HORSE:
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
				case CHAR_TYPE_PET:
#endif
					return;

				default:
					break;
			}

			//@fixme544
			// If the attack type is Melee
			if (packMelee->bType == 0)
			{
				// Get the difference in meter between the attacker and the victim
				// A value of 1.0f is considered a point blank hit
				const float fHittingDistance = DISTANCE_SQRT((victim->GetX() - ch->GetX()) / 100, (victim->GetY() - ch->GetY()) / 100);
				// The maximum possible distance to avoid false positive is 7.0m
				// Two hands or one handed sword with Aura doesn't allow this range.
				static const float fMaximumHittingDistance = 7.0f;
				// We got one - Abort and then Log / Disconnect / Ban / Whatever?
				if (fHittingDistance > fMaximumHittingDistance)
				{
					LogManager::instance().HackLog("LONG_DISTANCE_ATTACK", ch);
					return;
				}
			}
			//@end_fixme544

			if (packMelee->bType > 0)
			{
				if (false == ch->CheckSkillHitCount(packMelee->bType, victim->GetVID()))
					return;
			}

			ch->Attack(victim, packMelee->bType);
		}
		break;

		case HEADER_CG_SHOOT:
		{
			const TPacketCGShoot* const packShoot = reinterpret_cast<const TPacketCGShoot*>(data);

			ch->Shoot(packShoot->bType);
		}
		break;

		default:
			break;
	}
}

int CInputMain::SyncPosition(LPCHARACTER ch, const char* c_pcData, size_t uiBytes)
{
	const TPacketCGSyncPosition* pinfo = reinterpret_cast<const TPacketCGSyncPosition*>(c_pcData);

	if (uiBytes < pinfo->wSize)
		return -1;

	const int iExtraLen = pinfo->wSize - sizeof(TPacketCGSyncPosition);
	if (iExtraLen < 0)
	{
		sys_err("invalid packet length (len %d size %u buffer %u)", iExtraLen, pinfo->wSize, uiBytes);
		ch->GetDesc()->SetPhase(PHASE_CLOSE);
		return -1;
	}

	if (0 != (iExtraLen % sizeof(TPacketCGSyncPositionElement)))
	{
		sys_err("invalid packet length %d (name: %s)", pinfo->wSize, ch->GetName());
		return iExtraLen;
	}

	int iCount = iExtraLen / sizeof(TPacketCGSyncPositionElement);

	if (iCount <= 0)
		return iExtraLen;

	static constexpr int nCountLimit = 16;
	if (iCount > nCountLimit)
	{
		//LogManager::Instance().HackLog( "SYNC_POSITION_HACK", ch );
		sys_err("Too many SyncPosition Count(%d) from Name(%s)", iCount, ch->GetName());
		//ch->GetDesc()->SetPhase(PHASE_CLOSE);
		//return -1;
		iCount = nCountLimit;
	}

	TEMP_BUFFER tbuf;
	LPBUFFER lpBuf = tbuf.getptr();

	TPacketGCSyncPosition* pHeader = (TPacketGCSyncPosition*)buffer_write_peek(lpBuf);
	buffer_write_proceed(lpBuf, sizeof(TPacketGCSyncPosition));

	const TPacketGCSyncPositionElement* e =
		reinterpret_cast<const TPacketGCSyncPositionElement*>(c_pcData + sizeof(TPacketCGSyncPosition));

	timeval tvCurTime;
	gettimeofday(&tvCurTime, nullptr);

	for (int i = 0; i < iCount; ++i, ++e)
	{
		LPCHARACTER victim = CHARACTER_MANAGER::Instance().Find(e->dwVID);

		if (!victim)
			continue;

		switch (victim->GetCharType())
		{
			case CHAR_TYPE_NPC:
			case CHAR_TYPE_WARP:
			case CHAR_TYPE_GOTO:
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			case CHAR_TYPE_HORSE:
#endif
#ifdef ENABLE_PROTO_RENEWAL
			case CHAR_TYPE_PET_PAY:
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			case CHAR_TYPE_PET:
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			case CHAR_TYPE_SHOP:
#endif
				continue;

			default:
				break;
		}

		// Ownership check
		if (!victim->SetSyncOwner(ch))
			continue;

		const float fDistWithSyncOwner = DISTANCE_SQRT((victim->GetX() - ch->GetX()) / 100, (victim->GetY() - ch->GetY()) / 100);
		static constexpr float fLimitDistWithSyncOwner = 2500.f + 1000.f;
		// If the distance to the victim is more than 2500 + a, it is considered a nucleus.
		// distance reference: client's __GetSkillTargetRange, __GetBowRange functions
		// 2500: The range of the skill with the longest range in the skill proto, or the range of the bow
		// a = POINT_BOW_DISTANCE value... but I don't know if it is actually used. There are no items, potions, skills, or quests...
		// Even so, in case you want to use it as a buffer, set it to 1000.f...
		if (fDistWithSyncOwner > fLimitDistWithSyncOwner)
		{
			// Look up to g_iSyncHackLimitCount.
			if (ch->GetSyncHackCount() < g_iSyncHackLimitCount)
			{
				ch->SetSyncHackCount(ch->GetSyncHackCount() + 1);
				continue;
			}
#ifdef ENABLE_AUTO_SYSTEM
			else if (!ch->IsAffectFlag(AFF_AUTO_USE))
#else
			else
#endif
			{
				LogManager::Instance().HackLog("SYNC_POSITION_HACK", ch);

				sys_err("Too far SyncPosition DistanceWithSyncOwner(%f)(%s) from Name(%s) CH(%d,%d) VICTIM(%d,%d) SYNC(%d,%d)",
					fDistWithSyncOwner, victim->GetName(), ch->GetName(), ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY(),
					e->lX, e->lY);

#ifndef ENABLE_SYNCPOSITION_DISCONNECT_FIX
				ch->GetDesc()->SetPhase(PHASE_CLOSE);

				return -1;
#endif
			}
		}

		const float fDist = DISTANCE_SQRT((victim->GetX() - e->lX) / 100, (victim->GetY() - e->lY) / 100);
		static const long g_lValidSyncInterval = 100 * 1000; // 100ms
		const timeval& tvLastSyncTime = victim->GetLastSyncTime();
		const timeval* tvDiff = timediff(&tvCurTime, &tvLastSyncTime);

		// To defend against nuclear sending other users to strange places by exploiting SyncPosition,
		// If the same user tries to SyncPosition again within g_lValidSyncInterval ms, it is considered a hack.
		if (tvDiff->tv_sec == 0 && tvDiff->tv_usec < g_lValidSyncInterval)
		{
			// Look up to g_iSyncHackLimitCount.
			if (ch->GetSyncHackCount() < g_iSyncHackLimitCount)
			{
				ch->SetSyncHackCount(ch->GetSyncHackCount() + 1);
				continue;
			}
#ifdef ENABLE_AUTO_SYSTEM
			else if (!ch->IsAffectFlag(AFF_AUTO_USE))
#else
			else
#endif
			{
				LogManager::Instance().HackLog("SYNC_POSITION_HACK", ch);

				sys_err("Too often SyncPosition Interval(%ldms)(%s) from Name(%s) VICTIM(%d,%d) SYNC(%d,%d)",
					tvDiff->tv_sec * 1000 + tvDiff->tv_usec / 1000, victim->GetName(), ch->GetName(), victim->GetX(), victim->GetY(),
					e->lX, e->lY);

#ifndef ENABLE_SYNCPOSITION_DISCONNECT_FIX
				ch->GetDesc()->SetPhase(PHASE_CLOSE);

				return -1;
#endif
			}
		}
		else if (fDist > 25.0f
#ifdef ENABLE_AUTO_SYSTEM
			&& !ch->IsAffectFlag(AFF_AUTO_USE)
#endif
			)
		{
			LogManager::Instance().HackLog("SYNC_POSITION_HACK", ch);

			sys_err("Too far SyncPosition Distance(%f)(%s) from Name(%s) CH(%d,%d) VICTIM(%d,%d) SYNC(%d,%d)",
				fDist, victim->GetName(), ch->GetName(), ch->GetX(), ch->GetY(), victim->GetX(), victim->GetY(),
				e->lX, e->lY);

#ifndef ENABLE_SYNCPOSITION_DISCONNECT_FIX
			ch->GetDesc()->SetPhase(PHASE_CLOSE);

			return -1;
#endif
		}
		else
		{
			victim->SetLastSyncTime(tvCurTime);
			victim->Sync(e->lX, e->lY);
			buffer_write(lpBuf, e, sizeof(TPacketGCSyncPositionElement));
		}
	}

	if (buffer_size(lpBuf) != sizeof(TPacketGCSyncPosition))
	{
		pHeader->bHeader = HEADER_GC_SYNC_POSITION;
		pHeader->wSize = static_cast<uint16_t>(buffer_size(lpBuf));

		ch->PacketAround(buffer_read_peek(lpBuf), buffer_size(lpBuf), ch);
	}

	return iExtraLen;
}

void CInputMain::FlyTarget(LPCHARACTER ch, const char* pcData, uint8_t bHeader)
{
	if (!ch)
		return;

	const TPacketCGFlyTargeting* p = (TPacketCGFlyTargeting*)pcData;
	ch->FlyTarget(p->dwTargetVID, p->x, p->y, bHeader);
}

void CInputMain::UseSkill(LPCHARACTER ch, const char* pcData)
{
	if (!ch)
		return;

	const TPacketCGUseSkill* p = (TPacketCGUseSkill*)pcData;
	ch->UseSkill(p->dwVnum, CHARACTER_MANAGER::Instance().Find(p->dwVID));
}

void CInputMain::ScriptButton(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	const TPacketCGScriptButton* p = (TPacketCGScriptButton*)c_pData;
	sys_log(0, "QUEST ScriptButton pid %d idx %u", ch->GetPlayerID(), p->idx);

	const quest::PC* pc = quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID());
	if (pc && pc->IsConfirmWait())
		quest::CQuestManager::Instance().Confirm(ch->GetPlayerID(), quest::CONFIRM_TIMEOUT);
	else if (p->idx & 0x80000000) // Click here in the quest window (__SelectQuest)
		quest::CQuestManager::Instance().QuestInfo(ch->GetPlayerID(), p->idx & 0x7fffffff);
	else
		quest::CQuestManager::Instance().QuestButton(ch->GetPlayerID(), p->idx);
}

void CInputMain::ScriptAnswer(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	const TPacketCGScriptAnswer* p = (TPacketCGScriptAnswer*)c_pData;
	sys_log(0, "QUEST ScriptAnswer pid %d answer %d", ch->GetPlayerID(), p->answer);

	if (p->answer > 250) // If the packet came in response to the next button
		quest::CQuestManager::Instance().Resume(ch->GetPlayerID());
	else // In the case of a packet sent by selecting the select button
		quest::CQuestManager::Instance().Select(ch->GetPlayerID(), p->answer);
}

// SCRIPT_SELECT_ITEM
void CInputMain::ScriptSelectItem(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	const TPacketCGScriptSelectItem* p = (TPacketCGScriptSelectItem*)c_pData;
	sys_log(0, "QUEST ScriptSelectItem pid %d answer %d", ch->GetPlayerID(), p->selection);
	quest::CQuestManager::Instance().SelectItem(ch->GetPlayerID(), p->selection);
}
// END_OF_SCRIPT_SELECT_ITEM

void CInputMain::QuestInputString(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	const TPacketCGQuestInputString* p = (TPacketCGQuestInputString*)c_pData;

	char msg[65];
	strlcpy(msg, p->msg, sizeof(msg));
	sys_log(0, "QUEST InputString pid %u msg %s", ch->GetPlayerID(), msg);

	quest::CQuestManager::Instance().Input(ch->GetPlayerID(), msg);
}

#ifdef ENABLE_OX_RENEWAL
void CInputMain::QuestInputLongString(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	const TPacketCGQuestInputLongString* p = (TPacketCGQuestInputLongString*)c_pData;

	char msg[129];
	strlcpy(msg, p->msg, sizeof(msg));
	sys_log(0, "QUEST InputLongString pid %u msg %s", ch->GetPlayerID(), msg);

	quest::CQuestManager::Instance().Input(ch->GetPlayerID(), msg);
}
#endif

void CInputMain::QuestConfirm(LPCHARACTER ch, const void* c_pData)
{
	if (!ch)
		return;

	TPacketCGQuestConfirm* p = (TPacketCGQuestConfirm*)c_pData;
	LPCHARACTER ch_wait = CHARACTER_MANAGER::Instance().FindByPID(p->requestPID);
	if (p->answer)
		p->answer = quest::CONFIRM_YES;

	sys_log(0, "QuestConfirm from %s pid %u name %s answer %d", ch->GetName(), p->requestPID, (ch_wait) ? ch_wait->GetName() : "", p->answer);

	if (ch_wait)
		quest::CQuestManager::Instance().Confirm(ch_wait->GetPlayerID(), (quest::EQuestConfirmType)p->answer, ch->GetPlayerID());
}

void CInputMain::Target(LPCHARACTER ch, const char* pcData)
{
	if (!ch)
		return;

	const TPacketCGTarget* p = (TPacketCGTarget*)pcData;

	const building::LPOBJECT pkObj = building::CManager::Instance().FindObjectByVID(p->dwVID);

	if (pkObj)
	{
		TPacketGCTarget pckTarget{};
		pckTarget.header = HEADER_GC_TARGET;
		pckTarget.dwVID = p->dwVID;
		ch->GetDesc()->Packet(&pckTarget, sizeof(TPacketGCTarget));
	}
	else
		ch->SetTarget(CHARACTER_MANAGER::Instance().Find(p->dwVID));
}

void CInputMain::Warp(LPCHARACTER ch, const char* pcData)
{
	if (ch)
		ch->WarpEnd();
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CInputMain::SafeboxCheckin(LPCHARACTER ch, const char* c_pData, int bMall)
#else
void CInputMain::SafeboxCheckin(LPCHARACTER ch, const char* c_pData)
#endif
{
	if (!ch)
		return;

#ifdef ENABLE_GM_BLOCK
	if (ch->IsGM() && !ch->GetStaffPermissions())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[GM] GM's k?nen keine Gegenst?de in das Lager geben!");
		return;
	}
#endif

	if (quest::CQuestManager::Instance().GetPCForce(ch->GetPlayerID())->IsRunning() == true)
		return;

	const TPacketCGSafeboxCheckin* p = (TPacketCGSafeboxCheckin*)c_pData;

	if (!ch->CanHandleItem())
		return;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	CSafebox* pkSafebox = bMall == 2 ? ch->GetGuildstorage() : ch->GetSafebox();
#else
	CSafebox* pkSafebox = ch->GetSafebox();
#endif
	
	LPITEM pkItem = ch->GetItem(p->ItemPos);

	if (!pkSafebox || !pkItem)
		return;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	if (pkItem->GetCell() >= ch->GetExtendInvenMax() && IS_SET(pkItem->GetFlag(), ITEM_FLAG_IRREMOVABLE))
#else
	if (pkItem->GetCell() >= INVENTORY_MAX_NUM && IS_SET(pkItem->GetFlag(), ITEM_FLAG_IRREMOVABLE))
#endif
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1060]");
		return;
	}

	if (pkItem->IsEquipped()) {	//@fixme416
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Du kannst getragene Gegenstaende nicht lagern."));
		return;
	}

	if (!pkSafebox->IsEmpty(p->bSafePos, pkItem->GetSize()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;666]");
		return;
	}

	if (pkItem->GetVnum() == UNIQUE_ITEM_SAFEBOX_EXPAND)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;667]");
		return;
	}

	if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_SAFEBOX))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;667]");
		return;
	}

	if (pkItem->isLocked())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[667][Storeroom] The item cannot be stored."));
		return;
	}

#ifdef ENABLE_SEALBIND_SYSTEM
	if (pkItem->IsSealed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't store soulbinded items."));
		return;
	}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	if (pkItem->IsBasicItem())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return;
	}
#endif

	if (pkItem->IsEquipped())
	{
		if (IS_SET(pkItem->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1061]");
			return;
		}

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		const int iWearCell = pkItem->FindEquipCell(ch);
		if (iWearCell == WEAR_WEAPON)
		{
			LPITEM costumeWeapon = ch->GetWear(WEAR_COSTUME_WEAPON);
			if (costumeWeapon && !ch->UnequipItem(costumeWeapon))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return;
			}
		}
#endif
	}

	// @fixme140 BEGIN
	if (ITEM_BELT == pkItem->GetType() && CBeltInventoryHelper::IsExistItemInBeltInventory(ch))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1095]");
		return;
	}
	// @fixme140 END

	pkItem->RemoveFromCharacter();
	if (!pkItem->IsDragonSoul())
		ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(p->ItemPos.cell), QUICKSLOT_MAX_POS);
	pkSafebox->Add(p->bSafePos, pkItem);

	char szHint[128];
	snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(), pkItem->GetCount());
	LogManager::Instance().ItemLog(ch, pkItem, "SAFEBOX PUT", szHint);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	if (bMall == 2)
		LogManager::Instance().GuildLog(ch, ch->GetGuild()->GetID(), pkItem->GetOriginalVnum(), pkItem->GetName(), GUILD_ITEM_TYPE_IN, pkItem->GetCount(), 0);
#endif
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CInputMain::SafeboxCheckout(LPCHARACTER ch, const char* c_pData, int bMall)
#else
void CInputMain::SafeboxCheckout(LPCHARACTER ch, const char* c_pData, bool bMall)
#endif
{
	if (!ch)
		return;

#ifdef ENABLE_GM_BLOCK
	if (ch->IsGM() && !ch->GetStaffPermissions())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[GM] GM's k?nen keine Gegenst?de aus dem Lager herrausnehmen!");
		return;
	}
#endif

	TPacketCGSafeboxCheckout* p = (TPacketCGSafeboxCheckout*)c_pData;

	if (!ch->CanHandleItem())
		return;

	CSafebox* pkSafebox;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	pkSafebox = bMall == 2 ? ch->GetGuildstorage() : (bMall == 1 ? ch->GetMall() : ch->GetSafebox());
#else
	if (bMall)
		pkSafebox = ch->GetMall();
	else
		pkSafebox = ch->GetSafebox();
#endif

	if (!pkSafebox)
		return;

	LPITEM pkItem = pkSafebox->Get(p->bSafePos);

	if (!pkItem)
		return;

#ifdef ENABLE_SAFEBOX_IMPROVING
	if ((p->ItemPos.window_type == INVENTORY) && (p->ItemPos.cell == 0))
	{
		TItemPos DestPos = p->ItemPos;
#	ifdef ENABLE_SPECIAL_INVENTORY
		const int iCell = pkItem->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(pkItem) : ch->GetEmptyInventory(pkItem);
#	else
		const int iCell = pkItem->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(pkItem) : ch->GetEmptyInventory(pkItem->GetSize());
#	endif
		if (iCell < 0)
			return;

		DestPos = TItemPos(pkItem->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iCell);
		pkSafebox->Remove(p->bSafePos);
#	ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
		pkItem->SetLastOwnerPID(ch->GetPlayerID());
#	endif
		pkItem->AddToCharacter(ch, DestPos);
		ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);
	}
	else
#endif
	{
		if (!ch->IsEmptyItemGrid(p->ItemPos, pkItem->GetSize()))
			return;

		// DragonSoulStone special treatment in the part that is transferred from Item Mall to Inventory
		// (Items made in the mall have attributes as defined in item_proto,
		// In the case of DragonSoulStone, if this treatment is not performed, none of the attributes will be attached.)
		if (pkItem->IsDragonSoul())
		{
			if (bMall)
				DSManager::Instance().DragonSoulItemInitialize(pkItem);

			if (DRAGON_SOUL_INVENTORY != p->ItemPos.window_type)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;666]");
				return;
			}

			TItemPos DestPos = p->ItemPos;
			if (!DSManager::Instance().IsValidCellForThisItem(pkItem, DestPos))
			{
				const int iCell = ch->GetEmptyDragonSoulInventory(pkItem);
				if (iCell < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;666]");
					return;
				}
				DestPos = TItemPos(DRAGON_SOUL_INVENTORY, iCell);
			}

			pkSafebox->Remove(p->bSafePos);
			pkItem->SetLastOwnerPID(ch->GetPlayerID());
			pkItem->AddToCharacter(ch, DestPos);
			ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
			if (bMall == 2) {
				CGuild* pGuild = ch->GetGuild();
				if (pGuild) {
					pGuild->SetLastCheckout(ch->GetName(), GS_ITEM);
					pGuild->SetLastCheckoutP2P(ch->GetName(), GS_ITEM);
					pGuild->SendGuildInfoPacket(ch);
					TPacketGGGuild p1{};
					TPacketGGGuildChat p2{};

					p1.bHeader = HEADER_GG_GUILD;
					p1.bSubHeader = GUILD_SUBHEADER_GG_REFRESH;
					p1.dwGuild = pGuild->GetID();
					strlcpy(p2.szText, "none", sizeof(p2.szText));

					P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
					P2P_MANAGER::Instance().Send(&p2, sizeof(TPacketGGGuildChat));
				}
			}
#endif
		}
		else
		{
			if (DRAGON_SOUL_INVENTORY == p->ItemPos.window_type)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;666]");
				return;
			}
			// @fixme119
			if (p->ItemPos.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(pkItem))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1097]");
				return;
			}

#ifdef ENABLE_SPECIAL_INVENTORY
			if (pkItem->GetSpecialInventoryType() != p->ItemPos.GetSpecialInventoryType())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SPECIAL_INVENTORY_CANNOT_PLACE_HERE"));
				return;
			}
#endif

			pkSafebox->Remove(p->bSafePos);
			pkItem->AddToCharacter(ch, p->ItemPos);
			ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
			if (bMall == 2) {
				CGuild* pGuild = ch->GetGuild();
				if (pGuild) {
					pGuild->SetLastCheckout(ch->GetName(), GS_ITEM);
					pGuild->SetLastCheckoutP2P(ch->GetName(), GS_ITEM);
					pGuild->SendGuildInfoPacket(ch);
					TPacketGGGuild p1{};
					TPacketGGGuildChat p2{};

					p1.bHeader = HEADER_GG_GUILD;
					p1.bSubHeader = GUILD_SUBHEADER_GG_REFRESH;
					p1.dwGuild = pGuild->GetID();
					strlcpy(p2.szText, "none", sizeof(p2.szText));

					P2P_MANAGER::Instance().Send(&p1, sizeof(TPacketGGGuild));
					P2P_MANAGER::Instance().Send(&p2, sizeof(TPacketGGGuildChat));
				}
			}
#endif
		}
	}

	const uint32_t dwID = pkItem->GetID();
	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_FLUSH, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&dwID, sizeof(uint32_t));

	char szHint[128];
	snprintf(szHint, sizeof(szHint), "%s %u", pkItem->GetName(), pkItem->GetCount());
	if (bMall)
		LogManager::Instance().ItemLog(ch, pkItem, "MALL GET", szHint);
	else
		LogManager::Instance().ItemLog(ch, pkItem, "SAFEBOX GET", szHint);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	if (bMall == 2)
		LogManager::Instance().GuildLog(ch, ch->GetGuild()->GetID(), pkItem->GetOriginalVnum(), pkItem->GetName(), GUILD_ITEM_TYPE_OUT, pkItem->GetCount(), 0);
#endif
}

#ifdef ENABLE_MOVE_CHANNEL
void CInputMain::MoveChannel(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGMoveChannel* p = reinterpret_cast<const TPacketCGMoveChannel*>(c_pData);
	if (p == nullptr)
		return;

	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ì·¨ì†Œ ?˜?ˆ?µë‹ˆ??"));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	const uint8_t bChannel = p->channel;

	if (bChannel == g_bChannel
		|| g_bAuthServer
		|| g_bChannel == 99
		|| ch->GetMapIndex() >= 1000
		|| ch->GetDungeon()
		|| ch->CanWarp() == false
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You cannot change channel.");
		return;
	}

	TMoveChannel t{ bChannel, ch->GetMapIndex() };
	db_clientdesc->DBPacket(HEADER_GD_MOVE_CHANNEL, ch->GetDesc()->GetHandle(), &t, sizeof(t));
}
#endif

void CInputMain::SafeboxItemMove(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const struct command_item_move* pinfo = (struct command_item_move*)data;

	if (!ch->CanHandleItem())
		return;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (pinfo->count == 0) {
		if (!ch->GetSafebox())
			return;

		ch->GetSafebox()->MoveItem(pinfo->Cell.cell, pinfo->CellTo.cell, pinfo->count);
	}
	else if (pinfo->count == 2) {
		if (!ch->GetGuildstorage())
			return;

		ch->GetGuildstorage()->MoveItem(pinfo->Cell.cell, pinfo->CellTo.cell, 0);
	}
#else
	if (!ch->GetSafebox())
		return;

	ch->GetSafebox()->MoveItem(static_cast<uint8_t>(pinfo->Cell.cell), static_cast<uint8_t>(pinfo->CellTo.cell), pinfo->count);
#endif
}

// PARTY_JOIN_BUG_FIX
void CInputMain::PartyInvite(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[403]You cannot use this in the duel arena."));
		return;
	}

	const TPacketCGPartyInvite* p = (TPacketCGPartyInvite*)c_pData;

	LPCHARACTER pInvitee = CHARACTER_MANAGER::Instance().Find(p->vid);

	if (!pInvitee || !ch->GetDesc() || !pInvitee->GetDesc())
	{
		sys_err("PARTY Cannot find invited character");
		return;
	}

#ifdef ENABLE_GM_BLOCK
	if ((ch->IsGM() && !ch->GetStaffPermissions()) && !pInvitee->IsGM())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[GM] GM's k?nen zu keiner Gruppe einladen!");
		return;
	}
	else if (!ch->IsGM() && pInvitee->IsGM())
	{
		//ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't invite GM to your party"));
		return;
	}
#endif

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[118]This action is not possible on this map."));
		return;
	}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	if (MessengerManager::Instance().IsBlocked(ch->GetName(), pInvitee->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[138]Unblock %s to continue."), pInvitee->GetName());
		return;
	}
	else if (MessengerManager::Instance().IsBlocked(pInvitee->GetName(), ch->GetName()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[139]%s has blocked you."), pInvitee->GetName());
		return;
	}
#endif

	ch->PartyInvite(pInvitee);
}

void CInputMain::PartyInviteAnswer(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[403]You cannot use this in the duel arena."));
		return;
	}

	const TPacketCGPartyInviteAnswer* p = (TPacketCGPartyInviteAnswer*)c_pData;

	LPCHARACTER pInviter = CHARACTER_MANAGER::Instance().Find(p->leader_vid);

	// pInviter made a party request to ch.
	if (!pInviter)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[668][Group] The player who invited you is not online."));
	else if (!p->accept)
		pInviter->PartyInviteDeny(ch->GetPlayerID());
	else
		pInviter->PartyInviteAccept(ch);
}
// END_OF_PARTY_JOIN_BUG_FIX

void CInputMain::PartySetState(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;530]");
		return;
	}

	const TPacketCGPartySetState* p = (TPacketCGPartySetState*)c_pData;

	if (!ch->GetParty())
		return;

	if (ch->GetParty()->GetLeaderPID() != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;671]");
		return;
	}

	if (!ch->GetParty()->IsMember(p->pid))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;672]");
		return;
	}

	const uint32_t pid = p->pid;
	sys_log(0, "PARTY SetRole pid %d to role %d state %s", pid, p->byRole, p->flag ? "on" : "off");

	switch (p->byRole)
	{
		case PARTY_ROLE_NORMAL:
			break;

		case PARTY_ROLE_ATTACKER:
		case PARTY_ROLE_TANKER:
		case PARTY_ROLE_BUFFER:
		case PARTY_ROLE_SKILL_MASTER:
		case PARTY_ROLE_HASTE:
		case PARTY_ROLE_DEFENDER:
			if (ch->GetParty()->SetRole(pid, p->byRole, p->flag))
			{
				TPacketPartyStateChange pack{};
				pack.dwLeaderPID = ch->GetPlayerID();
				pack.dwPID = p->pid;
				pack.bRole = p->byRole;
				pack.bFlag = p->flag;
				db_clientdesc->DBPacket(HEADER_GD_PARTY_STATE_CHANGE, 0, &pack, sizeof(pack));
			}
			/* else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Failed to set <Party> Attacker."));*/
			break;

		default:
			sys_err("wrong byRole in PartySetState Packet name %s state %d", ch->GetName(), p->byRole);
			break;
	}
}

void CInputMain::PartyRemove(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[403]You cannot use this in the duel arena."));
		return;
	}

	if (!CPartyManager::Instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[530][Group] The server cannot execute this group request."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[673][Group] You cannot kick out a player while you are in a dungeon."));
		return;
	}

#ifdef ENABLE_12ZI
	if (ch->GetZodiac() || (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex())))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Group] You cannot leave a group while you are in a Zodiac Temple."));
		return;
	}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	//if (ch->GetMeleyLair() || CMeleyLairManager::Instance().IsMeleyMap(ch->GetMapIndex()))
	if (ch->GetMeleyLair())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[531][Group] You cannot leave a group while you are in a dungeon."));
		return;
	}
#endif

	const TPacketCGPartyRemove* p = (TPacketCGPartyRemove*)c_pData;

	if (!ch->GetParty())
		return;

	LPPARTY pParty = ch->GetParty();
	if (!pParty)
		return;

	if (pParty->GetLeaderPID() == ch->GetPlayerID())
	{
		if (ch->GetDungeon())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[673][Group] You cannot kick out a player while you are in a dungeon."));
		}
		else
		{
			// Prevent the party leader from disbanding the party outside the dungeon at Red Dragon Castle (metin2_map_n_flame_dungeon_01)
			if (pParty->IsPartyInDungeon(MAP_N_FLAME_DUNGEON_01))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[531][Group] You cannot leave a group while you are in a dungeon."));
				return;
			}

#ifdef ENABLE_QUEEN_NETHIS
			if (SnakeLair::CSnk::instance().IsSnakeMap(ch->GetMapIndex()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[531][Group] You cannot leave a group while you are in a dungeon."));
				return;
			}
#endif

			// leader can remove any member
			if (p->pid == ch->GetPlayerID() || pParty->GetMemberCount() == 2)
			{
				// party disband
				CPartyManager::Instance().DeleteParty(pParty);
			}
			else
			{
				LPCHARACTER B = CHARACTER_MANAGER::Instance().FindByPID(p->pid);
				if (B)
				{
					//pParty->SendPartyRemoveOneToAll(B);
					B->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[675][Group] You have been out kicked of the group."));
					//pParty->Unlink(B);
					//CPartyManager::Instance().SetPartyMember(B->GetPlayerID(), nullptr);
				}
				pParty->Quit(p->pid);
			}
		}
	}
	else
	{
		// otherwise, only remove itself
		if (p->pid == ch->GetPlayerID())
		{
			if (ch->GetDungeon())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[676][Group] You cannot leave a group while you are in a dungeon."));
			}
			else
			{
#ifdef ENABLE_QUEEN_NETHIS
				if (SnakeLair::CSnk::instance().IsSnakeMap(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[676][Group] You cannot leave a group while you are in a dungeon."));
					return;
				}
#endif

				if (pParty->GetMemberCount() == 2)
				{
					// party disband
					CPartyManager::Instance().DeleteParty(pParty);
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[532][Group] You have left the group."));
					//pParty->SendPartyRemoveOneToAll(ch);
					pParty->Quit(ch->GetPlayerID());
					//pParty->SendPartyRemoveAllToOne(ch);
					//CPartyManager::Instance().SetPartyMember(ch->GetPlayerID(), nullptr);
				}
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[677][Group] You cannot kick out group members."));
		}
	}

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	LPITEM eqpdweapon = ch->GetWear(WEAR_WEAPON);
	if (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON)) {
//	if (eqpdweapon && eqpdweapon->GetVnum() == 149) {
//		ch->ChatPacket(7, "[PartyRemove] Vnum: 149 Equipped");
		/*ch->ChatPacket(7, "[PartyRemove] ITEM_FLAG_GROUP_WEAPON WORKS!");*/
		if (eqpdweapon->GetAttributeValue(5) >= 1) {
			eqpdweapon->SetForceAttribute(5, 0, 0);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[PartyRemove] You Group-Points set to %d"), 0);
		}
		/*else {
			ch->ChatPacket(7, "[PartyRemove] No Attributes on Vnum 149");
		}*/
	}
	/*else {
		ch->ChatPacket(7, "[PartyRemove] No Vnum 149 Equipped");
	}*/
#endif
}

void CInputMain::AnswerMakeGuild(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGAnswerMakeGuild* p = (TPacketCGAnswerMakeGuild*)c_pData;

	if (ch->GetGold() < 200000)
		return;

	if (ch->GetLevel() < 40)	//@fixme463
		return;

	if (get_global_time() - ch->GetQuestFlag("guild_manage.new_disband_time") <
		CGuildManager::Instance().GetDisbandDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[678][Guild] After disbanding a guild, you cannot create a new one for %d days."),
			quest::CQuestManager::Instance().GetEventFlag("guild_disband_delay"));
		return;
	}

	if (get_global_time() - ch->GetQuestFlag("guild_manage.new_withdraw_time") <
		CGuildManager::Instance().GetWithdrawDelay())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[679][Guild] After leaving a guild, you cannot create a new one for %d days."),
			quest::CQuestManager::Instance().GetEventFlag("guild_withdraw_delay"));
		return;
	}

	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::Instance();

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, p->guild_name, sizeof(cp.name));

	if (cp.name[0] == 0 || !check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[680]This guild name is invalid."));
		return;
	}

	const uint32_t dwGuildID = gm.CreateGuild(cp);

	if (dwGuildID)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[682][Guild] [%s] guild has been created."), cp.name);

		const int GuildCreateFee = 200000;

		ch->PointChange(POINT_GOLD, -GuildCreateFee);
		DBManager::Instance().SendMoneyLog(MONEY_LOG_GUILD, ch->GetPlayerID(), -GuildCreateFee);

		char Log[128];
		snprintf(Log, sizeof(Log), "GUILD_NAME %s MASTER %s", cp.name, ch->GetName());
		LogManager::Instance().CharLog(ch, 0, "MAKE_GUILD", Log);

		ch->RemoveSpecifyItem(GUILD_CREATE_ITEM_VNUM, 1);
		//ch->SendGuildName(dwGuildID);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[683][Guild] Creation of the guild has failed."));
}

void CInputMain::PartyUseSkill(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGPartyUseSkill* p = (TPacketCGPartyUseSkill*)c_pData;
	if (!ch->GetParty())
		return;

	if (ch->GetPlayerID() != ch->GetParty()->GetLeaderPID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[684][Group] Only the group leader can use group skills."));
		return;
	}

	switch (p->bySkillIndex)
	{
		case PARTY_SKILL_HEAL:
			ch->GetParty()->HealParty();
			break;
		case PARTY_SKILL_WARP:
			{
				LPCHARACTER pch = CHARACTER_MANAGER::Instance().Find(p->vid);
				if (pch)
					ch->GetParty()->SummonToLeader(pch->GetPlayerID());
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;685]");
			}
			break;

		default:
			break;
	}
}

#ifdef ENABLE_SAFEBOX_MONEY
void CInputMain::SafeboxMoney(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGSafeboxMoney* p = reinterpret_cast<const TPacketCGSafeboxMoney*>(c_pData);

	if (!ch->CanHandleItem())
		return;

	CSafebox* pSafebox = ch->GetMall();
	if (pSafebox)
		return;

	pSafebox = ch->GetSafebox();
	if (!pSafebox)
		return;

	if (p->dwMoney < 1 || p->dwMoney >= GOLD_MAX)
		return;

	switch (p->bState)
	{
		case SAFEBOX_MONEY_STATE_SAVE:
		{
			if (ch->GetGold() < static_cast<int>(p->dwMoney))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "You don't have enough gold.");
				return;
			}
			if (pSafebox->GetSafeboxMoney() + p->dwMoney >= GOLD_MAX)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "You cannot deposit anymore gold.");
				return;
			}
			pSafebox->SetSafeboxMoney(pSafebox->GetSafeboxMoney() + p->dwMoney);
			ch->PointChange(POINT_GOLD, -static_cast<int>(p->dwMoney));
		}
		break;

		case SAFEBOX_MONEY_STATE_WITHDRAW:
		{
			if (pSafebox->GetSafeboxMoney() < p->dwMoney)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "You don't have enough gold.");
				return;
			}
			if (ch->GetGold() + static_cast<int>(p->dwMoney) >= GOLD_MAX)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "You cannot withdraw anymore gold.");
				return;
			}
			pSafebox->SetSafeboxMoney(pSafebox->GetSafeboxMoney() - p->dwMoney);
			ch->PointChange(POINT_GOLD, static_cast<int>(p->dwMoney));
		}
		break;

		default:
			break;
	}
}
#endif

void CInputMain::PartyParameter(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGPartyParameter* p = (TPacketCGPartyParameter*)c_pData;

	if (ch->GetParty() && ch->GetParty()->GetLeaderPID() == ch->GetPlayerID())	//@fixme403
		ch->GetParty()->SetParameter(p->bDistributeMode);
}

#ifdef ENABLE_INGAME_WIKI
void CInputMain::Wikipedia(const LPCHARACTER& ch, const char* c_pData)
{
	if (!ch || !c_pData)
		return;

	const TCGWikiPacket* p = (TCGWikiPacket*)c_pData;

	switch(p->bSubHeader)
	{
		case LOAD_WIKI_ITEM:
		{
			const std::vector<TWikiItemOriginInfo>& origin_vec = ITEM_MANAGER::Instance().GetItemOrigin(p->vnum);
			std::vector<CSpecialItemGroup::CSpecialItemInfo> chest_vec; ITEM_MANAGER::instance().GetWikiChestInfo(p->vnum, chest_vec);
			std::vector<TWikiRefineInfo> refine_vec; ITEM_MANAGER::Instance().GetWikiRefineInfo(p->vnum, refine_vec);
			const TWikiInfoTable* _wif = ITEM_MANAGER::Instance().GetItemWikiInfo(p->vnum);

			if (!_wif)
				return;

			// Struct Size
			const size_t origin_size = origin_vec.size();
			const size_t chest_info_count = _wif->chest_info_count;
			const size_t refine_infos_count = _wif->refine_infos_count;
			const size_t buf_data_dize = sizeof(TGCItemWikiPacket) +
				(origin_size * sizeof(TWikiItemOriginInfo)) +
				(chest_info_count * sizeof(TWikiChestInfo)) +
				(refine_infos_count * sizeof(TWikiRefineInfo));

			if (chest_info_count != chest_vec.size())
			{
				if (test_server)
					sys_log(0, "Chest Vnum : %d || ERROR TYPE -> 1", p->vnum);

				return;
			}

			if (refine_infos_count != refine_vec.size())
			{
				if (test_server)
					sys_log(0, "Item Vnum : %d || ERROR TYPE -> 2", p->vnum);

				return;
			}

			TGCWikiPacket pack{};
			pack.bHeader = HEADER_GC_WIKI;
			pack.wSize = (uint16_t(sizeof(pack) + buf_data_dize));
			pack.bSubHeader = LOAD_WIKI_ITEM;

			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(TGCWikiPacket));

			TGCItemWikiPacket data_packet;
			data_packet.wiki_info = *_wif;
			data_packet.origin_infos_count = origin_size;
			data_packet.vnum = p->vnum;
			data_packet.ret_id = p->ret_id;
			buf.write(&data_packet, sizeof(data_packet));

			{
				// Wiki origin Data
				if (origin_size)
				{
					for (int idx = 0; idx < static_cast<int>(origin_size); ++idx)
					{
						TWikiItemOriginInfo origin_info;
						origin_info.is_mob = origin_vec[idx].is_mob;
						origin_info.vnum = origin_vec[idx].vnum;
						buf.write(&origin_info, sizeof(TWikiItemOriginInfo));
					}
				}

				// Chest Info Data
				if (chest_info_count > 0)
				{
					for (int idx = 0; idx < static_cast<int>(chest_info_count); ++idx)
					{
						TWikiChestInfo chest_info;
						chest_info.vnum = chest_vec[idx].vnum;
						chest_info.count = chest_vec[idx].count;
						buf.write(&chest_info, sizeof(TWikiChestInfo));
					}
				}

				// Refine Info Data
				if (refine_infos_count > 0)
				{
					for (int idx = 0; idx < static_cast<int>(refine_infos_count); ++idx)
					{
						TWikiRefineInfo refine_info;
						refine_info.index = refine_vec[idx].index;
						for (auto j = 0; j < REFINE_MATERIAL_MAX_NUM; ++j)
						{
							refine_info.materials[j].vnum = refine_vec[idx].materials[j].vnum;
							refine_info.materials[j].count = refine_vec[idx].materials[j].count;
						}
						refine_info.mat_count = refine_vec[idx].mat_count;
						refine_info.price = refine_vec[idx].price;
						buf.write(&refine_info, sizeof(TWikiRefineInfo));
					}
				}
			}

			ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
		break;

		case LOAD_WIKI_MOB:
		{
			CMobManager::TMobWikiInfoVector& mob_vec = CMobManager::Instance().GetMobWikiInfo(p->vnum);
			const size_t _mobVec_size = mob_vec.size();

			if (!_mobVec_size)
			{
				if (test_server)
					sys_log(0, "Mob Vnum: %d : || LOG TYPE -> 1", p->vnum);

				return;
			}

			TGCWikiPacket pack{};
			pack.bHeader = HEADER_GC_WIKI;
			pack.wSize = (uint16_t(sizeof(pack) + sizeof(TGCMobWikiPacket) + (_mobVec_size * sizeof(TWikiMobDropInfo))));
			pack.bSubHeader = LOAD_WIKI_MOB;

			TEMP_BUFFER buf;
			buf.write(&pack, sizeof(TGCWikiPacket));

			TGCMobWikiPacket sub;
			sub.drop_info_count = _mobVec_size;
			sub.vnum = p->vnum;
			sub.ret_id = p->ret_id;
			buf.write(&sub, sizeof(TGCMobWikiPacket));

			if (_mobVec_size)
			{
				for (int idx = 0; idx < (int)_mobVec_size; ++idx)
				{
					TWikiMobDropInfo sub_drop_info;
					sub_drop_info.vnum = mob_vec[idx].vnum;
					sub_drop_info.count = mob_vec[idx].count;
					buf.write(&sub_drop_info, sizeof(TWikiMobDropInfo));
				}
			}

			ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
		break;

		default:
			sys_err("Wikipedia Sub out of range!");
			break;
	}
}
#endif

size_t GetSubPacketSize(const GUILD_SUBHEADER_CG& header)
{
	switch (header)
	{
		case GUILD_SUBHEADER_CG_DEPOSIT_MONEY:
			return sizeof(int);
		case GUILD_SUBHEADER_CG_WITHDRAW_MONEY:
			return sizeof(int);
		case GUILD_SUBHEADER_CG_ADD_MEMBER:
			return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_REMOVE_MEMBER:
			return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:
			return 10;
		case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:
			return sizeof(uint8_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_OFFER:
			return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_CHARGE_GSP:
			return sizeof(int);
		case GUILD_SUBHEADER_CG_POST_COMMENT:
			return 1;
		case GUILD_SUBHEADER_CG_DELETE_COMMENT:
			return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_REFRESH_COMMENT:
			return 0;
		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:
			return sizeof(uint32_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_USE_SKILL:
			return sizeof(TPacketCGGuildUseSkill);
		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:
			return sizeof(uint32_t) + sizeof(uint8_t);
		case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:
			return sizeof(uint32_t) + sizeof(uint8_t);
#ifdef ENABLE_GUILDBANK_LOG
		case GUILD_SUBHEADER_CG_REFRESH:
			return 0;
#endif
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
		case GUILD_SUBHEADER_CG_CHANGE_GUILDMASTER:
			return sizeof(uint32_t);
		case GUILD_SUBHEADER_CG_DELETE_LAND:
			return sizeof(int);
#endif
	}

	return 0;
}

int CInputMain::Guild(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (!ch)
		return -1;

	if (uiBytes < sizeof(TPacketCGGuild))
		return -1;

	const TPacketCGGuild* p = reinterpret_cast<const TPacketCGGuild*>(data);
	const char* c_pData = data + sizeof(TPacketCGGuild);

	uiBytes -= sizeof(TPacketCGGuild);

	const GUILD_SUBHEADER_CG SubHeader = static_cast<GUILD_SUBHEADER_CG>(p->subheader);
	const size_t SubPacketLen = GetSubPacketSize(SubHeader);

	if (uiBytes < SubPacketLen)
	{
		return -1;
	}

	CGuild* pGuild = ch->GetGuild();

	if (nullptr == pGuild)
	{
		if (SubHeader != GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[523][Guild] It does not belong to the guild."));
			return SubPacketLen;
		}
	}

	switch (SubHeader)
	{
		case GUILD_SUBHEADER_CG_DEPOSIT_MONEY:	//Einzahlen
			{
#ifndef ENABLE_GUILDRENEWAL_SYSTEM
				return SubPacketLen;
#endif
				const int gold = MIN(*reinterpret_cast<const int*>(c_pData), __deposit_limit());

				if (gold < 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;686]");
					return SubPacketLen;
				}

				if (ch->GetGold() < gold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;687]");
					return SubPacketLen;
				}

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
				/*if (gold > GOLD_MAX/2) {	//Can't Put more than 2kkk into Guildstorage from your Pocket
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_PUT_IN_SO_MUCH_MONEY_LIMIT_REACHED"));
					return SubPacketLen;
				}

				if (pGuild->GetGuildMoney() > GOLD_MAX/2) {	//Can't Put More than Gold_Max into Guildbank
					int gold_storage = pGuild->GetGuildMoney() - GOLD_MAX/2;
					int gold_add = GOLD_MAX/2 - gold_storage;

					if (gold + gold_storage > GOLD_MAX/2) {
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILDSTORAGE_ADDED_INSTEAD %d"), gold_add);
						pGuild->RequestDepositMoney(ch, gold_add);
					}
					else
						pGuild->RequestDepositMoney(ch, gold);
				}
				else
					pGuild->RequestDepositMoney(ch, gold);*/

				if (gold > GOLD_MAX - 1) {	//Can't Put more than 2kkk into Guildstorage from your Pocket
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_PUT_IN_SO_MUCH_MONEY_LIMIT_REACHED"));
					return SubPacketLen;
				}

				if ((gold + pGuild->GetGuildMoney()) > (GOLD_MAX - 1)) {
					int gold_add = (GOLD_MAX - 1) - pGuild->GetGuildMoney();
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILDSTORAGE_TOOK_INSTEAD %d"), gold_add);
					pGuild->RequestDepositMoney(ch, gold_add);
				}
				else
					pGuild->RequestDepositMoney(ch, gold);
#else
				pGuild->RequestDepositMoney(ch, gold);
#endif
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_WITHDRAW_MONEY:	//Auszahlen
			{
#ifndef ENABLE_GUILDRENEWAL_SYSTEM
				return SubPacketLen;
				const int gold = MIN(*reinterpret_cast<const int*>(c_pData), 500000);
#else
				const int gold = MIN(*reinterpret_cast<const int*>(c_pData), __deposit_limit());
#endif

				if (gold < 0) {
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;686]");
					return SubPacketLen;
				}

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
				if (gold > GOLD_MAX) {
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_PUT_MORE_THEN_MAX_GOLD_OUT"));
					return SubPacketLen;
				}
				if (gold > pGuild->GetGuildMoney()) {	//Can't put out more than Money in Guildbank
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_PUT_OUT_SO_MUCH_MONEY_LIMIT_REACHED"));
					return SubPacketLen;
				}

				if ((gold + ch->GetGold()) > (GOLD_MAX - 1)) {
					int gold_add = (GOLD_MAX - 1) - ch->GetGold();
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILDSTORAGE_TOOK_INSTEAD %d"), gold_add);
					pGuild->RequestWithdrawMoney(ch, gold_add);
				}
				else
					pGuild->RequestWithdrawMoney(ch, gold);

				/*if (gold > GOLD_MAX/2) {
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_PUT_MORE_THEN_MAX_GOLD_OUT"));
					return SubPacketLen;
				}

				if (gold > pGuild->GetGuildMoney()) {	//Can't put out more than Money in Guildbank
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_PUT_OUT_SO_MUCH_MONEY_LIMIT_REACHED"));
					return SubPacketLen;
				}*/

				/* OLD
				if (ch->GetGold() > GOLD_MAX/2) {	//Can't Put More than Gold_Max into Guildbank
					int gold_pocket = ch->GetGold() - GOLD_MAX/2;

					if (gold + gold_pocket >= GOLD_MAX/2) {
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("POCKET_MAX_LIMIT_REACHED"));
						return SubPacketLen;
					}
				}//OLD_END*/	

				/*if (ch->GetGold() > GOLD_MAX/2) {
					int gold_pocket = ch->GetGold() - GOLD_MAX/2;
					int gold_add = GOLD_MAX/2 - gold_pocket;

					if (gold + gold_pocket > GOLD_MAX/2) {
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GUILDSTORAGE_TOOK_INSTEAD %d"), gold_add);
						pGuild->RequestWithdrawMoney(ch, gold_add);
					}
					else
						pGuild->RequestWithdrawMoney(ch, gold);
				}
				else
					pGuild->RequestWithdrawMoney(ch, gold);*/
#else
				pGuild->RequestWithdrawMoney(ch, gold);
#endif
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_ADD_MEMBER:
			{
				const uint32_t vid = *reinterpret_cast<const uint32_t*>(c_pData);
				LPCHARACTER newmember = CHARACTER_MANAGER::Instance().Find(vid);

				if (!newmember)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;688]");
					return SubPacketLen;
				}
				
#ifdef ENABLE_MESSENGER_BLOCK
				if (MessengerManager::Instance().IsBlocked(ch->GetName(), newmember->GetName()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), newmember->GetName());
					return SubPacketLen;
				}
				else if (MessengerManager::Instance().IsBlocked(newmember->GetName(), ch->GetName()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), newmember->GetName());
					return SubPacketLen;
				}
#endif

				// @fixme145 BEGIN (+newmember ispc check)
				if (!ch->IsPC() || !newmember->IsPC())
					return SubPacketLen;
				// @fixme145 END

#ifdef ENABLE_BATTLE_FIELD
				if (CBattleField::Instance().IsBattleZoneMapIndex(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BATTLE_ZONE_CANT_INVITE_GUILD"));
					return SubPacketLen;
				}
#endif

				pGuild->Invite(ch, newmember);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_REMOVE_MEMBER:
			{
				if (pGuild->UnderAnyWar() != 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü Áß¿¡´Â ±æµå¿øÀ» Å»Åð½ÃÅ³ ¼ö ¾ø½À´Ï´Ù."));
					return SubPacketLen;
				}

				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				LPCHARACTER member = CHARACTER_MANAGER::Instance().FindByPID(pid);

				if (member)
				{
					if (member->GetGuild() != pGuild)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;689]");
						return SubPacketLen;
					}

					if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;690]");
						return SubPacketLen;
					}

					member->SetQuestFlag("guild_manage.new_withdraw_time", get_global_time());
					pGuild->RequestRemoveMember(member->GetPlayerID());

					if (g_bGuildInviteLimit)
					{
						DBManager::Instance().Query("REPLACE INTO guild_invite_limit VALUES(%d, %d)", pGuild->GetID(), get_global_time());
					}
				}
				else
				{
					if (!pGuild->HasGradeAuth(m->grade, GUILD_AUTH_REMOVE_MEMBER))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;690]");
						return SubPacketLen;
					}

					if (pGuild->RequestRemoveMember(pid))
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;693]");
					else
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;688]");
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME:
			{
				char gradename[GUILD_GRADE_NAME_MAX_LEN + 1];
				strlcpy(gradename, c_pData + 1, sizeof(gradename));

				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;694]");
				}
				else if (*c_pData == GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;695]");
				}
				else if (!check_name(gradename))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;696]");
				}
				else
				{
					pGuild->ChangeGradeName(*c_pData, gradename);
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY:
			{
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;697]");
				}
				else if (*c_pData == GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;698]");
				}
				else
				{
					pGuild->ChangeGradeAuth(*c_pData, *(c_pData + 1));
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_OFFER:
			{
				uint32_t offer = *reinterpret_cast<const uint32_t*>(c_pData);

				if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå°¡ ÀÌ¹Ì ÃÖ°í ·¹º§ÀÔ´Ï´Ù."));
				}
				else
				{
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
#	ifdef ENABLE_GUILD_DONATE_ATTENDANCE
					if (pGuild->GetLevel() >= 20)
					{
						pGuild->OfferExpNew(ch, offer);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
						ch->UpdateExtBattlePassMissionProgress(GUILD_SPENT_EXP, offer, 0);
#endif
					}
					else
#	endif
					{
						if (pGuild->OfferExp(ch, offer))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> %uÀÇ °æÇèÄ¡¸¦ ÅõÀÚÇÏ¿´½À´Ï´Ù."), offer);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
							ch->UpdateExtBattlePassMissionProgress(GUILD_SPENT_EXP, offer, 0);
#endif
						}
						else
							ch->ChatPacket(CHAT_TYPE_INFO, "[LS;700]");
					}
#else
					offer /= 100;
					offer *= 100;

					if (pGuild->OfferExp(ch, offer))
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> %uÀÇ °æÇèÄ¡¸¦ ÅõÀÚÇÏ¿´½À´Ï´Ù."), offer);
					else
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;700]");
#endif

				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHARGE_GSP:
			{
				const int offer = *reinterpret_cast<const int*>(c_pData);
				const int gold = offer * 100;

				if (offer < 0 || gold < offer || gold < 0 || ch->GetGold() < gold)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
					return SubPacketLen;
				}

				if (!pGuild->ChargeSP(ch, offer))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;702]");
				}

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_EXTENDED_LOGS)
				int _gold;
				if (pGuild->GetLevel() >= 15)	// Preisnachlass: Drachengeist (10% weniger Yang ben?igt) [Bonus 3]
					_gold = gold * 0.90;
				else
					_gold = gold;

				LogManager::Instance().GuildLog(ch, ch->GetGuild()->GetID(), 0, "DragonSP", GUILD_GOLD_TYPE_DRAGON_SP_RECOVER, _gold, 1);
				/*auto pmsg(DBManager::Instance().DirectQuery("INSERT INTO log.guildstorage (guild_id, time, playerid, chrname, itemvnum, itemname, iteminout, itemcount, datatype) VALUES(%u, NOW(), %u, '%s', %d, '%s', %d, %u, %d)",
					pGuild->GetID(), ch->GetPlayerID(), ch->GetName(), 0, "DragonSP", GUILD_GOLD_TYPE_DRAGON_SP_RECOVER, gold, 1));*/
#endif
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_POST_COMMENT:
			{
				const size_t length = *c_pData;

				if (length > GUILD_COMMENT_MAX_LEN)
				{
					// The wrong way... Let's cut it off.
					sys_err("POST_COMMENT: %s comment too long (length: %u)", ch->GetName(), length);
					ch->GetDesc()->SetPhase(PHASE_CLOSE);
					return -1;
				}

				if (uiBytes < 1 + length)
					return -1;

				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (length && !pGuild->HasGradeAuth(m->grade, GUILD_AUTH_NOTICE) && *(c_pData + 1) == '!')
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;704]");
				}
				else
				{
					std::string str(c_pData + 1, length);
					pGuild->AddComment(ch, str);
				}

				return (1 + length);
			}

		case GUILD_SUBHEADER_CG_DELETE_COMMENT:
			{
				const uint32_t comment_id = *reinterpret_cast<const uint32_t*>(c_pData);

				pGuild->DeleteComment(ch, comment_id);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_REFRESH_COMMENT:
			pGuild->RefreshComment(ch);
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE:
			{
				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t grade = *(c_pData + sizeof(uint32_t));
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;705]");
				else if (ch->GetPlayerID() == pid)
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;706]");
				else if (grade == 1)
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀåÀ¸·Î Á÷À§¸¦ º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù."));
				else
					pGuild->ChangeMemberGrade(pid, grade);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_USE_SKILL:
			{
				const TPacketCGGuildUseSkill* p = reinterpret_cast<const TPacketCGGuildUseSkill*>(c_pData);

				pGuild->UseSkill(p->dwVnum, ch, p->dwPID);
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL:
			{
				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t is_general = *(c_pData + sizeof(uint32_t));
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[LS;708]");	//[Gilde] Du bist nicht berechtigt, den Gildenmeister auszuw?len.
				}
				else
				{
					if (!pGuild->ChangeMemberGeneral(pid, is_general))
					{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Gilde] Du bist bereits ein Fighter des Meley!"));
#else
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;709]");	//[Gilde] Du kannst keine weiteren Gildenmeister ausw?len.
#endif
					}
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER:
			{
				const uint32_t guild_id = *reinterpret_cast<const uint32_t*>(c_pData);
				const uint8_t accept = *(c_pData + sizeof(uint32_t));

				CGuild* g = CGuildManager::Instance().FindGuild(guild_id);

				if (g)
				{
					if (accept)
						g->InviteAccept(ch);
					else
						g->InviteDeny(ch->GetPlayerID());
				}
			}
			return SubPacketLen;

#ifdef ENABLE_GUILDBANK_LOG
		case GUILD_SUBHEADER_CG_REFRESH:
			{
				pGuild->SetBankInfo();
			}
			return SubPacketLen;
#endif

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
		case GUILD_SUBHEADER_CG_CHANGE_GUILDMASTER:
			{
				if (pGuild->UnderAnyWar() != 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü Áß¿¡´Â ±æµå¿øÀ» Å»Åð½ÃÅ³ ¼ö ¾ø½À´Ï´Ù."));
					return SubPacketLen;
				}

				const uint32_t pid = *reinterpret_cast<const uint32_t*>(c_pData);
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				LPCHARACTER member = CHARACTER_MANAGER::Instance().FindByPID(pid);
				LPCHARACTER myself = CHARACTER_MANAGER::Instance().FindByPID(ch->GetPlayerID());

				if (!ch->CountSpecifyItem(71099)) {	//Ring der Nachfolge
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Gilde> Du ben?igst einen Ring der Nachfolge um die Gilde weiter geben zu k?nen. "));
					return SubPacketLen;
				}

				if (member)
				{
					if (member->GetGuild() != pGuild) {
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;689]");
						return SubPacketLen;
					}
					if (member == myself) {
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Gilde> Dir geh?t diese Gilde bereits. "));
						return SubPacketLen;
					}

					if (pGuild->ChangeMasterTo(member->GetPlayerID()))
					{
						ch->RemoveSpecifyItem(71099, 1);	//Ring der Nachfolge
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Guild] You have successfully handed over your guild! -- member = true"));
					}
					else
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;688]");
				}
			}
			return SubPacketLen;

		case GUILD_SUBHEADER_CG_DELETE_LAND:
			{
				const int vid = *reinterpret_cast<const int*>(c_pData);
				const TGuildMember* m = pGuild->GetMember(ch->GetPlayerID());

				if (nullptr == m)
					return -1;

				if (m->grade != GUILD_LEADER_GRADE) {
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Gilde] Du hast nicht die Befugnis dies zu tun!"));
				}
				else {
					if (!pGuild->HasLand()) {
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Gilde] Du besitzt kein Land um dies zu tun!"));
						return SubPacketLen;
					}

					if (pGuild->GetGuildLandDeletePulse() > time(0)/* && !ch->IsGM()*/) {
						int deltaInSeconds = pGuild->GetGuildLandDeletePulse() - time(0);

						uint32_t minutes, hours, secs_left, mins_left;
						hours = deltaInSeconds / 3600;
						minutes = deltaInSeconds / 60;
						mins_left = minutes % 60;
						secs_left = deltaInSeconds % 60;

						ch->ChatPacket(CHAT_TYPE_INFO, "Du kannst diese Aktion erst in: %02d Stunden %02d Minuten %02d Sekunden ausfuehren!", hours, mins_left, secs_left);
						return SubPacketLen;
					}

					pGuild->SetGuildLandDeletePulse(time(0) + 86400);	//[seconds] to do action again

					building::CManager::Instance().ClearLandByGuildID(vid);
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Land and Objects deleted!"));

					pGuild->SetGuildLandInfo();
				}
			}
			return SubPacketLen;
#endif
	}

	return 0;
}

void CInputMain::Fishing(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGFishing* p = (TPacketCGFishing*)c_pData;
	ch->SetRotation(static_cast<float>(p->dir * 5));
	ch->fishing();
	return;
}

#ifdef ENABLE_MAILBOX
void CInputMain::MailboxWrite(LPCHARACTER ch, const char* c_pData)
{
	const auto* p = reinterpret_cast<const TPacketCGMailboxWrite*>(c_pData);
	if (p == nullptr)
		return;
	
	CMailBox* mail = ch->GetMailBox();
	if (mail == nullptr)
		return;

	mail->Write(p->szName, p->szTitle, p->szMessage, p->pos, p->iYang, p->iWon);
}

void CInputMain::MailboxConfirm(LPCHARACTER ch, const char* c_pData)
{
	const auto* p = reinterpret_cast<const TPacketCGMailboxWriteConfirm*>(c_pData);
	if (p == nullptr)
		return;
	
	CMailBox* mail = ch->GetMailBox();
	if (mail == nullptr)
		return;

	mail->CheckPlayer(p->szName);
}

void CInputMain::MailboxProcess(LPCHARACTER ch, const char* c_pData)
{
	const auto* p = reinterpret_cast<const TPacketMailboxProcess*>(c_pData);
	if (p == nullptr)
		return;
	
	CMailBox* mail = ch->GetMailBox();
	if (mail == nullptr)
		return;

	switch (p->bSubHeader)
	{
		case CMailBox::EMAILBOX_CG::MAILBOX_CG_CLOSE:
			ch->SetMailBox(nullptr);
			break;

		case CMailBox::EMAILBOX_CG::MAILBOX_CG_DELETE:
			mail->DeleteMail(p->bArg1, false);
			break;

		case CMailBox::EMAILBOX_CG::MAILBOX_CG_ALL_DELETE:
			mail->DeleteAllMails();
			break;

		case CMailBox::EMAILBOX_CG::MAILBOX_CG_GET_ITEMS:
			mail->GetItem(p->bArg1, false);
			break;

		case CMailBox::EMAILBOX_CG::MAILBOX_CG_ALL_GET_ITEMS:
			mail->GetAllItems();
			break;

		case CMailBox::EMAILBOX_CG::MAILBOX_CG_ADD_DATA:
			mail->AddData(p->bArg1, p->bArg2);
			break;

		default:
			sys_err("CInputMain::MailboxProcess Unknown SubHeader (ch: %s) (%d)", ch->GetName(), p->bSubHeader);
			break;
	}
}
#endif

#ifdef ENABLE_PARTY_MATCH
#include "GroupMatchManager.h"
void CInputMain::PartyMatch(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const auto pinfo = reinterpret_cast<const TPacketCGPartyMatch*>(data);

	if (pinfo->SubHeader == CGroupMatchManager::EPacketGCPartyMatchSubHeader::PARTY_MATCH_SEARCH)
	{
		CGroupMatchManager::Instance().AddRestricted(pinfo->index);
		CGroupMatchManager::Instance().AddSearcher(ch, pinfo->index);
	}
	else
	{
		CGroupMatchManager::Instance().AddRestricted(0);
		CGroupMatchManager::Instance().StopSearching(ch, CGroupMatchManager::EPartyMatchMsg::PARTY_MATCH_CANCEL_SUCCESS, pinfo->index);
	}
}
#endif

void CInputMain::ItemGive(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGiveItem* p = (TPacketCGGiveItem*)c_pData;
	LPCHARACTER to_ch = CHARACTER_MANAGER::Instance().Find(p->dwTargetVID);

	if (to_ch)
		ch->GiveItem(to_ch, p->ItemPos);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[710]You cannot trade this item."));
}

void CInputMain::Hack(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGHack* p = (TPacketCGHack*)c_pData;

#ifdef ENABLE_MAIN_HACK
	if (!p || !p->szBuf || !strlen(p->szBuf))
		return;

	char buf[sizeof(p->szBuf)];
	strlcpy(buf, p->szBuf, sizeof(buf));

	char __escape_string[1024];
	DBManager::Instance().EscapeString(__escape_string, sizeof(__escape_string), buf, strlen(p->szBuf));

	LogManager::Instance().HackLog(__escape_string, ch);
#else
	char buf[sizeof(p->szBuf)];
	strlcpy(buf, p->szBuf, sizeof(buf));

	sys_err("HACK_DETECT: %s %s", ch->GetName(), buf);
#endif

	// Since there is no case of sending this packet from the current client, it should be unconditionally hang
	ch->GetDesc()->SetPhase(PHASE_CLOSE);
}

int CInputMain::MyShop(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	if (!ch)
		return -1;

	const TPacketCGMyShop* p = (TPacketCGMyShop*)c_pData;
	const int iExtraLen = p->bCount * sizeof(TShopItemTable);

	if (uiBytes < sizeof(TPacketCGMyShop) + iExtraLen)
		return -1;

	if (ch->GetGold() >= GOLD_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[712]You have more than 2 Billion Yang with you. You cannot trade."));
		sys_log(0, "MyShop ==> OverFlow Gold id %u name %s ", ch->GetPlayerID(), ch->GetName());
		return (iExtraLen);
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	if (ch->GetCheque() >= CHEQUE_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[712]You have more than 2 Billion Yang with you. You cannot trade."));
		sys_log(0, "MyShop ==> OverFlow Cheque id %u name %s ", ch->GetPlayerID(), ch->GetName());
		return (iExtraLen);
	}
#endif

	if (ch->IsStun() || ch->IsDead())
		return (iExtraLen);

	if (ch->GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_SHOP_OWNER | W_CUBE
#ifdef ENABLE_SKILLBOOK_COMBINATION
		| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
		| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
		| W_SWITCHBOT
#endif
	))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[664]You cannot open a private shop while another window is open."));
		return (iExtraLen);
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (!ch->GetShopItems().empty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1712][Personal Premium Shop] You can't open this, because you still have items to collect."));
		return iExtraLen;
	}
#endif

	// Disallow shops on certain maps
	if (!CShop::CanOpenShopHere(ch->GetMapIndex()))	//@custom023
		return iExtraLen;

	sys_log(0, "MyShop count %d", p->bCount);
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	sys_log(0, "MyShopDeco count PolyVnum: %d Type: %d", ch->GetMyShopDecoPolyVnum(), ch->GetMyShopType());
#endif
	ch->OpenMyShop(p->szSign, (TShopItemTable*)(c_pData + sizeof(TPacketCGMyShop)), p->bCount
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
		, ch->GetMyShopDecoPolyVnum(), ch->GetMyShopType()
#endif
	);

	const LPCHARACTER myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(ch->GetPlayerID());
	if (myShopChar) {
		myShopChar->ViewReencode();
	}

	return (iExtraLen);
}

void CInputMain::Refine(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGRefine* p = reinterpret_cast<const TPacketCGRefine*>(c_pData);

	if (ch->GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_SHOP_OWNER | W_CUBE
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
		| W_MYSHOP
#endif
#ifdef ENABLE_SKILLBOOK_COMBINATION
		| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
		| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
		| W_SWITCHBOT
#endif
	))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1014]You cannot upgrade anything while another window is open."));
		ch->ClearRefineMode();
		return;
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (ch->GetGrowthPetSystem()->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1767]You cannot improve that while an evolvable pet has been summoned."));
		ch->ClearRefineMode();
		return;
	}
#endif

	if (p->type == 255)
	{
		// DoRefine Cancel
		ch->ClearRefineMode();
		return;
	}

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	if (p->wPos >= ch->GetExtendInvenMax())
#else
	if (p->wPos >= INVENTORY_MAX_NUM)
#endif
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		if (!(p->wPos >= SPECIAL_INVENTORY_SLOT_START && p->wPos < SPECIAL_INVENTORY_SLOT_END))
		{
			ch->ClearRefineMode();
			return;
		}
#else
		ch->ClearRefineMode();
		return;
#endif
	}

	LPITEM item = ch->GetInventoryItem(p->wPos);
	if (!item)
	{
		ch->ClearRefineMode();
		return;
	}

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
	{
		ch->ClearRefineMode();
		ch->ChatPacket(CHAT_TYPE_INFO, "You can't refine this item because is binded!");
		return;
	}
#endif

/*#ifdef ENABLE_GIVE_BASIC_ITEM
	if (item->IsBasicItem()) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		ch->ClearRefineMode();
		return;
	}
#endif*/

	ch->SetRefineTime();

	if (p->type == REFINE_TYPE_NORMAL)
	{
		sys_log(0, "refine_type_normal");
		ch->DoRefine(item);
	}

	else if (p->type == REFINE_TYPE_SCROLL || p->type == REFINE_TYPE_HYUNIRON || p->type == REFINE_TYPE_MUSIN || p->type == REFINE_TYPE_BDRAGON
#ifdef ENABLE_RITUAL_STONE
		|| p->type == REFINE_TYPE_RITUALS_SCROLL
#endif
#ifdef ENABLE_SEAL_OF_GOD
		|| p->type == REFINE_TYPE_SEAL_GOD
#endif
		)
	{
		sys_log(0, "refine_type_scroll, ...");
		ch->DoRefineWithScroll(item);
	}
#ifdef ENABLE_SOUL_SYSTEM
	else if (p->type == REFINE_TYPE_SOUL_EVOLVE || p->type == REFINE_TYPE_SOUL_AWAKE)
	{
		sys_log(0, "refine_type_soul %d", p->type);
		ch->DoRefineSoul(item);
	}
#endif
	else if (p->type == REFINE_TYPE_MONEY_ONLY)
	{
		const LPITEM& item2 = ch->GetInventoryItem(p->wPos);

		if (nullptr != item2)
		{
			if (500 <= item2->GetRefineSet())
			{
				LogManager::Instance().HackLog("DEVIL_TOWER_REFINE_HACK", ch);
			}
			else
			{
#ifdef ENABLE_QUEEN_NETHIS
				if (SnakeLair::CSnk::Instance().IsSnakeMap(ch->GetMapIndex()))
				{
					if (get_global_time() > ch->GetQuestFlag("snake_lair.refine_time"))
					{
						if (ch->DoRefineSerpent(item2))
							ch->SetQuestFlag("snake_lair.refine_time", get_global_time() + 60 * 60 * 24);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can only be rewarded each 24h for the Serpent Temple Smith.")); // Custom
					}
				}
				else
#endif
				{
					if (ch->GetQuestFlag("deviltower_zone.can_refine") > 0) // @fixme158 (allow multiple refine attempts)
					{
						if (ch->DoRefine(item2, true))
							ch->SetQuestFlag("deviltower_zone.can_refine", ch->GetQuestFlag("deviltower_zone.can_refine") - 1);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1067]You can only be rewarded once for the Demon Tower Quest."));
					}
				}
			}
		}
	}

	ch->ClearRefineMode();
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
void CInputMain::AcceCloseRequest(LPCHARACTER ch, const char* c_pData)
{
	if (ch)
		ch->AcceClose();
}

void CInputMain::AcceRefineRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGSendAcceRefine* p = (TPacketCGSendAcceRefine*)c_pData;
	ch->AcceRefine(p->bAcceWindow, p->bSlotAcce, p->bSlotMaterial);
}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
void CInputMain::MoveCostumeAttrRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGSendCostumeRefine* p = (TPacketCGSendCostumeRefine*)c_pData;
	ch->MoveCostumeAttr(p->bSlotMedium, p->bSlotBase, p->bSlotMaterial);
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
#include "Transmutation.h"
void CInputMain::Transmutation(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	auto p = reinterpret_cast<const TPacketCGTransmutation*>(c_pData);
	
	CTransmutation* CLook = ch->IsChangeLookWindowOpen();
	if (CLook == nullptr)
		return;

	switch (static_cast<ECG_TRANSMUTATION_SHEADER>(p->subheader))
	{
		case ECG_TRANSMUTATION_SHEADER::ITEM_CHECK_IN:
			CLook->ItemCheckIn(p->pos, p->slot_type);
			break;

		case ECG_TRANSMUTATION_SHEADER::ITEM_CHECK_OUT:
			CLook->ItemCheckOut(p->slot_type);
			break;

		case ECG_TRANSMUTATION_SHEADER::FREE_ITEM_CHECK_IN:
			CLook->FreeItemCheckIn(p->pos);
			break;

		case ECG_TRANSMUTATION_SHEADER::FREE_ITEM_CHECK_OUT:
			CLook->FreeItemCheckOut();
			break;

		case ECG_TRANSMUTATION_SHEADER::ACCEPT:
			CLook->Accept();
			break;

		case ECG_TRANSMUTATION_SHEADER::CANCEL:
			ch->SetTransmutation(nullptr);
			break;

		default:
			sys_err("Unknown Subheader ch:%s, %d", ch->GetName(), p->subheader);
			return;
	}
}
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
void CInputMain::ExtendInvenRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	TPacketCGSendExtendInvenRequest* packet = (TPacketCGSendExtendInvenRequest*)c_pData;

	if (packet->bSpecialState) // Special Inv
		ch->ExtendSpecialInvenRequest(packet->bWindow);
	else // Inventory
		ch->ExtendInvenRequest();
# else
	ch->ExtendInvenRequest();
# endif
}

void CInputMain::ExtendInvenUpgrade(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	TPacketCGSendExtendInvenUpgrade* packet = (TPacketCGSendExtendInvenUpgrade*)c_pData;

	if (packet->bSpecialState)
		ch->ExtendSpecialInvenUpgrade(packet->bWindow);
	else
		ch->ExtendInvenUpgrade();
# else
	ch->ExtendInvenUpgrade();
# endif
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
size_t GetGrowthPetSubPacketLength(const EPacketCGGrowthPetSubHeader& SubHeader)
{
	switch (SubHeader)
	{
		case GROWTHPET_SUBHEADER_CG_WINDOW_TYPE:
			return sizeof(TSubPacketCGGrowthPetWindowType);
		case GROWTHPET_SUBHEADER_CG_HATCHING_WINDOW:
			return sizeof(TSubPacketCGGrowthPetHatchWindow);
	}

	return 0;
}

int CInputMain::GrowthPet(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (!ch)
		return -1;

	if (uiBytes < sizeof(TPacketCGGrowthPet))
		return -1;

	const TPacketCGGrowthPet* pinfo = reinterpret_cast<const TPacketCGGrowthPet*>(data);
	const char* c_pData = data + sizeof(TPacketCGGrowthPet);

	uiBytes -= sizeof(TPacketCGGrowthPet);

	const EPacketCGGrowthPetSubHeader SubHeader = static_cast<EPacketCGGrowthPetSubHeader>(pinfo->bSubHeader);
	const size_t SubPacketLength = GetGrowthPetSubPacketLength(SubHeader);
	if (uiBytes < SubPacketLength)
	{
		sys_err("invalid growth pet subpacket length (sublen %d size %u buffer %u)",
			SubPacketLength, sizeof(TPacketCGGrowthPet), uiBytes);
		return -1;
	}

	switch (SubHeader)
	{
		case GROWTHPET_SUBHEADER_CG_WINDOW_TYPE:
		{
			const TSubPacketCGGrowthPetWindowType* sp = reinterpret_cast<const TSubPacketCGGrowthPetWindowType*>(c_pData);
			ch->SetPetWindowType(sp->bPetWindowType);
		}
		return SubPacketLength;

		case GROWTHPET_SUBHEADER_CG_HATCHING_WINDOW:
		{
			const TSubPacketCGGrowthPetHatchWindow* sp = reinterpret_cast<const TSubPacketCGGrowthPetHatchWindow*>(c_pData);
			ch->SetGrowthPetHatchingWindow(sp->isOpen);
		}
		return SubPacketLength;
	}

	return 0;
}

void CInputMain::GrowthPetHatching(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetHatching* p = (TPacketCGGrowthPetHatching*)c_pData;

	LPITEM pPetEgg = ch->GetInventoryItem(p->bPos);
	if (!pPetEgg)
	{
		ch->SendGrowthPetHatching(EGG_USE_FAILED_TIMEOVER, p->bPos);
		return;
	}

	if ((pPetEgg->GetType() != ITEM_PET) || (pPetEgg->GetSubType() != PET_EGG))
	{
		ch->SendGrowthPetHatching(EGG_USE_FAILED_TIMEOVER, p->bPos);
		return;
	}

	if (ch->GetGold() < PET_HATCHING_MONEY)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1173]You do not have enough Yang."));
		return;
	}

	if (strlen(p->sGrowthPetName) < PET_NAME_MIN_SIZE || strlen(p->sGrowthPetName) > PET_NAME_MAX_SIZE)
	{
		ch->SendGrowthPetHatching(EGG_USE_FAILED_BECAUSE_NAME, p->bPos);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1171]The name entered is too short."));
		return;
	}

	if (!check_name(p->sGrowthPetName))
	{
		ch->SendGrowthPetHatching(EGG_USE_FAILED_BECAUSE_NAME, p->bPos);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1161]Please enter another name."));
		return;
	}

	// Check Name
	static char __pet_name[PET_NAME_MAX_SIZE * 2 + 1];
	DBManager::Instance().EscapeString(__pet_name, sizeof(__pet_name), p->sGrowthPetName, strnlen(p->sGrowthPetName, sizeof(p->sGrowthPetName)));
	if (strncmp(__pet_name, p->sGrowthPetName, strnlen(p->sGrowthPetName, sizeof(p->sGrowthPetName))))
	{
		sys_err("<GrowthPetSystem> EscapeString: Invalid pet name!.");

		ch->SendGrowthPetHatching(EGG_USE_FAILED_BECAUSE_NAME, p->bPos);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1161]Please enter another name."));
		return;
	}

	const uint32_t dwPetVnum = pPetEgg->GetValue(0);
	LPITEM pPetSeal = ITEM_MANAGER::Instance().CreateItem(dwPetVnum, 1, 0, true);
	if (!pPetSeal)
	{
		sys_err("<GrowthPet> could not create pet vnum (%d).", dwPetVnum);
		return;
	}

	// Pet Struct
	const uint8_t bPetInfo = dwPetVnum - 55701;
	TGrowthPetInfo petInfo = {};
	strncpy(petInfo.pet_nick, __pet_name, sizeof(petInfo.pet_nick));

	petInfo.pet_vnum = dwPetVnum;
	petInfo.pet_level = 1;
	petInfo.evol_level = 1;
	petInfo.pet_type = static_cast<uint8_t>(number(1, 6));

	petInfo.pet_hp = fnumber(0.1f, 2.3f);
	petInfo.pet_def = fnumber(0.1f, 2.1f);
	petInfo.pet_sp = fnumber(0.1f, 2.9f);

	petInfo.pet_max_time = (60 * 60 * 24) * (number(PET_HATCH_INFO_RANGE[bPetInfo][1], PET_HATCH_INFO_RANGE[bPetInfo][2]));
	petInfo.pet_birthday = time(0);
	petInfo.exp_monster = 0;
	petInfo.exp_item = 0;
	petInfo.skill_count = static_cast<uint8_t>(number(PET_HATCH_INFO_RANGE[bPetInfo][3], PET_HATCH_INFO_RANGE[bPetInfo][4]));

	const time_t duration = petInfo.pet_birthday + petInfo.pet_max_time;

	// Query
	char szQuery[QUERY_MAX_LEN], szColumns[QUERY_MAX_LEN], szValues[QUERY_MAX_LEN];

	snprintf(szColumns, sizeof(szColumns),
		"id, pet_vnum, pet_nick, pet_type, pet_hp, pet_def, pet_sp, "
		"pet_duration, pet_birthday, skill_count");

	snprintf(szValues, sizeof(szValues),
		"NULL, %d, '%s', %d, %.1f, %.1f, %.1f, %d, %d, %d",
		petInfo.pet_vnum, petInfo.pet_nick, petInfo.pet_type, petInfo.pet_hp, petInfo.pet_def, petInfo.pet_sp,
		petInfo.pet_max_time, petInfo.pet_birthday, petInfo.skill_count);

	snprintf(szQuery, sizeof(szQuery), "INSERT INTO pet (%s) VALUES(%s);", szColumns, szValues);

	auto pMsg(DBManager::Instance().DirectQuery(szQuery));
	const SQLResult* pRes = pMsg->Get();

	if (pRes->uiAffectedRows <= 0 || pRes->uiInsertID == 0)
	{
		ch->SendGrowthPetHatching(EGG_USE_FAILED_TIMEOVER, p->bPos);
		M2_DESTROY_ITEM(pPetSeal);
		return;
	}

	// Pet ID
	petInfo.pet_id = static_cast<uint32_t>(pMsg->Get()->uiInsertID);

	// Pet Egg Pos
	const auto cell = pPetEgg->GetCell();

	// Remove Gold
	DBManager::Instance().SendMoneyLog(MONEY_LOG_QUEST, ch->GetPlayerID(), -PET_HATCHING_MONEY);
	ch->PointChange(POINT_GOLD, -PET_HATCHING_MONEY, true);

	// Remove Pet Egg Item
	ITEM_MANAGER::Instance().RemoveItem(pPetEgg, "PET_HATCHING");

	// Pet Informations
	pPetSeal->SetSocket(0, duration); // Pet Duration
	pPetSeal->SetSocket(1, petInfo.pet_max_time); // Pet Birthdays
	pPetSeal->SetSocket(2, petInfo.pet_id); // Pet Id
	pPetSeal->SetGrowthPetItemInfo(petInfo);

	// Add to Character
	pPetSeal->AddToCharacter(ch, TItemPos(INVENTORY, cell));

	// Send Success
	ch->SendGrowthPetHatching(EGG_USE_SUCCESS, p->bPos);

	// Send pet info
	ch->SetGrowthPetInfo(petInfo);
	ch->SendGrowthPetInfoPacket();

	// Chat Message
	ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1150;%s]", pPetSeal->GetName());
}

void CInputMain::GrowthPetLearnSkill(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetLearnSkill* p = (TPacketCGGrowthPetLearnSkill*)c_pData;

	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	pGrowthPetSystem->LearnPetSkill(p->bSkillBookSlotIndex, p->bSkillBookInvenIndex);
}

void CInputMain::GrowthPetSkillUpgradeRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetSkillUpgrade* p = (TPacketCGGrowthPetSkillUpgrade*)c_pData;

	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	pGrowthPetSystem->PetSkillUpgradeRequest(p->bslotPos, p->bslotIndex);
}

void CInputMain::GrowthPetSkillUpgrade(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetSkillUpgrade* p = (TPacketCGGrowthPetSkillUpgrade*)c_pData;

	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	pGrowthPetSystem->IncreasePetSkill(p->bslotPos, p->bslotIndex);
}

void CInputMain::GrowthPetFeedRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	if (ch->IsRefineWindowOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1486]You cannot feed your pet while the refinement window is open."));
		return;
	}

	const TPacketCGGrowthPetFeedRequest* p = (TPacketCGGrowthPetFeedRequest*)c_pData;

	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	const uint32_t dwPetEvolution = pGrowthPetSystem->GetEvolution() - 1;
	const uint32_t dwEvolveCount = sizeof(PET_EVOLVE_CUBE[dwPetEvolution]) / sizeof(PET_EVOLVE_CUBE[dwPetEvolution][1]);

	if (p->bIndex == PET_EXP_ITEM_WINDOW || p->bIndex == PET_EVOLVE_WINDOW)
	{
		if ((PET_EXP_ITEM_WINDOW == p->bIndex && GROWTHPET_MAX_LEVEL_CONST == pGrowthPetSystem->GetLevel())
			|| (PET_EVOLVE_WINDOW == p->bIndex && PET_MAX_EVOLVE == pGrowthPetSystem->GetEvolution())
			)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1170]Your pet has already reached the highest stage of evolution."));
			return;
		}
	}

	ch->GetGrowthPetSystem()->ItemCubeFeed(p->bIndex, p->iFeedItemsCubeSlot, p->iFeedItemsCount);

	// Send Result
	TPacketGCGrowthPetFeedResult pResult{};
	pResult.bHeader = HEADER_GC_GROWTH_PET_FEED_RESULT;
	pResult.bResult = true;

	if (ch->GetDesc())
		ch->GetDesc()->Packet(&pResult, sizeof(TPacketGCGrowthPetFeedResult));
}

void CInputMain::GrowthPetDeleteSkillRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetDeleteSkill* p = (TPacketCGGrowthPetDeleteSkill*)c_pData;
	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	pGrowthPetSystem->DeleteSkill(p->bSkillBookDelSlotIndex, p->bSkillBookDelInvenIndex);
}

void CInputMain::GrowthPetDeleteAllSkillRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetDeleteAllSkill* p = (TPacketCGGrowthPetDeleteAllSkill*)c_pData;
	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	pGrowthPetSystem->DeleteAllSkill(p->bPetSkillAllDelBookIndex);
}

void CInputMain::GrowthPetNameChangeRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetNameChange* p = (TPacketCGGrowthPetNameChange*)c_pData;

	LPITEM pPetMaterial = ch->GetInventoryItem(p->bItemWindowCell);
	if (!pPetMaterial)
	{
		sys_err("invalid pet change name item (cell: %d)", p->bItemWindowCell);
		return;
	}

	if ((pPetMaterial->GetType() != ITEM_PET) || (pPetMaterial->GetSubType() != PET_NAME_CHANGE))
	{
		sys_err("invalid pet change name item (%d %d)", pPetMaterial->GetType(), pPetMaterial->GetSubType());
		return;
	}

	const LPITEM pPetSeal = ch->GetInventoryItem(p->bPetWindowCell);
	if (!pPetSeal)
	{
		sys_err("invalid pet seal %d", p->bPetWindowCell);
		return;
	}

	if ((pPetSeal->GetType() != ITEM_PET) || (pPetSeal->GetSubType() != PET_UPBRINGING))
	{
		sys_err("invalid pet upbriging categories");
		return;
	}

	if (strlen(p->sPetName) < PET_NAME_MIN_SIZE || strlen(p->sPetName) > PET_NAME_MAX_SIZE)
	{
		TPacketGCGrowthPetNameChangeResult pResult{};
		pResult.bHeader = HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT;
		pResult.bResult = NAME_CHANGE_USE_FAILED_BECAUSE_NAME;

		if (ch->GetDesc())
		{
			ch->GetDesc()->Packet(&pResult, sizeof(TPacketGCGrowthPetNameChangeResult));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1171]The name entered is too short."));
		}
		return;
	}

	static char __pet_name[PET_NAME_MAX_SIZE * 2 + 1];
	DBManager::Instance().EscapeString(__pet_name, sizeof(__pet_name), p->sPetName, strnlen(p->sPetName, sizeof(p->sPetName)));
	if (!check_name(p->sPetName) || strncmp(__pet_name, p->sPetName, strnlen(p->sPetName, sizeof(p->sPetName))))
	{
		TPacketGCGrowthPetNameChangeResult pResult{};
		pResult.bHeader = HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT;
		pResult.bResult = NAME_CHANGE_USE_FAILED_BECAUSE_NAME;

		if (ch->GetDesc())
		{
			ch->GetDesc()->Packet(&pResult, sizeof(TPacketGCGrowthPetNameChangeResult));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1161]Please enter another name."));
		}
		return;
	}

	// Update name if summoned
	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	// Check pet evolution
	uint32_t dwPetMobVnum = pPetSeal->GetValue(0);

	const uint32_t dwPetEvolution = pPetSeal->GetGrowthPetItemInfo().evol_level;
	if (dwPetEvolution >= 4)
		dwPetMobVnum = pPetSeal->GetValue(3);

	const CGrowthPetSystemActor* petActor = pGrowthPetSystem->GetByVnum(dwPetMobVnum);
	if (petActor && petActor->GetGrowthPet())
	{
		pGrowthPetSystem->ChangeName(__pet_name);
	}
	else
	{
		TPacketGCGrowthPetNameChangeResult pResult{};
		pResult.bHeader = HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT;
		pResult.bResult = NAME_CHANGE_USE_SUCCESS;

		if (ch->GetDesc())
		{
			ch->GetDesc()->Packet(&pResult, sizeof(TPacketGCGrowthPetNameChangeResult));
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1343]");
		}
		return;
	}

	// Remove Material
	ITEM_MANAGER::Instance().RemoveItem(pPetMaterial, "PET_NAME_CHANGE");

	// Send Result
	TPacketGCGrowthPetNameChangeResult pResult{};
	pResult.bHeader = HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT;
	pResult.bResult = NAME_CHANGE_USE_SUCCESS;

	if (ch->GetDesc())
		ch->GetDesc()->Packet(&pResult, sizeof(TPacketGCGrowthPetNameChangeResult));
}

#	ifdef ENABLE_PET_ATTR_DETERMINE
void CInputMain::GrowthPetAttrDetermineRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	CGrowthPetSystem* pGrowthPetSystem = ch->GetGrowthPetSystem();
	if (!pGrowthPetSystem)
		return;

	if (!pGrowthPetSystem->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1343]");
		return;
	}

	pGrowthPetSystem->Determine();
}

void CInputMain::GrowthPetAttrChangeRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGGrowthPetAttrChange* p = (TPacketCGGrowthPetAttrChange*)c_pData;
	ch->PetAttrChange(p->bPetSlotIndex, p->bMaterialSlotIndex);
}
#	endif

#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
void CInputMain::GrowthPetReviveRequest(LPCHARACTER ch, const char* c_pData)
{
	const TPacketCGGrowthPetReviveRequest* p = reinterpret_cast<const TPacketCGGrowthPetReviveRequest*>(c_pData);

	if (!ch || !ch->GetDesc())
		return;

	LPITEM pUpBringingItem = ch->GetInventoryItem(p->upBringingPos.cell);
	if (!pUpBringingItem)
		return;

	if (pUpBringingItem->GetType() != ITEM_PET || pUpBringingItem->GetSubType() != PET_UPBRINGING)
		return;

	ch->RevivePet(p, pUpBringingItem);
}
#	endif
#endif

#ifdef ENABLE_CUBE_RENEWAL
size_t GetCubeRenewalSubPacketLength(const EPacketCGCubeRenewalSubHeader& SubHeader)
{
	switch (SubHeader)
	{
		case CUBE_RENEWAL_MAKE:
			return sizeof(TSubPacketCGCubeRenwalMake);
		case CUBE_RENEWAL_CLOSE_STATE:
			return sizeof(TSubPacketCGCubeRenwalClose);
	}

	return 0;
}

#include "CubeManager.h"
int CInputMain::CubeRenewalSend(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (uiBytes < sizeof(TPacketCGCubeRenewal))
		return -1;

	const TPacketCGCubeRenewal* pinfo = reinterpret_cast<const TPacketCGCubeRenewal*>(data);
	const char* c_pData = data + sizeof(TPacketCGCubeRenewal);

	uiBytes -= sizeof(TPacketCGCubeRenewal);

	const EPacketCGCubeRenewalSubHeader SubHeader = static_cast<EPacketCGCubeRenewalSubHeader>(pinfo->bSubHeader);
	const size_t SubPacketLength = GetCubeRenewalSubPacketLength(SubHeader);
	if (uiBytes < SubPacketLength)
	{
		sys_err("invalid CubeRenewalSend subpacket length (sublen %d size %u buffer %u)", SubPacketLength, sizeof(TPacketCGCubeRenewal), uiBytes);
		return -1;
	}

	switch (SubHeader)
	{
		case CUBE_RENEWAL_MAKE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "CUBE_RENEWAL_MAKE");
				sys_err("CUBE_RENEWAL_MAKE");
				const TSubPacketCGCubeRenwalMake* sp = reinterpret_cast<const TSubPacketCGCubeRenwalMake*>(c_pData);

				int index_item = sp->vnum;
				int count_item = sp->multiplier;
				int indexImprove = sp->indexImprove;
				//int index_item_improve = pinfo->index;

				ch->ChatPacket(CHAT_TYPE_INFO, "refinecube");
				std::vector<int> listOfReqItems = {};
				for (int i = 0; i < 5; ++i) {
					listOfReqItems.emplace_back(sp->itemReq[i]);
					ch->ChatPacket(CHAT_TYPE_INFO, "refinecube %d", sp->itemReq[i]);
				}

				CCubeManager::Instance().RefineCube(ch, index_item, count_item, indexImprove, listOfReqItems);
			}
			return SubPacketLength;

		case CUBE_RENEWAL_CLOSE_STATE:
			{
				CCubeManager::Instance().Cube_close(ch);
			}
			return SubPacketLength;

		default:
			return -1;
	}

	return 0;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
size_t GetAuraSubPacketLength(const EPacketCGAuraSubHeader& SubHeader)
{
	switch (SubHeader)
	{
		case AURA_SUBHEADER_CG_REFINE_CHECKIN:
			return sizeof(TSubPacketCGAuraRefineCheckIn);
		case AURA_SUBHEADER_CG_REFINE_CHECKOUT:
			return sizeof(TSubPacketCGAuraRefineCheckOut);
		case AURA_SUBHEADER_CG_REFINE_ACCEPT:
			return sizeof(TSubPacketCGAuraRefineAccept);
		case AURA_SUBHEADER_CG_REFINE_CANCEL:
			return 0;
	}

	return 0;
}

int CInputMain::Aura(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (!ch)
		return -1;

	if (uiBytes < sizeof(TPacketCGAura))
		return -1;

	const TPacketCGAura* pinfo = reinterpret_cast<const TPacketCGAura*>(data);
	const char* c_pData = data + sizeof(TPacketCGAura);

	uiBytes -= sizeof(TPacketCGAura);

	const EPacketCGAuraSubHeader SubHeader = static_cast<EPacketCGAuraSubHeader>(pinfo->bSubHeader);
	const size_t SubPacketLength = GetAuraSubPacketLength(SubHeader);
	if (uiBytes < SubPacketLength)
	{
		sys_err("invalid aura subpacket length (sublen %d size %u buffer %u)", SubPacketLength, sizeof(TPacketCGAura), uiBytes);
		return -1;
	}

	switch (SubHeader)
	{
		case AURA_SUBHEADER_CG_REFINE_CHECKIN:
			{
				const TSubPacketCGAuraRefineCheckIn* sp = reinterpret_cast<const TSubPacketCGAuraRefineCheckIn*>(c_pData);
				ch->AuraRefineWindowCheckIn(sp->byAuraRefineWindowType, sp->AuraCell, sp->ItemCell);
			}
			return SubPacketLength;
		case AURA_SUBHEADER_CG_REFINE_CHECKOUT:
			{
				const TSubPacketCGAuraRefineCheckOut* sp = reinterpret_cast<const TSubPacketCGAuraRefineCheckOut*>(c_pData);
				ch->AuraRefineWindowCheckOut(sp->byAuraRefineWindowType, sp->AuraCell);
			}
			return SubPacketLength;
		case AURA_SUBHEADER_CG_REFINE_ACCEPT:
			{
				const TSubPacketCGAuraRefineAccept* sp = reinterpret_cast<const TSubPacketCGAuraRefineAccept*>(c_pData);
				ch->AuraRefineWindowAccept(sp->byAuraRefineWindowType);
			}
			return SubPacketLength;
		case AURA_SUBHEADER_CG_REFINE_CANCEL:
			{
				ch->AuraRefineWindowClose();
			}
			return SubPacketLength;
	}

	return 0;
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH
void CInputMain::Attr67Send(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	struct SPacketAttr67Send* pinfo = (struct SPacketAttr67Send*)data;

	switch (pinfo->subheader)
	{
		case ATTR67_SUB_HEADER_CLOSE:
		{
			if (!ch->IsOpenSkillBookComb())
				return;

			ch->SetOpenSkillBookComb(false);
		}
		break;

		case ATTR67_SUB_HEADER_OPEN:
		{
			if (ch->IsOpenSkillBookComb())
				return;
			ch->SetOpenSkillBookComb(true);
		}
		break;

		case ATTR67_SUB_HEADER_SKILLBOOK_COMB:
		{
			CAttr6th7th::Instance().StartComb(ch, pinfo->bCell);
		}
		break;

		case ATTR67_SUB_HEADER_GET_FRAGMENT:
		{
			CAttr6th7th::Instance().CheckFragment(ch, pinfo->bRegistSlot);
		}
		break;

		case ATTR67_SUB_HEADER_ADD:
		{
			CAttr6th7th::Instance().AddAttr6th7th(ch, pinfo->bRegistSlot, pinfo->bFragmentCount, pinfo->wCellAdditive, pinfo->bCountAdditive);
		}
		break;

		default:
			sys_err("Invalid Attr67Send subheader: %u", pinfo->subheader);
			break;
	}
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
void CInputMain::ElementsSpell(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const struct SPacketCGElementsSpell* pinfo = (struct SPacketCGElementsSpell*)data;

	if (!ch->IsOpenElementsSpell())
		return;

	switch (pinfo->subheader)
	{
		case ELEMENTS_SPELL_SUB_HEADER_CLOSE:
		{
			ch->SetOpenElementsSpell(false);
		}
		break;

		case ELEMENTS_SPELL_SUB_HEADER_ADD_BONUS:
		{
			if (pinfo->pos < 0)
			{
				ch->SetOpenElementsSpell(false);
				return;
			}

			ch->ElementsSpellItemFunc(pinfo->pos);
			ch->SetOpenElementsSpell(false);
		}
		break;

		case ELEMENTS_SPELL_SUB_HEADER_CHANGE_BONUS:
		{
			if (pinfo->pos < 0)
			{
				ch->SetOpenElementsSpell(false);
				return;
			}

			const uint8_t type_select = pinfo->type_select;

			if (type_select < 99 || type_select > 104)
			{
				ch->SetOpenElementsSpell(false);
				return;
			}

			ch->ElementsSpellItemFunc(pinfo->pos, type_select);
			ch->SetOpenElementsSpell(false);
		}
		break;

		default:
		{
			ch->SetOpenElementsSpell(false);
			break;
		}
	}
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CInputMain::RemoveMyShopItem(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketMyShopRemoveItem* p = reinterpret_cast<const TPacketMyShopRemoveItem*>(c_pData);
	const auto myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(ch->GetPlayerID());
	if (!myShopChar)
		return;

	if (ch->GetShopItems().empty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_NOT_OPEN"));
		if (myShopChar)
		{
			sys_err("Player #%u should not have a shop open! Closing it down.", ch->GetPlayerID());
			myShopChar->CloseShop();
			M2_DESTROY_CHARACTER(myShopChar);
		}

		return;
	}

	//Player needs to be in the shop's map AND near the shop
	if (!myShopChar ||
		myShopChar->GetMapIndex() != ch->GetMapIndex() ||
		DISTANCE_APPROX(myShopChar->GetX() - ch->GetX(), myShopChar->GetY() - ch->GetY()) > 20000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_TAKE_ITEMS_FROM_MYSHOP_FROM_FAR_AWAY"));
		return;
	}

	//The item on the position we are removing must exist on the shop.
	LPITEM item = myShopChar->GetItem(TItemPos(PREMIUM_PRIVATE_SHOP, p->slot));
	if (!item)
		return;

	if (LPSHOP shop = myShopChar->GetMyShop())
	{
		if (shop->TransferItemAway(ch, (uint8_t)p->slot, p->target))
		{
			LogManager::Instance().ItemLog(ch, item, "TAKE_FROM_SHOP", item->GetName());

			//Remove from DB
			const uint32_t pid = ch->GetPlayerID();
			const uint8_t pos = (uint8_t)p->slot;

			if (!shop->IsEmpty())
			{
				db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t));
				const uint8_t subheader = SHOP_SUBHEADER_GD_REMOVE;
				db_clientdesc->Packet(&subheader, sizeof(uint8_t));
				db_clientdesc->Packet(&pid, sizeof(uint32_t));
				db_clientdesc->Packet(&pos, sizeof(uint8_t));
			}

			//Remove it at the player interface
			TPacketPlayerShopSet pack{};
			pack.header = HEADER_GC_PLAYER_SHOP_SET;
			pack.pos = (uint8_t)p->slot;

			pack.count = 0;
			pack.vnum = 0;
			pack.price = 0;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			pack.cheque = 0;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //ENABLE_PREMIUM_PRIVATE_SHOP_CHANGELOOK
			pack.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
			pack.grade_element = 0;
			memset(pack.attack_element, 0, sizeof(pack.attack_element));
			pack.element_type_bonus = 0;
			memset(pack.elements_value_bonus, 0, sizeof(pack.elements_value_bonus));
#endif

			memset(pack.alSockets, 0, sizeof(pack.alSockets));
			memset(pack.aAttr, 0, sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			memset(pack.aApplyRandom, 0, sizeof(pack.aApplyRandom));
			memset(pack.alRandomValues, 0, sizeof(pack.alRandomValues));
#endif

			ch->GetDesc()->Packet(&pack, sizeof(TPacketPlayerShopSet));
		}

		//Closed shop, destroy it (through owner)
		if (shop->IsClosed())
		{
			TPacketPlayerShopSign p{}; // Also sync the shop sign here
			p.header = HEADER_GC_MY_SHOP_SIGN;
			memset(p.sign, 0, sizeof(p.sign));
			ch->GetDesc()->Packet(&p, sizeof(TPacketPlayerShopSign));

			M2_DESTROY_CHARACTER(myShopChar);
			//myShopChar = nullptr;
		}

		ch->SaveReal();
		db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
		uint32_t pID = ch->GetPlayerID();
		db_clientdesc->Packet(&pID, sizeof(uint32_t));
	}
}

/*
* Add an item to the player's private shop, provided the player
* is close to it.
*/
void CInputMain::AddMyShopItem(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketMyShopAddItem* p = reinterpret_cast<const TPacketMyShopAddItem*>(c_pData);

	if (ch->GetShopItems().empty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_NOT_OPEN"));
		return;
	}

	if (p->from.window_type != INVENTORY && p->from.window_type != DRAGON_SOUL_INVENTORY)
	{
		sys_err("Ignoring my shop item transfer because the window is not valid (%d)", p->from.window_type);
		return;
	}

	LPCHARACTER myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(ch->GetPlayerID());

	if (!myShopChar)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOUR_MYSHOP_COULD_NOT_BE_FOUND_ON_THIS_CHANNEL"));
		return;
	}

	if (DISTANCE_APPROX(myShopChar->GetX() - ch->GetX(), myShopChar->GetY() - ch->GetY()) > 20000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_ITEMS_TO_MYSHOP_FROM_FAR_AWAY"));
		return;
	}

	// Player needs to be in the shop's map AND near the shop
	if (myShopChar->GetMapIndex() != ch->GetMapIndex())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_ITEMS_TO_MYSHOP_FROM_ANOTHER_MAP"));
		return;
	}

#ifndef ENABLE_OPEN_SHOP_ONLY_IN_MARKET
	if (SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(ch->GetMapIndex()) != ch->GetEmpire())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_ITEMS_TO_MYSHOP_FROM_OTHER_COUNTRY"));
		return;
	}
#endif

	// If current gold stash plus the price we want to sell this item for would go over max gold,
	// then we can't add the item to the shop.
	const uint32_t currentStash = ch->GetShopGoldStash();
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	const uint32_t currentChequeStash = ch->GetShopChequeStash();
#endif
	if (currentStash + p->price >= GOLD_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_THIS_ITEM_OR_YOU_WILL_EXCEED_MAX_GOLD_IN_SHOP_STASH"));
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (currentChequeStash + p->cheque >= CHEQUE_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_THIS_ITEM_OR_YOU_WILL_EXCEED_MAX_CHEQUE_IN_SHOP_STASH"));
		return;
	}
#endif

	// That was a preliminary stash check. Now let's take into account aaall the items we have in the shop.
	// If we sold all of them, could the gold still fit in within the stash limits?
	uint32_t finalStash = currentStash + p->price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	uint32_t finalChequeStash = currentChequeStash + p->cheque;
#endif

	LPSHOP shop = myShopChar->GetMyShop();
	if (!shop)
		return;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	if (shop->GetOfflineMinutes() <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_THIS_ITEM_BECAUSE_YOUR_SHOP_WAS_EXPIRED"));
		return;
	}
#endif

	for (const auto& sItem : shop->GetItemVector())
	{
		finalStash += sItem.price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		finalChequeStash += sItem.cheque;
#endif
	}

	if (finalStash >= GOLD_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_THIS_ITEM_OR_YOU_WILL_EXCEED_MAX_GOLD_IN_SHOP_STASH"));
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (finalChequeStash >= CHEQUE_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_ADD_THIS_ITEM_OR_YOU_WILL_EXCEED_MAX_CHEQUE_IN_SHOP_STASH"));
		return;
	}
#endif

	// The item on the position we are removing must exist on the player.
	LPITEM item = ch->GetItem(p->from);
	if (!item)
		return;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;813]");
		return;
	}

	if (item->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1060]");
		return;
	}

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1013]");
		return;
	}
#endif

	if (item->isLocked())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEMS_BEING_IN_USE_CAN_NOT_BE_SOLD_IN_SHOP"));
		return;
	}

	if (item->GetVnum() == 50200 || item->GetVnum() == 71049
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
		|| item->GetVnum() == 71221
#endif
		) // SHOP bundle never let this get into the shop!
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEMS_BEING_IN_USE_CAN_NOT_BE_SOLD_IN_SHOP"));
		return;
	}

	if (shop)
	{
		TShopItemTable t;
		t.vnum = item->GetVnum();
		t.count = static_cast<uint8_t>(item->GetCount());
		t.display_pos = p->targetPos;
		t.price = p->price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		t.cheque = p->cheque;
#endif
		t.pos = p->from;

		shop->TransferItems(ch, &t, 1);
		shop->SetShopItem(&t);

		shop->BroadcastUpdateItem(p->targetPos);

		// Optimizable: Only save the new item's data (Or all items), instead of performing a full save.
		shop->Save();

		sys_log(0, "Completed addition of %lu at pos %d on #%lu's shop", t.vnum, t.display_pos, ch->GetPlayerID());
	}
}


void CInputMain::OpenPlayerShop(LPCHARACTER ch)
{
	if (!ch)
		return;

#ifdef ENABLE_PRIVATESHOP_SYSTEM_ALWAYS_SILK
	ch->UseSilkBotary();
#else
	ch->OpenPrivateShop();
#endif
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
void CInputMain::ReopenPlayerShop(LPCHARACTER ch)
{
	if (!ch)
		return;

	if (ch->GetShopItems().empty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_NOT_OPEN"));
		return;
	}

	LPCHARACTER myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(ch->GetPlayerID());
	if (!myShopChar)
	{
		sys_err("cant found shop : %d", ch->GetPlayerID());
		return;
	}

	LPSHOP shop = myShopChar->GetMyShop();
	if (!shop)
	{
		sys_err("No shop on shop char for %u!", ch->GetPlayerID());
		return;
	}

	// Check Time
	if (shop->GetOfflineMinutes() > 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1713]");
		return;
	}

	// Check Item
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	if (!shop->IsShopDeco())
#endif
	{
		if (ch->CountSpecifyItem(50200) <= 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_NEED_PRIVATE_SHOP_PACKAGE_TO_REOPEN_PRIVATE_SHOP"));
			return;
		}

		// Remove Item
		ch->RemoveSpecifyItem(50200, 1);
	}
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	else
	{
		if (ch->CountSpecifyItem(71221) <= 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have the necessary material to reopen a private shop with decoration"));
			return;
		}
	}
#endif

	if (myShopChar->IsAffectFlag(AFF_INVISIBILITY))
	{
		// Remove Affect
		myShopChar->RemoveAffect(AFFECT_INVISIBILITY);

		shop->SetClosed(false);

		myShopChar->ViewReencode();
		ch->ViewReencode();

		// Update Offline Time
		shop->SetOfflineMinutes(ch->GetPremiumPrivateShopRemainSeconds());
		shop->SaveOffline();

		// Sync Offline Time
		if (ch->GetDesc())
		{
			TPacketGCShopOffTimeSync pack{};
			pack.bHeader = HEADER_GC_SYNC_SHOP_OFFTIME;
			pack.value = ch->GetPremiumPrivateShopRemainSeconds();
			ch->GetDesc()->Packet(&pack, sizeof(TPacketGCShopOffTimeSync));
		}
	}
}
#endif

/*
* Close the private shop completely.
*/
void CInputMain::ClosePlayerShop(LPCHARACTER ch)
{
	if (ch->GetShopItems().empty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_NOT_OPEN"));
		return;
	}

	LPCHARACTER myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(ch->GetPlayerID());

	// Player needs to be in the shop's map AND near the shop
	if (!myShopChar || myShopChar->GetMapIndex() != ch->GetMapIndex() ||
		DISTANCE_APPROX(myShopChar->GetX() - ch->GetX(), myShopChar->GetY() - ch->GetY()) > 20000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_CLOSE_YOUR_SHOP_FROM_FAR_AWAY"));
		return;
	}

	LPSHOP shop = myShopChar->GetMyShop();
	if (!shop)
	{
		sys_err("No shop on shop char for %u!", ch->GetPlayerID());
		return;
	}

	// Create a grid and fill it with the inventory.
	CGrid inventoryGrid1(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW);
	CGrid inventoryGrid2(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW);
	CGrid inventoryGrid3(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW);
	CGrid inventoryGrid4(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW);

	inventoryGrid1.Clear();
	inventoryGrid2.Clear();
	inventoryGrid3.Clear();
	inventoryGrid4.Clear();

	LPITEM item;

	int i;

	constexpr int perPageSlotCount = INVENTORY_PAGE_SIZE;

	for (i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (!(item = ch->GetInventoryItem(i)))
			continue;

		const uint8_t itemSize = item->GetSize();

		if (i < perPageSlotCount) // Notice: This is adjusted for 4 Pages only!
			inventoryGrid1.Put(i, 1, itemSize);
		else if (i < perPageSlotCount * 2)
			inventoryGrid2.Put(i - perPageSlotCount, 1, itemSize);
		else if (i < perPageSlotCount * 3)
			inventoryGrid3.Put(i - perPageSlotCount * 2, 1, itemSize);
		else
			inventoryGrid4.Put(i - perPageSlotCount * 3, 1, itemSize);
	}

	// Create a vector just with the valid items and then reverse sort it
	std::vector<LPITEM> shopItems;
	for (const auto& sItem : shop->GetItemVector())
	{
		if (sItem.pkItem)
			shopItems.emplace_back(sItem.pkItem);
	}

	std::sort(shopItems.begin(), shopItems.end(), [](const LPITEM& lhs, const LPITEM& rhs) { return lhs->GetSize() > rhs->GetSize(); });

	// Add (to the grid) the items that shop contains
	for (const auto shopItem : shopItems)
	{
		const int height = shopItem->GetSize();
		if (!shopItem->IsDragonSoul())
		{
			const int inv1Pos = inventoryGrid1.FindBlank(1, height);
			const int inv2Pos = inventoryGrid2.FindBlank(1, height);
			const int inv3Pos = inventoryGrid3.FindBlank(1, height);
			const int inv4Pos = inventoryGrid4.FindBlank(1, height);

			if (inv1Pos >= 0)
			{
				inventoryGrid1.Put(inv1Pos, 1, height);
			}
			else if (inv2Pos >= 0)
			{
				inventoryGrid2.Put(inv2Pos, 1, height);
			}
			else if (inv3Pos >= 0)
			{
				inventoryGrid3.Put(inv3Pos, 1, height);
			}
			else if (inv4Pos >= 0)
			{
				inventoryGrid4.Put(inv4Pos, 1, height);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1715]");
				return;
			}
		}
		else
		{
			// We can almost always place Dragon Soul(s)
		}
	}

	// We got here, all is OK! Transfer.
	// Must transfer in this order.
	for (const auto item : shopItems)
	{
		const uint8_t displayPos = (uint8_t)item->GetCell();

		if (item->IsDragonSoul())
		{
			const int pos = ch->GetEmptyDragonSoulInventory(item);
			if (pos < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1715]");
				//sys_err("Closing shop failed. Could not place DS of vnum %d", item->GetVnum());
				return;
			}

			if (!shop->TransferItemAway(ch, displayPos, TItemPos(DRAGON_SOUL_INVENTORY, pos)))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FAILED_TO_TRANSFER_ITEM_TO_INVENTORY"));
				return;
			}

		}
		else
		{
#ifdef ENABLE_SPECIAL_INVENTORY
			const int pos = ch->GetEmptyInventory(item);
#else
			const int pos = ch->GetEmptyInventory(item->GetSize());
#endif
			if (pos < 0)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1715]");
				//sys_err("Closing shop failed. Unexpected could not place item of size %d", item->GetSize());
				return;
			}

			if (!shop->TransferItemAway(ch, displayPos, TItemPos(INVENTORY, pos)))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("FAILED_TO_TRANSFER_ITEM_TO_INVENTORY"));
				return;
			}
		}

		LogManager::Instance().ItemLog(ch, item, "TAKE_FROM_SHOP", item->GetName());

		// Remove from DB (if not done already)
		if (!shop->IsEmpty())
		{
			const uint32_t pid = ch->GetPlayerID();
			const uint8_t pos = displayPos;

			db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t));
			const uint8_t subheader = SHOP_SUBHEADER_GD_REMOVE;
			db_clientdesc->Packet(&subheader, sizeof(uint8_t));
			db_clientdesc->Packet(&pid, sizeof(uint32_t));
			db_clientdesc->Packet(&pos, sizeof(uint8_t));
		}

		// Remove it at the player interface
		TPacketPlayerShopSet pack{};
		pack.header = HEADER_GC_PLAYER_SHOP_SET;
		pack.pos = displayPos;

		pack.count = 0;
		pack.vnum = 0;
		pack.price = 0;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		pack.cheque = 0;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //ENABLE_PREMIUM_PRIVATE_SHOP_CHANGELOOK
		pack.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
		pack.grade_element = 0;
		memset(pack.attack_element, 0, sizeof(pack.attack_element));
		pack.element_type_bonus = 0;
		memset(pack.elements_value_bonus, 0, sizeof(pack.elements_value_bonus));
#endif

		memset(pack.alSockets, 0, sizeof(pack.alSockets));
		memset(pack.aAttr, 0, sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
		memset(pack.aApplyRandom, 0, sizeof(pack.aApplyRandom));
		memset(pack.alRandomValues, 0, sizeof(pack.alRandomValues));
#endif

		ch->GetDesc()->Packet(&pack, sizeof(TPacketPlayerShopSet));
	}

	// Closed shop, destroy it (through owner)
	if (shop->IsClosed())
	{
		TPacketPlayerShopSign p{}; // Also sync the shop sign here
		p.header = HEADER_GC_MY_SHOP_SIGN;
		memset(p.sign, 0, sizeof(p.sign));
		ch->GetDesc()->Packet(&p, sizeof(TPacketPlayerShopSign));

		M2_DESTROY_CHARACTER(myShopChar);
		myShopChar = nullptr;
	}

	ch->SaveReal();
	db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
	uint32_t pID = ch->GetPlayerID();
	db_clientdesc->Packet(&pID, sizeof(uint32_t));
}

/*
* Withdraw gold from the shop stash (generated by sales)
*/
void CInputMain::WithdrawShopStash(LPCHARACTER ch, const TPacketCGShopWithdraw* pack)
{
	if (!ch)
		return;

	const uint32_t withdrawGoldAmount = pack->amount;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	const uint32_t withdrawChequeAmount = pack->chequeAmount;
#endif

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (withdrawGoldAmount <= 0 && withdrawChequeAmount <= 0)
		return;
#else
	if (withdrawGoldAmount <= 0)
		return;
#endif

	if (withdrawGoldAmount > ch->GetShopGoldStash())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_WITHDRAW_MORE_THAN_YOU_HAVE"));
		return;
	}

	if (ch->GetGold() + withdrawGoldAmount >= GOLD_MAX) // change get allowed gold to GOLD_MAX > GetAllowedGold
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT WITHDRAW %d GOLD_INVENTORY_LIMIT %d"), withdrawGoldAmount, (GOLD_MAX - 1));
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	if (withdrawChequeAmount > ch->GetShopChequeStash())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_WITHDRAW_MORE_THAN_YOU_HAVE2"));
		return;
	}

	if (ch->GetCheque() + withdrawChequeAmount >= CHEQUE_MAX)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT WITHDRAW %u CHEQUE_INVENTORY_LIMIT %u"), withdrawChequeAmount, (CHEQUE_MAX - 1));
		return;
	}
#endif

	const auto pid = ch->GetPlayerID();

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, ch->GetDesc()->GetHandle(), sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
#else
	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, ch->GetDesc()->GetHandle(), sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t));
#endif
	const uint8_t subheader = SHOP_SUBHEADER_GD_WITHDRAW;
	db_clientdesc->Packet(&subheader, sizeof(uint8_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
	db_clientdesc->Packet(&withdrawGoldAmount, sizeof(uint32_t));
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	db_clientdesc->Packet(&withdrawChequeAmount, sizeof(uint32_t));
#endif

}
void CInputMain::RenameShop(LPCHARACTER ch, TPacketGCShopRename* pack)
{
	if (!ch)
		return;

	const uint32_t pid = ch->GetPlayerID();

	char newName[SHOP_SIGN_MAX_LEN + 1] = { "\0" };
	strlcpy(&newName[0], &pack->sign[0], sizeof(newName));

	const LPCHARACTER& myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(pid);
	if (!myShopChar)
		return;

	if ((myShopChar->GetMapIndex() != ch->GetMapIndex()) || (DISTANCE_APPROX(myShopChar->GetX() - ch->GetX(), myShopChar->GetY() - ch->GetY()) > 20000))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_CHANGE_NAME_OF_MYSHOP_FROM_FAR_AWAY"));
		return;
	}

	LPSHOP shop = myShopChar->GetMyShop();

	if (!shop)
		return;

	const int lastRenamePulse = shop->GetRenamePulse();
	const int currentPulse = thecore_pulse();

	if (lastRenamePulse > currentPulse)
	{
		const int deltaInSeconds = ((lastRenamePulse / PASSES_PER_SEC(1)) - (currentPulse / PASSES_PER_SEC(1)));
		const int minutes = deltaInSeconds / 60;
		const int seconds = (deltaInSeconds - (minutes * 60));

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CAN_ONLY_CHANGE_YOUR_SHOP_NAME_ONCE_IN_60_MINUTES (%02d:%02d left)"), minutes, seconds);
		return;
	}

	//Don't create the shop if the title has invalid words in it (or is empty)
	std::string shopSign = &newName[0];

	shopSign.erase(std::remove_if(shopSign.begin(), shopSign.end(), [](char c) {
		return !isalnum(c) && c != ' ' && c != '+'; //< remove if this applies
		}), shopSign.end());

	if (shopSign.length() < 3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHOP_SIGN_INVALID_OR_TOO_SMALL"));
		return;
	}

	if (CBanwordManager::Instance().CheckString(shopSign.c_str(), shopSign.length() + 1)) // Check for banned words
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;812]");
		return;
	}

	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(newName));
	constexpr uint8_t subheader = SHOP_SUBHEADER_GD_RENAME;
	db_clientdesc->Packet(&subheader, sizeof(uint8_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
	db_clientdesc->Packet(shopSign.c_str(), sizeof(newName));
}

#ifdef ENABLE_MYSHOP_DECO_PREMIUM
void CInputMain::MyShopDecoState(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGMyShopDecoState* p = (TPacketCGMyShopDecoState*)c_pData;

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "Deco-State %d", p->bState);

	ch->SetMyPrivShopState(p->bState);
}

void CInputMain::MyShopDecoAdd(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGMyShopDecoAdd* p = (TPacketCGMyShopDecoAdd*)c_pData;

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "Deco-Server: Type %d Vnum %d", p->bType, p->dwPolyVnum);

	ch->SetMyPrivShopAdd(p->bType, p->dwPolyVnum);
	ch->OpenPrivateShop(2, false); // tabCount, cashItem
}
#endif
#endif

#ifdef ENABLE_CHANGED_ATTR
void CInputMain::ItemSelectAttr(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGItemSelectAttr* p = reinterpret_cast<const TPacketCGItemSelectAttr*>(c_pData);

	ch->SelectAttrResult(p->bNew, p->pItemPos);
}
#endif

#ifdef ENABLE_HWID_BAN
void CInputMain::HWIDManager(LPCHARACTER ch, const char* c_pData)
{
	TPacketCGHwidBan* p = (TPacketCGHwidBan*)c_pData;

	if (!ch)
		return;

	if (!c_pData)
		return;

	LPDESC d = ch->GetDesc();

	if (!d)
		return;

	if (!ch->IsGM())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> You are not a gamemaster.");
		return;
	}
	if (!p->mode)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> select an action");
		return;
	}
	if (strlen(p->szPlayer) == 0) 
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> Player Name can not be empty");
		return;
	}
	if (strlen(p->szReason) == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> Reason can not be empty");
		return;
	}

	//UNBAN
	int account_id = 0;
	if (p->mode == 1)
	{
		if (ch->GetGMLevel() <= GM_UNBAN_RIGHT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> You do not have permission!");
			return;
		}

		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), p->szPlayer, strlen(p->szPlayer));

		char szEscapeReason[512 * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeReason, sizeof(szEscapeReason), p->szReason, strlen(p->szReason));
		DBManager::Instance().DirectQuery("INSERT INTO log.hwid_ban_log (hwid, player, gm, reason, mode) VALUES ('NO_HWID_NEEDED', '%s', '%s', '%s', 'UNBAN')", szEscapeName, ch->GetName(), szEscapeReason);
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

	// BAN
	if (p->mode == 2)
	{
		if (ch->GetGMLevel() <= GM_BAN_RIGHT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> You do not have permission!");
			return;
		}

		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), p->szPlayer, strlen(p->szPlayer));
		auto pMsg(DBManager::Instance().DirectQuery("SELECT player.account_id FROM player WHERE LOWER(name) like LOWER('%s') LIMIT 1", szEscapeName));
		if (pMsg->Get()->uiNumRows == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "%s doesnt exist, search for playername", szEscapeName);
			return;
		}
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		str_to_number(account_id, row[0]);
		DBManager::Instance().DirectQuery("UPDATE account.account set status='BLOCK' where id='%d'", account_id);
		char szEscapeReason[512 * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeReason, sizeof(szEscapeReason), p->szReason, strlen(p->szReason));
		DBManager::Instance().DirectQuery("INSERT INTO log.hwid_ban_log (hwid, player, gm, reason, mode) VALUES ('NO_HWID_NEEDED', '%s', '%s', '%s', 'BAN')", szEscapeName, ch->GetName(), szEscapeReason);
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

	//HWID BAN
	if (p->mode == 3)
	{
		if (ch->GetGMLevel() <= GM_HWID_BAN_RIGHT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> You do not have permission!");
			return;
		}

		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), p->szPlayer, strlen(p->szPlayer));	
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
		char szEscapeReason[512 * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeReason, sizeof(szEscapeReason), p->szReason, strlen(p->szReason));
		DBManager::Instance().DirectQuery("INSERT INTO log.hwid_ban_log (hwid, player, gm, reason, mode) VALUES ('%s', '%s', '%s', '%s', 'HWID_BAN')", row2[0], szEscapeName, ch->GetName(), szEscapeReason);
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

	//DELETE HWID BAN
	if (p->mode == 4)
	{
		if (ch->GetGMLevel() <= GM_DELETE_HWID_BAN_RIGHT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> You do not have permission!");
			return;
		}

		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), p->szPlayer, strlen(p->szPlayer));
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

		char szEscapeReason[512 * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeReason, sizeof(szEscapeReason), p->szReason, strlen(p->szReason));
		DBManager::Instance().DirectQuery("INSERT INTO log.hwid_ban_log (hwid, player, gm, reason, mode) VALUES ('%s', '%s', '%s', '%s', 'DELETE_HWID_BAN')", row2[0], szEscapeName, ch->GetName(), szEscapeReason);
		ch->ChatPacket(CHAT_TYPE_INFO,"%s HWID unbanned (%s)", szEscapeName, row2[0]);
		return;
	}

	//TOTAL HWID BAN
	if (p->mode == 5)
	{
		if (ch->GetGMLevel() <= GM_TOTAL_HWID_BAN_RIGHT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "<HWID-Manager> You do not have permission!");
			return;
		}

		char szEscapeName[CHARACTER_NAME_MAX_LEN * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeName, sizeof(szEscapeName), p->szPlayer, strlen(p->szPlayer));
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

		char szEscapeReason[512 * 2 + 1];
		DBManager::Instance().EscapeString(szEscapeReason, sizeof(szEscapeReason), p->szReason, strlen(p->szReason));
		DBManager::Instance().DirectQuery("INSERT INTO log.hwid_ban_log (hwid, player, gm, reason, mode) VALUES ('%s', '%s', '%s', '%s', 'TOTAL_HWID_BAN')", row2[0], szEscapeName, ch->GetName(), szEscapeReason);
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
size_t GetTicketSystemSubPacketLength(const EPacketCGTicketSystemSubHeader& SubHeader)
{
	switch (SubHeader)
	{
		case TICKET_SUBHEADER_CG_OPEN:
			return sizeof(TSubPacketCGTicketOpen);
		case TICKET_SUBHEADER_CG_CREATE:
			return sizeof(TSubPacketCGTicketCreate);
		case TICKET_SUBHEADER_CG_REPLY:
			return sizeof(TSubPacketCGTicketReply);
		case TICKET_SUBHEADER_CG_ADMIN:
			return sizeof(TSubPacketCGTicketAdmin);
		case TICKET_SUBHEADER_CG_ADMIN_PAGE:
			return sizeof(TSubPacketCGTicketAdminChangePage);
	}

	return 0;
}

int CInputMain::TicketSystem(LPCHARACTER ch, const char* data, size_t uiBytes)
{
		if (uiBytes < sizeof(TPacketCGTicketSystem))
		return -1;

	const TPacketCGTicketSystem* pinfo = reinterpret_cast<const TPacketCGTicketSystem*>(data);
	const char* c_pData = data + sizeof(TPacketCGTicketSystem);

	uiBytes -= sizeof(TPacketCGTicketSystem);

	const EPacketCGTicketSystemSubHeader SubHeader = static_cast<EPacketCGTicketSystemSubHeader>(pinfo->bSubHeader);
	const size_t SubPacketLength = GetTicketSystemSubPacketLength(SubHeader);
	if (uiBytes < SubPacketLength)
	{
		sys_err("invalid TicketSystem subpacket length (sublen %d size %u buffer %u)", SubPacketLength, sizeof(TPacketCGTicketSystem), uiBytes);
		return -1;
	}

	switch (SubHeader)
	{
		case TICKET_SUBHEADER_CG_OPEN:
			{
				const TSubPacketCGTicketOpen* sp = reinterpret_cast<const TSubPacketCGTicketOpen*>(c_pData);
				CTicketSystem::Instance().Open(ch, sp->action, sp->mode, sp->ticked_id);
			}
			return SubPacketLength;

		case TICKET_SUBHEADER_CG_CREATE:
			{
				const TSubPacketCGTicketCreate* sp = reinterpret_cast<const TSubPacketCGTicketCreate*>(c_pData);
				CTicketSystem::Instance().Create(ch, sp->title, sp->content, sp->priority);
			}
			return SubPacketLength;

		case TICKET_SUBHEADER_CG_REPLY:
			{
				const TSubPacketCGTicketReply* sp = reinterpret_cast<const TSubPacketCGTicketReply*>(c_pData);
				CTicketSystem::Instance().Reply(ch, sp->ticked_id, sp->reply);
			}
			return SubPacketLength;

		case TICKET_SUBHEADER_CG_ADMIN:
			{
				const TSubPacketCGTicketAdmin* sp = reinterpret_cast<const TSubPacketCGTicketAdmin*>(c_pData);
				CTicketSystem::Instance().Action(ch, sp->action, sp->ticked_id, sp->char_name, sp->reason);
			}
			return SubPacketLength;

		case TICKET_SUBHEADER_CG_ADMIN_PAGE:
			{
				const TSubPacketCGTicketAdminChangePage* sp = reinterpret_cast<const TSubPacketCGTicketAdminChangePage*>(c_pData);
				CTicketSystem::Instance().ChangePage(ch, sp->iStartPage);
			}
			return SubPacketLength;
	}

	return 0;
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
void CInputMain::LanguageChange(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGLanguageChange* p = reinterpret_cast<const TPacketCGLanguageChange*>(c_pData);

	TPacketGDLangaugeChange packet;
	packet.dwAID = ch->GetDesc()->GetAccountTable().id;
	packet.bLanguage = p->bLanguage;
	packet.bState = true;

	db_clientdesc->DBPacketHeader(HEADER_GD_LANGUAGE_CHANGE, ch->GetDesc()->GetHandle(), sizeof(TPacketGDLangaugeChange));
	db_clientdesc->Packet(&packet, sizeof(TPacketGDLangaugeChange));

	ch->GetDesc()->SetLanguage(p->bLanguage);
}

void CInputMain::TargetLanguageRequest(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	const TPacketCGTargetLanguageRequest* p = reinterpret_cast<const TPacketCGTargetLanguageRequest*>(c_pData);
	uint8_t bLanguage = LANGUAGE_NONE;

	// First check this core's char container and then P2P
	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().FindPC(p->szName);
	if (pChar)
		bLanguage = pChar->GetDesc()->GetLanguage();
	else
	{
		CCI* pCCI = P2P_MANAGER::instance().Find(p->szName);
		if (!pCCI)
			return;

		bLanguage = pCCI->bLanguage;
	}

	TPacketGCTargetLanguageResult packet;
	packet.bHeader = HEADER_GC_TARGET_LANGUAGE_RESULT;
	packet.bLanguage = bLanguage;
	strlcpy(packet.szName, p->szName, sizeof(packet.szName));

	ch->GetDesc()->Packet(&packet, sizeof(TPacketGCTargetLanguageResult));
}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
int CInputMain::BiologManager(LPCHARACTER ch, const char* c_pData, size_t uiBytes)
{
	if (!ch)
		return -1;

	const TPacketCGBiologManagerAction* p = (TPacketCGBiologManagerAction*)c_pData;
	c_pData += sizeof(TPacketCGBiologManagerAction);

	CBiologSystem* pkBiologManager = ch->GetBiologManager();
	if (!pkBiologManager)
		return -1;

	return pkBiologManager->RecvClientPacket(p->bSubHeader, c_pData, uiBytes);
}
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
size_t GetMiniGameSubPacketLength(const EPacketCGMiniGameSubHeaderOkeyNormal& SubHeader)
{
	switch (SubHeader)
	{
		case SUBHEADER_CG_RUMI_START:
			return sizeof(TSubPacketCGMiniGameCardOpenClose);
		case SUBHEADER_CG_RUMI_EXIT:
			return 0;
		case SUBHEADER_CG_RUMI_DECKCARD_CLICK:
			return 0;
		case SUBHEADER_CG_RUMI_HANDCARD_CLICK:
			return sizeof(TSubPacketCGMiniGameHandCardClick);
		case SUBHEADER_CG_RUMI_FIELDCARD_CLICK:
			return sizeof(TSubPacketCGMiniGameFieldCardClick);
		case SUBHEADER_CG_RUMI_DESTROY:
			return sizeof(TSubPacketCGMiniGameDestroy);
	}

	return 0;
}

int CInputMain::MiniGameOkeyCard(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (uiBytes < sizeof(TPacketCGMiniGameOkeyCard))
		return -1;

	const TPacketCGMiniGameOkeyCard* pinfo = reinterpret_cast<const TPacketCGMiniGameOkeyCard*>(data);
	const char* c_pData = data + sizeof(TPacketCGMiniGameOkeyCard);

	uiBytes -= sizeof(TPacketCGMiniGameOkeyCard);

	const EPacketCGMiniGameSubHeaderOkeyNormal SubHeader = static_cast<EPacketCGMiniGameSubHeaderOkeyNormal>(pinfo->bSubHeader);
	const size_t SubPacketLength = GetMiniGameSubPacketLength(SubHeader);
	if (uiBytes < SubPacketLength)
	{
		sys_err("invalid minigame subpacket length (sublen %d size %u buffer %u)", SubPacketLength, sizeof(TPacketCGMiniGameOkeyCard), uiBytes);
		return -1;
	}

	switch (SubHeader)
	{
		case SUBHEADER_CG_RUMI_START:
			{
				const TSubPacketCGMiniGameCardOpenClose* sp = reinterpret_cast<const TSubPacketCGMiniGameCardOpenClose*>(c_pData);
				ch->Cards_open(sp->bSafeMode);
			}
			return SubPacketLength;

		case SUBHEADER_CG_RUMI_EXIT:
			{
				ch->CardsEnd();
			}
			return SubPacketLength;

		case SUBHEADER_CG_RUMI_DESTROY:
			{
				const TSubPacketCGMiniGameDestroy* sp = reinterpret_cast<const TSubPacketCGMiniGameDestroy*>(c_pData);
				ch->CardsDestroy(sp->index);
			}
			return SubPacketLength;

		case SUBHEADER_CG_RUMI_DECKCARD_CLICK:
			{
				ch->Cards_pullout();
			}
			return SubPacketLength;

		case SUBHEADER_CG_RUMI_HANDCARD_CLICK:
			{
				const TSubPacketCGMiniGameHandCardClick* sp = reinterpret_cast<const TSubPacketCGMiniGameHandCardClick*>(c_pData);
				ch->CardsAccept(sp->index);
			}
			return SubPacketLength;

		case SUBHEADER_CG_RUMI_FIELDCARD_CLICK:
			{
				const TSubPacketCGMiniGameFieldCardClick* sp = reinterpret_cast<const TSubPacketCGMiniGameFieldCardClick*>(c_pData);
				ch->CardsRestore(sp->index);
			}
			return SubPacketLength;
	}

	return 0;
}
#endif

#ifdef ENABLE_FISH_EVENT
int CInputMain::FishEvent(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	TPacketCGFishEvent * p = (TPacketCGFishEvent *) data;

	if (uiBytes < sizeof(TPacketCGFishEvent))
		return -1;

	const char * c_pData = data + sizeof(TPacketCGFishEvent);
	uiBytes -= sizeof(TPacketCGFishEvent);

	switch (p->bSubheader)
	{
		case FISH_EVENT_SUBHEADER_BOX_USE:
			{
				if (uiBytes < sizeof(uint8_t) + sizeof(uint8_t))
					return -1;

				uint8_t bWindow = *(c_pData++);
				uint8_t wCell = *(c_pData);

				sys_log(0, "INPUT: %s FISH_EVENT: USE_BOX", ch->GetName());
				
				ch->FishEventUseBox(TItemPos(bWindow, wCell));
				return (sizeof(uint8_t) + sizeof(uint8_t));
			}
			
		case FISH_EVENT_SUBHEADER_SHAPE_ADD:
			{
				if (uiBytes < sizeof(uint8_t))
					return -1;

				uint8_t shapePos = *c_pData;

				sys_log(0, "INPUT: %s FISH_EVENT: ADD_SHAPE", ch->GetName());
				
				ch->FishEventAddShape(shapePos);
				return sizeof(uint8_t);
			}

		default:
			sys_err("CInputMain::FishEvent : Unknown subheader %d : %s", p->bSubheader, ch->GetName());
			break;
	}

	return 0;
}
#endif

#ifdef ENABLE_FLOWER_EVENT
void CInputMain::FlowerEventSend(LPCHARACTER ch, const char* data)
{
	if (!ch)
		return;

	const struct packet_send_flower_event* pinfo = (struct packet_send_flower_event*)data;
	const uint8_t id = pinfo->id;

	if (id < 0 || id >= PART_FLOWER_MAX_NUM)
		return;

	ch->SendFlowerEventExchange(id);
}
#endif

#ifdef ENABLE_HUNTING_SYSTEM
int CInputMain::ReciveHuntingAction(LPCHARACTER ch, const char* c_pData)
{
	TPacketGCHuntingAction* p = (TPacketGCHuntingAction*)c_pData;

	switch (p->bAction)
	{
		// Open Windows
		case 1:
			ch->OpenHuntingWindowMain();
			break;

		// Select Type
		case 2:
			if (ch->GetQuestFlag("hunting_system.is_active") == -1)
			{
				if (ch->GetLevel() < ch->GetQuestFlag("hunting_system.level"))
					return 0;

				ch->SetQuestFlag("hunting_system.is_active", 1);
				ch->SetQuestFlag("hunting_system.type", p->dValue);
				ch->SetQuestFlag("hunting_system.count", 0);

				ch->OpenHuntingWindowMain();
			} 
			else 
				ch->ChatPacket(CHAT_TYPE_INFO, "Es ist noch eine Jagdmission offen.");
			break;

		// Open Reward Window
		case 3:
			ch->OpenHuntingWindowReward();
			break;

		// Recive Reward
		case 4:
			ch->ReciveHuntingRewards();
			break;

		default:
			break;
	}

	return 0;
}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
void CInputMain::LottoOpenWindow(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc())
		return;

	TPacketCGLotteryOpenings* p = (TPacketCGLotteryOpenings*)c_pData;
	switch (p->subheader)
	{
		case SUBHEADER_CG_OPEN_LOTTERY_WINDOW:
		{
			ch->OpenLottoWindow();
		}
		break;

		case SUBHEADER_CG_OPEN_RANKING_WINDOW:
		{
			ch->SendLottoRankingInfo();
		}
		break;

		default:
			break;
	}
}

void CInputMain::LottoBuyTicket(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc())
		return;

	int has_ticket = -1;
	int lotto_rows = 0;

	TPacketCGSendLottoNewTicket* p = (TPacketCGSendLottoNewTicket*)c_pData;

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM player.lotto_tickets WHERE player_id=%d and slot=%d", ch->GetPlayerID(), p->slot));
	MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
	str_to_number(has_ticket, row[0]);

	if (has_ticket == 0)
	{
		if (ch->GetGold() < LOTTO_TICKET_COST)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Du hast nicht genug Yang.");
			return;
		}

		std::unique_ptr<SQLMsg> pMsg2(DBManager::instance().DirectQuery("SELECT COUNT(*) FROM player.lotto_numbers"));
		MYSQL_ROW row2 = mysql_fetch_row(pMsg2->Get()->pSQLResult);
		str_to_number(lotto_rows, row2[0]);

		ch->PointChange(POINT_GOLD, -LOTTO_TICKET_COST);

		int add_to_jackpot = LOTTO_TICKET_COST / 100 * LOTTO_PUT_TICKET_COST_PERC_TO_JACKPOT;
		DBManager::instance().DirectQuery("SET @max_values := (SELECT MAX(lotto_id) FROM lotto_numbers); UPDATE lotto_numbers SET next_jackpot=next_jackpot + %d WHERE lotto_id=@max_values", add_to_jackpot);

		DBManager::instance().DirectQuery("INSERT INTO player.lotto_tickets (ticket_id, player_id, player_name, slot, number1, number2, number3, number4, for_lotto_id, buytime, state, money_win) VALUES (0, %d, '%s', %d, %d, %d, %d, %d, %d, NOW(), 0, 0)", ch->GetPlayerID(), ch->GetName(), p->slot, p->num1, p->num2, p->num3, p->num4, lotto_rows+1);
		ch->ChatPacket(CHAT_TYPE_INFO, "Ticket erfolgreich hinzugef?t!");
		ch->SendLottoTicketInfo();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Auf dem Slot existiert bereits ein Ticket!");
	}
}

void CInputMain::LottoTicketOptions(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc())
		return;

	TPacketCGSendTicketOptions* p = (TPacketCGSendTicketOptions*)c_pData;
	switch (p->subheader)
	{
		case SUBHEADER_CG_TICKET_DELETE:
		{
			int state = 0;
			int money_win = 0;

			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT ticket_id, state, money_win FROM player.lotto_tickets WHERE player_id = %d and slot = %d", ch->GetPlayerID(), p->ticketSlot));
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

			if (pMsg->Get()->uiNumRows > 0)
			{
				str_to_number(state, row[1]);
				str_to_number(money_win, row[2]);

				if(money_win > 0 and state == 1) {
					ch->ChatPacket(CHAT_TYPE_INFO, "Du musst dein Gewinn erst abholen bevor du dieses Ticket l?chen kannst!");
					return;
				}

				if(state == 0 or state == 2)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Du hast das Ticket auf Slot %d erfolgreich gel?cht!", p->ticketSlot);
					DBManager::instance().DirectQuery("DELETE from player.lotto_tickets where player_id='%d' and slot='%d'", ch->GetPlayerID(), p->ticketSlot);
					ch->SendLottoTicketInfo();
				}
			}
		}
		break;

		case SUBHEADER_CG_RECIVE_MONEY:
		{
			int state = 0;
			int money_win = 0;

			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT ticket_id, state, money_win FROM player.lotto_tickets WHERE player_id = %d and slot = %d", ch->GetPlayerID(), p->ticketSlot));
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

			if (pMsg->Get()->uiNumRows > 0)
			{
				str_to_number(state, row[1]);

				if(state == 0){
					ch->ChatPacket(CHAT_TYPE_INFO, "Dieser Lottoschein wurde noch nicht ausgewertet.");
					return;
				}
				if(state == 2 and atoll(row[2]) > 0){
					ch->ChatPacket(CHAT_TYPE_INFO, "Du hast bereits das Geld abgeholt.");
					return;
				}
				if(state == 2 and atoll(row[2]) == 0){
					ch->ChatPacket(CHAT_TYPE_INFO, "Du hast leider nichts gewonnen. Du kannst daher nichts abholen.");
					return;
				}

				if(state == 1 and atoll(row[2]) > 0) {
					DBManager::instance().DirectQuery("UPDATE player.lotto_tickets SET state=2 WHERE player_id='%d' and slot='%d'", ch->GetPlayerID(), p->ticketSlot);

					ch->PointChange(POINT_LOTTO_MONEY, atoll(row[2]));
					ch->PointChange(POINT_LOTTO_TOTAL_MONEY, atoll(row[2]));
					ch->SendLottoTicketInfo();

					ch->ChatPacket(CHAT_TYPE_INFO, "Du hast deinen Gewinn abgeholt und liegt nun in deinem Yangspeicher!");
					return;
				}
			}
		}
		break;

		default:
			break;
	}
}

void CInputMain::LottoPickMoney(LPCHARACTER ch, const char* c_pData)
{
	if (!ch->GetDesc())
		return;

	TPacketCGSendLottoPickMoney* p = (TPacketCGSendLottoPickMoney*)c_pData;

	if (ch->GetGold() < GOLD_MAX)
	{
		if (ch->GetLottoMoney() >= p->amount)
		{
			ch->PointChange(POINT_LOTTO_MONEY, -p->amount);
			ch->PointChange(POINT_GOLD, p->amount);
			ch->ChatPacket(CHAT_TYPE_INFO, "Du hast %lld Yang abgehoben.", p->amount);
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Du hast nicht so viel Yang im Yang-Speicher.");	
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Du hast zu viel Yang im Inventar.");
}
#endif

int CInputMain::Analyze(LPDESC d, uint8_t bHeader, const char* c_pData)
{
	if (!d)
		return -1;

	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		d->SetPhase(PHASE_CLOSE);
		return (0);
	}

	int iExtraLen = 0;

	if (test_server && bHeader != HEADER_CG_MOVE)
		sys_log(0, "CInputMain::Analyze() ==> Header [%d] ", bHeader);

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_CHAT:
			if (test_server)
			{
				char* pBuf = (char*)c_pData;
				sys_log(0, "%s", pBuf + sizeof(TPacketCGChat));
			}

			if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_WHISPER:
			if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_MOVE:
			Move(ch, c_pData);
			// @fixme103 (removed CheckClientVersion since useless in here)
			break;

		case HEADER_CG_CHARACTER_POSITION:
			Position(ch, c_pData);
			break;

		case HEADER_CG_ITEM_USE:
			if (!ch->IsObserverMode())
				ItemUse(ch, c_pData);
			break;

		case HEADER_CG_ITEM_DROP:
			if (!ch->IsObserverMode())
			{
				ItemDrop(ch, c_pData);
			}
			break;

		case HEADER_CG_ITEM_DROP2:
			if (!ch->IsObserverMode())
				ItemDrop2(ch, c_pData);
			break;

#ifdef ENABLE_DESTROY_SYSTEM
		case HEADER_CG_ITEM_DESTROY:
			if (!ch->IsObserverMode())
				ItemDestroy(ch, c_pData);
			break;
#endif

		case HEADER_CG_ITEM_MOVE:
			if (!ch->IsObserverMode())
				ItemMove(ch, c_pData);
			break;

		case HEADER_CG_ITEM_PICKUP:
			if (!ch->IsObserverMode())
				ItemPickup(ch, c_pData);
			break;

		case HEADER_CG_ITEM_USE_TO_ITEM:
			if (!ch->IsObserverMode())
				ItemToItem(ch, c_pData);
			break;

		case HEADER_CG_ITEM_GIVE:
			if (!ch->IsObserverMode())
				ItemGive(ch, c_pData);
			break;

		case HEADER_CG_EXCHANGE:
			if (!ch->IsObserverMode())
				Exchange(ch, c_pData);
			break;

		case HEADER_CG_ATTACK:
		case HEADER_CG_SHOOT:
			if (!ch->IsObserverMode())
			{
				Attack(ch, bHeader, c_pData);
			}
			break;

		case HEADER_CG_USE_SKILL:
			if (!ch->IsObserverMode())
				UseSkill(ch, c_pData);
			break;
		
#ifdef ENABLE_CUBE_RENEWAL
		case HEADER_CG_CUBE_RENEWAL:
			if ((iExtraLen = CubeRenewalSend(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
		case HEADER_CG_SKILL_COLOR:
			SetSkillColor(ch, c_pData);
			break;
#endif

		case HEADER_CG_QUICKSLOT_ADD:
			QuickslotAdd(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_DEL:
			QuickslotDelete(ch, c_pData);
			break;

		case HEADER_CG_QUICKSLOT_SWAP:
			QuickslotSwap(ch, c_pData);
			break;

#ifdef ENABLE_SPECIAL_INVENTORY
		case HEADER_CG_SHOP:
			Shop(ch, c_pData);
			break;
#else
		case HEADER_CG_SHOP:
			if ((iExtraLen = Shop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

		case HEADER_CG_MESSENGER:
			if ((iExtraLen = Messenger(ch, c_pData, m_iBufferLeft))<0)
				return -1;
			break;

		case HEADER_CG_ON_CLICK:
			OnClick(ch, c_pData);
			break;

		case HEADER_CG_SYNC_POSITION:
			if ((iExtraLen = SyncPosition(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_ADD_FLY_TARGETING:
		case HEADER_CG_FLY_TARGETING:
			FlyTarget(ch, c_pData, bHeader);
			break;

		case HEADER_CG_SCRIPT_BUTTON:
			ScriptButton(ch, c_pData);
			break;

			// SCRIPT_SELECT_ITEM
		case HEADER_CG_SCRIPT_SELECT_ITEM:
			ScriptSelectItem(ch, c_pData);
			break;
			// END_OF_SCRIPT_SELECT_ITEM

		case HEADER_CG_SCRIPT_ANSWER:
			ScriptAnswer(ch, c_pData);
			break;

		case HEADER_CG_QUEST_INPUT_STRING:
			QuestInputString(ch, c_pData);
			break;

#ifdef ENABLE_OX_RENEWAL
		case HEADER_CG_QUEST_INPUT_LONG_STRING:
			QuestInputLongString(ch, c_pData);
			break;
#endif

		case HEADER_CG_QUEST_CONFIRM:
			QuestConfirm(ch, c_pData);
			break;

		case HEADER_CG_TARGET:
			Target(ch, c_pData);
			break;

		case HEADER_CG_WARP:
			Warp(ch, c_pData);
			break;

		case HEADER_CG_SAFEBOX_CHECKIN:
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			SafeboxCheckin(ch, c_pData, 0);
#else
			SafeboxCheckin(ch, c_pData);
#endif
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case HEADER_CG_GUILDSTORAGE_CHECKIN:
			SafeboxCheckin(ch, c_pData, 2);
			break;
#endif
			

		case HEADER_CG_SAFEBOX_CHECKOUT:
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			SafeboxCheckout(ch, c_pData, 0);
#else
			SafeboxCheckout(ch, c_pData, false);
#endif
			break;

		case HEADER_CG_SAFEBOX_ITEM_MOVE:
			SafeboxItemMove(ch, c_pData);
			break;

		case HEADER_CG_MALL_CHECKOUT:
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			SafeboxCheckout(ch, c_pData, 1);
#else
			SafeboxCheckout(ch, c_pData, true);
#endif
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case HEADER_CG_GUILDSTORAGE_CHECKOUT:
			SafeboxCheckout(ch, c_pData, 2);
			break;
#endif

#ifdef ENABLE_MOVE_CHANNEL
		case HEADER_CG_MOVE_CHANNEL:
			MoveChannel(ch, c_pData);
			break;
#endif

		case HEADER_CG_PARTY_INVITE:
			PartyInvite(ch, c_pData);
			break;

		case HEADER_CG_PARTY_REMOVE:
			PartyRemove(ch, c_pData);
			break;

		case HEADER_CG_PARTY_INVITE_ANSWER:
			PartyInviteAnswer(ch, c_pData);
			break;

		case HEADER_CG_PARTY_SET_STATE:
			PartySetState(ch, c_pData);
			break;

		case HEADER_CG_PARTY_USE_SKILL:
			PartyUseSkill(ch, c_pData);
			break;

		case HEADER_CG_PARTY_PARAMETER:
			PartyParameter(ch, c_pData);
			break;

#ifdef ENABLE_SAFEBOX_MONEY
		case HEADER_CG_SAFEBOX_MONEY:
			SafeboxMoney(ch, c_pData);
			break;
#endif

#ifdef ENABLE_INGAME_WIKI
		case HEADER_CG_WIKI:
			Wikipedia(ch, c_pData);
			break;
#endif

		case HEADER_CG_ANSWER_MAKE_GUILD:
#ifdef ENABLE_NEWGUILDMAKE
			ch->ChatPacket(CHAT_TYPE_INFO, "<%s> AnswerMakeGuild disabled", __FUNCTION__);
#else
			AnswerMakeGuild(ch, c_pData);
#endif
			break;

		case HEADER_CG_GUILD:
			if ((iExtraLen = Guild(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_FISHING:
			Fishing(ch, c_pData);
			break;

		case HEADER_CG_HACK:
			Hack(ch, c_pData);
			break;

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
		case HEADER_CG_DUNGEON_INFO:
			DungeonInfo(ch, c_pData);
			break;
#endif

#ifdef ENABLE_TICKET_SYSTEM
		case HEADER_CG_TICKET_SYSTEM:
			if ((iExtraLen = TicketSystem(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

		case HEADER_CG_MYSHOP:
			if ((iExtraLen = MyShop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_REFINE:
			Refine(ch, c_pData);
			break;

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		case HEADER_CG_GEM_SHOP:
			if ((iExtraLen = GemShop(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_SCRIPT_SELECT_ITEM_EX:
			ScriptSelectItemEx(ch, c_pData);
			break;
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
		case HEADER_CG_PRIVATE_SHOP_SEARCH_SEARCH:
			PrivateShopSearchSearch(ch, c_pData);
			break;

		case HEADER_CG_PRIVATE_SHOP_SEARCH_BUY:
			PrivateShopSearchBuy(ch, c_pData);
			break;

		case HEADER_CG_PRIVATE_SHOP_SEARCH_CLOSE:
			PrivateShopSearchClose(ch, c_pData);
			break;
#endif

		case HEADER_CG_CLIENT_VERSION:
			Version(ch, c_pData);
			break;

#ifdef ENABLE_SEND_TARGET_INFO
		case HEADER_CG_TARGET_INFO_LOAD:
		{
			TargetInfoLoad(ch, c_pData);
		}
		break;
#endif

		case HEADER_CG_DRAGON_SOUL_REFINE:
			{
				TPacketCGDragonSoulRefine* p = reinterpret_cast <TPacketCGDragonSoulRefine*>((void*)c_pData);
				switch(p->bSubType)
				{
					case DS_SUB_HEADER_CLOSE:
						ch->DragonSoul_RefineWindow_Close();
						break;

					case DS_SUB_HEADER_DO_REFINE_GRADE:
					{
						DSManager::Instance().DoRefineGrade(ch, p->ItemGrid);
					}
					break;

					case DS_SUB_HEADER_DO_REFINE_STEP:
					{
						DSManager::Instance().DoRefineStep(ch, p->ItemGrid);
					}
					break;

					case DS_SUB_HEADER_DO_REFINE_STRENGTH:
					{
						DSManager::Instance().DoRefineStrength(ch, p->ItemGrid);
					}
					break;

#ifdef ENABLE_DS_CHANGE_ATTR
					case DS_SUB_HEADER_DO_CHANGE_ATTR:
					{
						DSManager::Instance().DoChangeAttr(ch, p->ItemGrid);
					}
					break;
#endif

					default:
						break;
				}
			}
			break;

#ifdef ENABLE_HWID_BAN
		case HEADER_CG_HWID_SYSTEM:
			HWIDManager(ch, c_pData);
			break;
#endif

#ifdef ENABLE_SWITCHBOT
		case HEADER_CG_SWITCHBOT:
			if ((iExtraLen = Switchbot(ch, c_pData, m_iBufferLeft)) < 0)
			{
				return -1;
			}
			break;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		case HEADER_CG_LANGUAGE_CHANGE:
			LanguageChange(ch, c_pData);
			break;

		case HEADER_CG_TARGET_LANGUAGE_REQUEST:
			TargetLanguageRequest(ch, c_pData);
			break;
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
		case HEADER_CG_BIOLOG_MANAGER:
			if ((iExtraLen = BiologManager(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case HEADER_CG_REQUEST_ENTER_BATTLE:
			CBattleField::Instance().RequestEnter(ch);
			break;

		case HEADER_CG_REQUEST_EXIT_BATTLE:
			CBattleField::Instance().RequestExit(ch);
			break;
#endif

#ifdef ENABLE_ATTR_6TH_7TH
		case HEADER_CG_ATTR_6TH_7TH:
			Attr67Send(ch, c_pData);
			break;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case HEADER_CG_MYSHOP_REMOVE_ITEM:
			RemoveMyShopItem(ch, c_pData);
			break;

		case HEADER_CG_MYSHOP_ADD_ITEM:
			AddMyShopItem(ch, c_pData);
			break;
		case HEADER_CG_MYSHOP_OPEN:
			OpenPlayerShop(ch);
			break;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
		case HEADER_CG_MYSHOP_REOPEN:
			ReopenPlayerShop(ch);
			break;
#endif
		case HEADER_CG_MYSHOP_CLOSE:
			ClosePlayerShop(ch);
			break;

		case HEADER_CG_MYSHOP_WITHDRAW:
			WithdrawShopStash(ch, reinterpret_cast<const TPacketCGShopWithdraw*>(c_pData));
			break;
		case HEADER_CG_MYSHOP_RENAME:
			RenameShop(ch, (TPacketGCShopRename*)c_pData);
			break;

#ifdef ENABLE_MYSHOP_DECO
		case HEADER_CG_MYSHOP_DECO_STATE:
			MyShopDecoState(ch, c_pData);
			break;

		case HEADER_CG_MYSHOP_DECO_ADD:
			MyShopDecoAdd(ch, c_pData);
			break;
#endif
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case HEADER_CG_ACHIEVEMENT:
			CAchievementSystem::Instance().ProcessClientPackets(ch, c_pData);
			break;
#endif

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
		case HEADER_CG_WON_EXCHANGE:
			WonExchange(ch, c_pData);
			break;
#endif

#ifdef ENABLE_REFINE_ELEMENT
		case HEADER_CG_ELEMENTS_SPELL:
			ElementsSpell(ch, c_pData);
			break;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		case HEADER_CG_ACCE_CLOSE_REQUEST:
			AcceCloseRequest(ch, c_pData);
			break;

		case HEADER_CG_ACCE_REFINE_REQUEST:
			AcceRefineRequest(ch, c_pData);
			break;
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
		case HEADER_CG_COSTUME_REFINE_REQUEST:
			MoveCostumeAttrRequest(ch, c_pData);
			break;
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		case HEADER_CG_EXTEND_INVEN_REQUEST:
			ExtendInvenRequest(ch, c_pData);
			break;

		case HEADER_CG_EXTEND_INVEN_UPGRADE:
			ExtendInvenUpgrade(ch, c_pData);
			break;
#endif

#ifdef ENABLE_AURA_SYSTEM
		case HEADER_CG_AURA:
			if ((iExtraLen = Aura(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_CHANGED_ATTR
		case HEADER_CG_ITEM_SELECT_ATTR:
			ItemSelectAttr(ch, c_pData);
			break;
#endif

#ifdef ENABLE_MAILBOX
		case HEADER_CG_MAILBOX_WRITE:
			MailboxWrite(ch, c_pData);
			break;

		case HEADER_CG_MAILBOX_WRITE_CONFIRM:
			MailboxConfirm(ch, c_pData);
			break;

		case HEADER_CG_MAILBOX_PROCESS:
			MailboxProcess(ch, c_pData);
			break;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		case HEADER_CG_CHANGE_LOOK:
			Transmutation(ch, c_pData);
			break;
#endif

#ifdef ENABLE_PARTY_MATCH
		case HEADER_CG_PARTY_MATCH:
			if (!ch->IsObserverMode())
				PartyMatch(ch, c_pData);
			break;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case HEADER_CG_EXT_BATTLE_PASS_ACTION:
			if ((iExtraLen = ReciveExtBattlePassActions(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_EXT_SEND_BP_PREMIUM:
			if ((iExtraLen = ReciveExtBattlePassPremium(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		case HEADER_CG_GROWTH_PET:
			if ((iExtraLen = GrowthPet(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

			// Pet Hatching Request
		case HEADER_CG_GROWTH_PET_HATCHING:
			GrowthPetHatching(ch, c_pData);
			break;
			// Pet Learn Skill Request
		case HEADER_CG_GROWTH_PET_LEARN_SKILL:
			GrowthPetLearnSkill(ch, c_pData);
			break;
			// Pet Skill Upgrade Request
		case HEADER_CG_GROWTH_PET_SKILL_UPGRADE:
			GrowthPetSkillUpgrade(ch, c_pData);
			break;
		case HEADER_CG_GROWTH_PET_SKILL_UPGRADE_REQUEST:
			GrowthPetSkillUpgradeRequest(ch, c_pData);
			break;
		case HEADER_CG_GROWTH_PET_FEED_REQUEST:
			GrowthPetFeedRequest(ch, c_pData);
			break;
		case HEADER_CG_GROWTH_PET_DELETE_SKILL:
			GrowthPetDeleteSkillRequest(ch, c_pData);
			break;
		case HEADER_CG_GROWTH_PET_ALL_DEL_SKILL:
			GrowthPetDeleteAllSkillRequest(ch, c_pData);
			break;
		case HEADER_CG_GROWTH_PET_NAME_CHANGE:
			GrowthPetNameChangeRequest(ch, c_pData);
			break;
#	ifdef ENABLE_PET_ATTR_DETERMINE
		case HEADER_CG_GROWTH_PET_ATTR_DETERMINE:
			GrowthPetAttrDetermineRequest(ch, c_pData);
			break;
		case HEADER_CG_GROWTH_PET_ATTR_CHANGE:
			GrowthPetAttrChangeRequest(ch, c_pData);
			break;
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
		case HEADER_CG_GROWTH_PET_REVIVE_REQUEST:
			GrowthPetReviveRequest(ch, c_pData);
			break;
#	endif
#endif

#ifdef ENABLE_EVENT_MANAGER
		case HEADER_CG_REQUEST_EVENT_QUEST:
			RequestEventQuest(ch, c_pData);
			break;

		case HEADER_CG_REQUEST_EVENT_DATA:
			RequestEventData(ch, c_pData);
			break;
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		case HEADER_CG_OKEY_CARD:
			if ((iExtraLen = MiniGameOkeyCard(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif
#ifdef ENABLE_MONSTER_BACK
		case HEADER_CG_ATTENDANCE_REWARD:
			CMiniGameManager::Instance().AttendanceEventRequestReward(ch);
			break;
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
		case HEADER_CG_MINI_GAME_CATCH_KING:
			if ((iExtraLen = CMiniGameManager::Instance().MiniGameCatchKing(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif
#ifdef ENABLE_FISH_EVENT
		case HEADER_CG_FISH_EVENT_SEND:
			if ((iExtraLen = FishEvent(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		case HEADER_CG_SOUL_ROULETTE:
			if (!ch->IsObserverMode())
				SoulRoulette(ch, c_pData);
			break;
#endif

#ifdef ENABLE_MINI_GAME_BNW
		case HEADER_CG_MINI_GAME_BNW:
			if ((iExtraLen = CMiniGameManager::Instance().MiniGameBNW(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_MINI_GAME_FINDM
		case HEADER_CG_MINI_GAME_FIND_M:
			if ((iExtraLen = CMiniGameManager::instance().MiniGameFindM(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
		case HEADER_CG_MINI_GAME_YUT_NORI:
			if ((iExtraLen = CMiniGameManager::instance().MiniGameYutNori(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#endif

#ifdef ENABLE_FLOWER_EVENT
		case HEADER_CG_FLOWER_EVENT_EXCHANGE:
			FlowerEventSend(ch, c_pData);
			break;
#endif

#ifdef ENABLE_HUNTING_SYSTEM
		case HEADER_CG_SEND_HUNTING_ACTION:
			ReciveHuntingAction(ch, c_pData);
			break;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		case HEADER_CG_LOTTO_OPENINGS:
			LottoOpenWindow(ch, c_pData);
			break;

		case HEADER_CG_LOTTO_BUY_TICKET:
			LottoBuyTicket(ch, c_pData);
			break;

		case HEADER_CG_LOTTO_TICKET_OPTIONS:
			LottoTicketOptions(ch, c_pData);
			break;

		case HEADER_CG_LOTTO_PICK_MONEY:
			LottoPickMoney(ch, c_pData);
			break;
#endif

#ifdef ENABLE_FISHING_RENEWAL
		case HEADER_CG_FISHING_NEW:
			FishingNew(ch, c_pData);
			break;
#endif
	}

	return (iExtraLen);
}

int CInputDead::Analyze(LPDESC d, uint8_t bHeader, const char* c_pData)
{
	if (!d)
		return -1;

	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		sys_err("no character on desc");
		return 0;
	}

	int iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_CHAT:
			if ((iExtraLen = Chat(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;

			break;

		case HEADER_CG_WHISPER:
			if ((iExtraLen = Whisper(ch, c_pData, m_iBufferLeft)) < 0)
				return -1;

			break;

		case HEADER_CG_HACK:
			Hack(ch, c_pData);
			break;

		default:
			return (0);
	}

	return (iExtraLen);
}

#ifdef ENABLE_EVENT_MANAGER
void CInputMain::RequestEventQuest(LPCHARACTER ch, const void* c_pData)
{
	const TPacketCGRequestEventQuest* p = (TPacketCGRequestEventQuest*)c_pData;

	if (ch && ch->GetDesc())
		quest::CQuestManager::Instance().RequestEventQuest(p->szName, ch->GetPlayerID());
}

void CInputMain::RequestEventData(LPCHARACTER ch, const char* c_pData)
{
	if (ch && ch->GetDesc())
	{
		const TPacketCGRequestEventData* p = reinterpret_cast<const TPacketCGRequestEventData*>(c_pData);
		CEventManager::Instance().SendEventInfo(ch, p->bMonth);
	}
}
#endif

#ifdef ENABLE_SWITCHBOT
int CInputMain::Switchbot(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	const TPacketCGSwitchbot* p = reinterpret_cast<const TPacketCGSwitchbot*>(data);

	if (uiBytes < sizeof(TPacketCGSwitchbot))
	{
		return -1;
	}

	const char* c_pData = data + sizeof(TPacketCGSwitchbot);
	uiBytes -= sizeof(TPacketCGSwitchbot);

	switch (p->subheader)
	{
		case SUBHEADER_CG_SWITCHBOT_START:
		{
			size_t extraLen = sizeof(TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
			if (uiBytes < extraLen)
			{
				return -1;
			}

			std::vector<TSwitchbotAttributeAlternativeTable> vec_alternatives;

			for (uint8_t alternative = 0; alternative < SWITCHBOT_ALTERNATIVE_COUNT; ++alternative)
			{
				const TSwitchbotAttributeAlternativeTable* pAttr = reinterpret_cast<const TSwitchbotAttributeAlternativeTable*>(c_pData);
				c_pData += sizeof(TSwitchbotAttributeAlternativeTable);

				vec_alternatives.emplace_back(*pAttr);
			}

			CSwitchbotManager::Instance().Start(ch->GetPlayerID(), p->slot, vec_alternatives);
			return extraLen;
		}

		case SUBHEADER_CG_SWITCHBOT_STOP:
		{
			CSwitchbotManager::Instance().Stop(ch->GetPlayerID(), p->slot);
			return 0;
		}
	}

	return 0;
}
#endif

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
void CInputMain::WonExchange(LPCHARACTER ch, const char* pcData)
{
	const TPacketCGWonExchange* p = reinterpret_cast<const TPacketCGWonExchange*>(pcData);
	const EWonExchangeCGSubHeader SubHeader = static_cast<EWonExchangeCGSubHeader>(p->bSubHeader);
	switch (SubHeader)
	{
		case WON_EXCHANGE_CG_SUBHEADER_SELL:
		case WON_EXCHANGE_CG_SUBHEADER_BUY:
			ch->WonExchange(SubHeader, p->wValue);
			break;

		default:
			sys_err("invalid won exchange subheader: %u value: %u", SubHeader, p->wValue);
			break;
	}
}
#endif

#ifdef ENABLE_FISHING_RENEWAL
void CInputMain::FishingNew(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
		return;

	TPacketFishingNew* p = (TPacketFishingNew*)c_pData;

	switch (p->subheader)
	{
		case FISHING_SUBHEADER_NEW_START:
			{
				ch->SetRotation(p->dir * 5);
				ch->fishing_new_start();
			}
			break;

		case FISHING_SUBHEADER_NEW_STOP:
			{
				ch->SetRotation(p->dir * 5);
				ch->fishing_new_stop();
			}
			break;

		case FISHING_SUBHEADER_NEW_CATCH:
			{
				ch->fishing_new_catch();
			}
			break;

		case FISHING_SUBHEADER_NEW_CATCH_FAILED:
			{
				ch->fishing_new_catch_failed();
			}
			break;

		default:
			return;
	}
}
#endif

