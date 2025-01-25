#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc.h"
#include "char.h"
#include "char_manager.h"
#include "battle.h"
#include "item.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "vector.h"
#include "packet.h"
#include "pvp.h"
#include "profiler.h"
#include "guild.h"
#include "affect.h"
#include "unique_item.h"
#include "lua_incl.h"
#include "arena.h"
#include "castle.h"
#include "sectree.h"
#include "ani.h"
#include "locale_service.h"
#ifdef ENABLE_12ZI
# include "zodiac_temple.h"
#endif

#ifdef ENABLE_MONSTER_BACK
#	include "minigame_manager.h"
#endif

#ifdef ENABLE_QUEEN_NETHIS
# include "SnakeLair.h"
#endif

#ifdef ENABLE_CHECK_WAIT_HACK
#	include "log.h"
#endif

int battle_hit(LPCHARACTER ch, LPCHARACTER victim, int& iRetDam);

bool battle_distance_valid_by_xy(const CHARACTER* ch, const CHARACTER * victim)
{
	if (!ch || !victim)
		return false;

	const auto distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());
	float fHitRange = 300;
#ifdef ENABLE_NEW_DISTANCE_CALC
	fHitRange += static_cast<int>(victim->GetMonsterHitRange());
#endif

	if (distance > fHitRange)
		return false;

	return true;
}

bool battle_distance_valid(const CHARACTER* ch, const CHARACTER* victim)
{
	if (!ch || !victim)
		return false;

	return battle_distance_valid_by_xy(ch, victim);
}

bool timed_event_cancel(LPCHARACTER ch)
{
	if (ch->m_pkTimedEvent)
	{
		event_cancel(&ch->m_pkTimedEvent);
		return true;
	}

	/* RECALL_DELAY
	Uncomment if the return delay has to be canceled due to future battles
	if (ch->m_pk_RecallEvent)
	{
		event_cancel(&ch->m_pkRecallEvent);
		return true;
	}
	END_OF_RECALL_DELAY */

	return false;
}

#ifdef NEW_ICEDAMAGE_SYSTEM
bool battle_is_icedamage(const CHARACTER* pAttacker, LPCHARACTER pVictim)
{
	if (!pVictim)
		return false;

	if (pAttacker && pAttacker->IsPC())
	{
		const auto race = pAttacker->GetRaceNum();
		const uint32_t tmp_dwNDRFlag = pVictim->GetNoDamageRaceFlag();
		if (tmp_dwNDRFlag &&
			(race < MAIN_RACE_MAX_NUM) &&
			(IS_SET(tmp_dwNDRFlag, 1 << race))
			)
		{
			return false;
		}
		const std::set<uint32_t>& tmp_setNDAFlag = pVictim->GetNoDamageAffectFlag();
		if (tmp_setNDAFlag.size())
		{
			for (std::set<uint32_t>::iterator it = tmp_setNDAFlag.begin(); it != tmp_setNDAFlag.end(); ++it)
			{
				if (!pAttacker->IsAffectFlag(*it))
				{
					return false;
				}
			}
		}
	}
	return true;
}
#endif

bool battle_is_attackable(LPCHARACTER ch, LPCHARACTER victim)
{
	if (!ch || !victim)
		return false;

	// If the opponent dies, stop.
	if (victim->IsDead() || victim->IsObserverMode())	//@fixme452
		return false;

#ifdef ENABLE_12ZI
	if (ch->GetMapIndex() == 358)
	{
		if (victim->IsPC() && ch->IsPC())
			return false;
	}
#endif

	// stop in safe zone
	{
		SECTREE* sectree = nullptr;

		sectree = ch->GetSectree();
		if (sectree && sectree->IsAttr(ch->GetX(), ch->GetY(), ATTR_BANPK))
			return false;

		sectree = victim->GetSectree();
		if (sectree && sectree->IsAttr(victim->GetX(), victim->GetY(), ATTR_BANPK))
			return false;
	}

#ifdef NEW_ICEDAMAGE_SYSTEM
	if (!battle_is_icedamage(ch, victim))
		return false;
#endif

	// abort if i die
	if (ch->IsStun() || ch->IsDead() || ch->IsObserverMode())	//@fixme452
		return false;

	if (ch->IsPC() && victim->IsPC())
	{
		CGuild* g1 = ch->GetGuild();
		const CGuild* g2 = victim->GetGuild();

		if (g1 && g2)
		{
			if (g1->UnderWar(g2->GetID()))
				return true;
		}
	}

	if (IS_CASTLE_MAP(ch->GetMapIndex()) && !castle_can_attack(ch, victim))
		return false;

	if (CArenaManager::Instance().CanAttack(ch, victim))
		return true;

#ifdef ENABLE_DEFENSE_WAVE
	if (victim->IsMast() && ch->IsMonster())
		return true;
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	if (ch->IsPC() && ch->IsSungmaMap() && !(victim->IsPC()))
		if (ch->GetConquerorLevel() == 0)
			return false;
#endif

	return CPVPManager::Instance().CanAttack(ch, victim);
}

int battle_melee_attack(LPCHARACTER ch, LPCHARACTER victim)
{
	if (!ch || !victim)
		return BATTLE_NONE;

	if (test_server && ch->IsPC())
		sys_log(0, "battle_melee_attack : [%s] attack to [%s]", ch->GetName(), victim->GetName());

	if (!victim || ch == victim)
		return BATTLE_NONE;

	if (test_server && ch->IsPC())
		sys_log(0, "battle_melee_attack : [%s] attack to [%s]", ch->GetName(), victim->GetName());

	if (!battle_is_attackable(ch, victim))
		return BATTLE_NONE;

	if (test_server && ch->IsPC())
		sys_log(0, "battle_melee_attack : [%s] attack to [%s]", ch->GetName(), victim->GetName());

	// check distance
	const auto distance = DISTANCE_APPROX(ch->GetX() - victim->GetX(), ch->GetY() - victim->GetY());

	if (!victim->IsBuilding())
	{
		int max = battle_get_max_distance(ch, victim);	//@fixme499

		if (!ch->IsPC())
		{
			// For monsters, use the monster attack distance
			max = static_cast<int>(ch->GetMobAttackRange() * 1.15f);
		}
		else
		{
			// On PC, if the opponent is a melee mob, the mob's attack distance is the maximum attack distance
			if (!victim->IsPC() && BATTLE_TYPE_MELEE == victim->GetMobBattleType())
				max = MAX(max, static_cast<int>(victim->GetMobAttackRange() * 1.15f));
		}

		if (distance > max)
		{
			if (test_server)
				sys_log(0, "VICTIM_FAR: %s distance: %d max: %d", ch->GetName(), distance, max);

			return BATTLE_NONE;
		}
	}

	if (timed_event_cancel(ch))
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;485]");

	if (timed_event_cancel(victim))
		victim->ChatPacket(CHAT_TYPE_INFO, "[LS;485]");

	ch->SetPosition(POS_FIGHTING);
	ch->SetVictim(victim);

	const PIXEL_POSITION& vpos = victim->GetXYZ();
	ch->SetRotationToXY(vpos.x, vpos.y);

	int dam;
	const auto ret = battle_hit(ch, victim, dam);
	return (ret);
}

