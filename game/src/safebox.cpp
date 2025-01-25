#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "safebox.h"
#include "packet.h"
#include "char.h"
#include "desc_client.h"
#include "item.h"
#include "item_manager.h"
#include "config.h"

#ifdef ENABLE_SAFEBOX_MONEY
CSafebox::CSafebox(LPCHARACTER pkChrOwner, int iSize, uint32_t dwGold) : m_pkChrOwner(pkChrOwner), m_iSize(iSize), m_lGold(dwGold)
#else
CSafebox::CSafebox(LPCHARACTER pkChrOwner, int iSize) : m_pkChrOwner(pkChrOwner), m_iSize(iSize)
#endif
{
	assert(m_pkChrOwner != nullptr);
	memset(m_pkItems, 0, sizeof(m_pkItems));
#ifdef ENABLE_SAFEBOX_MONEY
	SetSafeboxMoney(m_lGold);
#endif

	if (m_iSize)
		m_pkGrid = M2_NEW CGrid(5, m_iSize);
	else
		m_pkGrid = nullptr;

	m_bWindowMode = SAFEBOX;
}

CSafebox::~CSafebox()
{
	__Destroy();
}

void CSafebox::SetWindowMode(uint8_t bMode)
{
	m_bWindowMode = bMode;
}

void CSafebox::__Destroy()
{
	for (int i = 0; i < SAFEBOX_MAX_NUM; ++i)
	{
		if (m_pkItems[i])
		{
			m_pkItems[i]->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(m_pkItems[i]);

			M2_DESTROY_ITEM(m_pkItems[i]->RemoveFromCharacter());
			m_pkItems[i] = nullptr;
		}
	}

	if (m_pkGrid)
	{
		M2_DELETE(m_pkGrid);
		m_pkGrid = nullptr;
	}
}

bool CSafebox::Add(uint32_t dwPos, LPITEM pkItem)
{
	if (!pkItem)
		return false;

	if (!IsValidPosition(dwPos))
	{
		sys_err("SAFEBOX: item on wrong position at %d (size of grid = %d)", dwPos, m_pkGrid->GetSize());
		return false;
	}

	pkItem->SetWindow(m_bWindowMode);
	pkItem->SetCell(m_pkChrOwner, static_cast<uint16_t>(dwPos));
	pkItem->Save(); // Forced to call Save.
	ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);

	m_pkGrid->Put(dwPos, 1, pkItem->GetSize());
	m_pkItems[dwPos] = pkItem;

	TPacketGCItemSet pack;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (m_bWindowMode == SAFEBOX)
		pack.header = HEADER_GC_SAFEBOX_SET;
	else if(m_bWindowMode == GUILDBANK)
		pack.header = HEADER_GC_GUILDSTORAGE_SET;
	else 
		pack.header = HEADER_GC_MALL_SET;
#else
	pack.header = m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_SET : HEADER_GC_MALL_SET;
#endif
	pack.Cell = TItemPos(m_bWindowMode, static_cast<uint16_t>(dwPos));
	pack.vnum = pkItem->GetVnum();
	pack.count = static_cast<uint8_t>(pkItem->GetCount());
	pack.flags = pkItem->GetFlag();
	pack.anti_flags = pkItem->GetAntiFlag();
#ifdef ENABLE_SEALBIND_SYSTEM
	pack.nSealDate = pkItem->GetSealDate();
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	pack.dwTransmutationVnum = pkItem->GetChangeLookVnum();
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	pack.is_basic = pkItem->IsBasicItem();
#endif
#ifdef ENABLE_REFINE_ELEMENT
	pack.grade_element = pkItem->GetElementGrade();
	thecore_memcpy(pack.attack_element, pkItem->GetElementAttacks(), sizeof(pack.attack_element));
	pack.element_type_bonus = pkItem->GetElementsType();
	thecore_memcpy(pack.elements_value_bonus, pkItem->GetElementsValues(), sizeof(pack.elements_value_bonus));
