#include "StdAfx.h"
#include "MappedFile.h"
#include "Debug.h"

CMappedFile::CMappedFile()
	: m_pbBufLinkData(nullptr),
	  m_dwBufLinkSize(0),
	  m_pbAppendResultDataBlock(nullptr),
	  m_dwAppendResultDataSize(0),
	  m_seekPosition(0),
	  m_hFM(nullptr),
	  m_dataOffset(0),
	  m_mapSize(0),
	  m_lpMapData(nullptr),
	  m_pLZObj(nullptr)
{
}

CMappedFile::~CMappedFile()
{
	Destroy();
}

BOOL CMappedFile::Create(const char * filename)
{
	Destroy();
	return CFileBase::Create(filename, FILEMODE_READ);
}

BOOL CMappedFile::Create(const char * filename, const void ** dest, int offset, int size)
{
	if (!Create(filename))
		return 0;

	int ret = Map(dest, offset, size);
	return (ret) > 0;
}

LPCVOID CMappedFile::Get() const
{
	return m_lpData;
}

void CMappedFile::Link(uint32_t dwBufSize, const void * c_pvBufData)
{
	m_dwBufLinkSize = dwBufSize;
	m_pbBufLinkData = (uint8_t *) c_pvBufData;
}

void CMappedFile::BindLZObject(CLZObject * pLZObj)
{
	assert(m_pLZObj == nullptr);
	m_pLZObj = pLZObj;

	Link(m_pLZObj->GetSize(), m_pLZObj->GetBuffer());
}

void CMappedFile::BindLZObjectWithBufferedSize(CLZObject * pLZObj)
{
	assert(m_pLZObj == nullptr);
	m_pLZObj = pLZObj;

	Link(m_pLZObj->GetBufferSize(), m_pLZObj->GetBuffer());
}

uint8_t * CMappedFile::AppendDataBlock(const void * pBlock, uint32_t dwBlockSize)
{
	delete[] m_pbAppendResultDataBlock;

	//realloc
	m_dwAppendResultDataSize = m_dwBufLinkSize + dwBlockSize;
	m_pbAppendResultDataBlock = new uint8_t[m_dwAppendResultDataSize];

	memcpy(m_pbAppendResultDataBlock, m_pbBufLinkData, m_dwBufLinkSize);
	memcpy(m_pbAppendResultDataBlock + m_dwBufLinkSize, pBlock, dwBlockSize);

	//redirect
	Link(m_dwAppendResultDataSize, m_pbAppendResultDataBlock);

	return m_pbAppendResultDataBlock;
}

void CMappedFile::Destroy()
{
	if (m_pLZObj) // 압축된 데이터가 이 포인터로 연결 된다
	{
		delete m_pLZObj;
		m_pLZObj = nullptr;
	}

	if (nullptr != m_lpMapData)
	{
		Unmap(m_lpMapData);
		m_lpMapData = nullptr;
	}

	if (nullptr != m_hFM)
	{
		CloseHandle(m_hFM);
		m_hFM = nullptr;
	}

	if (m_pbAppendResultDataBlock)
	{
		delete[] m_pbAppendResultDataBlock;
		m_pbAppendResultDataBlock = nullptr;
	}

	m_dwAppendResultDataSize = 0;

	m_pbBufLinkData = nullptr;
	m_dwBufLinkSize = 0;

	m_seekPosition = 0;
	m_dataOffset = 0;
	m_mapSize = 0;

	CFileBase::Destroy();
}

int CMappedFile::Seek(uint32_t offset, int iSeekType)
{
	switch (iSeekType)
	{
	case SEEK_TYPE_BEGIN:
		if (offset > m_dwSize)
			offset = m_dwSize;

		m_seekPosition = offset;
		break;

	case SEEK_TYPE_CURRENT:
		m_seekPosition = std::min(m_seekPosition + offset, Size());
		break;

	case SEEK_TYPE_END:
		m_seekPosition = std::max<size_t>(0, Size() - offset);
		break;
	}

	return m_seekPosition;
}

int CMappedFile::Map(const void ** dest, int offset, int size)
{
	m_dataOffset = offset;

	if (size == 0)
		m_mapSize = m_dwSize;
	else
		m_mapSize = size;

	if (m_dataOffset + m_mapSize > m_dwSize)
		return 0;

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	uint32_t dwSysGran = SysInfo.dwAllocationGranularity;
	uint32_t dwFileMapStart = (m_dataOffset / dwSysGran) * dwSysGran;
	uint32_t dwMapViewSize = (m_dataOffset % dwSysGran) + m_mapSize;
	int32_t iViewDelta = m_dataOffset - dwFileMapStart;


	m_hFM = CreateFileMapping(m_hFile, // handle
							  nullptr, // security
							  PAGE_READONLY, // flProtect
							  0, // high
							  m_dataOffset + m_mapSize, // low
							  nullptr); // name

	if (!m_hFM)
	{
		OutputDebugString("CMappedFile::Map !m_hFM\n");
		return 0;
	}

	m_lpMapData = MapViewOfFile(m_hFM, FILE_MAP_READ, 0, dwFileMapStart, dwMapViewSize);

	if (!m_lpMapData) // Success
	{
		TraceError("CMappedFile::Map !m_lpMapData %lu", GetLastError());
		return 0;
	}

	m_lpData = static_cast<char *>(m_lpMapData) + iViewDelta;
	*dest = static_cast<char *>(m_lpData);
	m_seekPosition = 0;

	Link(m_mapSize, m_lpData);

	return (m_mapSize);
}

uint8_t * CMappedFile::GetCurrentSeekPoint() const
{
	return m_pbBufLinkData + m_seekPosition;
}


uint32_t CMappedFile::Size() const
{
	return m_dwBufLinkSize;
}

uint32_t CMappedFile::GetPosition() const
{
	return m_dataOffset;
}

BOOL CMappedFile::Read(void * dest, int bytes)
{
	if (m_seekPosition + bytes > Size())
		return FALSE;

	memcpy(dest, GetCurrentSeekPoint(), bytes);
	m_seekPosition += bytes;
	return TRUE;
}

uint32_t CMappedFile::GetSeekPosition() const
{
	return m_seekPosition;
}

void CMappedFile::Unmap(LPCVOID data)
{
	if (!UnmapViewOfFile(data))
		TraceError("CMappedFile::Unmap - Error");
	m_lpData = nullptr;
}
