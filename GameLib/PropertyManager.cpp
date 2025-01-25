#include "StdAfx.h"
#include "../EterPack/EterPackManager.h"

#include "PropertyManager.h"
#include "Property.h"

CPropertyManager::CPropertyManager() : m_isFileMode(true) {}

CPropertyManager::~CPropertyManager()
{
	Clear();
}

bool CPropertyManager::Initialize(const char * c_pszPackFileName)
{
	if (c_pszPackFileName)
	{
		if (!m_pack.Create(m_fileDict, c_pszPackFileName, "", true))
		{
			LogBoxf("Cannot open property pack file (filename %s)", c_pszPackFileName);
			return false;
		}

		m_isFileMode = false;

		TDataPositionMap & indexMap = m_pack.GetIndexMap();

		TDataPositionMap::iterator itor = indexMap.begin();

		typedef std::map<uint32_t, TEterPackIndex *> TDataPositionMap;

		int i = 0;

		while (indexMap.end() != itor)
		{
			TEterPackIndex * pIndex = itor->second;
			++itor;

			if (!stricmp("property/reserve", pIndex->filename))
			{
				LoadReservedCRC(pIndex->filename);
				continue;
			}

			if (!Register(pIndex->filename))
				continue;

			++i;
		}
	}
	else
	{
		m_isFileMode = true;
		// NOTE : ���⼭ Property�� ��Ͻ�Ű�� WorldEditor���� �̻��� ���� ;
		//        ����, Property Tree List���� ����� ���Ѿ� �Ǳ� ������ �ٱ��ʿ���.. - [levites]
	}

	return true;
}

bool CPropertyManager::BuildPack()
{
	if (!m_pack.Create(m_fileDict, "property", ""))
		return false;

	WIN32_FIND_DATA fdata;
	HANDLE hFind = FindFirstFile("property\\*", &fdata);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		char szSourceFileName[256 + 1];
		_snprintf(szSourceFileName, sizeof(szSourceFileName), "property\\%s", fdata.cFileName);

		m_pack.Put(fdata.cFileName, szSourceFileName, COMPRESSED_TYPE_NONE, "");
	} while (FindNextFile(hFind, &fdata));

	FindClose(hFind);
	return true;
}

bool CPropertyManager::LoadReservedCRC(const char * c_pszFileName)
{
	CMappedFile file;
	LPCVOID c_pvData;

	if (!CEterPackManager::Instance().Get(file, c_pszFileName, &c_pvData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(file.Size(), c_pvData);

	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		const char * pszLine = textFileLoader.GetLineString(i).c_str();

		if (!pszLine || !*pszLine)
			continue;

		ReserveCRC(atoi(pszLine));
	}

	return true;
}

void CPropertyManager::ReserveCRC(uint32_t dwCRC)
{
	m_ReservedCRCSet.emplace(dwCRC);
}

uint32_t CPropertyManager::GetUniqueCRC(const char * c_szSeed)
{
	std::string stTmp = c_szSeed;

	while (true)
	{
		uint32_t dwCRC = GetCRC32(stTmp.c_str(), stTmp.length());

		if (m_ReservedCRCSet.find(dwCRC) == m_ReservedCRCSet.end() && m_PropertyByCRCMap.find(dwCRC) == m_PropertyByCRCMap.end())
			return dwCRC;

		char szAdd[2];
		_snprintf(szAdd, sizeof(szAdd), "%d", random() % 10);
		stTmp += szAdd;
	}
}

bool CPropertyManager::Register(const char * c_pszFileName, CProperty ** ppProperty)
{
	CMappedFile file;
	LPCVOID c_pvData;

	if (!CEterPackManager::Instance().Get(file, c_pszFileName, &c_pvData))
		return false;

	auto * pProperty = new CProperty(c_pszFileName);

	if (!pProperty->ReadFromMemory(c_pvData, file.Size(), c_pszFileName))
	{
		delete pProperty;
		return false;
	}

	uint32_t dwCRC = pProperty->GetCRC();

	auto itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() != itor)
	{
		Tracef("Property already registered, replace %s to %s\n", itor->second->GetFileName(), c_pszFileName);

		delete itor->second;
		itor->second = pProperty;
	}
	else
		m_PropertyByCRCMap.emplace(dwCRC, pProperty);

	if (ppProperty)
		*ppProperty = pProperty;

	return true;
}

bool CPropertyManager::Get(const char * c_pszFileName, CProperty ** ppProperty)
{
	return Register(c_pszFileName, ppProperty);
}

bool CPropertyManager::Get(uint32_t dwCRC, CProperty ** ppProperty)
{
	auto itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() == itor)
		return false;

	*ppProperty = itor->second;
	return true;
}

bool CPropertyManager::Put(const char * c_pszFileName, const char * c_pszSourceFileName)
{
	if (!CopyFile(c_pszSourceFileName, c_pszFileName, FALSE))
		return false;

	if (!m_isFileMode) // �� ���Ͽ��� ����
	{
		if (!m_pack.Put(c_pszFileName, nullptr, COMPRESSED_TYPE_NONE, ""))
		{
			assert(!"CPropertyManager::Put cannot write to pack file");
			return false;
		}
	}

	Register(c_pszFileName);
	return true;
}

bool CPropertyManager::Erase(uint32_t dwCRC)
{
	auto itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() == itor)
		return false;

	CProperty * pProperty = itor->second;
	m_PropertyByCRCMap.erase(itor);

	DeleteFile(pProperty->GetFileName());
	ReserveCRC(pProperty->GetCRC());

	if (!m_isFileMode) // ���� ��尡 �ƴϸ� �ѿ����� ����
		m_pack.Delete(pProperty->GetFileName());

	msl::file_ptr fPtr("property/reserve", "a+");
	if (!fPtr)
		LogBox("Cannot open the CRC file 'property/reserve'.");
	else
	{
		char szCRC[64 + 1];
		_snprintf(szCRC, sizeof(szCRC), "%u\r\n", pProperty->GetCRC());
		fputs(szCRC, fPtr.get());
	}

	delete pProperty;
	return true;
}

bool CPropertyManager::Erase(const char * c_pszFileName)
{
	CProperty * pProperty = nullptr;

	if (Get(c_pszFileName, &pProperty))
		return Erase(pProperty->GetCRC());

	return false;
}

void CPropertyManager::Clear()
{
	stl_wipe_second(m_PropertyByCRCMap);
}
