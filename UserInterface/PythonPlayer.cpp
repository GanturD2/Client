#include "StdAfx.h"
#include "PythonPlayerEventHandler.h"
#include "PythonApplication.h"
#include "PythonItem.h"
#include "../EterBase/Timer.h"

#include "AbstractPlayer.h"
#include "../GameLib/GameLibDefines.h"
#ifdef ENABLE_ELEMENT_ADD
#	include "../GameLib/ItemData.h"
#endif

const uint32_t POINT_MAGIC_NUMBER = 0xe73ac1da;

#ifdef ENABLE_REFINE_ELEMENT
void CPythonPlayer::SetElement(TItemPos Cell, uint8_t value_grade_element, uint8_t type_element)
{
	if (!Cell.IsValidCell())
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->grade_element = value_grade_element;
	(const_cast <TItemData*>(GetItemData(Cell)))->element_type_bonus = type_element;
}

void CPythonPlayer::SetElementAttack(TItemPos Cell, uint32_t attack_element_index, uint32_t attack_element)
{
	if (!Cell.IsValidCell())
		return;

	if (attack_element_index >= MAX_ELEMENTS_SPELL)
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->attack_element[attack_element_index] = attack_element;
}

void CPythonPlayer::SetElementValue(TItemPos Cell, uint32_t elements_value_bonus_index, int16_t elements_value_bonus)
{
	if (!Cell.IsValidCell())
		return;

	if (elements_value_bonus_index >= MAX_ELEMENTS_SPELL)
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->elements_value_bonus[elements_value_bonus_index] = elements_value_bonus;
}

uint8_t CPythonPlayer::GetElementGrade(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return GetItemData(Cell)->grade_element;
}

uint32_t CPythonPlayer::GetElementAttack(TItemPos Cell, uint32_t attack_element_index)
{
	if (!Cell.IsValidCell())
		return 0;

	if (attack_element_index >= MAX_ELEMENTS_SPELL)
		return 0;

	return GetItemData(Cell)->attack_element[attack_element_index];
}

uint8_t CPythonPlayer::GetElementType(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return GetItemData(Cell)->element_type_bonus;
}

int16_t CPythonPlayer::GetElementValue(TItemPos Cell, uint32_t elements_value_bonus_index)
{
	if (!Cell.IsValidCell())
		return 0;

	if (elements_value_bonus_index >= MAX_ELEMENTS_SPELL)
		return 0;

	return GetItemData(Cell)->elements_value_bonus[elements_value_bonus_index];
}
#endif

void CPythonPlayer::SPlayerStatus::SetPoint(uint32_t ePoint, long lPoint)
{
	m_alPoint[ePoint] = lPoint ^ POINT_MAGIC_NUMBER;
}

long CPythonPlayer::SPlayerStatus::GetPoint(uint32_t ePoint)
{
	return m_alPoint[ePoint] ^ POINT_MAGIC_NUMBER;
}

bool CPythonPlayer::AffectIndexToSkillIndex(uint32_t dwAffectIndex, uint32_t* pdwSkillIndex)
{
	if (m_kMap_dwAffectIndexToSkillIndex.end() == m_kMap_dwAffectIndexToSkillIndex.find(dwAffectIndex))
		return false;

	*pdwSkillIndex = m_kMap_dwAffectIndexToSkillIndex[dwAffectIndex];
	return true;
}

bool CPythonPlayer::AffectIndexToSkillSlotIndex(uint32_t uAffect, uint32_t* pdwSkillSlotIndex)
{
	uint32_t dwSkillIndex = m_kMap_dwAffectIndexToSkillIndex[uAffect];

	return GetSkillSlotIndex(dwSkillIndex, pdwSkillSlotIndex);
}

bool CPythonPlayer::__GetPickedActorPtr(CInstanceBase** ppkInstPicked) const
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstPicked = rkChrMgr.OLD_GetPickedInstancePtr();
	if (!pkInstPicked)
		return false;

	*ppkInstPicked = pkInstPicked;
	return true;
}

bool CPythonPlayer::__GetPickedActorID(uint32_t* pdwActorID) const
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.OLD_GetPickedInstanceVID(pdwActorID);
}

bool CPythonPlayer::__GetPickedItemID(uint32_t* pdwItemID) const
{
	CPythonItem& rkItemMgr = CPythonItem::Instance();
	return rkItemMgr.GetPickedItemID(pdwItemID);
}

bool CPythonPlayer::__GetPickedGroundPos(TPixelPosition* pkPPosPicked) const
{
	CPythonBackground& rkBG = CPythonBackground::Instance();

	TPixelPosition kPPosPicked;
	if (rkBG.GetPickingPoint(pkPPosPicked))
	{
		pkPPosPicked->y = -pkPPosPicked->y;
		return true;
	}

	return false;
}

void CPythonPlayer::NEW_GetMainActorPosition(TPixelPosition* pkPPosActor) const
{
	TPixelPosition kPPosMainActor;

	IAbstractPlayer& rkPlayer = GetSingleton();
	CInstanceBase* pInstance = rkPlayer.NEW_GetMainActorPtr();
	if (pInstance)
		pInstance->NEW_GetPixelPosition(pkPPosActor);
	else
		CPythonApplication::Instance().GetCenterPosition(pkPPosActor);
}


bool CPythonPlayer::RegisterEffect(uint32_t dwEID, const char* c_szFileName, bool isCache)
{
	if (dwEID >= EFFECT_NUM)
		return false;

	CEffectManager& rkEftMgr = CEffectManager::Instance();
	rkEftMgr.RegisterEffect2(c_szFileName, &m_adwEffect[dwEID], isCache);
	return true;
}

void CPythonPlayer::NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst)
{
	if (dwEID >= EFFECT_NUM)
		return;

	D3DXVECTOR3 kD3DVt3Pos(kPPosDst.x, -kPPosDst.y, kPPosDst.z);
	D3DXVECTOR3 kD3DVt3Dir(0.0f, 0.0f, 1.0f);

	CEffectManager& rkEftMgr = CEffectManager::Instance();
	rkEftMgr.CreateEffect(m_adwEffect[dwEID], kD3DVt3Pos, kD3DVt3Dir);
}

CInstanceBase* CPythonPlayer::NEW_FindActorPtr(uint32_t dwVID) const
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

CInstanceBase* CPythonPlayer::NEW_GetMainActorPtr()
{
	return NEW_FindActorPtr(m_dwMainCharacterIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////

void CPythonPlayer::Update()
{
	NEW_RefreshMouseWalkingDirection();

	CPythonPlayerEventHandler& rkPlayerEventHandler = CPythonPlayerEventHandler::GetSingleton();
	rkPlayerEventHandler.FlushVictimList();

	if (m_isDestPosition)
	{
		CInstanceBase* pInstance = NEW_GetMainActorPtr();
		if (pInstance)
		{
			TPixelPosition PixelPosition;
			pInstance->NEW_GetPixelPosition(&PixelPosition);

			if (abs(int(PixelPosition.x) - m_ixDestPos) + abs(int(PixelPosition.y) - m_iyDestPos) < 10000)
				m_isDestPosition = FALSE;
			else
			{
				if (CTimer::Instance().GetCurrentMillisecond() - m_iLastAlarmTime > 20000)
					AlarmHaveToGo();
			}
		}
	}

	if (m_isConsumingStamina)
	{
		float fElapsedTime = CTimer::Instance().GetElapsedSecond();
		m_fCurrentStamina -= (fElapsedTime * m_fConsumeStaminaPerSec);

		SetStatus(POINT_STAMINA, uint32_t(m_fCurrentStamina));

		PyCallClassMemberFunc(m_ppyGameWindow, "RefreshStamina", Py_BuildValue("()"));
	}

	__Update_AutoAttack();
	__Update_NotifyGuildAreaEvent();
#ifdef ENABLE_AUTO_SYSTEM
	if (AutoStatus())
		UpdateAuto();
#endif
}

bool CPythonPlayer::__IsUsingChargeSkill()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return false;

	if (__CheckDashAffect(*pkInstMain))
		return true;

	if (MODE_USE_SKILL != m_eReservedMode)
		return false;

	if (m_dwSkillSlotIndexReserved >= SKILL_MAX_NUM)
		return false;

	TSkillInstance& rkSkillInst = m_playerStatus.aSkill[m_dwSkillSlotIndexReserved];

	CPythonSkill::TSkillData* pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return false;

	return pSkillData->IsChargeSkill() ? true : false;
}

void CPythonPlayer::__Update_AutoAttack()
{
	if (0 == m_dwAutoAttackTargetVID)
		return;

	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;

	// 탄환격 쓰고 달려가는 도중에는 스킵
	if (__IsUsingChargeSkill())
		return;

	CInstanceBase* pkInstVictim = NEW_FindActorPtr(m_dwAutoAttackTargetVID);
	if (!pkInstVictim)
		__ClearAutoAttackTargetActorID();
	else
	{
		if (pkInstVictim->IsDead())
			__ClearAutoAttackTargetActorID();
#ifdef ENABLE_AUTO_SYSTEM
		else if (AutoStatus() && !GetAutoAttackOnOff())
		{
			__ClearAutoAttackTargetActorID();
			return;
		}
#endif
		else if (pkInstMain->IsMountingHorse() && !pkInstMain->CanAttackHorseLevel())
			__ClearAutoAttackTargetActorID();
		else if (pkInstMain->IsAttackableInstance(*pkInstVictim))
		{
			if (pkInstMain->IsSleep())
			{
				//TraceError("SKIP_AUTO_ATTACK_IN_SLEEPING");
			}
			else
				__ReserveClickActor(m_dwAutoAttackTargetVID);
		}
	}
}

void CPythonPlayer::__Update_NotifyGuildAreaEvent()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (pkInstMain)
	{
		TPixelPosition kPixelPosition;
		pkInstMain->NEW_GetPixelPosition(&kPixelPosition);

		uint32_t dwAreaID = CPythonMiniMap::Instance().GetGuildAreaID(ULONG(kPixelPosition.x), ULONG(kPixelPosition.y));

		if (dwAreaID != m_inGuildAreaID)
		{
			if (0xffffffff != dwAreaID)
				PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_Guild_EnterGuildArea", Py_BuildValue("(i)", dwAreaID));
			else
				PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_Guild_ExitGuildArea", Py_BuildValue("(i)", dwAreaID));

			m_inGuildAreaID = dwAreaID;
		}
	}
}

void CPythonPlayer::SetMainCharacterIndex(int iIndex)
{
	m_dwMainCharacterIndex = iIndex;

	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (pkInstMain)
	{
		CPythonPlayerEventHandler& rkPlayerEventHandler = CPythonPlayerEventHandler::GetSingleton();
		pkInstMain->SetEventHandler(&rkPlayerEventHandler);
	}
}

uint32_t CPythonPlayer::GetMainCharacterIndex()
{
	return m_dwMainCharacterIndex;
}

bool CPythonPlayer::IsMainCharacterIndex(uint32_t dwIndex)
{
	return (m_dwMainCharacterIndex == dwIndex);
}

uint32_t CPythonPlayer::GetGuildID()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return 0xffffffff;

	return pkInstMain->GetGuildID();
}

void CPythonPlayer::SetWeaponPower(uint32_t dwMinPower, uint32_t dwMaxPower, uint32_t dwMinMagicPower, uint32_t dwMaxMagicPower, uint32_t dwAddPower)
{
	m_dwWeaponMinPower = dwMinPower;
	m_dwWeaponMaxPower = dwMaxPower;
	m_dwWeaponMinMagicPower = dwMinMagicPower;
	m_dwWeaponMaxMagicPower = dwMaxMagicPower;
	m_dwWeaponAddPower = dwAddPower;

	__UpdateBattleStatus();
}

void CPythonPlayer::SetRace(uint32_t dwRace)
{
	m_dwRace = dwRace;
}

uint32_t CPythonPlayer::GetRace() const
{
	return m_dwRace;
}

uint32_t CPythonPlayer::__GetRaceStat()
{
	switch (GetRace())
	{
		case MAIN_RACE_WARRIOR_M:
		case MAIN_RACE_WARRIOR_W:
			return GetStatus(POINT_ST);
		case MAIN_RACE_ASSASSIN_M:
		case MAIN_RACE_ASSASSIN_W:
			return GetStatus(POINT_DX);
		case MAIN_RACE_SURA_M:
		case MAIN_RACE_SURA_W:
			return GetStatus(POINT_ST);
		case MAIN_RACE_SHAMAN_M:
		case MAIN_RACE_SHAMAN_W:
			return GetStatus(POINT_IQ);
#ifdef ENABLE_WOLFMAN_CHARACTER
		case MAIN_RACE_WOLFMAN_M:
			return GetStatus(POINT_DX);
#endif
		default:
			break;
	}
	return GetStatus(POINT_ST);
}

uint32_t CPythonPlayer::__GetLevelAtk()
{
	return 2 * GetStatus(POINT_LEVEL);
}

uint32_t CPythonPlayer::__GetStatAtk()
{
	return (4 * GetStatus(POINT_ST) + 2 * __GetRaceStat()) / 3;
}

uint32_t CPythonPlayer::__GetWeaponAtk(uint32_t dwWeaponPower) const
{
	return 2 * dwWeaponPower;
}

uint32_t CPythonPlayer::__GetTotalAtk(uint32_t dwWeaponPower, uint32_t dwRefineBonus)
{
	uint32_t dwLvAtk = __GetLevelAtk();
	uint32_t dwStAtk = __GetStatAtk();

	/////

	uint32_t dwWepAtk;
	uint32_t dwTotalAtk;

	if (LocaleService_IsCHEONMA())
	{
		dwWepAtk = __GetWeaponAtk(dwWeaponPower + dwRefineBonus);
		dwTotalAtk = dwLvAtk + (dwStAtk + dwWepAtk) * (GetStatus(POINT_DX) + 210) / 300;
	}
	else
	{
		int hr = __GetHitRate();
		dwWepAtk = __GetWeaponAtk(dwWeaponPower + dwRefineBonus);
		dwTotalAtk = dwLvAtk + (dwStAtk + dwWepAtk) * hr / 100;
	}

	return dwTotalAtk;
}

