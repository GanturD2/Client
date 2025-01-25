#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "battle.h"
#include "pvp.h"
#include "skill.h"
#include "start_position.h"
#include "profiler.h"
#include "cmd.h"
#include "dungeon.h"
#include "log.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "db.h"
#include "vector.h"
#include "marriage.h"
#include "arena.h"
#include "regen.h"
#include "monarch.h"
#include "exchange.h"
#include "shop_manager.h"
#include "castle.h"
#include "dev_log.h"
#include "ani.h"
#include "BattleArena.h"
#include "packet.h"
#include "party.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "questmanager.h"
#include "questlua.h"
#include "threeway_war.h"
#include "BlueDragon.h"
#include "DragonLair.h"
#ifdef ENABLE_DAWNMIST_DUNGEON
#	include "DawnMistDungeon.h"
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#ifdef ENABLE_12ZI
#	include "zodiac_temple.h"
#endif
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#include <map>
#ifdef ENABLE_MONSTER_BACK
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
#	include "minigame_manager.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "AchievementSystem.h"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif
#ifdef ENABLE_WHITE_DRAGON
#	include "WhiteDragon.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
EVENTFUNC(hitbuff_element_event)
{
	const hitbuff_event_info* info = dynamic_cast<hitbuff_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("hitbuff_event_info> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr)
		return 0;

	// Enchant
	const auto enchant_fire = ch->GetPoint(POINT_HIT_BUFF_ENCHANT_FIRE);
	if (enchant_fire)
		ch->AddAffect(AFFECT_HIT_BUFF_ENCHANT_FIRE, POINT_ENCHANT_FIRE, 30, AFF_NONE, enchant_fire, 0, true);

	const auto enchant_ice = ch->GetPoint(POINT_HIT_BUFF_ENCHANT_ICE);
	if (enchant_ice)
		ch->AddAffect(AFFECT_HIT_BUFF_ENCHANT_ICE, POINT_ENCHANT_ICE, 30, AFF_NONE, enchant_ice, 0, true);

	const auto enchant_elect = ch->GetPoint(POINT_HIT_BUFF_ENCHANT_ELEC);
	if (enchant_elect)
		ch->AddAffect(AFFECT_HIT_BUFF_ENCHANT_ELEC, POINT_ENCHANT_ELECT, 30, AFF_NONE, enchant_elect, 0, true);

	const auto enchant_wind = ch->GetPoint(POINT_HIT_BUFF_ENCHANT_WIND);
	if (enchant_wind)
		ch->AddAffect(AFFECT_HIT_BUFF_ENCHANT_WIND, POINT_ENCHANT_WIND, 30, AFF_NONE, enchant_wind, 0, true);

	const auto enchant_dark = ch->GetPoint(POINT_HIT_BUFF_ENCHANT_DARK);
	if (enchant_dark)
		ch->AddAffect(AFFECT_HIT_BUFF_ENCHANT_DARK, POINT_ENCHANT_DARK, 30, AFF_NONE, enchant_dark, 0, true);

	const auto enchant_earth = ch->GetPoint(POINT_HIT_BUFF_ENCHANT_EARTH);
	if (enchant_earth)
		ch->AddAffect(AFFECT_HIT_BUFF_ENCHANT_DARK, POINT_ENCHANT_DARK, 30, AFF_NONE, enchant_earth, 0, true);

	// Resist
	const auto resist_fire = ch->GetPoint(POINT_HIT_BUFF_RESIST_FIRE);
	if (resist_fire)
		ch->AddAffect(AFFECT_HIT_BUFF_RESIST_FIRE, POINT_RESIST_FIRE, 30, AFF_NONE, resist_fire, 0, true);

	const auto resist_ice = ch->GetPoint(POINT_HIT_BUFF_RESIST_ICE);
	if (resist_ice)
		ch->AddAffect(AFFECT_HIT_BUFF_RESIST_ICE, POINT_RESIST_ICE, 30, AFF_NONE, resist_ice, 0, true);

	const auto resist_elect = ch->GetPoint(POINT_HIT_BUFF_RESIST_ELEC);
	if (resist_elect)
		ch->AddAffect(AFFECT_HIT_BUFF_RESIST_ELEC, POINT_RESIST_ELEC, 30, AFF_NONE, resist_elect, 0, true);

	const auto resist_wind = ch->GetPoint(POINT_HIT_BUFF_RESIST_WIND);
	if (resist_wind)
		ch->AddAffect(AFFECT_HIT_BUFF_RESIST_WIND, POINT_RESIST_WIND, 30, AFF_NONE, resist_wind, 0, true);

	const auto resist_dark = ch->GetPoint(POINT_HIT_BUFF_RESIST_DARK);
	if (resist_dark)
		ch->AddAffect(AFFECT_HIT_BUFF_RESIST_DARK, POINT_RESIST_DARK, 30, AFF_NONE, resist_dark, 0, true);

	const auto resist_earth = ch->GetPoint(POINT_HIT_BUFF_RESIST_EARTH);
	if (resist_earth)
		ch->AddAffect(AFFECT_HIT_BUFF_RESIST_EARTH, POINT_RESIST_EARTH, 30, AFF_NONE, resist_earth, 0, true);

	// Sungma
	const auto sungma_str = ch->GetPoint(POINT_HIT_BUFF_SUNGMA_STR);
	if (sungma_str)
		ch->AddAffect(AFFECT_HIT_BUFF_SUNGMA_STR, POINT_SUNGMA_STR, 30, AFF_NONE, sungma_str, 0, true);

	const auto sungma_move = ch->GetPoint(POINT_HIT_BUFF_SUNGMA_MOVE);
	if (sungma_move)
		ch->AddAffect(AFFECT_HIT_BUFF_SUNGMA_MOVE, POINT_SUNGMA_MOVE, 30, AFF_NONE, sungma_move, 0, true);

	const auto sungma_hp = ch->GetPoint(POINT_HIT_BUFF_SUNGMA_HP);
	if (sungma_hp)
		ch->AddAffect(AFFECT_HIT_BUFF_SUNGMA_HP, POINT_SUNGMA_HP, 30, AFF_NONE, sungma_hp, 0, true);

	const auto sungma_immune = ch->GetPoint(POINT_HIT_BUFF_SUNGMA_IMMUNE);
	if (sungma_immune)
		ch->AddAffect(AFFECT_HIT_BUFF_SUNGMA_IMMUNE, POINT_SUNGMA_IMMUNE, 30, AFF_NONE, sungma_immune, 0, true);

	return PASSES_PER_SEC(60);
}

void CHARACTER::StartHitBuffElementEvent()
{
	if (m_pkHitBuffElementEvent)
		return;

	hitbuff_event_info* info = AllocEventInfo<hitbuff_event_info>();

	info->ch = this;
	m_pkHitBuffElementEvent = event_create(hitbuff_element_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopHitBuffElementEvent()
{
	event_cancel(&m_pkHitBuffElementEvent);
}
#endif

#ifdef ENABLE_ELEMENTAL_WORLD
EVENTFUNC(elemental_world_event)
{
	const elemental_world_event_info* info = dynamic_cast<elemental_world_event_info*>(event->info);

	if (!info)
	{
		sys_err("elemental_world_event_info> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr)
		return 0;

	if (ch->FindAffect(AFFECT_PROTECTION_OF_ELEMENTAL))
		return PASSES_PER_SEC(10);

	++ch->hp_reduce_count;

	int reduce_hp;
	if (firstDigit(ch->hp_reduce_count) < 7)
	{
		const static int hp_percent[] = { 2, 4, 6, 8, 10, 12, 14, 15 };
		reduce_hp = ch->GetMaxHP() / 100 * hp_percent[firstDigit(ch->hp_reduce_count)];
	}
	else
		reduce_hp = ch->GetMaxHP() / 100 * 15;

	if (test_server)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "hp_reduce_count %d", ch->hp_reduce_count);
		ch->ChatPacket(CHAT_TYPE_INFO, "firstDigit %d", firstDigit(ch->hp_reduce_count));
		ch->ChatPacket(CHAT_TYPE_INFO, "HP decrease %d", reduce_hp);
	}

	//ch->PointChange(POINT_HP, -reduce_hp); //Missing to optimize

	return PASSES_PER_SEC(5);
}

void CHARACTER::StartElementalWorldEvent()
{
	if (m_pkElementalWorldEvent)
		return;

	elemental_world_event_info* info = AllocEventInfo<elemental_world_event_info>();

	info->ch = this;
	m_pkElementalWorldEvent = event_create(elemental_world_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopElementalWorldEvent()
{
	event_cancel(&m_pkElementalWorldEvent);
}
#endif

static uint32_t __GetPartyExpNP(const uint32_t level)
{
	if (!level || level > PLAYER_EXP_TABLE_MAX)
		return 14000;
	return party_exp_distribute_table[level];
}

static int __GetExpLossPerc(const uint32_t level)
{
	if (!level || level > PLAYER_EXP_TABLE_MAX)
		return 1;
	return aiExpLossPercents[level];
}

uint32_t AdjustExpByLevel(const LPCHARACTER ch, const uint32_t exp)
{
	if (!ch)
		return 0;

	if (PLAYER_MAX_LEVEL_CONST < ch->GetLevel())
	{
		double ret = 0.95;
		double factor = 0.1;

		for (ssize_t i = 0; i < ch->GetLevel() - 100; ++i)
		{
			if ((i % 10) == 0)
				factor /= 2.0;

			ret *= 1.0 - factor;
		}

		ret = ret * static_cast<double>(exp);

		if (ret < 1.0)
			return 1;

		return static_cast<uint32_t>(ret);
	}

	return exp;
}

struct FPartyCHCollector
{
	std::vector <LPCHARACTER> vecCHPtr; FPartyCHCollector() { }

	void operator () (LPCHARACTER ch)
	{
		vecCHPtr.emplace_back(ch);
	}
};

bool CHARACTER::CanBeginFight() const
{
	if (!CanMove())
		return false;

	return m_pointsInstant.position == POS_STANDING && !IsDead() && !IsStun();
}

void CHARACTER::BeginFight(LPCHARACTER pkVictim)
{
	if (!pkVictim)
		return;

	SetVictim(pkVictim);
	SetPosition(POS_FIGHTING);
	SetNextStatePulse(1);

#ifdef ENABLE_DAWNMIST_DUNGEON
	constexpr long iTempleIdx = MAP_MT_THUNDER_DUNGEON;
	constexpr uint16_t iTempleGrdnVnum = CDawnMistDungeon::TEMPLE_GUARDIAN;

	if (IsMonster() && GetMapIndex() == iTempleIdx && GetRaceNum() == iTempleGrdnVnum)
		SetBattleStartTime(get_global_time());
	else if (pkVictim->IsMonster() && pkVictim->GetMapIndex() == iTempleIdx && pkVictim->GetRaceNum() == iTempleGrdnVnum)
		pkVictim->SetBattleStartTime(get_global_time());
#endif
}

bool CHARACTER::CanFight() const
{
	return m_pointsInstant.position >= POS_FIGHTING ? true : false;
}

void CHARACTER::CreateFly(uint8_t bType, LPCHARACTER pkVictim)
{
	if (!pkVictim)
		return;

	TPacketGCCreateFly packFly{};

	packFly.bHeader = HEADER_GC_CREATE_FLY;
	packFly.bType = bType;
	packFly.dwStartVID = GetVID();
	packFly.dwEndVID = pkVictim->GetVID();

	PacketAround(&packFly, sizeof(TPacketGCCreateFly));
}

void CHARACTER::DistributeSP(LPCHARACTER pkKiller, int iMethod)
{
	if (!pkKiller)
		return;

	if (pkKiller->GetSP() >= pkKiller->GetMaxSP())
		return;

	const bool bAttacking = (get_dword_time() - GetLastAttackTime()) < 3000;
	const bool bMoving = (get_dword_time() - GetLastMoveTime()) < 3000;

	if (iMethod == 1)
	{
		const int num = number(0, 3);

		if (!num)
		{
			const int iLvDelta = GetLevel() - pkKiller->GetLevel();
			int iAmount = 0;

			if (iLvDelta >= 5)
				iAmount = 10;
			else if (iLvDelta >= 0)
				iAmount = 6;
			else if (iLvDelta >= -3)
				iAmount = 2;

			if (iAmount != 0)
			{
				iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;

				if (iAmount >= 11)
					CreateFly(FLY_SP_BIG, pkKiller);
				else if (iAmount >= 7)
					CreateFly(FLY_SP_MEDIUM, pkKiller);
				else
					CreateFly(FLY_SP_SMALL, pkKiller);

				pkKiller->PointChange(POINT_SP, iAmount);
			}
		}
	}
	else
	{
		if (pkKiller->GetJob() == JOB_SHAMAN || (pkKiller->GetJob() == JOB_SURA && pkKiller->GetSkillGroup() == 2))
		{
			int iAmount = 0;

			if (bAttacking)
				iAmount = 2 + GetMaxSP() / 100;
			else if (bMoving)
				iAmount = 3 + GetMaxSP() * 2 / 100;
			else
				iAmount = 10 + GetMaxSP() * 3 / 100; // normally

			iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
			pkKiller->PointChange(POINT_SP, iAmount);
		}
		else
		{
			int iAmount = 0;

			if (bAttacking)
				iAmount = 2 + pkKiller->GetMaxSP() / 200;
			else if (bMoving)
				iAmount = 2 + pkKiller->GetMaxSP() / 100;
			else
			{
				// normally
				if (pkKiller->GetHP() < pkKiller->GetMaxHP())
					iAmount = 2 + (pkKiller->GetMaxSP() / 100); // When there is no blood
				else
					iAmount = 9 + (pkKiller->GetMaxSP() / 100); // basic
			}

			iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
			pkKiller->PointChange(POINT_SP, iAmount);
		}
	}
}

bool CHARACTER::Attack(LPCHARACTER pkVictim, uint8_t bType)
{
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack : %s type %d, MobBattleType %d", GetName(), bType, !GetMobBattleType() ? 0 : GetMobAttackRange());
	//PROF_UNIT puAttack("Attack");
	if (!CanMove() || IsObserverMode())	//@fixme453
		return false;

#ifdef ENABLE_GM_BLOCK
	if (IsGM() && !GetStaffPermissions() && IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[GM] Du kannst nicht angreifen, wenn du verwandelt bist!");
		return false;
	}
#endif

#if defined(ENABLE_CSHIELD) && defined(ENABLE_CHECK_ATTACKSPEED_HACK)
	if (IsPC())
	{
		uint8_t result = GetCShield()->CheckAttackspeedHack(IsRiding(), ani_attack_speed(this), static_cast<long long>(GetPoint(POINT_ATT_SPEED)), pkVictim->GetVID(), get_dword_time());
		if (result == 1)
		{
			LPDESC d = GetDesc();
			if (d)
			{
				if (d->DelayedDisconnect(3))
				{
					LogManager::Instance().HackLog("Attackspeed Hack", this);
				}
			}
			return false;
		}
		else if (result == 2)
			return false;
}
#endif

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
	if (pkVictim->GetShopOwner())	//@fixme443
		return false;
#endif

#ifdef ENABLE_AFTERDEATH_SHIELD
	if (IsAffectFlag(AFF_AFTERDEATH_SHIELD))
		RemoveShieldAffect();
#endif

	// CASTLE
	if (IS_CASTLE_MAP(GetMapIndex()) && !castle_can_attack(this, pkVictim))
		return false;
	// CASTLE

	// @fixme131
	if (!battle_is_attackable(this, pkVictim))
		return false;

#ifdef ENABLE_CHECK_ANTI_SAFEZONE
	SECTREE	*sectree = nullptr;
	SECTREE	*vsectree = nullptr;
	sectree = GetSectree();
	vsectree = pkVictim->GetSectree();

	if (sectree && vsectree) {
		if (sectree->IsAttr(GetX(), GetY(), ATTR_BANPK) || vsectree->IsAttr(pkVictim->GetX(), pkVictim->GetY(), ATTR_BANPK)) {
			if (GetDesc()) {
				return false;
				LogManager::Instance().HackLog("ANTISAFEZONE", this);
				GetDesc()->DelayedDisconnect(3);
			}
		}
	}
#endif

	const uint32_t dwCurrentTime = get_dword_time();

	if (IsPC())
	{
		if (IS_SPEED_HACK(this, pkVictim, dwCurrentTime))
			return false;

		if (bType == 0 && dwCurrentTime < GetSkipComboAttackByTime())
			return false;

		/*if (!GetWear(WEAR_WEAPON))	//NO DMG with Fists!
			return false;*/
	}
	else
	{
		MonsterChat(MONSTER_CHAT_ATTACK);
	}

	pkVictim->SetSyncOwner(this);

	if (pkVictim->CanBeginFight())
		pkVictim->BeginFight(this);

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	if (IsPC())
		this->StartHitBuffElementEvent();
#endif

	int iRet = BATTLE_NONE;

	if (bType == 0)
	{
		//
		// normal attack
		//
#ifdef ENABLE_SOUL_SYSTEM
		if (IsAffectFlag(AFF_SOUL_RED) || IsAffectFlag(AFF_SOUL_MIX))
			UseSoulAttack(RED_SOUL);
#endif

		switch (GetMobBattleType())
		{
			case BATTLE_TYPE_MELEE:
			case BATTLE_TYPE_POWER:
			case BATTLE_TYPE_TANKER:
			case BATTLE_TYPE_SUPER_POWER:
			case BATTLE_TYPE_SUPER_TANKER:
				iRet = battle_melee_attack(this, pkVictim);
				break;

			case BATTLE_TYPE_RANGE:
				FlyTarget(pkVictim->GetVID(), pkVictim->GetX(), pkVictim->GetY(), HEADER_CG_FLY_TARGETING);
				iRet = Shoot(0) ? BATTLE_DAMAGE : BATTLE_NONE;
				break;

			case BATTLE_TYPE_MAGIC:
				FlyTarget(pkVictim->GetVID(), pkVictim->GetX(), pkVictim->GetY(), HEADER_CG_FLY_TARGETING);
				iRet = Shoot(1) ? BATTLE_DAMAGE : BATTLE_NONE;
				break;

			default:
				sys_err("Unhandled battle type %d", GetMobBattleType());
				iRet = BATTLE_NONE;
				break;
		}
	}
	else
	{
#ifdef ENABLE_SOUL_SYSTEM
		if (IsAffectFlag(AFF_SOUL_BLUE) || IsAffectFlag(AFF_SOUL_MIX))
			UseSoulAttack(BLUE_SOUL);
#endif

		if (IsPC())
		{
			if (dwCurrentTime - m_dwLastSkillTime > 1500)
			{
				sys_log(1, "HACK: Too long skill using term. Name(%s) PID(%u) delta(%u)",
					GetName(), GetPlayerID(), (dwCurrentTime - m_dwLastSkillTime));
				return false;
			}
		}

		sys_log(1, "Attack call ComputeSkill %d %s", bType, pkVictim ? pkVictim->GetName() : "");
		iRet = ComputeSkill(bType, pkVictim);
	}

	//if (test_server && IsPC())
	//	sys_log(0, "%s Attack %s type %u ret %d", GetName(), pkVictim->GetName(), bType, iRet);

	if (iRet != BATTLE_NONE) {	//@fixme455
		pkVictim->SetSyncOwner(this);

		if (pkVictim->CanBeginFight()) {
			pkVictim->BeginFight(this);
		}
	}

	if (iRet == BATTLE_DAMAGE || iRet == BATTLE_DEAD)
	{
		OnMove(true);
		pkVictim->OnMove();

		// only pc sets victim null. For npc, state machine will reset this.
		if (BATTLE_DEAD == iRet && IsPC())
			SetVictim(nullptr);

		return true;
	}

	return false;
}

#ifdef ENABLE_CUBE_RENEWAL
#	include "CubeManager.h"
#endif
void CHARACTER::DeathPenalty(uint8_t bTown)
{
	sys_log(1, "DEATH_PERNALY_CHECK(%s) town(%d)", GetName(), bTown);

#ifndef ENABLE_CUBE_RENEWAL
	Cube_close(this);
#else
	CCubeManager::Instance().Cube_close(this);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	AcceClose();
#endif

	if (CBattleArena::Instance().IsBattleArenaMap(GetMapIndex()))
		return;

	if (GetLevel() < 10)
	{
		sys_log(0, "NO_DEATH_PENALTY_LESS_LV10(%s)", GetName());
		ChatPacket(CHAT_TYPE_INFO, "[LS;901]");
		return;
	}

	if (number(0, 2))
	{
		sys_log(0, "NO_DEATH_PENALTY_LUCK(%s)", GetName());
		ChatPacket(CHAT_TYPE_INFO, "[LS;901]");
		return;
	}

	if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY))
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);

		// NO_DEATH_PENALTY_BUG_FIX
		if (!bTown) // In the international version, Dragon God's Blessing is used only when resurrecting in place. (No experience penalty when returning to the village)
		{
			if (FindAffect(AFFECT_NO_DEATH_PENALTY))
			{
				sys_log(0, "NO_DEATH_PENALTY_AFFECT(%s)", GetName());
				ChatPacket(CHAT_TYPE_INFO, "[LS;901]");
				RemoveAffect(AFFECT_NO_DEATH_PENALTY);
				return;
			}
		}
		// END_OF_NO_DEATH_PENALTY_BUG_FIX

		int iLoss = ((GetNextExp() * __GetExpLossPerc(GetLevel())) / 100);

		iLoss = MIN(800000, iLoss);

		if (bTown)
			iLoss = 0;

		if (IsEquipUniqueItem(UNIQUE_ITEM_TEARDROP_OF_GODNESS))
			iLoss /= 2;

		sys_log(0, "DEATH_PENALTY(%s) EXP_LOSS: %d percent %d%%", GetName(), iLoss, __GetExpLossPerc(GetLevel()));

		PointChange(POINT_EXP, -iLoss, true);
	}
}

bool CHARACTER::IsStun() const
{
	if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN))
		return true;

	return false;
}

