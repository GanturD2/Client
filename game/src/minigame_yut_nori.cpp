#include "stdafx.h"

#ifdef ENABLE_MINI_GAME_YUTNORI
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

int CMiniGameManager::MiniGameYutNori(LPCHARACTER ch, const char * data, size_t uiBytes)
{
	TPacketCGMiniGameYutNori * p = (TPacketCGMiniGameYutNori *) data;

	if (uiBytes < sizeof(TPacketCGMiniGameYutNori))
		return -1;

	data + sizeof(TPacketCGMiniGameYutNori);
	uiBytes -= sizeof(TPacketCGMiniGameYutNori);

	switch (p->bSubheader)
	{
		case 0:
			MiniGameYutNoriStartGame(ch);
			return 0;
			
		case 1:
			MiniGameYutNoriYutThrow(ch, p->bSubArgument);
			return 0;
		
		case 2:
			MiniGameYutNoriComAction(ch);
			return 0;
			
		case 3:
			MiniGameYutNoriCharClick(ch, p->bSubArgument);
			return 0;
			
		case 4:
			MiniGameYutNoriYutMove(ch, p->bSubArgument);
			return 0;
			
		case 5:
			MiniGameYutNoriGiveup(ch);
			return 0;
			
		case 6:
			MiniGameYutNoriReward(ch);
			return 0;
			
		case 7:
			MiniGameYutNoriProb(ch, p->bSubArgument);
			return 0;
			
		default:
			sys_err("CMiniGameManager::MiniGameYutNori : Unknown subheader %d : %s", p->bSubheader, ch->GetName());
			break;
	}

	return 0;
}

void CMiniGameManager::InitializeMiniGameYutNori(int iEnable)
{
	if(iEnable)
	{
		SpawnEventNPC(20502);
	}
	else
	{
		CharacterVectorInteractor i;
		CHARACTER_MANAGER::instance().GetCharactersByRaceNum(20502);

		for (CharacterVectorInteractor::iterator it = i.begin(); it != i.end(); it++)
		{
			M2_DESTROY_CHARACTER(*it);
		}
	}
}

bool CMiniGameManager::MiniGameYutNoriIsValidGame(LPCHARACTER pkChar, bool bSkipGameStatus)
{
	if(pkChar == NULL)
		return false;
	
	if(!pkChar->GetDesc())
		return false;
	
	if(!bSkipGameStatus && pkChar->YutNoriGetGameStatus() == true)
		return false;
	
	if(quest::CQuestManager::instance().GetEventFlag("mini_game_yutnori_event") == 0)
		return false;
	
	return true;
}

void CMiniGameManager::MiniGameYutNoriStartGame(LPCHARACTER pkChar)
{
	if(!MiniGameYutNoriIsValidGame(pkChar, false))
		return;
	
	if (pkChar->GetGold() < YUT_NORI_PLAY_YANG)
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, "You don't have enought money to play a game.");
		return;
	}
	
	if (!pkChar->CountSpecifyItem(YUT_NORI_PLAY_ITEM))
	{
		pkChar->ChatPacket(CHAT_TYPE_INFO, "You don't have an okey card to play a game.");
		return;
	}
	
	pkChar->RemoveSpecifyItem(YUT_NORI_PLAY_ITEM, 1);
#ifdef ENABLE_EXTEND_GOLD_STASH
	pkChar->ChangeGold(-(long long)YUT_NORI_PLAY_YANG);
#else
	pkChar->PointChange(POINT_GOLD, -YUT_NORI_PLAY_YANG);
#endif	
//////////////////////////////////////////////////////////////////////////////////
	pkChar->YutNoriSetPcTurn(true);
	pkChar->YutNoriSetStartThrow(true);
	pkChar->YutNoriSetGameStatus(true);
	
	pkChar->YutNoriSetScore(250);
	pkChar->YutNoriSetRemainCount(20);
	
	pkChar->YutNoriSetThrowYut(-1, YUT_NORI_PLAYER);
	pkChar->YutNoriSetThrowYut(-1, YUT_NORI_COMPUTER);
	
	pkChar->YutNoriSetWhoIsNextCom(0);
	pkChar->YutNoriSetProbIndex(YUT_NORI_YUTSEM1);
	
	pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 0);
	pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 1);
	pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 0);
	pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 1);
	
	pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 0);
	pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 1);
	pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 0);
	pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 1);
