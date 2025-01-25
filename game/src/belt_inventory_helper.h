#ifndef __HEADER_BELT_INVENTORY_HELPER__
#define __HEADER_BELT_INVENTORY_HELPER__

#include "char.h"
#include "item.h"

extern bool belt_allow_all_items; //@custom009

class CBeltInventoryHelper
{
public:
	typedef uint8_t TGradeUnit;

	static TGradeUnit GetBeltGradeByRefineLevel(uint32_t level) noexcept
	{
		static TGradeUnit beltGradeByLevelTable[] =
		{
			0,
			1, // +1
			1, // +2
			2, // +3
			2, // +4,
			3, // +5
			4, // +6,
			5, // +7,
			6, // +8,
			7, // +9
		};

		if (level >= _countof(beltGradeByLevelTable))
		{
			sys_err("CBeltInventoryHelper::GetBeltGradeByRefineLevel - Overflow level (%u)", level);
			return 0;
		}

		return beltGradeByLevelTable[level];
	}

	static const TGradeUnit* GetAvailableRuleTableByGrade() noexcept
	{
		/**
		2(1) 4(2) 6(4) 8(6)
		5(3) 5(3) 6(4) 8(6)
		7(5) 7(5) 7(5) 8(6)
		9(7) 9(7) 9(7) 9(7)
		*/

		static TGradeUnit availableRuleByGrade[BELT_INVENTORY_SLOT_COUNT] =
		{
			1, 2, 4, 6, 3, 3, 4, 6, 5, 5, 5, 6, 7, 7, 7, 7
#ifdef ENABLE_BELT_INVENTORY_RENEWAL
			, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11
#endif
		};

		return availableRuleByGrade;
	}

	static bool IsAvailableCell(uint16_t cell, int beltGrade /*int beltLevel*/)
	{
		//const TGradeUnit beltGrade = GetBeltGradeByRefineLevel(beltLevel);
		const TGradeUnit* ruleTable = GetAvailableRuleTableByGrade();

		return ruleTable[cell] <= beltGrade;
	}

	static bool IsExistItemInBeltInventory(const LPCHARACTER pc)
	{
		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			const LPITEM& beltInventoryItem = pc->GetBeltInventoryItem(i);

			if (nullptr != beltInventoryItem)
				return true;
		}

		return false;
	}

	static int GetItemCount(const LPCHARACTER pc)
	{
		uint32_t count = 0;
		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetBeltInventoryItem(i);
			if (beltInventoryItem)
				count++;
		}
		return count;
	}

	static bool ClearBelt(LPCHARACTER pc)
	{
		const int EmptySlotCount = pc->GetEmptyInventoryCount(1);
		const int BeltItemCount = GetItemCount(pc);

		if (EmptySlotCount - 1 < BeltItemCount)
		{
			pc->ChatPacket(CHAT_TYPE_INFO, "[LS;1934]");
			return false;
		}

		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetInventoryItem(i);

			if (nullptr != beltInventoryItem)
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				const int NewPos = pc->GetEmptyInventory(beltInventoryItem);
#else
				const int NewPos = pc->GetEmptyInventory(1);
#endif
				if (NewPos != -1)
				{
					beltInventoryItem->RemoveFromCharacter();
					beltInventoryItem->AddToCharacter(pc, TItemPos(INVENTORY, NewPos));
				}
				else
				{
					pc->ChatPacket(CHAT_TYPE_INFO, "[LS;1934]");
					return false;
				}
			}
		}

		return true;
	}

	static bool CanMoveIntoBeltInventory(const LPITEM item)
	{
#ifdef ENABLE_BELT_INVENTORY_RENEWAL
		return true;
#else
		bool canMove = false;

		if (belt_allow_all_items) //@custom009
			return true;

		if (item->GetType() == ITEM_BLEND) //@fixme427
			return true;

		if (item->GetType() == ITEM_USE)
		{
			switch (item->GetSubType())
			{
				case USE_POTION:
				case USE_POTION_NODELAY:
				case USE_ABILITY_UP:
					canMove = true;
					break;

				default:
					break;
			}
		}

		return canMove;
#endif
	}
};

#endif //__HEADER_BELT_INVENTORY_HELPER__