uint32_t CPythonPlayer::__GetHitRate()
{
	int src = 0;

	if (LocaleService_IsCHEONMA())
		src = GetStatus(POINT_DX);
	else
		src = (GetStatus(POINT_DX) * 4 + GetStatus(POINT_LEVEL) * 2) / 6;

	return 100 * (std::min(90, src) + 210) / 300;
}

uint32_t CPythonPlayer::__GetEvadeRate()
{
	return 30 * (2 * GetStatus(POINT_DX) + 5) / (GetStatus(POINT_DX) + 95);
}

void CPythonPlayer::__UpdateBattleStatus()
{
	m_playerStatus.SetPoint(POINT_NONE, 0);
	m_playerStatus.SetPoint(POINT_EVADE_RATE, __GetEvadeRate());
	m_playerStatus.SetPoint(POINT_HIT_RATE, __GetHitRate());
	m_playerStatus.SetPoint(POINT_MIN_WEP, m_dwWeaponMinPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MAX_WEP, m_dwWeaponMaxPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MIN_MAGIC_WEP, m_dwWeaponMinMagicPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MAX_MAGIC_WEP, m_dwWeaponMaxMagicPower + m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MIN_ATK, __GetTotalAtk(m_dwWeaponMinPower, m_dwWeaponAddPower));
	m_playerStatus.SetPoint(POINT_MAX_ATK, __GetTotalAtk(m_dwWeaponMaxPower, m_dwWeaponAddPower));
}

void CPythonPlayer::SetStatus(uint32_t dwType, long lValue)
{
	if (dwType >= POINT_MAX_NUM)
	{
		assert(!" CPythonPlayer::SetStatus - Strange Status Type!");
		Tracef("CPythonPlayer::SetStatus - Set Status Type Error\n");
		return;
	}

	if (dwType == POINT_LEVEL)
	{
		CInstanceBase* pkPlayer = NEW_GetMainActorPtr();

		if (pkPlayer)
		{
#ifdef ENABLE_TEXT_LEVEL_REFRESH
			// basically, just for the /level command to refresh locally
			pkPlayer->SetLevel(lValue);
#endif
			pkPlayer->UpdateTextTailLevel(lValue);
		}
	}

#ifdef ENABLE_YOHARA_SYSTEM
	if (dwType == POINT_CONQUEROR_LEVEL)
	{
		CInstanceBase* pkPlayer = NEW_GetMainActorPtr();

		if (pkPlayer)
		{
#	ifdef ENABLE_TEXT_LEVEL_REFRESH
			// basically, just for the /level command to refresh locally
			pkPlayer->SetConquerorLevel(lValue);
#	endif
			pkPlayer->UpdateTextTailLevel(lValue);
		}
	}
#endif

	switch (dwType)
	{
		case POINT_MIN_WEP:
		case POINT_MAX_WEP:
		case POINT_MIN_ATK:
		case POINT_MAX_ATK:
		case POINT_HIT_RATE:
		case POINT_EVADE_RATE:
		case POINT_LEVEL:
		case POINT_ST:
		case POINT_DX:
		case POINT_IQ:
			m_playerStatus.SetPoint(dwType, lValue);
			__UpdateBattleStatus();
			break;
		default:
			m_playerStatus.SetPoint(dwType, lValue);
			break;
	}
}

int CPythonPlayer::GetStatus(uint32_t dwType)
{
	if (dwType >= POINT_MAX_NUM)
	{
		assert(!" CPythonPlayer::GetStatus - Strange Status Type!");
		Tracef("CPythonPlayer::GetStatus - Get Status Type Error\n");
		return 0;
	}

	return m_playerStatus.GetPoint(dwType);
}

const char* CPythonPlayer::GetName()
{
	return m_stName.c_str();
}

void CPythonPlayer::SetName(const char* name)
{
	m_stName = name;
}

void CPythonPlayer::NotifyDeletingCharacterInstance(uint32_t dwVID)
{
	if (m_dwMainCharacterIndex == dwVID)
		m_dwMainCharacterIndex = 0;
}

void CPythonPlayer::NotifyCharacterDead(uint32_t dwVID)
{
	if (__IsSameTargetVID(dwVID))
		SetTarget(0);
}

void CPythonPlayer::NotifyCharacterUpdate(uint32_t dwVID)
{
	if (__IsSameTargetVID(dwVID))
	{
		CInstanceBase* pMainInstance = NEW_GetMainActorPtr();
		CInstanceBase* pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
		if (pMainInstance && pTargetInstance)
		{
			if (!pMainInstance->IsTargetableInstance(*pTargetInstance))
			{
				SetTarget(0);
				PyCallClassMemberFunc(m_ppyGameWindow, "CloseTargetBoard", Py_BuildValue("()"));
			}
			else
				PyCallClassMemberFunc(m_ppyGameWindow, "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
		}
	}
}

void CPythonPlayer::NotifyDeadMainCharacter()
{
	__ClearAutoAttackTargetActorID();
}

void CPythonPlayer::NotifyChangePKMode()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "OnChangePKMode", Py_BuildValue("()"));
}

#ifdef ENABLE_SKILL_COOLTIME_UPDATE
void CPythonPlayer::ResetSkillCoolTimes()
{
	if (!CPythonNetworkStream::Instance().GetMainActorSkillGroup())
		return;

	for (int i = 0; i < SKILL_MAX_NUM; ++i) {
		TSkillInstance& rkSkillInst = m_playerStatus.aSkill[i];
		if (!rkSkillInst.fLastUsedTime && !rkSkillInst.fCoolTime)
			continue;

		rkSkillInst.fLastUsedTime = rkSkillInst.fCoolTime = 0.0f;
		PyCallClassMemberFunc(m_ppyGameWindow, "SkillClearCoolTime", Py_BuildValue("(i)", i));
	}
}
#endif

void CPythonPlayer::MoveItemData(TItemPos SrcCell, TItemPos DstCell)
{
	if (!SrcCell.IsValidCell() || !DstCell.IsValidCell())
		return;

	TItemData src_item(*GetItemData(SrcCell));
	TItemData dst_item(*GetItemData(DstCell));
	SetItemData(DstCell, src_item);
	SetItemData(SrcCell, dst_item);
}

const TItemData* CPythonPlayer::GetItemData(TItemPos Cell) const
{
	if (!Cell.IsValidCell())
		return nullptr;

	switch (Cell.window_type)
	{
		case INVENTORY:
			return &m_playerStatus.aInventoryItem[Cell.cell];
		case EQUIPMENT:
			return &m_playerStatus.aEquipmentItem[Cell.cell];
#ifdef ENABLE_BELT_INVENTORY
		case BELT_INVENTORY:
			return &m_playerStatus.aBeltInventoryItem[Cell.cell];
#endif
		case DRAGON_SOUL_INVENTORY:
			return &m_playerStatus.aDSItem[Cell.cell];
#ifdef ENABLE_ATTR_6TH_7TH
		case NPC_STORAGE:
			return &m_playerStatus.aNPCItem[Cell.cell];
			break;
#endif
#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return &m_playerStatus.aSwitchbotItem[Cell.cell];
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case ADDITIONAL_EQUIPMENT_1:
			return &m_playerStatus.aAdditionalEquipmentItem_1[Cell.cell];
#endif

		default:
			return nullptr;
	}
}

void CPythonPlayer::SetItemData(TItemPos Cell, const TItemData& c_rkItemInst)
{
	if (!Cell.IsValidCell())
		return;

	if (c_rkItemInst.vnum != 0)
	{
		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(c_rkItemInst.vnum, &pItemData))
		{
			TraceError("CPythonPlayer::SetItemData(window_type : %d, dwSlotIndex=%d, itemIndex=%d) - Failed to item data\n",
				Cell.window_type, Cell.cell, c_rkItemInst.vnum);
			return;
		}
	}

	switch (Cell.window_type)
	{
		case INVENTORY:
			m_playerStatus.aInventoryItem[Cell.cell] = c_rkItemInst;
			break;

		case EQUIPMENT:
			m_playerStatus.aEquipmentItem[Cell.cell] = c_rkItemInst;
			break;

#ifdef ENABLE_BELT_INVENTORY
		case BELT_INVENTORY:
			m_playerStatus.aBeltInventoryItem[Cell.cell] = c_rkItemInst;
			break;
#endif

		case DRAGON_SOUL_INVENTORY:
			m_playerStatus.aDSItem[Cell.cell] = c_rkItemInst;
			break;

#ifdef ENABLE_ATTR_6TH_7TH
		case NPC_STORAGE:
			m_playerStatus.aNPCItem[Cell.cell] = c_rkItemInst;
			break;
#endif

#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			m_playerStatus.aSwitchbotItem[Cell.cell] = c_rkItemInst;
			break;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case ADDITIONAL_EQUIPMENT_1:
			m_playerStatus.aAdditionalEquipmentItem_1[Cell.cell] = c_rkItemInst;
			break;
#endif

	default:
		TraceError("CPythonPlayer::SetItemData unknown window type: %d.", Cell.window_type);
		break;
	}
}

uint32_t CPythonPlayer::GetItemIndex(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return GetItemData(Cell)->vnum;
}

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
void CPythonPlayer::SetAccumulateDamage(uint32_t dwVid, uint32_t dwCount)
{
	bool bContain = false;

	if (!m_hitCount.empty())
	{
		for (uint32_t i = 0; i < m_hitCount.size(); i++)
		{
			if (m_hitCount[i].dwVid == dwVid)
			{
				bContain = true;
				m_hitCount[i].dwCount = dwCount;
				break;
			}
		}
	}

	if (!bContain)
	{
		m_hitCount.push_back(THitCountInfo(dwVid, dwCount));
	}
}

uint32_t CPythonPlayer::GetAccumulateDamage(uint32_t dwVid)
{
	uint32_t dwCount = 0;

	for (uint32_t i = 0; i < m_hitCount.size(); i++)
	{
		if (m_hitCount[i].dwVid == dwVid)
		{
			dwCount = m_hitCount[i].dwCount;
			break;
		}
	}

	return dwCount;
}
#endif

#ifdef ENABLE_MONSTER_BACK
void CPythonPlayer::SetRewardItem(uint32_t dwDay, uint32_t dwVnum, uint32_t dwCount)
{
	bool bContain = false;

	if (!m_rewardItems.empty())
	{
		for (uint32_t i = 0; i < m_rewardItems.size(); i++)
		{
			if (m_rewardItems[i].dwDay == dwDay)
			{
				bContain = true;
				m_rewardItems[i].dwVnum = dwVnum;
				m_rewardItems[i].dwCount = dwCount;
				break;
			}
		}
	}

	if (!bContain)
	{
		TRewardItem tempItem{};
		tempItem.dwDay = dwDay;
		tempItem.dwVnum = dwVnum;
		tempItem.dwCount = dwCount;

		m_rewardItems.emplace_back(tempItem);
	}
}
#endif

uint32_t CPythonPlayer::GetItemFlags(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	const TItemData* pItem = GetItemData(Cell);
	assert(pItem != nullptr);
	return pItem->flags;
}

uint32_t CPythonPlayer::GetItemAntiFlags(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData* pItem = GetItemData(Cell);
	assert(pItem != nullptr);
	return pItem->anti_flags;
}

uint8_t CPythonPlayer::GetItemTypeBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData* pItemDataPtr = nullptr;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetType();
	else
	{
		TraceError("FAILED\t\tCPythonPlayer::GetItemTypeBySlot()\t\tCell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return 0;
	}
}

uint8_t CPythonPlayer::GetItemSubTypeBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData* pItemDataPtr = nullptr;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetSubType();
	else
	{
		//TraceError("FAILED\t\tCPythonPlayer::GetItemSubTypeBySlot()\t\tCell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return 0;
	}
}

uint32_t CPythonPlayer::GetItemCount(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	const TItemData* pItem = GetItemData(Cell);
	if (pItem == nullptr)
		return 0;
	return pItem->count;
}

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
uint32_t CPythonPlayer::GetItemCountByVnum(uint32_t dwVnum, bool bIgnoreSetValue)
#else
uint32_t CPythonPlayer::GetItemCountByVnum(uint32_t dwVnum)
#endif
{
	uint32_t dwCount = 0;

	for (const auto& c_rItemData : m_playerStatus.aInventoryItem)
	{
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
		if (bIgnoreSetValue && c_rItemData.set_value)
			continue;
#endif

		if (c_rItemData.vnum == dwVnum
#ifdef ENABLE_SOULBIND_SYSTEM
			&& c_rItemData.nSealDate == CItemData::SEAL_DATE_DEFAULT_TIMESTAMP
#endif
			)
			dwCount += c_rItemData.count;
	}

	return dwCount;
}

uint32_t CPythonPlayer::GetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex) const
{
	if (!Cell.IsValidCell())
		return 0;

	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return 0;

	return GetItemData(Cell)->alSockets[dwMetinSocketIndex];
}

void CPythonPlayer::GetItemAttribute(TItemPos Cell, uint32_t dwAttrSlotIndex, uint16_t* pwType, int16_t* psValue) const	//@fixme436
{
	*pwType = 0;
	*psValue = 0;

	if (!Cell.IsValidCell())
		return;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	*pwType = GetItemData(Cell)->aAttr[dwAttrSlotIndex].wType;
	*psValue = GetItemData(Cell)->aAttr[dwAttrSlotIndex].sValue;
}

void CPythonPlayer::SetItemCount(TItemPos Cell, uint8_t byCount)
{
	if (!Cell.IsValidCell())
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->count = byCount;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

void CPythonPlayer::SetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber)
{
	if (!Cell.IsValidCell())
		return;
	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->alSockets[dwMetinSocketIndex] = dwMetinNumber;
}

#ifdef ENABLE_YOHARA_SYSTEM
void CPythonPlayer::SetRandomDefaultAttrs(TItemPos Cell, uint32_t dwRandomValueIndex, uint32_t dwRandomValueNumber)
{
	if (!Cell.IsValidCell())
		return;
	if (dwRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM)
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->alRandomValues[dwRandomValueIndex] = dwRandomValueNumber;
}