EVENTFUNC(StunEvent)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("StunEvent> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	ch->m_pkStunEvent = nullptr;
	ch->Dead();
	return 0;
}

void CHARACTER::Stun()
{
	if (IsStun())
		return;

	if (IsDead())
		return;

	if (!IsPC() && m_pkParty)
	{
		m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);
	}

	sys_log(1, "%s: Stun %p", GetName(), this);

	PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
	PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
	CloseMyShop();
#endif

	event_cancel(&m_pkRecoveryEvent); // Kill recovery events.

	TPacketGCStun pack{};
	pack.header = HEADER_GC_STUN;
	pack.vid = m_vid;
	PacketAround(&pack, sizeof(pack));

	SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

	if (m_pkStunEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkStunEvent = event_create(StunEvent, info, PASSES_PER_SEC(3));
}

EVENTINFO(SCharDeadEventInfo)
{
	bool isPC;
	uint32_t dwID;

	SCharDeadEventInfo()
		: isPC(0)
		, dwID(0)
	{
	}
};

EVENTFUNC(dead_event)
{
	const SCharDeadEventInfo* info = dynamic_cast<SCharDeadEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("dead_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = nullptr;

	if (true == info->isPC)
		ch = CHARACTER_MANAGER::Instance().FindByPID(info->dwID);
	else
		ch = CHARACTER_MANAGER::Instance().Find(info->dwID);

	if (nullptr == ch)
	{
		sys_err("DEAD_EVENT: cannot find char pointer with %s id(%d)", info->isPC ? "PC" : "MOB", info->dwID);
		return 0;
	}

	ch->m_pkDeadEvent = nullptr;

	if (ch->GetDesc())
	{
		ch->GetDesc()->SetPhase(PHASE_GAME);
		ch->SetPosition(POS_STANDING);

#ifdef ENABLE_DEVIL_TOWER
		if (ch->IsDevilTowerDungeon(ch->GetMapIndex()))
		{
			//const auto level = ch->GetDungeonForce()->GetFlag("level");
			LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (!pDungeon)
			{
				ch->WarpSet(590800, 111200, 65);
				return 0;
			}

			const auto level = pDungeon->GetFlag("level");
			const int floorPositions[10][2] =
			{
				{0, 0}, // 0th
				{0, 0}, // 1st
				{2174, 7040}, // 2nd
				{2182, 6803}, // 3rd
				{2417, 7285}, // 4th
				{2417, 7057}, // 5th
				{2422, 6823}, // 6th
				{2638, 7294}, // 7th
				{2638, 7059}, // 8th
				{2638, 6811}, // 9th (boss)
			};

			if (level <= 1)
				ch->WarpSet(590800, 111200, 65);
			else
				ch->WarpSet(floorPositions[level][0] * 100, floorPositions[level][1] * 100, ch->GetMapIndex());

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_DEVIL_CATACOMBS
		if (ch->IsDevilCatacombsDungeon(ch->GetMapIndex()))
		{
			//const auto level = ch->GetDungeonForce()->GetFlag("level");
			LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (!pDungeon)
			{
				ch->WarpSet(592300, 100200, 65);
				return 0;
			}

			const auto level = pDungeon->GetFlag("level");
			const int floorPositions[8][2] =
			{
				{0, 0}, // 0th
				{3145, 12095}, // 1st
				{3622, 12077}, // 2nd
				{4450, 12281}, // 3rd
				{3918, 12930}, // 4th (deleted floor)
				{3918, 12930}, // 5th
				{4434, 12698}, // 6th
				{3145, 13188}, // 7th
			};

			if (level == 0)
				ch->WarpSet(592300, 100200, 65);
			else
				ch->WarpSet(floorPositions[level][0] * 100, floorPositions[level][1] * 100, ch->GetMapIndex());

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_SPIDER_DUNGEON
		if (ch->IsSpiderDungeon(ch->GetMapIndex()))
		{
			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
				ch->WarpSet(88100, 614800, ch->GetMapIndex());
			else
			{
				sys_err("SpiderDungeon cannot find spawn position (name %s)", ch->GetName());
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_FLAME_DUNGEON
		if (ch->IsFlameDungeon(ch->GetMapIndex()))
		{
			//const auto level = ch->GetDungeonForce()->GetFlag("level");
			LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (!pDungeon)
			{
				ch->WarpSet(614400, 707000, 62);
				return 0;
			}

			const auto level = pDungeon->GetFlag("level");
			if (level == 17)
				ch->WarpSet(810900, 686700, ch->GetMapIndex());
			else
				ch->WarpSet(777600, 651600, ch->GetMapIndex());

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_SNOW_DUNGEON
		if (ch->IsSnowDungeon(ch->GetMapIndex()))
		{
			//const auto level = ch->GetDungeonForce()->GetFlag("level");
			LPDUNGEON pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (!pDungeon)
			{
				ch->WarpSet(431800, 164400, 61);
				return 0;
			}

			auto level = pDungeon->GetFlag("level");
			const int floorPositions[11][2] =
			{
				{0, 0}, // 0th
				{5293, 1816}, // 1st
				{5540, 1803}, // 2nd
				{5880, 1809}, // 3rd
				{5292, 2067}, // 4th
				{5539, 2072}, // 5th
				{5865, 2074}, // 6th
				{5423, 2251}, // 7th
				{5691, 2238}, // 8th
				{5969, 2229}, // 9th
				{6047, 1927}, // boss
			};
			if (level == 42)
				level = 4;

			if (level == 0)
				ch->WarpSet(431800, 164400, 61);
			else
				ch->WarpSet(floorPositions[level][0] * 100, floorPositions[level][1] * 100, ch->GetMapIndex());

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_DAWNMIST_DUNGEON
		if (CDawnMistDungeon::Instance().IsInDungeonInstance(ch->GetMapIndex()))
		{
			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
				ch->WarpSet(782500, 1501700, ch->GetMapIndex());
			else
			{
				sys_err("DawnmistDungeon cannot find spawn position (name %s)", ch->GetName());
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		if (ch->GetMeleyLair())
		{
			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
				ch->WarpSet(321000, 1526900, ch->GetMapIndex());
			else
			{
				sys_err("MeleyLair cannot find spawn position (name %s)", ch->GetName());
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_DEFENSE_WAVE
		if (ch->IsDefenseWaveDungeon(ch->GetMapIndex()))
		{
			const LPDUNGEON& pDungeon = CDungeonManager::Instance().FindByMapIndex(ch->GetMapIndex());
			if (!pDungeon)
			{
				ch->WarpSet(1107700, 1782100, 301);
				return 0;
			}

			ch->WarpSet(166500, 522100, ch->GetMapIndex());

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

#ifdef ENABLE_RESTART_CITY_POS
		if (ch->GetDungeon())
		{
			PIXEL_POSITION pos;
			if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), 0, pos))
				ch->WarpSet(pos.x, pos.y, ch->GetMapIndex());
			else
			{
				sys_err("dungeon cannot find spawn position (name %s)", ch->GetName());
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			}

			ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
			ch->DeathPenalty(0);
			ch->StartRecoveryEvent();
			ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			return 0;
		}
#endif

/*#ifdef ENABLE_ELEMENTAL_WORLD
		switch (ch->GetMapIndex())
		{
			case MAP_ELEMENTAL_01:
			case MAP_ELEMENTAL_02:
			case MAP_ELEMENTAL_03:
			case MAP_ELEMENTAL_04:
			{
				ch->WarpSet(128000, 563200, MAP_DEFENSEWAVE_PORT);
				ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
				ch->DeathPenalty(0);
				ch->StartRecoveryEvent();
				ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
			}
			break;

			default:
				break;
		}
#endif*/

		PIXEL_POSITION pos;
		if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
			ch->WarpSet(pos.x, pos.y);
		else
		{
			sys_err("cannot find spawn position (name %s)", ch->GetName());
			ch->GoHome();
		}

		ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);
		ch->DeathPenalty(0);
		ch->StartRecoveryEvent();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
	}
	else
	{
		if (ch->IsMonster())
		{
			if (!ch->IsRevive() && ch->HasReviverInParty())
			{
				ch->SetPosition(POS_STANDING);
				ch->SetHP(ch->GetMaxHP());

				ch->ViewReencode();

				ch->SetAggressive();
				ch->SetRevive(true);

				return 0;
			}
		}

		M2_DESTROY_CHARACTER(ch);
	}

	return 0;
}

bool CHARACTER::IsDead() const noexcept
{
	if (m_pointsInstant.position == POS_DEAD)
		return true;

	return false;
}

#define GetGoldMultipler() (distribution_test_server ? 3 : 1)

void CHARACTER::RewardGold(LPCHARACTER pkAttacker)
{
	// ADD_PREMIUM
	const bool isAutoLoot =
		(pkAttacker->GetPremiumRemainSeconds(PREMIUM_AUTOLOOT) > 0 ||
			pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_AUTOLOOT))
		? true : false; // third hand
		// END_OF_ADD_PREMIUM

	PIXEL_POSITION pos;

	if (!isAutoLoot)
	{
		if (!SECTREE_MANAGER::Instance().GetMovablePosition(GetMapIndex(), GetX(), GetY(), pos))
			return;
	}

	int iTotalGold = 0;
	//
	// --------- Calculate Money Drop Probability ----------
	//
	int iGoldPercent = MobRankStats[GetMobRank()].iGoldPercent;

	if (pkAttacker->IsPC())
		iGoldPercent = iGoldPercent * (100 + CPrivManager::Instance().GetPriv(pkAttacker, PRIV_GOLD_DROP)) / 100;

	if (pkAttacker->GetPoint(POINT_MALL_GOLDBONUS))
		iGoldPercent += (iGoldPercent * pkAttacker->GetPoint(POINT_MALL_GOLDBONUS) / 100);

	iGoldPercent = iGoldPercent * CHARACTER_MANAGER::Instance().GetMobGoldDropRate(pkAttacker) / 100;

	// ADD_PREMIUM
	if (pkAttacker->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0 ||
		pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_LUCKY_GOLD))
		iGoldPercent += iGoldPercent;
	// END_OF_ADD_PREMIUM

	if (iGoldPercent > 100)
		iGoldPercent = 100;

	int iPercent = 0;

	if (GetMobRank() >= MOB_RANK_BOSS)
		iPercent = ((iGoldPercent * PERCENT_LVDELTA_BOSS(pkAttacker->GetLevel(), GetLevel())) / 100);
	else
		iPercent = ((iGoldPercent * PERCENT_LVDELTA(pkAttacker->GetLevel(), GetLevel())) / 100);
	//int iPercent = CALCULATE_VALUE_LVDELTA(pkAttacker->GetLevel(), GetLevel(), iGoldPercent);

	if (number(1, 100) > iPercent)
		return;

	int iGoldMultipler = GetGoldMultipler();

#ifdef ENABLE_GOLD_MULTIPLIER
	if (1 == number(1, 50000)) // 1/50000 chance of 10x money
		iGoldMultipler *= 10;
	else if (1 == number(1, 10000)) // 1/10000 chance of 5x money
		iGoldMultipler *= 5;
#endif

	// personal application
	if (pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
	{
		if (number(1, 100) <= pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
			iGoldMultipler *= 2;
	}

#ifdef ENABLE_GOLD_MULTIPLIER
	//
	// --------- money drop multiple decision ----------
	//
	if (test_server)
		pkAttacker->ChatPacket(CHAT_TYPE_PARTY, "gold_mul %d rate %d", iGoldMultipler, CHARACTER_MANAGER::Instance().GetMobGoldAmountRate(pkAttacker));
#endif

	//
	// --------- actual drop handling -------------
	//
	LPITEM item;

#ifdef ENABLE_GOLD10_PCT
	int iGold10DropPct = 100;
	iGold10DropPct = (iGold10DropPct * 100) / (100 + CPrivManager::Instance().GetPriv(pkAttacker, PRIV_GOLD10_DROP));
#endif

	// If MOB_RANK is higher than BOSS, it is an unconditional money bomb.
	if (GetMobRank() >= MOB_RANK_BOSS && !IsStone() && GetMobTable().dwGoldMax != 0)
	{
#ifdef ENABLE_GOLD10_PCT
		if (1 == number(1, iGold10DropPct))
			iGoldMultipler *= 10; // 10x money with 1% chance
#endif

		const int iSplitCount = number(25, 35);

		for (int i = 0; i < iSplitCount; ++i)
		{
			int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax) / iSplitCount;
			if (test_server)
				sys_log(0, "iGold %d", iGold);
			iGold = iGold * CHARACTER_MANAGER::Instance().GetMobGoldAmountRate(pkAttacker) / 100;
			iGold *= iGoldMultipler;

			if (iGold == 0)
			{
				continue;
			}

			if (test_server)
			{
				sys_log(0, "Drop Moeny MobGoldAmountRate %d %d", CHARACTER_MANAGER::Instance().GetMobGoldAmountRate(pkAttacker), iGoldMultipler);
				sys_log(0, "Drop Money gold %d GoldMin %d GoldMax %d", iGold, GetMobTable().dwGoldMax, GetMobTable().dwGoldMax);
			}

			// NOTE: Money bombs do not deal with third hand
			if ((item = ITEM_MANAGER::Instance().CreateItem(1, iGold)))
			{
				pos.x = GetX() + ((number(-14, 14) + number(-14, 14)) * 23);
				pos.y = GetY() + ((number(-14, 14) + number(-14, 14)) * 23);

				item->AddToGround(GetMapIndex(), pos);
				item->StartDestroyEvent();

				iTotalGold += iGold; // Total gold
			}
		}
	}

	// 1% chance to drop 10 money. (10x drop)
#ifdef ENABLE_GOLD10_PCT
	else if (1 == number(1, iGold10DropPct))
	{
		//
		// money bomb drop
		//
		for (int i = 0; i < 10; ++i)
		{
			int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
			iGold = iGold * CHARACTER_MANAGER::Instance().GetMobGoldAmountRate(pkAttacker) / 100;
			iGold *= iGoldMultipler;

			if (iGold == 0)
			{
				continue;
			}

			// NOTE: Money bombs do not deal with third hand
			if ((item = ITEM_MANAGER::Instance().CreateItem(1, iGold)))
			{
				pos.x = GetX() + (number(-7, 7) * 20);
				pos.y = GetY() + (number(-7, 7) * 20);

				item->AddToGround(GetMapIndex(), pos);
				item->StartDestroyEvent();

				iTotalGold += iGold; // Total gold
			}
		}
	}
#endif
	else
	{
		//
		// money drop in the usual way
		//
		int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
		iGold = iGold * CHARACTER_MANAGER::Instance().GetMobGoldAmountRate(pkAttacker) / 100;
		iGold *= iGoldMultipler;

		int iSplitCount = 0;

		if (iGold >= 3)
			iSplitCount = number(1, 3);
		else if (GetMobRank() >= MOB_RANK_BOSS)
		{
			iSplitCount = number(3, 10);

			if ((iGold / iSplitCount) == 0)
				iSplitCount = 1;
		}
		else
			iSplitCount = 1;

		if (iGold != 0)
		{
			iTotalGold += iGold; // Total gold

			for (int i = 0; i < iSplitCount; ++i)
			{
				if (isAutoLoot)
				{
					pkAttacker->GiveGold(iGold / iSplitCount);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
					pkAttacker->UpdateExtBattlePassMissionProgress(YANG_COLLECT, iGold / iSplitCount, pkAttacker->GetMapIndex());
#endif
				}
				else if ((item = ITEM_MANAGER::Instance().CreateItem(1, iGold / iSplitCount)))
				{
					pos.x = GetX() + (number(-7, 7) * 20);
					pos.y = GetY() + (number(-7, 7) * 20);

					item->AddToGround(GetMapIndex(), pos);
					item->StartDestroyEvent();
				}
			}
		}
	}

	DBManager::Instance().SendMoneyLog(MONEY_LOG_MONSTER, GetRaceNum(), iTotalGold);
}

void CHARACTER::Reward(bool bItemDrop)
{
	if (GetRaceNum() == 5001) // Waegoo drop money unconditionally
	{
		PIXEL_POSITION pos;

		if (!SECTREE_MANAGER::Instance().GetMovablePosition(GetMapIndex(), GetX(), GetY(), pos))
			return;

		LPITEM item;
		int iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
		iGold = iGold * CHARACTER_MANAGER::Instance().GetMobGoldAmountRate(nullptr) / 100;
		iGold *= GetGoldMultipler();
		const int iSplitCount = number(25, 35);

		sys_log(0, "WAEGU Dead gold %d split %d", iGold, iSplitCount);

		for (int i = 1; i <= iSplitCount; ++i)
		{
			if ((item = ITEM_MANAGER::Instance().CreateItem(1, iGold / iSplitCount)))
			{
				if (i != 0)
				{
					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;

					pos.x += GetX();
					pos.y += GetY();
				}

				item->AddToGround(GetMapIndex(), pos);
				item->StartDestroyEvent();
			}
		}
		return;
	}

	//PROF_UNIT puReward("Reward");
	LPCHARACTER pkAttacker = DistributeExp();

	if (!pkAttacker
#ifdef ENABLE_KILL_EVENT_FIX
		&& !(pkAttacker = GetMostAttacked())
#endif
		)
	{
		return;
	}

	//PROF_UNIT pu1("r1");
	if (pkAttacker->IsPC())
	{
		if ((GetLevel() - pkAttacker->GetLevel()) >= -10)
		{
			if (pkAttacker->GetRealAlignment() < 0)
			{
				if (pkAttacker->IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_KILL))
					pkAttacker->UpdateAlignment(14);
				else
					pkAttacker->UpdateAlignment(7);
			}
			else
				pkAttacker->UpdateAlignment(2);
		}

#ifdef ENABLE_MONSTER_BACK
		if (IsMonsterBackVnum())
			RewardAttendance();
#endif

		pkAttacker->SetQuestNPCID(GetVID());
		quest::CQuestManager::Instance().Kill(pkAttacker->GetPlayerID(), GetRaceNum());
		CHARACTER_MANAGER::Instance().KillLog(GetRaceNum());
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		pkAttacker->UpdateExtBattlePassMissionProgress(KILL_MONSTER, 1, GetRaceNum());
#endif
#ifdef ENABLE_HUNTING_SYSTEM
		pkAttacker->UpdateHuntingMission(GetRaceNum());
#endif

		if (!number(0, 9))
		{
			if (pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY))
			{
				const int iHP = pkAttacker->GetMaxHP() * pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY) / 100;
				pkAttacker->PointChange(POINT_HP, iHP);
				CreateFly(FLY_HP_SMALL, pkAttacker);
			}

			if (pkAttacker->GetPoint(POINT_KILL_SP_RECOVER))
			{
				const int iSP = pkAttacker->GetMaxSP() * pkAttacker->GetPoint(POINT_KILL_SP_RECOVER) / 100;
				pkAttacker->PointChange(POINT_SP, iSP);
				CreateFly(FLY_SP_SMALL, pkAttacker);
			}
		}
	}
	//pu1.Pop();

	if (!bItemDrop)
		return;

#ifdef ENABLE_MULTI_FARM_BLOCK
	if (!pkAttacker->GetMultiStatus())
		return;
#endif

	PIXEL_POSITION pos = GetXYZ();

	if (!SECTREE_MANAGER::Instance().GetMovablePosition(GetMapIndex(), pos.x, pos.y, pos))
		return;

	//
	// money drop
	//
	//PROF_UNIT pu2("r2");
	if (test_server)
		sys_log(0, "Drop money : Attacker %s", pkAttacker->GetName());
	RewardGold(pkAttacker);
	//pu2.Pop();

	//
	// drop an item
	//
	//PROF_UNIT pu3("r3");
	LPITEM item;

	static std::vector<LPITEM> s_vec_item;
	s_vec_item.clear();

	if (ITEM_MANAGER::Instance().CreateDropItem(this, pkAttacker, s_vec_item))
	{
		if (s_vec_item.size() == 0);
		else if (s_vec_item.size() == 1)
		{
			item = s_vec_item[0];
			item->AddToGround(GetMapIndex(), pos);

			if (CBattleArena::Instance().IsBattleArenaMap(pkAttacker->GetMapIndex()) == false)
			{
#ifdef ENABLE_DICE_SYSTEM
				if (pkAttacker->GetParty())
				{
					FPartyDropDiceRoll f(item, pkAttacker);
					f.Process(this);
				}
				else
					item->SetOwnership(pkAttacker);
#else
				item->SetOwnership(pkAttacker);
#endif
			}

			item->StartDestroyEvent();

			pos.x = number(-7, 7) * 20;
			pos.y = number(-7, 7) * 20;
			pos.x += GetX();
			pos.y += GetY();

			sys_log(0, "DROP_ITEM: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
			if (pkAttacker && !CBattleArena::instance().IsBattleArenaMap(pkAttacker->GetMapIndex()))
				CAchievementSystem::Instance().Collect(pkAttacker, achievements::ETaskTypes::TYPE_COLLECT, item->GetVnum(), item->GetCount());
#endif
		}
		else
		{
			int iItemIdx = s_vec_item.size() - 1;

			std::priority_queue<std::pair<int, LPCHARACTER> > pq;

			int total_dam = 0;

			for (TDamageMap::iterator it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
			{
				int iDamage = it->second.iTotalDamage;
				if (iDamage > 0)
				{
					LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(it->first);

					if (ch)
					{
						pq.push(std::make_pair(iDamage, ch));
						total_dam += iDamage;
					}
				}
			}

			std::vector<LPCHARACTER> v;
#ifdef ENABLE_WB_RANKING
			std::vector<int> dmg;
#endif

			while (!pq.empty() && pq.top().first * 10 >= total_dam)
			{
				v.emplace_back(pq.top().second);
#ifdef ENABLE_WB_RANKING
				dmg.push_back(pq.top().first);
#endif
				pq.pop();
			}

			if (v.empty())
			{
				// No one has done any special damage, so no ownership
				while (iItemIdx >= 0)
				{
					item = s_vec_item[iItemIdx--];

					if (!item)
					{
						sys_err("item null in vector idx %d", iItemIdx + 1);
						continue;
					}

					item->AddToGround(GetMapIndex(), pos);
					// Those who inflict less than 10% damage have no ownership
					//item->SetOwnership(pkAttacker);
					item->StartDestroyEvent();

					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;
					pos.x += GetX();
					pos.y += GetY();

					sys_log(0, "DROP_ITEM: %s %d %d by %s", item->GetName(), pos.x, pos.y, GetName());
				}
			}
			else
			{
				// Ownership is divided only between those who have dealt a lot of damage.
				std::vector<LPCHARACTER>::iterator it = v.begin();
#ifdef ENABLE_WB_RANKING
				std::unordered_map<int, int> dmgranks;
				std::vector<int>::iterator it2 = dmg.begin();
#endif

				while (iItemIdx >= 0)
				{
#ifdef ENABLE_WB_RANKING
					auto dwvnum = CHARACTER_MANAGER::Instance().WBVnums;

					LPCHARACTER ch = *it;
					int iDMG = *it2;

					for (int i = 0; i < dwvnum.size(); ++i)
					{
						if (GetRaceNum() == dwvnum[i])
						{
							if (iDMG > WB_MIN_DMG)
							{
								sys_err("%s iDMG %d > WB_MIN_DMG %d", ch->GetName(), iDMG, WB_MIN_DMG);

								auto rank = dmgranks.find(ch->GetVID());

								if (rank == dmgranks.end())
									dmgranks.insert(std::make_pair(ch->GetVID(), iDMG));
							}
						}
					}

					++it;
					++it2;

					if (it == v.end())
						it = v.begin();
					if (it2 == dmg.end())
						it2 = dmg.begin();
#endif

					item = s_vec_item[iItemIdx--];

					if (!item)
					{
						sys_err("item null in vector idx %d", iItemIdx + 1);
						continue;
					}

					item->AddToGround(GetMapIndex(), pos);

#ifndef ENABLE_WB_RANKING
					LPCHARACTER ch = *it;
#endif

#ifndef ENABLE_GROUPLOOT_ONLY_FOR_ATTACKERS
					if (ch->GetParty())
						ch = ch->GetParty()->GetNextOwnership(ch, GetX(), GetY());
#endif

					++it;

					if (it == v.end())
						it = v.begin();

					if (CBattleArena::Instance().IsBattleArenaMap(ch->GetMapIndex()) == false)
					{
#ifdef ENABLE_DICE_SYSTEM
						if (ch->GetParty())
						{
							FPartyDropDiceRoll f(item, ch);
							f.Process(this);
						}
						else
							item->SetOwnership(ch);
#else
						item->SetOwnership(ch);
#endif
					}

					item->StartDestroyEvent();

					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;
					pos.x += GetX();
					pos.y += GetY();

					sys_log(0, "DROP_ITEM: %s %d %d by %s", item->GetName(), pos.x, pos.y, GetName());

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
					if (ch && CBattleArena::instance().IsBattleArenaMap(ch->GetMapIndex()))
						CAchievementSystem::Instance().Collect(ch, achievements::ETaskTypes::TYPE_COLLECT, item->GetVnum(), item->GetCount());
#endif
				}

#ifdef ENABLE_WB_RANKING
				for (auto ranks : dmgranks)
				{
					auto xch = CHARACTER_MANAGER::Instance().Find(ranks.first);

					if (xch)
					{
						ChatPacket(CHAT_TYPE_COMMAND, "worldboss_ranking update|%s|%d", xch->GetName(), ranks.second);
						sys_err("damage name %s dmg %d", xch->GetName(), ranks.second);
					}
					else
						sys_err("damage %d vid %d", ranks.second, ranks.first);
				}
#endif
			}
		}
	}

	m_map_kDamage.clear();
}

// ItemDropPenalty reworked like the last wiki -> 17/03/2020
struct TItemDropPenalty
{
	int iInventoryPct; // Range: 1 ~ 1000
	int iInventoryQty; // Range: --
	int iEquipmentPct; // Range: 1 ~ 100
	int iEquipmentQty; // Range: --
};

TItemDropPenalty aItemDropPenalty_kor[9] =
{
	//inv: %, count | equip: %, count
	{ 0, 0, 0, 0 }, // Chivalric
	{ 0, 0, 0, 0 }, // Noble
	{ 0, 0, 0, 0 }, // Good
	{ 0, 0, 0, 0 }, // Friendly
	{ 0, 0, 0, 0 }, // Neutral
	{ 20, 5, 5, 1 }, // Aggressive
	{ 30, 10, 10, 2 }, // Fraudulent
	{ 40, 20, 35, 3 }, // Malicious
	{ 80, 40, 40, 4 }, // Cruel
};

void CHARACTER::ItemDropPenalty(LPCHARACTER pkKiller)
{
	// Items are not dropped while the private store is open.
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
		return;
#else
	if (GetMyShop())
		return;
#endif

	if (IsGM())
		return;

	if (GetLevel() < 50)
		return;

	//@fixme469
	if (CArenaManager::Instance().IsArenaMap(GetMapIndex()))
		return;

	if (CBattleArena::Instance().IsBattleArenaMap(GetMapIndex()))
		return;

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
		return;
#endif

	switch (GetMapIndex())
	{
		case 90: // Budokan
		case 113: // OX Event
			return;
		default:
			break;
	}

	const struct TItemDropPenalty* table = &aItemDropPenalty_kor[0];

	const auto iAlignIndex = GetAlignRank();

	std::vector<std::pair<LPITEM, int> > vec_item;
	LPITEM pkItem;
	int i;

	const TItemDropPenalty& r = table[iAlignIndex];
	sys_log(0, "%s align %d inven_pct %d equip_pct %d", GetName(), iAlignIndex, r.iInventoryPct, r.iEquipmentPct);

	bool bDropInventory = r.iInventoryPct >= number(1, 100);
	bool bDropEquipment = r.iEquipmentPct >= number(1, 100);
	bool bDropAntiDropUniqueItem = false;

	if ((bDropInventory || bDropEquipment) && IsEquipUniqueItem(UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)) // Lucy's Ring
	{
		bDropInventory = false;
		bDropEquipment = false;
		bDropAntiDropUniqueItem = true;
	}

	if (bDropInventory) // Drop Inventory
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		std::vector<uint16_t> vec_bSlots;
#else
		std::vector<uint8_t> vec_bSlots;
#endif

		for (i = 0; i < INVENTORY_SLOT_COUNT; ++i)
		{
			if (GetInventoryItem(i))
				vec_bSlots.emplace_back(i);
		}

		if (!vec_bSlots.empty())
		{
			std::random_device rd;
			std::mt19937 mt(rd());
			std::shuffle(vec_bSlots.begin(), vec_bSlots.end(), mt);

			const int iQty = MIN(vec_bSlots.size(), number(1, r.iInventoryQty));

			for (i = 0; i < iQty; ++i)
			{
				pkItem = GetInventoryItem(vec_bSlots[i]);

				if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP | ITEM_ANTIFLAG_MYSHOP | ITEM_ANTIFLAG_MAIL)) // ITEM_ANTIFLAG_MAIL -> ENABLE_MAILBOX
					continue;

				if (pkItem->GetType() == ITEM_PET)
					continue;

#ifdef ENABLE_SEALBIND_SYSTEM
				if (pkItem->IsSealed())
					continue;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
				if (pkItem->IsBasicItem())
					continue;
#endif

				SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(vec_bSlots[i]), QUICKSLOT_MAX_POS);
				vec_item.emplace_back(std::make_pair(pkItem->RemoveFromCharacter(), INVENTORY));
			}
		}
	}

	if (bDropEquipment) // Drop Equipment
	{
		std::vector<uint8_t> vec_bSlots;

		for (i = 0; i < EQUIPMENT_SLOT_COUNT; ++i)
		{
			if (GetEquipmentItem(i))
				vec_bSlots.emplace_back(i);
		}

		if (!vec_bSlots.empty())
		{
			std::random_device rd;
			std::mt19937 mt(rd());
			std::shuffle(vec_bSlots.begin(), vec_bSlots.end(), mt);

			const int iQty = MIN(vec_bSlots.size(), r.iEquipmentQty);

			for (i = 0; i < iQty; ++i)
			{
				pkItem = GetEquipmentItem(vec_bSlots[i]);

				if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP | ITEM_ANTIFLAG_MYSHOP | ITEM_ANTIFLAG_MAIL)) // ITEM_ANTIFLAG_MAIL -> ENABLE_MAILBOX
					continue;

				if (pkItem->GetType() == ITEM_DS)
					continue;

				if (pkItem->GetType() == ITEM_PET)
					continue;

#ifdef ENABLE_SEALBIND_SYSTEM
				if (pkItem->IsSealed())
					continue;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
				if (pkItem->IsBasicItem())
					continue;
#endif

				SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], QUICKSLOT_MAX_POS);
				vec_item.emplace_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
			}
		}
	}

	if (bDropAntiDropUniqueItem)
	{
		LPITEM pkItem;

		pkItem = GetWear(WEAR_UNIQUE1);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE1, QUICKSLOT_MAX_POS);
			vec_item.emplace_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}

		pkItem = GetWear(WEAR_UNIQUE2);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE2, QUICKSLOT_MAX_POS);
			vec_item.emplace_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}
	}

	{
		PIXEL_POSITION pos{};
		pos.x = GetX();
		pos.y = GetY();

		for (size_t k = 0; k < vec_item.size(); ++k)
		{
			LPITEM item = vec_item[k].first;
			const int window = vec_item[k].second;

			item->AddToGround(GetMapIndex(), pos);
			item->StartDestroyEvent();

			sys_log(0, "DROP_ITEM_PK: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());
			LogManager::Instance().ItemLog(this, item, "DEAD_DROP", (window == INVENTORY) ? "INVENTORY" : ((window == EQUIPMENT) ? "EQUIPMENT" : ""));

			pos.x = GetX() + number(-7, 7) * 20;
			pos.y = GetY() + number(-7, 7) * 20;
		}
	}
}

