#pragma once

#include "Packet.h"

/*
 *	상점 처리
 *
 *	2003-01-16 anoa	일차 완료
 *	2003-12-26 levites 수정
 *
 *	2012-10-29 rtsummit 새로운 화폐 출현 및 tab 기능 추가로 인한 shop 확장.
 *
 */
typedef enum
{
	SHOP_COIN_TYPE_GOLD, // DEFAULT VALUE
	SHOP_COIN_TYPE_SECONDARY_COIN,
#ifdef ENABLE_RENEWAL_SHOPEX
	SHOP_COINT_TYPE_ITEM,
	SHOP_COINT_TYPE_EXP,
#endif
#ifdef ENABLE_BATTLE_FIELD
	SHOP_COIN_TYPE_BATTLE_POINT,
#endif
#ifdef ENABLE_10TH_EVENT
	SHOP_COIN_TYPE_10TH_EVENT,
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	SHOP_COIN_TYPE_MEDAL_OF_HONOR,
#endif
#ifdef ENABLE_MINI_GAME_BNW
	SHOP_COIN_TYPE_BNW,
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	SHOP_COIN_TYPE_ACHIEVEMENT,
#endif
	SHOP_COIN_TYPE_MAX
} EShopCoinType;

class CPythonShop : public CSingleton<CPythonShop>
{
public:
	CPythonShop();
	~CPythonShop();
	CLASS_DELETE_COPYMOVE(CPythonShop);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	struct ShopPosition
	{
		ShopPosition() : channel(0), x(0), y(0) {};
		int channel, x, y;
	};
#endif

	void Clear();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void ClearMyShopInfo();
#endif

	void SetItemData(uint32_t dwIndex, const TShopItemData & c_rShopItemData);
	BOOL GetItemData(uint32_t dwIndex, const TShopItemData ** c_ppItemData) const;

	void SetItemData(uint8_t tabIdx, uint32_t dwSlotPos, const TShopItemData & c_rShopItemData);
	BOOL GetItemData(uint8_t tabIdx, uint32_t dwSlotPos, const TShopItemData ** c_ppItemData) const;

	void SetTabCount(uint8_t bTabCount) { m_bTabCount = bTabCount; }
	uint8_t GetTabCount() const { return m_bTabCount; }

#ifdef ENABLE_MYSHOP_DECO	//ENABLE_RENEWAL_SHOPEX
	void SetShopExLoading(const bool bShopExLoad) { bShopExLoading = bShopExLoad; }
	uint8_t GetShopExLoading();
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	//My shop
	void SetMyShopItemData(uint32_t dwIndex, const TShopItemData& c_rShopItemData);
	void SetMyShopName(std::string name) { m_myShopName = name; };
	std::string GetMyShopName() const { return m_myShopName; }
#endif

	void SetTabCoinType(uint8_t tabIdx, uint8_t coinType);
	uint8_t GetTabCoinType(uint8_t tabIdx) const;

	void SetTabName(uint8_t tabIdx, const char * name);
	const char * GetTabName(uint8_t tabIdx) const;


	//BOOL GetSlotItemID(uint32_t dwSlotPos, uint32_t* pdwItemID);

	void Open(BOOL isPrivateShop, BOOL isMainPrivateShop);

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	void SetLimitedPurchaseInfo(uint32_t dwVnum, uint32_t dwLimitedCount, uint32_t dwLimitedPurchaseCount);
	uint32_t GetLimitedCount(uint32_t dwVnum);
	uint32_t GetLimitedPurchaseCount(uint32_t dwVnum);
#endif

#ifdef ENABLE_12ZI
	void SetLimitedItemShop(bool isLimitedItemShop);
	BOOL IsLimitedItemShop();
#endif
	void Close();
	BOOL IsOpen() const;
	BOOL IsPrivateShop() const;
	BOOL IsMainPlayerPrivateShop() const;

	void ClearPrivateShopStock();
#ifdef ENABLE_CHEQUE_SYSTEM
	void AddPrivateShopItemStock(TItemPos ItemPos, uint8_t dwDisplayPos, uint32_t dwPrice, uint32_t dwCheque);
#else
	void AddPrivateShopItemStock(TItemPos ItemPos, uint8_t dwDisplayPos, uint32_t dwPrice);
#endif
	void DelPrivateShopItemStock(TItemPos ItemPos);
	int GetPrivateShopItemPrice(TItemPos ItemPos);
#ifdef ENABLE_CHEQUE_SYSTEM
	int GetPrivateShopItemCheque(TItemPos ItemPos);
#endif
	void BuildPrivateShop(const char * c_szName);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void SetOfflineMinutes(int minutes) { m_offlineMinutes = minutes; }
	int GetOfflineMinutes() { return m_offlineMinutes; }

	void SetStashValue(uint32_t value) { m_stashValue = value; }
	uint32_t GetStashValue() const { return m_stashValue; }

#	ifdef ENABLE_CHEQUE_SYSTEM
	void SetChequeStashValue(uint32_t cheque_value) { m_chequeStashValue = cheque_value; }
	uint32_t GetChequeStashValue() const { return m_chequeStashValue; }
#	endif

	void SetLocation(int channel, int x, int y) { m_shopPosition.channel = channel; m_shopPosition.x = x; m_shopPosition.y = y; }
	const ShopPosition& GetLocation() { return m_shopPosition; }
#endif

protected:
	BOOL CheckSlotIndex(uint32_t dwIndex);

protected:
	BOOL m_isShoping;
	BOOL m_isPrivateShop;
	BOOL m_isMainPlayerPrivateShop;
#ifdef ENABLE_12ZI
	BOOL m_isLimitedItemShop;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	ShopPosition m_shopPosition;

	int m_offlineMinutes;
	uint32_t m_stashValue;
#	ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t m_chequeStashValue;
#	endif
#endif

	struct ShopTab
	{
		ShopTab() { coinType = SHOP_COIN_TYPE_GOLD; }
		uint8_t coinType;
		std::string name;
#ifdef ENABLE_MYSHOP_DECO
		TShopItemData items[SHOP_HOST_ITEM_MAX];
#else
		TShopItemData items[SHOP_HOST_ITEM_MAX_NUM];
#endif
	};

	uint8_t m_bTabCount;
	ShopTab m_aShoptabs[SHOP_TAB_COUNT_MAX];

	typedef std::map<TItemPos, TShopItemTable> TPrivateShopItemStock;
	TPrivateShopItemStock m_PrivateShopItemStock;

#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
	typedef std::map<uint32_t, TShopPriceLimitCount> TShopLimitCount;
	TShopLimitCount m_ShopLimitedCount;
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	std::string m_myShopName;
	ShopTab m_aMyShop;
#endif

#if defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_MEDAL_OF_HONOR)
public:
	void SetLimitMaxPoint(uint8_t coinType, int max_point) { m_aLimitMax[coinType] = max_point; }
	int GetLimitMaxPoint(uint8_t coinType) { return m_aLimitMax[coinType]; }

	void SetUsablePoint(uint8_t coinType, int usable_point) { m_aUsablePoint[coinType] = usable_point; };
	int GetUsablePoint(uint8_t coinType) { return m_aUsablePoint[coinType]; };
protected:
	int m_aLimitMax[SHOP_COIN_TYPE_MAX];
	int m_aUsablePoint[SHOP_COIN_TYPE_MAX];
#endif

#ifdef ENABLE_MYSHOP_DECO	//ENABLE_RENEWAL_SHOPEX
private:
	bool bShopExLoading;
#endif
};
