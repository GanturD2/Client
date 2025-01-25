#pragma once

#include "RaceData.h"

class CRaceManager : public CSingleton<CRaceManager>
{
public:
	typedef std::map<uint32_t, CRaceData *> TRaceDataMap;
	typedef TRaceDataMap::iterator TRaceDataIterator;

public:
	CRaceManager();
	virtual ~CRaceManager();

	void Create();
	void Destroy();

	void RegisterRaceName(uint32_t dwRaceIndex, const char * c_szName);
	void RegisterRaceSrcName(const char * c_szName, const char * c_szSrcName);

	void SetPathName(const char * c_szPathName);
	const char * GetFullPathFileName(const char * c_szFileName);

	// Handling
	void CreateRace(uint32_t dwRaceIndex);
	void SelectRace(uint32_t dwRaceIndex);
	CRaceData * GetSelectedRaceDataPointer();
	// Handling

#ifdef ENABLE_INGAME_WIKI
	BOOL GetRaceDataPointer(uint32_t dwRaceIndex, CRaceData** ppRaceData, bool printTrace = true);
#else
	BOOL GetRaceDataPointer(uint32_t dwRaceIndex, CRaceData ** ppRaceData);
#endif
	void SetRaceHeight(int iVnum, float fHeigt);
	float GetRaceHeight(int iVnum);

protected:
	CRaceData * __LoadRaceData(uint32_t dwRaceIndex);
	bool __LoadRaceMotionList(CRaceData & rkRaceData, const char * pathName, const char * motionListFileName);

	void __Initialize();
	void __DestroyRaceDataMap();

protected:
	TRaceDataMap m_RaceDataMap;

	std::map<std::string, std::string> m_kMap_stRaceName_stSrcName;
	std::map<uint32_t, std::string> m_kMap_dwRaceKey_stRaceName;
	std::map<int, float>	m_kMap_iRaceKey_fRaceAdditionalHeight;

private:
	std::string m_strPathName;
	CRaceData * m_pSelectedRaceData;
};