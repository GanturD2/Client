#include "StdAfx.h"
#include "PythonShop.h"

#include "PythonNetworkStream.h"

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
#	include "../GameLib/ItemManager.h"
#	include "PythonCharacterManager.h"
#	include "Pythonplayer.h"
#endif

void CPythonShop::SetTabCoinType(uint8_t tabIdx, uint8_t coinType)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return;
	}
	m_aShoptabs[tabIdx].coinType = coinType;
}

uint8_t CPythonShop::GetTabCoinType(uint8_t tabIdx) const
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return 0xff;
	}
	return m_aShoptabs[tabIdx].coinType;
}

void CPythonShop::SetTabName(uint8_t tabIdx, const char * name)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return;
	}
	m_aShoptabs[tabIdx].name = name;
}

const char * CPythonShop::GetTabName(uint8_t tabIdx) const
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return nullptr;
	}

	return m_aShoptabs[tabIdx].name.c_str();
}

void CPythonShop::SetItemData(uint32_t dwIndex, const TShopItemData & c_rShopItemData)
{
#ifdef ENABLE_MYSHOP_DECO
	const uint8_t tabIdx = dwIndex / GetShopExLoading();
	const uint32_t dwSlotPos = dwIndex % GetShopExLoading();
#else
	const uint8_t tabIdx = dwIndex / SHOP_HOST_ITEM_MAX_NUM;
	const uint32_t dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;
#endif

	SetItemData(tabIdx, dwSlotPos, c_rShopItemData);
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CPythonShop::SetMyShopItemData(uint32_t dwIndex, const TShopItemData& c_rShopItemData)
{
#ifdef ENABLE_MYSHOP_DECO
	if (dwIndex >= SHOP_HOST_ITEM_MAX)
#else
	if (dwIndex >= SHOP_HOST_ITEM_MAX_NUM)
#endif
	{
		TraceError("Out of index item on my shop (idx: %lu)", dwIndex);
		return;
	}

	m_aMyShop.items[dwIndex] = c_rShopItemData;
	if (IsOpen() && IsMainPlayerPrivateShop()) {
		m_aShoptabs[0].items[dwIndex] = c_rShopItemData;
	}

	Tracef("Added shop item on %lu.", dwIndex);
}
#endif

BOOL CPythonShop::GetItemData(uint32_t dwIndex, const TShopItemData ** c_ppItemData) const
{
#ifdef ENABLE_MYSHOP_DECO
	const uint8_t tabIdx = dwIndex / CPythonShop::Instance().GetShopExLoading();
	const uint32_t dwSlotPos = dwIndex % CPythonShop::Instance().GetShopExLoading();
#else
	const uint8_t tabIdx = dwIndex / SHOP_HOST_ITEM_MAX_NUM;
	const uint32_t dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;
#endif

	return GetItemData(tabIdx, dwSlotPos, c_ppItemData);
}

void CPythonShop::SetItemData(uint8_t tabIdx, uint32_t dwSlotPos, const TShopItemData & c_rShopItemData)
{
#ifdef ENABLE_MYSHOP_DECO
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= GetShopExLoading())
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX,
			dwSlotPos, GetShopExLoading());
		return;
	}
#else
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX,
			dwSlotPos, SHOP_HOST_ITEM_MAX_NUM);
		return;
	}
#endif

	m_aShoptabs[tabIdx].items[dwSlotPos] = c_rShopItemData;
}

BOOL CPythonShop::GetItemData(uint8_t tabIdx, uint32_t dwSlotPos, const TShopItemData ** c_ppItemData) const
{
#ifdef ENABLE_MYSHOP_DECO
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= CPythonShop::Instance().GetShopExLoading())
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX,
			dwSlotPos, CPythonShop::Instance().GetShopExLoading());
		return FALSE;
	}
#else
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX,
			dwSlotPos, SHOP_HOST_ITEM_MAX_NUM);
		return FALSE;
	}
#endif

	*c_ppItemData = &m_aShoptabs[tabIdx].items[dwSlotPos];

	return TRUE;
}

