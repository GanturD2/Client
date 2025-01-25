#include "stdafx.h"

#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "packet.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "battle.h"
#include "guild.h"
#include "utils.h"
#include "locale_service.h"
#include "lua_incl.h"
#include "arena.h"
#include "horsename_manager.h"
#include "item.h"
#include "DragonSoul.h"
#include "polymorph.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif

#define IS_NO_SAVE_AFFECT(type) ((type) == AFFECT_WAR_FLAG || (type) == AFFECT_REVIVE_INVISIBLE || ((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_PREMIUM_END) || (type) == AFFECT_MOUNT_BONUS || (type) == AFFECT_PREMIUM_PRIVATE_SHOP) // @fixme156 added MOUNT_BONUS (if the game core crashes, the bonus would double if present in player.affect)
#define IS_NO_CLEAR_ON_DEATH_AFFECT(type) ((type) == AFFECT_BLOCK_CHAT || ((type) >= 500 && (type) < 600))
#ifdef ENABLE_BATTLE_FIELD
#define NO_CLEAR_SKILL_ON_DEATH_AFFECT(type) ( \
((type) >= 1 && (type) <= 6) \
|| ((type) >= 16 && (type) <= 21) \
|| ((type) >= 31 && (type) <= 36) \
|| ((type) >= 46 && (type) <= 51) \
|| ((type) >= 61 && (type) <= 66) \
|| ((type) >= 76 && (type) <= 81) \
|| ((type) >= 91 && (type) <= 96) \
|| ((type) >= 106 && (type) <= 111) \
|| ((type) >= 170 && (type) <= 175) \
\
)
#endif

void SendAffectRemovePacket(LPDESC d, uint32_t pid, uint32_t type, uint16_t point) //@fixme532
{
	if (!d)
		return;

	TPacketGCAffectRemove ptoc{};
	ptoc.bHeader = HEADER_GC_AFFECT_REMOVE;
	ptoc.dwType = type;
	ptoc.wApplyOn = point;
	d->Packet(&ptoc, sizeof(TPacketGCAffectRemove));

	TPacketGDRemoveAffect ptod{};
	ptod.dwPID = pid;
	ptod.dwType = type;
	ptod.wApplyOn = point;
	db_clientdesc->DBPacket(HEADER_GD_REMOVE_AFFECT, 0, &ptod, sizeof(ptod));
}

void SendAffectAddPacket(LPDESC d, CAffect* pkAff)
{
	if (!d)
		return;

	TPacketGCAffectAdd ptoc{};
	ptoc.bHeader = HEADER_GC_AFFECT_ADD;
	ptoc.elem.dwType = pkAff->dwType;
	ptoc.elem.wApplyOn = pkAff->wApplyOn;
	ptoc.elem.lApplyValue = pkAff->lApplyValue;
	ptoc.elem.dwFlag = pkAff->dwFlag;
	ptoc.elem.lDuration = pkAff->lDuration;
	ptoc.elem.lSPCost = pkAff->lSPCost;
	d->Packet(&ptoc, sizeof(TPacketGCAffectAdd));
}
////////////////////////////////////////////////////////////////////
// Affect
CAffect* CHARACTER::FindAffect(uint32_t dwType, uint16_t wApply) const //@fixme532
{
	for (const auto& it : m_list_pkAffect)
	{
		CAffect* pkAffect = it;
		if (pkAffect && (pkAffect->dwType == dwType && (wApply == APPLY_NONE || wApply == pkAffect->wApplyOn))) // Nyx : Possible fix IsAffectFlag!
			return pkAffect;
	}

	return nullptr;
}

