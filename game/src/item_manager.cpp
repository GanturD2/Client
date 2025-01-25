#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "desc_client.h"
#include "db.h"
#include "log.h"
#include "skill.h"
#include "text_file_loader.h"
#include "priv_manager.h"
#include "questmanager.h"
#include "unique_item.h"
#include "safebox.h"
#include "blend_item.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
#	include "guild.h"
#endif
#include "item_manager.h"
#include "../../common/VnumHelper.h"
#include "DragonSoul.h"
#ifndef ENABLE_CUBE_RENEWAL
#include "cube.h"
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
#	include "shop.h"
#endif
#ifdef ENABLE_INGAME_WIKI
#	include "refine.h"
#endif
#ifdef ENABLE_YOHARA_SYSTEM
#	include "RandomHelper.h"
#endif

#ifdef ENABLE_EVENT_MANAGER
#	include "event_manager.h"
#endif

ITEM_MANAGER::ITEM_MANAGER()
	: m_iTopOfTable(0), m_dwVIDCount(0), m_dwCurrentID(0)
{
	m_ItemIDRange.dwMin = m_ItemIDRange.dwMax = m_ItemIDRange.dwUsableItemIDMin = 0;
	m_ItemIDSpareRange.dwMin = m_ItemIDSpareRange.dwMax = m_ItemIDSpareRange.dwUsableItemIDMin = 0;
}

ITEM_MANAGER::~ITEM_MANAGER()
{
	Destroy();
}

void ITEM_MANAGER::Destroy()
{
	auto it = m_VIDMap.begin();
	for (; it != m_VIDMap.end(); ++it)
	{
		M2_DELETE(it->second);
	}
	m_VIDMap.clear();
}

void ITEM_MANAGER::GracefulShutdown()
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.begin();

	while (it != m_set_pkItemForDelayedSave.end())
		SaveSingleItem(*(it++));

	m_set_pkItemForDelayedSave.clear();
}

bool ITEM_MANAGER::Initialize(TItemTable* table, int size)
{
	if (!m_vec_prototype.empty())
		m_vec_prototype.clear();

	int i = 0;

	m_vec_prototype.resize(size);
	thecore_memcpy(&m_vec_prototype[0], table, sizeof(TItemTable) * size);
	for (i = 0; i < size; i++)
	{
		if (0 != m_vec_prototype[i].dwVnumRange)
		{
			m_vec_item_vnum_range_info.emplace_back(&m_vec_prototype[i]);
		}
	}

	m_map_ItemRefineFrom.clear();
	for (i = 0; i < size; ++i)
	{
		if (m_vec_prototype[i].dwRefinedVnum)
			m_map_ItemRefineFrom.insert(std::make_pair(m_vec_prototype[i].dwRefinedVnum, m_vec_prototype[i].dwVnum));

		// NOTE: The QUEST_GIVE flag occurs as an npc event.
		if (m_vec_prototype[i].bType == ITEM_QUEST || IS_SET(m_vec_prototype[i].dwFlags, ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
			quest::CQuestManager::Instance().RegisterNPCVnum(m_vec_prototype[i].dwVnum);

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		if (m_vec_prototype[i].bType == ITEM_COSTUME && m_vec_prototype[i].bSubType == COSTUME_MOUNT)
			quest::CQuestManager::Instance().RegisterNPCVnum(m_vec_prototype[i].dwVnum);
#endif

#if defined(ENABLE_PET_SYSTEM) || defined(ENABLE_GROWTH_PET_SYSTEM)
		if (m_vec_prototype[i].bType == ITEM_PET)
			quest::CQuestManager::Instance().RegisterNPCVnum(m_vec_prototype[i].dwVnum);
#endif

		m_map_vid.insert(std::map<uint32_t, TItemTable>::value_type(m_vec_prototype[i].dwVnum, m_vec_prototype[i]));
		if (test_server)
			sys_log(0, "ITEM_INFO %d %s ", m_vec_prototype[i].dwVnum, m_vec_prototype[i].szName);
	}

	int len = 0, len2 = 0;
	char buf[512] = { '\0' };

	for (i = 0; i < size; ++i)
	{
		len2 = snprintf(buf + len, sizeof(buf) - len, "%5u %-16s", m_vec_prototype[i].dwVnum, m_vec_prototype[i].szLocaleName);

		if (len2 < 0 || len2 >= (int)sizeof(buf) - len)
			len += (sizeof(buf) - len) - 1;
		else
			len += len2;

		if (!((i + 1) % 4))
		{
			if (!test_server)
				sys_log(0, "%s", buf);

			len = 0;
		}
		else
		{
			buf[len++] = '\t';
			buf[len] = '\0';
		}
	}

	if ((i + 1) % 4)
	{
		if (!test_server)
			sys_log(0, "%s", buf);
	}

	ITEM_VID_MAP::iterator it = m_VIDMap.begin();

	sys_log(1, "ITEM_VID_MAP %d", m_VIDMap.size());

	while (it != m_VIDMap.end())
	{
		LPITEM item = it->second;
		++it;

		const TItemTable* tableInfo = GetTable(item->GetOriginalVnum());

		if (nullptr == tableInfo)
		{
			sys_err("cannot reset item table");
			item->SetProto(nullptr);
		}

		item->SetProto(tableInfo);
	}

	return true;
}

LPITEM ITEM_MANAGER::CreateItem(uint32_t vnum, uint32_t count, uint32_t id, bool bTryMagic, int iRarePct, bool bSkipSave)
{
	if (0 == vnum)
		return nullptr;

	uint32_t dwMaskVnum = 0;
	if (GetMaskVnum(vnum))
	{
		dwMaskVnum = GetMaskVnum(vnum);
	}

	const TItemTable* table = GetTable(vnum);

	if (nullptr == table)
		return nullptr;

	LPITEM item = nullptr;

	//If it exists by checking by id - return!
	if (m_map_pkItemByID.find(id) != m_map_pkItemByID.end())
	{
		item = m_map_pkItemByID[id];
		if (item)
		{
			LPCHARACTER owner = item->GetOwner();
			sys_err("ITEM_ID_DUP: %u %s owner %p", id, item->GetName(), get_pointer(owner));
		}

		return nullptr;
	}

	//Allocate one item
	item = M2_NEW CItem(vnum);

	const bool bIsNewItem = (0 == id);

	//Initialize it. Three tables
	item->Initialize();
	item->SetProto(table);
	item->SetMaskVnum(dwMaskVnum);
#ifdef ENABLE_SOUL_SYSTEM
	if (item->GetType() == ITEM_SOUL)
		item->SetSocket(2, item->GetValue(2));
#endif

#ifdef ENABLE_SEALBIND_SYSTEM
	if (bIsNewItem)
		item->SetSealDate(0);
#endif

	if (item->GetType() == ITEM_ELK) // Yang doesn't need an ID and doesn't need storage.
		item->SetSkipSave(true);

	// Let's set a unique ID
	else if (!bIsNewItem)
	{
		item->SetID(id);
		item->SetSkipSave(true);
	}
	else
	{
		item->SetID(GetNewID());

		bool bCheck = false;

		if (item->GetType() == ITEM_UNIQUE)
			bCheck = true;

		if (item->GetType() == ITEM_RING)
			bCheck = true;

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
		if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_MOUNT)
			bCheck = true;
#endif

		if (bCheck)
		{
			if (item->GetValue(2) == 0)
				item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, item->GetValue(0)); // Game time unique
			else
			{
				//int globalTime = get_global_time();
				//int lastTime = item->GetValue(0);
				//int endTime = get_global_time() + item->GetValue(0);
				item->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, get_global_time() + item->GetValue(0)); // Real-time unique
			}
		}
	}

#ifdef ENABLE_ANTI_EXP_RING
	switch (item->GetVnum())
	{
		case ITEM_AUTO_EXP_RING_S:
		case ITEM_AUTO_EXP_RING_M:
		case ITEM_AUTO_EXP_RING_G:
			if (bIsNewItem)
				item->SetSocket(1, item->GetValue(0), true);
			else
				item->SetSocket(1, item->GetValue(0), false);
			break;
	}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	if (item->GetApplyType(0) == APPLY_RANDOM || item->GetApplyType(1) == APPLY_RANDOM || item->GetApplyType(2) == APPLY_RANDOM)
		CRandomHelper::GenerateRandomAttr(item);
#endif

	switch (item->GetVnum())
	{
		case ITEM_AUTO_HP_RECOVERY_S:
		case ITEM_AUTO_HP_RECOVERY_M:
		case ITEM_AUTO_HP_RECOVERY_L:
		case ITEM_AUTO_HP_RECOVERY_X:
		case ITEM_AUTO_SP_RECOVERY_S:
		case ITEM_AUTO_SP_RECOVERY_M:
		case ITEM_AUTO_SP_RECOVERY_L:
		case ITEM_AUTO_SP_RECOVERY_X:
		case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
		case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
		case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
		case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
			{
				if (bIsNewItem)
					item->SetSocket(2, item->GetValue(0), true);
				else
					item->SetSocket(2, item->GetValue(0), false);
			}
			break;

		default:
			break;
	}

	if (item->GetType() == ITEM_ELK) // Yangs needs no processing
		;
	else if (item->IsStackable()) // For items that can be combined
	{
		count = MINMAX(1, count, g_bItemCountLimit);

		if (bTryMagic && count <= 1 && IS_SET(item->GetFlag(), ITEM_FLAG_MAKECOUNT))
			count = item->GetValue(1);
	}
	else
		count = 1;

	item->SetVID(++m_dwVIDCount);

	if (bSkipSave == false)
		m_VIDMap.insert(ITEM_VID_MAP::value_type(item->GetVID(), item));

	if (item->GetID() != 0 && bSkipSave == false)
		m_map_pkItemByID.insert(std::map<uint32_t, LPITEM>::value_type(item->GetID(), item));

	if (!item->SetCount(count))
		return nullptr;

	item->SetSkipSave(false);

	if (item->GetType() == ITEM_UNIQUE && item->GetValue(2) != 0)
		item->StartUniqueExpireEvent();

	if (item->GetType() == ITEM_RING && item->GetValue(2) != 0)
		item->StartUniqueExpireEvent();

#ifdef ENABLE_CHANGE_LOOK_MOUNT
	if (item->IsHorseSummonItem() && item->GetChangeLookVnum() && item->GetSocket(2))
		item->StartChangeLookExpireEvent();
#endif

#ifdef ENABLE_SOUL_SYSTEM
	if (item->GetType() == ITEM_SOUL && item->GetLimitValue(1) != 0)
		item->StartSoulTimeUseEvent();
#endif

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; i++)
	{
		// Time is deducted from the time of item creation even if it is not used
		if (LIMIT_REAL_TIME == item->GetLimitType(i))
		{
			if (item->GetLimitValue(i))
				item->SetSocket(0, time(0) + item->GetLimitValue(i));
			else
				item->SetSocket(0, time(0) + 60 * 60 * 24 * 7);

			item->StartRealTimeExpireEvent();
		}

		// As with existing unique items, the available time is deducted only when worn
		else if (LIMIT_TIMER_BASED_ON_WEAR == item->GetLimitType(i))
		{
			// If the item is already worn, the timer starts, and the new item sets the available time. (
			// In the case of payment to the Item Mall, the Socket0 value must be set before entering this logic.
			if (true == item->IsEquipped())
			{
				item->StartTimerBasedOnWearExpireEvent();
			}
			else if (0 == id)
			{
				long duration = item->GetSocket(0);
				if (0 == duration)
					duration = item->GetLimitValue(i);

				if (0 == duration)
					duration = 60 * 60 * 10; // If there is no information, the default setting is 10 hours

				item->SetSocket(0, duration);
			}
		}
	}

