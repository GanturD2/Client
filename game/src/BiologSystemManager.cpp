#include "stdafx.h"

#ifdef ENABLE_BIOLOG_SYSTEM
#include "BiologSystemManager.h"
#include "questmanager.h"
#include "char.h"
#include "packet.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "char_manager.h"
#include "p2p.h"
#include "item_manager.h"
#include "item.h"
#include "utils.h"

#include "mob_manager.h"

/*******************************************************************\
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
|||| CBiologSystem - CLASS
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
\*******************************************************************/

/*******************************************************************\
| [PUBLIC] (De-)Initialize Functions
\*******************************************************************/

CBiologSystem::CBiologSystem(LPCHARACTER m_ch)
{
	m_pkChar = m_ch;
}

CBiologSystem::~CBiologSystem()
{
}

/*******************************************************************\
| [PUBLIC] Update Functions
\*******************************************************************/

void CBiologSystem::ResetMission()
{
	if (!m_pkChar)
		return;

	m_pkChar->SetBiologCollectedItems(0);
	m_pkChar->SetBiologCooldown(0);
	m_pkChar->SetBiologCooldownReminder(0);
}

/*******************************************************************\
| [PUBLIC] General Functions
\*******************************************************************/

void CBiologSystem::SendBiologInformation(bool bUpdate)
{
	if (!m_pkChar)
		return;

	const auto bMission = m_pkChar->GetBiologMissions();
	const auto iCount = m_pkChar->GetBiologCollectedItems();
	const auto tWait = m_pkChar->GetBiologCooldown();
	const auto tReminder = m_pkChar->GetBiologCooldownReminder();

	const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(bMission);
	if (!pMission)
	{
		m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You currently have no research missions."));
		return;
	}

	if (m_pkChar->GetLevel() < pMission->bRequiredLevel)
	{
		m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You currently have no research missions."));
		return;
	}

	const TBiologRewardsProto* pReward = CBiologSystemManager::Instance().GetReward(bMission);
	if (!pReward)
		return;

	TPacketGCBiologManagerInfo kInfo;
	memset(&kInfo, 0, sizeof(kInfo));

	//copy data
	kInfo.bUpdate = bUpdate;
	kInfo.bRequiredLevel = pMission->bRequiredLevel;
	kInfo.iRequiredItem = pMission->iRequiredItem;
	kInfo.wGivenItems = iCount;
	kInfo.wRequiredItemCount = pMission->wRequiredItemCount;
	kInfo.iGlobalCooldown = pMission->iCooldown;
	kInfo.iCooldown = (tWait - get_global_time());
	kInfo.iCooldownReminder = tReminder;
	kInfo.bChance = pMission->bChance;
	for (size_t i = 0; i < MAX_BONUSES_LENGTH; i++)
	{
		kInfo.wApplyType[i] = pReward->wApplyType[i] - 1;
		kInfo.lApplyValue[i] = pReward->lApplyValue[i];
	}

	kInfo.dRewardItem = pReward->dRewardItem;
	kInfo.wRewardItemCount = pReward->wRewardItemCount;

	kInfo.bSubMission = pMission->bSubMission;
	kInfo.dwNpcVnum = pMission->dwNpcVnum;
	kInfo.dwRequiredSubItem = pMission->dwRequiredSubItem;

	SendClientPacket(m_pkChar->GetDesc(), BIOLOG_MANAGER_SUBHEADER_GC_OPEN, &kInfo, sizeof(kInfo));
}

void CBiologSystem::SendBiologItem()
{
	if (!m_pkChar)
		return;

	const auto bMission = m_pkChar->GetBiologMissions();
	const auto tWait = m_pkChar->GetBiologCooldown();
	const auto iCount = m_pkChar->GetBiologCollectedItems();

	// Getting actual biolog mission
	const TBiologMissionsProto* pMission = CBiologSystemManager::Instance().GetMission(bMission);
	if (!pMission || bMission == CBiologSystemManager::Instance().m_mapMission_Proto.size())
	{
		//We have to inform players they've done every mission!
		return;
	}

	const CMob* pkMob = CMobManager::Instance().Get(pMission->dwNpcVnum);

	// Checking required level
	if (m_pkChar->GetLevel() < pMission->bRequiredLevel)
	{
		m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s : You are not level enough to do this mission."), pkMob ? pkMob->m_table.szLocaleName : "Biologist");
		return;
	}

	// Counting required item
	if (m_pkChar->CountSpecifyItem(pMission->iRequiredItem) < 1)
	{
		m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s : You don't have the necessary item this investigation requires."), pkMob ? pkMob->m_table.szLocaleName : "Biologist");
		return;
	}

	// Checking time
	if (get_global_time() < tWait)
	{
		m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s : Investigation time has not been completed yet. Try again later."), pkMob ? pkMob->m_table.szLocaleName : "Biologist");

		return;
	}

	// Checking percent
	uint8_t chance = pMission->bChance;
#ifdef ENABLE_RESEARCHER_ELIXIR_FIX
	if (m_pkChar->FindAffect(AFFECT_RESEARCHER_ELIXIR))
	{
		m_pkChar->RemoveAffect(AFFECT_RESEARCHER_ELIXIR);
		chance = 90;
	}
#endif

	// Checking count
	if (iCount < pMission->wRequiredItemCount) // it means you haven't give all items
	{
		m_pkChar->RemoveSpecifyItem(pMission->iRequiredItem, 1);

		if (chance >= number(1, 100))
		{
			// Update count
			m_pkChar->SetBiologCollectedItems(iCount + 1);
			m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s : This item is of the best quality. I will begin his investigation immediately."), pkMob ? pkMob->m_table.szLocaleName : "Biologist");
		}
		else
		{
			m_pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s : I cannot learn much from this item due to its degree of deterioration. Please give me another one."), pkMob ? pkMob->m_table.szLocaleName : "Biologist");
		}

		// Compare current count with required count
		if (m_pkChar->GetBiologCollectedItems() == pMission->wRequiredItemCount)
		{
			if (pMission->bSubMission == true)
			{
				SendBiologInformation(true);
				char biolog_state[1024];
				snprintf(biolog_state, sizeof(biolog_state), "biolog_manager.can_do_sub_mission_%d", bMission);
				m_pkChar->SetQuestFlag(biolog_state, 1);
				return;
			}

			if ((pMission->bSubMission == false) && (pMission->dwNpcVnum == 20091))
			{
				SendBiologInformation(true);
				char biolog_reward[1024];
				snprintf(biolog_reward, sizeof(biolog_reward), "biolog_manager.can_select_reward_%d", bMission);
				m_pkChar->SetQuestFlag(biolog_reward, 1);
				return;
			}
		}

		// Update cooldown
		m_pkChar->SetBiologCooldown(get_global_time() + pMission->iCooldown);

		// Active Alert
		m_pkChar->SetBiologRemiderEvent(false);
		if (m_pkChar->GetBiologCooldownReminder())
			m_pkChar->SetBiologCooldownReminder(1);

		SendBiologInformation(true);
	}
}

