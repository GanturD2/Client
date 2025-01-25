#ifndef __INC_METIN_II_CHAR_H__
#define __INC_METIN_II_CHAR_H__

#include <unordered_map>
#include "../../common/stl.h"
#include "entity.h"
#include "FSM.h"
#include "horse_rider.h"
#include "vid.h"
#include "constants.h"
#include "affect.h"
#include "affect_flag.h"
#ifndef ENABLE_CUBE_RENEWAL
#include "cube.h"
#endif
#include "mining.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
# include "sectree_manager.h"
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
# include "SoulRoulette.h"
#endif
#if defined(ENABLE_MINI_GAME_CATCH_KING) || defined(ENABLE_MONSTER_BACK)
# include "packet.h"
#endif
#ifdef ENABLE_MAILBOX
# include "MailBox.h"
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
# include "Transmutation.h"
#endif
#ifdef ENABLE_PULSE_MANAGER
# include "PulseManager.h"
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#include "desc.h"
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
#	include "BiologSystemManager.h"
#endif

enum eMountType { MOUNT_TYPE_NONE = 0, MOUNT_TYPE_NORMAL = 1, MOUNT_TYPE_COMBAT = 2, MOUNT_TYPE_MILITARY = 3 };
eMountType GetMountLevelByVnum(uint32_t dwMountVnum, bool IsNew);
const uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);
#ifdef ENABLE_NINETH_SKILL
const uint32_t GetRandomForgetSkillVnum(uint8_t bJob = JOB_MAX_NUM);
#endif

#ifdef ENABLE_CSHIELD
class CShield;
typedef std::shared_ptr<CShield> spCShield;
#endif

class CBuffOnAttributes;
class CPetSystem;
#ifdef ENABLE_GROWTH_PET_SYSTEM
class CGrowthPetSystem;
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
class CMeleyLair;
#endif

#define INSTANT_FLAG_DEATH_PENALTY (1 << 0)
#define INSTANT_FLAG_SHOP (1 << 1)
#define INSTANT_FLAG_EXCHANGE (1 << 2)
#define INSTANT_FLAG_STUN (1 << 3)
#define INSTANT_FLAG_NO_REWARD (1 << 4)

#define AI_FLAG_NPC (1 << 0)
#define AI_FLAG_AGGRESSIVE (1 << 1)
#define AI_FLAG_HELPER (1 << 2)
#define AI_FLAG_STAYZONE (1 << 3)

#define SET_OVER_TIME(ch, time) (ch)->SetOverTime(time)

extern int g_nPortalLimitTime;

enum
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};

enum
{
	POISON_LENGTH = 30,
#ifdef ENABLE_WOLFMAN_CHARACTER
	BLEEDING_LENGTH = 30,
#endif
	STAMINA_PER_STEP = 1,
	SAFEBOX_PAGE_SIZE = 9,
	AI_CHANGE_ATTACK_POISITION_TIME_NEAR = 10000,
	AI_CHANGE_ATTACK_POISITION_TIME_FAR = 1000,
	AI_CHANGE_ATTACK_POISITION_DISTANCE = 100,
	SUMMON_MONSTER_COUNT = 3,
};

enum
{
	FLY_NONE,
	FLY_EXP,
	FLY_HP_MEDIUM,
	FLY_HP_BIG,
	FLY_SP_SMALL,
	FLY_SP_MEDIUM,
	FLY_SP_BIG,
	FLY_FIREWORK1,
	FLY_FIREWORK2,
	FLY_FIREWORK3,
	FLY_FIREWORK4,
	FLY_FIREWORK5,
	FLY_FIREWORK6,
	FLY_FIREWORK_CHRISTMAS,
	FLY_CHAIN_LIGHTNING,
	FLY_HP_SMALL,
	FLY_SKILL_MUYEONG,
#ifdef ENABLE_QUIVER_SYSTEM
	FLY_QUIVER_ATTACK_NORMAL,
#endif
#ifdef ENABLE_PROTO_RENEWAL
	FLY_ILGWANGPYO_NORMAL,
	FLY_ILGWANGPYO_MASTER,
	FLY_ILGWANGPYO_GRAND_MASTER,
	FLY_ILGWANGPYO_PERFECT_MASTER,
# ifdef ENABLE_YOHARA_SYSTEM
	FLY_CONQUEROR_EXP,
# endif
#endif
};

enum class EDamageType
{
	DAMAGE_TYPE_NONE,
	DAMAGE_TYPE_NORMAL,
	DAMAGE_TYPE_NORMAL_RANGE,
	DAMAGE_TYPE_MELEE,
	DAMAGE_TYPE_RANGE,
	DAMAGE_TYPE_FIRE,
	DAMAGE_TYPE_ICE,
	DAMAGE_TYPE_ELEC,
	DAMAGE_TYPE_MAGIC,
	DAMAGE_TYPE_POISON,
	DAMAGE_TYPE_SPECIAL,
#ifdef ENABLE_WOLFMAN_CHARACTER
	DAMAGE_TYPE_BLEEDING,
#endif
};

enum DamageFlag
{
	DAMAGE_NORMAL	= (1 << 0),
	DAMAGE_POISON	= (1 << 1),
	DAMAGE_DODGE	= (1 << 2),
	DAMAGE_BLOCK	= (1 << 3),
	DAMAGE_PENETRATE= (1 << 4),
	DAMAGE_CRITICAL = (1 << 5),
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON)
	DAMAGE_BLEEDING	= (1 << 6),
#endif
	DAMAGE_FIRE		= (1 << 7),
};

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
	POINT_DEF_GRADE,				// 16	- APPLY_DEF_GRADE,	83
	POINT_ATT_SPEED,				// 17	- APPLY_ATT_SPEED,	7
	POINT_ATT_GRADE,				// 18	- 
	POINT_MOV_SPEED,				// 19	- APPLY_MOV_SPEED,	8
	POINT_CLIENT_DEF_GRADE,			// 20	- 
	POINT_CASTING_SPEED,			// 21	- APPLY_CAST_SPEED,	9
	POINT_MAGIC_ATT_GRADE,			// 22	- 
	POINT_MAGIC_DEF_GRADE,			// 23	- 
	POINT_EMPIRE_POINT,				// 24	- 
	POINT_LEVEL_STEP,				// 25	- 
	POINT_STAT,						// 26	- 
	POINT_SUB_SKILL,				// 27	- 
	POINT_SKILL,					// 28	- APPLY_SKILL,	51
	POINT_WEAPON_MIN,				// 29	- 
	POINT_WEAPON_MAX,				// 30	- 
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
	POINT_ATTBONUS_TREE,			// 58	- 
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
	POINT_KILL_HP_RECOVERY,			// 87	- APPLY_KILL_HP_RECOVER,	47
	POINT_IMMUNE_STUN,				// 88	- APPLY_IMMUNE_STUN,	48
	POINT_IMMUNE_SLOW,				// 89	- APPLY_IMMUNE_SLOW,	49
	POINT_IMMUNE_FALL,				// 90	- APPLY_IMMUNE_FALL,	50
	POINT_PARTY_ATTACKER_BONUS,		// 91	- 
	POINT_PARTY_TANKER_BONUS,		// 92	- 
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
	POINT_RAMADAN_CANDY_BONUS_EXP,	// 127	- 
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
	POINT_ENCHANT_ELECT,			// 145
	POINT_ENCHANT_FIRE,				// 146
	POINT_ENCHANT_ICE,				// 147
	POINT_ENCHANT_WIND,				// 148
	POINT_ENCHANT_EARTH,			// 149
	POINT_ENCHANT_DARK,				// 150
	POINT_ATTBONUS_CZ,				// 151
	POINT_ATTBONUS_SWORD,			// 152
	POINT_ATTBONUS_TWOHAND,			// 153
	POINT_ATTBONUS_DAGGER,			// 154
	POINT_ATTBONUS_BELL,			// 155
	POINT_ATTBONUS_FAN,				// 156
	POINT_ATTBONUS_BOW,				// 157
	POINT_ATTBONUS_CLAW,			// 158
	POINT_RESIST_HUMAN,				// 159
	POINT_RESIST_MOUNT_FALL,		// 160
#ifdef ENABLE_YOHARA_SYSTEM
	POINT_CONQUEROR_LEVEL			= 165, // 295
	POINT_CONQUEROR_LEVEL_STEP		= 166,
	POINT_CONQUEROR_EXP				= 167,
	POINT_CONQUEROR_NEXT_EXP		= 168,
	POINT_CONQUEROR_POINT			= 169, // 299
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
	POINT_CHEQUE					= 190,
#endif
#ifdef ENABLE_GEM_SYSTEM
	POINT_GEM						= 191,
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	POINT_MEDAL_OF_HONOR			= 192,
#endif
#ifdef ENABLE_REFINE_ELEMENT
	POINT_ATT_ELEMENTS				= 193,
#endif
#ifdef ENABLE_BATTLE_FIELD
	POINT_BATTLE_FIELD				= 194,
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	POINT_RESIST_FIST				= 205,
	POINT_SKILL_DAMAGE_SAMYEON		= 206,
	POINT_SKILL_DAMAGE_TANHWAN		= 207,
	POINT_SKILL_DAMAGE_PALBANG		= 208,
	POINT_SKILL_DAMAGE_GIGONGCHAM	= 209,
	POINT_SKILL_DAMAGE_GYOKSAN		= 210,
	POINT_SKILL_DAMAGE_GEOMPUNG		= 211,
	POINT_SKILL_DAMAGE_AMSEOP		= 212,
	POINT_SKILL_DAMAGE_GUNGSIN		= 213,
	POINT_SKILL_DAMAGE_CHARYUN		= 214,
	POINT_SKILL_DAMAGE_SANGONG		= 215,
	POINT_SKILL_DAMAGE_YEONSA		= 216,
	POINT_SKILL_DAMAGE_KWANKYEOK	= 217,
	POINT_SKILL_DAMAGE_GIGUNG		= 218,
	POINT_SKILL_DAMAGE_HWAJO		= 219,
	POINT_SKILL_DAMAGE_SWAERYUNG	= 220,
	POINT_SKILL_DAMAGE_YONGKWON		= 221,
	POINT_SKILL_DAMAGE_PABEOB		= 222,
	POINT_SKILL_DAMAGE_MARYUNG		= 223,
	POINT_SKILL_DAMAGE_HWAYEOMPOK	= 224,
	POINT_SKILL_DAMAGE_MAHWAN		= 225,
	POINT_SKILL_DAMAGE_BIPABU		= 226,
	POINT_SKILL_DAMAGE_YONGBI		= 227,
	POINT_SKILL_DAMAGE_PAERYONG		= 228,
	POINT_SKILL_DAMAGE_NOEJEON		= 229,
	POINT_SKILL_DAMAGE_BYEURAK		= 230,
	POINT_SKILL_DAMAGE_CHAIN		= 231,
	POINT_SKILL_DAMAGE_CHAYEOL		= 232,
	POINT_SKILL_DAMAGE_SALPOONG		= 233,
	POINT_SKILL_DAMAGE_GONGDAB		= 234,
	POINT_SKILL_DAMAGE_PASWAE		= 235,
	POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE	= 236,
	POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE		= 237,
	POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE	= 238,
	POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE		= 239,
	POINT_HIT_BUFF_ENCHANT_FIRE					= 240,
	POINT_HIT_BUFF_ENCHANT_ICE					= 241,
	POINT_HIT_BUFF_ENCHANT_ELEC					= 242,
	POINT_HIT_BUFF_ENCHANT_WIND					= 243,
	POINT_HIT_BUFF_ENCHANT_DARK					= 244,
	POINT_HIT_BUFF_ENCHANT_EARTH				= 245,
	POINT_HIT_BUFF_RESIST_FIRE					= 246,
	POINT_HIT_BUFF_RESIST_ICE					= 247,
	POINT_HIT_BUFF_RESIST_ELEC					= 248,
	POINT_HIT_BUFF_RESIST_WIND					= 249,
	POINT_HIT_BUFF_RESIST_DARK					= 250,
	POINT_HIT_BUFF_RESIST_EARTH					= 251,
	POINT_USE_SKILL_CHEONGRANG_MOV_SPEED		= 252,
	POINT_USE_SKILL_CHEONGRANG_CASTING_SPEED	= 253,
	POINT_USE_SKILL_CHAYEOL_CRITICAL_PCT		= 254,
	POINT_USE_SKILL_SANGONG_ATT_GRADE_BONUS		= 255,
	POINT_USE_SKILL_GIGUNG_ATT_GRADE_BONUS		= 256,
	POINT_USE_SKILL_JEOKRANG_DEF_BONUS			= 257,
	POINT_USE_SKILL_GWIGEOM_DEF_BONUS			= 258,
	POINT_USE_SKILL_TERROR_ATT_GRADE_BONUS		= 259,
	POINT_USE_SKILL_MUYEONG_ATT_GRADE_BONUS		= 260,
	POINT_USE_SKILL_MANASHILED_CASTING_SPEED	= 261,
	POINT_USE_SKILL_HOSIN_DEF_BONUS				= 262,
	POINT_USE_SKILL_GICHEON_ATT_GRADE_BONUS		= 263,
	POINT_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS	= 264,
	POINT_USE_SKILL_JEUNGRYEOK_DEF_BONUS		= 265,
	POINT_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS		= 266,
	POINT_USE_SKILL_CHUNKEON_CASTING_SPEED		= 267,
	POINT_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS		= 268,
	POINT_SKILL_DURATION_INCREASE_EUNHYUNG		= 269,
	POINT_SKILL_DURATION_INCREASE_GYEONGGONG	= 270,
	POINT_SKILL_DURATION_INCREASE_GEOMKYUNG		= 271,
	POINT_SKILL_DURATION_INCREASE_JEOKRANG		= 272,
	POINT_USE_SKILL_PALBANG_HP_ABSORB			= 273,
	POINT_USE_SKILL_AMSEOP_HP_ABSORB			= 274,
	POINT_USE_SKILL_YEONSA_HP_ABSORB			= 275,
	POINT_USE_SKILL_YONGBI_HP_ABSORB			= 276,
	POINT_USE_SKILL_CHAIN_HP_ABSORB				= 277,
	POINT_USE_SKILL_PASWAE_SP_ABSORB			= 278,
	POINT_USE_SKILL_GIGONGCHAM_STUN				= 279,
	POINT_USE_SKILL_CHARYUN_STUN				= 280,
	POINT_USE_SKILL_PABEOB_STUN					= 281,
	POINT_USE_SKILL_MAHWAN_STUN					= 282,
	POINT_USE_SKILL_GONGDAB_STUN				= 283,
	POINT_USE_SKILL_SAMYEON_STUN				= 284,
	POINT_USE_SKILL_GYOKSAN_KNOCKBACK			= 285,
	POINT_USE_SKILL_SEOMJEON_KNOCKBACK			= 286,
	POINT_USE_SKILL_SWAERYUNG_KNOCKBACK			= 287,
	POINT_USE_SKILL_HWAYEOMPOK_KNOCKBACK		= 288,
	POINT_USE_SKILL_GONGDAB_KNOCKBACK			= 289,
	POINT_USE_SKILL_KWANKYEOK_KNOCKBACK			= 290,
	POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER	= 291,
	POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER	= 292,
	POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER	= 293,
	POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER	= 294,
	POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER	= 295,
	POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER	= 296,
	POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER		= 297,
	POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER	= 298,
	POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER	= 299,
	POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER		= 300,
	POINT_ATTBONUS_STONE						= 301,
	POINT_DAMAGE_HP_RECOVERY					= 302,
	POINT_DAMAGE_SP_RECOVERY					= 303,
	POINT_ALIGNMENT_DAMAGE_BONUS				= 304,
	POINT_NORMAL_DAMAGE_GUARD					= 305,
	POINT_MORE_THEN_HP90_DAMAGE_REDUCE			= 306,
	POINT_USE_SKILL_TUSOK_HP_ABSORB				= 307,
	POINT_USE_SKILL_PAERYONG_HP_ABSORB			= 308,
	POINT_USE_SKILL_BYEURAK_HP_ABSORB			= 309,
	POINT_FIRST_ATTRIBUTE_BONUS					= 310,
	POINT_SECOND_ATTRIBUTE_BONUS				= 311,
	POINT_THIRD_ATTRIBUTE_BONUS					= 312,
	POINT_FOURTH_ATTRIBUTE_BONUS				= 313,
	POINT_FIFTH_ATTRIBUTE_BONUS					= 314,
	POINT_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER	= 315,
	POINT_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER	= 316,
	POINT_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER	= 317,
	POINT_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER	= 318,
	POINT_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER	= 319,
	POINT_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER	= 320,
	POINT_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER		= 321,
	POINT_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER	= 322,
	POINT_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER	= 323,
	POINT_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER		= 324,
	POINT_USE_SKILL_CHAYEOL_HP_ABSORB			= 325,
	POINT_SUNGMA_STR							= 326,
	POINT_SUNGMA_HP								= 327,
	POINT_SUNGMA_MOVE							= 328,
	POINT_SUNGMA_IMMUNE							= 329,
	POINT_HIT_PCT								= 330,
	POINT_RANDOM								= 331,
	POINT_ATTBONUS_PER_HUMAN					= 332,
	POINT_ATTBONUS_PER_ANIMAL					= 333,
	POINT_ATTBONUS_PER_ORC						= 334,
	POINT_ATTBONUS_PER_MILGYO					= 335,
	POINT_ATTBONUS_PER_UNDEAD					= 336,
	POINT_ATTBONUS_PER_DEVIL					= 337,
	POINT_ENCHANT_PER_ELECT						= 338,
	POINT_ENCHANT_PER_FIRE						= 339,
	POINT_ENCHANT_PER_ICE						= 340,
	POINT_ENCHANT_PER_WIND						= 341,
	POINT_ENCHANT_PER_EARTH						= 342,
	POINT_ENCHANT_PER_DARK						= 343,
	POINT_ATTBONUS_PER_CZ						= 344,
	POINT_ATTBONUS_PER_INSECT					= 345,
	POINT_ATTBONUS_PER_DESERT					= 346,
	POINT_ATTBONUS_PER_STONE					= 347,
	POINT_ATTBONUS_PER_MONSTER					= 348,
	POINT_RESIST_PER_HUMAN						= 349,
	POINT_RESIST_PER_ICE						= 350,
	POINT_RESIST_PER_DARK						= 351,
	POINT_RESIST_PER_EARTH						= 352,
	POINT_RESIST_PER_FIRE						= 353,
	POINT_RESIST_PER_ELEC						= 354,
	POINT_RESIST_PER_MAGIC						= 355,
	POINT_RESIST_PER_WIND						= 356,
	POINT_HIT_BUFF_SUNGMA_STR					= 357,
	POINT_HIT_BUFF_SUNGMA_MOVE					= 358,
	POINT_HIT_BUFF_SUNGMA_HP					= 359,
	POINT_HIT_BUFF_SUNGMA_IMMUNE				= 360,
	POINT_MOUNT_MELEE_MAGIC_ATTBONUS_PER		= 361,
	POINT_DISMOUNT_MOVE_SPEED_BONUS_PER			= 362,
	POINT_HIT_AUTO_HP_RECOVERY					= 363,
	POINT_HIT_AUTO_SP_RECOVERY					= 364,
	POINT_USE_SKILL_COOLTIME_DECREASE_ALL		= 365,
	POINT_HIT_STONE_ATTBONUS_STONE				= 366,
	POINT_HIT_STONE_DEF_GRADE_BONUS				= 367,
	POINT_KILL_BOSS_ITEM_BONUS					= 368,
	POINT_MOB_HIT_MOB_AGGRESSIVE				= 369,
	POINT_NO_DEATH_AND_HP_RECOVERY30			= 370,
	POINT_AUTO_PICKUP							= 371,
	POINT_MOUNT_NO_KNOCKBACK					= 372,
	POINT_SUNGMA_PER_STR						= 373,
	POINT_SUNGMA_PER_HP							= 374,
	POINT_SUNGMA_PER_MOVE						= 375,
	POINT_SUNGMA_PER_IMMUNE						= 376,
	POINT_IMMUNE_POISON100						= 377,
	POINT_IMMUNE_BLEEDING100					= 378,
	POINT_MONSTER_DEFEND_BONUS					= 379,
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

	//POINT_MAX_NUM = 129	common/length.h
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

enum EPositions
{
	POS_DEAD,
	POS_SLEEPING,
	POS_RESTING,
	POS_SITTING,
	POS_FISHING,
	POS_FIGHTING,
	POS_MOUNTING,
	POS_STANDING
};

