#ifndef	__INC_GROWTH_PET_SYSTEM_H__
#define	__INC_GROWTH_PET_SYSTEM_H__

#ifdef ENABLE_GROWTH_PET_SYSTEM

#include "item.h"
#include "utils.h"

enum EGrowthPetFlashEvent
{
	FLASH_EVENT_FEED,
	FLASH_EVENT_EVOLVE,
	FLASH_EVENT_EXP_ITEM,

	FLASH_EVENT_NONE,
};

enum EGrowthPetTableDefinies
{
	PET_MAX_LEVEL = 105,
	PET_MAX_EVOLVE = 4,
	PET_SKILL_TABLE_VALUE = 4, //DEPRECATED
	PET_MAX_SKILL_POINTS = 20,
	PET_MAX_FEED_SLOT = 9,
};

enum EPetRevive
{
	PET_REVIVE_TYPE_NORMAL,
	PET_REVIVE_TYPE_PREMIUM,
};

enum EGrowthPetInfo
{
	//PET_EGG_INCUBATION_PRICE = 100000,
	SKILL_UPGRADE_PRICE = 5000000,
	EGG_USE_SUCCESS = 0,
	EGG_USE_FAILED_BECAUSE_NAME = 1,
	EGG_USE_FAILED_TIMEOVER = 2,

	//GROWTH_PET_ITEM_USE_COOL_TIME = 1,
	//PET_EGG_USE_TRUE = 0,
	PET_EGG_USE_FAILED_BECAUSE_TRADING = 1,
	PET_EGG_USE_FAILED_BECAUSE_SHOP_OPEN = 2,
	PET_EGG_USE_FAILED_BECAUSE_MALL_OPEN = 3,
	PET_EGG_USE_FAILED_BECAUSE_SAFEBOX_OPEN = 4,
	PET_HATCHING_MONEY = 1000000,
	PET_NAME_MIN_SIZE = 4,
};

enum EGrowthPetWindow
{
	PET_FEED_WINDOW,
	PET_EVOLVE_WINDOW,
	PET_EXP_ITEM_WINDOW,
};

enum EGrowthPetExpType
{
	PET_EXP_FROM_MOB,
	PET_EXP_FROM_ITEM,
};

enum EGrowthPetSkillIndex
{
	PET_SKILL_AFFECT_NONE,

	PET_SKILL_AFFECT_JIJOONG_WARRIOR = 1,
	PET_SKILL_AFFECT_JIJOONG_SURA = 2,
	PET_SKILL_AFFECT_JIJOONG_ASSASSIN = 3,
	PET_SKILL_AFFECT_JIJOONG_SHAMAN = 4,
#ifdef ENABLE_WOLFMAN_CHARACTER
	PET_SKILL_AFFECT_JIJOONG_WOLFMAN = 5,
#endif
	PET_SKILL_AFFECT_PACHEON = 6,
	PET_SKILL_AFFECT_CHEONRYEONG = 7,
	PET_SKILL_AFFECT_BANYA = 8,
	PET_SKILL_AFFECT_CHOEHOENBIMU = 9,
	PET_SKILL_AFFECT_HEAL = 10,
	PET_SKILL_AFFECT_STEALHP = 11,
	PET_SKILL_AFFECT_STEALMP = 12,
	PET_SKILL_AFFECT_BLOCK = 13,
	PET_SKILL_AFFECT_REFLECT_MELEE = 14,
	PET_SKILL_AFFECT_GOLD_DROP = 15,
	PET_SKILL_AFFECT_BOW_DISTANCE = 16,
	PET_SKILL_AFFECT_INVINCIBILITY = 17,
	PET_SKILL_AFFECT_REMOVAL = 18,
	PET_SKILL_AFFECT_POTION = 19,
	PET_SKILL_AFFECT_MOB_BONUS = 20,
	PET_SKILL_AFFECT_EXP = 21,
	PET_SKILL_HP_RECOVER = 22,
	PET_SKILL_FEATHER = 23,

	PET_SKILL_MAX,
};

