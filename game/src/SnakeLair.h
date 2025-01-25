#include "../../common/CommonDefines.h"

#ifdef ENABLE_QUEEN_NETHIS
#include "../../common/length.h"
#include "../../common/item_length.h"
#include "../../common/tables.h"
#include "guild.h"
#include "char_manager.h"
#include "sectree_manager.h"

namespace SnakeLair
{
	enum eSnakeConfig
	{
		PORTAL_VNUM = 4022,
		COOLDOWN_DUNGEON = 3600,
		TIME_LIMIT_DUNGEON = 3600,

		VNUM_KILL_PILAR = 70422,
		VNUM_KILL_BLACKSMITH = 70427,
		VNUM_BLACKSMITH_SERPENT = 20829,

		PILAR_STEP_4 = 6757,
		BLACKSMITH_5 = 20842,

		//Step Boss
		SNAKE_STATUE1 = 4024, //Fire
		SNAKE_STATUE2 = 4025, //Ice
		SNAKE_STATUE3 = 4026, //Wind
		SNAKE_STATUE4 = 4027, //Earth

		VNUM_ITEM_SNAKE_FIRE = 70423, //Statue1
		VNUM_ITEM_SNAKE_ICE = 70424, //Statue2
		VNUM_ITEM_SNAKE_WIND = 70425, //Statue3
		VNUM_ITEM_SNAKE_GROUND = 70426, //Statue4

		TIME_RESPAWM_MOB = 10,

		SNAKEMETIN_FIRE = 8630,
		SNAKEMETIN_ICE = 8631,
		SNAKEMETIN_WIND = 8632,
		SNAKEMETIN_EARTH = 8633,
		SNAKEMETIN_TIME = 8634,

		SIREN_ICE = 6755,
		SIREN_EARTH = 6765,
		SIREN_FIRE = 6855,

		SERPENT_QEEN = 6756,
	};

	extern int stoneKillSort[4][4];

	extern int EarthMonsterFloor1[15][3];
	extern int FireMonsterFloor1[15][3];
	extern int IceMonsterFloor1[15][3];
	extern int WindMonsterFloor1[15][3];

	extern int iSerpentStoneCoord[6][2];
	extern int iSortStoneCoord[6][4];

	extern int iSerpentFinalBoss[28][3];
	extern int iSungmaQueenPoint[5][8];

#ifdef ENABLE_YOHARA_SYSTEM
	typedef struct SSungmaQueenPoint
	{
		uint8_t bDungeonFloor;
		int iPointType;
		int iValue;
	} TSungmaQueenPoint;
#endif

	class CSnkMap
	{
	public:
		CSnkMap(long lMapIndex);
		~CSnkMap();

		void Destroy();

#ifdef ENABLE_YOHARA_SYSTEM
		void LoadSungmaAttr();
		int GetSungmaValuePoint(int iPoint);
#endif

		void StartDungeon(LPCHARACTER pkChar);
		void Start();
		void EndDungeonWarp();

		void OnKillBlackSmith(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkBlackSmith);
		void OnKillPilar(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkPilar);
		void OnKill(LPCHARACTER pkMonster, LPCHARACTER pKiller);
		void OnStatueSetRotation(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkStatue);

		LPCHARACTER Spawn(uint32_t dwVnum, int iX, int iY, int iDir = 0, bool bSpawnMotion = false);

		/*
		Get and Set
		*/

		void SetMapSectree(LPSECTREE_MAP pkSectree) noexcept { pkSectreeMap = pkSectree; };
		LPSECTREE_MAP& GetMapSectree() noexcept { return pkSectreeMap; };

		void SetMapIndex(long lMapIndex) noexcept { map_index = lMapIndex; };
		long GetMapIndex() noexcept { return map_index; };

		void SetPortalNPC(LPCHARACTER pkPortal) noexcept { pkPortalNPC = pkPortal; };
		LPCHARACTER& GetPortalNPC() noexcept { return pkPortalNPC; };

		void SetParty(LPPARTY pParty) noexcept { pPartyReg = pParty; };
		LPPARTY& GetParty() noexcept { return pPartyReg; };

