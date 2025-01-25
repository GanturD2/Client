#include "stdafx.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "packet.h"
#include "guild.h"
#include "vector.h"
#include "questmanager.h"
#include "item.h"
#include "horsename_manager.h"
#include "locale_service.h"
#include "arena.h"
#include "war_map.h"
#include "../../common/VnumHelper.h"
#include "mob_manager.h"
#ifdef ENABLE_CHANGE_LOOK_MOUNT
# include "item_manager.h"
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
extern bool IS_MOUNTABLE_ZONE(int map_index);	//@custom005
#endif

bool CHARACTER::StartRiding()
{
#ifdef ENABLE_NEWSTUFF
	if (g_NoMountAtGuildWar && GetWarMap())
	{
		RemoveAffect(AFFECT_MOUNT);
		RemoveAffect(AFFECT_MOUNT_BONUS);
		if (IsRiding())
			StopRiding();
		return false;
	}
#endif

	if (IsDead())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1936]");
		return false;
	}

	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;882]");
		return false;
	}

	if (IsRiding()) {	//@fixme447
		ChatPacket(CHAT_TYPE_INFO, "[LS;1054]");
		return false;
	}

	LPITEM armor = GetWear(WEAR_BODY);

	if (armor && (armor->GetVnum() >= 11901 && armor->GetVnum() <= 11904))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;883]");
		return false;
	}

	// @warme005
	if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
		return false;

	uint32_t dwMountVnum = m_chHorse ? m_chHorse->GetRaceNum() : GetMyHorseVnum();

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP	//custom005
	if (!IS_MOUNTABLE_ZONE(GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1098]");
		return false;
	}
#endif

	if (!CHorseRider::StartRiding())
	{
		if (GetHorseLevel() <= 0)
			ChatPacket(CHAT_TYPE_INFO, "[LS;884]");
		else if (GetHorseHealth() <= 0)
			ChatPacket(CHAT_TYPE_INFO, "[LS;885]");
		else if (GetHorseStamina() <= 0)
			ChatPacket(CHAT_TYPE_INFO, "[LS;886]");

		return false;
	}

#ifdef ENABLE_AFK_MODE_SYSTEM
	if (IsAway()) {
		SetAway(false);
		if (IsAffectFlag(AFF_AFK))
			RemoveAffect(AFFECT_AFK);
	}
#endif

	HorseSummon(false);

	MountVnum(dwMountVnum);

	if(test_server)
		sys_log(0, "Ride Horse : %s ", GetName());

	return true;
}

bool CHARACTER::StopRiding()
{
	if (CHorseRider::StopRiding())
	{
		quest::CQuestManager::Instance().Unmount(GetPlayerID());

		if (!IsDead() && !IsStun())
		{
			uint32_t dwOldVnum = GetMountVnum();
			MountVnum(0);

			// [NOTE] When dismounting from a horse, it is modified to summon the one it was riding on.
			HorseSummon(true, false, dwOldVnum);
		}
		else
		{
			m_dwMountVnum = 0;
			ComputePoints();
			UpdatePacket();
		}

		PointChange(POINT_ST, 0);
		PointChange(POINT_DX, 0);
		PointChange(POINT_HT, 0);
		PointChange(POINT_IQ, 0);
		return true;
	}

	return false;
}

EVENTFUNC(horse_dead_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "horse_dead_event> <Factor> Null pointer" );
		return 0;
	}

	// <Factor>
	LPCHARACTER ch = info->ch;
	if (ch == nullptr) {
		return 0;
	}
	ch->HorseSummon(false);
	return 0;
}

void CHARACTER::SetRider(LPCHARACTER ch)
{
	if (m_chRider)
		m_chRider->ClearHorseInfo();

	m_chRider = ch;

	if (m_chRider)
		m_chRider->SendHorseInfo();
}

LPCHARACTER CHARACTER::GetRider() const
{
	return m_chRider;
}

