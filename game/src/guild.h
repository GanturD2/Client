#ifndef __INC_GUILD_H
#define __INC_GUILD_H

#include "skill.h"
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
#define GS_ITEM 1
#define GS_GOLD 2
#endif

typedef struct _SQLMsg SQLMsg;

enum
{
	GUILD_GRADE_NAME_MAX_LEN = GUILD_GRADE_NAME_MAXLEN,
	GUILD_GRADE_COUNT = GUILD_GRADECOUNT,
	GUILD_COMMENT_MAX_COUNT = GUILD_COMMENT_MAXCOUNT,
	GUILD_COMMENT_MAX_LEN = GUILD_COMMENT_MAXLEN,
	GUILD_LEADER_GRADE = 1,
	GUILD_COLEADER_GRADE = 2,	// ENABLE_SHOW_GUILD_LEADER
	GUILD_BASE_POWER = GUILD_BASEPOWER,
	GUILD_POWER_PER_SKILL_LEVEL = GUILD_POWER_PER_SKILL_LEVEL_,
	GUILD_POWER_PER_LEVEL = GUILD_POWER_PER_LEVEL_,
	GUILD_MINIMUM_LEADERSHIP = GUILD_MINIMUM_LEADERSHIP_,
	GUILD_WAR_MIN_MEMBER_COUNT = GUILDWAR_MIN_MEMBER_COUNT,
	GUILD_LADDER_POINT_PER_LEVEL = GUILD_LADDERPOINT_PER_LEVEL,
	GUILD_CREATE_ITEM_VNUM = GUILD_CREATE_ITEMVNUM,
};

struct SGuildMaster
{
	uint32_t pid;
};


typedef struct SGuildMember
{
	uint32_t pid;
	uint8_t grade;
	uint8_t is_general;
	uint8_t job;
	uint8_t level;
	uint32_t offer_exp;
	uint8_t _dummy;
	std::string name;
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	uint32_t join_date;			// Time Check
	uint8_t donate_limit;		// Exp-Donateboard -> 0/3 Limit
	uint32_t last_donation;		// Time Check
	uint8_t daily_donate_count;	// DonationCount
	uint32_t last_daily_donate;	// Lock/Unlock
#endif

	SGuildMember(LPCHARACTER ch, uint8_t grade, uint32_t offer_exp
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		, uint32_t join_date, uint8_t donate_limit, uint32_t last_donation, uint8_t daily_donate_count, uint32_t last_daily_donate
#endif
	);
	SGuildMember(uint32_t pid, uint8_t grade, uint8_t is_general, uint8_t job, uint8_t level, uint32_t offer_exp, char* name
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		, uint32_t join_date, uint8_t donate_limit, uint32_t last_donation, uint8_t daily_donate_count, uint32_t last_daily_donate
#endif
	);
} TGuildMember;

#pragma pack(1)
typedef struct SGuildMemberPacketData
{
	uint32_t pid;
	uint8_t grade;
	uint8_t is_general;
	uint8_t job;
	uint8_t level;
	uint32_t offer;
	uint8_t name_flag;
	char name[CHARACTER_NAME_MAX_LEN + 1];
} TGuildMemberPacketData;

typedef struct packet_guild_sub_info
{
	uint16_t member_count;
	uint16_t max_member_count;
	uint32_t guild_id;
	uint32_t master_pid;
	uint32_t exp;
	uint8_t level;
	char name[GUILD_NAME_MAX_LEN + 1];
	uint32_t gold;
	uint8_t has_land;
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	char goldCheckout[CHARACTER_NAME_MAX_LEN + 1];
	char itemCheckout[CHARACTER_NAME_MAX_LEN + 1];
#endif
} TPacketGCGuildInfo;

typedef struct SGuildGrade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN + 1]; // 8+1 Name of guild leader, guild member, etc.
	uint8_t auth_flag;
} TGuildGrade;

struct TOneGradeNamePacket
{
	uint8_t grade;
	char grade_name[GUILD_GRADE_NAME_MAX_LEN + 1];
};

