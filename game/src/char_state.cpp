#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "battle.h"
#include "char_manager.h"
#include "packet.h"
#include "motion.h"
#include "party.h"
#include "affect.h"
#include "buffer_manager.h"
#include "questmanager.h"
#include "p2p.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "exchange.h"
#include "sectree_manager.h"
#include "xmas_event.h"
#include "guild_manager.h"
#include "war_map.h"
#include "locale_service.h"
#include "BlueDragon.h"
#include "../../common/VnumHelper.h"
#ifdef ENABLE_DAWNMIST_DUNGEON
#include "DawnMistDungeon.h"
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#include "MeleyLair.h"
#endif
#ifdef ENABLE_12ZI
#include "zodiac_temple.h"
#endif

extern LPCHARACTER FindVictim(LPCHARACTER pkChr, int iMaxDistance);

namespace
{
	class FuncFindChrForFlag
	{
		public:
			FuncFindChrForFlag(LPCHARACTER pkChr) noexcept
				: m_pkChr(pkChr), m_pkChrFind(nullptr), m_iMinDistance(INT_MAX)
			{}

			void operator() (LPENTITY ent)
			{
				if (!ent)
					return;

				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				if (ent->IsObserverMode())
					return;

				LPCHARACTER pkChr = dynamic_cast<LPCHARACTER>(ent);
				if (!pkChr)
					return;

				if (!pkChr->IsPC())
					return;

				if (!pkChr->GetGuild())
					return;

				if (pkChr->IsDead())
					return;

				const int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkChr->GetX(), pkChr->GetY() - m_pkChr->GetY());

				if (iDist <= 500 && m_iMinDistance > iDist &&
					!pkChr->IsAffectFlag(AFF_WAR_FLAG1) &&
					!pkChr->IsAffectFlag(AFF_WAR_FLAG2) &&
					!pkChr->IsAffectFlag(AFF_WAR_FLAG3))
				{
					// If it's our flag
					if (static_cast<uint32_t>(m_pkChr->GetPoint(POINT_STAT)) == pkChr->GetGuild()->GetID())
					{
						CWarMap* pMap = pkChr->GetWarMap();
						uint8_t idx;

						if (!pMap || !pMap->GetTeamIndex(pkChr->GetGuild()->GetID(), idx))
							return;

						// Draw a flag only when there is no flag in our base. Otherwise, the flag on the base
						// I want to leave it alone, but I can be pulled out...
						if (!pMap->IsFlagOnBase(idx))
						{
							m_pkChrFind = pkChr;
							m_iMinDistance = iDist;
						}
					}
					else
					{
						// If it is the opponent's flag, draw it unconditionally.
						m_pkChrFind = pkChr;
						m_iMinDistance = iDist;
					}
				}
			}

