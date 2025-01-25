#ifndef __INC_MAPPEDFILE_H__
#define __INC_MAPPEDFILE_H__

#include "lzo.h"
#include "FileBase.h"

class CMappedFile : public CFileBase
{
public:
	enum ESeekType
	{
		SEEK_TYPE_BEGIN,
		SEEK_TYPE_CURRENT,
		SEEK_TYPE_END
	};

public:
	CMappedFile();
	virtual ~CMappedFile();

	void Link(uint32_t dwBufSize, const void * c_pvBufData);

	BOOL Create(const char * filename);
	BOOL Create(const char * filename, const void ** dest, int offset, int size);
	LPCVOID Get() const;
	void Destroy();
	int Seek(uint32_t offset, int iSeekType = SEEK_TYPE_BEGIN);
	int Map(const void ** dest, int offset = 0, int size = 0);
	uint32_t Size() const;
	uint32_t GetPosition() const;
	BOOL Read(void * dest, int bytes);
	uint32_t GetSeekPosition() const;
	void BindLZObject(CLZObject * pLZObj);
	void BindLZObjectWithBufferedSize(CLZObject * pLZObj);
	uint8_t * AppendDataBlock(const void * pBlock, uint32_t dwBlockSize);

	uint8_t * GetCurrentSeekPoint() const;

private:
	void Unmap(LPCVOID data);

private:
	uint8_t * m_pbBufLinkData;
	uint32_t m_dwBufLinkSize;

	uint8_t * m_pbAppendResultDataBlock;
	uint32_t m_dwAppendResultDataSize;

	uint32_t m_seekPosition;
	HANDLE m_hFM;
	uint32_t m_dataOffset;
	uint32_t m_mapSize;
	LPVOID m_lpMapData;
	LPVOID m_lpData;

	CLZObject * m_pLZObj;
};

#endif