EVENTFUNC(affect_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("affect_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	if (!ch->UpdateAffect())
		return 0;
	else
		return passes_per_sec; // 1 second
}

bool CHARACTER::UpdateAffect()
{
	// It's not something to handle in affect_event, but it's better to handle it in a one-second event.
	// That's all, so here's the potion treatment.
	if (GetPoint(POINT_HP_RECOVERY) > 0)
	{
		if (GetMaxHP() <= GetHP())
		{
			PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
		}
		else
		{
			const int iVal = MIN(GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 7 / 100);

			PointChange(POINT_HP, iVal);
			PointChange(POINT_HP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_SP_RECOVERY) > 0)
	{
		if (GetMaxSP() <= GetSP())
			PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));
		else
		{
			const int iVal = MIN(GetPoint(POINT_SP_RECOVERY), GetMaxSP() * 7 / 100);

			PointChange(POINT_SP, iVal);
			PointChange(POINT_SP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_HP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_HP, GetPoint(POINT_HP_RECOVER_CONTINUE));
	}

	if (GetPoint(POINT_SP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_SP, GetPoint(POINT_SP_RECOVER_CONTINUE));
	}

	AutoRecoveryItemProcess(AFFECT_AUTO_HP_RECOVERY);
	AutoRecoveryItemProcess(AFFECT_AUTO_SP_RECOVERY);

	// stamina recovery
	if (GetMaxStamina() > GetStamina())
	{
		const int iSec = (get_dword_time() - GetStopTime()) / 3000;
		if (iSec)
			PointChange(POINT_STAMINA, GetMaxStamina() / 1);
	}

	// ProcessAffect returns true if there is no affect.
	if (ProcessAffect())
	{
		if (GetPoint(POINT_HP_RECOVERY) == 0 && GetPoint(POINT_SP_RECOVERY) == 0 && GetStamina() == GetMaxStamina())
		{
			m_pkAffectEvent = nullptr;
			return false;
		}
	}

	return true;
}

void CHARACTER::StartAffectEvent()
{
	if (m_pkAffectEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkAffectEvent = event_create(affect_event, info, passes_per_sec);
	sys_log(1, "StartAffectEvent %s %p %p", GetName(), this, get_pointer(m_pkAffectEvent));
}

void CHARACTER::ClearAffect(bool bSave)
{
	const TAffectFlag afOld = m_afAffectFlag;
	const uint16_t wMovSpd = static_cast<uint16_t>(GetPoint(POINT_MOV_SPEED));
	const uint16_t wAttSpd = static_cast<uint16_t>(GetPoint(POINT_ATT_SPEED));

	auto it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect* pkAff = *it;
		if (!pkAff) // Possible missing nullness verification
			continue;

		if (bSave)
		{
			if (IS_NO_CLEAR_ON_DEATH_AFFECT(pkAff->dwType) || IS_NO_SAVE_AFFECT(pkAff->dwType))
			{
				++it;
				continue;
			}

#ifdef ENABLE_12ZI
			if (pkAff->dwType == AFFECT_CZ_UNLIMIT_ENTER)
			{
				++it;
				continue;
			}
#endif
#ifdef ENABLE_AUTO_SYSTEM
			if (pkAff->dwType == AFFECT_AUTO)
			{
				++it;
				continue;
			}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (pkAff->dwType == AFFECT_GROWTH_PET)
			{
				++it;
				continue;
			}
#endif

#ifdef ENABLE_BATTLE_FIELD
			if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
			{
				if (NO_CLEAR_SKILL_ON_DEATH_AFFECT(pkAff->dwType) || pkAff->dwType == AFFECT_TARGET_VICTIM)
				{
					it++;
					continue;
				}
			}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
			if (pkAff->dwType == AFFECT_SUNGMA_MAP_INFO && IsSungmaMap())
			{
				it++;
				continue;
			}
#endif

			//@custom001
			const size_t iPotionAffects[] = { AFFECT_POTION_1, AFFECT_POTION_2, AFFECT_POTION_3, AFFECT_POTION_4, AFFECT_POTION_5, AFFECT_POTION_6 };
			for (size_t i = 0; i < _countof(iPotionAffects); i++)
			{
				if (pkAff->dwType == static_cast<uint32_t>(iPotionAffects[i]))
				{
					++it;
					continue;
				}
			}
			//@end_custom001

#ifdef ENABLE_SET_ITEM
			if (pkAff->dwType == AFFECT_SET_ITEM || pkAff->dwType >= AFFECT_SET_ITEM_SET_VALUE_1 && pkAff->dwType <= AFFECT_SET_ITEM_SET_VALUE_3)
			{
				++it;
				continue;
			}
#endif

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
			if (quest::CQuestManager::Instance().GetEventFlag("e_late_summer_event") != 0)
			{
				if (pkAff->dwType == AFFECT_LATE_SUMMER_EVENT_BUFF)
				{
					it++;
					continue;
				}
				if (pkAff->dwType == AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF)
				{
					it++;
					continue;
				}
			}
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
			if (pkAff->dwType == AFFECT_COLLECT)
			{
				++it;
				continue;
			}
#endif

#ifdef ENABLE_NINETH_SKILL
			if (pkAff->dwType == SKILL_CHEONUN)
			{
				++it;
				continue;
			}
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
			if (pkAff->dwType == AFFECT_MULTI_FARM_PREMIUM)
			{
				++it;
				continue;
			}
#endif

			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);
			}
		}

		ComputeAffect(pkAff, false);

		it = m_list_pkAffect.erase(it);
		CAffect::Release(pkAff);
	}

	if (afOld != m_afAffectFlag ||
		wMovSpd != GetPoint(POINT_MOV_SPEED) ||
		wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	CheckMaximumPoints();

	if (m_list_pkAffect.empty())
		event_cancel(&m_pkAffectEvent);
}

