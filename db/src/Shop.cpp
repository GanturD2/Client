#include "stdafx.h"
#include "Shop.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#include "ClientManager.h"

Shop::Shop() : 
	m_goldStash(0),
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	m_chequeStash(0),
#endif
	m_linkedPeer(nullptr),
	m_offlineMinutesLeft(0),
	m_isOwnerOnline(false)
{
	Initialize();
}

void Shop::Initialize()
{
	m_linkedPeer = nullptr;
	memset(&m_table, 0, sizeof(m_table));
}

void Shop::SetItem(int index, TShopItemTable table)
{
	m_table.items[index] = table;
}

bool Shop::HasItems() const
{
	for (const auto item : m_table.items)
	{
		if (item.vnum != 0)
			return true;
	}

	return false;
}

void Shop::AlterGoldStash(uint32_t price, bool bAdd)
{
	if (bAdd)
		m_goldStash += price;
	else
		m_goldStash -= price;

	// Limit goldstash values:
	m_goldStash = m2::minmax<uint32_t>(0, m_goldStash, GOLD_MAX);
}

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
void Shop::AlterChequeStash(uint32_t cheque, bool bAdd)
{
	if (bAdd)
		m_chequeStash += cheque;
	else
		m_chequeStash -= cheque;

	// Limit cheque_stash values:
	m_chequeStash = m2::minmax<uint32_t>(0, m_chequeStash, CHEQUE_MAX);
}
#endif

// Form the table that is required to save to cache.
TPlayerShopTableCache Shop::GetCacheTable()
{
	TPlayerShopTable* table = GetTable();

	TPlayerShopTableCache cacheTable;
	cacheTable.pid = table->pid;
	cacheTable.channel = table->channel;
	cacheTable.closed = table->closed;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	cacheTable.level = table->level;
	cacheTable.clevel = table->clevel;
	cacheTable.alignment = table->alignment;
	cacheTable.empire = table->empire;
	cacheTable.guild = table->guild;
#endif
	cacheTable.goldStash = GetGoldStash();
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	cacheTable.chequeStash = GetChequeStash();
#endif
	thecore_memcpy(cacheTable.items, table->items, sizeof(cacheTable.items));
	thecore_memcpy(cacheTable.playerName, table->playerName, sizeof(cacheTable.playerName));
	thecore_memcpy(cacheTable.shopName, table->shopName, sizeof(cacheTable.shopName));
	cacheTable.mapIndex = table->mapIndex;
	cacheTable.offlineMinutesLeft = GetOfflineMinutesLeft();
	cacheTable.x = table->x;
	cacheTable.y = table->y;
	cacheTable.openTime = table->openTime;
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	cacheTable.dwPolyVnum = table->dwPolyVnum;
	cacheTable.bTitleType = table->bTitleType;
	cacheTable.isShopDeco = table->isShopDeco;
#endif
	return cacheTable;
}

bool Shop::RemoveItem(int display_pos)
{
	int found = false;

	TShopItemTable newItemTable[SHOP_INVENTORY_MAX_NUM] = {};
#ifdef ENABLE_MYSHOP_DECO
	for (int i = 0, k = 0; i < SHOP_HOST_ITEM_MAX; ++i)
#else
	for (int i = 0, k = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
#endif
	{
		const TShopItemTable item = m_table.items[i];
		if (item.vnum == 0)
			continue;

		if (item.display_pos == display_pos) 
		{
			found = true;
			continue;
		}

		newItemTable[k++] = item;
	}

	thecore_memcpy(m_table.items, newItemTable, sizeof(m_table.items));
	return found;
}

int Shop::FindItem(int display_pos)
{
#ifdef ENABLE_MYSHOP_DECO
	for (int i = 0; i < SHOP_HOST_ITEM_MAX; ++i)
#else
	for (int i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
#endif
	{
		TShopItemTable item = m_table.items[i];
		if (item.vnum == 0)
			continue;

		if (item.display_pos == display_pos)
			return i;
	}

	return -1;
}
#endif