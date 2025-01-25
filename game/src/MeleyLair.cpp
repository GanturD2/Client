#include "stdafx.h"
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#include "MeleyLair.h"
#include "db.h"
#include "log.h"
#include "item.h"
#include "char.h"
#include "utils.h"
#include "party.h"
#include "regen.h"
#include "config.h"
#include "packet.h"
#include "item_manager.h"
#include "start_position.h"
#include "desc.h"
#include "questmanager.h"
#include "cmd.h"
#ifdef MELEY_GOLD_DROP
#	include "unique_item.h"
#endif

int stoneSpawnPos[][2] =
{
	{140, 131},
	{130, 122},
	{121, 128},
	{128, 140}
};

int monsterSpawnPos[][2] =
{
	{140, 131}, {130, 122}, {121, 128}, {128, 140},
	{128, 142}, {128, 131}, {135, 130}, {141, 126},
	{128, 122}, {117, 127}, {118, 136}, {126, 142}
};

#ifdef MELEY_REWARD_ITEM_DROP
int rewardItemList[] =
{
	10, 11, 12, 13, 14, 15, 16,
	17, 18
};
#endif

struct FNotice
{
	FNotice(const char* psz) noexcept : m_psz(psz) {}
	void operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChar = (LPCHARACTER)ent;
			if (pkChar && pkChar->IsPC())
			{
				pkChar->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_psz);
			}
		}
	}

	const char* m_psz;
};

struct FExitAndGoTo
{
	FExitAndGoTo() noexcept {};
	void operator()(LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChar = dynamic_cast<LPCHARACTER>(ent);
			if (pkChar && pkChar->IsPC())
			{
#ifdef MELEY_FORCE_WARPOUT
				pkChar->WarpSet(MELEY_WARP_OUT_X, MELEY_WARP_OUT_Y, MELEY_SUBMAP_INDEX);
#else
				const PIXEL_POSITION posSub = CMeleyLairManager::Instance().GetSubXYZ();
				if (!posSub.x)
					pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
				else
					pkChar->WarpSet(posSub.x, posSub.y);
#endif
			}
		}
	}
};

struct FKillMonsterSectree
{
	void operator () (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (!ch)
				return;

			if (ch->GetRaceNum() == MELEY_BOSS_VNUM)
				return;

			if (ch->IsMonster() || ch->IsDoor() || ch->IsStone())
				ch->DeadNoReward();
		}
	}
};

struct FKillFinalMonsterSectree
{
	void operator () (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (!ch)
				return;

			if (ch->GetRaceNum() == MELEY_BOSS_VNUM)
				return;

			if (ch->IsMonster())
				ch->DeadNoReward();
		}
	}
};

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
struct FMeleyLairTimeResult
{
	FMeleyLairTimeResult(uint8_t bType) noexcept : m_bType(bType) {}
	void operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			const auto& pkChar = dynamic_cast<LPCHARACTER>(ent);
			if (pkChar && pkChar->IsPC())
			{
				TPacketMeleyLairTimeResult packet{};
				packet.bHeader = HEADER_GC_MELEY_LAIR_TIME_RESULT;
				packet.bResult = m_bType;
				pkChar->GetDesc()->Packet(&packet, sizeof(packet));
			}
		}
	}

	uint8_t m_bType;
};
#endif

struct FMeleyLairTimeInfo
{
	FMeleyLairTimeInfo(uint32_t dwTime) noexcept : m_dwTime(dwTime) {}
	void operator() (LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			const auto& pkChar = dynamic_cast<LPCHARACTER>(ent);
			if (pkChar && pkChar->IsPC())
			{
				TPacketMeleyLairTimeInfo packet{};
				packet.bHeader = HEADER_GC_MELEY_LAIR_TIME_INFO;
				packet.dwTime = m_dwTime;
				pkChar->GetDesc()->Packet(&packet, sizeof(packet));
			}
		}
	}

	uint32_t m_dwTime;
};

EVENTINFO(r_meleystatues_info)
{
	bool bFirst;
	CMeleyLair* pMap;
};

EVENTFUNC(r_meleystatues_event)
{
	r_meleystatues_info* pEventInfo = dynamic_cast<r_meleystatues_info*>(event->info);
	if (pEventInfo)
	{
		if (pEventInfo->bFirst)
		{
			CMeleyLair* pMap = pEventInfo->pMap;
			if (pMap)
				pMap->DungeonResult();
		}
		else
		{
			pEventInfo->bFirst = true;
			return PASSES_PER_SEC(MELEY_TIME_LIMIT_TO_KILL_STATUE);
		}
	}

	return 0;
}

EVENTINFO(r_meleylimit_info)
{
	bool bWarp;
	CMeleyLair* pMap;
};

EVENTFUNC(r_meleylimit_event)
{
	r_meleylimit_info* pEventInfo = dynamic_cast<r_meleylimit_info*>(event->info);
	if (pEventInfo)
	{
		CMeleyLair* pMap = pEventInfo->pMap;
		if (pMap)
		{
			if (pEventInfo->bWarp)
				pMap->EndDungeonWarp();
			else
				pMap->EndDungeon(false);
		}
	}

	return 0;
}

EVENTINFO(r_meleyspawn_info)
{
	r_meleyspawn_info() = default;
	CMeleyLair* pMap{ nullptr };
	bool bFirst{ false };
	uint16_t bStep{ 0 };
	uint32_t dwTimeReload{ 0 };
	uint32_t dwMobVnum{ 0 };
	uint32_t dwMobCount{ 0 };
};

EVENTFUNC(r_meleyspawn_event)
{
	r_meleyspawn_info* pEventInfo = dynamic_cast<r_meleyspawn_info*>(event->info);
	if (!pEventInfo)
		return 0;

	auto pMap = pEventInfo->pMap;
	if (!pMap)
		return 0;

	if (!pMap->GetMapSectree())
		return 0;

	const uint32_t dwTimeReload = pEventInfo->dwTimeReload;
	const uint32_t dwMobVnum = pEventInfo->dwMobVnum;
	const uint32_t dwMobCount = pEventInfo->dwMobCount;
	const size_t mob_count = SECTREE_MANAGER::Instance().GetMonsterCountInMap(pMap->GetMapIndex(), dwMobVnum, true);
	const bool bRespawnCommon = dwMobCount > mob_count ? true : false;
	if (bRespawnCommon)
	{
		const uint32_t dwDiff = dwMobCount - mob_count;
		for (uint32_t i = 0; i < dwDiff; ++i)
		{
			const int posX = monsterSpawnPos[i][0];
			const int posY = monsterSpawnPos[i][1] + number(3, 10);
			pMap->Spawn(dwMobVnum, posX, posY, 0);
		}
	}

	if ((pEventInfo->bStep == 2) || (pEventInfo->bStep == 3))
	{
		uint32_t dwStoneVnum = MELEY_GUILD_MOBVNUM_RESPAWN_STONE_STEP2;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		if (pMap->GetDungeonType() == eType::MELEY_TYPE_PARTY_RED_DRAGON_LAIR)
			dwStoneVnum = MELEY_PARTY_MOBVNUM_RESPAWN_STONE_STEP2;
#endif
		const auto stones_count = SECTREE_MANAGER::Instance().GetMonsterCountInMap(pMap->GetMapIndex(), dwStoneVnum, true);
		if (stones_count == 0)
		{
			if (pEventInfo->bStep == 2)
			{
				if (!pMap->GetGateChar())
				{
					auto oldMainNPC = pMap->GetMainNPC();
					if (oldMainNPC)
						M2_DESTROY_CHARACTER(oldMainNPC);

					auto pkGate = pMap->Spawn(MELEY_GATE_VNUM, 129, 175, 5);
					if (pkGate)
						pMap->SetGateChar(pkGate);

					auto newMainNPC = pMap->Spawn(MELEY_NPC_VNUM, 134, 168, 0);
					if (newMainNPC)
						pMap->SetMainNPC(newMainNPC);
				}
			}

			const uint32_t dwTime = get_global_time();
			if (dwTime >= pMap->GetLastStoneKilledTime())
			{
				for (const auto& stonePos : stoneSpawnPos)
				{
					const int random_pos = number(3, 10);
					pMap->Spawn(dwStoneVnum, stonePos[0], stonePos[1] + random_pos, 0);
				}

				// Need optimization
				if (pEventInfo->bStep == 3)
				{
					if (pMap->GetStatue1Char() && !pMap->GetStatue1Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2))
						pMap->GetStatue1Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					if (pMap->GetStatue2Char() && !pMap->GetStatue2Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2))
						pMap->GetStatue2Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					if (pMap->GetStatue3Char() && !pMap->GetStatue3Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2))
						pMap->GetStatue3Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					if (pMap->GetStatue4Char() && !pMap->GetStatue4Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2))
						pMap->GetStatue4Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					pMap->SetKillCountStones(0);
				}
				else
				{
					if (pMap->GetStatue1Char() && !pMap->GetStatue1Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pMap->GetStatue1Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					if (pMap->GetStatue2Char() && !pMap->GetStatue2Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pMap->GetStatue2Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					if (pMap->GetStatue3Char() && !pMap->GetStatue3Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pMap->GetStatue3Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

					if (pMap->GetStatue4Char() && !pMap->GetStatue4Char()->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pMap->GetStatue4Char()->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);
				}
			}
		}

		if ((pEventInfo->bStep == 3) && (pEventInfo->bFirst))
		{
			for (const auto& stonePos : stoneSpawnPos)
			{
				const auto random_pos = static_cast<uint16_t>(number(3, 10));
				const auto posX = static_cast<uint16_t>(stonePos[0]);
				const auto posY = static_cast<uint16_t>(stonePos[1] + random_pos);
				uint32_t dwBossVnum = MELEY_GUILD_MOBVNUM_RESPAWN_BOSS_STEP3;

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
				if (pMap->GetDungeonType() == eType::MELEY_TYPE_PARTY_RED_DRAGON_LAIR)
					dwBossVnum = MELEY_PARTY_MOBVNUM_RESPAWN_BOSS_STEP3;
#endif
				pMap->Spawn(dwBossVnum, posX, posY, 0);
				pMap->Spawn(MELEY_MOBVNUM_RESPAWN_SUBBOSS_STEP3, posX, posY, 0);
				pMap->Spawn(MELEY_MOBVNUM_RESPAWN_SUBBOSS_STEP3, posX, posY, 0);
			}

			pEventInfo->bFirst = false;
		}
	}

	return PASSES_PER_SEC(dwTimeReload);
}

