#include "stdafx.h"

#ifdef ENABLE_GROWTH_PET_SYSTEM
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "GrowthPetSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "desc_client.h"
#include "log.h"

//// CHARACTER
void CHARACTER::SendGrowthPetHatching(uint8_t bResult, uint8_t pos)
{
	TPacketGCGrowthPetHatching sPacket;
	sPacket.bHeader = HEADER_GC_GROWTH_PET_HATCHING;
	sPacket.bPos = pos;
	sPacket.bResult = bResult;

	if (GetDesc())
		GetDesc()->Packet(&sPacket, sizeof(TPacketGCGrowthPetHatching));
}

void CHARACTER::SendGrowthPetUpgradeSkillRequest(uint8_t bSkillSlot, uint8_t bSkillIndex, int iPrice)
{
	TPacketGCGrowthPetSkillUpgradeRequest sPacket;
	sPacket.bHeader = HEADER_GC_GROWTH_PET_UPGRADE_SKILL_REQUEST;

	sPacket.bSkillSlot = bSkillSlot;
	sPacket.bSkillIndex = bSkillIndex;
	sPacket.iPrice = iPrice;

	if (GetDesc())
		GetDesc()->Packet(&sPacket, sizeof(TPacketGCGrowthPetSkillUpgradeRequest));
}

#ifdef ENABLE_PET_ATTR_DETERMINE
void CHARACTER::PetAttrChange(uint8_t bPetSlotIndex, uint8_t bMaterialSlotIndex)
{
	LPITEM itemPet = GetInventoryItem(bPetSlotIndex);
	if (!itemPet)
		return;

	LPITEM itemMaterial = GetInventoryItem(bMaterialSlotIndex);
	if (!itemMaterial)
		return;

	if ((itemPet->GetType() != ITEM_PET) || (itemPet->GetSubType() != PET_UPBRINGING))
		return;

	if ((itemMaterial->GetType() != ITEM_PET) || (itemMaterial->GetSubType() != PET_ATTR_CHANGE))
		return;

	const long lPetDuration = (itemPet->GetSocket(0) - get_global_time()) / 60;
	if (lPetDuration <= 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PET_DEAD_CANT_ATTR_CHANGE.")); // Already checked by pythonside
		return;
	}

	TGrowthPetInfo info = itemPet->GetGrowthPetItemInfo();
	info.pet_type = number(1, PET_HATCH_INFO_RANGE[itemPet->GetVnum() - 55701][5]);
	info.pet_max_time = (60 * 60 * 24) * number(PET_HATCH_INFO_RANGE[itemPet->GetVnum() - 55701][1], PET_HATCH_INFO_RANGE[itemPet->GetVnum() - 55701][2]);

	info.pet_hp = fnumber(0.1, 2.3);  // Max: 17.0 (perfect)
	info.pet_def = fnumber(0.1, 2.1); // Max: 11.1 (perfect)
	info.pet_sp = fnumber(0.1, 2.9);  // Max: 18.5 (perfect)

	const uint8_t bToLevel = info.pet_level - 1;
	uint8_t bDetermineLevel = 1;

	for (auto i = 0; i < bToLevel; ++i)
	{
		bDetermineLevel++;

		if (bDetermineLevel % int(PET_HP_RANGE[info.pet_type][2]) == 0)
			info.pet_hp += fnumber(PET_HP_RANGE[info.pet_type][0], PET_HP_RANGE[info.pet_type][1]);

		if (bDetermineLevel % int(PET_DEF_RANGE[info.pet_type][2]) == 0)
			info.pet_def += fnumber(PET_DEF_RANGE[info.pet_type][0], PET_DEF_RANGE[info.pet_type][1]);

		if (bDetermineLevel % int(PET_SP_RANGE[info.pet_type][2]) == 0)
			info.pet_sp += fnumber(PET_SP_RANGE[info.pet_type][0], PET_SP_RANGE[info.pet_type][1]);
	}

	bool bNewSkillCount = true;
	for (uint8_t i = 0; i < 3; ++i)
	{
		if (info.skill_vnum[i])
		{
			bNewSkillCount = false;
			break;
		}
	}

	if (bNewSkillCount)
		info.skill_count = number(PET_HATCH_INFO_RANGE[itemPet->GetVnum() - 55701][3], PET_HATCH_INFO_RANGE[itemPet->GetVnum() - 55701][4]);

	// Set Informations
	itemPet->SetSocket(0, time(0) + info.pet_max_time);
	itemPet->SetSocket(1, info.pet_max_time);

	// Save Pet Item Informations
	itemPet->SetGrowthPetItemInfo(info);

	// Send Pet Informations to Character
	SetGrowthPetInfo(info);
	SendGrowthPetInfoPacket();

	// Remove Item
	itemMaterial->SetCount(itemMaterial->GetCount() - 1);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	UpdateExtBattlePassMissionProgress(PET_ENCHANT, 1, info.pet_type);
#endif

	// Send Packet
	TPacketGCGrowthPetAttrChangeResult sPacket;
	sPacket.bHeader = HEADER_GC_GROWTH_ATTR_CHANGE_RESULT;
	sPacket.bResult = info.pet_type;

	if (GetDesc())
		GetDesc()->Packet(&sPacket, sizeof(TPacketGCGrowthPetAttrChangeResult));

	ChatPacket(CHAT_TYPE_INFO, "[LS;1350]");
}
#endif

void CHARACTER::EvolvePetRace(uint32_t dwRaceNum)
{
	if (m_dwPolymorphRace == dwRaceNum)
		return;

	m_bPolyMaintainStat = false;
	m_dwPolymorphRace = dwRaceNum;

	sys_log(0, "GROWTH_PET_EVOLUTION: %s race %u ", GetName(), dwRaceNum);

	StopRiding();

	SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
	ViewReencode();
	REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
}

void CHARACTER::SendPetLevelUpEffect(int vid, int value)
{
	struct packet_point_change pack;

	pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
	pack.dwVID = vid;
	pack.type = 1;
	pack.value = value;
	pack.amount = 1;
	PacketAround(&pack, sizeof(pack));
}
//// END_OF_CHARACTER

void GetGrowthPetSkillSpecialistValue(uint32_t dwPetVnum, uint32_t dwPetSkillVnum, float* value) noexcept
{
	if (!value)
		return;

	*value = 0;
	for (auto table : pet_skill_specialist_table)
	{
		if (table->pet_vnum == dwPetVnum && table->skill_vnum == dwPetSkillVnum)
		{
			*value = fnumber(table->min_value, table->max_value);
			break;
		}
	}
}

EVENTINFO(GrowthPetSystem_Event_Info)
{
	CGrowthPetSystem* pPetSystem;
};

EVENTFUNC(GrowthPetSystem_Update_Event)
{
	GrowthPetSystem_Event_Info* info = dynamic_cast<GrowthPetSystem_Event_Info*>(event->info);
	if (info == nullptr)
	{
		sys_err("check_speedhack_event> <Factor> Null pointer");
		return 0;
	}

	CGrowthPetSystem* pPetSystem = info->pPetSystem;
	if (nullptr == pPetSystem)
		return 0;

	pPetSystem->Update(0);
	return PASSES_PER_SEC(1) / 4;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CGrowthPetSystemActor
///////////////////////////////////////////////////////////////////////////////////////

CGrowthPetSystemActor::CGrowthPetSystemActor(LPCHARACTER owner, uint32_t vnum, uint32_t options)
{
	m_dwRaceVnum = vnum;
	m_dwRaceVID = 0;

	m_dwOptions = options;
	m_dwLastActionTime = 0;

	m_pkGrowthPet = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;

	//m_dwSummonItemVID = 0;
}

CGrowthPetSystemActor::~CGrowthPetSystemActor()
{
	Dismiss();
	m_pkOwner = 0;
}

void CGrowthPetSystemActor::UpdateFlashEvent() noexcept
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return;

	m_PetInfo.flash_event = FLASH_EVENT_NONE;
	if (m_PetInfo.exp_monster == m_PetInfo.next_exp_monster && m_PetInfo.exp_item == m_PetInfo.next_exp_item)
	{
		if (m_PetInfo.evol_level < 3)
			m_PetInfo.flash_event = FLASH_EVENT_EVOLVE;
	}
	else if (m_PetInfo.exp_monster == m_PetInfo.next_exp_monster && m_PetInfo.exp_item < m_PetInfo.next_exp_item)
	{
		m_PetInfo.flash_event = FLASH_EVENT_EXP_ITEM;
	}

	if (m_PetInfo.evol_level == 3 && GetPetAgeDays() >= 30)
		m_PetInfo.flash_event = FLASH_EVENT_EVOLVE;
}

