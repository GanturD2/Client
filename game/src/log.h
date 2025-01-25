#ifndef __INC_LOG_MANAGER_H__
#define __INC_LOG_MANAGER_H__

#include "../../libsql/AsyncSQL.h"
#include "locale_service.h"

// #ifdef ENABLE_NEWSTUFF
enum log_level { LOG_LEVEL_NONE = 0, LOG_LEVEL_MIN = 1, LOG_LEVEL_MID = 2, LOG_LEVEL_MAX = 3 };
// #endif

#ifdef ENABLE_NEWSTUFF
#define LOG_LEVEL_CHECK_N_RET(x) { if (g_iDbLogLevel < x) return; }
#else
#define LOG_LEVEL_CHECK_N_RET(x) { }
#endif

#ifdef ENABLE_NEWSTUFF
#define LOG_LEVEL_CHECK(x, fnc)	\
	{\
		if (g_iDbLogLevel >= (x))\
			fnc;\
	}
#else
#define LOG_LEVEL_CHECK(x, fnc) { fnc; }
#endif

enum GOLDBAR_HOW
{
	PERSONAL_SHOP_BUY = 1,
	PERSONAL_SHOP_SELL = 2,
	SHOP_BUY = 3,
	SHOP_SELL = 4,
	EXCHANGE_TAKE = 5,
	EXCHANGE_GIVE = 6,
	QUEST = 7,
};

class LogManager : public singleton<LogManager>
{
public:
	LogManager();
	virtual ~LogManager();

	bool IsConnected();

	bool Connect(const char* host, const int port, const char* user, const char* pwd, const char* db);

	void ItemLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dwItemID, const char* c_pszText, const char* c_pszHint, const char* c_pszIP, uint32_t dwVnum);
	void ItemLog(LPCHARACTER ch, LPITEM item, const char* c_pszText, const char* c_pszHint);
	void ItemLog(LPCHARACTER ch, int itemID, int itemVnum, const char* c_pszText, const char* c_pszHint);
	void CharLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t dw, const char* c_pszText, const char* c_pszHint, const char* c_pszIP);
	void CharLog(LPCHARACTER ch, uint32_t dw, const char* c_pszText, const char* c_pszHint);
	void LoginLog(bool isLogin, uint32_t dwAccountID, uint32_t dwPID, uint8_t bLevel, uint8_t bJob, uint32_t dwPlayTime);
	void MoneyLog(uint8_t type, uint32_t vnum, int gold);
	void HackLog(const char* c_pszHackName, const char* c_pszLogin, const char* c_pszName, const char* c_pszIP);
	void HackLog(const char* c_pszHackName, LPCHARACTER ch);
	void HackCRCLog(const char* c_pszHackName, const char* c_pszLogin, const char* c_pszName, const char* c_pszIP, uint32_t dwCRC);
	void GoldBarLog(uint32_t dwPID, uint32_t dwItemID, GOLDBAR_HOW eHow, const char* c_pszHint);
	void CubeLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum, uint32_t item_uid, int item_count, bool success);
	void GMCommandLog(uint32_t dwPID, const char* szName, const char* szIP, uint8_t byChannel, const char* szCommand);
	void SpeedHackLog(uint32_t pid, uint32_t x, uint32_t y, int hack_count);
	void ChangeNameLog(uint32_t pid, const char* old_name, const char* new_name, const char* ip);
	void RefineLog(uint32_t pid, const char* item_name, uint32_t item_id, int item_refine_level, int is_success, const char* how);
	void ShoutLog(uint8_t bChannel, uint8_t bEmpire, const char* pszText);
	void LevelLog(LPCHARACTER pChar, uint32_t level, uint32_t playhour);
	void BootLog(const char* c_pszHostName, uint8_t bChannel);
	void FishLog(uint32_t dwPID, int prob_idx, int fish_id, int fish_level, uint32_t dwMiliseconds, uint32_t dwVnum = false, uint32_t dwValue = 0);
	void QuestRewardLog(const char* c_pszQuestName, uint32_t dwPID, uint32_t dwLevel, int iValue1, int iValue2);
	void DetailLoginLog(bool isLogin, LPCHARACTER ch);
	void DragonSlayLog(uint32_t dwGuildID, uint32_t dwDragonVnum, uint32_t dwStartTime, uint32_t dwEndTime);
	void HackShieldLog(unsigned long ErrorCode, LPCHARACTER ch);
	void InvalidServerLog(enum eLocalization eLocaleType, const char* pcszIP, const char* pszRevision);
	void ChatLog(uint32_t where, uint32_t who_id, const char* who_name, uint32_t whom_id, const char* whom_name, const char* type, const char* msg, const char* ip);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	void AcceLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum, uint32_t item_uid, int item_count, int abs_chance, bool success);
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
	void MoveCostumeAttrLog(uint32_t dwPID, uint32_t x, uint32_t y, uint32_t item_vnum);
#endif
#ifdef ENABLE_DESTROY_SYSTEM
	void ItemDestroyLog(LPCHARACTER ch, LPITEM item);
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	void MeleyGuildLog(uint32_t dwGuildID, const char* szParticipantsList, uint32_t dwParticipantsCount, uint32_t dwTime);
	void MeleyPartyLog(const char* szLeaderName, const char* szParticipantsList, uint32_t dwParticipantsCount, uint32_t dwTime);
#endif
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDBANK_LOG)
	void GuildLog(LPCHARACTER ch, int guildID, uint32_t vnum, const char* typeName, int iteminout, int bPrice, uint32_t datatype);
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	void SoulRouletteLog(const char* table, const char* Name, const int vnum, const int count, const bool state);
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	void OkayEventLog(int dwPID, const char* c_pszText, int points);
#endif
#ifdef ENABLE_FISHING_RENEWAL
	void FishNewLog(uint32_t pid, uint32_t vnum, uint32_t success);
#endif

	size_t EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize);

private:
	void Query(const char* c_pszFormat, ...);

	CAsyncSQL m_sql;
	bool m_bIsConnect;
};
#define g_pLogManager singleton<LogManager>::instance_ptr()

#endif
