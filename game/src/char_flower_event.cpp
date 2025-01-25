#include "stdafx.h"

#ifdef ENABLE_FLOWER_EVENT
#include "char.h"
#include "item_manager.h"
#include "item.h"
#include "utils.h"

void CHARACTER::SendFlowerEventExchange(uint8_t id)
{
	const uint32_t count_flower = GetFlowerEventValue(id);
	if (count_flower <= 0)
		return;

	if (id == PART_FLOWER_1)
	{
		const int id_flower_random = number(POINT_FLOWER_TYPE_2, POINT_FLOWER_TYPE_6);
		PointChange(id_flower_random, FLOWER_COUNT_ITEM_GIVE_EXCHANGE);
		PointChange(POINT_FLOWER_TYPE_1, -FLOWER_COUNT_ITEM_GIVE_EXCHANGE);
		return;
	}

	if (count_flower < FLOWER_COUNT_FLOWER_EXCHANGE_CHEST)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1638;%d]", FLOWER_COUNT_FLOWER_EXCHANGE_CHEST);
		return;
	}

	const uint32_t list_cofres[] =
	{
		FLOWER_VNUM_CHEST_FLOWER_1,
		FLOWER_VNUM_CHEST_FLOWER_2,
		FLOWER_VNUM_CHEST_FLOWER_3,
		FLOWER_VNUM_CHEST_FLOWER_4,
		FLOWER_VNUM_CHEST_FLOWER_5
	};


	int count_chest_exchange = (count_flower / FLOWER_COUNT_FLOWER_EXCHANGE_CHEST);
	if (count_chest_exchange > 200)
		count_chest_exchange = 200;

	LPITEM pItem = ITEM_MANAGER::Instance().CreateItem(list_cofres[id - 1], count_chest_exchange);
	if (!pItem)
		return;
#ifdef ENABLE_SPECIAL_INVENTORY
	const int iEmptyPos = pItem->IsDragonSoul() ? GetEmptyDragonSoulInventory(pItem) : GetEmptyInventory(pItem);
#else
	const int iEmptyPos = pItem->IsDragonSoul() ? GetEmptyDragonSoulInventory(pItem) : GetEmptyInventory(pItem->GetSize());
#endif
	if (iEmptyPos < 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
		return;
	}

	pItem->AddToCharacter(this, TItemPos(pItem->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iEmptyPos));
	PointChange(POINT_FLOWER_TYPE_2 + (id - 1), -(FLOWER_COUNT_FLOWER_EXCHANGE_CHEST * count_chest_exchange));
}

void CHARACTER::SetFlowerEventValue(uint8_t id, uint32_t value)
{
	if (id >= PART_FLOWER_MAX_NUM)
		return;

	m_FlowerEvent[id].value = value;
}

uint32_t CHARACTER::GetFlowerEventValue(uint8_t id) noexcept
{
	if (id >= PART_FLOWER_MAX_NUM)
		return 0;

	return m_FlowerEvent[id].value;
}

//
void CHARACTER::FlowerSystem(LPITEM item)
{
	if (!item)
		return;

	const uint32_t dwType = item->GetValue(0);
	const uint16_t wApplyOn = item->GetValue(1);
	const uint8_t bIncreaseBonus = item->GetValue(2);
	const long lTimeExpire = item->GetValue(3);
	const int iFlowerType = item->GetValue(4);

	if (GetFlowerEventValue(STAGE_FLOWER) <= 0)
		SetFlowerEventValue(STAGE_FLOWER, 1);

	uint32_t dwFlowerStage = GetFlowerEventValue(STAGE_FLOWER);

	const int dwRandom = number(1, 10);

	if (FindAffect(dwType, aApplyInfo[wApplyOn].wPointType))
	{
		// increase the bonus
		if (dwFlowerStage >= FLOWER_POWER_MAX_STAGE)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;319]");
			return;
		}

		item->SetCount(item->GetCount() - 1);
		if (dwRandom > 5)
		{
			dwFlowerStage += 1;
			SetFlowerEventValue(STAGE_FLOWER, dwFlowerStage);
			RefreshFlowerAffect(dwType, wApplyOn, iFlowerType, bIncreaseBonus, dwFlowerStage, lTimeExpire);
			ChatPacket(CHAT_TYPE_INFO, "[LS;320;[IN;%d];%d]", item->GetVnum(), dwFlowerStage);
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;321;[IN;%d];%d]", item->GetVnum(), dwFlowerStage + 1);
		}
	}
	else
	{
		item->SetCount(item->GetCount() - 1);
		if (FindAffect(dwType, 0))
		{
			// change the bonus
			if (dwRandom > 5)
			{
				SetFlowerEventValue(STAGE_FLOWER, 1);
				RefreshFlowerAffect(dwType, wApplyOn, iFlowerType, bIncreaseBonus, 1, lTimeExpire);
				ChatPacket(CHAT_TYPE_INFO, "[LS;322;[IN;%d]]", item->GetVnum());
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;323;[IN;%d]]", item->GetVnum());
			}
			return;
		}

		// apply new bonus
		SetFlowerEventValue(STAGE_FLOWER, 1);
		RefreshFlowerAffect(dwType, wApplyOn, iFlowerType, bIncreaseBonus, 1, lTimeExpire);
		ChatPacket(CHAT_TYPE_INFO, "[LS;322;[IN;%d]]", item->GetVnum());
	}
}

void CHARACTER::RefreshFlowerAffect(uint32_t dwType, uint16_t wApplyOn, int iFlowerType, uint8_t bIncreaseBonus, uint32_t flowerStage, long lTimeExpire)
{
	AddAffect(dwType, aApplyInfo[wApplyOn].wPointType, (bIncreaseBonus * flowerStage), 0, lTimeExpire, 0, true, false, 0, (get_global_time() + static_cast<uint32_t>(lTimeExpire)));
	EffectPacket(SE_FLOWER_EVENT);
}
#endif