EVENTINFO(r_meleyeffect_info)
{
	r_meleyeffect_info() = default;
	CMeleyLair* pMap{ nullptr };
	uint16_t bStep{ 0 };
	uint16_t bEffectStep{ 0 };
};

EVENTFUNC(r_meleyeffect_event)
{
	auto* pEventInfo = dynamic_cast<r_meleyeffect_info*>(event->info);
	if (!pEventInfo)
		return 0;

	auto pMap = pEventInfo->pMap;
	if (!pMap)
		return 0;

	if (!pMap->GetBossChar() || !pMap->GetStatue1Char() || !pMap->GetStatue2Char() || !pMap->GetStatue3Char() || !pMap->GetStatue4Char())
		return 0;

	const auto bStep = pEventInfo->bStep;
	const auto bEffectStep = pEventInfo->bEffectStep;
	if (bStep == 1)
	{
		if (bEffectStep == 1)
		{
			// LASER_EFFECT
			const uint32_t timeNow = get_dword_time();

			if (pMap->GetMapSectree())
			{
				PIXEL_POSITION pos = CMeleyLairManager::Instance().GetXYZ();
				pos.x = pMap->GetMapSectree()->m_setting.iBaseX + 130 * 100;
				pos.y = pMap->GetMapSectree()->m_setting.iBaseY + 77 * 100;
				pos.z = 0;
				pMap->GetStatue1Char()->SetRotationToXY(pos.x, pos.y);
				pMap->GetStatue2Char()->SetRotationToXY(pos.x, pos.y);
				pMap->GetStatue3Char()->SetRotationToXY(pos.x, pos.y);
				pMap->GetStatue4Char()->SetRotationToXY(pos.x, pos.y);
			}

			pMap->GetStatue1Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue1Char()->GetX(), pMap->GetStatue1Char()->GetY(), 0, timeNow);
			pMap->GetStatue2Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue2Char()->GetX(), pMap->GetStatue2Char()->GetY(), 0, timeNow);
			pMap->GetStatue3Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue3Char()->GetX(), pMap->GetStatue3Char()->GetY(), 0, timeNow);
			pMap->GetStatue4Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue4Char()->GetX(), pMap->GetStatue4Char()->GetY(), 0, timeNow);

			// END_OF_LASER_EFFECT

			pEventInfo->bEffectStep = 2;
		}
		else
		{
			pMap->StartDungeonStep(2);
			return 0;
		}
	}
	else if (bStep == 2)
	{
		if (bEffectStep == 1)
		{
			// LASER_EFFECT
			const uint32_t timeNow = get_dword_time();

			if (pMap->GetMapSectree())
			{
				PIXEL_POSITION pos = CMeleyLairManager::Instance().GetXYZ();
				pos.x = pMap->GetMapSectree()->m_setting.iBaseX + 130 * 100;
				pos.y = pMap->GetMapSectree()->m_setting.iBaseY + 77 * 100;
				pos.z = 0;
				pMap->GetStatue1Char()->SetRotationToXY(pos.x, pos.y);
				pMap->GetStatue2Char()->SetRotationToXY(pos.x, pos.y);
				pMap->GetStatue3Char()->SetRotationToXY(pos.x, pos.y);
				pMap->GetStatue4Char()->SetRotationToXY(pos.x, pos.y);
			}

			pMap->GetStatue1Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue1Char()->GetX(), pMap->GetStatue1Char()->GetY(), 0, timeNow);
			pMap->GetStatue2Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue2Char()->GetX(), pMap->GetStatue2Char()->GetY(), 0, timeNow);
			pMap->GetStatue3Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue3Char()->GetX(), pMap->GetStatue3Char()->GetY(), 0, timeNow);
			pMap->GetStatue4Char()->SendMovePacket(FUNC_MOB_SKILL, 0, pMap->GetStatue4Char()->GetX(), pMap->GetStatue4Char()->GetY(), 0, timeNow);
			// END_OF_LASER_EFFECT

			pEventInfo->bEffectStep = 2;
		}
		else
		{
			pMap->StartDungeonStep(3);
			return 0;
		}
	}

	return PASSES_PER_SEC(5);
}

CMeleyLair::CMeleyLair(long lMapIndex)
{
	p_pGuild = nullptr;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	p_Party = nullptr;
#endif
	e_pEndEvent = nullptr;
	e_pWarpEvent = nullptr;
	e_SpawnEvent = nullptr;
	e_SEffectEvent = nullptr;
	e_DestroyStatues = nullptr;
	vPlayersStatue.clear();
	vPlayersReward.clear();
	vPlayersOut.clear();
	dungeon_type = 0;
	map_index = lMapIndex;
	dungeon_step = 0;
	reward = 0;
	pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(lMapIndex);
	pkMainNPC = nullptr;
	pkGate = nullptr;
	pkBoss = nullptr;
	pkStatue1 = nullptr;
	pkStatue2 = nullptr;
	pkStatue3 = nullptr;
	pkStatue4 = nullptr;
	ParticipantsName.clear();
	dwFirstRankingTime = 0;
	time_start = 0;
	bIsDungeonCompleted = false;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	p_szLeaderName.clear();
	bIsDungeonTimeSuccess = false;
#endif
	bRemainMessage = false;
}

CMeleyLair::~CMeleyLair()
{
	if (e_pEndEvent)
		event_cancel(&e_pEndEvent);

	if (e_pWarpEvent)
		event_cancel(&e_pWarpEvent);

	if (e_SpawnEvent)
		event_cancel(&e_SpawnEvent);

	if (e_SEffectEvent)
		event_cancel(&e_SEffectEvent);

	if (e_DestroyStatues)
		event_cancel(&e_DestroyStatues);
}

void CMeleyLair::Destroy()
{
	if (e_pEndEvent)
		event_cancel(&e_pEndEvent);

	if (e_pWarpEvent)
		event_cancel(&e_pWarpEvent);

	if (e_SpawnEvent)
		event_cancel(&e_SpawnEvent);

	if (e_SEffectEvent)
		event_cancel(&e_SEffectEvent);

	if (e_DestroyStatues)
		event_cancel(&e_DestroyStatues);

	//pkDungeon = nullptr;
	p_pGuild = nullptr;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	p_Party = nullptr;
#endif
	e_pEndEvent = nullptr;
	e_pWarpEvent = nullptr;
	e_SpawnEvent = nullptr;
	e_SEffectEvent = nullptr;
	e_DestroyStatues = nullptr;
	vPlayersStatue.clear();
	vPlayersReward.clear();
	vPlayersOut.clear();
	dungeon_type = 0;
	map_index = 0;
	dungeon_step = 0;
	reward = 0;
	pkSectreeMap = nullptr;
	pkMainNPC = nullptr;
	pkGate = nullptr;
	pkBoss = nullptr;
	pkStatue1 = nullptr;
	pkStatue2 = nullptr;
	pkStatue3 = nullptr;
	pkStatue4 = nullptr;
	ParticipantsName.clear();
	dwFirstRankingTime = 0;
	time_start = 0;
	bIsDungeonCompleted = false;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	p_szLeaderName.clear();
	bIsDungeonTimeSuccess = false;
#endif
}

void CMeleyLair::SendMeleyNotice(const char* szNotice)
{
	char szBuf[512]{};
	snprintf(szBuf, sizeof(szBuf), szNotice);
	FNotice fNotice(szBuf);
	GetMapSectree()->for_each(fNotice);
}

void CMeleyLair::RegisterParticipant(const LPCHARACTER& pkChar)
{
	if (!pkChar)
		return;

	const auto dwPlayerID = pkChar->GetPlayerID();
	const auto szPlayerName = pkChar->GetName();

	const auto it = ParticipantsName.find(dwPlayerID);
	if (it == ParticipantsName.end())
		ParticipantsName.insert(it, std::pair<uint32_t, std::string>(dwPlayerID, szPlayerName));
}

bool CMeleyLair::IsParticipantRegistered(uint32_t dwPlayerID)
{
	const auto it = ParticipantsName.find(dwPlayerID);
	if (it != ParticipantsName.end())
		return true;

	return false;
}

LPCHARACTER CMeleyLair::Spawn(uint32_t dwVnum, int iX, int iY, int iDir, bool bSpawnMotion)
{
	if (!GetMapSectree())
		return nullptr;

	auto pkMob = CHARACTER_MANAGER::Instance().SpawnMob(dwVnum, GetMapIndex(),
		GetMapSectree()->m_setting.iBaseX + iX * 100,
		GetMapSectree()->m_setting.iBaseY + iY * 100,
		0, bSpawnMotion, iDir == 0 ? -1 : (iDir - 1) * 45);

	if (pkMob)
	{
		pkMob->SetMeleyLair(this);
		sys_log(0, "<MeleyLair> Spawn: %s (map index: %d).", pkMob->GetName(), GetMapIndex());
	}

	return pkMob;
}

