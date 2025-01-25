#pragma once

#include "../GameLib/RaceData.h"
#include "../GameLib/ActorInstance.h"
#include "../GameLib/GameLibDefines.h"

#include "AffectFlagContainer.h"

class CInstanceBase
{
public:
	struct SCreateData
	{
		uint8_t m_bType;
		uint32_t m_dwStateFlags;
		uint32_t m_dwEmpireID;
		uint32_t m_dwGuildID;
		uint32_t m_dwLevel;
#ifdef ENABLE_YOHARA_SYSTEM
		uint32_t m_dwcLevel;
#endif
#ifdef WJ_SHOW_MOB_INFO
		uint32_t m_dwAIFlag;
#endif
		uint32_t m_dwVID;
		uint32_t m_dwRace;
		uint32_t m_dwMovSpd;
		uint32_t m_dwAtkSpd;
		LONG m_lPosX;
		LONG m_lPosY;
		FLOAT m_fRot;
		uint32_t m_dwArmor;
		uint32_t m_dwWeapon;
		uint32_t m_dwHair;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		uint32_t	m_dwAcce;
#endif
#ifdef ENABLE_PENDANT
		uint32_t m_dwPendant;
#endif
#ifdef ENABLE_REFINE_ELEMENT
		uint8_t 	m_dwElementsEffect;
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		long	m_lGroupWeapon;
#endif
		uint32_t m_dwMountVnum;
#ifdef ENABLE_AURA_SYSTEM
		uint32_t	m_dwAura;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		uint32_t	m_dwArrow;
#endif
#ifdef ENABLE_SHOW_GUILD_LEADER
		uint8_t	m_dwNewIsGuildName;
#endif
		int16_t m_sAlignment;
		uint8_t m_byPKMode;
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		uint32_t	m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif
		CAffectFlagContainer m_kAffectFlags;

		std::string m_stName;
#ifdef ENABLE_RENDER_TARGET_EFFECT
		bool m_isRenderTarget = false;
#endif
		bool m_isMain;
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		uint32_t m_dwTitle;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		uint8_t m_bLanguage;
#endif
	};

public:
	using TType = uint32_t;

	enum EDirection
	{
		DIR_NORTH,
		DIR_NORTHEAST,
		DIR_EAST,
		DIR_SOUTHEAST,
		DIR_SOUTH,
		DIR_SOUTHWEST,
		DIR_WEST,
		DIR_NORTHWEST,
		DIR_MAX_NUM
	};

	enum
	{
		FUNC_WAIT,
		FUNC_MOVE,
		FUNC_ATTACK,
		FUNC_COMBO,
		FUNC_MOB_SKILL,
		FUNC_EMOTION,
		FUNC_SKILL = 0x80
	};

	enum
	{
		AFFECT_YMIR,						// 0
		AFFECT_INVISIBILITY,				// 1
		AFFECT_SPAWN,						// 2
		AFFECT_POISON,						// 3
		AFFECT_SLOW,						// 4
		AFFECT_STUN,						// 5
		AFFECT_DUNGEON_READY,				// 6
		AFFECT_SHOW_ALWAYS,					// 7
		AFFECT_BUILDING_CONSTRUCTION_SMALL,	// 8
		AFFECT_BUILDING_CONSTRUCTION_LARGE,	// 9
		AFFECT_BUILDING_UPGRADE,			// 10
		AFFECT_MOV_SPEED_POTION,			// 11
		AFFECT_ATT_SPEED_POTION,			// 12
		AFFECT_FISH_MIND,					// 13
		AFFECT_JEONGWI,						// 14
		AFFECT_GEOMGYEONG,					// 15
		AFFECT_CHEONGEUN,					// 16
		AFFECT_GYEONGGONG,					// 17
		AFFECT_EUNHYEONG,					// 18
		AFFECT_GWIGEOM,						// 19
		AFFECT_GONGPO,						// 20
		AFFECT_JUMAGAP,						// 21
		AFFECT_HOSIN,						// 22
		AFFECT_BOHO,						// 23
		AFFECT_KWAESOK,						// 24
		AFFECT_HEUKSIN,						// 25
		AFFECT_MUYEONG,						// 26
		AFFECT_REVIVE_INVISIBILITY,			// 27
		AFFECT_FIRE,						// 28
		AFFECT_GICHEON,						// 29
		AFFECT_JEUNGRYEOK,					// 30
		AFFECT_DASH,						// 31
		AFFECT_PABEOP,						// 32
		AFFECT_FALLEN_CHEONGEUN,			// 33
		AFFECT_POLYMORPH,					// 34
		AFFECT_WAR_FLAG1,					// 35
		AFFECT_WAR_FLAG2,					// 36
		AFFECT_WAR_FLAG3,					// 37
		AFFECT_CHINA_FIREWORK,				// 38
		AFFECT_PREMIUM_SILVER,				// 39
		AFFECT_PREMIUM_GOLD,				// 40
		AFFECT_RAMADAN_RING,				// 41
#ifdef ENABLE_WOLFMAN_CHARACTER
		AFFECT_BLEEDING,					// 42
		AFFECT_RED_POSSESSION,				// 43
		AFFECT_BLUE_POSSESSION,				// 44
#endif
		AFFECT_DS,							// 45
#ifdef ENABLE_VIP_SYSTEM
		AFFECT_VIP,							// 46
#endif
#ifdef ENABLE_MONARCH_AFFECT
		AFFECT_KING,						// 47
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		AFFECT_DRAGONLAIR_STONE_UNBEATABLE_1,// 48
		AFFECT_DRAGONLAIR_STONE_UNBEATABLE_2,// 49
		AFFECT_DRAGONLAIR_STONE_UNBEATABLE_3,// 50
#endif
#ifdef ENABLE_SOUL_SYSTEM
		AFFECT_SOUL_RED,					// 51
		AFFECT_SOUL_BLUE,					// 52
		AFFECT_SOUL_MIX,					// 53
#endif
#ifdef ENABLE_12ZI
		AFFECT_CZ_UNLIMIT_ENTER,			// 54
#endif
#ifdef ENABLE_AFK_MODE_SYSTEM
		AFFECT_AFK,							// 55
#endif
#ifdef ENABLE_AFTERDEATH_SHIELD
		AFF_AFTERDEATH_SHIELD,				// 56
#endif
#ifdef ENABLE_BATTLE_FIELD
		AFFECT_BATTLE_RANKER_1,				// 57
		AFFECT_BATTLE_RANKER_2,				// 58
		AFFECT_BATTLE_RANKER_3,				// 59
		AFFECT_TARGET_VICTIM,				// 60
#endif
#ifdef ENABLE_NINETH_SKILL
		AFFECT_CHEONUN,						// 61
		AFFECT_CHUNWOON,					// 62
#endif
#ifdef ENABLE_AUTO_SYSTEM
		AFFECT_AUTO_USE,					// 63
#endif

		AFFECT_NUM = 64,

		AFFECT_HWAYEOM = AFFECT_GEOMGYEONG
	};

	enum
	{
		NEW_AFFECT_MOV_SPEED = 200,
		NEW_AFFECT_ATT_SPEED,
		NEW_AFFECT_ATT_GRADE,
		NEW_AFFECT_INVISIBILITY,
		NEW_AFFECT_STR,
		NEW_AFFECT_DEX,                 // 205
		NEW_AFFECT_CON,
		NEW_AFFECT_INT,
		NEW_AFFECT_FISH_MIND_PILL,
		NEW_AFFECT_POISON,
		NEW_AFFECT_STUN,                // 210
		NEW_AFFECT_SLOW,
		NEW_AFFECT_DUNGEON_READY,
		NEW_AFFECT_DUNGEON_UNIQUE,
		NEW_AFFECT_BUILDING,
		NEW_AFFECT_REVIVE_INVISIBLE,    // 215
		NEW_AFFECT_FIRE,
		NEW_AFFECT_CAST_SPEED,
		NEW_AFFECT_HP_RECOVER_CONTINUE,
		NEW_AFFECT_SP_RECOVER_CONTINUE,
		NEW_AFFECT_POLYMORPH,           // 220
		NEW_AFFECT_MOUNT,
		NEW_AFFECT_WAR_FLAG,            // 222
		NEW_AFFECT_BLOCK_CHAT,          // 223
		NEW_AFFECT_CHINA_FIREWORK,
		NEW_AFFECT_BOW_DISTANCE,        // 225