			LPCHARACTER m_pkChr;
			LPCHARACTER m_pkChrFind;
			int m_iMinDistance;
		};

		class FuncFindChrForFlagBase
		{
		public:
			FuncFindChrForFlagBase(LPCHARACTER pkChr) noexcept
				: m_pkChr(pkChr)
			{}

			void operator() (LPENTITY ent)
			{
				if (!ent)
					return;

				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				if (ent->IsObserverMode())
					return;

				const LPCHARACTER& pkChr = dynamic_cast<LPCHARACTER>(ent);
				if (!pkChr)
					return;

				if (!pkChr->IsPC())
					return;

				const CGuild* pkGuild = pkChr->GetGuild();

				if (!pkGuild)
					return;

				const int iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkChr->GetX(), pkChr->GetY() - m_pkChr->GetY());

				if (iDist <= 500 &&
					(pkChr->IsAffectFlag(AFF_WAR_FLAG1) ||
						pkChr->IsAffectFlag(AFF_WAR_FLAG2) ||
						pkChr->IsAffectFlag(AFF_WAR_FLAG3)))
				{
					CAffect* pkAff = pkChr->FindAffect(AFFECT_WAR_FLAG);

					sys_log(0, "FlagBase %s dist %d aff %p flag gid %d chr gid %u",
						pkChr->GetName(), iDist, pkAff, m_pkChr->GetPoint(POINT_STAT),
						pkChr->GetGuild()->GetID());

					if (pkAff)
					{
						if (static_cast<uint32_t>(m_pkChr->GetPoint(POINT_STAT)) == pkGuild->GetID() &&
							m_pkChr->GetPoint(POINT_STAT) != pkAff->lApplyValue)
						{
							CWarMap* pMap = pkChr->GetWarMap();
							uint8_t idx;

							if (!pMap || !pMap->GetTeamIndex(pkGuild->GetID(), idx))
								return;

							//if (pMap->IsFlagOnBase(idx))
							{
								const uint8_t idx_opp = idx == 0 ? 1 : 0;

								SendGuildWarScore(m_pkChr->GetPoint(POINT_STAT), pkAff->lApplyValue, 1);
								//SendGuildWarScore(pkAff->lApplyValue, m_pkChr->GetPoint(POINT_STAT), -1);

								pMap->ResetFlag();
								//pMap->AddFlag(idx_opp);
								//pkChr->RemoveAffect(AFFECT_WAR_FLAG);

								char buf[256];
								snprintf(buf, sizeof(buf), LC_TEXT("[381]The guild %s's flag has been stolen by player %s."), pMap->GetGuild(idx)->GetName(), pMap->GetGuild(idx_opp)->GetName());
								pMap->Notice(buf);
							}
						}
					}
				}
			}

			LPCHARACTER m_pkChr;
	};

	class FuncFindGuardVictim
	{
		public:
			FuncFindGuardVictim(LPCHARACTER pkChr, int iMaxDistance) :
				m_pkChr(pkChr),
				m_iMinDistance(INT_MAX),
				m_iMaxDistance(iMaxDistance),
				m_lx(pkChr->GetX()),
				m_ly(pkChr->GetY()),
				m_pkChrVictim(nullptr)
			{
			};

			void operator() (LPENTITY ent)
			{
				if (!ent)
					return;

				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				LPCHARACTER pkChr = dynamic_cast<LPCHARACTER>(ent);
				if (!pkChr)
					return;

				// Don't attack the PC first
				if (pkChr->IsPC())
					return;


				if (pkChr->IsNPC() && !pkChr->IsMonster())
					return;

				if (pkChr->IsDead())
					return;

				if (pkChr->IsAffectFlag(AFF_EUNHYUNG) ||
					pkChr->IsAffectFlag(AFF_INVISIBILITY) ||
					pkChr->IsAffectFlag(AFF_REVIVE_INVISIBLE))
					return;

				// dwarf pass
				if (pkChr->GetRaceNum() == 5001)
					return;

				const int iDistance = DISTANCE_APPROX(m_lx - pkChr->GetX(), m_ly - pkChr->GetY());

				if (iDistance < m_iMinDistance && iDistance <= m_iMaxDistance)
				{
					m_pkChrVictim = pkChr;
					m_iMinDistance = iDistance;
				}
			}

			LPCHARACTER GetVictim() noexcept { return (m_pkChrVictim); }

		private:
			LPCHARACTER m_pkChr;

			int m_iMinDistance;
			int m_iMaxDistance;
			long m_lx;
			long m_ly;

			LPCHARACTER m_pkChrVictim;
	};
}

bool CHARACTER::IsAggressive() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE);
}

void CHARACTER::SetAggressive() noexcept
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_AGGRESSIVE);
}

bool CHARACTER::IsCoward() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_COWARD);
}

void CHARACTER::SetCoward() noexcept
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_COWARD);
}

bool CHARACTER::IsBerserker() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_BERSERK);
}

bool CHARACTER::IsStoneSkinner() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_STONESKIN);
}

bool CHARACTER::IsGodSpeeder() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_GODSPEED);
}

bool CHARACTER::IsDeathBlower() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_DEATHBLOW);
}

bool CHARACTER::IsReviver() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_REVIVE);
}

#ifdef ENABLE_DAWNMIST_DUNGEON
bool CHARACTER::IsHealer() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_HEALER);
}
#endif

void CHARACTER::CowardEscape()
{
	const int iDist[4] = { 500, 1000, 3000, 5000 };

	for (int iDistIdx = 2; iDistIdx >= 0; --iDistIdx)
	{
		for (int iTryCount = 0; iTryCount < 8; ++iTryCount)
		{
			SetRotation(static_cast<float>(number(0, 359))); // direction is set randomly

			float fx, fy;
			const auto fDist = static_cast<float>(number(iDist[iDistIdx], iDist[iDistIdx + 1]));

			GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

			bool bIsWayBlocked = false;
			for (int j = 1; j <= 100; ++j)
			{
				if (!SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx) * j / 100, GetY() + static_cast<int>(fy) * j / 100))
				{
					bIsWayBlocked = true;
					break;
				}
			}

			if (bIsWayBlocked)
				continue;

			m_dwStateDuration = PASSES_PER_SEC(1);

			const int iDestX = GetX() + static_cast<int>(fx);
			const int iDestY = GetY() + static_cast<int>(fy);

			if (Goto(iDestX, iDestY))
				SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

			sys_log(0, "WAEGU move to %d %d (far)", iDestX, iDestY);
			return;
		}
	}
}

void CHARACTER::SetNoAttackShinsu() noexcept
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKSHINSU);
}
bool CHARACTER::IsNoAttackShinsu() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKSHINSU);
}

void CHARACTER::SetNoAttackChunjo() noexcept
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKCHUNJO);
}

bool CHARACTER::IsNoAttackChunjo() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKCHUNJO);
}