enum EGrowthPetSeals
{
	PET_MINI_MONKEY = 55701,
	PET_MINI_SPIDER = 55702,
	PET_MINI_RAZADOR = 55703,
	PET_MINI_NEMERE = 55704,
	PET_MINI_DRAGON = 55705,
	PET_MINI_MELEY = 55706,
	PET_MINI_AZRAEL = 55707,
	PET_MINI_EXECUTOR = 55708,
	PET_MINI_BAASHIDO = 55709,
	PET_MINI_NESSIE = 55710,
	PET_MINI_EXEDYAR = 55711,
	PET_MINI_ALASTOR = 55712,
};

enum EGrowthPetNameChangeResult
{
	NAME_CHANGE_USE_SUCCESS = 0,
	NAME_CHANGE_USE_FAILED_BECAUSE_NAME = 1,
};

const float PET_HP_RANGE[9][3] =
{
	//min, max, level
	{ 0.0, 0.0, 0 }, // type none
	{ 0.1, 0.5, 6 }, // type 1
	{ 0.2, 0.3, 5 }, // type 2
	{ 0.1, 0.7, 6 }, // type 3
	{ 0.3, 0.4, 5 }, // type 4
	{ 0.1, 0.9, 6 }, // type 5
	{ 0.5, 0.6, 5 }, // type 6
	{ 0.1, 1.0, 6 }, // type 7
	{ 0.5, 0.7, 5 }	 // type 8
};

const float PET_DEF_RANGE[9][3] =
{
	//min, max, level
	{ 0.0, 0.0, 0 }, // type none
	{ 0.1, 0.2, 8 }, // type 1
	{ 0.2, 0.3, 7 }, // type 2
	{ 0.1, 0.3, 8 }, // type 3
	{ 0.3, 0.4, 7 }, // type 4
	{ 0.1, 0.4, 8 }, // type 5
	{ 0.4, 0.5, 7 }, // type 6
	{ 0.1, 0.6, 8 }, // type 7
	{ 0.5, 0.6, 7 }	 // type 8
};

const float PET_SP_RANGE[9][3] =
{
	//min, max, level
	{ 0.0, 0.0, 0 }, // type none
	{ 0.1, 0.5, 6 }, // type 1
	{ 0.2, 0.3, 4 }, // type 2
	{ 0.1, 0.7, 6 }, // type 3
	{ 0.3, 0.4, 4 }, // type 4
	{ 0.1, 0.9, 6 }, // type 5
	{ 0.4, 0.5, 4 }, // type 6
	{ 0.1, 1.1, 6 }, // type 7
	{ 0.5, 0.6, 4 }	 // type 8
};

const float PET_HATCH_INFO_RANGE[12][6] =
{
	// pet_vnum, day_min, day_max, skill_min, skill_max, type_max
	{ PET_MINI_MONKEY,	 1,	 14, 1, 2, 6 },	// 55701
	{ PET_MINI_SPIDER,	 1,	 14, 1, 2, 6 },	// 55702
	{ PET_MINI_RAZADOR,	 1,	 14, 1, 3, 8 },	// 55703
	{ PET_MINI_NEMERE,	 1,	 14, 1, 3, 8 },	// 55704
	{ PET_MINI_DRAGON,	 1,	 14, 1, 3, 8 },	// 55705
	{ PET_MINI_MELEY,	 15, 45, 3, 3, 8 },	// 55706
	{ PET_MINI_AZRAEL,	 1,	 14, 1, 2, 6 },	// 55707
	{ PET_MINI_EXECUTOR, 1,	 14, 1, 3, 6 },	// 55708
	{ PET_MINI_BAASHIDO, 7,	 28, 1, 3, 8 },	// 55709
	{ PET_MINI_NESSIE,	 15, 45, 3, 3, 8 },	// 55710
	{ PET_MINI_EXEDYAR,	 15, 45, 3, 3, 8 },	// 55711
	{ PET_MINI_EXEDYAR,	 15, 45, 3, 3, 8 }	// 55712
};