//@fixme499
int battle_get_max_distance(const LPCHARACTER& ch, const LPCHARACTER& victim)
{
	int max = 300;

	if (!ch || !victim)
		return max;

	if (victim->IsMonster() || victim->IsStone())
		max = (max / 100) * static_cast<int>(victim->GetMonsterHitRange());

	if (ch->IsRiding())
		max += 100;

	return max;
}
//@end_fixme499

// Make the actual GET_BATTLE_VICTIM NULL and cancel the event.
void battle_end_ex(LPCHARACTER ch)
{
	if (ch && ch->IsPosition(POS_FIGHTING))
		ch->SetPosition(POS_STANDING);
}

void battle_end(LPCHARACTER ch)
{
	if (!ch)
		return;

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	ch->StopHitBuffElementEvent();
#endif
	battle_end_ex(ch);
}

// AG = Attack Grade
// AL = Attack Limit
int CalcBattleDamage(int iDam, int iAttackerLev, int iVictimLev) noexcept
{
	if (iDam < 3)
		iDam = number(1, 5);

	//return CALCULATE_DAMAGE_LVDELTA(iAttackerLev, iVictimLev, iDam);
	return iDam;
}

int CalcMagicDamageWithValue(int iDam, const CHARACTER* pkAttacker, const CHARACTER* pkVictim) noexcept
{
	if (!pkAttacker || !pkVictim)
		return 0;

	return CalcBattleDamage(iDam, pkAttacker->GetLevel(), pkVictim->GetLevel());
}

int CalcMagicDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
{
	if (!pkAttacker)
		return 0;

	int iDam = 0;

	if (pkAttacker->IsNPC())
	{
		iDam = CalcMeleeDamage(pkAttacker, pkVictim, false, false);
	}

	iDam += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);

	return CalcMagicDamageWithValue(iDam, pkAttacker, pkVictim);
}

float CalcAttackRating(const CHARACTER* pkAttacker, const CHARACTER* pkVictim, bool bIgnoreTargetRating)
{
	if (!pkAttacker || !pkVictim)
		return 0.0f;

	int iARSrc = 0;
	int iERSrc = 0;

	{
		const auto attacker_dx = pkAttacker->GetPolymorphPoint(POINT_DX);
		const auto attacker_lv = pkAttacker->GetLevel();

		const auto victim_dx = pkVictim->GetPolymorphPoint(POINT_DX);
		const auto victim_lv = pkAttacker->GetLevel();

		iARSrc = MIN(90, (attacker_dx * 4 + attacker_lv * 2) / 6);
		iERSrc = MIN(90, (victim_dx * 4 + victim_lv * 2) / 6);
	}

	const float fAR = static_cast<float>(iARSrc + 210.0f) / 300.0f; // fAR = 0.7 ~ 1.0

	if (bIgnoreTargetRating)
		return fAR;

	// ((Edx * 2 + 20) / (Edx + 110)) * 0.3
	const float fER = (static_cast<float>(iERSrc * 2 + 5) / (iERSrc + 95)) * 3.0f / 10.0f;

	return fAR - fER;
}

int CalcAttBonus(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int iAtk)
{
	if (!pkAttacker || !pkVictim)
		return 0;

	// Does not apply to PvP
	if (!pkVictim->IsPC())
		iAtk += pkAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_ATTACK_BONUS);

	// Does not apply to PvP
	if (!pkAttacker->IsPC())
	{
		const auto iReduceDamagePct = pkVictim->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_TRANSFER_DAMAGE);
		iAtk = iAtk * (100 + iReduceDamagePct) / 100;
	}

	if (pkAttacker->IsNPC() && pkVictim->IsPC())
	{
		iAtk = (iAtk * CHARACTER_MANAGER::Instance().GetMobDamageRate(pkAttacker)) / 100;
	}

#if defined(ENABLE_PENDANT) || defined(ENABLE_ELEMENT) || defined(ENABLE_REFINE_ELEMENT)
	if (pkAttacker->IsPC())
		iAtk = iAtk * (100 + ((pkAttacker->GetPointElementSum() / 10) / 2)) / 100;
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	/*
	* NOTE : This was changed following the last wiki information:
	* https://fr-wiki.metin2.gameforge.com/index.php/Si_vous_avez_plus_de_90%25_de_PV,_les_d%C3%A9g%C3%A2ts_que_vous_subissez_sont_r%C3%A9duits_(-1%25,_-2%25,_-3%25,_-4%25,_-5%25)
	*/
	if (pkVictim->IsPC() && (pkVictim->GetHP() >= (pkVictim->GetMaxHP() * 0.9)))
		iAtk -= (iAtk * pkVictim->GetPoint(POINT_MORE_THEN_HP90_DAMAGE_REDUCE)) / 100;
#endif

	if (pkVictim->IsNPC())
	{
		/*
		* NOTE : This was changed following the last wiki information:
		* https://fr-wiki.metin2.gameforge.com/index.php/Force_contre_les_animaux_(%2BX%25_de_la_valeur_totale)
		*/

		if (pkVictim->IsRaceFlag(RACE_FLAG_ANIMAL))
		{
			int iAttAnimalTotal = pkAttacker->GetPoint(POINT_ATTBONUS_ANIMAL);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Animals (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_ANIMAL) > 0)
				iAttAnimalTotal = iAttAnimalTotal + (iAttAnimalTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_ANIMAL)) / 100;
#endif
			iAtk += (iAtk * iAttAnimalTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_UNDEAD))
		{
			int iAttUndeadTotal = pkAttacker->GetPoint(POINT_ATTBONUS_UNDEAD);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Undead (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_UNDEAD) > 0)
				iAttUndeadTotal = iAttUndeadTotal + (iAttUndeadTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_UNDEAD)) / 100;
#endif
			iAtk += (iAtk * iAttUndeadTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_DEVIL))
		{
			int iAttDevilTotal = pkAttacker->GetPoint(POINT_ATTBONUS_DEVIL);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Devils (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_DEVIL) > 0)
				iAttDevilTotal = iAttDevilTotal + (iAttDevilTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_DEVIL)) / 100;
#endif
			iAtk += (iAtk * iAttDevilTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_HUMAN))
		{
			int iAttHumanTotal = pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Half Humans (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_HUMAN) > 0)
				iAttHumanTotal = iAttHumanTotal + (iAttHumanTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_HUMAN)) / 100;
#endif
			iAtk += (iAtk * iAttHumanTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_ORC))
		{
			int iAttOrcTotal = pkAttacker->GetPoint(POINT_ATTBONUS_ORC);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Orcs (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_ORC) > 0)
				iAttOrcTotal = iAttOrcTotal + (iAttOrcTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_ORC)) / 100;
#endif
			iAtk += (iAtk * iAttOrcTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_MILGYO))
		{
			int iAttMilgyoTotal = pkAttacker->GetPoint(POINT_ATTBONUS_MILGYO);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Mystics (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_MILGYO) > 0)
				iAttMilgyoTotal = iAttMilgyoTotal + (iAttMilgyoTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_MILGYO)) / 100;
