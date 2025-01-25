#ifndef __INC_PACKET_H__
#define __INC_PACKET_H__
#include "stdafx.h"
#ifdef ENABLE_TARGET_AFFECT_SHOWER
# include "affect.h"
#endif

enum EPacketClientGameHeaders
{
	HEADER_CG_LOGIN								= 1,
	HEADER_CG_ATTACK							= 2,
	HEADER_CG_CHAT								= 3,
	HEADER_CG_CHARACTER_CREATE					= 4,
	HEADER_CG_CHARACTER_DELETE					= 5,
	HEADER_CG_CHARACTER_SELECT					= 6,
	HEADER_CG_MOVE								= 7,
	HEADER_CG_SYNC_POSITION						= 8,
	HEADER_CG_ENTERGAME							= 10,
	HEADER_CG_ITEM_USE							= 11,
	HEADER_CG_ITEM_DROP							= 12,
	HEADER_CG_ITEM_MOVE							= 13,
	HEADER_CG_ITEM_PICKUP						= 15,
	HEADER_CG_QUICKSLOT_ADD						= 16,
	HEADER_CG_QUICKSLOT_DEL						= 17,
	HEADER_CG_QUICKSLOT_SWAP					= 18,
	HEADER_CG_WHISPER							= 19,
	HEADER_CG_ITEM_DROP2						= 20,
#ifdef ENABLE_DESTROY_SYSTEM
	HEADER_CG_ITEM_DESTROY						= 25,
#endif
	HEADER_CG_ON_CLICK							= 26,
	HEADER_CG_EXCHANGE							= 27,
	HEADER_CG_CHARACTER_POSITION				= 28,
	HEADER_CG_SCRIPT_ANSWER						= 29,
	HEADER_CG_QUEST_INPUT_STRING				= 30,
	HEADER_CG_QUEST_CONFIRM						= 31,
#ifdef ENABLE_PARTY_MATCH
	HEADER_CG_PARTY_MATCH						= 32,
#endif
#ifdef ENABLE_OX_RENEWAL
	HEADER_CG_QUEST_INPUT_LONG_STRING			= 33,
#endif
	HEADER_CG_SHOP								= 50,
	HEADER_CG_FLY_TARGETING						= 51,
	HEADER_CG_USE_SKILL							= 52,
	HEADER_CG_ADD_FLY_TARGETING					= 53,
	HEADER_CG_SHOOT								= 54,
	HEADER_CG_MYSHOP							= 55,
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	HEADER_CG_SKILL_COLOR						= 57,
#endif
#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_CG_TARGET_INFO_LOAD					= 59,
#endif
	HEADER_CG_ITEM_USE_TO_ITEM					= 60,
	HEADER_CG_TARGET							= 61,
	HEADER_CG_WARP								= 65,
	HEADER_CG_SCRIPT_BUTTON						= 66,
	HEADER_CG_MESSENGER							= 67,
	HEADER_CG_MALL_CHECKOUT						= 69,
	HEADER_CG_SAFEBOX_CHECKIN					= 70,
	HEADER_CG_SAFEBOX_CHECKOUT					= 71,
	HEADER_CG_PARTY_INVITE						= 72,
	HEADER_CG_PARTY_INVITE_ANSWER				= 73,
	HEADER_CG_PARTY_REMOVE						= 74,
	HEADER_CG_PARTY_SET_STATE					= 75,
	HEADER_CG_PARTY_USE_SKILL					= 76,
	HEADER_CG_SAFEBOX_ITEM_MOVE					= 77,
	HEADER_CG_PARTY_PARAMETER					= 78,
#ifdef ENABLE_SAFEBOX_MONEY
	HEADER_CG_SAFEBOX_MONEY						= 79,
#endif
	HEADER_CG_GUILD								= 80,
	HEADER_CG_ANSWER_MAKE_GUILD					= 81,
	HEADER_CG_FISHING							= 82,
	HEADER_CG_ITEM_GIVE							= 83,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	HEADER_CG_GUILDSTORAGE_CHECKIN				= 84,
	HEADER_CG_GUILDSTORAGE_CHECKOUT				= 85,
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	HEADER_CG_LOTTO_OPENINGS					= 86,
	HEADER_CG_LOTTO_BUY_TICKET					= 87,
	HEADER_CG_LOTTO_TICKET_OPTIONS				= 88,
	HEADER_CG_LOTTO_PICK_MONEY					= 89,
#endif
	HEADER_CG_EMPIRE							= 90,
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	HEADER_CG_GEM_SHOP							= 91,
	HEADER_CG_SCRIPT_SELECT_ITEM_EX				= 92,
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	HEADER_CG_PRIVATE_SHOP_SEARCH_SEARCH		= 93,
	HEADER_CG_PRIVATE_SHOP_SEARCH_BUY			= 94,
	HEADER_CG_PRIVATE_SHOP_SEARCH_CLOSE			= 95,
#endif
	HEADER_CG_REFINE							= 96,
	HEADER_CG_MARK_LOGIN						= 100,
	HEADER_CG_MARK_CRCLIST						= 101,
	HEADER_CG_MARK_UPLOAD						= 102,
	HEADER_CG_MARK_IDXLIST						= 104,
	HEADER_CG_HACK								= 105,
	HEADER_CG_CHANGE_NAME						= 106,
	HEADER_CG_LOGIN2							= 109,
	HEADER_CG_DUNGEON							= 110,
	HEADER_CG_LOGIN3							= 111,
	HEADER_CG_GUILD_SYMBOL_UPLOAD				= 112,
	HEADER_CG_GUILD_SYMBOL_CRC					= 113,
	HEADER_CG_SCRIPT_SELECT_ITEM				= 114,
#ifdef ENABLE_HWID_BAN
	HEADER_CG_HWID_SYSTEM						= 117,
#endif
#ifdef ENABLE_MYSHOP_DECO
	HEADER_CG_MYSHOP_DECO_STATE					= 118,
	HEADER_CG_MYSHOP_DECO_ADD					= 119,
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//Myshop
	HEADER_CG_MYSHOP_REMOVE_ITEM				= 120,
	HEADER_CG_MYSHOP_ADD_ITEM					= 121,
	HEADER_CG_MYSHOP_CLOSE						= 122,
	HEADER_CG_MYSHOP_WITHDRAW					= 123,
	HEADER_CG_MYSHOP_RENAME						= 124,
	HEADER_CG_MYSHOP_OPEN						= 125,
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	HEADER_CG_MYSHOP_REOPEN						= 126,
#	endif
#endif
#ifdef ENABLE_CHANGED_ATTR
	HEADER_CG_ITEM_SELECT_ATTR					= 127,
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	HEADER_CG_BIOLOG_MANAGER					= 128,
#endif
#ifdef ENABLE_TICKET_SYSTEM
	HEADER_CG_TICKET_SYSTEM						= 129,
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	HEADER_CG_EXTEND_INVEN_REQUEST				= 140,
	HEADER_CG_EXTEND_INVEN_UPGRADE				= 141,
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
	HEADER_CG_COSTUME_REFINE_REQUEST			= 142,
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	HEADER_CG_ACCE_CLOSE_REQUEST				= 143,
	HEADER_CG_ACCE_REFINE_REQUEST				= 144,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_CG_GROWTH_PET						= 145,
	HEADER_CG_GROWTH_PET_HATCHING				= 146,
	HEADER_CG_GROWTH_PET_LEARN_SKILL			= 147,
	HEADER_CG_GROWTH_PET_SKILL_UPGRADE			= 148,
	HEADER_CG_GROWTH_PET_SKILL_UPGRADE_REQUEST	= 149,
	HEADER_CG_GROWTH_PET_FEED_REQUEST			= 150,
	HEADER_CG_GROWTH_PET_DELETE_SKILL			= 151,
	HEADER_CG_GROWTH_PET_ALL_DEL_SKILL			= 152,
	HEADER_CG_GROWTH_PET_NAME_CHANGE			= 153,
#	ifdef ENABLE_PET_ATTR_DETERMINE
	HEADER_CG_GROWTH_PET_ATTR_DETERMINE			= 154,
	HEADER_CG_GROWTH_PET_ATTR_CHANGE			= 155,
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	HEADER_CG_GROWTH_PET_REVIVE_REQUEST			= 156,
#	endif
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM	
	HEADER_CG_DUNGEON_INFO						= 159,
#endif
#ifdef ENABLE_AURA_SYSTEM
	HEADER_CG_AURA								= 160,
#endif
	// MINI_EVENTS
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	HEADER_CG_OKEY_CARD							= 161,
#endif
#ifdef ENABLE_MONSTER_BACK
	HEADER_CG_ATTENDANCE_REWARD					= 162,
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	HEADER_CG_MINI_GAME_CATCH_KING				= 163,
#endif
#ifdef ENABLE_FISH_EVENT
	HEADER_CG_FISH_EVENT_SEND					= 164,
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	HEADER_CG_SOUL_ROULETTE						= 165,
#endif
#ifdef ENABLE_MINI_GAME_BNW
	HEADER_CG_MINI_GAME_BNW 					= 166,
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	HEADER_CG_MINI_GAME_FIND_M 					= 167,
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	HEADER_CG_MINI_GAME_YUT_NORI 				= 168,
#endif
#ifdef ENABLE_FLOWER_EVENT
	HEADER_CG_FLOWER_EVENT_EXCHANGE				= 169,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	HEADER_CG_ACHIEVEMENT						= 170,
#endif
	// END_MINI_EVENTS
#ifdef ENABLE_SWITCHBOT
	HEADER_CG_SWITCHBOT							= 171,
#endif
	HEADER_CG_DRAGON_SOUL_REFINE				= 205,
	HEADER_CG_STATE_CHECKER						= 206,
#ifdef ENABLE_INGAME_WIKI
	HEADER_CG_WIKI								= 207,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	HEADER_CG_CHANGE_LOOK						= 213,
#endif
#ifdef ENABLE_CUBE_RENEWAL
	HEADER_CG_CUBE_RENEWAL						= 217,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_CG_EXT_BATTLE_PASS_ACTION			= 218,
	HEADER_CG_EXT_SEND_BP_PREMIUM				= 219,
#endif
#ifdef ENABLE_HUNTING_SYSTEM
	HEADER_CG_SEND_HUNTING_ACTION				= 220,
#endif
#ifdef ENABLE_REFINE_ELEMENT
	HEADER_CG_ELEMENTS_SPELL 					= 221,
#endif
#ifdef ENABLE_FISHING_RENEWAL
	HEADER_CG_FISHING_NEW						= 222, 
#endif
#ifdef ENABLE_BATTLE_FIELD
	HEADER_CG_REQUEST_ENTER_BATTLE				= 223,
	HEADER_CG_REQUEST_EXIT_BATTLE				= 224,
#endif
#ifdef ENABLE_MAILBOX
	HEADER_CG_MAILBOX_WRITE						= 225,
	HEADER_CG_MAILBOX_WRITE_CONFIRM				= 226,
	HEADER_CG_MAILBOX_PROCESS					= 227,
#endif
#ifdef ENABLE_MOVE_CHANNEL
	HEADER_CG_MOVE_CHANNEL						= 229,
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	HEADER_CG_LANGUAGE_CHANGE					= 230,
	HEADER_CG_TARGET_LANGUAGE_REQUEST			= 231,
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	HEADER_CG_ATTR_6TH_7TH						= 234,
#endif
#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
	HEADER_CG_WON_EXCHANGE						= 235,
#endif
#ifdef ENABLE_EVENT_MANAGER
	HEADER_CG_REQUEST_EVENT_QUEST				= 239,
	HEADER_CG_REQUEST_EVENT_DATA				= 240,
#endif
	HEADER_CG_CLIENT_VERSION2					= 0xf1,	//241
	HEADER_CG_KEY_AGREEMENT						= 0xfb, //251	_IMPROVED_PACKET_ENCRYPTION_
	HEADER_CG_TIME_SYNC							= 0xfc,	//252
	HEADER_CG_CLIENT_VERSION					= 0xfd,	//253
	HEADER_CG_PONG								= 0xfe,	//254
	HEADER_CG_HANDSHAKE							= 0xff,	//255
};