class FPartyAlignmentCompute
{
	public:
		FPartyAlignmentCompute(int iAmount, int x, int y)
		{
			m_iAmount = iAmount;
			m_iCount = 0;
			m_iStep = 0;
			m_iKillerX = x;
			m_iKillerY = y;
		}

		void operator () (LPCHARACTER pkChr)
		{
			if (pkChr && (DISTANCE_APPROX(pkChr->GetX() - m_iKillerX, pkChr->GetY() - m_iKillerY) < PARTY_DEFAULT_RANGE))
			{
				if (m_iStep == 0)
					++m_iCount;
				else
					pkChr->UpdateAlignment(m_iAmount / m_iCount);
			}
		}

		int m_iAmount;
		int m_iCount;
		int m_iStep;

		int m_iKillerX;
		int m_iKillerY;
};

void CHARACTER::Dead(LPCHARACTER pkKiller, bool bImmediateDead)
{
	if (IsDead())
		return;

#ifdef ENABLE_UNMOUNT_WHEN_DEAD
	{
		if (IsHorseRiding())
		{
			StopRiding();
		}
		else if (GetMountVnum())
		{
			RemoveAffect(AFFECT_MOUNT_BONUS);
			m_dwMountVnum = 0;
			UnEquipSpecialRideUniqueItem();

			UpdatePacket();
		}
	}
#endif

	if (!pkKiller && m_dwKillerPID)
		pkKiller = CHARACTER_MANAGER::Instance().FindByPID(m_dwKillerPID);

	m_dwKillerPID = 0; // Must reset --> DO NOT DELETE THIS LINE UNLESS YOU ARE 1000000% SURE

#ifdef ENABLE_SKILL_COOLTIME_UPDATE
	if (IsPC())
		ResetSkillCoolTimes();
#endif

	bool isAgreedPVP = false;
	bool isUnderGuildWar = false;
	bool isDuel = false;
	bool isForked = false;
#ifdef ENABLE_BATTLE_FIELD
	bool isBattleField = false;
	bool isBattleFieldTarget = false;
#endif
	bool wasDuelingInstance = false;	//custom000

	if (pkKiller && pkKiller->IsPC())
	{
		if (pkKiller->m_pkChrTarget == this)
			pkKiller->SetTarget(nullptr);

		if (!IsPC() && pkKiller->GetDungeon())
			pkKiller->GetDungeon()->IncKillCount(pkKiller, this);

		wasDuelingInstance = CPVPManager::Instance().IsDuelingInstance(this);	//custom000
		isAgreedPVP = CPVPManager::Instance().Dead(this, pkKiller->GetPlayerID());
		isDuel = CArenaManager::Instance().OnDead(pkKiller, this);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (pkKiller) {
			if (IsPC() && pkKiller->IsPC()) {
				// if ((strcmp(GetDesc()->GetAccountTable().szHWID, pkKiller->GetDesc()->GetAccountTable().szHWID) != 0))
				CAchievementSystem::Instance().OnKill(pkKiller, this, isDuel || isAgreedPVP);

			}
			else if (!IsPC() && pkKiller->IsPC())
				CAchievementSystem::Instance().OnKill(pkKiller, this, false);
		}
#endif

		if (IsPC())
		{
			CGuild* g1 = GetGuild();
			const CGuild* g2 = pkKiller->GetGuild();

			if (g1 && g2)
			{
				if (g1->UnderWar(g2->GetID()))
					isUnderGuildWar = true;
			}

			pkKiller->SetQuestNPCID(GetVID());
			quest::CQuestManager::Instance().Kill(pkKiller->GetPlayerID(), quest::QUEST_NO_NPC);
			CGuildManager::Instance().Kill(pkKiller, this);
		}
#ifdef ENABLE_12ZI
		if ((IsMonster() || IsStone()) && pkKiller->IsPC() && (CZodiacManager::Instance().IsZiStageMapIndex(pkKiller->GetMapIndex())))
			CZodiacManager::Instance().DeadMob(this, pkKiller->GetMapIndex());
#endif
	}

#ifdef ENABLE_QUEST_DIE_EVENT
	if (IsPC())
	{
		if (pkKiller)
			SetQuestNPCID(pkKiller->GetVID());
		// quest::CQuestManager::Instance().Die(GetPlayerID(), quest::QUEST_NO_NPC);
		quest::CQuestManager::Instance().Die(GetPlayerID(), (pkKiller) ? pkKiller->GetRaceNum() : quest::QUEST_NO_NPC);
	}
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
	if (!IsPC() && pkKiller && pkKiller->IsPC())
	{
		if (IsStone())
		{
			if (pkKiller->GetQuestFlag("new_inventory_info.status") == 0)
			{
				pkKiller->SetQuestFlag("new_inventory_info.status", 1);
			}

			if (CMiniGameManager::Instance().IsStoneEvent())
			{
				if (pkKiller->GetLevel() > GetMobTable().bLevel + 20 || pkKiller->GetLevel() < GetMobTable().bLevel - 20)
				{
					pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;1935]");
				}
				else
				{
					CMiniGameManager::Instance().SetStoneKill(pkKiller->GetPlayerID());
					CMiniGameManager::Instance().StoneUpdateP2PPacket(pkKiller->GetPlayerID());
					CMiniGameManager::Instance().StoneInformation(pkKiller);
				}
			}
		}
	}
#endif

	//CHECK_FORKEDROAD_WAR
	if (IsPC())
	{
		if (CThreeWayWar::Instance().IsThreeWayWarMapIndex(GetMapIndex()))
			isForked = true;
#ifdef ENABLE_BATTLE_FIELD
		if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
		{
			isBattleField = true;
			if (IsAffectFlag(AFFECT_TARGET_VICTIM))
				isBattleFieldTarget = true;
		}
#endif
	}
	//END_CHECK_FORKEDROAD_WAR

	if (pkKiller &&
		!isAgreedPVP &&
		!isUnderGuildWar &&
		IsPC() &&
		!isDuel &&
		!isForked &&
#ifdef ENABLE_BATTLE_FIELD
		!isBattleField &&
#endif
		!IS_CASTLE_MAP(GetMapIndex()))
	{
		if (GetGMLevel() == GM_PLAYER || test_server)
		{
			if (test_server)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Player was dueling still registered in DuelManager %d"), wasDuelingInstance);

			ItemDropPenalty(pkKiller);
		}
	}

	// CASTLE_SIEGE
	if (IS_CASTLE_MAP(GetMapIndex()))
	{
		if (CASTLE_FROG_VNUM == GetRaceNum())
			castle_frog_die(this, pkKiller);
		else if (castle_is_guard_vnum(GetRaceNum()))
			castle_guard_die(this, pkKiller);
		else if (castle_is_tower_vnum(GetRaceNum()))
			castle_tower_die(this, pkKiller);
	}
	// CASTLE_SIEGE

	if (true == isForked)
	{
		CThreeWayWar::Instance().onDead(this, pkKiller);
	}

#ifdef ENABLE_BATTLE_FIELD
	if (isBattleField)
		CBattleField::Instance().PlayerKill(this, pkKiller);
#endif

#ifdef ENABLE_EVENT_MANAGER
	if (GetMapIndex() == CEventManager::SIEGE_MAP_INDEX)
		CEventManager::Instance().OnDead(this, pkKiller);