uint32_t CPythonPlayer::GetRandomDefaultAttrs(TItemPos Cell, uint32_t dwRandomValueIndex) const
{
	if (!Cell.IsValidCell())
		return 0;

	if (dwRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM)
		return 0;

	return GetItemData(Cell)->alRandomValues[dwRandomValueIndex];
}
#endif

void CPythonPlayer::SetItemAttribute(TItemPos Cell, uint32_t dwAttrIndex, uint16_t wType, int16_t sValue)	//@fixme436
{
	if (!Cell.IsValidCell())
		return;
	if (dwAttrIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	(const_cast<TItemData*>(GetItemData(Cell)))->aAttr[dwAttrIndex].wType = wType;
	(const_cast<TItemData*>(GetItemData(Cell)))->aAttr[dwAttrIndex].sValue = sValue;
}

#ifdef ENABLE_SOULBIND_SYSTEM
bool CPythonPlayer::SealItem(TItemPos Cell, long nSealDate)
{
	const TItemData* pData = GetItemData(Cell);
	if (!pData)
	{
		TraceError("FAILED  CPythonPlayer::SealItem()  Cell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return false;
	}

	(const_cast <TItemData*>(pData))->nSealDate = nSealDate;

	return true;
}

bool CPythonPlayer::UnSealItem(TItemPos Cell, long nSealDate)
{
	const TItemData* pData = GetItemData(Cell);
	if (!pData)
	{
		TraceError("FAILED  CPythonPlayer::SealItem()  Cell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return false;
	}

	(const_cast <TItemData*>(pData))->nSealDate = nSealDate;

	return true;
}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
uint8_t CPythonPlayer::GetItemMagicPctBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData* pItemDataPtr = nullptr;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetMagicPct();
	else
	{
		TraceError("FAILED\t\tCPythonPlayer::GetItemMagicPctBySlot()\t\tCell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return 0;
	}
}

void CPythonPlayer::SetItemCombinationWindowActivedItemSlot(uint32_t iSlot, uint32_t iIndex)
{
	CombinationSlot[iSlot] = iIndex;
}

uint32_t CPythonPlayer::GetConbWindowSlotByAttachedInvenSlot(uint32_t iSlot)
{
	return CombinationSlot[iSlot];
}
#endif

int CPythonPlayer::GetQuickPage() const
{
	return m_playerStatus.lQuickPageIndex;
}

void CPythonPlayer::SetQuickPage(int nQuickPageIndex)
{
	if (nQuickPageIndex < 0)
		m_playerStatus.lQuickPageIndex = QUICKSLOT_MAX_LINE + nQuickPageIndex;
	else if (nQuickPageIndex >= QUICKSLOT_MAX_LINE)
		m_playerStatus.lQuickPageIndex = nQuickPageIndex % QUICKSLOT_MAX_LINE;
	else
		m_playerStatus.lQuickPageIndex = nQuickPageIndex;

	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

uint32_t CPythonPlayer::LocalQuickSlotIndexToGlobalQuickSlotIndex(uint32_t dwLocalSlotIndex) const
{
	return m_playerStatus.lQuickPageIndex * QUICKSLOT_MAX_COUNT_PER_LINE + dwLocalSlotIndex;
}

void CPythonPlayer::GetGlobalQuickSlotData(uint32_t dwGlobalSlotIndex, uint32_t* pdwWndType, uint32_t* pdwWndItemPos)
{
	TQuickSlot& rkQuickSlot = __RefGlobalQuickSlot(dwGlobalSlotIndex);
	*pdwWndType = rkQuickSlot.Type;
	*pdwWndItemPos = rkQuickSlot.Position;
}

void CPythonPlayer::GetLocalQuickSlotData(uint32_t dwSlotPos, uint32_t* pdwWndType, uint32_t* pdwWndItemPos)
{
	TQuickSlot& rkQuickSlot = __RefLocalQuickSlot(dwSlotPos);
	*pdwWndType = rkQuickSlot.Type;
	*pdwWndItemPos = rkQuickSlot.Position;
}

TQuickSlot& CPythonPlayer::__RefLocalQuickSlot(int SlotIndex)
{
	return __RefGlobalQuickSlot(LocalQuickSlotIndexToGlobalQuickSlotIndex(SlotIndex));
}

TQuickSlot& CPythonPlayer::__RefGlobalQuickSlot(int SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= QUICKSLOT_MAX_NUM)
	{
		static TQuickSlot s_kQuickSlot;
		s_kQuickSlot.Type = 0;
		s_kQuickSlot.Position = 0;
		return s_kQuickSlot;
	}

	return m_playerStatus.aQuickSlot[SlotIndex];
}

void CPythonPlayer::RemoveQuickSlotByValue(int iType, int iPosition) const
{
	for (uint8_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (iType == m_playerStatus.aQuickSlot[i].Type)
			if (iPosition == m_playerStatus.aQuickSlot[i].Position)
				CPythonNetworkStream::Instance().SendQuickSlotDelPacket(i);
	}
}

char CPythonPlayer::IsItem(TItemPos Cell) const
{
	if (!Cell.IsValidCell())
		return 0;

	return 0 != GetItemData(Cell)->vnum;
}

void CPythonPlayer::RequestMoveGlobalQuickSlotToLocalQuickSlot(uint32_t dwGlobalSrcSlotIndex, uint32_t dwLocalDstSlotIndex) const
{
	//uint32_t dwGlobalSrcSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSrcSlotIndex);
	uint32_t dwGlobalDstSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalDstSlotIndex);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotMovePacket(static_cast<uint8_t>(dwGlobalSrcSlotIndex), static_cast<uint8_t>(dwGlobalDstSlotIndex));
}

void CPythonPlayer::RequestAddLocalQuickSlot(uint32_t dwLocalSlotIndex, uint32_t dwWndType, uint32_t dwWndItemPos) const
{
	if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
		return;

	uint32_t dwGlobalSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotAddPacket(static_cast<uint8_t>(dwGlobalSlotIndex), static_cast<uint8_t>(dwWndType),
		static_cast<uint8_t>(dwWndItemPos));
}

void CPythonPlayer::RequestAddToEmptyLocalQuickSlot(uint32_t dwWndType, uint32_t dwWndItemPos)
{
	for (int i = 0; i < QUICKSLOT_MAX_COUNT_PER_LINE; ++i)
	{
		TQuickSlot& rkQuickSlot = __RefLocalQuickSlot(i);

		if (0 == rkQuickSlot.Type)
		{
			uint32_t dwGlobalQuickSlotIndex = LocalQuickSlotIndexToGlobalQuickSlotIndex(i);
			CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
			rkNetStream.SendQuickSlotAddPacket(static_cast<uint8_t>(dwGlobalQuickSlotIndex), static_cast<uint8_t>(dwWndType),
				static_cast<uint8_t>(dwWndItemPos));
			return;
		}
	}
}

void CPythonPlayer::RequestDeleteGlobalQuickSlot(uint32_t dwGlobalSlotIndex) const
{
	if (dwGlobalSlotIndex >= QUICKSLOT_MAX_COUNT)
		return;

	//if (dwLocalSlotIndex>=QUICKSLOT_MAX_SLOT_PER_LINE)
	//	return;

	//uint32_t dwGlobalSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotDelPacket(static_cast<uint8_t>(dwGlobalSlotIndex));
}

void CPythonPlayer::RequestUseLocalQuickSlot(uint32_t dwLocalSlotIndex)
{
	if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
		return;

	uint32_t dwRegisteredType;
	uint32_t dwRegisteredItemPos;
	GetLocalQuickSlotData(dwLocalSlotIndex, &dwRegisteredType, &dwRegisteredItemPos);

	switch (dwRegisteredType)
	{
		case SLOT_TYPE_INVENTORY:
		{
			CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
			rkNetStream.SendItemUsePacket(TItemPos(INVENTORY, static_cast<uint16_t>(dwRegisteredItemPos)));
		}
		break;
		case SLOT_TYPE_SKILL:
		{
			ClickSkillSlot(dwRegisteredItemPos);
		}
		break;
		case SLOT_TYPE_EMOTION:
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_ActEmotion", Py_BuildValue("(i)", dwRegisteredItemPos));
		}
		break;
#ifdef ENABLE_BELT_INVENTORY
		case SLOT_TYPE_BELT_INVENTORY:
		{
			CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
			rkNetStream.SendItemUsePacket(TItemPos(BELT_INVENTORY, static_cast<uint16_t>(dwRegisteredItemPos)));
		}
		break;
#endif
		default:
			break;
	}
}

void CPythonPlayer::AddQuickSlot(int QuickSlotIndex, char IconType, char IconPosition)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	m_playerStatus.aQuickSlot[QuickSlotIndex].Type = IconType;
	m_playerStatus.aQuickSlot[QuickSlotIndex].Position = IconPosition;
}

void CPythonPlayer::DeleteQuickSlot(int QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	m_playerStatus.aQuickSlot[QuickSlotIndex].Type = 0;
	m_playerStatus.aQuickSlot[QuickSlotIndex].Position = 0;
}

void CPythonPlayer::MoveQuickSlot(int Source, int Target)
{
	if (Source < 0 || Source >= QUICKSLOT_MAX_NUM)
		return;

	if (Target < 0 || Target >= QUICKSLOT_MAX_NUM)
		return;

	TQuickSlot& rkSrcSlot = __RefGlobalQuickSlot(Source);
	TQuickSlot& rkDstSlot = __RefGlobalQuickSlot(Target);

	std::swap(rkSrcSlot, rkDstSlot);
}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
bool CPythonPlayer::IsBeltInventorySlot(TItemPos Cell) const
{
	return Cell.IsBeltInventoryCell();
}
#endif

bool CPythonPlayer::IsInventorySlot(TItemPos Cell) const
{
	return !Cell.IsEquipCell() && Cell.IsValidCell();
}

bool CPythonPlayer::IsEquipmentSlot(TItemPos Cell) const
{
	return Cell.IsEquipCell();
}

bool CPythonPlayer::IsEquipItemInSlot(TItemPos Cell)
{
	if (!Cell.IsEquipCell())
		return false;

	const TItemData* pData = GetItemData(Cell);

	if (nullptr == pData)
		return false;

	uint32_t dwItemIndex = pData->vnum;

	CItemManager::Instance().SelectItemData(dwItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
	{
		TraceError("Failed to find ItemData - CPythonPlayer::IsEquipItem(window_type=%d, iSlotindex=%d)\n", Cell.window_type, Cell.cell);
		return false;
	}

	return pItemData->IsEquipment() ? true : false;
}


void CPythonPlayer::SetSkill(uint32_t dwSlotIndex, uint32_t dwSkillIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	m_playerStatus.aSkill[dwSlotIndex].dwIndex = dwSkillIndex;
	m_skillSlotDict[dwSkillIndex] = dwSlotIndex;
}

int CPythonPlayer::GetSkillIndex(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].dwIndex;
}

bool CPythonPlayer::GetSkillSlotIndex(uint32_t dwSkillIndex, uint32_t* pdwSlotIndex)
{
	auto f = m_skillSlotDict.find(dwSkillIndex);
	if (m_skillSlotDict.end() == f)
		return false;

	*pdwSlotIndex = f->second;

	return true;
}

int CPythonPlayer::GetSkillGrade(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].iGrade;
}

int CPythonPlayer::GetSkillLevel(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].iLevel;
}

float CPythonPlayer::GetSkillCurrentEfficientPercentage(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage;
}

float CPythonPlayer::GetSkillNextEfficientPercentage(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage;
}

void CPythonPlayer::SetSkillLevel(uint32_t dwSlotIndex, uint32_t dwSkillLevel)
{
	assert(!"CPythonPlayer::SetSkillLevel - Don't use this function");
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	m_playerStatus.aSkill[dwSlotIndex].iGrade = -1;
	m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel;
}

void CPythonPlayer::SetSkillLevel_(uint32_t dwSkillIndex, uint32_t dwSkillGrade, uint32_t dwSkillLevel)
{
	uint32_t dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
		return;

	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	switch (dwSkillGrade)
	{
		case 0:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel;
			break;
		case 1:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
#ifdef ENABLE_REVERSED_FUNCTIONS
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 19;
#else
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 20 + 1;
#endif
			break;
		case 2:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
#ifdef ENABLE_REVERSED_FUNCTIONS
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 29;
#else
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 30 + 1;
#endif
			break;
		case 3:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
#ifdef ENABLE_REVERSED_FUNCTIONS
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 39;
#else
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel - 40 + 1;
#endif
			break;
	}

	const uint32_t SKILL_MAX_LEVEL = 40;


	if (dwSkillLevel > SKILL_MAX_LEVEL)
	{
		m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage = 0.0f;
		m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage = 0.0f;

#ifdef _DEBUG
		TraceError("CPythonPlayer::SetSkillLevel(SlotIndex=%d, SkillLevel=%d)", dwSlotIndex, dwSkillLevel);
#endif
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage = LocaleService_GetSkillPower(dwSkillLevel) / 100.0f;
	m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage = LocaleService_GetSkillPower(dwSkillLevel + 1) / 100.0f;
}

float CPythonPlayer::GetSkillCoolTime(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0.0f;

	return m_playerStatus.aSkill[dwSlotIndex].fCoolTime;
}

float CPythonPlayer::GetSkillElapsedCoolTime(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0.0f;

	return CTimer::Instance().GetCurrentSecond() - m_playerStatus.aSkill[dwSlotIndex].fLastUsedTime;
}

void CPythonPlayer::__ActivateSkillSlot(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::ActivavteSkill(dwSlotIndex=%d/%d) - OUT OF RANGE", dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].bActive = TRUE;
	PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSkillSlot", Py_BuildValue("(i)", dwSlotIndex));
}

void CPythonPlayer::__DeactivateSkillSlot(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::DeactivavteSkill(dwSlotIndex=%d/%d) - OUT OF RANGE", dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].bActive = FALSE;
	PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSkillSlot", Py_BuildValue("(i)", dwSlotIndex));
}

BOOL CPythonPlayer::IsSkillCoolTime(uint32_t dwSlotIndex)
{
	if (!__CheckRestSkillCoolTime(dwSlotIndex))
		return FALSE;

	return TRUE;
}

BOOL CPythonPlayer::IsSkillActive(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return FALSE;

	return m_playerStatus.aSkill[dwSlotIndex].bActive;
}

BOOL CPythonPlayer::IsToggleSkill(uint32_t dwSlotIndex) const
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return FALSE;

	uint32_t dwSkillIndex = m_playerStatus.aSkill[dwSlotIndex].dwIndex;

	CPythonSkill::TSkillData* pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return FALSE;

	return pSkillData->IsToggleSkill();
}

