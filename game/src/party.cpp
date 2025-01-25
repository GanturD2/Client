#include "stdafx.h"
#include "utils.h"
#include "char.h"
#include "party.h"
#include "char_manager.h"
#include "config.h"
#include "p2p.h"
#include "desc_client.h"
#include "dungeon.h"
#include "unique_item.h"
#ifdef ENABLE_12ZI
#	include "zodiac_temple.h"
#endif
#ifdef ENABLE_PASSIVE_ATTR
#	include "skill.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif
#ifdef ENABLE_QUEEN_NETHIS
#	include "SnakeLair.h"
#endif

CPartyManager::CPartyManager()
{
	Initialize();
}

CPartyManager::~CPartyManager()
{
}

void CPartyManager::Initialize()
{
	m_bEnablePCParty = false;
}

void CPartyManager::DeleteAllParty()
{
	TPCPartySet::iterator it = m_set_pkPCParty.begin();

	while (it != m_set_pkPCParty.end())
	{
		DeleteParty(*it);
		it = m_set_pkPCParty.begin();
	}
}

bool CPartyManager::SetParty(LPCHARACTER ch) // You must use PC only!!
{
	if (!ch)
		return false;

	const TPartyMap::iterator it = m_map_pkParty.find(ch->GetPlayerID());

	if (it == m_map_pkParty.end())
		return false;

	LPPARTY pParty = it->second;
	if (!pParty)
		return false;

	pParty->Link(ch);
	return true;
}

void CPartyManager::P2PLogin(uint32_t pid, const char* name)
{
	const TPartyMap::iterator it = m_map_pkParty.find(pid);

	if (it == m_map_pkParty.end())
		return;

	it->second->UpdateOnlineState(pid, name);
}
void CPartyManager::P2PLogout(uint32_t pid)
{
	const TPartyMap::iterator it = m_map_pkParty.find(pid);

	if (it == m_map_pkParty.end())
		return;

	it->second->UpdateOfflineState(pid);
}

void CPartyManager::P2PJoinParty(uint32_t leader, uint32_t pid, uint8_t role)
{
	const TPartyMap::iterator it = m_map_pkParty.find(leader);

	if (it != m_map_pkParty.end())
	{
		it->second->P2PJoin(pid);

		if (role >= PARTY_ROLE_MAX_NUM)
			role = PARTY_ROLE_NORMAL;

		it->second->SetRole(pid, role, true);
	}
	else
	{
		sys_err("No such party with leader [%d]", leader);
	}
}

void CPartyManager::P2PQuitParty(uint32_t pid)
{
	const TPartyMap::iterator it = m_map_pkParty.find(pid);

	if (it != m_map_pkParty.end())
		it->second->P2PQuit(pid);
	else
		sys_err("No such party with member [%d]", pid);
}

LPPARTY CPartyManager::P2PCreateParty(uint32_t pid)
{
	const TPartyMap::iterator it = m_map_pkParty.find(pid);
	if (it != m_map_pkParty.end())
		return it->second;

	LPPARTY pParty = M2_NEW CParty;

	m_set_pkPCParty.insert(pParty);

	SetPartyMember(pid, pParty);
	pParty->SetPCParty(true);
	pParty->P2PJoin(pid);

	return pParty;
}

void CPartyManager::P2PDeleteParty(uint32_t pid)
{
	const TPartyMap::iterator it = m_map_pkParty.find(pid);

	if (it != m_map_pkParty.end())
	{
		m_set_pkPCParty.erase(it->second);
		M2_DELETE(it->second);
	}
	else
		sys_err("PARTY P2PDeleteParty Cannot find party [%u]", pid);
}

LPPARTY CPartyManager::CreateParty(LPCHARACTER pLeader)
{
	if (pLeader->GetParty())
		return pLeader->GetParty();

	LPPARTY pParty = M2_NEW CParty;

	if (pLeader->IsPC())
	{
		TPacketPartyCreate p{};
		p.dwLeaderPID = pLeader->GetPlayerID();

		db_clientdesc->DBPacket(HEADER_GD_PARTY_CREATE, 0, &p, sizeof(TPacketPartyCreate));

		sys_log(0, "PARTY: Create %s pid %u", pLeader->GetName(), pLeader->GetPlayerID());
		pParty->SetPCParty(true);
		pParty->Join(pLeader->GetPlayerID());

		m_set_pkPCParty.insert(pParty);
	}
	else
	{
		pParty->SetPCParty(false);
		pParty->Join(pLeader->GetVID());
	}

	pParty->Link(pLeader);
	return (pParty);
}

void CPartyManager::DeleteParty(LPPARTY pParty)
{
	if (!pParty)
		return;

	TPacketPartyDelete p{};
	p.dwLeaderPID = pParty->GetLeaderPID();

	db_clientdesc->DBPacket(HEADER_GD_PARTY_DELETE, 0, &p, sizeof(TPacketPartyDelete));

	m_set_pkPCParty.erase(pParty);
	M2_DELETE(pParty);
}

void CPartyManager::SetPartyMember(uint32_t dwPID, LPPARTY pParty)
{
	const TPartyMap::iterator it = m_map_pkParty.find(dwPID);

	if (pParty == nullptr)
	{
		if (it != m_map_pkParty.end())
			m_map_pkParty.erase(it);
	}
	else
	{
		if (it != m_map_pkParty.end())
		{
			if (it->second != pParty)
			{
				it->second->Quit(dwPID);
				it->second = pParty;
			}
		}
		else
			m_map_pkParty.insert(TPartyMap::value_type(dwPID, pParty));
	}
}

EVENTINFO(party_update_event_info)
{
	uint32_t pid;

	party_update_event_info() noexcept
		: pid(0)
	{
	}
};

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
EVENTINFO(party_position_event_info)
{
	LPPARTY pParty;
	party_position_event_info(LPPARTY mParty)
		: pParty(mParty)
	{}
};