#ifdef ENABLE_BATTLE_FIELD
	// Item Gacha
	if (item->GetType() == ITEM_GACHA)
		item->SetSocket(0, item->GetLimitValue(1));
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	if (item->GetVnum() == 79603 || item->GetVnum() == 79604)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("enable_catch_king_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("catch_king_event_end_day");

			if (iEndTime)
			{
				item->SetSocket(0, iEndTime);
			}
		}
	}
#endif

	if (id == 0) // Process only when newly created
	{
		// In the case of newly added herbs, the performance is treated differently
		if (ITEM_BLEND == item->GetType())
		{
			if (Blend_Item_find(item->GetVnum()))
			{
				Blend_Item_set_value(item);
				return item;
			}
		}

		if (table->sAddonType)
		{
			item->ApplyAddon(table->sAddonType);
		}

		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_REINFORCE))
			bTryMagic = false;

		if (bTryMagic)
		{
			if (iRarePct == -1)
				iRarePct = item->GetMagicPct();

			if (number(1, 100) <= iRarePct)
				item->AlterToMagicItem();
		}

		if (table->bGainSocketPct)
			item->AlterToSocketItem(table->bGainSocketPct);

		if (vnum == 50300 || vnum == ITEM_SKILLFORGET_VNUM)	//Fertigkeitsbuch || Buch des Vergessens
		{
			if (vnum == ITEM_SKILLFORGET_VNUM)
			{
#ifdef ENABLE_78TH_SKILL
				if (number(1, 100) >= 70)
				{
					uint32_t dwSkillVnum;
					if (number(1, 100) >= 50)
					{
					#if defined(ENABLE_78TH_SKILL) && defined(ENABLE_WOLFMAN_CHARACTER)
						dwSkillVnum = number(221, 229);
					#else
						dwSkillVnum = number(221, 228);
					#endif
					}
					else
					{
					#if defined(ENABLE_78TH_SKILL) && defined(ENABLE_WOLFMAN_CHARACTER)
						dwSkillVnum = number(236, 244);
					#else
						dwSkillVnum = number(236, 243);
					#endif
					}

					item->SetSocket(0, dwSkillVnum);
				}
				else
#endif
				{
#ifdef ENABLE_NINETH_SKILL
					if (number(1, 100) >= 70)
					{
						extern const uint32_t GetRandomForgetSkillVnum(uint8_t bJob = JOB_MAX_NUM);
						item->SetSocket(0, GetRandomForgetSkillVnum());
					}
					else
#endif
					{
						extern const uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);
						item->SetSocket(0, GetRandomSkillVnum());
					}
				}
			}
			else
			{
				extern const uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);
				item->SetSocket(0, GetRandomSkillVnum());
			}
		}
		else if (ITEM_SKILLFORGET2_VNUM == vnum)
		{
			uint32_t dwSkillVnum;

			do
			{
				dwSkillVnum = number(112, 119);

				if (nullptr != CSkillManager::Instance().Get(dwSkillVnum))
					break;
			} while (true);

			item->SetSocket(0, dwSkillVnum);
		}
	}
	else
	{
		// There is a 100% chance that the attribute should be attached, but if not, it will be newly attached. ...............
		if (100 == item->GetMagicPct() && 0 == item->GetAttributeCount())
		{
			item->AlterToMagicItem();
		}
	}

	if (item->GetType() == ITEM_QUEST)
	{
		for (auto& it : m_map_pkQuestItemGroup)
		{
			if (it.second->m_bType == CSpecialItemGroup::QUEST && it.second->Contains(vnum))
				item->SetSIGVnum(it.first);
		}
	}
	else if (item->GetType() == ITEM_UNIQUE)
	{
		for (auto& it : m_map_pkSpecialItemGroup)
		{
			if (it.second->m_bType == CSpecialItemGroup::SPECIAL && it.second->Contains(vnum))
				item->SetSIGVnum(it.first);
		}
	}
	else if (item->GetType() == ITEM_RING)	//@fixme400
	{
		for (auto& it : m_map_pkSpecialItemGroup)
		{
			if (it.second->m_bType == CSpecialItemGroup::SPECIAL && it.second->Contains(vnum))
				item->SetSIGVnum(it.first);
		}
	}

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM	//Extended
	if (item->GetSubType() == COSTUME_MOUNT)
	{
		for (auto& it : m_map_pkSpecialItemGroup)
		{
			if (it.second->m_bType == CSpecialItemGroup::SPECIAL && it.second->Contains(vnum))
				item->SetSIGVnum(it.first);
		}
	}
#endif

#ifdef ENABLE_GACHA_SYSTEM
	else if (item->GetType() == ITEM_GACHA)
		item->SetSocket(0, item->GetLimitValue(1) * count);
#endif

	// Newly generated dragon spirit stone treatment.
	if (item->IsDragonSoul() && 0 == id)
	{
		DSManager::Instance().DragonSoulItemInitialize(item);
	}

#ifdef ENABLE_AURA_SYSTEM
	if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_AURA)
	{
		const uint8_t c_bGrade = item->GetOriginalVnum() % 10;
		const uint8_t c_bBaseLevel = GetAuraRefineInfo(c_bGrade, AURA_REFINE_INFO_LEVEL_MIN);
		const uint16_t c_wBaseExp = 0;
		item->SetSocket(ITEM_SOCKET_AURA_CURRENT_LEVEL, (1000 + c_bBaseLevel) * 100000 + c_wBaseExp);
	}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_ACCE)
	{
		if ((item->GetApplyType(0) == APPLY_ACCEDRAIN_RATE) && (item->GetSocket(1) == 0))
		{
			const long socket_value = item->GetApplyValue(0) == 20 ? number(11, 19) : item->GetApplyValue(0);
			item->SetSocket(1, socket_value);
		}
	}
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	if (item->GetVnum() == 79505 || item->GetVnum() == 79506)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("mini_game_okey_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("okey_event_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
	if (item->GetVnum() == YUT_NORI_DROP_ITEM || item->GetVnum() == YUT_NORI_PLAY_ITEM)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("mini_game_yutnori_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("yut_nori_event_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

#ifdef ENABLE_EASTER_EVENT
	if (item->GetVnum() == MAGIC_EGG_VNUM)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("enable_easter_event"))
			item->SetSocket(0, MAGIC_EGG_TOTAL_UNPACK);
	}
#endif