void CPythonPlayer::SetPlayTime(uint32_t dwPlayTime)
{
	m_dwPlayTime = dwPlayTime;
}

uint32_t CPythonPlayer::GetPlayTime() const
{
	return m_dwPlayTime;
}

void CPythonPlayer::SendClickItemPacket(uint32_t dwIID)
{
	if (IsObserverMode())
		return;

#ifndef ENABLE_NO_PICKUP_LIMIT
	static uint32_t s_dwNextTCPTime = 0;
	uint32_t dwCurTime = ELTimer_GetMSec();
	if (dwCurTime >= s_dwNextTCPTime)
#endif
	{
#ifndef ENABLE_NO_PICKUP_LIMIT
		// Changed the pickup delay for better user experience - [MartPwnS]
		// Lowered the pickup delay to 100 - [HuNterukh]
		s_dwNextTCPTime = dwCurTime + PICKUP_DELAY;	//@custom001
#endif

		const char* c_szOwnerName;
		if (!CPythonItem::Instance().GetOwnership(dwIID, &c_szOwnerName))
			return;

		if (strlen(c_szOwnerName) > 0)
		{
			if (0 != strcmp(c_szOwnerName, GetName()))
			{
				CItemData* pItemData;
				if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
				{
					Tracenf("CPythonPlayer::SendClickItemPacket(dwIID=%d) : Non-exist item.", dwIID);
					return;
				}
				if (!IsPartyMemberByName(c_szOwnerName) ||
					pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_DROP | CItemData::ITEM_ANTIFLAG_GIVE))
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotPickItem", Py_BuildValue("()"));
					return;
				}
			}
		}

#ifdef ENABLE_PICKUP_SOUND_EFFECT
		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
			return;
		 // -1: all subtypes
		 // don't forget to add #include <tuple>
		const std::vector<std::tuple<uint8_t, uint8_t, std::string>> pickup {
			{ CItemData::ITEM_TYPE_ELK, -1, "money.wav" },
			{ CItemData::ITEM_TYPE_WEAPON, -1, "pickup_item_in_inventory.wav" },
			//{ CItemData::ITEM_TYPE_ARMOR, CItemData::ARMOR_BODY, "bracelet.wav" },
			//{ CItemData::ITEM_TYPE_ARMOR, CItemData::ARMOR_SHIELD, "drop.wav" },
			{ CItemData::ITEM_TYPE_ARMOR, -1, "itemget.wav" }, // rest
		};

		std::string base = "sound/ui/";
		const decltype(pickup)::const_iterator it = std::find_if(pickup.begin(), pickup.end(), [&pItemData](const std::tuple<uint8_t, uint8_t, std::string>& t) { return pItemData->GetType() == std::get<0>(t) && (pItemData->GetSubType() == std::get<1>(t) || static_cast<uint8_t>(-1) == std::get<1>(t));});
		base += it != pickup.end() ? std::get<2>(*it) : "close.wav";
		CSoundManager::Instance().PlaySound2D(base.c_str());
#endif

		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
		rkNetStream.SendItemPickUpPacket(dwIID);
	}
}

void CPythonPlayer::__SendClickActorPacket(CInstanceBase& rkInstVictim)
{
	// 말을 타고 광산을 캐는 것에 대한 예외 처리
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (pkInstMain)
	{
		if (pkInstMain->IsHoldingPickAxe())
		{
			if (pkInstMain->IsMountingHorse())
			{
				if (rkInstVictim.IsResource())
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotMining", Py_BuildValue("()"));
					return;
				}
			}
		}
	}

	static uint32_t s_dwNextTCPTime = 0;

	uint32_t dwCurTime = ELTimer_GetMSec();

	if (dwCurTime >= s_dwNextTCPTime)
	{
		s_dwNextTCPTime = dwCurTime + 1000;

		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

		uint32_t dwVictimVID = rkInstVictim.GetVirtualID();
		rkNetStream.SendOnClickPacket(dwVictimVID);
	}
}

void CPythonPlayer::ActEmotion(uint32_t dwEmotionID)
{
	CInstanceBase* pkInstTarget = __GetAliveTargetInstancePtr();
	if (!pkInstTarget)
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_TARGET"));
		return;
	}

	CPythonNetworkStream::Instance().SendChatPacket(_getf("/kiss %s", pkInstTarget->GetNameString()));
}

void CPythonPlayer::StartEmotionProcess()
{
	__ClearReservedAction();
	__ClearAutoAttackTargetActorID();

	m_bisProcessingEmotion = TRUE;
}

void CPythonPlayer::EndEmotionProcess()
{
	m_bisProcessingEmotion = FALSE;
}

BOOL CPythonPlayer::__IsProcessingEmotion() const
{
	return m_bisProcessingEmotion;
}

// Dungeon
void CPythonPlayer::SetDungeonDestinationPosition(int ix, int iy)
{
	m_isDestPosition = TRUE;
	m_ixDestPos = ix;
	m_iyDestPos = iy;

	AlarmHaveToGo();
}

void CPythonPlayer::AlarmHaveToGo()
{
	m_iLastAlarmTime = CTimer::Instance().GetCurrentMillisecond();

	/////

	CInstanceBase* pInstance = NEW_GetMainActorPtr();
	if (!pInstance)
		return;

	TPixelPosition PixelPosition;
	pInstance->NEW_GetPixelPosition(&PixelPosition);

	float fAngle = GetDegreeFromPosition2(PixelPosition.x, PixelPosition.y, float(m_ixDestPos), float(m_iyDestPos));
	fAngle = fmod(540.0f - fAngle, 360.0f);
	D3DXVECTOR3 v3Rotation(0.0f, 0.0f, fAngle);

	PixelPosition.y *= -1.0f;

	CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");
	CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_middle.mse", PixelPosition, v3Rotation);
}

// Party
void CPythonPlayer::ExitParty()
{
	m_PartyMemberMap.clear();

	CPythonCharacterManager::Instance().RefreshAllPCTextTail();
}

void CPythonPlayer::AppendPartyMember(uint32_t dwPID, const char* c_szName)
{
	m_PartyMemberMap.emplace(dwPID, TPartyMemberInfo(dwPID, c_szName));
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	if (strlen(c_szName) == 0) // offline
		CPythonMiniMap::Instance().RemovePartyPositionInfo(dwPID);
#endif
}

void CPythonPlayer::LinkPartyMember(uint32_t dwPID, uint32_t dwVID)
{
	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::LinkPartyMember(dwPID=%d, dwVID=%d) - Failed to find party member", dwPID, dwVID);
		return;
	}

	pPartyMemberInfo->dwVID = dwVID;

	CInstanceBase* pInstance = NEW_FindActorPtr(dwVID);
	if (pInstance)
		pInstance->RefreshTextTail();
}

void CPythonPlayer::UnlinkPartyMember(uint32_t dwPID)
{
	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UnlinkPartyMember(dwPID=%d) - Failed to find party member", dwPID);
		return;
	}

	pPartyMemberInfo->dwVID = 0;
}

void CPythonPlayer::UpdatePartyMemberInfo(uint32_t dwPID, uint8_t byState, uint8_t byHPPercentage)
{
	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberInfo(dwPID=%d, byState=%d, byHPPercentage=%d) - Failed to find character", dwPID,
			byState, byHPPercentage);
		return;
	}

	pPartyMemberInfo->byState = byState;
	pPartyMemberInfo->byHPPercentage = byHPPercentage;
}

void CPythonPlayer::UpdatePartyMemberAffect(uint32_t dwPID, uint8_t byAffectSlotIndex, int16_t sAffectNumber)
{
	if (byAffectSlotIndex >= PARTY_AFFECT_SLOT_MAX_NUM)
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberAffect(dwPID=%d, byAffectSlotIndex=%d, sAffectNumber=%d) - Strange affect slot index",
			dwPID, byAffectSlotIndex, sAffectNumber);
		return;
	}

	TPartyMemberInfo* pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberAffect(dwPID=%d, byAffectSlotIndex=%d, sAffectNumber=%d) - Failed to find character",
			dwPID, byAffectSlotIndex, sAffectNumber);
		return;
	}

	pPartyMemberInfo->sAffects[byAffectSlotIndex] = sAffectNumber;
}

void CPythonPlayer::RemovePartyMember(uint32_t dwPID)
{
	uint32_t dwVID = 0;
	TPartyMemberInfo* pPartyMemberInfo;
	if (GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
		dwVID = pPartyMemberInfo->dwVID;

	m_PartyMemberMap.erase(dwPID);

	if (dwVID > 0)
	{
		CInstanceBase* pInstance = NEW_FindActorPtr(dwVID);
		if (pInstance)
			pInstance->RefreshTextTail();
	}
}

bool CPythonPlayer::IsPartyMemberByVID(uint32_t dwVID)
{
	auto itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo& rPartyMemberInfo = itor->second;
		if (dwVID == rPartyMemberInfo.dwVID)
			return true;
	}

	return false;
}

bool CPythonPlayer::IsPartyMemberByName(const char* c_szName)
{
	auto itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo& rPartyMemberInfo = itor->second;
		if (c_szName == rPartyMemberInfo.strName)
			return true;
	}

	return false;
}

bool CPythonPlayer::GetPartyMemberPtr(uint32_t dwPID, TPartyMemberInfo** ppPartyMemberInfo)
{
	auto itor = m_PartyMemberMap.find(dwPID);

	if (m_PartyMemberMap.end() == itor)
		return false;

	*ppPartyMemberInfo = &(itor->second);

	return true;
}

bool CPythonPlayer::PartyMemberPIDToVID(uint32_t dwPID, uint32_t* pdwVID)
{
	auto itor = m_PartyMemberMap.find(dwPID);

	if (m_PartyMemberMap.end() == itor)
		return false;

	const TPartyMemberInfo& c_rPartyMemberInfo = itor->second;
	*pdwVID = c_rPartyMemberInfo.dwVID;

	return true;
}

bool CPythonPlayer::PartyMemberVIDToPID(uint32_t dwVID, uint32_t* pdwPID)
{
	auto itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo& rPartyMemberInfo = itor->second;
		if (dwVID == rPartyMemberInfo.dwVID)
		{
			*pdwPID = rPartyMemberInfo.dwPID;
			return true;
		}
	}

	return false;
}

bool CPythonPlayer::IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2)
{
	return (IsPartyMemberByVID(dwVID1) && IsPartyMemberByVID(dwVID2));
}

// PVP
void CPythonPlayer::RememberChallengeInstance(uint32_t dwVID)
{
	m_RevengeInstanceSet.erase(dwVID);
	m_ChallengeInstanceSet.emplace(dwVID);
}
void CPythonPlayer::RememberRevengeInstance(uint32_t dwVID)
{
	m_ChallengeInstanceSet.erase(dwVID);
	m_RevengeInstanceSet.emplace(dwVID);
}
void CPythonPlayer::RememberCantFightInstance(uint32_t dwVID)
{
	m_CantFightInstanceSet.emplace(dwVID);
}
void CPythonPlayer::ForgetInstance(uint32_t dwVID)
{
	m_ChallengeInstanceSet.erase(dwVID);
	m_RevengeInstanceSet.erase(dwVID);
	m_CantFightInstanceSet.erase(dwVID);
}

bool CPythonPlayer::IsChallengeInstance(uint32_t dwVID)
{
	return m_ChallengeInstanceSet.end() != m_ChallengeInstanceSet.find(dwVID);
}
bool CPythonPlayer::IsRevengeInstance(uint32_t dwVID)
{
	return m_RevengeInstanceSet.end() != m_RevengeInstanceSet.find(dwVID);
}
bool CPythonPlayer::IsCantFightInstance(uint32_t dwVID)
{
	return m_CantFightInstanceSet.end() != m_CantFightInstanceSet.find(dwVID);
}

void CPythonPlayer::OpenPrivateShop()
{
	m_isOpenPrivateShop = TRUE;
}
void CPythonPlayer::ClosePrivateShop()
{
	m_isOpenPrivateShop = FALSE;
}

bool CPythonPlayer::IsOpenPrivateShop() const
{
	return m_isOpenPrivateShop;
}

void CPythonPlayer::SetObserverMode(bool isEnable)
{
	m_isObserverMode = isEnable;
}

bool CPythonPlayer::IsObserverMode() const
{
	return m_isObserverMode;
}

BOOL CPythonPlayer::__ToggleCoolTime()
{
	m_sysIsCoolTime = 1 - m_sysIsCoolTime;
	return m_sysIsCoolTime;
}

BOOL CPythonPlayer::__ToggleLevelLimit()
{
	m_sysIsLevelLimit = 1 - m_sysIsLevelLimit;
	return m_sysIsLevelLimit;
}

void CPythonPlayer::StartStaminaConsume(uint32_t dwConsumePerSec, uint32_t dwCurrentStamina)
{
	m_isConsumingStamina = TRUE;
	m_fConsumeStaminaPerSec = float(dwConsumePerSec);
	m_fCurrentStamina = float(dwCurrentStamina);

	SetStatus(POINT_STAMINA, dwCurrentStamina);
}

void CPythonPlayer::StopStaminaConsume(uint32_t dwCurrentStamina)
{
	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = float(dwCurrentStamina);

	SetStatus(POINT_STAMINA, dwCurrentStamina);
}

uint32_t CPythonPlayer::GetPKMode()
{
	CInstanceBase* pInstance = NEW_GetMainActorPtr();
	if (!pInstance)
		return 0;

	return pInstance->GetPKMode();
}

void CPythonPlayer::SetMobileFlag(BOOL bFlag)
{
	m_bMobileFlag = bFlag;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshMobile", Py_BuildValue("()"));
}

BOOL CPythonPlayer::HasMobilePhoneNumber() const
{
	return m_bMobileFlag;
}

