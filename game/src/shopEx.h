#ifndef __INC_METIN_II_GAME_SHOP_SECONDARY_COIN_H__
#define __INC_METIN_II_GAME_SHOP_SECONDARY_COIN_H__
#include "typedef.h"
#include "shop.h"

struct SShopTable;
typedef struct SShopTableEx : SShopTable
{
	std::string name;
	EShopCoinType coinType;
} TShopTableEx;

class CGroupNode;

/*
	Extension Shop.
	You can use Bright Challenge as currency, and you can divide items into multiple tabs and place them.
	However, the PC shop is not implemented.
	When communicating with the client, tabs are divided into pos 45 units.
	The m_itemVector of the existing shop is not used.
*/
class CShopEx : public CShop
{
public:
	bool Create(uint32_t dwVnum, uint32_t dwNPCVnum);
	bool AddShopTable(TShopTableEx& shopTable);

	virtual bool AddGuest(LPCHARACTER ch, uint32_t owner_vid, bool bOtherEmpire);
	virtual void SetPCShop(LPCHARACTER ch) noexcept { return; }
	virtual bool IsPCShop() noexcept { return false; }
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	virtual int Buy(LPCHARACTER ch, uint8_t pos, bool isPrivateShopSearch = false);
#else
	virtual int Buy(LPCHARACTER ch, uint8_t pos);
#endif
	virtual bool IsSellingItem(uint32_t itemID) noexcept { return false; }

	size_t GetTabCount() noexcept { return m_vec_shopTabs.size(); }
private:
	std::vector <TShopTableEx> m_vec_shopTabs;
};
typedef CShopEx* LPSHOPEX;

#endif