struct TOneGradeAuthPacket
{
	uint8_t grade;
	uint8_t auth;
};
#pragma pack()

enum
{
	GUILD_AUTH_ADD_MEMBER = (1 << 0),
	GUILD_AUTH_REMOVE_MEMBER = (1 << 1),
	GUILD_AUTH_NOTICE = (1 << 2),
	GUILD_AUTH_USE_SKILL = (1 << 3),
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	GUILD_AUTH_WAR = (1 << 4),
	GUILD_AUTH_BANK = (1 << 5),
#endif
};

typedef struct SGuildData
{
	uint32_t guild_id;
	uint32_t master_pid;
	uint32_t exp;
	uint8_t level;
	char name[GUILD_NAME_MAX_LEN + 1];

	TGuildGrade grade_array[GUILD_GRADE_COUNT];

	uint8_t skill_point;
	uint8_t abySkill[GUILD_SKILL_COUNT];

	int power;
	int max_power;

	int ladder_point;

	int win;
	int draw;
	int loss;

	int gold;

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	uint8_t dungeon_ch;
	long dungeon_map;
	uint32_t dungeon_cooldown;
#endif
#ifdef ENABLE_GUILD_WAR_SCORE
	int winTypes[3];
	int drawTypes[3];
	int lossType[3];
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	int guildstorage;
	char lastGold[CHARACTER_NAME_MAX_LEN + 1];
	char lastItem[CHARACTER_NAME_MAX_LEN + 1];
	int guildstoragestate;
	int guildstoragewho;
#endif
} TGuildData;

struct TGuildCreateParameter
{
	LPCHARACTER master;
	char name[GUILD_NAME_MAX_LEN + 1];
};

typedef struct SGuildWar
{
	uint32_t war_start_time;
	uint32_t score;
	uint32_t state;
	uint8_t type;
#ifdef ENABLE_NEW_WAR_OPTIONS
	uint8_t round;
	uint8_t points;
	uint8_t time;
#endif
	uint32_t map_index;

	SGuildWar(uint8_t type)
		: war_start_time(0),
		score(0),
		state(GUILD_WAR_RECV_DECLARE),
		type(type),
#ifdef ENABLE_NEW_WAR_OPTIONS
		round(0),
		points(0),
		time(0),
#endif
		map_index(0)
	{
	}
	bool IsWarBegin() const
	{
		return state == GUILD_WAR_ON_WAR;
	}
} TGuildWar;

class CGuild
{
public:
	CGuild(TGuildCreateParameter& cp);
	explicit CGuild(uint32_t guild_id) { Load(guild_id); }
	~CGuild();

	uint32_t GetID() const noexcept { return m_data.guild_id; }
	const char* GetName() const noexcept { return m_data.name; }
	int GetSP() const noexcept { return m_data.power; }
	int GetMaxSP() noexcept { return m_data.max_power; }
	uint32_t GetMasterPID() const noexcept { return m_data.master_pid; }
	LPCHARACTER GetMasterCharacter();
#ifdef ENABLE_COLEADER_WAR_PRIVILEGES
	bool IsOnlineLeader();
#endif
	uint8_t GetLevel() const noexcept { return m_data.level; }
#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	uint32_t GetExp() const { return m_data.exp; }
#endif

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void SetStorageState(bool val, uint32_t pid);
	bool IsStorageOpen() { return m_data.guildstoragestate; }
	uint32_t GetStoragePid() { return m_data.guildstoragewho; }

	int GetGuildstorage() { return m_data.guildstorage; }
	void SetGuildstorage(int val);
#endif
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_GUILDSTORAGE_SYSTEM)
	void RefreshP2P(CGuild* p);
	void SetLastCheckout(const char* szName, int type = -1);
	void SetLastCheckoutP2P(const char* szName, int type = -1);
	const char* GetLastCheckout(int type = -1);
