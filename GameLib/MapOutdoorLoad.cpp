#include "StdAfx.h"
#include "MapOutdoor.h"
#include "AreaTerrain.h"
#include "AreaLoaderThread.h"
#include "../EterLib/ResourceManager.h"
#include "../EterPack/EterPackManager.h"

bool CMapOutdoor::Load(float x, float y, float z)
{
	Destroy();

	CEterPackManager & rkPackMgr = CEterPackManager::Instance();
	{
		static std::string s_strOldPathName = s_strOldPathName;

		// 2004.08.09.myevan.Pack파일을 찾을때.. 폴더명만으로는 그냥 리턴되는 부분이 있다
		std::string c_rstrNewPathName = GetName() + "\\cache";

		s_strOldPathName = c_rstrNewPathName;
	}

	std::string strFileName = GetMapDataDirectory() + "\\Setting.txt";

	if (!LoadSetting(strFileName.c_str()))
		TraceError("CMapOutdoor::Load : LoadSetting(%s) Failed", strFileName.c_str());

#ifdef ENABLE_LOAD_SUNGMA_TABLE
	//LoadSungmaSetting();
#endif

#ifdef WORLD_EDITOR // @fixme015
	if (!LoadMonsterAreaInfo())
		TraceError("CMapOutdoor::Load - LoadMonsterAreaInfo ERROR");
#endif

	CreateTerrainPatchProxyList();
	BuildQuadTree();
	LoadWaterTexture();
	CreateCharacterShadowTexture();

	m_lOldReadX = -1;

	// TODO: SetRenderingDevice에서 Environment로 부터 라이트 속성을 넘겨줘야 스태틱 라이트가 제대로 작동한다.
	CSpeedTreeDirectX::Instance().SetRenderingDevice();

	Update(x, y, z);

	__HeightCache_Init();

	// LOCAL_ENVIRONMENT_DATA
#ifdef ENABLE_WEATHER_INFO_EXTENDED
	const std::string& c_rstrEnvironmentRoot = "d:/ymir work/environment/";
	const std::string& c_rstrMapName = GetName();
	const std::string& strAppendName = c_rstrMapName.substr(c_rstrMapName.size() - 2, 2);

	extern uint8_t GetWeatherDayMode(int Hour);
	extern std::string GetEnvironment(int dMode);
	extern bool GetMapEnvData(std::string strMapName);

	if (GetMapEnvData(c_rstrMapName))
	{
		time_t ct = time(nullptr);
		struct tm tm = *localtime(&ct);
		//TraceError("LOAD DayMode: %d", iBG.GetWeatherDayMode(tm.tm_hour));

		m_envDataName = c_rstrEnvironmentRoot + GetEnvironment(GetWeatherDayMode(tm.tm_hour));
		//TraceError("LOAD Env Path: %s", m_envDataName.c_str());
	}
	else
	{
		std::string local_envDataName = GetMapDataDirectory() + "\\" + m_settings_envDataName;
		if (rkPackMgr.isExist(local_envDataName.c_str()))
			m_envDataName = local_envDataName;
		else
		{
			m_envDataName = c_rstrEnvironmentRoot + m_settings_envDataName;
			if (c_rstrEnvironmentRoot == m_envDataName)
				m_envDataName = c_rstrEnvironmentRoot + strAppendName + ".msenv";
		}
	}
#else
	std::string local_envDataName = GetMapDataDirectory() + "\\" + m_settings_envDataName;
	if (rkPackMgr.isExist(local_envDataName.c_str()))
		m_envDataName = local_envDataName;
	else
	{
		const std::string & c_rstrEnvironmentRoot = "d:/ymir work/environment/";
		const std::string & c_rstrMapName = GetName();
		m_envDataName = c_rstrEnvironmentRoot + m_settings_envDataName;

		if (c_rstrEnvironmentRoot == m_envDataName)
		{
			const std::string & strAppendName = c_rstrMapName.substr(c_rstrMapName.size() - 2, 2);
			m_envDataName = c_rstrEnvironmentRoot + strAppendName + ".msenv";
		}
	}
#endif
	// LOCAL_ENVIRONMENT_DATA_END
	return true;
}

