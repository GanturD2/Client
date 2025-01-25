#include "StdAfx.h"

#include <io.h>
#include <cassert>

#include "EterPackManager.h"
#include "EterPackPolicy_CSHybridCrypt.h"

#include "../EterBase/Debug.h"
#include "../EterBase/CRC32.h"

#define PATH_ABSOLUTE_YMIRWORK1 "d:/ymir work/"
#define PATH_ABSOLUTE_YMIRWORK2 "d:\\ymir work\\"


CEterPack * CEterPackManager::FindPack(const char * c_szPathName)
{
	std::string strFileName;

	if (0 == ConvertFileName(c_szPathName, strFileName))
		return &m_RootPack;


	for (auto & itor : m_DirPackMap)
	{
		const std::string & c_rstrName = itor.first;
		CEterPack * pEterPack = itor.second;

		if (CompareName(c_rstrName.c_str(), c_rstrName.length(), strFileName.c_str()))
			return pEterPack;
	}


	return nullptr;
}

void CEterPackManager::SetCacheMode()
{
	m_isCacheMode = true;
}

void CEterPackManager::SetRelativePathMode()
{
	m_bTryRelativePath = true;
}


// StringPath std::string 버전
int CEterPackManager::ConvertFileName(const char * c_szFileName, std::string & rstrFileName)
{
	rstrFileName = c_szFileName;
	stl_lowers(rstrFileName);

	int iCount = 0;
	for (auto & i : rstrFileName)
	{
		if (i == '/')
			++iCount;
		else if (i == '\\')
		{
			i = '/';
			++iCount;
		}
	}

	return iCount;
}

bool CEterPackManager::CompareName(const char * c_szDirectoryName, uint32_t /*dwLength*/, const char * c_szFileName)
{
	const char * c_pszSrc = c_szDirectoryName;
	const char * c_pszCmp = c_szFileName;

	while (*c_pszSrc)
	{
		if (*(c_pszSrc++) != *(c_pszCmp++))
			return false;

		if (!*c_pszCmp)
			return false;
	}

	return true;
}

void CEterPackManager::LoadStaticCache(const char * c_szFileName)
{
	if (!m_isCacheMode)
		return;

	std::string strFileName;
	if (0 == ConvertFileName(c_szFileName, strFileName))
		return;

	uint32_t dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());

	std::unordered_map<uint32_t, SCache>::iterator f = m_kMap_dwNameKey_kCache.find(dwFileNameHash);
	if (m_kMap_dwNameKey_kCache.end() != f)
		return;

	CMappedFile kMapFile;
	const void * c_pvData;
	if (!Get(kMapFile, c_szFileName, &c_pvData))
		return;

	SCache kNewCache;
	kNewCache.m_dwBufSize = kMapFile.Size();
	kNewCache.m_abBufData = new uint8_t[kNewCache.m_dwBufSize];
	memcpy(kNewCache.m_abBufData, c_pvData, kNewCache.m_dwBufSize);
	m_kMap_dwNameKey_kCache.emplace(dwFileNameHash, kNewCache);
}

CEterPackManager::SCache * CEterPackManager::__FindCache(uint32_t dwFileNameHash)
{
	if (auto f = m_kMap_dwNameKey_kCache.find(dwFileNameHash); m_kMap_dwNameKey_kCache.end() != f)
		return &f->second;
	return nullptr;
}

void CEterPackManager::__ClearCacheMap()
{
	for (auto & i : m_kMap_dwNameKey_kCache)
		delete[] i.second.m_abBufData;

	m_kMap_dwNameKey_kCache.clear();
}

struct TimeChecker
{
	explicit TimeChecker(const char * name) : name(name) { baseTime = timeGetTime(); }
	~TimeChecker() { printf("load %s (%lu)\n", name, timeGetTime() - baseTime); }

	const char * name;
	uint32_t baseTime;
};

#ifdef USE_EPK_NEW_SIGNATURE
bool CEterPackManager::_Get(const char* c_szFileName, int, const int pass, int, int, int, LPCVOID* pData, CMappedFile& rMappedFile)
#else
bool CEterPackManager::Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
#endif
{
#ifdef USE_EPK_NEW_SIGNATURE
	if (USE_EPK_NEW_SIGNATURE != pass)
		return true;
#endif

	if (m_iSearchMode == SEARCH_FILE_FIRST)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
			return true;

		return GetFromPack(rMappedFile, c_szFileName, pData);
	}

	if (GetFromPack(rMappedFile, c_szFileName, pData))
		return true;

	return GetFromFile(rMappedFile, c_szFileName, pData);
}