//////////////////////////////////////////////////////////////////////////////////
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_START_GAME;
	packet.wSize = sizeof(packet);
	pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameYutNori));
}

void CMiniGameManager::MiniGameYutNoriSendThrow(LPCHARACTER pkChar, bool bIsPc, char cYut)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_THROW_YUT;
	
	TPacketGCMiniGameYutNoriThrow packetSecond;
	packetSecond.bIsPc = bIsPc;
	packetSecond.cYut = cYut;

	packet.wSize = sizeof(packet) + sizeof(packetSecond);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameYutNoriThrow));
}

void CMiniGameManager::MiniGameYutNoriSendNextTurn(LPCHARACTER pkChar, bool bPcTurn, uint8_t bState)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_NEXT_TURN;
	
	TPacketGCMiniGameYutNoriNextTurn packetSecond;
	packetSecond.bPcTurn = bPcTurn;
	packetSecond.bState = bState;

	packet.wSize = sizeof(packet) + sizeof(packetSecond);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameYutNoriNextTurn));
}

void CMiniGameManager::MiniGameYutNoriSendMove(LPCHARACTER pkChar, bool bIsPc, uint8_t bUnitIndex, bool bIsCatch, uint8_t bStartIndex, uint8_t bDestIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_YUT_MOVE;
	
	TPacketGCMiniGameYutNoriMove packetSecond;
	packetSecond.bIsPc = bIsPc;
	packetSecond.bUnitIndex = bUnitIndex;
	packetSecond.bIsCatch = bIsCatch;
	packetSecond.bStartIndex = bStartIndex;
	packetSecond.bDestIndex = bDestIndex;

	packet.wSize = sizeof(packet) + sizeof(packetSecond);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameYutNoriMove));
}

void CMiniGameManager::MiniGameYutNoriSendAvailableArea(LPCHARACTER pkChar, uint8_t bPlayerIndex, uint8_t bAvailableIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_SHOW_AVAILABLE_AREA;
	
	TPacketGCMiniGameYutNoriAvailableArea packetSecond;
	packetSecond.bPlayerIndex = bPlayerIndex;
	packetSecond.bAvailableIndex = bAvailableIndex;

	packet.wSize = sizeof(packet) + sizeof(packetSecond);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameYutNoriAvailableArea));
}

void CMiniGameManager::MiniGameYutNoriSendPushCatchYut(LPCHARACTER pkChar, bool bIsPc, uint8_t bUnitIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_PUSH_CATCH_YUT;
	
	TPacketGCMiniGameYutNoriPushCatchYut packetSecond;
	packetSecond.bIsPc = bIsPc;
	packetSecond.bUnitIndex = bUnitIndex;

	packet.wSize = sizeof(packet) + sizeof(packetSecond);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameYutNoriPushCatchYut));
}

void CMiniGameManager::MiniGameYutNoriSendSetProb(LPCHARACTER pkChar, uint8_t bProbIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_SET_PROB;

	packet.wSize = sizeof(packet) + sizeof(bProbIndex);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&bProbIndex, sizeof(uint8_t));
}

void CMiniGameManager::MiniGameYutNoriSendSetScore(LPCHARACTER pkChar, uint16_t wScore)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_SET_SCORE;

	packet.wSize = sizeof(packet) + sizeof(wScore);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&wScore, sizeof(uint16_t));
}

void CMiniGameManager::MiniGameYutNoriSendSetRemainCount(LPCHARACTER pkChar, uint8_t bRemainCount)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_SET_REMAIN_COUNT;

	packet.wSize = sizeof(packet) + sizeof(bRemainCount);
	
	pkChar->GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameYutNori));
	pkChar->GetDesc()->Packet(&bRemainCount, sizeof(uint8_t));
}

