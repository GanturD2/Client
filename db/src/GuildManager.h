// vim:ts=8 sw=4
#ifndef __INC_GUILD_MANAGER_H
#define __INC_GUILD_MANAGER_H

#include "Peer.h"
#include <queue>
#include <utility>
#include "../../libsql/libsql.h"
#include "../../libpoly/Poly.h"

enum
{
	GUILD_WARP_WAR_CHANNEL = 99
};

class CGuildWarReserve;

struct TGuildDeclareInfo
{
	uint8_t bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t bRound;
	uint8_t bPoints;
	uint8_t bTime;
#endif
	uint32_t dwGuildID[2];

#ifdef ENABLE_NEW_WAR_OPTIONS
	TGuildDeclareInfo(uint8_t _bType, uint8_t _bRound, uint8_t _bPoints, uint8_t _bTime, uint32_t _dwGuildID1, uint32_t _dwGuildID2)
		: bType(_bType), bRound(_bRound), bPoints(_bPoints), bTime(_bTime)
#else
	TGuildDeclareInfo(uint8_t _bType, uint32_t _dwGuildID1, uint32_t _dwGuildID2)
		: bType(_bType)
#endif
	{
		dwGuildID[0] = _dwGuildID1;
		dwGuildID[1] = _dwGuildID2;
	}

	bool operator < (const TGuildDeclareInfo& r) const
	{
		return ((dwGuildID[0] < r.dwGuildID[0]) || ((dwGuildID[0] == r.dwGuildID[0]) && (dwGuildID[1] < r.dwGuildID[1])));
	}

	TGuildDeclareInfo& operator = (const TGuildDeclareInfo& r)
	{
		bType = r.bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
		bRound = r.bRound;
		bPoints = r.bPoints;
		bTime = r.bTime;
#endif
		dwGuildID[0] = r.dwGuildID[0];
		dwGuildID[1] = r.dwGuildID[1];
		return *this;
	}
};

struct TGuildWaitStartInfo
{
	uint8_t bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t bRound;
	uint8_t bPoints;
	uint8_t bTime;
#endif
	uint32_t GID[2];
	long lWarPrice;
	long lInitialScore;
	CGuildWarReserve* pkReserve;

	TGuildWaitStartInfo(uint8_t _bType,
#ifdef ENABLE_NEW_WAR_OPTIONS
		uint8_t _bRound,
		uint8_t _bPoints,
		uint8_t _bTime,
#endif
		uint32_t _g1,
		uint32_t _g2,
		long _lWarPrice,
		long _lInitialScore,
		CGuildWarReserve* _pkReserve)
#ifdef ENABLE_NEW_WAR_OPTIONS
		: bType(_bType), bRound(_bRound), bPoints(_bPoints), bTime(_bTime), lWarPrice(_lWarPrice), lInitialScore(_lInitialScore), pkReserve(_pkReserve)
#else
		: bType(_bType), lWarPrice(_lWarPrice), lInitialScore(_lInitialScore), pkReserve(_pkReserve)
#endif
	{
		GID[0] = _g1;
		GID[1] = _g2;
	}

	bool operator < (const TGuildWaitStartInfo& r) const
	{
		return ((GID[0] < r.GID[0]) || ((GID[0] == r.GID[0]) && (GID[1] < r.GID[1])));
	}
};

struct TGuildWarPQElement
{
	bool bEnd;
	uint8_t bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t bRound;
	uint8_t bPoints;
	uint8_t bTime;
#endif
	uint32_t GID[2];
	uint32_t iScore[2];
	uint32_t iBetScore[2];

#ifdef ENABLE_NEW_WAR_OPTIONS
	TGuildWarPQElement(uint8_t _bType, uint8_t _bRound, uint8_t _bPoints, uint8_t _bTime, uint32_t GID1, uint32_t GID2) : bEnd(false), bType(_bType)
#else
	TGuildWarPQElement(uint8_t _bType, uint32_t GID1, uint32_t GID2) : bEnd(false), bType(_bType)
#endif
	{
		bType = _bType;
#ifdef ENABLE_NEW_WAR_OPTIONS
		bRound = _bRound;
		bPoints = _bPoints;
		bTime = _bTime;
#endif
		GID[0] = GID1;
		GID[1] = GID2;
		iScore[0] = iScore[1] = 0;
		iBetScore[0] = iBetScore[1] = 0;
	}
};