void CPythonShop::ClearPrivateShopStock()
{
	m_PrivateShopItemStock.clear();
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CPythonShop::AddPrivateShopItemStock(TItemPos ItemPos, uint8_t dwDisplayPos, uint32_t dwPrice, uint32_t dwCheque)
#else
void CPythonShop::AddPrivateShopItemStock(TItemPos ItemPos, uint8_t dwDisplayPos, uint32_t dwPrice)
#endif
{
	DelPrivateShopItemStock(ItemPos);

	TShopItemTable SellingItem;
	SellingItem.vnum = 0;
	SellingItem.count = 0;
	SellingItem.pos = ItemPos;
	SellingItem.price = dwPrice;
#ifdef ENABLE_CHEQUE_SYSTEM
	SellingItem.cheque = dwCheque;
#endif
	SellingItem.display_pos = dwDisplayPos;
	m_PrivateShopItemStock.emplace(ItemPos, SellingItem);
}
void CPythonShop::DelPrivateShopItemStock(TItemPos ItemPos)
{
	if (m_PrivateShopItemStock.end() == m_PrivateShopItemStock.find(ItemPos))
		return;

	m_PrivateShopItemStock.erase(ItemPos);
}
int CPythonShop::GetPrivateShopItemPrice(TItemPos ItemPos)
{
	auto itor = m_PrivateShopItemStock.find(ItemPos);

	if (m_PrivateShopItemStock.end() == itor)
		return 0;

	TShopItemTable & rShopItemTable = itor->second;
	return rShopItemTable.price;
}

#ifdef ENABLE_CHEQUE_SYSTEM
int CPythonShop::GetPrivateShopItemCheque(TItemPos ItemPos)
{
	TPrivateShopItemStock::iterator itor = m_PrivateShopItemStock.find(ItemPos);

	if (m_PrivateShopItemStock.end() == itor)
		return 0;

	TShopItemTable & rShopItemTable = itor->second;
	return rShopItemTable.cheque;
}
#endif

struct ItemStockSortFunc
{
	bool operator()(TShopItemTable & rkLeft, TShopItemTable & rkRight) const { return rkLeft.display_pos < rkRight.display_pos; }
};
void CPythonShop::BuildPrivateShop(const char * c_szName)
{
	std::vector<TShopItemTable> ItemStock;
	ItemStock.reserve(m_PrivateShopItemStock.size());

	auto itor = m_PrivateShopItemStock.begin();
	for (; itor != m_PrivateShopItemStock.end(); ++itor)
		ItemStock.emplace_back(itor->second);

	std::sort(ItemStock.begin(), ItemStock.end(), ItemStockSortFunc());

	CPythonNetworkStream::Instance().SendBuildPrivateShopPacket(c_szName, ItemStock);
}

void CPythonShop::Open(BOOL isPrivateShop, BOOL isMainPrivateShop)
{
	m_isShoping = TRUE;
	m_isPrivateShop = isPrivateShop;
	m_isMainPlayerPrivateShop = isMainPrivateShop;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (isMainPrivateShop)
		m_aShoptabs[0] = m_aMyShop;
#endif
}

void CPythonShop::Close()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
#ifdef ENABLE_12ZI
	m_isLimitedItemShop = FALSE;
#endif
}

BOOL CPythonShop::IsOpen() const
{
	return m_isShoping;
}

BOOL CPythonShop::IsPrivateShop() const
{
	return m_isPrivateShop;
}

BOOL CPythonShop::IsMainPlayerPrivateShop() const
{
	return m_isMainPlayerPrivateShop;
}

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
void CPythonShop::SetLimitedPurchaseInfo(uint32_t dwVnum, uint32_t dwLimitedCount, uint32_t dwLimitedPurchaseCount)
{
	const auto it = m_ShopLimitedCount.find(dwVnum);
	if (it != m_ShopLimitedCount.end())
	{
		it->second.dwLimitedCount = dwLimitedCount;
		it->second.dwLimitedPurchaseCount = dwLimitedPurchaseCount;
	}
	else
	{
		TShopPriceLimitCount test{};
		test.dwLimitedCount = dwLimitedCount;
		test.dwLimitedPurchaseCount = dwLimitedPurchaseCount;
		m_ShopLimitedCount.emplace(dwVnum, test);
	}
}

