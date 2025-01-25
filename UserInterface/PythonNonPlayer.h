#pragma once

#include <unordered_map>
#ifdef ENABLE_INGAME_WIKI
#	include "../GameLib/WikiTable.h"
#endif

/*
 *	NPC 데이터 프로토 타잎을 관리 한다.
 */
class CPythonNonPlayer : public CSingleton<CPythonNonPlayer>
{
public:
	enum EClickEvent
	{
		ON_CLICK_EVENT_NONE = 0,
		ON_CLICK_EVENT_BATTLE = 1,
		ON_CLICK_EVENT_SHOP = 2,
		ON_CLICK_EVENT_TALK = 3,
		ON_CLICK_EVENT_VEHICLE = 4,

		ON_CLICK_EVENT_MAX_NUM
	};

#ifdef WJ_SHOW_MOB_INFO
	enum EAIFlags
	{
		AIFLAG_AGGRESSIVE		= (1 << 0),
		AIFLAG_NOMOVE			= (1 << 1),
		AIFLAG_COWARD			= (1 << 2),
		AIFLAG_NOATTACKSHINSU	= (1 << 3),
		AIFLAG_NOATTACKJINNO	= (1 << 4),
		AIFLAG_NOATTACKCHUNJO	= (1 << 5),
		AIFLAG_ATTACKMOB		= (1 << 6),
		AIFLAG_BERSERK			= (1 << 7),
		AIFLAG_STONESKIN		= (1 << 8),
		AIFLAG_GODSPEED			= (1 << 9),
		AIFLAG_DEATHBLOW		= (1 << 10),
		AIFLAG_REVIVE			= (1 << 11),
		AIFLAG_HEALER			= (1 << 12),	//ENABLE_DAWNMIST_DUNGEON
#	ifdef ENABLE_PROTO_RENEWAL
		// Unknown Types
		AIFLAG_COUNT			= (1 << 13),
		AIFLAG_NORECOVERY		= (1 << 14),

		// Zodiac
		AIFLAG_REFLECT			= (1 << 15),
		AIFLAG_FALL				= (1 << 16),
		AIFLAG_VIT				= (1 << 17),
		AIFLAG_RATTSPEED		= (1 << 18),
		AIFLAG_RCASTSPEED		= (1 << 19),
		AIFLAG_RHP_REGEN		= (1 << 20),
		AIFLAG_TIMEVIT			= (1 << 21),
		AIFLAG_UNK_22			= (1 << 22),
		AIFLAG_UNK_23			= (1 << 23),
		AIFLAG_UNK_24			= (1 << 24),
		AIFLAG_UNK_25			= (1 << 25),
		AIFLAG_ELEMENT_BUFF_NONE	= (1 << 26),
		AIFLAG_ELEMENT_BUFF_FIRE	= (1 << 27),
		AIFLAG_ELEMENT_BUFF_ICE		= (1 << 28),
		AIFLAG_ELEMENT_BUFF_ELECT	= (1 << 29),
		AIFLAG_ELEMENT_BUFF_WIND	= (1 << 30),
		AIFLAG_ELEMENT_BUFF_EARTH	= (1 << 31),
#	endif
	};

	enum EAIFlagsEx
	{
		AIFLAG_ELEMENT_BUFF_DARK = (1 << 0),
	};
#endif

	enum EMobRanks
	{
		MOB_RANK_PAWN,
		MOB_RANK_S_PAWN,
		MOB_RANK_KNIGHT,
		MOB_RANK_S_KNIGHT,
		MOB_RANK_BOSS,
		MOB_RANK_KING,
		MOB_RANK_MAX_NUM
	};

	enum EMobImmuneFlags
	{
		IMMUNE_STUN		= (1 << 0),
		IMMUNE_SLOW		= (1 << 1),
		IMMUNE_FALL		= (1 << 2),
		IMMUNE_CURSE	= (1 << 3),
		IMMUNE_POISON	= (1 << 4),
		IMMUNE_TERROR	= (1 << 5),
		IMMUNE_REFLECT	= (1 << 6),
#ifdef ENABLE_PROTO_RENEWAL
		UNK_IMMUNE_FLAG = (1 << 7),
		UNK_IMMUNE_FLAG1 = (1 << 8),
		UNK_IMMUNE_FLAG2 = (1 << 9),
		UNK_IMMUNE_FLAG3 = (1 << 10),
		UNK_IMMUNE_FLAG4 = (1 << 11),
		UNK_IMMUNE_FLAG5 = (1 << 12),
		UNK_IMMUNE_FLAG6 = (1 << 13),
		UNK_IMMUNE_FLAG7 = (1 << 14),
		UNK_IMMUNE_FLAG8 = (1 << 15),
#endif
	};

