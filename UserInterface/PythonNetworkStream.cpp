#include "StdAfx.h"
#include "../EterLib/NetPacketHeaderMap.h"

#include "PythonNetworkStream.h"
#include "Packet.h"
#include "NetworkActorManager.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "MarkManager.h"

#include "ProcessCRC.h"
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "PythonDungeonInfo.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "PythonAchievement.h"
#endif

// MARK_BUG_FIX
static uint32_t gs_nextDownloadMarkTime = 0;
// END_OF_MARK_BUG_FIX

// Packet ---------------------------------------------------------------------------
class CMainPacketHeaderMap : public CNetworkPacketHeaderMap
{
public:
	enum
	{
		STATIC_SIZE_PACKET = false,
		DYNAMIC_SIZE_PACKET = true
	};

public:
	CMainPacketHeaderMap()
	{
		Set(HEADER_GC_EMPIRE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEmpire), STATIC_SIZE_PACKET));
		Set(HEADER_GC_WARP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWarp), STATIC_SIZE_PACKET));
		Set(HEADER_GC_QUEST_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuestInfo), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_REQUEST_MAKE_GUILD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PVP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPVP), STATIC_SIZE_PACKET));
		Set(HEADER_GC_DUEL_START, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDuelStart), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_CHARACTER_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterAdd), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHAR_ADDITIONAL_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterAdditionalInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHARACTER_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterUpdate), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHARACTER_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterDelete), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHARACTER_MOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMove), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHAT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChat), DYNAMIC_SIZE_PACKET));

		Set(HEADER_GC_SYNC_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSyncPosition), DYNAMIC_SIZE_PACKET));

		Set(HEADER_GC_LOGIN_SUCCESS_NEWSLOT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginSuccess), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LOGIN_FAILURE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginFailure), STATIC_SIZE_PACKET));

		Set(HEADER_GC_PLAYER_CREATE_SUCCESS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPlayerCreateSuccess), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PLAYER_CREATE_FAILURE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCreateFailure), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PLAYER_DELETE_SUCCESS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
#ifdef ENABLE_DELETE_FAILURE_TYPE
		Set(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDestroyCharacterFailure), STATIC_SIZE_PACKET));
#else
		Set(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
#endif

		Set(HEADER_GC_STUN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCStun), STATIC_SIZE_PACKET));
		Set(HEADER_GC_DEAD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDead), STATIC_SIZE_PACKET));

		Set(HEADER_GC_MAIN_CHARACTER, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MAIN_CHARACTER3_BGM, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter3_BGM), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MAIN_CHARACTER4_BGM_VOL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter4_BGM_VOL), STATIC_SIZE_PACKET));

		Set(HEADER_GC_PLAYER_POINTS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPoints), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PLAYER_POINT_CHANGE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPointChange), STATIC_SIZE_PACKET));

		Set(HEADER_GC_ITEM_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDelDeprecated), STATIC_SIZE_PACKET));
		Set(HEADER_GC_ITEM_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));

		Set(HEADER_GC_ITEM_USE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemUse), STATIC_SIZE_PACKET));
		Set(HEADER_GC_ITEM_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemUpdate), STATIC_SIZE_PACKET));

		Set(HEADER_GC_ITEM_GROUND_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemGroundAdd), STATIC_SIZE_PACKET));
		Set(HEADER_GC_ITEM_GROUND_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemGroundDel), STATIC_SIZE_PACKET));
		Set(HEADER_GC_ITEM_OWNERSHIP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemOwnership), STATIC_SIZE_PACKET));

		Set(HEADER_GC_QUICKSLOT_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotAdd), STATIC_SIZE_PACKET));
		Set(HEADER_GC_QUICKSLOT_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotDel), STATIC_SIZE_PACKET));
		Set(HEADER_GC_QUICKSLOT_SWAP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotSwap), STATIC_SIZE_PACKET));

		Set(HEADER_GC_WHISPER, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWhisper), STATIC_SIZE_PACKET));

		Set(HEADER_GC_CHARACTER_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPosition), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MOTION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMotion), STATIC_SIZE_PACKET));

		Set(HEADER_GC_SHOP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShop), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_SHOP_SIGN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopSign), STATIC_SIZE_PACKET));
		Set(HEADER_GC_EXCHANGE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExchange), STATIC_SIZE_PACKET));

		Set(HEADER_GC_PING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPing), STATIC_SIZE_PACKET));

		Set(HEADER_GC_SCRIPT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCScript), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_QUEST_CONFIRM, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuestConfirm), STATIC_SIZE_PACKET));

		Set(HEADER_GC_TARGET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTarget), STATIC_SIZE_PACKET));
#ifdef ENABLE_SEND_TARGET_INFO
		Set(HEADER_GC_TARGET_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetInfo), STATIC_SIZE_PACKET));