struct TGuildSkillUsed
{
	uint32_t GID;
	uint32_t dwSkillVnum;

	// GUILD_SKILL_COOLTIME_BUG_FIX
	TGuildSkillUsed(uint32_t _GID, uint32_t _dwSkillVnum) : GID(_GID), dwSkillVnum(_dwSkillVnum)
	{
	}
	// END_OF_GUILD_SKILL_COOLTIME_BUG_FIX
};

inline bool operator < (const TGuildSkillUsed& a, const TGuildSkillUsed& b)
{
	return ((a.GID < b.GID) || ((a.GID == b.GID) && (a.dwSkillVnum < b.dwSkillVnum)));
}

typedef struct SGuild
{
#ifdef ENABLE_GUILD_WAR_SCORE
	SGuild() : ladder_point(0), win(0), draw(0), loss(0), gold(0), level(0), winNew{ 0, 0, 0 }, drawNew{ 0, 0, 0 }, lossNew{ 0, 0, 0 }
#else
	SGuild() : ladder_point(0), win(0), draw(0), loss(0), gold(0), level(0)
#endif
	{
		memset(szName, 0, sizeof(szName));
	}

	char szName[GUILD_NAME_MAX_LEN + 1];
	int ladder_point;
	int win;
	int draw;
	int loss;
	int gold;
	int level;
#ifdef ENABLE_GUILD_WAR_SCORE
	int winNew[3];
	int drawNew[3];
	int lossNew[3];
#endif
} TGuild;

typedef struct SGuildWarInfo
{
	time_t tEndTime;
	TGuildWarPQElement* pElement;
	CGuildWarReserve* pkReserve;

	SGuildWarInfo() : pElement(nullptr)
	{
	}
} TGuildWarInfo;

class CGuildWarReserve
{
public:
	CGuildWarReserve(const TGuildWarReserve& rTable);

	void Initialize();

	TGuildWarReserve& GetDataRef()
	{
		return m_data;
	}

	void OnSetup(CPeer* peer);
	bool Bet(const char* pszLogin, uint32_t dwGold, uint32_t dwGuild);
	void Draw();
	void End(int iScoreFrom, int iScoreTo);

	int GetLastNoticeMin() { return m_iLastNoticeMin; }
	void SetLastNoticeMin(int iMin) { m_iLastNoticeMin = iMin; }

private:
	CGuildWarReserve(); // 기본 생성자를 사용하지 못하도록 의도적으로 구현하지 않음

	TGuildWarReserve m_data;
	// <login, <guild, gold>>
	std::map<std::string, std::pair<uint32_t, uint32_t> > mapBet;
	int m_iLastNoticeMin;
};

class CGuildManager : public singleton<CGuildManager>
{
public:
	CGuildManager();
	virtual ~CGuildManager();

	void Initialize();

	void Load(uint32_t dwGuildID);

	TGuild& TouchGuild(uint32_t GID);

	void Update();

	void OnSetup(CPeer* peer);
#ifdef ENABLE_NEW_WAR_OPTIONS
	void StartWar(uint8_t bType, uint32_t GID1, uint32_t GID2, uint8_t bRound, uint8_t bPoints, uint8_t bTime, CGuildWarReserve* pkReserve = nullptr);
#else
	void StartWar(uint8_t bType, uint32_t GID1, uint32_t GID2, CGuildWarReserve* pkReserve = nullptr);
#endif
	void UpdateScore(uint32_t guild_gain_point, uint32_t guild_opponent, int iScore, int iBetScore);

#ifdef ENABLE_NEW_WAR_OPTIONS
	void AddDeclare(uint8_t bType, uint32_t guild_from, uint32_t guild_to, uint8_t bRound, uint8_t bPoints, uint8_t bTime);
#else
	void AddDeclare(uint8_t bType, uint32_t guild_from, uint32_t guild_to);
#endif
	void RemoveDeclare(uint32_t guild_from, uint32_t guild_to);