	enum ERaceFlags
	{
		RACE_FLAG_ANIMAL	= (1 << 0),
		RACE_FLAG_UNDEAD	= (1 << 1),
		RACE_FLAG_DEVIL		= (1 << 2),
		RACE_FLAG_HUMAN		= (1 << 3),
		RACE_FLAG_ORC		= (1 << 4),
		RACE_FLAG_MILGYO	= (1 << 5),
		RACE_FLAG_INSECT	= (1 << 6),
		RACE_FLAG_DESERT	= (1 << 7),
		RACE_FLAG_TREE		= (1 << 8),
#ifdef ENABLE_PROTO_RENEWAL
		RACE_FLAG_DECO		= (1 << 9),
		RACE_FLAG_HIDE		= (1 << 10),
		RACE_FLAG_ATT_CZ	= (1 << 11),	//ENABLE_ELEMENT_ADD
		RACE_FLAG_AWEAKEN	= (1 << 12),
		RACE_FLAG_SUNGMAHEE	= (1 << 13),
		RACE_FLAG_OUTPOST	= (1 << 14),
		RACE_FLAG_UNKNOWN_1 = (1 << 15),
		RACE_FLAG_UNKNOWN_2 = (1 << 16),
#endif

		RACE_FLAG_MAX_NUM = 23,
	};

	enum EMobEnchants
	{
		MOB_ENCHANT_CURSE,
		MOB_ENCHANT_SLOW,
		MOB_ENCHANT_POISON,
		MOB_ENCHANT_STUN,
		MOB_ENCHANT_CRITICAL,
		MOB_ENCHANT_PENETRATE,
		MOB_ENCHANTS_MAX_NUM
	};

	enum EMobResists
	{
#ifdef ENABLE_PROTO_RENEWAL
		MOB_RESIST_FIST,
#endif
		MOB_RESIST_SWORD,
		MOB_RESIST_TWOHAND,
		MOB_RESIST_DAGGER,
		MOB_RESIST_BELL,
		MOB_RESIST_FAN,
		MOB_RESIST_BOW,
#ifdef ENABLE_PROTO_RENEWAL
		MOB_RESIST_CLAW,	//ENABLE_WOLFMAN_CHARACTER
#endif
		MOB_RESIST_FIRE,
		MOB_RESIST_ELECT,
		MOB_RESIST_MAGIC,
		MOB_RESIST_WIND,
		MOB_RESIST_POISON,
#ifdef ENABLE_PROTO_RENEWAL
		MOB_RESIST_BLEEDING,	//ENABLE_WOLFMAN_CHARACTER
#endif
		MOB_RESISTS_MAX_NUM
	};

#ifdef ENABLE_INGAME_WIKI
	enum EMobTypes
	{
		MONSTER,
		NPC,
		STONE,
		WARP,
		DOOR,
		BUILDING,
		PC,
		POLYMORPH_PC,
		HORSE,
		GOTO,
		PET,
		PET_PAY,
		SHOP
	};
#endif

#ifdef ENABLE_PROTO_RENEWAL
	enum EMobElements
	{
		MOB_ELEMENTAL_ELEC,
		MOB_ELEMENTAL_FIRE,
		MOB_ELEMENTAL_ICE,
		MOB_ELEMENTAL_WIND,
		MOB_ELEMENTAL_EARTH,
		MOB_ELEMENTAL_DARK,
		MOB_ELEMENTAL_MAX_NUM
	};
#endif

	enum EMobMaxNum
	{
		MOB_ATTRIBUTE_MAX_NUM = 12,
		MOB_SKILL_MAX_NUM = 5
	};

#pragma pack(push)
#pragma pack(1)
	typedef struct SMobSkillLevel
	{
		uint32_t dwVnum;
		uint8_t bLevel;
	} TMobSkillLevel;

