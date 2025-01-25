#include "stdafx.h"
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#include <array>

#include "QID.h"
#include "ClientManager.h"
#include "Cache.h"
#include "Config.h"
#include "Main.h"

extern int g_test_server;
extern std::map<uint32_t, spShop> shopMap;

spShop CClientManager::GetShop(uint32_t pid)
{
	//Shop not in map - return null
	if (shopMap.find(pid) == shopMap.end())
		return nullptr;

	//Shop in map, but invalid [delete & return null]
	spShop shop = shopMap[pid];
	if (!shop)
	{
		shopMap.erase(pid);
		return nullptr;
	}

	//Valid shop
	return shop;
}

/*
* Boot shop loading. This is a result of an async query
* with QID_LOAD_PLAYER_SHOPS.
*
* The function creates all the necessary Shop instances and
* starts an async query to load all the shops' items.
*/
void CClientManager::RESULT_LOAD_SHOPS(SQLMsg* pMsg)
{
	SQLResult* pRes = pMsg->Get();
	if (pRes->uiNumRows < 1) 
	{
		sys_log(0, "No shops to load.");
		return;
	}

	MYSQL_ROW shopRow;
	while ((shopRow = mysql_fetch_row(pRes->pSQLResult)))
	{
		uint32_t goldStash;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		uint32_t chequeStash;
#endif
		int offlineMinutesLeft;

		TPlayerShopTable table;
		int row = 0;
		str_to_number(table.pid, shopRow[row++]);
		str_to_number(table.x, shopRow[row++]);
		str_to_number(table.y, shopRow[row++]);
		str_to_number(table.channel, shopRow[row++]);
		str_to_number(table.mapIndex, shopRow[row++]);

		str_to_number(table.openTime, shopRow[row++]);
		str_to_number(offlineMinutesLeft, shopRow[row++]);

		strlcpy(table.shopName, shopRow[row++], sizeof(table.shopName));
		strlcpy(table.playerName, shopRow[row++], sizeof(table.playerName));
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		str_to_number(table.level, shopRow[row++]);
		str_to_number(table.clevel, shopRow[row++]);
		str_to_number(table.alignment, shopRow[row++]);
		str_to_number(table.empire, shopRow[row++]);
		str_to_number(table.guild, shopRow[row++]);
#endif
		str_to_number(goldStash, shopRow[row++]);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		str_to_number(chequeStash, shopRow[row++]);
#endif
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
		str_to_number(table.dwPolyVnum, shopRow[row++]);
		str_to_number(table.bTitleType, shopRow[row++]);
		str_to_bool(table.isShopDeco, shopRow[row++]);
#endif
		str_to_bool(table.closed, shopRow[row++]);

		memset(&table.items, 0, sizeof(table.items));

		spShop shop(new Shop());
		shop->SetTable(table);
		shop->SetGoldStash(goldStash);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		shop->SetChequeStash(chequeStash);
#endif
		shop->SetOfflineMinutesLeft(offlineMinutesLeft);

		shopMap[table.pid] = shop;
	}

	// Load the items now
	char szQuery[512];
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	snprintf(szQuery, sizeof(szQuery), 
		"SELECT "
			"pid, i.pos, price, cheque, vnum, count "
		"FROM "
			"private_shop_items AS pi "
			"INNER JOIN item AS i ON (pi.pos = i.pos AND pid = owner_id) "
		"WHERE "
			"window = 'PREMIUM_PRIVATE_SHOP'"
	);
#else
	snprintf(szQuery, sizeof(szQuery), 
		"SELECT "
			"pid, i.pos, price, vnum, count "
		"FROM "
			"private_shop_items AS pi "
			"INNER JOIN item AS i ON (pi.pos = i.pos AND pid = owner_id) "
		"WHERE "
			"window = 'PREMIUM_PRIVATE_SHOP'"
	);
#endif

	CDBManager::Instance().ReturnQuery(szQuery, QID_LOAD_PLAYER_SHOP_ITEMS, 0, nullptr);
}

