#pragma once

#include <Windows.h>
#include <unordered_map>
#include "../EterBase/Singleton.h"
#include "../EterBase/Stl.h"

#include "EterPack.h"
#include "../UserInterface/Locale_inc.h"

class CEterPackManager : public CSingleton<CEterPackManager>
{
public:
	struct SCache
	{
		uint8_t * m_abBufData;
		uint32_t m_dwBufSize;
	};

public:
	enum ESearchModes
	{
		SEARCH_FILE_FIRST,
		SEARCH_PACK_FIRST
	};

	using TEterPackList = std::list<CEterPack *>;
	typedef std::unordered_map<std::string, CEterPack *, stringhash> TEterPackMap;

public:
	CEterPackManager();
	~CEterPackManager() override;

	void SetCacheMode();
	void SetRelativePathMode();

	void LoadStaticCache(const char * c_szFileName);

	void SetSearchMode(bool bPackFirst);
	int GetSearchMode() const;

#ifdef USE_EPK_NEW_SIGNATURE
	__forceinline bool Get(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
	{
		return _Get(c_szFileName, 1, USE_EPK_NEW_SIGNATURE, 1, 1, 1, pData, rMappedFile);
	}
	bool _Get(const char* c_szFileName, int, const int pass, int, int, int, LPCVOID* pData, CMappedFile& rMappedFile);

	__forceinline bool GetFromPack(CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData)
	{
		return _GetFromPack(pData, 1, USE_EPK_NEW_SIGNATURE, 1, 1, 1, c_szFileName, rMappedFile);
	}
	bool _GetFromPack(LPCVOID* pData, int, const int pass, int, int, int, const char* c_szFileName, CMappedFile& rMappedFile);
#else
	bool Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
	bool GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
#endif

	static bool GetFromFile(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
	bool isExist(const char * c_szFileName);
	bool isExistInPack(const char * c_szFileName);

	bool RegisterPack(const char * c_szName, const char * c_szDirectory, const uint8_t * c_pbIV = nullptr);
	void RegisterRootPack(const char * c_szName);
	bool RegisterPackWhenPackMaking(const char * c_szName, const char * c_szDirectory, CEterPack * pPack);


	bool DecryptPackIV(uint32_t dwPanamaKey);

	const char * GetRootPackFileName() const;

	//for hybridcrypt
	void WriteHybridCryptPackInfo(const char * pFileName);

	void RetrieveHybridCryptPackKeys(const uint8_t * pStream);
	void RetrieveHybridCryptPackSDB(const uint8_t * pStream);

	// 메모리에 매핑된 팩들 가운데, 정리해야할 것들 정리.
public:
	static void ArrangeMemoryMappedPack();

protected:
	static int ConvertFileName(const char * c_szFileName, std::string & rstrFileName); // StringPath std::string 버전
	static bool CompareName(const char * c_szDirectoryName, uint32_t iLength, const char * c_szFileName);

	CEterPack * FindPack(const char * c_szPathName);

	SCache * __FindCache(uint32_t dwFileNameHash);
	void __ClearCacheMap();

protected:
	bool m_bTryRelativePath{false};
	bool m_isCacheMode{false};
	int m_iSearchMode{SEARCH_FILE_FIRST};

	CEterFileDict m_FileDict;
	CEterPack m_RootPack;
	TEterPackList m_PackList;
	TEterPackMap m_PackMap;
	TEterPackMap m_DirPackMap;

	std::unordered_map<uint32_t, SCache> m_kMap_dwNameKey_kCache;

	CRITICAL_SECTION m_csFinder;
};
