#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "questmanager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "locale_service.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "desc_client.h"
#endif
#ifdef ENABLE_12ZI
# include "zodiac_temple.h"
#endif

extern bool trade_effect;
extern int trade_effect_shop_threshold;
extern bool g_bEmpireShopPriceTripleDisable;

/* ------------------------------------------------------------------------------------ */
CShop::CShop()
	: m_dwVnum(0),
	m_dwNPCVnum(0),
#ifdef ENABLE_12ZI
	m_IsLimitedItemShop(false),
#endif
	m_pkPC(nullptr)
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	, m_closed(false),
	m_openTime(0),
	m_offlineMinutes(0)
	, m_renamePulse(0)
#endif
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	, isShopDeco(0)
#endif
{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	m_pGrid = M2_NEW CGrid(SHOP_GRID_WIDTH, SHOP_GRID_HEIGHT);
#else
	m_pGrid = M2_NEW CGrid(5, 9);
#endif
}

CShop::~CShop()
{
	TPacketGCShop pack{};

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;
		if (ch)
		{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			ch->SetViewingShop(nullptr);
#else
			ch->SetShop(nullptr);
#endif
		}
		++it;
	}

	M2_DELETE(m_pGrid);
}

void CShop::SetPCShop(LPCHARACTER ch)
{
	m_pkPC = ch;
}