enum EPacketGameClientHeaders
{
	HEADER_GC_CHARACTER_ADD						= 1,
	HEADER_GC_CHARACTER_DEL						= 2,
	HEADER_GC_MOVE								= 3,
	HEADER_GC_CHAT								= 4,
	HEADER_GC_SYNC_POSITION						= 5,
	HEADER_GC_LOGIN_FAILURE						= 7,
	HEADER_GC_CHARACTER_CREATE_SUCCESS			= 8,
	HEADER_GC_CHARACTER_CREATE_FAILURE			= 9,
	HEADER_GC_CHARACTER_DELETE_SUCCESS			= 10,
	HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID	= 11,
	HEADER_GC_STUN								= 13,
	HEADER_GC_DEAD								= 14,
	HEADER_GC_CHARACTER_POINTS					= 16,
	HEADER_GC_CHARACTER_POINT_CHANGE			= 17,
	HEADER_GC_CHARACTER_UPDATE					= 19,
	HEADER_GC_ITEM_DEL							= 20,
	HEADER_GC_ITEM_SET							= 21,
	HEADER_GC_ITEM_USE							= 22,
	HEADER_GC_ITEM_DROP							= 23,
	HEADER_GC_ITEM_UPDATE						= 25,
	HEADER_GC_ITEM_GROUND_ADD					= 26,
	HEADER_GC_ITEM_GROUND_DEL					= 27,
	HEADER_GC_QUICKSLOT_ADD						= 28,
	HEADER_GC_QUICKSLOT_DEL						= 29,
	HEADER_GC_QUICKSLOT_SWAP					= 30,
	HEADER_GC_ITEM_OWNERSHIP					= 31,
	HEADER_GC_LOGIN_SUCCESS_NEWSLOT				= 32,
	HEADER_GC_WHISPER							= 34,
	HEADER_GC_MOTION							= 36,
	HEADER_GC_SHOP								= 38,
	HEADER_GC_SHOP_SIGN							= 39,
	HEADER_GC_DUEL_START						= 40,
	HEADER_GC_PVP								= 41,
	HEADER_GC_EXCHANGE							= 42,
	HEADER_GC_CHARACTER_POSITION				= 43,
	HEADER_GC_PING								= 44,
	HEADER_GC_SCRIPT							= 45,
	HEADER_GC_QUEST_CONFIRM						= 46,
#ifdef ENABLE_CHANGED_ATTR
	HEADER_GC_ITEM_SELECT_ATTR					= 50,
#endif
#ifdef ENABLE_SEND_TARGET_INFO
	HEADER_GC_TARGET_INFO						= 58,
#endif
	HEADER_GC_OWNERSHIP							= 62,
	HEADER_GC_TARGET							= 63,
	HEADER_GC_WARP								= 65,
	HEADER_GC_ADD_FLY_TARGETING					= 69,
	HEADER_GC_CREATE_FLY						= 70,
	HEADER_GC_FLY_TARGETING						= 71,
	HEADER_GC_MESSENGER							= 74,
	HEADER_GC_GUILD								= 75,
	HEADER_GC_SKILL_LEVEL						= 76,
	HEADER_GC_PARTY_INVITE						= 77,
	HEADER_GC_PARTY_ADD							= 78,
	HEADER_GC_PARTY_UPDATE						= 79,
	HEADER_GC_PARTY_REMOVE						= 80,
	HEADER_GC_QUEST_INFO						= 81,
	HEADER_GC_REQUEST_MAKE_GUILD				= 82,
	HEADER_GC_PARTY_PARAMETER					= 83,
#ifdef ENABLE_SAFEBOX_MONEY
	HEADER_GC_SAFEBOX_MONEY_CHANGE				= 84,	// unused
#endif
	HEADER_GC_SAFEBOX_SET						= 85,
	HEADER_GC_SAFEBOX_DEL						= 86,
	HEADER_GC_SAFEBOX_WRONG_PASSWORD			= 87,
	HEADER_GC_SAFEBOX_SIZE						= 88,
	HEADER_GC_FISHING							= 89,
	HEADER_GC_EMPIRE							= 90,
	HEADER_GC_PARTY_LINK						= 91,
	HEADER_GC_PARTY_UNLINK						= 92,
	HEADER_GC_VIEW_EQUIP						= 99,
	HEADER_GC_MARK_BLOCK						= 100,
	HEADER_GC_MARK_IDXLIST						= 102,
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	HEADER_GC_PARTY_POSITION_INFO				= 104,
#endif
	HEADER_GC_TIME								= 106,
	HEADER_GC_CHANGE_NAME						= 107,
	HEADER_GC_DUNGEON							= 110,
	HEADER_GC_WALK_MODE							= 111,
	HEADER_GC_SKILL_GROUP						= 112,
	HEADER_GC_MAIN_CHARACTER					= 113,
	HEADER_GC_SEPCIAL_EFFECT					= 114,
	HEADER_GC_NPC_POSITION						= 115,
	HEADER_GC_REFINE_INFORMATION				= 119,
	HEADER_GC_CHANNEL							= 121,
	HEADER_GC_MALL_OPEN							= 122,
	HEADER_GC_TARGET_UPDATE						= 123,
	HEADER_GC_TARGET_DELETE						= 124,
	HEADER_GC_TARGET_CREATE						= 125,
	HEADER_GC_AFFECT_ADD						= 126,
	HEADER_GC_AFFECT_REMOVE						= 127,
	HEADER_GC_MALL_SET							= 128,
	HEADER_GC_MALL_DEL							= 129,
	HEADER_GC_LAND_LIST							= 130,
	HEADER_GC_LOVER_INFO						= 131,
	HEADER_GC_LOVE_POINT_UPDATE					= 132,
	HEADER_GC_GUILD_SYMBOL_DATA					= 133,
	HEADER_GC_DIG_MOTION						= 134,
	HEADER_GC_DAMAGE_INFO						= 135,
	HEADER_GC_CHAR_ADDITIONAL_INFO				= 136,
	HEADER_GC_MAIN_CHARACTER3_BGM				= 137,
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL			= 138,
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	HEADER_GC_GUILDSTORAGE_OPEN					= 141,
	HEADER_GC_GUILDSTORAGE_SET					= 142,
	HEADER_GC_GUILDSTORAGE_DEL					= 143,
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	HEADER_GC_SOUL_ROULETTE						= 144,
#endif
#ifdef ENABLE_AURA_SYSTEM
	HEADER_GC_AURA								= 145,
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	HEADER_GC_BIOLOG_MANAGER					= 146,
#endif
#ifdef ENABLE_PARTY_MATCH
	HEADER_GC_PARTY_MATCH						= 147,
#endif
#ifdef ENABLE_TICKET_SYSTEM
	HEADER_GC_TICKET_SYSTEM						= 148,
#endif
	HEADER_GC_AUTH_SUCCESS						= 150,
	HEADER_GC_PANAMA_PACK						= 151,
	HEADER_GC_HYBRIDCRYPT_KEYS					= 152,
	HEADER_GC_HYBRIDCRYPT_SDB					= 153,
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	HEADER_GC_GEM_SHOP_OPEN						= 155,
	HEADER_GC_GEM_SHOP_REFRESH					= 156,
	HEADER_GC_GEM_SHOP_BUY						= 157,
	HEADER_GC_GEM_SHOP_ADD						= 158,
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	HEADER_GC_ACCE_REFINE_REQUEST				= 160,
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	HEADER_GC_PRIVATE_SHOP_SEARCH_SET			= 161,
	HEADER_GC_PRIVATE_SHOP_SEARCH_OPEN			= 162,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	HEADER_GC_EXT_BATTLE_PASS_OPEN 				= 163,
	HEADER_GC_EXT_BATTLE_PASS_GENERAL_INFO 		= 164,
	HEADER_GC_EXT_BATTLE_PASS_MISSION_INFO 		= 165,
	HEADER_GC_EXT_BATTLE_PASS_MISSION_UPDATE 	= 166,
	HEADER_GC_EXT_BATTLE_PASS_SEND_RANKING 		= 167,
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
//	HEADER_GC_SHOP_POSITION						= 170,
	HEADER_GC_PLAYER_SHOP_SET					= 171,
	HEADER_GC_MY_SHOP_SIGN						= 172,
	HEADER_GC_SYNC_SHOP_STASH					= 173,
	HEADER_GC_SYNC_SHOP_OFFTIME					= 174,
	HEADER_GC_SYNC_SHOP_POSITION				= 175,
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	HEADER_GC_EXTEND_INVEN_INFO					= 177,
	HEADER_GC_EXTEND_INVEN_RESULT				= 178,
#endif
#ifdef ENABLE_SWITCHBOT
	HEADER_GC_SWITCHBOT							= 180,
#endif
#ifdef ENABLE_MAILBOX
	HEADER_GC_MAILBOX_PROCESS					= 182,
	HEADER_GC_MAILBOX							= 183,
	HEADER_GC_MAILBOX_ADD_DATA					= 184,
	HEADER_GC_MAILBOX_ALL						= 185,
	HEADER_GC_MAILBOX_UNREAD					= 186,
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM	
	HEADER_GC_DUNGEON_INFO						= 187,
	HEADER_GC_DUNGEON_RANKING					= 188,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	HEADER_GC_GROWTH_PET						= 189,
	HEADER_GC_GROWTH_PET_INFO					= 190,
	HEADER_GC_GROWTH_PET_HATCHING				= 191,
	HEADER_GC_GROWTH_PET_UPGRADE_SKILL_REQUEST	= 192,
	HEADER_GC_GROWTH_PET_FEED_RESULT			= 193,
	HEADER_GC_GROWTH_PET_NAME_CHANGE_RESULT		= 194,
#	ifdef ENABLE_PET_ATTR_DETERMINE
	HEADER_GC_GROWTH_ATTR_DETERMINE_RESULT		= 195,
	HEADER_GC_GROWTH_ATTR_CHANGE_RESULT			= 196,
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	HEADER_GC_GROWTH_PET_REVIVE_RESULT			= 197,
#	endif
#endif
#ifdef ENABLE_HUNTING_SYSTEM
	HEADER_GC_HUNTING_OPEN_MAIN					= 198,
	HEADER_GC_HUNTING_OPEN_SELECT				= 199,
	HEADER_GC_HUNTING_OPEN_REWARD				= 200,
	HEADER_GC_HUNTING_UPDATE					= 201,
	HEADER_GC_HUNTING_RECIVE_RAND_ITEMS			= 202,
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	HEADER_GC_TARGET_LANGUAGE_RESULT			= 203,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	HEADER_GC_ACHIEVEMENT						= 207,
#endif
	HEADER_GC_SPECIFIC_EFFECT					= 208,
	HEADER_GC_DRAGON_SOUL_REFINE				= 209,
	HEADER_GC_RESPOND_CHANNELSTATUS				= 210,
#ifdef ENABLE_FISHING_RENEWAL
	HEADER_GC_FISHING_NEW						= 211, 
#endif
#ifdef ENABLE_INGAME_WIKI
	HEADER_GC_WIKI								= 212,
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	HEADER_GC_LOTTO_OPENINGS					= 213,
	HEADER_GC_LOTTO_SEND_BASIC_INFO				= 214,
	HEADER_GC_LOTTO_SEND_TICKET_INFO			= 215,
	HEADER_GC_LOTTO_SEND_RANKING_JACKPOT		= 216,
	HEADER_GC_LOTTO_SEND_RANKING_MONEY			= 217,
#endif
#ifdef ENABLE_CUBE_RENEWAL
	HEADER_GC_CUBE_RENEWAL 						= 219,
#endif
#ifdef ENABLE_12ZI
	HEADER_GC_SEPCIAL_ZODIAC_EFFECT				= 220,
#endif
#ifdef ENABLE_REFINE_ELEMENT
	HEADER_GC_ELEMENTS_SPELL 					= 221,
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
	HEADER_GC_SEAL								= 222,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	HEADER_GC_CHANGE_LOOK						= 223,
#endif
#ifdef ENABLE_BATTLE_FIELD
	HEADER_GC_BATTLE_ZONE_INFO					= 227,
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	HEADER_GC_MELEY_LAIR_TIME_INFO				= 228,
	HEADER_GC_MELEY_LAIR_TIME_RESULT			= 229,
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	HEADER_GC_ATTR_6TH_7TH						= 233,
#endif
#ifdef NEW_SELECT_CHARACTER
	HEADER_GC_CHARACTER_INFORMATION				= 234,
#endif
	// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
	HEADER_GC_EVENT_INFO						= 235,
	HEADER_GC_EVENT_RELOAD						= 236,
	HEADER_GC_EVENT_KW_SCORE					= 237,
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	HEADER_GC_OKEY_CARD							= 238,
#endif
#ifdef ENABLE_MONSTER_BACK
	HEADER_GC_ATTENDANCE_EVENT_INFO				= 239,
	HEADER_GC_ATTENDANCE_EVENT					= 240,
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	HEADER_GC_HIT_COUNT_INFO					= 241,
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	HEADER_GC_MINI_GAME_CATCH_KING				= 242,
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	HEADER_GC_STONE_EVENT 						= 243,
#endif
#ifdef ENABLE_FISH_EVENT
	HEADER_GC_FISH_EVENT_INFO					= 244,
#endif
#ifdef ENABLE_MINI_GAME_BNW
	HEADER_GC_MINI_GAME_BNW 					= 245,
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	HEADER_GC_MINI_GAME_FIND_M					= 246,
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	HEADER_GC_MINI_GAME_YUT_NORI 				= 247,
#endif
	// END MINI GAME EVENTS //

