#include "StdAfx.h"

#include <Windows.h>
#include <mmsystem.h>
#include <cassert>

#include "EterPack.h"
#include "Inline.h"
#include "EterPackPolicy_CSHybridCrypt.h"

#pragma warning(push, 3)
#include <cryptopp/cryptlib.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/tiger.h>
#include <cryptopp/sha.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/whrlpool.h>
#include <cryptopp/panama.h>

#include <cryptopp/cryptoppLibLink.h>
#pragma warning(pop)

#include "../EterBase/Utils.h"
#include "../EterBase/Debug.h"
#include "../EterBase/CRC32.h"

#include <iostream>
#include <fstream>

void CMakePackLog::SetFileName(const char * c_szFileName)
{
	m_stFileName = c_szFileName;
	m_stFileName += ".log";
	m_stErrorFileName = c_szFileName;
	m_stErrorFileName += ".err";
}

CMakePackLog & CMakePackLog::GetSingleton()
{
	static CMakePackLog s_kMakePackLog;
	return s_kMakePackLog;
}

CMakePackLog::CMakePackLog()
{
	m_fp = nullptr;
	m_fp_err = nullptr;
}

CMakePackLog::~CMakePackLog()
{
	if (nullptr != m_fp)
	{
		fclose(m_fp);
		m_fp = nullptr;
	}
	if (nullptr != m_fp_err)
	{
		fclose(m_fp_err);
		m_fp_err = nullptr;
	}
}

bool CMakePackLog::__IsLogMode() const
{
	return !m_stFileName.empty();
}

void CMakePackLog::Writef(const char * c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024];
	int nBufLen = _vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	szBuf[nBufLen] = '\0';
	__Write(szBuf, nBufLen);
}

void CMakePackLog::Writenf(const char * c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024 + 1];
	int nBufLen = _vsnprintf(szBuf, sizeof(szBuf) - 1, c_szFormat, args);
	if (nBufLen > 0)
	{
		szBuf[nBufLen++] = '\n';
		szBuf[nBufLen] = '\0';
	}
	__Write(szBuf, nBufLen);
}

void CMakePackLog::Write(const char * c_szBuf)
{
	if (!__IsLogMode())
		return;

	__Write(c_szBuf, strlen(c_szBuf) + 1);
}

void CMakePackLog::__Write(const char * c_szBuf, int nBufLen)
{
	if (!__IsLogMode())
		return;

	if (nullptr == m_fp)
		m_fp = fopen(m_stFileName.c_str(), "w");

	fwrite(c_szBuf, nBufLen, 1, m_fp);

	printf("%s", c_szBuf);
}


void CMakePackLog::WriteErrorf(const char * c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024];
	int nBufLen = _vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	szBuf[nBufLen] = '\0';
	__WriteError(szBuf, nBufLen);
}

void CMakePackLog::WriteErrornf(const char * c_szFormat, ...)
{
	if (!__IsLogMode())
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[1024 + 1];
	int nBufLen = _vsnprintf(szBuf, sizeof(szBuf) - 1, c_szFormat, args);
	if (nBufLen > 0)
	{
		szBuf[nBufLen++] = '\n';
		szBuf[nBufLen] = '\0';
	}
	__WriteError(szBuf, nBufLen);
}

void CMakePackLog::WriteError(const char * c_szBuf)
{
	if (!__IsLogMode())
		return;

	__WriteError(c_szBuf, strlen(c_szBuf) + 1);
}

void CMakePackLog::__WriteError(const char * c_szBuf, int nBufLen)
{
	if (!__IsLogMode())
		return;

	if (nullptr == m_fp_err)
		m_fp_err = fopen(m_stErrorFileName.c_str(), "w");

	fwrite(c_szBuf, nBufLen, 1, m_fp_err);

	printf("Error: %s", c_szBuf);
}

void CMakePackLog::FlushError() const
{
	std::wifstream iFile(m_stErrorFileName.c_str());
	std::istream_iterator<std::wstring, wchar_t> iit(iFile);
	std::istream_iterator<std::wstring, wchar_t> eos;

	std::vector<std::wstring> vText;

	std::copy(iit, eos, std::back_inserter(vText));

	std::ostream_iterator<std::wstring, wchar_t, std::char_traits<wchar_t>> oit(std::wcout);

	std::sort(vText.begin(), vText.end());
	std::copy(vText.begin(), vText.end(), oit);
}
///////////////////////////////////////////////////////////////////////////////
CEterPack::CEterPack()
{
	m_pCSHybridCryptPolicy = std::make_unique<EterPackPolicy_CSHybridCrypt>();
}

CEterPack::~CEterPack()
{
	Destroy();
}

void CEterPack::Destroy()
{
	m_bReadOnly = false;
	m_bEncrypted = false;
	m_indexCount = 0;
	m_DataPositionMap.clear();

	for (auto & i : m_FreeIndexList)
		i.clear();

	m_indexData.clear();

	m_FragmentSize = 0;

	msl::refill(m_dbName);
	msl::refill(m_indexFileName);
}

const std::string & CEterPack::GetPathName() const
{
	return m_stPathName;
}