namespace PetEvolveItem
{
	extern std::map<uint32_t, uint8_t> GetEvolutionItems(int evolveStage)
	{
		if (evolveStage == 1)
			return { {55003, 10}, /*Junges Pet-Buch*/ {30058, 10}, /*Spinnen-Eiersack*/ {30073, 10}, /*Weißes Haarband+*/ {30041, 10}, /*Shuriken*/ {30017, 5}, /*Schmuckende Haarnadel*/ {30074, 5}, /*Schwarze Uniform+*/ {30088, 5}, /*Eisstück+*/ };
		else if (evolveStage == 2)
			return { {55004, 10}, /*Wildes Pet-Buch*/ {27994, 2}, /*Blutrote Perle*/ {30035, 10}, /*Gesichtscreme*/ {30089, 10}, /*Yetifell+*/ {30031, 10}, /*Schmuckgegenstand*/ {30011, 5}, /*Knauel*/ {30080, 5} /*Fluchsammlung+*/ };
		else if (evolveStage == 3)
			return { {55005, 10}, /*Tapferes Pet-Buch*/ {30083, 2}, /*Unbekannte Medizin+*/ {27992, 2}, /*Weiße Perle*/ {27993, 2}, /*Blaue Perle*/ {30086, 10}, /*Damonenandenken+*/ {30077, 10}, /*Orkzahn+*/ {30550, 5} /*Blauer Riemen*/ };

		return {};
	}
};

bool CGrowthPetSystemActor::ItemCubeFeed(uint8_t bIndex, const uint16_t iFeedItemsCubeSlot[9], uint16_t wFeedItemsCount)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	LPITEM pFeedItem;
	for (uint32_t i = 0; i < wFeedItemsCount; ++i)
	{
		pFeedItem = GetOwner()->GetInventoryItem(iFeedItemsCubeSlot[i]);

		if (!pFeedItem)
			return false;
	}

	if (m_pkOwner->GetExchange() || m_pkOwner->GetMyShop()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| m_pkOwner->GetViewingShopOwner()
#else
		|| m_pkOwner->GetShopOwner()
#endif
		|| m_pkOwner->IsOpenSafebox() || m_pkOwner->IsCubeOpen()
		)
	{
		return false;
	}

	if (pFeedItem->GetID() == m_pkPetSeal->GetID())
		return false;

	if (pFeedItem->isLocked())
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ëÁßÀÎ ¾ÆÀÌÅÛÀº °³ÀÎ»óÁ¡¿¡¼­ ÆÇ¸ÅÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (IS_SET(pFeedItem->GetAntiFlag(), ITEM_ANTIFLAG_PETFEED))
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ëÁßÀÎ ¾ÆÀÌÅÛÀº °³ÀÎ»óÁ¡¿¡¼­ ÆÇ¸ÅÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

#ifdef ENABLE_SEALBIND_SYSTEM
	if (pFeedItem->IsSealed())
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1332]");
		return false;
	}
#endif

	// Feed Life
	if (bIndex == PET_FEED_WINDOW)
	{
		if (pFeedItem->GetType() == ITEM_PET && (pFeedItem->GetSubType() == PET_EGG || pFeedItem->GetSubType() == PET_UPBRINGING))
		{
			const long lPetDuration = m_PetInfo.pet_max_time;
			const long lMaxPetLife = time(0) + lPetDuration;

			const long lFeedDuration = (lPetDuration * number(3, 5)) / 100;
			const long lCurrentPetTime = m_pkPetSeal->GetSocket(0);

			if (lCurrentPetTime < lMaxPetLife)
			{
				long lNewPetDuration = 0;
				if ((lCurrentPetTime + lFeedDuration) < lMaxPetLife)
					lNewPetDuration = lCurrentPetTime + lFeedDuration;
				else
					lNewPetDuration = lMaxPetLife;

				m_pkPetSeal->SetSocket(0, lNewPetDuration);
				m_PetInfo.pet_end_time = lNewPetDuration;

				m_pkOwner->SetGrowthPetInfo(m_PetInfo);
				m_pkOwner->SendGrowthPetInfoPacket();

				if (pFeedItem->GetSubType() == PET_UPBRINGING)
					ITEM_MANAGER::Instance().DestroyItem(pFeedItem);
				else
					ITEM_MANAGER::Instance().RemoveItem(pFeedItem, "PET_FEED_LIFE");
			}
		}

		return true;
	}

	// Feed Evolve
	if (bIndex == PET_EVOLVE_WINDOW)
	{
		const uint32_t bPetEvolution = GetEvolution();

		if (bPetEvolution == PET_MAX_EVOLVE) {
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1170]");
			return false;
		}

		if (!CanIncreaseEvolvePet())
		{
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot evolve your pet yet.")); // Already checked by pythonside
			return false;
		}

		std::map<uint32_t, uint8_t> EvItems = PetEvolveItem::GetEvolutionItems(bPetEvolution);

		const size_t itemlistcnt = EvItems.size();
		if (itemlistcnt > PET_MAX_FEED_SLOT) {
			sys_err("GrowthPet Evolve: Too much Materials in list.");
		}

		int iItemFound = 0;
		for (int i = 0; i < itemlistcnt; ++i)
		{
			const int slotIndex = iFeedItemsCubeSlot[i];
			const LPITEM item = GetOwner()->GetInventoryItem(slotIndex);
			if (!item)
				break;

			(EvItems.find(item->GetVnum()) != EvItems.end() && GetOwner()->CountSpecifyItem(item->GetVnum()) >= EvItems[item->GetVnum()]) ? ++iItemFound : --iItemFound;
		}

		if (iItemFound != itemlistcnt)
		{
			int fullslot = 0;
			for (const auto& v : EvItems)
			{
				const uint32_t itemVnum = v.first;
				const uint8_t itemCount = v.second;
				if (GetOwner()->CountSpecifyItem(itemVnum) < itemCount) {
					GetOwner()->ChatPacket(CHAT_TYPE_INFO, "[LS;1195;[IN;%d];%d] ", itemVnum, itemCount - GetOwner()->CountSpecifyItem(itemVnum));
				}
				else
					++fullslot;
			}

			if (fullslot == itemlistcnt) {
				GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Put the materials into your evolve window!"));
				--fullslot;
			}

			return false;
		}

		if (iItemFound == itemlistcnt)
		{
			for (int i = 0; i < PET_MAX_FEED_SLOT; ++i)
			{
				const int slotIndex = iFeedItemsCubeSlot[i];
				if (GetOwner()->GetInventoryItem(slotIndex))
				{
					const uint32_t itemVNum = GetOwner()->GetInventoryItem(slotIndex)->GetVnum();
					GetOwner()->RemoveSpecifyItem(itemVNum, EvItems[itemVNum]);
					//ITEM_MANAGER::Instance().RemoveItem(pFeedItem, "PET_FEED_EVOLVE");
				}
			}

			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You've taught your pet well! It's quickly learning new skills!"));
			EvolvePet();
		}

		return true;
	}

	// Feed Exp
	if (bIndex == PET_EXP_ITEM_WINDOW)
	{
		if (pFeedItem->GetType() == ITEM_PET && (pFeedItem->GetSubType() == PET_EXPFOOD || pFeedItem->GetSubType() == PET_EXPFOOD_PER))
		{
			if (pFeedItem->GetSubType() == PET_EXPFOOD)
			{
				if (!SetExp(pFeedItem->GetShopBuyPrice(), PET_EXP_FROM_MOB))
					return false;

				// Log
				char szHint[64];
				snprintf(szHint, sizeof(szHint), "%s %u %u", pFeedItem->GetName(), 1, pFeedItem->GetCount());
				LogManager::Instance().ItemLog(m_pkOwner, pFeedItem, "PET_FEED_ITEM", szHint);

				// Remove Item
				pFeedItem->SetCount(pFeedItem->GetCount() - 1);

				return true;
			}

			if (pFeedItem->GetSubType() == PET_EXPFOOD_PER)
			{
				long lExpPerValue = (GetNeedExpFromItem() * pFeedItem->GetShopBuyPrice()) / 100;
				if (lExpPerValue == 0)
					lExpPerValue = 1;

				if (!SetExp(lExpPerValue, PET_EXP_FROM_ITEM))
					return false;

				// Log
				char szHint[64];
				snprintf(szHint, sizeof(szHint), "%s %u %u", pFeedItem->GetName(), 1, pFeedItem->GetCount());
				LogManager::Instance().ItemLog(m_pkOwner, pFeedItem, "PET_FEED_ITEM", szHint);

				// Remove Item
				pFeedItem->SetCount(pFeedItem->GetCount() - 1);

				return true;
			}

			return false;
		}
		else if (pFeedItem->GetType() == ITEM_WEAPON || pFeedItem->GetType() == ITEM_ARMOR)
		{
			if (m_PetInfo.exp_item < GetNeedExpFromItem())
			{
				if (!SetExp(pFeedItem->GetShopBuyPrice() / 2, PET_EXP_FROM_ITEM))
					return false;

				ITEM_MANAGER::Instance().RemoveItem(pFeedItem, "PET_FEED_ITEM");
				return true;
			}
			else
			{
				m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1157;%s]", m_pkGrowthPet->GetName());
				return false;
			}
		}
	}

	return false;
}

