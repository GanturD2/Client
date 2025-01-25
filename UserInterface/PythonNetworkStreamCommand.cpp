#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonNonPlayer.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "AbstractCharacterManager.h"
#include "AbstractChat.h"
#include "InstanceBase.h"
#include "PythonBackground.h"
#ifdef ENABLE_WEATHER_INFO
#include "WeatherInfo.h"
#endif

#define ishan(ch) (((ch) &0xE0) > 0x90)
#define ishanasc(ch) (isascii(ch) || ishan(ch))
#define ishanalp(ch) (isalpha(ch) || ishan(ch))
#define isnhdigit(ch) (!ishan(ch) && isdigit(ch))
#define isnhspace(ch) (!ishan(ch) && isspace(ch))

#define LOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))
#define UPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))

void SkipSpaces(char ** string)
{
	for (; **string != '\0' && isnhspace((unsigned char) **string); ++(*string))
		continue;
}

char * OneArgument(char * argument, char * first_arg)
{
	char mark = FALSE;

	if (!argument)
	{
		*first_arg = '\0';
		return nullptr;
	}

	SkipSpaces(&argument);

	while (*argument)
	{
		if (*argument == '\"')
		{
			mark = !mark;
			++argument;
			continue;
		}

		if (!mark && isnhspace((unsigned char) *argument))
			break;

		*(first_arg++) = LOWER(*argument);
		++argument;
	}

	*first_arg = '\0';

	SkipSpaces(&argument);
	return (argument);
}

bool CPythonNetworkStream::ClientCommand(const char * c_szCommand) const
{
	return false;
}