	HEADER_GC_KEY_AGREEMENT_COMPLETED			= 0xfa, //250	_IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_KEY_AGREEMENT						= 0xfb, //251	_IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_TIME_SYNC							= 0xfc,	//252
	HEADER_GC_PHASE								= 0xfd,	//253
	HEADER_GC_BINDUDP							= 0xfe,	//254
	HEADER_GC_HANDSHAKE							= 0xff,	//255
};

enum EPacketGameGameHeaders
{
	HEADER_GG_LOGIN								= 1,
	HEADER_GG_LOGOUT							= 2,
	HEADER_GG_RELAY								= 3,
	HEADER_GG_NOTICE							= 4,
	HEADER_GG_SHUTDOWN							= 5,
	HEADER_GG_GUILD								= 6,
	HEADER_GG_DISCONNECT						= 7,
	HEADER_GG_SHOUT								= 8,
	HEADER_GG_SETUP								= 9,
	HEADER_GG_MESSENGER_ADD                     = 10,
	HEADER_GG_MESSENGER_REMOVE                  = 11,
	HEADER_GG_FIND_POSITION						= 12,
	HEADER_GG_WARP_CHARACTER					= 13,
	//HEADER_GG_MESSENGER_MOBILE					= 14,
	HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX			= 15,
	HEADER_GG_TRANSFER							= 16,
	HEADER_GG_XMAS_WARP_SANTA					= 17,
	HEADER_GG_XMAS_WARP_SANTA_REPLY				= 18,
	HEADER_GG_RELOAD_CRC_LIST					= 19,
	HEADER_GG_LOGIN_PING						= 20,
	HEADER_GG_CHECK_CLIENT_VERSION				= 21,
	HEADER_GG_BLOCK_CHAT						= 22,
	HEADER_GG_BLOCK_EXCEPTION					= 24,
	HEADER_GG_SIEGE								= 25,
	HEADER_GG_MONARCH_NOTICE					= 26,
	HEADER_GG_MONARCH_TRANSFER					= 27,
	//											= 28,
	HEADER_GG_CHECK_AWAKENESS					= 29,
#ifdef ENABLE_FULL_NOTICE
	HEADER_GG_BIG_NOTICE						= 30,
#endif
#ifdef ENABLE_SWITCHBOT
	HEADER_GG_SWITCHBOT							= 31,
#endif
#ifdef ENABLE_BATTLE_FIELD
	HEADER_GG_COMMAND							= 39,
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	HEADER_GG_SHOP_SALE_INFO					= 40,
	HEADER_GG_SHOP_OFFLINE_START				= 41,
	HEADER_GG_SHOP_OFFLINE_END					= 42,
#endif
#ifdef ENABLE_RANKING_SYSTEM
	HEADER_GG_LOAD_RANKING						= 43,
#endif
#ifdef ENABLE_MESSENGER_BLOCK
	HEADER_GG_MESSENGER_BLOCK_ADD 				= 44,
	HEADER_GG_MESSENGER_BLOCK_REMOVE 			= 45,
#endif
#ifdef ENABLE_MULTI_FARM_BLOCK
	HEADER_GG_MULTI_FARM						= 46,
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	HEADER_GG_STONE_EVENT						= 48,
#endif
#ifdef ENABLE_EVENT_MANAGER
	HEADER_GG_EVENT_RELOAD						= 49,
	HEADER_GG_EVENT								= 50,
	HEADER_GG_EVENT_HIDE_AND_SEEK				= 51,
	HEADER_GG_EVENT_BATTLE_PASS					= 52,
#endif
#ifdef ENABLE_WORLD_BOSS
	HEADER_GG_WORLD_BOSS						= 53,
#endif
};

