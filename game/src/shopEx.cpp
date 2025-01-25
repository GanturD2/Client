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
#include "shopEx.h"
#include "group_text_parse_tree.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_MEDAL_OF_HONOR
#	include "guild.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

bool CShopEx::Create(uint32_t dwVnum, uint32_t dwNPCVnum)
{
	m_dwVnum = dwVnum;
	m_dwNPCVnum = dwNPCVnum;
	return true;
}

bool CShopEx::AddShopTable(TShopTableEx& shopTable)
{
	for (auto it = m_vec_shopTabs.begin(); it != m_vec_shopTabs.end(); it++)
	{
		const TShopTableEx& _shopTable = *it;
		if (0 != _shopTable.dwVnum && _shopTable.dwVnum == shopTable.dwVnum)
			return false;
		if (0 != _shopTable.dwNPCVnum && _shopTable.dwNPCVnum == shopTable.dwVnum)
			return false;
	}
	m_vec_shopTabs.emplace_back(shopTable);
	return true;
}

bool CShopEx::AddGuest(LPCHARACTER ch, uint32_t owner_vid, bool bOtherEmpire)
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
	pack.subheader = SHOP_SUBHEADER_GC_START_EX;

	TPacketGCShopStartEx pack2;

	memset(&pack2, 0, sizeof(pack2));

	pack2.owner_vid = owner_vid;
	pack2.shop_tab_count = static_cast<uint8_t>(m_vec_shopTabs.size());

#ifdef ENABLE_RENEWAL_SHOPEX
	char temp[8096 * SHOP_TAB_COUNT_MAX];
#else
	char temp[8096];