bool CEterPack::Create(CEterFileDict & rkFileDict, const char * dbname, const char * pathName, bool bReadOnly, const uint8_t * iv)
{
	if (iv)
	{
		m_stIV_Panama.assign(reinterpret_cast<const char *>(iv), 32);
		m_bDecrypedIV = false;
	}

	m_stPathName = pathName;

	strncpy(m_dbName, dbname, DBNAME_MAX_LEN);

	strncpy(m_indexFileName, dbname, MAX_PATH);
	strcat(m_indexFileName, ".eix");

	m_stDataFileName = dbname;
	m_stDataFileName += ".epk";

	m_bReadOnly = bReadOnly;

	// bReadOnly ��尡 �ƴϰ� ������ ���̽��� �����ٸ� ���� ����
	if (!CreateIndexFile())
		return false;

	if (!CreateDataFile())
		return false;

	bool bOverwrite = (iv != nullptr);
	__BuildIndex(rkFileDict, bOverwrite);

	if (m_bReadOnly)
	{
		//m_bIsDataLoaded = true;
		//if (!m_file.Create(m_stDataFileName.c_str(), (const void**)&m_file_data, 0, 0))
		//	return false;
	}
	else
		DecryptIndexFile();

	return true;
}

bool CEterPack::DecryptIV(uint32_t dwPanamaKey)
{
	if (m_stIV_Panama.length() != 32)
		return false;

	if (m_bDecrypedIV) // �̹� ��ȣȭ�� Ǯ������ �ٽ� ó�� ����
		return true;

	auto ivs = reinterpret_cast<uint32_t *>(&m_stIV_Panama[0]);
	for (size_t i = 0; i != m_stIV_Panama.length() / sizeof(uint32_t); ++i)
		ivs[i] ^= dwPanamaKey + i * 16777619;

	m_bDecrypedIV = true;
	return true;
}

bool CEterPack::DecryptIndexFile()
{
	if (!m_bEncrypted)
		return true;

	CFileBase file;

	if (!file.Create(m_indexFileName, CFileBase::FILEMODE_WRITE))
		return false;

	file.Write(&eterpack::c_IndexCC, sizeof(uint32_t));
	file.Write(&eterpack::c_Version, sizeof(uint32_t));
	file.Write(&m_indexCount, sizeof(long));
	file.Write(m_indexData.data(), sizeof(TEterPackIndex) * m_indexCount);

	file.Close();

	m_bEncrypted = false;
	return true;
}

static uint32_t s_adwEterPackKey[] = {45129401, 92367215, 681285731, 1710201};

static uint32_t s_adwEterPackSecurityKey[] = {78952482, 527348324, 1632942, 486274726};

bool CEterPack::EncryptIndexFile()
{
	CMappedFile file;
	LPCVOID pvData;

	if (0 == file.Create(m_indexFileName, &pvData, 0, 0))
	{
		TraceError("EncryptIndex: Cannot open pack index file! %s", m_dbName);
		return false;
	}

	std::vector<uint8_t> pbData(file.Size());
	memcpy(pbData.data(), pvData, file.Size());

	CLZObject zObj;

	if (!CLZO::Instance().CompressEncryptedMemory(zObj, pbData.data(), file.Size(), s_adwEterPackKey))
	{
		TraceError("EncryptIndex: Cannot encrypt! %s", m_dbName);
		return false;
	}

	file.Destroy();

	while (!DeleteFile(m_indexFileName))
		continue;

	msl::file_ptr fPtr(m_indexFileName, "wb");

	if (!fPtr)
	{
		TraceError("EncryptIndex: Cannot open file for writing! %s", m_dbName);
		return false;
	}

	if (1 != fwrite(zObj.GetBuffer(), zObj.GetSize(), 1, fPtr.get()))
	{
		TraceError("Encryptindex: Cannot write to file! %s", m_indexFileName);
		return false;
	}

	m_bEncrypted = true;
	return true;
}

