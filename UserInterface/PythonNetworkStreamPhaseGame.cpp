#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonChat.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonQuest.h"
#include "PythonEventManager.h"
#include "PythonMessenger.h"
#include "PythonApplication.h"

#include "../EterPack/EterPackManager.h"
#include "../GameLib/ItemManager.h"

#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"
#ifdef ENABLE_CUBE_RENEWAL
#	include "PythonCubeRenewal.h"
#endif
#ifdef ENABLE_RANKING_SYSTEM
#	include "PythonRanking.h"
#endif
#ifdef ENABLE_TICKET_SYSTEM
#	include "PythonTicket.h"
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "PythonDungeonInfo.h"
#endif
#ifdef ENABLE_INGAME_WIKI
#	include "../GameLib/WikiTable.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "PythonAchievement.h"
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
#	include "PythonBiologManager.h"
#endif
#ifdef ENABLE_CSHIELD
#	include "CShield.h"
#endif

#include "ProcessCRC.h"

BOOL gs_bEmpireLanuageEnable = TRUE;

#if defined(ENABLE_AUTO_RESTART_EVENT)
#include "EventHandler.h"
auto SendAutoHuntRestartPauseBot()
{
	const auto& hndl = EventHandler::Instance().GetHandler("AutoHuntRestartPauseBot");
	if (!hndl)
		return;
	int count = hndl->get()->bind; // how many called
	if (count)
	{
		CPythonPlayer::Instance().SetAutoPause(false);
	}
}

auto SendAutoHuntRestart()
{
	const auto& hndl = EventHandler::Instance().GetHandler("AutoHuntRestart");
	if (!hndl)
		return;
	int count = hndl->get()->bind; // how many called
	if (count)
	{
		CPythonPlayer::Instance().SetAutoPause(true);

		EventHandler::Instance().AddEvent("AutoHuntRestartPauseBot", std::bind(&SendAutoHuntRestartPauseBot), 15, 1);

		CPythonNetworkStream::Instance().SendChatPacket("/restart_auto");
	}
}
#endif

void CPythonNetworkStream::__RefreshAlignmentWindow()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(uint32_t dwVID)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char* c_szName)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByName", Py_BuildValue("(s)", c_szName));
}

