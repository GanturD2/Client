#ifndef __INC_METIN_II_GAME_EXCHANGE_H__
#define __INC_METIN_II_GAME_EXCHANGE_H__

class CGrid;

enum EExchangeValues
{
	EXCHANGE_ITEM_MAX_NUM = EXCHANGE_PAGE_SIZE,
	EXCHANGE_MAX_DISTANCE = 1000
};

class CExchange
{
public:
	CExchange(LPCHARACTER pOwner);
	~CExchange();

	bool Accept(bool bIsAccept = true);
	void Cancel();

#ifdef ENABLE_CHEQUE_SYSTEM
	bool AddGold(long lGold, uint32_t lCheque);
#else
	bool AddGold(long lGold);
#endif
	bool AddItem(TItemPos item_pos, uint8_t display_pos);
	bool RemoveItem(uint8_t pos);

	LPCHARACTER GetOwner() { return m_pOwner; }
	CExchange* GetCompany() { return m_pCompany; }

	bool GetAcceptStatus() { return m_bAccept; }

	void SetCompany(CExchange* pExchange) { m_pCompany = pExchange; }

private:
	bool Done();
	bool Check(int* piItemCount);
	bool CheckSpace();

private:
	CExchange* m_pCompany;

	LPCHARACTER m_pOwner;

	TItemPos m_aItemPos[EXCHANGE_ITEM_MAX_NUM];
	LPITEM m_apItems[EXCHANGE_ITEM_MAX_NUM];
	uint8_t m_abItemDisplayPos[EXCHANGE_ITEM_MAX_NUM];

	bool m_bAccept;
	long m_lGold;
#ifdef ENABLE_CHEQUE_SYSTEM
	uint32_t m_lCheque;
#endif

	CGrid* m_pGrid;
};

#endif
