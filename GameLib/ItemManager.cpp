#include "StdAfx.h"
#include "../EterPack/EterPackManager.h"
#include "../EterLib/ResourceManager.h"

#include "ItemManager.h"

static uint32_t s_adwItemProtoKey[4] = {173217, 72619434, 408587239, 27973291};

BOOL CItemManager::SelectItemData(uint32_t dwIndex)
{
	auto f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const CItemData::TItemTable * pTable = p->GetTable();
			if ((pTable->dwVnum < dwIndex) && dwIndex < (pTable->dwVnum + pTable->dwVnumRange))
			{
				m_pSelectedItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::SelectItemData - FIND ERROR [%d]\n", dwIndex);
		return FALSE;
	}

	m_pSelectedItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::GetSelectedItemDataPointer()
{
	return m_pSelectedItemData;
}

BOOL CItemManager::GetItemDataPointer(uint32_t dwItemID, CItemData ** ppItemData)
{
	if (0 == dwItemID)
		return FALSE;

	auto f = m_ItemMap.find(dwItemID);

	if (m_ItemMap.end() == f)
	{
		int n = m_vec_ItemRange.size();
		for (int i = 0; i < n; i++)
		{
			CItemData * p = m_vec_ItemRange[i];
			const CItemData::TItemTable * pTable = p->GetTable();
			if ((pTable->dwVnum < dwItemID) && dwItemID < (pTable->dwVnum + pTable->dwVnumRange))
			{
				*ppItemData = p;
				return TRUE;
			}
		}
		Tracef(" CItemManager::GetItemDataPointer - FIND ERROR [%d]\n", dwItemID);
		return FALSE;
	}

	*ppItemData = f->second;

	return TRUE;
}

CItemData * CItemManager::MakeItemData(uint32_t dwIndex)
{
	auto f = m_ItemMap.find(dwIndex);

	if (m_ItemMap.end() == f)
	{
		CItemData * pItemData = CItemData::New();

		m_ItemMap.emplace(dwIndex, pItemData);

		return pItemData;
	}

	return f->second;
}

#ifdef ENABLE_SCALE_SYSTEM
bool CItemManager::LoadItemScale(const char* szItemScale)
{
	CMappedFile File;
	LPCVOID pData;
	if (!CEterPackManager::Instance().Get(File, szItemScale, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 6 || TokenVector.size() == 7))
		{
			TraceError(" CItemManager::LoadItemScale(%s) - Error on line %d\n", szItemScale, i);
			continue;
		}

		const std::string& c_rstrID = TokenVector[ITEM_SCALE_COL_VNUM];
		const std::string& c_rstrJob = TokenVector[ITEM_SCALE_COL_JOB];
		const std::string& c_rstrSex = TokenVector[ITEM_SCALE_COL_SEX];
		const std::string& c_rstrMeshScaleX = TokenVector[ITEM_SCALE_COL_MESH_SCALE_X];
		const std::string& c_rstrMeshScaleY = TokenVector[ITEM_SCALE_COL_MESH_SCALE_Y];
		const std::string& c_rstrMeshScaleZ = TokenVector[ITEM_SCALE_COL_MESH_SCALE_Z];

		uint32_t dwItemVnum = atoi(c_rstrID.c_str());
		uint8_t byJob = 0;
		if (!strcmp(c_rstrJob.c_str(), "JOB_WARRIOR"))
			byJob = NRaceData::JOB_WARRIOR;
		if (!strcmp(c_rstrJob.c_str(), "JOB_ASSASSIN"))
			byJob = NRaceData::JOB_ASSASSIN;
		if (!strcmp(c_rstrJob.c_str(), "JOB_SURA"))
			byJob = NRaceData::JOB_SURA;
		if (!strcmp(c_rstrJob.c_str(), "JOB_SHAMAN"))
			byJob = NRaceData::JOB_SHAMAN;
#ifdef ENABLE_WOLFMAN_CHARACTER
		if (!strcmp(c_rstrJob.c_str(), "JOB_WOLFMAN"))
			byJob = NRaceData::JOB_WOLFMAN;
#endif
		uint8_t bySex = c_rstrSex[0] == 'M';

		float fMeshScaleX = atof(c_rstrMeshScaleX.c_str()) * 0.01f;
		float fMeshScaleY = atof(c_rstrMeshScaleY.c_str()) * 0.01f;
		float fMeshScaleZ = atof(c_rstrMeshScaleZ.c_str()) * 0.01f;

		float fParticleScale = 1.0f;
		if (TokenVector.size() == 7)
		{
			const std::string& c_rstrParticleScale = TokenVector[ITEM_SCALE_COL_PARTICLE_SCALE];
			fParticleScale = atof(c_rstrParticleScale.c_str());
		}

		CItemData* pItemData = MakeItemData(dwItemVnum);
		uint8_t bGradeMax = 5;
#ifdef ENABLE_AURA_SYSTEM
		if (pItemData->GetType() == CItemData::ITEM_TYPE_COSTUME && pItemData->GetSubType() == CItemData::COSTUME_AURA)
			bGradeMax = 6;
#endif

		for (uint8_t i = 0; i < bGradeMax; ++i)
		{
			pItemData = MakeItemData(dwItemVnum + i);
			if (pItemData)
				pItemData->SetItemTableScaleData(byJob, bySex, fMeshScaleX, fMeshScaleY, fMeshScaleZ, fParticleScale);
		}
	}

	return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Load Item Table

bool CItemManager::LoadItemList(const char * c_szFileName)
{
	CMappedFile File;
	LPCVOID pData;

	if (!CEterPackManager::Instance().Get(File, c_szFileName, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (!(TokenVector.size() == 3 || TokenVector.size() == 4))
		{
			TraceError(" CItemManager::LoadItemList(%s) - StrangeLine in %d\n", c_szFileName, i);
			continue;
		}

		const std::string & c_rstrID = TokenVector[0];
#ifdef ENABLE_AURA_SYSTEM
		const std::string & c_rstrType = TokenVector[1];
#endif
		const std::string & c_rstrIcon = TokenVector[2];

		uint32_t dwItemVNum = atoi(c_rstrID.c_str());

		CItemData * pItemData = MakeItemData(dwItemVNum);

		if (4 == TokenVector.size())
		{
#ifdef ENABLE_AURA_SYSTEM
			if (!strcmp(c_rstrType.c_str(), "AURA"))
			{
				const std::string& c_rstrAuraEffectFileName = TokenVector[3];
				pItemData->SetAuraEffectID(c_rstrAuraEffectFileName.c_str());
				pItemData->SetDefaultItemData(c_rstrIcon.c_str());
			}
			else
			{
				const std::string& c_rstrModelFileName = TokenVector[3];
				pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
			}
#else
			const std::string & c_rstrModelFileName = TokenVector[3];
			pItemData->SetDefaultItemData(c_rstrIcon.c_str(), c_rstrModelFileName.c_str());
#endif
		}
		else
			pItemData->SetDefaultItemData(c_rstrIcon.c_str());
	}

	return true;
}

const std::string & __SnapString(const std::string & c_rstSrc, std::string & rstTemp)
{
	uint32_t uSrcLen = c_rstSrc.length();
	if (uSrcLen < 2)
		return c_rstSrc;

	if (c_rstSrc[0] != '"')
		return c_rstSrc;

	uint32_t uLeftCut = 1;

	uint32_t uRightCut = uSrcLen;
	if (c_rstSrc[uSrcLen - 1] == '"')
		uRightCut = uSrcLen - 1;

	rstTemp = c_rstSrc.substr(uLeftCut, uRightCut - uLeftCut);
	return rstTemp;
}

bool CItemManager::LoadItemDesc(const char * c_szFileName)
{
	const VOID * pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
	{
		Tracenf("CItemManager::LoadItemDesc(c_szFileName=%s) - Load Error", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	std::string stTemp;

	CTokenVector kTokenVector;
	for (uint32_t i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		while (kTokenVector.size() < ITEMDESC_COL_NUM)
			kTokenVector.emplace_back("");

		//assert(kTokenVector.size()==ITEMDESC_COL_NUM);

		uint32_t dwVnum = atoi(kTokenVector[ITEMDESC_COL_VNUM].c_str());
		const std::string & c_rstDesc = kTokenVector[ITEMDESC_COL_DESC];
		const std::string & c_rstSumm = kTokenVector[ITEMDESC_COL_SUMM];
		auto f = m_ItemMap.find(dwVnum);
		if (m_ItemMap.end() == f)
			continue;

		CItemData * pkItemDataFind = f->second;

		pkItemDataFind->SetDescription(__SnapString(c_rstDesc, stTemp));
		pkItemDataFind->SetSummary(__SnapString(c_rstSumm, stTemp));
	}
	return true;
}

uint32_t GetHashCode(const char * pString)
{
	unsigned long i, len;
	unsigned long ch;
	unsigned long result;

	len = strlen(pString);
	result = 5381;
	for (i = 0; i < len; i++)
	{
		ch = (unsigned long) pString[i];
		result = ((result << 5) + result) + ch; // hash * 33 + ch
	}

	return result;
}

bool CItemManager::LoadItemTable(const char * c_szFileName)
{
	CMappedFile file;
	LPCVOID pvData;

	if (!CEterPackManager::Instance().Get(file, c_szFileName, &pvData))
		return false;

	uint32_t dwFourCC, dwElements, dwDataSize;
	uint32_t dwVersion = 0;
	uint32_t dwStride = 0;

	file.Read(&dwFourCC, sizeof(uint32_t));

	if (dwFourCC == MAKEFOURCC('M', 'I', 'P', 'X'))
	{
		file.Read(&dwVersion, sizeof(uint32_t));
		file.Read(&dwStride, sizeof(uint32_t));

		if (dwVersion != 1)
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] VERSION[%d]", c_szFileName, dwVersion);
			return false;
		}

		if (dwStride != sizeof(CItemData::TItemTable))
		{
			TraceError("CPythonItem::LoadItemTable: invalid item_proto[%s] STRIDE[%d] != sizeof(SItemTable)", c_szFileName, dwStride,
					   sizeof(CItemData::TItemTable));
			return false;
		}
	}
	else if (dwFourCC != MAKEFOURCC('M', 'I', 'P', 'T'))
	{
		TraceError("CPythonItem::LoadItemTable: invalid item proto type %s", c_szFileName);
		return false;
	}

	file.Read(&dwElements, sizeof(uint32_t));
	file.Read(&dwDataSize, sizeof(uint32_t));

	std::vector<uint8_t> pbData(dwDataSize);
	file.Read(pbData.data(), dwDataSize);

	/////

	CLZObject zObj;

	if (!CLZO::Instance().Decompress(zObj, pbData.data(), s_adwItemProtoKey))
		return false;

	/////

	char szName[64 + 1];
	std::map<uint32_t, uint32_t> itemNameMap;

	for (uint32_t i = 0; i < dwElements; ++i)
	{
		CItemData::TItemTable & t = *((CItemData::TItemTable *) zObj.GetBuffer() + i);
		CItemData::TItemTable * table = &t;

		CItemData * pItemData;
		uint32_t dwVnum = table->dwVnum;

		if (auto f = m_ItemMap.find(dwVnum); m_ItemMap.end() == f)
		{
			_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum);
#ifdef ENABLE_INGAME_WIKI
			pItemData = CItemData::New();
#endif

			if (CResourceManager::Instance().IsFileExist(szName) == false)
			{
#ifdef ENABLE_INGAME_WIKI
				pItemData->ValidateImage(false);
#endif
				auto itVnum = itemNameMap.find(GetHashCode(table->szName));

				if (itVnum != itemNameMap.end())
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", itVnum->second);
				else
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", dwVnum - dwVnum % 10);

				if (CResourceManager::Instance().IsFileExist(szName) == false)
				{
#ifdef _DEBUG
					TraceError("%16s(#%-5d) cannot find icon file. setting to default.", table->szName, dwVnum);
#endif
					const uint32_t EmptyBowl = 27995;
					_snprintf(szName, sizeof(szName), "icon/item/%05d.tga", EmptyBowl);
				}
			}

			pItemData = CItemData::New();

			pItemData->SetDefaultItemData(szName);
			m_ItemMap.emplace(dwVnum, pItemData);
#ifdef ENABLE_INGAME_WIKI
			pItemData->SetItemTableData(table);
			if (!CResourceManager::Instance().IsFileExist(pItemData->GetIconFileName().c_str()))
				pItemData->ValidateImage(false);
#endif
		}
		else
		{
			pItemData = f->second;
#ifdef ENABLE_INGAME_WIKI
			pItemData->SetItemTableData(table);
#endif
		}
		if (itemNameMap.find(GetHashCode(table->szName)) == itemNameMap.end())
			itemNameMap.emplace(GetHashCode(table->szName), table->dwVnum);
		pItemData->SetItemTableData(table);
		if (0 != table->dwVnumRange)
			m_vec_ItemRange.emplace_back(pItemData);
	}
	return true;
}

#ifdef ENABLE_DS_SET
#include "ItemUtil.h"
float CItemManager::GetDSSetWeight(uint8_t bDSType, uint8_t bDSGrade) const
{
	float fResult = 0.0f;
	const uint8_t baStep[DS_STEP_MAX] =
	{
		DS_STEP_LOWEST,
		DS_STEP_LOW,
		DS_STEP_MID,
		DS_STEP_HIGH,
		DS_STEP_HIGHEST,
	};

	if (nullptr != m_pDSTable)
	{
		if (!m_pDSTable->GetWeight(bDSType, baStep[bDSGrade - 1], 0, 0, fResult))
			TraceError("Failed to get DSSetWeight type(%u) grade(%u)", bDSType, bDSGrade);
	}

	return fResult;
}

int CItemManager::GetDSBasicApplyCount(uint8_t bDSType, uint8_t bDSGrade) const
{
	int iBasicApplyNum = 0;
	int iAddMin, iAddMax;
	if (nullptr != m_pDSTable)
	{
		if (!m_pDSTable->GetApplyNumSettings(bDSType, bDSGrade, iBasicApplyNum, iAddMin, iAddMax))
			TraceError("Failed to get DSBasicApplyCount type(%u) grade(%u)", bDSType, bDSGrade);
	}

	return iBasicApplyNum;
}

int CItemManager::GetDSBasicApplyValue(uint8_t bDSType, uint16_t wApplyType) const // @fixme436
{
	int iApplyValue = 0;
	CDragonSoulTable::TVecApplys vecBasicApplys;
	if (nullptr != m_pDSTable)
	{
		if (!m_pDSTable->GetBasicApplys(bDSType, vecBasicApplys))
		{
			TraceError("There is no BasicApply about %u type dragon soul.", bDSType);
			return 0;
		}
		for (CDragonSoulTable::TVecApplys::const_iterator it = vecBasicApplys.begin(); it != vecBasicApplys.end(); ++it)
		{
			const CDragonSoulTable::SApply& apply = *it;
			if (apply.apply_type == wApplyType)
			{
				iApplyValue = apply.apply_value;
				break;
			}
		}
	}
	return iApplyValue;
}

int CItemManager::GetDSAdditionalApplyValue(uint8_t bDSType, uint16_t wApplyType) const // @fixme436
{
	int iApplyValue = 0;
	CDragonSoulTable::TVecApplys vecAdditionalApplys;
	if (nullptr != m_pDSTable)
	{
		if (!m_pDSTable->GetAdditionalApplys(bDSType, vecAdditionalApplys))
		{
			TraceError("There is no AdditionalApply about %u type dragon soul.", bDSType);
			return 0;
		}
		for (CDragonSoulTable::TVecApplys::const_iterator it = vecAdditionalApplys.begin(); it != vecAdditionalApplys.end(); ++it)
		{
			const CDragonSoulTable::SApply& apply = *it;
			if (apply.apply_type == wApplyType)
			{
				iApplyValue = apply.apply_value;
				break;
			}
		}
	}
	return iApplyValue;
}

bool CItemManager::LoadDragonSoulTable(const char* c_szFileName)
{
	if (nullptr != m_pDSTable)
	{
		m_pDSTable->Clear();
		delete m_pDSTable;
		m_pDSTable = nullptr;
	}

	m_pDSTable = new CDragonSoulTable;
	return m_pDSTable->ReadDragonSoulTableFile(c_szFileName);
}
#endif

#ifdef ENABLE_SHINING_SYSTEM
bool CItemManager::LoadShiningTable(const char* szShiningTable)
{
	CMappedFile File;
	LPCVOID pData;

	if (!CEterPackManager::Instance().Get(File, szShiningTable, &pData))
		return false;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(File.Size(), pData);

	CTokenVector TokenVector;
	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &TokenVector, "\t"))
			continue;

		if (TokenVector.size() > (1 + CItemData::ITEM_SHINING_MAX_COUNT))
		{
			TraceError("CItemManager::LoadShiningTable(%s) - LoadShiningTable in %d\n - RowSize: %d MaxRowSize: %d", szShiningTable, i, TokenVector.size(), CItemData::ITEM_SHINING_MAX_COUNT);
		}

		const std::string& c_rstrID = TokenVector[0];
		uint32_t dwItemVNum = atoi(c_rstrID.c_str());

		CItemData* pItemData = MakeItemData(dwItemVNum);
		if (pItemData)
		{
			for (uint8_t i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
			{
				if (i < (TokenVector.size() - 1))
				{
					const std::string& c_rstrEffectPath = TokenVector[i + 1];
					pItemData->SetItemShiningTableData(i, c_rstrEffectPath.c_str());
				}
				else
				{
					pItemData->SetItemShiningTableData(i, "");
				}
			}
		}
	}

	return true;
}
#endif

