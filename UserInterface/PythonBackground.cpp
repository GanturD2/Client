// PythonBackground.cpp: implementation of the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../EterLib/CullingManager.h"
#include "../EterLib/Camera.h"
#include "../EterPack/EterPackManager.h"
#include "../GameLib/MapOutdoor.h"
#include "../GameLib/PropertyLoader.h"

#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "PythonMiniMap.h"
#include "PythonSystem.h"
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "PythonDungeonInfo.h"
#endif

std::string g_strEffectName = "d:/ymir work/effect/etc/direction/direction_land.mse";
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
std::string g_strEffectShopPos = "d:/ymir work/effect/etc/direction/direction_land2.mse";
#endif

uint32_t CPythonBackground::GetRenderShadowTime() const
{
	return m_dwRenderShadowTime;
}

bool CPythonBackground::SetVisiblePart(int eMapOutDoorPart, bool isVisible)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.SetVisiblePart(eMapOutDoorPart, isVisible);
	return true;
}

void CPythonBackground::EnableTerrainOnlyForHeight()
{
	if (!m_pkMap)
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.EnableTerrainOnlyForHeight(TRUE);
}

bool CPythonBackground::SetSplatLimit(int iSplatNum)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.SetSplatLimit(iSplatNum);
	return true;
}

void CPythonBackground::CreateCharacterShadowTexture()
{
	if (!m_pkMap)
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.CreateCharacterShadowTexture();
}

void CPythonBackground::ReleaseCharacterShadowTexture()
{
	if (!m_pkMap)
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.ReleaseCharacterShadowTexture();
}

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
float CPythonBackground::GetShadowDistance()
{
	if (!m_pkMap)
		return 0.0f;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	return rkMap.GetShadowDistance();
}

void CPythonBackground::RefreshShadowTargetLevel()
{
	SetShadowTargetLevel(CPythonSystem::Instance().GetShadowTargetLevel());
}

bool CPythonBackground::SetShadowTargetLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	if (m_eShadowTargetLevel == eLevel)
		return true;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowTargetLevel = eLevel;

	switch (m_eShadowTargetLevel)
	{
		case SHADOW_NONE:
			rkMap.SetDrawShadow(false);
			rkMap.SetDrawCharacterShadow(false);
			break;

		case SHADOW_GROUND:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(false);
			break;

		case SHADOW_GROUND_AND_SOLO:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			break;

		case SHADOW_ALL:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			break;
	}

	return true;
}

void CPythonBackground::RefreshShadowQualityLevel()
{
	SetShadowQualityLevel(CPythonSystem::Instance().GetShadowQualityLevel());
}

bool CPythonBackground::SetShadowQualityLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	if (m_eShadowQualityLevel == eLevel)
		return true;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowQualityLevel = eLevel;

	switch (m_eShadowQualityLevel)
	{
		case SHADOW_AVERAGE:
			rkMap.SetShadowTextureSize(1024);
			break;

		case SHADOW_GOOD:
			rkMap.SetShadowTextureSize(2048);
			break;

		default:
			rkMap.SetShadowTextureSize(512);
			break;
	}

	return true;
}
#else
void CPythonBackground::RefreshShadowLevel()
{
	SetShadowLevel(CPythonSystem::Instance().GetShadowLevel());
}

bool CPythonBackground::SetShadowLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	if (m_eShadowLevel == eLevel)
		return true;

	CMapOutdoor & rkMap = GetMapOutdoorRef();

	m_eShadowLevel = eLevel;

	switch (m_eShadowLevel)
	{
		case SHADOW_NONE:
			rkMap.SetDrawShadow(false);
			rkMap.SetShadowTextureSize(512);
			break;

		case SHADOW_GROUND:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(false);
			rkMap.SetShadowTextureSize(512);
			break;

		case SHADOW_GROUND_AND_SOLO:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(512);
			break;

		case SHADOW_ALL:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(512);
			break;

		case SHADOW_ALL_HIGH:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(1024);
			break;

		case SHADOW_ALL_MAX:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(2048);
			break;
	}

	return true;
}
#endif

void CPythonBackground::SelectViewDistanceNum(int eNum)
{
	if (!m_pkMap)
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();

	if (!mc_pcurEnvironmentData)
	{
		TraceError("CPythonBackground::SelectViewDistanceNum(int eNum=%d) mc_pcurEnvironmentData is nullptr", eNum);
		return;
	}

	m_eViewDistanceNum = eNum;

	TEnvironmentData * env = (const_cast<TEnvironmentData *>(mc_pcurEnvironmentData));

	// 게임 분위기를 바꿔놓을 수 있으므로 reserve로 되어있으면 고치지 않는다.
	if (env->bReserve)
	{
		env->m_fFogNearDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
		env->m_fFogFarDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
		env->v3SkyBoxScale = m_ViewDistanceSet[m_eViewDistanceNum].m_v3SkyBoxScale;
		rkMap.SetEnvironmentSkyBox();
	}
}

