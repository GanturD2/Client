#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "desc.h"

class CLIENT_DESC : public DESC
{
public:
	CLIENT_DESC();
	virtual ~CLIENT_DESC();

	virtual uint8_t GetType() noexcept { return DESC_TYPE_CONNECTOR; }
	virtual void Destroy();
	virtual void SetPhase(int phase);

	bool Connect(int iPhaseWhenSucceed = 0);
	void Setup(LPFDWATCH _fdw, const char* _host, uint16_t _port);

	void SetRetryWhenClosed(bool);

	void DBPacketHeader(uint8_t bHeader, uint32_t dwHandle, uint32_t dwSize);
	void DBPacket(uint8_t bHeader, uint32_t dwHandle, const void* c_pvData, uint32_t dwSize);
	void Packet(const void* c_pvData, int iSize);
	bool IsRetryWhenClosed();

	void Update(uint32_t t);
	void UpdateChannelStatus(uint32_t t, bool fForce);

	// Non-destructive close for reuse
	void Reset();

private:
	void InitializeBuffers();

protected:
	int m_iPhaseWhenSucceed;
	bool m_bRetryWhenClosed;
	time_t m_LastTryToConnectTime;
	time_t m_tLastChannelStatusUpdateTime;

	CInputDB m_inputDB;
	CInputP2P m_inputP2P;
};


extern LPCLIENT_DESC db_clientdesc;
extern LPCLIENT_DESC g_pkAuthMasterDesc;
extern LPCLIENT_DESC g_NetmarbleDBDesc;

#endif