void CMeleyLair::SetDungeonStep(uint16_t bStep)
{
	if (!GetMapSectree())
	{
		EndDungeon(true);
		return;
	}

	if (!pkBoss || !pkStatue1 || !pkStatue2 || !pkStatue3 || !pkStatue4)
	{
		EndDungeon(true);
		return;
	}

	if (bStep == dungeon_step)
		return;

	dungeon_step = bStep;

	// Clear Spawn Event
	if (e_SpawnEvent)
		event_cancel(&e_SpawnEvent);

	e_SpawnEvent = nullptr;

	// Check Steps
	if (bStep == 1)
	{
		SetDungeonTimeStart(get_global_time());
		SetKillCountStones(0);
		auto* pEventInfo = AllocEventInfo<r_meleyspawn_info>();
		pEventInfo->pMap = this;
		pEventInfo->bFirst = false;
		pEventInfo->bStep = bStep;
		pEventInfo->dwTimeReload = MELEY_TIME_RESPAWN_COMMON_STEP1;

		pEventInfo->dwMobVnum = MELEY_GUILD_MOBVNUM_RESPAWN_COMMON_STEP1;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		if (GetDungeonType() == eType::MELEY_TYPE_PARTY_RED_DRAGON_LAIR)
			pEventInfo->dwMobVnum = MELEY_PARTY_MOBVNUM_RESPAWN_COMMON_STEP1;
#endif

		pEventInfo->dwMobCount = MELEY_MOBCOUNT_RESPAWN_COMMON_STEP;
		e_SpawnEvent = event_create(r_meleyspawn_event, pEventInfo, PASSES_PER_SEC(1));
	}
	else if (bStep == 2)
	{
		if (e_SEffectEvent)
			event_cancel(&e_SEffectEvent);

		e_SEffectEvent = nullptr;

		auto* pEventInfo = AllocEventInfo<r_meleyeffect_info>();
		pEventInfo->pMap = this;
		pEventInfo->bStep = 1;
		pEventInfo->bEffectStep = 1;
		e_SEffectEvent = event_create(r_meleyeffect_event, pEventInfo, PASSES_PER_SEC(5));
	}
	else if (bStep == 3)
	{
		if (e_SEffectEvent)
			event_cancel(&e_SEffectEvent);

		e_SEffectEvent = nullptr;

		auto* pEventInfo = AllocEventInfo<r_meleyeffect_info>();
		pEventInfo->pMap = this;
		pEventInfo->bStep = 2;
		pEventInfo->bEffectStep = 1;
		e_SEffectEvent = event_create(r_meleyeffect_event, pEventInfo, PASSES_PER_SEC(5));
	}
	else if (bStep == 4)
	{
		if (e_SEffectEvent)
			event_cancel(&e_SEffectEvent);

		if (e_DestroyStatues)
			event_cancel(&e_DestroyStatues);

		e_SEffectEvent = nullptr;
		e_DestroyStatues = nullptr;

		// Kill Final Monsters
		FKillFinalMonsterSectree fKillFinalMonsterSectree;
		GetMapSectree()->for_each(fKillFinalMonsterSectree);

		// Update Statue
		pkStatue1->SetArmada();
		pkStatue2->SetArmada();
		pkStatue3->SetArmada();
		pkStatue4->SetArmada();
	}
}

void CMeleyLair::StartDungeonStep(uint16_t bStep)
{
	// Clear Timers
	if (e_SpawnEvent)
		event_cancel(&e_SpawnEvent);

	if (e_SEffectEvent)
		event_cancel(&e_SEffectEvent);

	e_SpawnEvent = nullptr;
	e_SEffectEvent = nullptr;

	// Check Pointers
	if (!pkBoss || !pkStatue1 || !pkStatue2 || !pkStatue3 || !pkStatue4) // Test crashing meley
	{
		EndDungeon(true);
		return;
	}

	const auto bDungeonType = GetDungeonType();
	if (bStep == 2)
	{
		SetLastStoneKilledTime(0);
		auto* pEventInfo = AllocEventInfo<r_meleyspawn_info>();
		pEventInfo->pMap = this;
		pEventInfo->bFirst = false;
		pEventInfo->bStep = bStep;
		pEventInfo->dwTimeReload = MELEY_TIME_RESPAWN_COMMON_STEP2;

		pEventInfo->dwMobVnum = MELEY_GUILD_MOBVNUM_RESPAWN_COMMON_STEP2;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		if (bDungeonType == eType::MELEY_TYPE_PARTY_RED_DRAGON_LAIR)
			pEventInfo->dwMobVnum = MELEY_PARTY_MOBVNUM_RESPAWN_COMMON_STEP2;
#endif

		pEventInfo->dwMobCount = MELEY_MOBCOUNT_RESPAWN_COMMON_STEP;
		e_SpawnEvent = event_create(r_meleyspawn_event, pEventInfo, PASSES_PER_SEC(1));
	}
	else if (bStep == 3)
	{
		SetLastStoneKilledTime(0);
		SetKillCountStones(0);
		SetKillCountBosses(0);
		auto* pEventInfo = AllocEventInfo<r_meleyspawn_info>();
		pEventInfo->pMap = this;
		pEventInfo->bFirst = true;
		pEventInfo->bStep = bStep;
		pEventInfo->dwTimeReload = MELEY_TIME_RESPAWN_COMMON_STEP3;

		pEventInfo->dwMobVnum = MELEY_GUILD_MOBVNUM_RESPAWN_COMMON_STEP3;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		if (bDungeonType == eType::MELEY_TYPE_PARTY_RED_DRAGON_LAIR)
			pEventInfo->dwMobVnum = MELEY_PARTY_MOBVNUM_RESPAWN_COMMON_STEP3;
#endif

		pEventInfo->dwMobCount = MELEY_MOBCOUNT_RESPAWN_COMMON_STEP;
		e_SpawnEvent = event_create(r_meleyspawn_event, pEventInfo, PASSES_PER_SEC(1));
	}
}

void CMeleyLair::Start()
{
	if (!pkSectreeMap)
	{
		EndDungeon(true);
		return;
	}

	pkMainNPC = Spawn(MELEY_NPC_VNUM, 135, 179, 8);
	pkGate = Spawn(MELEY_GATE_VNUM, 129, 175, 5);
	pkBoss = Spawn(MELEY_BOSS_VNUM, 130, 77, 1);

	uint32_t dwStatueVnum = MELEY_GUILD_STATUE_VNUM;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	if (GetParty())
		dwStatueVnum = MELEY_PARTY_STATUE_VNUM;
#endif

	pkStatue1 = Spawn(dwStatueVnum, 123, 137, 5);
	pkStatue2 = Spawn(dwStatueVnum, 123, 124, 5);
	pkStatue3 = Spawn(dwStatueVnum, 136, 123, 5);
	pkStatue4 = Spawn(dwStatueVnum, 137, 137, 5);
}

void CMeleyLair::StartDungeon(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (GetDungeonStep() > 0)
		return;

	if (!IsParticipantRegistered(pkChar->GetPlayerID()))
		return;

	if (e_pEndEvent)
	{
		EndDungeon(true);
		return;
	}

	if (!pkMainNPC || !pkGate || !pkBoss || !pkStatue1 || !pkStatue2 || !pkStatue3 || !pkStatue4)
	{
		EndDungeon(true);
		return;
	}

	bool bCanStart = false;
	if (GetGuild())
	{
		const auto& pkGuild = GetGuild();
		if (!pkGuild)
			return;

		char szBuf1[512]{ 0 }, szBuf2[512]{ 0 };
		snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[43][Guild] The battle against Dragon Queen Meley is starting."));
		snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("[44][Guild] The gates to Meley's Lair are open. The time limit in the dungeon is one hour."));
		pkGuild->Chat(szBuf1);
		pkGuild->Chat(szBuf2);

		bCanStart = true;
	}
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	else if (GetParty())
	{
		char szBuf1[512]{ 0 }, szBuf2[512]{ 0 };

		snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[188][Group] The battle has begun."));
		snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("[189][Group] You have 1 hour to complete the dungeon."));

		SendNoticeMap(&szBuf1[0], GetMapIndex(), false);
		SendNoticeMap(&szBuf2[0], GetMapIndex(), false);

		bCanStart = true;
	}
#endif

	if (!bCanStart)
	{
		sys_err("could not start meley lair dungeon, guild and party null pointers.");
		return;
	}

	pkChar->SetQuestNPCID(0);
	pkGate->DeadNoReward();
	pkGate = nullptr;

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	// Start Party Ranking Time Cooldown
	if (GetMapSectree())
	{
		FMeleyLairTimeResult fMeleyLairTimeResult(0);
		GetMapSectree()->for_each(fMeleyLairTimeResult);
	}
#endif

	SetDungeonStep(1);
	if (e_pEndEvent)
		event_cancel(&e_pEndEvent);

	auto* pEventInfo = AllocEventInfo<r_meleylimit_info>();
	pEventInfo->pMap = this;
	pEventInfo->bWarp = false;
	e_pEndEvent = event_create(r_meleylimit_event, pEventInfo, PASSES_PER_SEC(MELEY_TIME_LIMIT_DUNGEON));
}

