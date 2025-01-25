#include "stdafx.h"
#include "constants.h"
#include "char.h"
#include "desc.h"
#include "desc_manager.h"
#include "packet.h"
#include "item.h"

/////////////////////////////////////////////////////////////////////////////
// QUICKSLOT HANDLING
/////////////////////////////////////////////////////////////////////////////
void CHARACTER::SyncQuickslot(uint8_t bType, uint8_t bOldPos, uint8_t bNewPos) // bNewPos == 255
{
	if (bOldPos == bNewPos)
		return;

	for (uint8_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (m_quickslot[i].type == bType && m_quickslot[i].pos == bOldPos)
		{
			if (bNewPos == QUICKSLOT_MAX_POS)
				DelQuickslot(i);
			else
			{
				TQuickSlot slot{};

				slot.type = bType;
				slot.pos = bNewPos;

				SetQuickslot(i, slot);
			}
		}
	}
}

int CHARACTER::GetQuickslotPosition(uint8_t bType, uint8_t bInventoryPos)
{
	for (int i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (m_quickslot[i].type == bType && m_quickslot[i].pos == bInventoryPos)
			return i;
	}

	return -1;
}

bool CHARACTER::GetQuickslot(uint8_t pos, TQuickSlot** ppSlot)
{
	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	*ppSlot = &m_quickslot[pos];
	return true;
}

bool CHARACTER::SetQuickslot(uint8_t pos, TQuickSlot& rSlot)
{
	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	if (rSlot.type >= QUICKSLOT_TYPE_MAX_NUM)
		return false;

	for (uint8_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (rSlot.type == 0)
			continue;

		// need to test
		if (m_quickslot[i].type == rSlot.type && m_quickslot[i].pos == rSlot.pos)
			DelQuickslot(i);
	}

	switch (rSlot.type)
	{
		case QUICKSLOT_TYPE_ITEM:
		{
			const TItemPos srcCell(INVENTORY, rSlot.pos);
			if (false == srcCell.IsDefaultInventoryPosition()
#ifdef ENABLE_SPECIAL_INVENTORY
				&& false == srcCell.IsSpecialInventoryPosition()
#endif
				)
				return false;
		}
		break;

		case QUICKSLOT_TYPE_SKILL:
		{
			if (rSlot.pos >= SKILL_MAX_NUM)
				return false;
		}
		break;

		case QUICKSLOT_TYPE_COMMAND:
			break;

#ifdef ENABLE_BELT_INVENTORY
		case QUICKSLOT_TYPE_BELT:
		{
			const TItemPos srcCell(BELT_INVENTORY, rSlot.pos);
			if (false == srcCell.IsBeltInventoryPosition())
				return false;
		}
		break;
#endif

		default:
			return false;
	}

	m_quickslot[pos] = rSlot;

	if (GetDesc())
	{
		TPacketGCQuickSlotAdd pack_quickslot_add{};
		pack_quickslot_add.header = HEADER_GC_QUICKSLOT_ADD;
		pack_quickslot_add.pos = pos;
		pack_quickslot_add.slot = m_quickslot[pos];

		GetDesc()->Packet(&pack_quickslot_add, sizeof(TPacketGCQuickSlotAdd));
	}

	return true;
}

bool CHARACTER::DelQuickslot(uint8_t pos)
{
	if (pos >= QUICKSLOT_MAX_NUM)
		return false;

	memset(&m_quickslot[pos], 0, sizeof(TQuickSlot));

	if (GetDesc())
	{
		TPacketGCQuickSlotDel pack_quickslot_del{};
		pack_quickslot_del.header = HEADER_GC_QUICKSLOT_DEL;
		pack_quickslot_del.pos = pos;

		GetDesc()->Packet(&pack_quickslot_del, sizeof(TPacketGCQuickSlotDel));
	}
	return true;
}

bool CHARACTER::SwapQuickslot(uint8_t a, uint8_t b)
{
	if (a >= QUICKSLOT_MAX_NUM || b >= QUICKSLOT_MAX_NUM)
		return false;

	// Swap the quick slot positions.
	TQuickSlot quickslot{};
	quickslot = m_quickslot[a];

	m_quickslot[a] = m_quickslot[b];
	m_quickslot[b] = quickslot;

	if (GetDesc())
	{
		TPacketGCQuickSlotSwap pack_quickslot_swap{};
		pack_quickslot_swap.header = HEADER_GC_QUICKSLOT_SWAP;
		pack_quickslot_swap.pos = a;
		pack_quickslot_swap.pos_to = b;

		GetDesc()->Packet(&pack_quickslot_swap, sizeof(TPacketGCQuickSlotSwap));
	}
	return true;
}

void CHARACTER::ChainQuickslotItem(LPITEM pItem, uint8_t bType, uint8_t bOldPos)
{
	if (!pItem)
		return;

	if (pItem->IsDragonSoul())
		return;

	for (uint8_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (m_quickslot[i].type == bType && m_quickslot[i].pos == bOldPos)
		{
			TQuickSlot slot{};
			slot.type = bType;
			slot.pos = static_cast<uint8_t>(pItem->GetCell());

			SetQuickslot(i, slot);

			break;
		}
	}
}

void CHARACTER::MoveQuickSlotItem(uint8_t bOldType, uint8_t bOldPos, uint8_t bNewType, uint8_t bNewPos)
{
	for (uint8_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (m_quickslot[i].type == bOldType && m_quickslot[i].pos == bOldPos)
		{
			TQuickSlot slot{};
			slot.type = bNewType;
			slot.pos = bNewPos;

			SetQuickslot(i, slot);

			break;
		}
	}
}
