#ifndef __INC_METIN_II_GAME_PARTY_H__
#define __INC_METIN_II_GAME_PARTY_H__

#include "char.h"

enum // unit : minute
{
	PARTY_ENOUGH_MINUTE_FOR_EXP_BONUS = 60, // 파티 결성 후 60분 후 부터 추가 경험치 보너스
	PARTY_HEAL_COOLTIME_LONG = 60,
	PARTY_HEAL_COOLTIME_SHORT = 30,
	PARTY_MAX_MEMBER = 8,
	PARTY_DEFAULT_RANGE = 5000,
};

enum EPartyRole
{
	PARTY_ROLE_NORMAL, //Player
	PARTY_ROLE_LEADER, //Leader

	PARTY_ROLE_ATTACKER, //Angreifer
	PARTY_ROLE_TANKER, //Nahk?pfer
	PARTY_ROLE_BUFFER, //Blocker
	PARTY_ROLE_SKILL_MASTER,//Fertigkeitsmeister
	PARTY_ROLE_HASTE, //Berserker
	PARTY_ROLE_DEFENDER, //Verteidiger
	PARTY_ROLE_MAX_NUM,
};

enum EPartyExpDistributionModes
{
	PARTY_EXP_DISTRIBUTION_NON_PARITY,
	PARTY_EXP_DISTRIBUTION_PARITY,
#ifdef ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL
	PARTY_EXP_DISTRIBUTION_EQUAL,
#endif
	PARTY_EXP_DISTRIBUTION_MAX_NUM
};

class CParty;
class CDungeon;
#ifdef ENABLE_12ZI
class CZodiac;
#endif

class CPartyManager : public singleton<CPartyManager>
{
public:
	typedef std::map<uint32_t, LPPARTY> TPartyMap;
	typedef std::set<LPPARTY> TPCPartySet;

public:
	CPartyManager();
	virtual ~CPartyManager();

	void Initialize();

	//void SendPartyToDB();

	void EnablePCParty() noexcept { m_bEnablePCParty = true; sys_log(0, "PARTY Enable"); }
	void DisablePCParty() noexcept { m_bEnablePCParty = false; sys_log(0, "PARTY Disable"); }
	bool IsEnablePCParty() noexcept { return m_bEnablePCParty; }

	LPPARTY CreateParty(LPCHARACTER pkLeader);
	void DeleteParty(LPPARTY pParty);
	void DeleteAllParty();
	bool SetParty(LPCHARACTER pkChr);

	void SetPartyMember(uint32_t dwPID, LPPARTY pParty);

	void P2PLogin(uint32_t pid, const char* name);
	void P2PLogout(uint32_t pid);

	LPPARTY P2PCreateParty(uint32_t pid);
	void P2PDeleteParty(uint32_t pid);
	void P2PJoinParty(uint32_t leader, uint32_t pid, uint8_t role = 0);
	void P2PQuitParty(uint32_t pid);

private:
	TPartyMap m_map_pkParty; // PID로 어느 파티에 있나 검색하기 위한 컨테이너
	TPartyMap m_map_pkMobParty; // Mob 파티는 PID 대신 VID 로 따로 관리한다.

	TPCPartySet m_set_pkPCParty; // 사람들의 파티 전체 집합

	bool m_bEnablePCParty; // 디비가 켜져있지 않으면 사람들의 파티 상태가 변경불가
};

enum EPartyMessages
{
	PM_ATTACK, // Attack him
	PM_RETURN, // Return back to position
	PM_ATTACKED_BY, // I was attacked by someone
	PM_AGGRO_INCREASE, // My aggro is increased
};

class CParty
{
public:
	typedef struct SMember
	{
		LPCHARACTER pCharacter;
		bool bNear;
		uint8_t bRole;
		uint8_t bLevel;
		std::string strName;
	} TMember;

	typedef std::map<uint32_t, TMember> TMemberMap;

	typedef std::map<std::string, int> TFlagMap;

public:
	CParty();
	virtual ~CParty();

	void P2PJoin(uint32_t dwPID);
	void P2PQuit(uint32_t dwPID);
	virtual void Join(uint32_t dwPID);
	void Quit(uint32_t dwPID);
	void Link(LPCHARACTER pkChr);
	void Unlink(LPCHARACTER pkChr);

	void ChatPacketToAllMember(uint8_t type, const char* format, ...);

	void UpdateOnlineState(uint32_t dwPID, const char* name);
	void UpdateOfflineState(uint32_t dwPID);

	bool IsAllMembersPoly();

	uint32_t GetLeaderPID() noexcept;
#ifdef ENABLE_PARTY_FEATURES
	const char* GetLeaderName();
#endif
	LPCHARACTER GetLeaderCharacter();
	LPCHARACTER GetLeader() noexcept { return m_pkChrLeader; }

	uint32_t GetMemberCount() noexcept;
	uint32_t GetNearMemberCount() noexcept { return m_iCountNearPartyMember; }