	typedef struct SMobTable
	{
#ifdef ENABLE_INGAME_WIKI
		SMobTable() = default;
		~SMobTable() = default;
#endif

		uint32_t		dwVnum;
		char			szName[CHARACTER_NAME_MAX_LEN + 1];
		char			szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

		uint8_t			bType;                  // Monster, NPC
		uint8_t			bRank;                  // PAWN, KNIGHT, KING
		uint8_t			bBattleType;            // MELEE, etc..
		uint8_t			bLevel;                 // Level
#ifdef ENABLE_PROTO_RENEWAL
		uint8_t			bScale;
#endif
		uint8_t			bSize;

		uint32_t		dwGoldMin;
		uint32_t		dwGoldMax;
		uint32_t		dwExp;
#ifdef ENABLE_PROTO_RENEWAL
		uint32_t		dwSungMaExp;
#endif
		uint32_t		dwMaxHP;
		uint8_t			bRegenCycle;
		uint8_t			bRegenPercent;
		uint16_t        wDef;

		uint32_t		dwAIFlag;
#ifdef ENABLE_PROTO_RENEWAL
		uint32_t		dwAIFlagEx;
#endif
		uint32_t		dwRaceFlag;
		uint32_t		dwImmuneFlag;

		uint8_t			bStr, bDex, bCon, bInt;
#ifdef ENABLE_PROTO_RENEWAL
		uint8_t			bSungMaSt, bSungMaDx, bSungMaHt, bSungMaIq;
#endif
		uint32_t		dwDamageRange[2];

		int16_t			sAttackSpeed;
		int16_t			sMovingSpeed;
		uint8_t			bAggresiveHPPct;
		uint16_t        wAggressiveSight;
		uint16_t        wAttackRange;

		char			cEnchants[MOB_ENCHANTS_MAX_NUM];
		char			cResists[MOB_RESISTS_MAX_NUM];
#ifdef ENABLE_PROTO_RENEWAL
		char			cElementalFlags[MOB_ELEMENTAL_MAX_NUM];
		char			cResistDark, cResistIce, cResistEarth;
#endif

		uint32_t		dwResurrectionVnum;
		uint32_t		dwDropItemVnum;

		uint8_t			bMountCapacity;
		uint8_t			bOnClickType;

		uint8_t			bEmpire;
		char			szFolder[64 + 1];
		float			fDamMultiply;
		uint32_t		dwSummonVnum;
		uint32_t		dwDrainSP;
		uint32_t		dwMonsterColor;
	    uint32_t		dwPolymorphItemVnum;

		TMobSkillLevel	Skills[MOB_SKILL_MAX_NUM];

	    uint8_t			bBerserkPoint;
		uint8_t			bStoneSkinPoint;
		uint8_t			bGodSpeedPoint;
		uint8_t			bDeathBlowPoint;
		uint8_t			bRevivePoint;

#ifdef ENABLE_PROTO_RENEWAL
		uint8_t			bHealPoint;
		uint8_t			bRAttSpeedPoint;
		uint8_t			bRCastSpeedPoint;
		uint8_t			bRHPRegenPoint;
		float			fHitRange;
#endif
	} TMobTable;
#pragma pack(pop)

#ifdef ENABLE_INGAME_WIKI
	typedef struct SWikiInfoTable
	{
		~SWikiInfoTable() = default;
		SWikiInfoTable() {
			isSet = false;
			isFiltered = false;

			dropList.clear();
			mobTable = nullptr;
		}

		bool isSet;
		bool isFiltered;

		std::vector<NWikiData::TWikiMobDropInfo> dropList;
		std::unique_ptr<TMobTable> mobTable;
	} TWikiInfoTable;

	typedef std::list<TMobTable*> TMobTableList;
	typedef std::map<uint32_t, TWikiInfoTable> TNonPlayerDataMap;
#else
	using TNonPlayerDataMap = std::map<uint32_t, TMobTable>;
#endif

public:
	CPythonNonPlayer();
	~CPythonNonPlayer();
	CLASS_DELETE_COPYMOVE(CPythonNonPlayer);

	void Clear() const;
	void Destroy();

	bool LoadNonPlayerData(const char * c_szFileName);

