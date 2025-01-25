#include "stdafx.h"
#include "../../libgame/include/grid.h"
#include "utils.h"
#include "desc.h"
#include "desc_client.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "locale_service.h"
#include "../../common/length.h"
#include "exchange.h"
#include "DragonSoul.h"
#include "questmanager.h" // @fixme150
#ifdef ENABLE_MESSENGER_BLOCK
#	include "messenger_manager.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif


void exchange_packet(LPCHARACTER ch, uint8_t sub_header, bool is_me, uint32_t arg1, TItemPos arg2, uint32_t arg3, void * pvData = nullptr
#ifdef ENABLE_CHEQUE_SYSTEM
	, uint32_t cheque = 0
#endif
);

extern bool trade_effect;
extern int trade_effect_exchange_threshold;

void exchange_packet(LPCHARACTER ch, uint8_t sub_header, bool is_me, uint32_t arg1, TItemPos arg2, uint32_t arg3, void * pvData
#ifdef ENABLE_CHEQUE_SYSTEM
	, uint32_t cheque
#endif
	)
{
	if (!ch->GetDesc())
		return;

	TPacketGCExchange pack_exchg;
	pack_exchg.header 		= HEADER_GC_EXCHANGE;
	pack_exchg.sub_header 	= sub_header;
	pack_exchg.is_me		= is_me;
	pack_exchg.arg1		= arg1;
	pack_exchg.arg2		= arg2;
	pack_exchg.arg3		= arg3;
#ifdef ENABLE_CHEQUE_SYSTEM
	pack_exchg.cheque = cheque;
#endif

	if (sub_header == EXCHANGE_SUBHEADER_GC_ITEM_ADD && pvData)
	{
#ifdef WJ_ENABLE_TRADABLE_ICON
		pack_exchg.arg4 = TItemPos(((LPITEM)pvData)->GetWindow(), ((LPITEM)pvData)->GetCell());
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		pack_exchg.dwTransmutationVnum = static_cast<LPITEM>(pvData)->GetChangeLookVnum();
#endif
#ifdef ENABLE_REFINE_ELEMENT
		pack_exchg.grade_element = ((LPITEM)pvData)->GetElementGrade();
		pack_exchg.element_type_bonus = ((LPITEM)pvData)->GetElementsType();
		thecore_memcpy(&pack_exchg.attack_element, ((LPITEM)pvData)->GetElementAttacks(), sizeof(pack_exchg.attack_element));
		thecore_memcpy(&pack_exchg.elements_value_bonus, ((LPITEM)pvData)->GetElementsValues(), sizeof(pack_exchg.elements_value_bonus));
#endif
		thecore_memcpy(&pack_exchg.alSockets, ((LPITEM) pvData)->GetSockets(), sizeof(pack_exchg.alSockets));
		thecore_memcpy(&pack_exchg.aAttr, ((LPITEM) pvData)->GetAttributes(), sizeof(pack_exchg.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
		thecore_memcpy(&pack_exchg.aApplyRandom, ((LPITEM)pvData)->GetApplysRandom(), sizeof(pack_exchg.aApplyRandom));
		thecore_memcpy(&pack_exchg.alRandomValues, ((LPITEM)pvData)->GetRandomDefaultAttrs(), sizeof(pack_exchg.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
		pack_exchg.set_value = ((LPITEM)pvData)->GetItemSetValue();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		pack_exchg.aPetInfo = ((LPITEM)pvData)->GetGrowthPetItemInfo();
#endif
	}
	else
	{
#ifdef WJ_ENABLE_TRADABLE_ICON
		pack_exchg.arg4 = TItemPos(RESERVED_WINDOW, 0);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		pack_exchg.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT
		pack_exchg.grade_element = 0;
		pack_exchg.element_type_bonus = 0;
		memset(&pack_exchg.attack_element, 0, sizeof(pack_exchg.attack_element));
		memset(&pack_exchg.elements_value_bonus, 0, sizeof(pack_exchg.elements_value_bonus));
#endif
		memset(&pack_exchg.alSockets, 0, sizeof(pack_exchg.alSockets));
		memset(&pack_exchg.aAttr, 0, sizeof(pack_exchg.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
		memset(&pack_exchg.aApplyRandom, 0, sizeof(pack_exchg.aApplyRandom));
		memset(&pack_exchg.alRandomValues, 0, sizeof(pack_exchg.alRandomValues));
#endif
#ifdef ENABLE_SET_ITEM
		pack_exchg.set_value = 0;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		memset(&pack_exchg.aPetInfo, 0, sizeof(pack_exchg.aPetInfo));
#endif
	}

	ch->GetDesc()->Packet(&pack_exchg, sizeof(pack_exchg));
}

bool CHARACTER::ExchangeStart(LPCHARACTER victim)
{
	if (this == victim)
		return false;

	if (IsObserverMode())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;893]");
		return false;
	}

	if (victim->IsNPC())
		return false;
	
#ifdef ENABLE_MESSENGER_BLOCK
	if (MessengerManager::Instance().IsBlocked(GetName(), victim->GetName()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Unblock %s to continue."), victim->GetName());
		return false;
	}
	else if (MessengerManager::Instance().IsBlocked(victim->GetName(), GetName()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has blocked you."), victim->GetName());
		return false;
	}
#endif

	//PREVENT_TRADE_WINDOW
	if (IsOpenSafebox() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		GetViewingShopOwner() || 
#else
		GetShopOwner() || GetMyShop() || 
#endif
		IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| IsOpenGuildstorage()
#endif
#ifdef ENABLE_AURA_SYSTEM
		|| IsAuraRefineWindowOpen()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| IsChangeLookWindowOpen()
#endif
#ifdef ENABLE_MAILBOX
		|| GetMailBox()
#endif
#ifdef ENABLE_CHANGED_ATTR
		|| IsSelectAttr()
#endif
	)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;894]");
		return false;
	}

#ifdef ENABLE_GM_BLOCK
	if (IsGM() && !GetStaffPermissions())
	{
		ChatPacket(CHAT_TYPE_INFO, "[GM] GM's k?nen nicht handeln!");
		return false;
	}
	if (victim->IsGM() && !victim->GetStaffPermissions())
	{
		//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Staff cannot exchange."));
		return false;
	}
#endif

	if ( victim->IsOpenSafebox() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		victim->GetViewingShopOwner() || 
#else
		victim->GetShopOwner() || 
#endif
		victim->GetMyShop() || victim->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| victim->IsOpenGuildstorage()
#endif
#ifdef ENABLE_AURA_SYSTEM
		|| victim->IsAuraRefineWindowOpen()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| victim->IsChangeLookWindowOpen()
#endif
#ifdef ENABLE_MAILBOX
		|| victim->GetMailBox()
#endif
#ifdef ENABLE_CHANGED_ATTR
		|| victim->IsSelectAttr()
#endif
	)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;895]");
		return false;
	}
	//END_PREVENT_TRADE_WINDOW
	int iDist = DISTANCE_APPROX(GetX() - victim->GetX(), GetY() - victim->GetY());

	if (iDist >= EXCHANGE_MAX_DISTANCE)
		return false;

	if (GetExchange())
		return false;

	if (victim->GetExchange())
	{
		exchange_packet(this, EXCHANGE_SUBHEADER_GC_ALREADY, 0, 0, NPOS, 0);
		return false;
	}

	if (victim->IsBlockMode(BLOCK_EXCHANGE))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;896]");
		return false;
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (GetGrowthPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1166]");
		return false;
	}

	if (victim->GetGrowthPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1167]");
		return false;
	}

	if (victim->IsGrowthPetHatchingWindow())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1163]");
		return false;
	}

