#include "StdAfx.h"
#include "NetDatagramSender.h"

BOOL CNetDatagramSender::SetSocket(const char * c_szIP, uint16_t wPortIndex)
{
	return SetSocket(inet_addr(c_szIP), wPortIndex);
}

BOOL CNetDatagramSender::SetSocket(uint32_t dwAddress, uint16_t wPortIndex)
{
	m_isSocket = TRUE;

	m_dwAddress = dwAddress;
	m_wPortIndex = wPortIndex;

	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);

	m_SockAddr = {};
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_addr.s_addr = dwAddress;
	m_SockAddr.sin_port = htons(wPortIndex);

	return TRUE;
}

BOOL CNetDatagramSender::Send(const void * pBuffer, int iSize)
{
	assert(isSocket());

	int iSendingLength = sendto(m_Socket, (const char *) pBuffer, iSize, 0, (PSOCKADDR) &m_SockAddr, sizeof(SOCKADDR_IN));
	if (iSendingLength < 0)
	{
		Tracef("Failed sending packet\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CNetDatagramSender::isSocket()
{
	return m_isSocket;
}

CNetDatagramSender::CNetDatagramSender()
{
	m_isSocket = FALSE;

	m_dwAddress = 0;
	m_wPortIndex = 1000;

	m_Socket = 0;
	m_SockAddr = {};
}

CNetDatagramSender::~CNetDatagramSender() = default;