#endif

	SetPosition(POS_DEAD);
	ClearAffect(true);

	if (pkKiller && IsPC())
	{
		if (!pkKiller->IsPC())
		{
			if (!isForked
#ifdef ENABLE_BATTLE_FIELD
				&& !isBattleField
#endif
				)
			{
				sys_log(1, "DEAD: %s %p WITH PENALTY", GetName(), this);
				SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
				LogManager::Instance().CharLog(this, pkKiller->GetRaceNum(), "DEAD_BY_NPC", pkKiller->GetName());
			}
#ifdef ENABLE_12ZI
			if (IsPC() && pkKiller->IsMonster() && (CZodiacManager::Instance().IsZiStageMapIndex(GetMapIndex())))
			{
				IncDeadCount();
				this->SetQuestFlag("12zi_temple.IsDead", 1);
				this->SetQuestFlag("12zi_temple.PrismNeed", this->GetDeadCount());
				CZodiacManager::Instance().DeadPC(this->GetMapIndex());
			}
#endif
		}
		else
		{
			sys_log(1, "DEAD_BY_PC: %s %p KILLER %s %p", GetName(), this, pkKiller->GetName(), get_pointer(pkKiller));
			REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
			pkKiller->UpdateExtBattlePassMissionProgress(KILL_PLAYER, 1, GetLevel());
#endif

			if (GetEmpire() != pkKiller->GetEmpire())
			{
				const int iEP = MIN(GetPoint(POINT_EMPIRE_POINT), pkKiller->GetPoint(POINT_EMPIRE_POINT));

				PointChange(POINT_EMPIRE_POINT, -(iEP / 10));
				pkKiller->PointChange(POINT_EMPIRE_POINT, iEP / 5);

				if (GetPoint(POINT_EMPIRE_POINT) < 10)
				{
					// TODO: You have to put the code that blows to the entrance.
				}

				char buf[256];
				snprintf(buf, sizeof(buf),
					"%d %d %d %s %d %d %d %s",
					GetEmpire(), GetAlignment(), GetPKMode(), GetName(),
					pkKiller->GetEmpire(), pkKiller->GetAlignment(), pkKiller->GetPKMode(), pkKiller->GetName());

				LogManager::Instance().CharLog(this, pkKiller->GetPlayerID(), "DEAD_BY_PC", buf);
			}
			else
			{
				if (!isAgreedPVP && !isUnderGuildWar && !IsKillerMode() && GetAlignment() >= 0 && !isDuel && !isForked && 
#ifdef ENABLE_BATTLE_FIELD
					!isBattleField &&
#endif
					pkKiller->GetPKMode() != PK_MODE_PEACE)
				{
					int iNoPenaltyProb = 0;

					if (pkKiller->GetAlignment() >= 0) // 1/3 percent down
						iNoPenaltyProb = 33;
					else // 4/5 percent down
						iNoPenaltyProb = 20;

					if (number(1, 100) < iNoPenaltyProb)
					{
						pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;910]");
					}
					else
					{
						if (pkKiller->GetParty())
						{
							FPartyAlignmentCompute f(-20000, pkKiller->GetX(), pkKiller->GetY());
							pkKiller->GetParty()->ForEachOnMapMember(f, pkKiller->GetMapIndex());	//@fixme522

							if (f.m_iCount == 0)
								pkKiller->UpdateAlignment(-20000);
							else
							{
								sys_log(0, "ALIGNMENT PARTY count %d amount %d", f.m_iCount, f.m_iAmount);

								f.m_iStep = 1;
								pkKiller->GetParty()->ForEachOnMapMember(f, pkKiller->GetMapIndex());	//@fixme522
							}
						}
						else
							pkKiller->UpdateAlignment(-20000);
					}
				}

				char buf[256];
				snprintf(buf, sizeof(buf),
					"%d %d %d %s %d %d %d %s",
					GetEmpire(), GetAlignment(), GetPKMode(), GetName(),
					pkKiller->GetEmpire(), pkKiller->GetAlignment(), pkKiller->GetPKMode(), pkKiller->GetName());

				LogManager::Instance().CharLog(this, pkKiller->GetPlayerID(), "DEAD_BY_PC", buf);
			}
		}
	}
	else
	{
		sys_log(1, "DEAD: %s %p", GetName(), this);
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
	}

	ClearSync();

	//sys_log(1, "stun cancel %s[%d]", GetName(), (uint32_t)GetVID());
	event_cancel(&m_pkStunEvent); // The stun event kills.

	if (IsPC())
	{
		m_dwLastDeadTime = get_dword_time();
		SetKillerMode(false);
		GetDesc()->SetPhase(PHASE_DEAD);
	}
	else
	{
		//@fixme504
		if (m_pkRegen) {
			if (m_pkDungeon) {
				// Dungeon regen may not be valid at this point
				if (m_pkDungeon->IsValidRegen(m_pkRegen, regen_id_)) {
					--m_pkRegen->count;
				}
			}
			else {
				// Is this really safe?
				--m_pkRegen->count;
			}
			m_pkRegen = nullptr;
		}

		if (m_pkDungeon) {
			SetDungeon(nullptr);
		}
		//@end_fixme504

		if (!HasNoRewardFlag())
		{
			if (!(pkKiller && pkKiller->IsPC() && pkKiller->GetGuild() && pkKiller->GetGuild()->UnderAnyWar(GUILD_WAR_TYPE_FIELD)))
			{
				// Resurrected monsters do not give rewards.
				if (GetMobTable().dwResurrectionVnum)
				{
					// DUNGEON_MONSTER_REBIRTH_BUG_FIX
					LPCHARACTER chResurrect = CHARACTER_MANAGER::Instance().SpawnMob(GetMobTable().dwResurrectionVnum, GetMapIndex(), GetX(), GetY(), GetZ(), true, (int)GetRotation());
					if (GetDungeon() && chResurrect)
					{
						chResurrect->SetDungeon(GetDungeon());
					}
#ifdef ENABLE_12ZI
					if (GetZodiac() && chResurrect)
						chResurrect->SetZodiac(GetZodiac());
#endif
					// END_OF_DUNGEON_MONSTER_REBIRTH_BUG_FIX

					Reward(false);
				}
				else if (IsRevive())
				{
					Reward(false);
				}
				else
				{
					Reward(true); // Drops gold, item, etc..
				}
			}
			else
			{
				if (pkKiller->m_dwUnderGuildWarInfoMessageTime < get_dword_time())
				{
					pkKiller->m_dwUnderGuildWarInfoMessageTime = get_dword_time() + 60000;
					pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;558]");
				}
			}
		}
	}

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	if (quest::CQuestManager::Instance().GetEventFlag("e_late_summer_event") != 0)
	{
		if (!IsPC() && (pkKiller && pkKiller->IsPC()))
		{
			if (number(1, 100) <= SOUL_COLLECT_PERC)
			{
				pkKiller->SetSoulPoint(pkKiller->GetSoulPoint() + 1);
				pkKiller->RemoveAffect(AFFECT_LATE_SUMMER_EVENT_BUFF);
				pkKiller->AddAffect(AFFECT_LATE_SUMMER_EVENT_BUFF, POINT_NONE, pkKiller->GetSoulPoint(), 0, INFINITE_AFFECT_DURATION, 0, true);
				pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;1453]");
			}

			/*for (int i = AFFECT_LATE_SUMMER_EVENT_BUFF; i <= AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF; ++i)	//
			{
				const CAffect* pSoulAffect = FindAffect(i);
				if (pSoulAffect)
				{
					if ((pSoulAffect->dwType == AFFECT_LATE_SUMMER_EVENT_BUFF && GetSoulPoint() >= 20) || (pSoulAffect->dwType == AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF && GetSoulPoint() >= 150))
					{
						//pkKiller->ChatPacket(CHAT_TYPE_INFO, "Total Souls received!");
						return;
					}

					if (number(1, 100) <= SOUL_COLLECT_PERC)
					{
						pkKiller->SetSoulPoint(pkKiller->GetSoulPoint() + 1);
						pkKiller->RemoveAffect(AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF);
						pkKiller->AddAffect(AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF, POINT_NONE, pkKiller->GetSoulPoint(), 0, INFINITE_AFFECT_DURATION, 0, true);
						pkKiller->ChatPacket(CHAT_TYPE_INFO, "[LS;1453]");
					}
				}
			}*/
		}
	}
#endif

	// BOSS_KILL_LOG
	if (GetMobRank() >= MOB_RANK_BOSS && pkKiller && pkKiller->IsPC())
	{
		char buf[51];
		snprintf(buf, sizeof(buf), "%d %ld", g_bChannel, pkKiller->GetMapIndex());
		if (IsStone())
			LogManager::Instance().CharLog(pkKiller, GetRaceNum(), "STONE_KILL", buf);
		else
			LogManager::Instance().CharLog(pkKiller, GetRaceNum(), "BOSS_KILL", buf);
	}
	// END_OF_BOSS_KILL_LOG

	TPacketGCDead pack{};
	pack.header = HEADER_GC_DEAD;
	pack.vid = m_vid;
#ifdef ENABLE_BATTLE_FIELD
	pack.bRestart = TRUE;

	if (isBattleFieldTarget)
		pack.bRestart = false;

	pack.lMapIdx = GetMapIndex();
#endif
	PacketAround(&pack, sizeof(pack));

	REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

	// If the player character
	if (GetDesc() != nullptr)
	{
		//
		// Sends an effect packet back to the client.
		//
		auto it = m_list_pkAffect.begin();

		while (it != m_list_pkAffect.end())
			SendAffectAddPacket(GetDesc(), *it++);
	}

	/*
	* create dead event,
	*
	* In the case of a dead event, the monster will be destroyed after a few seconds,
	* In the case of PC, after 3 minutes, let him out of the village. User within 3 minutes
	* Decide whether to start in town or here.
	*/
	if (!isDuel)
	{
		if (m_pkDeadEvent)
		{
			sys_log(1, "DEAD_EVENT_CANCEL: %s %p %p", GetName(), this, get_pointer(m_pkDeadEvent));
			event_cancel(&m_pkDeadEvent);
		}

		if (IsStone())
			ClearStone();

		if (GetDungeon())
			GetDungeon()->DeadCharacter(this);

		SCharDeadEventInfo* pEventInfo = AllocEventInfo<SCharDeadEventInfo>();

		if (IsPC())
		{
			pEventInfo->isPC = true;
			pEventInfo->dwID = this->GetPlayerID();

			m_pkDeadEvent = event_create(dead_event, pEventInfo, PASSES_PER_SEC(180));
		}
		else
		{
			pEventInfo->isPC = false;
			pEventInfo->dwID = this->GetVID();

			if (!IsRevive() && HasReviverInParty())
			{
				m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(3));
			}
#ifdef ENABLE_DEFENSE_WAVE
			else if (IsDefenseWaveMastAttackMob())
			{
				m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(3));
			}
			/*else if (IsMast())
			{
				m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(1));
			}*/
#endif
#ifdef ENABLE_QUEEN_NETHIS
			else if (GetRaceNum() == SnakeLair::eSnakeConfig::PILAR_STEP_4)
			{
				m_pkDeadEvent = event_create(dead_event, pEventInfo, PASSES_PER_SEC(1));
			}
#endif
			else
			{
				m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(10));
			}
		}

		sys_log(1, "DEAD_EVENT_CREATE: %s %p %p", GetName(), this, get_pointer(m_pkDeadEvent));
	}

	if (m_pkExchange != nullptr)
	{
		m_pkExchange->Cancel();
	}

#ifndef ENABLE_CUBE_RENEWAL
	if (IsCubeOpen())
		Cube_close(this);
#else
	if (IsCubeOpen())
		CCubeManager::Instance().Cube_close(this);
#endif

	if (IsPC())
	{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		if (m_bAcceCombination || m_bAcceAbsorption)
			AcceClose();
#endif
	}

#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen())
		AuraRefineWindowClose();
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	CShopManager::Instance().StopShopping(this);
	//Just in case we are ever .Dead()'ing a shop...
	if (IsShop())
		CloseShop();
#else
	CShopManager::Instance().StopShopping(this);
	CloseMyShop();
#endif

	CloseSafebox();
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	CloseGuildstorage();
#endif
#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen())
		AuraRefineWindowClose();
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	/*if (IsChangeLookWindowOpen())
		ChangeLookWindowClose();*/
#endif

	if (IsMonster() && 2493 == GetMobTable().dwVnum) {
		if (nullptr != pkKiller && nullptr != pkKiller->GetGuild())
			CDragonLairManager::Instance().OnDragonDead( this, pkKiller->GetGuild()->GetID() );
		else
			sys_err("DragonLair: Dragon killed by nobody");
	}

#ifdef ENABLE_QUEEN_NETHIS
	if ((IsStone()) || (IsMonster()))
	{
		if (pkKiller && pkKiller->IsPC())
		{
			if (SnakeLair::CSnk::instance().IsSnakeMap(pkKiller->GetMapIndex()))
				SnakeLair::CSnk::instance().OnKill(this, pkKiller);
		}
	}
#endif

#ifdef ENABLE_WHITE_DRAGON
	if (IsMonster())
	{
		if (pkKiller && pkKiller->IsPC())
		{
			if (WhiteDragon::CWhDr::instance().IsWhiteMap(pkKiller->GetMapIndex()))
				WhiteDragon::CWhDr::instance().OnKill(this, pkKiller);
		}
	}
#endif

#ifdef ENABLE_DAWNMIST_DUNGEON
	if (IsMonster())
	{
		int iRaceNum = GetRaceNum();

		switch (iRaceNum)
		{
			case CDawnMistDungeon::TEMPLE_BOSS:
				if (GetMapIndex() == MAP_MT_THUNDER_DUNGEON)
					CDawnMistDungeon::Instance().SpawnTempleGuardian();
				break;

			case CDawnMistDungeon::TEMPLE_GUARDIAN:
				if (GetMapIndex() == MAP_MT_THUNDER_DUNGEON)
					CDawnMistDungeon::Instance().SpawnTemplePortal(this, GetX(), GetY());
				break;

			case CDawnMistDungeon::FOREST_DUNGEON_BOSS:
				if (CDawnMistDungeon::Instance().IsInDungeonInstance(GetMapIndex()))
					SetMaxSP(0);
				break;

			case CDawnMistDungeon::FOREST_DUNGEON_HEALER:
				{
					if (!CDawnMistDungeon::Instance().IsInDungeonInstance(GetMapIndex()))
						break;

					if (!m_pkHealerEvent)
						break;

					event_cancel(&m_pkHealerEvent);
					m_pkHealerEvent = nullptr;
				}
				break;
		}
	}
#endif

#ifdef ENABLE_12ZI
	if (IsStone())
	{
		if (GetSectree())
		{
			GetSectree()->RemoveEntity(this);
			ViewCleanup();
		}
	}

	if (IsZodiacBoss())
	{
		if (m_pkZodiacSkill1)
			event_cancel(&m_pkZodiacSkill1);

		if (m_pkZodiacSkill2)
			event_cancel(&m_pkZodiacSkill2);

		if (m_pkZodiacSkill3)
			event_cancel(&m_pkZodiacSkill3);

		if (m_pkZodiacSkill4)
			event_cancel(&m_pkZodiacSkill4);

		if (m_pkZodiacSkill5)
			event_cancel(&m_pkZodiacSkill5);

		if (m_pkZodiacSkill6)
			event_cancel(&m_pkZodiacSkill6);

		if (m_pkZodiacSkill7)
			event_cancel(&m_pkZodiacSkill7);

		if (m_pkZodiacSkill8)
			event_cancel(&m_pkZodiacSkill8);

		if (m_pkZodiacSkill9)
			event_cancel(&m_pkZodiacSkill9);

		if (m_pkZodiacSkill10)
			event_cancel(&m_pkZodiacSkill10);

		if (m_pkZodiacSkill11)
			event_cancel(&m_pkZodiacSkill11);
	}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if ((IsStone()) || (IsMonster()))
	{
		if (pkKiller)
		{
			if (CMeleyLairManager::Instance().IsMeleyMap(pkKiller->GetMapIndex()))
			{
				if (pkKiller->GetMeleyLair())
				{
					switch (GetRaceNum())
					{
						case MELEY_GUILD_MOBVNUM_RESPAWN_STONE_STEP2:
						case MELEY_GUILD_MOBVNUM_RESPAWN_BOSS_STEP3:
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
						case MELEY_PARTY_MOBVNUM_RESPAWN_STONE_STEP2:
						case MELEY_PARTY_MOBVNUM_RESPAWN_BOSS_STEP3:
#	endif
							pkKiller->GetMeleyLair()->OnKill(GetMobTable().dwVnum);

						default:
							pkKiller->GetMeleyLair()->OnKillCommon(this);
					}
				}
			}
		}
	}
#endif

#ifdef ENABLE_WORLD_BOSS
	if (IsMonster() && quest::CQuestManager::instance().GetEventFlag("world_boss_event") == 1)
	{
		const uint16_t dwVnum = GetRaceNum();
		if (dwVnum)
		{
			auto vnums = CHARACTER_MANAGER::Instance().WBVnums;
			for (int i = 0; i < vnums.size(); ++i)
			{
				if (dwVnum == vnums[i])
				{
					CHARACTER_MANAGER::Instance().OnKill(GetVID());
				}
			}
		}
	}
#endif
}

struct FuncSetLastAttacked
{
	FuncSetLastAttacked(uint32_t dwTime) : m_dwTime(dwTime)
	{
	}

	void operator () (LPCHARACTER ch)
	{
		if (ch)
			ch->SetLastAttacked(m_dwTime);
	}

	uint32_t m_dwTime;
};

void CHARACTER::SetLastAttacked(uint32_t dwTime)
{
	assert(m_pkMobInst != nullptr);

	m_pkMobInst->m_dwLastAttackedTime = dwTime;
	m_pkMobInst->m_posLastAttacked = GetXYZ();
}

void CHARACTER::SendDamagePacket(LPCHARACTER pAttacker, int Damage, uint8_t DamageFlag)
{
	if (!pAttacker)
		return;

	if (IsPC() || (pAttacker->IsPC() && pAttacker->GetTarget() == this))
	{
		TPacketGCDamageInfo damageInfo;
		memset(&damageInfo, 0, sizeof(TPacketGCDamageInfo));

		damageInfo.header = HEADER_GC_DAMAGE_INFO;
		damageInfo.dwVID = (uint32_t)GetVID();
		damageInfo.flag = DamageFlag;
		damageInfo.damage = Damage;

		if (GetDesc() != nullptr)
		{
			GetDesc()->Packet(&damageInfo, sizeof(TPacketGCDamageInfo));
		}

		if (pAttacker->GetDesc() != nullptr)
		{
			pAttacker->GetDesc()->Packet(&damageInfo, sizeof(TPacketGCDamageInfo));
		}
		/*
		if (!GetArenaObserverMode() && GetArena() != nullptr)
		{
			GetArena()->SendPacketToObserver(&damageInfo, sizeof(TPacketGCDamageInfo));
		}
		*/
	}
}

