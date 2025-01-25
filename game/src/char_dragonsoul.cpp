#include "stdafx.h"
#include "char.h"
#include "item.h"
#include "desc.h"
#include "DragonSoul.h"
#include "log.h"

void CHARACTER::DragonSoul_Initialize()
{
	for (uint16_t i = WEAR_MAX_NUM; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
	{
		LPITEM pItem = GetItem(TItemPos(EQUIPMENT, i));
		if (nullptr != pItem)
			pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 0);
	}

	if (FindAffect(AFFECT_DRAGON_SOUL_DECK_0))
	{
		DragonSoul_ActivateDeck(DRAGON_SOUL_DECK_0);
	}
	else if (FindAffect(AFFECT_DRAGON_SOUL_DECK_1))
	{
		DragonSoul_ActivateDeck(DRAGON_SOUL_DECK_1);
	}
}

int CHARACTER::DragonSoul_GetActiveDeck() const noexcept
{
	return m_pointsInstant.iDragonSoulActiveDeck;
}

bool CHARACTER::DragonSoul_IsDeckActivated() const noexcept
{
	return m_pointsInstant.iDragonSoulActiveDeck >= 0;
}

bool CHARACTER::DragonSoul_IsQualified() const noexcept
{
#ifndef NON_CHECK_DS_QUALIFIED
	return FindAffect(AFFECT_DRAGON_SOUL_QUALIFIED) != nullptr;
#else
	return true;
#endif
}

void CHARACTER::DragonSoul_GiveQualification()
{
	if(nullptr == FindAffect(AFFECT_DRAGON_SOUL_QUALIFIED))
	{
		LogManager::Instance().CharLog(this, 0, "DS_QUALIFIED", "");
	}
	AddAffect(AFFECT_DRAGON_SOUL_QUALIFIED, APPLY_NONE, 0, AFF_NONE, INFINITE_AFFECT_DURATION, 0, false, false);
	//SetQuestFlag("dragon_soul.is_qualified", 1);
	//// 자격있다면 POINT_DRAGON_SOUL_IS_QUALIFIED는 무조건 1
	//PointChange(POINT_DRAGON_SOUL_IS_QUALIFIED, 1 - GetPoint(POINT_DRAGON_SOUL_IS_QUALIFIED));
}

bool CHARACTER::DragonSoul_ActivateDeck(int deck_idx)
{
	if (deck_idx < DRAGON_SOUL_DECK_0 || deck_idx >= DRAGON_SOUL_DECK_MAX_NUM)
	{
		return false;
	}

	if (DragonSoul_GetActiveDeck() == deck_idx)
		return true;

	DragonSoul_DeactivateAll();

	if (!DragonSoul_IsQualified())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Der Drachenstein ist nicht aktiviert."));
		return false;
	}

	//@fixme434
	AddAffect(AFFECT_DRAGON_SOUL_DECK_0 + deck_idx, APPLY_NONE, 0, AFF_DS, INFINITE_AFFECT_DURATION, 0, false);

	if (deck_idx == DRAGON_SOUL_DECK_0)
		SpecificEffectPacket("d:\\ymir work\\effect\\etc\\dragonsoul\\dragonsoul_earth.mse");
	else
		SpecificEffectPacket("d:\\ymir work\\effect\\etc\\dragonsoul\\dragonsoul_sky.mse");
	//@end_fixme434

	m_pointsInstant.iDragonSoulActiveDeck = deck_idx;

	for (int i = DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * deck_idx;
		i < DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * (deck_idx + 1); i++)
	{
		LPITEM pItem = GetEquipmentItem(i);
		if (nullptr != pItem)
			DSManager::Instance().ActivateDragonSoul(pItem);
	}

#ifdef ENABLE_DS_SET
	DragonSoul_HandleSetBonus();
#endif

	return true;
}

void CHARACTER::DragonSoul_DeactivateAll()
{
	for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
	{
		DSManager::Instance().DeactivateDragonSoul(GetEquipmentItem(i), true);
	}

#ifdef ENABLE_DS_SET
	DragonSoul_HandleSetBonus();
#endif

	m_pointsInstant.iDragonSoulActiveDeck = -1;
	RemoveAffect(AFFECT_DRAGON_SOUL_DECK_0);
	RemoveAffect(AFFECT_DRAGON_SOUL_DECK_1);
#ifdef ENABLE_DS_SET
	RemoveAffect(NEW_AFFECT_DS_SET);
#endif
}

