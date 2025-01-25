#ifndef __INC_METIN_II_GAME_SAFEBOX_H__
#define __INC_METIN_II_GAME_SAFEBOX_H__

class CHARACTER;
class CItem;
class CGrid;

class CSafebox
{
public:
#ifdef ENABLE_SAFEBOX_MONEY
	CSafebox(LPCHARACTER pkChrOwner, int iSize, uint32_t dwGold);
#else
	CSafebox(LPCHARACTER pkChrOwner, int iSize);
#endif
	~CSafebox();

	bool Add(uint32_t dwPos, LPITEM pkItem);
	LPITEM Get(uint32_t dwPos);
	LPITEM Remove(uint32_t dwPos);
	void ChangeSize(int iSize);

	bool MoveItem(uint8_t bCell, uint8_t bDestCell, uint8_t count);
	LPITEM GetItem(uint8_t bCell);

#ifdef ENABLE_SAFEBOX_MONEY
	void Save();
#endif

	bool IsEmpty(uint32_t dwPos, uint8_t bSize);
	bool IsValidPosition(uint32_t dwPos);

#ifdef ENABLE_SAFEBOX_MONEY
	uint32_t GetSafeboxMoney() const;
	void SetSafeboxMoney(uint32_t dGold);
#endif

	void SetWindowMode(uint8_t bWindowMode);

protected:
	void __Destroy();

	LPCHARACTER m_pkChrOwner;
	LPITEM m_pkItems[SAFEBOX_MAX_NUM];
	CGrid* m_pkGrid;
	int m_iSize;
#ifdef ENABLE_SAFEBOX_MONEY
	uint32_t m_lGold;
#endif

	uint8_t m_bWindowMode;
};

#endif