void CMiniGameManager::MiniGameYutNoriYutThrow(LPCHARACTER pkChar, uint8_t bIsPc)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	if(bIsPc && !pkChar->YutNoriIsPcTurn())
		return;
	
	if(!bIsPc && pkChar->YutNoriIsPcTurn())
		return;
	
	if(pkChar->YutNoriIsStartThrow())
	{
		char cRanomYut = MiniGameYutNoriGetRandomYut(false, true, YUT_NORI_YUTSEM_MAX);
		
		if(bIsPc)
		{
			pkChar->YutNoriSetPcTurn(false);
			pkChar->YutNoriSetThrowYut(cRanomYut, YUT_NORI_PLAYER);
			
			MiniGameYutNoriSendThrow(pkChar, true, cRanomYut);
			MiniGameYutNoriSendNextTurn(pkChar, false, YUTNORI_BEFORE_TURN_SELECT);
		}
		else
		{
			char cPcYut = pkChar->YutNoriGetThrowYut(YUT_NORI_PLAYER);
			if(cPcYut == -1)
			{
				pkChar->ChatPacket(CHAT_TYPE_INFO, "Something went wrong. Pc yut value is invalid and have nothing to compare to.");
				return;
			}
			
			bool bYouWon = (cPcYut < cRanomYut) ? true : false;
			
			pkChar->YutNoriSetStartThrow(false);
			pkChar->YutNoriSetPcTurn(bYouWon);

			MiniGameYutNoriSendThrow(pkChar, false, cRanomYut);
			MiniGameYutNoriSendNextTurn(pkChar, bYouWon, YUTNORI_AFTER_TURN_SELECT);
		}
	}
	else
	{
		bool bIsRethrow = false;
		if(pkChar->YutNoriIsReThrow())
		{
			pkChar->YutNoriSetReThrow(false);
			bIsRethrow = true;
		}	
		
		if(bIsPc && !bIsRethrow)
		{
			pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() - YUT_NORI_IN_DE_CREASE_SCORE));
			MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
			
			pkChar->YutNoriSetRemainCount(MAX(0, pkChar->YutNoriGetRemainCount() - 1));
			MiniGameYutNoriSendSetRemainCount(pkChar, pkChar->YutNoriGetRemainCount());
		}
		
		char cSearchPlayer = bIsPc ? YUT_NORI_PLAYER : YUT_NORI_COMPUTER;
		bool bExclude6 = ((pkChar->YutNoriGetUnitPos(cSearchPlayer, 0) == YUT_NORI_START_POS - 1) || 
						(pkChar->YutNoriGetUnitPos(cSearchPlayer, 1) == YUT_NORI_START_POS - 1)) ? true : false;
			
		//char cRanomYut = number(YUT_NORI_YUTSEM1, bExclude6 ? (bIsRethrow ? YUT_NORI_YUTSEM3 : YUT_NORI_YUTSEM5) : YUT_NORI_YUTSEM6);
		char cRanomYut = MiniGameYutNoriGetRandomYut(bIsRethrow, bExclude6, bIsPc ? pkChar->YutNoriGetProbIndex() : YUT_NORI_YUTSEM_MAX);
		
		char cLastThrowYut = pkChar->YutNoriGetThrowYut(bIsPc ? YUT_NORI_PLAYER : YUT_NORI_COMPUTER);
		while(cRanomYut == cLastThrowYut) // Don't do same Yut two times
		{
			cRanomYut = MiniGameYutNoriGetRandomYut(bIsRethrow, bExclude6, bIsPc ? pkChar->YutNoriGetProbIndex() : YUT_NORI_YUTSEM_MAX);
		}

		if(cRanomYut == YUT_NORI_YUTSEM6)
		{
			char cCompareFirstPos = bIsPc ? pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) : pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0);
			char cCompareSecondPos = bIsPc ? pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 1) : pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 1);
			
			bool bCanMove = ((cCompareFirstPos == YUT_NORI_START_POS || cCompareFirstPos == YUT_NORI_END_POS) && 
							(cCompareSecondPos == YUT_NORI_START_POS || cCompareSecondPos == YUT_NORI_END_POS)) ? false : true;
			
			pkChar->YutNoriSetPcTurn(bIsPc ? bCanMove : !bCanMove);
			pkChar->YutNoriSetThrowYut(cRanomYut, bIsPc ? YUT_NORI_PLAYER : YUT_NORI_COMPUTER);
			MiniGameYutNoriSendThrow(pkChar, bIsPc, cRanomYut);
			MiniGameYutNoriSendNextTurn(pkChar, bIsPc ? bCanMove : !bCanMove, bCanMove ? YUTNORI_STATE_MOVE : YUTNORI_STATE_THROW);
		}
		else
		{
			pkChar->YutNoriSetPcTurn(bIsPc);
			pkChar->YutNoriSetThrowYut(cRanomYut, bIsPc ? YUT_NORI_PLAYER : YUT_NORI_COMPUTER);
			
			MiniGameYutNoriSendThrow(pkChar, bIsPc, cRanomYut);
			MiniGameYutNoriSendNextTurn(pkChar, bIsPc, YUTNORI_STATE_MOVE);
		}
	}
}