/*
* The CHARACTER::Damage method causes this to take damage.
*
* Arguments
* pAttacker : the attacker
* dam : damage
* EDamageType: What type of attack is it?
*
* Return value
* true : dead
* false : not dead yet
*/
bool CHARACTER::Damage(LPCHARACTER pAttacker, int dam, EDamageType type) // returns true if dead
{
	if (!pAttacker)
		return false;

#ifdef ENABLE_SUNG_MAHI_TOWER
	if (pAttacker && pAttacker->IsPC())
	{
		LPDUNGEON dungeonInstance = pAttacker->GetDungeon();
		if (dungeonInstance && dungeonInstance->GetFlag("chessWrongMonster") < 1)
		{
			if (this->GetUniqueMaster())
			{
				pAttacker->AggregateMonsterByMaster();
				this->SetUniqueMaster(false);
				dungeonInstance->SetFlag("chessWrongMonster", 1);
			}
		}
	}
#endif

	//@fixme423
	if (pAttacker && this) {
		if (pAttacker->IsAffectFlag(AFF_GWIGUM) && !pAttacker->GetWear(WEAR_WEAPON)) {
			pAttacker->RemoveAffect(SKILL_GWIGEOM);		// Verzauberte Klinge
			return false;
		}

		if (pAttacker->IsAffectFlag(AFF_GEOMGYEONG) && !pAttacker->GetWear(WEAR_WEAPON)) {
			pAttacker->RemoveAffect(SKILL_GEOMKYUNG);	// Aura des Schwertes
			return false;
		}
	}
	//@END_fixme423

#ifdef ENABLE_NEWSTUFF
	if (pAttacker && IsStone() && pAttacker->IsPC())
	{
		if (GetEmpire() && GetEmpire() == pAttacker->GetEmpire())
		{
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
			return false;
		}
	}
#endif

#ifdef ENABLE_REVERSED_FUNCTIONS
	if (pAttacker && !CanAttack())
	{
		SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
		return false;
	}
#endif

#ifdef ENABLE_BLUEDRAGON_RENEWAL
	if (pAttacker && pAttacker->IsBlueDragonDungeon(pAttacker->GetMapIndex()))
	{
		const int iDragonVnum = GetRaceNum();
		if (iDragonVnum == BLUEDRAGON_BOSS)
		{
			const auto& pDungeon = pAttacker->GetDungeon();
			if (!pDungeon)
				return false;

			const size_t cnt_def = SECTREE_MANAGER::Instance().GetMonsterCountInMap(GetMapIndex(), BLUEDRAGON_DEF_STONE);
			const size_t cnt_atk = SECTREE_MANAGER::Instance().GetMonsterCountInMap(GetMapIndex(), BLUEDRAGON_ATK_STONE);
			const size_t cnt_rgn_time = SECTREE_MANAGER::Instance().GetMonsterCountInMap(GetMapIndex(), BLUEDRAGON_REGEN_TIME_STONE);
			const size_t cnt_rgn_cycle = SECTREE_MANAGER::Instance().GetMonsterCountInMap(GetMapIndex(), BLUEDRAGON_REGEN_PECT_STONE);

			const int total_stone = cnt_def + cnt_atk + cnt_rgn_time + cnt_rgn_cycle;

			if (total_stone >= 1)
			{
				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;
			}
		}
	}
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if (pAttacker && CMeleyLairManager::Instance().IsMeleyMap(pAttacker->GetMapIndex()))
	{
		switch (GetRaceNum())
		{
			case MELEY_GUILD_STATUE_VNUM:
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
			case MELEY_PARTY_STATUE_VNUM:
#	endif
			{
				if (pAttacker->GetMeleyLair())
				{
					if (!pAttacker->GetMeleyLair()->Damage(this))
					{
						SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
						return false;
					}
				}
			}
			break;

			case MELEY_BOSS_VNUM:
				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;

			default:
				break;
		}
	}
#endif

#ifdef ENABLE_NINETH_SKILL
	if (IsAffectFlag(AFF_CHUNWOON_MOOJUK))
	{
		SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
		return false;
	}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	if (pAttacker && pAttacker->IsPC() && pAttacker->IsSungmaMap())
	{
		if (!IsPC())
		{
			if (pAttacker->GetPoint(POINT_SUNGMA_STR) < pAttacker->GetSungmaMapAttribute(POINT_SUNGMA_STR))
				dam /= 2;

			if (!pAttacker->GetConquerorLevel())
				dam = 0;
		}
	}
#endif

#ifdef ENABLE_AFTERDEATH_SHIELD
	if (IsAffectFlag(AFF_AFTERDEATH_SHIELD))
		return false;
#endif

#ifdef ENABLE_PRECISION
	if (pAttacker->IsSungmaMap() && pAttacker->IsPC())
	{
		const int skillprecision = aiSkillPrecision[MINMAX(0, pAttacker->GetSkillLevel(SKILL_PRECISION), SKILL_MAX_LEVEL)]; //20%
		const int blockperc = skillprecision + pAttacker->GetPoint(POINT_HIT_PCT);
		const int sungmaMapAttr = pAttacker->GetSungmaMapAttribute(POINT_HIT_PCT);

		int iChance = 0;
		if (sungmaMapAttr > 0)
			iChance = rand() % sungmaMapAttr;

		if (test_server)
		{
			pAttacker->ChatPacket(CHAT_TYPE_INFO, "battle_hit - Precision: %d", blockperc);
			pAttacker->ChatPacket(CHAT_TYPE_INFO, "battle_hit - Map-Block: %d", iChance);
		}

		if (iChance > blockperc)
		{
			if (test_server)
				pAttacker->ChatPacket(CHAT_TYPE_INFO, "DAMAGE_BLOCK!");
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
			return false;
		}
	}
#endif

#ifdef ENABLE_NEWSTUFF
	if (pAttacker && IsStone() && pAttacker->IsPC())
	{
		if (GetEmpire() && GetEmpire() == pAttacker->GetEmpire())
		{
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
			return false;
		}
	}
#endif

	if (EDamageType::DAMAGE_TYPE_MAGIC == type)
	{
		dam = static_cast<int>(static_cast<float>(dam * (100 + (pAttacker->GetPoint(POINT_MAGIC_ATT_BONUS_PER) + pAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100.f + 0.5f));
	}

	if (GetRaceNum() == 5001)
	{
		bool bDropMoney = false;

		int iPercent = 0; // @fixme136
		if (GetMaxHP() >= 0)
			iPercent = (GetHP() * 100) / GetMaxHP();

		if (iPercent <= 10 && GetMaxSP() < 5)
		{
			SetMaxSP(5);
			bDropMoney = true;
		}
		else if (iPercent <= 20 && GetMaxSP() < 4)
		{
			SetMaxSP(4);
			bDropMoney = true;
		}
		else if (iPercent <= 40 && GetMaxSP() < 3)
		{
			SetMaxSP(3);
			bDropMoney = true;
		}
		else if (iPercent <= 60 && GetMaxSP() < 2)
		{
			SetMaxSP(2);
			bDropMoney = true;
		}
		else if (iPercent <= 80 && GetMaxSP() < 1)
		{
			SetMaxSP(1);
			bDropMoney = true;
		}

		if (bDropMoney)
		{
			constexpr uint32_t dwGold = 1000;
			const int iSplitCount = number(10, 13);

			sys_log(0, "WAEGU DropGoldOnHit %d times", GetMaxSP());

			for (int i = 1; i <= iSplitCount; ++i)
			{
				PIXEL_POSITION pos;
				LPITEM item;

				if ((item = ITEM_MANAGER::Instance().CreateItem(1, dwGold / iSplitCount)))
				{
					if (i != 0)
					{
						pos.x = (number(-14, 14) + number(-14, 14)) * 20;
						pos.y = (number(-14, 14) + number(-14, 14)) * 20;

						pos.x += GetX();
						pos.y += GetY();
					}

					item->AddToGround(GetMapIndex(), pos);
					item->StartDestroyEvent();
				}
			}
		}
	}

	// Handling fear when not hitting
	if (type != EDamageType::DAMAGE_TYPE_NORMAL && type != EDamageType::DAMAGE_TYPE_NORMAL_RANGE)
	{
		if (IsAffectFlag(AFF_TERROR))
		{
			const int pct = GetSkillPower(SKILL_TERROR) / 400;

			if (number(1, 100) <= pct)
				return false;
		}
	}

	const int iCurHP = GetHP();
	const int iCurSP = GetSP();

	bool IsCritical = false;
	bool IsPenetrate = false;
	bool IsDeathBlow = false;

	//PROF_UNIT puAttr("Attr");

	/*
	* Magic-type skills and range-type skills (the archerist) calculate critical and penetration attacks.
	* I shouldn't do it, but I can't do a Nerf (downbalance) patch, so
	* Do not use the original value of the penetration attack, and apply it more than /2.
	*
	* There are many samurai stories, so melee skill is also added
	*
	* 20091109: As a result, it was concluded that the samurai became extremely strong, and the proportion of samurai in Germany was close to 70%
	*/
	if (type == EDamageType::DAMAGE_TYPE_MELEE || type == EDamageType::DAMAGE_TYPE_RANGE || type == EDamageType::DAMAGE_TYPE_MAGIC)
	{
		if (pAttacker)
		{
			// critical
			int iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

			if (!IsPC())
				iCriticalPct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS);

			if (iCriticalPct)
			{
				if (iCriticalPct >= 10) // If greater than 10, 5% + (increases by 1% every 4), so if the number is 50, 20%
					iCriticalPct = 5 + (iCriticalPct - 10) / 4;
				else // If less than 10, simply cut in half, 10 = 5%
					iCriticalPct /= 2;

				//Apply critical resistance value.
				iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL);

				if (number(1, 100) <= iCriticalPct)
				{
					IsCritical = true;
					dam *= 2;
					EffectPacket(SE_CRITICAL);

					if (IsAffectFlag(AFF_MANASHIELD))
					{
						RemoveAffect(AFF_MANASHIELD);
					}
				}
			}

			// Penetration attack
			int iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

			if (!IsPC())
				iPenetratePct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS);

			if (iPenetratePct)
			{
				{
					CSkillProto* pkSk = CSkillManager::Instance().Get(SKILL_RESIST_PENETRATE);

					if (nullptr != pkSk)
					{
						pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_RESIST_PENETRATE) / 100.0f);

						iPenetratePct -= static_cast<int>(pkSk->kPointPoly.Eval());
					}
				}

				if (iPenetratePct >= 10)
				{
					// If greater than 10, 5% + (increases by 1% every 4), so if the number is 50, 20%
					iPenetratePct = 5 + (iPenetratePct - 10) / 4;
				}
				else
				{
					// Less than 10 simply cut in half, 10 = 5%
					iPenetratePct /= 2;
				}

				//Apply penetration resistance value.
				iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

				if (number(1, 100) <= iPenetratePct)
				{
					IsPenetrate = true;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, "Zusätzlicher Stichwaffenschaden %d", GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);

					dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;

					if (IsAffectFlag(AFF_MANASHIELD))
					{
						RemoveAffect(AFF_MANASHIELD);
					}
#ifdef ENABLE_EFFECT_PENETRATE
					EffectPacket(SE_PENETRATE);
#endif
				}
			}
		}
	}
	//
	// Attribute values are calculated only for combo attacks, bow attacks, that is, flat hits.
	//
	else if (type == EDamageType::DAMAGE_TYPE_NORMAL || type == EDamageType::DAMAGE_TYPE_NORMAL_RANGE)
	{
		if (type == EDamageType::DAMAGE_TYPE_NORMAL)
		{
			// Can be blocked if it is close to flat
			if (GetPoint(POINT_BLOCK) && number(1, 100) <= GetPoint(POINT_BLOCK))
			{
				if (test_server)
				{
					pAttacker->ChatPacket(CHAT_TYPE_INFO, "%s blocken! (%d%)", GetName(), GetPoint(POINT_BLOCK));
					ChatPacket(CHAT_TYPE_INFO, "%s blocken! (%d%)", GetName(), GetPoint(POINT_BLOCK));
				}

				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;
			}

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
			if (GetPoint(POINT_NORMAL_DAMAGE_GUARD) && number(1, 100) <= GetPoint(POINT_NORMAL_DAMAGE_GUARD))
			{
				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;
			}
#endif
		}
		else if (type == EDamageType::DAMAGE_TYPE_NORMAL_RANGE)
		{
			// Can be avoided in the case of long range hitting
			if (GetPoint(POINT_DODGE) && number(1, 100) <= GetPoint(POINT_DODGE))
			{
				if (test_server)
				{
					pAttacker->ChatPacket(CHAT_TYPE_INFO, "%s ausweichen! (%d%)", GetName(), GetPoint(POINT_DODGE));
					ChatPacket(CHAT_TYPE_INFO, "%s ausweichen! (%d%)", GetName(), GetPoint(POINT_DODGE));
				}

				SendDamagePacket(pAttacker, 0, DAMAGE_DODGE);
				return false;
			}
		}

		if (IsAffectFlag(AFF_JEONGWIHON))
			dam = static_cast<int>(dam * (100 + GetSkillPower(SKILL_JEONGWI) * 25 / 100) / 100);

		if (IsAffectFlag(AFF_TERROR))
			dam = static_cast<int>(dam * (95 - GetSkillPower(SKILL_TERROR) / 5) / 100);

		if (IsAffectFlag(AFF_HOSIN))
			dam = dam * (100 - GetPoint(POINT_RESIST_NORMAL_DAMAGE)) / 100;

		//
		// Apply attacker attribute
		//
		if (pAttacker)
		{
			if (type == EDamageType::DAMAGE_TYPE_NORMAL)
			{
				// reflect
				if (GetPoint(POINT_REFLECT_MELEE))
				{
					int reflectDamage = dam * GetPoint(POINT_REFLECT_MELEE) / 100;

					/* NOTE: If the attacker has the IMMUNE_REFLECT attribute, it is better not to reflect
					* No, the plan requested that it be fixed at 1/3 damage.*/
					if (pAttacker->IsImmune(IMMUNE_REFLECT))
						reflectDamage = static_cast<int>(reflectDamage / 3.0f + 0.5f);

					pAttacker->Damage(this, reflectDamage, EDamageType::DAMAGE_TYPE_SPECIAL);
				}
			}

			// critical
			int iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

			if (!IsPC())
				iCriticalPct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS);

			if (iCriticalPct)
			{
				//Apply critical resistance value.
				iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL);

				if (number(1, 100) <= iCriticalPct)
				{
					IsCritical = true;
					dam *= 2;
					EffectPacket(SE_CRITICAL);
				}
			}

			// Penetration attack
			int iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

			if (!IsPC())
				iPenetratePct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS);

			{
				CSkillProto* pkSk = CSkillManager::Instance().Get(SKILL_RESIST_PENETRATE);

				if (nullptr != pkSk)
				{
					pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_RESIST_PENETRATE) / 100.0f);

					iPenetratePct -= static_cast<int>(pkSk->kPointPoly.Eval());
				}
			}


			if (iPenetratePct)
			{
				//Apply penetration resistance value.
				iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

				if (number(1, 100) <= iPenetratePct)
				{
					IsPenetrate = true;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, "Zusätzlicher Stichwaffenschaden %d", GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);
					dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;
#ifdef ENABLE_EFFECT_PENETRATE
					EffectPacket(SE_PENETRATE);
#endif
				}
			}

			// HP Steal
			if (pAttacker->GetPoint(POINT_STEAL_HP))
			{
				if (number(1, 10) <= 1)
				{
					const int iHP = MIN(dam, MAX(0, iCurHP)) * pAttacker->GetPoint(POINT_STEAL_HP) / 100;

					if (iHP > 0 && GetHP() >= iHP)
					{
						CreateFly(FLY_HP_SMALL, pAttacker);
						pAttacker->PointChange(POINT_HP, iHP);
						PointChange(POINT_HP, -iHP);
					}
				}
			}

			// SP Steal
			if (pAttacker->GetPoint(POINT_STEAL_SP))
			{
				if (number(1, 10) <= 1)
				{
					int iCur = 0;

					if (IsPC())
						iCur = iCurSP;
					else
						iCur = iCurHP;

					const int iSP = MIN(dam, MAX(0, iCur)) * pAttacker->GetPoint(POINT_STEAL_SP) / 100;

					if (iSP > 0 && iCur >= iSP)
					{
						CreateFly(FLY_SP_SMALL, pAttacker);
						pAttacker->PointChange(POINT_SP, iSP);

						if (IsPC())
							PointChange(POINT_SP, -iSP);
					}
				}
			}

			// Money steal
			if (pAttacker->GetPoint(POINT_STEAL_GOLD))
			{
				if (number(1, 100) <= pAttacker->GetPoint(POINT_STEAL_GOLD))
				{
					const int iAmount = number(1, GetLevel());
					pAttacker->PointChange(POINT_GOLD, iAmount);
					DBManager::Instance().SendMoneyLog(MONEY_LOG_MISC, 1, iAmount);
				}
			}

			// HP recovery every time you hit
			if (pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) && number(0, 4) > 0) // 80% Enchanted Blade
			{
				const int i = ((iCurHP >= 0) ? MIN(dam, iCurHP) : dam) * pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) / 100; //@fixme107

				if (i)
				{
					CreateFly(FLY_HP_SMALL, pAttacker);
					pAttacker->PointChange(POINT_HP, i);
				}
			}

			// SP recovery every time you hit
			if (pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) && number(0, 4) > 0) // 80% chance
			{
				const int i = ((iCurHP >= 0) ? MIN(dam, iCurHP) : dam) * pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) / 100; //@fixme107

				if (i)
				{
					CreateFly(FLY_SP_SMALL, pAttacker);
					pAttacker->PointChange(POINT_SP, i);
				}
			}

			// Removes the opponent's mana.
			if (pAttacker->GetPoint(POINT_MANA_BURN_PCT))
			{
				if (number(1, 100) <= pAttacker->GetPoint(POINT_MANA_BURN_PCT))
					PointChange(POINT_SP, -50);
			}
		}
	}

	//
	// Calculate bonus damage/defense from flat hit or skill
	//
	switch (type)
	{
		case EDamageType::DAMAGE_TYPE_NORMAL:
		case EDamageType::DAMAGE_TYPE_NORMAL_RANGE:
			if (pAttacker)
			{
				if (pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS))
					dam = dam * (100 + pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS)) / 100;

#ifdef ENABLE_SOUL_SYSTEM
				if (!IsPC() && pAttacker->IsActivatedRedSoul())
					pAttacker->UseSoulAttack(RED_SOUL);
#endif
			}

			dam = dam * (100 - MIN(99, GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS))) / 100;
			break;

		case EDamageType::DAMAGE_TYPE_MELEE:
		case EDamageType::DAMAGE_TYPE_RANGE:
		case EDamageType::DAMAGE_TYPE_FIRE:
		case EDamageType::DAMAGE_TYPE_ICE:
		case EDamageType::DAMAGE_TYPE_ELEC:
		case EDamageType::DAMAGE_TYPE_MAGIC:
		{
			if (pAttacker)
			{
				if (pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS))
					dam = dam * (100 + pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS)) / 100;

#ifdef ENABLE_SOUL_SYSTEM
				if (!IsPC() && pAttacker->IsActivatedBlueSoul())
					pAttacker->UseSoulAttack(BLUE_SOUL);
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
				if (pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE))
				{
					// Only when you make skill to boss
					if ((GetMobRank() >= MOB_RANK_BOSS) && (!IsStone()))
						dam = dam * (100 + pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE)) / 100;
				}

				if (GetPoint(POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE))
				{
					// Only when boss make skill to you
					if ((pAttacker->GetMobRank() >= MOB_RANK_BOSS) && (!pAttacker->IsStone()))
						dam = dam * (100 - MIN(99, GetPoint(POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE))) / 100;
				}
#endif
			}

			dam = dam * (100 - MIN(99, GetPoint(POINT_SKILL_DEFEND_BONUS))) / 100;
		}
		break;

		default:
			break;
	}

	//
	// Mana Shield (Black God Guardian)
	//
	if (IsAffectFlag(AFF_MANASHIELD))
	{
		// The smaller the POINT_MANASHIELD is, the better
		const int iDamageSPPart = dam / 3;
		const int iDamageToSP = iDamageSPPart * GetPoint(POINT_MANASHIELD) / 100;
		const int iSP = GetSP();

		// With SP, damage is unconditionally reduced by half
		if (iDamageToSP <= iSP)
		{
			PointChange(POINT_SP, -iDamageToSP);
			dam -= iDamageSPPart;
		}
		else
		{
			// My mental power is insufficient, so I have to cut my blood more.
			PointChange(POINT_SP, -GetSP());
			dam -= iSP * 100 / MAX(GetPoint(POINT_MANASHIELD), 1);
		}
	}

	//
	// Increased overall defense (mole item)
	//
	if (GetPoint(POINT_MALL_DEFBONUS) > 0)
	{
		const int dec_dam = MIN(200, dam * GetPoint(POINT_MALL_DEFBONUS) / 100);
		dam -= dec_dam;
	}

	if (pAttacker)
	{
		//
		// Increase overall attack power (mole item)
		//
		if (pAttacker->GetPoint(POINT_MALL_ATTBONUS) > 0)
		{
			const int add_dam = MIN(300, dam * pAttacker->GetLimitPoint(POINT_MALL_ATTBONUS) / 100);
			dam += add_dam;
		}

		if (pAttacker->IsPC())
		{
			const int iEmpire = pAttacker->GetEmpire();
			const long lMapIndex = pAttacker->GetMapIndex();
			const int iMapEmpire = SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(lMapIndex);

			// If you are from another empire, damage is reduced by 10%.
			if (iEmpire && iMapEmpire && iEmpire != iMapEmpire)
			{
				dam = dam * 9 / 10;
			}

			if (!IsPC() && GetMonsterDrainSPPoint())
			{
				const int iDrain = GetMonsterDrainSPPoint();

				if (iDrain <= pAttacker->GetSP())
					pAttacker->PointChange(POINT_SP, -iDrain);
				else
				{
					const int iSP = pAttacker->GetSP();
					pAttacker->PointChange(POINT_SP, -iSP);
				}
			}

		}
		else if (pAttacker->IsGuardNPC())
		{
			SetNoRewardFlag();
			Stun();
			return true;
		}

		//
		// Lord's Geumgangwon & Lion
		//
		if (pAttacker->IsPC() && CMonarch::Instance().IsPowerUp(pAttacker->GetEmpire()))
		{
			// 10% increased damage
			dam += dam / 10;
		}

		if (IsPC() && CMonarch::Instance().IsDefenceUp(GetEmpire()))
		{
			// 10% damage reduction
			dam -= dam / 10;
		}
	}
	//puAttr.Pop();

	if (!GetSectree() || GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK))
		return false;

	if (!IsPC())
	{
		if (m_pkParty && m_pkParty->GetLeader())
			m_pkParty->GetLeader()->SetLastAttacked(get_dword_time());
		else
			SetLastAttacked(get_dword_time());

		// Monster dialogue: when correct
		MonsterChat(MONSTER_CHAT_ATTACKED);
	}

	if (IsStun())
	{
		bool bImmediateDead = false;
		Dead(pAttacker, bImmediateDead);
		return true;
	}

	if (IsDead())
		return true;

	// Prevents death from poison attacks
	if (type == EDamageType::DAMAGE_TYPE_POISON)
	{
		if (GetHP() - dam <= 0)
		{
			dam = GetHP() - 1;
		}
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (type == EDamageType::DAMAGE_TYPE_BLEEDING)
	{
		if (GetHP() - dam <= 0)
		{
			dam = GetHP();
		}
	}
#endif

	// ------------------------
	// German premium mode
	// -----------------------
	if (pAttacker && pAttacker->IsPC())
	{
		const int iDmgPct = CHARACTER_MANAGER::Instance().GetUserDamageRate(pAttacker);
		dam = dam * iDmgPct / 100;
	}

	// STONE SKIN: Damage reduced by half
	if (IsMonster() && IsStoneSkinner())
	{
		if (GetHPPct() < GetMobTable().bStoneSkinPoint)
			dam /= 2;
	}

	//PROF_UNIT puRest1("Rest1");
	if (pAttacker)
	{
		// DEATH BLOW: 4x damage with probability (!? Only use monsters for the current event or siege)
		if (pAttacker->IsMonster() && pAttacker->IsDeathBlower())
		{
			if (pAttacker->IsDeathBlow())
			{
				if (number(JOB_WARRIOR, (JOB_MAX_NUM - 1)) == GetJob())
				{
					IsDeathBlow = true;
					dam = dam * 4;
				}
			}
		}

#ifdef ENABLE_BLUEDRAGON_RENEWAL
		if (pAttacker && pAttacker->IsBlueDragonDungeon(pAttacker->GetMapIndex()))
			dam = BlueDragon_Damage(this, pAttacker, dam);
#else
		dam = BlueDragon_Damage(this, pAttacker, dam);
#endif

		uint8_t damageFlag = 0;

		if (type == EDamageType::DAMAGE_TYPE_POISON)
			damageFlag = DAMAGE_POISON;
#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON)
		else if (type == EDamageType::DAMAGE_TYPE_BLEEDING)
			damageFlag = DAMAGE_BLEEDING;
#elif defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_MOB_BLEEDING_AS_POISON)
		else if (type == EDamageType::DAMAGE_TYPE_BLEEDING)
			damageFlag = DAMAGE_POISON;
#endif
		else if (type == EDamageType::DAMAGE_TYPE_FIRE)
			damageFlag = DAMAGE_FIRE;
		else
			damageFlag = DAMAGE_NORMAL;

		if (IsCritical)
			damageFlag |= DAMAGE_CRITICAL;

		if (IsPenetrate)
			damageFlag |= DAMAGE_PENETRATE;

		//Final damage correction
		const float damMul = this->GetDamMul();
		const float tempDam = static_cast<float>(dam);

		dam = static_cast<int>(tempDam * damMul + 0.5f);

#if defined(ENABLE_12ZI) || defined(ENABLE_MONSTER_BACK) || defined(ENABLE_LUCKY_EVENT)
		if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_COUNT))
		{
			damageFlag = DAMAGE_NORMAL;
			dam = 1;
		}
