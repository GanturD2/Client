#include "stdafx.h"

#ifdef ENABLE_MINI_GAME_FINDM
#include "config.h"

#include <random>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <ctime>

#include "../../common/length.h"
#include "../../common/tables.h"
#include "p2p.h"
#include "locale_service.h"
#include "char.h"
#include "utils.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "questmanager.h"
#include "questlua.h"
#include "start_position.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "regen.h"
#include "log.h"
#include "db.h"
#include "target.h"
#include "party.h"

#include "minigame_manager.h"

int CMiniGameManager::MiniGameFindM(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (!ch)
		return -1;

	const TPacketCGMiniGameFindM* p = (TPacketCGMiniGameFindM*)data;

	if (uiBytes < sizeof(TPacketCGMiniGameFindM))
		return -1;

	uiBytes -= sizeof(TPacketCGMiniGameFindM);

	switch (p->bSubheader)
	{
		case 0:
			MiniGameFindMStartGame(ch);
			return 0;

		case 1:
			MiniGameFindMClickCard(ch, p->bSubArgument);
			return 0;

		case 2:
			MiniGameFindMReward(ch, p->bSubArgument);
			return 0;

		case 3:
			MiniGameFindMUpgrade(ch, p->bSubArgument);
			return 0;

#ifdef ENABLE_MINI_GAME_FINDM_HINT
		case 4:
			MiniGameFindMStartHint(ch);
			return 0;
#endif

		default:
			sys_err("CMiniGameManager::MiniGameFindM : Unknown subheader %d : %s", p->bSubheader, ch->GetName());
			break;
	}

	return 0;
}

void CMiniGameManager::InitializeMiniGameFindM(int iEnable)
{
	if (iEnable)
	{
		SpawnEventNPC(FIND_M_NPC);
	}
	else
	{
		const auto chars = CHARACTER_MANAGER::Instance().GetCharactersByRaceNum(FIND_M_NPC);
		for (auto ch : chars)
		{
			M2_DESTROY_CHARACTER(ch);
		}
	}
}

void CMiniGameManager::MiniGameFindMStartGame(LPCHARACTER pkChar)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (pkChar->MiniGameFindMGetGameStatus() == true)
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_findm_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->GetGold() < FIND_M_PLAY_YANG)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough yang to start this game."));
		return;
	}

	if (pkChar->CountSpecifyItem(FIND_M_PLAY_ITEM) < 1)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough sets to start this game."));
		return;
	}

	pkChar->RemoveSpecifyItem(FIND_M_PLAY_ITEM, 1);
	pkChar->PointChange(POINT_GOLD, -FIND_M_PLAY_YANG);

	std::vector<uint16_t> wMonsterList
	{
		153, 154, 193, 393, 394, 454, 533, 534, 1091, 1093, 1192, 1304, 1901, 2191,
		2206, 2492, 2598, 2802, 2842, 5161, 5163, 6009, 6091, 6109, 6116, 6191, 6192
	};

	std::vector<TFindMCard> m_vecFieldCards;

	for (int i = 0; i < 9; i++)
	{
		const int randIndex = number(0, wMonsterList.size() - 1);
		const uint16_t cardValue = wMonsterList[randIndex];

		wMonsterList.erase(std::remove(wMonsterList.begin(), wMonsterList.end(), cardValue), wMonsterList.end());

		m_vecFieldCards.push_back(TFindMCard(cardValue, false));
		m_vecFieldCards.push_back(TFindMCard(cardValue, false));
		m_vecFieldCards.push_back(TFindMCard(cardValue, false));
	}

	// Shuffle the cards set
	std::random_device rd;
	std::mt19937 mt(rd());
	std::shuffle(m_vecFieldCards.begin(), m_vecFieldCards.end(), mt);

	const uint32_t dwStartTime = time(0);
	pkChar->MiniGameFindMStartGame(m_vecFieldCards, dwStartTime);
#ifdef ENABLE_MINI_GAME_FINDM_HINT
	pkChar->MiniGameFindMSetHintCount(FIND_M_DEFAULT_HIT_COUNT);
