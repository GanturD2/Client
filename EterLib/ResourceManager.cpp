#include "StdAfx.h"
#include <io.h>
#include "../EterBase/CRC32.h"
#include "../EterBase/Timer.h"
#include "../EterBase/Stl.h"
#include "../EterPack/EterPackManager.h"

#include "ResourceManager.h"
#include "GrpImage.h"

int g_iLoadingDelayTime = 20;

const long c_Deleting_Wait_Time = 30000; // 삭제 대기 시간 (30초)
const long c_DeletingCountPerFrame = 30; // 프레임당 체크 리소스 갯수
const long c_Reference_Decrease_Wait_Time = 30000; // 선로딩 리소스의 해제 대기 시간 (30초)

CFileLoaderThread CResourceManager::ms_loadingThread;

void CResourceManager::LoadStaticCache(const char * c_szFileName)
{
	CResource * pkRes = GetResourcePointer(c_szFileName);
	if (!pkRes)
	{
		Lognf(1, "CResourceManager::LoadStaticCache %s - FAILED", c_szFileName);
		return;
	}

	uint32_t dwCacheKey = GetCRC32(c_szFileName, strlen(c_szFileName));
	auto f = m_pCacheMap.find(dwCacheKey);
	if (m_pCacheMap.end() != f)
		return;

	pkRes->AddReference();
	m_pCacheMap.emplace(dwCacheKey, pkRes);
}

void CResourceManager::ProcessBackgroundLoading()
{
	auto itor = m_RequestMap.begin();

	while (itor != m_RequestMap.end())
	{
		uint32_t dwFileCRC = itor->first;
		std::string & stFileName = itor->second;

		if (isResourcePointerData(dwFileCRC) || (m_WaitingMap.end() != m_WaitingMap.find(dwFileCRC)))
		{
			//printf("SKP %s\n", stFileName.c_str());
			itor = m_RequestMap.erase(itor);
			continue;
		}

		//printf("REQ %s\n", stFileName.c_str());
		ms_loadingThread.Request(stFileName);
		m_WaitingMap.emplace(dwFileCRC, stFileName);
		itor = m_RequestMap.erase(itor);
		//break; // NOTE: 여기서 break 하면 천천히 로딩 된다.
	}

	uint32_t dwCurrentTime = ELTimer_GetMSec();

	CFileLoaderThread::TData * pData;
	while (ms_loadingThread.Fetch(&pData))
	{
		//printf("LOD %s\n", pData->stFileName.c_str());
		CResource * pResource = GetResourcePointer(pData->stFileName.c_str());

		if (pResource)
		{
			if (pResource->IsEmpty())
			{
				pResource->OnLoad(pData->dwSize, pData->pvBuf);
				pResource->AddReferenceOnly();

				// 여기서 올라간 레퍼런스 카운트를 일정 시간이 지난 뒤에 풀어주기 위하여
				m_pResRefDecreaseWaitingMap.emplace(dwCurrentTime, pResource);
			}
		}

		m_WaitingMap.erase(GetCRC32(pData->stFileName.c_str(), pData->stFileName.size()));

		delete[]((char *) pData->pvBuf);
		delete pData;
	}

	// DO : 일정 시간이 지나고 난뒤 미리 로딩해 두었던 리소스의 레퍼런스 카운트를 감소 시킨다 - [levites]
	long lCurrentTime = ELTimer_GetMSec();

	auto itorRef = m_pResRefDecreaseWaitingMap.begin();

	while (itorRef != m_pResRefDecreaseWaitingMap.end())
	{
		const long & rCreatingTime = itorRef->first;

		if (lCurrentTime - rCreatingTime > c_Reference_Decrease_Wait_Time)
		{
			CResource * pResource = itorRef->second;

			// Decrease Reference Count
			pResource->Release();
			itorRef = m_pResRefDecreaseWaitingMap.erase(itorRef);
			//Tracef("Decrease Pre Loading Resource\n", rCreatingTime);
		}
		else
			++itorRef;
	}
}