void CPythonBackground::SetViewDistanceSet(int eNum, float fFarClip)
{
	if (!m_pkMap)
		return;

	m_ViewDistanceSet[eNum].m_fFogStart = fFarClip * 0.5f; //0.3333333f;
	m_ViewDistanceSet[eNum].m_fFogEnd = fFarClip * 0.7f; //0.6666667f;

	float fSkyBoxScale = fFarClip * 0.6f; //0.5773502f;
	m_ViewDistanceSet[eNum].m_v3SkyBoxScale = D3DXVECTOR3(fSkyBoxScale, fSkyBoxScale, fSkyBoxScale);
	m_ViewDistanceSet[eNum].m_fFarClip = fFarClip;

	if (eNum == m_eViewDistanceNum)
		SelectViewDistanceNum(eNum);
}

float CPythonBackground::GetFarClip()
{
	if (!m_pkMap)
		return 50000.0f;

	if (m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip == 0.0f)
	{
		TraceError("CPythonBackground::GetFarClip m_eViewDistanceNum=%d", m_eViewDistanceNum);
		m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip = 25600.0f;
	}

	return m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

void CPythonBackground::GetDistanceSetInfo(int * peNum, float * pfStart, float * pfEnd, float * pfFarClip) const
{
	if (!m_pkMap)
	{
		*peNum = 4;
		*pfStart = 10000.0f;
		*pfEnd = 15000.0f;
		*pfFarClip = 50000.0f;
		return;
	}
	*peNum = m_eViewDistanceNum;
	*pfStart = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
	*pfEnd = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
	*pfFarClip = m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPythonBackground::CPythonBackground()
{
	m_dwRenderShadowTime = 0;
	m_eViewDistanceNum = 0;
	m_eViewDistanceNum = 0;
	m_eViewDistanceNum = 0;
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	m_eShadowTargetLevel = SHADOW_NONE;
	m_eShadowQualityLevel = SHADOW_BAD;
#else
	m_eShadowLevel = SHADOW_NONE;
#endif
	m_dwBaseX = 0;
	m_dwBaseY = 0;
	m_strMapName = "";
	m_iDayMode = DAY_MODE_LIGHT;
	m_iXMasTreeGrade = 0;
	m_bVisibleGuildArea = FALSE;
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	m_bNightMode = TRUE;
	m_bSnowMode = TRUE;
	m_bSnowTextureMode = TRUE;
	m_bXMasEvent = FALSE;
#endif

	SetViewDistanceSet(4, 25600.0f);
	SetViewDistanceSet(3, 25600.0f);
	SetViewDistanceSet(2, 25600.0f);
	SetViewDistanceSet(1, 25600.0f);
	SetViewDistanceSet(0, 25600.0f);
	Initialize();
}

CPythonBackground::~CPythonBackground()
{
	Tracen("CPythonBackground Clear");
}

void CPythonBackground::Initialize()
{
#ifdef ENABLE_ATLASINFO_FROM_ROOT
	std::string stAtlasInfoFileName("AtlasInfo.txt");
#else
	std::string stAtlasInfoFileName(LocaleService_GetLocalePath());
	stAtlasInfoFileName += "/AtlasInfo.txt";
#endif
	SetAtlasInfoFileName(stAtlasInfoFileName.c_str());
	CMapManager::Initialize();
}

void CPythonBackground::__CreateProperty()
{
	if (CEterPackManager::SEARCH_FILE_FIRST == CEterPackManager::Instance().GetSearchMode() && _access("property", 0) == 0)
	{
		m_PropertyManager.Initialize(nullptr);

		CPropertyLoader PropertyLoader;
		PropertyLoader.SetPropertyManager(&m_PropertyManager);
		PropertyLoader.Create("*.*", "Property");
	}
	else
		m_PropertyManager.Initialize("pack/property");
}

//////////////////////////////////////////////////////////////////////
// Normal Functions
//////////////////////////////////////////////////////////////////////

bool CPythonBackground::GetPickingPoint(D3DXVECTOR3 * v3IntersectPt)
{
	CMapOutdoor & rkMap = GetMapOutdoorRef();
	return rkMap.GetPickingPoint(v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRay(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt)
{
	CMapOutdoor & rkMap = GetMapOutdoorRef();
	return rkMap.GetPickingPointWithRay(rRay, v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRayOnlyTerrain(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt)
{
	CMapOutdoor & rkMap = GetMapOutdoorRef();
	return rkMap.GetPickingPointWithRayOnlyTerrain(rRay, v3IntersectPt);
}

BOOL CPythonBackground::GetLightDirection(D3DXVECTOR3 & rv3LightDirection) const
{
	if (!mc_pcurEnvironmentData)
		return FALSE;

	rv3LightDirection.x = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x;
	rv3LightDirection.y = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y;
	rv3LightDirection.z = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CPythonBackground::Destroy()
{
	CMapManager::Destroy();
	m_SnowEnvironment.Destroy();
#ifdef ENABLE_ENVIRONMENT_RAIN
	m_RainEnvironment.Destroy();
#endif
	m_bVisibleGuildArea = FALSE;
}

void CPythonBackground::Create()
{
	static int s_isCreateProperty = false;

	if (!s_isCreateProperty)
	{
		s_isCreateProperty = true;
		__CreateProperty();
	}

	CMapManager::Create();

	m_SnowEnvironment.Create();
#ifdef ENABLE_ENVIRONMENT_RAIN
	m_RainEnvironment.Create();
#endif
}

struct FGetPortalID
{
	float m_fRequestX, m_fRequestY;
	std::set<int> m_kSet_iPortalID;
	FGetPortalID(float fRequestX, float fRequestY)
	{
		m_fRequestX = fRequestX;
		m_fRequestY = fRequestY;
	}
	void operator()(CGraphicObjectInstance * pObject)
	{
		for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
		{
			int iID = pObject->GetPortal(i);
			if (0 == iID)
				break;

			m_kSet_iPortalID.emplace(iID);
		}
	}
};

void CPythonBackground::Update(float fCenterX, float fCenterY, float fCenterZ)
{
	if (!IsMapReady())
		return;
	UpdateMap(fCenterX, fCenterY, fCenterZ);
	UpdateAroundAmbience(fCenterX, fCenterY, fCenterZ);
	m_SnowEnvironment.Update(D3DXVECTOR3(fCenterX, -fCenterY, fCenterZ));
#ifdef ENABLE_ENVIRONMENT_RAIN
	m_RainEnvironment.Update(D3DXVECTOR3(fCenterX, -fCenterY, fCenterZ));
#endif

	// Portal Process
	CMapOutdoor & rkMap = GetMapOutdoorRef();
	if (rkMap.IsEnablePortal())
	{
		CCullingManager & rkCullingMgr = CCullingManager::Instance();
		FGetPortalID kGetPortalID(fCenterX, -fCenterY);

		Vector3d aVector3d;
		aVector3d.Set(fCenterX, -fCenterY, fCenterZ);

		Vector3d toTop;
		toTop.Set(0, 0, 25000.0f);

		rkCullingMgr.ForInRay(aVector3d, toTop, &kGetPortalID);

		auto itor = kGetPortalID.m_kSet_iPortalID.begin();
		if (!__IsSame(kGetPortalID.m_kSet_iPortalID, m_kSet_iShowingPortalID))
		{
			ClearPortal();
			auto itor = kGetPortalID.m_kSet_iPortalID.begin();
			for (; itor != kGetPortalID.m_kSet_iPortalID.end(); ++itor)
				AddShowingPortalID(*itor);
			RefreshPortal();

			m_kSet_iShowingPortalID = kGetPortalID.m_kSet_iPortalID;
		}
	}

	// Target Effect Process
	{
		auto itor = m_kMap_dwTargetID_dwChrID.begin();
		for (; itor != m_kMap_dwTargetID_dwChrID.end(); ++itor)
		{
			uint32_t dwTargetID = itor->first;
			uint32_t dwChrID = itor->second;

			CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

			if (!pInstance)
				continue;

			TPixelPosition kPixelPosition;
			pInstance->NEW_GetPixelPosition(&kPixelPosition);

			CreateSpecialEffect(dwTargetID, +kPixelPosition.x, -kPixelPosition.y, +kPixelPosition.z, g_strEffectName.c_str());
		}
	}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	// Shop Pos Effect Process
	for (auto& id_dw_chr_id : m_kMapShop_dwTargetID_dwChrID)
	{
		uint32_t dwTargetID = id_dw_chr_id.first;
		uint32_t dwChrID = id_dw_chr_id.second;

		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

		if (!pInstance)
			continue;

		TPixelPosition kPixelPosition;
		pInstance->NEW_GetPixelPosition(&kPixelPosition);

		CreateSpecialShopPos(dwTargetID, +kPixelPosition.x, -kPixelPosition.y, +kPixelPosition.z, g_strEffectShopPos.c_str());
	}
#endif

	// Reserve Target Effect
	{
		auto itor = m_kMap_dwID_kReserveTargetEffect.begin();
		for (; itor != m_kMap_dwID_kReserveTargetEffect.end();)
		{
			uint32_t dwID = itor->first;
			SReserveTargetEffect & rReserveTargetEffect = itor->second;

			auto ilx = float(rReserveTargetEffect.ilx);
			auto ily = float(rReserveTargetEffect.ily);

			float fHeight = rkMap.GetHeight(ilx, ily);
			if (0.0f == fHeight)
			{
				++itor;
				continue;
			}

			CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());

			itor = m_kMap_dwID_kReserveTargetEffect.erase(itor);
		}
	}
}

bool CPythonBackground::__IsSame(std::set<int> & rleft, std::set<int> & rright) const
{
	for (auto & itor_l : rleft)
	{
		if (rright.end() == rright.find(itor_l))
			return false;
	}

	for (auto & itor_r : rright)
	{
		if (rleft.end() == rleft.find(itor_r))
			return false;
	}

	return true;
}

void CPythonBackground::Render()
{
	if (!IsMapReady())
		return;

	m_SnowEnvironment.Deform();
#ifdef ENABLE_ENVIRONMENT_RAIN
	m_RainEnvironment.Deform();
#endif

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.Render();
	if (m_bVisibleGuildArea)
		rkMap.RenderMarkedArea();
}

void CPythonBackground::RenderSnow()
{
	m_SnowEnvironment.Render();
}

#ifdef ENABLE_ENVIRONMENT_RAIN
void CPythonBackground::RenderRain()
{
	m_RainEnvironment.Render();
}
#endif

void CPythonBackground::RenderPCBlocker()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderPCBlocker();
}

void CPythonBackground::RenderCollision()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderCollision();
}

void CPythonBackground::RenderCharacterShadowToTexture()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	uint32_t t1 = ELTimer_GetMSec();

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	if (m_eShadowTargetLevel == SHADOW_ALL ||
		m_eShadowTargetLevel == SHADOW_GROUND_AND_SOLO ||
		m_eShadowTargetLevel == SHADOW_GROUND)
#else
	if (m_eShadowLevel == SHADOW_ALL ||
		m_eShadowLevel == SHADOW_ALL_HIGH ||
		m_eShadowLevel == SHADOW_ALL_MAX ||
		m_eShadowLevel == SHADOW_GROUND_AND_SOLO)
#endif
	{
		D3DXMATRIX matWorld;
		STATEMANAGER.GetTransform(D3DTS_WORLD, &matWorld);

		bool canRender = rkMap.BeginRenderCharacterShadowToTexture();
		if (canRender)
		{
			CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
			if (m_eShadowTargetLevel == SHADOW_GROUND_AND_SOLO)
#else
			if (m_eShadowLevel == SHADOW_GROUND_AND_SOLO)
#endif
				rkChrMgr.RenderShadowMainInstance();
			else
				rkChrMgr.RenderShadowAllInstances();
		}
		rkMap.EndRenderCharacterShadowToTexture();

		STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorld);
	}

	uint32_t t2 = ELTimer_GetMSec();

	m_dwRenderShadowTime = t2 - t1;
}

inline float Interpolate(float fStart, float fEnd, float fPercent)
{
	return fStart + (fEnd - fStart) * fPercent;
}
struct CollisionChecker
{
	bool isBlocked;
	CInstanceBase * pInstance;
	CollisionChecker(CInstanceBase * pInstance) : isBlocked(false), pInstance(pInstance) {}
	void operator()(CGraphicObjectInstance * pOpponent)
	{
		if (isBlocked)
			return;

		if (!pOpponent)
			return;

		if (pInstance->IsBlockObject(*pOpponent))
			isBlocked = true;
	}
};

struct CollisionAdjustChecker
{
	bool isBlocked;
	CInstanceBase * pInstance;
	CollisionAdjustChecker(CInstanceBase * pInstance) : isBlocked(false), pInstance(pInstance) {}
	void operator()(CGraphicObjectInstance * pOpponent)
	{
		if (!pOpponent)
			return;

		if (pInstance->AvoidObject(*pOpponent))
			isBlocked = true;
	}
};

bool CPythonBackground::CheckAdvancing(CInstanceBase * pInstance)
{
	if (!IsMapReady())
		return true;

	Vector3d center;
	float radius;
	pInstance->GetGraphicThingInstanceRef().GetBoundingSphere(center, radius);

	CCullingManager & rkCullingMgr = CCullingManager::Instance();

	CollisionAdjustChecker kCollisionAdjustChecker(pInstance);
	rkCullingMgr.ForInRange(center, radius, &kCollisionAdjustChecker);
	if (kCollisionAdjustChecker.isBlocked)
	{
		CollisionChecker kCollisionChecker(pInstance);
		rkCullingMgr.ForInRange(center, radius, &kCollisionChecker);
		if (kCollisionChecker.isBlocked)
		{
			pInstance->BlockMovement();
			return true;
		}
		pInstance->NEW_MoveToDestPixelPositionDirection(pInstance->NEW_GetDstPixelPositionRef());
		return false;
	}
	return false;
}

void CPythonBackground::RenderSky()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderSky();
}

void CPythonBackground::RenderCloud()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderCloud();
}

