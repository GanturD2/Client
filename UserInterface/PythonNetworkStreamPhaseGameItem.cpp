#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonSafeBox.h"
#include "PythonCharacterManager.h"
#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined(ENABLE_AURA_SYSTEM) || defined(ENABLE_GEM_SYSTEM)
#	include "PythonPlayer.h"
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
#	include "PythonGuildBank.h"
#endif
#include "AbstractPlayer.h"

//////////////////////////////////////////////////////////////////////////
// SafeBox

#ifdef ENABLE_SAFEBOX_MONEY
bool CPythonNetworkStream::SendSafeBoxMoneyPacket(uint8_t byState, uint32_t dwMoney)
{
	TPacketCGSafeboxMoney kSafeboxMoney;
	kSafeboxMoney.bHeader = HEADER_CG_SAFEBOX_MONEY;
	kSafeboxMoney.bState = byState;
	kSafeboxMoney.dwMoney = dwMoney;
	if (!Send(sizeof(kSafeboxMoney), &kSafeboxMoney))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos, uint8_t bySafeBoxPos)
{
	__PlayInventoryItemDropSound(InventoryPos);

	TPacketCGSafeboxCheckin kSafeboxCheckin;
	kSafeboxCheckin.bHeader = HEADER_CG_SAFEBOX_CHECKIN;
	kSafeboxCheckin.ItemPos = InventoryPos;
	kSafeboxCheckin.bSafePos = bySafeBoxPos;
	if (!Send(sizeof(kSafeboxCheckin), &kSafeboxCheckin))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(uint8_t bySafeBoxPos, TItemPos InventoryPos)
{
	__PlaySafeBoxItemDropSound(bySafeBoxPos);

	TPacketCGSafeboxCheckout kSafeboxCheckout;
	kSafeboxCheckout.bHeader = HEADER_CG_SAFEBOX_CHECKOUT;
	kSafeboxCheckout.bSafePos = bySafeBoxPos;
	kSafeboxCheckout.ItemPos = InventoryPos;
	if (!Send(sizeof(kSafeboxCheckout), &kSafeboxCheckout))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxItemMovePacket(uint8_t bySourcePos, uint8_t byTargetPos, uint8_t byCount)
{
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	if(byCount != 2)
		__PlaySafeBoxItemDropSound(bySourcePos);
#else
	__PlaySafeBoxItemDropSound(bySourcePos);
#endif

	TPacketCGItemMove kItemMove;
	kItemMove.header = HEADER_CG_SAFEBOX_ITEM_MOVE;
	kItemMove.pos = TItemPos(INVENTORY, bySourcePos);
	kItemMove.num = byCount;
	kItemMove.change_pos = TItemPos(INVENTORY, byTargetPos);
	if (!Send(sizeof(kItemMove), &kItemMove))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;
	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.grade_element = kItemSet.grade_element;
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
		kItemData.attack_element[i] = kItemSet.attack_element[i];
		kItemData.elements_value_bonus[i] = kItemSet.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = kItemSet.element_type_bonus;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	for (int iSungAttr = 0; iSungAttr < APPLY_RANDOM_SLOT_MAX_NUM; ++iSungAttr)
		kItemData.aApplyRandom[iSungAttr] = kItemSet.aApplyRandom[iSungAttr];
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		kItemData.alRandomValues[rValue] = kItemSet.alRandomValues[rValue];
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.nSealDate = kItemSet.nSealDate;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	kItemData.dwTransmutationVnum = kItemSet.dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	kItemData.is_basic = false;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = kItemSet.set_value;
#endif

	CPythonSafeBox::Instance().SetItemData(kItemSet.Cell.cell, kItemData);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelItemData(kItemDel.pos);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket()
{
	TPacketGCSafeboxWrongPassword kSafeboxWrongPassword;

	if (!Recv(sizeof(kSafeboxWrongPassword), &kSafeboxWrongPassword))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket()
{
	TPacketGCSafeboxSize kSafeBoxSize;
	if (!Recv(sizeof(kSafeBoxSize), &kSafeBoxSize))
		return false;

	CPythonSafeBox::Instance().OpenSafeBox(kSafeBoxSize.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow", Py_BuildValue("(i)", kSafeBoxSize.bSize));

	return true;
}

#ifdef ENABLE_SAFEBOX_MONEY
bool CPythonNetworkStream::RecvSafeBoxMoneyChangePacket()
{
	TPacketGCSafeboxMoneyChange kMoneyChange;
	if (!Recv(sizeof(kMoneyChange), &kMoneyChange))
		return false;

	CPythonSafeBox::Instance().SetMoney(kMoneyChange.dwMoney);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafeboxMoney", Py_BuildValue("()"));

	return true;
}
#endif

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(uint8_t byMallPos, TItemPos InventoryPos)
{
	__PlayMallItemDropSound(byMallPos);

	TPacketCGMallCheckout kMallCheckoutPacket;
	kMallCheckoutPacket.bHeader = HEADER_CG_MALL_CHECKOUT;
	kMallCheckoutPacket.bMallPos = byMallPos;
	kMallCheckoutPacket.ItemPos = InventoryPos;
	if (!Send(sizeof(kMallCheckoutPacket), &kMallCheckoutPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMallOpenPacket()
{
	TPacketGCMallOpen kMallOpen;
	if (!Recv(sizeof(kMallOpen), &kMallOpen))
		return false;

	CPythonSafeBox::Instance().OpenMall(kMallOpen.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow", Py_BuildValue("(i)", kMallOpen.bSize));

	return true;
}

bool CPythonNetworkStream::RecvMallItemSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;
	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.grade_element = kItemSet.grade_element;
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
		kItemData.attack_element[i] = kItemSet.attack_element[i];
		kItemData.elements_value_bonus[i] = kItemSet.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = kItemSet.element_type_bonus;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	for (int iSungAttr = 0; iSungAttr < APPLY_RANDOM_SLOT_MAX_NUM; ++iSungAttr)
		kItemData.aApplyRandom[iSungAttr] = kItemSet.aApplyRandom[iSungAttr];
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		kItemData.alRandomValues[rValue] = kItemSet.alRandomValues[rValue];
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.nSealDate = kItemSet.nSealDate;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	kItemData.dwTransmutationVnum = kItemSet.dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	kItemData.is_basic = false;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = kItemSet.set_value;
#endif

	CPythonSafeBox::Instance().SetMallItemData(kItemSet.Cell.cell, kItemData);

	__RefreshMallWindow();

	return true;
}

bool CPythonNetworkStream::RecvMallItemDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelMallItemData(kItemDel.pos);

	__RefreshMallWindow();
	Tracef(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

	return true;
}
// Mall
//////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
bool CPythonNetworkStream::SendGuildstorageCheckinPacket(TItemPos InventoryPos, uint8_t byGuildstoragePos)
{
	__PlayInventoryItemDropSound(InventoryPos);

	TPacketCGGuildstorageCheckin kGuildstorageCheckin;
	kGuildstorageCheckin.bHeader = HEADER_CG_GUILDSTORAGE_CHECKIN;
	kGuildstorageCheckin.ItemPos = InventoryPos;
	kGuildstorageCheckin.bSafePos = byGuildstoragePos;
	if (!Send(sizeof(kGuildstorageCheckin), &kGuildstorageCheckin))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendGuildstorageCheckoutPacket(uint8_t byGuildstoragePos, TItemPos InventoryPos)
{
	__PlayGuildstorageItemDropSound(byGuildstoragePos);

	TPacketCGGuildstorageCheckout kGuildstorageCheckoutPacket;
	kGuildstorageCheckoutPacket.bHeader = HEADER_CG_GUILDSTORAGE_CHECKOUT;
	kGuildstorageCheckoutPacket.bGuildstoragePos = byGuildstoragePos;
	kGuildstorageCheckoutPacket.ItemPos = InventoryPos;
	if (!Send(sizeof(kGuildstorageCheckoutPacket), &kGuildstorageCheckoutPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvGuildstorageOpenPacket()
{
	TPacketGCGuildstorageOpen kGuildstorageOpen;
	if (!Recv(sizeof(kGuildstorageOpen), &kGuildstorageOpen))
		return false;

	CPythonGuildBank::Instance().OpenGuildBank(kGuildstorageOpen.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenGuildstorageWindow", Py_BuildValue("(i)", kGuildstorageOpen.bSize));

	return true;
}

bool CPythonNetworkStream::RecvGuildstorageItemSetPacket()
{
	TPacketGCItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;

#ifdef ENABLE_REFINE_ELEMENT //GUILDBANK_ELEMENT
	kItemData.grade_element = kItemSet.grade_element;
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
		kItemData.attack_element[i] = kItemSet.attack_element[i];
		kItemData.elements_value_bonus[i] = kItemSet.elements_value_bonus[i];
	}

	kItemData.element_type_bonus = kItemSet.element_type_bonus;
#endif

	for (int isocket = 0; isocket < ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr = 0; iattr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_YOHARA_SYSTEM
	for (int iSungAttr = 0; iSungAttr < APPLY_RANDOM_SLOT_MAX_NUM; ++iSungAttr)
		kItemData.aApplyRandom[iSungAttr] = kItemSet.aApplyRandom[iSungAttr];
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		kItemData.alRandomValues[rValue] = kItemSet.alRandomValues[rValue];
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.nSealDate = kItemSet.nSealDate;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //ENABLE_GUILDBANK_CHANGELOOK
	kItemData.dwTransmutationVnum = kItemSet.dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	kItemData.is_basic = false;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = kItemSet.set_value;
#endif

	CPythonGuildBank::Instance().SetGuildBankItemData(kItemSet.Cell.cell, kItemData);

	__RefreshGuildstorageWindow();

	return true;
}

bool CPythonNetworkStream::RecvGuildstorageItemDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonGuildBank::Instance().DelGuildBankItemData(kItemDel.pos);

	__RefreshGuildstorageWindow();
	Tracef(" >> CPythonNetworkStream::RecvGuildstorageItemDelPacket\n");

	return true;
}
#endif

// Item
// Recieve
bool CPythonNetworkStream::RecvItemDelPacket()
{
	TPacketGCItemDelDeprecated packet_item_del;

	if (!Recv(sizeof(TPacketGCItemDelDeprecated), &packet_item_del))
		return false;

	TItemData kItemData;
	kItemData.vnum = packet_item_del.vnum;
	kItemData.count = packet_item_del.count;
	kItemData.flags = 0;
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.nSealDate = packet_item_del.nSealDate;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	kItemData.dwTransmutationVnum = packet_item_del.dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	kItemData.is_basic = packet_item_del.is_basic;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.grade_element = packet_item_del.grade_element;
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
		kItemData.attack_element[i] = packet_item_del.attack_element[i];
		kItemData.elements_value_bonus[i] = packet_item_del.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = packet_item_del.element_type_bonus;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = packet_item_del.set_value;
#endif
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i] = packet_item_del.alSockets[i];
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j] = packet_item_del.aAttr[j];

#ifdef ENABLE_YOHARA_SYSTEM
	for (int k = 0; k < APPLY_RANDOM_SLOT_MAX_NUM; ++k)
		kItemData.aApplyRandom[k] = packet_item_del.aApplyRandom[k];
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		kItemData.alRandomValues[rValue] = packet_item_del.alRandomValues[rValue];
#endif

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_del.Cell, kItemData);

#ifdef ENABLE_SWITCHBOT
	if (packet_item_del.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemSetPacket()
{
	TPacketGCItemSet packet_item_set;

	if (!Recv(sizeof(TPacketGCItemSet), &packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum = packet_item_set.vnum;
	kItemData.count = packet_item_set.count;
	kItemData.flags = packet_item_set.flags;
	kItemData.anti_flags = packet_item_set.anti_flags;
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.nSealDate = packet_item_set.nSealDate;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	kItemData.dwTransmutationVnum = packet_item_set.dwTransmutationVnum;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	kItemData.is_basic = packet_item_set.is_basic;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.grade_element = packet_item_set.grade_element;
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
		kItemData.attack_element[i] = packet_item_set.attack_element[i];
		kItemData.elements_value_bonus[i] = packet_item_set.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = packet_item_set.element_type_bonus;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = packet_item_set.set_value;
#endif
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i] = packet_item_set.alSockets[i];
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j] = packet_item_set.aAttr[j];

#ifdef ENABLE_YOHARA_SYSTEM
	for (int k = 0; k < APPLY_RANDOM_SLOT_MAX_NUM; ++k)
		kItemData.aApplyRandom[k] = packet_item_set.aApplyRandom[k];
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		kItemData.alRandomValues[rValue] = packet_item_set.alRandomValues[rValue];
#endif

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);

#ifdef ENABLE_SWITCHBOT
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif

	if (packet_item_set.highlight)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item",
							  Py_BuildValue("(ii)", packet_item_set.Cell.window_type, packet_item_set.Cell.cell));

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUsePacket()
{
	TPacketGCItemUse packet_item_use;

	if (!Recv(sizeof(TPacketGCItemUse), &packet_item_use))
		return false;

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUpdatePacket()
{
	TPacketGCItemUpdate packet_item_update;

	if (!Recv(sizeof(TPacketGCItemUpdate), &packet_item_update))
		return false;

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count);
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		rkPlayer.SetItemMetinSocket(packet_item_update.Cell, i, packet_item_update.alSockets[i]);
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		rkPlayer.SetItemAttribute(packet_item_update.Cell, j, packet_item_update.aAttr[j].wType, packet_item_update.aAttr[j].sValue);
#ifdef ENABLE_YOHARA_SYSTEM
	for (int j = 0; j < APPLY_RANDOM_SLOT_MAX_NUM; ++j)
		rkPlayer.SetSungmaAttribute(packet_item_update.Cell, j, packet_item_update.aApplyRandom[j].wType, packet_item_update.aApplyRandom[j].sValue);
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		rkPlayer.SetRandomDefaultAttrs(packet_item_update.Cell, rValue, packet_item_update.alRandomValues[rValue]);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	rkPlayer.SetItemTransmutationVnum(packet_item_update.Cell, packet_item_update.dwTransmutationVnum);
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
	rkPlayer.SetBasicItem(packet_item_update.Cell, packet_item_update.is_basic);
#endif
#ifdef ENABLE_REFINE_ELEMENT
	rkPlayer.SetElement(packet_item_update.Cell, packet_item_update.grade_element, packet_item_update.element_type_bonus);
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i) {
		rkPlayer.SetElementAttack(packet_item_update.Cell, i, packet_item_update.attack_element[i]);
		rkPlayer.SetElementValue(packet_item_update.Cell, i, packet_item_update.elements_value_bonus[i]);
	}
#endif
#ifdef ENABLE_SET_ITEM
	rkPlayer.SetItemSetValue(packet_item_update.Cell, packet_item_update.set_value);
#endif

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket()
{
	TPacketGCItemGroundAdd recv;

	if (!Recv(sizeof(TPacketGCItemGroundAdd), &recv))
		return false;

	__GlobalPositionToLocalPosition(recv.lX, recv.lY);

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	CPythonItem::Instance().CreateItem(recv.dwVID, recv.dwVnum, recv.lX, recv.lY, recv.lZ, true, recv.alSockets, recv.aAttrs);
#else
	CPythonItem::Instance().CreateItem(recv.dwVID, recv.dwVnum, recv.lX, recv.lY, recv.lZ);
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemOwnership()
{
	TPacketGCItemOwnership p;

	if (!Recv(sizeof(TPacketGCItemOwnership), &p))
		return false;

	CPythonItem::Instance().SetOwnership(p.dwVID, p.szName);
	return true;
}

bool CPythonNetworkStream::RecvItemGroundDelPacket()
{
	TPacketGCItemGroundDel packet_item_ground_del;

	if (!Recv(sizeof(TPacketGCItemGroundDel), &packet_item_ground_del))
		return false;

	CPythonItem::Instance().DeleteItem(packet_item_ground_del.vid);
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket()
{
	TPacketGCQuickSlotAdd packet_quick_slot_add;

	if (!Recv(sizeof(TPacketGCQuickSlotAdd), &packet_quick_slot_add))
		return false;

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.AddQuickSlot(packet_quick_slot_add.pos, packet_quick_slot_add.slot.Type, packet_quick_slot_add.slot.Position);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket()
{
	TPacketGCQuickSlotDel packet_quick_slot_del;

	if (!Recv(sizeof(TPacketGCQuickSlotDel), &packet_quick_slot_del))
		return false;

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.DeleteQuickSlot(packet_quick_slot_del.pos);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket()
{
	TPacketGCQuickSlotSwap packet_quick_slot_swap;

	if (!Recv(sizeof(TPacketGCQuickSlotSwap), &packet_quick_slot_swap))
		return false;

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.MoveQuickSlot(packet_quick_slot_swap.pos, packet_quick_slot_swap.change_pos);

	__RefreshInventoryWindow();

	return true;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
bool CPythonNetworkStream::RecvShopItemData()
{
	TPacketPlayerShopSet shop_item_set;

	if (!Recv(sizeof(TPacketPlayerShopSet), &shop_item_set))
		return false;

	TShopItemData kShopItemData;
	kShopItemData.vnum = shop_item_set.vnum;
	kShopItemData.count = shop_item_set.count;
#	ifdef ENABLE_CHEQUE_SYSTEM
	kShopItemData.price.dwPrice = shop_item_set.price;
	kShopItemData.price.dwCheque = shop_item_set.cheque;
#	else
	kShopItemData.price = shop_item_set.price;
#	endif
#	ifdef ENABLE_CHANGE_LOOK_SYSTEM
	kShopItemData.dwTransmutationVnum = shop_item_set.dwLookVnum;
#	endif
#	ifdef ENABLE_REFINE_ELEMENT //OFFSHOP_ELEMENT
	kShopItemData.grade_element = shop_item_set.grade_element;
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i)
	{
		kShopItemData.attack_element[i] = shop_item_set.attack_element[i];
		kShopItemData.elements_value_bonus[i] = shop_item_set.elements_value_bonus[i];
	}

	kShopItemData.element_type_bonus = shop_item_set.element_type_bonus;
#	endif
#	ifdef ENABLE_SET_ITEM
	kShopItemData.set_value = shop_item_set.set_value;
#	endif

	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kShopItemData.alSockets[i] = shop_item_set.alSockets[i];
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kShopItemData.aAttr[j] = shop_item_set.aAttr[j];
#	ifdef ENABLE_YOHARA_SYSTEM
	for (int k = 0; k < APPLY_RANDOM_SLOT_MAX_NUM; ++k)
		kShopItemData.aApplyRandom[k] = shop_item_set.aApplyRandom[k];
	for (int rValue = 0; rValue < ITEM_RANDOM_VALUES_MAX_NUM; ++rValue)
		kShopItemData.alRandomValues[rValue] = shop_item_set.alRandomValues[rValue];
#	endif

	CPythonShop& rkShop = CPythonShop::Instance();
	rkShop.SetMyShopItemData(shop_item_set.pos, kShopItemData);
	__RefreshShopInfoWindow();

	return true;
}
#endif

bool CPythonNetworkStream::SendShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

#ifdef ENABLE_SPECIAL_INVENTORY
	TPacketCGShop packet_shop = {};
#else
	TPacketCGShop packet_shop;
#endif
	packet_shop.header = HEADER_CG_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(sizeof(packet_shop), &packet_shop))
	{
		Tracef("SendShopEndPacket Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_SPECIAL_INVENTORY
bool CPythonNetworkStream::SendShopBuyPacket(uint16_t bPos)
#else
bool CPythonNetworkStream::SendShopBuyPacket(uint8_t bPos)
#endif
{
	if (!__CanActMainInstance())
		return true;

#ifdef ENABLE_SPECIAL_INVENTORY
	TPacketCGShop PacketShop = {};
#else
	TPacketCGShop PacketShop;
#endif
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;
#ifdef ENABLE_SPECIAL_INVENTORY
	PacketShop.wPos = bPos;
	PacketShop.bCount = 1;
#endif

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

#ifndef ENABLE_SPECIAL_INVENTORY
	uint8_t bCount = 1;
	if (!Send(sizeof(uint8_t), &bCount))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	if (!Send(sizeof(uint8_t), &bPos))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}
#endif

	return SendSequence();
}

#ifdef ENABLE_SPECIAL_INVENTORY
bool CPythonNetworkStream::SendShopSellPacket(uint16_t bySlot)
#else
bool CPythonNetworkStream::SendShopSellPacket(uint8_t bySlot)
#endif
{
	if (!__CanActMainInstance())
		return true;

#ifdef ENABLE_SPECIAL_INVENTORY
	TPacketCGShop PacketShop = {};
#else
	TPacketCGShop PacketShop;
#endif
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;
#ifdef ENABLE_SPECIAL_INVENTORY
	PacketShop.wPos = bySlot;
#endif

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
#ifndef ENABLE_SPECIAL_INVENTORY
	if (!Send(sizeof(uint8_t), &bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
#endif

	return SendSequence();
}

#ifdef ENABLE_SPECIAL_INVENTORY
bool CPythonNetworkStream::SendShopSellPacketNew(uint16_t wSlot, uint8_t byCount)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(uint8_t wSlot, uint8_t byCount)
#endif
{
	if (!__CanActMainInstance())
		return true;

#ifdef ENABLE_SPECIAL_INVENTORY
	TPacketCGShop PacketShop = {};
#else
	TPacketCGShop PacketShop;
#endif
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL2;
#ifdef ENABLE_SPECIAL_INVENTORY
	PacketShop.wPos = wSlot;
	PacketShop.bCount = byCount;
#endif

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
#ifndef ENABLE_SPECIAL_INVENTORY
	if (!Send(sizeof(uint8_t), &bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(uint8_t), &byCount))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
#endif

	Tracef(" SendShopSellPacketNewDrop(wSlot=%d, byCount=%d)\n", wSlot, byCount);

	return SendSequence();
}

// Send
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos)
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage",
								  Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
			return true;
		}

		if (CPythonShop::Instance().IsOpen())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage",
								  Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
			return true;
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemUseSound(pos);

	TPacketCGItemUse itemUsePacket;
	itemUsePacket.header = HEADER_CG_ITEM_USE;
	itemUsePacket.pos = pos;

	if (!Send(sizeof(TPacketCGItemUse), &itemUsePacket))
	{
		Tracen("SendItemUsePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemUseToItem itemUseToItemPacket;
	itemUseToItemPacket.header = HEADER_CG_ITEM_USE_TO_ITEM;
	itemUseToItemPacket.source_pos = source_pos;
	itemUseToItemPacket.target_pos = target_pos;

	if (!Send(sizeof(TPacketCGItemUseToItem), &itemUseToItemPacket))
	{
		Tracen("SendItemUseToItemPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemUseToItemPacket(src=%d, dst=%d)\n", source_pos, target_pos);
#endif

	return SendSequence();
}

bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, uint32_t elk)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP;
	itemDropPacket.pos = pos;
	itemDropPacket.elk = elk;

	if (!Send(sizeof(TPacketCGItemDrop), &itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_DESTROY_SYSTEM
bool CPythonNetworkStream::SendItemDestroyPacket(TItemPos pos, uint32_t elk, uint32_t count)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemRemove itemDestroyPacket;
	itemDestroyPacket.header = HEADER_CG_ITEM_DESTROY;
	itemDestroyPacket.pos = pos;
	itemDestroyPacket.gold = elk;
	itemDestroyPacket.count = count;

	if (!Send(sizeof(itemDestroyPacket), &itemDestroyPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return true;
}
#endif

bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, uint32_t elk, uint32_t count)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop2 itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP2;
	itemDropPacket.pos = pos;
	itemDropPacket.gold = elk;
	itemDropPacket.count = count;

	if (!Send(sizeof(itemDropPacket), &itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos) const
{
	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos) const
{
	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	uint32_t dwItemID = rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem & rkItem = CPythonItem::Instance();
	rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos) const
{
	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	uint32_t dwItemID = rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem & rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

//void CPythonNetworkStream::__PlayShopItemDropSound(uint32_t uSlotPos)
//{
//	uint32_t dwItemID;
//	CPythonShop& rkShop=CPythonShop::Instance();
//	if (!rkShop.GetSlotItemID(uSlotPos, &dwItemID))
//		return;
//
//	CPythonItem& rkItem=CPythonItem::Instance();
//	rkItem.PlayDropSound(dwItemID);
//}

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(uint32_t uSlotPos) const
{
	uint32_t dwItemID;
	CPythonSafeBox & rkSafeBox = CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem & rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(uint32_t uSlotPos) const
{
	uint32_t dwItemID;
	CPythonSafeBox & rkSafeBox = CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem & rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
void CPythonNetworkStream::__PlayGuildstorageItemDropSound(uint32_t uSlotPos)
{
	uint32_t dwItemID;
	CPythonGuildBank& rkSafeBox = CPythonGuildBank::Instance();
	if (!rkSafeBox.GetSlotGuildBankItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem = CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}
#endif

bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, uint8_t num)
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage",
									  Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
				return true;
			}
		}

		if (CPythonShop::Instance().IsOpen())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage",
									  Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
				return true;
			}
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemDropSound(pos);

	TPacketCGItemMove itemMovePacket;
	itemMovePacket.header = HEADER_CG_ITEM_MOVE;
	itemMovePacket.pos = pos;
	itemMovePacket.change_pos = change_pos;
	itemMovePacket.num = num;

	if (!Send(sizeof(TPacketCGItemMove), &itemMovePacket))
	{
		Tracen("SendItemMovePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemPickUpPacket(uint32_t vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemPickUp itemPickUpPacket;
	itemPickUpPacket.header = HEADER_CG_ITEM_PICKUP;
	itemPickUpPacket.vid = vid;

	if (!Send(sizeof(TPacketCGItemPickUp), &itemPickUpPacket))
	{
		Tracen("SendItemPickUpPacket Error");
		return false;
	}

	return SendSequence();
}


bool CPythonNetworkStream::SendQuickSlotAddPacket(uint8_t wpos, uint8_t type, uint8_t pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotAdd quickSlotAddPacket;

	quickSlotAddPacket.header = HEADER_CG_QUICKSLOT_ADD;
	quickSlotAddPacket.pos = wpos;
	quickSlotAddPacket.slot.Type = type;
	quickSlotAddPacket.slot.Position = pos;

	if (!Send(sizeof(TPacketCGQuickSlotAdd), &quickSlotAddPacket))
	{
		Tracen("SendQuickSlotAddPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(uint8_t pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotDel quickSlotDelPacket;

	quickSlotDelPacket.header = HEADER_CG_QUICKSLOT_DEL;
	quickSlotDelPacket.pos = pos;

	if (!Send(sizeof(TPacketCGQuickSlotDel), &quickSlotDelPacket))
	{
		Tracen("SendQuickSlotDelPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(uint8_t pos, uint8_t change_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotSwap quickSlotSwapPacket;

	quickSlotSwapPacket.header = HEADER_CG_QUICKSLOT_SWAP;
	quickSlotSwapPacket.pos = pos;
	quickSlotSwapPacket.change_pos = change_pos;

	if (!Send(sizeof(TPacketCGQuickSlotSwap), &quickSlotSwapPacket))
	{
		Tracen("SendQuickSlotSwapPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvSpecialEffect()
{
	TPacketGCSpecialEffect kSpecialEffect;
	if (!Recv(sizeof(kSpecialEffect), &kSpecialEffect))
		return false;

	uint32_t effect = -1;
	bool bPlayPotionSound = false;
	bool bAttachEffect = true;
	switch (kSpecialEffect.type)
	{
		case SE_HPUP_RED:
			effect = CInstanceBase::EFFECT_HPUP_RED;
			bPlayPotionSound = true;
			break;
		case SE_SPUP_BLUE:
			effect = CInstanceBase::EFFECT_SPUP_BLUE;
			bPlayPotionSound = true;
			break;
		case SE_SPEEDUP_GREEN:
			effect = CInstanceBase::EFFECT_SPEEDUP_GREEN;
			bPlayPotionSound = true;
			break;
		case SE_DXUP_PURPLE:
			effect = CInstanceBase::EFFECT_DXUP_PURPLE;
			bPlayPotionSound = true;
			break;
		case SE_CRITICAL:
			effect = CInstanceBase::EFFECT_CRITICAL;
			break;
		case SE_PENETRATE:
			effect = CInstanceBase::EFFECT_PENETRATE;
			break;
		case SE_BLOCK:
			effect = CInstanceBase::EFFECT_BLOCK;
			break;
		case SE_DODGE:
			effect = CInstanceBase::EFFECT_DODGE;
			break;
		case SE_CHINA_FIREWORK:
			effect = CInstanceBase::EFFECT_FIRECRACKER;
			bAttachEffect = false;
			break;
		case SE_SPIN_TOP:
			effect = CInstanceBase::EFFECT_SPIN_TOP;
			bAttachEffect = false;
			break;
		case SE_SUCCESS:
			effect = CInstanceBase::EFFECT_SUCCESS;
			bAttachEffect = false;
			break;
		case SE_FAIL:
			effect = CInstanceBase::EFFECT_FAIL;
			break;
		case SE_FR_SUCCESS:
			effect = CInstanceBase::EFFECT_FR_SUCCESS;
			bAttachEffect = false;
			break;
		case SE_LEVELUP_ON_14_FOR_GERMANY:
			effect = CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY;
			bAttachEffect = false;
			break;
		case SE_LEVELUP_UNDER_15_FOR_GERMANY:
			effect = CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY;
			bAttachEffect = false;
			break;
		case SE_PERCENT_DAMAGE1:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE1;
			break;
		case SE_PERCENT_DAMAGE2:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE2;
			break;
		case SE_PERCENT_DAMAGE3:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE3;
			break;
		case SE_AUTO_HPUP:
			effect = CInstanceBase::EFFECT_AUTO_HPUP;
			break;
		case SE_AUTO_SPUP:
			effect = CInstanceBase::EFFECT_AUTO_SPUP;
			break;
		case SE_EQUIP_RAMADAN_RING:
			effect = CInstanceBase::EFFECT_RAMADAN_RING_EQUIP;
			break;
		case SE_EQUIP_HALLOWEEN_CANDY:
			effect = CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP;
			break;
		case SE_EQUIP_HAPPINESS_RING:
			effect = CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP;
			break;
		case SE_EQUIP_LOVE_PENDANT:
			effect = CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP;
			break;
		case SE_EQUIP_BOOTS:
			effect = CInstanceBase::EFFECT_EQUIP_BOOTS;
			break;
		case SE_EQUIP_HEROMEDAL:
			effect = CInstanceBase::EFFECT_EQUIP_HEROMEDAL;
			break;
		case SE_EQUIP_CHOCOLATE_AMULET:
			effect = CInstanceBase::EFFECT_EQUIP_CHOCOLATE_AMULET;
			break;
		case SE_EQUIP_EMOTION_MASK:
			effect = CInstanceBase::EFFECT_EQUIP_EMOTION_MASK;
			break;
		case SE_EQUIP_WILLPOWER_RING:
			effect = CInstanceBase::EFFECT_WILLPOWER_RING;
			break;
		case SE_EQUIP_DEADLYPOWER_RING:
			effect = CInstanceBase::EFFECT_DEADLYPOWER_RING;
			break;
		case SE_EQUIP_EASTER_CANDY_EQIP:
			effect = CInstanceBase::EFFECT_EASTER_CANDY_EQIP;
			break;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		case SE_ACCE_SUCESS_ABSORB:
			effect = CInstanceBase::EFFECT_ACCE_SUCESS_ABSORB;
			break;
		case SE_ACCE_EQUIP:
			effect = CInstanceBase::EFFECT_ACCE_EQUIP;
			break;
		case SE_ACCE_BACK:
			effect = CInstanceBase::EFFECT_ACCE_BACK;
			break;
#endif
#ifdef ENABLE_DAWNMIST_DUNGEON
		case SE_HEAL:
			effect = CInstanceBase::EFFECT_HEAL;
			break;
#endif
#ifdef ENABLE_ANTI_EXP_RING
		case SE_AUTO_EXP:
			effect = CInstanceBase::EFFECT_AUTO_EXP;
			break;
#endif
		case SE_CAPE_OF_COURAGE_EFFECT:	//@fixme421
			effect = CInstanceBase::EFFECT_CAPE_OF_COURAGE;
			break;

#ifdef ENABLE_12ZI
		case SE_SKILL_DAMAGE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;
		case SE_SKILL_SAFE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;
		case SE_METEOR:
			effect = CInstanceBase::EFFECT_METEOR;
			break;
		case SE_BEAD_RAIN:
			effect = CInstanceBase::EFFECT_BEAD_RAIN;
			break;
		case SE_FALL_ROCK:
			effect = CInstanceBase::EFFECT_FALL_ROCK;
			break;
		case SE_ARROW_RAIN:
			effect = CInstanceBase::EFFECT_ARROW_RAIN;
			break;
		case SE_HORSE_DROP:
			effect = CInstanceBase::EFFECT_HORSE_DROP;
			break;
		case SE_EGG_DROP:
			effect = CInstanceBase::EFFECT_EGG_DROP;
			break;
		case SE_DEAPO_BOOM:
			effect = CInstanceBase::EFFECT_DEAPO_BOOM;
			break;
		case SE_SKILL_DAMAGE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG;
			break;
		case SE_SKILL_DAMAGE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE;
			break;
		case SE_SKILL_DAMAGE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL;
			break;
		case SE_SKILL_SAFE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG;
			break;
		case SE_SKILL_SAFE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE;
			break;
		case SE_SKILL_SAFE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL;
			break;
#endif
#ifdef ENABLE_BATTLE_FIELD
		case SE_EFFECT_BATTLE_POTION:
			effect = CInstanceBase::EFFECT_BATTLE_POTION;
			break;
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		case SE_DRAGONLAIR_STONE_UNBEATABLE_1:
			effect = CInstanceBase::EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1;
			break;
		case SE_DRAGONLAIR_STONE_UNBEATABLE_2:
			effect = CInstanceBase::EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2;
			break;
		case SE_DRAGONLAIR_STONE_UNBEATABLE_3:
			effect = CInstanceBase::EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3;
			break;
#endif
#ifdef ENABLE_FLOWER_EVENT
		case SE_FLOWER_EVENT:
			effect = CInstanceBase::EFFECT_FLOWER_EVENT;
			break;
#endif
#ifdef ENABLE_QUEEN_NETHIS
		case SE_EFFECT_SNAKE_REGEN:
			effect = CInstanceBase::EFFECT_SNAKE_REGEN;
			bAttachEffect = false;
			break;
#endif
#ifdef ENABLE_GEM_SYSTEM
		case SE_GEM_PENDANT:
			effect = CInstanceBase::EFFECT_GEM_PENDANT;
			break;
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
		case SE_EFFECT_BP_NORMAL_MISSION_COMPLETED:
			effect = CInstanceBase::EFFECT_BP_NORMAL_MISSION_COMPLETED;
			break;
		case SE_EFFECT_BP_PREMIUM_MISSION_COMPLETED:
			effect = CInstanceBase::EFFECT_BP_PREMIUM_MISSION_COMPLETED;
			break;
		case SE_EFFECT_BP_EVENT_MISSION_COMPLETED:
			effect = CInstanceBase::EFFECT_BP_EVENT_MISSION_COMPLETED;
			break;

		case SE_EFFECT_BP_NORMAL_BATTLEPASS_COMPLETED:
			effect = CInstanceBase::EFFECT_BP_NORMAL_BATTLEPASS_COMPLETED;
			break;
		case SE_EFFECT_BP_PREMIUM_BATTLEPASS_COMPLETED:
			effect = CInstanceBase::EFFECT_BP_PREMIUM_BATTLEPASS_COMPLETED;
			break;
		case SE_EFFECT_BP_EVENT_BATTLEPASS_COMPLETED:
			effect = CInstanceBase::EFFECT_BP_EVENT_BATTLEPASS_COMPLETED;
			break;
#endif

		default:
			TraceError("%d is not a special effect number. TPacketGCSpecialEffect", kSpecialEffect.type);
			break;
	}

	if (bPlayPotionSound)
	{
		IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
		if (rkPlayer.IsMainCharacterIndex(kSpecialEffect.vid))
		{
			CPythonItem & rkItem = CPythonItem::Instance();
			rkItem.PlayUsePotionSound();
		}
	}

	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
		if (pInstance)
		{
			if (bAttachEffect)
				pInstance->AttachSpecialEffect(effect);
			else
				pInstance->CreateSpecialEffect(effect);
		}
	}

	return true;
}

#ifdef ENABLE_12ZI
bool CPythonNetworkStream::RecvSpecialZodiacEffect()
{
	TPacketGCSpecialZodiacEffect kSpecialZodiacEffect;
	if (!Recv(sizeof(kSpecialZodiacEffect), &kSpecialZodiacEffect))
		return false;

	uint32_t effect = -1;
	switch (kSpecialZodiacEffect.type)
	{
		case SE_SKILL_DAMAGE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;
		case SE_SKILL_SAFE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;
		case SE_METEOR:
			effect = CInstanceBase::EFFECT_METEOR;
			break;
		case SE_BEAD_RAIN:
			effect = CInstanceBase::EFFECT_BEAD_RAIN;
			break;
		case SE_FALL_ROCK:
			effect = CInstanceBase::EFFECT_FALL_ROCK;
			break;
		case SE_ARROW_RAIN:
			effect = CInstanceBase::EFFECT_ARROW_RAIN;
			break;
		case SE_HORSE_DROP:
			effect = CInstanceBase::EFFECT_HORSE_DROP;
			break;
		case SE_EGG_DROP:
			effect = CInstanceBase::EFFECT_EGG_DROP;
			break;
		case SE_SKILL_DAMAGE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG;
			break;
		case SE_SKILL_DAMAGE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE;
			break;
		case SE_SKILL_DAMAGE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL;
			break;
		case SE_SKILL_SAFE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG;
			break;
		case SE_SKILL_SAFE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE;
			break;
		case SE_SKILL_SAFE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL;
			break;

		default:
			TraceError("TPacketGCSpecialZodiacEffect.type::Cannot packet:: %d", kSpecialZodiacEffect.type);
			break;
	}

	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialZodiacEffect.vid);
		if (pInstance)
		{
			pInstance->AttachSpecialZodiacEffect(effect, kSpecialZodiacEffect.x, kSpecialZodiacEffect.y, 0);
		}
	}

	uint32_t effect2 = -1;
	switch (kSpecialZodiacEffect.type2)
	{
		case SE_SKILL_DAMAGE_ZONE:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;
		case SE_SKILL_SAFE_ZONE:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;
		case SE_METEOR:
			effect2 = CInstanceBase::EFFECT_METEOR;
			break;
		case SE_BEAD_RAIN:
			effect2 = CInstanceBase::EFFECT_BEAD_RAIN;
			break;
		case SE_FALL_ROCK:
			effect2 = CInstanceBase::EFFECT_FALL_ROCK;
			break;
		case SE_ARROW_RAIN:
			effect2 = CInstanceBase::EFFECT_ARROW_RAIN;
			break;
		case SE_HORSE_DROP:
			effect2 = CInstanceBase::EFFECT_HORSE_DROP;
			break;
		case SE_EGG_DROP:
			effect2 = CInstanceBase::EFFECT_EGG_DROP;
			break;
		case SE_SKILL_DAMAGE_ZONE_BIG:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG;
			break;
		case SE_SKILL_DAMAGE_ZONE_MIDDLE:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE;
			break;
		case SE_SKILL_DAMAGE_ZONE_SMALL:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL;
			break;
		case SE_SKILL_SAFE_ZONE_BIG:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG;
			break;
		case SE_SKILL_SAFE_ZONE_MIDDLE:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE;
			break;
		case SE_SKILL_SAFE_ZONE_SMALL:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL;
			break;
	}

	if (-1 != effect2)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialZodiacEffect.vid);
		if (pInstance)
		{
			pInstance->AttachSpecialZodiacEffect(effect2, kSpecialZodiacEffect.x, kSpecialZodiacEffect.y, 0);
		}
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvSpecificEffect()
{
	TPacketGCSpecificEffect kSpecificEffect;
	if (!Recv(sizeof(kSpecificEffect), &kSpecificEffect))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecificEffect.vid);

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!pInstance)
		return false;

	if (!pInstance->IsShowEffects() || !pInstance->IsShowActor())
		return false;

	CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "", kSpecificEffect.effect_file, false);
	pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
#else
	//EFFECT_TEMP
	if (pInstance)
	{
		CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "", kSpecificEffect.effect_file, false);
		pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
	}
#endif

	return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine()
{
	TPacketGCDragonSoulRefine kDragonSoul;

	if (!Recv(sizeof(kDragonSoul), &kDragonSoul))
		return false;


	switch (kDragonSoul.bSubType)
	{
	case DS_SUB_HEADER_OPEN:
#ifdef ENABLE_DS_CHANGE_ATTR
	case DS_SUB_HEADER_OPEN_CHANGE_ATTR:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("(i)", kDragonSoul.bSubType));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("()"));
#endif
		break;
	case DS_SUB_HEADER_REFINE_FAIL:
	case DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE:
	case DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineFail",
							  Py_BuildValue("(iii)", kDragonSoul.bSubType, kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	case DS_SUB_HEADER_REFINE_SUCCEED:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineSucceed",
							  Py_BuildValue("(ii)", kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	}

	return true;
}

#ifdef ENABLE_SOULBIND_SYSTEM
bool CPythonNetworkStream::RecvItemSealPacket()
{
	TPacketGCSeal kSeal;

	if (!Recv(sizeof(kSeal), &kSeal))
		return false;

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

	if (kSeal.action == 0)
	{
		rkPlayer.SealItem(kSeal.pos, kSeal.nSealDate);
	}
	else
	{
		rkPlayer.UnSealItem(kSeal.pos, kSeal.nSealDate);
	}

	return true;
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
bool CPythonNetworkStream::SendPrivateShopSearchInfo(uint8_t bRace, uint8_t bMaskType, uint8_t bMaskSubType,
	uint8_t bMinRefine, uint8_t bMaxRefine, uint8_t bMinLevel, uint8_t bMaxLevel, int iMinGold, int iMaxGold, char* itemName
#	ifdef ENABLE_CHEQUE_SYSTEM
	, uint32_t dwMinCheque, uint32_t dwMaxCheque
#	endif
)
{
	if (!__CanActMainInstance())
		return true;

	CPythonPlayer::Instance().ClearShopSearchData();

	TPacketCGPrivateShopSearch ShopSearchPacket{};
	ShopSearchPacket.header = HEADER_CG_PRIVATE_SHOP_SEARCH_SEARCH;
	ShopSearchPacket.bRace = bRace;
	ShopSearchPacket.bMaskType = bMaskType;
	ShopSearchPacket.bMaskSubType = bMaskSubType;
	ShopSearchPacket.bMinRefine = bMinRefine;
	ShopSearchPacket.bMaxRefine = bMaxRefine;
	ShopSearchPacket.bMinLevel = bMinLevel;
	ShopSearchPacket.bMaxLevel = bMaxLevel;
	ShopSearchPacket.iMinGold = iMinGold;
	ShopSearchPacket.iMaxGold = iMaxGold;
	strncpy_s(ShopSearchPacket.itemName, itemName, sizeof(ShopSearchPacket.itemName) - 1);
#ifdef ENABLE_CHEQUE_SYSTEM
	ShopSearchPacket.dwMinCheque = dwMinCheque;
	ShopSearchPacket.dwMaxCheque = dwMaxCheque;
#endif

	if (!Send(sizeof(ShopSearchPacket), &ShopSearchPacket))
	{
		Tracen("SendPrivateShopSearchInfoSub Error");
		return false;
	}

	return true;
}

void CPythonNetworkStream::SendPrivateShopSearchInfoSub(int iPage)
{
	CPythonPlayer::Instance().ShopSearchChangePage(iPage);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshPShopSearchDialog", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendPrivateShopSearchBuyItem(uint32_t shopVid, uint8_t shopItemPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGPrivateShopSearchBuy PacketShopSearchBuy{};
	PacketShopSearchBuy.header = HEADER_CG_PRIVATE_SHOP_SEARCH_BUY;
	PacketShopSearchBuy.shopVid = shopVid;
	PacketShopSearchBuy.shopItemPos = shopItemPos;

	if (!Send(sizeof(TPacketCGPrivateShopSearchBuy), &PacketShopSearchBuy))
	{
		Tracef("SendPrivateShopSerchBuyItem Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendPrivateShopSearchClose()
{
	TPacketCGPrivateShopSearchClose packet{};
	packet.header = HEADER_CG_PRIVATE_SHOP_SEARCH_CLOSE;
	if (!Send(sizeof(packet), &packet))
		return false;

	CPythonPlayer::Instance().ClearShopSearchData();
	return SendSequence();
}

bool CPythonNetworkStream::RecvPrivateShopSearchSet()
{
	TPacketGCPrivateShopSearchItemInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TPrivateShopSearchItem kItem;
		if (!Recv(sizeof(kItem), &kItem))
			return false;

		TSearchItemData searchItem;
		searchItem.vid = kItem.vid;
		searchItem.Cell = kItem.Cell;
		searchItem.pItem = kItem.Item;
		searchItem.price = kItem.price;
#ifdef ENABLE_CHEQUE_SYSTEM
		searchItem.cheque = kItem.cheque;
#endif
		memcpy(searchItem.char_name, kItem.char_name, sizeof(searchItem.char_name));

		CPythonPlayer::Instance().ShopSearchChangePage(1);
		CPythonPlayer::Instance().SetShopSearchItemData(kItem.real_position, searchItem);

		packet.wSize -= sizeof(kItem);
	}

	__RefreshShopSearchWindow();
	return true;
}

bool CPythonNetworkStream::RecvPrivateShopSearchOpen()
{
	TPacketGCPrivateShopSearchOpen p;
	if (!Recv(sizeof(p), &p))
		return false;

	if (p.subheader)
	{
		if (p.subheader == 1)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenPShopSearchDialogCash", Py_BuildValue("()"));
			return true;
		}
		else
		{
			if (p.subheader != 2)
			{
				Tracef("private_shop_search : wrong sub header(%d)", p.subheader);
				return false;
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshPShopSearchDialog", Py_BuildValue("()"));

			return true;
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenPShopSearchDialog", Py_BuildValue("()"));
	}

	return true;
}
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
bool CPythonNetworkStream::SendGemShopBuy(uint8_t bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGemShop PacketGemShop{};

	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_BUY;

	if (!Send(sizeof(TPacketCGGemShop), &PacketGemShop))
	{
		Tracef("SendGemShopBuyPacket Error\n");
		return false;
	}

	if (!Send(sizeof(uint8_t), &bPos))
	{
		Tracef("SendGemShopBuyPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSlotAdd()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGemShop PacketGemShop{};

	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_ADD;

	if (!Send(sizeof(TPacketCGGemShop), &PacketGemShop))
	{
		Tracef("SendSlotAddPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendRequestRefresh()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGGemShop packet_gemshop{};

	packet_gemshop.header = HEADER_CG_GEM_SHOP;
	packet_gemshop.subheader = GEM_SHOP_SUBHEADER_CG_REFRESH;

	if (!Send(sizeof(TPacketCGGemShop), &packet_gemshop))
	{
		Tracef("SendRequestRefreshPacket Error\n");
		return false;
	}

	return SendSequence();
}
#endif