char CMiniGameManager::MiniGameYutNoriGetComUnitIndex(LPCHARACTER pkChar)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return -1;
	
	char cComYut = pkChar->YutNoriGetThrowYut(YUT_NORI_COMPUTER);
	
	char cFirstPos = pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0);
	char cSecondPos = pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 1);
	
	if(cFirstPos == YUT_NORI_END_POS && cSecondPos == YUT_NORI_END_POS)
		return -1; // Both at the end
	
	if(cFirstPos == cSecondPos)
		return 0;

	char cDestPosFirst = cFirstPos;
	char cDestLastPosFirst = pkChar->YutNoriGetUnitLastPos(YUT_NORI_COMPUTER, 0);
	
	MiniGameYutNoriGetDestPos((cComYut + 1), &cDestPosFirst, &cDestLastPosFirst);
	
	char cDestPosSecond = cSecondPos;
	char cDestLastPosSecond = pkChar->YutNoriGetUnitLastPos(YUT_NORI_COMPUTER, 1);
	
	MiniGameYutNoriGetDestPos((cComYut + 1), &cDestPosSecond, &cDestLastPosSecond);
	
	bool bCatchWithFirst = (pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) == cDestPosFirst) ? true : false;
	bool bCatchWithFirstS = (pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 1) == cDestPosFirst) ? true : false;
	bool bIsCatchWithFirst = (bCatchWithFirst || bCatchWithFirstS) ? true : false;
	
	bool bCatchWithSecond = (pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) == cDestPosSecond) ? true : false;
	bool bCatchWithSecondS = (pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 1) == cDestPosSecond) ? true : false;
	bool bIsCatchWithSecond = (bCatchWithSecond || bCatchWithSecondS) ? true : false;
	
	bool bExcludeFirst = false;
	bool bExcludeSecond = false;
	if(cComYut == YUT_NORI_YUTSEM6)
	{
		if(cDestPosFirst == YUT_NORI_START_POS)
			bExcludeFirst = true;
		
		if(cDestPosSecond == YUT_NORI_START_POS)
			bExcludeSecond = true;
	}
	
	if(cFirstPos == YUT_NORI_END_POS)
		bExcludeFirst = true;
	
	if(cSecondPos == YUT_NORI_END_POS)
		bExcludeSecond = true;
	
	if(bIsCatchWithFirst && !bExcludeFirst)
		return 0;
	
	if(bIsCatchWithSecond && !bExcludeSecond)
		return 1;
		
	if(pkChar->YutNoriGetWhoIsNextCom() == 0 && bExcludeFirst && !bExcludeSecond)
		return 1;

	if(pkChar->YutNoriGetWhoIsNextCom() == 1 && !bExcludeFirst && bExcludeSecond)
		return 0;
	
	if(bExcludeFirst && bExcludeSecond)
		return -1;

	return pkChar->YutNoriGetWhoIsNextCom();
}