bool CEterPack::__BuildIndex(CEterFileDict & rkFileDict, bool bOverwrite)
{
	//uint32_t dwBeginTime = ELTimer_GetMSec();
	CMappedFile file;
	LPCVOID pvData;
	CLZObject zObj;

	if (0 == file.Create(m_indexFileName, &pvData, 0, 0))
	{
		TraceError("Cannot open pack index file! %s", m_dbName);
		return false;
	}

	if (file.Size() < eterpack::c_HeaderSize)
	{
		TraceError("Pack index file header error! %s", m_dbName);
		return false;
	}

	const uint32_t fourcc = *static_cast<const uint32_t *>(pvData);

	uint8_t * pbData;
	uint32_t uiFileSize;

	if (fourcc == eterpack::c_IndexCC)
	{
		pbData = (uint8_t *) pvData;
		uiFileSize = file.Size();
	}
#ifdef ENABLE_PACK_TYPE_DIO
	else if (fourcc == CLZObject::ms_dwFourMS)
	{
		m_bEncrypted = true;

		if (!CLZO::Instance().DecompressDio(zObj, static_cast<const uint8_t*>(pvData)))
			return false;

		if (zObj.GetSize() < eterpack::c_HeaderSize)
			return false;

		pbData = zObj.GetBuffer();
		uiFileSize = zObj.GetSize();
	}
#endif
	else if (fourcc == CLZObject::ms_dwFourCC)
	{
		m_bEncrypted = true;

		if (!CLZO::Instance().Decompress(zObj, static_cast<const uint8_t *>(pvData), s_adwEterPackKey))
			return false;

		if (zObj.GetSize() < eterpack::c_HeaderSize)
			return false;

		pbData = zObj.GetBuffer();
		uiFileSize = zObj.GetSize();
	}
	else
	{
		TraceError("Pack index file fourcc error! %s", m_dbName);
		return false;
	}

	pbData += sizeof(uint32_t);

	uint32_t ver = *reinterpret_cast<uint32_t *>(pbData);
	pbData += sizeof(uint32_t);

	if (ver != eterpack::c_Version)
	{
		TraceError("Pack index file version error! %s", m_dbName);
		return false;
	}

	m_indexCount = *reinterpret_cast<long *>(pbData);
	pbData += sizeof(long);

	if (uiFileSize < eterpack::c_HeaderSize + sizeof(TEterPackIndex) * m_indexCount)
	{
		TraceError("Pack index file size error! %s, indexCount %d", m_dbName, m_indexCount);
		return false;
	}

	//Tracef("Loading Pack file %s elements: %d ... ", m_dbName, m_indexCount);

	m_indexData.resize(m_indexCount);
	memcpy(m_indexData.data(), pbData, sizeof(TEterPackIndex) * m_indexCount);

	TEterPackIndex * index = m_indexData.data();

	for (int i = 0; i < m_indexCount; ++i, ++index)
	{
		if (!index->filename_crc)
			PushFreeIndex(index);
		else
		{
			if (index->real_data_size > index->data_size)
				m_FragmentSize += index->real_data_size - index->data_size;

			m_DataPositionMap.emplace(index->filename_crc, index);

			if (bOverwrite) // ���� ���� ��ŷ ������ ���߿� �������� �ֻ����� ����ؾ��Ѵ�
				rkFileDict.UpdateItem(this, index);
			else
				rkFileDict.InsertItem(this, index);
		}
	}

	//Tracef("Done. (spent %dms)\n", ELTimer_GetMSec() - dwBeginTime);
	return true;
}
//
//void CEterPack::UpdateLastAccessTime()
//{
//	m_tLastAccessTime = time(nullptr);
//}
//
//void CEterPack::ClearDataMemoryMap()
//{
//	// m_file�� data file�̴�...
//	m_file.Destroy();
//	m_tLastAccessTime = 0;
//	m_bIsDataLoaded = false;
//}

