#include "stdafx.h"
#include "Peer.h"
#include "ItemIDRangeManager.h"

CPeer::CPeer()
{
	m_state = 0;
	m_bChannel = 0;
#ifdef ENABLE_MOVE_CHANNEL
	m_lAddr = 0;
#endif
	m_dwHandle = 0;
	m_dwUserCount = 0;
	m_wListenPort = 0;
	m_wP2PPort = 0;

	memset(m_alMaps, 0, sizeof(m_alMaps));

	m_itemRange.dwMin = m_itemRange.dwMax = m_itemRange.dwUsableItemIDMin = 0;
	m_itemSpareRange.dwMin = m_itemSpareRange.dwMax = m_itemSpareRange.dwUsableItemIDMin = 0;
}

CPeer::~CPeer()
{
	Close();
}

void CPeer::OnAccept()
{
	m_state = STATE_PLAYING;

	static uint32_t current_handle = 0;
	m_dwHandle = ++current_handle;

	sys_log(0, "Connection accepted. (host: %s handle: %u fd: %d)", m_host, m_dwHandle, m_fd);
}

void CPeer::OnConnect()
{
	sys_log(0, "Connection established. (host: %s handle: %u fd: %d)", m_host, m_dwHandle, m_fd);
	m_state = STATE_PLAYING;
}

void CPeer::OnClose()
{
	m_state = STATE_CLOSE;

	sys_log(0, "Connection closed. (host: %s)", m_host);
	sys_log(0, "ItemIDRange: returned. %u ~ %u", m_itemRange.dwMin, m_itemRange.dwMax);

	CItemIDRangeManager::Instance().UpdateRange(m_itemRange.dwMin, m_itemRange.dwMax);

	m_itemRange.dwMin = 0;
	m_itemRange.dwMax = 0;
	m_itemRange.dwUsableItemIDMin = 0;
}

uint32_t CPeer::GetHandle()
{
	return m_dwHandle;
}

uint32_t CPeer::GetUserCount()
{
	return m_dwUserCount;
}

void CPeer::SetUserCount(uint32_t dwCount)
{
	m_dwUserCount = dwCount;
}

bool CPeer::PeekPacket(int & iBytesProceed, uint8_t & header, uint32_t & dwHandle, uint32_t & dwLength, const char ** data)
{
	if (GetRecvLength() < iBytesProceed + 9)
		return false;

	const char * buf = (const char *) GetRecvBuffer();
	buf += iBytesProceed;

	header	= *(buf++);

	dwHandle	= *((uint32_t *) buf);
	buf		+= sizeof(uint32_t);

	dwLength	= *((uint32_t *) buf);
	buf		+= sizeof(uint32_t);

	//sys_log(0, "%d header %d handle %u length %u", GetRecvLength(), header, dwHandle, dwLength);
	if (iBytesProceed + dwLength + 9 > (uint32_t) GetRecvLength())
	{
		sys_log(0, "PeekPacket: not enough buffer size: len %u, recv %d",
			9+dwLength, GetRecvLength()-iBytesProceed);
		return false;
	}

	*data = buf;
	iBytesProceed += dwLength + 9;
	return true;
}

void CPeer::EncodeHeader(uint8_t header, uint32_t dwHandle, uint32_t dwSize)
{
	HEADER h;

	sys_log(1, "EncodeHeader %u handle %u size %u", header, dwHandle, dwSize);

	h.bHeader = header;
	h.dwHandle = dwHandle;
	h.dwSize = dwSize;

	Encode(&h, sizeof(HEADER));
}

void CPeer::EncodeReturn(uint8_t header, uint32_t dwHandle)
{
	EncodeHeader(header, dwHandle, 0);
}

int CPeer::Send()
{
	if (m_state == STATE_CLOSE)
		return -1;

	return (CPeerBase::Send());
}

void CPeer::SetP2PPort(uint16_t wPort)
{
	m_wP2PPort = wPort;
}

void CPeer::SetMaps(long * pl)
{
	thecore_memcpy(m_alMaps, pl, sizeof(m_alMaps));
}

#ifdef ENABLE_MOVE_CHANNEL
#include <algorithm>
bool CPeer::CheckMapIndex(const long lMapIndex) const
{
	return std::find(std::begin(m_alMaps), std::end(m_alMaps), lMapIndex) != std::end(m_alMaps);
}
#endif

void CPeer::SendSpareItemIDRange()
{
	if (m_itemSpareRange.dwMin == 0 || m_itemSpareRange.dwMax == 0 || m_itemSpareRange.dwUsableItemIDMin == 0)
	{
		EncodeHeader(HEADER_DG_ACK_SPARE_ITEM_ID_RANGE, 0, sizeof(TItemIDRangeTable));
		Encode(&m_itemSpareRange, sizeof(TItemIDRangeTable));
	}
	else
	{
		SetItemIDRange(m_itemSpareRange);

		if (SetSpareItemIDRange(CItemIDRangeManager::Instance().GetRange()) == false)
		{
			sys_log(0, "ItemIDRange: spare range set error");
			m_itemSpareRange.dwMin = m_itemSpareRange.dwMax = m_itemSpareRange.dwUsableItemIDMin = 0;
		}

		EncodeHeader(HEADER_DG_ACK_SPARE_ITEM_ID_RANGE, 0, sizeof(TItemIDRangeTable));
		Encode(&m_itemSpareRange, sizeof(TItemIDRangeTable));
	}
}

bool CPeer::SetItemIDRange(TItemIDRangeTable itemRange)
{
	if (itemRange.dwMin == 0 || itemRange.dwMax == 0 || itemRange.dwUsableItemIDMin == 0) return false;

	m_itemRange = itemRange;
	sys_log(0, "ItemIDRange: SET %s %u ~ %u start: %u", GetPublicIP(), m_itemRange.dwMin, m_itemRange.dwMax, m_itemRange.dwUsableItemIDMin);

	return true;
}

bool CPeer::SetSpareItemIDRange(TItemIDRangeTable itemRange)
{
	if (itemRange.dwMin == 0 || itemRange.dwMax == 0 || itemRange.dwUsableItemIDMin == 0) return false;

	m_itemSpareRange = itemRange;
	sys_log(0, "ItemIDRange: SPARE SET %s %u ~ %u start: %u", GetPublicIP(), m_itemSpareRange.dwMin, m_itemSpareRange.dwMax,
		m_itemSpareRange.dwUsableItemIDMin);

	return true;
}

bool CPeer::CheckItemIDRangeCollision(TItemIDRangeTable itemRange)
{
	if (m_itemRange.dwMin < itemRange.dwMax && m_itemRange.dwMax > itemRange.dwMin)
	{
		sys_err("ItemIDRange: Collision!! this %u ~ %u check %u ~ %u",
			m_itemRange.dwMin, m_itemRange.dwMax, itemRange.dwMin, itemRange.dwMax);
		return false;
	}

	if (m_itemSpareRange.dwMin < itemRange.dwMax && m_itemSpareRange.dwMax > itemRange.dwMin)
	{
		sys_err("ItemIDRange: Collision with spare range this %u ~ %u check %u ~ %u",
			m_itemSpareRange.dwMin, m_itemSpareRange.dwMax, itemRange.dwMin, itemRange.dwMax);
		return false;
	}

	return true;
}


