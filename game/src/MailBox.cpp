/*
* Blackdragonx61 / Mali
* 07.09.2021
*/
#include "stdafx.h"

#ifdef ENABLE_MAILBOX
#include "MailBox.h"
#include "char.h"
#include "char_manager.h"
#include "desc.h"
#include "packet.h"
#include "item.h"
#include "item_manager.h"
#include "banword.h"
#include "buffer_manager.h"
#include "db.h"
#include "config.h"
#include "desc_client.h"

CMailBox::CMailBox(const LPCHARACTER m_ch, const TMailBoxTable* pTable, const uint16_t Size)
	: Owner(m_ch)
{
	for (uint16_t i = 0; i < Size; ++i, ++pTable)
		vecMailBox.emplace_back(*pTable);

	/*Load*/
	TEMP_BUFFER buf;
	for (const TMailBoxTable& mail : vecMailBox)
	{
		TPacketGCMailBoxMessage p;
		p.SendTime = mail.Message.SendTime;
		p.DeleteTime = mail.Message.DeleteTime;
		std::memcpy(&p.szTitle, mail.Message.szTitle, sizeof(p.szTitle));
		p.bIsGMPost = mail.Message.bIsGMPost;
		p.bIsItemExist = mail.Message.bIsItemExist;
		p.bIsConfirm = mail.Message.bIsConfirm;
		buf.write(&p, sizeof(p));
	}
	
	TPacketGCMailBox pack{};
	pack.bHeader = HEADER_GC_MAILBOX;
	pack.wSize = sizeof(pack) + buf.size();
	if (buf.size())
	{
		Owner->GetDesc()->BufferedPacket(&pack, sizeof(pack));
		Owner->GetDesc()->Packet(buf.read_peek(), buf.size());
	}
	else
		Owner->GetDesc()->Packet(&pack, sizeof(pack));

	ServerProcess(EMAILBOX_GC::MAILBOX_GC_OPEN, true);
}

CMailBox::~CMailBox()
{
	ServerProcess(EMAILBOX_GC::MAILBOX_GC_CLOSE);
	Owner->SetMailBoxLoading(false);
	Owner->SetMyMailBoxTime();
}

void CMailBox::ServerProcess(const uint8_t SubHeader, const uint8_t arg1, const uint8_t arg2) const
{
	if (Owner == nullptr)
		return;

	const LPDESC d = Owner->GetDesc();
	if (d == nullptr)
		return;

	TPacketMailboxProcess p{};
	p.bHeader = HEADER_GC_MAILBOX_PROCESS;
	p.bSubHeader = SubHeader;
	p.bArg1 = arg1;
	p.bArg2 = arg2;
	d->Packet(&p, sizeof(p));
}

void CMailBox::Write(const char* const szName, const char* const szTitle, const char* const szMessage,
	const TItemPos& pos, const int iYang, const int iWon)
{
	const LPDESC d = Owner->GetDesc();
	if (d == nullptr)
		return;

	sys_err("szName %s, szTitle %s, szMessage %s", szName, szTitle, szMessage);
	
	const int TotalYang = iYang + EMAILBOX::MAILBOX_PRICE_YANG;
	if (TotalYang > Owner->GetGold())
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_YANG_NOT_ENOUGHT);
		return;
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	if (iWon > Owner->GetCheque())
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_WON_NOT_ENOUGHT);
		return;
	}
