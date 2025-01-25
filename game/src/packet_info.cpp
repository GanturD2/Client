#include "stdafx.h"
#include "../../common/stl.h"
#include "constants.h"
#include "packet_info.h"
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "DungeonInfo.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

CPacketInfo::CPacketInfo()
	: m_pCurrentPacket(nullptr), m_dwStartTime(0)
{
}

CPacketInfo::~CPacketInfo()
{
	itertype(m_pPacketMap) it = m_pPacketMap.begin();
	for ( ; it != m_pPacketMap.end(); ++it) {
		M2_DELETE(it->second);
	}
}

void CPacketInfo::Set(int header, int iSize, const char * c_pszName, bool bSeq)
{
	if (m_pPacketMap.find(header) != m_pPacketMap.end())
		return;

	TPacketElement * element = M2_NEW TPacketElement;

	element->iSize = iSize;
	element->stName.assign(c_pszName);
	element->iCalled = 0;
	element->dwLoad = 0;

#ifdef ENABLE_SEQUENCE_SYSTEM
	element->bSequencePacket = bSeq;

	if (element->bSequencePacket)
		element->iSize += sizeof(uint8_t);
#endif

	m_pPacketMap.insert(std::map<int, TPacketElement *>::value_type(header, element));
}

bool CPacketInfo::Get(int header, int * size, const char ** c_ppszName)
{
	std::map<int, TPacketElement *>::iterator it = m_pPacketMap.find(header);

	if (it == m_pPacketMap.end())
		return false;

	*size = it->second->iSize;
	*c_ppszName = it->second->stName.c_str();

	m_pCurrentPacket = it->second;
	return true;
}

#ifdef ENABLE_SEQUENCE_SYSTEM
bool CPacketInfo::IsSequence(int header)
{
	TPacketElement * pkElement = GetElement(header);
	return pkElement ? pkElement->bSequencePacket : false;
}

void CPacketInfo::SetSequence(int header, bool bSeq)
{
	TPacketElement * pkElem = GetElement(header);

	if (pkElem)
	{
		if (bSeq)
		{
			if (!pkElem->bSequencePacket)
				pkElem->iSize++;
		}
		else
		{
			if (pkElem->bSequencePacket)
				pkElem->iSize--;
		}

		pkElem->bSequencePacket = bSeq;
	}
}
#endif

TPacketElement * CPacketInfo::GetElement(int header)
{
	std::map<int, TPacketElement *>::iterator it = m_pPacketMap.find(header);

	if (it == m_pPacketMap.end())
		return nullptr;

	return it->second;
}

void CPacketInfo::Start()
{
	assert(m_pCurrentPacket != nullptr);
	m_dwStartTime = get_dword_time();
}

void CPacketInfo::End()
{
	++m_pCurrentPacket->iCalled;
	m_pCurrentPacket->dwLoad += get_dword_time() - m_dwStartTime;
}

void CPacketInfo::Log(const char * c_pszFileName)
{
	FILE * fp;

	fp = fopen(c_pszFileName, "w");

	if (!fp)
		return;

	std::map<int, TPacketElement *>::iterator it = m_pPacketMap.begin();

	fprintf(fp, "Name             Called     Load       Ratio\n");

	while (it != m_pPacketMap.end())
	{
		const TPacketElement * p = it->second;
		++it;

		fprintf(fp, "%-16s %-10d %-10u %.2f\n",
				p->stName.c_str(),
				p->iCalled,
				p->dwLoad,
				p->iCalled != 0 ? (float) p->dwLoad / p->iCalled : 0.0f);
	}

	fclose(fp);
}
///---------------------------------------------------------