bool CGrowthPetSystemActor::LearnPetSkill(uint8_t bSkillBookSlotIndex, uint8_t bSkillBookInvenIndex)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	// Check Level and Evolution
	if (GetPetLevel() < 82 && GetEvolution() < 3)
		return false;

	// Check Skill Slot
	if (m_PetInfo.skill_vnum[bSkillBookSlotIndex] > 0)
		return false;

	LPITEM petSkillBook = m_pkOwner->GetInventoryItem(bSkillBookInvenIndex);
	if (!petSkillBook)
		return false;

	const uint8_t skill_value = petSkillBook->GetValue(0);

	for (int i = 0; i < 3; ++i)
	{
		if (m_PetInfo.skill_vnum[i] == skill_value)
		{
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1176]");
			return false;
		}
	}

	if (m_PetInfo.skill_count < (bSkillBookSlotIndex + 1))
		return false;

	m_PetInfo.skill_vnum[bSkillBookSlotIndex] = skill_value;
	m_PetInfo.skill_level[bSkillBookSlotIndex] = 1;

	// skill specialist
	float specialist_value = 0;
	GetGrowthPetSkillSpecialistValue(m_pkPetSeal->GetVnum(), skill_value, &specialist_value);
	m_PetInfo.skill_spec[bSkillBookSlotIndex] = specialist_value;

	ITEM_MANAGER::Instance().RemoveItem(petSkillBook, "PET_LEARN_SKILL");

	GiveBuff();

	return true;
}

bool CGrowthPetSystemActor::PetSkillUpgradeRequest(uint8_t bSkillSlot, uint8_t bSkillIndex)
{
	// Check Skill Level
	if (m_PetInfo.skill_level[bSkillSlot] >= PET_MAX_SKILL_POINTS)
		return false;

	// Check Gold
	if (m_pkOwner->GetGold() < static_cast<int>(SKILL_UPGRADE_PRICE))
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1173]");
		return false;
	}

	// Send Packet Result
	m_pkOwner->SendGrowthPetUpgradeSkillRequest(bSkillSlot, bSkillIndex, SKILL_UPGRADE_PRICE);

	return true;
}

bool CGrowthPetSystemActor::IncreasePetSkill(uint8_t bSkillSlot)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	// Check Skill Level
	if (m_PetInfo.skill_level[bSkillSlot] >= PET_MAX_SKILL_POINTS)
		return false;

	// Check Gold
	if (m_pkOwner->GetGold() < static_cast<int>(SKILL_UPGRADE_PRICE))
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1173]");
		return false;
	}

	// Increase Skill
	m_PetInfo.skill_level[bSkillSlot]++;

	// Remove Gold
	m_pkOwner->PointChange(POINT_GOLD, -SKILL_UPGRADE_PRICE, false);

	GiveBuff();

	m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1174]");

	return true;
}

bool CGrowthPetSystemActor::CanIncreasePetLevel()
{
	if ((GetPetLevel() == 40 && GetEvolution() == 1) || (GetPetLevel() == 80 && GetEvolution() == 2))
		return false;
	else
		return true;

	return false;
}

bool CGrowthPetSystemActor::CanIncreaseEvolvePet()
{
	if (GetPetLevel() == 40 && GetEvolution() == 1)
		return true;

	if (GetPetLevel() >= 80 && GetEvolution() == 2)
		return true;

	if (GetPetLevel() >= 80 && GetEvolution() == 3 && GetPetBirthday() >= 30)
		return true;

	return false;
}

bool CGrowthPetSystemActor::EvolvePet()
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	if (!CanIncreaseEvolvePet())
		return false;

	if (GetEvolution() >= PET_MAX_EVOLVE)
		return false;

	// Change Race
	if (GetEvolution() >= PET_MAX_EVOLVE - 1)
	{
		m_pkGrowthPet->EvolvePetRace(GetRaceVnum() + 1);
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1170]");
	}
	else
	{
		const char* sEvolveName[] =
		{
			LC_TEXT("[1197]the level Young"),
			LC_TEXT("[1198]the level Wild"),
			LC_TEXT("[1199]the level Valiant"),
			LC_TEXT("[1200]the level Heroic")
		};

		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1156;%s;%d]", m_pkPetSeal->GetName(), sEvolveName[GetEvolution()]);

		if (GetEvolution() == 2)
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1178]");
	}

	if (GetEvolution() < PET_MAX_EVOLVE)
		m_PetInfo.evol_level++;

	if (GetPetLevel() < PET_MAX_LEVEL)
		SetPetLevel(m_PetInfo.pet_level + 1);

	m_PetInfo.exp_monster = 0;
	m_PetInfo.exp_item = 0;

	return true;
}

bool CGrowthPetSystemActor::DeleteSkill(uint8_t bSkillBookDelSlotIndex, uint8_t bSkillBookDelInvenIndex)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	LPITEM itemPetMaterial = m_pkOwner->GetInventoryItem(bSkillBookDelInvenIndex);
	if (!itemPetMaterial)
		return false;

	if (itemPetMaterial->GetType() != ITEM_PET || itemPetMaterial->GetSubType() != PET_SKILL_DEL_BOOK)
		return false;

	if (!m_PetInfo.skill_vnum[bSkillBookDelSlotIndex])
		return false;

	m_PetInfo.skill_vnum[bSkillBookDelSlotIndex] = 0;
	m_PetInfo.skill_level[bSkillBookDelSlotIndex] = 0;
	m_PetInfo.skill_spec[bSkillBookDelSlotIndex] = 0;
	m_PetInfo.skill_cool[bSkillBookDelSlotIndex] = 0;

	ITEM_MANAGER::Instance().RemoveItem(itemPetMaterial, "PET_DEL_SKILL");

	GiveBuff();

	return true;
}