		NEW_AFFECT_RAMADAN_ABILITY = 300,
		NEW_AFFECT_RAMADAN_RING = 301,
		NEW_AFFECT_NOG_POCKET_ABILITY = 302,
		AFFECT_POTION_1 = 303,
		AFFECT_POTION_2 = 304,
		AFFECT_POTION_3 = 305,
		AFFECT_POTION_4 = 306,
		AFFECT_POTION_5 = 307,
		AFFECT_POTION_6 = 308,

		NEW_AFFECT_EXP_BONUS = 500,
		NEW_AFFECT_ITEM_BONUS = 501,
		NEW_AFFECT_SAFEBOX = 502,
		NEW_AFFECT_AUTOLOOT = 503,
		NEW_AFFECT_FISH_MIND = 504,
		NEW_AFFECT_MARRIAGE_FAST = 505,
		NEW_AFFECT_GOLD_BONUS = 506,
		//#ifdef ENABLE_AUTO_SYSTEM
		NEW_AFFECT_AUTO_USE = 507,
		//#endif
		//#ifdef ENABLE_YOHARA_SYSTEM
		NEW_AFFECT_SUNGMA_BONUS = 508,
		//#endif

		NEW_AFFECT_MALL = 510,
		NEW_AFFECT_NO_DEATH_PENALTY = 511,
		NEW_AFFECT_SKILL_BOOK_BONUS = 512,
		NEW_AFFECT_SKILL_BOOK_NO_DELAY = 513,

		NEW_AFFECT_EXP_BONUS_EURO_FREE = 516,
		NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 = 517,

		NEW_AFFECT_AUTO_HP_RECOVERY = 534,
		NEW_AFFECT_AUTO_SP_RECOVERY = 535,
		//#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		NEW_AFFECT_PREMIUM_PRIVATE_SHOP = 536,
		//#endif

		NEW_AFFECT_DRAGON_SOUL_QUALIFIED = 540,
		NEW_AFFECT_DRAGON_SOUL_DECK1 = 541,
		NEW_AFFECT_DRAGON_SOUL_DECK2 = 542,

#ifdef ENABLE_SET_ITEM
		NEW_AFFECT_SET_ITEM = 550,
#endif

#ifdef ENABLE_AFK_MODE_SYSTEM
		NEW_AFFECT_AFK = 555,
#endif
#ifdef ENABLE_BATTLE_FIELD
		NEW_AFFECT_BATTLE_FIELD = 556,
		NEW_AFFECT_BATTLE_POTION = 557,
#endif

#ifdef ENABLE_DS_SET
		NEW_AFFECT_DS_SET = 571,
#endif

#ifdef ENABLE_FLOWER_EVENT
		AFFECT_FLOWER_EVENT = 576,
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		NEW_AFFECT_LATE_SUMMER_EVENT_BUFF = 583,
		NEW_AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF = 584,
#endif

#ifdef ENABLE_PASSIVE_SYSTEM
		NEW_AFFECT_PASSIVE_JOB_DECK = 593,
		//reserviert 594
#endif

#ifdef ENABLE_ELEMENTAL_WORLD
		NEW_AFFECT_PROTECTION_OF_ELEMENTAL = 595,
#endif

#ifdef ENABLE_12ZI
		NEW_AFFECT_CZ_UNLIMIT_ENTER = 600,
#endif

#ifdef ENABLE_SET_ITEM
		NEW_AFFECT_SET_ITEM_SET_VALUE_1 = 601,
		NEW_AFFECT_SET_ITEM_SET_VALUE_2 = 602,
		NEW_AFFECT_SET_ITEM_SET_VALUE_3 = 603,
		NEW_AFFECT_SET_ITEM_SET_VALUE_4 = 604,
		NEW_AFFECT_SET_ITEM_SET_VALUE_5 = 605,
#endif

#ifdef ENABLE_MONSTER_BACK
		NEW_AFFECT_EXP_BONUS_EVENT = 610,
		NEW_AFFECT_ATT_SPEED_SLOW = 611,
#endif

#ifdef ENABLE_YOHARA_SYSTEM
		NEW_AFFECT_SUNGMA_HP = 624,
		NEW_AFFECT_SUNGMA_IMMUNE = 626,
		NEW_AFFECT_SUNGMA_MOVE = 625,
		NEW_AFFECT_SUNGMA_STR = 623,
#endif

		AFFECT_EXPRING = 700,

#ifdef ENABLE_SUNG_MAHI_TOWER
		NEW_AFFECT_SUNG_MAHI_BP = 702,
		NEW_AFFECT_SUNG_MAHI_CURSE = 703,
#endif

#ifdef ENABLE_SOUL_SYSTEM
		NEW_AFFECT_SOUL_RED = 720,
		NEW_AFFECT_SOUL_BLUE = 721,
		NEW_AFFECT_SOUL_MIX = 723,
#endif

#ifdef ENABLE_YOHARA_SYSTEM
		NEW_AFFECT_SUNGMA_MAP_INFO = 800,
#endif

#ifdef ENABLE_MULTIFARM_BLOCK
		NEW_AFFECT_MULTI_FARM = 998,
#endif

#ifdef ENABLE_AUTO_SYSTEM
		NEW_AFFECT_AUTO = 999,
#endif

		NEW_AFFECT_QUEST_START_IDX = 1000
	};

	enum
	{
		STONE_SMOKE1 = 0, // 99%
		STONE_SMOKE2 = 1, // 85%
		STONE_SMOKE3 = 2, // 80%
		STONE_SMOKE4 = 3, // 60%
		STONE_SMOKE5 = 4, // 45%
		STONE_SMOKE6 = 5, // 40%
		STONE_SMOKE7 = 6, // 20%
		STONE_SMOKE8 = 7, // 10%
		STONE_SMOKE_NUM = 4
	};

	enum EBuildingAffect
	{
		BUILDING_CONSTRUCTION_SMALL = 0,
		BUILDING_CONSTRUCTION_LARGE = 1,
		BUILDING_UPGRADE = 2
	};

	enum
	{
		WEAPON_DUALHAND,
		WEAPON_ONEHAND,
		WEAPON_TWOHAND,
		WEAPON_NUM
	};

	enum
	{
		EMPIRE_NONE,
		EMPIRE_A,
		EMPIRE_B,
		EMPIRE_C,
		EMPIRE_NUM
	};

	enum
	{
		NAMECOLOR_MOB,
		NAMECOLOR_NPC,
		NAMECOLOR_PC,
		NAMECOLOR_PC_END = NAMECOLOR_PC + EMPIRE_NUM,
		NAMECOLOR_NORMAL_MOB,
		NAMECOLOR_NORMAL_NPC,
		NAMECOLOR_NORMAL_PC,
		NAMECOLOR_NORMAL_PC_END = NAMECOLOR_NORMAL_PC + EMPIRE_NUM,
		NAMECOLOR_EMPIRE_MOB,
		NAMECOLOR_EMPIRE_NPC,
		NAMECOLOR_EMPIRE_PC,
		NAMECOLOR_EMPIRE_PC_END = NAMECOLOR_EMPIRE_PC + EMPIRE_NUM,
		NAMECOLOR_FUNC,
		NAMECOLOR_PK,
		NAMECOLOR_PVP,
		NAMECOLOR_PARTY,
		NAMECOLOR_WARP,
		NAMECOLOR_WAYPOINT,
		NAMECOLOR_METIN,
		NAMECOLOR_BOSS,
#ifdef ENABLE_BATTLE_FIELD
		NAMECOLOR_TARGET_VICTIM,
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		NAMECOLOR_SHOP,
#endif
		NAMECOLOR_EXTRA = NAMECOLOR_FUNC + 10,
		NAMECOLOR_NUM = NAMECOLOR_EXTRA + 10
	};