	bool IsMember(uint32_t pid) { return m_memberMap.find(pid) != m_memberMap.end(); }

	bool IsNearLeader(uint32_t pid);

	bool IsPositionNearLeader(const LPCHARACTER& ch);

	void SendMessage(LPCHARACTER ch, uint8_t bMsg, uint32_t dwArg1, uint32_t dwArg2);

	void SendPartyJoinOneToAll(uint32_t dwPID);
	void SendPartyJoinAllToOne(const LPCHARACTER& ch);
	void SendPartyRemoveOneToAll(uint32_t dwPID);

	void SendPartyInfoOneToAll(uint32_t pid);
	void SendPartyInfoOneToAll(LPCHARACTER ch);
	void SendPartyInfoAllToOne(const LPCHARACTER& ch);

	void SendPartyLinkOneToAll(const LPCHARACTER& ch);
	void SendPartyLinkAllToOne(const LPCHARACTER& ch);
	void SendPartyUnlinkOneToAll(const LPCHARACTER& ch);
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	void SendPositionInfo() const;
#endif

	int GetPartyBonusExpPercent() noexcept { return m_iExpBonus; }
	int GetPartyBonusAttackGrade() noexcept { return m_iAttBonus; }
	int GetPartyBonusDefenseGrade() noexcept { return m_iDefBonus; }

	int ComputePartyBonusExpPercent();
	inline int ComputePartyBonusAttackGrade() noexcept;
	inline int ComputePartyBonusDefenseGrade() noexcept;

	template <class Func> void ForEachMember(Func& f);
	template <class Func> void ForEachMemberPtr(Func& f);
	template <class Func> void ForEachOnlineMember(Func& f);
	template <class Func> void ForEachNearMember(Func& f);
	template <class Func> void ForEachOnMapMember(Func& f, long lMapIndex);
	template <class Func> bool ForEachOnMapMemberBool(Func& f, long lMapIndex);

	void Update();

	int GetExpBonusPercent();

	bool SetRole(uint32_t pid, uint8_t bRole, bool on);
	uint8_t GetRole(uint32_t pid);
	bool IsRole(uint32_t pid, uint8_t bRole);

	uint8_t GetMemberMaxLevel() noexcept;
	uint8_t GetMemberMinLevel() noexcept;

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	uint8_t GetMemberChannel(uint32_t dwID);
	long GetMemberMapIndex(uint32_t dwID);
#endif

#ifdef ENABLE_PASSIVE_ATTR
	void ComputeRolePoint(LPCHARACTER ch, LPCHARACTER pkLeader, uint8_t bRole, bool bAdd);
#else
	void ComputeRolePoint(LPCHARACTER ch, uint8_t bRole, bool bAdd);
#endif

	void HealParty();
	void SummonToLeader(uint32_t pid);

	void SetPCParty(bool b) noexcept { m_bPCParty = b; }

	LPCHARACTER GetNextOwnership(LPCHARACTER ch, long x, long y);

	void SetFlag(const std::string& name, int value);
	int GetFlag(const std::string& name);

	void SetDungeon(LPDUNGEON pDungeon) noexcept;
	LPDUNGEON GetDungeon() noexcept;
#ifdef ENABLE_12ZI
	void SetZodiac(LPZODIAC pZodiac);
	LPZODIAC GetZodiac();
#endif

	uint8_t CountMemberByVnum(uint32_t dwVnum);

	void SetParameter(int iMode);
	int GetExpDistributionMode() noexcept;

	void SetExpCentralizeCharacter(uint32_t pid);
	LPCHARACTER GetExpCentralizeCharacter() noexcept;

	void RequestSetMemberLevel(uint32_t pid, uint8_t level);
	void P2PSetMemberLevel(uint32_t pid, uint8_t level);

	bool IsPartyInDungeon(int mapIndex);

protected:
	void IncreaseOwnership() noexcept;

	virtual void Initialize();
	void Destroy();

	void RemoveBonus();
	void RemoveBonusForOne(uint32_t pid);

	void SendParameter(const LPCHARACTER& ch);
	void SendParameterToAll();

	TMemberMap m_memberMap;
	uint32_t m_dwLeaderPID;
	LPCHARACTER m_pkChrLeader;

	LPEVENT m_eventUpdate;
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	LPEVENT m_PositionInfoEvent;
#endif

	TMemberMap::iterator m_itNextOwner;

private:
	int m_iExpDistributionMode;
	LPCHARACTER m_pkChrExpCentralize;

	uint32_t m_dwPartyStartTime;

	uint32_t m_dwPartyHealTime;
	bool m_bPartyHealReady;
	bool m_bCanUsePartyHeal;

	int m_anRoleCount[PARTY_ROLE_MAX_NUM];
	int m_anMaxRole[PARTY_ROLE_MAX_NUM];

	int m_iLongTimeExpBonus;

	// used in Update
	int m_iLeadership;
	int m_iExpBonus;
	int m_iAttBonus;
	int m_iDefBonus;

	// changed only in Update
	int m_iCountNearPartyMember;