enum EBlockAction
{
	BLOCK_EXCHANGE			= (1 << 0),
	BLOCK_PARTY_INVITE		= (1 << 1),
	BLOCK_GUILD_INVITE		= (1 << 2),
	BLOCK_WHISPER			= (1 << 3),
	BLOCK_MESSENGER_INVITE	= (1 << 4),
	BLOCK_PARTY_REQUEST		= (1 << 5),
};

// <Factor> Dynamically evaluated CHARACTER* equivalent.
// Referring to SCharDeadEventInfo.
struct DynamicCharacterPtr
{
	DynamicCharacterPtr() : is_pc(false), id(0) {}
	DynamicCharacterPtr(const DynamicCharacterPtr& o)
		: is_pc(o.is_pc), id(o.id) {}

	// Returns the LPCHARACTER found in CHARACTER_MANAGER.
	LPCHARACTER Get() const;
	// Clears the current settings.
	void Reset() {
		is_pc = false;
		id = 0;
	}

	// Basic assignment operator.
	DynamicCharacterPtr& operator=(const DynamicCharacterPtr& rhs) {
		is_pc = rhs.is_pc;
		id = rhs.id;
		return *this;
	}
	// Supports assignment with LPCHARACTER type.
	DynamicCharacterPtr& operator=(LPCHARACTER character);
	// Supports type casting to LPCHARACTER.
	operator LPCHARACTER() const {
		return Get();
	}

	bool is_pc;
	uint32_t id;
};

/* Data to store */
typedef struct character_point
{
	long points[POINT_MAX_NUM];
	uint8_t job;
	uint8_t voice;
	uint8_t level;
	uint32_t exp;
	long gold;
#ifdef ENABLE_CHEQUE_SYSTEM
	int cheque;
#endif
#ifdef ENABLE_GEM_SYSTEM
	uint32_t gem_point;
#endif
	int hp;
	int sp;
	int iRandomHP;
	int iRandomSP;
#ifdef ENABLE_HORSE_APPEARANCE
	unsigned int horse_appearance;
#endif
	int stamina;
	uint8_t skill_group;
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
	uint32_t conqueror_exp;
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	int soul;
	int soulre;
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
} CHARACTER_POINT;

//@fixme199 BEGIN
/*struct PlayerSlotT
{
	// Inventory
	std::array<LPITEM, INVENTORY_SLOT_COUNT> pInventoryItems;
	std::array<uint16_t, INVENTORY_SLOT_COUNT > wInventoryItemGrid;
	// Equipment
	std::array<LPITEM, EQUIPMENT_SLOT_COUNT> pEquipmentItems;
	std::array<uint8_t, EQUIPMENT_SLOT_COUNT > bEquipmentItemGrid;
	// Belt
	std::array<LPITEM, BELT_INVENTORY_SLOT_COUNT> pBeltItems;
	std::array<uint8_t, BELT_INVENTORY_SLOT_COUNT > bBeltGrid;
	// Dragonsoul
	std::array<LPITEM, DRAGON_SOUL_INVENTORY_MAX_NUM> pDSItems;
	std::array<uint16_t, DRAGON_SOUL_INVENTORY_MAX_NUM > wDSItemGrid;
	// Offlineshop
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	std::array<LPITEM, SHOP_INVENTORY_MAX_NUM> pShopItems;
	std::array<uint8_t, SHOP_INVENTORY_MAX_NUM > bShopItemGrid;
#endif
	// Switchbot
#ifdef ENABLE_SWITCHBOT
	std::array<LPITEM, SWITCHBOT_SLOT_COUNT > pSwitchbotItems;
#endif
	// Extra EQ-Page
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	std::array<LPITEM, ADDITIONAL_EQUIPMENT_MAX_SLOT > pAdditionalEquipmentItem_1;
#endif
#ifndef ENABLE_CUBE_RENEWAL
	std::array<LPITEM, CUBE_MAX_NUM > pCubeItems;
#endif
	// Quickslot
	//std::array<TQuickSlot, QUICKSLOT_MAX_NUM> pQuickslot;
};*/
//@fixme199 END

/* Unsaved character data */
typedef struct character_point_instant
{
	long points[POINT_MAX_NUM];

	float fRot;

	int iMaxHP;
	int iMaxSP;

#ifdef ENABLE_YOHARA_SYSTEM
	int iMove;
#endif

	long position;

	long instant_flag;
	uint32_t dwAIFlag;
	uint32_t dwImmuneFlag;
	uint32_t dwLastShoutPulse;

	uint32_t parts[PART_MAX_NUM]; //@fixme479 - uint16_t -> uint32_t

	// Oh... I can't do it without really swearing.
	// Char manages the grid as a uint8_t array for inventory, and the CGrid for exchange or cube manages the grid.
	// If you have created a grid, I mean use a grid!!!
	// I made the same dragon soul stone inventory and made it wrong
	LPITEM pInventoryItems[INVENTORY_SLOT_COUNT];
	uint16_t wInventoryItemGrid[INVENTORY_SLOT_COUNT];

	LPITEM pEquipmentItems[EQUIPMENT_SLOT_COUNT];
	uint8_t bEquipmentItemGrid[EQUIPMENT_SLOT_COUNT];

	LPITEM pBeltItems[BELT_INVENTORY_SLOT_COUNT];
	uint8_t bBeltGrid[BELT_INVENTORY_SLOT_COUNT];

	// Dragon Soul Inventory.
	LPITEM pDSItems[DRAGON_SOUL_INVENTORY_MAX_NUM];
	uint16_t wDSItemGrid[DRAGON_SOUL_INVENTORY_MAX_NUM];

#ifdef ENABLE_ATTR_6TH_7TH
	LPITEM pNPCStorageItems;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	LPITEM pShopItems[SHOP_INVENTORY_MAX_NUM];
	uint8_t bShopItemGrid[SHOP_INVENTORY_MAX_NUM];
#endif

#ifdef ENABLE_SWITCHBOT
	LPITEM pSwitchbotItems[SWITCHBOT_SLOT_COUNT];
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	LPITEM pAdditionalEquipmentItem_1[ADDITIONAL_EQUIPMENT_MAX_SLOT];
#endif

	// by mhh
#ifndef ENABLE_CUBE_RENEWAL
	LPITEM pCubeItems[CUBE_MAX_NUM];
#endif
	LPCHARACTER pCubeNpc;

	LPCHARACTER battle_victim;

	uint8_t gm_level;

	uint8_t bBasePart; // Casual clothes number

	int iMaxStamina;

	uint8_t bBlockMode;

	int iDragonSoulActiveDeck;
	LPENTITY m_pDragonSoulRefineWindowOpener;
#ifdef ENABLE_AURA_SYSTEM
	LPENTITY m_pAuraRefineWindowOpener;
#endif
} CHARACTER_POINT_INSTANT;

#define TRIGGERPARAM LPCHARACTER ch, LPCHARACTER causer

typedef struct trigger
{
	uint8_t type;
	int (*func) (TRIGGERPARAM);
	long value;
} TRIGGER;

class CTrigger
{
public:
	CTrigger() noexcept : bType(0), pFunc(nullptr)
	{
	}

	uint8_t bType;
	int (*pFunc) (TRIGGERPARAM);
};

EVENTINFO(char_event_info)
{
	DynamicCharacterPtr ch;
};

#ifdef ENABLE_NINETH_SKILL
EVENTINFO(cheonun_event_info)
{
	DynamicCharacterPtr ch;
	uint8_t bShieldChance;
	uint8_t bShieldDur;
};
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
EVENTINFO(hitbuff_event_info)
{
	DynamicCharacterPtr ch;
};
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
EVENTINFO(elemental_world_event_info)
{
	DynamicCharacterPtr ch;
};
#endif

#ifdef ENABLE_FISHING_RENEWAL
EVENTINFO(fishingnew_event_info)
{
	uint32_t pid, vnum, chance, sec;
	fishingnew_event_info() : pid(0), vnum(0), chance(0), sec(0) {}
};
#endif

typedef std::map<VID, size_t> target_map;
struct TSkillUseInfo
{
	int iHitCount;
	int iMaxHitCount;
	int iSplashCount;
	uint32_t dwNextSkillUsableTime;
	int iRange;
	bool bUsed;
#ifdef ENABLE_CHECK_SKILL_HACK
	bool bSkillCD;
	uint32_t dwHitCount;
#endif
	uint32_t dwVID;
	bool isGrandMaster;

	target_map TargetVIDMap;

	TSkillUseInfo() noexcept :
		iHitCount(0), iMaxHitCount(0), iSplashCount(0), dwNextSkillUsableTime(0), iRange(0), bUsed(false),
#ifdef ENABLE_CHECK_SKILL_HACK
		bSkillCD(false), dwHitCount(0),
#endif
		dwVID(0), isGrandMaster(false)
	{}

	bool HitOnce(uint32_t dwVnum = 0);
#ifdef ENABLE_CHECK_SKILL_HACK
	bool IsSkillCooldown(uint32_t dwVnum, float fSkillPower);
#endif

	bool UseSkill(bool isGrandMaster, uint32_t vid, uint32_t dwCooltime, int splashcount = 1, int hitcount = -1, int range = -1);
	uint32_t GetMainTargetVID() const noexcept { return dwVID; }
	void SetMainTargetVID(uint32_t vid) noexcept { dwVID = vid; }
	void ResetHitCount() noexcept { if (iSplashCount) { iHitCount = iMaxHitCount; iSplashCount--; } }
};

typedef struct packet_party_update TPacketGCPartyUpdate;
class CExchange;
class CSkillProto;
class CParty;
class CDungeon;
class CWarMap;
class CAffect;
class CGuild;
class CSafebox;
class CArena;
class CShop;
typedef class CShop* LPSHOP;

class CMob;
class CMobInstance;
typedef struct SMobSkillInfo TMobSkillInfo;

//SKILL_POWER_BY_LEVEL
extern int GetSkillPowerByLevelFromType(int job, int skillgroup, int skilllevel);
//END_SKILL_POWER_BY_LEVEL

namespace marriage
{
	class WeddingMap;
}
enum e_overtime
{
	OT_NONE,
	OT_3HOUR,
	OT_5HOUR,
};

class CHARACTER : public CEntity, public CFSM, public CHorseRider
{
protected:
	//////////////////////////////////////////////////////////////////////////////////
	// Entity related
	virtual void EncodeInsertPacket(LPENTITY entity);
	virtual void EncodeRemovePacket(LPENTITY entity);
	//////////////////////////////////////////////////////////////////////////////////

public:
	LPCHARACTER FindCharacterInView(const char* name, bool bFindPCOnly);
	void UpdatePacket();
	bool IsRestricted(); //@fixme500

	//////////////////////////////////////////////////////////////////////////////////
	// FSM (Finite State Machine) related
protected:
	CStateTemplate<CHARACTER> m_stateMove;
	CStateTemplate<CHARACTER> m_stateBattle;
	CStateTemplate<CHARACTER> m_stateIdle;

public:
	virtual void StateMove();
	virtual void StateBattle();
	virtual void StateIdle();
	virtual void StateFlag();
	virtual void StateFlagBase();
	void StateHorse();

protected:
	// STATE_IDLE_REFACTORING
	void __StateIdle_Monster();
	void __StateIdle_Stone();
	void __StateIdle_NPC();
	// END_OF_STATE_IDLE_REFACTORING

public:
	uint32_t GetAIFlag() const noexcept { return m_pointsInstant.dwAIFlag; }

	void SetAggressive() noexcept;
	bool IsAggressive() const noexcept;

	void SetCoward() noexcept;
	bool IsCoward() const noexcept;
	void CowardEscape();

	void SetNoAttackShinsu() noexcept;
	bool IsNoAttackShinsu() const noexcept;

	void SetNoAttackChunjo() noexcept;
	bool IsNoAttackChunjo() const noexcept;

	void SetNoAttackJinno() noexcept;
	bool IsNoAttackJinno() const noexcept;

	void SetAttackMob() noexcept;
	bool IsAttackMob() const noexcept;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void SetNoMoveFlag() noexcept { SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE); }
	bool IsNoMoveFlag() const noexcept { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE); }
#endif

	virtual void BeginStateEmpty();
	virtual void EndStateEmpty() noexcept {}

	void RestartAtSamePos();

protected:
	uint32_t m_dwStateDuration;
	//////////////////////////////////////////////////////////////////////////////////

public:
	CHARACTER();
	virtual ~CHARACTER();

	void Create(const char* c_pszName, uint32_t vid, bool isPC);
	void Destroy();

	void Disconnect(const char* c_pszReason);

protected:
	void Initialize();

	//////////////////////////////////////////////////////////////////////////////////
	// Basic Points

public:
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
# ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	void SetAlignment(int alignment) noexcept { m_iAlignment = alignment; };
# endif
	void SetPlayerID(uint32_t pid) noexcept { m_dwPlayerID = pid; }
#endif
	uint32_t GetPlayerID() const noexcept { return m_dwPlayerID; }

	void SetPlayerProto(const TPlayerTable* table);
	void CreatePlayerProto(TPlayerTable& tab); // use when saving

	void SetProto(const CMob* c_pkMob);
	uint16_t GetRaceNum() const noexcept;
	uint16_t GetPlayerRace() const;

	void Save(); // DelayedSave
	void SaveReal(); // actual storage
	void FlushDelayedSaveItem();
#ifdef ENABLE_VIP_SYSTEM
	void RefreshVip();
#endif
	void SetAffectPotion(LPITEM item); //@custom001

	const char* GetName() const;
	std::string GetStringName() const;
	const char* GetProtoName() const;

	const VID& GetVID() const noexcept { return m_vid; }

	void SetName(const std::string& name) { m_stName = name; }

	void SetRace(uint8_t race);
	bool ChangeSex();

	uint32_t GetAID() const;
	int GetChangeEmpireCount() const;
	void SetChangeEmpireCount();
	int ChangeEmpire(uint8_t empire);

	uint8_t GetJob() const;
	uint8_t GetCharType() const noexcept;

	bool IsPC() const { return GetDesc() ? true : false; }
	bool IsNPC() const noexcept { return m_bCharType != CHAR_TYPE_PC; }
	bool IsMonster() const noexcept { return m_bCharType == CHAR_TYPE_MONSTER; }
	bool IsStone() const noexcept { return m_bCharType == CHAR_TYPE_STONE; }
	bool IsDoor() const noexcept { return m_bCharType == CHAR_TYPE_DOOR; }
	bool IsBuilding() const noexcept { return m_bCharType == CHAR_TYPE_BUILDING; }
	bool IsWarp() const noexcept { return m_bCharType == CHAR_TYPE_WARP; }
	bool IsGoto() const noexcept { return m_bCharType == CHAR_TYPE_GOTO; }
#ifdef ENABLE_PROTO_RENEWAL
	bool IsHorse() const noexcept { return m_bCharType == CHAR_TYPE_HORSE; }
	bool IsPetPay() const noexcept { return m_bCharType == CHAR_TYPE_PET_PAY; }
	bool IsPetType() const noexcept { return m_bCharType == CHAR_TYPE_PET; }
#endif
#if defined (ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PROTO_RENEWAL)
	bool IsShop() const noexcept { return m_bCharType == CHAR_TYPE_SHOP; }
#else
	bool IsShop() const noexcept { return GetRaceNum() == 30000; };
#endif
#ifdef ENABLE_MONSTER_BACK
	bool IsMonsterBackVnum() noexcept
	{
# ifdef ENABLE_10TH_EVENT
		return (GetRaceNum() == ATTENDANCE_VNUM_SHADOW_WARRIOR);
# else
		return ((GetRaceNum() >= ATTENDANCE_VNUM_BERAN) && (GetRaceNum() <= ATTENDANCE_VNUM_RED_DRAGON));
# endif
	}
#endif

	uint32_t GetLastShoutPulse() const noexcept { return m_pointsInstant.dwLastShoutPulse; }
	void SetLastShoutPulse(uint32_t pulse) noexcept { m_pointsInstant.dwLastShoutPulse = pulse; }
	int GetLevel() const noexcept { return m_points.level; }
	void SetLevel(uint8_t level);

	uint8_t GetGMLevel() const noexcept;
	bool IsGM() const noexcept;
#ifdef ENABLE_VIP_SYSTEM
	BOOL IsVIP() const;
#endif
	void SetGMLevel();
#ifdef ENABLE_GM_BLOCK
	bool GetStaffPermissions() const noexcept;
#endif

	//@custom025
	void StripPowers() { m_powersStripped = true; }
	bool IsStrippedPowers() const { return m_powersStripped; }
	//@end_custom025

	uint32_t GetExp() const noexcept { return m_points.exp; }
	void SetExp(uint32_t exp) noexcept { m_points.exp = exp; }
	uint32_t GetNextExp() const noexcept;
	LPCHARACTER DistributeExp(); // Returns the person with the most hits.

	void DistributeHP(LPCHARACTER pkKiller);
	void DistributeSP(LPCHARACTER pkKiller, int iMethod = 0);

#ifdef ENABLE_KILL_EVENT_FIX
	LPCHARACTER GetMostAttacked();
#endif

	void SetPosition(int pos);
	bool IsPosition(int pos) const noexcept { return m_pointsInstant.position == pos ? true : false; }
	int GetPosition() const noexcept { return m_pointsInstant.position; }

	void SetPart(uint8_t bPartPos, uint32_t dwVal) noexcept; //@fixme479 - uint16_t -> uint32_t
	uint32_t GetPart(uint8_t bPartPos) const; //@fixme479 - uint16_t -> uint32_t
	uint32_t GetOriginalPart(uint8_t bPartPos) const; //@fixme479 - uint16_t -> uint32_t

#ifdef ENABLE_HORSE_APPEARANCE
	uint32_t GetHorseAppearance() { return m_points.horse_appearance; }
	void SetHorseAppearance(uint32_t vnum) { m_points.horse_appearance = vnum; }
#endif

	void SetHP(int hp) noexcept { m_points.hp = hp; }
	int GetHP() const noexcept { return m_points.hp; }

	void SetSP(int sp) noexcept { m_points.sp = sp; }
	int GetSP() const noexcept { return m_points.sp; }

	void SetStamina(int stamina) noexcept { m_points.stamina = stamina; }
	int GetStamina() const noexcept { return m_points.stamina; }

	void SetMaxHP(int iVal) noexcept { m_pointsInstant.iMaxHP = iVal; }
	int GetMaxHP() const noexcept { return m_pointsInstant.iMaxHP; }

	void SetMaxSP(int iVal) noexcept { m_pointsInstant.iMaxSP = iVal; }
	int GetMaxSP() const noexcept { return m_pointsInstant.iMaxSP; }

	void SetMaxStamina(int iVal) noexcept { m_pointsInstant.iMaxStamina = iVal; }
	int GetMaxStamina() const noexcept { return m_pointsInstant.iMaxStamina; }

	void SetRandomHP(int v) noexcept { m_points.iRandomHP = v; }
	void SetRandomSP(int v) noexcept { m_points.iRandomSP = v; }

	int GetRandomHP() const noexcept { return m_points.iRandomHP; }
	int GetRandomSP() const noexcept { return m_points.iRandomSP; }

	int GetHPPct() const noexcept;

	void SetRealPoint(uint16_t idx, int val) noexcept; //@fixme532
	int GetRealPoint(uint16_t idx) const noexcept; //@fixme532

	void SetPoint(uint16_t idx, int val); //@fixme532
	int GetPoint(uint16_t idx) const; //@fixme532
	int GetLimitPoint(uint16_t idx) const; //@fixme532
	int GetPolymorphPoint(uint16_t idx) const; //@fixme532

	const TMobTable& GetMobTable() const noexcept;
	uint8_t GetMobRank() const noexcept;
#ifdef ENABLE_NEWSTUFF
	uint8_t GetMobType() const noexcept;
#endif
	uint8_t GetMobBattleType() const noexcept;
	uint8_t GetMobSize() const noexcept;
	uint32_t GetMobDamageMin() const noexcept;
	uint32_t GetMobDamageMax() const noexcept;
	uint16_t GetMobAttackRange() const;
#if defined(ENABLE_PROTO_RENEWAL) && defined(ENABLE_NEW_DISTANCE_CALC)
	float GetMonsterHitRange() const;
#endif
	uint32_t GetMobDropItemVnum() const noexcept;
	float GetMobDamageMultiply() const noexcept;
#if defined(ENABLE_PROTO_RENEWAL) && defined(ENABLE_ELEMENT_ADD)
	uint8_t GetMobAttElement(uint8_t bElement) const noexcept;
#endif
#ifdef ENABLE_MOB_SCALE
	uint8_t GetMobScale() const;