int CHARACTER::ProcessAffect()
{
	bool bDiff = false;
	CAffect* pkAff = nullptr;

	//
	// premium processing
	//
	for (int i = 0; i < PREMIUM_MAX_NUM; ++i)
	{
		int aff_idx = i + AFFECT_PREMIUM_START;

		pkAff = FindAffect(aff_idx);

		if (!pkAff)
			continue;

		const int remain = GetPremiumRemainSeconds(i);

		if (remain < 0)
		{
			RemoveAffect(aff_idx);
			bDiff = true;
		}
		else
			pkAff->lDuration = remain + 1;
	}

	////////// HAIR_AFFECT
	pkAff = FindAffect(AFFECT_HAIR);
	if (pkAff)
	{
		// IF HAIR_LIMIT_TIME() < CURRENT_TIME()
		if (this->GetQuestFlag("hair.limit_time") < get_global_time())
		{
			// SET HAIR NORMAL
			this->SetPart(PART_HAIR, 0);
			// REMOVE HAIR AFFECT
			RemoveAffect(AFFECT_HAIR);
		}
		else
		{
			// INCREASE AFFECT DURATION
			++(pkAff->lDuration);
		}
	}
	////////// HAIR_AFFECT
	//

	CHorseNameManager::Instance().Validate(this);

	const TAffectFlag afOld = m_afAffectFlag;
	const long lMovSpd = GetPoint(POINT_MOV_SPEED);
	const long lAttSpd = GetPoint(POINT_ATT_SPEED);

	auto it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		pkAff = *it;

		bool bEnd = false;

		if (pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
		{
			if (!GetGuild() || !GetGuild()->UnderAnyWar())
				bEnd = true;
		}

		if (pkAff->lSPCost > 0)
		{
			if (GetSP() < pkAff->lSPCost)
				bEnd = true;
			else
				PointChange(POINT_SP, -pkAff->lSPCost);
		}

		// AFFECT_DURATION_BUG_FIX
		// Infinite effect items also reduce time.
		// I don't think it matters because it takes a very large amount of time.
		if (--pkAff->lDuration <= 0)
		{
			bEnd = true;
		}
		// END_AFFECT_DURATION_BUG_FIX

		if (bEnd)
		{
			it = m_list_pkAffect.erase(it);
			ComputeAffect(pkAff, false);
			bDiff = true;
			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);
			}

			CAffect::Release(pkAff);

			continue;
		}

		++it;
	}

	if (bDiff)
	{
		if (afOld != m_afAffectFlag ||
			lMovSpd != GetPoint(POINT_MOV_SPEED) ||
			lAttSpd != GetPoint(POINT_ATT_SPEED))
		{
			UpdatePacket();
		}

		CheckMaximumPoints();
	}

	if (m_list_pkAffect.empty())
		return true;

	return false;
}

