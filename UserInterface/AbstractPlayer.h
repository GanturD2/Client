#pragma once

#include "AbstractSingleton.h"

class CInstanceBase;

class IAbstractPlayer : public TAbstractSingleton<IAbstractPlayer>
{
public:
	IAbstractPlayer() = default;
	~IAbstractPlayer() = default;

	virtual uint32_t GetMainCharacterIndex() = 0;
	virtual void SetMainCharacterIndex(int iIndex) = 0;
	virtual bool IsMainCharacterIndex(uint32_t dwIndex) = 0;

	virtual int GetStatus(uint32_t dwType) = 0;

	virtual const char * GetName() = 0;

	virtual void SetRace(uint32_t dwRace) = 0;

	virtual void StartStaminaConsume(uint32_t dwConsumePerSec, uint32_t dwCurrentStamina) = 0;
	virtual void StopStaminaConsume(uint32_t dwCurrentStamina) = 0;

	virtual bool IsPartyMemberByVID(uint32_t dwVID) = 0;
	virtual bool PartyMemberVIDToPID(uint32_t dwVID, uint32_t * pdwPID) = 0;
	virtual bool IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2) = 0;

	virtual void SetItemData(TItemPos itemPos, const TItemData & c_rkItemInst) = 0;
	virtual void SetItemCount(TItemPos itemPos, uint8_t byCount) = 0;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	virtual void SetItemTransmutationVnum(TItemPos& Cell, const uint32_t blVnum) = 0;
	virtual uint32_t GetItemTransmutationVnum(TItemPos itemPos) const = 0;
#endif
	virtual void SetItemMetinSocket(TItemPos itemPos, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber) = 0;
	virtual void SetItemAttribute(TItemPos itemPos, uint32_t dwAttrIndex, uint16_t wType, int16_t sValue) = 0;	//@fixme436

#ifdef ENABLE_YOHARA_SYSTEM
	virtual void SetSungmaAttribute(TItemPos Cell, uint32_t dwSungmaAttrIndex, uint16_t wType, int16_t sValue) = 0;
	virtual void SetRandomDefaultAttrs(TItemPos Cell, uint32_t dwRandomValueIndex, uint32_t dwRandomValueNumber) = 0;
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	virtual bool	SealItem(TItemPos Cell, long nSealDate) = 0;
	virtual bool	UnSealItem(TItemPos Cell, long nSealDate) = 0;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	virtual void	SetBasicItem(TItemPos Cell, bool is_basic) = 0;
	virtual bool	IsBasicItem(TItemPos Cell) = 0;
#endif
#ifdef ENABLE_SET_ITEM
	virtual void	SetItemSetValue(TItemPos Cell, uint8_t set_value) = 0;
	virtual uint8_t	GetItemSetValue(TItemPos Cell) = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	virtual void	SetElement(TItemPos itemPos, uint8_t value_grade_element, uint8_t type_element) = 0;
	virtual void 	SetElementAttack(TItemPos itemPos, uint32_t attack_element_index, uint32_t attack_element) = 0;
	virtual void 	SetElementValue(TItemPos itemPos, uint32_t elements_value_bonus_index, int16_t elements_value_bonus) = 0;
#endif

	virtual uint32_t GetItemIndex(TItemPos itemPos) = 0;
	virtual uint32_t GetItemFlags(TItemPos itemPos) = 0;
	virtual uint32_t GetItemCount(TItemPos itemPos) = 0;

	virtual bool IsEquipItemInSlot(TItemPos itemPos) = 0;

	virtual void AddQuickSlot(int QuickslotIndex, char IconType, char IconPosition) = 0;
	virtual void DeleteQuickSlot(int QuickslotIndex) = 0;
	virtual void MoveQuickSlot(int Source, int Target) = 0;

	virtual void SetWeaponPower(uint32_t dwMinPower, uint32_t dwMaxPower, uint32_t dwMinMagicPower, uint32_t dwMaxMagicPower,
								uint32_t dwAddPower) = 0;

	virtual void SetTarget(uint32_t dwVID, BOOL bForceChange = TRUE) = 0;
	virtual void NotifyCharacterUpdate(uint32_t dwVID) = 0;
	virtual void NotifyCharacterDead(uint32_t dwVID) = 0;
	virtual void NotifyDeletingCharacterInstance(uint32_t dwVID) = 0;
	virtual void NotifyChangePKMode() = 0;

	virtual void SetObserverMode(bool isEnable) = 0;
	virtual void SetMobileFlag(BOOL bFlag) = 0;
	virtual void SetComboSkillFlag(BOOL bFlag) = 0;

	virtual void StartEmotionProcess() = 0;
	virtual void EndEmotionProcess() = 0;

	virtual CInstanceBase * NEW_GetMainActorPtr() = 0;
};
