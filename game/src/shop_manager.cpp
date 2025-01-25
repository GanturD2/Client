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
#include "desc_client.h"
#include "shop_manager.h"
#include "group_text_parse_tree.h"
#include "shopEx.h"
#include <boost/algorithm/string/predicate.hpp>
#include "shop_manager.h"
#include <cctype>
#include "../../common/CommonDefines.h"

extern bool g_bEmpireShopPriceTripleDisable;

CShopManager::CShopManager()
{
}

CShopManager::~CShopManager()
{
	Destroy();
}

bool CShopManager::Initialize(TShopTable* table, int size)
{
	if (!m_map_pkShop.empty())
		return false;

	int i;

	for (i = 0; i < size; ++i, ++table)
	{
		LPSHOP shop = M2_NEW CShop;

		if (!shop->Create(table->dwVnum, table->dwNPCVnum, table->items
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
			, table->is_limited
#endif
		))
		{
			M2_DELETE(shop);
			continue;
		}

		m_map_pkShop.insert(TShopMap::value_type(table->dwVnum, shop));
		m_map_pkShopByNPCVnum.insert(TShopMap::value_type(table->dwNPCVnum, shop));
	}

	char szShopTableExFileName[256];
	snprintf(szShopTableExFileName, sizeof(szShopTableExFileName),
		"%s/shop_table_ex.txt", LocaleService_GetBasePath().c_str());

	return ReadShopTableEx(szShopTableExFileName);
}

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
bool CShopManager::InitializeGemShop(TGemShopTable* table, int size)
{
	std::map<int, TGemShopTable*> map_shop;

#ifndef _DEBUG
	if (m_pGemShopTable)
	{
		delete[](m_pGemShopTable);
		m_pGemShopTable = nullptr;
	}
#endif

	TGemShopTable* shop_table = m_pGemShopTable;

	for (int i = 0; i < size; ++i, ++table)
	{
		if (map_shop.end() == map_shop.find(i))
		{
			const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(table->dwVnum);

			if (!pProto)
			{
				sys_err("No item by vnum : %d", table->dwVnum);
				continue;
			}

			shop_table = new TGemShopTable;
			memset(shop_table, 0, sizeof(TGemShopTable));

			shop_table->dwVnum = table->dwVnum;
			shop_table->bCount = table->bCount;
			shop_table->dwPrice = table->dwPrice;
			shop_table->dwRow = table->dwRow;

			map_shop[i] = shop_table;
			//sys_log(0, "GEM_ITEM: #%d PRICE %d", map_shop[i]->dwVnum, map_shop[i]->dwPrice);
		}
		else
		{
			shop_table = map_shop[i];
		}
	}

	m_pGemShopTable = new TGemShopTable[map_shop.size()];
	m_iGemShopTableSize = map_shop.size();

	typeof(map_shop.begin()) it = map_shop.begin();

	int i = 0;

	while (it != map_shop.end())
	{
		thecore_memcpy((m_pGemShopTable + i), (it++)->second, sizeof(TGemShopTable));
		++i;
	}

	return true;
}

uint32_t CShopManager::GemShopGetRandomId(uint32_t dwRow)
{
	std::vector<uint32_t> dwItemId;

	for (int id = 0; id < m_iGemShopTableSize; id++)
	{
		if (m_pGemShopTable[id].dwRow == dwRow)
			dwItemId.emplace_back(id);
	}

	const uint32_t randomNumber = number(0, dwItemId.size() - 1);

	return dwItemId[randomNumber];
}

uint32_t CShopManager::GemShopGetVnumById(int id)
{
	if (id < m_iGemShopTableSize)
		return m_pGemShopTable[id].dwVnum;

	return 0;
}

uint8_t CShopManager::GemShopGetCountById(int id)
{
	if (id < m_iGemShopTableSize)
		return m_pGemShopTable[id].bCount;

	return 0;
}

uint32_t CShopManager::GemShopGetPriceById(int id)
{
	if (id < m_iGemShopTableSize)
		return m_pGemShopTable[id].dwPrice;

	return 0;
}
#endif

void CShopManager::Destroy() //@fix
{
	for (auto it = m_map_pkShopByNPCVnum.begin(); it != m_map_pkShopByNPCVnum.end(); ++it)
		delete it->second;

	m_map_pkShopByNPCVnum.clear();
	m_map_pkShop.clear();
}