#endif
		Set(HEADER_GC_HANDSHAKE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHandshake), STATIC_SIZE_PACKET));
		Set(HEADER_GC_HANDSHAKE_OK, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
		Set(HEADER_GC_BINDUDP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBindUDP), STATIC_SIZE_PACKET));
		Set(HEADER_GC_OWNERSHIP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOwnership), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CREATE_FLY, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCreateFly), STATIC_SIZE_PACKET));
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		Set(HEADER_GC_KEY_AGREEMENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketKeyAgreement), STATIC_SIZE_PACKET));
		Set(HEADER_GC_KEY_AGREEMENT_COMPLETED, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketKeyAgreementCompleted), STATIC_SIZE_PACKET));
#endif
		Set(HEADER_GC_ADD_FLY_TARGETING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFlyTargeting), STATIC_SIZE_PACKET));
		Set(HEADER_GC_FLY_TARGETING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFlyTargeting), STATIC_SIZE_PACKET));

		Set(HEADER_GC_PHASE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPhase), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SKILL_LEVEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSkillLevel), STATIC_SIZE_PACKET));

		Set(HEADER_GC_MESSENGER, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMessenger), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_GUILD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGuild), DYNAMIC_SIZE_PACKET));

		Set(HEADER_GC_PARTY_INVITE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyInvite), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PARTY_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyAdd), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PARTY_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyUpdate), STATIC_SIZE_PACKET));
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
		Set(HEADER_GC_PARTY_POSITION_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyPosition), DYNAMIC_SIZE_PACKET));
#endif
		Set(HEADER_GC_PARTY_REMOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyRemove), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PARTY_LINK, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyLink), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PARTY_UNLINK, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyUnlink), STATIC_SIZE_PACKET));
		Set(HEADER_GC_PARTY_PARAMETER, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyParameter), STATIC_SIZE_PACKET));

		Set(HEADER_GC_SAFEBOX_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SAFEBOX_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SAFEBOX_WRONG_PASSWORD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxWrongPassword), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SAFEBOX_SIZE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxSize), STATIC_SIZE_PACKET));

		Set(HEADER_GC_FISHING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFishing), STATIC_SIZE_PACKET));
		Set(HEADER_GC_DUNGEON, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDungeon), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_TIME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTime), STATIC_SIZE_PACKET));
		Set(HEADER_GC_WALK_MODE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWalkMode), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHANGE_SKILL_GROUP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeSkillGroup), STATIC_SIZE_PACKET));
		Set(HEADER_GC_REFINE_INFORMATION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCRefineInformation), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SEPCIAL_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecialEffect), STATIC_SIZE_PACKET));
		Set(HEADER_GC_NPC_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNPCPosition), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_CHANGE_NAME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeName), STATIC_SIZE_PACKET));

		Set(HEADER_GC_AUTH_SUCCESS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAuthSuccess), STATIC_SIZE_PACKET));
		Set(HEADER_GC_CHANNEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChannel), STATIC_SIZE_PACKET));
		Set(HEADER_GC_VIEW_EQUIP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCViewEquip), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LAND_LIST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLandList), DYNAMIC_SIZE_PACKET));

		Set(HEADER_GC_TARGET_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetUpdate), STATIC_SIZE_PACKET));
		Set(HEADER_GC_TARGET_DELETE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetDelete), STATIC_SIZE_PACKET));
		Set(HEADER_GC_TARGET_CREATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetCreate), STATIC_SIZE_PACKET));

		Set(HEADER_GC_AFFECT_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAffectAdd), STATIC_SIZE_PACKET));
		Set(HEADER_GC_AFFECT_REMOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAffectRemove), STATIC_SIZE_PACKET));

		Set(HEADER_GC_MALL_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMallOpen), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MALL_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MALL_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		Set(HEADER_GC_GUILDSTORAGE_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGuildstorageOpen), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GUILDSTORAGE_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GUILDSTORAGE_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));
#endif

		Set(HEADER_GC_LOVER_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoverInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LOVE_POINT_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLovePointUpdate), STATIC_SIZE_PACKET));

		Set(HEADER_GC_DIG_MOTION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDigMotion), STATIC_SIZE_PACKET));
		Set(HEADER_GC_DAMAGE_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDamageInfo), STATIC_SIZE_PACKET));

		Set(HEADER_GC_HYBRIDCRYPT_KEYS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHybridCryptKeys), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_HYBRIDCRYPT_SDB, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHybridSDB), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_SPECIFIC_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecificEffect), STATIC_SIZE_PACKET));
		Set(HEADER_GC_DRAGON_SOUL_REFINE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDragonSoulRefine), STATIC_SIZE_PACKET));
#ifdef ENABLE_GROWTH_PET_SYSTEM
		Set(HEADER_GC_GROWTH_PET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPet), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_GROWTH_PET_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetInfo), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_GROWTH_PET_HATCHING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetHatching), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GROWTH_PET_UPGRADE_SKILL_REQUEST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetSkillUpgradeRequest), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GROWTH_PET_FEED_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetFeedResult), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetNameChangeResult), STATIC_SIZE_PACKET));