#endif

	if (CBanwordManager::Instance().CheckString(szName, strlen(szName)))
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_INVALID_NAME);
		return;
	}
	
	if (CBanwordManager::Instance().CheckString(szTitle, strlen(szTitle)))
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_WRONG_TITLE);
		return;
	}

	if (CBanwordManager::Instance().CheckString(szMessage, strlen(szMessage)))
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_WRONG_MESSAGE);
		return;
	}

	TMailBoxTable p;
	std::memcpy(p.szName, szName, sizeof(p.szName));
	p.bIsDeleted = false;

	p.AddData.bHeader = 0;
	p.AddData.Index = 0;
	std::memcpy(p.AddData.szFrom, Owner->GetName(), sizeof(p.AddData.szFrom));
	std::memcpy(p.AddData.szMessage, szMessage, sizeof(p.AddData.szMessage));
	p.AddData.iYang = iYang;
	p.AddData.iWon = iWon;
	p.AddData.ItemVnum = 0;
	p.AddData.ItemCount = 0;
	memset(p.AddData.alSockets, 0, sizeof(p.AddData.alSockets));
	memset(p.AddData.aAttr, 0, sizeof(p.AddData.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(p.AddData.aApplyRandom, 0, sizeof(p.AddData.aApplyRandom));
	memset(p.AddData.alRandomValues, 0, sizeof(p.AddData.alRandomValues));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
	p.AddData.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
	p.AddData.grade_element = 0;
	memset(p.AddData.attack_element, 0, sizeof(p.AddData.attack_element));
	p.AddData.element_type_bonus = 0;
	memset(p.AddData.elements_value_bonus, 0, sizeof(p.AddData.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
	p.AddData.set_value = 0;
#endif

	std::memcpy(p.Message.szTitle, szTitle, sizeof(p.Message.szTitle));
	p.Message.bIsGMPost = Owner->IsGM();
	p.Message.bIsConfirm = false;
	p.Message.SendTime = time(nullptr);
	p.Message.DeleteTime = p.Message.SendTime + (p.Message.bIsGMPost ? EMAILBOX::MAILBOX_REMAIN_DAY_GM : EMAILBOX::MAILBOX_REMAIN_DAY) * 60 * 60 * 24;

	if (pos.IsValidItemPosition())
	{
		if (pos.IsEquipPosition())
		{
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_WRONG_ITEM);
			return;
		}

		const LPITEM pItem = Owner->GetItem(pos);
		if (!pItem || pItem->IsExchanging() || pItem->isLocked())
		{
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_WRONG_ITEM);
			return;
		}

		p.AddData.ItemVnum = pItem->GetVnum();
		p.AddData.ItemCount = pItem->GetCount();
		std::memcpy(p.AddData.alSockets, pItem->GetSockets(), sizeof(p.AddData.alSockets));
		std::memcpy(p.AddData.aAttr, pItem->GetAttributes(), sizeof(p.AddData.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
		std::memcpy(p.AddData.aApplyRandom, pItem->GetApplysRandom(), sizeof(p.AddData.aApplyRandom));
		std::memcpy(p.AddData.alRandomValues, pItem->GetRandomDefaultAttrs(), sizeof(p.AddData.alRandomValues));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
		p.AddData.dwTransmutationVnum = pItem->GetChangeLookVnum();
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
		p.AddData.grade_element = pItem->GetElementGrade();
		std::memcpy(p.AddData.attack_element, pItem->GetElementAttacks(), sizeof(p.AddData.attack_element));
		p.AddData.element_type_bonus = pItem->GetElementsType();
		std::memcpy(p.AddData.elements_value_bonus, pItem->GetElementsValues(), sizeof(p.AddData.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
		p.AddData.set_value = pItem->GetItemSetValue();
#endif

		pItem->RemoveFromCharacter();
		ITEM_MANAGER::Instance().DestroyItem(pItem);
	}

	p.Message.bIsItemExist = p.AddData.ItemVnum > 0 || p.AddData.iYang > 0 || p.AddData.iWon > 0;

	Owner->PointChange(POINT_GOLD, -TotalYang);
#ifdef ENABLE_CHEQUE_SYSTEM
	Owner->PointChange(POINT_CHEQUE, -iWon);
#endif

	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_WRITE, d->GetHandle(), &p, sizeof(p));
	ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE, EMAILBOX_POST_WRITE::POST_WRITE_OK);
}

void CMailBox::WriteFromLua(const char* const szName, const char* const szTitle, const char* const szMessage, const char* const szFromName, uint32_t dwItem, uint8_t bCount)
{
	TMailBoxTable p;
	std::memcpy(p.szName, szName, sizeof(p.szName));
	p.bIsDeleted = false;

	// Message
	std::memcpy(p.Message.szTitle, szTitle, sizeof(p.Message.szTitle));
	p.Message.bIsGMPost = true;
	p.Message.bIsConfirm = false;
	p.Message.SendTime = time(nullptr);
	p.Message.DeleteTime = p.Message.SendTime + EMAILBOX::MAILBOX_REMAIN_REWARD * 60 * 60 * 24;

	// ItemData
	p.AddData.bHeader = 0;
	p.AddData.Index = 0;
	std::memcpy(p.AddData.szFrom, szFromName, sizeof(p.AddData.szFrom));
	std::memcpy(p.AddData.szMessage, szMessage, sizeof(p.AddData.szMessage));
	p.AddData.iYang = 0;
	p.AddData.iWon = 0;
	p.AddData.ItemVnum = dwItem;
	p.AddData.ItemCount = bCount;

	memset(p.AddData.alSockets, 0, sizeof(p.AddData.alSockets));
	memset(p.AddData.aAttr, 0, sizeof(p.AddData.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(p.AddData.aApplyRandom, 0, sizeof(p.AddData.aApplyRandom));
	memset(p.AddData.alRandomValues, 0, sizeof(p.AddData.alRandomValues));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	p.AddData.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	p.AddData.grade_element = 0;
	memset(p.AddData.attack_element, 0, sizeof(p.AddData.attack_element));
	p.AddData.element_type_bonus = 0;
	memset(p.AddData.elements_value_bonus, 0, sizeof(p.AddData.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
	p.AddData.set_value = 0;
#endif

	p.Message.bIsItemExist = p.AddData.ItemVnum > 0;

	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_WRITE, 0, &p, sizeof(p));
}

/*static*/ void CMailBox::Open(const LPCHARACTER ch)
{
	if (ch == nullptr)
		return;

	const LPDESC d = ch->GetDesc();
	if (d == nullptr)
		return;

	if (ch->GetLevel() < EMAILBOX::MAILBOX_LEVEL_LIMIT)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Its just allowed to use Mail with Lv. 20+.");
		return;
	}

	if (ch->GetMailBox() || ch->GetExchange() || ch->IsOpenSafebox() || 
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		ch->GetViewingShopOwner() ||
#else
		ch->GetShopOwner() || 
#endif
		ch->GetMyShop() || ch->IsCubeOpen()
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		|| ch->IsOpenGuildstorage()
#endif
#ifdef ENABLE_AURA_SYSTEM
		|| ch->IsAuraRefineWindowOpen()
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		|| ch->IsChangeLookWindowOpen()
#endif
#ifdef ENABLE_ATTR_6TH_7TH
		|| ch->IsOpenSkillBookComb()
#endif
	)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You have to close other windows.");
		return;
	}

	if (ch->IsMailBoxLoading())
		return;

	const int iPulse = thecore_pulse();
	if (iPulse - ch->GetMyMailBoxTime() < PASSES_PER_SEC(10))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You have to wait 10 seconds to open the mailbox again.");
		return;
	}

	ch->SetMailBoxLoading(true);

	/*Request*/
	TMailBox p;
	std::memcpy(p.szName, ch->GetName(), sizeof(p.szName));
	p.Index = 0;
	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_LOAD, d->GetHandle(), &p, sizeof(p));
}

/*static*/ void CMailBox::Create(const LPCHARACTER ch, const TMailBoxTable* pTable, const uint16_t Size)
{
	if (ch == nullptr)
		return;

	if (ch->GetMailBox())
		return;

	if (ch->IsMailBoxLoading() == false)
		return;
	
	ch->SetMailBoxLoading(false);
	ch->SetMailBox(M2_NEW CMailBox(ch, pTable, Size));
	ch->SetMyMailBoxTime();
}

/*static*/ void CMailBox::UnreadData(const LPCHARACTER ch)
{
	if (ch == nullptr)
		return;

	const LPDESC d = ch->GetDesc();
	if (d == nullptr)
		return;
	
	/*Request*/
	TMailBox p;
	std::memcpy(p.szName, ch->GetName(), sizeof(p.szName));
	p.Index = 0;
	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_UNREAD, d->GetHandle(), &p, sizeof(p));
}

/*static*/ void CMailBox::ResultUnreadData(const LPCHARACTER ch, TMailBoxRespondUnreadData* data)
{
	if (data == nullptr)
		return;
	
	if (ch == nullptr)
		return;

	const LPDESC d = ch->GetDesc();
	if (d == nullptr)
		return;

	data->bHeader = HEADER_GC_MAILBOX_UNREAD;
	d->Packet(&(*data), sizeof(TMailBoxRespondUnreadData));
}

void CMailBox::CheckPlayer(const char* const szName) const
{
	const LPDESC d = Owner->GetDesc();
	if (d == nullptr)
		return;

	if (Owner->IsMailBoxLoading())
		return;

	/*You can send mail to yourself at official game*/
	/*if (!strcasecmp(Owner->GetName(), szName))
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE_CONFIRM, POST_WRITE_FAIL);
		return;
	}*/

	Owner->SetMailBoxLoading(true);
	
	/*Request*/
	TMailBox p;
	std::memcpy(p.szName, szName, sizeof(p.szName));
	p.Index = 0;
	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_CHECK_NAME, d->GetHandle(), &p, sizeof(p));
}