#	ifdef ENABLE_PET_ATTR_DETERMINE
	if (IsGrowthPetDetermineWindow())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1344]");
		return false;
	}

	if (victim->IsGrowthPetDetermineWindow())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1348]");
		return false;
	}
#	endif
#endif

	SetExchange(M2_NEW CExchange(this));
	victim->SetExchange(M2_NEW CExchange(victim));

	victim->GetExchange()->SetCompany(GetExchange());
	GetExchange()->SetCompany(victim->GetExchange());

	//
	SetExchangeTime();
	victim->SetExchangeTime();

	exchange_packet(victim, EXCHANGE_SUBHEADER_GC_START, 0, GetVID(), NPOS, 0);
	exchange_packet(this, EXCHANGE_SUBHEADER_GC_START, 0, victim->GetVID(), NPOS, 0);

	return true;
}

CExchange::CExchange(LPCHARACTER pOwner)
{
	m_pCompany = nullptr;

	m_bAccept = false;

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		m_apItems[i] = nullptr;
		m_aItemPos[i] = NPOS;
		m_abItemDisplayPos[i] = 0;
	}

	m_lGold = 0;
#ifdef ENABLE_CHEQUE_SYSTEM
	m_lCheque = 0;
#endif

	m_pOwner = pOwner;
	pOwner->SetExchange(this);

	m_pGrid = M2_NEW CGrid(EXCHANGE_PAGE_COLUMN, EXCHANGE_PAGE_ROW);
}