void CHARACTER::SaveAffect()
{
	TPacketGDAddAffect p{};

	auto it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		const CAffect* pkAff = *it++;
		if (!pkAff) // Possible missing nullness verification
			continue;

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			continue;

#ifdef ENABLE_GROWTH_PET_SYSTEM
		if (pkAff->dwType == AFFECT_GROWTH_PET)
			continue;
#endif

#ifdef ENABLE_BATTLE_FIELD
		if (pkAff->dwType == AFFECT_TARGET_VICTIM)
			continue;
#endif

#ifdef ENABLE_NINETH_SKILL
		if (pkAff->dwType == SKILL_CHEONUN)
			continue;
#endif

		sys_log(1, "AFFECT_SAVE: %u %u %d %d", pkAff->dwType, pkAff->wApplyOn, pkAff->lApplyValue, pkAff->lDuration);

		p.dwPID = GetPlayerID();
		p.elem.dwType = pkAff->dwType;
		p.elem.wApplyOn = pkAff->wApplyOn;
		p.elem.lApplyValue = pkAff->lApplyValue;
		p.elem.dwFlag = pkAff->dwFlag;
		p.elem.lDuration = pkAff->lDuration;
		p.elem.lSPCost = pkAff->lSPCost;
#ifdef ENABLE_AFFECT_RENEWAL
		p.elem.dwExpireTime = pkAff->dwExpireTime;
#endif
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}
}

EVENTINFO(load_affect_login_event_info)
{
	uint32_t pid;
	uint32_t count;
	char* data;

	load_affect_login_event_info()
		: pid(0)
		, count(0)
		, data(0)
	{
	}
};

