#ifndef __INC_TABLES_H__
#define __INC_TABLES_H__

#include "length.h"
#include "item_length.h"
#include "CommonDefines.h"

typedef uint32_t IDENT;

/**
* @version 05/06/10 Bang2ni - Myshop Pricelist 관련 패킷 HEADER_XX_MYSHOP_PRICELIST_XXX 추가
*/
enum
{
	HEADER_GD_LOGIN								= 1,
	HEADER_GD_LOGOUT							= 2,
	HEADER_GD_PLAYER_LOAD						= 3,
	HEADER_GD_PLAYER_SAVE						= 4,
	HEADER_GD_PLAYER_CREATE						= 5,
	HEADER_GD_PLAYER_DELETE						= 6,
	HEADER_GD_LOGIN_KEY							= 7,
	// 8 empty
	HEADER_GD_BOOT								= 9,
	HEADER_GD_PLAYER_COUNT						= 10,
	HEADER_GD_QUEST_SAVE						= 11,
	HEADER_GD_SAFEBOX_LOAD						= 12,
#ifdef ENABLE_SAFEBOX_MONEY
	HEADER_GD_SAFEBOX_SAVE						= 13,
#endif
	HEADER_GD_SAFEBOX_CHANGE_SIZE				= 14,
	HEADER_GD_EMPIRE_SELECT						= 15,
	HEADER_GD_SAFEBOX_CHANGE_PASSWORD			= 16,
	HEADER_GD_SAFEBOX_CHANGE_PASSWORD_SECOND	= 17, // Not really a packet, used internal
	HEADER_GD_DIRECT_ENTER						= 18,
	HEADER_GD_GUILD_SKILL_UPDATE				= 19,
	HEADER_GD_GUILD_EXP_UPDATE					= 20,
	HEADER_GD_GUILD_ADD_MEMBER					= 21,
	HEADER_GD_GUILD_REMOVE_MEMBER				= 22,
	HEADER_GD_GUILD_CHANGE_GRADE				= 23,
	HEADER_GD_GUILD_CHANGE_MEMBER_DATA			= 24,
	HEADER_GD_GUILD_DISBAND						= 25,
	HEADER_GD_GUILD_WAR							= 26,
	HEADER_GD_GUILD_WAR_SCORE					= 27,
	HEADER_GD_GUILD_CREATE						= 28,
	HEADER_GD_ITEM_SAVE							= 30,
	HEADER_GD_ITEM_DESTROY						= 31,
	HEADER_GD_ADD_AFFECT						= 32,
	HEADER_GD_REMOVE_AFFECT						= 33,
	HEADER_GD_HIGHSCORE_REGISTER				= 34,
	HEADER_GD_ITEM_FLUSH						= 35,
	HEADER_GD_PARTY_CREATE						= 36,
	HEADER_GD_PARTY_DELETE						= 37,
	HEADER_GD_PARTY_ADD							= 38,
	HEADER_GD_PARTY_REMOVE						= 39,
	HEADER_GD_PARTY_STATE_CHANGE				= 40,
	HEADER_GD_PARTY_HEAL_USE					= 41,
	HEADER_GD_FLUSH_CACHE						= 42,
	HEADER_GD_RELOAD_PROTO						= 43,
	HEADER_GD_CHANGE_NAME						= 44,
	HEADER_GD_SMS								= 45,
	HEADER_GD_GUILD_CHANGE_LADDER_POINT			= 46,
	HEADER_GD_GUILD_USE_SKILL					= 47,
	HEADER_GD_REQUEST_EMPIRE_PRIV				= 48,
	HEADER_GD_REQUEST_GUILD_PRIV				= 49,
	HEADER_GD_MONEY_LOG							= 50,
	HEADER_GD_GUILD_DEPOSIT_MONEY				= 51,
	HEADER_GD_GUILD_WITHDRAW_MONEY				= 52,
	HEADER_GD_GUILD_WITHDRAW_MONEY_GIVE_REPLY	= 53,
	HEADER_GD_REQUEST_CHARACTER_PRIV			= 54,
	HEADER_GD_SET_EVENT_FLAG					= 55,
	HEADER_GD_PARTY_SET_MEMBER_LEVEL			= 56,
	HEADER_GD_GUILD_WAR_BET						= 57,
	HEADER_GD_CREATE_OBJECT						= 60,
	HEADER_GD_DELETE_OBJECT						= 61,
	HEADER_GD_UPDATE_LAND						= 62,
	HEADER_GD_MARRIAGE_ADD						= 70,
	HEADER_GD_MARRIAGE_UPDATE					= 71,
	HEADER_GD_MARRIAGE_REMOVE					= 72,
	HEADER_GD_WEDDING_REQUEST					= 73,
	HEADER_GD_WEDDING_READY						= 74,
	HEADER_GD_WEDDING_END						= 75,
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	HEADER_GD_SHOP								= 77,
	HEADER_GD_SAVE_SHOP							= 78,
	HEADER_GD_WITHDRAW_SHOP_GOLD				= 79,
#endif
	HEADER_GD_AUTH_LOGIN						= 100,
	HEADER_GD_LOGIN_BY_KEY						= 101,
	HEADER_GD_MALL_LOAD							= 107,
	HEADER_GD_MYSHOP_PRICELIST_UPDATE			= 108,
	HEADER_GD_MYSHOP_PRICELIST_REQ				= 109,
	HEADER_GD_BLOCK_CHAT						= 110,
	HEADER_GD_HAMMER_OF_TOR						= 114,
	HEADER_GD_RELOAD_ADMIN						= 115,
	HEADER_GD_BREAK_MARRIAGE					= 116,
	HEADER_GD_ELECT_MONARCH						= 117,
	HEADER_GD_CANDIDACY							= 118,
	HEADER_GD_ADD_MONARCH_MONEY					= 119,
	HEADER_GD_TAKE_MONARCH_MONEY				= 120,
	HEADER_GD_COME_TO_VOTE						= 121,
	HEADER_GD_RMCANDIDACY						= 122,
	HEADER_GD_SETMONARCH						= 123,
	HEADER_GD_RMMONARCH							= 124,
	HEADER_GD_DEC_MONARCH_MONEY					= 125,
	HEADER_GD_CHANGE_MONARCH_LORD				= 126,
	HEADER_GD_BLOCK_COUNTRY_IP					= 127,
	HEADER_GD_BLOCK_EXCEPTION					= 128,
	HEADER_GD_REQ_CHANGE_GUILD_MASTER			= 129,
	HEADER_GD_REQ_SPARE_ITEM_ID_RANGE			= 130,
	HEADER_GD_UPDATE_HORSE_NAME					= 131,
	HEADER_GD_REQ_HORSE_NAME					= 132,
	HEADER_GD_DC								= 133,
	HEADER_GD_VALID_LOGOUT						= 134,
	HEADER_GD_REQUEST_CHARGE_CASH				= 137,
	HEADER_GD_DELETE_AWARDID					= 138,
	HEADER_GD_UPDATE_CHANNELSTATUS				= 139,
	HEADER_GD_REQUEST_CHANNELSTATUS				= 140,
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	HEADER_GD_GUILD_DUNGEON						= 141,
	HEADER_GD_GUILD_DUNGEON_CD					= 142,
#endif
#ifdef ENABLE_MOVE_CHANNEL
	HEADER_GD_MOVE_CHANNEL						= 143,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	HEADER_GD_ACHIEVEMENT						= 144,
#endif
#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && !defined(ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE)
	HEADER_GD_PLAYER_LOGOUT						= 145,
#else
	HEADER_GD_PLAYER_SHOP_TIMER					= 145,
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	HEADER_GD_REQUEST_SUNGMA_PREMIUM			= 146,
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	HEADER_GD_LANGUAGE_CHANGE					= 149,
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	HEADER_GD_GUILDSTORAGE_LOAD					= 150,
	HEADER_GD_GUILDSTORAGE_CHANGE_SIZE			= 151,
#endif
#ifdef ENABLE_MAILBOX
	HEADER_GD_MAILBOX_LOAD						= 152,
	HEADER_GD_MAILBOX_CHECK_NAME				= 153,
	HEADER_GD_MAILBOX_WRITE						= 154,
	HEADER_GD_MAILBOX_DELETE					= 155,
	HEADER_GD_MAILBOX_CONFIRM					= 156,
	HEADER_GD_MAILBOX_GET						= 157,
	HEADER_GD_MAILBOX_UNREAD					= 158,
#endif
#ifdef ENABLE_EVENT_MANAGER
	HEADER_GD_UPDATE_EVENT_STATUS				= 159,
	HEADER_GD_EVENT_NOTIFICATION				= 160,
	HEADER_GD_RELOAD_EVENT						= 161,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_GD_SAVE_EXT_BATTLE_PASS				= 162,
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_GD_SKILL_COLOR_SAVE					= 170,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_GD_PET_ITEM_DESTROY					= 171,
#endif
	HEADER_GD_SETUP								= 0xff,	//255