CExchange::~CExchange()
{
	M2_DELETE(m_pGrid);
}

bool CExchange::AddItem(TItemPos item_pos, uint8_t display_pos)
{
	assert(m_pOwner != nullptr && GetCompany());

	if (!item_pos.IsValidItemPosition())
		return false;

	if (item_pos.IsEquipPosition())
		return false;

	LPITEM item;

	if (!(item = m_pOwner->GetItem(item_pos)))
		return false;

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed()) {
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_SEALED_CANNOT_DO"));
		return false;
	}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	if (item->IsBasicItem()) {
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE))
	{
		m_pOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;710]");
		return false;
	}

	if (true == item->isLocked())
	{
		return false;
	}

	if (item->IsExchanging())
	{
		sys_log(0, "EXCHANGE under exchanging");
		return false;
	}

	if (!m_pGrid->IsEmpty(display_pos, 1, item->GetSize()))
	{
		sys_log(0, "EXCHANGE not empty item_pos %d %d %d", display_pos, 1, item->GetSize());
		return false;
	}

	Accept(false);
	GetCompany()->Accept(false);

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (m_apItems[i])
			continue;

		m_apItems[i]		= item;
		m_aItemPos[i]		= item_pos;
		m_abItemDisplayPos[i]	= display_pos;
		m_pGrid->Put(display_pos, 1, item->GetSize());

		item->SetExchanging(true);

		exchange_packet(m_pOwner,
				EXCHANGE_SUBHEADER_GC_ITEM_ADD,
				true,
				item->GetVnum(),
				TItemPos(RESERVED_WINDOW, display_pos),
				item->GetCount(),
				item);

		exchange_packet(GetCompany()->GetOwner(),
				EXCHANGE_SUBHEADER_GC_ITEM_ADD,
				false,
				item->GetVnum(),
				TItemPos(RESERVED_WINDOW, display_pos),
				item->GetCount(),
				item);

		sys_log(0, "EXCHANGE AddItem success %s pos(%d, %d) %d", item->GetName(), item_pos.window_type, item_pos.cell, display_pos);

		return true;
	}

	return false;
}