#endif

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
		if (pAttacker && pAttacker->IsPC()) {
			LPPARTY pCH = pAttacker->GetParty();
			if (pCH && pCH->GetMemberCount() == 3) {
				int cnt = 0;
				float addVal  = 0.0f;
				float percent = 0.50f;

				bool isPvPFighting = CPVPManager::Instance().IsFighting(pAttacker->GetPlayerID());
				// PvP-Frei missing
				bool isUnderGuildWar = false;
				CGuild * g1 = GetGuild();
				CGuild * g2 = pAttacker->GetGuild();
				if (g1 && g2)
					if (g1->UnderWar(g2->GetID()))
						isUnderGuildWar = true;

				FPartyCHCollector f;

				pCH->ForEachOnMapMember(f, pCH->GetLeaderCharacter()->GetMapIndex());

				for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it) {	//@fixme541
					LPCHARACTER partyMember = *it;
					if (partyMember) {
						LPITEM weapon = partyMember->GetWear(WEAR_WEAPON);
						cnt += (weapon && IS_SET(weapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON) ? 1 : 0);
						addVal = (cnt >= 3 ? percent : 0.0f);
					}
				}
				if (cnt >= 3) {
					if (!isPvPFighting && !isUnderGuildWar) {
						dam += (addVal * dam);
						/*ChatPacket(7, "dam: %d val + %.2f", dam, (addVal * dam));
						pAttacker->ChatPacket(7, "dam: %d val + %.2f", dam, (addVal * dam));*/
					}
					else {
						pAttacker->ChatPacket(7, "You're in PvP-Mode, Group-DMG-Weapon isn't working there!");
					}
				}
				else if (cnt < 3) {
					pAttacker->ChatPacket(7, "You need 3 Players to get the Group-Weapon Bonus!");
				}
			}
		}
#endif

#ifdef ENABLE_12ZI
		if ((IsMonster() || IsStone()) && (pAttacker && pAttacker->IsPC()))
		{
			if (IsZodiacStatue())
			{
				dam = 1;

				if (number(1, 30) == 15)
				{
					LPZODIAC pZodiac = CZodiacManager::Instance().FindByMapIndex(pAttacker->GetMapIndex());
					if (pZodiac)
					{
						SpawnZodiacStone(pZodiac);
					}
				}
			}
			else if (IsZodiacCannon())
			{
				dam = 1;
				CanonDamage();
			}
			else if (GetRaceNum() >= 2900 && GetRaceNum() <= 2908)
			{
				if (number(0, 160) <= 7)
				{
					LPZODIAC pZodiac = CZodiacManager::Instance().FindByMapIndex(pAttacker->GetMapIndex());
					if (pZodiac)
					{
						SpawnZodiacGroup(pZodiac);
					}
				}
			}
		}
#endif

		if (pAttacker)
			SendDamagePacket(pAttacker, dam, damageFlag);

		if (test_server)
		{
			int iTmpPercent = 0; // @fixme136
			if (GetMaxHP() >= 0)
				iTmpPercent = (GetHP() * 100) / GetMaxHP();

			if (pAttacker)
			{
				pAttacker->ChatPacket(CHAT_TYPE_INFO, "-> %s, DAM %d HP %d(%d%%) %s%s",
					GetName(),
					dam,
					GetHP(),
					iTmpPercent,
					IsCritical ? "crit " : "",
					IsPenetrate ? "pene " : "",
					IsDeathBlow ? "deathblow " : "");
			}

			ChatPacket(CHAT_TYPE_PARTY, "<- %s, DAM %d HP %d(%d%%) %s%s",
				pAttacker ? pAttacker->GetName() : 0,
				dam,
				GetHP(),
				iTmpPercent,
				IsCritical ? "crit " : "",
				IsPenetrate ? "pene " : "",
				IsDeathBlow ? "deathblow " : "");
		}

		if (m_bDetailLog)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;915;%s;%d;%d;%d]", pAttacker->GetName(), (uint32_t)pAttacker->GetVID(), pAttacker->GetX(), pAttacker->GetY());
		}
	}

#ifdef ENABLE_DEFENSE_WAVE
	if (GetRaceNum() == 20437)	//Reparaturholz
		dam = 1;
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	if (type != EDamageType::DAMAGE_TYPE_POISON)
	{
		if (IsPC())
			pAttacker->UpdateExtBattlePassMissionProgress(DAMAGE_PLAYER, dam, GetLevel());
		else
			pAttacker->UpdateExtBattlePassMissionProgress(DAMAGE_MONSTER, dam, GetRaceNum());
	}
#endif

	//
	// !!!!!!!!! The actual HP reduction part !!!!!!!!!
	//
	if (!cannot_dead)
	{
		if (GetHP() - dam <= 0) // @fixme137
			dam = GetHP();

		PointChange(POINT_HP, -dam, false);

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		if (CMeleyLairManager::Instance().IsMeleyMap(pAttacker->GetMapIndex()))
		{
			switch (GetRaceNum())
			{
				case MELEY_GUILD_STATUE_VNUM:
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
				case MELEY_PARTY_STATUE_VNUM:
#	endif
				{
					if (GetHP() == 0)
						PointChange(POINT_HP, 1, false);
				}
				break;

				default:
					break;
			}
		}
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
		if (pAttacker)
			pAttacker->SetLastDamage(dam);
#endif
	}

	//puRest1.Pop();
	//PROF_UNIT puRest2("Rest2");
	if (pAttacker && dam > 0 && IsNPC())
	{
		//PROF_UNIT puRest20("Rest20");
		TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());

		if (it == m_map_kDamage.end())
		{
			m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(dam, 0)));
			it = m_map_kDamage.find(pAttacker->GetVID());
		}
		else
		{
			it->second.iTotalDamage += dam;
		}
		//puRest20.Pop();

		//PROF_UNIT puRest21("Rest21");
#if defined(ENABLE_DEFENSE_WAVE) || defined(ENABLE_MONSTER_BACK) || defined(ENABLE_LUCKY_EVENT)
		if (!IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NORECOVERY)) // Mobs will not heal if they have the AIFLAG NORECOVERY
#endif
		{
			StartRecoveryEvent();
		}
		//puRest21.Pop();

		//PROF_UNIT puRest22("Rest22");
		UpdateAggrPointEx(pAttacker, type, dam, it->second);
		//puRest22.Pop();
	}
	//puRest2.Pop();

#ifdef ENABLE_DAWNMIST_DUNGEON
	if (pAttacker && pAttacker->IsPC() && GetRaceNum() == CDawnMistDungeon::TEMPLE_GUARDIAN && GetMapIndex() == MAP_MT_THUNDER_DUNGEON)
		SetLastAttackedTime(get_global_time());
#endif

	//PROF_UNIT puRest3("Rest3");
	if (GetHP() <= 0)
	{
		Stun();

		if (pAttacker && !pAttacker->IsNPC())
			m_dwKillerPID = pAttacker->GetPlayerID();
		else
			m_dwKillerPID = 0;
	}

#ifdef ENABLE_12ZI
	if (pAttacker && pAttacker->IsZodiacCannon())
	{
		EffectPacket(SE_DEAPO_BOOM);
		pAttacker->Dead();
	}
#endif
#ifdef ENABLE_DEFENSE_WAVE
	if (IsMast() && GetDungeon())
		GetDungeon()->UpdateMastHP();
#endif

	return false;
}

void CHARACTER::DistributeHP(LPCHARACTER pkKiller)
{
	if (pkKiller && pkKiller->GetDungeon()) // There are no dumplings in the dungeon.
		return;
}

#define NEW_GET_LVDELTA(me, victim) aiPercentByDeltaLev[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
typedef long double rate_t;
#ifdef ENABLE_YOHARA_SYSTEM
static void GiveExp(LPCHARACTER from, LPCHARACTER to, int iExp, int iConquerorExp)
#else
static void GiveExp(LPCHARACTER from, LPCHARACTER to, int iExp)
#endif
{
	if (!from || !to)
		return;

	if (test_server && iExp < 0)
	{
		to->ChatPacket(CHAT_TYPE_INFO, "exp(%d) overflow", iExp);
		return;
	}

	auto rExp = static_cast<rate_t>(iExp);
#ifdef ENABLE_YOHARA_SYSTEM
	auto rConquerorExp = static_cast<rate_t>(iConquerorExp);
#endif

	// decrease/increase exp based on player<>mob level
	const auto lvFactor = static_cast<rate_t>(NEW_GET_LVDELTA(to->GetLevel(), from->GetLevel())) / 100.0L;
	rExp *= lvFactor;
#ifdef ENABLE_YOHARA_SYSTEM
	rConquerorExp *= lvFactor;
#endif

	// start calculating rate exp bonus
	const auto iBaseExp = iExp;
#ifdef ENABLE_YOHARA_SYSTEM
	const auto iBaseConquerorExp = iConquerorExp;
#endif

	auto rateFactor = 100.0L;

	// Fortune-telling, company experience event applied
	rateFactor += static_cast<rate_t>(CPrivManager::Instance().GetPriv(to, PRIV_EXP_PCT));

	// Labor day medal
	if (to->IsEquipUniqueItem(UNIQUE_ITEM_LARBOR_MEDAL))
		rateFactor += 20.0L;

	// Social Tower Experience Bonus
	if (to->IsDevilTowerDungeon(to->GetMapIndex()))
		rateFactor += 20.0L;

	// Double item experience
	if (to->GetPoint(POINT_EXP_DOUBLE_BONUS))
	{
		if (number(1, 100) <= to->GetPoint(POINT_EXP_DOUBLE_BONUS))
			rateFactor += 30.0L;
	}

	// Ring of Experience (2 hours long)
	if (to->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_EXP) || to->IsEquipUniqueItem(39002) || to->IsEquipUniqueItem(72303))
		rateFactor += 50.0L;

	if (to->IsEquipUniqueItem(71180))
		rateFactor += 80.0L;
	if (to->IsEquipUniqueItem(38058) || to->IsEquipUniqueItem(71181))
		rateFactor += 100.0L;
	if (to->IsEquipUniqueItem(72062))
		rateFactor += 200.0L;

	// Item Mall: EXP payment
	if (to->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		rateFactor += 50.0L;
	if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP))
		rateFactor += 50.0L;

	// Marriage bonus
	rateFactor += static_cast<rate_t>(to->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_EXP_BONUS));
	rateFactor += static_cast<rate_t>(to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP));
	rateFactor += static_cast<rate_t>(to->GetPoint(POINT_MALL_EXPBONUS));

	rExp = std::max<rate_t>(0.0L, rExp);
	rateFactor = std::max<rate_t>(100.0L, rateFactor);

	// Apply calculated rate bonus
	rExp *= (rateFactor / 100.0L);

	const auto iFinalExp = static_cast<int>(rExp);

#ifdef ENABLE_YOHARA_SYSTEM
	rConquerorExp = std::max<rate_t>(0.0L, rConquerorExp);
	rConquerorExp *= (rateFactor / 100.0L);
	const auto iFinalConquerorExp = static_cast<int>(rConquerorExp);
#endif

	if (test_server)
		to->ChatPacket(CHAT_TYPE_INFO, "base_exp(%d) * rate(%Lf) = exp(%d)", iBaseExp, rateFactor / 100.0L, iFinalExp);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	CGrowthPetSystem* pGrowthPetSystem = to->GetGrowthPetSystem();
	if (pGrowthPetSystem && pGrowthPetSystem->IsActivePet())
	{
		const auto dwPetExp = static_cast<uint32_t>(iFinalExp / 10);
		pGrowthPetSystem->SetExp(dwPetExp, 0);
	}
#endif

#ifdef ENABLE_ANTI_EXP_RING
	CAffect* pAffect = to->FindAffect(AFFECT_EXPRING);
	if (pAffect)
	{
		LPITEM expitem = to->FindItemByID(pAffect->dwFlag);//8 val0
		if (expitem && expitem->GetOwner() == to)
		{
			if (expitem->GetSocket(1) - iExp <= 0)
			{
				int cell = expitem->GetCell();
				to->RemoveAffect(pAffect);
				expitem->Lock(false);
				expitem->SetSocket(0, false);
				//to->EffectPacket(SE_AUTO_EXP);	//@fixme507 -> Delete?
				ITEM_MANAGER::Instance().RemoveItem(expitem, "EXP_EXPIRED");
				to->SyncQuickslot(QUICKSLOT_TYPE_ITEM, cell, 255);
			}
			else
			{
				expitem->SetSocket(1, expitem->GetSocket(1) - iExp);
				//to->EffectPacket(SE_AUTO_EXP);	//@fixme507 -> Delete?
			}
			return;
		}
	}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
	if (!to->GetMultiStatus())
		return;
#endif

	// set
#ifdef ENABLE_YOHARA_SYSTEM
	if (iBaseConquerorExp > 0 && to->GetConquerorLevel() >= 1)
	{
		to->PointChange(POINT_CONQUEROR_EXP, iFinalConquerorExp, true);
		from->CreateFly(FLY_CONQUEROR_EXP, to);
	}
#endif

	if (iBaseExp > 0)
	{
		to->PointChange(POINT_EXP, iFinalExp, true);
		from->CreateFly(FLY_EXP, to);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		to->UpdateExtBattlePassMissionProgress(EXP_COLLECT, iFinalExp, from->GetRaceNum());
#endif
	}

	// Marriage
	{
		const LPCHARACTER& you = to->GetMarryPartner();
		if (you)
		{
			// sometimes, this overflows
			auto dwUpdatePoint = static_cast<uint32_t>(2000.0L / to->GetLevel() / to->GetLevel() / 3) * static_cast<uint32_t>(iFinalExp);

			if (to->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0 ||
				you->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0)
			{
				dwUpdatePoint *= 3;
			}

			marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(to->GetPlayerID());

			// DIVORCE_NULL_BUG_FIX
			if (pMarriage && pMarriage->IsNear())
				pMarriage->Update(dwUpdatePoint);
			// END_OF_DIVORCE_NULL_BUG_FIX
		}
	}
}

namespace NPartyExpDistribute
{
	struct FPartyTotaler
	{
		int total;
		int member_count;
		int x, y;

		FPartyTotaler(LPCHARACTER center)
			: total(0), member_count(0), x(center->GetX()), y(center->GetY())
		{};

		void operator () (LPCHARACTER ch)
		{
			if (ch && (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE))
			{
				total += __GetPartyExpNP(ch->GetLevel());

				++member_count;
			}
		}
	};

	struct FPartyDistributor
	{
		int total;
		LPCHARACTER c;
		int x, y;
		uint32_t _iExp;
#ifdef ENABLE_YOHARA_SYSTEM
		uint32_t _iConquerorExp;
#endif
		int m_iMode;
		int m_iMemberCount;

		FPartyDistributor(LPCHARACTER center, int member_count, int total, uint32_t iExp, 
#ifdef ENABLE_YOHARA_SYSTEM
			uint32_t iConquerorExp,
#endif
			int iMode)
			: total(total), c(center), x(center->GetX()), y(center->GetY()), _iExp(iExp), 
#ifdef ENABLE_YOHARA_SYSTEM
				_iConquerorExp(iConquerorExp),
#endif
			m_iMode(iMode), m_iMemberCount(member_count)
			{
				if (m_iMemberCount == 0)
					m_iMemberCount = 1;
			};

		void operator () (LPCHARACTER ch)
		{
			if (!ch)
				return;

			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
			{
				uint32_t iExp2 = 0;
#ifdef ENABLE_YOHARA_SYSTEM
				uint32_t iConquerorExp2 = 0;
#endif

				switch (m_iMode)
				{
					case PARTY_EXP_DISTRIBUTION_NON_PARITY:
						{
							iExp2 = static_cast<uint32_t>(_iExp * (float)__GetPartyExpNP(ch->GetLevel()) / total);
#ifdef ENABLE_YOHARA_SYSTEM
							iConquerorExp2 = static_cast<uint32_t>(_iConquerorExp * (float)__GetPartyExpNP(ch->GetLevel()) / total);
#endif
						}
						break;

					case PARTY_EXP_DISTRIBUTION_PARITY:
						{
							iExp2 = _iExp / m_iMemberCount;
#ifdef ENABLE_YOHARA_SYSTEM
							iConquerorExp2 = _iConquerorExp / m_iMemberCount;
#endif
						}
						break;

#ifdef ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL
					case PARTY_EXP_DISTRIBUTION_EQUAL:
						{
							iExp2 = _iExp;
#	ifdef ENABLE_YOHARA_SYSTEM
							iConquerorExp2 = _iConquerorExp;
#	endif
						}
						break;
#endif

					default:
						sys_err("Unknown party exp distribution mode %d", m_iMode);
						return;
				}

#ifdef ENABLE_YOHARA_SYSTEM
				GiveExp(c, ch, iExp2, iConquerorExp2);
#else
				GiveExp(c, ch, iExp2);
#endif
			}
		}
	};
}