#	ifdef ENABLE_PET_ATTR_DETERMINE
		Set(HEADER_GC_GROWTH_ATTR_DETERMINE_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetAttrDetermineResult), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GROWTH_ATTR_CHANGE_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetAttrChangeResult), STATIC_SIZE_PACKET));
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
		Set(HEADER_GC_GROWTH_PET_REVIVE_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGrowthPetReviveResult), STATIC_SIZE_PACKET));
#	endif
#endif
#ifdef ENABLE_INGAME_WIKI
		Set(HEADER_GC_WIKI, CNetworkPacketHeaderMap::TPacketType(sizeof(TGCWikiPacket), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SWITCHBOT
		Set(HEADER_GC_SWITCHBOT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSwitchbot), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
		Set(HEADER_GC_SEAL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSeal), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_12ZI
		Set(HEADER_GC_SEPCIAL_ZODIAC_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecialZodiacEffect), STATIC_SIZE_PACKET));
#endif
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		Set(HEADER_GC_GEM_SHOP_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGemShopItems), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GEM_SHOP_REFRESH, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGemShopItems), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GEM_SHOP_BUY, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGemShopBuy), STATIC_SIZE_PACKET));
		Set(HEADER_GC_GEM_SHOP_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGemShopAdd), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
		Set(HEADER_GC_PRIVATE_SHOP_SEARCH_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPrivateShopSearchItemInfo), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_PRIVATE_SHOP_SEARCH_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPrivateShopSearchOpen), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_CUBE_RENEWAL
		Set(HEADER_GC_CUBE_RENEWAL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCubeRenewal), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_BATTLE_FIELD
		Set(HEADER_GC_BATTLE_ZONE_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBattleInfo), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		Set(HEADER_GC_ATTR_6TH_7TH, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAttr67Receive), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		Set(HEADER_GC_PLAYER_SHOP_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketPlayerShopSet), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MY_SHOP_SIGN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketMyShopSign), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SYNC_SHOP_STASH, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopStashSync), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SYNC_SHOP_OFFTIME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopOffTimeSync), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SYNC_SHOP_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopSyncPos), STATIC_SIZE_PACKET));
		Set(HEADER_GC_SHOP_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNPCPosition), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_REFINE_ELEMENT
		Set(HEADER_GC_ELEMENTS_SPELL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCElementsSpell), STATIC_SIZE_PACKET));
#endif
#ifdef NEW_SELECT_CHARACTER
		Set(HEADER_GC_CHARACTER_INFORMATION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketCharacterInformation), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		Set(HEADER_GC_EXTEND_INVEN_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtendInvenInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_EXTEND_INVEN_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtendInvenResult), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		Set(HEADER_GC_ACCE_REFINE_REQUEST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSendAcceRefine), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_AURA_SYSTEM
		Set(HEADER_GC_AURA, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAura), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_CHANGED_ATTR
		Set(HEADER_GC_ITEM_SELECT_ATTR, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSelectAttr), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_TICKET_SYSTEM
		Set(HEADER_GC_TICKET_SYSTEM, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTicketSystem), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MAILBOX
		Set(HEADER_GC_MAILBOX_PROCESS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketMailboxProcess), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MAILBOX, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMailBox), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_MAILBOX_ADD_DATA, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMailBoxAddData), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MAILBOX_ALL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMailBox), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_MAILBOX_UNREAD, CNetworkPacketHeaderMap::TPacketType(sizeof(TMailBoxRespondUnreadData), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		Set(HEADER_GC_CHANGE_LOOK, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTransmutation), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		Set(HEADER_GC_TARGET_LANGUAGE_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetLanguageResult), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_PARTY_MATCH
		Set(HEADER_GC_PARTY_MATCH, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyMatch), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
		Set(HEADER_GC_DUNGEON_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDungeonInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_DUNGEON_RANKING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDungeonInfoRank), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		Set(HEADER_GC_EXT_BATTLE_PASS_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtBattlePassOpen), STATIC_SIZE_PACKET));
		Set(HEADER_GC_EXT_BATTLE_PASS_GENERAL_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtBattlePassGeneralInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_EXT_BATTLE_PASS_MISSION_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtBattlePassMissionInfo), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtBattlePassMissionUpdate), STATIC_SIZE_PACKET));
		Set(HEADER_GC_EXT_BATTLE_PASS_SEND_RANKING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtBattlePassRanking), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		Set(HEADER_GC_MELEY_LAIR_TIME_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketMeleyLairTimeInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MELEY_LAIR_TIME_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketMeleyLairTimeResult), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		Set(HEADER_GC_ACHIEVEMENT, CNetworkPacketHeaderMap::TPacketType(sizeof(achievements::GC_packet), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
		Set(HEADER_GC_BIOLOG_MANAGER, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBiologManager), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_HUNTING_SYSTEM
		Set(HEADER_GC_HUNTING_OPEN_MAIN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOpenWindowHuntingMain), STATIC_SIZE_PACKET));
		Set(HEADER_GC_HUNTING_OPEN_SELECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOpenWindowHuntingSelect), STATIC_SIZE_PACKET));
		Set(HEADER_GC_HUNTING_OPEN_REWARD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOpenWindowReward), STATIC_SIZE_PACKET));
		Set(HEADER_GC_HUNTING_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCUpdateHunting), STATIC_SIZE_PACKET));
		Set(HEADER_GC_HUNTING_RECIVE_RAND_ITEMS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCReciveRandomItems), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		Set(HEADER_GC_LOTTO_OPENINGS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLotteryOpenings), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LOTTO_SEND_BASIC_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCReciveLottoBaseInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LOTTO_SEND_TICKET_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCReciveLottoTicketInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LOTTO_SEND_RANKING_JACKPOT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSendRankingJackpotInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_LOTTO_SEND_RANKING_MONEY, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSendRankingMoneyInfo), STATIC_SIZE_PACKET));
