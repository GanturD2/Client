#include "stdafx.h"

#ifdef ENABLE_MINI_GAME_BNW
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
#include "shop.h"
#include "shop_manager.h"

#include "minigame_manager.h"

int CMiniGameManager::MiniGameBNW(LPCHARACTER ch, const char* data, size_t uiBytes)
{
	if (!ch)
		return -1;

	const TPacketCGMiniGameBNW* p = (TPacketCGMiniGameBNW*)data;

	if (uiBytes < sizeof(TPacketCGMiniGameBNW))
		return -1;

	uiBytes -= sizeof(TPacketCGMiniGameBNW);

	sys_log(0, "CMiniGameManager::MiniGameBNW : Subheader %d - SubArgument %d", p->bSubheader, p->bSubArgument);

	switch (p->bSubheader)
	{
		case 0:
			MiniGameBNWStartGame(ch);
			return 0;

		case 1:
			MiniGameBNWCardClick(ch, p->bSubArgument);
			return 0;

		case 2:
			MiniGameBNWOpenShop(ch);
			return 0;

		case 3:
			MiniGameBNWOpenRanking(ch);
			return 0;

		case 4:
			MiniGameBNWDonateRanking(ch);
			return 0;

		default:
			sys_err("CMiniGameManager::MiniGameBNW : Unknown subheader %d : %s", p->bSubheader, ch->GetName());
			break;
	}

	return 0;
}

void CMiniGameManager::MiniGameBNWStartGame(LPCHARACTER pkChar)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (pkChar->MiniGameBNWGetGameStatus() == true)
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_bnw_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->GetGold() < BNW_PLAY_YANG)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough yang to start this game."));
		return;
	}

	if (pkChar->CountSpecifyItem(BNW_PLAY_ITEM) < 1)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You do not have enough sets to start this game."));
		return;
	}

	pkChar->RemoveSpecifyItem(BNW_PLAY_ITEM, 1);
	pkChar->PointChange(POINT_GOLD, -BNW_PLAY_YANG);

	pkChar->m_vecBNWPlayerCards.clear();
	pkChar->m_vecBNWOpponentCards.clear();

	for (uint8_t i = 0; i < BNW_CARD_MAX_NUM; i++)
	{
		pkChar->m_vecBNWPlayerCards.push_back(TBNWCard(i, false));
		pkChar->m_vecBNWOpponentCards.push_back(TBNWCard(i, false));
	}

	// Shuffle the cards set
	// std::random_device rd;
	// std::mt19937 mt(rd());
	// std::shuffle(pkChar->m_vecBNWOpponentCards.begin(), pkChar->m_vecBNWOpponentCards.end(), mt);

	// Player: Index = card number
	// Opponent: Index != card number

	pkChar->MiniGameBNWSetGameStatus(true);

	TPacketGCMiniGameBNW packet{};
	packet.bHeader = HEADER_GC_MINI_GAME_BNW;
	packet.bSubheader = SUBHEADER_GC_BNW_START;
	packet.wSize = sizeof(packet);
	pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameBNW));
}

uint8_t CMiniGameManager::MiniGameBNWGetRandomPos(LPCHARACTER pkChar)
{
	std::vector<uint8_t> bPositions;

	for (size_t i = 0; i < pkChar->m_vecBNWOpponentCards.size(); i++)
	{
		if (!pkChar->m_vecBNWOpponentCards[i].bIsUsed)
			bPositions.emplace_back(i);
	}

	const uint8_t randomPos = static_cast<uint8_t>(number(0, bPositions.size() - 1));
	return bPositions[randomPos];
}

bool CMiniGameManager::IsGameEnd(LPCHARACTER pkChar)
{
	bool isEnd = true;

	for (size_t i = 0; i < pkChar->m_vecBNWPlayerCards.size(); i++)
	{
		if (!pkChar->m_vecBNWPlayerCards[i].bIsUsed)
		{
			isEnd = false;
		}
	}

	for (size_t i = 0; i < pkChar->m_vecBNWOpponentCards.size(); i++)
	{
		if (!pkChar->m_vecBNWOpponentCards[i].bIsUsed)
		{
			isEnd = false;
		}
	}

	return isEnd;
}