/*
* Boot-time private shop item load. This is a result of an async query
* with QID_LOAD_PLAYER_SHOP_ITEMS.
*
* The function loops through each item fetched and stores it into its shop.
*/
void CClientManager::RESULT_LOAD_PLAYER_SHOP_ITEMS(SQLMsg* pMsg)
{
	int i = 0;
	std::map<uint32_t, int> itemsByPID;
	MYSQL_ROW row;

	//Data in each row, 0 - indexed:
	//  pid, pos, price, vnum, count
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		//Find shop
		uint32_t pid;
		str_to_number(pid, row[0]);
		spShop shop = GetShop(pid);

		if (!shop)
			continue;

		i = itemsByPID[pid];

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		TShopItemTable table;
		str_to_number(table.display_pos, row[1]);
		str_to_number(table.price, row[2]);
		str_to_number(table.cheque, row[3]);
		table.pos = TItemPos(EWindows::PREMIUM_PRIVATE_SHOP, table.display_pos);
		str_to_number(table.vnum, row[4]);
		str_to_number(table.count, row[5]);
#else
		TShopItemTable table;
		str_to_number(table.display_pos, row[1]);
		str_to_number(table.price, row[2]);
		table.pos = TItemPos(EWindows::PREMIUM_PRIVATE_SHOP, table.display_pos);
		str_to_number(table.vnum, row[3]);
		str_to_number(table.count, row[4]);
#endif

		shop->SetItem(i, table);
		++itemsByPID[pid];
	}

	sys_log(0, "RESULT_LOAD_PLAYER_SHOP_ITEMS success. Players: %d", itemsByPID.size());
}

/*
* Called upon a core boot, this loops through all our loaded shops and sends said core the shops that
* are open, not empty, and belong to it. This information is used solely to spawn them.
*
* All the shop information is sent under a single packet.
*/
void CClientManager::SendShops(CPeer* peer)
{
	long lMapArray[32];
	thecore_memcpy(lMapArray, peer->GetMaps(), sizeof(lMapArray));

	// Load banned players and skip those
	std::vector<uint32_t> bannedId;

	char bpQueryStr[QUERY_MAX_LEN];
	snprintf(bpQueryStr, sizeof(bpQueryStr),
		//"SELECT player.id FROM %s.player RIGHT JOIN %s.account ON account.id = player.account_id WHERE account.status = 'BLOCK'", GetPlayerDBName(), GetAccountDBName());
		"SELECT player.id FROM player.player RIGHT JOIN account.account ON account.id = player.account_id WHERE account.status = 'BLOCK'");

	auto upBPMsg(CDBManager::Instance().DirectQuery(bpQueryStr));
	SQLResult* bpRes = upBPMsg->Get();

	if (bpRes && bpRes->uiNumRows > 0)
	{
		for (size_t k = 0; k < bpRes->uiNumRows; ++k)
		{
			MYSQL_ROW row = mysql_fetch_row(upBPMsg->Get()->pSQLResult);
			if (!row || !row[0])
				continue;

			bannedId.emplace_back(atoi(row[0]));
		}
	}

	int shopsSent = 0;
	int bannedCount = 0;
	std::vector<TPlayerShopTableCache> tables;

	for (const auto it : shopMap)
	{
		spShop shop = it.second;

		if (shop->GetTable()->channel != peer->GetChannel())
		{
			//sys_log(0, "Skipping shop #%u because the channel is not correct! (%d vs %d)", shop->GetOwnerPID(), shop->GetTable()->channel, peer->GetChannel());
			continue;
		}

		// fixed
		if (!shop->IsClosed() && shop->HasItems() && shop->GetOfflineMinutesLeft() < 1)
			shop->SetOfflineMinutesLeft(1);

		if (!shop->HasItems() || (shop->GetOfflineMinutesLeft() < 1))
		{
			sys_log(0, "Skipping shop #%u boot spawn due to no items or no time", shop->GetOwnerPID());
			continue;
		}

		if (std::find(bannedId.begin(), bannedId.end(), shop->GetOwnerPID()) != bannedId.end())
		{
			sys_log(0, "Skipping shop #%u boot spawn because the owner is banned", shop->GetOwnerPID());
			++bannedCount;
			continue;
		}

		TPlayerShopTableCache shopCacheTable = shop->GetCacheTable();

		// Is the map index the shop is located at on this core?
		if (std::find(std::begin(lMapArray), std::end(lMapArray), shopCacheTable.mapIndex) != std::end(lMapArray))
		{
			shop->BindPeer(peer);
			// Reopen and save that
			if (shop->IsClosed())
			{
				shop->SetClosed(false);
				PutShopCache(shopCacheTable);
			}

			++shopsSent;
			tables.emplace_back(shopCacheTable);
		}
	}

	if (shopsSent < 1)
	{
		sys_log(0, "No shops to send from the map of size: %d", shopMap.size());
		return;
	}

	sys_log(0, "%d shops to be sent from the map of size: %d (%d banned)", shopsSent, shopMap.size(), bannedCount);

	if (shopsSent > 0)
	{
		peer->EncodeHeader(HEADER_DG_PLAYER_SPAWN_SHOP, 0, sizeof(int) + (sizeof(TPlayerShopTableCache) * shopsSent));
		peer->Encode(&shopsSent, sizeof(int));
		for (const auto shopCacheTable : tables)
		{
			TPlayerShopTableCache shopTable = (TPlayerShopTableCache)shopCacheTable;
			peer->Encode(&shopTable, sizeof(TPlayerShopTableCache));
		}
	}
}