	enum
	{
		ALIGNMENT_TYPE_WHITE,
		ALIGNMENT_TYPE_NORMAL,
		ALIGNMENT_TYPE_DARK
	};

	enum
	{
		EMOTICON_EXCLAMATION = 1,
		EMOTICON_FISH = 11,
		EMOTICON_NUM = 128,

		TITLE_NUM = 9,
		TITLE_NONE = 4,
	};

	enum
	{
		EFFECT_REFINED_NONE,

		/*---------------------------------------------------*/
				/*---------------SWORD---------------*/
		EFFECT_SWORD_REFINED7,
		EFFECT_SWORD_REFINED8,
		EFFECT_SWORD_REFINED9,
#ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_SWORD_REFINED7TH,
#endif

		/*---------------BOW---------------*/
		EFFECT_BOW_REFINED7,
		EFFECT_BOW_REFINED8,
		EFFECT_BOW_REFINED9,
#ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_BOW_REFINED7TH,
#endif

		/*---------------FAN---------------*/
		EFFECT_FANBELL_REFINED7,
		EFFECT_FANBELL_REFINED8,
		EFFECT_FANBELL_REFINED9,
#ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_FANBELL_REFINED7TH,
#endif

		/*---------------DAGGER---------------*/
		//RIGHT
		EFFECT_SMALLSWORD_REFINED7,
		EFFECT_SMALLSWORD_REFINED8,
		EFFECT_SMALLSWORD_REFINED9,
#ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_SMALLSWORD_REFINED7TH,
#endif
		//LEFT
		EFFECT_SMALLSWORD_REFINED7_LEFT,
		EFFECT_SMALLSWORD_REFINED8_LEFT,
		EFFECT_SMALLSWORD_REFINED9_LEFT,
#ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_SMALLSWORD_REFINED7TH_LEFT,
#endif

		/*---------------CLAW---------------*/
#ifdef ENABLE_WOLFMAN_CHARACTER
		//RIGHT
		EFFECT_SWORD_REFINED7_W,
		EFFECT_SWORD_REFINED8_W,
		EFFECT_SWORD_REFINED9_W,
#	ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_SWORD_REFINED7TH_W,
#	endif
		//LEFT
		EFFECT_SWORD_REFINED7_W_LEFT,
		EFFECT_SWORD_REFINED8_W_LEFT,
		EFFECT_SWORD_REFINED9_W_LEFT,
#	ifdef ENABLE_LVL96_WEAPON_EFFECT
		EFFECT_SWORD_REFINED7TH_W_LEFT,
#	endif
#endif

		/*---------------------------------------------------*/
				/*---------------ARMOR---------------*/
		EFFECT_BODYARMOR_REFINED7,
		EFFECT_BODYARMOR_REFINED8,
		EFFECT_BODYARMOR_REFINED9,
#ifdef ENABLE_LVL96_ARMOR_EFFECT
		EFFECT_BODYARMOR_SPECIAL_TH,
#endif
		EFFECT_BODYARMOR_SPECIAL,	// 4-2-1	Bubbles-Effekt
		EFFECT_BODYARMOR_SPECIAL2,	// 4-2-2	Blue-Shining-Effekt
		EFFECT_BODYARMOR_SPECIAL3,	// 5-1		Blitze-Effekt	ENABLE_LVL115_ARMOR_EFFECT

#ifdef USE_BODY_COSTUME_WITH_EFFECT
		EFFECT_COSTUMEBODY_YELLOW,
		EFFECT_COSTUMEBODY_ORANGE,
		EFFECT_COSTUMEBODY_BLUE,
		EFFECT_COSTUMEBODY_RED,
		EFFECT_COSTUMEBODY_GREEN,
		EFFECT_COSTUMEBODY_VIOLETT,
		EFFECT_COSTUMEBODY_WHITE,
#endif

#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		EFFECT_SWORD_RARITY0,
		EFFECT_SWORD_RARITY1,
		EFFECT_SWORD_RARITY2,
		EFFECT_SWORD_RARITY3,
		EFFECT_SWORD_RARITY4,
		EFFECT_SWORD_RARITY5,
		EFFECT_SWORD_RARITY6,

		EFFECT_BOW_RARITY0,
		EFFECT_BOW_RARITY1,
		EFFECT_BOW_RARITY2,
		EFFECT_BOW_RARITY3,
		EFFECT_BOW_RARITY4,
		EFFECT_BOW_RARITY5,
		EFFECT_BOW_RARITY6,

		EFFECT_FANBELL_RARITY0,
		EFFECT_FANBELL_RARITY1,
		EFFECT_FANBELL_RARITY2,
		EFFECT_FANBELL_RARITY3,
		EFFECT_FANBELL_RARITY4,
		EFFECT_FANBELL_RARITY5,
		EFFECT_FANBELL_RARITY6,

		EFFECT_SMALLSWORD_RARITY0,
		EFFECT_SMALLSWORD_RARITY1,
		EFFECT_SMALLSWORD_RARITY2,
		EFFECT_SMALLSWORD_RARITY3,
		EFFECT_SMALLSWORD_RARITY4,
		EFFECT_SMALLSWORD_RARITY5,
		EFFECT_SMALLSWORD_RARITY6,

		EFFECT_SMALLSWORD_RARITY0_LEFT,
		EFFECT_SMALLSWORD_RARITY1_LEFT,
		EFFECT_SMALLSWORD_RARITY2_LEFT,
		EFFECT_SMALLSWORD_RARITY3_LEFT,
		EFFECT_SMALLSWORD_RARITY4_LEFT,
		EFFECT_SMALLSWORD_RARITY5_LEFT,
		EFFECT_SMALLSWORD_RARITY6_LEFT,
#endif

		EFFECT_REFINED_NUM,
	};

#ifdef ENABLE_REFINE_ELEMENT
	enum
	{
		EFFECT_ELEMENTS_SWORD,
		EFFECT_ELEMENTS_SWORD_END = EFFECT_ELEMENTS_SWORD + 104,
		EFFECT_ELEMENTS_FANBELL,
		EFFECT_ELEMENTS_FANBELL_END = EFFECT_ELEMENTS_FANBELL + 104,
		EFFECT_ELEMENTS_BOW,
		EFFECT_ELEMENTS_BOW_END = EFFECT_ELEMENTS_BOW + 104,
		EFFECT_ELEMENTS_SMALLSWORD,
		EFFECT_ELEMENTS_SMALLSWORD_END = EFFECT_ELEMENTS_SMALLSWORD + 104,
		EFFECT_ELEMENTS_SMALLSWORD_LEFT,
		EFFECT_ELEMENTS_SMALLSWORD_LEFT_END = EFFECT_ELEMENTS_SMALLSWORD_LEFT + 104,
#	ifdef ENABLE_WOLFMAN_CHARACTER
		EFFECT_ELEMENTS_CLAW,
		EFFECT_ELEMENTS_CLAW_END = EFFECT_ELEMENTS_CLAW + 104,
		EFFECT_ELEMENTS_CLAW_LEFT,
		EFFECT_ELEMENTS_CLAW_LEFT_END = EFFECT_ELEMENTS_CLAW_LEFT + 104,
#	endif
		EFFECT_ELEMENTS_NUM,
	};
#endif