#ifdef ENABLE_SUMMER_EVENT
	if (item->GetVnum() == 50278 || item->GetVnum() == 50279)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("e_summer_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("e_summer_event_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

#ifdef ENABLE_2016_VALENTINE
	if (item->GetVnum() == 900128)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("valentine_drop_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("valentine_drop_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	if (item->GetVnum() == CATCH_KING_DROP_ITEM || item->GetVnum() == CATCH_KING_PLAY_ITEM)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("mini_game_catchking_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("catch_king_event_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
	if (item->GetVnum() == FIND_M_DROP_ITEM || item->GetVnum() == FIND_M_PLAY_ITEM)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("mini_game_findm_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("findm_event_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

#ifdef ENABLE_MINI_GAME_BNW
	if (item->GetVnum() == BNW_DROP_ITEM || item->GetVnum() == BNW_PLAY_ITEM || item->GetVnum() == BNW_REWARD_VNUM)
	{
		if (quest::CQuestManager::Instance().GetEventFlag("mini_game_bnw_event"))
		{
			int iEndTime = quest::CQuestManager::Instance().GetEventFlag("bnw_event_end_day");
			if (iEndTime)
				item->SetSocket(0, iEndTime);
		}
	}
#endif

	// @fixme400
	if (0 != item->GetSIGVnum())
	{
		const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::Instance().GetSpecialItemGroup(item->GetSIGVnum());
		if (nullptr == pItemGroup)
			return item;

		uint32_t dwAttrVnum = pItemGroup->GetAttrVnum(item->GetVnum());
		const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::Instance().GetSpecialAttrGroup(dwAttrVnum);
		if (nullptr == pAttrGroup)
			return item;

		int count = 0;
		for (auto it : pAttrGroup->m_vecAttrs) {
			item->SetForceAttribute(count, it.apply_type, it.apply_value);
		}
	}
	// @fixme400

	return item;
}

void ITEM_MANAGER::DelayedSave(LPITEM item)
{
	if (!item)
		return;

	if (item->GetID() != 0)
		m_set_pkItemForDelayedSave.insert(item);
}

void ITEM_MANAGER::FlushDelayedSave(LPITEM item)
{
	const std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.find(item);

	if (it == m_set_pkItemForDelayedSave.end())
	{
		return;
	}

	m_set_pkItemForDelayedSave.erase(it);
	SaveSingleItem(item);
}

void ITEM_MANAGER::SaveSingleItem(LPITEM item)
{
	if (!item)
		return;


	if (ITEM_MANAGER::Instance().FindByVID(item->GetVID()) == nullptr)
	{
		sys_err("Nullptr on SaveSingleItem: %d", item->GetVnum());
		return;
	}

	if (!item->GetOwner())
	{
		const uint32_t dwID = item->GetID();
		const uint32_t dwOwnerID = item->GetLastOwnerPID();

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(uint32_t) + sizeof(uint32_t));
		db_clientdesc->Packet(&dwID, sizeof(uint32_t));
		db_clientdesc->Packet(&dwOwnerID, sizeof(uint32_t));

		sys_log(1, "ITEM_DELETE %s:%u", item->GetName(), dwID);
		return;
	}

	sys_log(1, "ITEM_SAVE %s:%d in %s window %d", item->GetName(), item->GetID(), item->GetOwner()->GetName(), item->GetWindow());

	TPlayerItem t{};

	t.id = item->GetID();
	t.window = item->GetWindow();
	t.pos = item->GetCell();

	t.count = item->GetCount();
	t.vnum = item->GetOriginalVnum();
#ifdef ENABLE_SEALBIND_SYSTEM
	t.nSealDate = item->GetSealDate();
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	t.dwTransmutationVnum = item->GetChangeLookVnum();
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	t.is_basic = item->IsBasicItem();
#endif
#ifdef ENABLE_REFINE_ELEMENT
	t.grade_element = item->GetElementGrade();
	thecore_memcpy(t.attack_element, item->GetElementAttacks(), sizeof(t.attack_element));
	t.element_type_bonus = item->GetElementsType();
	thecore_memcpy(t.elements_value_bonus, item->GetElementsValues(), sizeof(t.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
	t.set_value = item->GetItemSetValue();
#endif

	switch (t.window)
	{
		case SAFEBOX:
		case MALL:
			t.owner = item->GetOwner()->GetDesc()->GetAccountTable().id;
			break;

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		case GUILDBANK:
			t.owner = item->GetOwner()->GetGuild()->GetID();
			break;
#endif

		default:
			t.owner = item->GetOwner()->GetPlayerID();
			break;
	}
	thecore_memcpy(t.alSockets, item->GetSockets(), sizeof(t.alSockets));
	thecore_memcpy(t.aAttr, item->GetAttributes(), sizeof(t.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	thecore_memcpy(t.aApplyRandom, item->GetApplysRandom(), sizeof(t.aApplyRandom));
	thecore_memcpy(t.alRandomValues, item->GetRandomDefaultAttrs(), sizeof(t.alRandomValues));
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	t.aPetInfo = item->GetGrowthPetItemInfo();
#endif

	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_SAVE, 0, sizeof(TPlayerItem));
	db_clientdesc->Packet(&t, sizeof(TPlayerItem));
}

void ITEM_MANAGER::Update()
{
	std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.begin();
	std::unordered_set<LPITEM>::iterator this_it;

	while (it != m_set_pkItemForDelayedSave.end())
	{
		this_it = it++;
		LPITEM item = *this_it;

		if (!item)
		{
			sys_err("1. Can not find item, prevent crash core!");
			m_set_pkItemForDelayedSave.erase(this_it);
			continue;
		}

		if (ITEM_MANAGER::Instance().FindByVID(item->GetVID()) == nullptr)
		{
			sys_err("2. Can not find item, prevent crash core!");
			m_set_pkItemForDelayedSave.erase(this_it);
			continue;
		}

		// Anything with the SLOW_QUERY flag is saved only at the end.
		if (item->GetOwner() && IS_SET(item->GetFlag(), ITEM_FLAG_SLOW_QUERY))
			continue;

		SaveSingleItem(item);

		m_set_pkItemForDelayedSave.erase(this_it);
	}
}

void ITEM_MANAGER::RemoveItem(LPITEM item, const char* c_pszReason)
{
	if (!item)
		return;

	LPCHARACTER o;

	if ((o = item->GetOwner()))
	{
		char szHint[64];
		snprintf(szHint, sizeof(szHint), "%s %u ", item->GetName(), item->GetCount());
		LogManager::Instance().ItemLog(o, item, c_pszReason ? c_pszReason : "REMOVE", szHint);

		// SAFEBOX_TIME_LIMIT_ITEM_BUG_FIX
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		if (item->GetWindow() == MALL || item->GetWindow() == SAFEBOX || item->GetWindow() == GUILDBANK)
#else
		if (item->GetWindow() == MALL || item->GetWindow() == SAFEBOX)
#endif
		{
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
			CSafebox* pSafebox = item->GetWindow() == MALL ? o->GetMall() : (item->GetWindow() == GUILDBANK ? o->GetGuildstorage() : o->GetSafebox());
#else
			CSafebox* pSafebox = item->GetWindow() == MALL ? o->GetMall() : o->GetSafebox();
#endif
			if (pSafebox)
			{
				pSafebox->Remove(static_cast<uint8_t>(item->GetCell()));
			}
		}
		// END_OF_SAFEBOX_TIME_LIMIT_ITEM_BUG_FIX
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		else if (item->GetWindow() == PREMIUM_PRIVATE_SHOP)
		{
			if (LPSHOP shop = o->GetMyShop())
			{
				shop->RemoveItemByID(item->GetID());
				//item->RemoveFromCharacter();
			}
		}
#endif
		else
		{
			// Deleted from Quickslot
			if (item->GetWindow() == INVENTORY)
				o->SyncQuickslot(QUICKSLOT_TYPE_ITEM, static_cast<uint8_t>(item->GetCell()), QUICKSLOT_MAX_POS);
			else if (item->GetWindow() == BELT_INVENTORY)
				o->SyncQuickslot(QUICKSLOT_TYPE_BELT, static_cast<uint8_t>(item->GetCell()), QUICKSLOT_MAX_POS);
			item->RemoveFromCharacter();
		}
	}

	M2_DESTROY_ITEM(item);
}

void ITEM_MANAGER::DestroyItem(LPITEM item)
{
	if (!item)
		return;

	if (item->GetSectree())
		item->RemoveFromGround();

	if (item->GetOwner())
	{
		if (CHARACTER_MANAGER::Instance().Find(item->GetOwner()->GetPlayerID()) != nullptr)
		{
			sys_err("DestroyItem: GetOwner %s %s!!", item->GetName(), item->GetOwner()->GetName());
			item->RemoveFromCharacter();
		}
		else
		{
			sys_err("WTH! Invalid item owner. owner pointer : %p", item->GetOwner());
		}
	}

	const std::unordered_set<LPITEM>::iterator it = m_set_pkItemForDelayedSave.find(item);

	if (it != m_set_pkItemForDelayedSave.end())
		m_set_pkItemForDelayedSave.erase(it);

	const uint32_t dwID = item->GetID();
	sys_log(2, "ITEM_DESTROY %s:%u", item->GetName(), dwID);

	if (!item->GetSkipSave() && dwID)
	{
/*#ifdef ENABLE_GROWTH_PET_SYSTEM
		if (item->GetType() == ITEM_PET && item->GetSubType() == PET_UPBRINGING)
		{
			const uint32_t dwSocketID = item->GetSocket(2);
			db_clientdesc->DBPacketHeader(HEADER_GD_PET_ITEM_DESTROY, 0, sizeof(uint32_t));
			db_clientdesc->Packet(&dwSocketID, sizeof(uint32_t));
		}
#endif*/

		const uint32_t dwOwnerID = item->GetLastOwnerPID();

		db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_DESTROY, 0, sizeof(uint32_t) + sizeof(uint32_t));
		db_clientdesc->Packet(&dwID, sizeof(uint32_t));
		db_clientdesc->Packet(&dwOwnerID, sizeof(uint32_t));
	}
	else
	{
		sys_log(2, "ITEM_DESTROY_SKIP %s:%u (skip=%d)", item->GetName(), dwID, item->GetSkipSave());
	}

	if (dwID)
		m_map_pkItemByID.erase(dwID);

	m_VIDMap.erase(item->GetVID());
	M2_DELETE(item);
}

LPITEM ITEM_MANAGER::Find(uint32_t id)
{
	const auto it = m_map_pkItemByID.find(id);
	if (it == m_map_pkItemByID.end())
		return nullptr;

	return it->second;
}

LPITEM ITEM_MANAGER::FindByVID(uint32_t vid)
{
	const ITEM_VID_MAP::iterator it = m_VIDMap.find(vid);

	if (it == m_VIDMap.end())
		return nullptr;

	return (it->second);
}

TItemTable* ITEM_MANAGER::GetTable(uint32_t vnum)
{
	const int rnum = RealNumber(vnum);

	if (rnum < 0)
	{
		for (size_t i = 0; i < m_vec_item_vnum_range_info.size(); i++)
		{
			TItemTable* p = m_vec_item_vnum_range_info[i];
			if ((p->dwVnum < vnum) &&
				vnum < (p->dwVnum + p->dwVnumRange))
			{
				return p;
			}
		}

		return nullptr;
	}

	return &m_vec_prototype[rnum];
}

int ITEM_MANAGER::RealNumber(uint32_t vnum)
{
	if (!vnum)
		return 0;

	int bot, top, mid;

	bot = 0;
	top = m_vec_prototype.size();

	const TItemTable* pTable = &m_vec_prototype[0];

	while (1)
	{
		mid = (bot + top) >> 1;

		if ((pTable + mid)->dwVnum == vnum)
			return (mid);

		if (bot >= top)
			return (-1);

		if ((pTable + mid)->dwVnum > vnum)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

bool ITEM_MANAGER::GetVnum(const char* c_pszName, uint32_t& r_dwVnum)
{
	const int len = strlen(c_pszName);

	TItemTable* pTable = &m_vec_prototype[0];

	for (uint32_t i = 0; i < m_vec_prototype.size(); ++i, ++pTable)
	{
		if (!strncasecmp(c_pszName, pTable->szLocaleName, len))
		{
			r_dwVnum = pTable->dwVnum;
			return true;
		}
	}

	return false;
}

bool ITEM_MANAGER::GetVnumByOriginalName(const char* c_pszName, uint32_t& r_dwVnum)
{
	const int len = strlen(c_pszName);

	TItemTable* pTable = &m_vec_prototype[0];

	for (uint32_t i = 0; i < m_vec_prototype.size(); ++i, ++pTable)
	{
		if (!strncasecmp(c_pszName, pTable->szName, len))
		{
			r_dwVnum = pTable->dwVnum;
			return true;
		}
	}

	return false;
}

std::set<uint32_t> g_set_lotto;

void load_lotto()
{
	static int bLoaded = false;

	if (bLoaded)
		return;

	bLoaded = true;
	FILE* fp = fopen("lotto.txt", "r");

	if (!fp)
		return;

	char buf[256];

	while (fgets(buf, 256, fp))
	{
		char* psz = strchr(buf, '\n');

		if (nullptr != psz)
			*psz = '\0';

		uint32_t dw = 0;
		str_to_number(dw, buf);
		g_set_lotto.insert(dw);
	}

	fclose(fp);
}

uint32_t lotto()
{
	load_lotto();

	char szBuf[6 + 1];

	do
	{
		for (int i = 0; i < 6; ++i)
			szBuf[i] = 48 + number(1, 9);

		szBuf[6] = '\0';

		uint32_t dw = 0;
		str_to_number(dw, szBuf);

		if (g_set_lotto.end() == g_set_lotto.find(dw))
		{
			FILE* fp = fopen("lotto.txt", "a+");
			if (fp)
			{
				fprintf(fp, "%u\n", dw);
				fclose(fp);
			}
			return dw;
		}
	}

	while (1);
}


class CItemDropInfo
{
	public:
		CItemDropInfo(int iLevelStart, int iLevelEnd, int iPercent, uint32_t dwVnum) :
			m_iLevelStart(iLevelStart), m_iLevelEnd(iLevelEnd), m_iPercent(iPercent), m_dwVnum(dwVnum)
		{
		}

		int m_iLevelStart;
		int m_iLevelEnd;
		int m_iPercent; // 1 ~ 1000
		uint32_t m_dwVnum;

		friend bool operator < (const CItemDropInfo& l, const CItemDropInfo& r)
		{
			return l.m_iLevelEnd < r.m_iLevelEnd;
		}
};

extern std::vector<CItemDropInfo> g_vec_pkCommonDropItem[MOB_RANK_MAX_NUM];

// 20050503.ipkn.
// If it is less than iMinimum, set iDefault (however, iMinimum should be greater than 0)
// Exist to support ON/OFF method in 1, 0 type
int GetDropPerKillPct(int iMinimum, int iDefault, int iDeltaPercent, const char* c_pszFlag)
{
	int iVal = 0;

	if ((iVal = quest::CQuestManager::Instance().GetEventFlag(c_pszFlag)))
	{
		if (!test_server)
		{
			if (iVal < iMinimum)
				iVal = iDefault;

			if (iVal < 0)
				iVal = iDefault;
		}
	}

	if (iVal == 0)
		return 0;

	// Default setting (iDeltaPercent=100)
	// One per 40000 iVal is a constant to give a feeling
	return (40000 * iDeltaPercent / iVal);
}

bool ITEM_MANAGER::GetDropPct(LPCHARACTER pkChr, LPCHARACTER pkKiller, OUT int& iDeltaPercent, OUT int& iRandRange)
{
	if (nullptr == pkChr || nullptr == pkKiller)
		return false;

	const int iLevel = pkKiller->GetLevel();
	iDeltaPercent = 100;

	if (!pkChr->IsStone() && pkChr->GetMobRank() >= MOB_RANK_BOSS)
		iDeltaPercent = PERCENT_LVDELTA_BOSS(pkKiller->GetLevel(), pkChr->GetLevel());
	else
		iDeltaPercent = PERCENT_LVDELTA(pkKiller->GetLevel(), pkChr->GetLevel());

	const uint8_t bRank = pkChr->GetMobRank();

	sys_log(3, "CreateDropItem for level: %d rank: %u pct: %d", iLevel, bRank, iDeltaPercent);

	// Nyx : Enabled MallItemBonus
	if (pkKiller->GetPoint(POINT_MALL_ITEMBONUS) > 0)
		iDeltaPercent += (iDeltaPercent * pkKiller->GetPoint(POINT_MALL_ITEMBONUS)) / 100;

	// ADD_PREMIUM
	if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
		iDeltaPercent += (iDeltaPercent * 50) / 100;
	// END_OF_ADD_PREMIUM

	// Nyx : Sum of wearable thief glove
	if (pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		iDeltaPercent += (iDeltaPercent * 50) / 100;

	// Nyx : Added percent for Positive Karma -> https://it-wiki.metin2.gameforge.com/index.php/Punti_Karma
	uint8_t bGoodKarmaPct = 0;
	switch (pkKiller->GetAlignRank())
	{
		case PVP_LEVEL_CHIVALRIC:
			bGoodKarmaPct = 5;
			break;
		case PVP_LEVEL_NOBLE:
			bGoodKarmaPct = 3;
			break;
		case PVP_LEVEL_GOOD:
			bGoodKarmaPct = 2;
			break;
		case PVP_LEVEL_FRIENDLY:
			bGoodKarmaPct = 1;
			break;
		default:
			break;
	}
	if (bGoodKarmaPct >= 1)
		iDeltaPercent += (iDeltaPercent * bGoodKarmaPct) / 100;

	iRandRange = 4000000;
	iRandRange = iRandRange * 100 /
		(100 +
			CPrivManager::Instance().GetPriv(pkKiller, PRIV_ITEM_DROP) +
			(pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ? 50 : 0) + // Nyx : Added premium item
			(pkKiller->GetPoint(POINT_MALL_ITEMBONUS) ? 50 : 0) + // Nyx : Added MallItem bonus
			(pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM) ? 50 : 0) + // Nyx: Changed for special_item_group
			(bGoodKarmaPct >= 1 ? bGoodKarmaPct : 0) // Nyx: Added percent for Positive Karma
			);

	return true;
}

#ifdef ENABLE_NEWSTUFF
bool ITEM_MANAGER::GetDropPctCustom(LPCHARACTER pkChr, LPCHARACTER pkKiller, OUT int& iDeltaPercent, OUT int& iRandRange)
{
	if (nullptr == pkChr || nullptr == pkKiller)
		return false;

	const int iLevel = pkKiller->GetLevel();
	iDeltaPercent = 100;

	if (!pkChr->IsStone() && pkChr->GetMobRank() >= MOB_RANK_BOSS)
		iDeltaPercent = PERCENT_LVDELTA_BOSS(pkKiller->GetLevel(), pkChr->GetLevel());
	else
		iDeltaPercent = PERCENT_LVDELTA(pkKiller->GetLevel(), pkChr->GetLevel());

	const uint8_t bRank = pkChr->GetMobRank();

	sys_log(3, "CreateDropItemCustom for level: %d rank: %u pct: %d", iLevel, bRank, iDeltaPercent);

	// Nyx : Enabled MallItemBonus
	if (pkKiller->GetPoint(POINT_MALL_ITEMBONUS) > 0)
		iDeltaPercent += (iDeltaPercent * 25) / 100;

	// ADD_PREMIUM
	if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0)
		iDeltaPercent += (iDeltaPercent * 25) / 100;
	// END_OF_ADD_PREMIUM

	// Nyx : Sum of wearable thief glove
	if (pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		iDeltaPercent += (iDeltaPercent * 25) / 100;

	// Nyx : Added percent for Positive Karma -> https://it-wiki.metin2.gameforge.com/index.php/Punti_Karma
	uint8_t bGoodKarmaPct = 0;
	switch (pkKiller->GetAlignRank())
	{
		case PVP_LEVEL_CHIVALRIC:
			bGoodKarmaPct = 5;
			break;
		case PVP_LEVEL_NOBLE:
			bGoodKarmaPct = 3;
			break;
		case PVP_LEVEL_GOOD:
			bGoodKarmaPct = 2;
			break;
		case PVP_LEVEL_FRIENDLY:
			bGoodKarmaPct = 1;
			break;
		default:
			break;
	}
	if (bGoodKarmaPct >= 1)
		iDeltaPercent += (iDeltaPercent * bGoodKarmaPct) / 100;

	iRandRange = 4000000;
	iRandRange = iRandRange * 100 /
		(100 +
			CPrivManager::Instance().GetPriv(pkKiller, PRIV_ITEM_DROP) +
			(pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ? 25 : 0) + // Nyx : Added premium item
			(pkKiller->GetPoint(POINT_MALL_ITEMBONUS) ? 25 : 0) + // Nyx : Added MallItem bonus
			(pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM) ? 25 : 0) + // Nyx: Changed for special_item_group
			(bGoodKarmaPct >= 1 ? bGoodKarmaPct : 0) // Nyx: Added percent for Positive Karma
			);

	return true;
}
#endif

bool ITEM_MANAGER::CreateDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item)
{
	const int iLevel = pkKiller->GetLevel();

#ifdef DISABLE_STONE_LV20_DROP
	if (pkChr->IsStone() && iLevel > pkChr->GetLevel() + 20 && !quest::CQuestManager::Instance().GetEventFlag("stone_drop_lv_disabled"))
		return false;
#endif
#ifdef DISABLE_BOSS_LV25_DROP
	if (pkChr->IsMonster() && !pkChr->IsStone() && pkChr->GetMobRank() >= MOB_RANK_BOSS && (iLevel - 25 > pkChr->GetLevel() || iLevel + 25 < pkChr->GetLevel()))
		return false;
#endif

	int iDeltaPercent, iRandRange;
	if (!GetDropPct(pkChr, pkKiller, iDeltaPercent, iRandRange))
		return false;

	uint8_t bRank = pkChr->GetMobRank();
#ifdef MOB_DROP_ITEM_RENEWAL
	int mMapIndex = pkChr->GetMapIndex();
#endif
	LPITEM item = nullptr;

	// Common Drop Items
	std::vector<CItemDropInfo>::iterator it = g_vec_pkCommonDropItem[bRank].begin();

	while (it != g_vec_pkCommonDropItem[bRank].end())
	{
		const CItemDropInfo& c_rInfo = *(it++);

		if (iLevel < c_rInfo.m_iLevelStart || iLevel > c_rInfo.m_iLevelEnd)
			continue;

		const int iPercent = (c_rInfo.m_iPercent * iDeltaPercent) / 100;
		sys_log(3, "CreateDropItem %d ~ %d %d(%d)", c_rInfo.m_iLevelStart, c_rInfo.m_iLevelEnd, c_rInfo.m_dwVnum, iPercent, c_rInfo.m_iPercent);

		if (iPercent >= number(1, iRandRange))
		{
			const TItemTable* table = GetTable(c_rInfo.m_dwVnum);

			if (!table)
				continue;

			item = nullptr;

			if (table->bType == ITEM_POLYMORPH)
			{
				if (c_rInfo.m_dwVnum == pkChr->GetPolymorphItemVnum())
				{
					item = CreateItem(c_rInfo.m_dwVnum, 1, 0, true);
					if (item)
						item->SetSocket(0, pkChr->GetRaceNum());
				}
			}
			else
				item = CreateItem(c_rInfo.m_dwVnum, 1, 0, true);

#ifdef ENABLE_COMMON_DROP_INFO
			if (item)
				vec_item.emplace_back(item);
#else
			if (item)
				return false;
#endif
		}
	}

	// Drop Item Group
	{
		const auto drop_it = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());

		if (drop_it != m_map_pkDropItemGroup.end())
		{
			typeof(drop_it->second->GetVector()) v = drop_it->second->GetVector();
			for (uint32_t i = 0; i < v.size(); ++i)
			{
				const int iPercent = (v[i].dwPct * iDeltaPercent) / 100;
				if (iPercent >= number(1, iRandRange))
				{
					item = CreateItem(v[i].dwVnum, v[i].iCount, 0, true);
					if (item)
					{
						if (item->GetType() == ITEM_POLYMORPH)
						{
							if (item->GetVnum() == pkChr->GetPolymorphItemVnum())
								item->SetSocket(0, pkChr->GetRaceNum());
						}

						vec_item.emplace_back(item);
					}
				}
			}
		}
	}

	// MobDropItem Group
	{
		const auto mob_kill_it = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());

		if (mob_kill_it != m_map_pkMobItemGroup.end())
		{
			const CMobItemGroup* pGroup = mob_kill_it->second;

			// MOB_DROP_ITEM_BUG_FIX
			// Fixed a problem when accessing CMobItemGroup::GetOne() when there is no item in 20050805.myevan.MobDropItem
			if (pGroup && !pGroup->IsEmpty())
			{
				const int iPercent = 40000 * iDeltaPercent / pGroup->GetKillPerDrop();
				if (iPercent >= number(1, iRandRange))
				{
					const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();
					item = CreateItem(info.dwItemVnum, info.iCount, 0, true, info.iRarePct);
					if (item)
						vec_item.emplace_back(item);
				}
			}
			// END_OF_MOB_DROP_ITEM_BUG_FIX
		}
	}

	// Level Item Group
	{
		const auto mob_level_it = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());

		if (mob_level_it != m_map_pkLevelItemGroup.end())
		{
			if (mob_level_it->second->GetLevelLimit() <= (uint32_t)iLevel)
			{
				typeof(mob_level_it->second->GetVector()) v = mob_level_it->second->GetVector();

				for (uint32_t i = 0; i < v.size(); i++)
				{
					if (v[i].dwPct >= (uint32_t)number(1, 1000000/*iRandRange*/))
					{
						const uint32_t dwVnum = v[i].dwVNum;
						item = CreateItem(dwVnum, v[i].iCount, 0, true);
						if (item)
							vec_item.emplace_back(item);
					}
				}
			}
		}
	}

#ifdef MOB_DROP_ITEM_RENEWAL
	{
		const auto map_index_it = m_map_pkMapIndexItemGroup.find(pkChr->GetRaceNum());

		if (map_index_it != m_map_pkMapIndexItemGroup.end())
		{
			if (map_index_it->second->GetMapIndex() == mMapIndex)
			{
				typeof(map_index_it->second->GetVector()) v = map_index_it->second->GetVector();

				for (uint32_t i = 0; i < v.size(); i++)
				{
					if (v[i].dwPct >= (uint32_t)number(1, 1000000/*iRandRange*/))
					{
						const uint32_t dwVnum = v[i].dwVNum;
						item = CreateItem(dwVnum, v[i].iCount, 0, true);
						if (item)
							vec_item.emplace_back(item);
					}
				}
			}
		}
	}
#endif

	// BuyerTheitGloves Item Group
	{
		// by mhh Temporarily apply the same as general drop in Japan
		if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||
			pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		{
			const auto mob_premium_it = m_map_pkGloveItemGroup.find(pkChr->GetRaceNum());

			if (mob_premium_it != m_map_pkGloveItemGroup.end())
			{
				typeof(mob_premium_it->second->GetVector()) v = mob_premium_it->second->GetVector();

				for (uint32_t i = 0; i < v.size(); ++i)
				{
					const int iPercent = (v[i].dwPct * iDeltaPercent) / 100;
					if (iPercent >= number(1, iRandRange))
					{
						const uint32_t dwVnum = v[i].dwVnum;
						item = CreateItem(dwVnum, v[i].iCount, 0, true);
						if (item)
							vec_item.emplace_back(item);
					}
				}
			}
		}
	}

	// dwDropItemVnum from mob_proto
	if (pkChr->GetMobDropItemVnum())
	{
		const auto mob_etc_it = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());

		if (mob_etc_it != m_map_dwEtcItemDropProb.end())
		{
			const int iPercent = (mob_etc_it->second * iDeltaPercent) / 100;
			if (iPercent >= number(1, iRandRange))
			{
				item = CreateItem(pkChr->GetMobDropItemVnum(), 1, 0, true);
				if (item)
					vec_item.emplace_back(item);
			}
		}
	}

	if (pkChr->IsStone())
	{
		if (pkChr->GetDropMetinStoneVnum())
		{
			const int iPercent = (pkChr->GetDropMetinStonePct() * iDeltaPercent) * 400;
			if (iPercent >= number(1, iRandRange))
			{
				item = CreateItem(pkChr->GetDropMetinStoneVnum(), 1, 0, true);
				if (item)
					vec_item.emplace_back(item);
			}
		}
	}

	if (pkKiller->IsHorseRiding() &&
		GetDropPerKillPct(1000, 1000000, iDeltaPercent, "horse_skill_book_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT HORSE_SKILL_BOOK_DROP");

		if ((item = CreateItem(ITEM_HORSE_SKILL_TRAIN_BOOK, 1, 0, true)))
			vec_item.emplace_back(item);
	}

#ifdef __UNIMPLEMENTED__
	if (8051 <= pkChr->GetRaceNum() && pkChr->GetRaceNum() <= 8056)
	{
		int ar_ListStones[10][2] =
		{
			{ 28044,	15}, // + 0
			{ 28144,	12}, // + 1
			{ 28244,	11}, // + 2
			{ 28344,	7}, // + 3
			{ 28444,	2}, // + 4

			{ 28045,	15}, // + 0
			{ 28145,	12}, // + 1
			{ 28245,	11}, // + 2
			{ 28345,	7}, // + 3
			{ 28445,	2} // + 4
		};
		
		int c_random = number(0, 9);
		int iPercent = (ar_ListStones[c_random][1] * iDeltaPercent) * 400;	
			
		if (iPercent >= number(1, iRandRange))
		{	
			item = CreateItem(ar_ListStones[c_random][0], 1, 0, true);
			if (item)
				vec_item.emplace_back(item);
		}
	}
#endif

	if (GetDropPerKillPct(100, 1000, iDeltaPercent, "lotto_drop") >= number(1, iRandRange))
	{
		uint32_t* pdw = M2_NEW uint32_t[3];

		pdw[0] = 50001;
		pdw[1] = 1;
		pdw[2] = quest::CQuestManager::Instance().GetEventFlag("lotto_round");

		// Lucky book set up socket
		DBManager::Instance().ReturnQuery(QID_LOTTO, pkKiller->GetPlayerID(), pdw,
			"INSERT INTO lotto_list VALUES(0, 'server%s', %u, NOW())",
			get_table_postfix(), pkKiller->GetPlayerID());
	}

	//
	// Special drop item
	//
#ifdef ENABLE_NEWSTUFF
	// Using for events that drop "x" item
	int iDeltaPercentCustom, iRandRangeCustom;
	if (GetDropPctCustom(pkChr, pkKiller, iDeltaPercentCustom, iRandRangeCustom))
	{
		CreateQuestDropItem(pkChr, pkKiller, vec_item, iDeltaPercentCustom, iRandRangeCustom);
	}
	else
#endif
	{
		CreateQuestDropItem(pkChr, pkKiller, vec_item, iDeltaPercent, iRandRange);
	}

	for (auto money_it = vec_item.begin(); money_it != vec_item.end(); ++money_it)
	{
		LPITEM item2 = *money_it;
		DBManager::Instance().SendMoneyLog(MONEY_LOG_DROP, item2->GetVnum(), item2->GetCount());
	}

	return vec_item.size();
}