void CItemManager::Destroy()
{
	for (auto & i : m_ItemMap)
		CItemData::Delete(i.second);

	m_ItemMap.clear();
#ifdef ENABLE_INGAME_WIKI
	m_tempItemVec.clear();
#endif

#ifdef ENABLE_DS_SET
	if (nullptr != m_pDSTable)
	{
		m_pDSTable->Clear();
		delete m_pDSTable;
		m_pDSTable = nullptr;
	}
#endif
}

#ifdef ENABLE_INGAME_WIKI
bool CItemManager::CanIncrSelectedItemRefineLevel()
{
	auto* tbl = GetSelectedItemDataPointer();
	if (!tbl)
		return FALSE;

	return (tbl->GetType() == CItemData::ITEM_TYPE_ARMOR || tbl->GetType() == CItemData::ITEM_TYPE_WEAPON);
}

bool CItemManager::CanIncrItemRefineLevel(uint32_t itemVnum)
{
	CItemData* tbl = nullptr;
	if (!GetItemDataPointer(itemVnum, &tbl))
		return false;

	if (!tbl)
		return FALSE;

	return (tbl->GetType() == CItemData::ITEM_TYPE_ARMOR || tbl->GetType() == CItemData::ITEM_TYPE_WEAPON);
}

bool CItemManager::CanLoadWikiItem(uint32_t dwVnum)
{
	uint32_t StartRefineVnum = GetWikiItemStartRefineVnum(dwVnum);

	if (StartRefineVnum != dwVnum)
		return false;

	if (StartRefineVnum % 10 != 0)
		return false;

	CItemData * tbl = nullptr;
	if (!GetItemDataPointer(StartRefineVnum, &tbl))
		return false;

	return true;
}