	enum DamageFlag
	{
		DAMAGE_NORMAL = (1 << 0),
		DAMAGE_POISON = (1 << 1),
		DAMAGE_DODGE = (1 << 2),
		DAMAGE_BLOCK = (1 << 3),
		DAMAGE_PENETRATE = (1 << 4),
		DAMAGE_CRITICAL = (1 << 5),
		DAMAGE_BLEEDING = (1 << 6),
		DAMAGE_FIRE = (1 << 7),
	};

	enum
	{
		EFFECT_DUST,
		EFFECT_STUN,
		EFFECT_HIT,
		EFFECT_FLAME_ATTACK,
		EFFECT_FLAME_HIT,
		EFFECT_FLAME_ATTACH,
		EFFECT_ELECTRIC_ATTACK,
		EFFECT_ELECTRIC_HIT,
		EFFECT_ELECTRIC_ATTACH,
		EFFECT_SPAWN_APPEAR,
		EFFECT_SPAWN_DISAPPEAR,
		EFFECT_LEVELUP,
		EFFECT_SKILLUP,
		EFFECT_HPUP_RED,
		EFFECT_SPUP_BLUE,
		EFFECT_SPEEDUP_GREEN,
		EFFECT_DXUP_PURPLE,
		EFFECT_CRITICAL,
		EFFECT_PENETRATE,
		EFFECT_BLOCK,
		EFFECT_DODGE,
		EFFECT_FIRECRACKER,
		EFFECT_SPIN_TOP,
		EFFECT_WEAPON,
		EFFECT_WEAPON_END = EFFECT_WEAPON + WEAPON_NUM,
		EFFECT_AFFECT,
		EFFECT_AFFECT_GYEONGGONG = EFFECT_AFFECT + AFFECT_GYEONGGONG,
		EFFECT_AFFECT_KWAESOK = EFFECT_AFFECT + AFFECT_KWAESOK,
		EFFECT_AFFECT_END = EFFECT_AFFECT + AFFECT_NUM,
		EFFECT_EMOTICON,
		EFFECT_EMOTICON_END = EFFECT_EMOTICON + EMOTICON_NUM,
#ifdef ENABLE_TARGET_SELECT_COLOR
		EFFECT_MONSTER,
		EFFECT_SHINSOO,
		EFFECT_CHUNJO,
		EFFECT_JINNOS,
		EFFECT_TARGET_SHINSOO,
		EFFECT_TARGET_JINNOS,
		EFFECT_TARGET_CHUNJO,
		EFFECT_TARGET_MONSTER,
#endif
		EFFECT_SELECT,
		EFFECT_TARGET,
		EFFECT_EMPIRE,
		EFFECT_EMPIRE_END = EFFECT_EMPIRE + EMPIRE_NUM,
		EFFECT_HORSE_DUST,
		EFFECT_REFINED,
		EFFECT_REFINED_END = EFFECT_REFINED + EFFECT_REFINED_NUM,
		EFFECT_DAMAGE_TARGET,
		EFFECT_DAMAGE_NOT_TARGET,
		EFFECT_DAMAGE_SELFDAMAGE,
		EFFECT_DAMAGE_SELFDAMAGE2,
		EFFECT_DAMAGE_POISON,
		EFFECT_DAMAGE_MISS,
		EFFECT_DAMAGE_TARGETMISS,
		EFFECT_DAMAGE_CRITICAL,
		EFFECT_SUCCESS,
		EFFECT_FAIL,
		EFFECT_FR_SUCCESS,
		EFFECT_LEVELUP_ON_14_FOR_GERMANY,
		EFFECT_LEVELUP_UNDER_15_FOR_GERMANY,
		EFFECT_PERCENT_DAMAGE1,
		EFFECT_PERCENT_DAMAGE2,
		EFFECT_PERCENT_DAMAGE3,
		EFFECT_AUTO_HPUP,
		EFFECT_AUTO_SPUP,
		EFFECT_RAMADAN_RING_EQUIP,
		EFFECT_HALLOWEEN_CANDY_EQUIP,
		EFFECT_HAPPINESS_RING_EQUIP,
		EFFECT_LOVE_PENDANT_EQUIP,
		EFFECT_TEMP,
		EFFECT_EQUIP_BOOTS,
		EFFECT_EQUIP_HEROMEDAL,
		EFFECT_EQUIP_CHOCOLATE_AMULET,
		EFFECT_EQUIP_EMOTION_MASK,
		EFFECT_WILLPOWER_RING,
		EFFECT_DEADLYPOWER_RING,
		EFFECT_EASTER_CANDY_EQIP,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		EFFECT_ACCE_SUCESS_ABSORB,
		EFFECT_ACCE_EQUIP,
		EFFECT_ACCE_BACK,
#endif
#ifdef ENABLE_DAWNMIST_DUNGEON
		EFFECT_HEAL,
#endif
		EFFECT_DAMAGE_BLEEDING,
		EFFECT_DAMAGE_FIRE,
#ifdef ENABLE_ANTI_EXP_RING
		EFFECT_AUTO_EXP,
#endif
		EFFECT_CAPE_OF_COURAGE,	//@fixme421
		EFFECT_GYEONGGONG_BOOM,
#ifdef ENABLE_12ZI
		EFFECT_SKILL_DAMAGE_ZONE,
		EFFECT_SKILL_SAFE_ZONE,

		EFFECT_METEOR,
		EFFECT_BEAD_RAIN,
		EFFECT_FALL_ROCK,
		EFFECT_ARROW_RAIN,
		EFFECT_HORSE_DROP,
		EFFECT_EGG_DROP,
		EFFECT_DEAPO_BOOM,

		EFFECT_SKILL_DAMAGE_ZONE_BIG,
		EFFECT_SKILL_DAMAGE_ZONE_MIDDLE,
		EFFECT_SKILL_DAMAGE_ZONE_SMALL,

		EFFECT_SKILL_SAFE_ZONE_BIG,
		EFFECT_SKILL_SAFE_ZONE_MIDDLE,
		EFFECT_SKILL_SAFE_ZONE_SMALL,
#endif
#ifdef ENABLE_BATTLE_FIELD
		EFFECT_BATTLE_POTION,
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1,
		EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2,
		EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3,
#endif
#ifdef ENABLE_REFINE_ELEMENT
		EFFECT_ELEMENTS_WEAPON,
		EFFECT_ELEMENTS_WEAPON_END = EFFECT_ELEMENTS_WEAPON + EFFECT_ELEMENTS_NUM,
#endif
		EFFECT_FEATHER_WALK,
#ifdef ENABLE_FLOWER_EVENT
		EFFECT_FLOWER_EVENT,
#endif
#ifdef ENABLE_GEM_SYSTEM
		EFFECT_GEM_PENDANT,
#endif
#ifdef ENABLE_QUEEN_NETHIS
		EFFECT_SNAKE_REGEN,
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		EFFECT_BP_NORMAL_MISSION_COMPLETED,
		EFFECT_BP_PREMIUM_MISSION_COMPLETED,
		EFFECT_BP_EVENT_MISSION_COMPLETED,
		EFFECT_BP_NORMAL_BATTLEPASS_COMPLETED,
		EFFECT_BP_PREMIUM_BATTLEPASS_COMPLETED,
		EFFECT_BP_EVENT_BATTLEPASS_COMPLETED,
#endif

		EFFECT_NUM
	};

