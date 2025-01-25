#pragma once

#include "Packet.h"

class CPythonGuild : public CSingleton<CPythonGuild>
{
public:
	enum
	{
		GUILD_SKILL_MAX_NUM = 12,
		ENEMY_GUILD_SLOT_MAX_COUNT = 6
	};

	enum WarTypes {
		GUILD_WAR_TYPE_NORMAL,
		GUILD_WAR_TYPE_DIE,
		GUILD_WAR_TYPE_FLAG,
		/*GUILD_WAR_TYPE_TIGER,
		GUILD_WAR_TYPE_DEFENSE,
		GUILD_WAR_TYPE_TIME,*/

		GUILD_WAR_TYPE_MAX_NUM
	};

#ifdef ENABLE_GUILDBANK_LOG
	typedef struct SGuildBank
	{
		char szName[24];
		char szItemName[24];
		int itemAction;
		int itemCount;
		int datatype;
	} BankLog;
#endif

	typedef struct SGulidInfo
	{
		uint32_t dwGuildID;
		char szGuildName[GUILD_NAME_MAX_LEN + 1];
		uint32_t dwMasterPID;
		uint32_t dwGuildLevel;
		uint32_t dwCurrentExperience;
		uint32_t dwCurrentMemberCount;
		uint32_t dwMaxMemberCount;
		uint32_t dwGuildMoney;
		BOOL bHasLand;
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
		char goldCheckout[CHARACTER_NAME_MAX_LEN + 1];
		char itemCheckout[CHARACTER_NAME_MAX_LEN + 1];
#endif
#ifdef ENABLE_GUILD_WAR_SCORE
		uint32_t winTypes[3];
		uint32_t lossTypes[3];
		uint32_t drawTypes[3];
		uint32_t ladderPoints;
		uint32_t rank;
#endif
#ifdef ENABLE_GUILD_LAND_INFO
		uint32_t mindex;
		uint32_t guildAltar;
		uint32_t guildAlchemist;
		uint32_t guildBlacksmith;
		uint32_t storage_lv;
#endif
#ifdef ENABLE_GUILDBANK_LOG
		uint32_t pLogCount;
#endif
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		uint8_t bMedals;
		uint8_t bDonateCount;
#endif
	} TGuildInfo;

	typedef struct SGuildGradeData
	{
		SGuildGradeData() {}
		SGuildGradeData(uint8_t byAuthorityFlag_, const char * c_szName_) : byAuthorityFlag(byAuthorityFlag_), strName(c_szName_) {}
		uint8_t byAuthorityFlag;
		std::string strName;
	} TGuildGradeData;
	typedef std::map<uint8_t, TGuildGradeData> TGradeDataMap;

	typedef struct SGuildMemberData
	{
		uint32_t dwPID;

		std::string strName;
		uint8_t byGrade;
		uint8_t byJob;
		uint8_t byLevel;
		uint8_t byGeneralFlag;
		uint32_t dwOffer;
	} TGuildMemberData;

	using TGuildMemberDataVector = std::vector<TGuildMemberData>;

	typedef struct SGuildBoardCommentData
	{
		uint32_t dwCommentID;
		std::string strName;
		std::string strComment;
	} TGuildBoardCommentData;

	using TGuildBoardCommentDataVector = std::vector<TGuildBoardCommentData>;

	typedef struct SGuildSkillData
	{
		uint8_t bySkillPoint;
		uint8_t bySkillLevel[GUILD_SKILL_MAX_NUM];
		uint16_t wGuildPoint;
		uint16_t wMaxGuildPoint;
	} TGuildSkillData;

#ifdef ENABLE_COLORED_GUILD_RANKS
	struct GuildNameRank
	{
		std::string name;
		int rank;
	};

	typedef std::map<uint32_t, GuildNameRank> TGuildNameMap;
#else
	typedef std::map<uint32_t, std::string> TGuildNameMap;
#endif

public:
	CPythonGuild();
	~CPythonGuild() = default;
	CLASS_DELETE_COPYMOVE(CPythonGuild);

	void Destroy();

	void EnableGuild();
	void SetGuildMoney(uint32_t dwMoney);
	void SetGuildEXP(uint8_t byLevel, uint32_t dwEXP);
	void SetGradeData(uint8_t byGradeNumber, const TGuildGradeData & rGuildGradeData);
	void SetGradeName(uint8_t byGradeNumber, const char * c_szName);
	void SetGradeAuthority(uint8_t byGradeNumber, uint8_t byAuthority);
	void ClearComment();
	void RegisterComment(uint32_t dwCommentID, const char * c_szName, const char * c_szComment);
	void RegisterMember(TGuildMemberData & rGuildMemberData);
	void ChangeGuildMemberGrade(uint32_t dwPID, uint8_t byGrade);
	void ChangeGuildMemberGeneralFlag(uint32_t dwPID, uint8_t byFlag);
	void RemoveMember(uint32_t dwPID);
#ifdef ENABLE_COLORED_GUILD_RANKS
	void RegisterGuildName(uint32_t dwID, const char * c_szName, int rank);
#else
	void RegisterGuildName(uint32_t dwID, const char * c_szName);
#endif