	///////////////////////////////////////////////
	HEADER_DG_NOTICE							= 1,
	HEADER_DG_LOGIN_SUCCESS						= 30,
	HEADER_DG_LOGIN_NOT_EXIST					= 31,
	HEADER_DG_LOGIN_WRONG_PASSWD				= 33,
	HEADER_DG_LOGIN_ALREADY						= 34,
	HEADER_DG_PLAYER_LOAD_SUCCESS				= 35,
	HEADER_DG_PLAYER_LOAD_FAILED				= 36,
	HEADER_DG_PLAYER_CREATE_SUCCESS				= 37,
	HEADER_DG_PLAYER_CREATE_ALREADY				= 38,
	HEADER_DG_PLAYER_CREATE_FAILED				= 39,
	HEADER_DG_PLAYER_DELETE_SUCCESS				= 40,
	HEADER_DG_PLAYER_DELETE_FAILED				= 41,
	HEADER_DG_ITEM_LOAD							= 42,
	HEADER_DG_BOOT								= 43,
	HEADER_DG_QUEST_LOAD						= 44,
	HEADER_DG_SAFEBOX_LOAD						= 45,
	HEADER_DG_SAFEBOX_CHANGE_SIZE				= 46,
	HEADER_DG_SAFEBOX_WRONG_PASSWORD			= 47,
	HEADER_DG_SAFEBOX_CHANGE_PASSWORD_ANSWER	= 48,
	HEADER_DG_EMPIRE_SELECT						= 49,
	HEADER_DG_AFFECT_LOAD						= 50,
	HEADER_DG_MALL_LOAD							= 51,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	HEADER_DG_GUILDSTORAGE_LOAD					= 52,
	HEADER_DG_GUILDSTORAGE_CHANGE_SIZE			= 53,
#endif
	HEADER_DG_DIRECT_ENTER						= 55,
	HEADER_DG_GUILD_SKILL_UPDATE				= 56,
	HEADER_DG_GUILD_SKILL_RECHARGE				= 57,
	HEADER_DG_GUILD_EXP_UPDATE					= 58,
	HEADER_DG_PARTY_CREATE						= 59,
	HEADER_DG_PARTY_DELETE						= 60,
	HEADER_DG_PARTY_ADD							= 61,
	HEADER_DG_PARTY_REMOVE						= 62,
	HEADER_DG_PARTY_STATE_CHANGE				= 63,
	HEADER_DG_PARTY_HEAL_USE					= 64,
	HEADER_DG_PARTY_SET_MEMBER_LEVEL			= 65,
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	HEADER_DG_SHOP_MYINFO_LOAD					= 68,
	HEADER_DG_SHOP_SALE_INFO					= 69,
	HEADER_DG_SHOP_WITHDRAW_RESULT				= 70,
	HEADER_DG_PLAYER_SPAWN_SHOP					= 71,
#	ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
	HEADER_DG_SHOP_STOP_OFFLINE					= 72,
#	endif
	HEADER_DG_SHOP_TIMER						= 73,
	HEADER_DG_SHOP_CLOSE						= 74,
	HEADER_DG_SHOP_UPDATE_NAME					= 75,
#endif
	HEADER_DG_TIME								= 90,
	HEADER_DG_ITEM_ID_RANGE						= 91,
	HEADER_DG_GUILD_ADD_MEMBER					= 92,
	HEADER_DG_GUILD_REMOVE_MEMBER				= 93,
	HEADER_DG_GUILD_CHANGE_GRADE				= 94,
	HEADER_DG_GUILD_CHANGE_MEMBER_DATA			= 95,
	HEADER_DG_GUILD_DISBAND						= 96,
	HEADER_DG_GUILD_WAR							= 97,
	HEADER_DG_GUILD_WAR_SCORE					= 98,
	HEADER_DG_GUILD_TIME_UPDATE					= 99,
	HEADER_DG_GUILD_LOAD						= 100,
	HEADER_DG_GUILD_LADDER						= 101,
	HEADER_DG_GUILD_SKILL_USABLE_CHANGE			= 102,
	HEADER_DG_GUILD_MONEY_CHANGE				= 103,
#ifndef ENABLE_USE_MONEY_FROM_GUILD
	HEADER_DG_GUILD_WITHDRAW_MONEY_GIVE			= 104,
#endif
	HEADER_DG_SET_EVENT_FLAG					= 105,
	HEADER_DG_GUILD_WAR_RESERVE_ADD				= 106,
	HEADER_DG_GUILD_WAR_RESERVE_DEL				= 107,
	HEADER_DG_GUILD_WAR_BET						= 108,
	HEADER_DG_RELOAD_PROTO						= 120,
	HEADER_DG_CHANGE_NAME						= 121,
	HEADER_DG_AUTH_LOGIN						= 122,
	HEADER_DG_CHANGE_EMPIRE_PRIV				= 124,
	HEADER_DG_CHANGE_GUILD_PRIV					= 125,
	HEADER_DG_MONEY_LOG							= 126,
	HEADER_DG_CHANGE_CHARACTER_PRIV				= 127,
	HEADER_DG_CREATE_OBJECT						= 140,
	HEADER_DG_DELETE_OBJECT						= 141,
	HEADER_DG_UPDATE_LAND						= 142,
	HEADER_DG_MARRIAGE_ADD						= 150,
	HEADER_DG_MARRIAGE_UPDATE					= 151,
	HEADER_DG_MARRIAGE_REMOVE					= 152,
	HEADER_DG_WEDDING_REQUEST					= 153,
	HEADER_DG_WEDDING_READY						= 154,
	HEADER_DG_WEDDING_START						= 155,
	HEADER_DG_WEDDING_END						= 156,
	HEADER_DG_MYSHOP_PRICELIST_RES				= 157,
	HEADER_DG_RELOAD_ADMIN						= 158,
	HEADER_DG_BREAK_MARRIAGE					= 159,
	HEADER_DG_ELECT_MONARCH						= 160,
	HEADER_DG_CANDIDACY							= 161,
	HEADER_DG_ADD_MONARCH_MONEY					= 162,
	HEADER_DG_TAKE_MONARCH_MONEY				= 163,
	HEADER_DG_COME_TO_VOTE						= 164,
	HEADER_DG_RMCANDIDACY						= 165,
	HEADER_DG_SETMONARCH						= 166,
	HEADER_DG_RMMONARCH							= 167,
	HEADER_DG_DEC_MONARCH_MONEY					= 168,
	HEADER_DG_CHANGE_MONARCH_LORD_ACK			= 169,
	HEADER_DG_UPDATE_MONARCH_INFO				= 170,
	HEADER_DG_BLOCK_COUNTRY_IP					= 171,
	HEADER_DG_BLOCK_EXCEPTION					= 172,
	HEADER_DG_ACK_CHANGE_GUILD_MASTER			= 173,
	HEADER_DG_ACK_SPARE_ITEM_ID_RANGE			= 174,
	HEADER_DG_UPDATE_HORSE_NAME 				= 175,
	HEADER_DG_ACK_HORSE_NAME					= 176,
	HEADER_DG_NEED_LOGIN_LOG					= 177,
	HEADER_DG_RESULT_CHARGE_CASH				= 179,
	HEADER_DG_ITEMAWARD_INFORMER				= 180,	//gift notify
	HEADER_DG_RESPOND_CHANNELSTATUS				= 181,
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	HEADER_DG_GUILD_DUNGEON						= 182,
	HEADER_DG_GUILD_DUNGEON_CD					= 183,
#endif
#ifdef ENABLE_MOVE_CHANNEL
	HEADER_DG_RESPOND_MOVE_CHANNEL				= 184,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	HEADER_DG_ACHIEVEMENT						= 185,
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_DG_SKILL_COLOR_LOAD					= 186,
#endif
#ifdef ENABLE_MAILBOX
	HEADER_DG_RESPOND_MAILBOX_LOAD				= 187,
	HEADER_DG_RESPOND_MAILBOX_CHECK_NAME		= 188,
	HEADER_DG_RESPOND_MAILBOX_UNREAD			= 189,
#endif
#ifdef ENABLE_EVENT_MANAGER
	HEADER_DG_UPDATE_EVENT_STATUS				= 190,
	HEADER_DG_EVENT_NOTIFICATION				= 191,
	HEADER_DG_RELOAD_EVENT						= 192,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_DG_EXT_BATTLE_PASS_LOAD				= 193,
#endif
#ifdef ENABLE_WEATHER_INFO
	HEADER_DG_WEATHER_INFO						= 195,
#endif
	HEADER_DG_MAP_LOCATIONS						= 0xfe,	//254
	HEADER_DG_P2P								= 0xff,	//255
};

