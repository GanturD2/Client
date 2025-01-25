#ifndef __INC_METIN_II_GAME_SHOP_H__
#define __INC_METIN_II_GAME_SHOP_H__

#include "../../common/CommonDefines.h"

enum
{
	SHOP_MAX_DISTANCE = 1000
};

class CGrid;

/* ---------------------------------------------------------------------------------- */
class CShop
{
public:
	typedef struct shop_item
	{
		uint32_t vnum;
		long price;
		uint8_t count;
#ifdef ENABLE_CHEQUE_SYSTEM
		int32_t cheque;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		uint32_t dwTransmutationVnum;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		uint32_t purchase_limit;
#endif
		LPITEM pkItem;
		int itemid;

		shop_item()
		{
			vnum = 0;
			price = 0;
			count = 0;
#ifdef ENABLE_CHEQUE_SYSTEM
			cheque = 0;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			dwTransmutationVnum = 0;
#endif
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
			purchase_limit = 0;
#endif
			itemid = 0;
			pkItem = nullptr;
		}
	} SHOP_ITEM;

	CShop();
	virtual ~CShop(); // @fixme139 (+virtual)

	bool Create(uint32_t dwVnum, uint32_t dwNPCVnum, TShopItemTable* pItemTable
#if defined(ENABLE_12ZI) && defined(ENABLE_12ZI_SHOP_LIMIT)
		, bool bIsLimitedItemShop
#endif
	);
	void SetShopItems(TShopItemTable* pItemTable, uint8_t bItemCount);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void TransferItems(LPCHARACTER owner, TShopItemTable* pTable, uint8_t bItemCount);
	void SetShopItem(TShopItemTable* pItemTable);
	LPCHARACTER GetOwner() { return m_pkPC; };
#endif

	virtual void SetPCShop(LPCHARACTER ch);
	virtual bool IsPCShop() { return m_pkPC ? true : false; }

	virtual bool AddGuest(LPCHARACTER ch, uint32_t owner_vid, bool bOtherEmpire);
	void RemoveGuest(LPCHARACTER ch);
	virtual int Buy(LPCHARACTER ch, uint8_t pos
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
		, bool isPrivateShopSearch = false
#endif
	);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	bool TransferItemAway(LPCHARACTER ch, uint8_t pos, TItemPos targetPos);
	bool RemoveItemByID(uint32_t itemID);
#endif

	void BroadcastUpdateItem(uint8_t pos);
#ifdef ENABLE_12ZI
	void BroadcastUpdateItemCh(uint8_t pos, LPCHARACTER ch);
#endif

	int GetNumberByVnum(uint32_t dwVnum);

	virtual bool IsSellingItem(uint32_t itemID);

	uint32_t GetVnum() { return m_dwVnum; }
	uint32_t GetNPCVnum() { return m_dwNPCVnum; }
#ifndef ENABLE_PREMIUM_PRIVATE_SHOP
	bool IsSoldOut() const; //@custom036
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	std::string GetShopSign() { return m_sign; }
	void SetShopSign(std::string sign) { m_sign = sign; }
	std::vector<SHOP_ITEM> GetItemVector() const { return m_itemVector; }

	void SetClosed(bool status) { m_closed = status; }
	bool IsClosed() const { return m_closed; }

	void SetOpenTime(uint32_t time) { m_openTime = time; }
	uint32_t GetOpenTime() const { return m_openTime; }

	void SetOfflineMinutes(int mins) { m_offlineMinutes = mins; }
	int GetOfflineMinutes() const { return m_offlineMinutes; }

	void SetNextRenamePulse(int pulse) { m_renamePulse = pulse; }
	int GetRenamePulse() { return m_renamePulse; }

	void Save();

#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	void SetTitleType(uint8_t dwTitleType) noexcept { m_bTitleType = dwTitleType; }
	uint8_t GetTitleType() noexcept { return m_bTitleType; }

	void SetShopDeco(bool status) noexcept { isShopDeco = status; }
	bool IsShopDeco() noexcept { return isShopDeco; }
#endif

	void SaveOffline();
	void CloseShopBecauseEditing();

	bool IsEmpty();
#endif

	static bool CanOpenShopHere(long mapindex); //@custom023

protected:
#ifdef ENABLE_MYSHOP_DECO_PREMIUM
	uint8_t m_bTitleType;
	bool isShopDeco;
#endif

	void Broadcast(const void* data, int bytes);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void CloseMyShop();
#endif

protected:
	uint32_t m_dwVnum;
	uint32_t m_dwNPCVnum;
#ifdef ENABLE_12ZI
	bool m_IsLimitedItemShop;
#endif

	CGrid* m_pGrid;

	typedef std::unordered_map<LPCHARACTER, bool> GuestMapType;
	GuestMapType m_map_guest;
	std::vector<SHOP_ITEM> m_itemVector;

	LPCHARACTER m_pkPC;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	std::string m_sign;
	bool m_closed;

	uint32_t m_openTime;
	int m_offlineMinutes;
	int m_renamePulse;
#endif
};

#endif