EVENTFUNC(party_position_event)
{
	const party_position_event_info* info = dynamic_cast<party_position_event_info*>(event->info);

	if (info == nullptr || info->pParty == nullptr)
	{
		sys_err("party_position_event> <Factor> Null pointer");
		return 0;
	}

	info->pParty->SendPositionInfo();
	return PASSES_PER_SEC(3);
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CParty begin!
//
/////////////////////////////////////////////////////////////////////////////
EVENTFUNC(party_update_event)
{
	const party_update_event_info* info = dynamic_cast<party_update_event_info*>(event->info);

	if (info == nullptr)
	{
		sys_err("party_update_event> <Factor> Null pointer");
		return 0;
	}

	const uint32_t pid = info->pid;
	const LPCHARACTER& leader = CHARACTER_MANAGER::Instance().FindByPID(pid);

	if (leader && leader->GetDesc())
	{
		LPPARTY pParty = leader->GetParty();

		if (pParty)
			pParty->Update();
	}

	return PASSES_PER_SEC(3);
}

CParty::CParty()
{
	Initialize();
}

CParty::~CParty()
{
	Destroy();
}

void CParty::Initialize()
{
	sys_log(2, "Party::Initialize");

	m_iExpDistributionMode = PARTY_EXP_DISTRIBUTION_NON_PARITY;
	m_pkChrExpCentralize = nullptr;

	m_dwLeaderPID = 0;

	m_eventUpdate = nullptr;
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	m_PositionInfoEvent = nullptr;
#endif

	memset(&m_anRoleCount, 0, sizeof(m_anRoleCount));
	memset(&m_anMaxRole, 0, sizeof(m_anMaxRole));
	m_anMaxRole[PARTY_ROLE_LEADER] = 1;
	m_anMaxRole[PARTY_ROLE_NORMAL] = 32;

	m_dwPartyStartTime = get_dword_time();
	m_iLongTimeExpBonus = 0;

	m_dwPartyHealTime = get_dword_time();
	m_bPartyHealReady = false;
	m_bCanUsePartyHeal = false;

	m_iLeadership = 0;
	m_iExpBonus = 0;
	m_iAttBonus = 0;
	m_iDefBonus = 0;

	m_itNextOwner = m_memberMap.begin();

	m_iCountNearPartyMember = 0;

	m_pkChrLeader = nullptr;
	m_bPCParty = false;
	m_pkDungeon = nullptr;
	m_pkDungeon_for_Only_party = nullptr;
#ifdef ENABLE_12ZI
	m_pkZodiac = nullptr;
	m_pkZodiac_for_Only_party = nullptr;
#endif
}

void CParty::Destroy()
{
	// If the party was created by a PC, the party manager should delete the PID from the map.
	if (m_bPCParty)
	{
		for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
			CPartyManager::Instance().SetPartyMember(it->first, nullptr);
	}

	event_cancel(&m_eventUpdate);
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	event_cancel(&m_PositionInfoEvent);
#endif

	RemoveBonus();

	TMemberMap::iterator it = m_memberMap.begin();

	const uint32_t dwTime = get_dword_time();

	while (it != m_memberMap.end())
	{
		TMember& rMember = it->second;
		++it;

		if (rMember.pCharacter)
		{
			if (rMember.pCharacter->GetDesc())
			{
				TPacketGCPartyRemove p{};
				p.header = HEADER_GC_PARTY_REMOVE;
				p.pid = rMember.pCharacter->GetPlayerID();
				rMember.pCharacter->GetDesc()->Packet(&p, sizeof(p));
				rMember.pCharacter->ChatPacket(CHAT_TYPE_INFO, "[LS;922]");

#ifdef ENABLE_QUEEN_NETHIS
				if (SnakeLair::CSnk::instance().IsSnakeMap(rMember.pCharacter->GetMapIndex()))
				{
					SnakeLair::CSnk::instance().LeaveParty(rMember.pCharacter->GetMapIndex());
					SnakeLair::CSnk::instance().Leave(rMember.pCharacter);
				}
#endif
			}
			else
			{
				// In the case of an NPC, an event that disappears when not in battle starts after a certain period of time.
				rMember.pCharacter->SetLastAttacked(dwTime);
				rMember.pCharacter->StartDestroyWhenIdleEvent();
			}

			rMember.pCharacter->SetParty(nullptr);
		}
	}

	m_memberMap.clear();
	m_itNextOwner = m_memberMap.begin();

	if (m_pkDungeon_for_Only_party != nullptr)
	{
		m_pkDungeon_for_Only_party->SetPartyNull();
		m_pkDungeon_for_Only_party = nullptr;
	}

#ifdef ENABLE_12ZI
	if (m_pkZodiac_for_Only_party != nullptr)
	{
		m_pkZodiac_for_Only_party->SetPartyNull();
		m_pkZodiac_for_Only_party = nullptr;
	}
#endif
}

void CParty::ChatPacketToAllMember(uint8_t type, const char* format, ...)
{
	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, format);
	vsnprintf(chatbuf, sizeof(chatbuf), format, args);
	va_end(args);

	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		TMember& rMember = it->second;

		if (rMember.pCharacter)
		{
			if (rMember.pCharacter->GetDesc())
			{
				rMember.pCharacter->ChatPacket(type, "%s", chatbuf);
			}
		}
	}
}

bool CParty::IsAllMembersPoly()
{
	TMemberMap::iterator it;

	auto partyMembers = 0;
	auto polyMembers = 0;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
		{
			if (it->second.pCharacter->IsPolymorphed())
				++polyMembers;

			++partyMembers;
		}
	}

	return (partyMembers == polyMembers);
}

uint32_t CParty::GetLeaderPID() noexcept
{
	return m_dwLeaderPID;
}

#ifdef ENABLE_PARTY_FEATURES
const char* CParty::GetLeaderName()
{
	/*LPCHARACTER leader_ch = CHARACTER_MANAGER::Instance().FindByPID(m_dwLeaderPID);
	return leader_ch->GetName();*/

	for (auto& it : m_memberMap)
	{
		if (it.second.bRole == PARTY_ROLE_LEADER)
			return it.second.strName.c_str();
	}

	return "";
}
#endif

uint32_t CParty::GetMemberCount() noexcept
{
	return m_memberMap.size();
}

void CParty::P2PJoin(uint32_t dwPID)
{
	const TMemberMap::iterator it = m_memberMap.find(dwPID);

	if (it == m_memberMap.end())
	{
		TMember Member;

		Member.pCharacter = nullptr;
		Member.bNear = false;

		if (m_memberMap.empty())
		{
			Member.bRole = PARTY_ROLE_LEADER;
			m_dwLeaderPID = dwPID;
		}
		else
			Member.bRole = PARTY_ROLE_NORMAL;

		if (m_bPCParty)
		{
			const LPCHARACTER& ch = CHARACTER_MANAGER::Instance().FindByPID(dwPID);

			if (ch)
			{
				sys_log(0, "PARTY: Join %s pid %u leader %u", ch->GetName(), dwPID, m_dwLeaderPID);
				Member.strName = ch->GetName();

				if (Member.bRole == PARTY_ROLE_LEADER)
					m_iLeadership = ch->GetLeadershipSkillLevel();
			}
			else
			{
				const CCI* pcci = P2P_MANAGER::Instance().FindByPID(dwPID);

				if (!pcci);
				else if (pcci->bChannel == g_bChannel)
				{
					Member.strName = pcci->szName;
				}
				else
					sys_err("member is not in same channel PID: %u channel %d, this channel %d", dwPID, pcci->bChannel, g_bChannel);
			}
		}

		sys_log(2, "PARTY[%d] MemberCountChange %d -> %d", GetLeaderPID(), GetMemberCount(), GetMemberCount() + 1);

		m_memberMap.insert(TMemberMap::value_type(dwPID, Member));

		if (m_memberMap.size() == 1)
			m_itNextOwner = m_memberMap.begin();

		if (m_bPCParty)
		{
			CPartyManager::Instance().SetPartyMember(dwPID, this);
			SendPartyJoinOneToAll(dwPID);

			const LPCHARACTER& ch = CHARACTER_MANAGER::Instance().FindByPID(dwPID);

			if (ch)
				SendParameter(ch);
		}
	}

	if (m_pkDungeon)
	{
		m_pkDungeon->QuitParty(this);
	}

#ifdef ENABLE_12ZI
	if (m_pkZodiac)
	{
		m_pkZodiac->QuitParty(this);
	}
#endif
}

void CParty::Join(uint32_t dwPID)
{
	P2PJoin(dwPID);

	if (m_bPCParty)
	{
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (GetLeaderCharacter() && GetMemberCount() == 8)
			CAchievementSystem::Instance().OnSocial(GetLeaderCharacter(), achievements::ETaskTypes::TYPE_PARTY);
#endif

		TPacketPartyAdd p{};
		p.dwLeaderPID = GetLeaderPID();
		p.dwPID = dwPID;
		p.bState = PARTY_ROLE_NORMAL; // #0000790: [M2EU] CZ Crash Increase: Reset Important!
		db_clientdesc->DBPacket(HEADER_GD_PARTY_ADD, 0, &p, sizeof(p));
	}
}

void CParty::P2PQuit(uint32_t dwPID)
{
	const TMemberMap::iterator it = m_memberMap.find(dwPID);

	if (it == m_memberMap.end())
		return;

	if (m_bPCParty)
		SendPartyRemoveOneToAll(dwPID);

	if (it == m_itNextOwner)
		IncreaseOwnership();

	if (m_bPCParty)
		RemoveBonusForOne(dwPID);

	LPCHARACTER ch = it->second.pCharacter;
	uint8_t bRole = it->second.bRole;

	m_memberMap.erase(it);

	sys_log(2, "PARTY[%d] MemberCountChange %d -> %d", GetLeaderPID(), GetMemberCount(), GetMemberCount() - 1);

	if (bRole < PARTY_ROLE_MAX_NUM)
	{
		--m_anRoleCount[bRole];
	}
	else
	{
		sys_err("ROLE_COUNT_QUIT_ERROR: INDEX(%d) > MAX(%d)", bRole, PARTY_ROLE_MAX_NUM);
	}

	if (ch)
	{
		ch->SetParty(nullptr);
#ifdef ENABLE_PASSIVE_ATTR
		ComputeRolePoint(ch, GetLeaderCharacter(), bRole, false);
#else
		ComputeRolePoint(ch, bRole, false);
#endif
	}

	if (m_bPCParty)
		CPartyManager::Instance().SetPartyMember(dwPID, nullptr);

	// When the leader leaves, the party must be disbanded.
	if (bRole == PARTY_ROLE_LEADER)
		CPartyManager::Instance().DeleteParty(this);

	// Do not add any code below this!!! If you use DeleteParty above, there is no this.
}

void CParty::Quit(uint32_t dwPID)
{
	// Always PC
	P2PQuit(dwPID);

	if (m_bPCParty && dwPID != GetLeaderPID())
	{
		TPacketPartyRemove p{};
		p.dwPID = dwPID;
		p.dwLeaderPID = GetLeaderPID();
		db_clientdesc->DBPacket(HEADER_GD_PARTY_REMOVE, 0, &p, sizeof(p));
	}
}

