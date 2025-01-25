#ifndef __INC_GUILD_DRAGONLAIR_SYSTEM_H__
#define __INC_GUILD_DRAGONLAIR_SYSTEM_H__

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#include "../../common/length.h"
#include "../../common/item_length.h"
#include "../../common/tables.h"
#include "utils.h"
#include "guild.h"
#include "char_manager.h"
#include "guild_manager.h"
#include "sectree_manager.h"

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
#	include "party.h"
#endif

#define MELEY_NOTICE_ALL
#define MELEY_FORCE_WARPOUT
//#define GUILD_DRAGONLAIR_TICKET
//#define MELEY_REWARD_ITEM_DROP

#define MELEY_GOLD_DROP
#ifdef MELEY_GOLD_DROP
#	define MELEY_MIN_GUILD_DROP_GOLD 70000000
#	define MELEY_MAX_GUILD_DROP_GOLD 78000000
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
#		define MELEY_MIN_PARTY_DROP_GOLD 40000000
#		define MELEY_MAX_PARTY_DROP_GOLD 45000000
#	endif
#endif

#define MELEY_ENABLE_COOLDOWN
#ifdef MELEY_ENABLE_COOLDOWN
#	define MELEY_GUILD_COOLDOWN_FLAG "meleylair.guild_duration"
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
#		define MELEY_PARTY_COOLDOWN_FLAG "meleylair.party_duration"
#	endif
#endif

enum eDefault
{
	MELEY_SUBMAP_INDEX = 62,
	MELEY_PARTECIPANTS_MAX_LIMIT = 16,
	MELEY_NPC_VNUM = 20419,
	MELEY_NPC_VNUM2 = 20421,
	MELEY_GATE_VNUM = 20388,
	MELEY_BOSS_VNUM = 6193,
	MELEY_REWARD_ITEMCHEST_VNUM_1 = 50270, // Guild
	MELEY_REWARD_ITEMCHEST_VNUM_2 = 50271, // Guild
	MELEY_REWARD_ITEMCHEST_VNUM_3 = 50294, // Party
	MELEY_TIME_LIMIT_DUNGEON = 3600,
	MELEY_SEAL_VNUM_KILL_STATUE = 30341,
#ifdef ENABLE_AZYRAH_CUSTOM_CHANGES
	MELEY_TIME_LIMIT_TO_KILL_STATUE = 60,
#else
	MELEY_TIME_LIMIT_TO_KILL_STATUE = 10,
#endif
	MELEY_TIME_RESPAWN_COMMON_STEP1 = 5,
	MELEY_TIME_RESPAWN_COMMON_STEP2 = 30,
	MELEY_MOBCOUNT_RESPAWN_COMMON_STEP = 12,
	MELEY_MOBCOUNT_RESPAWN_STONE_STEP2 = 4,
	MELEY_TIME_RESPAWN_COMMON_STEP3 = 30,
	MELEY_MOBVNUM_RESPAWN_SUBBOSS_STEP3 = 6117,
	MELEY_MOBCOUNT_RESPAWN_BOSS_STEP3 = 4,
	MELEY_END_WARP_TIME = 600,
	MELEY_EXPIRE_WARP_TIME = 60,

#ifdef MELEY_FORCE_WARPOUT
	MELEY_WARP_OUT_X = 597500,
	MELEY_WARP_OUT_Y = 699700,
#endif
};