uint32_t CPythonShop::GetLimitedCount(uint32_t dwVnum)
{
	const auto it = m_ShopLimitedCount.find(dwVnum);
	if (it != m_ShopLimitedCount.end())
		return it->second.dwLimitedCount;

	return 0;
}

uint32_t CPythonShop::GetLimitedPurchaseCount(uint32_t dwVnum)
{
	const auto it = m_ShopLimitedCount.find(dwVnum);
	if (it != m_ShopLimitedCount.end())
		return it->second.dwLimitedPurchaseCount;

	return 0;
}
#endif

#ifdef ENABLE_12ZI
void CPythonShop::SetLimitedItemShop(bool isLimitedItemShop)
{
	m_isLimitedItemShop = isLimitedItemShop;
}

BOOL CPythonShop::IsLimitedItemShop()
{
	return m_isLimitedItemShop;
}
#endif

#ifdef ENABLE_MYSHOP_DECO	//ENABLE_RENEWAL_SHOPEX
uint8_t CPythonShop::GetShopExLoading()
{
	return bShopExLoading ? SHOP_HOST_ITEM_MAX_NUM : SHOP_HOST_ITEM_MAX;
}
#endif

void CPythonShop::Clear()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
#ifdef ENABLE_12ZI
	m_isLimitedItemShop = FALSE;
#endif
#ifdef ENABLE_MYSHOP_DECO	//ENABLE_RENEWAL_SHOPEX
	bShopExLoading = false;
#endif
	ClearPrivateShopStock();
	m_bTabCount = 1;

	for (auto & m_aShoptab : m_aShoptabs)
	{
		// @fixme016 BEGIN
		m_aShoptab.coinType = SHOP_COIN_TYPE_GOLD;
		m_aShoptab.name = "";
		// @fixme016 END
		msl::refill(m_aShoptab.items);
	}

#ifdef ENABLE_BATTLE_FIELD
	memset(m_aLimitMax, 0, sizeof(m_aLimitMax));
	memset(m_aUsablePoint, 0, sizeof(m_aUsablePoint));
#endif
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void CPythonShop::ClearMyShopInfo()
{
#ifdef ENABLE_MYSHOP_DECO
	memset(m_aMyShop.items, 0, sizeof(TShopItemData) * SHOP_HOST_ITEM_MAX);
#else
	memset(m_aMyShop.items, 0, sizeof(TShopItemData) * SHOP_HOST_ITEM_MAX_NUM);
#endif
	m_myShopName.clear();
}
#endif

CPythonShop::CPythonShop()
{
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	m_stashValue = 0;
#	ifdef ENABLE_CHEQUE_SYSTEM
	m_chequeStashValue = 0;
#	endif
	m_offlineMinutes = 0;
#endif
#ifdef ENABLE_MYSHOP_DECO	//ENABLE_RENEWAL_SHOPEX
	bShopExLoading = false;
#endif
	Clear();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	ClearMyShopInfo();
#endif
}

CPythonShop::~CPythonShop(void) = default;

PyObject * shopOpen(PyObject * poSelf, PyObject * poArgs)
{
	int isPrivateShop = false;
	PyTuple_GetInteger(poArgs, 0, &isPrivateShop);
	int isMainPrivateShop = false;
	PyTuple_GetInteger(poArgs, 1, &isMainPrivateShop);

	CPythonShop & rkShop = CPythonShop::Instance();
	rkShop.Open(isPrivateShop, isMainPrivateShop);
	return Py_BuildNone();
}

PyObject * shopClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop & rkShop = CPythonShop::Instance();
	rkShop.Close();
	return Py_BuildNone();
}

PyObject * shopIsOpen(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop & rkShop = CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsOpen());
}

PyObject * shopIsPrviateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop & rkShop = CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsPrivateShop());
}

PyObject * shopIsMainPlayerPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop & rkShop = CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsMainPlayerPrivateShop());
}

PyObject * shopGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int nPos;
	if (!PyTuple_GetInteger(poArgs, 0, &nPos))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(nPos, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->vnum);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->count);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
#ifdef ENABLE_CHEQUE_SYSTEM
		return Py_BuildValue("i", c_pItemData->price.dwPrice);
#else
		return Py_BuildValue("i", c_pItemData->price);
#endif

	return Py_BuildValue("i", 0);
}

