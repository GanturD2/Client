#ifndef __INC_ETERPACKLIB_ETERPACK_H__
#define __INC_ETERPACKLIB_ETERPACK_H__

#include <list>
#include <unordered_map>

#include "../EterBase/MappedFile.h"

#ifndef MAKEFOURCC
#	define MAKEFOURCC(ch0, ch1, ch2, ch3) \
		((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif

#include "md5.h"

namespace eterpack
{
const uint32_t c_PackCC = MAKEFOURCC('E', 'P', 'K', 'D');
const uint32_t c_IndexCC = MAKEFOURCC('E', 'P', 'K', 'D');
const uint32_t c_Version = 2;
// FourCC + Version + m_indexCount
const uint32_t c_HeaderSize = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(long);
} // namespace eterpack

enum EEterPackTypes
{
	DBNAME_MAX_LEN = 255,
	FILENAME_MAX_LEN = 160,
	FREE_INDEX_BLOCK_SIZE = 32768,
	FREE_INDEX_MAX_SIZE = 512,
	DATA_BLOCK_SIZE = 256,

	COMPRESSED_TYPE_NONE = 0,
	COMPRESSED_TYPE_COMPRESS = 1,
	COMPRESSED_TYPE_SECURITY = 2,
	COMPRESSED_TYPE_PANAMA = 3,
	COMPRESSED_TYPE_HYBRIDCRYPT = 4,
	COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB = 5,
	COMPRESSED_TYPE_COUNT = 6
};

#pragma pack(push, 4)
typedef struct SEterPackIndex
{
	long id;
	char filename[FILENAME_MAX_LEN + 1];
	uint32_t filename_crc;
	long real_data_size;
	long data_size;
	uint32_t data_crc;
	long data_position;
	char compressed_type;
} TEterPackIndex;
#pragma pack(pop)

typedef std::unordered_map<uint32_t, TEterPackIndex *> TDataPositionMap;
using TFreeIndexList = std::list<TEterPackIndex *>;


class CEterPack;

class CEterFileDict
{
public:
	struct Item
	{
		Item() = default;

		CEterPack * pkPack{nullptr};
		TEterPackIndex * pkInfo{nullptr};
	};

	enum
	{
		BUCKET_SIZE = 16
	};

	typedef std::unordered_multimap<uint32_t, Item> TDict;

public:
	void InsertItem(CEterPack * pkPack, TEterPackIndex * pkInfo);
	void UpdateItem(CEterPack * pkPack, TEterPackIndex * pkInfo);

	Item * GetItem(uint32_t dwFileNameHash, const char * c_pszFileName);

	const TDict & GetDict() const { return m_dict; }

private:
	TDict m_dict;
};

class EterPackPolicy_CSHybridCrypt;

class CEterPack
{
public:
	CEterPack();
	virtual ~CEterPack();

	void Destroy();
	bool Create(CEterFileDict & rkFileDict, const char * dbname, const char * pathName, bool bReadOnly = true,
				const uint8_t * iv = nullptr);
	bool DecryptIV(uint32_t dwPanamaKey);

	const std::string & GetPathName() const;
	const char * GetDBName() const;

	bool Get(CMappedFile & out_file, const char * filename, LPCVOID * data);
	bool Get2(CMappedFile & out_file, const char * filename, TEterPackIndex * index, LPCVOID * data) const;


	bool Put(const char * filename, const char * sourceFilename, uint8_t packType, const std::string & strRelateMapName);
	bool Put(const char * filename, LPCVOID data, long len, uint8_t packType);

	bool Delete(const char * filename);

	bool Extract();

	long GetFragmentSize() const;

	bool IsExist(const char * filename);

	TDataPositionMap & GetIndexMap();

	bool EncryptIndexFile();
	bool DecryptIndexFile();

	uint32_t DeleteUnreferencedData(); // 몇개가 삭제 되었는지 리턴 한다.

	bool GetNames(std::vector<std::string> * retNames);

	EterPackPolicy_CSHybridCrypt * GetPackPolicy_HybridCrypt() const;

private:
	bool __BuildIndex(CEterFileDict & rkFileDict, bool bOverwrite = false);

	bool CreateIndexFile() const;
	TEterPackIndex * FindIndex(const char * filename);
	long GetNewIndexPosition(CFileBase & file);
	TEterPackIndex * NewIndex(CFileBase & file, const char * filename, long size);
	void WriteIndex(CFileBase & file, TEterPackIndex * index) const;
	static int GetFreeBlockIndex(long size);
	void PushFreeIndex(TEterPackIndex * index);

	bool CreateDataFile() const;
	static long GetNewDataPosition(CFileBase & file);
	static bool ReadData(CFileBase & file, TEterPackIndex * index, LPVOID data, long maxsize);
	static bool WriteData(CFileBase & file, TEterPackIndex * index, LPCVOID data);
	static bool WriteNewData(CFileBase & file, TEterPackIndex * index, LPCVOID data);

	bool Delete(TEterPackIndex * pIndex);

protected:
	CMappedFile m_file;

	char * m_file_data;
	unsigned m_file_size;

	long m_indexCount{0};
	bool m_bEncrypted{false};

	char m_dbName[DBNAME_MAX_LEN + 1];
	char m_indexFileName[MAX_PATH + 1];
	std::vector<TEterPackIndex> m_indexData;
	long m_FragmentSize{0};
	bool m_bReadOnly{false};
	bool m_bDecrypedIV{false};

	std::unordered_map<uint32_t, uint32_t> m_map_indexRefCount;
	TDataPositionMap m_DataPositionMap;
	TFreeIndexList m_FreeIndexList[FREE_INDEX_MAX_SIZE + 1]; // MAX 도 억세스 하므로 + 1 크기만큼 만든다.

	std::string m_stDataFileName;
	std::string m_stPathName;


	std::unique_ptr<EterPackPolicy_CSHybridCrypt> m_pCSHybridCryptPolicy;

private:
	static void __CreateFileNameKey_Panama(const char * filename, uint8_t * key, unsigned int keySize);
	bool __Decrypt_Panama(const char * filename, const uint8_t * data, SIZE_T dataSize, CLZObject & zObj) const;
	bool __Encrypt_Panama(const char * filename, const uint8_t * data, SIZE_T dataSize, CLZObject & zObj) const;
	std::string m_stIV_Panama;
};

class CMakePackLog
{
public:
	static CMakePackLog & GetSingleton();

public:
	CMakePackLog();
	~CMakePackLog();

	void SetFileName(const char * c_szFileName);

	void Writef(const char * c_szFormat, ...);
	void Writenf(const char * c_szFormat, ...);
	void Write(const char * c_szBuf);

	void WriteErrorf(const char * c_szFormat, ...);
	void WriteErrornf(const char * c_szFormat, ...);
	void WriteError(const char * c_szBuf);

	void FlushError() const;

private:
	void __Write(const char * c_szBuf, int nBufLen);
	void __WriteError(const char * c_szBuf, int nBufLen);
	bool __IsLogMode() const;

private:
	FILE * m_fp;
	FILE * m_fp_err;

	std::string m_stFileName;
	std::string m_stErrorFileName;
};

#endif
