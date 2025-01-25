#ifndef __INC_METIN_II_GAME_BATTLE_H__
#define __INC_METIN_II_GAME_BATTLE_H__

#include "char.h"

enum EBattleTypes
{
	BATTLE_NONE,
	BATTLE_DAMAGE,
	BATTLE_DEFENSE,
	BATTLE_DEAD
};

extern int CalcAttBonus(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, int iAtk);
extern int CalcBattleDamage(int iDam, int iAttackerLev, int iVictimLev) noexcept;
extern int CalcMeleeDamage(LPCHARACTER pAttacker, LPCHARACTER pVictim, bool bIgnoreDefense = false, bool bIgnoreTargetRating = false);
extern int CalcMagicDamage(LPCHARACTER pAttacker, LPCHARACTER pVictim);
extern int CalcArrowDamage(LPCHARACTER pkAttacker, LPCHARACTER pkVictim, LPITEM pkBow, LPITEM pkArrow, bool bIgnoreDefense = false);
extern float CalcAttackRating(const CHARACTER* pkAttacker, const CHARACTER* pkVictim, bool bIgnoreTargetRating = false);

#ifdef NEW_ICEDAMAGE_SYSTEM
extern bool battle_is_icedamage(const CHARACTER* pAttacker, LPCHARACTER pVictim);
#endif
extern bool battle_is_attackable(LPCHARACTER ch, LPCHARACTER victim);
extern int battle_get_max_distance(const LPCHARACTER& ch, const LPCHARACTER& victim);
extern int battle_melee_attack(LPCHARACTER ch, LPCHARACTER victim);
extern void battle_end(LPCHARACTER ch);

extern bool battle_distance_valid_by_xy(const CHARACTER* ch, const CHARACTER* victim);
extern bool battle_distance_valid(const CHARACTER* ch, const CHARACTER* victim);

extern void NormalAttackAffect(LPCHARACTER pkAttacker, LPCHARACTER pkVictim);

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
extern void AccumulateMonsterAttack(LPCHARACTER pkAttacker, const LPCHARACTER& pkVictim);
#endif

inline void AttackAffect(LPCHARACTER pkAttacker,
	LPCHARACTER pkVictim,
	uint16_t att_point, //@fixme532
	uint32_t immune_flag,
	uint32_t affect_idx,
	uint16_t affect_point, //@fixme532
	long affect_amount,
	uint32_t affect_flag,
	int time,
	const char* name)
{
	if (pkAttacker->GetPoint(att_point) && !pkVictim->IsAffectFlag(affect_flag))
	{
		if (number(1, 100) <= pkAttacker->GetPoint(att_point) && !pkVictim->IsImmune(immune_flag))
		{
			pkVictim->AddAffect(affect_idx, affect_point, affect_amount, affect_flag, time, 0, true);

			if (test_server)
			{
				pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s %s(%ld%%) SUCCESS", pkAttacker->GetName(), name, pkAttacker->GetPoint(att_point));
			}
		}
		else if (test_server)
		{
			pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s %s(%ld%%) FAIL", pkAttacker->GetName(), name, pkAttacker->GetPoint(att_point));
		}
	}
}

inline void SkillAttackAffect(LPCHARACTER pkVictim,
	int success_pct,
	uint32_t immune_flag,
	uint32_t affect_idx,
	uint16_t affect_point, //@fixme532
	long affect_amount,
	uint32_t affect_flag,
	int time,
	const char* name)
{
	if (success_pct && !pkVictim->IsAffectFlag(affect_flag))
	{
		if (number(1, 1000) <= success_pct && !pkVictim->IsImmune(immune_flag))
		{
			pkVictim->AddAffect(affect_idx, affect_point, affect_amount, affect_flag, time, 0, true);

			// SKILL_ATTACK_NO_LOG_TARGET_NAME_FIX
			if (test_server)
				pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s(%d%%) -> %s SUCCESS", name, success_pct, name);
			// END_OF_SKILL_ATTACK_LOG_NO_TARGET_NAME_FIX
		}
		else if (test_server)
		{
			// SKILL_ATTACK_NO_LOG_TARGET_NAME_FIX
			pkVictim->ChatPacket(CHAT_TYPE_PARTY, "%s(%d%%) -> %s FAIL", name, success_pct, name);
			// END_OF_SKILL_ATTACK_LOG_NO_TARGET_NAME_FIX
		}
	}
}

#define GET_SPEED_HACK_COUNT(ch) ((ch)->m_speed_hack_count)
#define INCREASE_SPEED_HACK_COUNT(ch) (++GET_SPEED_HACK_COUNT(ch))
uint32_t GET_ATTACK_SPEED(LPCHARACTER ch);
void SET_ATTACK_TIME(LPCHARACTER ch, const CHARACTER* victim, uint32_t current_time);
void SET_ATTACKED_TIME(const CHARACTER* ch, LPCHARACTER victim, uint32_t current_time);
bool IS_SPEED_HACK(LPCHARACTER ch, LPCHARACTER victim, uint32_t current_time);

#endif