#endif

	void Reset() noexcept { m_data.power = m_data.max_power; }

	void RequestDisband(uint32_t pid);
	void Disband();

	void RequestAddMember(LPCHARACTER ch, int grade = 15);
	void AddMember(TPacketDGGuildMember* p);

	bool RequestRemoveMember(uint32_t pid);
	bool RemoveMember(uint32_t pid);

	void LoginMember(LPCHARACTER ch);
	void P2PLoginMember(uint32_t pid);

	void LogoutMember(LPCHARACTER ch);
	void P2PLogoutMember(uint32_t pid);

	void ChangeMemberGrade(uint32_t pid, uint8_t grade);
	bool OfferExp(LPCHARACTER ch, int amount);
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	bool OfferExpNew(LPCHARACTER ch, int amount);
#endif
	void LevelChange(uint32_t pid, uint8_t level);

	void ChangeMemberData(uint32_t pid, uint32_t offer, uint8_t level, uint8_t grade
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		, uint32_t join_date, uint8_t donate_limit, uint32_t last_donation, uint8_t daily_donate_count, uint32_t last_daily_donate
#endif
	);

	void ChangeGradeName(uint8_t grade, const char* grade_name);
	void ChangeGradeAuth(uint8_t grade, uint8_t auth);
	void P2PChangeGrade(uint8_t grade);

	bool ChangeMemberGeneral(uint32_t pid, uint8_t is_general);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) || defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
	uint8_t IsGeneralMember(uint32_t pid) noexcept;
#endif

	bool ChangeMasterTo(uint32_t dwPID);

	void Packet(const void* buf, int size);

	void SendOnlineRemoveOnePacket(uint32_t pid);
	void SendAllGradePacket(LPCHARACTER ch);
	void SendListPacket(LPCHARACTER ch);
	void SendListOneToAll(uint32_t pid);
	void SendListOneToAll(LPCHARACTER ch);
	void SendLoginPacket(LPCHARACTER ch, LPCHARACTER chLogin);
	void SendLogoutPacket(LPCHARACTER ch, LPCHARACTER chLogout);
	void SendLoginPacket(LPCHARACTER ch, uint32_t pid);
	void SendLogoutPacket(LPCHARACTER ch, uint32_t pid);
	void SendGuildInfoPacket(LPCHARACTER ch);
	void SendGuildDataUpdateToAllMember(SQLMsg* pmsg);
#ifdef ENABLE_GUILDWAR_BUTTON
	void SendAllMembersButtonState(bool show);
#endif

	void Load(uint32_t guild_id);
	void SaveLevel();
	void SaveSkill();
	void SaveMember(uint32_t pid);

	int GetMaxMemberCount();
	int GetMemberCount() noexcept { return m_member.size(); }
	int GetTotalLevel() const;

	// GUILD_MEMBER_COUNT_BONUS
	void SetMemberCountBonus(int iBonus);
	void BroadcastMemberCountBonus();
	// END_OF_GUILD_MEMBER_COUNT_BONUS

	int GetMaxGeneralCount() const { return 1 /*+ GetSkillLevel(GUILD_SKILL_DEUNGYONG)/3*/; }
	int GetGeneralCount() const noexcept { return m_general_count; }

	TGuildMember* GetMember(uint32_t pid);
	uint32_t GetMemberPID(const std::string& strName);

#ifdef ENABLE_SHOW_GUILD_LEADER
	uint8_t NewIsGuildGeneral(uint32_t pid);
	bool IsCoLeader(uint32_t pid);