		void SetDungeonStep(uint8_t bStep);
		uint8_t GetDungeonStep() noexcept { return dungeon_step; };

		void SetElementSort(int iElement) noexcept { iElementSort = iElement; };
		int GetElementSort() noexcept { return iElementSort; };

		void SetSubElementSort(int iElement) noexcept { iSubElementSort = iElement; };
		int GetSubElementSort() noexcept { return iSubElementSort; };

		void SetRotationStatuesCount(uint32_t dwCount) noexcept { dwStatuesCount = dwCount; }
		uint32_t GetRotationStatuesCount() const noexcept { return dwStatuesCount; }

		void SetKillCountMonsters(uint32_t dwCount) noexcept { dwKillMonstersCount = dwCount; };
		uint32_t GetKillCountMonsters() const noexcept { return dwKillMonstersCount; };

		void SetKillCountPilars(uint32_t dwCount) noexcept { dwKillPilarsCount = dwCount; };
		uint32_t GetKillCountPilars() const noexcept { return dwKillPilarsCount; };

		void SetKillCountStones(uint32_t dwCount) noexcept { dwStonesCount = dwCount; };
		uint32_t GetKillCountStones() const noexcept { return dwStonesCount; };

		void SetSnakePilar1(LPCHARACTER pkPilar) noexcept { pPilar1 = pkPilar; };
		LPCHARACTER& GetSnakePilar1() noexcept { return pPilar1; };

		void SetSnakePilar2(LPCHARACTER pkPilar) noexcept { pPilar2 = pkPilar; };
		LPCHARACTER& GetSnakePilar2() noexcept { return pPilar2; };

		void SetSnakePilar3(LPCHARACTER pkPilar) noexcept { pPilar3 = pkPilar; };
		LPCHARACTER& GetSnakePilar3() noexcept { return pPilar3; };

		void SetSnakePilar4(LPCHARACTER pkPilar) noexcept { pPilar4 = pkPilar; };
		LPCHARACTER& GetSnakePilar4() noexcept { return pPilar4; };

		void SetSnakePilar5(LPCHARACTER pkPilar) noexcept { pPilar5 = pkPilar; };
		LPCHARACTER& GetSnakePilar5() noexcept { return pPilar5; };

		void SetSnakePilar6(LPCHARACTER pkPilar) noexcept { pPilar6 = pkPilar; };
		LPCHARACTER& GetSnakePilar6() noexcept { return pPilar6; };

		void SetSnakeBlackSmith1(LPCHARACTER pkBlackSmith) noexcept { pBlackSmith1 = pkBlackSmith; };
		LPCHARACTER& GetSnakeBLackSmith1() noexcept { return pBlackSmith1; };

		void SetSnakeBlackSmith2(LPCHARACTER pkBlackSmith) noexcept { pBlackSmith2 = pkBlackSmith; };
		LPCHARACTER& GetSnakeBLackSmith2() noexcept { return pBlackSmith2; };

		void SetSnakeBlackSmith3(LPCHARACTER pkBlackSmith) noexcept { pBlackSmith3 = pkBlackSmith; };
		LPCHARACTER& GetSnakeBLackSmith3() noexcept { return pBlackSmith3; };

		void SetSnakeBlackSmith4(LPCHARACTER pkBlackSmith) noexcept { pBlackSmith4 = pkBlackSmith; };
		LPCHARACTER& GetSnakeBLackSmith4() noexcept { return pBlackSmith4; };

		void SetSnakeStatue1(LPCHARACTER pkStatue) noexcept { pSnakeStatue1 = pkStatue; };
		LPCHARACTER& GetSnakeStatue1() noexcept { return pSnakeStatue1; };

		void SetSnakeStatue2(LPCHARACTER pkStatue) noexcept { pSnakeStatue2 = pkStatue; };
		LPCHARACTER& GetSnakeStatue2() noexcept { return pSnakeStatue2; }

		void SetSnakeStatue3(LPCHARACTER pkStatue) noexcept { pSnakeStatue3 = pkStatue; };
		LPCHARACTER& GetSnakeStatue3() noexcept { return pSnakeStatue3; };