#ifdef ENABLE_SEND_TARGET_INFO
bool ITEM_MANAGER::CreateDropItemVector(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item)
{
	if (!pkChr || !pkKiller)
		return false;

	if (pkChr->IsPolymorphed() || pkChr->IsPC())
		return false;

	const int iLevel = pkKiller->GetLevel();
	uint8_t bRank = pkChr->GetMobRank();
	LPITEM item = nullptr;

	std::vector<CItemDropInfo>::iterator it = g_vec_pkCommonDropItem[bRank].begin();

	while (it != g_vec_pkCommonDropItem[bRank].end())
	{
		const CItemDropInfo& c_rInfo = *(it++);

		if (iLevel < c_rInfo.m_iLevelStart || iLevel > c_rInfo.m_iLevelEnd)
			continue;

		if (c_rInfo.m_iPercent <= 0)
			continue;

		const TItemTable* table = GetTable(c_rInfo.m_dwVnum);
		if (!table)
			continue;

		item = nullptr;

		if (table->bType == ITEM_POLYMORPH)
		{
			if (c_rInfo.m_dwVnum == pkChr->GetPolymorphItemVnum())
			{
				item = CreateItem(c_rInfo.m_dwVnum, 1, 0, true);

				if (item)
					item->SetSocket(0, pkChr->GetRaceNum());
			}
		}
		else
			item = CreateItem(c_rInfo.m_dwVnum, 1, 0, true);

		if (item)
			vec_item.emplace_back(item);
	}

	// Drop Item Group
	{
		const auto drop_it = m_map_pkDropItemGroup.find(pkChr->GetRaceNum());
		if (drop_it != m_map_pkDropItemGroup.end())
		{
			typeof(drop_it->second->GetVector()) v = drop_it->second->GetVector();
			for (uint32_t i = 0; i < v.size(); ++i)
			{
				item = CreateItem(v[i].dwVnum, v[i].iCount, 0, true);

				if (item)
				{
					if (item->GetType() == ITEM_POLYMORPH)
					{
						if (item->GetVnum() == pkChr->GetPolymorphItemVnum())
						{
							item->SetSocket(0, pkChr->GetRaceNum());
						}
					}

					vec_item.emplace_back(item);
				}
			}
		}
	}

	// MobDropItem Group - KillDrop
	{
		const auto mob_drop_it = m_map_pkMobItemGroup.find(pkChr->GetRaceNum());

		if (mob_drop_it != m_map_pkMobItemGroup.end())
		{
			const CMobItemGroup* pGroup = mob_drop_it->second;
			if (pGroup && !pGroup->IsEmpty())
			{
				const CMobItemGroup::SMobItemGroupInfo& info = pGroup->GetOne();
				item = CreateItem(info.dwItemVnum, info.iCount, 0, true, info.iRarePct);

				if (item)
					vec_item.emplace_back(item);
			}
			// END_OF_MOB_DROP_ITEM_BUG_FIX
		}
	}

	// Level Item Group
	{
		const auto level_drop_it = m_map_pkLevelItemGroup.find(pkChr->GetRaceNum());
		if (level_drop_it != m_map_pkLevelItemGroup.end())
		{
			if (level_drop_it->second->GetLevelLimit() <= (uint32_t)iLevel)
			{
				typeof(level_drop_it->second->GetVector()) v = level_drop_it->second->GetVector();
				for (uint32_t i = 0; i < v.size(); i++)
				{
					uint32_t dwVnum = v[i].dwVNum;
					item = CreateItem(dwVnum, v[i].iCount, 0, true);
					if (item)
						vec_item.emplace_back(item);
				}
			}
		}
	}

	// BuyerTheitGloves Item Group
	{
		// by mhh Temporarily apply the same as general drop in Japan
		if (pkKiller->GetPremiumRemainSeconds(PREMIUM_ITEM) > 0 ||
			pkKiller->IsEquipUniqueGroup(UNIQUE_GROUP_DOUBLE_ITEM))
		{
			const auto premium_drop_it = m_map_pkGloveItemGroup.find(pkChr->GetRaceNum());
			if (premium_drop_it != m_map_pkGloveItemGroup.end())
			{
				typeof(premium_drop_it->second->GetVector()) v = premium_drop_it->second->GetVector();

				for (uint32_t i = 0; i < v.size(); ++i)
				{
					uint32_t dwVnum = v[i].dwVnum;
					item = CreateItem(dwVnum, v[i].iCount, 0, true);
					if (item)
						vec_item.emplace_back(item);
				}
			}
		}
	}

	// ETC DropItem
	if (pkChr->GetMobDropItemVnum())
	{
		const auto etc_drop_it = m_map_dwEtcItemDropProb.find(pkChr->GetMobDropItemVnum());
		if (etc_drop_it != m_map_dwEtcItemDropProb.end())
		{
			item = CreateItem(pkChr->GetMobDropItemVnum(), 1, 0, true);
			if (item)
				vec_item.emplace_back(item);
		}
	}

	if (pkChr->IsStone())
	{
		if (pkChr->GetDropMetinStoneVnum())
		{
			item = CreateItem(pkChr->GetDropMetinStoneVnum(), 1, 0, true);
			if (item) vec_item.emplace_back(item);
		}
	}

	return vec_item.size();
}
#endif