#endif
	uint8_t GetMobLevel() const;

	// NEWAI
	bool IsBerserker() const noexcept;
	bool IsBerserk() const noexcept;
	void SetBerserk(bool mode) noexcept;

	bool IsStoneSkinner() const noexcept;

	bool IsGodSpeeder() const noexcept;
	bool IsGodSpeed() const noexcept;
	void SetGodSpeed(bool mode);

	bool IsDeathBlower() const noexcept;
	bool IsDeathBlow() const noexcept;

	bool IsReviver() const noexcept;
	bool HasReviverInParty() const;
	bool IsRevive() const noexcept;
	void SetRevive(bool mode) noexcept;
	// NEWAI END

	bool IsRaceFlag(uint32_t dwBit) const noexcept;
	bool IsSummonMonster() const noexcept;
	uint32_t GetSummonVnum() const noexcept;

	// NEW RESPAWN SETTINGS
	//@custom014
	int m_newSummonInterval;
	int m_lastSummonTime;
	bool CanSummonMonster() const;
	void MarkSummonedMonster();
	//@end_custom014
	// RESPWN END

	uint32_t GetPolymorphItemVnum() const noexcept;
	uint32_t GetMonsterDrainSPPoint() const noexcept;

	void MainCharacterPacket(); // Send me as the main character.

	void ComputePoints();
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	void ComputeBattlePoints(bool equip_new = true);
#else
	void ComputeBattlePoints();
#endif

	void PointChange(uint16_t type, int amount, bool bAmount = false, bool bBroadcast = false); //@fixme532
	void PointsPacket();
	void ApplyPoint(uint16_t wApplyType, int iVal); //@fixme532
	void CheckMaximumPoints(); // Check if the current value of HP, SP, etc. is higher than the maximum value, and if it is higher, lower it.

	bool Show(long lMapIndex, long x, long y, long z = LONG_MAX, bool bShowSpawnMotion = false);

	void Sitdown(int is_ground);
	void Standup();

	void SetRotation(float fRot) noexcept;
	void SetRotationToXY(long x, long y);
	float GetRotation() const noexcept { return m_pointsInstant.fRot; }

	void MotionPacketEncode(uint8_t motion, const LPCHARACTER& victim, TPacketGCMotion * packet);
	void Motion(uint8_t motion, const LPCHARACTER& victim = nullptr);

	void ChatPacket(uint8_t type, const char* format, ...);

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#ifdef ENABLE_EMPIRE_FLAG
	void ChatPacket(uint8_t bType, uint8_t bEmpire, uint8_t bLanguage, bool bFlag = false, const char* c_szFormat = "", ...);
#else
	void ChatPacket(uint8_t bType, uint8_t bEmpire, uint8_t bLanguage, const char* c_szFormat, ...);
#endif
#endif

	void MonsterChat(uint8_t bMonsterChatType);
	void SendGreetMessage();

	void ResetPoint(int iLv);

	void SetBlockMode(uint8_t bFlag);
	void SetBlockModeForce(uint8_t bFlag);
	bool IsBlockMode(uint8_t bFlag) const noexcept { return (m_pointsInstant.bBlockMode & bFlag) ? true : false; }

	bool IsPolymorphed() const noexcept { return m_dwPolymorphRace ? m_dwPolymorphRace : 0; }
	bool IsPolyMaintainStat() const noexcept { return m_bPolyMaintainStat; } // A polymorph that retains its previous stats.
	void SetPolymorph(uint32_t dwRaceNum, bool bMaintainStat = false);
	uint32_t GetPolymorphVnum() const { return IsPolymorphed(); }
	int GetPolymorphPower() const;

	// FISING
#ifdef ENABLE_CHECK_VALID_FISH_POSITION
	bool IsNearWater() const;
#endif
	void fishing();
	void fishing_take();
	// END_OF_FISHING

	// MINING
	void mining(LPCHARACTER chLoad);
	void mining_cancel();
	void mining_take();
	// END_OF_MINING

	void ResetPlayTime(uint32_t dwTimeRemain = 0) noexcept;

	void CreateFly(uint8_t bType, LPCHARACTER pkVictim);

	void ResetChatCounter() noexcept;
	uint8_t IncreaseChatCounter() noexcept;
	uint8_t GetChatCounter() const noexcept;

#ifdef ENABLE_GROWTH_PET_SYSTEM
	void EvolvePetRace(uint32_t dwRaceNum);
	void SendPetLevelUpEffect(int vid, int value);
#endif

#ifdef ENABLE_SOUL_SYSTEM
	void UseSoulAttack(uint8_t bSoulType);
	bool IsActivatedRedSoul();
	bool IsActivatedBlueSoul();
	bool DoRefineSoul(LPITEM item);
#endif

#ifdef ENABLE_YOHARA_SYSTEM
public:
	int GetConquerorLevel() const noexcept { return m_points.conqueror_level; }
	void SetConquerorLevel(uint8_t conqueror_level) noexcept { m_points.conqueror_level = conqueror_level; }

	uint32_t GetConquerorExp() const noexcept { return m_points.conqueror_exp; }
	void SetConquerorExp(uint32_t conqueror_exp) noexcept { m_points.conqueror_exp = conqueror_exp; }
	uint32_t GetConquerorNextExp() const noexcept;

	void ResetConquerorPoint(int iLv);
	bool IsSungmaMap();

	void SetMove(int iVal) noexcept { m_pointsInstant.iMove = iVal; }
	int GetMove() const noexcept { return m_pointsInstant.iMove; }

	uint8_t GetSungmaMapAttribute(uint16_t wPoint);
	bool IsConquerorItem(LPITEM pItem);
	void DetermineDropSungmaStone();
#endif

#ifdef ENABLE_SET_ITEM
	void GetSetCount(int& setID, int& setCount);
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void MarkAsShop() { m_bCharType = CHAR_TYPE_SHOP; }
#endif
	void ResetWhisperCounter() { m_bWhisperCounter = 0; }
	bool IncreaseWhisperCounter() {
		++m_bWhisperCounter;
		return m_bWhisperCounter;
	}
	uint8_t GetWhisperCounter() const { return m_bWhisperCounter; }

protected:
	uint32_t m_dwPolymorphRace = { 0 };
	bool m_bPolyMaintainStat;
	uint32_t m_dwLoginPlayTime;
	uint32_t m_dwPlayerID;
	VID m_vid;
	std::string m_stName;
	uint8_t m_bCharType;

	CHARACTER_POINT m_points;
	CHARACTER_POINT_INSTANT m_pointsInstant;
	//std::unique_ptr<PlayerSlotT> m_PlayerSlots;

	int m_iMoveCount;
	uint32_t m_dwPlayStartTime;
	uint8_t m_bAddChrState;
	bool m_bSkipSave;
	std::string m_stMobile;
	char m_szMobileAuth[5];
	uint8_t m_bChatCounter;
	uint8_t m_bWhisperCounter;
	bool m_powersStripped; //@custom025

#ifdef ENABLE_SEND_TARGET_INFO
private:
	uint32_t dwLastTargetInfoPulse;

public:
	uint32_t GetLastTargetInfoPulse() const noexcept { return dwLastTargetInfoPulse; }
	void SetLastTargetInfoPulse(uint32_t pulse) noexcept { dwLastTargetInfoPulse = pulse; }
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
protected:
	uint8_t bInventoryStage;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bSpecialInventoryStage[3];
# endif

public:
	void SetExtendInvenStage(uint8_t bStage) noexcept { bInventoryStage = bStage; }
	int GetExtendInvenStage() const noexcept { return bInventoryStage; }
	int GetExtendInvenMax() const noexcept { return INVENTORY_OPEN_PAGE_SIZE + (INVENTORY_PAGE_COLUMN * GetExtendInvenStage()); }

	void ExtendInvenRequest();
	void ExtendInvenUpgrade();
	void SendExtendInvenInfo();

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	void SetExtendSpecialInvenStage(uint8_t bStage, uint8_t bPage) { bSpecialInventoryStage[bPage] = bStage; }
	int GetExtendSpecialInvenStage(uint8_t bPage) const { return bSpecialInventoryStage[bPage]; }
	int GetExtendSpecialInvenMax(uint8_t bPage) const
	{
		int SpecialSizeMax[3] =
		{
			SPECIAL_INVENTORY_SKILLBOOK_START,	//end 225
			SPECIAL_INVENTORY_STONE_START,		//end 405
			SPECIAL_INVENTORY_MATERIAL_START	//end 585
		};

		return SPECIAL_INVENTORY_OPEN_PAGE_SIZE + SpecialSizeMax[bPage] + (SPECIAL_INVENTORY_WIDTH * GetExtendSpecialInvenStage(bPage));
	}

	uint8_t GetExtendSpecialCellWindowType(TItemPos Cell);
	uint8_t GetExtendSpecialItemWindowType(LPITEM item);

	void ExtendSpecialInvenRequest(uint8_t bWindow);
	void ExtendSpecialInvenUpgrade(uint8_t bWindow);
# endif
#endif
	// End of Basic Points

	//////////////////////////////////////////////////////////////////////////////////
	// Move & Synchronize Positions
	//////////////////////////////////////////////////////////////////////////////////
public:
	bool IsStateMove() const { return IsState((CState&)m_stateMove); }
	bool IsStateIdle() const { return IsState((CState&)m_stateIdle); }
	bool IsWalking() const noexcept { return m_bNowWalking || GetStamina() <= 0; }
	void SetWalking(bool bWalkFlag) noexcept { m_bWalking = bWalkFlag; }
	void SetNowWalking(bool bWalkFlag);
	void ResetWalking() { SetNowWalking(m_bWalking); }

	bool Goto(long x, long y); // It does not move immediately, but makes BLENDING to the target position.
	void Stop();

	bool CanMove() const; // Can you move?

	void SyncPacket();
	bool Sync(long x, long y); // Move to this method actually (there is no movement impossible due to various conditions)
	bool Move(long x, long y); // Check the condition and go through the Sync method.
	void OnMove(bool bIsAttack = false); // called when moving. It can be called outside of the Move() method.
	uint32_t GetMotionMode() const;
	float GetMoveMotionSpeed() const;
	float GetMoveSpeed() const;
	void CalculateMoveDuration();
	void SendMovePacket(uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration, uint32_t dwTime = 0, int iRot = -1);
	uint32_t GetCurrentMoveDuration() const noexcept { return m_dwMoveDuration; }
	uint32_t GetWalkStartTime() const noexcept { return m_dwWalkStartTime; }
	uint32_t GetLastMoveTime() const noexcept { return m_dwLastMoveTime; }
	uint32_t GetLastAttackTime() const noexcept { return m_dwLastAttackTime; }

	void SetLastAttacked(uint32_t time); // Stores last attacked time and location

	bool SetSyncOwner(LPCHARACTER ch, bool bRemoveFromList = true);
	bool IsSyncOwner(const LPCHARACTER& ch) const noexcept;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void RemoveShopItemByPos(uint8_t pos);
#endif

#ifdef ENABLE_12ZI
	void SetLastZodiacCzLastTime(int time) { m_dwZodiacCzLastTime = time; }
	int GetLastZodiacCzLastTime() const { return m_dwZodiacCzLastTime; }
#endif

#ifdef ENABLE_MOVE_CHANNEL
	void MoveChannel(const TRespondMoveChannel* p);
	bool WarpSet(long x, long y, long lRealMapIndex = 0, long lCustomAddr = 0, uint16_t wCustomPort = 0);
#else
	bool WarpSet(long x, long y, long lRealMapIndex = 0);
#endif
	void SetWarpLocation(long lMapIndex, long x, long y) noexcept;
	void WarpEnd();
	const PIXEL_POSITION& GetWarpPosition() const noexcept { return m_posWarp; }
	bool WarpToPID(uint32_t dwPID);

	void SaveExitLocation();
	void ExitToSavedLocation();

	void StartStaminaConsume();
	void StopStaminaConsume();
	bool IsStaminaConsume() const noexcept;
	bool IsStaminaHalfConsume() const;

	void ResetStopTime() noexcept;
	uint32_t GetStopTime() const noexcept;

protected:
	void ClearSync();

	float m_fSyncTime;
	LPCHARACTER m_pkChrSyncOwner;
	CHARACTER_LIST m_kLst_pkChrSyncOwned; // Those I'm the SyncOwner

	PIXEL_POSITION m_posDest;
	PIXEL_POSITION m_posStart;
	PIXEL_POSITION m_posWarp;
	long m_lWarpMapIndex;

	PIXEL_POSITION m_posExit;
	long m_lExitMapIndex;

	uint32_t m_dwMoveStartTime;
	uint32_t m_dwMoveDuration;

	uint32_t m_dwLastMoveTime;
	uint32_t m_dwLastAttackTime;
	uint32_t m_dwWalkStartTime;
	uint32_t m_dwStopTime;

	bool m_bWalking;
	bool m_bNowWalking;
	bool m_bStaminaConsume;
#ifdef ENABLE_DAWNMIST_DUNGEON
	uint32_t m_dwBattleStartTime;
	uint32_t m_dwLastAttackedTime;
	uint8_t m_bySpecialSkillIndex;
#endif
#ifdef ENABLE_12ZI
	int m_dwZodiacCzLastTime;
#endif
	// End

	// Quickslot related
public:
	void SyncQuickslot(uint8_t bType, uint8_t bOldPos, uint8_t bNewPos);
	int GetQuickslotPosition(uint8_t bType, uint8_t bInventoryPos);
	bool GetQuickslot(uint8_t pos, TQuickSlot** ppSlot);
	bool SetQuickslot(uint8_t pos, TQuickSlot& rSlot);
	bool DelQuickslot(uint8_t pos);
	bool SwapQuickslot(uint8_t a, uint8_t b);
	void ChainQuickslotItem(LPITEM pItem, uint8_t bType, uint8_t bOldPos);
	void MoveQuickSlotItem(uint8_t bOldType, uint8_t bOldPos, uint8_t bNewType, uint8_t bNewPos);

protected:
	TQuickSlot m_quickslot[QUICKSLOT_MAX_NUM];
#ifdef ENABLE_FISH_EVENT
	TPlayerFishEventSlot* m_fishSlots;
#endif

	////////////////////////////////////////////////////////////////////////////////////////
	// Affect
public:
	void ban(char* reason2, unsigned int myid2);
	void unban(char* name);
	void StartAffectEvent();
	void ClearAffect(bool bSave = false);
	void ComputeAffect(const CAffect* pkAff, bool bAdd);

	bool AddAffect(uint32_t dwType, uint16_t wApplyOn, long lApplyValue, uint32_t dwFlag, long lDuration, long lSPCost, bool bOverride, bool IsCube = false
#ifdef ENABLE_NINETH_SKILL
		, uint8_t bShieldDuration = 0
#endif
#ifdef ENABLE_AFFECT_RENEWAL
		, uint32_t dwExpireTIme = 0
#endif
	); //@fixme532
	void RefreshAffect();
	bool RemoveAffect(uint32_t dwType);
	bool IsAffectFlag(uint32_t dwAff) const;

	bool UpdateAffect(); // called from EVENT
	int ProcessAffect();

	void LoadAffect(uint32_t dwCount, TPacketGDAffectElement* pElements);
	void SaveAffect();

	// Is the effect loading finished?
	bool IsLoadedAffect() const noexcept { return m_bIsLoadedAffect; }

	bool IsGoodAffect(uint8_t bAffectType) const;

	void RemoveGoodAffect();
	void RemoveBadAffect();

	CAffect* FindAffect(uint32_t dwType, uint16_t wApply = APPLY_NONE) const; //@fixme532
	const std::list<CAffect*>& GetAffectContainer() const noexcept { return m_list_pkAffect; }
	bool RemoveAffect(CAffect* pkAff, bool single = true); //@fixme433
#ifdef ENABLE_BATTLE_FIELD
	void SetAffectFlag(uint32_t dwAff) { m_afAffectFlag.Set(dwAff); };
#endif

protected:
	bool m_bIsLoadedAffect;
	TAffectFlag m_afAffectFlag;
	std::list<CAffect*> m_list_pkAffect;

public:
	// PARTY_JOIN_BUG_FIX
	void SetParty(LPPARTY pkParty);
	LPPARTY GetParty() const noexcept { return m_pkParty; }

	bool RequestToParty(LPCHARACTER leader);
	void DenyToParty(LPCHARACTER member);
	void AcceptToParty(LPCHARACTER member);

	/// Invite another character to your party.
	/**
	* @param pchInvitee The character to invite. You must be able to participate in the party.
	*
	* If the status of both characters is not the status to invite and be invited to the party, a chat message is sent to the inviting character.
	*/
	void PartyInvite(LPCHARACTER pchInvitee);

	/// Handle the acceptance of the invited character.
	/**
	* @param pchInvitee A character to join the party. You must be able to participate in the party.
	*
	* If pchInvitee is not able to join the party, the corresponding chat message is sent.
	*/
	void PartyInviteAccept(LPCHARACTER pchInvitee);

	/// Handle the invitation rejection of the invited character.
	/**
	* @param [in] dwPID PID of the invited character
	*/
	void PartyInviteDeny(uint32_t dwPID);

	bool BuildUpdatePartyPacket(TPacketGCPartyUpdate& out);
	int GetLeadershipSkillLevel() const;

	bool CanSummon(int iLeaderShip) noexcept;

	void SetPartyRequestEvent(LPEVENT pkEvent) { m_pkPartyRequestEvent = pkEvent; }

protected:

	/// Join the party.
	/**
	* @param pkLeader Leader of the party to join
	*/
	void PartyJoin(LPCHARACTER pkLeader);

	/**
	* Error code when you cannot join a party.
	* Error code is divided into mutable type and static type depending on whether it is time dependent.
	* If the value of error code is lower than PERR_SEPARATOR, it is a changeable type, and if it is high, it is a static type.
	*/
	enum PartyJoinErrCode
	{
		PERR_NONE = 0, ///< processing success
		PERR_SERVER, ///< Unable to process party related due to server problem
		PERR_DUNGEON, ///< Character is in a dungeon
		PERR_OBSERVER, ///< in spectator mode
		PERR_LVBOUNDARY, ///< There is a difference in level with the other character
		PERR_LOWLEVEL, ///< 30 levels lower than the max level of the opposing party
		PERR_HILEVEL, ///< 30 levels higher than the opponent's minimum level
		PERR_ALREADYJOIN, ///< The character to join the party is already in the party
		PERR_PARTYISFULL, ///< Party limit exceeded
		PERR_SEPARATOR, ///< Error type separator.
		PERR_DIFFEMPIRE, ///< It's a different empire from your opponent's character.
		PERR_MAX ///< Error code peak. Add an error code before this.
	};

	/// Check the conditions for joining or forming a party.
	/**
	* @param pchLeader Party leader or invited character
	* @param pchGuest invited character
	* @return Any PartyJoinErrCode can be returned.
	*/
	static PartyJoinErrCode IsPartyJoinableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest);

	/// Check the dynamic conditions for joining or forming a party.
	/**
	* @param pchLeader Party leader or invited character
	* @param pchGuest invited character
	* @return Only mutable type code is returned.
	*/
	static PartyJoinErrCode IsPartyJoinableMutableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest);

	LPPARTY m_pkParty;
	uint32_t m_dwLastDeadTime;
	LPEVENT m_pkPartyRequestEvent;

	/**
	* Party invitation Event map.
	* key: PID of the invited character
	* value: a pointer to the event
	*
	* Event map for invited characters.
	*/
	typedef std::map<uint32_t, LPEVENT> EventMap;
	EventMap m_PartyInviteEventMap;
	// END_OF_PARTY_JOIN_BUG_FIX

	////////////////////////////////////////////////////////////////////////////////////////
	// Dungeon
public:
	void SetDungeon(LPDUNGEON pkDungeon);
	LPDUNGEON GetDungeon() const noexcept { return m_pkDungeon; }
	LPDUNGEON GetDungeonForce() const;
protected:
	LPDUNGEON m_pkDungeon;
	int m_iEventAttr;

	////////////////////////////////////////////////////////////////////////////////////////
	// Zodiac
#ifdef ENABLE_12ZI
public:
	void SetZodiac(LPZODIAC pkZodiac);
	LPZODIAC GetZodiac() const { return m_pkZodiac; }
	LPZODIAC GetZodiacForce() const;

protected:
	LPZODIAC m_pkZodiac;
#endif

	////////////////////////////////////////////////////////////////////////////////////////
	// Guild
public:
	void SetGuild(CGuild* pGuild);
	CGuild* GetGuild() const noexcept { return m_pGuild; }

	void SetWarMap(CWarMap* pWarMap);
	CWarMap* GetWarMap() const noexcept { return m_pWarMap; }

protected:
	CGuild* m_pGuild;
	uint32_t m_dwUnderGuildWarInfoMessageTime;
	CWarMap* m_pWarMap;

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
public:
	int GetLastOfferNewExpTime() const noexcept { return m_dwLastOfferNewExpTime; }
	void SetLastOfferNewExpTime() noexcept { m_dwLastOfferNewExpTime = thecore_pulse(); }

