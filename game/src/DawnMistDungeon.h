#pragma once
#include "stdafx.h"

#include "entity.h"
#include "char_manager.h"
#include "char.h"
#include "utils.h"
#include "mob_manager.h"
#include "config.h"
#include "party.h"

#ifdef ENABLE_DAWNMIST_DUNGEON
class CDawnMistDungeon : public singleton<CDawnMistDungeon> {
public:
	enum EMainConfig {
		TEMPLE_BOSS					= 6392, // Mob that spawns the TEMPLE_GUARDIAN when killed
		TEMPLE_GUARDIAN				= 6405, // Mob that spawns the TEMPLE_PORTAL when killed
		TEMPLE_GUARDIAN_MOVE_TIME	= 60 * 3, // TEMPLE_GUARDIAN's changing position time (3 minutes)
		TEMPLE_GUARDIAN_DEFEAT_TIME	= 60 * 10, // TEMPLE_GUARDIAN's time limit to kill it (10 minutes)
		TEMPLE_PORTAL				= 20415, // Portal's vnum
		TEMPLE_PORTAL_ALIVE_TIME	= 60 * 1, // TEMPLE_PORTAL's will disappear after 1 minute

		FOREST_DUNGEON_BOSS			= 6192, // Dungeon's boss ('Jotun Thrym')
		FOREST_DUNGEON_HEALER		= 6409, // Dungeon boss' healer mob ('Ochao Healer')
	};

	/*
	When FOREST_DUNGEON_BOSS' HP reachs the following
	percentages, it'll spawn the healers.
	*/
	enum EHealerSpawnWavesData {
		FIRST_WAVE_PERC		= 75,
		SECOND_WAVE_PERC	= 50,
		THIRD_WAVE_PERC		= 25,
		FOURTH_WAVE_PERC	= 10,
		WAVE_COUNT			= 4,
	};

	enum EBossSpecialSkillData {
		SPECIAL_SKILL_INDEX		= 2, // Do not change this unless you know what it does.
		SPECIAL_SKILL_VNUM		= 306, // FOREST_DUNGEON_BOSS' special skill (thunder).
	};

	enum EBossSpecialSkillTypes {
		SPECIAL_SKILL_BLUE,
		SPECIAL_SKILL_GREEN,
		SPECIAL_SKILL_RED,
		SPECIAL_SKILL_YELLOW,
#ifdef ENABLE_WOLFMAN_CHARACTER
		SPECIAL_SKILL_PURPLE,
#endif
		SPECIAL_SKILL_BLACK,
		SPECIAL_SKILL_MAX_NUM,
	};

	struct FHeal
	{
		int arg;
		FHeal(int empty) noexcept :
			arg(empty)
		{}

		void operator() (LPENTITY entity)
		{
			if (entity && !entity->IsType(ENTITY_CHARACTER))
				return;

			LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(entity);
			if (!pChar)
				return;

			if (!pChar->IsMonster() || pChar->IsDead())
				return;

			pChar->PointChange(POINT_HP, MAX(1, (pChar->GetMaxHP() * pChar->GetMobTable().bRegenPercent) / 100));
			pChar->EffectPacket(SE_HEAL);
		}
	};

	void SpawnTempleGuardian();
	void SpawnTemplePortal(LPCHARACTER pGuardian, int iX, int iY);

	bool IsInDungeonInstance(long lMapIndex);
	bool IsForestDungeonBoss(LPCHARACTER pChar);
	bool IsHealer(LPCHARACTER pChar);

	bool CanSpawnHealerGroup(LPCHARACTER pBossChar);
	void SpawnHealerGroup(LPCHARACTER pBossChar);

	uint8_t GenerateBossSpecialSkillIndex(LPCHARACTER pBossChar);
	int GetDamageIncreaserValue(LPCHARACTER pChar, int iDam, int iJob);
};
#endif
