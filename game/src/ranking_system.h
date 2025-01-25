#ifndef __INC_METIN_II_RANKING_SYSTEM_H__
#define __INC_METIN_II_RANKING_SYSTEM_H__

#include "../../common/CommonDefines.h"

#ifdef ENABLE_RANKING_SYSTEM
#include "packet.h"

enum __UNK__
{
	RK_CATEGORY_BF = 0,
};

enum __RE_20200724_2__
{
	SOLO_RK_CATEGORY_BF_WEAK = 0,
	SOLO_RK_CATEGORY_BF_TOTAL = 1,
	SOLO_RK_CATEGORY_MD_RED = 2,
	SOLO_RK_CATEGORY_MD_BLUE = 3,
	// UNK_4
	// UNK_5
	SOLO_RK_CATEGORY_BNW = 6,
	SOLO_RK_CATEGORY_WORLD_BOSS = 7,

	SOLO_RK_CATEGORY_MAX,
};

class CRankingSystem : public singleton<CRankingSystem>
{
public:
	CRankingSystem();
	~CRankingSystem();

	void Initialize();
	void Destroy();

public:
	void LoadRanking(uint8_t bCategory);

#ifdef ENABLE_BATTLE_FIELD
public:
	void LoadBFRanking();
	void LoadRankingBFWeek();
	void LoadRankingWeekWinners();
	void LoadRankingBFTotal();
	void SendBFRanking(LPCHARACTER pChar);
	int8_t GetBFRankingPosition(uint32_t dwPlayerID);

private:
	std::vector<TBattleRankingMember> vecBattleFieldRanking{};
	std::vector<uint32_t> vecBattleFieldWeekRankingWinners{};
#endif
};

#endif
#endif