private:
	int m_dwLastOfferNewExpTime;
#endif

	////////////////////////////////////////////////////////////////////////////////////////
	// Item related
public:
	bool CanHandleItem(bool bSkipRefineCheck = false, bool bSkipObserver = false) noexcept; // Can you do item-related actions?

	bool IsItemLoaded() const noexcept { return m_bItemLoaded; }
	void SetItemLoaded() noexcept { m_bItemLoaded = true; }

	void ClearItem();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void ClearShopItem();
#endif
	void SetItem(TItemPos Cell, LPITEM item, bool isHighLight = false);
	LPITEM GetItem(TItemPos Cell) const;
	LPITEM GetItem_NEW(const TItemPos& Cell) const; //REAL item in the cell (Even if it's "empty" but actually used up). Lame name but well...
	LPITEM GetInventoryItem(uint16_t wCell) const;
	LPITEM GetEquipmentItem(uint16_t wCell) const;
	LPITEM GetDSInventoryItem(uint16_t wCell) const;
	LPITEM GetBeltInventoryItem(uint16_t wCell) const;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	LPITEM GetInventoryFeedItem(uint16_t wCell) const;
#endif
	bool IsEmptyItemGrid(TItemPos Cell, uint8_t size, int iExceptionCell = -1) const;
#ifdef ENABLE_SPECIAL_INVENTORY
	bool IsEmptySpecialItemGrid(TItemPos Cell, uint8_t size, int iExceptionCell = -1) const;
#endif

	void SetWear(uint16_t bCell, LPITEM item); //@fixme479 uint8_t -> uint16_t
	LPITEM GetWear(uint16_t bCell) const noexcept; //@fixme479 uint8_t -> uint16_t

	// MYSHOP_PRICE_LIST
	void UseSilkBotary(void); /// Use of Silk Bag Item

	/// Send the price information list received from the DB cache to the user and handle the use of the bundled item.
	/**
	* @param [in] p price information list packet
	*
	* When using a silk bag item for the first time after logging in, the price information list is requested from UseSilkBotary to the DB cache and
	* When the response is received, this function handles the actual use of the silk bag.
	*/
	void UseSilkBotaryReal(const TPacketMyshopPricelistHeader* p);
	// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void SetShopItems(std::vector<TPlayerItem> vec) { m_shopItems = vec; }
	std::vector<TPlayerItem> GetShopItems() const { return m_shopItems; }
#endif

	bool UseItemEx(LPITEM item, TItemPos DestCell);
	bool UseItem(TItemPos Cell, TItemPos DestCell = NPOS);
	bool StartWeaponBlockAttribute(int Vnum = 0, int Type = 0, int SubType = 0); //@custom007

	// ADD_REFINE_BUILDING
	bool IsRefineThroughGuild() const;
	CGuild* GetRefineGuild() const;
	int ComputeRefineFee(int iCost, int iMultiply = 5) const;
	void PayRefineFee(int iTotalMoney);
	void SetRefineNPC(const LPCHARACTER& ch);
	// END_OF_ADD_REFINE_BUILDING

	//Add_logical_getter_that_ymir_doesnt_use_because_its_ymir
	LPCHARACTER GetRefineNPC() const;
	//End_of_logic

	bool RefineItem(LPITEM pkItem, LPITEM pkTarget);
	bool DropItem(TItemPos Cell, uint8_t bCount = 0);
	bool GiveRecallItem(LPITEM item);
	void ProcessRecallItem(LPITEM item);

#ifdef ENABLE_DESTROY_SYSTEM
	bool RemoveItem(const TItemPos& Cell, uint8_t bCount = 0);
#endif

	// void PotionPacket(int iPotionType);
	void EffectPacket(int enumEffectType);
	void SpecificEffectPacket(const char filename[MAX_EFFECT_FILE_NAME]);

	// ADD_MONSTER_REFINE
	bool DoRefine(LPITEM item, bool bMoneyOnly = false);
	// END_OF_ADD_MONSTER_REFINE

	bool DoRefineWithScroll(LPITEM item);
	bool RefineInformation(uint16_t bCell, uint8_t bType, int iAdditionalCell = -1); //@fixme519

#ifdef ENABLE_QUEEN_NETHIS
	bool DoRefineSerpent(LPITEM item);
#endif

	void SetRefineMode(int iAdditionalCell = -1);
	void ClearRefineMode();
	bool IsRefineWindowOpen() noexcept { return m_bUnderRefine ? true : false; };

	bool GiveItem(LPCHARACTER victim, TItemPos Cell);
	bool CanReceiveItem(LPCHARACTER from, LPITEM item) const;
	void ReceiveItem(LPCHARACTER from, LPITEM item);
	bool GiveItemFromSpecialItemGroup(uint32_t dwGroupNum, std::vector <uint32_t>& dwItemVnums,
		std::vector <uint32_t>& dwItemCounts, std::vector <LPITEM>& item_gets, int& count);
	bool MoveItem(TItemPos pos, TItemPos change_pos, uint8_t num);
	bool PickupItem(uint32_t vid);
#ifdef PET_AUTO_PICKUP
	bool PickupItemByPet(uint32_t vid);
#endif
	bool EquipItem(LPITEM item, int iCandidateCell = -1);
	bool UnequipItem(LPITEM item);

public:
	uint32_t m_iSortInventoryPulse;
	void SetInventorySortPulse(int i) { m_iSortInventoryPulse = i; }
	int GetInventorySortPulse() { return m_iSortInventoryPulse; }

	// A function that checks if the current item can be worn, and if not, tells the character why.
	bool CanEquipNow(const LPITEM item, const TItemPos& srcCell = NPOS, const TItemPos& destCell = NPOS);

	// A function that checks whether the item being worn can be removed, and if not, tells the character why.
#ifdef ENABLE_SWAP_SYSTEM
	bool CanUnequipNow(const LPITEM item, const TItemPos& swapCell = NPOS);
#else
	bool CanUnequipNow(const LPITEM item, const TItemPos& srcCell = NPOS, const TItemPos& destCell = NPOS);
#endif
	bool SwapItem(uint16_t wCell, uint16_t wDestCell); //@fixme519
	LPITEM AutoGiveItem(uint32_t dwItemVnum, uint8_t bCount = 1, int iRarePct = -1, bool bMsg = true);
	void AutoGiveItem(LPITEM item, bool longOwnerShip = false);

#ifdef ENABLE_SPECIAL_INVENTORY
	int GetEmptyInventory(LPITEM item) const;
#else
	int GetEmptyInventory(uint8_t size) const;
#endif
	int GetEmptyInventoryCount(uint8_t size) const;
	int GetEmptyDragonSoulInventory(LPITEM pItem) const;
	void CopyDragonSoulItemGrid(std::vector<uint16_t>& vDragonSoulItemGrid) const;

	int CountEmptyInventory() const;

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
	int CountSpecifyItem(uint32_t vnum, int iExceptionCell = -1, bool bIgnoreSetValue = false) const; //@fixme491
	void RemoveSpecifyItem(uint32_t vnum, uint32_t count = 1, int iExceptionCell = -1, bool bIgnoreSetValue = false); //@fixme491
#else
	int CountSpecifyItem(uint32_t vnum, int iExceptionCell = -1) const; //@fixme491
	void RemoveSpecifyItem(uint32_t vnum, uint32_t count = 1, int iExceptionCell = -1); //@fixme491
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	int CountSpecifyPetFeedItem(uint32_t vnum, int iExceptionCell = -1) const;	//@fixme491
	void RemoveSpecifyPetFeedItem(uint32_t vnum, uint32_t count = 1, int iExceptionCell = -1); //@fixme491
#endif
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
	LPITEM FindSpecifyItem(uint32_t vnum, bool bIgnoreSetValue = false) const;
#else
	LPITEM FindSpecifyItem(uint32_t vnum) const;
#endif
	LPITEM FindItemByID(uint32_t id) const;

	int CountSpecifyTypeItem(uint8_t type) const;
	void RemoveSpecifyTypeItem(uint8_t type, uint32_t count = 1);

	bool IsEquipUniqueItem(uint32_t dwItemVnum) const;

	// CHECK_UNIQUE_GROUP
	bool IsEquipUniqueGroup(uint32_t dwGroupVnum) const;
	// END_OF_CHECK_UNIQUE_GROUP

	void SendEquipment(const LPCHARACTER& ch);
	// End of Item

protected:
	/// Sends price information for one item.
	/**
	* @param [in] dwItemVnum Item vnum
	* @param [in] dwItemPrice Item price
	*/
#ifdef ENABLE_CHEQUE_SYSTEM
	void SendMyShopPriceListCmd(uint32_t dwItemVnum, TItemPriceType ItemPrice);
#else
	void SendMyShopPriceListCmd(uint32_t dwItemVnum, uint32_t dwItemPrice);
#endif

	bool m_bNoOpenedShop; ///< Whether or not you have opened a personal shop after this connection (true if you have never opened it)

	bool m_bItemLoaded;
	int m_iRefineAdditionalCell;
	bool m_bUnderRefine;
	uint32_t m_dwRefineNPCVID;

public:
	////////////////////////////////////////////////////////////////////////////////////////
	// Money related
	int GetGold() const noexcept { return m_points.gold; }
	void SetGold(int gold) noexcept { m_points.gold = gold; }
	bool DropGold(int gold);
	int GetAllowedGold() const noexcept;
	void GiveGold(int iAmount); // If there is a party, distributing the party, handling logs, etc.
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t GetCheque() const noexcept { return m_points.cheque; }
	void SetCheque(uint32_t cheque) noexcept { m_points.cheque = cheque; }
	void GiveCheque(int cheque);
#ifdef ENABLE_CHEQUE_EXCHANGE_WINDOW
	void WonExchange(uint8_t bOption, uint16_t wValue);
#endif
#endif
#ifdef ENABLE_GEM_SYSTEM
	uint32_t GetGemPoint() const noexcept { return m_points.gem_point; }
	void SetGemPoint(uint32_t gem) noexcept { m_points.gem_point = gem; }
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	long long GetLottoMoney() const { return m_points.lotto_moneypool; }
	void SetLottoMoney(long long lotto_moneypool) { m_points.lotto_moneypool = lotto_moneypool; }

	long long GetLottoTotalMoney() const { return m_points.lotto_totalmoneywin; }
	void SetLottoTotalMoney(long long lotto_totalmoneywin) { m_points.lotto_totalmoneywin = lotto_totalmoneywin; }
#endif
	// End of Money

	////////////////////////////////////////////////////////////////////////////////////////
	// Shop related
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
public:
	typedef struct SShopPriceLimitCount
	{
		typedef struct SPurchaseData
		{
			uint32_t dwLimitedCount;
			uint32_t dwLimitedPurchaseCount;
			SPurchaseData() noexcept :
				dwLimitedCount(0),
				dwLimitedPurchaseCount(0)
			{}
		} TPurchaseData;

		std::map<uint32_t, TPurchaseData> data;
	} TShopPriceLimitCount;

	void SetPurchaseItemLimit(LPCHARACTER ch, uint32_t dwVnum, uint32_t dwLimitedCount, uint32_t dwLimitedPurchaseCount);
	void ShopPurchaseLimitItem(LPCHARACTER ch, uint32_t dwVnum);
	bool CanShopPurchaseLimitedItem(LPCHARACTER ch, uint32_t dwVnum);
	uint32_t GetShopLimitedCount(uint32_t dwPlayerID, uint32_t dwVnum);
	uint32_t GetShopLimitedPurchaseCount(uint32_t dwPlayerID, uint32_t dwVnum);

	void BroadcastUpdateLimitedPurchase(uint32_t dwVnum, uint32_t dwLimitedCount, uint32_t dwLimitedPurchaseCount);
protected:
	std::map<uint32_t, TShopPriceLimitCount> m_ShopLimitedPurchaseInfo;
#endif

public:
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, uint8_t bItemCount
# ifdef ENABLE_MYSHOP_DECO_PREMIUM
		, uint32_t dwPolyVnum, uint8_t bTitleType = 0
# endif
	);
	void CloseShop();

	void SetViewingShop(LPSHOP pkShop) noexcept;
	LPSHOP GetViewingShop() const noexcept { return m_pkViewingShop; }

	LPSHOP GetMyShop() const noexcept { return m_pkMyShop; }
	LPCHARACTER GetViewingShopOwner() const noexcept { return m_pkChrShopOwner; }

	void SetMyShop(LPSHOP shop) noexcept
	{
		m_pkMyShop = shop;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_MYSHOP, shop ? true : false);
#endif
	};

	void SetViewingShopOwner(LPCHARACTER ch) noexcept
	{
		m_pkChrShopOwner = ch;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_SHOP_OWNER, ch ? true : false);
#endif
	}

	// Inform the player that a successful sale took place
#ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	void ShopSellResult(uint32_t itemVnum, int amount, uint32_t gold, uint32_t cheque, uint8_t pos);
#else
	void ShopSellResult(uint32_t itemVnum, int amount, uint32_t gold, uint8_t pos);
#endif

	// Shop information
	void SetShopSign(std::string sign) { m_stShopSign = sign; }
	std::string GetShopSign() const { return m_stShopSign; }
	void ClearShopSign() noexcept { m_stShopSign.clear(); }

	void AlterShopGoldStash(uint32_t gold) noexcept { m_dwShopStash += gold; }
	uint32_t GetShopGoldStash() const noexcept { return m_dwShopStash; }

#	ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	void AlterShopChequeStash(uint32_t cheque) noexcept { m_dwShopChequeStash += cheque; }
	uint32_t GetShopChequeStash() const noexcept { return m_dwShopChequeStash; }
#	endif

	void SetEditingShop(bool bIsEditing) noexcept { m_pkEditingShop = bIsEditing; };
	bool IsEditingShop() noexcept { return m_pkEditingShop; }

	// Shop offline timing and events
#	ifdef ENABLE_OPEN_SHOP_WITH_PASSWORD
	void SetPremiumShopLoadTime(int t) noexcept { m_iPremiumShopLoadTime = t; }
	int GetPremiumShopLoadTime() const noexcept { return m_iPremiumShopLoadTime; }
#	endif

	void StartShopOfflineEvent();
	void CancelShopOfflineEvent();
	bool AlterShopOfflineTime(int delta);
	int GetShopOfflineTime();

	void SetShopOfflineEventRunning(bool value) noexcept { m_shopOfflineEventRunning = value; }
	bool IsShopOfflineEventRunning() const noexcept { return m_shopOfflineEventRunning; }

	std::vector<npc_info>::iterator m_npcListVectorPosition;
#else
	void SetShop(LPSHOP pkShop) noexcept;
	LPSHOP GetShop() const { return m_pkShop; }
	void ShopPacket(uint8_t bSubHeader);

	void SetShopOwner(LPCHARACTER ch)
	{
		m_pkChrShopOwner = ch;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_SHOP_OWNER, ch ? true : false);
#endif
	}
	LPCHARACTER GetShopOwner() const { return m_pkChrShopOwner; }

	void OpenMyShop(const char* c_pszSign, TShopItemTable* pTable, uint8_t bItemCount);

	void SetMyShop(LPSHOP shop) { m_pkMyShop = shop; };
	LPSHOP GetMyShop() const { return m_pkMyShop; }
	void CloseMyShop();
#endif

protected:
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	LPEVENT m_shopOfflineEvent;
	bool m_shopOfflineEventRunning;
	std::vector<TPlayerItem> m_shopItems;
	LPSHOP m_pkViewingShop;
	LPSHOP m_pkMyShop;

	LPCHARACTER m_pkChrShopOwner;

	std::string m_stShopSign;
	uint32_t m_dwShopStash;
# ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	uint32_t m_dwShopChequeStash;
# endif
	bool m_pkEditingShop;
# ifdef ENABLE_OPEN_SHOP_WITH_PASSWORD
	int m_iPremiumShopLoadTime{ 0 };
# endif
#else
	LPSHOP m_pkShop;
	LPSHOP m_pkMyShop;
	std::string m_stShopSign;
	LPCHARACTER m_pkChrShopOwner;
#endif
	// End of shop

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
public:
	uint8_t GetGemShopOpenSlotCount() noexcept;
	uint8_t GetGemShopOpenSlotItemCount(uint8_t slot_count);
	void OpenGemShop(bool bAllSlotUnlocked = false);
	void RefreshGemShop();
	void OpenGemShopFirstTime();
	void RefreshGemShopItems();
	void RefreshGemShopWithItem();
	void GemShopBuy(uint8_t bPos);
	void GemShopAdd();
	bool CreateGem(int glimmerstone_count, LPITEM metinstone_item, int cost, uint8_t pct);

	int GetGemNextRefresh() const noexcept { return m_dwGemNextRefresh; }
	void SetGemNextRefresh(int next_time) noexcept { m_dwGemNextRefresh = next_time; }

protected:
	TPlayerGemItems* m_gemItems{ nullptr };

private:
	int m_dwGemNextRefresh{ 0 };
	bool m_bGemShopAllSlotUnlocked{ false };
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
protected:
	std::unordered_map<uint32_t, TGrowthPetInfo> m_GrowthPetInfo;
public:
	std::vector<TGrowthPetInfo> GetPetList() const;
	void SetGrowthPetInfo(TGrowthPetInfo petInfo);
	void SendGrowthPetInfoPacket();
#endif

#ifdef ENABLE_CHANGED_ATTR
public:
	void SelectAttr(LPITEM material, LPITEM item);
	void SelectAttrResult(const bool bNew, const TItemPos& pos);
	bool IsSelectAttr() const;

private:
	struct SItemSelectAttr
	{
		uint32_t dwItemID;
		TPlayerItemAttribute Attr[ITEM_ATTRIBUTE_MAX_NUM];
	} m_ItemSelectAttr;
#endif

	////////////////////////////////////////////////////////////////////////////////////////
	// Exchange related
public:
	bool ExchangeStart(LPCHARACTER victim);
	void SetExchange(CExchange* pkExchange) noexcept;
	CExchange* GetExchange() const noexcept { return m_pkExchange; }

protected:
	CExchange* m_pkExchange;
	// End of Exchange

	////////////////////////////////////////////////////////////////////////////////////////
	// Battle
public:
	struct TBattleInfo
	{
		int iTotalDamage;
		int iAggro;

		TBattleInfo(int iTot, int iAggr) noexcept
			: iTotalDamage(iTot), iAggro(iAggr)
		{}
	};
	typedef std::map<VID, TBattleInfo> TDamageMap;

	typedef struct SAttackLog
	{
		uint32_t dwVID;
		uint32_t dwTime;
	} AttackLog;

	bool Damage(LPCHARACTER pAttacker, int dam, EDamageType type = EDamageType::DAMAGE_TYPE_NORMAL);
	void DeathPenalty(uint8_t bExpLossPercent);
	void ReviveInvisible(int iDur);

	bool Attack(LPCHARACTER pkVictim, uint8_t bType = 0);
	bool IsAlive() const noexcept { return m_pointsInstant.position == POS_DEAD ? false : true; }
	bool CanFight() const;

	bool CanBeginFight() const;
	void BeginFight(LPCHARACTER pkVictim); // Start fighting pkVictimr. (compulsory, use CanBeginFight to check if you can start)

	bool IsStun() const;
	void Stun();
	bool IsDead() const noexcept;
	void Dead(LPCHARACTER pkKiller = nullptr, bool bImmediateDead = true); //@fixme188

	void SetNoRewardFlag() { SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD); }
	void RemoveNoRewardFlag() { REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD); }
	bool HasNoRewardFlag() const { return IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD); }

	void Reward(bool bItemDrop);
	void RewardGold(LPCHARACTER pkAttacker);

	bool Shoot(uint8_t bType);
	void FlyTarget(uint32_t dwTargetVID, long x, long y, uint8_t bHeader);

	void ForgetMyAttacker();
	void AggregateMonster();
	void AttractRanger();
	void PullMonster();

	int GetArrowAndBow(LPITEM* ppkBow, LPITEM* ppkArrow, int iArrowCount = 1);
	void UseArrow(LPITEM pkArrow, uint32_t dwArrowCount);

	void AttackedByPoison(LPCHARACTER pkAttacker);
	void RemovePoison();
#ifdef ENABLE_WOLFMAN_CHARACTER
	void AttackedByBleeding(LPCHARACTER pkAttacker);
	void RemoveBleeding();