/*
* Given a shop table, save the shop to the shop map. If the shop didn't exist already, this function will create
* a new shop entry. Finally, the new shop information is put on the DB cache to be flushed out to the database.
*
* If the shop is marked as closed, it will be unbound from any peer.
*/
void CClientManager::SaveShopFlush(CPeer* peer, TPlayerShopTable* data)
{
	uint32_t dwPID = data->pid;

	CShopCache* pkCache = GetShopCache(dwPID);

	if (!pkCache)
		return;

	sys_log(0, "FLUSH_SHOP_CACHE: %u", dwPID);

	pkCache->Flush();
	//FlushItemCacheSet(dwPID);

	//m_map_playerCache.erase(dwPID);
	//delete pkCache;
}

void CClientManager::SaveShop(CPeer* peer, TPlayerShopTable* data)
{
	if (g_test_server)
	{
		sys_log(0, "SHOP_SAVE: %u", data->pid);
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
		sys_log(0, "MYSHOP_DECO_SAVE: PolyVnum: %d Type: %d IsShopDeco: %d", data->dwPolyVnum, data->bTitleType, data->isShopDeco);
#endif
	}

	spShop shop = shopMap[data->pid];
	if (shop)
	{
		shop->SetTable(*data);
		if (shop->IsClosed() && shop->HasPeer())
		{
			shop->BindPeer(nullptr);
		}
		else if (!shop->HasPeer() || shop->GetPeer() != peer)
		{
			sys_log(0, "Updating shop #%u's peer to %p.", data->pid, peer);
			shop->BindPeer(peer);
			shop->GetTable()->channel = peer->GetChannel();
		}
	}
	else
	{
		// We don't have it yet, so this is a new shop. Create it.
		spShop s(new Shop());
		s->SetTable(*data);
		s->BindPeer(peer);

		shopMap[data->pid] = s;

		shop = s;
		shop->SetOwnerOnline(true);
	}

	if (shop)
		PutShopCache(shop->GetCacheTable());
}

#ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
/*
* Notify the open shop that it should not be spending any
* more offline minutes.
*/
void CClientManager::CancelOpenShopOffline(uint32_t pid)
{
	spShop shop = GetShop(pid);
	if (!shop || shop->IsClosed() || !shop->GetPeer())
		return;

	CPeer* peer = shop->GetPeer();
	if (!peer)
		return;

	peer->EncodeHeader(HEADER_DG_SHOP_STOP_OFFLINE, 0, sizeof(uint32_t));
	peer->Encode(&pid, sizeof(pid));
}
#endif

/*
* Send a shop for a single player if the database determines it is
* not yet spawned and it has any items.
*/
void CClientManager::SendSingleShop(uint32_t pid)
{
	spShop shop = GetShop(pid);
	if (!shop)
		return;

	// Mark owner online
	shop->SetOwnerOnline(true);

	// If the shop has a bound peer and it is not closed, it is already open wherever.
	// We should not send anything.
	if (shop->HasPeer() && !shop->IsClosed())
		return;

	// Doesn't make any sense to request spawn for an empty shop
	if (!shop->HasItems())
		return;

	CPeer* myPeer = nullptr;
	TPlayerShopTable* table = shop->GetTable();
	TPlayerShopTableCache shopCacheTable = shop->GetCacheTable(); // added to use cache information
	for (const auto peer : m_peerList)
	{
		//if (!peer->IsAuth() && peer->IsValid() && peer->GetChannel() == table->channel) // Try to contact the peer we need in order to spawn the shop
		if (peer->GetChannel() == table->channel) // Try to contact the peer we need in order to spawn the shop
		{
			long mapList[32];
			thecore_memcpy(mapList, peer->GetMaps(), sizeof(mapList));

			if (std::find(std::begin(mapList), std::end(mapList), table->mapIndex) != std::end(mapList))
			{
				myPeer = peer; // Eureka!
				break;
			}
		}
	}

	if (!myPeer)
		return;

	// Bind & mark as open
	shop->BindPeer(myPeer);

	if (shop->IsClosed())
		shop->SetClosed(false);
	
	// Send the packet to spawn
	int shopsSent = 1;
	myPeer->EncodeHeader(HEADER_DG_PLAYER_SPAWN_SHOP, 0, sizeof(int) + (sizeof(TPlayerShopTableCache) * shopsSent));
	myPeer->Encode(&shopsSent, sizeof(int));
	TPlayerShopTableCache shopTable = (TPlayerShopTableCache)shopCacheTable;
	myPeer->Encode(&shopTable, sizeof(TPlayerShopTableCache));
}

/*
* Take gold from the shop. 
*
* This function directly reduces the gold amount (so long as there are no issues)
* and notifies the calling peer to award the gold to the player.
*/
void CClientManager::WithdrawShopGold(CPeer* peer, uint32_t playerHandle, const char * data)
{
	uint32_t pid = *(uint32_t *)data;
	data += sizeof(uint32_t);

	uint32_t gold = *(uint32_t *)data;
	data += sizeof(uint32_t);

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	uint32_t cheque = *(uint32_t *)data;
	data += sizeof(uint32_t);
#endif
	
	sys_log(0, "Withdrawing %u gold %u cheque from shop for player #%d", gold, cheque, pid);

	spShop shop = GetShop(pid);
	
	if (!shop)
		return;

	bool success = false;

	//Verify its all okay
	if (shop->GetGoldStash() < gold)
	{
		sys_err("Player #%u trying to withdraw %u gold from shop with just %u gold!", pid, gold, shop->GetGoldStash());
		return; // missing return
	}
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	else if (shop->GetChequeStash() < cheque)
	{
		sys_err("Player #%u trying to withdraw %u cheque from shop with just %u cheque!", pid, cheque, shop->GetChequeStash());
		return; // missing return
	}
#endif
	else
	{
		//Reduce & save
		shop->AlterGoldStash(gold, false);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		shop->AlterChequeStash(cheque, false);
#endif
		PutShopCache(shop->GetCacheTable());

		success = true;
	}

	TPacketGoldWithdrawResult p;
	p.shopPid = pid;
	p.goldAmount = gold;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	p.chequeAmount = cheque;
#endif
	p.success = success;

	//Notify
	peer->EncodeHeader(HEADER_DG_SHOP_WITHDRAW_RESULT, playerHandle, sizeof(p));
	peer->Encode(&p, sizeof(p));
}

