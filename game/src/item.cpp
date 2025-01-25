#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "desc.h"
#include "sectree_manager.h"
#include "packet.h"
#include "protocol.h"
#include "log.h"
#include "skill.h"
#include "unique_item.h"
#include "profiler.h"
#include "marriage.h"
#include "item_addon.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "affect.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "party.h"
#include "mob_manager.h"
#include "../../common/VnumHelper.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "db.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif

CItem::CItem(uint32_t dwVnum) :
	m_dwVnum(dwVnum),
	m_bWindow(0),
	m_dwID(0),
	m_bEquipped(false),
	m_dwVID(0),
	m_wCell(0),
	m_dwCount(0),
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	m_dwTransmutationVnum(0),
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	is_basic(false),
#endif
#ifdef ENABLE_SET_ITEM
	set_value(0),
#endif
	m_lFlag(0),
	m_dwLastOwnerPID(0),
	m_bExchanging(false),
	m_pkDestroyEvent(nullptr),
	m_pkExpireEvent(nullptr),
	m_pkUniqueExpireEvent(nullptr),
	m_pkTimerBasedOnWearExpireEvent(nullptr),
	m_pkRealTimeExpireEvent(nullptr),
	m_pkAccessorySocketExpireEvent(nullptr),
	m_pkOwnershipEvent(nullptr),
	m_dwOwnershipPID(0),
	m_bSkipSave(false),
	m_isLocked(false),
	m_dwMaskVnum(0),
	m_dwSIGVnum(0)
#ifdef ENABLE_SOUL_SYSTEM
	, m_pkSoulTimeUseEvent(nullptr)
#endif
#if defined(ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
	, m_pkChangeLookExpireEvent(nullptr)
#endif
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
	, m_pkAuraBoostSocketExpireEvent(nullptr)
