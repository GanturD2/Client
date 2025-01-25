#ifndef __INC_METIN_II_GAME_CHARACTER_MANAGER_H__
#define __INC_METIN_II_GAME_CHARACTER_MANAGER_H__

#include "../../common/stl.h"
#include "../../common/length.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#include "../../common/tables.h"
#endif
#include "vid.h"

class CDungeon;
class CHARACTER;
class CharacterVectorInteractor;

class CHARACTER_MANAGER : public singleton<CHARACTER_MANAGER>
{
public:
	typedef std::unordered_map<std::string, LPCHARACTER> NAME_MAP;

	CHARACTER_MANAGER();
	virtual ~CHARACTER_MANAGER();

	void Destroy();

	void GracefulShutdown(); // Used for normal shutdown. Save all PCs and destroy.

	uint32_t AllocVID();

	LPCHARACTER CreateCharacter(const char* name, uint32_t dwPID = 0);
	void DestroyCharacter(LPCHARACTER ch);

	void Update(int iPulse);

	LPCHARACTER SpawnMob(uint32_t dwVnum, long lMapIndex, long x, long y, long z, bool bSpawnMotion = false, int iRot = -1, bool bShow = true);
	LPCHARACTER SpawnMobRange(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, bool bIsException = false, bool bSpawnMotion = false, bool bAggressive = false
#ifdef ENABLE_12ZI
		, uint8_t bLevel = 0
#endif
	);
	LPCHARACTER SpawnGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPDUNGEON pDungeon = nullptr);
	bool SpawnGroupGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPDUNGEON pDungeon = nullptr);
	bool SpawnMoveGroup(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, int tx, int ty, LPREGEN pkRegen = nullptr, bool bAggressive_ = false);
#ifdef ENABLE_12ZI
	LPCHARACTER SpawnGroupZodiac(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPZODIAC pZodiac = nullptr, uint8_t bLevel = 0);
	bool SpawnGroupGroupZodiac(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPZODIAC pZodiac = nullptr, uint8_t bLevel = 0);
#endif
#ifdef ENABLE_NEWSTUFF //LEVEL_RANGE_EXTEND
	LPCHARACTER SpawnGroupLevelRange(uint32_t dwVnum, long lMapIndex, int sx, int sy, int ex, int ey, uint8_t min_level, uint8_t max_level,
		bool bIncreaseHP = false, bool bIncreaseDamage = false, LPREGEN pkRegen = nullptr, bool bAggressive_ = false, LPDUNGEON pDungeon = nullptr);
#endif
	LPCHARACTER SpawnMobRandomPosition(uint32_t dwVnum, long lMapIndex);

	void SelectStone(LPCHARACTER pkChrStone);

	NAME_MAP& GetPCMap() { return m_map_pkPCChr; }

	LPCHARACTER Find(uint32_t dwVID);
	LPCHARACTER Find(const VID& vid);
	LPCHARACTER FindPC(const char* name);
	LPCHARACTER FindByPID(uint32_t dwPID);

	bool AddToStateList(LPCHARACTER ch);
	void RemoveFromStateList(LPCHARACTER ch);

	// DelayedSave: Save when you do a lot of things that need to be saved in a routine
	// Since there are too many queries, just mark "Save" and wait for a while
	// Save after (eg 1 frame).
	void DelayedSave(LPCHARACTER ch);
	bool FlushDelayedSave(LPCHARACTER ch); // If it is in the Delayed list, delete it and save it. Used for break handling.
	void ProcessDelayedSave();

	template<class Func> Func for_each_pc(Func f);

	void RegisterForMonsterLog(LPCHARACTER ch);
	void UnregisterForMonsterLog(LPCHARACTER ch);
	void PacketMonsterLog(LPCHARACTER ch, const void* buf, int size);

	void KillLog(uint32_t dwVnum);

	void RegisterRaceNum(uint32_t dwVnum);
	void RegisterRaceNumMap(LPCHARACTER ch);
	void UnregisterRaceNumMap(LPCHARACTER ch);
	CharacterVectorInteractor GetCharactersByRaceNum(uint32_t dwRaceNum);
#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_MYSHOP_DECO_PREMIUM)
	void GetCharactersShopRaceNum(std::vector<CharacterVectorInteractor>& i);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	int CountCharactersByRaceNum(uint32_t dwRaceNum);
	int CountOfflineShops();