#endif

	bool HasGradeAuth(int grade, int auth_flag) const noexcept { return (bool)(m_data.grade_array[grade - 1].auth_flag & auth_flag); }

	void AddComment(LPCHARACTER ch, const std::string& str);
	void DeleteComment(LPCHARACTER ch, uint32_t comment_id);

	void RefreshComment(LPCHARACTER ch);
	void RefreshCommentForce(uint32_t player_id);

	int GetSkillLevel(uint32_t vnum);
	void SkillLevelUp(uint32_t dwVnum);
	void UseSkill(uint32_t dwVnum, LPCHARACTER ch, uint32_t pid);

	void SendSkillInfoPacket(LPCHARACTER ch) const;
	void ComputeGuildPoints();

	void GuildPointChange(uint16_t type, int amount, bool save = false); //@fixme532

	//void GuildUpdateAffect(LPCHARACTER ch);
	//void GuildRemoveAffect(LPCHARACTER ch);

	void UpdateSkill(uint8_t grade, uint8_t* skill_levels);
	void SendDBSkillUpdate(int amount = 0);

	void SkillRecharge();
	bool ChargeSP(LPCHARACTER ch, int iSP);

	void Chat(const char* c_pszText);
	void P2PChat(const char* c_pszText);

	void SkillUsableChange(uint32_t dwSkillVnum, bool bUsable);
	void AdvanceLevel(int iLevel);

	// Guild Money
	void RequestDepositMoney(LPCHARACTER ch, int iGold);
#ifdef ENABLE_USE_MONEY_FROM_GUILD
	void RequestWithdrawMoney(LPCHARACTER ch, int iGold, bool buildPrice = true);
#else
	void RequestWithdrawMoney(LPCHARACTER ch, int iGold);
#endif

	void RecvMoneyChange(int iGold);
#ifndef ENABLE_USE_MONEY_FROM_GUILD
	void RecvWithdrawMoneyGive(int iChangeGold);
#endif

	int GetGuildMoney() const noexcept { return m_data.gold; }
	void SetGuildMoney(int gold) { m_data.gold = gold; }

	// War general
#ifdef ENABLE_GUILD_WAR_SCORE
	void SetWarInfo();
#endif
#ifdef ENABLE_GUILD_LAND_INFO
	int GetGuildLandIndex();
	int GetGuildLandID();
	void SetGuildLandInfo();
#endif
#ifdef ENABLE_GUILDBANK_LOG
	void SetBankInfo();
#endif

	// Guild Donate
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	void SetGuildDonateInfo();
	uint32_t GetMemberJoinDate(uint32_t pid) noexcept;
	int GetGuildMemberDonateCount(uint32_t pid) noexcept;
	uint32_t GetMemberLastDonation(uint32_t pid) noexcept;

	int GetDailyGuildDonatePoints(uint32_t pid) noexcept;
	uint32_t GetMemberLastDailyDonate(uint32_t pid) noexcept;
	void SetGuildAttendanceState(uint32_t pid, int donateCount, uint32_t dwTime);
	void ResetGuildMemberDonateCount(uint32_t pid);
#endif

	// War general
#ifdef ENABLE_NEW_WAR_OPTIONS
	void GuildWarPacket(uint32_t guild_id, uint8_t bWarType, uint8_t bWarState, uint8_t bRound, uint8_t bPoints, uint8_t bTime);
#else
	void GuildWarPacket(uint32_t guild_id, uint8_t bWarType, uint8_t bWarState);
#endif
	void SendEnemyGuild(LPCHARACTER ch);

	uint8_t GetGuildWarState(uint32_t guild_id);
	bool CanStartWar(uint8_t bGuildWarType);
	uint32_t GetWarStartTime(uint32_t guild_id);
	bool UnderWar(uint32_t guild_id); // Are you at war?
	uint32_t UnderAnyWar(uint8_t bType = GUILD_WAR_TYPE_MAX_NUM);

	// War map relative
	void SetGuildWarMapIndex(uint32_t dwGuildID, long lMapIndex);
	uint8_t GetGuildWarType(uint32_t dwGuildOpponent);
	uint32_t GetGuildWarMapIndex(uint32_t dwGuildOpponent);
#ifdef ENABLE_NEW_WAR_OPTIONS
	int GetGuildWarSettingRound(uint32_t dwOppGID);
	int GetGuildWarSettingPoints(uint32_t dwOppGID);
	int GetGuildWarSettingTime(uint32_t dwOppGID);
#endif

	// War entry question
	void GuildWarEntryAsk(uint32_t guild_opp);
	void GuildWarEntryAccept(uint32_t guild_opp, LPCHARACTER ch);

	// War state relative
	void NotifyGuildMaster(const char* msg);