#ifdef ENABLE_CHEQUE_SYSTEM
PyObject* shopGetItemCheque(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData* c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->price.dwCheque);

	return Py_BuildValue("i", 0);
}
#endif

PyObject * shopGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iMetinSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->alSockets[iMetinSocketIndex]);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		const TShopItemData * c_pItemData;
		if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
			return Py_BuildValue("ii", c_pItemData->aAttr[iAttrSlotIndex].wType, c_pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject * shopClearPrivateShopStock(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop::Instance().ClearPrivateShopStock();
	return Py_BuildNone();
}
PyObject * shopAddPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();
	int iDisplaySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
		return Py_BuildException();
	int iPrice;
	if (!PyTuple_GetInteger(poArgs, 3, &iPrice))
		return Py_BuildException();

#ifdef ENABLE_CHEQUE_SYSTEM
	int iCheque;
	if (!PyTuple_GetInteger(poArgs, 4, &iCheque))
		return Py_BuildException();

	CPythonShop::Instance().AddPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex), iDisplaySlotIndex, iPrice, iCheque);
#else
	CPythonShop::Instance().AddPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex), iDisplaySlotIndex, iPrice);
#endif
	return Py_BuildNone();
}

PyObject * shopDelPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	CPythonShop::Instance().DelPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildNone();
}

PyObject * shopGetPrivateShopItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	int iValue = CPythonShop::Instance().GetPrivateShopItemPrice(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildValue("i", iValue);
}

#ifdef ENABLE_CHEQUE_SYSTEM
PyObject * shopGetPrivateShopItemCheque(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	uint16_t wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	int iValue = CPythonShop::Instance().GetPrivateShopItemCheque(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildValue("i", iValue);
}
#endif

PyObject * shopBuildPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonShop::Instance().BuildPrivateShop(szName);
	return Py_BuildNone();
}

PyObject * shopGetTabCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetTabCount());
}

PyObject * shopGetTabName(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bTabIdx;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		return Py_BuildException();

	return Py_BuildValue("s", CPythonShop::Instance().GetTabName(bTabIdx));
}

PyObject * shopGetTabCoinType(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bTabIdx;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonShop::Instance().GetTabCoinType(bTabIdx));
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject* shopGetItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData* c_pItemData;
	if (!CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", c_pItemData->dwTransmutationVnum);
}
#endif

#ifdef ENABLE_12ZI
PyObject * shopIsLimitedItemShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop = CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsLimitedItemShop());
}

PyObject * shopGetLimitedCount(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->getLimitedCount);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetLimitedPurchaseCount(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->getLimitedPurchaseCount);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
PyObject* shopGetMyName(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", CPythonShop::Instance().GetMyShopName().c_str());
}

PyObject* shopClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonShop::Instance().Clear();
	CPythonShop::Instance().ClearMyShopInfo();
	CPythonShop::Instance().SetOfflineMinutes(0);
	CPythonShop::Instance().SetStashValue(0);
#	ifdef ENABLE_CHEQUE_SYSTEM
	CPythonShop::Instance().SetChequeStashValue(0);
#	endif
	CPythonShop::Instance().SetLocation(0, 0, 0);
	return Py_BuildNone();
}

PyObject* shopIsMyShopOpen(PyObject* poSelf, PyObject* poArgs)
{
	if (CPythonShop::Instance().GetMyShopName().compare("") != 0)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

PyObject* shopGetOfflineMinutes(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetOfflineMinutes());
}

PyObject* shopGetGoldStash(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetStashValue());
}

#	ifdef ENABLE_CHEQUE_SYSTEM
PyObject* shopGetChequeStash(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetChequeStashValue());
}
#	endif

PyObject* shopGetLocation(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonShop::ShopPosition& pos = CPythonShop::Instance().GetLocation();
	return Py_BuildValue("(iii)", pos.channel, pos.x, pos.y);
}
#endif

#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
PyObject* shopGetLimitMaxPoint(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bCoinType;
	if (!PyTuple_GetInteger(poArgs, 0, &bCoinType))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonShop::Instance().GetLimitMaxPoint(bCoinType));
}