bool CExchange::RemoveItem(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return false;

	if (!m_apItems[pos])
		return false;

	TItemPos PosOfInventory = m_aItemPos[pos];
	m_apItems[pos]->SetExchanging(false);

	m_pGrid->Get(m_abItemDisplayPos[pos], 1, m_apItems[pos]->GetSize());

	exchange_packet(GetOwner(),	EXCHANGE_SUBHEADER_GC_ITEM_DEL, true, pos, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ITEM_DEL, false, pos, PosOfInventory, 0);

	Accept(false);
	GetCompany()->Accept(false);

	m_apItems[pos]	    = nullptr;
	m_aItemPos[pos]	    = NPOS;
	m_abItemDisplayPos[pos] = 0;
	return true;
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CExchange::AddGold(long gold, uint32_t cheque)
#else
bool CExchange::AddGold(long gold)
#endif
{
	if (gold <= 0
#ifdef ENABLE_CHEQUE_SYSTEM
		&& cheque <= 0
#endif
		)
	{
		return false;
	}

	if (GetOwner()->GetGold() < gold
#ifdef ENABLE_CHEQUE_SYSTEM
		&& GetOwner()->GetCheque() < cheque
#endif
		)
	{
		// Lack of money you have.
#ifdef ENABLE_CHEQUE_SYSTEM
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_GOLD, 0, 0, NPOS, 0, nullptr, 0);
#else
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_LESS_GOLD, 0, 0, NPOS, 0);
#endif
		return false;
	}

	if (m_lGold > 0
#ifdef ENABLE_CHEQUE_SYSTEM
		&& m_lCheque > 0
#endif
		)
	{
		return false;
	}

	Accept(false);
	GetCompany()->Accept(false);

	m_lGold = gold;
#ifdef ENABLE_CHEQUE_SYSTEM
	m_lCheque = cheque;
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, true, m_lGold, NPOS, 0, nullptr, m_lCheque);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, false, m_lGold, NPOS, 0, nullptr, m_lCheque);
#else
	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, true, m_lGold, NPOS, 0);
	exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_GOLD_ADD, false, m_lGold, NPOS, 0);
#endif

	return true;
}

bool CExchange::Check(int * piItemCount)
{
	if (GetOwner()->GetGold() < m_lGold)
		return false;

#ifdef ENABLE_CHEQUE_SYSTEM
	if (GetOwner()->GetCheque() < m_lCheque)
		return false;
#endif

	int item_count = 0;

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (!m_apItems[i])
			continue;

		if (!m_aItemPos[i].IsValidItemPosition())
			return false;

		if (m_apItems[i] != GetOwner()->GetItem(m_aItemPos[i]))
			return false;

		++item_count;
	}

	*piItemCount = item_count;
	return true;
}