enum eGuild
{
	MELEY_GUILD_CHEST_VNUM = 20420,
	MELEY_GUILD_COOLDOWN_DUNGEON = 5400/*60 * 60 * 2*/,
#ifdef ENABLE_AZYRAH_CUSTOM_CHANGES
	MELEY_PLAYER_GUILD_MIN_LEVEL = 75,
#else
	MELEY_PLAYER_GUILD_MIN_LEVEL = 90,
#endif
	MELEY_MIN_GUILD_LEVEL = 3,
	MELEY_GUILD_STATUE_VNUM = 6118,
	MELEY_GUILD_LADDER_POINTS_COST = 600,
	MELEY_GUILD_LADDER_POINTS_RETURN = 300,
	MELEY_GUILD_MOBVNUM_RESPAWN_COMMON_STEP1 = 6112,
	MELEY_GUILD_MOBVNUM_RESPAWN_COMMON_STEP2 = 6113,
	MELEY_GUILD_MOBVNUM_RESPAWN_STONE_STEP2 = 20422,
	MELEY_GUILD_MOBVNUM_RESPAWN_COMMON_STEP3 = 6115,
	MELEY_GUILD_MOBVNUM_RESPAWN_BOSS_STEP3 = 6116,
#ifdef GUILD_DRAGONLAIR_TICKET
	MELEY_GUILD_TICKET_VNUM = 71095,
#endif
};

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
enum eParty
{
	MELEY_PARTY_CHEST_VNUM = 20501,
#	ifdef MELEY_ENABLE_COOLDOWN
	MELEY_PARTY_COOLDOWN_DUNGEON = 60 * 60 * 2,
#	endif
	MELEY_PLAYER_PARTY_MIN_LEVEL = 75,
	MELEY_PARTECIPANTS_MIN_LIMIT = 4,
	MELEY_PARTY_STATUE_VNUM = 6209,
	MELEY_PARTY_MOBVNUM_RESPAWN_COMMON_STEP1 = 6203,
	MELEY_PARTY_MOBVNUM_RESPAWN_COMMON_STEP2 = 6202,
	MELEY_PARTY_MOBVNUM_RESPAWN_STONE_STEP2 = 20500,
	MELEY_PARTY_MOBVNUM_RESPAWN_COMMON_STEP3 = 6206,
	MELEY_PARTY_MOBVNUM_RESPAWN_BOSS_STEP3 = 6207,
};
#endif

enum eType
{
	MELEY_TYPE_GUILD_RED_DRAGON_LAIR = 1,
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	MELEY_TYPE_PARTY_RED_DRAGON_LAIR = 2,
#endif
};

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
struct FPartyPIDCollector
{
	std::vector<uint32_t> vecPIDs;
	FPartyPIDCollector() noexcept {}
	void operator () (LPCHARACTER ch)
	{
		if (ch)
			vecPIDs.emplace_back(ch->GetPlayerID());
	}
};
#endif

class CMeleyLair
{
public:
	CMeleyLair(long lMapIndex);
	~CMeleyLair();

	void Destroy();
	void SendMeleyNotice(const char* szNotice);

	void SetDungeonType(uint16_t bType) noexcept { dungeon_type = bType; };
	uint16_t GetDungeonType() noexcept { return dungeon_type; };
	const long GetMapIndex() noexcept { return map_index; }
	const uint16_t GetDungeonStep() noexcept { return dungeon_step; }
	void SetDungeonStep(uint16_t bStep);
	void StartDungeonStep(uint16_t bStep);
	const uint32_t GetDungeonTimeStart() noexcept { return time_start; }
	void SetDungeonTimeStart(uint32_t dwTime) noexcept { time_start = dwTime; }
	const uint32_t GetLastStoneKilledTime() noexcept { return last_stoneKilled; }
	void SetLastStoneKilledTime(uint32_t dwTime) noexcept { last_stoneKilled = dwTime; }
	const uint32_t GetKillCountStones() noexcept { return kill_stonesCount; }
	void SetKillCountStones(uint32_t dwCount) noexcept { kill_stonesCount = dwCount; }
	const uint32_t GetKillCountBosses() noexcept { return kill_bossesCount; }
	void SetKillCountBosses(uint32_t dwCount) noexcept { kill_bossesCount = dwCount; }
	const uint32_t GetRewardTime() noexcept { return reward; }
	void SetRewardTime(uint32_t dwTime) noexcept { reward = dwTime; }
	void RegisterParticipant(const LPCHARACTER& pkChar);
	bool IsParticipantRegistered(uint32_t dwPlayerID);
	uint32_t GetParticipantsCount() noexcept { return ParticipantsName.size(); }
	void Start();
	void StartDungeon(LPCHARACTER pkChar);
	void EndDungeon(bool bSuccess, bool bIsCancelled = false);
	void EndDungeonWarp();
	bool Damage(LPCHARACTER pkStatue);
	void OnKill(uint32_t dwVnum);
	void OnKillCommon(const LPCHARACTER& pkMonster);
	void OnKillStatue(LPITEM pkItem, const LPCHARACTER& pkChar, LPCHARACTER pkStatue);
	void DungeonResult();
	bool CheckRewarder(uint32_t dwPlayerID);
	void GiveReward(LPCHARACTER pkChar, uint16_t bReward);