void CMiniGameManager::MiniGameYutNoriComAction(LPCHARACTER pkChar)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	if(pkChar->YutNoriIsPcTurn())
		return;
	
	char cUnitIndex = MiniGameYutNoriGetComUnitIndex(pkChar);

	if(cUnitIndex == -1)
	{
		if(pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0) != YUT_NORI_END_POS)
			cUnitIndex = 0;
		else if(pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 1) != YUT_NORI_END_POS)
			cUnitIndex = 1;
		else
			return;
	}
	
	pkChar->YutNoriSetWhoIsNextCom(cUnitIndex ? 0 : 1);
	
	bool bIsDouble = ((pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0) == pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 1)) && 
					pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0) != YUT_NORI_START_POS && pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0) != YUT_NORI_END_POS) ? true : false;

	char cComYut = pkChar->YutNoriGetThrowYut(YUT_NORI_COMPUTER);

	char cCurrentPos = pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, cUnitIndex);
	if(cCurrentPos == YUT_NORI_END_POS)
		return;
	
	char cDestPos = cCurrentPos;
	char cDestLastPos = pkChar->YutNoriGetUnitLastPos(YUT_NORI_COMPUTER, cUnitIndex);
	
	MiniGameYutNoriGetDestPos((cComYut + 1), &cDestPos, &cDestLastPos);

	bool bCatchFirst = (pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) == cDestPos && cDestPos != YUT_NORI_START_POS && cDestPos != YUT_NORI_END_POS) ? true : false;
	bool bCatchSecond = (pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 1) == cDestPos && cDestPos != YUT_NORI_START_POS && cDestPos != YUT_NORI_END_POS) ? true : false;
	
	if(bCatchFirst)
	{
		pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() - YUT_NORI_IN_DE_CREASE_SCORE));
		MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		
		pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 0);
		pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 0);
		
		MiniGameYutNoriSendPushCatchYut(pkChar, true, 0);
	}
	
	if(bCatchSecond)
	{
		pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() - YUT_NORI_IN_DE_CREASE_SCORE));
		MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		
		pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 1);
		pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_PLAYER, 1);
		
		MiniGameYutNoriSendPushCatchYut(pkChar, true, 1);
	}
		
	bool bIsCatch = (bCatchFirst || bCatchSecond) ? true : false;
	
	MiniGameYutNoriSendMove(pkChar, false, cUnitIndex, bIsCatch, cCurrentPos, (cDestPos == YUT_NORI_END_POS) ? YUT_NORI_START_POS : cDestPos);
	
	if(bIsDouble)
	{
		if(cDestPos == YUT_NORI_END_POS)
		{
			pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() - (YUT_NORI_IN_DE_CREASE_SCORE * 2)));
			MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		}
		
		pkChar->YutNoriSetUnitPos(cDestPos, YUT_NORI_COMPUTER, 0);
		pkChar->YutNoriSetUnitLastPos(cDestLastPos, YUT_NORI_COMPUTER, 0);
		
		pkChar->YutNoriSetUnitPos(cDestPos, YUT_NORI_COMPUTER, 1);
		pkChar->YutNoriSetUnitLastPos(cDestLastPos, YUT_NORI_COMPUTER, 1);
	}
	else
	{
		if(cDestPos == YUT_NORI_END_POS)
		{
			pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() - YUT_NORI_IN_DE_CREASE_SCORE));
			MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		}
		
		pkChar->YutNoriSetUnitPos(cDestPos, YUT_NORI_COMPUTER, cUnitIndex);
		pkChar->YutNoriSetUnitLastPos(cDestLastPos, YUT_NORI_COMPUTER, cUnitIndex);
	}
	
	if(pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0) == YUT_NORI_END_POS && pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 1) == YUT_NORI_END_POS)
	{
		pkChar->YutNoriSetRewardStatus(true);
		MiniGameYutNoriSendNextTurn(pkChar, false, YUTNORI_STATE_END);
	}
	else
	{
		if(cComYut == YUT_NORI_YUTSEM4 || cComYut == YUT_NORI_YUTSEM5)
		{
			pkChar->YutNoriSetReThrow(true);
			MiniGameYutNoriSendNextTurn(pkChar, false, YUTNORI_STATE_RE_THROW);
		}
		else
		{
			pkChar->YutNoriSetPcTurn(true);
			MiniGameYutNoriSendNextTurn(pkChar, true, YUTNORI_STATE_THROW);
		}
	}
}

void CMiniGameManager::MiniGameYutNoriCharClick(LPCHARACTER pkChar, uint8_t bUnitIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	if(!pkChar->YutNoriIsPcTurn())
		return;
	
	if(bUnitIndex > 1)
		return;
	
	char cPlayerYut = pkChar->YutNoriGetThrowYut(YUT_NORI_PLAYER);
	char cCurrentPos = pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, bUnitIndex);
	
	if(cCurrentPos == YUT_NORI_END_POS)
		return;
	
	char cDestPos = cCurrentPos;
	char cDestLastPos = pkChar->YutNoriGetUnitLastPos(YUT_NORI_PLAYER, bUnitIndex);
	MiniGameYutNoriGetDestPos((cPlayerYut + 1), &cDestPos, &cDestLastPos);

	MiniGameYutNoriSendAvailableArea(pkChar, bUnitIndex, (cDestPos == YUT_NORI_END_POS) ? YUT_NORI_START_POS : cDestPos);
}