#endif
{
#ifdef ENABLE_REFINE_ELEMENT
	m_grade_element = 0;
	memset(&m_attack_element, 0, sizeof(m_attack_element));
	m_element_type_bonus = 0;
	memset(&m_elements_value_bonus, 0, sizeof(m_elements_value_bonus));
#endif
	memset(&m_alSockets, 0, sizeof(m_alSockets));
	memset(&m_aAttr, 0, sizeof(m_aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(&m_aApplyRandom, 0, sizeof(m_aApplyRandom));
	memset(&m_alRandomValues, 0, sizeof(m_alRandomValues));
#endif
#ifdef ENABLE_PET_CACHE_TEST // TEST
	memset(&m_dwPetInfo, 0, sizeof(m_dwPetInfo));
#endif
}

CItem::~CItem()
{
	Destroy();
}

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
struct FPartyCHCollector
{
	std::vector <LPCHARACTER> vecCHPtr; FPartyCHCollector() { }

	void operator () (LPCHARACTER ch)
	{
		vecCHPtr.emplace_back(ch);
	}
};
#endif

void CItem::Initialize()
{
	CEntity::Initialize(ENTITY_ITEM);

	m_bWindow = RESERVED_WINDOW;
	m_pOwner = nullptr;
	m_dwID = 0;
	m_bEquipped = false;
	m_dwVID = m_wCell = m_dwCount = m_lFlag = 0;
	m_pProto = nullptr;
	m_bExchanging = false;
#ifdef ENABLE_REFINE_ELEMENT
	m_grade_element = 0;
	memset(&m_attack_element, 0, sizeof(m_attack_element));
	m_element_type_bonus = 0;
	memset(&m_elements_value_bonus, 0, sizeof(m_elements_value_bonus));
#endif
	memset(&m_alSockets, 0, sizeof(m_alSockets));
	memset(&m_aAttr, 0, sizeof(m_aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(&m_aApplyRandom, 0, sizeof(m_aApplyRandom));
	memset(&m_alRandomValues, 0, sizeof(m_alRandomValues));
#endif
#ifdef ENABLE_PET_CACHE_TEST // TEST
	memset(&m_dwPetInfo, 0, sizeof(m_dwPetInfo));
#endif

	m_pkDestroyEvent = nullptr;
	m_pkOwnershipEvent = nullptr;
	m_dwOwnershipPID = 0;
	m_pkUniqueExpireEvent = nullptr;
	m_pkTimerBasedOnWearExpireEvent = nullptr;
	m_pkRealTimeExpireEvent = nullptr;

	m_pkAccessorySocketExpireEvent = nullptr;

	m_bSkipSave = false;
	m_dwLastOwnerPID = 0;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	m_dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_SOUL_SYSTEM
	m_pkSoulTimeUseEvent = nullptr;
#endif
#if defined(ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
	m_pkChangeLookExpireEvent = nullptr;
#endif
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
	m_pkAuraBoostSocketExpireEvent = nullptr;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	is_basic = false;
#endif
#ifdef ENABLE_SET_ITEM
	set_value = 0;
#endif
}

void CItem::Destroy()
{
	event_cancel(&m_pkDestroyEvent);
	event_cancel(&m_pkOwnershipEvent);
	event_cancel(&m_pkUniqueExpireEvent);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);
	event_cancel(&m_pkRealTimeExpireEvent);
	event_cancel(&m_pkAccessorySocketExpireEvent);
#ifdef ENABLE_SOUL_SYSTEM
	event_cancel(&m_pkSoulTimeUseEvent);
#endif
#if defined(ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
	event_cancel(&m_pkChangeLookExpireEvent);
#endif
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
	event_cancel(&m_pkAuraBoostSocketExpireEvent);
#endif

	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);
}

EVENTFUNC(item_destroy_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("item_destroy_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;
	if (!pkItem)
		return 0;

	if (pkItem->GetOwner())
		sys_err("item_destroy_event: Owner exist. (item %s owner %s)", pkItem->GetName(), pkItem->GetOwner()->GetName());

	pkItem->SetDestroyEvent(nullptr);
	M2_DESTROY_ITEM(pkItem);
	return 0;
}

void CItem::SetDestroyEvent(LPEVENT pkEvent)
{
	m_pkDestroyEvent = pkEvent;
}

void CItem::StartDestroyEvent(int iSec)
{
	if (m_pkDestroyEvent)
		return;

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetDestroyEvent(event_create(item_destroy_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::EncodeInsertPacket(LPENTITY ent)
{
	if (!ent)
		return;

	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	const PIXEL_POSITION& c_pos = GetXYZ();

	TPacketGCItemGroundAdd pack;

	pack.bHeader = HEADER_GC_ITEM_GROUND_ADD;
	pack.x = c_pos.x;
	pack.y = c_pos.y;
	pack.z = c_pos.z;
	pack.dwVnum = GetVnum();
	pack.dwVID = m_dwVID;
	//pack.count = m_dwCount;

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	for (size_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = GetSocket(i);

	thecore_memcpy(pack.aAttrs, GetAttributes(), sizeof(pack.aAttrs));
#endif

	d->Packet(&pack, sizeof(pack));

	if (m_pkOwnershipEvent != nullptr)
	{
		const item_event_info* info = dynamic_cast<item_event_info*>(m_pkOwnershipEvent->info);

		if (info == nullptr)
		{
			sys_err("CItem::EncodeInsertPacket> <Factor> Null pointer");
			return;
		}

		TPacketGCItemOwnership p{};

		p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
		p.dwVID = m_dwVID;
		strlcpy(p.szName, info->szOwnerName, sizeof(p.szName));

		d->Packet(&p, sizeof(TPacketGCItemOwnership));
	}
}

void CItem::EncodeRemovePacket(LPENTITY ent)
{
	if (!ent)
		return;

	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	TPacketGCItemGroundDel pack{};
	pack.bHeader = HEADER_GC_ITEM_GROUND_DEL;
	pack.dwVID = m_dwVID;

	d->Packet(&pack, sizeof(pack));
	sys_log(2, "Item::EncodeRemovePacket %s to %s", GetName(), (dynamic_cast<LPCHARACTER>(ent))->GetName());
}

void CItem::SetProto(const TItemTable* table)
{
	assert(table != nullptr);
	m_pProto = table;
	SetFlag(m_pProto->dwFlags);
}

void CItem::UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use* packet)
{
	if (!ch || !victim)
		return;

	if (!GetVnum())
		return;

	packet->header = HEADER_GC_ITEM_USE;
	packet->ch_vid = ch->GetVID();
	packet->victim_vid = victim->GetVID();
	packet->Cell = TItemPos(GetWindow(), m_wCell);
	packet->vnum = GetVnum();
}

void CItem::RemoveFlag(long bit)
{
	REMOVE_BIT(m_lFlag, bit);
}

void CItem::AddFlag(long bit)
{
	SET_BIT(m_lFlag, bit);
}

void CItem::UpdatePacket()
{
	if (!m_pOwner || !m_pOwner->GetDesc())
		return;

#ifdef ENABLE_SWITCHBOT
	if (m_bWindow == SWITCHBOT)
		return;
#endif

	TPacketGCItemUpdate pack;

	pack.header = HEADER_GC_ITEM_UPDATE;
	pack.Cell = TItemPos(GetWindow(), m_wCell);
	pack.count = static_cast<uint8_t>(m_dwCount);
#ifdef ENABLE_SEALBIND_SYSTEM
	pack.nSealDate = GetSealDate();
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	pack.dwTransmutationVnum = GetChangeLookVnum();
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	pack.is_basic = is_basic;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	pack.grade_element = m_grade_element;
	for (int e = 0; e < MAX_ELEMENTS_SPELL; ++e)
	{
		pack.attack_element[e] = m_attack_element[e];
		pack.elements_value_bonus[e] = m_elements_value_bonus[e];
	}
	pack.element_type_bonus = m_element_type_bonus;
#endif
#ifdef ENABLE_SET_ITEM
	pack.set_value = set_value;
#endif

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = m_alSockets[i];

	thecore_memcpy(pack.aAttr, GetAttributes(), sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	thecore_memcpy(pack.aApplyRandom, GetApplysRandom(), sizeof(pack.aApplyRandom));
	thecore_memcpy(pack.alRandomValues, GetRandomDefaultAttrs(), sizeof(pack.alRandomValues));
#endif

	sys_log(2, "UpdatePacket %s -> %s", GetName(), m_pOwner->GetName());
	m_pOwner->GetDesc()->Packet(&pack, sizeof(pack));
}

uint32_t CItem::GetCount()
{
	if (GetType() == ITEM_ELK)
		return MIN(m_dwCount, INT_MAX);
	else
		return MIN(m_dwCount, g_bItemCountLimit);
}

bool CItem::SetCount(uint32_t count)
{
	if (GetType() == ITEM_ELK)
	{
		m_dwCount = MIN(count, INT_MAX);
	}
	else
	{
		m_dwCount = MIN(count, g_bItemCountLimit);
	}

	if (count == 0 && m_pOwner)
	{
		if (GetSubType() == USE_ABILITY_UP || GetSubType() == USE_POTION || GetVnum() == 70020 || GetVnum() == 39003) // Nyx : added 39003
		{
			LPCHARACTER pOwner = GetOwner();
			if (!pOwner)
				return false;

			const uint16_t wCell = GetCell();
			const uint8_t bWindow = GetWindow();

			RemoveFromCharacter();

			if (!IsDragonSoul())
			{
				LPITEM pItem = pOwner->FindSpecifyItem(GetVnum());

				if (nullptr != pItem)
				{
					if (bWindow == INVENTORY)
						pOwner->ChainQuickslotItem(pItem, QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(wCell));
					else if (bWindow == BELT_INVENTORY)
						pOwner->SyncQuickslot(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(wCell), QUICKSLOT_MAX_POS);
				}
				else
				{
					if (bWindow == INVENTORY)
						pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(wCell), QUICKSLOT_MAX_POS);
					else if (bWindow == BELT_INVENTORY)
						pOwner->SyncQuickslot(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(wCell), QUICKSLOT_MAX_POS);
				}
			}

			M2_DESTROY_ITEM(this);
		}
		else
		{
			if (!IsDragonSoul())
			{
				m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(m_wCell), QUICKSLOT_MAX_POS);
			}
			M2_DESTROY_ITEM(RemoveFromCharacter());
		}

		return false;
	}

	UpdatePacket();

	Save();
	return true;
}

#ifdef ENABLE_GIVE_BASIC_ITEM
void CItem::SetBasic(bool b)
{
	is_basic = b;
	UpdatePacket();
}
#endif

#ifdef ENABLE_SET_ITEM
void CItem::SetItemSetValue(uint8_t iSet)
{
	set_value = iSet;
	UpdatePacket();
}
#endif

LPITEM CItem::RemoveFromCharacter()
{
	if (!m_pOwner)
	{
		sys_err("Item::RemoveFromCharacter owner null");
		return (this);
	}

	LPCHARACTER pOwner = m_pOwner;
	if (!pOwner)
	{
		sys_err("Item::RemoveFromCharacter LPCHARACTER pOwner is null");
		return (this);
	}

	if (m_bEquipped) // Is it equipped?
	{
		Unequip();
		//pOwner->UpdatePacket();

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
	else
	{
		if (GetWindow() != SAFEBOX && GetWindow() != MALL
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			&& GetWindow() != GUILDBANK
#endif
#ifdef ENABLE_ATTR_6TH_7TH
			&& GetWindow() != NPC_STORAGE
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			&& GetWindow() != PREMIUM_PRIVATE_SHOP
#endif
			)
		{
			if (IsDragonSoul())
			{
				if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= DRAGON_SOUL_INVENTORY_MAX_NUM");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}
			else if (GetWindow() == EQUIPMENT)
			{
				if (m_wCell >= EQUIPMENT_SLOT_COUNT)
					sys_err("CItem::RemoveFromCharacter: pos >= EQUIPMENT_SLOT_COUNT");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}
			else if (GetWindow() == BELT_INVENTORY)
			{
				if (m_wCell >= BELT_INVENTORY_SLOT_COUNT)
					sys_err("CItem::RemoveFromCharacter: pos >= BELT_INVENTORY_SLOT_COUNT");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}
#ifdef ENABLE_SWITCHBOT
			else if (GetWindow() == SWITCHBOT)
			{
				if (m_wCell >= SWITCHBOT_SLOT_COUNT)
					sys_err("CItem::RemoveFromCharacter: pos >= SWITCHBOT_SLOT_COUNT");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			else if (GetWindow() == ADDITIONAL_EQUIPMENT_1)
			{
				if (m_wCell >= ADDITIONAL_EQUIPMENT_MAX_SLOT)
					sys_err("CItem::RemoveFromCharacter: pos >= ADDITIONAL_EQUIPMENT_MAX_SLOT");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}
#endif
			else
			{
				const TItemPos cell(INVENTORY, m_wCell);

#ifdef ENABLE_SPECIAL_INVENTORY
				const bool defaultSpecialInventoryPosition = GetSpecialInventoryType() != -1 && cell.IsSpecialInventoryPosition();
				if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition() && false == defaultSpecialInventoryPosition)
#else
				if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition()) // Or in your belongings?
#endif
					sys_err("CItem::RemoveFromCharacter: Invalid Item Position");
				else
				{
					pOwner->SetItem(cell, nullptr);
				}
			}
		}
#ifdef ENABLE_ATTR_6TH_7TH
		else if (GetWindow() == NPC_STORAGE)
		{
			pOwner->SetItem(TItemPos(NPC_STORAGE, 0), nullptr);
		}
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		else if (GetWindow() == PREMIUM_PRIVATE_SHOP)
		{
			const TItemPos cell(PREMIUM_PRIVATE_SHOP, m_wCell);
			pOwner->SetItem(cell, nullptr);
		}
#endif

		m_pOwner = nullptr;
		m_wCell = 0;

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
}

bool CItem::AddToCharacter(LPCHARACTER ch, TItemPos Cell)
{
	if (!ch)
		return false;

	assert(GetSectree() == nullptr);
	assert(m_pOwner == nullptr);
	const uint16_t pos = Cell.cell;
	const uint8_t window_type = Cell.window_type;

	if (INVENTORY == window_type)
	{
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		if (m_wCell >= ch->GetExtendInvenMax())
#else
		if (m_wCell >= INVENTORY_MAX_NUM)
#endif
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (EQUIPMENT == window_type)
	{
		if (m_wCell >= EQUIPMENT_SLOT_COUNT)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (BELT_INVENTORY == window_type)
	{
		if (m_wCell >= BELT_INVENTORY_SLOT_COUNT)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (DRAGON_SOUL_INVENTORY == window_type)
	{
		if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	else if (PREMIUM_PRIVATE_SHOP == window_type)
	{
		if (m_wCell >= SHOP_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif
#ifdef ENABLE_SWITCHBOT
	else if (SWITCHBOT == window_type)
	{
		if (m_wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CItem::AddToCharacter:switchbot cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	else if (ADDITIONAL_EQUIPMENT_1 == window_type)
	{
		if (m_wCell >= ADDITIONAL_EQUIPMENT_MAX_SLOT)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif

	const bool isHighLight = this->GetLastOwnerPID() == ch->GetPlayerID();

	if (ch->GetDesc())
		m_dwLastOwnerPID = ch->GetPlayerID();

	event_cancel(&m_pkDestroyEvent);

	ch->SetItem(TItemPos(window_type, pos), this, isHighLight);
	m_pOwner = ch;

	Save();
	return true;
}

LPITEM CItem::RemoveFromGround()
{
	if (GetSectree())
	{
		SetOwnership(nullptr);

		GetSectree()->RemoveEntity(this);

		ViewCleanup();

		Save();
	}

	return (this);
}

bool CItem::AddToGround(long lMapIndex, const PIXEL_POSITION& pos, bool skipOwnerCheck)
{
	if (0 == lMapIndex)
	{
		sys_err("wrong map index argument: %d", lMapIndex);
		return false;
	}

	if (GetSectree())
	{
		sys_err("sectree already assigned");
		return false;
	}

	if (!skipOwnerCheck && m_pOwner)
	{
		sys_err("owner pointer not null");
		return false;
	}

	LPSECTREE tree = SECTREE_MANAGER::Instance().Get(lMapIndex, pos.x, pos.y);

	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d", pos.x, pos.y);
		return false;
	}

	//tree->Touch();

	SetWindow(GROUND);
	SetXYZ(pos.x, pos.y, pos.z);
	tree->InsertEntity(this);
	UpdateSectree();
	Save();
	return true;
}

bool CItem::DistanceValid(LPCHARACTER ch)
{
	if (!ch)
		return false;

	if (!GetSectree())
		return false;

	const int iDist = DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY());

	if (iDist > RANGE_PICK) // @fixme173
		return false;

	return true;
}

bool CItem::CanUsedBy(LPCHARACTER ch)
{
	if (!ch)
		return false;

	// Anti flag check
	switch (ch->GetJob())
	{
		case JOB_WARRIOR:
			if (GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case JOB_WOLFMAN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break;
#endif

		default:
			break;
	}

	return true;
}

int CItem::FindEquipCell(LPCHARACTER ch, int iCandidateCell)
{
	if (!ch)
		return -1;

	// Costume items (ITEM_COSTUME) do not need WearFlag. (Dividing the wear position by sub type. Do I need to give a wear flag again..)
	// Dragon Soul Stones (ITEM_DS, ITEM_SPECIAL_DS) are also classified by SUB_TYPE. New rings and belts are classified by ITEM_TYPE -_-
	if ((0 == GetWearFlag() || ITEM_TOTEM == GetType()) && ITEM_COSTUME != GetType() && ITEM_DS != GetType() && ITEM_SPECIAL_DS != GetType() && ITEM_RING != GetType() && ITEM_BELT != GetType()
#ifdef ENABLE_PET_SYSTEM
		&& ITEM_PET != GetType()
#endif
		)
		return -1;

	/* Dragon Soul Stone slots cannot be processed as WEAR (up to 32 WEARs are possible, but if you add Dragon Soul Stones, it will exceed 32.)
	* From a specific location in the inventory ((INVENTORY_MAX_NUM + WEAR_MAX_NUM) to (INVENTORY_MAX_NUM + WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX - 1))
	* Determined by Dragon Soul Stone slot.
	* When returning, the reason for subtracting INVENTORY_MAX_NUM is,
	* Because WearCell originally returns INVENTORY_MAX_NUM.
	*/
	if (GetType() == ITEM_DS || GetType() == ITEM_SPECIAL_DS)
	{
		if (iCandidateCell < 0)
		{
			return WEAR_MAX_NUM + GetSubType();
		}
		else
		{
			for (int i = 0; i < DRAGON_SOUL_DECK_MAX_NUM; i++)
			{
				if (WEAR_MAX_NUM + i * DS_SLOT_MAX + GetSubType() == iCandidateCell)
				{
					return iCandidateCell;
				}
			}
			return -1;
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		if (GetSubType() == COSTUME_BODY)
			return WEAR_COSTUME_BODY;
		else if (GetSubType() == COSTUME_HAIR)
			return WEAR_COSTUME_HAIR;
		else if (GetSubType() == COSTUME_MOUNT)	//ENABLE_MOUNT_COSTUME_SYSTEM
			return WEAR_COSTUME_MOUNT;
		else if (GetSubType() == COSTUME_ACCE)	//ENABLE_ACCE_COSTUME_SYSTEM
			return WEAR_COSTUME_ACCE;
		else if (GetSubType() == COSTUME_WEAPON)	//ENABLE_WEAPON_COSTUME_SYSTEM
			return WEAR_COSTUME_WEAPON;
		else if (GetSubType() == COSTUME_AURA)	//ENABLE_AURA_SYSTEM
			return WEAR_COSTUME_AURA;
	}
	else if (GetType() == ITEM_RING)
	{
		if (ch->GetWear(WEAR_RING1))
			return WEAR_RING2;
		else
			return WEAR_RING1;
	}

#if defined(ENABLE_PROTO_RENEWAL_CUSTOM) && defined(ENABLE_PET_SYSTEM)
	else if (GetType() == ITEM_PET)
	{
		if (GetSubType() == PET_PAY)
			return WEAR_PET;
	}
#endif

	else if (GetType() == ITEM_BELT)
		return WEAR_BELT;
	else if (GetWearFlag() & WEARABLE_BODY)
		return WEAR_BODY;
	else if (GetWearFlag() & WEARABLE_HEAD)
		return WEAR_HEAD;
	else if (GetWearFlag() & WEARABLE_FOOTS)
		return WEAR_FOOTS;
	else if (GetWearFlag() & WEARABLE_WRIST)
		return WEAR_WRIST;
	else if (GetWearFlag() & WEARABLE_WEAPON)
		return WEAR_WEAPON;
	else if (GetWearFlag() & WEARABLE_SHIELD)
		return WEAR_SHIELD;
	else if (GetWearFlag() & WEARABLE_NECK)
		return WEAR_NECK;
	else if (GetWearFlag() & WEARABLE_EAR)
		return WEAR_EAR;
	else if (GetWearFlag() & WEARABLE_ARROW)
		return WEAR_ARROW;
#ifdef ENABLE_PENDANT
	else if (GetWearFlag() & WEARABLE_PENDANT)
		return WEAR_PENDANT;
#endif
#ifdef ENABLE_GLOVE_SYSTEM
	else if (GetWearFlag() & WEARABLE_GLOVE)
		return WEAR_GLOVE;
#endif
	else if (GetWearFlag() & WEARABLE_UNIQUE)
	{
		if (ch->GetWear(WEAR_UNIQUE1))
			return WEAR_UNIQUE2;
		else
			return WEAR_UNIQUE1;
	}

	// Once the items for the collection quest are embedded, you can never -E.
	else if (GetWearFlag() & WEARABLE_ABILITY)
	{
		if (!ch->GetWear(WEAR_ABILITY1))
		{
			return WEAR_ABILITY1;
		}
		else if (!ch->GetWear(WEAR_ABILITY2))
		{
			return WEAR_ABILITY2;
		}
		else if (!ch->GetWear(WEAR_ABILITY3))
		{
			return WEAR_ABILITY3;
		}
		else if (!ch->GetWear(WEAR_ABILITY4))
		{
			return WEAR_ABILITY4;
		}
		else if (!ch->GetWear(WEAR_ABILITY5))
		{
			return WEAR_ABILITY5;
		}
		else if (!ch->GetWear(WEAR_ABILITY6))
		{
			return WEAR_ABILITY6;
		}
		else if (!ch->GetWear(WEAR_ABILITY7))
		{
			return WEAR_ABILITY7;
		}
		else if (!ch->GetWear(WEAR_ABILITY8))
		{
			return WEAR_ABILITY8;
		}
		else
		{
			return -1;
		}
	}
	return -1;
}


#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
long CItem::FirstAttribute(long attrVal, int i)
{
	switch (i)
	{
		case 0:
		{
			if (GetAttributeType(5) == APPLY_FIRST_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(5)) / 100;
			else if (GetAttributeType(6) == APPLY_FIRST_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(6)) / 100;
		}
		break;

		case 1:
		{
			if (GetAttributeType(5) == APPLY_SECOND_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(5)) / 100;
			else if (GetAttributeType(6) == APPLY_SECOND_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(6)) / 100;
		}
		break;

		case 2:
		{
			if (GetAttributeType(5) == APPLY_THIRD_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(5)) / 100;
			else if (GetAttributeType(6) == APPLY_THIRD_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(6)) / 100;
		}
		break;

		case 3:
		{
			if (GetAttributeType(5) == APPLY_FOURTH_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(5)) / 100;
			else if (GetAttributeType(6) == APPLY_FOURTH_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(6)) / 100;
		}
		break;

		case 4:
		{
			if (GetAttributeType(5) == APPLY_FIFTH_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(5)) / 100;
			else if (GetAttributeType(6) == APPLY_FIFTH_ATTRIBUTE_BONUS)
				attrVal = attrVal + (attrVal * GetAttributeValue(6)) / 100;
		}
		break;

		default:
			break;
	}

	return attrVal;
}
#endif

void CItem::ModifyPoints(bool bAdd)
{
	int accessoryGrade = 0;

	// Only weapons and armor are socketed.
	if (false == IsAccessoryForSocket())
	{
#ifdef ENABLE_QUIVER_SYSTEM
		if ((m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR) && m_pProto->bSubType != WEAPON_QUIVER && !IsRealTimeItem())
#else
		if (m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR)
#endif
		{
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
			if (m_pProto->bType == ITEM_ARMOR && m_pProto->bSubType == ARMOR_GLOVE)
			{
				for (int i = 3; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					auto lApplyType = GetSocket(i);
					if ((lApplyType == 0) || (lApplyType == 1) || (lApplyType == 28960))
						continue;

					const uint8_t bAttrValue[] = { 1, 3, 6, 9, 15 };
					int16_t sVal = 0;

					LPITEM pkStone = ITEM_MANAGER::Instance().CreateItem(GetSocket(i - 3));
					if (pkStone)
					{
						sVal = bAttrValue[pkStone->GetRefineLevel()];

						/*
						* Because there are no more sockets to store the information, I was forced to think
						* of a solution to find out which socket is "boosted" and which is not.
						* So I decided that the "boost" sockets will be the applyType x 10
						*/
						bool bGiveDoubleBonus = false;
						const auto wBoostApplyType = lApplyType / 10;
						if (wBoostApplyType >= APPLY_SUNGMA_STR && wBoostApplyType <= APPLY_SUNGMA_IMMUNE)
							bGiveDoubleBonus = true;

						// You have a small percentage of the bonus being doubled if the spiritStone you added is TYPE_GOLD
						if (pkStone->GetValue(5) == 2 && bGiveDoubleBonus)
						{
							sVal *= 2;
							lApplyType = wBoostApplyType;
						}
					}

					M2_DESTROY_ITEM(pkStone);
					m_pOwner->ApplyPoint(static_cast<uint16_t>(lApplyType), bAdd ? sVal : -sVal);
				}
			}
			else
#endif
			{
				// Not applied when the socket is used for attribute enhancement (ARMOR_WRIST, ARMOR_NECK, ARMOR_EAR)
#ifdef ENABLE_PROTO_RENEWAL
				for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
				for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
				{
					uint32_t dwVnum;

					if ((dwVnum = GetSocket(i)) <= 2)
						continue;

					const TItemTable* p = ITEM_MANAGER::Instance().GetTable(dwVnum);

					if (!p)
					{
						sys_err("cannot find table by vnum %u", dwVnum);
						continue;
					}

					if (ITEM_METIN == p->bType)
					{
						//m_pOwner->ApplyPoint(p->alValues[0], bAdd ? p->alValues[1] : -p->alValues[1]);
						for (int j = 0; j < ITEM_APPLY_MAX_NUM; ++j)
						{
							if (p->aApplies[j].wType == APPLY_NONE)
								continue;

							if (p->aApplies[j].wType == APPLY_SKILL)
							{
								m_pOwner->ApplyPoint(p->aApplies[j].wType, bAdd ? p->aApplies[j].lValue : p->aApplies[j].lValue ^ 0x00800000);
							}
							else
							{
								m_pOwner->ApplyPoint(p->aApplies[j].wType, bAdd ? p->aApplies[j].lValue : -p->aApplies[j].lValue);
							}
						}
					}
				}
			}
		}

		accessoryGrade = 0;
	}
	else
	{
		accessoryGrade = MIN(GetAccessorySocketGrade(), ITEM_ACCESSORY_SOCKET_MAX_NUM);
	}

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (m_pProto->aApplies[i].wType == APPLY_NONE)
			continue;

		long value = m_pProto->aApplies[i].lValue;

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		if (ITEM_COSTUME == GetType() && COSTUME_ACCE == GetSubType())
		{
			const uint32_t drainedItemVnum = GetSocket(0);
			if (drainedItemVnum == 0)
				continue;

			const TItemTable* p = GetAbsorbedItem();
			if (!p)
				continue;

			value = DrainedValue(p->aApplies[i].lValue);
			if (value < 0)
				value = 1;
		}
#endif

		if (m_pProto->aApplies[i].wType == APPLY_SKILL)
		{
			m_pOwner->ApplyPoint(m_pProto->aApplies[i].wType, bAdd ? value : value ^ 0x00800000);
		}
		else
		{
#ifdef ENABLE_YOHARA_SYSTEM
			if (!ItemHasApplyRandom())
#endif
			{
				if (0 != accessoryGrade && i < ITEM_APPLY_MAX_NUM - 1)	//@fixme170
					value += MAX(accessoryGrade, value * aiAccessorySocketEffectivePct[accessoryGrade] / 100);
			}

#if defined(ENABLE_MOUNT_PROTO_AFFECT_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
			if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_MOUNT)
			{
				if (m_pProto->aApplies[i].wType == APPLY_MOUNT && GetMountVnum())
					value = GetMountVnum();
			}
#endif

			m_pOwner->ApplyPoint(m_pProto->aApplies[i].wType, bAdd ? value : -value);
		}
	}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (GetType() == ITEM_COSTUME && GetSubType() == COSTUME_ACCE && GetSocket(ACCE_ABSORBED_SOCKET))
	{
		if (GetApplyType(ACCE_ABSORPTION_APPLY) == APPLY_ACCEDRAIN_RATE)
		{
			const TItemTable* pkItemAbsorbed = ITEM_MANAGER::Instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
			if (pkItemAbsorbed)
			{
				if (pkItemAbsorbed->bType == ITEM_ARMOR && pkItemAbsorbed->bSubType == ARMOR_BODY)
				{
					const long lDefGrade = DrainedValue(pkItemAbsorbed->alValues[1] + static_cast<long>(pkItemAbsorbed->alValues[5] * 2));
					m_pOwner->ApplyPoint(APPLY_DEF_GRADE_BONUS, bAdd ? lDefGrade : -lDefGrade);

					for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
					{
						if (pkItemAbsorbed->aApplies[i].wType == APPLY_NONE
							|| pkItemAbsorbed->aApplies[i].wType == APPLY_MOUNT
							|| pkItemAbsorbed->aApplies[i].wType == APPLY_DEF_GRADE_BONUS
							|| pkItemAbsorbed->aApplies[i].wType == APPLY_MOV_SPEED
							|| pkItemAbsorbed->aApplies[i].lValue <= 0)
							continue;

						const float fValue = static_cast<uint8_t>(DrainedValue(pkItemAbsorbed->aApplies[i].lValue));
						const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
						if (pkItemAbsorbed->aApplies[i].wType == APPLY_SKILL)
						{
							m_pOwner->ApplyPoint(pkItemAbsorbed->aApplies[i].wType, bAdd ? iValue : iValue ^ 0x00800000);
						}
						else
						{
							m_pOwner->ApplyPoint(pkItemAbsorbed->aApplies[i].wType, bAdd ? iValue : -iValue);
						}
					}
				}
				else if (pkItemAbsorbed->bType == ITEM_WEAPON)
				{
#	ifdef ENABLE_REFINE_ELEMENT //ACCE_ELEMENT
					long lAttGrade = DrainedValue(pkItemAbsorbed->alValues[4] + pkItemAbsorbed->alValues[5]);
					if (GetElementsType() != 0)
					{
						long lAttPlus = GetElementAttack(GetElementGrade() - 1);
						lAttPlus = DrainedValue(lAttPlus);
						if (lAttPlus > 0)
							lAttGrade += lAttPlus;
					}
#	else
					const long lAttGrade = DrainedValue(pkItemAbsorbed->alValues[4] + pkItemAbsorbed->alValues[5]);
#	endif
					m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? lAttGrade : -lAttGrade);

					const long lAttMagicGrade = DrainedValue(pkItemAbsorbed->alValues[2] + pkItemAbsorbed->alValues[5]);
					m_pOwner->ApplyPoint(APPLY_MAGIC_ATT_GRADE, bAdd ? lAttMagicGrade : -lAttMagicGrade);

					for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
					{
						if (pkItemAbsorbed->aApplies[i].wType == APPLY_NONE
							|| pkItemAbsorbed->aApplies[i].wType == APPLY_MOUNT
							|| pkItemAbsorbed->aApplies[i].wType == APPLY_ATT_GRADE_BONUS
							|| pkItemAbsorbed->aApplies[i].wType == APPLY_MAGIC_ATT_GRADE
							|| pkItemAbsorbed->aApplies[i].lValue <= 0)
							continue;

						const float fValue = static_cast<float>(DrainedValue(pkItemAbsorbed->aApplies[i].lValue));
						const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
						if (pkItemAbsorbed->aApplies[i].wType == APPLY_SKILL)
						{
							m_pOwner->ApplyPoint(pkItemAbsorbed->aApplies[i].wType, bAdd ? iValue : iValue ^ 0x00800000);
						}
						else
						{
							m_pOwner->ApplyPoint(pkItemAbsorbed->aApplies[i].wType, bAdd ? iValue : -iValue);
						}
					}

#	ifdef ENABLE_REFINE_ELEMENT //ACCE_ELEMENT
					if (GetElementsType() != 0)
					{
						long lBonusValue = GetElementsValue(GetElementGrade() - 1);
						lBonusValue = DrainedValue(lBonusValue);
						if (lBonusValue > 0)
						{
							m_pOwner->ApplyPoint(GetElementsType(), bAdd ? lBonusValue : -lBonusValue);
						}
					}
#	endif
				}
			}
		}
	}
#endif

#ifdef ENABLE_AURA_SYSTEM
	float fAuraDrainPer = 0.0f;
	if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA)
	{
		const long c_lLevelSocket = GetSocket(ITEM_SOCKET_AURA_CURRENT_LEVEL);
		const long c_lDrainSocket = GetSocket(ITEM_SOCKET_AURA_DRAIN_ITEM_VNUM);
#	ifdef ENABLE_AURA_BOOST
		const long c_lBoostSocket = GetSocket(ITEM_SOCKET_AURA_BOOST);
#	endif

		const uint8_t bCurLevel = static_cast<uint8_t>((c_lLevelSocket / 100000) - 1000);
#	ifdef ENABLE_AURA_BOOST
		const uint8_t bBoostIndex = static_cast<uint8_t>(c_lBoostSocket / 100000000);
		const TItemTable* pBoosterProto = ITEM_MANAGER::Instance().GetTable(ITEM_AURA_BOOST_ITEM_VNUM_BASE + bBoostIndex);
#	endif

		fAuraDrainPer = (1.0f * bCurLevel / 10.0f) / 100.0f;
#	ifdef ENABLE_AURA_BOOST
		if (pBoosterProto)
			fAuraDrainPer += 1.0f * pBoosterProto->alValues[ITEM_AURA_BOOST_PERCENT_VALUE] / 100.0f;
#	endif

		TItemTable* p = nullptr;
		if (c_lDrainSocket != 0)
			p = ITEM_MANAGER::Instance().GetTable(c_lDrainSocket);

		if (p != nullptr && (ITEM_ARMOR == p->bType && (ARMOR_SHIELD == p->bSubType || ARMOR_WRIST == p->bSubType || ARMOR_NECK == p->bSubType || ARMOR_EAR == p->bSubType)))
		{
			for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
			{
				if (p->aApplies[i].wType == APPLY_NONE || /*p->aApplies[i].bType == APPLY_MOUNT || */p->aApplies[i].wType == APPLY_MOV_SPEED || p->aApplies[i].lValue <= 0)
					continue;

				const float fValue = p->aApplies[i].lValue * fAuraDrainPer;
				const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
				if (p->aApplies[i].wType == APPLY_SKILL)
				{
					m_pOwner->ApplyPoint(p->aApplies[i].wType, bAdd ? iValue : iValue ^ 0x00800000);
				}
				else
				{
					m_pOwner->ApplyPoint(p->aApplies[i].wType, bAdd ? iValue : -iValue);
				}
			}
		}
	}
#endif

	/* For crescent rings, Halloween candy, happiness rings, and eternal love pendants
	* Although the property was forcibly assigned with the existing hard coding,
	* Removed that part and changed to assign attributes in the special item group table.
	* However, when it is hard-coded, the created item may remain, so it is treated specially.
	* In the case of these items, as attributes are given by processing when the item is ITEM_UNIQUE below,
	* Attributes embedded in items are not applied.
	*/
	if (GetType() != ITEM_RING)	//@fixme400
	{
		for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (GetAttributeType(i))
			{
				const TPlayerItemAttribute& ia = GetAttribute(i);
				long attrVal = ia.sValue;

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
				attrVal = FirstAttribute(attrVal, i);
#endif

				if (ia.wType == APPLY_SKILL)
				{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
					if (ITEM_COSTUME == GetType() && COSTUME_ACCE == GetSubType())
					{
						const uint32_t drainedItemVnum = GetSocket(0);
						if (drainedItemVnum == 0)
							continue;

						attrVal = DrainedValue(attrVal);

						if (attrVal < 0)
							attrVal = 1;

						m_pOwner->ApplyPoint(ia.wType, bAdd ? attrVal : attrVal ^ 0x00800000);
					}
#endif
#ifdef ENABLE_AURA_SYSTEM
					else if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA)
					{
						if (attrVal <= 0)
							continue;

						const float fValue = attrVal * fAuraDrainPer;
						const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
						m_pOwner->ApplyPoint(ia.wType, bAdd ? iValue : iValue ^ 0x00800000);
					}
#endif
#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_AURA_SYSTEM)
					else
#endif
					{
						m_pOwner->ApplyPoint(ia.wType, bAdd ? attrVal : attrVal ^ 0x00800000);
					}
				}
				else
				{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
					if (ITEM_COSTUME == GetType() && COSTUME_ACCE == GetSubType())
					{
						const uint32_t drainedItemVnum = GetSocket(0);
						if (drainedItemVnum == 0)
							continue;

						attrVal = DrainedValue(attrVal);

						if (attrVal < 0)
							attrVal = 1;

						m_pOwner->ApplyPoint(ia.wType, bAdd ? attrVal : -attrVal);
					}
#endif
#ifdef ENABLE_AURA_SYSTEM
					else if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA)
					{
						if (attrVal <= 0)
							continue;

						const float fValue = attrVal * fAuraDrainPer;
						const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
						m_pOwner->ApplyPoint(ia.wType, bAdd ? iValue : -iValue);
					}
#endif
#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_AURA_SYSTEM)
					else
#endif
					{
						m_pOwner->ApplyPoint(ia.wType, bAdd ? attrVal : -attrVal);
					}
				}
			}
		}
	}

#ifdef ENABLE_REFINE_ELEMENT
	if ((m_pProto->bType == ITEM_WEAPON) && (GetElementsType() != 0))
	{
		const long lAttPlus = GetElementAttack(GetElementGrade() - 1);
		if (lAttPlus > 0)
		{
			m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? lAttPlus : -lAttPlus);
		}

		const long lBonusValue = GetElementsValue(GetElementGrade() - 1);
		if (lBonusValue > 0)
		{
			m_pOwner->ApplyPoint(GetElementsType(), bAdd ? lBonusValue : -lBonusValue);
		}
	}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	for (int s = 0; s < APPLY_RANDOM_SLOT_MAX_NUM; ++s)
	{
		if (GetApplyRandomType(s))
		{
			const TPlayerItemApplyRandom& ia = GetApplyRandom(s);
			long randomVal = ia.sValue;

			if (ia.wType == APPLY_SKILL)
			{
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
				if (ITEM_COSTUME == GetType() && COSTUME_ACCE == GetSubType())
				{
					const uint32_t drainedItemVnum = GetSocket(0);
					if (drainedItemVnum == 0)
						continue;

					randomVal = DrainedValue(randomVal);

					if (randomVal < 0)
						randomVal = 1;

					m_pOwner->ApplyPoint(ia.wType, bAdd ? randomVal : randomVal ^ 0x00800000);
				}
#endif
#ifdef ENABLE_AURA_SYSTEM
				else if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA)
				{
					if (randomVal <= 0)
						continue;

					const float fValue = randomVal * fAuraDrainPer;
					const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
					m_pOwner->ApplyPoint(ia.wType, bAdd ? iValue : iValue ^ 0x00800000);
				}
#endif
#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_AURA_SYSTEM)
				else
#endif
				{
					m_pOwner->ApplyPoint(ia.wType, bAdd ? randomVal : randomVal ^ 0x00800000);
				}
			}
			else
			{
				if (0 != accessoryGrade)
					randomVal += MAX(accessoryGrade, randomVal * aiAccessorySocketEffectivePct[accessoryGrade] / 100);

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
				if (ITEM_COSTUME == GetType() && COSTUME_ACCE == GetSubType())
				{
					const uint32_t drainedItemVnum = GetSocket(0);
					if (drainedItemVnum == 0)
						continue;

					randomVal = DrainedValue(randomVal);

					if (randomVal < 0)
						randomVal = 1;

					m_pOwner->ApplyPoint(ia.wType, bAdd ? randomVal : -randomVal);
				}
#endif
#ifdef ENABLE_AURA_SYSTEM
				else if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA)
				{
					if (randomVal <= 0)
						continue;

					const float fValue = randomVal * fAuraDrainPer;
					const int iValue = static_cast<int>((fValue < 1.0f) ? ceilf(fValue) : truncf(fValue));
					m_pOwner->ApplyPoint(ia.wType, bAdd ? iValue : -iValue);
				}
#endif
#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_AURA_SYSTEM)
				else
#endif
				{
					m_pOwner->ApplyPoint(ia.wType, bAdd ? randomVal : -randomVal);
				}
			}
		}
	}
#endif

	switch (m_pProto->bType)
	{
		case ITEM_PICK:
		case ITEM_ROD:
		{
			if (bAdd)
			{
				if (m_wCell == WEAR_WEAPON)
					m_pOwner->SetPart(PART_WEAPON, GetVnum());
			}
			else
			{
				if (m_wCell == WEAR_WEAPON)
					m_pOwner->SetPart(PART_WEAPON, 0);
			}
		}
		break;

		case ITEM_WEAPON:
		{
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			if (0 != m_pOwner->GetWearDefault(WEAR_COSTUME_WEAPON))
				break;
			#else
			if (0 != m_pOwner->GetWear(WEAR_COSTUME_WEAPON))
				break;
			#endif
#endif

			if (bAdd)
			{
				if (m_wCell == WEAR_WEAPON)
				{
# ifdef ENABLE_CHANGE_LOOK_SYSTEM
					m_pOwner->SetPart(PART_WEAPON, GetChangeLookVnum() > 0 ? GetChangeLookVnum() : GetVnum());
# else
					m_pOwner->SetPart(PART_WEAPON, GetVnum());
# endif
				}
			}
			else
			{
				if (m_wCell == WEAR_WEAPON)
					m_pOwner->SetPart(PART_WEAPON, 0);
			}
		}
		break;

		case ITEM_ARMOR:
		{
			if (GetSubType() == ARMOR_BODY)
			{
				// If you're wearing a costume body, you shouldn't affect the visuals regardless of whether you're wearing armor or not.
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
				if (0 != m_pOwner->GetWearDefault(WEAR_COSTUME_BODY))
					break;
#else
				if (0 != m_pOwner->GetWear(WEAR_COSTUME_BODY))
					break;
#endif

				if (bAdd)
				{
					if (GetProto()->bSubType == ARMOR_BODY)
					{
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
						m_pOwner->SetPart(PART_MAIN, GetChangeLookVnum() > 0 ? GetChangeLookVnum() : GetVnum());
#else
						m_pOwner->SetPart(PART_MAIN, GetVnum());
#endif
					}
				}
				else
				{
					if (GetProto()->bSubType == ARMOR_BODY)
						m_pOwner->SetPart(PART_MAIN, m_pOwner->GetOriginalPart(PART_MAIN));
				}
			}
#ifdef ENABLE_PENDANT
			else if (GetSubType() == ARMOR_PENDANT)
			{
				if (bAdd)
				{
					if (GetProto()->bSubType == ARMOR_PENDANT)
						m_pOwner->SetPart(PART_PENDANT, GetVnum());
				}
				else
				{
					if (GetProto()->bSubType == ARMOR_PENDANT)
						m_pOwner->SetPart(PART_PENDANT, m_pOwner->GetOriginalPart(PART_PENDANT));
				}
			}
#endif
		}
		break;

		// Character parts information setting when wearing a costume item. Added to the original style.
		case ITEM_COSTUME:
		{
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //ENABLE_COSTUME_SYSTEM_CHANGE_LOOK
			uint32_t toSetValue = (this->GetChangeLookVnum() > 0) ? this->GetChangeLookVnum() : this->GetVnum();
#else
			uint32_t toSetValue = this->GetVnum();
#endif
			EParts toSetPart = PART_MAX_NUM;

			// armor costume
			if (GetSubType() == COSTUME_BODY)
			{
				toSetPart = PART_MAIN;

				if (false == bAdd)
				{
					// If you were wearing the original armor when you took off the costume armor, set the look to that armor, if not, the default look
					const CItem* pArmor = m_pOwner->GetWear(WEAR_BODY);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //ENABLE_COSTUME_SYSTEM_CHANGE_LOOK
					toSetValue = (nullptr != pArmor) ? pArmor->GetChangeLookVnum() > 0 ? pArmor->GetChangeLookVnum() : pArmor->GetVnum() : m_pOwner->GetOriginalPart(PART_MAIN);
#else
					toSetValue = (nullptr != pArmor) ? pArmor->GetVnum() : m_pOwner->GetOriginalPart(PART_MAIN);
#endif
				}

			}

			// hair costume
			else if (GetSubType() == COSTUME_HAIR)
			{
				toSetPart = PART_HAIR;

				// For costume hair, set the shape value to value3 of the item proto. There is no special reason, and the shape value of the existing armor (ARMOR_BODY) is the value3 of the prototype, so the hair is also set to value3.
				// [NOTE] The reason why armor sends item vnum and hair sends shape(value3) is... Existing system is like that...
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				if (GetChangeLookVnum() > 0)
				{
					const TItemTable* pHair = ITEM_MANAGER::Instance().GetTable(GetChangeLookVnum());
					if (!pHair)
						toSetValue = (true == bAdd) ? this->GetValue(3) : 0;
					else
						toSetValue = (true == bAdd) ? pHair->alValues[3] : 0;
				}
				else
					toSetValue = (true == bAdd) ? this->GetValue(3) : 0;
#else
				toSetValue = (true == bAdd) ? this->GetValue(3) : 0;
#endif
			}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			else if (GetSubType() == COSTUME_MOUNT)
			{
				// not need to do a thing in here
			}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			else if (GetSubType() == COSTUME_ACCE)
			{
				toSetPart = PART_ACCE;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				toSetValue = (true == bAdd) ? this->GetChangeLookVnum() > 0 ? this->GetChangeLookVnum() : this->GetVnum() : 0;
#else
				toSetValue = (true == bAdd) ? this->GetVnum() : 0;
#endif
			}
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			else if (GetSubType() == COSTUME_WEAPON)
			{
				toSetPart = PART_WEAPON;
				if (false == bAdd)
				{
					const CItem* pWeapon = m_pOwner->GetWear(WEAR_WEAPON);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //ENABLE_COSTUME_SYSTEM_CHANGE_LOOK
					toSetValue = (nullptr != pWeapon) ? pWeapon->GetChangeLookVnum() > 0 ? pWeapon->GetChangeLookVnum() : pWeapon->GetVnum() : m_pOwner->GetOriginalPart(PART_WEAPON);
#else
					toSetValue = (nullptr != pWeapon) ? pWeapon->GetVnum() : m_pOwner->GetOriginalPart(PART_WEAPON);
#endif
				}
			}
#endif

#ifdef ENABLE_AURA_SYSTEM
			else if (GetSubType() == COSTUME_AURA)
			{
				toSetPart = PART_AURA;
				toSetValue = (true == bAdd) ? this->GetVnum() : 0;
			}
#endif

			if (PART_MAX_NUM != toSetPart)
			{
				m_pOwner->SetPart(static_cast<uint8_t>(toSetPart), toSetValue);
				m_pOwner->UpdatePacket();
			}
		}
		break;

		case ITEM_UNIQUE:
		{
			if (0 != GetSIGVnum())
			{
				const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(GetSIGVnum());
				if (nullptr == pItemGroup)
					break;

				const uint32_t dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::Instance().GetSpecialAttrGroup(dwAttrVnum);
				if (nullptr == pAttrGroup)
					break;

				for (auto& it : pAttrGroup->m_vecAttrs)
				{
					m_pOwner->ApplyPoint(it.apply_type, bAdd ? it.apply_value : -it.apply_value);
				}
			}
		}
		break;

		case ITEM_RING:
		{
			if (0 != GetSIGVnum())
			{
				const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(GetSIGVnum());
				if (nullptr == pItemGroup)
					break;
				uint32_t dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::Instance().GetSpecialAttrGroup(dwAttrVnum);
				if (nullptr == pAttrGroup)
					break;
				for (auto it : pAttrGroup->m_vecAttrs)
				{
					m_pOwner->ApplyPoint(it.apply_type, bAdd ? it.apply_value : -it.apply_value);
				}
			}
		}
		break;

		default:
			break;
	}
}

bool CItem::IsEquipable() const
{
	switch (this->GetType())
	{
		case ITEM_COSTUME:
		case ITEM_ARMOR:
		case ITEM_WEAPON:
		case ITEM_ROD:
		case ITEM_PICK:
		case ITEM_UNIQUE:
		case ITEM_DS:
		case ITEM_SPECIAL_DS:
		case ITEM_RING:
		case ITEM_BELT:
#ifdef ENABLE_PASSIVE_SYSTEM
		case ITEM_PASSIVE:
#endif
			return true;

#ifdef ENABLE_PET_SYSTEM
		case ITEM_PET:
		{
			if (GetSubType() == PET_PAY)
				return true;
		}
#endif

		default:
			break;
	}

	return false;
}

// return false on error state
bool CItem::EquipTo(LPCHARACTER ch, uint8_t bWearCell)
{
	if (!ch)
	{
		sys_err("EquipTo: nil character");
		return false;
	}

	// Dragon Soul Slot index is greater than WEAR_MAX_NUM.
	if (IsDragonSoul())
	{
		if (bWearCell < WEAR_MAX_NUM || bWearCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
		{
			sys_err("EquipTo: invalid dragon soul cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetSubType(), bWearCell - WEAR_MAX_NUM);
			return false;
		}
	}
	else
	{
		if (bWearCell >= WEAR_MAX_NUM)
		{
			sys_err("EquipTo: invalid wear cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetWearFlag(), bWearCell);
			return false;
		}
	}

	if (ch->GetWear(bWearCell))
	{
		sys_err("EquipTo: item already exist (this: #%d %s cell: %d %s)", GetOriginalVnum(), GetName(), bWearCell, ch->GetWear(bWearCell)->GetName());
		return false;
	}

	if (GetOwner())
		RemoveFromCharacter();

	ch->SetWear(bWearCell, this); // Packet out here

	m_pOwner = ch;
	m_bEquipped = true;
	m_wCell = bWearCell;

#ifdef ENABLE_IMMUNE_FIX
	uint32_t dwImmuneFlag = 0;
	LPITEM item = NULL;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (item = m_pOwner->GetWear(i))
		{
			if (item->GetImmuneFlag() != 0)
				SET_BIT(dwImmuneFlag, item->GetImmuneFlag());
			if (item->GetAttributeCount() > 0)
			{
				if (item->HasAttr(APPLY_IMMUNE_STUN))
					SET_BIT(dwImmuneFlag, IMMUNE_STUN);
				if (item->HasAttr(APPLY_IMMUNE_SLOW))
					SET_BIT(dwImmuneFlag, IMMUNE_SLOW);
				if (item->HasAttr(APPLY_IMMUNE_FALL))
					SET_BIT(dwImmuneFlag, IMMUNE_FALL);
			}
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	if (IsDragonSoul())
	{
		DSManager::Instance().ActivateDragonSoul(this);
#ifdef ENABLE_DS_SET
		ch->DragonSoul_HandleSetBonus();
#endif
	}
	else
	{
#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
		if (IsRideItem() && ch->GetHorse())
			ch->HorseSummon(false);
#endif

		ModifyPoints(true);
		StartUniqueExpireEvent();
		if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
			StartTimerBasedOnWearExpireEvent();

		// ACCESSORY_REFINE
		StartAccessorySocketExpireEvent();
		// END_OF_ACCESSORY_REFINE
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
		StartAuraBoosterSocketExpireEvent();
#endif
	}

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	LPITEM eqpdFlagweapon = m_pOwner->GetWear(WEAR_WEAPON);
	if (eqpdFlagweapon && IS_SET(eqpdFlagweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON))
	{
		FPartyCHCollector f;

		LPCHARACTER pLeader = m_pOwner;
		LPPARTY pParty = m_pOwner->GetParty();

		if (pParty)
		{
			pLeader->GetParty()->ForEachOnMapMember(f, pLeader->GetParty()->GetLeaderCharacter()->GetMapIndex());
			int cnt = 0;
			for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it)	//@fixme541
			{
				LPCHARACTER partyMember = *it;
				if (partyMember)
				{
					LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
					cnt += (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON) ? 1 : 0);
				}
			}

			if (cnt >= 3)
			{
				for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it)	//@fixme541
				{
					LPCHARACTER partyMember = *it;
					if (!partyMember)
						continue;

					LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
					if (eqpdweapon)
					{
						eqpdweapon->SetForceAttribute(5, 0, 3);
						partyMember->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Equip] You earned %d Group-Points"), 3);
					}
				}
			}
			else if (cnt < 3)
			{
				for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it)	//@fixme541
				{
					LPCHARACTER partyMember = *it;
					LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
					if (eqpdweapon)
					{
						eqpdweapon->SetForceAttribute(5, 0, 0);
						partyMember->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Equip] Missing Group-Members to get Points!"));
					}
				}
			}
		}
		else
		{
			LPITEM eqpdweapon = m_pOwner->GetWear(WEAR_WEAPON);
			if (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON))
			{
				if (eqpdweapon->GetAttributeValue(5) >= 1)
				{
					eqpdweapon->SetForceAttribute(5, 0, 0);
					m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Equip] No Party, Points reset to %d"), 0);
				}
			}
		}
	}
#endif

#ifdef ENABLE_SET_ITEM
	m_pOwner->RefreshSetBonus();
#endif

	ch->BuffOnAttr_AddBuffsFromItem(this);
	m_pOwner->ComputeBattlePoints();
	m_pOwner->UpdatePacket();
	Save();

	return (true);
}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
bool CItem::EquipToDB(LPCHARACTER ch, uint8_t bWearCell)
{
	if (!ch)
	{
		sys_err("EquipToDB: nil character");
		return false;
	}

	// Dragon Soul Slot index is greater than WEAR_MAX_NUM.
	if (IsDragonSoul())
	{
		if (bWearCell < WEAR_MAX_NUM || bWearCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
		{
			sys_err("EquipToDB: invalid dragon soul cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetSubType(), bWearCell - WEAR_MAX_NUM);
			return false;
		}
	}
	else
	{
		if (bWearCell >= WEAR_MAX_NUM)
		{
			sys_err("EquipToDB: invalid wear cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetWearFlag(), bWearCell);
			return false;
		}
	}

	if (ch->GetWearDefault(bWearCell))
	{
		sys_err("EquipToDB: item already exist (this: #%d %s cell: %d %s)", GetOriginalVnum(), GetName(), bWearCell, ch->GetWearDefault(bWearCell)->GetName());
		return false;
	}

	if (GetOwner())
		RemoveFromCharacter();

	ch->SetWearDefault(bWearCell, this); // Packet out here

	m_pOwner = ch;
	m_bEquipped = true;

	m_wCell = bWearCell;

	m_pOwner->UpdatePacket();

	Save();

	return (true);
}

void CItem::RefreshAdditionalEquipmentItems(LPCHARACTER ch, uint8_t cell, bool add)
{
	m_pOwner = ch;

	m_bEquipped = add;

	m_wCell = cell;

	ModifyPoints(add);

	if (add)
	{
#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
		if (IsRideItem() && ch->GetHorse())
			ch->HorseSummon(false);
#endif

		StartUniqueExpireEvent();
		if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
			StartTimerBasedOnWearExpireEvent();

		// ACCESSORY_REFINE
		StartAccessorySocketExpireEvent();
		// END_OF_ACCESSORY_REFINE

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
		StartAuraBoosterSocketExpireEvent();
#endif
		ch->BuffOnAttr_AddBuffsFromItem(this);
	}
	else
	{
#ifndef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
		// Processing when removing new horse items
		if (IsRideItem())
			ClearMountAttributeAndAffect();
#endif

		StopUniqueExpireEvent();

		if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
			StopTimerBasedOnWearExpireEvent();

		// ACCESSORY_REFINE
		StopAccessorySocketExpireEvent();
		// END_OF_ACCESSORY_REFINE

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
		StopAuraBoosterSocketExpireEvent();
#endif

		m_pOwner->BuffOnAttr_RemoveBuffsFromItem(this);

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM // Need Review in Future
		if (IsEquipped() && IsRideItem())
		{
			m_pOwner->MountVnum(m_pOwner->GetPoint(POINT_MOUNT));
			m_pOwner->ComputePoints();
		}
#endif

	}

#ifndef ENABLE_IMMUNE_FIX
	uint32_t dwImmuneFlag = 0;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (m_pOwner->GetWear(i))
		{
			// m_pOwner->ChatPacket(CHAT_TYPE_INFO, "unequip immuneflag(%u)", m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag); // always 0
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

#ifdef ENABLE_SET_ITEM
	m_pOwner->RefreshSetBonus();
#endif
	m_pOwner->ComputeBattlePoints(add);

	if (m_pOwner->GetHP() > m_pOwner->GetMaxHP())
		m_pOwner->PointChange(POINT_HP, m_pOwner->GetMaxHP() - m_pOwner->GetHP());

	if (m_pOwner->GetSP() > m_pOwner->GetMaxSP())
		m_pOwner->PointChange(POINT_SP, m_pOwner->GetMaxSP() - m_pOwner->GetSP());

	m_pOwner->UpdatePacket();

	Save();
}
#endif

bool CItem::Unequip()
{
	if (!m_pOwner || GetCell() > EQUIPMENT_SLOT_COUNT)
	{
		// ITEM_OWNER_INVALID_PTR_BUG
		sys_err("%s %u m_pOwner %p, GetCell %d",
			GetName(), GetID(), get_pointer(m_pOwner), GetCell());
		// END_OF_ITEM_OWNER_INVALID_PTR_BUG
		return false;
	}

	if (this != m_pOwner->GetWear(static_cast<uint8_t>(GetCell())))
	{
		sys_err("m_pOwner->GetWear() != this");
		return false;
	}

#ifndef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
	// Processing when removing new horse items
	if (IsRideItem())
		ClearMountAttributeAndAffect();
#endif

	if (IsDragonSoul())
	{
		DSManager::Instance().DeactivateDragonSoul(this);
	}
	else
	{
		ModifyPoints(false);
	}

	StopUniqueExpireEvent();

	if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
		StopTimerBasedOnWearExpireEvent();

	// ACCESSORY_REFINE
	StopAccessorySocketExpireEvent();
	// END_OF_ACCESSORY_REFINE

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
	StopAuraBoosterSocketExpireEvent();
#endif

#ifdef ENABLE_GROUP_DAMAGE_WEAPON
	LPITEM eqpdFlagweapon = m_pOwner->GetWear(WEAR_WEAPON);
	if (eqpdFlagweapon && IS_SET(eqpdFlagweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON))
	{
		FPartyCHCollector f;
		if (m_pOwner->GetParty())
		{
			m_pOwner->GetParty()->ForEachOnMapMember(f, m_pOwner->GetParty()->GetLeaderCharacter()->GetMapIndex());
			for (std::vector <LPCHARACTER>::iterator it = f.vecCHPtr.begin(); it != f.vecCHPtr.end(); ++it)	//@fixme541
			{
				LPCHARACTER partyMember = *it;
				if (partyMember)
				{
					LPITEM eqpdweapon = partyMember->GetWear(WEAR_WEAPON);
					if (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON))
					{
						if (eqpdweapon->GetAttributeValue(5) >= 1)
						{
							eqpdweapon->SetForceAttribute(5, 0, 0);
							partyMember->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[UnEquip] You Group-Points set to %d"), 0);
						}
					}
				}
			}
		}

		if (!m_pOwner->GetParty())
		{
			LPITEM eqpdweapon = m_pOwner->GetWear(WEAR_WEAPON);
			if (eqpdweapon && IS_SET(eqpdweapon->GetFlag(), ITEM_FLAG_GROUP_DMG_WEAPON))
			{
				if (eqpdweapon->GetAttributeValue(5) >= 1)
				{
					eqpdweapon->SetForceAttribute(5, 0, 0);
					m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[UnEquip] No Party, Points reset to %d"), 0);
				}
			}
		}
	}
#endif

	m_pOwner->BuffOnAttr_RemoveBuffsFromItem(this);

	m_pOwner->SetWear(static_cast<uint8_t>(GetCell()), nullptr);

#ifdef ENABLE_IMMUNE_FIX
	uint32_t dwImmuneFlag = 0;
	LPITEM item = NULL;

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (item = m_pOwner->GetWear(i))
		{
			if (item->GetImmuneFlag() != 0)
				SET_BIT(dwImmuneFlag, item->GetImmuneFlag());
			if (item->GetAttributeCount() > 0)
			{
				if (item->HasAttr(APPLY_IMMUNE_STUN))
					SET_BIT(dwImmuneFlag, IMMUNE_STUN);
				if (item->HasAttr(APPLY_IMMUNE_SLOW))
					SET_BIT(dwImmuneFlag, IMMUNE_SLOW);
				if (item->HasAttr(APPLY_IMMUNE_FALL))
					SET_BIT(dwImmuneFlag, IMMUNE_FALL);
			}
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM // Need Review in Future
	if (IsEquipped() && IsRideItem())
	{
		m_pOwner->MountVnum(m_pOwner->GetPoint(POINT_MOUNT));
		m_pOwner->ComputePoints();
	}
#endif

#ifdef ENABLE_SET_ITEM
	m_pOwner->RefreshSetBonus();
#endif
	m_pOwner->ComputeBattlePoints();
	m_pOwner->UpdatePacket();

	m_pOwner = nullptr;
	m_wCell = 0;
	m_bEquipped = false;

	return true;
}

long CItem::GetValue(uint32_t idx)
{
	assert(idx < ITEM_VALUES_MAX_NUM);
	return GetProto()->alValues[idx];
}

void CItem::SetExchanging(bool bOn)
{
	m_bExchanging = bOn;
}

void CItem::Save()
{
	if (m_bSkipSave)
		return;

	ITEM_MANAGER::Instance().DelayedSave(this);
}

bool CItem::CreateSocket(uint8_t bSlot, uint8_t bGold)
{
	assert(bSlot < ITEM_SOCKET_MAX_NUM);

	if (m_alSockets[bSlot] != 0)
	{
		sys_err("Item::CreateSocket : socket already exist %s %d", GetName(), bSlot);
		return false;
	}

	if (bGold)
		m_alSockets[bSlot] = 2;
	else
		m_alSockets[bSlot] = 1;

	UpdatePacket();

	Save();
	return true;
}

void CItem::SetSockets(const long* c_al)
{
	thecore_memcpy(m_alSockets, c_al, sizeof(m_alSockets));
	Save();
}

void CItem::SetSocket(int i, long v, bool bLog)
{
	assert(i < ITEM_SOCKET_MAX_NUM);
	m_alSockets[i] = v;
	UpdatePacket();
	Save();
	if (bLog)
	{
#ifdef ENABLE_NEWSTUFF
		if (g_iDbLogLevel >= LOG_LEVEL_MAX)
#endif
			LogManager::Instance().ItemLog(i, v, 0, GetID(), "SET_SOCKET", "", "", GetOriginalVnum());
	}
}

#ifdef ENABLE_YOHARA_SYSTEM
// Random Attr
bool CItem::ItemHasApplyRandom()
{
	if (!m_pProto)
		return false;

	for (int i = 0; i < APPLY_RANDOM_SLOT_MAX_NUM; ++i)
	{
		if (m_pProto->aApplies[i].wType == APPLY_RANDOM)
			return true;
	}

	return false;
}

bool CItem::GetApplyRandomData(uint32_t vnum)	//need to be reworked, to use APPLY_RANDOM instead of fixed vnums!
{
	if (!vnum)
		return 0;

	int variable = 0;

	if (vnum >= 81000 && vnum <= 81559)
		variable = 1;

	else if ((vnum >= 14580 && vnum <= 14619) || (vnum >= 16580 && vnum <= 16619) || (vnum >= 17570 && vnum <= 17609))
		variable = 2;

	else if (vnum >= 23000 && vnum <= 23039)
		variable = 3;

	else if (vnum >= 360 && vnum <= 375)	//Schlangenwaffen
		variable = 4;
	else if ((vnum >= 21310 && vnum <= 21325) || (vnum >= 21330 && vnum <= 21345))	//SchlangenRüssis
		variable = 4;

	return variable;
}

uint16_t CItem::GetSungmaUniqueRandomSocket()
{
	std::vector<uint16_t> vSungmaSocketAttr =
	{
		APPLY_SUNGMA_STR, // STR
		APPLY_SUNGMA_HP, // VIT
		APPLY_SUNGMA_MOVE, // RES
		APPLY_SUNGMA_IMMUNE, // INT
	};

	for (int i = 3; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		auto lApplyType = GetSocket(i);

		/*
		* Because there are no more sockets to store the information, I was forced to think
		* of a solution to find out which socket is "boosted" and which is not.
		* So I decided that the "boost" sockets will be the applyType x 10
		*/
		const auto wBoostApplyType = lApplyType / 10;
		if (wBoostApplyType >= APPLY_SUNGMA_STR && wBoostApplyType <= APPLY_SUNGMA_IMMUNE)
			lApplyType = wBoostApplyType;

		vSungmaSocketAttr.erase(std::remove(vSungmaSocketAttr.begin(), vSungmaSocketAttr.end(), lApplyType), vSungmaSocketAttr.end());
	}

	if (vSungmaSocketAttr.empty())
		return 0;

	const int randomSocket = number(0, (vSungmaSocketAttr.size() - 1));
	return vSungmaSocketAttr[randomSocket];
}

// Random Atk
bool CItem::ItemHasRandomDefaultAttr()
{
	for (int i = 0; i < ITEM_RANDOM_VALUES_MAX_NUM; ++i)
	{
		if (m_alRandomValues[i] > 0)
			return true;
	}

	return false;
}

void CItem::SetRandomDefaultAttrs(const uint16_t * c_al)
{
	thecore_memcpy(m_alRandomValues, c_al, sizeof(m_alRandomValues));
	Save();
}

void CItem::SetRandomDefaultAttr(int i, uint16_t v)
{
	assert(i < ITEM_RANDOM_VALUES_MAX_NUM);
	m_alRandomValues[i] = v;
	UpdatePacket();
	Save();
}
#endif

#ifdef ENABLE_PROTO_RENEWAL
int CItem::GetShopBuyPrice()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (GetProto()->dwShopSellPrice == 0)
			return GetCount();
		else
			return GetCount() / GetProto()->dwShopSellPrice;
	}
	else
		return GetProto()->dwShopSellPrice;
}

int CItem::GetShopSellPrice()
{
	return GetProto()->dwShopBuyPrice;
}
#else
int CItem::GetGold()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (GetProto()->dwGold == 0)
			return GetCount();
		else
			return GetCount() / GetProto()->dwGold;
	}
	else
		return GetProto()->dwGold;
}

int CItem::GetShopBuyPrice()
{
	return GetProto()->dwShopBuyPrice;
}
#endif

bool CItem::IsOwnership(LPCHARACTER ch)
{
	if (!ch)
		return false;

	if (!m_pkOwnershipEvent)
		return true;

	return m_dwOwnershipPID == ch->GetPlayerID() ? true : false;
}

EVENTFUNC(ownership_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("ownership_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;
	if (!pkItem)
		return 0;

	pkItem->SetOwnershipEvent(nullptr);

	TPacketGCItemOwnership p{};

	p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID = pkItem->GetVID();
	p.szName[0] = '\0';

	pkItem->PacketAround(&p, sizeof(p));
	return 0;
}

void CItem::SetOwnershipEvent(LPEVENT pkEvent)
{
	m_pkOwnershipEvent = pkEvent;
}

void CItem::SetOwnership(LPCHARACTER ch, int iSec)
{
	if (!ch)
	{
		if (m_pkOwnershipEvent)
		{
			event_cancel(&m_pkOwnershipEvent);
			m_dwOwnershipPID = 0;

			TPacketGCItemOwnership p{};

			p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
			p.dwVID = m_dwVID;
			p.szName[0] = '\0';

			PacketAround(&p, sizeof(p));
		}
		return;
	}

	if (m_pkOwnershipEvent)
		return;

	if (iSec <= 10)
		iSec = OWNERSHIP_EVENT_TIME;

	m_dwOwnershipPID = ch->GetPlayerID();

	item_event_info* info = AllocEventInfo<item_event_info>();
	strlcpy(info->szOwnerName, ch->GetName(), sizeof(info->szOwnerName));
	info->item = this;

	SetOwnershipEvent(event_create(ownership_event, info, PASSES_PER_SEC(iSec)));

	TPacketGCItemOwnership p{};

	p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID = m_dwVID;
	strlcpy(p.szName, ch->GetName(), sizeof(p.szName));

	PacketAround(&p, sizeof(p));
}

int CItem::GetSocketCount()
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		if (GetSocket(i) == 0)
			return i;
	}
	return ITEM_SOCKET_MAX_NUM;
}

bool CItem::AddSocket()
{
	int count = GetSocketCount();
	if (count == ITEM_SOCKET_MAX_NUM)
		return false;
	m_alSockets[count] = 1;
	return true;
}

void CItem::AlterToSocketItem(int iSocketCount)
{
	if (iSocketCount >= ITEM_SOCKET_MAX_NUM)
	{
		sys_log(0, "Invalid Socket Count %d, set to maximum", ITEM_SOCKET_MAX_NUM);
		iSocketCount = ITEM_SOCKET_MAX_NUM;
	}

	for (int i = 0; i < iSocketCount; ++i)
		SetSocket(i, 1);
}

void CItem::AlterToMagicItem()
{
	const int idx = GetAttributeSetIndex();

	if (idx < 0)
		return;

	//		Appeariance	Second	Third
	// Weapon	50		20		5
	// Armor	30		10		2
	// Acc		20		10		1

	int iSecondPct = 0;
	int iThirdPct = 0;

	switch (GetType())
	{
		case ITEM_WEAPON:
			iSecondPct = 20;
			iThirdPct = 5;
			break;

		case ITEM_ARMOR:
		{
			if (GetSubType() == ARMOR_BODY)
			{
				iSecondPct = 10;
				iThirdPct = 2;
			}
			else
			{
				iSecondPct = 10;
				iThirdPct = 1;
			}
		}
		break;

#ifdef ENABLE_USE_COSTUME_ATTR
		case ITEM_COSTUME:
		{
			if (GetSubType() == COSTUME_BODY)
			{
				iSecondPct = 30;
				iThirdPct = 10;
			}
			else if (GetSubType() == COSTUME_HAIR)
			{
				iSecondPct = 30;
				iThirdPct = 10;
			}
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
			else if (GetSubType() == COSTUME_WEAPON)
			{
				iSecondPct = 30;
				iThirdPct = 10;
			}
#endif
			else
			{
				iSecondPct = 0;
				iThirdPct = 0;
			}
		}
		break;
#endif

		default:
			return;
	}

	// 100% chance of one good attribute
	PutAttribute(aiItemMagicAttributePercentHigh);

	if (number(1, 100) <= iSecondPct)
		PutAttribute(aiItemMagicAttributePercentLow);

	if (number(1, 100) <= iThirdPct)
		PutAttribute(aiItemMagicAttributePercentLow);
}

uint32_t CItem::GetRefineFromVnum()
{
	return ITEM_MANAGER::Instance().GetRefineFromVnum(GetVnum());
}

int CItem::GetRefineLevel()
{
	const char* name = GetBaseName();
	char* p = const_cast<char*>(strrchr(name, '+'));

	if (!p)
		return 0;

	int rtn = 0;
	str_to_number(rtn, p + 1);

	const char* locale_name = GetName();
	p = const_cast<char*>(strrchr(locale_name, '+'));

	if (p)
	{
		int locale_rtn = 0;
		str_to_number(locale_rtn, p + 1);
		if (locale_rtn != rtn)
		{
			sys_err("refine_level_based_on_NAME(%d) is not equal to refine_level_based_on_LOCALE_NAME(%d).", rtn, locale_rtn);
		}
	}

	return rtn;
}

#ifdef ENABLE_ANNOUNCEMENT_REFINE_SUCCES
int CItem::GetItemGrade()
{
	const char* c_szItemName = GetBaseName();

	std::string strName = c_szItemName;
	if (strName.empty())
		return 0;

	char chGrade = strName[strName.length() - 1];
	if (chGrade < '0' || chGrade > '9')
		chGrade = '0';

	char chGrade_ex = strName[strName.length() - 2];

	int iGrade;
	if (chGrade >= '0' && chGrade <= '9' && chGrade_ex != '1')
		iGrade = chGrade - '0';
	else if (chGrade_ex == '1' && (chGrade >= '0' && chGrade <= '5'))
		iGrade = 10 + (chGrade - '0');

	return iGrade;
}
#endif

bool CItem::IsPolymorphItem()
{
	return GetType() == ITEM_POLYMORPH;
}

EVENTFUNC(unique_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("unique_expire_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;
	if (!pkItem)
		return 0;

	if (pkItem->GetValue(2) == 0)
	{
		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= 1)
		{
			sys_log(0, "UNIQUE_ITEM: expire %s %u", pkItem->GetName(), pkItem->GetID());
			pkItem->SetUniqueExpireEvent(nullptr);
			ITEM_MANAGER::Instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			pkItem->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - 1);
			return PASSES_PER_SEC(60);
		}
	}
	else
	{
		const time_t cur = get_global_time();

		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= cur)
		{
			pkItem->SetUniqueExpireEvent(nullptr);
			ITEM_MANAGER::Instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			// There is a bug in the game where time-based items do not disappear quickly
			// correction
			// by rtsummit
			if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur < 600)
				return PASSES_PER_SEC(pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur);
			else
				return PASSES_PER_SEC(600);
		}
	}
}

