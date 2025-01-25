#pragma once

#include "Locale.h"
#include "../GameLib/RaceData.h"
#include "../GameLib/ItemData.h"

using TPacketHeader = unsigned char;

using TPacketHeader = unsigned char;

enum HeaderClientGame
{
	HEADER_CG_LOGIN = 1,
	HEADER_CG_ATTACK = 2,
	HEADER_CG_CHAT = 3,
	HEADER_CG_PLAYER_CREATE = 4,
	HEADER_CG_PLAYER_DESTROY = 5,
	HEADER_CG_PLAYER_SELECT = 6,
	HEADER_CG_CHARACTER_MOVE = 7,
	HEADER_CG_SYNC_POSITION = 8,
	HEADER_CG_ENTERGAME = 10,
	HEADER_CG_ITEM_USE = 11,
	HEADER_CG_ITEM_DROP = 12,
	HEADER_CG_ITEM_MOVE = 13,
	HEADER_CG_ITEM_PICKUP = 15,
	HEADER_CG_QUICKSLOT_ADD = 16,
	HEADER_CG_QUICKSLOT_DEL = 17,
	HEADER_CG_QUICKSLOT_SWAP = 18,
	HEADER_CG_WHISPER = 19,
	HEADER_CG_ITEM_DROP2 = 20,
#ifdef ENABLE_DESTROY_SYSTEM
	HEADER_CG_ITEM_DESTROY = 25,
#endif
	HEADER_CG_ON_CLICK = 26,
	HEADER_CG_EXCHANGE = 27,
	HEADER_CG_CHARACTER_POSITION = 28,
	HEADER_CG_SCRIPT_ANSWER = 29,
	HEADER_CG_QUEST_INPUT_STRING = 30,
	HEADER_CG_QUEST_CONFIRM = 31,
#ifdef ENABLE_PARTY_MATCH
	HEADER_CG_PARTY_MATCH = 32,
#endif
#ifdef ENABLE_OX_RENEWAL
	HEADER_CG_QUEST_INPUT_LONG_STRING = 33,
#endif
	HEADER_CG_SHOP = 50,
	HEADER_CG_FLY_TARGETING = 51,
	HEADER_CG_USE_SKILL = 52,
	HEADER_CG_ADD_FLY_TARGETING = 53,
	HEADER_CG_SHOOT = 54,
	HEADER_CG_MYSHOP = 55,
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_CG_SKILL_COLOR = 57,
#endif
#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_CG_TARGET_INFO_LOAD = 59,
#endif
	HEADER_CG_ITEM_USE_TO_ITEM = 60,
	HEADER_CG_TARGET = 61,
	HEADER_CG_WARP = 65,
	HEADER_CG_SCRIPT_BUTTON = 66,
	HEADER_CG_MESSENGER = 67,
	HEADER_CG_MALL_CHECKOUT = 69,
	HEADER_CG_SAFEBOX_CHECKIN = 70,
	HEADER_CG_SAFEBOX_CHECKOUT = 71,
	HEADER_CG_PARTY_INVITE = 72,
	HEADER_CG_PARTY_INVITE_ANSWER = 73,
	HEADER_CG_PARTY_REMOVE = 74,
	HEADER_CG_PARTY_SET_STATE = 75,
	HEADER_CG_PARTY_USE_SKILL = 76,
	HEADER_CG_SAFEBOX_ITEM_MOVE = 77,
	HEADER_CG_PARTY_PARAMETER = 78,
#ifdef ENABLE_SAFEBOX_MONEY
	HEADER_CG_SAFEBOX_MONEY = 79,
#endif
	HEADER_CG_GUILD = 80,
	HEADER_CG_ANSWER_MAKE_GUILD = 81,
	HEADER_CG_FISHING = 82,
	HEADER_CG_GIVE_ITEM = 83,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	HEADER_CG_GUILDSTORAGE_CHECKIN = 84,
	HEADER_CG_GUILDSTORAGE_CHECKOUT = 85,
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	HEADER_CG_LOTTO_OPENINGS = 86,
	HEADER_CG_LOTTO_BUY_TICKET = 87,
	HEADER_CG_LOTTO_TICKET_OPTIONS = 88,
	HEADER_CG_LOTTO_PICK_MONEY = 89,
#endif
	HEADER_CG_EMPIRE = 90,
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	HEADER_CG_GEM_SHOP = 91,
	HEADER_CG_SCRIPT_SELECT_ITEM_EX = 92,
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	HEADER_CG_PRIVATE_SHOP_SEARCH_SEARCH = 93,
	HEADER_CG_PRIVATE_SHOP_SEARCH_BUY = 94,
	HEADER_CG_PRIVATE_SHOP_SEARCH_CLOSE = 95,
#endif
	HEADER_CG_REFINE = 96,
	HEADER_CG_MARK_LOGIN = 100,
	HEADER_CG_MARK_CRCLIST = 101,
	HEADER_CG_MARK_UPLOAD = 102,
	HEADER_CG_MARK_IDXLIST = 104,
	HEADER_CG_HACK = 105,
	HEADER_CG_CHANGE_NAME = 106,
	HEADER_CG_LOGIN2 = 109,
	HEADER_CG_DUNGEON = 110,
	HEADER_CG_LOGIN3 = 111,
	HEADER_CG_GUILD_SYMBOL_UPLOAD = 112,
	HEADER_CG_GUILD_SYMBOL_CRC = 113,
	HEADER_CG_SCRIPT_SELECT_ITEM = 114,
#ifdef ENABLE_HWID_BAN
	HEADER_CG_HWID_SYSTEM = 117,
#endif
#ifdef ENABLE_MYSHOP_DECO
	HEADER_CG_MYSHOP_DECO_STATE = 118,
	HEADER_CG_MYSHOP_DECO_ADD = 119,
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//Myshop
	HEADER_CG_MYSHOP_REMOVE_ITEM = 120,
	HEADER_CG_MYSHOP_ADD_ITEM = 121,
	HEADER_CG_MYSHOP_CLOSE = 122,
	HEADER_CG_MYSHOP_WITHDRAW = 123,
	HEADER_CG_MYSHOP_RENAME = 124,
	HEADER_CG_MYSHOP_OPEN = 125,
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_EXPIRE
	HEADER_CG_MYSHOP_REOPEN = 126,
#	endif
#endif
#ifdef ENABLE_CHANGED_ATTR
	HEADER_CG_ITEM_SELECT_ATTR = 127,
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	HEADER_CG_BIOLOG_MANAGER = 128,
#endif
#ifdef ENABLE_TICKET_SYSTEM
	HEADER_CG_TICKET_SYSTEM = 129,
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	HEADER_CG_EXTEND_INVEN_REQUEST = 140,
	HEADER_CG_EXTEND_INVEN_UPGRADE = 141,
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
	HEADER_CG_COSTUME_REFINE_REQUEST = 142,
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	HEADER_CG_ACCE_CLOSE_REQUEST = 143,
	HEADER_CG_ACCE_REFINE_REQUEST = 144,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_CG_GROWTH_PET = 145,
	HEADER_CG_GROWTH_PET_HATCHING = 146,
	HEADER_CG_GROWTH_PET_LEARN_SKILL = 147,
	HEADER_CG_GROWTH_PET_SKILL_UPGRADE = 148,
	HEADER_CG_GROWTH_PET_SKILL_UPGRADE_REQUEST = 149,
	HEADER_CG_GROWTH_PET_FEED_REQUEST = 150,
	HEADER_CG_GROWTH_PET_DELETE_SKILL = 151,
	HEADER_CG_GROWTH_PET_ALL_DEL_SKILL = 152,
	HEADER_CG_GROWTH_PET_NAME_CHANGE = 153,
#	ifdef ENABLE_PET_ATTR_DETERMINE
	HEADER_CG_GROWTH_PET_ATTR_DETERMINE = 154,
	HEADER_CG_GROWTH_PET_ATTR_CHANGE = 155,
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	HEADER_CG_GROWTH_PET_REVIVE_REQUEST = 156,
#	endif
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM	
	HEADER_CG_DUNGEON_INFO = 159,
#endif
#ifdef ENABLE_AURA_SYSTEM
	HEADER_CG_AURA = 160,
#endif
	// MINI_EVENTS
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	HEADER_CG_OKEY_CARD = 161,
#endif
#ifdef ENABLE_MONSTER_BACK
	HEADER_CG_ATTENDANCE_REWARD = 162,
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	HEADER_CG_MINI_GAME_CATCH_KING = 163,
#endif
#ifdef ENABLE_FISH_EVENT
	HEADER_CG_FISH_EVENT_SEND = 164,
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	HEADER_CG_SOUL_ROULETTE = 165,
#endif
#ifdef ENABLE_MINI_GAME_BNW
	HEADER_CG_MINI_GAME_BNW = 166,
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	HEADER_CG_MINI_GAME_FIND_M = 167,
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	HEADER_CG_MINI_GAME_YUT_NORI = 168,
#endif
#ifdef ENABLE_FLOWER_EVENT
	HEADER_CG_FLOWER_EVENT_EXCHANGE = 169,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	HEADER_CG_ACHIEVEMENT = 170,
#endif
	// END_MINI_EVENTS
#ifdef ENABLE_SWITCHBOT
	HEADER_CG_SWITCHBOT = 171,
#endif
	HEADER_CG_DRAGON_SOUL_REFINE = 205,
	HEADER_CG_STATE_CHECKER = 206,
#ifdef ENABLE_INGAME_WIKI
	HEADER_CG_WIKI = 207,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	HEADER_CG_CHANGE_LOOK = 213,
#endif
#ifdef ENABLE_CUBE_RENEWAL
	HEADER_CG_CUBE_RENEWAL = 217,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_CG_EXT_BATTLE_PASS_ACTION = 218,
	HEADER_CG_EXT_SEND_BP_PREMIUM = 219,
#endif
#ifdef ENABLE_HUNTING_SYSTEM
	HEADER_CG_SEND_HUNTING_ACTION = 220,
#endif
#ifdef ENABLE_REFINE_ELEMENT
	HEADER_CG_ELEMENTS_SPELL = 221,
#endif
#ifdef ENABLE_FISHING_RENEWAL
	HEADER_CG_FISHING_NEW = 222,
#endif
#ifdef ENABLE_BATTLE_FIELD
	HEADER_CG_REQUEST_ENTER_BATTLE = 223,
	HEADER_CG_REQUEST_EXIT_BATTLE = 224,
#endif
#ifdef ENABLE_MAILBOX
	HEADER_CG_MAILBOX_WRITE = 225,
	HEADER_CG_MAILBOX_WRITE_CONFIRM = 226,
	HEADER_CG_MAILBOX_PROCESS = 227,
#endif
#ifdef ENABLE_MOVE_CHANNEL
	HEADER_CG_MOVE_CHANNEL = 229,
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	HEADER_CG_LANGUAGE_CHANGE = 230,
	HEADER_CG_TARGET_LANGUAGE_REQUEST = 231,
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	HEADER_CG_ATTR_6TH_7TH = 234,
#endif
#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
	HEADER_CG_WON_EXCHANGE = 235,
#endif
#ifdef ENABLE_EVENT_MANAGER
	HEADER_CG_REQUEST_EVENT_QUEST = 239,
	HEADER_CG_REQUEST_EVENT_DATA = 240,
#endif
	HEADER_CG_CLIENT_VERSION2 = 0xf1,	//241
	HEADER_CG_KEY_AGREEMENT = 0xfb, //251	_IMPROVED_PACKET_ENCRYPTION_
	HEADER_CG_TIME_SYNC = 0xfc,	//252
	HEADER_CG_CLIENT_VERSION = 0xfd,	//253
	HEADER_CG_PONG = 0xfe,	//254
	HEADER_CG_HANDSHAKE = 0xff,	//255
};

enum EPacketHeaderGameClient
{
	HEADER_GC_CHARACTER_ADD = 1,
	HEADER_GC_CHARACTER_DEL = 2,
	HEADER_GC_CHARACTER_MOVE = 3,
	HEADER_GC_CHAT = 4,
	HEADER_GC_SYNC_POSITION = 5,
	HEADER_GC_LOGIN_FAILURE = 7,
	HEADER_GC_PLAYER_CREATE_SUCCESS = 8,
	HEADER_GC_PLAYER_CREATE_FAILURE = 9,
	HEADER_GC_PLAYER_DELETE_SUCCESS = 10,
	HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID = 11,
	HEADER_GC_STUN = 13,
	HEADER_GC_DEAD = 14,
	HEADER_GC_PLAYER_POINTS = 16,
	HEADER_GC_PLAYER_POINT_CHANGE = 17,
	HEADER_GC_CHARACTER_UPDATE = 19,
	HEADER_GC_ITEM_DEL = 20,
	HEADER_GC_ITEM_SET = 21,
	HEADER_GC_ITEM_USE = 22,
	HEADER_GC_ITEM_DROP = 23,
	HEADER_GC_ITEM_UPDATE = 25,
	HEADER_GC_ITEM_GROUND_ADD = 26,
	HEADER_GC_ITEM_GROUND_DEL = 27,
	HEADER_GC_QUICKSLOT_ADD = 28,
	HEADER_GC_QUICKSLOT_DEL = 29,
	HEADER_GC_QUICKSLOT_SWAP = 30,
	HEADER_GC_ITEM_OWNERSHIP = 31,
	HEADER_GC_LOGIN_SUCCESS_NEWSLOT = 32,
	HEADER_GC_WHISPER = 34,
	HEADER_GC_MOTION = 36,
	HEADER_GC_SHOP = 38,
	HEADER_GC_SHOP_SIGN = 39,
	HEADER_GC_DUEL_START = 40,
	HEADER_GC_PVP = 41,
	HEADER_GC_EXCHANGE = 42,
	HEADER_GC_CHARACTER_POSITION = 43,
	HEADER_GC_PING = 44,
	HEADER_GC_SCRIPT = 45,
	HEADER_GC_QUEST_CONFIRM = 46,
#ifdef ENABLE_CHANGED_ATTR
	HEADER_GC_ITEM_SELECT_ATTR = 50,
#endif
#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_GC_TARGET_INFO = 58,
#endif
	HEADER_GC_OWNERSHIP = 62,
	HEADER_GC_TARGET = 63,
	HEADER_GC_WARP = 65,
	HEADER_GC_ADD_FLY_TARGETING = 69,
	HEADER_GC_CREATE_FLY = 70,
	HEADER_GC_FLY_TARGETING = 71,
	HEADER_GC_MESSENGER = 74,
	HEADER_GC_GUILD = 75,
	HEADER_GC_SKILL_LEVEL = 76,
	HEADER_GC_PARTY_INVITE = 77,
	HEADER_GC_PARTY_ADD = 78,
	HEADER_GC_PARTY_UPDATE = 79,
	HEADER_GC_PARTY_REMOVE = 80,
	HEADER_GC_QUEST_INFO = 81,
	HEADER_GC_REQUEST_MAKE_GUILD = 82,
	HEADER_GC_PARTY_PARAMETER = 83,
#ifdef ENABLE_SAFEBOX_MONEY
	HEADER_GC_SAFEBOX_MONEY_CHANGE = 84,	// unused
#endif
	HEADER_GC_SAFEBOX_SET = 85,
	HEADER_GC_SAFEBOX_DEL = 86,
	HEADER_GC_SAFEBOX_WRONG_PASSWORD = 87,
	HEADER_GC_SAFEBOX_SIZE = 88,
	HEADER_GC_FISHING = 89,
	HEADER_GC_EMPIRE = 90,
	HEADER_GC_PARTY_LINK = 91,
	HEADER_GC_PARTY_UNLINK = 92,
	HEADER_GC_VIEW_EQUIP = 99,
	HEADER_GC_MARK_BLOCK = 100,
	HEADER_GC_MARK_IDXLIST = 102,
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	HEADER_GC_PARTY_POSITION_INFO = 104,
#endif
	HEADER_GC_TIME = 106,
	HEADER_GC_CHANGE_NAME = 107,
	HEADER_GC_DUNGEON = 110,
	HEADER_GC_WALK_MODE = 111,
	HEADER_GC_CHANGE_SKILL_GROUP = 112,
	HEADER_GC_MAIN_CHARACTER = 113,
	HEADER_GC_SEPCIAL_EFFECT = 114,
	HEADER_GC_NPC_POSITION = 115,
	HEADER_GC_REFINE_INFORMATION = 119,
	HEADER_GC_CHANNEL = 121,
	HEADER_GC_MALL_OPEN = 122,
	HEADER_GC_TARGET_UPDATE = 123,
	HEADER_GC_TARGET_DELETE = 124,
	HEADER_GC_TARGET_CREATE = 125,
	HEADER_GC_AFFECT_ADD = 126,
	HEADER_GC_AFFECT_REMOVE = 127,
	HEADER_GC_MALL_SET = 128,
	HEADER_GC_MALL_DEL = 129,
	HEADER_GC_LAND_LIST = 130,
	HEADER_GC_LOVER_INFO = 131,
	HEADER_GC_LOVE_POINT_UPDATE = 132,
	HEADER_GC_GUILD_SYMBOL_DATA = 133,
	HEADER_GC_DIG_MOTION = 134,
	HEADER_GC_DAMAGE_INFO = 135,
	HEADER_GC_CHAR_ADDITIONAL_INFO = 136,
	HEADER_GC_MAIN_CHARACTER3_BGM = 137,
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL = 138,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	HEADER_GC_GUILDSTORAGE_OPEN = 141,
	HEADER_GC_GUILDSTORAGE_SET = 142,
	HEADER_GC_GUILDSTORAGE_DEL = 143,
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	HEADER_GC_SOUL_ROULETTE = 144,
#endif
#ifdef ENABLE_AURA_SYSTEM
	HEADER_GC_AURA = 145,
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	HEADER_GC_BIOLOG_MANAGER = 146,
#endif
#ifdef ENABLE_PARTY_MATCH
	HEADER_GC_PARTY_MATCH = 147,
#endif
#ifdef ENABLE_TICKET_SYSTEM
	HEADER_GC_TICKET_SYSTEM = 148,
#endif
	HEADER_GC_AUTH_SUCCESS = 150,
	HEADER_GC_PANAMA_PACK = 151,
	HEADER_GC_HYBRIDCRYPT_KEYS = 152,
	HEADER_GC_HYBRIDCRYPT_SDB = 153,
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	HEADER_GC_GEM_SHOP_OPEN = 155,
	HEADER_GC_GEM_SHOP_REFRESH = 156,
	HEADER_GC_GEM_SHOP_BUY = 157,
	HEADER_GC_GEM_SHOP_ADD = 158,
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	HEADER_GC_ACCE_REFINE_REQUEST = 160,
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	HEADER_GC_PRIVATE_SHOP_SEARCH_SET = 161,
	HEADER_GC_PRIVATE_SHOP_SEARCH_OPEN = 162,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_GC_EXT_BATTLE_PASS_OPEN = 163,
	HEADER_GC_EXT_BATTLE_PASS_GENERAL_INFO = 164,
	HEADER_GC_EXT_BATTLE_PASS_MISSION_INFO = 165,
	HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE = 166,
	HEADER_GC_EXT_BATTLE_PASS_SEND_RANKING = 167,
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	HEADER_GC_SHOP_POSITION = 170,
	HEADER_GC_PLAYER_SHOP_SET = 171,
	HEADER_GC_MY_SHOP_SIGN = 172,
	HEADER_GC_SYNC_SHOP_STASH = 173,
	HEADER_GC_SYNC_SHOP_OFFTIME = 174,
	HEADER_GC_SYNC_SHOP_POSITION = 175,
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	HEADER_GC_EXTEND_INVEN_INFO = 177,
	HEADER_GC_EXTEND_INVEN_RESULT = 178,
#endif
#ifdef ENABLE_SWITCHBOT
	HEADER_GC_SWITCHBOT = 180,
#endif
#ifdef ENABLE_MAILBOX
	HEADER_GC_MAILBOX_PROCESS = 182,
	HEADER_GC_MAILBOX = 183,
	HEADER_GC_MAILBOX_ADD_DATA = 184,
	HEADER_GC_MAILBOX_ALL = 185,
	HEADER_GC_MAILBOX_UNREAD = 186,
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM	
	HEADER_GC_DUNGEON_INFO = 187,
	HEADER_GC_DUNGEON_RANKING = 188,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_GC_GROWTH_PET = 189,
	HEADER_GC_GROWTH_PET_INFO = 190,
	HEADER_GC_GROWTH_PET_HATCHING = 191,
	HEADER_GC_GROWTH_PET_UPGRADE_SKILL_REQUEST = 192,
	HEADER_GC_GROWTH_PET_FEED_RESULT = 193,
	HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT = 194,
#	ifdef ENABLE_PET_ATTR_DETERMINE
	HEADER_GC_GROWTH_ATTR_DETERMINE_RESULT = 195,
	HEADER_GC_GROWTH_ATTR_CHANGE_RESULT = 196,
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	HEADER_GC_GROWTH_PET_REVIVE_RESULT = 197,
#	endif
#endif
#ifdef ENABLE_HUNTING_SYSTEM
	HEADER_GC_HUNTING_OPEN_MAIN = 198,
	HEADER_GC_HUNTING_OPEN_SELECT = 199,
	HEADER_GC_HUNTING_OPEN_REWARD = 200,
	HEADER_GC_HUNTING_UPDATE = 201,
	HEADER_GC_HUNTING_RECIVE_RAND_ITEMS = 202,
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	HEADER_GC_TARGET_LANGUAGE_RESULT = 203,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	HEADER_GC_ACHIEVEMENT = 207,
#endif
	HEADER_GC_SPECIFIC_EFFECT = 208,
	HEADER_GC_DRAGON_SOUL_REFINE = 209,
	HEADER_GC_RESPOND_CHANNELSTATUS = 210,
#ifdef ENABLE_FISHING_RENEWAL
	HEADER_GC_FISHING_NEW = 211,
#endif
#ifdef ENABLE_INGAME_WIKI
	HEADER_GC_WIKI = 212,
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	HEADER_GC_LOTTO_OPENINGS = 213,
	HEADER_GC_LOTTO_SEND_BASIC_INFO = 214,
	HEADER_GC_LOTTO_SEND_TICKET_INFO = 215,
	HEADER_GC_LOTTO_SEND_RANKING_JACKPOT = 216,
	HEADER_GC_LOTTO_SEND_RANKING_MONEY = 217,
#endif
#ifdef ENABLE_CUBE_RENEWAL
	HEADER_GC_CUBE_RENEWAL = 219,
#endif
#ifdef ENABLE_12ZI
	HEADER_GC_SEPCIAL_ZODIAC_EFFECT = 220,
#endif
#ifdef ENABLE_REFINE_ELEMENT
	HEADER_GC_ELEMENTS_SPELL = 221,
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	HEADER_GC_SEAL = 222,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	HEADER_GC_CHANGE_LOOK = 223,
#endif
#ifdef ENABLE_BATTLE_FIELD
	HEADER_GC_BATTLE_ZONE_INFO = 227,
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	HEADER_GC_MELEY_LAIR_TIME_INFO = 228,
	HEADER_GC_MELEY_LAIR_TIME_RESULT = 229,
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	HEADER_GC_ATTR_6TH_7TH = 233,
#endif
#ifdef NEW_SELECT_CHARACTER
	HEADER_GC_CHARACTER_INFORMATION = 234,
#endif
	// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
	HEADER_GC_EVENT_INFO = 235,
	HEADER_GC_EVENT_RELOAD = 236,
	HEADER_GC_EVENT_KW_SCORE = 237,
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	HEADER_GC_OKEY_CARD = 238,
#endif
#ifdef ENABLE_MONSTER_BACK
	HEADER_GC_ATTENDANCE_EVENT_INFO = 239,
	HEADER_GC_ATTENDANCE_EVENT = 240,
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	HEADER_GC_HIT_COUNT_INFO = 241,
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	HEADER_GC_MINI_GAME_CATCH_KING = 242,
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	HEADER_GC_STONE_EVENT = 243,
#endif
#ifdef ENABLE_FISH_EVENT
	HEADER_GC_FISH_EVENT_INFO = 244,
#endif
#ifdef ENABLE_MINI_GAME_BNW
	HEADER_GC_MINI_GAME_BNW = 245,
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	HEADER_GC_MINI_GAME_FIND_M = 246,
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	HEADER_GC_MINI_GAME_YUT_NORI = 247,
#endif
	// END MINI GAME EVENTS //