std::string & CMapOutdoor::GetEnvironmentDataName()
{
	return m_envDataName;
}


bool CMapOutdoor::isTerrainLoaded(uint16_t wX, uint16_t wY)
{
	for (auto pTerrain : m_TerrainVector)
	{
		uint16_t usCoordX, usCoordY;
		pTerrain->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordX == wX && usCoordY == wY)
			return true;
	}
	return false;
}

bool CMapOutdoor::isAreaLoaded(uint16_t wX, uint16_t wY)
{
	for (auto pArea : m_AreaVector)
	{
		uint16_t usCoordX, usCoordY;
		pArea->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordX == wX && usCoordY == wY)
			return true;
	}
	return false;
}


// 현재 좌표를 기반으로 주위(ex. 3x3)에 있는 Terrain과 Area포인터를
// m_pTerrain과 m_pArea에 연결한다.
void CMapOutdoor::AssignTerrainPtr()
{
	// 월드에디터에서 화면을 죽죽죽 넘길 때 터레인을 저장해야 하기
	// 때문에 이 virtual method를 호출 한다. 이 메소드는 CMapOutDoor에서는 아무 행동도
	// 하지 않는다.
	OnPreAssignTerrainPtr();

	int16_t sReferenceCoordMinX, sReferenceCoordMaxX, sReferenceCoordMinY, sReferenceCoordMaxY;
	sReferenceCoordMinX = std::max(m_CurCoordinate.m_sTerrainCoordX - LOAD_SIZE_WIDTH, 0);
	sReferenceCoordMaxX = std::min(m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH, m_sTerrainCountX - 1);
	sReferenceCoordMinY = std::max(m_CurCoordinate.m_sTerrainCoordY - LOAD_SIZE_WIDTH, 0);
	sReferenceCoordMaxY = std::min(m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH, m_sTerrainCountY - 1);

	uint32_t i;
	for (i = 0; i < AROUND_AREA_NUM; ++i)
	{
		m_pArea[i] = nullptr;
		m_pTerrain[i] = nullptr;
	}

	for (i = 0; i < m_TerrainVector.size(); ++i)
	{
		CTerrain * pTerrain = m_TerrainVector[i];
		uint16_t usCoordX, usCoordY;
		pTerrain->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordX >= sReferenceCoordMinX && usCoordX <= sReferenceCoordMaxX && usCoordY >= sReferenceCoordMinY &&
			usCoordY <= sReferenceCoordMaxY)
		{
			m_pTerrain[(usCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
					   (usCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH)] = pTerrain;
		}
	}

	for (i = 0; i < m_AreaVector.size(); ++i)
	{
		CArea * pArea = m_AreaVector[i];
		uint16_t usCoordX, usCoordY;
		pArea->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordX >= sReferenceCoordMinX && usCoordX <= sReferenceCoordMaxX && usCoordY >= sReferenceCoordMinY &&
			usCoordY <= sReferenceCoordMaxY)
		{
			m_pArea[(usCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
					(usCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH)] = pArea;
		}
	}
}

bool CMapOutdoor::LoadArea(uint16_t wAreaCoordX, uint16_t wAreaCoordY, uint16_t wCellCoordX, uint16_t wCellCoordY)
{
	if (isAreaLoaded(wAreaCoordX, wAreaCoordY))
		return true;
#ifdef _DEBUG
	uint32_t dwStartTime = ELTimer_GetMSec();
#endif
	unsigned long ulID = (unsigned long) (wAreaCoordX) *1000L + (unsigned long) (wAreaCoordY);
	char szAreaPathName[64 + 1];
	_snprintf(szAreaPathName, sizeof(szAreaPathName), "%s\\%06u\\", GetMapDataDirectory().c_str(), ulID);

	CArea * pArea = CArea::New();
	pArea->SetMapOutDoor(this);
#ifdef _DEBUG
	Tracef("CMapOutdoor::LoadArea1 %d\n", ELTimer_GetMSec() - dwStartTime);
	dwStartTime = ELTimer_GetMSec();
#endif

	pArea->SetCoordinate(wAreaCoordX, wAreaCoordY);
	if (!pArea->Load(szAreaPathName))
		TraceError(" CMapOutdoor::LoadArea(%d, %d) LoadShadowMap ERROR", wAreaCoordX, wAreaCoordY);
#ifdef _DEBUG
	Tracef("CMapOutdoor::LoadArea2 %d\n", ELTimer_GetMSec() - dwStartTime);
	dwStartTime = ELTimer_GetMSec();
#endif

	m_AreaVector.emplace_back(pArea);

	pArea->EnablePortal(m_bEnablePortal);
#ifdef _DEBUG
	Tracef("CMapOutdoor::LoadArea3 %d\n", ELTimer_GetMSec() - dwStartTime);
#endif

	return true;
}