#endif

	// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
		Set(HEADER_GC_EVENT_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEventInfo), DYNAMIC_SIZE_PACKET));
		Set(HEADER_GC_EVENT_RELOAD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEventReload), STATIC_SIZE_PACKET));
		Set(HEADER_GC_EVENT_KW_SCORE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEventKWScore), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		Set(HEADER_GC_OKEY_CARD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameOkeyCard), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MONSTER_BACK
		Set(HEADER_GC_ATTENDANCE_EVENT_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAttendanceEventInfo), STATIC_SIZE_PACKET));
		Set(HEADER_GC_ATTENDANCE_EVENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAttendanceEvent), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
		Set(HEADER_GC_HIT_COUNT_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHitCountInfo), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
		Set(HEADER_GC_MINI_GAME_CATCH_KING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameCatchKing), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
		Set(HEADER_GC_STONE_EVENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCStoneEvent), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_FISH_EVENT
		Set(HEADER_GC_FISH_EVENT_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFishEventInfo), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		Set(HEADER_GC_SOUL_ROULETTE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSoulRoulette), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_BNW
		Set(HEADER_GC_MINI_GAME_BNW, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameBNW), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_FINDM
		Set(HEADER_GC_MINI_GAME_FIND_M, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameFindM), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		Set(HEADER_GC_MINI_GAME_YUT_NORI, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameYutNori), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_FISHING_RENEWAL
		Set(HEADER_GC_FISHING_NEW, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketFishingNew), STATIC_SIZE_PACKET));
#endif
	// END MINI GAME EVENTS //
	}
};

int g_iLastPacket[2] = {0, 0};

void CPythonNetworkStream::ExitApplication()
{
	if (__IsNotPing())
		AbsoluteExitApplication();
	else
		SendChatPacket("/quit");
}

void CPythonNetworkStream::ExitGame()
{
	if (__IsNotPing())
		LogOutGame();
	else
		SendChatPacket("/phase_select");
}


void CPythonNetworkStream::LogOutGame()
{
	if (__IsNotPing())
		AbsoluteExitGame();
	else
		SendChatPacket("/logout");
}

void CPythonNetworkStream::AbsoluteExitGame()
{
	if (!IsOnline())
		return;

	OnRemoteDisconnect();
	Disconnect();
}

void CPythonNetworkStream::AbsoluteExitApplication() const
{
	PostQuitMessage(0);
}

bool CPythonNetworkStream::__IsNotPing() const
{
	return false;
}

uint32_t CPythonNetworkStream::GetGuildID() const
{
	return m_dwGuildID;
}

uint32_t CPythonNetworkStream::UploadMark(const char * c_szImageFileName) const
{
	// MARK_BUG_FIX
	if (0 == m_dwGuildID)
		return ERROR_MARK_UPLOAD_NEED_RECONNECT;

	gs_nextDownloadMarkTime = 0;
	// END_OF_MARK_BUG_FIX

	uint32_t uError = ERROR_UNKNOWN;
	CGuildMarkUploader & rkGuildMarkUploader = CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID,
									 c_szImageFileName, &uError))
	{
		switch (uError)
		{
			case CGuildMarkUploader::ERROR_CONNECT:
				return ERROR_CONNECT_MARK_SERVER;
			case CGuildMarkUploader::ERROR_LOAD:
				return ERROR_LOAD_MARK;
			case CGuildMarkUploader::ERROR_WIDTH:
				return ERROR_MARK_WIDTH;
			case CGuildMarkUploader::ERROR_HEIGHT:
				return ERROR_MARK_HEIGHT;
			default:
				return ERROR_UNKNOWN;
		}
	}

	// MARK_BUG_FIX
	__DownloadMark();
	// END_OF_MARK_BUG_FIX

	if (CGuildMarkManager::INVALID_MARK_ID == CGuildMarkManager::Instance().GetMarkID(m_dwGuildID))
		return ERROR_MARK_CHECK_NEED_RECONNECT;

	return ERROR_NONE;
}