#endif
			iAtk += (iAtk * iAttMilgyoTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_INSECT))
		{
			int iAttInsectTotal = pkAttacker->GetPoint(POINT_ATTBONUS_INSECT);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Insects (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_INSECT) > 0)
				iAttInsectTotal = iAttInsectTotal + (iAttInsectTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_INSECT)) / 100;
#endif
			iAtk += (iAtk * iAttInsectTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_DESERT))
		{
			int iAttDesertTotal = pkAttacker->GetPoint(POINT_ATTBONUS_DESERT);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Desert Monsters (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_DESERT) > 0)
				iAttDesertTotal = iAttDesertTotal + (iAttDesertTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_DESERT)) / 100;
#endif
			iAtk += (iAtk * iAttDesertTotal) / 100;
		}

		if (pkVictim->IsRaceFlag(RACE_FLAG_TREE))
			iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_TREE)) / 100;

#if defined(ENABLE_ELEMENT_ADD) || defined(ENABLE_12ZI)
		if (pkVictim->IsRaceFlag(RACE_FLAG_ATT_CZ))
		{
			int iAttZodiacTotal = pkAttacker->GetPoint(POINT_ATTBONUS_CZ);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Strong against Zodiac Monsters (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_CZ) > 0)
				iAttZodiacTotal = iAttZodiacTotal + (iAttZodiacTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_CZ)) / 100;
#endif
			iAtk += (iAtk * iAttZodiacTotal) / 100;
		}
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		if (pkVictim->IsStone())
		{
			// +x% strength against Metin stones
			int iAttStoneTotal = pkAttacker->GetPoint(POINT_ATTBONUS_STONE);

			// Strong against Metin Stones (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_STONE) > 0)
				iAttStoneTotal = iAttStoneTotal + (iAttStoneTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_STONE)) / 100;

			iAtk += (iAtk * iAttStoneTotal) / 100;
		}

		// +x% attack damage against bosses
		if ((pkVictim->GetMobRank() >= MOB_RANK_BOSS) && (!pkVictim->IsStone()))
			iAtk += (iAtk * pkAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE)) / 100;
#endif

		int iAttMonsterTotal = pkAttacker->GetPoint(POINT_ATTBONUS_MONSTER);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		// Strength against Monsters (+x% of total value)
		if (pkAttacker->GetPoint(POINT_ATTBONUS_PER_MONSTER) > 0)
			iAttMonsterTotal = iAttMonsterTotal + (iAttMonsterTotal * pkAttacker->GetPoint(POINT_ATTBONUS_PER_MONSTER)) / 100;
#endif
		iAtk += (iAtk * iAttMonsterTotal) / 100;

#ifdef ENABLE_SHAM_ASSA_ATT_BONUS
		if (pkAttacker->IsPC() && (pkAttacker->GetJob() == JOB_ASSASSIN || pkAttacker->GetJob() == JOB_SHAMAN))
		{
#ifdef ENABLE_SHAM_ASSA_ATT_BONUS_FOR_VIP
			if (pkAttacker->GetGMLevel() != GM_VIP) {
				pkAttacker->ChatPacket(CHAT_TYPE_INFO, "NO_VIP_PERMISSIONS");
				return;
			}
#endif
			iAtk += (iAtk * SHAM_ASSA_ATT_BONUS) / 100;
		}
#endif
#ifdef ENABLE_KK_ATT_BONUS_AGAINST_NH
		int VALUE_ATT_WARRIOR = pkAttacker->GetPoint(POINT_ATTBONUS_WARRIOR);
		if (pkAttacker->GetMapIndex() == MAP_BATTLEFIELD)
		{
#ifdef ENABLE_KK_ATT_BONUS_AGAINST_NH_FOR_VIP
			if (pkAttacker->GetGMLevel() != GM_VIP) {
				pkAttacker->ChatPacket(CHAT_TYPE_INFO, "NO_VIP_PERMISSIONS");
				return;
			}
#endif
			if (pkAttacker->GetJob() == JOB_WARRIOR && pkAttacker->GetSkillGroup() == 1 && pkVictim->GetSkillGroup() == 2)
				VALUE_ATT_WARRIOR += KK_ATT_BONUS_AGAINST_NH;
		}
		iAtk += (iAtk * VALUE_ATT_WARRIOR) / 100;
#endif
	}
	else if (pkVictim->IsPC())
	{
#ifdef ENABLE_PENDANT
		int iResistHuman = pkVictim->GetPoint(POINT_RESIST_HUMAN);
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		// Resistance to Half Humans (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_HUMAN) > 0)
			iResistHuman = iResistHuman + (iResistHuman * pkVictim->GetPoint(POINT_RESIST_PER_HUMAN)) / 100;
#endif

		iAtk += (iAtk * (pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN) - iResistHuman)) / 100;
#else
		iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_HUMAN)) / 100;
#endif

		switch (pkVictim->GetJob())
		{
			case JOB_WARRIOR:
				iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_WARRIOR)) / 100;
				break;

			case JOB_ASSASSIN:
				iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_ASSASSIN)) / 100;
				break;

			case JOB_SURA:
				iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SURA)) / 100;
				break;

			case JOB_SHAMAN:
				iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_SHAMAN)) / 100;
				break;

#ifdef ENABLE_WOLFMAN_CHARACTER
			case JOB_WOLFMAN:
				iAtk += (iAtk * pkAttacker->GetPoint(POINT_ATTBONUS_WOLFMAN)) / 100;
				break;
#endif

			default:
				break;
		}
	}

	if (pkAttacker->IsPC())
	{
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		/*
		* NOTE : This was changed following the last wiki information:
		* https://fr-wiki.metin2.gameforge.com/index.php/Avec_un_nombre_de_points_de_grade_plus_faible,_les_d%C3%A9g%C3%A2ts_inflig%C3%A9s_sont_augment%C3%A9s
		*/
		if ((pkAttacker->GetPoint(POINT_ALIGNMENT_DAMAGE_BONUS) > 0) && (pkAttacker->GetAlignRank() > PVP_LEVEL_NEUTRAL))
		{
			uint8_t iAlignmentDamage = 0;
			switch (pkAttacker->GetAlignRank())
			{
				case PVP_LEVEL_AGGRESSIVE:
					iAlignmentDamage = 1;
					break;

				case PVP_LEVEL_FRAUDULENT:
					iAlignmentDamage = 2;
					break;

				case PVP_LEVEL_MALICIOUS:
					iAlignmentDamage = 3;
					break;

				case PVP_LEVEL_CRUEL:
					iAlignmentDamage = 5;
					break;

				default:
					break;
			}

			iAtk += (iAtk * iAlignmentDamage) / 100;
		}
#endif

		switch (pkAttacker->GetJob())
		{
			case JOB_WARRIOR:
				iAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_WARRIOR)) / 100;
				break;

			case JOB_ASSASSIN:
				iAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_ASSASSIN)) / 100;
				break;

			case JOB_SURA:
				iAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_SURA)) / 100;
				break;

			case JOB_SHAMAN:
				iAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_SHAMAN)) / 100;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case JOB_WOLFMAN:
				iAtk -= (iAtk * pkVictim->GetPoint(POINT_RESIST_WOLFMAN)) / 100;
				break;