void CPythonNetworkStream::__RefreshTargetBoard()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoard", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage()
{
	m_isRefreshGuildWndGradePage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage()
{
	m_isRefreshGuildWndSkillPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox()
{
	m_isRefreshGuildWndMemberPageGradeComboBox = true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage()
{
	m_isRefreshGuildWndMemberPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage()
{
	m_isRefreshGuildWndBoardPage = true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage()
{
	m_isRefreshGuildWndInfoPage = true;
}

void CPythonNetworkStream::__RefreshMessengerWindow()
{
	m_isRefreshMessengerWnd = true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow()
{
	m_isRefreshSafeboxWnd = true;
}

void CPythonNetworkStream::__RefreshMallWindow()
{
	m_isRefreshMallWnd = true;
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CPythonNetworkStream::__RefreshGuildstorageWindow()
{
	m_isRefreshGuildstorageWnd = true;
}
#endif

void CPythonNetworkStream::__RefreshSkillWindow()
{
	m_isRefreshSkillWnd = true;
}

void CPythonNetworkStream::__RefreshExchangeWindow()
{
	m_isRefreshExchangeWnd = true;
}

void CPythonNetworkStream::__RefreshStatus()
{
	m_isRefreshStatus = true;
}

void CPythonNetworkStream::__RefreshCharacterWindow()
{
	m_isRefreshCharacterWnd = true;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CPythonNetworkStream::__RefreshShopInfoWindow()
{
	m_isRefreshShopInfoWnd = true;
}
#endif

void CPythonNetworkStream::__RefreshInventoryWindow()
{
	m_isRefreshInventoryWnd = true;
}

void CPythonNetworkStream::__RefreshEquipmentWindow()
{
	m_isRefreshEquipmentWnd = true;
}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
void CPythonNetworkStream::__RefreshShopSearchWindow()
{
	m_isRefreshShopSearchWnd = true;
}
#endif

void CPythonNetworkStream::__SetGuildID(uint32_t id)
{
	if (m_dwGuildID != id)
	{
		m_dwGuildID = id;
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

		for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (!strncmp(m_akSimplePlayerInfo[i].szName, rkPlayer.GetName(), CHARACTER_NAME_MAX_LEN))
			{
				m_adwGuildID[i] = id;

				std::string guildName;
				if (CPythonGuild::Instance().GetGuildName(id, &guildName))
					m_astrGuildName[i] = guildName;
				else
					m_astrGuildName[i] = "";
			}
		}
	}
}

struct PERF_PacketInfo
{
	uint32_t dwCount;
	uint32_t dwTime;

	PERF_PacketInfo()
	{
		dwCount = 0;
		dwTime = 0;
	}
};

// Game Phase ---------------------------------------------------------------------------
void CPythonNetworkStream::GamePhase()
{
#if defined(ENABLE_CSHIELD) && !defined(_DEBUG)
	if (hackFound)
	{
		NotifyHack("Clientside hacking attempt");
		hackFound = FALSE;
	}
#endif

	if (!m_kQue_stHack.empty())
	{
		__SendHack(m_kQue_stHack.front().c_str());
		m_kQue_stHack.pop_front();
	}

	TPacketHeader header = 0;
	bool ret = true;

#ifndef ENABLE_NO_RECV_GAME_LIMIT
	const uint32_t MAX_RECV_COUNT = 4;
	const uint32_t SAFE_RECV_BUFSIZE = 8192;
	uint32_t dwRecvCount = 0;
#endif

	while (ret)
	{
#ifndef ENABLE_NO_RECV_GAME_LIMIT
		if (dwRecvCount++ >= MAX_RECV_COUNT - 1 && GetRecvBufferSize() < SAFE_RECV_BUFSIZE &&
			m_strPhase == "Game")
			break;
#endif

		if (!CheckPacket(&header))
			break;

		switch (header)
		{
		case HEADER_GC_WARP:
			ret = RecvWarpPacket();
			break;

		case HEADER_GC_PHASE:
			ret = RecvPhasePacket();
			return;

		case HEADER_GC_PVP:
			ret = RecvPVPPacket();
			break;

		case HEADER_GC_DUEL_START:
			ret = RecvDuelStartPacket();
			break;

		case HEADER_GC_CHARACTER_ADD:
			ret = RecvCharacterAppendPacket();
			break;

		case HEADER_GC_CHAR_ADDITIONAL_INFO:
			ret = RecvCharacterAdditionalInfo();
			break;

		case HEADER_GC_CHARACTER_UPDATE:
			ret = RecvCharacterUpdatePacket();
			break;

		case HEADER_GC_CHARACTER_DEL:
			ret = RecvCharacterDeletePacket();
			break;

		case HEADER_GC_CHAT:
			ret = RecvChatPacket();
			break;

		case HEADER_GC_SYNC_POSITION:
			ret = RecvSyncPositionPacket();
			break;

		case HEADER_GC_OWNERSHIP:
			ret = RecvOwnerShipPacket();
			break;

		case HEADER_GC_WHISPER:
			ret = RecvWhisperPacket();
			break;

		case HEADER_GC_CHARACTER_MOVE:
			ret = RecvCharacterMovePacket();
			break;

			// Position
		case HEADER_GC_CHARACTER_POSITION:
			ret = RecvCharacterPositionPacket();
			break;

			// Battle Packet
		case HEADER_GC_STUN:
			ret = RecvStunPacket();
			break;

		case HEADER_GC_DEAD:
			ret = RecvDeadPacket();
			break;

		case HEADER_GC_PLAYER_POINT_CHANGE:
			ret = RecvPointChange();
			break;

			// item packet.
		case HEADER_GC_ITEM_DEL:
			ret = RecvItemDelPacket();
			break;

		case HEADER_GC_ITEM_SET:
			ret = RecvItemSetPacket();
			break;

		case HEADER_GC_ITEM_USE:
			ret = RecvItemUsePacket();
			break;

		case HEADER_GC_ITEM_UPDATE:
			ret = RecvItemUpdatePacket();
			break;

		case HEADER_GC_ITEM_GROUND_ADD:
			ret = RecvItemGroundAddPacket();
			break;

		case HEADER_GC_ITEM_GROUND_DEL:
			ret = RecvItemGroundDelPacket();
			break;

		case HEADER_GC_ITEM_OWNERSHIP:
			ret = RecvItemOwnership();
			break;

		case HEADER_GC_QUICKSLOT_ADD:
			ret = RecvQuickSlotAddPacket();
			break;

		case HEADER_GC_QUICKSLOT_DEL:
			ret = RecvQuickSlotDelPacket();
			break;

		case HEADER_GC_QUICKSLOT_SWAP:
			ret = RecvQuickSlotMovePacket();
			break;

		case HEADER_GC_MOTION:
			ret = RecvMotionPacket();
			break;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case HEADER_GC_PLAYER_SHOP_SET:
			ret = RecvShopItemData();
			break;
#endif

		case HEADER_GC_SHOP:
			ret = RecvShopPacket();
			break;

		case HEADER_GC_SHOP_SIGN:
			ret = RecvShopSignPacket();
			break;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case HEADER_GC_MY_SHOP_SIGN:
			ret = RecvMyShopSignPacket();
			break;

		case HEADER_GC_SYNC_SHOP_STASH:
			ret = RecvShopStashSync();
			break;

		case HEADER_GC_SYNC_SHOP_OFFTIME:
			ret = RecvShopOfflineTimeSync();
			break;

		case HEADER_GC_SYNC_SHOP_POSITION:
			ret = RecvShopPositionSync();
			break;
#endif

		case HEADER_GC_EXCHANGE:
			ret = RecvExchangePacket();
			break;

		case HEADER_GC_QUEST_INFO:
			ret = RecvQuestInfoPacket();
			break;

		case HEADER_GC_REQUEST_MAKE_GUILD:
			ret = RecvRequestMakeGuild();
			break;

		case HEADER_GC_PING:
			ret = RecvPingPacket();
			break;

		case HEADER_GC_SCRIPT:
			ret = RecvScriptPacket();
			break;

		case HEADER_GC_QUEST_CONFIRM:
			ret = RecvQuestConfirmPacket();
			break;

		case HEADER_GC_TARGET:
			ret = RecvTargetPacket();
			break;

#ifdef ENABLE_SEND_TARGET_INFO
		case HEADER_GC_TARGET_INFO:
			ret = RecvTargetInfoPacket();
			break;
#endif

		case HEADER_GC_DAMAGE_INFO:
			ret = RecvDamageInfoPacket();
			break;

		case HEADER_GC_PLAYER_POINTS:
			ret = __RecvPlayerPoints();
			break;

		case HEADER_GC_CREATE_FLY:
			ret = RecvCreateFlyPacket();
			break;

		case HEADER_GC_FLY_TARGETING:
			ret = RecvFlyTargetingPacket();
			break;

		case HEADER_GC_ADD_FLY_TARGETING:
			ret = RecvAddFlyTargetingPacket();
			break;

		case HEADER_GC_SKILL_LEVEL:
			ret = RecvSkillLevel();
			break;

		case HEADER_GC_MESSENGER:
			ret = RecvMessenger();
			break;

		case HEADER_GC_GUILD:
			ret = RecvGuild();
			break;

		case HEADER_GC_PARTY_INVITE:
			ret = RecvPartyInvite();
			break;

		case HEADER_GC_PARTY_ADD:
			ret = RecvPartyAdd();
			break;

		case HEADER_GC_PARTY_UPDATE:
			ret = RecvPartyUpdate();
			break;

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
		case HEADER_GC_PARTY_POSITION_INFO:
			ret = RecvPartyPositionInfo();
			break;
#endif

		case HEADER_GC_PARTY_REMOVE:
			ret = RecvPartyRemove();
			break;

		case HEADER_GC_PARTY_LINK:
			ret = RecvPartyLink();
			break;

		case HEADER_GC_PARTY_UNLINK:
			ret = RecvPartyUnlink();
			break;

		case HEADER_GC_PARTY_PARAMETER:
			ret = RecvPartyParameter();
			break;

		case HEADER_GC_SAFEBOX_SET:
			ret = RecvSafeBoxSetPacket();
			break;

		case HEADER_GC_SAFEBOX_DEL:
			ret = RecvSafeBoxDelPacket();
			break;

		case HEADER_GC_SAFEBOX_WRONG_PASSWORD:
			ret = RecvSafeBoxWrongPasswordPacket();
			break;

		case HEADER_GC_SAFEBOX_SIZE:
			ret = RecvSafeBoxSizePacket();
			break;

#ifdef ENABLE_SAFEBOX_MONEY
		case HEADER_GC_SAFEBOX_MONEY_CHANGE:
			ret = RecvSafeBoxMoneyChangePacket();
			break;
#endif

		case HEADER_GC_FISHING:
			ret = RecvFishing();
			break;

		case HEADER_GC_DUNGEON:
			ret = RecvDungeon();
			break;

		case HEADER_GC_TIME:
			ret = RecvTimePacket();
			break;

		case HEADER_GC_WALK_MODE:
			ret = RecvWalkModePacket();
			break;

		case HEADER_GC_CHANGE_SKILL_GROUP:
			ret = RecvChangeSkillGroupPacket();
			break;

		case HEADER_GC_REFINE_INFORMATION:
			ret = RecvRefineInformationPacket();
			break;

		case HEADER_GC_SEPCIAL_EFFECT:
			ret = RecvSpecialEffect();
			break;

		case HEADER_GC_NPC_POSITION:
			ret = RecvNPCList();
			break;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case HEADER_GC_SHOP_POSITION:
			ret = RecvSHOPList();
			break;
#endif

		case HEADER_GC_CHANNEL:
			ret = RecvChannelPacket();
			break;

		case HEADER_GC_VIEW_EQUIP:
			ret = RecvViewEquipPacket();
			break;

		case HEADER_GC_LAND_LIST:
			ret = RecvLandPacket();
			break;

		case HEADER_GC_TARGET_CREATE:
			ret = RecvTargetCreatePacket();
			break;

		case HEADER_GC_TARGET_UPDATE:
			ret = RecvTargetUpdatePacket();
			break;

		case HEADER_GC_TARGET_DELETE:
			ret = RecvTargetDeletePacket();
			break;

		case HEADER_GC_AFFECT_ADD:
			ret = RecvAffectAddPacket();
			break;

		case HEADER_GC_AFFECT_REMOVE:
			ret = RecvAffectRemovePacket();
			break;

		case HEADER_GC_MALL_OPEN:
			ret = RecvMallOpenPacket();
			break;

		case HEADER_GC_MALL_SET:
			ret = RecvMallItemSetPacket();
			break;

		case HEADER_GC_MALL_DEL:
			ret = RecvMallItemDelPacket();
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case HEADER_GC_GUILDSTORAGE_OPEN:
			ret = RecvGuildstorageOpenPacket();
			break;

		case HEADER_GC_GUILDSTORAGE_SET:
			ret = RecvGuildstorageItemSetPacket();
			break;

		case HEADER_GC_GUILDSTORAGE_DEL:
			ret = RecvGuildstorageItemDelPacket();
			break;
#endif

		case HEADER_GC_LOVER_INFO:
			ret = RecvLoverInfoPacket();
			break;

		case HEADER_GC_LOVE_POINT_UPDATE:
			ret = RecvLovePointUpdatePacket();
			break;

		case HEADER_GC_DIG_MOTION:
			ret = RecvDigMotionPacket();
			break;

		case HEADER_GC_HANDSHAKE:
			RecvHandshakePacket();
			return;

		case HEADER_GC_HANDSHAKE_OK:
			RecvHandshakeOKPacket();
			return;

		case HEADER_GC_HYBRIDCRYPT_KEYS:
			RecvHybridCryptKeyPacket();
			return;

		case HEADER_GC_HYBRIDCRYPT_SDB:
			RecvHybridCryptSDBPacket();
			return;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		case HEADER_GC_KEY_AGREEMENT:
			RecvKeyAgreementPacket();
			return;

		case HEADER_GC_KEY_AGREEMENT_COMPLETED:
			RecvKeyAgreementCompletedPacket();
			return;
#endif

		case HEADER_GC_SPECIFIC_EFFECT:
			ret = RecvSpecificEffect();
			break;

		case HEADER_GC_DRAGON_SOUL_REFINE:
			ret = RecvDragonSoulRefine();
			break;

#ifdef ENABLE_GROWTH_PET_SYSTEM
		case HEADER_GC_GROWTH_PET:
			ret = RecvGrowthPetPacket();
			break;
		case HEADER_GC_GROWTH_PET_INFO:
			ret = RecvGrowthPetInfoPacket();
			break;
		case HEADER_GC_GROWTH_PET_HATCHING:
			ret = RecvGrowthPetHatching();
			break;
		case HEADER_GC_GROWTH_PET_UPGRADE_SKILL_REQUEST:
			ret = RecvGrowthPetSkillUpgradeRequest();
			break;
		case HEADER_GC_GROWTH_PET_FEED_RESULT:
			ret = RecvGrowthPetFeedItemResult();
			break;
		case HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT:
			ret = RecvGrowthPetNameChangeResult();
			break;
#	ifdef ENABLE_PET_ATTR_DETERMINE
		case HEADER_GC_GROWTH_ATTR_DETERMINE_RESULT:
			ret = RecvGrowthPetAttrDetermineResult();
			break;
		case HEADER_GC_GROWTH_ATTR_CHANGE_RESULT:
			ret = RecvGrowthPetAttrChangeResult();
			break;
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
		case HEADER_GC_GROWTH_PET_REVIVE_RESULT:
			ret = RecvGrowthPetReviveItemResult();
			break;
#	endif
#endif

#ifdef ENABLE_INGAME_WIKI
		case HEADER_GC_WIKI:
			ret = RecvWikiPacket();
			break;
#endif

#ifdef ENABLE_SWITCHBOT
		case HEADER_GC_SWITCHBOT:
			ret = RecvSwitchbotPacket();
			break;
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
		case HEADER_GC_SEAL:
			ret = RecvItemSealPacket();
			break;
#endif

#ifdef ENABLE_12ZI
		case HEADER_GC_SEPCIAL_ZODIAC_EFFECT:
			ret = RecvSpecialZodiacEffect();
			break;
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		case HEADER_GC_GEM_SHOP_OPEN:
			ret = RecvGemShopOpen();
			break;

		case HEADER_GC_GEM_SHOP_REFRESH:
			ret = RecvRefreshGemShopWIndow();
			break;

		case HEADER_GC_GEM_SHOP_BUY:
			ret = RecvGemShopSlotBuy();
			break;

		case HEADER_GC_GEM_SHOP_ADD:
			ret = RecvGemShopSlotAdd();
			break;
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
		case HEADER_GC_PRIVATE_SHOP_SEARCH_SET:
			ret = RecvPrivateShopSearchSet();
			break;

		case HEADER_GC_PRIVATE_SHOP_SEARCH_OPEN:
			ret = RecvPrivateShopSearchOpen();
			break;
#endif

#ifdef ENABLE_CUBE_RENEWAL
		case HEADER_GC_CUBE_RENEWAL:
			ret = RecvCubeRenewalPacket();
			break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case HEADER_GC_BATTLE_ZONE_INFO:
			ret = RecvBattleZoneInfo();
			break;
#endif

#ifdef ENABLE_ATTR_6TH_7TH
		case HEADER_GC_ATTR_6TH_7TH:
			ret = RecvAttr67Packet();
			break;
#endif

#ifdef ENABLE_REFINE_ELEMENT
		case HEADER_GC_ELEMENTS_SPELL:
			ret = RecvElementsSpellPacket();
			break;
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		case HEADER_GC_EXTEND_INVEN_INFO:
			ret = RecvExtendInvenInfo();
			break;
		case HEADER_GC_EXTEND_INVEN_RESULT:
			ret = RecvExtendInvenResult();
			break;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		case HEADER_GC_ACCE_REFINE_REQUEST:
			ret = RecvAcceRefinePacket();
			break;
#endif

#ifdef ENABLE_AURA_SYSTEM
		case HEADER_GC_AURA:
			ret = RecvAuraPacket();
			break;
#endif

#ifdef NEW_SELECT_CHARACTER
		case HEADER_GC_CHARACTER_INFORMATION:
			ret = RecvCharacterInformation();
			break;
#endif

#ifdef ENABLE_CHANGED_ATTR
		case HEADER_GC_ITEM_SELECT_ATTR:
			ret = RecvSelectAttr();
			break;
#endif

#ifdef ENABLE_TICKET_SYSTEM
		case HEADER_GC_TICKET_SYSTEM:
			ret = RecvTicketSystemPacket();
			break;
#endif

#ifdef ENABLE_MAILBOX
		case HEADER_GC_MAILBOX_PROCESS:
			ret = RecvMailboxProcess();
			break;

		case HEADER_GC_MAILBOX:
			ret = RecvMailbox();
			break;

		case HEADER_GC_MAILBOX_ADD_DATA:
			ret = RecvMailboxAddData();
			break;

		case HEADER_GC_MAILBOX_ALL:
			ret = RecvMailboxAll();
			break;

		case HEADER_GC_MAILBOX_UNREAD:
			ret = RecvMailboxUnread();
			break;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		case HEADER_GC_CHANGE_LOOK:
			ret = RecvChangeLookPacket();
			break;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		case HEADER_GC_TARGET_LANGUAGE_RESULT:
			ret = RecvTargetLanguageResult();
			break;
#endif

#ifdef ENABLE_PARTY_MATCH
		case HEADER_GC_PARTY_MATCH:
			ret = RecvPartyMatch();
			break;
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
		case HEADER_GC_DUNGEON_INFO:
			ret = RecvDungeonInfo();
			break;

		case HEADER_GC_DUNGEON_RANKING:
			ret = RecvDungeonRanking();
			break;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case HEADER_GC_EXT_BATTLE_PASS_OPEN:
			ret = RecvExtBattlePassOpenPacket();
			break;

		case HEADER_GC_EXT_BATTLE_PASS_GENERAL_INFO:
			ret = RecvExtBattlePassGeneralInfoPacket();
			break;

		case HEADER_GC_EXT_BATTLE_PASS_MISSION_INFO:
			ret = RecvExtBattlePassMissionInfoPacket();
			break;

		case HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE:
			ret = RecvExtBattlePassMissionUpdatePacket();
			break;

		case HEADER_GC_EXT_BATTLE_PASS_SEND_RANKING:
			ret = RecvExtBattlePassRankingPacket();
			break;
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		case HEADER_GC_MELEY_LAIR_TIME_INFO:
			ret = RecvMeleyLairRankingTimeInfo();
			break;
		case HEADER_GC_MELEY_LAIR_TIME_RESULT:
			ret = RecvMeleyLairRankingTimeResult();
			break;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case HEADER_GC_ACHIEVEMENT:
			ret = CAchievementSystem::Instance().ProcessPackets();
			break;
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
		case HEADER_GC_BIOLOG_MANAGER:
			ret = RecvBiologManager();
			break;
#endif

#ifdef ENABLE_HUNTING_SYSTEM
		case HEADER_GC_HUNTING_OPEN_MAIN:
			ret = RecvHuntingOpenWindowMain();
			break;

		case HEADER_GC_HUNTING_OPEN_SELECT:
			ret = RecvHuntingOpenWindowSelect();
			break;

		case HEADER_GC_HUNTING_OPEN_REWARD:
			ret = RecvHuntingOpenWindowReward();
			break;

		case HEADER_GC_HUNTING_UPDATE:
			ret = RecvHuntingUpdate();
			break;

		case HEADER_GC_HUNTING_RECIVE_RAND_ITEMS:
			ret = RecvHuntingRandomItems();
			break;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		case HEADER_GC_LOTTO_OPENINGS:
			ret = RecvLottoOpenWindows();
			break;

		case HEADER_GC_LOTTO_SEND_BASIC_INFO:
			ret = RecvLottoBasicInfo();
			break;

		case HEADER_GC_LOTTO_SEND_TICKET_INFO:
			ret = RecvLottoTicketInfo();
			break;

		case HEADER_GC_LOTTO_SEND_RANKING_JACKPOT:
			ret = RecvLottoRankingJackpotInfo();
			break;

		case HEADER_GC_LOTTO_SEND_RANKING_MONEY:
			ret = RecvLottoRankingMoneyInfo();
			break;
#endif

			// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
		case HEADER_GC_EVENT_INFO:
			ret = RecvEventInformation();
			break;

		case HEADER_GC_EVENT_RELOAD:
			ret = RecvEventReload();
			break;
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		case HEADER_GC_OKEY_CARD:
			ret = RecvMiniGameOkeyCardPacket();
			break;
#endif

#ifdef ENABLE_MONSTER_BACK
		case HEADER_GC_ATTENDANCE_EVENT_INFO:
			ret = RecvAttendanceEventInfo();
			break;

		case HEADER_GC_ATTENDANCE_EVENT:
			ret = RecvAttendanceEvent();
			break;
#endif

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
		case HEADER_GC_HIT_COUNT_INFO:
			ret = RecvHitCountInfo();
			break;
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
		case HEADER_GC_MINI_GAME_CATCH_KING:
			ret = RecvMiniGameCatchKingPacket();
			break;
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
		case HEADER_GC_STONE_EVENT:
			ret = RecvStoneEvent();
			break;
#endif

#ifdef ENABLE_FISH_EVENT
		case HEADER_GC_FISH_EVENT_INFO:
			ret = RecvFishEventInfo();
			break;
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		case HEADER_GC_SOUL_ROULETTE:
			ret = RecvSoulRoulette();
			break;
#endif

#ifdef ENABLE_MINI_GAME_BNW
		case HEADER_GC_MINI_GAME_BNW:
			ret = RecvMiniGameBNWPacket();
			break;
#endif

#ifdef ENABLE_MINI_GAME_FINDM
		case HEADER_GC_MINI_GAME_FIND_M:
			ret = RecvMiniGameFindMPacket();
			break;
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
		case HEADER_GC_MINI_GAME_YUT_NORI:
			ret = RecvMiniGameYutNoriPacket();
			break;
#endif

#ifdef ENABLE_FISHING_RENEWAL
		case HEADER_GC_FISHING_NEW:
			ret = RecvFishingNew();
			break;
#endif
			// END MINI GAME EVENTS //

		default:
			ret = RecvDefaultPacket(header);
			break;
		}
	}

	if (!ret)
		RecvErrorPacket(header);

	static uint32_t s_nextRefreshTime = ELTimer_GetMSec();

	uint32_t curTime = ELTimer_GetMSec();
	if (s_nextRefreshTime > curTime)
		return;

	if (m_isRefreshCharacterWnd)
	{
		m_isRefreshCharacterWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshEquipmentWnd)
	{
		m_isRefreshEquipmentWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipment", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshInventoryWnd)
	{
		m_isRefreshInventoryWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshExchangeWnd)
	{
		m_isRefreshExchangeWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchange", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSkillWnd)
	{
		m_isRefreshSkillWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSafeboxWnd)
	{
		m_isRefreshSafeboxWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMallWnd)
	{
		m_isRefreshMallWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshStatus)
	{
		m_isRefreshStatus = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMessengerWnd)
	{
		m_isRefreshMessengerWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMessenger", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndInfoPage)
	{
		m_isRefreshGuildWndInfoPage = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildInfoPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndBoardPage)
	{
		m_isRefreshGuildWndBoardPage = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBoardPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPage)
	{
		m_isRefreshGuildWndMemberPage = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPageGradeComboBox)
	{
		m_isRefreshGuildWndMemberPageGradeComboBox = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPageGradeComboBox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndSkillPage)
	{
		m_isRefreshGuildWndSkillPage = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildSkillPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndGradePage)
	{
		m_isRefreshGuildWndGradePage = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGradePage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (m_isRefreshGuildstorageWnd)
	{
		m_isRefreshGuildstorageWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildstorage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (m_isRefreshShopInfoWnd)
	{
		m_isRefreshShopInfoWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShopInfo", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	if (m_isRefreshShopSearchWnd)
	{
		m_isRefreshShopSearchWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshPShopSearchDialog", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}
#endif
}

void CPythonNetworkStream::__InitializeGamePhase()
{
	__ServerTimeSync_Initialize();

	m_isRefreshStatus = false;
	m_isRefreshCharacterWnd = false;
	m_isRefreshEquipmentWnd = false;
	m_isRefreshInventoryWnd = false;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	m_isRefreshShopInfoWnd = false;
#endif
	m_isRefreshExchangeWnd = false;
	m_isRefreshSkillWnd = false;
	m_isRefreshSafeboxWnd = false;
	m_isRefreshMallWnd = false;
	m_isRefreshMessengerWnd = false;
	m_isRefreshGuildWndInfoPage = false;
	m_isRefreshGuildWndBoardPage = false;
	m_isRefreshGuildWndMemberPage = false;
	m_isRefreshGuildWndMemberPageGradeComboBox = false;
	m_isRefreshGuildWndSkillPage = false;
	m_isRefreshGuildWndGradePage = false;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	m_isRefreshGuildstorageWnd = false;
#endif

	m_EmoticonStringVector.clear();

	m_pInstTarget = nullptr;
}

void CPythonNetworkStream::Warp(LONG lGlobalX, LONG lGlobalY)
{
	CPythonBackground& rkBgMgr = CPythonBackground::Instance();
	rkBgMgr.Destroy();
	rkBgMgr.Create();
	rkBgMgr.Warp(lGlobalX, lGlobalY);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	rkBgMgr.RefreshShadowTargetLevel();
	rkBgMgr.RefreshShadowQualityLevel();
#else
	rkBgMgr.RefreshShadowLevel();
#endif

	// NOTE : Warp 했을때 CenterPosition의 Height가 0이기 때문에 카메라가 땅바닥에 박혀있게 됨
	//        움직일때마다 Height가 갱신 되기 때문이므로 맵을 이동하면 Position을 강제로 한번
	//        셋팅해준다 - [levites]
	LONG lLocalX = lGlobalX;
	LONG lLocalY = lGlobalY;
	__GlobalPositionToLocalPosition(lLocalX, lLocalY);
	float fHeight = CPythonBackground::Instance().GetHeight(float(lLocalX), float(lLocalY));

	IAbstractApplication& rkApp = IAbstractApplication::GetSingleton();
	rkApp.SetCenterPosition(float(lLocalX), float(lLocalY), fHeight);

	__ShowMapName(lLocalX, lLocalY);
}

void CPythonNetworkStream::__ShowMapName(LONG lLocalX, LONG lLocalY)
{
	const std::string& c_rstrMapFileName = CPythonBackground::Instance().GetWarpMapName();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName",
		Py_BuildValue("(sii)", c_rstrMapFileName.c_str(), lLocalX, lLocalY));
}

void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;

	IAbstractCharacterManager& rkChrMgr = IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr = CPythonItem::Instance();
	rkItemMgr.Destroy();
}

void CPythonNetworkStream::SetGamePhase()
{
	if ("Game" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Game Phase ##");
	Tracen("");

	m_strPhase = "Game";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);

	// Main Character 등록O

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetMainCharacterIndex(GetMainActorVID());

	__RefreshStatus();
}

#ifdef ENABLE_SEND_TARGET_INFO
bool CPythonNetworkStream::SendTargetInfoLoadPacket(uint32_t dwVID)
{
	TPacketCGTargetInfoLoad TargetInfoLoadPacket{};
	TargetInfoLoadPacket.header = HEADER_CG_TARGET_INFO_LOAD;
	TargetInfoLoadPacket.dwVID = dwVID;

	if (!Send(sizeof(TargetInfoLoadPacket), &TargetInfoLoadPacket))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::RecvWarpPacket()
{
	TPacketGCWarp kWarpPacket;

	if (!Recv(sizeof(kWarpPacket), &kWarpPacket))
		return false;

	//Update the channel info
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UpdateChannelDisplayInfo", Py_BuildValue("(i)", kWarpPacket.wPort));	// RUBINUM

	__DirectEnterMode_Set(m_dwSelectedCharacterIndex);

	Connect(static_cast<uint32_t>(kWarpPacket.lAddr), kWarpPacket.wPort);

	return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket()
{
	TPacketGCDuelStart kDuelStartPacket;
	if (!Recv(sizeof(kDuelStartPacket), &kDuelStartPacket))
		return false;

	uint32_t count = (kDuelStartPacket.wSize - sizeof(kDuelStartPacket)) / sizeof(uint32_t);

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
	{
		TraceError("CPythonNetworkStream::RecvDuelStartPacket - MainCharacter is nullptr");
		return false;
	}
	uint32_t dwVIDSrc = pkInstMain->GetVirtualID();
	uint32_t dwVIDDest{};

	for (uint32_t i = 0; i < count; i++)
	{
		Recv(sizeof(dwVIDDest), &dwVIDDest);
		CInstanceBase::InsertDUELKey(dwVIDSrc, dwVIDDest);
	}

	if (count == 0)
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
	else
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

	rkChrMgr.RefreshAllPCTextTail();

	return true;
}

bool CPythonNetworkStream::RecvPVPPacket()
{
	TPacketGCPVP kPVPPacket;
	if (!Recv(sizeof(kPVPPacket), &kPVPPacket))
		return false;

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

	switch (kPVPPacket.bMode)
	{
	case PVP_MODE_AGREE:
		rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

		// 상대가 나(Dst)에게 동의를 구했을때
		if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDDst))
			rkPlayer.RememberChallengeInstance(kPVPPacket.dwVIDSrc);

		// 상대에게 동의를 구한 동안에는 대결 불능
		if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDSrc))
			rkPlayer.RememberCantFightInstance(kPVPPacket.dwVIDDst);
		break;
	case PVP_MODE_REVENGE:
	{
		rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

		uint32_t dwKiller = kPVPPacket.dwVIDSrc;
		uint32_t dwVictim = kPVPPacket.dwVIDDst;

		// 내(victim)가 상대에게 복수할 수 있을때
		if (rkPlayer.IsMainCharacterIndex(dwVictim))
			rkPlayer.RememberRevengeInstance(dwKiller);

		// 상대(victim)가 나에게 복수하는 동안에는 대결 불능
		if (rkPlayer.IsMainCharacterIndex(dwKiller))
			rkPlayer.RememberCantFightInstance(dwVictim);
		break;
	}

	case PVP_MODE_FIGHT:
		rkChrMgr.InsertPVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
		rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
		rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
		break;
	case PVP_MODE_NONE:
		rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
		rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
		rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
		break;
	}

	// NOTE : PVP 토글시 TargetBoard 를 업데이트 합니다.
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDSrc);
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDDst);

	return true;
}

void CPythonNetworkStream::NotifyHack(const char* c_szMsg)
{
	if (!m_kQue_stHack.empty())
		if (c_szMsg == m_kQue_stHack.back())
			return;

	m_kQue_stHack.emplace_back(c_szMsg);
}

bool CPythonNetworkStream::__SendHack(const char* c_szMsg)
{
	Tracen(c_szMsg);

	TPacketCGHack kPacketHack{};
	kPacketHack.bHeader = HEADER_CG_HACK;
	strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf) - 1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(uint32_t vid)
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char* c_szName)
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN - 1);
	szName[CHARACTER_NAME_MAX_LEN - 1] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemovePacket(const char* c_szKey, const char* c_szName)
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE;

	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN - 1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}

#ifdef ENABLE_MESSENGER_BLOCK
bool CPythonNetworkStream::SendMessengerBlockAddByVIDPacket(uint32_t vid)
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_VID;

	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerBlockAddByNamePacket(const char* c_szName)
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_NAME;

	if (!Send(sizeof(packet), &packet))
		return false;

	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN - 1);
	szName[CHARACTER_NAME_MAX_LEN - 1] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;

	Tracef(" SendMessengerAddBlockByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerBlockRemovePacket(const char* c_szKey, const char* c_szName)
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_BLOCK_REMOVE_BLOCK;

	if (!Send(sizeof(packet), &packet))
		return false;

	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN - 1);
	if (!Send(sizeof(szKey), &szKey))
		return false;

	CPythonMessenger::Instance().RemoveBlock(szKey);
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerBlockRemoveByVIDPacket(uint32_t vid)	//this is reversed
{
	TPacketCGMessenger packet{};
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_BLOCK_REMOVE_BLOCK;

	if (!Send(sizeof(packet), &packet))
		return false;

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(vid);

	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, pInstance->GetNameString(), CHARACTER_NAME_MAX_LEN - 1);
	if (!Send(sizeof(szKey), &szKey))
		return false;

	CPythonMessenger::Instance().RemoveBlock(szKey);
	__RefreshTargetBoardByName(pInstance->GetNameString());
	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg)
{
	NANOBEGIN
		if (!__CanActMainInstance())
			return true;

	if (fDstRot < 0.0f)
		fDstRot = 360 + fDstRot;
	else if (fDstRot > 360.0f)
		fDstRot = fmodf(fDstRot, 360.0f);

	TPacketCGMove kStatePacket{};
	kStatePacket.bHeader = HEADER_CG_CHARACTER_MOVE;
	kStatePacket.bFunc = eFunc;
	kStatePacket.bArg = uArg;
	kStatePacket.bRot = fDstRot / 5.0f;
	kStatePacket.lX = long(c_rkPPosDst.x);
	kStatePacket.lY = long(c_rkPPosDst.y);
	kStatePacket.dwTime = ELTimer_GetServerMSec();

	assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

	__LocalPositionToGlobalPosition(kStatePacket.lX, kStatePacket.lY);

	if (!Send(sizeof(kStatePacket), &kStatePacket))
	{
		Tracenf("CPythonNetworkStream::SendCharacterStatePacket(dwCmdTime=%u, fDstPos=(%f, %f), fDstRot=%f, eFunc=%d uArg=%d) - PACKET "
			"SEND ERROR",
			kStatePacket.dwTime, float(kStatePacket.lX), float(kStatePacket.lY), fDstRot, kStatePacket.bFunc, kStatePacket.bArg);
		return false;
	}
	NANOEND
		return SendSequence();
}

bool CPythonNetworkStream::SendUseSkillPacket(uint32_t dwSkillIndex, uint32_t dwTargetVID)
{
	TPacketCGUseSkill UseSkillPacket{};
	UseSkillPacket.bHeader = HEADER_CG_USE_SKILL;
	UseSkillPacket.dwVnum = dwSkillIndex;
	UseSkillPacket.dwTargetVID = dwTargetVID;
	if (!Send(sizeof(TPacketCGUseSkill), &UseSkillPacket))
	{
		Tracen("CPythonNetworkStream::SendUseSkillPacket - SEND PACKET ERROR");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChatPacket(const char* c_szChat, uint8_t byType)
{
	if (strlen(c_szChat) == 0)
		return true;

	if (strlen(c_szChat) >= 512)
		return true;

	if (c_szChat[0] == '/')
	{
		if (1 == strlen(c_szChat))
		{
			if (!m_strLastCommand.empty())
				c_szChat = m_strLastCommand.c_str();
		}
		else
			m_strLastCommand = c_szChat;
	}

	if (ClientCommand(c_szChat))
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGChat ChatPacket{};
	ChatPacket.header = HEADER_CG_CHAT;
	ChatPacket.length = sizeof(ChatPacket) + iTextLen;
	ChatPacket.type = byType;

	if (!Send(sizeof(ChatPacket), &ChatPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

//////////////////////////////////////////////////////////////////////////
// Emoticon
void CPythonNetworkStream::RegisterEmoticonString(const char* pcEmoticonString)
{
	if (m_EmoticonStringVector.size() >= CInstanceBase::EMOTICON_NUM)
	{
		TraceError("Can't register emoticon string... vector is full (size:%d)", m_EmoticonStringVector.size());
		return;
	}
	m_EmoticonStringVector.emplace_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char* pChatMsg, uint32_t* pdwEmoticon)
{
	for (uint32_t dwEmoticonIndex = 0; dwEmoticonIndex < m_EmoticonStringVector.size(); ++dwEmoticonIndex)
	{
		if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
			continue;

		const char* pcFind = strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

		if (pcFind != pChatMsg)
			continue;

		*pdwEmoticon = dwEmoticonIndex;

		return true;
	}

	return false;
}
// Emoticon
//////////////////////////////////////////////////////////////////////////

void CPythonNetworkStream::__ConvertEmpireText(uint32_t dwEmpireID, char* szText)
{
	if (dwEmpireID < 1 || dwEmpireID > 3)
		return;

	STextConvertTable& rkTextConvTable = m_aTextConvTable[dwEmpireID - 1];

	auto* pbText = reinterpret_cast<uint8_t*>(szText);
	while (*pbText)
	{
		if (*pbText & 0x80)
		{
			if (pbText[0] >= 0xb0 && pbText[0] <= 0xc8 && pbText[1] >= 0xa1 && pbText[1] <= 0xfe)
			{
				uint32_t uHanPos = (pbText[0] - 0xb0) * (0xfe - 0xa1 + 1) + (pbText[1] - 0xa1);
				pbText[0] = rkTextConvTable.aacHan[uHanPos][0];
				pbText[1] = rkTextConvTable.aacHan[uHanPos][1];
			}
			pbText += 2;
		}
		else
		{
			if (*pbText >= 'a' && *pbText <= 'z')
				*pbText = rkTextConvTable.acLower[*pbText - 'a'];
			else if (*pbText >= 'A' && *pbText <= 'Z')
				*pbText = rkTextConvTable.acUpper[*pbText - 'A'];
			pbText++;
		}
	}
}

bool CPythonNetworkStream::RecvChatPacket()
{
	TPacketGCChat kChat;
	char buf[1024 + 1]{};
	char line[1024 + 1]{};

	if (!Recv(sizeof(kChat), &kChat))
		return false;

	uint32_t uChatSize = kChat.size - sizeof(kChat);

	if (!Recv(uChatSize, buf))
		return false;

	buf[uChatSize] = '\0';

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	TTokenVector vecMultiLine;

	if (kChat.bCanFormat && CHAT_TYPE_SHOUT != kChat.type) {
		std::string sFormat{ buf };
		CPythonLocale::Instance().FormatString(sFormat);
		strncpy(buf, sFormat.c_str(), sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';

		CPythonLocale::Instance().MultiLineSplit(sFormat, vecMultiLine);
	}
#endif

	// 유럽 아랍 버전 처리
	// "이름: 내용" 입력을 "내용: 이름" 순서로 출력하기 위해 탭(0x08)을 넣음
	// 탭을 아랍어 기호로 처리해 (영어1) : (영어2) 로 입력되어도 (영어2) : (영어1) 로 출력하게 만든다
	if (LocaleService_IsEUROPE() && GetDefaultCodePage() == 1256)
	{
		char* p = strchr(buf, ':');
		if (p && p[1] == ' ')
			p[1] = 0x08;
	}

	if (kChat.type >= CHAT_TYPE_MAX_NUM)
		return true;

	if (CHAT_TYPE_COMMAND == kChat.type)
	{
		ServerCommand(buf);
		return true;
	}

	if (kChat.dwVID != 0)
	{
		CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
		CInstanceBase* pkInstChatter = rkChrMgr.GetInstancePtr(kChat.dwVID);
		if (nullptr == pkInstChatter)
			return true;

		switch (kChat.type)
		{
		case CHAT_TYPE_TALKING: /* 그냥 채팅 */
		case CHAT_TYPE_PARTY: /* 파티말 */
		case CHAT_TYPE_GUILD: /* 길드말 */
		case CHAT_TYPE_SHOUT: /* 외치기 */
		case CHAT_TYPE_WHISPER: // 서버와는 연동되지 않는 Only Client Enum
		{
			char* p = strchr(buf, ':');

			if (p)
				p += 2;
			else
				p = buf;

			uint32_t dwEmoticon;

#if defined(ENABLE_MESSENGER_BLOCK)
			if (CPythonMessenger::Instance().IsBlockFriendByName(pkInstChatter->GetNameString()))
				return true;
#endif

			if (ParseEmoticon(p, &dwEmoticon))
			{
				pkInstChatter->SetEmoticon(dwEmoticon);
				return true;
			}
			else
			{
				if (gs_bEmpireLanuageEnable)
				{
					CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
					if (pkInstMain)
						if (!pkInstMain->IsSameEmpire(*pkInstChatter))
							__ConvertEmpireText(pkInstChatter->GetEmpireID(), p);
				}

				if (m_isEnableChatInsultFilter)
				{
					if (!pkInstChatter->IsNPC() && !pkInstChatter->IsEnemy())
						__FilterInsult(p, strlen(p));
				}

				_snprintf(line, sizeof(line), "%s", p);
			}
		}
		break;

		case CHAT_TYPE_COMMAND:
		case CHAT_TYPE_INFO:
		case CHAT_TYPE_NOTICE:
		case CHAT_TYPE_BIG_NOTICE:
#ifdef ENABLE_OX_RENEWAL
		case CHAT_TYPE_CONTROL_NOTICE:
#endif
#ifdef ENABLE_DICE_SYSTEM
		case CHAT_TYPE_DICE_INFO:
#endif
#ifdef ENABLE_12ZI
		case CHAT_TYPE_MISSION:
		case CHAT_TYPE_SUB_MISSION:
		case CHAT_TYPE_CLEAR_MISSION:
#endif
#ifdef ENABLE_CHAT_SETTINGS_EXTEND
		case CHAT_TYPE_EXP_INFO:
		case CHAT_TYPE_ITEM_INFO:
		case CHAT_TYPE_MONEY_INFO:
#endif
		case CHAT_TYPE_MAX_NUM:
		default:
			_snprintf(line, sizeof(line), "%s", buf);
			break;
		}

		if (CHAT_TYPE_SHOUT != kChat.type)
			CPythonTextTail::Instance().RegisterChatTail(kChat.dwVID, line);

#ifdef ENABLE_MESSENGER_BLOCK
		if (CPythonMessenger::Instance().IsBlockFriendByName(pkInstChatter->GetNameString()))
			return true;
#endif

#ifdef ENABLE_EMPIRE_FLAG
		uint8_t bFlag = kChat.bFlag ? kChat.bEmpire : 0;
#endif

#ifdef ENABLE_12ZI
		if (pkInstChatter->IsPC() && (CHAT_TYPE_MISSION != kChat.type && CHAT_TYPE_SUB_MISSION != kChat.type && CHAT_TYPE_CLEAR_MISSION != kChat.type))
#else
		if (pkInstChatter->IsPC())
#endif
			CPythonChat::Instance().AppendChat(kChat.type,
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				kChat.bLanguage,
#endif
#ifdef ENABLE_EMPIRE_FLAG
				bFlag,
#endif
				buf);
	}
	else
	{
		if (CHAT_TYPE_NOTICE == kChat.type)
		{
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			if (!vecMultiLine.empty()) {
				for (std::string& sMessage : vecMultiLine)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", sMessage.c_str()));
			}
			else {
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
			}
#else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
#endif
		}
		else if (CHAT_TYPE_BIG_NOTICE == kChat.type)
		{
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			if (!vecMultiLine.empty()) {
				for (std::string& sMessage : vecMultiLine)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", sMessage.c_str()));
			}
			else {
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
			}
#else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
#endif
		}
#ifdef ENABLE_OX_RENEWAL
		else if (CHAT_TYPE_CONTROL_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigControlMessage", Py_BuildValue("(s)", buf));
		}
#endif
		else if (CHAT_TYPE_SHOUT == kChat.type)
		{
			char* p = strchr(buf, ':');

			if (p)
			{
				if (m_isEnableChatInsultFilter)
					__FilterInsult(p, strlen(p));
			}
		}

#ifdef ENABLE_12ZI
		else if (CHAT_TYPE_MISSION == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetMissionMessage", Py_BuildValue("(s)", buf));
			return true;
		}
		else if (CHAT_TYPE_SUB_MISSION == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetSubMissionMessage", Py_BuildValue("(s)", buf));
			return true;
		}
		else if (CHAT_TYPE_CLEAR_MISSION == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CleanMissionMessage", Py_BuildValue("()"));
			return true;
		}
#endif

#ifdef ENABLE_EMPIRE_FLAG
		uint8_t bFlag = kChat.bFlag ? kChat.bEmpire : 0;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (!vecMultiLine.empty())
		{
			for (std::string& sMessage : vecMultiLine)
			{
				CPythonChat::Instance().AppendChat(kChat.type, kChat.bLanguage,
# ifdef ENABLE_EMPIRE_FLAG
					bFlag,
# endif
					sMessage.c_str());
			}
		}
		else
		{
			CPythonChat::Instance().AppendChat(kChat.type, kChat.bLanguage,
# ifdef ENABLE_EMPIRE_FLAG
				bFlag,
# endif
				buf);
		}
#else
		CPythonChat::Instance().AppendChat(kChat.type,
# ifdef ENABLE_EMPIRE_FLAG
			bFlag,
# endif
			buf);
#endif
	}

	return true;
}

bool CPythonNetworkStream::RecvWhisperPacket()
{
	TPacketGCWhisper whisperPacket;
	char buf[512 + 1]{};

	if (!Recv(sizeof(whisperPacket), &whisperPacket))
		return false;

	assert(whisperPacket.wSize - sizeof(whisperPacket) < 512);

	if (!Recv(whisperPacket.wSize - sizeof(whisperPacket), &buf))
		return false;

	buf[whisperPacket.wSize - sizeof(whisperPacket)] = '\0';

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	if (whisperPacket.bCanFormat)
		CPythonLocale::Instance().FormatString(buf, sizeof(buf));
#endif

	static char line[256];
	if (CPythonChat::WHISPER_TYPE_CHAT == whisperPacket.bType || CPythonChat::WHISPER_TYPE_GM == whisperPacket.bType)
	{
		_snprintf(line, sizeof(line), "%s : %s", whisperPacket.szNameFrom, buf);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(iss)", static_cast<int>(whisperPacket.bType), whisperPacket.szNameFrom, line));
	}
	else if (CPythonChat::WHISPER_TYPE_SYSTEM == whisperPacket.bType || CPythonChat::WHISPER_TYPE_ERROR == whisperPacket.bType)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage", Py_BuildValue("(iss)", static_cast<int>(whisperPacket.bType), whisperPacket.szNameFrom, buf));
	else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError", Py_BuildValue("(iss)", static_cast<int>(whisperPacket.bType), whisperPacket.szNameFrom, buf));

	return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char* name, const char* c_szChat)
{
	if (strlen(c_szChat) >= 255)
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGWhisper WhisperPacket{};
	WhisperPacket.bHeader = HEADER_CG_WHISPER;
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;

	strncpy(WhisperPacket.szNameTo, name, sizeof(WhisperPacket.szNameTo) - 1);

	if (!Send(sizeof(WhisperPacket), &WhisperPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvPointChange()
{
	TPacketGCPointChange PointChange;

	if (!Recv(sizeof(TPacketGCPointChange), &PointChange))
	{
		Tracen("Recv Point Change Packet Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ShowPointEffect(PointChange.wType, PointChange.dwVID);

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	// 자신의 Point가 변경되었을 경우..
	if (pInstance && PointChange.dwVID == pInstance->GetVirtualID())
	{
#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
		int curPoint = CPythonPlayer::Instance().GetStatus(PointChange.wType);
#endif
		CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
		rkPlayer.SetStatus(PointChange.wType, PointChange.value);

		switch (PointChange.wType)
		{
		case POINT_STAT_RESET_COUNT:
			__RefreshStatus();
			break;
		case POINT_LEVEL:
		case POINT_ST:
		case POINT_DX:
		case POINT_HT:
		case POINT_IQ:
#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_CONQUEROR_LEVEL:
#endif
			__RefreshStatus();
			__RefreshSkillWindow();
			break;
		case POINT_SKILL:
		case POINT_SUB_SKILL:
		case POINT_HORSE_SKILL:
			__RefreshSkillWindow();
			break;
		case POINT_ENERGY:
			if (PointChange.value == 0)
				rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
			__RefreshStatus();
			break;

#ifdef ENABLE_BATTLE_FIELD
		case POINT_BATTLE_FIELD:
		{
			const int dif = PointChange.value - curPoint;
			if (CPythonShop::Instance().IsOpen() && dif < 0)
			{
				CPythonShop::Instance().SetUsablePoint(EShopCoinType::SHOP_COIN_TYPE_BATTLE_POINT, dif + CPythonShop::Instance().GetUsablePoint(EShopCoinType::SHOP_COIN_TYPE_BATTLE_POINT));
				CPythonPlayer::Instance().SetStatus(PointChange.wType, PointChange.value);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
			}
			break;
		}
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
		case POINT_MEDAL_OF_HONOR:
		{
			const int dif = PointChange.value - curPoint;
			if (CPythonShop::Instance().IsOpen() && dif < 0)
			{
				CPythonShop::Instance().SetUsablePoint(EShopCoinType::SHOP_COIN_TYPE_MEDAL_OF_HONOR, dif + CPythonShop::Instance().GetUsablePoint(EShopCoinType::SHOP_COIN_TYPE_MEDAL_OF_HONOR));
				CPythonPlayer::Instance().SetStatus(PointChange.wType, PointChange.value);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
			}
			break;
		}
#endif
		default:
			__RefreshStatus();
			break;
		}

		if (POINT_GOLD == PointChange.wType)
		{
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney", Py_BuildValue("(i)", PointChange.amount));
		}

#ifdef ENABLE_CHEQUE_SYSTEM
		if (POINT_CHEQUE == PointChange.wType)
		{
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickCheque", Py_BuildValue("(i)", PointChange.amount));
		}
#endif

#ifdef ENABLE_GEM_SYSTEM
		if (POINT_GEM == PointChange.wType)
		{
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickGem", Py_BuildValue("(i)", PointChange.amount));
		}
#endif

#ifdef ENABLE_BATTLE_FIELD
		if (POINT_BATTLE_FIELD == PointChange.wType)
		{
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickBattlePoint", Py_BuildValue("(b)", PointChange.amount));
		}
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
		if (POINT_MEDAL_OF_HONOR == PointChange.wType)
		{
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMedalOfHonor", Py_BuildValue("(b)", PointChange.amount)); // MEDAL_OF_HONOR_SYSTEM_PICK
		}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		if (POINT_LOTTO_MONEY == PointChange.wType)
		{
			if (PointChange.amount > 0)
			{
				PyObject* args = PyTuple_New(1);
				PyTuple_SetItem(args, 0, PyLong_FromLongLong(PointChange.amount));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickLottoMoney", args);
				//PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickLottoMoney", Py_BuildValue("(i)", PointChange.amount));
			}
		}

		if (POINT_LOTTO_TOTAL_MONEY == PointChange.wType)
		{
			if (PointChange.amount > 0)
			{
				PyObject* args = PyTuple_New(1);
				PyTuple_SetItem(args, 0, PyLong_FromLongLong(PointChange.amount));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickLottoTotalMoneyWin", args);
			}
		}
#endif
	}
#ifdef ENABLE_TEXT_LEVEL_REFRESH
	else
	{
		// the /advance command will provide no global refresh! it sends the pointchange only to the specific player and not all
		pInstance = CPythonCharacterManager::Instance().GetInstancePtr(PointChange.dwVID);
		if (pInstance && PointChange.wType == POINT_LEVEL)
		{
			pInstance->SetLevel(PointChange.value);
			pInstance->UpdateTextTailLevel(PointChange.value);
		}
#	ifdef ENABLE_YOHARA_SYSTEM
		if (pInstance && PointChange.wType == POINT_CONQUEROR_LEVEL)
		{
			pInstance->SetConquerorLevel(PointChange.value);
			pInstance->UpdateTextTailLevel(PointChange.value);
		}
#	endif
	}
#endif

	return true;
}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
bool CPythonNetworkStream::RecvGemShopOpen()
{
	TPacketGCGemShopItems p_gemshop_refresh;
	if (!Recv(sizeof(p_gemshop_refresh), &p_gemshop_refresh))
		return false;

	CPythonPlayer::Instance().ClearGemShopItemVector();
	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		TGemShopItem packet_gemshop_item{};
		packet_gemshop_item.display_pos = p_gemshop_refresh.gemShopItems[i].display_pos;
		packet_gemshop_item.status = p_gemshop_refresh.gemShopItems[i].status;
		packet_gemshop_item.unlocked = p_gemshop_refresh.gemShopItems[i].unlocked;

		packet_gemshop_item.vnum = p_gemshop_refresh.gemShopItems[i].vnum;
		packet_gemshop_item.count = p_gemshop_refresh.gemShopItems[i].count;
		packet_gemshop_item.price = p_gemshop_refresh.gemShopItems[i].price;

		CPythonPlayer::Instance().SetGemShopOpenSlotItemCount(i, p_gemshop_refresh.bGemShopOpenSlotItemCount[i]);

		CPythonPlayer::Instance().SetGemShopItemData(p_gemshop_refresh.gemShopItems[i].display_pos, packet_gemshop_item);
	}

	CPythonPlayer::Instance().SetGemShopRefreshTime(p_gemshop_refresh.nextRefreshTime);
	CPythonPlayer::Instance().SetGemShopOpenSlotCount(p_gemshop_refresh.bGemShopOpenSlotCount);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenGemShop", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvRefreshGemShopWIndow()
{
	TPacketGCGemShopItems p_gemshop_refresh;
	if (!Recv(sizeof(p_gemshop_refresh), &p_gemshop_refresh))
		return false;

	CPythonPlayer::Instance().ClearGemShopItemVector();
	for (uint8_t i = 0; i < GEM_SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		TGemShopItem packet_gemshop_item{};
		packet_gemshop_item.display_pos = p_gemshop_refresh.gemShopItems[i].display_pos;
		packet_gemshop_item.status = p_gemshop_refresh.gemShopItems[i].status;
		packet_gemshop_item.unlocked = p_gemshop_refresh.gemShopItems[i].unlocked;

		packet_gemshop_item.vnum = p_gemshop_refresh.gemShopItems[i].vnum;
		packet_gemshop_item.count = p_gemshop_refresh.gemShopItems[i].count;
		packet_gemshop_item.price = p_gemshop_refresh.gemShopItems[i].price;

		CPythonPlayer::Instance().SetGemShopOpenSlotItemCount(i, p_gemshop_refresh.bGemShopOpenSlotItemCount[i]);

		CPythonPlayer::Instance().SetGemShopItemData(p_gemshop_refresh.gemShopItems[i].display_pos, packet_gemshop_item);
	}

	CPythonPlayer::Instance().SetGemShopRefreshTime(p_gemshop_refresh.nextRefreshTime);
	CPythonPlayer::Instance().SetGemShopOpenSlotCount(p_gemshop_refresh.bGemShopOpenSlotCount);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGemShopWIndow", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvGemShopSlotBuy()
{
	TPacketGCGemShopBuy p_gemshop_buy;
	if (!Recv(sizeof(p_gemshop_buy), &p_gemshop_buy))
		return false;

	CPythonPlayer::Instance().SetGemShopSlotAdd(p_gemshop_buy.bSlotIndex, p_gemshop_buy.bEnable);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GemShopSlotBuy", Py_BuildValue("(ii)", p_gemshop_buy.bSlotIndex, p_gemshop_buy.bEnable));

	return true;
}

bool CPythonNetworkStream::RecvGemShopSlotAdd()
{
	TPacketGCGemShopAdd p_gemshop_add;
	if (!Recv(sizeof(p_gemshop_add), &p_gemshop_add))
		return false;

	const uint8_t bGemShopOpenSlotCount = CPythonPlayer::Instance().GetGemShopOpenSlotCount() + 1;
	CPythonPlayer::Instance().SetGemShopOpenSlotCount(bGemShopOpenSlotCount);
	CPythonPlayer::Instance().SetGemShopSlotAdd(p_gemshop_add.bSlotIndex, p_gemshop_add.bEnable);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GemShopSlotAdd", Py_BuildValue("(ii)", p_gemshop_add.bSlotIndex, p_gemshop_add.bEnable));

	return true;
}

bool CPythonNetworkStream::SendSelectmetinstone(uint32_t dwItemPos)
{
	TPacketCGScriptSelectItemEx kScriptSelectItemEx{};
	kScriptSelectItemEx.header = HEADER_CG_SCRIPT_SELECT_ITEM_EX;
	kScriptSelectItemEx.selection = dwItemPos;

	if (!Send(sizeof(kScriptSelectItemEx), &kScriptSelectItemEx))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
bool CPythonNetworkStream::SendRemoveFromMyShop(int slot, TItemPos target)
{
	TPacketMyShopRemoveItem p;
	p.header = HEADER_CG_MYSHOP_REMOVE_ITEM;
	p.slot = slot;
	p.target = target;

	if (!Send(sizeof(TPacketMyShopRemoveItem), &p))
	{
		Tracen("Send remove from my shop error.");
		return false;
	}

	return true;
}

#	ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendAddToMyShop(TItemPos from, int target, uint32_t price, uint32_t cheque)
#	else
bool CPythonNetworkStream::SendAddToMyShop(TItemPos from, int target, uint32_t price)
#	endif
{
	TPacketMyShopAddItem p;
	p.header = HEADER_CG_MYSHOP_ADD_ITEM;
	p.from = from;
	p.targetPos = target;
	p.price = price;
#	ifdef ENABLE_CHEQUE_SYSTEM
	p.cheque = cheque;
#	endif

	if (!Send(sizeof(TPacketMyShopAddItem), &p))
	{
		Tracen("Send add to my shop error.");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::CloseMyShop()
{
	uint8_t header = HEADER_CG_MYSHOP_CLOSE;
	if (!Send(sizeof(uint8_t), &header))
	{
		Tracen("Send close my shop error.");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::OpenMyShop()
{
	uint8_t header = HEADER_CG_MYSHOP_OPEN;
	if (!Send(sizeof(uint8_t), &header))
	{
		Tracen("Send open my shop error.");
		return false;
	}

	return true;
}

#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_EXPIRE
bool CPythonNetworkStream::ReopenMyShop()
{
	uint8_t header = HEADER_CG_MYSHOP_REOPEN;
	if (!Send(sizeof(uint8_t), &header))
	{
		Tracen("Send reopen my shop error.");
		return false;
	}

	return true;
}
#	endif

bool CPythonNetworkStream::WithdrawMyShopMoney(uint32_t goldAmount, uint8_t chequeAmount)
{
	TPacketCGShopWithdraw pack{};
	pack.bHeader = HEADER_CG_MYSHOP_WITHDRAW;
	pack.amount = goldAmount;
#	ifdef ENABLE_CHEQUE_SYSTEM
	pack.chequeAmount = chequeAmount;
#	endif

	if (!Send(sizeof(TPacketCGShopWithdraw), &pack))
	{
		Tracen("Send withdraw my shop error.");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RenameMyShop(char* newName)
{
	CPythonShop& shopInstance = CPythonShop::Instance();

	TPacketGCShopRename p{};
	p.bHeader = HEADER_CG_MYSHOP_RENAME;
	strncpy_s(p.sign, newName, SHOP_SIGN_MAX_LEN);

	if (!Send(sizeof(TPacketGCShopRename), &p))
	{
		Tracen("Send rename my shop error.");
		return false;
	}

	return true;
}

#ifdef ENABLE_MYSHOP_DECO
bool CPythonNetworkStream::SendMyShopDecoState(uint8_t bState)
{
	TPacketCGMyShopDecoState pack{};
	pack.header = HEADER_CG_MYSHOP_DECO_STATE;
	pack.bState = bState;

	if (!Send(sizeof(pack), &pack))
	{
		Tracef("TPacketCGMyShopDeco Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMyShopDecoSet(uint8_t bType, uint32_t dwPolyVNum)
{
	TPacketCGMyShopDecoAdd pack{};
	pack.header = HEADER_CG_MYSHOP_DECO_ADD;
	pack.bType = bType;
	pack.dwPolyVnum = dwPolyVNum;

	if (!Send(sizeof(pack), &pack))
	{
		Tracef("TPacketCGMyShopDecoAdd Error\n");
		return false;
	}

	return SendSequence();
}
#endif
#endif

bool CPythonNetworkStream::RecvStunPacket()
{
	TPacketGCStun StunPacket;

	if (!Recv(sizeof(StunPacket), &StunPacket))
	{
		Tracen("CPythonNetworkStream::RecvStunPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstSel = rkChrMgr.GetInstancePtr(StunPacket.vid);

	if (pkInstSel)
	{
		if (CPythonCharacterManager::Instance().GetMainInstancePtr() == pkInstSel)
			pkInstSel->Die();
		else
			pkInstSel->Stun();
	}

	return true;
}

bool CPythonNetworkStream::RecvDeadPacket()
{
	TPacketGCDead DeadPacket;
	if (!Recv(sizeof(DeadPacket), &DeadPacket))
	{
		Tracen("CPythonNetworkStream::RecvDeadPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkChrInstSel = rkChrMgr.GetInstancePtr(DeadPacket.vid);
	if (pkChrInstSel)
	{
		CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
		if (pkInstMain == pkChrInstSel)
		{
			Tracenf("On MainActor");

#ifdef ENABLE_AUTO_SYSTEM
			//if (CPythonPlayer::Instance().AutoStatus()) { CPythonPlayer::Instance().AutoStartOnOff(false); }
			if (CPythonPlayer::Instance().AutoStatus())
			{
# if defined(ENABLE_AUTO_RESTART_EVENT)
				if (CPythonPlayer::Instance().GetAutoRestart())
				{
					EventHandler::Instance().AddEvent("AutoHuntRestart", std::bind(&SendAutoHuntRestart), 10, 1); // 60-> 1 min, -1 ->endless event
					//EventHandler::Instance().AddEvent("PlayMessage", [&] {SendMessageEvent; }, 60, -1); // 60-> 1 min, -1 ->endless event
				}
# endif	
			}
			//pkChrInstSel->SetAutoAffect(CPythonPlayer::Instance().CanStartAuto());
#endif

#ifdef ENABLE_REVERSED_FUNCTIONS
			for (uint32_t i = 0; i <= SKILL_MAX_NUM; ++i)
			{
				if (CPythonPlayer::Instance().IsSkillActive(i))
					CPythonPlayer::Instance().__DeactivateSkillSlot(i);
			}
#endif

			if (false == pkInstMain->GetDuelMode())
			{
#ifdef ENABLE_BATTLE_FIELD
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("(ii)", DeadPacket.bRestart, DeadPacket.lMapIdx));
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("()"));
#endif
			}
			CPythonPlayer::Instance().NotifyDeadMainCharacter();
#ifdef ENABLE_SKILL_COOLTIME_UPDATE
			CPythonPlayer::Instance().ResetSkillCoolTimes();
#endif
		}

		pkChrInstSel->Die();
	}

	return true;
}

bool CPythonNetworkStream::SendCharacterPositionPacket(uint8_t iPosition)
{
	TPacketCGPosition PositionPacket{};

	PositionPacket.header = HEADER_CG_CHARACTER_POSITION;
	PositionPacket.position = iPosition;

	if (!Send(sizeof(TPacketCGPosition), &PositionPacket))
	{
		Tracen("Send Character Position Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOnClickPacket(uint32_t vid)
{
	TPacketCGOnClick OnClickPacket{};
	OnClickPacket.header = HEADER_CG_ON_CLICK;
	OnClickPacket.vid = vid;

	if (!Send(sizeof(OnClickPacket), &OnClickPacket))
	{
		Tracen("Send On_Click Packet Error");
		return false;
	}

	Tracef("SendOnClickPacket\n");
	return SendSequence();
}

bool CPythonNetworkStream::RecvCharacterPositionPacket()
{
	TPacketGCPosition PositionPacket;

	if (!Recv(sizeof(TPacketGCPosition), &PositionPacket))
		return false;

	CInstanceBase* pChrInstance = CPythonCharacterManager::Instance().GetInstancePtr(PositionPacket.vid);

	if (!pChrInstance)
		return true;

	return true;
}

bool CPythonNetworkStream::RecvMotionPacket()
{
	TPacketGCMotion MotionPacket;

	if (!Recv(sizeof(TPacketGCMotion), &MotionPacket))
		return false;

	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.vid);
	CInstanceBase* pVictimInstance = nullptr;

	if (0 != MotionPacket.victim_vid)
		pVictimInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.victim_vid);

	if (!pMainInstance)
		return false;

	return true;
}

bool CPythonNetworkStream::RecvShopPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCShop packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;

	int iSize = packet_shop.size - sizeof(packet_shop);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet_shop.subheader)
	{
	case SHOP_SUBHEADER_GC_START:
	{
		CPythonShop::Instance().Clear();

		uint32_t dwVID = *(uint32_t*)&vecBuffer[0];

		auto* pShopStartPacket = (TPacketGCShopStart*)&vecBuffer[4];

#ifdef ENABLE_MYSHOP_DECO
		const uint8_t shop_tab_count = pShopStartPacket->shop_tab_count;
		CPythonShop::Instance().SetTabCount(shop_tab_count);

		//#ifdef ENABLE_RENEWAL_SHOPEX
		CPythonShop::Instance().SetShopExLoading(false);
		//#endif

		for (uint8_t iItemIndex = 0; iItemIndex < SHOP_HOST_ITEM_MAX; ++iItemIndex)
			CPythonShop::Instance().SetItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
#else
		for (uint8_t iItemIndex = 0; iItemIndex < SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
			CPythonShop::Instance().SetItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
#endif

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		CPythonShop::Instance().SetLimitedItemShop(pShopStartPacket->islimiteditemshop);
#endif

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
	}
	break;

	case SHOP_SUBHEADER_GC_END:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop", Py_BuildValue("()"));
		break;

	case SHOP_SUBHEADER_GC_UPDATE_ITEM:
	{
		auto* pShopUpdateItemPacket = reinterpret_cast<TPacketGCShopUpdateItem*>(&vecBuffer[0]);
		CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_UPDATE_PRICE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice", Py_BuildValue("(i)", *(int*)&vecBuffer[0]));
		break;

		//SHOP_SUBHEADER_GC_OK

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
		break;

	case SHOP_SUBHEADER_GC_SOLDOUT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
		break;

	case SHOP_SUBHEADER_GC_INVENTORY_FULL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
		break;

	case SHOP_SUBHEADER_GC_INVALID_POS:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
		break;

		//SHOP_SUBHEADER_GC_SOLD_OUT

	case SHOP_SUBHEADER_GC_START_EX:
	{
		CPythonShop::Instance().Clear();

		auto* pShopStartPacket = (TPacketGCShopStartEx*)&vecBuffer[0];
		size_t read_point = sizeof(TPacketGCShopStartEx);

		uint32_t dwVID = pShopStartPacket->owner_vid;
		uint8_t shop_tab_count = pShopStartPacket->shop_tab_count;

		CPythonShop::Instance().SetTabCount(shop_tab_count);

		for (uint8_t i = 0; i < shop_tab_count; i++)
		{
			auto* pPackTab = reinterpret_cast<TPacketGCShopStartEx::TSubPacketShopTab*>(&vecBuffer[read_point]);
			read_point += sizeof(TPacketGCShopStartEx::TSubPacketShopTab);

			CPythonShop::Instance().SetTabCoinType(i, pPackTab->coin_type);
			CPythonShop::Instance().SetTabName(i, pPackTab->name);
#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
			CPythonShop::Instance().SetLimitMaxPoint(pPackTab->coin_type, pPackTab->limitMaxPoint);
			CPythonShop::Instance().SetUsablePoint(pPackTab->coin_type, pPackTab->usablePoints);
#endif

			struct packet_shop_item* item = &pPackTab->items[0];

			for (uint8_t j = 0; j < SHOP_HOST_ITEM_MAX_NUM; j++)
			{
				TShopItemData* itemData = (item + j);
				if (itemData) {
#ifdef ENABLE_MYSHOP_DECO	//ENABLE_RENEWAL_SHOPEX
					CPythonShop::Instance().SetShopExLoading(true);
#endif
					CPythonShop::Instance().SetItemData(i, j, *itemData);
				}
			}
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
	}
	break;

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
		break;

#ifdef ENABLE_BATTLE_FIELD
	case SHOP_SUBHEADER_GC_NOT_ENOUGH_BATTLE_POINTS:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_BP"));
		break;
#endif

#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
	case SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "EXCEED_LIMIT_TODAY"));
		break;
#endif

#ifdef ENABLE_MYSHOP_DECO
	case SHOP_SUBHEADER_GC_MYPRIV_SHOP_OPEN: // 15
	{
		const auto* pMyPrivShopOpen = (TPacketGCMyPrivShopOpen*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MyPrivShopOpen", Py_BuildValue("(bi)", pMyPrivShopOpen->bCashItem, pMyPrivShopOpen->bTabCount));
	}
	break;
#endif

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_10TH_COIN:
#ifdef ENABLE_10TH_EVENT
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_10TH_COIN"));
#endif
		break;

#ifdef ENABLE_12ZI
	case SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER: // 17
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "LIMITED_PURCHASE_OVER"));
		break;
	case SHOP_SUBHEADER_GC_LIMITED_DATA_LOADING: // 18
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "LIMITED_DATA_LOADING"));
		break;
#endif

	case SHOP_SUBHEADER_GC_UNK_19:
		break;
	case SHOP_SUBHEADER_GC_UNK_20:
		break;

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	case SHOP_SUBHEADER_GC_UNK_21:
	{
		const auto* pShopLimitedPurchaseInfo = reinterpret_cast<TPacketGCShopUpdateLimitedPurchase*>(&vecBuffer[0]);
		CPythonShop::Instance().SetLimitedPurchaseInfo(pShopLimitedPurchaseInfo->vnum,
			pShopLimitedPurchaseInfo->limited_count, pShopLimitedPurchaseInfo->limited_purchase_count);

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShopItemToolTip", Py_BuildValue("()"));
	}
	break;
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
	case SHOP_SUBHEADER_GC_NOT_ENOUGH_GUILD_LEVEL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_GUILD_LEVEL"));
		break;

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_MEDAL_OF_HONOR:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MEDAL_OF_HONOR"));
		break;
#endif

#ifdef ENABLE_MINI_GAME_BNW
	case SHOP_SUBHEADER_GC_NOT_ENOUGH_BNW:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_BNW"));
		break;
#endif

		// Custom
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	case SHOP_SUBHEADER_GC_OPEN_SHOP_EDITOR:
	{
#	ifdef ENABLE_MYSHOP_DECO
		const auto* pMyPrivShopOpen = (TPacketGCMyPrivShopOpen*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenShopEditor", Py_BuildValue("(i)", pMyPrivShopOpen->bTabCount));
		TraceError("OpenShopEditor TabCount %d", pMyPrivShopOpen->bTabCount);
#	else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenShopEditor", Py_BuildValue("()"));
#	endif
	}
	break;
#endif

#ifdef ENABLE_RENEWAL_SHOPEX
	case SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_ITEM"));
		break;

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_EXP:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_EXP"));
		break;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	case SHOP_SUBHEADER_GC_NOT_ENOUGH_ACHIEVEMENT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_ACHIEVEMENT"));
		break;
#endif

	default:
		TraceError("CPythonNetworkStream::RecvShopPacket: Unknown subheader\n");
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvExchangePacket()
{
	TPacketGCExchange exchange_packet;

	if (!Recv(sizeof(exchange_packet), &exchange_packet))
		return false;

	switch (exchange_packet.subheader)
	{
	case EXCHANGE_SUBHEADER_GC_START:
		CPythonExchange::Instance().Clear();
		CPythonExchange::Instance().Start();
		CPythonExchange::Instance().SetSelfName(CPythonPlayer::Instance().GetName());
#ifdef ENABLE_LEVEL_IN_TRADE
		CPythonExchange::Instance().SetSelfLevel(CPythonPlayer::Instance().GetStatus(POINT_LEVEL));
#endif

		{
			CInstanceBase* pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(exchange_packet.arg1);

			if (pCharacterInstance)
			{
				CPythonExchange::Instance().SetTargetName(pCharacterInstance->GetNameString());
#ifdef ENABLE_LEVEL_IN_TRADE
				CPythonExchange::Instance().SetTargetLevel(pCharacterInstance->GetLevel());
#endif
			}
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExchange", Py_BuildValue("()"));
		break;

	case EXCHANGE_SUBHEADER_GC_ITEM_ADD:
		if (exchange_packet.is_me)
		{
			int iSlotIndex = exchange_packet.arg2.cell;
			CPythonExchange::Instance().SetItemToSelf(iSlotIndex, exchange_packet.arg1, (uint8_t)exchange_packet.arg3);

#ifdef ENABLE_REFINE_ELEMENT
			CPythonExchange::Instance().SetItemElementToSelf(iSlotIndex, exchange_packet.grade_element, exchange_packet.element_type_bonus);
			for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
				CPythonExchange::Instance().SetItemElementAttackToSelf(iSlotIndex, i, exchange_packet.attack_element[i]);
				CPythonExchange::Instance().SetItemElementValueToSelf(iSlotIndex, i, exchange_packet.elements_value_bonus[i]);
			}
#endif

			for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
				CPythonExchange::Instance().SetItemMetinSocketToSelf(iSlotIndex, i, exchange_packet.alValues[i]);
			for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
				CPythonExchange::Instance().SetItemAttributeToSelf(iSlotIndex, j, exchange_packet.aAttr[j].wType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_YOHARA_SYSTEM
			for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; ++s)
			{
				CPythonExchange::Instance().SetItemApplyRandomToSelf(iSlotIndex, s, exchange_packet.aApplyRandom[s].wType, exchange_packet.aApplyRandom[s].sValue);
			}

			for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; ++r)
			{
				CPythonExchange::Instance().SetRandomDefaultAttrs(iSlotIndex, r, exchange_packet.alRandomValues[r], true);
			}
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			CPythonExchange::Instance().SetItemChangeLookVnum(iSlotIndex, exchange_packet.dwTransmutationVnum, true);
#endif
#ifdef ENABLE_SET_ITEM
			CPythonExchange::Instance().SetItemSetValue(iSlotIndex, exchange_packet.set_value, true);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			CPythonPlayer::Instance().SetPetInfo(exchange_packet.aPetInfo);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddExchangeItemSlotIndex", Py_BuildValue("(i)", exchange_packet.arg4.cell));
#endif
		}
		else
		{
			int iSlotIndex = exchange_packet.arg2.cell;
			CPythonExchange::Instance().SetItemToTarget(iSlotIndex, exchange_packet.arg1, (uint8_t)exchange_packet.arg3);

#ifdef ENABLE_REFINE_ELEMENT
			CPythonExchange::Instance().SetItemElementToTarget(iSlotIndex, exchange_packet.grade_element, exchange_packet.element_type_bonus);
			for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
				CPythonExchange::Instance().SetItemElementAttackToTarget(iSlotIndex, i, exchange_packet.attack_element[i]);
				CPythonExchange::Instance().SetItemElementValueToTarget(iSlotIndex, i, exchange_packet.elements_value_bonus[i]);
			}
#endif

			for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
				CPythonExchange::Instance().SetItemMetinSocketToTarget(iSlotIndex, i, exchange_packet.alValues[i]);
			for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
				CPythonExchange::Instance().SetItemAttributeToTarget(iSlotIndex, j, exchange_packet.aAttr[j].wType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_YOHARA_SYSTEM
			for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; ++s)
			{
				CPythonExchange::Instance().SetItemApplyRandomToTarget(iSlotIndex, s, exchange_packet.aApplyRandom[s].wType, exchange_packet.aApplyRandom[s].sValue);
			}

			for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; ++r)
			{
				CPythonExchange::Instance().SetRandomDefaultAttrs(iSlotIndex, r, exchange_packet.alRandomValues[r], false);
			}
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			CPythonExchange::Instance().SetItemChangeLookVnum(iSlotIndex, exchange_packet.dwTransmutationVnum, false);
#endif
#ifdef ENABLE_SET_ITEM
			CPythonExchange::Instance().SetItemSetValue(iSlotIndex, exchange_packet.set_value, false);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			CPythonPlayer::Instance().SetPetInfo(exchange_packet.aPetInfo);
#endif
		}

		__RefreshExchangeWindow();
		__RefreshInventoryWindow();
		break;

	case EXCHANGE_SUBHEADER_GC_ITEM_DEL:
		if (exchange_packet.is_me)
			CPythonExchange::Instance().DelItemOfSelf((uint8_t)exchange_packet.arg1);
		else
			CPythonExchange::Instance().DelItemOfTarget((uint8_t)exchange_packet.arg1);
		__RefreshExchangeWindow();
		__RefreshInventoryWindow();
		break;

	case EXCHANGE_SUBHEADER_GC_ELK_ADD:
		if (exchange_packet.is_me)
		{
			CPythonExchange::Instance().SetElkToSelf(exchange_packet.arg1);
#ifdef ENABLE_CHEQUE_SYSTEM
			CPythonExchange::Instance().SetChequeToSelf(exchange_packet.cheque);
#endif
		}
		else
		{
			CPythonExchange::Instance().SetElkToTarget(exchange_packet.arg1);
#ifdef ENABLE_CHEQUE_SYSTEM
			CPythonExchange::Instance().SetChequeToTarget(exchange_packet.cheque);
#endif
		}

		__RefreshExchangeWindow();
		break;

	case EXCHANGE_SUBHEADER_GC_ACCEPT:
		if (exchange_packet.is_me)
			CPythonExchange::Instance().SetAcceptToSelf((uint8_t)exchange_packet.arg1);
		else
			CPythonExchange::Instance().SetAcceptToTarget((uint8_t)exchange_packet.arg1);
		__RefreshExchangeWindow();
		break;

	case EXCHANGE_SUBHEADER_GC_END:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchange", Py_BuildValue("()"));
		__RefreshInventoryWindow();
		CPythonExchange::Instance().End();
		break;

	case EXCHANGE_SUBHEADER_GC_ALREADY:
		Tracef("trade_already");
		break;

	case EXCHANGE_SUBHEADER_GC_LESS_ELK:
		Tracef("trade_less_elk");
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket()
{
	TPacketGCQuestInfo QuestInfo;

	if (!Peek(sizeof(TPacketGCQuestInfo), &QuestInfo))
	{
		Tracen("Recv Quest Info Packet Error #1");
		return false;
	}

	if (!Peek(QuestInfo.size))
	{
		Tracen("Recv Quest Info Packet Error #2");
		return false;
	}

	Recv(sizeof(TPacketGCQuestInfo));

	const uint8_t& c_rFlag = QuestInfo.flag;

	enum
	{
		QUEST_PACKET_TYPE_NONE,
		QUEST_PACKET_TYPE_BEGIN,
		QUEST_PACKET_TYPE_UPDATE,
		QUEST_PACKET_TYPE_END
	};

	uint8_t byQuestPacketType = QUEST_PACKET_TYPE_NONE;

	if (0 != (c_rFlag & QUEST_SEND_IS_BEGIN))
	{
		uint8_t isBegin;
		if (!Recv(sizeof(isBegin), &isBegin))
			return false;

		if (isBegin)
			byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
		else
			byQuestPacketType = QUEST_PACKET_TYPE_END;
	}
	else
		byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;

	// Recv Data Start
	char szTitle[30 + 1] = "";
	char szClockName[16 + 1] = "";
	int iClockValue = 0;
	char szCounterName[16 + 1] = "";
	int iCounterValue = 0;
	char szIconFileName[24 + 1] = "";

	if (0 != (c_rFlag & QUEST_SEND_TITLE))
	{
		if (!Recv(sizeof(szTitle), &szTitle))
			return false;

		szTitle[30] = '\0';
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CPythonLocale::Instance().FormatString(szTitle, sizeof(szTitle));
#endif
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_NAME))
	{
		if (!Recv(sizeof(szClockName), &szClockName))
			return false;

		szClockName[16] = '\0';
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CPythonLocale::Instance().FormatString(szClockName, sizeof(szClockName));
#endif
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_VALUE))
	{
		if (!Recv(sizeof(iClockValue), &iClockValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_NAME))
	{
		if (!Recv(sizeof(szCounterName), &szCounterName))
			return false;

		szCounterName[16] = '\0';
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CPythonLocale::Instance().FormatString(szCounterName, sizeof(szCounterName));
#endif
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_VALUE))
	{
		if (!Recv(sizeof(iCounterValue), &iCounterValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_ICON_FILE))
	{
		if (!Recv(sizeof(szIconFileName), &szIconFileName))
			return false;

		szIconFileName[24] = '\0';
	}
	// Recv Data End

	CPythonQuest& rkQuest = CPythonQuest::Instance();

	// Process Start
	if (QUEST_PACKET_TYPE_END == byQuestPacketType)
		rkQuest.DeleteQuestInstance(QuestInfo.index);
	else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType)
	{
		if (!rkQuest.IsQuest(QuestInfo.index))
		{
#ifdef ENABLE_QUEST_RENEWAL
			rkQuest.MakeQuest(QuestInfo.index, QuestInfo.c_index);
#else
			rkQuest.MakeQuest(QuestInfo.index);
#endif
		}

		if (strlen(szTitle) > 0)
			rkQuest.SetQuestTitle(QuestInfo.index, szTitle);
		if (strlen(szClockName) > 0)
			rkQuest.SetQuestClockName(QuestInfo.index, szClockName);
		if (strlen(szCounterName) > 0)
			rkQuest.SetQuestCounterName(QuestInfo.index, szCounterName);
		if (strlen(szIconFileName) > 0)
			rkQuest.SetQuestIconFileName(QuestInfo.index, szIconFileName);

		if (c_rFlag & QUEST_SEND_CLOCK_VALUE)
			rkQuest.SetQuestClockValue(QuestInfo.index, iClockValue);
		if (c_rFlag & QUEST_SEND_COUNTER_VALUE)
			rkQuest.SetQuestCounterValue(QuestInfo.index, iCounterValue);
	}
	else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType)
	{
		CPythonQuest::SQuestInstance QuestInstance;
		QuestInstance.dwIndex = QuestInfo.index;
		QuestInstance.strTitle = szTitle;
		QuestInstance.strClockName = szClockName;
		QuestInstance.iClockValue = iClockValue;
		QuestInstance.strCounterName = szCounterName;
		QuestInstance.iCounterValue = iCounterValue;
		QuestInstance.strIconFileName = szIconFileName;
		CPythonQuest::Instance().RegisterQuestInstance(QuestInstance);
	}
	// Process Start End

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest", Py_BuildValue("()"));
	return true;
}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
bool CPythonNetworkStream::SendLanguageChangePacket(uint8_t bLanguage)
{
	if (bLanguage > LANGUAGE_MAX_NUM || bLanguage == LANGUAGE_NONE)
	{
		TraceError("Invalid language selection - %u", bLanguage);
		return false;
	}

	UpdateLanguageLocale(bLanguage);

	TPacketCGLanguageChange packet;
	packet.bHeader = HEADER_CG_LANGUAGE_CHANGE;
	packet.bLanguage = bLanguage;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendTargetLanguageRequestPacket(const char* c_szName)
{
	TPacketCGTargetLanguageRequest packet;
	packet.bHeader = HEADER_CG_TARGET_LANGUAGE_REQUEST;
	strncpy(packet.szName, c_szName, sizeof(packet.szName));

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvTargetLanguageResult()
{
	TPacketGCTargetLanguageResult packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AttachLanguageImage", Py_BuildValue("(si)", packet.szName, packet.bLanguage));

	return true;
}
#endif

bool CPythonNetworkStream::RecvQuestConfirmPacket()
{
	TPacketGCQuestConfirm kQuestConfirmPacket;
	if (!Recv(sizeof(kQuestConfirmPacket), &kQuestConfirmPacket))
	{
		Tracen("RecvQuestConfirmPacket Error");
		return false;
	}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	CPythonLocale::Instance().FormatString(kQuestConfirmPacket.msg, sizeof(kQuestConfirmPacket.msg));
#endif

	PyObject* poArg = Py_BuildValue("(sii)", kQuestConfirmPacket.msg, kQuestConfirmPacket.timeout, kQuestConfirmPacket.requestPID);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnQuestConfirm", poArg);
	return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild()
{
	TPacketGCBlank blank;
	if (!Recv(sizeof(blank), &blank))
	{
		Tracen("RecvRequestMakeGuild Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName", Py_BuildValue("()"));

	return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendExchangeStartPacket(uint32_t vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_START;
	packet.arg1 = vid;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_start_packet Error\n");
		return false;
	}

	Tracef("send_trade_start_packet   vid %d \n", vid);
	return SendSequence();
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendExchangeElkAddPacket(uint32_t elk, uint32_t cheque)
#else
bool CPythonNetworkStream::SendExchangeElkAddPacket(uint32_t elk)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_ELK_ADD;
	packet.arg1 = elk;
#ifdef ENABLE_CHEQUE_SYSTEM
	packet.cheque = cheque;
#endif

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_elk_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
bool CPythonNetworkStream::SendWonExchangeSellPacket(uint16_t wValue)
{
	TPacketCGWonExchange kWonExchangePacket;
	kWonExchangePacket.bSubHeader = WON_EXCHANGE_CG_SUBHEADER_SELL;
	kWonExchangePacket.wValue = wValue;

	if (!Send(sizeof(TPacketCGWonExchange), &kWonExchangePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendWonExchangeBuyPacket(uint16_t wValue)
{
	TPacketCGWonExchange kWonExchangePacket;
	kWonExchangePacket.bSubHeader = WON_EXCHANGE_CG_SUBHEADER_BUY;
	kWonExchangePacket.wValue = wValue;

	if (!Send(sizeof(TPacketCGWonExchange), &kWonExchangePacket))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos, uint8_t byDisplayPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_ITEM_ADD;
	packet.Pos = ItemPos;
	packet.arg2 = byDisplayPos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(uint8_t pos) const
{
	assert(!"Can't be called function - CPythonNetworkStream::SendExchangeItemDelPacket");
	return true;
}

bool CPythonNetworkStream::SendExchangeAcceptPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_ACCEPT;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_accept_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeExitPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_CANCEL;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_exit_packet Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_GUILDBANK_LOG
void CPythonNetworkStream::GuildBankInfoOpen()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenGuildBankInfo", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendGuildBankInfoOpen()
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REFRESH;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildBankLogInfoRefresh", Py_BuildValue("()"));
	//Tracef(" SendGuildLogRefreshPacket\n");

	return SendSequence();
}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
void CPythonNetworkStream::GuildDonateInfoOpen()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildDonateInfoRefresh", Py_BuildValue("()"));
}
#endif

#ifdef ENABLE_FLOWER_EVENT
bool CPythonNetworkStream::SendFlowerEventExchange(uint8_t id)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFlowerEventSend packet{};
	packet.header = HEADER_CG_FLOWER_EVENT_EXCHANGE;
	packet.id = id;

	if (!Send(sizeof(TPacketCGFlowerEventSend), &packet))
	{
		Tracef("SendFlowerEventExchange Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
bool CPythonNetworkStream::RecvShopStashSync()
{
	TPacketGCShopStashSync p;
	if (!Recv(sizeof(TPacketGCShopStashSync), &p))
		return false;

	CPythonShop& rkShop = CPythonShop::Instance();
	rkShop.SetStashValue(p.value);
#	ifdef ENABLE_CHEQUE_SYSTEM
	rkShop.SetChequeStashValue(p.cheque);
#	endif

	__RefreshShopInfoWindow();

	return true;
}

bool CPythonNetworkStream::RecvShopOfflineTimeSync()
{
	TPacketGCShopOffTimeSync p;
	if (!Recv(sizeof(TPacketGCShopOffTimeSync), &p))
		return false;

	CPythonShop& rkShop = CPythonShop::Instance();
	rkShop.SetOfflineMinutes(p.value);

	__RefreshShopInfoWindow();

	return true;
}

bool CPythonNetworkStream::RecvShopPositionSync()
{
	TPacketGCShopSyncPos p;
	if (!Recv(sizeof(TPacketGCShopSyncPos), &p))
		return false;

	CPythonShop& rkShop = CPythonShop::Instance();
	rkShop.SetLocation(p.channel, p.xGlobal, p.yGlobal);

	__RefreshShopInfoWindow();

	return true;
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH
bool CPythonNetworkStream::SendSkillBookCombinationPacket(PyObject* cell, uint8_t bIndexComb)
{
	if (!__CanActMainInstance())
		return true;

	if (bIndexComb < ATTR67_SUB_HEADER_CLOSE && bIndexComb > ATTR67_SUB_HEADER_SKILLBOOK_COMB)
		return false;

	TPacketCGAttr67Send packet{};
	packet.header = HEADER_CG_ATTR_6TH_7TH;
	packet.subheader = bIndexComb;

	if (bIndexComb == ATTR67_SUB_HEADER_SKILLBOOK_COMB)
	{
		int size = PyList_Size(cell);
		PyObject* v3;
		int v6;

		for (int i = 0; i < size; ++i)
		{
			v3 = PyList_GetItem(cell, i);
			v6 = PyInt_AsLong(v3);

			if (v6 == -1)
			{
				return false;
			}

			packet.bCell[i] = v6;
		}
	}

	if (!Send(sizeof(TPacketCGAttr67Send), &packet))
	{
		Tracef("SendSkillBookCombinationPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendRegistFragmentPacket(uint8_t bRegistSlot)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGAttr67Send packet{};
	packet.header = HEADER_CG_ATTR_6TH_7TH;
	packet.subheader = ATTR67_SUB_HEADER_GET_FRAGMENT;
	packet.bRegistSlot = bRegistSlot;

	if (!Send(sizeof(TPacketCGAttr67Send), &packet))
	{
		Tracef("SendRegistFragmentPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAttr67ClosePacket(uint8_t bIndexComb)
{
	if (!__CanActMainInstance())
		return true;

	if (bIndexComb < ATTR67_SUB_HEADER_CLOSE && bIndexComb > ATTR67_SUB_HEADER_OPEN)
		return false;

	TPacketCGAttr67Send packet{};
	packet.header = HEADER_CG_ATTR_6TH_7TH;
	packet.subheader = bIndexComb;

	if (!Send(sizeof(TPacketCGAttr67Send), &packet))
	{
		Tracef("SendAttr67ClosePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAttr67AddPacket(uint8_t bRegistSlot, uint8_t bFragmentCount, uint16_t wCellAdditive, uint8_t bCountAdditive)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGAttr67Send packet{};
	packet.header = HEADER_CG_ATTR_6TH_7TH;
	packet.subheader = ATTR67_SUB_HEADER_ADD;
	packet.bRegistSlot = bRegistSlot;
	packet.bFragmentCount = bFragmentCount;
	packet.wCellAdditive = wCellAdditive;
	packet.bCountAdditive = bCountAdditive;

	if (!Send(sizeof(TPacketCGAttr67Send), &packet))
	{
		Tracef("SendAddAttrPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvAttr67Packet()
{
	TPacketGCAttr67Receive p;

	if (!Recv(sizeof(p), &p))
		return false;

	switch (p.subheader)
	{
	case ATTR67_SUB_HEADER_FRAGMENT_RECEIVE:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddMaterialSlot", Py_BuildValue("(i)", p.fragmentVnum));
	}
	break;
	}

	return true;
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
bool CPythonNetworkStream::RecvElementsSpellPacket()
{
	Tracef("RecvElementsSpellPacket Error\n");

	TPacketGCElementsSpell ElementsSpellPacket;

	if (!Recv(sizeof(ElementsSpellPacket), &ElementsSpellPacket))
		return false;

	switch (ElementsSpellPacket.subheader)
	{
	case ELEMENT_SPELL_SUB_HEADER_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ELEMENTS_SPELL_OPEN", Py_BuildValue("(ibii)", ElementsSpellPacket.pos, ElementsSpellPacket.func, ElementsSpellPacket.cost, ElementsSpellPacket.grade_add));
		break;

	case ELEMENT_SPELL_SUB_HEADER_OPEN_CHANGE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ELEMENTS_SPELL_OPEN_CHANGE", Py_BuildValue("(ii)", ElementsSpellPacket.pos, ElementsSpellPacket.cost));
		break;
	}

	return true;
}

bool CPythonNetworkStream::SendElementsSpellClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGElementsSpell packet{};
	packet.header = HEADER_CG_ELEMENTS_SPELL;
	packet.subheader = ELEMENTS_SPELL_SUB_HEADER_CLOSE;

	if (!Send(sizeof(TPacketCGElementsSpell), &packet)) {
		Tracef("SendElementsSpellClose Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendElementsSpellAdd(int pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGElementsSpell packet{};
	packet.header = HEADER_CG_ELEMENTS_SPELL;
	packet.subheader = ELEMENTS_SPELL_SUB_HEADER_ADD_BONUS;
	packet.pos = pos;

	if (!Send(sizeof(TPacketCGElementsSpell), &packet))
	{
		Tracef("SendElementsSpellAdd Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendElementsSpellChange(int pos, int type_select)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGElementsSpell packet{};
	packet.header = HEADER_CG_ELEMENTS_SPELL;
	packet.subheader = ELEMENTS_SPELL_SUB_HEADER_CHANGE_BONUS;
	packet.pos = pos;
	packet.type_select = type_select;

	if (!Send(sizeof(TPacketCGElementsSpell), &packet)) {
		Tracef("SendElementsSpellChange Error\n");
		return false;
	}

	return SendSequence();
}
#endif

// PointReset 개임시
bool CPythonNetworkStream::SendPointResetPacket()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking() const
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	if (!pkInstMain->IsAttacking())
		return false;

	return true;
}

bool CPythonNetworkStream::RecvScriptPacket()
{
	TPacketGCScript ScriptPacket;

	if (!Recv(sizeof(TPacketGCScript), &ScriptPacket))
	{
		TraceError("RecvScriptPacket_RecvError");
		return false;
	}

	if (ScriptPacket.size < sizeof(TPacketGCScript))
	{
		TraceError("RecvScriptPacket_SizeError");
		return false;
	}

	ScriptPacket.size -= sizeof(TPacketGCScript);

	static std::string str;
	str = "";
	str.resize(ScriptPacket.size + 1);

	if (!Recv(ScriptPacket.size, &str[0]))
		return false;

	str[str.size() - 1] = '\0';

	TraceError("QuestStr: %s", str.c_str());
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	CPythonLocale::Instance().FormatString(str);
#endif
	TraceError("After QuestStr: %s", str.c_str());

	int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(str);
	if (-1 != iIndex)
	{
		CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);
		Instance().OnScriptEventStart(ScriptPacket.skin, iIndex);
	}

	return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer)
{
	TPacketCGScriptAnswer ScriptAnswer;

	ScriptAnswer.header = HEADER_CG_SCRIPT_ANSWER;
	ScriptAnswer.answer = static_cast<uint8_t>(iAnswer);
	if (!Send(sizeof(TPacketCGScriptAnswer), &ScriptAnswer))
	{
		Tracen("Send Script Answer Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex)
{
	TPacketCGScriptButton ScriptButton;

	ScriptButton.header = HEADER_CG_SCRIPT_BUTTON;
	ScriptButton.idx = iIndex;
	if (!Send(sizeof(TPacketCGScriptButton), &ScriptButton))
	{
		Tracen("Send Script Button Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char* c_szName)
{
	TPacketCGAnswerMakeGuild Packet{};

	Packet.header = HEADER_CG_ANSWER_MAKE_GUILD;
	strncpy(Packet.guild_name, c_szName, GUILD_NAME_MAX_LEN);
	Packet.guild_name[GUILD_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendAnswerMakeGuild Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char* c_szString)
{
	TPacketCGQuestInputString Packet{};
	Packet.bHeader = HEADER_CG_QUEST_INPUT_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputStringPacket Error");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_OX_RENEWAL
bool CPythonNetworkStream::SendQuestInputStringLongPacket(const char* c_szString)
{
	TPacketCGQuestInputLongString Packet{};
	Packet.bHeader = HEADER_CG_QUEST_INPUT_LONG_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_LONG_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputLongStringPacket Error");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendQuestConfirmPacket(uint8_t byAnswer, uint32_t dwPID)
{
	TPacketCGQuestConfirm kPacket{};
	kPacket.header = HEADER_CG_QUEST_CONFIRM;
	kPacket.answer = byAnswer;
	kPacket.requestPID = dwPID;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracen("SendQuestConfirmPacket Error");
		return false;
	}

	Tracenf(" SendQuestConfirmPacket : %d, %d", byAnswer, dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::RecvSkillLevel()
{
	TPacketGCSkillLevel packet;

	if (!Recv(sizeof(TPacketGCSkillLevel), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevel - RecvError");
		return false;
	}

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

	rkPlayer.SetSkill(7, 0);
	rkPlayer.SetSkill(8, 0);

	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		TPlayerSkill& rPlayerSkill = packet.skills[i];

		if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);

#ifdef ENABLE_78TH_SKILL
		if (i >= 221 && i <= 229 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		//TODO: 8 Skill Boost 
		if (i >= 236 && i <= 244 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);
#endif

		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
	}

	__RefreshSkillWindow();
	__RefreshStatus();
	return true;
}

bool CPythonNetworkStream::RecvDamageInfoPacket()
{
	TPacketGCDamageInfo DamageInfoPacket;

	if (!Recv(sizeof(TPacketGCDamageInfo), &DamageInfoPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	CInstanceBase* pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(DamageInfoPacket.dwVID);
	bool bSelf = (pInstTarget == CPythonCharacterManager::Instance().GetMainInstancePtr());
	bool bTarget = (pInstTarget == m_pInstTarget);
	if (pInstTarget)
	{
		if (DamageInfoPacket.damage >= 0)
			pInstTarget->AddDamageEffect(DamageInfoPacket.damage, DamageInfoPacket.flag, bSelf, bTarget);
		else
			TraceError("Damage is equal or below 0.");
	}

	return true;
}
bool CPythonNetworkStream::RecvTargetPacket()
{
	TPacketGCTarget TargetPacket;

	if (!Recv(sizeof(TPacketGCTarget), &TargetPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	const CInstanceBase* pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase* pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(TargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			if (pInstTarget->IsShop())
			{
				m_pInstTarget = pInstTarget;
				return true;
			}
#endif

			if (pInstTarget->IsBuilding())
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
			else if (pInstTarget->IsPC())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard",
					Py_BuildValue("(biiii)", true, TargetPacket.dwVID,
						((TargetPacket.icurHP > 0) ? MINMAX(0, ((TargetPacket.icurHP * 100) / (TargetPacket.iMaxHP)), 100) : 0),
						TargetPacket.icurHP, TargetPacket.iMaxHP)
				);
#endif

#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT_ADD)
				PyObject* oElementsDict = PyDict_New();
				if (oElementsDict)
				{
					for (uint8_t i = 0; i < CPythonNonPlayer::MOB_ELEMENTAL_MAX_NUM; i++)
					{
						PyDict_SetItem(oElementsDict, Py_BuildValue("i", i), Py_BuildValue("i", TargetPacket.cElementalFlags[i]));
					}
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowTargetElementEnchant", Py_BuildValue("(O)", oElementsDict));
				}
#endif

#ifdef ENABLE_TARGET_AFFECT_SHOWER
				for (int i = 0; i < TARGET_AFFECT_MAX_NUM; ++i)
				{
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetAffect",
						Py_BuildValue("(iii)", TargetPacket.dwVID, TargetPacket.affects[i].dwAffectNum, TargetPacket.affects[i].lDuration));
				}
#endif
			}
			else if (pInstPlayer->CanViewTargetHP(*pInstTarget))
			{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard",
					Py_BuildValue("(biiii)", false, TargetPacket.dwVID,
						((TargetPacket.icurHP > 0) ? MINMAX(0, static_cast<uint64_t>((static_cast<int64_t>(TargetPacket.icurHP * static_cast<int64_t>(100)) / static_cast<int64_t>(TargetPacket.iMaxHP))), 100) : 0),
						TargetPacket.icurHP, TargetPacket.iMaxHP)
				);
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard", Py_BuildValue("(ii)", TargetPacket.dwVID, TargetPacket.bHPPercent));
#endif

#ifdef ENABLE_TARGET_AFFECT_SHOWER
				for (int i = 0; i < TARGET_AFFECT_MAX_NUM; ++i)
				{
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetAffect",
						Py_BuildValue("(iii)", TargetPacket.dwVID, TargetPacket.affects[i].dwAffectNum, TargetPacket.affects[i].lDuration));
				}
#endif
			}
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

			m_pInstTarget = pInstTarget;
		}
	}
	else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

	return true;
}

#ifdef ENABLE_SEND_TARGET_INFO
bool CPythonNetworkStream::RecvTargetInfoPacket()
{
	TPacketGCTargetInfo pInfoTargetPacket;

	if (!Recv(sizeof(TPacketGCTargetInfo), &pInfoTargetPacket))
	{
		Tracen("Recv Info Target Packet Error");
		return false;
	}

	CInstanceBase* pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase* pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(pInfoTargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			if (pInstTarget->IsEnemy() || pInstTarget->IsStone())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddTargetMonsterDropInfo",
					Py_BuildValue("(iii)", pInfoTargetPacket.race, pInfoTargetPacket.dwVnum, pInfoTargetPacket.count));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshTargetMonsterDropInfo", Py_BuildValue("(i)", pInfoTargetPacket.race));
			}
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

			// m_pInstTarget = pInstTarget;
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Recv

bool CPythonNetworkStream::SendAttackPacket(uint32_t uMotAttack, uint32_t dwVIDVictim)
{
	if (!__CanActMainInstance())
		return true;

#ifdef ATTACK_TIME_LOG
	static uint32_t prevTime = timeGetTime();
	uint32_t curTime = timeGetTime();
	TraceError("TIME: %.4f(%.4f) ATTACK_PACKET: %d TARGET: %d", curTime / 1000.0f, (curTime - prevTime) / 1000.0f, uMotAttack, dwVIDVictim);
	prevTime = curTime;
#endif

	if (!__IsPlayerAttacking())	//@fixme
		return true;

	TPacketCGAttack kPacketAtk{};

	kPacketAtk.header = HEADER_CG_ATTACK;
	kPacketAtk.bType = uMotAttack;
	kPacketAtk.dwVictimVID = dwVIDVictim;

#ifdef ENABLE_AUTO_SYSTEM
	CPythonPlayer::Instance().SetLastAttackTime(CTimer::Instance().GetCurrentSecond());
#endif

	if (!SendSpecial(sizeof(kPacketAtk), &kPacketAtk))
	{
		Tracen("Send Battle Attack Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSpecial(int nLen, void* pvBuf)
{
	uint8_t bHeader = *static_cast<uint8_t*>(pvBuf);

	switch (bHeader)
	{
	case HEADER_CG_ATTACK:
	{
		auto* pkPacketAtk = static_cast<TPacketCGAttack*>(pvBuf);
		pkPacketAtk->bCRCMagicCubeProcPiece = GetProcessCRCMagicCubePiece();
		pkPacketAtk->bCRCMagicCubeFilePiece = GetProcessCRCMagicCubePiece();
		return Send(nLen, pvBuf);
	}
	}

	return Send(nLen, pvBuf);
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	Tracef("VID [%d] Added to target settings\n", kPacket.dwShooterVID);

	CPythonCharacterManager& rpcm = CPythonCharacterManager::Instance();

	CInstanceBase* pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifndef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase* pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(pTarget->GetGraphicThingInstancePtr());
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(D3DXVECTOR3(kPacket.lX, kPacket.lY, h));
	}

	return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	CPythonCharacterManager& rpcm = CPythonCharacterManager::Instance();

	CInstanceBase* pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifdef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase* pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(pTarget->GetGraphicThingInstancePtr());
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(D3DXVECTOR3(kPacket.lX, kPacket.lY, h));
	}

	return true;
}

bool CPythonNetworkStream::SendShootPacket(uint32_t uSkill)
{
	TPacketCGShoot kPacketShoot{};
	kPacketShoot.bHeader = HEADER_CG_SHOOT;
	kPacketShoot.bType = uSkill;

	if (!Send(sizeof(kPacketShoot), &kPacketShoot))
	{
		Tracen("SendShootPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAddFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition& kPPosTarget)
{
	TPacketCGFlyTargeting packet{};

	packet.bHeader = HEADER_CG_ADD_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}


bool CPythonNetworkStream::SendFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition& kPPosTarget)
{
	TPacketCGFlyTargeting packet{};

	packet.bHeader = HEADER_CG_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvCreateFlyPacket()
{
	TPacketGCCreateFly kPacket;
	if (!Recv(sizeof(TPacketGCCreateFly), &kPacket))
		return false;

	CFlyingManager& rkFlyMgr = CFlyingManager::Instance();
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* pkStartInst = rkChrMgr.GetInstancePtr(kPacket.dwStartVID);
	CInstanceBase* pkEndInst = rkChrMgr.GetInstancePtr(kPacket.dwEndVID);
	if (!pkStartInst || !pkEndInst)
		return true;

	rkFlyMgr.CreateIndexedFly(kPacket.bType, pkStartInst->GetGraphicThingInstancePtr(), pkEndInst->GetGraphicThingInstancePtr());

	return true;
}

bool CPythonNetworkStream::SendTargetPacket(uint32_t dwVID)
{
	TPacketCGTarget packet{};
	packet.header = HEADER_CG_TARGET;
	packet.dwVID = dwVID;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send Target Packet Error");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
bool CPythonNetworkStream::SendSkillColorPacket(uint8_t bSkillSlot, uint32_t dwColor1, uint32_t dwColor2, uint32_t dwColor3, uint32_t dwColor4, uint32_t dwColor5)
{
	TPacketCGSkillColor pack{};
	pack.bheader = HEADER_CG_SKILL_COLOR;
	pack.skill = bSkillSlot;
	pack.col1 = dwColor1;
	pack.col2 = dwColor2;
	pack.col3 = dwColor3;
	pack.col4 = dwColor4;
	pack.col5 = dwColor5;

	if (!Send(sizeof(pack), &pack))
	{
		Tracen("Send Skill Color Packet Error");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendSyncPositionElementPacket(uint32_t dwVictimVID, uint32_t dwVictimX, uint32_t dwVictimY)
{
	TPacketCGSyncPositionElement kSyncPos{};
	kSyncPos.dwVID = dwVictimVID;
	kSyncPos.lX = dwVictimX;
	kSyncPos.lY = dwVictimY;

	__LocalPositionToGlobalPosition(kSyncPos.lX, kSyncPos.lY);

	if (!Send(sizeof(kSyncPos), &kSyncPos))
	{
		Tracen("CPythonNetworkStream::SendSyncPositionElementPacket - ERROR");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMessenger()
{
	TPacketGCMessenger p;
	if (!Recv(sizeof(p), &p))
		return false;

	int iSize = p.size - sizeof(p);
	char char_name[24 + 1]{};

	switch (p.subheader)
	{
	case MESSENGER_SUBHEADER_GC_LIST:
	{
		TPacketGCMessengerListOnline on{};
		while (iSize)
		{
			if (!Recv(sizeof(TPacketGCMessengerListOffline), &on))
				return false;

			if (!Recv(on.length, char_name))
				return false;

			char_name[on.length] = 0;

			if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
				CPythonMessenger::Instance().OnFriendLogin(char_name);
			else
				CPythonMessenger::Instance().OnFriendLogout(char_name);

			if (on.connected & MESSENGER_CONNECTED_STATE_MOBILE)
				CPythonMessenger::Instance().SetMobile(char_name, TRUE);

			iSize -= sizeof(TPacketGCMessengerListOffline);
			iSize -= on.length;
		}
		break;
	}

	case MESSENGER_SUBHEADER_GC_LOGIN:
	{
		TPacketGCMessengerLogin p;
		if (!Recv(sizeof(p), &p))
			return false;
		if (!Recv(p.length, char_name))
			return false;
		char_name[p.length] = 0;
		CPythonMessenger::Instance().OnFriendLogin(char_name);
		__RefreshTargetBoardByName(char_name);
		break;
	}

	case MESSENGER_SUBHEADER_GC_LOGOUT:
	{
		TPacketGCMessengerLogout logout;
		if (!Recv(sizeof(logout), &logout))
			return false;
		if (!Recv(logout.length, char_name))
			return false;
		char_name[logout.length] = 0;
		CPythonMessenger::Instance().OnFriendLogout(char_name);
		break;
	}

#ifdef ENABLE_GM_MESSENGER_LIST
	case MESSENGER_SUBHEADER_GC_GM_LIST:
	{
		TPacketGCMessengerGMListOnline gm_on;
		while (iSize)
		{
			if (!Recv(sizeof(TPacketGCMessengerGMListOffline), &gm_on))
				return false;

			if (!Recv(gm_on.length, char_name))
				return false;

			char_name[gm_on.length] = 0;

			if (gm_on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
				CPythonMessenger::Instance().OnGMLogin(char_name);
			else
				CPythonMessenger::Instance().OnGMLogout(char_name);

			iSize -= sizeof(TPacketGCMessengerGMListOffline);
			iSize -= gm_on.length;
		}
		break;
	}

	case MESSENGER_SUBHEADER_GC_GM_LOGIN:
	{
		TPacketGCMessengerLogin gm_p;
		if (!Recv(sizeof(gm_p), &gm_p))
			return false;
		if (!Recv(gm_p.length, char_name))
			return false;
		char_name[gm_p.length] = 0;
		CPythonMessenger::Instance().OnGMLogin(char_name);
		__RefreshTargetBoardByName(char_name);
		break;
	}

	case MESSENGER_SUBHEADER_GC_GM_LOGOUT:
	{
		TPacketGCMessengerLogout gm_logout;
		if (!Recv(sizeof(gm_logout), &gm_logout))
			return false;
		if (!Recv(gm_logout.length, char_name))
			return false;
		char_name[gm_logout.length] = 0;
		CPythonMessenger::Instance().OnGMLogout(char_name);
		break;
	}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	case MESSENGER_SUBHEADER_GC_BLOCK_LIST:
	{
		TPacketGCMessengerBlockListOnline block_on;
		while (iSize)
		{
			if (!Recv(sizeof(TPacketGCMessengerBlockListOffline), &block_on))
				return false;

			if (!Recv(block_on.length, char_name))
				return false;

			char_name[block_on.length] = 0;

			if (block_on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
				CPythonMessenger::Instance().OnBlockLogin(char_name);
			else
				CPythonMessenger::Instance().OnBlockLogout(char_name);

			iSize -= sizeof(TPacketGCMessengerBlockListOffline);
			iSize -= block_on.length;
		}
		break;
	}

	case MESSENGER_SUBHEADER_GC_BLOCK_LOGIN:
	{
		TPacketGCMessengerLogin block_p;
		if (!Recv(sizeof(block_p), &block_p))
			return false;
		if (!Recv(block_p.length, char_name))
			return false;
		char_name[block_p.length] = 0;
		CPythonMessenger::Instance().OnBlockLogin(char_name);
		__RefreshTargetBoardByName(char_name);
		break;
	}

	case MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT:
	{
		TPacketGCMessengerLogout block_logout;
		if (!Recv(sizeof(block_logout), &block_logout))
			return false;
		if (!Recv(block_logout.length, char_name))
			return false;
		char_name[block_logout.length] = 0;
		CPythonMessenger::Instance().OnBlockLogout(char_name);
		break;
	}
#endif

	case MESSENGER_SUBHEADER_GC_MOBILE:
	{
		uint8_t byState{};
		uint8_t byLength{};
		if (!Recv(sizeof(byState), &byState))
			return false;
		if (!Recv(sizeof(byLength), &byLength))
			return false;
		if (!Recv(byLength, char_name))
			return false;
		char_name[byLength] = 0;
		CPythonMessenger::Instance().SetMobile(char_name, byState);
		break;
	}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Party

bool CPythonNetworkStream::SendPartyInvitePacket(uint32_t dwVID)
{
	TPacketCGPartyInvite kPartyInvitePacket{};
	kPartyInvitePacket.header = HEADER_CG_PARTY_INVITE;
	kPartyInvitePacket.vid = dwVID;

	if (!Send(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInvitePacket [%ud] - PACKET SEND ERROR", dwVID);
		return false;
	}

	Tracef(" << SendPartyInvitePacket : %d\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(uint32_t dwLeaderVID, uint8_t byAnswer)
{
	TPacketCGPartyInviteAnswer kPartyInviteAnswerPacket{};
	kPartyInviteAnswerPacket.header = HEADER_CG_PARTY_INVITE_ANSWER;
	kPartyInviteAnswerPacket.leader_pid = dwLeaderVID;
	kPartyInviteAnswerPacket.accept = byAnswer;

	if (!Send(sizeof(kPartyInviteAnswerPacket), &kPartyInviteAnswerPacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInviteAnswerPacket [%ud %ud] - PACKET SEND ERROR", dwLeaderVID, byAnswer);
		return false;
	}

	Tracef(" << SendPartyInviteAnswerPacket : %d, %d\n", dwLeaderVID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyRemovePacket(uint32_t dwPID)
{
	TPacketCGPartyRemove kPartyInviteRemove{};
	kPartyInviteRemove.header = HEADER_CG_PARTY_REMOVE;
	kPartyInviteRemove.pid = dwPID;

	if (!Send(sizeof(kPartyInviteRemove), &kPartyInviteRemove))
	{
		Tracenf("CPythonNetworkStream::SendPartyRemovePacket [%ud] - PACKET SEND ERROR", dwPID);
		return false;
	}

	Tracef(" << SendPartyRemovePacket : %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartySetStatePacket(uint32_t dwVID, uint8_t byState, uint8_t byFlag)
{
	TPacketCGPartySetState kPartySetState{};
	kPartySetState.byHeader = HEADER_CG_PARTY_SET_STATE;
	kPartySetState.dwVID = dwVID;
	kPartySetState.byState = byState;
	kPartySetState.byFlag = byFlag;

	if (!Send(sizeof(kPartySetState), &kPartySetState))
	{
		Tracenf("CPythonNetworkStream::SendPartySetStatePacket(%ud, %ud) - PACKET SEND ERROR", dwVID, byState);
		return false;
	}

	Tracef(" << SendPartySetStatePacket : %d, %d, %d\n", dwVID, byState, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(uint8_t bySkillIndex, uint32_t dwVID)
{
	TPacketCGPartyUseSkill kPartyUseSkill{};
	kPartyUseSkill.byHeader = HEADER_CG_PARTY_USE_SKILL;
	kPartyUseSkill.bySkillIndex = bySkillIndex;
	kPartyUseSkill.dwTargetVID = dwVID;

	if (!Send(sizeof(kPartyUseSkill), &kPartyUseSkill))
	{
		Tracenf("CPythonNetworkStream::SendPartyUseSkillPacket(%ud, %ud) - PACKET SEND ERROR", bySkillIndex, dwVID);
		return false;
	}

	Tracef(" << SendPartyUseSkillPacket : %d, %d\n", bySkillIndex, dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyParameterPacket(uint8_t byDistributeMode)
{
	TPacketCGPartyParameter kPartyParameter{};
	kPartyParameter.bHeader = HEADER_CG_PARTY_PARAMETER;
	kPartyParameter.bDistributeMode = byDistributeMode;

	if (!Send(sizeof(kPartyParameter), &kPartyParameter))
	{
		Tracenf("CPythonNetworkStream::SendPartyParameterPacket(%d) - PACKET SEND ERROR", byDistributeMode);
		return false;
	}

	Tracef(" << SendPartyParameterPacket : %d\n", byDistributeMode);
	return SendSequence();
}

bool CPythonNetworkStream::RecvPartyInvite()
{
	TPacketGCPartyInvite kPartyInvitePacket;
	if (!Recv(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
		return false;

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kPartyInvitePacket.leader_pid);
	if (!pInstance)
	{
		TraceError(" CPythonNetworkStream::RecvPartyInvite - Failed to find leader instance [%d]\n", kPartyInvitePacket.leader_pid);
		return true;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvPartyInviteQuestion",
		Py_BuildValue("(is)", kPartyInvitePacket.leader_pid, pInstance->GetNameString()));
	Tracef(" >> RecvPartyInvite : %d, %s\n", kPartyInvitePacket.leader_pid, pInstance->GetNameString());

	return true;
}

bool CPythonNetworkStream::RecvPartyAdd()
{
	TPacketGCPartyAdd kPartyAddPacket;
	if (!Recv(sizeof(kPartyAddPacket), &kPartyAddPacket))
		return false;

	CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid, kPartyAddPacket.name);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember",
		Py_BuildValue("(is"
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
			"ii"
#endif
			")",
			kPartyAddPacket.pid, kPartyAddPacket.name
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
			, kPartyAddPacket.mapidx
			, kPartyAddPacket.channel
#endif
		));

	Tracef(" >> RecvPartyAdd : %d, %s\n", kPartyAddPacket.pid, kPartyAddPacket.name);

	return true;
}

bool CPythonNetworkStream::RecvPartyUpdate()
{
	TPacketGCPartyUpdate kPartyUpdatePacket;
	if (!Recv(sizeof(kPartyUpdatePacket), &kPartyUpdatePacket))
		return false;

	CPythonPlayer::TPartyMemberInfo* pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(kPartyUpdatePacket.pid, &pPartyMemberInfo))
		return true;

	uint8_t byOldState = pPartyMemberInfo->byState;

	CPythonPlayer::Instance().UpdatePartyMemberInfo(kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);
	for (int i = 0; i < PARTY_AFFECT_SLOT_MAX_NUM; ++i)
		CPythonPlayer::Instance().UpdatePartyMemberAffect(kPartyUpdatePacket.pid, i, kPartyUpdatePacket.affects[i]);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UpdatePartyMemberInfo", Py_BuildValue("(i)", kPartyUpdatePacket.pid));

	// 만약 리더가 바뀌었다면, TargetBoard 의 버튼을 업데이트 한다.
	uint32_t dwVID;
	if (CPythonPlayer::Instance().PartyMemberPIDToVID(kPartyUpdatePacket.pid, &dwVID))
		if (byOldState != kPartyUpdatePacket.state)
			__RefreshTargetBoardByVID(dwVID);

	return true;
}

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
bool CPythonNetworkStream::RecvPartyPositionInfo()
{
	TPacketGCPartyPosition Packet;
	if (!Recv(sizeof(Packet), &Packet))
	{
		Tracen("RecvPartyPositionInfo Error");
		return false;
	}

	for (auto iPacketSize = Packet.wSize - sizeof(Packet); iPacketSize > 0; iPacketSize -= sizeof(SPartyPosition))
	{
		SPartyPosition PositionInfo;
		if (Recv(sizeof(PositionInfo), &PositionInfo))
			CPythonMiniMap::Instance().AddPartyPositionInfo(PositionInfo);
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvPartyRemove()
{
	TPacketGCPartyRemove kPartyRemovePacket;
	if (!Recv(sizeof(kPartyRemovePacket), &kPartyRemovePacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember", Py_BuildValue("(i)", kPartyRemovePacket.pid));
	Tracef(" >> RecvPartyRemove : %d\n", kPartyRemovePacket.pid);

	return true;
}

bool CPythonNetworkStream::RecvPartyLink()
{
	TPacketGCPartyLink kPartyLinkPacket;
	if (!Recv(sizeof(kPartyLinkPacket), &kPartyLinkPacket))
		return false;

	CPythonPlayer::Instance().LinkPartyMember(kPartyLinkPacket.pid, kPartyLinkPacket.vid);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember",
		Py_BuildValue("(ii"
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
			"ii"
#endif
			")",
			kPartyLinkPacket.pid, kPartyLinkPacket.vid
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
			, kPartyLinkPacket.mapIdx
			, kPartyLinkPacket.channel
#endif
		));

	Tracef(" >> RecvPartyLink : %d, %d\n", kPartyLinkPacket.pid, kPartyLinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyUnlink()
{
	TPacketGCPartyUnlink kPartyUnlinkPacket;
	if (!Recv(sizeof(kPartyUnlinkPacket), &kPartyUnlinkPacket))
		return false;

	CPythonPlayer::Instance().UnlinkPartyMember(kPartyUnlinkPacket.pid);

	if (CPythonPlayer::Instance().IsMainCharacterIndex(kPartyUnlinkPacket.vid))
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkAllPartyMember", Py_BuildValue("()"));
	else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkPartyMember", Py_BuildValue("(i)", kPartyUnlinkPacket.pid));

	Tracef(" >> RecvPartyUnlink : %d, %d\n", kPartyUnlinkPacket.pid, kPartyUnlinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyParameter()
{
	TPacketGCPartyParameter kPartyParameterPacket;
	if (!Recv(sizeof(kPartyParameterPacket), &kPartyParameterPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ChangePartyParameter",
		Py_BuildValue("(i)", kPartyParameterPacket.bDistributeMode));
	Tracef(" >> RecvPartyParameter : %d\n", kPartyParameterPacket.bDistributeMode);

	return true;
}

// Party
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Guild

bool CPythonNetworkStream::SendGuildAddMemberPacket(uint32_t dwVID)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_ADD_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwVID), &dwVID))
		return false;

	Tracef(" SendGuildAddMemberPacket\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(uint32_t dwPID)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REMOVE_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwPID), &dwPID))
		return false;

	Tracef(" SendGuildRemoveMemberPacket %d\n", dwPID);
	return SendSequence();
}

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
bool CPythonNetworkStream::SendGuildVoteChangeMaster(uint32_t dwPID)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GUILDMASTER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwPID), &dwPID))
		return false;

	Tracef(" SendGuildVoteChangeMaster %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildVoteLandAbndon(int iVID)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DELETE_LAND;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(iVID), &iVID))
		return false;

	Tracef(" SendGuildVoteLandAbndon %d\n", iVID);
	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(uint8_t byGradeNumber, const char* c_szName)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;

	char szName[GUILD_GRADE_NAME_MAX_LEN + 1];
	strncpy(szName, c_szName, GUILD_GRADE_NAME_MAX_LEN);
	szName[GUILD_GRADE_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;

	Tracef(" SendGuildChangeGradeNamePacket %d, %s\n", byGradeNumber, c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(uint8_t byGradeNumber, uint8_t byAuthority)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;
	if (!Send(sizeof(byAuthority), &byAuthority))
		return false;

	Tracef(" SendGuildChangeGradeAuthorityPacket %d, %d\n", byGradeNumber, byAuthority);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildOfferPacket(uint32_t dwExperience)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_OFFER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwExperience), &dwExperience))
		return false;

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildDonateInfoRefresh", Py_BuildValue("()"));
#endif

	Tracef(" SendGuildOfferPacket %d\n", dwExperience);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char* c_szMessage)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_POST_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	uint8_t bySize = uint8_t(strlen(c_szMessage)) + 1;
	if (!Send(sizeof(bySize), &bySize))
		return false;
	if (!Send(bySize, c_szMessage))
		return false;

	Tracef(" SendGuildPostCommentPacket %d, %s\n", bySize, c_szMessage);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(uint32_t dwIndex)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DELETE_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwIndex), &dwIndex))
		return false;

	Tracef(" SendGuildDeleteCommentPacket %d\n", dwIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(uint32_t dwHighestIndex)
{
	static uint32_t s_LastTime = timeGetTime() - 1001;

	if (timeGetTime() - s_LastTime < 1000)
		return true;
	s_LastTime = timeGetTime();

	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REFRESH_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	Tracef(" SendGuildRefreshCommentPacket %d\n", dwHighestIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(uint32_t dwPID, uint8_t byGrade)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byGrade), &byGrade))
		return false;

	Tracef(" SendGuildChangeMemberGradePacket %d, %d\n", dwPID, byGrade);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(uint32_t dwSkillID, uint32_t dwTargetVID)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_USE_SKILL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwSkillID), &dwSkillID))
		return false;
	if (!Send(sizeof(dwTargetVID), &dwTargetVID))
		return false;

	Tracef(" SendGuildUseSkillPacket %d, %d\n", dwSkillID, dwTargetVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(uint32_t dwPID, uint8_t byFlag)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byFlag), &byFlag))
		return false;

	Tracef(" SendGuildChangeMemberGeneralFlagPacket %d, %d\n", dwPID, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(uint32_t dwGuildID, uint8_t byAnswer)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwGuildID), &dwGuildID))
		return false;
	if (!Send(sizeof(byAnswer), &byAnswer))
		return false;

	Tracef(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(uint32_t dwMoney)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHARGE_GSP;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildChargeGSPPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(uint32_t dwMoney)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DEPOSIT_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildDepositMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(uint32_t dwMoney)
{
	TPacketCGGuild GuildPacket{};
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_WITHDRAW_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildWithdrawMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::RecvGuild()
{
	TPacketGCGuild GuildPacket;
	if (!Recv(sizeof(GuildPacket), &GuildPacket))
		return false;

	switch (GuildPacket.subheader)
	{
	case GUILD_SUBHEADER_GC_LOGIN:
	{
		uint32_t dwPID;
		if (!Recv(sizeof(uint32_t), &dwPID))
			return false;

		// Messenger
		CPythonGuild::TGuildMemberData* pGuildMemberData;
		if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
			if (CPythonPlayer::Instance().GetName() != pGuildMemberData->strName)
				CPythonMessenger::Instance().LoginGuildMember(pGuildMemberData->strName.c_str());

		break;
	}
	case GUILD_SUBHEADER_GC_LOGOUT:
	{
		uint32_t dwPID;
		if (!Recv(sizeof(uint32_t), &dwPID))
			return false;

		// Messenger
		CPythonGuild::TGuildMemberData* pGuildMemberData;
		if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
			if (CPythonPlayer::Instance().GetName() != pGuildMemberData->strName)
				CPythonMessenger::Instance().LogoutGuildMember(pGuildMemberData->strName.c_str());

		break;
	}
	case GUILD_SUBHEADER_GC_REMOVE:
	{
		uint32_t dwPID;
		if (!Recv(sizeof(dwPID), &dwPID))
			return false;

		// Main Player 일 경우 DeleteGuild
		if (CPythonGuild::Instance().IsMainPlayer(dwPID))
		{
			CPythonGuild::Instance().Destroy();
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteGuild", Py_BuildValue("()"));
			CPythonMessenger::Instance().RemoveAllGuildMember();
			__SetGuildID(0);
			__RefreshMessengerWindow();
			__RefreshTargetBoard();
			__RefreshCharacterWindow();
		}
		else
		{
			// Get Member Name
			std::string strMemberName;
			CPythonGuild::TGuildMemberData* pData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pData))
			{
				strMemberName = pData->strName;
				CPythonMessenger::Instance().RemoveGuildMember(pData->strName.c_str());
			}

			CPythonGuild::Instance().RemoveMember(dwPID);

			// Refresh
			__RefreshTargetBoardByName(strMemberName.c_str());
			__RefreshGuildWindowMemberPage();
		}

		Tracef(" <Remove> %d\n", dwPID);
		break;
	}
	case GUILD_SUBHEADER_GC_LIST:
	{
		int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

		for (; iPacketSize > 0;)
		{
			TPacketGCGuildSubMember memberPacket;
			if (!Recv(sizeof(memberPacket), &memberPacket))
				return false;

			char szName[CHARACTER_NAME_MAX_LEN + 1] = "";
			if (memberPacket.byNameFlag)
			{
				if (!Recv(sizeof(szName), &szName))
					return false;

				iPacketSize -= CHARACTER_NAME_MAX_LEN + 1;
			}
			else
			{
				CPythonGuild::TGuildMemberData* pMemberData;
				if (CPythonGuild::Instance().GetMemberDataPtrByPID(memberPacket.pid, &pMemberData))
					strncpy(szName, pMemberData->strName.c_str(), CHARACTER_NAME_MAX_LEN);
			}

			CPythonGuild::SGuildMemberData GuildMemberData;
			GuildMemberData.dwPID = memberPacket.pid;
			GuildMemberData.byGrade = memberPacket.byGrade;
			GuildMemberData.strName = szName;
			GuildMemberData.byJob = memberPacket.byJob;
			GuildMemberData.byLevel = memberPacket.byLevel;
			GuildMemberData.dwOffer = memberPacket.dwOffer;
			GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
			CPythonGuild::Instance().RegisterMember(GuildMemberData);

			// Messenger
			if (strcmp(szName, CPythonPlayer::Instance().GetName()) != 0)
				CPythonMessenger::Instance().AppendGuildMember(szName);

			__RefreshTargetBoardByName(szName);

			iPacketSize -= sizeof(memberPacket);
		}

		__RefreshGuildWindowInfoPage();
		__RefreshGuildWindowMemberPage();
		__RefreshMessengerWindow();
		__RefreshCharacterWindow();
		break;
	}
	case GUILD_SUBHEADER_GC_GRADE:
	{
		uint8_t byCount;
		if (!Recv(sizeof(byCount), &byCount))
			return false;

		for (uint8_t i = 0; i < byCount; ++i)
		{
			uint8_t byIndex;
			if (!Recv(sizeof(byCount), &byIndex))
				return false;
			TPacketGCGuildSubGrade GradePacket;
			if (!Recv(sizeof(GradePacket), &GradePacket))
				return false;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			CPythonLocale::Instance().FormatString(GradePacket.grade_name, sizeof(GradePacket.grade_name));
#endif

			auto data = CPythonGuild::SGuildGradeData(GradePacket.auth_flag, GradePacket.grade_name);
			CPythonGuild::Instance().SetGradeData(byIndex, data);
		}

		__RefreshGuildWindowGradePage();
		__RefreshGuildWindowMemberPageGradeComboBox();
		break;
	}
	case GUILD_SUBHEADER_GC_GRADE_NAME:
	{
		uint8_t byGradeNumber;
		if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
			return false;

		char szGradeName[GUILD_GRADE_NAME_MAX_LEN + 1] = "";
		if (!Recv(sizeof(szGradeName), &szGradeName))
			return false;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CPythonLocale::Instance().FormatString(szGradeName, sizeof(szGradeName));
#endif

		CPythonGuild::Instance().SetGradeName(byGradeNumber, szGradeName);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

		Tracef(" <Change Grade Name> %d, %s\n", byGradeNumber, szGradeName);
		__RefreshGuildWindowGradePage();
		__RefreshGuildWindowMemberPageGradeComboBox();
		break;
	}
	case GUILD_SUBHEADER_GC_GRADE_AUTH:
	{
		uint8_t byGradeNumber;
		if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
			return false;
		uint8_t byAuthorityFlag;
		if (!Recv(sizeof(byAuthorityFlag), &byAuthorityFlag))
			return false;

		CPythonGuild::Instance().SetGradeAuthority(byGradeNumber, byAuthorityFlag);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

		Tracef(" <Change Grade Authority> %d, %d\n", byGradeNumber, byAuthorityFlag);
		__RefreshGuildWindowGradePage();
		break;
	}
	case GUILD_SUBHEADER_GC_INFO:
	{
		TPacketGCGuildInfo GuildInfo;
		if (!Recv(sizeof(GuildInfo), &GuildInfo))
			return false;

		CPythonGuild::Instance().EnableGuild();
		CPythonGuild::TGuildInfo& rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
		strncpy(rGuildInfo.szGuildName, GuildInfo.name, GUILD_NAME_MAX_LEN);
		rGuildInfo.szGuildName[GUILD_NAME_MAX_LEN] = '\0';

		rGuildInfo.dwGuildID = GuildInfo.guild_id;
		rGuildInfo.dwMasterPID = GuildInfo.master_pid;
		rGuildInfo.dwGuildLevel = GuildInfo.level;
		rGuildInfo.dwCurrentExperience = GuildInfo.exp;
		rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
		rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
		rGuildInfo.dwGuildMoney = GuildInfo.gold;
		rGuildInfo.bHasLand = GuildInfo.hasLand;
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
		strncpy(rGuildInfo.goldCheckout, GuildInfo.goldCheckout, 48);
		rGuildInfo.goldCheckout[48] = '\0';

		strncpy(rGuildInfo.itemCheckout, GuildInfo.itemCheckout, 48);
		rGuildInfo.itemCheckout[48] = '\0';
#endif

		__RefreshGuildWindowInfoPage();
		break;
	}
	case GUILD_SUBHEADER_GC_COMMENTS:
	{
		uint8_t byCount;
		if (!Recv(sizeof(byCount), &byCount))
			return false;

		CPythonGuild::Instance().ClearComment();

		for (uint8_t i = 0; i < byCount; ++i)
		{
			uint32_t dwCommentID;
			if (!Recv(sizeof(dwCommentID), &dwCommentID))
				return false;

			char szName[CHARACTER_NAME_MAX_LEN + 1] = "";
			if (!Recv(sizeof(szName), &szName))
				return false;

			char szComment[GULID_COMMENT_MAX_LEN + 1] = "";
			if (!Recv(sizeof(szComment), &szComment))
				return false;

			CPythonGuild::Instance().RegisterComment(dwCommentID, szName, szComment);
		}

		__RefreshGuildWindowBoardPage();
		break;
	}
	case GUILD_SUBHEADER_GC_CHANGE_EXP:
	{
		uint8_t byLevel;
		if (!Recv(sizeof(byLevel), &byLevel))
			return false;
		uint32_t dwEXP;
		if (!Recv(sizeof(dwEXP), &dwEXP))
			return false;
		CPythonGuild::Instance().SetGuildEXP(byLevel, dwEXP);
		Tracef(" <ChangeEXP> %d, %d\n", byLevel, dwEXP);
		__RefreshGuildWindowInfoPage();
		break;
	}
	case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE:
	{
		uint32_t dwPID;
		if (!Recv(sizeof(dwPID), &dwPID))
			return false;
		uint8_t byGrade;
		if (!Recv(sizeof(byGrade), &byGrade))
			return false;
		CPythonGuild::Instance().ChangeGuildMemberGrade(dwPID, byGrade);
		Tracef(" <ChangeMemberGrade> %d, %d\n", dwPID, byGrade);
		__RefreshGuildWindowMemberPage();
		break;
	}
	case GUILD_SUBHEADER_GC_SKILL_INFO:
	{
		CPythonGuild::TGuildSkillData& rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
		if (!Recv(sizeof(rSkillData.bySkillPoint), &rSkillData.bySkillPoint))
			return false;
		if (!Recv(sizeof(rSkillData.bySkillLevel), rSkillData.bySkillLevel))
			return false;
		if (!Recv(sizeof(rSkillData.wGuildPoint), &rSkillData.wGuildPoint))
			return false;
		if (!Recv(sizeof(rSkillData.wMaxGuildPoint), &rSkillData.wMaxGuildPoint))
			return false;

		Tracef(" <SkillInfo> %d / %d, %d\n", rSkillData.bySkillPoint, rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
		__RefreshGuildWindowSkillPage();
		break;
	}
	case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL:
	{
		uint32_t dwPID;
		if (!Recv(sizeof(dwPID), &dwPID))
			return false;
		uint8_t byFlag;
		if (!Recv(sizeof(byFlag), &byFlag))
			return false;

		CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(dwPID, byFlag);
		Tracef(" <ChangeMemberGeneralFlag> %d, %d\n", dwPID, byFlag);
		__RefreshGuildWindowMemberPage();
		break;
	}
	case GUILD_SUBHEADER_GC_GUILD_INVITE:
	{
		uint32_t dwGuildID;
		if (!Recv(sizeof(dwGuildID), &dwGuildID))
			return false;
		char szGuildName[GUILD_NAME_MAX_LEN + 1];
		if (!Recv(GUILD_NAME_MAX_LEN + 1, &szGuildName))	//@fixme434
			return false;

		szGuildName[GUILD_NAME_MAX_LEN] = 0;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvGuildInviteQuestion", Py_BuildValue("(is)", dwGuildID, szGuildName));
		Tracef(" <Guild Invite> %d, %s\n", dwGuildID, szGuildName);
		break;
	}
	case GUILD_SUBHEADER_GC_WAR:
	{
		TPacketGCGuildWar kGuildWar;
		if (!Recv(sizeof(kGuildWar), &kGuildWar))
			return false;

		switch (kGuildWar.bWarState)
		{
		case GUILD_WAR_SEND_DECLARE:
			Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_SEND_DECLARE\n");
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnSendDeclare",
				Py_BuildValue("(i)", kGuildWar.dwGuildOpp));
			break;

		case GUILD_WAR_RECV_DECLARE:
			Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_RECV_DECLARE\n");
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnRecvDeclare",
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_NEW_WAR_OPTIONS)
				Py_BuildValue("(iiiii)", kGuildWar.dwGuildOpp, kGuildWar.bType, kGuildWar.bRound, kGuildWar.bPoints, kGuildWar.dwTime));
#else
				Py_BuildValue("(ii)", kGuildWar.dwGuildOpp, kGuildWar.bType));
#endif
			break;

		case GUILD_WAR_ON_WAR:
			Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_ON_WAR : %d, %d\n", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnStart",
				Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp));
			CPythonGuild::Instance().StartGuildWar(kGuildWar.dwGuildOpp);
#if defined(ENABLE_KEYCHANGE_SYSTEM) && defined(ENABLE_TAB_TARGETING)
			CPythonPlayer::Instance().SetWarState(true);
#endif
			break;

		case GUILD_WAR_END:
			Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_END\n");
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnEnd",
				Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp));
			CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
#if defined(ENABLE_KEYCHANGE_SYSTEM) && defined(ENABLE_TAB_TARGETING)
			CPythonPlayer::Instance().SetWarState(false);
#endif
			break;
		}
		break;
	}
	case GUILD_SUBHEADER_GC_GUILD_NAME:
	{
		uint32_t dwID{};
		char szGuildName[GUILD_NAME_MAX_LEN + 1]{};

#ifdef ENABLE_COLORED_GUILD_RANKS
		int guildRank;
#endif
		int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

		int nItemSize = sizeof(dwID) + GUILD_NAME_MAX_LEN;

		assert(iPacketSize % nItemSize == 0 && "GUILD_SUBHEADER_GC_GUILD_NAME");

		for (; iPacketSize > 0;)
		{
			if (!Recv(sizeof(dwID), &dwID))
				return false;

			if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
				return false;

#ifdef ENABLE_COLORED_GUILD_RANKS
			if (!Recv(sizeof(guildRank), &guildRank))
				return false;
#endif

			szGuildName[GUILD_NAME_MAX_LEN] = 0;

#ifdef ENABLE_COLORED_GUILD_RANKS
			CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName, guildRank);
#else
			CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName);
#endif
			iPacketSize -= nItemSize;
		}
		break;
	}
	case GUILD_SUBHEADER_GC_GUILD_WAR_LIST:
	{
		uint32_t dwSrcGuildID{};
		uint32_t dwDstGuildID{};

		int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
		int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

		assert(iPacketSize % nItemSize == 0 && "GUILD_SUBHEADER_GC_GUILD_WAR_LIST");

		for (; iPacketSize > 0;)
		{
			if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
				return false;

			if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
				return false;

			Tracef(" >> GulidWarList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
			CInstanceBase::InsertGVGKey(dwSrcGuildID, dwDstGuildID);
			CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
			iPacketSize -= nItemSize;
		}
		break;
	}
	case GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST:
	{
		uint32_t dwSrcGuildID{};
		uint32_t dwDstGuildID{};

		int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
		int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

		assert(iPacketSize % nItemSize == 0 && "GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST");

		for (; iPacketSize > 0;)
		{
			if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
				return false;

			if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
				return false;

			Tracef(" >> GulidWarEndList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
			CInstanceBase::RemoveGVGKey(dwSrcGuildID, dwDstGuildID);
			CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
			iPacketSize -= nItemSize;
		}
		break;
	}
	case GUILD_SUBHEADER_GC_WAR_POINT:
	{
		TPacketGuildWarPoint GuildWarPoint;
		if (!Recv(sizeof(GuildWarPoint), &GuildWarPoint))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_GuildWar_OnRecvPoint",
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILD_RANKING)
			Py_BuildValue("(iiii)", GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint, GuildWarPoint.lWinPoint));
#else
			Py_BuildValue("(iiii)", GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint, 0));
#endif
		break;
	}
	case GUILD_SUBHEADER_GC_MONEY_CHANGE:
	{
		uint32_t dwMoney;
		if (!Recv(sizeof(dwMoney), &dwMoney))
			return false;

		CPythonGuild::Instance().SetGuildMoney(dwMoney);

		__RefreshGuildWindowInfoPage();
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBaseInfoPageBankGold", Py_BuildValue("()"));
#endif
		Tracef(" >> Guild Money Change : %d\n", dwMoney);
		break;
	}
#ifdef ENABLE_GUILD_WAR_SCORE
	case GUILD_SUBHEADER_GC_WAR_INFO:
	{
		uint32_t winTypes[3];
		uint32_t drawTypes[3];
		uint32_t lossTypes[3];
		uint32_t ladderPoints;
		int rank;

		for (int i = 0; i < 3; ++i)
		{
			if (!Recv(sizeof(winTypes[i]), &winTypes[i]))
				return false;
		}
		for (int i = 0; i < 3; ++i)
		{
			if (!Recv(sizeof(lossTypes[i]), &lossTypes[i]))
				return false;
		}
		for (int i = 0; i < 3; ++i)
		{
			if (!Recv(sizeof(drawTypes[i]), &drawTypes[i]))
				return false;
		}

		if (!Recv(sizeof(ladderPoints), &ladderPoints))
			return false;
		if (!Recv(sizeof(rank), &rank))
			return false;

		CPythonGuild::Instance().SetGuildWarInfo(winTypes, lossTypes, drawTypes, ladderPoints, rank);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildWarInfoPage", Py_BuildValue("()"));
		__RefreshGuildWindowInfoPage();
		//Tracef(" >> Guild Money Change : %d\n", dwMoney);
		break;
	}
#endif
#ifdef ENABLE_GUILD_LAND_INFO
	case GUILD_SUBHEADER_GC_LAND_INFO:
	{
		int mindex;	//Baseinfo
		int guildAltar;	//Altar der Kraft LV.
		int guildAlchemist;	//Gildenalchemist
		int guildBlacksmith;	//Gildenschmied
		int storage_lv;	//Gildenlager LV.

		if (!Recv(sizeof(mindex), &mindex))
			return false;
		if (!Recv(sizeof(guildAltar), &guildAltar))
			return false;
		if (!Recv(sizeof(guildAlchemist), &guildAlchemist))
			return false;
		if (!Recv(sizeof(guildBlacksmith), &guildBlacksmith))
			return false;
		if (!Recv(sizeof(storage_lv), &storage_lv))
			return false;

		CPythonGuild::Instance().SetGuildLandInfo(mindex, guildAltar, guildAlchemist, guildBlacksmith, storage_lv);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBaseInfoPage", Py_BuildValue("()"));
		__RefreshGuildWindowInfoPage();
		break;
	}
#endif
#ifdef ENABLE_GUILDBANK_LOG
	case GUILD_SUBHEADER_GC_BANK_DELETE:
	{
		CPythonGuild::Instance().ClearBankLog();
		//TraceError("Clear bank log recv");
		break;
	}

	case GUILD_SUBHEADER_GC_BANK_INFO:
	{
		char szName[24];
		char szItemName[24];
		int itemAction;
		int itemCount;
		int datatype;
		//if (!Recv(sizeof(index), &index))
		//	return false;

		if (!Recv(sizeof(szName), &szName))
			return false;

		if (!Recv(sizeof(szItemName), &szItemName))
			return false;

		if (!Recv(sizeof(itemAction), &itemAction))
			return false;

		if (!Recv(sizeof(itemCount), &itemCount))
			return false;

		if (!Recv(sizeof(datatype), &datatype))
			return false;

		CPythonGuild::Instance().SetGuildBankLogs(szName, szItemName, itemAction, itemCount, datatype);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildBankLogInfoRefresh", Py_BuildValue("()"));
		CPythonGuild::Instance().IncreaseBankLog();

		__RefreshGuildWindowInfoPage();
		//TraceError("GUILD_SUBHEADER_GC_BANK_INFO received %s\n", szItemName);
		break;
	}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	case GUILD_SUBHEADER_GC_DONATE:
	{
		int medals;
		if (!Recv(sizeof(medals), &medals))
			return false;

		int count;
		if (!Recv(sizeof(count), &count))
			return false;

		CPythonGuild::Instance().SetGuildDonateInfo(medals);
		CPythonGuild::Instance().SetGuilDonateCount(count);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "GuildDonateInfoRefresh", Py_BuildValue("()"));
	}
#endif
	}

	return true;
}

// Guild
//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// Fishing

bool CPythonNetworkStream::SendFishingPacket(int iRotation)
{
	uint8_t byHeader = HEADER_CG_FISHING;
	if (!Send(sizeof(byHeader), &byHeader))
		return false;
	uint8_t byPacketRotation = iRotation / 5;
	if (!Send(sizeof(uint8_t), &byPacketRotation))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendGiveItemPacket(uint32_t dwTargetVID, TItemPos ItemPos, int iItemCount)
{
	TPacketCGGiveItem GiveItemPacket;
	GiveItemPacket.byHeader = HEADER_CG_GIVE_ITEM;
	GiveItemPacket.dwTargetVID = dwTargetVID;
	GiveItemPacket.ItemPos = ItemPos;
	GiveItemPacket.byItemCount = iItemCount;

	if (!Send(sizeof(GiveItemPacket), &GiveItemPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvFishing()
{
	TPacketGCFishing FishingPacket;
	if (!Recv(sizeof(FishingPacket), &FishingPacket))
		return false;

	CInstanceBase* pFishingInstance = nullptr;
	if (FISHING_SUBHEADER_GC_FISH != FishingPacket.subheader)
	{
		pFishingInstance = CPythonCharacterManager::Instance().GetInstancePtr(FishingPacket.info);
		if (!pFishingInstance)
			return true;
	}

	switch (FishingPacket.subheader)
	{
	case FISHING_SUBHEADER_GC_START:
		pFishingInstance->StartFishing(float(FishingPacket.dir) * 5.0f);
		break;
	case FISHING_SUBHEADER_GC_STOP:
	{
		if (pFishingInstance->IsFishing())
			pFishingInstance->StopFishing();

#ifdef ENABLE_FISHING_RENEWAL
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingStop", Py_BuildValue("()"));
#endif
	}
	break;
	case FISHING_SUBHEADER_GC_REACT:
		if (pFishingInstance->IsFishing())
		{
			pFishingInstance->SetFishEmoticon(); // Fish Emoticon
			pFishingInstance->ReactFishing();
		}
		break;
	case FISHING_SUBHEADER_GC_SUCCESS:
		pFishingInstance->CatchSuccess();
		break;
	case FISHING_SUBHEADER_GC_FAIL:
		pFishingInstance->CatchFail();
		if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr())
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingFailure", Py_BuildValue("()"));
		break;
	case FISHING_SUBHEADER_GC_FISH:
	{
		uint32_t dwFishID = FishingPacket.info;

		if (0 == FishingPacket.info)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotifyUnknown", Py_BuildValue("()"));
			return true;
		}

		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(dwFishID, &pItemData))
			return true;

		CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (!pMainInstance)
			return true;

		if (pMainInstance->IsFishing())
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotify",
				Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
		else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingSuccess",
				Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
		break;
	}
	}

	return true;
}
// Fishing
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Dungeon
bool CPythonNetworkStream::RecvDungeon()
{
	TPacketGCDungeon DungeonPacket;
	if (!Recv(sizeof(DungeonPacket), &DungeonPacket))
		return false;

	switch (DungeonPacket.subheader)
	{
	case DUNGEON_SUBHEADER_GC_TIME_ATTACK_START:
	{
		break;
	}
	case DUNGEON_SUBHEADER_GC_DESTINATION_POSITION:
	{
		unsigned long ulx{}, uly{};
		if (!Recv(sizeof(ulx), &ulx))
			return false;
		if (!Recv(sizeof(uly), &uly))
			return false;

		CPythonPlayer::Instance().SetDungeonDestinationPosition(ulx, uly);
		break;
	}
	}

	return true;
}
// Dungeon
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MyShop
bool CPythonNetworkStream::SendBuildPrivateShopPacket(const char* c_szName, const std::vector<TShopItemTable>& c_rSellingItemStock)
{
	TPacketCGMyShop packet{};
	packet.bHeader = HEADER_CG_MYSHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = msl::truncate_cast<uint8_t>(c_rSellingItemStock.size());
	if (!Send(sizeof(packet), &packet))
		return false;

	for (auto& c_rItem : c_rSellingItemStock)
	{
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvShopSignPacket()
{
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;

#ifdef ENABLE_GRAPHIC_ON_OFF
	CInstanceBase* pChrInst = CPythonCharacterManager::Instance().GetInstancePtr(p.dwVID);
	if (!pChrInst)
		return true;

	CActorInstance* pActor = pChrInst->GetGraphicThingInstancePtr();
	if (!pActor)
		return true;
#endif

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

	if (0 == strlen(p.szSign))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_PrivateShop_Disappear",
			Py_BuildValue("(i)", p.dwVID));

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.ClosePrivateShop();
#ifdef ENABLE_GRAPHIC_ON_OFF
		pActor->SetPrivateShopSign(false);
#endif
	}
	else
	{
#ifdef ENABLE_MYSHOP_DECO
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Appear", Py_BuildValue("(isi)", p.dwVID, p.szSign, p.bType));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_PrivateShop_Appear", Py_BuildValue("(is)", p.dwVID, p.szSign));
#endif

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.OpenPrivateShop();
#ifdef ENABLE_GRAPHIC_ON_OFF
		pActor->SetPrivateShopSign(true);
#endif
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
bool CPythonNetworkStream::RecvMyShopSignPacket()
{
	TPacketMyShopSign p;
	if (!Recv(sizeof(TPacketMyShopSign), &p))
		return false;

	CPythonShop& rkShop = CPythonShop::Instance();
	if (strcmp(p.sign, "") == 0)
		rkShop.ClearMyShopInfo();
	else
		rkShop.SetMyShopName(p.sign);

	__RefreshShopInfoWindow();
	return true;
}
#endif

bool CPythonNetworkStream::RecvTimePacket()
{
	TPacketGCTime TimePacket;
	if (!Recv(sizeof(TimePacket), &TimePacket))
		return false;

	IAbstractApplication& rkApp = IAbstractApplication::GetSingleton();
	rkApp.SetServerTime(TimePacket.time);

	return true;
}

bool CPythonNetworkStream::RecvWalkModePacket()
{
	TPacketGCWalkMode WalkModePacket;
	if (!Recv(sizeof(WalkModePacket), &WalkModePacket))
		return false;

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(WalkModePacket.vid);
	if (pInstance)
	{
		if (WALKMODE_RUN == WalkModePacket.mode)
			pInstance->SetRunMode();
		else
			pInstance->SetWalkMode();
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket()
{
	TPacketGCChangeSkillGroup ChangeSkillGroup;
	if (!Recv(sizeof(ChangeSkillGroup), &ChangeSkillGroup))
		return false;

	m_dwMainActorSkillGroup = ChangeSkillGroup.skill_group;

	CPythonPlayer::Instance().NEW_ClearSkillData();
	__RefreshCharacterWindow();
	return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex)
{
	m_dwMainActorSkillGroup = iIndex;

	CPythonPlayer::Instance().NEW_ClearSkillData();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendRefinePacket(uint16_t wPos, uint8_t byType)	//@fixme425
{
	TPacketCGRefine kRefinePacket{};
	kRefinePacket.header = HEADER_CG_REFINE;
	kRefinePacket.pos = wPos;
	kRefinePacket.type = byType;

	if (!Send(sizeof(kRefinePacket), &kRefinePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSelectItemPacket(uint32_t dwItemPos)
{
	TPacketCGScriptSelectItem kScriptSelectItem{};
	kScriptSelectItem.header = HEADER_CG_SCRIPT_SELECT_ITEM;
	kScriptSelectItem.selection = dwItemPos;

	if (!Send(sizeof(kScriptSelectItem), &kScriptSelectItem))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvRefineInformationPacket()
{
	TPacketGCRefineInformation kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable& rkRefineTable = kRefineInfoPacket.refine_table;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenRefineDialog",
		Py_BuildValue("(iiiii)", kRefineInfoPacket.pos, kRefineInfoPacket.refine_table.result_vnum, rkRefineTable.cost,
			rkRefineTable.prob, kRefineInfoPacket.type));

	for (int i = 0; i < rkRefineTable.material_count; ++i)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog",
			Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));

#ifdef _DEBUG
	Tracef(" >> RecvRefineInformationPacketNew(pos=%d, result_vnum=%d, cost=%d, prob=%d, type=%d)\n", kRefineInfoPacket.pos,
		kRefineInfoPacket.refine_table.result_vnum, rkRefineTable.cost, rkRefineTable.prob, kRefineInfoPacket.type);
#endif

	return true;
}

bool CPythonNetworkStream::RecvNPCList()
{
	TPacketGCNPCPosition kNPCPosition;
	if (!Recv(sizeof(kNPCPosition), &kNPCPosition))
		return false;

	assert(int(kNPCPosition.size) - sizeof(kNPCPosition) == kNPCPosition.count * sizeof(TNPCPosition) && "HEADER_GC_NPC_POSITION");

	CPythonMiniMap::Instance().ClearAtlasMarkInfo();

	for (int i = 0; i < kNPCPosition.count; ++i)
	{
		TNPCPosition NPCPosition;
		if (!Recv(sizeof(TNPCPosition), &NPCPosition))
			return false;

		CPythonMiniMap::Instance().RegisterAtlasMark(NPCPosition.bType, NPCPosition.name, NPCPosition.dwMobVnum, NPCPosition.x, NPCPosition.y);
	}

	return true;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
bool CPythonNetworkStream::RecvSHOPList()
{
	TPacketGCShopPosition kShopPosition;
	if (!Recv(sizeof(kShopPosition), &kShopPosition))
		return false;

	assert(int(kShopPosition.size) - sizeof(kShopPosition) == kShopPosition.count * sizeof(TShopPosition) && "HEADER_GC_SHOP_POSITION");

	CPythonMiniMap::Instance().ClearAtlasShopInfo(); // Clear existing shops

	for (int i = 0; i < kShopPosition.count; ++i)
	{
		TShopPosition SHOPPosition;
		if (!Recv(sizeof(TShopPosition), &SHOPPosition))
			return false;

		CPythonMiniMap::Instance().RegisterAtlasMark(CActorInstance::TYPE_SHOP, SHOPPosition.name, SHOPPosition.dwMobVnum, SHOPPosition.x, SHOPPosition.y);
	}

	return true;
}
#endif

bool CPythonNetworkStream::__SendCRCReportPacket() const
{
	return true;
}

bool CPythonNetworkStream::SendClientVersionPacket()
{
	std::string filename;

	GetExcutedFileName(filename);

	filename = CFileNameHelper::NoPath(filename);
	CFileNameHelper::ChangeDosPath(filename);

	if (LocaleService_IsEUROPE() && false == LocaleService_IsYMIR())
	{
		TPacketCGClientVersion2 kVersionPacket{};
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION2;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename) - 1);
		strncpy(kVersionPacket.timestamp, "1215955205", sizeof(kVersionPacket.timestamp) - 1); // # python time.time 앞자리

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	else
	{
		TPacketCGClientVersion kVersionPacket{};
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename) - 1);
		strncpy(kVersionPacket.timestamp, __TIMESTAMP__, sizeof(kVersionPacket.timestamp) - 1);

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	return SendSequence();
}

bool CPythonNetworkStream::RecvAffectAddPacket()
{
	TPacketGCAffectAdd kAffectAdd;
	if (!Recv(sizeof(kAffectAdd), &kAffectAdd))
		return false;

	TPacketAffectElement& rkElement = kAffectAdd.elem;
	if (rkElement.wPointIdxApplyOn == POINT_ENERGY)
	{
		CPythonPlayer::Instance().SetStatus(POINT_ENERGY_END_TIME,
			CPythonApplication::Instance().GetServerTimeStamp() + rkElement.lDuration);
		__RefreshStatus();
	}

#if defined(ENABLE_FLOWER_EVENT) || defined(ENABLE_AUTO_SYSTEM)
	CPythonPlayer::Instance().AddAffect(rkElement.dwType, kAffectAdd.elem);
#endif

#ifdef ENABLE_AUTO_SYSTEM
	if (rkElement.dwType == CInstanceBase::NEW_AFFECT_AUTO_USE)
	{
		//CPythonPlayer::Instance().StartAuto(true);
		CPythonCharacterManager::Instance().SetAutoOnOff(true);
	}
	if (rkElement.dwType == CInstanceBase::NEW_AFFECT_AUTO)
	{
		CPythonCharacterManager::Instance().GetInstancePtrByName(CPythonPlayer::Instance().GetName())->SetAutoAffect(true);
	}
#endif

	PyCallClassMemberFunc(
		m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_AddAffect",
		Py_BuildValue("(iiii)", rkElement.dwType, rkElement.wPointIdxApplyOn, rkElement.lApplyValue, rkElement.lDuration));

	return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket()
{
	TPacketGCAffectRemove kAffectRemove;
	if (!Recv(sizeof(kAffectRemove), &kAffectRemove))
		return false;

#ifdef ENABLE_FLOWER_EVENT
	CPythonPlayer::Instance().RemoveAffect(kAffectRemove.dwType, kAffectRemove.wApplyOn);
#endif

#ifdef ENABLE_AUTO_SYSTEM
	if (kAffectRemove.dwType == CInstanceBase::NEW_AFFECT_AUTO_USE)
	{
		//CPythonPlayer::Instance().StartAuto(false);
		CPythonCharacterManager::Instance().SetAutoOnOff(false);
	}
	if (kAffectRemove.dwType == CInstanceBase::NEW_AFFECT_AUTO)
	{
		CPythonCharacterManager::Instance().GetInstancePtrByName(CPythonPlayer::Instance().GetName())->SetAutoAffect(false);
	}
#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_RemoveAffect",
		Py_BuildValue("(ii)", kAffectRemove.dwType, kAffectRemove.wApplyOn));

	return true;
}

bool CPythonNetworkStream::RecvChannelPacket()
{
	TPacketGCChannel kChannelPacket;
	if (!Recv(sizeof(kChannelPacket), &kChannelPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvViewEquipPacket()
{
	TPacketGCViewEquip kViewEquipPacket;
	if (!Recv(sizeof(kViewEquipPacket), &kViewEquipPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenEquipmentDialog", Py_BuildValue("(i)", kViewEquipPacket.dwVID));

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		TEquipmentItemSet& rItemSet = kViewEquipPacket.equips[i];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count, rItemSet.dwTransmutationVnum));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count));
#endif

		for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogSocket",
				Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, j, rItemSet.alSockets[j]));
		}

		for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr",
				Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k, rItemSet.aAttr[k].wType, rItemSet.aAttr[k].sValue));
		}

#ifdef ENABLE_YOHARA_SYSTEM
		for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; ++s)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogApplyRandom",
				Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, s, rItemSet.aApplyRandom[s].wType, rItemSet.aApplyRandom[s].sValue));

		}

		for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; ++r)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogRandomValue",
				Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, r, rItemSet.alRandomValues[r]));

		}
#endif
	}

	return true;
}

bool CPythonNetworkStream::RecvLandPacket()
{
	TPacketGCLandList kLandList;
	if (!Recv(sizeof(kLandList), &kLandList))
		return false;

	std::vector<uint32_t> kVec_dwGuildID;

	CPythonMiniMap& rkMiniMap = CPythonMiniMap::Instance();
	CPythonBackground& rkBG = CPythonBackground::Instance();
	CInstanceBase* pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();

	rkMiniMap.ClearGuildArea();
	rkBG.ClearGuildArea();

	int iPacketSize = (kLandList.size - sizeof(TPacketGCLandList));
	for (; iPacketSize > 0; iPacketSize -= sizeof(TLandPacketElement))
	{
		TLandPacketElement kElement;
		if (!Recv(sizeof(TLandPacketElement), &kElement))
			return false;

		rkMiniMap.RegisterGuildArea(kElement.dwID, kElement.dwGuildID, kElement.x, kElement.y, kElement.width, kElement.height);

		if (pMainInstance)
			if (kElement.dwGuildID == pMainInstance->GetGuildID())
			{
				rkBG.RegisterGuildArea(kElement.x, kElement.y, kElement.x + kElement.width, kElement.y + kElement.height);
			}

		if (0 != kElement.dwGuildID)
			kVec_dwGuildID.emplace_back(kElement.dwGuildID);
	}
	// @fixme006
	if (!kVec_dwGuildID.empty())
		__DownloadSymbol(kVec_dwGuildID);

	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket()
{
	TPacketGCTargetCreate kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
	CPythonBackground& rkpyBG = CPythonBackground::Instance();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	CPythonLocale::Instance().FormatString(kTargetCreate.szTargetName, sizeof(kTargetCreate.szTargetName));
#endif

	if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.byType)
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);
	}
	else
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
		rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket()
{
	TPacketGCTargetUpdate kTargetUpdate;
	if (!Recv(sizeof(kTargetUpdate), &kTargetUpdate))
		return false;

	CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	CPythonBackground& rkpyBG = CPythonBackground::Instance();
	rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket()
{
	TPacketGCTargetDelete kTargetDelete;
	if (!Recv(sizeof(kTargetDelete), &kTargetDelete))
		return false;

	CPythonMiniMap& rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

	CPythonBackground& rkpyBG = CPythonBackground::Instance();
	rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

	return true;
}

bool CPythonNetworkStream::RecvLoverInfoPacket()
{
	TPacketGCLoverInfo kLoverInfo;
	if (!Recv(sizeof(kLoverInfo), &kLoverInfo))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo",
		Py_BuildValue("(si)", kLoverInfo.szName, kLoverInfo.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVER INFO : %s, %d\n", kLoverInfo.szName, kLoverInfo.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket()
{
	TPacketGCLovePointUpdate kLovePointUpdate;
	if (!Recv(sizeof(kLovePointUpdate), &kLovePointUpdate))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateLovePoint", Py_BuildValue("(i)", kLovePointUpdate.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVE POINT UPDATE : %d\n", kLovePointUpdate.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket()
{
	TPacketGCDigMotion kDigMotion;
	if (!Recv(sizeof(kDigMotion), &kDigMotion))
		return false;

#ifdef _DEBUG
	Tracef(" Dig Motion [%d/%d]\n", kDigMotion.vid, kDigMotion.count);
#endif

	IAbstractCharacterManager& rkChrMgr = IAbstractCharacterManager::GetSingleton();
	CInstanceBase* pkInstMain = rkChrMgr.GetInstancePtr(kDigMotion.vid);
	CInstanceBase* pkInstTarget = rkChrMgr.GetInstancePtr(kDigMotion.target_vid);
	if (nullptr == pkInstMain)
		return true;

	if (pkInstTarget)
		pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

	for (int i = 0; i < kDigMotion.count; ++i)
		pkInstMain->PushOnceMotion(CRaceMotionData::NAME_DIG);

	return true;
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
bool CPythonNetworkStream::RecvGrowthPetPacket()
{
	TPacketGCGrowthPet kGrowthPetPacket;
	if (!Recv(sizeof(TPacketGCGrowthPet), &kGrowthPetPacket))
	{
		Tracef("GrowthPetPacket Error SubHeader %u\n", kGrowthPetPacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kGrowthPetPacket.wSize) - sizeof(TPacketGCGrowthPet);

	switch (kGrowthPetPacket.bSubHeader)
	{
	case GROWTHPET_SUBHEADER_GC_WINDOW:
	{
		if (iPacketSize > 0)
		{
			TSubPacketCGGrowthPetWindowType kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketCGGrowthPetWindowType) == 0 && "GROWTHPET_SUBHEADER_GC_WINDOW");
			if (!Recv(sizeof(TSubPacketCGGrowthPetWindowType), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetPetWindowType", Py_BuildValue("(i)", kSubPacket.bPetWindowType));
		}
		else
			TraceError(" RecvGrowthPetPacket Error 0x04100%u0F", kGrowthPetPacket.bSubHeader);

		break;
	}
	}

	return true;
}

// Send ----------------------------------------
bool CPythonNetworkStream::SendPetWindowType(uint8_t pet_window_type)
{
	TPacketCGGrowthPet kPacket;
	kPacket.wSize = sizeof(TPacketCGGrowthPet) + sizeof(TSubPacketCGGrowthPetWindowType);
	kPacket.bSubHeader = GROWTHPET_SUBHEADER_CG_WINDOW_TYPE;

	TSubPacketCGGrowthPetWindowType kSubPacket;
	kSubPacket.bPetWindowType = pet_window_type;

	if (!Send(sizeof(TPacketCGGrowthPet), &kPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGGrowthPetWindowType), &kSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetHatchingWindowPacket(bool bisOpenPetHatchingWindow)
{
	TPacketCGGrowthPet kPacket;
	kPacket.wSize = sizeof(TPacketCGGrowthPet) + sizeof(TSubPacketCGGrowthPetHatchWindow);
	kPacket.bSubHeader = GROWTHPET_SUBHEADER_CG_HATCHING_WINDOW;

	TSubPacketCGGrowthPetHatchWindow kSubPacket;
	kSubPacket.isOpen = bisOpenPetHatchingWindow;

	if (!Send(sizeof(TPacketCGGrowthPet), &kPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGGrowthPetHatchWindow), &kSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetHatchingPacket(char* sGrowthPetName, uint8_t bPos)
{
	TPacketCGGrowthPetHatching pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_HATCHING;
	pPacket.bPos = bPos;

	strncpy(pPacket.sGrowthPetName, sGrowthPetName, CItemData::PET_NAME_MAX_SIZE);
	pPacket.sGrowthPetName[CItemData::PET_NAME_MAX_SIZE] = '\0';

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetLearnSkillPacket(uint8_t bSkillBookSlotIndex, uint8_t bSkillBookInvenIndex)
{
	TPacketCGGrowthPetLearnSkill pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_LEARN_SKILL;

	pPacket.bSkillBookSlotIndex = bSkillBookSlotIndex;
	pPacket.bSkillBookInvenIndex = bSkillBookInvenIndex;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetSkillUpgradePacket(uint8_t bslotPos, uint8_t bslotIndex)
{
	TPacketCGGrowthPetSkillUpgrade pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_SKILL_UPGRADE;

	pPacket.bslotPos = bslotPos;
	pPacket.bslotIndex = bslotIndex;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetSkillUpgradeRequestPacket(uint8_t bslotPos, uint8_t bslotIndex)
{
	TPacketCGGrowthPetSkillUpgradeRequest pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_SKILL_UPGRADE_REQUEST;

	pPacket.bslotPos = bslotPos;
	pPacket.bslotIndex = bslotIndex;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetFeedPacket(uint8_t feedIndex, PyObject* poFeedItemsList, PyObject* poFeedItemsCount)
{
	__int16 iFeedItemsCubePos[9];
	for (signed int i = 0; i < 9; ++i)
		iFeedItemsCubePos[i] = -1;

	TPacketCGGrowthPetFeedRequest pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_FEED_REQUEST;
	pPacket.bIndex = feedIndex;

	int iPetFeedTotalItems = PyList_Size(poFeedItemsList);
	if (iPetFeedTotalItems <= 0)
		return false;

	pPacket.iFeedItemsCount = iPetFeedTotalItems;

	for (int j = 0; j < iPetFeedTotalItems; ++j)
	{
		// Item Inventory Slot Pos
		PyObject* poFeedItemSlotPos = PyList_GetItem(poFeedItemsList, j);
		int iItemSlotPos = PyInt_AsLong(poFeedItemSlotPos);
		if (iItemSlotPos == -1)
			return false;

		// Item Count
		PyObject* poFeedItemCount = PyList_GetItem(poFeedItemsCount, j);
		int iFeedItemCount = PyInt_AsLong(poFeedItemCount);
		if (!iFeedItemCount)
			return false;

		iFeedItemsCubePos[j] = iItemSlotPos;

		// Set to Packet
		pPacket.iFeedItemsCubeSlot[j] = iItemSlotPos;
	}

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetDeleteSkill(uint8_t bSkillBookDelSlotIndex, uint8_t bSkillBookDelInvenIndex)
{
	TPacketCGGrowthPetDeleteSkill pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_DELETE_SKILL;
	pPacket.bSkillBookDelSlotIndex = bSkillBookDelSlotIndex;
	pPacket.bSkillBookDelInvenIndex = bSkillBookDelInvenIndex;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetDeleteAllSkill(uint8_t bPetSkillAllDelBookIndex)
{
	TPacketCGGrowthPetDeleteAllSkill pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_ALL_DEL_SKILL;
	pPacket.bPetSkillAllDelBookIndex = bPetSkillAllDelBookIndex;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendPetNameChange(char* sPetName, uint8_t bItemWindowType, uint16_t bItemWindowCell, uint8_t bPetWindowType, uint16_t bPetWindowCell)
{
	TPacketCGGrowthPetNameChange pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_NAME_CHANGE;

	strncpy(pPacket.sPetName, sPetName, CItemData::PET_NAME_MAX_SIZE);
	pPacket.sPetName[CItemData::PET_NAME_MAX_SIZE] = '\0';

	pPacket.bItemWindowType = bItemWindowType;
	pPacket.bItemWindowCell = bItemWindowCell;
	pPacket.bPetWindowType = bPetWindowType;
	pPacket.bPetWindowCell = bPetWindowCell;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

#ifdef ENABLE_PET_ATTR_DETERMINE
bool CPythonNetworkStream::SendPetAttrDetermine()
{
	TPacketCGGrowthPetAttrDetermine pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_ATTR_DETERMINE;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendChangePetPacket(uint8_t bSkillBookDebPetSlotIndexlSlotIndex, uint8_t bMaterialSlotIndex)
{
	TPacketCGGrowthPetAttrChange pPacket;

	pPacket.bHeader = HEADER_CG_GROWTH_PET_ATTR_CHANGE;
	pPacket.bPetSlotIndex = bSkillBookDebPetSlotIndexlSlotIndex;
	pPacket.bMaterialSlotIndex = bMaterialSlotIndex;

	if (!Send(sizeof(pPacket), &pPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
bool CPythonNetworkStream::SendPetRevive(TItemPos upBringingPos, const std::vector<std::pair<uint16_t, uint16_t>>& itemVec)
{
	TPacketCGGrowthPetReviveRequest packet;
	packet.bHeader = HEADER_CG_GROWTH_PET_REVIVE_REQUEST;
	packet.upBringingPos = upBringingPos;
	memset(packet.pos, 0, sizeof(packet.pos));
	memset(packet.count, 0, sizeof(packet.count));

	for (int i = 0; i < itemVec.size(); i++)
	{
		packet.pos[i] = itemVec.at(i).first;
		packet.count[i] = itemVec.at(i).second;
	}

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::RecvGrowthPetInfoPacket()
{
	TPacketGCGrowthPetInfo packet;
	if (!Recv(sizeof(TPacketGCGrowthPetInfo), &packet))
	{
		Tracef("GrowthPetInfo Packet Error\n");
		return false;
	}

	CPythonPlayer::Instance().SetActivePetItemID(0);
	CPythonPlayer::Instance().SetActivePetItemVnum(0);

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TGrowthPetInfo kGrowthPetInfo;
		if (!Recv(sizeof(kGrowthPetInfo), &kGrowthPetInfo))
			return false;

		TGrowthPetInfo petInfo = {};
		petInfo.pet_id = kGrowthPetInfo.pet_id;
		petInfo.pet_vnum = kGrowthPetInfo.pet_vnum;
		petInfo.pet_level = kGrowthPetInfo.pet_level;
		petInfo.evol_level = kGrowthPetInfo.evol_level;
		petInfo.pet_birthday = kGrowthPetInfo.pet_birthday;
		strncpy(petInfo.pet_nick, kGrowthPetInfo.pet_nick, sizeof(petInfo.pet_nick));
		petInfo.pet_hp = kGrowthPetInfo.pet_hp;
		petInfo.pet_def = kGrowthPetInfo.pet_def;
		petInfo.pet_sp = kGrowthPetInfo.pet_sp;
		petInfo.exp_monster = kGrowthPetInfo.exp_monster;
		petInfo.next_exp_monster = kGrowthPetInfo.next_exp_monster;
		petInfo.exp_item = kGrowthPetInfo.exp_item;
		petInfo.next_exp_item = kGrowthPetInfo.next_exp_item;

		petInfo.pet_end_time = kGrowthPetInfo.pet_end_time;
		petInfo.pet_max_time = kGrowthPetInfo.pet_max_time;

		petInfo.skill_count = kGrowthPetInfo.skill_count;

		for (int i = 0; i < 3; i++)
		{
			petInfo.skill_vnum[i] = kGrowthPetInfo.skill_vnum[i];
			petInfo.skill_level[i] = kGrowthPetInfo.skill_level[i];
			petInfo.skill_spec[i] = kGrowthPetInfo.skill_spec[i];
			petInfo.skill_cool[i] = kGrowthPetInfo.skill_cool[i];

			petInfo.next_skill_formula1[i] = kGrowthPetInfo.skill_formula1[i];
			petInfo.next_skill_formula1[i] = kGrowthPetInfo.next_skill_formula1[i];

			petInfo.skill_formula2[i] = kGrowthPetInfo.skill_formula2[i];
			petInfo.next_skill_formula2[i] = kGrowthPetInfo.next_skill_formula2[i];
		}

		if (kGrowthPetInfo.is_summoned)
		{
			CPythonPlayer::Instance().SetActivePetItemID(kGrowthPetInfo.pet_id);
			CPythonPlayer::Instance().SetActivePetItemVnum(kGrowthPetInfo.pet_vnum);

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetFlashEvent", Py_BuildValue("(i)",
				kGrowthPetInfo.flash_event));

			const uint8_t evolInfo_index = kGrowthPetInfo.evol_level - 1;
			uint8_t evolInfo_value = 0;
			switch (evolInfo_index)
			{
			case 0:
				evolInfo_value = 40; // level
				break;
			case 1:
				evolInfo_value = 80; // level
				break;
			case 2:
				evolInfo_value = 30; // days for evolve
				break;
			default:
				break;
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetEvolInfo", Py_BuildValue("(ii)", evolInfo_index, evolInfo_value));
		}

		CPythonPlayer::Instance().SetPetInfo(petInfo);

		packet.wSize -= sizeof(kGrowthPetInfo);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory", Py_BuildValue("()"));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvGrowthPetHatching()
{
	TPacketGCGrowthPetHatching sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetHatchingWindowCommand", Py_BuildValue("(iii)", sPacket.bResult, INVENTORY, sPacket.bPos));

	return true;
}

bool CPythonNetworkStream::RecvGrowthPetSkillUpgradeRequest()
{
	TPacketGCGrowthPetSkillUpgradeRequest sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetSkillUpgradeDlgOpen", Py_BuildValue("(iii)", sPacket.bSkillSlot, sPacket.bSkillIndex, sPacket.iPrice));

	return true;
}

bool CPythonNetworkStream::RecvGrowthPetFeedItemResult()
{
	TPacketGCGrowthPetFeedResult sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetFeedReuslt", Py_BuildValue("(i)", sPacket.bResult));

	return true;
}

bool CPythonNetworkStream::RecvGrowthPetNameChangeResult()
{
	TPacketGCGrowthPetNameChangeResult sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetNameChangeWindowCommand", Py_BuildValue("(iiiii)", sPacket.bResult, 0, 0, 0, 0));

	return true;
}

#ifdef ENABLE_PET_ATTR_DETERMINE
bool CPythonNetworkStream::RecvGrowthPetAttrDetermineResult()
{
	TPacketGCGrowthPetAttrDetermineResult sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetAttrDetermineResult", Py_BuildValue("(i)", sPacket.bResult));

	return true;
}

bool CPythonNetworkStream::RecvGrowthPetAttrChangeResult()
{
	TPacketGCGrowthPetAttrChangeResult sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetAttrChangeResult", Py_BuildValue("(i)", sPacket.bResult));

	return true;
}
#endif

#ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
bool CPythonNetworkStream::RecvGrowthPetReviveItemResult()
{
	TPacketGCGrowthPetReviveResult sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PetReviveResult", Py_BuildValue("(i)", sPacket.bResult));

	return true;
}
#endif
#endif

#ifdef ENABLE_INGAME_WIKI
extern PyObject* wikiModule;

bool CPythonNetworkStream::SendWikiRequestInfo(uint64_t retID, uint32_t vnum, uint8_t isMob)
{
	TCGWikiPacket pack;
	pack.bHeader = HEADER_CG_WIKI;
	pack.bSubHeader = isMob;
	pack.vnum = vnum;
	pack.ret_id = retID;

	if (!Send(sizeof(TCGWikiPacket), &pack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvWikiPacket()
{
	TGCWikiPacket pack;
	if (!Recv(sizeof(TGCWikiPacket), &pack))
		return false;

	uint16_t iPacketSize = pack.wSize - sizeof(TGCWikiPacket);
	if (iPacketSize <= 0)
		return false;

	uint64_t ret_id = 0;
	uint32_t data_vnum = 0;

	switch (pack.bSubHeader)
	{
	case LOAD_WIKI_ITEM:
	{
		const size_t recv_size = sizeof(TGCItemWikiPacket);
		iPacketSize -= uint16_t(recv_size);

		TGCItemWikiPacket item_data;
		if (!Recv(recv_size, &item_data))
			return false;

		ret_id = item_data.ret_id;
		data_vnum = item_data.vnum;

		CItemData* pData = nullptr;
		if (!CItemManager::Instance().GetItemDataPointer(item_data.vnum, &pData))
		{
			TraceError("Cant get pointer from item -> %d", item_data.vnum);
			return false;
		}

		auto& recv_wiki = item_data.wiki_info;
		auto wikiInfo = pData->GetWikiTable();

		const int origin_size = item_data.origin_infos_count;
		const int chest_info_count = recv_wiki.chest_info_count;
		const int refine_infos_count = recv_wiki.refine_infos_count;

		wikiInfo->isSet = true;
		wikiInfo->hasData = true;
		wikiInfo->bIsCommon = recv_wiki.is_common;
		wikiInfo->dwOrigin = recv_wiki.origin_vnum;
		wikiInfo->maxRefineLevel = refine_infos_count;

		{
			// Wiki Data
			wikiInfo->pOriginInfo.clear();
			const size_t origin_info_recv_base_size = sizeof(NWikiData::TWikiItemOriginInfo);
			for (int idx = 0; idx < origin_size; ++idx)
			{
				NWikiData::TWikiItemOriginInfo origin_data;
				if (!Recv(origin_info_recv_base_size, &origin_data))
					return false;

				wikiInfo->pOriginInfo.emplace_back(origin_data);
				iPacketSize -= uint16_t(origin_info_recv_base_size);
			}

			// Wiki Chest
			wikiInfo->pChestInfo.clear();
			const size_t chest_info_recv_base_size = sizeof(NWikiData::TWikiChestInfo);
			for (int idx = 0; idx < chest_info_count; ++idx)
			{
				NWikiData::TWikiChestInfo chest_data;
				if (!Recv(chest_info_recv_base_size, &chest_data))
					return false;

				wikiInfo->pChestInfo.emplace_back(chest_data);
				iPacketSize -= uint16_t(chest_info_recv_base_size);
			}

			// Wiki Refine
			wikiInfo->pRefineData.clear();
			const size_t refine_info_recv_base_size = sizeof(NWikiData::TWikiRefineInfo);
			for (int idx = 0; idx < refine_infos_count; ++idx)
			{
				NWikiData::TWikiRefineInfo refine_info_data;
				if (!Recv(refine_info_recv_base_size, &refine_info_data))
					return false;

				wikiInfo->pRefineData.emplace_back(refine_info_data);
				iPacketSize -= uint16_t(refine_info_recv_base_size);
			}
		}

		if (iPacketSize != 0)
			return false;
	}
	break;

	case LOAD_WIKI_MOB:
	{
		const size_t recv_size = sizeof(TGCMobWikiPacket);
		iPacketSize -= uint16_t(recv_size);

		TGCMobWikiPacket mob_data;
		if (!Recv(recv_size, &mob_data))
			return false;

		ret_id = mob_data.ret_id;
		data_vnum = mob_data.vnum;
		const int drop_info_count = mob_data.drop_info_count;

		CPythonNonPlayer::TWikiInfoTable* mobData = nullptr;
		if (!(mobData = CPythonNonPlayer::Instance().GetWikiTable(mob_data.vnum)))
		{
			TraceError("Cant get mob data from monster -> %d", mob_data.vnum);
			return false;
		}

		mobData->isSet = (drop_info_count > 0);

		{
			mobData->dropList.clear();
			const size_t mob_drop_info_recv_base_size = sizeof(NWikiData::TWikiMobDropInfo);

			for (int idx = 0; idx < drop_info_count; ++idx) {
				NWikiData::TWikiMobDropInfo drop_data;
				if (!Recv(mob_drop_info_recv_base_size, &drop_data))
					return false;

				mobData->dropList.push_back(drop_data);
				iPacketSize -= uint16_t(mob_drop_info_recv_base_size);
			}
		}

		if (iPacketSize != 0)
			return false;
	}
	break;

	default:
		break;
	}

	if (wikiModule)
		PyCallClassMemberFunc(wikiModule, "BINARY_LoadInfo", Py_BuildValue("(Li)", (long long)ret_id, data_vnum));

	return true;
}
#endif

// 용혼석 강화
bool CPythonNetworkStream::SendDragonSoulRefinePacket(uint8_t bRefineType, const TItemPos* pos)
{
	TPacketCGDragonSoulRefine pk;
	pk.header = HEADER_CG_DRAGON_SOUL_REFINE;
	pk.bSubType = bRefineType;
	memcpy(pk.ItemGrid, pos, sizeof(TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
	if (!Send(sizeof(pk), &pk))
		return false;
	return true;
}

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
bool CPythonNetworkStream::SendExtendInvenRequest(uint8_t bStepIndex, uint8_t bWindow)
# else
bool CPythonNetworkStream::SendExtendInvenRequest(uint8_t bStepIndex)
# endif
{
	TPacketCGSendExtendInvenRequest pPacket{};
	pPacket.bHeader = HEADER_CG_EXTEND_INVEN_REQUEST;
	pPacket.bStepIndex = bStepIndex;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	pPacket.bWindow = bWindow;
	pPacket.bSpecialState = bWindow == 10 ? false : true;
# endif

	if (Send(sizeof(pPacket), &pPacket))
		return SendSequence();

	Tracef("SendExtendInvenRequest Error");
	return false;
}

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
bool CPythonNetworkStream::SendExtendInvenUpgrade(uint8_t bWindow)
# else
bool CPythonNetworkStream::SendExtendInvenUpgrade()
# endif
{
	TPacketCGSendExtendInvenUpgrade pPacket{};
	pPacket.bHeader = HEADER_CG_EXTEND_INVEN_UPGRADE;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	pPacket.bWindow = bWindow;
	pPacket.bSpecialState = bWindow == 10 ? false : true;
# endif

	if (Send(sizeof(pPacket), &pPacket))
		return SendSequence();

	Tracef("SendExtendInvenUpgrade Error");
	return false;
}

bool CPythonNetworkStream::RecvExtendInvenInfo()
{
	TPacketGCExtendInvenInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	CPythonPlayer::Instance().SetExtendInvenStage(packet.bExtendStage);
	CPythonPlayer::Instance().SetExtendInvenMax(packet.bExtendMax);
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	for (int i = 0; i < 3; ++i)
	{
		CPythonPlayer::Instance().SetExtendSpecialInvenStage(packet.bExtendSpecialStage[i], i);
		CPythonPlayer::Instance().SetExtendSpecialInvenMax(packet.bExtendSpecialMax[i], i);
	}
# endif

	return true;
}

bool CPythonNetworkStream::RecvExtendInvenResult()
{
	TPacketGCExtendInvenResult pPacket;
	if (!Recv(sizeof(pPacket), &pPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExInvenItemUseMsg", Py_BuildValue("(iii)", pPacket.dwVnum, pPacket.bMsgResult, pPacket.bEnoughCount));

	return true;
}
#endif

#ifdef NEW_SELECT_CHARACTER
bool CPythonNetworkStream::RecvCharacterInformation()
{
	TPacketCharacterInformation packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	memcpy(m_akSimplePlayerInfo, packet.players, sizeof(packet.players));
	return true;
}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
bool CPythonNetworkStream::SendItemCombinationPacket(uint8_t bSlotMedium, uint8_t bSlotBase, uint8_t bSlotMaterial)
{
	TPacketCGSendCostumeRefine pSendItemCombinationPacket{};
	pSendItemCombinationPacket.bHeader = HEADER_CG_COSTUME_REFINE_REQUEST;
	pSendItemCombinationPacket.bSlotMedium = bSlotMedium;
	pSendItemCombinationPacket.bSlotBase = bSlotBase;
	pSendItemCombinationPacket.bSlotMaterial = bSlotMaterial;

	if (Send(sizeof(pSendItemCombinationPacket), &pSendItemCombinationPacket))
		return SendSequence();

	Tracef("SendItemCombinationPacket Error");
	return false;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
bool CPythonNetworkStream::RecvAuraPacket()
{
	TPacketGCAura kAuraPacket;
	if (!Recv(sizeof(TPacketGCAura), &kAuraPacket))
	{
		Tracef("Aura Packet Error SubHeader %u\n", kAuraPacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kAuraPacket.wSize) - sizeof(TPacketGCAura);

	switch (kAuraPacket.bSubHeader)
	{
	case AURA_SUBHEADER_GC_OPEN:
	case AURA_SUBHEADER_GC_CLOSE:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCAuraOpenClose kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCAuraOpenClose) == 0 && "AURA_SUBHEADER_GC_OPENCLOSE");
			if (!Recv(sizeof(TSubPacketGCAuraOpenClose), &kSubPacket))
				return false;

			if (kAuraPacket.bSubHeader == AURA_SUBHEADER_GC_OPEN)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AuraWindowOpen", Py_BuildValue("(i)", kSubPacket.bAuraWindowType));

			else if (kAuraPacket.bSubHeader == AURA_SUBHEADER_GC_CLOSE)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AuraWindowClose", Py_BuildValue("()"));
				CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_MAIN);
				CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_SUB);
				CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_RESULT);
			}

			CPythonPlayer::Instance().SetAuraRefineWindowOpen(kSubPacket.bAuraWindowType);
		}
		else
			TraceError(" RecvAuraPacket Error 0x04100%u0F", kAuraPacket.bSubHeader);

		break;
	}
	case AURA_SUBHEADER_GC_SET_ITEM:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCAuraSetItem kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCAuraSetItem) == 0 && "AURA_SUBHEADER_GC_SET_ITEM");
			if (!Recv(sizeof(TSubPacketGCAuraSetItem), &kSubPacket))
				return false;

			TItemData kItemData;
			kItemData.vnum = kSubPacket.pItem.vnum;
			kItemData.count = kSubPacket.pItem.count;
			for (int iSocket = 0; iSocket < ITEM_SOCKET_SLOT_MAX_NUM; ++iSocket)
				kItemData.alSockets[iSocket] = kSubPacket.pItem.alSockets[iSocket];
			for (int iAttr = 0; iAttr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iAttr)
				kItemData.aAttr[iAttr] = kSubPacket.pItem.aAttr[iAttr];
#ifdef ENABLE_YOHARA_SYSTEM
			for (int iRandom = 0; iRandom < APPLY_RANDOM_SLOT_MAX_NUM; ++iRandom)
				kItemData.aApplyRandom[iRandom] = kSubPacket.pItem.aApplyRandom[iRandom];
			for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
				kItemData.alRandomValues[rValue] = kSubPacket.pItem.alRandomValues[rValue];
#endif

			if (kSubPacket.Cell.IsValidCell() && !kSubPacket.Cell.IsEquipCell())
				CPythonPlayer::Instance().SetActivatedAuraSlot(uint8_t(kSubPacket.AuraCell.cell), kSubPacket.Cell);

			CPythonPlayer::Instance().SetAuraItemData(uint8_t(kSubPacket.AuraCell.cell), kItemData);
		}
		else
			TraceError(" RecvAuraPacket Error 0x040%uBABE", kAuraPacket.bSubHeader);

		break;
	}
	case AURA_SUBHEADER_GC_CLEAR_SLOT:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCAuraClearSlot kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCAuraClearSlot) == 0 && "AURA_SUBHEADER_GC_CLEAR_SLOT");
			if (!Recv(sizeof(TSubPacketGCAuraClearSlot), &kSubPacket))
				return false;

			CPythonPlayer::Instance().DelAuraItemData(kSubPacket.bAuraSlotPos);
		}
		else
			TraceError(" RecvAuraPacket Error 0x04FF0%uAA", kAuraPacket.bSubHeader);

		break;
	}
	case AURA_SUBHEADER_GC_CLEAR_ALL:
	{
		CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_MAIN);
		CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_SUB);
		CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_RESULT);
		break;
	}
	case AURA_SUBHEADER_GC_CLEAR_RIGHT:
	{
		if (iPacketSize == 0)
			CPythonPlayer::Instance().DelAuraItemData(AURA_SLOT_SUB);
		else
			TraceError("invalid packet size %d", iPacketSize);
		break;
	}
	case AURA_SUBHEADER_GC_REFINE_INFO:
	{
		if (iPacketSize > 0)
		{
			for (size_t i = 0; iPacketSize > 0; ++i)
			{
				assert(iPacketSize % sizeof(TSubPacketGCAuraRefineInfo) == 0 && "AURA_SUBHEADER_GC_REFINE_INFO");
				TSubPacketGCAuraRefineInfo kSubPacket;
				if (!Recv(sizeof(TSubPacketGCAuraRefineInfo), &kSubPacket))
					return false;

				CPythonPlayer::Instance().SetAuraRefineInfo(kSubPacket.bAuraRefineInfoType, kSubPacket.bAuraRefineInfoLevel, kSubPacket.bAuraRefineInfoExpPercent);
				iPacketSize -= sizeof(TSubPacketGCAuraRefineInfo);
			}

		}
		else
			TraceError(" RecvAuraPacket Error 0x04000%FF", kAuraPacket.bSubHeader);

		break;
	}
	}

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::SendAuraRefineCheckIn(TItemPos InventoryCell, TItemPos AuraCell, uint8_t byAuraRefineWindowType)
{
	__PlayInventoryItemDropSound(InventoryCell);

	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura) + sizeof(TSubPacketCGAuraRefineCheckIn);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_CHECKIN;

	TSubPacketCGAuraRefineCheckIn kAuraSubPacket;
	kAuraSubPacket.ItemCell = InventoryCell;
	kAuraSubPacket.AuraCell = AuraCell;
	kAuraSubPacket.byAuraRefineWindowType = byAuraRefineWindowType;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGAuraRefineCheckIn), &kAuraSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefineCheckOut(TItemPos AuraCell, uint8_t byAuraRefineWindowType)
{
	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura) + sizeof(TSubPacketCGAuraRefineCheckOut);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_CHECKOUT;

	TSubPacketCGAuraRefineCheckOut kAuraSubPacket;
	kAuraSubPacket.AuraCell = AuraCell;
	kAuraSubPacket.byAuraRefineWindowType = byAuraRefineWindowType;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGAuraRefineCheckOut), &kAuraSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefineAccept(uint8_t byAuraRefineWindowType)
{
	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura) + sizeof(TSubPacketCGAuraRefineAccept);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_ACCEPT;

	TSubPacketCGAuraRefineAccept kAuraSubPacket;
	kAuraSubPacket.byAuraRefineWindowType = byAuraRefineWindowType;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGAuraRefineAccept), &kAuraSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefineCancel()
{
	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_CANCEL;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_SWITCHBOT
bool CPythonNetworkStream::RecvSwitchbotPacket()
{
	TPacketGCSwitchbot pack;
	if (!Recv(sizeof(pack), &pack))
	{
		return false;
	}

	size_t packet_size = int(pack.size) - sizeof(TPacketGCSwitchbot);

	switch (pack.subheader)
	{
	case SUBHEADER_GC_SWITCHBOT_UPDATE:
	{
		if (packet_size != sizeof(CPythonSwitchbot::TSwitchbotTable))
		{
			return false;
		}

		CPythonSwitchbot::TSwitchbotTable table;
		if (!Recv(sizeof(table), &table))
		{
			return false;
		}

		CPythonSwitchbot::Instance().Update(table);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
	}
	break;

	case SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM:
	{
		if (packet_size != sizeof(TSwitchbotUpdateItem))
		{
			return false;
		}

		TSwitchbotUpdateItem update;
		if (!Recv(sizeof(update), &update))
		{
			return false;
		}

		TItemPos pos(SWITCHBOT, update.slot);

		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
		rkPlayer.SetItemCount(pos, update.count);

		for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		{
			rkPlayer.SetItemMetinSocket(pos, i, update.alSockets[i]);
		}

		for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		{
			rkPlayer.SetItemAttribute(pos, j, update.aAttr[j].wType, update.aAttr[j].sValue);
		}

#ifdef ENABLE_YOHARA_SYSTEM
		for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; ++s)
		{
			rkPlayer.SetSungmaAttribute(pos, s, update.aApplyRandom[s].wType, update.aApplyRandom[s].sValue);
		}

		for (int r = 0; r < ITEM_RANDOM_VALUES_MAX_NUM; ++r)
		{
			rkPlayer.SetRandomDefaultAttrs(pos, r, update.alRandomValues[r]);
		}
#endif

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotItem", Py_BuildValue("(i)", update.slot));
		return true;
	}
	break;

	case SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION:
	{
		CPythonSwitchbot::Instance().ClearAttributeMap();

		size_t table_size = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);
		while (packet_size >= table_size)
		{
			const int test = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);

			CPythonSwitchbot::TSwitchbottAttributeTable table;
			if (!Recv(table_size, &table))
			{
				return false;
			}

			CPythonSwitchbot::Instance().AddAttributeToMap(table);
			packet_size -= table_size;
		}
	}
	break;

	default:
		break;
	}

	return true;
}

bool CPythonNetworkStream::SendSwitchbotStartPacket(uint8_t slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives)
{
	TPacketCGSwitchbot pack{};
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_START;
	pack.size = sizeof(TPacketCGSwitchbot) + sizeof(CPythonSwitchbot::TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
	{
		return false;
	}

	for (const auto& it : alternatives)
	{
		if (!Send(sizeof(it), &it))
		{
			return false;
		}
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSwitchbotStopPacket(uint8_t slot)
{
	TPacketCGSwitchbot pack{};
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_STOP;
	pack.size = sizeof(TPacketCGSwitchbot);
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
	{
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_BATTLE_FIELD
bool CPythonNetworkStream::RecvBattleZoneInfo()
{
	TPacketGCBattleInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

#	ifdef ENABLE_RANKING_SYSTEM
	CPythonRanking::Instance().ClearBFRanking();

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TBattleRankingMember kTab;
		if (!Recv(sizeof(kTab), &kTab))
			return false;

		CPythonRanking::Instance().ReceiveBattleFieldRanking(kTab);
		packet.wSize -= sizeof(kTab);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRankingBoard", Py_BuildValue("(ii)", 0, 0));
#	endif

	return true;
}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
bool CPythonNetworkStream::SendAcceClosePacket()
{
	TPacketCGSendAcceClose pPacketAcceClose{};
	pPacketAcceClose.bHeader = HEADER_CG_ACCE_CLOSE_REQUEST;
	if (Send(sizeof(pPacketAcceClose), &pPacketAcceClose))
		return SendSequence();

	Tracef("SendAcceClosePacket Error");
	return false;
}

bool CPythonNetworkStream::SendAcceRefinePacket(uint8_t bAcceWindow, uint8_t bSlotAcce, uint8_t bSlotMaterial)
{
	TPacketCGSendAcceRefine pSendAcceRefinePacket{};
	pSendAcceRefinePacket.bHeader = HEADER_CG_ACCE_REFINE_REQUEST;
	pSendAcceRefinePacket.bAcceWindow = bAcceWindow;
	pSendAcceRefinePacket.bSlotAcce = bSlotAcce;
	pSendAcceRefinePacket.bSlotMaterial = bSlotMaterial;

	if (Send(sizeof(pSendAcceRefinePacket), &pSendAcceRefinePacket))
		return SendSequence();

	Tracef("SendAcceRefinePacket Error");
	return false;
}

bool CPythonNetworkStream::RecvAcceRefinePacket()
{
	TPacketGCSendAcceRefine sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;

	if (sPacket.bResult)
	{
		for (unsigned int i = 0; i < 3; ++i)
			CPythonPlayer::Instance().ClearAcceActivedItemSlot(i);
	}
	else
		CPythonPlayer::Instance().ClearAcceActivedItemSlot(ACCE_SLOT_RIGHT);

	return true;
}
#endif

#ifdef ENABLE_HWID_BAN
bool CPythonNetworkStream::SendHwidBanPacket(int mode, const char* c_szPlayer, const char* c_szReason)
{
	TPacketCGHwidBan pack{};
	pack.header = HEADER_CG_HWID_SYSTEM;
	pack.mode = mode;
	strncpy(pack.szPlayer, c_szPlayer, sizeof(pack.szPlayer));
	strncpy(pack.szReason, c_szReason, sizeof(pack.szReason));

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_CHANGED_ATTR
bool CPythonNetworkStream::RecvSelectAttr()
{
	TPacketGCItemSelectAttr p;

	if (!Recv(sizeof(p), &p))
		return false;

	CPythonPlayer::Instance().SetSelectAttr(p.aAttr);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSelectAttrDialog", Py_BuildValue("(ii)", p.pItemPos.window_type, p.pItemPos.cell));

	return true;
}

bool CPythonNetworkStream::SendChagedItemAttributePacket(const bool bNew, const TItemPos& pos)
{
	TPacketCGItemSelectAttr p;
	p.bHeader = HEADER_CG_ITEM_SELECT_ATTR;
	p.bNew = bNew;
	p.pItemPos = pos;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendChagedItemAttributePacket Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_CUBE_RENEWAL
bool CPythonNetworkStream::SendCubeRefinePacket(int vnum, int multiplier, int indexImprove, int itemReq[5])
{
	TPacketCGCubeRenewal kCubeRenewalPacket;
	kCubeRenewalPacket.wSize = sizeof(TPacketCGCubeRenewal) + sizeof(TSubPacketCGCubeRenwalMake);
	kCubeRenewalPacket.bSubHeader = CUBE_RENEWAL_MAKE;

	TSubPacketCGCubeRenwalMake kCubeRenewalSubPacket;
	kCubeRenewalSubPacket.vnum = vnum;
	kCubeRenewalSubPacket.multiplier = multiplier;
	kCubeRenewalSubPacket.indexImprove = indexImprove;
	//pack.index = index;
	for (int i = 0; i < 5; ++i) {
		kCubeRenewalSubPacket.itemReq[i] = itemReq[i];
		TraceError("%d = %d", i, itemReq[i]);
	}

	if (!Send(sizeof(TPacketCGCubeRenewal), &kCubeRenewalPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGCubeRenwalMake), &kCubeRenewalSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendCubeRenewalClosePacket()
{
	TPacketCGCubeRenewal kCubeRenewalPacket;
	kCubeRenewalPacket.wSize = sizeof(TPacketCGCubeRenewal) + sizeof(TSubPacketCGCubeRenwalClose);
	kCubeRenewalPacket.bSubHeader = CUBE_RENEWAL_CLOSE_STATE;

	TSubPacketCGCubeRenwalClose kCubeRenewalSubPacket;
	kCubeRenewalSubPacket.close_state = 0;

	if (!Send(sizeof(TPacketCGCubeRenewal), &kCubeRenewalPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGCubeRenwalClose), &kCubeRenewalSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvCubeRenewalPacket()
{
	TPacketGCCubeRenewal kCubeRenewalPacket;
	if (!Recv(sizeof(TPacketGCCubeRenewal), &kCubeRenewalPacket))
	{
		Tracef("CubeRenewal Packet Error SubHeader %u\n", kCubeRenewalPacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kCubeRenewalPacket.wSize) - sizeof(TPacketGCCubeRenewal);

	switch (kCubeRenewalPacket.bSubHeader)
	{
	case CUBE_RENEWAL_OPEN:
	case CUBE_RENEWAL_CLOSE:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCubeRenewalOpenClose kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCubeRenewalOpenClose) == 0 && "CUBE_RENEWAL_OPENCLOSE");
			if (!Recv(sizeof(TSubPacketGCCubeRenewalOpenClose), &kSubPacket))
				return false;

			if (kCubeRenewalPacket.bSubHeader == CUBE_RENEWAL_OPEN)
			{
				CPythonPlayer::Instance().SetOpenCubeRenewal(kSubPacket.open_state);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CUBE_RENEWAL_OPEN", Py_BuildValue("(i)", kSubPacket.npc_vnum));
			}

			if (kCubeRenewalPacket.bSubHeader == CUBE_RENEWAL_CLOSE)
			{
				CPythonPlayer::Instance().SetOpenCubeRenewal(kSubPacket.open_state);
				//PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CUBE_RENEWAL_CLOSE", Py_BuildValue("()"));
			}
		}
		else
			TraceError(" RecvCubeRenewalPacket Error 0x04100%u0F", kCubeRenewalPacket.bSubHeader);

		break;
	}
	}

	return true;
}
#endif

#ifdef ENABLE_TICKET_SYSTEM
/*
bool CPythonNetworkStream::RecvTicketInfoLogs()
{

	return true;
}

bool CPythonNetworkStream::RecvTicketInfoLogsReply()
{

	return true;
}
*/
bool CPythonNetworkStream::RecvTicketSystemPacket()
{
	TPacketGCTicketSystem kTicketSystemPacket;
	if (!Recv(sizeof(TPacketGCTicketSystem), &kTicketSystemPacket))
	{
		Tracef("TicketSystem Packet Error SubHeader %u\n", kTicketSystemPacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kTicketSystemPacket.wSize) - sizeof(TPacketGCTicketSystem);

	switch (kTicketSystemPacket.bSubHeader)
	{
	case TICKET_SUBHEADER_GC_LOGS:
	{
		if (iPacketSize > 0)
		{
			TSubPacketTicketLogsData kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketTicketLogsData) == 0 && "HEADER_GC_TICKET_SYSTEM_LOGS");
			if (!Recv(sizeof(TSubPacketTicketLogsData), &kSubPacket))
				return false;

			CPythonTicketLogs::instance()->AddLogDetails(kSubPacket);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Ticket_Logs_General", Py_BuildValue("()"));

		}
		else
			TraceError(" RecvTicketSystemPacket Error 0x04100%u0F", kTicketSystemPacket.bSubHeader);

		break;
	}

	case TICKET_SUBHEADER_GC_LOGS_REPLY:
	{
		if (iPacketSize > 0)
		{
			TSubPacketTicketLogsDataReply kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketTicketLogsDataReply) == 0 && "HEADER_GC_TICKET_SYSTEM_LOGS_REPLY");
			if (!Recv(sizeof(TSubPacketTicketLogsDataReply), &kSubPacket))
				return false;

			CPythonTicketLogsReply::instance()->AddLogDetails(kSubPacket);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Ticket_Logs_Reply", Py_BuildValue("()"));
		}
		else
			TraceError(" RecvTicketSystemPacket Error 0x04101%u0F", kTicketSystemPacket.bSubHeader);
	}
	}

	__RefreshInventoryWindow();

	return true;
}

void SendTicketErrorSize()
{
	char szBuf[256 + 1];
	_snprintf(szBuf, sizeof(szBuf), "<<Ticket>> Error size characters.");
	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
}

bool CPythonNetworkStream::SendTicketPacketOpen(int szAction, int szMode, const char* szTicketID)
{
	if (szAction == 2 && strlen(szTicketID) <= 0)
		return false;

	TPacketCGTicketSystem kTicketSystemPacket;
	TSubPacketCGTicketOpen kTicketSystemSubPacket;
	kTicketSystemPacket.wSize = sizeof(TPacketCGTicketSystem) + sizeof(TSubPacketCGTicketOpen);
	kTicketSystemPacket.bSubHeader = TICKET_SUBHEADER_CG_OPEN;
	kTicketSystemSubPacket.action = szAction;
	kTicketSystemSubPacket.mode = szMode;
	strncpy(kTicketSystemSubPacket.ticked_id, szTicketID, sizeof(kTicketSystemSubPacket.ticked_id));

	if (!Send(sizeof(TPacketCGTicketSystem), &kTicketSystemPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGTicketOpen), &kTicketSystemSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendTicketPacketCreate(const char* szTitle, const char* szContent, int szPriority)
{
	if (strlen(szTitle) <= 0 || strlen(szContent) <= 0 || strlen(szContent) > 450 || strlen(szTitle) > 32)
	{
		SendTicketErrorSize();
		return false;
	}

	TPacketCGTicketSystem kTicketSystemPacket;
	TSubPacketCGTicketCreate kTicketSystemSubPacket;
	kTicketSystemPacket.wSize = sizeof(TPacketCGTicketSystem) + sizeof(TSubPacketCGTicketCreate);
	kTicketSystemPacket.bSubHeader = TICKET_SUBHEADER_CG_CREATE;
	strncpy(kTicketSystemSubPacket.title, szTitle, sizeof(kTicketSystemSubPacket.title));
	strncpy(kTicketSystemSubPacket.content, szContent, sizeof(kTicketSystemSubPacket.content));
	kTicketSystemSubPacket.priority = szPriority;

	if (!Send(sizeof(TPacketCGTicketSystem), &kTicketSystemPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGTicketCreate), &kTicketSystemSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendTicketPacketReply(const char* szTicketID, const char* szReply)
{
	if (strlen(szTicketID) <= 0 || strlen(szReply) <= 0 || strlen(szReply) > 200)
	{
		SendTicketErrorSize();
		return false;
	}

	TPacketCGTicketSystem kTicketSystemPacket;
	TSubPacketCGTicketReply kTicketSystemSubPacket;
	kTicketSystemPacket.wSize = sizeof(TPacketCGTicketSystem) + sizeof(TSubPacketCGTicketReply);
	kTicketSystemPacket.bSubHeader = TICKET_SUBHEADER_CG_REPLY;
	strncpy(kTicketSystemSubPacket.ticked_id, szTicketID, sizeof(kTicketSystemSubPacket.ticked_id));
	strncpy(kTicketSystemSubPacket.reply, szReply, sizeof(kTicketSystemSubPacket.reply));

	if (!Send(sizeof(TPacketCGTicketSystem), &kTicketSystemPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGTicketReply), &kTicketSystemSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendTicketPacketAdmin(int szAction, const char* szTicketID, const char* szName, const char* szReason)
{
	if (strlen(szTicketID) > 10 || strlen(szName) > 12 || strlen(szReason) > 32)
	{
		SendTicketErrorSize();
		return false;
	}

	TPacketCGTicketSystem kTicketSystemPacket;
	TSubPacketCGTicketAdmin kTicketSystemSubPacket;
	kTicketSystemPacket.wSize = sizeof(TPacketCGTicketSystem) + sizeof(TSubPacketCGTicketAdmin);
	kTicketSystemPacket.bSubHeader = TICKET_SUBHEADER_CG_ADMIN;
	kTicketSystemSubPacket.action = szAction;
	strncpy(kTicketSystemSubPacket.ticked_id, szTicketID, sizeof(kTicketSystemSubPacket.ticked_id));
	strncpy(kTicketSystemSubPacket.char_name, szName, sizeof(kTicketSystemSubPacket.char_name));
	strncpy(kTicketSystemSubPacket.reason, szReason, sizeof(kTicketSystemSubPacket.reason));

	if (!Send(sizeof(TPacketCGTicketSystem), &kTicketSystemPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGTicketAdmin), &kTicketSystemSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendTicketPacketAdminChangePage(int iStartPage)
{
	if (iStartPage < 0)
		return false;

	TPacketCGTicketSystem kTicketSystemPacket;
	TSubPacketCGTicketAdminChangePage kTicketSystemSubPacket;
	kTicketSystemPacket.wSize = sizeof(TPacketCGTicketSystem) + sizeof(TSubPacketCGTicketAdminChangePage);
	kTicketSystemPacket.bSubHeader = TICKET_SUBHEADER_CG_ADMIN_PAGE;
	kTicketSystemSubPacket.iStartPage = iStartPage;

	if (!Send(sizeof(TPacketCGTicketSystem), &kTicketSystemPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGTicketAdminChangePage), &kTicketSystemSubPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_MAILBOX
#include "PythonMailBox.h"

bool CPythonNetworkStream::RecvMailboxProcess()
{
	TPacketMailboxProcess p;

	if (!Recv(sizeof(p), &p))
		return false;

	switch (p.bSubHeader)
	{
	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_CLOSE:
		CPythonMailBox::Instance().Destroy();
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", p.bSubHeader, p.bArg1));
		break;

	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_OPEN:
	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_POST_WRITE_CONFIRM:
	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_POST_WRITE:
	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_POST_ALL_DELETE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", p.bSubHeader, p.bArg1));
		break;

	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_ADD_DATA:
	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_POST_DELETE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(i(ii))", p.bSubHeader, p.bArg1, p.bArg2));
		break;

	case CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS:
		CPythonMailBox::Instance().ResetAddData(p.bArg1);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(i(ii))", p.bSubHeader, p.bArg1, p.bArg2));
		break;

	default:
		TraceError("CPythonNetworkStream::RecvMailboxProcess: Unknown subheader: %d\n", p.bSubHeader);
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvMailbox()
{
	TPacketGCMailBox p;
	if (!Recv(sizeof(p), &p))
		return false;

	CPythonMailBox::Instance().Destroy();

	unsigned int iPacketSize = (p.wSize - sizeof(TPacketGCMailBox));
	for (; iPacketSize > 0; iPacketSize -= sizeof(TPacketGCMailBoxMessage))
	{
		TPacketGCMailBoxMessage Message;
		if (!Recv(sizeof(Message), &Message))
			return false;

		CPythonMailBox::Instance().AddMail(new CPythonMailBox::SMailBox(Message.SendTime, Message.DeleteTime, Message.szTitle, Message.bIsGMPost, Message.bIsItemExist, Message.bIsConfirm));
	}

	return true;
}

bool CPythonNetworkStream::RecvMailboxAddData()
{
	TPacketGCMailBoxAddData p;

	if (!Recv(sizeof(p), &p))
		return false;

	CPythonMailBox::SMailBox* _Data = CPythonMailBox::Instance().GetMail(p.Index);
	if (_Data == nullptr)
	{
		Tracef("RecvMailboxAddData Error: SMailBox is null.\n");
		return true;
	}

	CPythonMailBox::SMailBoxAddData*& _AddData = _Data->AddData;
	if (_AddData != nullptr)
	{
		Tracef("RecvMailboxAddData Error: SMailBoxAddData is not null.\n");
		return true;
	}

	_Data->bIsConfirm = true;
	_AddData = new CPythonMailBox::SMailBoxAddData(p.szFrom, p.szMessage, p.iYang, p.iWon, p.ItemVnum, p.ItemCount, p.alSockets, p.aAttr
#ifdef ENABLE_YOHARA_SYSTEM
		, p.aApplyRandom
		, p.alRandomValues
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		, p.dwTransmutationVnum
#endif
#ifdef ENABLE_REFINE_ELEMENT
		, p.grade_element
		, p.attack_element
		, p.element_type_bonus
		, p.elements_value_bonus
#endif
#ifdef ENABLE_SET_ITEM
		, p.set_value
#endif
	);
	return true;
}

bool CPythonNetworkStream::RecvMailboxAll()
{
	TPacketGCMailBox p;
	if (!Recv(sizeof(p), &p))
		return false;

	PyObject* list = PyList_New(0);

	unsigned int iPacketSize = (p.wSize - sizeof(TPacketGCMailBox));
	for (; iPacketSize > 0; iPacketSize -= sizeof(TPacketGCMailboxProcessAll))
	{
		TPacketGCMailboxProcessAll Index;
		if (!Recv(sizeof(Index), &Index))
			return false;

		CPythonMailBox::Instance().ResetAddData(Index.Index);
		PyList_Append(list, Py_BuildValue("i", Index.Index));
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("i(iO)",
		CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_POST_ALL_GET_ITEMS, CPythonMailBox::EMAILBOX_POST_ALL_GET_ITEMS::POST_ALL_GET_ITEMS_OK, list));

	Py_DECREF(list);
	return true;
}

bool CPythonNetworkStream::RecvMailboxUnread()
{
	TMailBoxRespondUnreadData p;

	if (!Recv(sizeof(p), &p))
		return false;

	const bool bFlash = p.bItemMessageCount > 0;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("i(iiiii)", CPythonMailBox::EMAILBOX_GC::MAILBOX_GC_UNREAD_DATA,
		bFlash, (p.bItemMessageCount + p.bCommonMessageCount), p.bItemMessageCount, p.bCommonMessageCount, p.bGMVisible));

	return true;
}

bool CPythonNetworkStream::SendPostWriteConfirm(const char* szName)
{
	TPacketCGMailboxWriteConfirm p{};
	p.bHeader = HEADER_CG_MAILBOX_WRITE_CONFIRM;
	std::strcpy(p.szName, szName);

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostWriteConfirm Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxClose()
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMailBox::EMAILBOX_CG::MAILBOX_CG_CLOSE;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendMailBoxClose Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostDelete(const uint8_t Index)
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMailBox::EMAILBOX_CG::MAILBOX_CG_DELETE;
	p.bArg1 = Index;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostDelete Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostAllDelete()
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMailBox::EMAILBOX_CG::MAILBOX_CG_ALL_DELETE;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostAllDelete Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostGetItems(const uint8_t Index)
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMailBox::EMAILBOX_CG::MAILBOX_CG_GET_ITEMS;
	p.bArg1 = Index;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostGetItems Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostAllGetItems()
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMailBox::EMAILBOX_CG::MAILBOX_CG_ALL_GET_ITEMS;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostAllGetItems Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RequestPostAddData(const uint8_t ButtonIndex, const uint8_t DataIndex)
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMailBox::EMAILBOX_CG::MAILBOX_CG_ADD_DATA;
	p.bArg1 = ButtonIndex;
	p.bArg2 = DataIndex;

	if (!Send(sizeof(p), &p))
	{
		Tracef("RequestPostAddData Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostWrite(const char* szName, const char* szTitle, const char* szMessage, const TItemPos& pos, const int iYang, const int iWon)
{
	TPacketCGMailboxWrite p;
	p.bHeader = HEADER_CG_MAILBOX_WRITE;
	std::strcpy(p.szName, szName);
	std::strcpy(p.szTitle, szTitle);
	std::strcpy(p.szMessage, szMessage);
	p.pos = pos;
	p.iYang = iYang;
	p.iWon = iWon;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostWrite Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM

#define TransmutationPacket(x) TPacketCGTransmutation p(static_cast<std::underlying_type_t<ECG_TRANSMUTATION_SHEADER>>(ECG_TRANSMUTATION_SHEADER::x))

bool CPythonNetworkStream::SendChangeLookCheckInPacket(const TItemPos& pos, const uint8_t bSlotType)
{
	TransmutationPacket(ITEM_CHECK_IN);
	p.slot_type = bSlotType;
	p.pos = pos;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckInPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCheckOutPacket(const uint8_t bSlotType)
{
	TransmutationPacket(ITEM_CHECK_OUT);
	p.slot_type = bSlotType;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckOutPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCheckInFreeYangItemPacket(const TItemPos& pos)
{
	TransmutationPacket(FREE_ITEM_CHECK_IN);
	p.pos = pos;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckInFreeYangItemPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCheckOutFreeYangItemPacket()
{
	TransmutationPacket(FREE_ITEM_CHECK_OUT);

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckOutFreeYangItemPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookAcceptPacket()
{
	TransmutationPacket(ACCEPT);

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookAcceptPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCancelPacket()
{
	TransmutationPacket(CANCEL);

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCancelPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvChangeLookPacket()
{
	TPacketGCTransmutation kChangeLookPacket;
	if (!Recv(sizeof(TPacketGCTransmutation), &kChangeLookPacket))
	{
		Tracef("ChangeLook Packet Error SubHeader %u\n", kChangeLookPacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kChangeLookPacket.wSize) - sizeof(TPacketGCTransmutation);

	switch (kChangeLookPacket.bSubHeader)
	{
	case static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_SET):
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCChangeLookItemSet kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCChangeLookItemSet) == 0 && "CL_SUBHEADER_GC_SET");
			if (!Recv(sizeof(TSubPacketGCChangeLookItemSet), &kSubPacket))
				return false;

			if (kSubPacket.bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
				return true;

			CPythonPlayer::Instance().SetChangeLookItemData(kSubPacket);

			__RefreshInventoryWindow();
		}
		else
			TraceError(" RecvChangeLookPacket Error 0x04100%u0F", kChangeLookPacket.bSubHeader);
	}
	break;

	case static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_DEL):
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCChangeLookDel kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCChangeLookDel) == 0 && "CL_SUBHEADER_GC_DEL");
			if (!Recv(sizeof(TSubPacketGCChangeLookDel), &kSubPacket))
				return false;

			CPythonPlayer::Instance().DelChangeLookItemData(kSubPacket.bSlotType, true);

			__RefreshInventoryWindow();
		}
		else
			TraceError(" RecvChangeLookPacket Error 0x04101%u0F", kChangeLookPacket.bSubHeader);
	}
	break;

	case static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_FREE_SET):
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCChangeLookItemSet kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCChangeLookItemSet) == 0 && "CL_SUBHEADER_GC_SET");
			if (!Recv(sizeof(TSubPacketGCChangeLookItemSet), &kSubPacket))
				return false;

			CPythonPlayer::Instance().SetChangeLookFreeItemData(kSubPacket);

			__RefreshInventoryWindow();
		}
		else
			TraceError(" RecvChangeLookPacket Error 0x04102%u0F", kChangeLookPacket.bSubHeader);
	}
	break;

	case static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_FREE_DEL):
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCChangeLookDel kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCChangeLookDel) == 0 && "CL_SUBHEADER_GC_FREE_DEL");
			if (!Recv(sizeof(TSubPacketGCChangeLookDel), &kSubPacket))
				return false;

			CPythonPlayer::Instance().DelChangeLookFreeItemData(true);

			__RefreshInventoryWindow();
		}
		else
			TraceError(" RecvChangeLookPacket Error 0x04103%u0F", kChangeLookPacket.bSubHeader);
	}
	break;
	}

	return true;
}
#endif

#ifdef ENABLE_MOVE_CHANNEL
bool CPythonNetworkStream::SendMoveChannelPacket(const uint8_t bChannelIndex)
{
	TPacketCGMoveChannel p{};
	p.header = HEADER_CG_MOVE_CHANNEL;
	p.channel = bChannelIndex;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendMoveChannelPacket Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_PARTY_MATCH
bool CPythonNetworkStream::PartyMatch(int index, uint8_t setting)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGPartyMatch packet{};
	packet.Header = HEADER_CG_PARTY_MATCH;
	packet.index = index;
	packet.SubHeader = setting;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Error in CPythonNetworkStream::PartyMatch");
		return false;
	}

	return SendSequence();
}
bool CPythonNetworkStream::RecvPartyMatch()
{
	TPacketGCPartyMatch Packet;
	if (!Recv(sizeof(Packet), &Packet))
	{
		Tracen("RecvPartyMatch Error");
		return false;
	}

	switch (Packet.SubHeader)
	{
	case CPythonPlayer::EPacketGCPartyMatchSubHeader::PARTY_MATCH_SEARCH:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PartyMatchResult", Py_BuildValue("(ii)", Packet.MSG, Packet.index));
		break;

	case CPythonPlayer::EPacketGCPartyMatchSubHeader::PARTY_MATCH_CANCEL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PartyMatchResult", Py_BuildValue("(i(ii))", Packet.SubHeader, Packet.MSG, Packet.index));
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
bool CPythonNetworkStream::RecvDungeonInfo()
{
	TPacketGCDungeonInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	switch (packet.bySubHeader)
	{
	case SUBHEADER_DUNGEON_INFO_SEND:
	{
		CPythonDungeonInfo::Instance().AddDungeon(packet.byIndex, packet);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonInfoReload", Py_BuildValue("(i)", packet.bReset));
	}
	break;

	case SUBHEADER_DUNGEON_INFO_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonInfoOpen", Py_BuildValue("()"));
		break;
	}

	return true;
}

bool CPythonNetworkStream::SendDungeonInfo(uint8_t byAction, uint8_t byIndex, uint8_t byRankType)
{
	TPacketCGDungeonInfo packet;
	packet.byHeader = HEADER_CG_DUNGEON_INFO;
	packet.bySubHeader = byAction;
	packet.byIndex = byIndex;
	packet.byRankType = byRankType;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvDungeonRanking()
{
	TPacketGCDungeonInfoRank packet;
	if (!Recv(sizeof(packet), &packet))
	{
		Tracen("RecvDungeonRanking Error");
		return false;
	}

	CPythonDungeonInfo::Instance().AddRanking(packet.szName, packet.iLevel, packet.dwPoints);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DungeonRankingRefresh", Py_BuildValue("()"));

	return true;
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
bool CPythonNetworkStream::SendExtBattlePassAction(uint8_t bAction)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExtBattlePassAction packet;
	packet.bHeader = HEADER_CG_EXT_BATTLE_PASS_ACTION;
	packet.bAction = bAction;

	if (!Send(sizeof(TPacketCGExtBattlePassAction), &packet))
	{
		Tracef("SendExtBattlePassAction Send Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExtBattlePassPremium(bool premium)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExtBattlePassSendPremium packet;
	packet.bHeader = HEADER_CG_EXT_SEND_BP_PREMIUM;
	packet.premium = premium;

	if (!Send(sizeof(TPacketCGExtBattlePassSendPremium), &packet))
	{
		Tracef("SendExtBattlePassPremiumItem Send Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvExtBattlePassOpenPacket()
{
	SPacketGCExtBattlePassOpen packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtOpenBattlePass", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvExtBattlePassGeneralInfoPacket()
{
	TPacketGCExtBattlePassGeneralInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtBattlePassAddGeneralInfo", Py_BuildValue("(isiii)", packet.bBattlePassType, packet.szSeasonName, packet.dwBattlePassID, packet.dwBattlePassStartTime, packet.dwBattlePassEndTime));
	return true;
}

bool CPythonNetworkStream::RecvExtBattlePassMissionInfoPacket()
{
	TPacketGCExtBattlePassMissionInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);

	while (packet.wSize > 0)
	{
		TExtBattlePassMissionInfo missionInfo;
		if (!Recv(sizeof(missionInfo), &missionInfo))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtBattlePassAddMission", Py_BuildValue("(iiiiiii)",
			packet.bBattlePassType, packet.dwBattlePassID, missionInfo.bMissionIndex, missionInfo.bMissionType, missionInfo.dwMissionInfo[0], missionInfo.dwMissionInfo[1], missionInfo.dwMissionInfo[2]));

		for (int i = 0; i < 3; ++i)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtBattlePassAddMissionReward", Py_BuildValue("(iiiiii)",
				packet.bBattlePassType, packet.dwBattlePassID, missionInfo.bMissionIndex, missionInfo.bMissionType, missionInfo.aRewardList[i].dwVnum, missionInfo.aRewardList[i].bCount));

		packet.wSize -= sizeof(missionInfo);
	}

	while (packet.wRewardSize > 0)
	{
		TExtBattlePassRewardItem rewardInfo;
		if (!Recv(sizeof(rewardInfo), &rewardInfo))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtBattlePassAddReward", Py_BuildValue("(iiii)", packet.bBattlePassType, packet.dwBattlePassID, rewardInfo.dwVnum, rewardInfo.bCount));

		packet.wRewardSize -= sizeof(rewardInfo);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtOpenBattlePass", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvExtBattlePassMissionUpdatePacket()
{
	TPacketGCExtBattlePassMissionUpdate packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtBattlePassUpdate", Py_BuildValue("(iiii)", packet.bBattlePassType, packet.bMissionIndex, packet.bMissionType, packet.dwNewProgress));
	return true;
}

bool CPythonNetworkStream::RecvExtBattlePassRankingPacket()
{
	TPacketGCExtBattlePassRanking packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ExtBattlePassAddRanklistEntry", Py_BuildValue("(siiii)",
		packet.szPlayerName, packet.bBattlePassType, packet.bBattlePassID, packet.dwStartTime, packet.dwEndTime
	));
	return true;
}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
bool CPythonNetworkStream::RecvMeleyLairRankingTimeInfo()
{
	TPacketMeleyLairTimeInfo packet;

	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetGuildDragonLairFistGuildText",
		Py_BuildValue("(i)", packet.dwTime));

	return true;
}

bool CPythonNetworkStream::RecvMeleyLairRankingTimeResult()
{
	TPacketMeleyLairTimeResult packet;

	if (!Recv(sizeof(packet), &packet))
		return false;

	switch (packet.bResult)
	{
	case 0:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetGuildDragonLiarStart",
			Py_BuildValue("()"));
		break;

	case 1:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetGuildDragonLiarSuccess",
			Py_BuildValue("()"));
		break;

	default:
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
bool CPythonNetworkStream::SendBiologManagerAction(uint8_t bSubHeader)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGBiologManagerAction packet{};
	packet.bHeader = HEADER_CG_BIOLOG_MANAGER;
	packet.bSubHeader = bSubHeader;

	if (!Send(sizeof(TPacketCGBiologManagerAction), &packet))
	{
		Tracef("SendBiologManagerAction Send Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvBiologManager()
{
	TPacketGCBiologManager packet;
	if (!Recv(sizeof(packet), &packet))
	{
		TraceError("RecvBiologManager Error");
		return false;
	}

	switch (packet.bSubHeader)
	{
	case BIOLOG_MANAGER_SUBHEADER_GC_OPEN:
	{
		TPacketGCBiologManagerInfo kInfo;
		if (!Recv(sizeof(kInfo), &kInfo))
			return false;

		CPythonBiologManager::Instance()._LoadBiologInformation(&kInfo);
		if (kInfo.bUpdate)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BiologManagerUpdate", Py_BuildValue("()"));
		else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BiologManagerOpen", Py_BuildValue("()"));
	}
	break;

	case BIOLOG_MANAGER_SUBHEADER_GC_CLOSE:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BiologManagerClose", Py_BuildValue("()"));
	}
	break;

	default:
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_AUTO_SYSTEM
void CPythonNetworkStream::SendAutoCoolTime(int slotIndex, int iValue)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetAutoCoolTime", Py_BuildValue("(ii)", slotIndex, iValue));
}
#endif

#ifdef ENABLE_HUNTING_SYSTEM
bool CPythonNetworkStream::SendHuntingAction(uint8_t bAction, uint32_t dValue)
{
	if (!__CanActMainInstance())
		return true;

	TPacketGCHuntingAction packet;
	packet.bHeader = HEADER_CG_SEND_HUNTING_ACTION;
	packet.bAction = bAction;
	packet.dValue = dValue;

	if (!Send(sizeof(TPacketGCHuntingAction), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvHuntingOpenWindowMain()
{
	TPacketGCOpenWindowHuntingMain packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenHuntingWindowMain", Py_BuildValue("(iiiiiiiiii)",
		packet.dLevel, packet.dMonster, packet.dCurCount, packet.dDestCount, packet.dMoneyMin, packet.dMoneyMax, packet.dExpMin, packet.dExpMax, packet.dRaceItem, packet.dRaceItemCount
	));

	return true;
}

bool CPythonNetworkStream::RecvHuntingOpenWindowSelect()
{
	TPacketGCOpenWindowHuntingSelect packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenHuntingWindowSelect", Py_BuildValue("(iiiiiiiiii)",
		packet.dLevel, packet.bType, packet.dMonster, packet.dCount, packet.dMoneyMin, packet.dMoneyMax, packet.dExpMin, packet.dExpMax, packet.dRaceItem, packet.dRaceItemCount
	));

	return true;
}

bool CPythonNetworkStream::RecvHuntingOpenWindowReward()
{
	TPacketGCOpenWindowReward packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenHuntingWindowReward", Py_BuildValue("(iiiiiii)", packet.dLevel, packet.dReward, packet.dRewardCount, packet.dRandomReward, packet.dRandomRewardCount, packet.dMoney, packet.bExp));

	return true;
}

bool CPythonNetworkStream::RecvHuntingUpdate()
{
	TPacketGCUpdateHunting packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateHuntingMission", Py_BuildValue("(i)", packet.dCount));

	return true;
}

bool CPythonNetworkStream::RecvHuntingRandomItems()
{
	TPacketGCReciveRandomItems packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_HuntingReciveRandomItem", Py_BuildValue("(iii)", packet.bWindow, packet.dItemVnum, packet.dItemCount));

	return true;
}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
bool CPythonNetworkStream::RecvLottoOpenWindows()
{
	TPacketGCLotteryOpenings packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	switch (packet.subheader)
	{
	case SUBHEADER_GC_OPEN_LOTTERY_WINDOW:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenLotteryWindow", Py_BuildValue("()"));
		break;
	}
	case SUBHEADER_GC_OPEN_RANKING_WINDOW:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenLotteryRankingWindow", Py_BuildValue("()"));
		break;
	}

	default:
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvLottoBasicInfo()
{
	TPacketGCReciveLottoBaseInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LotterySetBaseData", Py_BuildValue("(iiiiii)",
		packet.lottoSlot,
		packet.lottoNum,
		packet.num1,
		packet.num2,
		packet.num3,
		packet.num4
	));

	PyObject* args = PyTuple_New(3);
	PyTuple_SetItem(args, 0, PyLong_FromLongLong(packet.lottoSlot));
	PyTuple_SetItem(args, 1, PyLong_FromLongLong(packet.jackpot));
	PyTuple_SetItem(args, 2, PyLong_FromLongLong(packet.nextRefresh));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LotterySetLongLongValues", args);
	return true;
}

bool CPythonNetworkStream::RecvLottoTicketInfo()
{
	TPacketGCReciveLottoTicketInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyObject* args = PyTuple_New(11);
	PyTuple_SetItem(args, 0, PyLong_FromLongLong(packet.tNum));
	PyTuple_SetItem(args, 1, PyLong_FromLongLong(packet.tID));
	PyTuple_SetItem(args, 2, PyLong_FromLongLong(packet.num1));
	PyTuple_SetItem(args, 3, PyLong_FromLongLong(packet.num2));
	PyTuple_SetItem(args, 4, PyLong_FromLongLong(packet.num3));
	PyTuple_SetItem(args, 5, PyLong_FromLongLong(packet.num4));
	PyTuple_SetItem(args, 6, PyLong_FromLongLong(packet.lottoID));
	PyTuple_SetItem(args, 7, Py_BuildValue("s", packet.buytime));
	PyTuple_SetItem(args, 8, PyLong_FromLongLong(packet.state));
	PyTuple_SetItem(args, 9, PyLong_FromLongLong(packet.winNumbers));
	PyTuple_SetItem(args, 10, PyLong_FromLongLong(packet.MoneyWin));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LotterySetTicketData", args);
	return true;
}

bool CPythonNetworkStream::RecvLottoRankingJackpotInfo()
{
	TPacketGCSendRankingJackpotInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyObject* args = PyTuple_New(4);
	PyTuple_SetItem(args, 0, Py_BuildValue("s", packet.playername));
	PyTuple_SetItem(args, 1, Py_BuildValue("i", packet.lottoID));
	PyTuple_SetItem(args, 2, PyLong_FromLongLong(packet.money));
	PyTuple_SetItem(args, 3, Py_BuildValue("s", packet.date));

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LotterySetRanklistJackpotData", args);
	return true;
}

bool CPythonNetworkStream::RecvLottoRankingMoneyInfo()
{
	TPacketGCSendRankingMoneyInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyObject* args = PyTuple_New(4);
	PyTuple_SetItem(args, 0, Py_BuildValue("s", packet.playername));
	PyTuple_SetItem(args, 1, Py_BuildValue("i", packet.level));
	PyTuple_SetItem(args, 2, Py_BuildValue("i", packet.empire));
	PyTuple_SetItem(args, 3, PyLong_FromLongLong(packet.money));

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LotterySetRanklistMoneyData", args);
	return true;
}

bool CPythonNetworkStream::SendOpenLottoWindow()
{
	TPacketCGLotteryOpenings pack;
	pack.header = HEADER_CG_LOTTO_OPENINGS;
	pack.subheader = SUBHEADER_CG_OPEN_LOTTERY_WINDOW;

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendOpenRankingWindow()
{
	TPacketCGLotteryOpenings pack;
	pack.header = HEADER_CG_LOTTO_OPENINGS;
	pack.subheader = SUBHEADER_CG_OPEN_RANKING_WINDOW;

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendLottoBuyTicket(int slot, int num1, int num2, int num3, int num4)
{
	TPacketCGSendLottoNewTicket pack;
	pack.header = HEADER_CG_LOTTO_BUY_TICKET;
	pack.slot = slot;
	pack.num1 = num1;
	pack.num2 = num2;
	pack.num3 = num3;
	pack.num4 = num4;

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendLottoDeleteTicket(int slot)
{
	TPacketCGSendTicketOptions pack;
	pack.header = HEADER_CG_LOTTO_TICKET_OPTIONS;
	pack.subheader = SUBHEADER_CG_TICKET_DELETE;
	pack.ticketSlot = slot;

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendLottoReciveMoney(int slot)
{
	TPacketCGSendTicketOptions pack;
	pack.header = HEADER_CG_LOTTO_TICKET_OPTIONS;
	pack.subheader = SUBHEADER_CG_RECIVE_MONEY;
	pack.ticketSlot = slot;

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendLottoPickMoney(long long amount)
{
	TPacketCGSendLottoPickMoney pack;
	pack.header = HEADER_CG_LOTTO_PICK_MONEY;
	pack.amount = amount;

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}
#endif

// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
bool CPythonNetworkStream::SendRequestEventData(int iMonth)
{
	// Return if data has been already requested
	if (InGameEventManager::Instance().GetRequestEventData())
		return false;

	SPacketCGRequestEventData p{};
	p.bHeader = HEADER_CG_REQUEST_EVENT_DATA;
	p.bMonth = uint8_t(iMonth);

	if (!Send(sizeof(p), &p))
	{
		Tracen("Send SendRequestEventData Packet Error");
		return false;
	}

	Tracef("SendRequestEventData\n");
	return SendSequence();
}

bool CPythonNetworkStream::RecvEventInformation()
{
	TPacketGCEventInfo p;

	if (!Recv(sizeof(TPacketGCEventInfo), &p))
	{
		TraceError("CPythonNetworkStream::RecvEventInformation TPacketGCEventInfo Error");
		return false;
	}

	const time_t serverTimeStamp = CPythonApplication::Instance().GetServerTimeStamp();
	const time_t clientTimeStamp = time(0);
	const time_t deltaTime = clientTimeStamp - serverTimeStamp;

	std::vector<InGameEventManager::TEventTable> eventVec;

	int iPacketSize = (p.wSize - sizeof(TPacketGCEventInfo));
	for (; iPacketSize > 0; iPacketSize -= sizeof(TPacketEventData))
	{
		TPacketEventData kElement;
		if (!Recv(sizeof(TPacketEventData), &kElement))
		{
			TraceError("CPythonNetworkStream::RecvEventInformation TPacketEventData Error");
			return false;
		}

		InGameEventManager::TEventTable table;
		table.dwID = kElement.dwID;
		table.bType = kElement.bType;
		table.startTime = kElement.startTime + deltaTime;
		table.endTime = kElement.endTime + deltaTime;
		table.dwVnum = kElement.dwVnum;
		table.iPercent = kElement.iPercent;
		table.iDropType = kElement.iDropType;
		table.bCompleted = kElement.bCompleted;

		eventVec.emplace_back(table);
	}

	InGameEventManager::Instance().SetRequestEventData(true);
	InGameEventManager::Instance().AddEventData(eventVec);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenInGameEvent", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvEventReload()
{
	TPacketGCEventReload p;

	if (!Recv(sizeof(TPacketGCEventReload), &p))
	{
		TraceError("CPythonNetworkStream::RecvEventReload TPacketGCEventReload Error");
		return false;
	}

	InGameEventManager::Instance().SetRequestEventData(false);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CloseInGameEvent", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::SendRequestEventQuest(const char* c_szString)
{
	TPacketCGRequestEventQuest Packet{};
	Packet.bHeader = HEADER_CG_REQUEST_EVENT_QUEST;
	strncpy(Packet.szName, c_szString, QUEST_NAME_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendRequestEventQuest Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvEventKWScore()
{
	TPacketGCEventKWScore p;

	if (!Recv(sizeof(TPacketGCEventKWScore), &p))
	{
		TraceError("CPythonNetworkStream::RecvEventKWScore TPacketGCEventKWScore Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_KingdomWarScore", Py_BuildValue("(iii)", p.wKingdomScores[0], p.wKingdomScores[1], p.wKingdomScores[2]));
	return true;
}
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
bool CPythonNetworkStream::SendMiniGameRumiStart(int safemode)
{
	TPacketCGMiniGameOkeyCard kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameOkeyCard) + sizeof(TSubPacketCGMiniGameCardOpenClose);
	kMiniGamePacket.bSubHeader = SUBHEADER_CG_RUMI_START;

	TSubPacketCGMiniGameCardOpenClose kMiniGameSubPacket;
	kMiniGameSubPacket.bSafeMode = safemode;

	if (!Send(sizeof(TPacketCGMiniGameOkeyCard), &kMiniGamePacket))
		return false;

	if (!Send(sizeof(TSubPacketCGMiniGameCardOpenClose), &kMiniGameSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameRumiExit()
{
	TPacketCGMiniGameOkeyCard kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameOkeyCard);
	kMiniGamePacket.bSubHeader = SUBHEADER_CG_RUMI_EXIT;

	if (!Send(sizeof(TPacketCGMiniGameOkeyCard), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameRumiDeckCardClick()
{
	TPacketCGMiniGameOkeyCard kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameOkeyCard);
	kMiniGamePacket.bSubHeader = SUBHEADER_CG_RUMI_DECKCARD_CLICK;

	if (!Send(sizeof(TPacketCGMiniGameOkeyCard), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameRumiHandCardClick(int index)
{
	TPacketCGMiniGameOkeyCard kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameOkeyCard) + sizeof(TSubPacketCGMiniGameHandCardClick);
	kMiniGamePacket.bSubHeader = SUBHEADER_CG_RUMI_HANDCARD_CLICK;

	TSubPacketCGMiniGameHandCardClick kMiniGameSubPacket;
	kMiniGameSubPacket.index = index;

	if (!Send(sizeof(TPacketCGMiniGameOkeyCard), &kMiniGamePacket))
		return false;

	if (!Send(sizeof(TSubPacketCGMiniGameHandCardClick), &kMiniGameSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameRumiFieldCardClick(int index)
{
	TPacketCGMiniGameOkeyCard kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameOkeyCard) + sizeof(TSubPacketCGMiniGameFieldCardClick);
	kMiniGamePacket.bSubHeader = SUBHEADER_CG_RUMI_FIELDCARD_CLICK;

	TSubPacketCGMiniGameFieldCardClick kMiniGameSubPacket;
	kMiniGameSubPacket.index = index;

	if (!Send(sizeof(TPacketCGMiniGameOkeyCard), &kMiniGamePacket))
		return false;

	if (!Send(sizeof(TSubPacketCGMiniGameFieldCardClick), &kMiniGameSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameRumiDestroy(int index)
{
	TPacketCGMiniGameOkeyCard kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameOkeyCard) + sizeof(TSubPacketCGMiniGameDestroy);
	kMiniGamePacket.bSubHeader = SUBHEADER_CG_RUMI_DESTROY;

	TSubPacketCGMiniGameDestroy kMiniGameSubPacket;
	kMiniGameSubPacket.index = index;

	if (!Send(sizeof(TPacketCGMiniGameOkeyCard), &kMiniGamePacket))
		return false;

	if (!Send(sizeof(TSubPacketCGMiniGameDestroy), &kMiniGameSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMiniGameOkeyCardPacket()
{
	TPacketGCMiniGameOkeyCard kMiniGamePacket;
	if (!Recv(sizeof(TPacketGCMiniGameOkeyCard), &kMiniGamePacket))
	{
		Tracef("MiniGame Packet Error SubHeader %u\n", kMiniGamePacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kMiniGamePacket.wSize) - sizeof(TPacketGCMiniGameOkeyCard);

	switch (kMiniGamePacket.bSubHeader)
	{
	case SUBHEADER_GC_RUMI_OPEN:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCMiniGameCardOpenClose kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCMiniGameCardOpenClose) == 0 && "MINI_GAME_RUMI_OPENCLOSE");
			if (!Recv(sizeof(TSubPacketGCMiniGameCardOpenClose), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameStart", Py_BuildValue("(i)", kSubPacket.bSafeMode));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x04100%u0F", kMiniGamePacket.bSubHeader);

		break;
	}

	case SUBHEADER_GC_RUMI_CARD_UPDATE:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCMiniGameCardsInfo kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCMiniGameCardsInfo) == 0 && "MINI_GAME_RUMI_CARD_UPDATE");
			if (!Recv(sizeof(TSubPacketGCMiniGameCardsInfo), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_UpdateInfo", Py_BuildValue("(iiiiiiiiiiii)",
				kSubPacket.cardHandType[0], kSubPacket.cardHandValue[0], kSubPacket.cardHandType[1], kSubPacket.cardHandValue[1], kSubPacket.cardHandType[2], kSubPacket.cardHandValue[2], kSubPacket.cardHandType[3], kSubPacket.cardHandValue[3], kSubPacket.cardHandType[4], kSubPacket.cardHandValue[4], kSubPacket.cardHandLeft, kSubPacket.cHandPoint));

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_FieldUpdateInfo", Py_BuildValue("(iiiiiii)", kSubPacket.cardFieldType[0], kSubPacket.cardFieldValue[0], kSubPacket.cardFieldType[1], kSubPacket.cardFieldValue[1], kSubPacket.cardFieldType[2], kSubPacket.cardFieldValue[2], kSubPacket.cFieldPoint));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBABE", kMiniGamePacket.bSubHeader);
		break;
	}

	case SUBHEADER_GC_RUMI_CARD_REWARD:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCMiniGameCardsReward kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCMiniGameCardsReward) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCMiniGameCardsReward), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_PutReward", Py_BuildValue("(iiiiiii)", kSubPacket.cardType[0], kSubPacket.cardValue[0], kSubPacket.cardType[1], kSubPacket.cardValue[1], kSubPacket.cardType[2], kSubPacket.cardValue[2], kSubPacket.cPoint));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBA", kMiniGamePacket.bSubHeader);
		break;
	}
	}

	return true;
}
#endif
#ifdef ENABLE_MONSTER_BACK
bool CPythonNetworkStream::SendAttendanceGetReward()
{
	constexpr uint8_t header = HEADER_CG_ATTENDANCE_REWARD;
	if (!Send(sizeof(uint8_t), &header))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvAttendanceEvent()
{
	TPacketGCAttendanceEvent packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameAttendanceSetData", Py_BuildValue("(ii)", packet.bType, packet.bValue));

	return true;
}

bool CPythonNetworkStream::RecvAttendanceEventInfo()
{
	TPacketGCAttendanceEventInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TRewardItem kReward;
		if (!Recv(sizeof(kReward), &kReward))
			return false;

		CPythonPlayer::Instance().SetRewardItem(kReward.dwDay, kReward.dwVnum, kReward.dwCount);

		packet.wSize -= sizeof(kReward);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameAttendanceSetData", Py_BuildValue("(ii)", 1, packet.dwDay));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameAttendanceSetData", Py_BuildValue("(ii)", 2, packet.bClear));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameAttendanceSetData", Py_BuildValue("(ii)", 3, packet.bReward));

	return true;
}
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
bool CPythonNetworkStream::RecvHitCountInfo()
{
	TPacketGCHitCountInfo hitCountPacket;
	if (!Recv(sizeof(hitCountPacket), &hitCountPacket))
		return false;

	CPythonPlayer::Instance().SetAccumulateDamage(hitCountPacket.dwVid, hitCountPacket.dwCount);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAccumulateCount", Py_BuildValue("(i)", hitCountPacket.dwVid));

	return true;
}
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
bool CPythonNetworkStream::SendMiniGameCatchKingStart(int ibetNumber)
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameCatchKing) + sizeof(TSubPacketCGMiniGameCatchKingStart);
	kMiniGamePacket.bSubheader = SUBHEADER_CG_CATCH_KING_START;

	TSubPacketCGMiniGameCatchKingStart kMiniGameSubPacket;
	kMiniGameSubPacket.betNumber = ibetNumber;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	if (!Send(sizeof(TSubPacketCGMiniGameCatchKingStart), &kMiniGameSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameCatchKingDeckCardClick()
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameCatchKing);
	kMiniGamePacket.bSubheader = SUBHEADER_CG_CATCH_KING_DECKCARD_CLICK;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameCatchKingFieldCardClick(int icardNumber)
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameCatchKing) + sizeof(TSubPacketCGMiniGameCatchKingFieldCardClick);
	kMiniGamePacket.bSubheader = SUBHEADER_CG_CATCH_KING_FIELDCARD_CLICK;

	TSubPacketCGMiniGameCatchKingFieldCardClick kMiniGameSubPacket;
	kMiniGameSubPacket.cardNumber = icardNumber;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	if (!Send(sizeof(TSubPacketCGMiniGameCatchKingFieldCardClick), &kMiniGameSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameCatchKingReward()
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.wSize = sizeof(TPacketCGMiniGameCatchKing);
	kMiniGamePacket.bSubheader = SUBHEADER_CG_CATCH_KING_REWARD;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMiniGameCatchKingPacket()
{
	TPacketGCMiniGameCatchKing kMiniGamePacket;
	if (!Recv(sizeof(TPacketGCMiniGameCatchKing), &kMiniGamePacket))
	{
		Tracef("MiniGame Packet Error SubHeader %u\n", kMiniGamePacket.bSubheader);
		return false;
	}

	int iPacketSize = int(kMiniGamePacket.wSize) - sizeof(TPacketGCMiniGameCatchKing);

	switch (kMiniGamePacket.bSubheader)
	{
	case SUBHEADER_GC_CATCH_KING_EVENT_INFO:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCatchKingEventInfo kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCatchKingEventInfo) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCCatchKingEventInfo), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEvent", Py_BuildValue("(b)", kSubPacket.bIsEnable));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBC1", kMiniGamePacket.bSubheader);
		break;
	}

	case SUBHEADER_GC_CATCH_KING_START:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCatchKingStart kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCatchKingStart) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCCatchKingStart), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEventStart", Py_BuildValue("(i)", kSubPacket.dwBigScore));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBC2", kMiniGamePacket.bSubheader);
		break;
	}

	case SUBHEADER_GC_CATCH_KING_SET_CARD:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCatchKingSetCard kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCatchKingSetCard) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCCatchKingSetCard), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetHandCard", Py_BuildValue("(i)", kSubPacket.bCardInHand));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBC3", kMiniGamePacket.bSubheader);
		break;
	}

	case SUBHEADER_GC_CATCH_KING_RESULT_FIELD:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCatchKingResult kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCatchKingResult) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCCatchKingResult), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingResultField", Py_BuildValue("(iiiibbbb)",
				kSubPacket.dwPoints, kSubPacket.bRowType, kSubPacket.bCardPos, kSubPacket.bCardValue,
				kSubPacket.bKeepFieldCard, kSubPacket.bDestroyHandCard, kSubPacket.bGetReward, kSubPacket.bIsFiveNearBy));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBC4", kMiniGamePacket.bSubheader);
		break;
	}

	case SUBHEADER_GC_CATCH_KING_SET_END_CARD:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCatchKingSetEndCard kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCatchKingSetEndCard) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCCatchKingSetEndCard), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetEndCard", Py_BuildValue("(ii)", kSubPacket.bCardPos, kSubPacket.bCardValue));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBC5", kMiniGamePacket.bSubheader);
		break;
	}

	case SUBHEADER_GC_CATCH_KING_REWARD:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCCatchKingReward kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCCatchKingReward) == 0 && "MINI_GAME_RUMI_CARD_REWARD");
			if (!Recv(sizeof(TSubPacketGCCatchKingReward), &kSubPacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingReward", Py_BuildValue("(i)", kSubPacket.bReturnCode));
		}
		else
			TraceError(" RecvMiniGamePacket Error 0x040%uBEBC6", kMiniGamePacket.bSubheader);
		break;
	}
	}

	return true;
}
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
bool CPythonNetworkStream::RecvStoneEvent()
{
	TPacketGCStoneEvent p;
	if (!Recv(sizeof(TPacketGCStoneEvent), &p))
	{
		Tracenf("Recv Stone Event Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_STONE_EVENT", Py_BuildValue("(i)", p.stone_point));
	return true;
}
#endif
#ifdef ENABLE_FISH_EVENT
bool CPythonNetworkStream::SendFishBoxUse(uint8_t bWindow, uint16_t wCell)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT_SEND;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_BOX_USE;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishEventPacket Error\n");
		return false;
	}

	if (!Send(sizeof(bWindow), &bWindow))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}

	if (!Send(sizeof(wCell), &wCell))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendFishShapeAdd(uint8_t bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT_SEND;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_SHAPE_ADD;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishEventPacket Error\n");
		return false;
	}

	if (!Send(sizeof(uint8_t), &bPos))
	{
		Tracef("SendFishShapeAdd Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvFishEventInfo()
{
	TPacketGCFishEventInfo fishEventPacket;

	if (!Recv(sizeof(fishEventPacket), &fishEventPacket))
		return false;

	switch (fishEventPacket.bSubheader)
	{
	case FISH_EVENT_SUBHEADER_BOX_USE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishUse", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
		break;

	case FISH_EVENT_SUBHEADER_SHAPE_ADD:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishAdd", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
		break;

	case FISH_EVENT_SUBHEADER_GC_REWARD:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishReward", Py_BuildValue("(i)", fishEventPacket.dwFirstArg));
		break;

	case FISH_EVENT_SUBHEADER_GC_ENABLE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishEvent", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
		break;
	};

	return true;
}
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
bool CPythonNetworkStream::RecvSoulRoulette()
{
	TPacketGCSoulRoulette Packet;
	if (!Recv(sizeof(Packet), &Packet))
	{
		Tracen("RecvSoulRoulette Error");
		return false;
	}

	enum { OPEN, CLOSE, TURN };
	switch (Packet.option)
	{
	case OPEN:
		for (int i = 0; i < ROULETTE_ITEM_MAX; i++)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_ICON", Py_BuildValue("(iii)", i, Packet.ItemInfo[i].vnum, Packet.ItemInfo[i].count));
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_OPEN", Py_BuildValue("(ii)", Packet.yang, Packet.soul));
		break;
	case CLOSE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_CLOSE", Py_BuildValue("()"));
		break;
	case TURN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_TURN", Py_BuildValue("(ii)", Packet.ItemInfo[0].vnum, Packet.ItemInfo[0].count));
		break;
	}

	return true;
}

bool CPythonNetworkStream::SoulRoulette(const uint8_t option)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGSoulRoulette p{};
	p.header = HEADER_CG_SOUL_ROULETTE;
	p.option = option;

	if (!Send(sizeof(p), &p))
	{
		Tracen("Error SoulRoulette");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_MINI_GAME_BNW
bool CPythonNetworkStream::SendMiniGameBNW(uint8_t bSubHeader, uint8_t bSubArgument)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameBNW packet;
	packet.bHeader = HEADER_CG_MINI_GAME_BNW;
	packet.bSubheader = bSubHeader;
	packet.bSubArgument = bSubArgument;

	if (!Send(sizeof(TPacketCGMiniGameBNW), &packet))
	{
		Tracef("SendMiniGameBNW Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameBNWPacket()
{
	TPacketGCMiniGameBNW packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);

	switch (packet.bSubheader)
	{
	case SUBHEADER_GC_BNW_START:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameBNWStart", Py_BuildValue("()"));
	}
	break;

	case SUBHEADER_GC_BNW_COMPARE:
	{
		if (packet.wSize > 0)
		{
			TPacketGCMiniGameBNWCompare packSecond;
			if (!Recv(sizeof(packSecond), &packSecond))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameBNWCompare", Py_BuildValue("(iiii)",
				packSecond.cWinIndex, packSecond.bClickedIndex, packSecond.bOpponentIndex, packSecond.bMyScore));
		}
	}
	break;

	case SUBHEADER_GC_BNW_RANKING:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameBNWClearRanking", Py_BuildValue("()"));

		while (packet.wSize > 0)
		{
			TBNWRankPlayer pRankP;
			if (!Recv(sizeof(pRankP), &pRankP))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameBNWAddRanking", Py_BuildValue("(iisi)",
				pRankP.bPos, pRankP.bIsMain, pRankP.szName, pRankP.wDonationsCount));

			packet.wSize -= sizeof(pRankP);
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameBNWOpenRanking", Py_BuildValue("()"));
	}
	break;

	case SUBHEADER_GC_BNW_UPDATE_RANKING:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameBNWUpdateRanking", Py_BuildValue("()"));
	}
	break;

	default:
		TraceError("CPythonNetworkStream::RecvMiniGameBNWPacket: Unknown subheader\n");
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
bool CPythonNetworkStream::SendMiniGameFindM(uint8_t bSubHeader, uint8_t bSubArgument)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameFindM packet;
	packet.bHeader = HEADER_CG_MINI_GAME_FIND_M;
	packet.bSubheader = bSubHeader;
	packet.bSubArgument = bSubArgument;

	if (!Send(sizeof(TPacketCGMiniGameFindM), &packet))
	{
		Tracef("SendMiniGameFindM Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameFindMPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCMiniGameFindM packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	int iSize = packet.wSize - sizeof(packet);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet.bSubheader)
	{
	case SUBHEADER_GC_FIND_M_START:
	{
		TPacketGCMiniGameFindMStart* packSecond = (TPacketGCMiniGameFindMStart*)&vecBuffer[0];

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMEventStart", Py_BuildValue("(ii)",
			packSecond->dwStartTime, packSecond->dwNextReveal));

#ifdef ENABLE_MINI_GAME_FINDM_HINT
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMHintCount", Py_BuildValue("(i)", packSecond->bHintCount));
#endif
	}
	break;

	case SUBHEADER_GC_FIND_M_RESULT_CLICK:
	{
		TPacketGCMiniGameFindMResult* packSecond = (TPacketGCMiniGameFindMResult*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMClickCard", Py_BuildValue("(iiiii)",
			packSecond->bCardIndex, packSecond->wMonsterVnum, packSecond->bHideRevealed, packSecond->dwNextReveal, packSecond->wTryCount));
	}
	break;

	case SUBHEADER_GC_FIND_M_REWARD:
	{
		uint32_t dwRewardVnum = *(uint32_t*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMReward", Py_BuildValue("(i)", dwRewardVnum));
	}
	break;

	case SUBHEADER_GC_FIND_M_UPGRADE:
	{
		TPacketGCMiniGameFindMUpgrade* packSecond = (TPacketGCMiniGameFindMUpgrade*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMUpgrade", Py_BuildValue("(ii)",
			packSecond->dwRewardVnum, packSecond->dwNeedMoney));
	}
	break;

	case SUBHEADER_GC_FIND_M_END:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMGameEnd", Py_BuildValue("()"));
	}
	break;

#ifdef ENABLE_MINI_GAME_FINDM_HINT
	case SUBHEADER_GC_FIND_M_HINT:
	{
		TPacketGCMiniGameFindMHint* packSecond = (TPacketGCMiniGameFindMHint*)&vecBuffer[0];

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMHintCount", Py_BuildValue("(ii)",
			packSecond->bHintCount, packSecond->dwNextReveal));

		for (int i = 0; i < 27; i++)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFindMHint", Py_BuildValue("(ii)",
				i, packSecond->wMonsterVnum[i]));
		}
	}
	break;
#endif	

	default:
		TraceError("CPythonNetworkStream::RecvMiniGameFindMPacket: Unknown subheader\n");
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
bool CPythonNetworkStream::SendMiniGameYutNori(uint8_t bSubHeader, uint8_t bSubArgument)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMiniGameYutNori packet;
	packet.bHeader = HEADER_CG_MINI_GAME_YUT_NORI;
	packet.bSubheader = bSubHeader;
	packet.bSubArgument = bSubArgument;

	if (!Send(sizeof(TPacketCGMiniGameYutNori), &packet))
	{
		Tracef("SendMiniGameYutNori Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameYutNoriPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCMiniGameYutNori packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	int iSize = packet.wSize - sizeof(packet);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet.bSubheader)
	{
	case SUBHEADER_GC_START_GAME:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,())", 0));
	}
	break;

	case SUBHEADER_GC_END_GAME:
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,())", 1));
	}
	break;

	case SUBHEADER_GC_THROW_YUT:
	{
		TPacketGCMiniGameYutNoriThrow* packSecond = (TPacketGCMiniGameYutNoriThrow*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(bi))", 2, packSecond->bIsPc, packSecond->cYut));
	}
	break;

	case SUBHEADER_GC_NEXT_TURN:
	{
		TPacketGCMiniGameYutNoriNextTurn* packSecond = (TPacketGCMiniGameYutNoriNextTurn*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(bi))", 3, packSecond->bPcTurn, packSecond->bState));
	}
	break;

	case SUBHEADER_GC_YUT_MOVE:
	{
		TPacketGCMiniGameYutNoriMove* packSecond = (TPacketGCMiniGameYutNoriMove*)&vecBuffer[0];

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(bibii))",
			5, packSecond->bIsPc, packSecond->bUnitIndex, packSecond->bIsCatch, packSecond->bStartIndex, packSecond->bDestIndex));
	}
	break;

	case SUBHEADER_GC_SHOW_AVAILABLE_AREA:
	{
		TPacketGCMiniGameYutNoriAvailableArea* packSecond = (TPacketGCMiniGameYutNoriAvailableArea*)&vecBuffer[0];

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(ii))",
			6, packSecond->bPlayerIndex, packSecond->bAvailableIndex));
	}
	break;

	case SUBHEADER_GC_PUSH_CATCH_YUT:
	{
		TPacketGCMiniGameYutNoriPushCatchYut* packSecond = (TPacketGCMiniGameYutNoriPushCatchYut*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(ii))", 7, packSecond->bIsPc, packSecond->bUnitIndex));
	}
	break;

	case SUBHEADER_GC_SET_PROB:
	{
		uint8_t bProbIndex = *(uint8_t*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(ii)", 8, bProbIndex));
	}
	break;

	case SUBHEADER_GC_SET_SCORE:
	{
		uint16_t wScore = *(uint16_t*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(ii)", 9, wScore));
	}
	break;

	case SUBHEADER_GC_SET_REMAIN_COUNT:
	{
		uint8_t bRemainCount = *(uint8_t*)&vecBuffer[0];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(ii)", 10, bRemainCount));
	}
	break;

	default:
		TraceError("CPythonNetworkStream::RecvMiniGameYutNoriPacket: Unknown subheader - %d\n", packet.bSubheader);
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameYutNoriSetYut()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,())", 4));
	return true;
}
#endif

#ifdef ENABLE_FISHING_RENEWAL
bool CPythonNetworkStream::SendFishingPacketNew(int r, int i)
{
	TPacketFishingNew p;
	p.header = HEADER_CG_FISHING_NEW;
	switch (i) {
	case 0:
	{
		p.subheader = FISHING_SUBHEADER_NEW_STOP;
	}
	break;
	case 1:
	{
		p.subheader = FISHING_SUBHEADER_NEW_START;
	}
	break;
	case 2:
	{
		p.subheader = FISHING_SUBHEADER_NEW_CATCH;
	}
	break;
	case 3:
	{
		p.subheader = FISHING_SUBHEADER_NEW_CATCH_FAILED;
	}
	break;
	default:
		return false;
	}
	p.vid = 0;
	p.dir = r / 5;
	p.need = 0;
	p.count = 0;

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvFishingNew()
{
	TPacketFishingNew p;
	if (!Recv(sizeof(p), &p))
		return false;

	CInstanceBase* pFishingInstance = CPythonCharacterManager::Instance().GetInstancePtr(p.vid);
	if (!pFishingInstance) {
		return true;
	}

	switch (p.subheader)
	{
	case FISHING_SUBHEADER_NEW_START:
	{
		if (pFishingInstance->IsFishing()) {
			if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
				SendFishingPacketNew(0, 0);
			}
			return true;
		}
		else {
			pFishingInstance->StartFishing(float(p.dir) * 5.0f);
			if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingStart", Py_BuildValue("(ii)", p.count, p.need));
			}
		}
	}
	break;
	case FISHING_SUBHEADER_NEW_STOP:
	{
		if (pFishingInstance->IsFishing()) {
			pFishingInstance->StopFishing();
		}

		if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingStop", Py_BuildValue("()"));
		}
	}
	break;
	case FISHING_SUBHEADER_NEW_CATCH:
	{
		if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingCatch", Py_BuildValue("(i)", p.count));
		}
	}
	break;
	case FISHING_SUBHEADER_NEW_CATCH_FAILED:
	{
		if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingCatchFailed", Py_BuildValue("()"));
		}
	}
	break;
	case FISHING_SUBHEADER_NEW_CATCH_FAIL:
	case FISHING_SUBHEADER_NEW_CATCH_SUCCESS:
	{
		if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr()) {
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingStop", Py_BuildValue("()"));
		}

		pFishingInstance->SetFishEmoticon();
		if (p.subheader == FISHING_SUBHEADER_NEW_CATCH_SUCCESS) {
			pFishingInstance->CatchSuccess();
		}
		else {
			pFishingInstance->CatchFail();
		}
	}
	break;
	}

	return true;
}
#endif
// END MINI GAME EVENTS //