bool CShop::Create(uint32_t dwVnum, uint32_t dwNPCVnum, TShopItemTable* pTable
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	, bool bIsLimitedItemShop
#endif
)
{
	/*
	if (nullptr == CMobManager::Instance().Get(dwNPCVnum))
	{
		sys_err("No such a npc by vnum %d", dwNPCVnum);
		return false;
	}
	*/
	sys_log(0, "SHOP #%d (Shopkeeper %d)", dwVnum, dwNPCVnum);

	m_dwVnum = dwVnum;
	m_dwNPCVnum = dwNPCVnum;

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	m_IsLimitedItemShop = bIsLimitedItemShop;
#endif
#if defined(ENABLE_12ZI) && !defined(ENABLE_12ZI_SHOP_LIMIT)
	if (dwNPCVnum == 20451)
		m_IsLimitedItemShop = true;
#endif

#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	isShopDeco = 0;
#endif

	uint8_t bItemCount;

#ifdef ENABLE_MYSHOP_DECO
	for (bItemCount = 0; bItemCount < SHOP_HOST_ITEM_MAX; ++bItemCount)
#else
	for (bItemCount = 0; bItemCount < SHOP_HOST_ITEM_MAX_NUM; ++bItemCount)
#endif
	{
		if (0 == (pTable + bItemCount)->vnum)
			break;
	}

	SetShopItems(pTable, bItemCount);
	return true;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CShop::TransferItems(LPCHARACTER owner, TShopItemTable* pTable, uint8_t bItemCount)
{
	if (!owner)
		return;

	std::vector<TPlayerItem> shopItems = owner->GetShopItems();

	// Start copying the items over
	for (int i = 0; i < bItemCount; ++i, ++pTable)
	{
		LPITEM pkItem = nullptr;
		pkItem = owner->GetItem(pTable->pos);

		if (!pkItem)
			continue;

		if (pkItem->IsEquipped())
			continue;

		if (!m_pGrid->IsEmpty(pTable->display_pos, 1, pkItem->GetProto()->bSize))
		{
			sys_err("Prevented from overriding item on position %d for shop #%lu!", pTable->display_pos, GetOwner()->GetPlayerID());
			continue;
		}

		if (owner->GetDesc()) // Actual player, not fake char from DB -> core shop spawns
		{
			// Store it into the shop vector
			TPlayerItem playerItem{};
			playerItem.id = pkItem->GetID();
			playerItem.vnum = pkItem->GetVnum();
			playerItem.count = pkItem->GetCount();
			playerItem.owner = owner->GetPlayerID();
			playerItem.pos = pTable->display_pos;
			playerItem.window = pkItem->GetWindow();
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
			playerItem.dwTransmutationVnum = pkItem->GetChangeLookVnum();
#endif
#ifdef ENABLE_SEALBIND_SYSTEM
			playerItem.nSealDate = pkItem->GetSealDate();
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
			playerItem.grade_element = pkItem->GetElementGrade();
			thecore_memcpy(playerItem.attack_element, pkItem->GetElementAttacks(), sizeof(playerItem.attack_element));
			playerItem.element_type_bonus = pkItem->GetElementsType();
			thecore_memcpy(playerItem.elements_value_bonus, pkItem->GetElementsValues(), sizeof(playerItem.elements_value_bonus));
#endif
			thecore_memcpy(playerItem.alSockets, pkItem->GetSockets(), sizeof(playerItem.alSockets));
			thecore_memcpy(playerItem.aAttr, pkItem->GetAttributes(), sizeof(playerItem.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(playerItem.aApplyRandom, pkItem->GetApplysRandom(), sizeof(playerItem.aApplyRandom));
			thecore_memcpy(playerItem.alRandomValues, pkItem->GetRandomDefaultAttrs(), sizeof(playerItem.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
			playerItem.set_value = pkItem->GetItemSetValue();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			playerItem.aPetInfo = pkItem->GetGrowthPetItemInfo();
#endif
			shopItems.emplace_back(playerItem);	//Uninitialized struct member: [playerItem.is_basic & playerItem.petInfo] [uninitStructMember]

			//Inform the client
			TPacketPlayerShopSet pack{};
			pack.header = HEADER_GC_PLAYER_SHOP_SET;
			pack.pos = pTable->display_pos;

			pack.count = static_cast<uint8_t>(playerItem.count);
			pack.vnum = playerItem.vnum;
			pack.price = pTable->price;
#ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
			pack.cheque = pTable->cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
			pack.dwTransmutationVnum = playerItem.dwTransmutationVnum;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
			pack.grade_element = playerItem.grade_element;
			pack.element_type_bonus = playerItem.element_type_bonus;
			thecore_memcpy(pack.attack_element, playerItem.attack_element, sizeof(pack.attack_element));
			thecore_memcpy(pack.elements_value_bonus, playerItem.elements_value_bonus, sizeof(pack.elements_value_bonus));
#endif

			thecore_memcpy(pack.alSockets, playerItem.alSockets, sizeof(pack.alSockets));
			thecore_memcpy(pack.aAttr, playerItem.aAttr, sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(pack.aApplyRandom, playerItem.aApplyRandom, sizeof(pack.aApplyRandom));
			thecore_memcpy(pack.alRandomValues, playerItem.alRandomValues, sizeof(pack.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
			pack.set_value = playerItem.set_value;
#endif

			owner->GetDesc()->Packet(&pack, sizeof(TPacketPlayerShopSet));
		}

		//Remove from quickslot
		owner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(pkItem->GetCell()), QUICKSLOT_MAX_POS);

		//Store latest window & transfer
		const uint8_t prevWindow = pkItem->GetWindow();
		if (prevWindow == PREMIUM_PRIVATE_SHOP)
			pkItem->SetSkipSave(true); //If it was already in SHOP window, we are transferring from some sort of "fake" char. No need to save.

		pkItem->RemoveFromCharacter();

		//We'll be setting it to a separate window
		pTable->pos = TItemPos(PREMIUM_PRIVATE_SHOP, pTable->display_pos);

		pkItem->AddToCharacter(m_pkPC, pTable->pos);

		if (prevWindow != PREMIUM_PRIVATE_SHOP) //Only if it's coming from someplace else that's not shop window (i.e only if its the first time this item goes to shop)
		{
			// Save
			ITEM_MANAGER::Instance().FlushDelayedSave(pkItem);

			// Log
			char buf[512];
			snprintf(buf, sizeof(buf), "%s gold(%u) cheque(%u)", pkItem->GetName(), pTable->price, pTable->cheque);
			LogManager::Instance().ItemLog(owner, pkItem, "PUT_INTO_SHOP", buf);
		}
		else
		{
			pkItem->SetSkipSave(false); //as we enabled it before
		}
	}

	owner->SetShopItems(shopItems);
}
#endif

void CShop::SetShopItems(TShopItemTable* pTable, uint8_t bItemCount)
{
#ifdef ENABLE_MYSHOP_DECO
	if (bItemCount > SHOP_HOST_ITEM_MAX)
		return;
#else
	if (bItemCount > SHOP_HOST_ITEM_MAX_NUM)
		return;
#endif

	m_pGrid->Clear();

#ifdef ENABLE_MYSHOP_DECO
	m_itemVector.resize(SHOP_HOST_ITEM_MAX);
#else
	m_itemVector.resize(SHOP_HOST_ITEM_MAX_NUM);
#endif

	msl::refill(m_itemVector);

	for (int i = 0; i < bItemCount; ++i)
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		SetShopItem(pTable);
#else
		LPITEM pkItem = nullptr;
		const TItemTable* item_table{};

		if (m_pkPC)
		{
			pkItem = m_pkPC->GetItem(pTable->pos);

			if (!pkItem)
			{
				sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
				continue;
			}

			item_table = pkItem->GetProto();
		}
		else
		{
			if (!pTable->vnum)
				continue;

			item_table = ITEM_MANAGER::Instance().GetTable(pTable->vnum);
		}

		if (!item_table)
		{
			sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
			continue;
		}

		int iPos;

		if (IsPCShop())
		{
			sys_log(0, "MyShop: use position %d", pTable->display_pos);
			iPos = pTable->display_pos;
		}
		else
			iPos = m_pGrid->FindBlank(1, item_table->bSize);

		if (iPos < 0)
		{
			sys_err("not enough shop window");
			continue;
		}

		if (!m_pGrid->IsEmpty(iPos, 1, item_table->bSize))
		{
			if (IsPCShop())
			{
				sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
			}
			else
			{
				sys_err("not empty position for npc shop");
			}
			continue;
		}

		m_pGrid->Put(iPos, 1, item_table->bSize);

		SHOP_ITEM& item = m_itemVector[iPos];

		item.pkItem = pkItem;
		item.itemid = 0;

		if (item.pkItem)
		{
			item.vnum = pkItem->GetVnum();
			item.count = pkItem->GetCount(); // In the case of a PC shop, the number of items should be the number of real items.
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item.dwTransmutationVnum = pkItem->GetChangeLookVnum();
#endif
			item.price = pTable->price; // The price is set by the user..
#ifdef ENABLE_CHEQUE_SYSTEM
			item.cheque = pTable->cheque;
#endif
			item.itemid = pkItem->GetID();
		}
		else
		{
			item.vnum = pTable->vnum;
			item.count = pTable->count;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			item.dwTransmutationVnum = pTable->dwTransmutationVnum;
#endif

#ifdef ENABLE_PROTO_RENEWAL
			if (IS_SET(item_table->dwFlags, ITEM_FLAG_COUNT_PER_1GOLD))
			{
				if (item_table->dwShopBuyPrice == 0)
					item.price = item.count;
				else
					item.price = item.count / item_table->dwShopBuyPrice;
			}
			else
				item.price = item_table->dwShopBuyPrice * item.count;
#else
			if (IS_SET(item_table->dwFlags, ITEM_FLAG_COUNT_PER_1GOLD))
			{
				if (item_table->dwGold == 0)
					item.price = item.count;
				else
					item.price = item.count / item_table->dwGold;
			}
			else
				item.price = item_table->dwGold * item.count;
#endif
		}

		char name[512];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int)item.vnum, item.count, item.dwTransmutationVnum);
#else
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int)item.vnum, item.count);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
		sys_log(0, "SHOP_ITEM: %-48s PRICE %-5d CHEQUE_PRICE %-5d", name, item.price, item.cheque);
#else
		sys_log(0, "SHOP_ITEM: %-36s PRICE %-5d", name, item.price);
#endif
#endif
		++pTable;
	}
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CShop::SetShopItem(TShopItemTable* pTable)
{
	LPITEM pkItem = nullptr;
	const TItemTable* item_table;

	if (IsPCShop())
	{
		pkItem = m_pkPC->GetItem(pTable->pos);

		if (!pkItem)
		{
			sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
			return;
		}

		item_table = pkItem->GetProto();
	}
	else
	{
		if (!pTable->vnum)
			return;

		item_table = ITEM_MANAGER::Instance().GetTable(pTable->vnum);
	}

	if (!item_table)
	{
		sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
		return;
	}

	int iPos;

	if (IsPCShop())
	{
		sys_log(0, "MyShop: use position %d", pTable->display_pos);
		iPos = pTable->display_pos;
	}
	else
		iPos = m_pGrid->FindBlank(1, item_table->bSize);

	if (iPos < 0)
	{
		sys_err("not enough shop window");
		return;
	}

	if (!m_pGrid->IsEmpty(iPos, 1, item_table->bSize))
	{
		if (IsPCShop())
		{
			sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
		}
		else
		{
			sys_err("not empty position for npc shop");
		}
		return;
	}

	m_pGrid->Put(iPos, 1, item_table->bSize);

	SHOP_ITEM& item = m_itemVector[iPos];

	item.pkItem = pkItem;
	item.itemid = 0;

	if (item.pkItem)
	{
		item.vnum = pkItem->GetVnum();
		item.count = static_cast<uint8_t>(pkItem->GetCount()); // In the case of a PC shop, the number of items should be the number of real items.
		item.price = pTable->price; // The price is set by the user..
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		item.cheque = pTable->cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		item.dwTransmutationVnum = pTable->dwTransmutationVnum;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		item.purchase_limit = pTable->purchase_limit;
#endif
		item.itemid = pkItem->GetID();
	}
	else
	{
		item.vnum = pTable->vnum;
		item.count = pTable->count;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		item.dwTransmutationVnum = pTable->dwTransmutationVnum;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		item.purchase_limit = pTable->purchase_limit;
#endif

#ifdef ENABLE_PROTO_RENEWAL
		if (IS_SET(item_table->dwFlags, ITEM_FLAG_COUNT_PER_1GOLD))
		{
			if (item_table->dwShopBuyPrice == 0)
				item.price = item.count;
			else
				item.price = item.count / item_table->dwShopBuyPrice;
		}
		else
			item.price = item_table->dwShopBuyPrice * item.count;
#else
		if (IS_SET(item_table->dwFlags, ITEM_FLAG_COUNT_PER_1GOLD))
		{
			if (item_table->dwGold == 0)
				item.price = item.count;
			else
				item.price = item.count / item_table->dwGold;
		}
		else
			item.price = item_table->dwGold * item.count;
#endif
	}

	char name[512];
	snprintf(name, sizeof(name), "%-32s(#%-5d) (x %d)", item_table->szLocaleName, (int)item.vnum, item.count);

#ifdef ENABLE_CHEQUE_SYSTEM //OFFLINE_SHOP_CHEQUE
	sys_log(0, "SHOP_ITEM: %-48s PRICE %-5d CHEQUE_PRICE %-5d", name, item.price, item.cheque);
#else
	sys_log(0, "SHOP_ITEM: %-48s PRICE %-5d", name, item.price);
#endif
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
int CShop::Buy(LPCHARACTER ch, uint8_t pos, bool isPrivateShopSearch)
#else
int CShop::Buy(LPCHARACTER ch, uint8_t pos)
#endif
{
	if (pos >= m_itemVector.size())
	{
		sys_log(0, "Shop::Buy : invalid position %d : %s", pos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "Shop::Buy : name %s pos %d", ch->GetName(), pos);

	const GuestMapType::iterator it = m_map_guest.find(ch);

	if (it == m_map_guest.end()
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
		&& !isPrivateShopSearch
#endif
		)
		return SHOP_SUBHEADER_GC_END;

	SHOP_ITEM& r_item = m_itemVector[pos];

	if (r_item.price < 0
#ifdef ENABLE_CHEQUE_SYSTEM
		&& r_item.cheque < 0
#endif
		)
	{
		LogManager::Instance().HackLog("SHOP_BUY_GOLD_OVERFLOW", ch);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	LPITEM pkSelectedItem = ITEM_MANAGER::Instance().Find(r_item.itemid);

	if (IsPCShop())
	{
		if (!pkSelectedItem)
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
				ch->GetPlayerID(),
				m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @fixme132 false to SHOP_SUBHEADER_GC_SOLD_OUT
		}

		if ((pkSelectedItem->GetOwner() != m_pkPC))
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
				ch->GetPlayerID(),
				m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @fixme132 false to SHOP_SUBHEADER_GC_SOLD_OUT
		}
	}

	uint32_t dwPrice = r_item.price;
#ifdef ENABLE_CHEQUE_SYSTEM
	const uint32_t dwCheque = r_item.cheque;
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	if (!isPrivateShopSearch)
#endif
	{
#	ifdef ENABLE_NEWSTUFF
		if (it->second && !g_bEmpireShopPriceTripleDisable)
#	else
		if (it->second)	// if other empire, price is triple
#	endif
			dwPrice *= 3;
	}

	if (ch->GetGold() < dwPrice)
	{
		sys_log(1, "Shop::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	if (ch->GetCheque() < static_cast<uint32_t>(dwCheque))
	{
		sys_log(1, "Shop::Buy : Not enough cheque : %s has %d, price %d", ch->GetName(), ch->GetCheque(), dwCheque);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}
#endif

#if defined(ENABLE_12ZI) && !defined(ENABLE_12ZI_SHOP_LIMIT)
	if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
	{
		if (ch->CountZodiacItems(r_item.vnum) < (uint32_t)r_item.count)
		{
			sys_log(1, "Shop::Buy : Shop limited purchase : %s has %d, price %d", ch->GetName(), r_item.count, dwPrice);
			return SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER;
		}
	}
#endif

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	if (m_IsLimitedItemShop)
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		if (ch && ch->GetViewingShopOwner())
		{
			if (!ch->GetViewingShopOwner()->CanShopPurchaseLimitedItem(ch, r_item.vnum))
			{
				sys_log(1, "Shop::Buy : Shop limited purchase : %s has %d, price %d", ch->GetName(), r_item.count, dwPrice);
				return SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER;
			}
		}
#else
		if (ch && ch->GetShopOwner())
		{
			if (!ch->GetShopOwner()->CanShopPurchaseLimitedItem(ch, r_item.vnum))
			{
				sys_log(1, "Shop::Buy : Shop limited purchase : %s has %d, price %d", ch->GetName(), r_item.count, dwPrice);
				return SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER;
			}
		}
#endif
	}
#endif

	LPITEM item;

	if (m_pkPC) // A shop run by a PC requires the PC to have actual items.
		item = r_item.pkItem;
	else
		item = ITEM_MANAGER::Instance().CreateItem(r_item.vnum, r_item.count);

	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

#ifdef ENABLE_SHOP_BLACKLIST
	if (!m_pkPC)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("hivalue_item_sell") == 0)
		{
			//Ball of Blessing && Annual Event
			if (item->GetVnum() == 70024 || item->GetVnum() == 70035)
			{
				return SHOP_SUBHEADER_GC_END;
			}
		}
	}
#endif

	int iEmptyPos;
	if (item->IsDragonSoul())
	{
		iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
	}
	else
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		iEmptyPos = ch->GetEmptyInventory(item);
#else
		iEmptyPos = ch->GetEmptyInventory(item->GetSize());
#endif
	}

	if (iEmptyPos < 0)
	{
		if (m_pkPC)
		{
			sys_log(1, "Shop::Buy at PC Shop : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
		else
		{
			sys_log(1, "Shop::Buy : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			M2_DESTROY_ITEM(item);
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
	}

#if defined(ENABLE_12ZI) && !defined(ENABLE_12ZI_SHOP_LIMIT)
	if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
	{
		ch->SetZodiacItems(r_item.vnum, 0);
		ch->SetPurchaseZodiacItems(r_item.vnum, r_item.count);
	}
#endif

	if (dwPrice)
		ch->PointChange(POINT_GOLD, -dwPrice, false);
#ifdef ENABLE_CHEQUE_SYSTEM
	if (dwCheque)
		ch->PointChange(POINT_CHEQUE, -static_cast<int>(dwCheque), false);
#endif

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	if (m_IsLimitedItemShop)
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		if (ch && ch->GetViewingShopOwner())
			ch->GetViewingShopOwner()->ShopPurchaseLimitItem(ch, item->GetVnum());
#else
		if (ch && ch->GetShopOwner())
			ch->GetShopOwner()->ShopPurchaseLimitItem(ch, item->GetVnum());
#endif
	}
#endif

	//calculate tax
	uint32_t dwTax = 0;
	int iVal = 0;

	{
		iVal = quest::CQuestManager::Instance().GetEventFlag("personal_shop");

		if (0 < iVal)
		{
			if (iVal > 100)
				iVal = 100;

			dwTax = dwPrice * iVal / 100;
			dwPrice = dwPrice - dwTax;
		}
		else
		{
			iVal = 0;
			dwTax = 0;
		}
	}

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
	// 5% tax on purchase at the shop
	if (!m_pkPC)
	{
		CMonarch::Instance().SendtoDBAddMoney(dwTax, ch->GetEmpire(), ch);
	}
#endif

	// Monarchy System: Tax Collection
	if (m_pkPC)
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		if (!m_pkPC->GetPlayerID())
		{
			sys_err("Shop not bound to any player ID!");
			return SHOP_SUBHEADER_GC_SOLD_OUT;
		}

		char buf[512];

		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %u", item->GetName(), ch->GetPlayerID(), m_pkPC->GetPlayerID(), dwPrice);
			LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), SHOP_BUY, buf);
			LogManager::Instance().GoldBarLog(m_pkPC->GetPlayerID(), item->GetID(), SHOP_SELL, buf);
		}

		if (trade_effect) {	//@custom020
			if (m_pkPC) {
				for (int i = 1; i < r_item.price + 1; i = i + trade_effect_shop_threshold)
					ch->CreateFly(4, m_pkPC); // 1 Fly pro 100kk , sonst laggt es O:
				m_pkPC->CreateFly(5, ch); // 1 Item
				m_pkPC->CreateFly(6, ch); // Allgemeines anzeigen
			}
		}

		item->RemoveFromCharacter();
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		ITEM_MANAGER::Instance().FlushDelayedSave(item);

#	ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		sys_log(0, "Shop trade: %u to %u, price: %u cheque: %u", m_pkPC->GetPlayerID(), ch->GetPlayerID(), dwPrice, dwCheque);
#	else
		sys_log(0, "Shop trade: %u to %u, price %u", m_pkPC->GetPlayerID(), ch->GetPlayerID(), dwPrice);
#	endif

#	ifdef ENABLE_CHEQUE_SYSTEM
		snprintf(buf, sizeof(buf), "%s %u(%s) gold(%u) cheque(%u) count(%u)", item->GetName(), m_pkPC->GetPlayerID(), m_pkPC->GetName(), dwPrice, dwCheque, item->GetCount());
#	else
		snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", item->GetName(), m_pkPC->GetPlayerID(), m_pkPC->GetName(), dwPrice, item->GetCount());
#	endif
		LogManager::Instance().ItemLog(ch, item, "SHOP_BUY", buf);

#	ifdef ENABLE_CHEQUE_SYSTEM
		snprintf(buf, sizeof(buf), "%s %u(%s) gold(%u) cheque(%u) count(%u)", item->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice, dwCheque, item->GetCount());
#	else
		snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", item->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice, item->GetCount());
#	endif
		LogManager::Instance().ItemLog(m_pkPC, item, "SHOP_SELL", buf);

		//Reset the SHOP_ITEM in the vector
		r_item.count = 0;
		r_item.pkItem = nullptr;
		r_item.itemid = 0;
		r_item.vnum = 0;
		r_item.price = 0;
#	ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		r_item.cheque = 0;
#	endif

		//Clear these slots on the grid
		m_pGrid->Get(pos, 1, item->GetSize());

		//Update everyone seeing this shop that the item was sold
		BroadcastUpdateItem(pos);

		//Inform database of the sale (which will inform the player as well, if online)
		const uint32_t pid = m_pkPC->GetPlayerID();

		db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t));
		constexpr uint8_t subheader = SHOP_SUBHEADER_GD_BUY;
		db_clientdesc->Packet(&subheader, sizeof(uint8_t));
		db_clientdesc->Packet(&pid, sizeof(uint32_t));
		db_clientdesc->Packet(&pos, sizeof(uint8_t));

		//Tax goes for the seller's empire
		//CMonarch::Instance().SendtoDBAddMoney(dwTax, m_pkPC->GetEmpire(), m_pkPC);

		//If there's nothing more, we close shop.
		if (IsEmpty())
		{
			sys_log(0, "Closing player #%u shop because there's nothing more to sell.", m_pkPC->GetPlayerID());
			CloseMyShop();
		}
#else
		m_pkPC->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), QUICKSLOT_MAX_POS);

		char buf[512];

		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %u", item->GetName(), ch->GetPlayerID(), m_pkPC->GetPlayerID(), dwPrice);
			LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), SHOP_BUY, buf);
			LogManager::Instance().GoldBarLog(m_pkPC->GetPlayerID(), item->GetID(), SHOP_SELL, buf);
		}

		item->RemoveFromCharacter();
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		ITEM_MANAGER::Instance().FlushDelayedSave(item);

		snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", item->GetName(), m_pkPC->GetPlayerID(), m_pkPC->GetName(), dwPrice, item->GetCount());
		LogManager::Instance().ItemLog(ch, item, "SHOP_BUY", buf);

		snprintf(buf, sizeof(buf), "%s %u(%s) %u %u", item->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice, item->GetCount());
		LogManager::Instance().ItemLog(m_pkPC, item, "SHOP_SELL", buf);

		r_item.pkItem = nullptr;
		BroadcastUpdateItem(pos);

		m_pkPC->PointChange(POINT_GOLD, dwPrice, false);