#endif
	pkChar->MiniGameFindMStartReveal(FIND_M_REVEAL_TIME);

	TPacketGCMiniGameFindM packet{};
	packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
	packet.bSubheader = SUBHEADER_GC_FIND_M_START;

	TPacketGCMiniGameFindMStart packetSecond{};
	packetSecond.dwStartTime = dwStartTime;
	packetSecond.dwNextReveal = pkChar->MiniGameFindMGetNextReveal();
#ifdef ENABLE_MINI_GAME_FINDM_HINT
	packetSecond.bHintCount = pkChar->MiniGameFindMGetHintCount();
#endif

	packet.wSize = sizeof(packet) + sizeof(packetSecond);

	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameFindM));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameFindMStart));
}

int CMiniGameManager::MiniGameFindMPickRandomCard(LPCHARACTER pkChar)
{
	std::vector<uint8_t> bCardIndex;

	for (size_t i = 0; i < pkChar->m_vecFindMFieldCards.size(); i++)
	{
		if (pkChar->m_vecFindMFieldCards[i].bIsRevealed == false)
			bCardIndex.emplace_back(i);
	}

	if (bCardIndex.empty())
		return -1;

	const int randomIndex = number(0, bCardIndex.size() - 1);
	return bCardIndex[randomIndex];
}

void CMiniGameManager::MiniGameFindMClickCard(LPCHARACTER pkChar, uint8_t bFieldPos, bool bSkipCheks)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_findm_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->MiniGameFindMGetGameStatus() == false)
		return;

	if (bFieldPos < 0 || bFieldPos > 26)
		return;

	const int iFirstCard = pkChar->iRevealedIndex[0];
	const int iSecondCard = pkChar->iRevealedIndex[1];
	const TFindMCard filedCard = pkChar->m_vecFindMFieldCards[bFieldPos];
	if (filedCard.bIsRevealed == true)
		return;

	if (!bSkipCheks)
	{
		// Set Game status to false for processing
		pkChar->MiniGameFindMSetGameStatus(false);
	}

	uint8_t bClientSend = 0;
	bool bIsTheEnd = false;
	uint32_t dwRewardVnum = 0;

	if (iFirstCard == -1 && iSecondCard == -1)
	{
		pkChar->m_vecFindMFieldCards[bFieldPos].bIsRevealed = true;
		pkChar->iRevealedIndex[0] = bFieldPos;
	}
	else if (iFirstCard != -1 && iSecondCard == -1)
	{
		pkChar->m_vecFindMFieldCards[bFieldPos].bIsRevealed = true;
		pkChar->iRevealedIndex[1] = bFieldPos;
	}
	else if (iFirstCard != -1 && iSecondCard != -1)
	{
		pkChar->m_vecFindMFieldCards[bFieldPos].bIsRevealed = true;
		pkChar->MiniGameFindMSetTryCount(pkChar->MiniGameFindMGetTryCount() + 1);

		const TFindMCard firstCard = pkChar->m_vecFindMFieldCards[iFirstCard];
		const TFindMCard secondCard = pkChar->m_vecFindMFieldCards[iSecondCard];

		if (firstCard.wMonster == secondCard.wMonster && secondCard.wMonster == filedCard.wMonster)
		{
			pkChar->iRevealedIndex[0] = -1;
			pkChar->iRevealedIndex[1] = -1;
			bClientSend = 2;

			if (pkChar->MiniGameFindMIsAllRevealed())
			{
				bIsTheEnd = true;

				pkChar->m_vecFindMFieldCards.clear();
				const uint32_t dwPlayTime = time(0) - pkChar->MiniGameFindMGetStartTime();
				const uint16_t wTryCount = pkChar->MiniGameFindMGetTryCount();

				MiniGameFindMRegisterScore(pkChar, dwPlayTime, wTryCount);

				if (wTryCount < 17)
					dwRewardVnum = FIND_M_REWARD_GOLD;
				else if (wTryCount >= 17 && wTryCount <= 24)
					dwRewardVnum = FIND_M_REWARD_SILVER;
				else if (wTryCount > 24)
					dwRewardVnum = FIND_M_REWARD_BRONZE;
			}
		}
		else
		{
			pkChar->iRevealedIndex[0] = -1;
			pkChar->iRevealedIndex[1] = -1;
			pkChar->m_vecFindMFieldCards[iFirstCard].bIsRevealed = false;
			pkChar->m_vecFindMFieldCards[iSecondCard].bIsRevealed = false;
			pkChar->m_vecFindMFieldCards[bFieldPos].bIsRevealed = false;
			bClientSend = 1;
		}

		pkChar->MiniGameFindMStartReveal(bIsTheEnd ? -1 : FIND_M_REVEAL_TIME);
	}

	TPacketGCMiniGameFindM packet{};
	packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
	packet.bSubheader = SUBHEADER_GC_FIND_M_RESULT_CLICK;

	TPacketGCMiniGameFindMResult packetSecond{};
	packetSecond.bCardIndex = bFieldPos;
	packetSecond.wMonsterVnum = filedCard.wMonster;
	packetSecond.bHideRevealed = bClientSend;

	packetSecond.dwNextReveal = pkChar->MiniGameFindMGetNextReveal();
	packetSecond.wTryCount = pkChar->MiniGameFindMGetTryCount();

	/*
		bHideRevealed
		0 = don't do nothing
		1 = hide all revealed cards
		2 = keep all revealed cards, show effect
	*/

	packet.wSize = sizeof(packet) + sizeof(packetSecond);

	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameFindM));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameFindMResult));

	if (bIsTheEnd)
	{
		pkChar->MiniGameFindMSetRewardVnum(dwRewardVnum);
		pkChar->MiniGameFindMSetTryCount(0);

		TPacketGCMiniGameFindM packetEnd{};
		packetEnd.bHeader = HEADER_GC_MINI_GAME_FIND_M;
		packetEnd.bSubheader = SUBHEADER_GC_FIND_M_REWARD;

		packetEnd.wSize = sizeof(packetEnd) + sizeof(dwRewardVnum);

		pkChar->GetDesc()->BufferedPacket(&packetEnd, sizeof(TPacketGCMiniGameFindM));
		pkChar->GetDesc()->Packet(&dwRewardVnum, sizeof(uint32_t));
	}

	if (!bSkipCheks)
	{
		// Set the game status to true so we can process more cards
		pkChar->MiniGameFindMSetGameStatus(true);
	}
}

