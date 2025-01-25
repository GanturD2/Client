#ifndef __INC_METIN_II_GAME_DUNGEON_H
#define __INC_METIN_II_GAME_DUNGEON_H

#include "sectree_manager.h"

class CParty;
#ifdef ENABLE_D_NJGUILD
class CGuild;
#endif

class CDungeon
{
	typedef std::unordered_map<LPPARTY, int> TPartyMap;
	typedef std::map<std::string, LPCHARACTER> TUniqueMobMap;

public:
	// <Factor> Non-persistent identifier type
	typedef uint32_t IdType;

	~CDungeon();

	// <Factor>
	IdType GetId() const { return m_id; }

	// DUNGEON_NOTICE
	void Notice(const char* msg);
#ifdef ENABLE_NEW_CHAT_DUNGEON
	void Syschat(const char* msg);
	void BigNotice(const char* msg);
#endif
	// END_OF_DUNGEON_NOTICE

#ifdef ENABLE_12ZI
	void ZodiacMessage(const char* msg);
	void ZodiacMessageClear();

	void ZodiacTime(uint8_t currentfloor, uint8_t nextfloor, int time);
	void ZodiacTimeClear();

	void SetZodiacCountMonster(int count) { m_iZodiacMonsterCount = count; }
	void ZodiacIncMonster() { m_iZodiacMonsterCount++; }
	void ZodiacDecMonster() { m_iZodiacMonsterCount--; }
	int ZodiacCountMonster() { return m_iZodiacMonsterCount; }
#endif

	void JoinParty(LPPARTY pParty);
	void QuitParty(LPPARTY pParty);

	void Join(LPCHARACTER ch);

	void IncMember(LPCHARACTER ch);
	void DecMember(LPCHARACTER ch);

	// DUNGEON_KILL_ALL_BUG_FIX
	void Purge();
	void KillAll();
#ifdef ENABLE_NEWSTUFF
	void KillMonsters();
#endif
	// END_OF_DUNGEON_KILL_ALL_BUG_FIX

	void IncMonster() noexcept { m_iMonsterCount++; sys_log(0, "MonsterCount %d", m_iMonsterCount); }
	void DecMonster() noexcept { m_iMonsterCount--; CheckEliminated(); }
	int CountMonster() noexcept { return m_iMonsterCount; } // Number of monsters regenerated with data
	int CountRealMonster(); // Monsters that are actually on the map

	void IncPartyMember(LPPARTY pParty, LPCHARACTER ch);
	void DecPartyMember(LPPARTY pParty, LPCHARACTER ch);

	void IncKillCount(LPCHARACTER pkKiller, LPCHARACTER pkVictim);
	int GetKillMobCount();
	int GetKillStoneCount();
	bool IsUsePotion();
	bool IsUseRevive();
	void UsePotion(LPCHARACTER ch);
	void UseRevive(LPCHARACTER ch);

	long GetMapIndex() noexcept { return m_lMapIndex; }

	void Spawn(uint32_t vnum, const char* pos);
#ifdef ENABLE_SUNG_MAHI_TOWER
	LPCHARACTER	SpawnMob(uint32_t vnum, int x, int y, int dir = 0, bool isNomove = false);
#else
	LPCHARACTER SpawnMob(uint32_t vnum, int x, int y, int dir = 0);
#endif
	LPCHARACTER SpawnMob_ac_dir(uint32_t vnum, int x, int y, int dir = 0);
	LPCHARACTER SpawnGroup(uint32_t vnum, long x, long y, float radius, bool bAggressive = false, int count = 1);

	void SpawnNameMob(uint32_t vnum, int x, int y, const char* name);
	void SpawnGotoMob(long lFromX, long lFromY, long lToX, long lToY);

	void SpawnRegen(const char* filename, bool bOnce = true);
	void AddRegen(LPREGEN regen);
	void ClearRegen();
	bool IsValidRegen(LPREGEN regen, size_t regen_id);