void CMeleyLair::EndDungeon(bool bSuccess, bool bIsCancelled)
{
	int iWarpTime = 0;
	int iReturn = 0;
	if (GetGuild())
	{
		auto pkGuild = GetGuild();
		if (pkGuild)
		{
			char szBuf1[512]{ 0 }, szBuf2[512]{ 0 };

			if (bSuccess)
			{
				if (IsDungeonCompleted())
				{
					char szParticipantsList[512]{};
					int iParticipantsLen = 0;
					const auto tElapsedTime = GetElapsedTime();

					for (auto it = ParticipantsName.begin(); it != ParticipantsName.end(); it++)
					{
						if (std::next(it) != ParticipantsName.end())
							iParticipantsLen += snprintf(szParticipantsList + iParticipantsLen, sizeof(szParticipantsList), "%s, ", it->second.c_str());
						else
							iParticipantsLen += snprintf(szParticipantsList + iParticipantsLen, sizeof(szParticipantsList), "%s", it->second.c_str());
					}

					LogManager::Instance().MeleyGuildLog(pkGuild->GetID(), &szParticipantsList[0], GetParticipantsCount(), tElapsedTime);

					const int iMinutes = (tElapsedTime / 60) % 60;
					const int iSeconds = tElapsedTime % 60;

					const uint32_t dwCoolDownTime = get_global_time() + MELEY_GUILD_COOLDOWN_DUNGEON + 5;
					pkGuild->SetDungeonCooldown(dwCoolDownTime);

					iWarpTime = MELEY_END_WARP_TIME;
					iReturn = MELEY_GUILD_LADDER_POINTS_COST;
					snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[45][Guild] You have defeated Meley the Dragon Queen!"));
					snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("[47][Guild] Required time: %d min. %d sec."), iMinutes, iSeconds);

#ifdef MELEY_NOTICE_ALL
					char szBufNoticeAll[256]{ 0 };
					snprintf(&szBufNoticeAll[0], sizeof(szBufNoticeAll), LC_TEXT("The %s Guild overcame evil by completing Meley's Lair."), pkGuild->GetName());
					BroadcastNotice(&szBufNoticeAll[0]);
#endif

				}
				else
				{
					iWarpTime = MELEY_EXPIRE_WARP_TIME;
					iReturn = MELEY_GUILD_LADDER_POINTS_COST;
					snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("<Guild> The dungeon can't be started, please contact our staff."));
					snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("<Guild> %d ladder points will be returned."), iReturn);
				}
			}
			else
			{
				const uint32_t dwCoolDownTime = get_global_time() + MELEY_GUILD_COOLDOWN_DUNGEON + 5;
				pkGuild->SetDungeonCooldown(dwCoolDownTime);
				SetDungeonStep(0);

				iWarpTime = MELEY_EXPIRE_WARP_TIME;
				iReturn = MELEY_GUILD_LADDER_POINTS_RETURN;
				if (bIsCancelled)
					snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[53][Guild] You have cancelled the fight in Meley's Lair."));
				else
					snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[49][Guild] Time's up."));

				snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("[50][Guild] The gates to the lair are closed once more."));
			}

			if (strlen(szBuf1) > 0)
			{
				pkGuild->Chat(szBuf1);
			}

			if (strlen(&szBuf1[0]) > 0)
			{
				pkGuild->Chat(&szBuf1[0]);
			}

			if (pkSectreeMap)
			{
				char szBuf[512]{ 0 };
				if (iWarpTime == MELEY_END_WARP_TIME)
					snprintf(&szBuf[0], sizeof(szBuf), LC_TEXT("[48][Guild] For your own safety, you will be teleported away from the nest in 10 minutes."));
				else
					snprintf(&szBuf[0], sizeof(szBuf), LC_TEXT("[51][Guild] For your own safety, you will be teleported away from the nest in 1 minute."));

				pkGuild->Chat(szBuf);
			}

			if (iReturn)
			{
				pkGuild->ChangeLadderPoint(+iReturn);
				pkGuild->SetLadderPoint(pkGuild->GetLadderPoint() + iReturn);
			}

			pkGuild->RequestDungeon(0, 0);
		}
	}
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	else if (GetParty())
	{
		if (bSuccess)
		{
			if (IsDungeonCompleted())
			{
				char szParticipantsList[512]{ 0 };
				int iParticipantsLen = 0;
				const time_t tElapsedTime = GetElapsedTime();

				for (auto it = ParticipantsName.begin(); it != ParticipantsName.end(); it++)
				{
					if (std::next(it) != ParticipantsName.end())
						iParticipantsLen += snprintf(&szParticipantsList[0] + iParticipantsLen, sizeof(szParticipantsList), "%s, ", it->second.c_str());
					else
						iParticipantsLen += snprintf(&szParticipantsList[0] + iParticipantsLen, sizeof(szParticipantsList), "%s", it->second.c_str());
				}

				LogManager::Instance().MeleyPartyLog(GetPartyLeaderName().c_str(), &szParticipantsList[0], GetParticipantsCount(), tElapsedTime);

				const int iMinutes = (tElapsedTime / 60) % 60;
				const int iSeconds = tElapsedTime % 60;

				iWarpTime = MELEY_END_WARP_TIME;

				char szBuf1[512]{ 0 }, szBuf2[512]{ 0 }, szBuf3[512]{ 0 };
				snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[190][Group] You have completed your task with distinction!"));
				snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("[191][Group] Time until teleportation: %d min. %d sec."), iMinutes, iSeconds);
				snprintf(&szBuf3[0], sizeof(szBuf3), LC_TEXT("[192][Group] You will be transported from the dungeon in 10 minutes."));

				SendNoticeMap(&szBuf1[0], GetMapIndex(), false);
				SendNoticeMap(&szBuf2[0], GetMapIndex(), false);
				SendNoticeMap(&szBuf3[0], GetMapIndex(), false);

#ifdef MELEY_NOTICE_ALL
				char szBufNoticeAll[256]{ 0 };
				snprintf(&szBufNoticeAll[0], sizeof(szBufNoticeAll), LC_TEXT("%s and his group defeated Meley, the queen of dragons."), GetPartyLeaderName().c_str());
				BroadcastNotice(&szBufNoticeAll[0]);
#endif

			}
			else
			{
				iWarpTime = MELEY_EXPIRE_WARP_TIME;
				char szBuf[512]{ 0 };
				snprintf(&szBuf[0], sizeof(szBuf), LC_TEXT("<Group> The dungeon can't be started, please contact our staff."));
				SendNoticeMap(&szBuf[0], GetMapIndex(), false);
			}
		}
		else
		{
			iWarpTime = MELEY_EXPIRE_WARP_TIME;

			char szBuf1[512]{ 0 }, szBuf2[512]{ 0 };
			if (bIsCancelled)
				snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[195][Group] You have conceded."));
			else
				snprintf(&szBuf1[0], sizeof(szBuf1), LC_TEXT("[201][Group] Your time in the dungeon is up."));

			snprintf(&szBuf2[0], sizeof(szBuf2), LC_TEXT("[196][Group] You will be teleported to the city in 1 minute."));

			SendNoticeMap(&szBuf1[0], GetMapIndex(), false);
			SendNoticeMap(&szBuf2[0], GetMapIndex(), false);
		}
	}
#endif

	if (GetMapSectree())
	{
		if (bIsCancelled)
		{
			// Set Time Dungeon to 1 second
			FMeleyLairTimeInfo fMeleyLairTimeInfo(1);
			pkSectreeMap->for_each(fMeleyLairTimeInfo);

			// Send the information to binary
			FMeleyLairTimeResult fMeleyLairTimeResult(0);
			pkSectreeMap->for_each(fMeleyLairTimeResult);

			// Clear spawn regen
			if (e_SpawnEvent)
				event_cancel(&e_SpawnEvent);
			e_SpawnEvent = nullptr;
		}
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		else
		{
			// Update Success 1st Ranking Time
			if (GetElapsedTime() < GetFirstRankingTime())
			{
				SetDungeonTimeSuccess();
				FMeleyLairTimeResult fMeleyLairTimeResult(1);
				pkSectreeMap->for_each(fMeleyLairTimeResult);
			}
		}
#endif

		// Kill Monsters
		FKillMonsterSectree fKillMonsterSectree;
		GetMapSectree()->for_each(fKillMonsterSectree);
	}

	if (!iWarpTime)
	{
		EndDungeonWarp();
		return;
	}

	if (e_pEndEvent)
		event_cancel(&e_pEndEvent);

	e_pEndEvent = nullptr;

	if (e_pWarpEvent)
		event_cancel(&e_pWarpEvent);

	auto* pEventInfo = AllocEventInfo<r_meleylimit_info>();
	pEventInfo->pMap = this;
	pEventInfo->bWarp = true;
	e_pWarpEvent = event_create(r_meleylimit_event, pEventInfo, PASSES_PER_SEC(iWarpTime));
}

void CMeleyLair::EndDungeonWarp()
{
	if (GetMapSectree())
	{
		FExitAndGoTo f;
		GetMapSectree()->for_each(f);
	}

	if (GetGuild())
		CMeleyLairManager::Instance().ClearGuild(GetGuild());
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	else if (GetParty())
		CMeleyLairManager::Instance().ClearParty(GetParty());
#endif

	CMeleyLairManager::Instance().DestroyPrivateMap(GetMapIndex());
	Destroy();
	M2_DELETE(this);
}

bool CMeleyLair::Damage(LPCHARACTER pkStatue)
{
	if (!pkStatue)
		return false;

	const auto bStep = GetDungeonStep();
	if (!bStep)
		return false;

	//if ((pkStatue != pkStatue1) && (pkStatue != pkStatue2) && (pkStatue != pkStatue3) && (pkStatue != pkStatue4))
	//	return false;

	if (pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1) ||
		pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2) ||
		pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		return false;

	int iHPCalc = 0;
	if (bStep == 1)
		iHPCalc = ((pkStatue->GetMaxHP() * 75) / 100);
	else if (bStep == 2)
		iHPCalc = ((pkStatue->GetMaxHP() * 50) / 100);
	else if (bStep == 3)
		iHPCalc = ((pkStatue->GetMaxHP() * 10) / 100);

	if (pkStatue->GetHP() <= iHPCalc)
	{
		const int iCalc = iHPCalc - pkStatue->GetHP();
		pkStatue->PointChange(POINT_HP, iCalc);
		if (bStep == 1)
		{
			if (!pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
				pkStatue->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

			const bool bNextStep = ((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) && (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) &&
				(pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) && (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))) ? true : false;
			if (bNextStep)
				SetDungeonStep(2);

			return false;
		}
		else if (bStep == 2)
		{
			if (!pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
				pkStatue->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_1, 3600, 0, true);

			const bool bNextStep = ((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) && (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) &&
				(pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) && (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))) ? true : false;
			if (bNextStep)
				SetDungeonStep(3);

			return false;
		}
		else if (bStep == 3)
		{
			if (!pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2))
				pkStatue->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_2, 3600, 0, true);

			const bool bNextStep = ((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) && (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) &&
				(pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) && (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2))) ? true : false;
			if (bNextStep)
				SetDungeonStep(4);

			return false;
		}
	}

	return true;
}