const uint32_t PET_EVOLVE_CUBE[3][7][2] =
{
	// Evolution 1
	{	// item, count
		{ 55003, 10 }, // Young Pet Book
		{ 30058, 10 }, // Spider Egg Sack
		{ 30073, 10 }, // White Hairband+
		{ 30041, 10 }, // Shiriken
		{ 30017, 10 }, // Ornamental Hairpin
		{ 30074, 5 },  // Black Uniform+
		{ 30088, 5 }   // Piece of Ice+
	},

	// Evolution 2
	{
		// item, count
		{ 55004, 10 }, // Wild Pet Book
		{ 27994, 2 },  // Blood-Red Pearl
		{ 30035, 10 }, // Face Cream
		{ 30089, 10 }, // Yeti Fur+
		{ 30031, 10 }, // Ornament
		{ 30011, 10 }, // Ball
		{ 30080, 5 }   // Curse Book+
	},

	// Evolution 3
	{
		// item, count
		{ 55005, 10 }, // Valiant Pet Book
		{ 30083, 10 }, // Unknown Medicine+
		{ 27992, 2 },  // White Pearl
		{ 27993, 2 },  // Blue Pearl
		{ 30086, 10 }, // Demon's Keepsake+
		{ 30077, 10 }, // Orc Tooth+
		{ 30550, 5 }   // Blue Belt
	},
};

// Skills
enum ePetSkillType
{
	PET_SKILL_USE_TYPE_NONE,
	PET_SKILL_USE_TYPE_PASSIVE,
	PET_SKILL_USE_TYPE_AUTO,
	PET_SKILL_USE_TYPE_MAX,
};

typedef struct SPetSkillTable
{
	uint32_t skill_vnum;
	uint8_t skill_type;
	uint16_t skill_apply;
	float skill_formula1;
	float next_skill_formula1;
	float skill_formula2;
	float next_skill_formula2;
} TPetSkillTable;

const TPetSkillTable pet_skill_table[PET_SKILL_MAX][6] =
{
	// skill_vnum	skill_type	skill_affect	skill_value1	skill_value2	skill_cooldown
	{ PET_SKILL_AFFECT_NONE, PET_SKILL_USE_TYPE_NONE, APPLY_NONE, 0, 0, 0 },

	{ PET_SKILL_AFFECT_JIJOONG_WARRIOR, PET_SKILL_USE_TYPE_PASSIVE, APPLY_RESIST_WARRIOR, 12, 0, 0 },
	{ PET_SKILL_AFFECT_JIJOONG_SURA, PET_SKILL_USE_TYPE_PASSIVE, APPLY_RESIST_SURA, 12, 0, 0 },
	{ PET_SKILL_AFFECT_JIJOONG_ASSASSIN, PET_SKILL_USE_TYPE_PASSIVE, APPLY_RESIST_ASSASSIN, 12, 0, 0 },
	{ PET_SKILL_AFFECT_JIJOONG_SHAMAN, PET_SKILL_USE_TYPE_PASSIVE, APPLY_RESIST_SHAMAN, 12, 0, 0 },
#ifdef ENABLE_WOLFMAN_CHARACTER
	{ PET_SKILL_AFFECT_JIJOONG_WOLFMAN, PET_SKILL_USE_TYPE_PASSIVE, APPLY_RESIST_WOLFMAN, 12, 0, 0 },
#endif
	{ PET_SKILL_AFFECT_PACHEON, PET_SKILL_USE_TYPE_PASSIVE, APPLY_MELEE_MAGIC_ATTBONUS_PER, 8, 0, 0 },
	{ PET_SKILL_AFFECT_CHEONRYEONG, PET_SKILL_USE_TYPE_PASSIVE, APPLY_RESIST_MAGIC_REDUCTION, 10, 0, 0 },
	{ PET_SKILL_AFFECT_BANYA, PET_SKILL_USE_TYPE_PASSIVE, APPLY_CAST_SPEED, 18, 0, 0 },
	{ PET_SKILL_AFFECT_CHOEHOENBIMU, PET_SKILL_USE_TYPE_PASSIVE, APPLY_PENETRATE_PCT, 10, 0, 0 },
	{ PET_SKILL_AFFECT_HEAL, PET_SKILL_USE_TYPE_AUTO, APPLY_NONE, 57, 0, 480 },
	{ PET_SKILL_AFFECT_STEALHP, PET_SKILL_USE_TYPE_PASSIVE, APPLY_STEAL_HP, 6, 0, 0 },
	{ PET_SKILL_AFFECT_STEALMP, PET_SKILL_USE_TYPE_PASSIVE, APPLY_STEAL_SP, 7, 0, 0 },
	{ PET_SKILL_AFFECT_BLOCK, PET_SKILL_USE_TYPE_PASSIVE, APPLY_BLOCK, 12, 0, 0 },
	{ PET_SKILL_AFFECT_REFLECT_MELEE, PET_SKILL_USE_TYPE_PASSIVE, APPLY_REFLECT_MELEE, 6, 0, 0 },
	{ PET_SKILL_AFFECT_GOLD_DROP, PET_SKILL_USE_TYPE_PASSIVE, APPLY_MALL_GOLDBONUS, 106, 0, 0 },
	{ PET_SKILL_AFFECT_BOW_DISTANCE, PET_SKILL_USE_TYPE_PASSIVE, APPLY_BOW_DISTANCE, 6, 0, 0 },
	{ PET_SKILL_AFFECT_INVINCIBILITY, PET_SKILL_USE_TYPE_AUTO, APPLY_NONE, 52, 2.7, 600 },
	{ PET_SKILL_AFFECT_REMOVAL, PET_SKILL_USE_TYPE_AUTO, APPLY_NONE, 41, 0, 480 },

	{ PET_SKILL_AFFECT_POTION, PET_SKILL_USE_TYPE_PASSIVE, APPLY_HP_REGEN, 10, 0, 0 },
	{ PET_SKILL_AFFECT_MOB_BONUS, PET_SKILL_USE_TYPE_PASSIVE, APPLY_ATTBONUS_MONSTER, 8, 0, 0 },
	{ PET_SKILL_AFFECT_EXP, PET_SKILL_USE_TYPE_PASSIVE, APPLY_EXP_DOUBLE_BONUS, 6, 0, 0 },
	{ PET_SKILL_HP_RECOVER, PET_SKILL_USE_TYPE_PASSIVE, APPLY_KILL_HP_RECOVER, 6, 0, 0 },

	{ PET_SKILL_FEATHER, PET_SKILL_USE_TYPE_AUTO, APPLY_NONE, 57, 0, 180 },
};

