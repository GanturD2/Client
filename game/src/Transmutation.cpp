#include "stdafx.h"

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
#include "Transmutation.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "unique_item.h"
#include "desc.h"
#include "packet.h"

CTransmutation::CTransmutation(const LPCHARACTER ch, const bool bType) :
	m_ch(ch),
	m_type(static_cast<decltype(m_type)>(bType)),
	m_Item({}),
	m_FreeItem(nullptr)
{
	m_ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowChangeDialog %d", bType);
}

CTransmutation::~CTransmutation()
{
}

/*static*/ void CTransmutation::Open(const LPCHARACTER ch, const bool bType)
{
	if (ch == nullptr)
		return;

	if (ch->IsChangeLookWindowOpen() || ch->GetExchange() || ch->IsOpenSafebox() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetMyShop() || ch->GetShopOwner() ||
#endif
		ch->IsCubeOpen()
#ifdef ENABLE_CHANGED_ATTR
		|| ch->IsSelectAttr()
#endif
#ifdef ENABLE_MAILBOX
		|| ch->GetMailBox()
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		|| ch->IsOpenSkillBookComb()
#endif
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You have to close other windows.");
		return;
	}

	ch->SetTransmutation(new CTransmutation(ch, bType));
}

void CTransmutation::ItemCheckIn(const TItemPos& pos, const size_t bSlotType)
{
	if (bSlotType >= m_Item.size())
	{
		sys_err("Unknown SlotType m_ch:%s, %d", m_ch->GetName(), bSlotType);
		return;
	}

	LPITEM& item = m_Item.at(bSlotType);
	if (item != nullptr)
		return;

	if (!pos.IsDefaultInventoryPosition())
		return;

	const LPITEM inven_item = m_ch->GetItem(pos);
	if (inven_item == nullptr)
		return;

	if (inven_item->isLocked())
		return;

	switch (static_cast<ETRANSMUTATIONSLOTTYPE>(bSlotType))
	{
		case ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_LEFT:
			if (!CanAddItem(inven_item))
				return;
			break;

		case ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_RIGHT:
			if (!CheckOtherItem(inven_item))
				return;
			break;

		default:
			sys_err("Cant add transmutation material");
			return;
	}

	TPacketGCTransmutation pack =
	{
		HEADER_GC_CHANGE_LOOK,
		sizeof(TPacketGCTransmutation) + sizeof(TSubPacketGCChangeLookItemSet),
		static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_SET),
	};

	TSubPacketGCChangeLookItemSet sub =
	{
		pos.cell,
		static_cast<uint8_t>(bSlotType),
	};

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&sub, sizeof(sub));
	m_ch->GetDesc()->Packet(buf.read_peek(), buf.size());

	item = inven_item;
}

void CTransmutation::ItemCheckOut(const size_t bSlotType)
{
	if (bSlotType >= m_Item.size())
	{
		sys_err("Unknown SlotType (m_ch:%s), %d", m_ch->GetName(), bSlotType);
		return;
	}

	LPITEM& item = m_Item.at(bSlotType);
	if (item == nullptr)
		return;

	TPacketGCTransmutation pack =
	{
		HEADER_GC_CHANGE_LOOK,
		sizeof(TPacketGCTransmutation) + sizeof(TSubPacketGCChangeLookDel),
		static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_DEL),
	};

	TSubPacketGCChangeLookDel sub =
	{
		item->GetCell(),
		static_cast<uint8_t>(bSlotType),
	};

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&sub, sizeof(sub));
	m_ch->GetDesc()->Packet(buf.read_peek(), buf.size());

	item = nullptr;
}

void CTransmutation::FreeItemCheckIn(const TItemPos& pos)
{
	if (m_FreeItem != nullptr)
		return;

	if (pos.IsDefaultInventoryPosition() == false)
		return;

	const LPITEM inven_item = m_ch->GetItem(pos);
	if (inven_item == nullptr)
		return;

	if (inven_item->isLocked())
		return;

	if (CanAddFreeItem(inven_item) == false)
		return;

	TPacketGCTransmutation pack =
	{
		HEADER_GC_CHANGE_LOOK,
		sizeof(TPacketGCTransmutation) + sizeof(TSubPacketGCChangeLookItemSet),
		static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_FREE_SET),
	};

	TSubPacketGCChangeLookItemSet sub =
	{
		pos.cell,
		0,
	};

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&sub, sizeof(sub));
	m_ch->GetDesc()->Packet(buf.read_peek(), buf.size());

	m_FreeItem = inven_item;
}