void CHARACTER::SetNoAttackJinno() noexcept
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKJINNO);
}

bool CHARACTER::IsNoAttackJinno() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOATTACKJINNO);
}

void CHARACTER::SetAttackMob() noexcept
{
	SET_BIT(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB);
}

bool CHARACTER::IsAttackMob() const noexcept
{
	return IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_ATTACKMOB);
}

// STATE_IDLE_REFACTORING
void CHARACTER::StateIdle()
{
	if (IsStone())
	{
		__StateIdle_Stone();
		return;
	}
	else if (IsWarp() || IsGoto())
	{
		// Warps are treated as events
		m_dwStateDuration = 60 * passes_per_sec;
		return;
	}

	if (IsPC())
		return;

	// NPC handling
	if (!IsMonster())
	{
		__StateIdle_NPC();
		return;
	}

	__StateIdle_Monster();
}

void CHARACTER::__StateIdle_Stone()
{
	m_dwStateDuration = PASSES_PER_SEC(1);

	int iPercent = 0; // @fixme136
	if (GetMaxHP() >= 0)
		iPercent = (GetHP() * 100) / GetMaxHP();

	const uint32_t dwVnum = number(MIN(GetMobTable().sAttackSpeed, GetMobTable().sMovingSpeed), MAX(GetMobTable().sAttackSpeed, GetMobTable().sMovingSpeed));
	if (dwVnum == 0)
		return;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if (IsAffectFlag(AFF_DRAGONLAIR_STONE_UNBEATABLE_3))
		return;
#endif

	if (iPercent <= 10 && GetMaxSP() < 10)
	{
		SetMaxSP(10);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1500, GetY() - 1500, GetX() + 1500, GetY() + 1500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 20 && GetMaxSP() < 9)
	{
		SetMaxSP(9);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1500, GetY() - 1500, GetX() + 1500, GetY() + 1500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 30 && GetMaxSP() < 8)
	{
		SetMaxSP(8);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 40 && GetMaxSP() < 7)
	{
		SetMaxSP(7);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 50 && GetMaxSP() < 6)
	{
		SetMaxSP(6);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 60 && GetMaxSP() < 5)
	{
		SetMaxSP(5);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 70 && GetMaxSP() < 4)
	{
		SetMaxSP(4);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 80 && GetMaxSP() < 3)
	{
		SetMaxSP(3);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 90 && GetMaxSP() < 2)
	{
		SetMaxSP(2);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 500, GetY() - 500, GetX() + 500, GetY() + 500);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else if (iPercent <= 99 && GetMaxSP() < 1)
	{
		SetMaxSP(1);
		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0);

		CHARACTER_MANAGER::Instance().SelectStone(this);
		CHARACTER_MANAGER::Instance().SpawnGroup(dwVnum, GetMapIndex(), GetX() - 1000, GetY() - 1000, GetX() + 1000, GetY() + 1000);
		CHARACTER_MANAGER::Instance().SelectStone(nullptr);
	}
	else
		return;

	UpdatePacket();
	return;
}

void CHARACTER::__StateIdle_NPC()
{
	MonsterChat(MONSTER_CHAT_WAIT);
	m_dwStateDuration = PASSES_PER_SEC(5);

	// Idle processing of the pet system is handled by CPetActor::Update, not the state machine shared by almost all types of characters.
	if (IsPet()
#ifdef ENABLE_PROTO_RENEWAL
		|| IsPetPay()
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		|| IsGrowthPet()
#endif
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		|| IsHorse()
#endif
	)
	{
		return;
	}
	else if (IsGuardNPC())
	{
		if (!quest::CQuestManager::Instance().GetEventFlag("noguard"))
		{
			FuncFindGuardVictim f(this, 50000);

			if (GetSectree())
				GetSectree()->ForEachAround(f);

			LPCHARACTER victim = f.GetVictim();

			if (victim)
			{
				m_dwStateDuration = passes_per_sec / 2;

				if (CanBeginFight())
					BeginFight(victim);
			}
		}
	}
	else
	{
		if (GetRaceNum() == xmas::MOB_SANTA_VNUM) // Santa
		{
			if (get_dword_time() > m_dwPlayStartTime)
			{
				constexpr int next_warp_time = 2 * 1000; // 2 seconds

				m_dwPlayStartTime = get_dword_time() + next_warp_time;

				// Time has passed, so let's warp.
				/*
				* For santa
				const int WARP_MAP_INDEX_NUM = 4;
				static const long c_lWarpMapIndexs[WARP_MAP_INDEX_NUM] = {61, 62, 63, 64};
				*/
				// Shinseonja No Haeyong
				const int WARP_MAP_INDEX_NUM = 7;
				static const long c_lWarpMapIndexs[WARP_MAP_INDEX_NUM] = { 61, 62, 63, 64, 3, 23, 43 };
				long lNextMapIndex;
				lNextMapIndex = c_lWarpMapIndexs[number(1, WARP_MAP_INDEX_NUM) - 1];

				if (map_allow_find(lNextMapIndex))
				{
					// It is here.
					M2_DESTROY_CHARACTER(this);
					constexpr int iNextSpawnDelay = 50 * 60;

					xmas::SpawnSanta(lNextMapIndex, iNextSpawnDelay);
				}
				else
				{
					// Another server.
					TPacketGGXmasWarpSanta p{};
					p.bHeader = HEADER_GG_XMAS_WARP_SANTA;
					p.bChannel = g_bChannel;
					p.lMapIndex = lNextMapIndex;
					P2P_MANAGER::Instance().Send(&p, sizeof(TPacketGGXmasWarpSanta));
				}
				return;
			}
		}

		if (!IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
		{
			//
			// It moves from place to place.
			//
			LPCHARACTER pkChrProtege = GetProtege();

			if (pkChrProtege)
			{
				if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 500)
				{
					if (Follow(pkChrProtege, static_cast<float>(number(100, 300))))
						return;
				}
			}

			if (!number(0, 6))
			{
				SetRotation(static_cast<float>(number(0, 359))); // Direction is set at random

				float fx = 0.0f, fy = 0.0f;
				const auto fDist = static_cast<float>(number(200, 400));

				GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

				// Loose nailing property check; If you can't go between the final and intermediate positions, don't go.
				if (!(SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy))
					&& SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx) / 2, GetY() + static_cast<int>(fy) / 2)))
					return;

				SetNowWalking(true);

				if (Goto(GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy)))
					SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

				return;
			}
		}
	}
}

