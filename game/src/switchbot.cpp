#include "stdafx.h"

#ifdef ENABLE_SWITCHBOT
#include "switchbot.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "char_manager.h"
#include "buffer_manager.h"
#include "char.h"
#include "config.h"
#include "p2p.h"

bool ValidPosition(uint32_t wCell)
{
	return wCell < SWITCHBOT_SLOT_COUNT;
}

constexpr float c_fSpeed = 0.2f; // safe speed

bool SwitchbotHelper::IsValidItem(LPITEM pkItem)
{
	if (!pkItem)
		return false;

	switch (pkItem->GetType())
	{
	case ITEM_WEAPON:
		return true;

	case ITEM_ARMOR:
		return true;

#ifdef ENABLE_USE_COSTUME_ATTR
	case ITEM_COSTUME:
	{
		if (pkItem->GetSubType() == COSTUME_BODY || pkItem->GetSubType() == COSTUME_HAIR || pkItem->GetSubType() == COSTUME_WEAPON)
			return true;
	}
#endif

	default:
		break;
	}

	return false;
}


CSwitchbot::CSwitchbot()
{
	m_pkSwitchEvent = nullptr;
	m_table = {};
	m_isWarping = false;
}

CSwitchbot::~CSwitchbot()
{
	if (m_pkSwitchEvent)
		event_cancel(&m_pkSwitchEvent);

	m_pkSwitchEvent = nullptr;
	m_table = {};
	m_isWarping = false;
}

void CSwitchbot::SetTable(TSwitchbotTable table)
{
	m_table = table;
}

TSwitchbotTable CSwitchbot::GetTable()
{
	return m_table;
}

void CSwitchbot::SetPlayerId(uint32_t player_id)
{
	m_table.player_id = player_id;
}

uint32_t CSwitchbot::GetPlayerId(uint32_t player_id)
{
	return m_table.player_id;
}

void CSwitchbot::RegisterItem(uint16_t wCell, uint32_t item_id)
{
	if (!ValidPosition(wCell))
		return;

	m_table.items[wCell] = item_id;
}

void CSwitchbot::UnregisterItem(uint16_t wCell)
{
	if (!ValidPosition(wCell))
		return;

	m_table.items[wCell] = 0;
	m_table.active[wCell] = false;
	m_table.finished[wCell] = false;
	memset(&m_table.alternatives[wCell], 0, sizeof(m_table.alternatives[wCell]));
}

void CSwitchbot::SetAttributes(uint8_t slot, std::vector<TSwitchbotAttributeAlternativeTable> vec_alternatives)
{
	if (!ValidPosition(slot))
		return;

	for (uint8_t alternative = 0; alternative < SWITCHBOT_ALTERNATIVE_COUNT; ++alternative)
	{
		for (uint8_t attrIdx = 0; attrIdx < ITEM_ATTRIBUTE_NORM_NUM; ++attrIdx)
		{
			m_table.alternatives[slot][alternative].attributes[attrIdx].wType = vec_alternatives[alternative].attributes[attrIdx].wType;
			m_table.alternatives[slot][alternative].attributes[attrIdx].sValue = vec_alternatives[alternative].attributes[attrIdx].sValue;
		}
	}
}

void CSwitchbot::SetActive(uint8_t slot, bool active)
{
	if (!ValidPosition(slot))
		return;

	m_table.active[slot] = active;
	m_table.finished[slot] = false;
}

EVENTINFO(TSwitchbotEventInfo)
{
	CSwitchbot* pkSwitchbot;

	TSwitchbotEventInfo() : pkSwitchbot(nullptr)
	{
	}
};

EVENTFUNC(switchbot_event)
{
	TSwitchbotEventInfo* info = dynamic_cast<TSwitchbotEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("switchbot_event> <Factor> Info Null pointer");
		return 0;
	}

	if (!info->pkSwitchbot)
	{
		sys_err("switchbot_event> <Factor> Switchbot Null pointer");
		return 0;
	}

	info->pkSwitchbot->SwitchItems();

	return PASSES_PER_SEC(c_fSpeed);
}