bool CExchange::CheckSpace()
{
	static CGrid s_grid1(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 1
	static CGrid s_grid2(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 2
#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_EXTEND_INVEN_PAGE_SYSTEM)
	static CGrid s_grid3(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 3
	static CGrid s_grid4(INVENTORY_PAGE_COLUMN, INVENTORY_PAGE_ROW); // inven page 4
#endif

	s_grid1.Clear();
	s_grid2.Clear();
#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_EXTEND_INVEN_PAGE_SYSTEM)
	s_grid3.Clear();
	s_grid4.Clear();
#endif

	LPCHARACTER	victim = GetCompany()->GetOwner();
	LPITEM item;

	int i;

	for (i = 0; i < INVENTORY_PAGE_SIZE*1; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid1.Put(i, 1, item->GetSize());
	}
	for (i = INVENTORY_PAGE_SIZE * 1; i < INVENTORY_PAGE_SIZE * 2; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid2.Put(i - INVENTORY_PAGE_SIZE * 1, 1, item->GetSize());
	}
#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_EXTEND_INVEN_PAGE_SYSTEM)
	for (i = INVENTORY_PAGE_SIZE * 2; i < INVENTORY_PAGE_SIZE * 3; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid3.Put(i - INVENTORY_PAGE_SIZE * 2, 1, item->GetSize());
	}
	for (i = INVENTORY_PAGE_SIZE * 3; i < INVENTORY_PAGE_SIZE * 4; ++i)
	{
		if (!(item = victim->GetInventoryItem(i)))
			continue;

		s_grid4.Put(i - INVENTORY_PAGE_SIZE * 3, 1, item->GetSize());
	}
#endif

	static std::vector <uint16_t> s_vDSGrid(DRAGON_SOUL_INVENTORY_MAX_NUM);

	bool bDSInitialized = false;

	for (i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (!(item = m_apItems[i]))
			continue;

		if (item->IsDragonSoul())
		{
			if (!victim->DragonSoul_IsQualified())
			{
				return false;
			}

			if (!bDSInitialized)
			{
				bDSInitialized = true;
				victim->CopyDragonSoulItemGrid(s_vDSGrid);
			}

			bool bExistEmptySpace = false;
			uint16_t wBasePos = DSManager::Instance().GetBasePosition(item);
			if (wBasePos >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				return false;

			for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; i++)
			{
				uint16_t wPos = wBasePos + i;
				if (0 == s_vDSGrid[wPos]) // @fixme159 (wBasePos to wPos)
				{
					bool bEmpty = true;
					for (int j = 1; j < item->GetSize(); j++)
					{
						if (s_vDSGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM])
						{
							bEmpty = false;
							break;
						}
					}
					if (bEmpty)
					{
						for (int j = 0; j < item->GetSize(); j++)
						{
							s_vDSGrid[wPos + j * DRAGON_SOUL_BOX_COLUMN_NUM] =  wPos + 1;
						}
						bExistEmptySpace = true;
						break;
					}
				}
				if (bExistEmptySpace)
					break;
			}
			if (!bExistEmptySpace)
				return false;
		}
		else
		{
			int iPos;

			if ((iPos = s_grid1.FindBlank(1, item->GetSize())) >= 0)
			{
				s_grid1.Put(iPos, 1, item->GetSize());
			}
			else if ((iPos = s_grid2.FindBlank(1, item->GetSize())) >= 0)
			{
				s_grid2.Put(iPos, 1, item->GetSize());
			}
#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_EXTEND_INVEN_PAGE_SYSTEM)
			else if ((iPos = s_grid3.FindBlank(1, item->GetSize())) >= 0)
			{
#	ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
				int wSlotPos = (victim->GetExtendInvenStage() * INVENTORY_PAGE_COLUMN) - 1;
				if (iPos > wSlotPos)
					return false;

				if (item->GetSize() > 1 && iPos > wSlotPos - (INVENTORY_PAGE_COLUMN * (item->GetSize() - 1)))
					return false;

#	endif
				s_grid3.Put(iPos, 1, item->GetSize());
			}
			else if ((iPos = s_grid4.FindBlank(1, item->GetSize())) >= 0)
			{
#	ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
				int wSlotPos = ((victim->GetExtendInvenStage() - INVENTORY_PAGE_ROW) * INVENTORY_PAGE_COLUMN) - 1;
				if (iPos > wSlotPos)
					return false;

				if (item->GetSize() > 1 && iPos > wSlotPos - (INVENTORY_PAGE_COLUMN * (item->GetSize() - 1)))
#	endif
					s_grid4.Put(iPos, 1, item->GetSize());
			}
#endif
			else
				return false;
		}
	}

	return true;
}

bool CExchange::Done()
{
	int empty_pos;
	LPITEM item;

	LPCHARACTER victim = GetCompany()->GetOwner();

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (!(item = m_apItems[i]))
			continue;

		if (item->IsDragonSoul())
			empty_pos = victim->GetEmptyDragonSoulInventory(item);
		else
#ifdef ENABLE_SPECIAL_INVENTORY
			empty_pos = victim->GetEmptyInventory(item);
#else
			empty_pos = victim->GetEmptyInventory(item->GetSize());
#endif

		if (empty_pos < 0)
		{
			sys_err("Exchange::Done : Cannot find blank position in inventory %s <-> %s item %s", m_pOwner->GetName(), victim->GetName(), item->GetName());

			m_pOwner->ChatPacket(CHAT_TYPE_INFO, "cant do, because no space to get Item");
			return false;
		}

		assert(empty_pos >= 0);

		m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

		item->RemoveFromCharacter();
		if (item->IsDragonSoul())
			item->AddToCharacter(victim, TItemPos(DRAGON_SOUL_INVENTORY, empty_pos));
		else
			item->AddToCharacter(victim, TItemPos(INVENTORY, empty_pos));
		ITEM_MANAGER::Instance().FlushDelayedSave(item);

#ifdef ENABLE_EFFECT_FOR_TRADE
		if (trade_effect)	//@custom020
			GetOwner()->CreateFly(5, victim);
#endif

		item->SetExchanging(false);
		{
			char exchange_buf[51];

			snprintf(exchange_buf, sizeof(exchange_buf), "%s %u %u", item->GetName(), GetOwner()->GetPlayerID(), item->GetCount());
			LogManager::Instance().ItemLog(victim, item, "EXCHANGE_TAKE", exchange_buf);

			snprintf(exchange_buf, sizeof(exchange_buf), "%s %u %u", item->GetName(), victim->GetPlayerID(), item->GetCount());
			LogManager::Instance().ItemLog(GetOwner(), item, "EXCHANGE_GIVE", exchange_buf);

			if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
			{
				LogManager::Instance().GoldBarLog(victim->GetPlayerID(), item->GetID(), EXCHANGE_TAKE, "");
				LogManager::Instance().GoldBarLog(GetOwner()->GetPlayerID(), item->GetID(), EXCHANGE_GIVE, "");
			}
		}

		m_apItems[i] = nullptr;
	}

	if (m_lGold)
	{
#ifdef ENABLE_EFFECT_FOR_TRADE
		if (trade_effect)
			for (int i = 1; i < m_lGold + 1; i = i + trade_effect_exchange_threshold)	//@custom020
				GetOwner()->CreateFly(4, victim);  // 1 Fly pro 500k . dh max 20 auf einmal (TradeLimit: 10kkk)
#endif

		GetOwner()->PointChange(POINT_GOLD, -m_lGold, true);
		victim->PointChange(POINT_GOLD, m_lGold, true);

		if (m_lGold > 1000)
		{
			char exchange_buf[51];
			snprintf(exchange_buf, sizeof(exchange_buf), "%u %s", GetOwner()->GetPlayerID(), GetOwner()->GetName());
			LogManager::Instance().CharLog(victim, m_lGold, "EXCHANGE_GOLD_TAKE", exchange_buf);

			snprintf(exchange_buf, sizeof(exchange_buf), "%u %s", victim->GetPlayerID(), victim->GetName());
			LogManager::Instance().CharLog(GetOwner(), m_lGold, "EXCHANGE_GOLD_GIVE", exchange_buf);
		}
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	if (m_lCheque)
	{
		if ((victim->GetCheque() + m_lCheque) >= CHEQUE_MAX)
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The trade can't be done because the other character exceeds %d won"), CHEQUE_MAX - 1);
			sys_err("[OVERFLOW_CHEQUE] CHEQUE_ADD (%u) id %u name %s ", m_lCheque, victim->GetPlayerID(), victim->GetName());
			return false;
		}

#ifdef ENABLE_EFFECT_FOR_TRADE
		if (!GetOwner()->IsGM() && !victim->IsGM() && trade_effect)	//@custom020
		{
			for (int i = 1; i < static_cast<int>(m_lCheque) + 1; i = i + 999)
				GetOwner()->CreateFly(6, victim);
		}
#endif

		GetOwner()->PointChange(POINT_CHEQUE, -m_lCheque, true);
		victim->PointChange(POINT_CHEQUE, m_lCheque, true);

		if (m_lCheque > 5)
		{
			char exchange_buf[51];
			snprintf(exchange_buf, sizeof(exchange_buf), "%u %s", GetOwner()->GetPlayerID(), GetOwner()->GetName());
			LogManager::Instance().CharLog(victim, m_lCheque, "EXCHANGE_CHEQUE_TAKE", exchange_buf);

			snprintf(exchange_buf, sizeof(exchange_buf), "%u %s", victim->GetPlayerID(), victim->GetName());
			LogManager::Instance().CharLog(GetOwner(), m_lCheque, "EXCHANGE_CHEQUE_GIVE", exchange_buf);
		}
	}
#endif

	m_pGrid->Clear();
	return true;
}

bool CExchange::Accept(bool bAccept)
{
	if (m_bAccept == bAccept)
		return true;

	m_bAccept = bAccept;

	if (m_bAccept && GetCompany()->m_bAccept)
	{
		int	iItemCount;

		LPCHARACTER victim = GetCompany()->GetOwner();

		//PREVENT_PORTAL_AFTER_EXCHANGE
		GetOwner()->SetExchangeTime();
		victim->SetExchangeTime();
		//END_PREVENT_PORTAL_AFTER_EXCHANGE

		// @fixme150 BEGIN
		if (quest::CQuestManager::Instance().GetPCForce(GetOwner()->GetPlayerID())->IsRunning() == true)
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if you're using quests"));
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if the other part using quests"));
			goto EXCHANGE_END;
		}
		else if (quest::CQuestManager::Instance().GetPCForce(victim->GetPlayerID())->IsRunning() == true)
		{
			victim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if you're using quests"));
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot trade if the other part using quests"));
			goto EXCHANGE_END;
		}
		// @fixme150 END

		if (!Check(&iItemCount))
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "[LS;897]");
			victim->ChatPacket(CHAT_TYPE_INFO, "[LS;898]");
			goto EXCHANGE_END;
		}

		if (!CheckSpace())
		{
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "[LS;899]");
			victim->ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
			goto EXCHANGE_END;
		}

		if (!GetCompany()->Check(&iItemCount))
		{
			victim->ChatPacket(CHAT_TYPE_INFO, "[LS;897]");
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "[LS;898]");
			goto EXCHANGE_END;
		}

		if (!GetCompany()->CheckSpace())
		{
			victim->ChatPacket(CHAT_TYPE_INFO, "[LS;899]");
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
			goto EXCHANGE_END;
		}

		if (db_clientdesc->GetSocket() == INVALID_SOCKET)
		{
			sys_err("Cannot use exchange feature while DB cache connection is dead.");
			victim->ChatPacket(CHAT_TYPE_INFO, "Unknown error");
			GetOwner()->ChatPacket(CHAT_TYPE_INFO, "Unknown error");
			goto EXCHANGE_END;
		}

		if (Done())
		{
			if (m_lGold)
				GetOwner()->Save();

#ifdef ENABLE_CHEQUE_SYSTEM
			if (m_lCheque)
				GetOwner()->Save();
#endif

			if (GetCompany()->Done())
			{
				if (GetCompany()->m_lGold)
					victim->Save();

#ifdef ENABLE_CHEQUE_SYSTEM
				if (GetCompany()->m_lCheque)
					victim->Save();
#endif

				// INTERNATIONAL_VERSION
				GetOwner()->ChatPacket(CHAT_TYPE_INFO, "[LS;900;%s]", victim->GetName());
				victim->ChatPacket(CHAT_TYPE_INFO, "[LS;900;%s]", GetOwner()->GetName());
				// END_OF_INTERNATIONAL_VERSION

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
				if (GetOwner())
					CAchievementSystem::Instance().OnTrade(GetOwner());

				if (victim)
					CAchievementSystem::Instance().OnTrade(victim);
#endif
			}
		}

EXCHANGE_END:
		Cancel();
		return false;
	}
	else
	{
		exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, true, m_bAccept, NPOS, 0);
		exchange_packet(GetCompany()->GetOwner(), EXCHANGE_SUBHEADER_GC_ACCEPT, false, m_bAccept, NPOS, 0);
		return true;
	}
}

void CExchange::Cancel()
{
	exchange_packet(GetOwner(), EXCHANGE_SUBHEADER_GC_END, 0, 0, NPOS, 0);
	GetOwner()->SetExchange(nullptr);

	for (int i = 0; i < EXCHANGE_ITEM_MAX_NUM; ++i)
	{
		if (m_apItems[i])
			m_apItems[i]->SetExchanging(false);
	}

	if (GetCompany())
	{
		GetCompany()->SetCompany(nullptr);
		GetCompany()->Cancel();
	}

	M2_DELETE(this);
}