bool CEterPack::Get(CMappedFile & out_file, const char * filename, LPCVOID * data)
{
	TEterPackIndex * index = FindIndex(filename);

	if (!index)
		return false;

#ifdef USE_PACK_TYPE_DIO_ONLY
	if (index->compressed_type != COMPRESSED_TYPE_DIO_SECURITY)
		return false;
#endif

	//UpdateLastAccessTime();
	//if (!m_bIsDataLoaded)
	//{
	//	if (!m_file.Create(m_stDataFileName.c_str(), (const void**)&m_file_data, 0, 0))
	//		return false;
	//
	//	m_bIsDataLoaded = true;
	//}

	// �������� CEterPack���� epk�� memory map�� �÷�����, ��û�� ���� �� �κ��� ��ũ�ؼ� �Ѱ� ����µ�,
	// ������ ��û�� ����, �ʿ��� �κи� memory map�� �ø���, ��û�� ������ �����ϰ� ��.
	out_file.Create(m_stDataFileName.c_str(), data, index->data_position, index->data_size);

	bool bIsSecurityCheckRequired =
		(index->compressed_type == COMPRESSED_TYPE_SECURITY ||
#ifdef ENABLE_PACK_TYPE_DIO
			index->compressed_type == COMPRESSED_TYPE_DIO_SECURITY ||
#endif
			index->compressed_type == COMPRESSED_TYPE_PANAMA);


	if (bIsSecurityCheckRequired)
	{
#ifdef ENABLE_CRC32_CHECK
		uint32_t dwCrc32 = GetCRC32(static_cast<const char *>(*data), index->data_size);

		if (index->data_crc != dwCrc32)
			return false;
#endif
	}

	if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
	{
		auto * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data)))
		{
			TraceError("Failed to decompress : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
	{
		auto * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data), s_adwEterPackSecurityKey))
		{
			TraceError("Failed to encrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_PANAMA == index->compressed_type)
	{
		auto * zObj = new CLZObject;
		__Decrypt_Panama(filename, static_cast<const uint8_t *>(*data), index->data_size, *zObj);
		out_file.BindLZObjectWithBufferedSize(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_HYBRIDCRYPT == index->compressed_type || COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB == index->compressed_type)
	{
		auto * zObj = new CLZObject;

		auto ssvv = std::string(filename);
		if (!m_pCSHybridCryptPolicy->DecryptMemory(ssvv, static_cast<const uint8_t*>(*data), index->data_size, *zObj))
		{
			delete zObj;
			return false;
		}

		out_file.BindLZObjectWithBufferedSize(zObj);
		if (COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB == index->compressed_type)
		{
			uint8_t * pSDBData;
			int iSDBSize;

			if (!m_pCSHybridCryptPolicy->GetSupplementaryDataBlock(ssvv, pSDBData, iSDBSize))
				return false;

			*data = out_file.AppendDataBlock(pSDBData, iSDBSize);
		}
		else
			*data = zObj->GetBuffer();
	}
#ifdef ENABLE_PACK_TYPE_DIO
	else if (COMPRESSED_TYPE_DIO_SECURITY == index->compressed_type)
	{
		CLZObject* zObj = new CLZObject;
		if (!CLZO::Instance().DecompressDio(*zObj, static_cast<const uint8_t*>(*data)))
		{
			TraceError("Failed to decrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
#endif

	return true;
}

bool CEterPack::Get2(CMappedFile & out_file, const char * filename, TEterPackIndex * index, LPCVOID * data) const
{
	if (!index)
		return false;

#ifdef USE_PACK_TYPE_DIO_ONLY
	if (index->compressed_type != COMPRESSED_TYPE_DIO_SECURITY)
		return false;
#endif

	//UpdateLastAccessTime();
	//if (!m_bIsDataLoaded)
	//{
	//	if (!m_file.Create(m_stDataFileName.c_str(), (const void**)&m_file_data, 0, 0))
	//		return false;
	//
	//	m_bIsDataLoaded = true;
	//}
	out_file.Create(m_stDataFileName.c_str(), data, index->data_position, index->data_size);

	bool bIsSecurityCheckRequired =
		(index->compressed_type == COMPRESSED_TYPE_SECURITY ||
#ifdef ENABLE_PACK_TYPE_DIO
			index->compressed_type == COMPRESSED_TYPE_DIO_SECURITY ||
#endif
			index->compressed_type == COMPRESSED_TYPE_PANAMA);


	if (bIsSecurityCheckRequired)
	{
#ifdef ENABLE_CRC32_CHECK
		uint32_t dwCrc32 = GetCRC32(static_cast<const char *>(*data), index->data_size);

		if (index->data_crc != dwCrc32)
			return false;
#endif
	}

	if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
	{
		auto * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data)))
		{
			TraceError("Failed to decompress : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
	{
		auto * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const uint8_t *>(*data), s_adwEterPackSecurityKey))
		{
			TraceError("Failed to encrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_PANAMA == index->compressed_type)
	{
		auto * zObj = new CLZObject;
		__Decrypt_Panama(filename, static_cast<const uint8_t *>(*data), index->data_size, *zObj);
		out_file.BindLZObjectWithBufferedSize(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_HYBRIDCRYPT == index->compressed_type || COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB == index->compressed_type)
	{
		auto * zObj = new CLZObject;

		auto ssvv = std::string(filename);
		if (!m_pCSHybridCryptPolicy->DecryptMemory(ssvv, static_cast<const uint8_t*>(*data), index->data_size, *zObj))
		{
			delete zObj;
			return false;
		}

		out_file.BindLZObjectWithBufferedSize(zObj);

		if (COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB == index->compressed_type)
		{
			uint8_t * pSDBData;
			int iSDBSize;

			if (!m_pCSHybridCryptPolicy->GetSupplementaryDataBlock(ssvv, pSDBData, iSDBSize))
				return false;
			*data = out_file.AppendDataBlock(pSDBData, iSDBSize);
		}
		else
			*data = zObj->GetBuffer();
	}
#ifdef ENABLE_PACK_TYPE_DIO
	else if (COMPRESSED_TYPE_DIO_SECURITY == index->compressed_type)
	{
		CLZObject* zObj = new CLZObject;
		if (!CLZO::Instance().DecompressDio(*zObj, static_cast<const uint8_t*>(*data)))
		{
			TraceError("Failed to decrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
#endif

	return true;
}


bool CEterPack::Delete(TEterPackIndex * pIndex)
{
	CFileBase fileIndex;

	if (!fileIndex.Create(m_indexFileName, CFileBase::FILEMODE_WRITE))
		return false;

	PushFreeIndex(pIndex);
	WriteIndex(fileIndex, pIndex);
	return true;
}

bool CEterPack::Delete(const char * filename)
{
	TEterPackIndex * pIndex = FindIndex(filename);

	if (!pIndex)
		return false;

	return Delete(pIndex);
}

bool CEterPack::Extract()
{
#ifdef ENABLE_PACK_TYPE_DIO
	return true;
#endif

	CMappedFile dataMapFile;
	LPCVOID data;

	if (!dataMapFile.Create(m_stDataFileName.c_str(), &data, 0, 0))
		return false;

	CLZObject zObj;

	for (auto & i : m_DataPositionMap)
	{
		TEterPackIndex * index = i.second;
		CFileBase writeFile;

		inlinePathCreate(index->filename);
		printf("%s\n", index->filename);

		writeFile.Create(index->filename, CFileBase::FILEMODE_WRITE);

		if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
		{
			if (!CLZO::Instance().Decompress(zObj, static_cast<const uint8_t *>(data) + index->data_position))
			{
				printf("cannot decompress");
				return false;
			}

			writeFile.Write(zObj.GetBuffer(), zObj.GetSize());
			zObj.Clear();
		}
		else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
		{
			if (!CLZO::Instance().Decompress(zObj, static_cast<const uint8_t *>(data) + index->data_position, s_adwEterPackSecurityKey))
			{
				printf("cannot decompress");
				return false;
			}

			writeFile.Write(zObj.GetBuffer(), zObj.GetSize());
			zObj.Clear();
		}
		else if (COMPRESSED_TYPE_PANAMA == index->compressed_type)
		{
			__Decrypt_Panama(index->filename, static_cast<const uint8_t *>(data) + index->data_position, index->data_size, zObj);
			writeFile.Write(zObj.GetBuffer(), zObj.GetBufferSize());
			zObj.Clear();
		}
		else if (COMPRESSED_TYPE_HYBRIDCRYPT == index->compressed_type || COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB == index->compressed_type)
		{
			auto ssvv = std::string(index->filename);
			if (!m_pCSHybridCryptPolicy->DecryptMemory(ssvv, (const uint8_t*)data + index->data_position, index->data_size, zObj))
				return false;

			if (COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB == index->compressed_type)
			{
				dataMapFile.BindLZObjectWithBufferedSize(&zObj);

				uint8_t * pSDBData;
				int iSDBSize;

				if (!m_pCSHybridCryptPolicy->GetSupplementaryDataBlock(ssvv, pSDBData, iSDBSize))
					return false;

				dataMapFile.AppendDataBlock(pSDBData, iSDBSize);
				writeFile.Write(dataMapFile.AppendDataBlock(pSDBData, iSDBSize), dataMapFile.Size());
			}
			else
				writeFile.Write(zObj.GetBuffer(), zObj.GetBufferSize());
			zObj.Clear();
		}

		else if (COMPRESSED_TYPE_NONE == index->compressed_type)
			writeFile.Write(static_cast<const char *>(data) + index->data_position, index->data_size);

		writeFile.Destroy();
	}
	return true;
}

bool CEterPack::GetNames(std::vector<std::string> * retNames)
{
	CMappedFile dataMapFile;
	LPCVOID data;

	if (!dataMapFile.Create(m_stDataFileName.c_str(), &data, 0, 0))
		return false;

	CLZObject zObj;

	for (auto & i : m_DataPositionMap)
	{
		TEterPackIndex * index = i.second;

		inlinePathCreate(index->filename);

		retNames->emplace_back(index->filename);
	}
	return true;
}

bool CEterPack::Put(const char * filename, const char * sourceFilename, uint8_t packType, const std::string & strRelateMapName)
{
	CMappedFile mapFile;
	LPCVOID data;

	if (sourceFilename)
	{
		if (!mapFile.Create(sourceFilename, &data, 0, 0))
			return false;
	}
	else if (!mapFile.Create(filename, &data, 0, 0))
		return false;

	auto * pMappedData = (uint8_t *) data;
	int iMappedDataSize = mapFile.Size();

	if (packType == COMPRESSED_TYPE_HYBRIDCRYPT || packType == COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB)
	{
		auto ssvvv = std::string(filename);
		m_pCSHybridCryptPolicy->GenerateCryptKey(ssvvv);

		if (packType == COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB)
		{
			if (!m_pCSHybridCryptPolicy->GenerateSupplementaryDataBlock(ssvvv, strRelateMapName,
																		static_cast<const uint8_t *>(data), mapFile.Size(), pMappedData,
																		iMappedDataSize))
				return false;
		}
	}

	return Put(filename, pMappedData, iMappedDataSize, packType);
}

bool CEterPack::Put(const char * filename, LPCVOID data, long len, uint8_t packType)
{
	if (m_bEncrypted)
	{
		TraceError("EterPack::Put : Cannot put to encrypted pack (filename: %s, DB: %s)", filename, m_dbName);
		return false;
	}

	CFileBase fileIndex;

	if (!fileIndex.Create(m_indexFileName, CFileBase::FILEMODE_WRITE))
		return false;

	CFileBase fileData;

	if (!fileData.Create(m_stDataFileName.c_str(), CFileBase::FILEMODE_WRITE))
		return false;

	TEterPackIndex * pIndex = FindIndex(filename);

	CLZObject zObj;
	std::string encryptStr;

	if (packType == COMPRESSED_TYPE_SECURITY || packType == COMPRESSED_TYPE_COMPRESS)
	{
		if (packType == COMPRESSED_TYPE_SECURITY)
		{
			if (!CLZO::Instance().CompressEncryptedMemory(zObj, data, len, s_adwEterPackSecurityKey))
				return false;
		}
		else
		{
			if (!CLZO::Instance().CompressMemory(zObj, data, len))
				return false;
		}

		data = zObj.GetBuffer();
		len = zObj.GetSize();
	}
	else if (packType == COMPRESSED_TYPE_PANAMA)
	{
		if (!__Encrypt_Panama(filename, static_cast<const uint8_t *>(data), len, zObj))
			return false;

		data = zObj.GetBuffer();
		len = zObj.GetBufferSize();
	}
	else if (packType == COMPRESSED_TYPE_HYBRIDCRYPT || packType == COMPRESSED_TYPE_HYBRIDCRYPT_WITHSDB)
	{
		auto ssvv = std::string(filename);
		if (!m_pCSHybridCryptPolicy->EncryptMemory(ssvv, (const uint8_t*)data, len, zObj))
			return false;

		data = zObj.GetBuffer();
		len = zObj.GetBufferSize();
	}


	uint32_t data_crc = GetCRC32(static_cast<const char *>(data), len);

	// ���� �����Ͱ� ������..
	if (pIndex)
	{
		// ���� data ũ�Ⱑ ���� ������ ũ�⸦ ������ �� �ִٸ�
		if (pIndex->real_data_size >= len)
		{
			++m_map_indexRefCount[pIndex->id];

			// ���̰� Ʋ���ų�, checksum�� Ʋ�� ���� ���� �Ѵ�.
			if ((pIndex->data_size != len) || (pIndex->data_crc != data_crc))
			{
				pIndex->data_size = len;
				pIndex->data_crc = data_crc;

				pIndex->compressed_type = packType;

				CMakePackLog::GetSingleton().Writef("Overwrite[type:%u] %s\n", pIndex->compressed_type, pIndex->filename);

				WriteIndex(fileIndex, pIndex);
				WriteData(fileData, pIndex, data);
			}

			return true;
		}

		// ���� ������ ũ�Ⱑ ���� �� �� ���� ���ٸ�, ���� �ε����� �Ҵ���
		// �־�� �Ѵ�. ���� �ִ� �ε����� ��� �д�.
		PushFreeIndex(pIndex);
		WriteIndex(fileIndex, pIndex);
	}

	// �� ������
	pIndex = NewIndex(fileIndex, filename, len);
	pIndex->data_size = len;

	pIndex->data_crc = data_crc;


	pIndex->data_position = GetNewDataPosition(fileData);
	pIndex->compressed_type = packType;

	WriteIndex(fileIndex, pIndex);
	WriteNewData(fileData, pIndex, data);

	++m_map_indexRefCount[pIndex->id];

	CMakePackLog::GetSingleton().Writef("Write[type:%u] %s\n", pIndex->compressed_type, pIndex->filename);

	return true;
}

long CEterPack::GetFragmentSize() const
{
	return m_FragmentSize;
}

// Private methods
bool CEterPack::CreateIndexFile() const
{
	msl::file_ptr fPtr(m_indexFileName, "rb");
	if (fPtr) // close and exit if exists
		return true;

	if (m_bReadOnly)
		return false;

	//
	// ������ �����Ƿ� ���� �����.
	//
	fPtr.open(m_indexFileName, "wb");
	if (!fPtr)
		return false;

	fwrite(&eterpack::c_IndexCC, sizeof(uint32_t), 1, fPtr.get());
	fwrite(&eterpack::c_Version, sizeof(uint32_t), 1, fPtr.get());
	fwrite(&m_indexCount, sizeof(long), 1, fPtr.get());
	return true;
}


void CEterPack::WriteIndex(CFileBase & file, TEterPackIndex * index) const
{
	file.Seek(sizeof(uint32_t) + sizeof(uint32_t));
	file.Write(&m_indexCount, sizeof(long));
	file.Seek(eterpack::c_HeaderSize + (index->id * sizeof(TEterPackIndex)));

	if (!file.Write(index, sizeof(TEterPackIndex)))
		assert(!"WriteIndex: fwrite failed");
}

/*
 *	Free Block �̶� �����Ϳ��� ������ �κ��� ���Ѵ�.
 *	Free Block ���� ���� FREE_INDEX_BLOCK_SIZE (32768) ������ ��������
 *	����Ʈ�� �����ȴ�.
 *
 *	���� ��� 128k �� �����ʹ�
 *	128 * 1024 / FREE_INDEX_BLOCK_SIZE = 4 �̹Ƿ�
 *	���� �����δ� m_FreeIndexList[4] �� ����.
 *
 *	FREE_INDEX_BLOCK_SIZE �� �ִ� ���� FREE_INDEX_MAX_SIZE(512) �̴�.
 *	���� 16MB �̻��� �����ʹ� ������ �迭�� 512 ��ġ�� ����.
 */
int CEterPack::GetFreeBlockIndex(long size)
{
	return std::min<int>(FREE_INDEX_MAX_SIZE, size / FREE_INDEX_BLOCK_SIZE);
}

void CEterPack::PushFreeIndex(TEterPackIndex * index)
{
	if (index->filename_crc != 0)
	{
		if (auto i = m_DataPositionMap.find(index->filename_crc); i != m_DataPositionMap.end())
			m_DataPositionMap.erase(i);

		index->filename_crc = 0;
		msl::refill(index->filename);
	}

	int blockidx = GetFreeBlockIndex(index->real_data_size);
	m_FreeIndexList[blockidx].emplace_back(index);
	m_FragmentSize += index->real_data_size;
}

long CEterPack::GetNewIndexPosition(CFileBase & file)
{
	long pos = (file.Size() - eterpack::c_HeaderSize) / sizeof(TEterPackIndex);
	++m_indexCount;
	return (pos);
}

TEterPackIndex * CEterPack::NewIndex(CFileBase & file, const char * filename, long size)
{
	TEterPackIndex * index = nullptr;
	int block_size = size + (DATA_BLOCK_SIZE - (size % DATA_BLOCK_SIZE));

	int blockidx = GetFreeBlockIndex(block_size);

	for (auto i = m_FreeIndexList[blockidx].begin(); i != m_FreeIndexList[blockidx].end(); ++i)
	{
		if ((*i)->real_data_size >= size)
		{
			index = *i;
			m_FreeIndexList[blockidx].erase(i);

			assert(index->filename_crc == 0);
			break;
		}
	}

	if (!index)
	{
		index = new TEterPackIndex;
		index->real_data_size = block_size;
		index->id = GetNewIndexPosition(file);
	}

	strncpy(index->filename, filename, FILENAME_MAX_LEN);
	index->filename[FILENAME_MAX_LEN] = '\0';
	inlineConvertPackFilename(index->filename);

	index->filename_crc = GetCRC32(index->filename, strlen(index->filename));

	m_DataPositionMap.emplace(index->filename_crc, index);
	return index;
}

TEterPackIndex * CEterPack::FindIndex(const char * filename)
{
	static char tmpFilename[MAX_PATH + 1];
	strncpy(tmpFilename, filename, MAX_PATH);
	inlineConvertPackFilename(tmpFilename);

	uint32_t filename_crc = GetCRC32(tmpFilename, strlen(tmpFilename));


	if (auto i = m_DataPositionMap.find(filename_crc); i != m_DataPositionMap.end())
		return (i->second);

	return nullptr;
}

bool CEterPack::IsExist(const char * filename)
{
	return FindIndex(filename) != nullptr;
}

bool CEterPack::CreateDataFile() const
{
	msl::file_ptr fPtr(m_stDataFileName.c_str(), "rb");
	if (fPtr) // close and exit if exists
		return true;

	if (m_bReadOnly)
		return false;

	fPtr.open(m_stDataFileName.c_str(), "wb");
	if (!fPtr)
		return false;

	return true;
}

long CEterPack::GetNewDataPosition(CFileBase & file)
{
	return file.Size();
}

bool CEterPack::ReadData(CFileBase & file, TEterPackIndex * index, LPVOID data, long maxsize)
{
	if (index->data_size > maxsize)
		return false;

	file.Seek(index->data_position);
	file.Read(data, index->data_size);
	return true;
}

bool CEterPack::WriteData(CFileBase & file, TEterPackIndex * index, LPCVOID data)
{
	file.Seek(index->data_position);

	if (!file.Write(data, index->data_size))
	{
		assert(!"WriteData: fwrite data failed");
		return false;
	}

	return true;
}

bool CEterPack::WriteNewData(CFileBase & file, TEterPackIndex * index, LPCVOID data)
{
	file.Seek(index->data_position);

	if (!file.Write(data, index->data_size))
	{
		assert(!"WriteData: fwrite data failed");
		return false;
	}

	const int empty_size = index->real_data_size - index->data_size;

	if (empty_size < 0)
	{
		printf("SYSERR: WriteNewData(): CRITICAL ERROR: empty_size lower than 0!\n");
		exit(1);
	}

	if (empty_size == 0)
		return true;

	std::vector<char> empty_buf(empty_size);
	if (!file.Write(empty_buf.data(), empty_size))
	{
		assert(!"WriteData: fwrite empty data failed");
		return false;
	}
	return true;
}

TDataPositionMap & CEterPack::GetIndexMap()
{
	return m_DataPositionMap;
}

uint32_t CEterPack::DeleteUnreferencedData()
{
	TDataPositionMap::iterator i = m_DataPositionMap.begin();
	uint32_t dwCount = 0;

	while (i != m_DataPositionMap.end())
	{
		TEterPackIndex * pIndex = (i++)->second;

		if (0 == m_map_indexRefCount[pIndex->id])
		{
			printf("Unref File %s\n", pIndex->filename);
			Delete(pIndex);
			++dwCount;
		}
	}

	return dwCount;
}

const char * CEterPack::GetDBName() const
{
	return m_dbName;
}

void CEterPack::__CreateFileNameKey_Panama(const char * filename, uint8_t * key, unsigned int keySize)
{
	// Ű ��ȣȭ
	if (keySize != 32)
		return;

	std::string SrcStringForKey(filename);
	unsigned int idx = GetCRC32(SrcStringForKey.c_str(), SrcStringForKey.length()) & 3;

	CryptoPP::HashTransformation * hm1 = nullptr;
	CryptoPP::HashTransformation * hm2 = nullptr;

	static CryptoPP::Tiger tiger;
	static CryptoPP::SHA1 sha1;
	static CryptoPP::RIPEMD128 ripemd128;
	static CryptoPP::Whirlpool whirlpool;

	switch (idx & 3)
	{
	case 0:
		hm1 = &whirlpool;
		break;

	case 1:
		hm1 = &tiger;
		break;

	case 2:
		hm1 = &sha1;
		break;

	case 3:
		hm1 = &ripemd128;
		break;
	}

	CryptoPP::StringSource give_me_a_name(SrcStringForKey, true,
										  new CryptoPP::HashFilter(*hm1,
																   //new CryptoPP::HexEncoder(
																   new CryptoPP::ArraySink(key, 16)
																   //) // HexEncoder
																   ) // HashFilter
	); // StringSource

	// ������� Ű�� ù��° 4����Ʈ�� ���� 16����Ʈ Ű ���� �˰��� ����
	unsigned int idx2 = *reinterpret_cast<unsigned int *>(key);

	switch (idx2 & 3)
	{
	case 0:
		hm2 = &sha1;
		break;

	case 1:
		hm2 = &ripemd128;
		break;

	case 2:
		hm2 = &whirlpool;
		break;

	case 3:
		hm2 = &tiger;
		break;
	}

	CryptoPP::StringSource(SrcStringForKey, true,
						   new CryptoPP::HashFilter(*hm2,
													//new CryptoPP::HexEncoder(
													new CryptoPP::ArraySink(key + 16, 16)
													//) // HexEncoder
													) // HashFilter
	); // StringSource
	// Ű ���� �Ϸ�
}

bool CEterPack::__Encrypt_Panama(const char * filename, const uint8_t * data, SIZE_T dataSize, CLZObject & zObj) const
{
	if (32 != m_stIV_Panama.length())
	{
		// ��Ŀ�� �� �޼����� ���� ��Ʈ�� ������� ����׿����� ���
#ifdef _DEBUG
		TraceError("IV not set (filename: %s)", filename);
#endif
		return false;
	}

	CryptoPP::PanamaCipher<CryptoPP::LittleEndian>::Encryption Encryptor;

	if (dataSize < Encryptor.MandatoryBlockSize())
	{
#ifdef _DEBUG
		TraceError("Type 3 pack file must be bigger than %u bytes (filename: %s)", Encryptor.MandatoryBlockSize(), filename);
#endif
		return false;
	}

	uint8_t key[32];

	__CreateFileNameKey_Panama(filename, key, sizeof(key));
	Encryptor.SetKeyWithIV(key, sizeof(key), reinterpret_cast<const uint8_t *>(m_stIV_Panama.c_str()), 32);

	// MandatoryBlockSize�� ������ �������� ����� �ִ� 2048 ����Ʈ��
	uint32_t cryptSize = dataSize - (dataSize % Encryptor.MandatoryBlockSize());
	cryptSize = cryptSize > 2048 ? 2048 : cryptSize;

	std::string tmp;

	tmp.reserve(cryptSize);

	CryptoPP::ArraySource(data, cryptSize, true, new CryptoPP::StreamTransformationFilter(Encryptor, new CryptoPP::StringSink(tmp)));

	if (tmp.length() != cryptSize)
	{
#ifdef _DEBUG
		TraceError("Type 3 pack crypt buffer size error (out %u should be %u)", tmp.length(), cryptSize);
#endif
		return false;
	}

	zObj.AllocBuffer(dataSize);
	memcpy(zObj.GetBuffer(), tmp.c_str(), cryptSize);

	if (dataSize - cryptSize > 0)
		memcpy(zObj.GetBuffer() + cryptSize, data + cryptSize, dataSize - cryptSize);

	return true;
}

bool CEterPack::__Decrypt_Panama(const char * filename, const uint8_t * data, SIZE_T dataSize, CLZObject & zObj) const
{
	if (32 != m_stIV_Panama.length())
	{
		// ��Ŀ�� �� �޼����� ���� ��Ʈ�� ������� ����׿����� ���
#ifdef _DEBUG
		TraceError("IV not set (filename: %s)", filename);
#endif
		return false;
	}

	CryptoPP::PanamaCipher<CryptoPP::LittleEndian>::Decryption Decryptor;

	uint8_t key[32];

	__CreateFileNameKey_Panama(filename, key, sizeof(key));
	Decryptor.SetKeyWithIV(key, sizeof(key), reinterpret_cast<const uint8_t *>(m_stIV_Panama.c_str()), 32);
	// MandatoryBlockSize�� ������ �������� ����� �ִ� 2048 ����Ʈ��
	uint32_t cryptSize = dataSize - (dataSize % Decryptor.MandatoryBlockSize());
	cryptSize = cryptSize > 2048 ? 2048 : cryptSize;

	std::string tmp;

	tmp.reserve(cryptSize);

	CryptoPP::ArraySource(data, cryptSize, true, new CryptoPP::StreamTransformationFilter(Decryptor, new CryptoPP::StringSink(tmp)));

	if (tmp.length() != cryptSize)
	{
#ifdef _DEBUG
		TraceError("Type 3 pack crypt buffer size error (out %u should be %u)", tmp.length(), cryptSize);
#endif
		return false;
	}

	zObj.AllocBuffer(dataSize);
	memcpy(zObj.GetBuffer(), tmp.c_str(), cryptSize);

	if (dataSize - cryptSize > 0)
		memcpy(zObj.GetBuffer() + cryptSize, data + cryptSize, dataSize - cryptSize);

	return true;
}

EterPackPolicy_CSHybridCrypt * CEterPack::GetPackPolicy_HybridCrypt() const
{
	return m_pCSHybridCryptPolicy.get();
}


/////////////////////////

void CEterFileDict::InsertItem(CEterPack * pkPack, TEterPackIndex * pkInfo)
{
	Item item;

	item.pkPack = pkPack;
	item.pkInfo = pkInfo;

	m_dict.emplace(pkInfo->filename_crc, item);
}

void CEterFileDict::UpdateItem(CEterPack * pkPack, TEterPackIndex * pkInfo)
{
	Item item;

	item.pkPack = pkPack;
	item.pkInfo = pkInfo;


	if (auto f = m_dict.find(pkInfo->filename_crc); f != m_dict.end())
	{
		if (strcmp(f->second.pkInfo->filename, item.pkInfo->filename) == 0)
			f->second = item;
		else
			TraceError("NAME_COLLISION: OLD: %s NEW: %s", f->second.pkInfo->filename, item.pkInfo->filename);
	}
	else
		m_dict.emplace(pkInfo->filename_crc, item);
}

CEterFileDict::Item * CEterFileDict::GetItem(uint32_t dwFileNameHash, const char * c_pszFileName)
{
	auto iter_pair = m_dict.equal_range(dwFileNameHash);
	auto iter = iter_pair.first;

	while (iter != iter_pair.second)
	{
		Item & item = iter->second;

		if (0 == strcmp(c_pszFileName, item.pkInfo->filename))
			return &item;

		++iter;
	}

	return nullptr;
}