#endif
	char* buf = &temp[0];
	size_t size = 0;
	for (auto it = m_vec_shopTabs.begin(); it != m_vec_shopTabs.end(); it++)
	{
		const TShopTableEx& shop_tab = *it;
		TPacketGCShopStartEx::TSubPacketShopTab pack_tab;
		pack_tab.coin_type = shop_tab.coinType;
		memcpy(pack_tab.name, shop_tab.name.c_str(), SHOP_TAB_NAME_MAX);
#if defined(ENABLE_BATTLE_FIELD) && !defined(ENABLE_MEDAL_OF_HONOR)
		pack_tab.limitMaxPoint = LIMIT_DAILY_MAX_BATTLE;
		pack_tab.usablePoints = ch->GetShopExUsablePoint(shop_tab.coinType);
#elif defined(ENABLE_BATTLE_FIELD) && defined(ENABLE_MEDAL_OF_HONOR)
		pack_tab.limitMaxPoint = (shop_tab.coinType == SHOP_COIN_TYPE_BATTLE_POINT) ? LIMIT_DAILY_MAX_BATTLE : LIMIT_DAILY_MAX_MEDALS;
		pack_tab.usablePoints = ch->GetShopExUsablePoint(shop_tab.coinType);
#endif

		for (uint8_t i = 0; i < SHOP_HOST_ITEM_MAX_NUM; i++)
		{
			pack_tab.items[i].vnum = shop_tab.items[i].vnum;
			pack_tab.items[i].count = shop_tab.items[i].count;
			switch (shop_tab.coinType)
			{
				case SHOP_COIN_TYPE_GOLD:
				{
#ifdef ENABLE_NEWSTUFF
					if (bOtherEmpire && !g_bEmpireShopPriceTripleDisable) // no empire price penalty for pc shop
#else
					if (bOtherEmpire) // no empire price penalty for pc shop
#endif
					{
#ifdef ENABLE_CHEQUE_SYSTEM
						pack_tab.items[i].price.dwPrice = shop_tab.items[i].price * 3;
#else
						pack_tab.items[i].price = shop_tab.items[i].price * 3;
#endif
					}
					else
					{
#ifdef ENABLE_CHEQUE_SYSTEM
						pack_tab.items[i].price.dwPrice = shop_tab.items[i].price;
#else
						pack_tab.items[i].price = shop_tab.items[i].price;
#endif
					}
				}
				break;

#ifdef ENABLE_RENEWAL_SHOPEX
				case SHOP_COINT_TYPE_EXP:
				case SHOP_COINT_TYPE_ITEM:
#endif
				case SHOP_COIN_TYPE_SECONDARY_COIN:
#ifdef ENABLE_BATTLE_FIELD
				case SHOP_COIN_TYPE_BATTLE_POINT:
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
				case SHOP_COIN_TYPE_MEDAL_OF_HONOR:
#endif
#ifdef ENABLE_MINI_GAME_BNW
				case SHOP_COIN_TYPE_BNW:
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				case SHOP_COIN_TYPE_ACHIEVEMENT:
#endif
				{
#ifdef ENABLE_CHEQUE_SYSTEM
					pack_tab.items[i].price.dwPrice = shop_tab.items[i].price;
#else
					pack_tab.items[i].price = shop_tab.items[i].price;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
					pack_tab.items[i].limit_level = shop_tab.items[i].limit_level;
#endif
				}
				break;

				default:
					break;
			}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			pack_tab.items[i].dwTransmutationVnum = 0;
#endif
			memset(pack_tab.items[i].aAttr, 0, sizeof(pack_tab.items[i].aAttr));
			memset(pack_tab.items[i].alSockets, 0, sizeof(pack_tab.items[i].alSockets));
#ifdef ENABLE_YOHARA_SYSTEM
			memset(pack_tab.items[i].aApplyRandom, 0, sizeof(pack_tab.items[i].aApplyRandom));
			memset(pack_tab.items[i].alRandomValues, 0, sizeof(pack_tab.items[i].alRandomValues));
#endif
#ifdef ENABLE_REFINE_ELEMENT
			pack_tab.items[i].grade_element = 0;
			memset(pack_tab.items[i].attack_element, 0, sizeof(pack_tab.items[i].attack_element));
			pack_tab.items[i].element_type_bonus = 0;
			memset(pack_tab.items[i].elements_value_bonus, 0, sizeof(pack_tab.items[i].elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
			pack_tab.items[i].set_value = 0;
#endif
		}

		memcpy(buf, &pack_tab, sizeof(pack_tab));
		buf += sizeof(pack_tab);
		size += sizeof(pack_tab);
	}

	pack.size = static_cast<uint16_t>(sizeof(pack) + sizeof(pack2) + size);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->BufferedPacket(&pack2, sizeof(TPacketGCShopStartEx));
	ch->GetDesc()->Packet(temp, size);

	return true;
}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
int CShopEx::Buy(LPCHARACTER ch, uint8_t pos, bool isPrivateShopSearch)
#else
int CShopEx::Buy(LPCHARACTER ch, uint8_t pos)
#endif
{
	const uint8_t tabIdx = pos / SHOP_HOST_ITEM_MAX_NUM;
	const uint8_t slotPos = pos % SHOP_HOST_ITEM_MAX_NUM;
	if (tabIdx >= GetTabCount())
	{
		sys_log(0, "ShopEx::Buy : invalid position %d : %s", pos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "ShopEx::Buy : name %s pos %d", ch->GetName(), pos);

	const GuestMapType::iterator it = m_map_guest.find(ch);

	if (it == m_map_guest.end())
		return SHOP_SUBHEADER_GC_END;

	const TShopTableEx& shopTab = m_vec_shopTabs[tabIdx];
	const TShopItemTable& r_item = shopTab.items[slotPos];

	if (r_item.price < 0)
	{
		LogManager::Instance().HackLog("SHOP_BUY_GOLD_OVERFLOW", ch);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	int dwPrice = r_item.price;
#ifdef ENABLE_MEDAL_OF_HONOR
	const uint8_t dwLimitLevel = r_item.limit_level;
#endif

	switch (shopTab.coinType)
	{
		case SHOP_COIN_TYPE_GOLD:
		{
			if (it->second) // if other empire, price is triple
				dwPrice *= 3;

			if (ch->GetGold() < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Not enough money : %s has %d, price %d", ch->GetName(), ch->GetGold(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
			}
		}
		break;

		case SHOP_COIN_TYPE_SECONDARY_COIN:
		{
			const int count = ch->CountSpecifyTypeItem(ITEM_SECONDARY_COIN);
			if (count < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Not enough myeongdojun : %s has %d, price %d", ch->GetName(), count, dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX;
			}
		}
		break;

#ifdef ENABLE_RENEWAL_SHOPEX
		case SHOP_COINT_TYPE_ITEM: // item
		{
			uint32_t count = ch->CountSpecifyItem(dwPrice);
			if (count < r_item.count) {
				sys_log(1, "ShopEx::Buy : Not enough items : %s has %d, item %d", ch->GetName(), count, dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM;
			}
		}
		break;

		case SHOP_COINT_TYPE_EXP: // exp
		{
			if (ch->GetExp() < static_cast<uint32_t>(dwPrice)) {
				sys_log(1, "ShopEx::Buy : Not enough exp : %s has %d, exp %d", ch->GetName(), ch->GetExp(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_EXP;
			}
		}
		break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case SHOP_COIN_TYPE_BATTLE_POINT:
		{
			if (ch->GetBattlePoint() < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Not enough battle point : %s has %d, price %d", ch->GetName(), ch->GetBattlePoint(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_BATTLE_POINTS;
			}

			if (ch->GetShopExUsablePoint(shopTab.coinType) < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Exceed battle point limit today : %s has %d, price %d", ch->GetName(), ch->GetShopExUsablePoint(shopTab.coinType), dwPrice);
				return SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY;
			}
		}
		break;
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
		case SHOP_COIN_TYPE_MEDAL_OF_HONOR:
		{
			const CGuild* pGuild = ch->GetGuild();
			if (!pGuild)
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_GUILD_LEVEL;

			if (pGuild->GetLevel() < dwLimitLevel)
			{
				sys_log(1, "ShopEx::Buy : Not enough guild level : player %s with guild %s has guild level %d level_required %d", ch->GetName(), pGuild->GetName(), pGuild->GetLevel(), dwLimitLevel);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_GUILD_LEVEL;
			}

			if (ch->GetMedalHonor() < static_cast<long>(dwPrice))
			{
				sys_log(1, "ShopEx::Buy : Not enough medals of honor : %s has %d, price %d", ch->GetName(), ch->GetMedalHonor(), dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_MEDAL_OF_HONOR;
			}

			if (ch->GetShopExUsablePoint(shopTab.coinType) < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Exceed limit medals spent today : %s has %d, price %d", ch->GetName(), ch->GetShopExUsablePoint(shopTab.coinType), dwPrice);
				return SHOP_SUBHEADER_GC_EXCEED_LIMIT_TODAY;
			}
		}
		break;
#endif

#ifdef ENABLE_MINI_GAME_BNW
		case SHOP_COIN_TYPE_BNW:
		{
			const int count = ch->CountSpecifyItem(BNW_REWARD_VNUM);
			if (count < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Not enough Seer(bnw) Coin : %s has %d, price %d", ch->GetName(), count, dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_BNW;
			}
		}
		break;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case SHOP_COIN_TYPE_ACHIEVEMENT:
		{
			auto count = ch->GetAchievementPoints();
			if (count < dwPrice)
			{
				sys_log(1, "ShopEx::Buy : Not enough achievement : %s has %d, price %d", ch->GetName(), count, dwPrice);
				return SHOP_SUBHEADER_GC_NOT_ENOUGH_ACHIEVEMENT;
			}
		}
		break;
#endif

		default:
			break;
	}

	LPITEM item;

	item = ITEM_MANAGER::Instance().CreateItem(r_item.vnum, r_item.count);

	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

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
		sys_log(1, "ShopEx::Buy : Inventory full : %s size %d", ch->GetName(), item->GetSize());
		M2_DESTROY_ITEM(item);
		return SHOP_SUBHEADER_GC_INVENTORY_FULL;
	}

	switch (shopTab.coinType)
	{
		case SHOP_COIN_TYPE_GOLD:
			ch->PointChange(POINT_GOLD, -dwPrice, false);
			break;

		case SHOP_COIN_TYPE_SECONDARY_COIN:
			ch->RemoveSpecifyTypeItem(ITEM_SECONDARY_COIN, dwPrice);
			break;

#ifdef ENABLE_RENEWAL_SHOPEX
		case SHOP_COINT_TYPE_ITEM: // item
			ch->RemoveSpecifyItem(dwPrice, r_item.count);
			break;
		case SHOP_COINT_TYPE_EXP: // exp
			ch->PointChange(POINT_EXP, -dwPrice, false);
			break;
#endif

#ifdef ENABLE_BATTLE_FIELD
		case SHOP_COIN_TYPE_BATTLE_POINT:
		{
			ch->PointChange(POINT_BATTLE_FIELD, -dwPrice, false);
			ch->SetShopExUsablePoint(shopTab.coinType, (ch->GetShopExUsablePoint(shopTab.coinType) - dwPrice));
		}
		break;
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
		case SHOP_COIN_TYPE_MEDAL_OF_HONOR:
		{
			ch->PointChange(POINT_MEDAL_OF_HONOR, -dwPrice, false);
			ch->SetShopExUsablePoint(shopTab.coinType, (ch->GetShopExUsablePoint(shopTab.coinType) - dwPrice));
		}
		break;
#endif
#ifdef ENABLE_MINI_GAME_BNW
		case SHOP_COIN_TYPE_BNW:
			ch->RemoveSpecifyItem(BNW_REWARD_VNUM, dwPrice);
			break;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		case SHOP_COIN_TYPE_ACHIEVEMENT:
			CAchievementSystem::Instance().ChangeAchievementPoints(ch, -dwPrice);
			break;
#endif

		default:
			break;
	}

	if (item->IsDragonSoul())
		item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
	else
		item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));

	ITEM_MANAGER::Instance().FlushDelayedSave(item);
	LogManager::Instance().ItemLog(ch, item, "BUY", item->GetName());

	if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
	{
		LogManager::Instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), PERSONAL_SHOP_BUY, "");
	}

	DBManager::Instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), -dwPrice);

	if (item)
		sys_log(0, "ShopEx: BUY: name %s %s(x %d):%u price %u", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);

#ifdef ENABLE_FLUSH_CACHE_FEATURE // @warme006
	{
		ch->SaveReal();
		db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
		uint32_t pid = ch->GetPlayerID();
		db_clientdesc->Packet(&pid, sizeof(uint32_t));
	}
#else
	{
		ch->Save();
	}
#endif

	return (SHOP_SUBHEADER_GC_OK);
}