void CMiniGameManager::MiniGameFindMReward(LPCHARACTER pkChar, uint8_t bFlag)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_findm_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->MiniGameFindMGetGameStatus() == false)
		return;

	const uint32_t dwRewardVnum = pkChar->MiniGameFindMGetRewardVnum();
	if (!dwRewardVnum)
		return;

	if (bFlag == 0)
	{
		pkChar->MiniGameFindMSetRewardVnum(0);
		pkChar->AutoGiveItem(dwRewardVnum, 1);
		pkChar->MiniGameFindMSetGameStatus(false);

		TPacketGCMiniGameFindM packet{};
		packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
		packet.bSubheader = SUBHEADER_GC_FIND_M_END;
		packet.wSize = sizeof(packet);
		pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameFindM));
	}
	else if (bFlag == 1)
	{
		switch (dwRewardVnum)
		{
		case FIND_M_REWARD_BRONZE:
			pkChar->MiniGameFindMSetRewardCost(FIND_M_BRONZE_START_COST);
			break;

		case FIND_M_REWARD_SILVER:
			pkChar->MiniGameFindMSetRewardCost(FIND_M_SILVER_START_COST);
			break;

		case FIND_M_REWARD_GOLD:
			pkChar->MiniGameFindMSetRewardCost(FIND_M_GOLD_START_COST);
			break;

		default:
			pkChar->MiniGameFindMSetRewardCost(FIND_M_GOLD_START_COST);
			break;
		}

		TPacketGCMiniGameFindM packet{};
		packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
		packet.bSubheader = SUBHEADER_GC_FIND_M_UPGRADE;

		TPacketGCMiniGameFindMUpgrade packetSecond{};
		packetSecond.dwRewardVnum = dwRewardVnum;
		packetSecond.dwNeedMoney = pkChar->MiniGameFindMGetRewardCost();

		packet.wSize = sizeof(packet) + sizeof(packetSecond);

		pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameFindM));
		pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameFindMUpgrade));
	}
}