void CHARACTER::HorseSummon(bool bSummon, bool bFromFar, uint32_t dwVnum, const char* pPetName)
{
	if ( bSummon )
	{
		if( m_chHorse != nullptr )
			return;

		if (GetHorseLevel() <= 0)
			return;

		if (IsRiding())
			return;

		sys_log(0, "HorseSummon : %s lv:%d bSummon:%d fromFar:%d", GetName(), GetLevel(), bSummon, bFromFar);

		long x = GetX();
		long y = GetY();

		if (GetHorseHealth() <= 0)
			bFromFar = false;

		if (bFromFar)
		{
			x += (number(0, 1) * 2 - 1) * number(2000, 2500);
			y += (number(0, 1) * 2 - 1) * number(2000, 2500);
		}
		else
		{
			x += number(-100, 100);
			y += number(-100, 100);
		}

		m_chHorse = CHARACTER_MANAGER::Instance().SpawnMob(
				(0 == dwVnum) ? GetMyHorseVnum() : dwVnum,
				GetMapIndex(),
				x, y,
				GetZ(), false, (int)(GetRotation()+180), false);

		if (!m_chHorse)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;887]");
			return;
		}

		if (GetHorseHealth() <= 0)
		{
			m_chHorse->SetPosition(POS_DEAD);

			char_event_info* info = AllocEventInfo<char_event_info>();
			info->ch = this;
			m_chHorse->m_pkDeadEvent = event_create(horse_dead_event, info, PASSES_PER_SEC(60));
		}

		m_chHorse->SetLevel(GetHorseLevel());

		const char* pHorseName = CHorseNameManager::Instance().GetHorseName(GetPlayerID());

		if ( pHorseName != nullptr && strlen(pHorseName) != 0 )
		{
			m_chHorse->m_stName = pHorseName;
		}
		else
		{
			m_chHorse->m_stName = GetName();
#ifndef ENABLE_MULTI_LANGUAGE_SYSTEM
#ifdef ENABLE_HORSE_APPEARANCE
			if (dwVnum >= 20110 && dwVnum <= 20274) {
				m_chHorse->m_stName += "'s ";
				auto it = CMobManager::Instance().Get(dwVnum);
				m_chHorse->m_stName += it ? it->m_table.szLocaleName : "";
			}
			else
				m_chHorse->m_stName += LC_TEXT("´ÔÀÇ ¸»");
#else
			m_chHorse->m_stName += LC_TEXT("´ÔÀÇ ¸»");
#endif
#endif
		}

		if (!m_chHorse->Show(GetMapIndex(), x, y, GetZ()))
		{
			M2_DESTROY_CHARACTER(m_chHorse);
			sys_err("cannot show monster");
			m_chHorse = nullptr;
			return;
		}

		if ((GetHorseHealth() <= 0))
		{
			TPacketGCDead pack{};
			pack.header	= HEADER_GC_DEAD;
			pack.vid    = m_chHorse->GetVID();
			PacketAround(&pack, sizeof(pack));
		}

		m_chHorse->SetRider(this);
	}
	else
	{
		if (!m_chHorse)
			return;

		LPCHARACTER chHorse = m_chHorse;

		chHorse->SetRider(nullptr); // m_chHorse assign to nullptr

		if ((GetHorseHealth() <= 0))
			bFromFar = false;

		if (!bFromFar)
		{
			M2_DESTROY_CHARACTER(chHorse);
		}
		else
		{
			chHorse->SetNowWalking(false);
			float fx, fy;
			chHorse->SetRotation(GetDegreeFromPositionXY(chHorse->GetX(), chHorse->GetY(), GetX(), GetY())+180);
			GetDeltaByDegree(chHorse->GetRotation(), 3500, &fx, &fy);
			chHorse->Goto((long)(chHorse->GetX()+fx), (long) (chHorse->GetY()+fy));
			chHorse->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
		}

		m_chHorse = nullptr;

#ifdef ENABLE_CHANGE_LOOK_MOUNT
		SetHorseChangeLookVnum(0);
#endif
	}
}

