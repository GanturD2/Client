#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonApplication.h"
#include "Packet.h"
#include "../EterPack/EterPackManager.h"

// HandShake ---------------------------------------------------------------------------
void CPythonNetworkStream::HandShakePhase()
{
	TPacketHeader header;

	if (!CheckPacket(&header))
		return;

	switch (header)
	{
		case HEADER_GC_PHASE:
			if (RecvPhasePacket())
				return;
			break;

		case HEADER_GC_BINDUDP:
			{
				TPacketGCBindUDP BindUDP;

				if (!Recv(sizeof(TPacketGCBindUDP), &BindUDP))
					return;

				return;
			}

		case HEADER_GC_HANDSHAKE:
			{
				if (!Recv(sizeof(TPacketGCHandshake), &m_HandshakeData))
					return;

				Tracenf("HANDSHAKE RECV %u %d", m_HandshakeData.dwTime, m_HandshakeData.lDelta);

				ELTimer_SetServerMSec(m_HandshakeData.dwTime + m_HandshakeData.lDelta);

				//m_dwBaseServerTime = m_HandshakeData.dwTime+ m_HandshakeData.lDelta;
				//m_dwBaseClientTime = ELTimer_GetMSec();

				m_HandshakeData.dwTime = m_HandshakeData.dwTime + m_HandshakeData.lDelta + m_HandshakeData.lDelta;
				m_HandshakeData.lDelta = 0;

				Tracenf("HANDSHAKE SEND %u", m_HandshakeData.dwTime);

				if (!Send(sizeof(TPacketGCHandshake), &m_HandshakeData))
				{
					assert(!"Failed Sending Handshake");
					return;
				}

				CTimer::Instance().SetBaseTime();
				return;
			}
		case HEADER_GC_PING:
			RecvPingPacket();
			return;

		case HEADER_GC_HYBRIDCRYPT_KEYS:
			RecvHybridCryptKeyPacket();
			return;

		case HEADER_GC_HYBRIDCRYPT_SDB:
			RecvHybridCryptSDBPacket();
			return;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
		case HEADER_GC_KEY_AGREEMENT:
			RecvKeyAgreementPacket();
			return;

		case HEADER_GC_KEY_AGREEMENT_COMPLETED:
			RecvKeyAgreementCompletedPacket();
			return;
#endif
	}

	RecvErrorPacket(header);
}

void CPythonNetworkStream::SetHandShakePhase()
{
	if ("HandShake" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Hand Shake Phase ##");
	Tracen("");

	m_strPhase = "HandShake";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::HandShakePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveHandshakePhase);

	SetGameOnline();

	if (__DirectEnterMode_IsSet())
	{
		// None
	}
	else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnHandShake", Py_BuildValue("()"));
}

bool CPythonNetworkStream::RecvHandshakePacket()
{
	TPacketGCHandshake kHandshakeData;
	if (!Recv(sizeof(TPacketGCHandshake), &kHandshakeData))
		return false;

	Tracenf("HANDSHAKE RECV %u %d", kHandshakeData.dwTime, kHandshakeData.lDelta);

	m_kServerTimeSync.m_dwChangeServerTime = kHandshakeData.dwTime + kHandshakeData.lDelta;
	m_kServerTimeSync.m_dwChangeClientTime = ELTimer_GetMSec();

	kHandshakeData.dwTime = kHandshakeData.dwTime + kHandshakeData.lDelta + kHandshakeData.lDelta;
	kHandshakeData.lDelta = 0;

	Tracenf("HANDSHAKE SEND %u", kHandshakeData.dwTime);

	kHandshakeData.header = HEADER_CG_TIME_SYNC;
	if (!Send(sizeof(TPacketGCHandshake), &kHandshakeData))
	{
		assert(!"Failed Sending Handshake");
		return false;
	}

	SendSequence();

	return true;
}

bool CPythonNetworkStream::RecvHandshakeOKPacket()
{
	TPacketGCBlank kBlankPacket;
	if (!Recv(sizeof(TPacketGCBlank), &kBlankPacket))
		return false;

	uint32_t dwDelta = ELTimer_GetMSec() - m_kServerTimeSync.m_dwChangeClientTime;
	ELTimer_SetServerMSec(m_kServerTimeSync.m_dwChangeServerTime + dwDelta);

	Tracenf("HANDSHAKE OK RECV %u %u", m_kServerTimeSync.m_dwChangeServerTime, dwDelta);

	return true;
}

bool CPythonNetworkStream::RecvHybridCryptKeyPacket()
{
	int iFixedHeaderSize = TPacketGCHybridCryptKeys::GetFixedHeaderSize();

	TDynamicSizePacketHeader header;
	if (!Peek(sizeof(header), &header))
		return false;

	TPacketGCHybridCryptKeys kPacket(header.size - iFixedHeaderSize);

	if (!Recv(iFixedHeaderSize, &kPacket))
		return false;

	if (!Recv(kPacket.iKeyStreamLen, kPacket.m_pStream.data()))
		return false;

	CEterPackManager::Instance().RetrieveHybridCryptPackKeys(kPacket.m_pStream.data());
	return true;
}

bool CPythonNetworkStream::RecvHybridCryptSDBPacket()
{
	int iFixedHeaderSize = TPacketGCHybridSDB::GetFixedHeaderSize();

	TDynamicSizePacketHeader header;
	if (!Peek(sizeof(header), &header))
		return false;

	TPacketGCHybridSDB kPacket(header.size - iFixedHeaderSize);

	if (!Recv(iFixedHeaderSize, &kPacket))
		return false;

	if (!Recv(kPacket.iSDBStreamLen, kPacket.m_pStream.data()))
		return false;

	CEterPackManager::Instance().RetrieveHybridCryptPackSDB(kPacket.m_pStream.data());
	return true;
}


#ifdef _IMPROVED_PACKET_ENCRYPTION_
bool CPythonNetworkStream::RecvKeyAgreementPacket()
{
	TPacketKeyAgreement packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	Tracenf("KEY_AGREEMENT RECV %u", packet.wDataLength);

	TPacketKeyAgreement packetToSend;
	size_t dataLength = TPacketKeyAgreement::MAX_DATA_LEN;
	size_t agreedLength = Prepare(packetToSend.data, &dataLength);
	if (agreedLength == 0)
	{
		// 초기화 실패
		Disconnect();
		return false;
	}
	assert(dataLength <= TPacketKeyAgreement::MAX_DATA_LEN);

	if (Activate(packet.wAgreedLength, packet.data, packet.wDataLength))
	{
		// Key agreement 성공, 응답 전송
		packetToSend.bHeader = HEADER_CG_KEY_AGREEMENT;
		packetToSend.wAgreedLength = static_cast<uint16_t>(agreedLength);
		packetToSend.wDataLength = static_cast<uint16_t>(dataLength);

		if (!Send(sizeof(packetToSend), &packetToSend))
		{
			assert(!"Failed Sending KeyAgreement");
			return false;
		}
		Tracenf("KEY_AGREEMENT SEND %u", packetToSend.wDataLength);
	}
	else
	{
		// 키 협상 실패
		Disconnect();
		return false;
	}
	return true;
}

bool CPythonNetworkStream::RecvKeyAgreementCompletedPacket()
{
	TPacketKeyAgreementCompleted packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	Tracenf("KEY_AGREEMENT_COMPLETED RECV");

	ActivateCipher();

	return true;
}
#endif // _IMPROVED_PACKET_ENCRYPTION_
