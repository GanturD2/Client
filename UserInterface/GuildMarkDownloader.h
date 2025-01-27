#pragma once

#include "../EterLib/NetStream.h"
#include "MarkManager.h"

class CGuildMarkDownloader : public CNetworkStream, public CSingleton<CGuildMarkDownloader>
{
public:
	CGuildMarkDownloader();
	~CGuildMarkDownloader();

	bool Connect(const CNetworkAddress & c_rkNetAddr, uint32_t dwHandle, uint32_t dwRandomKey);
	bool ConnectToRecvSymbol(const CNetworkAddress & c_rkNetAddr, uint32_t dwHandle, uint32_t dwRandomKey,
							 const std::vector<uint32_t> & c_rkVec_dwGuildID);

	void Process();

private:
	enum
	{
		STATE_OFFLINE,
		STATE_LOGIN,
		STATE_COMPLETE
	};

	enum
	{
		TODO_RECV_NONE,
		TODO_RECV_MARK,
		TODO_RECV_SYMBOL
	};

private:
	void OnConnectFailure() override;
	void OnConnectSuccess() override;
	void OnRemoteDisconnect() override;
	void OnDisconnect() override;

	void __Initialize();
	bool __StateProcess();

	uint32_t __GetPacketSize(uint32_t header) const;
	bool __DispatchPacket(uint32_t header);

	void __OfflineState_Set();
	void __CompleteState_Set();

	void __LoginState_Set();
	bool __LoginState_Process();
	bool __LoginState_RecvPhase();
	bool __LoginState_RecvHandshake();
	bool __LoginState_RecvPing();
	bool __LoginState_RecvMarkIndex();
	bool __LoginState_RecvMarkBlock();
	bool __LoginState_RecvSymbolData();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	bool __LoginState_RecvKeyAgreement();
	bool __LoginState_RecvKeyAgreementCompleted();
#endif
	bool __SendMarkIDXList();
	bool __SendMarkCRCList();
	bool __SendSymbolCRCList();

private:
	uint32_t m_dwHandle;
	uint32_t m_dwRandomKey;
	uint32_t m_dwTodo;

	std::vector<uint32_t> m_kVec_dwGuildID;

	uint32_t m_eState;

	uint8_t m_currentRequestingImageIndex;

	CGuildMarkManager * m_pkMarkMgr;

	uint32_t m_dwBlockIndex;
	uint32_t m_dwBlockDataSize;
	uint32_t m_dwBlockDataPos;
};