void CMailBox::CheckPlayerResult(const TMailBox* t)
{
	if (Owner->IsMailBoxLoading() == false)
		return;

	Owner->SetMailBoxLoading(false);

	if (t == nullptr)
		return;
	
	uint8_t arg = EMAILBOX_POST_WRITE::POST_WRITE_INVALID_NAME;
	if (strlen(t->szName) != 0)
	{
		arg = EMAILBOX_POST_WRITE::POST_WRITE_OK;
		if (t->Index >= MAILBOX_MAX_MAIL)
			arg = EMAILBOX_POST_WRITE::POST_WRITE_FULL_MAILBOX;
	}

	ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_WRITE_CONFIRM, arg);
}

void CMailBox::AddData(const uint8_t button_idx, const uint8_t data_idx)
{
	const LPDESC d = Owner->GetDesc();
	if (d == nullptr)
		return;
	
	if (data_idx >= vecMailBox.size())
		return;

	TMailBoxTable& mail = vecMailBox.at(data_idx);
	if (mail.bIsDeleted)
		return;

	/*Notify DB*/
	TMailBox p;
	std::memcpy(p.szName, Owner->GetName(), sizeof(p.szName));
	p.Index = data_idx;
	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_CONFIRM, Owner->GetDesc()->GetHandle(), &p, sizeof(p));
	
	mail.AddData.bHeader = HEADER_GC_MAILBOX_ADD_DATA;
	mail.AddData.Index = data_idx;
	mail.Message.bIsConfirm = true;
	d->Packet(&mail.AddData, sizeof(mail.AddData));
	ServerProcess(EMAILBOX_GC::MAILBOX_GC_ADD_DATA, button_idx, data_idx);
}

