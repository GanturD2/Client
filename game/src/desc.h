#ifndef __INC_METIN_II_GAME_DESC_H__
#define __INC_METIN_II_GAME_DESC_H__

#include "constants.h"
#include "input.h"
#ifdef _IMPROVED_PACKET_ENCRYPTION_
#include "cipher.h"
#endif

#define MAX_ALLOW_USER 4096
//#define MAX_INPUT_LEN 2048
#define MAX_INPUT_LEN 65536

#define HANDSHAKE_RETRY_LIMIT 32

class CInputProcessor;

enum EDescType
{
	DESC_TYPE_ACCEPTOR,
	DESC_TYPE_CONNECTOR
};

class CLoginKey
{
public:
	CLoginKey(uint32_t dwKey, LPDESC pkDesc) : m_dwKey(dwKey), m_pkDesc(pkDesc)
	{
		m_dwExpireTime = 0;
	}

	void Expire()
	{
		m_dwExpireTime = get_dword_time();
		m_pkDesc = nullptr;
	}

	operator uint32_t() const
	{
		return m_dwKey;
	}

	uint32_t m_dwKey;
	uint32_t m_dwExpireTime;
	LPDESC m_pkDesc;
};

#ifdef ENABLE_SEQUENCE_SYSTEM
struct seq_t
{
	uint8_t hdr;
	uint8_t seq;
};
typedef std::vector<seq_t> seq_vector_t;
#endif

class DESC
{
public:
	EVENTINFO(desc_event_info)
	{
		LPDESC desc;

		desc_event_info()
			: desc(0)
		{
		}
	};

public:
	DESC();
	virtual ~DESC();

	virtual uint8_t GetType() { return DESC_TYPE_ACCEPTOR; }
	virtual void Destroy();
	virtual void SetPhase(int _phase);

	void FlushOutput();

	bool Setup(LPFDWATCH _fdw, socket_t _fd, const struct sockaddr_in& c_rSockAddr, uint32_t _handle, uint32_t _handshake);

	socket_t GetSocket() const { return m_sock; }
	const char* GetHostName() { return m_stHost.c_str(); }
	uint16_t GetPort() { return m_wPort; }

	void SetP2P(const char* h, uint16_t w, uint8_t b) { m_stP2PHost = h; m_wP2PPort = w; m_bP2PChannel = b; }
	const char* GetP2PHost() { return m_stP2PHost.c_str(); }
	uint16_t GetP2PPort() const { return m_wP2PPort; }
	uint8_t GetP2PChannel() const { return m_bP2PChannel; }

	void BufferedPacket(const void* c_pvData, int iSize);
	void Packet(const void* c_pvData, int iSize);
	void LargePacket(const void* c_pvData, int iSize);

	int ProcessInput(); // returns -1 if error
	int ProcessOutput(); // returns -1 if error

	CInputProcessor* GetInputProcessor() { return m_pInputProcessor; }

	uint32_t GetHandle() const { return m_dwHandle; }
	LPBUFFER GetOutputBuffer() { return m_lpOutputBuffer; }

	void BindAccountTable(TAccountTable* pTable);
	TAccountTable& GetAccountTable() { return m_accountTable; }

	void BindCharacter(LPCHARACTER ch);
	LPCHARACTER GetCharacter() { return m_lpCharacter; }

	bool IsPhase(int phase) const { return m_iPhase == phase ? true : false; }

	const struct sockaddr_in& GetAddr() { return m_SockAddr; }

	void UDPGrant(const struct sockaddr_in& c_rSockAddr);
	const struct sockaddr_in& GetUDPAddr() { return m_UDPSockAddr; }

	void Log(const char* format, ...);

	void StartHandshake(uint32_t _dw);
	void SendHandshake(uint32_t dwCurTime, long lNewDelta);
	bool HandshakeProcess(uint32_t dwTime, long lDelta, bool bInfiniteRetry = false);
	bool IsHandshaking();

	uint32_t GetHandshake() const { return m_dwHandshake; }
	uint32_t GetClientTime();

#ifdef _IMPROVED_PACKET_ENCRYPTION_
	void SendKeyAgreement();
	void SendKeyAgreementCompleted();
	bool FinishHandshake(size_t agreed_length, const void* buffer, size_t length);
	bool IsCipherPrepared();
#else
	// Obsolete encryption stuff here
	void SetSecurityKey(const uint32_t* c_pdwKey);
	const uint32_t* GetEncryptionKey() const { return &m_adwEncryptionKey[0]; }
	const uint32_t* GetDecryptionKey() const { return &m_adwDecryptionKey[0]; }
#endif

	uint8_t GetEmpire();