void CSwitchbot::Start()
{
	TSwitchbotEventInfo* info = AllocEventInfo<TSwitchbotEventInfo>();
	info->pkSwitchbot = this;

	m_pkSwitchEvent = event_create(switchbot_event, info, c_fSpeed);

	CSwitchbotManager::Instance().SendSwitchbotUpdate(m_table.player_id);
}

void CSwitchbot::Stop()
{
	if (m_pkSwitchEvent)
	{
		event_cancel(&m_pkSwitchEvent);
		m_pkSwitchEvent = nullptr;
	}

	memset(&m_table.active, 0, sizeof(m_table.active));

	CSwitchbotManager::Instance().SendSwitchbotUpdate(m_table.player_id);
}

void CSwitchbot::Pause()
{
	if (m_pkSwitchEvent)
	{
		event_cancel(&m_pkSwitchEvent);
		m_pkSwitchEvent = nullptr;
	}
}

bool CSwitchbot::IsActive(uint8_t slot)
{
	if (!ValidPosition(slot))
		return false;

	return m_table.active[slot];
}

bool CSwitchbot::HasActiveSlots()
{
	for (const auto& it : m_table.active)
	{
		if (it)
			return true;
	}

	return false;
}

bool CSwitchbot::IsSwitching()
{
	return m_pkSwitchEvent != nullptr;
}

bool CSwitchbot::IsWarping()
{
	return m_isWarping;
}

void CSwitchbot::SetIsWarping(bool warping)
{
	m_isWarping = warping;
}

