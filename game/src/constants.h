#ifndef __INC_METIN_II_GAME_CONSTANTS_H__
#define __INC_METIN_II_GAME_CONSTANTS_H__

#include "../../common/tables.h"

enum EMonsterChatState
{
	MONSTER_CHAT_WAIT,
	MONSTER_CHAT_ATTACK,
	MONSTER_CHAT_CHASE,
	MONSTER_CHAT_ATTACKED,
};

typedef struct SMobRankStat
{
	int iGoldPercent;
} TMobRankStat;

typedef struct SMobStat
{
	uint8_t byLevel;
	uint16_t HP;
	uint32_t dwExp;
	uint16_t wDefGrade;
} TMobStat;

#ifdef ENABLE_YOHARA_SYSTEM
typedef struct SRandomJewelry
{
	uint8_t byLevel;
	uint8_t byRandomValue[4];
} TRandomJewelry;

typedef struct SRandomMysterious
{
	uint8_t byLevel;
	uint8_t byRandomValue[30];
} TRandomMysterious;

typedef struct SRandomGloveInfo
{
	uint8_t byLevel;
	uint8_t byRandomValue[5];
} TRandomGloveInfo;

// Snake
typedef struct SRandomSnakeGloveInfo
{
	uint8_t byLevel;
	uint8_t byRandomValue[4];
} TRandomSnakeGloveInfo;

typedef struct SRandomSnakeArmorInfo
{
	uint32_t refineNumber;
	int iRefineValues[16][5];
} TRandomSnakeArmorInfo;

typedef struct SRandomSnakeWeaponInfo
{
	uint32_t refineNumber;
	int iRefineValues[24][3];
} TRandomSnakeWeaponInfo;
#endif

typedef struct SBattleTypeStat
{
	int AttGradeBias;
	int DefGradeBias;
	int MagicAttGradeBias;
	int MagicDefGradeBias;
} TBattleTypeStat;

typedef struct SJobInitialPoints
{
	int st, ht, dx, iq;
	int max_hp, max_sp;
	int hp_per_ht, sp_per_iq;
	int hp_per_lv_begin, hp_per_lv_end;
	int sp_per_lv_begin, sp_per_lv_end;
	int max_stamina;
	int stamina_per_con;
	int stamina_per_lv_begin, stamina_per_lv_end;
} TJobInitialPoints;

typedef struct __coord
{
	int x, y;
} Coord;

typedef struct SApplyInfo
{
	uint16_t wPointType; //@fixme532
} TApplyInfo;

enum {
	FORTUNE_BIG_LUCK,
	FORTUNE_LUCK,
	FORTUNE_SMALL_LUCK,
	FORTUNE_NORMAL,
	FORTUNE_SMALL_BAD_LUCK,
	FORTUNE_BAD_LUCK,
	FORTUNE_BIG_BAD_LUCK,
	FORTUNE_MAX_NUM,
};

const int STONE_INFO_MAX_NUM = 10;
const int STONE_LEVEL_MAX_NUM = 4;
#ifdef ENABLE_YOHARA_SYSTEM
const int SUNGMA_STONE_INFO_MAX_NUM = 7;
#endif

struct SStoneDropInfo
{
	uint32_t dwMobVnum;
	int iDropPct;
	int iLevelPct[STONE_LEVEL_MAX_NUM + 1];
};

inline bool operator < (const SStoneDropInfo& l, uint32_t r)
{
	return l.dwMobVnum < r;
}

inline bool operator < (uint32_t l, const SStoneDropInfo& r)
{
	return l < r.dwMobVnum;
}

inline bool operator < (const SStoneDropInfo& l, const SStoneDropInfo& r)
{
	return l.dwMobVnum < r.dwMobVnum;
}

extern const TApplyInfo aApplyInfo[MAX_APPLY_NUM];
extern const TMobRankStat MobRankStats[MOB_RANK_MAX_NUM];

extern TBattleTypeStat BattleTypeStats[BATTLE_TYPE_MAX_NUM];

extern const uint32_t party_exp_distribute_table[PLAYER_EXP_TABLE_MAX + 1];