// Pay after time
// When starting the timer, it does not deduct
// When the timer fires, the time is deducted by the time the timer was operated.
EVENTFUNC(timer_based_on_wear_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("expire_event <Factor> Null pointer");
		return 0;
	}

	LPITEM pkItem = info->item;
	if (!pkItem)
		return 0;

	const int remain_time = pkItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) - processing_time / passes_per_sec;
	if (remain_time <= 0)
	{
		sys_log(0, "ITEM EXPIRED : expired %s %u", pkItem->GetName(), pkItem->GetID());
		pkItem->SetTimerBasedOnWearExpireEvent(nullptr);
		pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, 0);

		// Once timer based on wear dragon soul stone does not get rid of when time runs out.
		if (pkItem->IsDragonSoul())
		{
			DSManager::Instance().DeactivateDragonSoul(pkItem);
		}
		else
		{
			ITEM_MANAGER::Instance().RemoveItem(pkItem, "TIMER_BASED_ON_WEAR_EXPIRE");
		}
		return 0;
	}
	pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	return PASSES_PER_SEC(MIN(60, remain_time));
}

void CItem::SetUniqueExpireEvent(LPEVENT pkEvent)
{
	m_pkUniqueExpireEvent = pkEvent;
}

void CItem::SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent)
{
	m_pkTimerBasedOnWearExpireEvent = pkEvent;
}