#endif
	void AttackedByFire(const LPCHARACTER& pkAttacker, int amount, int count);
	void RemoveFire();

	void UpdateAlignment(int iAmount);
	int GetAlignment() const;

	//get good and bad
	int GetRealAlignment() const;
	void ShowAlignment(bool bShow);

	void SetKillerMode(bool bOn);
	bool IsKillerMode() const;
	void UpdateKillerMode();

	uint8_t GetPKMode() const;
	void SetPKMode(uint8_t bPKMode);

	void ItemDropPenalty(LPCHARACTER pkKiller);

	void UpdateAggrPoint(LPCHARACTER ch, EDamageType type, int dam);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	bool CanAttack() { return FindAffect(AFFECT_IMPOSSIBLE_ATTACK) ? false : true; };
#endif

#ifdef ENABLE_AFK_MODE_SYSTEM
public:
	void SetAway(bool f) { m_isAway = f; }
	bool IsAway() const { return m_isAway; }
	void StartUpdateCharacterEvent();
#endif

	//
	// HACK
	//

#ifdef ENABLE_CSHIELD
private:
	spCShield m_cshield;

public:
	spCShield GetCShield() noexcept { return m_cshield; };

#ifdef ENABLE_CHECK_PICKUP_HACK
private:
	uint32_t m_dwLastPickupTime;
public:
	void SetLastPickupTime() { m_dwLastPickupTime = get_dword_time(); }
	uint32_t GetLastPickupTime() { return m_dwLastPickupTime; }
#endif

#ifdef ENABLE_CHECK_GHOSTMODE
private:
	uint32_t m_dwCountGhostmodePoint;
public:
	uint32_t GetGhostmodeCount() { return m_dwCountGhostmodePoint; }
	void AddGhostmodeCount() { m_dwCountGhostmodePoint += 1; }
	void ResetGhostmodeCount() { m_dwCountGhostmodePoint = 0; }
#endif

#ifdef ENABLE_CHECK_WALLHACK
private:
	uint32_t m_dwCountWallhackPoint;
public:
	uint32_t GetWallhackCount() { return m_dwCountWallhackPoint; }
	void AddWallhackCount() { m_dwCountWallhackPoint += 1; }
	void ResetWallhackCount() { m_dwCountWallhackPoint = 0; }
#endif
#endif

public:
	void SetComboSequence(uint8_t seq) noexcept;
	uint8_t GetComboSequence() const noexcept;

	void SetLastComboTime(uint32_t time) noexcept;
	uint32_t GetLastComboTime() const noexcept;

	int GetValidComboInterval() const noexcept;
	void SetValidComboInterval(int interval) noexcept;

	uint8_t GetComboIndex() const noexcept;

	void IncreaseComboHackCount(int k = 1);
	void ResetComboHackCount() noexcept;
	void SkipComboAttackByTime(int interval) noexcept;
	uint32_t GetSkipComboAttackByTime() const noexcept;

protected:
	uint8_t m_bComboSequence;
	uint32_t m_dwLastComboTime;
	int m_iValidComboInterval;
	uint8_t m_bComboIndex;
	int m_iComboHackCount;
	uint32_t m_dwSkipComboAttackByTime;

protected:
	void UpdateAggrPointEx(LPCHARACTER ch, EDamageType type, int dam, TBattleInfo& info);
	void ChangeVictimByAggro(int iNewAggro, LPCHARACTER pNewVictim);

	uint32_t m_dwFlyTargetID;
	std::vector<uint32_t> m_vec_dwFlyTargets;
	TDamageMap m_map_kDamage; // How much damage did a character do to me?
	// AttackLog m_kAttackLog;
	uint32_t m_dwKillerPID;

	int m_iAlignment; // Lawful/Chaotic value -200000 ~ 200000
	int m_iRealAlignment;
	int m_iKillerModePulse;
	uint8_t m_bPKMode;

	// Aggro
	uint32_t m_dwLastVictimSetTime;
	int m_iMaxAggro;
	// End of Battle

	// Stone
public:
	void SetStone(LPCHARACTER pkChrStone);
	void ClearStone();
	void DetermineDropMetinStone();

	uint32_t GetDropMetinStoneVnum() const noexcept { return m_dwDropMetinStone; }
	uint8_t GetDropMetinStonePct() const noexcept { return m_bDropMetinStonePct; }

protected:
	LPCHARACTER m_pkChrStone; // the stone that spawned me
	CHARACTER_SET m_set_pkChrSpawnedBy; // the ones i sponsored
	uint32_t m_dwDropMetinStone;
	uint8_t m_bDropMetinStonePct;
	// End of Stone

public:
	enum
	{
		SKILL_UP_BY_POINT,
		SKILL_UP_BY_BOOK,
		SKILL_UP_BY_TRAIN,

		// ADD_GRANDMASTER_SKILL
		SKILL_UP_BY_QUEST,
		// END_OF_ADD_GRANDMASTER_SKILL
	};

	void SkillLevelPacket();
#ifdef ENABLE_678TH_SKILL
	bool SkillCanUp(uint32_t dwVnum);
	bool Is678Skill(uint32_t dwSkillVnum);
	bool Learn678SkillByBook(uint32_t dwSkillVnum);
#endif
#ifdef ENABLE_NINETH_SKILL
	bool NineSkillCanUp(uint32_t dwVnum);
	bool IsNineSkill(uint32_t dwSkillVnum);
	bool LearnNineSkillByBook(uint32_t dwSkillVnum, uint8_t bSkillGroup);
#endif
	void SkillLevelUp(uint32_t dwVnum, uint8_t bMethod = SKILL_UP_BY_POINT);
	bool SkillLevelDown(uint32_t dwVnum);
	// ADD_GRANDMASTER_SKILL
	bool UseSkill(uint32_t dwVnum, LPCHARACTER pkVictim, bool bUseGrandMaster = true);
#ifdef ENABLE_CHECK_SKILL_HACK
	bool IsSkillCooldown(uint32_t dwVnum, float fSkillPower) { return m_SkillUseInfo[dwVnum].IsSkillCooldown(dwVnum, fSkillPower) ? true : false; }
#endif
#ifdef ENABLE_SKILL_COOLTIME_UPDATE
	void ResetSkillCoolTimes();
#endif

	void ResetSkill();
	void SetSkillLevel(uint32_t dwVnum, uint8_t bLev);
	int GetUsedSkillMasterType(uint32_t dwVnum);

	bool IsLearnableSkill(uint32_t dwSkillVnum) const;
	// END_OF_ADD_GRANDMASTER_SKILL

	bool CheckSkillHitCount(const uint8_t SkillID, const VID dwTargetVID);
	bool CanUseSkill(uint32_t dwSkillVnum) const;
	bool IsUsableSkillMotion(uint32_t dwMotionIndex) const;
	int GetSkillLevel(uint32_t dwVnum) const;
	int GetSkillMasterType(uint32_t dwVnum) const;
	int GetSkillPower(uint32_t dwVnum, uint8_t bLevel = 0) const;

	time_t GetSkillNextReadTime(uint32_t dwVnum) const;
	void SetSkillNextReadTime(uint32_t dwVnum, time_t time);
	void SkillLearnWaitMoreTimeMessage(uint32_t dwVnum);

	void ComputePassiveSkill(uint32_t dwVnum);
	int ComputeSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel = 0);
#ifdef ENABLE_PVP_BALANCE
	int ComputeGyeongGongSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel = 0);
#endif
#ifdef ENABLE_WOLFMAN_CHARACTER
	int ComputeSkillParty(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel = 0);
#endif
#ifdef ENABLE_QUEEN_NETHIS
	int ComputeSnakeSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel);
#endif
//#ifdef ENABLE_SHAMAN_PARTY_BUFF_SYSTEM
	bool IsBuffSkill(uint32_t dwSkillVnum) const noexcept;
//#endif
	int ComputeSkillAtPosition(uint32_t dwVnum, const PIXEL_POSITION& posTarget, uint8_t bSkillLevel = 0);
	void ComputeSkillPoints();

	void SetSkillGroup(uint8_t bSkillGroup);
	uint8_t GetSkillGroup() const noexcept { return m_points.skill_group; }

	int ComputeCooltime(int time);

	void GiveRandomSkillBook();

	void DisableCooltime();
	bool LearnSkillByBook(uint32_t dwSkillVnum, uint8_t bProb = 0);
	bool LearnGrandMasterSkill(uint32_t dwSkillVnum);

private:
	bool m_bDisableCooltime;
	uint32_t m_dwLastSkillTime; ///< The last time the skill was written in milliseconds.
	// End of Skill

	// MOB_SKILL
public:
	bool HasMobSkill() const;
	size_t CountMobSkill() const;
	const TMobSkillInfo* GetMobSkill(uint32_t idx) const;
	bool CanUseMobSkill(uint32_t idx) const;
	bool UseMobSkill(uint32_t idx);
	void ResetMobSkillCooltime();
protected:
	uint32_t m_adwMobSkillCooltime[MOB_SKILL_MAX_NUM];
	// END_OF_MOB_SKILL

	// for SKILL_MUYEONG
public:
	void StartMuyeongEvent();
	void StopMuyeongEvent();
#ifdef ENABLE_PVP_BALANCE
	void StartGyeongGongEvent();
	void StopGyeongGongEvent();
#endif
#ifdef ENABLE_NINETH_SKILL
	void StartCheonunEvent(uint8_t bShieldChance, uint8_t bShieldDuration);
	void StopCheonunEvent();
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	// Timer running when an item have hit buff element bonus
	void StartHitBuffElementEvent();
	void StopHitBuffElementEvent();
#endif
#ifdef ENABLE_ELEMENTAL_WORLD
	void StartElementalWorldEvent();
	void StopElementalWorldEvent();
#endif

private:
	LPEVENT m_pkMuyeongEvent;
#ifdef ENABLE_PVP_BALANCE
	LPEVENT m_pkGyeongGongEvent;
#endif
#ifdef ENABLE_NINETH_SKILL
	LPEVENT m_pkCheonunEvent;
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	LPEVENT m_pkHitBuffElementEvent;
#endif
#ifdef ENABLE_QUEEN_NETHIS
	LPEVENT m_pkSnakeSkillEvent;
#endif
#ifdef ENABLE_ELEMENTAL_WORLD
public:
	LPEVENT m_pkElementalWorldEvent;
	uint32_t hp_reduce_count;
#endif

	// for SKILL_CHAIN lighting
public:
	int GetChainLightningIndex() const noexcept { return m_iChainLightingIndex; }
	void IncChainLightningIndex() noexcept { ++m_iChainLightingIndex; }
	void AddChainLightningExcept(LPCHARACTER ch) { m_setExceptChainLighting.insert(ch); }
	void ResetChainLightningIndex() noexcept { m_iChainLightingIndex = 0; m_setExceptChainLighting.clear(); }
	int GetChainLightningMaxCount() const;
	const CHARACTER_SET& GetChainLightingExcept() const noexcept { return m_setExceptChainLighting; }

private:
	int m_iChainLightingIndex;
	CHARACTER_SET m_setExceptChainLighting;

	// for SKILL_EUNHYUNG
public:
	void SetAffectedEunhyung();
	void ClearAffectedEunhyung() noexcept { m_dwAffectedEunhyungLevel = 0; }
	bool GetAffectedEunhyung() const noexcept { return m_dwAffectedEunhyungLevel; }

private:
	uint32_t m_dwAffectedEunhyungLevel;

	//
	// Skill levels
	//
protected:
	TPlayerSkill* m_pSkillLevels;
	std::unordered_map<uint8_t, int> m_SkillDamageBonus;
	std::map<int, TSkillUseInfo> m_SkillUseInfo;

	////////////////////////////////////////////////////////////////////////////////////////
	// AI related
public:
	void AssignTriggers(const TMobTable* table);
	LPCHARACTER GetVictim() const; // Return target to attack
	void SetVictim(LPCHARACTER pkVictim);
	LPCHARACTER GetNearestVictim(LPCHARACTER pkChr);
	LPCHARACTER GetProtege() const; // return target to be protected

	bool Follow(LPCHARACTER pkChr, float fMinimumDistance = 150.0f);
	bool Return();
	bool IsGuardNPC() const noexcept;
	bool IsChangeAttackPosition(const LPCHARACTER& target) const;
	void ResetChangeAttackPositionTime() noexcept { m_dwLastChangeAttackPositionTime = get_dword_time() - AI_CHANGE_ATTACK_POISITION_TIME_NEAR; }
	void SetChangeAttackPositionTime() noexcept { m_dwLastChangeAttackPositionTime = get_dword_time(); }

	bool OnIdle() noexcept;

	void OnClick(LPCHARACTER pkChrCauser);

	VID m_kVIDVictim;

protected:
	uint32_t m_dwLastChangeAttackPositionTime;
	CTrigger m_triggerOnClick;
	// End of AI

	////////////////////////////////////////////////////////////////////////////////////////
	// Target
protected:
	LPCHARACTER m_pkChrTarget; // my target
	CHARACTER_SET m_set_pkChrTargetedBy; // people who have me as a target

public:
	void SetTarget(LPCHARACTER pkChrTarget);
	void BroadcastTargetPacket();
	void ClearTarget();
	void CheckTarget();
	LPCHARACTER GetTarget() const noexcept { return m_pkChrTarget; }

	////////////////////////////////////////////////////////////////////////////////////////
	// Safebox
public:
	int GetSafeboxSize() const noexcept;
	void QuerySafeboxSize();
	void SetSafeboxSize(int size);

	CSafebox* GetSafebox() const noexcept;
#ifdef ENABLE_SAFEBOX_MONEY
	void LoadSafebox(int iSize, uint32_t dwGold, int iItemCount, TPlayerItem* pItems);
#else
	void LoadSafebox(int iSize, int iItemCount, TPlayerItem* pItems);
#endif
	void ChangeSafeboxSize(uint8_t bSize);
	void CloseSafebox();

	/// Request to open warehouse
	/**
	* @param [in] pszPassword Warehouse password between 1 and 6 characters
	*
	* Request to open warehouse in DB.
	* Warehouses cannot be opened repeatedly, and cannot be opened within 10 seconds of the last warehouse closing time.
	*/
	void ReqSafeboxLoad(const char* pszPassword);

	/// Cancel warehouse open request
	/**
	* If you call this function when ReqSafeboxLoad is called and CloseSafebox is not closed, the warehouse can be opened.
	* When a request to open a warehouse receives a failure response from the DB server, this function is used to make the request.
	*/
	void CancelSafeboxLoad(void) noexcept { m_bOpeningSafebox = false; }

	void SetMallLoadTime(int t) noexcept { m_iMallLoadTime = t; }
	int GetMallLoadTime() const noexcept { return m_iMallLoadTime; }

	CSafebox* GetMall() const noexcept;
	void LoadMall(int iItemCount, TPlayerItem* pItems);
	void CloseMall();

	void SetSafeboxOpenPosition();
	float GetDistanceFromSafeboxOpen() const;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	int GetGuildstorageSize() const;
	void QueryGuildstorageSize();
	void SetGuildstorageSize(int iSize);
	void ChangeGuildstorageSize(uint8_t bSize);

	CSafebox* GetGuildstorage() const;
	void LoadGuildstorage(int iSize, int iItemCount, TPlayerItem* pItems);
	void CloseGuildstorage();
	void ReqGuildstorageLoad();

	void SetGuildstorageOpenPosition();
	float GetDistanceFromGuildstorageOpen() const;
#endif

protected:
	CSafebox* m_pkSafebox;
	int m_iSafeboxSize;
	int m_iSafeboxLoadTime;
	bool m_bOpeningSafebox; ///< Whether the warehouse is requesting to open or open, if true, is requesting to open or open.

	CSafebox* m_pkMall;
	int m_iMallLoadTime;

	PIXEL_POSITION m_posSafeboxOpen;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	CSafebox* m_pkGuildstorage;
	int m_iGuildstorageSize;
	int m_iGuildstorageLoadTime;
	bool m_bOpeningGuildstorage;

	PIXEL_POSITION m_posGuildstorageOpen;
#endif
	////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////
	// Mounting
public:
	void MountVnum(uint32_t vnum);
	uint32_t GetMountVnum() const noexcept { return m_dwMountVnum; }
	uint32_t GetLastMountTime() const noexcept { return m_dwMountTime; }

	bool CanUseHorseSkill();

	// Horse
	virtual void SetHorseLevel(int iLevel);

	virtual bool StartRiding();
	virtual bool StopRiding();

#ifdef ENABLE_HORSE_APPEARANCE
	virtual uint32_t GetMyHorseVnum();
#else
	virtual uint32_t GetMyHorseVnum() const;
#endif

	virtual void HorseDie();
	virtual bool ReviveHorse();

	virtual void SendHorseInfo();
	virtual void ClearHorseInfo();

	void HorseSummon(bool bSummon, bool bFromFar = false, uint32_t dwVnum = 0, const char* name = 0);

	LPCHARACTER GetHorse() const noexcept { return m_chHorse; } // currently being summoned
	LPCHARACTER GetRider() const; // rider on horse
	void SetRider(LPCHARACTER ch);

	bool IsRiding() const;

#ifdef __PET_SYSTEM__
public:
	CPetSystem* GetPetSystem() noexcept { return m_petSystem; }

protected:
	CPetSystem* m_petSystem;
#endif

#ifdef ENABLE_PET_SYSTEM
public:
	void PetSummon(LPITEM petItem);
	void PetUnsummon(LPITEM petItem);
	void CheckPet();
	bool IsPetSummon();
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
public:
	CGrowthPetSystem* GetGrowthPetSystem() noexcept { return m_GrowthPetSystem; }
protected:
	CGrowthPetSystem* m_GrowthPetSystem;
#endif

protected:
	LPCHARACTER m_chHorse;
	LPCHARACTER m_chRider;

	uint32_t m_dwMountVnum;
	uint32_t m_dwMountTime;

	uint8_t m_bSendHorseLevel;
	uint8_t m_bSendHorseHealthGrade;
	uint8_t m_bSendHorseStaminaGrade;
#ifdef ENABLE_AFK_MODE_SYSTEM
	bool m_isAway;
#endif

	////////////////////////////////////////////////////////////////////////////////////////
	// Detailed Log
public:
	void DetailLog() noexcept { m_bDetailLog = !m_bDetailLog; }
	void ToggleMonsterLog();
	void MonsterLog(const char* format, ...);
private:
	bool m_bDetailLog;
	bool m_bMonsterLog;

	////////////////////////////////////////////////////////////////////////////////////////
	// Empire

public:
	void SetEmpire(uint8_t bEmpire) noexcept;
	uint8_t GetEmpire() const noexcept { return m_bEmpire; }

protected:
	uint8_t m_bEmpire;

	////////////////////////////////////////////////////////////////////////////////////////
	// Regen
public:
	void SetRegen(LPREGEN pkRegen);

protected:
	PIXEL_POSITION m_posRegen;
	float m_fRegenAngle;
	LPREGEN m_pkRegen;
	size_t regen_id_; // to help dungeon regen identification
	// End of Regen

	////////////////////////////////////////////////////////////////////////////////////////
	// Resists & Proofs
public:
	bool CannotMoveByAffect() const; // Are you unable to move due to a specific effect?
	bool IsImmune(uint32_t dwImmuneFlag);
	void SetImmuneFlag(uint32_t dw) noexcept { m_pointsInstant.dwImmuneFlag = dw; }

protected:
	void ApplyMobAttribute(const TMobTable* table);
	// End of Resists & Proofs

	////////////////////////////////////////////////////////////////////////////////////////
	// QUEST
	//
public:
	void SetQuestNPCID(uint32_t vid) noexcept;
	uint32_t GetQuestNPCID() const noexcept { return m_dwQuestNPCVID; }
	LPCHARACTER GetQuestNPC() const;

	void SetQuestItemPtr(LPITEM item) noexcept;
	void ClearQuestItemPtr() noexcept;
	LPITEM GetQuestItemPtr() const noexcept;

#ifdef ENABLE_QUEST_DND_EVENT
	void SetQuestDNDItemPtr(LPITEM item);
	void ClearQuestDNDItemPtr();
	LPITEM GetQuestDNDItemPtr() const;
#endif

	void SetQuestBy(uint32_t dwQuestVnum) noexcept { m_dwQuestByVnum = dwQuestVnum; }
	uint32_t GetQuestBy() const noexcept { return m_dwQuestByVnum; }

	int GetQuestFlag(const std::string& flag) const;
	void SetQuestFlag(const std::string& flag, int value);

	void ConfirmWithMsg(const char* szMsg, int iTimeout, uint32_t dwRequestPID);

private:
	uint32_t m_dwQuestNPCVID;
	uint32_t m_dwQuestByVnum;
	LPITEM m_pQuestItem;
#ifdef ENABLE_QUEST_DND_EVENT
	LPITEM m_pQuestDNDItem{ nullptr };
#endif

	// Events