#ifdef ENABLE_KEYCHANGE_SYSTEM
void CPythonPlayer::OpenKeyChangeWindow()
{
	if (!m_isOpenKeySettingWindow)
		PyCallClassMemberFunc(m_ppyGameWindow, "OpenKeyChangeWindow", Py_BuildValue("()"));
}
#endif

void CPythonPlayer::SetGameWindow(PyObject* ppyObject)
{
	m_ppyGameWindow = ppyObject;
}

void CPythonPlayer::NEW_ClearSkillData(bool bAll)
{
	for (auto it = m_skillSlotDict.begin(); it != m_skillSlotDict.end();)
	{
		if (bAll || __GetSkillType(it->first) == CPythonSkill::SKILL_TYPE_ACTIVE)
			it = m_skillSlotDict.erase(it);
		else
			++it;
	}

	for (auto& i : m_playerStatus.aSkill)
		ZeroMemory(&i, sizeof(TSkillInstance));

	for (auto& j : m_playerStatus.aSkill)
	{
		// 2004.09.30.myevan.스킬갱신시 스킬 포인트업[+] 버튼이 안나와 처리
		j.iGrade = 0;
		j.fcurEfficientPercentage = 0.0f;
		j.fnextEfficientPercentage = 0.05f;
	}

	if (m_ppyGameWindow)
		PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_CheckGameButton", Py_BuildNone());
}

void CPythonPlayer::ClearSkillDict()
{
	// ClearSkillDict
	m_skillSlotDict.clear();

	// Game End - Player Data Reset
	m_isOpenPrivateShop = false;
	m_isObserverMode = false;

	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = 0.0f;

	m_bMobileFlag = FALSE;

	__ClearAutoAttackTargetActorID();
}

void CPythonPlayer::Clear()
{
	m_playerStatus = {};
	NEW_ClearSkillData(true);

	m_bisProcessingEmotion = FALSE;

	m_dwSendingTargetVID = 0;
	m_fTargetUpdateTime = 0.0f;

	// Test Code for Status Interface
	m_stName = "";
	m_dwMainCharacterIndex = 0;
	m_dwRace = 0;
	m_dwWeaponMinPower = 0;
	m_dwWeaponMaxPower = 0;
	m_dwWeaponMinMagicPower = 0;
	m_dwWeaponMaxMagicPower = 0;
	m_dwWeaponAddPower = 0;

	/////
	m_MovingCursorPosition = TPixelPosition(0, 0, 0);
	m_fMovingCursorSettingTime = 0.0f;

	m_eReservedMode = MODE_NONE;
	m_fReservedDelayTime = 0.0f;
	m_kPPosReserved = TPixelPosition(0, 0, 0);
	m_dwVIDReserved = 0;
	m_dwIIDReserved = 0;
	m_dwSkillSlotIndexReserved = 0;
	m_dwSkillRangeReserved = 0;

	m_isUp = false;
	m_isDown = false;
	m_isLeft = false;
	m_isRight = false;
	m_isSmtMov = false;
	m_isDirMov = false;
	m_isDirKey = false;
	m_isAtkKey = false;

	m_isCmrRot = true;
	m_fCmrRotSpd = 20.0f;

	m_iComboOld = 0;

	m_dwVIDPicked = 0;
	m_dwIIDPicked = 0;

	m_dwcurSkillSlotIndex = uint32_t(-1);

	m_dwTargetVID = 0;
	m_dwTargetEndTime = 0;

	m_PartyMemberMap.clear();

	m_ChallengeInstanceSet.clear();
	m_RevengeInstanceSet.clear();

	m_isOpenPrivateShop = false;
	m_isObserverMode = false;

	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = 0.0f;

	m_inGuildAreaID = 0xffffffff;

#ifdef ENABLE_KEYCHANGE_SYSTEM
	m_isOpenKeySettingWindow = FALSE;
	m_keySettingMap.clear();
#endif

	m_bMobileFlag = FALSE;

	__ClearAutoAttackTargetActorID();

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	m_bExtendInvenStage = 0;
	m_bExtendInvenMax = 0;
#endif

#ifdef ENABLE_OFFICAL_FEATURES
	m_isOpenSafeBox = false;
	m_isOpenMall = false;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	m_isAcceRefineWindowOpen = false;
	m_iAcceRefineWindowType = 0;

	m_iAcceActivedItemSlot[0] = 180;
	m_iAcceActivedItemSlot[1] = 180;
	m_iAcceActivedItemSlot[2] = 180;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	m_isOpenPetHatchingWindow = false;
	m_isOpenPetFeedWindow = false;

#	ifdef ENABLE_PET_ATTR_DETERMINE
	PetAttrChangeSlot[0] = -1;
	PetAttrChangeSlot[1] = -1;
	PetAttrChangeSlot[2] = -1;
#	endif

	m_GrowthPetInfo.clear();
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
	// need optimization (vector)
	CombinationSlot[0] = -1;
	CombinationSlot[1] = -1;
	CombinationSlot[2] = -1;
#endif

#ifdef ENABLE_BATTLE_FIELD
	bBattleFieldIsButtonFlush = false;
	bBattleFieldIsOpen = false;
	bBattleFieldIsEnabled = false;
	bBattleFieldIsEventOpen = false;
	bBattleFieldIsEventEnable = false;

	iBattleFieldStartTime = 0;
	iBattleFieldEndTime = 0;
#endif

#ifdef ENABLE_AURA_SYSTEM
	m_AuraItemInstanceVector.clear();
	m_AuraItemInstanceVector.resize(AURA_SLOT_MAX);
	for (size_t i = 0; i < m_AuraItemInstanceVector.size(); ++i)
	{
		TItemData& rkAuraItemInstance = m_AuraItemInstanceVector[i];
		ZeroMemory(&rkAuraItemInstance, sizeof(TItemData));
	}

	m_bAuraWindowOpen = false;
	m_bOpenedAuraWindowType = AURA_WINDOW_TYPE_MAX;

	for (uint8_t j = 0; j < AURA_SLOT_MAX; ++j)
		m_AuraRefineActivatedCell[j] = NPOS;

	ZeroMemory(&m_bAuraRefineInfo, sizeof(TAuraRefineInfo));
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	m_GemItemsMap.clear();
	m_pRefreshTime = 0;
	memset(bGemShopOpenSlotItemCount, 0, sizeof(bGemShopOpenSlotItemCount));
	m_isOpenGemShopWindow = false;
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	ClearShopSearchData();
#endif

#ifdef ENABLE_CHANGED_ATTR
	memset(m_SelectAttrArr, 0, sizeof m_SelectAttrArr);
#endif

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	m_hitCount.clear();
#endif

#ifdef ENABLE_MONSTER_BACK
	m_rewardItems.clear();
#endif

#ifdef ENABLE_AUTO_SYSTEM
	memset(&m_playerStatus, 0, sizeof(m_playerStatus));
	autoStatus = false;
	autoStart = true;	//false
	autoPause = false;
	autohuntStartLocation = TPixelPosition(0, 0, 0);
	findTargetMs = 0;

	zLastSec = 0;
	kpLastMs = 0;
	mpLastMs = 0;
	auto_HP = false;
	autp_MP = false;
	affect_control = 0;

	autoAttackOnOff = false;
	autoSkillOnOff = false;
	autoPositionsOnOff = false;
	autoRangeOnOff = false;
	AutoRestart = false;
#endif

#ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
	m_bParaState = false;
#endif
#if defined(ENABLE_AUTO_SYSTEM) || defined(ENABLE_FLOWER_EVENT)
	m_mapAffectData.clear();
#endif
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CPythonPlayer::SetOpenPetHatchingWindow(bool isOpen)
{
	m_isOpenPetHatchingWindow = isOpen;
}

bool CPythonPlayer::IsOpenPetHatchingWindow()
{
	return m_isOpenPetHatchingWindow;
}

void CPythonPlayer::SetOpenPetFeedWindow(bool isOpen)
{
	m_isOpenPetFeedWindow = isOpen;
}

bool CPythonPlayer::IsOpenPetFeedWindow()
{
	return m_isOpenPetFeedWindow;
}
#endif

#ifdef ENABLE_ACCE_SECOND_COSTUME_SYSTEM
void CPythonPlayer::RefreshAcceWindow()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
	PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_ServerCommand_Run", Py_BuildValue("(s)", "RefreshAcceWindow"));
}

void CPythonPlayer::ClearAcceActivedItemSlot(int iAcceSlot)
{
	int iSlotIndex = m_iAcceActivedItemSlot[iAcceSlot];
	m_iAcceActivedItemSlot[iAcceSlot] = 180; // Clear Slot

	// Callback
	PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSlot", Py_BuildValue("(ii)", iSlotIndex, 0));
	RefreshAcceWindow();
}

void CPythonPlayer::SetAcceActivedItemSlot(int iAcceSlot, int iSlotIndex)
{
	m_iAcceActivedItemSlot[iAcceSlot] = iSlotIndex;

	// Callback
	PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSlot", Py_BuildValue("(ii)", iSlotIndex, 0));
	RefreshAcceWindow();
}
#endif

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
void CPythonPlayer::SetItemAttrChangeWindowActivedItemSlot(uint32_t iSlot, uint32_t iIndex)
{
	PetAttrChangeSlot[iSlot] = iIndex;
}

uint32_t CPythonPlayer::GetAttrChangeWindowSlotByAttachedInvenSlot(uint32_t iSlot)
{
	return PetAttrChangeSlot[iSlot];
}
#endif

#ifdef ENABLE_BATTLE_FIELD
bool CPythonPlayer::IsBattleButtonFlush() const
{
	return bBattleFieldIsButtonFlush;
}

void CPythonPlayer::SetBattleButtonFlush(bool bIsFlush)
{
	bBattleFieldIsButtonFlush = bIsFlush;
}

bool CPythonPlayer::IsBattleFieldOpen() const
{
	return bBattleFieldIsOpen;
}

void CPythonPlayer::SetBattleFieldOpen(bool bIsOpen)
{
	bBattleFieldIsOpen = bIsOpen;
}

bool CPythonPlayer::GetBattleFieldEnable() const
{
	return bBattleFieldIsEnabled;
}

void CPythonPlayer::SetBattleFieldInfo(bool bIsOpen)
{
	bBattleFieldIsEnabled = bIsOpen;
}

bool CPythonPlayer::IsBattleFieldEventOpen() const
{
	return bBattleFieldIsEventOpen;
}

void CPythonPlayer::SetBattleFieldEventOpen(bool bIsEventOpen)
{
	bBattleFieldIsEventOpen = bIsEventOpen;
}

bool CPythonPlayer::GetBattleFieldEventEnable() const
{
	return bBattleFieldIsEventOpen;
}

void CPythonPlayer::SetBattleFieldEventInfo(bool bIsEventEnable, int iStartTime, int iEndTime)
{
	bBattleFieldIsEventEnable = bIsEventEnable;
	iBattleFieldStartTime = iStartTime;
	iBattleFieldEndTime = iEndTime;
}
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
bool CPythonPlayer::GetPickedActorID(uint32_t* pdwActorID)
{
	return __GetPickedActorID(pdwActorID);
}

bool CPythonPlayer::GetPickedItemID(uint32_t* pdwItemID)
{
	return __GetPickedItemID(pdwItemID);
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
bool CPythonPlayer::IsPoly()
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pMainInstance)
		return false;

	return pMainInstance->IsPoly();
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
#include "../EterPythonLib/PythonSlotWindow.h"

void CPythonPlayer::SetItemTransmutationVnum(TItemPos& Cell, const uint32_t blVnum)
{
	if (!Cell.IsValidCell())
		return;

	const TItemData* item = GetItemData(Cell);
	if (item == nullptr)
		return;

	const_cast<TItemData*>(item)->dwTransmutationVnum = blVnum;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

uint32_t CPythonPlayer::GetItemTransmutationVnum(TItemPos Cell) const
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData* pItem = GetItemData(Cell);
	return pItem ? pItem->dwTransmutationVnum : 0;
}

void CPythonPlayer::ClearChangeLook(const bool bClearEffect)
{
	for (uint8_t i = 0; i < static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX); i++)
		DelChangeLookItemData(i, bClearEffect);

	DelChangeLookFreeItemData(bClearEffect);
}

void CPythonPlayer::SetChangeLookWindowOpen(const bool blOpen)
{
	bIsChangeLookWindowOpen = blOpen;
	CPythonPlayer::Instance().ClearChangeLook(true);
}

bool CPythonPlayer::GetChangeLookWindowOpen() const
{
	return bIsChangeLookWindowOpen;
}

void CPythonPlayer::SetChangeLookWindowType(const bool blType)
{
	bChangeLookWindowType = blType;
}

bool CPythonPlayer::GetChangeLookWindowType() const
{
	return bChangeLookWindowType;
}