typedef struct SPetSkillSpecialistTable
{
	uint32_t pet_vnum;
	uint8_t skill_vnum;
	float min_value;
	float max_value;
} TPetSkillSpecialistTable;

const TPetSkillSpecialistTable pet_skill_specialist_table[][4] =
{
	// monkey
	{ PET_MINI_MONKEY, PET_SKILL_AFFECT_JIJOONG_WARRIOR, 13, 14 },
	{ PET_MINI_MONKEY, PET_SKILL_AFFECT_JIJOONG_SURA, 13, 14 },
	{ PET_MINI_MONKEY, PET_SKILL_AFFECT_HEAL, 7067, 7200 },
	{ PET_MINI_MONKEY, PET_SKILL_AFFECT_GOLD_DROP, 10.8, 11.6 },
	{ PET_MINI_MONKEY, PET_SKILL_AFFECT_BOW_DISTANCE, 6, 9 },

	// spider
	{ PET_MINI_SPIDER, PET_SKILL_AFFECT_JIJOONG_WOLFMAN, 13, 14 },
	{ PET_MINI_SPIDER, PET_SKILL_AFFECT_REFLECT_MELEE, 7, 9 },
	{ PET_MINI_SPIDER, PET_SKILL_AFFECT_HEAL, 7133, 7333 },
	{ PET_MINI_SPIDER, PET_SKILL_AFFECT_BOW_DISTANCE, 6, 8 },

	// yamachun
	{ PET_MINI_RAZADOR, PET_SKILL_AFFECT_PACHEON, 11, 14 },
	{ PET_MINI_RAZADOR, PET_SKILL_AFFECT_CHOEHOENBIMU, 11, 13 },
	{ PET_MINI_RAZADOR, PET_SKILL_AFFECT_JIJOONG_ASSASSIN, 13, 14 },
	{ PET_MINI_RAZADOR, PET_SKILL_AFFECT_HEAL, 7133, 7333 },
	{ PET_MINI_RAZADOR, PET_SKILL_AFFECT_BOW_DISTANCE, 6, 9 },

	// hanma
	{ PET_MINI_NEMERE, PET_SKILL_AFFECT_JIJOONG_SHAMAN, 13, 14 },
	{ PET_MINI_NEMERE, PET_SKILL_AFFECT_CHEONRYEONG, 11, 14 },
	{ PET_MINI_NEMERE, PET_SKILL_AFFECT_HEAL, 7133, 7333 },
	{ PET_MINI_NEMERE, PET_SKILL_AFFECT_BOW_DISTANCE, 6, 9 },

	// blue dragon
	{ PET_MINI_DRAGON, PET_SKILL_AFFECT_JIJOONG_ASSASSIN, 13, 14 },
	{ PET_MINI_DRAGON, PET_SKILL_AFFECT_PACHEON, 11, 14 },
	{ PET_MINI_DRAGON, PET_SKILL_AFFECT_CHOEHOENBIMU, 11, 13 },
	{ PET_MINI_DRAGON, PET_SKILL_AFFECT_HEAL, 7133, 7333 },
	{ PET_MINI_DRAGON, PET_SKILL_AFFECT_BOW_DISTANCE, 6, 9 },

	// red dragon
	{ PET_MINI_MELEY, PET_SKILL_AFFECT_BANYA, 19, 22 },
	{ PET_MINI_MELEY, PET_SKILL_AFFECT_HEAL, 7133, 7333 },
	{ PET_MINI_MELEY, PET_SKILL_AFFECT_BLOCK, 12, 15 },
	{ PET_MINI_MELEY, PET_SKILL_AFFECT_POTION, 11, 14 },
	{ PET_MINI_MELEY, PET_SKILL_AFFECT_MOB_BONUS, 9, 12 },

	// pwahuang
	{ PET_MINI_AZRAEL, PET_SKILL_AFFECT_JIJOONG_WOLFMAN, 13, 14 },
	{ PET_MINI_AZRAEL, PET_SKILL_AFFECT_HEAL, 7133, 7200 },
	{ PET_MINI_AZRAEL, PET_SKILL_AFFECT_REFLECT_MELEE, 6, 8 },
	{ PET_MINI_AZRAEL, PET_SKILL_AFFECT_BOW_DISTANCE, 6, 8 },

	// redthief officer
	{ PET_MINI_EXECUTOR, PET_SKILL_AFFECT_PACHEON, 11, 16 },
	{ PET_MINI_EXECUTOR, PET_SKILL_AFFECT_CHEONRYEONG, 11, 16 },
	{ PET_MINI_EXECUTOR, PET_SKILL_AFFECT_CHOEHOENBIMU, 11, 13 },
	{ PET_MINI_EXECUTOR, PET_SKILL_AFFECT_HEAL, 7138, 7200 },
	{ PET_MINI_EXECUTOR, PET_SKILL_AFFECT_BOW_DISTANCE, 7, 8 },

	// luck sheep
	{ PET_MINI_BAASHIDO, PET_SKILL_AFFECT_JIJOONG_WOLFMAN, 13, 14 },
	{ PET_MINI_BAASHIDO, PET_SKILL_AFFECT_PACHEON, 11, 13 },
	{ PET_MINI_BAASHIDO, PET_SKILL_AFFECT_HEAL, 7000, 7000 },
	{ PET_MINI_BAASHIDO, PET_SKILL_AFFECT_STEALHP, 6, 8 },
	{ PET_MINI_BAASHIDO, PET_SKILL_AFFECT_STEALMP, 7, 10 },
	{ PET_MINI_BAASHIDO, PET_SKILL_AFFECT_GOLD_DROP, 10.8, 11.6 },

	// defensewave hydra
	{ PET_MINI_NESSIE, PET_SKILL_AFFECT_BANYA, 19, 22 },
	{ PET_MINI_NESSIE, PET_SKILL_AFFECT_HEAL, 7138, 7333 },
	{ PET_MINI_NESSIE, PET_SKILL_AFFECT_BLOCK, 12, 15 },
	{ PET_MINI_NESSIE, PET_SKILL_AFFECT_POTION, 11, 14 },
	{ PET_MINI_NESSIE, PET_SKILL_AFFECT_MOB_BONUS, 9, 12 },

	// Exedyar
	{ PET_MINI_EXEDYAR, PET_SKILL_AFFECT_JIJOONG_WOLFMAN, 13, 14 },
	{ PET_MINI_EXEDYAR, PET_SKILL_AFFECT_PACHEON, 11, 13 },
	{ PET_MINI_EXEDYAR, PET_SKILL_AFFECT_STEALHP, 6, 8 },
	{ PET_MINI_EXEDYAR, PET_SKILL_AFFECT_STEALMP, 7, 10 },
	{ PET_MINI_EXEDYAR, PET_SKILL_AFFECT_GOLD_DROP, 10.8, 11.6 },
	{ PET_MINI_EXEDYAR, PET_SKILL_AFFECT_HEAL, 7000, 7000 },
};