#endif
	thecore_memcpy(pack.alSockets, pkItem->GetSockets(), sizeof(pack.alSockets));
	thecore_memcpy(pack.aAttr, pkItem->GetAttributes(), sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	thecore_memcpy(pack.aApplyRandom, pkItem->GetApplysRandom(), sizeof(pack.aApplyRandom));
	thecore_memcpy(pack.alRandomValues, pkItem->GetRandomDefaultAttrs(), sizeof(pack.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
	pack.set_value = pkItem->GetItemSetValue();
#endif

	m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
	sys_log(1, "SAFEBOX: ADD %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return true;
}

LPITEM CSafebox::Get(uint32_t dwPos)
{
	if (dwPos >= m_pkGrid->GetSize())
		return nullptr;

	return m_pkItems[dwPos];
}

LPITEM CSafebox::Remove(uint32_t dwPos)
{
	LPITEM pkItem = Get(dwPos);

	if (!pkItem)
		return nullptr;

	if (!m_pkGrid)
		sys_err("Safebox::Remove : nil grid");
	else
		m_pkGrid->Get(dwPos, 1, pkItem->GetSize());

	pkItem->RemoveFromCharacter();

	m_pkItems[dwPos] = nullptr;

	TPacketGCItemDel pack{};

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if (m_bWindowMode == SAFEBOX)
		pack.header = HEADER_GC_SAFEBOX_DEL;
	else if(m_bWindowMode == GUILDBANK)
		pack.header = HEADER_GC_GUILDSTORAGE_DEL;
	else 
		pack.header = HEADER_GC_MALL_DEL;
#else
	pack.header = m_bWindowMode == SAFEBOX ? HEADER_GC_SAFEBOX_DEL : HEADER_GC_MALL_DEL;
#endif

	pack.pos = dwPos;

	m_pkChrOwner->GetDesc()->Packet(&pack, sizeof(pack));
	sys_log(1, "SAFEBOX: REMOVE %s %s count %d", m_pkChrOwner->GetName(), pkItem->GetName(), pkItem->GetCount());
	return pkItem;
}

#ifdef ENABLE_SAFEBOX_MONEY
void CSafebox::Save()
{
	TSafeboxTable t;

	memset(&t, 0, sizeof(TSafeboxTable));

	t.dwID = m_pkChrOwner->GetDesc()->GetAccountTable().id;
#ifdef ENABLE_SAFEBOX_MONEY
	t.dwGold = m_lGold;
#endif

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_SAVE, 0, &t, sizeof(TSafeboxTable));
	sys_log(1, "SAFEBOX: SAVE %s", m_pkChrOwner->GetName());
}
#endif

bool CSafebox::IsEmpty(uint32_t dwPos, uint8_t bSize)
{
	if (!m_pkGrid)
		return false;

	return m_pkGrid->IsEmpty(dwPos, 1, bSize);
}

void CSafebox::ChangeSize(int iSize)
{
	// If the current size is larger than the argument, the size is left alone.
	if (m_iSize >= iSize)
		return;

	m_iSize = iSize;

	CGrid* pkOldGrid = m_pkGrid;

	if (pkOldGrid)
	{
		m_pkGrid = M2_NEW CGrid(pkOldGrid, 5, m_iSize);
		delete pkOldGrid; //@fixme523
	}
	else
		m_pkGrid = M2_NEW CGrid(5, m_iSize);
}

#ifdef ENABLE_SAFEBOX_MONEY
uint32_t CSafebox::GetSafeboxMoney() const
{
	return m_lGold;
}

void CSafebox::SetSafeboxMoney(uint32_t dGold)
{
	m_lGold = dGold;
	if (!m_pkChrOwner || !m_pkChrOwner->GetDesc())
		return;

	TPacketGCSafeboxMoneyChange pMoney;
	pMoney.bHeader = HEADER_GC_SAFEBOX_MONEY_CHANGE;
	pMoney.dwMoney = m_lGold;
	m_pkChrOwner->GetDesc()->Packet(&pMoney, sizeof(TPacketGCSafeboxMoneyChange));
}
#endif

LPITEM CSafebox::GetItem(uint8_t bCell)
{
	if (bCell >= 5 * m_iSize)
	{
		sys_err("CHARACTER::GetItem: invalid item cell %d", bCell);
		return nullptr;
	}

	return m_pkItems[bCell];
}

bool CSafebox::MoveItem(uint8_t bCell, uint8_t bDestCell, uint8_t count)
{
	LPITEM item;

	const int max_position = 5 * m_iSize;

	if (bCell >= max_position || bDestCell >= max_position)
		return false;

	if (!(item = GetItem(bCell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	{
		LPITEM item2;

		if ((item2 = GetItem(bDestCell)) && item != item2 && item2->IsStackable() &&
			!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
			item2->GetVnum() == item->GetVnum()) // For items that can be combined
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;
			}

			if (count == 0)
				count = static_cast<uint8_t>(item->GetCount());

			count = MIN(g_bItemCountLimit - item2->GetCount(), count);

			if (item->GetCount() >= count)
				Remove(bCell);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);

			sys_log(1, "SAFEBOX: STACK %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell, item2->GetName(), item2->GetCount());
			return true;
		}

		if (!IsEmpty(bDestCell, item->GetSize()))
			return false;

		m_pkGrid->Get(bCell, 1, item->GetSize());

		if (!m_pkGrid->Put(bDestCell, 1, item->GetSize()))
		{
			m_pkGrid->Put(bCell, 1, item->GetSize());
			return false;
		}
		else
		{
			m_pkGrid->Get(bDestCell, 1, item->GetSize());
			m_pkGrid->Put(bCell, 1, item->GetSize());
		}

		sys_log(1, "SAFEBOX: MOVE %s %d -> %d %s count %d", m_pkChrOwner->GetName(), bCell, bDestCell, item->GetName(), item->GetCount());

		Remove(bCell);
		Add(bDestCell, item);
	}

	return true;
}

bool CSafebox::IsValidPosition(uint32_t dwPos)
{
	if (!m_pkGrid)
		return false;

	if (dwPos >= m_pkGrid->GetSize())
		return false;

	return true;
}