EVENTFUNC(load_affect_login_event)
{
	load_affect_login_event_info* info = dynamic_cast<load_affect_login_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("load_affect_login_event_info> <Factor> Null pointer");
		return 0;
	}

	const uint32_t dwPID = info->pid;
	LPCHARACTER ch = CHARACTER_MANAGER::Instance().FindByPID(dwPID);

	if (!ch)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	const LPDESC& d = ch->GetDesc();

	if (!d)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	if (d->IsPhase(PHASE_HANDSHAKE) ||
		d->IsPhase(PHASE_LOGIN) ||
		d->IsPhase(PHASE_SELECT) ||
		d->IsPhase(PHASE_DEAD) ||
		d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		sys_log(1, "Affect Load by Event");
		ch->LoadAffect(info->count, (TPacketGDAffectElement*)info->data);
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else
	{
		sys_err("input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
}

void CHARACTER::LoadAffect(uint32_t dwCount, TPacketGDAffectElement* pElements)
{
	m_bIsLoadedAffect = false;

	if (!GetDesc()->IsPhase(PHASE_GAME))
	{
		if (test_server)
			sys_log(0, "LOAD_AFFECT: Creating Event", GetName(), dwCount);

		load_affect_login_event_info* info = AllocEventInfo<load_affect_login_event_info>();

		info->pid = GetPlayerID();
		info->count = dwCount;
		info->data = M2_NEW char[sizeof(TPacketGDAffectElement) * dwCount];
		thecore_memcpy(info->data, pElements, sizeof(TPacketGDAffectElement) * dwCount);

		event_create(load_affect_login_event, info, PASSES_PER_SEC(1));

		return;
	}

	ClearAffect(true);

	if (test_server)
		sys_log(0, "LOAD_AFFECT: %s count %d", GetName(), dwCount);

	const TAffectFlag afOld = m_afAffectFlag;

	const long lMovSpd = GetPoint(POINT_MOV_SPEED);
	const long lAttSpd = GetPoint(POINT_ATT_SPEED);

	for (uint32_t i = 0; i < dwCount; ++i, ++pElements)
	{
#ifdef ENABLE_YOHARA_SYSTEM
		if (pElements->dwType == AFFECT_SUNGMA_MAP_INFO)
			continue;
#endif

		// Youngjin Moo does not load.
		if (pElements->dwType == SKILL_MUYEONG)
			continue;

#ifdef ENABLE_NINETH_SKILL
		if (pElements->dwType == SKILL_CHEONUN)
			continue;
#endif

		if (AFFECT_AUTO_HP_RECOVERY == pElements->dwType || AFFECT_AUTO_SP_RECOVERY == pElements->dwType)
		{
			LPITEM item = FindItemByID(pElements->dwFlag);

			if (nullptr == item)
				continue;

			item->Lock(true);
		}

#ifdef ENABLE_ANTI_EXP_RING
		if (AFFECT_EXPRING == pElements->dwType)
		{
			LPITEM item = FindItemByID(pElements->dwFlag);

			if (!item)
				continue;

			item->Lock(true);
			ChatPacket(7, "exp ring affect found: %d", pElements->dwType);
		}
#endif

		//@custom002
		if (AFFECT_POLYMORPH == pElements->dwType && CPolymorphUtils::Instance().IsOnPolymorphMapBlacklist(GetMapIndex()))
			continue;

		if (pElements->wApplyOn >= POINT_MAX_NUM)
		{
			sys_err("invalid affect data %s ApplyOn %u ApplyValue %d",
				GetName(), pElements->wApplyOn, pElements->lApplyValue);
			continue;
		}

		if (test_server)
		{
			sys_log(0, "Load Affect : Affect %s %d %d", GetName(), pElements->dwType, pElements->wApplyOn);
		}

		CAffect* pkAff = CAffect::Acquire();
		m_list_pkAffect.emplace_back(pkAff);

		pkAff->dwType = pElements->dwType;
		pkAff->wApplyOn = pElements->wApplyOn;
		pkAff->lApplyValue = pElements->lApplyValue;
		pkAff->dwFlag = pElements->dwFlag;
		pkAff->lDuration = pElements->lDuration;
		pkAff->lSPCost = pElements->lSPCost;
#ifdef ENABLE_AFFECT_RENEWAL
		pkAff->dwExpireTime = pElements->dwExpireTime;

		if (pElements->dwExpireTime)
			pkAff->lDuration = (pElements->dwExpireTime - get_global_time());
#endif

		SendAffectAddPacket(GetDesc(), pkAff);

		ComputeAffect(pkAff, true);
	}

	if (CArenaManager::Instance().IsArenaMap(GetMapIndex()))
	{
		RemoveGoodAffect();
	}

#ifdef ENABLE_BATTLE_FIELD
	if (!CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
	{
		RemoveAffect(AFFECT_BATTLE_POTION);
	}
#endif

	if (afOld != m_afAffectFlag || lMovSpd != GetPoint(POINT_MOV_SPEED) || lAttSpd != GetPoint(POINT_ATT_SPEED))
	{
		UpdatePacket();
	}

	StartAffectEvent();

#ifdef ENABLE_AFK_MODE_SYSTEM
	if (IsAffectFlag(AFF_AFK))
		RemoveAffect(AFFECT_AFK);
#endif

	m_bIsLoadedAffect = true;

	ComputePoints(); // @fixme156

	// Loading and resetting Dragon Soul Stone settings
	DragonSoul_Initialize();

	// @fixme118 (regain affect hp/mp)
	if (!IsDead())
	{
		PointChange(POINT_HP, GetMaxHP() - GetHP());
		PointChange(POINT_SP, GetMaxSP() - GetSP());
	}
}

bool CHARACTER::AddAffect(uint32_t dwType, uint16_t wApplyOn, long lApplyValue, uint32_t dwFlag, long lDuration, long lSPCost, bool bOverride, bool IsCube
#ifdef ENABLE_NINETH_SKILL
	, uint8_t bShieldDuration
#endif
#ifdef ENABLE_AFFECT_RENEWAL
	, uint32_t dwExpireTime
#endif
)	//@fixme532
{
	// CHAT_BLOCK
	if (dwType == AFFECT_BLOCK_CHAT && lDuration > 1)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;652]");
	}
	// END_OF_CHAT_BLOCK

	if (IsDead())	//@fixme000
		return false;

	if (lDuration == 0)
	{
		sys_err("Character::AddAffect lDuration == 0 type %d", dwType);
		lDuration = 1;
	}

	switch (dwType) {	//@fixme406
		case SKILL_HOSIN:		// Segen
		case SKILL_REFLECT:		// Reflektieren
		case SKILL_GICHEON:		// Hilfe des Drachen
		case SKILL_JEONGEOP:	// Kurieren
		case SKILL_KWAESOK:		// Schnelligkeit
		case SKILL_JEUNGRYEOK:	// Angriff+
#ifdef ENABLE_WOLFMAN_CHARACTER
		case SKILL_CHEONGRANG:	// Indigowolfseele
#endif
#ifdef ENABLE_NINETH_SKILL
		case SKILL_CHEONUN:		// Ätherschild
#endif
		{
			const CAffect * pkAffect = FindAffect(dwType);
			if (!pkAffect)
				break;

			if (lApplyValue < pkAffect->lApplyValue) {
				ChatPacket(CHAT_TYPE_INFO, "<AddAffect> has blocked receiving skill (%s) because power is (%ld%%) more small then current one (%ld%%).", CSkillManager::Instance().Get(dwType)->szName, lApplyValue, pkAffect->lApplyValue);
				return false;
			}
		}
		break;

		default:
			break;
	}	//@END_fixme406

	CAffect* pkAff = nullptr;

	if (IsCube)
		pkAff = FindAffect(dwType, wApplyOn);
	else
		pkAff = FindAffect(dwType);

	if (dwFlag == AFF_STUN)
	{
		if (m_posDest.x != GetX() || m_posDest.y != GetY())
		{
			m_posDest.x = m_posStart.x = GetX();
			m_posDest.y = m_posStart.y = GetY();
			battle_end(this);

			SyncPacket();
		}
	}

	// Overwriting of effects that already exist
	if (pkAff && bOverride)
	{
		ComputeAffect(pkAff, false); // Once the effect is removed

		if (GetDesc())
			SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);
	}
	else
	{
		// Add new effect
		// NOTE: Therefore, multiple effects can be attached to the same type.
		pkAff = CAffect::Acquire();
		m_list_pkAffect.emplace_back(pkAff);

	}

	sys_log(1, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), dwType, wApplyOn, lApplyValue, dwFlag, lDuration);
	//sys_log(0, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), dwType, wApplyOn, lApplyValue, dwFlag, lDuration); //Duplicated

	pkAff->dwType = dwType;
	pkAff->wApplyOn = wApplyOn;
	pkAff->lApplyValue = lApplyValue;
	pkAff->dwFlag = dwFlag;
	pkAff->lDuration = lDuration;
	pkAff->lSPCost = lSPCost;