class CHARACTER;

struct SGrowthPetAbility {};

class CGrowthPetSystemActor //: public CHARACTER
{
public:
	enum EGrowthPetOptions
	{
		EPetOption_Followable = 1 << 0,
		EPetOption_Mountable = 1 << 1,
		EPetOption_Summonable = 1 << 2,
		EPetOption_Combatable = 1 << 3,
	};

protected:
	friend class CGrowthPetSystem;
	CGrowthPetSystemActor(LPCHARACTER owner, uint32_t vnum, uint32_t options = EPetOption_Followable | EPetOption_Summonable);
	virtual ~CGrowthPetSystemActor();
	virtual bool Update(uint32_t deltaTime);
protected:
	virtual bool _UpdateFollowAI();

private:
	bool Follow(float fMinDistance = 50.f);

public:

	bool HasOption(EGrowthPetOptions option) const noexcept { return m_dwOptions & option; }

	// REVISED
	LPCHARACTER GetGrowthPet() const { return m_pkGrowthPet; }
	LPCHARACTER GetOwner() const { return m_pkOwner; }

	uint32_t GetVID() const { return m_dwRaceVID; }
	uint32_t GetRaceVnum() const { return m_dwRaceVnum; }

	void UpdateFlashEvent() noexcept;
	void SetPetLevel(uint8_t bLevel);