	bool TakeBetPrice(uint32_t dwGuildTo, uint32_t dwGuildFrom, long lWarPrice);

	bool WaitStart(TPacketGuildWar* p);

	void RecvWarEnd(uint32_t GID1, uint32_t GID2);
	void RecvWarOver(uint32_t dwGuildWinner, uint32_t dwGuildLoser, bool bDraw, long lWarPrice);

	void ChangeLadderPoint(uint32_t GID, int change);

	void UseSkill(uint32_t dwGuild, uint32_t dwSkillVnum, uint32_t dwCooltime);

	INT GetGuildGold(uint32_t dwGuild);
	void DepositMoney(uint32_t dwGuild, INT lGold);
#ifdef ENABLE_USE_MONEY_FROM_GUILD
	void WithdrawMoney(uint32_t dwGuild, INT lGold);
#else
	void WithdrawMoney(CPeer* peer, uint32_t dwGuild, INT lGold);
#endif
	void WithdrawMoneyReply(uint32_t dwGuild, uint8_t bGiveSuccess, INT lGold);

	void MoneyChange(uint32_t dwGuild, uint32_t dwGold);

	void QueryRanking();
	void ResultRanking(MYSQL_RES* pRes);
	int GetRanking(uint32_t dwGID);

	//
	// Reserve War
	//
	void BootReserveWar();
	bool ReserveWar(TPacketGuildWar* p);
	void ProcessReserveWar();
	bool Bet(uint32_t dwID, const char* c_pszLogin, uint32_t dwGold, uint32_t dwGuild);

	void CancelWar(uint32_t GID1, uint32_t GID2);

	bool ChangeMaster(uint32_t dwGID, uint32_t dwFrom, uint32_t dwTo);

private:
	void ParseResult(SQLResult* pRes);

	void RemoveWar(uint32_t GID1, uint32_t GID2); // erase war from m_WarMap and set end on priority queue

	void WarEnd(uint32_t GID1, uint32_t GID2, bool bDraw = false);

	int GetLadderPoint(uint32_t GID);

#ifdef ENABLE_GUILD_WAR_SCORE
	int GetGuildWarType(int GID);
#endif

	void GuildWarWin(uint32_t GID);
	void GuildWarDraw(uint32_t GID);
	void GuildWarLose(uint32_t GID);

	void ProcessDraw(uint32_t dwGuildID1, uint32_t dwGuildID2);
	void ProcessWinLose(uint32_t dwGuildWinner, uint32_t dwGuildLoser);

	bool IsHalfWinLadderPoint(uint32_t dwGuildWinner, uint32_t dwGuildLoser);

	std::map<uint32_t, TGuild> m_map_kGuild;
	std::map<uint32_t, std::map<uint32_t, time_t> > m_mapGuildWarEndTime;

	std::set<TGuildDeclareInfo> m_DeclareMap; // 선전 포고 상태를 저장
	std::map<uint32_t, std::map<uint32_t, TGuildWarInfo> > m_WarMap;

	typedef std::pair<time_t, TGuildWarPQElement*> stPairGuildWar;
	typedef std::pair<time_t, TGuildSkillUsed> stPairSkillUsed;
	typedef std::pair<time_t, TGuildWaitStartInfo> stPairWaitStart;

	std::priority_queue<stPairGuildWar, std::vector<stPairGuildWar>, std::greater<stPairGuildWar> >
		m_pqOnWar;
	std::priority_queue<stPairWaitStart, std::vector<stPairWaitStart>, std::greater<stPairWaitStart> >
		m_pqWaitStart;
	std::priority_queue<stPairSkillUsed, std::vector<stPairSkillUsed>, std::greater<stPairSkillUsed> >
		m_pqSkill;

	std::map<uint32_t, CGuildWarReserve*> m_map_kWarReserve;
	CPoly polyPower;
	CPoly polyHandicap;

	// GID Ranking
	std::map<uint32_t, int> map_kLadderPointRankingByGID;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
public:
	void Dungeon(uint32_t dwGuildID, uint8_t bChannel, long lMapIndex);
	void DungeonCooldown(uint32_t dwGuildID, uint32_t dwTime);
#endif
};

#endif