void CMiniGameManager::MiniGameYutNoriYutMove(LPCHARACTER pkChar, uint8_t bUnitIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	if(!pkChar->YutNoriIsPcTurn())
		return;
	
	if(bUnitIndex > 1)
		return;
	
	char cPlayerYut = pkChar->YutNoriGetThrowYut(YUT_NORI_PLAYER);
	char cCurrentPos = pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, bUnitIndex);
	bool bIsDouble = ((pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) == pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 1)) && 
					pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) != YUT_NORI_START_POS && pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) != YUT_NORI_END_POS) ? true : false;
	
	if(cCurrentPos == YUT_NORI_END_POS)
		return;
	
	char cDestPos = cCurrentPos;
	char cDestLastPos = pkChar->YutNoriGetUnitLastPos(YUT_NORI_PLAYER, bUnitIndex);
	
	MiniGameYutNoriGetDestPos((cPlayerYut + 1), &cDestPos, &cDestLastPos);
	
	bool bCatchFirst = (pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 0) == cDestPos && cDestPos != YUT_NORI_START_POS && cDestPos != YUT_NORI_END_POS) ? true : false;
	bool bCatchSecond = (pkChar->YutNoriGetUnitPos(YUT_NORI_COMPUTER, 1) == cDestPos && cDestPos != YUT_NORI_START_POS && cDestPos != YUT_NORI_END_POS) ? true : false;
	
	if(bCatchFirst)
	{
		pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() + YUT_NORI_IN_DE_CREASE_SCORE));
		MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		
		pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 0);
		pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 0);
		
		MiniGameYutNoriSendPushCatchYut(pkChar, false, 0);
	}
	
	if(bCatchSecond)
	{
		pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() + YUT_NORI_IN_DE_CREASE_SCORE));
		MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		
		pkChar->YutNoriSetUnitPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 1);
		pkChar->YutNoriSetUnitLastPos(YUT_NORI_START_POS, YUT_NORI_COMPUTER, 1);
		
		MiniGameYutNoriSendPushCatchYut(pkChar, false, 1);
	}
	
	bool bIsCatch = (bCatchFirst || bCatchSecond) ? true : false;

	MiniGameYutNoriSendMove(pkChar, true, bUnitIndex, bIsCatch, cCurrentPos, (cDestPos == YUT_NORI_END_POS) ? YUT_NORI_START_POS : cDestPos);
	
	if(bIsDouble)
	{
		if(cDestPos == YUT_NORI_END_POS)
		{
			pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() + (YUT_NORI_IN_DE_CREASE_SCORE * 2)));
			MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		}
		
		pkChar->YutNoriSetUnitPos(cDestPos, YUT_NORI_PLAYER, 0);
		pkChar->YutNoriSetUnitLastPos(cDestLastPos, YUT_NORI_PLAYER, 0);
		
		pkChar->YutNoriSetUnitPos(cDestPos, YUT_NORI_PLAYER, 1);
		pkChar->YutNoriSetUnitLastPos(cDestLastPos, YUT_NORI_PLAYER, 1);
	}
	else
	{
		if(cDestPos == YUT_NORI_END_POS)
		{
			pkChar->YutNoriSetScore(MAX(0, pkChar->YutNoriGetScore() + YUT_NORI_IN_DE_CREASE_SCORE));
			MiniGameYutNoriSendSetScore(pkChar, pkChar->YutNoriGetScore());
		}
		
		pkChar->YutNoriSetUnitPos(cDestPos, YUT_NORI_PLAYER, bUnitIndex);
		pkChar->YutNoriSetUnitLastPos(cDestLastPos, YUT_NORI_PLAYER, bUnitIndex);
	}
	
	if(pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 0) == YUT_NORI_END_POS && pkChar->YutNoriGetUnitPos(YUT_NORI_PLAYER, 1) == YUT_NORI_END_POS)
	{
		pkChar->YutNoriSetRewardStatus(true);
		MiniGameYutNoriSendNextTurn(pkChar, true, YUTNORI_STATE_END);
	}
	else
	{
		if(pkChar->YutNoriGetRemainCount() == 0 || pkChar->YutNoriGetScore() == 0)
		{
			if(pkChar->YutNoriGetScore() == 0)
				pkChar->YutNoriSetRewardStatus(false);
			else
				pkChar->YutNoriSetRewardStatus(true);
			
			MiniGameYutNoriSendNextTurn(pkChar, false, YUTNORI_STATE_END);
		}
		else
		{
			if(cPlayerYut == YUT_NORI_YUTSEM4 || cPlayerYut == YUT_NORI_YUTSEM5)
			{
				pkChar->YutNoriSetReThrow(true);
				MiniGameYutNoriSendNextTurn(pkChar, true, YUTNORI_STATE_RE_THROW);
			}
			else
			{
				pkChar->YutNoriSetPcTurn(false);
				MiniGameYutNoriSendNextTurn(pkChar, false, YUTNORI_STATE_THROW);
			}
		}
	}
}