CPacketInfoCG::CPacketInfoCG()
{
	Set(HEADER_CG_HANDSHAKE, sizeof(TPacketCGHandshake), "Handshake", false);
	Set(HEADER_CG_TIME_SYNC, sizeof(TPacketCGHandshake), "TimeSync", true);
	Set(HEADER_CG_MARK_LOGIN, sizeof(TPacketCGMarkLogin), "MarkLogin", false);
	Set(HEADER_CG_MARK_IDXLIST, sizeof(TPacketCGMarkIDXList), "MarkIdxList", false);
	Set(HEADER_CG_MARK_CRCLIST, sizeof(TPacketCGMarkCRCList), "MarkCrcList", false);
	Set(HEADER_CG_MARK_UPLOAD, sizeof(TPacketCGMarkUpload), "MarkUpload", false);
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	Set(HEADER_CG_KEY_AGREEMENT, sizeof(TPacketKeyAgreement), "KeyAgreement", false);
#endif

	Set(HEADER_CG_GUILD_SYMBOL_UPLOAD, sizeof(TPacketCGGuildSymbolUpload), "SymbolUpload", false);
	Set(HEADER_CG_GUILD_SYMBOL_CRC, sizeof(TPacketCGSymbolCRC), "SymbolCRC", false);
	Set(HEADER_CG_LOGIN, sizeof(TPacketCGLogin), "Login", true);
	Set(HEADER_CG_LOGIN2, sizeof(TPacketCGLogin2), "Login2", true);
	Set(HEADER_CG_LOGIN3, sizeof(TPacketCGLogin3), "Login3", true);
	Set(HEADER_CG_ATTACK, sizeof(TPacketCGAttack), "Attack", true);
	Set(HEADER_CG_CHAT, sizeof(TPacketCGChat), "Chat", true);
	Set(HEADER_CG_WHISPER, sizeof(TPacketCGWhisper), "Whisper", true);

	Set(HEADER_CG_CHARACTER_SELECT, sizeof(TPacketCGPlayerSelect), "Select", true);
	Set(HEADER_CG_CHARACTER_CREATE, sizeof(TPacketCGPlayerCreate), "Create", true);
	Set(HEADER_CG_CHARACTER_DELETE, sizeof(TPacketCGPlayerDelete), "Delete", true);
	Set(HEADER_CG_ENTERGAME, sizeof(TPacketCGEnterGame), "EnterGame", true);

	Set(HEADER_CG_ITEM_USE, sizeof(TPacketCGItemUse), "ItemUse", true);
	Set(HEADER_CG_ITEM_DROP, sizeof(TPacketCGItemDrop), "ItemDrop", true);
	Set(HEADER_CG_ITEM_DROP2, sizeof(TPacketCGItemDrop2), "ItemDrop2", true);
#ifdef ENABLE_DESTROY_SYSTEM
	Set(HEADER_CG_ITEM_DESTROY, sizeof(TPacketCGItemDestroy), "ItemDestroy");
#endif
	Set(HEADER_CG_ITEM_MOVE, sizeof(TPacketCGItemMove), "ItemMove", true);
	Set(HEADER_CG_ITEM_PICKUP, sizeof(TPacketCGItemPickup), "ItemPickup", true);

	Set(HEADER_CG_QUICKSLOT_ADD, sizeof(TPacketCGQuickslotAdd), "QuickslotAdd", true);
	Set(HEADER_CG_QUICKSLOT_DEL, sizeof(TPacketCGQuickslotDel), "QuickslotDel", true);
	Set(HEADER_CG_QUICKSLOT_SWAP, sizeof(TPacketCGQuickslotSwap), "QuickslotSwap", true);

	Set(HEADER_CG_SHOP, sizeof(TPacketCGShop), "Shop", true);

	Set(HEADER_CG_ON_CLICK, sizeof(TPacketCGOnClick), "OnClick", true);
	Set(HEADER_CG_EXCHANGE, sizeof(TPacketCGExchange), "Exchange", true);
	Set(HEADER_CG_CHARACTER_POSITION, sizeof(TPacketCGPosition), "Position", true);
	Set(HEADER_CG_SCRIPT_ANSWER, sizeof(TPacketCGScriptAnswer), "ScriptAnswer", true);
	Set(HEADER_CG_SCRIPT_BUTTON, sizeof(TPacketCGScriptButton), "ScriptButton", true);
	Set(HEADER_CG_QUEST_INPUT_STRING, sizeof(TPacketCGQuestInputString), "QuestInputString", true);
#ifdef ENABLE_OX_RENEWAL
	Set(HEADER_CG_QUEST_INPUT_LONG_STRING, sizeof(TPacketCGQuestInputLongString), "QuestInputLongString", false);
#endif
	Set(HEADER_CG_QUEST_CONFIRM, sizeof(TPacketCGQuestConfirm), "QuestConfirm", true);

	Set(HEADER_CG_MOVE, sizeof(TPacketCGMove), "Move", true);
	Set(HEADER_CG_SYNC_POSITION, sizeof(TPacketCGSyncPosition), "SyncPosition", true);

	Set(HEADER_CG_FLY_TARGETING, sizeof(TPacketCGFlyTargeting), "FlyTarget", true);
	Set(HEADER_CG_ADD_FLY_TARGETING, sizeof(TPacketCGFlyTargeting), "AddFlyTarget", true);
	Set(HEADER_CG_SHOOT, sizeof(TPacketCGShoot), "Shoot", true);

	Set(HEADER_CG_USE_SKILL, sizeof(TPacketCGUseSkill), "UseSkill", true);
#ifdef ENABLE_CUBE_RENEWAL
	Set(HEADER_CG_CUBE_RENEWAL, sizeof(TPacketCGCubeRenewal), "CubeRenewalSend", true);
#endif
	Set(HEADER_CG_ITEM_USE_TO_ITEM, sizeof(TPacketCGItemUseToItem), "UseItemToItem", true);
	Set(HEADER_CG_TARGET, sizeof(TPacketCGTarget), "Target", true);
	Set(HEADER_CG_WARP, sizeof(TPacketCGWarp), "Warp", true);
	Set(HEADER_CG_MESSENGER, sizeof(TPacketCGMessenger), "Messenger", true);

	Set(HEADER_CG_PARTY_REMOVE, sizeof(TPacketCGPartyRemove), "PartyRemove", true);
	Set(HEADER_CG_PARTY_INVITE, sizeof(TPacketCGPartyInvite), "PartyInvite", true);
	Set(HEADER_CG_PARTY_INVITE_ANSWER, sizeof(TPacketCGPartyInviteAnswer), "PartyInviteAnswer", true);
	Set(HEADER_CG_PARTY_SET_STATE, sizeof(TPacketCGPartySetState), "PartySetState", true);
	Set(HEADER_CG_PARTY_USE_SKILL, sizeof(TPacketCGPartyUseSkill), "PartyUseSkill", true);
	Set(HEADER_CG_PARTY_PARAMETER, sizeof(TPacketCGPartyParameter), "PartyParameter", true);

#ifdef ENABLE_SAFEBOX_MONEY
	Set(HEADER_CG_SAFEBOX_MONEY, sizeof(TPacketCGSafeboxMoney), "SafeboxMoney", true);
#endif

	Set(HEADER_CG_EMPIRE, sizeof(TPacketCGEmpire), "Empire", true);
	Set(HEADER_CG_SAFEBOX_CHECKOUT, sizeof(TPacketCGSafeboxCheckout), "SafeboxCheckout", true);
	Set(HEADER_CG_SAFEBOX_CHECKIN, sizeof(TPacketCGSafeboxCheckin), "SafeboxCheckin", true);

	Set(HEADER_CG_SAFEBOX_ITEM_MOVE, sizeof(TPacketCGItemMove), "SafeboxItemMove", true);

#ifdef ENABLE_INGAME_WIKI
	Set(HEADER_CG_WIKI, sizeof(TCGWikiPacket), "Wikipedia", true);
#endif

	Set(HEADER_CG_GUILD, sizeof(TPacketCGGuild), "Guild", true);
	Set(HEADER_CG_ANSWER_MAKE_GUILD, sizeof(TPacketCGAnswerMakeGuild), "AnswerMakeGuild", true);

	Set(HEADER_CG_FISHING, sizeof(TPacketCGFishing), "Fishing", true);
	Set(HEADER_CG_ITEM_GIVE, sizeof(TPacketCGGiveItem), "ItemGive", true);
	Set(HEADER_CG_HACK, sizeof(TPacketCGHack), "Hack", true);
	Set(HEADER_CG_MYSHOP, sizeof(TPacketCGMyShop), "MyShop", true);
	Set(HEADER_CG_REFINE, sizeof(TPacketCGRefine), "Refine", true);
	Set(HEADER_CG_CHANGE_NAME, sizeof(TPacketCGChangeName), "ChangeName", true);
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	Set(HEADER_CG_DUNGEON_INFO, sizeof(TPacketCGDungeonInfo), "DungeonInfo", true);
#endif

	Set(HEADER_CG_CLIENT_VERSION, sizeof(TPacketCGClientVersion), "Version", true);
	Set(HEADER_CG_CLIENT_VERSION2, sizeof(TPacketCGClientVersion2), "Version", true);
	Set(HEADER_CG_PONG, sizeof(TPacketCGPong), "Pong", true);
	Set(HEADER_CG_MALL_CHECKOUT, sizeof(TPacketCGSafeboxCheckout), "MallCheckout", true);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	Set(HEADER_CG_GUILDSTORAGE_CHECKIN, sizeof(TPacketCGSafeboxCheckin), "GuildstorageCheckin", true);
	Set(HEADER_CG_GUILDSTORAGE_CHECKOUT, sizeof(TPacketCGSafeboxCheckout), "GuildstorageCheckout", true);
#endif

	Set(HEADER_CG_SCRIPT_SELECT_ITEM, sizeof(TPacketCGScriptSelectItem), "ScriptSelectItem", true);
	Set(HEADER_CG_DRAGON_SOUL_REFINE, sizeof(TPacketCGDragonSoulRefine), "DragonSoulRefine", false);
#ifdef ENABLE_SWITCHBOT
	Set(HEADER_CG_SWITCHBOT, sizeof(TPacketGCSwitchbot), "Switchbot", true);
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	Set(HEADER_CG_SKILL_COLOR, sizeof(TPacketCGSkillColor), "ChangeSkillColor", true);
#endif
	Set(HEADER_CG_STATE_CHECKER, sizeof(uint8_t), "ServerStateCheck", false);
#ifdef ENABLE_HWID_BAN
	Set(HEADER_CG_HWID_SYSTEM, sizeof(TPacketCGHwidBan), "HwidManager", true);
#endif
#ifdef ENABLE_SEND_TARGET_INFO
	Set(HEADER_CG_TARGET_INFO_LOAD, sizeof(TPacketCGTargetInfoLoad), "TargetInfoLoad", true);
#endif
#ifdef ENABLE_BATTLE_FIELD
	Set(HEADER_CG_REQUEST_ENTER_BATTLE, sizeof(uint8_t), "RequestEnterBattle", true);
	Set(HEADER_CG_REQUEST_EXIT_BATTLE, sizeof(uint8_t), "RequestExitBattle", true);
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	Set(HEADER_CG_ATTR_6TH_7TH, sizeof(TPacketCGAttr67Send), "Attr67Send", true);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	// Myshop
	Set(HEADER_CG_MYSHOP_REMOVE_ITEM, sizeof(TPacketMyShopRemoveItem), "MyShopRemove", false);
	Set(HEADER_CG_MYSHOP_ADD_ITEM, sizeof(TPacketMyShopAddItem), "MyShopAdd", false);
	Set(HEADER_CG_MYSHOP_CLOSE, sizeof(uint8_t), "MyShopClose", false); // Header-only
	Set(HEADER_CG_MYSHOP_OPEN, sizeof(uint8_t), "MyShopOpen", false); // Header-only
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	Set(HEADER_CG_MYSHOP_REOPEN, sizeof(uint8_t), "MyShopReopen", false); // Header-only
#	endif
	Set(HEADER_CG_MYSHOP_WITHDRAW, sizeof(TPacketCGShopWithdraw), "MyShopWithdraw", false); // Header-only
	Set(HEADER_CG_MYSHOP_RENAME, sizeof(TPacketGCShopRename), "MyShopRename", false);
#ifdef ENABLE_MYSHOP_DECO
	Set(HEADER_CG_MYSHOP_DECO_STATE, sizeof(TPacketCGMyShopDecoState), "MyShopDecoState", true);
	Set(HEADER_CG_MYSHOP_DECO_ADD, sizeof(TPacketCGMyShopDecoAdd), "MyShopDecoAdd", true);
#endif
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	Set(HEADER_CG_PRIVATE_SHOP_SEARCH_SEARCH, sizeof(TPacketCGPrivateShopSearch), "PrivateShopSearchSearch", true);
	Set(HEADER_CG_PRIVATE_SHOP_SEARCH_BUY, sizeof(TPacketCGPrivateShopSearchBuy), "PrivateShopSearchBuy", true);
	Set(HEADER_CG_PRIVATE_SHOP_SEARCH_CLOSE, sizeof(TPacketCGPrivateShopSearchClose), "PrivateShopSearchClose", true);
#endif
#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
	Set(HEADER_CG_WON_EXCHANGE, sizeof(TPacketCGWonExchange), "WonExchange", true);
#endif
#ifdef ENABLE_REFINE_ELEMENT
	Set(HEADER_CG_ELEMENTS_SPELL, sizeof(TPacketCGElementsSpell), "ElementsSpell", true);
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	Set(HEADER_CG_EXTEND_INVEN_REQUEST, sizeof(TPacketCGSendExtendInvenRequest), "ExtendInvenRequest", true);
	Set(HEADER_CG_EXTEND_INVEN_UPGRADE, sizeof(TPacketCGSendExtendInvenUpgrade), "ExtendInvenUpgrade", true);
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
	Set(HEADER_CG_COSTUME_REFINE_REQUEST, sizeof(TPacketCGSendCostumeRefine), "CostumeRefinePacket", true);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	Set(HEADER_CG_ACCE_CLOSE_REQUEST, sizeof(TPacketCGSendAcceClose), "AcceCloseRequest", true);
	Set(HEADER_CG_ACCE_REFINE_REQUEST, sizeof(TPacketCGSendAcceRefine), "AcceRefineRequest", true);
#endif
#ifdef ENABLE_AURA_SYSTEM
	Set(HEADER_CG_AURA, sizeof(TPacketCGAura), "Aura", true);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	Set(HEADER_CG_CHANGE_LOOK, sizeof(TPacketCGTransmutation), "Transmutation", true);
#endif
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	Set(HEADER_CG_GEM_SHOP, sizeof(TPacketCGGemShop), "GemShop", true);
	Set(HEADER_CG_SCRIPT_SELECT_ITEM_EX, sizeof(TPacketCGScriptSelectItemEx), "ScriptSelectItemEx", true);
#endif
#ifdef ENABLE_CHANGED_ATTR
	Set(HEADER_CG_ITEM_SELECT_ATTR, sizeof(TPacketCGItemSelectAttr), "ItemSelectAttr", true);
#endif
#ifdef ENABLE_TICKET_SYSTEM
	Set(HEADER_CG_TICKET_SYSTEM, sizeof(TPacketCGTicketSystem), "TicketSystem", true);
#endif
#ifdef ENABLE_MAILBOX
	Set(HEADER_CG_MAILBOX_WRITE, sizeof(TPacketCGMailboxWrite), "MailboxWrite", true);
	Set(HEADER_CG_MAILBOX_WRITE_CONFIRM, sizeof(TPacketCGMailboxWriteConfirm), "MailboxConfirm", true);
	Set(HEADER_CG_MAILBOX_PROCESS, sizeof(TPacketMailboxProcess), "MailboxProcess", true);
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	Set(HEADER_CG_LANGUAGE_CHANGE, sizeof(TPacketCGLanguageChange), "LanguageChange", true);
	Set(HEADER_CG_TARGET_LANGUAGE_REQUEST, sizeof(TPacketCGTargetLanguageRequest), "TargetLanguageRequest", true);
#endif
#ifdef ENABLE_MOVE_CHANNEL
	Set(HEADER_CG_MOVE_CHANNEL, sizeof(TPacketCGMoveChannel), "MoveChannel", true);
#endif
#ifdef ENABLE_PARTY_MATCH
	Set(HEADER_CG_PARTY_MATCH, sizeof(TPacketCGPartyMatch), "PartyMatch", true);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	Set(HEADER_CG_EXT_BATTLE_PASS_ACTION, sizeof(TPacketCGExtBattlePassAction), "ReciveExtBattlePassActions", true);
	Set(HEADER_CG_EXT_SEND_BP_PREMIUM, sizeof(TPacketCGExtBattlePassSendPremium), "ReciveExtBattlePassPremium", true);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	Set(HEADER_CG_GROWTH_PET, sizeof(TPacketCGGrowthPet), "GrowthPet", true);
	Set(HEADER_CG_GROWTH_PET_HATCHING, sizeof(TPacketCGGrowthPetHatching), "GrowthPetHatching", true);
	Set(HEADER_CG_GROWTH_PET_LEARN_SKILL, sizeof(TPacketCGGrowthPetLearnSkill), "GrowthPetLearnSkill", true);
	Set(HEADER_CG_GROWTH_PET_SKILL_UPGRADE, sizeof(TPacketCGGrowthPetSkillUpgrade), "GrowthPetSkillUpgrade", true);
	Set(HEADER_CG_GROWTH_PET_SKILL_UPGRADE_REQUEST, sizeof(TPacketCGGrowthPetSkillUpgradeRequest), "GrowthPetSkillUpgradeRequest", true);
	Set(HEADER_CG_GROWTH_PET_FEED_REQUEST, sizeof(TPacketCGGrowthPetFeedRequest), "GrowthPetFeedRequest", true);
	Set(HEADER_CG_GROWTH_PET_DELETE_SKILL, sizeof(TPacketCGGrowthPetDeleteSkill), "GrowthPetDeleteSkill", true);
	Set(HEADER_CG_GROWTH_PET_ALL_DEL_SKILL, sizeof(TPacketCGGrowthPetDeleteAllSkill), "GrowthPetDeleteAllSkill", true);
	Set(HEADER_CG_GROWTH_PET_NAME_CHANGE, sizeof(TPacketCGGrowthPetNameChange), "GrowthPetNameChange", true);
#	ifdef ENABLE_PET_ATTR_DETERMINE
	Set(HEADER_CG_GROWTH_PET_ATTR_DETERMINE, sizeof(TPacketCGGrowthPetAttrDetermine), "GrowthPetAttrDetermine", true);
	Set(HEADER_CG_GROWTH_PET_ATTR_CHANGE, sizeof(TPacketCGGrowthPetAttrChange), "GrowthPetAttrChange", true);
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	Set(HEADER_CG_GROWTH_PET_REVIVE_REQUEST, sizeof(TPacketCGGrowthPetReviveRequest), "GrowthPetReviveRequest", true);
#	endif
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	Set(HEADER_CG_ACHIEVEMENT, sizeof(achievements::CG_packet), "ProcessClientPackets");
#endif

#ifdef ENABLE_EVENT_MANAGER
	Set(HEADER_CG_REQUEST_EVENT_QUEST, sizeof(TPacketCGRequestEventQuest), "RequestEventQuest", true);
	Set(HEADER_CG_REQUEST_EVENT_DATA, sizeof(TPacketCGRequestEventData), "EventRequest", true);
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	Set(HEADER_CG_OKEY_CARD, sizeof(TPacketCGMiniGameOkeyCard), "MiniGameOkeyCard", true);
#endif
#ifdef ENABLE_MONSTER_BACK
	Set(HEADER_CG_ATTENDANCE_REWARD, sizeof(uint8_t), "RequestAttendanceReward", true);
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	Set(HEADER_CG_MINI_GAME_CATCH_KING, sizeof(TPacketCGMiniGameCatchKing), "MiniGameCatchKing", true);
#endif
#ifdef ENABLE_FISH_EVENT
	Set(HEADER_CG_FISH_EVENT_SEND, sizeof(TPacketCGFishEvent), "FishEvent", true);
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	Set(HEADER_CG_SOUL_ROULETTE, sizeof(TPacketCGSoulRoulette), "SoulRoulette", true);
#endif
#ifdef ENABLE_MINI_GAME_BNW
	Set(HEADER_CG_MINI_GAME_BNW, sizeof(TPacketCGMiniGameBNW), "MiniGameBNW");
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	Set(HEADER_CG_MINI_GAME_FIND_M, sizeof(TPacketCGMiniGameFindM), "MiniGameFindM");
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	Set(HEADER_CG_MINI_GAME_YUT_NORI, sizeof(TPacketCGMiniGameYutNori), "MiniGameYutNori");
#endif
#ifdef ENABLE_FLOWER_EVENT
	Set(HEADER_CG_FLOWER_EVENT_EXCHANGE, sizeof(TPacketCGFlowerEventSend), "FlowerEventSend", true);
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	Set(HEADER_CG_BIOLOG_MANAGER, sizeof(TPacketCGBiologManagerAction), "BiologManager", false);
#endif
#ifdef ENABLE_HUNTING_SYSTEM
	Set(HEADER_CG_SEND_HUNTING_ACTION, sizeof(TPacketGCHuntingAction), "ReciveHuntingAction", true);
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	Set(HEADER_CG_LOTTO_OPENINGS, sizeof(TPacketCGLotteryOpenings), "LottoOpenWindow", true);
	Set(HEADER_CG_LOTTO_BUY_TICKET, sizeof(TPacketCGSendLottoNewTicket), "LottoBuyTicket", true);
	Set(HEADER_CG_LOTTO_TICKET_OPTIONS, sizeof(TPacketCGSendTicketOptions), "LottoTicketOptions", true);
	Set(HEADER_CG_LOTTO_PICK_MONEY, sizeof(TPacketCGSendLottoPickMoney), "LottoPickMoney", true);
#endif
#ifdef ENABLE_FISHING_RENEWAL
	Set(HEADER_CG_FISHING_NEW, sizeof(TPacketFishingNew), "PacketFishingNew", true);
#endif
}