LPSHOP CShopManager::Get(uint32_t dwVnum)
{
	const TShopMap::const_iterator it = m_map_pkShop.find(dwVnum);

	if (it == m_map_pkShop.end())
		return nullptr;

	return (it->second);
}

LPSHOP CShopManager::GetByNPCVnum(uint32_t dwVnum)
{
	const TShopMap::const_iterator it = m_map_pkShopByNPCVnum.find(dwVnum);

	if (it == m_map_pkShopByNPCVnum.end())
		return nullptr;

	return (it->second);
}

/*
 * Interface functions
 */

// Start a shop deal
bool CShopManager::StartShopping(LPCHARACTER pkChr, LPCHARACTER pkChrShopKeeper, int iShopVnum)
{
	if (!pkChr)
		return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (pkChr->GetViewingShopOwner() == pkChrShopKeeper)
		return false;
#else
	if (pkChr->GetShopOwner() == pkChrShopKeeper)
		return false;
#endif

	// this method is only for NPC
	if (pkChrShopKeeper->IsPC())
		return false;

	//PREVENT_TRADE_WINDOW
	if (pkChr->GetOpenedWindow(W_SAFEBOX | W_EXCHANGE | W_CUBE
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
		| W_MYSHOP
#endif
#ifdef ENABLE_SKILLBOOK_COMBINATION
		| W_SKILLBOOK_COMB
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		| W_ATTR_6TH_7TH
#endif
#ifdef ENABLE_AURA_SYSTEM
		| W_AURA
#endif
#ifdef ENABLE_SWITCHBOT
		| W_SWITCHBOT
#endif
	))
	{
		pkChr->ChatPacket(CHAT_TYPE_INFO, "[LS;876]");
		return false;
	}
	//END_PREVENT_TRADE_WINDOW

	const long distance = DISTANCE_APPROX(pkChr->GetX() - pkChrShopKeeper->GetX(), pkChr->GetY() - pkChrShopKeeper->GetY());

	if (distance >= SHOP_MAX_DISTANCE)
	{
		sys_log(1, "SHOP: TOO_FAR: %s distance %d", pkChr->GetName(), distance);
		return false;
	}

	LPSHOP pkShop;

	if (iShopVnum)
		pkShop = Get(iShopVnum);
	else
		pkShop = GetByNPCVnum(pkChrShopKeeper->GetRaceNum());

	if (!pkShop)
	{
		sys_log(1, "SHOP: NO SHOP");
		return false;
	}

	bool bOtherEmpire = false;

	if (pkChr->GetEmpire() != pkChrShopKeeper->GetEmpire())
		bOtherEmpire = true;

	pkShop->AddGuest(pkChr, pkChrShopKeeper->GetVID(), bOtherEmpire);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	pkChr->SetViewingShopOwner(pkChrShopKeeper);
#else
	pkChr->SetShopOwner(pkChrShopKeeper);
#endif
	sys_log(0, "SHOP: START: %s", pkChr->GetName());
	return true;
}

LPSHOP CShopManager::FindPCShop(uint32_t dwVID)
{
	const TShopMap::iterator it = m_map_pkShopByPC.find(dwVID);

	if (it == m_map_pkShopByPC.end())
		return nullptr;

	return it->second;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
LPSHOP CShopManager::CreatePCShop(LPCHARACTER ch, LPCHARACTER owner, TShopItemTable* pTable, uint8_t bItemCount, std::string sign)
{
	if (!ch)
		return nullptr;

	if (FindPCShop(ch->GetVID()))
		return nullptr;

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetPCShop(ch);
	pkShop->TransferItems(owner, pTable, bItemCount);
	pkShop->SetShopItems(pTable, bItemCount);
	pkShop->SetShopSign(sign);

	m_map_pkShopByPC.insert(TShopMap::value_type(ch->GetVID(), pkShop));
	return pkShop;
}
#else
LPSHOP CShopManager::CreatePCShop(LPCHARACTER ch, TShopItemTable* pTable, uint8_t bItemCount)
{
	if (FindPCShop(ch->GetVID()))
		return nullptr;

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetPCShop(ch);
	pkShop->SetShopItems(pTable, bItemCount);

	m_map_pkShopByPC.insert(TShopMap::value_type(ch->GetVID(), pkShop));
	return pkShop;
}
#endif

void CShopManager::DestroyPCShop(LPCHARACTER ch)
{
	if (!ch)
		return;

	LPSHOP pkShop = FindPCShop(ch->GetVID());

	if (!pkShop)
		return;

	//PREVENT_ITEM_COPY;
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	m_map_pkShopByPC.erase(ch->GetVID());
	M2_DELETE(pkShop);
}

// Close the shop transaction
void CShopManager::StopShopping(LPCHARACTER ch)
{
	if (!ch)
		return;

	LPSHOP shop;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (!(shop = ch->GetViewingShop()))
		return;
#else
	if (!(shop = ch->GetShop()))
		return;
#endif

	//PREVENT_ITEM_COPY;
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	shop->RemoveGuest(ch);
	sys_log(0, "SHOP: END: %s", ch->GetName());
}

// Item purchase
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CShopManager::Buy(LPCHARACTER ch, uint8_t pos)
{
	if (!ch)
		return;

#ifdef ENABLE_PREVENT_FLOOD_PACKETS
	const int iPulse = thecore_pulse();
	if (iPulse - ch->GetLastBuyTime() < 10) // 0.5 sec
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1554]Please try again later."));
		return;
	}

	ch->SetLastBuyTime();