	enum
	{
		DUEL_NONE,
		DUEL_CANNOTATTACK,
		DUEL_START
	};

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	enum
	{
		GUILD_MELEY_LAIR_DUNGEON_STATUE = 6118,
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		PARTY_MELEY_LAIR_DUNGEON_STATUE = 6209,
#	endif
	};
#endif

public:
	static void DestroySystem();
	static void CreateSystem(uint32_t uCapacity);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	static bool RegisterEffect(uint32_t eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache, const char* name = nullptr);
#else
	static bool RegisterEffect(uint32_t eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache);
#endif
	static void RegisterTitleName(int iIndex, const char* c_szTitleName);
	static bool RegisterNameColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b);
	static bool RegisterTitleColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b);
	static bool ChangeEffectTexture(uint32_t eEftType, const char* c_szSrcFileName, const char* c_szDstFileName);

	static void SetDustGap(float fDustGap);
	static void SetHorseDustGap(float fDustGap);

	static void SetEmpireNameMode(bool isEnable);
	static const D3DXCOLOR& GetIndexedNameColor(uint32_t eNameColor);

	int HasAffect(uint32_t dwIndex);

public:
	void SetMainInstance();

	void OnSelected();
	void OnUnselected();
	void OnTargeted();
	void OnUntargeted();

protected:
	bool __IsExistMainInstance() const;
	bool __IsMainInstance();
	bool __MainCanSeeHiddenThing() const;
	float __GetBowRange();

protected:
#ifdef ENABLE_SCALE_SYSTEM
	uint32_t	__AttachEffect(uint32_t eEftType, float fScale = 1.0f);
#else
	uint32_t	__AttachEffect(uint32_t eEftType);
#endif
	uint32_t __AttachEffect(char filename[128]);
	void __DetachEffect(uint32_t dwEID);

public:
#ifdef ENABLE_SCALE_SYSTEM
	void CreateSpecialEffect(uint32_t iEffectIndex, float fScale = 1.0f);
	void AttachSpecialEffect(uint32_t effect, float fScale = 1.0f);
#else
	void CreateSpecialEffect(uint32_t iEffectIndex);
	void AttachSpecialEffect(uint32_t effect);
#endif
#ifdef ENABLE_12ZI
	void AttachSpecialZodiacEffect(uint32_t eEftType, long GetX, long GetY, float fParticleScale);
#endif

protected:
	static std::string ms_astAffectEffectAttachBone[EFFECT_NUM];
	static uint32_t ms_adwCRCAffectEffect[EFFECT_NUM];
	static float ms_fDustGap;
	static float ms_fHorseDustGap;

public:
	CInstanceBase();
	virtual ~CInstanceBase();

	bool LessRenderOrder(CInstanceBase* pkInst);

	void MountHorse(uint32_t eRace);
	void DismountHorse();

	// 스크립트용 테스트 함수. 나중에 없에자
	void SCRIPT_SetAffect(uint32_t eAffect, bool isVisible);

	float CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst);

	// Instance Data
	bool IsFlyTargetObject();
	void ClearFlyTargetInstance();
	void SetFlyTargetInstance(CInstanceBase& rkInstDst);
	void AddFlyTargetInstance(CInstanceBase& rkInstDst);
	void AddFlyTargetPosition(const TPixelPosition& c_rkPPosDst);

	float GetFlyTargetDistance();

	void SetAlpha(float fAlpha);

	void DeleteBlendOut();

	void AttachTextTail();
	void DetachTextTail();
	void UpdateTextTailLevel(uint32_t level);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void UpdateTextTailLanguage(uint8_t bLanguage);
#endif

	void RefreshTextTail();
	void RefreshTextTailTitle();

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	void UpdatePortalTextTailLevelLimit();
#endif

	bool Create(const SCreateData& c_rkCreateData);

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	void Destroy();

	void Update();
	bool UpdateDeleting();

	void Transform();
	void Deform();
	void Render();
	void RenderTrace();
	void RenderToShadowMap();
	void RenderCollision();
	void RenderCollisionNew();
	void RegisterBoundingSphere();

	// Temporary
	void GetBoundBox(D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax);

	void SetNameString(const char* c_szName, int len);
	bool SetRace(uint32_t eRace);
	void SetVirtualID(uint32_t dwVirtualID);
	void SetVirtualNumber(uint32_t dwVirtualNumber);
	void SetInstanceType(int iInstanceType);
	void					SetInstanceFallState(bool bIsFall);
	void SetAlignment(int16_t sAlignment);
	void SetPKMode(uint8_t byPKMode);
	void SetKiller(bool bFlag);
	void SetPartyMemberFlag(bool bFlag);
	void SetStateFlags(uint32_t dwStateFlags);

#ifdef ENABLE_PENDANT
	bool					SetPendant(uint32_t ePendant);
#endif
	void SetArmor(uint32_t dwArmor);
	void SetShape(uint32_t eShape, float fSpecular = 0.0f);
	void SetHair(uint32_t eHair);
	bool SetWeapon(uint32_t eWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		, long lGroupWeapon = 0
#endif
	);
	bool ChangeArmor(uint32_t dwArmor);
	void ChangeWeapon(uint32_t eWeapon
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		, long lGroupWeapon = 0
#endif
	);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	bool SetAcce(uint32_t eAcce);
	void ChangeAcce(uint32_t eAcce);
#endif
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	void SetGroupWeapon(long lGroupWeapon) { m_lGroupWeapon = lGroupWeapon; }
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	bool SetArrow(uint32_t eArrow);
#endif
	void ChangeHair(uint32_t eHair);
#ifdef ENABLE_SHOW_GUILD_LEADER
	void ChangeGuild(uint32_t dwGuildID, uint32_t dwNewIsGuildName);
#else
	void ChangeGuild(uint32_t dwGuildID);
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void					ChangeSkillColor(const uint32_t* dwSkillColor);
#endif
	uint32_t GetWeaponType();
#ifdef ENABLE_PENDANT
	void					ChangePendant(uint32_t ePendant);
#endif
	void SetComboType(uint32_t uComboType);
	void SetAttackSpeed(uint32_t uAtkSpd);
	void SetMoveSpeed(uint32_t uMovSpd);
	void SetRotationSpeed(float fRotSpd);

	const char* GetNameString() const;
#ifdef ENABLE_LEVEL_IN_TRADE
	uint32_t GetLevel() const;
#endif
#ifdef ENABLE_TEXT_LEVEL_REFRESH
	void SetLevel(uint32_t dwLevel);
#endif
#if defined(ENABLE_YOHARA_SYSTEM) && defined(ENABLE_TEXT_LEVEL_REFRESH)
	uint32_t GetConquerorLevel();
	void SetConquerorLevel(uint32_t dwcLevel);
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void SetLanguage(uint8_t bLanguage) { m_bLanguage = bLanguage; }
	uint8_t GetLanguage() { return m_bLanguage; }
#endif
	int GetInstanceType() const;
	uint32_t GetPart(CRaceData::EParts part);
	uint32_t GetShape() const;
	uint32_t GetRace() const;
	uint32_t GetVirtualID();
	uint32_t GetVirtualNumber() const;
	uint32_t GetEmpireID() const;
#ifdef WJ_SHOW_MOB_INFO
	uint32_t GetAIFlag() const;
#endif
	uint32_t GetGuildID() const;
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t					GetNewIsGuildName();
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t* GetSkillColor(const uint32_t dwSkillIndex);
#endif
	int GetAlignment() const;
	uint32_t GetAlignmentGrade() const;
	int GetAlignmentType() const;
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	long					GetGroupWeapon() { return m_lGroupWeapon; }
#endif
	uint8_t GetPKMode() const;
	bool IsKiller() const;
	bool IsPartyMember() const;

	void ActDualEmotion(CInstanceBase& rkDstInst, uint16_t wMotionNumber1, uint16_t wMotionNumber2);
	void ActEmotion(uint32_t dwMotionNumber);
	void LevelUp();
	void SkillUp();
	void UseSpinTop();
	void Revive();
	void Stun();
	void Die();
#ifdef ENABLE_AUTO_SYSTEM
	void SetAutoAffect(bool val);
