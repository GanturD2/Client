#pragma once

#include "../eterPack/EterPack.h"

class CPropertyManager : public CSingleton<CPropertyManager>
{
public:
	CPropertyManager();
	virtual ~CPropertyManager();

	void Clear();

	void SetPack(CEterPack * pPack);
	bool BuildPack();

	bool LoadReservedCRC(const char * c_pszFileName);
	void ReserveCRC(uint32_t dwCRC);
	uint32_t GetUniqueCRC(const char * c_szSeed);

	bool Initialize(const char * c_pszPackFileName = nullptr);
	bool Register(const char * c_pszFileName, CProperty ** ppProperty = nullptr);

	bool Get(uint32_t dwCRC, CProperty ** ppProperty);
	bool Get(const char * c_pszFileName, CProperty ** ppProperty);

	//		bool			Add(const char * c_pszFileName);
	//		bool			Remove(uint32_t dwCRC);

	bool Put(const char * c_pszFileName, const char * c_pszSourceFileName);

	bool Erase(uint32_t dwCRC);
	bool Erase(const char * c_pszFileName);

protected:
	typedef std::map<uint32_t, CProperty *> TPropertyCRCMap;
	typedef std::set<uint32_t> TCRCSet;

	bool m_isFileMode;
	TPropertyCRCMap m_PropertyByCRCMap;
	TCRCSet m_ReservedCRCSet;
	CEterPack m_pack;
	CEterFileDict m_fileDict;
};