typedef struct SDamageInfo
{
	int iDam;
	LPCHARACTER pAttacker;
	LPPARTY pParty;

	void Clear()
	{
		pAttacker = nullptr;
		pParty = nullptr;
	}

#ifdef ENABLE_YOHARA_SYSTEM
	inline void Distribute(LPCHARACTER ch, int iExp, int iConquerorExp)
#else
	inline void Distribute(LPCHARACTER ch, int iExp)
#endif
	{
		if (!ch)
			return;

		if (pAttacker)
		{
#ifdef ENABLE_YOHARA_SYSTEM
			GiveExp(ch, pAttacker, iExp, iConquerorExp);
#else
			GiveExp(ch, pAttacker, iExp);
#endif
		}
		else if (pParty)
		{
			NPartyExpDistribute::FPartyTotaler f(ch);
			pParty->ForEachOnMapMember(f, ch->GetMapIndex());	//@fixme522

			if (pParty->IsPositionNearLeader(ch))
				iExp = iExp * (100 + pParty->GetExpBonusPercent()) / 100;

			if (test_server)
			{
				if (quest::CQuestManager::Instance().GetEventFlag("exp_bonus_log") && pParty->GetExpBonusPercent())
					pParty->ChatPacketToAllMember(CHAT_TYPE_INFO, "exp party bonus %d%%", pParty->GetExpBonusPercent());
			}

			// Drive experience (the party's earned experience is subtracted by 5% and given first)
			if (pParty->GetExpCentralizeCharacter())
			{
				const LPCHARACTER& tch = pParty->GetExpCentralizeCharacter();
				if (!tch)
					return;

				if (DISTANCE_APPROX(ch->GetX() - tch->GetX(), ch->GetY() - tch->GetY()) <= PARTY_DEFAULT_RANGE)
				{
					const int iExpCenteralize = static_cast<int>(iExp * 0.05f);
					iExp -= iExpCenteralize;

#ifdef ENABLE_YOHARA_SYSTEM
					int iConquerorExpCenteralize = static_cast<int>(iExp * 0.05f);
					iConquerorExp -= iConquerorExpCenteralize;
					GiveExp(ch, pParty->GetExpCentralizeCharacter(), iExpCenteralize, iConquerorExpCenteralize);
#else
					GiveExp(ch, pParty->GetExpCentralizeCharacter(), iExpCenteralize);
#endif
				}
			}

#ifdef ENABLE_YOHARA_SYSTEM
			NPartyExpDistribute::FPartyDistributor fDist(ch, f.member_count, f.total, iExp, iConquerorExp, pParty->GetExpDistributionMode());
#else
			NPartyExpDistribute::FPartyDistributor fDist(ch, f.member_count, f.total, iExp, pParty->GetExpDistributionMode());
#endif
			pParty->ForEachOnMapMember(fDist, ch->GetMapIndex());	//@fixme522
		}
	}
} TDamageInfo;

#ifdef ENABLE_KILL_EVENT_FIX
LPCHARACTER CHARACTER::GetMostAttacked()
{
	int iMostDam = -1;
	LPCHARACTER pkChrMostAttacked = nullptr;
	auto it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end())
	{
		// getting information from the iterator
		const VID& c_VID = it->first;
		const int iDam = it->second.iTotalDamage;

		// increasing the iterator
		++it;

		// finding the character from his vid
		LPCHARACTER pAttacker = CHARACTER_MANAGER::Instance().Find(c_VID);

		// if the attacked is now offline
		if (!pAttacker)
			continue;

		// if the attacker is not a player
		if (pAttacker->IsNPC())
			continue;

		// if the player is too far
		if (DISTANCE_APPROX(GetX() - pAttacker->GetX(), GetY() - pAttacker->GetY()) > 5000)
			continue;

		if (iDam > iMostDam)
		{
			pkChrMostAttacked = pAttacker;
			iMostDam = iDam;
		}
	}

	return pkChrMostAttacked;
}
#endif

LPCHARACTER CHARACTER::DistributeExp()
{
#ifdef ENABLE_YOHARA_SYSTEM
	int iExpToDistribute = GetExp();
	if (iExpToDistribute <= 0)
		iExpToDistribute = 0;

	int iExpConquerorToDistribute = GetConquerorExp();
	if (iExpConquerorToDistribute <= 0)
		iExpConquerorToDistribute = 0;
#else
	int iExpToDistribute = GetExp();
	if (iExpToDistribute <= 0)
		return nullptr;
#endif

	int iTotalDam = 0;
	LPCHARACTER pkChrMostAttacked = nullptr;
	int iMostDam = 0;

	typedef std::vector<TDamageInfo> TDamageInfoTable;
	TDamageInfoTable damage_info_table;
	std::map<LPPARTY, TDamageInfo> map_party_damage;

	damage_info_table.reserve(m_map_kDamage.size());

	TDamageMap::iterator it = m_map_kDamage.begin();

	// First, filter out people who are not around. (50m)
	while (it != m_map_kDamage.end())
	{
		const VID& c_VID = it->first;
		const int iDam = it->second.iTotalDamage;

		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::Instance().Find(c_VID);

		// Can NPCs hit you? -.-;
		if (!pAttacker || pAttacker->IsNPC() || DISTANCE_APPROX(GetX() - pAttacker->GetX(), GetY() - pAttacker->GetY()) > 5000)
			continue;

		iTotalDam += iDam;
		if (!pkChrMostAttacked || iDam > iMostDam)
		{
			pkChrMostAttacked = pAttacker;
			iMostDam = iDam;
		}

		if (pAttacker->GetParty())
		{
			const std::map<LPPARTY, TDamageInfo>::iterator it2 = map_party_damage.find(pAttacker->GetParty());
			if (it2 == map_party_damage.end())
			{
				TDamageInfo di{};
				di.iDam = iDam;
				di.pAttacker = nullptr;
				di.pParty = pAttacker->GetParty();
				map_party_damage.insert(std::make_pair(di.pParty, di));
			}
			else
			{
				it2->second.iDam += iDam;
			}
		}
		else
		{
			TDamageInfo di{};

			di.iDam = iDam;
			di.pAttacker = pAttacker;
			di.pParty = nullptr;

			//sys_log(0, "__ pq_damage %s %d", pAttacker->GetName(), iDam);
			//pq_damage.push(di);
			damage_info_table.emplace_back(di);
		}
	}

	for (std::map<LPPARTY, TDamageInfo>::iterator it3 = map_party_damage.begin(); it3 != map_party_damage.end(); ++it3)
	{
		damage_info_table.emplace_back(it3->second);
		//sys_log(0, "__ pq_damage_party [%u] %d", it->second.pParty->GetLeaderPID(), it->second.iDam);
	}

	SetExp(0);
#ifdef ENABLE_YOHARA_SYSTEM
	SetConquerorExp(0);
#endif
	//m_map_kDamage.clear();

	if (iTotalDam == 0) // Returns if damage dealt is 0
		return nullptr;

	if (m_pkChrStone) // If there is a stone, half of the experience is passed to the stone.
	{
		//sys_log(0, "__ Give half to Stone : %d", iExpToDistribute>>1);
		const int iExp = iExpToDistribute >> 1;
		m_pkChrStone->SetExp(m_pkChrStone->GetExp() + iExp);
		iExpToDistribute -= iExp;
	}

	sys_log(1, "%s total exp: %d, damage_info_table.size() == %d, TotalDam %d",
		GetName(), iExpToDistribute, damage_info_table.size(), iTotalDam);
	//sys_log(1, "%s total exp: %d, pq_damage.size() == %d, TotalDam %d",
	//GetName(), iExpToDistribute, pq_damage.size(), iTotalDam);

	if (damage_info_table.empty())
		return nullptr;

	// The person who inflicts the most damage recovers HP.
	DistributeHP(pkChrMostAttacked); // dumpling system

	{
		// The person or party that inflicts the most damage eats 20% of the total experience + the amount of experience it hits.
		TDamageInfoTable::iterator di = damage_info_table.begin();
		{
			TDamageInfoTable::iterator it4;

			for (it4 = damage_info_table.begin(); it4 != damage_info_table.end(); ++it4)
			{
				if (it4->iDam > di->iDam)
					di = it4;
			}
		}

		int iExp = iExpToDistribute / 5;
		iExpToDistribute -= iExp;

#ifdef ENABLE_YOHARA_SYSTEM
		int	iConquerorExp = iExpConquerorToDistribute / 5;
		iExpConquerorToDistribute -= iConquerorExp;
#endif

		float fPercent = static_cast<float>(di->iDam) / iTotalDam;

		if (fPercent > 1.0f)
		{
			sys_err("DistributeExp percent over 1.0 (fPercent %f name %s)", fPercent, di->pAttacker->GetName());
			fPercent = 1.0f;
		}

		iExp += static_cast<int>(iExpToDistribute * fPercent);
#ifdef ENABLE_YOHARA_SYSTEM
		iConquerorExp += static_cast<int>(iExpConquerorToDistribute * fPercent);
#endif

		//sys_log(0, "%s given exp percent %.1f + 20 dam %d", GetName(), fPercent * 100.0f, di.iDam);

#ifdef ENABLE_YOHARA_SYSTEM
		di->Distribute(this, iExp, iConquerorExp);
#else
		di->Distribute(this, iExp);
#endif

		// If you eat 100%, return it.
		if (fPercent == 1.0f)
			return pkChrMostAttacked;

		di->Clear();
	}

	{
		// The remaining 80% of the experience is distributed.
		TDamageInfoTable::iterator it5;

		for (it5 = damage_info_table.begin(); it5 != damage_info_table.end(); ++it5)
		{
			TDamageInfo& di = *it5;

			float fPercent = static_cast<float>(di.iDam) / iTotalDam;

			if (fPercent > 1.0f)
			{
				sys_err("DistributeExp percent over 1.0 (fPercent %f name %s)", fPercent, di.pAttacker->GetName());
				fPercent = 1.0f;
			}

			//sys_log(0, "%s given exp percent %.1f dam %d", GetName(), fPercent * 100.0f, di.iDam);
#ifdef ENABLE_YOHARA_SYSTEM
			di.Distribute(this, static_cast<int>(iExpToDistribute* fPercent), static_cast<int>(iExpConquerorToDistribute* fPercent));
#else
			di.Distribute(this, (int) (iExpToDistribute * fPercent));
#endif
		}
	}

	return pkChrMostAttacked;
}

int CHARACTER::GetArrowAndBow(LPITEM * ppkBow, LPITEM * ppkArrow, int iArrowCount/* = 1 */)
{
	LPITEM pkBow;
	if (!(pkBow = GetWear(WEAR_WEAPON)) || pkBow->GetSubType() != WEAPON_BOW)
		return 0;

	LPITEM pkArrow;
#ifdef ENABLE_QUIVER_SYSTEM
	if (!(pkArrow = GetWear(WEAR_ARROW)) || pkArrow->GetType() != ITEM_WEAPON || (pkArrow->GetSubType() != WEAPON_ARROW && pkArrow->GetSubType() != WEAPON_QUIVER))
#else
	if (!(pkArrow = GetWear(WEAR_ARROW)) || pkArrow->GetType() != ITEM_WEAPON || pkArrow->GetSubType() != WEAPON_ARROW)
#endif
		return 0;

#ifdef ENABLE_QUIVER_SYSTEM
	if (pkArrow->GetSubType() == WEAPON_QUIVER)
		iArrowCount = MIN(iArrowCount, pkArrow->GetSocket(0) - time(0));
	else
		iArrowCount = MIN(iArrowCount, pkArrow->GetCount());
#else
	iArrowCount = MIN(iArrowCount, pkArrow->GetCount());
#endif

	*ppkBow = pkBow;
	*ppkArrow = pkArrow;

	return iArrowCount;
}

void CHARACTER::UseArrow(LPITEM pkArrow, uint32_t dwArrowCount)
{
	if (!pkArrow)
		return;

#ifdef ENABLE_QUIVER_SYSTEM
	if (pkArrow->GetSubType() == WEAPON_QUIVER)
		return;
#endif

	int iCount = pkArrow->GetCount();
	const uint32_t dwVnum = pkArrow->GetVnum();
	iCount = iCount - MIN(iCount, dwArrowCount);
	pkArrow->SetCount(iCount);

	if (iCount == 0)
	{
		LPITEM pkNewArrow = FindSpecifyItem(dwVnum);

		sys_log(0, "UseArrow : FindSpecifyItem %u %p", dwVnum, get_pointer(pkNewArrow));

		if (pkNewArrow)
			EquipItem(pkNewArrow);
	}
}

class CFuncShoot
{
	public:
		LPCHARACTER m_me;
		uint8_t m_bType;
		bool m_bSucceed;

		CFuncShoot(LPCHARACTER ch, uint8_t bType) : m_me(ch), m_bType(bType), m_bSucceed(FALSE)
		{
		}

		void operator () (uint32_t dwTargetVID)
		{
			if (m_bType > 1)
			{
				if (g_bSkillDisable)
					return;

				m_me->m_SkillUseInfo[m_bType].SetMainTargetVID(dwTargetVID);
				/*if (m_bType == SKILL_BIPABU || m_bType == SKILL_KWANKYEOK)
					m_me->m_SkillUseInfo[m_bType].ResetHitCount();*/
			}

			LPCHARACTER pkVictim = CHARACTER_MANAGER::Instance().Find(dwTargetVID);

			if (!pkVictim)
				return;

			// cannot attack
			if (!battle_is_attackable(m_me, pkVictim))
				return;

			if (m_me->IsNPC())
			{
				if (DISTANCE_APPROX(m_me->GetX() - pkVictim->GetX(), m_me->GetY() - pkVictim->GetY()) > 5000)
					return;
			}

#ifdef ENABLE_CHECK_SKILL_HACK
			if (m_me->IsPC() && m_bType > 0 && m_me->IsSkillCooldown(m_bType, static_cast<float>(m_me->GetSkillPower(m_bType) / 100.0f)))
				return;
#endif

#ifdef ENABLE_AFTERDEATH_SHIELD
			if (m_me->IsAffectFlag(AFF_AFTERDEATH_SHIELD))
				m_me->RemoveShieldAffect();
#endif

			LPITEM pkBow, pkArrow;

			switch (m_bType)
			{
				case 0: // normal bow
				{
					int iDam = 0;

					if (m_me->IsPC())
					{
						if (m_me->GetJob() != JOB_ASSASSIN)
							return;

						if (0 == m_me->GetArrowAndBow(&pkBow, &pkArrow))
							return;

						if (m_me->GetSkillGroup() != 0)
						{
							if (!m_me->IsNPC() && m_me->GetSkillGroup() != 2)
							{
								if (m_me->GetSP() < 5)
									return;

								m_me->PointChange(POINT_SP, -5);
							}
						}

						iDam = CalcArrowDamage(m_me, pkVictim, pkBow, pkArrow);
						m_me->UseArrow(pkArrow, 1);

#if defined(ENABLE_CSHIELD) && defined(ENABLE_CHECK_ATTACKSPEED_HACK)
						uint8_t result = m_me->GetCShield()->CheckAttackspeedBowHack(ani_attack_speed(m_me), static_cast<long long>(m_me->GetPoint(POINT_ATT_SPEED)), get_dword_time());
						if (result == 1)
						{
							LPDESC d = m_me->GetDesc();
							if (d)
							{
								if (d->DelayedDisconnect(3))
								{
									LogManager::Instance().HackLog("Attackspeed Hack", m_me);
								}
							}
							iDam = 0;
						}
						else if (result == 2)
							iDam = 0;
#endif

						// check speed hack
						const uint32_t dwCurrentTime = get_dword_time();
						if (IS_SPEED_HACK(m_me, pkVictim, dwCurrentTime))
							iDam = 0;
					}
					else
						iDam = CalcMeleeDamage(m_me, pkVictim);

					NormalAttackAffect(m_me, pkVictim);

					// Damage calculation
					iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_BOW)) / 100;

					//sys_log(0, "%s arrow %s dam %d", m_me->GetName(), pkVictim->GetName(), iDam);

					m_me->OnMove(true);
					pkVictim->OnMove();

					if (pkVictim->CanBeginFight())
						pkVictim->BeginFight(m_me);

					pkVictim->Damage(m_me, iDam, EDamageType::DAMAGE_TYPE_NORMAL_RANGE);
					// End of hit calculation section
				}
				break;

				case 1:	// common magic
				{
					int iDam = 0;

					if (m_me->IsPC())
						return;

					iDam = CalcMagicDamage(m_me, pkVictim);

					NormalAttackAffect(m_me, pkVictim);

					// Damage calculation
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
					int resist_magic = pkVictim->GetPoint(POINT_RESIST_MAGIC);
#	ifdef ENABLE_ATTR_6TH_7TH_EXTEND
					// Magic Resistance (+x% of total value)
					if (pkVictim->GetPoint(POINT_RESIST_PER_MAGIC) > 0)
						resist_magic = MINMAX(0, resist_magic + (resist_magic * pkVictim->GetPoint(POINT_RESIST_PER_MAGIC)) / 100, 100);
#	endif

					const int resist_magic_reduction = MINMAX(0, (m_me->GetJob() == JOB_SURA) ? m_me->GetPoint(POINT_RESIST_MAGIC_REDUCTION) / 2 : m_me->GetPoint(POINT_RESIST_MAGIC_REDUCTION), 50);
					const int total_res_magic = MINMAX(0, resist_magic - resist_magic_reduction, 100);
					iDam = iDam * (100 - total_res_magic) / 100;
#else
					iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_MAGIC)) / 100;
#endif

					//sys_log(0, "%s arrow %s dam %d", m_me->GetName(), pkVictim->GetName(), iDam);

					m_me->OnMove(true);
					pkVictim->OnMove();

					if (pkVictim->CanBeginFight())
						pkVictim->BeginFight(m_me);

					pkVictim->Damage(m_me, iDam, EDamageType::DAMAGE_TYPE_MAGIC);
					// End of hit calculation section
				}
				break;

				case SKILL_YEONSA: // Repetitive Shot
				{
					//int iUseArrow = 2 + (m_me->GetSkillPower(SKILL_YEONSA) *6/100);
					constexpr int iUseArrow = 1;

					// When calculating only the total
					{
						if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
						{
							m_me->OnMove(true);
							pkVictim->OnMove();

							if (pkVictim->CanBeginFight())
								pkVictim->BeginFight(m_me);

							m_me->ComputeSkill(m_bType, pkVictim);
							m_me->UseArrow(pkArrow, iUseArrow);

							if (pkVictim->IsDead())
								break;

						}
						else
							break;
					}
				}
				break;


				case SKILL_KWANKYEOK:
				{
					constexpr int iUseArrow = 1;

					if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s kwankeyok %s", m_me->GetName(), pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);
						m_me->UseArrow(pkArrow, iUseArrow);
					}
				}
				break;

#ifdef ENABLE_NINETH_SKILL
				case SKILL_PUNGLOEPO:
				{
					const int iUseArrow = 1;

					if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s 9th skill ninja archer %s", m_me->GetName(), pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);
						m_me->UseArrow(pkArrow, iUseArrow);
					}
				}
				break;
#endif

				case SKILL_GIGUNG:
				{
					constexpr int iUseArrow = 1;
					if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s gigung %s", m_me->GetName(), pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);
						m_me->UseArrow(pkArrow, iUseArrow);
					}
				}
				break;

				case SKILL_HWAJO:
				{
					constexpr int iUseArrow = 1;
					if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s hwajo %s", m_me->GetName(), pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);
						m_me->UseArrow(pkArrow, iUseArrow);
					}
				}
				break;

				case SKILL_HORSE_WILDATTACK_RANGE:
				{
					constexpr int iUseArrow = 1;
					if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s horse_wildattack %s", m_me->GetName(), pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);
						m_me->UseArrow(pkArrow, iUseArrow);
					}
				}
				break;

				case SKILL_MARYUNG:
				//case SKILL_GUMHWAN:
				case SKILL_TUSOK:
				case SKILL_BIPABU:
#ifdef ENABLE_PVP_BALANCE
				case SKILL_PAERYONG: // Shaman skill
#endif
				case SKILL_YONGBI:		//@fixme474
				case SKILL_NOEJEON:
				case SKILL_GEOMPUNG:
				case SKILL_SANGONG:
				case SKILL_MAHWAN:
				case SKILL_PABEOB:
#ifdef ENABLE_NINETH_SKILL
				case SKILL_ILGWANGPYO: // 177 ninja job 1
				case SKILL_MABEOBAGGWI: // 180 sura job 2
				case SKILL_METEO: // 181 shaman job 1
#endif
				//case SKILL_CURSE:
				{
					m_me->OnMove(true);
					pkVictim->OnMove();

					if (pkVictim->CanBeginFight())
						pkVictim->BeginFight(m_me);

					sys_log(0, "%s - Skill %d -> %s", m_me->GetName(), m_bType, pkVictim->GetName());
					m_me->ComputeSkill(m_bType, pkVictim);
				}
				break;

				case SKILL_CHAIN:
				{
					m_me->OnMove(true);
					pkVictim->OnMove();

					if (pkVictim->CanBeginFight())
						pkVictim->BeginFight(m_me);

					sys_log(0, "%s - Skill %d -> %s", m_me->GetName(), m_bType, pkVictim->GetName());
					m_me->ComputeSkill(m_bType, pkVictim);
					// TODO shudder with multiple people
				}
				break;

				//@fixme474
				/*case SKILL_YONGBI:
				{
					m_me->OnMove(true);
				}
				break;*/

				/*case SKILL_BUDONG:
				{
					m_me->OnMove(true);
					pkVictim->OnMove();

					uint32_t * pdw;
					uint32_t dwEI = AllocEventInfo(sizeof(uint32_t) * 2, &pdw);
					pdw[0] = m_me->GetVID();
					pdw[1] = pkVictim->GetVID();

					event_create(budong_event_func, dwEI, PASSES_PER_SEC(1));
				}
				break;*/

				default:
					sys_err("CFuncShoot: I don't know this type [%d] of range attack.", (int) m_bType);
					break;
			}

			m_bSucceed = TRUE;
		}
};