#endif
	void Hide();
	void Show();

	bool CanAct();
	bool CanMove();
	bool CanAttack();
	bool CanUseSkill();
	bool CanFishing();
	bool IsConflictAlignmentInstance(const CInstanceBase& rkInstVictim) const;
	bool IsAttackableInstance(CInstanceBase& rkInstVictim);
	bool IsTargetableInstance(CInstanceBase& rkInstVictim) const;
	bool IsPVPInstance(CInstanceBase& rkInstSel);
	bool CanChangeTarget();
	bool CanPickInstance();
	bool CanViewTargetHP(CInstanceBase& rkInstVictim) const;
	DWORD FindNearestVictimVID(DWORD dwCurrentVID = 0);

	// Movement
	BOOL IsGoing() const;
	bool NEW_Goto(const TPixelPosition& c_rkPPosDst, float fDstRot);
	void EndGoing();

	void SetRunMode();
	void SetWalkMode();

	bool IsAffect(uint32_t uAffect) const;
	BOOL IsInvisibility();
	BOOL IsParalysis();
	BOOL IsGameMaster() const;
#ifdef ENABLE_BATTLE_FIELD
	BOOL IsTargetVictim() const;
#endif
#ifdef ENABLE_VIP_SYSTEM
	BOOL IsVIP() const;
#endif
	BOOL IsSameEmpire(const CInstanceBase& rkInstDst) const;
	BOOL IsBowMode();
	BOOL IsHandMode();
	BOOL IsFishingMode();
	BOOL IsFishing();

	BOOL IsWearingDress() const;
	BOOL IsHoldingPickAxe();
	BOOL IsMountingHorse() const;
	BOOL IsMountingHorseOnly();
	BOOL IsNewMount() const;
	BOOL IsForceVisible();
#ifdef ENABLE_EVENT_BANNER_FLAG
	BOOL IsBannerFlag();
#endif
	BOOL IsInSafe();
	BOOL IsEnemy();
	BOOL IsShop();
	BOOL IsStone();
	BOOL IsBoss();
	BOOL IsResource() const;
	BOOL IsNPC();
	BOOL IsPC();
	BOOL IsPoly();
	BOOL IsWarp();
	BOOL IsGoto();
#ifdef ENABLE_PROTO_RENEWAL
	BOOL IsPetPay();
	BOOL IsHorse();
#endif
	BOOL IsObject();
	BOOL IsDoor();
	BOOL IsBuilding();
	BOOL IsWoodenDoor();
	BOOL IsStoneDoor();
	BOOL IsFlag() const;
	BOOL IsGuildWall() const;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	BOOL IsGrowthPet();
#endif
#ifdef ENABLE_EXTENDED_CONFIGS
	BOOL IsMount();
#endif
#ifdef ENABLE_BATTLE_FIELD
	BOOL IsBattleFieldMap();
#endif
	BOOL IsDead();
	BOOL IsStun();
	BOOL IsSleep();
	BOOL __IsSyncing();
	BOOL IsWaiting();
	BOOL IsWalking();
	BOOL IsPushing();
	BOOL IsAttacking();
	BOOL IsActingEmotion();
	BOOL IsAttacked();
	BOOL IsKnockDown();
	BOOL IsUsingSkill();
	BOOL IsUsingMovingSkill();
	BOOL CanCancelSkill();
	BOOL CanAttackHorseLevel() const;

#ifdef __MOVIE_MODE__
	BOOL IsMovieMode();
#endif
	bool NEW_CanMoveToDestPixelPosition(const TPixelPosition& c_rkPPosDst);

	void NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);
	void NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
	bool NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	void SetAdvancingRotation(float fRotation);

	void EndWalking(float fBlendingTime = 0.15f);
	void EndWalkingWithoutBlending();

	// Battle
	void SetEventHandler(CActorInstance::IEventHandler* pkEventHandler);

	void PushUDPState(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg) const;
	void PushTCPState(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg);
	void PushTCPStateExpanded(uint32_t dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg, uint32_t uTargetVID);

	void NEW_Stop();

	bool NEW_UseSkill(uint32_t uSkill, uint32_t uMot, uint32_t uMotLoopCount, bool isMovingSkill);
	void NEW_Attack();
	void NEW_Attack(float fDirRot);
	void NEW_AttackToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
	bool NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst, IFlyEventHandler* pkFlyHandler);
	bool NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst);

	bool NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
	void NEW_MoveToDestInstanceDirection(CInstanceBase& rkInstDst);
	void NEW_MoveToDirection(float fDirRot);

	float NEW_GetDistanceFromDirPixelPosition(const TPixelPosition& c_rkPPosDir) const;
	float NEW_GetDistanceFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	float NEW_GetDistanceFromDestInstance(CInstanceBase& rkInstDst);

	float NEW_GetRotation();
	float NEW_GetRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	float NEW_GetRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir) const;
	float NEW_GetRotationFromDestInstance(CInstanceBase& rkInstDst);

	float NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir) const;
	float NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	float NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst) const;

	BOOL NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition& c_rkPPosDst);
	BOOL NEW_IsClickableDistanceDestInstance(CInstanceBase& rkInstDst);

	bool NEW_GetFrontInstance(CInstanceBase** ppoutTargetInstance, float fDistance);
	void NEW_GetRandomPositionInFanRange(CInstanceBase& rkInstTarget, TPixelPosition* pkPPosDst);
	bool NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase& rkInstTarget, std::vector<CInstanceBase*>* pkVct_pkInst);
	bool NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase*>* pkVct_pkInst);

	void NEW_SetOwner(uint32_t dwVIDOwner);
	void NEW_SyncPixelPosition(const long& nPPosX, const long& nPPosY);
	void NEW_SyncCurrentPixelPosition();

	void NEW_SetPixelPosition(const TPixelPosition& c_rPixelPosition);

	bool NEW_IsLastPixelPosition();
	const TPixelPosition& NEW_GetLastPixelPositionRef();


	// Battle
	BOOL isNormalAttacking();
	BOOL isComboAttacking();
	MOTION_KEY GetNormalAttackIndex();
	uint32_t GetComboIndex();
	float GetAttackingElapsedTime();
	void InputNormalAttack(float fAtkDirRot);
	void InputComboAttack(float fAtkDirRot);

	void RunNormalAttack(float fAtkDirRot);
	void RunComboAttack(float fAtkDirRot, uint32_t wMotionIndex);

	CInstanceBase* FindNearestVictim();
	BOOL CheckAdvancing();


	bool AvoidObject(const CGraphicObjectInstance& c_rkBGObj);
	bool IsBlockObject(const CGraphicObjectInstance& c_rkBGObj);
	void BlockMovement();

#ifdef ENABLE_REFINE_ELEMENT
	void					SetElementEffect(uint8_t wElementsEffect);
	uint8_t					GetElementEffect();
	void					ClearElementEffect();
	void					AttachElementEffect(const CItemData* pItem);
#endif