EVENTFUNC(real_time_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (nullptr == info)
		return 0;

	const LPITEM item = ITEM_MANAGER::Instance().FindByVID(info->item_vid);

	if (nullptr == item)
		return 0;

	const time_t current = get_global_time();

	if (current > item->GetSocket(0))
	{
#ifndef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
		if (item->GetVnum() && item->IsNewMountItem())
			item->ClearMountAttributeAndAffect();
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		//@fixme468
		if (item->GetType() == ITEM_WEAPON) {
			if (item->GetOwner() && item->GetOwner()->GetWear(WEAR_COSTUME_WEAPON) != nullptr)
				item->GetOwner()->UnequipItem(item->GetOwner()->GetWear(WEAR_COSTUME_WEAPON));
		}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		if (item->GetType() == ITEM_PET && ((item->GetSubType() == PET_UPBRINGING) || item->GetSubType() == PET_BAG))
			return 0;
#endif

		ITEM_MANAGER::Instance().RemoveItem(item, "REAL_TIME_EXPIRE");

		return 0;
	}

	return PASSES_PER_SEC(1);
}

void CItem::StartRealTimeExpireEvent()
{
	if (m_pkRealTimeExpireEvent)
		return;
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType || LIMIT_REAL_TIME_START_FIRST_USE == GetProto()->aLimits[i].bType)
		{
			item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
			info->item_vid = GetVID();

			m_pkRealTimeExpireEvent = event_create(real_time_expire_event, info, PASSES_PER_SEC(1));

			sys_log(0, "REAL_TIME_EXPIRE: StartRealTimeExpireEvent");

			return;
		}
	}
}