struct FinderLock
{
	explicit FinderLock(CRITICAL_SECTION & cs) : p_cs(&cs) { EnterCriticalSection(p_cs); }

	~FinderLock() { LeaveCriticalSection(p_cs); }

	CRITICAL_SECTION * p_cs;
};

#ifdef USE_EPK_NEW_SIGNATURE
bool CEterPackManager::_GetFromPack(LPCVOID* pData, int, const int pass, int, int, int, const char* c_szFileName, CMappedFile& rMappedFile)
#else
bool CEterPackManager::GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
#endif
{
#ifdef USE_EPK_NEW_SIGNATURE
	if (USE_EPK_NEW_SIGNATURE != pass)
		return true;
#endif

	FinderLock lock(m_csFinder);

	static std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
		return m_RootPack.Get(rMappedFile, strFileName.c_str(), pData);


	uint32_t dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
	SCache * pkCache = __FindCache(dwFileNameHash);

	if (pkCache)
	{
		rMappedFile.Link(pkCache->m_dwBufSize, pkCache->m_abBufData);
		return true;
	}

	CEterFileDict::Item * pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

	if (pkFileItem)
		if (pkFileItem->pkPack)
		{
			bool r = pkFileItem->pkPack->Get2(rMappedFile, strFileName.c_str(), pkFileItem->pkInfo, pData);
			return r;
		}

#ifdef _DEBUG
	//TraceError("CANNOT_FIND_PACK_FILE [%s]", strFileName.c_str());	//@fixme401
#endif

	return false;
}

const time_t g_tCachingInterval = 10; // 10초
void CEterPackManager::ArrangeMemoryMappedPack() {}

bool CEterPackManager::GetFromFile(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
#ifdef ENABLE_PREVENT_FILE_READ_FROM_DRIVE
	if (strlen(c_szFileName) > 2 && c_szFileName[1] == ':')
		return false;
#endif

	return rMappedFile.Create(c_szFileName, pData, 0, 0) != 0;
}

bool CEterPackManager::isExistInPack(const char * c_szFileName)
{
	std::string strFileName;

	if (0 == ConvertFileName(c_szFileName, strFileName))
		return m_RootPack.IsExist(strFileName.c_str());


	uint32_t dwFileNameHash = GetCRC32(strFileName.c_str(), strFileName.length());
	CEterFileDict::Item * pkFileItem = m_FileDict.GetItem(dwFileNameHash, strFileName.c_str());

	if (pkFileItem)
	{
		if (pkFileItem->pkPack)
			return pkFileItem->pkPack->IsExist(strFileName.c_str());
	}


	// NOTE : 매치 되는 팩이 없다면 false - [levites]
	return false;
}

bool CEterPackManager::isExist(const char * c_szFileName)
{
	if (m_iSearchMode == SEARCH_PACK_FIRST)
	{
		if (isExistInPack(c_szFileName))
			return true;

		return _access(c_szFileName, 0) == 0;
	}

	if (_access(c_szFileName, 0) == 0)
		return true;

	return isExistInPack(c_szFileName);
}


void CEterPackManager::RegisterRootPack(const char * c_szName)
{
	if (!m_RootPack.Create(m_FileDict, c_szName, ""))
		TraceError("%s: Pack file does not exist", c_szName);
}

const char * CEterPackManager::GetRootPackFileName() const
{
	return m_RootPack.GetDBName();
}

bool CEterPackManager::DecryptPackIV(uint32_t dwPanamaKey)
{
	TEterPackMap::iterator itor = m_PackMap.begin();
	while (itor != m_PackMap.end())
	{
		itor->second->DecryptIV(dwPanamaKey);
		++itor;
	}
	return true;
}

bool CEterPackManager::RegisterPackWhenPackMaking(const char * c_szName, const char * c_szDirectory, CEterPack * pPack)
{
	m_PackMap.emplace(c_szName, pPack);
	m_PackList.push_front(pPack);

	m_DirPackMap.emplace(c_szDirectory, pPack);
	return true;
}