void CMiniGameManager::MiniGameFindMUpgrade(LPCHARACTER pkChar, uint8_t bFlag)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_findm_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->MiniGameFindMGetGameStatus() == false)
		return;

	const uint32_t dwRewardVnum = pkChar->MiniGameFindMGetRewardVnum();
	if (!dwRewardVnum)
		return;

	if (bFlag == 0)
	{
		pkChar->MiniGameFindMSetRewardVnum(0);
		pkChar->MiniGameFindMSetRewardCost(0);

		pkChar->AutoGiveItem(dwRewardVnum, 1);
		pkChar->MiniGameFindMSetGameStatus(false);

		TPacketGCMiniGameFindM packet{};
		packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
		packet.bSubheader = SUBHEADER_GC_FIND_M_END;
		packet.wSize = sizeof(packet);
		pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameFindM));
	}
	else if (bFlag == 1)
	{
		uint32_t dwNewRewardVnum = 0;
		switch (dwRewardVnum)
		{
		case FIND_M_REWARD_BRONZE:
			dwNewRewardVnum = FIND_M_REWARD_SILVER;
			break;

		case FIND_M_REWARD_SILVER:
			dwNewRewardVnum = FIND_M_REWARD_GOLD;
			break;

		case FIND_M_REWARD_GOLD:
			dwNewRewardVnum = FIND_M_REWARD_VIP;
			break;

		default:
			break;
		}

		if (!dwNewRewardVnum)
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This is already the best reward."));
			return;
		}

		const uint32_t dwUpgradeCost = pkChar->MiniGameFindMGetRewardCost();
		if (pkChar->GetGold() < static_cast<int>(dwUpgradeCost))
		{
			pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough Yang."));
			return;
		}

		if (dwUpgradeCost * 2 > GOLD_MAX)
			return;

		pkChar->PointChange(POINT_GOLD, -static_cast<int>(dwUpgradeCost));
		pkChar->MiniGameFindMSetRewardCost(dwUpgradeCost * 2);

		const int prob = number(1, 100);
		if (prob > (100 - FIND_M_UPGRADE_CHANCE)) // >70 not <30
		{
			pkChar->MiniGameFindMSetRewardVnum(dwNewRewardVnum);
		}

		TPacketGCMiniGameFindM packet{};
		packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
		packet.bSubheader = SUBHEADER_GC_FIND_M_UPGRADE;

		TPacketGCMiniGameFindMUpgrade packetSecond{};
		packetSecond.dwRewardVnum = pkChar->MiniGameFindMGetRewardVnum();
		packetSecond.dwNeedMoney = pkChar->MiniGameFindMGetRewardCost();

		packet.wSize = sizeof(packet) + sizeof(packetSecond);

		pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameFindM));
		pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameFindMUpgrade));
	}
}

#ifdef ENABLE_MINI_GAME_FINDM_HINT
void CMiniGameManager::MiniGameFindMStartHint(LPCHARACTER pkChar)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_findm_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->MiniGameFindMGetGameStatus() == false)
		return;

	const uint8_t bHintCount = pkChar->MiniGameFindMGetHintCount();
	if (!bHintCount)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("No more hints for you."));
		return;
	}

	const uint8_t bCardNeed = (bHintCount == 2) ? 3 : 5;

	if (pkChar->CountSpecifyItem(FIND_M_DROP_ITEM) < bCardNeed)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough cards to receive a hint."));
		return;
	}

	pkChar->RemoveSpecifyItem(FIND_M_DROP_ITEM, bCardNeed);
	pkChar->MiniGameFindMSetHintCount(pkChar->MiniGameFindMGetHintCount() - 1);
	pkChar->MiniGameFindMStartReveal(FIND_M_REVEAL_TIME);

	TPacketGCMiniGameFindM packet{};
	packet.bHeader = HEADER_GC_MINI_GAME_FIND_M;
	packet.bSubheader = SUBHEADER_GC_FIND_M_HINT;

	TPacketGCMiniGameFindMHint packetSecond{};
	packetSecond.bHintCount = pkChar->MiniGameFindMGetHintCount();
	packetSecond.dwNextReveal = pkChar->MiniGameFindMGetNextReveal();

	for (int i = 0; i < 27; i++)
	{
		packetSecond.wMonsterVnum[i] = 0;

		if (i >= pkChar->m_vecFindMFieldCards.size())
			continue;

		packetSecond.wMonsterVnum[i] = pkChar->m_vecFindMFieldCards[i].wMonster;
	}

	packet.wSize = sizeof(packet) + sizeof(packetSecond);

	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameFindM));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameFindMHint));
}
#endif