	// for p2p
	void SetRelay(const char* c_pszName);
	bool DelayedDisconnect(int iSec);
	void DisconnectOfSameLogin();

	void SetAdminMode();
	bool IsAdminMode();

	void SetPong(bool b);
	bool IsPong();

#ifdef ENABLE_SEQUENCE_SYSTEM
	uint8_t GetSequence();
	void SetNextSequence();
#endif

	void SendLoginSuccessPacket();

	void SetPanamaKey(uint32_t dwKey) { m_dwPanamaKey = dwKey; }
	uint32_t GetPanamaKey() const { return m_dwPanamaKey; }

	void SetLoginKey(uint32_t dwKey);
	void SetLoginKey(CLoginKey* pkKey);
	uint32_t GetLoginKey();

	void AssembleCRCMagicCube(uint8_t bProcPiece, uint8_t bFilePiece);

	void SetClientVersion(const char* c_pszTimestamp) { m_stClientVersion = c_pszTimestamp; }
	const char* GetClientVersion() { return m_stClientVersion.c_str(); }

	bool isChannelStatusRequested() const { return m_bChannelStatusRequested; }
	void SetChannelStatusRequested(bool bChannelStatusRequested) { m_bChannelStatusRequested = bChannelStatusRequested; }

protected:
	void Initialize();

protected:
	CInputProcessor* m_pInputProcessor;
	CInputClose m_inputClose;
	CInputHandshake m_inputHandshake;
	CInputLogin m_inputLogin;
	CInputMain m_inputMain;
	CInputDead m_inputDead;
	CInputAuth m_inputAuth;


	LPFDWATCH m_lpFdw;
	socket_t m_sock;
	int m_iPhase;
	uint32_t m_dwHandle;

	std::string m_stHost;
	uint16_t m_wPort;
	time_t m_LastTryToConnectTime;

	LPBUFFER m_lpInputBuffer;
	int m_iMinInputBufferLen;

	uint32_t m_dwHandshake;
	uint32_t m_dwHandshakeSentTime;
	int m_iHandshakeRetry;
	uint32_t m_dwClientTime;
	bool m_bHandshaking;

	LPBUFFER m_lpBufferedOutputBuffer;
	LPBUFFER m_lpOutputBuffer;

	LPEVENT m_pkPingEvent;
	LPCHARACTER m_lpCharacter;
	TAccountTable m_accountTable;

	struct sockaddr_in m_SockAddr;
	struct sockaddr_in m_UDPSockAddr;

	FILE* m_pLogFile;
	std::string m_stRelayName;

	std::string m_stP2PHost;
	uint16_t m_wP2PPort;
	uint8_t m_bP2PChannel;

	bool m_bAdminMode;
	bool m_bPong;

#ifdef ENABLE_SEQUENCE_SYSTEM
	int m_iCurrentSequence;
#endif

	CLoginKey* m_pkLoginKey;
	uint32_t m_dwLoginKey;
	uint32_t m_dwPanamaKey;

	uint8_t m_bCRCMagicCubeIdx;
	uint32_t m_dwProcCRC;
	uint32_t m_dwFileCRC;
	bool m_bHackCRCQuery;

	std::string m_stClientVersion;
	std::string m_stMatrixCode;

	std::string m_Login;
	int m_outtime;
	int m_playtime;
	int m_offtime;

	bool m_bDestroyed;
	bool m_bChannelStatusRequested;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
	Cipher cipher_;
#else
	// Obsolete encryption stuff here
	bool m_bEncrypted;
	uint32_t m_adwDecryptionKey[4];
	uint32_t m_adwEncryptionKey[4];
#endif

public:
	LPEVENT m_pkDisconnectEvent;

public:
	void SetLogin(const std::string& login) { m_Login = login; }
	void SetLogin(const char* login) { m_Login = login; }
	const std::string& GetLogin() { return m_Login; }

	void SetOutTime(int outtime) { m_outtime = outtime; }
	void SetOffTime(int offtime) { m_offtime = offtime; }
	void SetPlayTime(int playtime) { m_playtime = playtime; }

	void RawPacket(const void* c_pvData, int iSize);
	void ChatPacket(uint8_t type, const char* format, ...);

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void ChatPacket(uint8_t bType, uint8_t bEmpire, uint8_t bLanguage, const char* format, ...);
	void SetLanguage(uint8_t bLanguage) { m_accountTable.bLanguage = bLanguage; }
	uint8_t GetLanguage() { return m_accountTable.bLanguage; }
#endif

#ifdef ENABLE_SEQUENCE_SYSTEM
public:
	seq_vector_t m_seq_vector;
	void push_seq(uint8_t hdr, uint8_t seq);
#endif
};
#endif