	void SetUnique(const char* key, uint32_t vid);
#ifdef ENABLE_SUNG_MAHI_TOWER
	void SetUniqueMaster(const char* key);
	void ClearDungeonFlags() { m_map_Flag.clear(); }

	void SetDungeonDifficulty(uint8_t dungeonLevel) { m_bDungeon_Difficulty = dungeonLevel; }
	uint8_t GetDungeonDifficulty() const { return m_bDungeon_Difficulty; }
#endif
	void SpawnMoveUnique(const char* key, uint32_t vnum, const char* pos_from, const char* pos_to);
	void SpawnMoveGroup(uint32_t vnum, const char* pos_from, const char* pos_to, int count = 1);
	void SpawnUnique(const char* key, uint32_t vnum, const char* pos);
	void SpawnStoneDoor(const char* key, const char* pos);
	void SpawnWoodenDoor(const char* key, const char* pos);
	void KillUnique(const std::string& key);
	void PurgeUnique(const std::string& key);
	bool IsUniqueDead(const std::string& key);
	float GetUniqueHpPerc(const std::string& key);
	uint32_t GetUniqueVid(const std::string& key);

	void DeadCharacter(LPCHARACTER ch);

	void UniqueSetMaxHP(const std::string& key, int iMaxHP);
	void UniqueSetHP(const std::string& key, int iHP);
	void UniqueSetDefGrade(const std::string& key, int iGrade);

	void SendDestPositionToParty(LPPARTY pParty, long x, long y);

	void CheckEliminated();

	void JumpAll(long lFromMapIndex, int x, int y);
	void WarpAll(long lFromMapIndex, int x, int y);
	void JumpParty(LPPARTY pParty, long lFromMapIndex, int x, int y);
#ifdef ENABLE_D_NJGUILD
	void JumpGuild(CGuild* pGuild, long lFromMapIndex, int x, int y);
#endif

	void ExitAll();
	void ExitAllToStartPosition();
	void JumpToEliminateLocation();
	void SetExitAllAtEliminate(long time);
	void SetWarpAtEliminate(long time, long lMapIndex, int x, int y, const char* regen_file);

	int GetFlag(std::string name);
	void SetFlag(std::string name, int value);
	void SetWarpLocation(long map_index, int x, int y);

	// An item group consists of item_vnum and item_count.
	typedef std::vector <std::pair <uint32_t, int> > ItemGroup;
	void CreateItemGroup(std::string& group_name, ItemGroup& item_group);
	const ItemGroup* GetItemGroup(std::string& group_name);
	//void InsertItemGroup (std::string& group_name, uint32_t item_vnum);

#ifdef ENABLE_NEWSTUFF //LEVEL_RANGE_EXTEND
	LPCHARACTER SpawnGroupLevelRange(uint32_t vnum, long x, long y, float radius, bool bAggressive, int count, uint8_t min_level, uint8_t max_level, bool bIncreaseHP, bool bIncreaseDamage);
#endif

	template <class Func> Func ForEachMember(Func f);

	bool IsAllPCNearTo(int x, int y, int dist);

protected:
	CDungeon(IdType id, long lOriginalMapIndex, long lMapIndex);

	void Initialize();
	void CheckDestroy();

private:
	IdType m_id; // <Factor>
	uint32_t m_lOrigMapIndex;
	uint32_t m_lMapIndex;

	CHARACTER_SET m_set_pkCharacter;
	std::map<std::string, int> m_map_Flag;
	typedef std::map<std::string, ItemGroup> ItemGroupMap;
	ItemGroupMap m_map_ItemGroup;
	TPartyMap m_map_pkParty;
	TAreaMap& m_map_Area;
	TUniqueMobMap m_map_UniqueMob;
#ifdef ENABLE_SUNG_MAHI_TOWER
	uint8_t m_bDungeon_Difficulty;
#endif