#ifdef ENABLE_FISH_EVENT
typedef struct SPlayerFishEventSlot
{
	bool bIsMain;
	uint8_t bShape;
} TPlayerFishEventSlot;
#endif

/* ----------------------------------------------
* table
* ----------------------------------------------
*/

/* game Server -> DB Server */
#pragma pack(1)
enum ERequestChargeType
{
	ERequestCharge_Cash = 0,
	ERequestCharge_Mileage,
};

typedef struct SRequestChargeCash
{
	uint32_t dwAID; // id(primary key) - Account Table
	uint32_t dwAmount;
	ERequestChargeType eChargeType;

} TRequestChargeCash;

typedef struct SSimplePlayer
{
	uint32_t dwID;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t byJob;
	uint8_t byLevel;
#ifdef ENABLE_YOHARA_SYSTEM
	uint8_t bycLevel;
#endif
	uint32_t dwPlayMinutes;
	uint8_t byST, byHT, byDX, byIQ;
#ifdef ENABLE_YOHARA_SYSTEM
	uint8_t bySungST, bySungHT, bySungDX, bySungIQ;
#endif
	uint16_t wMainPart;
	uint8_t bChangeName;
	uint16_t wHairPart;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	uint32_t wAccePart;
#endif
#ifdef ENABLE_AURA_SYSTEM
	uint32_t wAuraPart;
#endif
#ifdef NEW_SELECT_CHARACTER
	uint32_t dwLastPlayTime;
#endif
	uint8_t bDummy[4];
	long x, y;
	long lAddr;
	uint16_t wPort;
	uint8_t skill_group;
} TSimplePlayer;

typedef struct SAccountTable
{
	uint32_t id;
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
#ifdef ENABLE_HWID_BAN
	char hwid[HWID_MAX_LEN + 1];
#endif
	char social_id[SOCIAL_ID_MAX_LEN + 1];
	char status[ACCOUNT_STATUS_MAX_LEN + 1];
	uint8_t bEmpire;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
	TSimplePlayer players[PLAYER_PER_ACCOUNT];
} TAccountTable;

typedef struct SPacketDGCreateSuccess
{
	uint8_t bAccountCharacterIndex;
	TSimplePlayer player;
} TPacketDGCreateSuccess;

typedef struct TPlayerItemAttribute
{
	uint16_t wType; //@fixme532
	int16_t sValue;
} TPlayerItemAttribute;

#ifdef ENABLE_YOHARA_SYSTEM
typedef struct TPlayerItemApplyRandom
{
	uint16_t wType; //@fixme532
	int16_t sValue;
	uint8_t row;
} TPlayerItemApplyRandom;

typedef struct SSungmaTable
{
	long lMapIdx;
	uint8_t bSungmaStr;
	uint8_t bSungmaHp;
	uint8_t bSungmaMove;
	uint8_t bSungmaImmune;
#ifdef ENABLE_PRECISION
	uint8_t bHitPct;
#endif
} TSungmaTable;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
typedef struct SGrowthPetInfo
{
	uint32_t pet_id;
	uint32_t pet_vnum;
	bool is_summoned;
	uint8_t flash_event;
	char pet_nick[25]; // CHARACTER_NAME_MAX_LEN + 1
	uint8_t pet_level;
	uint8_t evol_level;
	uint8_t pet_type;
	float pet_hp;
	float pet_def;
	float pet_sp;
	time_t pet_birthday;
	time_t pet_end_time;
	time_t pet_max_time;
	uint32_t exp_monster;
	uint32_t exp_item;
	uint32_t next_exp_monster;
	uint32_t next_exp_item;
	uint8_t skill_count;
	uint8_t skill_vnum[3];
	uint8_t skill_level[3];
	float skill_spec[3];
	time_t skill_cool[3];
	float skill_formula1[3];
	float next_skill_formula1[3];
	float skill_formula2[3];
	float next_skill_formula2[3];
} TGrowthPetInfo;
#endif

typedef struct SPlayerItem
{
	uint32_t id;
	uint8_t window;
	uint16_t pos;
	uint32_t count;
	uint32_t vnum;
#ifdef ENABLE_SEALBIND_SYSTEM
	long nSealDate;
#endif
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
	uint32_t owner;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	bool is_basic;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t grade_element;
	uint32_t attack_element[MAX_ELEMENTS_SPELL];
	uint8_t element_type_bonus;
	int16_t elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_SET_ITEM
	uint8_t set_value;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	TGrowthPetInfo aPetInfo;
#endif
} TPlayerItem;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct SMyShopPriceInfo
{
	uint16_t pos;
	uint32_t price;
# ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	uint32_t cheque;
# endif
} TMyShopPriceInfo;
#endif

typedef struct SQuickslot
{
	uint8_t type;
	uint8_t pos;
} TQuickSlot;

typedef struct SPlayerSkill
{
	uint8_t bMasterType;
	uint8_t bLevel;
	time_t tNextRead;
} TPlayerSkill;

struct THorseInfo
{
	uint8_t bLevel;
	uint8_t bRiding;
	int16_t sStamina;
	int16_t sHealth;
	uint32_t dwHorseHealthDropTime;
};

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
typedef struct SPlayerGemItems
{
	uint8_t bItemId;
	uint8_t bSlotStatus;
	uint8_t bSlotUnlocked;
} TPlayerGemItems;

typedef struct SGemShopTable
{
	uint32_t dwVnum;
	uint8_t bCount;
	uint32_t dwPrice;
	uint32_t dwRow;
} TGemShopTable;

typedef struct packet_gemshop_item
{
	uint32_t vnum;
	uint32_t price;
	uint8_t count;
	uint8_t display_pos;

	uint8_t status;
	uint8_t unlocked;
} TGemShopItem;
#endif

#ifdef ENABLE_FLOWER_EVENT
typedef struct SFlowerTable
{
	uint32_t value;
} TFlowerEvent;
#endif

typedef struct SPlayerTable
{
	uint32_t id;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	char ip[IP_ADDRESS_LENGTH + 1];
	uint16_t job;
	uint8_t voice;
	uint8_t level;
	uint8_t level_step;
	int16_t st, ht, dx, iq;
	uint32_t exp;
	INT gold;
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
	uint8_t dir;
	INT x, y, z;
	INT lMapIndex;
	long lExitX, lExitY;
	long lExitMapIndex;

	// @fixme301
	int hp;
	int sp;

	int16_t sRandomHP;
	int16_t sRandomSP;
#ifdef ENABLE_HORSE_APPEARANCE
	uint32_t sHorse_appearance;
#endif
	int playtime;
	int16_t stat_point;
	int16_t skill_point;
	int16_t sub_skill_point;
	int16_t horse_skill_point;
	TPlayerSkill skills[SKILL_MAX_NUM];
	TQuickSlot quickslot[QUICKSLOT_MAX_NUM];
	uint8_t part_base;
	uint32_t parts[PART_MAX_NUM]; //@fixme479 - uint16_t -> uint32_t
	int16_t stamina;
	uint8_t skill_group;
	long lAlignment;
	char szMobile[MOBILE_MAX_LEN + 1];
	int16_t stat_reset_count;
	THorseInfo horse;
	uint32_t logoff_interval;
	int aiPremiumTimes[PREMIUM_MAX_NUM];
#ifdef ENABLE_GEM_SYSTEM
	int gem_point;
# if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	int gem_next_refresh;
	TPlayerGemItems gemItems[GEM_SHOP_HOST_ITEM_MAX_NUM];
# endif
#endif
#ifdef ENABLE_BATTLE_FIELD
	int battle_point;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	int medal_honor;
#endif
#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
	int aiShopExUsablePoint[SHOP_COIN_TYPE_MAX];
	int aiShopExDailyUse[SHOP_COIN_TYPE_MAX];
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	uint8_t conqueror_level;
	uint8_t conqueror_level_step;
	int16_t conqueror_st, conqueror_ht, conqueror_mov, conqueror_imu;
	uint32_t conqueror_exp;
	int16_t conqueror_point;
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	uint8_t m_BiologActualMission;
	uint16_t m_BiologCollectedItems;
	uint8_t m_BiologCooldownReminder;
	long m_BiologCooldown;
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	int inventory_stage;
#endif
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	int special_stage[3];
# endif
#ifdef NEW_SELECT_CHARACTER
	uint32_t last_playtime;
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	INT soul;
	INT soulre;
#endif
#ifdef ENABLE_FISH_EVENT
	uint32_t fishEventUseCount;
	TPlayerFishEventSlot fishSlots[FISH_EVENT_SLOTS_NUM];
#endif
#ifdef ENABLE_FLOWER_EVENT
	TFlowerEvent flowerEvent[PART_FLOWER_MAX_NUM];
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	int	battle_pass_premium_id;
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	uint8_t page_equipment;
	uint8_t unlock_page_equipment;
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	long long lotto_moneypool;
	long long lotto_totalmoneywin;
#endif
} TPlayerTable;