	bool IsPlayerOut(uint32_t dwPlayerID);
	void SetPlayerOut(uint32_t dwPlayerID);

	void SetMainNPC(LPCHARACTER pk_MainNPC) noexcept { pkMainNPC = pk_MainNPC; }
	void SetGateChar(LPCHARACTER pk_Gate) noexcept { pkGate = pk_Gate; }

	LPCHARACTER Spawn(uint32_t dwVnum, int iX, int iY, int iDir = 0, bool bSpawnMotion = false);
	const LPCHARACTER& GetMainNPC() noexcept { return pkMainNPC; }
	const LPCHARACTER& GetGateChar() noexcept { return pkGate; }
	const LPCHARACTER& GetBossChar() noexcept { return pkBoss; }
	const LPCHARACTER& GetStatue1Char() noexcept { return pkStatue1; }
	const LPCHARACTER& GetStatue2Char() noexcept { return pkStatue2; }
	const LPCHARACTER& GetStatue3Char() noexcept { return pkStatue3; }
	const LPCHARACTER& GetStatue4Char() noexcept { return pkStatue4; }

	const LPSECTREE_MAP& GetMapSectree() noexcept { return pkSectreeMap; }

	// Guild
	CGuild* GetGuild() noexcept { return p_pGuild; };
	void SetGuild(CGuild* pGuild) noexcept { p_pGuild = pGuild; };

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	// Party
	void SetParty(LPPARTY pParty) noexcept { p_Party = pParty; }
	const LPPARTY GetParty() noexcept { return p_Party; }
	void SetPartyLeaderName(std::string pszLeaderName) { p_szLeaderName = pszLeaderName; }
	std::string GetPartyLeaderName() { return p_szLeaderName; }
	void SetFirstRankingTime(uint32_t dwTime) noexcept { dwFirstRankingTime = dwTime; }
	const time_t GetFirstRankingTime() noexcept { return dwFirstRankingTime; }
	const time_t GetElapsedTime() noexcept { return GetDungeonTimeStart() ? (get_global_time() - GetDungeonTimeStart()) : 0; }

	bool IsDungeonTimeSuccess() noexcept { return bIsDungeonTimeSuccess; };
	void SetDungeonTimeSuccess() noexcept { bIsDungeonTimeSuccess = true; };
#endif

	bool IsDungeonCompleted() noexcept { return bIsDungeonCompleted; };
	void SetDungeonCompleted() noexcept { bIsDungeonCompleted = true; };

	//LPDUNGEON GetDungeon() { return pkDungeon; };

private:
	//LPDUNGEON pkDungeon;

	CGuild* p_pGuild;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	LPPARTY p_Party;
#endif

	uint16_t dungeon_type;
	long map_index;
	uint32_t time_start;
	uint32_t last_stoneKilled;
	uint32_t kill_stonesCount;
	uint32_t kill_bossesCount;
	uint32_t reward;
	uint16_t dungeon_step;