	HEADER_GC_KEY_AGREEMENT_COMPLETED = 0xfa, //250	_IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_KEY_AGREEMENT = 0xfb, //251	_IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_HANDSHAKE_OK = 0xfc, //252
	HEADER_GC_PHASE = 0xfd,	//253
	HEADER_GC_BINDUDP = 0xfe,	//254
	HEADER_GC_HANDSHAKE = 0xff,	//255
};

enum
{
	ID_MAX_NUM = 30,
	PASS_MAX_NUM = 16,
#ifdef ENABLE_HWID_BAN
	HWID_MAX_LEN = 64,
#endif
	CHAT_MAX_NUM = 128,
	PATH_NODE_MAX_NUM = 64,
	SHOP_SIGN_MAX_LEN = 32,

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT = 5,
#else
	PLAYER_PER_ACCOUNT = 4,
#endif

	PLAYER_ITEM_SLOT_MAX_NUM = 20,

	QUICKSLOT_MAX_LINE = 4,
	QUICKSLOT_MAX_COUNT_PER_LINE = 8,
	QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,

	QUICKSLOT_MAX_NUM = 36,

#ifdef ENABLE_MYSHOP_DECO
	SHOP_HOST_ITEM_MAX = 80, // using only for private shops
#endif
	SHOP_HOST_ITEM_MAX_NUM = 40, // Using only for NPC-Shops
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	GEM_SHOP_HOST_ITEM_MAX_NUM = 9,
#endif

	METIN_SOCKET_COUNT = 6,

	PARTY_AFFECT_SLOT_MAX_NUM = 7,

	GUILD_GRADE_NAME_MAX_LEN = 8,
	GUILD_NAME_MAX_LEN = 12,
	GUILD_GRADE_COUNT = 15,
	GULID_COMMENT_MAX_LEN = 50,

	MARK_CRC_NUM = 8 * 8,
	MARK_DATA_SIZE = 16 * 12,
	SYMBOL_DATA_SIZE = 128 * 256,
	QUEST_INPUT_STRING_MAX_NUM = 64,
#ifdef ENABLE_OX_RENEWAL
	QUEST_INPUT_LONG_STRING_MAX_NUM = 128,
#endif
#ifdef ENABLE_EVENT_MANAGER
	QUEST_NAME_MAX_NUM = 64,
#endif

	PRIVATE_CODE_LENGTH = 8,

	REFINE_MATERIAL_MAX_NUM = 5,

	CHINA_MATRIX_ANSWER_MAX_LEN = 8,

	RUNUP_MATRIX_QUIZ_MAX_LEN = 8,
	RUNUP_MATRIX_ANSWER_MAX_LEN = 4,
	NEWCIBN_PASSPOD_ANSWER_MAX_LEN = 8,
	NEWCIBN_PASSPOD_FAILURE_MAX_LEN = 128,

	WEAR_MAX_NUM = CItemData::WEAR_MAX_NUM,

	SHOP_TAB_NAME_MAX = 32,
	SHOP_TAB_COUNT_MAX = 3
};

#pragma pack(push)
#pragma pack(1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mark
typedef struct command_mark_login
{
	uint8_t header;
	uint32_t handle;
	uint32_t random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
	uint8_t header;
	uint32_t gid;
	uint8_t image[16 * 12 * 4];
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
	uint8_t header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
	uint8_t header;
	uint8_t imgIdx;
	uint32_t crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
	uint8_t header;
	uint32_t bufSize;
	uint16_t count;
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	uint8_t header;
	uint32_t bufSize;
	uint8_t imgIdx;
	uint32_t count;
} TPacketGCMarkBlock;

typedef struct command_symbol_upload
{
	uint8_t header;
	uint16_t size;
	uint32_t handle;
} TPacketCGSymbolUpload;