bool CGrowthPetSystemActor::DeleteAllSkill(uint8_t bPetSkillAllDelBookIndex)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	LPITEM itemPetMaterial = m_pkOwner->GetInventoryItem(bPetSkillAllDelBookIndex);
	if (!itemPetMaterial)
		return false;

	if (itemPetMaterial->GetType() != ITEM_PET || itemPetMaterial->GetSubType() != PET_SKILL_ALL_DEL_BOOK)
		return false;

	for (uint8_t i = 0; i < 3; ++i)
	{
		m_PetInfo.skill_vnum[i] = 0;
		m_PetInfo.skill_level[i] = 0;
		m_PetInfo.skill_spec[i] = 0;
		m_PetInfo.skill_cool[i] = 0;
	}

	ITEM_MANAGER::Instance().RemoveItem(itemPetMaterial, "PET_DEL_ALL_SKILL");

	GiveBuff(); // Update Pet Bonus

	return true;
}

#ifdef ENABLE_PET_ATTR_DETERMINE
bool CGrowthPetSystemActor::Determine()
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	const auto iDestinyOrbCount = m_pkOwner->CountSpecifyItem(55032);
	if (iDestinyOrbCount <= 0)
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1342]");
		return false;
	}

	const auto bPetType = m_PetInfo.pet_type;

	// Send Packet
	TPacketGCGrowthPetAttrDetermineResult sPacket;
	sPacket.bHeader = HEADER_GC_GROWTH_ATTR_DETERMINE_RESULT;
	sPacket.bResult = bPetType;

	if (m_pkOwner->GetDesc())
	{
		m_pkOwner->RemoveSpecifyItem(55032, 1); // remove destiny orb
		m_pkOwner->GetDesc()->Packet(&sPacket, sizeof(TPacketGCGrowthPetAttrDetermineResult)); // send packet
		return true;
	}

	return false;
}
#endif

#ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
void CHARACTER::RevivePet(const TPacketCGGrowthPetReviveRequest* revivePacket, LPITEM pSummonItem)
{
	for (int i = 0; i < PET_REVIVE_MATERIAL_SLOT_MAX; ++i)
	{
		LPITEM itemPet = GetInventoryItem(pSummonItem->GetCell());	// Pet
		if (!itemPet)
			return;

		LPITEM material = GetInventoryItem(revivePacket->pos[i]);	// Material
		if (!material)
			return;

		if (revivePacket->count[i] == 0)
			break;

		if (itemPet->GetType() == ITEM_PET && itemPet->GetSubType() != PET_UPBRINGING)
			return;

		if (material->GetType() == ITEM_PET && material->GetSubType() != PET_PREMIUM_FEEDSTUFF)
			return;

		TGrowthPetInfo info = itemPet->GetGrowthPetItemInfo();

		time_t petAge = pSummonItem->GetSocket(0) - info.pet_birthday;

		uint32_t materialCount = 0;
		materialCount += revivePacket->count[i];

		uint32_t requiredMaterialCount = petAge / (3600 * 24 * 10);	// Required Material depend on Age of Pet
		if (!requiredMaterialCount)
			requiredMaterialCount = 1;

		// Items count check
		if (materialCount < requiredMaterialCount)
		{
			TPacketGCGrowthPetReviveResult packet{};
			packet.bHeader = HEADER_GC_GROWTH_PET_REVIVE_RESULT;
			packet.bResult = false;
			GetDesc()->Packet(&packet, sizeof(packet));
			sys_err("Not enought materials");
			return;
		}
		else // Remove required number of items
		{
			material->SetCount(material->GetCount() - requiredMaterialCount);
			requiredMaterialCount = 0;
		}

		if (!requiredMaterialCount)
		{
			Revive(pSummonItem, PET_REVIVE_TYPE_PREMIUM);

			TPacketGCGrowthPetReviveResult packet{};
			packet.bHeader = HEADER_GC_GROWTH_PET_REVIVE_RESULT;
			packet.bResult = true;
			GetDesc()->Packet(&packet, sizeof(packet));
		}
	}
}

/*
	Takes dead pet upbringing item pointer and revive type
	as arguments. Revives pet based on type.
*/
void CHARACTER::Revive(LPITEM pSummonItem, uint8_t bType)
{
	if (!pSummonItem)
		return;

	LPITEM itemPet = GetInventoryItem(pSummonItem->GetCell());
	TGrowthPetInfo info = itemPet->GetGrowthPetItemInfo();

	time_t petAge = pSummonItem->GetSocket(0) - info.pet_birthday;
	uint32_t dwMaxDuration = info.pet_max_time;

	float fDurationFactor = 0.5f;
	if (bType == PET_REVIVE_TYPE_PREMIUM)
		fDurationFactor = 1.0f;
	dwMaxDuration *= fDurationFactor;


	info.pet_max_time = time(0) + dwMaxDuration;
	pSummonItem->SetSocket(0, time(0) + dwMaxDuration);
	pSummonItem->StartRealTimeExpireEvent();

	float fAgeFactor = 0.5f;
	if (bType == PET_REVIVE_TYPE_PREMIUM)
		fAgeFactor = 0.8f;

	if (bType == PET_REVIVE_TYPE_PREMIUM)
		info.pet_birthday = time(0) - (petAge * fAgeFactor);
	else
	{
		info.pet_birthday = time(0);
		ChatPacket(CHAT_TYPE_INFO, "Your pet enthusiastically tucks into the Protein Snack.");
	}
}
#endif

uint32_t CGrowthPetSystemActor::GetNextExpFromTable()
{
	if (105 < GetPetLevel())
		return 2500000000u;
	else
		return exp_pet_table[GetPetLevel()];
}

void CGrowthPetSystemActor::SetPetLevel(uint8_t bLevel)
{
	if (!m_pkOwner || !m_pkGrowthPet)
		return;

	if (bLevel <= 0 || bLevel > PET_MAX_LEVEL)
		return;

	m_PetInfo.exp_monster = 0;
	m_PetInfo.exp_item = 0;

	float fIncreaseAttrByAge = 0.0f;
	const float fGetValueIncrease = (static_cast<float>(GetPetAgeDays()) / 1000);
	if (GetPetAgeDays() >= 60)
		fIncreaseAttrByAge = (GetPetAgeDays() < 100) ? 0.1f : trunc(fGetValueIncrease * 10) / 10;

	if (bLevel % int(PET_HP_RANGE[m_PetInfo.pet_type][2]) == 0)
		m_PetInfo.pet_hp += fIncreaseAttrByAge + fnumber(PET_HP_RANGE[m_PetInfo.pet_type][0], PET_HP_RANGE[m_PetInfo.pet_type][1]);

	if (bLevel % int(PET_DEF_RANGE[m_PetInfo.pet_type][2]) == 0)
		m_PetInfo.pet_def += fIncreaseAttrByAge + fnumber(PET_DEF_RANGE[m_PetInfo.pet_type][0], PET_DEF_RANGE[m_PetInfo.pet_type][1]);

	if (bLevel % int(PET_SP_RANGE[m_PetInfo.pet_type][2]) == 0)
		m_PetInfo.pet_sp += fIncreaseAttrByAge + fnumber(PET_SP_RANGE[m_PetInfo.pet_type][0], PET_SP_RANGE[m_PetInfo.pet_type][1]);

	// Set New Level
	m_PetInfo.pet_level = bLevel;

	// Update Flash
	UpdateFlashEvent();

	// Update Pet Char Informations
	m_pkGrowthPet->SetLevel(bLevel);
	m_pkGrowthPet->SendPetLevelUpEffect(m_pkGrowthPet->GetVID(), bLevel);

	// Compute Skill
	m_pkOwner->SetGrowthPetInfo(m_PetInfo);
	m_pkOwner->SendGrowthPetInfoPacket();

	m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1148;%s;%d]", m_pkPetSeal->GetName(), GetPetLevel());
}