extern const uint32_t exp_table_common[PLAYER_MAX_LEVEL_CONST + 1];
extern const uint32_t* exp_table;
#ifdef ENABLE_YOHARA_SYSTEM
extern const uint32_t exp_table_conqueror_common[PLAYER_CONQUEROR_LEVEL + 1];
extern const uint32_t* conqueror_exp_table;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
extern const uint32_t exp_pet_table_common[GROWTHPET_MAX_LEVEL_CONST + 1];
extern const uint32_t* exp_pet_table;
#endif
#ifdef ENABLE_AURA_SYSTEM
int* GetAuraRefineInfo(uint8_t bLevel);
int GetAuraRefineInfo(uint8_t bGrade, uint8_t bInfo);
#endif

extern const uint32_t guild_exp_table[GUILD_MAX_LEVEL + 1];
extern const uint32_t guild_exp_table2[GUILD_MAX_LEVEL + 1];

#define MAX_EXP_DELTA_OF_LEV 31
#define PERCENT_LVDELTA(me, victim) aiPercentByDeltaLev[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
#define PERCENT_LVDELTA_BOSS(me, victim) aiPercentByDeltaLevForBoss[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
#define CALCULATE_VALUE_LVDELTA(me, victim, val) ((val * PERCENT_LVDELTA(me, victim)) / 100)
extern const int aiPercentByDeltaLev_euckr[MAX_EXP_DELTA_OF_LEV];
extern const int aiPercentByDeltaLevForBoss_euckr[MAX_EXP_DELTA_OF_LEV];
extern const int* aiPercentByDeltaLev;
extern const int* aiPercentByDeltaLevForBoss;

#define ARROUND_COORD_MAX_NUM 161
extern Coord aArroundCoords[ARROUND_COORD_MAX_NUM];
extern TJobInitialPoints JobInitialPoints[JOB_MAX_NUM];

extern const int MAX_DEFENSE_PCT;
extern const int aiMobEnchantApplyIdx[MOB_ENCHANTS_MAX_NUM];
extern const int aiMobResistsApplyIdx[MOB_RESISTS_MAX_NUM];
#ifdef ENABLE_PROTO_RENEWAL
extern const int aiMobElementalApplyIdx[MOB_ELEMENTAL_MAX_NUM];
#endif

extern const int aSkillAttackAffectProbByRank[MOB_RANK_MAX_NUM];

extern const int aiItemMagicAttributePercentHigh[ITEM_ATTRIBUTE_MAX_LEVEL];
extern const int aiItemMagicAttributePercentLow[ITEM_ATTRIBUTE_MAX_LEVEL];

extern int aiItemAttributeAddPercent[ITEM_ATTRIBUTE_MAX_NUM]; //@custom031

extern const int aiWeaponSocketQty[WEAPON_NUM_TYPES];
extern const int aiArmorSocketQty[ARMOR_NUM_TYPES];
extern const int aiSocketPercentByQty[5][4];

extern const int aiExpLossPercents[PLAYER_EXP_TABLE_MAX + 1];

extern const int* aiSkillPowerByLevel;
extern const int aiSkillPowerByLevel_euckr[SKILL_MAX_LEVEL + 1];

extern const int aiPolymorphPowerByLevel[SKILL_MAX_LEVEL + 1];
#ifdef ENABLE_REFINE_ABILITY_SKILL
extern const int aiRefinePowerByLevel[REFINE_SKILL_MAX_LEVEL + 1];
extern const int aiGuildRefinePowerByLevel[REFINE_SKILL_MAX_LEVEL + 1];
#endif
#if defined(ENABLE_PASSIVE_ATTR) && defined(ENABLE_CHARISMA)
extern const int aiPartyPassiveSkillCharisma[SKILL_MAX_LEVEL + 1];
#endif
#if defined(ENABLE_PASSIVE_ATTR) && defined(ENABLE_PRECISION)
extern const int aiSkillPrecision[SKILL_MAX_LEVEL + 1];
#endif
#ifdef ENABLE_YOHARA_SYSTEM
extern const std::vector<TRandomJewelry> aiItemApplyRandomTalisman;
extern const std::vector<TRandomMysterious> aiItemApplyRandomMysterious;
extern const std::vector<TRandomJewelry> aiItemApplyRandomJewelry;
extern const std::vector<TRandomGloveInfo> aiItemApplyRandomGlove;
extern const std::vector<TRandomSnakeGloveInfo> aiItemApplyRandomSnakeGlove;
extern const std::vector <TRandomSnakeArmorInfo> aiItemApplyRandomSnakeArmor;
extern const std::vector <TRandomSnakeWeaponInfo> aiItemApplyRandomSnakeWeapon;