	BOOL IsMainPlayer(uint32_t dwPID);
	BOOL IsGuildEnable() const;
	TGuildInfo & GetGuildInfoRef();
	BOOL GetGradeDataPtr(uint32_t dwGradeNumber, TGuildGradeData ** ppData);
	const TGuildBoardCommentDataVector & GetGuildBoardCommentVector() const;
	uint32_t GetMemberCount() const;
	BOOL GetMemberDataPtr(uint32_t dwIndex, TGuildMemberData ** ppData);
	BOOL GetMemberDataPtrByPID(uint32_t dwPID, TGuildMemberData ** ppData);
	BOOL GetMemberDataPtrByName(const char * c_szName, TGuildMemberData ** ppData);
	uint32_t GetGuildMemberLevelSummary() const;
	uint32_t GetGuildMemberLevelAverage() const;
	uint32_t GetGuildExperienceSummary() const;
	TGuildSkillData & GetGuildSkillDataRef();
	bool GetGuildName(uint32_t dwID, std::string * pstrGuildName);
	uint32_t GetGuildID() const;
	BOOL HasGuildLand() const;

	void StartGuildWar(uint32_t dwEnemyGuildID);
	void EndGuildWar(uint32_t dwEnemyGuildID);
	uint32_t GetEnemyGuildID(uint32_t dwIndex);
	BOOL IsDoingGuildWar();

#ifdef ENABLE_GUILD_WAR_SCORE
	void SetGuildWarInfo(uint32_t * winType, uint32_t * lossType, uint32_t * drawType, uint32_t ladderPoints, uint32_t rank)
	{
		for (int i = 0; i < 3; ++i) {
			m_GuildInfo.winTypes[i] = winType[i];
			m_GuildInfo.lossTypes[i] = lossType[i];
			m_GuildInfo.drawTypes[i] = drawType[i];
		}

		m_GuildInfo.ladderPoints = ladderPoints;
		m_GuildInfo.rank = rank;
	}
#endif
#ifdef ENABLE_GUILD_LAND_INFO
	void SetGuildLandInfo(uint32_t mindex, uint32_t guildAltar, uint32_t guildAlchemist, uint32_t guildBlacksmith, uint32_t storage_lv)
	{
		m_GuildInfo.mindex = mindex;
		m_GuildInfo.guildAltar = guildAltar;
		m_GuildInfo.guildAlchemist = guildAlchemist;
		m_GuildInfo.guildBlacksmith = guildBlacksmith;
		m_GuildInfo.storage_lv = storage_lv;
	}
#endif
#ifdef ENABLE_GUILDBANK_LOG
	void IncreaseBankLog()
	{
		++m_GuildInfo.pLogCount;
	}

	void ClearBankLog()
	{
		log.clear();
		m_GuildInfo.pLogCount = 0;
	}

	void SetGuildBankLogs(const char* name, const char* itemName, int itemAction, int itemCount, int datatype)
	{
		const int index = m_GuildInfo.pLogCount;
		if (index == 0 || index <= 0)
		{
			log.clear();
		}
		BankLog entry;
		entry.datatype = datatype;
		entry.itemCount = itemCount;
		entry.itemAction = itemAction;

		strncpy(entry.szName, name, sizeof(entry.szName));
		strncpy(entry.szItemName, itemName, sizeof(entry.szItemName));
		//m_GuildInfo.log.emplace(std::pair<int, BankLog>(index, entry));
		log[index] = entry;
	}
	std::map<int, BankLog> GetLogs()
	{
		return log;
	}
private:
	std::map<int, BankLog> log;
	//BankLog log[1024];
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
public:
	void SetGuildDonateInfo(int medals) { m_GuildInfo.bMedals = medals; }
	void SetGuilDonateCount(int count) { m_GuildInfo.bDonateCount = count; }
#endif

protected:
	void __CalculateLevelAverage();
	void __SortMember();
	BOOL __IsGradeData(uint8_t byGradeNumber);

	void __Initialize();

protected:
	TGuildInfo m_GuildInfo;
	TGradeDataMap m_GradeDataMap;
	TGuildMemberDataVector m_GuildMemberDataVector;
	TGuildBoardCommentDataVector m_GuildBoardCommentVector;
	TGuildSkillData m_GuildSkillData;
	TGuildNameMap m_GuildNameMap;
	uint32_t m_adwEnemyGuildID[ENEMY_GUILD_SLOT_MAX_COUNT];

	uint32_t m_dwMemberLevelSummary;
	uint32_t m_dwMemberLevelAverage;
	uint32_t m_dwMemberExperienceSummary;

	BOOL m_bGuildEnable;
};