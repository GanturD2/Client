#include "StdAfx.h"
#include "EterPackCursor.h"
#include "Inline.h"

CEterPackCursor::CEterPackCursor(CEterPack * pack) : m_pPack(pack), m_pData(nullptr), m_ReadPoint(0) {}

CEterPackCursor::~CEterPackCursor()
{
	Close();
}

bool CEterPackCursor::Open(const char * filename)
{
	assert(m_pPack != nullptr);

	char tmpFilename[MAX_PATH + 1];
	strncpy(tmpFilename, filename, MAX_PATH);
	inlineConvertPackFilename(tmpFilename);

	return m_pPack->Get(m_file, tmpFilename, &m_pData);
}

void CEterPackCursor::Close()
{
	m_file.Destroy();
	m_pData = nullptr;
	m_ReadPoint = 0;
}

void CEterPackCursor::Seek(long offset)
{
	m_ReadPoint = std::max(0l, std::min(Size(), offset));
}

bool CEterPackCursor::Read(LPVOID data, long size)
{
	if (m_file.IsNull())
		return false;

	if (m_ReadPoint + size > Size())
		return false;

	memcpy(data, static_cast<const char *>(m_pData) + m_ReadPoint, size);
	m_ReadPoint += size;
	return true;
}

long CEterPackCursor::Size() const
{
	if (m_file.IsNull())
		return 0;

	return m_file.Size();
}