void CParty::Link(LPCHARACTER pkChr)
{
	if (!pkChr)
		return;

	TMemberMap::iterator it;

	if (pkChr->IsPC())
		it = m_memberMap.find(pkChr->GetPlayerID());
	else
		it = m_memberMap.find(pkChr->GetVID());

	if (it == m_memberMap.end())
	{
		sys_err("%s is not member of this party", pkChr->GetName());
		return;
	}

	// If it is a player party, create an update event
	if (m_bPCParty && !m_eventUpdate)
	{
		party_update_event_info* info = AllocEventInfo<party_update_event_info>();
		info->pid = m_dwLeaderPID;
		m_eventUpdate = event_create(party_update_event, info, PASSES_PER_SEC(3));
	}

	if (it->second.bRole == PARTY_ROLE_LEADER)
		m_pkChrLeader = pkChr;

	sys_log(2, "PARTY[%d] %s linked to party", GetLeaderPID(), pkChr->GetName());

	it->second.pCharacter = pkChr;
	pkChr->SetParty(this);
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	if (m_bPCParty && m_PositionInfoEvent == nullptr)
		m_PositionInfoEvent = event_create(party_position_event, new party_position_event_info(this), PASSES_PER_SEC(1));
#endif

	if (pkChr->IsPC())
	{
		if (it->second.strName.empty())
		{
			it->second.strName = pkChr->GetName();
		}

		SendPartyJoinOneToAll(pkChr->GetPlayerID());

		SendPartyJoinAllToOne(pkChr);
		SendPartyLinkOneToAll(pkChr);
		SendPartyLinkAllToOne(pkChr);
		SendPartyInfoAllToOne(pkChr);
		SendPartyInfoOneToAll(pkChr);
		SendParameter(pkChr);

		//sys_log(0, "PARTY-DUNGEON connect %p %p", this, GetDungeon());
		if (GetDungeon() && GetDungeon()->GetMapIndex() == pkChr->GetMapIndex())
		{
			pkChr->SetDungeon(GetDungeon());
		}

#ifdef ENABLE_12ZI
		if (GetZodiac() && GetZodiac()->GetMapIndex() == pkChr->GetMapIndex())
		{
			pkChr->SetZodiac(GetZodiac());
		}
#endif

		RequestSetMemberLevel(pkChr->GetPlayerID(), static_cast<uint8_t>(pkChr->GetLevel()));

	}
}

void CParty::RequestSetMemberLevel(uint32_t pid, uint8_t level)
{
	TPacketPartySetMemberLevel p{};
	p.dwLeaderPID = GetLeaderPID();
	p.dwPID = pid;
	p.bLevel = level;
	db_clientdesc->DBPacket(HEADER_GD_PARTY_SET_MEMBER_LEVEL, 0, &p, sizeof(TPacketPartySetMemberLevel));
}

void CParty::P2PSetMemberLevel(uint32_t pid, uint8_t level)
{
	if (!m_bPCParty)
		return;

	TMemberMap::iterator it;

	sys_log(0, "PARTY P2PSetMemberLevel leader %d pid %d level %d", GetLeaderPID(), pid, level);

	it = m_memberMap.find(pid);
	if (it != m_memberMap.end())
	{
		it->second.bLevel = level;
	}
}

namespace
{
	struct FExitDungeon
	{
		void operator() (LPCHARACTER ch)
		{
			if (!ch)
				return;

			ch->ExitToSavedLocation();
		}
	};
}

void CParty::Unlink(LPCHARACTER pkChr)
{
	if (!pkChr)
		return;

	TMemberMap::iterator it;

	if (pkChr->IsPC())
		it = m_memberMap.find(pkChr->GetPlayerID());
	else
		it = m_memberMap.find(pkChr->GetVID());

	if (it == m_memberMap.end())
	{
		sys_err("%s is not member of this party", pkChr->GetName());
		return;
	}

	if (pkChr->IsPC())
	{
		SendPartyUnlinkOneToAll(pkChr);
		//SendPartyUnlinkAllToOne(pkChr); // It is disconnected, so there is no need to send the unlink packet.

		if (it->second.bRole == PARTY_ROLE_LEADER)
		{
			RemoveBonus();

#ifdef ENABLE_12ZI
			if (it->second.pCharacter->GetDungeon() || it->second.pCharacter->GetZodiac())
#else
			if (it->second.pCharacter->GetDungeon())
#endif
			{
				// TODO: If you're in the dungeon, the rest will go out too.
				FExitDungeon f;
				ForEachNearMember(f);
			}
		}
	}

	if (it->second.bRole == PARTY_ROLE_LEADER)
		m_pkChrLeader = nullptr;

	it->second.pCharacter = nullptr;
	pkChr->SetParty(nullptr);
}

void CParty::SendPartyRemoveOneToAll(uint32_t pid)
{
	TMemberMap::iterator it;

	TPacketGCPartyRemove p{};
	p.header = HEADER_GC_PARTY_REMOVE;
	p.pid = pid;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
	}
}

void CParty::SendPartyJoinOneToAll(uint32_t pid)
{
	const TMember& r = m_memberMap[pid];

	TPacketGCPartyAdd p{};

	p.header = HEADER_GC_PARTY_ADD;
	p.pid = pid;
	strlcpy(p.name, r.strName.c_str(), sizeof(p.name));
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	p.mapIdx = GetMemberMapIndex(pid);
	p.channel = GetMemberChannel(pid);
#endif

	for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
	}
}

void CParty::SendPartyJoinAllToOne(const LPCHARACTER& ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TPacketGCPartyAdd p;

	p.header = HEADER_GC_PARTY_ADD;
	p.name[CHARACTER_NAME_MAX_LEN] = '\0';

	for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		p.pid = it->first;
		strlcpy(p.name, it->second.strName.c_str(), sizeof(p.name));
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
		p.mapIdx = GetMemberMapIndex(it->first);
		p.channel = GetMemberChannel(it->first);
#endif
		ch->GetDesc()->Packet(&p, sizeof(p));
	}
}

void CParty::SendPartyUnlinkOneToAll(const LPCHARACTER& ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TMemberMap::iterator it;

	TPacketGCPartyLink p{};
	p.header = HEADER_GC_PARTY_UNLINK;
	p.pid = ch->GetPlayerID();
	p.vid = (uint32_t)ch->GetVID();

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
	}
}

void CParty::SendPartyLinkOneToAll(const LPCHARACTER& ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TMemberMap::iterator it;

	TPacketGCPartyLink p{};
	p.header = HEADER_GC_PARTY_LINK;
	p.vid = ch->GetVID();
	p.pid = ch->GetPlayerID();
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	p.mapIdx = GetMemberMapIndex(ch->GetPlayerID());
	p.channel = GetMemberChannel(ch->GetPlayerID());
#endif

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
	}
}

void CParty::SendPartyLinkAllToOne(const LPCHARACTER& ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TMemberMap::iterator it;

	TPacketGCPartyLink p{};
	p.header = HEADER_GC_PARTY_LINK;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter)
		{
			p.vid = it->second.pCharacter->GetVID();
			p.pid = it->second.pCharacter->GetPlayerID();
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
			p.mapIdx = GetMemberMapIndex(it->second.pCharacter->GetPlayerID());
			p.channel = GetMemberChannel(it->second.pCharacter->GetPlayerID());
#endif
			ch->GetDesc()->Packet(&p, sizeof(p));
		}
	}
}

void CParty::SendPartyInfoOneToAll(uint32_t pid)
{
	TMemberMap::iterator it = m_memberMap.find(pid);

	if (it == m_memberMap.end())
		return;

	if (it->second.pCharacter)
	{
		SendPartyInfoOneToAll(it->second.pCharacter);
		return;
	}

	// Data Building
	TPacketGCPartyUpdate p;
	memset(&p, 0, sizeof(p));
	p.header = HEADER_GC_PARTY_UPDATE;
	p.pid = pid;
	p.percent_hp = 255;
	p.role = it->second.bRole;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if ((it->second.pCharacter) && (it->second.pCharacter->GetDesc()))
		{
			//sys_log(2, "PARTY send info %s[%d] to %s[%d]", ch->GetName(), (uint32_t)ch->GetVID(), it->second.pCharacter->GetName(), (uint32_t)it->second.pCharacter->GetVID());
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
		}
	}
}