#ifdef ENABLE_CHEQUE_SYSTEM
		m_pkPC->PointChange(POINT_CHEQUE, dwCheque, false);
#endif

		if (iVal > 0)
			m_pkPC->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[881]This sale will be taxed %d%%."), iVal);

		CMonarch::Instance().SendtoDBAddMoney(dwTax, m_pkPC->GetEmpire(), m_pkPC);
#endif
	}
	else
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		ITEM_MANAGER::Instance().FlushDelayedSave(item);
		LogManager::Instance().ItemLog(ch, item, "BUY", item->GetName());

#ifdef ENABLE_12ZI
		if (CZodiacManager::Instance().IsZiStageMapIndex(ch->GetMapIndex()))
		{
			BroadcastUpdateItemCh(pos, ch);
		}
#endif

		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), PERSONAL_SHOP_BUY, "");
		}

		DBManager::Instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), -dwPrice);
	}

	if (item)
		sys_log(0, "SHOP: BUY: name %s %s(x %d):%u price %u", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);

	ch->Save();

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsPCShop() && IsSoldOut()) {	//@custom036
		m_pkPC->CloseMyShop();
		m_pkPC->ChatPacket(CHAT_TYPE_NOTICE, "Your store closed automatically, reason: sold out!");
	}
#endif

	return (SHOP_SUBHEADER_GC_OK);
}