void CTransmutation::FreeItemCheckOut()
{
	if (m_FreeItem == nullptr)
		return;

	TPacketGCTransmutation pack =
	{
		HEADER_GC_CHANGE_LOOK,
		sizeof(TPacketGCTransmutation) + sizeof(TSubPacketGCChangeLookDel),
		static_cast<uint8_t>(EGC_TRANSMUTATION_SHEADER::ITEM_FREE_DEL),
	};

	TSubPacketGCChangeLookDel sub =
	{
		m_FreeItem->GetCell(),
		0,
	};

	TEMP_BUFFER buf;
	buf.write(&pack, sizeof(pack));
	buf.write(&sub, sizeof(sub));
	m_ch->GetDesc()->Packet(buf.read_peek(), buf.size());

	m_FreeItem = nullptr;
}

void CTransmutation::Accept()
{
	const LPITEM left	= GetLeftItem();
	const LPITEM right	= GetRightItem();
	const LPITEM free	= GetFreeItem();

	if (left == nullptr || right == nullptr)
		return;

	const int iPrice = static_cast<int>(GetPrice());
	if (m_ch->GetGold() < iPrice)
	{
		m_ch->ChatPacket(CHAT_TYPE_INFO, "[Transmutation] You don't have enough yang.");
		return;
	}

	if (iPrice)
		m_ch->PointChange(POINT_GOLD, -iPrice);

	left->SetChangeLookVnum(right->GetVnum());
	left->UpdatePacket();

	/*Clear Slots*/
	for (size_t i = 0; i < m_Item.size(); i++)
		ItemCheckOut(i);

	FreeItemCheckOut();

	/*Remove*/
	ITEM_MANAGER::Instance().RemoveItem(right, "REMOVE (Transmutation - Item)");
	if (free != nullptr)
		ITEM_MANAGER::Instance().RemoveItem(free, "REMOVE (Transmutation - Free Item)");
}

bool CTransmutation::CanAddItem(const LPITEM target) const
{
	const uint8_t bItemType = target->GetType();
	const uint8_t bItemSubType = target->GetSubType();

	switch (m_type)
	{
		case ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_MOUNT:
		{
			if (bItemType == EItemTypes::ITEM_QUEST && (target->GetVnum() == 50051 || target->GetVnum() == 50052 || target->GetVnum() == 50053))
				return true;

			if (bItemType == EItemTypes::ITEM_COSTUME && bItemSubType == ECostumeSubTypes::COSTUME_MOUNT)
				return true;
		}
		break;

		case ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_ITEM:
		{
			if (bItemType == EItemTypes::ITEM_WEAPON && bItemSubType != EWeaponSubTypes::WEAPON_ARROW)
				return true;

			if (bItemType == EItemTypes::ITEM_ARMOR && bItemSubType == EArmorSubTypes::ARMOR_BODY)
				return true;

			if (bItemType == EItemTypes::ITEM_COSTUME && bItemSubType == ECostumeSubTypes::COSTUME_BODY)
				return true;
		}
		break;
	}

	return false;
}

bool CTransmutation::CheckOtherItem(const LPITEM material) const
{
	const auto target = GetLeftItem();

	m_ch->ChatPacket(CHAT_TYPE_INFO, "CheckOtherItem other_item %d", target->GetVnum());
	m_ch->ChatPacket(CHAT_TYPE_INFO, "CheckOtherItem item %d", material->GetVnum());

	if (target == nullptr)
		return false;

	if (target == material)
		return false;

	if (target->GetVnum() == material->GetVnum())
		return false;

	if (target->GetVnum() >= 50051 && target->GetVnum() <= 50053)
	{
		if (material->GetType() == EItemTypes::ITEM_COSTUME && material->GetSubType() != ECostumeSubTypes::COSTUME_MOUNT)
			return false;
	}
	else
	{
		if (target->GetType() != material->GetType())
			return false;

		if (target->GetSubType() != material->GetSubType())
			return false;

		if (target->GetAntiFlag() != material->GetAntiFlag())
			return false;
	}

	return true;
}

bool CTransmutation::CanAddFreeItem(const LPITEM item) const
{
	const uint32_t dwVnum = item->GetVnum();
	return dwVnum == TRANSMUTATION_TICKET_1
		|| dwVnum == TRANSMUTATION_TICKET_2;
}

bool CTransmutation::IsTypeItem() const
{
	return m_type == ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_ITEM;
}

bool CTransmutation::IsTypeMount() const
{
	return m_type == ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_MOUNT;
}

LPITEM CTransmutation::GetLeftItem() const
{
	return m_Item.at(static_cast<size_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_LEFT));
}

LPITEM CTransmutation::GetRightItem() const
{
	return m_Item.at(static_cast<size_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_RIGHT));
}

LPITEM CTransmutation::GetFreeItem() const
{
	return m_FreeItem;
}

uint32_t CTransmutation::GetPrice() const
{
	if (GetFreeItem() == nullptr)
	{
		if (IsTypeItem())
			return static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_ITEM_PRICE);

		if (IsTypeMount())
			return static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_MOUNT_PRICE);
	}

	return 0;
}

#endif