void CParty::SendPartyInfoOneToAll(LPCHARACTER ch)
{
	if (!ch || !ch->GetDesc())
		return;

	TMemberMap::iterator it;

	// Data Building
	TPacketGCPartyUpdate p;
	ch->BuildUpdatePartyPacket(p);

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if ((it->second.pCharacter) && (it->second.pCharacter->GetDesc()))
		{
			sys_log(2, "PARTY send info %s[%d] to %s[%d]", ch->GetName(), (uint32_t)ch->GetVID(), it->second.pCharacter->GetName(), (uint32_t)it->second.pCharacter->GetVID());
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
		}
	}
}

void CParty::SendPartyInfoAllToOne(const LPCHARACTER& ch)
{
	if (!ch)
		return;

	TMemberMap::iterator it;

	TPacketGCPartyUpdate p{};

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (!it->second.pCharacter)
		{
			const uint32_t pid = it->first;
			memset(&p, 0, sizeof(p));
			p.header = HEADER_GC_PARTY_UPDATE;
			p.pid = pid;
			p.percent_hp = 255;
			p.role = it->second.bRole;
			ch->GetDesc()->Packet(&p, sizeof(p));
			continue;
		}

		it->second.pCharacter->BuildUpdatePartyPacket(p);
		sys_log(2, "PARTY send info %s[%d] to %s[%d]", it->second.pCharacter->GetName(), (uint32_t)it->second.pCharacter->GetVID(), ch->GetName(), (uint32_t)ch->GetVID());
		ch->GetDesc()->Packet(&p, sizeof(p));
	}
}

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
uint8_t CParty::GetMemberChannel(uint32_t dwPID)
{
	if (!dwPID)
		return 0;

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindByPID(dwPID);
	if (pkChr)
		return g_bChannel;

	CCI* pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
	if (pkCCI)
		return pkCCI->bChannel;

	return 0;
}

long CParty::GetMemberMapIndex(uint32_t dwPID)
{
	if (!dwPID)
		return 0;

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindByPID(dwPID);
	if (pkChr)
		return pkChr->GetMapIndex();

	CCI* pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
	if (pkCCI)
		return pkCCI->lMapIndex;

	return 0;
}

#include "buffer_manager.h"
void CParty::SendPositionInfo() const
{
	for (TMemberMap::const_iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		const LPCHARACTER ch = it->second.pCharacter;
		if (ch == nullptr || ch->GetDesc() == nullptr)
			continue;

		TEMP_BUFFER buf;
		for (TMemberMap::const_iterator it2 = m_memberMap.begin(); it2 != m_memberMap.end(); ++it2)
		{
			if (it == it2)
				continue;

			const LPCHARACTER ch2 = it2->second.pCharacter;
			if (ch2 == nullptr || ch2->GetDesc() == nullptr)
				continue;

			if (ch->GetMapIndex() != ch2->GetMapIndex())
				continue;

			TPartyPosition T{};
			T.dwPID = ch2->GetPlayerID();
			T.lX = ch2->GetX();
			T.lY = ch2->GetY();
			T.fRot = ch2->GetRotation();
			buf.write(&T, sizeof(T));
		}

		if (buf.size())
		{
			TPacketGCPartyPosition packet{};
			packet.bHeader = HEADER_GC_PARTY_POSITION_INFO;
			packet.wSize = sizeof(packet) + buf.size();
			ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
			ch->GetDesc()->Packet(buf.read_peek(), buf.size());
		}
	}
}
#endif

void CParty::SendMessage(LPCHARACTER ch, uint8_t bMsg, uint32_t dwArg1, uint32_t dwArg2)
{
	if (!ch)
		return;

	if (ch->GetParty() != this)
	{
		sys_err("%s is not member of this party %p", ch->GetName(), this);
		return;
	}

	switch (bMsg)
	{
		case PM_ATTACK:
			break;

		case PM_RETURN:
		{
			TMemberMap::iterator it = m_memberMap.begin();

			while (it != m_memberMap.end())
			{
				TMember& rMember = it->second;
				++it;

				LPCHARACTER pkChr;

				if ((pkChr = rMember.pCharacter) && ch != pkChr)
				{
					const uint32_t x = dwArg1 + number(-500, 500);
					const uint32_t y = dwArg2 + number(-500, 500);

					pkChr->SetVictim(nullptr);
					pkChr->SetRotationToXY(x, y);

					if (pkChr->Goto(x, y))
					{
						LPCHARACTER victim = pkChr->GetVictim();
						sys_log(0, "%s %p RETURN victim %p", pkChr->GetName(), get_pointer(pkChr), get_pointer(victim));
						pkChr->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
					}
				}
			}
		}
		break;

	case PM_ATTACKED_BY: // Attacked, ask leader for help
		{
			// when there is no leader
			LPCHARACTER pkChrVictim = ch->GetVictim();

			if (!pkChrVictim)
				return;

			TMemberMap::iterator it = m_memberMap.begin();

			while (it != m_memberMap.end())
			{
				TMember& rMember = it->second;
				++it;

				LPCHARACTER pkChr;

				if ((pkChr = rMember.pCharacter) && ch != pkChr)
				{
					if (pkChr->CanBeginFight())
						pkChr->BeginFight(pkChrVictim);
				}
			}
		}
		break;

	case PM_AGGRO_INCREASE:
		{
			LPCHARACTER victim = CHARACTER_MANAGER::Instance().Find(dwArg2);

			if (!victim)
				return;

			TMemberMap::iterator it = m_memberMap.begin();

			while (it != m_memberMap.end())
			{
				TMember& rMember = it->second;
				++it;

				LPCHARACTER pkChr;

				if ((pkChr = rMember.pCharacter) && ch != pkChr)
				{
					pkChr->UpdateAggrPoint(victim, EDamageType::DAMAGE_TYPE_SPECIAL, dwArg1);
				}
			}
		}
		break;

		default:
			break;
	}
}

LPCHARACTER CParty::GetLeaderCharacter()
{
	return m_memberMap[GetLeaderPID()].pCharacter;
}

bool CParty::SetRole(uint32_t dwPID, uint8_t bRole, bool bSet)
{
	const TMemberMap::iterator it = m_memberMap.find(dwPID);

	if (it == m_memberMap.end())
	{
		return false;
	}

	LPCHARACTER ch = it->second.pCharacter;

	if (bSet)
	{
		if (m_anRoleCount[bRole] >= m_anMaxRole[bRole])
			return false;

		if (it->second.bRole != PARTY_ROLE_NORMAL)
			return false;

		it->second.bRole = bRole;

		if (ch && GetLeader())
#ifdef ENABLE_PASSIVE_ATTR
			ComputeRolePoint(ch, GetLeaderCharacter(), bRole, true);
#else
			ComputeRolePoint(ch, bRole, true);
#endif

		if (bRole < PARTY_ROLE_MAX_NUM)
		{
			++m_anRoleCount[bRole];
		}
		else
		{
			sys_err("ROLE_COUNT_INC_ERROR: INDEX(%d) > MAX(%d)", bRole, PARTY_ROLE_MAX_NUM);
		}
	}
	else
	{
		if (it->second.bRole == PARTY_ROLE_LEADER)
			return false;

		if (it->second.bRole == PARTY_ROLE_NORMAL)
			return false;

		it->second.bRole = PARTY_ROLE_NORMAL;

		if (ch && GetLeader())
#ifdef ENABLE_PASSIVE_ATTR
			ComputeRolePoint(ch, GetLeaderCharacter(), PARTY_ROLE_NORMAL, false);
#else
			ComputeRolePoint(ch, PARTY_ROLE_NORMAL, false);
#endif

		if (bRole < PARTY_ROLE_MAX_NUM)
		{
			--m_anRoleCount[bRole];
		}
		else
		{
			sys_err("ROLE_COUNT_DEC_ERROR: INDEX(%d) > MAX(%d)", bRole, PARTY_ROLE_MAX_NUM);
		}
	}

	SendPartyInfoOneToAll(dwPID);
	return true;
}

uint8_t CParty::GetRole(uint32_t pid)
{
	const TMemberMap::iterator it = m_memberMap.find(pid);

	if (it == m_memberMap.end())
		return PARTY_ROLE_NORMAL;
	else
		return it->second.bRole;
}

bool CParty::IsRole(uint32_t pid, uint8_t bRole)
{
	const TMemberMap::iterator it = m_memberMap.find(pid);

	if (it == m_memberMap.end())
		return false;

	return it->second.bRole == bRole;
}