	uint32_t Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar = false);
	bool Dismiss();

	uint32_t GetPetID() { return m_pkPetSeal ? m_pkPetSeal->GetSocket(2) : 0; };
	uint32_t GetPetVnum() { return m_pkPetSeal ? m_pkPetSeal->GetVnum() : 0; };

	// LEVEL		--------------------------------------------------------------------------------------------------------------

	int GetPetLevel() { return m_PetInfo.pet_level; };

	bool EvolvePet();

	// EVOLUTION	--------------------------------------------------------------------------------------------------------------

	uint32_t GetEvolution() { return m_PetInfo.evol_level; };

	// PET TYPE		--------------------------------------------------------------------------------------------------------------

	uint16_t GetPetType() { return m_pkPetSeal ? m_pkPetSeal->GetAttributeValue(2) : 0; };

	// PET BIRTHDAY
	long GetPetBirthday() { return m_pkPetSeal ? ((get_global_time() - m_pkPetSeal->GetSocket(1)) / (60 * 60 * 24)) : 0; };

	// EXP			--------------------------------------------------------------------------------------------------------------
	uint32_t GetNextExpFromTable();

	bool SetExp(uint32_t dwExpValue, uint8_t bExpMode);

	bool SetBirthday(uint32_t bBirthday); // workout

	void SetNeedExpFromMonster(uint64_t dwNextExpFromMonster) { m_dwNeedExpFromMonster = dwNextExpFromMonster; };
	uint64_t GetNeedExpFromMonster() { return GetNextExpFromTable(); };

	uint64_t GetNeedExpFromItem() { return GetNextExpFromTable() / 9; };

	float GetPetHP() { return (m_PetInfo.pet_hp * 10); };
	float GetPetDEF() { return (m_PetInfo.pet_def * 10); };
	float GetPetSP() { return (m_PetInfo.pet_sp * 10); };

	long GetPetAgeDays() { return ((time(0) - m_PetInfo.pet_birthday) / 60 / 60 / 24); };

	// SKILL		--------------------------------------------------------------------------------------------------------------

	bool LearnPetSkill(uint8_t bSkillBookSlotIndex, uint8_t bSkillBookInvenIndex);
	bool PetSkillUpgradeRequest(uint8_t bSkillSlot, uint8_t bSkillIndex);

	bool DeleteSkill(uint8_t bSkillBookDelSlotIndex, uint8_t bSkillBookDelInvenIndex);
	bool DeleteAllSkill(uint8_t bPetSkillAllDelBookIndex);