/*
* Rollback if previous gold withdraw transaction failed.
*
*/
void CClientManager::RollbackWithdrawShopGold(CPeer* peer, uint32_t playerHandle, const char * data)
{
	uint32_t pid = *(uint32_t *)data;
	data += sizeof(uint32_t);

	uint32_t gold = *(uint32_t *)data;
	data += sizeof(uint32_t);

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	uint32_t cheque = *(uint32_t *)data;
	data += sizeof(uint32_t);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	sys_log(0, "Rolling back transaction [%d gold] [%d cheque] on shop #%u", gold, cheque, pid);
#else
	sys_log(0, "Rolling back transaction [%d gold] on shop #%u", gold, pid);
#endif

	spShop shop = GetShop(pid);
	if (!shop)
	{
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		sys_err("Critical: failed to rollback withdraw action of %d gold %d cheque on shop #%u [no shop]", gold, cheque, pid);
#else
		sys_err("Critical: failed to rollback withdraw action of %d gold on shop #%u [no shop]", gold, pid);
#endif
		return;
	}

	shop->AlterGoldStash(gold, true);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	shop->AlterChequeStash(cheque, true);
#endif
	PutShopCache(shop->GetCacheTable());
}


/*
* Send information about the shop and its state to the player.
* This is used to inform the client interface of the shop status.
*/
void CClientManager::SendMyShopInfo(CPeer* peer, uint32_t dwHandle, uint32_t pid)
{
	spShop shop = GetShop(pid);
	if (!shop)
		return;

	uint8_t bCount = 0;
	TPlayerShopTable * table = shop->GetTable();
#ifdef ENABLE_MYSHOP_DECO
	std::array<TMyShopPriceInfo, SHOP_HOST_ITEM_MAX> s_info;
#else
	std::array<TMyShopPriceInfo, SHOP_HOST_ITEM_MAX_NUM> s_info;
#endif
	for (const auto& item : table->items)
	{
		if (item.vnum == 0)
			continue;

#ifdef ENABLE_MYSHOP_DECO
		if (item.display_pos > SHOP_HOST_ITEM_MAX)
#else
		if (item.display_pos > SHOP_HOST_ITEM_MAX_NUM)
#endif
		{
			sys_err("Something is wrong. Item position is over the max (%d)", item.display_pos);
			continue;
		}

		TMyShopPriceInfo inf;
		inf.pos = item.display_pos;
		inf.price = item.price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		inf.cheque = item.cheque;
#endif
		thecore_memcpy(&s_info[bCount++], &inf, sizeof(TMyShopPriceInfo));
	}

	char sign[SHOP_SIGN_MAX_LEN + 1];
	strlcpy(sign, shop->GetTable()->shopName, sizeof(sign));

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	int offlineMinutes = shop->GetOfflineMinutesLeft();
#endif
	int channel = table->channel;
	int x = table->x;
	int y = table->y;

	peer->EncodeHeader(HEADER_DG_SHOP_MYINFO_LOAD, dwHandle, 
		sizeof(uint8_t) +
		sizeof(sign) +
		sizeof(uint32_t) +
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		sizeof(uint32_t) +
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
		sizeof(int) +
#endif
		(3 * sizeof(int)) + // channel + x + y
		sizeof(TMyShopPriceInfo) * bCount);
	peer->EncodeBYTE(bCount);
	peer->Encode(&sign, sizeof(sign));
	peer->EncodeDWORD(shop->GetGoldStash());
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	peer->EncodeDWORD(shop->GetChequeStash());
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	peer->Encode(&offlineMinutes, sizeof(int));
#endif
	peer->Encode(&channel, sizeof(int));
	peer->Encode(&x, sizeof(int));
	peer->Encode(&y, sizeof(int));

	if (bCount)
		peer->Encode(&s_info[0], sizeof(TMyShopPriceInfo) * bCount); //Copy the whole arr
}