public:
	BOOL CheckAttacking(CInstanceBase& rkInstVictim);
	void ProcessHitting(uint32_t dwMotionKey, CInstanceBase* pVictimInstance) const;
	void ProcessHitting(uint32_t dwMotionKey, uint8_t byEventIndex, CInstanceBase* pVictimInstance) const;
	void GetBlendingPosition(TPixelPosition* pPixelPosition);
	void SetBlendingPosition(const TPixelPosition& c_rPixelPosition);

	// Fishing
	void StartFishing(float frot);
	void StopFishing();
	void ReactFishing();
	void CatchSuccess();
	void CatchFail();
	BOOL GetFishingRot(int* pirot);

	// Render Mode
	void RestoreRenderMode();
	void SetAddRenderMode();
	void SetModulateRenderMode();
	void SetRenderMode(int iRenderMode);
	void SetAddColor(const D3DXCOLOR& c_rColor);

	// Position
	void SCRIPT_SetPixelPosition(float fx, float fy);
	void NEW_GetPixelPosition(TPixelPosition* pPixelPosition);

	// Rotation
	void NEW_LookAtFlyTarget();
	void NEW_LookAtDestInstance(CInstanceBase& rkInstDst);
	void NEW_LookAtDestPixelPosition(const TPixelPosition& c_rkPPosDst);

	float GetRotation();
	float GetAdvancingRotation();
	void SetRotation(float fRotation);
	void BlendRotation(float fRotation, float fBlendTime = 0.1f);

	void SetDirection(int dir);
	void BlendDirection(int dir, float blendTime);
	float GetDegreeFromDirection(int dir) const;

	// Motion
	//	Motion Deque
	BOOL isLock();

	void SetMotionMode(int iMotionMode);
	int GetMotionMode(uint32_t dwMotionIndex);

	// Motion
	//	Pushing Motion
	void ResetLocalTime();
	void SetLoopMotion(uint16_t wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
	void PushOnceMotion(uint16_t wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
	void PushLoopMotion(uint16_t wMotion, float fBlendTime = 0.1f, float fSpeedRatio = 1.0f);
	void SetEndStopMotion();

	// Intersect
	bool IntersectDefendingSphere();
	bool IntersectBoundingBox();

	// Part
	//void					SetParts(const uint16_t * c_pParts);
	void Refresh(uint32_t dwMotIndex, bool isLoop);

	//void					AttachEffectByID(uint32_t dwParentPartIndex, const char * c_pszBoneName, uint32_t dwEffectID, int dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // 수명은 ms단위입니다.
	//void					AttachEffectByName(uint32_t dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName, int dwLife = CActorInstance::EFFECT_LIFE_INFINITE ); // 수명은 ms단위입니다.

	float GetDistance(CInstanceBase* pkTargetInst);
	float GetDistance(const TPixelPosition& c_rPixelPosition);

	float					GetBaseHeight();

	// ETC
	CActorInstance& GetGraphicThingInstanceRef();
	CActorInstance* GetGraphicThingInstancePtr();

	bool __Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos) const;
	bool __Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight) const;

	// 2004.07.25.myevan.이펙트 안나오는 문제
	/////////////////////////////////////////////////////////////
	void __ClearAffectFlagContainer();
	void __ClearAffects();
	/////////////////////////////////////////////////////////////

	void __SetAffect(uint32_t eAffect, bool isVisible);

	void SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

	void __SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);
	void __SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

	void SetEmoticon(uint32_t eEmoticon);
	void SetFishEmoticon();
	bool IsPossibleEmoticon() const;

protected:
	uint32_t __LessRenderOrder_GetLODLevel();
	void __Initialize();
	void __InitializeRotationSpeed();

	void __Create_SetName(const SCreateData& c_rkCreateData);
	void __Create_SetWarpName(const SCreateData& c_rkCreateData);

	CInstanceBase* __GetMainInstancePtr() const;
	CInstanceBase* __FindInstancePtr(uint32_t dwVID) const;

	bool __FindRaceType(uint32_t dwRace, uint8_t* pbType) const;
	uint32_t __GetRaceType() const;

	bool __IsShapeAnimalWear() const;
	BOOL __IsChangableWeapon(int iWeaponID) const;

	void __EnableSkipCollision();
	void __DisableSkipCollision();

	void __ClearMainInstance() const;

	void __Shaman_SetParalysis(bool isParalysis);
	void __Warrior_SetGeomgyeongAffect(bool isVisible);
	void __Assassin_SetEunhyeongAffect(bool isVisible);
	void __Assassin_SetGyeongGongAffect(bool isVisible);
	void __SetReviveInvisibilityAffect(bool isVisible);

	BOOL __CanProcessNetworkStatePacket();

	bool __IsInDustRange();

	// Emotion
	void __ProcessFunctionEmotion(uint32_t dwMotionNumber, uint32_t dwTargetVID, const TPixelPosition& c_rkPosDst);
	void __EnableChangingTCPState();
	void __DisableChangingTCPState();
	BOOL __IsEnableTCPProcess(uint32_t eCurFunc);

	// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
	bool __CanRender();
	bool __IsInViewFrustum();

	// HORSE
	void __AttachHorseSaddle();
	void __DetachHorseSaddle();

	struct SHORSE
	{
		bool m_isMounting;
		std::unique_ptr<CActorInstance> m_pkActor;

		SHORSE();
		~SHORSE();

		void Destroy();
		void Create(const TPixelPosition& c_rkPPos, uint32_t eRace, uint32_t eHitEffect);

		void SetAttackSpeed(uint32_t uAtkSpd) const;
		void SetMoveSpeed(uint32_t uMovSpd) const;
		void Deform() const;
		void Render() const;
		CActorInstance& GetActorRef() const;
		CActorInstance* GetActorPtr() const;

		bool IsMounting() const;
		bool CanAttack() const;
		bool CanUseSkill() const;

		uint32_t GetLevel() const;
		bool IsNewMount() const;

		void __Initialize();
	} m_kHorse;


protected:
	// Blend Mode
	void __SetBlendRenderingMode();
	void __SetAlphaValue(float fAlpha);
	float __GetAlphaValue();

	void __ComboProcess() const;
	void MovementProcess();
	void TodoProcess();
	void StateProcess();
	void AttackProcess();

	void StartWalking();
	float GetLocalTime() const;

	void RefreshState(uint32_t dwMotIndex, bool isLoop);
	void RefreshActorInstance();

protected:
	void OnSyncing();
	void OnWaiting();
	void OnMoving();

	void NEW_SetCurPixelPosition(const TPixelPosition& c_rkPPosDst);
	void NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc);
	void NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst);
	void NEW_SetDstPixelPositionZ(FLOAT z);

	const TPixelPosition& NEW_GetCurPixelPositionRef();
	const TPixelPosition& NEW_GetSrcPixelPositionRef();

public:
	const TPixelPosition& NEW_GetDstPixelPositionRef();

protected:
	BOOL m_isTextTail;

	// Instance Data
	std::string m_stName;

	uint32_t m_awPart[CRaceData::PART_MAX_NUM];

	uint32_t m_dwLevel;
#ifdef ENABLE_YOHARA_SYSTEM
	uint32_t m_dwcLevel;
#endif
#ifdef WJ_SHOW_MOB_INFO
	uint32_t m_dwAIFlag;
#endif
	uint32_t m_dwEmpireID;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t m_bLanguage;
#endif
	uint32_t m_dwGuildID;
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	long m_lGroupWeapon;
#endif
#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t m_dwNewIsGuildName;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t* m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif

protected:
	CAffectFlagContainer m_kAffectFlagContainer;
	uint32_t m_adwCRCAffectEffect[AFFECT_NUM];

	uint32_t	__GetRefinedEffect(CItemData* pItem
#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
		, long lGroupWeapon = 0
#endif
	);
	void __ClearWeaponRefineEffect();
	void __ClearArmorRefineEffect();
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	void __ClearAcceRefineEffect();
#endif

#ifdef ENABLE_HEROIC_EFFECT
	void	__GetHeroicEffect(CItemData* pItem);
	void	__ClearHeroicEffect();
#endif

#ifdef ENABLE_SHINING_SYSTEM
	void __GetShiningEffect(CItemData* pItem);
	void __ClearShiningEffect(uint8_t bType, uint8_t sub = 0);
	void __AttachShiningEffect(int effectIndex, const char* effectFileName, const char* boneName, uint8_t bType, uint8_t sub = 0);
#endif

protected:
	void __AttachSelectEffect();
	void __DetachSelectEffect();

	void __AttachTargetEffect();
	void __DetachTargetEffect();