void CHARACTER::__StateIdle_Monster()
{
	if (IsStun())
		return;

	if (!CanMove())
		return;

	if (IsCoward())
	{
		// Cowardly monsters only run away.
		if (!IsDead())
			CowardEscape();

		return;
	}

	if (IsBerserker())
	{
		if (IsBerserk())
			SetBerserk(false);
	}

	if (IsGodSpeeder())
	{
		if (IsGodSpeed())
			SetGodSpeed(false);
	}

	LPCHARACTER victim = GetVictim();

	if (!victim || victim->IsDead())
	{
		SetVictim(nullptr);
		victim = nullptr;
		m_dwStateDuration = PASSES_PER_SEC(1);
	}

	if (!victim || victim->IsBuilding())
	{
		// stone protection treatment
		if (m_pkChrStone)
		{
			victim = m_pkChrStone->GetNearestVictim(m_pkChrStone);
		}
		// Preemptive monster treatment
		else if (!no_wander && IsAggressive())
		{
			// If there is a tree in Seohansan Mountain, it is not preempted.
			if (GetMapIndex() == 61 && quest::CQuestManager::Instance().GetEventFlag("xmas_tree"));
			else
			{
				victim = FindVictim(this, m_pkMobData->m_table.wAggressiveSight);
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
				if ((!victim) && (GetRaceNum() == MELEY_BOSS_VNUM))
					victim = FindVictim(this, 40000);
#endif
			}
		}
	}

	if (victim && !victim->IsDead())
	{
		if (CanBeginFight())
			BeginFight(victim);

		return;
	}

	if (IsAggressive() && !victim)
		m_dwStateDuration = PASSES_PER_SEC(number(1, 3));
	else
		m_dwStateDuration = PASSES_PER_SEC(number(3, 5));

	LPCHARACTER pkChrProtege = GetProtege();

	// If it is far from the thing to protect (stone, party leader), follow it.
	if (pkChrProtege)
	{
		if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 1000)
		{
			if (Follow(pkChrProtege, static_cast<float>(number(150, 400))))
			{
				MonsterLog("[IDLE] You're too far away from the leader! return");
				return;
			}
		}
	}

	//
	// just go back and forth
	//
	if (!no_wander && !IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
	{
		if (!number(0, 6))
		{
			SetRotation(static_cast<float>(number(0, 359))); // direction is set randomly

			float fx, fy;
			const auto fDist = static_cast<float>(number(300, 700));

			GetDeltaByDegree(GetRotation(), fDist, &fx, &fy);

			// Loose nail property check; If the final and intermediate positions cannot be reached, do not go.
			if (!(SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy))
				&& SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx) / 2, GetY() + static_cast<int>(fy) / 2)))
				return;

			// NOTE: When a monster wanders around in IDLE state, it is currently run unconditionally. (Never walk)
			// The graphics team wants to see the monsters walking, so they temporarily walk or run with a certain probability. (It only works once in test mode, because the overall feel of the game is wrong)
			if (test_server) // @warme010
			{
				if (number(0, 100) < 60)
					SetNowWalking(false);
				else
					SetNowWalking(true);
			}

			if (Goto(GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy)))
				SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

			return;
		}
	}

	MonsterChat(MONSTER_CHAT_WAIT);
}
// END_OF_STATE_IDLE_REFACTORING

