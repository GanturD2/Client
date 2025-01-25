#ifndef __INC_METIN_II_MINI_GAME_H__
#define __INC_METIN_II_MINI_GAME_H__

#ifdef ENABLE_MINI_GAME
#include <unordered_map>

#include "../../common/stl.h"
#include "../../common/length.h"
#include "../../common/tables.h"
#include "packet.h"
#include "questmanager.h"

#ifdef ENABLE_MINI_GAME_BNW
# include "lua_incl.h"
# include "quest.h"
#endif

#ifdef ENABLE_EVENT_BANNER_FLAG
typedef std::map<uint32_t, std::string> BannerMapType;
#endif

class CMiniGameManager : public singleton<CMiniGameManager>
{
public:
	CMiniGameManager();
	virtual ~CMiniGameManager();

	bool Initialize();
	void Destroy();

	void SpawnEventNPC(uint32_t dwVnum);

#ifdef ENABLE_EVENT_BANNER_FLAG
public:
	bool InitializeBanners();
	bool InitializeEvent(uint8_t type, uint16_t value);

private:
	BannerMapType BannerMap;
	bool m_bIsLoadedBanners;
#endif

#ifdef ENABLE_MONSTER_BACK
public:
	void InitializeAttendance(int iEnable);

	void AttendanceMonsterAttack(LPCHARACTER pkAttacker, LPCHARACTER pkVictim);
	void AttendanceEventInfo(LPCHARACTER pChar);
	void AttendanceEventRequestReward(LPCHARACTER pChar);
	bool ReadRewardItemFile(const char* c_pszFileName);

private:
	std::vector<TRewardItem> attendanceRewardVec;
#endif // ENABLE_MONSTER_BACK


#ifdef ENABLE_MINI_GAME_CATCH_KING
public:
	int MiniGameCatchKing(LPCHARACTER ch, const char* data, size_t uiBytes);

	void MiniGameCatchKingEventInfo(LPCHARACTER pkChar);
	void InitializeMiniGameCatchKing(int iEnable);
	void InitializeMiniGameCatchKingEndTime(int iEndTime) noexcept { iCatchKingEndTime = iEndTime; }
	void MiniGameCatchKingCheckEnd();

	void MiniGameCatchKingStartGame(LPCHARACTER pkChar, uint8_t bSetCount);
	void MiniGameCatchKingDeckCardClick(LPCHARACTER pkChar);
	void MiniGameCatchKingFieldCardClick(LPCHARACTER pkChar, uint8_t bFieldPos);
	void MiniGameCatchKingGetReward(LPCHARACTER pkChar);

	void MiniGameCatchKingRegisterScore(LPCHARACTER pkChar, uint32_t dwScore);
	int MiniGameCatchKingGetScore(lua_State* L, bool isTotal);
	int MiniGameCatchKingGetMyScore(LPCHARACTER pkChar, bool isTotal = false);

protected:
	int iCatchKingEndTime;
#endif // ENABLE_MINI_GAME_CATCH_KING

#ifdef ENABLE_MINI_GAME_BNW
public:
	int MiniGameBNW(LPCHARACTER ch, const char* data, size_t uiBytes);

	void MiniGameBNWStartGame(LPCHARACTER pkChar);
	void MiniGameBNWCardClick(LPCHARACTER pkChar, uint8_t bCardPos);
	void MiniGameBNWOpenShop(LPCHARACTER pkChar);
	void MiniGameBNWOpenRanking(LPCHARACTER pkChar);
	void MiniGameBNWDonateRanking(LPCHARACTER pkChar);

	uint8_t MiniGameBNWGetRandomPos(LPCHARACTER pkChar);
	bool IsGameEnd(LPCHARACTER pkChar);
#endif // ENABLE_MINI_GAME_BNW

#ifdef ENABLE_MINI_GAME_FINDM
public:
	int MiniGameFindM(LPCHARACTER ch, const char* data, size_t uiBytes);

	void InitializeMiniGameFindM(int iEnable);
	void InitializeMiniGameFindMEndTime(int iEndTime) noexcept { iFindEndTime = iEndTime; }
	void MiniGameFindMCheckEnd();