uint32_t CItemManager::GetWikiItemStartRefineVnum(uint32_t dwVnum)
{
	auto baseItemName = GetWikiItemBaseRefineName(dwVnum);
	if (!baseItemName.size())
		return 0;

	uint32_t manage_vnum = dwVnum;
	while (!(strcmp(baseItemName.c_str(), GetWikiItemBaseRefineName(manage_vnum).c_str())))
		--manage_vnum;

	return (manage_vnum + 1);
}

std::string CItemManager::GetWikiItemBaseRefineName(uint32_t dwVnum)
{
	CItemData * tbl = nullptr;
	if (!GetItemDataPointer(dwVnum, &tbl))
		return "";

	auto* p = const_cast<char*>(strrchr(tbl->GetName(), '+'));
	if (!p)
		return "";

	std::string sFirstItemName(tbl->GetName(),
				(tbl->GetName() + (p - tbl->GetName())));

	return sFirstItemName;
}

bool CItemManager::IsFilteredAntiflag(CItemData* itemData, uint32_t raceFilter)
{
	if (raceFilter != 0)
	{
		if (!itemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SHAMAN) && raceFilter & CItemData::ITEM_ANTIFLAG_SHAMAN)
			return false;

		if (!itemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SURA) && raceFilter & CItemData::ITEM_ANTIFLAG_SURA)
			return false;

		if (!itemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_ASSASSIN) && raceFilter & CItemData::ITEM_ANTIFLAG_ASSASSIN)
			return false;

		if (!itemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WARRIOR) && raceFilter & CItemData::ITEM_ANTIFLAG_WARRIOR)
			return false;