void CMiniGameManager::MiniGameFindMRegisterScore(LPCHARACTER pkChar, uint32_t dwSeconds, uint16_t wTry)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	char querySelect[256] = { '\0' };

	snprintf(&querySelect[0], sizeof(querySelect), "SELECT play_time, try_count FROM log.find_m_event WHERE name = '%s' LIMIT 1;", pkChar->GetName());

	auto pSelectMsg(DBManager::Instance().DirectQuery(&querySelect[0]));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		uint32_t dwPlayTime = 0;
		uint16_t wTryCount = 0;
		const MYSQL_ROW& row = mysql_fetch_row(resSelect->pSQLResult);
		str_to_number(dwPlayTime, row[0]);
		str_to_number(wTryCount, row[1]);

		if (dwSeconds < dwPlayTime)
		{
			if (wTry < wTryCount)
				DBManager::Instance().DirectQuery("UPDATE log.find_m_event SET play_time = %d, try_count = %d WHERE name = '%s';", dwSeconds, wTry, pkChar->GetName());
			else
				DBManager::Instance().DirectQuery("UPDATE log.find_m_event SET play_time = %d WHERE name = '%s';", dwSeconds, pkChar->GetName());
		}
		else
		{
			if (wTry < wTryCount)
				DBManager::Instance().DirectQuery("UPDATE log.find_m_event SET try_count = %d WHERE name = '%s';", wTry, pkChar->GetName());
		}
	}
	else
	{
		DBManager::Instance().DirectQuery("REPLACE INTO log.find_m_event (name, empire, play_time, try_count) VALUES ('%s', %d, %d, %d);",
			pkChar->GetName(), pkChar->GetEmpire(), dwSeconds, wTry);
	}
}

int CMiniGameManager::MiniGameFindMGetScore(lua_State* L, bool isTry)
{
	uint32_t index = 1;
	lua_newtable(L);

	char querySelect[256] = { '\0' };

	if (isTry)
		snprintf(&querySelect[0], sizeof(querySelect), "SELECT name, empire, try_count FROM log.find_m_event ORDER BY try_count ASC LIMIT 10;");
	else
		snprintf(&querySelect[0], sizeof(querySelect), "SELECT name, empire, play_time FROM log.find_m_event ORDER BY play_time ASC LIMIT 10;");

	auto pSelectMsg(DBManager::Instance().DirectQuery(&querySelect[0]));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (uint i = 0; i < resSelect->uiNumRows; i++)
		{
			const MYSQL_ROW& row = mysql_fetch_row(resSelect->pSQLResult);
			uint8_t bEmpire = 0;
			uint32_t dwArg = 0;

			str_to_number(bEmpire, row[1]);
			str_to_number(dwArg, row[2]);

			lua_newtable(L);

			lua_pushstring(L, row[0]);
			lua_rawseti(L, -2, 1);

			lua_pushnumber(L, bEmpire);
			lua_rawseti(L, -2, 2);

			lua_pushnumber(L, dwArg);
			lua_rawseti(L, -2, 3);

			lua_rawseti(L, -2, index++);
		}
	}

	return 0;
}

void CMiniGameManager::MiniGameFindMCheckEnd()
{
	if (time(0) > iFindEndTime && iFindEndTime != 0)
	{
		quest::CQuestManager::Instance().RequestSetEventFlag("mini_game_findm_event", 0);
		iFindEndTime = 0;
	}
}
#endif