void CParty::RemoveBonus()
{
	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		LPCHARACTER ch;

		if ((ch = it->second.pCharacter))
		{
#ifdef ENABLE_PASSIVE_ATTR
			ComputeRolePoint(ch, GetLeaderCharacter(), it->second.bRole, false);
#else
			ComputeRolePoint(ch, it->second.bRole, false);
#endif
		}

		it->second.bNear = false;
	}
}

void CParty::RemoveBonusForOne(uint32_t pid)
{
	const TMemberMap::iterator it = m_memberMap.find(pid);

	if (it == m_memberMap.end())
		return;

	LPCHARACTER ch;

	if ((ch = it->second.pCharacter))
#ifdef ENABLE_PASSIVE_ATTR
		ComputeRolePoint(ch, GetLeaderCharacter(), it->second.bRole, false);
#else
		ComputeRolePoint(ch, it->second.bRole, false);
#endif
}

void CParty::HealParty()
{
	// XXX DELETEME until client completes
	{
		return;
	}
	if (!m_bPartyHealReady)
		return;

	TMemberMap::iterator it;
	const LPCHARACTER& l = GetLeaderCharacter();

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (!it->second.pCharacter)
			continue;

		LPCHARACTER ch = it->second.pCharacter;

		if (DISTANCE_APPROX(l->GetX() - ch->GetX(), l->GetY() - ch->GetY()) < PARTY_DEFAULT_RANGE)
		{
			ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
			ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
		}
	}

	m_bPartyHealReady = false;
	m_dwPartyHealTime = get_dword_time();
}

void CParty::SummonToLeader(uint32_t pid)
{
	const int xy[12][2] =
	{
		{ 250, 0 },
		{ 216, 125 },
		{ 125, 216 },
		{ 0, 250 },
		{ -125, 216 },
		{ -216, 125 },
		{ -250, 0 },
		{ -216, -125 },
		{ -125, -216 },
		{ 0, -250 },
		{ 125, -216 },
		{ 216, -125 },
	};

	int n = 0;
	int x[12], y[12];

	SECTREE_MANAGER& s = SECTREE_MANAGER::Instance();
	LPCHARACTER l = GetLeaderCharacter();
	if (!l)
		return;

	if (m_memberMap.find(pid) == m_memberMap.end())
	{
		l->ChatPacket(CHAT_TYPE_INFO, "[LS;685]");
		return;
	}

	LPCHARACTER ch = m_memberMap[pid].pCharacter;

	if (!ch)
	{
		l->ChatPacket(CHAT_TYPE_INFO, "[LS;685]");
		return;
	}

	if (!ch->CanSummon(m_iLeadership))
	{
		l->ChatPacket(CHAT_TYPE_INFO, "[LS;923]");
		return;
	}

	if (ch->IsDead()) {	//fixme477
		l->ChatPacket(CHAT_TYPE_INFO, "I can't teleport the character if he is dead.");
		return;
	}

	for (int i = 0; i < 12; ++i)
	{
		PIXEL_POSITION p;

		if (s.GetMovablePosition(l->GetMapIndex(), l->GetX() + xy[i][0], l->GetY() + xy[i][1], p))
		{
			x[n] = p.x;
			y[n] = p.y;
			n++;
		}
	}

	if (n == 0)
		l->ChatPacket(CHAT_TYPE_INFO, "[LS;924]");
	else
	{
		const int i = number(0, n - 1);
		ch->Show(l->GetMapIndex(), x[i], y[i]);
		ch->Stop();
	}
}

void CParty::IncreaseOwnership() noexcept
{
	if (m_memberMap.empty())
	{
		m_itNextOwner = m_memberMap.begin();
		return;
	}

	if (m_itNextOwner == m_memberMap.end())
		m_itNextOwner = m_memberMap.begin();
	else
	{
		++m_itNextOwner;	//@fixme541

		if (m_itNextOwner == m_memberMap.end())
			m_itNextOwner = m_memberMap.begin();
	}
}

LPCHARACTER CParty::GetNextOwnership(LPCHARACTER ch, long x, long y)
{
	if (m_itNextOwner == m_memberMap.end())
		return ch;

	int size = m_memberMap.size();

	while (size-- > 0)
	{
		LPCHARACTER pkMember = m_itNextOwner->second.pCharacter;

		if (pkMember && DISTANCE_APPROX(pkMember->GetX() - x, pkMember->GetY() - y) < 3000)
		{
			IncreaseOwnership();
			return pkMember;
		}

		IncreaseOwnership();
	}

	return ch;
}

#ifdef ENABLE_PASSIVE_ATTR
void CParty::ComputeRolePoint(LPCHARACTER ch, LPCHARACTER pkLeader, uint8_t bRole, bool bAdd)//you have the problem here right? normally n
{
	if (!ch || !pkLeader)
		return;

	if (!bAdd)
	{
		ch->PointChange(POINT_PARTY_ATTACKER_BONUS, -ch->GetPoint(POINT_PARTY_ATTACKER_BONUS));
		ch->PointChange(POINT_PARTY_TANKER_BONUS, -ch->GetPoint(POINT_PARTY_TANKER_BONUS));
		ch->PointChange(POINT_PARTY_BUFFER_BONUS, -ch->GetPoint(POINT_PARTY_BUFFER_BONUS));
		ch->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, -ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
		ch->PointChange(POINT_PARTY_DEFENDER_BONUS, -ch->GetPoint(POINT_PARTY_DEFENDER_BONUS));
		ch->PointChange(POINT_PARTY_HASTE_BONUS, -ch->GetPoint(POINT_PARTY_HASTE_BONUS));
		ch->ComputeBattlePoints();

#	ifdef ENABLE_CHARISMA
		if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0) {
			pkLeader->PointChange(POINT_PARTY_ATTACKER_BONUS, -pkLeader->GetPoint(POINT_PARTY_ATTACKER_BONUS));
			pkLeader->PointChange(POINT_PARTY_TANKER_BONUS, -pkLeader->GetPoint(POINT_PARTY_TANKER_BONUS));
			pkLeader->PointChange(POINT_PARTY_BUFFER_BONUS, -pkLeader->GetPoint(POINT_PARTY_BUFFER_BONUS));
			pkLeader->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, -pkLeader->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
			pkLeader->PointChange(POINT_PARTY_DEFENDER_BONUS, -pkLeader->GetPoint(POINT_PARTY_DEFENDER_BONUS));
			pkLeader->PointChange(POINT_PARTY_HASTE_BONUS, -pkLeader->GetPoint(POINT_PARTY_HASTE_BONUS));
			pkLeader->ComputeBattlePoints();
		}
#	endif

		return;
	}

	//SKILL_POWER_BY_LEVEL
	float k = (float)ch->GetSkillPowerByLevel(MIN(SKILL_MAX_LEVEL, m_iLeadership)) / 100.0f;
	//sys_log(0,"ComputeRolePoint %fi %d, %d ", k, SKILL_MAX_LEVEL, m_iLeadership );
	//END_SKILL_POWER_BY_LEVEL

#	ifdef ENABLE_CHARISMA
	int lBonus = 0;
	if (pkLeader && aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0) {
		int lBonus_value[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };
		for (int i = 0; i < MINMAX(0, m_iLeadership, SKILL_MAX_LEVEL); i++)
			lBonus = lBonus_value[i];
	}
#	endif

	switch (bRole)
	{
		case PARTY_ROLE_ATTACKER:
			{
				int iBonus = (int)(10 + 60 * k);
				int iBonus2 = 0;

#	ifdef ENABLE_INSPIRATION
				if (ch->GetSkillLevel(SKILL_INSPIRATION) > 0) {
					int pAttk_value[] = { 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 13 };
					for (int i = 0; i < MINMAX(0, ch->GetSkillLevel(SKILL_INSPIRATION), SKILL_MAX_LEVEL); i++)
						iBonus2 = pAttk_value[i];
				}
#	endif

				if (ch->GetPoint(POINT_PARTY_ATTACKER_BONUS) != (iBonus + iBonus2)) {
					ch->PointChange(POINT_PARTY_ATTACKER_BONUS, (iBonus + iBonus2) - ch->GetPoint(POINT_PARTY_ATTACKER_BONUS));
					ch->ComputePoints();
				}

#	ifdef ENABLE_CHARISMA
				if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0)
				{
					int iLBonus = (iBonus / 100.0f) * (lBonus + aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)]);
					if (pkLeader->GetPoint(POINT_PARTY_ATTACKER_BONUS) != iLBonus) {
						pkLeader->PointChange(POINT_PARTY_ATTACKER_BONUS, iLBonus - pkLeader->GetPoint(POINT_PARTY_ATTACKER_BONUS));
						pkLeader->ComputePoints();
					}
				}