	bool m_bPCParty;

	TFlagMap m_map_iFlag;

	LPDUNGEON m_pkDungeon;
	LPDUNGEON m_pkDungeon_for_Only_party;
#ifdef ENABLE_12ZI
	LPZODIAC m_pkZodiac;
	LPZODIAC m_pkZodiac_for_Only_party;
#endif

public:
	void SetDungeon_for_Only_party(LPDUNGEON pDungeon) noexcept;
	LPDUNGEON GetDungeon_for_Only_party() noexcept;
#ifdef ENABLE_12ZI
	void SetZodiac_for_Only_party(LPZODIAC pZodiac);
	LPZODIAC GetZodiac_for_Only_party();
#endif
};

template <class Func> void CParty::ForEachMember(Func& f)
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
		f(it->first);
}

template <class Func> void CParty::ForEachMemberPtr(Func& f)
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
		f(it->second.pCharacter);
}

template <class Func> void CParty::ForEachOnlineMember(Func& f)
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
		if (it->second.pCharacter)
			f(it->second.pCharacter);
}

template <class Func> void CParty::ForEachNearMember(Func& f)
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
		if (it->second.pCharacter && it->second.bNear)
			f(it->second.pCharacter);
}

template <class Func> void CParty::ForEachOnMapMember(Func& f, long lMapIndex)
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		LPCHARACTER ch = it->second.pCharacter;
		if (ch)
		{
			if (ch->GetMapIndex() == lMapIndex)
				f(ch);
		}
	}
}

template <class Func> bool CParty::ForEachOnMapMemberBool(Func& f, long lMapIndex)
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		LPCHARACTER ch = it->second.pCharacter;
		if (ch)
		{
			if (ch->GetMapIndex() == lMapIndex)
			{
				if (f(ch) == false)
				{
					return false;

				}
			}
		}
	}
	return true;
}

inline int CParty::ComputePartyBonusAttackGrade() noexcept
{
	/*
	if (GetNearMemberCount() <= 1)
	return 0;

	int leadership = GetLeaderCharacter()->GetLeadershipSkillLevel();
	int n = GetNearMemberCount();

	if (n >= 3 && leadership >= 10)
	return 2;

	if (n >= 2 && leadership >= 4)
	return 1;
	*/
	return 0;
}

inline int CParty::ComputePartyBonusDefenseGrade() noexcept
{
	/*
	if (GetNearMemberCount() <= 1)
	return 0;

	int leadership = GetLeaderCharacter()->GetLeadershipSkillLevel();
	int n = GetNearMemberCount();

	if (n >= 5 && leadership >= 24)
	return 2;

	if (n >= 4 && leadership >= 16)
	return 1;
	*/
	return 0;
}


#ifdef ENABLE_DICE_SYSTEM
#include "item.h"

struct FPartyDropDiceRoll
{
	const LPITEM m_itemDrop;
	LPCHARACTER m_itemOwner;
	int m_lastNumber;

	FPartyDropDiceRoll(const LPITEM itemDrop, LPCHARACTER itemOwner) : m_itemDrop(itemDrop), m_itemOwner(itemOwner), m_lastNumber(0)
	{
	};

	const void Process(const LPCHARACTER mobVictim)
	{
		if ((!mobVictim || (mobVictim->GetMobRank() >= MOB_RANK_BOSS && mobVictim->GetMobRank() <= MOB_RANK_KING)) && m_itemOwner->GetParty() && m_itemOwner->GetParty()->GetNearMemberCount() > 1)
		{
			LPPARTY pParty = m_itemOwner->GetParty();
			if (!pParty)
				return;
			pParty->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "[LS;1201;%s]", m_itemDrop->GetName());

			pParty->ForEachNearMember(*this);
			if (m_itemOwner)
			{
				m_itemDrop->SetOwnership(m_itemOwner);
				pParty->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "[LS;1203;%s;%s]", m_itemDrop->GetName(), m_itemOwner->GetName());
			}
		}
		else
			m_itemDrop->SetOwnership(m_itemOwner);
	}
	LPCHARACTER GetItemOwner() noexcept
	{
		return m_itemOwner;
	}
	const LPITEM GetItemDrop() noexcept
	{
		return m_itemDrop;
	}
	void operator () (LPCHARACTER ch)
	{
		if (!ch)
			return;

		LPPARTY pParty = ch->GetParty();
		if (!pParty)
			return;

		while (true)
		{
			const int pickedNumber = number(10000, 99999);
			if (pickedNumber > m_lastNumber)
			{
				m_lastNumber = pickedNumber;
				m_itemOwner = ch;
			}
			else if (pickedNumber == m_lastNumber)
			{
				continue;
			}
			else // if (pickedNumber < m_lastNumber)
			{
			}
			pParty->ChatPacketToAllMember(CHAT_TYPE_DICE_INFO, "[LS;1202;%s;%d]", ch->GetName(), pickedNumber);
			break;
		}
	}
};
#endif

#endif