void CMeleyLair::OnKill(uint32_t dwVnum)
{
	const auto bStep = GetDungeonStep();
	if (!bStep || (!pkStatue1 || !pkStatue2 || !pkStatue3 || !pkStatue4))
		return;

	if ((bStep == 2) || (bStep == 3))
	{
		uint32_t dwStoneVnum = MELEY_GUILD_MOBVNUM_RESPAWN_STONE_STEP2;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		if (GetParty())
			dwStoneVnum = MELEY_PARTY_MOBVNUM_RESPAWN_STONE_STEP2;
#endif
		if (dwVnum == dwStoneVnum)
		{
			constexpr auto dwLimit = MELEY_MOBCOUNT_RESPAWN_STONE_STEP2 - 1;
			if (GetKillCountStones() < dwLimit)
			{
				SetKillCountStones(GetKillCountStones() + 1);
				return;
			}

			if (GetKillCountStones() >= dwLimit)
			{
				if (bStep == 2)
					SetKillCountStones(0);
				else
					SetKillCountStones(GetKillCountStones() + 1);

				const uint32_t dwRandomMin = number(2, 4);
				const uint32_t dwLastKilledTime = get_global_time() + (60 * dwRandomMin);
				SetLastStoneKilledTime(dwLastKilledTime);

				bool bDoAff = true;
				if (bStep == 3)
				{
					constexpr auto dwLimit2 = static_cast<uint32_t>(eDefault::MELEY_MOBCOUNT_RESPAWN_BOSS_STEP3);
					if (GetKillCountBosses() >= dwLimit2)
						SetKillCountStones(0);
					else
						bDoAff = false;
				}

				if (bDoAff)
				{
					if ((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) || ((bStep == 3) && (pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))))
						pkStatue1->RemoveAffect(AFFECT_STATUE);

					if ((pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) || ((bStep == 3) && (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))))
						pkStatue2->RemoveAffect(AFFECT_STATUE);

					if ((pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) || ((bStep == 3) && (pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))))
						pkStatue3->RemoveAffect(AFFECT_STATUE);

					if ((pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1)) || ((bStep == 3) && (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))))
						pkStatue4->RemoveAffect(AFFECT_STATUE);
				}
			}
		}
		else if ((dwVnum == MELEY_GUILD_MOBVNUM_RESPAWN_BOSS_STEP3) || (dwVnum == MELEY_PARTY_MOBVNUM_RESPAWN_BOSS_STEP3))
		{
			constexpr auto dwLimit = MELEY_MOBCOUNT_RESPAWN_BOSS_STEP3 - 1;
			if (GetKillCountBosses() < dwLimit)
			{
				SetKillCountBosses(GetKillCountBosses() + 1);
				return;
			}

			if (GetKillCountBosses() >= dwLimit)
			{
				SetKillCountBosses(GetKillCountBosses() + 1);

				constexpr auto dwLimit2 = MELEY_MOBCOUNT_RESPAWN_STONE_STEP2;

				if (GetKillCountStones() >= dwLimit2)
				{
					SetKillCountStones(0);

					if (pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pkStatue1->RemoveAffect(AFFECT_STATUE);

					if (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pkStatue2->RemoveAffect(AFFECT_STATUE);

					if (pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pkStatue3->RemoveAffect(AFFECT_STATUE);

					if (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_1))
						pkStatue4->RemoveAffect(AFFECT_STATUE);
				}
			}
		}
	}
}

void CMeleyLair::OnKillCommon(const LPCHARACTER& pkMonster)
{
	if (!pkMonster)
		return;

	const int iChance = number(1, 100);
	if (iChance >= 95)
	{
		auto pkItem = ITEM_MANAGER::Instance().CreateItem(MELEY_SEAL_VNUM_KILL_STATUE);
		if (!pkItem)
			return;

		PIXEL_POSITION mPos;
		mPos.x = pkMonster->GetX();
		mPos.y = pkMonster->GetY();

		pkItem->AddToGround(GetMapIndex(), mPos);
		pkItem->StartDestroyEvent();
	}
}