typedef struct command_symbol_crc
{
	uint8_t header;
	uint32_t dwGuildID;
	uint32_t dwCRC;
	uint32_t dwSize;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data
{
	uint8_t header;
	uint16_t size;
	uint32_t guild_id;
} TPacketGCGuildSymbolData;

typedef struct command_login
{
	uint8_t header;
	char name[ID_MAX_NUM + 1];
	char pwd[PASS_MAX_NUM + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	uint8_t header;
	char name[ID_MAX_NUM + 1];
	uint32_t login_key;
	uint32_t adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	uint8_t header;
	char name[ID_MAX_NUM + 1];
	char pwd[PASS_MAX_NUM + 1];
#ifdef ENABLE_HWID_BAN
	char hwid[HWID_MAX_LEN + 1];
#endif
	uint32_t adwClientKey[4];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketCGLogin3;

typedef struct command_player_select
{
	uint8_t header;
	uint8_t player_index;
} TPacketCGSelectCharacter;

typedef struct command_attack
{
	uint8_t header;
	uint8_t bType;
	uint32_t dwVictimVID;
	uint8_t bCRCMagicCubeProcPiece;
	uint8_t bCRCMagicCubeFilePiece;
} TPacketCGAttack;

typedef struct command_chat
{
	uint8_t header;
	uint16_t length;
	uint8_t type;
} TPacketCGChat;

typedef struct command_whisper
{
	uint8_t bHeader;
	uint16_t wSize;
	char szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

enum EBattleMode
{
	BATTLEMODE_ATTACK = 0,
	BATTLEMODE_DEFENSE = 1
};

typedef struct command_EnterFrontGame
{
	uint8_t header;
} TPacketCGEnterFrontGame;

typedef struct command_item_use
{
	uint8_t header;
	TItemPos pos;
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	uint8_t header;
	TItemPos source_pos;
	TItemPos target_pos;
} TPacketCGItemUseToItem;

typedef struct command_item_drop
{
	uint8_t header;
	TItemPos pos;
	uint32_t elk;
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
	uint8_t header;
	TItemPos pos;
	uint32_t gold;
	uint8_t count;
} TPacketCGItemDrop2;

typedef struct command_item_move
{
	uint8_t header;
	TItemPos pos;
	TItemPos change_pos;
	uint8_t num;
} TPacketCGItemMove;

#ifdef ENABLE_DESTROY_SYSTEM
typedef struct command_item_remove
{
	uint8_t header;
	TItemPos pos;
	uint32_t gold;
	uint8_t count;
} TPacketCGItemRemove;
#endif

typedef struct command_item_pickup
{
	uint8_t header;
	uint32_t vid;
} TPacketCGItemPickUp;

typedef struct command_quickslot_add
{
	uint8_t header;
	uint8_t pos;
	TQuickSlot slot;
} TPacketCGQuickSlotAdd;

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
enum
{
	GEM_SHOP_SUBHEADER_CG_BUY,
	GEM_SHOP_SUBHEADER_CG_ADD,
	GEM_SHOP_SUBHEADER_CG_REFRESH,
};

typedef struct command_gem_shop
{
	uint8_t header;
	uint8_t subheader;
} TPacketCGGemShop;

typedef struct command_gem_shop_items
{
	uint8_t header;
	int nextRefreshTime;
	uint8_t bGemShopOpenSlotCount;
	uint8_t bGemShopOpenSlotItemCount[GEM_SHOP_HOST_ITEM_MAX_NUM];
	TGemShopItem gemShopItems[GEM_SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCGemShopItems;

typedef struct command_gem_shop_buy
{
	uint8_t header;
	uint8_t bSlotIndex;
	uint8_t bEnable;
} TPacketGCGemShopBuy;

typedef struct command_gem_shop_add
{
	uint8_t header;
	uint8_t bSlotIndex;
	uint8_t bEnable;
} TPacketGCGemShopAdd;

typedef struct command_script_select_item_ex
{
	uint8_t header;
	uint32_t selection;
} TPacketCGScriptSelectItemEx;
#endif

typedef struct command_quickslot_del
{
	uint8_t header;
	uint8_t pos;
} TPacketCGQuickSlotDel;

typedef struct command_quickslot_swap
{
	uint8_t header;
	uint8_t pos;
	uint8_t change_pos;
} TPacketCGQuickSlotSwap;

typedef struct command_on_click
{
	uint8_t header;
	uint32_t vid;
} TPacketCGOnClick;

enum
{
	SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
	SHOP_SUBHEADER_CG_SELL2
};

typedef struct command_shop
{
	uint8_t header;
	uint8_t subheader;
#ifdef ENABLE_SPECIAL_INVENTORY
	uint16_t wPos;
	uint8_t bCount;
#endif
} TPacketCGShop;

enum
{
	EXCHANGE_SUBHEADER_CG_START,	/* arg1 == vid of target character */
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,	/* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ELK_ADD,	/* arg1 == amount of elk */
	EXCHANGE_SUBHEADER_CG_ACCEPT,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_CG_CANCEL,	/* arg1 == not used */
};

typedef struct command_exchange
{
	uint8_t header;
	uint8_t subheader;
	uint32_t arg1;
	uint8_t arg2;
	TItemPos Pos;
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
} TPacketCGExchange;

typedef struct command_position
{
	uint8_t header;
	uint8_t position;
} TPacketCGPosition;

typedef struct command_script_answer
{
	uint8_t header;
	uint8_t answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
	uint8_t header;
	uint32_t idx;
} TPacketCGScriptButton;

typedef struct command_target
{
	uint8_t header;
	uint32_t dwVID;
} TPacketCGTarget;

typedef struct command_move
{
	uint8_t bHeader;
	uint8_t bFunc;
	uint8_t bArg;
	uint8_t bRot;
	LONG lX;
	LONG lY;
	uint32_t dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	uint32_t dwVID;
	long lX;
	long lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting
{
	uint8_t bHeader;
	uint32_t dwTargetVID;
	long lX;
	long lY;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	uint8_t bHeader;
	uint32_t dwShooterVID;
	uint32_t dwTargetVID;
	long lX;
	long lY;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
	uint8_t bHeader;
	uint8_t bType;
} TPacketCGShoot;

typedef struct command_warp
{
	uint8_t bHeader;
} TPacketCGWarp;

enum
{
#ifdef ENABLE_GM_MESSENGER_LIST
	MESSENGER_SUBHEADER_GC_GM_LIST,
	MESSENGER_SUBHEADER_GC_GM_LOGIN,
	MESSENGER_SUBHEADER_GC_GM_LOGOUT,
#endif
#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_GC_BLOCK_LIST,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGIN,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT,
	MESSENGER_SUBHEADER_GC_BLOCK_INVITE, //not used
#endif
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_INVITE,
	MESSENGER_SUBHEADER_GC_MOBILE
};

typedef struct packet_messenger
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_list_offline
{
	uint8_t connected; // always 0
	uint8_t length;
} TPacketGCMessengerListOffline;

#ifdef ENABLE_MESSENGER_BLOCK
typedef struct packet_messenger_block_list_offline
{
	uint8_t connected; // always 0
	uint8_t length;
} TPacketGCMessengerBlockListOffline;

typedef struct packet_messenger_block_list_online
{
	uint8_t connected; // always 1
	uint8_t length;
} TPacketGCMessengerBlockListOnline;
#endif

#ifdef ENABLE_GM_MESSENGER_LIST
typedef struct packet_messenger_gm_list_offline
{
	uint8_t connected; // always 0
	uint8_t length;
} TPacketGCMessengerGMListOffline;

typedef struct packet_messenger_gm_list_online
{
	uint8_t connected; // always 1
	uint8_t length;
} TPacketGCMessengerGMListOnline;
#endif

enum
{
	MESSENGER_CONNECTED_STATE_OFFLINE,
	MESSENGER_CONNECTED_STATE_ONLINE,
	MESSENGER_CONNECTED_STATE_MOBILE
};

typedef struct packet_messenger_list_online
{
	uint8_t connected;
	uint8_t length;
} TPacketGCMessengerListOnline;

typedef struct packet_messenger_login
{
	uint8_t length;
} TPacketGCMessengerLogin;

typedef struct packet_messenger_logout
{
	uint8_t length;
} TPacketGCMessengerLogout;

enum
{
#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_BLOCK_REMOVE_BLOCK,
#endif
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
	//	MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
};

typedef struct command_messenger
{
	uint8_t header;
	uint8_t subheader;
} TPacketCGMessenger;

enum
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW
};

#ifdef ENABLE_SAFEBOX_MONEY
typedef struct command_safebox_money
{
	uint8_t bHeader;
	uint8_t bState;
	uint32_t dwMoney;
} TPacketCGSafeboxMoney;
#endif

typedef struct command_safebox_checkout
{
	uint8_t bHeader;
	uint8_t bSafePos;
	TItemPos ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
	uint8_t bHeader;
	uint8_t bSafePos;
	TItemPos ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_mall_checkout
{
	uint8_t bHeader;
	uint8_t bMallPos;
	TItemPos ItemPos;
} TPacketCGMallCheckout;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
typedef struct command_guildstorage_checkin
{
	uint8_t bHeader;
	uint8_t bSafePos;
	TItemPos ItemPos;
} TPacketCGGuildstorageCheckin;

typedef struct command_guildstorage_checkout
{
	uint8_t bHeader;
	uint8_t bGuildstoragePos;
	TItemPos ItemPos;
} TPacketCGGuildstorageCheckout;
#endif

///////////////////////////////////////////////////////////////////////////////////
// Party

typedef struct command_use_skill
{
	uint8_t bHeader;
	uint32_t dwVnum;
	uint32_t dwTargetVID;
} TPacketCGUseSkill;

typedef struct command_party_invite
{
	uint8_t header;
	uint32_t vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer
{
	uint8_t header;
	uint32_t leader_pid;
	uint8_t accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove
{
	uint8_t header;
	uint32_t pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	uint8_t byHeader;
	uint32_t dwVID;
	uint8_t byState;
	uint8_t byFlag;
} TPacketCGPartySetState;

typedef struct packet_party_link
{
	uint8_t header;
	uint32_t pid;
	uint32_t vid;
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	long mapIdx;
	uint8_t channel;
#endif
} TPacketGCPartyLink;

typedef struct packet_party_unlink
{
	uint8_t header;
	uint32_t pid;
	uint32_t vid;
} TPacketGCPartyUnlink;

typedef struct command_party_use_skill
{
	uint8_t byHeader;
	uint8_t bySkillIndex;
	uint32_t dwTargetVID;
} TPacketCGPartyUseSkill;

enum
{
	GUILD_SUBHEADER_CG_ADD_MEMBER,
	GUILD_SUBHEADER_CG_REMOVE_MEMBER,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
	GUILD_SUBHEADER_CG_OFFER,
	GUILD_SUBHEADER_CG_POST_COMMENT,
	GUILD_SUBHEADER_CG_DELETE_COMMENT,
	GUILD_SUBHEADER_CG_REFRESH_COMMENT,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_CG_USE_SKILL,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
	GUILD_SUBHEADER_CG_CHARGE_GSP,
	GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
#ifdef ENABLE_GUILDBANK_LOG
	GUILD_SUBHEADER_CG_REFRESH,
#endif
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
	GUILD_SUBHEADER_CG_CHANGE_GUILDMASTER,
	GUILD_SUBHEADER_CG_DELETE_LAND,
#endif
};

typedef struct command_guild
{
	uint8_t byHeader;
	uint8_t bySubHeader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	uint8_t header;
	char guild_name[GUILD_NAME_MAX_LEN + 1];
} TPacketCGAnswerMakeGuild;

typedef struct command_give_item
{
	uint8_t byHeader;
	uint32_t dwTargetVID;
	TItemPos ItemPos;
	uint8_t byItemCount;
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
	uint8_t bHeader;
	char szBuf[255 + 1];
} TPacketCGHack;

// Private Shop
typedef struct SShopItemTable
{
	uint32_t vnum;
	uint8_t count;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
	TItemPos pos; // PC-Shop only
	uint32_t price; // PC-Shop & shop_table_ex.txt-Shop only
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	uint8_t limit_level;
#endif
	uint8_t display_pos; // PC-Shop & shop_table_ex.txt-Shop only -> visible location.
} TShopItemTable;

typedef struct SPacketCGMyShop
{
	uint8_t bHeader;
	char szSign[SHOP_SIGN_MAX_LEN + 1];
	uint8_t bCount;
} TPacketCGMyShop;

typedef struct SPacketCGRefine
{
	uint8_t header;
	uint16_t pos; //@fixme425
	uint8_t type;
} TPacketCGRefine;

typedef struct SPacketCGChangeName
{
	uint8_t header;
	uint8_t index;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGChangeName;

typedef struct command_client_version
{
	uint8_t header;
	char filename[32 + 1];
	char timestamp[32 + 1];
} TPacketCGClientVersion;

typedef struct command_client_version2
{
	uint8_t header;
	char filename[32 + 1];
	char timestamp[32 + 1];
} TPacketCGClientVersion2;

enum EPartyExpDistributionType
{
	PARTY_EXP_DISTRIBUTION_NON_PARITY,
	PARTY_EXP_DISTRIBUTION_PARITY,
#ifdef ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL
	PARTY_EXP_DISTRIBUTION_EQUAL,
#endif
};

typedef struct command_party_parameter
{
	uint8_t bHeader;
	uint8_t bDistributeMode;
} TPacketCGPartyParameter;

typedef struct command_quest_input_string
{
	uint8_t bHeader;
	char szString[QUEST_INPUT_STRING_MAX_NUM + 1];
} TPacketCGQuestInputString;

#ifdef ENABLE_OX_RENEWAL
typedef struct command_quest_input_long_string
{
	uint8_t bHeader;
	char szString[QUEST_INPUT_LONG_STRING_MAX_NUM + 1];
} TPacketCGQuestInputLongString;
#endif

typedef struct command_quest_confirm
{
	uint8_t header;
	uint8_t answer;
	uint32_t requestPID;
} TPacketCGQuestConfirm;

typedef struct command_script_select_item
{
	uint8_t header;
	uint32_t selection;
} TPacketCGScriptSelectItem;

enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_DBCLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};

typedef struct packet_phase
{
	uint8_t header;
	uint8_t phase;
} TPacketGCPhase;

typedef struct packet_blank
{
	uint8_t header;
} TPacketGCBlank;

typedef struct packet_blank_dynamic
{
	uint8_t header;
	uint16_t size;
} TPacketGCBlankDynamic;

typedef struct packet_header_handshake
{
	uint8_t header;
	uint32_t dwHandshake;
	uint32_t dwTime;
	LONG lDelta;
} TPacketGCHandshake;

typedef struct packet_header_bindudp
{
	uint8_t header;
	uint32_t addr;
	uint16_t port;
} TPacketGCBindUDP;

typedef struct packet_header_dynamic_size
{
	uint8_t header;
	uint16_t size;
} TDynamicSizePacketHeader;

typedef struct SSimplePlayerInformation
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
	// uint16_t wParts[CRaceData::PART_MAX_NUM];
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
	LONG lAddr;
	uint16_t wPort;
	uint8_t bySkillGroup;
} TSimplePlayerInformation;

typedef struct packet_login_success
{
	uint8_t header;
	TSimplePlayerInformation akSimplePlayerInformation[PLAYER_PER_ACCOUNT];
	uint32_t guild_id[PLAYER_PER_ACCOUNT];
	char guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN + 1];
	uint32_t handle;
	uint32_t random_key;
} TPacketGCLoginSuccess;

enum
{
	LOGIN_STATUS_MAX_LEN = 8
};
typedef struct packet_login_failure
{
	uint8_t header;
	char szStatus[LOGIN_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct command_player_create
{
	uint8_t header;
	uint8_t index;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint16_t job;
	uint8_t shape;
	uint8_t CON;
	uint8_t INT;
	uint8_t STR;
	uint8_t DEX;
} TPacketCGCreateCharacter;

typedef struct command_player_create_success
{
	uint8_t header;
	uint8_t bAccountCharacterSlot;
	TSimplePlayerInformation kSimplePlayerInfomation;
} TPacketGCPlayerCreateSuccess;

typedef struct command_create_failure
{
	uint8_t header;
	uint8_t bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete
{
	uint8_t header;
	uint8_t index;
	char szPrivateCode[PRIVATE_CODE_LENGTH];
} TPacketCGDestroyCharacter;

typedef struct packet_player_delete_success
{
	uint8_t header;
	uint8_t account_index;
} TPacketGCDestroyCharacterSuccess;

#ifdef ENABLE_DELETE_FAILURE_TYPE
typedef struct packet_player_delete_error
{
	uint8_t header;
	uint8_t type;
	uint32_t time;
} TPacketGCDestroyCharacterFailure;
#endif

enum
{
	ADD_CHARACTER_STATE_DEAD = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY = (1 << 4)
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
#ifdef ENABLE_BATTLE_FIELD
	PK_MODE_BATTLE,
#endif
	PK_MODE_MAX_NUM
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	CHR_EQUIPPART_ACCE,
#endif
#ifdef ENABLE_PENDANT
	CHR_EQUIPPART_PENDANT,
#endif
#ifdef ENABLE_AURA_SYSTEM
	CHR_EQUIPPART_AURA,
#endif
	CHR_EQUIPPART_NUM
};

typedef struct packet_add_char
{
	uint8_t header;
	uint32_t dwVID;
#ifdef WJ_SHOW_MOB_INFO
	uint32_t dwLevel;
	uint32_t dwAIFlag;
#endif

	float angle;
	long x;
	long y;
	long z;

	uint8_t bType;
	uint16_t wRaceNum;
	uint8_t bMovingSpeed;
	uint8_t bAttackSpeed;

	uint8_t bStateFlag;
	uint32_t dwAffectFlag[2];
#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	int iPortalMinLevelLimit;
	int iPortalMaxLevelLimit;
#endif
} TPacketGCCharacterAdd;

typedef struct packet_char_additional_info
{
	uint8_t header;
	uint32_t dwVID;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t adwPart[CHR_EQUIPPART_NUM]; //@fixme411 - uint16_t -> uint32_t
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t dwElementsEffect;
#endif
	uint8_t bEmpire;
	uint32_t dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t dwNewIsGuildName;
#endif
	uint32_t dwLevel;
	int16_t sAlignment;
	uint8_t bPKMode;
	uint32_t dwMountVnum;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	uint32_t dwTitle;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	uint32_t dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	long lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t dwcLevel;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_update_char
{
	uint8_t header;
	uint32_t dwVID;

	uint32_t adwPart[CHR_EQUIPPART_NUM]; //@fixme411 - uint16_t -> uint32_t
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t dwElementsEffect;
#endif
	uint8_t bMovingSpeed;
	uint8_t bAttackSpeed;

	uint8_t bStateFlag;
	uint32_t dwAffectFlag[2];

	uint32_t dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t dwNewIsGuildName;
#endif
	int16_t sAlignment;
#ifdef WJ_SHOW_MOB_INFO
	uint32_t dwLevel;
#endif
	uint8_t bPKMode;
	uint32_t dwMountVnum;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	uint32_t dwTitle;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	uint32_t dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	long lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t dwcLevel;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	uint8_t header;
	uint32_t dwVID;
} TPacketGCCharacterDelete;

enum EChatType
{
	CHAT_TYPE_TALKING,
	CHAT_TYPE_INFO,
	CHAT_TYPE_NOTICE,
	CHAT_TYPE_PARTY,
	CHAT_TYPE_GUILD,
	CHAT_TYPE_COMMAND,
	CHAT_TYPE_SHOUT,
	CHAT_TYPE_WHISPER,
	CHAT_TYPE_BIG_NOTICE,
	CHAT_TYPE_MONARCH_NOTICE,
#ifdef ENABLE_OX_RENEWAL
	CHAT_TYPE_CONTROL_NOTICE,
#endif
#ifdef ENABLE_DICE_SYSTEM
	CHAT_TYPE_DICE_INFO,
#endif
#ifdef ENABLE_12ZI
	CHAT_TYPE_MISSION,
	CHAT_TYPE_SUB_MISSION,
	CHAT_TYPE_CLEAR_MISSION,
#endif
#ifdef ENABLE_CHAT_SETTINGS_EXTEND
	CHAT_TYPE_EXP_INFO,
	CHAT_TYPE_ITEM_INFO,
	CHAT_TYPE_MONEY_INFO,
#endif
	CHAT_TYPE_MAX_NUM
};

typedef struct packet_chatting
{
	uint8_t header;
	uint16_t size;
	uint8_t type;
	uint32_t dwVID;
	uint8_t bEmpire;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	bool bCanFormat;
	uint8_t bLanguage;
#endif
#ifdef ENABLE_EMPIRE_FLAG
	bool bFlag;
#endif
} TPacketGCChat;

typedef struct packet_whisper
{
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bType;
	char szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	bool bCanFormat;
#endif
} TPacketGCWhisper;

typedef struct packet_stun
{
	uint8_t header;
	uint32_t vid;
} TPacketGCStun;

typedef struct packet_dead
{
	uint8_t header;
	uint32_t vid;
#ifdef ENABLE_BATTLE_FIELD
	bool bRestart;
	long lMapIdx;
#endif
} TPacketGCDead;

typedef struct packet_main_character
{
	uint8_t header;
	uint32_t dwVID;
	uint16_t wRaceNum;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lX, lY, lZ;
	uint8_t byEmpire;
	uint8_t bySkillGroup;
} TPacketGCMainCharacter;

typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24
	};

	uint8_t header;
	uint32_t dwVID;
	uint16_t wRaceNum;
	char szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char szBGMName[MUSIC_NAME_MAX_LEN + 1];
	long lX, lY, lZ;
	uint8_t byEmpire;
	uint8_t bySkillGroup;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24
	};

	uint8_t header;
	uint32_t dwVID;
	uint16_t wRaceNum;
	char szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char szBGMName[MUSIC_NAME_MAX_LEN + 1];
	float fBGMVol;
	long lX, lY, lZ;
	uint8_t byEmpire;
	uint8_t bySkillGroup;
} TPacketGCMainCharacter4_BGM_VOL;

enum EPointTypes
{
	POINT_NONE,						// 0	- APPLY_NONE,	0
	POINT_LEVEL,					// 1	- 
	POINT_VOICE,					// 2	- 
	POINT_EXP,						// 3	- 
	POINT_NEXT_EXP,					// 4	- 
	POINT_HP,						// 5	- 
	POINT_MAX_HP,					// 6	- APPLY_MAX_HP,	1
	POINT_SP,						// 7	- 
	POINT_MAX_SP,					// 8	- APPLY_MAX_SP,	2
	POINT_STAMINA,					// 9	- 
	POINT_MAX_STAMINA,				// 10	- APPLY_MAX_STAMINA,	58
	POINT_GOLD,						// 11	- 
	POINT_ST,						// 12	- APPLY_STR,	5
	POINT_HT,						// 13	- APPLY_CON,	3
	POINT_DX,						// 14	- APPLY_DEX,	6
	POINT_IQ,						// 15	- APPLY_INT,	4
	POINT_ATT_POWER,				// 16	- // different to source
	POINT_ATT_SPEED,				// 17	- APPLY_ATT_SPEED,	7
	POINT_EVADE_RATE,				// 18	- // different to source
	POINT_MOV_SPEED,				// 19	- APPLY_MOV_SPEED,	8
	POINT_DEF_GRADE,				// 20	- // different to source
	POINT_CASTING_SPEED,			// 21	- APPLY_CAST_SPEED,	9
	POINT_MAGIC_ATT_GRADE,			// 22	- 
	POINT_MAGIC_DEF_GRADE,			// 23	- 
	POINT_EMPIRE_POINT,				// 24	- 
	POINT_LEVEL_STEP,				// 25	- 
	POINT_STAT,						// 26	- 
	POINT_SUB_SKILL,				// 27	- 
	POINT_SKILL,					// 28	- APPLY_SKILL,	51
	//	POINT_SKILL_PASV,	// 27
	//	POINT_SKILL_ACTIVE,	// 28 
	POINT_MIN_ATK,					// 29	- 
	POINT_MAX_ATK,					// 30	- 
	POINT_PLAYTIME,					// 31	- 
	POINT_HP_REGEN,					// 32	- APPLY_HP_REGEN,	10
	POINT_SP_REGEN,					// 33	- APPLY_SP_REGEN,	11
	POINT_BOW_DISTANCE,				// 34	- APPLY_BOW_DISTANCE,	52
	POINT_HP_RECOVERY,				// 35	- 
	POINT_SP_RECOVERY,				// 36	- 
	POINT_POISON_PCT,				// 37	- APPLY_POISON_PCT,	12
	POINT_STUN_PCT,					// 38	- APPLY_STUN_PCT,	13
	POINT_SLOW_PCT,					// 39	- APPLY_SLOW_PCT,	14
	POINT_CRITICAL_PCT,				// 40	- APPLY_CRITICAL_PCT,	15
	POINT_PENETRATE_PCT,			// 41	- APPLY_PENETRATE_PCT,	16
	POINT_CURSE_PCT,				// 42	- APPLY_CURSE_PCT,	57
	POINT_ATTBONUS_HUMAN,			// 43	- APPLY_ATTBONUS_HUMAN,	17
	POINT_ATTBONUS_ANIMAL,			// 44	- APPLY_ATTBONUS_ANIMAL,	18
	POINT_ATTBONUS_ORC,				// 45	- APPLY_ATTBONUS_ORC,	19
	POINT_ATTBONUS_MILGYO,			// 46	- APPLY_ATTBONUS_MILGYO,	20
	POINT_ATTBONUS_UNDEAD,			// 47	- APPLY_ATTBONUS_UNDEAD,	21
	POINT_ATTBONUS_DEVIL,			// 48	- APPLY_ATTBONUS_DEVIL,	22
	POINT_ATTBONUS_INSECT,			// 49	- APPLY_ATTBONUS_INSECT,	102
	POINT_ATTBONUS_FIRE,			// 50	- 
	POINT_ATTBONUS_ICE,				// 51	- 
	POINT_ATTBONUS_DESERT,			// 52	- APPLY_ATTBONUS_DESERT,	101
	POINT_ATTBONUS_MONSTER,			// 53	- APPLY_ATTBONUS_MONSTER,	63
	POINT_ATTBONUS_WARRIOR,			// 54	- APPLY_ATTBONUS_WARRIOR,	59
	POINT_ATTBONUS_ASSASSIN,		// 55	- APPLY_ATTBONUS_ASSASSIN,	60
	POINT_ATTBONUS_SURA,			// 56	- APPLY_ATTBONUS_SURA,	61
	POINT_ATTBONUS_SHAMAN,			// 57	- APPLY_ATTBONUS_SHAMAN,	62
	POINT_ATTBONUS_UNUSED5,			// 58	- xxx
	POINT_RESIST_WARRIOR,			// 59	- APPLY_RESIST_WARRIOR,	78
	POINT_RESIST_ASSASSIN,			// 60	- APPLY_RESIST_ASSASSIN,	79
	POINT_RESIST_SURA,				// 61	- APPLY_RESIST_SURA,	80
	POINT_RESIST_SHAMAN,			// 62	- APPLY_RESIST_SHAMAN,	81
	POINT_STEAL_HP,					// 63	- APPLY_STEAL_HP,	23
	POINT_STEAL_SP,					// 64	- APPLY_STEAL_SP,	24
	POINT_MANA_BURN_PCT,			// 65	- APPLY_MANA_BURN_PCT,	25
	POINT_DAMAGE_SP_RECOVER,		// 66	- APPLY_DAMAGE_SP_RECOVER,	26
	POINT_BLOCK,					// 67	- APPLY_BLOCK,	27
	POINT_DODGE,					// 68	- APPLY_DODGE,	28
	POINT_RESIST_SWORD,				// 69	- APPLY_RESIST_SWORD,	29
	POINT_RESIST_TWOHAND,			// 70	- APPLY_RESIST_TWOHAND,	30
	POINT_RESIST_DAGGER,			// 71	- APPLY_RESIST_DAGGER,	31
	POINT_RESIST_BELL,				// 72	- APPLY_RESIST_BELL,	32
	POINT_RESIST_FAN,				// 73	- APPLY_RESIST_FAN,	33
	POINT_RESIST_BOW,				// 74	- APPLY_RESIST_BOW,	34
	POINT_RESIST_FIRE,				// 75	- APPLY_RESIST_FIRE,	35
	POINT_RESIST_ELEC,				// 76	- APPLY_RESIST_ELEC,	36
	POINT_RESIST_MAGIC,				// 77	- APPLY_RESIST_MAGIC,	37
	POINT_RESIST_WIND,				// 78	- APPLY_RESIST_WIND,	38
	POINT_REFLECT_MELEE,			// 79	- APPLY_REFLECT_MELEE,	39
	POINT_REFLECT_CURSE,			// 80	- APPLY_REFLECT_CURSE,	40
	POINT_POISON_REDUCE,			// 81	- APPLY_POISON_REDUCE,	41
	POINT_KILL_SP_RECOVER,			// 82	- APPLY_KILL_SP_RECOVER,	42
	POINT_EXP_DOUBLE_BONUS,			// 83	- APPLY_EXP_DOUBLE_BONUS,	43
	POINT_GOLD_DOUBLE_BONUS,		// 84	- APPLY_GOLD_DOUBLE_BONUS,	44
	POINT_ITEM_DROP_BONUS,			// 85	- APPLY_ITEM_DROP_BONUS,	45
	POINT_POTION_BONUS,				// 86	- APPLY_POTION_BONUS,	46
	POINT_KILL_HP_RECOVER,			// 87	- APPLY_KILL_HP_RECOVER,	47
	POINT_IMMUNE_STUN,				// 88	- APPLY_IMMUNE_STUN,	48
	POINT_IMMUNE_SLOW,				// 89	- APPLY_IMMUNE_SLOW,	49
	POINT_IMMUNE_FALL,				// 90	- APPLY_IMMUNE_FALL,	50
	POINT_PARTY_ATT_GRADE,			// 91	- xxx
	POINT_PARTY_DEF_GRADE,			// 92	- xxx
	POINT_ATT_BONUS,				// 93	- APPLY_MALL_ATTBONUS	64
	POINT_DEF_BONUS,				// 94	- 
	POINT_ATT_GRADE_BONUS,			// 95	- APPLY_ATT_GRADE,	53
	POINT_DEF_GRADE_BONUS,			// 96	- APPLY_DEF_GRADE,	54
	POINT_MAGIC_ATT_GRADE_BONUS,	// 97	- APPLY_MAGIC_ATT_GRADE,	55
	POINT_MAGIC_DEF_GRADE_BONUS,	// 98	- APPLY_MAGIC_DEF_GRADE,	56
	POINT_RESIST_NORMAL_DAMAGE,		// 99	- 
	POINT_HIT_HP_RECOVERY,			// 100	- 
	POINT_HIT_SP_RECOVERY,			// 101	- 
	POINT_MANASHIELD,				// 102	- 
	POINT_PARTY_BUFFER_BONUS,		// 103	- 
	POINT_PARTY_SKILL_MASTER_BONUS,	// 104	- 
	POINT_HP_RECOVER_CONTINUE,		// 105	- 
	POINT_SP_RECOVER_CONTINUE,		// 106	- 
	POINT_STEAL_GOLD,				// 107	- 
	POINT_POLYMORPH,				// 108	- 
	POINT_MOUNT,					// 109	- 
	POINT_PARTY_HASTE_BONUS,		// 110	- 
	POINT_PARTY_DEFENDER_BONUS,		// 111	- 
	POINT_STAT_RESET_COUNT,			// 112	- 
	POINT_HORSE_SKILL,				// 113	- 
	POINT_MALL_ATTBONUS,			// 114	- 
	POINT_MALL_DEFBONUS,			// 115	- APPLY_MALL_DEFBONUS,	65
	POINT_MALL_EXPBONUS,			// 116	- APPLY_MALL_EXPBONUS,	66
	POINT_MALL_ITEMBONUS,			// 117	- APPLY_MALL_ITEMBONUS,	67
	POINT_MALL_GOLDBONUS,			// 118	- APPLY_MALL_GOLDBONUS,	68
	POINT_MAX_HP_PCT,				// 119	- APPLY_MAX_HP_PCT,	69
	POINT_MAX_SP_PCT,				// 120	- APPLY_MAX_SP_PCT,	70
	POINT_SKILL_DAMAGE_BONUS,		// 121	- APPLY_SKILL_DAMAGE_BONUS,	71
	POINT_NORMAL_HIT_DAMAGE_BONUS,	// 122	- APPLY_NORMAL_HIT_DAMAGE_BONUS,	72
	POINT_SKILL_DEFEND_BONUS,		// 123	- APPLY_SKILL_DEFEND_BONUS,	73
	POINT_NORMAL_HIT_DEFEND_BONUS,	// 124	- APPLY_NORMAL_HIT_DEFEND_BONUS,	74
	POINT_PC_BANG_EXP_BONUS,		// 125	- APPLY_PC_BANG_EXP_BONUS,	75
	POINT_PC_BANG_DROP_BONUS,		// 126	- APPLY_PC_BANG_DROP_BONUS,	76
	POINT_RAMADAN_CANDY_BONUS_EXP,	// 127	- xxx
	POINT_ENERGY,					// 128	- APPLY_ENERGY,	82
	POINT_ENERGY_END_TIME,			// 129	- 
	POINT_COSTUME_ATTR_BONUS,		// 130	- APPLY_COSTUME_ATTR_BONUS,	84
	POINT_MAGIC_ATT_BONUS_PER,		// 131	- APPLY_MAGIC_ATTBONUS_PER,	85
	POINT_MELEE_MAGIC_ATT_BONUS_PER,// 132	- APPLY_MELEE_MAGIC_ATTBONUS_PER,	86
	POINT_RESIST_ICE,				// 133	- APPLY_RESIST_ICE,	87
	POINT_RESIST_EARTH,				// 134	- APPLY_RESIST_EARTH,	88
	POINT_RESIST_DARK,				// 135	- APPLY_RESIST_DARK,	89
	POINT_RESIST_CRITICAL,			// 136	- APPLY_ANTI_CRITICAL_PCT,	90
	POINT_RESIST_PENETRATE,			// 137	- APPLY_ANTI_PENETRATE_PCT,	91
	POINT_BLEEDING_REDUCE,			// 138	- APPLY_BLEEDING_REDUCE,	92
	POINT_BLEEDING_PCT,				// 139	- APPLY_BLEEDING_PCT,	93
	POINT_ATTBONUS_WOLFMAN,			// 140	- APPLY_ATTBONUS_WOLFMAN,	94
	POINT_RESIST_WOLFMAN,			// 141	- APPLY_RESIST_WOLFMAN,	95
	POINT_RESIST_CLAW,				// 142	- APPLY_RESIST_CLAW,	96
	POINT_ACCEDRAIN_RATE,			// 143	- APPLY_ACCEDRAIN_RATE,	97
	POINT_RESIST_MAGIC_REDUCTION,	// 144	- APPLY_RESIST_MAGIC_REDUCTION,	98
	POINT_ENCHANT_ELECT,
	POINT_ENCHANT_FIRE,
	POINT_ENCHANT_ICE,
	POINT_ENCHANT_WIND,
	POINT_ENCHANT_EARTH,
	POINT_ENCHANT_DARK,
	POINT_ATTBONUS_CZ,
	POINT_ATTBONUS_SWORD,
	POINT_ATTBONUS_TWOHAND,
	POINT_ATTBONUS_DAGGER,
	POINT_ATTBONUS_BELL,
	POINT_ATTBONUS_FAN,
	POINT_ATTBONUS_BOW,
	POINT_ATTBONUS_CLAW,
	POINT_RESIST_HUMAN,
	POINT_RESIST_MOUNT_FALL,

#ifdef ENABLE_YOHARA_SYSTEM
	POINT_CONQUEROR_LEVEL = 165, // 295
	POINT_CONQUEROR_LEVEL_STEP = 166,
	POINT_CONQUEROR_EXP = 167,
	POINT_CONQUEROR_NEXT_EXP = 168,
	POINT_CONQUEROR_POINT = 169, // 299
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	POINT_CHEQUE = 190, // 145 the correct one
#endif
#ifdef ENABLE_GEM_SYSTEM
	POINT_GEM = 191,
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	POINT_MEDAL_OF_HONOR = 192,
#endif
#ifdef ENABLE_REFINE_ELEMENT
	POINT_ATT_ELEMENTS = 193,
#endif
#ifdef ENABLE_BATTLE_FIELD
	POINT_BATTLE_FIELD = 194,
#endif

	POINT_MIN_WEP = 200,
	POINT_MAX_WEP,
	POINT_MIN_MAGIC_WEP,
	POINT_MAX_MAGIC_WEP,
	POINT_HIT_RATE,

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	POINT_RESIST_FIST = 205,
	POINT_SKILL_DAMAGE_SAMYEON = 206,
	POINT_SKILL_DAMAGE_TANHWAN = 207,
	POINT_SKILL_DAMAGE_PALBANG = 208,
	POINT_SKILL_DAMAGE_GIGONGCHAM = 209,
	POINT_SKILL_DAMAGE_GYOKSAN = 210,
	POINT_SKILL_DAMAGE_GEOMPUNG = 211,
	POINT_SKILL_DAMAGE_AMSEOP = 212,
	POINT_SKILL_DAMAGE_GUNGSIN = 213,
	POINT_SKILL_DAMAGE_CHARYUN = 214,
	POINT_SKILL_DAMAGE_SANGONG = 215,
	POINT_SKILL_DAMAGE_YEONSA = 216,
	POINT_SKILL_DAMAGE_KWANKYEOK = 217,
	POINT_SKILL_DAMAGE_GIGUNG = 218,
	POINT_SKILL_DAMAGE_HWAJO = 219,
	POINT_SKILL_DAMAGE_SWAERYUNG = 220,
	POINT_SKILL_DAMAGE_YONGKWON = 221,
	POINT_SKILL_DAMAGE_PABEOB = 222,
	POINT_SKILL_DAMAGE_MARYUNG = 223,
	POINT_SKILL_DAMAGE_HWAYEOMPOK = 224,
	POINT_SKILL_DAMAGE_MAHWAN = 225,
	POINT_SKILL_DAMAGE_BIPABU = 226,
	POINT_SKILL_DAMAGE_YONGBI = 227,
	POINT_SKILL_DAMAGE_PAERYONG = 228,
	POINT_SKILL_DAMAGE_NOEJEON = 229,
	POINT_SKILL_DAMAGE_BYEURAK = 230,
	POINT_SKILL_DAMAGE_CHAIN = 231,
	POINT_SKILL_DAMAGE_CHAYEOL = 232,
	POINT_SKILL_DAMAGE_SALPOONG = 233,
	POINT_SKILL_DAMAGE_GONGDAB = 234,
	POINT_SKILL_DAMAGE_PASWAE = 235,
	POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE = 236,
	POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE = 237,
	POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE = 238,
	POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE = 239,
	POINT_HIT_BUFF_ENCHANT_FIRE = 240,
	POINT_HIT_BUFF_ENCHANT_ICE = 241,
	POINT_HIT_BUFF_ENCHANT_ELEC = 242,
	POINT_HIT_BUFF_ENCHANT_WIND = 243,
	POINT_HIT_BUFF_ENCHANT_DARK = 244,
	POINT_HIT_BUFF_ENCHANT_EARTH = 245,
	POINT_HIT_BUFF_RESIST_FIRE = 246,
	POINT_HIT_BUFF_RESIST_ICE = 247,
	POINT_HIT_BUFF_RESIST_ELEC = 248,
	POINT_HIT_BUFF_RESIST_WIND = 249,
	POINT_HIT_BUFF_RESIST_DARK = 250,
	POINT_HIT_BUFF_RESIST_EARTH = 251,
	POINT_USE_SKILL_CHEONGRANG_MOV_SPEED = 252,
	POINT_USE_SKILL_CHEONGRANG_CASTING_SPEED = 253,
	POINT_USE_SKILL_CHAYEOL_CRITICAL_PCT = 254,
	POINT_USE_SKILL_SANGONG_ATT_GRADE_BONUS = 255,
	POINT_USE_SKILL_GIGUNG_ATT_GRADE_BONUS = 256,
	POINT_USE_SKILL_JEOKRANG_DEF_BONUS = 257,
	POINT_USE_SKILL_GWIGEOM_DEF_BONUS = 258,
	POINT_USE_SKILL_TERROR_ATT_GRADE_BONUS = 259,
	POINT_USE_SKILL_MUYEONG_ATT_GRADE_BONUS = 260,
	POINT_USE_SKILL_MANASHILED_CASTING_SPEED = 261,
	POINT_USE_SKILL_HOSIN_DEF_BONUS = 262,
	POINT_USE_SKILL_GICHEON_ATT_GRADE_BONUS = 263,
	POINT_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS = 264,
	POINT_USE_SKILL_JEUNGRYEOK_DEF_BONUS = 265,
	POINT_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS = 266,
	POINT_USE_SKILL_CHUNKEON_CASTING_SPEED = 267,
	POINT_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS = 268,
	POINT_SKILL_DURATION_INCREASE_EUNHYUNG = 269,
	POINT_SKILL_DURATION_INCREASE_GYEONGGONG = 270,
	POINT_SKILL_DURATION_INCREASE_GEOMKYUNG = 271,
	POINT_SKILL_DURATION_INCREASE_JEOKRANG = 272,
	POINT_USE_SKILL_PALBANG_HP_ABSORB = 273,
	POINT_USE_SKILL_AMSEOP_HP_ABSORB = 274,
	POINT_USE_SKILL_YEONSA_HP_ABSORB = 275,
	POINT_USE_SKILL_YONGBI_HP_ABSORB = 276,
	POINT_USE_SKILL_CHAIN_HP_ABSORB = 277,
	POINT_USE_SKILL_PASWAE_SP_ABSORB = 278,
	POINT_USE_SKILL_GIGONGCHAM_STUN = 279,
	POINT_USE_SKILL_CHARYUN_STUN = 280,
	POINT_USE_SKILL_PABEOB_STUN = 281,
	POINT_USE_SKILL_MAHWAN_STUN = 282,
	POINT_USE_SKILL_GONGDAB_STUN = 283,
	POINT_USE_SKILL_SAMYEON_STUN = 284,
	POINT_USE_SKILL_GYOKSAN_KNOCKBACK = 285,
	POINT_USE_SKILL_SEOMJEON_KNOCKBACK = 286,
	POINT_USE_SKILL_SWAERYUNG_KNOCKBACK = 287,
	POINT_USE_SKILL_HWAYEOMPOK_KNOCKBACK = 288,
	POINT_USE_SKILL_GONGDAB_KNOCKBACK = 289,
	POINT_USE_SKILL_KWANKYEOK_KNOCKBACK = 290,
	POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER = 291,
	POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER = 292,
	POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER = 293,
	POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER = 294,
	POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER = 295,
	POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER = 296,
	POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER = 297,
	POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER = 298,
	POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER = 299,
	POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER = 300,
	POINT_ATTBONUS_STONE = 301,
	POINT_DAMAGE_HP_RECOVERY = 302,
	POINT_DAMAGE_SP_RECOVERY = 303,
	POINT_ALIGNMENT_DAMAGE_BONUS = 304,
	POINT_NORMAL_DAMAGE_GUARD = 305,
	POINT_MORE_THEN_HP90_DAMAGE_REDUCE = 306,
	POINT_USE_SKILL_TUSOK_HP_ABSORB = 307,
	POINT_USE_SKILL_PAERYONG_HP_ABSORB = 308,
	POINT_USE_SKILL_BYEURAK_HP_ABSORB = 309,
	POINT_FIRST_ATTRIBUTE_BONUS = 310,
	POINT_SECOND_ATTRIBUTE_BONUS = 311,
	POINT_THIRD_ATTRIBUTE_BONUS = 312,
	POINT_FOURTH_ATTRIBUTE_BONUS = 313,
	POINT_FIFTH_ATTRIBUTE_BONUS = 314,
	POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER = 315,
	POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER = 316,
	POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER = 317,
	POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER = 318,
	POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER = 319,
	POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER = 320,
	POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER = 321,
	POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER = 322,
	POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER = 323,
	POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER = 324,
	POINT_USE_SKILL_CHAYEOL_HP_ABSORB = 325,
	POINT_SUNGMA_STR = 326,
	POINT_SUNGMA_HP = 327,
	POINT_SUNGMA_MOVE = 328,
	POINT_SUNGMA_IMMUNE = 329,
	POINT_HIT_PCT = 330,
	POINT_RANDOM = 331,
	POINT_ATTBONUS_PER_HUMAN = 332,
	POINT_ATTBONUS_PER_ANIMAL = 333,
	POINT_ATTBONUS_PER_ORC = 334,
	POINT_ATTBONUS_PER_MILGYO = 335,
	POINT_ATTBONUS_PER_UNDEAD = 336,
	POINT_ATTBONUS_PER_DEVIL = 337,
	POINT_ENCHANT_PER_ELECT = 338,
	POINT_ENCHANT_PER_FIRE = 339,
	POINT_ENCHANT_PER_ICE = 340,
	POINT_ENCHANT_PER_WIND = 341,
	POINT_ENCHANT_PER_EARTH = 342,
	POINT_ENCHANT_PER_DARK = 343,
	POINT_ATTBONUS_PER_CZ = 344,
	POINT_ATTBONUS_PER_INSECT = 345,
	POINT_ATTBONUS_PER_DESERT = 346,
	POINT_ATTBONUS_PER_STONE = 347,
	POINT_ATTBONUS_PER_MONSTER = 348,
	POINT_RESIST_PER_HUMAN = 349,
	POINT_RESIST_PER_ICE = 350,
	POINT_RESIST_PER_DARK = 351,
	POINT_RESIST_PER_EARTH = 352,
	POINT_RESIST_PER_FIRE = 353,
	POINT_RESIST_PER_ELEC = 354,
	POINT_RESIST_PER_MAGIC = 355,
	POINT_RESIST_PER_WIND = 356,
	POINT_HIT_BUFF_SUNGMA_STR = 357,
	POINT_HIT_BUFF_SUNGMA_MOVE = 358,
	POINT_HIT_BUFF_SUNGMA_HP = 359,
	POINT_HIT_BUFF_SUNGMA_IMMUNE = 360,
	POINT_MOUNT_MELEE_MAGIC_ATTBONUS_PER = 361,
	POINT_DISMOUNT_MOVE_SPEED_BONUS_PER = 362,
	POINT_HIT_AUTO_HP_RECOVERY = 363,
	POINT_HIT_AUTO_SP_RECOVERY = 364,
	POINT_USE_SKILL_COOLTIME_DECREASE_ALL = 365,
	POINT_HIT_STONE_ATTBONUS_STONE = 366,
	POINT_HIT_STONE_DEF_GRADE_BONUS = 367,
	POINT_KILL_BOSS_ITEM_BONUS = 368,
	POINT_MOB_HIT_MOB_AGGRESSIVE = 369,
	POINT_NO_DEATH_AND_HP_RECOVERY30 = 370,
	POINT_AUTO_PICKUP = 371,
	POINT_MOUNT_NO_KNOCKBACK = 372,
	POINT_SUNGMA_PER_STR = 373,
	POINT_SUNGMA_PER_HP = 374,
	POINT_SUNGMA_PER_MOVE = 375,
	POINT_SUNGMA_PER_IMMUNE = 376,
	POINT_IMMUNE_POISON100 = 377,
	POINT_IMMUNE_BLEEDING100 = 378,
	POINT_MONSTER_DEFEND_BONUS = 379,
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	POINT_SOUL,
	POINT_SOUL_RE,
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	POINT_BATTLE_PASS_PREMIUM_ID,
#endif

#ifdef ENABLE_FLOWER_EVENT
	POINT_FLOWER_TYPE_1,
	POINT_FLOWER_TYPE_2,
	POINT_FLOWER_TYPE_3,
	POINT_FLOWER_TYPE_4,
	POINT_FLOWER_TYPE_5,
	POINT_FLOWER_TYPE_6,
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	POINT_NEW_EQUIPMENT_ACTUAL,
	POINT_NEW_EQUIPMENT_TOTAL,
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	POINT_LOTTO_MONEY,
	POINT_LOTTO_TOTAL_MONEY,
#endif

#ifdef ENABLE_FISHING_RENEWAL
	POINT_FISHING_RARE,
#endif

	//POINT_MAX_NUM = 255,=>stdafx.h 로/
};

typedef struct packet_points
{
	uint8_t header;
	long points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_point_change
{
	int header;
	uint32_t dwVID;
	uint16_t wType; //@fixme436
	long amount;
	long value;
} TPacketGCPointChange;

typedef struct packet_motion
{
	uint8_t header;
	uint32_t vid;
	uint32_t victim_vid;
	uint16_t motion;
} TPacketGCMotion;

typedef struct packet_item_del_deprecated
{
	uint8_t header;
	TItemPos Cell;
	uint32_t vnum;
	uint8_t count;
#ifdef ENABLE_SOULBIND_SYSTEM
	long nSealDate;
#endif
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
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TPacketGCItemDelDeprecated;

typedef struct packet_set_item
{
	uint8_t header;
	TItemPos Cell;
	uint32_t vnum;
	uint8_t count;
	uint32_t flags;
	uint32_t anti_flags;
	bool highlight;
#ifdef ENABLE_SOULBIND_SYSTEM
	long nSealDate;
#endif
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
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TPacketGCItemSet;

typedef struct packet_item_del
{
	uint8_t header;
	uint8_t pos;
} TPacketGCItemDel;

typedef struct packet_use_item
{
	uint8_t header;
	TItemPos Cell;
	uint32_t ch_vid;
	uint32_t victim_vid;
	uint32_t vnum;
} TPacketGCItemUse;

typedef struct packet_update_item
{
	uint8_t header;
	TItemPos Cell;
	uint8_t count;
#ifdef ENABLE_SOULBIND_SYSTEM
	long nSealDate;
#endif
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
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TPacketGCItemUpdate;

typedef struct packet_ground_add_item
{
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	packet_ground_add_item()
	{
		memset(&alSockets, 0, sizeof(alSockets));
		memset(&aAttrs, 0, sizeof(aAttrs));
	}
#endif

	uint8_t bHeader;
	long lX;
	long lY;
	long lZ;
	uint32_t dwVID;
	uint32_t dwVnum;
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttrs[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#endif
} TPacketGCItemGroundAdd;

typedef struct packet_ground_del_item
{
	uint8_t header;
	uint32_t vid;
} TPacketGCItemGroundDel;

typedef struct packet_item_ownership
{
	uint8_t bHeader;
	uint32_t dwVID;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_quickslot_add
{
	uint8_t header;
	uint8_t pos;
	TQuickSlot slot;
} TPacketGCQuickSlotAdd;

typedef struct packet_quickslot_del
{
	uint8_t header;
	uint8_t pos;
} TPacketGCQuickSlotDel;

typedef struct packet_quickslot_swap
{
	uint8_t header;
	uint8_t pos;
	uint8_t change_pos;
} TPacketGCQuickSlotSwap;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct packet_shop_set
{
	uint8_t header;
	uint8_t pos;
	uint32_t vnum;
	uint8_t count;
	uint32_t price;
# ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
# endif
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
# ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
# endif
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwLookVnum;
# endif
# ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
	uint8_t grade_element;
	uint32_t attack_element[MAX_ELEMENTS_SPELL];
	uint8_t element_type_bonus;
	int16_t elements_value_bonus[MAX_ELEMENTS_SPELL];
# endif
# ifdef ENABLE_SET_ITEM
	uint8_t set_value;
# endif
} TPacketPlayerShopSet;

typedef struct packet_shop_sign
{
	uint8_t header;
	char sign[SHOP_SIGN_MAX_LEN + 1];
} TPacketMyShopSign;
#endif

typedef struct packet_shop_start
{
#ifdef ENABLE_MYSHOP_DECO
	uint8_t shop_tab_count;
	struct packet_shop_item items[SHOP_HOST_ITEM_MAX];
#else
	struct packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
#endif
#ifdef ENABLE_12ZI
	bool islimiteditemshop;
#endif
} TPacketGCShopStart;

typedef struct packet_shop_start_ex
{
	typedef struct sub_packet_shop_tab
	{
		char name[SHOP_TAB_NAME_MAX];
		uint8_t coin_type;
#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
		int limitMaxPoint;
		int usablePoints;
#endif
		packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
	} TSubPacketShopTab;
	uint32_t owner_vid;
	uint8_t shop_tab_count;
} TPacketGCShopStartEx;

typedef struct packet_shop_update_item
{
	uint8_t pos;
	struct packet_shop_item item;
} TPacketGCShopUpdateItem;

enum EPacketShopSubHeaders
{
	SHOP_SUBHEADER_GC_START = 1,
	SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
	SHOP_SUBHEADER_GC_OK,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
	SHOP_SUBHEADER_GC_SOLDOUT,
	SHOP_SUBHEADER_GC_INVENTORY_FULL,
	SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
	SHOP_SUBHEADER_GC_START_EX,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
	//#ifdef ENABLE_BATTLE_FIELD
	SHOP_SUBHEADER_GC_NOT_ENOUGH_BATTLE_POINTS,
	//#endif
	//#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
	SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY,
	//#endif
	//#ifdef ENABLE_MYSHOP_DECO
	SHOP_SUBHEADER_GC_MYPRIV_SHOP_OPEN,
	//#endif
	//#ifdef ENABLE_10TH_EVENT
	SHOP_SUBHEADER_GC_NOT_ENOUGH_10TH_COIN,
	//#endif
	//#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER,
	SHOP_SUBHEADER_GC_LIMITED_DATA_LOADING,
	//#endif
	SHOP_SUBHEADER_GC_UNK_19, // sub_981F780
	SHOP_SUBHEADER_GC_UNK_20, // sub_981F830
#//if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	SHOP_SUBHEADER_GC_UNK_21,
	//#endif
	//#ifdef ENABLE_MEDAL_OF_HONOR
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MEDAL_OF_HONOR,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_GUILD_LEVEL,
	//#endif
	//#ifdef ENABLE_MINI_GAME_BNW
	SHOP_SUBHEADER_GC_NOT_ENOUGH_BNW,
	//#endif

		// Custom
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	SHOP_SUBHEADER_GC_OPEN_SHOP_EDITOR,
#endif
#ifdef ENABLE_RENEWAL_SHOPEX
	SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_EXP,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	SHOP_SUBHEADER_GC_NOT_ENOUGH_ACHIEVEMENT,
#endif
};

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
typedef struct SPacketGCShopUpdateLimitedPurchase
{
	uint32_t vnum;
	uint32_t limited_count;
	uint32_t limited_purchase_count;
} TPacketGCShopUpdateLimitedPurchase;
#endif

typedef struct packet_shop
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
} TPacketGCShop;

typedef struct packet_exchange
{
	uint8_t header;
	uint8_t subheader;
	uint8_t is_me;
	uint32_t arg1;
	TItemPos arg2;
	uint32_t arg3;
#ifdef WJ_ENABLE_TRADABLE_ICON
	TItemPos arg4;
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
	long alValues[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
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
#ifdef ENABLE_GROWTH_PET_SYSTEM
	TGrowthPetInfo aPetInfo{ 0 };
#endif
} TPacketGCExchange;

enum EPacketTradeSubHeaders
{
	EXCHANGE_SUBHEADER_GC_START,	/* arg1 == vid */
	EXCHANGE_SUBHEADER_GC_ITEM_ADD,	/* arg1 == vnum arg2 == pos arg3 == count */
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,
	EXCHANGE_SUBHEADER_GC_ELK_ADD,	/* arg1 == gold */
	EXCHANGE_SUBHEADER_GC_ACCEPT,	/* arg1 == accept */
	EXCHANGE_SUBHEADER_GC_END,		/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_ALREADY,	/* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_LESS_ELK,	/* arg1 == not used */
};

typedef struct packet_position
{
	uint8_t header;
	uint32_t vid;
	uint8_t position;
} TPacketGCPosition;

typedef struct packet_ping
{
	uint8_t header;
} TPacketGCPing;

typedef struct packet_pong
{
	uint8_t bHeader;
} TPacketCGPong;

typedef struct packet_script
{
	uint8_t header;
	uint16_t size;
	uint8_t skin;
	uint16_t src_size;
} TPacketGCScript;

#ifdef ENABLE_TARGET_AFFECT_SHOWER
enum ETargetAffects
{
	TARGET_AFFECT_POISON,
	TARGET_AFFECT_FIRE,
	TARGET_AFFECT_STUN,
	TARGET_AFFECT_SLOW,
# if defined(ENABLE_PROTO_RENEWAL_CUSTOM) && defined(ENABLE_WOLFMAN_CHARACTER)
	TARGET_AFFECT_BLEEDING,
# endif
	TARGET_AFFECT_MAX_NUM,
};

struct TargetAffectInfo
{
	uint32_t dwAffectNum;
	long lDuration;
};
#endif

typedef struct packet_target
{
	uint8_t header;
	uint32_t dwVID;
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
	int icurHP;
	int iMaxHP;
#else
	uint8_t bHPPercent;
#endif
#ifdef ENABLE_TARGET_AFFECT_SHOWER
	TargetAffectInfo affects[TARGET_AFFECT_MAX_NUM];
#endif
#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT_ADD)
	char cElementalFlags[6]; // CPythonNonPlayer::MOB_ELEMENTAL_MAX_NUM
#endif
} TPacketGCTarget;

#ifdef ENABLE_SEND_TARGET_INFO
typedef struct packet_target_info
{
	uint8_t header;
	uint32_t dwVID;
	uint32_t race;
	uint32_t dwVnum;
	uint8_t count;
} TPacketGCTargetInfo;

typedef struct packet_target_info_load
{
	uint8_t header;
	uint32_t dwVID;
} TPacketCGTargetInfoLoad;
#endif

typedef struct packet_damage_info
{
	uint8_t header;
	uint32_t dwVID;
	uint8_t flag;
	int damage;
} TPacketGCDamageInfo;

typedef struct packet_move
{
	uint8_t bHeader;
	uint8_t bFunc;
	uint8_t bArg;
	uint8_t bRot;
	uint32_t dwVID;
	LONG lX;
	LONG lY;
	uint32_t dwTime;
	uint32_t dwDuration;
} TPacketGCMove;

enum
{
	QUEST_SEND_IS_BEGIN = 1 << 0,
	QUEST_SEND_TITLE = 1 << 1,
	QUEST_SEND_CLOCK_NAME = 1 << 2,
	QUEST_SEND_CLOCK_VALUE = 1 << 3,
	QUEST_SEND_COUNTER_NAME = 1 << 4,
	QUEST_SEND_COUNTER_VALUE = 1 << 5,
	QUEST_SEND_ICON_FILE = 1 << 6,
};

typedef struct packet_quest_info
{
	uint8_t header;
	uint16_t size;
	uint16_t index;
	uint8_t flag;
#ifdef ENABLE_QUEST_RENEWAL
	uint16_t c_index;
#endif
} TPacketGCQuestInfo;

typedef struct packet_quest_confirm
{
	uint8_t header;
	char msg[64 + 1];
	long timeout;
	uint32_t requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_sync_position
{
	uint8_t header;
	uint16_t wSize;
} TPacketGCSyncPosition;

typedef struct packetd_sync_position_element
{
	uint32_t dwVID;
	long lX;
	long lY;
} TPacketGCSyncPositionElement;

typedef struct packet_ownership
{
	uint8_t bHeader;
	uint32_t dwOwnerVID;
	uint32_t dwVictimVID;
} TPacketGCOwnership;

#define SKILL_MAX_NUM 255

typedef struct SPlayerSkill
{
	uint8_t bMasterType;
	uint8_t bLevel;
	time_t tNextRead;
} TPlayerSkill;

typedef struct packet_skill_level
{
	uint8_t bHeader;
	TPlayerSkill skills[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

// fly
typedef struct packet_fly
{
	uint8_t bHeader;
	uint8_t bType;
	uint32_t dwStartVID;
	uint32_t dwEndVID;
} TPacketGCCreateFly;

enum EPVPModes
{
	PVP_MODE_NONE,
	PVP_MODE_AGREE,
	PVP_MODE_FIGHT,
	PVP_MODE_REVENGE
};

typedef struct packet_duel_start
{
	uint8_t header;
	uint16_t wSize;
} TPacketGCDuelStart;

typedef struct packet_pvp
{
	uint8_t header;
	uint32_t dwVIDSrc;
	uint32_t dwVIDDst;
	uint8_t bMode;
} TPacketGCPVP;

typedef struct packet_warp
{
	uint8_t bHeader;
	LONG lX;
	LONG lY;
	LONG lAddr;
	uint16_t wPort;
} TPacketGCWarp;

typedef struct packet_party_invite
{
	uint8_t header;
	uint32_t leader_pid;
} TPacketGCPartyInvite;

typedef struct packet_party_add
{
	uint8_t header;
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	uint32_t mapidx;
	uint8_t channel;
#endif
} TPacketGCPartyAdd;

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
struct SPartyPosition
{
	uint32_t dwPID;
	long lX;
	long lY;
	float fRot;
};

typedef struct SPacketGCPartyPosition
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketGCPartyPosition;
#endif

typedef struct packet_party_update
{
	uint8_t header;
	uint32_t pid;
	uint8_t state;
	uint8_t percent_hp;
	int16_t affects[PARTY_AFFECT_SLOT_MAX_NUM];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	uint8_t header;
	uint32_t pid;
} TPacketGCPartyRemove;

using TPacketGCSafeboxCheckout = TPacketCGSafeboxCheckout;
using TPacketGCSafeboxCheckin = TPacketCGSafeboxCheckin;

typedef struct packet_safebox_wrong_password
{
	uint8_t bHeader;
} TPacketGCSafeboxWrongPassword;

typedef struct packet_safebox_size
{
	uint8_t bHeader;
	uint8_t bSize;
} TPacketGCSafeboxSize;

#ifdef ENABLE_SAFEBOX_MONEY
typedef struct packet_safebox_money_change
{
	uint8_t bHeader;
	uint32_t dwMoney;
} TPacketGCSafeboxMoneyChange;
#endif

typedef struct command_empire
{
	uint8_t bHeader;
	uint8_t bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
	uint8_t bHeader;
	uint8_t bEmpire;
} TPacketGCEmpire;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
	FISHING_SUBHEADER_GC_FISH
};

typedef struct packet_fishing
{
	uint8_t header;
	uint8_t subheader;
	uint32_t info;
	uint8_t dir;
} TPacketGCFishing;

typedef struct paryt_parameter
{
	uint8_t bHeader;
	uint8_t bDistributeMode;
} TPacketGCPartyParameter;

// Guild

enum
{
	GUILD_SUBHEADER_GC_LOGIN,
	GUILD_SUBHEADER_GC_LOGOUT,
	GUILD_SUBHEADER_GC_LIST,
	GUILD_SUBHEADER_GC_GRADE,
	GUILD_SUBHEADER_GC_ADD,
	GUILD_SUBHEADER_GC_REMOVE,
	GUILD_SUBHEADER_GC_GRADE_NAME,
	GUILD_SUBHEADER_GC_GRADE_AUTH,
	GUILD_SUBHEADER_GC_INFO,
	GUILD_SUBHEADER_GC_COMMENTS,
	GUILD_SUBHEADER_GC_CHANGE_EXP,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_GC_SKILL_INFO,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_GC_GUILD_INVITE,
	GUILD_SUBHEADER_GC_WAR,
	GUILD_SUBHEADER_GC_GUILD_NAME,
	GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
	GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
	GUILD_SUBHEADER_GC_WAR_POINT,
	GUILD_SUBHEADER_GC_MONEY_CHANGE,
#ifdef ENABLE_GUILD_WAR_SCORE
	GUILD_SUBHEADER_GC_WAR_INFO,
#endif
#ifdef ENABLE_GUILD_LAND_INFO
	GUILD_SUBHEADER_GC_LAND_INFO,
#endif
#ifdef ENABLE_GUILDBANK_LOG
	GUILD_SUBHEADER_GC_BANK_INFO,
	GUILD_SUBHEADER_GC_BANK_DELETE,
#endif
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	GUILD_SUBHEADER_GC_DONATE,
#endif
};

typedef struct packet_guild
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
} TPacketGCGuild;

enum
{
	GUILD_AUTH_ADD_MEMBER = (1 << 0),
	GUILD_AUTH_REMOVE_MEMBER = (1 << 1),
	GUILD_AUTH_NOTICE = (1 << 2),
	GUILD_AUTH_SKILL = (1 << 3),
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	GUILD_AUTH_WAR = (1 << 4),
	GUILD_AUTH_BANK = (1 << 5),
#endif
};

typedef struct packet_guild_sub_grade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN + 1]; // 8+1 길드장, 길드원 등의 이름
	uint8_t auth_flag;
} TPacketGCGuildSubGrade;

typedef struct packet_guild_sub_member
{
	uint32_t pid;
	uint8_t byGrade;
	uint8_t byIsGeneral;
	uint8_t byJob;
	uint8_t byLevel;
	uint32_t dwOffer;
	uint8_t byNameFlag;
	// if NameFlag is TRUE, name is sent from server.
	// char szName[CHARACTER_ME_MAX_LEN+1];
} TPacketGCGuildSubMember;

typedef struct packet_guild_sub_info
{
	uint16_t member_count;
	uint16_t max_member_count;
	uint32_t guild_id;
	uint32_t master_pid;
	uint32_t exp;
	uint8_t level;
	char name[GUILD_NAME_MAX_LEN + 1];
	uint32_t gold;
	uint8_t hasLand;
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	char goldCheckout[CHARACTER_NAME_MAX_LEN + 1];
	char itemCheckout[CHARACTER_NAME_MAX_LEN + 1];
#endif
} TPacketGCGuildInfo;

enum EGuildWarState
{
	GUILD_WAR_NONE,
	GUILD_WAR_SEND_DECLARE,
	GUILD_WAR_REFUSE,
	GUILD_WAR_RECV_DECLARE,
	GUILD_WAR_WAIT_START,
	GUILD_WAR_CANCEL,
	GUILD_WAR_ON_WAR,
	GUILD_WAR_END,

	GUILD_WAR_DURATION = 2 * 60 * 60,
};

typedef struct packet_guild_war
{
	uint32_t dwGuildSelf;
	uint32_t dwGuildOpp;
	uint8_t bType;
	uint8_t bWarState;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t bRound;
	uint8_t bPoints;
	uint32_t dwTime;
#endif
} TPacketGCGuildWar;

typedef struct SPacketGuildWarPoint
{
	uint32_t dwGainGuildID;
	uint32_t dwOpponentGuildID;
	long lPoint;
#ifdef ENABLE_GUILD_RANKING
	long lWinPoint;
#endif
} TPacketGuildWarPoint;

// SubHeader - Dungeon
enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1
};

typedef struct packet_dungeon
{
	uint8_t bHeader;
	uint16_t size;
	uint8_t subheader;
} TPacketGCDungeon;

// Private Shop
typedef struct SPacketGCShopSign
{
	uint8_t bHeader;
	uint32_t dwVID;
	char szSign[SHOP_SIGN_MAX_LEN + 1];
#ifdef ENABLE_MYSHOP_DECO
	uint8_t bType;
#endif
} TPacketGCShopSign;

typedef struct SPacketGCTime
{
	uint8_t bHeader;
	time_t time;
} TPacketGCTime;

enum
{
	WALKMODE_RUN,
	WALKMODE_WALK
};

typedef struct SPacketGCWalkMode
{
	uint8_t header;
	uint32_t vid;
	uint8_t mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
	uint8_t header;
	uint8_t skill_group;
} TPacketGCChangeSkillGroup;

struct TMaterial
{
	uint32_t vnum;
	uint32_t count;
};

typedef struct SRefineTable
{
	uint32_t src_vnum;
	uint32_t result_vnum;
	uint8_t material_count;
	int cost;
	int prob;
	TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGCRefineInformation
{
	uint8_t header;
	uint8_t type;
	uint16_t pos; //@fixme425
	TRefineTable refine_table;
} TPacketGCRefineInformation;

enum SPECIAL_EFFECT
{
	SE_NONE,
	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
	SE_LEVELUP_ON_14_FOR_GERMANY,
	SE_LEVELUP_UNDER_15_FOR_GERMANY,
	SE_PERCENT_DAMAGE1,
	SE_PERCENT_DAMAGE2,
	SE_PERCENT_DAMAGE3,
	SE_AUTO_HPUP,
	SE_AUTO_SPUP,
	SE_EQUIP_RAMADAN_RING,
	SE_EQUIP_HALLOWEEN_CANDY,
	SE_EQUIP_HAPPINESS_RING,
	SE_EQUIP_LOVE_PENDANT,
	SE_CAPE_OF_COURAGE_EFFECT, //@fixme421
	SE_EQUIP_BOOTS,
	SE_EQUIP_HEROMEDAL,
	SE_EQUIP_CHOCOLATE_AMULET,
	SE_EQUIP_EMOTION_MASK,
	SE_EQUIP_WILLPOWER_RING,
	SE_EQUIP_DEADLYPOWER_RING,
	SE_EQUIP_EASTER_CANDY_EQIP,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	SE_ACCE_SUCESS_ABSORB,
	SE_ACCE_EQUIP,
	SE_ACCE_BACK,
#endif
#ifdef ENABLE_DAWNMIST_DUNGEON
	SE_HEAL,
#endif
#ifdef ENABLE_ANTI_EXP_RING
	SE_AUTO_EXP,
#endif
#ifdef ENABLE_12ZI
	SE_SKILL_DAMAGE_ZONE,
	SE_SKILL_SAFE_ZONE,

	SE_METEOR,
	SE_BEAD_RAIN,
	SE_FALL_ROCK,
	SE_ARROW_RAIN,
	SE_HORSE_DROP,
	SE_EGG_DROP,
	SE_DEAPO_BOOM,

	SE_SKILL_DAMAGE_ZONE_BIG,
	SE_SKILL_DAMAGE_ZONE_MIDDLE,
	SE_SKILL_DAMAGE_ZONE_SMALL,

	SE_SKILL_SAFE_ZONE_BIG,
	SE_SKILL_SAFE_ZONE_MIDDLE,
	SE_SKILL_SAFE_ZONE_SMALL,
#endif
#ifdef ENABLE_BATTLE_FIELD
	SE_EFFECT_BATTLE_POTION,
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	SE_DRAGONLAIR_STONE_UNBEATABLE_1,
	SE_DRAGONLAIR_STONE_UNBEATABLE_2,
	SE_DRAGONLAIR_STONE_UNBEATABLE_3,
#endif
#ifdef ENABLE_FLOWER_EVENT
	SE_FLOWER_EVENT,
#endif
#ifdef ENABLE_GEM_SYSTEM
	SE_GEM_PENDANT,
#endif
#ifdef ENABLE_QUEEN_NETHIS
	SE_EFFECT_SNAKE_REGEN,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	SE_EFFECT_BP_NORMAL_MISSION_COMPLETED,
	SE_EFFECT_BP_PREMIUM_MISSION_COMPLETED,
	SE_EFFECT_BP_EVENT_MISSION_COMPLETED,
	SE_EFFECT_BP_NORMAL_BATTLEPASS_COMPLETED,
	SE_EFFECT_BP_PREMIUM_BATTLEPASS_COMPLETED,
	SE_EFFECT_BP_EVENT_BATTLEPASS_COMPLETED,
#endif
};

typedef struct SPacketGCSpecialEffect
{
	uint8_t header;
	uint8_t type;
	uint32_t vid;
} TPacketGCSpecialEffect;

#ifdef ENABLE_12ZI
typedef struct SPacketGCSpecialZodiacEffect
{
	uint8_t header;
	uint8_t type;
	uint8_t type2;
	uint32_t vid;
	long x;
	long y;
} TPacketGCSpecialZodiacEffect;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct SPacketGCNPCPosition
{
	uint8_t header;
	uint16_t size;
	uint16_t count;
} TPacketGCNPCPosition, TPacketGCShopPosition;

typedef struct TNPCPosition
{
	uint8_t bType;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t dwMobVnum;
	long x;
	long y;
} TShopPosition;
#else
struct TNPCPosition
{
	uint8_t bType;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	long x;
	long y;
};

typedef struct SPacketGCNPCPosition
{
	uint8_t header;
	uint16_t size;
	uint16_t count;
} TPacketGCNPCPosition;
#endif

typedef struct SPacketGCChangeName
{
	uint8_t header;
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCChangeName;

enum EBlockAction
{
	BLOCK_EXCHANGE = (1 << 0),
	BLOCK_PARTY_INVITE = (1 << 1),
	BLOCK_GUILD_INVITE = (1 << 2),
	BLOCK_WHISPER = (1 << 3),
	BLOCK_MESSENGER_INVITE = (1 << 4),
	BLOCK_PARTY_REQUEST = (1 << 5),
};

typedef struct packet_auth_success
{
	uint8_t bHeader;
	uint32_t dwLoginKey;
	uint8_t bResult;
} TPacketGCAuthSuccess;

typedef struct packet_channel
{
	uint8_t header;
	uint8_t channel;
} TPacketGCChannel;

typedef struct SEquipmentItemSet
{
	uint32_t vnum;
	uint8_t count;
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TEquipmentItemSet;

typedef struct pakcet_view_equip
{
	uint8_t header;
	uint32_t dwVID;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketGCViewEquip;

typedef struct
{
	uint32_t dwID;
	long x, y;
	long width, height;
	uint32_t dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list
{
	uint8_t header;
	uint16_t size;
} TPacketGCLandList;

enum
{
	CREATE_TARGET_TYPE_NONE,
	CREATE_TARGET_TYPE_LOCATION,
	CREATE_TARGET_TYPE_CHARACTER
};

typedef struct
{
	uint8_t bHeader;
	long lID;
	char szTargetName[32 + 1];
	uint32_t dwVID;
	uint8_t byType;
} TPacketGCTargetCreate;

typedef struct
{
	uint8_t bHeader;
	long lID;
	long lX, lY;
} TPacketGCTargetUpdate;

typedef struct
{
	uint8_t bHeader;
	long lID;
} TPacketGCTargetDelete;

typedef struct
{
	uint32_t dwType;
	uint16_t wPointIdxApplyOn; //@fixme436
	long lApplyValue;
	uint32_t dwFlag;
	long lDuration;
	long lSPCost;
} TPacketAffectElement;

typedef struct
{
	uint8_t bHeader;
	TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct
{
	uint8_t bHeader;
	uint32_t dwType;
	uint16_t wApplyOn; //@fixme436
} TPacketGCAffectRemove;

typedef struct packet_mall_open
{
	uint8_t bHeader;
	uint8_t bSize;
} TPacketGCMallOpen;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
typedef struct packet_guildstorage_open
{
	uint8_t bHeader;
	uint8_t bSize;
} TPacketGCGuildstorageOpen;
#endif

typedef struct packet_lover_info
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t byLovePoint;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	uint8_t bHeader;
	uint8_t byLovePoint;
} TPacketGCLovePointUpdate;

typedef struct packet_dig_motion
{
	uint8_t header;
	uint32_t vid;
	uint32_t target_vid;
	uint8_t count;
} TPacketGCDigMotion;

typedef struct SPacketGCPanamaPack
{
	uint8_t bHeader;
	char szPackName[256];
	uint8_t abIV[32];
} TPacketGCPanamaPack;

typedef struct SPacketGCHybridCryptKeys
{
private:
	SPacketGCHybridCryptKeys() = default;

public:
	SPacketGCHybridCryptKeys(int iStreamSize) : iKeyStreamLen(iStreamSize) { m_pStream.resize(iStreamSize); }
	~SPacketGCHybridCryptKeys() = default;
	CLASS_DELETE_COPYMOVE(SPacketGCHybridCryptKeys);

	static int GetFixedHeaderSize() { return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(int); }

	uint8_t bHeader{};
	uint16_t wDynamicPacketSize{};
	int iKeyStreamLen{};
	std::vector<uint8_t> m_pStream;

} TPacketGCHybridCryptKeys;

typedef struct SPacketGCHybridSDB
{
private:
	SPacketGCHybridSDB() = default;

public:
	SPacketGCHybridSDB(int iStreamSize) : iSDBStreamLen(iStreamSize) { m_pStream.resize(iStreamSize); }
	~SPacketGCHybridSDB() = default;
	CLASS_DELETE_COPYMOVE(SPacketGCHybridSDB);

	static int GetFixedHeaderSize() { return sizeof(uint8_t) + sizeof(uint16_t) + sizeof(int); }

	uint8_t bHeader{};
	uint16_t wDynamicPacketSize{};
	int iSDBStreamLen{};
	std::vector<uint8_t> m_pStream;

} TPacketGCHybridSDB;

typedef struct packet_state
{
	uint8_t bHeader;
	uint8_t bFunc;
	uint8_t bArg;
	uint8_t bRot;
	uint32_t dwVID;
	uint32_t dwTime;
	TPixelPosition kPPos;
} TPacketCCState;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
struct TPacketKeyAgreement
{
	static const int MAX_DATA_LEN = 256;
	uint8_t bHeader;
	uint16_t wAgreedLength;
	uint16_t wDataLength;
	uint8_t data[MAX_DATA_LEN];
};

struct TPacketKeyAgreementCompleted
{
	uint8_t bHeader;
	uint8_t data[3]; // dummy (not used)
};
#endif // _IMPROVED_PACKET_ENCRYPTION_

typedef struct SPacketGCSpecificEffect
{
	uint8_t header;
	uint32_t vid;
	char effect_file[128];
} TPacketGCSpecificEffect;

enum EDragonSoulRefineWindowRefineType
{
	DragonSoulRefineWindow_UPGRADE,
	DragonSoulRefineWindow_IMPROVEMENT,
	DragonSoulRefineWindow_REFINE
};

enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_UPGRADE,
	DS_SUB_HEADER_DO_IMPROVEMENT,
	DS_SUB_HEADER_DO_REFINE,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
#ifdef ENABLE_DS_CHANGE_ATTR
	DS_SUB_HEADER_OPEN_CHANGE_ATTR,
	DS_SUB_HEADER_DO_CHANGE_ATTR,
#endif
};

typedef struct SPacketCGDragonSoulRefine
{
	SPacketCGDragonSoulRefine() = default;
	uint8_t header{ HEADER_CG_DRAGON_SOUL_REFINE };
	uint8_t bSubType{};
	TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() = default;
	uint8_t header{ HEADER_GC_DRAGON_SOUL_REFINE };
	uint8_t bSubType{};
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SChannelStatus
{
	int16_t nPort;
	uint8_t bStatus;
} TChannelStatus;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
typedef struct SPacketCGLanguageChange
{
	uint8_t		bHeader;
	uint8_t		bLanguage;
} TPacketCGLanguageChange;

typedef struct SPacketCGTargetLanguageRequest
{
	uint8_t		bHeader;
	char		szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGTargetLanguageRequest;

typedef struct SPacketGCTargetLanguageResult
{
	uint8_t		bHeader;
	char		szName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t		bLanguage;
} TPacketGCTargetLanguageResult;
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
typedef struct SPacketCGSendExtendInvenRequest
{
	uint8_t bHeader;
	uint8_t bStepIndex;
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bWindow;
	bool bSpecialState;
#endif
} TPacketCGSendExtendInvenRequest;

typedef struct SPacketCGSendExtendInvenUpgrade
{
	uint8_t bHeader;
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bWindow;
	bool bSpecialState;
#endif
} TPacketCGSendExtendInvenUpgrade;

typedef struct SPacketGCExtendInvenInfo
{
	uint8_t bHeader;
	uint8_t bExtendStage;
	uint8_t bExtendMax;
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bExtendSpecialStage[3];
	uint16_t bExtendSpecialMax[3];
#endif
} TPacketGCExtendInvenInfo;

typedef struct SPacketGCExtendInvenResult
{
	uint8_t bHeader;

	uint32_t dwVnum;
	uint8_t bMsgResult;
	uint8_t bEnoughCount;
} TPacketGCExtendInvenResult;
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
typedef struct SPacketGCSeal
{
	uint8_t header;
	TItemPos pos;
	uint8_t action;
	long nSealDate;
} TPacketGCSeal;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
// Client to Game
typedef struct SPacketCGSendAcceClose
{
	uint8_t bHeader;
} TPacketCGSendAcceClose;

typedef struct SPacketCGSendAcceRefine
{
	uint8_t bHeader;
	uint8_t bAcceWindow;
	uint8_t bSlotAcce;
	uint8_t bSlotMaterial;
} TPacketCGSendAcceRefine;

// Game to Client
typedef struct SPacketGCSendAcceRefine
{
	uint8_t bHeader;
	uint8_t bAcceWindow;
	bool bResult;
} TPacketGCSendAcceRefine;
#endif


#ifdef ENABLE_SWITCHBOT
enum ECGSwitchbotSubheader
{
	SUBHEADER_CG_SWITCHBOT_START,
	SUBHEADER_CG_SWITCHBOT_STOP,
};

struct TPacketCGSwitchbot
{
	uint8_t header;
	int size;
	uint8_t subheader;
	uint8_t slot;
};

enum EGCSwitchbotSubheader
{
	SUBHEADER_GC_SWITCHBOT_UPDATE,
	SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM,
	SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION,
};

struct TPacketGCSwitchbot
{
	uint8_t header;
	int size;
	uint8_t subheader;
	uint8_t slot;
};

struct TSwitchbotUpdateItem
{
	uint8_t slot;
	uint8_t vnum;
	uint8_t count;
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
# ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
# endif
};
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
typedef struct packet_skill_color
{
	uint8_t bheader;
	uint8_t skill;
	uint32_t col1;
	uint32_t col2;
	uint32_t col3;
	uint32_t col4;
	uint32_t col5;
} TPacketCGSkillColor;
#endif

#ifdef ENABLE_BATTLE_FIELD
typedef struct SBattleRankingMember
{
	uint8_t bPosition;
	uint8_t bType;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t bEmpire;
	long lScore;
} TBattleRankingMember;

typedef struct SPacketGCBattleInfo
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketGCBattleInfo;
#endif

#ifdef ENABLE_ATTR_6TH_7TH
enum
{
	ATTR67_SUB_HEADER_CLOSE = 0,
	ATTR67_SUB_HEADER_OPEN = 1,
	ATTR67_SUB_HEADER_SKILLBOOK_COMB = 2,

	ATTR67_SUB_HEADER_GET_FRAGMENT,
	ATTR67_SUB_HEADER_ADD,

	ATTR67_SUB_HEADER_FRAGMENT_RECEIVE,
};

typedef struct SPacketAttr67Send
{
	uint8_t header;
	uint8_t subheader;
	uint8_t bCell[c_SkillBook_Comb_Slot_Max];
	uint8_t bRegistSlot;
	uint8_t bFragmentCount;
	uint16_t wCellAdditive;
	uint8_t bCountAdditive;
} TPacketCGAttr67Send;

typedef struct SPacketAttr67Receive
{
	SPacketAttr67Receive() : header(HEADER_GC_ATTR_6TH_7TH)
	{}

	uint8_t header;
	uint8_t subheader;
	uint32_t fragmentVnum;
} TPacketGCAttr67Receive;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct myshop_remove
{
	uint8_t header;
	int slot;
	TItemPos target;
} TPacketMyShopRemoveItem;

typedef struct myshop_add
{
	uint8_t header;
	TItemPos from;
	int targetPos;
	uint32_t price;
# ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
# endif
} TPacketMyShopAddItem;

typedef struct shopStashSync
{
	uint8_t bHeader;
	uint32_t value;
# ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
# endif
} TPacketGCShopStashSync;

typedef struct shopOfflineTimeSync
{
	uint8_t bHeader;
	uint32_t value;
} TPacketGCShopOffTimeSync;

typedef struct shopSyncPos
{
	uint8_t bHeader;
	int channel;
	int xGlobal;
	int yGlobal;
} TPacketGCShopSyncPos;

typedef struct shopRename
{
	uint8_t bHeader;
	char sign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopRename;

typedef struct shopWithdraw
{
	uint8_t bHeader;
	uint32_t amount;
# ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t chequeAmount;
# endif
} TPacketCGShopWithdraw;

#ifdef ENABLE_MYSHOP_DECO
typedef struct SPacketGCMyPrivShopOpen
{
	uint8_t bHeader;
	bool bCashItem;
	uint8_t bTabCount;
} TPacketGCMyPrivShopOpen;

typedef struct SPacketCGMyShopDecoState
{
	uint8_t header;
	uint8_t bState;
} TPacketCGMyShopDecoState;

typedef struct SPacketCGMyShopDecoAdd
{
	uint8_t header;
	uint8_t bType;
	uint32_t dwPolyVnum;
} TPacketCGMyShopDecoAdd;
#endif
#endif

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
enum EWonExchangeCGSubHeader
{
	WON_EXCHANGE_CG_SUBHEADER_SELL,
	WON_EXCHANGE_CG_SUBHEADER_BUY
};

typedef struct SPacketCGWonExchange
{
	SPacketCGWonExchange() :
		bHeader(HEADER_CG_WON_EXCHANGE)
	{}
	uint8_t bHeader;
	uint8_t bSubHeader;
	uint16_t wValue;
} TPacketCGWonExchange;
#endif

#ifdef ENABLE_REFINE_ELEMENT
enum
{
	ELEMENT_SPELL_SUB_HEADER_OPEN,
	ELEMENT_SPELL_SUB_HEADER_OPEN_CHANGE,

	ELEMENTS_SPELL_SUB_HEADER_CLOSE,
	ELEMENTS_SPELL_SUB_HEADER_ADD_BONUS,
	ELEMENTS_SPELL_SUB_HEADER_CHANGE_BONUS,
};

typedef struct SPacketCGElementsSpell
{
	uint8_t header;
	uint8_t subheader;
	int pos;
	uint8_t type_select;
} TPacketCGElementsSpell;

typedef struct SPacketGCElementsSpell
{
	uint8_t header;
	uint8_t subheader;
	int pos;
	bool func;
	int cost;
	uint8_t grade_add;
} TPacketGCElementsSpell;
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
typedef struct command_cgroulette
{
	uint8_t header;
	uint8_t option;
} TPacketCGSoulRoulette;

typedef struct command_gcroulette
{
	uint8_t header;
	uint8_t option;
	int yang;
	int soul;
	struct {
		uint32_t vnum;
		uint8_t count;
	} ItemInfo[ROULETTE_ITEM_MAX];
} TPacketGCSoulRoulette;
#endif

#ifdef NEW_SELECT_CHARACTER
typedef struct SPacketCharacterInformation
{
	uint8_t bHeader;
	TSimplePlayerInformation players[PLAYER_PER_ACCOUNT];
} TPacketCharacterInformation;
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
typedef struct SPacketCGSendCostumeRefine
{
	uint8_t bHeader;
	uint8_t bSlotMedium;
	uint8_t bSlotBase;
	uint8_t bSlotMaterial;
} TPacketCGSendCostumeRefine;
#endif

#ifdef ENABLE_AURA_SYSTEM
enum EPacketGCAuraSubHeader
{
	AURA_SUBHEADER_GC_OPEN,
	AURA_SUBHEADER_GC_CLOSE,
	AURA_SUBHEADER_GC_SET_ITEM,
	AURA_SUBHEADER_GC_CLEAR_SLOT,
	AURA_SUBHEADER_GC_CLEAR_ALL,
	AURA_SUBHEADER_GC_CLEAR_RIGHT,
	AURA_SUBHEADER_GC_REFINE_INFO,
};

typedef struct SSubPacketGCAuraOpenClose
{
	uint8_t bAuraWindowType;
} TSubPacketGCAuraOpenClose;

typedef struct SSubPacketGCAuraSetItem
{
	TItemPos Cell;
	TItemPos AuraCell;
	TItemData pItem;
} TSubPacketGCAuraSetItem;

typedef struct SSubPacketGCAuraClearSlot
{
	uint8_t bAuraSlotPos;
} TSubPacketGCAuraClearSlot;

typedef struct SSubPacketGCAuraRefineInfo
{
	uint8_t bAuraRefineInfoType;
	uint8_t bAuraRefineInfoLevel;
	uint8_t bAuraRefineInfoExpPercent;
} TSubPacketGCAuraRefineInfo;

enum EPacketCGAuraSubHeader
{
	AURA_SUBHEADER_CG_REFINE_CHECKIN,
	AURA_SUBHEADER_CG_REFINE_CHECKOUT,
	AURA_SUBHEADER_CG_REFINE_ACCEPT,
	AURA_SUBHEADER_CG_REFINE_CANCEL,
};

typedef struct SSubPacketCGAuraRefineCheckIn
{
	TItemPos ItemCell;
	TItemPos AuraCell;
	uint8_t byAuraRefineWindowType;
} TSubPacketCGAuraRefineCheckIn;

typedef struct SSubPacketCGAuraRefineCheckOut
{
	TItemPos AuraCell;
	uint8_t byAuraRefineWindowType;
} TSubPacketCGAuraRefineCheckOut;

typedef struct SSubPacketCGAuraRefineAccept
{
	uint8_t byAuraRefineWindowType;
} TSubPacketCGAuraRefineAccept;

typedef struct SPacketGCAura
{
	SPacketGCAura() : bHeader(HEADER_GC_AURA) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCAura;

typedef struct SPacketCGAura
{
	SPacketCGAura() : bHeader(HEADER_CG_AURA) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketCGAura;
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
struct TSearchItemData
{
	uint32_t vid;
	char char_name[CHARACTER_NAME_MAX_LEN + 1];
	char item_name[24 + 1]; // ITEM_NAME_MAX_LEN
	TItemPos Cell;
	TItemData pItem;
	uint32_t price;
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
};

typedef struct SPrivateShopSearchItem
{
	uint32_t real_position;
	uint32_t vid;
	char char_name[CHARACTER_NAME_MAX_LEN + 1];
	int price;
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
	TItemPos Cell;
	TItemData Item;
} TPrivateShopSearchItem;

typedef struct SPacketGCPrivateShopSearchItemInfo
{
	uint8_t bHeader;
	uint32_t wSize;
} TPacketGCPrivateShopSearchItemInfo;

typedef struct command_private_shop_search
{
	uint8_t header;
	uint8_t bRace;
	uint8_t bMaskType;
	uint8_t bMaskSubType;
	uint8_t bMinRefine;
	uint8_t bMaxRefine;
	uint8_t bMinLevel;
	uint8_t bMaxLevel;
	int iMinGold;
	int iMaxGold;
	char itemName[CItemData::ITEM_NAME_MAX_LEN + 1];
# ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t dwMinCheque;
	uint32_t dwMaxCheque;
# endif
} TPacketCGPrivateShopSearch;

typedef struct command_private_shop_search_open
{
	uint8_t header;
	uint8_t subheader;
} TPacketGCPrivateShopSearchOpen;

typedef struct command_private_shop_search_buy
{
	uint8_t header;
	uint32_t shopVid;
	uint8_t shopItemPos;
} TPacketCGPrivateShopSearchBuy;

typedef struct command_private_shop_search_close
{
	uint8_t header;
} TPacketCGPrivateShopSearchClose;
#endif

#ifdef ENABLE_HWID_BAN
typedef struct SPacketCGHwidBan
{
	uint8_t header;
	int mode;
	char szPlayer[24 + 1];
	char szReason[512 + 1];
} TPacketCGHwidBan;
#endif

#ifdef ENABLE_CHANGED_ATTR
typedef struct packet_gc_item_select_attr
{
	uint8_t bHeader;
	TItemPos pItemPos;
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TPacketGCItemSelectAttr;

typedef struct packet_cg_item_select_attr
{
	uint8_t bHeader;
	bool bNew;
	TItemPos pItemPos;
} TPacketCGItemSelectAttr;
#endif

#ifdef ENABLE_CUBE_RENEWAL
enum EPacketCGCubeRenewalSubHeader
{
	CUBE_RENEWAL_MAKE,
	CUBE_RENEWAL_CLOSE_STATE,
};

/*Client -> Game*/
typedef struct SPacketCGCubeRenewal
{
	SPacketCGCubeRenewal() : bHeader(HEADER_CG_CUBE_RENEWAL) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketCGCubeRenewal;

typedef struct SSubPacketCGCubeRenwalMake
{
	int vnum;
	int multiplier;
	int indexImprove;
	int itemReq[5];
} TSubPacketCGCubeRenwalMake;

typedef struct SSubPacketCGCubeRenwalClose
{
	int close_state;
} TSubPacketCGCubeRenwalClose;


/*Game -> Client*/
enum EPacketGCCubeRenewalSubHeader
{
	CUBE_RENEWAL_OPEN,
	CUBE_RENEWAL_CLOSE,
};

typedef struct SPacketGCCubeRenewal
{
	SPacketGCCubeRenewal() : bHeader(HEADER_GC_CUBE_RENEWAL) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCCubeRenewal;

typedef struct SSubPacketGCCubeRenewalOpenClose
{
	uint32_t npc_vnum;
	bool open_state;
} TSubPacketGCCubeRenewalOpenClose;

typedef struct dates_cube_renewal
{
	uint32_t npc_vnum;
	uint32_t index;

	uint32_t vnum_reward;
	int count_reward;
	bool item_reward_stackable;

	uint32_t vnum_material_1;
	int count_material_1;
	uint32_t vnum_material_2;
	int count_material_2;
	uint32_t vnum_material_3;
	int count_material_3;
	uint32_t vnum_material_4;
	int count_material_4;
	uint32_t vnum_material_5;
	int count_material_5;

	int gold;
	int percent;

#ifdef ENABLE_GEM_SYSTEM
	int gem;
#endif
	uint32_t allowCopy;
#ifdef ENABLE_SET_ITEM
	uint8_t set_value;
	uint32_t not_remove;
#endif

	char category[100];
}TInfoDateCubeRenewal;
#endif

#ifdef ENABLE_TICKET_SYSTEM
/*Client -> Server*/
enum EPacketCGTicketSystemSubHeader
{
	TICKET_SUBHEADER_CG_OPEN,
	TICKET_SUBHEADER_CG_CREATE,
	TICKET_SUBHEADER_CG_REPLY,
	TICKET_SUBHEADER_CG_ADMIN,
	TICKET_SUBHEADER_CG_ADMIN_PAGE
};

typedef struct SPacketCGTicketSystem
{
	SPacketCGTicketSystem() : bHeader(HEADER_CG_TICKET_SYSTEM) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketCGTicketSystem;

typedef struct SSubPacketCGTicketOpen
{
	int action;
	int mode;
	char ticked_id[10 + 1];
} TSubPacketCGTicketOpen;

typedef struct SSubPacketCGTicketCreate
{
	char title[32 + 1];
	char content[512 + 1];
	int priority;
} TSubPacketCGTicketCreate;

typedef struct SSubPacketCGTicketReply
{
	char ticked_id[10 + 1];
	char reply[512 + 1];
} TSubPacketCGTicketReply;

typedef struct SSubPacketCGTicketAdmin
{
	int action;
	char ticked_id[10 + 1];
	char char_name[12 + 1];
	char reason[32 + 1];
} TSubPacketCGTicketAdmin;

typedef struct SSubPacketCGTicketAdminChangePage
{
	int iStartPage;
} TSubPacketCGTicketAdminChangePage;

/*Server -> Client*/
enum EPacketGCTicketSystemSubHeader
{
	TICKET_SUBHEADER_GC_LOGS,
	TICKET_SUBHEADER_GC_LOGS_REPLY
};

typedef struct SPacketGCTicketSystem
{
	SPacketGCTicketSystem() : bHeader(HEADER_GC_TICKET_SYSTEM) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCTicketSystem;

typedef struct SSubPacketTicketLogs
{
	char ticked_id[10 + 1];
	char title[32 + 1];
	char content[512 + 1];
	int priority;
	char create_date[19 + 1];
	int status;
} TSubPacketTicketLogs;

typedef struct SSubPacketTicketLogsData
{
	TSubPacketTicketLogs logs[40];
} TSubPacketTicketLogsData;

typedef struct SSubPacketTicketLogsReply
{
	char reply_from[12 + 1];
	char reply_content[512 + 1];
	char reply_date[19 + 1];
} TSubPacketTicketLogsReply;

typedef struct SSubPacketTicketLogsDataReply
{
	TSubPacketTicketLogsReply logs[40];
} TSubPacketTicketLogsDataReply;
#endif

#ifdef ENABLE_MAILBOX
typedef struct packet_mailbox_process
{
	packet_mailbox_process()
		: bArg1(0), bArg2(0) {}
	uint8_t bHeader;
	uint8_t bSubHeader;
	uint8_t bArg1;
	uint8_t bArg2;
} TPacketMailboxProcess;

typedef struct SMailBoxRespondUnreadData
{
	uint8_t bHeader;
	uint8_t bItemMessageCount;
	uint8_t bCommonMessageCount;
	bool bGMVisible;
} TMailBoxRespondUnreadData;

typedef struct packet_mailbox_process_all
{
	uint8_t Index;
} TPacketGCMailboxProcessAll;

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
	long alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
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
	short elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_SET_ITEM
	uint8_t set_value;
#endif
} TPacketGCMailBoxAddData;

typedef struct packet_mailbox_message
{
	__time32_t SendTime;
	__time32_t DeleteTime;
	char szTitle[25 + 1];
	bool bIsGMPost;
	bool bIsItemExist;
	bool bIsConfirm;
} TPacketGCMailBoxMessage;

typedef struct packet_mailbox
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketGCMailBox;

typedef struct packet_mailbox_write
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	char szTitle[25 + 1];
	char szMessage[100 + 1];
	TItemPos pos;
	int iYang;
	int iWon;
} TPacketCGMailboxWrite;

typedef struct packet_mailbox_write_confirm
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGMailboxWriteConfirm;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
enum class EGC_TRANSMUTATION_SHEADER : uint8_t
{
	ITEM_SET,
	ITEM_DEL,
	ITEM_FREE_SET,
	ITEM_FREE_DEL
};

typedef struct SPacketGCTransmutation
{
	uint8_t header;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCTransmutation;

typedef struct SSubPacketGCChangeLookItemSet
{
	uint16_t wCell;
	uint8_t bSlotType;
} TSubPacketGCChangeLookItemSet;

typedef struct SSubPacketGCChangeLookDel
{
	uint16_t wCell;
	uint8_t bSlotType;
} TSubPacketGCChangeLookDel;

enum class ECG_TRANSMUTATION_SHEADER : uint8_t
{
	ITEM_CHECK_IN,
	ITEM_CHECK_OUT,
	FREE_ITEM_CHECK_IN,
	FREE_ITEM_CHECK_OUT,
	ACCEPT,
	CANCEL
};

typedef struct SPacketCGTransmutation
{
	SPacketCGTransmutation(const uint8_t _Sub) :
		header(HEADER_CG_CHANGE_LOOK),
		subheader(_Sub),
		slot_type(255)
	{}
	uint8_t header;
	uint8_t subheader;
	uint8_t slot_type;
	TItemPos pos;
} TPacketCGTransmutation;
#endif

#ifdef ENABLE_MOVE_CHANNEL
typedef struct command_move_channel
{
	uint8_t header;
	uint8_t channel;
} TPacketCGMoveChannel;
#endif

#ifdef ENABLE_PARTY_MATCH
typedef struct SPacketCGPartyMatch
{
	uint8_t Header;
	uint8_t SubHeader;
	int index;
} TPacketCGPartyMatch;

typedef struct SPacketGCPartyMatch
{
	uint8_t Header;
	uint8_t SubHeader;
	uint8_t MSG;
	uint32_t index;
} TPacketGCPartyMatch;
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
enum ESubHeader
{
	SUBHEADER_DUNGEON_INFO_SEND,
	SUBHEADER_DUNGEON_INFO_OPEN,
};

typedef struct SPacketCGDungeonInfo
{
	uint8_t byHeader;
	uint8_t bySubHeader;
	uint8_t byIndex;
	uint8_t byRankType;
} TPacketCGDungeonInfo;

struct LevelLimit { int iLevelMin, iLevelMax, iMemberMin, iMemberMax; };
struct Item { uint32_t dwVnum; uint16_t wCount; };
struct Bonus { uint16_t byAttBonus[POINT_MAX_NUM]; uint16_t byDefBonus[POINT_MAX_NUM]; };
struct Results { uint32_t dwFinish, dwFinishTime, dwFinishDamage; };

typedef struct SPacketGCDungeonInfo
{
	SPacketGCDungeonInfo() : byHeader(HEADER_GC_DUNGEON_INFO), bySubHeader(0), byIndex(0), byType(0),
		bReset(false),
		lMapIndex(0), lEntryMapIndex(0),
		dwBossVnum(0),
		dwDuration(0), dwCooldown(0), byElement(0)
	{
		memset(&sLevelLimit, 0, sizeof(sLevelLimit));
		memset(&sRequiredItem, 0, sizeof(sRequiredItem));
		memset(&sBonus.byAttBonus, 0, sizeof(sBonus.byAttBonus));
		memset(&sBonus.byDefBonus, 0, sizeof(sBonus.byDefBonus));
		memset(&sBossDropItem, 0, sizeof(sBossDropItem));
		memset(&sResults, 0, sizeof(sResults));
	}

	uint8_t byHeader;
	uint8_t bySubHeader;
	uint16_t byIndex;
	uint16_t byType;
	bool bReset;
	long lMapIndex;
	long lEntryMapIndex;
	uint32_t dwBossVnum;
	LevelLimit sLevelLimit;
	Item sRequiredItem[MAX_REQUIRED_ITEMS];
	uint32_t dwDuration;
	uint32_t dwCooldown;
	uint8_t byElement;
	Bonus sBonus;
	Item sBossDropItem[MAX_BOSS_ITEM_SLOTS];
	Results sResults;
} TPacketGCDungeonInfo;

typedef struct SPacketGCDungeonInfoRank
{
	SPacketGCDungeonInfoRank() { strncpy(szName, "", sizeof(szName)); }
	SPacketGCDungeonInfoRank(const char* c_szName, const int c_iLevel, const uint32_t c_dwPoints) :
		iLevel(c_iLevel), dwPoints(c_dwPoints)
	{
		strncpy(szName, c_szName, sizeof(szName));
	}

	uint8_t byHeader = HEADER_GC_DUNGEON_RANKING;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	int iLevel = 0;
	uint32_t dwPoints = 0;
} TPacketGCDungeonInfoRank;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
typedef struct SPacketCGExtBattlePassAction
{
	uint8_t bHeader;
	uint8_t bAction;
} TPacketCGExtBattlePassAction;

typedef struct SPacketCGExtBattlePassSendPremium
{
	uint8_t bHeader;
	bool premium;
} TPacketCGExtBattlePassSendPremium;

typedef struct SPacketGCExtBattlePassOpen
{
	uint8_t bHeader;
} TPacketGCExtBattlePassOpen;

typedef struct SPacketGCExtBattlePassGeneralInfo
{
	uint8_t bHeader;
	uint8_t bBattlePassType;
	char szSeasonName[64 + 1];
	uint32_t dwBattlePassID;
	uint32_t dwBattlePassStartTime;
	uint32_t dwBattlePassEndTime;
} TPacketGCExtBattlePassGeneralInfo;

typedef struct SPacketGCExtBattlePassMissionInfo
{
	uint8_t bHeader;
	uint16_t wSize;
	uint16_t wRewardSize;
	uint8_t bBattlePassType;
	uint32_t dwBattlePassID;
} TPacketGCExtBattlePassMissionInfo;

typedef struct SPacketGCExtBattlePassMissionUpdate
{
	uint8_t bHeader;
	uint8_t bBattlePassType;
	uint8_t bMissionIndex;
	uint8_t bMissionType;
	uint32_t dwNewProgress;
} TPacketGCExtBattlePassMissionUpdate;

typedef struct SPacketGCExtBattlePassRanking
{
	uint8_t bHeader;
	char szPlayerName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t bBattlePassType;
	uint8_t	bBattlePassID;
	uint32_t dwStartTime;
	uint32_t dwEndTime;
} TPacketGCExtBattlePassRanking;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
enum EPacketGCGrowthPetSubHeader
{
	GROWTHPET_SUBHEADER_GC_WINDOW,
};

enum EPacketCGGrowthPetSubHeader
{
	GROWTHPET_SUBHEADER_CG_WINDOW_TYPE,
	GROWTHPET_SUBHEADER_CG_HATCHING_WINDOW,
};

typedef struct SSubPacketCGGrowthPetWindowType
{
	uint8_t bPetWindowType;
} TSubPacketCGGrowthPetWindowType;

typedef struct SSubPacketCGGrowthPetHatchWindow
{
	bool isOpen;
} TSubPacketCGGrowthPetHatchWindow;

typedef struct SPacketGCGrowthPet
{
	SPacketGCGrowthPet() : bHeader(HEADER_GC_GROWTH_PET) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCGrowthPet;

typedef struct SPacketCGGrowthPet
{
	SPacketCGGrowthPet() : bHeader(HEADER_CG_GROWTH_PET) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketCGGrowthPet;

// Client > Game Pet Hatching
typedef struct SPacketCGGrowthPetHatching
{
	uint8_t bHeader;
	uint8_t bPos;
	char sGrowthPetName[CItemData::PET_NAME_MAX_SIZE + 1];
} TPacketCGGrowthPetHatching;

// GC: Pet Hatching Response
typedef struct SPacketGCGrowthPetHatching
{
	uint8_t bHeader;
	uint8_t bPos;
	uint8_t bResult;
} TPacketGCGrowthPetHatching;

// CG: Pet Learn Skill Request
typedef struct SPacketCGGrowthPetLearnSkill
{
	uint8_t bHeader;
	uint8_t bSkillBookSlotIndex;
	uint8_t bSkillBookInvenIndex;
} TPacketCGGrowthPetLearnSkill;

// Client to Game
typedef struct SPacketCGGrowthPetSkillUpgrade
{
	uint8_t bHeader;
	uint8_t bslotPos;
	uint8_t bslotIndex;
} TPacketCGGrowthPetSkillUpgrade;

typedef struct SPacketCGGrowthPetSkillUpgradeRequest
{
	uint8_t bHeader;
	uint8_t bslotPos;
	uint8_t bslotIndex;
} TPacketCGGrowthPetSkillUpgradeRequest;

typedef struct SPacketGCGrowthPetSkillUpgradeRequest
{
	uint8_t bHeader;
	uint8_t bSkillSlot;
	uint8_t bSkillIndex;
	uint32_t iPrice;
} TPacketGCGrowthPetSkillUpgradeRequest;

typedef struct SPacketCGGrowthPetFeedRequest
{
	uint8_t bHeader;
	uint8_t bIndex;
	uint16_t iFeedItemsCount;
	uint16_t iFeedItemsCubeSlot[9];
} TPacketCGGrowthPetFeedRequest;

typedef struct SPacketGCGrowthPetFeedResult
{
	uint8_t bHeader;
	bool bResult;
} TPacketGCGrowthPetFeedResult;

typedef struct SPacketCGGrowthPetDeleteSkill
{
	uint8_t bHeader;
	uint8_t bSkillBookDelSlotIndex;
	uint8_t bSkillBookDelInvenIndex;
} TPacketCGGrowthPetDeleteSkill;

typedef struct SPacketCGGrowthPetDeleteAllSkill
{
	uint8_t bHeader;
	uint8_t bPetSkillAllDelBookIndex;
} TPacketCGGrowthPetDeleteAllSkill;

typedef struct SPacketCGGrowthPetNameChange
{
	uint8_t bHeader;
	char sPetName[CItemData::PET_NAME_MAX_SIZE + 1];
	uint8_t bItemWindowType;
	uint16_t bItemWindowCell;
	uint8_t bPetWindowType;
	uint16_t bPetWindowCell;
} TPacketCGGrowthPetNameChange;

typedef struct SPacketGCGrowthPetNameChangeResult
{
	uint8_t bHeader;
	uint8_t bResult;
} TPacketGCGrowthPetNameChangeResult;

#ifdef ENABLE_PET_ATTR_DETERMINE
// Client Game
// Pet Attr Determine
typedef struct SPacketCGGrowthPetAttrDetermine
{
	uint8_t bHeader;
} TPacketCGGrowthPetAttrDetermine;

// Pet Attr Change
typedef struct SPacketCGGrowthPetAttrChange
{
	uint8_t bHeader;
	uint8_t bPetSlotIndex;
	uint8_t bMaterialSlotIndex;
} TPacketCGGrowthPetAttrChange;

// Game Client
// Pet Attr Determine Result
typedef struct SPacketGCGrowthPetAttrDetermineResult
{
	uint8_t bHeader;
	uint8_t bResult;
} TPacketGCGrowthPetAttrDetermineResult;

// Pet Attr Change
typedef struct SPacketGCGrowthPetAttrChangeResult
{
	uint8_t bHeader;
	uint8_t bResult;
} TPacketGCGrowthPetAttrChangeResult;
#endif

#ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
typedef struct SPacketCGGrowthPetReviveRequest
{
	uint8_t bHeader;
	TItemPos upBringingPos;
	uint16_t pos[PET_REVIVE_MATERIAL_SLOT_MAX];
	uint16_t count[PET_REVIVE_MATERIAL_SLOT_MAX];
} TPacketCGGrowthPetReviveRequest;

typedef struct SPacketGCGrowthPetReviveResult
{
	uint8_t bHeader;
	bool bResult;
} TPacketGCGrowthPetReviveResult;
#endif

typedef struct SPacketGrowthPetInfo
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketGCGrowthPetInfo;
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
typedef struct SPacketMeleyLairTimeInfo
{
	uint8_t bHeader;
	uint32_t dwTime;
} TPacketMeleyLairTimeInfo;

typedef struct SPacketMeleyLairTimeResult
{
	uint8_t bHeader;
	uint8_t bResult;
} TPacketMeleyLairTimeResult;
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
enum ECGBiologSubHeaders
{
	BIOLOG_MANAGER_SUBHEADER_CG_OPEN,
	BIOLOG_MANAGER_SUBHEADER_CG_SEND,
	BIOLOG_MANAGER_SUBHEADER_CG_TIMER,
};

enum EGCBiologSubHeaders
{
	BIOLOG_MANAGER_SUBHEADER_GC_OPEN,
	BIOLOG_MANAGER_SUBHEADER_GC_CLOSE,
};

typedef struct SPacketCGBiologManagerAction
{
	uint8_t bHeader;
	uint8_t bSubHeader;
} TPacketCGBiologManagerAction;

typedef struct SPacketGCBiologManager
{
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCBiologManager;

typedef struct SPacketGCBiologManagerInfo
{
	bool bUpdate;
	uint8_t bRequiredLevel;
	uint32_t iRequiredItem;
	uint16_t wGivenItems;
	uint16_t wRequiredItemCount;
	time_t iGlobalCooldown;
	time_t iCooldown;
	bool iCooldownReminder;
	uint8_t bChance;
	uint16_t wApplyType[MAX_BONUSES_LENGTH]; //@fixme436
	long lApplyValue[MAX_BONUSES_LENGTH];
	uint32_t dRewardItem;
	uint16_t wRewardItemCount;
	bool bSubMission;
	uint32_t dwNpcVnum;
	uint32_t dwRequiredSubItem;
} TPacketGCBiologManagerInfo;
#endif

#ifdef ENABLE_HUNTING_SYSTEM
typedef struct SPacketCGHuntingAction
{
	uint8_t bHeader;
	uint8_t bAction;
	uint32_t dValue;
} TPacketGCHuntingAction;

typedef struct SPacketCGOpenWindowHuntingMain
{
	uint8_t bHeader;
	uint32_t dLevel;
	uint32_t dMonster;
	uint32_t dCurCount;
	uint32_t dDestCount;
	uint32_t dMoneyMin;
	uint32_t dMoneyMax;
	uint32_t dExpMin;
	uint32_t dExpMax;
	uint32_t dRaceItem;
	uint32_t dRaceItemCount;
} TPacketGCOpenWindowHuntingMain;

typedef struct SPacketCGOpenWindowHuntingSelect
{
	uint8_t bHeader;
	uint32_t dLevel;
	uint8_t bType;
	uint32_t dMonster;
	uint32_t dCount;
	uint32_t dMoneyMin;
	uint32_t dMoneyMax;
	uint32_t dExpMin;
	uint32_t dExpMax;
	uint32_t dRaceItem;
	uint32_t dRaceItemCount;
} TPacketGCOpenWindowHuntingSelect;

typedef struct SPacketGCOpenWindowReward
{
	uint8_t bHeader;
	uint32_t dLevel;
	uint32_t dReward;
	uint32_t dRewardCount;
	uint32_t dRandomReward;
	uint32_t dRandomRewardCount;
	uint32_t dMoney;
	uint8_t bExp;
} TPacketGCOpenWindowReward;

typedef struct SPacketGCUpdateHunting
{
	uint8_t bHeader;
	uint32_t dCount;
} TPacketGCUpdateHunting;

typedef struct SPacketGCReciveRandomItems
{
	uint8_t bHeader;
	uint8_t bWindow;
	uint32_t dItemVnum;
	uint32_t dItemCount;
} TPacketGCReciveRandomItems;
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
enum
{
	SUBHEADER_CG_OPEN_LOTTERY_WINDOW,
	SUBHEADER_CG_OPEN_RANKING_WINDOW,
	SUBHEADER_GC_OPEN_LOTTERY_WINDOW,
	SUBHEADER_GC_OPEN_RANKING_WINDOW,
	SUBHEADER_CG_TICKET_DELETE,
	SUBHEADER_CG_RECIVE_MONEY
};

typedef struct SPacketCGLotteryOpenings
{
	uint8_t header;
	uint8_t subheader;
} TPacketCGLotteryOpenings;

typedef struct SPacketCGSendTicketOptions
{
	uint8_t header;
	uint8_t subheader;
	int	ticketSlot;
} TPacketCGSendTicketOptions;

typedef struct SPacketCGSendLottoNewTicket
{
	uint8_t header;
	int slot;
	int num1;
	int num2;
	int num3;
	int num4;
} TPacketCGSendLottoNewTicket;

typedef struct SPacketCGSendLottoPickMoney
{
	uint8_t header;
	long long amount;
} TPacketCGSendLottoPickMoney;

typedef struct SPacketGCLotteryOpenings
{
	uint8_t header;
	uint8_t subheader;
} TPacketGCLotteryOpenings;

typedef struct SPacketGCReciveLottoBaseInfo
{
	uint8_t header;
	int lottoSlot;
	int lottoNum;
	int num1;
	int num2;
	int num3;
	int num4;
	long long jackpot;
	long long nextRefresh;
} TPacketGCReciveLottoBaseInfo;

typedef struct SPacketGCReciveLottoTicketInfo
{
	uint8_t header;
	int tNum;
	int tID;
	int num1;
	int num2;
	int num3;
	int num4;
	int lottoID;
	char buytime[24 + 1];
	int state;
	int winNumbers;
	long long MoneyWin;
} TPacketGCReciveLottoTicketInfo;

typedef struct SPacketGCSendRankingJackpotInfo
{
	uint8_t header;
	char playername[24 + 1];
	int lottoID;
	long long money;
	char date[24 + 1];
} TPacketGCSendRankingJackpotInfo;

typedef struct SPacketGCSendRankingMoneyInfo
{
	uint8_t header;
	char playername[24 + 1];
	int level;
	int empire;
	long long money;
} TPacketGCSendRankingMoneyInfo;
#endif

// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
typedef struct SPacketCGRequestEventData
{
	uint8_t bHeader;
	uint8_t bMonth;
} TPacketCGRequestEventData;

typedef struct SPacketGCEventInfo
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketGCEventInfo;

typedef struct SPacketGCEventReload
{
	uint8_t bHeader;
} TPacketGCEventReload;

typedef struct SPacketEventData
{
	uint32_t dwID;
	uint8_t bType;
	long startTime;
	long endTime;
	uint32_t dwVnum;
	int iPercent;
	int iDropType;
	uint8_t bCompleted;
} TPacketEventData;

typedef struct command_request_event_quest
{
	uint8_t bHeader;
	char szName[QUEST_NAME_MAX_NUM + 1];
} TPacketCGRequestEventQuest;

typedef struct SPacketGCEventKWScore
{
	uint8_t bHeader;
	uint16_t wKingdomScores[3];
} TPacketGCEventKWScore;
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
/*Client -> Server*/
enum EPacketCGMiniGameSubHeaderOkeyNormal
{
	SUBHEADER_CG_RUMI_START,
	SUBHEADER_CG_RUMI_EXIT,
	SUBHEADER_CG_RUMI_DECKCARD_CLICK,
	SUBHEADER_CG_RUMI_HANDCARD_CLICK,
	SUBHEADER_CG_RUMI_FIELDCARD_CLICK,
	SUBHEADER_CG_RUMI_DESTROY,
};

typedef struct SPacketCGMiniGameOkeyCard
{
	SPacketCGMiniGameOkeyCard() : bHeader(HEADER_CG_OKEY_CARD) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketCGMiniGameOkeyCard;

typedef struct SSubPacketCGMiniGameCardOpenClose
{
	uint8_t bSafeMode;
} TSubPacketCGMiniGameCardOpenClose;

typedef struct SSubPacketCGMGHandCardClick
{
	int index;
} TSubPacketCGMiniGameHandCardClick;

typedef struct SSubPacketCGMGFieldCardClick
{
	int index;
} TSubPacketCGMiniGameFieldCardClick;

typedef struct SSubPacketCGMGDestroy
{
	int index;
} TSubPacketCGMiniGameDestroy;

/*Server -> Client*/
enum EPacketGCMiniGameSubHeaderOkeyNormal
{
	SUBHEADER_GC_RUMI_OPEN,
	SUBHEADER_GC_RUMI_CARD_UPDATE,
	SUBHEADER_GC_RUMI_CARD_REWARD,
};

typedef struct SPacketGCMiniGameOkeyCard
{
	SPacketGCMiniGameOkeyCard() : bHeader(HEADER_GC_OKEY_CARD) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TPacketGCMiniGameOkeyCard;

typedef struct SSubPacketGCMiniGameCardOpenClose
{
	uint8_t bSafeMode;
} TSubPacketGCMiniGameCardOpenClose;

typedef struct SSubPacketGCMiniGameCardsInfo
{
	uint32_t cardHandType[5];
	uint32_t cardHandValue[5];
	uint8_t cardHandLeft;
	uint32_t cHandPoint;

	uint32_t cardFieldType[3];
	uint32_t cardFieldValue[3];
	uint32_t cFieldPoint;
} TSubPacketGCMiniGameCardsInfo;

typedef struct SSubPacketGCMiniGameCardsReward
{
	uint32_t cardType[3];
	uint32_t cardValue[3];
	uint32_t cPoint;
} TSubPacketGCMiniGameCardsReward;
#endif
#ifdef ENABLE_MONSTER_BACK
typedef struct SRewardItem
{
	uint32_t dwDay;
	uint32_t dwVnum;
	uint32_t dwCount;
} TRewardItem;

typedef struct SPacketGCAttendanceEventInfo
{
	uint8_t bHeader;
	uint16_t wSize;
	bool bIsEnable;
	uint32_t dwDay;
	uint8_t bClear;
	uint8_t bReward;
} TPacketGCAttendanceEventInfo;

typedef struct SPacketGCAttendanceEvent
{
	uint8_t bHeader;
	uint8_t bType;
	uint8_t bValue;
} TPacketGCAttendanceEvent;
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
typedef struct SHitCountInfo
{
	SHitCountInfo() { dwVid = dwCount = 0; }
	SHitCountInfo(uint32_t vid, uint32_t count)
	{
		dwVid = vid;
		dwCount = count;
	}
	uint32_t dwVid;
	uint32_t dwCount;
} THitCountInfo;

typedef struct SPacketGCHitCountInfo {
	uint8_t bHeader;
	uint32_t dwVid;
	uint32_t dwCount;
} TPacketGCHitCountInfo;
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
/*Client -> Server*/
enum EPacketCGMiniGameSubHeaderCatchKing
{
	SUBHEADER_CG_CATCH_KING_START,
	SUBHEADER_CG_CATCH_KING_DECKCARD_CLICK,
	SUBHEADER_CG_CATCH_KING_FIELDCARD_CLICK,
	SUBHEADER_CG_CATCH_KING_REWARD,
};

typedef struct SPacketCGMiniGameCatchKing
{
	SPacketCGMiniGameCatchKing() : bHeader(HEADER_CG_MINI_GAME_CATCH_KING) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubheader;
} TPacketCGMiniGameCatchKing;

typedef struct SSubPacketCGMiniGameCatchKingStart {
	uint8_t betNumber;
} TSubPacketCGMiniGameCatchKingStart;

typedef struct SSubPacketCGMiniGameCatchKingFieldCardClick {
	uint8_t cardNumber;
} TSubPacketCGMiniGameCatchKingFieldCardClick;

/*Server -> Client*/
enum EPacketGCMiniGameSubHeaderCatchKing
{
	SUBHEADER_GC_CATCH_KING_EVENT_INFO,
	SUBHEADER_GC_CATCH_KING_START,
	SUBHEADER_GC_CATCH_KING_SET_CARD,
	SUBHEADER_GC_CATCH_KING_RESULT_FIELD,
	SUBHEADER_GC_CATCH_KING_SET_END_CARD,
	SUBHEADER_GC_CATCH_KING_REWARD,
};

typedef struct SPacketGCMiniGameCatchKing
{
	SPacketGCMiniGameCatchKing() : bHeader(HEADER_GC_MINI_GAME_CATCH_KING) {}
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubheader;
} TPacketGCMiniGameCatchKing;

typedef struct SSubPacketGCCatchKingStart
{
	uint32_t dwBigScore;
} TSubPacketGCCatchKingStart;

typedef struct SSubPacketGCCatchKingSetCard
{
	uint8_t bCardInHand;
} TSubPacketGCCatchKingSetCard;

typedef struct SSubPacketGCCatchKingResult
{
	uint32_t dwPoints;
	uint8_t bRowType;
	uint8_t bCardPos;
	uint8_t bCardValue;
	bool bKeepFieldCard;
	bool bDestroyHandCard;
	bool bGetReward;
	bool bIsFiveNearBy;
} TSubPacketGCCatchKingResult;

typedef struct SSubPacketGCCatchKingSetEndCard
{
	uint8_t bCardPos;
	uint8_t bCardValue;
} TSubPacketGCCatchKingSetEndCard;

typedef struct SSubPacketGCCatchKingReward
{
	uint8_t bReturnCode;
} TSubPacketGCCatchKingReward;

typedef struct SSubPacketGCCatchKingEventInfo
{
	uint8_t bIsEnable;
} TSubPacketGCCatchKingEventInfo;
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
typedef struct packet_stone_event
{
	uint8_t header;
	int stone_point;
} TPacketGCStoneEvent;
#endif
#ifdef ENABLE_FISH_EVENT
enum EFishEvent
{
	FISH_EVENT_SUBHEADER_BOX_USE,
	FISH_EVENT_SUBHEADER_SHAPE_ADD,
	FISH_EVENT_SUBHEADER_GC_REWARD,
	FISH_EVENT_SUBHEADER_GC_ENABLE,
};

typedef struct SPacketGCFishEvent
{
	uint8_t bHeader;
	uint8_t bSubheader;
} TPacketCGFishEvent;

typedef struct SPacketGCFishEventInfo
{
	uint8_t bHeader;
	uint8_t bSubheader;
	uint32_t dwFirstArg;
	uint32_t dwSecondArg;
} TPacketGCFishEventInfo;
#endif

#ifdef ENABLE_MINI_GAME_BNW
enum
{
	SUBHEADER_GC_BNW_START,
	SUBHEADER_GC_BNW_COMPARE,
	SUBHEADER_GC_BNW_RANKING,
	SUBHEADER_GC_BNW_UPDATE_RANKING,
};

typedef struct SPacketCGMiniGameBNW {
	uint8_t bHeader;
	uint8_t bSubheader;
	uint8_t bSubArgument;
} TPacketCGMiniGameBNW;

typedef struct SPacketGCMiniGameBNW {
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubheader;
} TPacketGCMiniGameBNW;

typedef struct SPacketGCMiniGameBNWCompare {
	char cWinIndex;
	uint8_t bClickedIndex;
	uint8_t bOpponentIndex;
	uint8_t bMyScore;
} TPacketGCMiniGameBNWCompare;

typedef struct SBNWRankPlayer {
	uint8_t bPos;
	uint8_t bIsMain;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint16_t wDonationsCount;
} TBNWRankPlayer;
#endif

#ifdef ENABLE_MINI_GAME_FINDM
enum
{
	SUBHEADER_GC_FIND_M_START,
	SUBHEADER_GC_FIND_M_RESULT_CLICK,
	SUBHEADER_GC_FIND_M_REWARD,
	SUBHEADER_GC_FIND_M_UPGRADE,
	SUBHEADER_GC_FIND_M_END,
#ifdef ENABLE_MINI_GAME_FINDM_HINT
	SUBHEADER_GC_FIND_M_HINT,
#endif
};

typedef struct SFindMCard
{
	SFindMCard() { wMonster = 0; bIsRevealed = false; }
	SFindMCard(uint16_t monster, bool isRevealed)
	{
		wMonster = monster;
		bIsRevealed = isRevealed;
	}

	uint16_t wMonster;
	bool bIsRevealed;
} TFindMCard;

typedef struct SPacketCGMiniGameFindM {
	uint8_t bHeader;
	uint8_t bSubheader;
	uint8_t bSubArgument;
} TPacketCGMiniGameFindM;

typedef struct SPacketGCMiniGameFindM {
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubheader;
} TPacketGCMiniGameFindM;

typedef struct SPacketGCMiniGameFindMStart {
	uint32_t dwStartTime;
	uint32_t dwNextReveal;
#ifdef ENABLE_MINI_GAME_FINDM_HINT
	uint8_t bHintCount;
#endif
} TPacketGCMiniGameFindMStart;

typedef struct SPacketGCMiniGameFindMUpgrade {
	uint32_t dwRewardVnum;
	uint32_t dwNeedMoney;
} TPacketGCMiniGameFindMUpgrade;

typedef struct SPacketGCMiniGameFindMResult {
	uint8_t bCardIndex;
	uint16_t wMonsterVnum;
	uint8_t bHideRevealed;
	uint32_t dwNextReveal;
	uint16_t wTryCount;
} TPacketGCMiniGameFindMResult;

#ifdef ENABLE_MINI_GAME_FINDM_HINT
typedef struct SPacketGCMiniGameFindMHint {
	uint8_t bHintCount;
	uint32_t dwNextReveal;
	uint16_t wMonsterVnum[27];
} TPacketGCMiniGameFindMHint;
#endif
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
enum
{
	SUBHEADER_GC_START_GAME,
	SUBHEADER_GC_END_GAME,

	SUBHEADER_GC_THROW_YUT,
	SUBHEADER_GC_NEXT_TURN,
	SUBHEADER_GC_YUT_MOVE,
	SUBHEADER_GC_SHOW_AVAILABLE_AREA,
	SUBHEADER_GC_PUSH_CATCH_YUT,

	SUBHEADER_GC_SET_PROB,
	SUBHEADER_GC_SET_SCORE,
	SUBHEADER_GC_SET_REMAIN_COUNT,
};

typedef struct SPacketCGMiniGameYutNori {
	uint8_t bHeader;
	uint8_t bSubheader;
	uint8_t bSubArgument;
} TPacketCGMiniGameYutNori;

typedef struct SPacketGCMiniGameYutNori {
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubheader;
} TPacketGCMiniGameYutNori;

typedef struct SPacketGCMiniGameYutNoriThrow {
	bool bIsPc;
	char cYut;
} TPacketGCMiniGameYutNoriThrow;

typedef struct SPacketGCMiniGameYutNoriNextTurn {
	bool bPcTurn;
	uint8_t bState;
} TPacketGCMiniGameYutNoriNextTurn;

typedef struct SPacketGCMiniGameYutNoriMove {
	bool bIsPc;
	uint8_t bUnitIndex;
	bool bIsCatch;
	uint8_t bStartIndex;
	uint8_t bDestIndex;
} TPacketGCMiniGameYutNoriMove;

typedef struct SPacketGCMiniGameYutNoriAvailableArea {
	uint8_t bPlayerIndex;
	uint8_t bAvailableIndex;
} TPacketGCMiniGameYutNoriAvailableArea;

typedef struct SPacketGCMiniGameYutNoriPushCatchYut {
	bool bIsPc;
	uint8_t bUnitIndex;
} TPacketGCMiniGameYutNoriPushCatchYut;
#endif

#ifdef ENABLE_FLOWER_EVENT
typedef struct packet_send_flower_event
{
	uint8_t header;
	uint8_t id;
} TPacketCGFlowerEventSend;
#endif
// END MINI GAME EVENTS //

#ifdef ENABLE_INGAME_WIKI
enum SHCommunicationsPacketsGC
{
	LOAD_WIKI_ITEM,
	LOAD_WIKI_MOB,
};

typedef struct SCGWikiPacket
{
	uint8_t bHeader;
	uint8_t bSubHeader;
	uint32_t vnum;
	uint64_t ret_id;
} TCGWikiPacket;

typedef struct SGCWikiPacket
{
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bSubHeader;
} TGCWikiPacket;

typedef struct SGCItemWikiPacket
{
	NWikiData::TWikiInfoTable wiki_info;
	int origin_infos_count;
	uint32_t vnum;
	uint64_t ret_id;
} TGCItemWikiPacket;

typedef struct SGCMobWikiPacket
{
	int drop_info_count;
	uint32_t vnum;
	uint64_t ret_id;
} TGCMobWikiPacket;
#endif

#ifdef ENABLE_FISHING_RENEWAL
enum {
	FISHING_SUBHEADER_NEW_START = 0,
	FISHING_SUBHEADER_NEW_STOP = 1,
	FISHING_SUBHEADER_NEW_CATCH = 2,
	FISHING_SUBHEADER_NEW_CATCH_FAIL = 3,
	FISHING_SUBHEADER_NEW_CATCH_SUCCESS = 4,
	FISHING_SUBHEADER_NEW_CATCH_FAILED = 5,
};

typedef struct SPacketFishingNew
{
	uint8_t header;
	uint8_t subheader;
	uint32_t vid;
	int dir;
	uint8_t need;
	uint8_t count;
} TPacketFishingNew;
#endif

#pragma pack(pop)
