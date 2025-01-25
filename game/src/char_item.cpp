#include "stdafx.h"
#include <stack>
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "desc.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "skill.h"
#include "start_position.h"
#include "mob_manager.h"
#include "db.h"
#include "log.h"
#include "vector.h"
#include "buffer_manager.h"
#include "questmanager.h"
#include "fishing.h"
#include "party.h"
#include "dungeon.h"
#include "refine.h"
#include "unique_item.h"
#include "war_map.h"
#include "xmas_event.h"
#include "marriage.h"
#include "monarch.h"
#include "polymorph.h"
#include "blend_item.h"
#include "castle.h"
#include "BattleArena.h"
#include "arena.h"
#include "dev_log.h"
#include "threeway_war.h"
#include "safebox.h"
#include "shop.h"
#ifdef ENABLE_NEWSTUFF
#	include "pvp.h"
#endif
#include "PetSystem.h"
#include "../../common/VnumHelper.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "../../common/CommonDefines.h"
#ifdef ENABLE_SWITCHBOT
#	include "switchbot.h"
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
#	include "MeleyLair.h"
#endif
#include "p2p.h"
#include "gm.h"
#include "OXEvent.h"
#ifdef ENABLE_BATTLE_FIELD
#	include "battle_field.h"
#endif
#ifdef ENABLE_YOHARA_SYSTEM
#	include "RandomHelper.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "GrowthPetSystem.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "AchievementSystem.h"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif

#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
extern bool IS_MOUNTABLE_ZONE(int map_index);	//@custom005
#endif
extern bool IS_BLOCKED_PET_SUMMON_MAP(int map_index);	//@custom006
constexpr int ITEM_BROKEN_METIN_VNUM = 28960;

struct FPartyCHCollector3 {
	std::vector <LPCHARACTER> vecCHPtr; FPartyCHCollector3() { }

	void operator () (LPCHARACTER ch) {
		vecCHPtr.emplace_back(ch);
	}
};

extern bool attr_always_add;	//@custom029
extern bool attr_always_5_add;	//@custom030
extern bool attr_rare_enable;	//@custom026
extern bool belt_allow_all_items;

// CHANGE_ITEM_ATTRIBUTES
// const uint32_t CHARACTER::msc_dwDefaultChangeItemAttrCycle = 10;
const char CHARACTER::msc_szLastChangeItemAttrFlag[] = "Item.LastChangeItemAttr";
// const char CHARACTER::msc_szChangeItemAttrCycleFlag[] = "change_itemattr_cycle";
// END_OF_CHANGE_ITEM_ATTRIBUTES
const uint16_t g_aBuffOnAttrPoints[] = { POINT_ENERGY, POINT_COSTUME_ATTR_BONUS };	//@fixme532

//@fixme404
#define VERIFY_POTION(affect, afftype)  \
	if (FindAffect(affect, afftype)) { \
			ChatPacket(CHAT_TYPE_INFO, "[LS;99]"); \
			return false; \
			} //@END_fixme404

static bool FN_compare_item_socket(const LPITEM pkItemSrc, const LPITEM pkItemDest)	//@fixme535
{
	if (!pkItemSrc || !pkItemDest)
		return false;

	return memcmp(pkItemSrc->GetSockets(), pkItemDest->GetSockets(), sizeof(long) * ITEM_SOCKET_MAX_NUM) == 0;
}

struct FFindStone
{
	std::map<uint32_t, LPCHARACTER> m_mapStone;

	void operator()(LPENTITY pEnt)
	{
		if (pEnt && pEnt->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(pEnt);

			if (pChar && pChar->IsStone())
			{
				m_mapStone[(uint32_t)pChar->GetVID()] = pChar;
			}
		}
	}
};

// Find Mob Vnum
struct FFindMobVnum
{
	uint32_t dwMobVnum;
	FFindMobVnum(uint32_t dwMobVnum) : dwMobVnum(dwMobVnum)
	{}

	std::map<uint32_t, LPCHARACTER> m_mapVID;
	void operator()(LPENTITY pEnt)
	{
		if (pEnt && pEnt->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(pEnt);

			if (pChar && pChar->GetRaceNum() == dwMobVnum)
			{
				m_mapVID[(uint32_t)pChar->GetVID()] = pChar;
			}
		}
	}
};

// Nyx: Changed like GameForge
static bool IS_SUMMON_ITEM(LPITEM item) noexcept
{
	if (!item)
		return false;

	// Vnum
	switch (item->GetVnum())
	{
		case ITEM_MARRIAGE_RING:
			return true;
		default:
			break;
	}

	// Type
	switch (item->GetType())
	{
		case ITEM_QUEST:
		{
			if (item->GetSubType() == QUEST_WARP)
				return true;
		}
		break;
		case ITEM_USE:
		{
			if (item->GetSubType() == USE_TALISMAN)
				return true;
		}
		break;
		default:
			break;
	}

	return false;
}

static bool IS_MONKEY_DUNGEON(int map_index) noexcept
{
	switch (map_index)
	{
		case MAP_MONKEY_DUNGEON_11:
		case MAP_MONKEY_DUNGEON_12:
		case MAP_MONKEY_DUNGEON_13:
		case MAP_MONKEY_DUNGEON:	//@fixme422
		case MAP_MONKEY_DUNGEON2:
		case MAP_MONKEY_DUNGEON3:
			return true;

		default:
			return false;
	}

	return false;
}

// MarriageRing & others usage
bool IS_SUMMONABLE_ZONE(int map_index) noexcept
{
	// Monkey Dungeon
	if (IS_MONKEY_DUNGEON(map_index))
		return false;

	// Castle
	if (IS_CASTLE_MAP(map_index))
		return false;

	switch (map_index)
	{
		case MAP_DEVILTOWER1:
		case MAP_SPIDERDUNGEON_02:
		case MAP_SKIPIA_DUNGEON_01:
		case MAP_SKIPIA_DUNGEON_02:
		case MAP_DEVILCATACOMB:
		case MAP_SPIDERDUNGEON_03:
		case MAP_SKIPIA_DUNGEON_BOSS:
		case MAP_OXEVENT:
		case MAP_12ZI_STAGE:
		case MAP_BATTLEFIELD:
			return false;

		default:
			break;
	}

	if (CBattleArena::IsBattleArenaMap(map_index))
		return false;

	// All private maps are not warpable
	if (map_index > 10000)
		return false;

	return true;
}

bool IS_BOTARYABLE_ZONE(int nMapIndex) noexcept
{
	if (!g_bEnableBootaryCheck)
		return true;

	switch (nMapIndex)
	{
		case MAP_A1:
		case MAP_A3:
		case MAP_B1:
		case MAP_B3:
		case MAP_C1:
		case MAP_C3:
			return true;

		default:
			break;
	}

	return false;
}

// Check if item socket is same as prototype - by mhh
static bool FN_check_item_socket(LPITEM item)
{
	if (!item)
		return false;

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (item->GetSocket(i) != item->GetProto()->alSockets[i])
			return false;
	}

	return true;
}

#ifdef ENABLE_SUNG_MAHI_TOWER
bool IS_SUNG_MAHI_ENABLE_ITEM(const LPCHARACTER& ch, int iMapIndex, uint32_t dwVnum)
{
	if (!ch)
		return false;

	if (ch->IsSungMahiDungeon(iMapIndex) && !CItemVnumHelper::IsSungMahiItem(dwVnum))
		return false;

	if (!ch->IsSungMahiDungeon(iMapIndex) && CItemVnumHelper::IsSungMahiItem(dwVnum))
		return false;

	return true;
}
#endif

// item socket copy - by mhh
static void FN_copy_item_socket(LPITEM dest, LPITEM src)
{
	if (!dest || !src)
		return;

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		dest->SetSocket(i, src->GetSocket(i));
	}
}

static bool FN_check_item_sex(const LPCHARACTER& ch, LPITEM item)
{
	if (!ch || !item)
		return false;

	// No man
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MALE))
	{
		if (ESex::SEX_MALE == GET_SEX(ch))
			return false;
	}
	// No women
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))
	{
		if (ESex::SEX_FEMALE == GET_SEX(ch))
			return false;
	}

	return true;
}

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
bool IsPlus15(uint32_t vnum)
{
	switch (vnum)
	{
		case 0: // add vnums here.
			return true;
		default:
			break;
	}

	return false;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// ITEM HANDLING
/////////////////////////////////////////////////////////////////////////////
bool CHARACTER::CanHandleItem(bool bSkipCheckRefine, bool bSkipObserver) noexcept
{
	if (!bSkipObserver && m_bIsObserver)
		return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (IsShop())
		return false;
#else
	if (GetMyShop())
		return false;
#endif

#ifdef ENABLE_REFINE_ELEMENT
	if (IsOpenElementsSpell())
		return false;
#endif

	if (!bSkipCheckRefine && m_bUnderRefine)
		return false;

	if (IsCubeOpen() || nullptr != DragonSoul_RefineWindow_GetOpener())
		return false;

	if (IsWarping())
		return false;

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if ((m_bAcceCombination) || (m_bAcceAbsorption))
		return false;
#endif

#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen() || nullptr != GetAuraRefineWindowOpener())
		return false;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	if (IsChangeLookWindowOpen())
		return false;
#endif

#ifdef ENABLE_MAILBOX
	if (GetMailBox())
		return false;
#endif

#ifdef ENABLE_CHANGED_ATTR
	if (IsSelectAttr())
		return false;
#endif

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
	if (IsGrowthPetDetermineWindow())
		return false;
#endif

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_PRIMIUM_FEEDSTUFF)
	if (IsGrowthPetPrimiumFeedWindow())
		return false;
#endif

	return true;
}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE 
LPITEM CHARACTER::GetAdditionalEquipmentItem(uint16_t wCell) const
{
	return GetItem(TItemPos(ADDITIONAL_EQUIPMENT_1, wCell));
}
#endif

LPITEM CHARACTER::GetInventoryItem(uint16_t wCell) const
{
	return GetItem(TItemPos(INVENTORY, wCell));
}

LPITEM CHARACTER::GetEquipmentItem(uint16_t wCell) const
{
	return GetItem(TItemPos(EQUIPMENT, wCell));
}

LPITEM CHARACTER::GetDSInventoryItem(uint16_t wCell) const
{
	return GetItem(TItemPos(DRAGON_SOUL_INVENTORY, wCell));
}

LPITEM CHARACTER::GetBeltInventoryItem(uint16_t wCell) const
{
	return GetItem(TItemPos(BELT_INVENTORY, wCell));
}

#ifdef ENABLE_ATTR_6TH_7TH
LPITEM CHARACTER::GetAttr67InventoryItem() const noexcept
{
	return m_pointsInstant.pNPCStorageItems;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
LPITEM CHARACTER::GetInventoryFeedItem(uint16_t wCell) const
{
	return GetItem(TItemPos(PET_FEED, wCell));
}
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
uint8_t CHARACTER::GetExtendSpecialCellWindowType(TItemPos Cell)
{
	uint8_t bWindow = -1;
	if (Cell.GetSpecialInventoryType() == SPECIAL_INVENTORY_TYPE_SKILLBOOK)
		bWindow = 0;
	else if (Cell.GetSpecialInventoryType() == SPECIAL_INVENTORY_TYPE_STONE)
		bWindow = 1;
	else if (Cell.GetSpecialInventoryType() == SPECIAL_INVENTORY_TYPE_MATERIAL)
		bWindow = 2;

	if (bWindow == -1 || bWindow >= 3)
		return -1;

	return bWindow;
}

uint8_t CHARACTER::GetExtendSpecialItemWindowType(LPITEM item)
{
	uint8_t bWindow = -1;
	if (item->GetSpecialInventoryType() == SPECIAL_INVENTORY_TYPE_SKILLBOOK)
		bWindow = 0;
	else if (item->GetSpecialInventoryType() == SPECIAL_INVENTORY_TYPE_STONE)
		bWindow = 1;
	else if (item->GetSpecialInventoryType() == SPECIAL_INVENTORY_TYPE_MATERIAL)
		bWindow = 2;

	if (bWindow == -1 || bWindow >= 3)
		return -1;

	return bWindow;
}
#endif

LPITEM CHARACTER::GetItem(TItemPos Cell) const
{
	/*if (!m_PlayerSlots) //@fixme199
		return nullptr;*/

	if (!IsValidItemPosition(Cell))
		return nullptr;

	const uint16_t wCell = Cell.cell;
	const uint8_t window_type = Cell.window_type;

	switch (window_type)
	{
		case INVENTORY:
			if (wCell >= INVENTORY_SLOT_COUNT)
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid inventory item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pInventoryItems[wCell];

		case EQUIPMENT:
			if (wCell >= EQUIPMENT_SLOT_COUNT) // need review related about ds, because when you equip a ds item is in EQUIPMENT too
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid equipment item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pEquipmentItems[wCell];

		case BELT_INVENTORY:
			if (wCell >= BELT_INVENTORY_SLOT_COUNT)
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid belt item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pBeltItems[wCell];

		case DRAGON_SOUL_INVENTORY:
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid DS item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pDSItems[wCell];

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case PREMIUM_PRIVATE_SHOP:
			if (wCell >= SHOP_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid shop item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pShopItems[wCell];
#endif

#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			if (wCell >= SWITCHBOT_SLOT_COUNT)
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid SWITCHBOT item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pSwitchbotItems[wCell];
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case ADDITIONAL_EQUIPMENT_1:
			if (wCell >= ADDITIONAL_EQUIPMENT_MAX_SLOT)
			{
				sys_err("CHARACTER::GetItem: window_type: %d -> invalid ADDITIONAL_EQUIPMENT_1 item cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pAdditionalEquipmentItem_1[wCell];
#endif

		default:
			return nullptr;
	}

	return nullptr;
}

LPITEM CHARACTER::GetItem_NEW(const TItemPos &Cell) const
{
	LPITEM cellItem = GetItem(Cell);
	if (cellItem)
		return cellItem;

	//There's no item in this cell, but that does not mean there is not an item which currently uses up this cell.
	const uint16_t bCell = Cell.cell;
	const uint8_t bPage = bCell / (INVENTORY_PAGE_SIZE);

	for (int j = -2; j < 0; ++j)
	{
		uint8_t p = bCell + (INVENTORY_PAGE_COUNT * j);

		if (p / (INVENTORY_PAGE_SIZE) != bPage)
			continue;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
		if (p >= GetExtendInvenMax())
#else
		if (p >= INVENTORY_MAX_NUM)
#endif
			continue;

		LPITEM curCellItem = GetItem(TItemPos(INVENTORY, p));
		if (!curCellItem)
			continue;

		if (p + (curCellItem->GetSize() - 1) * INVENTORY_PAGE_COUNT < Cell.cell) //Doesn't reach Cell.cell
			continue;

		return curCellItem;
	}

	return nullptr;
}

void CHARACTER::SetItem(TItemPos Cell, LPITEM pItem, bool isHighLight)
{
	/*if (!m_PlayerSlots) //@fixme199
		return;*/

	const uint16_t wCell = Cell.cell;
	const uint8_t window_type = Cell.window_type;

	if ((unsigned long)((CItem*)pItem) == 0xff || (unsigned long)((CItem*)pItem) == 0xffffffff)
	{
		sys_err("!!! FATAL ERROR !!! item == 0xff (char: %s cell: %u)", GetName(), wCell);
		core_dump();
		return;
	}

	if (pItem && pItem->GetOwner())
	{
		assert(!"GetOwner exist");
		return;
	}

	switch (window_type)
	{
		case INVENTORY:
		{
			if (wCell >= INVENTORY_SLOT_COUNT)
			{
				sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
				return;
			}

			LPITEM pOld = m_pointsInstant.pInventoryItems[wCell];

			if (pOld)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						uint16_t p = wCell + (i * 5);

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
						if (p >= GetExtendInvenMax())
#else
						if (p >= INVENTORY_MAX_NUM)
#endif
						{
							continue;
						}

						if (m_pointsInstant.pInventoryItems[p] && m_pointsInstant.pInventoryItems[p] != pOld)
							continue;

						m_pointsInstant.wInventoryItemGrid[p] = 0;
					}
				}
#ifdef ENABLE_SPECIAL_INVENTORY
				else if (Cell.IsSpecialInventoryPosition())
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						uint16_t p = wCell + (i * 5);

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
						uint8_t bPage = GetExtendSpecialCellWindowType(Cell);
						if (p >= GetExtendSpecialInvenMax(bPage))
# else
#	ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
						if (p >= GetExtendInvenMax())
#	else
						if (p >= INVENTORY_MAX_NUM)
#	endif
# endif
						{
							if (!TItemPos(INVENTORY, p).IsSpecialInventoryPosition())
							{
								continue;
							}
						}

						if (m_pointsInstant.pInventoryItems[p] && m_pointsInstant.pInventoryItems[p] != pOld)
							continue;

						m_pointsInstant.wInventoryItemGrid[p] = 0;
					}
				}
#endif
				else
					m_pointsInstant.wInventoryItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						uint16_t p = wCell + (i * 5);

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
						if (p >= GetExtendInvenMax())
#else
						if (p >= INVENTORY_MAX_NUM)
#endif
						{
							continue;
						}

						// wCell + 1 is the same when checking empty
						// Items are for exception handling
						m_pointsInstant.wInventoryItemGrid[p] = wCell + 1;
					}
				}
#ifdef ENABLE_SPECIAL_INVENTORY
				else if (Cell.IsSpecialInventoryPosition())
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						uint16_t p = wCell + (i * 5);

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
						uint8_t bPage = GetExtendSpecialCellWindowType(Cell);
						if (p >= GetExtendSpecialInvenMax(bPage))
# else
#	ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
						if (p >= GetExtendInvenMax())
#	else
						if (p >= INVENTORY_MAX_NUM)
#	endif
# endif
						{
							if (!TItemPos(INVENTORY, p).IsSpecialInventoryPosition())
							{
								continue;
							}
						}

						// wCell + 1 is the same when checking empty
						// Items are for exception handling
						m_pointsInstant.wInventoryItemGrid[p] = wCell + 1;
					}
				}
#endif
				else
					m_pointsInstant.wInventoryItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pInventoryItems[wCell] = pItem;
		}
		break;

		case EQUIPMENT:
		{
			if (wCell >= EQUIPMENT_SLOT_COUNT)
			{
				sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
				return;
			}

			LPITEM pOld = m_pointsInstant.pEquipmentItems[wCell];

			if (pOld)
			{
				if (wCell < EQUIPMENT_SLOT_COUNT)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						int p = wCell + i;

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pEquipmentItems[p] && m_pointsInstant.pEquipmentItems[p] != pOld)
							continue;

						m_pointsInstant.bEquipmentItemGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.bEquipmentItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < EQUIPMENT_SLOT_COUNT)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						int p = wCell + i;

						if (p >= EQUIPMENT_SLOT_COUNT)
							continue;

						m_pointsInstant.bEquipmentItemGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.bEquipmentItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pEquipmentItems[wCell] = pItem;
		}
		break;

		case BELT_INVENTORY:
		{
			LPITEM pOld = m_pointsInstant.pBeltItems[wCell];

			if (pOld)
			{
				if (wCell < BELT_INVENTORY_SLOT_COUNT)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						int p = wCell + (i * BELT_INVENTORY_SLOT_WIDTH);

						if (p >= BELT_INVENTORY_SLOT_COUNT)
							continue;

						if (m_pointsInstant.pBeltItems[p] && m_pointsInstant.pBeltItems[p] != pOld)
							continue;

						m_pointsInstant.bBeltGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.bBeltGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell >= BELT_INVENTORY_SLOT_COUNT)
				{
					sys_err("CHARACTER::SetItem: invalid belt item cell %d", wCell);
					return;
				}

				if (wCell < BELT_INVENTORY_SLOT_COUNT)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						int p = wCell + (i * BELT_INVENTORY_SLOT_WIDTH);

						if (p >= BELT_INVENTORY_SLOT_COUNT)
							continue;

						// wCell + 1 is the same when checking empty
						// Items are for exception handling
						m_pointsInstant.bBeltGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.bBeltGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pBeltItems[wCell] = pItem;
		}
		break;

		case DRAGON_SOUL_INVENTORY:
		{
			LPITEM pOld = m_pointsInstant.pDSItems[wCell];

			if (pOld)
			{
				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pOld->GetSize(); ++i)
					{
						int p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pDSItems[p] && m_pointsInstant.pDSItems[p] != pOld)
							continue;

						m_pointsInstant.wDSItemGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.wDSItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					sys_err("CHARACTER::SetItem: invalid DS item cell %d", wCell);
					return;
				}

				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (int i = 0; i < pItem->GetSize(); ++i)
					{
						int p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						// wCell + 1 is the same when checking empty
						// Items are for exception handling
						m_pointsInstant.wDSItemGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wDSItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pDSItems[wCell] = pItem;
		}
		break;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case PREMIUM_PRIVATE_SHOP:
		{
			if (wCell >= SHOP_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: invalid shop item cell %d", wCell);
				return;
			}

			LPITEM pOld = m_pointsInstant.pShopItems[wCell];

			if (pOld)
			{
				for (int i = 0; i < pOld->GetSize(); ++i)
				{
					int p = wCell + (i * 5);

					if (p >= SHOP_INVENTORY_MAX_NUM)
						continue;

					if (m_pointsInstant.pShopItems[p] && m_pointsInstant.pShopItems[p] != pOld)
						continue;

					m_pointsInstant.bShopItemGrid[p] = 0;
				}
			}

			if (pItem)
			{
				for (int i = 0; i < pItem->GetSize(); ++i)
				{
					int p = wCell + (i * 5);

					if (p >= SHOP_INVENTORY_MAX_NUM)
						continue;

					// wCell + 1 is the same when checking empty
					// Items are for exception handling
					m_pointsInstant.bShopItemGrid[p] = wCell + 1;
				}
			}

			m_pointsInstant.pShopItems[wCell] = pItem;
		}
		break;
#endif

#ifdef ENABLE_SWITCHBOT
	case SWITCHBOT:
	{
		if (wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CHARACTER::SetItem: invalid SWITCHBOT item cell %d", wCell);
			return;
		}

		const LPITEM& pOld = m_pointsInstant.pSwitchbotItems[wCell];
		if (pOld && pItem)
			return;

		if (pItem)
			CSwitchbotManager::Instance().RegisterItem(GetPlayerID(), pItem->GetID(), wCell);
		else
			CSwitchbotManager::Instance().UnregisterItem(GetPlayerID(), wCell);

		m_pointsInstant.pSwitchbotItems[wCell] = pItem;
	}
	break;
#endif

#ifdef ENABLE_ATTR_6TH_7TH
		case NPC_STORAGE:
		{
			if (wCell >= NPC_STORAGE_SLOT_COUNT)
			{
				sys_err("CHARACTER::SetItem: invalid NPC_STORAGE item cell %d", wCell);
				return;
			}

			m_pointsInstant.pNPCStorageItems = pItem;
		}
		break;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case ADDITIONAL_EQUIPMENT_1:
		{
			if (wCell >= ADDITIONAL_EQUIPMENT_MAX_SLOT)
			{
				sys_err("CHARACTER::SetItem: invalid ADDITIONAL_EQUIPMENT_1 item cell %d", wCell);
				return;
			}
			const LPITEM& pOld = m_pointsInstant.pAdditionalEquipmentItem_1[wCell];
			if (pOld && pItem)
				return;

			m_pointsInstant.pAdditionalEquipmentItem_1[wCell] = pItem;
		}
		break;
#endif

		default:
			sys_err("Invalid Inventory type %d", window_type);
			return;
	}

	if (GetDesc())
	{
		// Extended item: Send item flag information from server
		if (pItem)
		{
			TPacketGCItemSet pack;
			pack.header = HEADER_GC_ITEM_SET;
			pack.Cell = Cell;
			pack.count = static_cast<uint8_t>(pItem->GetCount());
			pack.vnum = pItem->GetVnum();
			pack.flags = pItem->GetFlag();
			pack.anti_flags = pItem->GetAntiFlag();
			pack.highlight = !isHighLight || (Cell.window_type == DRAGON_SOUL_INVENTORY);
#ifdef ENABLE_SEALBIND_SYSTEM
			pack.nSealDate = pItem->GetSealDate();
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			pack.dwTransmutationVnum = pItem->GetChangeLookVnum();
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			pack.is_basic = pItem->IsBasicItem();
#endif
#ifdef ENABLE_REFINE_ELEMENT
			pack.grade_element = pItem->GetElementGrade();
			thecore_memcpy(pack.attack_element, pItem->GetElementAttacks(), sizeof(pack.attack_element));
			pack.element_type_bonus = pItem->GetElementsType();
			thecore_memcpy(pack.elements_value_bonus, pItem->GetElementsValues(), sizeof(pack.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
			pack.set_value = pItem->GetItemSetValue();
#endif
			thecore_memcpy(pack.alSockets, pItem->GetSockets(), sizeof(pack.alSockets));
			thecore_memcpy(pack.aAttr, pItem->GetAttributes(), sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			thecore_memcpy(pack.aApplyRandom, pItem->GetApplysRandom(), sizeof(pack.aApplyRandom));
			thecore_memcpy(pack.alRandomValues, pItem->GetRandomDefaultAttrs(), sizeof(pack.alRandomValues));
#endif
			GetDesc()->Packet(&pack, sizeof(TPacketGCItemSet));
		}
		else
		{
			TPacketGCItemDelDeprecated pack;
			pack.header = HEADER_GC_ITEM_DEL;
			pack.Cell = Cell;
			pack.vnum = 0;
			pack.count = 0;
#ifdef ENABLE_SEALBIND_SYSTEM
			pack.nSealDate = 0;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			pack.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
			pack.is_basic = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT
			pack.grade_element = 0;
			memset(pack.attack_element, 0, sizeof(pack.attack_element));
			pack.element_type_bonus = 0;
			memset(pack.elements_value_bonus, 0, sizeof(pack.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
			pack.set_value = 0;
#endif
			memset(pack.alSockets, 0, sizeof(pack.alSockets));
			memset(pack.aAttr, 0, sizeof(pack.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
			memset(pack.aApplyRandom, 0, sizeof(pack.aApplyRandom));
			memset(pack.alRandomValues, 0, sizeof(pack.alRandomValues));
#endif
			GetDesc()->Packet(&pack, sizeof(TPacketGCItemDelDeprecated));
		}
	}

	if (pItem)
	{
		pItem->SetCell(this, wCell);
		switch (window_type)
		{
			case INVENTORY:
				pItem->SetWindow(INVENTORY);
				break;
			case EQUIPMENT:
				pItem->SetWindow(EQUIPMENT);
				break;
			case BELT_INVENTORY:
				pItem->SetWindow(BELT_INVENTORY);
				break;
			case DRAGON_SOUL_INVENTORY:
				pItem->SetWindow(DRAGON_SOUL_INVENTORY);
				break;
#ifdef ENABLE_ATTR_6TH_7TH
			case NPC_STORAGE:
				pItem->SetWindow(NPC_STORAGE);
				break;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			case PREMIUM_PRIVATE_SHOP:
				pItem->SetWindow(PREMIUM_PRIVATE_SHOP);
				break;
#endif
#ifdef ENABLE_SWITCHBOT
			case SWITCHBOT:
				pItem->SetWindow(SWITCHBOT);
				break;
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			case ADDITIONAL_EQUIPMENT_1:
				pItem->SetWindow(ADDITIONAL_EQUIPMENT_1);
				break;
#endif
			default:
				sys_err("Trying to set window %d, non determined behaviour!", window_type);
				break;
		}
	}
}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
LPITEM CHARACTER::GetWearDefault(uint8_t bCell) const
{
	/*if (!m_PlayerSlots) //@fixme199
		return nullptr;*/

	// > WEAR_MAX_NUM: Dragon Soul Slots.
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::GetWearDefault: invalid wear cell %d", bCell);
		return nullptr;
	}

	return m_pointsInstant.pEquipmentItems[bCell];
}

void CHARACTER::SetWearDefault(uint8_t bCell, LPITEM item)
{
	// > WEAR_MAX_NUM: Dragon Soul Slots.
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::SetWearDefault: invalid item cell %d", bCell);
		return;
	}

	SetItem(TItemPos(EQUIPMENT, bCell), item);
}
#endif

LPITEM CHARACTER::GetWear(uint16_t bCell) const noexcept	//@fixme479 uint8_t -> uint16_t
{
	/*if (!m_PlayerSlots) //@fixme199
		return nullptr;*/

	// > WEAR_MAX_NUM: Dragon Soul Slots.
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::GetWear: invalid wear cell %d", bCell);
		return nullptr;
	}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	if (!CheckAdditionalEquipment(bCell))
		return m_pointsInstant.pEquipmentItems[bCell];
	else
		return m_pointsInstant.pAdditionalEquipmentItem_1[bCell];
#endif

	return m_pointsInstant.pEquipmentItems[bCell];
}

void CHARACTER::SetWear(uint16_t bCell, LPITEM item)	//@fixme479 uint8_t -> uint16_t
{
	// > WEAR_MAX_NUM: Dragon Soul Slots.
	if (bCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::SetItem: invalid item cell %d", bCell);
		return;
	}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	if (!CheckAdditionalEquipment(bCell))
		SetItem(TItemPos(EQUIPMENT, bCell), item);
	else
		SetItem(TItemPos(ADDITIONAL_EQUIPMENT_1, bCell), item);
#else
	SetItem(TItemPos(EQUIPMENT, bCell), item);
#endif

	if (!item && bCell == WEAR_WEAPON)
	{
		// If you take it off when using an ear sword, you should get rid of the effect.
		if (IsAffectFlag(AFF_GWIGUM))
			RemoveAffect(SKILL_GWIGEOM);

		if (IsAffectFlag(AFF_GEOMGYEONG))
			RemoveAffect(SKILL_GEOMKYUNG);
	}
}

void CHARACTER::ClearItem()
{
	int i;
	LPITEM item;

	for (i = 0; i < INVENTORY_SLOT_COUNT; ++i)
	{
		if ((item = GetInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, QUICKSLOT_MAX_POS);
		}
	}

	for (i = 0; i < EQUIPMENT_SLOT_COUNT; ++i)
	{
		if ((item = GetEquipmentItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}

	for (i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
	{
		if ((item = GetBeltInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(QUICKSLOT_TYPE_BELT, i, QUICKSLOT_MAX_POS);
		}
	}

	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}

#ifdef ENABLE_ATTR_6TH_7TH
	item = GetAttr67InventoryItem();
	if (item)
	{
		item->SetSkipSave(true);
		ITEM_MANAGER::Instance().FlushDelayedSave(item);

		item->RemoveFromCharacter();
		M2_DESTROY_ITEM(item);
	}
#endif

#ifdef ENABLE_SWITCHBOT
	for (i = 0; i < SWITCHBOT_SLOT_COUNT; ++i)
	{
		if ((item = GetItem(TItemPos(SWITCHBOT, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	for (i = 0; i < ADDITIONAL_EQUIPMENT_MAX_SLOT; ++i)
	{
		if ((item = GetItem(TItemPos(ADDITIONAL_EQUIPMENT_1, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#endif

}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CHARACTER::ClearShopItem()
{
	LPITEM item;

	for (int i = 0; i < SHOP_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(PREMIUM_PRIVATE_SHOP, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
}
#endif

bool CHARACTER::IsEmptyItemGrid(TItemPos Cell, uint8_t bSize, int iExceptionCell) const
{
	switch (Cell.window_type)
	{
		case INVENTORY:
		{
			const uint16_t wCell = Cell.cell;

			// bItemCell handles by +1 to indicate that 0 is false.
			// Therefore, compare by adding 1 to iExceptionCell.
			++iExceptionCell;

			if (Cell.IsBeltInventoryPosition())
			{
				LPITEM beltItem = GetWear(WEAR_BELT);

				if (nullptr == beltItem)
					return false;

				if (false == CBeltInventoryHelper::IsAvailableCell(wCell - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
					return false;

				if (m_pointsInstant.wInventoryItemGrid[wCell])
				{
					if (m_pointsInstant.wInventoryItemGrid[wCell] == iExceptionCell)
						return true;

					return false;
				}

				if (bSize == 1)
					return true;
			}

#ifdef ENABLE_SPECIAL_INVENTORY
			else if (Cell.IsSpecialInventoryPosition())
			{
				return IsEmptySpecialItemGrid(Cell, bSize, iExceptionCell);
			}
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
			else if (wCell >= GetExtendInvenMax())
				return false;
#else
			else if (wCell >= INVENTORY_MAX_NUM)
				return false;
#endif

			if (m_pointsInstant.wInventoryItemGrid[wCell])
			{
				if (m_pointsInstant.wInventoryItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int j = 1;
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
					const uint16_t wPage = wCell / (INVENTORY_PAGE_SIZE);
#else
					const uint16_t wPage = wCell / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT);
#endif

					do
					{
						const uint16_t p = wCell + (5 * j);

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
						if (p >= GetExtendInvenMax())
							return false;

						if (p / (INVENTORY_PAGE_SIZE) != wPage)
							return false;
#else
						if (p >= INVENTORY_MAX_NUM)
							return false;

						if (p / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT) != wPage)
							return false;
#endif

						if (m_pointsInstant.wInventoryItemGrid[p])
						{
							if (m_pointsInstant.wInventoryItemGrid[p] != iExceptionCell)
								return false;
						}
					} while (++j < bSize);

					return true;
				}
				else
					return false;
			}

			// If the size is 1, it occupies one space, so just return
			if (1 == bSize)
				return true;
			else
			{
				int j = 1;
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
				const uint16_t wPage = wCell / (INVENTORY_PAGE_SIZE);
#else
				const uint16_t wPage = wCell / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT);
#endif

				do
				{
					const uint16_t p = wCell + (5 * j);

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
					if (p >= GetExtendInvenMax())
						return false;

					if (p / (INVENTORY_PAGE_SIZE) != wPage)
						return false;
#else
					if (p >= INVENTORY_MAX_NUM)
						return false;

					if (p / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT) != wPage)
						return false;
#endif

					if (m_pointsInstant.wInventoryItemGrid[p])
					{
						if (m_pointsInstant.wInventoryItemGrid[p] != iExceptionCell)
							return false;
					}
				} while (++j < bSize);

				return true;
			}
		}
		break;

		case DRAGON_SOUL_INVENTORY:
		{
			const uint16_t wCell = Cell.cell;
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				return false;

			// bItemCell is processed by +1 to indicate that 0 is false.
			// Therefore, 1 is added to the iExceptionCell for comparison.
			iExceptionCell++;

			if (m_pointsInstant.wDSItemGrid[wCell])
			{
				if (m_pointsInstant.wDSItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int j = 1;

					do
					{
						const int p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							return false;

						if (m_pointsInstant.wDSItemGrid[p])
						{
							if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
								return false;
						}
					} while (++j < bSize);

					return true;
				}
				else
					return false;
			}

			// If the size is 1, it occupies one space, so just return
			if (1 == bSize)
				return true;
			else
			{
				int j = 1;

				do
				{
					const int p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);

					if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						return false;

					if (m_pointsInstant.wDSItemGrid[p])
					{
						if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
							return false;
					}
				} while (++j < bSize);

				return true;
			}
		}
		break;

		case BELT_INVENTORY:
		{
			const uint16_t wCell = Cell.cell;
			if (wCell >= BELT_INVENTORY_SLOT_COUNT)
				return false;

			// bItemCell is processed by +1 to indicate that 0 is false.
			// Therefore, 1 is added to iExceptionCell for comparison.
			iExceptionCell++;

			if (m_pointsInstant.bBeltGrid[wCell])
			{
				if (m_pointsInstant.bBeltGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int j = 1;

					do
					{
						const int p = wCell + (BELT_INVENTORY_SLOT_WIDTH * j);

						if (p >= BELT_INVENTORY_SLOT_COUNT)
							return false;

						if (m_pointsInstant.bBeltGrid[p])
						{
							if (m_pointsInstant.bBeltGrid[p] != iExceptionCell)
								return false;
						}
					} while (++j < bSize);

					return true;
				}
				else
					return false;
			}

			// If the size is 1, it occupies one space, so just return
			if (1 == bSize)
				return true;
			else
			{
				int j = 1;

				do
				{
					const int p = wCell + (BELT_INVENTORY_SLOT_WIDTH * j);

					if (p >= BELT_INVENTORY_SLOT_COUNT)
						return false;

					if (m_pointsInstant.wInventoryItemGrid[p])
					{
						if (m_pointsInstant.bBeltGrid[p] != iExceptionCell)
							return false;
					}
				} while (++j < bSize);

				return true;
			}
		}
		break;

#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
		{
			const uint16_t wCell = Cell.cell;
			if (wCell >= SWITCHBOT_SLOT_COUNT)
			{
				return false;
			}

			if (m_pointsInstant.pSwitchbotItems[wCell])
			{
				return false;
			}

			return true;
		}
		break;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case ADDITIONAL_EQUIPMENT_1:
		{
			const uint16_t wCell = Cell.cell;
			if (wCell >= ADDITIONAL_EQUIPMENT_MAX_SLOT)
				return false;

			if (m_pointsInstant.pAdditionalEquipmentItem_1[wCell])
				return false;

			return true;
		}
		break;
#endif

		default:
			break;
	}

	return false;
}

int CHARACTER::GetEmptyInventoryCount(uint8_t size) const
{
	int emptyCount = 0;
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i) {
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
			++emptyCount;
	}

	return emptyCount;
}

#ifdef ENABLE_SPECIAL_INVENTORY
bool CHARACTER::IsEmptySpecialItemGrid(TItemPos Cell, uint8_t bSize, int iExceptionCell) const
{
	if (bSize > 1)
	{
		return false;
	}

	const uint16_t wCell = Cell.cell;

	const SpecialInventoryRange range = SpecialInventoryRange(Cell.GetSpecialInventoryType(), 0, 0);
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	if (wCell < range.start || wCell >= GetExtendSpecialInvenMax(range.type))
# else
	if (wCell < range.start || wCell >= range.end)
# endif
	{
		return false;
	}

	if (m_pointsInstant.wInventoryItemGrid[wCell])
	{
		return m_pointsInstant.wInventoryItemGrid[wCell] == iExceptionCell;
	}

	return true;
}

int CHARACTER::GetEmptyInventory(LPITEM item) const
{
	if (!item)
		return -1;

	int start = 0;
#	ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	int end = GetExtendInvenMax();
#	else
	int end = INVENTORY_MAX_NUM;
#	endif

	if (item->GetSpecialInventoryType() != -1)
	{
		const SpecialInventoryRange range = SpecialInventoryRange(item->GetSpecialInventoryType(), start, end);
		start = range.start;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
		end = GetExtendSpecialInvenMax(range.type);
# else
		end = range.end;
# endif
	}

	for (int i = start; i < end; ++i)
	{
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), item->GetSize()))
		{
			return i;
		}
	}

	return -1;
}
#else
int CHARACTER::GetEmptyInventory(uint8_t size) const
{
	// NOTE: Currently, this function is used to find empty spaces in the inventory when performing actions such as item payment and acquisition.
	// Belt inventory is special inventory, so don't check it. (Basic inventory: scan up to INVENTORY_MAX_NUM)
#	ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#	else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#	endif
	{
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
			return i;
	}

	return -1;
}
#endif

int CHARACTER::GetEmptyDragonSoulInventory(LPITEM pItem) const
{
	if (nullptr == pItem || !pItem->IsDragonSoul())
		return -1;
	if (!DragonSoul_IsQualified())
	{
		return -1;
	}
	const uint8_t bSize = pItem->GetSize();
	const uint16_t wBaseCell = DSManager::Instance().GetBasePosition(pItem);

	if (WORD_MAX == wBaseCell)
		return -1;

	for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
		if (IsEmptyItemGrid(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), bSize))
			return i + wBaseCell;

	return -1;
}

void CHARACTER::CopyDragonSoulItemGrid(std::vector<uint16_t>& vDragonSoulItemGrid) const
{
	vDragonSoulItemGrid.resize(DRAGON_SOUL_INVENTORY_MAX_NUM);

	//std::copy(m_pointsInstant.wDSItemGrid.begin(), m_pointsInstant.wDSItemGrid.end() + DRAGON_SOUL_INVENTORY_MAX_NUM, vDragonSoulItemGrid.begin());
	std::copy(m_pointsInstant.wDSItemGrid, m_pointsInstant.wDSItemGrid + DRAGON_SOUL_INVENTORY_MAX_NUM, vDragonSoulItemGrid.begin());
}

int CHARACTER::CountEmptyInventory() const
{
	int count = 0;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (GetInventoryItem(i))
			count += GetInventoryItem(i)->GetSize();
	}

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	return (GetExtendInvenMax() - count);
#else
	return (INVENTORY_MAX_NUM - count);
#endif
}

void TransformRefineItem(LPITEM pkOldItem, LPITEM pkNewItem)
{
	if (!pkOldItem || !pkNewItem)
		return;

	// ACCESSORY_REFINE
	if (pkOldItem->IsAccessoryForSocket())
	{
#ifdef ENABLE_PROTO_RENEWAL
		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
		//pkNewItem->StartAccessorySocketExpireEvent();
	}
	// END_OF_ACCESSORY_REFINE
	else
	{
		// Here, the broken stone is automatically cleaned
#ifdef ENABLE_PROTO_RENEWAL
		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		// Socket setup
		int slot = 0;

#ifdef ENABLE_PROTO_RENEWAL
		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
		{
			const long socket = pkOldItem->GetSocket(i);

			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}

#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
		// Here, the broken stone is automatically cleaned
		for (int s = 3; s < ITEM_SOCKET_MAX_NUM; ++s)
		{
			if (!pkOldItem->GetSocket(s))
				break;
			else
				pkNewItem->SetSocket(s, 1);
		}

		// Sungma socket setup
		int socketRandomSlot = 3;

		for (int s = 3; s < ITEM_SOCKET_MAX_NUM; ++s)
		{
			const long socketRandom = pkOldItem->GetSocket(s);
			if (socketRandom > 5 && socketRandom != ITEM_BROKEN_METIN_VNUM)
			{
				pkNewItem->SetSocket(socketRandomSlot++, socketRandom);
			}
		}
#endif
	}

	// Magic item setting
	pkOldItem->CopyAttributeTo(pkNewItem);
}

void NotifyRefineSuccess(LPCHARACTER ch, LPITEM item, const char* way)
{
	if (nullptr != ch && item != nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");

		LogManager::Instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), 1, way);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		{
			if ((item->GetVnum() % 10) == 9 && !IsPlus15(item->GetVnum()))
				CAchievementSystem::Instance().OnUpgrade(ch, item->GetVnum(), false);
			else if (IsPlus15(item->GetVnum()))
				CAchievementSystem::Instance().OnUpgrade(ch, item->GetVnum(), true);
		}
#endif
	}
}

void NotifyRefineFail(LPCHARACTER ch, LPITEM item, const char* way, int success = 0)
{
	if (nullptr != ch && nullptr != item)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");

		LogManager::Instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), success, way);
	}
}

#ifdef ENABLE_REFINE_MSG_ADD
void NotifyRefineFailType(const LPCHARACTER pkChr, const LPITEM pkItem, const uint8_t bType, const std::string stRefineType, const uint8_t bSuccess = 0)
{
	if (pkChr && pkItem)
	{
		pkChr->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailedType %d", bType);
		LogManager::Instance().RefineLog(pkChr->GetPlayerID(), pkItem->GetName(), pkItem->GetID(), pkItem->GetRefineLevel(), bSuccess, stRefineType.c_str());
	}
}
#endif

void CHARACTER::SetRefineNPC(const LPCHARACTER& ch)
{
	if (ch != nullptr)
	{
		m_dwRefineNPCVID = ch->GetVID();
	}
	else
	{
		m_dwRefineNPCVID = 0;
	}
}

struct FindBlacksmith
{
	LPCHARACTER m_ch;
	bool found;
	bool m_throughGuild;
	int MAX_BLACKSMITH_DIST = 7500; //Around 5000 is the max distance, but to have a bit of room.

	FindBlacksmith(LPCHARACTER ch, bool throughGuild) {
		m_ch = ch;
		found = false;
		m_throughGuild = throughGuild;
	}

	void operator()(LPENTITY ent)
	{
		if (found) //Skip all the next entities - we already got what we wanted
			return;

		if (ent->IsType(ENTITY_CHARACTER)) {
			LPCHARACTER ch = (LPCHARACTER)ent;
			if (ch->IsNPC()) {
				uint32_t vnum = ch->GetMobTable().dwVnum;
				switch (vnum) {
					case BLACKSMITH_WEAPON_MOB:
					case BLACKSMITH_ARMOR_MOB:
					case BLACKSMITH_ACCESSORY_MOB:
					{
						if (!m_throughGuild) //We can ignore proximity to guild blacksmiths if we are not using guild
							return;

						int dist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY());
						if (test_server)
							sys_log(0, "Guild blacksmith found within %d distance units.", dist);

						if (dist < MAX_BLACKSMITH_DIST) {
							found = true;
						}

						break;
					}

					case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
					case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
					case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
					case BLACKSMITH_MOB:
					case BLACKSMITH2_MOB:
					{
						if (m_throughGuild) //Poximity to non-guild blacksmiths is irrelevant if we are using guild
							return;

						int dist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY());
						if (test_server)
							sys_log(0, "Blacksmith (%lu) found within %d distance units.", vnum, dist);

						if (dist < MAX_BLACKSMITH_DIST) {
							found = true;
						}

						break;
					}
				}
			}
		}

		return;
	}
};

bool CHARACTER::DoRefine(LPITEM item, bool bMoneyOnly)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	//@fixme410
	FindBlacksmith f(this, IsRefineThroughGuild());
	if (LPSECTREE tree = GetSectree())
		tree->ForEachAround(f);

	if (!f.found) {
		LogManager::Instance().HackLog("REFINE_FAR_BLACKSMITH", this);
		//We let continue - GMs will decide/ban.
	}
	//@end_@fixme410

#ifdef BLOCK_REFINE_ON_BASIC
	if (item->IsBasicItem()) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	//Improvement time limit: In upgrade_refine_scroll.quest, perform general improvement within 5 minutes after improvement.
	//Cannot proceed
	if (quest::CQuestManager::Instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::Instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable* prt = CRefineManager::Instance().GetRefineRecipe(item->GetRefineSet());
	if (!prt)
		return false;

	const uint32_t result_vnum = item->GetRefinedVnum();

	// REFINE_COST
	int cost = ComputeRefineFee(prt->cost);

	int RefineChance = GetQuestFlag("main_quest_lv7.refine_chance");

	if (RefineChance > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1003]");
			return false;
		}

		cost = 0;
		SetQuestFlag("main_quest_lv7.refine_chance", RefineChance - 1);
	}
	// END_OF_REFINE_COST

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;991]");
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
		return false;

	const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefine NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
		return false;
	}

	// REFINE_COST
	if (GetGold() < cost)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
		return false;
	}

	if (!bMoneyOnly && !RefineChance)
	{
		for (int i = 0; i < prt->material_count; ++i)
		{
#ifdef ENABLE_PENDANT
			if (CountSpecifyItem(prt->materials[i].vnum, item->GetCell()) < prt->materials[i].count)	//@fixme491
#else
			if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
#endif
			{
				if (test_server)
				{
					ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
				}
				ChatPacket(CHAT_TYPE_INFO, "[LS;1035]");
				return false;
			}
		}

		for (int i = 0; i < prt->material_count; ++i)
		{
#ifdef ENABLE_PENDANT
			RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count, item->GetCell());
#else
			RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);
#endif
		}
	}

	int prob = number(1, 100);

#ifdef ENABLE_REFINE_ABILITY_SKILL
	if (IsRefineThroughGuild() || bMoneyOnly) {
		prob += 10 + aiGuildRefinePowerByLevel[MINMAX(0, GetSkillLevel(SKILL_REFINE), 20)/*+3%*/];
	}
	else {
		prob += aiRefinePowerByLevel[MINMAX(0, GetSkillLevel(SKILL_REFINE), 20)/*+6%*/];
	}
#else
	if (IsRefineThroughGuild() || bMoneyOnly)
		prob -= 10;
#endif
	// END_OF_REFINE_COST

	if (prob <= prt->prob)
	{
		// Success! All items disappear, and other items with the same attribute are acquired
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
#ifdef ENABLE_SEALBIND_SYSTEM
			pkNewItem->SetSealDate(item->GetSealDate());
#endif
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::Instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			const uint16_t wCell = item->GetCell();	//@fixme519

			// DETAIL_REFINE_LOG
			NotifyRefineSuccess(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
			DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
#ifdef ENABLE_STACKABLES_STONES
			if (item->GetType() == ITEM_METIN)
				item->SetCount(item->GetCount() - 1);
			else
				ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
#else
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
#endif
			// END_OF_DETAIL_REFINE_LOG

#ifdef ENABLE_ANNOUNCEMENT_REFINE_SUCCES
			if (!item->CheckItemUseLevel(ITEM_LEVEL_LIMIT) && item->GetType() == ITEM_WEAPON ||	// Item-Levellimit <64 (ab 65) - Waffen
				!item->CheckItemUseLevel(ITEM_LEVEL_LIMIT) && item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_BODY)		// Item-Levellimit <64 (ab 65) - Rüstungen
			{
				if (pkNewItem->GetItemGrade() >= ANNOUNCEMENT_REFINE_SUCCES_MIN_LEVEL)	// Upgradestufe (+8 -> +9)
				{
					char szUpgradeAnnouncement[QUERY_MAX_LEN];
					snprintf(szUpgradeAnnouncement, sizeof(szUpgradeAnnouncement), "%s hat soebend den Gegenstand %s erfolgreich upgraded.", GetName(), pkNewItem->GetName());
					BroadcastNotice(szUpgradeAnnouncement);
				}
			}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
			if (IsConquerorItem(item))
				CRandomHelper::RefineRandomAttr(item, pkNewItem, true);
#endif

#ifdef ENABLE_STACKABLES_STONES
			if (item->GetType() == ITEM_METIN) // Nyx: Fix refinement Stackables SpiritStones
			{
				const auto iEmptyPos = GetEmptyInventory(item);
				if (iEmptyPos != -1)
				{
					pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, iEmptyPos));
				}
				else
				{
					PIXEL_POSITION pos = GetXYZ();
					pkNewItem->AddToGround(GetMapIndex(), pos);
					pkNewItem->SetOwnership(this);
					pkNewItem->StartDestroyEvent();

					pos.x += number(-7, 7) * 20;
					pos.y += number(-7, 7) * 20;
				}
			}
			else
				pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
#else
			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
			UpdateExtBattlePassMissionProgress(BP_ITEM_REFINE, 1, item->GetVnum());
#endif
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

			sys_log(0, "Refine Success %d", cost);
			pkNewItem->AttrLog();
			//PointChange(POINT_GOLD, -cost);
			sys_log(0, "PayPee %d", cost);
			PayRefineFee(cost);
			sys_log(0, "PayPee End %d", cost);
		}
		else
		{
			// DETAIL_REFINE_LOG
			// Failed to create item -> Considered as improvement failure
			sys_err("cannot create item %u", result_vnum);
#ifdef ENABLE_REFINE_MSG_ADD
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, IsRefineThroughGuild() ? "GUILD" : "POWER");
#else
			NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
#endif
			// END_OF_DETAIL_REFINE_LOG
		}
	}
	else
	{
		// Failure! All items disappear.
		DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
#ifdef ENABLE_REFINE_MSG_ADD
		NotifyRefineFailType(this, item, REFINE_FAIL_DEL_ITEM, IsRefineThroughGuild() ? "GUILD" : "POWER");
#else
		NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
#endif
		item->AttrLog();
#ifdef ENABLE_STACKABLES_STONES
		if (item->GetType() == ITEM_METIN)
			item->SetCount(item->GetCount() - 1);
		else
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
#else
		ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
#endif

		//PointChange(POINT_GOLD, -cost);
		PayRefineFee(cost);
	}

	return true;
}

#ifdef ENABLE_QUEEN_NETHIS
bool CHARACTER::DoRefineSerpent(LPITEM item)
{
	if (!item)
		return false;

	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	const TRefineTable* prt = CRefineManager::Instance().GetRefineRecipe(item->GetRefineSet());
	if (!prt)
		return false;

	const uint32_t result_vnum = item->GetRefinedVnum();

	// REFINE_COST
	const int cost = ComputeRefineFee(prt->cost);
	// END_OF_REFINE_COST

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;991]");
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
		return false;

	const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(item->GetRefinedVnum());
	if (!pProto)
	{
		sys_err("DoRefineSerpent NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
		return false;
	}

	// REFINE_COST
	if (GetGold() < cost)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
		return false;
	}

	int prob = number(1, 100);
	prob -= 10;
	// END_OF_REFINE_COST

	if (prob <= prt->prob)
	{
		// Success! All items disappear, and other items with the same attribute are acquired
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
#ifdef ENABLE_SEALBIND_SYSTEM
			pkNewItem->SetSealDate(item->GetSealDate());
#endif
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::Instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			const uint16_t wCell = item->GetCell();	//@fixme519

			// DETAIL_REFINE_LOG
			NotifyRefineSuccess(this, item, "SERPENT");
			DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
			// END_OF_DETAIL_REFINE_LOG

#ifdef ENABLE_YOHARA_SYSTEM
			if (IsConquerorItem(item))
				CRandomHelper::RefineRandomAttr(item, pkNewItem, true);
#endif

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

			sys_log(0, "Refine Success %d", cost);
			pkNewItem->AttrLog();
			//PointChange(POINT_GOLD, -cost);
			sys_log(0, "PayPee %d", cost);
			PayRefineFee(cost);
			sys_log(0, "PayPee End %d", cost);
		}
		else
		{
			// DETAIL_REFINE_LOG
			// Failed to create item -> Considered as improvement failure
			// sys_err("cannot create item %u", result_vnum);
#ifdef ENABLE_REFINE_MSG_ADD
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, "SERPENT");
#else
			NotifyRefineFail(this, item, "SERPENT");
#endif
			// END_OF_DETAIL_REFINE_LOG
		}
	}
	else
	{
		// Failure! All items disappear.
		DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
#ifdef ENABLE_REFINE_MSG_ADD
		NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, "SERPENT");
#else
		NotifyRefineFail(this, item, "SERPENT");
#endif
		PayRefineFee(cost);
	}

	return true;
}
#endif

enum enum_RefineScrolls
{
	CHUKBOK_SCROLL			= 0,	// 0%	Segenssschriftrolle [25040, 72301, 76016]
	MAGIC_STONE_SCROLL		= 1,	// 0%	Magischer Stein [39001, 25041]
	DRAGON_SCROLL			= 2,	// 10%	Schriftrolle des Drachen	[39022, 71032, 72314, 76009]
	MUSIN_SCROLL			= 3,	// 100%	Schriftrolle des Krieges [39014, 71021]									-> Nur bis +4
	SMITH_HANDBOOK_SCROLL	= 4,	// 20%	Schmiede-Handbuch [39007, 70039, 72309]
	MEMO_SCROLL				= 5,	// 100%	Notiz des Schmieds, Notiz des Weisen [-[70027, 72306], [70028, 72307]-]	-> Nur +4 Items
	BDRAGON_SCROLL			= 6,	// 
#ifdef ENABLE_RITUAL_STONE
	RITUALS_SCROLL			= 7,	// 15%	Ritualstein [25042]
#endif
#ifdef ENABLE_SOUL_SYSTEM
	SOUL_EVOLVE_SCROLL		= 8,	// 100%	Seelenpergament (100%) [70602]
	SOUL_AWAKE_SCROLL		= 9,	// 60%	Seelenpergament [70603]
#endif
#ifdef ENABLE_SEAL_OF_GOD
	SEAL_GOD_SCROLL			= 10,	// 20%	Siegel der Götter [25043]
#endif
};

bool CHARACTER::DoRefineWithScroll(LPITEM item)
{
	if (!item)
		return false;

	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

#ifdef BLOCK_REFINE_ON_BASIC
	if (item->IsBasicItem()) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	ClearRefineMode();

	//Improvement time limit: In upgrade_refine_scroll.quest, perform general improvement within 5 minutes after improvement.
	//Cannot proceed
	if (quest::CQuestManager::Instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::Instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable* prt = CRefineManager::Instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	LPITEM pkItemScroll;

	// Check
	if (m_iRefineAdditionalCell < 0)
		return false;

	pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

	if (!pkItemScroll)
		return false;

	if (!(pkItemScroll->GetType() == ITEM_USE && pkItemScroll->GetSubType() == USE_TUNING))
		return false;

	if (pkItemScroll->GetVnum() == item->GetVnum())
		return false;

	const uint32_t result_vnum = item->GetRefinedVnum();
	const uint32_t result_fail_vnum = item->GetRefineFromVnum();

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;991]");
		return false;
	}

	const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefineWithScroll NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
		return false;
	}

	if (GetGold() < prt->cost)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;67]");
		return false;
	}

	for (int i = 0; i < prt->material_count; ++i)
	{
#ifdef ENABLE_PENDANT
		if (CountSpecifyItem(prt->materials[i].vnum, item->GetCell()) < prt->materials[i].count)	//@fixme491
#else
		if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
#endif
		{
			if (test_server)
			{
				ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
			}
			ChatPacket(CHAT_TYPE_INFO, "[LS;1035]");
			return false;
		}
	}

	for (int i = 0; i < prt->material_count; ++i)
	{
#ifdef ENABLE_PENDANT
		RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count, item->GetCell());	//@fixme491
#else
		RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);
#endif
	}

	const int prob = number(1, 100);
	int success_prob = prt->prob;
	bool bDestroyWhenFail = false;

	const char* szRefineType = "SCROLL";

	switch (pkItemScroll->GetValue(0))
	{
		case CHUKBOK_SCROLL:
			szRefineType = "CHUKBOK";
			break;

		case MAGIC_STONE_SCROLL: // Magischer Stein
		{
			const int percentageAdd = 10;	// +10% [total]
			success_prob = MINMAX(1, prt->prob + percentageAdd, 100);
			szRefineType = "MSTONE";
			bDestroyWhenFail = true;
		}
		break;

		case DRAGON_SCROLL: // Schriftrolle des Drachen
		{
			const char hyuniron_prob[9] = { 100, 75, 65, 55, 45, 40, 35, 25, 20 };
			success_prob = hyuniron_prob[MINMAX(0, item->GetRefineLevel(), 8)];
			szRefineType = "DRAGON_SCROLL";
		}
		break;

		case MUSIN_SCROLL: // Schriftrolle des Krieges
		{
			// War God's blessing book is 100% successful (up to +4 only)
			success_prob = 100;
			szRefineType = "MUSIN_SCROLL";
		}
		break;

		case SMITH_HANDBOOK_SCROLL: // Schmiede-Handbuch
		{
			const char yagong_prob[9] = { 100, 100, 90, 80, 70, 60, 50, 30, 20 };
			success_prob = yagong_prob[MINMAX(0, item->GetRefineLevel(), 8)];
			szRefineType = "SMITH_HANDBOOK_SCROLL";
		}
		break;

		case MEMO_SCROLL: // Notiz des Schmieds | Notiz des Weisen
		{
			success_prob = 100;
			szRefineType = "MEMO_SCROLL";
		}
		break;

		case BDRAGON_SCROLL: // x
		{
			success_prob = 80;
			szRefineType = "BDRAGON_SCROLL";
		}
		break;

#ifdef ENABLE_RITUAL_STONE
		case RITUALS_SCROLL:
		{
			const int percentageAdd = 15;
			success_prob = MINMAX(1, prt->prob + percentageAdd, 100);
			szRefineType = "RITUALS_SCROLL";
			bDestroyWhenFail = true;
		}
		break;
#endif

#ifdef ENABLE_SOUL_SYSTEM
		case SOUL_EVOLVE_SCROLL:
		case SOUL_AWAKE_SCROLL:
			szRefineType = "SOUL_SCROLL";
			break;
#endif

#ifdef ENABLE_SEAL_OF_GOD
		case SEAL_GOD_SCROLL: // Siegel der Götter
		{
			const int percentageAdd = 20;
			success_prob = MINMAX(1, prt->prob + percentageAdd, 100);
			szRefineType = "SEAL_GOD_SCROLL";
			bDestroyWhenFail = true;
		}
		break;
#endif

		default:
			sys_err("REFINE : Unknown refine scroll item. Value0: %d", pkItemScroll->GetValue(0));
			break;
	}

	pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

	if (prob <= success_prob)
	{
		// Success! All items disappear, and other items with the same attribute are acquired
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
#ifdef ENABLE_SEALBIND_SYSTEM
			// Setting the SealDate of the old item into the new item
			// so the new item is still bound
			pkNewItem->SetSealDate(item->GetSealDate());
#endif
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::Instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			const uint16_t wCell = item->GetCell();	//@fixme519

			NotifyRefineSuccess(this, item, szRefineType);
			DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
#ifdef ENABLE_STACKABLES_STONES
			if (item->GetType() == ITEM_METIN) // Nyx: Fix refinement Stackables SpiritStones
				item->SetCount(item->GetCount() - 1);
			else
				ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
#else
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
#endif

#ifdef ENABLE_ANNOUNCEMENT_REFINE_SUCCES
			if (!item->CheckItemUseLevel(ITEM_LEVEL_LIMIT) && item->GetType() == ITEM_WEAPON ||	// Item-Levellimit <64 (ab 65) - Waffen
				!item->CheckItemUseLevel(ITEM_LEVEL_LIMIT) && item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_BODY)		// Item-Levellimit <64 (ab 65) - Rüstungen
			{
				if (pkNewItem->GetItemGrade() >= ANNOUNCEMENT_REFINE_SUCCES_MIN_LEVEL)	// Upgradestufe (+8 -> +9)
				{
					char szUpgradeAnnouncement[QUERY_MAX_LEN];
					snprintf(szUpgradeAnnouncement, sizeof(szUpgradeAnnouncement), "%s hat soebend den Gegenstand %s erfolgreich upgraded.", GetName(), pkNewItem->GetName());
					BroadcastNotice(szUpgradeAnnouncement);
				}
			}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
			if (IsConquerorItem(item))
				CRandomHelper::RefineRandomAttr(item, pkNewItem, true);
#endif

#ifdef ENABLE_STACKABLES_STONES
			if (item->GetType() == ITEM_METIN) // Nyx: Fix refinement Stackables SpiritStones
			{
				const auto iEmptyPos = GetEmptyInventory(item);
				if (iEmptyPos != -1)
				{
					pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, iEmptyPos));
				}
				else
				{
					PIXEL_POSITION pos = GetXYZ();
					pkNewItem->AddToGround(GetMapIndex(), pos);
					pkNewItem->SetOwnership(this);
					pkNewItem->StartDestroyEvent();

					pos.x += number(-7, 7) * 20;
					pos.y += number(-7, 7) * 20;
				}
			}
			else
				pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
#else
			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
			UpdateExtBattlePassMissionProgress(BP_ITEM_REFINE, 1, item->GetVnum());
#endif
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();
			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			// Failed to create item -> Considered as improvement failure
			sys_err("cannot create item %u", result_vnum);
#ifdef ENABLE_REFINE_MSG_ADD
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
			NotifyRefineFail(this, item, szRefineType);
#endif
		}
	}
	else if (!bDestroyWhenFail && result_fail_vnum)
	{
		// Failure! All items disappear, and lower grade items of the same attribute are acquired.
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(result_fail_vnum, 1, 0, false);

		if (pkNewItem)
		{
#ifdef ENABLE_SEALBIND_SYSTEM
			// Setting the SealDate of the old item into the new item
			// so the new item is still bound
			pkNewItem->SetSealDate(item->GetSealDate());
#endif
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::Instance().ItemLog(this, pkNewItem, "REFINE FAIL", pkNewItem->GetName());

			const uint16_t wCell = item->GetCell();	//@fixme519

			DBManager::Instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
#ifdef ENABLE_REFINE_MSG_ADD
			NotifyRefineFailType(this, item, REFINE_FAIL_GRADE_DOWN, szRefineType, -1);
#else
			NotifyRefineFail(this, item, szRefineType, -1);
#endif
#ifdef ENABLE_STACKABLES_STONES
			if (item->GetType() == ITEM_METIN) // Nyx: Fix refinement Stackables SpiritStones
				item->SetCount(item->GetCount() - 1);
			else
				ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
#else
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
#endif

#ifdef ENABLE_YOHARA_SYSTEM
			if (IsConquerorItem(item))
				CRandomHelper::RefineRandomAttr(item, pkNewItem, false);
#endif

#ifdef ENABLE_STACKABLES_STONES
			if (item->GetType() == ITEM_METIN) // Nyx: Fix refinement Stackables SpiritStones
			{
				const auto iEmptyPos = GetEmptyInventory(item);
				if (iEmptyPos != -1)
				{
					pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, iEmptyPos));
				}
				else
				{
					PIXEL_POSITION pos = GetXYZ();
					pkNewItem->AddToGround(GetMapIndex(), pos);
					pkNewItem->SetOwnership(this);
					pkNewItem->StartDestroyEvent();

					pos.x += number(-7, 7) * 20;
					pos.y += number(-7, 7) * 20;
				}
			}
			else
				pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
#else
			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
#endif
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

			pkNewItem->AttrLog();

			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			// Failed to create item -> Considered as improvement failure
			sys_err("cannot create item %u", result_fail_vnum);
#ifdef ENABLE_REFINE_MSG_ADD
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
			NotifyRefineFail(this, item, szRefineType);
#endif
		}
	}
	else
	{
#ifdef ENABLE_REFINE_MSG_ADD
		NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
		NotifyRefineFail(this, item, szRefineType); // Items do not disappear when improving
#endif
		PayRefineFee(prt->cost);
	}

	return true;
}

#ifdef ENABLE_REFINE_ELEMENT
bool CHARACTER::ElementsSpellItem(LPITEM pkItem, LPITEM pkTarget)
{
	if (!pkItem || !pkTarget)
		return false;

	if (!CanHandleItem())
		return false;

	const uint16_t wCell = pkTarget->GetCell();

	if (wCell > INVENTORY_MAX_NUM)
		return false;

	const LPITEM& item = GetInventoryItem(wCell);

	if (!item)
		return false;

	TPacketGCElementsSpell p{};

	p.header = HEADER_GC_ELEMENTS_SPELL;
	p.pos = wCell;

	if (pkItem->GetSubType() == USE_ELEMENT_DOWNGRADE)
	{
		p.subheader = ELEMENT_SPELL_SUB_HEADER_OPEN;
		p.cost = GOLD_DOWNGRADE_BONUS_ELEMENTS_SPELL;
		p.func = false;
		p.grade_add = 0;
	}
	else if (pkItem->GetSubType() == USE_ELEMENT_UPGRADE)
	{
		p.subheader = ELEMENT_SPELL_SUB_HEADER_OPEN;
		p.cost = GOLD_ADD_BONUS_ELEMENTS_SPELL;
		p.func = true;

		if (pkTarget->GetElementGrade() == 0)
			p.grade_add = static_cast<uint8_t>(pkItem->GetValue(0));
		else
			p.grade_add = 0;
	}
	else if (pkItem->GetSubType() == USE_ELEMENT_CHANGE)
	{
		p.subheader = ELEMENT_SPELL_SUB_HEADER_OPEN_CHANGE;
		p.cost = GOLD_CHANGE_BONUS_ELEMENTS_SPELL;
	}

	GetDesc()->Packet(&p, sizeof(TPacketGCElementsSpell));

	SetOpenElementsSpell(true, pkItem->GetCell());
	return true;
}

void CHARACTER::ElementsSpellItemFunc(int pos, uint8_t type_select)
{
	if (GetOpenedWindow(W_EXCHANGE | W_SHOP_OWNER | W_SAFEBOX | W_CUBE
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
		| W_MYSHOP
#endif
#ifdef ENABLE_SWITCHBOT
		| W_SWITCHBOT
#endif
	))
	{
		SetOpenElementsSpell(false);
		return;
	}

	if (m_iElementsAdditionalCell < 0 || pos < 0)
		return;

	LPITEM itemWeapon = GetInventoryItem(pos);
	LPITEM itemElements = GetInventoryItem(m_iElementsAdditionalCell);

	if (!itemWeapon || !itemElements)
		return;

	if (itemElements->GetSubType() != USE_ELEMENT_UPGRADE && itemElements->GetSubType() != USE_ELEMENT_DOWNGRADE && itemElements->GetSubType() != USE_ELEMENT_CHANGE)
		return;

	if (itemWeapon->GetType() != ITEM_WEAPON)
		return;

	if (itemElements->GetSubType() == USE_ELEMENT_UPGRADE)
	{
		if (GetGold() < GOLD_ADD_BONUS_ELEMENTS_SPELL)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1240]");
			return;
		}

		const int percent = number(1, 100);

		if (percent <= PERCENT_ADD_BONUS_ELEMENTS_SPELL)
		{
			if (itemWeapon->GetElementGrade() == 0)
			{
				const uint32_t attack_element = number(ATTACK_RANGE_BONUS_ELEMENTS_SPELL_MIN, ATTACK_RANGE_BONUS_ELEMENTS_SPELL_MAX);
				const int16_t sValue = number(VALUES_RANGE_BONUS_ELEMENTS_SPELL_MIN, VALUES_RANGE_BONUS_ELEMENTS_SPELL_MAX);
				itemWeapon->SetElementNew(1, attack_element, static_cast<uint8_t>(itemElements->GetValue(0)), sValue);
				ChatPacket(CHAT_TYPE_COMMAND, "ElementsSpellSuceeded");
			}
			else if (itemWeapon->GetElementGrade() > 0)
			{
				if (itemWeapon->GetElementsType() != itemElements->GetValue(0))
					return;

				const uint32_t attack_element = itemWeapon->GetElementAttack(itemWeapon->GetElementGrade() - 1) + ATTACK_RANGE_BONUS_ELEMENTS_SPELL_MIN;
				const int16_t sValue = itemWeapon->GetElementsValue(itemWeapon->GetElementGrade() - 1) + VALUES_RANGE_BONUS_ELEMENTS_SPELL_MIN;
				itemWeapon->SetElementNew(itemWeapon->GetElementGrade() + 1, number(attack_element, attack_element + (ATTACK_RANGE_BONUS_ELEMENTS_SPELL_MAX - ATTACK_RANGE_BONUS_ELEMENTS_SPELL_MIN)), itemWeapon->GetElementsType(), number(sValue, sValue + (VALUES_RANGE_BONUS_ELEMENTS_SPELL_MAX - VALUES_RANGE_BONUS_ELEMENTS_SPELL_MIN)));
				ChatPacket(CHAT_TYPE_COMMAND, "ElementsSpellSuceeded");
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_COMMAND, "ElementsSpellFailed");
		}

		PointChange(POINT_GOLD, -GOLD_ADD_BONUS_ELEMENTS_SPELL);
	}
	else if (itemElements->GetSubType() == USE_ELEMENT_DOWNGRADE)
	{
		if (GetGold() < GOLD_DOWNGRADE_BONUS_ELEMENTS_SPELL)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1240]");
			return;
		}

		if ((itemWeapon->GetElementGrade()) - 1 <= 0)
			itemWeapon->DeleteAllElement(0);
		else
			itemWeapon->DeleteAllElement((itemWeapon->GetElementGrade()) - 1);

		ChatPacket(CHAT_TYPE_COMMAND, "ElementsSpellDownGradeSuceeded");

		PointChange(POINT_GOLD, -GOLD_DOWNGRADE_BONUS_ELEMENTS_SPELL);
	}
	else if (itemElements->GetSubType() == USE_ELEMENT_CHANGE)
	{
		if (GetGold() < GOLD_CHANGE_BONUS_ELEMENTS_SPELL)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1240]");
			return;
		}

		if (itemWeapon->GetElementGrade() <= 0)
			return;

		if (type_select == itemWeapon->GetElementsType())
			return;

		itemWeapon->ChangeElement(type_select);
		PointChange(POINT_GOLD, -GOLD_CHANGE_BONUS_ELEMENTS_SPELL);
	}

	itemElements->SetCount(itemElements->GetCount() - 1);
}

void CHARACTER::SetOpenElementsSpell(bool b, int iAdditionalCell)
{
	m_OpenElementsSpell = b;
	m_iElementsAdditionalCell = iAdditionalCell;
}
#endif

bool CHARACTER::RefineInformation(uint16_t wCell, uint8_t bType, int iAdditionalCell)	//@fixme519
{
#ifdef ENABLE_SPECIAL_INVENTORY
	if (wCell > INVENTORY_MAX_NUM)
	{
		if (!(wCell >= SPECIAL_INVENTORY_STONE_START && wCell < SPECIAL_INVENTORY_SLOT_END))
		{
			return false;
		}
	}
#else
	if (wCell > INVENTORY_MAX_NUM)
		return false;
#endif

	LPITEM item = GetInventoryItem(wCell);

	if (!item)
		return false;

	// REFINE_COST
	if (bType == REFINE_TYPE_MONEY_ONLY)
	{
#ifdef ENABLE_QUEEN_NETHIS
		if (SnakeLair::CSnk::Instance().IsSnakeMap(GetMapIndex()))
		{
			if (get_global_time() < GetQuestFlag("snake_lair.refine_time"))
			{
				ChatPacket(CHAT_TYPE_INFO, "You can only be rewarded each 24h for the Serpent Temple Smith."); // Custom
				return false;
			}
		}
		else
#endif
		{
			if (!GetQuestFlag("deviltower_zone.can_refine"))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1067]");
				return false;
			}
		}
	}
	// END_OF_REFINE_COST

	TPacketGCRefineInformation p{};

	p.header = HEADER_GC_REFINE_INFORMATION;
	p.wPos = wCell;
	p.src_vnum = item->GetVnum();
	p.result_vnum = item->GetRefinedVnum();
	p.type = bType;

	if (p.result_vnum == 0)
	{
		sys_err("RefineInformation p.result_vnum == 0");
		ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
	{
		if (bType == 0)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1077]");
			return false;
		}
		else
		{
			const LPITEM& itemScroll = GetInventoryItem(iAdditionalCell);
			if (!itemScroll || item->GetVnum() == itemScroll->GetVnum())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1106]");
				ChatPacket(CHAT_TYPE_INFO, "[LS;1096]");
				return false;
			}
		}
	}

#ifdef ENABLE_SOUL_SYSTEM
	if (item->GetType() == ITEM_SOUL)
	{
		if (bType == REFINE_TYPE_SOUL_EVOLVE)
		{
			p.prob = SOUL_REFINE_EVOLVE_PROB;
			p.material_count = 0;
			memset(p.materials, 0, sizeof(p.materials));
			p.cost = 0;
		}
		else if (bType == REFINE_TYPE_SOUL_AWAKE)
		{
			p.prob = SOUL_REFINE_AWAKE_PROB;
			p.material_count = 0;
			memset(p.materials, 0, sizeof(p.materials));
			p.cost = 0;
		}

		GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));
		SetRefineMode(iAdditionalCell);
		return true;
	}
#endif

	CRefineManager& rm = CRefineManager::Instance();

	const TRefineTable* prt = rm.GetRefineRecipe(item->GetRefineSet());

	if (!prt)
	{
		sys_err("RefineInformation NOT GET REFINE SET %d", item->GetRefineSet());
		ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
		return false;
	}

	// REFINE_COST

	p.cost = ComputeRefineFee(prt->cost);
	//MAIN_QUEST_LV7
	if (GetQuestFlag("main_quest_lv7.refine_chance") > 0)
	{
		// Excluding Japan
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1003]");
			return false;
		}
		p.cost = 0;
	}
	//END_MAIN_QUEST_LV7

#ifdef ENABLE_REFINE_ABILITY_SKILL
	if (IsRefineThroughGuild())	//@fixme446
	{
		p.prob = MINMAX(0, prt->prob + 10 + aiGuildRefinePowerByLevel[MINMAX(0, GetSkillLevel(SKILL_REFINE), 20)/*+3%*/], 100);
	}
	else
	{
		const int refine_skill = aiRefinePowerByLevel[MINMAX(0, GetSkillLevel(SKILL_REFINE), 20)/*+6%*/];

		int scroll_buff = 0;
		const LPITEM& iScroll = GetInventoryItem(iAdditionalCell);
		if (iScroll)
		{
			if (iScroll->GetValue(0) == MAGIC_STONE_SCROLL)
				scroll_buff = 10;
			else if (iScroll->GetValue(0) == DRAGON_SCROLL) {
				const char hyuniron_prob[9] = { 100, 75, 65, 55, 45, 40, 35, 25, 20 };
				scroll_buff = hyuniron_prob[MINMAX(0, item->GetRefineLevel(), 8)];
			}
			else if (iScroll->GetValue(0) == MUSIN_SCROLL)
				scroll_buff = 100;
			else if (iScroll->GetValue(0) == SMITH_HANDBOOK_SCROLL) {
				const char yagong_prob[9] = { 100, 100, 90, 80, 70, 60, 50, 30, 20 };
				scroll_buff = yagong_prob[MINMAX(0, item->GetRefineLevel(), 8)];
			}
			else if (iScroll->GetValue(0) == MEMO_SCROLL)
				scroll_buff = 100;
			else if (iScroll->GetValue(0) == BDRAGON_SCROLL)
				scroll_buff = 80;
#ifdef ENABLE_RITUAL_STONE
			else if (iScroll->GetValue(0) == RITUALS_SCROLL)
				scroll_buff = 15;
#endif
#ifdef ENABLE_SEAL_OF_GOD
			else if (iScroll->GetValue(0) == SEAL_GOD_SCROLL)
				scroll_buff = 20;
#endif
		}

		p.prob = MINMAX(0, prt->prob + refine_skill + scroll_buff, 100);
	}
#else
	p.prob = prt->prob;
#endif

	if (bType == REFINE_TYPE_MONEY_ONLY)
	{
		p.material_count = 0;
		memset(p.materials, 0, sizeof(p.materials));
	}
	else
	{
		p.material_count = prt->material_count;
		thecore_memcpy(&p.materials, prt->materials, sizeof(prt->materials));
	}
	// END_OF_REFINE_COST

	GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));

	SetRefineMode(iAdditionalCell);
	return true;
}

bool CHARACTER::RefineItem(LPITEM pkItem, LPITEM pkTarget)
{
	if (!pkItem || !pkTarget)
		return false;

	if (!CanHandleItem())
		return false;

	if (pkItem->GetSubType() == USE_TUNING)
	{
		// XXX performance, socket improvements are gone...
		// XXX performance improvement book became a blessing book!

		uint8_t refType = REFINE_TYPE_SCROLL;
		switch (pkItem->GetValue(0))
		{
			case MAGIC_STONE_SCROLL:
				refType = REFINE_TYPE_HYUNIRON;
				break;

			case MEMO_SCROLL:
			{
				if (pkTarget->GetRefineLevel() != pkItem->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;162]");
					return false;
				}
			}
			break;

			case MUSIN_SCROLL:
			{
				if (pkTarget->GetRefineLevel() >= 4)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;1056]");
					return false;
				}

				refType = REFINE_TYPE_MUSIN;
			}
			break;

			case BDRAGON_SCROLL:
			{
				if (pkTarget->GetType() != ITEM_METIN || pkTarget->GetRefineLevel() != 4)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;162]");
					return false;
				}

				if (pkTarget->GetRefineSet() != 702)
					return false;

				refType = REFINE_TYPE_BDRAGON;
			}
			break;

#ifdef ENABLE_RITUAL_STONE
			case RITUALS_SCROLL:
			{
				if (pkTarget->GetLevelLimit() <= 80)
					return false;

				refType = REFINE_TYPE_RITUALS_SCROLL;
			}
			break;
#endif
#ifdef ENABLE_SOUL_SYSTEM
			case SOUL_EVOLVE_SCROLL:
			case SOUL_AWAKE_SCROLL:
			{
				if (pkTarget->GetType() != ITEM_SOUL)
					return false;

				if ((pkTarget->GetType() == ITEM_SOUL) && (pkTarget->GetSocket(1) == 1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot upgrade a soul item if it is activated."));
					return false;
				}

				if (pkItem->GetValue(0) == SOUL_EVOLVE_SCROLL)
					refType = REFINE_TYPE_SOUL_EVOLVE;
				else if (pkItem->GetValue(0) == SOUL_EVOLVE_SCROLL)
					refType = REFINE_TYPE_SOUL_AWAKE;
			}
			break;
#endif
#ifdef ENABLE_SEAL_OF_GOD
			case SEAL_GOD_SCROLL:
			{
				refType = REFINE_TYPE_SEAL_GOD;
			}
			break;
#endif

			default:
			{
				if (pkTarget->GetRefineSet() == 501)
					return false;

				if (pkTarget->GetVnum() >= 28300 && pkTarget->GetVnum() <= 28345)	//@fixme437 - Gesteine+3
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]Spirit Stones +3 cannot be upgraded with this item."));
					return false;
				}

				if (pkTarget->GetVnum() >= 28400 && pkTarget->GetVnum() <= 28445)	//@fixme437 - Gesteine+4
				{
					if (pkItem->GetVnum() != 71056) // Tinktur des Himmels
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]This Spirit Stone cannot be upgraded with this item."));
						return false;
					}
				}

				refType = REFINE_TYPE_SCROLL;
			}
			break;
		}

		RefineInformation(pkTarget->GetCell(), refType, pkItem->GetCell());
	}
	else if (pkItem->GetSubType() == USE_DETACHMENT && IS_SET(pkTarget->GetFlag(), ITEM_FLAG_REFINEABLE))
	{
		LogManager::Instance().ItemLog(this, pkTarget, "USE_DETACHMENT", pkTarget->GetName());

		bool bHasMetinStone = false;

#ifdef ENABLE_PROTO_RENEWAL
		for (int i = 0; i < METIN_SOCKET_MAX_NUM; i++)
#else
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
#endif
		{
			const long socket = pkTarget->GetSocket(i);
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
			{
				bHasMetinStone = true;
				break;
			}
		}

		if (bHasMetinStone)
		{
#ifdef ENABLE_PROTO_RENEWAL
			for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
			{
				const long socket = pkTarget->GetSocket(i);
				if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				{
					AutoGiveItem(socket);
					//TItemTable* pTable = ITEM_MANAGER::Instance().GetTable(pkTarget->GetSocket(i));
					//pkTarget->SetSocket(i, pTable->alValues[2]);
					// Replace with broken stone
					pkTarget->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
				}
			}

#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
			bool bHasSocketRandom = false;

			for (int s = 3; s < ITEM_SOCKET_MAX_NUM; s++)
			{
				const long socketRandom = pkTarget->GetSocket(s);
				if (socketRandom > 5 && socketRandom != ITEM_BROKEN_METIN_VNUM)
				{
					bHasSocketRandom = true;
					break;
				}
			}

			if (bHasSocketRandom)
			{
				for (int s = 3; s < ITEM_SOCKET_MAX_NUM; ++s)
				{
					const long socketRandom = pkTarget->GetSocket(s);
					if (socketRandom > 5 && socketRandom != ITEM_BROKEN_METIN_VNUM)
					{
						// Replace with empty bonus
						pkTarget->SetSocket(s, ITEM_BROKEN_METIN_VNUM);
					}
				}
			}
#endif

			pkItem->SetCount(pkItem->GetCount() - 1);
			return true;
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1108]");
			return false;
		}
	}

	return false;
}

EVENTFUNC(kill_campfire_event)
{
	const char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("kill_campfire_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) // <Factor>
		return 0;

	ch->m_pkMiningEvent = nullptr;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

bool CHARACTER::GiveRecallItem(LPITEM item)
{
	if (!item)
		return false;

	const int idx = GetMapIndex();
	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	//Maps where player can't save their scroll position
	switch (idx)
	{
		case MAP_DEVILTOWER1:
		case MAP_DEVILCATACOMB:
		// block dd maps	//@fixme476
		case MAP_CAPEDRAGONHEAD:
		case MAP_DAWNMISTWOOD:
		case MAP_BAYBLACKSAND:
		case MAP_MT_THUNDER:
		//End block dd maps
			iEmpireByMapIndex = -1;
			break;

		default:
			break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1119]");
		return false;
	}

	int pos = -1;

	if (item->GetCount() == 1) // If there is only one item, just set it.
	{
		item->SetSocket(0, GetX());
		item->SetSocket(1, GetY());
	}
#ifdef ENABLE_SPECIAL_INVENTORY
	else if ((pos = GetEmptyInventory(item)) != -1)
#else
	else if ((pos = GetEmptyInventory(item->GetSize())) != -1) // If not, find another inventory slot.
#endif
	{
		LPITEM item2 = ITEM_MANAGER::Instance().CreateItem(item->GetVnum(), 1);

		if (nullptr != item2)
		{
			item2->SetSocket(0, GetX());
			item2->SetSocket(1, GetY());
			item2->AddToCharacter(this, TItemPos(INVENTORY, pos));

			item->SetCount(item->GetCount() - 1);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
		return false;
	}

	return true;
}

void CHARACTER::ProcessRecallItem(LPITEM item)
{
	if (!item)
		return;

	int idx;

	if ((idx = SECTREE_MANAGER::Instance().GetMapIndex(item->GetSocket(0), item->GetSocket(1))) == 0)
		return;

	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case MAP_DEVILTOWER1:
		case MAP_DEVILCATACOMB:
			iEmpireByMapIndex = -1;
			break;

		case MAP_CAPEDRAGONHEAD:
		case MAP_DAWNMISTWOOD:
		case MAP_BAYBLACKSAND:
		case MAP_MT_THUNDER:
			if (GetLevel() < 90)
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1013]");
				return;
			}
			else
				break;

		default:
			break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1141]");
		item->SetSocket(0, 0);
		item->SetSocket(1, 0);
	}
	else
	{
		sys_log(1, "Recall: %s %d %d -> %d %d", GetName(), GetX(), GetY(), item->GetSocket(0), item->GetSocket(1));
		WarpSet(item->GetSocket(0), item->GetSocket(1));
		item->SetCount(item->GetCount() - 1);
	}
}

void CHARACTER::__OpenPrivateShop()
{
#ifdef ENABLE_OPEN_SHOP_ONLY_IN_MARKET
	if (!CShop::CanOpenShopHere(GetMapIndex()) && GetGMLevel() != GM_IMPLEMENTOR)	//@custom023
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_OPEN_SHOP_ON_THIS_MAP_2"));
		return;
	}
#endif

#ifdef ENABLE_GM_BLOCK
	if (IsGM() && !GetStaffPermissions())
	{
		ChatPacket(CHAT_TYPE_INFO, "[GM] GM's können keinen Privatenladen öffnen!");
		return;
}
#endif

#ifdef ENABLE_CANT_CREATE_SHOPS_IN_OTHER_EMPIRE
	if (SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(GetMapIndex()) != GetEmpire() && GetGMLevel() != GM_IMPLEMENTOR) {
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_OPEN_SHOP_IN_OTHER_EMPIRE"));
		return;
	}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (GetGrowthPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1165]");
		return;
	}
#endif

#ifdef ENABLE_MYSHOP_DECO
	SendMyPrivShopOpen(1, false);
#else
#ifdef ENABLE_OPEN_SHOP_WITH_ARMOR
	ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
#else
	unsigned bodyPart = GetPart(PART_MAIN);
	switch (bodyPart)
	{
		case 0:
		case 1:
		case 2:
			ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
			break;

		default:
			ChatPacket(CHAT_TYPE_INFO, "[LS;1025]");
			break;
	}
#endif
#endif
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#ifdef ENABLE_MYSHOP_DECO
void CHARACTER::OpenPrivateShop(uint8_t bTabCount /*= 1*/, bool bIsCashItem /*= false*/)
#else
void CHARACTER::OpenPrivateShop(bool shopstate)
#endif
{
#ifdef ENABLE_GM_BLOCK
	if (IsGM() && !GetStaffPermissions())
	{
		ChatPacket(CHAT_TYPE_INFO, "[GM] GM's können keinen Privatenladen öffnen!");
		return;
	}
#endif

	/*const LPCHARACTER& myShopChar = CHARACTER_MANAGER::Instance().FindPCShopCharacterByPID(GetPlayerID());
	if (!myShopChar && !shopstate)
	{
		ChatPacket(CHAT_TYPE_INFO, "not able to open shop, use bag instead");
		return;
	}*/

	if (GetShopGoldStash() > 0
#	ifdef ENABLE_CHEQUE_SYSTEM //__OFFLINE_SHOP_CHEQUE__
		|| GetShopChequeStash() > 0
#	endif
		)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1711]");
		return;
	}

	if (!GetShopItems().empty())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1712]");
		return;
	}

#	ifdef ENABLE_CANT_CREATE_SHOPS_IN_OTHER_EMPIRE
	if (SECTREE_MANAGER::Instance().GetEmpireFromMapIndex(GetMapIndex()) != GetEmpire() && GetGMLevel() != GM_IMPLEMENTOR)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_OPEN_SHOP_IN_OTHER_EMPIRE"));
		return;
	}
#	endif

#	ifdef ENABLE_OPEN_SHOP_ONLY_IN_MARKET
	if (!CShop::CanOpenShopHere(GetMapIndex()) && GetGMLevel() != GM_IMPLEMENTOR)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1687]");
		return;
	}
#	endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (GetGrowthPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1165]");
		return;
	}
#endif

#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	SendMyPrivShopOpen(bTabCount, bIsCashItem);
#else
	ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
#endif
}
#endif

// MYSHOP_PRICE_LIST
#ifdef ENABLE_CHEQUE_SYSTEM
void CHARACTER::SendMyShopPriceListCmd(uint32_t dwItemVnum, TItemPriceType ItemPrice)
#else
void CHARACTER::SendMyShopPriceListCmd(uint32_t dwItemVnum, uint32_t dwItemPrice)
#endif
{
#ifdef ENABLE_CHEQUE_SYSTEM
	// missing reverse
#else
	char szLine[256];
	snprintf(szLine, sizeof(szLine), "MyShopPriceList %u %u", dwItemVnum, dwItemPrice);
	ChatPacket(CHAT_TYPE_COMMAND, szLine);
	sys_log(0, szLine);
#endif
}

//
// Sends the list received from the DB cache to the user and sends a command to open a store.
//
void CHARACTER::UseSilkBotaryReal(const TPacketMyshopPricelistHeader* p)
{
	const TItemPriceInfo* pInfo = (const TItemPriceInfo*)(p + 1);

	if (!p->byCount)
	{
		// There is no price list. Sends a command with dummy data.
#ifdef ENABLE_CHEQUE_SYSTEM
		SendMyShopPriceListCmd(1, TItemPriceType());
#else
		SendMyShopPriceListCmd(1, 0);
#endif
	}
	else
	{
		for (int idx = 0; idx < p->byCount; idx++)
		{
#ifdef ENABLE_CHEQUE_SYSTEM
			SendMyShopPriceListCmd(pInfo[idx].dwVnum, TItemPriceType(pInfo[idx].price.dwPrice, pInfo[idx].price.dwCheque));
#else
			SendMyShopPriceListCmd(pInfo[idx].dwVnum, pInfo[idx].dwPrice);
#endif
		}
	}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	OpenPrivateShop();
#else
	__OpenPrivateShop();
#endif
}

//
// When the shop is opened for the first time after this connection, a price information list request packet is sent to the DB cache to load the list.
// From then on, they immediately reply to open the shop.
//
void CHARACTER::UseSilkBotary(void)
{
	if (m_bNoOpenedShop)
	{
		const uint32_t dwPlayerID = GetPlayerID();
		db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_REQ, GetDesc()->GetHandle(), &dwPlayerID, sizeof(uint32_t));
		m_bNoOpenedShop = false;
	}
	else
	{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		OpenPrivateShop();
#else
		__OpenPrivateShop();
#endif
	}
}
// END_OF_MYSHOP_PRICE_LIST


#ifdef ENABLE_ATTRIBUTE_CHANGE_ON_START_ITEM	//@custom007
bool CHARACTER::StartWeaponBlockAttribute(int Vnum, int Type, int SubType)
{
/*	int listBlockVnums[] = {
		21900,	// Durendals Rächer
		21901,	// Meuchelnder Zahnstocher
		21902	// Traumfänger
	};

	for (int i = 0; i < _countof(listBlockVnums); i++)
	{
		if (Vnum == listBlockVnums[i])
		{
			return true;
		}
	}*/

	if (Vnum >= 21900 && Vnum <= 21993)
	{
		return true;
	}

	return false;
}
#endif

//@custom001
void CHARACTER::SetAffectPotion(LPITEM item)
{
	if (!item)
		return;

	const int iPotionAffects[] = { AFFECT_POTION_1, AFFECT_POTION_2, AFFECT_POTION_3, AFFECT_POTION_4, AFFECT_POTION_5, AFFECT_POTION_6 };
	const int iPotionVnums[] = { 50821, 50822, 50823, 50824, 50825, 50826 };

	for (int i = 0; i < _countof(iPotionVnums); i++)
	{
		if (item->GetVnum() == static_cast<int>(iPotionVnums[i]))
			AddAffect(iPotionAffects[i], APPLY_NONE, 0, AFF_NONE, item->GetSocket(2), 0, false, false);
	}
}

#ifdef ENABLE_YOHARA_SYSTEM
bool CHARACTER::IsConquerorItem(LPITEM pItem)
{
	if (pItem->ItemHasApplyRandom())
		return true;

	return false;
}
#endif

int CalculateConsume(LPCHARACTER ch)
{
	if (!ch)
		return 0;

	static constexpr int WARP_NEED_LIFE_PERCENT = 30;
	static constexpr int WARP_MIN_LIFE_PERCENT = 10;
	// CONSUME_LIFE_WHEN_USE_WARP_ITEM
	int consumeLife = 0;
	{
		// CheckNeedLifeForWarp
		const int curLife = ch->GetHP();
		constexpr int needPercent = WARP_NEED_LIFE_PERCENT;
		const int needLife = ch->GetMaxHP() * needPercent / 100;
		if (curLife < needLife)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;1152]");
			return -1;
		}

		consumeLife = needLife;

		// CheckMinLifeForWarp: You must not be killed by poison, so leave a minimum amount of life.
		constexpr int minPercent = WARP_MIN_LIFE_PERCENT;
		const int minLife = ch->GetMaxHP() * minPercent / 100;
		if (curLife - needLife < minLife)
			consumeLife = curLife - minLife;

		if (consumeLife < 0)
			consumeLife = 0;
	}
	// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM
	return consumeLife;
}

int CalculateConsumeSP(LPCHARACTER lpChar)
{
	if (!lpChar)
		return 0;

	static constexpr int NEED_WARP_SP_PERCENT = 30;

	const int curSP = lpChar->GetSP();
	const int needSP = lpChar->GetMaxSP() * NEED_WARP_SP_PERCENT / 100;

	if (curSP < needSP)
	{
		lpChar->ChatPacket(CHAT_TYPE_INFO, "[LS;1162]");
		return -1;
	}

	return needSP;
}

bool CHARACTER::UseItemEx(LPITEM item, TItemPos DestCell)
{
	int iLimitRealtimeStartFirstUseFlagIndex = -1;
	//int iLimitTimerBasedOnWearFlagIndex = -1;

	const uint16_t wDestCell = DestCell.cell;
	const uint8_t bDestInven = DestCell.window_type;
	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		const long limitValue = item->GetProto()->aLimits[i].lValue;

		switch (item->GetProto()->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
				if (GetLevel() < limitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;1013]");
					return false;
				}
				break;

			case LIMIT_REAL_TIME_START_FIRST_USE:
				iLimitRealtimeStartFirstUseFlagIndex = i;
				break;

			case LIMIT_TIMER_BASED_ON_WEAR:
				//iLimitTimerBasedOnWearFlagIndex = i;
				break;

			default:
				break;
		}
	}

	if (test_server)
	{
		sys_log(0, "USE_ITEM %s, Inven %d, Cell %d, ItemType %d, SubType %d", item->GetName(), bDestInven, wDestCell, item->GetType(), item->GetSubType());
	}

	if (CArenaManager::Instance().IsLimitedItem(GetMapIndex(), item->GetVnum()))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
		return false;
	}
#ifdef ENABLE_NEWSTUFF
	else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && IsLimitedPotionOnPVP(item->GetVnum()))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
		return false;
	}
#endif

	// @fixme402 (IsLoadedAffect to block affect hacking)
	if (!IsLoadedAffect())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Affects are not loaded yet!"));
		return false;
	}

	// @fixme141 BEGIN
	if (TItemPos(item->GetWindow(), item->GetCell()).IsBeltInventoryPosition())
	{
		LPITEM beltItem = GetWear(WEAR_BELT);

		if (nullptr == beltItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Belt> You can't use this item if you have no equipped belt."));
			return false;
		}

		if (false == CBeltInventoryHelper::IsAvailableCell(item->GetCell() - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Belt> You can't use this item if you don't upgrade your belt."));
			return false;
		}
	}
	// @fixme141 END

	// After the initial use of an item, time is deducted even if it is not used.
	if (-1 != iLimitRealtimeStartFirstUseFlagIndex
#ifdef ENABLE_GROWTH_PET_SYSTEM
		&& item->GetType() != ITEM_PET
#endif
		)
	{
		// Whether the item has been used even once is determined by looking at Socket1. (Record the number of uses in Socket1)
		if (0 == item->GetSocket(1))
		{
			// Use the limit value as the default value for the available time, but if there is a value in Socket0, use that value. (Unit is seconds)
			long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[iLimitRealtimeStartFirstUseFlagIndex].lValue;

			if (0 == duration)
				duration = 60 * 60 * 24 * 7;

			item->SetSocket(0, time(0) + duration);
			item->StartRealTimeExpireEvent();
		}

		if (false == item->IsEquipped())
			item->SetSocket(1, item->GetSocket(1) + 1);
	}

	switch (item->GetType())
	{
		case ITEM_HAIR:
			return ItemProcess_Hair(item, wDestCell);

		case ITEM_POLYMORPH:
			return ItemProcess_Polymorph(item);

		case ITEM_QUEST:
		{
#ifdef ENABLE_QUEST_DND_EVENT
			if (IS_SET(item->GetFlag(), ITEM_FLAG_APPLICABLE))
			{
				LPITEM item2;

				if (!GetItem(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
					return false;

				quest::CQuestManager::instance().DND(GetPlayerID(), item, item2, false);
				return true;
			}
#endif

			if (GetArena() != nullptr || IsObserverMode() == true)
			{
				if (item->IsHorseSummonItem() || item->IsRideItem())
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
					return false;
				}
			}

#ifdef ENABLE_MAILBOX
			if (item->GetVnum() == MOBILE_MAILBOX)
			{
				CMailBox::Open(this);
			}
#endif

#ifdef ENABLE_CHANGE_LOOK_MOUNT
			if (item->IsHorseSummonItem() && item->GetChangeLookVnum())
			{
				ChatPacket(CHAT_TYPE_INFO, "SetHorseChangeLookVnum while activation");
				SetHorseChangeLookVnum(item->GetChangeLookVnum());
				/*TItemTable* pMountItem = nullptr;
				pMountItem = ITEM_MANAGER::Instance().GetTable(item->GetChangeLookVnum());
				SetHorseChangeLookVnum(pMountItem->aApplies[0].lValue);*/
			}
#endif

			if (!IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
			{
				if (item->GetSIGVnum() == 0)
				{
					quest::CQuestManager::Instance().UseItem(GetPlayerID(), item, false);
				}
				else
				{
					quest::CQuestManager::Instance().SIGUse(GetPlayerID(), item->GetSIGVnum(), item, false);
				}
			}
		}
		break;

		case ITEM_CAMPFIRE:
		{
			if (thecore_pulse() < LastCampFireUse + 60)	{	//@fixme502
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Please wait a second."));
				return false;
			}

			float fx, fy;
			GetDeltaByDegree(GetRotation(), 100.0f, &fx, &fy);

			LPSECTREE tree = SECTREE_MANAGER::Instance().Get(GetMapIndex(), static_cast<long>(GetX() + fx), static_cast<long>(GetY() + fy));

			if (!tree)
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1217]");
				return false;
			}

			if (tree->IsAttr(static_cast<long>(GetX() + fx), static_cast<long>(GetY() + fy), ATTR_WATER))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1228]");
				return false;
			}

			LPCHARACTER campfire = CHARACTER_MANAGER::Instance().SpawnMob(fishing::CAMPFIRE_MOB, GetMapIndex(), static_cast<long>(GetX() + fx), static_cast<long>(GetY() + fy), 0, false, number(0, 359));

			char_event_info* info = AllocEventInfo<char_event_info>();

			info->ch = campfire;

			campfire->m_pkMiningEvent = event_create(kill_campfire_event, info, PASSES_PER_SEC(40));

			item->SetCount(item->GetCount() - 1);

			LastCampFireUse = thecore_pulse();	//@fixme502
		}
		break;

		case ITEM_UNIQUE:
		{
			switch (item->GetSubType())
			{
				case USE_ABILITY_UP:
				{
					if (FindAffect(AFFECT_BLEND, static_cast<uint16_t>(item->GetValue(0)))) {	//@custom032
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_WATER_AND_BLEND_ITEM_TOGETHER"));
						return false;
					}

					switch (item->GetValue(0))
					{
						case APPLY_MOV_SPEED:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true, true);
							break;

						case APPLY_ATT_SPEED:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true, true);
							break;

						case APPLY_STR:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_DEX:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_CON:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_INT:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_CAST_SPEED:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_RESIST_MAGIC:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_ATT_GRADE_BONUS:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_GRADE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_DEF_GRADE_BONUS:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DEF_GRADE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

#ifdef ENABLE_YOHARA_SYSTEM
						case APPLY_SUNGMA_STR:
							AddAffect(AFFECT_SUNGMA_STR, POINT_SUNGMA_STR, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_SUNGMA_HP:
							AddAffect(AFFECT_SUNGMA_HP, POINT_SUNGMA_HP, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_SUNGMA_MOVE:
							AddAffect(AFFECT_SUNGMA_MOVE, POINT_SUNGMA_MOVE, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_SUNGMA_IMMUNE:
							AddAffect(AFFECT_SUNGMA_IMMUNE, POINT_SUNGMA_IMMUNE, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;
#endif

						default:
							break;
					}

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
				}
				break;

				case UNIQUE_10: // BundleItems
				{
					switch (item->GetVnum())
					{
						case 71049: // Silk
						{
							if (g_bEnableBootaryCheck)
							{
								if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
									UseSilkBotary();
								else
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot open a private shop on this map."));
							}
							else
								UseSilkBotary();
						}
						break;

#ifdef ENABLE_MYSHOP_DECO
						case 71221:
						{
#ifndef ENABLE_OPEN_SHOP_WITH_ARMOR
							if (GetWear(ARMOR_BODY))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1025]");
								return false;
							}
#endif
							ChatPacket(CHAT_TYPE_COMMAND, "OpenMyShopDecoWnd");
						}
						break;
#endif
						default:
							break;
					}
				}
				break;

				default:
				{
					if (!item->IsEquipped())
						EquipItem(item);
					else
						UnequipItem(item);
				}
				break;
			}
		}
		break;

		case ITEM_COSTUME:
		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_ROD:
		case ITEM_RING:
		case ITEM_BELT:
		case ITEM_PICK:
#ifdef ENABLE_PASSIVE_SYSTEM
		case ITEM_PASSIVE:
#endif
		{
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
		}
		break;

		/*
		* Dragon Spirit Stones that are not worn cannot be used.
		* If it's a normal cla, you can't send an item use packet about Dragon Spirit
		* Wearing Dragon Spirit Stone is done by item move packet.
		* Extract the worn dragon spirit stone.
		*/
		case ITEM_DS:
		{
			if (!item->IsEquipped())
				return false;

			return DSManager::Instance().PullOut(this, NPOS, item);
		}
		break;

		case ITEM_SPECIAL_DS:
		{
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
		}
		break;

		case ITEM_FISH:
		{
			if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
				return false;
			}
#ifdef ENABLE_NEWSTUFF
			else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
				return false;
			}
#endif

			if (item->GetSubType() == FISH_ALIVE)
				fishing::UseFish(this, item);
		}
		break;

		case ITEM_TREASURE_BOX:
		{
			return false;
		}
		break;

		case ITEM_TREASURE_KEY:
		{
			LPITEM item2;

			if (!GetItem(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
				return false;

			if (item2->GetType() != ITEM_TREASURE_BOX)
			{
				ChatPacket(CHAT_TYPE_TALKING, "[LS;1248]");
				return false;
			}

			if (item->GetValue(0) == item2->GetValue(0))
			{
				const uint32_t dwBoxVnum = item2->GetVnum();
				std::vector <uint32_t> dwVnums;
				std::vector <uint32_t> dwCounts;
				std::vector <LPITEM> item_gets(0);
				int count = 0;

				if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
				{
					item->SetCount(item->GetCount() - 1);
					item2->SetCount(item2->GetCount() - 1);

					for (int i = 0; i < count; i++)
					{
						switch (dwVnums[i])
						{
							case CSpecialItemGroup::GOLD:
#ifdef ENABLE_CHAT_SETTINGS
								ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#else
								ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#endif
								break;
							case CSpecialItemGroup::EXP:
#ifdef ENABLE_CHAT_SETTINGS
								ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1279]");
								ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1290;%d]", dwCounts[i]);
#else
								ChatPacket(CHAT_TYPE_INFO, "[LS;1279]");
								ChatPacket(CHAT_TYPE_INFO, "[LS;1290;%d]", dwCounts[i]);
#endif
								break;
							case CSpecialItemGroup::MOB:
								ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
								break;
							case CSpecialItemGroup::SLOW:
								ChatPacket(CHAT_TYPE_INFO, "[LS;1310]");
								break;
							case CSpecialItemGroup::DRAIN_HP:
								ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
								break;
							case CSpecialItemGroup::POISON:
								ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
								break;
#ifdef ENABLE_WOLFMAN_CHARACTER
							case CSpecialItemGroup::BLEEDING:
								ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
								break;
#endif
							case CSpecialItemGroup::MOB_GROUP:
								ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");

								break;
							default:
								if (item_gets[i])
								{
#ifdef ENABLE_CHAT_SETTINGS
									if (dwCounts[i] > 1)
										ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
									else
										ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetVnum());
#else
									if (dwCounts[i] > 1)
										ChatPacket(CHAT_TYPE_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
									else
										ChatPacket(CHAT_TYPE_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetName());
#endif
								}
						}
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, "[LS;46]");
					return false;
				}
			}
			else
			{
				ChatPacket(CHAT_TYPE_TALKING, "[LS;46]");
				return false;
			}
		}
		break;

		case ITEM_GIFTBOX:
		{
#ifdef ENABLE_NEWSTUFF
			if (0 != g_BoxUseTimeLimitValue)
			{
				if (get_dword_time() < m_dwLastBoxUseTime + g_BoxUseTimeLimitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot drop Yang yet"));
					return false;
				}
			}

			m_dwLastBoxUseTime = get_dword_time();
#endif
			const uint32_t dwBoxVnum = item->GetVnum();
			std::vector <uint32_t> dwVnums;
			std::vector <uint32_t> dwCounts;
			std::vector <LPITEM> item_gets(0);
			int count = 0;

			if (dwBoxVnum > 51500 && dwBoxVnum < 52000) // Dragon Spirit Stones
			{
				if (!(this->DragonSoul_IsQualified()))
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;1094]");
					return false;
				}
			}

#ifdef ENABLE_BLOCK_CHEST_INVENTORY_FULL	//@fixme489
			int iEmptyCell = -1;
			if (item->IsDragonSoul())
			{
				if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
					return false;
				}
			}
			else
			{
#	ifdef ENABLE_SPECIAL_INVENTORY
				if ((iEmptyCell = GetEmptyInventory(item)) == -1)
#	else
				if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
#	endif
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
					return false;
				}
			}
#endif	//@end_fixme489

			if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
			{
				item->SetCount(item->GetCount() - 1);

				for (int i = 0; i < count; i++)
				{
					switch (dwVnums[i])
					{
						case CSpecialItemGroup::GOLD:
#ifdef ENABLE_CHAT_SETTINGS
							ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#else
							ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#endif
							break;

						case CSpecialItemGroup::EXP:
#ifdef ENABLE_CHAT_SETTINGS
							ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1279]");
							ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1290;%d]", dwCounts[i]);
#else
							ChatPacket(CHAT_TYPE_INFO, "[LS;1279]");
							ChatPacket(CHAT_TYPE_INFO, "[LS;1290;%d]", dwCounts[i]);
#endif
							break;

						case CSpecialItemGroup::MOB:
							ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
							break;

						case CSpecialItemGroup::SLOW:
							ChatPacket(CHAT_TYPE_INFO, "[LS;1310]");
							break;

						case CSpecialItemGroup::DRAIN_HP:
							ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
							break;

						case CSpecialItemGroup::POISON:
							ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
							break;

#ifdef ENABLE_WOLFMAN_CHARACTER
						case CSpecialItemGroup::BLEEDING:
							ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
							break;
#endif

						case CSpecialItemGroup::MOB_GROUP:
							ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
							break;

						default:
							if (item_gets[i])
							{
#ifdef ENABLE_CHAT_SETTINGS
								if (dwCounts[i] > 1)
									ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
								else
									ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetVnum());
#else
								if (dwCounts[i] > 1)
									ChatPacket(CHAT_TYPE_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
								else
									ChatPacket(CHAT_TYPE_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetVnum());
#endif
							}
					}
				}
			}
			else
			{
				ChatPacket(CHAT_TYPE_TALKING, "[LS;56]");
				return false;
			}
		}
		break;

		case ITEM_SKILLFORGET:
		{
			if (!item->GetSocket(0))
			{
				ITEM_MANAGER::Instance().RemoveItem(item);
				return false;
			}

			const uint32_t dwVnum = item->GetSocket(0);

			if (SkillLevelDown(dwVnum))
			{
#ifdef ENABLE_BOOKS_STACKFIX
				item->SetCount(item->GetCount() - 1);
#else
				ITEM_MANAGER::Instance().RemoveItem(item);
#endif
				ChatPacket(CHAT_TYPE_INFO, "[LS;78]");
			}
			else
				ChatPacket(CHAT_TYPE_INFO, "[LS;88]");
		}
		break;

		case ITEM_SKILLBOOK:
		{
			if (IsPolymorphed())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
				return false;
			}

			uint32_t dwVnum = 0;

			if (item->GetVnum() == 50300)
			{
				dwVnum = item->GetSocket(0);
			}
			else
			{
				// The new training book has a skill number at value 0, so use it.
				dwVnum = item->GetValue(0);
			}

			if (0 == dwVnum)
			{
				ITEM_MANAGER::Instance().RemoveItem(item);

				return false;
			}

#ifdef ENABLE_678TH_SKILL
			if (Is678Skill(dwVnum))
			{
				if (true == Learn678SkillByBook(dwVnum))
				{
#	ifdef ENABLE_BOOKS_STACKFIX
					item->SetCount(item->GetCount() - 1);
#	else
					ITEM_MANAGER::Instance().RemoveItem(item);
#	endif
				}
				return false;
			}
#endif

#ifdef ENABLE_NINETH_SKILL
			if (IsNineSkill(dwVnum))
			{
				const auto bSkillGroup = static_cast<uint8_t>(item->GetValue(3));
				if (LearnNineSkillByBook(dwVnum, bSkillGroup))
				{
#	ifdef ENABLE_BOOKS_STACKFIX
					item->SetCount(item->GetCount() - 1);
#	else
					ITEM_MANAGER::Instance().RemoveItem(item);
#	endif
				}
				return false;
			}
#endif

			if (LearnSkillByBook(dwVnum))
			{
#ifdef ENABLE_BOOKS_STACKFIX
				item->SetCount(item->GetCount() - 1);
#else
				ITEM_MANAGER::Instance().RemoveItem(item);
#endif

				int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

				if (distribution_test_server)
					iReadDelay /= 3;

				SetSkillNextReadTime(dwVnum, get_global_time() + iReadDelay);
			}
		}
		break;

		case ITEM_USE:
		{
			if (item->GetVnum() > 50800 && item->GetVnum() <= 50820)
			{
				if (test_server)
					sys_log(0, "ADD addtional effect : vnum(%d) subtype(%d)", item->GetOriginalVnum(), item->GetSubType());

				const int affect_type = AFFECT_EXP_BONUS_EURO_FREE;
				const int apply_type = aApplyInfo[item->GetValue(0)].wPointType;
				const int apply_value = item->GetValue(2);
				const int apply_duration = item->GetValue(1);

				switch (item->GetSubType())
				{
					case USE_ABILITY_UP:
					{
						if (FindAffect(affect_type, apply_type))
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
							return false;
						}

						if (FindAffect(AFFECT_BLEND, static_cast<uint16_t>(item->GetValue(0)))) {	//@custom032
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_WATER_AND_BLEND_ITEM_TOGETHER"));
							return false;
						}

						{
							switch (item->GetValue(0))
							{
								case APPLY_MOV_SPEED:
									AddAffect(affect_type, apply_type, apply_value, AFF_MOV_SPEED_POTION, apply_duration, 0, true, true);
									break;

								case APPLY_ATT_SPEED:
									AddAffect(affect_type, apply_type, apply_value, AFF_ATT_SPEED_POTION, apply_duration, 0, true, true);
									break;

								case APPLY_STR:
								case APPLY_DEX:
								case APPLY_CON:
								case APPLY_INT:
								case APPLY_CAST_SPEED:
								case APPLY_RESIST_MAGIC:
								case APPLY_ATT_GRADE_BONUS:
								case APPLY_DEF_GRADE_BONUS:
#ifdef ENABLE_YOHARA_SYSTEM
								case APPLY_SUNGMA_STR:
								case APPLY_SUNGMA_HP:
								case APPLY_SUNGMA_MOVE:
								case APPLY_SUNGMA_IMMUNE:
#endif
									AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, true, true);
									break;

								default:
									break;
							}
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
					}
					break;

					case USE_AFFECT:
					{
						if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].wPointType))
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
						}
						else
						{
							AddAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].wPointType, item->GetValue(2), 0, item->GetValue(3), 0, false, true);
							item->SetCount(item->GetCount() - 1);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
							CAchievementSystem::Instance().OnFishItem(this, achievements::ETaskTypes::TYPE_USE_BURN, item->GetVnum());
#endif
						}
					}
					break;

					case USE_POTION_NODELAY:
#ifdef ENABLE_SUNG_MAHI_TOWER
					case USE_POTION_NODELAY_TOWER:
#endif
					{
#ifdef ENABLE_SUNG_MAHI_TOWER
						if (!IS_SUNG_MAHI_ENABLE_ITEM(this, GetMapIndex(), item->GetVnum()))
						{
							ChatPacket(CHAT_TYPE_INFO, "You cannot use this type of potion here.");
							return false;
						}
#endif

						if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
						{
							if (quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit") > 0)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
								return false;
							}

							switch (item->GetVnum())
							{
								case 70020:	//Pfirsichblütenwein
								case 71018:	//Segen des Lebens
								case 71019:	//Segen der Magie
								case 71020:	//Segen des Drachen
								{
									if (quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit_count") < 10000)
									{
										if (m_nPotionLimit <= 0)
										{
											ChatPacket(CHAT_TYPE_INFO, "[LS;122]");
											return false;
										}
									}
								}
								break;

								default:
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
									return false;
								}
								break;
							}
						}
	#ifdef ENABLE_NEWSTUFF
						else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
							return false;
						}
	#endif

						bool used = false;

						if (item->GetValue(0) != 0) // HP Absolute Value Recovery
						{
							if (GetHP() < GetMaxHP())
							{
								PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
								EffectPacket(SE_HPUP_RED);
								used = TRUE;
							}
						}

						if (item->GetValue(1) != 0) // SP absolute value recovery
						{
							if (GetSP() < GetMaxSP())
							{
								PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
								EffectPacket(SE_SPUP_BLUE);
								used = TRUE;
							}
						}

						if (item->GetValue(3) != 0) // HP% recovery
						{
							if (GetHP() < GetMaxHP())
							{
								PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
								EffectPacket(SE_HPUP_RED);
								used = TRUE;
							}
						}

						if (item->GetValue(4) != 0) // SP% recovery
						{
							if (GetSP() < GetMaxSP())
							{
								PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
								EffectPacket(SE_SPUP_BLUE);
								used = TRUE;
							}
						}

						if (used)
						{
							if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
							{
								if (test_server)
									ChatPacket(CHAT_TYPE_INFO, "Benutzter Mondkuchen oder Samen.");

								SetUseSeedOrMoonBottleTime();
							}
							if (GetDungeon())
								GetDungeon()->UsePotion(this);

							if (GetWarMap())
								GetWarMap()->UsePotion(this, item);

							m_nPotionLimit--;

							//RESTRICT_USE_SEED_OR_MOONBOTTLE
							item->SetCount(item->GetCount() - 1);
							//END_RESTRICT_USE_SEED_OR_MOONBOTTLE
						}
					}
					break;

					default:
						break;
				}

				return true;
			}

			if (item->GetVnum() >= 27863 && item->GetVnum() <= 27883)
			{
				if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
					return false;
				}
#ifdef ENABLE_NEWSTUFF
				else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
					return false;
				}
#endif
			}

			if (test_server)
			{
				sys_log(0, "USE_ITEM %s Type %d SubType %d vnum %d", item->GetName(), item->GetType(), item->GetSubType(), item->GetOriginalVnum());
			}

			switch (item->GetSubType())
			{
				case USE_TIME_CHARGE_PER:
				{
					LPITEM pDestItem = GetItem(DestCell);
					if (nullptr == pDestItem)
					{
						return false;
					}

					// First of all, let's talk about Dragon Spirit.
					if (pDestItem->IsDragonSoul())
					{
						int ret;
						char buf[128];
						if (item->GetVnum() == DRAGON_HEART_VNUM)
						{
							ret = pDestItem->GiveMoreTime_Per((float)item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
						}
						else
						{
							ret = pDestItem->GiveMoreTime_Per((float)item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
						}
						if (ret > 0)
						{
							if (item->GetVnum() == DRAGON_HEART_VNUM)
							{
								sprintf(buf, "Inc %ds by item{VN:%d SOC%d:%ld}", ret, item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
							}
							else
							{
								sprintf(buf, "Inc %ds by item{VN:%d VAL%d:%ld}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							}

							ChatPacket(CHAT_TYPE_INFO, "[LS;1093;%d]", ret);
							item->SetCount(item->GetCount() - 1);
							LogManager::Instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
							return true;
						}
						else
						{
							if (item->GetVnum() == DRAGON_HEART_VNUM)
							{
								sprintf(buf, "No change by item{VN:%d SOC%d:%ld}", item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
							}
							else
							{
								sprintf(buf, "No change by item{VN:%d VAL%d:%ld}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							}

							ChatPacket(CHAT_TYPE_INFO, "[LS;1066]");
							LogManager::Instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
							return false;
						}
					}
					else
						return false;
				}
				break;

				case USE_TIME_CHARGE_FIX:
				{
					LPITEM pDestItem = GetItem(DestCell);
					if (nullptr == pDestItem)
					{
						return false;
					}
					// First of all, let's talk about Dragon Spirit.
					if (pDestItem->IsDragonSoul())
					{
						const int ret = pDestItem->GiveMoreTime_Fix(item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
						char buf[128];
						if (ret)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;1093;%d]", ret);
							sprintf(buf, "Increase %ds by item{VN:%d VAL%d:%ld}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							LogManager::Instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
							item->SetCount(item->GetCount() - 1);
							return true;
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;1066]");
							sprintf(buf, "No change by item{VN:%d VAL%d:%ld}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							LogManager::Instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
							return false;
						}
					}
					else
						return false;
				}
				break;

				case USE_SPECIAL:
				{
					switch (item->GetVnum())
					{
#ifdef ENABLE_RESEARCHER_ELIXIR_FIX
						case 39023:
						case 71035:
						case 76020:
						{
							if (FindAffect(AFFECT_RESEARCHER_ELIXIR))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
								return false;
							}

							AddAffect(AFFECT_RESEARCHER_ELIXIR, APPLY_NONE, 0, AFF_NONE, INFINITE_AFFECT_DURATION, 0, false);
							item->SetCount(item->GetCount() - 1);
							return true;
						}
						break;
#endif

#ifdef ENABLE_DEFENSE_WAVE
						case 31107: //Reparaturholz
						{
							if (GetDungeon())
								GetDungeon()->UpdateMastHP(true);

							AddAffect(AFFECT_STUN, POINT_NONE, 0, AFF_STUN, 5, 0, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#ifdef ENABLE_ELEMENTAL_WORLD
						case 39113: // Orden der Urgewalt
						{
							if (GetMapIndex() != MAP_ELEMENTAL_04)
							{
								ChatPacket(CHAT_TYPE_INFO, "Can't use here!");
								return false;
							}

							CAffect* pAffect = FindAffect(AFFECT_PROTECTION_OF_ELEMENTAL);
							if (pAffect)
							{
								const int aff_dur = pAffect->lDuration;
								if (aff_dur + 600 > 3600)
								{
									ChatPacket(CHAT_TYPE_INFO, "Just only able to stack 6 times!");
									return false;
								}

								RemoveAffect(AFFECT_PROTECTION_OF_ELEMENTAL);
								AddAffect(AFFECT_PROTECTION_OF_ELEMENTAL, APPLY_NONE, 0, AFF_NONE, aff_dur + 600, 0, false);
							}
							else
								AddAffect(AFFECT_PROTECTION_OF_ELEMENTAL, APPLY_NONE, 0, AFF_NONE, 10 * 60, 0, false);
							
							ChatPacket(CHAT_TYPE_INFO, "Elemental Protection active for 10min!");
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
						case 70403: // Rätselhafter Schlüssel
						case 70405: // SungMa-Antidot
						{
							quest::CQuestManager::Instance().UseItem(GetPlayerID(), item, false);
						}
						break;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
						case 39046:	// Urkunde der Umkehr+
						case 90000:	// Urkunde der Umkehr
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item->IsExchanging() || item2->IsExchanging() || item2->IsEquipped())
								return false;

							if (item->isLocked() || item2->isLocked())
								return false;

#	ifdef ENABLE_SEALBIND_SYSTEM
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1521]");
								return false;
							}
#	endif

							if (item2->GetType() != ITEM_COSTUME || item2->GetSubType() != COSTUME_ACCE)
								return false;

							item2->SetSocket(0, 0);
							item2->ClearAllAttribute();
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

						//Christmas Lanzhou
						case ITEM_NOG_POCKET:
						{
							/*
							* Ranju ability value: Meaning of item_proto value
							* Movement speed value 1
							* Attack power value 2
							* Experience value 3
							* Duration value 0 (unit seconds)
							*/
							if (FindAffect(AFFECT_NOG_ABILITY))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
								return false;
							}
							const long time = item->GetValue(0);
							const long moveSpeedPer = item->GetValue(1);
							const long attPer = item->GetValue(2);
							const long expPer = item->GetValue(3);
							AddAffect(AFFECT_NOG_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
							AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
							AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						//Ramadan candy
						case ITEM_RAMADAN_CANDY:
						{
							/*
							* Candy stat value: meaning item_proto value
							* Movement speed value 1
							* Attack power value 2
							* Experience value 3
							* Duration value 0 (unit seconds)
							*/
							// @fixme147 BEGIN
							if (FindAffect(AFFECT_RAMADAN_ABILITY))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
								return false;
							}
							// @fixme147 END
							const long time = item->GetValue(0);
							const long moveSpeedPer = item->GetValue(1);
							const long attPer = item->GetValue(2);
							const long expPer = item->GetValue(3);
							AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
							AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
							AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_MARRIAGE_RING:	//Ehering
						{
							const marriage::TMarriage* pMarriage = marriage::CManager::Instance().Get(GetPlayerID());
							if (pMarriage)
							{
								if (pMarriage->ch1 != nullptr)
								{
									if (CArenaManager::Instance().IsArenaMap(pMarriage->ch1->GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
										break;
									}
#ifdef ENABLE_BATTLE_FIELD
									if (CBattleField::Instance().IsBattleZoneMapIndex(pMarriage->ch1->GetMapIndex()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't warp to this player right now."));
										break;
									}
#endif
								}

								if (pMarriage->ch2 != nullptr)
								{
									if (CArenaManager::Instance().IsArenaMap(pMarriage->ch2->GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
										break;
									}
#ifdef ENABLE_BATTLE_FIELD
									if (CBattleField::Instance().IsBattleZoneMapIndex(pMarriage->ch2->GetMapIndex()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't warp to this player right now."));
										break;
									}
#endif
								}

								const int consumeSP = CalculateConsumeSP(this);
								if (consumeSP < 0)
									return false;

								PointChange(POINT_SP, -consumeSP, false);

								WarpToPID(pMarriage->GetOther(GetPlayerID()));
							}
							else
								ChatPacket(CHAT_TYPE_INFO, "[LS;143]");
						}
						break;


#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
						case PRIVATE_SHOP_SEARCH_LOOKING_GLASS:
						case PRIVATE_SHOP_SEARCH_TRADING_GLASS:
						{
							OpenPrivateShopSearch(item->GetVnum());
						}
						break;
#endif

						case UNIQUE_ITEM_WHITE_FLAG:	//Weiße Flagge
						{
							ForgetMyAttacker();
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case UNIQUE_ITEM_TREASURE_BOX:	//Schatztruhe
						{
						}
						break;

#ifdef ENABLE_VIP_SYSTEM
						case 38001: //VIP-Feature 
						{

							if (GetGMLevel() >= GM_LOW_WIZARD) {	//@fixme440
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("GM_CANT_BE_VIPS"));
								return false;
							}

							char szQueryInsertVip[QUERY_MAX_LEN];
							snprintf(szQueryInsertVip, sizeof(szQueryInsertVip), "INSERT INTO common.gmlist (mAccount, mName, mContactIP, mServerIP, mAuthority) VALUES('%s', '%s', '%s', '%s', '%s')", GetDesc()->GetAccountTable().login, GetName(), "ALL", "ALL", "VIP");
							DBManager::Instance().DirectQuery(szQueryInsertVip);

							db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, nullptr, 0);
							RefreshVip();
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_GRANT_VIP"));
							ITEM_MANAGER::Instance().RemoveItem(item);

							//AutoGiveItem(27002);
							//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("WELCOME_VIP_GIFT"));
						}
						break;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
						case TRANSMUTATION_REVERSAL:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item->IsExchanging() || item2->IsExchanging() || item2->IsEquipped())
								return false;

							if (item->isLocked() || item2->isLocked())
								return false;

							if (!item2->GetChangeLookVnum())
								return false;

							item2->SetChangeLookVnum(0);
							item2->UpdatePacket();
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
						case 38306:
						case 76055:
						{
							if (GetPageTotalEquipment() + 1 >= NEW_EQUIPMENT_MAX_PAGE)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have unlocked the max pages of equipment."));
								return false;
							}

							PointChange(POINT_NEW_EQUIPMENT_TOTAL, 1);
							ChatPacket(CHAT_TYPE_COMMAND, "BINARY_REFRESH_NEW_EQUIPMENT");
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("EQUIPMENT_ADDITIONAL_PAGE_ACTIVE"));
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
						case 55610:
						case 55611:
						case 55612:
						case 55613:
						case 55614:
						case 55615:
						{
							if (FindAffect(AFFECT_MULTI_FARM_PREMIUM))
							{
								ChatPacket(CHAT_TYPE_INFO, "You have already this affect!");
								return false;
							}
							else
							{
								AddAffect(AFFECT_MULTI_FARM_PREMIUM, POINT_NONE, item->GetValue(1), AFF_NONE, item->GetValue(0), 0, false, false);
								item->SetCount(item->GetCount() - 1);
								CHARACTER_MANAGER::Instance().CheckMultiFarmAccount(GetDesc()->GetHostName(), GetPlayerID(), GetName(), GetMultiStatus());
								ChatPacket(CHAT_TYPE_INFO, "Affect succesfully added on your character!");
								ChatPacket(CHAT_TYPE_INFO, "If you want use this affect this character need active drop status!");
							}
						}
						break;
#endif

#ifdef ENABLE_NEWSTUFF_2017
						// Ice Cream Cone Nyx 2017.07.09
						case 50278:
						{
							/*if (FindAffect(AFFECT_ICE_CREAM_CONE))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
								return false;
							}*/

							const long time = item->GetValue(0);
							const long moveSpeedPer = item->GetValue(1);
							const long attPer = item->GetValue(2);
							const long expPer = item->GetValue(3);

							AddAffect(AFFECT_MALL, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
							AddAffect(AFFECT_MALL, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
							AddAffect(AFFECT_MALL, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);

							item->SetCount(item->GetCount() - 1);
						}
						break;

						// Dragon God Support
						case 39021:
						case 71031:
						{
							/*if (FindAffect(AFFECT_DRAGON_GOD_SUPPORT))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
								return false;
							}*/

							constexpr long duration = 60 * 30;
							AddAffect(AFFECT_MALL, POINT_ST, 5, AFF_NONE, duration, 0, true, true);
							AddAffect(AFFECT_MALL, POINT_HT, 5, AFF_NONE, duration, 0, true, true);
							AddAffect(AFFECT_MALL, POINT_DX, 5, AFF_NONE, duration, 0, true, true);
							AddAffect(AFFECT_MALL, POINT_IQ, 5, AFF_NONE, duration, 0, true, true);

							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

						case 30093:	//Glückstasche
						case 30094:	//Glückstasche
						case 30095:	//Glückstasche
						case 30096:	//Glückstasche
						//@fixme436
						case 30098:	//Glückstasche
						case 30099:	//Glückstasche
						case 30100:	//Glückstasche
						//@END_fixme436
							// Lucky bag
						{
							constexpr int MAX_BAG_INFO = 26;
							static struct LuckyBagInfo
							{
								uint32_t count;
								int prob;
								uint32_t vnum;
							} b1[MAX_BAG_INFO] =
							{
								{ 1000, 302, 1 },
								{ 10, 150, 27002 },
								{ 10, 75, 27003 },
								{ 10, 100, 27005 },
								{ 10, 50, 27006 },
								{ 10, 80, 27001 },
								{ 10, 50, 27002 },
								{ 10, 80, 27004 },
								{ 10, 50, 27005 },
								{ 1, 10, 50300 },
								{ 1, 6, 92 },
								{ 1, 2, 132 },
								{ 1, 6, 1052 },
								{ 1, 2, 1092 },
								{ 1, 6, 2082 },
								{ 1, 2, 2122 },
								{ 1, 6, 3082 },
								{ 1, 2, 3122 },
								{ 1, 6, 5052 },
								{ 1, 2, 5082 },
								{ 1, 6, 7082 },
								{ 1, 2, 7122 },
								{ 1, 1, 11282 },
								{ 1, 1, 11482 },
								{ 1, 1, 11682 },
								{ 1, 1, 11882 },
							};

							LuckyBagInfo* bi = nullptr;
							bi = b1;

							int pct = number(1, 1000);

							int i;
							for (i = 0; i < MAX_BAG_INFO; i++)
							{
								if (pct <= bi[i].prob)
									break;
								pct -= bi[i].prob;
							}
							if (i >= MAX_BAG_INFO)
								return false;

							if (bi[i].vnum == 50300)	//Fertigkeitsbuch
							{
								// Skill training book is given specially.
								GiveRandomSkillBook();
							}
							else if (bi[i].vnum == 1)
							{
								PointChange(POINT_GOLD, 1000, true);
							}
							else
							{
								AutoGiveItem(bi[i].vnum, bi[i].count);
							}
							ITEM_MANAGER::Instance().RemoveItem(item);
						}
						break;

						case 71144:	//@infome001
						{
							bool isGiftBundle = item->GetVnum() == 71144;

							if (P2P_MANAGER::Instance().GetDescCount() < 2) //Not really connected!
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT_GIFT_NOP2P"));
								return false;
							}

							int iTotalAmount;
							int * paiEmpireUserCount;
							int iLocalAmount;
							DESC_MANAGER::Instance().GetUserCount(iTotalAmount, &paiEmpireUserCount, iLocalAmount);
							int iRealTotal = P2P_MANAGER::Instance().CountPeerPlayers() + iLocalAmount;

							if (iRealTotal < 50 || iLocalAmount < 1) //Too little players!
							{
								sys_log(0, "%s: Not enough players when trying to gift (%d online)", GetName(), iRealTotal);

								if (!test_server || iRealTotal == 0 || iLocalAmount == 0) {
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT_GIFT_NO_ENOUGH_PEOPLE"));
									return false;
								}
							}

							//Let's pick our item from the possible pool
							struct LootInfo
							{
								uint32_t vnum;
								int count;
								int weight;
							};

							std::vector<LootInfo> loot
							{
								//{ vnum, { amount, weight } } - higher weight, higher chance
								{ 71085, 5, 10 },
								{ 71085, 2, 50 },
								{ 71084, 5, 5 },
								{ 71084, 2, 30 },
								{ 71002, 1, 1 },
								{ 70024, 3, 30 },
								{ 71003, 1, 3 },
								{ 71083, 1, 20 },
								{ 71083, 2, 5 },
								{ 71025, 2, 7 },
								{ 71025, 1, 20 },
								{ 71095, 10, 30 },
								{ 71095, 20, 20 },
								{ 71095, 30, 5 },
								{ 72726, 1, 10 },
								{ 72730, 1, 15 },
								{ 71107, 1, 15 },
								{ 71107, 2, 3 },
								{ 90094, 1, 15 },
								{ 71001, 1, 20 },
								{ 71001, 3, 7 },
								{ 71001, 5, 1 },
								{ 25040, 1, 10 },
								{ 25040, 2, 9 },
								{ 90091, 1, 7 },
								{ 90092, 1, 10 },
								{ 90093, 1, 13 },
								{ 70027, 1, 2 },

								//Costumes + hairstyles
								{ 41003, 1, 2 },
								{ 41004, 1, 2 },
								{ 41117, 1, 2 },
								{ 41118, 1, 2 },
								{ 45003, 1, 2 },
								{ 45002, 1, 2 },
								{ 73201, 1, 2 },
								{ 73205, 1, 2 },
								{ 45053, 1, 2 },
								{ 45054, 1, 2 },
								{ 45055, 1, 2 },
								{ 45056, 1, 2 },
								{ 45057, 1, 2 },
								{ 45058, 1, 2 },
								{ 41047, 1, 2 },
								{ 41048, 1, 2 },
								{ 41313, 1, 2 },
								{ 41314, 1, 2 },
								{ 41311, 1, 2 },
								{ 41312, 1, 2 },
								{ 45139, 1, 2 },
								{ 45140, 1, 2 },
								{ 45141, 1, 2 },
								{ 45142, 1, 2 },
								{ 45143, 1, 2 },
								{ 45144, 1, 2 },

								//MOre
								{ 50216, 1, 30 },
								{ 50216, 2, 20 },
								{ 50216, 3, 10 },
								{ 71136, 1, 5 },
								{ 27115, 10, 10 },
								{ 27115, 20, 5 },
								{ 27987, 1, 5 },
								{ 27989, 1, 5 },
							};

							int giftsToDistribute = isGiftBundle ? 3 : 1;
							if (iRealTotal - 1 < giftsToDistribute)
							{
								sys_log(0, "There were too little people online to give out %d gifts, reducing to %d gifts", giftsToDistribute, iRealTotal - 1);
								giftsToDistribute = iRealTotal - 1;
							}

							int snowmanCostumeChance = isGiftBundle ? 10 : 30;
							if (uniform_random(1, snowmanCostumeChance) == snowmanCostumeChance / 2)
							{
								uint32_t partReceived;
								//Body part
								if (number(0, 50) >= 25)
									partReceived = number(41482, 41485);
								else //Head part
									partReceived = number(45172, 45175);

								LPITEM newItem = ITEM_MANAGER::Instance().CreateItem(partReceived, 1);
								AutoGiveItem(newItem, true);

								char noticeBuf[128]{};
								snprintf(noticeBuf, sizeof(noticeBuf), LC_TEXT("%s_RECEIVED_SNOWMAN_COSTUME_PART"), GetName());
								BroadcastNotice(noticeBuf);
							}

							//Gifting weight algorithm
							int totalLootWeight = 0;
							for (const auto it : loot)
							{
								totalLootWeight += it.weight;
							}

							int totalAttempts = 0; //Safeguard to avoid an infinite loop
							int giftsDistributed = 0;
							std::vector<std::string> pickedPlayers;
							while (giftsToDistribute > 0 && totalAttempts < 50)
							{
								++totalAttempts;

								int pickedValue = uniform_random(0, totalLootWeight);

								uint32_t vnumPicked = 0;
								int amountPicked = 1;
								for (const auto it : loot)
								{
									pickedValue -= it.weight;
									if (pickedValue <= 0)
									{
										vnumPicked = it.vnum;
										amountPicked = it.count;
										break;
									}
								}

								if (!vnumPicked)
								{
									sys_err("Couldn't pick an vnum while trying to hand out a gift! [tv: %d, pv: %d]", totalLootWeight, pickedValue);
									return false;
								}

								const TItemTable* itemTable = ITEM_MANAGER::Instance().GetTable(vnumPicked);

								if (!itemTable)
								{
									sys_err("Item picked for gifting didn't seem to exist (%d)", vnumPicked);
									return false;
								}

								//We'll randomly pick either someone over P2P or someone locally
								int attemptsLeft = 10;
								bool gifted = false;
								char targetName[CHARACTER_NAME_MAX_LEN + 1]{};
								char targetLogin[LOGIN_MAX_LEN + 1]{};

								while (--attemptsLeft && !gifted)
								{
									if (uniform_random(1, iRealTotal) > iLocalAmount)
									{
										CCI * player = P2P_MANAGER::Instance().PickRandomCCI();
										if (!player)
											continue;

										if (player->level < 20)
											continue;

										//GMs shouldn't get stuff
										if (!test_server && gm_get_level(player->szName, 0, 0) > GM_PLAYER)
											continue;

										snprintf(targetName, sizeof(targetName), "%s", player->szName);
										snprintf(targetLogin, sizeof(targetLogin), "%s", player->login);

										LPDESC pkDesc = player->pkDesc;
										pkDesc->SetRelay(targetName);

										std::string cmdbuf = "gift_random";

										TPacketGCChat pack_chat{};
										pack_chat.header = HEADER_GC_CHAT;
										pack_chat.size = sizeof(TPacketGCChat) + cmdbuf.length() + 1;
										pack_chat.type = CHAT_TYPE_COMMAND;
										pack_chat.id = 0;
										pack_chat.bEmpire = GetDesc()->GetEmpire();

										TEMP_BUFFER buf;
										buf.write(&pack_chat, sizeof(TPacketGCChat));
										buf.write(cmdbuf.c_str(), cmdbuf.length() + 1);

										//Relay to another core
										pkDesc->Packet(buf.read_peek(), buf.size());
										pkDesc->SetRelay("");
									}
									else
									{
										const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::Instance().GetClientSet();
										auto it = c_ref_set.begin();
										std::advance(it, uniform_random(0, iLocalAmount - 1));

										if (it == c_ref_set.end())
											continue;

										LPDESC d = *it;
										if (!d || !d->GetCharacter())
											continue;

										if (d->GetCharacter() == this)
										{
											if (test_server)
												sys_log(0, "Skip - Random gifting picked myself! %s", GetName());

											continue;
										}

										if (d->GetCharacter()->IsGM() && !test_server)
											continue;

										if (d->GetCharacter()->GetLevel() < 20)
											continue;

										snprintf(targetName, sizeof(targetName), "%s", d->GetCharacter()->GetName());
										snprintf(targetLogin, sizeof(targetLogin), "%s", d->GetAccountTable().login);

										d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "gift_random");
									}

									gifted = true; //If reached here, its all ok
									break;
								}

								//Already gifted this guy
								if (std::find(pickedPlayers.begin(), pickedPlayers.end(), targetName) != pickedPlayers.end())
									continue;

								//All ok!
								pickedPlayers.emplace_back(targetName);
								if (targetLogin && *targetLogin)
								{
									if (!IS_SET(itemTable->dwFlags, ITEM_FLAG_STACKABLE))
									{
										for (int i = 0; i < amountPicked; ++i)
										{
											auto pmsg(DBManager::Instance().DirectQuery(
												"INSERT INTO item_award (login, vnum, count, given_time, mall) "
												"VALUES('%s', %lu, %d, NOW(), 1)", targetLogin, vnumPicked, 1));
										}
									}
									else
									{
										auto pmsg(DBManager::Instance().DirectQuery(
											"INSERT INTO item_award (login, vnum, count, given_time, mall) "
											"VALUES('%s', %lu, %d, NOW(), 1)", targetLogin, vnumPicked, amountPicked));
									}

									sys_log(0, "Gift! To: %s, vnum: %lu, am: %lu", targetLogin, vnumPicked, amountPicked);
								}
								else
								{
									sys_err("Gift (%d of %lu) could not be delivered to %s [wrong login]!", amountPicked, vnumPicked, targetName);
									continue;
								}

								--giftsToDistribute;

								char noticeBuf[128]{};
								snprintf(noticeBuf, sizeof(noticeBuf), LC_TEXT("%s_RECEIVED_%d_%s_FROM_RANDOM_%s"), targetName, amountPicked, itemTable->szLocaleName, GetName());
								BroadcastNotice(noticeBuf);

								++giftsDistributed;
							}

							if (!giftsDistributed)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT_GIFT_UNKNOWN_ERROR"));
								return false;
							}

							if (isGiftBundle && giftsDistributed > 0)
							{
								int pickedValue = uniform_random(0, totalLootWeight);

								uint32_t vnumPicked = 0;
								int amountPicked = 1;
								for (const auto it : loot)
								{
									pickedValue -= it.weight;
									if (pickedValue <= 0)
									{
										vnumPicked = it.vnum;
										amountPicked = it.count;
										break;
									}
								}

								//Reward and move on
								LPITEM newItem = ITEM_MANAGER::Instance().CreateItem(vnumPicked, amountPicked);
								AutoGiveItem(newItem, true);
							}

							SetQuestFlag("Item.gifts_given", GetQuestFlag("Item.gifts_given") + giftsDistributed);
							LogManager::Instance().CharLog(this, item->GetID(), "GIFT_OPEN", "");
							item->SetCount(item->GetCount() - 1);
							return true;
						}
						break;

						case 50004: //Ereignis-Detektor
						{
							if (item->GetSocket(0))
							{
								item->SetSocket(0, item->GetSocket(0) + 1);
							}
							else
							{
								// First time use
								const long iMapIndex = GetMapIndex();

								PIXEL_POSITION pos;

								if (SECTREE_MANAGER::Instance().GetRandomLocation(iMapIndex, pos, 700))
								{
									item->SetSocket(0, 1);
									item->SetSocket(1, pos.x);
									item->SetSocket(2, pos.y);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;154]");
									return false;
								}
							}

							int dist = 0;
							const float distance = (DISTANCE_SQRT(GetX() - item->GetSocket(1), GetY() - item->GetSocket(2)));

							if (distance < 1000.0f)
							{
								// discovery!
								ChatPacket(CHAT_TYPE_INFO, "[LS;165]");

								// Different items are given depending on the number of uses.
								struct TEventStoneInfo
								{
									uint32_t dwVnum;
									int count;
									int prob;
								};
								constexpr int EVENT_STONE_MAX_INFO = 15;
								TEventStoneInfo info_10[EVENT_STONE_MAX_INFO] =
								{
									{ 27001, 10, 8 },	//Roter Trank (K)
									{ 27004, 10, 6 },	//Blauer Trank (K)
									{ 27002, 10, 12 },	//Roter Trank (M)
									{ 27005, 10, 12 },	//Blauer Trank (M)
									{ 27100, 1, 9 },	//Grüner Trank (K)
									{ 27103, 1, 9 },	//Violetter Trank (K)
									{ 27101, 1, 10 },	//Grüner Trank (M)
									{ 27104, 1, 10 },	//Violetter Trank (M)
									{ 27999, 1, 12 },	//Geiststeintasche
									{ 25040, 1, 4 },	//Segensschriftrolle
									{ 27410, 1, 0 },	//Angelrute+2
									{ 27600, 1, 0 },	//Lagerfeuer
									{ 25100, 1, 0 },	//Geiststein-Schriftrolle
									{ 50001, 1, 0 },	//Glücksbuch
									{ 50003, 1, 1 },	//Fertigkeitszurücksetzung
								};
								TEventStoneInfo info_7[EVENT_STONE_MAX_INFO] =
								{
									{ 27001, 10, 1 },	//Roter Trank (K)
									{ 27004, 10, 1 },	//Blauer Trank (K)
									{ 27004, 10, 9 },	//Blauer Trank (K)
									{ 27005, 10, 9 },	//Blauer Trank (M)
									{ 27100, 1, 5 },	//Grüner Trank (K)
									{ 27103, 1, 5 },	//Violetter Trank (K)
									{ 27101, 1, 10 },	//Grüner Trank (M)
									{ 27104, 1, 10 },	//Violetter Trank (M)
									{ 27999, 1, 14 },	//Geiststeintasche
									{ 25040, 1, 5 },	//Segensschriftrolle
									{ 27410, 1, 5 },	//Angelrute+2
									{ 27600, 1, 5 },	//Lagerfeuer
									{ 25100, 1, 5 },	//Geiststein-Schriftrolle
									{ 50001, 1, 0 },	//Glücksbuch
									{ 50003, 1, 5 },	//Fertigkeitszurücksetzung
								};
								TEventStoneInfo info_4[EVENT_STONE_MAX_INFO] =
								{
									{ 27001, 10, 0 },	//Roter Trank (K)
									{ 27004, 10, 0 },	//Blauer Trank (K)
									{ 27002, 10, 0 },	//Roter Trank (M)
									{ 27005, 10, 0 },	//Blauer Trank (M)
									{ 27100, 1, 0 },	//Grüner Trank (K)
									{ 27103, 1, 0 },	//Violetter Trank (K)
									{ 27101, 1, 0 },	//Grüner Trank (M)
									{ 27104, 1, 0 },	//Violetter Trank (M)
									{ 27999, 1, 25 },	//Geiststeintasche
									{ 25040, 1, 0 },	//Segensschriftrolle
									{ 27410, 1, 0 },	//Angelrute+2
									{ 27600, 1, 0 },	//Lagerfeuer
									{ 25100, 1, 15 },	//Geiststein-Schriftrolle
									{ 50001, 1, 10 },	//Glücksbuch
									{ 50003, 1, 50 },	//Fertigkeitszurücksetzung
								};

								{
									TEventStoneInfo* info{};
									if (item->GetSocket(0) <= 4)
										info = info_4;
									else if (item->GetSocket(0) <= 7)
										info = info_7;
									else
										info = info_10;

									int prob = number(1, 100);

									for (int i = 0; i < EVENT_STONE_MAX_INFO; ++i)
									{
										if (!info[i].prob)
											continue;

										if (prob <= info[i].prob)
										{
											if (info[i].dwVnum == 50001)
											{
												uint32_t* pdw = M2_NEW uint32_t[2];

												pdw[0] = info[i].dwVnum;
												pdw[1] = info[i].count;

												// Lottery sets socket
												DBManager::Instance().ReturnQuery(QID_LOTTO, GetPlayerID(), pdw,
													"INSERT INTO lotto_list VALUES(0, 'server%s', %u, NOW())",
													get_table_postfix(), GetPlayerID());
											}
											else
												AutoGiveItem(info[i].dwVnum, info[i].count);

											break;
										}
										prob -= info[i].prob;
									}
								}

								char chatbuf[CHAT_MAX_LEN + 1];
								int len = snprintf(chatbuf, sizeof(chatbuf), "StoneDetect %u 0 0", (uint32_t)GetVID());

								if (len < 0 || len >= (int)sizeof(chatbuf))
									len = sizeof(chatbuf) - 1;

								++len; // Send up to \0 characters

								TPacketGCChat pack_chat{};
								pack_chat.header = HEADER_GC_CHAT;
								pack_chat.size = sizeof(TPacketGCChat) + len;
								pack_chat.type = CHAT_TYPE_COMMAND;
								pack_chat.id = 0;
								pack_chat.bEmpire = GetDesc()->GetEmpire();
								//pack_chat.id = vid;

								TEMP_BUFFER buf;
								buf.write(&pack_chat, sizeof(TPacketGCChat));
								buf.write(chatbuf, len);

								PacketAround(buf.read_peek(), buf.size());

								ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 1");
								return true;
							}
							else if (distance < 20000)
								dist = 1;
							else if (distance < 70000)
								dist = 2;
							else
								dist = 3;

							// If you use it a lot, it disappears.
							constexpr int STONE_DETECT_MAX_TRY = 10;
							if (item->GetSocket(0) >= STONE_DETECT_MAX_TRY)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;176]");
								ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 0");
								AutoGiveItem(27002);
								return true;
							}

							if (dist)
							{
								char chatbuf[CHAT_MAX_LEN + 1];
								int len = snprintf(chatbuf, sizeof(chatbuf),
									"StoneDetect %u %d %d",
									(uint32_t)GetVID(), dist, (int)GetDegreeFromPositionXY(GetX(), item->GetSocket(2), item->GetSocket(1), GetY()));

								if (len < 0 || len >= (int)sizeof(chatbuf))
									len = sizeof(chatbuf) - 1;

								++len; // Send up to \0 characters

								TPacketGCChat pack_chat{};
								pack_chat.header = HEADER_GC_CHAT;
								pack_chat.size = sizeof(TPacketGCChat) + len;
								pack_chat.type = CHAT_TYPE_COMMAND;
								pack_chat.id = 0;
								pack_chat.bEmpire = GetDesc()->GetEmpire();
								//pack_chat.id = vid;

								TEMP_BUFFER buf;
								buf.write(&pack_chat, sizeof(TPacketGCChat));
								buf.write(chatbuf, len);

								PacketAround(buf.read_peek(), buf.size());
							}

						}
						break;

						case 27989: // Kompass des Metinsteins
						case 76006: // Kompass des Metinsteins
						{
							LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(GetMapIndex());

							if (pMap != nullptr)
							{
								item->SetSocket(0, item->GetSocket(0) + 1);

								FFindStone f;

								// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
								pMap->for_each(f);

								if (f.m_mapStone.size() > 0)
								{
									std::map<uint32_t, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

									uint32_t max = UINT_MAX;
									LPCHARACTER pTarget = stone->second;

									while (stone != f.m_mapStone.end())
									{
										const uint32_t dist = (uint32_t)DISTANCE_SQRT(GetX()-stone->second->GetX(), GetY()-stone->second->GetY());

										if (dist != 0 && max > dist)
										{
											max = dist;
											pTarget = stone->second;
										}
										++stone;	//@fixme541
									}

									if (pTarget != nullptr)
									{
										int val = 3;

										if (max < 10000) val = 2;
										else if (max < 70000) val = 1;

										ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (uint32_t)GetVID(), val,
											(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]The compass for Metins is activated but does not detect any."));
									}
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]The compass for Metins is activated but does not detect any."));
								}

								if (item->GetSocket(0) >= 6)
								{
									ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (uint32_t)GetVID());
									item->SetCount(item->GetCount() - 1);
									item->SetSocket(0, item->GetSocket(0) - 6);
									// ITEM_MANAGER::Instance().RemoveItem(item);
								}
							}
							break;
						}
						break;

#ifdef ENABLE_DAWNMIST_DUNGEON
						// Guard Compass
						case 79602:
						{
							if (GetMapIndex() != MapTypes::MAP_MT_THUNDER_DUNGEON)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;118]");
								return false;
							}

							LPSECTREE_MAP pMap = SECTREE_MANAGER::Instance().GetMap(GetMapIndex());

							if (pMap != nullptr)
							{
								item->SetSocket(0, item->GetSocket(0) + 1);

								FFindMobVnum f(6405);

								// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
								pMap->for_each(f);

								if (f.m_mapVID.size() > 0)
								{
									std::map<uint32_t, LPCHARACTER>::iterator mapList = f.m_mapVID.begin();

									uint32_t max = UINT_MAX;
									LPCHARACTER pTarget = mapList->second;

									while (mapList != f.m_mapVID.end())
									{
										const uint32_t dist = (uint32_t)DISTANCE_SQRT(GetX() - mapList->second->GetX(), GetY() - mapList->second->GetY());

										if (dist != 0 && max > dist)
										{
											max = dist;
											pTarget = mapList->second;
										}
										mapList++;
									}

									if (pTarget != nullptr)
									{
										int val = 3;

										if (max < 10000)
											val = 2;
										else if (max < 70000)
											val = 1;

										ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (uint32_t)GetVID(), val,
											(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1461]");
									}
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;1461]");
								}

								if (item->GetSocket(0) >= 6)
								{
									ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (uint32_t)GetVID());

									// Remove Item
									item->SetCount(item->GetCount() - 1);
									item->SetSocket(0, item->GetSocket(0) - 6);
									// ITEM_MANAGER::Instance().RemoveItem(item);
								}
							}
							break;
						}
						break;
#endif

						case 27996: // Giftflasche
						{
							item->SetCount(item->GetCount() - 1);
							AttackedByPoison(nullptr); // @warme008
						}
						break;

						case fishing::SHELLFISH_VNUM: // Muschel
						{
							item->SetCount(item->GetCount() - 1);

							const int r = number(1, 100);

							if (r <= 50)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;221]");
								AutoGiveItem(fishing::STONEPIECE_VNUM);
							}
							else
							{
								const int prob_table[] =
								{
									95, 97, 99, /*custom*/ 99, 99, 99
								};

								if (r <= prob_table[0])
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;232]");
								}
								else if (r <= prob_table[1])
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;243]");
									AutoGiveItem(fishing::WHITE_PEARL_VNUM);
								}
								else if (r <= prob_table[2])
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;253]");
									AutoGiveItem(fishing::BLUE_PEARL_VNUM);
								}
								else if (r <= prob_table[3])
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;264]");
									AutoGiveItem(fishing::RED_PEARL_VNUM);
								}
								//@custom021
								else if (r <= prob_table[4])
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHELLFISH_GREEN_PEARL"));
									AutoGiveItem(fishing::GREEN_PEARL_VNUM);
								}
								else if (r <= prob_table[5])
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SHELLFISH_VIOLET_PEARL"));
									AutoGiveItem(fishing::VIOLET_PEARL_VNUM);
								}
								//@end_custom021
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;264]");
									AutoGiveItem(fishing::RED_PEARL_VNUM);
								}
							}
						}
						break;

						case 71013:	// Feuerwerk
						{
							CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), this);
							item->SetCount(item->GetCount() - 1);
						}
						break;

#ifdef ENABLE_YOHARA_SYSTEM
						case 999999:	//SungMa-Premium-Buff
						{
							const uint32_t dwAccID = GetDesc()->GetAccountTable().id;
							db_clientdesc->DBPacket(HEADER_GD_REQUEST_SUNGMA_PREMIUM, GetDesc()->GetHandle(), &dwAccID, sizeof(uint32_t));
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

						case 50100:	// Feuerwerk
						case 50101:	// Feuerwerk
						case 50102:	// Feuerwerk
						case 50103:	// Feuerwerk
						case 50104:	// Feuerwerk
						case 50105:	// Feuerwerk
						case 50106:	// Feuerwerk
						{
							CreateFly(item->GetVnum() - 50100 + FLY_FIREWORK1, this);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50200: // Bündel
						{
#ifdef ENABLE_MYSHOP_DECO
							SetMyShopType(0);
							SetMyShopDecoPolyVnum(0);
#endif
							if (g_bEnableBootaryCheck)
							{
								if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
								{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
									OpenPrivateShop();
#else
									__OpenPrivateShop();
#endif
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot open a private shop on this map."));
								}
							}
							else
							{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
								OpenPrivateShop();
#else
								__OpenPrivateShop();
#endif
							}
						}
						break;

						case fishing::FISH_MIND_PILL_VNUM:	//Fischkugel
						{
#ifdef ENABLE_FISHING_RENEWAL
							if (FindAffect(AFFECT_FISH_MIND_PILL))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;61]");
								return false;
							}
#endif

							AddAffect(AFFECT_FISH_MIND_PILL, POINT_NONE, 0, AFF_FISH_MIND, 20 * 60, 0, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50301: // Sun-zi-Kriegskunst
						case 50302: // Wu-zi-Kriegskunst
						case 50303: // WeiLiao-zi Kriegskunst
						{
							if (IsPolymorphed() == true)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
								return false;
							}

							const int lv = GetSkillLevel(SKILL_LEADERSHIP);

							if (lv < item->GetValue(0))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;274]");
								return false;
							}

							if (lv >= item->GetValue(1))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;284]");
								return false;
							}

							if (LearnSkillByBook(SKILL_LEADERSHIP))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(SKILL_LEADERSHIP, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50304: // Combobeherrschung
						case 50305: // Combomeisterbuch
						case 50306: // Hohe Combokunst
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
								return false;

							}
							if (GetSkillLevel(SKILL_COMBO) == 0 && GetLevel() < 30)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;295]");
								return false;
							}

							if (GetSkillLevel(SKILL_COMBO) == 1 && GetLevel() < 50)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;305]");
								return false;
							}

							if (GetSkillLevel(SKILL_COMBO) >= 2)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;316]");
								return false;
							}

							const uint8_t iPct = static_cast<uint8_t>(item->GetValue(0));

							if (LearnSkillByBook(SKILL_COMBO, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(SKILL_COMBO, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50311: // Shinsoo-Sprachen
						case 50312: // Chunjo-Sprachen
						case 50313: // Jinno-Sprachen
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
								return false;

							}
							const uint32_t dwSkillVnum = item->GetValue(0);
							const uint8_t iPct = MINMAX(0, item->GetValue(1), 100);
							if (GetSkillLevel(dwSkillVnum) >= 20 || dwSkillVnum - SKILL_LANGUAGE1 + 1 == GetEmpire())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;328]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50061: // Handbuch Pferdezähmen
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
								return false;

							}
							const uint32_t dwSkillVnum = item->GetValue(0);
							const uint8_t iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= 10)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50314:	// Buch der Verwandlung
						case 50315:	// Buch fort. Verwandlung
						case 50316:	// Buch exp. Verwandlung
						case 51300:	// Fertigkeitsbuch Regenerationswille (M)
						case 51301:	// Fertigkeitsbuch Regenerationswille (G)
						case 51302:	// Fertigkeitsbuch Regenerationswille (P)
						case 51303:	// Fertigkeitsbuch Durchbruchzerstörung (M)
						case 51304:	// Fertigkeitsbuch Durchbruchzerstörung (G)
						case 51305:	// Fertigkeitsbuch Durchbruchzerstörung (P)
						{
							if (IsPolymorphed() == true)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
								return false;
							}

							const int iSkillLevelLowLimit = item->GetValue(0);
							const int iSkillLevelHighLimit = item->GetValue(1);
							const uint8_t iPct = MINMAX(0, item->GetValue(2), 100);
							const int iLevelLimit = item->GetValue(3);
							uint32_t dwSkillVnum = 0;

							switch (item->GetVnum())
							{
								case 50314:	// Buch der Verwandlung
								case 50315:	// Buch fort. Verwandlung
								case 50316:	// Buch exp. Verwandlung
									dwSkillVnum = SKILL_POLYMORPH;
									break;

								case 51300:	// Fertigkeitsbuch Regenerationswille (M)
								case 51301:	// Fertigkeitsbuch Regenerationswille (G)
								case 51302:	// Fertigkeitsbuch Regenerationswille (P)
									dwSkillVnum = SKILL_ADD_HP;
									break;

								case 51303:	// Fertigkeitsbuch Durchbruchzerstörung (M)
								case 51304:	// Fertigkeitsbuch Durchbruchzerstörung (G)
								case 51305:	// Fertigkeitsbuch Durchbruchzerstörung (P)
									dwSkillVnum = SKILL_RESIST_PENETRATE;
									break;

								default:
									return false;
							}

							if (0 == dwSkillVnum)
								return false;

							if (GetLevel() < iLevelLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;350]");
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;274]");
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;371]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50902:	// Anfängerrezept
						case 50903:	// Rezept
						case 50904:	// Expertenrezept
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
								return false;

							}
							constexpr uint32_t dwSkillVnum = SKILL_CREATE;
							const uint8_t iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);

								if (test_server)
								{
									ChatPacket(CHAT_TYPE_INFO, "Success to learn skill ");
								}
							}
							else
							{
								if (test_server)
								{
									ChatPacket(CHAT_TYPE_INFO, "Failed to learn skill ");
								}
							}
						}
						break;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
						// EXTEND INVENTORY ITEM
						case INVENTORY_OPEN_KEY_GOLD:
						case INVENTORY_OPEN_KEY_SILVER:
						{
							ExtendInvenRequest();
						}
						break;
#endif

						// MINING
						case ITEM_MINING_SKILL_TRAIN_BOOK:	// Bergbau-Buch
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
								return false;

							}
							constexpr uint32_t dwSkillVnum = SKILL_MINING;
							const uint8_t iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;
						// END_OF_MINING

#ifdef ENABLE_REFINE_ABILITY_SKILL	// SKILL_REFINE
						case ITEM_REFINE_ABILITY_SKILL_TRAIN_BOOK:	// Buch des Schmiedens
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
								return false;
							}
							uint32_t dwSkillVnum = SKILL_REFINE;
							int iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= REFINE_SKILL_MAX_LEVEL)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#	ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#	else
								ITEM_MANAGER::Instance().RemoveItem(item);
#	endif

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;
#endif	// END_SKILL_REFINE

						case ITEM_HORSE_SKILL_TRAIN_BOOK:	// Buch der Reitkunst
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1041]");
								return false;
							}

							constexpr uint32_t dwSkillVnum = SKILL_HORSE;
							const uint8_t iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetLevel() < 50)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;376]");
								return false;
							}

							if (!test_server && get_global_time() < GetSkillNextReadTime(dwSkillVnum))
							{
								if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
								{
									// Ignore the time limit while using the magic spell
									RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
									ChatPacket(CHAT_TYPE_INFO, "[LS;377]");
								}
								else
								{
									SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
									return false;
								}
							}

							if (GetPoint(POINT_HORSE_SKILL) >= 20 ||
								GetSkillLevel(SKILL_HORSE_WILDATTACK) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60 ||
								GetSkillLevel(SKILL_HORSE_WILDATTACK_RANGE) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;378]");
								return false;
							}

							if (number(1, 100) <= iPct)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;379]");
								ChatPacket(CHAT_TYPE_INFO, "[LS;380]");
								PointChange(POINT_HORSE_SKILL, 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								if (!test_server)
									SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;382]");
							}
#ifdef ENABLE_BOOKS_STACKFIX
							item->SetCount(item->GetCount() - 1);
#else
							ITEM_MANAGER::Instance().RemoveItem(item);
#endif
						}
						break;

#ifdef ENABLE_PASSIVE_ATTR
						case 50325:	// Buch der Präzision
						case 50338:	// Charisma des Novizen
						case 50339:	// Charisma des Adepten
						case 50340:	// Charisma des Profis
						case 50341:	// Inspiration des Novizen
						case 50342:	// Inspiration des Adepten
						case 50343:	// Inspiration des Profis
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;521]");
								return false;
							}

							const int iSkillLevelLowLimit = item->GetValue(0);
							const int iSkillLevelHighLimit = item->GetValue(1);
							const uint8_t iPct = MINMAX(0, item->GetValue(2), 100);
							const int iNeedExp = item->GetValue(3);
							uint32_t dwSkillVnum = 0;

							switch (item->GetVnum())
							{
								case 50325:	// Buch der Präzision
									dwSkillVnum = SKILL_PRECISION;
									break;

								case 50338:	// Charisma des Novizen
								case 50339:	// Charisma des Adepten
								case 50340:	// Charisma des Profis
									dwSkillVnum = SKILL_CHARISMA;
									break;

								case 50341:	// Inspiration des Novizen
								case 50342:	// Inspiration des Adepten
								case 50343:	// Inspiration des Profis
									dwSkillVnum = SKILL_INSPIRATION;
									break;

								default:
									return false;
							}

							if (0 == dwSkillVnum)
								return false;

							if (dwSkillVnum != SKILL_PRECISION && GetExp() < static_cast<uint32_t>(iNeedExp))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;957]");
								return false;
							}

#	ifdef ENABLE_YOHARA_SYSTEM
							if (dwSkillVnum == SKILL_PRECISION && GetConquerorLevel() < iSkillLevelLowLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;957]");
								return false;
							}
#	endif

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;948]");
								return false;
							}

							if (dwSkillVnum != SKILL_PRECISION && GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;274]");
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;371]");
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
#ifdef ENABLE_BOOKS_STACKFIX
								item->SetCount(item->GetCount() - 1);
#else
								ITEM_MANAGER::Instance().RemoveItem(item);
#endif

								if (dwSkillVnum != SKILL_PRECISION)
									PointChange(POINT_EXP, -iNeedExp);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server)
									iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;
#endif

						case 70102: // Zen-Bohne
						case 70103: // Sutra
						{
							if (GetAlignment() >= 0)
								return false;

							const int delta = MIN(-GetAlignment(), item->GetValue(0));

							sys_log(0, "%s ALIGNMENT ITEM %d", GetName(), delta);

							UpdateAlignment(delta);
							item->SetCount(item->GetCount() - 1);

							if (delta / 10 > 0)
							{
								ChatPacket(CHAT_TYPE_TALKING, "[LS;383]");
								ChatPacket(CHAT_TYPE_INFO, "[LS;384;%d]", delta / 10);
							}
						}
						break;

						case 71107: // Frucht des Lebens
						case 39032:	//@fixme169
						{
							int val = item->GetValue(0);
							const int interval = item->GetValue(1);
							quest::PC* pPC = quest::CQuestManager::Instance().GetPC(GetPlayerID());
							if (!pPC) //@fixme169
								return false;

							int last_use_time = pPC->GetFlag("mythical_peach.last_use_time");

							if (get_global_time() - last_use_time < interval * 60 * 60)
							{
								if (!test_server)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot use this item yet."));
									return false;
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;1034]");
								}
							}

							if (GetAlignment() == 200000)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You already have your rank at the maximum."));
								return false;
							}

							if (200000 - GetAlignment() < val * 10)
							{
								val = (200000 - GetAlignment()) / 10;
							}

							const int old_alignment = GetAlignment() / 10;

							UpdateAlignment(val * 10);

							item->SetCount(item->GetCount() - 1);
							pPC->SetFlag("mythical_peach.last_use_time", get_global_time());

							ChatPacket(CHAT_TYPE_TALKING, "[LS;383]");
							ChatPacket(CHAT_TYPE_INFO, "[LS;384;%d]", val);

							char buf[256 + 1];
							snprintf(buf, sizeof(buf), "%d %d", old_alignment, GetAlignment() / 10);
							LogManager::Instance().CharLog(this, val, "MYTHICAL_PEACH", buf);
						}
						break;

						case 39033: // Nyx : alternative vnum
						case 71109: // Rolle der Korrektur
						//case 72719:	//unused VNUM
						{
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
								return false;

							if (item2->GetSocketCount() == 0)
								return false;

							switch (item2->GetType())
							{
								case ITEM_WEAPON:
									break;

								case ITEM_ARMOR:
								{
									switch (item2->GetSubType())
									{
										case ARMOR_EAR:
										case ARMOR_WRIST:
										case ARMOR_NECK:
											ChatPacket(CHAT_TYPE_INFO, "[LS;1032]");
											return false;

										default:
											break;
									}
								}
								break;

								default:
									return false;
							}

							std::stack<long> socket;

#ifdef ENABLE_PROTO_RENEWAL
							for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
								socket.push(item2->GetSocket(i));

							int idx = METIN_SOCKET_MAX_NUM - 1;
#else
							for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
								socket.push(item2->GetSocket(i));

							int idx = ITEM_SOCKET_MAX_NUM - 1;
#endif

							while (socket.size() > 0)
							{
								if (socket.top() > 2 && socket.top() != ITEM_BROKEN_METIN_VNUM)
									break;

								idx--;
								socket.pop();
							}

							if (socket.size() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1032]");
								return false;
							}

							LPITEM pItemReward = AutoGiveItem(socket.top());

							if (pItemReward != nullptr)
							{
								item2->SetSocket(idx, 1);
#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_GLOVE)
									item2->SetSocket(idx + 3, 1);
#endif

								char buf[256 + 1];
								snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
									item2->GetName(), item2->GetID(), pItemReward->GetName(), pItemReward->GetID());
								LogManager::Instance().ItemLog(this, item, "USE_DETACHMENT_ONE", buf);

								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

						case 70201:	// Bleichmittel
						case 70202:	// Weißes Haarfärbemittel
						case 70203:	// Blondes Haarfärbemittel
						case 70204:	// Rotes Haarfärbemittel
						case 70205:	// Braunes Haarfärbemittel
						case 70206:	// Schwarz Haarfärbemittel
						{
							// NEW_HAIR_STYLE_ADD
							if (GetPart(PART_HAIR) >= 1001)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;385]");
							}
							// END_NEW_HAIR_STYLE_ADD
							else
							{
								quest::CQuestManager& q = quest::CQuestManager::Instance();
								quest::PC* pPC = q.GetPC(GetPlayerID());

								if (pPC)
								{
									int last_dye_level = pPC->GetFlag("dyeing_hair.last_dye_level");

									if (last_dye_level == 0 ||
										last_dye_level + 3 <= GetLevel() ||
										item->GetVnum() == 70201)	// Bleichmittel
									{
										SetPart(PART_HAIR, item->GetVnum() - 70201);	// Bleichmittel

										if (item->GetVnum() == 70201)	// Bleichmittel
											pPC->SetFlag("dyeing_hair.last_dye_level", 0);
										else
											pPC->SetFlag("dyeing_hair.last_dye_level", GetLevel());

										item->SetCount(item->GetCount() - 1);
										UpdatePacket();
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;386;%d]", last_dye_level + 3);
									}
								}
							}
						}
						break;

						case ITEM_NEW_YEAR_GREETING_VNUM:	// Geldtasche
						{
							constexpr uint32_t dwBoxVnum = ITEM_NEW_YEAR_GREETING_VNUM;
							std::vector <uint32_t> dwVnums;
							std::vector <uint32_t> dwCounts;
							std::vector <LPITEM> item_gets;
							int count = 0;

							if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
							{
								for (int i = 0; i < count; i++)
								{
									if (dwVnums[i] == CSpecialItemGroup::GOLD)
									{
#ifdef ENABLE_CHAT_SETTINGS
										ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#else
										ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#endif
									}
								}

								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

						case ITEM_VALENTINE_ROSE:	// Rose
						case ITEM_VALENTINE_CHOCOLATE:	// Schokolade
						{
							const uint32_t dwBoxVnum = item->GetVnum();
							std::vector <uint32_t> dwVnums;
							std::vector <uint32_t> dwCounts;
							std::vector <LPITEM> item_gets(0);
							int count = 0;

							if (((item->GetVnum() == ITEM_VALENTINE_ROSE) && (ESex::SEX_MALE == GET_SEX(this))) ||
								((item->GetVnum() == ITEM_VALENTINE_CHOCOLATE) && (ESex::SEX_FEMALE == GET_SEX(this))))
							{
								// Can't write because gender doesn't match.
								ChatPacket(CHAT_TYPE_INFO, "[LS;387]");
								return false;
							}


							if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_WHITEDAY_CANDY:	// Bonbon
						case ITEM_WHITEDAY_ROSE:	// Rose
						{
							const uint32_t dwBoxVnum = item->GetVnum();
							std::vector <uint32_t> dwVnums;
							std::vector <uint32_t> dwCounts;
							std::vector <LPITEM> item_gets(0);
							int count = 0;

							if (((item->GetVnum() == ITEM_WHITEDAY_CANDY) && (ESex::SEX_MALE == GET_SEX(this))) ||
								((item->GetVnum() == ITEM_WHITEDAY_ROSE) && (ESex::SEX_FEMALE == GET_SEX(this))))
							{
								// Can't write because gender doesn't match.
								ChatPacket(CHAT_TYPE_INFO, "[LS;387]");
								return false;
							}


							if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50011: // Mondlicht-Schatztruhe
						{
							constexpr uint32_t dwBoxVnum = 50011;
							std::vector <uint32_t> dwVnums;
							std::vector <uint32_t> dwCounts;
							std::vector <LPITEM> item_gets(0);
							int count = 0;

							if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
							{
								for (int i = 0; i < count; i++)
								{
									char buf[50 + 1];
									snprintf(buf, sizeof(buf), "%u %u", dwVnums[i], dwCounts[i]);
									LogManager::Instance().ItemLog(this, item, "MOONLIGHT_GET", buf);

									//ITEM_MANAGER::Instance().RemoveItem(item);
									item->SetCount(item->GetCount() - 1);

									switch (dwVnums[i])
									{
										case CSpecialItemGroup::GOLD:
#ifdef ENABLE_CHAT_SETTINGS
											ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#else
											ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#endif
											break;

										case CSpecialItemGroup::EXP:
#ifdef ENABLE_CHAT_SETTINGS
											ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1279]");
											ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1290;%d]", dwCounts[i]);
#else
											ChatPacket(CHAT_TYPE_INFO, "[LS;1279]");
											ChatPacket(CHAT_TYPE_INFO, "[LS;1290;%d]", dwCounts[i]);
#endif
											break;

										case CSpecialItemGroup::MOB:
											ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
											break;

										case CSpecialItemGroup::SLOW:
											ChatPacket(CHAT_TYPE_INFO, "[LS;1310]");
											break;

										case CSpecialItemGroup::DRAIN_HP:
											ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
											break;

										case CSpecialItemGroup::POISON:
											ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
											break;
#ifdef ENABLE_WOLFMAN_CHARACTER
										case CSpecialItemGroup::BLEEDING:
											ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
											break;
#endif
										case CSpecialItemGroup::MOB_GROUP:
											ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
											break;

										default:
											if (item_gets[i])
											{
#ifdef ENABLE_CHAT_SETTINGS
												if (dwCounts[i] > 1)
													ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
												else
													ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetVnum());
#else
												if (dwCounts[i] > 1)
													ChatPacket(CHAT_TYPE_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
												else
													ChatPacket(CHAT_TYPE_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetVnum());
#endif
											}
											break;
									}
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_TALKING, "[LS;56]");
								return false;
							}
						}
						break;

						case ITEM_GIVE_STAT_RESET_COUNT_VNUM:	// Bluttablette
						{
							//PointChange(POINT_GOLD, -iCost);
							PointChange(POINT_STAT_RESET_COUNT, 1);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50107:	// Chinesisches Feuerwerk
						{
							if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
								return false;
							}
#ifdef ENABLE_NEWSTUFF
							else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
								return false;
							}
#endif

							EffectPacket(SE_CHINA_FIREWORK);
#ifdef ENABLE_FIREWORK_STUN
							// Increases stun attack
							AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5 * 60, 0, true);
#endif
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50108:	// Feuerwerkskreisel
						{
							if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
								return false;
							}
#ifdef ENABLE_NEWSTUFF
							else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
								return false;
							}
#endif

							EffectPacket(SE_SPIN_TOP);
#ifdef ENABLE_FIREWORK_STUN
							// Increases stun attack
							AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5 * 60, 0, true);
#endif
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_WONSO_BEAN_VNUM:	// Bohnenkuchen
						{
							PointChange(POINT_HP, GetMaxHP() - GetHP());
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_WONSO_SUGAR_VNUM:	// Zuckerkuchen
						{
							PointChange(POINT_SP, GetMaxSP() - GetSP());
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_WONSO_FRUIT_VNUM:	// Obstkuchen
						{
							PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_ELK_VNUM: // Münzen
						{
							const int iGold = item->GetSocket(0);
							ITEM_MANAGER::Instance().RemoveItem(item);
#ifdef ENABLE_CHAT_SETTINGS
							ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", iGold);
#else
							ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", iGold);
#endif
							PointChange(POINT_GOLD, iGold);
						}
						break;

						case 70021:	// Segen des Engels
						{
							int HealPrice = quest::CQuestManager::Instance().GetEventFlag("MonarchHealGold");
							if (HealPrice == 0)
								HealPrice = 2000000;

							if (CMonarch::Instance().HealMyEmpire(this, HealPrice))
							{
								char szNotice[256];
								snprintf(szNotice, sizeof(szNotice), LC_TEXT("[559]When the Blessing of the Emperor is used %s the HP and SP are restored again."), EMPIRE_NAME(GetEmpire()));
								SendNoticeMap(szNotice, GetMapIndex(), false);

								ChatPacket(CHAT_TYPE_INFO, "[LS;570]");
							}
						}
						break;

						case 27995:	// Leere Flasche
						{
						}
						break;

						case 71092: // Buch der Verwandlung
						{
							if (m_pkChrTarget != nullptr)
							{
								if (m_pkChrTarget->IsPolymorphed())
								{
									m_pkChrTarget->SetPolymorph(0);
									m_pkChrTarget->RemoveAffect(AFFECT_POLYMORPH);
								}
							}
							else
							{
								if (IsPolymorphed())
								{
									SetPolymorph(0);
									RemoveAffect(AFFECT_POLYMORPH);
								}
							}
						}
						break;

						case 71051: //Gegenstand segnen
						{
							// Banned by earthquakes in Europe, Singapore and Vietnam
							if (!attr_rare_enable) {	//@custom026
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("rare attributues have been disabled!"));
								return false;
							}

							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
								return false;

							if (ITEM_COSTUME == item2->GetType()
#ifdef ENABLE_PENDANT
								|| (ITEM_ARMOR == item2->GetType() && ARMOR_PENDANT == item2->GetSubType())
#endif
#ifdef ENABLE_GLOVE_SYSTEM
								|| (ITEM_ARMOR == item2->GetType() && ARMOR_GLOVE == item2->GetSubType())
#endif
								) // @fixme124
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
								return false;

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

#ifdef ENABLE_SEALBIND_SYSTEM
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1101]");
								return false;
							}
#endif

							// @Nyx 20173006
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_APPLY))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if (item2->AddRareAttribute())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;400]");

								const int iAddedIdx = item2->GetRareAttrCount() + 4;
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								LogManager::Instance().ItemLog(
									GetPlayerID(),
									item2->GetAttributeType(iAddedIdx),
									item2->GetAttributeValue(iAddedIdx),
									item->GetID(),
									"ADD_RARE_ATTR",
									buf,
									GetDesc()->GetHostName(),
									item->GetOriginalVnum());

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You can no longer add enhancements to this item."));
							}
						}
						break;

						case 71052: //Gegenstand verhexen
						{
							// Banned by earthquakes in Europe, Singapore and Vietnam
							if (!attr_rare_enable) {	//@custom026
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("rare attributues have been disabled!"));
								return false;
							}

							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (ITEM_COSTUME == item2->GetType()
#ifdef ENABLE_PENDANT
								|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
#endif
								) // @fixme124
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
								return false;

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

#ifdef ENABLE_SEALBIND_SYSTEM
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1101]");
								return false;
							}
#endif

							// @Nyx 20173006
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_APPLY))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if (item2->ChangeRareAttribute())
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::Instance().ItemLog(this, item, "CHANGE_RARE_ATTR", buf);

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
							}
						}
						break;

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
						case ITEM_AURA_BOOST_ITEM_VNUM_BASE + ITEM_AURA_BOOST_ERASER:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging() || item2->IsEquipped())
								return false;

#ifdef ENABLE_SEALBIND_SYSTEM
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1468]");
								return false;
							}
#endif

							if (item2->GetSocket(ITEM_SOCKET_AURA_BOOST) == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Aura] There is no boost in your aura costume."));
								return false;
							}

							if (IS_SET(item->GetFlag(), ITEM_FLAG_STACKABLE) && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK) && item->GetCount() > 1)
								item->SetCount(item->GetCount() - 1);
							else
								ITEM_MANAGER::Instance().RemoveItem(item);

							item2->SetSocket(ITEM_SOCKET_AURA_BOOST, 0);
						}
						break;
#endif

						case ITEM_AUTO_HP_RECOVERY_S:	// Elixier der Sonne (K)
						case ITEM_AUTO_HP_RECOVERY_M:	// Elixier der Sonne (M)
						case ITEM_AUTO_HP_RECOVERY_L:	// Elixier der Sonne (G)
						case ITEM_AUTO_HP_RECOVERY_X:	// Elixier der Sonne (S)
						case ITEM_AUTO_SP_RECOVERY_S:	// Elixier des Mondes (K)
						case ITEM_AUTO_SP_RECOVERY_M:	// Elixier des Mondes (M)
						case ITEM_AUTO_SP_RECOVERY_L:	// Elixier des Mondes (G)
						case ITEM_AUTO_SP_RECOVERY_X:	// Elixier des Mondes (S)
						// Horrible, but scary to fix what you did before...
						// So just hard coded. Automatic potion items for gift boxes.
						case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:	// Elixier des Mondes (K)
						case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:	// Elixier des Mondes (M)
						case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:	// Elixier der Sonne (M)
						case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:	// Elixier der Sonne (K)
						{
							if (CArenaManager::Instance().IsArenaMap(GetMapIndex()))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
								return false;
							}
#ifdef ENABLE_NEWSTUFF
							else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
								return false;
							}
#endif

							EAffectTypes type = AFFECT_NONE;
							bool isSpecialPotion = false;

							switch (item->GetVnum())
							{
								case ITEM_AUTO_HP_RECOVERY_X:
									isSpecialPotion = true;

								case ITEM_AUTO_HP_RECOVERY_S:
								case ITEM_AUTO_HP_RECOVERY_M:
								case ITEM_AUTO_HP_RECOVERY_L:
								case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
								case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
									type = AFFECT_AUTO_HP_RECOVERY;
									break;

								case ITEM_AUTO_SP_RECOVERY_X:
									isSpecialPotion = true;

								case ITEM_AUTO_SP_RECOVERY_S:
								case ITEM_AUTO_SP_RECOVERY_M:
								case ITEM_AUTO_SP_RECOVERY_L:
								case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
								case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
									type = AFFECT_AUTO_SP_RECOVERY;
									break;

								default:
									break;
							}

							if (AFFECT_NONE == type)
								break;

							if (item->GetCount() > 1)
							{
#ifdef ENABLE_SPECIAL_INVENTORY
								const int pos = GetEmptyInventory(item);
#else
								const int pos = GetEmptyInventory(item->GetSize());
#endif

								if (-1 == pos)
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
									break;
								}

								item->SetCount(item->GetCount() - 1);

								LPITEM item2 = ITEM_MANAGER::Instance().CreateItem(item->GetVnum(), 1);
								item2->AddToCharacter(this, TItemPos(INVENTORY, pos));

								if (item->GetSocket(1) != 0)
								{
									item2->SetSocket(1, item->GetSocket(1));

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
									CAchievementSystem::Instance().OnToggle(this, item->GetVnum());
#endif
								}

								item = item2;
							}

							CAffect* pAffect = FindAffect(type);

							if (nullptr == pAffect)
							{
								EPointTypes bonus = POINT_NONE;

								if (isSpecialPotion)
								{
									if (type == AFFECT_AUTO_HP_RECOVERY)
									{
										bonus = POINT_MAX_HP_PCT;
									}
									else if (type == AFFECT_AUTO_SP_RECOVERY)
									{
										bonus = POINT_MAX_SP_PCT;
									}
								}

								AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

								//@custom039
								if (type == AFFECT_AUTO_HP_RECOVERY) {
									EffectPacket(SE_AUTO_HPUP);
									//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Load type red: %d"), type);

								}
								else if (type == AFFECT_AUTO_SP_RECOVERY) {
									EffectPacket(SE_AUTO_SPUP);
									//ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Load type blue: %d"), type);
								}
								//@end_custom039

								item->Lock(true);
								item->SetSocket(0, true);

								AutoRecoveryItemProcess(type);
							}
							else
							{
								if (item->GetID() == pAffect->dwFlag)
								{
									RemoveAffect(pAffect);

									item->Lock(false);
									item->SetSocket(0, false);
								}
								else
								{
									LPITEM old = FindItemByID(pAffect->dwFlag);

									if (nullptr != old)
									{
										old->Lock(false);
										old->SetSocket(0, false);
									}

									RemoveAffect(pAffect);

									EPointTypes bonus = POINT_NONE;

									if (isSpecialPotion)
									{
										if (type == AFFECT_AUTO_HP_RECOVERY)
										{
											bonus = POINT_MAX_HP_PCT;
										}
										else if (type == AFFECT_AUTO_SP_RECOVERY)
										{
											bonus = POINT_MAX_SP_PCT;
										}
									}

									AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

									item->Lock(true);
									item->SetSocket(0, true);

									AutoRecoveryItemProcess(type);
								}
							}
						}
						break;

#ifdef ENABLE_ANTI_EXP_RING
						case ITEM_AUTO_EXP_RING_S:	// Elixier der Erfahrung (K)
						case ITEM_AUTO_EXP_RING_M:	// Elixier der Erfahrung (M)
						case ITEM_AUTO_EXP_RING_G:	// Elixier der Erfahrung (G)
						{
							EAffectTypes type = AFFECT_EXPRING;
							int bonus = 0;
							if (item->GetVnum() == ITEM_AUTO_EXP_RING_S){ bonus = 150; }
							else if (item->GetVnum() == ITEM_AUTO_EXP_RING_M){ bonus = 151; }
							else if (item->GetVnum() == ITEM_AUTO_EXP_RING_G){ bonus = 152; }

							if (item->GetCount() > 1) {
#ifdef ENABLE_SPECIAL_INVENTORY
								int pos = GetEmptyInventory(item);
#else
								int pos = GetEmptyInventory(item->GetSize());
#endif

								if (-1 == pos) {
									ChatPacket(CHAT_TYPE_INFO, "[LS;1130]");
									break;
								}

								item->SetCount(item->GetCount() - 1);

								LPITEM item2 = ITEM_MANAGER::Instance().CreateItem(item->GetVnum(), 1);
								item2->AddToCharacter(this, TItemPos(INVENTORY, pos));

								if (item->GetSocket(1) != 0)
									item2->SetSocket(1, item->GetSocket(1));

								item = item2;
							}

							CAffect* pAffect = FindAffect(AFFECT_EXPRING);
							if (nullptr == pAffect)
							{
								AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);
								if (type == AFFECT_EXPRING) {
									EffectPacket(SE_AUTO_EXP);
								}
								//EffectPacket(SE_AUTO_EXP);	//@fixme507
								item->Lock(true);
								item->SetSocket(0, true);
								//AutoRecoveryItemProcess(type); //test
							}
							else
							{
								CAffect* pSearchAffect = FindAffect(type);
								if (type == AFFECT_EXPRING && item->GetID() == pAffect->dwFlag && pSearchAffect->wApplyOn == pAffect->wApplyOn) {
									RemoveAffect(pAffect);
				
									item->Lock(false);
									item->SetSocket(0, false);
								}
								else {
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("disable the other exp ring"));
								}
							}
						}
						break;
#endif

#ifdef ENABLE_GLOVE_SYSTEM
						case 72352:
						{
							if (GetQuestFlag("glove_system.can_equip_glove") != 0)
								return false;

							SetQuestFlag("glove_system.can_equip_glove", 1);
							ChatPacket(CHAT_TYPE_INFO, "[LS;1676]");
							ITEM_MANAGER::Instance().RemoveItem(item);
							return true;
						}
						break;
#endif


#ifdef ENABLE_12ZI
						case 72327: // Sphere Parchment (Trade)
						case 72329: // Sphere Parchment
							{
								if (GetOpenedWindow(W_EXCHANGE | W_SAFEBOX | W_SHOP_OWNER | W_CUBE))
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can not use this while you have an open window."));
									return false;
								}

								SetAnimaSphere(30);
								item->SetCount(item->GetCount() - 1);
							}
							break;

						case 72328: // Mark of the Guardian
							{
								if (FindAffect(AFFECT_CZ_UNLIMIT_ENTER))
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
									return false;
								}

								const long lDuration = item->GetValue(0) * 60;
								AddAffect(AFFECT_CZ_UNLIMIT_ENTER, 0, 0, AFF_CZ_UNLIMIT_ENTER, lDuration, 0, true, false, 0, get_global_time() + lDuration);
								item->SetCount(item->GetCount() - 1);
							}
							break;
#endif

#ifdef ENABLE_SET_ITEM
						case ITEM_DELETE_SET_EFFECT:	//Set-Effekt löschen
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item->IsExchanging() || item2->IsExchanging() || item2->IsEquipped())
								return false;

							if (item->isLocked() || item2->isLocked())
								return false;

							item2->SetItemSetValue(0);
							item2->UpdatePacket();
							item->SetCount(item->GetCount() - 1);
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You've deleted your SetEffect!"));
						}
						break;
#endif

#ifdef ENABLE_EASTER_EVENT
						case MAGIC_EGG_VNUM:
						{
							if (quest::CQuestManager::Instance().GetEventFlag("enable_easter_event") == 0)
								return false;

							if (get_global_time() < item->GetSocket(1))
							{
								const long secondsLeft = item->GetSocket(1) - get_global_time();
								ChatPacket(CHAT_TYPE_INFO, "[LS;1506;%d]", (secondsLeft / 60));
								return false;
							}

							const uint32_t dwBoxVnum = item->GetVnum();
							std::vector <uint32_t> dwVnums;
							std::vector <uint32_t> dwCounts;
							std::vector <LPITEM> item_gets(0);
							int count = 0;

#ifdef ENABLE_BLOCK_CHEST_INVENTORY_FULL	//@fixme489
							int iEmptyCell = -1;
							if (item->IsDragonSoul())
							{
								if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
									return false;
								}
							}
							else
							{
#	ifdef ENABLE_SPECIAL_INVENTORY
								if ((iEmptyCell = GetEmptyInventory(item)) == -1)
#	else
								if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
#	endif
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
									return false;
								}
							}
#endif	//@end_fixme489

							if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
							{
								for (int i = 0; i < count; i++)
								{
									switch (dwVnums[i])
									{
									case CSpecialItemGroup::GOLD:
#ifdef ENABLE_CHAT_SETTINGS
										ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#else
										ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#endif
										break;
									case CSpecialItemGroup::EXP:
#ifdef ENABLE_CHAT_SETTINGS
										ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1279]");
										ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1290;%d]", dwCounts[i]);
#else
										ChatPacket(CHAT_TYPE_INFO, "[LS;1279]");
										ChatPacket(CHAT_TYPE_INFO, "[LS;1290;%d]", dwCounts[i]);
#endif
										break;
									case CSpecialItemGroup::MOB:
										ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
										break;
									case CSpecialItemGroup::SLOW:
										ChatPacket(CHAT_TYPE_INFO, "[LS;1310]");
										break;
									case CSpecialItemGroup::DRAIN_HP:
										ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
										break;
									case CSpecialItemGroup::POISON:
										ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
										break;
#	ifdef ENABLE_WOLFMAN_CHARACTER
									case CSpecialItemGroup::BLEEDING:
										ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
										break;
#	endif
									case CSpecialItemGroup::MOB_GROUP:
										ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
										break;
									default:
										if (item_gets[i])
										{
#ifdef ENABLE_CHAT_SETTINGS
											if (dwCounts[i] > 1)
												ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetVnum(), dwCounts[i]);
											else
												ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetVnum());
#else
											if (dwCounts[i] > 1)
												ChatPacket(CHAT_TYPE_INFO, "[LS;204;[IN;%d];%d]", item_gets[i]->GetName(), dwCounts[i]);
											else
												ChatPacket(CHAT_TYPE_INFO, "[LS;35;[IN;%d]]", item_gets[i]->GetName());
#endif
										}
									}
								}

								if (item->GetSocket(0) > 1)
								{
									item->SetSocket(0, item->GetSocket(0) - 1);
									item->SetSocket(1, get_global_time() + MAGIC_EGG_TIME_TO_WAIT);
									ChatPacket(CHAT_TYPE_INFO, "[LS;1505;%d;%d]", MAGIC_EGG_TOTAL_UNPACK, item->GetSocket(0));
								}
								else
								{
									ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (EASTER MAGIC_EGG)");
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_TALKING, "[LS;56]");
								return false;
							}
						}
						break;
#endif

						default:
							break;
					}
				}
				break;

				case USE_CLEAR:
				{
					switch (item->GetVnum())
					{
#ifdef ENABLE_WOLFMAN_CHARACTER
						case REWARD_UNIQUE_BANDAGE: // Verband
							RemoveBleeding();
							break;
#endif
						case 27874: // Gegrillter Barsch
							RemoveBadAffect();	//@custom027
							break;
					}
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_INVISIBILITY:
				{
					if (item->GetVnum() == 70026 || item->GetVnum() == 72305)	// Unsichtbarkeitsspiegel
					{
						quest::CQuestManager& q = quest::CQuestManager::Instance();
						quest::PC* pPC = q.GetPC(GetPlayerID());

						if (pPC != nullptr)
						{
							int last_use_time = pPC->GetFlag("mirror_of_disapper.last_use_time");

							if (get_global_time() - last_use_time < 10 * 60)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot use this item yet."));
								return false;
							}

							pPC->SetFlag("mirror_of_disapper.last_use_time", get_global_time());
						}
					}

					AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, 300, 0, true);
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_POTION_NODELAY:
#ifdef ENABLE_SUNG_MAHI_TOWER
				case USE_POTION_NODELAY_TOWER:
#endif
				{
#ifdef ENABLE_SUNG_MAHI_TOWER
					if (!IS_SUNG_MAHI_ENABLE_ITEM(this, GetMapIndex(), item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, "You cannot use this type of potion here.");
						return false;
					}
#endif

					if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
					{
						if (quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit") > 0)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
							return false;
						}

						switch (item->GetVnum())
						{
							case 39003: // Nyx : added 39003
							case 39011: // Nyx : added 39011
							case 39012: // Nyx : added 39012
							case 39013: // Nyx : added 39013
							case 70020:	// Pfirsichblütenwein
							case 71018:	// Segen des Lebens
							case 71019:	// Segen der Magie
							case 71020:	// Segen des Drachen
							case 72311: // Nyx : added 72311
								if (quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit_count") < 10000)
								{
									if (m_nPotionLimit <= 0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;122]");
										return false;
									}
								}
								break;

							default:
								ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
								return false;
						}
					}
#ifdef ENABLE_NEWSTUFF
					else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
						return false;
					}
#endif

					bool used = false;

					if (item->GetValue(0) != 0) // Absolute HP recovery
					{
						if (GetHP() < GetMaxHP())
						{
							PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
							EffectPacket(SE_HPUP_RED);
							used = TRUE;
						}
					}

					if (item->GetValue(1) != 0) // SP Absolute Value Recovery
					{
						if (GetSP() < GetMaxSP())
						{
							PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
							EffectPacket(SE_SPUP_BLUE);
							used = TRUE;
						}
					}

					if (item->GetValue(3) != 0) // HP % recovery
					{
						if (GetHP() < GetMaxHP())
						{
							PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
							EffectPacket(SE_HPUP_RED);
							used = TRUE;
						}
					}

					if (item->GetValue(4) != 0) // SP % recovery
					{
						if (GetSP() < GetMaxSP())
						{
							PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
							EffectPacket(SE_SPUP_BLUE);
							used = TRUE;
						}
					}

					if (used)
					{
						if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
						{
							if (test_server)
								ChatPacket(CHAT_TYPE_INFO, "Benutzter Mondkuchen oder Samen.");

							SetUseSeedOrMoonBottleTime();
						}
						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						m_nPotionLimit--;

						//RESTRICT_USE_SEED_OR_MOONBOTTLE
						item->SetCount(item->GetCount() - 1);
						//END_RESTRICT_USE_SEED_OR_MOONBOTTLE
					}
				}
				break;

				case USE_POTION:
#ifdef ENABLE_SUNG_MAHI_TOWER
				case USE_POTION_TOWER:
#endif
				{
#ifdef ENABLE_SUNG_MAHI_TOWER
					if (!IS_SUNG_MAHI_ENABLE_ITEM(this, GetMapIndex(), item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, "You cannot use this type of potion here.");
						return false;
					}
#endif

					if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
					{
						if (quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit") > 0)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
							return false;
						}

						switch (item->GetVnum())
						{
							case 27001:	// Roter Trank (K)
							case 27002:	// Roter Trank (M)
							case 27003:	// Roter Trank (G)
							case 27004:	// Blauer Trank (K)
							case 27005:	// Blauer Trank (M)
							case 27006:	// Blauer Trank (G)
								if (quest::CQuestManager::Instance().GetEventFlag("arena_potion_limit_count") < 10000)
								{
									if (m_nPotionLimit <= 0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;122]");
										return false;
									}
								}
								break;

							default:
								ChatPacket(CHAT_TYPE_INFO, "[LS;403]");
								return false;
						}
					}
#ifdef ENABLE_NEWSTUFF
					else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
						return false;
					}
#endif

					if (item->GetValue(1) != 0)
					{
						if (GetPoint(POINT_SP_RECOVERY) + GetSP() >= GetMaxSP())
						{
							return false;
						}

						PointChange(POINT_SP_RECOVERY, item->GetValue(1) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
						StartAffectEvent();
						EffectPacket(SE_SPUP_BLUE);
					}

					if (item->GetValue(0) != 0)
					{
						if (GetPoint(POINT_HP_RECOVERY) + GetHP() >= GetMaxHP())
						{
							return false;
						}

						PointChange(POINT_HP_RECOVERY, item->GetValue(0) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
						StartAffectEvent();
						EffectPacket(SE_HPUP_RED);
					}

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
					m_nPotionLimit--;
				}
				break;

				case USE_POTION_CONTINUE:
				{
					if (item->GetValue(0) != 0)
						AddAffect(AFFECT_HP_RECOVER_CONTINUE, POINT_HP_RECOVER_CONTINUE, item->GetValue(0), 0, item->GetValue(2), 0, true);
					else if (item->GetValue(1) != 0)
						AddAffect(AFFECT_SP_RECOVER_CONTINUE, POINT_SP_RECOVER_CONTINUE, item->GetValue(1), 0, item->GetValue(2), 0, true);
					else
						return false;

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_ABILITY_UP:
				{
					if (FindAffect(AFFECT_BLEND, static_cast<uint16_t>(item->GetValue(0)))) {	//@custom032
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_WATER_AND_BLEND_ITEM_TOGETHER"));
						return false;
					}

					int afftype = aApplyInfo[item->GetValue(0)].wPointType; //@fixme404
					switch (item->GetValue(0))
					{
						case APPLY_MOV_SPEED:
						{
							VERIFY_POTION(AFFECT_MOV_SPEED, afftype); //@fixme404
							AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef ENABLE_EFFECT_EXTRAPOT
							EffectPacket(SE_DXUP_PURPLE);
#endif
						}
						break;

						case APPLY_ATT_SPEED:
						{
							VERIFY_POTION(AFFECT_ATT_SPEED, afftype); //@fixme404
							AddAffect(AFFECT_ATT_SPEED, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef ENABLE_EFFECT_EXTRAPOT
							EffectPacket(SE_SPEEDUP_GREEN);
#endif
						}
						break;

						case APPLY_STR:
						{
							VERIFY_POTION(AFFECT_STR, afftype); //@fixme404
							AddAffect(AFFECT_STR, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_DEX:
						{
							VERIFY_POTION(AFFECT_DEX, afftype); //@fixme404
							AddAffect(AFFECT_DEX, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_CON:
						{
							VERIFY_POTION(AFFECT_CON, afftype); //@fixme404
							AddAffect(AFFECT_CON, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_INT:
						{
							VERIFY_POTION(AFFECT_INT, afftype); //@fixme404
							AddAffect(AFFECT_INT, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_CAST_SPEED:
						{
							VERIFY_POTION(AFFECT_CAST_SPEED, afftype); //@fixme404
							AddAffect(AFFECT_CAST_SPEED, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_ATT_GRADE_BONUS:
						{
							VERIFY_POTION(AFFECT_ATT_GRADE, afftype); //@fixme404
							AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_RESIST_MAGIC:
						{
							VERIFY_POTION(APPLY_RESIST_MAGIC, afftype); //@fixme404
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_DEF_GRADE_BONUS:
						{
							VERIFY_POTION(AFFECT_DEF_GRADE, afftype); //@fixme404
							AddAffect(AFFECT_DEF_GRADE, POINT_DEF_GRADE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

#ifdef ENABLE_YOHARA_SYSTEM
						case APPLY_SUNGMA_STR:
						{
							VERIFY_POTION(APPLY_SUNGMA_STR, afftype); //@fixme404
							AddAffect(AFFECT_SUNGMA_STR, POINT_SUNGMA_STR, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_SUNGMA_HP:
						{
							VERIFY_POTION(APPLY_SUNGMA_HP, afftype); //@fixme404
							AddAffect(AFFECT_SUNGMA_HP, POINT_SUNGMA_HP, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_SUNGMA_MOVE:
						{
							VERIFY_POTION(APPLY_SUNGMA_MOVE, afftype); //@fixme404
							AddAffect(AFFECT_SUNGMA_MOVE, POINT_SUNGMA_MOVE, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_SUNGMA_IMMUNE:
						{
							VERIFY_POTION(APPLY_SUNGMA_IMMUNE, afftype); //@fixme404
							AddAffect(AFFECT_SUNGMA_IMMUNE, POINT_SUNGMA_IMMUNE, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;
#endif

						default:
							break;
					}

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

#ifdef ENABLE_BATTLE_FIELD
					if (item->GetVnum() == 27125)
					{
						if (FindAffect(AFFECT_BATTLE_POTION))
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
							return false;
						}

						if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
						{
							int iMultiply = 0;
							if (GetLevel() <= 64)
							{
								iMultiply = 4;
							}
							else if (GetLevel() > 64 && GetLevel() < 90)
							{
								iMultiply = 3;
							}
							else if (GetLevel() >= 90)
							{
								iMultiply = 2;
							}

							const int iAttackDefenceValue = (gPlayerMaxLevel - (GetLevel() - 50)) * iMultiply;

							//AddAffect(AFFECT_BATTLE_POTION, POINT_MAX_HP, 50000, 0, INFINITE_AFFECT_DURATION, 0, false);
							AddAffect(AFFECT_BATTLE_POTION, POINT_MAX_HP, 15000, 0, INFINITE_AFFECT_DURATION, 0, false);
							AddAffect(AFFECT_BATTLE_POTION, POINT_ATT_GRADE_BONUS, iAttackDefenceValue, 0, INFINITE_AFFECT_DURATION, 0, false);
							AddAffect(AFFECT_BATTLE_POTION, POINT_DEF_GRADE_BONUS, iAttackDefenceValue, 0, INFINITE_AFFECT_DURATION, 0, false);

							EffectPacket(SE_EFFECT_BATTLE_POTION);
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;118]");
							return false;
						}
					}
#endif

					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_TALISMAN:
				{
					const int TOWN_PORTAL = 1;
					const int MEMORY_PORTAL = 2;

					switch (GetMapIndex())
					{
						// gm_guild_build, disables the homing memory in the oxevent map
						case MAP_GM_GUILD:
						case 113:
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;388]");
							return false;
						}
						default:
							break;
					}

					if (CArenaManager::Instance().IsArenaMap(GetMapIndex()) == true)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
						return false;
					}
#ifdef ENABLE_NEWSTUFF
					else if (g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1205]");
						return false;
					}
#endif

					if (m_pkWarpEvent)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;389]");
						return false;
					}

					// CONSUME_LIFE_WHEN_USE_WARP_ITEM
					const int consumeLife = CalculateConsume(this);

					if (consumeLife < 0)
						return false;
					// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

					if (item->GetValue(0) == TOWN_PORTAL) // return
					{
						if (item->GetSocket(0) == 0)
						{
							if (!GetDungeon())
							{
								if (!GiveRecallItem(item))
									return false;
							}

							PIXEL_POSITION posWarp;

							if (SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp))
							{
								// CONSUME_LIFE_WHEN_USE_WARP_ITEM
								PointChange(POINT_HP, -consumeLife, false);
								// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

								WarpSet(posWarp.x, posWarp.y);
							}
							else
							{
								sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", GetName(), GetX(), GetY());
							}
						}
						else
						{
							if (test_server)
								ChatPacket(CHAT_TYPE_INFO, "Du wirst zum ursprünglichen Ort zurückgebracht.");

							ProcessRecallItem(item);
						}
					}
					else if (item->GetValue(0) == MEMORY_PORTAL) // return memory
					{
						if (item->GetSocket(0) == 0)
						{
							if (GetDungeon())
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;391;%s;%s]", item->GetName(), "");
								return false;
							}

							if (!GiveRecallItem(item))
								return false;
						}
						else
						{
							// CONSUME_LIFE_WHEN_USE_WARP_ITEM
							PointChange(POINT_HP, -consumeLife, false);
							// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

							ProcessRecallItem(item);
						}
					}
				}
				break;

				case USE_TUNING:
				case USE_DETACHMENT:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
						return false;

#ifdef ENABLE_SEALBIND_SYSTEM
					if (item2->IsSealed())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1521]");
						return false;
					}
#endif

					RefineItem(item, item2);
				}
				break;

#ifdef ENABLE_USE_COSTUME_ATTR
				case USE_CHANGE_COSTUME_ATTR:
				case USE_RESET_COSTUME_ATTR:
				{
					LPITEM item2;
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsEquipped())
					{
						BuffOnAttr_RemoveBuffsFromItem(item2);
					}

					if (ITEM_COSTUME != item2->GetType())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

					if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_ENCHANT))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

#ifdef ENABLE_SEALBIND_SYSTEM
					if (item2->IsSealed())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1103]");
						return false;
					}
#endif

					if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
						return false;

#ifdef ENABLE_ATTRIBUTE_CHANGE_ON_START_ITEM
					if (item2->IsBasicItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
#endif

					if (item2->GetAttributeSetIndex() == -1/* || item2->GetMagicPct() == 0*/)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

					if (item2->GetAttributeCount() == 0)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
						return false;
					}

					switch (item->GetSubType())
					{
						case USE_CHANGE_COSTUME_ATTR:
						{
							if (item2->GetAttributeCount() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
								return false;
							}

							item2->ChangeAttribute();
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::Instance().ItemLog(this, item, "CHANGE_COSTUME_ATTR", buf);
							}
						}
						break;

						case USE_RESET_COSTUME_ATTR:
						{
							item2->ClearAttribute();
							item2->AlterToMagicItem();
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::Instance().ItemLog(this, item, "RESET_COSTUME_ATTR", buf);
							}
						}
						break;

						default:
							break;
					}

					ChatPacket(CHAT_TYPE_INFO, "[LS;399]");

					item->SetCount(item->GetCount() - 1);
				}
				break;
#endif

#ifdef ENABLE_REFINE_ELEMENT
				case USE_ELEMENT_DOWNGRADE:
				case USE_ELEMENT_UPGRADE:
				case USE_ELEMENT_CHANGE:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsExchanging() || item2->IsEquipped())
						return false;

#	ifdef ENABLE_SEALBIND_SYSTEM
					if (item2->IsSealed() && !item->IsSocketModifyingItem())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1101]");
						return false;
					}
#	endif

					ElementsSpellItem(item, item2);
				}
				break;
#endif

#ifdef ENABLE_SEALBIND_SYSTEM	// Item Sealing
				case USE_BIND:	// Pergament der Bindung
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsEquipped() || item2->IsExchanging())
						return false;

					if (!item2->IsSealAble())
					{
						if (item2->GetSubType() != USE_BIND)
							ChatPacket(CHAT_TYPE_INFO, "[LS;1106]");
						return false;
					}

#ifdef ENABLE_GIVE_BASIC_ITEM
					if (item2->IsBasicItem()) {
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
#endif

					// sealed item
					if (item2->GetSealDate() == -1) {
						ChatPacket(CHAT_TYPE_INFO, "[LS;1109]");
						return false;
					}

					// unsealmode
					/*if (item2->GetSealDate() > time(0)) {
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_IN_UNSEAL_MODE"));
						return false;
					}*/

					item2->SetSealDate(-1);

					TPacketGCSeal pack;
					pack.header = HEADER_GC_SEAL;
					pack.pos = DestCell;
					pack.action = 0;
					pack.nSealDate = -1;

					GetDesc()->Packet(&pack, sizeof(pack));

					ITEM_MANAGER::Instance().SaveSingleItem(item2);

					item->SetCount(item->GetCount() - 1);

					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have bind succesfully this item."));
				}
				break;

				case USE_UNBIND:	// Pergament der Lösung
				{
					LPITEM item2;
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->isLocked() || item2->IsEquipped())
						return false;

					if (!item2->IsSealAble())
						return false;

#ifdef ENABLE_GIVE_BASIC_ITEM
					if (item2->IsBasicItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
#endif

					if (item2->GetSealDate() == 0 || item2->GetSealDate() > time(0))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1111]");
						return false;
					}

					if (item2->GetSealDate() == -1)
					{
						long sealTime = 0;

						if (test_server)
							sealTime = get_global_time() + 60;
						else
							sealTime = get_global_time() + (60 * 60 * SOULUNBIND_TIME);

						item2->SetSealDate(sealTime);

						item->SetCount(item->GetCount() - 1);

						ITEM_MANAGER::Instance().SaveSingleItem(item2);

						TPacketGCSeal pack;
						pack.header = HEADER_GC_SEAL;
						pack.pos = DestCell;
						pack.action = 1;
						pack.nSealDate = sealTime;
						GetDesc()->Packet(&pack, sizeof(pack));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1110]");
						return false;
					}
				}
				break;
#endif		// Item Sealing End

				case USE_AGGREGATE_MONSTER:
				{
					AggregateMonster();
					item->SetCount(item->GetCount() - 1);
				}
				break;


				//  ACCESSORY_REFINE & ADD/CHANGE_ATTRIBUTES
				case USE_PUT_INTO_BELT_SOCKET:
				case USE_PUT_INTO_RING_SOCKET:
				case USE_PUT_INTO_ACCESSORY_SOCKET:
				case USE_ADD_ACCESSORY_SOCKET:
				case USE_CLEAN_SOCKET:
				case USE_CHANGE_ATTRIBUTE:
				case USE_CHANGE_ATTRIBUTE2:
				case USE_ADD_ATTRIBUTE:
				case USE_ADD_ATTRIBUTE2:
#ifdef ENABLE_AURA_SYSTEM
				case USE_PUT_INTO_AURA_SOCKET:
#endif
				{
					LPITEM item2;
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item->GetVnum() == item2->GetVnum())	// Switcher/adder stackable
					{
						int d = item->GetCount();
						if ((item->GetCount() + item2->GetCount()) > 200)
							d = 200 - item2->GetCount();
						item2->SetCount(item2->GetCount() + d);
						item->SetCount(item->GetCount() - d);
						return false;
					}

					if (item2->IsEquipped())
					{
						BuffOnAttr_RemoveBuffsFromItem(item2);
					}

#ifdef ENABLE_SEALBIND_SYSTEM
					if (item2->IsSealed() && !item->IsSocketModifyingItem())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1101]");
						return false;
					}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
					if (item2->IsBasicItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
						return false;
					}
#endif

#ifdef ENABLE_ATTRIBUTE_CHANGE_ON_START_ITEM
					if (StartWeaponBlockAttribute(item2->GetVnum())) {	//@custom007
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Nicht moeglich. USE_ADD_ATTRIBUTE2"));
						return false;
					}
#endif

					if (item2->IsOldHair() && item->IsCostumeModifyItem()) {	//@fixme445
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CHANGE_ATTR_CANNOT_CHANGE_THIS_ITEM"));
						return false;
					}


					// [NOTE] There was a request to give a costume item a random attribute when the item is first created, but to block financial resources, etc.
					// Originally, an item flag such as ANTI_CHANGE_ATTRIBUTE was added to allow flexible control at the planning level.
					// I don't need that, so shut up and ask me to do it quickly, so I just stop here... -_-
					if (ITEM_COSTUME == item2->GetType())
#ifdef ENABLE_CHANGED_ATTR
					if (item->GetSubType() != USE_SELECT_ATTRIBUTE)
#endif
#ifdef ENABLE_AURA_SYSTEM
					if (item->GetSubType() != USE_PUT_INTO_AURA_SOCKET)
#endif
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

					if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
						return false;

					switch (item->GetSubType())
					{
						case USE_CLEAN_SOCKET:
						{
							int i;
#ifdef ENABLE_PROTO_RENEWAL
							for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
							for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
							{
								if (item2->GetSocket(i) == ITEM_BROKEN_METIN_VNUM)
									break;
							}

#ifdef ENABLE_PROTO_RENEWAL
							if (i == METIN_SOCKET_MAX_NUM)
#else
							if (i == ITEM_SOCKET_MAX_NUM)
#endif
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;395]");
								return false;
							}

							int j = 0;

#ifdef ENABLE_PROTO_RENEWAL
							for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
							for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
							{
								if (item2->GetSocket(i) != ITEM_BROKEN_METIN_VNUM && item2->GetSocket(i) != 0)
									item2->SetSocket(j++, item2->GetSocket(i));
							}

#ifdef ENABLE_PROTO_RENEWAL
							for (; j < METIN_SOCKET_MAX_NUM; ++j)
#else
							for (; j < ITEM_SOCKET_MAX_NUM; ++j)
#endif
							{
								if (item2->GetSocket(j) > 0)
									item2->SetSocket(j, 1);
							}

							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::Instance().ItemLog(this, item, "CLEAN_SOCKET", buf);
							}

#if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
							if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_GLOVE)
							{
								int s;
								for (s = 3; s < ITEM_SOCKET_MAX_NUM; ++s)
								{
									if (item2->GetSocket(s) == ITEM_BROKEN_METIN_VNUM)
										break;
								}

								if (s == ITEM_SOCKET_MAX_NUM)
									return false;

								int k = 3;

								for (s = 3; s < ITEM_SOCKET_MAX_NUM; ++s)
								{
									if (item2->GetSocket(s) != ITEM_BROKEN_METIN_VNUM && item2->GetSocket(s) != 0)
										item2->SetSocket(k++, item2->GetSocket(s));
								}

								for (; k < ITEM_SOCKET_MAX_NUM; ++k)
								{
									if (item2->GetSocket(k) > 0)
										item2->SetSocket(k, 1);
								}

								{
									char buf[50];
									snprintf(buf, sizeof(buf), "%u", item2->GetID());
									LogManager::Instance().ItemLog(this, item, "CLEAN_SOCKET_RANDOM", buf);
								}
							}
#endif

							item->SetCount(item->GetCount() - 1);
						}
						break;

						case USE_CHANGE_ATTRIBUTE:
#ifndef ENABLE_ATTR_6TH_7TH
						case USE_CHANGE_ATTRIBUTE2: // @fixme123
#endif
						{
							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if (item2->GetAttributeCount() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
								return false;
							}

#ifdef ENABLE_PROTO_RENEWAL
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_ENCHANT))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
							if (item2->GetType() == ITEM_COSTUME && item2->GetSubType() == COSTUME_ACCE)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}
#endif

							if ((GM_PLAYER == GetGMLevel()) && !test_server && (g_dwItemBonusChangeTime > 0))
							{
								//
								// Check whether sufficient time has elapsed from the time the item property was changed previously through Event Flag
								// If enough time has elapsed, set the time for the current property change.
								//

								/*uint32_t dwChangeItemAttrCycle = quest::CQuestManager::Instance().GetEventFlag(msc_szChangeItemAttrCycleFlag);
								if (dwChangeItemAttrCycle < msc_dwDefaultChangeItemAttrCycle)
									dwChangeItemAttrCycle = msc_dwDefaultChangeItemAttrCycle;*/
								const uint32_t dwChangeItemAttrCycle = g_dwItemBonusChangeTime;

								quest::PC* pPC = quest::CQuestManager::Instance().GetPC(GetPlayerID());

								if (pPC)
								{
									const uint32_t dwNowSec = get_global_time();

									uint32_t dwLastChangeItemAttrSec = pPC->GetFlag(msc_szLastChangeItemAttrFlag);

									if (dwLastChangeItemAttrSec + dwChangeItemAttrCycle > dwNowSec)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;398;%d;%d]", dwChangeItemAttrCycle, dwChangeItemAttrCycle - (dwNowSec - dwLastChangeItemAttrSec));
										return false;
									}

									pPC->SetFlag(msc_szLastChangeItemAttrFlag, dwNowSec);
								}
							}

							if (item->GetSubType() == USE_CHANGE_ATTRIBUTE2)
							{
								const int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = { 0, 0, 30, 40, 3 };
								item2->ChangeAttribute(aiChangeProb);
							}
							else if (item->GetVnum() == 76014)	// Gegenstand verzaubern(b)
							{
								const int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = { 0, 10, 50, 39, 1 };
								item2->ChangeAttribute(aiChangeProb);
							}
							else
							{
								// Special treatment for serialization
								// Hard-coded because the series will never be added.
								if (item->GetVnum() == 71151 || item->GetVnum() == 76023)	// Grüner Zauber | Grüner Zauber
								{
									if ((item2->GetType() == ITEM_WEAPON) || (item2->GetType() == ITEM_ARMOR/* && item2->GetSubType() == ARMOR_BODY)*/))	//@custom028
									{
										bool bCanUse = true;
										for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
										{
											if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)	// Ab Lvl 41 können diese Switcher nicht mehr verwendet werden!
											{
												bCanUse = false;
												break;
											}
										}
										if (false == bCanUse)
										{
											ChatPacket(CHAT_TYPE_INFO, "[LS;1064]");
											break;
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1065]");
										break;
									}
								}

								item2->ChangeAttribute();
							}

							ChatPacket(CHAT_TYPE_INFO, "[LS;399]");
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::Instance().ItemLog(this, item, "CHANGE_ATTRIBUTE", buf);
							}

							item->SetCount(item->GetCount() - 1);
						}
						break;

#ifdef ENABLE_ATTR_6TH_7TH
						case USE_CHANGE_ATTRIBUTE2:
						{
							// Disaster prevention in Europe, Singapore and Vietnam is prohibited
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (ITEM_COSTUME == item2->GetType()) // @fixme124
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
								return false;

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							if ((item->GetVnum() == 72351) && (number(1, 100) <= 85))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;401]");
								item->SetCount(item->GetCount() - 1);
								return false;
							}

							if (item2->ChangeRareAttribute() == true)
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::Instance().ItemLog(this, item, "CHANGE_RARE_ATTR", buf);

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
							}
						}
						break;
#endif

						case USE_ADD_ATTRIBUTE:
						{
							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

#ifdef ENABLE_PROTO_RENEWAL
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_REINFORCE))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;401]");
								return false;
							}
#endif

							if (item2->IsEquipped())	//@fixme424
								return false;

							if (item2->GetAttributeCount() < 4)
							{
								// Serialization is specially processed
								// Hard-coded because the series will never be added.
								if (item->GetVnum() == 71152 || item->GetVnum() == 76024)	// Grüne Stärke | Grüne Stärke
								{
									if ((item2->GetType() == ITEM_WEAPON) || (item2->GetType() == ITEM_ARMOR/* && item2->GetSubType() == ARMOR_BODY)*/))	//@custom028
									{
										bool bCanUse = true;
										for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
										{
											if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)	// Ab Lvl 41 können diese Switcher nicht mehr verwendet werden!
											{
												bCanUse = false;
												break;
											}
										}
										if (!bCanUse)
										{
											ChatPacket(CHAT_TYPE_INFO, "[LS;1064]");
											break;
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1065]");
										break;
									}
								}

								if (attr_always_add)	//@custom029
								{
									//@custom031
									aiItemAttributeAddPercent[0] = 100;
									aiItemAttributeAddPercent[1] = 100;
									aiItemAttributeAddPercent[2] = 100;
									aiItemAttributeAddPercent[3] = 100;
								}
								if (attr_always_5_add)	//@custom030
								{
									aiItemAttributeAddPercent[4] = 100;	//@custom031
								}

								if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])	//@custom031
								{
									item2->AddAttribute();
									ChatPacket(CHAT_TYPE_INFO, "[LS;400]");

									const int iAddedIdx = item2->GetAttributeCount() - 1;
									LogManager::Instance().ItemLog(
										GetPlayerID(),
										item2->GetAttributeType(iAddedIdx),
										item2->GetAttributeValue(iAddedIdx),
										item->GetID(),
										"ADD_ATTRIBUTE_SUCCESS",
										std::to_string(item2->GetID()).c_str(),
										GetDesc()->GetHostName(),
										item->GetOriginalVnum());
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;401]");
									LogManager::Instance().ItemLog(this, item, "ADD_ATTRIBUTE_FAIL", std::to_string(item2->GetID()).c_str());
								}

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;402]");
							}
						}
						break;

						case USE_ADD_ATTRIBUTE2:
						{
							// Blessing Orb
							// Add one more attribute to the item to which 4 attributes have been added through the home secretary.
							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

#ifdef ENABLE_PROTO_RENEWAL
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_REINFORCE))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;401]");
								return false;
							}
#endif

							if (item2->IsEquipped())	//@fixme424
								return false;

							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_REINFORCE))
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
								return false;
							}

							// Attributes can be added only when 4 attributes have already been added.
							if (item2->GetAttributeCount() == 4)
							{
								if (attr_always_5_add) {	//@custom030
									aiItemAttributeAddPercent[4] = 100;	//@custom031
								}

								if (number(1, 100) <= aiItemAttributeAddPercent[4])	//@custom031
							//	if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
								{
									item2->AddAttribute();
									ChatPacket(CHAT_TYPE_INFO, "[LS;400]");

									const int iAddedIdx = item2->GetAttributeCount() - 1;
									LogManager::Instance().ItemLog(
										GetPlayerID(),
										item2->GetAttributeType(iAddedIdx),
										item2->GetAttributeValue(iAddedIdx),
										item->GetID(),
										"ADD_ATTRIBUTE2_SUCCESS",
										std::to_string(item2->GetID()).c_str(),
										GetDesc()->GetHostName(),
										item->GetOriginalVnum());
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;401]");
									LogManager::Instance().ItemLog(this, item, "ADD_ATTRIBUTE2_FAIL", std::to_string(item2->GetID()).c_str());
								}

								item->SetCount(item->GetCount() - 1);
							}
							else if (item2->GetAttributeCount() == 5)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;404]");
							}
							else if (item2->GetAttributeCount() < 4)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;405]");
							}
							else
							{
								// wtf ?!
								sys_err("ADD_ATTRIBUTE2 : Item has wrong AttributeCount(%d)", item2->GetAttributeCount());
							}
						}
						break;

						case USE_ADD_ACCESSORY_SOCKET:
						{
							char buf[21];
							snprintf(buf, sizeof(buf), "%u", item2->GetID());

#ifdef ENABLE_PENDANT
							if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Nicht moeglich. USE_ADD_ACCESSORY_SOCKET"));
								return false;
							}
#endif

							if (item2->IsAccessoryForSocket())
							{
								if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
								{
#ifdef ENABLE_ADDSTONE_FAILURE
									if (number(1, 100) <= 50)
#else
									if (1)
#endif
									{
										item2->SetAccessorySocketMaxGrade(item2->GetAccessorySocketMaxGrade() + 1);
										ChatPacket(CHAT_TYPE_INFO, "[LS;406]");
										LogManager::Instance().ItemLog(this, item, "ADD_SOCKET_SUCCESS", buf);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;407]");
										LogManager::Instance().ItemLog(this, item, "ADD_SOCKET_FAIL", buf);
									}

									item->SetCount(item->GetCount() - 1);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, "[LS;408]");
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;409]");
							}
						}
						break;

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
						case USE_PUT_INTO_AURA_SOCKET:
						{
							if (item2->IsAuraBoosterForSocket() && item->CanPutInto(item2))
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								const uint8_t c_bAuraBoostIndex = item->GetOriginalVnum() - ITEM_AURA_BOOST_ITEM_VNUM_BASE;
								item2->SetSocket(ITEM_SOCKET_AURA_BOOST, c_bAuraBoostIndex * 100000000 + item->GetValue(ITEM_AURA_BOOST_TIME_VALUE));

								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Aura] Aura booster attached successfully."));

								LogManager::Instance().ItemLog(this, item, "PUT_AURA_SOCKET", buf);

								if (IS_SET(item->GetFlag(), ITEM_FLAG_STACKABLE) && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK) && item->GetCount() > 1)
									item->SetCount(item->GetCount() - 1);
								else
									ITEM_MANAGER::Instance().RemoveItem(item, "PUT_AURA_SOCKET_REMOVE");
							}
							else
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Aura] You cannot add aura boost to this item."));
						}
						break;
#endif

						case USE_PUT_INTO_BELT_SOCKET:
						case USE_PUT_INTO_ACCESSORY_SOCKET:
						{
#ifdef ENABLE_PENDANT
							if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Nicht moeglich. USE_PUT_INTO_ACCESSORY_SOCKET"));
								return false;
							}
#endif

							if (item2->IsAccessoryForSocket() && item->CanPutInto(item2))
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								if (item2->GetAccessorySocketGrade() < item2->GetAccessorySocketMaxGrade())
								{
									if (number(1, 100) <= aiAccessorySocketPutPct[item2->GetAccessorySocketGrade()])
									{
										item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
										ChatPacket(CHAT_TYPE_INFO, "[LS;410]");
										LogManager::Instance().ItemLog(this, item, "PUT_SOCKET_SUCCESS", buf);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;411]");
										LogManager::Instance().ItemLog(this, item, "PUT_SOCKET_FAIL", buf);
									}

									item->SetCount(item->GetCount() - 1);
								}
								else
								{
									if (item2->GetAccessorySocketMaxGrade() == 0)
										ChatPacket(CHAT_TYPE_INFO, "[LS;412]");
									else if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;413]");
										ChatPacket(CHAT_TYPE_INFO, "[LS;415]");
									}
									else
										ChatPacket(CHAT_TYPE_INFO, "[LS;416]");
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;417]");
							}
						}
						break;

						default:
							break;
					}

					if (item2->IsEquipped())
					{
						BuffOnAttr_AddBuffsFromItem(item2);
					}
				}
				break;
				// END_OF_ACCESSORY_REFINE & END_OF_ADD_ATTRIBUTES & END_OF_CHANGE_ATTRIBUTES

#ifdef ENABLE_CHANGED_ATTR
				case USE_SELECT_ATTRIBUTE:
				{
					LPITEM item2 = nullptr;
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsEquipped())
						BuffOnAttr_RemoveBuffsFromItem(item2);

#ifdef ENABLE_SEALBIND_SYSTEM
					if (item2->IsSealed() && !item->IsSocketModifyingItem())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1101]");
						return false;
					}
#endif

					if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_ENCHANT))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

					if (ITEM_COSTUME == item2->GetType())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

					if (item2->IsExchanging() || item2->IsEquipped())
						return false;

					if (item2->GetAttributeSetIndex() == -1)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;396]");
						return false;
					}

					if (item2->GetAttributeCount() == 0)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
						return false;
					}

					SelectAttr(item, item2);
				}
				break;
#endif

				case USE_BAIT:
				{
					if (m_pkFishingEvent
#ifdef ENABLE_FISHING_RENEWAL
						|| m_pkFishingNewEvent
#endif
						)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;418]");
						return false;
					}

					LPITEM weapon = GetWear(WEAR_WEAPON);

					if (!weapon || weapon->GetType() != ITEM_ROD)
						return false;

					if (weapon->GetSocket(2))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;419;[IN;%d]]", item->GetVnum());
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;420;[IN;%d]]", item->GetVnum());
					}

					weapon->SetSocket(2, item->GetValue(0));
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_MOVE:
				case USE_TREASURE_BOX:
				case USE_MONEYBAG:
				{
				}
				break;

				case USE_AFFECT:
				{
					if (FindAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].wPointType))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
					}
					else
					{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
						if (item->GetVnum() == 72355 || item->GetVnum() == 72356)
						{
							AddAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].wPointType, item->GetValue(2), 0, item->GetValue(3), 0, true, false, 0, (get_global_time() + static_cast<uint32_t>(item->GetValue(3))));
						}
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
						else if (item->GetVnum() == 70311 /*Blutopfer*/ || item->GetVnum() == 70312 /*Kostbares Blutopfer*/)
						{
							if (quest::CQuestManager::Instance().GetEventFlag("e_late_summer_event") == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, "event not active!");
								return false;
							}

							if (GetLevel() < 30)
							{
								ChatPacket(CHAT_TYPE_INFO, "only at lvl30");
								return false;
							}

							AddAffect(item->GetValue(0), POINT_NONE, GetSoulPoint(), AFF_NONE, INFINITE_AFFECT_DURATION, 0, false);
						}
#endif
#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) || defined(ENABLE_SUMMER_EVENT_ROULETTE)
						else
#endif
						{
							AddAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].wPointType, item->GetValue(2), 0, item->GetValue(3), 0, false);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
							CAchievementSystem::Instance().OnFishItem(this, achievements::ETaskTypes::TYPE_USE_BURN, item->GetVnum());
#endif
						}

						item->SetCount(item->GetCount() - 1);
					}
				}
				break;

				case USE_CREATE_STONE:
				{
					AutoGiveItem(number(28000, 28013));
					item->SetCount(item->GetCount() - 1);
				}
				break;

				// Recipe processing for potion making skill
				case USE_RECIPE:
				{
					LPITEM pSource1 = FindSpecifyItem(item->GetValue(1));
					const uint32_t dwSourceCount1 = item->GetValue(2);

					LPITEM pSource2 = FindSpecifyItem(item->GetValue(3));
					const uint32_t dwSourceCount2 = item->GetValue(4);

					if (dwSourceCount1 != 0)
					{
						if (pSource1 == nullptr)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;421]");
							return false;
						}
					}

					if (dwSourceCount2 != 0)
					{
						if (pSource2 == nullptr)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;421]");
							return false;
						}
					}

					if (pSource1 != nullptr)
					{
						if (pSource1->GetCount() < dwSourceCount1)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;422;[IN;%d]]", pSource1->GetVnum());
							return false;
						}

						pSource1->SetCount(pSource1->GetCount() - dwSourceCount1);
					}

					if (pSource2 != nullptr)
					{
						if (pSource2->GetCount() < dwSourceCount2)
						{
							ChatPacket(CHAT_TYPE_INFO, "[LS;422;[IN;%d]]", pSource2->GetVnum());
							return false;
						}

						pSource2->SetCount(pSource2->GetCount() - dwSourceCount2);
					}

					LPITEM pBottle = FindSpecifyItem(50901);

					if (!pBottle || pBottle->GetCount() < 1)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;423]");
						return false;
					}

					pBottle->SetCount(pBottle->GetCount() - 1);

					if (number(1, 100) > item->GetValue(5))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;424]");
						return false;
					}

					AutoGiveItem(item->GetValue(0));
				}
				break;

				case USE_CALL:
				{
					AggregateMonster();
	#if defined(ENABLE_CAPE_OF_COURAGE_INFINITE) && defined(ENABLE_ITEM_USE_CUSTOM_AFFECT)
					if (item->GetVnum() != 900116)
	#endif
					{
						item->SetCount(item->GetCount() - 1);
					}
				}
				break;

#ifdef ENABLE_EXPRESSING_EMOTION
				case USE_EMOTION_PACK:
				{
					if (CountEmotion() >= 12)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't add more emotions."));
						return false;
					}

					item->SetCount(item->GetCount() - 1);

					const uint8_t pct = number(1, 100);

					if (pct <= 30)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You couldn't add a new emotion."));
						return false;
					}

					InsertEmotion();
				}
				break;
#endif

#ifdef ENABLE_FLOWER_EVENT
				case USE_FLOWER:
				{
					// Flower power
					if (item->GetVnum() >= 25121 && item->GetVnum() <= 25125)
					{
						FlowerSystem(item);
						return false;
					}
				}
				break;
#endif

				default:
					break;
			}  // switch subtype
		}
		break;

		case ITEM_METIN:
		{
			LPITEM item2;

			if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsExchanging() || item2->IsEquipped()) // @fixme114
				return false;

			if (item2->GetType() == ITEM_PICK) return false;
			if (item2->GetType() == ITEM_ROD) return false;

			int i;

#ifdef ENABLE_PROTO_RENEWAL
			for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
			for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
			{
				uint32_t dwVnum;

				if ((dwVnum = item2->GetSocket(i)) <= 2)
					continue;

				const TItemTable* p = ITEM_MANAGER::Instance().GetTable(dwVnum);

				if (!p)
					continue;

				if (item->GetValue(5) == p->alValues[5]
#ifdef ENABLE_GLOVE_SYSTEM
					&& (item->GetSubType() != METIN_SUNGMA)
#endif
					)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;426]");
					return false;
				}
			}

			if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY)
			{
				if (!IS_SET(item->GetWearFlag(), WEARABLE_BODY) || !IS_SET(item2->GetWearFlag(), WEARABLE_BODY))
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;427]");
					return false;
				}
			}
/*#ifdef ENABLE_GLOVE_SYSTEM
			else if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_GLOVE)
			{
				if (!IS_SET(item->GetWearFlag(), WEARABLE_GLOVE) || !IS_SET(item2->GetWearFlag(), WEARABLE_GLOVE))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Du kannst diesen Geiststein nicht in diesen Handschuh anbringen."));
					return false;
				}
			}
#endif*/
			else if (item2->GetType() == ITEM_WEAPON)
			{
				if (!IS_SET(item->GetWearFlag(), WEARABLE_WEAPON))
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;428]");
					return false;
				}
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;431]");
				return false;
			}

#ifdef ENABLE_ATTRIBUTE_CHANGE_ON_START_ITEM
			if (StartWeaponBlockAttribute(item2->GetVnum()))	//@custom007
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Nicht moeglich. ITEM_METIN"));
				return false;
			}
#endif

#ifdef ENABLE_PENDANT
			if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Nicht moeglich. ITEM_METIN"));
				return false;
			}
#endif

#ifdef ENABLE_PROTO_RENEWAL
			for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
#else
			for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
#endif
			{
				if (item2->GetSocket(i) >= 1 && item2->GetSocket(i) <= 2 && item2->GetSocket(i) >= item->GetValue(2))
				{
#ifdef ENABLE_ADDSTONE_FAILURE_CUSTOM
					// NEW ADDSTONE_FAILURE_SYSTEM [Custom]
					switch (item->GetVnum())
					{
						// +0er Stones
						case 28000: case 28004: case 28008: case 28012:
						case 28030: case 28031: case 28032: case 28033:
						case 28034: case 28035: case 28036: case 28037:
						case 28038: case 28039: case 28040: case 28041:
						case 28042: case 28043: case 28044: case 28045:
#	ifdef ENABLE_YOHARA_SYSTEM
						case 28046: case 28047:
#	endif
						{
							if (number(1, 100) <= STONE_FAILURE0)	// 80% Succes Rate
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
								item2->SetSocket(i, item->GetVnum());
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
								{
									int iRandomType = item2->GetSungmaUniqueRandomSocket();
									if (item->GetValue(5) == 2 && number(1, 100) <= STONE_FAILURE0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1613]");
										iRandomType *= 10;
									}

									item2->SetSocket(i + 3, iRandomType);
								}
#	endif
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
								item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
									item2->SetSocket(i + 3, ITEM_BROKEN_METIN_VNUM);
#	endif
							}
						}
						break;

						// +1er Stones
						case 28100: case 28104: case 28108: case 28112:
						case 28130: case 28131: case 28132: case 28133:
						case 28134: case 28135: case 28136: case 28137:
						case 28138: case 28139: case 28140: case 28141:
						case 28142: case 28143: case 28144: case 28145:
#	ifdef ENABLE_YOHARA_SYSTEM
						case 28146: case 28147:
#	endif
						{
							if (number(1, 100) <= STONE_FAILURE1)	// 70% Succes Rate
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
								item2->SetSocket(i, item->GetVnum());
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
								{
									int iRandomType = item2->GetSungmaUniqueRandomSocket();
									if (item->GetValue(5) == 2 && number(1, 100) <= STONE_FAILURE0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1613]");
										iRandomType *= 10;
									}

									item2->SetSocket(i + 3, iRandomType);
								}
#	endif
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
								item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
									item2->SetSocket(i + 3, ITEM_BROKEN_METIN_VNUM);
#	endif
							}
						}
						break;

						// +2er Stones
						case 28200: case 28204: case 28208: case 28212:
						case 28230: case 28231: case 28232: case 28233:
						case 28234: case 28235: case 28236: case 28237:
						case 28238: case 28239: case 28240: case 28241:
						case 28242: case 28243: case 28244: case 28245:
#	ifdef ENABLE_YOHARA_SYSTEM
						case 28246: case 28247:
#	endif
						{
							if (number(1, 100) <= STONE_FAILURE2)	// 60% Succes Rate
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
								item2->SetSocket(i, item->GetVnum());
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
								{
									int iRandomType = item2->GetSungmaUniqueRandomSocket();
									if (item->GetValue(5) == 2 && number(1, 100) <= STONE_FAILURE0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1613]");
										iRandomType *= 10;
									}

									item2->SetSocket(i + 3, iRandomType);
								}
#	endif
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
								item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
									item2->SetSocket(i + 3, ITEM_BROKEN_METIN_VNUM);
#	endif
							}
						}
						break;

						// +3er Stones
						case 28300: case 28304: case 28308: case 28312:
						case 28330: case 28331: case 28332: case 28333:
						case 28334: case 28335: case 28336: case 28337:
						case 28338: case 28339: case 28340: case 28341:
						case 28342: case 28343: case 28344: case 28345:
#	ifdef ENABLE_YOHARA_SYSTEM
						case 28346: case 28347:
#	endif
						{
							if (number(1, 100) <= STONE_FAILURE3)	// 50% Succes Rate
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
								item2->SetSocket(i, item->GetVnum());
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
								{
									int iRandomType = item2->GetSungmaUniqueRandomSocket();
									if (item->GetValue(5) == 2 && number(1, 100) <= STONE_FAILURE0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1613]");
										iRandomType *= 10;
									}

									item2->SetSocket(i + 3, iRandomType);
								}
#	endif
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
								item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
									item2->SetSocket(i + 3, ITEM_BROKEN_METIN_VNUM);
#	endif
							}
						}
						break;

						// +4er Stones
						case 28400: case 28404: case 28408: case 28412:
						case 28430: case 28431: case 28432: case 28433:
						case 28434: case 28435: case 28436: case 28437:
						case 28438: case 28439: case 28440: case 28441:
						case 28442: case 28443: case 28444: case 28445:
#	ifdef ENABLE_YOHARA_SYSTEM
						case 28446: case 28447:
#	endif
						{
							if (number(1, 100) <= STONE_FAILURE4)	// 40% Succes Rate
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
								item2->SetSocket(i, item->GetVnum());
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
								{
									int iRandomType = item2->GetSungmaUniqueRandomSocket();
									if (item->GetValue(5) == 2 && number(1, 100) <= STONE_FAILURE0)
									{
										ChatPacket(CHAT_TYPE_INFO, "[LS;1613]");
										iRandomType *= 10;
									}

									item2->SetSocket(i + 3, iRandomType);
								}
#	endif
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
								item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
								if (item->GetSubType() == METIN_SUNGMA)
									item2->SetSocket(i + 3, ITEM_BROKEN_METIN_VNUM);
#	endif
							}
						}
						break;

						default:
							break;
					}//-> Switch Ends
					// END_NEW_ADDSTONE_FAILURE_SYSTEM [Custom]
#else
#ifdef ENABLE_ADDSTONE_FAILURE
					if (number(1, 100) <= 30)
#else
					if (1)
#endif
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;429]");
						item2->SetSocket(i, item->GetVnum());
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
						if (item->GetSubType() == METIN_SUNGMA)
						{
							int iRandomType = item2->GetSungmaUniqueRandomSocket();
							// You have a small percentage of the bonus being doubled if the spiritStone you added is TYPE_GOLD
							if (item->GetValue(5) == 2 && number(1, 100) <= STONE_FAILURE0)
							{
								ChatPacket(CHAT_TYPE_INFO, "[LS;1613]");
								iRandomType *= 10;
							}

							item2->SetSocket(i + 3, iRandomType);
						}
#	endif
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;430]");
						item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
#	if defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)
						if (item->GetSubType() == METIN_SUNGMA)
							item2->SetSocket(i + 3, ITEM_BROKEN_METIN_VNUM);
#	endif
					}
#endif

					LogManager::Instance().ItemLog(this, item2, "SOCKET", item->GetName());
#ifdef ENABLE_STACKABLES_STONES
					item->SetCount(item->GetCount() - 1);
#else
					ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (METIN)");
#endif
					break;
				}
			}

#ifdef ENABLE_PROTO_RENEWAL
			if (i == METIN_SOCKET_MAX_NUM)
#else
			if (i == ITEM_SOCKET_MAX_NUM)
#endif
				ChatPacket(CHAT_TYPE_INFO, "[LS;431]");
		}
		break;

		case ITEM_AUTOUSE:
		case ITEM_MATERIAL:
		case ITEM_SPECIAL:
		case ITEM_TOOL:
		case ITEM_LOTTERY:
			break;

		case ITEM_TOTEM:
		{
			if (!item->IsEquipped())
				EquipItem(item);
		}
		break;

		case ITEM_BLEND:
		{
			// new herbs
			sys_log(0, "ITEM_BLEND!!");
			if (Blend_Item_find(item->GetVnum()))
			{
				constexpr int affect_type = AFFECT_BLEND;
				const int apply_type = aApplyInfo[item->GetSocket(0)].wPointType;
				const int apply_value = item->GetSocket(1);
				const int apply_duration = item->GetSocket(2);

				bool affectAbilityFound = FindAffect(AFFECT_UNIQUE_ABILITY, apply_type);	//@custom032

				if (FindAffect(affect_type, apply_type))
				{
					if (!affectAbilityFound)
						ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
					else
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANT_USE_WATER_AND_BLEND_ITEM_TOGETHER"));	//@custom032
				}
				else
				{
					if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, POINT_RESIST_MAGIC))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
					}
					else
					{
						SetAffectPotion(item);	//@custom001
						AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, false);
						item->SetCount(item->GetCount() - 1);
					}
				}
			}
		}
		break;

		case ITEM_EXTRACT:
		{
			LPITEM pDestItem = GetItem(DestCell);
			if (nullptr == pDestItem)
				return false;

			switch (item->GetSubType())
			{
				case EXTRACT_DRAGON_SOUL:
				{
					if (pDestItem->IsDragonSoul())
					{
						return DSManager::Instance().PullOut(this, NPOS, pDestItem, item);
					}
					return false;
				}
				break;

				case EXTRACT_DRAGON_HEART:
				{
					if (pDestItem->IsDragonSoul())
					{
						return DSManager::Instance().ExtractDragonHeart(this, pDestItem, item);
					}
					return false;
				}
				break;

				default:
					return false;
			}
		}
		break;

		case ITEM_PET:
		{
			switch (item->GetSubType())
			{
#ifdef ENABLE_GROWTH_PET_SYSTEM
				case PET_EGG:
				{
					if (GetOpenedWindow(W_EXCHANGE | W_MYSHOP | W_SHOP_OWNER | W_SAFEBOX | W_CUBE
	#ifdef ENABLE_SWITCHBOT
						| W_SWITCHBOT
	#endif
					))
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1164]");
						return false;
					}
				}
				break;

				case PET_UPBRINGING:
				{
					if (m_pkFishingEvent)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1440]");
						return false;
					}

					if (GetExchange())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1192]");
						return false;
					}

					CGrowthPetSystem* petSystem = GetGrowthPetSystem();
					if (!petSystem)
						return false;

					// Check pet evolution
					const uint32_t dwPetCurrentVnum = GetQuestFlag("growthpet_system.growthpet_summoned");
	#ifdef ENABLE_PET_SUMMON_AFTER_REWARP
					const uint32_t dwPetCurrentID = GetQuestFlag("growthpet_system.growthpet_id");
	#endif

					uint32_t dwPetMobVnum = item->GetValue(0);
					const uint8_t dwPetEvolution = item->GetGrowthPetItemInfo().evol_level;
					if (dwPetEvolution >= 4)
						dwPetMobVnum = item->GetValue(3);

					// Check Pet Dead
					if (static_cast<long>((item->GetSocket(0) - get_global_time()) / 60) <= 0)
						return false;

					const auto petLevel = static_cast<int>(item->GetGrowthPetItemInfo().pet_level);
					if (petLevel > GetLevel())
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;1144]");
						return false;
					}

					if (petSystem->IsActivePet())
					{
						petSystem->Dismiss();
	#ifdef ENABLE_PET_SUMMON_AFTER_REWARP
						LPITEM pSummonItem = ITEM_MANAGER::Instance().Find(dwPetCurrentID);
						if (pSummonItem)
							pSummonItem->SetSocket(3, FALSE);
	#endif

						if (dwPetMobVnum != dwPetCurrentVnum)
						{
							petSystem->Summon(dwPetMobVnum, item, "", false);
							SetQuestFlag("growthpet_system.growthpet_summoned", dwPetMobVnum);
							SetQuestFlag("growthpet_system.growthpet_id", item->GetID());
	#ifdef ENABLE_PET_SUMMON_AFTER_REWARP
							item->SetSocket(3, TRUE);
	#endif
						}
						return false;
					}
					else
					{
						if (petSystem->CountSummoned() >= 1)
							return false;

						petSystem->Summon(dwPetMobVnum, item, "", false);
						SetQuestFlag("growthpet_system.growthpet_summoned", dwPetMobVnum);
						SetQuestFlag("growthpet_system.growthpet_id", item->GetID());
	#ifdef ENABLE_PET_SUMMON_AFTER_REWARP
						item->SetSocket(3, TRUE);
	#endif
						return true;
					}

					return false;
				}
				break;

				case PET_BAG:
				{
					const long GetPetBagTime = (item->GetSocket(0) - get_global_time()) / 60;

					// Bagging
					if (GetPetBagTime <= 0)
					{
						LPITEM item2 = GetItem(DestCell);
						if (!item2)
							return false;
						if (item2->IsExchanging())
							return false;
						if (item2->GetType() != ITEM_PET)
							return false;
						if (item2->isLocked())
							return false;

						const long lExpireTime = time(0) + item->GetLimitValue(0);
						item->SetSocket(0, lExpireTime);
						item->SetSocket(1, item2->GetSocket(1));
						item->SetSocket(2, item2->GetSocket(2));

						item->SetGrowthPetItemInfo(item2->GetGrowthPetItemInfo());

						item->StartRealTimeExpireEvent();

						item->UpdatePacket();
						item->Save();

						ITEM_MANAGER::Instance().RemoveItem(item2, "PET_BAGGING");

						// Send Message
						ChatPacket(CHAT_TYPE_INFO, "[LS;1154;%s]", item2->GetName());

						return true;
					}

					// Unbagging
					if (GetPetBagTime > 0)
					{
						if (item->isLocked())
							return false;
						if (item->IsExchanging())
							return false;

						const uint16_t wCell = item->GetCell();
						const uint32_t dwVnum = item->GetGrowthPetItemInfo().pet_vnum;
						if (!dwVnum)
							return false;

						LPITEM item2 = ITEM_MANAGER::Instance().CreateItem(dwVnum, 1, 0, true);
						if (!item2)
						{
							sys_err("<GrowthPet> there was an error removing the pet from the bag vnum: %d.", dwVnum);
							return false;
						}

						// Check Pet Dead
						if (static_cast<long>((item->GetSocket(0) - get_global_time()) / 60) <= 0)
							return false;

						const long lExpireTime = time(0) + item->GetGrowthPetItemInfo().pet_max_time;
						item2->SetSocket(0, lExpireTime);
						item2->SetSocket(1, item->GetSocket(1));
						item2->SetSocket(2, item->GetSocket(2));

						item2->SetGrowthPetItemInfo(item->GetGrowthPetItemInfo());

						item2->UpdatePacket();
						item2->Save();

						// Remove Bag
						ITEM_MANAGER::Instance().RemoveItem(item, "PET_UNBAGGING");

						// Add Pet to Character
						item2->AddToCharacter(this, TItemPos(INVENTORY, wCell));

						// Send Message
						ChatPacket(CHAT_TYPE_INFO, "[LS;1155;%s]", item2->GetName());

						return true;
					}
				}
				break;

				case PET_FEEDSTUFF:
				{
					LPITEM item2 = GetItem(DestCell);
					if (!item2)
						return false;
					if (item2->IsExchanging())
						return false;
					if (!((item2->GetType() == ITEM_PET) && (item2->GetSubType() == PET_UPBRINGING)))
						return false;

					bool isDead = false;
					const long duration = (item2->GetSocket(0) - get_global_time()) / 60;
					if (duration < 0)
						isDead = true;

					TGrowthPetInfo petInfo = item2->GetGrowthPetItemInfo();

					time_t pet_duration = petInfo.pet_max_time;
					if (isDead)
					{
						pet_duration /= 2;
						petInfo.pet_birthday = time(0);
					}

					item2->SetSocket(0, time(0) + pet_duration);
					item2->SetGrowthPetItemInfo(petInfo);

					item->SetCount(item->GetCount() - 1);

					if (isDead)
						ChatPacket(CHAT_TYPE_INFO, "[LS;1153;%s]", item2->GetName());
					else
						ChatPacket(CHAT_TYPE_INFO, "[LS;1151]");

					return true;

				}
				break;
#endif

#ifdef ENABLE_PET_SYSTEM
				case PET_PAY:
				{
					if (!item->IsEquipped())
					{
						EquipItem(item);
						PetSummon(item);
					}
					else
					{
						UnequipItem(item);
						PetUnsummon(item);
					}

				/*	const long lPetMobVNum = item->GetValue(0);
					if (lPetMobVNum == 0)
						return false;

					CPetSystem* petSystem = GetPetSystem();
					if (!petSystem)
						return false;

					const size_t petCount = petSystem->CountSummoned();
					if (petCount > 0)
					{
						ChatPacket(CHAT_TYPE_INFO, 1323);
						return false;
					}

					petSystem->Summon(lPetMobVNum, item, "", false);
					return true;*/

					/*const long lPetMobVNum = item->GetValue(0);
					if (lPetMobVNum == 0)
						return false;

					CPetSystem* petSystem = GetPetSystem();
					if (!petSystem)
						return false;

					const CPetActor* petActor = petSystem->GetByVnum(lPetMobVNum);
					if (petActor && petActor->IsSummoned())
					{
						petSystem->Unsummon(lPetMobVNum);
						return true;
					}
					else
					{
						const size_t petCount = petSystem->CountSummoned();
						if (petCount > 0)
						{
							// 1323 - You cannot summon your mount / pet right now.
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot summon your pet right now."));
							return false;
						}

						std::string petName = "NoName";
						const CMob* pkMob = CMobManager::Instance().Get(lPetMobVNum);
						if (pkMob)
						{
							petName = " - ";
							petName += pkMob->m_table.szLocaleName;
						}

						const CPetActor* pet = petSystem->Summon(lPetMobVNum, item, petName.c_str(), false);
						if (pet)
							return true;
					}

					return false;*/
				}
				break;
#endif

				default:
					quest::CQuestManager::Instance().UseItem(GetPlayerID(), item, false);
					break;
			}
		}
		break;

#ifdef ENABLE_GACHA_SYSTEM
		case ITEM_GACHA:
		{
#	ifdef ENABLE_NEWSTUFF
			if (0 != g_BoxUseTimeLimitValue)
			{
				if (get_dword_time() < m_dwLastBoxUseTime + g_BoxUseTimeLimitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot drop Yang yet"));
					return false;
				}
			}

			m_dwLastBoxUseTime = get_dword_time();
#	endif

			const uint32_t dwBoxVnum = item->GetVnum();
			std::vector <uint32_t> dwVnums;
			std::vector <uint32_t> dwCounts;
			std::vector <LPITEM> item_gets(0);
			int count = 0;

#	ifdef ENABLE_BLOCK_CHEST_INVENTORY_FULL	//@fixme489
			int iEmptyCell = -1;
			if (item->IsDragonSoul())
			{
				if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
					return false;
				}
			}
			else
			{
#	ifdef ENABLE_SPECIAL_INVENTORY
				if ((iEmptyCell = GetEmptyInventory(item)) == -1)
#	else
				if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
#	endif
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
					return false;
				}
			}
#	endif	//@end_fixme489

			if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count))
			{
				for (int i = 0; i < count; i++)
				{
					switch (dwVnums[i])
					{
						case CSpecialItemGroup::GOLD:
#	ifdef ENABLE_CHAT_SETTINGS
							ChatPacket(CHAT_TYPE_MONEY_INFO, "[LS;1269;%d]", dwCounts[i]);
#	else
							ChatPacket(CHAT_TYPE_INFO, "[LS;1269;%d]", dwCounts[i]);
#	endif
							break;

						case CSpecialItemGroup::EXP:
		#ifdef ENABLE_CHAT_SETTINGS
							ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1279]");
							ChatPacket(CHAT_TYPE_EXP_INFO, "[LS;1290;%d]", dwCounts[i]);
		#else
							ChatPacket(CHAT_TYPE_INFO, "[LS;1279]");
							ChatPacket(CHAT_TYPE_INFO, "[LS;1290;%d]", dwCounts[i]);
		#endif
							break;

						case CSpecialItemGroup::MOB:
							ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
							break;

						case CSpecialItemGroup::SLOW:
							ChatPacket(CHAT_TYPE_INFO, "[LS;1310]");
							break;

						case CSpecialItemGroup::DRAIN_HP:
							ChatPacket(CHAT_TYPE_INFO, "[LS;3]");
							break;

						case CSpecialItemGroup::POISON:
							ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
							break;

#	ifdef ENABLE_WOLFMAN_CHARACTER
						case CSpecialItemGroup::BLEEDING:
							ChatPacket(CHAT_TYPE_INFO, "[LS;13]");
							break;
#	endif

						case CSpecialItemGroup::MOB_GROUP:
							ChatPacket(CHAT_TYPE_INFO, "[LS;1299]");
							break;

						default:
						{
							if (item_gets[i])
							{
#	ifdef ENABLE_CHAT_SETTINGS
								if (dwCounts[i] > 1)
									ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;204;%d;%d]", item_gets[i]->GetName(), dwCounts[i]);
								else
									ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;35;%d]", item_gets[i]->GetName());
#	else
								if (dwCounts[i] > 1)
									ChatPacket(CHAT_TYPE_INFO, "[LS;204;%d;%d]", item_gets[i]->GetName(), dwCounts[i]);
								else
									ChatPacket(CHAT_TYPE_INFO, "[LS;35;%d]", item_gets[i]->GetName());
#	endif
							}
						}
						break;
					}
				}

				if (item->GetSocket(0) > 1)
					item->SetSocket(0, item->GetSocket(0) - 1);
				else
					ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (ITEM_GACHA)");
			}
			else
			{
				ChatPacket(CHAT_TYPE_TALKING, "[LS;56]");
				return false;
			}
		}
		break;
#endif

#ifdef ENABLE_SOUL_SYSTEM
		case ITEM_SOUL:
		{
			const uint8_t bSoulGrade = static_cast<uint8_t>(item->GetValue(1));
			uint32_t dwPlayTimeBonus = 0;

			const uint32_t dwPlayTime = item->GetSocket(3);

			if (bSoulGrade == BASIC_SOUL)
			{
				if (dwPlayTime >= 60)
					dwPlayTimeBonus = item->GetValue(3);
			}
			else if (bSoulGrade == GLEAMING_SOUL)
			{
				if (dwPlayTime >= 60)
					dwPlayTimeBonus = item->GetValue(3);
			}
			else if (bSoulGrade == LUSTROUS_SOUL)
			{
				if (dwPlayTime >= 60 && dwPlayTime < 120)
					dwPlayTimeBonus = item->GetValue(3);
				else if (dwPlayTime >= 120)
					dwPlayTimeBonus = item->GetValue(4);
			}
			else if (bSoulGrade == PRISMATIC_SOUL)
			{
				if (dwPlayTime >= 60 && dwPlayTime < 120)
					dwPlayTimeBonus = item->GetValue(3);
				else if (dwPlayTime >= 120 && dwPlayTime < 180)
					dwPlayTimeBonus = item->GetValue(4);
				else if (dwPlayTime >= 180)
					dwPlayTimeBonus = item->GetValue(5);
			}
			else if (bSoulGrade == ILUMINED_SOUL)
			{
				if (dwPlayTime >= 60 && dwPlayTime < 120)
					dwPlayTimeBonus = item->GetValue(3);
				else if (dwPlayTime >= 120 && dwPlayTime < 180)
					dwPlayTimeBonus = item->GetValue(4);
				else if (dwPlayTime >= 180)
					dwPlayTimeBonus = item->GetValue(5);
			}
			else
			{
				break;
			}

			EAffectTypes type = AFFECT_NONE;
			EAffectBits flag = AFF_NONE;

			switch (item->GetSubType())
			{
				case RED_SOUL:
					type = AFFECT_SOUL_RED;
					flag = AFF_SOUL_RED;
					break;

				case BLUE_SOUL:
					type = AFFECT_SOUL_BLUE;
					flag = AFF_SOUL_BLUE;
					break;

				default:
					break;
			}

			if (AFFECT_NONE == type)
				break;

			// Check if ItemSoul is activated
			if (item->GetSubType() == RED_SOUL && item->GetSocket(1) == 0 && FindAffect(AFFECT_SOUL_RED, POINT_NORMAL_HIT_DAMAGE_BONUS))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
				return false;
			}

			if (item->GetSubType() == BLUE_SOUL && item->GetSocket(1) == 0 && FindAffect(AFFECT_SOUL_BLUE, POINT_SKILL_DAMAGE_BONUS))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;99]");
				return false;
			}

			CAffect* pAffect = FindAffect(type);
			CAffect* pAffectMixed = FindAffect(AFFECT_SOUL_MIX);

			if (pAffect == nullptr || pAffectMixed == nullptr)
			{
				EPointTypes bonus = POINT_NONE;

				if (type == AFFECT_SOUL_RED)
				{
					bonus = POINT_NORMAL_HIT_DAMAGE_BONUS;
				}
				else if (type == AFFECT_SOUL_BLUE)
				{
					bonus = POINT_SKILL_DAMAGE_BONUS;
				}

				AddAffect(type, bonus, dwPlayTimeBonus, item->GetID(), item->GetLimitValue(0), 0, true, false);
				if (FindAffect(AFFECT_SOUL_RED) && FindAffect(AFFECT_SOUL_BLUE))
					AddAffect(AFFECT_SOUL_MIX, POINT_NONE, 0, AFF_SOUL_MIX, item->GetLimitValue(0), 0, true, false);
				else
					AddAffect(AFFECT_SOUL_MIX, POINT_NONE, 0, flag, item->GetLimitValue(0), 0, true, false);

				item->Lock(true);
				item->SetSocket(1, true);
			}
			else
			{
				if (item->GetID() == pAffect->dwFlag && flag == pAffectMixed->dwFlag)
				{
					RemoveAffect(pAffect);
					RemoveAffect(pAffectMixed);

					item->Lock(false);
					item->SetSocket(1, false);
				}
				else
				{
					LPITEM old = FindItemByID(pAffect->dwFlag);

					if (nullptr != old)
					{
						old->Lock(false);
						old->SetSocket(1, false);
					}

					RemoveAffect(pAffect);

					if (FindAffect(AFFECT_SOUL_RED))
						AddAffect(AFFECT_SOUL_MIX, POINT_NONE, 0, AFF_SOUL_RED, item->GetLimitValue(0), 0, true, false);
					else if (FindAffect(AFFECT_SOUL_BLUE))
						AddAffect(AFFECT_SOUL_MIX, POINT_NONE, 0, AFF_SOUL_BLUE, item->GetLimitValue(0), 0, true, false);
					else
						RemoveAffect(pAffectMixed);

					item->Lock(false);
					item->SetSocket(1, false);
				}
			}
		}
		break;
#endif

		case ITEM_NONE:
			sys_err("Item type NONE %s", item->GetName());
			break;

		default:
			sys_log(0, "UseItemEx: Unknown type %s %d", item->GetName(), item->GetType());
			return false;
	}

	return true;
}

int g_nPortalLimitTime = 10;

bool CHARACTER::UseItem(TItemPos Cell, TItemPos DestCell)
{
	const uint16_t wCell = Cell.cell;
	const uint8_t window_type = Cell.window_type;
	//uint16_t wDestCell = DestCell.cell;
	//uint8_t bDestInven = DestCell.window_type;
	LPITEM item;

	if (!CanHandleItem())
	{
#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, "[LS;1345]");
#endif
		return false;
	}

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	//@custom008
	//We don't want to use it if we are dragging it over another item of the same type...
	LPITEM destItem = GetItem(DestCell);
	if (destItem && item != destItem && destItem->IsStackable() && !IS_SET(destItem->GetAntiFlag(), ITEM_ANTIFLAG_STACK) && destItem->GetVnum() == item->GetVnum()) {
		if (MoveItem(Cell, DestCell, 0))
			return false;
	}

	sys_log(0, "%s: USE_ITEM %s (inven %d, cell: %d)", GetName(), item->GetName(), window_type, wCell);

	if (item->IsExchanging())
		return false;

	if (!item->CanUsedBy(this))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1004]");
		return false;
	}

	if (IsStun())
		return false;

	if (false == FN_check_item_sex(this, item))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1005]");
		return false;
	}

#ifdef ENABLE_SWITCHBOT
	if (Cell.IsSwitchbotPosition())
	{
		CSwitchbot* pkSwitchbot = CSwitchbotManager::Instance().FindSwitchbot(GetPlayerID());
		if (pkSwitchbot && pkSwitchbot->IsActive(Cell.cell))
		{
			return false;
		}

#ifdef ENABLE_SPECIAL_INVENTORY
		int iEmptyCell = GetEmptyInventory(item);
#else
		int iEmptyCell = GetEmptyInventory(item->GetSize());
#endif

		if (iEmptyCell == -1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot remove item from switchbot. Inventory is full."));
			return false;
		}

		MoveItem(Cell, TItemPos(INVENTORY, iEmptyCell), item->GetCount());
		return true;
	}
#endif

	if ((item->IsHorseSummonItem() || item->IsRideItem())
#	ifdef DISABLE_RIDE_ON_SPECIAL_MAP
		&& !IS_MOUNTABLE_ZONE(GetMapIndex())
#	endif
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot ride here"));
		return false;
	}

#ifdef ENABLE_MOUNT_CHECK
	// If you want to use a wild super mount and your horse is not armed.
	if ((GetHorseLevel() < 11) && (
		(item->GetVnum() >= 52006 && item->GetVnum() <= 52010) || (item->GetVnum() >= 52021 && item->GetVnum() <= 52025) ||
		(item->GetVnum() >= 52036 && item->GetVnum() <= 52040) || (item->GetVnum() >= 52051 && item->GetVnum() <= 52055) ||
		(item->GetVnum() >= 52066 && item->GetVnum() <= 52070) || (item->GetVnum() >= 52081 && item->GetVnum() <= 52085) ||
		(item->GetVnum() >= 52096 && item->GetVnum() <= 52100) || (item->GetVnum() >= 52111 && item->GetVnum() <= 52115)
		)
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must have an armed horse to be able to ride this mount."));
		return false;
	}

	// If you want to use a super brave mount and your horse is not military.
	if ((GetHorseLevel() < 21) && (
		(item->GetVnum() >= 52011 && item->GetVnum() <= 52015) || (item->GetVnum() >= 52026 && item->GetVnum() <= 52030) ||
		(item->GetVnum() >= 52041 && item->GetVnum() <= 52045) || (item->GetVnum() >= 52056 && item->GetVnum() <= 52060) ||
		(item->GetVnum() >= 52071 && item->GetVnum() <= 52075) || (item->GetVnum() >= 52086 && item->GetVnum() <= 52090) ||
		(item->GetVnum() >= 52101 && item->GetVnum() <= 52105) || (item->GetVnum() >= 52116 && item->GetVnum() <= 52120)
		)
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must have a military horse to be able to ride this mount."));
		return false;
	}
#endif

#ifdef ENABLE_GLOVE_SYSTEM
	if ((item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_GLOVE) &&
		(GetQuestFlag("glove_system.can_equip_glove") == 0))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1677]");
		return false;
	}
#endif

	//PREVENT_TRADE_WINDOW
	if (IS_SUMMON_ITEM(item))
	{
		if (!IS_SUMMONABLE_ZONE(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;97]");
			return false;
		}

		// WarpToPC() checks whether the other party is in SUMMONABLE_ZONE.

		//In the three-way related map, the return unit is blocked.
		if (CThreeWayWar::Instance().IsThreeWayWarMapIndex(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;433]");
			return false;
		}
		const int iPulse = thecore_pulse();

		//Check after warehouse
		if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;434;%d]", g_nPortalLimitTime);

			if (test_server)
				ChatPacket(CHAT_TYPE_INFO, "Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
			return false;
		}

		//Check the transaction related window
		if (IsShop() || GetOpenedWindow(W_EXCHANGE | W_SHOP_OWNER | W_SAFEBOX | W_CUBE
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
			// Missing CHANGE_LOOK?
		))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[You cannot use a Scroll of the Location while another window is open.]You cannot use a Scroll of the Location while another window is open."));
			return false;
		}

		//PREVENT_REFINE_HACK
		//Time check after improvement
		{
			if (iPulse - GetRefineTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;437;%d]", g_nPortalLimitTime);
				return false;
			}
		}
		//END_PREVENT_REFINE_HACK

		//PREVENT_ITEM_COPY
		{
			if (iPulse - GetMyShopTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;438;%d]", g_nPortalLimitTime);
				return false;
			}

		}
		//END_PREVENT_ITEM_COPY

#ifdef ENABLE_MAILBOX
		{
			if (iPulse - GetMyMailBoxTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, "You cannot use a Return Scroll %d seconds after opening a mailbox.", g_nPortalLimitTime);
				return false;
			}
		}
#endif

		//Return distance check
		//if (item->GetVnum() != ITEM_MARRIAGE_RING)
		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TALISMAN)
		{
			PIXEL_POSITION posWarp;

			int x = 0;
			int y = 0;

			double nDist = 0;
			constexpr double nDistant = 5000.0;
			//Return memory
			if (item->GetVnum() == 22010)	// Schriftrolle des Ortes
			{
				x = item->GetSocket(0) - GetX();
				y = item->GetSocket(1) - GetY();
			}
			//Return
			else if (item->GetVnum() == 22000)	// Schriftrolle Stadt
			{
				SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp);

				if (item->GetSocket(0) == 0)
				{
					x = posWarp.x - GetX();
					y = posWarp.y - GetY();
				}
				else
				{
					x = item->GetSocket(0) - GetX();
					y = item->GetSocket(1) - GetY();
				}
			}

			nDist = sqrt(pow((float)x, 2) + pow((float)y, 2));

			if (nDistant > nDist)
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;439]");
				if (test_server)
					ChatPacket(CHAT_TYPE_INFO, "PossibleDistant %f nNowDist %f", nDistant, nDist);
				return false;
			}
		}

		//PREVENT_PORTAL_AFTER_EXCHANGE
		//Time check after exchange
		if (iPulse - GetExchangeTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;440;%d]", g_nPortalLimitTime);
			return false;
		}
		//END_PREVENT_PORTAL_AFTER_EXCHANGE

	}

	// Check the limit of the transaction window when using the silk
	if ((item->GetVnum() == 50200) || (item->GetVnum() == 71049))
	{
		if (IsShop() || GetOpenedWindow(W_EXCHANGE | W_SHOP_OWNER | W_SAFEBOX | W_CUBE
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
			// Missing CHANGE_LOOK ???
		))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1440]");
			return false;
		}
	}
	//END_PREVENT_TRADE_WINDOW

	// @fixme150 BEGIN
	if (quest::CQuestManager::Instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot use this item if you're using quests"));
		return false;
	}
	// @fixme150 END

	if (IS_SET(item->GetFlag(), ITEM_FLAG_LOG)) // Handling items that leave usage logs
	{
		const uint32_t vid = item->GetVID();
		const uint32_t oldCount = item->GetCount();
		const uint32_t vnum = item->GetVnum();

		char hint[ITEM_NAME_MAX_LEN + 32 + 1];
		int len = snprintf(hint, sizeof(hint) - 32, "%s", item->GetName());

		if (len < 0 || len >= (int)sizeof(hint) - 32)
			len = (sizeof(hint) - 32) - 1;

		const bool ret = UseItemEx(item, DestCell);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		if (ret and item->GetType() == ITEM_USE or ret and item->GetType() == ITEM_SKILLBOOK or ret and item->GetType() == ITEM_GIFTBOX)
			UpdateExtBattlePassMissionProgress(BP_ITEM_USE, 1, item->GetVnum());
#endif

		if (nullptr == ITEM_MANAGER::Instance().FindByVID(vid)) // The item was deleted from UseItemEx. Leave a log for deletion
		{
			LogManager::Instance().ItemLog(this, vid, vnum, "REMOVE", hint);
		}
		else if (oldCount != item->GetCount())
		{
			snprintf(hint + len, sizeof(hint) - len, " %u", oldCount - 1);
			LogManager::Instance().ItemLog(this, vid, vnum, "USE_ITEM", hint);
		}
		return (ret);
	}
	else
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	{
		bool ret = UseItemEx(item, DestCell);

		if (ret and item->GetType() == ITEM_USE or ret and item->GetType() == ITEM_SKILLBOOK or ret and item->GetType() == ITEM_GIFTBOX)
			UpdateExtBattlePassMissionProgress(BP_ITEM_USE, 1, item->GetVnum());

		return (ret);
	}
#else
		return UseItemEx(item, DestCell);
#endif
}

bool CHARACTER::DropItem(TItemPos Cell, uint8_t bCount)
{
	LPITEM item = nullptr;

	if (!CanHandleItem())
	{
		if (nullptr != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, "[LS;1069]");

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, "[LS;1346]");
#endif
		return false;
	}

#ifdef ENABLE_NEWSTUFF
	if (0 != g_ItemDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastItemDropTime + g_ItemDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot drop Yang yet"));
			return false;
		}
	}

	m_dwLastItemDropTime = get_dword_time();
#endif

	if (IsDead())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->isLocked())
		return false;

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1103]");
		return false;
	}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	if (item->IsBasicItem())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	if (quest::CQuestManager::Instance().GetPCForce(GetPlayerID())->IsRunning())
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP | ITEM_ANTIFLAG_GIVE))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;442]");
		return false;
	}

#ifdef ENABLE_GM_BLOCK
	if (IsGM() && !GetStaffPermissions())
	{
		ChatPacket(CHAT_TYPE_INFO, "[GM] GM's können keine Gegenstände fallen lassen!");
		return false;
	}
#endif

	if (bCount == 0 || bCount > item->GetCount())
		bCount = item->GetCount();

	// Deleted from Quickslot
	if (item->GetWindow() == INVENTORY)
		SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(Cell.cell), QUICKSLOT_MAX_POS);
	else if (item->GetWindow() == BELT_INVENTORY)
		SyncQuickslot(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(Cell.cell), QUICKSLOT_MAX_POS);

	LPITEM pkItemToDrop;

	if (bCount == item->GetCount())
	{
		item->RemoveFromCharacter();
		pkItemToDrop = item;
	}
	else
	{
		if (bCount == 0)
		{
			if (test_server)
				sys_log(0, "[DROP_ITEM] drop item count == 0");
			return false;
		}

		item->SetCount(item->GetCount() - bCount);
		ITEM_MANAGER::Instance().FlushDelayedSave(item);

		pkItemToDrop = ITEM_MANAGER::Instance().CreateItem(item->GetVnum(), bCount);

		// copy item socket -- by mhh
		FN_copy_item_socket(pkItemToDrop, item);

		char szBuf[51 + 1];
		snprintf(szBuf, sizeof(szBuf), "%u %u", pkItemToDrop->GetID(), pkItemToDrop->GetCount());
		LogManager::Instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
	}

	const PIXEL_POSITION pxPos = GetXYZ();

	if (pkItemToDrop->AddToGround(GetMapIndex(), pxPos))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;443]");
#ifdef ENABLE_NEWSTUFF
		pkItemToDrop->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]);
#else
		pkItemToDrop->StartDestroyEvent();
#endif

		ITEM_MANAGER::Instance().FlushDelayedSave(pkItemToDrop);

		char szHint[32 + 1];
		snprintf(szHint, sizeof(szHint), "%s %u %u", pkItemToDrop->GetName(), pkItemToDrop->GetCount(), pkItemToDrop->GetOriginalVnum());
		LogManager::Instance().ItemLog(this, pkItemToDrop, "DROP", szHint);
		//Motion(MOTION_PICKUP);
	}

	return true;
}

#ifdef ENABLE_DESTROY_SYSTEM
bool CHARACTER::RemoveItem(const TItemPos &Cell, uint8_t bCount)
{
	LPITEM item = nullptr;

	if (!CanHandleItem())
	{
		if (nullptr != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, "[LS;1069]");

#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot destroy items while modifying your pet's stats.")); // unofficial text
#endif

		return false;
	}

	if (IsDead())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->isLocked())
		return false;

#	ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't destroy this item because is binded!"));
		return false;
	}
#	endif

#	ifdef BLOCK_DESTRY_ATTR_ON_BASIC
	if (item->IsBasicItem())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#	endif

	if (quest::CQuestManager::Instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return false;

	if (item->GetCount() <= 0)
		return false;

	// Deleted from Quickslot
	if (item->GetWindow() == INVENTORY)
		SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(Cell.cell), QUICKSLOT_MAX_POS);
	else if (item->GetWindow() == BELT_INVENTORY)
		SyncQuickslot(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(Cell.cell), QUICKSLOT_MAX_POS);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	UpdateExtBattlePassMissionProgress(BP_ITEM_DESTROY, 1, item->GetVnum());
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (item->GetType() == ITEM_PET && item->GetSubType() == PET_UPBRINGING)
		ITEM_MANAGER::Instance().DestroyItem(item);
	else
		ITEM_MANAGER::Instance().RemoveItem(item, "DESTROY");
#else
	ITEM_MANAGER::Instance().RemoveItem(item);
#endif

	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have destroyed %s."), item->GetName());
	return true;
}
#endif

bool CHARACTER::DropGold(int gold)
{
#ifdef ENABLE_DROP_GOLD
	if (gold <= 0 || gold > GetGold())
		return false;

	if (!CanHandleItem())
		return false;

	if (0 != g_GoldDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastGoldDropTime + g_GoldDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[#Unk]You cannot drop Yang yet"));
			return false;
		}
	}

	m_dwLastGoldDropTime = get_dword_time();

	LPITEM item = ITEM_MANAGER::Instance().CreateItem(1, gold);

	if (item)
	{
		const PIXEL_POSITION pos = GetXYZ();

		if (item->AddToGround(GetMapIndex(), pos))
		{
			//Motion(MOTION_PICKUP);
			PointChange(POINT_GOLD, -gold, true);

			if (gold > DROPABLE_GOLD_LIMIT) // Records over 1,000 won.
				LogManager::Instance().CharLog(this, gold, "DROP_GOLD", "");

#ifdef ENABLE_NEWSTUFF
			item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD]);
#else
			item->StartDestroyEvent();
#endif
			ChatPacket(CHAT_TYPE_INFO, "[LS;1057;%d]", 150 / 60);
		}

		Save();
		return true;
	}
#else
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't drop gold."));
#endif

	return false;
}

bool CHARACTER::MoveItem(TItemPos Cell, TItemPos DestCell, uint8_t count)
{
	LPITEM item = nullptr;

	if (!IsValidItemPosition(Cell))
		return false;

	if (!(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	if (INVENTORY == Cell.window_type && Cell.cell >= GetExtendInvenMax() && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
#else
	if (INVENTORY == Cell.window_type && Cell.cell >= INVENTORY_MAX_NUM && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
#endif
		return false;

	if (item->isLocked())
		return false;

	if (!IsValidItemPosition(DestCell))
		return false;

	if (!CanHandleItem())
	{
		if (nullptr != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, "[LS;1069]");
#ifdef ENABLE_AURA_SYSTEM
		if (IsAuraRefineWindowOpen())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Aura] You cannot move items until the aura window is still opened."));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		if (IsChangeLookWindowOpen())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<¾Æ¿ì¶ó> ¿À¶ó Ã¢ÀÌ ¿­·ÁÀÖÀ» ¶§±îÁö Ç×¸ñÀ» ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
#endif
#if defined(ENABLE_GROWTH_PET_SYSTEM) && defined(ENABLE_PET_ATTR_DETERMINE)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, "[LS;1347]");
#endif
		return false;
	}

	// At the request of the planner, only certain types of items can be placed in the belt inventory.
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	if (DestCell.IsBeltInventoryPosition() && !DestCell.IsAdditionalEquipment1())
#else
	if (DestCell.IsBeltInventoryPosition())
#endif
	{
		if (false == CBeltInventoryHelper::CanMoveIntoBeltInventory(item))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1097]");
			return false;
		}

		LPITEM beltItem = GetWear(WEAR_BELT);
		if (nullptr == beltItem)
			return false;

		if (false == CBeltInventoryHelper::IsAvailableCell(DestCell.cell, beltItem->GetValue(0)))
			return false;
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	if (item->GetSpecialInventoryType() == -1 && DestCell.IsSpecialInventoryPosition())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SPECIAL_INVENTORY_CANNOT_PLACE_HERE"));
		return false;
	}

	if ((INVENTORY == Cell.window_type) && (item->GetSpecialInventoryType() != DestCell.GetSpecialInventoryType()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SPECIAL_INVENTORY_CANNOT_PLACE_HERE"));
		return false;
	}
#endif

#ifdef ENABLE_SWITCHBOT
	if (Cell.IsSwitchbotPosition() && CSwitchbotManager::Instance().IsActive(GetPlayerID(), Cell.cell))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot move active switchbot item."));
		return false;
	}

	if (DestCell.IsSwitchbotPosition() && !SwitchbotHelper::IsValidItem(item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Invalid item type for switchbot."));
		return false;
	}
#endif

	// When moving an item that is already worn to another location, check if it is possible to 'Remove the Book' and move it
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	if (Cell.IsEquipPosition() || Cell.IsAdditionalEquipment1())
#else
	if (Cell.IsEquipPosition())
#endif
	{
		if (!CanUnequipNow(item))
			return false;

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
		const int iWearCell = item->FindEquipCell(this);
		if (iWearCell == WEAR_WEAPON)
		{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			LPITEM costumeWeapon = GetWearDefault(WEAR_COSTUME_WEAPON);
#else
			LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
#endif
			if (costumeWeapon && !UnequipItem(costumeWeapon))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return false;
			}

			if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell))
				return UnequipItem(item);
		}
#endif
	}

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	if (DestCell.IsEquipPosition() || DestCell.IsAdditionalEquipment1())
#else
	if (DestCell.IsEquipPosition())
#endif
	{
		if (GetItem(DestCell)) // In the case of equipment, only one location may be inspected.
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1092]");

			return false;
		}

		EquipItem(item, DestCell.cell);
	}
	else
	{
		if (item->IsDragonSoul())
		{
			if (item->IsEquipped())
			{
				return DSManager::Instance().PullOut(this, DestCell, item);
			}
			else
			{
				if (DestCell.window_type != DRAGON_SOUL_INVENTORY)
					return false;

				if (!DSManager::Instance().IsValidCellForThisItem(item, DestCell))
					return false;
			}
		}
		// Other items than DragonSoulStone cannot enter DragonSoulStone Inventory.
		else if (DRAGON_SOUL_INVENTORY == DestCell.window_type)
			return false;

		LPITEM item2;

		if ((item2 = GetItem(DestCell)) && item != item2 && item2->IsStackable() &&
			!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
			item2->GetVnum() == item->GetVnum() && !item2->IsExchanging()) //@fixme444	- For items that can be combined
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;
			}

			if (count == 0)
				count = item->GetCount();

			sys_log(0, "%s: ITEM_STACK %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			count = MIN(g_bItemCountLimit - item2->GetCount(), count);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
			if (item2->GetVnum() == 79505 && item2->GetCount() >= 24)
			{
				const LPITEM& OkeyCardSet = AutoGiveItem(79506);
				if (OkeyCardSet != nullptr)
					item2->SetCount(item2->GetCount() - 24);
			}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
			if (item2->GetVnum() == CATCH_KING_DROP_ITEM && item2->GetCount() >= 25)
			{
				const LPITEM& KingDeck = AutoGiveItem(CATCH_KING_PLAY_ITEM);
				if (KingDeck != nullptr)
					item2->SetCount(item2->GetCount() - 25);
			}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
			if (item2->GetVnum() == FIND_M_DROP_ITEM && item2->GetCount() >= 27)
			{
				const LPITEM& MemoSet = AutoGiveItem(FIND_M_PLAY_ITEM);
				if (MemoSet != nullptr)
					item2->SetCount(item2->GetCount() - 27);
			}
#endif
			return true;
		}

#ifdef ENABLE_SPECIAL_INVENTORY
#ifndef ENABLE_SWAP_SYSTEM
		if (item2 && ((item->GetVnum() != item2->GetVnum()) || (!item->IsStackable() || !item2->IsStackable())))
			return false;
#endif

		if (item->GetSpecialInventoryType() != -1)
		{
#ifdef ENABLE_SWAP_SYSTEM
			if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell)) //It's not empty - Let's try swapping.
			{
				if (count != 0 && count != item->GetCount()) //Can't swap if not the item as a whole is being moved
					return false;

				if (!DestCell.IsSpecialInventoryPosition() || !Cell.IsSpecialInventoryPosition()) //Only this kind of swapping on inventory
					return false;

				LPITEM targetItem = GetItem_NEW(DestCell);

				if (!targetItem)
					return false;

				if (targetItem && targetItem->GetVID() == item->GetVID()) //Can't swap over my own slots
					return false;

				if (targetItem)
				{
					DestCell = TItemPos(INVENTORY, targetItem->GetCell());
				}

				if (targetItem->isLocked())	//@fixme490
					return false;

				if (item->IsExchanging() || (targetItem && targetItem->IsExchanging()))
					return false;

				uint8_t basePage = DestCell.cell / (INVENTORY_PAGE_SIZE);
				std::map<uint16_t, LPITEM> moveItemMap;
				uint8_t sizeLeft = item->GetSize();

				for (uint16_t i = 0; i < item->GetSize(); ++i)
				{
					uint16_t cellNumber = DestCell.cell + i * 5;

					uint8_t cPage = cellNumber / (INVENTORY_PAGE_SIZE);
					if (basePage != cPage)
						return false;

					LPITEM mvItem = GetItem(TItemPos(INVENTORY, cellNumber));
					if (mvItem)
					{
						if (mvItem->GetSize() > item->GetSize())
							return false;

						if (mvItem->IsExchanging())
							return false;

						moveItemMap.insert({ Cell.cell + i * 5, mvItem });
						sizeLeft -= mvItem->GetSize();

						if (mvItem->GetSize() > 1)
							i += mvItem->GetSize() - 1; //Skip checking the obviously used cells
					}
					else
						sizeLeft -= 1; //Empty slot
				}

				if (sizeLeft != 0)
					return false;

				//This map will hold cell positions for syncing the quickslots afterwards
				std::map<uint8_t, uint16_t> syncCells; //Quickslot pos -> Target cell.

				//Let's remove the original item
				syncCells.insert({ GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, (uint8_t)item->GetCell()), DestCell.cell });
				item->RemoveFromCharacter();

				for (auto it = moveItemMap.begin(); it != moveItemMap.end(); ++it)
				{
					uint16_t toCellNumber = it->first;
					LPITEM mvItem = it->second;

					syncCells.insert({ GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, (uint8_t)mvItem->GetCell()), toCellNumber });
					mvItem->RemoveFromCharacter();

					SetItem(TItemPos(INVENTORY, toCellNumber), mvItem, true);
				}

				SetItem(DestCell, item, true);

				//Sync quickslots only after all is set
				for (auto& sCell : syncCells)
				{
					TQuickSlot qs;
					qs.type = QUICKSLOT_TYPE_ITEM;
					qs.pos = (uint8_t)sCell.second;

					SetQuickslot(sCell.first, qs);
				}

				return true;
			}
#else
			if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell)) {
				return false;
			}
#endif
		}
		else
		{
#endif
#ifdef ENABLE_SWAP_SYSTEM
			if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell)) //It's not empty - Let's try swapping.
			{
				if (count != 0 && count != item->GetCount()) //Can't swap if not the item as a whole is being moved
					return false;

				if (!DestCell.IsDefaultInventoryPosition() || !Cell.IsDefaultInventoryPosition()) //Only this kind of swapping on inventory
					return false;

				LPITEM targetItem = GetItem_NEW(DestCell);

				if (!targetItem)
					return false;

				if (targetItem && targetItem->GetVID() == item->GetVID()) //Can't swap over my own slots
					return false;

				if (targetItem) {
					DestCell = TItemPos(INVENTORY, targetItem->GetCell());
				}

				if (targetItem->isLocked())	//@fixme490
					return false;

				if (item->IsExchanging() || (targetItem && targetItem->IsExchanging()))
					return false;

#ifdef ENABLE_SEALBIND_SYSTEM
				if (item->GetType() == ITEM_USE && item->GetSubType() == USE_BIND || item->GetType() == ITEM_USE && item->GetSubType() == USE_UNBIND)
					return false;
#endif

				uint8_t basePage = DestCell.cell / (INVENTORY_PAGE_SIZE);
				std::map<uint16_t, LPITEM> moveItemMap;
				uint8_t sizeLeft = item->GetSize();

				for (uint16_t i = 0; i < item->GetSize(); ++i)
				{
					uint16_t cellNumber = DestCell.cell + i * 5;

					uint8_t cPage = cellNumber / (INVENTORY_PAGE_SIZE);
					if (basePage != cPage)
						return false;

					LPITEM mvItem = GetItem(TItemPos(INVENTORY, cellNumber));
					if (mvItem) {
						if (mvItem->GetSize() > item->GetSize())
							return false;

						if (mvItem->IsExchanging())
							return false;

						moveItemMap.insert({ Cell.cell + i * 5, mvItem });
						sizeLeft -= mvItem->GetSize();

						if (mvItem->GetSize() > 1)
							i += mvItem->GetSize() - 1; //Skip checking the obviously used cells
					}
					else {
						sizeLeft -= 1; //Empty slot
					}
				}

				if (sizeLeft != 0)
					return false;

				//This map will hold cell positions for syncing the quickslots afterwards
				std::map<uint8_t, uint16_t> syncCells; //Quickslot pos -> Target cell.

				//Let's remove the original item
				syncCells.insert({ GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, (uint8_t)item->GetCell()), DestCell.cell });
				item->RemoveFromCharacter();

				for (auto it = moveItemMap.begin(); it != moveItemMap.end(); ++it)
				{
					uint16_t toCellNumber = it->first;
					LPITEM mvItem = it->second;

					syncCells.insert({ GetQuickslotPosition(QUICKSLOT_TYPE_ITEM, (uint8_t)mvItem->GetCell()), toCellNumber });
					mvItem->RemoveFromCharacter();

					SetItem(TItemPos(INVENTORY, toCellNumber), mvItem, true);
				}

				SetItem(DestCell, item, true);

				//Sync quickslots only after all is set
				for (auto& sCell : syncCells)
				{
					TQuickSlot qs;
					qs.type = QUICKSLOT_TYPE_ITEM;
					qs.pos = (uint8_t)sCell.second;

					SetQuickslot(sCell.first, qs);
				}

				return true;
			}
#else
			if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell))
			{
				return false;
			}
#endif
#ifdef ENABLE_SPECIAL_INVENTORY
		}
#endif

		if (count == 0 || count >= item->GetCount() || !item->IsStackable() || IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
		{
			sys_log(0, "%s: ITEM_MOVE %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->RemoveFromCharacter();
			SetItem(DestCell, item, true);

			if (INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(Cell.cell), static_cast<uint8_t>(DestCell.cell));
			else if (BELT_INVENTORY == Cell.window_type && BELT_INVENTORY == DestCell.window_type)
				SyncQuickslot(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(Cell.cell), static_cast<uint8_t>(DestCell.cell));
			else if (INVENTORY == Cell.window_type && BELT_INVENTORY == DestCell.window_type)
				MoveQuickSlotItem(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(Cell.cell), QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(DestCell.cell));
			else if (BELT_INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
				MoveQuickSlotItem(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(Cell.cell), QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(DestCell.cell));
		}
		else if (count < item->GetCount())
		{
#ifdef ENABLE_PULSE_MANAGER
			if (!PulseManager::Instance().IncreaseClock(GetPlayerID(), ePulse::MoveItem, std::chrono::milliseconds(1000)))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1554]");
				return false;
			}
#endif

			sys_log(0, "%s: ITEM_SPLIT %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->SetCount(item->GetCount() - count);
			LPITEM item3 = ITEM_MANAGER::Instance().CreateItem(item->GetVnum(), count);

			// copy socket -- by mhh
			FN_copy_item_socket(item3, item);

			item3->AddToCharacter(this, DestCell);

			char szBuf[51 + 1];
			snprintf(szBuf, sizeof(szBuf), "%u %u %u %u ", item3->GetID(), item3->GetCount(), item->GetCount(), item->GetCount() + item3->GetCount());
			LogManager::Instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
		}
	}

	return true;
}

namespace NPartyPickupDistribute
{
	struct FFindOwnership
	{
		LPITEM item;
		LPCHARACTER owner;

		FFindOwnership(LPITEM item) :
			item(item), owner(nullptr)
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (item->IsOwnership(ch))
				owner = ch;
		}
	};

	struct FCountNearMember
	{
		int total;
		int x, y;

		FCountNearMember(LPCHARACTER center)
			: total(0), x(center->GetX()), y(center->GetY())
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (!ch)
				return;

			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				total += 1;
		}
	};

	struct FMoneyDistributor
	{
		int total;
		LPCHARACTER c;
		int x, y;
		int iMoney;

		FMoneyDistributor(LPCHARACTER center, int iMoney)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iMoney(iMoney)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (!ch)
				return;

			if (ch != c)
			{
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->PointChange(POINT_GOLD, iMoney, true);

					if (iMoney > 1000) // Records over 1,000 won.
						LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().CharLog(ch, iMoney, "GET_GOLD", ""));

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
					if (ch && ch->IsPC())
						CAchievementSystem::Instance().Collect(ch, achievements::ETaskTypes::TYPE_COLLECT_GOLD, iMoney, 0);
#endif
				}
			}
		}
	};
}

void CHARACTER::GiveGold(int iAmount)
{
	if (iAmount <= 0)
		return;

	sys_log(0, "GIVE_GOLD: %s %d", GetName(), iAmount);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();
		if (!pParty)
			return;

		// If there is a party, share it.
		const uint32_t dwTotal = iAmount;
		uint32_t dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnMapMember(funcCountNearMember, GetMapIndex());	//@fixme522

		if (funcCountNearMember.total > 1)
		{
			const uint32_t dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);
			pParty->ForEachOnMapMember(funcMoneyDist, GetMapIndex());	//@fixme522
		}

		PointChange(POINT_GOLD, dwMyAmount, true);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		UpdateExtBattlePassMissionProgress(YANG_COLLECT, dwMyAmount, GetMapIndex());
#endif

		if (dwMyAmount > 1000) // Records over 1,000 won.
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().CharLog(this, dwMyAmount, "GET_GOLD", ""));
	}
	else
	{
		PointChange(POINT_GOLD, iAmount, true);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		UpdateExtBattlePassMissionProgress(YANG_COLLECT, iAmount, GetMapIndex());
#endif

		if (iAmount > 1000) // Records over 1,000 won.
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().CharLog(this, iAmount, "GET_GOLD", ""));

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (IsPC())
			CAchievementSystem::Instance().Collect(this, achievements::ETaskTypes::TYPE_COLLECT_GOLD, iAmount, 0);
#endif
	}
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CHARACTER::GiveCheque(int cheque)
{
	if (cheque <= 0)
		return;

	sys_log(0, "GIVE_CHEQUE: %s %d", GetName(), cheque);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

		uint32_t dwTotal = cheque;
		uint32_t dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnMapMember(funcCountNearMember, GetMapIndex());	//@fixme522

		if (funcCountNearMember.total > 1)
		{
			uint32_t dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);
			pParty->ForEachOnMapMember(funcMoneyDist, GetMapIndex());	//@fixme522
		}

		PointChange(POINT_CHEQUE, dwMyAmount, true);

		if (dwMyAmount > 1000)
		{
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().CharLog(this, dwMyAmount, "GET_CHEQUE", ""));
		}
	}
	else
	{
		PointChange(POINT_CHEQUE, cheque, true);

		if (cheque > 1000)
		{
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::Instance().CharLog(this, cheque, "GET_CHEQUE", ""));
		}
	}
}
#endif

bool CHARACTER::PickupItem(uint32_t dwVID)
{
	if (IsPC() && IsDead())	//@fixme409
		return false;


	LPITEM item = ITEM_MANAGER::Instance().FindByVID(dwVID);

	if (IsObserverMode())
		return false;

	if (!item || !item->GetSectree())
		return false;

	if (item->DistanceValid(this))
	{
		// @fixme150 BEGIN
		// if (item->GetType() == ITEM_QUEST
		//@fixme471
		if (item->GetType() == ITEM_QUEST || IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
#ifdef ENABLE_PET_SYSTEM
			|| (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY)
#endif
			)
		{
			if (quest::CQuestManager::Instance().GetPCForce(GetPlayerID())->IsRunning() == true)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot pickup this item if you're using quests"));
				return false;
			}
		}
		// @fixme150 END

		if (item->IsOwnership(this))
		{
			// If the item you want to give is an elk
			if (item->GetType() == ITEM_ELK)
			{
				GiveGold(item->GetCount());
				item->RemoveFromGround();

				M2_DESTROY_ITEM(item);
				Save();
			}
			// If it's an ordinary item
			else
			{
				if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
					uint8_t bCount = item->GetCount();

					//@fixme414
					for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
					{
						LPITEM item2 = GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							}

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							const uint8_t bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
							if (item2->GetVnum() == 79505 && item2->GetCount() >= 24)
							{
								const LPITEM& OkeyCardSet = AutoGiveItem(79506);
								if (OkeyCardSet != nullptr)
									item2->SetCount(item2->GetCount() - 24);
							}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
							if (item2->GetVnum() == CATCH_KING_DROP_ITEM && item2->GetCount() >= 25)
							{
								const LPITEM& KingDeck = AutoGiveItem(CATCH_KING_PLAY_ITEM);
								if (KingDeck != nullptr)
									item2->SetCount(item2->GetCount() - 25);
							}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
							if (item2->GetVnum() == FIND_M_DROP_ITEM && item2->GetCount() >= 27)
							{
								const LPITEM& MemoSet = AutoGiveItem(FIND_M_PLAY_ITEM);
								if (MemoSet != nullptr)
									item2->SetCount(item2->GetCount() - 27);
							}
#endif

							if (bCount == 0)
							{
#ifdef ENABLE_CHAT_SETTINGS
								ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#else
								ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
								if (item->IsOwnership(this))
									UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, bCount2, item->GetVnum());
#endif
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::Instance().PickupItem(GetPlayerID(), item2);
								return true;
							}
						}
					}

					item->SetCount(bCount);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
					if (item->IsOwnership(this))
						UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, bCount, item->GetVnum());
#endif
					//@fixme414_END

					for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							}

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							const uint8_t bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef ENABLE_CHAT_SETTINGS
								ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#else
								ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
								if (item->IsOwnership(this))
									UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, bCount2, item->GetVnum());
#endif
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::Instance().PickupItem (GetPlayerID(), item2);
								return true;
							}
						}
					}

#ifdef ENABLE_SPECIAL_INVENTORY
					for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
					{
						LPITEM item2 = GetInventoryItem(i);

						if (!item2)
						{
							continue;
						}

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							}

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							const uint8_t bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef ENABLE_CHAT_SETTINGS
								ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#else
								ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#endif
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::Instance().PickupItem(GetPlayerID(), item2);

								return true;
							}
						}
					}
#endif

					item->SetCount(bCount);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
					if (item->IsOwnership(this))
						UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, bCount, item->GetVnum());
#endif
				}

				int iEmptyCell = -1;
				if (item->IsDragonSoul())
				{
					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						sys_log(0, "No empty ds inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
						return false;
					}
				}
				else
				{
#ifdef ENABLE_SPECIAL_INVENTORY
					if ((iEmptyCell = GetEmptyInventory(item)) == -1)
#else
					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
#endif
					{
						sys_log(0, "No empty inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
						return false;
					}
				}

				item->RemoveFromGround();

				if (item->IsDragonSoul())
					item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
				else
					item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell));

				char szHint[32 + 1];
				snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
				LogManager::Instance().ItemLog(this, item, "GET", szHint);
#ifdef ENABLE_CHAT_SETTINGS
				ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#else
				ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#endif

				//if (item->GetType() == ITEM_QUEST)
				//@fixme471
				if (item->GetType() == ITEM_QUEST || IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
#ifdef ENABLE_PET_SYSTEM
					|| (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY)
#endif
					)
					quest::CQuestManager::Instance().PickupItem(GetPlayerID(), item);
			}

			//Motion(MOTION_PICKUP);
			return true;
		}
		else if (!IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_DROP) && GetParty())
		{
			// If you want to give another party member ownership item
			NPartyPickupDistribute::FFindOwnership funcFindOwnership(item);
			GetParty()->ForEachOnMapMember(funcFindOwnership, GetMapIndex());	//@fixme522

			LPCHARACTER owner = funcFindOwnership.owner;
			// @fixme115
			if (!owner)
				return false;

			int iEmptyCell = -1;

			//@fixme418
			if (owner)
			{
				if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
					uint8_t bCount = item->GetCount();

					for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = owner->GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							}

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							const uint8_t bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
							if (item2->GetVnum() == 79505 && item2->GetCount() >= 24)
							{
								const LPITEM& OkeyCardSet = AutoGiveItem(79506);
								if (OkeyCardSet != nullptr)
									item2->SetCount(item2->GetCount() - 24);
							}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
							if (item2->GetVnum() == CATCH_KING_DROP_ITEM && item2->GetCount() >= 25)
							{
								const LPITEM& KingDeck = AutoGiveItem(CATCH_KING_PLAY_ITEM);
								if (KingDeck != nullptr)
									item2->SetCount(item2->GetCount() - 25);
							}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
							if (item2->GetVnum() == FIND_M_DROP_ITEM && item2->GetCount() >= 27)
							{
								const LPITEM& MemoSet = AutoGiveItem(FIND_M_PLAY_ITEM);
								if (MemoSet != nullptr)
									item2->SetCount(item2->GetCount() - 27);
							}
#endif

							if (bCount == 0)
							{
#ifdef ENABLE_CHAT_SETTINGS
								owner->ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;446;[IN;%d]]", GetName(), item2->GetVnum());
								ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;449;[IN;%d]]", owner->GetName(), item2->GetVnum());
#else
								owner->ChatPacket(CHAT_TYPE_INFO, "[LS;446;[IN;%d]]", GetName(), item2->GetVnum());
								ChatPacket(CHAT_TYPE_INFO, "[LS;449;[IN;%d]]", owner->GetName(), item2->GetVnum());
#endif
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::Instance().PickupItem(owner->GetPlayerID(), item2);
								return true;
							}
						}
					}

#ifdef ENABLE_SPECIAL_INVENTORY
					for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
					{
						LPITEM item2 = owner->GetInventoryItem(i);

						if (!item2)
						{
							continue;
						}

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							}

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							const uint8_t bCount2 = MIN(g_bItemCountLimit - item2->GetCount(), bCount);
							bCount -= bCount2;

							item2->SetCount(item2->GetCount() + bCount2);

							if (bCount == 0)
							{
#ifdef ENABLE_CHAT_SETTINGS
								owner->ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;446;[IN;%d]]", GetName(), item->GetVnum());
								ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;449;[IN;%d]]", owner->GetName(), item->GetVnum());
#else
								owner->ChatPacket(CHAT_TYPE_INFO, "[LS;446;[IN;%d]]", GetName(), item->GetVnum());
								ChatPacket(CHAT_TYPE_INFO, "[LS;449;[IN;%d]]", owner->GetName(), item->GetVnum());
#endif
								M2_DESTROY_ITEM(item);
								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::Instance().PickupItem(owner->GetPlayerID(), item2);
								return true;
							}
						}
					}
#endif
					item->SetCount(bCount);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
					if (item->IsOwnership(this))
						UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, bCount, item->GetVnum());
#endif
				}
			}
			//END_@fixme418

			if (item->IsDragonSoul())
			{
				if (!(owner && (iEmptyCell = owner->GetEmptyDragonSoulInventory(item)) != -1))
				{
					owner = this;

					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						owner->ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
						return false;
					}
				}
			}
			else
			{
#ifdef ENABLE_SPECIAL_INVENTORY
				if (!(owner && (iEmptyCell = owner->GetEmptyInventory(item)) != -1))
#else
				if (!(owner && (iEmptyCell = owner->GetEmptyInventory(item->GetSize())) != -1))
#endif
				{
					owner = this;

#ifdef ENABLE_SPECIAL_INVENTORY
					if ((iEmptyCell = GetEmptyInventory(item)) == -1)
#else
					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
#endif
					{
						owner->ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
						return false;
					}
				}
			}

			item->RemoveFromGround();

			if (item->IsDragonSoul())
				item->AddToCharacter(owner, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
			else
				item->AddToCharacter(owner, TItemPos(INVENTORY, iEmptyCell));

			char szHint[32 + 1];
			snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
			LogManager::Instance().ItemLog(owner, item, "GET", szHint);

			if (owner == this)
			{
#ifdef ENABLE_CHAT_SETTINGS
				ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#else
				ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#endif
			}
			else
			{
#ifdef ENABLE_CHAT_SETTINGS
				owner->ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;446;%s;[IN;%d]]", GetName(), item->GetVnum());
				ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;449;%s;[IN;%d]]", owner->GetName(), item->GetVnum());
#else
				owner->ChatPacket(CHAT_TYPE_INFO, "[LS;446;%s;[IN;%d]]", GetName(), item->GetVnum());
				ChatPacket(CHAT_TYPE_INFO, "[LS;449;%s;[IN;%d]]", owner->GetName(), item->GetVnum());
#endif
			}

			//if (item->GetType() == ITEM_QUEST)
			//@fixme471
			if (item->GetType() == ITEM_QUEST || IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
#ifdef ENABLE_PET_SYSTEM
				|| (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY)
#endif
				)
				quest::CQuestManager::Instance().PickupItem (owner->GetPlayerID(), item);

			return true;
		}
	}

	return false;
}

#ifdef PET_AUTO_PICKUP
bool CHARACTER::PickupItemByPet(uint32_t dwVID)
{
	LPITEM item = ITEM_MANAGER::Instance().FindByVID(dwVID);
	if (IsObserverMode())
		return false;

	if (!item || !item->GetSectree())
		return false;

	if (item->IsOwnership(this))
	{
		if (item->GetType() == ITEM_ELK)
		{
			GiveGold(item->GetCount());
			item->RemoveFromGround();

			M2_DESTROY_ITEM(item);
			Save();
		}
		else
		{
			if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				uint8_t bCount = item->GetCount();

				for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item2 = GetInventoryItem(i);

					if (!item2)
						continue;

					if (item2->GetVnum() == item->GetVnum())
					{
						int j;

						for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							if (item2->GetSocket(j) != item->GetSocket(j))
								break;

						if (j != ITEM_SOCKET_MAX_NUM)
							continue;

						uint8_t bCount2 = MIN(200 - item2->GetCount(), bCount);
						bCount -= bCount2;

						item2->SetCount(item2->GetCount() + bCount2);

						if (bCount == 0)
						{
#ifdef ENABLE_CHAT_SETTINGS
							ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#else
							ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item2->GetVnum());
#endif
							M2_DESTROY_ITEM(item);
							if (item2->GetType() == ITEM_QUEST)
								quest::CQuestManager::Instance().PickupItem(GetPlayerID(), item2);
							return true;
						}
					}
				}

				item->SetCount(bCount);
#ifdef ENABLE_BATTLE_PASS_SYSTEM
				if (item->IsOwnership(this))
					UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, bCount, item->GetVnum());
#endif
			}

			int iEmptyCell;
			if (item->IsDragonSoul())
			{
				if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
				{
					sys_log(0, "No empty ds inventory pid %u size itemid % u", GetPlayerID(), item->GetSize(), item->GetID());
					ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
					return false;
				}
			}
			else
			{
#	ifdef ENABLE_SPECIAL_INVENTORY
				if ((iEmptyCell = GetEmptyInventory(item)) == -1)
#	else
				if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
#	endif
				{
					sys_log(0, "No empty inventory pid %u size itemid % u", GetPlayerID(), item->GetSize(), item->GetID());
					ChatPacket(CHAT_TYPE_INFO, "[LS;445]");
					return false;
				}
			}

			item->RemoveFromGround();

			if (item->IsDragonSoul())
				item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
			else
				item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell));

			char szHint[32 + 1];
			snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
			LogManager::Instance().ItemLog(this, item, "GET", szHint);
#ifdef ENABLE_CHAT_SETTINGS
			ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#else
			ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
			if (item->IsOwnership(this))
				UpdateExtBattlePassMissionProgress(BP_ITEM_COLLECT, item->GetCount(), item->GetVnum());
#endif

			if (item->GetType() == ITEM_QUEST)
				quest::CQuestManager::Instance().PickupItem(GetPlayerID(), item);
		}
		//Motion(MOTION_PICKUP);
		return true;
	}

	return false;
}
#endif

bool CHARACTER::SwapItem(uint16_t wCell, uint16_t wDestCell)	//@fixme519
{
	if (!CanHandleItem())
		return false;

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	TItemPos srcCell(INVENTORY, wCell), destCell(EQUIPMENT, wDestCell);

	if (!CheckAdditionalEquipment(static_cast<uint8_t>(wCell)))
		TItemPos srcCell(INVENTORY, wCell), destCell(EQUIPMENT, wDestCell);
	else
		TItemPos srcCell(INVENTORY, wCell), destCell(ADDITIONAL_EQUIPMENT_1, wDestCell);
#else
	const TItemPos srcCell(INVENTORY, wCell), destCell(EQUIPMENT, wDestCell);
#endif

	// Check for correct cell
	// Dragon Spirit Stone cannot be swapped, so it gets stuck here.
	//if (wCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM || wDestCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM)
	if (srcCell.IsDragonSoulEquipPosition() || destCell.IsDragonSoulEquipPosition())
		return false;

	// If both are in the equipment window, you cannot swap.
	if (srcCell.IsEquipPosition() && destCell.IsEquipPosition())
		return false;

	LPITEM item1 = nullptr, item2 = nullptr;

	// so that item2 is in the equipment window.
	if (srcCell.IsEquipPosition())
	{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		if (!CheckAdditionalEquipment(static_cast<uint8_t>(wDestCell)))
			item1 = GetEquipmentItem(wDestCell);
		else
			item1 = GetAdditionalEquipmentItem(wDestCell);
#else
		item1 = GetEquipmentItem(wDestCell);
#endif
		item2 = GetInventoryItem(wCell);
	}
	else
	{
		item1 = GetInventoryItem(wCell);
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		if (!CheckAdditionalEquipment(static_cast<uint8_t>(wDestCell)))
			item2 = GetEquipmentItem(wDestCell);
		else
			item2 = GetAdditionalEquipmentItem(wDestCell);
#else
		item2 = GetEquipmentItem(wDestCell);
#endif
	}

	if (!item1 || !item2)
		return false;

	if (item1 == item2)
	{
		sys_log(0, "[WARNING][WARNING][HACK USER!] : %s %d %d", m_stName.c_str(), wCell, wDestCell);
		return false;
	}

	// Check if item2 can enter the wCell location.
	if (!IsEmptyItemGrid(TItemPos(INVENTORY, item1->GetCell()), item2->GetSize(), item1->GetCell()))
		return false;

	// If the item to be changed is in the equipment window
	if (TItemPos(EQUIPMENT, item2->GetCell()).IsEquipPosition()
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		&& !CheckAdditionalEquipment(static_cast<uint8_t>(item2->GetCell()))
#endif
		)
	{
		const uint8_t bEquipCell = static_cast<uint8_t>(item2->GetCell());
		const uint16_t wInvenCell = item1->GetCell();

		// The item being worn can be removed, and the item to be worn must be in a wearable state.
		if (item2->IsDragonSoul() || item2->GetType() == ITEM_BELT) // @fixme117
		{
			if (false == CanUnequipNow(item2) || false == CanEquipNow(item1))
				return false;
		}

		if (bEquipCell != item1->FindEquipCell(this)) // Allowed only in the same location
			return false;

		item2->RemoveFromCharacter();

		if (item1->EquipTo(this, bEquipCell))
			item2->AddToCharacter(this, TItemPos(INVENTORY, wInvenCell));
		else
			sys_err("SwapItem cannot equip %s! item1 %s", item2->GetName(), item1->GetName());
	}
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	else if (TItemPos(ADDITIONAL_EQUIPMENT_1, item2->GetCell()).IsAdditionalEquipment1() && CheckAdditionalEquipment(static_cast<uint8_t>(item2->GetCell())))
	{
		const uint8_t bEquipCell = static_cast<uint8_t>(item2->GetCell());
		const uint16_t wInvenCell = item1->GetCell();

		// The item being worn can be removed, and the item to be worn must be in a wearable state.
		if (item2->IsDragonSoul() || item2->GetType() == ITEM_BELT) // @fixme117
		{
			if (false == CanUnequipNow(item2) || false == CanEquipNow(item1))
				return false;
		}

		if (bEquipCell != item1->FindEquipCell(this)) // Allowed only in the same location
			return false;

		item2->RemoveFromCharacter();

		if (item1->EquipTo(this, bEquipCell))
			item2->AddToCharacter(this, TItemPos(INVENTORY, wInvenCell));
		else
			sys_err("SwapItem cannot equip %s! item1 %s", item2->GetName(), item1->GetName());
	}
#endif
	else
	{
		const uint16_t wCell1 = item1->GetCell();	//@fixme519
		const uint16_t wCell2 = item2->GetCell();	//@fixme519

		item1->RemoveFromCharacter();
		item2->RemoveFromCharacter();

		item1->AddToCharacter(this, TItemPos(EQUIPMENT, wCell2));
		item2->AddToCharacter(this, TItemPos(INVENTORY, wCell1));
	}

	return true;
}

bool CHARACTER::UnequipItem(LPITEM item)
{
	if (!item)
		return false;

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	const int iWearCell = item->FindEquipCell(this);
	if (iWearCell == WEAR_WEAPON)
	{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		LPITEM costumeWeapon = GetWearDefault(WEAR_COSTUME_WEAPON);
#else
		LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
#endif
		if (costumeWeapon && !UnequipItem(costumeWeapon))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
			return false;
		}
	}
#endif

	if (false == CanUnequipNow(item))
		return false;

	int pos = -1;
	if (item->IsDragonSoul())
		pos = GetEmptyDragonSoulInventory(item);
	else
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		pos = GetEmptyInventory(item);
#else
		pos = GetEmptyInventory(item->GetSize());
#endif
	}

	// HARD CODING
	if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		ShowAlignment(true);

	item->RemoveFromCharacter();
	if (item->IsDragonSoul())
		item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, pos));
	else
		item->AddToCharacter(this, TItemPos(INVENTORY, pos));

	CheckMaximumPoints();

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM // Need Review in Future
	if (item->IsRideItem())
	{
		MountVnum(GetPoint(POINT_MOUNT));
		ComputePoints();
	}
#endif

	return true;
}

//
// @version 05/07/05 Bang2ni-Do not wear equipment within 1.5 seconds after using the skill
//
bool CHARACTER::EquipItem(LPITEM item, int iCandidateCell)
{
	if (item->IsExchanging())
		return false;

	if (!item->IsEquipable())
		return false;

	if (!CanEquipNow(item))
		return false;

	const int iWearCell = item->FindEquipCell(this, iCandidateCell);

	if (iWearCell < 0)
		return false;

	// Do not wear a tuxedo while wearing something
	if (iWearCell == WEAR_BODY && IsRiding() && (item->GetVnum() >= 11901 && item->GetVnum() <= 11904))	// Marriage Armors
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;883]");
		return false;
	}

	//@fixme450
	if (iWearCell == WEAR_WEAPON && IsRiding() && (item->GetVnum() == 50201)) {	// Marriage Weapon
	//if (iWearCell == WEAR_WEAPON && IsRiding() && (item->GetType() == ITEM_WEAPON && item->GetSubType() == WEAPON_BOUQUET)) {	// Marriage Weapon
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("asdf"));
		return false;
	}
	//@end_fixme450

	if (iWearCell != WEAR_ARROW && IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;450]");
		return false;
	}

	if (FN_check_item_sex(this, item) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1005]");
		return false;
	}

#ifdef ENABLE_PULSE_MANAGER
		if (!PulseManager::Instance().IncreaseClock(GetPlayerID(), ePulse::EquipItem, std::chrono::milliseconds(1000))) {
			ChatPacket(CHAT_TYPE_INFO, "Remaining time: %.2f", PULSEMANAGER_CLOCK_TO_SEC2(GetPlayerID(), ePulse::EquipItem));
			return false;
		}
#endif

	if ((item->IsHorseSummonItem() || item->IsRideItem())
#	ifdef DISABLE_RIDE_ON_SPECIAL_MAP
		&& !IS_MOUNTABLE_ZONE(GetMapIndex())
#	endif
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot ride here"));
		return false;
	}

#ifdef ENABLE_MOUNT_CHECK
	// If you want to use a wild super mount and your horse is not armed.
	if ((GetHorseLevel() < 11) && (
		(item->GetVnum() >= 52006 && item->GetVnum() <= 52010) || (item->GetVnum() >= 52021 && item->GetVnum() <= 52025) ||
		(item->GetVnum() >= 52036 && item->GetVnum() <= 52040) || (item->GetVnum() >= 52051 && item->GetVnum() <= 52055) ||
		(item->GetVnum() >= 52066 && item->GetVnum() <= 52070) || (item->GetVnum() >= 52081 && item->GetVnum() <= 52085) ||
		(item->GetVnum() >= 52096 && item->GetVnum() <= 52100) || (item->GetVnum() >= 52111 && item->GetVnum() <= 52115)
		))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must have an armed horse to be able to ride this mount."));
		return false;
	}

	// If you want to use a super brave mount and your horse is not military.
	if ((GetHorseLevel() < 21) && (
		(item->GetVnum() >= 52011 && item->GetVnum() <= 52015) || (item->GetVnum() >= 52026 && item->GetVnum() <= 52030) ||
		(item->GetVnum() >= 52041 && item->GetVnum() <= 52045) || (item->GetVnum() >= 52056 && item->GetVnum() <= 52060) ||
		(item->GetVnum() >= 52071 && item->GetVnum() <= 52075) || (item->GetVnum() >= 52086 && item->GetVnum() <= 52090) ||
		(item->GetVnum() >= 52101 && item->GetVnum() <= 52105) || (item->GetVnum() >= 52116 && item->GetVnum() <= 52120)
		))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must have a military horse to be able to ride this mount."));
		return false;
	}
#endif

#ifdef ENABLE_GLOVE_SYSTEM
	if ((item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_GLOVE) &&
		(GetQuestFlag("glove_system.can_equip_glove") == 0))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1677]");
		return false;
	}
#endif

	// Except for arrows, equipment can be replaced after the last attack time or 1.5 of skill use
	const uint32_t dwCurTime = get_dword_time();

#ifdef ENABLE_NO_WAIT_TO_RIDE
	if ((iWearCell != WEAR_ARROW && item->GetSubType() != COSTUME_MOUNT)
#else
	if (iWearCell != WEAR_ARROW
#endif
		&& (dwCurTime - GetLastAttackTime() <= 1500 || dwCurTime - m_dwLastSkillTime <= 1500))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;451]");
		return false;
	}

	//When using a new mount, check whether an existing horse is used
	if (item->IsRideItem())
	{
		if (IsRiding())
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1054]");
			return false;
		}

#if defined(ENABLE_MOUNT_PROTO_AFFECT_SYSTEM) && defined(DISABLE_RIDE_ON_SPECIAL_MAP)
		if (!IS_MOUNTABLE_ZONE(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot ride here"));
			return false;
		}

#ifdef ENABLE_PULSE_MANAGER
		if (!PulseManager::Instance().IncreaseClock(GetPlayerID(), ePulse::RideMount, std::chrono::milliseconds(1000))) {
			ChatPacket(CHAT_TYPE_INFO, "Remaining time: %.2f", PULSEMANAGER_CLOCK_TO_SEC2(GetPlayerID(), ePulse::EquipItem));
			return false;
		}
#endif
#endif
	}

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (iWearCell == WEAR_WEAPON)
	{
		if (item->GetType() == ITEM_WEAPON)
		{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			LPITEM costumeWeapon = GetWearDefault(WEAR_COSTUME_WEAPON);
#else
			LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
#endif
			if (costumeWeapon && costumeWeapon->GetValue(3) != item->GetSubType() && !UnequipItem(costumeWeapon))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return false;
			}
		}
		else //fishrod/pickaxe
		{
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
			LPITEM costumeWeapon = GetWearDefault(WEAR_COSTUME_WEAPON);
#else
			LPITEM costumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
#endif
			if (costumeWeapon && !UnequipItem(costumeWeapon))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot unequip the costume weapon. Not enough space."));
				return false;
			}
		}
	}
	else if (iWearCell == WEAR_COSTUME_WEAPON)
	{
		if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_WEAPON)
		{
			const LPITEM& pkWeapon = GetWear(WEAR_WEAPON);
			if (!pkWeapon || pkWeapon->GetType() != ITEM_WEAPON || item->GetValue(3) != pkWeapon->GetSubType())
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;1220]");
				return false;
			}
		}
	}
#endif

	// Dragon Soul Stone Special Treatment
	if (item->IsDragonSoul())
	{
		// It cannot be worn if the same type of dragon spirit stone is already in it.
		// Dragon Soul Stone should not support swap.
		if (GetEquipmentItem(iWearCell))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;1090]");
			return false;
		}

		if (!item->EquipTo(this, iWearCell))
			return false;
	}
	// It is not a dragon soul stone.
	else
	{
		// If there is an item in place to wear,
		if (GetWear(iWearCell) && !IS_SET(GetWear(iWearCell)->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		{
			// Once stuck, this item cannot be changed. swap is also completely impossible
			if (item->GetWearFlag() == WEARABLE_ABILITY)
				return false;

			if (!SwapItem(item->GetCell(), iWearCell))
				return false;
		}
		else
		{
			const uint8_t bOldCell = static_cast<uint8_t>(item->GetCell());

			if (item->EquipTo(this, iWearCell))
			{
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, bOldCell, iWearCell);
			}
		}
	}

	if (true == item->IsEquipped())
	{
		// After the initial use of the item, time is deducted even if it is not used
		if (-1 != item->GetProto()->cLimitRealTimeFirstUseIndex)
		{
			// Whether the item has been used even once is determined by looking at Socket1. (Record the number of uses in Socket1)
			if (0 == item->GetSocket(1))
			{
				// Use the limit value as the default value for the available time, but if there is a value in Socket0, use that value. (Unit is seconds)
				long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[(uint8_t)(item->GetProto()->cLimitRealTimeFirstUseIndex)].lValue;

				if (0 == duration)
					duration = 60 * 60 * 24 * 7;

				item->SetSocket(0, time(0) + duration);
				item->StartRealTimeExpireEvent();
			}

			item->SetSocket(1, item->GetSocket(1) + 1);
		}

		if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
			ShowAlignment(false);

		const uint32_t& dwVnum = item->GetVnum();

		if (CItemVnumHelper::IsRamadanMoonRing(dwVnum))
			this->EffectPacket(SE_EQUIP_RAMADAN_RING);
		else if (CItemVnumHelper::IsHalloweenCandy(dwVnum))
			this->EffectPacket(SE_EQUIP_HALLOWEEN_CANDY);
		else if (CItemVnumHelper::IsHappinessRing(dwVnum))
			this->EffectPacket(SE_EQUIP_HAPPINESS_RING);
		else if (CItemVnumHelper::IsLovePendant(dwVnum))
			this->EffectPacket(SE_EQUIP_LOVE_PENDANT);
		else if (CItemVnumHelper::IsSpeedBoots(dwVnum))			// Windschuhe
			this->EffectPacket(SE_EQUIP_BOOTS);
		else if (CItemVnumHelper::IsHeromedal(dwVnum))			// Heldenmedaille
			this->EffectPacket(SE_EQUIP_HEROMEDAL);
		else if (CItemVnumHelper::IsChocolatePendant(dwVnum))	// Schoko-Amulett
			this->EffectPacket(SE_EQUIP_CHOCOLATE_AMULET);
		else if (CItemVnumHelper::IsEmotionMask(dwVnum))		// Emotionsmask
			this->EffectPacket(SE_EQUIP_EMOTION_MASK);
		else if (CItemVnumHelper::IsWillPowerRing(dwVnum))		// Ring der Willenskraft
			this->EffectPacket(SE_EQUIP_WILLPOWER_RING);
		else if (CItemVnumHelper::IsDeadlyPowerRing(dwVnum))	// Ring der tödlichen Macht
			this->EffectPacket(SE_EQUIP_DEADLYPOWER_RING);
		else if (CItemVnumHelper::IsEasterCandyEqip(dwVnum))	// Lolli der Magie
			this->EffectPacket(SE_EQUIP_EASTER_CANDY_EQIP);

		// In the case of ITEM_UNIQUE, it is defined in SpecialItemGroup, and (item->GetSIGVnum() != nullptr)
		else if (ITEM_UNIQUE == item->GetType() && 0 != item->GetSIGVnum())
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (nullptr != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::Instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (nullptr != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}

		// In the case of ITEM_RING, it is defined in SpecialItemGroup, and (item->GetSIGVnum() != nullptr)
		else if (ITEM_RING == item->GetType() && 0 != item->GetSIGVnum())	//@fixme400
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (nullptr != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::Instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (nullptr != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}

#ifdef ENABLE_PET_SYSTEM
		if (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY)
			quest::CQuestManager::Instance().UseItem(GetPlayerID(), item, false);
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		else if ((item->GetType() == ITEM_COSTUME) && (item->GetSubType() == COSTUME_ACCE))
		{
			if (item->GetSocket(1) > 18)
				this->EffectPacket(SE_ACCE_BACK);
			this->EffectPacket(SE_ACCE_EQUIP);
		}
#endif
	}

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM // Need review in future
	if (item->IsRideItem())
	{
		MountVnum(GetPoint(POINT_MOUNT));
		ComputePoints();
	}
#endif

	return true;
}

void CHARACTER::BuffOnAttr_AddBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		const TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->AddBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		const TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->RemoveBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_ClearAll()
{
	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); it++)
	{
		CBuffOnAttributes* pBuff = it->second;
		if (pBuff)
		{
			pBuff->Initialize();
		}
	}
}

void CHARACTER::BuffOnAttr_ValueChange(uint16_t wType, uint8_t bOldValue, uint8_t bNewValue)	//@fixme532
{
	const TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(wType);

	if (0 == bNewValue)
	{
		if (m_map_buff_on_attrs.end() == it)
			return;
		else
			it->second->Off();
	}
	else if (0 == bOldValue)
	{
		CBuffOnAttributes* pBuff = nullptr;
		if (m_map_buff_on_attrs.end() == it)
		{
			switch (wType)
			{
				case POINT_ENERGY:
				{
					static uint8_t abSlot[] =
					{
						WEAR_BODY,
						WEAR_HEAD,
						WEAR_FOOTS,
						WEAR_WRIST,
						WEAR_WEAPON,
						WEAR_NECK,
						WEAR_EAR,
						WEAR_SHIELD,
#ifdef ENABLE_PENDANT
						WEAR_PENDANT,
#endif
					};

					static std::vector <uint8_t> vec_slots(abSlot, abSlot + _countof(abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, wType, &vec_slots);
				}
				break;

				case POINT_COSTUME_ATTR_BONUS:
				{
					static uint8_t abSlot[] =
					{
						WEAR_COSTUME_BODY,
						WEAR_COSTUME_HAIR,
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
						WEAR_COSTUME_WEAPON,
#endif
					};
					static std::vector <uint8_t> vec_slots(abSlot, abSlot + _countof(abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, wType, &vec_slots);
				}
				break;

				default:
					break;
			}
			m_map_buff_on_attrs.insert(TMapBuffOnAttrs::value_type(wType, pBuff));

		}
		else
			pBuff = it->second;
		if (pBuff != nullptr)
			pBuff->On(bNewValue);
	}
	else
	{
		assert(m_map_buff_on_attrs.end() != it);
		it->second->ChangeBuffValue(bNewValue);
	}
}


#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
LPITEM CHARACTER::FindSpecifyItem(uint32_t vnum, bool bIgnoreSetValue) const
#else
LPITEM CHARACTER::FindSpecifyItem(uint32_t vnum) const
#endif
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
		{
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
			if (bIgnoreSetValue && GetInventoryItem(i)->GetItemSetValue())
				continue;
#endif

			return GetInventoryItem(i);
		}
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
	{
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
			return GetInventoryItem(i);
	}
#endif

	return nullptr;
}

LPITEM CHARACTER::FindItemByID(uint32_t id) const
{
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (nullptr != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);
	}

	for (int i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
	{
		if (nullptr != GetBeltInventoryItem(i) && GetBeltInventoryItem(i)->GetID() == id)
			return GetBeltInventoryItem(i);
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
	{
		if (nullptr != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);
	}
#endif

	return nullptr;
}

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
int CHARACTER::CountSpecifyItem(uint32_t vnum, int iExceptionCell, bool bIgnoreSetValue) const	//@fixme491
#else
int CHARACTER::CountSpecifyItem(uint32_t vnum, int iExceptionCell) const	//@fixme491
#endif
{
	int count = 0;
	LPITEM item = nullptr;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (i == iExceptionCell)	//@fixme491
			continue;

		item = GetInventoryItem(i);
		if (nullptr != item && item->GetVnum() == vnum)
		{
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
			if (bIgnoreSetValue && item->GetItemSetValue())
				continue;
#endif

			// If it is an item registered in a personal store, it will be skipped.
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
#ifdef ENABLE_SEALBIND_SYSTEM
			else if (item->IsSealed())
			{
				continue;
			}
#endif
			else
			{
				count += item->GetCount();
			}
		}
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
	{
		item = GetInventoryItem(i);
		if (item && item->GetVnum() == vnum)
		{
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
			if (bIgnoreSetValue && item->GetItemSetValue())
				continue;
#endif

			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
			else
			{
				count += item->GetCount();
			}
		}
	}
#endif

	return count;
}

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
void CHARACTER::RemoveSpecifyItem(uint32_t vnum, uint32_t count, int iExceptionCell, bool bIgnoreSetValue)	//@fixme491
#else
void CHARACTER::RemoveSpecifyItem(uint32_t vnum, uint32_t count, int iExceptionCell)	//@fixme491
#endif
{
	if (0 == count)
		return;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (uint32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (i == iExceptionCell)	//@fixme491
			continue;

		if (nullptr == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
		if (bIgnoreSetValue && GetInventoryItem(i)->GetItemSetValue())
			continue;
#endif

#ifdef ENABLE_SEALBIND_SYSTEM
		if (GetInventoryItem(i)->IsSealed())
			continue;
#endif

		// If it is an item registered in a personal store, it will be skipped. (It is a problem if you enter this part when it is sold in a private store!)
		if (m_pkMyShop)
		{
			const bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (vnum >= 80003 && vnum <= 80007)	// Silberbarren, Goldbarren
			LogManager::Instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
	{
		LPITEM item = GetInventoryItem(i);
		if (item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
			if (bIgnoreSetValue && item->GetItemSetValue())
				continue;
#endif

			if (vnum >= 80003 && vnum <= 80007)
			{
				LogManager::Instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");
			}

			if (count >= GetInventoryItem(i)->GetCount())
			{
				count -= GetInventoryItem(i)->GetCount();
				GetInventoryItem(i)->SetCount(0);

				if (0 == count)
				{
					return;
				}
			}
			else
			{
				GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
				return;
			}
		}
	}
#endif

	// Exception handling is weak.
	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
int CHARACTER::CountSpecifyPetFeedItem(uint32_t vnum, int iExceptionCell) const	//@fixme491
{
	int	count = 0;
	LPITEM item;

	for (int i = 0; i < PET_MAX_FEED_SLOT; ++i)
	{
		if (i == iExceptionCell)	//@fixme491
			continue;

		item = GetInventoryFeedItem(i);
		if (nullptr != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
			else
			{
				count += item->GetCount();
			}
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyPetFeedItem(uint32_t vnum, uint32_t count, int iExceptionCell)	//@fixme491
{
	if (0 == count)
		return;

	for (uint32_t i = 0; i < PET_MAX_FEED_SLOT; ++i)
	{
		if (i == iExceptionCell)	//@fixme491
			continue;

		if (nullptr == GetInventoryFeedItem(i))
			continue;

		if (GetInventoryFeedItem(i)->GetVnum() != vnum)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryFeedItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryFeedItem(i)->GetCount())
		{
			count -= GetInventoryFeedItem(i)->GetCount();
			GetInventoryFeedItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryFeedItem(i)->SetCount(GetInventoryFeedItem(i)->GetCount() - count);
			return;
		}
	}

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyPetFeedItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}
#endif

int CHARACTER::CountSpecifyTypeItem(uint8_t type) const
{
	int count = 0;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		LPITEM pItem = GetInventoryItem(i);
		if (pItem != nullptr && pItem->GetType() == type)
		{
			count += pItem->GetCount();
		}
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
	{
		LPITEM pItem = GetInventoryItem(i);
		if (pItem && pItem->GetType() == type)
		{
			count += pItem->GetCount();
		}
	}
#endif

	return count;
}

void CHARACTER::RemoveSpecifyTypeItem(uint8_t type, uint32_t count)
{
	if (0 == count)
		return;

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (uint32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (nullptr == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetType() != type)
			continue;

		// If it is an item registered in a personal store, it will be skipped. (It is a problem if you enter this part when it is sold in a private store!)
		if (m_pkMyShop)
		{
			const bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#ifdef ENABLE_SPECIAL_INVENTORY
	for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
	{
		if (nullptr == GetInventoryItem(i))
		{
			continue;
		}

		if (GetInventoryItem(i)->GetType() != type)
		{
			continue;
		}

		if (m_pkMyShop)
		{
			const bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
			{
				continue;
			}
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
			{
				return;
			}
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}
#endif
}

void CHARACTER::AutoGiveItem(LPITEM item, bool longOwnerShip)
{
	if (nullptr == item)
	{
		sys_err("nullptr point.");
		return;
	}
	if (item->GetOwner())
	{
		sys_err("item %d 's owner exists!", item->GetID());
		return;
	}

	WORD bCount = item->GetCount();

	if (item->GetType() == ITEM_BLEND)
	{
		for (int i = 0; i < INVENTORY_MAX_NUM; i++)
		{
			LPITEM inv_item = GetInventoryItem(i);

			if (inv_item == NULL)
				continue;

			if (inv_item->GetType() == ITEM_BLEND && inv_item->GetVnum() == item->GetVnum())
			{
				if (inv_item->GetSocket(0) == item->GetSocket(0) && inv_item->GetSocket(1) == item->GetSocket(1) && inv_item->GetSocket(2) == item->GetSocket(2) && inv_item->GetCount() < ITEM_MAX_COUNT)
				{
					inv_item->SetCount(inv_item->GetCount() + item->GetCount());
					M2_DESTROY_ITEM(item);
					return;
				}
			}
		}
	}
	else
	{
		TItemTable* p = ITEM_MANAGER::instance().GetTable(item->GetVnum());
		if (!p)
		{
			M2_DESTROY_ITEM(item);
			return;
		}
		if (p->dwFlags & ITEM_FLAG_STACKABLE)
		{
			for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
			{
				LPITEM invItem = GetInventoryItem(i);
				if (!invItem)
					continue;

				if (invItem->GetVnum() == item->GetVnum() && FN_check_item_socket(invItem))
				{
					if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
					{
						if (bCount < p->alValues[1])
							bCount = p->alValues[1];
					}
					const WORD bCount2 = MIN(ITEM_MAX_COUNT - invItem->GetCount(), bCount);
					bCount -= bCount2;
					invItem->SetCount(invItem->GetCount() + bCount2);
					if (bCount == 0)
					{
						if (invItem->GetType() == ITEM_QUEST || invItem->GetType() == ITEM_MATERIAL)
							quest::CQuestManager::instance().PickupItem(GetPlayerID(), invItem);
						M2_DESTROY_ITEM(item);
						return;
					}
				}
			}
		}
	}

	item->SetCount(bCount);

	int cell = -1;
	if (item->IsDragonSoul())
	{
		cell = GetEmptyDragonSoulInventory(item);
	}
	else
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		cell = GetEmptyInventory(item);
#else
		cell = GetEmptyInventory(item->GetSize());
#endif
	}

	if (cell != -1)
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, cell));
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, cell));

		LogManager::Instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickSlot* pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickSlot slot{};
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = cell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
#ifdef ENABLE_NEWSTUFF
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
#else
		item->StartDestroyEvent();
#endif

		if (longOwnerShip)
			item->SetOwnership(this, OWNERSHIP_EVENT_TIME);
		else
			item->SetOwnership(this, OWNERSHIP_EVENT_TIME / 2);
		LogManager::Instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}
}

LPITEM CHARACTER::AutoGiveItem(uint32_t dwItemVnum, uint8_t bCount, int iRarePct, bool bMsg)
{
	const TItemTable* p = ITEM_MANAGER::Instance().GetTable(dwItemVnum);

	if (!p)
		return nullptr;

	DBManager::Instance().SendMoneyLog(MONEY_LOG_DROP, dwItemVnum, bCount);

	if (p->dwFlags & ITEM_FLAG_STACKABLE && p->bType != ITEM_BLEND)
	{
		for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item = GetInventoryItem(i);

			if (!item)
				continue;

			if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
			{
				if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
				{
					if (bCount < p->alValues[1])
						bCount = static_cast<uint8_t>(p->alValues[1]);
				}

				const uint8_t bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
				bCount -= bCount2;

				item->SetCount(item->GetCount() + bCount2);

				if (bCount == 0)
				{
					if (bMsg)
					{
#ifdef ENABLE_CHAT_SETTINGS
						ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#else
						ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#endif
					}

					return item;
				}
			}
		}

#ifdef ENABLE_SPECIAL_INVENTORY
		for (int i = SPECIAL_INVENTORY_SLOT_START; i < SPECIAL_INVENTORY_SLOT_END; ++i)
		{
			LPITEM item = GetInventoryItem(i);

			if (!item)
			{
				continue;
			}

			if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
			{
				if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
				{
					if (bCount < p->alValues[1])
					{
						bCount = static_cast<uint8_t>(p->alValues[1]);
					}
				}

				const uint8_t bCount2 = MIN(g_bItemCountLimit - item->GetCount(), bCount);
				bCount -= bCount2;
				item->SetCount(item->GetCount() + bCount2);

				if (bCount == 0)
				{
					if (bMsg)
					{
						ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
					}
					return item;
				}
			}
		}
#endif
	}

	LPITEM item = ITEM_MANAGER::Instance().CreateItem(dwItemVnum, bCount, 0, true);

	if (!item)
	{
		sys_err("cannot create item by vnum %u (name: %s)", dwItemVnum, GetName());
		return nullptr;
	}

	if (item->GetType() == ITEM_BLEND)
	{
		for (int i = 0; i < INVENTORY_MAX_NUM; i++)
		{
			LPITEM inv_item = GetInventoryItem(i);

			if (inv_item == nullptr)
				continue;

			if (inv_item->GetType() == ITEM_BLEND)
			{
				if (inv_item->GetVnum() == item->GetVnum())
				{
					if (inv_item->GetSocket(0) == item->GetSocket(0) &&
						inv_item->GetSocket(1) == item->GetSocket(1) &&
						inv_item->GetSocket(2) == item->GetSocket(2) &&
						inv_item->GetCount() < g_bItemCountLimit)
					{
						inv_item->SetCount(inv_item->GetCount() + item->GetCount());
#ifdef ENABLE_MEMORY_LEAK_FIX
						M2_DESTROY_ITEM(item);	//@fixme472
#endif
						return inv_item;
					}
				}
			}
		}
	}

	int iEmptyCell = -1;
	if (item->IsDragonSoul())
	{
		iEmptyCell = GetEmptyDragonSoulInventory(item);
	}
	else
	{
#ifdef ENABLE_SPECIAL_INVENTORY
		iEmptyCell = GetEmptyInventory(item);
#else
		iEmptyCell = GetEmptyInventory(item->GetSize());
#endif
	}

	if (iEmptyCell != -1)
	{
		if (bMsg)
		{
#ifdef ENABLE_CHAT_SETTINGS
			ChatPacket(CHAT_TYPE_ITEM_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#else
			ChatPacket(CHAT_TYPE_INFO, "[LS;444;[IN;%d]]", item->GetVnum());
#endif
		}

		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell));

		LogManager::Instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickSlot* pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickSlot slot{};
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = iEmptyCell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
#ifdef ENABLE_NEWSTUFF
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
#else
		item->StartDestroyEvent();
#endif

		// For items with anti-drop flag,
		// If there is no empty space in the inventory and you have to drop it,
		// Maintain ownership until the item disappears (300 seconds).
		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP))
			item->SetOwnership(this, OWNERSHIP_EVENT_TIME);
		else
			item->SetOwnership(this, OWNERSHIP_EVENT_TIME / 2);
		LogManager::Instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}

	sys_log(0,
		"7: %d %d", dwItemVnum, bCount);
	return item;
}

bool CHARACTER::GiveItem(LPCHARACTER victim, TItemPos Cell)
{
	if (!victim)
		return false;

	if (!CanHandleItem())
		return false;

	// @fixme150 BEGIN
	if (quest::CQuestManager::Instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot take this item if you're using quests"));
		return false;
	}
	// @fixme150 END

	LPITEM item = GetItem(Cell);

	if (item && !item->IsExchanging())
	{
		if (victim->CanReceiveItem(this, item))
		{
			victim->ReceiveItem(this, item);
			return true;
		}
	}

	return false;
}

bool CHARACTER::CanReceiveItem(LPCHARACTER from, LPITEM item) const
{
	if (!from || !item)
		return false;

	if (IsDead()) //@fixme543
		return false;

	if (IsPC())
		return false;

#ifdef ENABLE_SEALBIND_SYSTEM
	if (item->IsSealed())
	{
		from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't upgrade sealbind item."));
		return false;
	}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	if (item->IsBasicItem()) {
		from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ITEM_IS_BASIC_CANNOT_DO"));
		return false;
	}
#endif

	// TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX
	if (DISTANCE_APPROX(GetX() - from->GetX(), GetY() - from->GetY()) > 2000)
		return false;
	// END_OF_TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX

	switch (GetRaceNum())
	{
		case fishing::CAMPFIRE_MOB:
		{
			if (item->GetType() == ITEM_FISH &&
				(item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				return true;
		}
		break;

		case fishing::FISHER_MOB:
		{
			if (item->GetType() == ITEM_ROD)
				return true;
		}
		break;

		// BUILDING_NPC
		case BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
		{
#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (from->GetGrowthPetSystem()->IsActivePet())
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1767]");
				return false;
			}
#endif

			if (item->GetType() == ITEM_WEAPON && item->GetRefinedVnum())
				return true;
			else
				return false;
		}
		break;

		case BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
		{
#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (from->GetGrowthPetSystem()->IsActivePet())
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1767]");
				return false;
			}
#endif

			if (item->GetType() == ITEM_ARMOR &&
				(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
				item->GetRefinedVnum())
				return true;
			else
				return false;
		}
		break;

		case BLACKSMITH_ACCESSORY_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
		{
#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (from->GetGrowthPetSystem()->IsActivePet())
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1767]");
				return false;
			}
#endif

			if (item->GetType() == ITEM_ARMOR &&
				!(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
				item->GetRefinedVnum())
				return true;
			else
				return false;
		}
		break;
		// END_OF_BUILDING_NPC

		case BLACKSMITH_MOB:
		{
#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (from->GetGrowthPetSystem()->IsActivePet())
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1767]");
				return false;
			}
#endif
			if (item->GetRefinedVnum() && item->GetRefineSet() < 500)
				return true;
			else
				return false;
		}
		break;

		case BLACKSMITH2_MOB:
		{
#ifdef ENABLE_GROWTH_PET_SYSTEM
			if (from->GetGrowthPetSystem()->IsActivePet())
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1767]");
				return false;
			}
#endif

			if (item->GetRefineSet() >= 500)
				return true;
			else
				return false;
		}
		break;

		case ALCHEMIST_MOB:
		{
			if (item->GetRefinedVnum())
				return true;
		}
		break;

		case 20101:
		case 20102:
		case 20103:
		{
			// Beginner horse
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1)	// Kraut der leichten Affen
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, "[LS;452]");
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1)	// Heu
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, "[LS;453]");
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2 || item->GetVnum() == ITEM_HORSE_FOOD_3)	// Karotte | Roter Ginseng
			{
				return false;
			}
		}
		break;

		case 20104:
		case 20105:
		case 20106:
		{
			// Intermediate horse
			if (item->GetVnum() == ITEM_REVIVE_HORSE_2)	// Kraut d. normalen Affen
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, "[LS;452]");
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2)	// Karotte
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, "[LS;453]");
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_3)	// Karotte | Roter Ginseng
			{
				return false;
			}
		}
		break;

		case 20107:
		case 20108:
		case 20109:
		{
			// Advanced horse
			if (item->GetVnum() == ITEM_REVIVE_HORSE_3)	// Kraut d. schweren Affen
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, "[LS;452]");
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_3)	// Roter Ginseng
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, "[LS;453]");
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_2)	// Heu | Karotte
			{
				return false;
			}
		}
		break;

		default:
			break;
	}

	//if (IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_GIVE))
	{
		return true;
	}

	return false;
}

void CHARACTER::ReceiveItem(LPCHARACTER from, LPITEM item)
{
	if (!from || !item)
		return;

	if (IsPC())
		return;

	switch (GetRaceNum())
	{
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		case MELEY_GUILD_STATUE_VNUM:
#	ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
		case MELEY_PARTY_STATUE_VNUM:
#	endif
		{
			if (CMeleyLairManager::Instance().IsMeleyMap(from->GetMapIndex()))
			{
				switch (item->GetVnum())
				{
					case MELEY_SEAL_VNUM_KILL_STATUE:
					{
						if (from->GetMeleyLair())
							from->GetMeleyLair()->OnKillStatue(item, from, this);
					}
					break;

					default:
						break;
				}
			}
		}
		break;
#endif

		case fishing::CAMPFIRE_MOB:
		{
			if (item->GetType() == ITEM_FISH && (item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				fishing::Grill(from, item);
			else
			{
				// TAKE_ITEM_BUG_FIX
				from->SetQuestNPCID(GetVID());
				// END_OF_TAKE_ITEM_BUG_FIX
				quest::CQuestManager::Instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
			}
		}
		break;

#ifdef ENABLE_QUEEN_NETHIS
		case SnakeLair::PILAR_STEP_4:
		{
			if (from->IsPC())
			{
				if (SnakeLair::CSnk::instance().IsSnakeMap(from->GetMapIndex()))
					SnakeLair::CSnk::instance().OnKillPilar(item, from, this);
			}
		}
		break;

		case SnakeLair::BLACKSMITH_5:
		{
			if (from->IsPC())
			{
				if (SnakeLair::CSnk::instance().IsSnakeMap(from->GetMapIndex()))
					SnakeLair::CSnk::instance().OnKillBlackSmith(item, from, this);
			}
		}
		break;

		case SnakeLair::SNAKE_STATUE1:
		case SnakeLair::SNAKE_STATUE2:
		case SnakeLair::SNAKE_STATUE3:
		case SnakeLair::SNAKE_STATUE4:
		{
			if (from->IsPC())
			{
				if (SnakeLair::CSnk::instance().IsSnakeMap(from->GetMapIndex()))
					SnakeLair::CSnk::instance().OnStatueSetRotation(item, from, this);
			}
		}
		break;
#endif

		// DEVILTOWER_NPC
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
		{
			if (item->GetRefinedVnum() != 0 && item->GetRefineSet() != 0 && item->GetRefineSet() < 500)
			{
#ifdef ENABLE_SOUL_SYSTEM
				if (item->GetType() == ITEM_SOUL)
					return;
#endif
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_MONEY_ONLY);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
			}
		}
		break;
		// END_OF_DEVILTOWER_NPC

#ifdef ENABLE_QUEEN_NETHIS
		case SnakeLair::VNUM_BLACKSMITH_SERPENT:
		{
#ifdef ENABLE_GLOVE_SYSTEM
			if (item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_GLOVE)
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_MONEY_ONLY);
			}
			else
#endif
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
			}
		}
		break;
#endif // ENABLE_QUEEN_NETHIS

		case BLACKSMITH_MOB:
		case BLACKSMITH2_MOB:
		case BLACKSMITH_WEAPON_MOB:
		case BLACKSMITH_ARMOR_MOB:
		case BLACKSMITH_ACCESSORY_MOB:
		{
			if (item->GetRefinedVnum())
			{
#ifdef ENABLE_SOUL_SYSTEM
				if (item->GetType() == ITEM_SOUL)
					return;
#endif
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_NORMAL);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
			}
		}
		break;

		case 20101:
		case 20102:
		case 20103:
		case 20104:
		case 20105:
		case 20106:
		case 20107:
		case 20108:
		case 20109:
		{
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1 || item->GetVnum() == ITEM_REVIVE_HORSE_2 || item->GetVnum() == ITEM_REVIVE_HORSE_3)	// Kraut der Affen
			{
				from->ReviveHorse();
				item->SetCount(item->GetCount() - 1);
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;454]");
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_2 || item->GetVnum() == ITEM_HORSE_FOOD_3)	// Heu | Karotte | Roter Ginseng
			{
				from->FeedHorse();
				from->ChatPacket(CHAT_TYPE_INFO, "[LS;455]");
				item->SetCount(item->GetCount() - 1);
				EffectPacket(SE_HPUP_RED);
			}
		}
		break;

		default:
		{
			sys_log(0, "TakeItem %s %d %s", from->GetName(), GetRaceNum(), item->GetName());
			from->SetQuestNPCID(GetVID());
			quest::CQuestManager::Instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
		}
		break;
	}
}

bool CHARACTER::IsEquipUniqueItem(uint32_t dwItemVnum) const
{
	{
		const CItem* u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	{
		const CItem* u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	// If it is a language ring, check the recognition of the language ring (sample) as well.
	if (dwItemVnum == UNIQUE_ITEM_RING_OF_LANGUAGE)
		return IsEquipUniqueItem(UNIQUE_ITEM_RING_OF_LANGUAGE_SAMPLE);

	return false;
}

// CHECK_UNIQUE_GROUP
bool CHARACTER::IsEquipUniqueGroup(uint32_t dwGroupVnum) const
{
	{
		const LPITEM& u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetSpecialGroup() == static_cast<int>(dwGroupVnum))
			return true;
	}

	{
		const LPITEM& u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetSpecialGroup() == static_cast<int>(dwGroupVnum))
			return true;
	}

	return false;
}
// END_OF_CHECK_UNIQUE_GROUP

void CHARACTER::SetRefineMode(int iAdditionalCell)
{
	m_iRefineAdditionalCell = iAdditionalCell;
	m_bUnderRefine = true;
}

void CHARACTER::ClearRefineMode()
{
	m_bUnderRefine = false;
	SetRefineNPC(nullptr);
}

bool CHARACTER::GiveItemFromSpecialItemGroup(uint32_t dwGroupNum, std::vector<uint32_t>& dwItemVnums,
	std::vector<uint32_t>& dwItemCounts, std::vector <LPITEM>& item_gets, int& count)
{
	const CSpecialItemGroup* pGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(dwGroupNum);

	if (!pGroup)
	{
		sys_err("cannot find special item group %d", dwGroupNum);
		return false;
	}

	std::vector <int> idxes;
	const int n = pGroup->GetMultiIndex(idxes);

	bool bSuccess = false;

	for (int i = 0; i < n; i++)
	{
		bSuccess = false;
		const int idx = idxes[i];
		uint32_t dwVnum = pGroup->GetVnum(idx);
		uint32_t dwCount = pGroup->GetCount(idx);
		const int iRarePct = pGroup->GetRarePct(idx);
		LPITEM item_get = nullptr;
		switch (dwVnum)
		{
			case CSpecialItemGroup::GOLD:
				PointChange(POINT_GOLD, dwCount);
				LogManager::Instance().CharLog(this, dwCount, "TREASURE_GOLD", "");

				bSuccess = true;
				break;

			case CSpecialItemGroup::EXP:
			{
				PointChange(POINT_EXP, dwCount);
				LogManager::Instance().CharLog(this, dwCount, "TREASURE_EXP", "");

				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::MOB:
			{
				sys_log(0, "CSpecialItemGroup::MOB %d", dwCount);
				const int x = GetX() + number(-500, 500);
				const int y = GetY() + number(-500, 500);

				LPCHARACTER ch = CHARACTER_MANAGER::Instance().SpawnMob(dwCount, GetMapIndex(), x, y, 0, true, -1);
				if (ch)
					ch->SetAggressive();
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::SLOW:
			{
				sys_log(0, "CSpecialItemGroup::SLOW %d", -static_cast<int>(dwCount));
				AddAffect(AFFECT_SLOW, POINT_MOV_SPEED, -static_cast<int>(dwCount), AFF_SLOW, 300, 0, true);
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::DRAIN_HP:
			{
				int iDropHP = GetMaxHP() * dwCount / 100;
				sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
				iDropHP = MIN(iDropHP, GetHP() - 1);
				sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
				PointChange(POINT_HP, -iDropHP);
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::POISON:
			{
				AttackedByPoison(nullptr);
				bSuccess = true;
			}
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
			case CSpecialItemGroup::BLEEDING:
			{
				AttackedByBleeding(nullptr);
				bSuccess = true;
			}
			break;
#endif

			case CSpecialItemGroup::MOB_GROUP:
			{
				const int sx = GetX() - number(300, 500);
				const int sy = GetY() - number(300, 500);
				const int ex = GetX() + number(300, 500);
				const int ey = GetY() + number(300, 500);
				CHARACTER_MANAGER::Instance().SpawnGroup(dwCount, GetMapIndex(), sx, sy, ex, ey, nullptr, true);

				bSuccess = true;
			}
			break;

			default:
			{
				item_get = AutoGiveItem(dwVnum, dwCount, iRarePct);

				if (item_get)
				{
					bSuccess = true;
				}
			}
			break;
		}

		if (bSuccess)
		{
			dwItemVnums.emplace_back(dwVnum);
			dwItemCounts.emplace_back(dwCount);
			item_gets.emplace_back(item_get);
			count++;

		}
		else
		{
			return false;
		}
	}
	return bSuccess;
}

// NEW_HAIR_STYLE_ADD
bool CHARACTER::ItemProcess_Hair(LPITEM item, int iDestCell)
{
	if (!item)
		return false;

	if (item->CheckItemUseLevel(GetLevel()) == false)
	{
		// level limit
		ChatPacket(CHAT_TYPE_INFO, "[LS;456]");
		return false;
	}

	uint32_t hair = item->GetVnum();

	switch (GetJob())
	{
		case JOB_WARRIOR:
			hair -= 72000; // 73001 - 72000 = hair number starts from 1001
			break;

		case JOB_ASSASSIN:
			hair -= 71250;
			break;

		case JOB_SURA:
			hair -= 70500;
			break;

		case JOB_SHAMAN:
			hair -= 69750;
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case JOB_WOLFMAN:
			break; // NOTE: This hair code is not used, so pass. (Currently, the hair system has already been replaced with a costume)
#endif

		default:
			return false;
			break;
	}

	if (hair == GetPart(PART_HAIR))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;457]");
		return true;
	}

	item->SetCount(item->GetCount() - 1);

	SetPart(PART_HAIR, hair);
	UpdatePacket();

	return true;
}
// END_NEW_HAIR_STYLE_ADD

bool CHARACTER::ItemProcess_Polymorph(LPITEM item)
{
	if (!item)
		return false;

#ifdef ENABLE_BATTLE_FIELD
	if (CBattleField::Instance().IsBattleZoneMapIndex(GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't polymorph in battle zone."));
		return false;
	}
#endif

	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;458]");
		return false;
	}

	if (IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1053]");
		return false;
	}

	if (CPolymorphUtils::Instance().IsOnPolymorphMapBlacklist(GetMapIndex())) {	//@custom002
		ChatPacket(CHAT_TYPE_INFO, "[LS;499]");
		return false;
	}
	const uint32_t dwVnum = item->GetSocket(0);

	if (dwVnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;460]");
		item->SetCount(item->GetCount() - 1);
		return false;
	}

	const CMob* pMob = CMobManager::Instance().Get(dwVnum);

	if (pMob == nullptr)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;460]");
		item->SetCount(item->GetCount() - 1);
		return false;
	}

	//@fixme482
	uint32_t dwSkillsVnums[12] = {
		3, 4, 19, 34, 49, 63, 64, 65, 78, 79, 174, 175
	};
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't use buffs if you are polymorphed."));
	for (int i = 0; i < 12; ++i) {
		CSkillProto* pkSk = CSkillManager::Instance().Get(dwSkillsVnums[i]);

		if (FindAffect(pkSk->dwVnum, pkSk->wPointOn)) {
			RemoveAffect(pkSk->dwVnum);
		}
	}	//@end_fixme482

	switch (item->GetVnum())
	{
		case 70104:	// Verwandlungskugel
		case 70105:	// Verwandlungskugel
		case 70106:	// Verwandlungskugel
		case 70107:	// Verwandlungskugel
		case 71093:	// Verwandlungskugel
		{
			// processing of armor
			sys_log(0, "USE_POLYMORPH_BALL PID(%d) vnum(%d)", GetPlayerID(), dwVnum);

			// level limit check
			const int iPolymorphLevelLimit = MAX(0, 20 - GetLevel() * 3 / 10);
			if (pMob->m_table.bLevel >= GetLevel() + iPolymorphLevelLimit)
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;461]");
				return false;
			}

			int iDuration = GetSkillLevel(POLYMORPH_SKILL_ID) == 0 ? 5 : (5 + (5 + GetSkillLevel(POLYMORPH_SKILL_ID) / 40 * 25));
			iDuration *= 60;

			uint32_t dwBonus = 0;

			dwBonus = (2 + GetSkillLevel(POLYMORPH_SKILL_ID) / 40) * 100;

			AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, AFF_POLYMORPH, iDuration, 0, true);
			AddAffect(AFFECT_POLYMORPH, POINT_ATT_BONUS, dwBonus, AFF_POLYMORPH, iDuration, 0, false);

			item->SetCount(item->GetCount() - 1);
		}
		break;

		case 50322:	// Umwandlungsrolle
		{
			// Transformation Role
	
			// handle disguise
			// socket 0 socket 1 socket 2
			// Monster number to be disguised as training level
			sys_log(0, "USE_POLYMORPH_BOOK: %s(%u) vnum(%u)", GetName(), GetPlayerID(), dwVnum);

			if (CPolymorphUtils::Instance().PolymorphCharacter(this, item, pMob) == true)
			{
				CPolymorphUtils::Instance().UpdateBookPracticeGrade(this, item);
			}
			else
			{
			}
		}
		break;

		default:
			sys_err("POLYMORPH invalid item passed PID(%d) vnum(%d)", GetPlayerID(), item->GetOriginalVnum());
			return false;
	}

	return true;
}

bool CHARACTER::CanDoCube() const
{
	if (m_bIsObserver)
		return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (GetViewingShop())
		return false;
#else
	if (GetShop())
		return false;

	if (GetMyShop())
		return false;
#endif

	if (m_bUnderRefine)
		return false;

	if (IsWarping())
		return false;

#ifdef ENABLE_REFINE_ELEMENT
	if (IsOpenElementsSpell())
		return false;
#endif

#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen())
		return false;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	if (IsChangeLookWindowOpen())
		return false;
#endif

	return true;
}

bool CHARACTER::UnEquipCostumeMoudeItem(bool isOnDeath)
{
	LPITEM CostumeMount = GetWear(WEAR_COSTUME_MOUNT);

	if (nullptr != CostumeMount)
	{
		if (CostumeMount->IsCostumeMount())
		{
			return UnequipItem(CostumeMount);
		}
	}

	return true;
}

bool CHARACTER::UnEquipSpecialRideUniqueItem()
{
	LPITEM Unique1 = GetWear(WEAR_UNIQUE1);
	LPITEM Unique2 = GetWear(WEAR_UNIQUE2);
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	LPITEM MountCostume = GetWear(WEAR_COSTUME_MOUNT);
#endif

	if (nullptr != Unique1)
	{
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup())
		{
			return UnequipItem(Unique1);
		}
	}

	if (nullptr != Unique2)
	{
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup())
		{
			return UnequipItem(Unique2);
		}
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	if (MountCostume)
		return UnequipItem(MountCostume);
#endif

	return true;
}

void CHARACTER::AutoRecoveryItemProcess(const EAffectTypes type)
{
	if (IsDead() || IsStun())
		return;

	if (!IsPC())
		return;

	if (AFFECT_AUTO_HP_RECOVERY != type && AFFECT_AUTO_SP_RECOVERY != type)
		return;

#ifdef ENABLE_SUNG_MAHI_TOWER
	if (IsSungMahiDungeon(GetMapIndex()))
		return;
#endif

	if (nullptr != FindAffect(AFFECT_STUN))
		return;

	{
		const uint32_t stunSkills[] = { SKILL_TANHWAN, SKILL_GEOMPUNG, SKILL_BYEURAK, SKILL_GIGUNG };

		for (size_t i = 0; i < sizeof(stunSkills) / sizeof(uint32_t); ++i)
		{
			CAffect* p = FindAffect(stunSkills[i]);
			if (nullptr != p && AFF_STUN == p->dwFlag)
				return;
		}
	}

	const CAffect* pAffect = FindAffect(type);
	constexpr size_t idx_of_amount_of_used = 1;
	constexpr size_t idx_of_amount_of_full = 2;

	if (nullptr != pAffect)
	{
		LPITEM pItem = FindItemByID(pAffect->dwFlag);

		if ((nullptr != pItem) && (pItem->GetSocket(0) > 0))
		{
			if (!CArenaManager::Instance().IsArenaMap(GetMapIndex())
#ifdef ENABLE_NEWSTUFF
				&& !(g_NoPotionsOnPVP && CPVPManager::Instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(pItem->GetVnum()))
#endif
				)
			{
				const long amount_of_used = pItem->GetSocket(idx_of_amount_of_used);
				const long amount_of_full = pItem->GetSocket(idx_of_amount_of_full);

				const int32_t avail = amount_of_full - amount_of_used;

				int32_t amount = 0;

				if (AFFECT_AUTO_HP_RECOVERY == type)
				{
					amount = GetMaxHP() - (GetHP() + GetPoint(POINT_HP_RECOVERY));
				}
				else if (AFFECT_AUTO_SP_RECOVERY == type)
				{
					amount = GetMaxSP() - (GetSP() + GetPoint(POINT_SP_RECOVERY));
				}

				if (amount > 0)
				{
					if (avail > amount)
					{
						const int pct_of_used = amount_of_used * 100 / amount_of_full;
						const int pct_of_will_used = (amount_of_used + amount) * 100 / amount_of_full;

						bool bLog = false;
						// Logs in units of 10% of usage
						// (In % of usage, log each time the tens place changes.)
						if ((pct_of_will_used / 10) - (pct_of_used / 10) >= 1)
							bLog = true;
						pItem->SetSocket(idx_of_amount_of_used, amount_of_used + amount, bLog);
					}
					else
					{
						amount = avail;

						ITEM_MANAGER::Instance().RemoveItem(pItem);
					}

					if (AFFECT_AUTO_HP_RECOVERY == type)
					{
						PointChange(POINT_HP_RECOVERY, amount);
						EffectPacket(SE_AUTO_HPUP);
					}
					else if (AFFECT_AUTO_SP_RECOVERY == type)
					{
						PointChange(POINT_SP_RECOVERY, amount);
						EffectPacket(SE_AUTO_SPUP);
					}
				}
			}
			else
			{
				pItem->Lock(false);
				pItem->SetSocket(0, false);
				RemoveAffect(const_cast<CAffect*>(pAffect));
			}
		}
		else
		{
			RemoveAffect(const_cast<CAffect*>(pAffect));
		}
	}
}

bool CHARACTER::IsValidItemPosition(TItemPos Pos) const
{
	const uint8_t window_type = Pos.window_type;
	const uint16_t cell = Pos.cell;

	switch (window_type)
	{
		case RESERVED_WINDOW:
			return false;

		case INVENTORY:
			return cell < (INVENTORY_SLOT_COUNT);

		case EQUIPMENT:
			return cell < (EQUIPMENT_SLOT_COUNT);

		case BELT_INVENTORY:
			return cell < (BELT_INVENTORY_SLOT_COUNT);

		case DRAGON_SOUL_INVENTORY:
			return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);

		case SAFEBOX:
			if (nullptr != m_pkSafebox)
				return m_pkSafebox->IsValidPosition(cell);
			else
				return false;

		case MALL:
			if (nullptr != m_pkMall)
				return m_pkMall->IsValidPosition(cell);
			else
				return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		case PREMIUM_PRIVATE_SHOP:
			return cell < (SHOP_INVENTORY_MAX_NUM);
#endif

#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
#endif

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case GUILDBANK:
			if (nullptr != m_pkGuildstorage)
				return m_pkGuildstorage->IsValidPosition(cell);
			else
				return false;
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		case ADDITIONAL_EQUIPMENT_1:
			return cell < ADDITIONAL_EQUIPMENT_MAX_SLOT;
#endif

		default:
			sys_err("!! invalid window: %d", window_type);
			return false;
	}
}

// A macro created because of annoyance.. If exp is true, a macro that prints msg and returns false.
#define VERIFY_MSG(exp, msg) \
	if (true == (exp)) { \
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT(msg)); \
		return false; \
	}

/// A function that checks whether the given item can be worn based on the current character's state, and if not, informs the character why.
bool CHARACTER::CanEquipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell) /*const*/
{
	const TItemTable* itemTable = item->GetProto();
	//uint8_t itemType = item->GetType();
	//uint8_t itemSubType = item->GetSubType();

	switch (GetJob())
	{
		case JOB_WARRIOR:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;

#ifdef ENABLE_WOLFMAN_CHARACTER
		case JOB_WOLFMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break; // TODO: Handling of wearability of beast-type items
#endif

		default:
			break;
	}

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		const long limit = itemTable->aLimits[i].lValue;
		switch (itemTable->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
			{
				if (GetLevel() < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;462]");
					return false;
				}
			}
			break;

			case LIMIT_STR:
			{
				if (GetPoint(POINT_ST) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;463]");
					return false;
				}
			}
			break;

			case LIMIT_INT:
			{
				if (GetPoint(POINT_IQ) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;464]");
					return false;
				}
			}
			break;

			case LIMIT_DEX:
			{
				if (GetPoint(POINT_DX) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;465]");
					return false;
				}
			}
			break;

			case LIMIT_CON:
			{
				if (GetPoint(POINT_HT) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, "[LS;466]");
					return false;
				}
			}
			break;

			default:
				break;
		}
	}

	if (item->GetWearFlag() & WEARABLE_UNIQUE)
	{
		if ((GetWear(WEAR_UNIQUE1) && GetWear(WEAR_UNIQUE1)->IsSameSpecialGroup(item)) ||
			(GetWear(WEAR_UNIQUE2) && GetWear(WEAR_UNIQUE2)->IsSameSpecialGroup(item)))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;468]");
			return false;
		}

		if (marriage::CManager::Instance().IsMarriageUniqueItem(item->GetVnum()) &&
			!marriage::CManager::Instance().IsMarried(GetPlayerID()))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;467]");
			return false;
		}
	}

	if (item->GetType() == ITEM_RING) {	// @fixme401
		if ((GetWear(WEAR_RING1) && GetWear(WEAR_RING1)->GetVnum() == item->GetVnum()) || (GetWear(WEAR_RING2) && GetWear(WEAR_RING2)->GetVnum() == item->GetVnum())) {
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't equip 2 same ring"));
			return false;
		}
	}	// @end_fixme401

	if (item->IsCostumeMount()
#ifdef DISABLE_RIDE_ON_SPECIAL_MAP
		&& !IS_MOUNTABLE_ZONE(GetMapIndex())	//@custom005
#endif
		)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("CANNOT_EQUIP_MOUNT"));
		return false;
	}

	if (item->IsPetItem() && IS_BLOCKED_PET_SUMMON_MAP(GetMapIndex())) {	//@custom006
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_SUMMON_A_PET_HERE"));
		return false;
	}

	if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_MOUNT) {	//@custom012
		if (CThreeWayWar::Instance().IsThreeWayWarMapIndex(GetMapIndex()))
			return false;
	}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	const TItemTable* changeLookTable = ITEM_MANAGER::Instance().GetTable(item->GetChangeLookVnum());

	if (!changeLookTable)
		return true;

	if ((changeLookTable->bType == ITEM_WEAPON) && (changeLookTable->bSubType == WEAPON_SWORD))
	{
		if ((GetJob() != JOB_SURA) && IS_SET(changeLookTable->dwAntiFlags, ITEM_ANTIFLAG_WARRIOR | ITEM_ANTIFLAG_ASSASSIN))
		{
			ChatPacket(CHAT_TYPE_INFO, "[LS;71]");
			return false;
		}
	}
	else if ((changeLookTable->bType == ITEM_COSTUME) && (changeLookTable->bSubType == COSTUME_BODY
		|| changeLookTable->bSubType == COSTUME_HAIR
#ifdef ENABLE_ACCE_COSTUME_SYSTEM // custom
		|| changeLookTable->bSubType == COSTUME_ACCE
#endif
		))
	{
		if (IS_SET(changeLookTable->dwAntiFlags, ITEM_ANTIFLAG_MALE))
		{
			if (ESex::SEX_MALE == GET_SEX(this))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;70]");
				return false;
			}
		}
		if (IS_SET(changeLookTable->dwAntiFlags, ITEM_ANTIFLAG_FEMALE))
		{
			if (ESex::SEX_FEMALE == GET_SEX(this))
			{
				ChatPacket(CHAT_TYPE_INFO, "[LS;70]");
				return false;
			}
		}
	}
#	endif

	return true;
}

/// A function that checks whether the item being worn can be taken off based on the current character's state, and if not, informs the character of the reason
#ifdef ENABLE_SWAP_SYSTEM
bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos& swapCell) /*const*/
{
	if (!item)
		return false;

#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen())
		return false;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	if (IsChangeLookWindowOpen())
		return false;
#endif

	if (ITEM_BELT == item->GetType())
		VERIFY_MSG(CBeltInventoryHelper::IsExistItemInBeltInventory(this), "[1095]You can only discard the belt when there are no longer any items in its inventory.");

	// Items that can't be unlocked forever
	if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

	// Check if there is an empty space when moving items to inventory when unequip
	{
		int pos = -1;

		if (item->IsDragonSoul())
			pos = GetEmptyDragonSoulInventory(item);
		else
#ifdef ENABLE_SPECIAL_INVENTORY
			pos = GetEmptyInventory(item);
#else
			pos = GetEmptyInventory(item->GetSize());
#endif

		//Not enough space, but we are swapping, so let's figure out if I can unequip after the theorical swap - [Think]
		if (pos == -1 && !item->IsDragonSoul() && swapCell != NPOS) {
			LPITEM destItem = GetItem(swapCell);

			if (destItem) {
				if (destItem->GetSize() - item->GetSize() >= 0) //Target is equal/larger than after the change
					return true;

				if (IsEmptyItemGrid(TItemPos(INVENTORY, destItem->GetCell()), item->GetSize(), destItem->GetCell()))
					return true;
			}
		}

		VERIFY_MSG(-1 == pos, "[1130]There isn't enough space in your inventory.");
	}


	return true;
}
#else
bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell) /*const*/
{
	if (!item)
		return false;

	if (ITEM_BELT == item->GetType())
		VERIFY_MSG(CBeltInventoryHelper::IsExistItemInBeltInventory(this), "[1095]You can only discard the belt when there are no longer any items in its inventory.");

	// Items that can't be unlocked forever
	if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

#ifdef ENABLE_AURA_SYSTEM
	if (IsAuraRefineWindowOpen())
		return false;
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	if (IsChangeLookWindowOpen())
		return false;
#endif

#ifdef ENABLE_ATTR_6TH_7TH
	if (IsOpenAttr67Dialog())
		return false;
#endif

	// Check if there is an empty space when moving items to inventory when unequip
	{
		int pos = -1;

		if (item->IsDragonSoul())
			pos = GetEmptyDragonSoulInventory(item);
		else
		{
#ifdef ENABLE_SPECIAL_INVENTORY
			pos = GetEmptyInventory(item);
#else
			pos = GetEmptyInventory(item->GetSize());
#endif
		}

		VERIFY_MSG(-1 == pos, "[1130]There isn't enough space in your inventory.");
	}

	return true;
}
#endif

#ifdef ENABLE_SOUL_SYSTEM
void CHARACTER::UseSoulAttack(uint8_t bSoulType)
{
	if (bSoulType == RED_SOUL && !IsActivatedRedSoul())
		return;

	if (bSoulType == BLUE_SOUL && !IsActivatedBlueSoul())
		return;

	if (bSoulType > BLUE_SOUL)
		return;

	const uint32_t vnum[][5] =
	{
		{70500, 70501, 70502, 70503, 70504},
		{70505, 70506, 70507, 70508, 70509},
	};

	for (int i = 0; i < 5; ++i)
	{
		LPITEM item = FindSpecifyItem(vnum[bSoulType][i]);
		if ((item != nullptr) && (item->GetSocket(1) == 1) && (item->GetSocket(3) >= item->GetLimitValue(1)))
		{
			if (item->GetSocket(2) <= (item->GetLimitValue(1) * 10000))
			{
				RemoveAffect(AFFECT_SOUL_MIX);
				if (bSoulType == RED_SOUL)
					RemoveAffect(AFFECT_SOUL_RED);
				else
					RemoveAffect(AFFECT_SOUL_BLUE);

				item->SetSocket(1, 0);
				item->SetSocket(2, item->GetValue(2));
				item->SetSocket(3, 0);
				item->StartSoulTimeUseEvent();
			}
			else
				item->SetSocket(2, item->GetSocket(2) - 1);
		}
	}
}

bool CHARACTER::IsActivatedRedSoul()
{
	const uint32_t dwVnum[] = { 70500, 70501, 70502, 70503, 70504 };

	for (int i = 0; i < 5; ++i)
	{
		LPITEM item = FindSpecifyItem(dwVnum[i]);
		if ((item != nullptr) && (item->GetSocket(1) == 1))
			return true;
	}

	return false;
}

bool CHARACTER::IsActivatedBlueSoul()
{
	const uint32_t dwVnum[] = { 70505, 70506, 70507, 70508, 70509 };

	for (int i = 0; i < 5; ++i)
	{
		LPITEM item = FindSpecifyItem(dwVnum[i]);
		if ((item != nullptr) && (item->GetSocket(1) == 1))
			return true;
	}

	return false;
}

bool CHARACTER::DoRefineSoul(LPITEM item)
{
	if (!item)
		return false;

	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	ClearRefineMode();

	if (m_iRefineAdditionalCell < 0)
		return false;

	LPITEM pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

	if (!pkItemScroll)
		return false;

	if (!(pkItemScroll->GetType() == ITEM_USE && pkItemScroll->GetSubType() == USE_TUNING))
		return false;

	if (pkItemScroll->GetVnum() == item->GetVnum())
		return false;

	const uint32_t result_vnum = item->GetRefinedVnum();
	const uint32_t result_fail_vnum = item->GetRefineFromVnum();

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;991]");
		return false;
	}

	const TItemTable* pProto = ITEM_MANAGER::Instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefineSoul NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, "[LS;1002]");
		return false;
	}

	const int prob = number(1, 100);

	int success_prob = 0;
	const char* szRefineType{ 0 };

	if (pkItemScroll->GetValue(0) == SOUL_EVOLVE_SCROLL)
	{
		success_prob = SOUL_REFINE_EVOLVE_PROB;
		szRefineType = "SOUL_EVOLVE_SCROLL";
	}
	else if (pkItemScroll->GetValue(0) == SOUL_AWAKE_SCROLL)
	{
		success_prob = SOUL_REFINE_AWAKE_PROB;
		szRefineType = "SOUL_AWAKE_SCROLL";
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;991]");
		return false;
	}

	pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

	if (prob <= success_prob)
	{
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
#ifdef ENABLE_SEALBIND_SYSTEM
			pkNewItem->SetSealDate(item->GetSealDate());
#endif
			LogManager::Instance().ItemLog(this, pkNewItem, "REFINE_SOUL SUCCESS", pkNewItem->GetName());

			const uint16_t wCell = item->GetCell();
			NotifyRefineSuccess(this, item, szRefineType);
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();
		}
		else
		{
			sys_err("cannot create item %u", result_vnum);
			NotifyRefineFail(this, item, szRefineType);
		}
	}
	else if (result_fail_vnum)
	{
		LPITEM pkNewItem = ITEM_MANAGER::Instance().CreateItem(result_fail_vnum, 1, 0, false);

		if (pkNewItem)
		{
#ifdef ENABLE_SEALBIND_SYSTEM
			pkNewItem->SetSealDate(item->GetSealDate());
#endif
			LogManager::Instance().ItemLog(this, pkNewItem, "REFINE_SOUL FAIL", pkNewItem->GetName());

			const uint16_t wCell = item->GetCell();
			NotifyRefineFail(this, item, szRefineType, -1);
			ITEM_MANAGER::Instance().RemoveItem(item, "REMOVE (REFINE_SOUL FAIL)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::Instance().FlushDelayedSave(pkNewItem);

			pkNewItem->AttrLog();
		}
		else
		{
			sys_err("cannot create item %u", result_fail_vnum);
			NotifyRefineFail(this, item, szRefineType);
		}
	}
	else
	{
		NotifyRefineFail(this, item, szRefineType);
	}

	return true;
}
#endif

#ifdef ENABLE_SET_ITEM
void CHARACTER::GetSetCount(int& setID, int& setCount)
{
	setID = 0;
	setCount = 0;

	std::vector<LPITEM> items = { GetWear(WEAR_BODY), GetWear(WEAR_HEAD), GetWear(WEAR_WEAPON) };

	std::vector<uint8_t> itemSet = {};
	for (const auto& item : items) {
		if (item)
			itemSet.emplace_back(item->GetItemSetValue());
	}

	std::map<uint8_t, int> msetCount = {
		{1, 0},
		{2, 0},
		{3, 0},
		{4, 0},
		{5, 0},
	};

	for (const auto& v : itemSet) {
		++msetCount[v];
	}

	for (const auto& info : msetCount) {
		if (info.second >= 2) {
			setID = info.first;
			setCount = info.second;
			break;
		}
	}
}
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
struct ItemList
{
	uint32_t dwVnum;
	uint32_t iCount;
	long sockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_NORM_NUM];
	bool IsBasicItem;

	ItemList(uint32_t _dwVnum, uint32_t _iCount, std::vector<uint32_t> _sockets, std::vector<TPlayerItemAttribute> attributes, bool _IsBasicItem)
	{
		if (_sockets.size() != ITEM_SOCKET_MAX_NUM || attributes.size() != ITEM_ATTRIBUTE_NORM_NUM)
		{
			sys_err("ERROR. Basic item %d", _dwVnum);
		}
		else
		{
			dwVnum = _dwVnum;
			iCount = _iCount;
			for (size_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				sockets[i] = _sockets[i];
			for (size_t i = 0; i < ITEM_ATTRIBUTE_NORM_NUM; ++i) {
				aAttr[i].wType = attributes[i].wType;
				aAttr[i].sValue = attributes[i].sValue;
			}
			IsBasicItem = _IsBasicItem;
		}
	}
};

#include "item_manager.h"
#include "unique_item.h"
void CHARACTER::EquipItems(LPCHARACTER ch)
{
	if (!ch)
		return;

	std::map<int, std::vector<ItemList>> items =
	{
		{0, {	//WARRIOR_M
			ItemList(19, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Schwert+9
			ItemList(11200, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Mönchsplattenpanzer+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12209, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Traditioneller Helm+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{1, {	//NINJA_W
			ItemList(1009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Dolch+9
			ItemList(11400, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Azuranzug+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12349, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Lederkapuze+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{2, {	//SURA_M
			ItemList(19, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Schwert+9
			ItemList(11600, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Requiem-Plattenpanzer+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12489, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Blutiger Helm+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{3, {	//SHAMAN_W
			ItemList(7009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Fächer+9
			ItemList(11800, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Purpurkleidung+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12629, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Mönchshut+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{4, {	//WARRIOR_W
			ItemList(19, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Schwert+9
			ItemList(11200, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Mönchsplattenpanzer+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12209, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Traditioneller Helm+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{5, {	//NINJA_M
			ItemList(1009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Dolch+9
			ItemList(11400, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Azuranzug+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12349, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Lederkapuze+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{6, {	//SURA_W
			ItemList(19, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Schwert+9
			ItemList(11600, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Requiem-Plattenpanzer+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12489, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Blutiger Helm+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
		{7, {	//SHAMAN_M
			ItemList(7009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Fächer+9
			ItemList(11800, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Purpurkleidung+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(12629, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Mönchshut+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
#	ifdef ENABLE_WOLFMAN_CHARACTER
		{8, {	//WOLFMAN
			ItemList(6009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {20,20}, {16,10}, {15,10}, }, true),	// Stahlfackel+9
			ItemList(21000, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {23,10}, {24,10}, {53,50}, {9,20}, }, true),	// Himmelswindpanzer+0
			ItemList(50188, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lerlingstruhe
			ItemList(27053, 20 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Trank (+30)
			ItemList(27054, 30 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Violetter Trank (+30)
			ItemList(76023, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüner Zauber
			ItemList(76024, 15 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Grüne Stärke
			ItemList(71009, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Lagertruhe (Mobil)
			ItemList(76021, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Elixier der Sonne (K)
			#ifdef ENABLE_ANTI_EXP_RING
			ItemList(ITEM_AUTO_EXP_RING_S, 1 , {0, 10000000, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),// Elixier der Erfahrung (K)
			#endif
			#ifdef GIVE_BASIC_SKILLS
			ItemList(50052, 1 , {0, 0, 0, 0, 0, 0}, { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, }, true),			// Waffen-Pferdebuch
			#endif
			#ifdef GIVE_BASIC_EXTENDED_ITEMS
			ItemList(21509, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {7,8}, {28,15}, {12,8}, }, true),	// Skeletthelm+9
			ItemList(13009, 1 , {0, 0, 0, 0, 0, 0}, { {48,1}, {49,1}, {43,20}, {44,20}, {39,15}, }, true),	// Schlachtschild+9
			ItemList(14009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {16,10}, {18,20}, {19,20}, {23,10}, }, true),// Holzarmband+9
			ItemList(15009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {28,15}, {15,10}, {44,20}, {45,20}, }, true),// Lederschuhe+9
			ItemList(16009, 1 , {0, 0, 0, 0, 0, 0}, { {1,2000}, {15,10}, {16,10}, {18,20}, {19,20}, }, true),// Holzhalskette+9
			ItemList(17009, 1 , {0, 0, 0, 0, 0, 0}, { {18,20}, {19,20}, {45,20}, {41,8}, {8,20}, }, true),	// Holzohrringe+9
			#endif
		},},
#	endif
	};

	if (items.find(ch->GetRaceNum()) == items.end()) {
		return;//race not found
	}

	for (auto itemIt : items[ch->GetRaceNum()])
	{
		//LPITEM pItem = ITEM_MANAGER::Instance().CreateItem(itemIt.dwVnum, itemIt.iCount);
		//int iEmptyPos = pItem->IsDragonSoul() ? ch->GetEmptyDragonSoulInventory(pItem) : ch->GetEmptyInventory(pItem);
		LPITEM pItem = ITEM_MANAGER::Instance().CreateItem(itemIt.dwVnum, itemIt.iCount);
		if (!pItem || !itemIt.dwVnum)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "wrong item at creation!");
			return;
		}

		int iEmptyPos = ch->GetEmptyInventory(pItem);
		if (iEmptyPos != -1)
		{
			//pItem->AddToCharacter(ch, TItemPos(pItem->IsDragonSoul() ? DRAGON_SOUL_INVENTORY : INVENTORY, iEmptyPos));
			pItem->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			pItem->SetBasic(itemIt.IsBasicItem);

			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				pItem->SetSocket(i, itemIt.sockets[i]);
			for (int i = 0; i < ITEM_ATTRIBUTE_NORM_NUM; ++i)
				pItem->SetAttribute(i, itemIt.aAttr[i].wType, itemIt.aAttr[i].sValue);

			if (pItem->IsEquipable())
				ch->EquipItem(pItem);
		}
	}
}
#endif

#ifdef ENABLE_CHANGED_ATTR
void CHARACTER::SelectAttr(LPITEM material, LPITEM item)
{
	const LPDESC d = GetDesc();
	if (d == nullptr)
		return;

	if (IsSelectAttr() || GetExchange() || IsOpenSafebox() || /*GetShopOwner() ||*/ GetMyShop() || IsCubeOpen())
	{
		ChatPacket(CHAT_TYPE_INFO, "You have to close other windows.");
		return;
	}

	if (item->GetAttributeSetIndex() == -1)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("123123"));
		return;
	}

	if (item->GetAttributeCount() < 1)
		return;

	const TItemPos pos(item->GetWindow(), item->GetCell());

	if (pos.IsDefaultInventoryPosition() == false)
		return;

	m_ItemSelectAttr.dwItemID = item->GetID();
	item->GetSelectAttr(m_ItemSelectAttr.Attr);

	TPacketGCItemSelectAttr p;
	p.bHeader = HEADER_GC_ITEM_SELECT_ATTR;
	p.pItemPos = pos;
	std::copy(std::begin(m_ItemSelectAttr.Attr), std::end(m_ItemSelectAttr.Attr), std::begin(p.aAttr));
	d->Packet(&p, sizeof p);

	material->SetCount(material->GetCount() - 1);
	}

void CHARACTER::SelectAttrResult(const bool bNew, const TItemPos& pos)
{
	if (IsSelectAttr() == false)
		return;

	if (bNew)
	{
		const LPITEM item = GetItem(pos);
		if (item && item->GetID() == m_ItemSelectAttr.dwItemID)
		{
			item->SetAttributes(m_ItemSelectAttr.Attr);
			item->UpdatePacket();
			ChatPacket(CHAT_TYPE_INFO, "You have changed the upgrade.");
		}
	}

	memset(&m_ItemSelectAttr, 0, sizeof m_ItemSelectAttr);
}

bool CHARACTER::IsSelectAttr() const
{
	return m_ItemSelectAttr.dwItemID != 0;
}
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
bool CHARACTER::GetWearNotChange(uint16_t cell) const
{
	const uint8_t wear_block[] =
	{
		WEAR_COSTUME_BODY,
		WEAR_COSTUME_HAIR,
		WEAR_COSTUME_WEAPON,
		WEAR_COSTUME_MOUNT,
		WEAR_COSTUME_ACCE,
		WEAR_COSTUME_AURA,
		WEAR_RING1,
		WEAR_RING2,
	};

	for (uint8_t i = 0; i < sizeof(wear_block) / sizeof(*wear_block); i++)
	{
		if (wear_block[i] == cell)
			return false;
	}

	if (cell >= WEAR_MAX_NUM && cell < WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		return false;
	}

	return true;
}

bool CHARACTER::CheckAdditionalEquipment(uint16_t cell) const
{
	/*
	false = inventory
	true = new_inventory
	*/

	if (!GetWearNotChange(cell))
		return false;

	if (GetPageEquipment() == 0)
		return false;

	return true;
}
#endif