#	endif
			}
			break;

		case PARTY_ROLE_TANKER:
			{
				int iBonus = (int)(50 + 1450 * k);
				int iBonus2 = 0;

#	ifdef ENABLE_INSPIRATION
				if (ch->GetSkillLevel(SKILL_INSPIRATION) > 0) {
					int pTank_value[] = { 36, 41, 51, 62, 72, 82, 93, 103, 114, 124, 134, 145, 155, 166, 176, 186, 197, 207, 218, 270, 280, 290, 301, 311, 322, 337, 353, 368, 384, 436, 452, 467, 483, 498, 519, 540, 561, 582, 608, 660 };
					for (int i = 0; i < MINMAX(0, ch->GetSkillLevel(SKILL_INSPIRATION), SKILL_MAX_LEVEL); i++)
						iBonus2 = pTank_value[i];
				}
#	endif

				if (ch->GetPoint(POINT_PARTY_TANKER_BONUS) != (iBonus + iBonus2)) {
					ch->PointChange(POINT_PARTY_TANKER_BONUS, (iBonus + iBonus2) - ch->GetPoint(POINT_PARTY_TANKER_BONUS));
					ch->ComputePoints();
				}

#	ifdef ENABLE_CHARISMA
				if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0)
				{
					int iLBonus = (iBonus / 100.0f) * (lBonus + aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)]);
					if (pkLeader->GetPoint(POINT_PARTY_TANKER_BONUS) != iLBonus) {
						pkLeader->PointChange(POINT_PARTY_TANKER_BONUS, iLBonus - pkLeader->GetPoint(POINT_PARTY_TANKER_BONUS));
						pkLeader->ComputePoints();
					}
				}
#	endif
			}
			break;

		case PARTY_ROLE_BUFFER:
			{
				int iBonus = (int)(5 + 45 * k);
				int iBonus2 = 0;

#	ifdef ENABLE_INSPIRATION
				if (ch->GetSkillLevel(SKILL_INSPIRATION) > 0) {
					int pBlock_value[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6 };
					for (int i = 0; i < MINMAX(0, ch->GetSkillLevel(SKILL_INSPIRATION), SKILL_MAX_LEVEL); i++)
						iBonus2 = pBlock_value[i];
				}
#	endif

				if (ch->GetPoint(POINT_PARTY_BUFFER_BONUS) != (iBonus + iBonus2)) {
					ch->PointChange(POINT_PARTY_BUFFER_BONUS, (iBonus + iBonus2) - ch->GetPoint(POINT_PARTY_BUFFER_BONUS));
					ch->ComputePoints();
				}

#	ifdef ENABLE_CHARISMA
				if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0)
				{
					int iLBonus = (iBonus / 100.0f) * (lBonus + aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)]);
					if (pkLeader->GetPoint(POINT_PARTY_BUFFER_BONUS) != iLBonus) {
						pkLeader->PointChange(POINT_PARTY_BUFFER_BONUS, iLBonus - pkLeader->GetPoint(POINT_PARTY_BUFFER_BONUS));
						pkLeader->ComputePoints();
					}
				}
#	endif
			}
			break;

		case PARTY_ROLE_SKILL_MASTER:
			{
				int iBonus = (int) (25 + 600 * k);
				int iBonus2 = 0;

#	ifdef ENABLE_INSPIRATION
				if (ch->GetSkillLevel(SKILL_INSPIRATION) > 0) {
					int pSkillM_value[] = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7 };
					for (int i = 0; i < MINMAX(0, ch->GetSkillLevel(SKILL_INSPIRATION), SKILL_MAX_LEVEL); i++)
						iBonus2 = (ch->GetPoint(POINT_MAX_SP) / 100.0f) * pSkillM_value[i];
				}
#	endif

				if (ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS) != (iBonus + iBonus2)) {
					ch->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, (iBonus + iBonus2) - ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
					ch->ComputePoints();
				}

#	ifdef ENABLE_CHARISMA
				if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0)
				{
					int iLBonus = (iBonus / 100.0f) * (lBonus + aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)]);
					if (pkLeader->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS) != iLBonus) {
						pkLeader->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, iLBonus - pkLeader->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
						pkLeader->ComputePoints();
					}
				}
#	endif
			}
			break;

		case PARTY_ROLE_HASTE:
			{
				int iBonus = (int) (1+5*k);
				int iBonus2 = 0;

#	ifdef ENABLE_INSPIRATION
				if (ch->GetSkillLevel(SKILL_INSPIRATION) > 0) {
					int pBers_value[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2 };
					for (int i = 0; i < MINMAX(0, ch->GetSkillLevel(SKILL_INSPIRATION), SKILL_MAX_LEVEL); i++)
						iBonus2 = pBers_value[i];
				}
#	endif

				if (ch->GetPoint(POINT_PARTY_HASTE_BONUS) != (iBonus + iBonus2)) {
					ch->PointChange(POINT_PARTY_HASTE_BONUS, (iBonus + iBonus2) - ch->GetPoint(POINT_PARTY_HASTE_BONUS));
					ch->ComputePoints();
				}

#	ifdef ENABLE_CHARISMA
				if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0)
				{
					int iLBonus = (iBonus / 100.0f) * (lBonus + aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)]);
					if (pkLeader->GetPoint(POINT_PARTY_HASTE_BONUS) != iLBonus) {
						pkLeader->PointChange(POINT_PARTY_HASTE_BONUS, iLBonus - pkLeader->GetPoint(POINT_PARTY_HASTE_BONUS));
						pkLeader->ComputePoints();
					}
				}
#	endif
			}
			break;

		case PARTY_ROLE_DEFENDER:
			{
				int iBonus = (int) (5+30*k);
				int iBonus2 = 0;

#	ifdef ENABLE_INSPIRATION
				if (ch->GetSkillLevel(SKILL_INSPIRATION) > 0) {
					int pDef_value[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3 };
					for (int i = 0; i < MINMAX(0, ch->GetSkillLevel(SKILL_INSPIRATION), SKILL_MAX_LEVEL); i++)
						iBonus2 = (ch->GetPoint(POINT_DEF_GRADE) / 100.0f) * pDef_value[i];
				}
#	endif

				if (ch->GetPoint(POINT_PARTY_DEFENDER_BONUS) != (iBonus + iBonus2)) {
					ch->PointChange(POINT_PARTY_DEFENDER_BONUS, (iBonus + iBonus2) - ch->GetPoint(POINT_PARTY_DEFENDER_BONUS));
					ch->ComputePoints();
				}

#	ifdef ENABLE_CHARISMA
				if (aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)] > 0)
				{
					int iLBonus = (iBonus / 100.0f) * (lBonus + aiPartyPassiveSkillCharisma[MINMAX(0, pkLeader->GetSkillLevel(SKILL_CHARISMA), SKILL_MAX_LEVEL)]);
					if (pkLeader->GetPoint(POINT_PARTY_DEFENDER_BONUS) != iLBonus) {
						pkLeader->PointChange(POINT_PARTY_DEFENDER_BONUS, iLBonus - pkLeader->GetPoint(POINT_PARTY_DEFENDER_BONUS));
						pkLeader->ComputePoints();
					}
				}