void CPythonBackground::RenderWater()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderWater();
}

void CPythonBackground::RenderEffect()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderEffect();
}

void CPythonBackground::RenderBeforeLensFlare()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderBeforeLensFlare();
}

void CPythonBackground::RenderAfterLensFlare()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RenderAfterLensFlare();
}

void CPythonBackground::ClearGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.ClearGuildArea();
}

void CPythonBackground::RegisterGuildArea(int isx, int isy, int iex, int iey)
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.RegisterGuildArea(isx, isy, iex, iey);
}

void CPythonBackground::SetCharacterDirLight()
{
	if (!IsMapReady())
		return;

	if (!mc_pcurEnvironmentData)
		return;

	STATEMANAGER.SetLight(0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_CHARACTER]);
}

void CPythonBackground::SetBackgroundDirLight()
{
	if (!IsMapReady())
		return;
	if (!mc_pcurEnvironmentData)
		return;

	STATEMANAGER.SetLight(0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND]);
}

void CPythonBackground::GlobalPositionToLocalPosition(LONG & rGlobalX, LONG & rGlobalY) const
{
	rGlobalX -= m_dwBaseX;
	rGlobalY -= m_dwBaseY;
}

void CPythonBackground::LocalPositionToGlobalPosition(LONG & rLocalX, LONG & rLocalY) const
{
	rLocalX += m_dwBaseX;
	rLocalY += m_dwBaseY;
}