public:
	bool StartStateMachine(int iPulse = 1);
	void StopStateMachine();
	void UpdateStateMachine(uint32_t dwPulse);
	void SetNextStatePulse(int iPulseNext);

	/*
	* Character instance update function. Previously, the CFSM::Update function was called or the
	* UpdateStateMachine function was used with a strange inheritance structure,
	* but a separate update function was added.
	*/
	void UpdateCharacter(uint32_t dwPulse);

protected:
	uint32_t m_dwNextStatePulse;

	// Marriage
public:
	LPCHARACTER GetMarryPartner() const noexcept;
	void SetMarryPartner(LPCHARACTER ch) noexcept;
	int GetMarriageBonus(uint32_t dwItemVnum, bool bSum = true);

	void SetWeddingMap(marriage::WeddingMap* pMap);
	marriage::WeddingMap* GetWeddingMap() const noexcept { return m_pWeddingMap; }

private:
	marriage::WeddingMap* m_pWeddingMap;
	LPCHARACTER m_pkChrMarried;

	// Warp Character
public:
	void StartWarpNPCEvent();

public:
	void StartSaveEvent();
	void StartRecoveryEvent();
	void StartCheckSpeedHackEvent();
	void StartDestroyWhenIdleEvent();

	LPEVENT m_pkDeadEvent;
	LPEVENT m_pkStunEvent;
	LPEVENT m_pkSaveEvent;
	LPEVENT m_pkRecoveryEvent;
	LPEVENT m_pkTimedEvent;
	LPEVENT m_pkFishingEvent;
	LPEVENT m_pkAffectEvent;
	LPEVENT m_pkPoisonEvent;
#ifdef ENABLE_WOLFMAN_CHARACTER
	LPEVENT m_pkBleedingEvent;
#endif
	LPEVENT m_pkFireEvent;
	LPEVENT m_pkWarpNPCEvent;
	//DELAYED_WARP
	//END_DELAYED_WARP

	// MINING
	LPEVENT m_pkMiningEvent;
	// END_OF_MINING
	LPEVENT m_pkWarpEvent;
	LPEVENT m_pkCheckSpeedHackEvent;
	LPEVENT m_pkDestroyWhenIdleEvent;
	LPEVENT m_pkPetSystemUpdateEvent;
#ifdef ENABLE_BATTLE_FIELD
	LPEVENT m_pkBattleZoneEvent;
#endif
#ifdef ENABLE_AFK_MODE_SYSTEM
	LPEVENT m_pkUpdateCharacter;
#endif

	bool IsWarping() const noexcept { return m_pkWarpEvent ? true : false; }

	bool m_bHasPoisoned;
#ifdef ENABLE_WOLFMAN_CHARACTER
	bool m_bHasBled;
#endif

	const CMob* m_pkMobData;
	CMobInstance* m_pkMobInst;

	std::map<int, LPEVENT> m_mapMobSkillEvent;

	friend struct FuncSplashDamage;
	friend struct FuncSplashAffect;
	friend struct RemoveInvisibleVictim; //@fixme512
	friend class CFuncShoot;

public:
	int GetPremiumRemainSeconds(uint8_t bType) const;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	int GetPremiumPrivateShopRemainSeconds() const;
#endif
#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_OPEN_SHOP_WITH_PASSWORD)
public:
	void OpenShopEditorWithPassword(const LPCHARACTER& ch, bool bMatchPass = false);
#endif

private:
	int m_aiPremiumTimes[PREMIUM_MAX_NUM];

	// CHANGE_ITEM_ATTRIBUTES
	// static const uint32_t msc_dwDefaultChangeItemAttrCycle; ///< Default item attribute changeable period
	static const char msc_szLastChangeItemAttrFlag[]; ///< The name of the Quest Flag at the time the last item attribute was changed.
	// static const char msc_szChangeItemAttrCycleFlag[]; ///< Quest Flag name of the item attribute changeable cycle
	// END_OF_CHANGE_ITEM_ATTRIBUTES

	// NEW_HAIR_STYLE_ADD
public:
	bool ItemProcess_Hair(LPITEM item, int iDestCell);
	// END_NEW_HAIR_STYLE_ADD

public:
	void ClearSkill();
	void ClearSubSkill();

	// RESET_ONE_SKILL
	bool ResetOneSkill(uint32_t dwVnum);
	// END_RESET_ONE_SKILL

private:
	void SendDamagePacket(LPCHARACTER pAttacker, int Damage, uint8_t DamageFlag);

	// ARENA
private:
	CArena* m_pArena;
	bool m_ArenaObserver;
	int m_nPotionLimit;

public:
	void SetArena(CArena* pArena) noexcept { m_pArena = pArena; }
	void SetArenaObserverMode(bool flag) noexcept { m_ArenaObserver = flag; }

	CArena* GetArena() const noexcept { return m_pArena; }
	bool GetArenaObserverMode() const noexcept { return m_ArenaObserver; }

	void SetPotionLimit(int count) noexcept { m_nPotionLimit = count; }
	int GetPotionLimit() const noexcept { return m_nPotionLimit; }
	// END_ARENA

	//PREVENT_TRADE_WINDOW
public:
	bool IsOpenSafebox() const noexcept { return m_isOpenSafebox ? true : false; }
	void SetOpenSafebox(bool b) noexcept
	{
		m_isOpenSafebox = b;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_SAFEBOX, b);
#endif
	}

	int GetSafeboxLoadTime() const noexcept { return m_iSafeboxLoadTime; }
	void SetSafeboxLoadTime() noexcept { m_iSafeboxLoadTime = thecore_pulse(); }


#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	bool IsOpenGuildstorage() const { return m_isOpenGuildstorage ? true : false; }
	void SetOpenGuildstorage(bool b) {
		m_isOpenGuildstorage = b;
# ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_GUILDBANK, m_isOpenGuildstorage ? true : false);
# endif
	}

	int GetGuildstorageLoadTime() const { return m_iGuildstorageLoadTime; }
	void SetGuildstorageLoadTime() { m_iGuildstorageLoadTime = thecore_pulse(); }
#endif

	//END_PREVENT_TRADE_WINDOW
private:
	bool m_isOpenSafebox;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	bool m_isOpenGuildstorage;
#endif

public:
	int GetSkillPowerByLevel(int level, bool bMob = false) const;

	//PREVENT_REFINE_HACK
	int GetRefineTime() const noexcept { return m_iRefineTime; }
	void SetRefineTime() noexcept { m_iRefineTime = thecore_pulse(); }
	int m_iRefineTime;
	//END_PREVENT_REFINE_HACK

	//RESTRICT_USE_SEED_OR_MOONBOTTLE
	int GetUseSeedOrMoonBottleTime() const noexcept { return m_iSeedTime; }
	void SetUseSeedOrMoonBottleTime() noexcept { m_iSeedTime = thecore_pulse(); }
	int m_iSeedTime;
	//END_RESTRICT_USE_SEED_OR_MOONBOTTLE

	//PREVENT_PORTAL_AFTER_EXCHANGE
	int GetExchangeTime() const noexcept { return m_iExchangeTime; }
	void SetExchangeTime() noexcept { m_iExchangeTime = thecore_pulse(); }
	int m_iExchangeTime;
	//END_PREVENT_PORTAL_AFTER_EXCHANGE

	int m_iMyShopTime;
	int GetMyShopTime() const noexcept { return m_iMyShopTime; }
	void SetMyShopTime() noexcept { m_iMyShopTime = thecore_pulse(); }

	// Check to prevent hacking.
	bool IsHack(bool bSendMsg = true, bool bCheckShopOwner = true, int limittime = g_nPortalLimitTime);

	// MONARCH
	bool IsMonarch() const;
	// END_MONARCH
	void Say(const std::string& s);

	enum MONARCH_COOLTIME
	{
		MC_HEAL = 10,
		MC_WARP = 60,
		MC_TRANSFER = 60,
		MC_TAX = (60 * 60 * 24 * 7),
		MC_SUMMON = (60 * 60),
	};

	enum MONARCH_INDEX
	{
		MI_HEAL = 0,
		MI_WARP,
		MI_TRANSFER,
		MI_TAX,
		MI_SUMMON,
		MI_MAX
	};

	uint32_t m_dwMonarchCooltime[MI_MAX];
	uint32_t m_dwMonarchCooltimelimit[MI_MAX];

	void InitMC() noexcept;
	uint32_t GetMC(enum MONARCH_INDEX e) const noexcept;
	void SetMC(enum MONARCH_INDEX e) noexcept;
	bool IsMCOK(enum MONARCH_INDEX e) const;
	uint32_t GetMCL(enum MONARCH_INDEX e) const noexcept;
	uint32_t GetMCLTime(enum MONARCH_INDEX e) const;

public:
	bool ItemProcess_Polymorph(LPITEM item);

	// by mhh
#ifndef ENABLE_CUBE_RENEWAL
	LPITEM* GetCubeItem() noexcept { return m_pointsInstant.playerSlots->pCubeItems; }
#endif
	bool IsCubeOpen() const noexcept { return (m_pointsInstant.pCubeNpc ? true : false); }
	void SetCubeNpc(LPCHARACTER npc) noexcept
	{
		m_pointsInstant.pCubeNpc = npc;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_CUBE, npc ? true : false);
#endif
	}
	bool CanDoCube() const;

#ifdef ENABLE_CUBE_RENEWAL
	void SetTempCubeNPC(uint32_t vnum) { tempCubeNPC = vnum; }
	uint32_t GetTempCubeNPC() { return tempCubeNPC; }

	int GetCubeLastTime() const { return m_iCubeTime; }
	void SetCubeTime() { m_iCubeTime = thecore_pulse(); }

private:
	uint32_t tempCubeNPC;
	int m_iCubeTime;
#endif

public:
	bool IsSiegeNPC() const noexcept;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#ifdef ENABLE_MYSHOP_DECO
protected:
	uint8_t m_bMyPrivShopState;

	bool m_bMyPrivShopIsCashItem;
	uint8_t m_bMyPrivShopTabCount;

	uint8_t m_bMyPrivShopType;
	uint32_t m_bMyPrivShopPolyVnum;
public:
	void SetMyPrivShopState(uint8_t bState) noexcept { m_bMyPrivShopState = bState; };
	uint8_t GetMyPrivShopState() const noexcept { return m_bMyPrivShopState; };

	void SetMyShopTabCount(uint8_t bShopTabCount) noexcept { m_bMyPrivShopTabCount = bShopTabCount; };
	uint8_t GetMyShopTabCount() const noexcept { return m_bMyPrivShopTabCount; };

	void SetMyShopType(uint8_t bShopType) noexcept { m_bMyPrivShopType = bShopType; };
	uint8_t GetMyShopType() const noexcept { return m_bMyPrivShopType; };

	void SetMyShopDecoPolyVnum(uint32_t dwPolyVnum) noexcept { m_bMyPrivShopPolyVnum = dwPolyVnum; };
	uint32_t GetMyShopDecoPolyVnum() const noexcept { return m_bMyPrivShopPolyVnum; };

	void SetMyPrivShopAdd(uint8_t bType, uint32_t dwPolyVnum);
	void SendMyPrivShopOpen(uint8_t bTabCount, bool bIsCashItem);

	void OpenPrivateShop(uint8_t bTabCount = 1, bool bIsCashItem = false);
#else
public:
	void OpenPrivateShop(bool shopstate = true);
#endif

# ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
private:
	time_t m_dwLastSyncPrivateShopInfo;
public:
	void SyncPrivateShopTailInfo();
# endif
#endif

private:
	//China only
	//For under 18 years old only
	//3 hours: 50 % 5 hours 0%
	e_overtime m_eOverTime;

public:
	bool IsOverTime(e_overtime e) const noexcept { return (e == m_eOverTime); }
	void SetOverTime(e_overtime e) noexcept { m_eOverTime = e; }

private:
	int m_deposit_pulse;

public:
	void UpdateDepositPulse() noexcept;
	bool CanDeposit() const noexcept;

private:
	void __OpenPrivateShop();

public:
	struct AttackedLog
	{
		uint32_t dwPID;
		uint32_t dwAttackedTime;

		AttackedLog() noexcept : dwPID(0), dwAttackedTime(0)
		{
		}
	};

	AttackLog m_kAttackLog;
	AttackedLog m_AttackedLog;
	int m_speed_hack_count;

private:
	std::string m_strNewName;

public:
	const std::string GetNewName() const { return this->m_strNewName; }
	void SetNewName(const std::string name) { this->m_strNewName = name; }

public:
	void GoHome();

private:
	std::set<uint32_t> m_known_guild;

public:
	void SendGuildName(const CGuild* pGuild);
	void SendGuildName(uint32_t dwGuildID);

private:
	uint32_t m_dwLogOffInterval;

public:
	uint32_t GetLogOffInterval() const noexcept { return m_dwLogOffInterval; }

public:
	bool UnEquipSpecialRideUniqueItem();

	bool CanWarp() const noexcept;

protected:
	int LastCampFireUse; //@fixme502

private:
	uint32_t m_dwLastGoldDropTime;
#ifdef ENABLE_NEWSTUFF
	uint32_t m_dwLastItemDropTime;
	uint32_t m_dwLastBoxUseTime;
	uint32_t m_dwLastBuySellTime;
public:
	uint32_t GetLastBuySellTime() const noexcept { return m_dwLastBuySellTime; }
	void SetLastBuySellTime(uint32_t dwLastBuySellTime) noexcept { m_dwLastBuySellTime = dwLastBuySellTime; }
#endif

public:
	void AutoRecoveryItemProcess(const EAffectTypes);

public:
	void BuffOnAttr_AddBuffsFromItem(LPITEM pItem);
	void BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem);

private:
	void BuffOnAttr_ValueChange(uint16_t wType, uint8_t bOldValue, uint8_t bNewValue); //@fixme532
	void BuffOnAttr_ClearAll();

	typedef std::map <uint16_t, CBuffOnAttributes*> TMapBuffOnAttrs; //@fixme532
	TMapBuffOnAttrs m_map_buff_on_attrs;
	// Invincible: For smooth testing.
public:
	void SetArmada() noexcept { cannot_dead = true; }
	void ResetArmada() noexcept { cannot_dead = false; }
private:
	bool cannot_dead;

#ifdef __PET_SYSTEM__
private:
	bool m_bIsPet;
public:
	void SetPet() noexcept { m_bIsPet = true; }
	bool IsPet() noexcept { return m_bIsPet; }
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
private:
	bool m_bIsGrowthPet;
	int m_GrowthPetEggVID;
	int m_GrowthPetEvolution;
	bool m_GrowthPetHachWindowIsOpen;
	uint8_t m_GrowthPetWindowType;
protected:
	int m_iGrowthPetDetermineLoadTime;
public:
	void SetGrowthPet() noexcept { m_bIsGrowthPet = true; }
	bool IsGrowthPet() const noexcept { return m_bIsGrowthPet ? true : false; }

	void SetGrowthPetEggVID(int EggVID) noexcept { m_GrowthPetEggVID = EggVID; }
	int GetGrowthPetEggVID() noexcept { return m_GrowthPetEggVID; }
	void SetPetEvolution(int PetEvolution) noexcept { m_GrowthPetEvolution = PetEvolution; }
	int GetPetEvolution() noexcept { return m_GrowthPetEvolution; }

	void SendGrowthPetHatching(uint8_t bResult, uint8_t pos);
	void SetGrowthPetHatchingWindow(bool isOpen) noexcept { m_GrowthPetHachWindowIsOpen = isOpen; };
	bool IsGrowthPetHatchingWindow() const noexcept { return m_GrowthPetHachWindowIsOpen; };

	void SetPetWindowType(uint8_t pet_window_type);
	uint8_t GetPetWindowType() const noexcept { return m_GrowthPetWindowType; };

	void SendGrowthPetUpgradeSkillRequest(uint8_t bSkillSlot, uint8_t bSkillIndex, int iPrice);
#	ifdef ENABLE_PET_ATTR_DETERMINE
	void PetAttrChange(uint8_t bPetSlotIndex, uint8_t bMaterialSlotIndex);
	bool IsGrowthPetDetermineWindow() const noexcept { return m_GrowthPetWindowType == 1; };
	int GetGrowthPetDetermineLoadTime() const noexcept { return m_iGrowthPetDetermineLoadTime; }
	void SetGrowthPetDetermineLoadTime() noexcept { m_iGrowthPetDetermineLoadTime = thecore_pulse(); }
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	bool IsGrowthPetPrimiumFeedWindow() const noexcept { return m_GrowthPetWindowType == 2; };
	void RevivePet(const TPacketCGGrowthPetReviveRequest* revivePacket, LPITEM pSummonItem);
	void Revive(LPITEM pSummonItem, uint8_t bType);
#	endif
#endif

#ifdef NEW_ICEDAMAGE_SYSTEM
private:
	uint32_t m_dwNDRFlag;
	std::set<uint32_t> m_setNDAFlag;
public:
	const uint32_t GetNoDamageRaceFlag() noexcept;
	void SetNoDamageRaceFlag(uint32_t dwRaceFlag) noexcept;
	void UnsetNoDamageRaceFlag(uint32_t dwRaceFlag) noexcept;
	void ResetNoDamageRaceFlag() noexcept;
	const std::set<uint32_t>& GetNoDamageAffectFlag() noexcept;
	void SetNoDamageAffectFlag(uint32_t dwAffectFlag);
	void UnsetNoDamageAffectFlag(uint32_t dwAffectFlag);
	void ResetNoDamageAffectFlag() noexcept;
#endif

	//Final damage correction.
private:
	float m_fAttMul;
	float m_fDamMul;
public:
	float GetAttMul() noexcept { return this->m_fAttMul; }
	void SetAttMul(float newAttMul) noexcept { this->m_fAttMul = newAttMul; }
	float GetDamMul() noexcept { return this->m_fDamMul; }
	void SetDamMul(float newDamMul) noexcept { this->m_fDamMul = newDamMul; }

private:
	bool IsValidItemPosition(TItemPos Pos) const;

public:
	// Dragon Soul Stone

	// Do not call DragonSoul_Initialize before the affect or quest of the character is loaded.
	// affect is loaded last and called from LoadAffect.
	void DragonSoul_Initialize();

	bool DragonSoul_IsQualified() const noexcept;
	void DragonSoul_GiveQualification();

	int DragonSoul_GetActiveDeck() const noexcept;
	bool DragonSoul_IsDeckActivated() const noexcept;
	bool DragonSoul_ActivateDeck(int deck_idx);

	void DragonSoul_DeactivateAll();
	/* Must be called before ClearItem.
	* because....
	* Whenever you deactivate one Dragon Spirit Stone, check if there is an active Dragon Spirit Stone in your deck,
	* If there is no active dragon spirit stone, remove the character's dragon spirit stone affect and active state.
	*
	* However, when ClearItem, since all items worn by the character are unequiped,
	* Dragon Spirit Stone Affect is removed, and eventually, when you log in, Dragon Spirit Stone is not activated.
	* (When you unequip, you can't know whether you are logged out or not.)
	* Deactivate only the dragon spirit stone and do not touch the active state of the character's dragon spirit stone deck.
	*/
	void DragonSoul_CleanUp();
#ifdef ENABLE_DS_SET
	void DragonSoul_HandleSetBonus();
#endif

	// Dragon Spirit Stone Strengthening Spear
public:
	bool DragonSoul_RefineWindow_Open(LPENTITY pEntity);
	bool DragonSoul_RefineWindow_Close() noexcept;
	LPENTITY DragonSoul_RefineWindow_GetOpener() noexcept { return m_pointsInstant.m_pDragonSoulRefineWindowOpener; }
	bool DragonSoul_RefineWindow_CanRefine() noexcept;
#ifdef ENABLE_DS_CHANGE_ATTR
	bool DragonSoul_ChangeAttrWindow_Open(LPENTITY pEntity);
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
public:
	CEntity::ENTITY_MAP GetViewMap() const { return m_map_view; }
	void OpenPrivateShopSearch(uint32_t dwVnum);
	bool IsPrivateShopSearchOpen() const noexcept { return bPrivateShopSearchOpen; }
	void SetPrivateShopSearchOpen(bool bState) noexcept { bPrivateShopSearchOpen = bState; }
protected:
	bool bPrivateShopSearchOpen;
#endif

	// German gift function packet temporary storage
private:
	uint32_t itemAward_vnum;
	char itemAward_cmd[20];
	//bool itemAward_flag;
public:
	uint32_t GetItemAward_vnum() noexcept { return itemAward_vnum; }
	char* GetItemAward_cmd() noexcept { return itemAward_cmd; }
	//bool GetItemAward_flag() { return itemAward_flag; }
	void SetItemAward_vnum(uint32_t vnum) noexcept { itemAward_vnum = vnum; }
	void SetItemAward_cmd(const char* cmd) noexcept { strcpy(itemAward_cmd, cmd); }
	//void SetItemAward_flag(bool flag) { itemAward_flag = flag; }
