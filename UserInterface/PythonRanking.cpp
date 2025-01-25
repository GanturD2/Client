#include "StdAfx.h"
#include "PythonRanking.h"

CPythonRanking::CPythonRanking()
{
	battleFieldRanking.clear();
}

CPythonRanking::~CPythonRanking()
{
	battleFieldRanking.clear();
}

void CPythonRanking::ReceiveBattleFieldRanking(const TBattleRankingMember& TInfo)
{
	battleFieldRanking.emplace_back(TInfo);
}

const CPythonRanking::TBattleFieldRanking& CPythonRanking::GetBattleFieldRanking()
{
	return battleFieldRanking;
}

void CPythonRanking::ClearBFRanking()
{
	battleFieldRanking.clear();
}