PyObject* shopSetUsablePoint(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bCoinType;
	if (!PyTuple_GetInteger(poArgs, 0, &bCoinType))
		return Py_BuildException();

	int iLimitMaxPoint;
	if (!PyTuple_GetInteger(poArgs, 1, &iLimitMaxPoint))
		return Py_BuildException();

	CPythonShop::Instance().SetUsablePoint(bCoinType, iLimitMaxPoint);
	return Py_BuildNone();
}

PyObject* shopGetUsablePoint(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bCoinType;
	if (!PyTuple_GetInteger(poArgs, 0, &bCoinType))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonShop::Instance().GetUsablePoint(bCoinType));
}
#endif

#ifdef ENABLE_MEDAL_OF_HONOR
PyObject* shopGetLimitLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData* c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->limit_level);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
PyObject* shopGetItemElements(PyObject* poTarget, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData* c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData)) {
		if (c_pItemData->grade_element > 0)
			return Py_BuildValue("iiii", c_pItemData->grade_element, c_pItemData->attack_element[c_pItemData->grade_element - 1], c_pItemData->element_type_bonus, c_pItemData->elements_value_bonus[c_pItemData->grade_element - 1]);
	}

	return Py_BuildValue("iiii", 0, 0, 0, 0);
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
PyObject* shopGetItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < APPLY_RANDOM_SLOT_MAX_NUM)
	{
		const TShopItemData* c_pItemData;
		if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
			return Py_BuildValue("ii", c_pItemData->aApplyRandom[iAttrSlotIndex].wType, c_pItemData->aApplyRandom[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* shopGetRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int iRandomValueIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
		return Py_BuildException();

	const TShopItemData* c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->alRandomValues[iRandomValueIndex]);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_SET_ITEM
PyObject* shopGetItemSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData* c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->set_value);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
PyObject* shopGetPrivateShopSelectItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(iSlotIndex);
	return Py_BuildValue("i", pItemData.pItem.alSockets[iSocketIndex]);
}

PyObject* shopGetPrivateShopSelectItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(iSlotIndex);
		return Py_BuildValue("ii", pItemData.pItem.aAttr[iAttrSlotIndex].wType, pItemData.pItem.aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* shopGetPrivateShopSelectItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(ipos);
	return Py_BuildValue("i", pItemData.pItem.vnum);
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM //SEARCHSHOP_CHANGELOOK
PyObject* shopGetPrivateShopItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	const TSearchItemData pInstance = CPythonPlayer::Instance().GetShopSearchItemData(ipos);
	return Py_BuildValue("i", pInstance.pItem.dwTransmutationVnum);
}
#endif

#ifdef ENABLE_REFINE_ELEMENT //SEARCHSHOP_ELEMENT
PyObject* shopGetPrivateShopItemRefineElement(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	const TSearchItemData pInstance = CPythonPlayer::Instance().GetShopSearchItemData(ipos);
	if (pInstance.pItem.grade_element > 0)
	{
		return Py_BuildValue("iiii",
			pInstance.pItem.grade_element,
			pInstance.pItem.attack_element[pInstance.pItem.grade_element - 1],
			pInstance.pItem.element_type_bonus,
			pInstance.pItem.elements_value_bonus[pInstance.pItem.grade_element - 1]
		);
	}

	return Py_BuildValue("iiii", 0, 0, 0, 0);
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
PyObject* shopGetPrivateShopItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < APPLY_RANDOM_SLOT_MAX_NUM)
	{
		const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(iSlotIndex);
		return Py_BuildValue("ii", pItemData.pItem.aApplyRandom[iAttrSlotIndex].wType, pItemData.pItem.aApplyRandom[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* shopGetPrivateShopGetRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iRandomValueIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
		return Py_BadArgument();

	if (iRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM)
		return Py_BuildException();

	const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(iSlotIndex);
	return Py_BuildValue("i", pItemData.pItem.alRandomValues[iRandomValueIndex]);
}
#endif

#ifdef ENABLE_SET_ITEM
PyObject* shopGetPrivateShopItemSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(iSlotIndex);
	return Py_BuildValue("i", pItemData.pItem.set_value);
}
#endif

PyObject* shopGetPrivateShopSearchResult(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(ipos);
	return Py_BuildValue("ssiii", pItemData.item_name, pItemData.char_name, pItemData.pItem.count, pItemData.price, pItemData.cheque);
}

PyObject* shopGetPrivateShopSearchResultCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetShopSearchResultCount());
}