#ifdef ENABLE_NEW_WAR_OPTIONS
	void RequestDeclareWar(uint32_t guild_id, uint8_t type, uint8_t flag, uint8_t round, uint8_t points, uint32_t time);
#else
	void RequestDeclareWar(uint32_t guild_id, uint8_t type);
#endif
	void RequestRefuseWar(uint32_t guild_id);

#ifdef ENABLE_NEW_WAR_OPTIONS
	bool DeclareWar(uint32_t guild_id, uint8_t type, uint8_t state, uint8_t round, uint8_t points, uint8_t time);
#else
	bool DeclareWar(uint32_t guild_id, uint8_t type, uint8_t state);
#endif
	void RefuseWar(uint32_t guild_id);
	bool WaitStartWar(uint32_t guild_id);
	bool CheckStartWar(uint32_t guild_id); // check if StartWar method fails (call it before StartWar)
	void StartWar(uint32_t guild_id);
	void EndWar(uint32_t guild_id);
	void ReserveWar(uint32_t guild_id, uint8_t type);

	// War points relative
	void SetWarScoreAgainstTo(uint32_t guild_opponent, int newpoint);
	int GetWarScoreAgainstTo(uint32_t guild_opponent);
#ifdef ENABLE_GUILD_RANKING
	int GetWarWinPointAgainstTo(uint32_t dwOppGID);
#endif

	int GetLadderPoint() const noexcept { return m_data.ladder_point; }
	void SetLadderPoint(int point);

	void SetWarData(int iWin, int iDraw, int iLoss) noexcept { m_data.win = iWin, m_data.draw = iDraw, m_data.loss = iLoss; }
#ifdef ENABLE_GUILD_WAR_SCORE
	void SetNewWarData(long iWin[3], long iDraw[3], long iLoss[3]) noexcept
	{
		for (int i = 0; i < 3; ++i)
		{
			m_data.winTypes[i] = iWin[i];
			m_data.drawTypes[i] = iDraw[i];
			m_data.lossType[i] = iLoss[i];
		}
	}
#endif

	void ChangeLadderPoint(int iChange);

	int GetGuildWarWinCount() const noexcept { return m_data.win; }
	int GetGuildWarDrawCount() const noexcept { return m_data.draw; }
	int GetGuildWarLossCount() const noexcept { return m_data.loss; }

	bool HasLand();

	// GUILD_JOIN_BUG_FIX
	/// Invite character to join the guild.
	/**
	* @param pchInviter invited character.
	* @param pchInvitee invite character.
	*
	* If it is not possible to invite or receive, the corresponding chat message is sent.
	*/
	void Invite(LPCHARACTER pchInviter, LPCHARACTER pchInvitee);

	/// Handles acceptance of the opponent's character for guild invitation.
	/**
	* @param pchInvitee invited character
	*
	* If it is not possible to join the guild, the corresponding chat message is sent.
	*/
	void InviteAccept(LPCHARACTER pchInvitee);

	/// Handles rejection of the opponent's character for guild invitation.
	/**
	* @param dwPID PID of the invited character
	*/
	void InviteDeny(uint32_t dwPID);
	// END_OF_GUILD_JOIN_BUG_FIX

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
	int GetGuildLandDeletePulse() const { return m_guildLandDeletePulse; }
	void SetGuildLandDeletePulse(int landPulse) { m_guildLandDeletePulse = landPulse; }
	int m_guildLandDeletePulse;
#endif

private:
	void Initialize();

	TGuildData m_data;
	int m_general_count;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	bool storageOpen;
	uint32_t storagePidOpen;