void CResourceManager::PushBackgroundLoadingSet(std::set<std::string> & LoadingSet)
{
	auto itor = LoadingSet.begin();

	while (itor != LoadingSet.end())
	{
		uint32_t dwFileCRC = __GetFileCRC(itor->c_str());

		if (0 != isResourcePointerData(dwFileCRC))
		{
			++itor;
			continue;
		}

		m_RequestMap.emplace(dwFileCRC, itor->c_str());
		++itor;
	}
}

void CResourceManager::__DestroyCacheMap()
{
	TResourcePointerMap::iterator i;
	for (i = m_pCacheMap.begin(); i != m_pCacheMap.end(); ++i)
	{
		CResource * pResource = i->second;
		pResource->Release();
	}

	m_pCacheMap.clear();
}

void CResourceManager::__DestroyDeletingResourceMap()
{
	Tracenf("CResourceManager::__DestroyDeletingResourceMap %d", m_ResourceDeletingMap.size());
	for (auto & i : m_ResourceDeletingMap)
		(i.first)->Clear();

	m_ResourceDeletingMap.clear();
}

void CResourceManager::__DestroyResourceMap()
{
	Tracenf("CResourceManager::__DestroyResourceMap %d", m_pResMap.size());

	TResourcePointerMap::iterator i;
	for (i = m_pResMap.begin(); i != m_pResMap.end(); ++i)
	{
		CResource * pResource = i->second;
		pResource->Clear();
	}

	stl_wipe_second(m_pResMap);
}

void CResourceManager::DestroyDeletingList()
{
	CResource::SetDeleteImmediately(true);

	__DestroyCacheMap();
	__DestroyDeletingResourceMap();
}

void CResourceManager::Destroy()
{
	assert(m_ResourceDeletingMap.empty() && "CResourceManager::Destroy - YOU MUST CALL DestroyDeletingList");
	__DestroyResourceMap();
}

void CResourceManager::RegisterResourceNewFunctionPointer(const char * c_szFileExt, CResource * (*pNewFunc)(const char * c_szFileName))
{
	m_pResNewFuncMap[c_szFileExt] = pNewFunc;
}

void CResourceManager::RegisterResourceNewFunctionByTypePointer(int iType, CResource * (*pNewFunc)(const char * c_szFileName))
{
	assert(iType >= 0);
	m_pResNewFuncByTypeMap[iType] = pNewFunc;
}

CResource * CResourceManager::InsertResourcePointer(uint32_t dwFileCRC, CResource * pResource)
{
	auto itor = m_pResMap.find(dwFileCRC);

	if (m_pResMap.end() != itor)
	{
		TraceError("CResource::InsertResourcePointer: %s is already registered\n", pResource->GetFileName());
		assert(!"CResource::InsertResourcePointer: Resource already resistered");
		delete pResource;
		return itor->second;
	}

	m_pResMap.emplace(dwFileCRC, pResource);
	return pResource;
}


int __ConvertPathName(const char * c_szPathName, char * pszRetPathName, int retLen)
{
	const char * pc;
	int len = 0;

	for (pc = c_szPathName; *pc && len < retLen; ++pc, ++len)
	{
		if (*pc == '/')
			*(pszRetPathName++) = '\\';
		else
			*(pszRetPathName++) = (char) korean_tolower(*pc);
	}

	*pszRetPathName = '\0';
	return len;
}