uint32_t CPythonNetworkStream::UploadSymbol(const char * c_szImageFileName) const
{
	uint32_t uError = ERROR_UNKNOWN;
	CGuildMarkUploader & rkGuildMarkUploader = CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.ConnectToSendSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID,
												 c_szImageFileName, &uError))
	{
		switch (uError)
		{
			case CGuildMarkUploader::ERROR_CONNECT:
				return ERROR_CONNECT_MARK_SERVER;
			case CGuildMarkUploader::ERROR_LOAD:
				return ERROR_LOAD_MARK;
			case CGuildMarkUploader::ERROR_WIDTH:
				return ERROR_MARK_WIDTH;
			case CGuildMarkUploader::ERROR_HEIGHT:
				return ERROR_MARK_HEIGHT;
			default:
				return ERROR_UNKNOWN;
		}
	}

	return ERROR_NONE;
}

void CPythonNetworkStream::__DownloadMark() const
{
	uint32_t curTime = ELTimer_GetMSec();

	if (curTime < gs_nextDownloadMarkTime)
		return;

	gs_nextDownloadMarkTime = curTime + 60000 * 3; // 3분

	CGuildMarkDownloader & rkGuildMarkDownloader = CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey);
}

void CPythonNetworkStream::__DownloadSymbol(const std::vector<uint32_t> & c_rkVec_dwGuildID) const
{
	CGuildMarkDownloader & rkGuildMarkDownloader = CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.ConnectToRecvSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, c_rkVec_dwGuildID);
}

void CPythonNetworkStream::SetPhaseWindow(uint32_t ePhaseWnd, PyObject * poPhaseWnd)
{
	if (ePhaseWnd >= PHASE_WINDOW_NUM)
		return;

	m_apoPhaseWnd[ePhaseWnd] = poPhaseWnd;
}

void CPythonNetworkStream::ClearPhaseWindow(uint32_t ePhaseWnd, const PyObject * poPhaseWnd)
{
	if (ePhaseWnd >= PHASE_WINDOW_NUM)
		return;

	if (poPhaseWnd != m_apoPhaseWnd[ePhaseWnd])
		return;

	m_apoPhaseWnd[ePhaseWnd] = nullptr;
#ifdef ENABLE_FISHING_RENEWAL
	m_phaseWindowGame = false;
#endif
}

void CPythonNetworkStream::SetServerCommandParserWindow(PyObject * poWnd)
{
	m_poSerCommandParserWnd = poWnd;
}

bool CPythonNetworkStream::IsSelectedEmpire() const
{
	if (m_dwEmpireID)
		return true;

	return false;
}

uint32_t CPythonNetworkStream::GetAccountCharacterSlotDatau(uint32_t iSlot, uint32_t eType)
{
	if (iSlot >= PLAYER_PER_ACCOUNT)
		return 0;

	TSimplePlayerInformation & rkSimplePlayerInfo = m_akSimplePlayerInfo[iSlot];

	switch (eType)
	{
	case ACCOUNT_CHARACTER_SLOT_ID:
		return rkSimplePlayerInfo.dwID;
	case ACCOUNT_CHARACTER_SLOT_RACE:
		return rkSimplePlayerInfo.byJob;
	case ACCOUNT_CHARACTER_SLOT_LEVEL:
		return rkSimplePlayerInfo.byLevel;
#ifdef ENABLE_YOHARA_SYSTEM
	case ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL:
		return rkSimplePlayerInfo.bycLevel;
#endif
	case ACCOUNT_CHARACTER_SLOT_STR:
		return rkSimplePlayerInfo.byST;
	case ACCOUNT_CHARACTER_SLOT_DEX:
		return rkSimplePlayerInfo.byDX;
	case ACCOUNT_CHARACTER_SLOT_HTH:
		return rkSimplePlayerInfo.byHT;
	case ACCOUNT_CHARACTER_SLOT_INT:
		return rkSimplePlayerInfo.byIQ;
#ifdef ENABLE_YOHARA_SYSTEM
	case ACCOUNT_CHARACTER_SLOT_SUNGMA_STR:
		return rkSimplePlayerInfo.bySungST;
	case ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE:
		return rkSimplePlayerInfo.bySungDX;
	case ACCOUNT_CHARACTER_SLOT_SUNGMA_HP:
		return rkSimplePlayerInfo.bySungHT;
	case ACCOUNT_CHARACTER_SLOT_SUNGMA_IMMUNE:
		return rkSimplePlayerInfo.bySungIQ;
#endif
	case ACCOUNT_CHARACTER_SLOT_PLAYTIME:
		return rkSimplePlayerInfo.dwPlayMinutes;
	case ACCOUNT_CHARACTER_SLOT_FORM:
//		return rkSimplePlayerInfo.wParts[CRaceData::PART_MAIN];
		return rkSimplePlayerInfo.wMainPart;
	case ACCOUNT_CHARACTER_SLOT_PORT:
		return rkSimplePlayerInfo.wPort;
	case ACCOUNT_CHARACTER_SLOT_GUILD_ID:
		return m_adwGuildID[iSlot];
	case ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG:
		return rkSimplePlayerInfo.bChangeName;
	case ACCOUNT_CHARACTER_SLOT_HAIR:
		return rkSimplePlayerInfo.wHairPart;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	case ACCOUNT_CHARACTER_SLOT_ACCE:
		return rkSimplePlayerInfo.wAccePart;
#endif
#ifdef ENABLE_AURA_SYSTEM
	case ACCOUNT_CHARACTER_SLOT_AURA:
		return rkSimplePlayerInfo.wAuraPart;
#endif
#ifdef NEW_SELECT_CHARACTER
	case ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME:
		return rkSimplePlayerInfo.dwLastPlayTime;
#endif
	}
	return 0;
}