#endif
			default:
				break;
		}
	}

	if (pkAttacker->IsNPC() && pkVictim->IsPC())
	{
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		// Attack damage from bosses reduce
		if ((pkAttacker->GetMobRank() >= MOB_RANK_BOSS) && (!pkAttacker->IsStone()))
			iAtk -= (iAtk * pkVictim->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE)) / 100;

		// Resistance to monster attacks
		iAtk -= (iAtk * pkVictim->GetPoint(POINT_MONSTER_DEFEND_BONUS)) / 100;
#endif

		/*
		* Schyck
		* [mob -> PC] Elemental property resistance applied
		* 2021/05/08
		* Resistance is applied from mob_proto (att_xxx -> gameforge tables)
		* Resistance is applied only to a value equal to bPercDmgVal% of the monster attack damage
		*/
		constexpr uint8_t bPercDmgVal = 75; // Tested and is like 99% similar in gameforge!

		int iResistElec = pkVictim->GetPoint(POINT_RESIST_ELEC);
		int iResistFire = pkVictim->GetPoint(POINT_RESIST_FIRE);
		int iResistIce = pkVictim->GetPoint(POINT_RESIST_ICE);
		int iResistWind = pkVictim->GetPoint(POINT_RESIST_WIND);
		int iResistEarth = pkVictim->GetPoint(POINT_RESIST_EARTH);
		int iResistDark = pkVictim->GetPoint(POINT_RESIST_DARK);

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
		// Lightning Resistance (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_ELEC) > 0)
			iResistElec = iResistElec + (iResistElec * pkVictim->GetPoint(POINT_RESIST_PER_ELEC)) / 100;

		// Fire Resistance (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_FIRE) > 0)
			iResistFire = iResistFire + (iResistFire * pkVictim->GetPoint(POINT_RESIST_PER_FIRE)) / 100;

		// Ice Resistance (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_ICE) > 0)
			iResistIce = iResistIce + (iResistIce * pkVictim->GetPoint(POINT_RESIST_PER_ICE)) / 100;

		// Wind Resistance (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_WIND) > 0)
			iResistWind = iResistWind + (iResistWind * pkVictim->GetPoint(POINT_RESIST_PER_WIND)) / 100;

		// Earth Resistance (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_EARTH) > 0)
			iResistEarth = iResistEarth + (iResistEarth * pkVictim->GetPoint(POINT_RESIST_PER_EARTH)) / 100;

		// Darkness Resistance (+x% of total value)
		if (pkVictim->GetPoint(POINT_RESIST_PER_DARK) > 0)
			iResistDark = iResistDark + (iResistDark * pkVictim->GetPoint(POINT_RESIST_PER_DARK)) / 100;
#endif

		if (pkAttacker->GetMobAttElement(MOB_ELEMENTAL_ELEC))
			iAtk -= (iAtk * bPercDmgVal * iResistElec) / 10000;
		else if (pkAttacker->GetMobAttElement(MOB_ELEMENTAL_FIRE))
			iAtk -= (iAtk * bPercDmgVal * iResistFire) / 10000;
		else if (pkAttacker->GetMobAttElement(MOB_ELEMENTAL_ICE))
			iAtk -= (iAtk * bPercDmgVal * iResistIce) / 10000;
		else if (pkAttacker->GetMobAttElement(MOB_ELEMENTAL_WIND))
			iAtk -= (iAtk * bPercDmgVal * iResistWind) / 10000;
		else if (pkAttacker->GetMobAttElement(MOB_ELEMENTAL_EARTH))
			iAtk -= (iAtk * bPercDmgVal * iResistEarth) / 10000;
		else if (pkAttacker->GetMobAttElement(MOB_ELEMENTAL_DARK))
			iAtk -= (iAtk * bPercDmgVal * iResistDark) / 10000;
	}

	return iAtk;
}

void Item_GetDamage(LPITEM pkItem, int* pdamMin, int* pdamMax)
{
	if (!pdamMin || !pdamMax)
		return;

	*pdamMin = 0;
	*pdamMax = 1;

	if (!pkItem)
		return;

	switch (pkItem->GetType())
	{
		case ITEM_ROD:
		case ITEM_PICK:
			return;

		default:
			break;
	}

	if (pkItem->GetType() != ITEM_WEAPON)
		sys_err("Item_GetDamage - !ITEM_WEAPON vnum=%d, type=%d", pkItem->GetOriginalVnum(), pkItem->GetType());

	auto damMin = pkItem->GetValue(3);
	auto damMax = pkItem->GetValue(4);

#ifdef ENABLE_YOHARA_SYSTEM
	if (pkItem->ItemHasRandomDefaultAttr())
	{
		damMin = pkItem->GetRandomDefaultAttr(2);
		damMax = pkItem->GetRandomDefaultAttr(3);
	}
#endif

	*pdamMin = damMin;
	*pdamMax = damMax;
}

int CalcMeleeDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, bool bIgnoreDefense, bool bIgnoreTargetRating)
{
	LPITEM pWeapon = pkAttacker->GetWear(WEAR_WEAPON);
	const auto bPolymorphed = pkAttacker->IsPolymorphed();

	if (pWeapon && !(bPolymorphed && !pkAttacker->IsPolyMaintainStat()))
	{
		if (pWeapon->GetType() != ITEM_WEAPON)
			return 0;

		switch (pWeapon->GetSubType())
		{
			case WEAPON_SWORD:
			case WEAPON_DAGGER:
			case WEAPON_TWO_HANDED:
			case WEAPON_BELL:
			case WEAPON_FAN:
			case WEAPON_MOUNT_SPEAR:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case WEAPON_CLAW:
#endif
				break;

			case WEAPON_BOW:
				sys_err("CalcMeleeDamage should not handle bows (name: %s)", pkAttacker->GetName());
				return 0;

			default:
				return 0;
		}
	}

	int iDam = 0;
	const auto fAR = CalcAttackRating(pkAttacker, pkVictim, bIgnoreTargetRating);
	int iDamMin = 0, iDamMax = 0;

	// TESTSERVER_SHOW_ATTACKINFO
	int DEBUG_iDamCur = 0;
	int DEBUG_iDamBonus = 0;
	// END_OF_TESTSERVER_SHOW_ATTACKINFO

	if (bPolymorphed && !pkAttacker->IsPolyMaintainStat())
	{
		// MONKEY_ROD_ATTACK_BUG_FIX
		Item_GetDamage(pWeapon, &iDamMin, &iDamMax);
		// END_OF_MONKEY_ROD_ATTACK_BUG_FIX

		const auto dwMobVnum = pkAttacker->GetPolymorphVnum();
		const CMob* pMob = CMobManager::Instance().Get(dwMobVnum);

		if (pMob)
		{
			const auto iPower = pkAttacker->GetPolymorphPower();
			iDamMin += pMob->m_table.dwDamageRange[0] * iPower / 100;
			iDamMax += pMob->m_table.dwDamageRange[1] * iPower / 100;
		}
	}
	else if (pWeapon)
	{
		// MONKEY_ROD_ATTACK_BUG_FIX
		Item_GetDamage(pWeapon, &iDamMin, &iDamMax);
		// END_OF_MONKEY_ROD_ATTACK_BUG_FIX
	}
	else if (pkAttacker->IsNPC())
	{
		iDamMin = pkAttacker->GetMobDamageMin();
		iDamMax = pkAttacker->GetMobDamageMax();
	}

	iDam = number(iDamMin, iDamMax) * 2;

	// TESTSERVER_SHOW_ATTACKINFO
	DEBUG_iDamCur = iDam;
	// END_OF_TESTSERVER_SHOW_ATTACKINFO
	//
	int iAtk = 0;

	// level must be ignored when multiply by fAR, so subtract it before calculation.
	iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) + iDam - (pkAttacker->GetLevel() * 2);
	iAtk = static_cast<int>(iAtk * fAR);
	iAtk += pkAttacker->GetLevel() * 2; // and add again

	if (pWeapon)
	{
		iAtk += pWeapon->GetValue(5) * 2;

		// 2004.11.12.myevan.TESTSERVER_SHOW_ATTACKINFO
		DEBUG_iDamBonus = pWeapon->GetValue(5) * 2;
		///////////////////////////////////////////////
	}

	iAtk += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS); // party attacker role bonus
	iAtk = iAtk * (100 + (pkAttacker->GetPoint(POINT_ATT_BONUS) + pkAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100;

	iAtk = CalcAttBonus(pkAttacker, pkVictim, iAtk);

	int iDef = 0;

	if (!bIgnoreDefense)
	{
		iDef = (pkVictim->GetPoint(POINT_DEF_GRADE) * (100 + pkVictim->GetPoint(POINT_DEF_BONUS)) / 100);

		if (!pkAttacker->IsPC())
			iDef += pkVictim->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_DEFENSE_BONUS);
	}

	if (pkAttacker->IsNPC())
		iAtk = static_cast<int>(iAtk * pkAttacker->GetMobDamageMultiply());

	iDam = MAX(0, iAtk - iDef);

	if (test_server)
	{
		const auto DEBUG_iLV = pkAttacker->GetLevel() * 2;
		const auto DEBUG_iST = static_cast<int>((pkAttacker->GetPoint(POINT_ATT_GRADE) - DEBUG_iLV) * fAR);
		const auto DEBUG_iPT = pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);
		int DEBUG_iWP = 0;
		int DEBUG_iPureAtk = 0;
		int DEBUG_iPureDam = 0;
		char szRB[32] = "";
		char szGradeAtkBonus[32] = "";

		DEBUG_iWP = static_cast<int>(DEBUG_iDamCur * fAR);
		DEBUG_iPureAtk = DEBUG_iLV + DEBUG_iST + DEBUG_iWP + DEBUG_iDamBonus;
		DEBUG_iPureDam = iAtk - iDef;

		if (pkAttacker->IsNPC())
		{
			snprintf(&szGradeAtkBonus[0], sizeof(szGradeAtkBonus), "=%d*%.1f", DEBUG_iPureAtk, pkAttacker->GetMobDamageMultiply());
			DEBUG_iPureAtk = static_cast<int>(DEBUG_iPureAtk * pkAttacker->GetMobDamageMultiply());
		}

		if (DEBUG_iDamBonus != 0)
			snprintf(&szRB[0], sizeof(szRB), "+RB(%d)", DEBUG_iDamBonus);

		char szPT[32] = "";

		if (DEBUG_iPT != 0)
			snprintf(&szPT[0], sizeof(szPT), ", PT=%d", DEBUG_iPT);

		char szUnknownAtk[32] = "";

		if (iAtk != DEBUG_iPureAtk)
			snprintf(&szUnknownAtk[0], sizeof(szUnknownAtk), "+?(%d)", iAtk - DEBUG_iPureAtk);

		char szUnknownDam[32] = "";

		if (iDam != DEBUG_iPureDam)
			snprintf(&szUnknownDam[0], sizeof(szUnknownDam), "+?(%d)", iDam - DEBUG_iPureDam);

		char szMeleeAttack[128] = "";

		snprintf(&szMeleeAttack[0], sizeof(szMeleeAttack),
			"%s(%d)-%s(%d)=%d%s, ATK=LV(%d)+ST(%d)+WP(%d)%s%s%s, AR=%.3g%s",
			pkAttacker->GetName(),
			iAtk,
			pkVictim->GetName(),
			iDef,
			iDam,
			&szUnknownDam[0],
			DEBUG_iLV,
			DEBUG_iST,
			DEBUG_iWP,
			&szRB[0],
			&szUnknownAtk[0],
			&szGradeAtkBonus[0],
			fAR,
			&szPT[0]);

		pkAttacker->ChatPacket(CHAT_TYPE_TALKING, "%s", &szMeleeAttack[0]);
		pkVictim->ChatPacket(CHAT_TYPE_TALKING, "%s", &szMeleeAttack[0]);
	}

#ifdef ENABLE_12ZI
	if (pkAttacker->IsMonster() && CZodiacManager::Instance().IsZiStageMapIndex(pkAttacker->GetMapIndex()))
	{
		if (pkAttacker->GetLevel() > 85)
			iDam += pkAttacker->GetLevel() * 8;
		else
			iDam += pkAttacker->GetLevel() * 5;
	}
#endif

	return CalcBattleDamage(iDam, pkAttacker->GetLevel(), pkVictim->GetLevel());
}

int CalcArrowDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, LPITEM pkBow, LPITEM pkArrow, bool bIgnoreDefense)
{
	if (!pkAttacker || !pkVictim)
		return 0;

	if (!pkBow || pkBow->GetType() != ITEM_WEAPON || pkBow->GetSubType() != WEAPON_BOW)
		return 0;

	if (!pkArrow)
		return 0;

	// hit calculator
	const auto iDist = static_cast<int>(DISTANCE_SQRT(pkAttacker->GetX() - pkVictim->GetX(), pkAttacker->GetY() - pkVictim->GetY()));
	//int iGap = (iDist / 100) - 5 - pkBow->GetValue(5) - pkAttacker->GetPoint(POINT_BOW_DISTANCE);
	const auto iGap = (iDist / 100) - 5 - pkAttacker->GetPoint(POINT_BOW_DISTANCE);
	auto iPercent = 100 - (iGap * 5);

#ifdef ENABLE_QUIVER_SYSTEM
	if (pkArrow->GetSubType() == WEAPON_QUIVER)
		iPercent = 100;
#endif

	if (iPercent <= 0)
		return 0;
	else if (iPercent > 100)
		iPercent = 100;

	int iDam = 0;

	const auto fAR = CalcAttackRating(pkAttacker, pkVictim, false);
	iDam = number(pkBow->GetValue(3), pkBow->GetValue(4)) * 2 + pkArrow->GetValue(3);
	int iAtk;

	// level must be ignored when multiply by fAR, so subtract it before calculation.
	iAtk = pkAttacker->GetPoint(POINT_ATT_GRADE) + iDam - (pkAttacker->GetLevel() * 2);
	iAtk = static_cast<int>(iAtk * fAR);
	iAtk += pkAttacker->GetLevel() * 2; // and add again

	// Refine Grade
	iAtk += pkBow->GetValue(5) * 2;

	iAtk += pkAttacker->GetPoint(POINT_PARTY_ATTACKER_BONUS);
	iAtk = iAtk * (100 + (pkAttacker->GetPoint(POINT_ATT_BONUS) + pkAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100;

	iAtk = CalcAttBonus(pkAttacker, pkVictim, iAtk);

	int iDef = 0;

	if (!bIgnoreDefense)
		iDef = (pkVictim->GetPoint(POINT_DEF_GRADE) * (100 + pkAttacker->GetPoint(POINT_DEF_BONUS)) / 100);

	if (pkAttacker->IsNPC())
		iAtk = static_cast<int>(iAtk * pkAttacker->GetMobDamageMultiply());

	iDam = MAX(0, iAtk - iDef);

	int iPureDam = iDam;

	iPureDam = (iPureDam * iPercent) / 100;

	if (test_server)
	{
		pkAttacker->ChatPacket(CHAT_TYPE_INFO, "ARROW %s -> %s, DAM %d DIST %d GAP %d %% %d",
			pkAttacker->GetName(),
			pkVictim->GetName(),
			iPureDam,
			iDist, iGap, iPercent);
	}

	return iPureDam;
	//return iDam;
}

void NormalAttackAffect(LPCHARACTER pkAttacker, LPCHARACTER pkVictim)
{
	if (!pkAttacker || !pkVictim)
		return;

	// Poison attacks are unique, so special treatment
	if (pkAttacker->GetPoint(POINT_POISON_PCT) && !pkVictim->IsAffectFlag(AFF_POISON))
	{
		int poison_pct = pkAttacker->GetPoint(POINT_POISON_PCT);

#ifdef ENABLE_YOHARA_SYSTEM
		if (pkAttacker->IsSungmaMap() && (pkAttacker->GetPoint(POINT_SUNGMA_IMMUNE) < pkAttacker->GetSungmaMapAttribute(POINT_SUNGMA_IMMUNE)) && poison_pct >= 2)
			poison_pct /= 2;
#endif

		if (number(1, 100) <= poison_pct)
		{
			pkVictim->AttackedByPoison(pkAttacker);
		}
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (pkAttacker->GetPoint(POINT_BLEEDING_PCT) && !pkVictim->IsAffectFlag(AFF_BLEEDING))
	{
		int bleeding_pct = pkAttacker->GetPoint(POINT_BLEEDING_PCT);

#ifdef ENABLE_YOHARA_SYSTEM
		if (pkAttacker->IsSungmaMap() && (pkAttacker->GetPoint(POINT_SUNGMA_IMMUNE) < pkAttacker->GetSungmaMapAttribute(POINT_SUNGMA_IMMUNE)) && bleeding_pct >= 2)
			bleeding_pct /= 2;
#endif

		if (number(1, 100) <= bleeding_pct)
			pkVictim->AttackedByBleeding(pkAttacker);
	}
#endif
	int iStunDuration = 2;
	if (pkAttacker->IsPC() && !pkVictim->IsPC())
		iStunDuration = 4;

	AttackAffect(pkAttacker, pkVictim, POINT_STUN_PCT, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, iStunDuration, "STUN");
	AttackAffect(pkAttacker, pkVictim, POINT_SLOW_PCT, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, 20, "SLOW");
}

int battle_hit(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int& iRetDam)
{
	if (!pkAttacker || !pkVictim)
		return 0;

	//PROF_UNIT puHit("Hit");
	if (test_server)
		sys_log(0, "battle_hit : [%s] attack to [%s] : dam :%d type :%d", pkAttacker->GetName(), pkVictim->GetName(), iRetDam);

	int iDam = CalcMeleeDamage(pkAttacker, pkVictim);

	if (iDam <= 0)
		return (BATTLE_DAMAGE);

#ifdef ENABLE_CHECK_WAIT_HACK
	if (pkAttacker->IsPC())
	{
		if (pkAttacker->GetCShield()->CheckWaithack(pkAttacker->GetX(), pkAttacker->GetY(), get_dword_time(), pkAttacker->GetMoveSpeed()))
		{
			LPDESC d = pkAttacker->GetDesc();
			if (d)
			{
				if (d->DelayedDisconnect(3))
				{
					LogManager::instance().HackLog("CSHIELD WAIT_HACK", pkAttacker);
				}
			}
		}
	}
#endif

	NormalAttackAffect(pkAttacker, pkVictim);

	// Damage calculation
	//iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST)) / 100;

#ifdef ENABLE_ELEMENT_ADD
	if (pkAttacker->IsPC() && pkVictim->IsNPC())
	{
		if (pkVictim->GetMobAttElement(MOB_ELEMENTAL_ELEC) || (pkVictim->GetPoint(POINT_RESIST_ELEC) != 0))
		{
			int iPCAttElec = 0;
#if defined(ENABLE_PENDANT) || defined(ENABLE_REFINE_ELEMENT)
			iPCAttElec += pkAttacker->GetPoint(POINT_ENCHANT_ELECT);
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Power of Lightning(+x% of total value)
			if (pkAttacker->GetPoint(POINT_ENCHANT_PER_ELECT) > 0)
				iPCAttElec = iPCAttElec + (iPCAttElec * pkAttacker->GetPoint(POINT_ENCHANT_PER_ELECT)) / 100;
#endif
			const int iVictimResistElec = pkVictim->GetPoint(POINT_RESIST_ELEC);
			const int iDamageElec = iPCAttElec - ((iPCAttElec > 0) ? iVictimResistElec : 0);
			iDam = iDam * (100 + (iDamageElec / 2)) / 100;
		}

		if (pkVictim->GetMobAttElement(MOB_ELEMENTAL_FIRE) || (pkVictim->GetPoint(POINT_RESIST_FIRE) != 0))
		{
			int iPCAttFire = 0;
#if defined(ENABLE_PENDANT) || defined(ENABLE_REFINE_ELEMENT)
			iPCAttFire += pkAttacker->GetPoint(POINT_ENCHANT_FIRE);
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Power of Fire (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ENCHANT_PER_FIRE) > 0)
				iPCAttFire = iPCAttFire + (iPCAttFire * pkAttacker->GetPoint(POINT_ENCHANT_PER_FIRE)) / 100;
#endif
			const int iVictimResistFire = pkVictim->GetPoint(POINT_RESIST_FIRE);
			const int iDamageFire = iPCAttFire - ((iPCAttFire > 0) ? iVictimResistFire : 0);
			iDam = iDam * (100 + (iDamageFire / 2)) / 100;
		}

		if (pkVictim->GetMobAttElement(MOB_ELEMENTAL_ICE) || (pkVictim->GetPoint(POINT_RESIST_ICE) != 0))
		{
			int iPCAttIce = 0;
#if defined(ENABLE_PENDANT) || defined(ENABLE_REFINE_ELEMENT)
			iPCAttIce += pkAttacker->GetPoint(POINT_ENCHANT_ICE);
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Power of Ice (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ENCHANT_PER_ICE) > 0)
				iPCAttIce = iPCAttIce + (iPCAttIce * pkAttacker->GetPoint(POINT_ENCHANT_PER_ICE)) / 100;
#endif
			const int iVictimResistIce = pkVictim->GetPoint(POINT_RESIST_ICE);
			const int iDamageIce = iPCAttIce - ((iPCAttIce > 0) ? iVictimResistIce : 0);
			iDam = iDam * (100 + (iDamageIce / 2)) / 100;
		}

		if (pkVictim->GetMobAttElement(MOB_ELEMENTAL_WIND) || (pkVictim->GetPoint(POINT_RESIST_WIND) != 0))
		{
			int iPCAttWind = 0;
#if defined(ENABLE_PENDANT) || defined(ENABLE_REFINE_ELEMENT)
			iPCAttWind += pkAttacker->GetPoint(POINT_ENCHANT_WIND);
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Power of Wind (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ENCHANT_PER_WIND) > 0)
				iPCAttWind = iPCAttWind + (iPCAttWind * pkAttacker->GetPoint(POINT_ENCHANT_PER_WIND)) / 100;
#endif
			const int iVictimResistWind = pkVictim->GetPoint(POINT_RESIST_WIND);
			const int iDamageWind = iPCAttWind - ((iPCAttWind > 0) ? iVictimResistWind : 0);
			iDam = iDam * (100 + (iDamageWind / 2)) / 100;
		}

		if (pkVictim->GetMobAttElement(MOB_ELEMENTAL_EARTH) || (pkVictim->GetPoint(POINT_RESIST_EARTH) != 0))
		{
			int iPCAttEarth = 0;
#if defined(ENABLE_PENDANT) || defined(ENABLE_REFINE_ELEMENT)
			iPCAttEarth += pkAttacker->GetPoint(POINT_ENCHANT_EARTH);
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Power of Earth (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ENCHANT_PER_EARTH) > 0)
				iPCAttEarth = iPCAttEarth + (iPCAttEarth * pkAttacker->GetPoint(POINT_ENCHANT_PER_EARTH)) / 100;
#endif
			const int iVictimResistEarth = pkVictim->GetPoint(POINT_RESIST_EARTH);
			const int iDamageEarth = iPCAttEarth - ((iPCAttEarth > 0) ? iVictimResistEarth : 0);
			iDam = iDam * (100 + (iDamageEarth / 2)) / 100;
		}

		if (pkVictim->GetMobAttElement(MOB_ELEMENTAL_DARK) || (pkVictim->GetPoint(POINT_RESIST_DARK) != 0))
		{
			int iPCAttDark = 0;
#if defined(ENABLE_PENDANT) || defined(ENABLE_REFINE_ELEMENT)
			iPCAttDark += pkAttacker->GetPoint(POINT_ENCHANT_DARK);
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			// Power of Darkness (+x% of total value)
			if (pkAttacker->GetPoint(POINT_ENCHANT_PER_DARK) > 0)
				iPCAttDark = iPCAttDark + (iPCAttDark * pkAttacker->GetPoint(POINT_ENCHANT_PER_DARK)) / 100;
#endif
			const int iVictimResistDark = pkVictim->GetPoint(POINT_RESIST_DARK);
			const int iDamageDark = iPCAttDark - ((iPCAttDark > 0) ? iVictimResistDark : 0);
			iDam = iDam * (100 + (iDamageDark / 2)) / 100;
		}
	}