bool CEterPackManager::RegisterPack(const char * c_szName, const char * c_szDirectory, const uint8_t * c_pbIV)
{
	CEterPack * pEterPack;
	{
		auto itor = m_PackMap.find(c_szName);

		if (m_PackMap.end() == itor)
		{
			bool bReadOnly = true;

			pEterPack = new CEterPack;
			if (pEterPack->Create(m_FileDict, c_szName, c_szDirectory, bReadOnly, c_pbIV))
				m_PackMap.emplace(c_szName, pEterPack);
			else
			{
#ifdef _DEBUG
				Tracef("The eterpack doesn't exist [%s]\n", c_szName);
#endif
				delete pEterPack;
				pEterPack = nullptr;
				return false;
			}
		}
		else
			pEterPack = itor->second;
	}

	if (c_szDirectory && c_szDirectory[0] != '*')
	{
		auto itor = m_DirPackMap.find(c_szDirectory);
		if (m_DirPackMap.end() == itor)
		{
			m_PackList.push_front(pEterPack);
			m_DirPackMap.emplace(c_szDirectory, pEterPack);
		}
	}

	return true;
}

void CEterPackManager::SetSearchMode(bool bPackFirst)
{
	m_iSearchMode = bPackFirst ? SEARCH_PACK_FIRST : SEARCH_FILE_FIRST;
}

int CEterPackManager::GetSearchMode() const
{
	return m_iSearchMode;
}

CEterPackManager::CEterPackManager()
{
	InitializeCriticalSection(&m_csFinder);
}

CEterPackManager::~CEterPackManager()
{
	__ClearCacheMap();

	TEterPackMap::iterator i = m_PackMap.begin();
	TEterPackMap::iterator e = m_PackMap.end();
	while (i != e)
	{
		delete i->second;
		++i;
	}
	DeleteCriticalSection(&m_csFinder);
}

void CEterPackManager::RetrieveHybridCryptPackKeys(const uint8_t * pStream)
{
	////dump file format
	//total packagecnt (4byte)
	//	for	packagecntpackage
	//		db name hash ( stl.h stringhash )
	//		extension cnt( 4byte)
	//		for	extension cnt
	//			ext hash ( stl.h stringhash )
	//			key-16byte
	//			iv-16byte
	int iMemOffset = 0;

	int iPackageCnt;
	uint32_t dwPackageNameHash;

	memcpy(&iPackageCnt, pStream + iMemOffset, sizeof(int));
	iMemOffset += sizeof(iPackageCnt);

	for (int i = 0; i < iPackageCnt; ++i)
	{
		int iRecvedCryptKeySize = 0;
		memcpy(&iRecvedCryptKeySize, pStream + iMemOffset, sizeof(iRecvedCryptKeySize));
		iRecvedCryptKeySize -=
			sizeof(dwPackageNameHash); // 서버에서 받은 key stream에는 filename hash가 포함되어 있으므로, hash 사이즈 만큼 배줌.
		iMemOffset += sizeof(iRecvedCryptKeySize);

		memcpy(&dwPackageNameHash, pStream + iMemOffset, sizeof(dwPackageNameHash));
		iMemOffset += sizeof(dwPackageNameHash);

		for (auto & cit : m_PackMap)
		{
			auto ssvv = std::string(cit.first);
			std::string noPathName = CFileNameHelper::NoPath(ssvv);
			if (dwPackageNameHash == stringhash::GetHash(noPathName))
			{
				EterPackPolicy_CSHybridCrypt * pCryptPolicy = cit.second->GetPackPolicy_HybridCrypt();
				int iHavedCryptKeySize = pCryptPolicy->ReadCryptKeyInfoFromStream(pStream + iMemOffset);
				if (iRecvedCryptKeySize != iHavedCryptKeySize)
					TraceError("CEterPackManager::RetrieveHybridCryptPackKeys	cryptokey length of file(%s) is not matched. received(%d) "
							   "!= haved(%d)",
							   noPathName.c_str(), iRecvedCryptKeySize, iHavedCryptKeySize);
				break;
			}
		}
		iMemOffset += iRecvedCryptKeySize;
	}
}

void CEterPackManager::RetrieveHybridCryptPackSDB(const uint8_t * pStream)
{
	//cnt
	//for cnt
	//uint32_t				dwPackageIdentifier;
	//uint32_t				dwFileIdentifier;
	//std::vector<uint8_t>	vecSDBStream;
	int iReadOffset = 0;
	int iSDBInfoCount = 0;

	memcpy(&iSDBInfoCount, pStream + iReadOffset, sizeof(int));
	iReadOffset += sizeof(int);

	for (int i = 0; i < iSDBInfoCount; ++i)
	{
		uint32_t dwPackgeIdentifier;
		memcpy(&dwPackgeIdentifier, pStream + iReadOffset, sizeof(uint32_t));
		iReadOffset += sizeof(uint32_t);

		for (auto & cit : m_PackMap)
		{
			auto ssvv = std::string(cit.first);
			std::string noPathName = CFileNameHelper::NoPath(ssvv);
			if (dwPackgeIdentifier == stringhash::GetHash(noPathName))
			{
				EterPackPolicy_CSHybridCrypt * pCryptPolicy = cit.second->GetPackPolicy_HybridCrypt();
				iReadOffset += pCryptPolicy->ReadSupplementatyDataBlockFromStream(pStream + iReadOffset);
				break;
			}
		}
	}
}