void CMailBox::GetAllItems()
{
	const LPDESC d = Owner->GetDesc();
	if (d == nullptr)
		return;

	if (vecMailBox.empty())
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_ALL_GET_ITEMS, EMAILBOX_POST_ALL_GET_ITEMS::POST_ALL_GET_ITEMS_EMPTY);
		return;
	}

	TEMP_BUFFER buf;

	for (size_t i = 0; i < vecMailBox.size(); i++)
	{
		const bool bReceive = GetItem(static_cast<uint8_t>(i), true);
		if (bReceive == false)
			continue;

		TPacketGCMailboxProcessAll p{};
		p.Index = static_cast<uint8_t>(i);
		buf.write(&p, sizeof(p));
	}

	TPacketGCMailBox pack{};
	pack.bHeader = HEADER_GC_MAILBOX_ALL;
	pack.wSize = sizeof(pack) + buf.size();
	if (buf.size())
	{
		d->BufferedPacket(&pack, sizeof(pack));
		d->Packet(buf.read_peek(), buf.size());
	}
	else
		d->Packet(&pack, sizeof(pack));
}

bool CMailBox::GetItem(const uint8_t data_idx, const bool bAll)
{
	if (data_idx >= vecMailBox.size())
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_FAIL);
		return false;
	}

	TMailBoxTable& mail = vecMailBox.at(data_idx);
	if (mail.bIsDeleted)
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_NONE);
		return false;
	}

	if (mail.Message.bIsItemExist == false)
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_NONE);
		return false;
	}
	
	static_assert(EMAILBOX::MAILBOX_TAX >= 0 && EMAILBOX::MAILBOX_TAX <= 100, "EMAILBOX::MAILBOX_TAX must be: [0-100]");
	const int TotalYang = mail.AddData.iYang - mail.AddData.iYang * EMAILBOX::MAILBOX_TAX / 100;

	if (TotalYang + Owner->GetGold() >= GOLD_MAX)
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_YANG_OVERFLOW);
		return false;
	}

#ifdef ENABLE_CHEQUE_SYSTEM
	if (mail.AddData.iWon + Owner->GetCheque() >= CHEQUE_MAX)
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_WON_OVERFLOW);
		return false;
	}
#endif
	
	if (mail.AddData.ItemVnum > 0)
	{
		const LPITEM item = ITEM_MANAGER::Instance().CreateItem(mail.AddData.ItemVnum, mail.AddData.ItemCount);
		if (item == nullptr)
			return false;

#ifdef ENABLE_SPECIAL_INVENTORY
		const int pos = item->IsDragonSoul() ? Owner->GetEmptyDragonSoulInventory(item) : Owner->GetEmptyInventory(item);
#else
		const int pos = item->IsDragonSoul() ? Owner->GetEmptyDragonSoulInventory(item) : Owner->GetEmptyInventory(item->GetSize());
#endif

		if (-1 == pos)
		{
			if (bAll == false)
				ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_NOT_ENOUGHT_INVENTORY);
			ITEM_MANAGER::Instance().DestroyItem(item);
			return false;
		}

		item->SetSockets(mail.AddData.alSockets);
		item->SetAttributes(mail.AddData.aAttr);