#endif

	const CItem* pkWeapon = pkAttacker->GetWear(WEAR_WEAPON);
	if (pkWeapon)
	{
		switch (pkWeapon->GetSubType())
		{
			case WEAPON_SWORD:
			{
				int iResistSword = pkVictim->GetPoint(POINT_RESIST_SWORD);
#ifdef ENABLE_PENDANT
				iResistSword -= pkAttacker->GetPoint(POINT_ATTBONUS_SWORD);
#endif
				iDam = iDam * (100 - iResistSword) / 100;
			}
			break;

			case WEAPON_TWO_HANDED:
			{
				int iResistTwoHand = pkVictim->GetPoint(POINT_RESIST_TWOHAND);
#ifdef ENABLE_PENDANT
				iResistTwoHand -= pkAttacker->GetPoint(POINT_ATTBONUS_TWOHAND);
#endif
				iDam = iDam * (100 - iResistTwoHand) / 100;
			}
			break;

			case WEAPON_DAGGER:
			{
				int iResistDagger = pkVictim->GetPoint(POINT_RESIST_DAGGER);
#ifdef ENABLE_PENDANT
				iResistDagger -= pkAttacker->GetPoint(POINT_ATTBONUS_DAGGER);
#endif
				iDam = iDam * (100 - iResistDagger) / 100;
			}
			break;

			case WEAPON_BELL:
			{
				int iResistBell = pkVictim->GetPoint(POINT_RESIST_BELL);
#ifdef ENABLE_PENDANT
				iResistBell -= pkAttacker->GetPoint(POINT_ATTBONUS_BELL);
#endif
				iDam = iDam * (100 - iResistBell) / 100;
			}
			break;

			case WEAPON_FAN:
			{
				int iResistFan = pkVictim->GetPoint(POINT_RESIST_FAN);
#ifdef ENABLE_PENDANT
				iResistFan -= pkAttacker->GetPoint(POINT_ATTBONUS_FAN);
#endif
				iDam = iDam * (100 - iResistFan) / 100;
			}
			break;

			case WEAPON_BOW:
			{
				int iResistBow = pkVictim->GetPoint(POINT_RESIST_BOW);
#ifdef ENABLE_PENDANT
				iResistBow -= pkAttacker->GetPoint(POINT_ATTBONUS_BOW);
#endif
				iDam = iDam * (100 - iResistBow) / 100;
			}
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
			case WEAPON_CLAW:
			{
				int iResistClaw = pkVictim->GetPoint(POINT_RESIST_CLAW);
#ifdef ENABLE_PENDANT
				iResistClaw -= pkAttacker->GetPoint(POINT_ATTBONUS_CLAW);
#endif
				iDam = iDam * (100 - iResistClaw) / 100;
			}
			break;
#endif

			default:
				break;
		}
	}
	else
	{
		if (pkAttacker->IsPC())
			iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_FIST)) / 100;
	}

	//Final damage correction. (As of February 2011, only applied to giant spiders.)
	const float attMul = pkAttacker->GetAttMul();
	const float tempIDam = static_cast<float>(iDam);
	iDam = static_cast<int>(attMul * tempIDam + 0.5f);

	iRetDam = iDam;

	//PROF_UNIT puDam("Dam");
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	AccumulateMonsterAttack(pkAttacker, pkVictim);
#endif
#ifdef ENABLE_MONSTER_BACK
	CMiniGameManager::Instance().AttendanceMonsterAttack(pkAttacker, pkVictim);
#endif
#ifdef ENABLE_QUEEN_NETHIS
	SnakeLair::CSnk::Instance().QueenDebuffAttack(pkAttacker, pkVictim);
#endif