bool SplitToken(const char * c_szLine, CTokenVector * pstTokenVector, const char * c_szDelimeter = " ")
{
	pstTokenVector->reserve(10);
	pstTokenVector->clear();

	std::string stToken;
	std::string strLine = c_szLine;

	uint32_t basePos = 0;

	do
	{
		int beginPos = strLine.find_first_not_of(c_szDelimeter, basePos);
		if (beginPos < 0)
			return false;

		int endPos;

		if (strLine[beginPos] == '"')
		{
			++beginPos;
			endPos = strLine.find_first_of('\"', beginPos);

			if (endPos < 0)
				return false;

			basePos = endPos + 1;
		}
		else
		{
			endPos = strLine.find_first_of(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pstTokenVector->emplace_back(strLine.substr(beginPos, endPos - beginPos));

		// 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
		if (int(strLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
			break;
	} while (basePos < strLine.length());

	return true;
}

void CPythonNetworkStream::ServerCommand(char * c_szCommand)
{
	if (strcmpi(c_szCommand, "ConsoleEnable") == 0)
		return;

	if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
	{
		bool isTrue;
		if (PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ServerCommand_Run", Py_BuildValue("(s)", c_szCommand), &isTrue))
		{
			if (isTrue)
				return;
		}
	}
	else if (m_poSerCommandParserWnd)
	{
		bool isTrue;
		if (PyCallClassMemberFunc(m_poSerCommandParserWnd, "BINARY_ServerCommand_Run", Py_BuildValue("(s)", c_szCommand), &isTrue))
		{
			if (isTrue)
				return;
		}
	}

	CTokenVector TokenVector;
	if (!SplitToken(c_szCommand, &TokenVector))
		return;
	if (TokenVector.empty())
		return;

	const char * szCmd = TokenVector[0].c_str();

	if (!strcmpi(szCmd, "quit"))
		PostQuitMessage(0);
	else if (!strcmpi(szCmd, "BettingMoney"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
		}

		//uint32_t uMoney= atoi(TokenVector[1].c_str());
	}
	// GIFT NOTIFY
	else if (!strcmpi(szCmd, "gift"))
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Gift_Show", Py_BuildValue("()"));
#ifdef ENABLE_WEATHER_INFO
	else if (!strcmpi(szCmd, "WeatherRefresh"))
	{
		if (TokenVector.size() < 2)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint8_t serverDayMode = atoi(TokenVector[1].c_str());
		bool serverSeason = atoi(TokenVector[2].c_str());
		uint8_t bRainActive = atoi(TokenVector[3].c_str());
		//TraceError("Server: DayMode %d Season %d Rain %d", serverDayMode, serverSeason, bRainActive);

		IWeatherInfo& iBG = IWeatherInfo::Instance();
		iBG.WeatherRefresh(serverDayMode, serverSeason, bRainActive);
	}
#endif
	// CUBE
	else if (!strcmpi(szCmd, "cube"))
	{
		if (TokenVector.size() < 2)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		if ("open" == TokenVector[1])
		{
			if (4 > TokenVector.size())	//@fixme424 (3 -> 4)
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			auto npcVNUM = static_cast<uint32_t>(atoi(TokenVector[2].c_str()));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Open", Py_BuildValue("(i)", npcVNUM));
		}
		else if ("close" == TokenVector[1])
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Close", Py_BuildValue("()"));
		else if ("info" == TokenVector[1])
		{
#ifdef ENABLE_CUBE_PERCENT_INFO
			if (6 != TokenVector.size())
#else
			if (5 != TokenVector.size())
#endif
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			uint32_t gold = atoi(TokenVector[2].c_str());
			uint32_t itemVnum = atoi(TokenVector[3].c_str());
			uint32_t count = atoi(TokenVector[4].c_str());
#ifdef ENABLE_CUBE_PERCENT_INFO
			uint32_t perc = atoi(TokenVector[5].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_UpdateInfo", Py_BuildValue("(iiii)", gold, itemVnum, count, perc));
#else
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_UpdateInfo", Py_BuildValue("(iii)", gold, itemVnum, count));
#endif
		}
		else if ("success" == TokenVector[1])
		{
			if (4 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			uint32_t itemVnum = atoi(TokenVector[2].c_str());
			uint32_t count = atoi(TokenVector[3].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Succeed", Py_BuildValue("(ii)", itemVnum, count));
		}
		else if ("fail" == TokenVector[1])
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Failed", Py_BuildValue("()"));
		else if ("r_list" == TokenVector[1])
		{
			// result list (/cube r_list npcVNUM resultCount resultText)
			// 20383 4 72723,1/72725,1/72730.1/50001,5 <- 이런식으로 "/" 문자로 구분된 리스트를 줌
			if (5 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand, 5);
				return;
			}

			auto npcVNUM = static_cast<uint32_t>(atoi(TokenVector[2].c_str()));

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_ResultList",
								  Py_BuildValue("(is)", npcVNUM, TokenVector[4].c_str()));
		}
		else if ("m_info" == TokenVector[1])
		{
			// material list (/cube m_info requestStartIndex resultCount MaterialText)
			// ex) requestStartIndex: 0, resultCount : 5 - 해당 NPC가 만들수 있는 아이템 중 0~4번째에 해당하는 아이템을 만드는 데 필요한 모든 재료들이 MaterialText에 들어있음
			// 위 예시처럼 아이템이 다수인 경우 구분자 "@" 문자를 사용
			// 0 5 125,1|126,2|127,2|123,5&555,5&555,4/120000 <- 이런식으로 서버에서 클라로 리스트를 줌

			if (5 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand, 5);
				return;
			}

			auto requestStartIndex = static_cast<uint32_t>(atoi(TokenVector[2].c_str()));
			auto resultCount = static_cast<uint32_t>(atoi(TokenVector[3].c_str()));

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_MaterialInfo",
								  Py_BuildValue("(iis)", requestStartIndex, resultCount, TokenVector[4].c_str()));
		}
	}
	// CUEBE_END
	else if (!strcmpi(szCmd, "ObserverCount"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint32_t uObserverCount = atoi(TokenVector[1].c_str());

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BettingGuildWar_UpdateObserverCount",
							  Py_BuildValue("(i)", uObserverCount));
	}
	else if (!strcmpi(szCmd, "ObserverMode"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint32_t uMode = atoi(TokenVector[1].c_str());

		IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
		rkPlayer.SetObserverMode(uMode ? true : false);

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_BettingGuildWar_SetObserverMode",
#ifdef ENABLE_GUILDWAR_BUTTON
			Py_BuildValue("(ii)", uMode, uMode));
#else
			Py_BuildValue("(i)", uMode));
#endif
	}
	else if (!strcmpi(szCmd, "ObserverTeamInfo"))
	{
	}
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	else if (!strcmpi(szCmd, "ShopSearchError"))
	{
		if (TokenVector.size() < 2)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		const uint8_t error = atoi(TokenVector[1].c_str());
		switch (error)
		{
			case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
				break;

			case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
				break;

			case SHOP_SUBHEADER_GC_SOLDOUT:
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
				break;

			case SHOP_SUBHEADER_GC_INVENTORY_FULL:
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
				break;

			case SHOP_SUBHEADER_GC_INVALID_POS:
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
				break;

			default:
				break;
		}
	}
#endif
#ifdef ENABLE_12ZI
	else if (!strcmpi(szCmd, "ZodiacTime"))
	{
		if (5 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint8_t current = atoi(TokenVector[1].c_str());
		uint8_t next = atoi(TokenVector[2].c_str());
		int time = atoi(TokenVector[3].c_str());
		int elapseTime = atoi(TokenVector[4].c_str());

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Refresh12ziTimer", Py_BuildValue("(bbii)", current, next, time, elapseTime));
	}
	else if (!strcmpi(szCmd, "ZodiacTimeClear"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Hide12ziTimer", Py_BuildValue("()"));
	}
	else if (!strcmpi(szCmd, "Bead_count"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		int count = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetBeadCount", Py_BuildValue("(i)", count));
	}
	else if (!strcmpi(szCmd, "Bead_time"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		int count = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "NextBeadUpdateTime", Py_BuildValue("(i)", count));
	}
	else if (!strcmpi(szCmd, "OpenReviveDialog"))
	{
		if (3 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint32_t vid = atoi(TokenVector[1].c_str());
		uint8_t count = atoi(TokenVector[2].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenReviveDialog", Py_BuildValue("(ib)", vid, count));
	}
	else if (!strcmpi(szCmd, "NotEnoughPrism"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint8_t count = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "NotEnoughPrism", Py_BuildValue("(b)", count));
	}

#ifdef ENABLE_CHAT_MISSION_ALTERNATIVE
	else if (!strcmpi(szCmd, "SetMissionMessage"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		const std::string & c_rstrMessage = TokenVector[1].c_str();
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetMissionMessage", Py_BuildValue("(s)", c_rstrMessage.c_str()));
	}
	
	else if (!strcmpi(szCmd, "SetSubMissionMessage"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		const std::string & c_rstrMessage = TokenVector[1].c_str();
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetSubMissionMessage", Py_BuildValue("(s)", c_rstrMessage.c_str()));
	}
	
	else if (!strcmpi(szCmd, "CleanMissionMessage"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CleanMissionMessage", Py_BuildValue("()"));
	}
#endif
#endif

#ifdef ENABLE_REFINE_MSG_ADD
	else if (!strcmpi(szCmd, "RefineFailedType"))
	{
		if (2 != TokenVector.size()) {
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		const uint8_t bType = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefineFailedTypeMessage", Py_BuildValue("(i)", bType));
	}
#endif

	else if (!strcmpi(szCmd, "xmas_snow"))
	{
		CPythonBackground& rkBG = CPythonBackground::Instance();
		uint32_t dwMode = atoi(TokenVector[1].c_str());
		if (dwMode == 1)
			rkBG.EnableSnowEnvironment();
		else
			rkBG.DisableSnowEnvironment();
	}

	else if (!strcmpi(szCmd, "StoneDetect"))
	{
		if (4 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		// vid distance(1-3) angle(0-360)
		uint32_t dwVID = atoi(TokenVector[1].c_str());
		uint8_t byDistance = atoi(TokenVector[2].c_str());
		float fAngle = atof(TokenVector[3].c_str());
		fAngle = fmod(540.0f - fAngle, 360.0f);
		Tracef("StoneDetect [VID:%d] [Distance:%d] [Angle:%d->%f]\n", dwVID, byDistance, atoi(TokenVector[3].c_str()), fAngle);

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();

		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(dwVID);
		if (!pInstance)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Not Exist Instance", c_szCommand);
			return;
		}

		TPixelPosition PixelPosition;
		D3DXVECTOR3 v3Rotation(0.0f, 0.0f, fAngle);
		pInstance->NEW_GetPixelPosition(&PixelPosition);

		PixelPosition.y *= -1.0f;

		switch (byDistance)
		{
			case 0:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/firecracker/find_out.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/firecracker/find_out.mse", PixelPosition, v3Rotation);
				break;
			case 1:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_small.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_small.mse", PixelPosition, v3Rotation);
				break;
			case 2:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_middle.mse", PixelPosition, v3Rotation);
				break;
			case 3:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_large.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_large.mse", PixelPosition, v3Rotation);
				break;
			default:
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Distance", c_szCommand);
				break;
		}

#ifdef _DEBUG
		IAbstractChat & rkChat = IAbstractChat::GetSingleton();
		rkChat.AppendChat(CHAT_TYPE_INFO, c_szCommand);
#endif
	}
	else if (!strcmpi(szCmd, "StartStaminaConsume"))
	{
		if (3 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		uint32_t dwConsumePerSec = atoi(TokenVector[1].c_str());
		uint32_t dwCurrentStamina = atoi(TokenVector[2].c_str());

		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.StartStaminaConsume(dwConsumePerSec, dwCurrentStamina);
	}

	else if (!strcmpi(szCmd, "StopStaminaConsume"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand,
					   TokenVector.size());
			return;
		}

		uint32_t dwCurrentStamina = atoi(TokenVector[1].c_str());

		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.StopStaminaConsume(dwCurrentStamina);
	}
	else if (!strcmpi(szCmd, "sms"))
	{
		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.SetMobileFlag(TRUE);
	}
	else if (!strcmpi(szCmd, "nosms"))
	{
		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.SetMobileFlag(FALSE);
	}
	else if (!strcmpi(szCmd, "mobile_auth"))
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMobileAuthority", Py_BuildValue("()"));
	else if (!strcmpi(szCmd, "messenger_auth"))
	{
		const std::string & c_rstrName = TokenVector[1];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMessengerAddFriendQuestion", Py_BuildValue("(s)", c_rstrName.c_str()));
	}
	else if (!strcmpi(szCmd, "combo"))
	{
		int iFlag = atoi(TokenVector[1].c_str());
		IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.SetComboSkillFlag(iFlag);
		m_bComboSkillFlag = iFlag ? true : false;
	}
	else if (!strcmpi(szCmd, "setblockmode"))
	{
		int iFlag = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnBlockMode", Py_BuildValue("(i)", iFlag));
	}
	// Emotion Start
	else if (!strcmpi(szCmd, "french_kiss"))
	{
		int iVID1 = atoi(TokenVector[1].c_str());
		int iVID2 = atoi(TokenVector[2].c_str());

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
		CInstanceBase * pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
		if (pInstance1 && pInstance2)
			pInstance1->ActDualEmotion(*pInstance2, CRaceMotionData::NAME_FRENCH_KISS_START, CRaceMotionData::NAME_FRENCH_KISS_START);
	}
	else if (!strcmpi(szCmd, "kiss"))
	{
		int iVID1 = atoi(TokenVector[1].c_str());
		int iVID2 = atoi(TokenVector[2].c_str());

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
		CInstanceBase * pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
		if (pInstance1 && pInstance2)
			pInstance1->ActDualEmotion(*pInstance2, CRaceMotionData::NAME_KISS_START, CRaceMotionData::NAME_KISS_START);
	}
	else if (!strcmpi(szCmd, "slap"))
	{
		int iVID1 = atoi(TokenVector[1].c_str());
		int iVID2 = atoi(TokenVector[2].c_str());

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
		CInstanceBase * pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
		if (pInstance1 && pInstance2)
			pInstance1->ActDualEmotion(*pInstance2, CRaceMotionData::NAME_SLAP_HURT_START, CRaceMotionData::NAME_SLAP_HIT_START);
	}
	else if (!strcmpi(szCmd, "clap"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_CLAP);
	}
	else if (!strcmpi(szCmd, "cheer1"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_CHEERS_1);
	}
	else if (!strcmpi(szCmd, "cheer2"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_CHEERS_2);
	}
	else if (!strcmpi(szCmd, "dance1"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_DANCE_1);
	}
	else if (!strcmpi(szCmd, "dance2"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_DANCE_2);
	}
	else if (!strcmpi(szCmd, "dig_motion"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_DIG);
	}
	// Emotion End
#ifdef ENABLE_BATTLE_FIELD
	else if (!strcmpi(szCmd, "BattleFieldLeftTime"))
	{
		if (3 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand,
				TokenVector.size());
			return;
		}

		uint32_t uLeftOpen = atoi(TokenVector[1].c_str());
		uint32_t uLeftEnd = atoi(TokenVector[2].c_str());

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetBattleFieldLeftTime", Py_BuildValue("(ii)", uLeftOpen, uLeftEnd));
	}
	else if (!strcmpi(szCmd, "ExitBattleField"))
	{
		if (TokenVector.size() <= 1)
			return;

		uint32_t uPoint = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExitBattleField", Py_BuildValue("(i)", uPoint));
	}
	else if (!strcmpi(szCmd, "ExitBattleFieldOnDead"))
	{
		if (TokenVector.size() <= 1)
			return;

		uint32_t uPoint = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExitBattleFieldOnDead", Py_BuildValue("(i)", uPoint));
	}
	else if (!strcmpi(szCmd, "ResetUsedBP"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ResetUsedBP", Py_BuildValue("()"));
	}
	else if (!strcmpi(szCmd, "OpenRestartDialog"))
	{
		if (1 != TokenVector.size())
			return;

		uint32_t uMapIdx = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenRestartDialog", Py_BuildValue("(i)", uMapIdx));
	}
#endif
#ifdef ENABLE_MEDAL_OF_HONOR
	else if (!strcmpi(szCmd, "ResetUsedMedal"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ResetUsedMedal", Py_BuildValue("()"));
	}
#endif
#ifdef ENABLE_TICKET_SYSTEM
	else if (!strcmpi(szCmd, "ticket"))
	{
#define MAX_ARGUMENTS_A 10
#define MAX_ARGUMENTS_B 4

		if ("team_logs" == TokenVector[1])
		{
			if (TokenVector.size() != MAX_ARGUMENTS_A)
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			int index = atoi(TokenVector[2].c_str());
			std::string ticked_id = TokenVector[3].c_str();
			std::string char_name = TokenVector[4].c_str();
			std::string title = TokenVector[5].c_str();
			std::string content = TokenVector[6].c_str();
			int priority = atoi(TokenVector[7].c_str());
			std::string create_date = TokenVector[8].c_str();
			int status = atoi(TokenVector[9].c_str());

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Ticket_Logs_Team", Py_BuildValue("(issssisi)", index, ticked_id.c_str(), char_name.c_str(), title.c_str(), content.c_str(), priority, create_date.c_str(), status));
		}
		else if ("elevate" == TokenVector[1])
		{
			if (TokenVector.size() != MAX_ARGUMENTS_B)
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			int value = atoi(TokenVector[3].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Ticket_Sort_Admin", Py_BuildValue("(si)", TokenVector[2].c_str(), value));
		}
	}
#endif
	else
	{
		static std::map<std::string, int> s_emotionDict;

		static bool s_isFirst = true;
		if (s_isFirst)
		{
			s_isFirst = false;

			s_emotionDict["dance3"] = CRaceMotionData::NAME_DANCE_3;
			s_emotionDict["dance4"] = CRaceMotionData::NAME_DANCE_4;
			s_emotionDict["dance5"] = CRaceMotionData::NAME_DANCE_5;
			s_emotionDict["dance6"] = CRaceMotionData::NAME_DANCE_6;
			s_emotionDict["congratulation"] = CRaceMotionData::NAME_CONGRATULATION;
			s_emotionDict["forgive"] = CRaceMotionData::NAME_FORGIVE;
			s_emotionDict["angry"] = CRaceMotionData::NAME_ANGRY;
			s_emotionDict["attractive"] = CRaceMotionData::NAME_ATTRACTIVE;
			s_emotionDict["sad"] = CRaceMotionData::NAME_SAD;
			s_emotionDict["shy"] = CRaceMotionData::NAME_SHY;
			s_emotionDict["cheerup"] = CRaceMotionData::NAME_CHEERUP;
			s_emotionDict["banter"] = CRaceMotionData::NAME_BANTER;
			s_emotionDict["joy"] = CRaceMotionData::NAME_JOY;
#ifdef ENABLE_EXPRESSING_EMOTION
			s_emotionDict["pushup"] = CRaceMotionData::NAME_PUSHUP;
			s_emotionDict["dance_7"] = CRaceMotionData::NAME_DANCE_7;
			s_emotionDict["exercise"] = CRaceMotionData::NAME_EXERCISE;
			s_emotionDict["doze"] = CRaceMotionData::NAME_DOZE;
			s_emotionDict["selfie"] = CRaceMotionData::NAME_SELFIE;
#endif
		}

		auto f = s_emotionDict.find(szCmd);
		if (f == s_emotionDict.end())
			TraceError("Unknown Server Command %s | %s", c_szCommand, szCmd);
		else
		{
			int emotionIndex = f->second;

			int iVID = atoi(TokenVector[1].c_str());
			IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
			CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);

			if (pInstance)
				pInstance->ActEmotion(emotionIndex);
		}
	}
}
