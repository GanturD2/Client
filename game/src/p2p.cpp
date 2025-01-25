#include "stdafx.h"
#include "../../common/stl.h"

#include "constants.h"
#include "config.h"
#include "p2p.h"
#include "desc_p2p.h"
#include "char.h"
#include "char_manager.h"
#include "sectree_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "messenger_manager.h"
#include "marriage.h"
#include "utils.h"
#include "locale_service.h"
#include <sstream>

P2P_MANAGER::P2P_MANAGER()
{
	m_pkInputProcessor = nullptr;
	m_iHandleCount = 0;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	m_map_shopCCI.clear();
#endif
	memset(m_aiEmpireUserCount, 0, sizeof(m_aiEmpireUserCount));
}

P2P_MANAGER::~P2P_MANAGER()
{
}

void P2P_MANAGER::Boot(LPDESC d)
{
	CHARACTER_MANAGER::NAME_MAP & map = CHARACTER_MANAGER::Instance().GetPCMap();
	CHARACTER_MANAGER::NAME_MAP::iterator it = map.begin();

	TPacketGGLogin p{};

	while (it != map.end())
	{
		LPCHARACTER ch = it->second;
		++it;	//@fixme515 (it++ to ++it)

		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, ch->GetName(), sizeof(p.szName));
		p.dwPID = ch->GetPlayerID();
		p.bEmpire = ch->GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::Instance().GetMapIndex(ch->GetX(), ch->GetY());
		p.bChannel = g_bChannel;

		d->Packet(&p, sizeof(p));
	}
}

void P2P_MANAGER::FlushOutput()
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.begin();

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;
		pkDesc->FlushOutput();
	}
}

void P2P_MANAGER::RegisterAcceptor(LPDESC d)
{
	sys_log(0, "P2P Acceptor opened (host %s)", d->GetHostName());
	m_set_pkPeers.insert(d);
	Boot(d);
}

void P2P_MANAGER::UnregisterAcceptor(LPDESC d)
{
	sys_log(0, "P2P Acceptor closed (host %s)", d->GetHostName());
	EraseUserByDesc(d);
	m_set_pkPeers.erase(d);
}

void P2P_MANAGER::RegisterConnector(LPDESC d)
{
	sys_log(0, "P2P Connector opened (host %s)", d->GetHostName());
	m_set_pkPeers.insert(d);
	Boot(d);

	TPacketGGSetup p{};
	p.bHeader = HEADER_GG_SETUP;
	p.wPort = p2p_port;
	p.bChannel = g_bChannel;
	d->Packet(&p, sizeof(p));
}

void P2P_MANAGER::UnregisterConnector(LPDESC d)
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.find(d);

	if (it != m_set_pkPeers.end())
	{
		sys_log(0, "P2P Connector closed (host %s)", d->GetHostName());
		EraseUserByDesc(d);
		m_set_pkPeers.erase(it);
	}
}

void P2P_MANAGER::EraseUserByDesc(LPDESC d)
{
	TCCIMap::iterator it = m_map_pkCCI.begin();

	while (it != m_map_pkCCI.end())
	{
		CCI * pkCCI = it->second;
		++it;	//@fixme515 (it++ to ++it)

		if (pkCCI->pkDesc == d)
			Logout(pkCCI);
	}
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	for (const auto it : m_map_shopCCI)
	{
		if (it.second->pkDesc == d)
			EndShopOffline(it.second);
	}
#endif
}

void P2P_MANAGER::Send(const void * c_pvData, int iSize, LPDESC except)
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.begin();

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;

		if (except == pkDesc)
			continue;

		pkDesc->Packet(c_pvData, iSize);
	}
}

void P2P_MANAGER::Login(LPDESC d, const TPacketGGLogin * p)
{
	CCI* pkCCI = Find(p->szName);

	bool UpdateP2P = false;

	if (nullptr == pkCCI)
	{
		UpdateP2P = true;
		pkCCI = M2_NEW CCI;

		strlcpy(pkCCI->szName, p->szName, sizeof(pkCCI->szName));

		pkCCI->dwPID = p->dwPID;
		pkCCI->bEmpire = p->bEmpire;

		if (p->bChannel == g_bChannel)
		{
			if (pkCCI->bEmpire < EMPIRE_MAX_NUM)
			{
				++m_aiEmpireUserCount[pkCCI->bEmpire];
			}
			else
			{
				sys_err("LOGIN_EMPIRE_ERROR: %d >= MAX(%d)", pkCCI->bEmpire, EMPIRE_MAX_NUM);
			}
		}

		m_map_pkCCI.insert(std::make_pair(pkCCI->szName, pkCCI));
		m_map_dwPID_pkCCI.insert(std::make_pair(pkCCI->dwPID, pkCCI));
	}

	pkCCI->lMapIndex = p->lMapIndex;
	pkCCI->pkDesc = d;
	pkCCI->bChannel = p->bChannel;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pkCCI->bLanguage = p->bLanguage;
#endif
	sys_log(0, "P2P: Login %s", pkCCI->szName);

	CGuildManager::Instance().P2PLoginMember(pkCCI->dwPID);

	CPartyManager::Instance().P2PLogin(pkCCI->dwPID, pkCCI->szName);

	if (UpdateP2P) {
		std::string name(pkCCI->szName);
	    MessengerManager::Instance().P2PLogin(name);
	}
}