	void MiniGameFindMStartGame(LPCHARACTER pkChar);
	void MiniGameFindMClickCard(LPCHARACTER pkChar, uint8_t bFieldPos, bool bSkipCheks = false);
	void MiniGameFindMReward(LPCHARACTER pkChar, uint8_t bFlag);
	void MiniGameFindMUpgrade(LPCHARACTER pkChar, uint8_t bFlag);
	int MiniGameFindMPickRandomCard(LPCHARACTER pkChar);
#ifdef ENABLE_MINI_GAME_FINDM_HINT
	void MiniGameFindMStartHint(LPCHARACTER pkChar);
#endif
	void MiniGameFindMRegisterScore(LPCHARACTER pkChar, uint32_t dwSeconds, uint16_t wTry);
	int MiniGameFindMGetScore(lua_State* L, bool isTry);

protected:
	int iFindEndTime;
#endif // ENABLE_MINI_GAME_FINDM

#ifdef ENABLE_MINI_GAME_YUTNORI
public:
	int MiniGameYutNori(LPCHARACTER ch, const char* data, size_t uiBytes);

	void InitializeMiniGameYutNori(int iEnable);
	void InitializeMiniGameYutNoriEndTime(int iEndTime) { iYutNoriEndTime = iEndTime; }
	void InitializeMiniGameYutNoriRewardEndTime(int iEndTime) { iYutNoriRewardEndTime = iEndTime; }
	void MiniGameYutNoriCheckEnd();

	void MiniGameYutNoriStartGame(LPCHARACTER pkChar);
	void MiniGameYutNoriYutThrow(LPCHARACTER pkChar, uint8_t bIsPc);
	void MiniGameYutNoriComAction(LPCHARACTER pkChar);
	void MiniGameYutNoriCharClick(LPCHARACTER pkChar, uint8_t bUnitIndex);
	void MiniGameYutNoriYutMove(LPCHARACTER pkChar, uint8_t bUnitIndex);
	void MiniGameYutNoriGiveup(LPCHARACTER pkChar);
	void MiniGameYutNoriReward(LPCHARACTER pkChar);
	void MiniGameYutNoriProb(LPCHARACTER pkChar, uint8_t bProbIndex);

	void MiniGameYutNoriSendThrow(LPCHARACTER pkChar, bool bIsPc, char cYut);
	void MiniGameYutNoriSendNextTurn(LPCHARACTER pkChar, bool bPcTurn, uint8_t bState);
	void MiniGameYutNoriSendMove(LPCHARACTER pkChar, bool bIsPc, uint8_t bUnitIndex, bool bIsCatch, uint8_t bStartIndex, uint8_t bDestIndex);
	void MiniGameYutNoriSendAvailableArea(LPCHARACTER pkChar, uint8_t bPlayerIndex, uint8_t bAvailableIndex);
	void MiniGameYutNoriSendPushCatchYut(LPCHARACTER pkChar, bool bIsPc, uint8_t bUnitIndex);
	void MiniGameYutNoriSendSetProb(LPCHARACTER pkChar, uint8_t bProbIndex);
	void MiniGameYutNoriSendSetScore(LPCHARACTER pkChar, uint16_t wScore);
	void MiniGameYutNoriSendSetRemainCount(LPCHARACTER pkChar, uint8_t bRemainCount);

	uint8_t MiniGameYutNoriGetRandomYut(bool bIsRethrow, bool bExclude6, uint8_t bProbIndex);
	bool MiniGameYutNoriIsValidGame(LPCHARACTER pkChar, bool bSkipGameStatus = true);
	void MiniGameYutNoriGetDestPos(char cMoveCount, char* bDestPos, char* bDestLastPos);
	char MiniGameYutNoriGetComUnitIndex(LPCHARACTER pkChar);

	void MiniGameYutNoriRegisterScore(LPCHARACTER pkChar, uint16_t wScore);
	int MiniGameYutNoriGetScore(lua_State* L);
	uint16_t MiniGameYutNoriGetMyScore(LPCHARACTER pkChar);

protected:
	int iYutNoriEndTime, iYutNoriRewardEndTime;
#endif // ENABLE_MINI_GAME_YUTNORI

#ifdef ENABLE_EASTER_EVENT
public:
	void InitializeEasterEvent(int iEnable);
#endif // ENABLE_EASTER_EVENT

#ifdef ENABLE_METINSTONE_RAIN_EVENT
public:
	enum EStoneEvent
	{
		STONE_EVENT_POINT = 1,
	};

	uint32_t OnePlayer = 0;
	uint32_t OnePlayerPoint = 0;
	void SetStoneKill(uint32_t PID);
	void StoneInformation(LPCHARACTER pkChr);
	void DeleteStoneEvent();
	void RewardItem();
	int16_t GetStoneEventPoint(LPCHARACTER pkChr);
	bool IsStoneEvent();
	bool IsStoneEventSystemStatus();
	void StoneUpdateP2PPacket(uint32_t PID);
	std::unordered_map<uint32_t, int16_t> m_Stone;
#endif // ENABLE_METINSTONE_RAIN_EVENT
};

#endif
#endif