	int m_iMobKill;
	int m_iStoneKill;
	bool m_bUsePotion;
	bool m_bUseRevive;

	int m_iMonsterCount;
#ifdef ENABLE_12ZI
	int m_iZodiacMonsterCount;
#endif

	bool m_bExitAllAtEliminate;
	bool m_bWarpAtEliminate;

	// Warp position when an enemy is annihilated
	int m_iWarpDelay;
	long m_lWarpMapIndex;
	long m_lWarpX;
	long m_lWarpY;
	std::string m_stRegenFile;

	std::vector<LPREGEN> m_regen;

	LPEVENT deadEvent;
	// <Factor>
	LPEVENT exit_all_event_;
	LPEVENT jump_to_event_;
	size_t regen_id_;

	friend class CDungeonManager;
	friend EVENTFUNC(dungeon_dead_event);
	// <Factor>
	friend EVENTFUNC(dungeon_exit_all_event);
	friend EVENTFUNC(dungeon_jump_to_event);

	// Temporary variable for party-level dungeon entry.
	// It is judged that m_map_pkParty cannot be used due to poor management,
	// Create a variable that temporarily manages one party.

	LPPARTY m_pParty;
#ifdef ENABLE_D_NJGUILD
	CGuild* m_pGuild;
#endif
public:
	void SetPartyNull();

#ifdef ENABLE_DUNGEON_DEAD_TIME_RENEWAL
public:
	void SetDeadEvent(bool bDeadEvent) noexcept { m_bIsDeadEvent = bDeadEvent; };
	long GetDeadTime() noexcept { return m_lDeadTime; };
	void SetDeadTime(long lDeadTime) noexcept { m_lDeadTime = lDeadTime; };

private:
	bool m_bIsDeadEvent;
	long m_lDeadTime;
#endif
#ifdef ENABLE_DUNGEON_RENEWAL
private:
	std::map<uint32_t, std::string> m_Participants{};
public:
	void RegisterParticipant(const LPCHARACTER& pkChar);
	bool IsParticipantRegistered(uint32_t dwPlayerID);
	void ClearParticipants() noexcept;
#endif

#ifdef ENABLE_DEFENSE_WAVE
protected:
	LPCHARACTER m_Mast;

public:
	LPCHARACTER GetMast() noexcept { return m_Mast; }
	void SetMast(LPCHARACTER Mast) noexcept { m_Mast = Mast; }
	void UpdateMastHP(bool woodBuff = false);
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
protected:
	void FinishDungeon();

public:
	void SetDungeonID(const uint32_t id) {
		dungeon_id = id;
	}

	void SetStartTime(uint32_t startTime) { startTime_ = startTime; }
	uint32_t GetStartTime() const { return startTime_; }

	void SetEndTime(uint32_t endTime) { endTime_ = endTime; FinishDungeon(); }
	uint32_t GetEndTime() const { return endTime_; }

	uint32_t GetFinishTime() const { return endTime_ - startTime_; };

protected:
	uint32_t dungeon_id;
	uint32_t startTime_;
	uint32_t endTime_;
#endif
};

class CDungeonManager : public singleton<CDungeonManager>
{
	typedef std::map<CDungeon::IdType, LPDUNGEON> TDungeonMap;
	typedef std::map<long, LPDUNGEON> TMapDungeon;

public:
	CDungeonManager();
	virtual ~CDungeonManager();

	LPDUNGEON Create(long lOriginalMapIndex);
	void Destroy(CDungeon::IdType dungeon_id);
	LPDUNGEON Find(CDungeon::IdType dungeon_id);
	LPDUNGEON FindByMapIndex(long lMapIndex);

private:
	TDungeonMap m_map_pkDungeon;
	TMapDungeon m_map_pkMapDungeon;

	// <Factor> Introduced unsigned 32-bit dungeon identifier
	CDungeon::IdType next_id_;
};

#endif