#ifdef ENABLE_ANTI_CMD_FLOOD
private:
	int m_dwCmdAntiFloodPulse;
	uint32_t m_dwCmdAntiFloodCount;
public:
	int GetCmdAntiFloodPulse() noexcept { return m_dwCmdAntiFloodPulse; }
	uint32_t GetCmdAntiFloodCount() noexcept { return m_dwCmdAntiFloodCount; }
	uint32_t IncreaseCmdAntiFloodCount() noexcept { return ++m_dwCmdAntiFloodCount; }
	void SetCmdAntiFloodPulse(int dwPulse) noexcept { m_dwCmdAntiFloodPulse = dwPulse; }
	void SetCmdAntiFloodCount(uint32_t dwCount) noexcept { m_dwCmdAntiFloodCount = dwCount; }
#endif

	//@fixme503
	int waitHackCounter;
	void ClearWaitHackCounter(void) { waitHackCounter = 0; }
	void SetWaitHackCounter(void) { waitHackCounter = 1; }
	int GetWaitHackCounter(void) const { return waitHackCounter; }
	//@end_fixme503

private:
	// To defend against nuclear sending other users to strange places by exploiting SyncPosition,
	// Record when SyncPosition occurs.
	timeval m_tvLastSyncTime;
	int m_iSyncHackCount;

public:
	void SetLastSyncTime(const timeval& tv) noexcept { memcpy(&m_tvLastSyncTime, &tv, sizeof(timeval)); }
	const timeval& GetLastSyncTime() noexcept { return m_tvLastSyncTime; }
	void SetSyncHackCount(int iCount) noexcept { m_iSyncHackCount = iCount; }
	int GetSyncHackCount() noexcept { return m_iSyncHackCount; }


#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
public:
	void OpenLottoWindow();
	void SendLottoTicketInfo();
	void SendLottoBasicInfo();
	void SendLottoRankingInfo();

protected:
	uint32_t m_dwLastLottOpenMainTime;
	uint32_t m_dwLastLottoOpenRankingTime;
#endif

	// @fixme188 BEGIN
public:
	void ResetRewardInfo()
	{
		m_map_kDamage.clear();
		if (!IsPC())
			SetExp(0);
	}
	void DeadNoReward()
	{
		if (!IsDead())
			ResetRewardInfo();
		Dead();
	}
	// @fixme188 END

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
protected:
	bool m_bAcceCombination;
	bool m_bAcceAbsorption;

public:
	void OpenAcceCombination();
	void OpenAcceAbsorption();
	void AcceClose();
	bool AcceRefine(uint8_t bAcceWindow, uint8_t bSlotAcce, uint8_t BSlotMaterial);
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
public:
	bool MoveCostumeAttr(uint8_t bSlotMedium, uint8_t bSlotBase, uint8_t bSlotMaterial);
#endif

#ifdef ENABLE_BATTLE_FIELD
public:
	int GetBattlePoint() const noexcept { return m_points.battle_point; }
	void SetBattlePoint(int battle_point) noexcept { m_points.battle_point = battle_point; }

	uint8_t GetBattleDeadLimit() const noexcept { return m_bBattleDeadLimit; }
	void IncreaseBattleDeadLimit() noexcept { m_bBattleDeadLimit < 30 ? m_bBattleDeadLimit += 5 : 0; }

protected:
	uint8_t m_bBattleDeadLimit;
#endif

#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
public:
	int GetShopExUsablePoint(uint8_t coin_type) const noexcept { return m_points.aiShopExUsablePoint[coin_type]; }
	void SetShopExUsablePoint(uint8_t coin_type, int usable_point) noexcept { m_points.aiShopExUsablePoint[coin_type] = usable_point; }

	int GetShopExDailyTimePoint(uint8_t coin_type) const noexcept { return m_points.aiShopExDailyUse[coin_type]; }
	void SetShopExDailyTimePoint(uint8_t coin_type, int dayli_time) noexcept { m_points.aiShopExDailyUse[coin_type] = dayli_time; }
#endif

#ifdef ENABLE_BATTLE_FIELD
private:
	uint32_t dwBattleFieldPoints;
	typedef std::map<uint32_t, time_t> TBattleFieldKillMap;
	TBattleFieldKillMap m_BattleFieldKillMap;

public:
	uint32_t GetBattleFieldPoint() const noexcept { return dwBattleFieldPoints; }
	void SetBattleFieldPoint(uint32_t point) noexcept { dwBattleFieldPoints = point; }
	bool SetBattleKill(uint32_t dwPlayerID);
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
public:
	long GetMedalHonor() const noexcept { return m_points.medal_honor; }
	void SetMedalHonor(long medalHonor) noexcept { m_points.medal_honor = medalHonor; }
	void CheckMedals();
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
public:
	void SetTransmutation(CTransmutation* c);
	CTransmutation* IsChangeLookWindowOpen() const;

protected:
	CTransmutation* m_pkTransmutation;
#endif


#ifdef ENABLE_EXPRESSING_EMOTION
	struct EmotionsValues
	{
		int id_emotion;
		uint32_t tiempo_emotion;

	} save_info_emotion, copy_info_emotion;

public:
	void LoadingInfoEmotions();
	void LoadInfoEmotions();
	void LoadEmotions();
	bool CheckEmotionList(int emotion);
	int EmotionsList();
	void InsertEmotion();
	int CountEmotion();
	void StartCheckTimeEmotion();
	int get_time_emotion(int value);
	int get_id_emotion(int value);

private:
	LPEVENT TimeEmotionUpdateTime;
	std::vector<EmotionsValues> load_info_emotion;
#endif

#ifdef ENABLE_ATTR_6TH_7TH
public:
	bool IsOpenSkillBookComb() const noexcept { return m_isOpenSkillBookComb ? true : false; }
	void SetOpenSkillBookComb(bool b) noexcept {
		m_isOpenSkillBookComb = b;
#ifdef ENABLE_CHECK_WINDOW_RENEWAL
		SetOpenedWindow(W_ATTR_6TH_7TH, b);
#endif
	}

	void SetSkillBookCombType(int iType) { m_isSkillBookCombType = iType; }
	int GetSkillBookCombType() { return m_isSkillBookCombType; }

	void ItemAdded(LPITEM item) noexcept { m_AttrItemAdded = item; }
	LPITEM GetItemAdded() noexcept { return m_AttrItemAdded; }
	LPITEM GetAttr67InventoryItem() const noexcept;

private:
	bool m_isOpenSkillBookComb;
	int m_isSkillBookCombType;
	LPITEM m_AttrItemAdded;
#endif

#ifdef ENABLE_DEVIL_TOWER
public:
	bool IsDevilTowerDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_DEVILTOWER1 * 10000) && (lMapIndex < (MAP_DEVILTOWER1 + 1) * 10000)); }
#endif
#ifdef ENABLE_BLUEDRAGON_RENEWAL
public:
	bool IsBlueDragonDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_SKIPIA_DUNGEON_BOSS * 10000) && (lMapIndex < (MAP_SKIPIA_DUNGEON_BOSS + 1) * 10000)); }
#endif
#ifdef ENABLE_DEVIL_CATACOMBS
public:
	bool IsDevilCatacombsDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_DEVILCATACOMB * 10000) && (lMapIndex < (MAP_DEVILCATACOMB + 1) * 10000)); }
#endif
#ifdef ENABLE_SPIDER_DUNGEON
public:
	bool IsSpiderDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_SPIDERDUNGEON_03 * 10000) && (lMapIndex < (MAP_SPIDERDUNGEON_03 + 1) * 10000)); }
#endif
#ifdef ENABLE_FLAME_DUNGEON
public:
	bool IsFlameDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_N_FLAME_DUNGEON_01 * 10000) && (lMapIndex < (MAP_N_FLAME_DUNGEON_01 + 1) * 10000)); }
#endif
#ifdef ENABLE_SNOW_DUNGEON
public:
	bool IsSnowDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_N_SNOW_DUNGEON_01 * 10000) && (lMapIndex < (MAP_N_SNOW_DUNGEON_01 + 1) * 10000)); }
#endif

#ifdef ENABLE_DAWNMIST_DUNGEON
	LPEVENT m_pkTempleGuardianMoveEvent;
	LPEVENT m_pkHealerEvent;

	uint32_t GetBattleStartTime() const { return m_dwBattleStartTime; }
	uint32_t GetLastAttackedTime() const { return m_dwLastAttackedTime; }

	void SetBattleStartTime(uint32_t dwTime) { m_dwBattleStartTime = dwTime; }
	void SetLastAttackedTime(uint32_t dwTime) { m_dwLastAttackedTime = dwTime; }

	void SetSpecialSkillIndex(uint8_t bySpecialSkillIndex) noexcept { m_bySpecialSkillIndex = bySpecialSkillIndex; }
	uint8_t GetSpecialSkillIndex() noexcept { return m_bySpecialSkillIndex; }
	bool IsHealer() const noexcept;
#endif

#ifdef ENABLE_DEFENSE_WAVE
public:
	bool IsDefenseWaveDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_DEFENSEWAVE * 10000) && (lMapIndex < (MAP_DEFENSEWAVE + 1) * 10000)); }

	bool IsMast() const noexcept;
	bool IsShipSteeringWheel() const noexcept;
	bool IsDefenseWavePortal() const noexcept;

	bool IsHydraBoss() const noexcept;
	bool IsDefenseWaveMobs() const noexcept;
	bool IsDefenseWaveMastAttackMob(int iMobVnum = 0) const noexcept;
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
public:
	void SetNomove() { SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE); }
	void RemoveNomove() { REMOVE_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE); }
	bool IsNomove() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE); }

	void SetNoattack() { SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACK); }
	void RemoveNoattack() { REMOVE_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACK); }
	bool IsNoattack() const { return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACK); }

	void AggregateMonsterByMaster();
	void SetUniqueMaster(bool bInfo) { m_bIsUniqueMaster = bInfo; }
	bool GetUniqueMaster() const { return m_bIsUniqueMaster; }
	void SetDungeonMultipliers(uint8_t dungeonLevel);

	bool IsSungMahiDungeon(long lMapIndex) noexcept { return ((lMapIndex >= MAP_SMG_DUNGEON_02 * 10000) && (lMapIndex < (MAP_SMG_DUNGEON_02 + 1) * 10000)); }
	uint8_t GetSungMahiTowerDungeonValue(uint16_t wPoint);

protected:
	bool m_bIsUniqueMaster;
#endif

#ifdef ENABLE_REFINE_ELEMENT
public:
	bool ElementsSpellItem(LPITEM pkItem, LPITEM pkTarget);
	bool IsOpenElementsSpell() const noexcept { return m_OpenElementsSpell ? true : false; }
	void SetOpenElementsSpell(bool b, int iAdditionalCell = -1);
	void ElementsSpellItemFunc(int pos, uint8_t type_select = -1);
	uint8_t GetElementsEffect();

private:
	bool m_OpenElementsSpell;
	int m_iElementsAdditionalCell;
#endif
#ifdef ENABLE_AFTERDEATH_SHIELD
public:
	void StartShieldCountdownEvent(LPCHARACTER ch, int iSec);
	void SetShieldAffect() { m_afAffectFlag.Set(AFF_AFTERDEATH_SHIELD); };
	void RemoveShieldAffect() { return m_afAffectFlag.Reset(AFF_AFTERDEATH_SHIELD); };
#endif

#ifdef ENABLE_AURA_SYSTEM
private:
	uint8_t m_bAuraRefineWindowType;
	bool m_bAuraRefineWindowOpen;
	TItemPos m_pAuraRefineWindowItemSlot[AURA_SLOT_MAX];
	TAuraRefineInfo m_bAuraRefineInfo[AURA_REFINE_INFO_SLOT_MAX];

protected:
#	ifdef ENABLE_AURA_BOOST
	uint8_t __GetAuraAbsorptionRate(uint8_t bLevel, uint8_t bBoostIndex) const;
#	endif
	TAuraRefineInfo __GetAuraRefineInfo(TItemPos Cell);
	TAuraRefineInfo __CalcAuraRefineInfo(TItemPos Cell, TItemPos MaterialCell);
	TAuraRefineInfo __GetAuraEvolvedRefineInfo(TItemPos Cell);

public:
	void OpenAuraRefineWindow(LPENTITY pOpener, EAuraWindowType type);
	bool IsAuraRefineWindowOpen() const noexcept { return m_bAuraRefineWindowOpen; }
	uint8_t GetAuraRefineWindowType() const noexcept { return m_bAuraRefineWindowType; }
	LPENTITY GetAuraRefineWindowOpener() noexcept { return m_pointsInstant.m_pAuraRefineWindowOpener; }

	bool IsAuraRefineWindowCanRefine();

	void AuraRefineWindowCheckIn(uint8_t bAuraRefineWindowType, TItemPos AuraCell, TItemPos ItemCell);
	void AuraRefineWindowCheckOut(uint8_t bAuraRefineWindowType, TItemPos AuraCell);
	void AuraRefineWindowAccept(uint8_t bAuraRefineWindowType);
	void AuraRefineWindowClose();
#endif

#ifdef ENABLE_12ZI
public:
	void BeadTime();
	void MarkTime();

	void SetAnimaSphere(int amount);
	int GetAnimaSphere();

	void IsZodiacEffectMob();
	void IsZodiacEffectPC(uint32_t Monster);

	void ZodiacFloorMessage(uint8_t Floor);

	void SetLastZodiacAttackTime(uint32_t time) { m_dwLastZodiacAttackTime = time; }
	uint32_t GetLastZodiacAttackTime() const { return m_dwLastZodiacAttackTime; }

	void IncDeadCount() { m_dwDeadCount++; }
	void SetDeadCount(uint32_t dead) { m_dwDeadCount = dead; }
	uint32_t GetDeadCount() const { return m_dwDeadCount; }

	void EffectZodiacPacket(long X, long Y, int enumEffectType, int enumEffectType2 = 0);

	bool IsZodiacBoss();
	bool IsZodiacOfficer();
	bool IsZodiacStatue();
	uint16_t GetStatueVnum() const;

	LPEVENT m_pkZodiacSkill1;
	LPEVENT m_pkZodiacSkill2;
	LPEVENT m_pkZodiacSkill3;
	LPEVENT m_pkZodiacSkill4;
	LPEVENT m_pkZodiacSkill5;
	LPEVENT m_pkZodiacSkill6;
	LPEVENT m_pkZodiacSkill7;
	LPEVENT m_pkZodiacSkill8;
	LPEVENT m_pkZodiacSkill9;
	LPEVENT m_pkZodiacSkill10;
	LPEVENT m_pkZodiacSkill11;

	void ZodiacDamage(uint8_t Type, LPCHARACTER Victim = nullptr);

	int ZodiacSkillAttack(LPCHARACTER Victim, uint32_t dwVnum, uint8_t bSkillLevel);

	uint32_t CountZodiacItems(uint32_t Vnum);
	void SetZodiacItems(uint32_t Vnum, int Count);

	uint32_t PurchaseCountZodiacItems(uint32_t Vnum);
	void SetPurchaseZodiacItems(uint32_t Vnum, int Count);

	void CanonDamage();

	void SpawnZodiacGroup(LPZODIAC pZodiac);
	void SpawnZodiacStone(LPZODIAC pZodiac);

	void ZTT_CHECK_BOX(int color, int index);
	void ZTT_LOAD_INFO();
	int ZTT_CHECK_ITEM_ROW(int color, int index);
	int ZTT_CHECK_ITEM_COLUMN(int color, int index);
	void ZTT_CHECK_REWARD();
	void ZTT_REWARD(int type);

	uint16_t IsZodiacCannon() { return GetRaceNum() == 20464; }

private:
	uint32_t m_dwLastZodiacAttackTime;
	uint32_t m_dwDeadCount;
#endif

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
public:
	void SetHideCostumePulse(int iPulse) { m_HideCostumePulse = iPulse; }
	int GetHideCostumePulse() { return m_HideCostumePulse; }

	void SetBodyCostumeHidden(bool hidden) noexcept;
	bool IsBodyCostumeHidden() const noexcept { return m_bHideBodyCostume; };

	void SetHairCostumeHidden(bool hidden) noexcept;
	bool IsHairCostumeHidden() const noexcept { return m_bHideHairCostume; };

# ifdef ENABLE_ACCE_COSTUME_SYSTEM
	void SetAcceCostumeHidden(bool hidden) noexcept;
	bool IsAcceCostumeHidden() const noexcept { return m_bHideAcceCostume; };
# endif

# ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	void SetWeaponCostumeHidden(bool hidden) noexcept;
	bool IsWeaponCostumeHidden() const noexcept { return m_bHideWeaponCostume; };
# endif

# ifdef ENABLE_AURA_SYSTEM
	void SetAuraCostumeHidden(bool hidden) noexcept;
	bool IsAuraCostumeHidden() const noexcept { return m_bHideAuraCostume; };
# endif

private:
	int m_HideCostumePulse;
	bool m_bHideBodyCostume;
	bool m_bHideHairCostume;
	bool m_bHideAcceCostume;
	bool m_bHideWeaponCostume;
	bool m_bHideAuraCostume;
#endif

#ifdef ENABLE_GUILDWAR_BUTTON
public:
	void SendWarTeleportButton(bool show = false);
#endif

#ifdef ENABLE_SET_ITEM
public:
	void RefreshSetBonus();
#endif

#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT_ADD) || defined(ENABLE_REFINE_ELEMENT)
public:
	uint16_t GetPointElementSum();
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
public:
	uint8_t GetAlignRank();
#endif

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
public:
	std::vector<THitCountInfo> m_hitCount;
	void RewardAttendance();
#endif // ENABLE_MONSTER_BACK

#ifdef ENABLE_CHECK_WINDOW_RENEWAL
private:
	uint32_t m_dwOpenedWindow;

public:
	uint32_t GetOpenedWindow(uint32_t dwWindow) const noexcept;
	void SetOpenedWindow(uint32_t dwWindow, bool bIsOpen) noexcept;
#endif

#ifdef ENABLE_HORSESKILLS_ON_MOUNTS
protected:
	uint32_t m_dwHorseChangeLookVnum;

public:
	int GetHorseChangeLookVnum() const noexcept { return m_dwHorseChangeLookVnum; }
	void SetHorseChangeLookVnum(uint32_t dwHorseChangeLookVnum) noexcept { m_dwHorseChangeLookVnum = dwHorseChangeLookVnum; }
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
public:
	bool GetMultiStatus() { return m_bmultiFarmStatus; }
	void SetMultiStatus(bool bValue) { m_bmultiFarmStatus = bValue; }

	void SetProtectTime(const std::string& flagname, int value);
	int GetProtectTime(const std::string& flagname) const;

protected:
	bool m_bmultiFarmStatus;
	std::map<std::string, int>  m_protection_Time;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
public:
	void SetSkillColor(uint32_t* dwSkillColor);
	uint32_t* GetSkillColor() { return m_dwSkillColor[0]; }

protected:
	uint32_t m_dwSkillColor[ESkillColorLength::MAX_SKILL_COUNT + ESkillColorLength::MAX_BUFF_COUNT][ESkillColorLength::MAX_EFFECT_COUNT];
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
public:
	void SetDungeonInfoOpen(bool bOpen = true) { m_bDungeonInfoOpen = bOpen; };
	bool IsDungeonInfoOpen() { return m_bDungeonInfoOpen; }

	void StartDungeonInfoReloadEvent();
	void StopDungeonInfoReloadEvent();

	bool UpdateDungeonRanking(const std::string c_strQuestName);

	void SetLastDamage(int iLastDamage) { m_iLastDamage = iLastDamage; }
	int GetLastDamage() { return m_iLastDamage; }

protected:
	bool m_bDungeonInfoOpen;
	LPEVENT m_pkDungeonInfoReloadEvent;
	int m_iLastDamage;
#endif

#ifdef ENABLE_MAILBOX
public:
	int GetMyMailBoxTime() const { return m_iMyMailBoxTime; }
	void SetMyMailBoxTime() { m_iMyMailBoxTime = thecore_pulse(); }
	void SetMailBox(CMailBox* m);
	void SetMailBoxLoading(const bool b) { bMailBoxLoading = b; }
	bool IsMailBoxLoading() const { return bMailBoxLoading; }
	CMailBox* GetMailBox() const { return m_pkMailBox; }

private:
	CMailBox* m_pkMailBox;
	bool bMailBoxLoading;
	int m_iMyMailBoxTime;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
typedef std::list<TPlayerExtBattlePassMission*> ListExtBattlePassMap;