bool __CHARACTER_GotoNearTarget(LPCHARACTER self, LPCHARACTER victim)
{
	if (!self)
		return false;

	if (IS_SET(self->GetAIFlag(), AIFLAG_NOMOVE))
		return false;

	switch (self->GetMobBattleType())
	{
		case BATTLE_TYPE_RANGE:
		case BATTLE_TYPE_MAGIC:
		{
			// The wizard or archer goes up to 80% of the attack distance and starts attacking.
			if (self->Follow(victim, static_cast<float>(self->GetMobAttackRange() * 8 / 10)))
				return true;
		}
		break;

		default:
			// 90% of the rest?
			if (self->Follow(victim, static_cast<float>(self->GetMobAttackRange() * 9 / 10)))
				return true;
	}

	return false;
}

void CHARACTER::StateMove()
{
	const uint32_t dwElapsedTime = get_dword_time() - m_dwMoveStartTime;
	float fRate = static_cast<float>(dwElapsedTime) / static_cast<float>(m_dwMoveDuration);

	if (fRate > 1.0f)
		fRate = 1.0f;

	const int x = static_cast<int>(static_cast<float>(m_posDest.x - m_posStart.x) * fRate + m_posStart.x);
	const int y = static_cast<int>(static_cast<float>(m_posDest.y - m_posStart.y) * fRate + m_posStart.y);

	Move(x, y);

	if (IsPC() && (thecore_pulse() & 15) == 0)
	{
		UpdateSectree();

		if (GetExchange())
		{
			const LPCHARACTER& victim = GetExchange()->GetCompany()->GetOwner();
			if (!victim)
				return;

			const int iDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

			// distance check
			if (iDist >= EXCHANGE_MAX_DISTANCE)
			{
				GetExchange()->Cancel();
			}
		}
	}

	// Stamina must be 0 or higher.
	if (IsPC())
	{
		if (IsWalking() && GetStamina() < GetMaxStamina())
		{
			// Stamina increases after 5 seconds
			if (get_dword_time() - GetWalkStartTime() > 5000)
				PointChange(POINT_STAMINA, GetMaxStamina() / 1);
		}

		// If you're running while you're in combat
		if (!IsWalking() && !IsRiding())
		{
			if ((get_dword_time() - GetLastAttackTime()) < 20000)
			{
				StartAffectEvent();

				if (IsStaminaHalfConsume())
				{
					if (thecore_pulse() & 1)
						PointChange(POINT_STAMINA, -STAMINA_PER_STEP);
				}
				else
					PointChange(POINT_STAMINA, -STAMINA_PER_STEP);

				StartStaminaConsume();

				if (GetStamina() <= 0)
				{
					// I don't have enough stamina so I have to walk
					SetStamina(0);
					SetNowWalking(true);
					StopStaminaConsume();
				}
			}
			else if (IsStaminaConsume())
			{
				StopStaminaConsume();
			}
		}
	}
	else
	{
		// XXX AGGRO
		if (IsMonster() && GetVictim())
		{
			LPCHARACTER victim = GetVictim();
			if (!victim)
				return;

			UpdateAggrPoint(victim, EDamageType::DAMAGE_TYPE_NORMAL, -(victim->GetLevel() / 3 + 1));

			if (test_server) // @warme010
			{
				// If the monster is chasing the enemy, it will run unconditionally.
				SetNowWalking(false);
			}
		}

		if (IsMonster() && GetMobRank() >= MOB_RANK_BOSS && GetVictim())
		{
			LPCHARACTER victim = GetVictim();
			if (!victim)
				return;

			// giant tortoise
			if (GetRaceNum() == 2191 && number(1, 20) == 1 && get_dword_time() - m_pkMobInst->m_dwLastWarpTime > 1000)
			{
				// warp test
				float fx, fy;
				GetDeltaByDegree(victim->GetRotation(), 400, &fx, &fy);
				const long new_x = victim->GetX() + static_cast<long>(fx);
				const long new_y = victim->GetY() + static_cast<long>(fy);
				SetRotation(GetDegreeFromPositionXY(new_x, new_y, victim->GetX(), victim->GetY()));
				Show(victim->GetMapIndex(), new_x, new_y, 0, true);
				GotoState(m_stateBattle);
				m_dwStateDuration = 1;
				ResetMobSkillCooltime();
				m_pkMobInst->m_dwLastWarpTime = get_dword_time();
				return;
			}

			// TODO Change direction and be less stupid!
			if (number(0, 3) == 0)
			{
				if (__CHARACTER_GotoNearTarget(this, victim))
					return;
			}
		}
	}

	if (1.0f == fRate)
	{
		if (IsPC())
		{
			sys_log(1, "arrival %s %d %d", GetName(), x, y);
			GotoState(m_stateIdle);
			StopStaminaConsume();
		}
		else
		{
			if (GetVictim() && !IsCoward())
			{
				if (!IsState(m_stateBattle))
					MonsterLog("[BATTLE] Coming close, start attacking %s", GetVictim()->GetName());

				GotoState(m_stateBattle);
				m_dwStateDuration = 1;
			}
			else
			{
				if (!IsState(m_stateIdle))
					MonsterLog("[IDLE] Let's rest because there is no target");

				GotoState(m_stateIdle);

				//LPCHARACTER rider = GetRider();

				m_dwStateDuration = PASSES_PER_SEC(number(1, 3));
			}
		}
	}
}