#ifdef ENABLE_WOLFMAN_CHARACTER
		if (!itemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WOLFMAN) && raceFilter & CItemData::ITEM_ANTIFLAG_WOLFMAN)
			return false;
#endif
	}

	return true;
}

size_t CItemManager::WikiLoadClassItems(uint8_t classType, uint32_t raceFilter)
{
	m_tempItemVec.clear();

	for (TItemMap::iterator it = m_ItemMap.begin(); it != m_ItemMap.end(); ++it)
	{
		if (!it->second->IsValidImage() || it->first < 10 || it->second->IsBlacklisted())
			continue;

		bool _can_load = CanLoadWikiItem(it->first);

		switch (classType)
		{
		case 0: // weapon
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_WEAPON && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 1: // body
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_BODY && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 2:
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_EAR && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 3:
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_FOOTS && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 4:
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_HEAD && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 5:
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_NECK && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 6:
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_SHIELD && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 7:
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_WRIST && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 8: // chests
			if (it->second->GetType() == CItemData::ITEM_TYPE_GIFTBOX)
				m_tempItemVec.emplace_back(it->first);
			break;
		case 9: // belts
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_BELT)
				m_tempItemVec.emplace_back(it->first);
			break;
		case 10: // talisman
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_PENDANT && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		case 11: // gloves
			if (_can_load && it->second->GetType() == CItemData::ITEM_TYPE_ARMOR && it->second->GetSubType() == CItemData::ARMOR_GLOVE && !IsFilteredAntiflag(it->second, raceFilter))
				m_tempItemVec.emplace_back(it->first);
			break;
		}
	}

	return m_tempItemVec.size();
}

