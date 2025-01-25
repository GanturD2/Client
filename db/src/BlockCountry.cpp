// vim:ts=4 sw=4
/*********************************************************************
 * date        : 2007.05.31
 * file        : BlockCountry.cpp
 * author      : mhh
 * description :
 */

#include "stdafx.h"

#include "BlockCountry.h"

#include "DBManager.h"

#define DO_ALL_BLOCK_IP(iter)	\
	for ((iter) = m_block_ip.begin(); (iter) != m_block_ip.end(); ++(iter))

#define DO_ALL_BLOCK_EXCEPTION(iter) \
	for ((iter) = m_block_exception.begin(); (iter) != m_block_exception.end(); ++(iter))

CBlockCountry::CBlockCountry()
{



}

CBlockCountry::~CBlockCountry()
{
	BLOCK_IP	*block_ip;
	BLOCK_IP_VECTOR::iterator	iter;

	DO_ALL_BLOCK_IP(iter)
	{
		block_ip = *iter;
		delete block_ip;
	}

	m_block_ip.clear();
}


bool CBlockCountry::Load()
{
	// load blocked ip
	{
		char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT IP_FROM, IP_TO, COUNTRY_NAME FROM iptocountry");
		auto pMsg = CDBManager::Instance().DirectQuery(szQuery, SQL_ACCOUNT);

		if (pMsg->Get()->uiNumRows == 0)
		{
			sys_err(" DirectQuery failed(%s)", szQuery);
			return false;
		}

		MYSQL_ROW row;
		for (int n = 0; (row = mysql_fetch_row(pMsg->Get()->pSQLResult)) != nullptr; ++n)
		{
			BLOCK_IP	*block_ip = new BLOCK_IP;
			block_ip->ip_from	= strtoul(row[0], nullptr, 10);
			block_ip->ip_to		= strtoul(row[1], nullptr, 10);
			strlcpy(block_ip->country, row[2], sizeof(block_ip->country));

			m_block_ip.emplace_back(block_ip);
			sys_log(0, "BLOCKED_IP : %u - %u", block_ip->ip_from, block_ip->ip_to);

		}
	}


	// load block exception account
	{
		char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT login FROM block_exception");
		auto pMsg = CDBManager::Instance().DirectQuery(szQuery, SQL_ACCOUNT);

		if (pMsg->Get()->uiNumRows == 0)
		{
			sys_err(" DirectQuery failed(%s)", szQuery);
			return true;
		}

		MYSQL_ROW row;
		for (int n = 0; (row = mysql_fetch_row(pMsg->Get()->pSQLResult)) != nullptr; ++n)
		{
			const char	*login = row[0];

			m_block_exception.emplace_back(strdup(login));

			sys_log(0, "BLOCK_EXCEPTION = %s", login);

		}
	}

    return true;
}

bool CBlockCountry::IsBlockedCountryIp(const char *user_ip)
{
	BLOCK_IP* block_ip;
	BLOCK_IP_VECTOR::iterator	iter;
	struct in_addr st_addr;

#ifndef __WIN32__
	if (0 == inet_aton(user_ip, &st_addr))
#else
	unsigned long in_address;
	in_address = inet_addr(user_ip);
	st_addr.s_addr = in_address;
	if (INADDR_NONE == in_address)
#endif
		return true;  // 아이피가 괴상하니 일단 블럭처리

	DO_ALL_BLOCK_IP(iter)
	{
		block_ip = *iter;

		if (st_addr.s_addr >= block_ip->ip_from && st_addr.s_addr <= block_ip->ip_to)
			return true;
	}

	return false;
}

void CBlockCountry::SendBlockedCountryIp(CPeer *peer)
{
	sys_log(0, "SendBlockedCountryIp start");
	BLOCK_IP	*block_ip;
	BLOCK_IP_VECTOR::iterator	iter;
	TPacketBlockCountryIp	packet;

	DO_ALL_BLOCK_IP(iter)
	{
		block_ip = *iter;

		packet.ip_from	= block_ip->ip_from;
		packet.ip_to	= block_ip->ip_to;

		peer->EncodeHeader(HEADER_DG_BLOCK_COUNTRY_IP, 0, sizeof(TPacketBlockCountryIp));
		peer->Encode(&packet, sizeof(packet));
	}

	sys_log(0, "[DONE] CBlockCountry::SendBlockedCountryIp() : count = %d",
			m_block_ip.size());
	sys_log(0, "SendBlockedCountryIp end");
} /* end of CBlockCountry::SendBlockedCountryIp() */


void CBlockCountry::SendBlockException(CPeer *peer)
{
	BLOCK_EXCEPTION_VECTOR::iterator	iter;

	DO_ALL_BLOCK_EXCEPTION(iter)
	{
		const char *login = *iter;

		this->SendBlockExceptionOne(peer, login, BLOCK_EXCEPTION_CMD_ADD);
	}
} /* end of CBlockCountry::SendBlockException() */

void CBlockCountry::SendBlockExceptionOne(CPeer *peer, const char *login, uint8_t cmd)
{
	if (nullptr == peer || nullptr == login)
		return;

	if (BLOCK_EXCEPTION_CMD_ADD != cmd && BLOCK_EXCEPTION_CMD_DEL != cmd)
		return;

	TPacketBlockException	packet;

	packet.cmd = cmd;
	strlcpy(packet.login, login, sizeof(packet.login));

	peer->EncodeHeader(HEADER_DG_BLOCK_EXCEPTION, 0, sizeof(TPacketBlockException));
	peer->Encode(&packet, sizeof(packet));
}

void CBlockCountry::AddBlockException(const char *login)
{
	BLOCK_EXCEPTION_VECTOR::iterator	iter;
	DO_ALL_BLOCK_EXCEPTION(iter)
	{
		const char	*saved_login = *iter;

		if (!strcmp(saved_login, login))
			return;
	}

	m_block_exception.emplace_back(strdup(login));
	return;
}

void CBlockCountry::DelBlockException(const char *login)
{
	BLOCK_EXCEPTION_VECTOR::iterator	iter;
	DO_ALL_BLOCK_EXCEPTION(iter)
	{
		const char	*saved_login = *iter;

		if (!strcmp(saved_login, login))
		{
			::free((void*)saved_login);
			m_block_exception.erase(iter);
			return;
		}
	}
	return;
}