#pragma pack(1)
typedef struct SPacketGGSetup
{
	uint8_t bHeader;
	uint16_t wPort;
	uint8_t bChannel;
} TPacketGGSetup;

typedef struct SPacketGGLogin
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t dwPID;
	uint8_t bEmpire;
	long lMapIndex;
	uint8_t bChannel;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketGGLogin;

typedef struct SPacketGGLogout
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGLogout;

typedef struct SPacketGGRelay
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lSize;
} TPacketGGRelay;

typedef struct SPacketGGNotice
{
	uint8_t bHeader;
	long lSize;
} TPacketGGNotice;

typedef struct SPacketGGMonarchNotice
{
	uint8_t bHeader;
	uint8_t bEmpire;
	long lSize;
} TPacketGGMonarchNotice;

typedef struct SPacketGGShutdown
{
	uint8_t bHeader;
} TPacketGGShutdown;

typedef struct SPacketGGGuild
{
	uint8_t bHeader;
	uint8_t bSubHeader;
	uint32_t dwGuild;
} TPacketGGGuild;

enum
{
	GUILD_SUBHEADER_GG_CHAT,
	GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS,
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	GUILD_SUBHEADER_GG_REFRESH,
	GUILD_SUBHEADER_GG_REFRESH1,
#endif
};

typedef struct SPacketGGGuildChat
{
	uint8_t bHeader;
	uint8_t bSubHeader;
	uint32_t dwGuild;
	char szText[CHAT_MAX_LEN + 1];
} TPacketGGGuildChat;

enum
{
	PARTY_SUBHEADER_GG_CREATE,
	PARTY_SUBHEADER_GG_DESTROY,
	PARTY_SUBHEADER_GG_JOIN,
	PARTY_SUBHEADER_GG_QUIT,
};

typedef struct SPacketGGDisconnect
{
	uint8_t bHeader;
	char szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGDisconnect;

typedef struct SPacketGGShout
{
	uint8_t bHeader;
	uint8_t bEmpire;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
	char szText[CHAT_MAX_LEN + 1];
#ifdef ENABLE_MESSENGER_BLOCK
	char szName[CHARACTER_NAME_MAX_LEN + 1];
#endif
#ifdef ENABLE_EMPIRE_FLAG
	bool bFlag;
#endif
} TPacketGGShout;

typedef struct SPacketGGXmasWarpSanta
{
	uint8_t bHeader;
	uint8_t bChannel;
	long lMapIndex;
} TPacketGGXmasWarpSanta;

typedef struct SPacketGGXmasWarpSantaReply
{
	uint8_t bHeader;
	uint8_t bChannel;
} TPacketGGXmasWarpSantaReply;

typedef struct SPacketGGMessenger
{
	uint8_t bHeader;
	char szAccount[CHARACTER_NAME_MAX_LEN + 1];
	char szCompanion[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGGMessenger;

typedef struct SPacketGGMessengerMobile
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	char szMobile[MOBILE_MAX_LEN + 1];
} TPacketGGMessengerMobile;

typedef struct SPacketGGFindPosition
{
	uint8_t header;
	uint32_t dwFromPID; // who wants to warp to that position
	uint32_t dwTargetPID; // seeker
} TPacketGGFindPosition;

typedef struct SPacketGGWarpCharacter
{
	uint8_t header;
	uint32_t pid;
	long x;
	long y;
#ifdef ENABLE_CMD_WARP_IN_DUNGEON
	int mapIndex;
#endif
} TPacketGGWarpCharacter;

typedef struct SPacketGGGuildWarMapIndex
{
	uint8_t bHeader;
	uint32_t dwGuildID1;
	uint32_t dwGuildID2;
	long lMapIndex;
} TPacketGGGuildWarMapIndex;

typedef struct SPacketGGTransfer
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lX, lY;
} TPacketGGTransfer;

typedef struct SPacketGGLoginPing
{
	uint8_t bHeader;
	char szLogin[LOGIN_MAX_LEN + 1];
} TPacketGGLoginPing;

typedef struct SPacketGGBlockChat
{
	uint8_t bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lBlockDuration;
} TPacketGGBlockChat;

#ifdef ENABLE_RANKING_SYSTEM
typedef struct SPacketGGLoadRanking
{
	uint8_t bHeader;
	uint8_t bCategory;
} TPacketGGLoadRanking;
#endif

#ifdef ENABLE_BATTLE_FIELD
typedef struct SPacketGGCommand
{
	uint8_t bHeader;
	long lSize;
} TPacketGGCommand;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct SPacketGGShopStartOffline
{
	uint8_t bHeader;
	uint32_t dwPID;
	uint8_t bEmpire;
	long lMapIndex;
	uint8_t bChannel;
} TPacketGGShopStartOffline;

typedef struct SPacketGGShopEndOffline
{
	uint8_t bHeader;
	uint32_t dwPID;
} TPacketGGShopEndOffline;
#endif

typedef struct tag_GGSiege
{
	uint8_t bHeader;
	uint8_t bEmpire;
	uint8_t bTowerCount;
} TPacketGGSiege;

typedef struct SPacketGGMonarchTransfer
{
	uint8_t bHeader;
	uint32_t dwTargetPID;
	long x;
	long y;
} TPacketMonarchGGTransfer;

typedef struct SPacketGGBigNotice
{
	uint8_t bHeader;
	long lSize;
} TPacketGGBigNotice;

typedef struct SPacketGGCheckAwakeness
{
	uint8_t bHeader;
} TPacketGGCheckAwakeness;

#ifdef ENABLE_MULTI_FARM_BLOCK
enum
{
	MULTI_FARM_SET,
	MULTI_FARM_REMOVE,
};

typedef struct SPacketGGMultiFarm
{
	uint8_t header;
	uint32_t size;
	uint8_t subHeader;
	char playerIP[IP_ADDRESS_LENGTH + 1];
	uint32_t playerID;
	char playerName[CHARACTER_NAME_MAX_LEN + 1];
	bool farmStatus;
	uint8_t affectType;
	int affectTime;
} TPacketGGMultiFarm;
#endif

#ifdef ENABLE_EVENT_MANAGER
typedef struct SPacketGGReloadEvent
{
	uint8_t bHeader;
} TPacketGGReloadEvent;

typedef struct SPacketGGEvent
{
	uint8_t bHeader;
	TEventTable table;
	bool bState;
} TPacketGGEvent;

typedef struct SPacketGGEventHideAndSeek
{
	uint8_t bHeader;
	int iPosition;
	int iRound;
} TPacketGGEventHideAndSeek;

typedef struct SPacketGCEventKWScore
{
	uint8_t bHeader;
	uint16_t wKingdomScores[3];
} TPacketGCEventKWScore;

typedef struct SPacketGGEventBattlePass
{
	uint8_t bHeader;
	TEventTable table;
	bool bState;
	uint8_t bType;
} TPacketGGEventBattlePass;
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
typedef struct packet_stone_event_p2p
{
	uint8_t header;
	uint32_t pid;
} TPacketGGStoneEvent;
#endif

#ifdef ENABLE_WORLD_BOSS
typedef struct SPacketGGSendWorldBossStates
{
	uint8_t	bHeader;
	uint8_t	WBState;
	long long WBTimer;
	long long WBCooldown;
} TPacketGGSendWorldBossStates;
#endif

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
	// Data is appended as much as size * (uint16_t + uint16_t) after
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
	uint8_t header;
	uint32_t bufSize;
	uint8_t imgIdx;
	uint32_t count;
	// 64 x 48 x pixel size (4 bytes) = 12288 data appended after
} TPacketGCMarkBlock;

