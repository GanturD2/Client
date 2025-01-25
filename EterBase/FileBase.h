#ifndef __INC_ETERBASE_FILEBASE_H__
#define __INC_ETERBASE_FILEBASE_H__

#include <Windows.h>

class CFileBase
{
public:
	enum EFileMode
	{
		FILEMODE_READ = (1 << 0),
		FILEMODE_WRITE = (1 << 1)
	};

	CFileBase();
	virtual ~CFileBase();

	void Destroy();
	void Close();

	BOOL Create(const char * filename, EFileMode mode);
	uint32_t Size() const;
	void SeekCur(uint32_t size) const;
	void Seek(uint32_t offset) const;
	uint32_t GetPosition() const;

	virtual BOOL Write(const void * src, int bytes);
	BOOL Read(void * dest, int bytes) const;

	char * GetFileName();
	BOOL IsNull() const;

protected:
	int m_mode;
	char m_filename[MAX_PATH + 1];
	HANDLE m_hFile;
	uint32_t m_dwSize;
};

#endif