void CMeleyLair::OnKillStatue(LPITEM pkItem, const LPCHARACTER& pkChar, LPCHARACTER pkStatue)
{
	if (!pkItem || !pkChar || !pkStatue)
		return;

	const auto bStep = GetDungeonStep();
	if (bStep != 4)
		return;

	if (!pkBoss || !pkStatue1 || !pkStatue2 || !pkStatue3 || !pkStatue4)
		return;

	if (e_pWarpEvent)
		return;

	const bool bNextStep = (((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) || (pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3)))
		&& ((pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) || (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3)))
		&& ((pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) || (pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3)))
		&& ((pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_2)) || (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3)))) ? true : false;

	if (!bNextStep)
		return;

	if (pkStatue->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		return;

	const auto dwPlayerID = pkChar->GetPlayerID();
	const bool bCheck = std::find(vPlayersStatue.begin(), vPlayersStatue.end(), dwPlayerID) != vPlayersStatue.end();
	if (bCheck)
		return;

	if (!bCheck && pkChar->GetGMLevel() <= GM_PLAYER)
		vPlayersStatue.emplace_back(dwPlayerID);

	if (!e_DestroyStatues)
	{
		r_meleystatues_info* pEventInfo = AllocEventInfo<r_meleystatues_info>();
		pEventInfo->bFirst = false;
		pEventInfo->pMap = this;
		e_DestroyStatues = event_create(r_meleystatues_event, pEventInfo, PASSES_PER_SEC(1));
	}

	/*
	* This text doenst appear in Gameforge!
	if (!bRemainMessage)
	{
		bRemainMessage = true;
		char szBuf[512]{ 0 };
		snprintf(&szBuf[0], sizeof(szBuf), LC_TEXT("You have %d second(s) to destroy the statues, hurry up!"), MELEY_TIME_LIMIT_TO_KILL_STATUE);
		SendNoticeMap(&szBuf[0], GetMapIndex(), false);
	}
	*/

	ITEM_MANAGER::Instance().RemoveItem(pkItem, "MELEY_STATUE");
	pkStatue->RemoveAffect(AFFECT_STATUE);
	pkStatue->AddAffect(AFFECT_STATUE, POINT_NONE, 0, AFF_DRAGONLAIR_STONE_UNBEATABLE_3, 3600, 0, true);
	const bool bNextStepPass = ((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		&& (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		&& (pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		&& (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))) ? true : false;

	if (bNextStepPass)
		DungeonResult();
}

void CMeleyLair::DungeonResult()
{
	if (!pkBoss || !pkStatue1 || !pkStatue2 || !pkStatue3 || !pkStatue4)
		return;

	if (e_DestroyStatues)
		event_cancel(&e_DestroyStatues);

	e_DestroyStatues = nullptr;

	const bool bNextStep = ((pkStatue1->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		&& (pkStatue2->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		&& (pkStatue3->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		&& (pkStatue4->IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))) ? true : false;

	if (!bNextStep)
	{
		EndDungeon(false);
		return;
	}

	vPlayersReward.clear();
	SetRewardTime(get_global_time() + 10);

	pkStatue1->DeadNoReward();
	pkStatue1 = nullptr;
	pkStatue2->DeadNoReward();
	pkStatue2 = nullptr;
	pkStatue3->DeadNoReward();
	pkStatue3 = nullptr;
	pkStatue4->DeadNoReward();
	pkStatue4 = nullptr;
	pkBoss->DeadNoReward();
	pkBoss = nullptr;

	uint32_t dwChestVnum = MELEY_GUILD_CHEST_VNUM;
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	if (GetParty())
		dwChestVnum = MELEY_PARTY_CHEST_VNUM;
#endif

	const auto& pkChest = Spawn(dwChestVnum, 130, 130, 1);
	if (pkChest)
	{
#ifdef MELEY_GOLD_DROP
		int iMinGold = 0;
		int iMaxGold = 0;
		int iTotalGold = 0;
		if (GetGuild())
		{
			iMinGold = MELEY_MIN_GUILD_DROP_GOLD;
			iMaxGold = MELEY_MAX_GUILD_DROP_GOLD;
		}
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		else if (GetParty())
		{
			iMinGold = MELEY_MIN_PARTY_DROP_GOLD;
			iMaxGold = MELEY_MAX_PARTY_DROP_GOLD;
		}
#	endif

		const auto iSplitCount = number(25, 35);
		for (int i = 0; i < iSplitCount; ++i)
		{
			if ((!iMinGold) || (!iMaxGold))
				continue;

			const auto iGold = number(iMinGold, iMaxGold) / iSplitCount;

			auto item = ITEM_MANAGER::Instance().CreateItem(1, iGold);
			if (item)
			{
				PIXEL_POSITION pos;
				pos.x = pkChest->GetX() + ((number(-14, 14) + number(-14, 14)) * 23);
				pos.y = pkChest->GetY() + ((number(-14, 14) + number(-14, 14)) * 23);

				item->AddToGround(GetMapIndex(), pos);
				item->StartDestroyEvent();

				iTotalGold += iGold; // Total gold
			}
		}
#endif
#ifdef MELEY_REWARD_ITEM_DROP
		const int iItemDropRate = number(1, 100);
		if (iItemDropRate <= 60)
		{
			const int iItemDropCount = number(1, 3);
			constexpr int iItemRewardCount = sizeof(rewardItemList) / sizeof(rewardItemList[0]);
			for (int i = 0; i < iItemDropCount; ++i)
			{
				const auto dwRewardVnum = rewardItemList[number(1, iItemRewardCount)];
				auto pkItem = ITEM_MANAGER::Instance().CreateItem(dwRewardVnum);
				if (!pkItem)
					continue;

				PIXEL_POSITION pos;
				pos.x = pkChest->GetX() + ((number(-14, 14) + number(-14, 14)) * 23);
				pos.y = pkChest->GetY() + ((number(-14, 14) + number(-14, 14)) * 23);

				pkItem->AddToGround(GetMapIndex(), pos);
				pkItem->StartDestroyEvent();
			}
		}
#endif
	}

	SetDungeonCompleted();
	EndDungeon(true);

}

bool CMeleyLair::CheckRewarder(uint32_t dwPlayerID)
{
	const bool bCheck = std::find(vPlayersReward.begin(), vPlayersReward.end(), dwPlayerID) != vPlayersReward.end();
	return bCheck;
}

void CMeleyLair::GiveReward(LPCHARACTER pkChar, uint16_t bReward)
{
	if (!pkChar)
		return;

	const auto dwPlayerID = pkChar->GetPlayerID();
	if (!IsParticipantRegistered(dwPlayerID))
		return;

	if (CheckRewarder(dwPlayerID))
		return;

	if (bReward <= 0 || bReward > 3)
		return;

	vPlayersReward.emplace_back(dwPlayerID);

	const uint32_t dwVnumReward[] = { MELEY_REWARD_ITEMCHEST_VNUM_1, MELEY_REWARD_ITEMCHEST_VNUM_2, MELEY_REWARD_ITEMCHEST_VNUM_3 };
	const auto dwVnum = dwVnumReward[bReward - 1];
	pkChar->AutoGiveItem(dwVnum, 1);
}

bool CMeleyLair::IsPlayerOut(uint32_t dwPlayerID)
{
	const bool bCheck = std::find(vPlayersOut.begin(), vPlayersOut.end(), dwPlayerID) != vPlayersOut.end();
	return bCheck;
}

void CMeleyLair::SetPlayerOut(uint32_t dwPlayerID)
{
	if (!IsParticipantRegistered(dwPlayerID))
		return;

	vPlayersOut.emplace_back(dwPlayerID);
}

void CMeleyLairManager::Initialize()
{
	m_RegGuilds.clear();
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	m_RegPartys.clear();
#endif

	SetXYZ(0, 0, 0);
	SetSubXYZ(0, 0, 0);
}

void CMeleyLairManager::Destroy()
{
	// Guild
	for (auto it = m_RegGuilds.begin(); it != m_RegGuilds.end(); ++it)
	{
		auto* pMap = it->second;
		if (pMap)
		{
			CMeleyLairManager::Instance().DestroyPrivateMap(pMap->GetMapIndex());
			pMap->Destroy();
			M2_DELETE(pMap);
		}
	}

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	// Party
	for (auto it = m_RegPartys.begin(); it != m_RegPartys.end(); ++it)
	{
		auto* pMap = it->second;
		if (pMap)
		{
			CMeleyLairManager::Instance().DestroyPrivateMap(pMap->GetMapIndex());
			pMap->Destroy();
			M2_DELETE(pMap);
		}
	}
#endif

	m_RegGuilds.clear();
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	m_RegPartys.clear();
#endif

	SetXYZ(0, 0, 0);
	SetSubXYZ(0, 0, 0);
}

bool CMeleyLairManager::isGuildRegistered(const CGuild* pkGuild, int& iCH) noexcept
{
	iCH = 0;
	if (pkGuild)
	{
		if (pkGuild->GetDungeonCH())
		{
			iCH = pkGuild->GetDungeonCH();
			return true;
		}
	}

	return false;
}

void CMeleyLairManager::RegisterGuild(const LPCHARACTER& pkChar, int& iRes1, int& iRes2)
{
	iRes1 = 0;
	iRes2 = 0;

	if (!pkChar)
		return;

	CGuild* pkGuild = pkChar->GetGuild();
	if (!pkGuild)
		return;

	if (pkGuild->GetMasterPID() != pkChar->GetPlayerID())
		return;

	int iCH;
	if (isGuildRegistered(pkGuild, iCH))
	{
		iRes1 = 2;
		iRes2 = iCH;
		return;
	}

	if (pkGuild->GetLadderPoint() < MELEY_GUILD_LADDER_POINTS_COST)
	{
		iRes1 = 3;
		iRes2 = MELEY_GUILD_LADDER_POINTS_COST;
		return;
	}

	if (pkGuild->GetLevel() < MELEY_MIN_GUILD_LEVEL)
	{
		iRes1 = 4;
		iRes2 = MELEY_MIN_GUILD_LEVEL;
		return;
	}

	const uint32_t dwTimeNow = get_global_time();
	if ((pkGuild->GetDungeonCooldown() > dwTimeNow) && (!pkGuild->GetDungeonCH()))
	{
		int iDif = pkGuild->GetDungeonCooldown() - dwTimeNow;
		iRes1 = 5;
		iRes2 = iDif;
		return;
	}

	if (pkGuild->UnderAnyWar())
	{
		iRes1 = 6;
		iRes2 = 0;
		return;
	}

	/*const long lMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(MapTypes::MAP_N_FLAME_DRAGON);
	if (!lMapIndex)
	{
		iRes1 = 7;
		return;
	}*/

	if (pkGuild->GetDungeonCH()) // Not official
	{
		iRes1 = 8;
		iRes2 = 0;
		return;
	}

	constexpr auto iCost = static_cast<int>(eGuild::MELEY_GUILD_LADDER_POINTS_COST);
	pkGuild->ChangeLadderPoint(-iCost);
	pkGuild->SetLadderPoint(pkGuild->GetLadderPoint() - iCost);

	const PIXEL_POSITION pos = GetXYZ(), posSub = GetSubXYZ();
	if (!pos.x)
	{
		const auto& pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(MapTypes::MAP_N_FLAME_DRAGON);
		if (pkSectreeMap)
			SetXYZ(pkSectreeMap->m_setting.iBaseX + 130 * 100, pkSectreeMap->m_setting.iBaseY + 130 * 100, 0);
	}

	if (!posSub.x)
	{
		const auto& pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(MELEY_SUBMAP_INDEX);
		if (pkSectreeMap)
			SetSubXYZ(pkSectreeMap->m_setting.iBaseX + 87 * 100, pkSectreeMap->m_setting.iBaseY + 853 * 100, 0);
	}

	pkGuild->RequestDungeon(g_bChannel, 0);
	pkGuild->SetDungeonCooldown(0);

	iRes1 = 1;
	iRes2 = MELEY_GUILD_LADDER_POINTS_RETURN;
}

bool CMeleyLairManager::EnterGuild(LPCHARACTER pkChar, int& iLimit)
{
	iLimit = 0;

	if (!pkChar)
		return false;

	CGuild* pkGuild = pkChar->GetGuild();
	if (!pkGuild)
		return false;

	constexpr auto dwMapIndex = MapTypes::MAP_N_FLAME_DRAGON;
	constexpr auto dwSubMapIndex = MELEY_SUBMAP_INDEX;

	PIXEL_POSITION mPos;
	if (!SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(dwMapIndex, 0, mPos))
	{
		iLimit = 1;
		return true;
	}

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) || defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
	if (!pkGuild->GetMasterCharacter())	//if kein Leader Enter
	{
		pkChar->ChatPacket(7, "I'm not a leader");
		if (!pkGuild->IsGeneralMember(pkChar->GetPlayerID())) // If no berechtigung ENTER
		{
			pkChar->ChatPacket(7, "I have no privileges to enter!");
			iLimit = 2;
			return true;
		}
	}
#else
	if (pkChar->GetPKMode() != PK_MODE_GUILD)
	{
		iLimit = 2;
		return true;
	}
#endif

	if (pkChar->IsRiding())
	{
		iLimit = 3;
		return true;
	}

	if (pkChar->GetLevel() < MELEY_PLAYER_GUILD_MIN_LEVEL)
	{
		iLimit = 4;
		return true;
	}

	if (m_RegGuilds.find(pkGuild) == m_RegGuilds.end())
	{
		const auto bCH = pkGuild->GetDungeonCH();
		if (bCH && (g_bChannel != bCH))
		{
			iLimit = pkGuild->GetDungeonCH();
			return false;
		}

		if (g_bChannel != pkGuild->GetDungeonCH())
			return false;

		const long lMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(MapTypes::MAP_N_FLAME_DRAGON);
		if (!lMapIndex)
			return false;

		if (!lMapIndex)
		{
			constexpr auto iCost = MELEY_GUILD_LADDER_POINTS_COST;
			pkGuild->ChangeLadderPoint(+iCost);
			pkGuild->SetLadderPoint(pkGuild->GetLadderPoint() + iCost);
			return false;
		}

		const PIXEL_POSITION pos = GetXYZ(), posSub = GetSubXYZ();
		if (!pos.x)
		{
			const auto& pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(dwMapIndex);
			if (pkSectreeMap)
				SetXYZ(pkSectreeMap->m_setting.iBaseX + 130 * 100, pkSectreeMap->m_setting.iBaseY + 130 * 100, 0);
		}

		if (!posSub.x)
		{
			const auto& pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(dwSubMapIndex);
			if (pkSectreeMap)
				SetSubXYZ(pkSectreeMap->m_setting.iBaseX + 87 * 100, pkSectreeMap->m_setting.iBaseY + 853 * 100, 0);
		}

		pkGuild->RequestDungeon(g_bChannel, lMapIndex);
		CMeleyLair* pMap = new CMeleyLair(lMapIndex);
		if (!pMap)
		{
			sys_err("Could not create MeleyLair instance.");
			return false;
		}

		m_map_pkMapDungeon.insert(std::make_pair(lMapIndex, pMap));
		m_RegGuilds.insert(std::make_pair(pkGuild, pMap));

		pMap->SetDungeonType(MELEY_TYPE_GUILD_RED_DRAGON_LAIR);
		pMap->SetGuild(pkGuild);

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		// UPDATE_FIRST_RANKING_TIME
		if (!pMap->GetFirstRankingTime())
		{
			auto pMsg2(DBManager::Instance().DirectQuery(
				"SELECT time FROM log.meley_lair_guild%s ORDER by time ASC LIMIT 1",
				get_table_postfix()));

			if (pMsg2->Get()->uiNumRows > 0)
			{
				const auto& row = mysql_fetch_row(pMsg2->Get()->pSQLResult);
				pMap->SetFirstRankingTime(atoi(row[0]));
			}
		}
		// END_OF_UPDATE_FIRST_RANKING_TIME
#endif

		pMap->Start();
	}

	auto pMap = m_RegGuilds.find(pkGuild)->second;
	if (!pMap)
		return false;

	// Partecipants Limit
	constexpr auto dwMaxPartecipants = MELEY_PARTECIPANTS_MAX_LIMIT;
	if (pMap->GetParticipantsCount() >= dwMaxPartecipants)
	{
		iLimit = 5;
		return true;
	}

	// Dungeon Start
	if (pMap->GetDungeonStep() >= 1)
	{
		iLimit = 6;
		return true;
	}

	const auto dwPlayerID = pkChar->GetPlayerID();
	if (pMap->IsPlayerOut(dwPlayerID))
	{
		iLimit = 7;
		return true;
	}

#ifdef MELEY_ENABLE_COOLDOWN
	auto qPC = quest::CQuestManager::Instance().GetPC(dwPlayerID);
	if (!qPC)
	{
		iLimit = 1;
		return true;
	}

	int duration_time = qPC->GetFlag(MELEY_GUILD_COOLDOWN_FLAG);
	if (get_global_time() < duration_time)
	{
		iLimit = 8;
		return true;
	}
#endif

	// Check if there is a war!
	if (pkGuild->UnderAnyWar())
	{
		iLimit = 10;
		return true;
	}

	if (!pMap->IsParticipantRegistered(pkChar->GetPlayerID()))
	{
#ifdef GUILD_DRAGONLAIR_TICKET
		const auto* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(10040);
		if (pItemGroup)
		{
			bool canPass = false;
			const int iTicketCount = pItemGroup->GetGroupSize();
			for (int i = 0; i < iTicketCount; i++)
			{
				const uint32_t dwTicketVnum = pItemGroup->GetVnum(i);
				if (pkChar->CountSpecifyItem(dwTicketVnum) >= 1)
				{
					canPass = true;
					pkChar->RemoveSpecifyItem(dwTicketVnum, 1);
					break;
				}
			}

			if (!canPass)
			{
				iLimit = 9;
				return true;
			}
		}
#endif
#ifdef MELEY_ENABLE_COOLDOWN
		constexpr time_t guild_cooldown = MELEY_GUILD_COOLDOWN_DUNGEON;
		qPC->SetFlag(MELEY_GUILD_COOLDOWN_FLAG, get_global_time() + guild_cooldown);
#endif
		pMap->RegisterParticipant(pkChar);
	}

	pkChar->WarpSet(mPos.x, mPos.y, pMap->GetMapIndex());

	return true;
}

CMeleyLair* CMeleyLairManager::FindByMapIndex(long lMapIndex)
{
	const auto it = m_map_pkMapDungeon.find(lMapIndex);
	if (it != m_map_pkMapDungeon.end())
		return it->second;

	return nullptr;
}

uint16_t CMeleyLairManager::GetCharacterDungeonType(const LPCHARACTER& pkChar)
{
	if (!pkChar)
		return 0;

	CMeleyLair* pMap = nullptr;
	if (pkChar->GetGuild())
	{
		if (m_RegGuilds.find(pkChar->GetGuild()) != m_RegGuilds.end())
			pMap = m_RegGuilds.find(pkChar->GetGuild())->second;
	}
#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	if (pkChar->GetParty())
	{
		if (m_RegPartys.find(pkChar->GetParty()) != m_RegPartys.end())
			pMap = m_RegPartys.find(pkChar->GetParty())->second;
	}
#endif

	if (!pMap)
		return 0;

	return pMap->GetDungeonType();
}

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
bool CMeleyLairManager::IsPartyMemberRegistered(const LPCHARACTER& pkChar)
{
	if (!pkChar || !pkChar->GetParty())
		return false;

	if (m_RegPartys.find(pkChar->GetParty()) == m_RegPartys.end())
		return false;

	auto pMap = m_RegPartys.find(pkChar->GetParty())->second;
	if (!pMap)
		return false;

	if (!pMap->IsParticipantRegistered(pkChar->GetPlayerID()))
		return false;

	return true;
}

bool CMeleyLairManager::EnterParty(LPCHARACTER pkChar, int& iLimit)
{
	iLimit = 0;

	if (!pkChar)
		return false;

	constexpr uint32_t dwMapIndex = MapTypes::MAP_N_FLAME_DRAGON;
	constexpr uint32_t dwSubMapIndex = MELEY_SUBMAP_INDEX;

	PIXEL_POSITION mPos;
	if (!SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(dwMapIndex, 0, mPos))
	{
		iLimit = 1;
		return true;
	}

	if (!pkChar->GetParty())
	{
		iLimit = 2;
		return false;
	}

	if (!test_server && (pkChar->GetParty()->GetNearMemberCount() < MELEY_PARTECIPANTS_MIN_LIMIT))
	{
		iLimit = 3;
		return true;
	}

	if (pkChar->GetParty()->GetNearMemberCount() > eDefault::MELEY_PARTECIPANTS_MAX_LIMIT)
	{
		iLimit = 4;
		return true;
	}

	if (pkChar->IsRiding())
	{
		iLimit = 5;
		return false;
	}

	if (pkChar->GetLevel() < MELEY_PLAYER_PARTY_MIN_LEVEL)
	{
		iLimit = 6;
		return true;
	}

	bool bWarpParty = false;

	const auto pParty = pkChar->GetParty();
	const auto pszLeaderName = pkChar->GetName();

	FPartyPIDCollector fPartyPIDCollector;
	pParty->ForEachOnMapMember(fPartyPIDCollector, pkChar->GetMapIndex());

	if (m_RegPartys.find(pParty) == m_RegPartys.end())
	{
		if (pParty->GetLeaderPID() != pkChar->GetPlayerID())
		{
			iLimit = 7;
			return false;
		}

		// Remove Tickets
		const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(10033);
		if (!pItemGroup)
			return false;

		const auto iTicketCount = pItemGroup->GetGroupSize();
		for (std::vector<uint32_t>::iterator pPartyPID = fPartyPIDCollector.vecPIDs.begin();
			pPartyPID != fPartyPIDCollector.vecPIDs.end(); pPartyPID++)
		{
			auto pPC = CHARACTER_MANAGER::Instance().FindByPID(*pPartyPID);
			if (!pPC)
				continue;

			for (int i = 0; i < iTicketCount; i++)
			{
				const uint32_t dwTicketVnum = pItemGroup->GetVnum(i);
				if (pPC->CountSpecifyItem(dwTicketVnum) >= 1)
				{
					pPC->RemoveSpecifyItem(dwTicketVnum, 1);
					break;
				}
			}
		}

		// Create Dungeon
		const long lMapIndex = SECTREE_MANAGER::Instance().CreatePrivateMap(MapTypes::MAP_N_FLAME_DRAGON);
		if (!lMapIndex)
			return false;

		const PIXEL_POSITION pos = GetXYZ(), posSub = GetSubXYZ();
		if (!pos.x)
		{
			const auto& pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(dwMapIndex);
			if (pkSectreeMap)
				SetXYZ(pkSectreeMap->m_setting.iBaseX + 130 * 100, pkSectreeMap->m_setting.iBaseY + 130 * 100, 0);
		}

		if (!posSub.x)
		{
			const auto& pkSectreeMap = SECTREE_MANAGER::Instance().GetMap(dwSubMapIndex);
			if (pkSectreeMap)
				SetSubXYZ(pkSectreeMap->m_setting.iBaseX + 87 * 100, pkSectreeMap->m_setting.iBaseY + 853 * 100, 0);
		}

		CMeleyLair* pMap = new CMeleyLair(lMapIndex);
		if (!pMap)
		{
			sys_err("Could not create MeleyLair instance.");
			return false;
		}

		m_map_pkMapDungeon.insert(std::make_pair(lMapIndex, pMap));
		m_RegPartys.insert(std::make_pair(pParty, pMap));

		pMap->SetDungeonType(MELEY_TYPE_PARTY_RED_DRAGON_LAIR);
		pMap->SetParty(pParty);
		pMap->SetPartyLeaderName(pszLeaderName);

		// UPDATE_FIRST_RANKING_TIME
		if (!pMap->GetFirstRankingTime())
		{
			// Check the First of Ranking
			auto pMsg(DBManager::Instance().DirectQuery(
				"SELECT time FROM log.meley_lair_party%s ORDER by time ASC LIMIT 1",
				get_table_postfix()));

			if (pMsg->Get()->uiNumRows > 0)
			{
				const auto& row = mysql_fetch_row(pMsg->Get()->pSQLResult);
				pMap->SetFirstRankingTime(atoi(row[0]));
			}
		}
		// END_OF_UPDATE_FIRST_RANKING_TIME

		pMap->Start();
		bWarpParty = true;
	}

	auto pMap = m_RegPartys.find(pParty)->second;
	if (!pMap)
		return false;

	if (pMap->GetDungeonStep() >= 2)
	{
		iLimit = 8;
		return true;
	}

	const auto dwPlayerID = pkChar->GetPlayerID();
	if (pMap->IsPlayerOut(dwPlayerID))
	{
		iLimit = 9;
		return true;
	}

#ifdef MELEY_ENABLE_COOLDOWN
	const auto qPC = quest::CQuestManager::Instance().GetPCForce(dwPlayerID);
	if (!qPC)
	{
		iLimit = 1;
		return true;
	}

	int duration_time = qPC->GetFlag(MELEY_PARTY_COOLDOWN_FLAG);
	if (get_global_time() < duration_time)
	{
		iLimit = 10;
		return true;
	}
#endif

	// Check if there is a war!
	CGuild* pkGuild = pkChar->GetGuild();
	if (pkGuild && pkGuild->UnderAnyWar())
	{
		iLimit = 12;
		return true;
	}

	if (bWarpParty)
	{
		for (std::vector<uint32_t>::iterator pPartyPID = fPartyPIDCollector.vecPIDs.begin();
			pPartyPID != fPartyPIDCollector.vecPIDs.end(); pPartyPID++)
		{
			auto pPC = CHARACTER_MANAGER::Instance().FindByPID(*pPartyPID);
			if (!pPC)
				continue;

			if (!pMap->IsParticipantRegistered(pPC->GetPlayerID()))
			{
#ifdef MELEY_ENABLE_COOLDOWN
				const auto qPartyPC = quest::CQuestManager::Instance().GetPCForce(*pPartyPID);
				if (qPartyPC)
				{
					qPartyPC->SetFlag(MELEY_PARTY_COOLDOWN_FLAG, get_global_time() + (MELEY_PARTY_COOLDOWN_DUNGEON));
				}
#endif

				pMap->RegisterParticipant(pPC);
				pPC->WarpSet(mPos.x, mPos.y, pMap->GetMapIndex());
			}
		}
	}
	else
	{
		if (!pMap->IsParticipantRegistered(pkChar->GetPlayerID()))
		{
			iLimit = 11;
			return true;
		}

		pkChar->WarpSet(mPos.x, mPos.y, pMap->GetMapIndex());
	}

	return true;
}
#endif

void CMeleyLairManager::EnterGame(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	auto pMap = CMeleyLairManager::Instance().FindByMapIndex(pkChar->GetMapIndex());
	if (!pMap)
	{
		// Skip Warpout for GM
		//if (pkChar->IsGM())
		//	return;

		ExitCharacter(pkChar);
		return;
	}

	// Check if Player is Registered
	if (pMap && !pMap->IsParticipantRegistered(pkChar->GetPlayerID()))
	{
		if (!pkChar->IsGM()) // Skip Warpout for GM
		{
			ExitCharacter(pkChar);
			return;
		}
	}

	// Register Meley
	pkChar->SetMeleyLair(pMap);

	// Update Times
	if (pMap && (pkChar->GetMapIndex() == pMap->GetMapIndex()))
	{
		const uint32_t dwFirstRankingTime = pMap->GetFirstRankingTime();
		if (dwFirstRankingTime > 0)
		{
			// Send Packet Time
			TPacketMeleyLairTimeInfo packet{};
			packet.bHeader = HEADER_GC_MELEY_LAIR_TIME_INFO;
			packet.dwTime = ((time_t)pMap->GetElapsedTime() < (time_t)pMap->GetFirstRankingTime()) ?
				(dwFirstRankingTime - pMap->GetElapsedTime()) : 1;
			pkChar->GetDesc()->Packet(&packet, sizeof(packet));

			// Start time
			if (pMap->GetDungeonTimeStart() && !pMap->IsDungeonCompleted())
			{
				TPacketMeleyLairTimeResult packetResult{};
				packetResult.bHeader = HEADER_GC_MELEY_LAIR_TIME_RESULT;
				packetResult.bResult = 0;
				pkChar->GetDesc()->Packet(&packetResult, sizeof(packetResult));
			}

			// Completed
			if (pMap->IsDungeonCompleted())
			{
				TPacketMeleyLairTimeResult packetResult2{};
				packetResult2.bHeader = HEADER_GC_MELEY_LAIR_TIME_RESULT;
				packetResult2.bResult = pMap->IsDungeonTimeSuccess() ? 1 : 0;
				pkChar->GetDesc()->Packet(&packetResult2, sizeof(packetResult2));
			}
		}
		else // Set NONE if not ranking exist
		{
			// Send Packet Time
			TPacketMeleyLairTimeInfo packet{};
			packet.bHeader = HEADER_GC_MELEY_LAIR_TIME_INFO;
			packet.dwTime = 0;
			pkChar->GetDesc()->Packet(&packet, sizeof(packet));
		}
	}
}

/*void CMeleyLairManager::LeaveGame(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (!pkChar->GetDungeon())
		return;

	if (!pkChar->GetDungeon()->GetMeleyLair())
		return;

	CMeleyLair* pMap = pkChar->GetDungeon()->GetMeleyLair();

	if (!pMap)
	{
		ExitCharacter(pkChar);
		return;
	}

	if (!pMap->IsParticipantRegistered(pkChar->GetPlayerID()))
	{
		ExitCharacter(pkChar);
		return;
	}

}*/

void CMeleyLairManager::LeaveRequest(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (!pkChar->GetMeleyLair())
		return;

	auto pMap = pkChar->GetMeleyLair();

	if (!pMap)
	{
		ExitCharacter(pkChar);
		return;
	}

	const auto dwPlayerID = pkChar->GetPlayerID();
	if (!pMap->IsPlayerOut(dwPlayerID))
		pMap->SetPlayerOut(dwPlayerID);

	ExitCharacter(pkChar);
}

bool CMeleyLairManager::IsMeleyMap(long lMapIndex) noexcept
{
	constexpr long map_index = MapTypes::MAP_N_FLAME_DRAGON;

	if (lMapIndex == map_index)
		return true;

	if (lMapIndex >= (map_index * 10000) && lMapIndex < ((map_index + 1) * 10000))
		return true;

	return false;
}

void CMeleyLairManager::ExitCharacter(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

#ifdef MELEY_FORCE_WARPOUT
	pkChar->WarpSet(MELEY_WARP_OUT_X, MELEY_WARP_OUT_Y, MELEY_SUBMAP_INDEX);
#else
	PIXEL_POSITION posSub = GetSubXYZ();
	if (!posSub.x)
		pkChar->WarpSet(EMPIRE_START_X(pkChar->GetEmpire()), EMPIRE_START_Y(pkChar->GetEmpire()));
	else
		pkChar->WarpSet(posSub.x, posSub.y);
#endif
}

void CMeleyLairManager::DestroyPrivateMap(long lMapIndex)
{
	m_map_pkMapDungeon.erase(lMapIndex);
	SECTREE_MANAGER::Instance().DestroyPrivateMap(lMapIndex);
}

void CMeleyLairManager::ClearGuild(CGuild* pkGuild)
{
	if (!pkGuild)
		return;

	const auto iter = m_RegGuilds.find(pkGuild), iterEnd = m_RegGuilds.end();
	if (iter != iterEnd)
		m_RegGuilds.erase(iter, iterEnd);
}

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
void CMeleyLairManager::ClearParty(LPPARTY pkParty)
{
	if (!pkParty)
		return;

	const auto iter = m_RegPartys.find(pkParty), iterEnd = m_RegPartys.end();
	if (iter != iterEnd)
		m_RegPartys.erase(iter, iterEnd);
}
#endif

bool CMeleyLairManager::CanGetReward(LPCHARACTER pkChar)
{
	if (!pkChar)
		return false;

	if (!pkChar->GetMeleyLair())
		return false;

	auto pMap = pkChar->GetMeleyLair();
	if (!pMap)
		return false;

	if (!pMap->IsParticipantRegistered(pkChar->GetPlayerID()))
		return false;

	if (pMap->GetDungeonStep() != 4)
		return false;

	const uint32_t dwTime = get_global_time();
	if (pMap->GetRewardTime() > dwTime)
		return false;

	if (pMap->CheckRewarder(pkChar->GetPlayerID()))
		return false;

	return true;
}

void CMeleyLairManager::Reward(LPCHARACTER pkChar, uint16_t bReward)
{
	if (!pkChar)
		return;

	if (!pkChar->GetMeleyLair())
		return;

	auto pMap = pkChar->GetMeleyLair();
	if (!pMap)
		return;

	pMap->GiveReward(pkChar, bReward);
}

void CMeleyLairManager::OpenRanking(const LPCHARACTER& pkChar)
{
	if (!pkChar)
		return;

	//sys_err("unfinished");
}

void CMeleyLairManager::GuildMemberRemoved(LPCHARACTER pkChar, CGuild* pkGuild)
{
	if (!pkChar || !pkGuild)
		return;

	if (!IsMeleyMap(pkChar->GetMapIndex()))
		return;

	if (m_RegGuilds.find(pkGuild) != m_RegGuilds.end())
	{
		const auto pMap = m_RegGuilds.find(pkGuild)->second;
		if (pMap)
			ExitCharacter(pkChar);
	}
}

void CMeleyLairManager::GuildRemoved(CGuild* pkGuild)
{
	if (!pkGuild)
		return;

	if (m_RegGuilds.find(pkGuild) != m_RegGuilds.end())
	{
		auto pMap = m_RegGuilds.find(pkGuild)->second;
		if (pMap)
			pMap->EndDungeonWarp();
	}
}

void CMeleyLairManager::StartDungeon(LPCHARACTER pkChar)
{
	if (!pkChar)
		return;

	if (!pkChar->GetMeleyLair())
		return;

	auto pMap = pkChar->GetMeleyLair();
	if (!pMap)
		return;

	return pMap->StartDungeon(pkChar);
}

void CMeleyLairManager::EndDungeon(LPCHARACTER pkChar, bool bSuccess, bool bIsCancelled)
{
	if (!pkChar)
		return;

	if (!pkChar->GetMeleyLair())
		return;

	auto pMap = pkChar->GetMeleyLair();
	if (!pMap)
		return;

	pMap->EndDungeon(bSuccess, bIsCancelled);
}

uint16_t CMeleyLairManager::GetDungeonStep(const LPCHARACTER& pkChar) noexcept
{
	if (!pkChar)
		return 0;

	if (!pkChar->GetMeleyLair())
		return 0;

	auto pMap = pkChar->GetMeleyLair();
	if (!pMap)
		return 0;

	const auto bStep = pMap->GetDungeonStep();
	return bStep;
}
#endif