bool CMapOutdoor::LoadTerrain(uint16_t wTerrainCoordX, uint16_t wTerrainCoordY, uint16_t wCellCoordX, uint16_t wCellCoordY)
{
	if (isTerrainLoaded(wTerrainCoordX, wTerrainCoordY))
		return true;

	//////////////////////////////////////////////////////////////////////////
	uint32_t dwStartTime = ELTimer_GetMSec();

	unsigned long ulID = (unsigned long) (wTerrainCoordX) *1000L + (unsigned long) (wTerrainCoordY);
	char filename[256];
	sprintf(filename, "%s\\%06lu\\AreaProperty.txt", GetMapDataDirectory().c_str(), ulID);

	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(filename, stTokenVectorMap))
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty Read Error\n");
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 1\n");
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("areaname"))
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 2\n");
		return false;
	}

	const std::string & c_rstrType = stTokenVectorMap["scripttype"][0];
	const std::string & c_rstrAreaName = stTokenVectorMap["areaname"][0];

	if (c_rstrType != "AreaProperty")
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 3\n");
		return false;
	}

	CTerrain * pTerrain = CTerrain::New();

	pTerrain->Clear();
	pTerrain->SetMapOutDoor(this);

	pTerrain->SetCoordinate(wTerrainCoordX, wTerrainCoordY);

	pTerrain->CopySettingFromGlobalSetting();

	char szRawHeightFieldname[64 + 1];
	char szWaterMapName[64 + 1];
	char szAttrMapName[64 + 1];
	char szShadowTexName[64 + 1];
	char szShadowMapName[64 + 1];
	char szMiniMapTexName[64 + 1];
	char szSplatName[64 + 1];

	_snprintf(szRawHeightFieldname, sizeof(szRawHeightFieldname), "%s\\%06u\\height.raw", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szSplatName, sizeof(szSplatName), "%s\\%06u\\tile.raw", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szAttrMapName, sizeof(szAttrMapName), "%s\\%06u\\attr.atr", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szWaterMapName, sizeof(szWaterMapName), "%s\\%06u\\water.wtr", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szShadowTexName, sizeof(szShadowTexName), "%s\\%06u\\shadowmap.dds", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szShadowMapName, sizeof(szShadowMapName), "%s\\%06u\\shadowmap.raw", GetMapDataDirectory().c_str(), ulID);
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	if (IsSnowTextureMode())
	{
		_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap_snow.dds", GetMapDataDirectory().c_str(), ulID);

		if (!CEterPackManager::Instance().isExist(szMiniMapTexName))
			_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);
	}
	else
		_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);
#else
	_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);