void CPythonPlayer::SetChangeLookItemData(TSubPacketGCChangeLookItemSet& data)
{
	if (data.bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return;

	m_ChangeLookSlot[data.bSlotType] = data;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSlot", Py_BuildValue("(ii)", data.wCell, UI::ESlotHighLight::HILIGHTSLOT_CHANGE_LOOK));
#endif
}

TSubPacketGCChangeLookItemSet* CPythonPlayer::GetChangeLookItemData(const uint8_t bSlotType)
{
	if (bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return nullptr;

	return &m_ChangeLookSlot[bSlotType];
}

void CPythonPlayer::DelChangeLookItemData(const uint8_t bSlotType, const bool bClearEffect)
{
	if (bSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return;

	TSubPacketGCChangeLookItemSet& rInstance = m_ChangeLookSlot[bSlotType];

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	if (bClearEffect && rInstance.wCell < c_Inventory_Slot_Count)
		PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSlot", Py_BuildValue("(ii)", rInstance.wCell, UI::ESlotHighLight::HILIGHTSLOT_CHANGE_LOOK));
#endif

	ZeroMemory(&rInstance, sizeof(rInstance));
	rInstance.wCell = c_Inventory_Slot_Count;
}

void CPythonPlayer::SetChangeLookFreeItemData(TSubPacketGCChangeLookItemSet& data)
{
	m_ChangeLookFreeYangItemSlot = data;
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSlot", Py_BuildValue("(ii)", data.wCell, UI::ESlotHighLight::HILIGHTSLOT_CHANGE_LOOK));
#endif
}

TSubPacketGCChangeLookItemSet* CPythonPlayer::GetChangeLookFreeItemData()
{
	return &m_ChangeLookFreeYangItemSlot;
}

void CPythonPlayer::DelChangeLookFreeItemData(const bool bClearEffect)
{
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	if (bClearEffect && m_ChangeLookFreeYangItemSlot.wCell < c_Inventory_Slot_Count)
		PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSlot", Py_BuildValue("(ii)", m_ChangeLookFreeYangItemSlot.wCell, UI::ESlotHighLight::HILIGHTSLOT_CHANGE_LOOK));
#endif

	ZeroMemory(&m_ChangeLookFreeYangItemSlot, sizeof(m_ChangeLookFreeYangItemSlot));
	m_ChangeLookFreeYangItemSlot.wCell = c_Inventory_Slot_Count;
}
#endif

#if defined(ENABLE_AUTO_SYSTEM) || defined(ENABLE_FLOWER_EVENT)
void CPythonPlayer::AddAffect(uint32_t dwType, TPacketAffectElement kElem)
{
	const int iAffIndex = GetAffectDataIndex(dwType, kElem.wPointIdxApplyOn);
	if (iAffIndex != -1)
		m_mapAffectData.at(iAffIndex) = kElem;
	else
		m_mapAffectData.emplace_back(kElem);
}

void CPythonPlayer::RemoveAffect(uint32_t dwType, uint16_t wApplyOn)
{
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it)
	{
		const TPacketAffectElement elem = *it;
		if (elem.dwType == dwType && (wApplyOn == 0 || wApplyOn == elem.wPointIdxApplyOn))
		{
			m_mapAffectData.erase(it);
			break;
		}
	}
}

int CPythonPlayer::GetAffectDataIndex(uint32_t dwType, uint16_t wApplyOn)
{
	int ret = -1, i = 0;
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it, ++i)
	{
		const TPacketAffectElement elem = *it;
		if (elem.dwType == dwType && (wApplyOn == 0 || wApplyOn == elem.wPointIdxApplyOn))
		{
			ret = i;
			break;
		}
	}

	return ret;
}

TPacketAffectElement CPythonPlayer::GetAffectData(uint32_t dwType, uint16_t wApplyOn)
{
	TPacketAffectElement ret;
	memset(&ret, 0, sizeof(TPacketAffectElement));
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it)
	{
		const TPacketAffectElement elem = *it;
		if (elem.dwType == dwType && (wApplyOn == 0 || wApplyOn == elem.wPointIdxApplyOn))
		{
			ret = elem;
			break;
		}
	}

	return ret;
}

int CPythonPlayer::GetAffectDuration(uint32_t dwType)
{
	int ret = 0, i = 0;
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it, ++i)
	{
		if (it->dwType == dwType)
		{
			ret = it->lDuration;
			break;
		}
	}

	return ret;
}
#endif

#ifdef ENABLE_PENDANT
uint32_t CPythonPlayer::GetElementByVID(uint32_t dwVID)
{
	CInstanceBase* pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
	if (!pTargetInstance)
		return 0;

	//const uint32_t dwPendantVnum = pTargetInstance->GetPendant();
	const uint32_t dwPendantVnum = pTargetInstance->GetPart(CRaceData::PART_PENDANT);
	if (!dwPendantVnum)
		return 0;

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwPendantVnum, &pItemData))
		return 0;

	CItemData::TItemApply ItemApply;
	if (!pItemData->GetApply(0, &ItemApply))
		return 0;

	switch (ItemApply.wType)
	{
		case CItemData::APPLY_ENCHANT_ELECT:
			return CPythonNonPlayer::MOB_ELEMENTAL_ELEC;
			break;
		case CItemData::APPLY_ENCHANT_FIRE:
			return CPythonNonPlayer::MOB_ELEMENTAL_FIRE;
			break;
		case CItemData::APPLY_ENCHANT_ICE:
			return CPythonNonPlayer::MOB_ELEMENTAL_ICE;
			break;
		case CItemData::APPLY_ENCHANT_WIND:
			return CPythonNonPlayer::MOB_ELEMENTAL_WIND;
			break;
		case CItemData::APPLY_ENCHANT_EARTH:
			return CPythonNonPlayer::MOB_ELEMENTAL_EARTH;
			break;
		case CItemData::APPLY_ENCHANT_DARK:
			return CPythonNonPlayer::MOB_ELEMENTAL_DARK;
			break;
	}

	return 0;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
void CPythonPlayer::__ClearAuraRefineWindow()
{
	for (uint8_t i = 0; i < AURA_SLOT_MAX; ++i)
	{
		if (!m_AuraRefineActivatedCell[i].IsNPOS())
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSlot", Py_BuildValue("(ii)", m_AuraRefineActivatedCell[i].cell, UI::HILIGHTSLOT_AURA));
			m_AuraRefineActivatedCell[i] = NPOS;
		}
	}
}

void CPythonPlayer::SetAuraRefineWindowOpen(uint8_t wndType)
{
	m_bOpenedAuraWindowType = wndType;
	m_bAuraWindowOpen = AURA_WINDOW_TYPE_MAX != wndType;
	if (!m_bAuraWindowOpen)
		__ClearAuraRefineWindow();
}

bool CPythonPlayer::IsAuraRefineWindowEmpty()
{
	for (size_t i = 0; i < m_AuraItemInstanceVector.size(); ++i)
	{
		if (m_AuraItemInstanceVector[i].vnum)
			return false;
	}

	return true;
}

void CPythonPlayer::SetAuraRefineInfo(uint8_t bAuraRefineInfoSlot, uint8_t bAuraRefineInfoLevel, uint8_t bAuraRefineInfoExpPercent)
{
	if (bAuraRefineInfoSlot >= AURA_REFINE_INFO_SLOT_MAX)
		return;

	m_bAuraRefineInfo[bAuraRefineInfoSlot].bAuraRefineInfoLevel = bAuraRefineInfoLevel;
	m_bAuraRefineInfo[bAuraRefineInfoSlot].bAuraRefineInfoExpPercent = bAuraRefineInfoExpPercent;
}

uint8_t CPythonPlayer::GetAuraRefineInfoLevel(uint8_t bAuraRefineInfoSlot)
{
	if (bAuraRefineInfoSlot >= AURA_REFINE_INFO_SLOT_MAX)
		return 0;

	return m_bAuraRefineInfo[bAuraRefineInfoSlot].bAuraRefineInfoLevel;
}

uint8_t CPythonPlayer::GetAuraRefineInfoExpPct(uint8_t bAuraRefineInfoSlot)
{
	if (bAuraRefineInfoSlot >= AURA_REFINE_INFO_SLOT_MAX)
		return 0;

	return m_bAuraRefineInfo[bAuraRefineInfoSlot].bAuraRefineInfoExpPercent;
}

void CPythonPlayer::SetAuraItemData(uint8_t bySlotIndex, const TItemData& rItemInstance)
{
	if (bySlotIndex >= m_AuraItemInstanceVector.size())
	{
		TraceError("CPythonPlayer::SetAuraItemData(bySlotIndex=%u) - Strange slot index", bySlotIndex);
		return;
	}

	m_AuraItemInstanceVector[bySlotIndex] = rItemInstance;

	if (bySlotIndex != AURA_SLOT_RESULT)
		PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSlot", Py_BuildValue("(ii)", m_AuraRefineActivatedCell[bySlotIndex].cell, UI::HILIGHTSLOT_AURA));
}

void CPythonPlayer::DelAuraItemData(uint8_t bySlotIndex)
{
	if (bySlotIndex >= AURA_SLOT_MAX || bySlotIndex >= (uint8_t)m_AuraItemInstanceVector.size())
	{
		TraceError("CPythonPlayer::DelAuraItemData(bySlotIndex=%u) - Strange slot index", bySlotIndex);
		return;
	}

	TItemData& rInstance = m_AuraItemInstanceVector[bySlotIndex];
	CItemData* pItemData;
	if (CItemManager::Instance().GetItemDataPointer(rInstance.vnum, &pItemData))
	{
		if (bySlotIndex == AURA_SLOT_MAIN || bySlotIndex == AURA_SLOT_SUB)
			DelAuraItemData(AURA_SLOT_RESULT);

		if (bySlotIndex != AURA_SLOT_RESULT)
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSlot", Py_BuildValue("(ii)", m_AuraRefineActivatedCell[bySlotIndex].cell, UI::HILIGHTSLOT_AURA));
			m_AuraRefineActivatedCell[bySlotIndex] = NPOS;
		}

		ZeroMemory(&rInstance, sizeof(TItemData));
	}
}

uint8_t CPythonPlayer::FineMoveAuraItemSlot()
{
	for (size_t i = 0; i < m_AuraItemInstanceVector.size(); ++i)
	{
		if (!m_AuraItemInstanceVector[i].vnum)
			return (uint8_t)i;
	}

	return AURA_SLOT_MAX;
}

uint8_t CPythonPlayer::GetAuraCurrentItemSlotCount()
{
	uint8_t bCurCount = 0;
	for (uint8_t i = 0; i < AURA_SLOT_MAX; ++i)
	{
		if (m_AuraItemInstanceVector[i].vnum)
			++bCurCount;
	}

	return bCurCount;
}

BOOL CPythonPlayer::GetAuraItemDataPtr(uint8_t bySlotIndex, TItemData** ppInstance)
{
	if (bySlotIndex >= m_AuraItemInstanceVector.size())
	{
		TraceError("CPythonPlayer::GetAuraItemDataPtr(bySlotIndex=%u) - Strange slot index", bySlotIndex);
		return FALSE;
	}

	*ppInstance = &m_AuraItemInstanceVector[bySlotIndex];

	return TRUE;
}

void CPythonPlayer::SetActivatedAuraSlot(uint8_t bySlotIndex, TItemPos ItemCell)
{
	if (bySlotIndex >= AURA_SLOT_MAX)
		return;

	m_AuraRefineActivatedCell[bySlotIndex] = ItemCell;
}

uint8_t CPythonPlayer::FindActivatedAuraSlot(TItemPos ItemCell)
{
	for (uint8_t i = AURA_SLOT_MAIN; i < AURA_SLOT_MAX; ++i)
	{
		if (m_AuraRefineActivatedCell[i] == ItemCell)
			return uint8_t(i);
	}

	return AURA_SLOT_MAX;
}

