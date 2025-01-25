#include "stdafx.h"

#ifdef ENABLE_ATTR_6TH_7TH
#include <unordered_map>
#include "../../common/stl.h"
#include "../../common/tables.h"

enum
{
	ATTR67_ADD_TIME_TO_WAIT = 86400, //Seconds
	MATERIAL_MAX_COUNT = 10,
	SUCCESS_PER_MATERIAL = 2,
	SUPPORT_MAX_COUNT = 5,
};

class CAttr6th7th : public singleton<CAttr6th7th>
{
public:
	CAttr6th7th() noexcept;

	void StartComb(LPCHARACTER ch, uint8_t bCell[c_SkillBook_Comb_Slot_Max]);
	bool CheckCombStart(LPCHARACTER ch, uint8_t bCell[c_SkillBook_Comb_Slot_Max]);
	void DeleteCombItems(LPCHARACTER, uint8_t bCell[c_SkillBook_Comb_Slot_Max]);

	void AddAttr6th7th(LPCHARACTER ch, uint8_t bRegistSlot, uint8_t bFragmentCount, uint16_t wCellAdditive, uint8_t bCountAdditive);
	void SetAddAttrTime(LPCHARACTER ch, int time);
	int GetAddAttrTime(const CHARACTER* ch);
	bool CheckItemAdded(LPITEM item);

	void SetTotalPercentAttr(LPCHARACTER ch, uint8_t bPercent);
	uint8_t GetPercentAttr(const CHARACTER* ch);

	void CheckFragment(LPCHARACTER ch, uint8_t bRegistSlot);
	uint32_t GetFragmentVnum(const CHARACTER* ch, LPITEM item);

	bool GetEnoughInventory(const CHARACTER* ch);
	void GetItemAttr(LPCHARACTER ch, uint8_t* bResult, uint16_t* wSlot);
	void RecvAttr67Packet(const CHARACTER* ch, uint32_t vnum);
};
#endif
