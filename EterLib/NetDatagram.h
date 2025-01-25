#pragma once

class CNetworkDatagram
{
public:
	CNetworkDatagram();
	virtual ~CNetworkDatagram();

	void Destroy();
	bool Create(uint32_t uPort);

	void Update();

	bool CanRecv();

	int PeekRecvFrom(uint32_t uBufLen, void * pvBuf, SOCKADDR_IN * pkSockAddrIn);
	int RecvFrom(uint32_t uBufLen, void * pvBuf, SOCKADDR_IN * pkSockAddrIn);
	int SendTo(uint32_t uBufLen, const void * c_pvBuf, const SOCKADDR_IN & c_rkSockAddrIn);

private:
	void __Initialize();

private:
	SOCKET m_sock;

	fd_set m_fdsRecv;
	fd_set m_fdsSend;
};