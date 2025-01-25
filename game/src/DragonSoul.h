#ifndef __INC_METIN_II_GAME_DRAGON_SOUL_H__
#define __INC_METIN_II_GAME_DRAGON_SOUL_H__

#include "../../common/length.h"

class CHARACTER;
class CItem;

class DragonSoulTable;

class DSManager : public singleton<DSManager>
{
public:
	DSManager();
	~DSManager();
	bool ReadDragonSoulTableFile(const char* c_pszFileName);

	void GetDragonSoulInfo(uint32_t dwVnum, OUT uint8_t& bType, OUT uint8_t& bGrade, OUT uint8_t& bStep, OUT uint8_t& bRefine) const;
	// fixme : titempos-in
	uint16_t GetBasePosition(const LPITEM pItem) const;
	bool IsValidCellForThisItem(const LPITEM pItem, const TItemPos& Cell) const;
	int GetDuration(const LPITEM pItem) const;

	// A function that receives a dragon spirit and extracts a specific dragon heart
	bool ExtractDragonHeart(LPCHARACTER ch, LPITEM pItem, LPITEM pExtractor = nullptr);

	// Determines success or failure when removing a specific dragon spirit stone (pItem) from the equipment window,
	// A function that gives a by-product in case of failure. (By-product is defined in dragon_soul_table.txt)
	// If invalid value is entered in DestCell, if successful, dragon spirit stone is automatically added to the empty space.
	// In case of failure, dragon spirit stone (pItem) is deleted.
	// If there is an extraction item, the extraction success probability increases by pExtractor->GetValue(0)%.
	// By-products are always added automatically.
	bool PullOut(LPCHARACTER ch, TItemPos DestCell, IN OUT LPITEM& pItem, LPITEM pExtractor = nullptr);

	// Dragon Soul upgrade function
	bool DoRefineGrade(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);
	bool DoRefineStep(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);
	bool DoRefineStrength(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);
#ifdef ENABLE_DS_CHANGE_ATTR
	bool DoChangeAttr(LPCHARACTER ch, TItemPos(&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);
#endif

	bool DragonSoulItemInitialize(LPITEM pItem);

	bool IsTimeLeftDragonSoul(LPITEM pItem) const;
	int LeftTime(LPITEM pItem) const;
	bool ActivateDragonSoul(LPITEM pItem);
	bool DeactivateDragonSoul(LPITEM pItem, bool bSkipRefreshOwnerActiveState = false);
	bool IsActiveDragonSoul(LPITEM pItem) const;
#ifdef ENABLE_DS_SET
	bool GetDSSetGrade(LPCHARACTER ch, uint8_t& iSetGrade);
	int GetDSSetValue(uint8_t iAttributeIndex, uint16_t iApplyType, uint32_t iVnum, uint8_t iSetGrade); // @fixme-WL001
#endif

private:
	void SendRefineResultPacket(LPCHARACTER ch, uint8_t bSubHeader, const TItemPos& pos);

	// Function that checks the character's dragon spirit stone deck and turns off the character's dragon soul stone active state if there is no active dragon spirit stone.
	void RefreshDragonSoulState(LPCHARACTER ch);

	uint32_t MakeDragonSoulVnum(uint8_t bType, uint8_t grade, uint8_t step, uint8_t refine) noexcept;
#ifdef ENABLE_DS_CHANGE_ATTR
public:
	bool PutAttributes(LPITEM pDS);
private:
#else
	bool PutAttributes(LPITEM pDS);
#endif
	bool RefreshItemAttributes(LPITEM pItem);

	DragonSoulTable* m_pTable;
};

#endif
