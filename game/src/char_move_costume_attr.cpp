#include "stdafx.h"
#ifdef ENABLE_MOVE_COSTUME_ATTR
#include "constants.h"
#include "log.h"
#include "char.h"
#include "item.h"
#include "item_manager.h"

bool CHARACTER::MoveCostumeAttr(uint8_t bSlotMedium, uint8_t bSlotBase, uint8_t bSlotMaterial)
{
	if (IsHack() || IsWarping())
		return false;

	/*if (m_pkTimedEvent)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;511]");
		event_cancel(&m_pkTimedEvent);
		return false;
	}*/

	LPITEM itemMedium = GetInventoryItem(bSlotMedium);
	if (!itemMedium)
		return false;

	LPITEM itemBase = GetInventoryItem(bSlotBase);
	if (!itemBase)
		return false;

	LPITEM itemMaterial = GetInventoryItem(bSlotMaterial);
	if (!itemMaterial)
		return false;

	LPCHARACTER npc = GetQuestNPC();
	if (!npc)
	{
		sys_log(0, "%s has try to open the transfer the bonuses between costumes without talk to the NPC.", GetName());
		return false;
	}

	if ((itemMedium->GetType() != ITEM_MEDIUM) &&
		(itemMedium->GetSubType() != MEDIUM_MOVE_COSTUME_ATTR
#	ifdef ENABLE_MOVE_COSTUME_ACCE_ATTR
		|| itemMedium->GetSubType() != MEDIUM_MOVE_ACCE_ATTR
#	endif
		))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1056]");
		return false;
	}

	if (itemBase->GetType() != ITEM_COSTUME || itemMaterial->GetType() != ITEM_COSTUME)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1937]");
		return false;
	}

	if ((itemBase->GetSubType() != COSTUME_BODY && itemBase->GetSubType() != COSTUME_HAIR) && 
		(itemBase->GetSubType() != COSTUME_WEAPON || itemMaterial->GetSubType() != COSTUME_BODY) && 
		(itemMaterial->GetSubType() != COSTUME_HAIR) && (itemMaterial->GetSubType() != COSTUME_WEAPON))
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1938]");
		return false;
	}

	if (itemMaterial->GetSubType() != itemBase->GetSubType())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1939]");
		return false;
	}

#ifdef ENABLE_SEALBIND_SYSTEM
	if (itemBase->IsSealed() || itemMaterial->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1101]");
		return false;
	}
#endif

	const TItemTable* itemBaseTable = ITEM_MANAGER::Instance().GetTable(itemBase->GetVnum());
	if (nullptr == itemBaseTable)
		return false;

	const TItemTable* itemMaterialTable = ITEM_MANAGER::Instance().GetTable(itemMaterial->GetVnum());
	if (nullptr == itemMaterialTable)
		return false;

	if (itemBaseTable->bAlterToMagicItemPct != 100 || itemMaterialTable->bAlterToMagicItemPct != 100)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;1940]");
		return false;
	}

	if (itemMaterial->GetAttributeCount() == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, "[LS;397]");
		return false;
	}

	for (int i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		itemBase->SetForceAttribute(i, itemMaterial->GetAttributeType(i), itemMaterial->GetAttributeValue(i));
	}

	itemMedium->SetCount(itemMedium->GetCount() - 1);
	itemMaterial->SetCount(itemMaterial->GetCount() - 1);

	LogManager::Instance().MoveCostumeAttrLog(GetPlayerID(), GetX(), GetY(), itemBase->GetVnum());

	ChatPacket(CHAT_TYPE_INFO, "[LS;1336]");

	return true;
}


#endif