PyObject* shopGetPrivateShopSearchResultMaxCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetShopSearchResultMaxCount());
}

PyObject* shopGetPrivateShopSearchResultPage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPlayer::Instance().GetShopSearchPage());
}

PyObject* shopGetPrivateShopSelectItemChrVID(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(ipos);
	return Py_BuildValue("i", pItemData.vid);
}
#endif

void initshop()
{
	static PyMethodDef s_methods[] = {// Shop
									  {"Open", shopOpen, METH_VARARGS},
									  {"Close", shopClose, METH_VARARGS},
									  {"IsOpen", shopIsOpen, METH_VARARGS},
									  {"IsPrivateShop", shopIsPrviateShop, METH_VARARGS},
									  {"IsMainPlayerPrivateShop", shopIsMainPlayerPrivateShop, METH_VARARGS},
									  {"GetItemID", shopGetItemID, METH_VARARGS},
									  {"GetItemCount", shopGetItemCount, METH_VARARGS},
									  {"GetItemPrice", shopGetItemPrice, METH_VARARGS},
#ifdef ENABLE_CHEQUE_SYSTEM
		{ "GetItemCheque",				shopGetItemCheque,				METH_VARARGS },
#endif
									  {"GetItemMetinSocket", shopGetItemMetinSocket, METH_VARARGS},
									  {"GetItemAttribute", shopGetItemAttribute, METH_VARARGS},
									  {"GetTabCount", shopGetTabCount, METH_VARARGS},
									  {"GetTabName", shopGetTabName, METH_VARARGS},
									  {"GetTabCoinType", shopGetTabCoinType, METH_VARARGS},

									  // Private Shop
									  {"ClearPrivateShopStock", shopClearPrivateShopStock, METH_VARARGS},
									  {"AddPrivateShopItemStock", shopAddPrivateShopItemStock, METH_VARARGS},
									  {"DelPrivateShopItemStock", shopDelPrivateShopItemStock, METH_VARARGS},
									  {"GetPrivateShopItemPrice", shopGetPrivateShopItemPrice, METH_VARARGS},
#ifdef ENABLE_CHEQUE_SYSTEM
		{ "GetPrivateShopItemCheque",	shopGetPrivateShopItemCheque,	METH_VARARGS },
#endif
									  {"BuildPrivateShop", shopBuildPrivateShop, METH_VARARGS},

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "GetItemChangeLookVnum",		shopGetItemChangeLookVnum,		METH_VARARGS },
		{ "GetPrivateShopItemChangeLookVnum",	shopGetItemChangeLookVnum,		METH_VARARGS},
#endif
#ifdef ENABLE_12ZI
		{ "IsLimitedItemShop",			shopIsLimitedItemShop,			METH_VARARGS },
		{ "GetLimitedCount",			shopGetLimitedCount,			METH_VARARGS },
		{ "GetLimitedPurchaseCount",	shopGetLimitedPurchaseCount,	METH_VARARGS },
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		// MY shop
		{"GetMyShopName", shopGetMyName, METH_VARARGS},
		{"IsMyShopOpen", shopIsMyShopOpen, METH_VARARGS},
		{"GetOfflineMinutes", shopGetOfflineMinutes, METH_VARARGS},
		{"GetStashValue" , shopGetGoldStash, METH_VARARGS},
#	ifdef ENABLE_CHEQUE_SYSTEM
		{"GetChequeStashValue" , shopGetChequeStash, METH_VARARGS},
#	endif
		{"GetLocation", shopGetLocation, METH_VARARGS},
		{"Clear", shopClear, METH_VARARGS},
#endif

#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
		{"GetLimitMaxPoint", shopGetLimitMaxPoint, METH_VARARGS},
		{"SetUsablePoint", shopSetUsablePoint, METH_VARARGS},
		{"GetUsablePoint", shopGetUsablePoint, METH_VARARGS},
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
		{"GetLimitLevel", shopGetLimitLevel, METH_VARARGS},
#endif

#ifdef ENABLE_REFINE_ELEMENT
		{ "GetItemElements",			shopGetItemElements,			METH_VARARGS },
#endif

#ifdef ENABLE_YOHARA_SYSTEM
		{"GetItemApplyRandom", shopGetItemApplyRandom, METH_VARARGS},
		{"GetRandomValue", shopGetRandomValue, METH_VARARGS},
#endif

#ifdef ENABLE_SET_ITEM
		{"GetItemSetValue", shopGetItemSetValue, METH_VARARGS},
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
									  {"GetPrivateShopSelectItemAttribute", shopGetPrivateShopSelectItemAttribute, METH_VARARGS},
									  {"GetPrivateShopSelectItemMetinSocket", shopGetPrivateShopSelectItemMetinSocket, METH_VARARGS},
									  {"GetPrivateShopSelectItemVnum", shopGetPrivateShopSelectItemVnum, METH_VARARGS},
#	ifdef ENABLE_CHANGE_LOOK_SYSTEM
									  {"GetPrivateShopItemChangeLookVnum", shopGetPrivateShopItemChangeLookVnum, METH_VARARGS},
#	endif
#	ifdef ENABLE_REFINE_ELEMENT
									  {"GetPrivateShopItemRefineElement", shopGetPrivateShopItemRefineElement, METH_VARARGS},
#	endif
#	ifdef ENABLE_YOHARA_SYSTEM							  
									  {"GetPrivateShopItemApplyRandom", shopGetPrivateShopItemApplyRandom, METH_VARARGS},
									  {"GetPrivateShopGetRandomValue", shopGetPrivateShopGetRandomValue, METH_VARARGS},
#	endif
#	ifdef ENABLE_SET_ITEM
									  {"GetPrivateShopItemSetValue", shopGetPrivateShopItemSetValue, METH_VARARGS},
#	endif

									  {"GetPrivateShopSearchResult", shopGetPrivateShopSearchResult, METH_VARARGS},
									  {"GetPrivateShopSearchResultCount", shopGetPrivateShopSearchResultCount, METH_VARARGS},
									  {"GetPrivateShopSearchResultMaxCount", shopGetPrivateShopSearchResultMaxCount, METH_VARARGS},
									  {"GetPrivateShopSearchResultPage", shopGetPrivateShopSearchResultPage, METH_VARARGS},
									  {"GetPrivateShopSelectItemChrVID", shopGetPrivateShopSelectItemChrVID, METH_VARARGS},
#endif

									  {nullptr, nullptr, 0}};
	PyObject * poModule = Py_InitModule("shop", s_methods);

	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_GOLD", SHOP_COIN_TYPE_GOLD);
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_SECONDARY_COIN", SHOP_COIN_TYPE_SECONDARY_COIN);
#ifdef ENABLE_MYSHOP_DECO
	PyModule_AddIntConstant(poModule, "SHOP_HOST_ITEM_MAX", SHOP_HOST_ITEM_MAX);
#endif
	PyModule_AddIntConstant(poModule, "SHOP_SLOT_COUNT", SHOP_HOST_ITEM_MAX_NUM);
	PyModule_AddIntConstant(poModule, "OFFLINESHOP_COUNT_SLOT", SHOP_HOST_ITEM_MAX_NUM);
#ifdef ENABLE_RENEWAL_SHOPEX
	PyModule_AddIntConstant(poModule, "SHOP_COINT_TYPE_ITEM", SHOP_COINT_TYPE_ITEM);
	PyModule_AddIntConstant(poModule, "SHOP_COINT_TYPE_EXP", SHOP_COINT_TYPE_EXP);
#endif
#ifdef ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_BATTLE_POINT", SHOP_COIN_TYPE_BATTLE_POINT);
#endif
#ifdef ENABLE_10TH_EVENT
	SHOP_COIN_TYPE_10TH_EVENT,
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_MEDAL_OF_HONOR", SHOP_COIN_TYPE_MEDAL_OF_HONOR);
#endif
#ifdef ENABLE_MINI_GAME_BNW
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_BNW", SHOP_COIN_TYPE_BNW);
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_ACHIEVEMENT", SHOP_COIN_TYPE_ACHIEVEMENT);
#endif
}