/*******************************************************************\
| [PUBLIC] Incoming Packet Functions
\*******************************************************************/

int CBiologSystem::RecvClientPacket(uint8_t bSubHeader, const char* c_pData, size_t uiBytes)
{
	switch (bSubHeader)
	{
	case BIOLOG_MANAGER_SUBHEADER_CG_OPEN:
	{
		SendBiologInformation();
		return 0;
	}

	case BIOLOG_MANAGER_SUBHEADER_CG_SEND:
	{
		SendBiologItem();
		return 0;
	}

	case BIOLOG_MANAGER_SUBHEADER_CG_TIMER:
	{
		if (uiBytes < sizeof(bool))
			return -1;

		const bool bReminder = *(bool*)c_pData;
		c_pData += sizeof(bool);
		uiBytes -= sizeof(bool);

		m_pkChar->SetBiologCooldownReminder(bReminder);
		return sizeof(bool);
	}

	default:
		sys_err("CBiologSystem::RecvClientPacket : Unknown subheader %d : %s", bSubHeader, m_pkChar->GetName());
		break;
	}

	return 0;
}

/*******************************************************************\
| [PUBLIC] Outgoing Packet Functions
\*******************************************************************/

void CBiologSystem::SendClientPacket(LPDESC pkDesc, uint8_t bSubHeader, const void* c_pvData, size_t iSize)
{
	if (!pkDesc)
		return;

	TPacketGCBiologManager packet{};
	packet.bHeader = HEADER_GC_BIOLOG_MANAGER;
	packet.wSize = static_cast<uint16_t>(sizeof(packet) + iSize);
	packet.bSubHeader = bSubHeader;

	TEMP_BUFFER buf;
	buf.write(&packet, sizeof(packet));
	if (iSize)
		buf.write(c_pvData, iSize);

	pkDesc->Packet(buf.read_peek(), buf.size());
}

void CBiologSystem::SendClientPacket(uint32_t dwPID, uint8_t bSubHeader, const void* c_pvData, size_t iSize)
{
	const LPCHARACTER& pkChr = CHARACTER_MANAGER::Instance().FindByPID(dwPID);

	if (pkChr)
	{
		SendClientPacket(pkChr->GetDesc(), bSubHeader, c_pvData, iSize);
	}
	else
	{
		CCI* pkCCI = P2P_MANAGER::Instance().FindByPID(dwPID);
		if (pkCCI)
		{
			pkCCI->pkDesc->SetRelay(pkCCI->szName);
			SendClientPacket(pkCCI->pkDesc, bSubHeader, c_pvData, iSize);
		}
		else
			sys_err("cannot send client packet to pid %u subheader %hu [cannot find player]", dwPID, bSubHeader);
	}
}

/*******************************************************************\
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
|||| CBiologSystemManager - CLASS
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
\*******************************************************************/

/*******************************************************************\
| [PUBLIC] (De-)Initialize Functions
\*******************************************************************/

CBiologSystemManager::CBiologSystemManager() {}

CBiologSystemManager::~CBiologSystemManager() {}

void CBiologSystemManager::InitializeMissions(TBiologMissionsProto* pData, size_t size)
{
	if (test_server)
		sys_err("InitializeMissions %d", size);

	for (size_t i{ 0 }; i < size; ++i, ++pData)
	{
		//sys_log(0, "AppendData before", m_mapMission_Proto.size());
		thecore_memcpy(&m_mapMission_Proto[pData->bMission], pData, sizeof(TBiologMissionsProto));
		//sys_log(0, "AppendData after", m_mapMission_Proto.size());
	}
}

TBiologMissionsProto* CBiologSystemManager::GetMission(uint8_t bMission)
{
	const TMissionProtoMap::iterator it = m_mapMission_Proto.find(bMission);
	if (it == m_mapMission_Proto.end())
		return nullptr;

	return &it->second;
}

void CBiologSystemManager::InitializeRewards(TBiologRewardsProto* pData, size_t size)
{
	for (size_t i{ 0 }; i < size; ++i, ++pData)
	{
		thecore_memcpy(&m_mapReward_Proto[pData->bMission], pData, sizeof(TBiologRewardsProto));
	}
}

TBiologRewardsProto* CBiologSystemManager::GetReward(uint8_t bMission)
{
	const TRewardProtoMap::iterator it = m_mapReward_Proto.find(bMission);
	if (it == m_mapReward_Proto.end())
		return nullptr;

	return &it->second;
}
#endif