void CSwitchbot::SwitchItems()
{
	for (uint8_t bSlot = 0; bSlot < SWITCHBOT_SLOT_COUNT; ++bSlot)
	{
		if (!m_table.active[bSlot])
			continue;

		m_table.finished[bSlot] = false;

		const uint32_t item_id = m_table.items[bSlot];

		LPITEM pkItem = ITEM_MANAGER::Instance().Find(item_id);
		if (!pkItem)
			continue;

		LPCHARACTER pkOwner = pkItem->GetOwner();
		if (!pkOwner)
			return;

		if (CheckItem(pkItem, bSlot))
		{
			const LPDESC& desc = pkOwner->GetDesc();
			if (desc)
			{
				char buf[255];
				const int len = snprintf(buf, sizeof(buf), LC_TEXT("(Slot: %d) Bonuschange of %s successfully finished."), bSlot + 1, pkItem->GetName());

				TPacketGCWhisper pack{};
				pack.bHeader = HEADER_GC_WHISPER;
				pack.bType = WHISPER_TYPE_SYSTEM;
				pack.wSize = sizeof(TPacketGCWhisper) + len;
				strlcpy(pack.szNameFrom, "[Switchbot]", sizeof(pack.szNameFrom));
				pkOwner->GetDesc()->BufferedPacket(&pack, sizeof(pack));
				pkOwner->GetDesc()->Packet(buf, len);
			}

			SetActive(bSlot, false);

			m_table.finished[bSlot] = true;

			if (!HasActiveSlots())
				Stop();
			else
				CSwitchbotManager::Instance().SendSwitchbotUpdate(m_table.player_id);
		}
		else
		{
			bool stop = true;
			if (SWITCHBOT_PRICE_TYPE == 1)
			{
				for (const auto& itemVnum : c_arSwitchingItems)
				{
					switch (itemVnum)
					{
						//CHECK_LIMITED_ITEM START
					case 71151:
					case 76023:
					{
#ifndef ENABLE_IGNORE_TYPE_GREEN_ATTR_ITEM
						if ((pkItem->GetType() == ITEM_WEAPON) || (pkItem->GetType() == ITEM_ARMOR && pkItem->GetSubType() == ARMOR_BODY))
#endif
						{
							bool bCanUse = true;

#ifdef ENABLE_USE_COSTUME_ATTR
							if (pkItem->GetType() == ITEM_COSTUME)
								bCanUse = false;
#endif

							for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
							{
								if (pkItem->GetLimitType(i) == LIMIT_LEVEL && pkItem->GetLimitValue(i) > 40)
								{
									bCanUse = false;
									break;
								}
							}
							if (false == bCanUse)
								continue;
						}
#ifndef ENABLE_IGNORE_TYPE_GREEN_ATTR_ITEM
						else
						{
							continue;
						}
#endif
					}
					break;
					//CHECK_LIMITED_ITEM END

#ifdef ENABLE_USE_COSTUME_ATTR
					case 70064:
					{
						bool bCanUse = false;
						if (pkItem->GetType() == ITEM_COSTUME && (pkItem->GetSubType() == COSTUME_BODY || pkItem->GetSubType() == COSTUME_HAIR || pkItem->GetSubType() == COSTUME_WEAPON))
							bCanUse = true;

						if (false == bCanUse)
							continue;
					}
					break;
#endif

					default:
						break;
					}

					if (pkOwner->CountSpecifyItem(itemVnum) >= SWITCHBOT_PRICE_AMOUNT)
					{
						stop = false;
					}
				}
			}
			else if (SWITCHBOT_PRICE_TYPE == 2)
			{
				if (pkOwner->GetGold() >= SWITCHBOT_PRICE_AMOUNT)
				{
					stop = false;
				}
			}

			if (stop)
			{
				Stop();

#ifdef ENABLE_SWITCHBOT_PM
				if (SWITCHBOT_PRICE_TYPE == 1)
				{
					pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Switchbot stopped. Out of switchers."));
				}
				else
				{
					pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Switchbot stopped. Not enough money."));
				}
#endif
				return;
			}

			if (SWITCHBOT_PRICE_TYPE == 1)
			{
				for (const auto& itemVnum : c_arSwitchingItems)
				{
					LPITEM pkItem = pkOwner->FindSpecifyItem(itemVnum);
					if (pkItem)
					{
						pkItem->SetCount(pkItem->GetCount() - SWITCHBOT_PRICE_AMOUNT);
						break;
					}
				}
			}
			else if (SWITCHBOT_PRICE_TYPE == 2)
			{
				pkOwner->PointChange(POINT_GOLD, -SWITCHBOT_PRICE_AMOUNT, false);
			}

			pkItem->ChangeAttribute();
			SendItemUpdate(pkOwner, bSlot, pkItem);
		}
	}
}

bool CSwitchbot::CheckItem(LPITEM pkItem, uint8_t slot)
{
	if (!ValidPosition(slot))
		return false;

	if (!pkItem)
		return false;

	bool checked = 0;

	for (const auto& alternative : m_table.alternatives[slot])
	{
		if (!alternative.IsConfigured())
			continue;

		uint8_t configuredAttrCount = 0;
		uint8_t correctAttrCount = 0;

		for (const auto& destAttr : alternative.attributes)
		{
			if (!destAttr.wType || !destAttr.sValue)
				continue;

			++configuredAttrCount;

			for (uint8_t attrIdx = 0; attrIdx < ITEM_ATTRIBUTE_NORM_NUM; ++attrIdx)
			{
				const TPlayerItemAttribute& curAttr = pkItem->GetAttribute(attrIdx);
				if (curAttr.wType != destAttr.wType || curAttr.sValue < destAttr.sValue)
					continue;

				++correctAttrCount;
				break;
			}
		}

		checked = true;

		if (configuredAttrCount == correctAttrCount)
			return true;
	}

	if (!checked)
		return true;

	return false;
}

