#ifndef __GUILD_WAR_MAP_MANAGER_H
#define __GUILD_WAR_MAP_MANAGER_H

#include "constants.h"
#include "guild.h"

enum EWarMapTypes
{
	WAR_MAP_TYPE_NORMAL,
	WAR_MAP_TYPE_FLAG,
};

typedef struct SWarMapInfo
{
	uint8_t bType;
	long lMapIndex;
	PIXEL_POSITION posStart[3];
} TWarMapInfo;

namespace warmap
{
	enum
	{
		WAR_FLAG_VNUM_START = 20035,
		WAR_FLAG_VNUM_END = 20037,

		WAR_FLAG_VNUM0 = 20035,
		WAR_FLAG_VNUM1 = 20036,
		WAR_FLAG_VNUM2 = 20037,

		WAR_FLAG_BASE_VNUM = 20038
	};

	inline bool IsWarFlag(uint32_t dwVnum)
	{
		if (dwVnum >= WAR_FLAG_VNUM_START && dwVnum <= WAR_FLAG_VNUM_END)
			return true;

		return false;
	}

	inline bool IsWarFlagBase(uint32_t dwVnum)
	{
		return dwVnum == WAR_FLAG_BASE_VNUM ? true : false;
	}
};

class CWarMap
{
public:
	friend class CGuild;

	CWarMap(long lMapIndex, const TGuildWarInfo& r_info, TWarMapInfo* pkWarMapInfo, uint32_t dwGuildID1, uint32_t dwGuildID2);
	~CWarMap();

	bool GetTeamIndex(uint32_t dwGuild, uint8_t& bIdx);

	void IncMember(LPCHARACTER ch);
	void DecMember(LPCHARACTER ch);

	CGuild* GetGuild(uint8_t bIdx);
	uint32_t GetGuildID(uint8_t bIdx);

	uint8_t GetType();
	long GetMapIndex();
	uint32_t GetGuildOpponent(LPCHARACTER ch);

	uint32_t GetWinnerGuild();
	void UsePotion(LPCHARACTER ch, LPITEM item);

	void Draw(); // ���� ���º� ó��
	void Timeout();
	void CheckWarEnd();
	bool SetEnded();
	void ExitAll();

	void SetBeginEvent(LPEVENT pkEv);
	void SetTimeoutEvent(LPEVENT pkEv);
	void SetEndEvent(LPEVENT pkEv);
	void SetResetFlagEvent(LPEVENT pkEv);

	void UpdateScore(uint32_t g1, int score1, uint32_t g2, int score2);
	bool CheckScore();

	int GetRewardGold(uint8_t bWinnerIdx);

	bool GetGuildIndex(uint32_t dwGuild, int& iIndex);

	void Packet(const void* pv, int size);
	void Notice(const char* psz);
	void SendWarPacket(LPDESC d);
	void SendScorePacket(uint8_t bIdx, LPDESC d = nullptr);

	void OnKill(LPCHARACTER killer, LPCHARACTER ch);

	void AddFlag(uint8_t bIdx, uint32_t x = 0, uint32_t y = 0);
	void AddFlagBase(uint8_t bIdx, uint32_t x = 0, uint32_t y = 0);
	void RemoveFlag(uint8_t bIdx);
	bool IsFlagOnBase(uint8_t bIdx);
	void ResetFlag();

private:
	void UpdateUserCount();

private:
	TWarMapInfo m_kMapInfo;
	bool m_bEnded;

	LPEVENT m_pkBeginEvent;
	LPEVENT m_pkTimeoutEvent;
	LPEVENT m_pkEndEvent;
	LPEVENT m_pkResetFlagEvent;

	typedef struct STeamData
	{
		uint32_t dwID;
		CGuild* pkGuild;
		int iMemberCount;
		int iUsePotionPrice;
		int iScore;
		LPCHARACTER pkChrFlag;
		LPCHARACTER pkChrFlagBase;

		std::set<uint32_t> set_pidJoiner;

		void Initialize();

		int GetAccumulatedJoinerCount(); // ������ ������ ��
		int GetCurJointerCount(); // ���� ������ ��

		void AppendMember(LPCHARACTER ch);
		void RemoveMember(LPCHARACTER ch);
	} TeamData;

	TeamData m_TeamData[2];
	int m_iObserverCount;
	uint32_t m_dwStartTime;
	uint8_t m_bTimeout;

	TGuildWarInfo m_WarInfo;

	CHARACTER_SET m_set_pkChr;
};

class CWarMapManager : public singleton<CWarMapManager>
{
public:
	CWarMapManager();
	virtual ~CWarMapManager();

	bool LoadWarMapInfo(const char* c_pszFileName);
	bool IsWarMap(long lMapIndex);
	TWarMapInfo* GetWarMapInfo(long lMapIndex);
	bool GetStartPosition(long lMapIndex, uint8_t bIdx, PIXEL_POSITION& pos);

	template <typename Func> Func for_each(Func f);
	long CreateWarMap(const TGuildWarInfo& r_WarInfo, uint32_t dwGuildID1, uint32_t dwGuildID2);
	void DestroyWarMap(CWarMap* pMap);
	CWarMap* Find(long lMapIndex);
	int CountWarMap() { return m_mapWarMap.size(); }

	void OnShutdown();

private:
	std::map<long, TWarMapInfo*> m_map_kWarMapInfo;
	std::map<long, CWarMap*> m_mapWarMap;
};

template <typename Func> Func CWarMapManager::for_each(Func f)
{
	for (auto it : m_mapWarMap)
		f(it.second);

	return f;
}

#endif
