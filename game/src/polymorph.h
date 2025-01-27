
#ifndef __POLYMORPH_UTILS__
#define __POLYMORPH_UTILS__

#include <unordered_map>

#define POLYMORPH_SKILL_ID 129
#define POLYMORPH_BOOK_ID 50322

enum POLYMORPH_BONUS_TYPE
{
	POLYMORPH_NO_BONUS,
	POLYMORPH_ATK_BONUS,
	POLYMORPH_DEF_BONUS,
	POLYMORPH_SPD_BONUS,
};

class CPolymorphUtils : public singleton<CPolymorphUtils>
{
private:
	std::unordered_map<uint32_t, uint32_t> m_mapSPDType;
	std::unordered_map<uint32_t, uint32_t> m_mapATKType;
	std::unordered_map<uint32_t, uint32_t> m_mapDEFType;

public:
	CPolymorphUtils();

	POLYMORPH_BONUS_TYPE GetBonusType(uint32_t dwVnum);

	bool PolymorphCharacter(LPCHARACTER pChar, LPITEM pItem, const CMob* pMob);
	bool UpdateBookPracticeGrade(LPCHARACTER pChar, LPITEM pItem);
	bool GiveBook(LPCHARACTER pChar, uint32_t dwMobVnum, uint32_t dwPracticeCount, uint8_t BookLevel, uint8_t LevelLimit);
	bool BookUpgrade(LPCHARACTER pChar, LPITEM pItem);
	bool IsOnPolymorphMapBlacklist(uint32_t mapIndex); //@custom002
};

#endif /*__POLYMORPH_UTILS__*/