void CSwitchbot::SendItemUpdate(LPCHARACTER ch, uint8_t slot, LPITEM item)
{
	LPDESC desc = ch->GetDesc();
	if (!desc)
		return;

	TPacketGCSwitchbot pack{};
	pack.header = HEADER_GC_SWITCHBOT;
	pack.subheader = SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM;
	pack.size = sizeof(TPacketGCSwitchbot) + sizeof(TSwitchbotUpdateItem);

	TSwitchbotUpdateItem update = {};
	update.slot = slot;
	update.vnum = item->GetVnum();
	update.count = item->GetCount();

	thecore_memcpy(update.alSockets, item->GetSockets(), sizeof(update.alSockets));
	thecore_memcpy(update.aAttr, item->GetAttributes(), sizeof(update.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	thecore_memcpy(update.aApplyRandom, item->GetApplysRandom(), sizeof(update.aApplyRandom));
	thecore_memcpy(update.alRandomValues, item->GetRandomDefaultAttrs(), sizeof(update.alRandomValues));
#endif

	desc->BufferedPacket(&pack, sizeof(pack));
	desc->Packet(&update, sizeof(TSwitchbotUpdateItem));
}

CSwitchbotManager::CSwitchbotManager()
{
	Initialize();
}

CSwitchbotManager::~CSwitchbotManager()
{
	Initialize();
}

void CSwitchbotManager::Initialize()
{
	m_map_Switchbots.clear();
}

void CSwitchbotManager::RegisterItem(uint32_t player_id, uint32_t item_id, uint16_t wCell)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
	{
		pkSwitchbot = new CSwitchbot();
		pkSwitchbot->SetPlayerId(player_id);
		m_map_Switchbots.insert(std::make_pair(player_id, pkSwitchbot));
	}

	if (pkSwitchbot->IsWarping())
		return;

	pkSwitchbot->RegisterItem(wCell, item_id);
	SendSwitchbotUpdate(player_id);
}

void CSwitchbotManager::UnregisterItem(uint32_t player_id, uint16_t wCell)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
		return;

	if (pkSwitchbot->IsWarping())
		return;

	pkSwitchbot->UnregisterItem(wCell);
	SendSwitchbotUpdate(player_id);
}

void CSwitchbotManager::Start(uint32_t player_id, uint8_t slot, std::vector<TSwitchbotAttributeAlternativeTable> vec_alternatives)
{
	if (!ValidPosition(slot))
		return;

	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
	{
		sys_err("No Switchbot found for player_id %d slot %d", player_id, slot);
		return;
	}

	if (pkSwitchbot->IsActive(slot))
	{
		sys_err("Switchbot slot %d already running for player_id %d", slot, player_id);
		return;
	}

	pkSwitchbot->SetActive(slot, true);
	pkSwitchbot->SetAttributes(slot, vec_alternatives);

	if (pkSwitchbot->HasActiveSlots() && !pkSwitchbot->IsSwitching())
		pkSwitchbot->Start();
	else
		SendSwitchbotUpdate(player_id);
}

void CSwitchbotManager::Stop(uint32_t player_id, uint8_t slot)
{
	if (!ValidPosition(slot))
		return;

	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
	{
		sys_err("No Switchbot found for player_id %d slot %d", player_id, slot);
		return;
	}

	if (!pkSwitchbot->IsActive(slot))
	{
		sys_err("Switchbot slot %d is not running for player_id %d", slot, player_id);
		return;
	}

	pkSwitchbot->SetActive(slot, false);

	if (!pkSwitchbot->HasActiveSlots() && pkSwitchbot->IsSwitching())
		pkSwitchbot->Stop();
	else
		SendSwitchbotUpdate(player_id);
}

bool CSwitchbotManager::IsActive(uint32_t player_id, uint8_t slot)
{
	if (!ValidPosition(slot))
		return false;

	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
		return false;

	return pkSwitchbot->IsActive(slot);
}

bool CSwitchbotManager::IsWarping(uint32_t player_id)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
		return false;

	return pkSwitchbot->IsWarping();
}

void CSwitchbotManager::SetIsWarping(uint32_t player_id, bool warping)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
		return;

	pkSwitchbot->SetIsWarping(warping);
}