CResource * CResourceManager::GetTypeResourcePointer(const char * c_szFileName, int iType)
{
	if (!c_szFileName || !*c_szFileName)
	{
		assert(c_szFileName != nullptr && *c_szFileName != '\0');
		return nullptr;
	}

	const char * c_pszFile;
	uint32_t dwFileCRC = __GetFileCRC(c_szFileName, &c_pszFile);
	CResource * pResource = FindResourcePointer(dwFileCRC);

	if (pResource) // 이미 리소스가 있으면 리턴 한다.
		return pResource;

	CResource * (*newFunc)(const char *) = nullptr;

	if (iType != -1)
	{
		auto f = m_pResNewFuncByTypeMap.find(iType);

		if (m_pResNewFuncByTypeMap.end() != f)
			newFunc = f->second;
	}
	else
	{
		const char * pcFileExt = strrchr(c_pszFile, '.');

		if (pcFileExt)
		{
			static char s_szFileExt[8 + 1];
			strncpy(s_szFileExt, pcFileExt + 1, 8);

			auto f = m_pResNewFuncMap.find(s_szFileExt);

			if (m_pResNewFuncMap.end() != f)
				newFunc = f->second;
		}
	}

	if (!newFunc)
	{
		Tracenf("ResourceManager::GetResourcePointer: NOT SUPPORT FILE %s", c_pszFile); // @warme668
		return nullptr;
	}

	pResource = InsertResourcePointer(dwFileCRC, newFunc(c_pszFile));
	return pResource;
}

CResource * CResourceManager::GetResourcePointer(const char * c_szFileName)
{
	if (!c_szFileName || !*c_szFileName)
	{
		TraceError("CResourceManager::GetResourcePointer: filename error!");
		return nullptr;
	}

	const char * c_pszFile;
	uint32_t dwFileCRC = __GetFileCRC(c_szFileName, &c_pszFile);
	CResource * pResource = FindResourcePointer(dwFileCRC);

	if (pResource) // 이미 리소스가 있으면 리턴 한다.
		return pResource;

	const char * pcFileExt = strrchr(c_pszFile, '.');

#ifdef _DEBUG
	if (!IsFileExist(c_szFileName))
	{
		if (pcFileExt == nullptr || (stricmp(pcFileExt, ".fnt") != 0))
			TraceError("CResourceManager::GetResourcePointer: File not exist %s", c_szFileName);
	}
#endif

	CResource * (*newFunc)(const char *) = nullptr;

	if (pcFileExt)
	{
		static char s_szFileExt[8 + 1];
		strncpy(s_szFileExt, pcFileExt + 1, 8);

		auto f = m_pResNewFuncMap.find(s_szFileExt);

		if (m_pResNewFuncMap.end() != f)
			newFunc = f->second;
	}

	if (!newFunc)
	{
		TraceError("ResourceManager::GetResourcePointer: NOT SUPPORT FILE %s", c_pszFile);
		return nullptr;
	}

	pResource = InsertResourcePointer(dwFileCRC, newFunc(c_pszFile));
	return pResource;
}

CResource * CResourceManager::FindResourcePointer(uint32_t dwFileCRC)
{
	auto itor = m_pResMap.find(dwFileCRC);

	if (m_pResMap.end() == itor)
		return nullptr;

	return itor->second;
}

bool CResourceManager::isResourcePointerData(uint32_t dwFileCRC)
{
	auto itor = m_pResMap.find(dwFileCRC);

	if (m_pResMap.end() == itor)
		return false;

	return (itor->second)->IsData();
}

uint32_t CResourceManager::__GetFileCRC(const char * c_szFileName, const char ** c_ppszLowerFileName)
{
	static char s_szFullPathFileName[MAX_PATH];
	const char * src = c_szFileName;
	char * dst = s_szFullPathFileName;
	int len = 0;

	while (src[len])
	{
		if (src[len] == '/')
			dst[len] = '\\';
		else
			dst[len] = (char) korean_tolower(src[len]);

		++len;
	}

	dst[len] = '\0';

	if (c_ppszLowerFileName)
		*c_ppszLowerFileName = &s_szFullPathFileName[0];

	return (GetCRC32(s_szFullPathFileName, len));
}

typedef struct SDumpData
{
	const char * filename;
	float KB;
	uint32_t cost;
} TDumpData;

bool DumpKBCompare(const TDumpData & lhs, const TDumpData & rhs)
{
	return (lhs.KB > rhs.KB) ? true : false;
}

