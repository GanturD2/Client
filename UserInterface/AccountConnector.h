#pragma once

#include "../EterLib/NetStream.h"
#include "../EterLib/FuncObject.h"

class CAccountConnector : public CNetworkStream, public CSingleton<CAccountConnector>
{
public:
	enum
	{
		STATE_OFFLINE,
		STATE_HANDSHAKE,
		STATE_AUTH
	};

public:
	CAccountConnector();
	~CAccountConnector();

	void SetHandler(PyObject * poHandler);
	void SetLoginInfo(const char * c_szName, const char * c_szPwd);
	void ClearLoginInfo(void);

	bool Connect(const char * c_szAddr, int iPort, const char * c_szAccountAddr, int iAccountPort);
	void Disconnect();
	void Process();

protected:
	void OnConnectFailure() override;
	void OnConnectSuccess() override;
	void OnRemoteDisconnect() override;
	void OnDisconnect() override;

protected:
	void __Inialize();
	bool __StateProcess();

	void __OfflineState_Set();
	void __HandshakeState_Set();
	void __AuthState_Set();

	bool __HandshakeState_Process();
	bool __AuthState_Process();

	bool __AuthState_RecvEmpty();
	bool __AuthState_RecvPhase();
	bool __AuthState_RecvHandshake();
	bool __AuthState_RecvPing();
	bool __AuthState_SendPong();
	bool __AuthState_RecvAuthSuccess();
	bool __AuthState_RecvAuthFailure();
	bool __AuthState_RecvPanamaPack();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	bool __AuthState_RecvKeyAgreement();
	bool __AuthState_RecvKeyAgreementCompleted();
#endif
	bool __AuthState_RecvHybridCryptKeys(int iTotalSize);
	bool __AuthState_RecvHybridCryptSDB(int iTotalSize);

	bool __AnalyzePacket(uint32_t uHeader, uint32_t uPacketSize, bool (CAccountConnector::*pfnDispatchPacket)());
	// TODO:  지금 현재는 임시다.  header뒤에 size 4byte가 무조건 온다는 가정임.
	// 제대로 하려면  Packet System Refactoring해야 한다.
	bool __AnalyzeVarSizePacket(uint32_t uHeader, bool (CAccountConnector::*pfnDispatchPacket)(int));

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	void __BuildClientKey();
#endif

protected:
	uint32_t m_eState;
	std::string m_strID;
	std::string m_strPassword;

	std::string m_strAddr;
	int m_iPort;
	BOOL m_isWaitKey;

	PyObject * m_poHandler;

	// CHINA_CRYPT_KEY
	void __BuildClientKey_20050304Myevan() const;
	// END_OF_CHINA_CRYPT_KEY
};