#endif

	if (!ch->GetViewingShop())
		return;

#ifdef ENABLE_MINI_GAME_BNW
	// NEED_TEST
	if (ch->GetViewingShopOwner())
	{
		if (DISTANCE_APPROX(ch->GetX() - ch->GetViewingShopOwner()->GetX(), ch->GetY() - ch->GetViewingShopOwner()->GetY()) > 2000)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;877]");
			return;
		}
	}
	// END_OF_NEED_TEST
#else
	if (!ch->GetViewingShopOwner())
		return;

	if (DISTANCE_APPROX(ch->GetX() - ch->GetViewingShopOwner()->GetX(), ch->GetY() - ch->GetViewingShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;877]");
		return;
	}
#endif

	CShop* pkShop = ch->GetViewingShop();

	if (!pkShop)
		return;

	if (pkShop->IsClosed())
	{
		sys_err("Player %lu trying to buy from closed shop.", ch->GetPlayerID());
		return;
	}

	//PREVENT_ITEM_COPY
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	const int ret = pkShop->Buy(ch, pos);

	if (SHOP_SUBHEADER_GC_OK != ret) // If there is a problem, send it.
	{
		TPacketGCShop pack{};

		pack.header = HEADER_GC_SHOP;
		pack.subheader = ret;
		pack.size = sizeof(TPacketGCShop);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}

	//If, after buying, the shop is closed, destroy it (through its owner char)
	if (pkShop->IsClosed() && pkShop->IsPCShop())
	{
		M2_DESTROY_CHARACTER(pkShop->GetOwner());
	}
}
#else
void CShopManager::Buy(LPCHARACTER ch, uint8_t pos)
{
#ifdef ENABLE_NEWSTUFF
	if (0 != g_BuySellTimeLimitValue)
	{
		if (get_dword_time() < ch->GetLastBuySellTime() + g_BuySellTimeLimitValue)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1554]Please try again later."));
			return;
		}
	}

	ch->SetLastBuySellTime(get_dword_time());
#endif

	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner())
		return;

	if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(), ch->GetY() - ch->GetShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[877]You are too far away from the shop to buy something."));
		return;
	}

	CShop* pkShop = ch->GetShop();

	if (!pkShop->IsPCShop())
	{
		/*
		if (pkShop->GetVnum() == 0)
			return;
		const CMob* pkMob = CMobManager::Instance().Get(pkShop->GetNPCVnum());
		if (!pkMob)
			return;

		if (pkMob->m_table.bType != CHAR_TYPE_NPC)
		{
			return;
		}
		*/
	}
	else
	{
	}

	//PREVENT_ITEM_COPY
	ch->SetMyShopTime();
	//END_PREVENT_ITEM_COPY

	int ret = pkShop->Buy(ch, pos);

	if (SHOP_SUBHEADER_GC_OK != ret) // If there is a problem, send it.
	{
		TPacketGCShop pack{};

		pack.header = HEADER_GC_SHOP;
		pack.subheader = ret;
		pack.size = sizeof(TPacketGCShop);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}
}
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
void CShopManager::Sell(LPCHARACTER ch, uint16_t wCell, uint8_t bCount)
#else
void CShopManager::Sell(LPCHARACTER ch, uint8_t bCell, uint8_t bCount)
#endif
{
	if (!ch)
		return;

#ifdef ENABLE_NEWSTUFF
	if (0 != g_BuySellTimeLimitValue)
	{
		if (get_dword_time() < ch->GetLastBuySellTime() + g_BuySellTimeLimitValue)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot drop Yang yet"));
			return;
		}
	}

	ch->SetLastBuySellTime(get_dword_time());
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (!ch->GetViewingShop())
		return;

	if (!ch->GetViewingShopOwner())
		return;