bool DumpCostCompare(const TDumpData & lhs, const TDumpData & rhs)
{
	return (lhs.cost > rhs.cost) ? true : false;
}

struct FDumpPrint
{
	FILE * m_fp;
	static float m_totalKB;

	void operator()(TDumpData & data)
	{
		m_totalKB += data.KB;
		fprintf(m_fp, "%6.1f %s\n", data.KB, data.filename);
	}
};

float FDumpPrint::m_totalKB;

struct FDumpCostPrint
{
	FILE * m_fp;

	void operator()(TDumpData & data) { fprintf(m_fp, "%-4u %s\n", data.cost, data.filename); }
};

void CResourceManager::DumpFileListToTextFile(const char * c_szFileName)
{
	std::vector<TDumpData> dumpVector;

	for (auto & i : m_pResMap)
	{
		CResource * pResource = i.second;
		TDumpData data;

		if (pResource->IsEmpty())
			continue;

		data.filename = pResource->GetFileName();

		int filesize;

		const char * ext = strrchr(data.filename, '.');

		if (pResource->IsType(CGraphicImage::Type()) && strnicmp(ext, ".sub", 4) != 0)
			filesize = (msl::inherit_cast<CGraphicImage *>(pResource))->GetWidth() *
				(msl::inherit_cast<CGraphicImage *>(pResource))->GetHeight() * 4;
		else
		{
			msl::file_ptr fPtr2(data.filename, "rb");

			if (fPtr2)
				filesize = fPtr2.size();
			else
				filesize = 0;
		}

		data.KB = (float) filesize / (float) 1024;
		data.cost = pResource->GetLoadCostMilliSecond();

		dumpVector.emplace_back(data);
	}

	msl::file_ptr fPtr(c_szFileName, "w");

	if (fPtr)
	{
		std::sort(dumpVector.begin(), dumpVector.end(), DumpKBCompare);

		FDumpPrint DumpPrint;
		DumpPrint.m_fp = fPtr.get();
		DumpPrint.m_totalKB = 0;

		std::for_each(dumpVector.begin(), dumpVector.end(), DumpPrint);
		fprintf(fPtr.get(), "total: %.2fmb", DumpPrint.m_totalKB / 1024.0f);

		FDumpCostPrint DumpCostPrint;
		DumpCostPrint.m_fp = fPtr.get();

		std::sort(dumpVector.begin(), dumpVector.end(), DumpCostCompare);
		std::for_each(dumpVector.begin(), dumpVector.end(), DumpCostPrint);
		fprintf(fPtr.get(), "total: %.2fmb", DumpPrint.m_totalKB / 1024.0f);
	}
}

bool CResourceManager::IsFileExist(const char * c_szFileName)
{
	return CEterPackManager::Instance().isExist(c_szFileName);
}

void CResourceManager::Update()
{
	uint32_t CurrentTime = ELTimer_GetMSec();
	CResource * pResource;
	int Count = 0;

	auto itor = m_ResourceDeletingMap.begin();

	while (itor != m_ResourceDeletingMap.end())
	{
		pResource = itor->first;

		if (CurrentTime >= itor->second)
		{
			if (pResource->canDestroy())
			{
				//Tracef("Resource Clear %s\n", pResource->GetFileName());
				pResource->Clear();
			}

			itor = m_ResourceDeletingMap.erase(itor);

			if (++Count >= c_DeletingCountPerFrame)
				break;
		}
		else
			++itor;
	}

	ProcessBackgroundLoading();
}

void CResourceManager::ReserveDeletingResource(CResource * pResource)
{
	uint32_t dwCurrentTime = ELTimer_GetMSec();
	m_ResourceDeletingMap.emplace(pResource, dwCurrentTime + c_Deleting_Wait_Time);
}

CResourceManager::CResourceManager() = default;

CResourceManager::~CResourceManager()
{
	Destroy();
}