#ifdef ENABLE_HORSE_APPEARANCE
uint32_t CHARACTER::GetMyHorseVnum()
#else
uint32_t CHARACTER::GetMyHorseVnum() const
#endif
{
	int delta = 0;

#ifdef ENABLE_HORSE_APPEARANCE
	uint32_t horse_looks = GetHorseAppearance();
	if (horse_looks > 0) {
		return horse_looks;
	}
#endif

	if (GetGuild())
	{
		++delta;

		if (GetGuild()->GetMasterPID() == GetPlayerID())
			++delta;
	}

#ifdef ENABLE_CHANGE_LOOK_MOUNT
	uint32_t horseVnum = c_aHorseStat[GetHorseLevel()].iNPCRace + delta;

	if (GetHorseChangeLookVnum())
	{
		const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(GetHorseChangeLookVnum());
		if (pProto)
			horseVnum = pProto->aApplies[0].lValue;
	}

	return horseVnum;
#else
	return c_aHorseStat[GetHorseLevel()].iNPCRace + delta;
#endif
}

void CHARACTER::HorseDie()
{
	CHorseRider::HorseDie();
	HorseSummon(false);
}

bool CHARACTER::ReviveHorse()
{
	if (CHorseRider::ReviveHorse())
	{
		HorseSummon(false);
		HorseSummon(true);
		return true;
	}
	return false;
}

void CHARACTER::ClearHorseInfo()
{
	if (!IsHorseRiding())
	{
		ChatPacket(CHAT_TYPE_COMMAND, "hide_horse_state");

		m_bSendHorseLevel = 0;
		m_bSendHorseHealthGrade = 0;
		m_bSendHorseStaminaGrade = 0;
	}

	m_chHorse = nullptr;

}

void CHARACTER::SendHorseInfo()
{
	if (m_chHorse || IsHorseRiding())
	{
		int iHealthGrade;
		int iStaminaGrade;
		/*
		   HP
3: 70% < ~ <= 100%
2: 30% < ~ <= 70%
1:  0% < ~ <= 30%
0: »ç¸Á

STM

3: 71% < ~ <= 100%
2: 31% < ~ <= 70%
1: 10% < ~ <= 30%
0:	 ~ <= 10%
		 */
		if (GetHorseHealth() == 0)
			iHealthGrade = 0;
		else if (GetHorseHealth() * 10 <= GetHorseMaxHealth() * 3)
			iHealthGrade = 1;
		else if (GetHorseHealth() * 10 <= GetHorseMaxHealth() * 7)
			iHealthGrade = 2;
		else
			iHealthGrade = 3;

		if (GetHorseStamina() * 10 <= GetHorseMaxStamina())
			iStaminaGrade = 0;
		else if (GetHorseStamina() * 10 <= GetHorseMaxStamina() * 3)
			iStaminaGrade = 1;
		else if (GetHorseStamina() * 10 <= GetHorseMaxStamina() * 7)
			iStaminaGrade = 2;
		else
			iStaminaGrade = 3;

		if (m_bSendHorseLevel != GetHorseLevel() ||
				m_bSendHorseHealthGrade != iHealthGrade ||
				m_bSendHorseStaminaGrade != iStaminaGrade)
		{
			ChatPacket(CHAT_TYPE_COMMAND, "horse_state %d %d %d", GetHorseLevel(), iHealthGrade, iStaminaGrade);

			m_bSendHorseLevel = GetHorseLevel();
			m_bSendHorseHealthGrade = iHealthGrade;
			m_bSendHorseStaminaGrade = iStaminaGrade;
		}
	}
}

bool CHARACTER::CanUseHorseSkill()
{
#ifdef ENABLE_HORSESKILLS_ON_MOUNTS
	return (IsRiding() && GetHorseGrade() == 3);
#else
	if(IsRiding())
	{
		if (GetHorseGrade() == 3)
			return true;
		else
			return false;

		if (GetMountVnum())
		{
			if (GetMountVnum() >= 20209 && GetMountVnum() <= 20212)
				return true;

			if (CMobVnumHelper::IsRamadanBlackHorse(GetMountVnum()))
				return true;
		}
		else
			return false;

	}

	return false;
#endif
}

void CHARACTER::SetHorseLevel(int iLevel)
{
	CHorseRider::SetHorseLevel(iLevel);
	SetSkillLevel(SKILL_HORSE, GetHorseLevel());
}