#if defined(ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
EVENTFUNC(change_look_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (nullptr == info)
		return 0;

	const LPITEM item = ITEM_MANAGER::Instance().FindByVID(info->item_vid);

	if (nullptr == item)
		return 0;

	if ((item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_MOUNT) || (item->IsHorseSummonItem()))
	{
		if (item->GetSocket(2) < get_global_time())
			item->StopChangeLookExpireEvent();
	}

	const int iSec = test_server ? 1 : 60;
	return PASSES_PER_SEC(iSec);
}

void CItem::StartChangeLookExpireEvent()
{
	if (m_pkChangeLookExpireEvent)
		return;

	if (((GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT) || (IsHorseSummonItem())) && (GetChangeLookVnum() && GetSocket(2) != 0))
	{
		const int iSec = test_server ? 1 : 60;
		item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
		info->item_vid = GetVID();
		m_pkChangeLookExpireEvent = event_create(change_look_expire_event, info, PASSES_PER_SEC(iSec));

		sys_log(0, "CHANGE_LOOK: StartChangeLookExpireEvent");
		return;
	}
}

void CItem::StopChangeLookExpireEvent()
{
	if (!m_pkChangeLookExpireEvent)
		return;

	if ((GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT) || (IsHorseSummonItem()))
	{
		SetChangeLookVnum(0);
		SetSocket(2, 0);
	}

	event_cancel(&m_pkChangeLookExpireEvent);
	//ITEM_MANAGER::Instance().SaveSingleItem(this);
}