#ifdef ENABLE_YOHARA_SYSTEM
		item->SetRandomAttrs(mail.AddData.aApplyRandom);
		item->SetRandomDefaultAttrs(mail.AddData.alRandomValues);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
		item->SetChangeLookVnum(mail.AddData.dwTransmutationVnum);
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
		item->SetElement(mail.AddData.grade_element, mail.AddData.attack_element, mail.AddData.element_type_bonus, mail.AddData.elements_value_bonus);
#endif
#ifdef ENABLE_SET_ITEM
		item->SetItemSetValue(mail.AddData.set_value);
#endif
		Owner->AutoGiveItem(item);
	}

	Owner->GiveGold(TotalYang);

#ifdef ENABLE_CHEQUE_SYSTEM
	Owner->GiveCheque(mail.AddData.iWon);
#endif

	mail.AddData.iYang = 0;
	mail.AddData.iWon = 0;
	mail.AddData.ItemVnum = 0;
	mail.AddData.ItemCount = 0;
	memset(mail.AddData.alSockets, 0, sizeof(mail.AddData.alSockets));
	memset(mail.AddData.aAttr, 0, sizeof(mail.AddData.aAttr));
#ifdef ENABLE_YOHARA_SYSTEM
	memset(mail.AddData.aApplyRandom, 0, sizeof(mail.AddData.aApplyRandom));
	memset(mail.AddData.alRandomValues, 0, sizeof(mail.AddData.alRandomValues));
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //OFFLINE_SHOP_CHANGELOOK
	mail.AddData.dwTransmutationVnum = 0;
#endif
#ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
	mail.AddData.grade_element = 0;
	memset(mail.AddData.attack_element, 0, sizeof(mail.AddData.attack_element));
	mail.AddData.element_type_bonus = 0;
	memset(mail.AddData.elements_value_bonus, 0, sizeof(mail.AddData.elements_value_bonus));
#endif
#ifdef ENABLE_SET_ITEM
	mail.AddData.set_value = 0;
#endif
	mail.Message.bIsItemExist = false;

	/*Notify DB*/
	TMailBox p;
	std::memcpy(p.szName, Owner->GetName(), sizeof(p.szName));
	p.Index = data_idx;
	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_GET, Owner->GetDesc()->GetHandle(), &p, sizeof(p));

	if (bAll == false)
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS, data_idx, EMAILBOX_POST_GET_ITEMS::POST_GET_ITEMS_OK);

	return true;
}

void CMailBox::DeleteAllMails()
{
	const LPDESC d = Owner->GetDesc();
	if (d == nullptr)
		return;

	if (vecMailBox.empty())
	{
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_ALL_DELETE, EMAILBOX_POST_ALL_DELETE::POST_ALL_DELETE_FAIL_EMPTY);
		return;
	}

	TEMP_BUFFER buf;

	bool bIsFail = false;
	for (size_t i = 0; i < vecMailBox.size(); i++)
		if (DeleteMail(static_cast<uint8_t>(i), true) == false)
			bIsFail = true;

	if (bIsFail == false)
		ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_ALL_DELETE, EMAILBOX_POST_ALL_DELETE::POST_ALL_DELETE_OK);
}

bool CMailBox::DeleteMail(const uint8_t data_idx, const bool bAll)
{
	if (data_idx >= vecMailBox.size())
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_DELETE, data_idx, EMAILBOX_POST_DELETE_FAIL::POST_DELETE_FAIL);
		return false;
	}

	TMailBoxTable& mail = vecMailBox.at(data_idx);
	if (mail.bIsDeleted)
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_DELETE, data_idx, EMAILBOX_POST_DELETE_FAIL::POST_DELETE_FAIL);
		return true;
	}
	
	if (mail.Message.bIsItemExist)
	{
		if (bAll == false)
			ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_DELETE, data_idx, EMAILBOX_POST_DELETE_FAIL::POST_DELETE_FAIL_EXIST_ITEMS);
		return false;
	}

	/*Notify DB*/
	TMailBox p;
	std::memcpy(p.szName, Owner->GetName(), sizeof(p.szName));
	p.Index = data_idx;
	db_clientdesc->DBPacket(HEADER_GD_MAILBOX_DELETE, Owner->GetDesc()->GetHandle(), &p, sizeof(p));

	mail.bIsDeleted = true;

	ServerProcess(EMAILBOX_GC::MAILBOX_GC_POST_DELETE, data_idx, EMAILBOX_POST_DELETE_FAIL::POST_DELETE_OK);
	return true;
}
#endif