// ADD_GRANDMASTER_SKILL
int GetThreeSkillLevelAdjust(int level)
{
	if (level < 40)
		return 32;
	if (level < 45)
		return 16;
	if (level < 50)
		return 8;
	if (level < 55)
		return 4;
	if (level < 60)
		return 2;
	return 1;
}
// END_OF_ADD_GRANDMASTER_SKILL

// DROPEVENT_CHARSTONE
// drop_char_stone 1
// drop_char_stone.percent_lv01_10 5
// drop_char_stone.percent_lv11_30 10
// drop_char_stone.percent_lv31_MX 15
// drop_char_stone.level_range 10
static struct DropEvent_CharStone
{
	int percent_lv01_10;
	int percent_lv11_30;
	int percent_lv31_MX;
	int level_range;
	bool alive;

	DropEvent_CharStone()
	{
		percent_lv01_10 = 100;
		percent_lv11_30 = 200;
		percent_lv31_MX = 300;
		level_range = 10;
		alive = false;
	}
} gs_dropEvent_charStone;

static int __DropEvent_CharStone_GetDropPercent(int killer_level)
{
	const int killer_levelStep = (killer_level - 1) / 10;

	switch (killer_levelStep)
	{
		case 0:
			return gs_dropEvent_charStone.percent_lv01_10;

		case 1:
		case 2:
			return gs_dropEvent_charStone.percent_lv11_30;

		default:
			break;
	}

	return gs_dropEvent_charStone.percent_lv31_MX;
}