std::tuple<const char*, int> CItemManager::SelectByNamePart(const char* namePart)
{
	char searchName[CItemData::ITEM_NAME_MAX_LEN + 1];
	memcpy(searchName, namePart, sizeof(searchName));
	for (size_t j = 0; j < sizeof(searchName); j++)
		searchName[j] = static_cast<char>(tolower(searchName[j]));
	std::string tempSearchName = searchName;

	for (TItemMap::iterator i = m_ItemMap.begin(); i != m_ItemMap.end(); i++)
	{
		const CItemData::TItemTable* tbl = i->second->GetTable();

		if (!i->second->IsBlacklisted())
		{
			uint32_t StartRefineVnum = GetWikiItemStartRefineVnum(i->first);
			if (StartRefineVnum != 0)
			{
				CItemData * _sRb = nullptr;
				if (!GetItemDataPointer(StartRefineVnum, &_sRb))
					continue;

				if (_sRb->IsBlacklisted())
					continue;
			}
		}
		else
			continue;

		CItemData* itemData = nullptr;
		if (!GetItemDataPointer(i->first, &itemData))
			continue;

		std::string tempName = itemData->GetName();
		if (!tempName.size())
			continue;

		std::transform(tempName.begin(), tempName.end(), tempName.begin(), ::tolower);

		const size_t tempSearchNameLenght = tempSearchName.length();
		if (tempName.length() < tempSearchNameLenght)
			continue;

		if (!tempName.substr(0, tempSearchNameLenght).compare(tempSearchName))
			return std::make_tuple(itemData->GetName(), i->first);
	}

	return std::make_tuple("", -1);
}
#endif


#ifdef ENABLE_DS_SET
CItemManager::CItemManager() : m_pSelectedItemData(nullptr), m_pDSTable(nullptr) {}
#else
CItemManager::CItemManager() : m_pSelectedItemData(nullptr) {}
#endif

CItemManager::~CItemManager()
{
	Destroy();
}