		void SetSnakeStatue4(LPCHARACTER pkStatue) noexcept { pSnakeStatue4 = pkStatue; };
		LPCHARACTER& GetSnakeStatue4() noexcept { return pSnakeStatue4; };

		void SetBlackSmithSerpent(LPCHARACTER pkBlackSmith) noexcept { pkBlackSmithSerpent = pkBlackSmith; };
		LPCHARACTER& GetBlackSmithSerpent() noexcept { return pkBlackSmithSerpent; }

		void SetBlockSnake1(bool bBlock) noexcept { bBlockSnake1 = bBlock; };
		bool GetBlockSnake1() noexcept { return bBlockSnake1; };

		void SetBlockSnake2(bool bBlock) noexcept { bBlockSnake2 = bBlock; };
		bool GetBlockSnake2() noexcept { return bBlockSnake2; };

		void SetBlockSnake3(bool bBlock) noexcept { bBlockSnake3 = bBlock; };
		bool GetBlockSnake3() noexcept { return bBlockSnake3; };

		void SetBlockSnake4(bool bBlock) noexcept { bBlockSnake4 = bBlock; };
		bool GetBlockSnake4() noexcept { return bBlockSnake4; };

		long map_index;
		LPSECTREE_MAP pkSectreeMap;
		LPCHARACTER pkPortalNPC;

		LPCHARACTER pSnakeStatue1, pSnakeStatue2, pSnakeStatue3, pSnakeStatue4;
		LPCHARACTER pPilar1, pPilar2, pPilar3, pPilar4, pPilar5, pPilar6;
		LPCHARACTER pBlackSmith1, pBlackSmith2, pBlackSmith3, pBlackSmith4;
		LPCHARACTER pkBlackSmithSerpent;

		LPPARTY pPartyReg;
		uint8_t dungeon_step;
		int iElementSort, iSubElementSort;
		uint32_t dwKillMonstersCount, dwKillPilarsCount, dwStatuesCount, dwStonesCount;

		bool bBlockSnake1, bBlockSnake2, bBlockSnake3, bBlockSnake4;

	protected:
		LPEVENT e_SpawnEvent, e_pEndEvent, e_pSkillEvent;

#ifdef ENABLE_YOHARA_SYSTEM
	private:
		std::vector<TSungmaQueenPoint> m_vec_mapSungmaQueenPoint;
#endif
	};

	class CSnk : public singleton<CSnk>
	{
	public:
		void Initialize();
		void Destroy();

		bool IsSnakeMap(long lMapIndex);

		void Remove(long lMapIndex);

		void OnKill(LPCHARACTER pkMonster, LPCHARACTER pKiller);
		void OnKillPilar(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkPilar);
		void OnKillBlackSmith(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkBlackSmith);
		void OnStatueSetRotation(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkStatue);

		void SetDungeonStep(LPCHARACTER pkChar);
#ifdef ENABLE_YOHARA_SYSTEM
		int GetSungmaQueenDungeonValue(LPCHARACTER pkChar, int iPoint);
#endif
		void QueenDebuffAttack(LPCHARACTER pkAttacker, LPCHARACTER pkVictim);
		void LeaveParty(long lMapIndex);
		void Leave(LPCHARACTER pChar);
		bool Access(LPCHARACTER pChar);
		void Start(LPCHARACTER pkChar);

		//Set
		void SetSubXYZ(long lX, long lY, long lZ) noexcept { lSubMapPos.x = lX, lSubMapPos.y = lY, lSubMapPos.z = lZ; }
		void SetXYZ(long lX, long lY, long lZ) noexcept { lMapCenterPos.x = lX, lMapCenterPos.y = lY, lMapCenterPos.z = lZ; }

		//Get
		const PIXEL_POSITION& GetSubXYZ() const noexcept { return lSubMapPos; }
		const PIXEL_POSITION& GetXYZ() const noexcept { return lMapCenterPos; }

	private:
		std::map<long, CSnkMap*> m_dwRegGroups;
		PIXEL_POSITION lMapCenterPos, lSubMapPos;
	};
};
#endif