typedef struct command_symbol_upload
{
	uint8_t header;
	uint16_t size;
	uint32_t guild_id;
} TPacketCGGuildSymbolUpload;

typedef struct command_symbol_crc
{
	uint8_t header;
	uint32_t guild_id;
	uint32_t crc;
	uint32_t size;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data
{
	uint8_t header;
	uint16_t size;
	uint32_t guild_id;
} TPacketGCGuildSymbolData;

typedef struct packet_pong
{
	uint8_t bHeader;
} TPacketCGPong;

enum GUILD_SUBHEADER_CG
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
	uint8_t header;
	uint8_t subheader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	uint8_t header;
	char guild_name[GUILD_NAME_MAX_LEN + 1];
} TPacketCGAnswerMakeGuild;

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
	GUILD_SUBHEADER_GC_WAR_SCORE,
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

typedef struct packet_guild_name_t
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
	uint32_t guildID;
	char guildName[GUILD_NAME_MAX_LEN];
#ifdef ENABLE_COLORED_GUILD_RANKS
	int guildRank;
#endif
} TPacketGCGuildName;

typedef struct command_guild_use_skill
{
	uint32_t dwVnum;
	uint32_t dwPID;
} TPacketCGGuildUseSkill;

typedef struct command_login
{
	uint8_t header;
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
} TPacketCGLogin;