#ifdef ENABLE_NINETH_SKILL
	pkAff->bShieldDuration = bShieldDuration;
#endif
#ifdef ENABLE_AFFECT_RENEWAL
	pkAff->dwExpireTime = dwExpireTime;
#endif

	const auto wMovSpd = static_cast<uint16_t>(GetPoint(POINT_MOV_SPEED));
	const auto wAttSpd = static_cast<uint16_t>(GetPoint(POINT_ATT_SPEED));

	ComputeAffect(pkAff, true);

	if (pkAff->dwFlag || wMovSpd != GetPoint(POINT_MOV_SPEED) || wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	StartAffectEvent();

	if (IsPC())
	{
		SendAffectAddPacket(GetDesc(), pkAff);

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			return true;

#ifdef ENABLE_GROWTH_PET_SYSTEM
		if (pkAff->dwType == AFFECT_GROWTH_PET)
			return true;
#endif

#ifdef ENABLE_BATTLE_FIELD
		if (pkAff->dwType == AFFECT_TARGET_VICTIM)
			return true;
#endif

#ifdef ENABLE_NINETH_SKILL
		if (pkAff->dwType == SKILL_CHEONUN)
			return true;
#endif

		TPacketGDAddAffect p{};
		p.dwPID = GetPlayerID();
		p.elem.dwType = pkAff->dwType;
		p.elem.wApplyOn = pkAff->wApplyOn;
		p.elem.lApplyValue = pkAff->lApplyValue;
		p.elem.dwFlag = pkAff->dwFlag;
		p.elem.lDuration = pkAff->lDuration;
		p.elem.lSPCost = pkAff->lSPCost;
#ifdef ENABLE_AFFECT_RENEWAL
		p.elem.dwExpireTime = pkAff->dwExpireTime;
#endif
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}

	return true;
}