static void __DropEvent_CharStone_DropItem(CHARACTER& killer, CHARACTER& victim, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item)
{
	if (!gs_dropEvent_charStone.alive)
		return;

	const int killer_level = killer.GetLevel();
	const int dropPercent = __DropEvent_CharStone_GetDropPercent(killer_level);

	const int MaxRange = 10000;

	if (number(1, MaxRange) <= dropPercent)
	{
		const int log_level = (test_server || killer.GetGMLevel() >= GM_LOW_WIZARD) ? 0 : 1;
		const int victim_level = victim.GetLevel();
		const int level_diff = victim_level - killer_level;

		if (level_diff >= +gs_dropEvent_charStone.level_range || level_diff <= -gs_dropEvent_charStone.level_range)
		{
			sys_log(log_level,
				"dropevent.drop_char_stone.level_range_over: killer(%s: lv%d), victim(%s: lv:%d), level_diff(%d)",
				killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), level_diff);
			return;
		}

		static const int Stones[] = { 30210, 30211, 30212, 30213, 30214, 30215, 30216, 30217, 30218, 30219, 30258, 30259, 30260, 30261, 30262, 30263 };
		int item_vnum = Stones[number(0, _countof(Stones) - 1)]; // @fixme189

		LPITEM p_item = nullptr;

		if ((p_item = itemMgr.CreateItem(item_vnum, 1, 0, true)))
		{
			vec_item.emplace_back(p_item);

			sys_log(log_level,
				"dropevent.drop_char_stone.item_drop: killer(%s: lv%d), victim(%s: lv:%d), item_name(%s)",
				killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), p_item->GetName());
		}
	}
}

bool DropEvent_CharStone_SetValue(const std::string& name, int value)
{
	if (name == "drop_char_stone")
	{
		gs_dropEvent_charStone.alive = value;

		if (value)
			sys_log(0, "dropevent.drop_char_stone = on");
		else
			sys_log(0, "dropevent.drop_char_stone = off");

	}
	else if (name == "drop_char_stone.percent_lv01_10")
		gs_dropEvent_charStone.percent_lv01_10 = value;
	else if (name == "drop_char_stone.percent_lv11_30")
		gs_dropEvent_charStone.percent_lv11_30 = value;
	else if (name == "drop_char_stone.percent_lv31_MX")
		gs_dropEvent_charStone.percent_lv31_MX = value;
	else if (name == "drop_char_stone.level_range")
		gs_dropEvent_charStone.level_range = value;
	else
		return false;

	sys_log(0, "dropevent.drop_char_stone: %d", gs_dropEvent_charStone.alive ? true : false);
	sys_log(0, "dropevent.drop_char_stone.percent_lv01_10: %f", gs_dropEvent_charStone.percent_lv01_10 / 100.0f);
	sys_log(0, "dropevent.drop_char_stone.percent_lv11_30: %f", gs_dropEvent_charStone.percent_lv11_30 / 100.0f);
	sys_log(0, "dropevent.drop_char_stone.percent_lv31_MX: %f", gs_dropEvent_charStone.percent_lv31_MX / 100.0f);
	sys_log(0, "dropevent.drop_char_stone.level_range: %d", gs_dropEvent_charStone.level_range);

	return true;
}

// END_OF_DROPEVENT_CHARSTONE

// fixme
/*Let's subtract with the above as a quest.
This is too dirty...
I hate hardcoding.
Start rewarding the weighing item.
by rtsummit let's fix it*/
static struct DropEvent_RefineBox
{
	int percent_low;
	int low;
	int percent_mid;
	int mid;
	int percent_high;
	//int level_range;
	bool alive;

	DropEvent_RefineBox()
	{
		percent_low = 100;
		low = 20;
		percent_mid = 100;
		mid = 45;
		percent_high = 100;
		//level_range = 10;
		alive = false;
	}
} gs_dropEvent_refineBox;

static LPITEM __DropEvent_RefineBox_GetDropItem(CHARACTER& killer, CHARACTER& victim, ITEM_MANAGER& itemMgr)
{
	static const int lowerBox[] = { 50197, 50198, 50199 };
	static const int lowerBox_range = 3;
	static const int midderBox[] = { 50203, 50204, 50205, 50206 };
	static const int midderBox_range = 4;
	static const int higherBox[] = { 50207, 50208, 50209, 50210, 50211 };
	static const int higherBox_range = 5;

	if (victim.GetMobRank() < MOB_RANK_KNIGHT)
		return nullptr;

	const int killer_level = killer.GetLevel();
	/*int level_diff = victim_level - killer_level;

	if (level_diff >= +gs_dropEvent_refineBox.level_range || level_diff <= -gs_dropEvent_refineBox.level_range)
	{
		sys_log(log_level,
			"dropevent.drop_refine_box.level_range_over: killer(%s: lv%d), victim(%s: lv:%d), level_diff(%d)",
			killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), level_diff);
		return nullptr;
	}*/

	if (killer_level <= gs_dropEvent_refineBox.low)
	{
		if (number(1, gs_dropEvent_refineBox.percent_low) == 1)
		{
			return itemMgr.CreateItem(lowerBox[number(1, lowerBox_range) - 1], 1, 0, true);
		}
	}
	else if (killer_level <= gs_dropEvent_refineBox.mid)
	{
		if (number(1, gs_dropEvent_refineBox.percent_mid) == 1)
		{
			return itemMgr.CreateItem(midderBox[number(1, midderBox_range) - 1], 1, 0, true);
		}
	}
	else
	{
		if (number(1, gs_dropEvent_refineBox.percent_high) == 1)
		{
			return itemMgr.CreateItem(higherBox[number(1, higherBox_range) - 1], 1, 0, true);
		}
	}
	return nullptr;
}

static void __DropEvent_RefineBox_DropItem(CHARACTER& killer, CHARACTER& victim, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item)
{
	if (!gs_dropEvent_refineBox.alive)
		return;

	const int log_level = (test_server || killer.GetGMLevel() >= GM_LOW_WIZARD) ? 0 : 1;

	LPITEM p_item = __DropEvent_RefineBox_GetDropItem(killer, victim, itemMgr);

	if (p_item)
	{
		vec_item.emplace_back(p_item);

		sys_log(log_level,
			"dropevent.drop_refine_box.item_drop: killer(%s: lv%d), victim(%s: lv:%d), item_name(%s)",
			killer.GetName(), killer.GetLevel(), victim.GetName(), victim.GetLevel(), p_item->GetName());
	}
}

bool DropEvent_RefineBox_SetValue(const std::string& name, int value)
{
	if (name == "refine_box_drop")
	{
		gs_dropEvent_refineBox.alive = value;

		if (value)
			sys_log(0, "refine_box_drop = on");
		else
			sys_log(0, "refine_box_drop = off");

	}
	else if (name == "refine_box_low")
		gs_dropEvent_refineBox.percent_low = value < 100 ? 100 : value;
	else if (name == "refine_box_mid")
		gs_dropEvent_refineBox.percent_mid = value < 100 ? 100 : value;
	else if (name == "refine_box_high")
		gs_dropEvent_refineBox.percent_high = value < 100 ? 100 : value;
	//else if (name == "refine_box_level_range")
	// gs_dropEvent_refineBox.level_range = value;
	else
		return false;

	sys_log(0, "refine_box_drop: %d", gs_dropEvent_refineBox.alive ? true : false);
	sys_log(0, "refine_box_low: %d", gs_dropEvent_refineBox.percent_low);
	sys_log(0, "refine_box_mid: %d", gs_dropEvent_refineBox.percent_mid);
	sys_log(0, "refine_box_high: %d", gs_dropEvent_refineBox.percent_high);
	//sys_log(0, "refine_box_low_level_range: %d", gs_dropEvent_refineBox.level_range);

	return true;
}
// End of improvement item reward.

#ifdef ENABLE_EVENT_MANAGER
void ITEM_MANAGER::DropEventItem(const CHARACTER& pkKiller, const CHARACTER& pkChr, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item,
	uint32_t dwVnum, uint8_t iLevelDiff, uint8_t bDropType, int iChanceMax, int iDeltaPercent, int iRandRange)
{
	if ((dwVnum <= 0) || (iLevelDiff <= 0) || (iChanceMax <= 0))
		return;

	iDeltaPercent = 40000 * iDeltaPercent / iChanceMax;
	if ((iDeltaPercent < 0) || (iRandRange < 0))
		return;

	if (iDeltaPercent < number(1, iRandRange))
		return;

	bool canDropLevel = false;
	bool canReceiveDrop = false;

	if (abs(pkKiller.GetLevel() - pkChr.GetLevel()) <= iLevelDiff)
		canDropLevel = true;

	if (!canDropLevel)
		return;

	switch (bDropType)
	{
		case CEventManager::DROP_TYPE_GENERAL:
		{
			canReceiveDrop = true;
		}
		break;
		case CEventManager::DROP_TYPE_BOSS_AND_MORE:
		{
			if (pkChr.GetMobRank() >= MOB_RANK_BOSS)
				canReceiveDrop = true;
		}
		break;
		case CEventManager::DROP_TYPE_BOSS:
		{
			if (pkChr.GetMobRank() >= MOB_RANK_BOSS && !pkChr.IsStone())
				canReceiveDrop = true;
		}
		break;
		case CEventManager::DROP_TYPE_STONE:
		{
			if (pkChr.IsStone())
				canReceiveDrop = true;
		}
		break;
		default:
			break;
	}

	if (!canReceiveDrop)
		return;

	LPITEM item = nullptr;
	if ((item = itemMgr.CreateItem(dwVnum, 1, 0)))
		vec_item.emplace_back(item);
}
#endif