void CMiniGameManager::MiniGameYutNoriGiveup(LPCHARACTER pkChar)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	pkChar->YutNoriSetGameStatus(false);
	pkChar->YutNoriSetScore(0);
	pkChar->YutNoriSetRemainCount(0);
	
	TPacketGCMiniGameYutNori packet;
	packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
	packet.bSubheader = SUBHEADER_GC_END_GAME;
	packet.wSize = sizeof(packet);
	pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameYutNori));
}

void CMiniGameManager::MiniGameYutNoriReward(LPCHARACTER pkChar)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	if(!pkChar->YutNoriGetRewardStatus())
		return;
	
	pkChar->YutNoriSetRewardStatus(false);
	
	uint16_t wScore = pkChar->YutNoriGetScore();
	uint32_t dwRewardVnum = 0;
	
	if(wScore < 150)
		dwRewardVnum = YUT_NORI_REWARD_BRONZE;
	else if(wScore >= 150 && wScore < 220)
		dwRewardVnum = YUT_NORI_REWARD_SILVER;
	else if(wScore >= 220)
		dwRewardVnum = YUT_NORI_REWARD_GOLD;
	
	if(dwRewardVnum)
	{
		MiniGameYutNoriRegisterScore(pkChar, wScore);
		
		pkChar->AutoGiveItem(dwRewardVnum, 1);
		
		pkChar->YutNoriSetGameStatus(false);
		pkChar->YutNoriSetScore(0);
		pkChar->YutNoriSetRemainCount(0);
		
		TPacketGCMiniGameYutNori packet;
		packet.bHeader = HEADER_GC_MINI_GAME_YUT_NORI;
		packet.bSubheader = SUBHEADER_GC_END_GAME;
		packet.wSize = sizeof(packet);
		pkChar->GetDesc()->Packet(&packet, sizeof(TPacketGCMiniGameYutNori));
	}
}

void CMiniGameManager::MiniGameYutNoriProb(LPCHARACTER pkChar, uint8_t bProbIndex)
{
	if(!MiniGameYutNoriIsValidGame(pkChar))
		return;
	
	if(bProbIndex >= YUT_NORI_YUTSEM_MAX)
		return;
	
	pkChar->YutNoriSetProbIndex(bProbIndex);
	MiniGameYutNoriSendSetProb(pkChar, pkChar->YutNoriGetProbIndex());
}

uint8_t CMiniGameManager::MiniGameYutNoriGetRandomYut(bool bIsRethrow, bool bExclude6, uint8_t bProbIndex)
{
	std::vector<uint8_t> bIndexes;
	
	for(int i = YUT_NORI_YUTSEM1; i < YUT_NORI_YUTSEM_MAX; i++)
	{
		if(bIsRethrow && (i == YUT_NORI_YUTSEM4 || i == YUT_NORI_YUTSEM5))
			continue;
		
		if(bExclude6 && i == YUT_NORI_YUTSEM6)
			continue;
		
		if(i == bProbIndex)
		{
			bIndexes.push_back(i);
		}
		
		bIndexes.push_back(i);
	}
	
	uint8_t bRandomIndex = number(0, bIndexes.size() - 1);
	return bIndexes[bRandomIndex];
}

void CMiniGameManager::MiniGameYutNoriGetDestPos(char cMoveCount, char * bDestPos, char * bDestLastPos)
{
	uint8_t bStartPos = *bDestPos;
	uint8_t bLastPos = *bDestPos;
	
	if(cMoveCount == YUT_NORI_YUTSEM_MAX)
	{
		*bDestPos = *bDestLastPos;
	}
	else
	{
		for(int i = 0; i < cMoveCount; i++)
		{
			if(*bDestPos == 1)
			{
				bLastPos = *bDestPos;
				*bDestPos = (bStartPos == 1) ? 21 : 20;
				*bDestLastPos = bLastPos;
				continue;
			}
			else if(*bDestPos == 6)
			{
				bLastPos = *bDestPos;
				*bDestPos = (bStartPos == 6) ? 26 : 5;
				*bDestLastPos = bLastPos;
				continue;
			}
			else if(*bDestPos == 23)
			{
				bLastPos = *bDestPos;
				*bDestPos = (bStartPos == 23 || *bDestLastPos == 22) ? 24 : 28;
				*bDestLastPos = bLastPos;
				continue;
			}
			else if(*bDestPos == 27)
			{
				bLastPos = *bDestPos;
				*bDestPos = 23;
				*bDestLastPos = bLastPos;
				continue;
			}
			else if(*bDestPos == 29)
			{
				bLastPos = *bDestPos;
				*bDestPos = 16;
				*bDestLastPos = bLastPos;
				continue;
			}
			else if(*bDestPos == 25 || *bDestPos == 12)
			{
				bLastPos = *bDestPos;
				*bDestPos = YUT_NORI_END_POS;
				*bDestLastPos = bLastPos;
				break;
			}
			else 
			{
				bLastPos = *bDestPos;
				*bDestPos = (*bDestPos <= 20) ? *bDestPos - 1 : *bDestPos + 1;
				*bDestLastPos = bLastPos;
				continue;
			}
		}
	}
}