#if defined(ENABLE_12ZI) || defined(ENABLE_MONSTER_BACK) || defined(ENABLE_LUCKY_EVENT)
	auto damageType = EDamageType::DAMAGE_TYPE_NORMAL;

	if (IS_SET(pkVictim->GetAIFlag(), AIFLAG_COUNT))
		damageType = EDamageType::DAMAGE_TYPE_SPECIAL;

	if (pkVictim->Damage(pkAttacker, iDam, damageType))
#else
	if (pkVictim->Damage(pkAttacker, iDam, DAMAGE_TYPE_NORMAL))
#endif
	{
		return (BATTLE_DEAD);
	}

	return (BATTLE_DAMAGE);
}

uint32_t GET_ATTACK_SPEED(LPCHARACTER ch)
{
	if (nullptr == ch)
		return 1000;

	const CItem* item = ch->GetWear(WEAR_WEAPON);
	const uint32_t default_bonus = SPEEDHACK_LIMIT_BONUS; // Yuduri attack speed (base 80)
	uint32_t riding_bonus = 0;

	if (ch->IsRiding())
	{
		// 50 bonus attack speed when riding something
		riding_bonus = 50;
	}

	const uint32_t ani_speed = ani_attack_speed(ch);
	uint32_t real_speed = (ani_speed * 100) / (default_bonus + ch->GetPoint(POINT_ATT_SPEED) + riding_bonus);

	// Double attack speed for daggers
	if (item && item->GetSubType() == WEAPON_DAGGER)
		real_speed /= 2;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (item && item->GetSubType() == WEAPON_CLAW)
		real_speed /= 2;
#endif

	return real_speed;

}

void SET_ATTACK_TIME(LPCHARACTER ch, const CHARACTER* victim, uint32_t current_time)
{
	if (nullptr == ch || nullptr == victim)
		return;

	if (!ch->IsPC())
		return;

	ch->m_kAttackLog.dwVID = victim->GetVID();
	ch->m_kAttackLog.dwTime = current_time;
}

void SET_ATTACKED_TIME(const CHARACTER* ch, LPCHARACTER victim, uint32_t current_time)
{
	if (nullptr == ch || nullptr == victim)
		return;

	if (!ch->IsPC())
		return;

	victim->m_AttackedLog.dwPID = ch->GetPlayerID();
	victim->m_AttackedLog.dwAttackedTime = current_time;
}

bool IS_SPEED_HACK(LPCHARACTER ch, LPCHARACTER victim, uint32_t current_time)
{
	if (nullptr == ch || nullptr == victim)
		return false;

	if (!gHackCheckEnable) return false;

	if (ch->m_kAttackLog.dwVID == victim->GetVID())
	{
		if (current_time - ch->m_kAttackLog.dwTime < GET_ATTACK_SPEED(ch))
		{
			INCREASE_SPEED_HACK_COUNT(ch);

			if (test_server)
			{
				sys_log(0, "%s attack hack! time (delta, limit)=(%u, %u) hack_count %d",
					ch->GetName(),
					current_time - ch->m_kAttackLog.dwTime,
					GET_ATTACK_SPEED(ch),
					ch->m_speed_hack_count);

				ch->ChatPacket(CHAT_TYPE_INFO, "%s attack hack! time (delta, limit)=(%u, %u) hack_count %d",
					ch->GetName(),
					current_time - ch->m_kAttackLog.dwTime,
					GET_ATTACK_SPEED(ch),
					ch->m_speed_hack_count);
			}

			SET_ATTACK_TIME(ch, victim, current_time);
			SET_ATTACKED_TIME(ch, victim, current_time);

			if (ch->m_speed_hack_count > 3)	//@fixme503
				ch->SetWaitHackCounter();

			return true;
		}
	}

	SET_ATTACK_TIME(ch, victim, current_time);

	if (victim->m_AttackedLog.dwPID == ch->GetPlayerID())
	{
		if (current_time - victim->m_AttackedLog.dwAttackedTime < GET_ATTACK_SPEED(ch))
		{
			INCREASE_SPEED_HACK_COUNT(ch);

			if (test_server)
			{
				sys_log(0, "%s Attack Speed HACK! time (delta, limit)=(%u, %u), hack_count = %d",
					ch->GetName(),
					current_time - victim->m_AttackedLog.dwAttackedTime,
					GET_ATTACK_SPEED(ch),
					ch->m_speed_hack_count);

				ch->ChatPacket(CHAT_TYPE_INFO, "Attack Speed Hack(%s), (delta, limit)=(%u, %u)",
					ch->GetName(),
					current_time - victim->m_AttackedLog.dwAttackedTime,
					GET_ATTACK_SPEED(ch));
			}

			SET_ATTACKED_TIME(ch, victim, current_time);
			return true;
		}
	}

	SET_ATTACKED_TIME(ch, victim, current_time);
	return false;
}

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
struct AccumulateInfo
{
	bool enable;
	bool effect;
};

AccumulateInfo IsRaceInfo(uint32_t raceIndex) noexcept
{
	AccumulateInfo result;
	switch (raceIndex)
	{
		case 691:	//Custom with effect
			result = { true, true };	// Accumulate Yes | Effect YES
			break;

		case 1091:	//Custom without effect
			result = { true, false };	// Accumulate Yes | Effect NO
			break;

		default:
			result = { false, false };	// Disabled
			break;
	}

	return result;
}

void AccumulateMonsterAttack(LPCHARACTER pkAttacker, const LPCHARACTER& pkVictim)
{
	if (pkAttacker == nullptr || pkVictim == nullptr)
		return;

	if (!pkAttacker->GetDesc())
		return;

	const auto ret = IsRaceInfo(pkVictim->GetRaceNum());

	if (pkVictim->GetMobRank() >= MOB_RANK_BOSS && ret.enable)
	{
		if (pkAttacker->IsPC() && ret.effect)
		{
			if (!pkAttacker->FindAffect(AFFECT_ATT_SPEED_SLOW))
				pkAttacker->AddAffect(AFFECT_ATT_SPEED_SLOW, POINT_ATT_SPEED, -20, 0, 60, 0, false);
		}

		bool bContain = false;
		uint32_t dwCount = 1;

		if (!pkAttacker->m_hitCount.empty())
		{
			for (uint32_t i = 0; i < pkAttacker->m_hitCount.size(); i++)
			{
				if (pkAttacker->m_hitCount[i].dwVid == pkVictim->GetVID())
				{
					bContain = true;
					pkAttacker->m_hitCount[i].dwCount += 1;
					dwCount = pkAttacker->m_hitCount[i].dwCount;
					break;
				}
			}
		}

		if (!bContain)
			pkAttacker->m_hitCount.emplace_back(THitCountInfo(pkVictim->GetVID(), dwCount));

		TPacketGCHitCountInfo packHitCount{};
		packHitCount.bHeader = HEADER_GC_HIT_COUNT_INFO;
		packHitCount.dwVid = pkVictim->GetVID();
		packHitCount.dwCount = dwCount;
		pkAttacker->GetDesc()->Packet(&packHitCount, sizeof(TPacketGCHitCountInfo));
	}
}
#endif