#endif

	// GUILD_MEMBER_COUNT_BONUS
	int m_iMemberCountBonus;
	// END_OF_GUILD_MEMBER_COUNT_BONUS

	typedef std::map<uint32_t, TGuildMember> TGuildMemberContainer;
	TGuildMemberContainer m_member;

	typedef CHARACTER_SET TGuildMemberOnlineContainer;
	TGuildMemberOnlineContainer m_memberOnline;

	typedef std::set<uint32_t> TGuildMemberP2POnlineContainer;
	TGuildMemberP2POnlineContainer m_memberP2POnline;

	void LoadGuildData(SQLMsg* pmsg);
	void LoadGuildGradeData(SQLMsg* pmsg);
	void LoadGuildMemberData(SQLMsg* pmsg);

	void __P2PUpdateGrade(SQLMsg* pmsg);

	typedef std::map<uint32_t, TGuildWar> TEnemyGuildContainer;
	TEnemyGuildContainer m_EnemyGuild;

	std::map<uint32_t, uint32_t> m_mapGuildWarEndTime;

	bool abSkillUsable[GUILD_SKILL_COUNT];

	// GUILD_JOIN_BUG_FIX
	/// Error code when you cannot join the guild.
	enum GuildJoinErrCode
	{
		GERR_NONE = 0, ///< Processing success
		GERR_WITHDRAWPENALTY, ///< After withdrawal, the time available for joining has not passed.
		GERR_COMMISSIONPENALTY, ///< After dissolution, the time available for sign-up has not passed.
		GERR_ALREADYJOIN, ///< The character targeted for guild membership has already joined the guild
		GERR_GUILDISFULL, ///< Guild number limit exceeded
		GERR_GUILD_IS_IN_WAR, ///< Guild is currently at war
		GERR_INVITE_LIMIT, ///< Guild member membership restrictions
		GERR_MAX ///< Highest error code. Add an error code before this.
	};

	/// Check the conditions for joining the guild.
	/**
	* @param [in] pchInvitee Invited character
	* @return GuildJoinErrCode
	*/
	GuildJoinErrCode VerifyGuildJoinableCondition(const LPCHARACTER pchInvitee);

	typedef std::map< uint32_t, LPEVENT > EventMap;
	EventMap m_GuildInviteEventMap; ///< Guild invitation event map. key: PID of the invited character
	// END_OF_GUILD_JOIN_BUG_FIX

public:
	template <class Func> void ForEachOnlineMember(Func f);

#ifdef ENABLE_D_NJGUILD
private:
	LPDUNGEON m_pkDungeon_for_Only_guild;

public:
	void SetDungeon_for_Only_guild(LPDUNGEON pDungeon);
	LPDUNGEON GetDungeon_for_Only_guild();
#endif
#if defined(ENABLE_D_NJGUILD) || defined(ENABLE_ACHIEVEMENT_SYSTEM)
	template <class Func> void ForEachOnMapMember(Func& f, long lMapIndex);
#endif
#ifdef ENABLE_NEWSTUFF
public:
	void SetSkillLevel(uint32_t dwVnum, uint8_t level, uint8_t point = 0);
	uint32_t GetSkillPoint();
	void SetSkillPoint(uint8_t point);
#endif
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
public:
	uint8_t GetDungeonCH() const noexcept { return m_data.dungeon_ch; }
	long GetDungeonMapIndex() const noexcept { return m_data.dungeon_map; }
	bool RequestDungeon(uint8_t bChannel, long lMapIndex);
	void RecvDungeon(uint8_t bChannel, long lMapIndex);
	uint32_t GetDungeonCooldown() const noexcept { return m_data.dungeon_cooldown; }
	bool SetDungeonCooldown(uint32_t dwTime);
	void RecvDungeonCD(uint32_t dwTime);
#endif
};

#if defined(ENABLE_D_NJGUILD) || defined(ENABLE_ACHIEVEMENT_SYSTEM)
#include "char.h"
template <class Func> void CGuild::ForEachOnMapMember(Func& f, long lMapIndex)
{
	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		if (ch)
		{
			if (ch->GetMapIndex() == lMapIndex)
				f(ch);
		}
	}
}
#endif

template <class Func> void CGuild::ForEachOnlineMember(Func f)
{
	TGuildMemberOnlineContainer::iterator it;

	for (it = m_memberOnline.begin(); it != m_memberOnline.end(); ++it)
	{
		LPCHARACTER ch = *it;
		if (ch)
			f(ch);
	}
}

#endif