#else
	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner())
		return;
#endif

	if (!ch->CanHandleItem())
	{
#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
		if (ch->IsGrowthPetDetermineWindow())
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1344]");
#endif
		return;
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (ch->GetViewingShop()->IsPCShop())
		return;

	if (DISTANCE_APPROX(ch->GetX() - ch->GetViewingShopOwner()->GetX(), ch->GetY() - ch->GetViewingShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;879]");
		return;
	}
#else
	if (ch->GetShop()->IsPCShop())
		return;

	if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(), ch->GetY() - ch->GetShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;879]");
		return;
	}
#endif

	LPITEM item = ch->GetInventoryItem(wCell);

	if (!item)
		return;

	if (item->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1059]");
		return;
	}

	if (item->isLocked())
	{
		return;
	}

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1113]");
		return;
	}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	if (item->IsBasicItem()) {
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return;
	}
#endif

#ifdef ENABLE_REVERSED_FUNCTIONS
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
	{
		// In clientside the sell is blocked by python if a player arrive here he's a hacker, maybe.
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't sell this item."));
		sys_err("[HACKER] Force sell-script used by name [%u]%s.", ch->GetPlayerID(), ch->GetName());
		return;
	}
#else
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
		return;
#endif

	uint32_t dwPrice = 0;

	if (bCount == 0 || bCount > item->GetCount())
		bCount = item->GetCount();

#ifdef ENABLE_PROTO_RENEWAL
	dwPrice = item->GetShopSellPrice();
#else
	dwPrice = item->GetShopBuyPrice();
#endif

	if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (dwPrice == 0)
			dwPrice = bCount;
		else
			dwPrice = bCount / dwPrice;
	}
	else
		dwPrice *= bCount;

	uint32_t dwTax = 0;

#ifdef ENABLE_NEWSTUFF
	int iVal;
	if (!g_bEmpireShopPriceTripleDisable)
		iVal = 3;
	else
		iVal = 0;
#else
	int iVal = 3;
#endif

	{
		dwTax = dwPrice * iVal / 100;
		dwPrice -= dwTax;
	}

	if (test_server)
		sys_log(0, "Sell Item price id %d %s itemid %d", ch->GetPlayerID(), ch->GetName(), item->GetID());

	const int64_t nTotalMoney = static_cast<int64_t>(ch->GetGold()) + static_cast<int64_t>(dwPrice);

	if (GOLD_MAX <= nTotalMoney)
	{
		sys_err("[OVERFLOW_GOLD] id %u name %s gold %u", ch->GetPlayerID(), ch->GetName(), ch->GetGold());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[879]You cannot trade because you are carrying more than 2 billion Yang."));
		return;
	}

	// 20050802.myevan.Add item ID to store sales log
	sys_log(0, "SHOP: SELL: %s item name: %s(x%d):%u price: %u", ch->GetName(), item->GetName(), bCount, item->GetID(), dwPrice);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	ch->UpdateExtBattlePassMissionProgress(BP_ITEM_SELL, bCount, item->GetVnum());
#endif
	
	if (iVal > 0)
		ch->ChatPacket(CHAT_TYPE_INFO, "[LS;881;%d]", iVal);

	DBManager::Instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), dwPrice);

	if (bCount == item->GetCount())
		ITEM_MANAGER::Instance().RemoveItem(item, "SELL");
	else
		item->SetCount(item->GetCount() - bCount);

	//Monarchy System: Tax Collection
	CMonarch::Instance().SendtoDBAddMoney(dwTax, ch->GetEmpire(), ch);

	ch->PointChange(POINT_GOLD, dwPrice, false);
}

bool CompareShopItemName(const SShopItemTable& lhs, const SShopItemTable& rhs)
{
	const TItemTable* lItem = ITEM_MANAGER::Instance().GetTable(lhs.vnum);
	const TItemTable* rItem = ITEM_MANAGER::Instance().GetTable(rhs.vnum);
	if (lItem && rItem)
		return strcmp(lItem->szLocaleName, rItem->szLocaleName) < 0;
	else
		return true;
}