#endif

	if (!pTerrain->LoadWaterMap(szWaterMapName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadWaterMap ERROR", wTerrainCoordX, wTerrainCoordY);

	if (!pTerrain->LoadHeightMap(szRawHeightFieldname))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadHeightMap ERROR", wTerrainCoordX, wTerrainCoordY);

	if (!pTerrain->LoadAttrMap(szAttrMapName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadAttrMap ERROR", wTerrainCoordX, wTerrainCoordY);

	if (!pTerrain->RAW_LoadTileMap(szSplatName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) RAW_LoadTileMap ERROR", wTerrainCoordX, wTerrainCoordY);

	pTerrain->LoadShadowTexture(szShadowTexName);

	if (!pTerrain->LoadShadowMap(szShadowMapName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadShadowMap ERROR", wTerrainCoordX, wTerrainCoordY);

	pTerrain->LoadMiniMapTexture(szMiniMapTexName);
	pTerrain->SetName(c_rstrAreaName);
	pTerrain->CalculateTerrainPatch();

	pTerrain->SetReady();

	Tracef("CMapOutdoor::LoadTerrain %d\n", ELTimer_GetMSec() - dwStartTime);

	m_TerrainVector.emplace_back(pTerrain);

	return true;
}

bool CMapOutdoor::LoadSetting(const char * c_szFileName)
{
	NANOBEGIN
	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(c_szFileName, stTokenVectorMap))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - LoadMultipleTextData", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'scripttype' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("viewradius"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'viewradius' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("cellscale"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'cellscale' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("heightscale"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'heightscale' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("mapsize"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'mapsize' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("textureset"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'textureset' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() != stTokenVectorMap.find("terrainvisible"))
		m_bSettingTerrainVisible = (bool) (atoi(stTokenVectorMap["terrainvisible"][0].c_str()) != 0);
	else
		m_bSettingTerrainVisible = true;

	const std::string & c_rstrType = stTokenVectorMap["scripttype"][0];
	const std::string & c_rstrViewRadius = stTokenVectorMap["viewradius"][0];
	//const std::string & c_rstrCellScale = stTokenVectorMap["cellscale"][0];
	const std::string & c_rstrHeightScale = stTokenVectorMap["heightscale"][0];
	const std::string & c_rstrMapSizeX = stTokenVectorMap["mapsize"][0];
	const std::string & c_rstrMapSizeY = stTokenVectorMap["mapsize"][1];

	std::string strTextureSet;
	TTokenVector & rkVec_strToken = stTokenVectorMap["textureset"];
	if (!rkVec_strToken.empty())
		strTextureSet = rkVec_strToken[0];

	if (c_rstrType != "MapSetting")
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - Resourse Type ERROR", c_szFileName);
		return false;
	}

	m_lViewRadius = atol(c_rstrViewRadius.c_str());

#ifdef WORLD_EDITOR
	m_lViewRadius <<= 1;
#endif

	if (0L >= m_lViewRadius)
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - VIEWRADIUS IS NOT GREATER THAN 0", c_szFileName);
		return false;
	}

	m_fHeightScale = atof(c_rstrHeightScale.c_str());

	SetTerrainCount(atoi(c_rstrMapSizeX.c_str()), atoi(c_rstrMapSizeY.c_str()));

	m_fTerrainTexCoordBase = 1.0f / (float) (CTerrainImpl::PATCH_XSIZE * CTerrainImpl::CELLSCALE);

	if (stTokenVectorMap.end() != stTokenVectorMap.find("baseposition"))
	{
		const std::string & c_rstrMapBaseX = stTokenVectorMap["baseposition"][0];
		const std::string & c_rstrMapBaseY = stTokenVectorMap["baseposition"][1];
		SetBaseXY((uint32_t) atol(c_rstrMapBaseX.c_str()), (uint32_t) atol(c_rstrMapBaseY.c_str()));
	}

	std::string stTextureSetFileName = strTextureSet;

	// TextureSet 이 이미 붙어 있을 경우 안붙인다.
	if (0 != stTextureSetFileName.find_first_of("textureset", 0))
		stTextureSetFileName = "textureset\\" + strTextureSet;

	std::string stSnowTextureSetFileName = "textureset\\snow\\snow_";
	stSnowTextureSetFileName.append(CFileNameHelper::NoPath(stTextureSetFileName));

	if (!m_TextureSet.Load(stTextureSetFileName.c_str(), m_fTerrainTexCoordBase))
	{
#ifdef WORLD_EDITOR
		// TextureSet 이 적용되어 있지 않아도 진행
		LogBoxf("TextureSet not successfully applied to [%s].\nTexture not found [%s].", c_szFileName, stTextureSetFileName.c_str());
#else
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - LOAD TEXTURE SET(%s) ERROR", c_szFileName, stTextureSetFileName.c_str());
		return false;
#endif
	}

	if (!m_SnowTextureSet.Load(stSnowTextureSetFileName.c_str(), m_fTerrainTexCoordBase))
	{
#ifdef WORLD_EDITOR
		// TextureSet 이 적용되어 있지 않아도 진행
		LogBox("TextureSet 이 적용되어있지 않은 맵 입니다.\n지형 텍스춰 작업에 주의하시기 바랍니다.");
#else
		TraceError("%s Could not load ", stSnowTextureSetFileName.c_str());
		m_SnowTextureSet.Load(stTextureSetFileName.c_str(), m_fTerrainTexCoordBase);
#endif
	}

	CTerrain::SetTextureSet(&m_TextureSet);
	CTerrain::SetSnowTextureSet(&m_SnowTextureSet);

	if (stTokenVectorMap.end() != stTokenVectorMap.find("environment"))
	{
		const CTokenVector & c_rEnvironmentVector = stTokenVectorMap["environment"];
		if (!c_rEnvironmentVector.empty())
			m_settings_envDataName = c_rEnvironmentVector[0];
		else
			TraceError("CMapOutdoor::LoadSetting(c_szFileName=%s) - Failed to load environment data\n", c_szFileName);
	}

	m_fWaterTexCoordBase = 1.0f / (float) (CTerrainImpl::CELLSCALE * 4);

	D3DXMatrixScaling(
		&m_matSplatAlpha,
		+m_fTerrainTexCoordBase * 2.0f * (float) (CTerrainImpl::PATCH_XSIZE) / (float) (CTerrainImpl::SPLATALPHA_RAW_XSIZE - 2),
		-m_fTerrainTexCoordBase * 2.0f * (float) (CTerrainImpl::PATCH_YSIZE) / (float) (CTerrainImpl::SPLATALPHA_RAW_XSIZE - 2), 0.0f);
	m_matSplatAlpha._41 = m_fTerrainTexCoordBase * 4.6f;
	m_matSplatAlpha._42 = m_fTerrainTexCoordBase * 4.6f;

	D3DXMatrixScaling(&m_matStaticShadow, +m_fTerrainTexCoordBase * ((float) CTerrainImpl::PATCH_XSIZE / CTerrainImpl::XSIZE),
					  -m_fTerrainTexCoordBase * ((float) CTerrainImpl::PATCH_YSIZE / CTerrainImpl::XSIZE), 0.0f);
	m_matStaticShadow._41 = 0.0f;
	m_matStaticShadow._42 = 0.0f;

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	D3DXMatrixScaling(&m_matDynamicShadowScale, 1.0f / m_fShadowSizeX, -1.0f / m_fShadowSizeY, 1.0f);
#else
	D3DXMatrixScaling(&m_matDynamicShadowScale, 1.0f / 2550.0f, -1.0f / 2550.0f, 1.0f);
#endif
	m_matDynamicShadowScale._41 = 0.5f;
	m_matDynamicShadowScale._42 = 0.5f;

	// Transform
	D3DXMatrixScaling(&m_matBuildingTransparent, 1.0f / ((float) ms_iWidth), -1.0f / ((float) ms_iHeight), 1.0f);
	m_matBuildingTransparent._41 = 0.5f;
	m_matBuildingTransparent._42 = 0.5f;
	NANOEND
	return true;
}

#ifdef ENABLE_LOAD_SUNGMA_TABLE
bool CMapOutdoor::LoadSungmaSetting()
{
	char c_szFileName[256];
	sprintf(c_szFileName, "%s\\sungma_attr.txt", GetMapDataDirectory().c_str());

	LPCVOID pData;
	CMappedFile kFile;

	TraceError("LoadSungmaSetting");

	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pData))
	{
		//TraceError(" CMapOutdoorAccessor::LoadSungmaSetting Load File %s ERROR", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader textFileLoader;
	//CTokenVector stTokenVector;
	textFileLoader.Bind(kFile.Size(), pData);

	char szSungmaName[256];
	int iSungmaAttr;

	for (uint32_t uLineIndex = 0; uLineIndex < textFileLoader.GetLineCount(); ++uLineIndex)
	{
		const std::string& c_rstLine = textFileLoader.GetLineString(uLineIndex);
		sscanf(c_rstLine.c_str(), "%s %d", szSungmaName, &iSungmaAttr);

		if ('\0' == szSungmaName[0])
			continue;

		TSungmaMapInfo kMapInfo;
		kMapInfo.m_strName = szSungmaName;
		kMapInfo.m_dwValue = iSungmaAttr;
		TraceError("LoadSungmaSetting %s %d", szSungmaName, iSungmaAttr);
		m_kVct_kSungmaMapInfo.emplace_back(kMapInfo);
	}

	return true;
}
#endif

bool CMapOutdoor::LoadMonsterAreaInfo()
{
	RemoveAllMonsterAreaInfo();

	char c_szFileName[256];
	sprintf(c_szFileName, "%s\\regen.txt", GetMapDataDirectory().c_str());

	LPCVOID pModelData;
	CMappedFile File;

	if (!CEterPackManager::Instance().Get(File, c_szFileName, &pModelData))
	{
		//TraceError(" CMapOutdoorAccessor::LoadMonsterAreaInfo Load File %s ERROR", c_szFileName);
		return false;
	}

	CMemoryTextFileLoader textFileLoader;
	CTokenVector stTokenVector;

	textFileLoader.Bind(File.Size(), pModelData);

	for (uint32_t i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &stTokenVector))
			continue;

		stl_lowers(stTokenVector[0]);

		// Start or End
		if ("m" == stTokenVector[0] || "g" == stTokenVector[0])
		{
			if (stTokenVector.size() < 11)
			{
				TraceError("CMapOutdoorAccessor::LoadMonsterAreaInfo Get MonsterInfo File Format ERROR! continue....");
				continue;
			}

			CMonsterAreaInfo::EMonsterAreaInfoType eMonsterAreaInfoType;
			if ("m" == stTokenVector[0])
				eMonsterAreaInfoType = CMonsterAreaInfo::MONSTERAREAINFOTYPE_MONSTER;
			else if ("g" == stTokenVector[0])
				eMonsterAreaInfoType = CMonsterAreaInfo::MONSTERAREAINFOTYPE_GROUP;
			else
			{
				TraceError("CMapOutdoorAccessor::LoadMonsterAreaInfo Get MonsterInfo Data ERROR! continue....");
				continue;
			}

			const std::string & c_rstrOriginX = stTokenVector[1];
			const std::string & c_rstrOriginY = stTokenVector[2];
			const std::string & c_rstrSizeX = stTokenVector[3];
			const std::string & c_rstrSizeY = stTokenVector[4];
			const std::string & c_rstrZ = stTokenVector[5];
			const std::string & c_rstrDir = stTokenVector[6];
			const std::string & c_rstrTime = stTokenVector[7];
			const std::string & c_rstrPercent = stTokenVector[8];
			const std::string & c_rstrCount = stTokenVector[9];
			const std::string & c_rstrVID = stTokenVector[10];

			long lOriginX, lOriginY, lSizeX, lSizeY, lZ, lTime, lPercent;
			CMonsterAreaInfo::EMonsterDir eMonsterDir;
			uint32_t dwMonsterCount;
			uint32_t dwMonsterVID;

			lOriginX = atol(c_rstrOriginX.c_str());
			lOriginY = atol(c_rstrOriginY.c_str());
			lSizeX = atol(c_rstrSizeX.c_str());
			lSizeY = atol(c_rstrSizeY.c_str());
			lZ = atol(c_rstrZ.c_str());
			eMonsterDir = (CMonsterAreaInfo::EMonsterDir) atoi(c_rstrDir.c_str());
			lTime = atol(c_rstrTime.c_str());
			lPercent = atol(c_rstrPercent.c_str());
			dwMonsterCount = (uint32_t) atoi(c_rstrCount.c_str());
			dwMonsterVID = (uint32_t) atoi(c_rstrVID.c_str());

			//			lOriginX -= m_dwBaseX / 100;
			//			lOriginY -= m_dwBaseY / 100;

			CMonsterAreaInfo * pMonsterAreaInfo = AddMonsterAreaInfo(lOriginX, lOriginY, lSizeX, lSizeY);
			pMonsterAreaInfo->SetMonsterAreaInfoType(eMonsterAreaInfoType);
			if (CMonsterAreaInfo::MONSTERAREAINFOTYPE_MONSTER == eMonsterAreaInfoType)
				pMonsterAreaInfo->SetMonsterVID(dwMonsterVID);
			else if (CMonsterAreaInfo::MONSTERAREAINFOTYPE_GROUP == eMonsterAreaInfoType)
				pMonsterAreaInfo->SetMonsterGroupID(dwMonsterVID);
			pMonsterAreaInfo->SetMonsterCount(dwMonsterCount);
			pMonsterAreaInfo->SetMonsterDirection(eMonsterDir);
		}
	}

	return true;
}

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
bool CMapOutdoor::ReloadMinimapTexture(bool bSnowTexture)
{
	for (uint8_t byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
	{
		CTerrain* pTerrain;
		if (GetTerrainPointer(byTerrainNum, &pTerrain))
		{
			uint16_t usCoordX, usCoordY;
			pTerrain->GetCoordinate(&usCoordX, &usCoordY);
			unsigned long ulID = (unsigned long)(usCoordX) * 1000L + (unsigned long)(usCoordY);
			char szMiniMapTexName[64 + 1];
			if (bSnowTexture)
			{
				_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap_snow.dds", GetMapDataDirectory().c_str(), ulID);

				if (!CEterPackManager::Instance().isExist(szMiniMapTexName))
					_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);
			}
			else
				_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);

			pTerrain->LoadMiniMapTexture(szMiniMapTexName);
		}
	}

	return true;
}