/*
* Unbind all the shops associated with a given peer.
* Called upon peer crash/shutdown.
*/

void CClientManager::UnbindPeer(const CPeer* peer)
{
	int unbound = 0;
	for (const auto it : shopMap)
	{
		spShop shop = it.second;
		if (!shop)
			continue;

		if (shop->GetPeer() == peer)
		{
			shop->BindPeer(nullptr);
			++unbound;
		}
	}

	sys_log(0, "Unbound %d shops from their peer (peer has closed)", unbound);
}

/*
* Close the shop instantly, regardless of offline time. This does not block it from respawning
* if the player logs back in.
*/

void CClientManager::RequestCloseShop(spShop shop, bool isDeleteShop)
{
	if (shop->IsClosed())
		return;

	shop->SetClosed(true);

	CPeer* peer = shop->GetPeer();
	if (!peer)
		return;

	shop->BindPeer(nullptr);

	uint32_t pid = shop->GetOwnerPID();
	peer->EncodeHeader(HEADER_DG_SHOP_CLOSE, 0, sizeof(uint32_t) + sizeof(bool));
	peer->Encode(&pid, sizeof(uint32_t));
	peer->Encode(&isDeleteShop, sizeof(bool));
}

// If we got a close request from one of the cores.
void CClientManager::RequestCloseShop(const char* data)
{
	uint32_t pid = *(uint32_t *)data;
	data += sizeof(uint32_t);

	spShop shop = GetShop(pid);

	if (!shop)
		return;

	RequestCloseShop(shop, false);
}

// If we got a rename request from one of the cores.
void CClientManager::RenameShop(const char* data)
{
	uint32_t pid = *(uint32_t *)data;
	data += sizeof(uint32_t);

	char newNameChr[SHOP_SIGN_MAX_LEN + 1];
	strlcpy(newNameChr, data, sizeof(newNameChr));
	std::string newName(newNameChr);
	data += sizeof(newNameChr);	

	spShop shop = GetShop(pid);
	if (!shop || newName.empty())
		return;

	shop->SetName(newName);
	PutShopCache(shop->GetCacheTable()); //Save

	CPeer* peer = shop->GetPeer();
	if (!peer)
		return;

	peer->EncodeHeader(HEADER_DG_SHOP_UPDATE_NAME, 0, sizeof(uint32_t) + sizeof(newNameChr));
	peer->Encode(&pid, sizeof(uint32_t));
	peer->Encode(newNameChr, sizeof(newNameChr));
}

/************************************************************************/
/*					        Packet processing                           */
/************************************************************************/
void CClientManager::ProcessShopPacket(CPeer* peer, uint32_t dwHandle, const char * data)
{
	uint8_t subHeader = *(uint8_t*)data;
	data += sizeof(uint8_t);

	switch (subHeader)
	{
		case SHOP_SUBHEADER_GD_SAVE:
			SaveShop(peer, (TPlayerShopTable*)data);
			break;

		case SHOP_SUBHEADER_GD_SAVE_FLUSH:
			{
				SaveShop(peer, (TPlayerShopTable*)data);
				SaveShopFlush(peer, (TPlayerShopTable*)data);
			}
			break;

		case SHOP_SUBHEADER_GD_BUY:
			ShopSaleResult(data);
			break;

		case SHOP_SUBHEADER_GD_REMOVE:
			ShopRemove(data);
			break;

		case SHOP_SUBHEADER_GD_WITHDRAW:
			WithdrawShopGold(peer, dwHandle, data);
			break;

		case SHOP_SUBHEADER_GD_WITHDRAW_ROLLBACK:
			RollbackWithdrawShopGold(peer, dwHandle, data);
			break;

		case SHOP_SUBHEADER_GD_UPDATE_OFFLINE:
			UpdateOfflineTime(data);
			break;

		case SHOP_SUBHEADER_GD_CLOSE_REQUEST:
			RequestCloseShop(data);
			break;

		case SHOP_SUBHEADER_GD_RENAME:
			RenameShop(data);
			break;

		default:
			sys_err("UNKNOWN SHOP SUBHEADER: %d", subHeader);
	}
}