const char * CPythonNetworkStream::GetAccountCharacterSlotDataz(uint32_t iSlot, uint32_t eType)
{
	static const char * sc_szEmpty = "";

	if (iSlot >= PLAYER_PER_ACCOUNT)
		return sc_szEmpty;

	TSimplePlayerInformation & rkSimplePlayerInfo = m_akSimplePlayerInfo[iSlot];

	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ADDR:
		{
			uint8_t ip[4];

			const int LEN = 4;
			for (auto & i : ip)
			{
				i = rkSimplePlayerInfo.lAddr & 0xff;
				rkSimplePlayerInfo.lAddr >>= 8;
			}


			static char s_szAddr[256];
			sprintf(s_szAddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
			return s_szAddr;
		}
		case ACCOUNT_CHARACTER_SLOT_NAME:
			return rkSimplePlayerInfo.szName;
		case ACCOUNT_CHARACTER_SLOT_GUILD_NAME:
			return m_astrGuildName[iSlot].c_str();
	}

	return sc_szEmpty;
}

void CPythonNetworkStream::ConnectLoginServer(const char * c_szAddr, uint32_t uPort)
{
	Connect(c_szAddr, uPort);
}

void CPythonNetworkStream::SetMarkServer(const char * c_szAddr, uint32_t uPort)
{
	m_kMarkAuth.m_kNetAddr.Set(c_szAddr, uPort);
}

void CPythonNetworkStream::ConnectGameServer(uint32_t iChrSlot)
{
	if (iChrSlot >= PLAYER_PER_ACCOUNT)
		return;

	m_dwSelectedCharacterIndex = iChrSlot;

	__DirectEnterMode_Set(iChrSlot);

	TSimplePlayerInformation & rkSimplePlayerInfo = m_akSimplePlayerInfo[iChrSlot];
	Connect(static_cast<uint32_t>(rkSimplePlayerInfo.lAddr), rkSimplePlayerInfo.wPort);
}

void CPythonNetworkStream::SetLoginInfo(const char * c_szID, const char * c_szPassword)
{
	m_stID = c_szID;
	m_stPassword = c_szPassword;
}

void CPythonNetworkStream::ClearLoginInfo()
{
	m_stPassword = "";
}

void CPythonNetworkStream::SetLoginKey(uint32_t dwLoginKey)
{
	m_dwLoginKey = dwLoginKey;
}

#ifdef NEW_SELECT_CHARACTER
const char* CPythonNetworkStream::GetLoginID()
{
return m_stID.c_str();
}
#endif

bool CPythonNetworkStream::CheckPacket(TPacketHeader * pRetHeader)
{
	*pRetHeader = 0;

	static CMainPacketHeaderMap s_packetHeaderMap;

	TPacketHeader header;

	if (!Peek(sizeof(TPacketHeader), &header))
		return false;

	if (0 == header)
	{
		if (!Recv(sizeof(TPacketHeader), &header))
			return false;

		while (Peek(sizeof(TPacketHeader), &header))
		{
			if (0 == header)
			{
				if (!Recv(sizeof(TPacketHeader), &header))
					return false;
			}
			else
				break;
		}

		if (0 == header)
			return false;
	}

	CNetworkPacketHeaderMap::TPacketType PacketType;

	if (!s_packetHeaderMap.Get(header, &PacketType))
	{
		TraceError("Unknown packet header: %d, last: %d %d", header, g_iLastPacket[0], g_iLastPacket[1]);
		ClearRecvBuffer();

		PostQuitMessage(0);
		return false;
	}

	// Code for dynamic size packet
	if (PacketType.isDynamicSizePacket)
	{
		TDynamicSizePacketHeader DynamicSizePacketHeader;

		if (!Peek(sizeof(TDynamicSizePacketHeader), &DynamicSizePacketHeader))
			return false;

		if (!Peek(DynamicSizePacketHeader.size))
		{
			Tracef("CPythonNetworkStream::CheckPacket - Not enough dynamic packet size: header %d packet size: %d\n",
				   DynamicSizePacketHeader.header, DynamicSizePacketHeader.size);
			return false;
		}
	}
	else
	{
		if (!Peek(PacketType.iPacketSize))
		{
			//Tracef("Not enough packet size: header %d packet size: %d, recv buffer size: %d",
			//	header,
			//	PacketType.iPacketSize,
			//	GetRecvBufferSize());
			return false;
		}
	}

	if (!header)
		return false;

	*pRetHeader = header;

	g_iLastPacket[0] = g_iLastPacket[1];
	g_iLastPacket[1] = header;
	//Tracenf("header %d size %d", header, PacketType.iPacketSize);
	//Tracenf("header %d size %d outputpos[%d] security %u", header, PacketType.iPacketSize, m_recvBufOutputPos, IsSecurityMode());
	return true;
}