void CPythonBackground::RegisterDungeonMapName(const char * c_szMapName)
{
	m_kSet_strDungeonMapName.emplace(c_szMapName);
}

CPythonBackground::TMapInfo * CPythonBackground::GlobalPositionToMapInfo(uint32_t dwGlobalX, uint32_t dwGlobalY)
{
	auto f = std::find_if(m_kVct_kMapInfo.begin(), m_kVct_kMapInfo.end(), FFindWarpMapName(dwGlobalX, dwGlobalY));
	if (f == m_kVct_kMapInfo.end())
		return nullptr;

	return &(*f);
}

void CPythonBackground::Warp(uint32_t dwX, uint32_t dwY)
{
	TMapInfo * pkMapInfo = GlobalPositionToMapInfo(dwX, dwY);
	if (!pkMapInfo)
	{
		TraceError("NOT_FOUND_GLOBAL_POSITION(%d, %d)", dwX, dwY);
		return;
	}

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	RefreshShadowTargetLevel();
	RefreshShadowQualityLevel();
#else
	RefreshShadowLevel();
#endif
	TMapInfo & rMapInfo = *pkMapInfo;
	assert((dwX >= rMapInfo.m_dwBaseX) && (dwY >= rMapInfo.m_dwBaseY));

	if (!LoadMap(rMapInfo.m_strName, float(dwX - rMapInfo.m_dwBaseX), float(dwY - rMapInfo.m_dwBaseY), 0))
	{
		// LOAD_MAP_ERROR_HANDLING
		PostQuitMessage(0);
		// END_OF_LOAD_MAP_ERROR_HANDLING
		return;
	}

	CPythonMiniMap::Instance().LoadAtlas();
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	CPythonDungeonInfo::Instance().Unload();
#endif

	m_dwBaseX = rMapInfo.m_dwBaseX;
	m_dwBaseY = rMapInfo.m_dwBaseY;

	m_strMapName = rMapInfo.m_strName;

	SetXMaxTree(m_iXMasTreeGrade);

#ifdef ENABLE_FOG_FIX
	RenderFogFix();
#endif

	if (m_kSet_strDungeonMapName.end() != m_kSet_strDungeonMapName.find(m_strMapName))
	{
		EnableTerrainOnlyForHeight();

		CMapOutdoor & rkMap = GetMapOutdoorRef();
		rkMap.EnablePortal(TRUE);
	}

	m_kSet_iShowingPortalID.clear();
	m_kMap_dwTargetID_dwChrID.clear();
	m_kMap_dwID_kReserveTargetEffect.clear();

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	if (IsXMasMap(m_strMapName.c_str()))
	{
		//if (IsXMasShowEvent())
		{
			if (IsSnowModeOption())
				EnableSnowEnvironment();
			else
				DisableSnowEnvironment();

			EnableSnowTextureMode(IsSnowTextureModeOption());
			CPythonMiniMap::Instance().ReloadAtlas(IsSnowTextureModeOption());
		}
	}
	else
		DisableSnowEnvironment();

	const TEnvironmentData* c_pEnvironmenData;
#ifdef ENABLE_WEATHER_INFO
	if (GetEnvironmentData(DAY_MODE_DARK, &c_pEnvironmenData))
	{
		RegisterEnvironmentData(DAY_MODE_DARK, "d:/ymir work/environment/cloudymonth.msenv");
		ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
#else
	if (IsNightModeOption() && IsBoomMap(m_strMapName.c_str()))
	{
		RegisterEnvironmentData(DAY_MODE_DARK, "d:/ymir work/environment/moonlight04.msenv");
		if (GetEnvironmentData(DAY_MODE_DARK, &c_pEnvironmenData))
			ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	else if (GetEnvironmentData(DAY_MODE_RED, &c_pEnvironmenData))
	{
		RegisterEnvironmentData(DAY_MODE_RED, "d:/ymir work/environment/metin2_map_n_flame_dragon_01.msenv");
		ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
#endif
#ifdef ENABLE_WEATHER_INFO
	else if (GetEnvironmentData(DAY_MODE_MORNING, &c_pEnvironmenData))
	{
		RegisterEnvironmentData(DAY_MODE_MORNING, "d:/ymir work/environment/sunset.msenv");
		ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
	else if (GetEnvironmentData(DAY_MODE_EVENING, &c_pEnvironmenData))
	{
		RegisterEnvironmentData(DAY_MODE_EVENING, "d:/ymir work/environment/eveningsun.msenv");
		ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
	else if (GetEnvironmentData(DAY_MODE_RAIN, &c_pEnvironmenData))
	{
		RegisterEnvironmentData(DAY_MODE_RAIN, "d:/ymir work/environment/rainyday.msenv");
		ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
#endif
	else
		SetEnvironmentData(DAY_MODE_LIGHT);
#endif
}

void CPythonBackground::VisibleGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.VisibleMarkedArea();

	m_bVisibleGuildArea = TRUE;
}

void CPythonBackground::DisableGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.DisableMarkedArea();

	m_bVisibleGuildArea = FALSE;
}

const char * CPythonBackground::GetWarpMapName() const
{
	return m_strMapName.c_str();
}

void CPythonBackground::ChangeToDay()
{
	m_iDayMode = DAY_MODE_LIGHT;
}

void CPythonBackground::ChangeToNight()
{
	m_iDayMode = DAY_MODE_DARK;
}

void CPythonBackground::EnableSnowEnvironment()
{
	m_SnowEnvironment.Enable();
}

void CPythonBackground::DisableSnowEnvironment()
{
	m_SnowEnvironment.Disable();
}

#ifdef ENABLE_ENVIRONMENT_RAIN
void CPythonBackground::EnableRainEnvironment()
{
	m_RainEnvironment.Enable();
}

void CPythonBackground::DisableRainEnvironment()
{
	m_RainEnvironment.Disable();
}
#endif

const D3DXVECTOR3 c_v3TreePos = D3DXVECTOR3(76500.0f, -60900.0f, 20215.0f);

void CPythonBackground::SetXMaxTree(int iGrade)
{
	if (!m_pkMap)
		return;

	assert(iGrade >= 0 && iGrade <= 3);
	m_iXMasTreeGrade = iGrade;

	CMapOutdoor & rkMap = GetMapOutdoorRef();

	if ("map_n_snowm_01" != m_strMapName)
	{
		rkMap.XMasTree_Destroy();
		return;
	}

	if (0 == iGrade)
	{
		rkMap.XMasTree_Destroy();
		return;
	}

	//////////////////////////////////////////////////////////////////////

	iGrade -= 1;
	iGrade = std::max(iGrade, 0);
	iGrade = std::min(iGrade, 2);

	static std::string s_strTreeName[3] = {"d:/ymir work/tree/christmastree1.spt", "d:/ymir work/tree/christmastree2.spt",
										   "d:/ymir work/tree/christmastree3.spt"};
	static std::string s_strEffectName[3] = {"d:/ymir work/effect/etc/christmas_tree/tree_1s.mse",
											 "d:/ymir work/effect/etc/christmas_tree/tree_2s.mse",
											 "d:/ymir work/effect/etc/christmas_tree/tree_3s.mse"};
	rkMap.XMasTree_Set(c_v3TreePos.x, c_v3TreePos.y, c_v3TreePos.z, s_strTreeName[iGrade].c_str(), s_strEffectName[iGrade].c_str());
}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
void CPythonBackground::CreatePrivateShopPos(uint32_t dwID, uint32_t dwChrVID)
{
	// TraceError("%d %d CreatePrivateShopPos", dwID, dwChrVID); // just for test
	m_kMapShop_dwTargetID_dwChrID.emplace(dwID, dwChrVID);
}
#endif

void CPythonBackground::CreateTargetEffect(uint32_t dwID, uint32_t dwChrVID)
{
	m_kMap_dwTargetID_dwChrID.emplace(dwID, dwChrVID);
}

void CPythonBackground::CreateTargetEffect(uint32_t dwID, long lx, long ly)
{
	if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
		return;

	CMapOutdoor & rkMap = GetMapOutdoorRef();

	uint32_t dwBaseX;
	uint32_t dwBaseY;
	rkMap.GetBaseXY(&dwBaseX, &dwBaseY);

	int ilx = +(lx - int(dwBaseX));
	int ily = -(ly - int(dwBaseY));

	float fHeight = rkMap.GetHeight(float(ilx), float(ily));

	if (0.0f == fHeight)
	{
		SReserveTargetEffect ReserveTargetEffect;
		ReserveTargetEffect.ilx = ilx;
		ReserveTargetEffect.ily = ily;
		m_kMap_dwID_kReserveTargetEffect.emplace(dwID, ReserveTargetEffect);
		return;
	}

	CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());
}

void CPythonBackground::DeleteTargetEffect(uint32_t dwID)
{
	if (m_kMap_dwID_kReserveTargetEffect.end() != m_kMap_dwID_kReserveTargetEffect.find(dwID))
		m_kMap_dwID_kReserveTargetEffect.erase(dwID);
	if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
		m_kMap_dwTargetID_dwChrID.erase(dwID);

	DeleteSpecialEffect(dwID);
}

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
void CPythonBackground::DeletePrivateShopPos(uint32_t dwID, uint32_t dwChrVID)
{
	if (m_kMapShop_dwTargetID_dwChrID.end() != m_kMapShop_dwTargetID_dwChrID.find(dwID))
	{
		m_kMapShop_dwTargetID_dwChrID.erase(dwID);
	}

	DeleteSpecialShopPos(dwID);
}

void CPythonBackground::CreateSpecialShopPos(uint32_t dwID, float fx, float fy, float fz, const char* c_szFileName)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SpecialEffectShopPos_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialShopPos(uint32_t dwID)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SpecialEffectShopPos_Delete(dwID);
}
#endif

void CPythonBackground::CreateSpecialEffect(uint32_t dwID, float fx, float fy, float fz, const char * c_szFileName)
{
	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.SpecialEffect_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialEffect(uint32_t dwID)
{
	CMapOutdoor & rkMap = GetMapOutdoorRef();
	rkMap.SpecialEffect_Delete(dwID);
}

#ifdef ENABLE_FOG_FIX
void CPythonBackground::RenderFogFix()
{
	if (!CPythonSystem::Instance().IsFogMode())
	{
		CPythonBackground::Instance().SetEnvironmentFog(false);
		CPythonSystem::Instance().SetFogMode(false);
	}
}
#endif

#ifdef ENABLE_BATTLE_FIELD
bool CPythonBackground::IsBattleFieldMap()
{
	return (!strcmp(GetWarpMapName(), "metin2_map_battlefied"));
}
#endif

#ifdef ENABLE_12ZI
bool CPythonBackground::IsReviveTargetMap()
{
	return (!strcmp(GetWarpMapName(), "metin2_12zi_stage"));
}
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
bool CPythonBackground::IsBoomMap(const char* c_szMapName)
{
	const char* WhiteList[39] = {
		"season1/metin2_map_sungzi_flame_hill_01",
		"season1/metin2_map_sungzi_flame_hill_02",
		"season1/metin2_map_sungzi_flame_hill_03",
		"season1/metin2_map_sungzi_desert_01",
		"season1/metin2_map_sungzi_desert_hill_01",
		"season1/metin2_map_sungzi_desert_hill_02",
		"season1/metin2_map_sungzi_desert_hill_03",
		"season2/metin2_map_empirewar03",
		"metin2_map_devilsCatacomb",
		"metin2_map_Mt_Thunder",
		"metin2_map_n_flame_dungeon_01",
		"metin2_map_n_snow_dungeon_01",
		"metin2_map_spiderdungeon_01",
		"metin2_map_spiderdungeon_02",
		"metin2_map_spiderdungeon_03",
		"metin2_map_deviltower1",
		"metin2_map_dawnmist_dungeon_01",
		"metin2_map_Mt_Th_dungeon_01",
		"metin2_12zi_stage",
		"metin2_map_n_flame_dragon",
		"metin2_map_defensewave",
		"metin2_map_defensewave_port",
		"metin2_map_miniboss_01",
		"metin2_map_miniboss_02",
		"metin2_map_labyrinth",
		"metin2_map_boss_crack_skipia",
		"metin2_map_boss_crack_flame",
		"metin2_map_boss_crack_snow",
		"metin2_map_boss_crack_dawnmist",
		"metin2_map_boss_awaken_skipia",
		"metin2_map_boss_awaken_flame",
		"metin2_map_boss_awaken_snow",
		"metin2_map_boss_awaken_dawnmist",
		"metin2_map_elemental_01",
		"metin2_map_elemental_02",
		"metin2_map_elemental_03",
		"metin2_map_elemental_04",
		"metin2_map_smhdungeon_01",
		"metin2_map_smhdungeon_02"
	};

	bool bRet = true;
	for (uint8_t i = 0; i < sizeof(WhiteList) / sizeof(WhiteList[0]); i++)
	{
		if (!strcmp(c_szMapName, WhiteList[i]))
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

bool CPythonBackground::IsXMasMap(const char* c_szMapName)
{
	const char* WhiteList[45] = {
		"metin2_map_n_flame_01",
		"metin2_map_n_desert_01",
		"season1/metin2_map_sungzi_flame_hill_01",
		"season1/metin2_map_sungzi_flame_hill_02",
		"season1/metin2_map_sungzi_flame_hill_03",
		"season1/metin2_map_sungzi_desert_01",
		"season1/metin2_map_sungzi_desert_hill_01",
		"season1/metin2_map_sungzi_desert_hill_02",
		"season1/metin2_map_sungzi_desert_hill_03",
		"season2/metin2_map_empirewar03",
		"metin2_map_devilsCatacomb",
		"metin2_map_Mt_Thunder",
		"metin2_map_n_flame_dungeon_01",
		"metin2_map_spiderdungeon_01",
		"metin2_map_spiderdungeon_02",
		"metin2_map_spiderdungeon_03",
		"metin2_map_deviltower1",
		"metin2_map_dawnmist_dungeon_01",
		"metin2_map_Mt_Th_dungeon_01",
		"metin2_map_n_flame_dragon",
		"metin2_map_defensewave",
		"metin2_map_defensewave_port",
		"metin2_map_mists_of_island",
		"metin2_map_miniboss_01",
		"metin2_map_miniboss_02",
		"metin2_map_labyrinth",
		"metin2_map_boss_crack_skipia",
		"metin2_map_boss_crack_flame",
		"metin2_map_boss_crack_snow",
		"metin2_map_boss_crack_dawnmist",
		"metin2_map_boss_awaken_skipia",
		"metin2_map_boss_awaken_flame",
		"metin2_map_boss_awaken_snow",
		"metin2_map_boss_awaken_dawnmist",
		"metin2_guild_pve",
		"metin2_map_elemental_01",
		"metin2_map_elemental_02",
		"metin2_map_elemental_03",
		"metin2_map_elemental_04",
		"metin2_map_battleroyale",
		"metin2_map_maze_dungeon1",
		"metin2_map_maze_dungeon2",
		"metin2_map_maze_dungeon3",
		"metin2_map_smhdungeon_01",
		"metin2_map_smhdungeon_02"
	};

	bool bRet = true;
	for (uint8_t i = 0; i < sizeof(WhiteList) / sizeof(WhiteList[0]); i++)
	{
		if (!strcmp(c_szMapName, WhiteList[i]))
		{
			bRet = false;
			break;
		}
	}

	return bRet;
}

int CPythonBackground::GetDayMode()
{
	return m_iDayMode;
};

void CPythonBackground::SetXMasShowEvent(BOOL bXMasEvent)
{
	m_bXMasEvent = bXMasEvent;
}

BOOL CPythonBackground::IsXMasShowEvent()
{
	return m_bXMasEvent;
}

void CPythonBackground::SetNightModeOption(BOOL bNightMode)
{
	m_bNightMode = bNightMode;
}

BOOL CPythonBackground::IsNightModeOption()
{
	return m_bNightMode;
}

void CPythonBackground::SetSnowModeOption(BOOL bSnowMode, bool bStartup)
{
	m_bSnowMode = bSnowMode;
	if (!bStartup)
	{
		if (IsXMasMap(GetWarpMapName()))
		{
			if (bSnowMode)
				EnableSnowEnvironment();
			else
				DisableSnowEnvironment();
		}
		else
			m_bSnowMode = FALSE;
	}
}

BOOL CPythonBackground::IsSnowModeOption()
{
	return m_bSnowMode;
}

#ifdef ENABLE_ENVIRONMENT_RAIN
void CPythonBackground::SetRainModeOption(BOOL bRainMode, bool bStartup)
{
	m_bRainMode = bRainMode;
	if (!bStartup)
	{
		if (bRainMode)
			EnableRainEnvironment();
		else
			DisableRainEnvironment();
	}
}

BOOL CPythonBackground::IsRainModeOption()
{
	return m_bRainMode;
}
#endif

bool CPythonBackground::EnableSnowTextureMode(BOOL bSnowTextureMode)
{
	if (!m_pkMap)
	{
		m_bSnowTextureMode = bSnowTextureMode;
		return false;
	}

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SetSnowTextureMode(bSnowTextureMode);
	rkMap.ReloadSetting(bSnowTextureMode);
	rkMap.ReloadMinimapTexture(bSnowTextureMode);

	m_bSnowTextureMode = bSnowTextureMode;
	return true;
}

void CPythonBackground::SetSnowTextureModeOption(BOOL bSnowTextureMode)
{
	m_bSnowTextureMode = bSnowTextureMode;
}

BOOL CPythonBackground::IsSnowTextureModeOption()
{
	return m_bSnowTextureMode;
}
#endif