void CHARACTER::StateBattle()
{
	if (IsStone())
	{
		sys_err("Stone must not use battle state (name %s)", GetName());
		return;
	}

	if (IsPC())
		return;

	if (!CanMove())
		return;

	if (IsStun())
		return;

	LPCHARACTER victim = GetVictim();

	if (IsCoward())
	{
		if (IsDead())
			return;

		SetVictim(nullptr);

		if (number(1, 50) != 1)
		{
			GotoState(m_stateIdle);
			m_dwStateDuration = 1;
		}
		else
			CowardEscape();

		return;
	}

	if (!victim || (victim->IsStun() && IsGuardNPC()) || victim->IsDead())
	{
		if (victim && victim->IsDead() &&
			!no_wander && IsAggressive() && (!GetParty() || GetParty()->GetLeader() == this))
		{
			LPCHARACTER new_victim = FindVictim(this, m_pkMobData->m_table.wAggressiveSight);

			SetVictim(new_victim);
			m_dwStateDuration = PASSES_PER_SEC(1);

			if (!new_victim)
			{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
				if (IsMonster() && GetRaceNum() == MELEY_BOSS_VNUM)
					return;
#endif

				switch (GetMobBattleType())
				{
					case BATTLE_TYPE_MELEE:
					case BATTLE_TYPE_SUPER_POWER:
					case BATTLE_TYPE_SUPER_TANKER:
					case BATTLE_TYPE_POWER:
					case BATTLE_TYPE_TANKER:
						{
							float fx, fy;
							const float fDist = static_cast<float>(number(400, 1500));

							GetDeltaByDegree(static_cast<float>(number(0, 359)), fDist, &fx, &fy);

							if (SECTREE_MANAGER::Instance().IsMovablePosition(victim->GetMapIndex(),
								victim->GetX() + static_cast<int>(fx),
								victim->GetY() + static_cast<int>(fy)) &&
								SECTREE_MANAGER::Instance().IsMovablePosition(victim->GetMapIndex(),
									victim->GetX() + static_cast<int>(fx) / 2,
									victim->GetY() + static_cast<int>(fy) / 2))
							{
								const float dx = victim->GetX() + fx;
								const float dy = victim->GetY() + fy;

								SetRotation(GetDegreeFromPosition(dx, dy));

								if (Goto(static_cast<long>(dx), static_cast<long>(dy)))
								{
									sys_log(0, "KILL_AND_GO: %s distance %.1f", GetName(), fDist);
									SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
								}
							}
						}

						default:
							break;
				}
			}
			return;
		}

		SetVictim(nullptr);

		if (IsGuardNPC())
			Return();

		m_dwStateDuration = PASSES_PER_SEC(1);
		return;
	}

	//if (IsSummonMonster() && !IsDead() && !IsStun())
	if (CanSummonMonster() && !IsDead() && !IsStun())	//@custom014
	{
		if (!GetParty())
		{
			// Summon and make a party to fill.
			CPartyManager::Instance().CreateParty(this);
		}

		const auto pParty = GetParty();
		{
			const bool bPct = number(0, 3) == 0 ? true : false;

			if (bPct && pParty->CountMemberByVnum(GetSummonVnum()) < SUMMON_MONSTER_COUNT)
			{
				MonsterLog("Summon subordinate monsters!");
				// Let's call the guy named Hat and fill it up.
				const int sx = GetX() - 300;
				const int sy = GetY() - 300;
				const int ex = GetX() + 300;
				const int ey = GetY() + 300;

				const auto tch = CHARACTER_MANAGER::Instance().SpawnMobRange(GetSummonVnum(), GetMapIndex(), sx, sy, ex, ey, true, true);
				if (tch)
				{
					pParty->Join(tch->GetVID());
					pParty->Link(tch);
				}
			}
			else
				MarkSummonedMonster();	//@custom014
		}
	}

#ifdef ENABLE_DAWNMIST_DUNGEON
	if (CDawnMistDungeon::Instance().IsForestDungeonBoss(this) && !IsDead() && CDawnMistDungeon::Instance().CanSpawnHealerGroup(this))
		CDawnMistDungeon::Instance().SpawnHealerGroup(this);
#endif

	LPCHARACTER pkChrProtege = GetProtege();
	auto fDist = static_cast<float>(DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY()));
	float fDistCheck = 4000.0f;