/*
* Receive the position of the sold item and properly update internals,
* such as gold stash, or shop status (open, closed), if the item were
* to be the last one.
*
* This function attempts to find the peer the player is at.
*/
void CClientManager::ShopSaleResult(const char* data)
{
	uint32_t playerID = *(uint32_t *)data;
	data += sizeof(uint32_t);

	uint8_t displayPos = *(uint8_t *)data;
	data += sizeof(uint8_t);

	spShop shop = GetShop(playerID);
	if (!shop)
	{
		sys_err("Shop sale result for player %u on an apparently non-existing shop.", playerID);
		return;
	}

	if (shop->IsClosed())
	{
		sys_log(1, "Closed shops do not process ShopSaleResult.");
		return;
	}

	// Let's see what we sold
	TPlayerShopTable* shopTable = shop->GetTable();
	int arrIndex = shop->FindItem(displayPos);

	if (arrIndex >= 0) //Valid position
	{
		TShopItemTable sold = shopTable->items[arrIndex];

		// Add gold to the stash
		shop->AlterGoldStash(sold.price, true);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
		shop->AlterChequeStash(sold.cheque, true);
#endif

		// (Try to) notify the player
		CLoginData * pkLD = GetLoginDataByPID(playerID);
		if (pkLD && pkLD->IsPlay() && pkLD->GetConnectedPeerHandle())
		{
			for (const auto peer : m_peerList)
			{
				if (!peer->GetChannel() || peer->GetHandle() != pkLD->GetConnectedPeerHandle())
					continue;

				TPacketShopSaleResult ssr;
				ssr.amount = sold.count;
				ssr.gold = sold.price;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
				ssr.cheque = sold.cheque;
#endif
				ssr.itemVnum = sold.vnum;
				ssr.pos = sold.display_pos;

				peer->EncodeHeader(HEADER_DG_SHOP_SALE_INFO, 0, sizeof(ssr) + sizeof(uint32_t));
				peer->Encode(&playerID, sizeof(uint32_t));
				peer->Encode(&ssr, sizeof(ssr));
				break;
			}
		}

		// Remove the item from the shop
		shop->RemoveItem(displayPos);

		// If we have ran out of items, close the shop
		if (!shop->HasItems())
		{
			sys_log(0, "Closing player %u's shop due to lack of any more items to sell.", playerID);
			shop->SetClosed(true);
		}

		// Save
		PutShopCache(shop->GetCacheTable());
	}
}

/*
* Remove an item from the shop (by position).
* Closes the shop if no more items are left.
*/
void CClientManager::ShopRemove(const char* data)
{
	uint32_t playerID = *(uint32_t *)data;
	data += sizeof(uint32_t);

	uint8_t displayPos = *(uint8_t *)data;
	data += sizeof(uint8_t);

	spShop shop = GetShop(playerID);
	if (!shop)
	{
		sys_err("Shop item removal for #%u's shop on a non-existing shop.", playerID);
		return;
	}

	if (!shop->RemoveItem(displayPos))
	{
		sys_err("Item to remove from #%u shop not found (display pos %d)", playerID, displayPos);
		return;
	}

	// If we have ran out of items, close the shop
	if (!shop->HasItems())
	{
		if (g_test_server)
			sys_log(0, "Closing player %u's shop due to lack of more items to sell", playerID);

		shop->SetClosed(true);
	}

	// Save
	PutShopCache(shop->GetCacheTable());
}

#ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
/*
* When the player logs out, this function simply sends a packet to the
* shop core indicating the minutes it should start its timer from.
*/
void CClientManager::NotifyShopLogout(uint32_t pid)
{
	spShop shop = GetShop(pid);
	if (!shop)
		return;

	shop->SetOwnerOnline(false);

	if (shop->IsClosed())
		return;

	if (!shop->HasPeer()) 
	{
		sys_err("Trying to start offline checks for shop but shop had no peer!");
		return;
	}

	int offMinutes = shop->GetOfflineMinutesLeft();

	CPeer* peer = shop->GetPeer();
	if (!peer)
		return;

	peer->EncodeHeader(HEADER_DG_SHOP_TIMER, 0, sizeof(uint32_t) + sizeof(int));
	peer->Encode(&pid, sizeof(uint32_t));
	peer->Encode(&offMinutes, sizeof(int));
	sys_log(0, "Requesting ShopTimer start at CH%d for %d minutes (peer %p, pid %u)", peer->GetChannel(), offMinutes, peer, pid);
}
#else
void CClientManager::NotifyShopTime(uint32_t pid)
{
	spShop shop = GetShop(pid);
	if (!shop)
		return;

	if (shop->IsClosed())
		return;

	if (!shop->HasPeer())
	{
		sys_err("Trying to start offline checks for shop but shop had no peer!");
		return;
	}

	int offMinutes = shop->GetOfflineMinutesLeft();

	CPeer* peer = shop->GetPeer();
	if (!peer)
		return;

	peer->EncodeHeader(HEADER_DG_SHOP_TIMER, 0, sizeof(uint32_t) + sizeof(int));
	peer->Encode(&pid, sizeof(uint32_t));
	peer->Encode(&offMinutes, sizeof(int));
	sys_log(0, "Requesting ShopTimer start at CH%d for %d minutes (peer %p, pid %u)", peer->GetChannel(), offMinutes, peer, pid);
}
#endif

// Updates a shop's offline time and saves.
void CClientManager::UpdateOfflineTime(const char* data)
{
	uint32_t pid = *(uint32_t *)data;
	data += sizeof(uint32_t);

	int offMinutes = *(int *)data;
	data += sizeof(int);

	spShop shop = GetShop(pid);
	if (!shop)
		return;

	if (shop->IsClosed())
		return;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	shop->SetOfflineMinutesLeft(offMinutes);
#endif
	PutShopCache(shop->GetCacheTable());
}

/************************************************************************/
/*					     	Cache operations                            */
/************************************************************************/
CShopCache * CClientManager::GetShopCache(uint32_t pid)
{
	TShopCacheMap::iterator it = m_map_shopCache.find(pid);

	if (it == m_map_shopCache.end())
		return nullptr;

	return it->second;
}

void CClientManager::PutShopCache(TPlayerShopTableCache pNew)
{
	CShopCache * c;

	c = GetShopCache(pNew.pid);

	if (!c)
	{
		c = new CShopCache;
		m_map_shopCache.insert(TShopCacheMap::value_type(pNew.pid, c));
	}

	c->Put(&pNew);
}

void CClientManager::UpdateShopCache()
{
	TShopCacheMap::iterator it = m_map_shopCache.begin();

	while (it != m_map_shopCache.end())
	{
		CShopCache* pCache = it->second;

		if (pCache->CheckFlushTimeout()) {
			pCache->Flush();
		}

		if (pCache->CheckTimeout())
		{
			it = m_map_shopCache.erase(it);
			delete pCache;
		}
		else
		{
			++it;
		}
	}
}
#endif