typedef struct SMobSkillLevel
{
	uint32_t dwVnum;
	uint8_t bLevel;
} TMobSkillLevel;

typedef struct SEntityTable
{
	uint32_t dwVnum;
} TEntityTable;

typedef struct SMobTable : public SEntityTable
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	char szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

	uint8_t bType; // Monster, NPC
	uint8_t bRank; // PAWN, KNIGHT, KING
	uint8_t bBattleType; // MELEE, etc..
	uint8_t bLevel; // Level
#ifdef ENABLE_PROTO_RENEWAL
	uint8_t bScale;
#endif
	uint8_t bSize;

	uint32_t dwGoldMin;
	uint32_t dwGoldMax;
	uint32_t dwExp;
#ifdef ENABLE_PROTO_RENEWAL
	uint32_t dwSungMaExp;
#endif
	uint32_t dwMaxHP;
	uint8_t bRegenCycle;
	uint8_t bRegenPercent;
	uint16_t wDef;

	uint32_t dwAIFlag;
#ifdef ENABLE_PROTO_RENEWAL
	uint32_t dwAIFlagEx;
#endif
	uint32_t dwRaceFlag;
	uint32_t dwImmuneFlag;

	uint8_t bStr;
	uint8_t bDex;
	uint8_t bCon;
	uint8_t bInt;
#ifdef ENABLE_PROTO_RENEWAL
	uint8_t bSungMaSt;
	uint8_t bSungMaDx;
	uint8_t bSungMaHt;
	uint8_t bSungMaIq;
#endif
	uint32_t dwDamageRange[2];

	uint16_t sAttackSpeed;
	uint16_t sMovingSpeed;
	uint8_t bAggresiveHPPct;
	uint16_t wAggressiveSight;
	uint16_t wAttackRange;

	char cEnchants[MOB_ENCHANTS_MAX_NUM];
	char cResists[MOB_RESISTS_MAX_NUM];
#ifdef ENABLE_PROTO_RENEWAL
	char cElementalFlags[MOB_ELEMENTAL_MAX_NUM];
	char cResistDark, cResistIce, cResistEarth;
#endif

	uint32_t dwResurrectionVnum;
	uint32_t dwDropItemVnum;

	uint8_t bMountCapacity;
	uint8_t bOnClickType;

	uint8_t bEmpire;
	char szFolder[CHARACTER_FOLDER_MAX_LEN + 1];

	float fDamMultiply;

	uint32_t dwSummonVnum;
	uint32_t dwDrainSP;
	uint32_t dwMobColor;
	uint32_t dwPolymorphItemVnum;

	TMobSkillLevel Skills[MOB_SKILL_MAX_NUM];

	uint8_t bBerserkPoint;
	uint8_t bStoneSkinPoint;
	uint8_t bGodSpeedPoint;
	uint8_t bDeathBlowPoint;
	uint8_t bRevivePoint;

#ifdef ENABLE_PROTO_RENEWAL
	uint8_t bHealPoint;
	uint8_t bRAttSpeedPoint;
	uint8_t bRCastSpeedPoint;
	uint8_t bRHPRegenPoint;
	float fHitRange;
#endif
} TMobTable;

typedef struct SSkillTable
{
	uint32_t dwVnum;
	char szName[32 + 1];
	uint8_t bType;
	uint8_t bMaxLevel;
	uint32_t dwSplashRange;

	char szPointOn[64];
	char szPointPoly[100 + 1];
	char szSPCostPoly[100 + 1];
	char szDurationPoly[100 + 1];
	char szDurationSPCostPoly[100 + 1];
	char szCooldownPoly[100 + 1];
	char szMasterBonusPoly[100 + 1];
	//char szAttackGradePoly[100 + 1];
	char szGrandMasterAddSPCostPoly[100 + 1];
	uint32_t dwFlag;
	uint32_t dwAffectFlag;

	// Data for secondary skill
	char szPointOn2[64];
	char szPointPoly2[100 + 1];
	char szDurationPoly2[100 + 1];
	uint32_t dwAffectFlag2;

	// Data for grand master point
	char szPointOn3[64];
	char szPointPoly3[100 + 1];
	char szDurationPoly3[100 + 1];

#ifdef ENABLE_SKILL_TABLE_RENEWAL
	char szPointOn4[64];
	char szPointPoly4[100 + 1];
	char szDurationPoly4[100 + 1];
#endif

	uint8_t bLevelStep;
	uint8_t bLevelLimit;
	uint32_t preSkillVnum;
	uint8_t preSkillLevel;

	long lMaxHit;
	char szSplashAroundDamageAdjustPoly[100 + 1];

	uint8_t bSkillAttrType;

	uint32_t dwTargetRange;
} TSkillTable;

typedef struct SShopItemTable
{
	uint32_t vnum;
	uint8_t count;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
	TItemPos pos;
	uint32_t price;
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	uint8_t limit_level;
#endif
	uint8_t display_pos;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	SShopItemTable() : vnum(0), count(0),
		pos(0, 0),
		price(0),
# ifdef ENABLE_CHEQUE_SYSTEM
		cheque(0),
# endif
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
		dwTransmutationVnum(0),
# endif
# ifdef ENABLE_MEDAL_OF_HONOR
		limit_level(0),
# endif
		display_pos(0) {}

	bool operator!= (const struct SShopItemTable& other)
	{
		return (vnum != other.vnum || count != other.count ||
			pos != other.pos ||
			price != other.price ||
# ifdef ENABLE_CHEQUE_SYSTEM
			cheque != other.cheque ||
# endif
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
			dwTransmutationVnum != other.dwTransmutationVnum ||
# endif
# ifdef ENABLE_MEDAL_OF_HONOR
			limit_level != other.limit_level ||
# endif
			display_pos != other.display_pos);
	}
#endif
} TShopItemTable;

#ifdef ENABLE_CHEQUE_SYSTEM
typedef struct SItemPriceType
{
	SItemPriceType(){ dwPrice = dwCheque = 0; }
	SItemPriceType(uint32_t gold, uint32_t cheque)
	{
		dwPrice = gold;
		dwCheque = cheque;
	}
	uint32_t dwPrice;
	uint32_t dwCheque;
} TItemPriceType;
#endif

typedef struct SShopTable
{
	uint32_t dwVnum;
	uint32_t dwNPCVnum;

	uint8_t byItemCount;
#ifdef ENABLE_MYSHOP_DECO
	TShopItemTable items[SHOP_HOST_ITEM_MAX];
#else
	TShopItemTable items[SHOP_HOST_ITEM_MAX_NUM];
#endif
#ifdef ENABLE_12ZI_SHOP_LIMIT
	uint8_t is_limited;
#endif
} TShopTable;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct SPlayerShopTable
{
	uint32_t pid;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	char shopName[SHOP_SIGN_MAX_LEN + 1];
# ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	uint32_t level;
//#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t clevel;
//#endif
	uint32_t alignment;
	uint32_t empire;
	uint32_t guild;
# endif
	long x;
	long y;
	long mapIndex;
	int channel;
	uint32_t openTime;
	bool closed;
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	uint32_t dwPolyVnum;
	uint8_t bTitleType;
	bool isShopDeco;
	TShopItemTable items[SHOP_HOST_ITEM_MAX];
#else
	TShopItemTable items[SHOP_HOST_ITEM_MAX_NUM];
#endif

	bool operator!= (const struct SPlayerShopTable& other)
	{
		if (pid != other.pid || strcmp(shopName, other.shopName) != 0 ||
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
			level != other.level ||
//#ifdef ENABLE_YOHARA_SYSTEM
//			clevel != other.clevel ||
//#endif
			alignment != other.alignment ||
			empire != other.empire || guild != other.guild ||
#endif
			x != other.x || y != other.y || channel != other.channel ||
			mapIndex != other.mapIndex || closed != other.closed ||
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
			dwPolyVnum != other.dwPolyVnum || bTitleType != other.bTitleType || isShopDeco != other.isShopDeco ||
#endif
			openTime != other.openTime)
			return true;

		for (size_t i = 0; i < sizeof(items) / sizeof(TShopItemTable); ++i)
		{
			if (items[i] != other.items[i])
				return true;
		}

		return false;
	}
} TPlayerShopTable;