bool CGrowthPetSystemActor::SetExp(uint32_t dwExpValue, uint8_t bExpMode)
{
	// debug
	if (test_server)
		m_pkOwner->ChatPacket(CHAT_TYPE_TALKING, "SetExp called (%ld, %d)", dwExpValue, bExpMode);

	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	if (dwExpValue <= 0)
		return false;

	if (GetPetLevel() >= PET_MAX_LEVEL)
		return false;

	switch (bExpMode)
	{
		// Exp Monster
		case PET_EXP_FROM_MOB:
		{
			if (m_PetInfo.exp_monster >= m_PetInfo.next_exp_monster)
				return false;

			const uint32_t remain = m_PetInfo.next_exp_monster - m_PetInfo.exp_monster;
			m_PetInfo.exp_monster += (dwExpValue > remain ? remain : dwExpValue);
			break;
		}
		// Exp Item
		case PET_EXP_FROM_ITEM:
		{
			if (m_PetInfo.exp_item >= m_PetInfo.next_exp_item)
				return false;

			const uint32_t remain = m_PetInfo.next_exp_item - m_PetInfo.exp_item;
			m_PetInfo.exp_item += (dwExpValue >= remain ? remain : dwExpValue);
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1175;%d]", (dwExpValue >= remain ? remain : dwExpValue));
			break;
		}
		default:
			return false;
	}

	if (m_PetInfo.exp_monster >= m_PetInfo.next_exp_monster && m_PetInfo.exp_item < m_PetInfo.next_exp_item)
	{
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1168]");
	}

	// Evolve Pet
	if (m_PetInfo.exp_monster >= m_PetInfo.next_exp_monster && m_PetInfo.exp_item >= m_PetInfo.next_exp_item)
	{
		// Level Up
		if (CanIncreasePetLevel())
			SetPetLevel(m_PetInfo.pet_level + 1);
		else
			m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1169]");
	}

	UpdateFlashEvent();
	m_pkOwner->SetGrowthPetInfo(m_PetInfo);
	m_pkOwner->SendGrowthPetInfoPacket();

	return true;
}

bool CGrowthPetSystemActor::SetBirthday(uint32_t bBirthday)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	if (bBirthday <= 0)
		return false;

	const long time = 60 * 60 * 24 * bBirthday;
	m_PetInfo.pet_birthday -= time;

	m_pkOwner->SetGrowthPetInfo(m_PetInfo);
	m_pkOwner->SendGrowthPetInfoPacket();

	m_pkGrowthPet->SendPetLevelUpEffect(m_pkGrowthPet->GetVID(), GetPetLevel());

	return true;
}

bool CGrowthPetSystemActor::Dismiss()
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	ClearBuff();

	m_PetInfo.is_summoned = false;

	m_pkPetSeal->Lock(false);
	m_pkPetSeal->SetGrowthPetItemInfo(m_PetInfo);
	m_pkPetSeal->Save();

	m_pkPetSeal = nullptr;

	if (m_pkOwner)
	{
		m_pkOwner->SetGrowthPetInfo(m_PetInfo);
		m_pkOwner->SendGrowthPetInfoPacket();

		m_pkOwner->ComputePoints();

		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1147;%s]", m_PetInfo.pet_nick);
	}
	if (m_pkGrowthPet)
		M2_DESTROY_CHARACTER(m_pkGrowthPet);

	m_pkGrowthPet = nullptr;
	m_dwRaceVID = 0;

	m_PetInfo = {};

	return true;
}

bool CGrowthPetSystemActor::ChangeName(const char* sName)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	strcpy(m_PetInfo.pet_nick, sName);

	m_pkGrowthPet->SetName(sName);
	m_pkGrowthPet->ViewReencode();

	return true;
}

uint32_t CGrowthPetSystemActor::Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar)
{
	if (!pSummonItem)
	{
		sys_err("[CGrowthPetSystemActor::Summon] Failed to summon pet, cant find item.");
		return 0;
	}

	long x = m_pkOwner->GetX();
	long y = m_pkOwner->GetY();

	if (bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (0 != m_pkGrowthPet)
	{
		m_pkGrowthPet->Show(m_pkOwner->GetMapIndex(), x, y);
		m_dwRaceVID = m_pkGrowthPet->GetVID();

		return m_dwRaceVID;
	}

	const int iRotation = (int)(m_pkOwner->GetRotation() + 180);
	m_pkGrowthPet = CHARACTER_MANAGER::Instance().SpawnMob(GetRaceVnum(), m_pkOwner->GetMapIndex(), x, y, m_pkOwner->GetZ(), false, iRotation, false);
	if (!m_pkGrowthPet)
	{
		sys_err("[CGrowthPetSystemActor::Summon] Failed to summon the pet. (vnum: %d)", GetRaceVnum());
		return 0;
	}

	m_pkGrowthPet->SetGrowthPet();
	m_pkGrowthPet->SetEmpire(m_pkOwner->GetEmpire());

	m_dwRaceVID = m_pkGrowthPet->GetVID();

	m_pkPetSeal = pSummonItem;

	m_PetInfo = pSummonItem->GetGrowthPetItemInfo();

	m_pkGrowthPet->SetName(m_PetInfo.pet_nick);
	m_pkGrowthPet->SetLevel(m_PetInfo.pet_level);

	m_PetInfo.is_summoned = true;
	m_PetInfo.next_exp_monster = GetNeedExpFromMonster();
	m_PetInfo.next_exp_item = GetNeedExpFromItem();
	m_PetInfo.pet_end_time = pSummonItem->GetSocket(0);

	SetNeedExpFromMonster(GetNextExpFromTable());

	UpdateFlashEvent();

	GiveBuff();

	for (uint8_t slot = 0; slot < 3; ++slot)
	{
		const auto skill_vnum = m_PetInfo.skill_vnum[slot];
		TPetSkillTable table = pet_skill_table[skill_vnum][0];
		const auto skill_type = table.skill_type;

		switch (skill_type)
		{
			case PET_SKILL_USE_TYPE_PASSIVE:
			{
				//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1182]%s effect activated."), LC_LOCALE_PET_SKILL_TEXT(skill_vnum));
			}
			break;
			case PET_SKILL_USE_TYPE_AUTO:
			{
				//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1184]%s effect not activated."), LC_LOCALE_PET_SKILL_TEXT(skill_vnum));
			}
			break;
			default:
				break;
		}
	}

	m_pkOwner->ComputePoints();

	m_pkOwner->ChatPacket(CHAT_TYPE_INFO, "[LS;1146;%s]", m_PetInfo.pet_nick);

	pSummonItem->Lock(true);

	return m_dwRaceVID;
}

bool CGrowthPetSystemActor::_UpdateFollowAI()
{
	if (0 == m_pkGrowthPet->m_pkMobData)
		return false;

	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(GetRaceVnum());

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}

	constexpr float START_FOLLOW_DISTANCE = 300.0f;
	constexpr float START_RUN_DISTANCE = 900.0f;

	constexpr float RESPAWN_DISTANCE = 4500.f;
	constexpr int  APPROACH = 290;

	const uint32_t currentTime = get_dword_time();

	const long ownerX = m_pkOwner->GetX();
	const long ownerY = m_pkOwner->GetY();

	const long charX = m_pkGrowthPet->GetX();
	const long charY = m_pkGrowthPet->GetY();

	const float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		const float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		const float fx = -APPROACH * cos(fOwnerRot);
		const float fy = -APPROACH * sin(fOwnerRot);

		if (m_pkGrowthPet->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			return true;
		}
	}

	if (fDist >= START_FOLLOW_DISTANCE)
	{
		bool bRun = false;
		if (fDist >= START_RUN_DISTANCE)
			bRun = true;

		m_pkGrowthPet->SetNowWalking(!bRun);

		Follow(APPROACH);

		m_pkGrowthPet->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else
		m_pkGrowthPet->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	return true;
}