/* #ifdef ENABLE_SUNG_MAHI_TOWER
	if (IsSungMahiDungeon(GetMapIndex()))
		fDistCheck = 16000.0f;
#endif */

	if (fDist >= fDistCheck) // Giving up if more than 40 meters away
	{
#if  defined(ENABLE_12ZI) || defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
		bool bPass = true;
#endif

#ifdef ENABLE_12ZI
		if (IsZodiacBoss())
			bPass = false;
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		if ((GetRaceNum() == MELEY_BOSS_VNUM) && (fDist < 16000.0f))
			bPass = false;
#endif

#if  defined(ENABLE_12ZI) || defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
		if (bPass)
#endif
		{
			MonsterLog("Giving up because the target is too far");
			SetVictim(nullptr);

			// Things to protect (stones, party halls) Go around.
			if (pkChrProtege)
			{
				if (DISTANCE_APPROX(GetX() - pkChrProtege->GetX(), GetY() - pkChrProtege->GetY()) > 1000)
					Follow(pkChrProtege, static_cast<float>(number(150, 400)));
			}

			return;
		}
	}

	if (fDist >= GetMobAttackRange() * 1.15)
	{
		__CHARACTER_GotoNearTarget(this, victim);
		return;
	}

	if (m_pkParty)
		m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);

	if (2493 == m_pkMobData->m_table.dwVnum)
	{
		// Water Dragon (2493) Special Treatment
		m_dwStateDuration = BlueDragon_StateBattle(this);
		return;
	}

	const uint32_t dwCurTime = get_dword_time();
	const uint32_t dwDuration = CalculateDuration(GetLimitPoint(POINT_ATT_SPEED), 2000);

	if ((dwCurTime - m_dwLastAttackTime) < dwDuration) // Water Dragon (2493) Special Treatment
	{
		m_dwStateDuration = MAX(1, (passes_per_sec * (dwDuration - (dwCurTime - m_dwLastAttackTime)) / 1000));
		return;
	}

	if (IsBerserker() == true)
	{
		if (GetHPPct() < m_pkMobData->m_table.bBerserkPoint)
		{
			if (IsBerserk() != true)
				SetBerserk(true);
		}
	}

	if (IsGodSpeeder() == true)
	{
		if (GetHPPct() < m_pkMobData->m_table.bGodSpeedPoint)
		{
			if (IsGodSpeed() != true)
				SetGodSpeed(true);
		}
	}

	//
	// mob skill handling
	//
	if (HasMobSkill())
	{
		for (uint32_t iSkillIdx = 0; iSkillIdx < MOB_SKILL_MAX_NUM; ++iSkillIdx)
		{
			if (CanUseMobSkill(iSkillIdx))
			{
				SetRotationToXY(victim->GetX(), victim->GetY());

#ifdef ENABLE_DAWNMIST_DUNGEON
				if (CDawnMistDungeon::Instance().IsForestDungeonBoss(this) && iSkillIdx == CDawnMistDungeon::SPECIAL_SKILL_INDEX)
					iSkillIdx += CDawnMistDungeon::Instance().GenerateBossSpecialSkillIndex(this);
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
				if ((GetRaceNum() == MELEY_BOSS_VNUM) && (CMeleyLairManager::Instance().IsMeleyMap(victim->GetMapIndex())))
				{
					const PIXEL_POSITION pos = CMeleyLairManager::Instance().GetXYZ();
					if (pos.x)
						SetRotationToXY(pos.x, pos.y);
				}
#endif

				if (UseMobSkill(iSkillIdx))
				{
					SendMovePacket(FUNC_MOB_SKILL, iSkillIdx, GetX(), GetY(), 0, dwCurTime);

					const auto fDuration = CMotionManager::Instance().GetMotionDuration(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_SPECIAL_1 + iSkillIdx));
					m_dwStateDuration = static_cast<uint32_t>(fDuration == 0.0f ? PASSES_PER_SEC(2) : PASSES_PER_SEC(fDuration));

					if (test_server)
						sys_log(0, "USE_MOB_SKILL: %s idx %u motion %u duration %.0f", GetName(), iSkillIdx, MOTION_SPECIAL_1 + iSkillIdx, fDuration);

					return;
				}
			}
		}
	}

#ifdef ENABLE_12ZI
	if (IsZodiacBoss())
	{
		if (GetLastZodiacAttackTime() + number(5, 15) < (uint32_t)get_global_time())
		{
			SetLastZodiacAttackTime(get_global_time());

			m_dwLastAttackTime = dwCurTime + (dwDuration - 1);

			uint8_t type = number(0, 2);
			ZodiacDamage(type, victim);
			return;
		}
	}

	if (IsZodiacCannon())
		return;