bool ConvertToShopItemTable(IN CGroupNode* pNode, OUT TShopTableEx& shopTable)
{
	if (!pNode->GetValue("vnum", 0, shopTable.dwVnum))
	{
		sys_err("Group %s does not have vnum.", pNode->GetNodeName().c_str());
		return false;
	}

	if (!pNode->GetValue("name", 0, shopTable.name))
	{
		sys_err("Group %s does not have name.", pNode->GetNodeName().c_str());
		return false;
	}

	if (shopTable.name.length() >= SHOP_TAB_NAME_MAX)
	{
		sys_err("Shop name length must be less than %d. Error in Group %s, name %s", SHOP_TAB_NAME_MAX, pNode->GetNodeName().c_str(), shopTable.name.c_str());
		return false;
	}

	std::string stCoinType;
	if (!pNode->GetValue("cointype", 0, stCoinType))
	{
		stCoinType = "Gold";
	}

	if (boost::iequals(stCoinType, "Gold"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_GOLD;
	}
	else if (boost::iequals(stCoinType, "SecondaryCoin"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_SECONDARY_COIN;
	}
#ifdef ENABLE_RENEWAL_SHOPEX
	else if (boost::iequals(stCoinType, "Item"))
	{
		shopTable.coinType = SHOP_COINT_TYPE_ITEM;
	}
	else if (boost::iequals(stCoinType, "Exp"))
	{
		shopTable.coinType = SHOP_COINT_TYPE_EXP;
	}
#endif
#ifdef ENABLE_BATTLE_FIELD
	else if (boost::iequals(stCoinType, "BattlePoint"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_BATTLE_POINT;
	}
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	else if (boost::iequals(stCoinType, "MedalHonor"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_MEDAL_OF_HONOR;
	}
#endif
#ifdef ENABLE_MINI_GAME_BNW
	else if (boost::iequals(stCoinType, "BNWCoin"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_BNW;
	}
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	else if (boost::iequals(stCoinType, "Achievement"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_ACHIEVEMENT;
	}
#endif
	else
	{
		sys_err("Group %s has undefine cointype(%s).", pNode->GetNodeName().c_str(), stCoinType.c_str());
		return false;
	}

	CGroupNode* pItemGroup = pNode->GetChildNode("items");
	if (!pItemGroup)
	{
		sys_err("Group %s does not have 'group items'.", pNode->GetNodeName().c_str());
		return false;
	}

	const int itemGroupSize = pItemGroup->GetRowCount();
	std::vector <TShopItemTable> shopItems(itemGroupSize);
	if (itemGroupSize >= SHOP_HOST_ITEM_MAX_NUM)
	{
		sys_err("count(%d) of rows of group items of group %s must be smaller than %d", itemGroupSize, pNode->GetNodeName().c_str(), SHOP_HOST_ITEM_MAX_NUM);
		return false;
	}

	for (int i = 0; i < itemGroupSize; i++)
	{
		if (!pItemGroup->GetValue(i, "vnum", shopItems[i].vnum))
		{
			sys_err("row(%d) of group items of group %s does not have vnum column", i, pNode->GetNodeName().c_str());
			return false;
		}

		if (!pItemGroup->GetValue(i, "count", shopItems[i].count))
		{
			sys_err("row(%d) of group items of group %s does not have count column", i, pNode->GetNodeName().c_str());
			return false;
		}
		if (!pItemGroup->GetValue(i, "price", shopItems[i].price))
		{
			sys_err("row(%d) of group items of group %s does not have price column", i, pNode->GetNodeName().c_str());
			return false;
		}
#ifdef ENABLE_MEDAL_OF_HONOR
		if (!pItemGroup->GetValue(i, "limitlevel", shopItems[i].limit_level))
		{
			sys_err("row(%d) of group items of group %s does not have limitlevel column", i, pNode->GetNodeName().c_str());
			return false;
		}
#endif
	}
	std::string stSort;
	if (!pNode->GetValue("sort", 0, stSort))
	{
		stSort = "None";
	}

	if (boost::iequals(stSort, "Asc"))
	{
		std::sort(shopItems.begin(), shopItems.end(), CompareShopItemName);
	}
	else if (boost::iequals(stSort, "Desc"))
	{
		std::sort(shopItems.rbegin(), shopItems.rend(), CompareShopItemName);
	}

	CGrid grid = CGrid(5, 9);	//@infome000
	int iPos;

	msl::refill(shopTable.items);

	for (size_t i = 0; i < shopItems.size(); i++)
	{
		const TItemTable* item_table = ITEM_MANAGER::Instance().GetTable(shopItems[i].vnum);
		if (!item_table)
		{
			sys_err("vnum(%d) of group items of group %s does not exist", shopItems[i].vnum, pNode->GetNodeName().c_str());
			return false;
		}

		iPos = grid.FindBlank(1, item_table->bSize);

		grid.Put(iPos, 1, item_table->bSize);
		shopTable.items[iPos] = shopItems[i];
	}

	shopTable.byItemCount = static_cast<uint8_t>(shopItems.size());
	return true;
}

bool CShopManager::ReadShopTableEx(const char* stFileName)
{
	// Check if file exists.
	// If it does not exist, it is not treated as an error.
	FILE* fp = fopen(stFileName, "rb");
	if (nullptr == fp)
		return true;
	fclose(fp);

	CGroupTextParseTreeLoader loader;
	if (!loader.Load(stFileName))
	{
		sys_err("%s Load fail.", stFileName);
		return false;
	}

	CGroupNode* pShopNPCGroup = loader.GetGroup("shopnpc");
	if (nullptr == pShopNPCGroup)
	{
		sys_err("Group ShopNPC is not exist.");
		return false;
	}

	typedef std::multimap <uint32_t, TShopTableEx> TMapNPCshop;
	TMapNPCshop map_npcShop;
	for (int i = 0; i < pShopNPCGroup->GetRowCount(); i++)
	{
		uint32_t npcVnum;
		std::string shopName;
		if (!pShopNPCGroup->GetValue(i, "npc", npcVnum) || !pShopNPCGroup->GetValue(i, "group", shopName))
		{
			sys_err("Invalid row(%d). Group ShopNPC rows must have 'npc', 'group' columns", i);
			return false;
		}
		std::transform(shopName.begin(), shopName.end(), shopName.begin(), (int(*)(int))std::tolower);
		CGroupNode* pShopGroup = loader.GetGroup(shopName.c_str());
		if (!pShopGroup)
		{
			sys_err("Group %s is not exist.", shopName.c_str());
			return false;
		}
		TShopTableEx table;
		if (!ConvertToShopItemTable(pShopGroup, table))
		{
			sys_err("Cannot read Group %s.", shopName.c_str());
			return false;
		}
		if (m_map_pkShopByNPCVnum.find(npcVnum) != m_map_pkShopByNPCVnum.end())
		{
			sys_err("%d cannot have both original shop and extended shop", npcVnum);
			return false;
		}

		map_npcShop.insert(TMapNPCshop::value_type(npcVnum, table));
	}

	for (TMapNPCshop::iterator it = map_npcShop.begin(); it != map_npcShop.end(); ++it)
	{
		uint32_t npcVnum = it->first;
		TShopTableEx& table = it->second;
		if (m_map_pkShop.find(table.dwVnum) != m_map_pkShop.end())
		{
			sys_err("Shop vnum(%d) already exists", table.dwVnum);
			return false;
		}
		const TShopMap::iterator shop_it = m_map_pkShopByNPCVnum.find(npcVnum);

		LPSHOPEX pkShopEx = nullptr;
		if (m_map_pkShopByNPCVnum.end() == shop_it)
		{
			pkShopEx = M2_NEW CShopEx;
			pkShopEx->Create(0, npcVnum);
			m_map_pkShopByNPCVnum.insert(TShopMap::value_type(npcVnum, pkShopEx));
		}
		else
		{
			pkShopEx = dynamic_cast <CShopEx*> (shop_it->second);
			if (nullptr == pkShopEx)
			{
				sys_err("WTF!!! It can't be happend. NPC(%d) Shop is not extended version.", shop_it->first);
				return false;
			}
		}

		if (pkShopEx->GetTabCount() >= SHOP_TAB_COUNT_MAX)
		{
			sys_err("ShopEx cannot have tab more than %d", SHOP_TAB_COUNT_MAX);
			return false;
		}

		if (m_map_pkShop.find(table.dwVnum) != m_map_pkShop.end()) //@fix
		{
			sys_err("Shop vnum(%d) already exist.", table.dwVnum);
			return false;
		}
		m_map_pkShop.insert(TShopMap::value_type(table.dwVnum, pkShopEx));

		pkShopEx->AddShopTable(table);
	}

	return true;
}