bool CGrowthPetSystemActor::Update(uint32_t deltaTime)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	if (m_pkPetSeal->GetOwner() != GetOwner())
		return false;

	if ((m_pkOwner->GetLevel() < GetPetLevel()) || (m_pkGrowthPet->IsDead()) || (IsDead(m_pkPetSeal)))
	{
		Dismiss();
		return false;
	}

	if (IsActivePassiveSkill())
	{
		for (uint8_t slot = 0; slot < 3; ++slot)
		{
			switch (m_PetInfo.skill_vnum[slot])
			{
				case PET_SKILL_AFFECT_HEAL:
				{
					/* need to review */
					if (m_pkOwner && (m_PetInfo.skill_cool[slot] < time(0)) && (m_pkOwner->GetHPPct() < 20))
					{
						const int rand = number(1, 100);
						const int value = (int)m_PetInfo.skill_formula1[slot];
						if (rand <= value)
						{
							m_PetInfo.skill_cool[slot] = get_global_time() + 480;

							const int restore_hp = MIN((m_pkOwner->GetHP() + (int)m_PetInfo.skill_spec[slot]), m_pkOwner->GetMaxHP());

							m_pkOwner->PointChange(POINT_HP, restore_hp);
							m_pkOwner->EffectPacket(SE_HPUP_RED);

							// Update Pet Info
							m_pkOwner->SetGrowthPetInfo(m_PetInfo);
							m_pkOwner->SendGrowthPetInfoPacket();

							//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Restoration skill activated. HP restored by %d."), (int)m_PetInfo.skill_spec[slot]);
							//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1177]%s skill activated. HP restored by %d."), LC_LOCALE_PET_SKILL_TEXT((int)m_PetInfo.skill_vnum[slot]), restore_hp);

							// debug
							if (test_server)
								m_pkOwner->ChatPacket(CHAT_TYPE_PARTY, "Heal! recovered %1.f (%d) hp.", m_PetInfo.skill_formula1[slot], restore_hp);
						}
					}

					break;
				}
				case PET_SKILL_AFFECT_INVINCIBILITY:
				{
					if (m_pkOwner && (m_PetInfo.skill_cool[slot] < time(0)) && (m_pkOwner->GetHPPct() < 10))
					{
						if (!m_pkOwner->FindAffect(AFFECT_IMPOSSIBLE_ATTACK))
						{
							const int rand = number(1, 100);
							const int value = (int)m_PetInfo.next_skill_formula1[slot];
							if (rand <= value)
							{
								m_PetInfo.skill_cool[slot] = get_global_time() + 600;

								//const long duration = m_dwSkillDuration[slot];
								const long duration = 123; //review

								m_pkOwner->AddAffect(AFFECT_IMPOSSIBLE_ATTACK, APPLY_NONE, 10, AFF_NONE, duration, 0, false, false);
								//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1185]%s skill activated. You will now receive no damage."), LC_LOCALE_PET_SKILL_TEXT((int)m_PetInfo.skill_vnum[slot]));

								// debug
								if (test_server)
									m_pkOwner->ChatPacket(CHAT_TYPE_PARTY, "Invincibility actived, probably: %d, duration: %ld", value, duration);
							}
						}

					}

					break;
				}
				case PET_SKILL_AFFECT_REMOVAL:
				{
					if (m_pkOwner && (m_PetInfo.skill_cool[slot] < time(0)))
					{
						const int rand = number(1, 100);
						const int value = (int)m_PetInfo.next_skill_formula1[slot];
						if (rand <= value)
						{
							bool bIsBadAffect = false;

							if (m_pkOwner->IsAffectFlag(AFF_STUN))
								bIsBadAffect = true;
							if (m_pkOwner->IsAffectFlag(AFF_SLOW))
								bIsBadAffect = true;
							if (m_pkOwner->IsAffectFlag(AFF_POISON))
								bIsBadAffect = true;
							if (m_pkOwner->IsAffectFlag(AFF_FIRE))
								bIsBadAffect = true;
#ifdef ENABLE_WOLFMAN_CHARACTER
							if (m_pkOwner->IsAffectFlag(AFF_BLEEDING))
								bIsBadAffect = true;
#endif

							if (bIsBadAffect)
							{
								m_PetInfo.skill_cool[slot] = get_global_time() + 480;
								m_pkOwner->RemoveBadAffect();

								//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1186]%s skill activated. Slow, poison and bleeding effects were removed."), LC_LOCALE_PET_SKILL_TEXT((int)m_PetInfo.skill_vnum[slot]));

								// debug
								if (test_server)
									m_pkOwner->ChatPacket(CHAT_TYPE_PARTY, "Panacea actived, negative effects removed!");
							}
						}
					}

					break;
				}
				case PET_SKILL_FEATHER:
				{
					return false;
					// under construction
					//m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[1401]%s's pet skill has been activated. %s uses Feather Walk."), LC_LOCALE_PET_SKILL_TEXT((int)m_PetInfo.skill_vnum[slot]));
					break;
				}

				default:
					break;
			}
		}
	}

	bool bResult = true;
	if (HasOption(EPetOption_Followable))
		bResult = bResult && _UpdateFollowAI();

	return bResult;
}

bool CGrowthPetSystemActor::Follow(float fMinDistance)
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return false;

	const float fOwnerX = m_pkOwner->GetX();
	const float fOwnerY = m_pkOwner->GetY();

	const float fPetX = m_pkGrowthPet->GetX();
	const float fPetY = m_pkGrowthPet->GetY();

	const float fDist = DISTANCE_SQRT(fOwnerX - fPetX, fOwnerY - fPetY);
	if (fDist <= fMinDistance)
		return false;

	m_pkGrowthPet->SetRotationToXY(fOwnerX, fOwnerY);

	float fx = 0.0f, fy = 0.0f;
	const float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkGrowthPet->GetRotation(), fDistToGo, &fx, &fy);

	const long posX = (long)(fPetX + fx + 0.5f);
	const long posY = (long)(fPetY + fy + 0.5f);
	if (!m_pkGrowthPet->Goto(posX, posY))
		return false;

	m_pkGrowthPet->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