typedef struct SPlayerShopTableCache : TPlayerShopTable
{
	uint32_t goldStash;
# ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	uint32_t chequeStash;
# endif
	int offlineMinutesLeft;
} TPlayerShopTableCache;
#endif

#define QUEST_NAME_MAX_LEN 32
#define QUEST_STATE_MAX_LEN 64

typedef struct SQuestTable
{
	uint32_t dwPID;
	char szName[QUEST_NAME_MAX_LEN + 1];
	char szState[QUEST_STATE_MAX_LEN + 1];
	long lValue;
} TQuestTable;

typedef struct SItemLimit
{
	uint8_t bType;
	long lValue;
} TItemLimit;

typedef struct SItemApply
{
	uint16_t wType; //@fixme532
	long lValue;
} TItemApply;

typedef struct SItemTable : public SEntityTable
{
	uint32_t dwVnumRange;
	char szName[ITEM_NAME_MAX_LEN + 1];
	char szLocaleName[ITEM_NAME_MAX_LEN + 1];
	uint8_t bType;
	uint8_t bSubType;

	uint8_t bWeight;
	uint8_t bSize;

	uint32_t dwAntiFlags;
	uint32_t dwFlags;
	uint32_t dwWearFlags;
	uint32_t dwImmuneFlag;

#ifdef ENABLE_PROTO_RENEWAL
	uint32_t dwShopBuyPrice;
	uint32_t dwShopSellPrice;
#else
	uint32_t dwGold;
	uint32_t dwShopBuyPrice;
#endif

	TItemLimit aLimits[ITEM_LIMIT_MAX_NUM];
	TItemApply aApplies[ITEM_APPLY_MAX_NUM];
	long alValues[ITEM_VALUES_MAX_NUM];
	long alSockets[ITEM_SOCKET_MAX_NUM];

#ifdef ENABLE_PROTO_RENEWAL
	uint32_t dwRefineElementApplyType;
	uint32_t dwRefineElementGrade;
	uint32_t dwRefineElementValue;
	uint32_t dwRefineElementBonus;
#endif

	uint32_t dwRefinedVnum;
	uint16_t wRefineSet;
#ifdef ENABLE_PROTO_RENEWAL
	uint32_t dwMaterial67;
#endif
	uint8_t bAlterToMagicItemPct;
	uint8_t bSpecular;
	uint8_t bGainSocketPct;

#ifdef ENABLE_PROTO_RENEWAL
	uint8_t bMaskType;
	uint8_t bMaskSubType;
#endif

	int16_t sAddonType; // Basic attribute

	// The limit flags below are often checked in realtime, and are fixed values per item VNUM.
	// As per the current structure, if necessary for each item, the load to check while looping up to LIMIT_MAX_NUM is large, so save it in advance.
	char cLimitRealTimeFirstUseIndex; // The position of the LIMIT_REAL_TIME_FIRST_USE flag among the item limit field values (-1 if none)
	char cLimitTimerBasedOnWearIndex; // The position of the LIMIT_TIMER_BASED_ON_WEAR flag among the item limit field values (-1 if none)
} TItemTable;

struct TItemAttrTable
{
	TItemAttrTable() noexcept :
		dwApplyIndex(0),
		dwProb(0)
	{
		szApply[0] = 0;
		memset(&lValues, 0, sizeof(lValues));
		memset(&bMaxLevelBySet, 0, sizeof(bMaxLevelBySet));
	}

	char szApply[APPLY_NAME_MAX_LEN + 1];
	uint32_t dwApplyIndex;
	uint32_t dwProb;
	long lValues[ITEM_ATTRIBUTE_MAX_LEVEL];
	uint8_t bMaxLevelBySet[ATTRIBUTE_SET_MAX_NUM];
};

typedef struct SConnectTable
{
	char login[LOGIN_MAX_LEN + 1];
	IDENT ident;
} TConnectTable;

typedef struct SLoginPacket
{
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
} TLoginPacket;

typedef struct SPlayerLoadPacket
{
	uint32_t account_id;
	uint32_t player_id;
	uint8_t account_index;
} TPlayerLoadPacket;

typedef struct SPlayerCreatePacket
{
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
	uint32_t account_id;
	uint8_t account_index;
	TPlayerTable player_table;
} TPlayerCreatePacket;

typedef struct SPlayerDeletePacket
{
	char login[LOGIN_MAX_LEN + 1];
	uint32_t player_id;
	uint8_t account_index;
	//char name[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t account_id;
	char private_code[8];
} TPlayerDeletePacket;

typedef struct SLogoutPacket
{
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
} TLogoutPacket;

typedef struct SPlayerCountPacket
{
	uint32_t dwCount;
} TPlayerCountPacket;

#define SAFEBOX_MAX_NUM 135
#define SAFEBOX_PASSWORD_MAX_LEN 6

typedef struct SSafeboxTable
{
	uint32_t dwID;
	uint8_t bSize;
#ifdef ENABLE_SAFEBOX_MONEY
	uint32_t dwGold;
#endif
	uint16_t wItemCount;
} TSafeboxTable;

typedef struct SSafeboxChangeSizePacket
{
	uint32_t dwID;
	uint8_t bSize;
} TSafeboxChangeSizePacket;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
typedef struct SGuildstorageChangeSizePacket
{
	uint32_t dwID;
	uint8_t bSize;
} TGuildstorageChangeSizePacket;
#endif

