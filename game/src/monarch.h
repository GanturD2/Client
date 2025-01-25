#ifndef __INC_METIN_II_MONARCH_H__
#define __INC_METIN_II_MONARCH_H__

#include "../../common/tables.h"

class CMonarch : public singleton<CMonarch>
{
public:
	CMonarch();
	virtual ~CMonarch();

	bool Initialize();

	int HealMyEmpire(LPCHARACTER ch, uint32_t price);
	void SetMonarchInfo(TMonarchInfo* pInfo);

	bool IsMonarch(uint32_t pid, uint8_t bEmpire);
	bool IsMoneyOk(int price, uint8_t bEmpire);
	bool SendtoDBAddMoney(int Money, uint8_t bEmpire, LPCHARACTER ch);
	bool SendtoDBDecMoney(int Money, uint8_t bEmpire, LPCHARACTER ch);

	bool AddMoney(int Money, uint8_t bEmpire);
	bool DecMoney(int Money, uint8_t bEmpire);
	int GetMoney(uint8_t bEmpire);

	TMonarchInfo* GetMonarch();

	uint32_t GetMonarchPID(uint8_t Empire);

	bool IsPowerUp(uint8_t Empire);
	bool IsDefenceUp(uint8_t Empire);

	int GetPowerUpCT(uint8_t Empire)
	{
		return Empire < _countof(m_PowerUpCT) ? m_PowerUpCT[Empire] : 0;
	}

	bool CheckPowerUpCT(uint8_t Empire);
	bool CheckDefenseUpCT(uint8_t Empire);

	int GetDefenseUpCT(uint8_t Empire)
	{
		return Empire < _countof(m_DefenseUpCT) ? m_DefenseUpCT[Empire] : 0;
	}

	void PowerUp(uint8_t Empire, bool On);
	void DefenseUp(uint8_t Empire, bool On);

private:
	TMonarchInfo m_MonarchInfo;

	int m_PowerUp[4];
	int m_DefenseUp[4];

	int m_PowerUpCT[4];
	int m_DefenseUpCT[4];
};

bool IsMonarchWarpZone(int map_idx);

#endif