void CHARACTER::RefreshAffect()
{
	auto it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		const CAffect* pkAff = *it++;
		if (pkAff)
		{
			ComputeAffect(pkAff, true);
		}
	}
}

void CHARACTER::ComputeAffect(const CAffect* pkAff, bool bAdd)
{
	if (bAdd && pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
	{
		if (!GetGuild())
			return;

		if (!GetGuild()->UnderAnyWar())
			return;
	}

	if (pkAff->dwFlag)
	{
		if (!bAdd)
			m_afAffectFlag.Reset(pkAff->dwFlag);
		else
			m_afAffectFlag.Set(pkAff->dwFlag);
	}

	PointChange(pkAff->wApplyOn, (bAdd ? pkAff->lApplyValue : -pkAff->lApplyValue));

	if (pkAff->dwType == SKILL_MUYEONG)
	{
		if (bAdd)
			StartMuyeongEvent();
		else
			StopMuyeongEvent();
	}

#ifdef ENABLE_PVP_BALANCE
	if (pkAff->dwType == SKILL_GYEONGGONG)
	{
		if (bAdd)
			StartGyeongGongEvent();
		else
			StopGyeongGongEvent();
	}
#endif

#ifdef ENABLE_NINETH_SKILL
	if (pkAff->dwType == SKILL_CHEONUN)
	{
		if (bAdd)
			StartCheonunEvent(static_cast<uint8_t>(pkAff->lApplyValue), pkAff->bShieldDuration);
		else
			StopCheonunEvent();
	}
#endif
}

bool CHARACTER::RemoveAffect(CAffect* pkAff, bool single)	//@fixme433
{
	if (!pkAff)
		return false;

	// AFFECT_BUF_FIX
	m_list_pkAffect.remove(pkAff);
	// END_OF_AFFECT_BUF_FIX

	ComputeAffect(pkAff, false);

	/*
	* Fix white flag bug.
	* The white flag bug occurs when attacking immediately after casting buff skill -> transforming -> using white flag (AFFECT_REVIVE_INVISIBLE).
	* The reason is that at the time of casting the disguise, only the disguise effect is applied, ignoring the buff skill effect,
	* If you attack immediately after using the white flag, RemoveAffect is called, and it becomes a disguise effect + buff skill effect while doing ComputePoints.
	* If you are disguised in ComputePoints, you can make the buff skill effect not work,
	* ComputePoints are widely used, so I'm reluctant to make big changes (it's hard to know what side effects will happen).
	* Therefore, it is modified only when AFFECT_REVIVE_INVISIBLE is deleted with RemoveAffect.
	* If the time runs out and the white flag is released, no bug occurs, so do the same.
	* (If you look at ProcessAffect, if the time runs out and the Affect is deleted, ComputePoints is not called.)
	*/
	if (single) {	//@fixme433
		if (AFFECT_REVIVE_INVISIBLE != pkAff->dwType)
			ComputePoints();
		else  // @fixme110
			UpdatePacket();
	}

	CheckMaximumPoints();

	if (test_server)
		sys_log(0, "AFFECT_REMOVE: %s (flag %u apply: %u)", GetName(), pkAff->dwFlag, pkAff->wApplyOn);

	if (IsPC())
	{
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);
	}

	CAffect::Release(pkAff);
	return true;
}

bool CHARACTER::RemoveAffect(uint32_t dwType)
{
	// CHAT_BLOCK
	if (dwType == AFFECT_BLOCK_CHAT)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;988]");
	}
	// END_OF_CHAT_BLOCK

	bool flag = false;

	CAffect* pkAff;

	while ((pkAff = FindAffect(dwType)))
	{
		RemoveAffect(pkAff);
		flag = true;
	}

	return flag;
}

bool CHARACTER::IsAffectFlag(uint32_t dwAff) const
{
	return m_afAffectFlag.IsSet(dwAff);
}