#ifdef ENABLE_PET_ATTR_DETERMINE
	bool Determine();
#endif

	bool ChangeName(const char* sName);
	//				--------------------------------------------------------------------------------------------------------------
	//				--------------------------------------------------------------------------------------------------------------
	// TEST

	bool IsActivePassiveSkill() { return m_bActivePassiveSkill; };
	void ActivePassiveSkill(bool bOption) { m_bActivePassiveSkill = bOption; };
	bool m_bActivePassiveSkill = false;

	bool ItemCubeFeed(uint8_t bIndex, const uint16_t iFeedItemsCubeSlot[9], uint16_t wFeedItemsCount);

	bool IncreasePetSkill(uint8_t bSkillSlot);

	void GiveBuff();
	void ClearBuff();

	bool CanIncreasePetLevel();
	bool CanIncreaseEvolvePet();

	bool IsDead(LPITEM petSeal);

	TPetSkillTable GetPetSkillInformation(uint8_t skill_slot);

private:
	LPCHARACTER m_pkGrowthPet;
	LPCHARACTER m_pkOwner;

	TGrowthPetInfo m_PetInfo;

	LPITEM m_pkPetSeal;

	uint32_t m_dwRaceVID;
	uint32_t m_dwRaceVnum;

	uint64_t m_dwNeedExpFromMonster;

	uint32_t m_dwOptions;
	uint32_t m_dwLastActionTime;

	int16_t m_originalMoveSpeed;
};

class CGrowthPetSystem
{
public:
	typedef std::unordered_map<uint32_t, CGrowthPetSystemActor*> TGrowthPetActorMap;

public:
	CGrowthPetSystem(LPCHARACTER owner);
	virtual ~CGrowthPetSystem();

	CGrowthPetSystemActor* GetByVID(uint32_t vid) const;
	CGrowthPetSystemActor* GetByVnum(uint32_t vnum) const;

	bool Update(uint32_t deltaTime);
	void Destroy();

	size_t CountSummoned() const;

public:
	CGrowthPetSystemActor* Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options = CGrowthPetSystemActor::EPetOption_Followable | CGrowthPetSystemActor::EPetOption_Summonable);

	void Dismiss(uint32_t mobVnum, bool bDeleteFromList = false);

	bool IsDead(LPITEM petSeal);

	bool LearnPetSkill(uint8_t bSkillBookSlotIndex, uint8_t bSkillBookInvenIndex);

	bool PetSkillUpgradeRequest(uint8_t bSkillSlot, uint8_t bSkillIndex);
	bool IncreasePetSkill(uint8_t bSkillSlot, uint8_t bSkillBookInvenIndex);

	void DeletePet(uint32_t mobVnum);
	void DeletePet(CGrowthPetSystemActor* petActor);
	bool IsActivePet();
	bool Dismiss();
	bool ChangeName(const char* sName);
	void SetExp(uint32_t dExpValue, uint8_t bExpMode);
	void SetLevel(uint8_t bLevel); //workout
	void SetBirthday(uint32_t bBirthday); //workout
	bool ItemCubeFeed(uint8_t bIndex, const uint16_t wFeedItemsCubeSlot[9], uint16_t wFeedItemsCount);

	bool EvolvePet();

	bool DeleteSkill(uint8_t bSkillBookDelSlotIndex, uint8_t bSkillBookDelInvenIndex);
	bool DeleteAllSkill(uint8_t bPetSkillAllDelBookIndex);
#ifdef ENABLE_PET_ATTR_DETERMINE
	bool Determine();
#endif

	uint32_t GetLevel();
	uint32_t GetEvolution();
	bool CanEvolve();

private:
	TGrowthPetActorMap m_petActorMap;
	LPCHARACTER m_pkOwner;

	uint32_t m_dwUpdatePeriod;
	uint32_t m_dwLastUpdateTime;

	LPEVENT m_pkGrowthPetSystemUpdateEvent;
};
#endif
#endif	/* __INC_GROWTH_PET_SYSTEM_H__ */