void P2P_MANAGER::Logout(CCI * pkCCI)
{
	if (pkCCI->bChannel == g_bChannel)
	{
		if (pkCCI->bEmpire < EMPIRE_MAX_NUM)
		{
			--m_aiEmpireUserCount[pkCCI->bEmpire];
			if (m_aiEmpireUserCount[pkCCI->bEmpire] < 0)
			{
				sys_err("m_aiEmpireUserCount[%d] < 0", pkCCI->bEmpire);
			}
		}
		else
		{
			sys_err("LOGOUT_EMPIRE_ERROR: %d >= MAX(%d)", pkCCI->bEmpire, EMPIRE_MAX_NUM);
		}
	}

	std::string name(pkCCI->szName);

	CGuildManager::Instance().P2PLogoutMember(pkCCI->dwPID);
	CPartyManager::Instance().P2PLogout(pkCCI->dwPID);
	MessengerManager::Instance().P2PLogout(name);
	marriage::CManager::Instance().Logout(pkCCI->dwPID);

	m_map_pkCCI.erase(name);
	m_map_dwPID_pkCCI.erase(pkCCI->dwPID);
	M2_DELETE(pkCCI);
}

void P2P_MANAGER::Logout(const char * c_pszName)
{
	CCI * pkCCI = Find(c_pszName);

	if (!pkCCI)
		return;

	Logout(pkCCI);
	sys_log(0, "P2P: Logout %s", c_pszName);
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
void P2P_MANAGER::StartShopOffline(LPDESC d, const TPacketGGShopStartOffline* p)
{
	spShopCCI shopCCI = FindShop(p->dwPID);

	if (!shopCCI) //Did not exist previously (this should be what always happens)
	{
		shopCCI = std::shared_ptr<ShopCCI>(new ShopCCI());
		shopCCI->dwPID = p->dwPID;
		shopCCI->bEmpire = p->bEmpire;

		m_map_shopCCI.insert(std::make_pair(shopCCI->dwPID, shopCCI));
	}

	shopCCI->pkDesc = d;
	shopCCI->bChannel = p->bChannel;
	shopCCI->lMapIndex = p->lMapIndex;
	sys_log(0, "P2P: Shop enter offline #%u", p->dwPID);
}

void P2P_MANAGER::EndShopOffline(uint32_t pid)
{
	spShopCCI shopCCI = FindShop(pid);

	if (!shopCCI)
		return;

	EndShopOffline(shopCCI);
	sys_log(0, "P2P: End shop offline %u", pid);
}

void P2P_MANAGER::EndShopOffline(spShopCCI shopCCI)
{
	m_map_shopCCI.erase(shopCCI->dwPID);
}
#endif

CCI * P2P_MANAGER::FindByPID(uint32_t pid)
{
	TPIDCCIMap::iterator it = m_map_dwPID_pkCCI.find(pid);
	if (it == m_map_dwPID_pkCCI.end())
		return nullptr;
	return it->second;
}

CCI * P2P_MANAGER::Find(const char * c_pszName)
{
	TCCIMap::const_iterator it;

	it = m_map_pkCCI.find(c_pszName);

	if (it == m_map_pkCCI.end())
		return nullptr;

	return it->second;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
spShopCCI P2P_MANAGER::FindShop(uint32_t pid)
{
	for (const auto it : m_map_shopCCI)
	{
		if (it.first == pid)
			return it.second;
	}

	return nullptr;
}
#endif

CCI * P2P_MANAGER::PickRandomCCI()
{
	if (m_map_pkCCI.size() < 1)
		return nullptr;

	auto it = m_map_pkCCI.begin();
	std::advance(it, uniform_random(0, m_map_pkCCI.size() - 1));

	return it->second;
}

int P2P_MANAGER::GetCount()
{
	//return m_map_pkCCI.size();
	return m_aiEmpireUserCount[1] + m_aiEmpireUserCount[2] + m_aiEmpireUserCount[3];
}

int P2P_MANAGER::GetEmpireUserCount(int idx)
{
	assert(idx < EMPIRE_MAX_NUM);
	return m_aiEmpireUserCount[idx];
}


int P2P_MANAGER::GetDescCount()
{
	return m_set_pkPeers.size();
}

void P2P_MANAGER::GetP2PHostNames(std::string& hostNames)
{
	std::unordered_set<LPDESC>::iterator it = m_set_pkPeers.begin();

	std::ostringstream oss(std::ostringstream::out);

	while (it != m_set_pkPeers.end())
	{
		LPDESC pkDesc = *it++;

		oss << pkDesc->GetP2PHost() << " " << pkDesc->GetP2PPort() << "\n";

	}
	hostNames += oss.str();
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
int P2P_MANAGER::GetForeignShopCount()
{
	return m_map_shopCCI.size();
}
#endif
