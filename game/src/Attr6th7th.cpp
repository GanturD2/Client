#include "stdafx.h"

#ifdef ENABLE_ATTR_6TH_7TH
#include "config.h"
#include "Attr6th7th.h"
#include "char.h"
#include "locale_service.h"
#include "packet.h"
#include "desc_client.h"
#include "item.h"
#include "item_manager.h"
#include "constants.h"
#include "utils.h"
#include "unique_item.h"
#include "log.h"

CAttr6th7th::CAttr6th7th() noexcept
{
}

void CAttr6th7th::StartComb(LPCHARACTER ch, uint8_t bCell[c_SkillBook_Comb_Slot_Max])
{
	if (!ch || !bCell)
		return;

	bool checkCount = CheckCombStart(ch, bCell);
	if (!checkCount)
		return;

	DeleteCombItems(ch, bCell);
}

bool CAttr6th7th::CheckCombStart(LPCHARACTER ch, uint8_t bCell[c_SkillBook_Comb_Slot_Max])
{
	if (!ch || !bCell)
		return false;

	int total_items = -1;

	for (uint8_t i = 0; i < c_SkillBook_Comb_Slot_Max; ++i)
	{
		LPITEM pItem = ch->GetInventoryItem(bCell[i]);
		if (pItem)
		{
			if (pItem->GetType() == ITEM_SKILLBOOK)
			{
				total_items++;
			}
		}
	}

	if (total_items != (c_SkillBook_Comb_Slot_Max - 1))
		return false;

	if (ch->GetGold() < GOLD_COMB_SKILLBOOK)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1240]");
		return false;
	}

	return true;
}

void CAttr6th7th::DeleteCombItems(LPCHARACTER ch, uint8_t bCell[c_SkillBook_Comb_Slot_Max])
{
	if (!ch || !bCell)
		return;

	for (uint8_t i = 0; i < c_SkillBook_Comb_Slot_Max; ++i)
	{
		LPITEM pItem = ch->GetInventoryItem(bCell[i]);
		if (pItem)
		{
			pItem->SetCount(pItem->GetCount() - 1);
		}
	}

	ch->PointChange(POINT_GOLD, -GOLD_COMB_SKILLBOOK);

	if (ch->GetSkillBookCombType() == 1)
	{
		ch->AutoGiveItem(FRAGMENT_BOX_VNUM);
	}
	else
	{
#ifdef ENABLE_WOLFMAN_CHARACTER
		uint32_t dwBooks[5][2][2] =
#else
		uint32_t dwBooks[4][2][2] =
#endif
		{
			{	//Warrior
				{50401, 50406}, // Skill Group 1
				{50416, 50421}, // Skill Group 2
			},
			{	//Ninja
				{50431, 50436}, // Skill Group 1
				{50446, 50451}, // Skill Group 2
			},
			{	//Sura
				{50461, 50466}, // Skill Group 1
				{50476, 50481}, // Skill Group 2
			},
			{	//Shaman
				{50491, 50496}, // Skill Group 1
				{50506, 50511}, // Skill Group 2
			},
#ifdef ENABLE_WOLFMAN_CHARACTER
			{	// Wolfman
				{50530, 50535}, // Skill Group 1
			},
#endif
		};

		if (ch->GetSkillGroup() != 0)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(dwBooks[ch->GetJob()][ch->GetSkillGroup() - 1][0], dwBooks[ch->GetJob()][ch->GetSkillGroup() - 1][1]);
			ch->AutoGiveItem(static_cast<uint32_t>(dis(gen)), 1);
		}
		else
			ch->AutoGiveItem(ITEM_SKILLBOOK_VNUM, 1);
	}

	ch->SetSkillBookCombType(0);
}

void CAttr6th7th::AddAttr6th7th(LPCHARACTER ch, uint8_t bRegistSlot, uint8_t bFragmentCount, uint16_t wCellAdditive, uint8_t bCountAdditive)
{
	if (!ch)
		return;

	if ((bFragmentCount <= 0 && bCountAdditive <= 0) || (bRegistSlot < 0))
		return;

	const LPITEM& get_space = ch->GetAttr67InventoryItem();
	LPITEM get_item = ch->GetItemAdded();
	LPITEM item_send = ch->GetInventoryItem(bRegistSlot);
	LPITEM item_additive = nullptr;
	uint8_t bTotalPercent = 0;

	if (!get_item || !item_send || get_space)
		return;

	if ((bCountAdditive > 0 && bFragmentCount <= 0) || (bCountAdditive > 0 && wCellAdditive <= 0) || (bCountAdditive > SUPPORT_MAX_COUNT))
		return;

	if (get_item->GetID() != item_send->GetID())
		return;

	if (CheckItemAdded(item_send) == false || GetFragmentVnum(ch, item_send) == 0)
		return;

	if (bFragmentCount != 0)
	{
		if (ch->CountSpecifyItem(GetFragmentVnum(ch, item_send)) < bFragmentCount || bFragmentCount > MATERIAL_MAX_COUNT || bFragmentCount <= 0)
			return;

		ch->RemoveSpecifyItem(GetFragmentVnum(ch, item_send), bFragmentCount);
		bTotalPercent += (bFragmentCount * SUCCESS_PER_MATERIAL);
	}

	if (wCellAdditive >= 0 && bCountAdditive > 0)
	{
		item_additive = ch->GetInventoryItem(wCellAdditive);
		if (!item_additive)
			return;

		const auto itemVnum = item_additive->GetVnum();
		const auto itemCount = item_additive->GetCount();

		if (itemVnum >= 72064 && itemVnum <= 72067)
		{
			if (itemCount < bCountAdditive)
				return;

			bTotalPercent += bCountAdditive * (static_cast<uint8_t>(item_additive->GetValue(1)) / SUPPORT_MAX_COUNT);
			item_additive->SetCount(item_additive->GetCount() - bCountAdditive);
		}
		else
			return;
	}

	ch->ItemAdded(nullptr);

	if (test_server || ch->IsGM())
		SetAddAttrTime(ch, get_global_time() + 30);
	else
		SetAddAttrTime(ch, get_global_time() + ATTR67_ADD_TIME_TO_WAIT);

	SetTotalPercentAttr(ch, bTotalPercent);

	sys_log(0, "Attr6th7th::AddAttr6th7th: %d", item_send->GetID());
	LogManager::Instance().ItemLog(ch, item_send, "ATTR_6TH_7TH", "ADD");

	item_send->RemoveFromCharacter();
	ch->SetItem(TItemPos(NPC_STORAGE, 0), item_send);
}