void CEterPackManager::WriteHybridCryptPackInfo(const char * pFileName)
{
	//NOTE : this file format contains a little bit of redundant data.
	//however it`s better for seperating cryptkey & supplementary data block.

	//dump file format

	//SDB data offset(4)

	// about cryptkey
	//total packagecnt (4byte)
	//	for	packagecnt
	//		db name hash 4byte( stl.h stringhash )
	//		extension cnt( 4byte)
	//		for	extension cnt
	//			ext hash ( stl.h stringhash )
	//			key-16byte
	//			iv-16byte

	//about SDB data
	//total packagecnt (4byte)
	//	for	packagecnt
	//		db name hash 4byte( stl.h stringhash ) +child node size(4byte)
	//		sdb file cnt( 4byte )
	//		for	sdb file cnt
	//			filename hash ( stl.h stringhash )
	//			related map name size(4), relate map name
	//			sdb block size( 1byte )
	//			sdb blocks

	CFileBase keyFile;

	if (!keyFile.Create(pFileName, CFileBase::FILEMODE_WRITE))
	{
		//TODO : write log
		return;
	}

	int iKeyPackageCount = 0;

	//write later ( SDB Offset & PackageCnt for Key )
	keyFile.SeekCur(2 * sizeof(int));

	for (auto & cit : m_PackMap)
	{
		EterPackPolicy_CSHybridCrypt * pPolicy = cit.second->GetPackPolicy_HybridCrypt();
		if (!pPolicy || !pPolicy->IsContainingCryptKey())
			continue;

		iKeyPackageCount++;

		auto ssvv = std::string(cit.first);
		std::string noPathName = CFileNameHelper::NoPath(ssvv);

		uint32_t dwPackNamehash = stringhash::GetHash(noPathName);

		CMakePackLog::GetSingleton().Writef("CEterPackManager::WriteHybridCryptPackInfo PackName : %s, Hash : %x", noPathName.c_str(),
											dwPackNamehash);
		keyFile.Write(&dwPackNamehash, sizeof(uint32_t));

		pPolicy->WriteCryptKeyToFile(keyFile);
	}

	//Write SDB Data
	int iSDBDataOffset = keyFile.GetPosition();
	int iSDBPackageCnt = 0;

	//Write SDB PackageCnt Later
	keyFile.SeekCur(sizeof(int));
	for (auto & cit : m_PackMap)
	{
		EterPackPolicy_CSHybridCrypt * pPolicy = cit.second->GetPackPolicy_HybridCrypt();
		if (!pPolicy || !pPolicy->IsContainingSDBFile())
			continue;

		iSDBPackageCnt++;

		auto ssvv1 = std::string(cit.first);
		auto noPathName = CFileNameHelper::NoPath(ssvv1);

		uint32_t dwPackNamehash = stringhash::GetHash(noPathName);
		keyFile.Write(&dwPackNamehash, sizeof(uint32_t));

		int iSDBSizeWriteOffset = keyFile.GetPosition();
		keyFile.SeekCur(sizeof(int));

		pPolicy->WriteSupplementaryDataBlockToFile(keyFile);
		int iSDBSizeAfterWrite = keyFile.GetPosition();

		keyFile.Seek(iSDBSizeWriteOffset);

		int iSDBSize = iSDBSizeAfterWrite - (iSDBSizeWriteOffset + 4);
		keyFile.Write(&iSDBSize, sizeof(int));

		keyFile.Seek(iSDBSizeAfterWrite);
	}

	//write sdb data start offset & package cnt
	keyFile.Seek(0);
	keyFile.Write(&iSDBDataOffset, sizeof(int));
	keyFile.Write(&iKeyPackageCount, sizeof(int));

	keyFile.Seek(iSDBDataOffset);
	keyFile.Write(&iSDBPackageCnt, sizeof(int));

	keyFile.Close();
}