typedef std::map<uint32_t, TSungmaTable> TSungmaMap;
extern TSungmaMap g_map_SungmaTable;
#endif
extern const int aiSkillBookCountForLevelUp[10];
extern const int aiGrandMasterSkillBookCountForLevelUp[10];
extern const int aiGrandMasterSkillBookMinCount[10];
extern const int aiGrandMasterSkillBookMaxCount[10];
extern const int CHN_aiPartyBonusExpPercentByMemberCount[9];
extern const int KOR_aiPartyBonusExpPercentByMemberCount[9];
extern const int KOR_aiUniqueItemPartyBonusExpPercentByMemberCount[9];

typedef std::map<uint32_t, TItemAttrTable> TItemAttrMap;
extern TItemAttrMap g_map_itemAttr;
extern TItemAttrMap g_map_itemRare;

extern const int* aiChainLightningCountBySkillLevel;
extern const int aiChainLightningCountBySkillLevel_euckr[SKILL_MAX_LEVEL + 1];

extern const char* c_apszEmpireNames[EMPIRE_MAX_NUM];
extern const char* c_apszEmpireNamesAlt[EMPIRE_MAX_NUM];
extern const char* c_apszPrivNames[MAX_PRIV_NUM];
extern const SStoneDropInfo aStoneDrop[STONE_INFO_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
extern const SStoneDropInfo aSungmaStoneDrop[SUNGMA_STONE_INFO_MAX_NUM];
#endif

typedef struct
{
	long lMapIndex;
	int iWarPrice;
	int iWinnerPotionRewardPctToWinner;
	int iLoserPotionRewardPctToWinner;
	int iInitialScore;
	int iEndScore;
} TGuildWarInfo;

extern TGuildWarInfo KOR_aGuildWarInfo[GUILD_WAR_TYPE_MAX_NUM];

// ACCESSORY_REFINE
enum
{
	ITEM_ACCESSORY_SOCKET_MAX_NUM = 3
};

extern const int aiAccessorySocketAddPct[ITEM_ACCESSORY_SOCKET_MAX_NUM];
extern const int aiAccessorySocketEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
extern const int aiAccessorySocketDegradeTime[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
extern const int aiAccessorySocketPutPct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1];
long FN_get_apply_type(const char* apply_type_string);

// END_OF_ACCESSORY_REFINE

long FN_get_apply_type(const char* apply_type_string);
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
extern bool TryGetAttributeFromString(const std::string& in, uint8_t& out);
#endif

#ifdef ENABLE_HUNTING_SYSTEM
extern const uint32_t THuntingMissions[HUNTING_MISSION_COUNT + 1][2][2];
extern const uint32_t THuntingRewardItem[HUNTING_MISSION_COUNT + 1][2][4][2];

struct SHuntingRewardMoney
{
	uint32_t	dLevelFrom;
	uint32_t	dLevelTo;
	uint32_t	dRandMoney[5];
};

struct SHuntingRewardEXP
{
	uint32_t	dLevelFrom;
	uint32_t	dLevelTo;
	uint32_t	dPerc[2];
};

extern const SHuntingRewardMoney THuntingRewardMoney[HUNTING_MONEY_TABLE_SIZE];
extern const SHuntingRewardEXP THuntingRewardEXP[HUNTING_EXP_TABLE_SIZE];

extern const uint32_t THuntingRandomItem_01_20[6][2];
extern const uint32_t THuntingRandomItem_21_40[13][2];
extern const uint32_t THuntingRandomItem_41_60[13][2];
extern const uint32_t THuntingRandomItem_61_80[13][2];
extern const uint32_t THuntingRandomItem_81_90[13][2];
#endif

#endif