TItemPos CPythonPlayer::FindUsingAuraSlot(uint8_t bySlotIndex)
{
	if (bySlotIndex >= AURA_SLOT_MAX)
		return NPOS;

	return m_AuraRefineActivatedCell[bySlotIndex];
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
void CPythonPlayer::SetSungmaAttribute(TItemPos Cell, uint32_t dwSungmaAttrIndex, uint16_t wType, int16_t sValue)	//@fixme436
{
	if (!Cell.IsValidCell())
		return;
	if (dwSungmaAttrIndex >= APPLY_RANDOM_SLOT_MAX_NUM)
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->aApplyRandom[dwSungmaAttrIndex].wType = wType;
	(const_cast <TItemData*>(GetItemData(Cell)))->aApplyRandom[dwSungmaAttrIndex].sValue = sValue;
}

void CPythonPlayer::GetItemApplyRandom(TItemPos Cell, uint32_t dwSungmaAttrIndex, uint16_t* pwType, int16_t* psValue)	//@fixme436
{
	*pwType = 0;
	*psValue = 0;

	if (!Cell.IsValidCell())
		return;

	if (dwSungmaAttrIndex >= APPLY_RANDOM_SLOT_MAX_NUM)
		return;

	*pwType = GetItemData(Cell)->aApplyRandom[dwSungmaAttrIndex].wType;
	*psValue = GetItemData(Cell)->aApplyRandom[dwSungmaAttrIndex].sValue;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CPythonPlayer::SetPetInfo(TGrowthPetInfo pet_info)
{
	const auto it = m_GrowthPetInfo.find(pet_info.pet_id);
	if (it != m_GrowthPetInfo.end())
	{
		it->second = pet_info;
		return;
	}

	m_GrowthPetInfo.emplace(pet_info.pet_id, pet_info);
}

TGrowthPetInfo CPythonPlayer::GetPetInfo(uint32_t pet_id)
{
	TGrowthPetInfo petInfo = {};
	const auto it = m_GrowthPetInfo.find(pet_id);
	if (it != m_GrowthPetInfo.end())
		petInfo = it->second;

	return petInfo;
}
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
void CPythonPlayer::SetGemShopItemData(uint8_t slotIndex, const TGemShopItem& rItemInstance)
{
	m_GemItemsMap.emplace(slotIndex, rItemInstance);
}

bool CPythonPlayer::GetGemShopItemData(uint8_t slotIndex, const TGemShopItem** ppGemItemInfo)
{
	if (!ppGemItemInfo)
		return false;

	const auto it = m_GemItemsMap.find(slotIndex);
	if (m_GemItemsMap.end() == it)
		return false;

	*ppGemItemInfo = &(it->second);

	return true;
}

void CPythonPlayer::SetOpenGemShopWindow(bool isOpen)
{
	m_isOpenGemShopWindow = isOpen;
	if (isOpen)
		PyCallClassMemberFunc(m_ppyGameWindow, "RefreshGemShopWIndow", Py_BuildValue("()"));
}

bool CPythonPlayer::IsOpenGemShopWindow()
{
	return m_isOpenGemShopWindow;
}

void CPythonPlayer::SetGemShopSlotAdd(uint8_t slotIndex, uint8_t status)
{
	const auto it = m_GemItemsMap.find(slotIndex);
	if (m_GemItemsMap.end() == it)
		return;

	it->second.status = status;
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
void CPythonPlayer::ShopSearchChangePage(int iPage)
{
	iShopSearchPage = iPage;
}

void CPythonPlayer::SetShopSearchItemData(uint32_t real_position, TSearchItemData rItemData)
{
	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(rItemData.pItem.vnum, &pItemData))
		return;

	memcpy(rItemData.item_name, pItemData->GetName(), sizeof(rItemData.item_name));

	m_ItemInstanceVector.emplace(real_position, rItemData);
}

void CPythonPlayer::ClearShopSearchData()
{
	m_ItemInstanceVector.clear();
}

TSearchItemData CPythonPlayer::GetShopSearchItemData(uint32_t index)
{
	const uint32_t page = (10 * GetShopSearchPage()) - 10;
	TSearchItemData pSearchItemData = {};
	const auto it = m_ItemInstanceVector.find(index + page);
	if (it != m_ItemInstanceVector.end())
		pSearchItemData = it->second;

	return pSearchItemData;
}
#endif

#ifdef ENABLE_CHANGED_ATTR
void CPythonPlayer::GetItemChangedAttribute(uint32_t dwAttrSlotIndex, uint16_t* pbyType, int16_t* psValue)	//@fixme436
{
	*pbyType = 0;
	*psValue = 0;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	*pbyType = m_SelectAttrArr[dwAttrSlotIndex].wType;
	*psValue = m_SelectAttrArr[dwAttrSlotIndex].sValue;
}

void CPythonPlayer::SetSelectAttr(const TPlayerItemAttribute* attr)
{
	memcpy(m_SelectAttrArr, attr, sizeof(m_SelectAttrArr));
}
#endif

#ifdef ENABLE_TAB_TARGETING
void CPythonPlayer::SelectNearTarget()
{
	const auto rkPlayer = CPythonPlayer::InstancePtr();
	if (rkPlayer)
	{
		const auto pkInstTarget = CPythonCharacterManager::Instance().GetTabNextTargetPointer(rkPlayer->NEW_GetMainActorPtr());
		if (pkInstTarget)
			rkPlayer->SetTarget(pkInstTarget->GetVirtualID(), true);
	}
}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
void CPythonPlayer::SetBasicItem(TItemPos Cell, bool is_basic)
{
	const TItemData* pData = GetItemData(Cell);
	if (!pData) {
		TraceError("FAILED CPythonPlayer::SetBasicItem() Cell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return;
	}

	(const_cast <TItemData *>(pData))->is_basic = is_basic;
}

bool CPythonPlayer::IsBasicItem(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return false;

	const TItemData * pItem = GetItemData(Cell);
	if (pItem == nullptr)
		return false;
	else
		return pItem->is_basic;
}
#endif

#ifdef ENABLE_SET_ITEM
void CPythonPlayer::SetItemSetValue(TItemPos Cell, uint8_t set_value)
{
	if (!Cell.IsValidCell())
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->set_value = set_value;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

uint8_t CPythonPlayer::GetItemSetValue(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData* pItem = GetItemData(Cell);
	if (pItem == NULL)
		return 0;

	return pItem->set_value;
}
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
int CPythonPlayer::GetSpecialInventoryTypeByGlobalSlot(int iSlot)
{
	TItemPos Cell(INVENTORY, iSlot);
	if (!Cell.IsValidCell())
	{
		return -1;
	}

	if (iSlot >= 0 && iSlot < 180)
	{
		return INVENTORY_TYPE_INVENTORY;
	}
	else if (iSlot >= c_Special_Inventory_Skillbook_Slot_Start && iSlot < c_Special_Inventory_Skillbook_Slot_End)
	{
		return INVENTORY_TYPE_SKILLBOOK;
	}
	else if (iSlot >= c_Special_Inventory_Stone_Slot_Start && iSlot < c_Special_Inventory_Stone_Slot_End)
	{
		return INVENTORY_TYPE_STONE;
	}
	else if (iSlot >= c_Special_Inventory_Material_Slot_Start && iSlot < c_Special_Inventory_Material_Slot_End)
	{
		return INVENTORY_TYPE_MATERIAL;
	}

	return -1;
}

std::tuple<int, int> CPythonPlayer::GetSpecialInventoryRange(uint8_t invenType)
{
	switch (invenType)
	{
		case INVENTORY_TYPE_INVENTORY:
			return std::tuple<int, int>(0, 180);
		case INVENTORY_TYPE_SKILLBOOK:
			return std::tuple<int, int>(c_Special_Inventory_Skillbook_Slot_Start, c_Special_Inventory_Skillbook_Slot_End);
		case INVENTORY_TYPE_STONE:
			return std::tuple<int, int>(c_Special_Inventory_Stone_Slot_Start, c_Special_Inventory_Stone_Slot_End);
		case INVENTORY_TYPE_MATERIAL:
			return std::tuple<int, int>(c_Special_Inventory_Material_Slot_Start, c_Special_Inventory_Material_Slot_End);

	default:
		return std::tuple<int, int>(-1, -1);
	}
}
#endif

CPythonPlayer::CPythonPlayer()
{
	SetMovableGroundDistance(40.0f);

	// AffectIndex To SkillIndex
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_JEONGWI, 3);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_GEOMGYEONG, 4);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_CHEONGEUN, 19);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_GYEONGGONG, 49);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_EUNHYEONG, 34);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_GONGPO, 64);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_JUMAGAP, 65);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_HOSIN, 94);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_BOHO, 95);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_KWAESOK, 110);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_GICHEON, 96);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_JEUNGRYEOK, 111);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_PABEOP, 66);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_FALLEN_CHEONGEUN, 19);
	/////
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_GWIGEOM, 63);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_MUYEONG, 78);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_HEUKSIN, 79);

#ifdef ENABLE_WOLFMAN_CHARACTER
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_RED_POSSESSION, 174);
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_BLUE_POSSESSION, 175);
#endif

#ifdef ENABLE_NINETH_SKILL
	m_kMap_dwAffectIndexToSkillIndex.emplace(CInstanceBase::AFFECT_CHEONUN, 182);
#endif

	m_ppyGameWindow = nullptr;

	m_sysIsCoolTime = TRUE;
	m_sysIsLevelLimit = TRUE;
	m_dwPlayTime = 0;

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	ClearChangeLook();
	bIsChangeLookWindowOpen = false;
	bChangeLookWindowType = static_cast<decltype(bChangeLookWindowType)>(ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_ITEM);
#endif

#if defined(ENABLE_KEYCHANGE_SYSTEM) && defined(ENABLE_TAB_TARGETING)
	m_bWarState = false;
#endif

	m_aeMBFButton[MBT_LEFT] = MBF_SMART;
	m_aeMBFButton[MBT_RIGHT] = MBF_CAMERA;
	m_aeMBFButton[MBT_MIDDLE] = MBF_CAMERA;

	msl::refill(m_adwEffect);

	m_isDestPosition = FALSE;
	m_ixDestPos = 0;
	m_iyDestPos = 0;
	m_iLastAlarmTime = 0;
#if defined(ENABLE_KEYCHANGE_SYSTEM)
	m_bAutoRun = false;
#endif

	Clear();
}

CPythonPlayer::~CPythonPlayer(void) = default;

#ifdef ENABLE_AUTO_SYSTEM
void CPythonPlayer::UpdateAuto()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
	{
		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		return;
	}

	if (pkInstMain->IsMountingHorse())
	{
		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		return;
	}

	if (GetAutoPause())
	{
		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		return;
	}

	if (!CanStartAuto())
	{
		AutoStartOnOff(false);
		CPythonChat::Instance().AppendChat(1, "Not used premium item");
	}

	bool isPass = false;

	if (findTargetMs != 0 && ((findTargetMs + AUTO_MAX_KILL_SECOND) <= CTimer::Instance().GetCurrentSecond()))
	{
		isPass = true;
		findTargetMs = 0;
	}

	if (GetAutoSkillOnOff())
	{
		AutoHuntTimer(true);
	}

	if (GetAutoPositionOnOff())
	{
		AutoHuntTimer(false);
		AutoReserveUse();
	}

	// if ((m_dwAutoAttackTargetVID == 0 && GetAutoAttackOnOff()) || isPass) //@bizeps001
	// {
		// CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
		// if (!pkInstMain) return;

		// SetTarget(pkInstMain->FindNearestVictimVID(pkInstMain->GetVirtualID()), true);
		// m_dwAutoAttackTargetVID = pkInstMain->FindNearestVictimVID(pkInstMain->GetVirtualID());
	// }
	
	if ((m_dwAutoAttackTargetVID == 0 && GetAutoAttackOnOff()) || isPass)
	{
		CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
		if (!pkInstMain) return;
		CInstanceBase* pkInstTarget = CPythonCharacterManager::Instance().AutoHuntingGetMob(pkInstMain, pkInstMain->FindNearestVictimVID(pkInstMain->GetVirtualID()), isPass);
		if (pkInstTarget)
		{
			SetTarget(pkInstMain->FindNearestVictimVID(pkInstMain->GetVirtualID()));
			m_dwAutoAttackTargetVID = pkInstMain->FindNearestVictimVID(pkInstMain->GetVirtualID());
		}
	}
}

void CPythonPlayer::AutoStartOnOff(bool gelenDurum)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;

	findTargetMs = 0;
	autoStatus = gelenDurum;

	if (!gelenDurum)
	{
		CPythonNetworkStream::Instance().SendChatPacket("/autohunt d");
		__ClearReservedAction();
		__ClearAutoAttackTargetActorID();
	}
	else
	{
		CPythonNetworkStream::Instance().SendChatPacket("/autohunt b");
		autohuntStartLocation = TPixelPosition(0, 0, 0);
		pkInstMain->NEW_GetPixelPosition(&autohuntStartLocation);
	}

	auto_HP = false;
	autp_MP = false;
}

void CPythonPlayer::AutoHuntTimer(bool beceriSlot)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	uint32_t suanTime = CTimer::Instance().GetCurrentSecond();
	int maxMesafe = 3000;

	if (beceriSlot)
	{
		if (pkInstMain->IsUsingSkill())
			return;

		for (uint8_t i = 0; i < AUTO_SKILL_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos && m_playerStatus.aAutoSlot[i].fillingTime)
			{
				uint32_t sPos = m_playerStatus.aAutoSlot[i].slotPos;
				uint32_t sDS = m_playerStatus.aAutoSlot[i].fillingTime;
				uint32_t sSK = m_playerStatus.aAutoSlot[i].nextUsage;

				if (sSK <= suanTime)
				{
					CPythonSkill::SSkillData* c_pSkillData;
					if (!CPythonSkill::Instance().GetSkillData(GetSkillIndex(sPos), &c_pSkillData))
					{
						TraceError("otoAv - Failed to find skill by %d", sPos);
						return;
					}

					if (c_pSkillData->IsAttackSkill() && zLastSec != suanTime)
					{
						if (m_dwAutoAttackTargetVID != 0 && __GetAliveTargetInstancePtr() && !IsSkillCoolTime(sPos))
						{	//Eger %1000000 emin degilsen burayi elleme
							CInstanceBase* pInstance = __GetAliveTargetInstancePtr();
							if (!pInstance)
							{
								TraceError("AutoHuntTimer - hedef yok!");
								return;
							}

							maxMesafe = c_pSkillData->IsNeedBow() ? 2500 : 250;

							if ((int)pkInstMain->GetDistance(pInstance) <= maxMesafe && !pInstance->IsDead() && !pInstance->IsStun())
							{
								if (!GetTargetVID() || !m_dwAutoAttackTargetVID)
									return;

								UseAutoSkill(sPos, suanTime, i);
							}
						}
					}
					else
					{
						if (!IsSkillCoolTime(sPos))
						{
							if (IsToggleSkill(sPos) && IsSkillActive(sPos))
								continue;

							UseAutoSkill(sPos, suanTime, i);
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = (int)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos && m_playerStatus.aAutoSlot[i].fillingTime)
			{
				uint32_t iPos = m_playerStatus.aAutoSlot[i].slotPos;
				uint32_t iDS = m_playerStatus.aAutoSlot[i].fillingTime;
				uint32_t iSK = m_playerStatus.aAutoSlot[i].nextUsage;
				int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));

				CItemManager::Instance().SelectItemData(iVnum);
				CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
				if (!pItem)
				{
					TraceError("Not Used 1", iVnum);
					//TraceError("Otomatik av item bilgisi alinamadi. itemVnum=%d", iVnum);
					continue;
				}

				bool kirmiziMi = false, maviMi = false;
				int iType = pItem->GetType(), iSubType = pItem->GetSubType();
				if (iType == pItem->ITEM_TYPE_USE)
				{
					TItemPos itemPos;
					itemPos.cell = iPos;	//#19.01.2019 06:02 - Test et.

					if (iSubType == pItem->USE_POTION)
					{
						for (int k = 0; k < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); k++)
						{
							if (iVnum == auto_red_potions[k])
							{
								kirmiziMi = true;
								break;
							}
						}

						for (int m = 0; m < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); m++)
						{
							if (iVnum == auto_blue_potions[m])
							{
								maviMi = true;
								break;
							}
						}

						if (kirmiziMi)
						{
							uint8_t hpYuzde = MINMAX(0, GetStatus(POINT_HP) * 100 / GetStatus(POINT_MAX_HP), 100);
							if (hpYuzde <= iDS)
								auto_HP = true;
						}

						if (maviMi)
						{
							uint8_t spYuzde = MINMAX(0, GetStatus(POINT_SP) * 100 / GetStatus(POINT_MAX_SP), 100);
							if (spYuzde <= iDS)
								autp_MP = true;
						}
					}
					else if (iSubType == pItem->USE_ABILITY_UP || iSubType == pItem->USE_AFFECT)
					{
						if (iSK <= suanTime)
						{
							CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
							m_playerStatus.aAutoSlot[i].nextUsage = m_playerStatus.aAutoSlot[i].fillingTime + suanTime;
						}
					}
				}
				else if (iType == pItem->ITEM_TYPE_BLEND)
				{
					TItemPos itemPos;
					itemPos.cell = iPos;
					if (iSK <= suanTime)
					{
						CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
						m_playerStatus.aAutoSlot[i].nextUsage = m_playerStatus.aAutoSlot[i].fillingTime + suanTime;
					}
				}
			}
		}
	}
}