bool CItem::IsExpireTimeItem()
{
	if (!GetProto())
		return false;

	if (GetType() != ITEM_COSTUME && GetSubType() != COSTUME_MOUNT)
		return false;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if ((LIMIT_REAL_TIME == GetProto()->aLimits[i].bType) || (LIMIT_TIMER_BASED_ON_WEAR == GetProto()->aLimits[i].bType) || (LIMIT_REAL_TIME_START_FIRST_USE == GetProto()->aLimits[i].bType))
			return true;
	}
	return false;
}

long CItem::GetRealExpireTime()
{
	if (!GetProto())
		return false;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		switch (GetProto()->aLimits[i].bType)
		{
			case LIMIT_REAL_TIME:
				return GetSocket(0);
			case LIMIT_TIMER_BASED_ON_WEAR:
				return GetSocket(0) + time(0);
			case LIMIT_REAL_TIME_START_FIRST_USE:
				return GetSocket(0) != 0 ? GetSocket(0) : GetProto()->aLimits[i].lValue + time(0);
			default:
				break;
		}
	}

	return 0;
}
#endif

#ifdef ENABLE_SOUL_SYSTEM
EVENTFUNC(soul_time_use_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (nullptr == info)
		return 0;

	const LPITEM item = ITEM_MANAGER::Instance().FindByVID(info->item_vid);

	if (nullptr == item)
		return 0;

	item->SetSoulKeepTime();

	const int iSec = test_server ? 1 : 60;
	return PASSES_PER_SEC(iSec);
}