void CAttr6th7th::SetAddAttrTime(LPCHARACTER ch, int time)
{
	if (!ch)
		return;

	ch->SetQuestFlag("attr_6th_7th.time_to_wait", time);
}

int CAttr6th7th::GetAddAttrTime(const CHARACTER* ch)
{
	if (!ch)
		return 0;

	return ch->GetQuestFlag("attr_6th_7th.time_to_wait") - get_global_time();
}

bool CAttr6th7th::CheckItemAdded(LPITEM item)
{
	if (!item)
		return false;

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
		return false;
#endif

	if (item->GetType() == ITEM_WEAPON && item->GetSubType() == WEAPON_ARROW)
		return false;

#ifdef ENABLE_QUIVER_SYSTEM
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() == WEAPON_QUIVER)
		return false;
#endif

#ifdef ENABLE_GLOVE_SYSTEM
	if (item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_GLOVE)
		return false;
#endif

	if (item->GetType() == ITEM_ARMOR || item->GetType() == ITEM_WEAPON)
	{
		if (item->GetAttributeCount() >= 5 && item->GetAttributeCount() < 7)
			return true;
		else
			return false;
	}

	return false;
}

void CAttr6th7th::SetTotalPercentAttr(LPCHARACTER ch, uint8_t bPercent)
{
	if (!ch)
		return;

	ch->SetQuestFlag("attr_6th_7th.percent", bPercent);
}

uint8_t CAttr6th7th::GetPercentAttr(const CHARACTER* ch)
{
	if (!ch)
		return 0;

	uint8_t bPercent = static_cast<uint8_t>(ch->GetQuestFlag("attr_6th_7th.percent"));
	return bPercent;
}

void CAttr6th7th::CheckFragment(LPCHARACTER ch, uint8_t bRegistSlot)
{
	if (!ch)
		return;

	if (bRegistSlot < 0)
		return;

	LPITEM item = ch->GetInventoryItem(bRegistSlot);
	if (!item)
		return;

	if (!CheckItemAdded(item))
		return;

	const uint32_t dwVnum = GetFragmentVnum(ch, item);
	if (dwVnum == 0)
		return;

	ch->ItemAdded(item);
	RecvAttr67Packet(ch, dwVnum);
}

uint32_t CAttr6th7th::GetFragmentVnum(const CHARACTER* ch, LPITEM item)
{
	if (!ch || !item)
		return 0;

	const TItemTable* item_table = item->GetProto();
	return item_table ? item_table->dwMaterial67 : 0;
}

bool CAttr6th7th::GetEnoughInventory(const CHARACTER* ch)
{
	if (!ch)
		return false;

	LPITEM item = ch->GetAttr67InventoryItem();
	if (item)
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		const int iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item);
#else
		const int iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item->GetSize());
#endif
		if (iEmptyPos < 0)
			return false;

		return true;
	}

	return false;
}

void CAttr6th7th::GetItemAttr(LPCHARACTER ch, uint8_t* bResult, uint16_t* wSlot)
{
	*bResult = 0;
	*wSlot = 0;

	if (!ch)
		return;

	LPITEM item = ch->GetAttr67InventoryItem();
	if (item)
	{
		sys_log(0, "Attr6th7th::GetItemAttr: %d", item->GetID());
		LogManager::Instance().ItemLog(ch, item, "ATTR_6TH_7TH", "GET_ITEM_ATTR");

#ifdef ENABLE_SPECIAL_INVENTORY
		const uint16_t iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item);
#else
		const uint16_t iEmptyPos = item->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(item) : ch->GetEmptyInventory(item->GetSize());
#endif
		item->RemoveFromCharacter();

		const auto percent = number(1, 100);

		// 1 = true attr - 2 = false attr

		if (percent <= GetPercentAttr(ch))
		{
			item->AddRareAttribute();
			ch->SetItem(TItemPos(item->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iEmptyPos), item);
			SetAddAttrTime(ch, 0);
			SetTotalPercentAttr(ch, 0);
			*bResult = 1;
			*wSlot = iEmptyPos;
			return;
		}
		else
		{
			ch->SetItem(TItemPos(item->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iEmptyPos), item);
			SetAddAttrTime(ch, 0);
			SetTotalPercentAttr(ch, 0);
			*bResult = 2;
			*wSlot = iEmptyPos;
			return;
		}
	}
}

void CAttr6th7th::RecvAttr67Packet(const CHARACTER* ch, uint32_t vnum)
{
	if (!ch)
		return;

	TPacketGCAttr67Receive pack;
	pack.subheader = ATTR67_SUB_HEADER_FRAGMENT_RECEIVE;
	pack.fragmentVnum = vnum;

	LPDESC d = ch->GetDesc();

	if (nullptr == d)
	{
		sys_err("User RecvAttr67Packet (%s)'s DESC is nullptr POINT.", ch->GetName());
		return;
	}

	d->Packet(&pack, sizeof(pack));
}
#endif