#ifdef ENABLE_DS_SET
void CHARACTER::DragonSoul_HandleSetBonus()
{
	bool bAdd = true;
	uint8_t iSetGrade;;
	if (!DSManager::Instance().GetDSSetGrade(this, iSetGrade))
	{
		CAffect* pkAffect = FindAffect(NEW_AFFECT_DS_SET);
		if (!pkAffect)
		{
			return;
		}

		iSetGrade = pkAffect->lApplyValue;
		bAdd = false;
	}
	else
	{
		AddAffect(NEW_AFFECT_DS_SET, APPLY_NONE, iSetGrade, 0, INFINITE_AFFECT_DURATION, 0, true);
	}

	const uint8_t iDeckIdx = DragonSoul_GetActiveDeck();
	const uint8_t iStartSlotIndex = WEAR_MAX_NUM + (iDeckIdx * DS_SLOT_MAX);
	const uint8_t iEndSlotIndex = iStartSlotIndex + DS_SLOT_MAX;

	for (uint8_t iSlotIndex = iStartSlotIndex; iSlotIndex < iEndSlotIndex; ++iSlotIndex)
	{
		const LPITEM pkItem = GetWear(iSlotIndex);
		if (!pkItem)
		{
			return;
		}

		for (uint8_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (pkItem->GetAttributeType(i))
			{
				const TPlayerItemAttribute& ia = pkItem->GetAttribute(i);
				int iSetValue = DSManager::Instance().GetDSSetValue(i, ia.wType, pkItem->GetVnum(), iSetGrade);

				if (ia.wType == APPLY_SKILL)
				{
					iSetValue = bAdd ? iSetValue : iSetValue ^ 0x00800000;
				}
				else
				{
					iSetValue = bAdd ? iSetValue : -iSetValue;
				}

				ApplyPoint(ia.wType, iSetValue);
			}
		}
	}
}
#endif

void CHARACTER::DragonSoul_CleanUp()
{
	for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_END; i++)
	{
		DSManager::Instance().DeactivateDragonSoul(GetEquipmentItem(i), true);
	}
}

bool CHARACTER::DragonSoul_RefineWindow_Open(LPENTITY pEntity)
{
	if (nullptr == m_pointsInstant.m_pDragonSoulRefineWindowOpener)
	{
		m_pointsInstant.m_pDragonSoulRefineWindowOpener = pEntity;
	}

	TPacketGCDragonSoulRefine PDS;
	PDS.header = HEADER_GC_DRAGON_SOUL_REFINE;
	PDS.bSubType = DS_SUB_HEADER_OPEN;
	LPDESC d = GetDesc();

	if (nullptr == d)
	{
		sys_err ("User(%s)'s DESC is nullptr POINT.", GetName());
		return false;
	}

	d->Packet(&PDS, sizeof(PDS));
	return true;
}

#ifdef ENABLE_DS_CHANGE_ATTR
bool CHARACTER::DragonSoul_ChangeAttrWindow_Open(LPENTITY pEntity)
{
	if (nullptr == m_pointsInstant.m_pDragonSoulRefineWindowOpener)
	{
		m_pointsInstant.m_pDragonSoulRefineWindowOpener = pEntity;
	}

	TPacketGCDragonSoulRefine PDS;
	PDS.header = HEADER_GC_DRAGON_SOUL_REFINE;
	PDS.bSubType = DS_SUB_HEADER_OPEN_CHANGE_ATTR;
	LPDESC d = GetDesc();

	if (nullptr == d)
	{
		sys_err("User(%s)'s DESC is nullptr POINT.", GetName());
		return false;
	}

	d->Packet(&PDS, sizeof(PDS));
	return true;
}
#endif

bool CHARACTER::DragonSoul_RefineWindow_Close() noexcept
{
	m_pointsInstant.m_pDragonSoulRefineWindowOpener = nullptr;
	return true;
}

bool CHARACTER::DragonSoul_RefineWindow_CanRefine() noexcept
{
	return nullptr != m_pointsInstant.m_pDragonSoulRefineWindowOpener;
}