	std::vector<uint32_t> vPlayersStatue;
	std::vector<uint32_t> vPlayersReward;
	std::vector<uint32_t> vPlayersOut;

	LPSECTREE_MAP pkSectreeMap;
	LPCHARACTER pkMainNPC;
	LPCHARACTER pkGate;
	LPCHARACTER pkBoss;
	LPCHARACTER pkStatue1;
	LPCHARACTER pkStatue2;
	LPCHARACTER pkStatue3;
	LPCHARACTER pkStatue4;

	std::map<uint32_t, std::string> ParticipantsName;

	uint32_t dwFirstRankingTime;
	bool bIsDungeonCompleted;

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	std::string p_szLeaderName;
	bool bIsDungeonTimeSuccess;
#endif

	bool bRemainMessage{false};

protected:
	LPEVENT e_pEndEvent;
	LPEVENT e_pWarpEvent;
	LPEVENT e_SpawnEvent;
	LPEVENT e_SEffectEvent;
	LPEVENT e_DestroyStatues;
};

class CMeleyLairManager : public singleton<CMeleyLairManager>
{
	typedef std::map<long, CMeleyLair*> TMapDungeon;
	typedef std::map<CGuild*, CMeleyLair*> TMapGuild;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	typedef std::map<LPPARTY, CMeleyLair*> TMapParty;
#endif
public:
	void Initialize();
	void Destroy();

	CMeleyLair* FindByMapIndex(long lMapIndex);
	void DestroyPrivateMap(long lMapIndex);

	uint16_t GetCharacterDungeonType(const LPCHARACTER& pkChar);

	void EnterGame(LPCHARACTER pkChar);
	//void LeaveGame(LPCHARACTER pkChar);
	void LeaveRequest(LPCHARACTER pkChar);
	bool IsMeleyMap(long lMapIndex) noexcept;
	void ExitCharacter(LPCHARACTER pkChar);
	bool CanGetReward(LPCHARACTER pkChar);
	void Reward(LPCHARACTER pkChar, uint16_t bReward);
	void OpenRanking(const LPCHARACTER& pkChar);

	void SetXYZ(long lX, long lY, long lZ) noexcept { lMapCenterPos.x = lX, lMapCenterPos.y = lY, lMapCenterPos.z = lZ; }
	const PIXEL_POSITION& GetXYZ() noexcept { return lMapCenterPos; }
	void SetSubXYZ(long lX, long lY, long lZ) noexcept { lSubMapPos.x = lX, lSubMapPos.y = lY, lSubMapPos.z = lZ; }
	const PIXEL_POSITION& GetSubXYZ() noexcept { return lSubMapPos; }

	// Guild
	bool isGuildRegistered(const CGuild* pkGuild, int& iCH) noexcept;
	void RegisterGuild(const LPCHARACTER& pkChar, int& iRes1, int& iRes2);
	bool EnterGuild(LPCHARACTER pkChar, int& iLimit);
	void ClearGuild(CGuild* pkGuild);
	void GuildMemberRemoved(LPCHARACTER pkChar, CGuild* pkGuild);
	void GuildRemoved(CGuild* pkGuild);

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	// Party
	bool IsPartyMemberRegistered(const LPCHARACTER& pkChar);
	bool EnterParty(LPCHARACTER pkChar, int& iLimit);
	void ClearParty(LPPARTY pkParty);
#endif

	void StartDungeon(LPCHARACTER pkChar);
	void EndDungeon(LPCHARACTER pkChar, bool bSuccess, bool bIsCancelled = false);
	uint16_t GetDungeonStep(const LPCHARACTER& pkChar) noexcept;

private:
	PIXEL_POSITION lMapCenterPos, lSubMapPos;
	TMapDungeon m_map_pkMapDungeon;
	TMapGuild m_RegGuilds;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	TMapParty m_RegPartys;
#endif
};
#endif

#endif /* __INC_GUILD_DRAGONLAIR_SYSTEM_H__ */