#endif

	if (!Attack(victim)) // What if the attack fails? Why did it fail? TODO
		m_dwStateDuration = passes_per_sec / 2;
	else
	{
		// Makes the enemy look
		SetRotationToXY(victim->GetX(), victim->GetY());

		SendMovePacket(FUNC_ATTACK, 0, GetX(), GetY(), 0, dwCurTime);

		const float fDuration = CMotionManager::Instance().GetMotionDuration(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, MOTION_NORMAL_ATTACK));
		m_dwStateDuration = static_cast<uint32_t>(fDuration == 0.0f ? PASSES_PER_SEC(2) : PASSES_PER_SEC(fDuration));
	}
}

void CHARACTER::StateFlag()
{
	m_dwStateDuration = static_cast<uint32_t>(PASSES_PER_SEC(0.5));

	CWarMap* pMap = GetWarMap();

	if (!pMap)
		return;

	FuncFindChrForFlag f(this);
	GetSectree()->ForEachAround(f);

	if (!f.m_pkChrFind)
		return;

	if (nullptr == f.m_pkChrFind->GetGuild())
		return;

	char buf[256];
	uint8_t idx;

	if (!pMap->GetTeamIndex(GetPoint(POINT_STAT), idx))
		return;

	f.m_pkChrFind->AddAffect(AFFECT_WAR_FLAG, POINT_NONE, GetPoint(POINT_STAT), idx == 0 ? AFF_WAR_FLAG1 : AFF_WAR_FLAG2, INFINITE_AFFECT_DURATION, 0, false);
	f.m_pkChrFind->AddAffect(AFFECT_WAR_FLAG, POINT_MOV_SPEED, 50 - f.m_pkChrFind->GetPoint(POINT_MOV_SPEED), 0, INFINITE_AFFECT_DURATION, 0, false);

	pMap->RemoveFlag(idx);

	snprintf(buf, sizeof(buf), LC_TEXT("[327]%s has captured the flag of %s!"), pMap->GetGuild(idx)->GetName(), f.m_pkChrFind->GetName());
	pMap->Notice(buf);
}

void CHARACTER::StateFlagBase()
{
	m_dwStateDuration = static_cast<uint32_t>(PASSES_PER_SEC(0.5));

	FuncFindChrForFlagBase f(this);
	GetSectree()->ForEachAround(f);
}

void CHARACTER::StateHorse()
{
	constexpr float START_FOLLOW_DISTANCE = 400.0f; // Start chasing if you fall more than this distance
	constexpr float START_RUN_DISTANCE = 700.0f; // If you fall more than this distance, run to chase.
	constexpr int MIN_APPROACH = 150; // min approach distance
	constexpr int MAX_APPROACH = 300; // max approach distance

	const auto STATE_DURATION = static_cast<uint32_t>(PASSES_PER_SEC(0.5)); // state duration

	constexpr bool bDoMoveAlone = true; // Whether to move around alone when close to the character -_-;
	constexpr bool bRun = true; // Should I run?

	if (IsDead())
		return;

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (!IsHorse())
		return;
#endif

	m_dwStateDuration = STATE_DURATION;

	LPCHARACTER victim = GetRider();

	// ! no // if there is no target, the summoner will clear me directly
	if (!victim)
	{
		M2_DESTROY_CHARACTER(this);
		return;
	}

	m_pkMobInst->m_posLastAttacked = GetXYZ();

	const auto fDist = static_cast<float>(DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY()));

	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if (fDist > START_RUN_DISTANCE)
			SetNowWalking(!bRun); // NOTE: I thought it would stop when I saw the function name, but when I use SetNowWalking(false) it starts running.. -_-;

		Follow(victim, static_cast<float>(number(MIN_APPROACH, MAX_APPROACH)));

		m_dwStateDuration = STATE_DURATION;
	}
	else if (bDoMoveAlone && (get_dword_time() > m_dwLastAttackTime))
	{
		// wondering-.-
		m_dwLastAttackTime = get_dword_time() + number(5000, 12000);

		SetRotation(static_cast<float>(number(0, 359))); // direction is set randomly

		float fx, fy;
		const float fDist2 = static_cast<float>(number(200, 400));

		GetDeltaByDegree(GetRotation(), fDist2, &fx, &fy);

		// Check for loose nail properties; If the final and intermediate positions cannot be reached, do not go.
		if (!(SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy))
			&& SECTREE_MANAGER::Instance().IsMovablePosition(GetMapIndex(), GetX() + static_cast<int>(fx) / 2, GetY() + static_cast<int>(fy) / 2)))
			return;

		SetNowWalking(true);

		if (Goto(GetX() + static_cast<int>(fx), GetY() + static_cast<int>(fy)))
			SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	}
}