void CMiniGameManager::MiniGameBNWCardClick(LPCHARACTER pkChar, uint8_t bCardPos)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (pkChar->MiniGameBNWGetGameStatus() == false)
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_bnw_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (bCardPos >= BNW_CARD_MAX_NUM)
		return;

	const TBNWCard clickedCard = pkChar->m_vecBNWPlayerCards[bCardPos];

	if (clickedCard.bIsUsed)
		return;

	const uint8_t bOpponentPos = MiniGameBNWGetRandomPos(pkChar);

	if (bOpponentPos >= BNW_CARD_MAX_NUM)
		return;

	const TBNWCard opponentCard = pkChar->m_vecBNWOpponentCards[bOpponentPos];

	if (opponentCard.bIsUsed)
		return;

	pkChar->m_vecBNWPlayerCards[bCardPos].bIsUsed = true;
	pkChar->m_vecBNWOpponentCards[bOpponentPos].bIsUsed = true;

	char cWinIndex = -1;
	if (clickedCard.bIndex < opponentCard.bIndex)
	{
		cWinIndex = 0;
		pkChar->MiniGameBNWSetOpponentScore(pkChar->MiniGameBNWGetOpponentScore() + 1);
	}
	else if (clickedCard.bIndex > opponentCard.bIndex)
	{
		cWinIndex = 1;
		pkChar->MiniGameBNWSetPlayerScore(pkChar->MiniGameBNWGetPlayerScore() + 1);
	}
	else
	{
		cWinIndex = 2;
	}

	const bool bIsGameEnd = IsGameEnd(pkChar);
	if (bIsGameEnd)
	{
		{
			if (pkChar->MiniGameBNWGetOpponentScore() < pkChar->MiniGameBNWGetPlayerScore())
				cWinIndex = 3;
			else if (pkChar->MiniGameBNWGetOpponentScore() > pkChar->MiniGameBNWGetPlayerScore())
				cWinIndex = 4;
			else
				cWinIndex = 5;
		}

		pkChar->AutoGiveItem(BNW_REWARD_VNUM, pkChar->MiniGameBNWGetPlayerScore());
	}

	TPacketGCMiniGameBNW packet{};
	packet.bHeader = HEADER_GC_MINI_GAME_BNW;
	packet.bSubheader = SUBHEADER_GC_BNW_COMPARE;

	TPacketGCMiniGameBNWCompare packetSecond{};
	packetSecond.cWinIndex = cWinIndex;
	packetSecond.bClickedIndex = bCardPos;
	packetSecond.bOpponentIndex = opponentCard.bIndex;
	packetSecond.bMyScore = pkChar->MiniGameBNWGetPlayerScore();

	packet.wSize = sizeof(packet) + sizeof(packetSecond);

	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameBNW));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameBNWCompare));

	if (bIsGameEnd)
	{
		// Clear data for the next game
		pkChar->MiniGameBNWSetGameStatus(false);
		pkChar->MiniGameBNWSetPlayerScore(0);
		pkChar->MiniGameBNWSetOpponentScore(0);
		pkChar->m_vecBNWPlayerCards.clear();
		pkChar->m_vecBNWOpponentCards.clear();
	}
}

void CMiniGameManager::MiniGameBNWOpenShop(LPCHARACTER pkChar)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_bnw_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->GetExchange()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		|| pkChar->GetMyShop()
		|| pkChar->GetViewingShopOwner()
#else
		|| pkChar->GetMyShop()
		|| pkChar->GetShopOwner()
#endif
		|| pkChar->IsOpenSafebox() || pkChar->IsCubeOpen()
#ifdef ENABLE_ATTR_6TH_7TH
		|| pkChar->IsOpenSkillBookComb()
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	//	|| pkChar->IsOpenAttr67Dialog()
#endif
#ifdef ENABLE_AURA_SYSTEM
		|| pkChar->IsAuraRefineWindowOpen()
#endif
		|| pkChar->IsStun() || pkChar->IsDead())
	{
		return;
	}

	LPSHOP shop = CShopManager::Instance().Get(103);
	if (shop)
	{
		shop->AddGuest(pkChar, 0, false);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		pkChar->SetViewingShopOwner(nullptr);
#else
		pkChar->SetShopOwner(nullptr);
#endif
	}
}