void CMiniGameManager::MiniGameYutNoriRegisterScore(LPCHARACTER pkChar, uint16_t wScore)
{
	if(pkChar == NULL)
		return;
	
	if(!pkChar->GetDesc())
		return;
	
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect), "SELECT total_score FROM log.yut_nori_event WHERE name = '%s' LIMIT 1;", pkChar->GetName());
	
	std::unique_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	
	SQLResult* resSelect = pSelectMsg->Get();
	if (resSelect && resSelect->uiNumRows > 0)
	{
		DBManager::instance().DirectQuery("UPDATE log.yut_nori_event SET total_score = total_score + %d WHERE name = '%s';", wScore, pkChar->GetName());
	}
	else
	{
		DBManager::instance().DirectQuery("REPLACE INTO log.yut_nori_event (name, total_score) VALUES ('%s', %d);", pkChar->GetName(), wScore);
	}
}

int CMiniGameManager::MiniGameYutNoriGetScore(lua_State* L)
{
	uint32_t index = 1;
	lua_newtable(L);
	
	char querySelect[256];

	snprintf(querySelect, sizeof(querySelect), "SELECT name, total_score FROM log.yut_nori_event ORDER BY total_score DESC LIMIT 10;");
	std::unique_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	
	SQLResult* resSelect = pSelectMsg->Get();
	if (resSelect && resSelect->uiNumRows > 0)
	{
		for (uint i = 0; i < resSelect->uiNumRows; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
			uint16_t wScore;

			str_to_number(wScore, row[1]);
			
			lua_newtable(L);
	
			lua_pushstring(L, row[0]);
			lua_rawseti(L, -2, 1);
	
			lua_pushnumber(L, wScore);
			lua_rawseti(L, -2, 2);
	
			lua_rawseti(L, -2, index++);
		}
	}
	
	return 0;
}

uint16_t CMiniGameManager::MiniGameYutNoriGetMyScore(LPCHARACTER pkChar)
{
	if(!pkChar)
		return 0;
	
	if(!pkChar->GetDesc())
		return 0;
	
	char querySelect[256];
	snprintf(querySelect, sizeof(querySelect), "SELECT total_score FROM log.yut_nori_event WHERE name = '%s' LIMIT 1;", pkChar->GetName());
	std::unique_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	
	uint16_t wScore = 0;
	if (pSelectMsg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pSelectMsg->Get()->pSQLResult);
		str_to_number(wScore, row[0]);
	}
	
	return wScore;
}

void CMiniGameManager::MiniGameYutNoriCheckEnd()
{
	if(time(0) > iYutNoriEndTime && iYutNoriEndTime != 0)
	{
		int iRewardEndTime = time(0) + 60*60*24*7;
		
		quest::CQuestManager::instance().RequestSetEventFlag("mini_game_yutnori_event", 0);
		quest::CQuestManager::instance().RequestSetEventFlag("enable_yut_nori_event_drop", 0);
		
		quest::CQuestManager::instance().RequestSetEventFlag("enable_yut_nori_event_get_reward", 1);
		quest::CQuestManager::instance().RequestSetEventFlag("yut_nori_event_get_reward_end_day", iRewardEndTime);
		
		iYutNoriEndTime = 0;
	}
	
	if(time(0) > iYutNoriRewardEndTime && iYutNoriRewardEndTime != 0)
	{
		quest::CQuestManager::instance().RequestSetEventFlag("enable_yut_nori_event_get_reward", 0);
		iYutNoriRewardEndTime = 0;
	}
}
#endif