void CPythonPlayer::UseAutoSkill(uint32_t dwSlotIndex, long iTime, int slotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	TSkillInstance& rkSkillInst = m_playerStatus.aSkill[dwSlotIndex];
	CPythonSkill::TSkillData* pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return;

	if (CPythonSkill::SKILL_TYPE_GUILD == pSkillData->byType)
	{
		UseGuildSkill(dwSlotIndex);
		return;
	}

	if (!pSkillData->IsCanUseSkill())
		return;

	if (IsSkillCoolTime(dwSlotIndex))
		return;

	if (pSkillData->IsStandingSkill())
	{
		if (pSkillData->IsToggleSkill())
		{
			if (!IsSkillActive(dwSlotIndex))
			{
				CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
				if (!pkInstMain)
					return;

				if (pkInstMain->IsUsingSkill())
					return;

				//CPythonNetworkStream::Instance().SendUseSkillPacket(rkSkillInst.dwIndex);
				if (__UseSkill(dwSlotIndex))
				{
					m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
					zLastSec = iTime;
				}
			}
		}
		else
		{
			if (__UseSkill(dwSlotIndex))
			{
				m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
				zLastSec = iTime;
			}
		}
	}
	else if (m_dwcurSkillSlotIndex == dwSlotIndex)
	{
		if (__UseSkill(m_dwcurSkillSlotIndex))
		{
			m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
			zLastSec = iTime;
		}
	}

	if (!__IsRightButtonSkillMode())
	{
		if (__UseSkill(dwSlotIndex))
		{
			m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
			zLastSec = iTime;
		}
	}
	else
	{
		m_dwcurSkillSlotIndex = dwSlotIndex;
		PyCallClassMemberFunc(m_ppyGameWindow, "ChangeCurrentSkill", Py_BuildValue("(i)", dwSlotIndex));
	}
}

void CPythonPlayer::AutoReserveUse()
{
	if (auto_HP)
	{
		TItemPos itemPos;
		uint8_t otoAvSlotIndex;
		bool pVar = false;

		for (uint8_t i = (uint8_t)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos)
			{
				uint32_t iPos = m_playerStatus.aAutoSlot[i].slotPos;
				int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));

				CItemManager::Instance().SelectItemData(iVnum);
				CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
				if (pItem)
				{
					for (int x = 0; x < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); x++)
					{
						if (iVnum == auto_red_potions[x])
						{
							pVar = true;
							itemPos.cell = iPos;
							otoAvSlotIndex = i;
							break;
						}
					}
				}
			}
		}

		if (pVar)
		{
			if (GetStatus(POINT_HP) + GetStatus(POINT_HP_RECOVERY) < GetStatus(POINT_MAX_HP))
			{
				if (kpLastMs <= CTimer::Instance().GetCurrentMillisecond())
				{
					CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
					kpLastMs = CTimer::Instance().GetCurrentMillisecond() + 400;
					AutoSlotControl(otoAvSlotIndex);
				}
			}
			else
				auto_HP = false;
		}
	}

	if (autp_MP)
	{
		TItemPos itemPos;
		uint8_t autoSlotIndex;
		bool pVar = false;

		for (uint8_t i = (uint8_t)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos)
			{
				uint32_t iPos = m_playerStatus.aAutoSlot[i].slotPos;
				int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));

				CItemManager::Instance().SelectItemData(iVnum);
				CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
				if (pItem)
				{
					for (int x = 0; x < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); x++)
					{
						if (iVnum == auto_blue_potions[x])
						{
							pVar = true;
							itemPos.cell = iPos;
							autoSlotIndex = i;
							break;
						}
					}
				}
			}
		}

		if (pVar)
		{
			if (GetStatus(POINT_SP) + GetStatus(POINT_SP_RECOVERY) < GetStatus(POINT_MAX_SP))
			{
				if (mpLastMs <= CTimer::Instance().GetCurrentMillisecond())
				{
					CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
					mpLastMs = CTimer::Instance().GetCurrentMillisecond() + 400;
					AutoSlotControl(autoSlotIndex);
				}
			}
			else
				autp_MP = false;
		}
	}
}

bool CPythonPlayer::AutoBarrierCheck(CInstanceBase* pTarget, CInstanceBase* pInstance)
{
	return 0;//empty function
}

void CPythonPlayer::AutoSlotControl(uint32_t slotIndex)
{
	if (slotIndex < 0 || slotIndex >= AUTO_POSITINO_SLOT_MAX)
		return;

	if (slotIndex >= AUTO_SKILL_SLOT_MAX)
	{
		uint32_t iPos = m_playerStatus.aAutoSlot[slotIndex].slotPos;
		if (CPythonPlayer::Instance().GetItemCount(TItemPos(INVENTORY, iPos)) < 1)
			ClearAutoPositionSlot(slotIndex, true);

		int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));
		if (!iVnum)
			ClearAutoPositionSlot(slotIndex, true);
	}

	PyCallClassMemberFunc(m_ppyGameWindow, "AutoSlotRefresh", Py_BuildValue("()"));
}

void CPythonPlayer::SetAutoSkillSlotIndex(int iSlotIndex, uint32_t dwIndex)
{
	if (iSlotIndex < 0 || iSlotIndex >= AUTO_SKILL_SLOT_MAX)
		return;

	memset(&m_playerStatus.aAutoSlot[iSlotIndex], 0, sizeof(m_playerStatus.aAutoSlot[iSlotIndex]));

	if (iSlotIndex >= 0 && iSlotIndex < AUTO_SKILL_SLOT_MAX)
	{
		int skillIndex = GetSkillIndex(dwIndex);
		if (skillIndex)
		{
			int skillPuani = GetSkillGrade(dwIndex);
			float skillDerece = GetSkillCurrentEfficientPercentage(dwIndex);
			// TraceError("skillPuani:%d,skillIndex:%d,skillDerece:%d", skillPuani, skillIndex, skillDerece);

			CPythonSkill::SSkillData* c_pSkillData;
			if (!CPythonSkill::Instance().GetSkillData(skillIndex, &c_pSkillData))
				return;

			int skillDS = (int)c_pSkillData->GetSkillCoolTime(skillDerece);
			if (skillDS == 0)
				skillDS = (int)c_pSkillData->GetDuration(skillDerece);

			if (!skillDS)
				return;

			for (uint8_t i = 0; i < AUTO_SKILL_SLOT_MAX; i++)
			{
				if (m_playerStatus.aAutoSlot[i].slotPos == dwIndex)
					ClearAutoSKillSlot();
			}

			m_playerStatus.aAutoSlot[iSlotIndex].slotPos = dwIndex;
			m_playerStatus.aAutoSlot[iSlotIndex].fillingTime = skillDS;
			CPythonNetworkStream::Instance().SendAutoCoolTime(iSlotIndex, skillDS);
		}
	}
}

void CPythonPlayer::SetAutoPositionSlotIndex(int iSlotIndex, uint32_t dwIndex)
{
	if (iSlotIndex < AUTO_SKILL_SLOT_MAX || iSlotIndex >= AUTO_POSITINO_SLOT_MAX)
		return;

	memset(&m_playerStatus.aAutoSlot[iSlotIndex], 0, sizeof(m_playerStatus.aAutoSlot[iSlotIndex]));


	if (iSlotIndex < AUTO_POSITINO_SLOT_MAX)
	{
		if (dwIndex == 0)
		{
			CPythonChat::Instance().AppendChat(1, "Not used");
			return;
		}

		int iVnum = GetItemIndex(TItemPos(INVENTORY, dwIndex));
		CItemManager::Instance().SelectItemData(iVnum);
		CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();

		int iType = pItem->GetType();
		int iSubType = pItem->GetSubType();
		int rtDs = 0;

		if (iType == pItem->ITEM_TYPE_USE)
		{
			if (iSubType == pItem->USE_ABILITY_UP)
				rtDs = pItem->GetValue(1);

			if (iSubType == pItem->USE_AFFECT)
				rtDs = pItem->GetValue(3);

			/*if (iSubType == pItem->USE_POTION && (iVnum == 27003 || iVnum == 27006))
				rtDs = 50;*/

			if (iSubType == pItem->USE_POTION)
			{
				for (int k = 0; k < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); k++)
				{
					if (iVnum == auto_red_potions[k])
					{
						rtDs = 50;
						break;
					}
				}

				for (int m = 0; m < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); m++)
				{
					if (iVnum == auto_blue_potions[m])
					{
						rtDs = 50;
						break;
					}
				}
			}
		}
		else if (iType == pItem->ITEM_TYPE_BLEND)
		{
			rtDs = 600;
		}

		if (rtDs > 0)
		{
			for (auto i = (int)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
			{
				if (m_playerStatus.aAutoSlot[i].slotPos == dwIndex)
				{
					//TraceError("m_playerStatus.aAutoSlot[i].slotPos:%d, %d,%d", m_playerStatus.aAutoSlot[i].slotPos, m_playerStatus.aAutoSlot[i].fillingTime, m_playerStatus.aAutoSlot[i].nextUsage);
					ClearAutoPositionSlot();
				}
			}

			m_playerStatus.aAutoSlot[iSlotIndex].slotPos = dwIndex;
			m_playerStatus.aAutoSlot[iSlotIndex].fillingTime = rtDs;
			CPythonNetworkStream::Instance().SendAutoCoolTime(iSlotIndex, rtDs);
		}
	}
}

void CPythonPlayer::SetAutoSlotCoolTime(int otoAvSlotIndex, uint32_t dSuresi)
{
	if (otoAvSlotIndex < 0 || otoAvSlotIndex >= AUTO_POSITINO_SLOT_MAX)
		return;

	m_playerStatus.aAutoSlot[otoAvSlotIndex].fillingTime = dSuresi;
}

void CPythonPlayer::ClearAutoSKillSlot()
{
	for (int i = 0; i < AUTO_SKILL_SLOT_MAX; ++i)
	{
		m_playerStatus.aAutoSlot[i].slotPos = 0;
		m_playerStatus.aAutoSlot[i].fillingTime = 0;
		m_playerStatus.aAutoSlot[i].nextUsage = 0;
	}
}

void CPythonPlayer::ClearAutoPositionSlot(int autoSlotIndex, bool manual)
{
	if (manual)
	{
		m_playerStatus.aAutoSlot[autoSlotIndex].slotPos = 0;
		m_playerStatus.aAutoSlot[autoSlotIndex].fillingTime = 0;
		m_playerStatus.aAutoSlot[autoSlotIndex].nextUsage = 0;
	}
	else
	{
		for (int i = AUTO_SKILL_SLOT_MAX; i < autoSlotIndex; ++i)
		{
			m_playerStatus.aAutoSlot[i].slotPos = 0;
			m_playerStatus.aAutoSlot[i].fillingTime = 0;
			m_playerStatus.aAutoSlot[i].nextUsage = 0;
		}
	}
}

void CPythonPlayer::ClearAutoAllSlot()
{
	ClearAutoSKillSlot();
	ClearAutoPositionSlot();
}

TAutoSlot& CPythonPlayer::AutoSlotData(int iSlotIndex)
{
	if (iSlotIndex < 0 || iSlotIndex >= AUTO_POSITINO_SLOT_MAX)
	{
		static TAutoSlot s_kOtoAvSlot;
		s_kOtoAvSlot.slotPos = 0;
		s_kOtoAvSlot.fillingTime = 0;
		return s_kOtoAvSlot;
	}

	return m_playerStatus.aAutoSlot[iSlotIndex];
}

void CPythonPlayer::GetAutoSlotIndex(uint32_t dwSlotPos, uint32_t* dwVnum, uint32_t* fillingTime)
{
	TAutoSlot& rkOtoAvSlot = AutoSlotData(dwSlotPos);

	if (dwSlotPos >= AUTO_POSITINO_SLOT_MAX)
	{
		*dwVnum = 0;
		return;
	}

	*dwVnum = rkOtoAvSlot.slotPos;
	*fillingTime = rkOtoAvSlot.fillingTime;
}

int CPythonPlayer::CheckSkillSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime)
{
	uint32_t dwVnum, fillingTime;

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.GetAutoSlotIndex(bIndex, &dwVnum, &fillingTime);
	if (dwVnum && dwVnum == iSlotIndex)
	{
		if (bIndex >= 0 && bIndex < AUTO_SKILL_SLOT_MAX)
		{
			int skillIndex = rkPlayer.GetSkillIndex(dwVnum);
			if (skillIndex)
			{
				int skillPuani = rkPlayer.GetSkillGrade(dwVnum);
				float skillDerece = rkPlayer.GetSkillCurrentEfficientPercentage(dwVnum);
				//TraceError("skillPuani:%d,skillIndex:%d,skillDerece:%d", skillPuani, skillIndex, skillDerece);

				CPythonSkill::SSkillData* c_pSkillData;
				if (!CPythonSkill::Instance().GetSkillData(skillIndex, &c_pSkillData))
				{
					TraceError("slotIndex:%d", dwVnum);
					return 0;
				}

				int skillDS = (int)c_pSkillData->GetSkillCoolTime(skillDerece);
				if (skillDS == 0)
				{
					skillDS = (int)c_pSkillData->GetDuration(skillDerece);
				}

				if (!skillDS)
					return 0;

				return iCoolTime < skillDS ? skillDS : iCoolTime;
			}
		}
	}

	return 0;
}

int CPythonPlayer::CheckPositionSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime)
{
	uint32_t dwVnum, fillingTime;

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.GetAutoSlotIndex(bIndex, &dwVnum, &fillingTime);
	if (dwVnum && dwVnum == iSlotIndex)
	{
		if (bIndex >= AUTO_SKILL_SLOT_MAX && bIndex < AUTO_POSITINO_SLOT_MAX)
		{
			int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, dwVnum));
			CItemManager::Instance().SelectItemData(iVnum);
			CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();

			int iType = pItem->GetType(), iSubType = pItem->GetSubType(), rtDs = 0;
			if (iType == pItem->ITEM_TYPE_USE)
			{
				if (iSubType == pItem->USE_ABILITY_UP)
					rtDs = pItem->GetValue(1);
				if (iSubType == pItem->USE_AFFECT)
					rtDs = pItem->GetValue(3);
				if (iSubType == pItem->USE_POTION)
				{
					for (int k = 0; k < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); k++)
					{
						if (iVnum == auto_red_potions[k])
						{
							rtDs = 50;
							break;
						}
					}
					for (int m = 0; m < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); m++)
					{
						if (iVnum == auto_blue_potions[m])
						{
							rtDs = 50;
							break;
						}
					}

				}
			}
			return rtDs < iCoolTime ? iCoolTime : rtDs;
		}
	}

	return 0;
}
#endif