void CItem::StartSoulTimeUseEvent()
{
	if (m_pkSoulTimeUseEvent)
		return;

	if (GetType() == ITEM_SOUL)
	{
		const int iSec = test_server ? 1 : 60;
		item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
		info->item_vid = GetVID();
		m_pkSoulTimeUseEvent = event_create(soul_time_use_event, info, PASSES_PER_SEC(iSec));

		sys_log(0, "SOUL_TIME_USE: StartSoulTimeUseEvent");

		return;
	}
}

void CItem::SetSoulKeepTime()
{
	if (GetType() != ITEM_SOUL)
		return;

	const LPCHARACTER& ch = GetOwner();
	if (!ch)
		return;

	uint32_t dwItemPlayTime = GetSocket(3);
	const uint32_t dwMaxTime = GetLimitValue(1);
	const int iSoulAttacks = GetValue(2);

	if (dwItemPlayTime < dwMaxTime)
	{
		dwItemPlayTime += 1;

		const uint32_t dwUseData = (dwItemPlayTime * 10000) + iSoulAttacks;
		SetSocket(2, dwUseData);
		SetSocket(3, dwItemPlayTime);
	}
	else
	{
		event_cancel(&m_pkSoulTimeUseEvent);
		return;
	}
}
#endif

bool CItem::IsRealTimeItem()
{
	if (!GetProto())
		return false;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return true;
	}

	return false;
}

void CItem::StartUniqueExpireEvent()
{
	if (GetType() != ITEM_UNIQUE)
		return;

	if (m_pkUniqueExpireEvent)
		return;

	//For fixed-term items, time-based items do not work.
	if (IsRealTimeItem())
		return;

	// HARD CODING
	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(false);

	int iSec = GetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME);

	if (iSec == 0)
		iSec = 60;
	else
		iSec = MIN(iSec, 60);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, 0);

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetUniqueExpireEvent(event_create(unique_expire_event, info, PASSES_PER_SEC(iSec)));
}

// Pay after time
// see timer_based_on_wear_expire_event description
void CItem::StartTimerBasedOnWearExpireEvent()
{
	if (m_pkTimerBasedOnWearExpireEvent)
		return;

	//In case of a fixed-term item, the part-time item does not work.
	if (IsRealTimeItem())
		return;

	if (-1 == GetProto()->cLimitTimerBasedOnWearIndex)
		return;

	int iSec = GetSocket(0);

	// To cut off the remaining time in minutes...
	if (0 != iSec)
	{
		iSec %= 60;
		if (0 == iSec)
			iSec = 60;
	}

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetTimerBasedOnWearExpireEvent(event_create(timer_based_on_wear_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopUniqueExpireEvent()
{
	if (!m_pkUniqueExpireEvent)
		return;

	if (GetValue(2) != 0) // Items other than game-timed cannot stop UniqueExpireEvent.
		return;

	// HARD CODING
	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(true);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, event_time(m_pkUniqueExpireEvent) / passes_per_sec);
	event_cancel(&m_pkUniqueExpireEvent);

	ITEM_MANAGER::Instance().SaveSingleItem(this);
}

void CItem::StopTimerBasedOnWearExpireEvent()
{
	if (!m_pkTimerBasedOnWearExpireEvent)
		return;

	const int remain_time = GetSocket(ITEM_SOCKET_REMAIN_SEC) - event_processing_time(m_pkTimerBasedOnWearExpireEvent) / passes_per_sec;

	SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);

	ITEM_MANAGER::Instance().SaveSingleItem(this);
}

void CItem::ApplyAddon(int iAddonType)
{
	CItemAddonManager::Instance().ApplyAddonTo(iAddonType, this);
}

int CItem::GetSpecialGroup() const
{
	return ITEM_MANAGER::Instance().GetSpecialGroupFromItem(GetVnum());
}

//
// Accessory socket treatment.
//
bool CItem::IsAccessoryForSocket()
{
	return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR)) ||
		(m_pProto->bType == ITEM_BELT); // In the case of the'Belt' item newly added in February 2013, the planning team suggested using the accessory socket system as it is.
}

void CItem::SetAccessorySocketGrade(int iGrade)
{
	SetSocket(0, MINMAX(0, iGrade, GetAccessorySocketMaxGrade()));

	const int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

	//if (test_server)
	//	iDownTime /= 60;

	SetAccessorySocketDownGradeTime(iDownTime);
}

void CItem::SetAccessorySocketMaxGrade(int iMaxGrade)
{
	SetSocket(1, MINMAX(0, iMaxGrade, ITEM_ACCESSORY_SOCKET_MAX_NUM));
}

void CItem::SetAccessorySocketDownGradeTime(uint32_t time)
{
	SetSocket(2, time);

	if (test_server && GetOwner())
		GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s¿¡¼­ ¼ÒÄÏ ºüÁú¶§±îÁö ³²Àº ½Ã°£ %d"), GetName(), time);
}

EVENTFUNC(accessory_socket_expire_event)
{
	const item_vid_event_info* info = dynamic_cast<item_vid_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("accessory_socket_expire_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM item = ITEM_MANAGER::Instance().FindByVID(info->item_vid);
	if (!item)
		return 0;

	if (item->GetAccessorySocketDownGradeTime() <= 1)
	{
	degrade:
		item->SetAccessorySocketExpireEvent(nullptr);
		item->AccessorySocketDegrade();
		return 0;
	}
	else
	{
		const int iTime = item->GetAccessorySocketDownGradeTime() - 60;

		if (iTime <= 1)
			goto degrade;

		item->SetAccessorySocketDownGradeTime(iTime);

		if (iTime > 60)
			return PASSES_PER_SEC(60);
		else
			return PASSES_PER_SEC(iTime);
	}
}

void CItem::StartAccessorySocketExpireEvent()
{
	if (!IsAccessoryForSocket())
		return;

	if (m_pkAccessorySocketExpireEvent)
		return;

	if (GetAccessorySocketMaxGrade() == 0)
		return;

	if (GetAccessorySocketGrade() == 0)
		return;

	int iSec = GetAccessorySocketDownGradeTime();
	SetAccessorySocketExpireEvent(nullptr);

	if (iSec <= 1)
		iSec = 5;
	else
		iSec = MIN(iSec, 60);

	item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
	info->item_vid = GetVID();

	SetAccessorySocketExpireEvent(event_create(accessory_socket_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopAccessorySocketExpireEvent()
{
	if (!m_pkAccessorySocketExpireEvent)
		return;

	if (!IsAccessoryForSocket())
		return;

	const int new_time = GetAccessorySocketDownGradeTime() - (60 - event_time(m_pkAccessorySocketExpireEvent) / passes_per_sec);

	event_cancel(&m_pkAccessorySocketExpireEvent);

	if (new_time <= 1)
	{
		AccessorySocketDegrade();
	}
	else
	{
		SetAccessorySocketDownGradeTime(new_time);
	}
}

bool CItem::IsRideItem()
{
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType())
		return true;
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType())
		return true;
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType()
#	ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
		&& GetApplyType(0) == APPLY_MOUNT && GetApplyValue(0) > 0
#	endif
		)
		return true;
#endif
	return false;
}

bool CItem::IsRamadanRing()
{
	if (GetVnum() == UNIQUE_ITEM_RAMADAN_RING)
		return true;
	if (GetVnum() == UNIQUE_ITEM_RAMADAN_RING_PLUS)
		return true;
	return false;
}

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
uint32_t CItem::GetMountVnum()
{
#if defined(ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
	if (GetChangeLookVnum())
	{
		const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(GetChangeLookVnum());
		if (!pProto)
			return 0;

		return pProto->aApplies[0].lValue;
	}
#endif

	return GetApplyValue(0);
}
#else
void CItem::ClearMountAttributeAndAffect()
{
	LPCHARACTER ch = GetOwner();
	if (!ch) //@fixme186
		return;

	ch->RemoveAffect(AFFECT_MOUNT);
	ch->RemoveAffect(AFFECT_MOUNT_BONUS);

	ch->MountVnum(0);

	ch->PointChange(POINT_ST, 0);
	ch->PointChange(POINT_DX, 0);
	ch->PointChange(POINT_HT, 0);
	ch->PointChange(POINT_IQ, 0);
}

// fixme
// I don't use this right now... But I left it just in case.
// by rtsummit
bool CItem::IsNewMountItem()
{
	return (
		(ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType() && IS_SET(GetFlag(), ITEM_FLAG_QUEST_USE))
		|| (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType() && IS_SET(GetFlag(), ITEM_FLAG_QUEST_USE))
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		|| (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType()
#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
			&& GetApplyType(0) == APPLY_MOUNT && GetApplyValue(0) > 0
#endif
			)
#endif
		); // @fixme152
}
#endif

void CItem::SetAccessorySocketExpireEvent(LPEVENT pkEvent)
{
	m_pkAccessorySocketExpireEvent = pkEvent;
}

void CItem::AccessorySocketDegrade()
{
	if (GetAccessorySocketGrade() > 0)
	{
		LPCHARACTER ch = GetOwner();

		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;471;%s]", GetName());
		}

		ModifyPoints(false);
		SetAccessorySocketGrade(GetAccessorySocketGrade() - 1);
		ModifyPoints(true);

		int iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

		if (test_server)
			iDownTime /= 60;

		SetAccessorySocketDownGradeTime(iDownTime);

		if (iDownTime)
			StartAccessorySocketExpireEvent();
	}
}

// Checks whether an item can be inserted into the ring and returns it
static const bool CanPutIntoRing(LPITEM ring, LPITEM item)
{
	//const uint32_t vnum = item->GetVnum();
	return false;
}

bool CItem::CanPutInto(LPITEM item)
{
	if (!item)
		return false;

	if (item->GetType() == ITEM_BELT)
		return this->GetSubType() == USE_PUT_INTO_BELT_SOCKET;

	else if (item->GetType() == ITEM_RING)
		return CanPutIntoRing(item, this);

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
	else if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_AURA)
		return this->GetSubType() == USE_PUT_INTO_AURA_SOCKET;
#endif

	else if (item->GetType() != ITEM_ARMOR)
		return false;

	uint32_t vnum = item->GetVnum();

	struct JewelAccessoryInfo
	{
		uint32_t jewel;
		uint32_t wrist;
		uint32_t neck;
		uint32_t ear;
	};
	const static JewelAccessoryInfo infos[] =
	{
		{ 50634, 14420, 16220, 17220 },	//Seelenkristall
		{ 50635, 14500, 16500, 17500 },	//Rubin
		{ 50636, 14520, 16520, 17520 },	//Granat
		{ 50637, 14540, 16540, 17540 },	//Smaragd
		{ 50638, 14560, 16560, 17560 },	//Saphir
		{ 50639, 14570, 16570, 17610 },	//Turmalin
#ifdef ENABLE_YOHARA_SYSTEM
		{ 50661, 14590, 16590, 17580 },	//Sonnenjedit
		{ 50662, 14600, 16600, 17590 },	//Nachtjedit
		{ 50663, 14610, 16610, 17600 },	//Rauchjedit
		{ 50664, 14580, 16580, 17570 },	//Nebeljedit
#endif
	};

	const uint32_t item_type = (item->GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infos) / sizeof(infos[0]); i++)
	{
		const JewelAccessoryInfo& info = infos[i];
		switch (item->GetSubType())
		{
			case ARMOR_WRIST:
			{
				if (info.wrist == item_type)
				{
					if (info.jewel == GetVnum())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			break;

			case ARMOR_NECK:
			{
				if (info.neck == item_type)
				{
					if (info.jewel == GetVnum())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			break;

			case ARMOR_EAR:
			{
				if (info.ear == item_type)
				{
					if (info.jewel == GetVnum())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			break;

			default:
				break;
		}
	}

	if (item->GetSubType() == ARMOR_WRIST)
		vnum -= 14000;
	else if (item->GetSubType() == ARMOR_NECK)
		vnum -= 16000;
	else if (item->GetSubType() == ARMOR_EAR)
		vnum -= 17000;
	else
		return false;

	uint32_t type = vnum / 20;

	if (type < 0 || type > 11)
	{
		type = (vnum - 170) / 20;

		if (50623 + type != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219)
	{
		if (50625 != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239)
	{
		if (50626 != GetVnum())
			return false;
		else
			return true;
	}

	return 50623 + type == GetVnum();
}

bool CItem::CheckItemUseLevel(int nLevel)
{
	// Nyx: Not remove item if is test server
	if (test_server)
		return true;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			if (this->m_pProto->aLimits[i].lValue > nLevel)
				return false;
			else
				return true;
		}
	}
	return true;
}

long CItem::FindApplyValue(uint16_t wApplyType)	//@fixme532
{
	if (m_pProto == nullptr)
		return 0;

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (m_pProto->aApplies[i].wType == wApplyType)
			return m_pProto->aApplies[i].lValue;
	}

	return 0;
}

void CItem::CopySocketTo(LPITEM pItem)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		pItem->m_alSockets[i] = m_alSockets[i];
	}
}

int CItem::GetAccessorySocketGrade()
{
	return MINMAX(0, GetSocket(0), GetAccessorySocketMaxGrade());
}

int CItem::GetAccessorySocketMaxGrade()
{
	return MINMAX(0, GetSocket(1), ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int CItem::GetAccessorySocketDownGradeTime()
{
	return MINMAX(0, GetSocket(2), aiAccessorySocketDegradeTime[GetAccessorySocketGrade()]);
}

void CItem::AttrLog()
{
	const char* pszIP = nullptr;

	if (GetOwner() && GetOwner()->GetDesc())
		pszIP = GetOwner()->GetDesc()->GetHostName();

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (m_alSockets[i])
		{
#ifdef ENABLE_NEWSTUFF
			if (g_iDbLogLevel >= LOG_LEVEL_MAX)
#endif
				LogManager::Instance().ItemLog(i, m_alSockets[i], 0, GetID(), "INFO_SOCKET", "", pszIP ? pszIP : "", GetOriginalVnum());
		}
	}

	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		const int type = m_aAttr[i].wType;
		const int value = m_aAttr[i].sValue;

		if (type)
		{
#ifdef ENABLE_NEWSTUFF
			if (g_iDbLogLevel >= LOG_LEVEL_MAX)
#endif
				LogManager::Instance().ItemLog(i, type, value, GetID(), "INFO_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum());
		}
	}
}

int CItem::GetLevelLimit()
{
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			return this->m_pProto->aLimits[i].lValue;
		}
	}
	return 0;
}

bool CItem::OnAfterCreatedItem()
{
	// If an item is used at least once, the time is deducted even if it is not being used after that
	if (-1 != this->GetProto()->cLimitRealTimeFirstUseIndex)
	{
		// Since the number of times an item has been used is recorded in Socket1, an item that has been used even once starts a timer.
		if (0 != GetSocket(1))
		{
			StartRealTimeExpireEvent();
		}
	}

#if defined(ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
	if (IsHorseSummonItem() && GetChangeLookVnum() && GetSocket(2))
		StartChangeLookExpireEvent();
#endif

#ifdef ENABLE_SOUL_SYSTEM
	if (GetType() == ITEM_SOUL)
		StartSoulTimeUseEvent();
#endif

	return true;
}

bool CItem::IsDragonSoul()
{
	return GetType() == ITEM_DS;
}

int CItem::GiveMoreTime_Per(float fPercent)
{
	if (IsDragonSoul())
	{
		const uint32_t duration = DSManager::Instance().GetDuration(this);
		const uint32_t remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		const uint32_t given_time = static_cast<uint32_t>(fPercent * duration / 100u);
		if (remain_sec == duration)
			return false;
		if ((given_time + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, given_time + remain_sec);
			return given_time;
		}
	}
	// First of all, let's talk about the Dragon Soul Stone.
	else
		return 0;
}

int CItem::GiveMoreTime_Fix(uint32_t dwTime)
{
	if (IsDragonSoul())
	{
		const uint32_t duration = DSManager::Instance().GetDuration(this);
		const uint32_t remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		if (remain_sec == duration)
			return false;
		if ((dwTime + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remain_sec);
			return dwTime;
		}
	}
	// First of all, let's talk about the Dragon Soul Stone.
	else
		return 0;
}


int CItem::GetDuration()
{
	if (!GetProto())
		return -1;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return GetProto()->aLimits[i].lValue;
	}

	if (GetProto()->cLimitTimerBasedOnWearIndex >= 0)
	{
		const uint8_t cLTBOWI = GetProto()->cLimitTimerBasedOnWearIndex;
		return GetProto()->aLimits[cLTBOWI].lValue;
	}

	return -1;
}

