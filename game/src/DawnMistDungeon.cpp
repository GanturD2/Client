#include "stdafx.h"

#ifdef ENABLE_DAWNMIST_DUNGEON
#include "DawnMistDungeon.h"

EVENTINFO(TempleGuardianMove_info)
{
	LPCHARACTER pChar;
};

EVENTFUNC(TempleGuardianMove_event)
{
	TempleGuardianMove_info* info = dynamic_cast<TempleGuardianMove_info*>(event->info);

	if (!info || !info->pChar || info->pChar->IsDead())
		return 0;

	int iLastAttack = get_global_time() - info->pChar->GetLastAttackedTime();
	int iBattleStartTime = get_global_time() - info->pChar->GetBattleStartTime();

	if (iLastAttack < CDawnMistDungeon::TEMPLE_GUARDIAN_MOVE_TIME && iBattleStartTime < CDawnMistDungeon::TEMPLE_GUARDIAN_DEFEAT_TIME)
		return PASSES_PER_SEC(5);

	M2_DESTROY_CHARACTER(info->pChar);
	CDawnMistDungeon::Instance().SpawnTempleGuardian();
	return 0;
};

EVENTINFO(TemplePortalPurge_info)
{
	LPCHARACTER pChar;
};

EVENTFUNC(TemplePortalPurge_event)
{
	TemplePortalPurge_info* info = dynamic_cast<TemplePortalPurge_info*>(event->info);

	if (!info || !info->pChar)
		return 0;

	M2_DESTROY_CHARACTER(info->pChar);
	return 0;
};

EVENTINFO(HealerHeal_info)
{
	LPCHARACTER pChar;
};

EVENTFUNC(HealerHeal_event)
{
	HealerHeal_info* info = dynamic_cast<HealerHeal_info*>(event->info);

	if (!info || !info->pChar || info->pChar->IsDead() || info->pChar->IsStun())
		return 0;

	LPPARTY pParty = info->pChar->GetParty();

	if (!pParty)
		return 0;

	long lMapIndex = info->pChar->GetMapIndex();

	if (!CDawnMistDungeon::Instance().IsInDungeonInstance(lMapIndex))
		return 0;

	CDawnMistDungeon::FHeal f(0);
	pParty->ForEachMemberPtr(f);

	return PASSES_PER_SEC(15);
};

void CDawnMistDungeon::SpawnTempleGuardian()
{
	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().SpawnMobRandomPosition(TEMPLE_GUARDIAN, MAP_MT_THUNDER_DUNGEON);

	if (!pChar)
		return;

	pChar->SetBattleStartTime(0);

	TempleGuardianMove_info* info = AllocEventInfo<TempleGuardianMove_info>();
	info->pChar = pChar;
	pChar->m_pkTempleGuardianMoveEvent = event_create(TempleGuardianMove_event, info, PASSES_PER_SEC(TEMPLE_GUARDIAN_MOVE_TIME));
}

void CDawnMistDungeon::SpawnTemplePortal(LPCHARACTER pGuardian, int iX, int iY)
{
	LPCHARACTER pChar = CHARACTER_MANAGER::Instance().SpawnMob(TEMPLE_PORTAL, MAP_MT_THUNDER_DUNGEON, iX, iY, 0, false, 0, true);

	if (!pChar)
		return;

	// Clear the event
	if (pGuardian->m_pkTempleGuardianMoveEvent){
		event_cancel(&pGuardian->m_pkTempleGuardianMoveEvent);
		pGuardian->m_pkTempleGuardianMoveEvent = nullptr;
	}

	TemplePortalPurge_info* info = AllocEventInfo<TemplePortalPurge_info>();
	info->pChar = pChar;
	event_create(TemplePortalPurge_event, info, PASSES_PER_SEC(TEMPLE_PORTAL_ALIVE_TIME));
}

bool CDawnMistDungeon::IsInDungeonInstance(long lMapIndex)
{
	return lMapIndex >= MAP_DAWNMISTWOOD_DUNGEON * 10000 && lMapIndex < (MAP_DAWNMISTWOOD_DUNGEON + 1) * 10000;
}