void ITEM_MANAGER::CreateQuestDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item, int iDeltaPercent, int iRandRange)
{
	if (!pkChr || !pkKiller)
		return;

	sys_log(1, "CreateQuestDropItem victim(%s), killer(%s)", pkChr->GetName(), pkKiller->GetName());

#ifdef ENABLE_EVENT_MANAGER
	uint32_t dwVnum = 0;
	uint8_t iLevelDiff = 0;
	uint8_t bDropType = CEventManager::DROP_TYPE_NONE;
	int iChanceMax = 0;

	// Official Events
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MYSTERY_BOX_DROP))
	{
		dwVnum = quest::CQuestManager::Instance().GetEventFlag("e_random_drop_vnum");
		iLevelDiff = 20;

		bDropType = quest::CQuestManager::Instance().GetEventFlag("e_random_drop_type");
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("e_random_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MINI_GAME_OKEY))
	{
		dwVnum = 79505;	// Okey-Sammelkarte
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("okey_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_NEW_XMAS_EVENT))
	{
		dwVnum = 50010;	// Socke
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;

		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("xmas_perc");
		int iDropPerKill[MOB_RANK_MAX_NUM] = { iChanceMax+5, iChanceMax, iChanceMax-5, iChanceMax-10, 0, 0 };
		if (iChanceMax < 0)
			iChanceMax = 0;
		if (iChanceMax > 100)
			iChanceMax = 100;
		iChanceMax = iDropPerKill[pkChr->GetMobRank()];

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}

#ifdef ENABLE_MINI_GAME_YUTNORI
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MINI_GAME_YUTNORI))
	{
		dwVnum = YUT_NORI_DROP_ITEM;	// Birkenstamm
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("yutnori_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_EASTER_EVENT
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_EASTER_DROP))
	{
		// Ostereier
		dwVnum = number(EASTER_EGG_VNUM_START, EASTER_EGG_VNUM_END);
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("easter_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);

		// Zauber-Ei
		uint32_t dwVnum2 = 0;
		uint8_t iLevelDiff2 = 0, bDropType2 = CEventManager::DROP_TYPE_NONE;
		int iChanceMax2 = 0;

		dwVnum2 = 71150;
		iLevelDiff2 = 20;

		bDropType2 = CEventManager::DROP_TYPE_STONE;
		iChanceMax2 = quest::CQuestManager::Instance().GetEventFlag("easter_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum2, iLevelDiff2, bDropType2, iChanceMax2, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_SUMMER_EVENT
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_E_SUMMER_EVENT))
	{
		dwVnum = 50279;	// Eiskugeln
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("e_summer_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_2017_RAMADAN
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_RAMADAN_DROP))
	{
		dwVnum = 30315;	// Brot
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("ramadan_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_NEW_HALLOWEEN_EVENT
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_HALLOWEEN_BOX))
	{
		dwVnum = number(30321, 30322);	// Halloween-Kürbis | Zauberstab
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("halloween_box_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_FOOTBALL_DROP))
	{
		dwVnum = 50096;	// Fußball
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("football_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}

	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MEDAL_PART_DROP))
	{
		dwVnum = number(30265, 30269);	// Medaillenstück
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("medal_part_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}

#ifdef ENABLE_2016_VALENTINE
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_VALENTINE_DROP))
	{
		dwVnum = number(50024, 50025);	// Rose | Schokolade
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("valentine_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MINI_GAME_CATCHKING))
	{
		dwVnum = CATCH_KING_DROP_ITEM;	// Königskarte
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("catch_king_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_MINI_GAME_BNW
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MINI_GAME_BNW))
	{
		dwVnum = BNW_DROP_ITEM;	// Birkenstamm
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("bnw_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MINI_GAME_FINDM))
	{
		dwVnum = FIND_M_DROP_ITEM;
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("findm_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_WORD_GAME_EVENT
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_WORLD_BOSS))
	{
		const static uint32_t word_event[] = { 30216, 30213, 30219, 30214, 30217, 30210 };
		dwVnum = word_event[number(0, 5)];
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_BOSS;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("world_boss_event");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

#ifdef ENABLE_FLOWER_EVENT
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_E_FLOWER_DROP))
	{
		iLevelDiff = 20;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("e_flower_drop_perc");

		if (iChanceMax <= 0)
			return;

		iDeltaPercent = 40000 * iDeltaPercent / iChanceMax;
		if ((iDeltaPercent < 0) || (iRandRange < 0))
			return;

		if (iDeltaPercent < number(1, iRandRange))
			return;

		bool canDropLevel = false;
		if ((pkKiller->GetLevel() < 30) || (abs(pkKiller->GetLevel() - pkChr->GetLevel()) <= iLevelDiff))
			canDropLevel = true;

		if (!canDropLevel)
			return;

		pkKiller->PointChange(POINT_FLOWER_TYPE_1, FLOWER_COUNT_ITEM_DROP);
	}
#endif

#ifdef ENABLE_ELEMENTAL_WORLD
	if (pkKiller->GetMapIndex() == MAP_ELEMENTAL_04)
	{
		const static uint32_t drop_items[] =
		{
			39088,	//Heil.Helmscherbe II
			39094,	//Heil.Rüstscherbe II
			39100,	//Heil.Waffenscherbe II
			39089,	//Prächt.Helmscherbe II
			39095,	//Prächt.Rüstscherbe II
			39101,	//Prächt.Waffenscherbe II
		};

		dwVnum = drop_items[number(0, 5)];
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = DROP_PERC_ELEMENT_04;

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#endif

	// Custom drop events
	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_MOONLIGHT))
	{
		dwVnum = 50011;	// Mondlicht-Schatztruhe
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("moon_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}

	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_HEXEGONAL_CHEST))
	{
		dwVnum = 50037;	// Sechseckige Schatztruhe
		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_GENERAL;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("hexegonal_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}

	if (CEventManager::Instance().GetEventState(CEventManager::EVENT_TYPE_HUNT_YOUR_MOUNT))
	{
		time_t iTime;
		time(&iTime);
		const tm* pTimeInfo = localtime(&iTime);
		const int iDay = pTimeInfo->tm_wday; // 0 - 6

		switch (iDay)
		{
			case EWeekDays::SUNDAY:
				dwVnum = 52702;
				break;
			case EWeekDays::MONDAY:
				dwVnum = number(52705, 52706);
				break;
			case EWeekDays::TUESDAY:
				dwVnum = 52701;
				break;
			case EWeekDays::WEDNESDAY:
				dwVnum = 52704;
				break;
			case EWeekDays::THURSDAY:
				dwVnum = 52708;
				break;
			case EWeekDays::FRIDAY:
				dwVnum = 52707;
				break;
			case EWeekDays::SATURDAY:
				dwVnum = 52703;
				break;
			default:
				break;
		}

		iLevelDiff = 20;

		bDropType = CEventManager::DROP_TYPE_BOSS_AND_MORE;
		iChanceMax = quest::CQuestManager::Instance().GetEventFlag("hunt_your_mount_drop_perc");

		DropEventItem(*pkKiller, *pkChr, *this, vec_item, dwVnum, iLevelDiff, bDropType, iChanceMax, iDeltaPercent, iRandRange);
	}
#else
	LPITEM item = nullptr;

	// DROPEVENT_CHARSTONE
	__DropEvent_CharStone_DropItem(*pkKiller, *pkChr, *this, vec_item);
	// END_OF_DROPEVENT_CHARSTONE
	__DropEvent_RefineBox_DropItem(*pkKiller, *pkChr, *this, vec_item);

	// Christmas socks
	if (quest::CQuestManager::Instance().GetEventFlag("xmas_sock"))
	{
		const uint32_t SOCK_ITEM_VNUM = 50010;

		int iDropPerKill[MOB_RANK_MAX_NUM] =
		{
			2000,
			1000,
			300,
			50,
			0,
			0,
		};

		if (iDropPerKill[pkChr->GetMobRank()] != 0)
		{
			int iPercent = 40000 * iDeltaPercent / iDropPerKill[pkChr->GetMobRank()];

			sys_log(0, "SOCK DROP %d %d", iPercent, iRandRange);
			if (iPercent >= number(1, iRandRange))
			{
				if ((item = CreateItem(SOCK_ITEM_VNUM, 1, 0, true)))
					vec_item.emplace_back(item);
			}
		}
	}

	// Moonlight
	if (quest::CQuestManager::Instance().GetEventFlag("drop_moon"))
	{
		const uint32_t ITEM_VNUM = 50011;

		int iDropPerKill[MOB_RANK_MAX_NUM] =
		{
			2000,
			1000,
			300,
			50,
			0,
			0,
		};

		if (iDropPerKill[pkChr->GetMobRank()])
		{
			int iPercent = 40000 * iDeltaPercent / iDropPerKill[pkChr->GetMobRank()];

			if (iPercent >= number(1, iRandRange))
			{
				if ((item = CreateItem(ITEM_VNUM, 1, 0, true)))
					vec_item.emplace_back(item);
			}
		}
	}

	if (pkKiller->GetLevel() >= 15 && abs(pkKiller->GetLevel() - pkChr->GetLevel()) <= 5)
	{
		int pct = quest::CQuestManager::Instance().GetEventFlag("hc_drop");

		if (pct > 0)
		{
			const uint32_t ITEM_VNUM = 30178;

			if (number(1, 100) <= pct)
			{
				if ((item = CreateItem(ITEM_VNUM, 1, 0, true)))
					vec_item.emplace_back(item);
			}
		}
	}

	//Hexagon
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "2006_drop") >= number(1, iRandRange))
	{
		sys_log(0, "Hexagon DROP EVENT ");

		const static uint32_t dwVnum = 50037;

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.emplace_back(item);

	}

	//Hexagon+
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "2007_drop") >= number(1, iRandRange))
	{
		sys_log(0, "Hexagon DROP EVENT ");

		const static uint32_t dwVnum = 50043;

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// New year firecracker event
	if (GetDropPerKillPct(/* minimum */ 100, /* default */ 1000, iDeltaPercent, "newyear_fire") >= number(1, iRandRange))
	{
		// Chinese firecracker, Korean top
		const uint32_t ITEM_VNUM_FIRE = 50107;

		if ((item = CreateItem(ITEM_VNUM_FIRE, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// New year day event
	if (GetDropPerKillPct(100, 500, iDeltaPercent, "newyear_moon") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT NEWYEAR_MOON DROP");

		const static uint32_t wonso_items[6] = { 50016, 50017, 50018, 50019, 50019, 50019, };
		uint32_t dwVnum = wonso_items[number(0, 5)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// Valentine's Day event. According to the request of OGE, the minimum value of the event is changed to 1. (Other events are left as they are).
	if (GetDropPerKillPct(1, 2000, iDeltaPercent, "valentine_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT VALENTINE_DROP");

		const static uint32_t valentine_items[2] = { 50024, 50025 };
		uint32_t dwVnum = valentine_items[number(0, 1)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// Ice cream event
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "icecream_drop") >= number(1, iRandRange))
	{
		const static uint32_t icecream = 50123;

		if ((item = CreateItem(icecream, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// new christmas event
	// 53002: Baby Reindeer Summon Ticket
	if ((pkKiller->CountSpecifyItem(53002) > 0) && (GetDropPerKillPct(50, 100, iDeltaPercent, "new_xmas_event") >= number(1, iRandRange)))
	{
		const static uint32_t xmas_sock = 50010;
		pkKiller->AutoGiveItem(xmas_sock, 1);
	}

	if ((pkKiller->CountSpecifyItem(53007) > 0) && (GetDropPerKillPct(50, 100, iDeltaPercent, "new_xmas_event") >= number(1, iRandRange)))
	{
		const static uint32_t xmas_sock = 50010;
		pkKiller->AutoGiveItem(xmas_sock, 1);
	}

	/*
	if (pkChr->GetLevel() >= 30 && (GetDropPerKillPct(50, 100, iDeltaPercent, "ds_drop") >= number(1, iRandRange)))
	{
		const static uint32_t dragon_soul_gemstone = 30270;
		if ((item = CreateItem(dragon_soul_gemstone, 1, 0, true)))
		vec_item.emplace_back(item);
	}
	*/

	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "halloween_drop") >= number(1, iRandRange))
	{
		const static uint32_t halloween_item = 30321;

		if ((item = CreateItem(halloween_item, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// Commented for 2013 Ramadan event
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "ramadan_drop") >= number(1, iRandRange))
	{
		const static uint32_t ramadan_item = 30315;

		if ((item = CreateItem(ramadan_item, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "easter_drop") >= number(1, iRandRange))
	{
		const static uint32_t easter_item_base = 50160;

		if ((item = CreateItem(easter_item_base + number(0, 19), 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// World Cup event
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "football_drop") >= number(1, iRandRange))
	{
		const static uint32_t football_item = 50096;

		if ((item = CreateItem(football_item, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// White day event
	if (GetDropPerKillPct(100, 2000, iDeltaPercent, "whiteday_drop") >= number(1, iRandRange))
	{
		sys_log(0, "EVENT WHITEDAY_DROP");
		const static uint32_t whiteday_items[2] = { ITEM_WHITEDAY_ROSE, ITEM_WHITEDAY_CANDY };
		uint32_t dwVnum = whiteday_items[number(0, 1)];

		if ((item = CreateItem(dwVnum, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// Children's Day Mystery Box Event
	if (pkKiller->GetLevel() >= 50)
	{
		if (GetDropPerKillPct(100, 1000, iDeltaPercent, "kids_day_drop_high") >= number(1, iRandRange))
		{
			uint32_t ITEM_QUIZ_BOX = 50034;

			if ((item = CreateItem(ITEM_QUIZ_BOX, 1, 0, true)))
				vec_item.emplace_back(item);
		}
	}
	else
	{
		if (GetDropPerKillPct(100, 1000, iDeltaPercent, "kids_day_drop") >= number(1, iRandRange))
		{
			uint32_t ITEM_QUIZ_BOX = 50034;

			if ((item = CreateItem(ITEM_QUIZ_BOX, 1, 0, true)))
				vec_item.emplace_back(item);
		}
	}

	// Olympic drop event
	if (pkChr->GetLevel() >= 30 && GetDropPerKillPct(50, 100, iDeltaPercent, "medal_part_drop") >= number(1, iRandRange))
	{
		const static uint32_t drop_items[] = { 30265, 30266, 30267, 30268, 30269 };
		int i = number(0, 4);
		item = CreateItem(drop_items[i]);
		if (item != nullptr)
			vec_item.emplace_back(item);
	}

	// ADD_GRANDMASTER_SKILL
	// Mixture item drop
	if (pkChr->GetLevel() >= 40 && pkChr->GetMobRank() >= MOB_RANK_BOSS && GetDropPerKillPct(/* minimum */ 1, /* default */ 1000, iDeltaPercent, "three_skill_item") / GetThreeSkillLevelAdjust(pkChr->GetLevel()) >= number(1, iRandRange))
	{
		const uint32_t ITEM_VNUM = 50513;

		if ((item = CreateItem(ITEM_VNUM, 1, 0, true)))
			vec_item.emplace_back(item);
	}
	// END_OF_ADD_GRANDMASTER_SKILL

	//
	// Seed item drop
	//
	if (GetDropPerKillPct(100, 1000, iDeltaPercent, "dragon_boat_festival_drop") >= number(1, iRandRange))
	{
		const uint32_t ITEM_SEED = 50085;

		if ((item = CreateItem(ITEM_SEED, 1, 0, true)))
			vec_item.emplace_back(item);
	}

	// War God's Blessing Book of the Years drop
	if (pkKiller->GetLevel() >= 15 && quest::CQuestManager::Instance().GetEventFlag("mars_drop"))
	{
		const uint32_t ITEM_HANIRON = 70035;
		int iDropMultiply[MOB_RANK_MAX_NUM] =
		{
			50,
			30,
			5,
			1,
			0,
			0,
		};

		if (iDropMultiply[pkChr->GetMobRank()] &&
			GetDropPerKillPct(1000, 1500, iDeltaPercent, "mars_drop") >= number(1, iRandRange) * iDropMultiply[pkChr->GetMobRank()])
		{
			if ((item = CreateItem(ITEM_HANIRON, 1, 0, true)))
				vec_item.emplace_back(item);
		}
	}
#endif
}

uint32_t ITEM_MANAGER::GetRefineFromVnum(uint32_t dwVnum)
{
	const auto it = m_map_ItemRefineFrom.find(dwVnum);
	if (it != m_map_ItemRefineFrom.end())
		return it->second;

	return 0;
}

const CSpecialItemGroup* ITEM_MANAGER::GetSpecialItemGroup(uint32_t dwVnum)
{
	const auto it = m_map_pkSpecialItemGroup.find(dwVnum);
	if (it != m_map_pkSpecialItemGroup.end())
		return it->second;

	return nullptr;
}

const CSpecialAttrGroup* ITEM_MANAGER::GetSpecialAttrGroup(uint32_t dwVnum)
{
	const auto it = m_map_pkSpecialAttrGroup.find(dwVnum);
	if (it != m_map_pkSpecialAttrGroup.end())
		return it->second;

	return nullptr;
}

uint32_t ITEM_MANAGER::GetMaskVnum(uint32_t dwVnum)
{
	const TMapDW2DW::iterator it = m_map_new_to_ori.find(dwVnum);
	if (it != m_map_new_to_ori.end())
		return it->second;
	else
		return 0;
}

// A function that retrieves all attributes and socket values with pkNewItem.
// TransformRefineItem, a local function that was previously in the char_item.cpp file, is copied as it is
void ITEM_MANAGER::CopyAllAttrTo(LPITEM pkOldItem, LPITEM pkNewItem)
{
	if (!pkOldItem || !pkNewItem)
		return;

	// ACCESSORY_REFINE
	if (pkOldItem->IsAccessoryForSocket())
	{
//#ifdef ENABLE_PROTO_RENEWAL
//		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
//#else
		for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
//#endif
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
			constexpr int ITEM_BROKEN_METIN_VNUM = 28960; // Is there 3 places with the same constant? I'll put it in one
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
			constexpr int ITEM_BROKEN_METIN_VNUM = 28960; // Is there 3 places with the same constant? I'll put it in one
			if (socketRandom > 5 && socketRandom != ITEM_BROKEN_METIN_VNUM)
			{
				pkNewItem->SetSocket(socketRandomSlot++, socketRandom);
			}
		}
#endif
	}

#ifdef ENABLE_REFINE_ELEMENT
	pkNewItem->SetElement(pkOldItem->GetElementGrade(), pkOldItem->GetElementAttacks(), pkOldItem->GetElementsType(), pkOldItem->GetElementsValues());
#endif

	// Magic item setting
	pkOldItem->CopyAttributeTo(pkNewItem);
#ifdef ENABLE_YOHARA_SYSTEM
	pkOldItem->CopyApplyRandomTo(pkNewItem);
#endif
}

#ifdef ENABLE_INGAME_WIKI
uint32_t ITEM_MANAGER::GetWikiItemStartRefineVnum(uint32_t dwVnum)
{
	auto baseItemName = GetWikiItemBaseRefineName(dwVnum);
	if (!baseItemName.size())
		return 0;

	uint32_t manage_vnum = dwVnum;
	while (!(strcmp(baseItemName.c_str(), GetWikiItemBaseRefineName(manage_vnum).c_str())))
		--manage_vnum;

	return (manage_vnum + 1);
}

std::string ITEM_MANAGER::GetWikiItemBaseRefineName(uint32_t dwVnum)
{
	auto* tbl = GetTable(dwVnum);
	if (!tbl)
		return "";

	auto* p = const_cast<char*>(strrchr(tbl->szLocaleName, '+'));
	if (!p)
		return "";

	std::string sFirstItemName(tbl->szLocaleName, (tbl->szLocaleName + (p - tbl->szLocaleName)));

	return sFirstItemName;
}

int ITEM_MANAGER::GetWikiMaxRefineLevel(uint32_t dwVnum)
{
	uint32_t manage_vnum = (GetWikiItemStartRefineVnum(dwVnum) + 1);
	if (manage_vnum <= 1)
		return MAX_REFINE_COUNT;

	int refine_count = 0;
	std::string firstName, secondName;

	while (GetRefineFromVnum(manage_vnum) != 0)
	{
		firstName = GetWikiItemBaseRefineName(manage_vnum);
		secondName = GetWikiItemBaseRefineName(dwVnum);

		if (strcmp(firstName.c_str(), secondName.c_str()))
			break;

		++manage_vnum;
		++refine_count;
	}

	return MAX(refine_count, MAX_REFINE_COUNT);
}

TWikiInfoTable* ITEM_MANAGER::GetItemWikiInfo(uint32_t vnum)
{
	const auto it = m_wikiInfoMap.find(vnum);
	if (it != m_wikiInfoMap.end())
		return it->second.get();

	const auto* tbl = GetTable(vnum);
	if (!tbl)
		return nullptr;

	auto newTable = M2_NEW TWikiInfoTable();
	newTable->is_common = false;

	for (int it = 0; it < MOB_RANK_MAX_NUM && !newTable->is_common; ++it)
	{
		for (auto it2 = g_vec_pkCommonDropItem[it].begin(); it2 != g_vec_pkCommonDropItem[it].end() && !newTable->is_common; ++it2)
		{
			if (it2->m_dwVnum == vnum)
				newTable->is_common = true;
		}
	}

	newTable->origin_vnum = 0;
	newTable->chest_info_count = 0;
	m_wikiInfoMap.insert(std::make_pair(vnum, std::unique_ptr<TWikiInfoTable>(newTable)));

	if ((tbl->bType == ITEM_WEAPON || tbl->bType == ITEM_ARMOR || tbl->bType == ITEM_BELT) && vnum % 10 == 0 && tbl->dwRefinedVnum)
		newTable->refine_infos_count = GetWikiMaxRefineLevel(vnum);
	else if (tbl->bType == ITEM_GIFTBOX)
	{
		CSpecialItemGroup* ptr = nullptr;
		auto it = m_map_pkSpecialItemGroup.find(vnum);
		if (it == m_map_pkSpecialItemGroup.end())
		{
			it = m_map_pkQuestItemGroup.find(vnum);
			if (it != m_map_pkQuestItemGroup.end())
				ptr = it->second;
		}
		else
			ptr = it->second;

		if (ptr)
			newTable->chest_info_count = ptr->m_vecItems.size();
	}

	return newTable;
}

bool ITEM_MANAGER::GetWikiRefineInfo(uint32_t vnum, std::vector<TWikiRefineInfo>& wikiVec)
{
	auto* tbl = GetTable(vnum);
	if (!tbl)
		return false;

	const TRefineTable* refTbl;
	auto* tblTemp = tbl;
	bool success = true;
	const int maxRefineLevelCount = GetWikiMaxRefineLevel(vnum);

	for (int i = 0; i < maxRefineLevelCount; ++i)
	{
		if (!tblTemp) {
			success = false;
			break;
		}

		refTbl = CRefineManager::Instance().GetRefineRecipe(tblTemp->wRefineSet);
		if (!refTbl) {
			success = false;
			break;
		}

		TWikiRefineInfo tmpStruct;
		tmpStruct.index = i;
		tmpStruct.mat_count = refTbl->material_count;
		tmpStruct.price = refTbl->cost;
		for (auto j = 0; j < REFINE_MATERIAL_MAX_NUM; ++j)
		{
			tmpStruct.materials[j].vnum = refTbl->materials[j].vnum;
			tmpStruct.materials[j].count = refTbl->materials[j].count;
		}

		wikiVec.emplace_back(tmpStruct);
		tblTemp = GetTable(tblTemp->dwVnum + 1);
	}

	return true;
}

bool ITEM_MANAGER::GetWikiChestInfo(uint32_t dwItemVnum, std::vector<CSpecialItemGroup::CSpecialItemInfo>& vec_item)
{
	const auto* itemTable = GetTable(dwItemVnum);
	if (!itemTable)
		return false;

	if (itemTable->bType == ITEM_GIFTBOX)
	{
		const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(dwItemVnum);
		if (!pGroup)
		{
			sys_err("cannot find special item group %d", dwItemVnum);
			return false;
		}

		vec_item.assign(pGroup->m_vecItems.begin(), pGroup->m_vecItems.end());
	}

	return true;
}
#endif