TPetSkillTable CGrowthPetSystemActor::GetPetSkillInformation(uint8_t skill_slot)
{
	const auto skill_vnum = m_PetInfo.skill_vnum[skill_slot];
	const auto skill_level = m_PetInfo.skill_level[skill_slot];
	const auto skill_spec = m_PetInfo.pet_type >= 5 ? m_PetInfo.skill_spec[skill_slot] : 0;

	TPetSkillTable table = pet_skill_table[skill_vnum][0];
	const auto skill_type = table.skill_type;
	const auto skill_apply = table.skill_apply;

	float skill_formula1 = 0.0f;
	float next_skill_formula1 = 0.0f;

	float skill_formula2 = 0.0f;
	float next_skill_formula2 = 0.0f;

	switch (table.skill_type)
	{
		case PET_SKILL_USE_TYPE_PASSIVE:
		{
			const float skill_value = skill_spec ? skill_spec : table.skill_formula1;
			skill_formula2 = (skill_value / PET_MAX_SKILL_POINTS) * skill_level;
			next_skill_formula2 = (skill_value / PET_MAX_SKILL_POINTS) * (skill_level + 1);

			if (skill_spec)
			{
				skill_formula2 = (skill_spec / PET_MAX_SKILL_POINTS) * skill_level;
				next_skill_formula2 = (skill_spec / PET_MAX_SKILL_POINTS) * (skill_level + 1);
			}

			// debug
			/*sys_err("founded passive: %.1f %.1f : %.1f %.1f %.1f %.1f",
				skill_formula2, next_skill_formula2,
				table.skill_formula1, table.next_skill_formula1,
				table.skill_formula2, table.next_skill_formula2);*/

			break;
		}
		case PET_SKILL_USE_TYPE_AUTO:
		{
			skill_formula1 = (table.skill_formula1 / PET_MAX_SKILL_POINTS) * skill_level;
			next_skill_formula1 = (table.skill_formula1 / PET_MAX_SKILL_POINTS) * (skill_level + 1);

			const float skill_value = skill_spec ? skill_spec : table.next_skill_formula1;
			skill_formula2 = (skill_value / PET_MAX_SKILL_POINTS) * skill_level;
			next_skill_formula2 = (skill_value / PET_MAX_SKILL_POINTS) * (skill_level + 1);

			// debug
			/*sys_err("founded auto: %.1f %.1f %.1f %.1f : %.1f %.1f %.1f %.1f",
				skill_formula1, next_skill_formula1,
				skill_formula2, next_skill_formula2,
				table.skill_formula1, table.next_skill_formula1,
				table.skill_formula2, table.next_skill_formula2);*/

			break;
		}
		default:
			break;
	}

	if (GetPetLevel() < PET_MAX_LEVEL)
	{
		const float decrease_value = (PET_MAX_LEVEL - GetPetLevel()) / 5;

		skill_formula1 -= decrease_value;
		if (skill_formula1 < 0)
			skill_formula1 = 0;

		next_skill_formula1 -= decrease_value;
		if (next_skill_formula1 < 0)
			next_skill_formula1 = 0;

		skill_formula2 -= decrease_value;
		if (skill_formula2 < 0)
			skill_formula2 = 0;

		next_skill_formula2 -= decrease_value;
		if (next_skill_formula2 < 0)
			next_skill_formula2 = 0;

		// debug
		/*sys_err("decreased to: %.1f %.1f %.1f %.1f",
			skill_formula1, next_skill_formula1,
			skill_formula2, next_skill_formula2);*/
	}

	table.skill_vnum = skill_vnum;
	table.skill_type = skill_type;
	table.skill_apply = skill_apply;

	table.skill_formula1 = skill_formula1;
	table.next_skill_formula1 = next_skill_formula1;

	table.skill_formula2 = skill_formula2;
	table.next_skill_formula2 = next_skill_formula2;

	return table;
}
// END_OF_TESTE

void CGrowthPetSystemActor::GiveBuff()
{
	if (!m_pkOwner || !m_pkGrowthPet || !m_pkPetSeal)
		return;

	// clear affect
	m_pkOwner->RemoveAffect(AFFECT_GROWTH_PET); // deprecated

	// pet status
	const auto expire_time = m_pkPetSeal->GetSocket(0);

	const auto hp_value = (int)(m_pkOwner->GetMaxHP() * GetPetHP() / 1000);
	const auto sp_value = (int)(m_pkOwner->GetMaxSP() * GetPetSP() / 1000);
	const auto def_value = (int)(m_pkOwner->GetPoint(POINT_DEF_GRADE) * GetPetDEF() / 1000);

	m_pkOwner->AddAffect(AFFECT_GROWTH_PET, aApplyInfo[APPLY_MAX_HP].wPointType, hp_value, 0, expire_time, 0, false);
	m_pkOwner->AddAffect(AFFECT_GROWTH_PET, aApplyInfo[APPLY_MAX_SP].wPointType, sp_value, 0, expire_time, 0, false);
	m_pkOwner->AddAffect(AFFECT_GROWTH_PET, aApplyInfo[APPLY_DEF_GRADE_BONUS].wPointType, def_value, 0, expire_time, 0, false);

	// pet_skill
	if (GetPetLevel() >= 81 && GetEvolution() >= 3)
	{
		// debug
		if (test_server)
			m_pkOwner->ChatPacket(CHAT_TYPE_TALKING, "GrowthPetSystemActor::GiveBuff -> SKILL~");

		for (auto i = 0; i < m_PetInfo.skill_count; ++i)
		{
			if (!m_PetInfo.skill_vnum[i])
				continue;

			// debug
			if (test_server)
			{
				m_pkOwner->ChatPacket(CHAT_TYPE_TALKING, "GrowthPetSystemActor::GiveBuff -> SKILL (slot: %d, vnum: %d, value: %d)",
					i, m_PetInfo.skill_vnum[i], m_PetInfo.skill_level[i]);
			}

			const auto table = GetPetSkillInformation(i);

			switch (table.skill_type)
			{
				case PET_SKILL_USE_TYPE_PASSIVE:
				{
					if (table.skill_formula2)
					{
						const long lApplyValue = table.skill_formula2;
						m_pkOwner->AddAffect(AFFECT_GROWTH_PET, aApplyInfo[table.skill_apply].wPointType, lApplyValue, 0, expire_time, 0, false);
					}

					m_PetInfo.skill_formula2[i] = table.skill_formula2;
					m_PetInfo.next_skill_formula2[i] = table.next_skill_formula2;

					sys_err("PASSIVE formula2: (%.1f) (%.1f)", table.skill_formula2, table.next_skill_formula2);

					break;
				}
				case PET_SKILL_USE_TYPE_AUTO:
					ActivePassiveSkill(true);

					m_PetInfo.skill_formula1[i] = table.skill_formula1;
					m_PetInfo.next_skill_formula1[i] = table.next_skill_formula1;

					m_PetInfo.skill_formula2[i] = table.skill_formula2;
					m_PetInfo.next_skill_formula2[i] = table.next_skill_formula2;

					sys_err("AUTO: formula (%.1f) (%.1f) (%.1f) (%.1f)",
						table.skill_formula1, table.next_skill_formula1,
						table.skill_formula2, table.next_skill_formula2);

					break;
				default:
					break;
			}

		}

	}

	m_pkOwner->SetGrowthPetInfo(m_PetInfo);
	m_pkOwner->SendGrowthPetInfoPacket();
}

void CGrowthPetSystemActor::ClearBuff()
{
	m_pkOwner->RemoveAffect(AFFECT_GROWTH_PET);
}

bool CGrowthPetSystemActor::IsDead(LPITEM petSeal)
{
	if (!petSeal)
		return false;

	const long duration = (petSeal->GetSocket(0) - get_global_time()) / 60;
	if (duration < 0)
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////
//  CGrowthPetSystem
///////////////////////////////////////////////////////////////////////////////////////

CGrowthPetSystem::CGrowthPetSystem(LPCHARACTER owner)
{
	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;
	m_dwLastUpdateTime = 0;
}

CGrowthPetSystem::~CGrowthPetSystem()
{
	Destroy();
}

////////////////////////////////////////////////////////////

bool CGrowthPetSystem::IsDead(LPITEM petSeal)
{
	if (!petSeal)
		return false;

	const long duration = (petSeal->GetSocket(0) - get_global_time()) / 60;
	if (duration < 0)
		return true;

	return false;
}

bool CGrowthPetSystem::LearnPetSkill(uint8_t bSkillBookSlotIndex, uint8_t bSkillBookInvenIndex)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
		{
			return petActor->LearnPetSkill(bSkillBookSlotIndex, bSkillBookInvenIndex);
		}
	}
	return false;
}