bool CItem::IsSameSpecialGroup(const LPITEM item) const
{
	// If they have the same VNUM, they are considered to be in the same group.
	if (this->GetVnum() == item->GetVnum())
		return true;

	if (GetSpecialGroup() && (item->GetSpecialGroup() == GetSpecialGroup()))
		return true;

	return false;
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CItem::SetChangeLookVnum(uint32_t blVnum)
{
	m_dwTransmutationVnum = blVnum;
	Save();
}

uint32_t CItem::GetChangeLookVnum() const
{
	return m_dwTransmutationVnum;
}
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
int CItem::GetSpecialInventoryType()
{
	switch (GetVnum())
	{
		case 27987: //Muschel
			return SPECIAL_INVENTORY_TYPE_MATERIAL;
	}

	switch (GetType())
	{
		case ITEM_SKILLBOOK:
			return SPECIAL_INVENTORY_TYPE_SKILLBOOK;

		case ITEM_METIN:
			return SPECIAL_INVENTORY_TYPE_STONE;

		case ITEM_MATERIAL:
		case ITEM_RESOURCE:
			return SPECIAL_INVENTORY_TYPE_MATERIAL;

		default:
			return -1;
	}
}
#endif

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
const char* CItem::GetName()
{
	if (m_pProto == nullptr)
		return "Unknown";

	static char szItemName[256];
	memset(szItemName, 0, sizeof(szItemName));
	if (GetProto())
	{
		int len = 0;
		switch (GetType())
		{
			case ITEM_POLYMORPH:
			{
				const uint32_t dwMobVnum = GetSocket(0);
				const CMob* pMob = CMobManager::instance().Get(dwMobVnum);
				if (pMob)
					len = snprintf(szItemName, sizeof(szItemName), "%s", pMob->m_table.szLocaleName);

				break;
			}

			case ITEM_SKILLBOOK:
			case ITEM_SKILLFORGET:
			{
				const uint32_t dwSkillVnum = (GetVnum() == ITEM_SKILLBOOK_VNUM || GetVnum() == ITEM_SKILLFORGET_VNUM) ? GetSocket(0) : 0;
				const CSkillProto* pSkill = (dwSkillVnum != 0) ? CSkillManager::instance().Get(dwSkillVnum) : NULL;
				if (pSkill)
					len = snprintf(szItemName, sizeof(szItemName), "%s", pSkill->szName);

				break;
			}
		}

		len += snprintf(szItemName + len, sizeof(szItemName) - len, (len>0)?" %s":"%s", GetProto()->szLocaleName);
	}

	return szItemName;
}
#endif

bool CItem::CanStackWith(LPITEM other)
{
	bool can = other && other != this && other->GetVnum() == GetVnum() && other->IsStackable() && !IsExchanging() && !other->IsExchanging();
	return can && IsSameItem(*this, *other);
}

bool CItem::IsSameItem(CItem o, CItem t)
{
	//Make sure vnum matches
	bool matched = o.GetVnum() == t.GetVnum();

	// Verify sockets (until it no longer matches or we're done with all sockets)
	for (int j = 0; j < ITEM_SOCKET_MAX_NUM && matched; ++j)
		matched = o.GetSocket(j) == t.GetSocket(j);

	return matched;
}

#ifdef ENABLE_SEALBIND_SYSTEM
bool CItem::IsSealed()
{
	if (m_sealDate == -1)
		return true;

	if (m_sealDate > get_global_time())
		return true;

	return false;
}

bool CItem::IsSealAble() const
{
	if (GetType() == ITEM_WEAPON)	//Waffen
		return GetSubType() != WEAPON_ARROW && GetSubType() != WEAPON_QUIVER;

	if (GetType() == ITEM_RING)		// Ringe
		return true;

	if (GetType() == ITEM_DS)		// Drachensteine
		return true;

	if (GetType() == ITEM_COSTUME)	//Kostüme
		return true;

	if (GetType() == ITEM_QUEST)	// Aufträge
		return true;

	return GetType() == ITEM_ARMOR || GetType() == ITEM_BELT;
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
void CItem::SetElement(uint8_t value_grade_element, const uint32_t* attack_element, uint8_t element_type_bonus, const int16_t* elements_value_bonus)
{
	m_grade_element = value_grade_element;
	thecore_memcpy(m_attack_element, attack_element, sizeof(m_attack_element));
	m_element_type_bonus = element_type_bonus;
	thecore_memcpy(m_elements_value_bonus, elements_value_bonus, sizeof(m_elements_value_bonus));
	UpdatePacket();
	Save();
}

void CItem::SetElementNew(uint8_t value_grade_element, uint32_t attack_element, uint8_t element_type_bonus, int16_t elements_value_bonus)
{
	m_grade_element = value_grade_element;
	m_attack_element[value_grade_element - 1] = attack_element;
	m_element_type_bonus = element_type_bonus;
	m_elements_value_bonus[value_grade_element - 1] = elements_value_bonus;
	UpdatePacket();
	Save();
}

void CItem::DeleteAllElement(uint8_t value_grade_element)
{
	m_grade_element = value_grade_element;
	m_attack_element[value_grade_element] = 0;
	m_elements_value_bonus[value_grade_element] = 0;
	if (value_grade_element == 0)
	{
		m_element_type_bonus = 0;
	}
	UpdatePacket();
	Save();
}

void CItem::ChangeElement(uint8_t element_type_bonus)
{
	m_element_type_bonus = element_type_bonus;
	UpdatePacket();
	Save();
}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
long CItem::DrainedValue(long value)
{
	return static_cast<long>(floor(static_cast<float>(value) * (GetDrainPercentage()) / 100.0f));
}

float CItem::GetDrainPercentage()
{
	return static_cast<float>(MINMAX(1, GetSocket(1), 25)); // ACCE_SOCKET_DRAIN_PCT
}

TItemTable* CItem::GetAbsorbedItem()
{
	if (GetSocket(0) < 1) // ACCE_SOCKET_ITEM_VNUM
		return nullptr;

	return ITEM_MANAGER::Instance().GetTable(GetSocket(0)); // ACCE_SOCKET_ITEM_VNUM
}
#endif

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
bool CItem::IsAuraBoosterForSocket()
{
	if (m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA)
		return this->GetSocket(ITEM_SOCKET_AURA_BOOST) == 0;

	return false;
}

EVENTFUNC(aura_boost_socket_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>(event->info);
	if (info == nullptr)
	{
		sys_err("aura_boost_socket_expire_event> <Factor> Null pointer");
		return 0;
	}

	LPITEM pkAura = info->item;
	if (!pkAura)
		return 0;

	const long c_lBoosterSocket = pkAura->GetSocket(ITEM_SOCKET_AURA_BOOST);
	const long c_lBoosterIndex = c_lBoosterSocket / 100000000;
	if (c_lBoosterIndex <= ITEM_AURA_BOOST_ERASER || c_lBoosterIndex >= ITEM_AURA_BOOST_MAX)
		return 0;

	const uint32_t dwBoosterItemVnum = ITEM_AURA_BOOST_ITEM_VNUM_BASE + c_lBoosterIndex;
	const TItemTable* pBoosterProto = ITEM_MANAGER::Instance().GetTable(dwBoosterItemVnum);
	if (!pBoosterProto)
		return 0;

	if (pBoosterProto->alValues[ITEM_AURA_BOOST_UNLIMITED_VALUE] == 1)
		return 0;

	const long lBoostTimeSec = (c_lBoosterSocket % 100000000) - processing_time / passes_per_sec;
	if (lBoostTimeSec <= 0)
	{
		sys_log(0, "AURA BOOST EXPIRED : expired %s %lu", pkAura->GetName(), pkAura->GetID());
		pkAura->SetAuraBoosterSocketExpireEvent(nullptr);
		pkAura->ModifyPoints(false);
		pkAura->SetSocket(ITEM_SOCKET_AURA_BOOST, 0);
		pkAura->ModifyPoints(true);
		if (pkAura->GetOwner())
		{
			pkAura->GetOwner()->ComputeBattlePoints();
			pkAura->GetOwner()->UpdatePacket();
		}

		LogManager::Instance().ItemLog(pkAura->GetOwner(), pkAura, "AURA_BOOST_EXPIRED", pkAura->GetName());

		return 0;
	}

	pkAura->SetSocket(ITEM_SOCKET_AURA_BOOST, c_lBoosterIndex * 100000000 + lBoostTimeSec);
	return PASSES_PER_SEC(MIN(60, lBoostTimeSec));
}

void CItem::StartAuraBoosterSocketExpireEvent()
{
	if (!(m_pProto->bType == ITEM_COSTUME && m_pProto->bSubType == COSTUME_AURA))
		return;

	if (GetSocket(ITEM_SOCKET_AURA_BOOST) == 0)
		return;

	if (m_pkAuraBoostSocketExpireEvent)
		return;

	const long c_lBoosterSocket = GetSocket(ITEM_SOCKET_AURA_BOOST);
	const long c_lBoosterIndex = c_lBoosterSocket / 100000000;
	if (c_lBoosterIndex <= ITEM_AURA_BOOST_ERASER || c_lBoosterIndex >= ITEM_AURA_BOOST_MAX)
		return;

	const uint32_t dwBoosterItemVnum = ITEM_AURA_BOOST_ITEM_VNUM_BASE + c_lBoosterIndex;
	const TItemTable* pBoosterProto = ITEM_MANAGER::Instance().GetTable(dwBoosterItemVnum);
	if (!pBoosterProto)
		return;

	if (pBoosterProto->alValues[ITEM_AURA_BOOST_UNLIMITED_VALUE] == 1)
		return;

	long lBoostTimeSec = c_lBoosterSocket % 100000000;
	if (0 != lBoostTimeSec)
	{
		lBoostTimeSec %= 60;
		if (0 == lBoostTimeSec)
			lBoostTimeSec = 60;
	}

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetAuraBoosterSocketExpireEvent(event_create(aura_boost_socket_expire_event, info, PASSES_PER_SEC(lBoostTimeSec)));
}

void CItem::StopAuraBoosterSocketExpireEvent()
{
	if (!m_pkAuraBoostSocketExpireEvent)
		return;

	const long c_lBoosterSocket = GetSocket(ITEM_SOCKET_AURA_BOOST);
	const long c_lBoosterIndex = c_lBoosterSocket / 100000000;
	if (c_lBoosterIndex <= ITEM_AURA_BOOST_ERASER || c_lBoosterIndex >= ITEM_AURA_BOOST_MAX)
		return;

	const uint32_t dwBoosterItemVnum = ITEM_AURA_BOOST_ITEM_VNUM_BASE + c_lBoosterIndex;
	const TItemTable* pBoosterProto = ITEM_MANAGER::Instance().GetTable(dwBoosterItemVnum);
	if (!pBoosterProto)
		return;

	const long lBoostTimeSec = (c_lBoosterSocket % 100000000) - event_processing_time(m_pkAuraBoostSocketExpireEvent) / passes_per_sec;
	SetSocket(ITEM_SOCKET_AURA_BOOST, c_lBoosterIndex * 100000000 + lBoostTimeSec);
	event_cancel(&m_pkAuraBoostSocketExpireEvent);

	ITEM_MANAGER::Instance().SaveSingleItem(this);
}

void CItem::SetAuraBoosterSocketExpireEvent(LPEVENT pkEvent)
{
	m_pkAuraBoostSocketExpireEvent = pkEvent;
}
#endif