#	endif
			}
			break;

		default:
			break;
	}
}
#else
void CParty::ComputeRolePoint(LPCHARACTER ch, uint8_t bRole, bool bAdd)
{
	if (!bAdd)
	{
		ch->PointChange(POINT_PARTY_ATTACKER_BONUS, -ch->GetPoint(POINT_PARTY_ATTACKER_BONUS));
		ch->PointChange(POINT_PARTY_TANKER_BONUS, -ch->GetPoint(POINT_PARTY_TANKER_BONUS));
		ch->PointChange(POINT_PARTY_BUFFER_BONUS, -ch->GetPoint(POINT_PARTY_BUFFER_BONUS));
		ch->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, -ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
		ch->PointChange(POINT_PARTY_DEFENDER_BONUS, -ch->GetPoint(POINT_PARTY_DEFENDER_BONUS));
		ch->PointChange(POINT_PARTY_HASTE_BONUS, -ch->GetPoint(POINT_PARTY_HASTE_BONUS));
		ch->ComputeBattlePoints();
		return;
	}

	//SKILL_POWER_BY_LEVEL
	float k = (float)ch->GetSkillPowerByLevel(MIN(SKILL_MAX_LEVEL, m_iLeadership)) / 100.0f;
	//sys_log(0,"ComputeRolePoint %fi %d, %d ", k, SKILL_MAX_LEVEL, m_iLeadership );
	//END_SKILL_POWER_BY_LEVEL

	switch (bRole)
	{
		case PARTY_ROLE_ATTACKER:
			{
				int iBonus = (int)(10 + 60 * k);
				if (ch->GetPoint(POINT_PARTY_ATTACKER_BONUS) != iBonus) {
					ch->PointChange(POINT_PARTY_ATTACKER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_ATTACKER_BONUS));
					ch->ComputePoints();
				}
			}
			break;

		case PARTY_ROLE_TANKER:
			{
				int iBonus = (int)(50 + 1450 * k);
				if (ch->GetPoint(POINT_PARTY_TANKER_BONUS) != iBonus) {
					ch->PointChange(POINT_PARTY_TANKER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_TANKER_BONUS));
					ch->ComputePoints();
				}
			}
			break;

		case PARTY_ROLE_BUFFER:
			{
				int iBonus = (int)(5 + 45 * k);
				if (ch->GetPoint(POINT_PARTY_BUFFER_BONUS) != iBonus) {
					ch->PointChange(POINT_PARTY_BUFFER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_BUFFER_BONUS));
				}
			}
			break;

		case PARTY_ROLE_SKILL_MASTER:
			{
				int iBonus = (int)(25 + 600 * k);
				if (ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS) != iBonus) {
					ch->PointChange(POINT_PARTY_SKILL_MASTER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_SKILL_MASTER_BONUS));
					ch->ComputePoints();
				}
			}
			break;

		case PARTY_ROLE_HASTE:
			{
				int iBonus = (int) (1+5*k);
				if (ch->GetPoint(POINT_PARTY_HASTE_BONUS) != iBonus) {
					ch->PointChange(POINT_PARTY_HASTE_BONUS, iBonus - ch->GetPoint(POINT_PARTY_HASTE_BONUS));
					ch->ComputePoints();
				}
			}
			break;

		case PARTY_ROLE_DEFENDER:
			{
				int iBonus = (int)(5 + 30 * k);
				if (ch->GetPoint(POINT_PARTY_DEFENDER_BONUS) != iBonus) {
					ch->PointChange(POINT_PARTY_DEFENDER_BONUS, iBonus - ch->GetPoint(POINT_PARTY_DEFENDER_BONUS));
					ch->ComputePoints();
				}
			}
			break;
		default:
			break;
	}
}
#endif

void CParty::Update()
{
	const LPCHARACTER& l = GetLeaderCharacter();

	if (!l)
		return;

	TMemberMap::iterator it;

	int iNearMember = 0;
	bool bResendAll = false;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		const LPCHARACTER& ch = it->second.pCharacter;

		it->second.bNear = false;

		if (!ch)
			continue;

		if (l->GetDungeon())
			it->second.bNear = l->GetDungeon() == ch->GetDungeon();
#ifdef ENABLE_12ZI
		else if (l->GetZodiac())
			it->second.bNear = l->GetZodiac() == ch->GetZodiac();
#endif
		else
			it->second.bNear = (DISTANCE_APPROX(l->GetX() - ch->GetX(), l->GetY() - ch->GetY()) < PARTY_DEFAULT_RANGE);

		if (it->second.bNear)
		{
			++iNearMember;
			//sys_log(0,"NEAR %s", ch->GetName());
		}
	}

	if (iNearMember <= 1 && !l->GetDungeon())
	{
		for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
			it->second.bNear = false;

		iNearMember = 0;
	}

#ifdef ENABLE_12ZI
	if (iNearMember <= 1 && !l->GetZodiac())
	{
		for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
			it->second.bNear = false;

		iNearMember = 0;
	}
#endif

	if (iNearMember != m_iCountNearPartyMember)
	{
		m_iCountNearPartyMember = iNearMember;
		bResendAll = true;
	}

	m_iLeadership = l->GetLeadershipSkillLevel();
	const int iNewExpBonus = ComputePartyBonusExpPercent();
	m_iAttBonus = ComputePartyBonusAttackGrade();
	m_iDefBonus = ComputePartyBonusDefenseGrade();

	if (m_iExpBonus != iNewExpBonus)
	{
		bResendAll = true;
		m_iExpBonus = iNewExpBonus;
	}

	bool bLongTimeExpBonusChanged = false;

	// After enough time has passed after forming the party, you will receive an experience bonus.
	if (!m_iLongTimeExpBonus && (get_dword_time() - m_dwPartyStartTime > PARTY_ENOUGH_MINUTE_FOR_EXP_BONUS * 60 * 1000 / 1))
	{
		bLongTimeExpBonusChanged = true;
		m_iLongTimeExpBonus = 5;
		bResendAll = true;
	}

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		LPCHARACTER ch = it->second.pCharacter;

		if (!ch)
			continue;

		if (bLongTimeExpBonusChanged && ch->GetDesc())
			ch->ChatPacket(CHAT_TYPE_INFO, "[LS;926]");

		const bool bNear = it->second.bNear;

#ifdef ENABLE_PASSIVE_ATTR
		ComputeRolePoint(ch, GetLeaderCharacter(), it->second.bRole, bNear);
#else
		ComputeRolePoint(ch, it->second.bRole, bNear);
#endif

		if (bNear)
		{
			if (!bResendAll)
				SendPartyInfoOneToAll(ch);
		}
	}

	// PARTY_ROLE_LIMIT_LEVEL_BUG_FIX
	m_anMaxRole[PARTY_ROLE_ATTACKER] = m_iLeadership >= 10 ? 1 : 0;
	m_anMaxRole[PARTY_ROLE_HASTE] = m_iLeadership >= 20 ? 1 : 0;
	m_anMaxRole[PARTY_ROLE_TANKER] = m_iLeadership >= 20 ? 1 : 0;
	m_anMaxRole[PARTY_ROLE_BUFFER] = m_iLeadership >= 25 ? 1 : 0;
	m_anMaxRole[PARTY_ROLE_SKILL_MASTER] = m_iLeadership >= 35 ? 1 : 0;
	m_anMaxRole[PARTY_ROLE_DEFENDER] = m_iLeadership >= 40 ? 1 : 0;
	m_anMaxRole[PARTY_ROLE_ATTACKER] += m_iLeadership >= 40 ? 1 : 0;
	// END_OF_PARTY_ROLE_LIMIT_LEVEL_BUG_FIX

	// Party Heal Update
	if (!m_bPartyHealReady)
	{
		if (!m_bCanUsePartyHeal && m_iLeadership >= 18)
			m_dwPartyHealTime = get_dword_time();

		m_bCanUsePartyHeal = m_iLeadership >= 18; // Leadership 18 or higher can use heels.

		// Party Heal cooldown is small with Leadership 40 or higher.
		const auto PartyHealCoolTime = (m_iLeadership >= 40) ? PARTY_HEAL_COOLTIME_SHORT * 60 * 1000 : PARTY_HEAL_COOLTIME_LONG * 60 * 1000;

		if (m_bCanUsePartyHeal)
		{
			if (get_dword_time() > m_dwPartyHealTime + PartyHealCoolTime)
			{
				m_bPartyHealReady = true;

				// send heal ready
				if (0) // XXX DELETEME until client completes
				{
					if (GetLeaderCharacter())
						GetLeaderCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "PartyHealReady");
				}
			}
		}
	}

	if (bResendAll)
	{
		for (TMemberMap::iterator it2 = m_memberMap.begin(); it2 != m_memberMap.end(); ++it2)
		{
			if (it2->second.pCharacter)
				SendPartyInfoOneToAll(it2->second.pCharacter);
		}
	}
}

