#ifndef __INC_ETERPACKLIB_ETERPACKPOLICY_CSHYBRIDCRYPT_H__
#define __INC_ETERPACKLIB_ETERPACKPOLICY_CSHYBRIDCRYPT_H__

#include <unordered_map>
#include <cryptopp/cryptlib.h>
#include <cryptopp/camellia.h>
#include <cryptopp/twofish.h>
#include <cryptopp/tea.h>

enum eHybridCipherAlgorithm
{
	e_Cipher_Camellia,
	e_Cipher_Twofish,
	e_Cipher_XTEA,
	Num_Of_Ciphers
};


class CFileBase;
class CLZObject;

class EterPackPolicy_CSHybridCrypt
{
public:
	~EterPackPolicy_CSHybridCrypt();

	bool GenerateCryptKey(std::string & rfileName);
	bool EncryptMemory(std::string & rfileName, IN const uint8_t * pSrcData, IN int iSrcLen, OUT CLZObject & zObj);
	bool DecryptMemory(std::string & rfilename, IN const uint8_t * pEncryptedData, IN int iEncryptedLen, OUT CLZObject & zObj);
	bool IsContainingCryptKey() const;

	//Supplementary Data Block (SDB)
	bool GenerateSupplementaryDataBlock(std::string & rfilename, const std::string & strMapName, IN const uint8_t * pSrcData,
										IN int iSrcLen, OUT LPBYTE & pDestData, OUT int & iDestLen);
	bool GetSupplementaryDataBlock(std::string & rfilename, OUT LPBYTE & pSDB, OUT int & iSDBSize);
	bool IsContainingSDBFile() const;

	// Read/Write IO
	void WriteCryptKeyToFile(CFileBase & rFile);
	int ReadCryptKeyInfoFromStream(IN const uint8_t * pStream);

	void WriteSupplementaryDataBlockToFile(CFileBase & rFile);
	int ReadSupplementatyDataBlockFromStream(IN const uint8_t * pStream);

protected:
	typedef union UEncryptKey {
		uint8_t key[16];
		uint8_t keyCamellia[CryptoPP::Camellia::DEFAULT_KEYLENGTH];
		uint8_t keyTwofish[CryptoPP::Twofish::DEFAULT_KEYLENGTH];
		uint8_t keyXTEA[CryptoPP::XTEA::DEFAULT_KEYLENGTH];

	} TEncryptKey;

	typedef union UEncryptIV {
		uint8_t iv[16];
		uint8_t ivCamellia[CryptoPP::Camellia::BLOCKSIZE];
		uint8_t ivTwofish[CryptoPP::Twofish::BLOCKSIZE];
		uint8_t ivXTEA[CryptoPP::XTEA::BLOCKSIZE];

	} TEncryptIV;


	typedef struct SCSHybridCryptKey
	{
		TEncryptKey uEncryptKey;
		TEncryptIV uEncryptIV;

	} TCSHybridCryptKey;

	typedef std::unordered_map<uint32_t, TCSHybridCryptKey> TCSHybridCryptKeyMap;
	TCSHybridCryptKeyMap m_mapHybridCryptKey;

	typedef struct SSupplementaryDataBlockInfo
	{
		std::string strRelatedMapName;
		std::vector<uint8_t> vecStream;

	} TSupplementaryDataBlockInfo;

	typedef std::unordered_map<uint32_t, TSupplementaryDataBlockInfo> TSupplementaryDataBlockMap; //key filename hash
	TSupplementaryDataBlockMap m_mapSDBMap;

private:
	bool GetPerFileCryptKey(std::string & rfileName, eHybridCipherAlgorithm & eAlgorithm, TEncryptKey & key, TEncryptIV & iv);
};


#endif // __INC_ETERPACKLIB_ETERPACKPOLICY_CSHYBRIDCRYPT_H__