typedef struct SSafeboxLoadPacket
{
	uint32_t dwID;
	char szLogin[LOGIN_MAX_LEN + 1];
	char szPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxLoadPacket;

typedef struct SSafeboxChangePasswordPacket
{
	uint32_t dwID;
	char szOldPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
	char szNewPassword[SAFEBOX_PASSWORD_MAX_LEN + 1];
} TSafeboxChangePasswordPacket;

typedef struct SSafeboxChangePasswordPacketAnswer
{
	uint8_t flag;
} TSafeboxChangePasswordPacketAnswer;

#ifdef ENABLE_MOVE_CHANNEL
typedef struct SMoveChannel
{
	uint8_t bChannel;
	long lMapIndex;
} TMoveChannel;

typedef struct SRespondMoveChannel
{
	uint16_t wPort;
	long lAddr;
} TRespondMoveChannel;
#endif

typedef struct SEmpireSelectPacket
{
	uint32_t dwAccountID;
	uint8_t bEmpire;
} TEmpireSelectPacket;

typedef struct SPacketGDSetup
{
	char szPublicIP[16]; // Public IP which listen to users
	uint8_t bChannel;
	uint16_t wListenPort;
	uint16_t wP2PPort;
	long alMaps[MAP_ALLOW_LIMIT];
	uint32_t dwLoginCount;
	uint8_t bAuthServer;
} TPacketGDSetup;

typedef struct SPacketDGMapLocations
{
	uint8_t bCount;
} TPacketDGMapLocations;

typedef struct SMapLocation
{
	long alMaps[MAP_ALLOW_LIMIT];
	char szHost[MAX_HOST_LENGTH + 1];
	uint16_t wPort;
} TMapLocation;

typedef struct SPacketDGP2P
{
	char szHost[MAX_HOST_LENGTH + 1];
	uint16_t wPort;
	uint8_t bChannel;
} TPacketDGP2P;

typedef struct SPacketGDDirectEnter
{
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
	uint8_t index;
} TPacketGDDirectEnter;

typedef struct SPacketDGDirectEnter
{
	TAccountTable accountTable;
	TPlayerTable playerTable;
} TPacketDGDirectEnter;

typedef struct SPacketGuildSkillUpdate
{
	uint32_t guild_id;
	int amount;
	uint8_t skill_levels[12];
	uint8_t skill_point;
	uint8_t save;
} TPacketGuildSkillUpdate;

typedef struct SPacketGuildExpUpdate
{
	uint32_t guild_id;
	int amount;
} TPacketGuildExpUpdate;

typedef struct SPacketGuildChangeMemberData
{
	uint32_t guild_id;
	uint32_t pid;
	uint32_t offer;
	uint8_t level;
	uint8_t grade;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	uint32_t join_date;
	uint8_t donate_limit;
	uint32_t last_donation;
	uint8_t daily_donate_count;
	uint32_t last_daily_donate;
#endif
} TPacketGuildChangeMemberData;

typedef struct SPacketDGLoginAlready
{
	char szLogin[LOGIN_MAX_LEN + 1];
} TPacketDGLoginAlready;

typedef struct SPacketGDAffectElement
{
	uint32_t dwType;
	uint16_t wApplyOn; //@fixme532
	long lApplyValue;
	uint32_t dwFlag;
	long lDuration;
	long lSPCost;
#ifdef ENABLE_AFFECT_RENEWAL
	uint32_t dwExpireTime;
#endif
} TPacketGDAffectElement;

typedef struct SPacketGDAddAffect
{
	uint32_t dwPID;
	TPacketGDAffectElement elem;
} TPacketGDAddAffect;

typedef struct SPacketGDRemoveAffect
{
	uint32_t dwPID;
	uint32_t dwType;
	uint16_t wApplyOn; //@fixme532
} TPacketGDRemoveAffect;

typedef struct SPacketGDHighscore
{
	uint32_t dwPID;
	long lValue;
	char cDir;
	char szBoard[21];
} TPacketGDHighscore;

typedef struct SPacketPartyCreate
{
	uint32_t dwLeaderPID;
} TPacketPartyCreate;

typedef struct SPacketPartyDelete
{
	uint32_t dwLeaderPID;
} TPacketPartyDelete;

typedef struct SPacketPartyAdd
{
	uint32_t dwLeaderPID;
	uint32_t dwPID;
	uint8_t bState;
} TPacketPartyAdd;

typedef struct SPacketPartyRemove
{
	uint32_t dwLeaderPID;
	uint32_t dwPID;
} TPacketPartyRemove;

typedef struct SPacketPartyStateChange
{
	uint32_t dwLeaderPID;
	uint32_t dwPID;
	uint8_t bRole;
	uint8_t bFlag;
} TPacketPartyStateChange;

typedef struct SPacketPartySetMemberLevel
{
	uint32_t dwLeaderPID;
	uint32_t dwPID;
	uint8_t bLevel;
} TPacketPartySetMemberLevel;

typedef struct SPacketGDBoot
{
	uint32_t dwItemIDRange[2];
	char szIP[16];
} TPacketGDBoot;

typedef struct SPacketGuild
{
	uint32_t dwGuild;
	uint32_t dwInfo;
} TPacketGuild;

typedef struct SPacketGDGuildAddMember
{
	uint32_t dwPID;
	uint32_t dwGuild;
	uint8_t bGrade;
} TPacketGDGuildAddMember;

typedef struct SPacketDGGuildMember
{
	uint32_t dwPID;
	uint32_t dwGuild;
	uint8_t bGrade;
	uint8_t isGeneral;
	uint8_t bJob;
	uint8_t bLevel;
	uint32_t dwOffer;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	uint32_t dwJoinDate;
	uint8_t bDonateLimit;
	uint32_t dwLastDonation;
	uint8_t bDailyDonateCount;
	uint32_t dwLastDailyDonate;
#endif
} TPacketDGGuildMember;

typedef struct SPacketGuildWar
{
	uint8_t bType;
	uint8_t bWar;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t bRound;
	uint8_t bPoints;
	uint8_t bTime;
#endif
	uint32_t dwGuildFrom;
	uint32_t dwGuildTo;
	long lWarPrice;
	long lInitialScore;
} TPacketGuildWar;

typedef struct SPacketGuildWarScore
{
	uint32_t dwGuildGainPoint;
	uint32_t dwGuildOpponent;
	long lScore;
	long lBetScore;
} TPacketGuildWarScore;

typedef struct SRefineMaterial
{
	uint32_t vnum;
	int count;
} TRefineMaterial;

typedef struct SRefineTable
{
	//uint32_t src_vnum;
	//uint32_t result_vnum;
	uint32_t id;
	uint8_t material_count;
	int cost;
	int prob;
	TRefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SBanwordTable
{
	char szWord[BANWORD_MAX_LEN + 1];
} TBanwordTable;

typedef struct SPacketGDChangeName
{
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGDChangeName;

typedef struct SPacketDGChangeName
{
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketDGChangeName;

typedef struct SPacketGuildLadder
{
	uint32_t dwGuild;
	long lLadderPoint;
	long lWin;
	long lDraw;
	long lLoss;
#ifdef ENABLE_GUILD_WAR_SCORE
	long lWinNew[3];
	long lDrawNew[3];
	long lLossNew[3];
#endif
} TPacketGuildLadder;

typedef struct SPacketGuildLadderPoint
{
	uint32_t dwGuild;
	long lChange;
} TPacketGuildLadderPoint;

typedef struct SPacketGDSMS
{
	char szFrom[CHARACTER_NAME_MAX_LEN + 1];
	char szTo[CHARACTER_NAME_MAX_LEN + 1];
	char szMobile[MOBILE_MAX_LEN + 1];
	char szMsg[SMS_MAX_LEN + 1];
} TPacketGDSMS;

typedef struct SPacketGuildUseSkill
{
	uint32_t dwGuild;
	uint32_t dwSkillVnum;
	uint32_t dwCooltime;
} TPacketGuildUseSkill;

typedef struct SPacketGuildSkillUsableChange
{
	uint32_t dwGuild;
	uint32_t dwSkillVnum;
	uint8_t bUsable;
} TPacketGuildSkillUsableChange;

typedef struct SPacketGDLoginKey
{
	uint32_t dwAccountID;
	uint32_t dwLoginKey;
} TPacketGDLoginKey;

typedef struct SPacketGDAuthLogin
{
	uint32_t dwID;
	uint32_t dwLoginKey;
	char szLogin[LOGIN_MAX_LEN + 1];
#ifdef ENABLE_HWID_BAN
	char hwid[HWID_MAX_LEN + 1];
#endif
	char szSocialID[SOCIAL_ID_MAX_LEN + 1];
	uint32_t adwClientKey[4];
	int iPremiumTimes[PREMIUM_MAX_NUM];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketGDAuthLogin;

typedef struct SPacketGDLoginByKey
{
	char szLogin[LOGIN_MAX_LEN + 1];
	uint32_t dwLoginKey;
	uint32_t adwClientKey[4];
	char szIP[MAX_HOST_LENGTH + 1];
} TPacketGDLoginByKey;

/**
 * @version 05/06/08 Bang2ni-added duration
 */
typedef struct SPacketGiveGuildPriv
{
	uint8_t type;
	int value;
	uint32_t guild_id;
	time_t duration_sec;
} TPacketGiveGuildPriv;
typedef struct SPacketGiveEmpirePriv
{
	uint8_t type;
	int value;
	uint8_t empire;
	time_t duration_sec;
} TPacketGiveEmpirePriv;
typedef struct SPacketGiveCharacterPriv
{
	uint8_t type;
	int value;
	uint32_t pid;
} TPacketGiveCharacterPriv;
typedef struct SPacketRemoveGuildPriv
{
	uint8_t type;
	uint32_t guild_id;
} TPacketRemoveGuildPriv;
typedef struct SPacketRemoveEmpirePriv
{
	uint8_t type;
	uint8_t empire;
} TPacketRemoveEmpirePriv;

typedef struct SPacketDGChangeCharacterPriv
{
	uint8_t type;
	int value;
	uint32_t pid;
	uint8_t bLog;
} TPacketDGChangeCharacterPriv;

/**
* @version 05/06/08 Bang2ni-added duration
*/
typedef struct SPacketDGChangeGuildPriv
{
	uint8_t type;
	int value;
	uint32_t guild_id;
	uint8_t bLog;
	time_t end_time_sec;
} TPacketDGChangeGuildPriv;

typedef struct SPacketDGChangeEmpirePriv
{
	uint8_t type;
	int value;
	uint8_t empire;
	uint8_t bLog;
	time_t end_time_sec;
} TPacketDGChangeEmpirePriv;

typedef struct SPacketMoneyLog
{
	uint8_t type;
	uint32_t vnum;
	INT gold;
} TPacketMoneyLog;

typedef struct SPacketGDGuildMoney
{
	uint32_t dwGuild;
	INT iGold;
} TPacketGDGuildMoney;

typedef struct SPacketDGGuildMoneyChange
{
	uint32_t dwGuild;
	INT iTotalGold;
} TPacketDGGuildMoneyChange;

typedef struct SPacketDGGuildMoneyWithdraw
{
	uint32_t dwGuild;
	INT iChangeGold;
} TPacketDGGuildMoneyWithdraw;

typedef struct SPacketGDGuildMoneyWithdrawGiveReply
{
	uint32_t dwGuild;
	INT iChangeGold;
	uint8_t bGiveSuccess;
} TPacketGDGuildMoneyWithdrawGiveReply;

typedef struct SPacketSetEventFlag
{
	char szFlagName[EVENT_FLAG_NAME_MAX_LEN + 1];
	long lValue;
} TPacketSetEventFlag;

typedef struct SPacketLoginOnSetup
{
	uint32_t dwID;
	char szLogin[LOGIN_MAX_LEN + 1];
	char szSocialID[SOCIAL_ID_MAX_LEN + 1];
	char szHost[MAX_HOST_LENGTH + 1];
	uint32_t dwLoginKey;
	uint32_t adwClientKey[4];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketLoginOnSetup;

typedef struct SPacketGDCreateObject
{
	uint32_t dwVnum;
	uint32_t dwLandID;
	INT lMapIndex;
	INT x, y;
	float xRot;
	float yRot;
	float zRot;
} TPacketGDCreateObject;

typedef struct SPacketGDHammerOfTor
{
	uint32_t key;
	uint32_t delay;
} TPacketGDHammerOfTor;

typedef struct SGuildReserve
{
	uint32_t dwID;
	uint32_t dwGuildFrom;
	uint32_t dwGuildTo;
	uint32_t dwTime;
	uint8_t bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t bRound;
	uint8_t bPoints;
	uint8_t bTime;
#endif
	long lWarPrice;
	long lInitialScore;
	bool bStarted;
	uint32_t dwBetFrom;
	uint32_t dwBetTo;
	long lPowerFrom;
	long lPowerTo;
	long lHandicap;
} TGuildWarReserve;

typedef struct
{
	uint32_t dwWarID;
	char szLogin[LOGIN_MAX_LEN + 1];
	uint32_t dwGold;
	uint32_t dwGuild;
} TPacketGDGuildWarBet;

#ifdef ENABLE_WEATHER_INFO
typedef struct SPacketDGWeatherInfo
{
	uint8_t bDayMode;
	bool bSeason;
} TPacketDGWeatherInfo;
#endif

// Marriage

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
	time_t tMarryTime;
	char szName1[CHARACTER_NAME_MAX_LEN + 1];
	char szName2[CHARACTER_NAME_MAX_LEN + 1];
} TPacketMarriageAdd;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
	INT iLovePoint;
	uint8_t byMarried;
} TPacketMarriageUpdate;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketMarriageRemove;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketWeddingRequest;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
	uint32_t dwMapIndex;
} TPacketWeddingReady;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketWeddingStart;

typedef struct
{
	uint32_t dwPID1;
	uint32_t dwPID2;
} TPacketWeddingEnd;

/// Header of personal store price information. This is a variable packet, followed by TItemPriceInfo as much as byCount.
typedef struct SPacketMyshopPricelistHeader
{
	uint32_t dwOwnerID; ///< Player ID with price information
	uint8_t byCount; ///< Number of price information
} TPacketMyshopPricelistHeader;

/// Price information for a single item in a private store
typedef struct SItemPriceInfo
{
	uint32_t dwVnum; ///< Item vnum
#ifdef ENABLE_CHEQUE_SYSTEM
	TItemPriceType price;
#else
	uint32_t dwPrice; ///< price
#endif
} TItemPriceInfo;

/// Personal store item price information list table
typedef struct SItemPriceListTable
{
	uint32_t dwOwnerID; ///< Player ID with price information
	uint8_t byCount; ///< Number of price information list

	TItemPriceInfo aPriceInfo[SHOP_PRICELIST_MAX_NUM]; ///< List of price information
} TItemPriceListTable;

typedef struct
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lDuration;
} TPacketBlockChat;

//ADMIN_MANAGER
typedef struct TAdminInfo
{
	int m_ID; //Unique ID
	char m_szAccount[32]; //account
	char m_szName[32]; //Character name
	char m_szContactIP[16]; //Approach IP
	char m_szServerIP[16]; //Server IP
	int m_Authority; //authority
} tAdminInfo;
//END_ADMIN_MANAGER

//BOOT_LOCALIZATION
struct tLocale
{
	char szValue[32];
	char szKey[32];
};
//BOOT_LOCALIZATION

//RELOAD_ADMIN
typedef struct SPacketReloadAdmin
{
	char szIP[16];
} TPacketReloadAdmin;
//END_RELOAD_ADMIN

typedef struct TMonarchInfo
{
	uint32_t pid[4]; // PID of the monarch
	int64_t money[4]; // Monarch's separate money
	char name[4][32]; // The name of the monarch
	char date[4][32]; // Date of monarchy registration
} MonarchInfo;

typedef struct TMonarchElectionInfo
{
	uint32_t pid; // Voting Person PID
	uint32_t selectedpid; // PID (lord participant) voted
	char date[32]; // Voting date
} MonarchElectionInfo;

// A monarch runner
typedef struct tMonarchCandidacy
{
	uint32_t pid;
	char name[32];
	char date[32];
} MonarchCandidacy;

typedef struct tChangeMonarchLord
{
	uint8_t bEmpire;
	uint32_t dwPID;
} TPacketChangeMonarchLord;

typedef struct tChangeMonarchLordACK
{
	uint8_t bEmpire;
	uint32_t dwPID;
	char szName[32];
	char szDate[32];
} TPacketChangeMonarchLordACK;

// Block Country Ip
typedef struct tBlockCountryIp
{
	uint32_t ip_from;
	uint32_t ip_to;
} TPacketBlockCountryIp;

enum EBlockExceptionCommand
{
	BLOCK_EXCEPTION_CMD_ADD = 1,
	BLOCK_EXCEPTION_CMD_DEL = 2,
};

// Block Exception Account
typedef struct tBlockException
{
	uint8_t cmd; // 1 == add, 2 == delete
	char login[LOGIN_MAX_LEN + 1];
}TPacketBlockException;

typedef struct tChangeGuildMaster
{
	uint32_t dwGuildID;
	uint32_t idFrom;
	uint32_t idTo;
} TPacketChangeGuildMaster;

typedef struct tItemIDRange
{
	uint32_t dwMin;
	uint32_t dwMax;
	uint32_t dwUsableItemIDMin;
} TItemIDRangeTable;

typedef struct tUpdateHorseName
{
	uint32_t dwPlayerID;
	char szHorseName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketUpdateHorseName;

typedef struct tDC
{
	char login[LOGIN_MAX_LEN + 1];
} TPacketDC;

typedef struct tNeedLoginLogInfo
{
	uint32_t dwPlayerID;
} TPacketNeedLoginLogInfo;

#ifdef ENABLE_DELETE_FAILURE_TYPE
typedef struct tFailureDeleteInfo
{
	uint8_t type;
	int time;
} TPacketDeleteFailureInfo;
#endif

//German gift notification function test packet information
typedef struct tItemAwardInformer
{
	char login[LOGIN_MAX_LEN + 1];
	char command[20]; //command
	uint32_t vnum; //item
} TPacketItemAwardInfromer;
// Packet information for deletion of gift notification function
typedef struct tDeleteAwardID
{
	uint32_t dwID;
} TPacketDeleteAwardID;

typedef struct SChannelStatus
{
	int16_t nPort;
	uint8_t bStatus;
} TChannelStatus;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
typedef struct SPacketGDLangaugeChange
{
	uint32_t dwAID;
	uint8_t bLanguage;
	bool bState;
} TPacketGDLangaugeChange;
#endif

#ifdef ENABLE_MESSENGER_BLOCK
enum MessengerBlock
{
	MESSENGER_BLOCK,
	MESSENGER_FRIEND
};
#endif

#ifdef ENABLE_SWITCHBOT
struct TSwitchbotAttributeAlternativeTable
{
	TPlayerItemAttribute attributes[ITEM_ATTRIBUTE_NORM_NUM];

	bool IsConfigured() const
	{
		for (const auto& it : attributes)
		{
			if (it.wType && it.sValue)
			{
				return true;
			}
		}

		return false;
	}
};

struct TSwitchbotTable
{
	uint32_t player_id;
	bool active[SWITCHBOT_SLOT_COUNT];
	bool finished[SWITCHBOT_SLOT_COUNT];
	uint32_t items[SWITCHBOT_SLOT_COUNT];
	TSwitchbotAttributeAlternativeTable alternatives[SWITCHBOT_SLOT_COUNT][SWITCHBOT_ALTERNATIVE_COUNT];

	TSwitchbotTable() : player_id(0)
	{
		memset(&items, 0, sizeof(items));
		memset(&alternatives, 0, sizeof(alternatives));
		memset(&active, false, sizeof(active));
		memset(&finished, false, sizeof(finished));
	}
};

struct TSwitchbottAttributeTable
{
	uint8_t attribute_set;
	int apply_num;
	long max_value;
};
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
typedef struct SPacketGDGuildDungeon
{
	uint32_t dwGuildID;
	uint8_t bChannel;
	long lMapIndex;
} TPacketGDGuildDungeon;

typedef struct SPacketDGGuildDungeon
{
	uint32_t dwGuildID;
	uint8_t bChannel;
	long lMapIndex;
} TPacketDGGuildDungeon;

typedef struct SPacketGDGuildDungeonCD
{
	uint32_t dwGuildID;
	uint32_t dwTime;
} TPacketGDGuildDungeonCD;

typedef struct SPacketDGGuildDungeonCD
{
	uint32_t dwGuildID;
	uint32_t dwTime;
} TPacketDGGuildDungeonCD;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
typedef struct
{
	uint32_t player_id;
	uint32_t dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
} TSkillColor;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct
{
	uint32_t itemVnum;
	int amount;
	uint32_t gold;
# ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	uint32_t cheque;
# endif
	uint8_t pos;
} TPacketShopSaleResult;

typedef struct
{
	bool success;
	uint32_t goldAmount;
# ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	uint32_t chequeAmount;
# endif
	uint32_t shopPid;
} TPacketGoldWithdrawResult;

enum ShopSubheaders
{
	SHOP_SUBHEADER_GD_SAVE,
	SHOP_SUBHEADER_GD_BUY,
	SHOP_SUBHEADER_GD_REMOVE,
	SHOP_SUBHEADER_GD_WITHDRAW,
	SHOP_SUBHEADER_GD_UPDATE_OFFLINE,
	SHOP_SUBHEADER_GD_CLOSE_REQUEST,
	SHOP_SUBHEADER_GD_RENAME,
	SHOP_SUBHEADER_GD_WITHDRAW_ROLLBACK,
	SHOP_SUBHEADER_GD_SAVE_FLUSH,
};
#endif

#ifdef ENABLE_MAILBOX
enum EMAILBOX
{
	MAILBOX_TAX = 5,
	MAILBOX_REMAIN_DAY = 30,
	MAILBOX_REMAIN_DAY_GM = 7,
	MAILBOX_REMAIN_REWARD = 30,
	MAILBOX_LEVEL_LIMIT = 20,
	MAILBOX_PRICE_YANG = 1000,
	MAILBOX_PAGE_SIZE = 9,
	MAILBOX_PAGE_COUNT = 10,
	MAILBOX_MAX_MAIL = MAILBOX_PAGE_SIZE * MAILBOX_PAGE_COUNT,
};

typedef struct SMailBoxRespondUnreadData
{
	SMailBoxRespondUnreadData() :
		bHeader(0),
		bItemMessageCount(0),
		bCommonMessageCount(0),
		bGMVisible(false)
	{}
	uint8_t bHeader;
	uint8_t bItemMessageCount;
	uint8_t bCommonMessageCount;
	bool bGMVisible;
} TMailBoxRespondUnreadData;

typedef struct SMailBox
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t Index;
} TMailBox;

typedef struct packet_mailbox_add_data
{
	uint8_t bHeader;
	uint8_t Index;
	char szFrom[CHARACTER_NAME_MAX_LEN + 1];
	char szMessage[100 + 1];
	int iYang;
	int iWon;
	uint32_t ItemVnum;
	uint32_t ItemCount;
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t grade_element;
	uint32_t attack_element[MAX_ELEMENTS_SPELL];
	uint8_t element_type_bonus;
	int16_t elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_SET_ITEM
	uint8_t set_value;
#endif
} TPacketGCMailBoxAddData;

typedef struct packet_mailbox_message
{
	time_t SendTime;
	time_t DeleteTime;
	char szTitle[25 + 1];
	bool bIsGMPost;
	bool bIsItemExist;
	bool bIsConfirm;
} TPacketGCMailBoxMessage;

typedef struct SMailBoxTable
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	bool bIsDeleted;
	packet_mailbox_message Message;
	packet_mailbox_add_data AddData;
} TMailBoxTable;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
typedef struct SPlayerExtBattlePassMission
{
	uint32_t dwPlayerId;
	uint32_t dwBattlePassType;
	uint32_t dwMissionIndex;
	uint32_t dwMissionType;
	uint32_t dwBattlePassId;
	uint32_t dwExtraInfo;
	uint8_t bCompleted;
	uint8_t bIsUpdated;
} TPlayerExtBattlePassMission;

typedef struct SExtBattlePassRewardItem
{
	uint32_t dwVnum;
	uint8_t bCount;
} TExtBattlePassRewardItem;

typedef struct SExtBattlePassMissionInfo
{
	uint8_t bMissionIndex;
	uint8_t bMissionType;
	uint32_t dwMissionInfo[3];
	TExtBattlePassRewardItem aRewardList[3];
} TExtBattlePassMissionInfo;

typedef struct SExtBattlePassTimeTable
{
	uint8_t bBattlePassId;
	uint32_t dwStartTime;
	uint32_t dwEndTime;
} TExtBattlePassTimeTable;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include <map>
enum SAchievementSubHeaders : int8_t
{
	ACHIEVEMENT_SUBHEADER_LOGIN,
	ACHIEVEMENT_SUBHEADER_LOGOUT,

	ACHIEVEMENT_SUBHEADER_RANKING,

};

typedef std::map<uint32_t, uint64_t> TTasksList;
typedef std::map<uint32_t, TTasksList> TAchievementsMap;

typedef struct SPlayerAchievementInfo
{
	uint32_t pid;
	uint32_t points;
	uint32_t title;
	TAchievementsMap achievements;
}TPlayerAchievementInfo;

typedef struct SAchievementRanking
{
	char szName[CHARACTER_NAME_MAX_LEN];
	uint8_t level;
	char szGuild[GUILD_NAME_MAX_LEN];
	uint32_t finished;
} TAchievementRanking;
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
typedef struct SMultiFarm
{
	uint32_t playerID;
	bool farmStatus;
	uint8_t affectType;
	int affectTime;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	SMultiFarm(uint32_t id_, const char* playerName_, bool status_, uint8_t type_, int time_) : playerID(id_), farmStatus(status_), affectType(type_), affectTime(time_) {
		strlcpy(playerName, playerName_, sizeof(playerName));
	}
}TMultiFarm;
#endif

#ifdef ENABLE_EVENT_MANAGER
typedef struct SEventTable
{
	uint32_t dwID;
	char szType[64];
	long startTime;
	long endTime;
	uint32_t dwVnum;
	int iPercent;
	int iDropType;
	uint8_t bCompleted;
} TEventTable;
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
typedef struct SBiologMissions
{
	uint8_t bMission;
	bool bSubMission;
	uint32_t dwNpcVnum;
	uint8_t bRequiredLevel;
	uint32_t iRequiredItem;
	uint16_t wRequiredItemCount;
	uint32_t dwRequiredSubItem;
	time_t iCooldown;
	uint8_t bChance;
} TBiologMissionsProto;

typedef struct SBiologRewards
{
	uint8_t bMission;
	uint32_t dRewardItem;
	uint16_t wRewardItemCount;
	uint16_t wApplyType[MAX_BONUSES_LENGTH];	//@fixme532
	long lApplyValue[MAX_BONUSES_LENGTH];
} TBiologRewardsProto;
#endif

#ifdef ENABLE_MINI_GAME_BNW
typedef struct SBNWCard
{
	SBNWCard() { bIndex = 0; bIsUsed = false; }
	SBNWCard(uint8_t index, bool isUsed)
	{
		bIndex = index;
		bIsUsed = isUsed;
	}

	uint8_t bIndex;
	bool bIsUsed;
} TBNWCard;
#endif

#ifdef ENABLE_INGAME_WIKI
typedef struct SRefineMaterialNew
{
	uint32_t vnum;
	int count;
} TRefineMaterialNew;

typedef struct SWikiChestInfo
{
	uint32_t vnum;
	int count;
} TWikiChestInfo;

typedef struct SWikiMobDropInfo
{
	uint32_t vnum;
	int count;
} TWikiMobDropInfo;

typedef struct SWikiRefineInfo
{
	int index;
	TRefineMaterialNew materials[REFINE_MATERIAL_MAX_NUM];
	uint32_t mat_count;
	int price;
} TWikiRefineInfo;

typedef struct SWikiItemOriginInfo
{
	uint32_t vnum;
	bool is_mob;
} TWikiItemOriginInfo;

typedef struct SWikiInfoTable
{
	bool is_common;
	int refine_infos_count;
	int chest_info_count;
	uint32_t origin_vnum;
} TWikiInfoTable;
#endif

#pragma pack()
#endif