bool CPythonNetworkStream::RecvErrorPacket(int header)
{
	TraceError("Phase %s does not handle this header (header: %d, last: %d, %d)", m_strPhase.c_str(), header, g_iLastPacket[0],
			   g_iLastPacket[1]);

	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::RecvPhasePacket()
{
	TPacketGCPhase packet_phase;

	if (!Recv(sizeof(TPacketGCPhase), &packet_phase))
		return false;

	switch (packet_phase.phase)
	{
	case PHASE_CLOSE: // 끊기는 상태 (또는 끊기 전 상태)
		ClosePhase();
		break;

	case PHASE_HANDSHAKE: // 악수..;;
		SetHandShakePhase();
		break;

	case PHASE_LOGIN: // 로그인 중
		SetLoginPhase();
		break;

	case PHASE_SELECT: // 캐릭터 선택 화면
		SetSelectPhase();

		BuildProcessCRC();

		// MARK_BUG_FIX
		__DownloadMark();
		// END_OF_MARK_BUG_FIX
		break;

	case PHASE_LOADING: // 선택 후 로딩 화면
		SetLoadingPhase();
		break;

	case PHASE_GAME: // 게임 화면
		SetGamePhase();
#ifdef ENABLE_MAP_LOCATION_APP_NAME
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetApplicationText", Py_BuildValue("()"));
#endif
		break;

	case PHASE_DEAD: // 죽었을 때.. (게임 안에 있는 것일 수도..)
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvPingPacket()
{
	Tracef("recv ping packet. (securitymode %u)\n", IsSecurityMode());

	TPacketGCPing kPacketPing;

	if (!Recv(sizeof(TPacketGCPing), &kPacketPing))
		return false;

	m_dwLastGamePingTime = ELTimer_GetMSec();

	TPacketCGPong kPacketPong;
	kPacketPong.bHeader = HEADER_CG_PONG;

	if (!Send(sizeof(TPacketCGPong), &kPacketPong))
		return false;

	if (IsSecurityMode())
		return SendSequence();
	return true;
}

bool CPythonNetworkStream::RecvDefaultPacket(int header)
{
	if (!header)
		return true;

	TraceError("Unprocessed packet header %d, state %s\n", header, m_strPhase.c_str());
	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::OnProcess()
{
	if (m_isStartGame)
	{
		m_isStartGame = FALSE;

		PyCallClassMemberFunc(m_poHandler, "SetGamePhase", Py_BuildValue("()"));
		//		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartGame", Py_BuildValue("()"));
	}

	m_rokNetActorMgr->Update();

	if (m_phaseProcessFunc.IsEmpty())
		return true;

	//TPacketHeader header;
	//while(CheckPacket(&header))
	{
		m_phaseProcessFunc.Run();
	}

	return true;
}

// Set
void CPythonNetworkStream::SetOffLinePhase()
{
	if ("OffLine" != m_strPhase)
		m_phaseLeaveFunc.Run();

	m_strPhase = "OffLine";

	Tracen("");
	Tracen("## Network - OffLine Phase ##");
	Tracen("");

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::OffLinePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveOfflinePhase);

	SetGameOffline();

	m_dwSelectedCharacterIndex = 0;

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();
}

void CPythonNetworkStream::HideQuestWindows()	//@fixme419
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "HideAllQuestWindow", Py_BuildValue("()"));
}

void CPythonNetworkStream::ClosePhase() const
{
	PyCallClassMemberFunc(m_poHandler, "SetLoginPhase", Py_BuildValue("()"));
}

// Game Online
void CPythonNetworkStream::SetGameOnline()
{
	m_isGameOnline = TRUE;
}

void CPythonNetworkStream::SetGameOffline()
{
	m_isGameOnline = FALSE;
}

BOOL CPythonNetworkStream::IsGameOnline() const
{
	return m_isGameOnline;
}

// Handler
void CPythonNetworkStream::SetHandler(PyObject * poHandler)
{
	m_poHandler = poHandler;
}

// ETC
uint32_t CPythonNetworkStream::GetMainActorVID() const
{
	return m_dwMainActorVID;
}

uint32_t CPythonNetworkStream::GetMainActorRace() const
{
	return m_dwMainActorRace;
}

uint32_t CPythonNetworkStream::GetMainActorEmpire() const
{
	return m_dwMainActorEmpire;
}

uint32_t CPythonNetworkStream::GetMainActorSkillGroup() const
{
	return m_dwMainActorSkillGroup;
}

void CPythonNetworkStream::SetEmpireID(uint32_t dwEmpireID)
{
	m_dwEmpireID = dwEmpireID;
}

uint32_t CPythonNetworkStream::GetEmpireID() const
{
	return m_dwEmpireID;
}

void CPythonNetworkStream::__ClearSelectCharacterData()
{
	NANOBEGIN
	msl::refill(m_akSimplePlayerInfo);

	for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
	{
		m_adwGuildID[i] = 0;
		m_astrGuildName[i] = "";
	}
	NANOEND
}

void CPythonNetworkStream::__DirectEnterMode_Initialize()
{
	m_kDirectEnterMode.m_isSet = false;
	m_kDirectEnterMode.m_dwChrSlotIndex = 0;
}

void CPythonNetworkStream::__DirectEnterMode_Set(uint32_t uChrSlotIndex)
{
	m_kDirectEnterMode.m_isSet = true;
	m_kDirectEnterMode.m_dwChrSlotIndex = uChrSlotIndex;
}

bool CPythonNetworkStream::__DirectEnterMode_IsSet() const
{
	return m_kDirectEnterMode.m_isSet;
}

void CPythonNetworkStream::__InitializeMarkAuth()
{
	m_kMarkAuth.m_dwHandle = 0;
	m_kMarkAuth.m_dwRandomKey = 0;
}

void CPythonNetworkStream::__BettingGuildWar_Initialize()
{
	m_kBettingGuildWar.m_dwBettingMoney = 0;
	m_kBettingGuildWar.m_dwObserverCount = 0;
}

void CPythonNetworkStream::__BettingGuildWar_SetObserverCount(uint32_t uObserverCount)
{
	m_kBettingGuildWar.m_dwObserverCount = uObserverCount;
}

void CPythonNetworkStream::__BettingGuildWar_SetBettingMoney(uint32_t uBettingMoney)
{
	m_kBettingGuildWar.m_dwBettingMoney = uBettingMoney;
}

uint32_t CPythonNetworkStream::EXPORT_GetBettingGuildWarValue(const char * c_szValueName) const
{
	if (stricmp(c_szValueName, "OBSERVER_COUNT") == 0)
		return m_kBettingGuildWar.m_dwObserverCount;

	if (stricmp(c_szValueName, "BETTING_MONEY") == 0)
		return m_kBettingGuildWar.m_dwBettingMoney;

	return 0;
}

void CPythonNetworkStream::__ServerTimeSync_Initialize()
{
	m_kServerTimeSync.m_dwChangeClientTime = 0;
	m_kServerTimeSync.m_dwChangeServerTime = 0;
}

void CPythonNetworkStream::SetWaitFlag()
{
	m_isWaitLoginKey = TRUE;
}

void CPythonNetworkStream::SendEmoticon(uint32_t eEmoticon)
{
	if (eEmoticon < m_EmoticonStringVector.size())
		SendChatPacket(m_EmoticonStringVector[eEmoticon].c_str());
	else
		assert(false && "SendEmoticon Error");
}

CPythonNetworkStream::CPythonNetworkStream()
{
	m_rokNetActorMgr = new CNetworkActorManager;

	msl::refill(m_akSimplePlayerInfo);

	m_phaseProcessFunc.Clear();

	m_dwEmpireID = 0;
	m_dwGuildID = 0;

	m_dwMainActorVID = 0;
	m_dwMainActorRace = 0;
	m_dwMainActorEmpire = 0;
	m_dwMainActorSkillGroup = 0;
	m_poHandler = nullptr;

	m_dwLastGamePingTime = 0;

	m_dwLoginKey = 0;
	m_isWaitLoginKey = FALSE;
	m_isStartGame = FALSE;
	m_isEnableChatInsultFilter = FALSE;
	m_bComboSkillFlag = FALSE;
	m_strPhase = "OffLine";

	__InitializeGamePhase();
	__InitializeMarkAuth();

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();

	std::fill(m_apoPhaseWnd, m_apoPhaseWnd + PHASE_WINDOW_NUM, (PyObject *) nullptr);
	m_poSerCommandParserWnd = nullptr;

	SetOffLinePhase();
#ifdef ENABLE_FISHING_RENEWAL
	m_phaseWindowGame = false;
#endif
}

CPythonNetworkStream::~CPythonNetworkStream()
{
#ifdef ENABLE_PARTY_MATCH
	m_PartyMatch.clear();
#endif
	Tracen("PythonNetworkMainStream Clear");
}