#ifdef ENABLE_TARGET_SELECT_COLOR
	void __AttachTargetEffectMonster();
	void __DetachTargetEffectMonster();
	void __AttachSelectEffectMonster();
	void __DetachSelectEffectMonster();

	void __AttachTargetEffectShinsoo();
	void __DetachTargetEffectShinsoo();
	void __AttachSelectEffectShinsoo();
	void __DetachSelectEffectShinsoo();

	void __AttachTargetEffectJinnos();
	void __DetachTargetEffectJinnos();
	void __AttachSelectEffectJinnos();
	void __DetachSelectEffectJinnos();

	void __AttachTargetEffectChunjo();
	void __DetachTargetEffectChunjo();
	void __AttachSelectEffectChunjo();
	void __DetachSelectEffectChunjo();
#endif

	void __AttachEmpireEffect(uint32_t eEmpire);

protected:
	struct SEffectContainer
	{
		typedef std::map<uint32_t, uint32_t> Dict;
		Dict m_kDct_dwEftID;
	} m_kEffectContainer;

	void __EffectContainer_Initialize();
	void __EffectContainer_Destroy();

	uint32_t __EffectContainer_AttachEffect(uint32_t dwEftKey);
	void __EffectContainer_DetachEffect(uint32_t dwEftKey);

	SEffectContainer::Dict& __EffectContainer_GetDict();

protected:
	struct SStoneSmoke
	{
		uint32_t m_dwEftID;
	} m_kStoneSmoke;

	void __StoneSmoke_Inialize();
	void __StoneSmoke_Destroy();
	void __StoneSmoke_Create(uint32_t eSmoke);


protected:
	// Emoticon
	//uint32_t					m_adwCRCEmoticonEffect[EMOTICON_NUM];

	uint8_t m_eType;
	uint8_t m_eRaceType;
	uint32_t m_eShape;
	uint32_t m_dwRace;
	uint32_t m_dwVirtualNumber;
	int16_t m_sAlignment;
	uint8_t m_byPKMode;

	bool m_isKiller;
	bool m_isPartyMember;

	// Movement
	int m_iRotatingDirection;

	uint32_t m_dwAdvActorVID;
	uint32_t m_dwLastDmgActorVID;

	LONG m_nAverageNetworkGap;
	uint32_t m_dwNextUpdateHeightTime;

	bool m_isGoing;

	TPixelPosition m_kPPosDust;

	uint32_t m_dwLastComboIndex;

	uint32_t m_swordRefineEffectRight;
	uint32_t m_swordRefineEffectLeft;
	uint32_t m_armorRefineEffect[2];
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	uint32_t				m_acceRefineEffect;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	uint8_t 					m_dwElementsEffect;
	uint32_t					m_swordElementEffectRight;
	uint32_t					m_swordElementEffectLeft;
#endif
#ifdef ENABLE_HEROIC_EFFECT
	uint32_t					m_heroicEffectsRight;
	uint32_t					m_heroicEffectsLeft;
#endif
#ifdef ENABLE_SHINING_SYSTEM
	uint32_t					m_weaponShiningEffects[2][CItemData::ITEM_SHINING_MAX_COUNT];
	uint32_t					m_armorShiningEffects[CItemData::ITEM_SHINING_MAX_COUNT];
	uint32_t                    m_acceShiningEffects[CItemData::ITEM_SHINING_MAX_COUNT];
#endif

	struct SMoveAfterFunc
	{
		uint32_t eFunc;
		uint32_t uArg;

		// For Emotion Function
		uint32_t uArgExpanded;
		TPixelPosition kPosDst;
	};

	SMoveAfterFunc m_kMovAfterFunc;

	float m_fDstRot;
	float m_fAtkPosTime;
	float m_fRotSpd;
	float m_fMaxRotSpd;

	BOOL m_bEnableTCPState;

	// Graphic Instance
	CActorInstance m_GraphicThingInstance;


protected:
	struct SCommand
	{
		uint32_t m_dwChkTime;
		uint32_t m_dwCmdTime;
		float m_fDstRot;
		uint32_t m_eFunc;
		uint32_t m_uArg;
		uint32_t m_uTargetVID;
		TPixelPosition m_kPPosDst;
	};

	using CommandQueue = std::list<SCommand>;

	uint32_t m_dwBaseChkTime;
	uint32_t m_dwBaseCmdTime;

	uint32_t m_dwSkipTime;

	CommandQueue m_kQue_kCmdNew;

	BOOL m_bDamageEffectType;

	struct SEffectDamage
	{
		uint32_t damage;
		uint8_t flag;
		BOOL bSelf;
		BOOL bTarget;
	};

	using CommandDamageQueue = std::list<SEffectDamage>;
	CommandDamageQueue m_DamageQueue;

	bool ProcessDamage();

public:
	void AddDamageEffect(uint32_t damage, uint8_t flag, BOOL bSelf, BOOL bTarget);

protected:
	struct SWarrior
	{
		uint32_t m_dwGeomgyeongEffect;
	};

	SWarrior m_kWarrior;

	void __Warrior_Initialize();

protected:
	struct SAssassin
	{
		uint32_t m_dwGyeongGongEffect;
	};

	SAssassin m_kAssassin;

	void __Assassin_Initialize();

public:
	static void ClearPVPKeySystem();

	static void InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	static void InsertPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	static void RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);

	static void InsertGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID);
	static void RemoveGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID);

	static void InsertDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);

	uint32_t GetNameColorIndex();

	const D3DXCOLOR& GetNameColor();
	const D3DXCOLOR& GetTitleColor() const;

protected:
	static uint32_t __GetPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	static bool __FindPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	static bool __FindPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	static bool __FindGVGKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);
	static bool __FindDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);

protected:
	CActorInstance::IEventHandler* GetEventHandlerPtr();
	CActorInstance::IEventHandler& GetEventHandlerRef();

protected:
	static float __GetBackgroundHeight(float x, float y);
	static uint32_t __GetShadowMapColor(float x, float y);

public:
	static void ResetPerformanceCounter();
	static void GetInfo(std::string* pstInfo);

public:
	static CInstanceBase* New();
	static void Delete(CInstanceBase* pkInst);

	static CDynamicPool<CInstanceBase> ms_kPool;

protected:
	static uint32_t ms_dwUpdateCounter;
	static uint32_t ms_dwRenderCounter;
	static uint32_t ms_dwDeformCounter;

public:
	uint32_t GetDuelMode() const;
	void SetDuelMode(uint32_t type);

protected:
	uint32_t m_dwDuelMode;
	uint32_t m_dwEmoticonTime;

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
public:
	void SetTitleID(uint32_t title) { m_dwTitle = title; }
	uint32_t GetTitleID() const { return m_dwTitle; }

protected:
	uint32_t m_dwTitle;
#endif

#ifdef ENABLE_RENDER_TARGET
protected:
	bool m_IsAlwaysRender;

public:
	bool IsAlwaysRender();
	void SetAlwaysRender(bool val);
#endif
#ifdef ENABLE_AURA_SYSTEM
public:
	bool	SetAura(uint32_t eAura);
	void	ChangeAura(uint32_t eAura);
protected:
	uint32_t	m_auraRefineEffect;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
private:
	uint8_t m_bRank;

public:
	void SetRank(uint8_t bRank);
	uint32_t NEW_GetDistanceFromMainInstance();

	bool IsShowEffects();
	bool HasPrivateShopSign();
	bool IsShowActor();
#endif
};

inline int RaceToJob(int race)
{
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (race == 8)
		return 4;
#endif

	const int JOB_NUM = 4;
	return race % JOB_NUM;
}

inline int RaceToSex(int race)
{
	switch (race)
	{
	case 0:
	case 2:
	case 5:
	case 7:
#ifdef ENABLE_WOLFMAN_CHARACTER
	case 8:
#endif
		return 1;
	case 1:
	case 3:
	case 4:
	case 6:
		return 0;
	}
	return 0;
}