const std::vector<uint16_t> GoodAffects =
{
	AFFECT_MOV_SPEED,
	AFFECT_ATT_SPEED,

	AFFECT_STR,
	AFFECT_DEX,
	AFFECT_INT,
	AFFECT_CON,

	AFFECT_CHINA_FIREWORK,

	// Body Warrior
	SKILL_JEONGWI,		// Kampfrausch
	SKILL_GEOMKYUNG,	// Aura des Schwertes

	// Mental Warrior
	SKILL_CHUNKEON,		// Starker Körper

	// Blade Ninja
	SKILL_EUNHYUNG,		// Tarnung

	// Archery Ninja
	SKILL_GYEONGGONG,	// Federschreiten

	// Weaponry Sura
	SKILL_GWIGEOM,		// Verzauberte Klinge
	SKILL_TERROR,		// Furcht
	SKILL_JUMAGAP,		// Verzauberte Rüstung

	// Black Magic Sura
	SKILL_MANASHILED,	// Dunkler Schutz

	// Dragon Force Shaman
	SKILL_HOSIN,		// Segen
	SKILL_REFLECT,		// Reflektieren
	SKILL_GICHEON,		// Hilfe des Drachen

	// Healing Force Shaman
	SKILL_KWAESOK,		// Schnelligkeit
	SKILL_JEUNGRYEOK,	// Angriff+
#ifdef ENABLE_NINETH_SKILL
	SKILL_CHEONUN,
#endif

#ifdef ENABLE_WOLFMAN_CHARACTER
	SKILL_JEOKRANG,		// Purpurwolfseele
	SKILL_CHEONGRANG,	// Indigowolfseele
#endif
};

void CHARACTER::RemoveGoodAffect()
{
	for (auto it : GoodAffects)
	{
		const CAffect* pkAff = FindAffect(it);
		if (pkAff)
			RemoveAffect(const_cast<CAffect *>(pkAff), false);
	}

	ComputePoints();
}

bool CHARACTER::IsGoodAffect(uint8_t bAffectType) const
{
	switch (bAffectType)
	{
		case (AFFECT_MOV_SPEED):
		case (AFFECT_ATT_SPEED):
		case (AFFECT_STR):
		case (AFFECT_DEX):
		case (AFFECT_INT):
		case (AFFECT_CON):
		case (AFFECT_CHINA_FIREWORK):
		case (SKILL_JEONGWI):
		case (SKILL_GEOMKYUNG):
		case (SKILL_CHUNKEON):
		case (SKILL_EUNHYUNG):
		case (SKILL_GYEONGGONG):
		case (SKILL_GWIGEOM):
		case (SKILL_TERROR):
		case (SKILL_JUMAGAP):
		case (SKILL_MANASHILED):
		case (SKILL_HOSIN):
		case (SKILL_REFLECT):
		case (SKILL_KWAESOK):
		case (SKILL_JEUNGRYEOK):
		case (SKILL_GICHEON):
#ifdef ENABLE_WOLFMAN_CHARACTER
		case (SKILL_JEOKRANG):
		case (SKILL_CHEONGRANG):
#endif
#ifdef ENABLE_NINETH_SKILL
		case (SKILL_CHEONUN):
#endif
			return true;

		default:
			break;
	}

	return false;
}

const std::vector<uint16_t> BadAffects =
{
	AFFECT_FIRE,
	AFFECT_POISON,
#ifdef ENABLE_WOLFMAN_CHARACTER
	AFFECT_BLEEDING,
#endif
	AFFECT_STUN,
	AFFECT_SLOW,
	SKILL_TUSOK,
};

void CHARACTER::RemoveBadAffect()
{
	for (auto it : BadAffects)
	{
		const CAffect* pkAff = FindAffect(it);
		if (pkAff)
		{
			RemoveAffect(const_cast<CAffect*>(pkAff), false);

			switch (it)
			{
				case AFFECT_FIRE:
					event_cancel(&m_pkFireEvent);
					break;
				case AFFECT_POISON:
					event_cancel(&m_pkPoisonEvent);
					break;
#ifdef ENABLE_WOLFMAN_CHARACTER
				case AFFECT_BLEEDING:
					event_cancel(&m_pkBleedingEvent);
					break;
#endif

				default:
					break;
			}
		}
	}

	UpdatePacket();
}