CPacketInfoCG::~CPacketInfoCG()
{
	Log("packet_info.txt");
}

////////////////////////////////////////////////////////////////////////////////
CPacketInfoGG::CPacketInfoGG()
{
	Set(HEADER_GG_SETUP,		sizeof(TPacketGGSetup),		"Setup", false);
	Set(HEADER_GG_LOGIN,		sizeof(TPacketGGLogin),		"Login", false);
	Set(HEADER_GG_LOGOUT,		sizeof(TPacketGGLogout),	"Logout", false);
	Set(HEADER_GG_RELAY,		sizeof(TPacketGGRelay),		"Relay", false);
	Set(HEADER_GG_NOTICE,		sizeof(TPacketGGNotice),	"Notice", false);
#ifdef ENABLE_FULL_NOTICE
	Set(HEADER_GG_BIG_NOTICE,	sizeof(TPacketGGNotice),	"BigNotice", false);
#endif
	Set(HEADER_GG_SHUTDOWN,		sizeof(TPacketGGShutdown),	"Shutdown", false);
	Set(HEADER_GG_GUILD,		sizeof(TPacketGGGuild),		"Guild", false);
	Set(HEADER_GG_SHOUT,		sizeof(TPacketGGShout),		"Shout", false);
	Set(HEADER_GG_DISCONNECT,	    	sizeof(TPacketGGDisconnect),	"Disconnect", false);
	Set(HEADER_GG_MESSENGER_ADD,	sizeof(TPacketGGMessenger),	"MessengerAdd", false);
	Set(HEADER_GG_MESSENGER_REMOVE,	sizeof(TPacketGGMessenger),	"MessengerRemove", false);
#ifdef ENABLE_MESSENGER_BLOCK
	Set(HEADER_GG_MESSENGER_BLOCK_ADD, sizeof(TPacketGGMessenger), "MessengerBlockAdd", false);
	Set(HEADER_GG_MESSENGER_BLOCK_REMOVE, sizeof(TPacketGGMessenger), "MessengerBlockRemove", false);
#endif
	Set(HEADER_GG_FIND_POSITION,	sizeof(TPacketGGFindPosition),	"FindPosition", false);
	Set(HEADER_GG_WARP_CHARACTER,	sizeof(TPacketGGWarpCharacter),	"WarpCharacter", false);
	//Set(HEADER_GG_MESSENGER_MOBILE,	sizeof(TPacketGGMessengerMobile), "MessengerMobile", false);
	Set(HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX, sizeof(TPacketGGGuildWarMapIndex), "GuildWarMapIndex", false);
	Set(HEADER_GG_TRANSFER,		sizeof(TPacketGGTransfer),	"Transfer", false);
	Set(HEADER_GG_XMAS_WARP_SANTA,	sizeof(TPacketGGXmasWarpSanta),	"XmasWarpSanta", false);
	Set(HEADER_GG_XMAS_WARP_SANTA_REPLY, sizeof(TPacketGGXmasWarpSantaReply), "XmasWarpSantaReply", false);
	Set(HEADER_GG_RELOAD_CRC_LIST,	sizeof(uint8_t),			"ReloadCRCList", false);
	Set(HEADER_GG_CHECK_CLIENT_VERSION, sizeof(uint8_t),			"CheckClientVersion", false);
	Set(HEADER_GG_LOGIN_PING,		sizeof(TPacketGGLoginPing),	"LoginPing", false);

	// BLOCK_CHAT
	Set(HEADER_GG_BLOCK_CHAT,		sizeof(TPacketGGBlockChat),	"BlockChat", false);
	// END_OF_BLOCK_CHAT
	Set(HEADER_GG_SIEGE,	sizeof(TPacketGGSiege),	"Siege", false);

	Set(HEADER_GG_MONARCH_NOTICE,		sizeof(TPacketGGMonarchNotice),	"MonarchNotice", false);
	Set(HEADER_GG_MONARCH_TRANSFER,		sizeof(TPacketMonarchGGTransfer),	"MonarchTransfer", false);
	Set(HEADER_GG_CHECK_AWAKENESS,		sizeof(TPacketGGCheckAwakeness),	"CheckAwakeness",		false);
#ifdef ENABLE_SWITCHBOT
	Set(HEADER_GG_SWITCHBOT, sizeof(TPacketGGSwitchbot), "Switchbot", false);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	Set(HEADER_GG_SHOP_OFFLINE_START, sizeof(TPacketGGShopStartOffline), "ShopStartOffline", false);
	Set(HEADER_GG_SHOP_OFFLINE_END, sizeof(TPacketGGShopEndOffline), "ShopEndOffline", false);
#endif
#ifdef ENABLE_BATTLE_FIELD
	Set(HEADER_GG_COMMAND, sizeof(TPacketGGCommand), "BroadcastCommand", false);
#endif
#ifdef ENABLE_RANKING_SYSTEM
	Set(HEADER_GG_LOAD_RANKING, sizeof(TPacketGGLoadRanking), "LoadRanking", false);
#endif
#ifdef ENABLE_MULTI_FARM_BLOCK
	Set(HEADER_GG_MULTI_FARM, sizeof(TPacketGGMultiFarm), "MultiFarm", false);
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	Set(HEADER_GG_STONE_EVENT, sizeof(TPacketGGStoneEvent), "StoneEvent", false);
#endif
#ifdef ENABLE_EVENT_MANAGER
	Set(HEADER_GG_EVENT_RELOAD, sizeof(TPacketGGReloadEvent), "Event", false);
	Set(HEADER_GG_EVENT, sizeof(TPacketGGEvent), "Event", false);
	Set(HEADER_GG_EVENT_HIDE_AND_SEEK, sizeof(TPacketGGEventHideAndSeek), "Hide&SeekEvent", false);
	Set(HEADER_GG_EVENT_BATTLE_PASS, sizeof(TPacketGGEventBattlePass), "BattlePassEvent", false);
#endif
}

CPacketInfoGG::~CPacketInfoGG()
{
	Log("p2p_packet_info.txt");
}

