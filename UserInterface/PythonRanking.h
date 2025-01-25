#pragma once
#include "Packet.h"

class CPythonRanking : public CSingleton<CPythonRanking>
{
public:
	typedef std::vector<TBattleRankingMember> TBattleFieldRanking;
public:
	CPythonRanking();
	~CPythonRanking();

	void ReceiveBattleFieldRanking(const TBattleRankingMember& TInfo);
	const TBattleFieldRanking& GetBattleFieldRanking();
	void ClearBFRanking();
private:
	TBattleFieldRanking battleFieldRanking;
};