CSwitchbot* CSwitchbotManager::FindSwitchbot(uint32_t player_id)
{
	const auto& it = m_map_Switchbots.find(player_id);
	if (it == m_map_Switchbots.end())
		return nullptr;

	return it->second;
}

void CSwitchbotManager::P2PSendSwitchbot(uint32_t player_id, uint16_t wTargetPort)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
	{
		// sys_err("No switchbot found to transfer. (pid %d source_port %d target_port %d)", player_id, mother_port, wTargetPort); // just for debug
		return;
	}

	pkSwitchbot->Pause();
	m_map_Switchbots.erase(player_id);

	TPacketGGSwitchbot pack;
	pack.wPort = wTargetPort;
	pack.table = pkSwitchbot->GetTable();

	P2P_MANAGER::Instance().Send(&pack, sizeof(pack));
}

void CSwitchbotManager::P2PReceiveSwitchbot(TSwitchbotTable table)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(table.player_id);
	if (!pkSwitchbot)
	{
		pkSwitchbot = new CSwitchbot();
		m_map_Switchbots.insert(std::make_pair(table.player_id, pkSwitchbot));
	}

	pkSwitchbot->SetTable(table);
}

void CSwitchbotManager::SendItemAttributeInformations(LPCHARACTER ch)
{
	if (!ch)
		return;

	LPDESC desc = ch->GetDesc();
	if (!desc)
		return;

	TPacketGCSwitchbot pack{};
	pack.header = HEADER_GC_SWITCHBOT;
	pack.subheader = SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION;
	pack.size = sizeof(TPacketGCSwitchbot);

	TEMP_BUFFER buf;
	for (uint8_t bAttributeSet = 0; bAttributeSet < ATTRIBUTE_SET_MAX_NUM; ++bAttributeSet)
	{
		for (int iApplyNum = 0; iApplyNum < MAX_APPLY_NUM; ++iApplyNum)
		{
			const TItemAttrTable& r = g_map_itemAttr[iApplyNum];

			const uint8_t max = r.bMaxLevelBySet[bAttributeSet];
			if (max > 0)
			{
				TSwitchbottAttributeTable table = {};
				table.attribute_set = bAttributeSet;
				table.apply_num = iApplyNum;
				table.max_value = r.lValues[max - 1];

				buf.write(&table, sizeof(table));
			}
		}
	}

	if (buf.size())
	{
		pack.size += buf.size();
		desc->BufferedPacket(&pack, sizeof(pack));
		desc->Packet(buf.read_peek(), buf.size());
	}
	else
	{
		desc->Packet(&pack, sizeof(pack));
	}
}

void CSwitchbotManager::SendSwitchbotUpdate(uint32_t player_id)
{
	CSwitchbot* pkSwitchbot = FindSwitchbot(player_id);
	if (!pkSwitchbot)
		return;

	const LPCHARACTER& ch = CHARACTER_MANAGER::Instance().FindByPID(player_id);
	if (!ch)
		return;

	LPDESC desc = ch->GetDesc();
	if (!desc)
		return;

	const TSwitchbotTable table = pkSwitchbot->GetTable();

	TPacketGCSwitchbot pack{};
	pack.header = HEADER_GC_SWITCHBOT;
	pack.subheader = SUBHEADER_GC_SWITCHBOT_UPDATE;
	pack.size = sizeof(TPacketGCSwitchbot) + sizeof(TSwitchbotTable);

	desc->BufferedPacket(&pack, sizeof(pack));
	desc->Packet(&table, sizeof(table));
}

void CSwitchbotManager::EnterGame(LPCHARACTER ch)
{
	if (!ch)
		return;

	SendItemAttributeInformations(ch);
	SetIsWarping(ch->GetPlayerID(), false);
	SendSwitchbotUpdate(ch->GetPlayerID());

	CSwitchbot* pkSwitchbot = FindSwitchbot(ch->GetPlayerID());
	if (pkSwitchbot && pkSwitchbot->HasActiveSlots() && !pkSwitchbot->IsSwitching())
		pkSwitchbot->Start();
}
#endif