bool CHARACTER::Shoot(uint8_t bType)
{
	sys_log(1, "Shoot %s type %u flyTargets.size %zu", GetName(), bType, m_vec_dwFlyTargets.size());

	if (!CanMove())
	{
		return false;
	}

	CFuncShoot f(this, bType);

	if (m_dwFlyTargetID != 0)
	{
		f(m_dwFlyTargetID);
		m_dwFlyTargetID = 0;
	}

	f = std::for_each(m_vec_dwFlyTargets.begin(), m_vec_dwFlyTargets.end(), f);
	m_vec_dwFlyTargets.clear();

	return f.m_bSucceed;
}

void CHARACTER::FlyTarget(uint32_t dwTargetVID, long x, long y, uint8_t bHeader)
{
	if (!dwTargetVID)
		return;

	const LPCHARACTER& pkVictim = CHARACTER_MANAGER::Instance().Find(dwTargetVID);
	TPacketGCFlyTargeting pack{};

	//pack.bHeader = HEADER_GC_FLY_TARGETING;
	pack.bHeader = (bHeader == HEADER_CG_FLY_TARGETING) ? HEADER_GC_FLY_TARGETING : HEADER_GC_ADD_FLY_TARGETING;
	pack.dwShooterVID = GetVID();

	if (pkVictim)
	{
		pack.dwTargetVID = pkVictim->GetVID();
		pack.x = pkVictim->GetX();
		pack.y = pkVictim->GetY();

		if (bHeader == HEADER_CG_FLY_TARGETING)
			m_dwFlyTargetID = dwTargetVID;
		else
			m_vec_dwFlyTargets.emplace_back(dwTargetVID);
	}
	else
	{
		pack.dwTargetVID = 0;
		pack.x = x;
		pack.y = y;
	}

	sys_log(1, "FlyTarget %s vid %d x %d y %d", GetName(), pack.dwTargetVID, pack.x, pack.y);
	PacketAround(&pack, sizeof(pack), this);
}

LPCHARACTER CHARACTER::GetNearestVictim(LPCHARACTER pkChr)
{
	if (nullptr == pkChr)
		pkChr = this;

	float fMinDist = 99999.0f;
	LPCHARACTER pkVictim = nullptr;

	TDamageMap::iterator it = m_map_kDamage.begin();

	// First, filter out people who are not around.
	while (it != m_map_kDamage.end())
	{
		const VID& c_VID = it->first;
		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::Instance().Find(c_VID);

		if (!pAttacker)
			continue;

		if (pAttacker->IsAffectFlag(AFF_EUNHYUNG) ||
			pAttacker->IsAffectFlag(AFF_INVISIBILITY) ||
			pAttacker->IsAffectFlag(AFF_REVIVE_INVISIBLE))
			continue;

		const float fDist = static_cast<float>(DISTANCE_APPROX(pAttacker->GetX() - pkChr->GetX(), pAttacker->GetY() - pkChr->GetY()));

		if (fDist < fMinDist)
		{
			pkVictim = pAttacker;
			fMinDist = fDist;
		}
	}

	return pkVictim;
}

void CHARACTER::SetVictim(LPCHARACTER pkVictim)
{
	if (!pkVictim)
	{
		if (0 != (uint32_t)m_kVIDVictim)
			MonsterLog("°ø°Ý ´ë»óÀ» ÇØÁ¦");

		m_kVIDVictim.Reset();
		battle_end(this);
	}
	else
	{
		if (m_kVIDVictim != pkVictim->GetVID())
			MonsterLog("Set attack target: %s", pkVictim->GetName());

		m_kVIDVictim = pkVictim->GetVID();
		m_dwLastVictimSetTime = get_dword_time();
	}
}

LPCHARACTER CHARACTER::GetVictim() const
{
	return CHARACTER_MANAGER::Instance().Find(m_kVIDVictim);
}

LPCHARACTER CHARACTER::GetProtege() const // Returns the object to be protected
{
	if (m_pkChrStone)
		return m_pkChrStone;

	if (m_pkParty)
		return m_pkParty->GetLeader();

	return nullptr;
}

LPCHARACTER CHARACTER::GetHighestDpsVictim(LPCHARACTER pkChr)	//@fixme512 (Optional)
{
	if (nullptr == pkChr)
		pkChr = this;

	float fMinDist = 99999.0f;
	float fMaxDamage = 0.0f;
	LPCHARACTER pkVictim = nullptr;

	TDamageMap::iterator it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end())
	{
		const VID& c_VID = it->first;
		float fDamage = it->second.iTotalDamage;
		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::Instance().Find(c_VID);

		if (!pAttacker)
			continue;

		if (pAttacker->IsAffectFlag(AFF_EUNHYUNG) ||
			pAttacker->IsAffectFlag(AFF_INVISIBILITY) ||
			pAttacker->IsAffectFlag(AFF_REVIVE_INVISIBLE))
			continue;

		float fDist = DISTANCE_APPROX(pAttacker->GetX() - pkChr->GetX(), pAttacker->GetY() - pkChr->GetY());

		if (fDist < fMinDist && !pAttacker->IsDead() && fDamage > fMaxDamage)
		{
			pkVictim = pAttacker;
			fMaxDamage = fDamage;
		}
	}

	return pkVictim;
}

int CHARACTER::GetAlignment() const
{
	return m_iAlignment;
}

int CHARACTER::GetRealAlignment() const
{
	return m_iRealAlignment;
}

void CHARACTER::ShowAlignment(bool bShow)
{
	if (bShow)
	{
		if (m_iAlignment != m_iRealAlignment)
		{
			m_iAlignment = m_iRealAlignment;
			UpdatePacket();
		}
	}
	else
	{
		if (m_iAlignment != 0)
		{
			m_iAlignment = 0;
			UpdatePacket();
		}
	}
}

void CHARACTER::UpdateAlignment(int iAmount)
{
	bool bShow = false;

#ifdef ENABLE_QUEEN_NETHIS
	if (SnakeLair::CSnk::instance().IsSnakeMap(GetMapIndex()) && iAmount < 0)
		return;
#endif

	if (m_iAlignment == m_iRealAlignment)
		bShow = true;

	const int i = m_iAlignment / 10;

	//m_iRealAlignment = MINMAX(-200000, m_iRealAlignment + iAmount, 200000);
	m_iRealAlignment = MINMAX((MAX_RANK_POINTS*-10), m_iRealAlignment + iAmount, (MAX_RANK_POINTS * 10));	//fixme426

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	if (iAmount > 0)
		CAchievementSystem::Instance().Collect(this, achievements::ETaskTypes::TYPE_COLLECT_ALIGNMENT, iAmount, 0);
#endif

	if (bShow)
	{
		m_iAlignment = m_iRealAlignment;

		if (i != m_iAlignment / 10)
			UpdatePacket();
	}

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL)
	SyncPrivateShopTailInfo();
#endif
}

void CHARACTER::SetKillerMode(bool isOn)
{
	if ((isOn ? ADD_CHARACTER_STATE_KILLER : 0) == IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER))
		return;

	if (isOn)
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
	else
		REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);

	m_iKillerModePulse = thecore_pulse();
	UpdatePacket();
	sys_log(0, "SetKillerMode Update %s[%d]", GetName(), GetPlayerID());
}

bool CHARACTER::IsKillerMode() const
{
	return IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
}

void CHARACTER::UpdateKillerMode()
{
	if (!IsKillerMode())
		return;

	if (thecore_pulse() - m_iKillerModePulse >= PASSES_PER_SEC(30))
		SetKillerMode(false);
}

void CHARACTER::SetPKMode(uint8_t bPKMode)
{
	if (bPKMode >= PK_MODE_MAX_NUM)
		return;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	if ((CMeleyLairManager::Instance().IsMeleyMap(GetMapIndex())) && (bPKMode != PK_MODE_GUILD))
		bPKMode = PK_MODE_GUILD;
#endif

	if (m_bPKMode == bPKMode)
		return;

	if (bPKMode == PK_MODE_GUILD && !GetGuild())
		bPKMode = PK_MODE_FREE;

	m_bPKMode = bPKMode;
	UpdatePacket();

	sys_log(0, "PK_MODE: %s %d", GetName(), m_bPKMode);
}

uint8_t CHARACTER::GetPKMode() const
{
	return m_bPKMode;
}

struct FuncForgetMyAttacker
{
	LPCHARACTER m_ch;
	FuncForgetMyAttacker(LPCHARACTER ch)
	{
		m_ch = ch;
	}
	void operator()(LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (ch && !ch->IsPC() && ch->m_kVIDVictim == m_ch->GetVID())
				ch->SetVictim(nullptr);
		}
	}
};

struct FuncAggregateMonster
{
	LPCHARACTER m_ch;
	FuncAggregateMonster(LPCHARACTER ch)
	{
		m_ch = ch;
	}
	void operator()(LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (!ch || ch->IsPC() || !ch->IsMonster() || ch->GetVictim())
				return;

			if (number(1, 100) <= 50) // Temporarily attracts enemies with a 50% chance
			{
				if (DISTANCE_APPROX(ch->GetX() - m_ch->GetX(), ch->GetY() - m_ch->GetY()) < 5000)
				{
					if (ch->CanBeginFight())
						ch->BeginFight(m_ch);
				}
			}
		}
	}
};

struct FuncAttractRanger
{
	LPCHARACTER m_ch;
	FuncAttractRanger(LPCHARACTER ch)
	{
		m_ch = ch;
	}

	void operator()(LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (!ch || ch->IsPC() || !ch->IsMonster())
				return;
			if (ch->GetVictim() && ch->GetVictim() != m_ch)
				return;
			if (ch->GetMobAttackRange() > 150)
			{
				int iNewRange = 150;//(int)(ch->GetMobAttackRange() * 0.2);
				if (iNewRange < 150)
					iNewRange = 150;

				ch->AddAffect(AFFECT_BOW_DISTANCE, POINT_BOW_DISTANCE, iNewRange - ch->GetMobAttackRange(), AFF_NONE, 3 * 60, 0, false);
			}
		}
	}
};

#ifdef ENABLE_SUNG_MAHI_TOWER
struct FuncAggregateByMaster
{
	LPCHARACTER m_ch;
	FuncAggregateByMaster(LPCHARACTER ch)
	{
		m_ch = ch;
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (ch->IsPC())
				return;
			if (!ch->IsMonster())
				return;

			ch->SetVictim(NULL);
			ch->RemoveNomove();
			ch->RemoveNoattack();

			if (ch->CanBeginFight())
				ch->BeginFight(m_ch);
		}
	}
};
#endif

struct FuncPullMonster
{
	LPCHARACTER m_ch;
	int m_iLength;
	FuncPullMonster(LPCHARACTER ch, int iLength = 300)
	{
		m_ch = ch;
		m_iLength = iLength;
	}

	void operator()(LPENTITY ent)
	{
		if (ent && ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = dynamic_cast<LPCHARACTER>(ent);
			if (!ch || ch->IsPC() || !ch->IsMonster())
				return;
			//if (ch->GetVictim() && ch->GetVictim() != m_ch)
			//	return;
			const float fDist = static_cast<float>(DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY()));
			if (fDist > 3000 || fDist < 100)
				return;

			float fNewDist = fDist - m_iLength;
			if (fNewDist < 100)
				fNewDist = 100;

			const float degree = GetDegreeFromPositionXY(ch->GetX(), ch->GetY(), m_ch->GetX(), m_ch->GetY());
			float fx;
			float fy;

			GetDeltaByDegree(degree, fDist - fNewDist, &fx, &fy);
			const long tx = static_cast<long>(ch->GetX() + fx);
			const long ty = static_cast<long>(ch->GetY() + fy);

			ch->Sync(tx, ty);
			ch->Goto(tx, ty);
			ch->CalculateMoveDuration();

			ch->SyncPacket();
		}
	}
};

void CHARACTER::ForgetMyAttacker()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncForgetMyAttacker f(this);
		pSec->ForEachAround(f);
	}
	ReviveInvisible(5);
}

void CHARACTER::AggregateMonster()
{
#ifdef ENABLE_PULSE_MANAGER
	if (!PulseManager::Instance().IncreaseClock(GetPlayerID(), ePulse::BraveCape, std::chrono::milliseconds(1000))) {
		//ChatPacket(CHAT_TYPE_INFO, "Remaining time: %.2f", PULSEMANAGER_CLOCK_TO_SEC2(GetPlayerID(), ePulse::BraveCape));
		return;
	}
#endif

	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncAggregateMonster f(this);
		pSec->ForEachAround(f);
		EffectPacket(SE_CAPE_OF_COURAGE_EFFECT);	//@fixme510
	}
}

void CHARACTER::AttractRanger()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncAttractRanger f(this);
		pSec->ForEachAround(f);
	}
}

#ifdef ENABLE_SUNG_MAHI_TOWER
void CHARACTER::AggregateMonsterByMaster()
{
	LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());
	if (pMap)
	{
		FuncAggregateByMaster f(this);
		pMap->for_each(f);
	}
}
#endif

void CHARACTER::PullMonster()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncPullMonster f(this);
		pSec->ForEachAround(f);
	}
}

void CHARACTER::UpdateAggrPointEx(LPCHARACTER pAttacker, EDamageType type, int dam, CHARACTER::TBattleInfo& info)
{
	if (!pAttacker)
		return;

	// It goes up higher depending on the specific attack type.
	switch (type)
	{
		case EDamageType::DAMAGE_TYPE_NORMAL_RANGE:
			dam = static_cast<int>(dam * 1.2f);
			break;

		case EDamageType::DAMAGE_TYPE_RANGE:
			dam = static_cast<int>(dam * 1.5f);
			break;

		case EDamageType::DAMAGE_TYPE_MAGIC:
			dam = static_cast<int>(dam * 1.2f);
			break;

		default:
			break;
	}

	// Gives a bonus if the attacker is the current target.
	if (pAttacker == GetVictim())
		dam = static_cast<int>(dam * 1.2f);

	info.iAggro += dam;

	if (info.iAggro < 0)
		info.iAggro = 0;

	//sys_log(0, "UpdateAggrPointEx for %s by %s dam %d total %d", GetName(), pAttacker->GetName(), dam, total);
	if (GetParty() && dam > 0 && type != EDamageType::DAMAGE_TYPE_SPECIAL)
	{
		LPPARTY pParty = GetParty();
		if (!pParty)
			return;

		// If you are a leader, you have more influence.
		int iPartyAggroDist = dam;

		if (pParty->GetLeaderPID() == GetVID())
			iPartyAggroDist /= 2;
		else
			iPartyAggroDist /= 3;

		pParty->SendMessage(this, PM_AGGRO_INCREASE, iPartyAggroDist, pAttacker->GetVID());
	}

	//fixme435
	//Poison won't draw monsters aggro anymore - [Vanilla]
	/*if (type != EDamageType::DAMAGE_TYPE_POISON && poison_draw_monsters_aggro)
		ChangeVictimByAggro(info.iAggro, pAttacker);*/

	switch (type)
	{
		case EDamageType::DAMAGE_TYPE_POISON:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case EDamageType::DAMAGE_TYPE_BLEEDING:
#endif
		case EDamageType::DAMAGE_TYPE_FIRE:
			return;

		default:
			break;
	}

	ChangeVictimByAggro(info.iAggro, pAttacker);
}

void CHARACTER::UpdateAggrPoint(LPCHARACTER pAttacker, EDamageType type, int dam)
{
	if (!pAttacker)
		return;

	if (IsDead() || IsStun())
		return;

	TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());

	if (it == m_map_kDamage.end())
	{
		m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(0, dam)));
		it = m_map_kDamage.find(pAttacker->GetVID());
	}

	UpdateAggrPointEx(pAttacker, type, dam, it->second);
}

void CHARACTER::ChangeVictimByAggro(int iNewAggro, LPCHARACTER pNewVictim)
{
	if (get_dword_time() - m_dwLastVictimSetTime < 3000) // you have to wait 3 seconds
		return;

	if (pNewVictim == GetVictim())
	{
		if (m_iMaxAggro < iNewAggro)
		{
			m_iMaxAggro = iNewAggro;
			return;
		}

		// When Aggro is reduced
		TDamageMap::iterator it;
		TDamageMap::iterator itFind = m_map_kDamage.end();

		for (it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
		{
			if (it->second.iAggro > iNewAggro)
			{
				const LPCHARACTER& ch = CHARACTER_MANAGER::Instance().Find(it->first);
				if (!ch)
					continue;

				if (ch && !ch->IsDead() && DISTANCE_APPROX(ch->GetX() - GetX(), ch->GetY() - GetY()) < 5000)
				{
					itFind = it;
					iNewAggro = it->second.iAggro;
				}
			}
		}

		if (itFind != m_map_kDamage.end())
		{
			m_iMaxAggro = iNewAggro;
#ifdef ENABLE_DEFENSE_WAVE
			if (!IsDefenseWaveMastAttackMob())
#endif
			{
				SetVictim(CHARACTER_MANAGER::Instance().Find(itFind->first));
			}
			m_dwStateDuration = 1;
		}
	}
	else
	{
		if (m_iMaxAggro < iNewAggro)
		{
			m_iMaxAggro = iNewAggro;
#ifdef ENABLE_DEFENSE_WAVE
			if (!IsDefenseWaveMastAttackMob())
#endif
			{
				SetVictim(pNewVictim);
			}
			m_dwStateDuration = 1;
		}
	}
}

#ifdef ENABLE_MONSTER_BACK
void CHARACTER::RewardAttendance()
{
	for (TDamageMap::iterator it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
	{
		const int iDamage = it->second.iTotalDamage;
		if (iDamage > 0)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::Instance().Find(it->first);

			if (ch)
			{
				uint32_t dwCount = 0;

#ifndef ENABLE_10TH_EVENT
				std::string flagNames[5] =
				{
					"attendance_event.red_dragon_hit_points",
					"attendance_event.jotun_thrym_hit_points",
					"attendance_event.razador_hit_points",
					"attendance_event.nemere_hit_points",
					"attendance_event.beran_hit_points",
				};
#endif

				if (!ch->m_hitCount.empty())
				{
					for (uint32_t i = 0; i < ch->m_hitCount.size(); i++)
					{
						if (ch->m_hitCount[i].dwVid == GetVID())
						{
							dwCount = ch->m_hitCount[i].dwCount;
							break;
						}
					}
				}

				if (dwCount)
				{
					if (ch->GetLevel() < 30)
						continue;

					if (!ch->FindAffect(AFFECT_EXP_BONUS_EVENT))
						ch->AddAffect(AFFECT_EXP_BONUS_EVENT, POINT_EXP, 20, 0, 1800, 0, false);

					if (ch->FindAffect(AFFECT_ATT_SPEED_SLOW))
						ch->RemoveAffect(AFFECT_ATT_SPEED_SLOW);

#ifdef ENABLE_10TH_EVENT
					ch->SetQuestFlag("attendance_event.shadow_warrior_hit_points", ch->GetQuestFlag("attendance_event.shadow_warrior_hit_points") + dwCount);
#else
					ch->SetQuestFlag(flagNames[ATTENDANCE_VNUM_RED_DRAGON - GetRaceNum()], ch->GetQuestFlag(flagNames[ATTENDANCE_VNUM_RED_DRAGON - GetRaceNum()]) + dwCount);
#endif

					time_t iTime;
					time(&iTime);
					const tm* pTimeInfo = localtime(&iTime);
					char szFlagname[32 + 1];
					snprintf(szFlagname, sizeof(szFlagname), "attendance.clear_day_%d", pTimeInfo->tm_yday);

					if (ch->GetQuestFlag(szFlagname) == 0)
					{
						ch->SetQuestFlag(szFlagname, 1);
						ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1253]");
						TPacketGCAttendanceEvent packEvent{};
						packEvent.bHeader = HEADER_GC_ATTENDANCE_EVENT;
						packEvent.bType = 2;
						packEvent.bValue = 1;
						ch->GetDesc()->Packet(&packEvent, sizeof(TPacketGCAttendanceEvent));
					}
				}
			}
		}
	}
}
#endif