void CMiniGameManager::MiniGameBNWOpenRanking(LPCHARACTER pkChar)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_bnw_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	char querySelect[256] = { '\0' };
	snprintf(&querySelect[0], sizeof(querySelect), "SELECT p.name, b.donations FROM log.bnw_ranking b "
		"INNER JOIN player.player p ON p.id = b.player_id "
		"ORDER BY b.donations DESC, b.last_donation DESC LIMIT 10");

	auto pSelectMsg(DBManager::Instance().DirectQuery(&querySelect[0]));
	SQLResult* resSelect = pSelectMsg->Get();

	std::vector<TBNWRankPlayer> vec_RankingList;
	TBNWRankPlayer kTempRank{};

	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (uint i = 0; i < resSelect->uiNumRows; i++)
		{
			const MYSQL_ROW& row = mysql_fetch_row(resSelect->pSQLResult);

			kTempRank.bPos = i + 1;
			strlcpy(&kTempRank.szName[0], row[0], sizeof(kTempRank.szName));
			str_to_number(kTempRank.wDonationsCount, row[1]);

			vec_RankingList.push_back(kTempRank);
		}
	}

	char querySetMy[24] = { '\0' };
	snprintf(&querySetMy[0], sizeof(querySetMy), "SET @pos_%d = 0;", pkChar->GetPlayerID());
	auto pSetPos0(DBManager::Instance().DirectQuery(&querySetMy[0]));

	char queryMyData[256] = { '\0' };
	snprintf(&queryMyData[0], sizeof(queryMyData), "SELECT * FROM (SELECT (@pos_%d:=@pos_%d+1), donations, player_id FROM log.bnw_ranking ORDER BY donations DESC) A "
		"WHERE player_id = %d LIMIT 1", pkChar->GetPlayerID(), pkChar->GetPlayerID(), pkChar->GetPlayerID());

	auto pMyDataMsg(DBManager::Instance().DirectQuery(&queryMyData[0]));
	SQLResult* resMyData = pMyDataMsg->Get();
	if (resMyData && resMyData->uiNumRows > 0)
	{
		const MYSQL_ROW& row = mysql_fetch_row(resMyData->pSQLResult);

		str_to_number(kTempRank.bPos, row[0]);
		strlcpy(&kTempRank.szName[0], pkChar->GetName(), sizeof(kTempRank.szName));
		kTempRank.bIsMain = 1;
		str_to_number(kTempRank.wDonationsCount, row[1]);

		vec_RankingList.push_back(kTempRank);
	}

	if (vec_RankingList.empty())
	{
		TPacketGCMiniGameBNW packet{};
		packet.bHeader = HEADER_GC_MINI_GAME_BNW;
		packet.bSubheader = SUBHEADER_GC_BNW_RANKING;
		packet.wSize = sizeof(packet);
		pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameBNW));
	}
	else
	{
		TPacketGCMiniGameBNW packet{};
		packet.bHeader = HEADER_GC_MINI_GAME_BNW;
		packet.bSubheader = SUBHEADER_GC_BNW_RANKING;
		packet.wSize = static_cast<uint16_t>(sizeof(packet) + sizeof(TBNWRankPlayer) * vec_RankingList.size());

		pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameBNW));
		pkChar->GetDesc()->Packet(&vec_RankingList[0], sizeof(TBNWRankPlayer) * vec_RankingList.size());
	}
}

void CMiniGameManager::MiniGameBNWDonateRanking(LPCHARACTER pkChar)
{
	if (pkChar == nullptr)
		return;

	if (!pkChar->GetDesc())
		return;

	if (quest::CQuestManager::Instance().GetEventFlag("mini_game_bnw_event") == 0)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This event is not currently active."));
		return;
	}

	if (pkChar->CountSpecifyItem(BNW_REWARD_VNUM) < BNW_DONATION_PRICE)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You need %d Seer Coins to be able to make a donation."), BNW_DONATION_PRICE);
		return;
	}

	pkChar->RemoveSpecifyItem(BNW_REWARD_VNUM, BNW_DONATION_PRICE);

	char querySelect[256] = { '\0' };
	snprintf(&querySelect[0], sizeof(querySelect), "SELECT donations FROM log.bnw_ranking WHERE player_id = %d LIMIT 1;", pkChar->GetPlayerID());

	auto pSelectMsg(DBManager::Instance().DirectQuery(&querySelect[0]));

	const SQLResult* resSelect = pSelectMsg->Get();
	if (resSelect && resSelect->uiNumRows > 0)
	{
		auto pUpdateMsg(
			DBManager::Instance().DirectQuery("UPDATE log.bnw_ranking SET donations = donations + 1, last_donation = NOW() WHERE player_id = %d;", pkChar->GetPlayerID())
		);
	}
	else
	{
		auto pReplaceMsg(
			DBManager::Instance().DirectQuery("REPLACE INTO log.bnw_ranking (player_id, donations, last_donation) VALUES (%d, 1, NOW());", pkChar->GetPlayerID())
		);
	}

	TPacketGCMiniGameBNW packet{};
	packet.bHeader = HEADER_GC_MINI_GAME_BNW;
	packet.bSubheader = SUBHEADER_GC_BNW_UPDATE_RANKING;
	packet.wSize = sizeof(packet);
	pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameBNW));
}
#endif