bool CMapOutdoor::ReloadSetting(bool bSnowTexture)
{
	char szFileName[64 + 1];
	_snprintf(szFileName, sizeof(szFileName), "%s\\Setting.txt", GetMapDataDirectory().c_str());
	CTokenVectorMap stTokenVectorMap;
	if (!LoadMultipleTextData(szFileName, stTokenVectorMap))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - LoadMultipleTextData", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - FIND 'scripttype' - FAILED", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("viewradius"))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - FIND 'viewradius' - FAILED", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("cellscale"))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - FIND 'cellscale' - FAILED", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("heightscale"))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - FIND 'heightscale' - FAILED", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("mapsize"))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - FIND 'mapsize' - FAILED", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("textureset"))
	{
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - FIND 'textureset' - FAILED", szFileName);
		return false;
	}

	if (stTokenVectorMap.end() != stTokenVectorMap.find("terrainvisible"))
	{
		m_bSettingTerrainVisible = (bool)(atoi(stTokenVectorMap["terrainvisible"][0].c_str()) != 0);
	}
	else
	{
		m_bSettingTerrainVisible = true;
	}

	std::string strTextureSet;
	TTokenVector& rkVec_strToken = stTokenVectorMap["textureset"];
	if (!rkVec_strToken.empty())
	{
		strTextureSet = rkVec_strToken[0];
	}

	m_fTerrainTexCoordBase = 1.0f / (float)(CTerrainImpl::PATCH_XSIZE * CTerrainImpl::CELLSCALE);

	std::string stTextureSetFileName = strTextureSet;
	if (0 != stTextureSetFileName.find_first_of("textureset", 0))
	{
		if (bSnowTexture)
			stTextureSetFileName = "textureset\\snow\\snow_" + strTextureSet;
		else
			stTextureSetFileName = "textureset\\" + strTextureSet;
	}
	else
	{
		std::string stTextureFileName = strTextureSet.substr(strTextureSet.find("\\") + 1);
		if (bSnowTexture)
			stTextureSetFileName = "textureset\\snow\\snow_" + stTextureFileName;
		else
			stTextureSetFileName = "textureset\\" + stTextureFileName;
	}

	if (!m_TextureSet.Load(stTextureSetFileName.c_str(), m_fTerrainTexCoordBase))
	{
#ifdef WORLD_EDITOR
		// TextureSet 이 적용되어 있지 않아도 진행
		LogBoxf("TextureSet not successfully applied to [%s].\nTexture not found [%s].", c_szFileName, stTextureSetFileName.c_str());
#else
		TraceError("MapOutdoor::ReloadSetting(c_szFileName=%s) - LOAD TEXTURE SET(%s) ERROR", szFileName, stTextureSetFileName.c_str());
		return false;
#endif
	}

	CTerrain::SetTextureSet(&m_TextureSet);
	return true;
}
#endif