void CParty::UpdateOnlineState(uint32_t dwPID, const char* name)
{
	TMember& r = m_memberMap[dwPID];

	TPacketGCPartyAdd p{};

	p.header = HEADER_GC_PARTY_ADD;
	p.pid = dwPID;
	r.strName = name;
	strlcpy(p.name, name, sizeof(p.name));
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	p.mapIdx = GetMemberMapIndex(dwPID);
	p.channel = GetMemberChannel(dwPID);
#endif

	for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
	}
}
void CParty::UpdateOfflineState(uint32_t dwPID)
{
	//const TMember& r = m_memberMap[dwPID];
#ifdef ENABLE_PASSIVE_ATTR
	RemoveBonusForOne(dwPID);
#endif
	TPacketGCPartyAdd p{};
	p.header = HEADER_GC_PARTY_ADD;
	p.pid = dwPID;
	memset(p.name, 0, CHARACTER_NAME_MAX_LEN + 1);
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	p.mapIdx = 0;
	p.channel = 0;
#endif

	for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		if (it->second.pCharacter && it->second.pCharacter->GetDesc())
			it->second.pCharacter->GetDesc()->Packet(&p, sizeof(p));
	}
}

int CParty::GetFlag(const std::string& name)
{
	const TFlagMap::iterator it = m_map_iFlag.find(name);

	if (it != m_map_iFlag.end())
	{
		//sys_log(0,"PARTY GetFlag %s %d", name.c_str(), it->second);
		return it->second;
	}

	//sys_log(0,"PARTY GetFlag %s 0", name.c_str());
	return 0;
}

void CParty::SetFlag(const std::string& name, int value)
{
	const TFlagMap::iterator it = m_map_iFlag.find(name);

	//sys_log(0,"PARTY SetFlag %s %d", name.c_str(), value);
	if (it == m_map_iFlag.end())
	{
		m_map_iFlag.insert(make_pair(name, value));
	}
	else if (it->second != value)
	{
		it->second = value;
	}
}

void CParty::SetDungeon(LPDUNGEON pDungeon) noexcept
{
	m_pkDungeon = pDungeon;
	m_map_iFlag.clear();
}

LPDUNGEON CParty::GetDungeon() noexcept
{
	return m_pkDungeon;
}

void CParty::SetDungeon_for_Only_party(LPDUNGEON pDungeon) noexcept
{
	m_pkDungeon_for_Only_party = pDungeon;
}

LPDUNGEON CParty::GetDungeon_for_Only_party() noexcept
{
	return m_pkDungeon_for_Only_party;
}

#ifdef ENABLE_12ZI
void CParty::SetZodiac(LPZODIAC pZodiac)
{
	m_pkZodiac = pZodiac;
	m_map_iFlag.clear();
}

LPZODIAC CParty::GetZodiac()
{
	return m_pkZodiac;
}

void CParty::SetZodiac_for_Only_party(LPZODIAC pZodiac)
{
	m_pkZodiac_for_Only_party = pZodiac;
}

LPZODIAC CParty::GetZodiac_for_Only_party()
{
	return m_pkZodiac_for_Only_party;
}
#endif

bool CParty::IsPositionNearLeader(const LPCHARACTER& ch)
{
	if (!ch || !m_pkChrLeader)
		return false;

	if (DISTANCE_APPROX(ch->GetX() - m_pkChrLeader->GetX(), ch->GetY() - m_pkChrLeader->GetY()) >= PARTY_DEFAULT_RANGE)
		return false;

	return true;
}

int CParty::GetExpBonusPercent()
{
	if (GetNearMemberCount() <= 1)
		return 0;

	return m_iExpBonus + m_iLongTimeExpBonus;
}

bool CParty::IsNearLeader(uint32_t pid)
{
	const TMemberMap::iterator it = m_memberMap.find(pid);

	if (it == m_memberMap.end())
		return false;

	return it->second.bNear;
}

uint8_t CParty::CountMemberByVnum(uint32_t dwVnum)
{
	if (m_bPCParty)
		return 0;

	uint8_t bCount = 0;

	//TMemberMap::iterator it;
	//for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	for (auto& it : m_memberMap)
	{
		const auto& tch = it.second.pCharacter;
		if (!tch)
			continue;

		if (tch->IsPC())
			continue;

#ifdef ENABLE_12ZI_AZYRAH
		if (tch->IsDead())
			continue;
#endif

		if (tch->GetRaceNum() == dwVnum)
			++bCount;
	}

	return bCount;
}

void CParty::SendParameter(const LPCHARACTER& ch)
{
	if (!ch)
		return;

	TPacketGCPartyParameter p{};

	p.bHeader = HEADER_GC_PARTY_PARAMETER;
	p.bDistributeMode = static_cast<uint8_t>(m_iExpDistributionMode);

	LPDESC d = ch->GetDesc();
	if (d)
		d->Packet(&p, sizeof(TPacketGCPartyParameter));
}

void CParty::SendParameterToAll()
{
	if (!m_bPCParty)
		return;

	TMemberMap::iterator it;

	for (it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
		if (it->second.pCharacter)
			SendParameter(it->second.pCharacter);
}

void CParty::SetParameter(int iMode)
{
	if (iMode >= PARTY_EXP_DISTRIBUTION_MAX_NUM)
	{
		sys_err("Invalid exp distribution mode %d", iMode);
		return;
	}

	m_iExpDistributionMode = iMode;
	SendParameterToAll();
}

int CParty::GetExpDistributionMode() noexcept
{
	return m_iExpDistributionMode;
}

void CParty::SetExpCentralizeCharacter(uint32_t dwPID)
{
	const TMemberMap::iterator it = m_memberMap.find(dwPID);

	if (it == m_memberMap.end())
		return;

	m_pkChrExpCentralize = it->second.pCharacter;
}

LPCHARACTER CParty::GetExpCentralizeCharacter() noexcept
{
	return m_pkChrExpCentralize;
}

uint8_t CParty::GetMemberMaxLevel() noexcept
{
	uint8_t bMax = 0;

	auto it = m_memberMap.begin();
	while (it != m_memberMap.end())
	{
		if (!it->second.bLevel)
		{
			++it;
			continue;
		}

		if (!bMax)
			bMax = it->second.bLevel;
		else if (it->second.bLevel)
			bMax = static_cast<uint8_t>(MAX(bMax, it->second.bLevel));
		++it;
	}
	return bMax;
}

uint8_t CParty::GetMemberMinLevel() noexcept
{
	uint8_t bMin = PLAYER_MAX_LEVEL_CONST;

	auto it = m_memberMap.begin();
	while (it != m_memberMap.end())
	{
		if (!it->second.bLevel)
		{
			++it;
			continue;
		}

		if (!bMin)
			bMin = it->second.bLevel;
		else if (it->second.bLevel)
			bMin = static_cast<uint8_t>(MIN(bMin, it->second.bLevel));
		++it;
	}
	return bMin;
}

int CParty::ComputePartyBonusExpPercent()
{
	if (GetNearMemberCount() <= 1)
		return 0;

	const LPCHARACTER& leader = GetLeaderCharacter();

	int iBonusPartyExpFromItem = 0;

	// UPGRADE_PARTY_BONUS
	const int iMemberCount = MIN(8, GetNearMemberCount());

	if (leader && (leader->IsEquipUniqueItem(UNIQUE_ITEM_PARTY_BONUS_EXP) || leader->IsEquipUniqueItem(UNIQUE_ITEM_PARTY_BONUS_EXP_MALL)
		|| leader->IsEquipUniqueItem(UNIQUE_ITEM_PARTY_BONUS_EXP_GIFT) || leader->IsEquipUniqueGroup(10010)))
	{
		// It is necessary to confirm the application of Yukdo on the Chinese side.
		iBonusPartyExpFromItem = 30;
	}

#ifdef ENABLE_FLOWER_EVENT
	if (leader && (leader->IsEquipUniqueItem(72059) || leader->IsEquipUniqueItem(72060)))
	{
		// It is necessary to confirm the application of Yukdo on the Chinese side.
		iBonusPartyExpFromItem = 100;
	}
#endif

	return iBonusPartyExpFromItem + CHN_aiPartyBonusExpPercentByMemberCount[iMemberCount];
	// END_OF_UPGRADE_PARTY_BONUS
}

bool CParty::IsPartyInDungeon(int mapIndex)
{
	// Check if party members are in the dungeon with mapIndex in order
	for (TMemberMap::iterator it = m_memberMap.begin(); it != m_memberMap.end(); ++it)
	{
		const LPCHARACTER& ch = it->second.pCharacter;

		if (nullptr == ch)
		{
			continue;
		}

		LPDUNGEON d = ch->GetDungeon();

		if (nullptr == d)
		{
			sys_log(0, "not in dungeon");
			continue;
		}

		if (mapIndex == (d->GetMapIndex()) / 10000)
		{
			return true;
		}

	}
	return false;
}