public:
	void LoadExtBattlePass(uint32_t dwCount, TPlayerExtBattlePassMission* data);
	uint32_t GetExtBattlePassMissionProgress(uint32_t dwBattlePassType, uint8_t bMissionIndex, uint8_t bMissionType);
	bool IsExtBattlePassCompletedMission(uint32_t dwBattlePassType, uint8_t bMissionIndex, uint8_t bMissionType);
	bool IsExtBattlePassRegistered(uint8_t bBattlePassType, uint32_t dwBattlePassID);
	void UpdateExtBattlePassMissionProgress(uint32_t dwMissionID, uint32_t dwUpdateValue, uint32_t dwCondition, bool isOverride = false);
	void SetExtBattlePassMissionProgress(uint8_t bBattlePassType, uint32_t dwMissionIndex, uint32_t dwMissionType, uint32_t dwUpdateValue);

	bool IsLoadedExtBattlePass() const { return m_bIsLoadedExtBattlePass; }
	int GetExtBattlePassPremiumID()	const { return m_points.battle_pass_premium_id; }
	void SetExtBattlePassPremiumID(int battle_pass_premium_id) { m_points.battle_pass_premium_id = battle_pass_premium_id; }

	void SetLastReciveExtBattlePassInfoTime(uint32_t time);
	uint32_t GetLastReciveExtBattlePassInfoTime() const { return m_dwLastReciveExtBattlePassInfoTime; }
	void SetLastReciveExtBattlePassOpenRanking(uint32_t time);
	uint32_t GetLastReciveExtBattlePassOpenRanking() const { return m_dwLastExtBattlePassOpenRankingTime; }

protected:
	uint32_t m_dwLastReciveExtBattlePassInfoTime;
	uint32_t m_dwLastExtBattlePassOpenRankingTime;

private:
	bool m_bIsLoadedExtBattlePass;
	ListExtBattlePassMap m_listExtBattlePass;
#endif

#ifdef ENABLE_FISHING_RENEWAL
public:
	LPEVENT m_pkFishingNewEvent;

	void fishing_new_start();
	void fishing_new_stop();
	void fishing_new_catch();
	void fishing_new_catch_failed();
	void fishing_catch_decision(uint32_t itemVnum);
	void SetFishCatch(int i) noexcept { m_bFishCatch = i; }
	uint8_t GetFishCatch() noexcept { return m_bFishCatch; }
	void SetLastCatchTime(uint32_t i) noexcept { m_dwLastCatch = i; }
	int GetLastCatchTime() noexcept { return m_dwLastCatch; }
	void SetFishCatchFailed(int i) noexcept { m_dwCatchFailed = i; }
	uint32_t GetFishCatchFailed() noexcept { return m_dwCatchFailed; }

private:
	uint8_t m_bFishCatch;
	uint32_t m_dwCatchFailed;
	int m_dwLastCatch;
#endif

#ifdef ENABLE_MOUNT_CHECK
public:
	void Unmount(LPCHARACTER ch);
#endif

public:
	void EquipItems(LPCHARACTER ch);
	LPCHARACTER GetHighestDpsVictim(LPCHARACTER pkChr); //@fixme512 (Optional)
	bool UnEquipCostumeMoudeItem(bool isOnDeath);
	bool IsInSafezone() const;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
public:
	void SetMeleyLair(CMeleyLair* pkMeleyLairDungeon) noexcept { m_pkMeleyLairDungeon = pkMeleyLairDungeon; };
	CMeleyLair* GetMeleyLair() noexcept { return m_pkMeleyLairDungeon; };
protected:
	CMeleyLair* m_pkMeleyLairDungeon;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
public:
	bool IsLearnableSkillAchievement(uint32_t dwSkillVnum) const;
	uint32_t GetAchievementTitle() const { return m_dwSelectedTitle; }
	uint32_t GetAchievementPoints() const { return m_dwAchievementPoints; }

	void SetAchievementTitle(uint32_t title) { m_dwSelectedTitle = title; }
	void SetAchievementPoints(uint32_t points) { m_dwAchievementPoints = points; }

	void SetAchievementInfo(TAchievementsMap& achievement);
	std::unique_ptr<TAchievementsMap>& GetAchievementInfo();

	std::vector<uint32_t>& GetAchievementTitles() { return _achievement_titles; }

	uint32_t GetStartPlayTime() const { return m_dwPlayStartTime; }

private:
	std::unique_ptr<TAchievementsMap> _achievement;
	std::vector<uint32_t> _achievement_titles;
	uint32_t m_dwAchievementPoints;
	uint32_t m_dwSelectedTitle;
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
protected:
	uint8_t m_BiologActualMission;
	uint16_t m_BiologCollectedItems;
	uint8_t m_BiologCooldownReminder;
	long m_BiologCooldown;
	bool m_BiologReminderEventState;

public:
	LPEVENT s_pkReminderEvent;
	CBiologSystem* GetBiologManager() const noexcept { return m_pkBiologManager; }

	uint8_t GetBiologMissions() const noexcept { return m_BiologActualMission; }
	uint16_t GetBiologCollectedItems() const noexcept { return m_BiologCollectedItems; }
	uint8_t GetBiologCooldownReminder() const noexcept { return m_BiologCooldownReminder; }
	long GetBiologCooldown() const noexcept { return m_BiologCooldown; }

	void SetBiologMissions(uint8_t value) noexcept { m_BiologActualMission = value; }
	void SetBiologCollectedItems(uint16_t value) noexcept { m_BiologCollectedItems = value; }
	void SetBiologCooldownReminder(uint8_t value);
	void SetBiologCooldown(long value) noexcept { m_BiologCooldown = value; }

	bool IsBiologRemiderEvent() const noexcept { return m_BiologReminderEventState; }
	void SetBiologRemiderEvent(bool state) noexcept { m_BiologReminderEventState = state; }

protected:
	CBiologSystem* m_pkBiologManager;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
public:
	LPITEM GetAdditionalEquipmentItem(uint16_t wCell) const;
	void SetTimeChangeEquip(int time) { time_change_equipment_page = time; }
	int GetTimeChangeEquip() { return time_change_equipment_page; }
	uint32_t GetLastSkillTime() const { return m_dwLastSkillTime; }
	bool CheckAdditionalEquipment(uint16_t cell) const;

	void SetWearDefault(uint8_t bCell, LPITEM item);
	LPITEM GetWearDefault(uint8_t bCell) const;

	bool GetWearNotChange(uint16_t cell) const;

	uint8_t GetPageEquipment() const { return m_points.page_equipment; }
	void SetPageEquipment(uint8_t page) { m_points.page_equipment = page; }

	uint8_t GetPageTotalEquipment() const { return m_points.unlock_page_equipment; }
	void SetPageTotalEquipment(uint8_t page_total) { m_points.unlock_page_equipment = page_total; }

private:
	int time_change_equipment_page;
#endif

#ifdef ENABLE_HUNTING_SYSTEM
public:
	void CheckHunting(bool isLevelUp = 0);
	void OpenHuntingWindowMain();
	void OpenHuntingWindowSelect();
	void OpenHuntingWindowReward();

	void UpdateHuntingMission(uint32_t dwMonsterVnum);
	void ReciveHuntingRewards();
	void SetCachedRewards();

	void SetRewardRandomItemFromTable();
	void SendRandomItemPacket(bool IsSelectWindow);

	int GetRaceTable();
	int GetMinMaxMoney(uint32_t missionLevel, bool AskMax);
	int GetRandomMoney(uint32_t missionLevel);
	int GetMinMaxExp(uint32_t missionLevel, bool AskMax);
	int GetRandomExp(uint32_t missionLevel);
#endif

#ifdef ENABLE_AUTO_RESTART_EVENT
public:
	bool autohunt_restart;
#endif

#ifdef ENABLE_EVENTS
# ifdef ENABLE_MINI_GAME_OKEY_NORMAL
public:
	struct S_CARD
	{
		uint32_t type;
		uint32_t value;
	};

	struct CARDS_INFO
	{
		S_CARD cards_in_hand[EMonsterOkeyCardEvent::HAND_CARD_INDEX_MAX];
		S_CARD cards_in_field[EMonsterOkeyCardEvent::FIELD_CARD_INDEX_MAX];
		uint32_t cards_left;
		uint32_t field_points;
		uint32_t points;
	};

	void Cards_open(uint8_t safemode);
	void Cards_clean_list();
	uint32_t GetEmptySpaceInHand();
	void Cards_pullout();
	void RandomizeCards();
	bool CardWasRandomized(uint32_t type, uint32_t value);
	void SendUpdatedInformations();
	void SendReward();
	void CardsDestroy(uint32_t reject_index);
	void CardsAccept(uint32_t accept_index);
	void CardsRestore(uint32_t restore_index);
	uint32_t GetEmptySpaceInField();
	uint32_t GetAllCardsCount();
	bool TypesAreSame();
	bool ValuesAreSame();
	bool CardsMatch();
	uint32_t GetLowestCard();
	bool CheckReward();
	void CheckCards();
	void RestoreField();
	void ResetField();
	void CardsEnd();
	void GetGlobalRank(char* buffer, size_t buflen);
	void GetRundRank(char* buffer, size_t buflen);
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	void GetGlobalRankRoulette(char* buffer, size_t buflen);
#endif

protected:
	CARDS_INFO character_cards;
	S_CARD randomized_cards[EMonsterOkeyCardEvent::DECK_COUNT_MAX];
# endif // ENABLE_MINI_GAME_OKEY_NORMAL

# ifdef ENABLE_MINI_GAME_CATCH_KING
public:
	void MiniGameCatchKingSetFieldCards(std::vector<TCatchKingCard> vec) { m_vecCatchKingFieldCards = vec; }

	uint32_t MiniGameCatchKingGetScore() const noexcept { return dwCatchKingTotalScore; }
	void MiniGameCatchKingSetScore(uint32_t dwScore) noexcept { dwCatchKingTotalScore = dwScore; }

	uint32_t MiniGameCatchKingGetBetNumber() const noexcept { return bCatchKingBetSetNumber; }
	void MiniGameCatchKingSetBetNumber(uint8_t bSetNr) noexcept { bCatchKingBetSetNumber = bSetNr; }

	uint8_t MiniGameCatchKingGetHandCard() const noexcept { return bCatchKingHandCard; }
	void MiniGameCatchKingSetHandCard(uint8_t bKingCard) noexcept { bCatchKingHandCard = bKingCard; }

	uint8_t MiniGameCatchKingGetHandCardLeft() const noexcept { return bCatchKingHandCardLeft; }
	void MiniGameCatchKingSetHandCardLeft(uint8_t bHandCard) noexcept { bCatchKingHandCardLeft = bHandCard; }

	bool MiniGameCatchKingGetGameStatus() const noexcept { return dwCatchKingGameStatus; }
	void MiniGameCatchKingSetGameStatus(bool bStatus) noexcept { dwCatchKingGameStatus = bStatus; }

	std::vector<TCatchKingCard> m_vecCatchKingFieldCards;

protected:
	uint8_t bCatchKingHandCard;
	uint8_t bCatchKingHandCardLeft;
	bool dwCatchKingGameStatus;
	uint8_t bCatchKingBetSetNumber;
	uint32_t dwCatchKingTotalScore;
# endif // ENABLE_MINI_GAME_CATCH_KING

# ifdef ENABLE_SUMMER_EVENT_ROULETTE
public:
	INT GetSoulPoint() const { return m_points.soul; }
	void SetSoulPoint(INT soul) { m_points.soul = soul; }
	INT GetSoulRePoint() const { return m_points.soulre; }
	void SetSoulRePoint(INT soulre) { m_points.soulre = soulre; }
# endif

# ifdef ENABLE_FISH_EVENT
private:
	uint8_t m_dwFishUseCount;
	uint8_t m_bFishAttachedShape;

public:
	uint8_t GetFishEventUseCount() const noexcept { return m_dwFishUseCount; }
	void FishEventIncreaseUseCount() noexcept { m_dwFishUseCount++; }

	uint8_t GetFishAttachedShape() const noexcept { return m_bFishAttachedShape; }
	void SetFishAttachedShape(uint8_t bShape) noexcept { m_bFishAttachedShape = bShape; }

	void FishEventGeneralInfo();
	void FishEventUseBox(TItemPos itemPos);
	bool FishEventIsValidPosition(uint8_t shapePos, uint8_t shapeType);
	void FishEventPlaceShape(uint8_t shapePos, uint8_t shapeType);
	void FishEventAddShape(uint8_t shapePos);
	void FishEventCheckEnd();
# endif // ENABLE_FISH_EVENT

#ifdef ENABLE_MINI_GAME_BNW
public:
	bool MiniGameBNWGetGameStatus() const { return m_bBNWGameStatus; }
	void MiniGameBNWSetGameStatus(bool bStatus) { m_bBNWGameStatus = bStatus; }

	std::vector<TBNWCard> m_vecBNWOpponentCards;
	std::vector<TBNWCard> m_vecBNWPlayerCards;

	void MiniGameBNWSetPlayerScore(uint8_t bScore) { m_bPlayerScore = bScore; }
	void MiniGameBNWSetOpponentScore(uint8_t bScore) { m_bOpponentScore = bScore; }

	uint8_t MiniGameBNWGetPlayerScore() { return m_bPlayerScore; }
	uint8_t MiniGameBNWGetOpponentScore() { return m_bOpponentScore; }

protected:
	bool m_bBNWGameStatus;
	uint8_t m_bPlayerScore;
	uint8_t m_bOpponentScore;
#endif // ENABLE_MINI_GAME_BNW

#ifdef ENABLE_MINI_GAME_FINDM
public:
	void MiniGameFindMStartGame(std::vector<TFindMCard> cardsVector, uint32_t dwStartTime);

	bool MiniGameFindMGetGameStatus() const noexcept { return bFindMGameStatus; }
	void MiniGameFindMSetGameStatus(bool bStatus) noexcept { bFindMGameStatus = bStatus; }

	uint16_t MiniGameFindMGetTryCount() const noexcept { return wFindMTryCount; }
	void MiniGameFindMSetTryCount(uint16_t wCount) noexcept { wFindMTryCount = wCount; }

	uint32_t MiniGameFindMGetNextReveal() const noexcept { return dwFindMNextReveal; }
	uint32_t MiniGameFindMGetStartTime() const noexcept { return dwFindMStartTime; }

	uint32_t MiniGameFindMGetRewardVnum() const noexcept { return dwFindMRewardVnum; }
	void MiniGameFindMSetRewardVnum(uint32_t dwVnum) noexcept { dwFindMRewardVnum = dwVnum; }

	uint32_t MiniGameFindMGetRewardCost() const noexcept { return dwFindMRewardCost; }
	void MiniGameFindMSetRewardCost(uint32_t dwVnum) noexcept { dwFindMRewardCost = dwVnum; }

	void MiniGameFindMStartReveal(int seconds);
	bool MiniGameFindMIsAllRevealed();

	std::vector<TFindMCard> m_vecFindMFieldCards;
	int iRevealedIndex[2];

protected:
	uint16_t wFindMTryCount;
	bool bFindMGameStatus;
	uint32_t dwFindMStartTime;
	uint32_t dwFindMNextReveal;
	uint32_t dwFindMRewardVnum;
	uint32_t dwFindMRewardCost;
	LPEVENT m_revealEvent;

#ifdef ENABLE_MINI_GAME_FINDM_HINT
public:
	uint8_t MiniGameFindMGetHintCount() const noexcept { return bFindMHintCount; }
	void MiniGameFindMSetHintCount(uint8_t bCount) noexcept { bFindMHintCount = bCount; }

protected:
	uint8_t bFindMHintCount;
#endif
#endif // ENABLE_MINI_GAME_FINDM

#ifdef ENABLE_MINI_GAME_YUTNORI
public:
	bool YutNoriGetGameStatus() const { return bYutNoriGameStatus; }
	void YutNoriSetGameStatus(bool bStatus) { bYutNoriGameStatus = bStatus; }

	bool YutNoriIsPcTurn() const { return bYutNoriIsPcTurn; }
	void YutNoriSetPcTurn(bool bStatus) { bYutNoriIsPcTurn = bStatus; }

	bool YutNoriIsStartThrow() const { return bYutNoriIsStartThrow; }
	void YutNoriSetStartThrow(bool bStatus) { bYutNoriIsStartThrow = bStatus; }

	char YutNoriGetThrowYut(uint8_t bPlayerIndex) const { return cYutNoriThrowYut[bPlayerIndex]; }
	void YutNoriSetThrowYut(char cYut, uint8_t bPlayerIndex) { cYutNoriThrowYut[bPlayerIndex] = cYut; }

	char YutNoriGetUnitPos(uint8_t bPlayerIndex, uint8_t bUnitIndex) const { return cYutNoriUnitPos[bPlayerIndex][bUnitIndex]; }
	void YutNoriSetUnitPos(char cPos, uint8_t bPlayerIndex, uint8_t bUnitIndex) { cYutNoriUnitPos[bPlayerIndex][bUnitIndex] = cPos; }

	char YutNoriGetUnitLastPos(uint8_t bPlayerIndex, uint8_t bUnitIndex) const { return cYutNoriUnitLastPos[bPlayerIndex][bUnitIndex]; }
	void YutNoriSetUnitLastPos(char cPos, uint8_t bPlayerIndex, uint8_t bUnitIndex) { cYutNoriUnitLastPos[bPlayerIndex][bUnitIndex] = cPos; }

	char YutNoriGetWhoIsNextCom() const { return cYutNoriWhoIsNextCom; }
	void YutNoriSetWhoIsNextCom(char cUnitIndex) { cYutNoriWhoIsNextCom = cUnitIndex; }

	uint16_t YutNoriGetScore() const { return wYutNoriScore; }
	void YutNoriSetScore(uint16_t wScore) { wYutNoriScore = wScore; }

	uint8_t YutNoriGetRemainCount() const { return bYutNoriRemainCount; }
	void YutNoriSetRemainCount(uint8_t bRemainCount) { bYutNoriRemainCount = bRemainCount; }

	uint8_t YutNoriGetProbIndex() const { return bYutNoriProbIndex; }
	void YutNoriSetProbIndex(uint8_t bProbIndex) { bYutNoriProbIndex = bProbIndex; }

	bool YutNoriIsReThrow() const { return bYutNoriIsReThrow; }
	void YutNoriSetReThrow(bool bStatus) { bYutNoriIsReThrow = bStatus; }

	bool YutNoriGetRewardStatus() const { return bYutNoriRewardStatus; }
	void YutNoriSetRewardStatus(bool bStatus) { bYutNoriRewardStatus = bStatus; }

protected:
	bool bYutNoriGameStatus;
	bool bYutNoriIsPcTurn;
	bool bYutNoriIsStartThrow;
	char cYutNoriThrowYut[YUT_NORI_PLAYER_MAX];
	char cYutNoriUnitPos[YUT_NORI_PLAYER_MAX][YUT_NORI_UNIT_MAX];
	char cYutNoriUnitLastPos[YUT_NORI_PLAYER_MAX][YUT_NORI_UNIT_MAX];
	char cYutNoriWhoIsNextCom;

	uint16_t wYutNoriScore;
	uint8_t bYutNoriRemainCount;
	uint8_t bYutNoriProbIndex;

	bool bYutNoriIsReThrow;
	bool bYutNoriRewardStatus;
#endif // ENABLE_MINI_GAME_YUTNORI

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
public:
	void SetSoulRoulette(CSoulRoulette* pt);
	CSoulRoulette* GetSoulRoulette() const { return pSoulRoulette; }
private:
	CSoulRoulette* pSoulRoulette;
#endif // ENABLE_SUMMER_EVENT_ROULETTE

#ifdef ENABLE_FLOWER_EVENT
public:
	void FlowerSystem(LPITEM item);
	void RefreshFlowerAffect(uint32_t dwType, uint16_t wApplyOn, int iFlowerType, uint8_t bIncreaseBonus, uint32_t flowerStage, long lTimeExpire);

	void SetFlowerEventValue(uint8_t id, uint32_t value);
	uint32_t GetFlowerEventValue(uint8_t id) noexcept;
	void SendFlowerEventExchange(uint8_t id);

protected:
	TFlowerEvent* m_FlowerEvent{};
#endif // ENABLE_FLOWER_EVENT

#ifdef ENABLE_WORLD_BOSS
public:
	void SetTier(uint8_t bTier) { m_pTier = bTier; };
	uint8_t GetTier() { return m_pTier; };
	void SetWBRewards(bool bGaveRewards) { m_pGotRewards = bGaveRewards; };
	bool GotWBRewards() { return m_pGotRewards; };

private:
	uint8_t m_pTier;
	bool m_pGotRewards;
#endif // ENABLE_WORLD_BOSS
#endif // ENABLE_EVENTS
};

ESex GET_SEX(const LPCHARACTER& ch) noexcept;

#endif