bool CShop::AddGuest(LPCHARACTER ch, uint32_t owner_vid, bool bOtherEmpire)
{
	if (!ch)
		return false;

	if (ch->GetExchange())
		return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (ch->GetViewingShop())
		return false;

	ch->SetViewingShop(this);
#else
	if (ch->GetShop())
		return false;

	ch->SetShop(this);
#endif

	m_map_guest.insert(GuestMapType::value_type(ch, bOtherEmpire));

	TPacketGCShop pack{};

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_START;
	TPacketGCShopStart pack2{};

	pack2.owner_vid = owner_vid;
#ifdef ENABLE_MYSHOP_DECO
	if (IsShopDeco())
		pack2.shop_tab_count = 2;
	else
		pack2.shop_tab_count = 1;
#endif
#ifdef ENABLE_12ZI
	pack2.isLimitedItemShop = m_IsLimitedItemShop;
#endif

#ifdef ENABLE_MYSHOP_DECO
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX; ++i)
#else
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
#endif
	{
		const SHOP_ITEM& item = m_itemVector[i];

#ifdef ENABLE_SHOP_BLACKLIST
		//HIVALUE_ITEM_EVENT
		if (quest::CQuestManager::Instance().GetEventFlag("hivalue_item_sell") == 0)
		{
			//Ball of Blessing && Annual Event
			if (item.vnum == 70024 || item.vnum == 70035)
			{
				continue;
			}
		}
#endif
		//END_HIVALUE_ITEM_EVENT

		if (m_pkPC && !item.pkItem)
			continue;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		// Do not broadcast if price is zero or no item exists
		if (item.price == 0
#	ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			&& item.cheque == 0
#	endif
			)
		{
			continue;
		}
#endif

		pack2.items[i].vnum = item.vnum;

		// REMOVED_EMPIRE_PRICE_LIFT
#ifdef ENABLE_NEWSTUFF
		if (bOtherEmpire && !g_bEmpireShopPriceTripleDisable) // no empire price penalty for pc shop
#else
		if (bOtherEmpire) // no empire price penalty for pc shop
#endif
		{
#ifdef ENABLE_CHEQUE_SYSTEM
			pack2.items[i].price.dwPrice = item.price * 3;
#else
			pack2.items[i].price = item.price * 3;
#endif
		}
		else
		{
#ifdef ENABLE_CHEQUE_SYSTEM
			pack2.items[i].price.dwPrice = item.price;
			pack2.items[i].price.dwCheque = item.cheque;
#else
			pack2.items[i].price = item.price;
#endif
		}
		// END_REMOVED_EMPIRE_PRICE_LIFT

		pack2.items[i].count = item.count;

		if (item.pkItem)
		{
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			pack2.items[i].dwTransmutationVnum = item.pkItem->GetChangeLookVnum();
#endif
#ifdef ENABLE_REFINE_ELEMENT
			pack2.items[i].grade_element = item.pkItem->GetElementGrade();
			thecore_memcpy(pack2.items[i].attack_element, item.pkItem->GetElementAttacks(), sizeof(pack2.items[i].attack_element));
			pack2.items[i].element_type_bonus = item.pkItem->GetElementsType();
			thecore_memcpy(pack2.items[i].elements_value_bonus, item.pkItem->GetElementsValues(), sizeof(pack2.items[i].elements_value_bonus));
#endif
			thecore_memcpy(pack2.items[i].alSockets, item.pkItem->GetSockets(), sizeof(pack2.items[i].alSockets));
			thecore_memcpy(pack2.items[i].aAttr, item.pkItem->GetAttributes(), sizeof(pack2.items[i].aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(pack2.items[i].aApplyRandom, item.pkItem->GetApplysRandom(), sizeof(pack2.items[i].aApplyRandom));
			thecore_memcpy(pack2.items[i].alRandomValues, item.pkItem->GetRandomDefaultAttrs(), sizeof(pack2.items[i].alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
			pack2.items[i].set_value = item.pkItem->GetItemSetValue();
#endif
		}

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		if (m_IsLimitedItemShop)
		{
			const auto pkShopKeeper = CHARACTER_MANAGER::Instance().Find(owner_vid);
			if (pkShopKeeper)
				pkShopKeeper->SetPurchaseItemLimit(ch, item.vnum, item.purchase_limit, 0);
		}
#endif
	}

	pack.size = sizeof(pack) + sizeof(pack2);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->Packet(&pack2, sizeof(TPacketGCShopStart));
	return true;
}

void CShop::RemoveGuest(LPCHARACTER ch)
{
	if (!ch)
		return;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (ch->GetViewingShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetViewingShop(nullptr);
#else
	if (ch->GetShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetShop(nullptr);
#endif

	TPacketGCShop pack{};

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_END;
	pack.size = sizeof(TPacketGCShop);

	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void CShop::Broadcast(const void* data, int bytes)
{
	sys_log(1, "Shop::Broadcast %p %d", data, bytes);

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		const LPCHARACTER& ch = it->first;

		if (ch && ch->GetDesc())
			ch->GetDesc()->Packet(data, bytes);

		++it;
	}
}

void CShop::BroadcastUpdateItem(uint8_t pos)
{
	TPacketGCShop pack{};
	TPacketGCShopUpdateItem pack2;

	TEMP_BUFFER buf;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size = sizeof(pack) + sizeof(pack2);

	pack2.pos = pos;

	if (m_pkPC && !m_itemVector[pos].pkItem)
		pack2.item.vnum = 0;
	else
	{
		pack2.item.vnum = m_itemVector[pos].vnum;
		if (m_itemVector[pos].pkItem)
		{
#ifdef ENABLE_REFINE_ELEMENT
			pack2.item.grade_element = m_itemVector[pos].pkItem->GetElementGrade();
			thecore_memcpy(pack2.item.attack_element, m_itemVector[pos].pkItem->GetElementAttacks(), sizeof(pack2.item.attack_element));
			pack2.item.element_type_bonus = m_itemVector[pos].pkItem->GetElementsType();
			thecore_memcpy(pack2.item.elements_value_bonus, m_itemVector[pos].pkItem->GetElementsValues(), sizeof(pack2.item.elements_value_bonus));
#endif
			thecore_memcpy(pack2.item.alSockets, m_itemVector[pos].pkItem->GetSockets(), sizeof(pack2.item.alSockets));
			thecore_memcpy(pack2.item.aAttr, m_itemVector[pos].pkItem->GetAttributes(), sizeof(pack2.item.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(pack2.item.aApplyRandom, m_itemVector[pos].pkItem->GetApplysRandom(), sizeof(pack2.item.aApplyRandom));
			thecore_memcpy(pack2.item.alRandomValues, m_itemVector[pos].pkItem->GetRandomDefaultAttrs(), sizeof(pack2.item.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
			pack2.item.set_value = m_itemVector[pos].pkItem->GetItemSetValue();
#endif
		}
		else
		{
#ifdef ENABLE_REFINE_ELEMENT
			pack2.item.grade_element = 0;
			thecore_memcpy(pack2.item.attack_element, 0, sizeof(pack2.item.attack_element));
			pack2.item.element_type_bonus = 0;
			thecore_memcpy(pack2.item.elements_value_bonus, 0, sizeof(pack2.item.elements_value_bonus));
#endif
			memset(pack2.item.alSockets, 0, sizeof(pack2.item.alSockets));
			memset(pack2.item.aAttr, 0, sizeof(pack2.item.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			memset(pack2.item.aApplyRandom, 0, sizeof(pack2.item.aApplyRandom));
			memset(pack2.item.alRandomValues, 0, sizeof(pack2.item.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
			pack2.item.set_value = 0;
#endif
		}
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	pack2.item.price.dwPrice = m_itemVector[pos].price;
	pack2.item.price.dwCheque = m_itemVector[pos].cheque;
#else
	pack2.item.price = m_itemVector[pos].price;
#endif
	pack2.item.count	= m_itemVector[pos].count;

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}

#ifdef ENABLE_12ZI
void CShop::BroadcastUpdateItemCh(uint8_t pos, LPCHARACTER ch)
{
	if (!ch || !ch->IsPC())
	{
		sys_err("Shop.cpp: BroadcastUpdateItemCh: No existe ch o ch->IsPC");
		return;
	}

	TPacketGCShop pack{};
	TPacketGCShopUpdateItem pack2;

	TEMP_BUFFER	buf;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size = sizeof(pack) + sizeof(pack2);

	pack2.pos = pos;

	if (m_pkPC && !m_itemVector[pos].pkItem)
		pack2.item.vnum = 0;
	else
	{
		pack2.item.vnum = m_itemVector[pos].vnum;
		if (m_itemVector[pos].pkItem)
		{
			thecore_memcpy(pack2.item.alSockets, m_itemVector[pos].pkItem->GetSockets(), sizeof(pack2.item.alSockets));
			thecore_memcpy(pack2.item.aAttr, m_itemVector[pos].pkItem->GetAttributes(), sizeof(pack2.item.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(pack2.item.aApplyRandom, m_itemVector[pos].pkItem->GetApplysRandom(), sizeof(pack2.item.aApplyRandom));
			thecore_memcpy(pack2.item.alRandomValues, m_itemVector[pos].pkItem->GetRandomDefaultAttrs(), sizeof(pack2.item.alRandomValues));
#endif
		}
		else
		{
			memset(pack2.item.alSockets, 0, sizeof(pack2.item.alSockets));
			memset(pack2.item.aAttr, 0, sizeof(pack2.item.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			memset(pack2.item.aApplyRandom, 0, sizeof(pack2.item.aApplyRandom));
			memset(pack2.item.alRandomValues, 0, sizeof(pack2.item.alRandomValues));
#endif
		}
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	pack2.item.price.dwPrice = m_itemVector[pos].price;
	pack2.item.price.dwCheque = m_itemVector[pos].cheque;
#else
	pack2.item.price = m_itemVector[pos].price;
#endif
	pack2.item.count = m_itemVector[pos].count;

	pack2.item.getLimitedCount = m_itemVector[pos].count;
	pack2.item.getLimitedPurchaseCount = ch->PurchaseCountZodiacItems(m_itemVector[pos].vnum);

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}
#endif

int CShop::GetNumberByVnum(uint32_t dwVnum)
{
	int itemNumber = 0;

#ifdef ENABLE_MYSHOP_DECO
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX; ++i)
#else
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
#endif
	{
		const SHOP_ITEM& item = m_itemVector[i];

		if (item.vnum == dwVnum)
		{
			itemNumber += item.count;
		}
	}

	return itemNumber;
}

bool CShop::IsSellingItem(uint32_t itemID)
{
	bool isSelling = false;

#ifdef ENABLE_MYSHOP_DECO
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX; ++i)
#else
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
#endif
	{
		if ((uint32_t)(m_itemVector[i].itemid) == itemID)
		{
			isSelling = true;
			break;
		}
	}

	return isSelling;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
bool CShop::IsEmpty()
{
	for (const auto& i : m_itemVector)
	{
		if (i.vnum != 0)
			return false;
	}

	return true;
}

bool CShop::TransferItemAway(LPCHARACTER ch, uint8_t pos, TItemPos targetPos)
{
	if (!ch)
		return false;

	if (pos > m_itemVector.size())
	{
		sys_err("Out of bounds attempt to transfer an item out of the shop (pos %d)", pos);
		return false;
	}

	if (!IsPCShop())
	{
		sys_err("Trying to transfer an item from a non-pc shop. HACKER!");
		return false;
	}

	if (targetPos.window_type != INVENTORY && targetPos.window_type != DRAGON_SOUL_INVENTORY)
	{
		sys_err("Invalid transfer window (%d) by %s.", targetPos.window_type, ch->GetName());
		return false;
	}

	// Can't deploy directly to equipment.
	if (targetPos.IsEquipPosition())
	{
		return false;
	}

	SHOP_ITEM& r_item = m_itemVector[pos];
	if (!r_item.itemid)
		return false;

	LPITEM item = ITEM_MANAGER::Instance().Find(r_item.itemid);
	if (!item)
	{
		sys_err("No item could be transferred for pos %d (Doesn't exist)", pos);
		return false;
	}

	// Determine if the position given for the item is OK or if
	// we need to find a new one (or whether we can't find any).
	// Note: Dragon Soul items are always sent to a position determined by the system.
	int iEmptyCell;
	if (!ch->GetItem(targetPos) && ch->IsEmptyItemGrid(targetPos, item->GetSize()) && !item->IsDragonSoul())
	{
		iEmptyCell = targetPos.cell;
	}
	else
	{
		if (item->IsDragonSoul())
			iEmptyCell = ch->GetEmptyDragonSoulInventory(item);
		else
		{
#ifdef ENABLE_SPECIAL_INVENTORY
			iEmptyCell = ch->GetEmptyInventory(item);
#else
			iEmptyCell = ch->GetEmptyInventory(item->GetSize());
#endif
		}
	}

	if (iEmptyCell < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1715]");
		return false;
	}

	item->RemoveFromCharacter();

	// Remove it from the list of the shop items on the receiver
	std::vector<TPlayerItem> shopItems = ch->GetShopItems();
	for (auto it = shopItems.begin(); it != shopItems.end(); ++it)
	{
		if (it->pos == pos)
		{
			shopItems.erase(it);
			break;
		}
	}
	ch->SetShopItems(shopItems);

	// Add the item
	if (item->IsDragonSoul())
		item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
	else
		item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyCell));

	ITEM_MANAGER::Instance().FlushDelayedSave(item);

	// Remove the item from the vector & update everyone
	r_item.count = 0;
	r_item.pkItem = nullptr;
	r_item.itemid = 0;
	r_item.vnum = 0;
	r_item.price = 0;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	r_item.cheque = 0;
#endif

	// Clear these slots on the grid
	m_pGrid->Get(pos, 1, item->GetSize());

	BroadcastUpdateItem(pos);

	if (IsEmpty())
	{
		sys_log(0, "Closing player #%u shop because the owner took the last item from it.", m_pkPC->GetPlayerID());
		CloseMyShop();
	}

	return true;
}

void CShop::CloseMyShop()
{
	if (!IsPCShop())
		return;

	// Save the latest shop data
	SetClosed(true);
	Save();

	// We'll destroy the character and the shop from the outside.
}

void CShop::Save()
{
	if (!GetOwner())
	{
		sys_err("Trying to save non-pc shop.");
		return;
	}

	// Create a shop table and fill in basic info
	std::string shopSign = GetShopSign();

	TPlayerShopTable table{};
	table.pid = m_pkPC->GetPlayerID();
	strlcpy(table.playerName, m_pkPC->GetName(), sizeof(table.playerName));
	strlcpy(table.shopName, shopSign.c_str(), sizeof(table.shopName));
	table.x = m_pkPC->GetX();
	table.y = m_pkPC->GetY();
	table.channel = g_bChannel;
	table.mapIndex = m_pkPC->GetMapIndex();
	table.closed = IsClosed();
	table.openTime = GetOpenTime();
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	const auto& mTable = m_pkPC->GetMobTable();
	if (mTable.dwVnum)
		table.dwPolyVnum = mTable.dwVnum;
	table.bTitleType = m_bTitleType;
	table.isShopDeco = IsShopDeco();
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	if (m_pkPC->GetConquerorLevel())
		table.clevel = m_pkPC->GetConquerorLevel();
	else
		table.level = m_pkPC->GetLevel();
	table.alignment = m_pkPC->GetAlignment();
	table.empire = m_pkPC->GetEmpire();
	table.guild = 0; // check_me
#endif

	// Populate the items table.
	std::vector<CShop::SHOP_ITEM> shopItems = GetItemVector();

	int i = 0;
	for (const auto& si : shopItems)
	{
		if (!si.vnum || !si.pkItem) // Empty OR already bought
			continue;

		TShopItemTable t{};
		t.vnum = si.vnum;
		t.price = si.price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		t.cheque = si.cheque;
#endif
		t.pos = TItemPos(PREMIUM_PRIVATE_SHOP, si.pkItem->GetCell());
		t.display_pos = static_cast<uint8_t>(si.pkItem->GetCell());
		t.count = si.count;

		table.items[i] = t;
		++i;
	}

	// Send to DB
	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(TPlayerShopTable));
	//constexpr uint8_t subHeader = SHOP_SUBHEADER_GD_SAVE;
	constexpr uint8_t subHeader = SHOP_SUBHEADER_GD_SAVE_FLUSH;

	db_clientdesc->Packet(&subHeader, sizeof(uint8_t));
	db_clientdesc->Packet(&table, sizeof(TPlayerShopTable));
}

// Save offline minutes via a packet to DB.
void CShop::SaveOffline()
{
	if (!GetOwner())
	{
		sys_err("Trying to save non-pc shop.");
		return;
	}

	db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(int));

	constexpr uint8_t subtype = SHOP_SUBHEADER_GD_UPDATE_OFFLINE;
	db_clientdesc->Packet(&subtype, sizeof(uint8_t));

	const uint32_t pid = GetOwner()->GetPlayerID();
	db_clientdesc->Packet(&pid, sizeof(uint32_t));

	const int offTime = GetOfflineMinutes();
	db_clientdesc->Packet(&offTime, sizeof(int));
}

void CShop::CloseShopBecauseEditing()
{
	for (const auto& ch : m_map_guest)
	{
		RemoveGuest(ch.first);
		ch.first->ChatPacket(CHAT_TYPE_INFO, "[LS;1714]");
	}
}

bool CShop::RemoveItemByID(uint32_t itemID)
{
#ifdef ENABLE_MYSHOP_DECO
	for (size_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX; ++i)
#else
	for (size_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
#endif
	{
		SHOP_ITEM& item = m_itemVector[i];
		if (!item.pkItem || !item.vnum)
			continue;

		// Found! Delete, return OK
		if (item.itemid == static_cast<int>(itemID))
		{
			const uint8_t pos = static_cast<uint8_t>(i);
			const uint32_t pid = m_pkPC->GetPlayerID();

			LPITEM pkItem = item.pkItem;
			if (!pkItem)
				continue;

			LPCHARACTER pkOwner = CHARACTER_MANAGER::Instance().FindByPID(pid);
			if (pkOwner)
			{
				std::vector<TPlayerItem> shopItems = pkOwner->GetShopItems(); // Remove it from the list of the shop items on the receiver
				for (auto it = shopItems.begin(); it != shopItems.end(); ++it)
				{
					if (it->pos == pos)
					{
						shopItems.erase(it);
						break;
					}
				}

				pkOwner->SetShopItems(shopItems);
			}


			item.itemid = 0;
			item.pkItem = nullptr;
			item.vnum = 0;
			item.count = 0;
			item.price = 0;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
			item.cheque = 0;
#endif

			m_pGrid->Get(pos, 1, pkItem->GetSize());

			BroadcastUpdateItem(static_cast<uint8_t>(i));

			if (!IsEmpty())
			{
				db_clientdesc->DBPacketHeader(HEADER_GD_SHOP, 0, sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t));
				constexpr uint8_t subheader = SHOP_SUBHEADER_GD_REMOVE;
				db_clientdesc->Packet(&subheader, sizeof(uint8_t));
				db_clientdesc->Packet(&pid, sizeof(uint32_t));
				db_clientdesc->Packet(&pos, sizeof(uint8_t));
			}

			if (pkOwner)
				pkOwner->RemoveShopItemByPos(pos);

			// fix item expired when shop boot
			if (pkItem)
				pkItem->RemoveFromCharacter();

			if (IsEmpty())
			{
				sys_log(0, "Closing player #%u shop because there's nothing more to sell.", m_pkPC->GetPlayerID());
				CloseMyShop();
			}

			// Closed shop, destroy it (through owner)
			if (IsClosed() && IsPCShop())
			{
				if (pkOwner)
				{
					TPacketPlayerShopSign p{}; // Also sync the shop sign here
					p.header = HEADER_GC_MY_SHOP_SIGN;
					memset(p.sign, 0, sizeof(p.sign));
					pkOwner->GetDesc()->Packet(&p, sizeof(TPacketPlayerShopSign));
				}

				M2_DESTROY_CHARACTER(GetOwner());
			}

			return true;
		}
	}

	return false;
}
#endif

#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
bool CShop::IsSoldOut() const	//@custom036
{
	for (const auto & r_item : m_itemVector) {
		if (r_item.pkItem)
			return false;
	}
}
#endif

bool CShop::CanOpenShopHere(long mapindex)	//@custom023
{
	if (test_server && mapindex == MAP_C1) // city1 map on test server
		return true;

	switch (mapindex)	// Only Mapsindexes - where you're able to create an Shop!
	{
#ifdef ENABLE_OPEN_SHOP_ONLY_IN_MARKET
		case MAP_PRIVATSHOP:
#else
		//Map1
		case MAP_A1:
		case MAP_B1:
		case MAP_C1:

		//Map2
		case MAP_A3:
		case MAP_B3:
		case MAP_C3:
#endif
			return true;
			break;

		default:
			return false;
	}
}
