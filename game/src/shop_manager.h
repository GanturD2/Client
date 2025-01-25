#ifndef __INC_METIN_II_GAME_SHOP_MANAGER_H__
#define __INC_METIN_II_GAME_SHOP_MANAGER_H__

#include "../../common/CommonDefines.h"

class CShop;
typedef class CShop* LPSHOP;

class CShopManager : public singleton<CShopManager>
{
public:
	typedef std::map<uint32_t, CShop*> TShopMap;

public:
	CShopManager();
	virtual ~CShopManager();

	bool Initialize(TShopTable* table, int size);
	void Destroy();

	LPSHOP Get(uint32_t dwVnum);
	LPSHOP GetByNPCVnum(uint32_t dwVnum);

	bool StartShopping(LPCHARACTER pkChr, LPCHARACTER pkShopKeeper, int iShopVnum = 0);
	void StopShopping(LPCHARACTER ch);

	void Buy(LPCHARACTER ch, uint8_t pos);
#ifdef ENABLE_SPECIAL_INVENTORY
	void Sell(LPCHARACTER ch, uint16_t wCell, uint8_t bCount = 0);
#else
	void Sell(LPCHARACTER ch, uint8_t bCell, uint8_t bCount = 0);
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	LPSHOP CreatePCShop(LPCHARACTER ch, LPCHARACTER owner, TShopItemTable* pTable, uint8_t bItemCount, std::string sign);
#else
	LPSHOP CreatePCShop(LPCHARACTER ch, TShopItemTable* pTable, uint8_t bItemCount);
#endif
	LPSHOP FindPCShop(uint32_t dwVID);
	void DestroyPCShop(LPCHARACTER ch);
#ifdef ENABLE_EXTENDED_RELOAD
	bool ReadShopTableEx(const char* stFileName);
#endif

private:
	TShopMap m_map_pkShop;
	TShopMap m_map_pkShopByNPCVnum;
	TShopMap m_map_pkShopByPC;

#ifndef ENABLE_EXTENDED_RELOAD
	bool ReadShopTableEx(const char* stFileName);
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
public:
	bool InitializeGemShop(TGemShopTable* table, int size);
	uint32_t GemShopGetRandomId(uint32_t dwRow);
	uint32_t GemShopGetVnumById(int id);
	uint8_t GemShopGetCountById(int id);
	uint32_t GemShopGetPriceById(int id);

private:
	int m_iGemShopTableSize;
	TGemShopTable* m_pGemShopTable;
#endif
};

#endif