bool CDawnMistDungeon::IsForestDungeonBoss(LPCHARACTER pChar)
{
	return IsInDungeonInstance(pChar->GetMapIndex()) && pChar->GetRaceNum() == FOREST_DUNGEON_BOSS;
}

bool CDawnMistDungeon::IsHealer(LPCHARACTER pChar)
{
	return pChar->IsHealer() && pChar->GetRaceNum() == FOREST_DUNGEON_HEALER && IsInDungeonInstance(pChar->GetMapIndex());
}

bool CDawnMistDungeon::CanSpawnHealerGroup(LPCHARACTER pBossChar)
{
	int iPerc = pBossChar->GetHP() * 100 / pBossChar->GetMaxHP();
	int iSpawnStep = pBossChar->GetMaxSP();
	int iPercByStep[WAVE_COUNT] = {FIRST_WAVE_PERC, SECOND_WAVE_PERC, THIRD_WAVE_PERC, FOURTH_WAVE_PERC};

	return iPerc <= iPercByStep[iSpawnStep];
}

void CDawnMistDungeon::SpawnHealerGroup(LPCHARACTER pBossChar)
{
	LPPARTY pParty = pBossChar->GetParty();

	if (!pParty)
		pParty = CPartyManager::Instance().CreateParty(pBossChar);

	for (int i = 0; i < 4; ++i){
		LPCHARACTER pHealer = CHARACTER_MANAGER::Instance().SpawnMobRange(FOREST_DUNGEON_HEALER, 
			pBossChar->GetMapIndex(), 
			pBossChar->GetX() - 600, pBossChar->GetY() - 600, 
			pBossChar->GetX() + 600, pBossChar->GetY() + 600, 
			true, true);

		if (!pHealer)
			continue;

		pParty->Join(pHealer->GetVID());
		pParty->Link(pHealer);

		// Start heal event
		HealerHeal_info* info = AllocEventInfo<HealerHeal_info>();
		info->pChar = pHealer;
		pHealer->m_pkHealerEvent = event_create(HealerHeal_event, info, PASSES_PER_SEC(5));
	}

	pBossChar->SetMaxSP(pBossChar->GetMaxSP() + 1);
}

uint8_t CDawnMistDungeon::GenerateBossSpecialSkillIndex(LPCHARACTER pBossChar)
{
	pBossChar->SetSpecialSkillIndex(number(SPECIAL_SKILL_BLUE, SPECIAL_SKILL_BLACK));
	return pBossChar->GetSpecialSkillIndex();
}

int CDawnMistDungeon::GetDamageIncreaserValue(LPCHARACTER pChar, int iDam, int iJob)
{
	uint8_t bySpecialSkillIndex = pChar->GetSpecialSkillIndex();

#ifdef ENABLE_WOLFMAN_CHARACTER
	float fDamageIncArray[JOB_MAX_NUM][SPECIAL_SKILL_MAX_NUM] = {
	//	BLUE   GREEN 	RED   YELLOW   PURPLE	BLACK
		{0.0,	0.0,	0.2,	0.0,	0.0,	0.2}, // JOB_WARRIOR
		{0.0,	0.0,	0.0,	0.15,	0.0,	0.2}, // JOB_ASSASSIN
		{0.0,	0.2,	0.0,	0.0,	0.0,	0.2}, // JOB_SURA
		{0.1,	0.0,	0.0,	0.0,	0.0,	0.2}, // JOB_SHAMAN
		{0.0,	0.0,	0.0,	0.0,	0.2,	0.2}, // JOB_WOLFMAN
	};
#else
	float fDamageIncArray[JOB_MAX_NUM][SPECIAL_SKILL_MAX_NUM] = {
	//	BLUE   GREEN 	RED   YELLOW   BLACK
		{0.0,	0.0,	0.2,	0.0,	0.2}, // JOB_WARRIOR
		{0.0,	0.0,	0.0,	0.15,	0.2}, // JOB_ASSASSIN
		{0.0,	0.2,	0.0,	0.0,	0.2}, // JOB_SURA
		{0.1,	0.0,	0.0,	0.0,	0.2}, // JOB_SHAMAN
	};
#endif

	return iDam * fDamageIncArray[iJob][bySpecialSkillIndex];
}
#endif