	const TMobTable * GetTable(uint32_t dwVnum);
#ifdef ENABLE_INGAME_WIKI
	TWikiInfoTable*		GetWikiTable(uint32_t dwVnum);
	bool				CanRenderMonsterModel(uint32_t dwMonsterVnum);
	size_t				WikiLoadClassMobs(uint8_t bType, uint16_t fromLvl, uint16_t toLvl);
	void				WikiSetBlacklisted(uint32_t vnum);
	void				BuildWikiSearchList();
	std::tuple<const char*, int>	GetMonsterDataByNamePart(const char* namePart);
	std::vector<uint32_t>* WikiGetLastMobs() { return &m_vecTempMob; }
#endif
	bool GetName(uint32_t dwVnum, const char ** c_pszName);
	bool GetInstanceType(uint32_t dwVnum, uint8_t * pbType);
	uint8_t GetEventType(uint32_t dwVnum);
	uint8_t GetEventTypeByVID(uint32_t dwVID);
	uint32_t GetMonsterColor(uint32_t dwVnum);
	const char * GetMonsterName(uint32_t dwVnum);

#ifdef ENABLE_SEND_TARGET_INFO
	// TARGET_INFO
	uint32_t GetRaceNumByVID(uint32_t iVID);
	uint32_t GetMonsterMaxHP(uint32_t dwVnum);
	uint32_t GetMonsterRaceFlag(uint32_t dwVnum);
	uint32_t GetMonsterDamage1(uint32_t dwVnum);
	uint32_t GetMonsterDamage2(uint32_t dwVnum);
	uint32_t GetMonsterExp(uint32_t dwVnum);
	float GetMonsterDamageMultiply(uint32_t dwVnum);
	uint32_t GetMonsterST(uint32_t dwVnum);
	uint32_t GetMonsterDX(uint32_t dwVnum);
	bool IsMonsterStone(uint32_t dwVnum);
	uint8_t GetMobRegenCycle(uint32_t dwVnum);
	uint8_t GetMobRegenPercent(uint32_t dwVnum);
	uint32_t GetMobGoldMin(uint32_t dwVnum);
	uint32_t GetMobGoldMax(uint32_t dwVnum);
	uint32_t GetMobResist(uint32_t dwVnum, uint8_t bResistNum);
#endif

	uint32_t GetMobAIFlag(uint32_t dwVnum);
	uint32_t GetMobLevel(uint32_t dwVnum);
	uint32_t GetMobRank(uint32_t dwVnum);
	uint32_t GetMobType(uint32_t dwVnum);

	bool IsImmuneFlagByVnum(uint32_t dwVnum, uint32_t dwImmuneFlag);

#ifdef ENABLE_SEND_TARGET_INFO
	uint32_t GetMonsterLevel(uint32_t dwVnum);
#endif

#ifdef WJ_SHOW_MOB_INFO
	bool IsAggressive(uint32_t dwVnum);
#endif
	uint8_t GetMonsterRank(uint32_t dwVnum);

#ifdef ENABLE_SCALE_SYSTEM
	uint8_t GetMonsterScalePercent(uint32_t dwVnum);
#endif
#ifdef ENABLE_NEW_DISTANCE_CALC
	float GetMonsterHitRange(uint32_t dwVnum);
#endif

#ifdef ENABLE_ELEMENT_ADD
	uint8_t GetMonsterEnchantElement(uint32_t dwVnum, uint8_t bElement);
#endif

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	void AddPortalLevelLimit(uint32_t dwRace, int iMinLevel, int iMaxLevel);
	const std::pair<int, int>* GetPortalLevelLimit(uint32_t dwRace) const;
#endif

protected:
	TNonPlayerDataMap m_NonPlayerDataMap;
#ifdef ENABLE_INGAME_WIKI
	void SortMobDataName();
	std::vector<uint32_t> m_vecTempMob;
	std::vector<TMobTable*> m_vecWikiNameSort;
#endif
#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	std::unordered_map<uint32_t, std::pair<int, int>> m_NonPlayerPortalLevelLimit;
#endif
};

inline bool operator<(const CPythonNonPlayer::TMobTable & lhs, const CPythonNonPlayer::TMobTable & rhs)
{
	return lhs.dwVnum < rhs.dwVnum;
}