bool CGrowthPetSystem::PetSkillUpgradeRequest(uint8_t bSkillSlot, uint8_t bSkillIndex)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
		{
			return petActor->PetSkillUpgradeRequest(bSkillSlot, bSkillIndex);
		}
	}
	return false;
}

bool CGrowthPetSystem::IncreasePetSkill(uint8_t bSkillSlot, uint8_t bSkillBookInvenIndex)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
		{
			return petActor->IncreasePetSkill((bSkillSlot));
		}
	}
	return false;
}

void CGrowthPetSystem::Destroy()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor != 0)
			delete petActor;
	}

	event_cancel(&m_pkGrowthPetSystemUpdateEvent);

	m_petActorMap.clear();
}

bool CGrowthPetSystem::Update(uint32_t deltaTime)
{
	const uint32_t currentTime = get_dword_time();

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	bool bResult = true;
	std::vector <CGrowthPetSystemActor*> v_garbageActor;
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;

		if (petActor && petActor->GetGrowthPet())
		{
			if (nullptr == CHARACTER_MANAGER::Instance().Find(petActor->GetGrowthPet()->GetVID()))
			{
				v_garbageActor.emplace_back(petActor);
			}
			else
			{
				bResult = bResult && petActor->Update(deltaTime);
			}
		}
	}

	for (std::vector<CGrowthPetSystemActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeletePet(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CGrowthPetSystem::DeletePet(uint32_t mobVnum)
{
	const TGrowthPetActorMap::iterator iter = m_petActorMap.find(mobVnum);

	if (m_petActorMap.end() == iter)
	{
		sys_err("[CGrowthPetSystem::DeletePet] Can't find pet on my list (VNUM: %d)", mobVnum);
		return;
	}

	CGrowthPetSystemActor* petActor = iter->second;
	if (!petActor)
		sys_err("[CGrowthPetSystem::DeletePet] Null Pointer (petActor)");
	else
		delete petActor;

	m_petActorMap.erase(iter);
}

void CGrowthPetSystem::DeletePet(CGrowthPetSystemActor* petActor)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		if (iter->second == petActor)
		{
			delete petActor;
			m_petActorMap.erase(iter);
			return;
		}
	}

	sys_err("[CGrowthPetSystem::DeletePet] Can't find petActor(0x%x) on my list(size: %d) ", petActor, m_petActorMap.size());
}

void CGrowthPetSystem::Dismiss(uint32_t vnum, bool bDeleteFromList)
{
	CGrowthPetSystemActor* petActor = this->GetByVnum(vnum);

	if (!petActor)
	{
		sys_err("[CGrowthPetSystem::GetByVnum(%d)] Null Pointer (petActor)", vnum);
		return;
	}

	petActor->Dismiss();

	if (true == bDeleteFromList)
		this->DeletePet(petActor);

	for (TGrowthPetActorMap::iterator it = m_petActorMap.begin(); it != m_petActorMap.end(); it++)
	{
		if (it->second->GetGrowthPet())
		{
			event_cancel(&m_pkGrowthPetSystemUpdateEvent);
			m_pkGrowthPetSystemUpdateEvent = nullptr;
			break;
		}
	}
}

bool CGrowthPetSystem::IsActivePet()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		const CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return true;
	}
	return false;
}

void CGrowthPetSystem::SetExp(uint32_t dExpValue, uint8_t bExpMode)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
		{
			petActor->SetExp(dExpValue, bExpMode);
			break;
		}
	}
}

void CGrowthPetSystem::SetLevel(uint8_t bLevel)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
		{
			//petActor->SetLevel(bLevel);
			petActor->SetPetLevel(bLevel);
			break;
		}
	}
}

void CGrowthPetSystem::SetBirthday(uint32_t bBirthday)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
		{
			petActor->SetBirthday(bBirthday);
			break;
		}
	}
}

bool CGrowthPetSystem::ItemCubeFeed(uint8_t bIndex, const uint16_t wFeedItemsCubeSlot[9], uint16_t wFeedItemsCount)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->ItemCubeFeed(bIndex, wFeedItemsCubeSlot, wFeedItemsCount);
	}

	return false;
}

bool CGrowthPetSystem::EvolvePet()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->EvolvePet();
	}

	return false;
}

bool CGrowthPetSystem::DeleteSkill(uint8_t bSkillBookDelSlotIndex, uint8_t bSkillBookDelInvenIndex)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->DeleteSkill(bSkillBookDelSlotIndex, bSkillBookDelInvenIndex);
	}

	return false;
}

bool CGrowthPetSystem::DeleteAllSkill(uint8_t bPetSkillAllDelBookIndex)
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->DeleteAllSkill(bPetSkillAllDelBookIndex);
	}

	return false;
}

#ifdef ENABLE_PET_ATTR_DETERMINE
bool CGrowthPetSystem::Determine()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->Determine();
	}

	return false;
}
#endif

uint32_t CGrowthPetSystem::GetLevel()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->GetPetLevel();
	}

	return 0;
}

uint32_t CGrowthPetSystem::GetEvolution()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->GetEvolution();
	}

	return 0;
}

bool CGrowthPetSystem::CanEvolve()
{
	for (TGrowthPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;
		if (petActor && petActor->GetGrowthPet())
			return petActor->CanIncreaseEvolvePet();
	}

	return false;
}

CGrowthPetSystemActor* CGrowthPetSystem::Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options)
{
	CGrowthPetSystemActor* petActor = this->GetByVnum(mobVnum);
	if (!petActor)
	{
		petActor = new CGrowthPetSystemActor(m_pkOwner, mobVnum, options);
		m_petActorMap.insert(std::make_pair(mobVnum, petActor));
	}

	const uint32_t petVID = petActor->Summon(petName, pSummonItem, bSpawnFar);
#ifdef ENABLE_NEWSTUFF
	if (!petVID)
		sys_err("[CGrowthPetSystem::Summon(%d)] Null Pointer (petVID)", pSummonItem);
#endif

	if (nullptr == m_pkGrowthPetSystemUpdateEvent)
	{
		GrowthPetSystem_Event_Info* info = AllocEventInfo<GrowthPetSystem_Event_Info>();
		info->pPetSystem = this;
		m_pkGrowthPetSystemUpdateEvent = event_create(GrowthPetSystem_Update_Event, info, PASSES_PER_SEC(1) / 4);
	}

	return petActor;
}


CGrowthPetSystemActor* CGrowthPetSystem::GetByVID(uint32_t vid) const
{
	CGrowthPetSystemActor* petActor = 0;

	bool bFound = false;

	for (TGrowthPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		petActor = iter->second;
		if (!petActor)
		{
			sys_err("[CPetSystem::GetByVID(%d)] Null Pointer (petActor)", vid);
			continue;
		}

		bFound = petActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? petActor : 0;
}

CGrowthPetSystemActor* CGrowthPetSystem::GetByVnum(uint32_t vnum) const
{
	CGrowthPetSystemActor* petActor = 0;

	const TGrowthPetActorMap::const_iterator iter = m_petActorMap.find(vnum);

	if (m_petActorMap.end() != iter)
		petActor = iter->second;

	return petActor;
}

size_t CGrowthPetSystem::CountSummoned() const
{
	size_t count = 0;

	for (TGrowthPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		const CGrowthPetSystemActor* petActor = iter->second;

		if (petActor && petActor->GetGrowthPet())
			++count;
	}

	return count;
}

bool CGrowthPetSystem::Dismiss()
{
	for (TGrowthPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;

		if (petActor && petActor->GetGrowthPet())
			return petActor->Dismiss();
	}
	return false;
}

bool CGrowthPetSystem::ChangeName(const char* sName)
{
	for (TGrowthPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CGrowthPetSystemActor* petActor = iter->second;

		if (petActor && petActor->GetGrowthPet())
			return petActor->ChangeName(sName);
	}
	return false;
}
#endif