#endif

	LPCHARACTER FindSpecifyPC(uint32_t uiJobFlag, long lMapIndex, LPCHARACTER except = nullptr, int iMinLevel = 1, int iMaxLevel = PLAYER_MAX_LEVEL_CONST);

	void SetMobItemRate(int value) { m_iMobItemRate = value; }
	void SetMobDamageRate(int value) { m_iMobDamageRate = value; }
	void SetMobGoldAmountRate(int value) { m_iMobGoldAmountRate = value; }
	void SetMobGoldDropRate(int value) { m_iMobGoldDropRate = value; }
	void SetMobExpRate(int value) { m_iMobExpRate = value; }

	void SetMobItemRatePremium(int value) { m_iMobItemRatePremium = value; }
	void SetMobGoldAmountRatePremium(int value) { m_iMobGoldAmountRatePremium = value; }
	void SetMobGoldDropRatePremium(int value) { m_iMobGoldDropRatePremium = value; }
	void SetMobExpRatePremium(int value) { m_iMobExpRatePremium = value; }

	void SetUserDamageRatePremium(int value) { m_iUserDamageRatePremium = value; }
	void SetUserDamageRate(int value) { m_iUserDamageRate = value; }
	int GetMobItemRate(LPCHARACTER ch);
	int GetMobDamageRate(LPCHARACTER ch);
	int GetMobGoldAmountRate(LPCHARACTER ch);
	int GetMobGoldDropRate(LPCHARACTER ch);
	int GetMobExpRate(LPCHARACTER ch);

	int GetUserDamageRate(LPCHARACTER ch);
	void SendScriptToMap(long lMapIndex, const std::string& s);

	bool BeginPendingDestroy();
	void FlushPendingDestroy();

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	void InitPortalLevelLimit(const char* FileName);
	void AddPortalLevelLimit(uint32_t dwRace, int iMinLevel, int iMaxLevel);
	const std::pair<int, int>* GetPortalLevelLimit(uint32_t dwRace) const;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	LPCHARACTER SpawnShop(LPCHARACTER ch, std::string shopSign, TShopItemTable* shopItemTable, uint8_t itemCount, uint32_t startTime = 0
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
		, uint32_t dwPolyVnum = 0, uint8_t bTitleType = 0, bool isShopDeco = false
#endif
	);
	LPCHARACTER FindPCShopCharacterByPID(uint32_t pid);
#endif

#ifdef ENABLE_SUPERSTONE_RAIN_EVENT
	void InitializeSuperStoneEvent(int iEnable);
#endif
#ifdef ENABLE_MINING_EVENT
	void InitializeMiningEvent(int iEnable);
#endif

#ifdef ENABLE_WORLD_BOSS
public:
	void OnKill(uint32_t vid);
	std::vector <int> WBMapIndexes = { 61, 62, 63, 64 };
	std::vector <int> WBVnums = { 1093, 1093, 1093, 1093, 1093 };

protected:
	uint32_t m_dwWBVID;
	LPCHARACTER pkWB;
	long long m_lWBPhase;
	long long m_lWBCooldown;
	uint8_t m_bWBState;
#endif

private:
	int m_iMobItemRate;
	int m_iMobDamageRate;
	int m_iMobGoldAmountRate;
	int m_iMobGoldDropRate;
	int m_iMobExpRate;

	int m_iMobItemRatePremium;
	int m_iMobGoldAmountRatePremium;
	int m_iMobGoldDropRatePremium;
	int m_iMobExpRatePremium;

	int m_iUserDamageRate;
	int m_iUserDamageRatePremium;
	int m_iVIDCount;

	std::unordered_map<uint32_t, LPCHARACTER> m_map_pkChrByVID;
	std::unordered_map<uint32_t, LPCHARACTER> m_map_pkChrByPID;
	NAME_MAP m_map_pkPCChr;

	char dummy1[1024]; // memory barrier
	CHARACTER_SET m_set_pkChrState; // niggas with FSM running
	CHARACTER_SET m_set_pkChrForDelayedSave;
	CHARACTER_SET m_set_pkChrMonsterLog;

	LPCHARACTER m_pkChrSelectedStone;

	std::map<uint32_t, uint32_t> m_map_dwMobKillCount;

	std::set<uint32_t> m_set_dwRegisteredRaceNum;
	std::map<uint32_t, CHARACTER_SET> m_map_pkChrByRaceNum;

	bool m_bUsePendingDestroy;
	CHARACTER_SET m_set_pkChrPendingDestroy;

#ifdef ENABLE_PORTAL_LEVEL_LIMIT
	std::unordered_map<uint32_t, std::pair<int, int>> m_mapPlayerPortalLevelLimit;
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
public:
	int GetMultiFarmCount(const char* playerIP, std::map<uint32_t, std::pair<std::string, bool>>& m_mapNames);
	void CheckMultiFarmAccount(const char* szIP, const uint32_t playerID, const char* playerName, const bool bStatus, uint8_t affectType = 0, int affectDuration = 0, bool isP2P = false);
	void SetMultiFarm(const char* szIP, const uint32_t playerID, const char* playerName, const bool bStatus, const uint8_t affectType, const int affectTime);
	void RemoveMultiFarm(const char* szIP, const uint32_t playerID, const bool isP2P);
	void CheckMultiFarmAccounts(const char* szIP);

protected:
	std::map<std::string, std::vector<TMultiFarm>> m_mapmultiFarm;
#endif
};

template<class Func>
Func CHARACTER_MANAGER::for_each_pc(Func f)
{
	std::unordered_map<uint32_t, LPCHARACTER>::iterator it;

	for (it = m_map_pkChrByPID.begin(); it != m_map_pkChrByPID.end(); ++it)
		f(it->second);

	return f;
}

class CharacterVectorInteractor : public CHARACTER_VECTOR
{
public:
	CharacterVectorInteractor() : m_bMyBegin(false) { }

	CharacterVectorInteractor(const CHARACTER_SET& r);
	virtual ~CharacterVectorInteractor();

private:
	bool m_bMyBegin;
};

#define M2_DESTROY_CHARACTER(ptr) CHARACTER_MANAGER::Instance().DestroyCharacter(ptr)

#endif
