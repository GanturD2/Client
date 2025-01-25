#ifndef __INC_METIN_II_GAME_ZODIAC_H
#define __INC_METIN_II_GAME_ZODIAC_H

#include "../../common/CommonDefines.h"
#include "../../common/length.h"

#ifdef ENABLE_12ZI
#include "dungeon.h"
#include <unordered_map>
#include "sectree_manager.h"

#ifdef ENABLE_SERVERTIME_PORTAL_SPAWN
class CZodiac : public singleton<CZodiac>
#else
class CZodiac
#endif
{
	typedef std::unordered_map<LPPARTY, int> TPartyMap;
	typedef std::map<std::string, LPCHARACTER> TUniqueMobMap;

public:
	typedef uint32_t IdType;

	~CZodiac();

	IdType GetId() const { return m_id; }

	void SetFloor(uint8_t completedFloor);
	void SetFlag(std::string name, int value);
	int GetFlag(std::string name);

	void Jump(LPCHARACTER ch, long lFromMapIndex, int x, int y);
	void JumpAll(long lFromMapIndex, int x, int y);
	void JumpParty(LPPARTY pParty, long lFromMapIndex, int x, int y);

	void SpawnRegen(const char * filename, bool bOnce = true);
#ifdef ENABLE_SERVERTIME_PORTAL_SPAWN
	void SpawnRegenZodiac(const char * filename, bool bOnce = true);
#endif
	void AddRegen(LPREGEN regen);
	void ClearRegen();
	bool IsValidRegen(LPREGEN regen, size_t regen_id);

	void ZodiacMessage(const char * msg);
	void ZodiacMessageClear();

	void ZodiacMessageMission(uint8_t Floor);
	void ZodiacFloorMessage(uint8_t Floor, uint8_t Count = 0);
	void ZodiacFloorSubMessage(const char * msg);

	void ZodiacTime(uint8_t currentfloor, uint8_t nextfloor, int time, int time2 = 0);
	void ZodiacTimeClear();

	long GetMapIndex() { return m_lMapIndex; }
	uint8_t GetPortal() { return m_bPortal; }
	uint8_t GetFloor() { return m_bFloor; }
	uint8_t GetNextFloor() { return m_bNextFloor; }
	bool IsNextFloor() { return m_bNextFloorControl; }
	uint32_t GetTime() { return m_dwGetGlobalTime; }

	void ControlMob();
	int CountMonster();
	int CountIsBoss();
	int CountIsStone();
	bool ControlCharacter();
	void CheckPlayers(LPPARTY pParty);

	void ExitTemple();
	void TimeIsUp();

	void Purge();
	void KillAll();

	void NewFloor(uint8_t Floor);

	void SetParty(LPPARTY pParty);
	void JoinParty(LPPARTY pParty);
	void QuitParty(LPPARTY pParty);

	void Join(LPCHARACTER ch);

	void IncMember(LPCHARACTER ch);
	void DecMember(LPCHARACTER ch);

	void IncPartyMember(LPPARTY pParty, LPCHARACTER ch);
	void DecPartyMember(LPPARTY pParty, LPCHARACTER ch);

	void IncMonster() { m_iMonsterCount++; sys_log(0, "MonsterCount %d", m_iMonsterCount); }
	void DecMonster() { m_iMonsterCount--; }

	//
	void CheckPolymorphedCharacters();
	//

	void PolyCharacter(LPCHARACTER Mob);
	void ProcessZodiacItems(bool enable);

	void NextFloorButton();
	void SpawnMob();
	void SpawnStone(bool Statue = false);
	void SpawnBoss(uint8_t Flag = 0);

	// Rewards when you complete each floor (i think could be improved)
	void GiveGiftBox();

	void GiveGiftZodiacSmall();
	void GiveGiftZodiacMiddle();
	void GiveGiftZodiacBig();

	void GiveGiftZi();
	void GiveGiftChou();
	void GiveGiftYin();
	void GiveGiftMao();
	void GiveGiftChen();
	void GiveGiftSi();
	void GiveGiftWu();
	void GiveGiftWei();
	void GiveGiftShen();
	void GiveGiftYu();
	void GiveGiftXu();
	void GiveGiftHai();

	void StartLogin();

protected:
	CZodiac(IdType id, long lMapIndex, uint8_t bPortal);
	void Initialize();

private:
	IdType m_id;
	long m_lMapIndex;
	uint8_t m_bPortal;
	uint8_t m_bFloor;
	uint8_t m_bNextFloor;
	bool m_bNextFloorControl;
	uint8_t m_bPosition;
	uint32_t m_dwGetGlobalTime;
	bool m_bTimeDown;

	CHARACTER_SET m_set_pkCharacter;
	std::map<std::string, int> m_map_Flag;

	TPartyMap m_map_pkParty;
	TUniqueMobMap m_map_UniqueMob;

	int m_iMonsterCount;

	int m_iMobKill;
	int m_iStoneKill;
	int m_iBossKill;
	int m_iTotalMonster;

	std::vector<LPREGEN> m_regen;

	size_t regen_id_;

	LPEVENT zodiac_floor_event_data;
	LPEVENT zodiac_remaining_time_temple_floor;
	LPEVENT zodiac_exit_temple_event_data;
	LPEVENT deadEvent;

	friend class CZodiacManager;
	friend EVENTFUNC(Floor_event_data_start);
	friend EVENTFUNC(Floor_remaining_time_event_data_start);
	friend EVENTFUNC(Exit_temple_event_data_start);
	friend EVENTFUNC(zodiac_dead_event);

	LPPARTY m_pParty;

public:
	void SetPartyNull();
};

class CZodiacManager : public singleton<CZodiacManager>
{
	typedef std::map<CZodiac::IdType, LPZODIAC> TZodiacMap;
	typedef std::map<long, LPZODIAC> TMapZodiac;

public:
	CZodiacManager();
	virtual ~CZodiacManager();

#ifdef ENABLE_SERVERTIME_PORTAL_SPAWN
	bool Initialize();
	LPEVENT zodiac_spawn_timer{ nullptr };

	int GetCurrentDay();
	bool GetIsDaily();
	bool SpawnPortals();
#endif

	LPZODIAC Create(uint8_t bPortal);
	void Destroy(CZodiac::IdType zodiac_id);
	LPZODIAC Find(CZodiac::IdType zodiac_id);
	LPZODIAC FindByMapIndex(long lMapIndex);

	bool IsZiStageMapIndex(long lMapIndex) { return ((lMapIndex >= static_cast<int>(MAP_12ZI_STAGE) * 10000) && (lMapIndex < (static_cast<int>(MAP_12ZI_STAGE) + 1) * 10000)); }
	int GetZiStageMapIndex() { return static_cast<int>(MAP_12ZI_STAGE); }

	void StartTemple(LPCHARACTER pkChar, uint8_t portal);

	void DeadMob(LPCHARACTER Mob, uint32_t mapIndex);
	void DeadPC(uint32_t mapIndex);

#ifndef ENABLE_SERVERTIME_PORTAL_SPAWN
private:
#endif
	TZodiacMap m_map_pkZodiac;
	TMapZodiac m_map_pkMapZodiac;
	CZodiac::IdType next_id_;
};
#endif

#endif
