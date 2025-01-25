#pragma once

#ifndef VC_EXTRALEAN

class CNetDatagramSender
{
public:
	CNetDatagramSender();
	virtual ~CNetDatagramSender();

	BOOL isSocket();

	BOOL SetSocket(const char * c_szIP, uint16_t wPortIndex);
	BOOL SetSocket(uint32_t dwAddress, uint16_t wPortIndex);
	BOOL Send(const void * pBuffer, int iSize);

protected:
	BOOL m_isSocket;

	uint16_t m_dwAddress;
	uint16_t m_wPortIndex;

	SOCKET m_Socket;
	SOCKADDR_IN m_SockAddr;
};

#endif