// start - packets to connect to the authorization server
typedef struct command_login2
{
	uint8_t header;
	char login[LOGIN_MAX_LEN + 1];
	uint32_t dwLoginKey;
	uint32_t adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
	uint8_t header;
	char login[LOGIN_MAX_LEN + 1];
	char passwd[PASSWD_MAX_LEN + 1];
#ifdef ENABLE_HWID_BAN
	char hwid[HWID_MAX_LEN + 1];
#endif
	uint32_t adwClientKey[4];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketCGLogin3;
// end - packets for accessing the authorization server

typedef struct command_player_select
{
	uint8_t header;
	uint8_t index;
} TPacketCGPlayerSelect;

// attack
typedef struct command_attack
{
	uint8_t bHeader;
	uint8_t bType; // attack type
	uint32_t dwVID; // Enemy VID
	uint8_t bCRCMagicCubeProcPiece;
	uint8_t bCRCMagicCubeFilePiece;
} TPacketCGAttack;

/* chat (3) */
typedef struct command_chat // variable packets
{
	uint8_t header;
	uint16_t size;
	uint8_t type;
} TPacketCGChat;

/* whisper */
typedef struct command_whisper
{
	uint8_t bHeader;
	uint16_t wSize;
	char szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

typedef struct command_entergame
{
	uint8_t header;
} TPacketCGEnterGame;

typedef struct command_item_use
{
	uint8_t header;
	TItemPos Cell;
} TPacketCGItemUse;

typedef struct command_item_use_to_item
{
	uint8_t header;
	TItemPos Cell;
	TItemPos TargetCell;
} TPacketCGItemUseToItem;

typedef struct command_item_drop
{
	uint8_t header;
	TItemPos Cell;
	uint32_t gold;
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
	uint8_t header;
	TItemPos Cell;
	uint32_t gold;
	uint8_t count;
} TPacketCGItemDrop2;

typedef struct command_item_move
{
	uint8_t header;
	TItemPos Cell;
	TItemPos CellTo;
	uint8_t count;
} TPacketCGItemMove;

#ifdef ENABLE_DESTROY_SYSTEM
typedef struct command_item_destroy
{
	uint8_t header;
	TItemPos Cell;
	uint32_t gold;
	uint8_t count;
} TPacketCGItemDestroy;
#endif

typedef struct command_item_pickup
{
	uint8_t header;
	uint32_t vid;
} TPacketCGItemPickup;

typedef struct command_quickslot_add
{
	uint8_t header;
	uint8_t pos;
	TQuickSlot slot;
} TPacketCGQuickslotAdd;

typedef struct command_quickslot_del
{
	uint8_t header;
	uint8_t pos;
} TPacketCGQuickslotDel;

typedef struct command_quickslot_swap
{
	uint8_t header;
	uint8_t pos;
	uint8_t change_pos;
} TPacketCGQuickslotSwap;

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
	EXCHANGE_SUBHEADER_CG_START, /* arg1 == vid of target character */
	EXCHANGE_SUBHEADER_CG_ITEM_ADD, /* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ITEM_DEL, /* arg1 == position of item */
	EXCHANGE_SUBHEADER_CG_ELK_ADD, /* arg1 == amount of gold */
	EXCHANGE_SUBHEADER_CG_ACCEPT, /* arg1 == not used */
	EXCHANGE_SUBHEADER_CG_CANCEL, /* arg1 == not used */
};

typedef struct command_exchange
{
	uint8_t header;
	uint8_t sub_header;
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
	//char file[32 + 1];
	//uint8_t answer[16 + 1];
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
	long lX;
	long lY;
	uint32_t dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
	uint32_t dwVID;
	long lX;
	long lY;
} TPacketCGSyncPositionElement;

// location sync
typedef struct command_sync_position // variable packets
{
	uint8_t bHeader;
	uint16_t wSize;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting
{
	uint8_t bHeader;
	uint32_t dwTargetVID;
	long x;
	long y;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
	uint8_t bHeader;
	uint32_t dwShooterVID;
	uint32_t dwTargetVID;
	long x;
	long y;
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

typedef struct command_use_skill
{
	uint8_t bHeader;
	uint32_t dwVnum;
	uint32_t dwVID;
} TPacketCGUseSkill;

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

///////////////////////////////////////////////////////////////////////////////////
// Messenger
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

typedef struct packet_messenger_list_online
{
	uint8_t connected;
	uint8_t length;
	//uint8_t length_char_name;
} TPacketGCMessengerListOnline;

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
#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_BLOCK_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_BLOCK_REMOVE_BLOCK,
#endif
	MESSENGER_SUBHEADER_CG_ADD_BY_VID,
	MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
	MESSENGER_SUBHEADER_CG_REMOVE,
	// MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
};

typedef struct command_messenger
{
	uint8_t header;
	uint8_t subheader;
} TPacketCGMessenger;

typedef struct command_messenger_add_by_vid
{
	uint32_t vid;
} TPacketCGMessengerAddByVID;

#ifdef ENABLE_MESSENGER_BLOCK
typedef struct command_messenger_add_block_by_vid
{
	uint32_t vid;
} TPacketCGMessengerAddBlockByVID;

typedef struct command_messenger_add_block_by_name
{
	uint8_t length;
} TPacketCGMessengerAddBlockByName;

typedef struct command_messenger_remove_block
{
	char login[LOGIN_MAX_LEN + 1];
} TPacketCGMessengerRemoveBlock;
#endif

///////////////////////////////////////////////////////////////////////////////////
// Party
enum
{
	PARTY_SKILL_HEAL = 1,
	PARTY_SKILL_WARP = 2
};

typedef struct command_party_invite
{
	uint8_t header;
	uint32_t vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer
{
	uint8_t header;
	uint32_t leader_vid;
	uint8_t accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove
{
	uint8_t header;
	uint32_t pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
	uint8_t header;
	uint32_t pid;
	uint8_t byRole;
	uint8_t flag;
} TPacketCGPartySetState;

typedef struct command_party_use_skill
{
	uint8_t header;
	uint8_t bySkillIndex;
	uint32_t vid;
} TPacketCGPartyUseSkill;

typedef struct command_party_parameter
{
	uint8_t bHeader;
	uint8_t bDistributeMode;
} TPacketCGPartyParameter;

typedef struct packet_party_link
{
	uint8_t header;
	uint32_t pid;
	uint32_t vid;
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	uint32_t mapIdx;
	uint8_t channel;
#endif
} TPacketGCPartyLink;

typedef struct packet_party_invite
{
	uint8_t header;
	uint32_t leader_vid;
} TPacketGCPartyInvite;

typedef struct packet_party_add
{
	uint8_t header;
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	uint32_t mapIdx;
	uint8_t channel;
#endif
} TPacketGCPartyAdd;

typedef struct packet_party_update
{
	uint8_t header;
	uint32_t pid;
	uint8_t role;
	uint8_t percent_hp;
	int16_t affects[7];
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
	uint8_t header;
	uint32_t pid;
} TPacketGCPartyRemove;

typedef struct party_parameter
{
	uint8_t bHeader;
	uint8_t bDistributeMode;
} TPacketGCPartyParameter;

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
struct TPartyPosition
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

//////
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

typedef struct SPacketCGMyShop
{
	uint8_t bHeader;
	char szSign[SHOP_SIGN_MAX_LEN + 1];
	uint8_t bCount; // count of TShopItemTable, max 39
} TPacketCGMyShop;

typedef struct SPacketCGRefine
{
	uint8_t header;
	uint16_t wPos; //@fixme519
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

typedef struct command_quest_input_string
{
	uint8_t header;
	char msg[64 + 1];
} TPacketCGQuestInputString;

#ifdef ENABLE_OX_RENEWAL
typedef struct command_quest_input_long_string
{
	uint8_t header;
	char msg[128 + 1];
} TPacketCGQuestInputLongString;
#endif

typedef struct command_quest_confirm
{
	uint8_t header;
	uint8_t answer;
	uint32_t requestPID;
} TPacketCGQuestConfirm;

// SCRIPT_SELECT_ITEM
typedef struct command_script_select_item
{
	uint8_t header;
	uint32_t selection;
} TPacketCGScriptSelectItem;
// END_OF_SCRIPT_SELECT_ITEM

enum EPhase
{
	PHASE_CLOSE, // Hanging up (or before hanging up)
	PHASE_HANDSHAKE, // handshake..;;
	PHASE_LOGIN, // Logging in
	PHASE_SELECT, // character selection screen
	PHASE_LOADING, // Loading screen after selection
	PHASE_GAME, // game screen
	PHASE_DEAD, // When dead... (maybe in-game...)

	PHASE_CLIENT_CONNECTING, // for server
	PHASE_DBCLIENT, // for server
	PHASE_P2P, // for server
	PHASE_AUTH, // for login authentication
};

typedef struct packet_phase
{
	uint8_t header;
	uint8_t phase;
} TPacketGCPhase;


/* Login (1) */
typedef struct command_handshake
{
	uint8_t bHeader;
	uint32_t dwHandshake;
	uint32_t dwTime;
	long lDelta;
} TPacketCGHandshake;

typedef struct packet_handshake
{
	uint8_t bHeader;
	uint32_t dwHandshake;
	uint32_t dwTime;
	long lDelta;
} TPacketGCHandshake;

typedef struct packet_bindudp
{
	uint8_t header;
	uint32_t addr;
	uint16_t port;
} TPacketGCBindUDP;

enum
{
	LOGIN_FAILURE_ALREADY = 1,
	LOGIN_FAILURE_ID_NOT_EXIST = 2,
	LOGIN_FAILURE_WRONG_PASS = 3,
	LOGIN_FAILURE_FALSE = 4,
	LOGIN_FAILURE_NOT_TESTOR = 5,
	LOGIN_FAILURE_NOT_TEST_TIME = 6,
	LOGIN_FAILURE_FULL = 7
};

typedef struct packet_login_success
{
	uint8_t bHeader;
	TSimplePlayer players[PLAYER_PER_ACCOUNT];
	uint32_t guild_id[PLAYER_PER_ACCOUNT];
	char guild_name[PLAYER_PER_ACCOUNT][GUILD_NAME_MAX_LEN + 1];
	uint32_t handle;
	uint32_t random_key;
} TPacketGCLoginSuccess;

typedef struct packet_login_failure
{
	uint8_t header;
	char szStatus[ACCOUNT_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct command_player_create
{
	uint8_t header;
	uint8_t index;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint16_t job;
	uint8_t shape;
	uint8_t Con;
	uint8_t Int;
	uint8_t Str;
	uint8_t Dex;
} TPacketCGPlayerCreate;

typedef struct command_player_create_success
{
	uint8_t header;
	uint8_t bAccountCharacterIndex;
	TSimplePlayer player;
} TPacketGCPlayerCreateSuccess;

typedef struct packet_create_failure
{
	uint8_t header;
	uint8_t bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete
{
	uint8_t header;
	uint8_t index;
	char private_code[8];
} TPacketCGPlayerDelete;

enum
{
	ADD_CHARACTER_STATE_DEAD = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY = (1 << 4)
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

typedef struct packet_char_additional_info
{
	uint8_t header;
	uint32_t dwVID;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t adwPart[CHR_EQUIPPART_NUM]; //@fixme479 - uint16_t -> uint32_t
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t dwElementsEffect;
#endif
	uint8_t bEmpire;
	uint32_t dwGuildID;
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t dwNewIsGuildName;
#endif
	uint32_t dwLevel;
	int16_t sAlignment; //good and bad
	uint8_t bPKMode;
	uint32_t dwMountVnum;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	uint32_t dwTitle;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	uint32_t dwArrow;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	long lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t dwcLevel;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
} TPacketGCCharacterAdditionalInfo;

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
	uint32_t dwAffectFlag[2]; // effect
#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	int iPortalMinLevelLimit;
	int iPortalMaxLevelLimit;
#endif
} TPacketGCCharacterAdd;

typedef struct packet_update_char
{
	uint8_t header;
	uint32_t dwVID;

	uint32_t adwPart[CHR_EQUIPPART_NUM]; //@fixme479 - uint16_t -> uint32_t
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
#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	long lGroupWeapon;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
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
	uint32_t id;
} TPacketGCCharacterDelete;

typedef struct packet_chat // variable packets
{
	uint8_t header;
	uint16_t size;
	uint8_t type;
	uint32_t id;
	uint8_t bEmpire;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	bool bCanFormat;
	packet_chat() : bCanFormat(true) {}
	uint8_t bLanguage;
#endif
#ifdef ENABLE_EMPIRE_FLAG
	bool bFlag;
#endif
} TPacketGCChat;

typedef struct packet_whisper // variable packets
{
	uint8_t bHeader;
	uint16_t wSize;
	uint8_t bType;
	char szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	bool bCanFormat;
	packet_whisper() : bCanFormat(true) {}
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
#if defined (ENABLE_BATTLE_FIELD) && defined(ENABLE_BATTLE_FIELD)
	bool bRestart;
	long lMapIdx;
#endif
} TPacketGCDead;

// SUPPORT_BGM
typedef struct packet_main_character
{
	uint8_t header;
	uint32_t dwVID;
	uint16_t wRaceNum;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	long lx, ly, lz;
	uint8_t empire;
	uint8_t skill_group;
} TPacketGCMainCharacter;

typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_LEN = 24
	};

	uint8_t header;
	uint32_t dwVID;
	uint16_t wRaceNum;
	char szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char szBGMName[MUSIC_NAME_LEN + 1];
	long lx, ly, lz;
	uint8_t empire;
	uint8_t skill_group;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_LEN = 24
	};

	uint8_t header;
	uint32_t dwVID;
	uint16_t wRaceNum;
	char szChrName[CHARACTER_NAME_MAX_LEN + 1];
	char szBGMName[MUSIC_NAME_LEN + 1];
	float fBGMVol;
	long lx, ly, lz;
	uint8_t empire;
	uint8_t skill_group;
} TPacketGCMainCharacter4_BGM_VOL;
// END_OF_SUPPORT_BGM

typedef struct packet_points
{
	uint8_t header;
	long points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_point_change
{
	int header;
	uint32_t dwVID;
	uint16_t type; //@fixme532
	long amount; // changed value
	long value; // current value
} TPacketGCPointChange;

typedef struct packet_motion
{
	uint8_t header;
	uint32_t vid;
	uint32_t victim_vid;
	uint16_t motion;
} TPacketGCMotion;

typedef struct SPacketGCItemDelDeprecated
{
	uint8_t header;
	TItemPos Cell;
	uint32_t vnum;
	uint8_t count;
#ifdef ENABLE_SEALBIND_SYSTEM
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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TPacketGCItemDelDeprecated;

typedef struct packet_item_set
{
	uint8_t header;
	TItemPos Cell;
	uint32_t vnum;
	uint8_t count;
	uint32_t flags; // add flag
	uint32_t anti_flags; // add flag
	bool highlight;
#ifdef ENABLE_SEALBIND_SYSTEM
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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
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

typedef struct packet_item_use
{
	uint8_t header;
	TItemPos Cell;
	uint32_t ch_vid;
	uint32_t victim_vid;
	uint32_t vnum;
} TPacketGCItemUse;

typedef struct packet_item_update
{
	uint8_t header;
	TItemPos Cell;
	uint8_t count;
#ifdef ENABLE_SEALBIND_SYSTEM
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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TPacketGCItemUpdate;

typedef struct packet_item_ground_add
{
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	packet_item_ground_add()
	{
		memset(&alSockets, 0, sizeof(alSockets));
		memset(&aAttrs, 0, sizeof(aAttrs));
	}
#endif

	uint8_t bHeader;
	long x;
	long y;
	long z;

	uint32_t dwVID;
	uint32_t dwVnum;
#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttrs[ITEM_ATTRIBUTE_MAX_NUM];
#endif
} TPacketGCItemGroundAdd;

typedef struct packet_item_ground_del
{
	uint8_t bHeader;
	uint32_t dwVID;
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
	uint8_t pos_to;
} TPacketGCQuickSlotSwap;

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
//#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	SHOP_SUBHEADER_GC_UNK_21, // RefreshShopItemToolTip
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

struct packet_shop_item
{
	uint32_t vnum;
#ifdef ENABLE_CHEQUE_SYSTEM
	TItemPriceType price;
#else
	uint32_t price;
#endif
	uint8_t count;
#ifdef ENABLE_MEDAL_OF_HONOR
	uint8_t limit_level;
#endif
	uint8_t display_pos;
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_12ZI
	uint32_t getLimitedCount;
	uint32_t getLimitedPurchaseCount;
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
};

typedef struct packet_shop // variable packets
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
} TPacketGCShop;

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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
# ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
# endif
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
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
} TPacketPlayerShopSign;
#endif

typedef struct packet_shop_start
{
	uint32_t owner_vid;
#ifdef ENABLE_MYSHOP_DECO
	uint8_t shop_tab_count;
	struct packet_shop_item items[SHOP_HOST_ITEM_MAX];
#else
	struct packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
#endif
#ifdef ENABLE_12ZI
	bool isLimitedItemShop;
#endif
} TPacketGCShopStart;

typedef struct packet_shop_start_ex // This is followed by TSubPacketShopTab* shop_tabs.
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

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
typedef struct packet_shop_update_limited_purchase
{
	uint32_t vnum;
	uint32_t limited_count;
	uint32_t limited_purchase_count;
} TPacketGCShopUpdateLimitedPurchase;
#endif

typedef struct packet_exchange
{
	uint8_t header;
	uint8_t sub_header;
	uint8_t is_me;
	uint32_t arg1; // vnum
	TItemPos arg2; // cell
	uint32_t arg3; // count
#ifdef WJ_ENABLE_TRADABLE_ICON
	TItemPos arg4; // srccell
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t cheque;
#endif
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
#ifdef ENABLE_GROWTH_PET_SYSTEM
	TGrowthPetInfo aPetInfo{ 0 };
#endif
} TPacketGCExchange;

enum EPacketTradeSubHeaders
{
	EXCHANGE_SUBHEADER_GC_START, /* arg1 == vid */
	EXCHANGE_SUBHEADER_GC_ITEM_ADD, /* arg1 == vnum arg2 == pos arg3 == count */
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,
	EXCHANGE_SUBHEADER_GC_GOLD_ADD, /* arg1 == gold */
	EXCHANGE_SUBHEADER_GC_ACCEPT, /* arg1 == accept */
	EXCHANGE_SUBHEADER_GC_END, /* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_ALREADY, /* arg1 == not used */
	EXCHANGE_SUBHEADER_GC_LESS_GOLD, /* arg1 == not used */
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

typedef struct packet_script
{
	uint8_t header;
	uint16_t size;
	uint8_t skin;
	uint16_t src_size;
} TPacketGCScript;

#ifdef ENABLE_TARGET_AFFECT_SHOWER
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
	char cElementalFlags[MOB_ELEMENTAL_MAX_NUM];
#endif
} TPacketGCTarget;

typedef struct packet_damage_info
{
	uint8_t header;
	uint32_t dwVID;
	uint8_t flag;
	int damage;
} TPacketGCDamageInfo;

enum EMoveFuncType
{
	FUNC_WAIT,
	FUNC_MOVE,
	FUNC_ATTACK,
	FUNC_COMBO,
	FUNC_MOB_SKILL,
	_FUNC_SKILL,
	FUNC_MAX_NUM,
	FUNC_SKILL = 0x80,
};

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

typedef struct packet_move
{
	uint8_t bHeader;
	uint8_t bFunc;
	uint8_t bArg;
	uint8_t bRot;
	uint32_t dwVID;
	long lX;
	long lY;
	uint32_t dwTime;
	uint32_t dwDuration;
} TPacketGCMove;

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

/*
* packets sent from server side
*/
typedef struct packet_quest_confirm
{
	uint8_t header;
	char msg[64 + 1];
	long timeout;
	uint32_t requestPID;
} TPacketGCQuestConfirm;

// Unit attached as much as bCount of Position Synchronization Packet
typedef struct packet_sync_position_element
{
	uint32_t dwVID;
	long lX;
	long lY;
} TPacketGCSyncPositionElement;

// location sync
typedef struct packet_sync_position // variable packets
{
	uint8_t bHeader;
	uint16_t wSize; // Count = (wSize - sizeof(TPacketGCSyncPosition)) / sizeof(TPacketGCSyncPositionElement)
} TPacketGCSyncPosition;

// ownership
typedef struct packet_ownership
{
	uint8_t bHeader;
	uint32_t dwOwnerVID;
	uint32_t dwVictimVID;
} TPacketGCOwnership;

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
	uint16_t wSize; // How many uint32_ts? Count = (wSize - sizeof(TPacketGCPVPList)) / 4
} TPacketGCDuelStart;

typedef struct packet_pvp
{
	uint8_t bHeader;
	uint32_t dwVIDSrc;
	uint32_t dwVIDDst;
	uint8_t bMode; // 0 is off, 1 is on
} TPacketGCPVP;

typedef struct packet_warp
{
	uint8_t bHeader;
	long lX;
	long lY;
	long lAddr;
	uint16_t wPort;
} TPacketGCWarp;

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

typedef struct command_fishing
{
	uint8_t header;
	uint8_t dir;
} TPacketCGFishing;

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

typedef struct packet_dungeon_dest_position
{
	long x;
	long y;
} TPacketGCDungeonDestPosition;

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

enum class WalkMode
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

typedef struct SPacketGCRefineInformaion
{
	uint8_t header;
	uint8_t type;
	uint16_t wPos; //@fixme519
	uint32_t src_vnum;
	uint32_t result_vnum;
	uint8_t material_count;
	int cost; // cost
	int prob; // percentage
	TRefineMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TPacketGCRefineInformation;

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

	// array of TNPCPosition
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
typedef struct SPacketGCNPCPosition
{
	uint8_t header;
	uint16_t size;
	uint16_t count;

	// array of TNPCPosition
} TPacketGCNPCPosition;

struct TNPCPosition
{
	uint8_t bType;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	long x;
	long y;
};
#endif

typedef struct SPacketGCChangeName
{
	uint8_t header;
	uint32_t pid;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCChangeName;

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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t dwTransmutationVnum;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TEquipmentItemSet;

typedef struct packet_view_equip // pakcet_view_equip
{
	uint8_t header;
	uint32_t vid;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketViewEquip;

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

typedef struct
{
	uint8_t bHeader;
	long lID;
	char szName[32 + 1];
	uint32_t dwVID;
	uint8_t bType;
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

typedef struct SPacketGCAffectElement
{
	uint32_t dwType;
	uint16_t wApplyOn; //@fixme532
	long lApplyValue;
	uint32_t dwFlag;
	long lDuration;
	long lSPCost;
} TPacketGCAffectElement;

typedef struct SPacketGCAffectAdd
{
	uint8_t bHeader;
	TPacketGCAffectElement elem;
} TPacketGCAffectAdd;

typedef struct SPacketGCAffectRemove
{
	uint8_t bHeader;
	uint32_t dwType;
	uint16_t wApplyOn; //@fixme532
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
	uint8_t header;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t love_point;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	uint8_t header;
	uint8_t love_point;
} TPacketGCLovePointUpdate;

// MINING
typedef struct packet_dig_motion
{
	uint8_t header;
	uint32_t vid;
	uint32_t target_vid;
	uint8_t count;
} TPacketGCDigMotion;
// END_OF_MINING

typedef struct SPacketGCPanamaPack
{
	uint8_t bHeader;
	char szPackName[256];
	uint8_t abIV[32];
} TPacketGCPanamaPack;

//TODO: Oh my gosh.. Let's refactor the packet handler to accept variable packet size.
typedef struct SPacketGCHybridCryptKeys
{
	SPacketGCHybridCryptKeys() : m_pStream(nullptr) {}
	~SPacketGCHybridCryptKeys()
	{
		//Is it okay to delete nullptr in GCC? Let's safely check nullptr first. (But wasn't this the C++ standard --a )
		if (m_pStream)
		{
			delete[] m_pStream;
			m_pStream = nullptr;
		}
	}

	uint32_t GetStreamSize()
	{
		return sizeof(bHeader) + sizeof(uint16_t) + sizeof(int) + KeyStreamLen;
	}

	uint8_t* GetStreamData()
	{
		if (m_pStream)
			delete[] m_pStream;

		uDynamicPacketSize = (uint16_t)GetStreamSize();

		m_pStream = M2_NEW uint8_t[uDynamicPacketSize];

		memcpy(m_pStream, &bHeader, 1);
		memcpy(m_pStream + 1, &uDynamicPacketSize, 2);
		memcpy(m_pStream + 3, &KeyStreamLen, 4);

		if (KeyStreamLen > 0)
			memcpy(m_pStream + 7, pDataKeyStream, KeyStreamLen);

		return m_pStream;
	}

	uint8_t bHeader;
	uint16_t uDynamicPacketSize; // You have to match it because of the damn class DynamicPacketHeader structure -_-;
	int KeyStreamLen;
	uint8_t* pDataKeyStream;

private:
	uint8_t* m_pStream;
} TPacketGCHybridCryptKeys;


typedef struct SPacketGCPackageSDB
{
	SPacketGCPackageSDB() : m_pDataSDBStream(nullptr), m_pStream(nullptr) {}
	~SPacketGCPackageSDB()
	{
		if (m_pStream)
		{
			delete[] m_pStream;
			m_pStream = nullptr;
		}
	}

	uint32_t GetStreamSize()
	{
		return sizeof(bHeader) + sizeof(uint16_t) + sizeof(int) + iStreamLen;
	}

	uint8_t* GetStreamData()
	{
		if (m_pStream)
			delete[] m_pStream;

		uDynamicPacketSize = GetStreamSize();

		m_pStream = M2_NEW uint8_t[uDynamicPacketSize];

		memcpy(m_pStream, &bHeader, 1);
		memcpy(m_pStream + 1, &uDynamicPacketSize, 2);
		memcpy(m_pStream + 3, &iStreamLen, 4);

		if (iStreamLen > 0)
			memcpy(m_pStream + 7, m_pDataSDBStream, iStreamLen);

		return m_pStream;
	}

	uint8_t bHeader;
	uint16_t uDynamicPacketSize; // You have to match it because of the damn class DynamicPacketHeader structure -_-;
	int iStreamLen;
	uint8_t* m_pDataSDBStream;

private:
	uint8_t* m_pStream;
} TPacketGCPackageSDB;

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

#define MAX_EFFECT_FILE_NAME 128
typedef struct SPacketGCSpecificEffect
{
	uint8_t header;
	uint32_t vid;
	char effect_file[MAX_EFFECT_FILE_NAME];
} TPacketGCSpecificEffect;

// Dragon Soul Stone
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
	DS_SUB_HEADER_DO_REFINE_GRADE,
	DS_SUB_HEADER_DO_REFINE_STEP,
	DS_SUB_HEADER_DO_REFINE_STRENGTH,
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
	TItemPos ItemGrid[DRAGON_SOUL_REFINE_GRID_SIZE];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() = default;
	uint8_t header{ HEADER_GC_DRAGON_SOUL_REFINE };
	uint8_t bSubType{};
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

#ifdef ENABLE_SEALBIND_SYSTEM
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

#ifdef ENABLE_MOVE_COSTUME_ATTR
typedef struct SPacketCGSendCostumeRefine
{
	uint8_t bHeader;
	uint8_t bSlotMedium;
	uint8_t bSlotBase;
	uint8_t bSlotMaterial;
} TPacketCGSendCostumeRefine;
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
	uint8_t header;
	uint8_t subheader;
	uint8_t slot_type;
	TItemPos pos;
} TPacketCGTransmutation;
#endif

#ifdef NEW_SELECT_CHARACTER
typedef struct SPacketCharacterInformation
{
	uint8_t bHeader;
	TSimplePlayer players[PLAYER_PER_ACCOUNT];
} TPacketCharacterInformation;
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
typedef struct SPacketCGSendExtendInvenRequest
{
	uint8_t bHeader;
	uint8_t bStepIndex;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bWindow;
	bool bSpecialState;
# endif
} TPacketCGSendExtendInvenRequest;

typedef struct SPacketCGSendExtendInvenUpgrade
{
	uint8_t bHeader;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bWindow;
	bool bSpecialState;
# endif
} TPacketCGSendExtendInvenUpgrade;

typedef struct SPacketGCExtendInvenInfo
{
	uint8_t bHeader;
	uint8_t bExtendStage;
	uint8_t bExtendMax;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bExtendSpecialStage[3];
	uint16_t bExtendSpecialMax[3];
# endif
} TPacketGCExtendInvenInfo;

typedef struct SPacketGCExtendInvenResult
{
	uint8_t bHeader;

	uint32_t dwVnum;
	uint8_t bMsgResult;
	uint8_t bEnoughCount;
} TPacketGCExtendInvenResult;
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
	char sGrowthPetName[PET_NAME_MAX_SIZE + 1];
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
	char sPetName[PET_NAME_MAX_SIZE + 1];
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

#ifdef ENABLE_SWITCHBOT
struct TPacketGGSwitchbot
{
	uint8_t bHeader;
	uint16_t wPort;
	TSwitchbotTable table;

	TPacketGGSwitchbot() : bHeader(HEADER_GG_SWITCHBOT), wPort(0)
	{
		table = {};
	}
};

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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
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
	uint8_t bCategory;
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

#ifdef ENABLE_CUBE_RENEWAL
enum EPacketCGCubeRenewalSubHeader
{
	CUBE_RENEWAL_MAKE,
	CUBE_RENEWAL_CLOSE_STATE,
};

/*Client -> Game*/
typedef struct SPacketCGCubeRenewal
{
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
#endif

//ENABLE_AURA_SYSTEM
typedef struct SItemData
{
	uint32_t vnum;
	uint8_t count;
	uint32_t flags;
	uint32_t anti_flags;
#ifdef ENABLE_SEALBIND_SYSTEM
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
	long alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif
} TItemData;

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

#ifdef ENABLE_ATTR_6TH_7TH
enum
{
	ATTR67_SUB_HEADER_CLOSE,
	ATTR67_SUB_HEADER_OPEN,
	ATTR67_SUB_HEADER_SKILLBOOK_COMB,

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
/*******************************************************************\
| [GAME to CLIENT] Packets
\*******************************************************************/
typedef struct SPacketGCMyPrivShopOpen
{
	uint8_t bHeader;
	bool bCashItem;
	uint8_t bTabCount;
} TPacketGCMyPrivShopOpen;

/*******************************************************************\
| [CLIENT to GAME] Packets
\*******************************************************************/
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

/*typedef struct command_gem_shop_open
{
	uint8_t header;
	int nextRefreshTime;
	TGemShopItem shopItems[GEM_SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCGemShopOpen;

typedef struct command_gem_shop_refresh
{
	uint8_t header;
	int nextRefreshTime;
	TGemShopItem shopItems[GEM_SHOP_HOST_ITEM_MAX_NUM];
} TPacketGCGemShopRefresh;*/

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

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
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
	char itemName[ITEM_NAME_MAX_LEN + 1];
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

#ifdef ENABLE_CHANGED_ATTR
typedef struct packet_gc_item_select_attr
{
	uint8_t bHeader;
	TItemPos pItemPos;
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_MAX_NUM];
} TPacketGCItemSelectAttr;

typedef struct packet_cg_item_select_attr
{
	uint8_t bHeader;
	bool bNew;
	TItemPos pItemPos;
} TPacketCGItemSelectAttr;
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
	uint8_t bHeader;
	uint8_t bSubHeader;
	uint8_t bArg1;
	uint8_t bArg2;
} TPacketMailboxProcess;

typedef struct packet_mailbox_process_all
{
	uint8_t Index;
} TPacketGCMailboxProcessAll;

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
	TWikiInfoTable wiki_info;
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
	Item sRequiredItem[EDungeonInfoData::MAX_REQUIRED_ITEMS];
	uint32_t dwDuration;
	uint32_t dwCooldown;
	uint8_t byElement;
	Bonus sBonus;
	Item sBossDropItem[EDungeonInfoData::MAX_BOSS_ITEM_SLOTS];
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
	uint16_t wApplyType[MAX_BONUSES_LENGTH]; //@fixme532
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

#ifdef ENABLE_FISHING_RENEWAL
enum
{
	FISHING_SUBHEADER_NEW_START,
	FISHING_SUBHEADER_NEW_STOP,
	FISHING_SUBHEADER_NEW_CATCH,
	FISHING_SUBHEADER_NEW_CATCH_FAIL,
	FISHING_SUBHEADER_NEW_CATCH_SUCCESS,
	FISHING_SUBHEADER_NEW_CATCH_FAILED,
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
typedef struct SCatchKingCard
{
	SCatchKingCard()
	{
		bIndex = 0;
		bIsExposed = false;
	}

	SCatchKingCard(uint8_t index, bool isExposed)
	{
		bIndex = index;
		bIsExposed = isExposed;
	}

	uint8_t bIndex;
	bool bIsExposed;
} TCatchKingCard;

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
enum
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

#ifdef ENABLE_FLOWER_EVENT
typedef struct packet_send_flower_event
{
	uint8_t header;
	uint8_t id;
} TPacketCGFlowerEventSend;
#endif
// END MINI GAME EVENTS //

#pragma pack()
#endif
