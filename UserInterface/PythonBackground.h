// PythonBackground.h: interface for the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_)
#	define AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_

#	if _MSC_VER > 1000
#		pragma once
#	endif // _MSC_VER > 1000

#	include "../GameLib/MapManager.h"
#	include "../GameLib/TerrainDecal.h"
#	include "../GameLib/SnowEnvironment.h"
#ifdef ENABLE_ENVIRONMENT_RAIN
#	include "../GameLib/RainEnvironment.h"
#endif

class CInstanceBase;

class CPythonBackground : public CMapManager, public CSingleton<CPythonBackground>
{
public:
	enum
	{
		SHADOW_NONE,
		SHADOW_GROUND,
		SHADOW_GROUND_AND_SOLO,
		SHADOW_ALL,
#ifndef ENABLE_SHADOW_RENDER_QUALITY_OPTION
		SHADOW_ALL_HIGH,
		SHADOW_ALL_MAX
#endif
	};

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	enum EShadowQuality
	{
		SHADOW_BAD,
		SHADOW_AVERAGE,
		SHADOW_GOOD,
	};
#endif

	enum
	{
		DISTANCE0,
		DISTANCE1,
		DISTANCE2,
		DISTANCE3,
		DISTANCE4,
		NUM_DISTANCE_SET
	};

	enum
	{
		DAY_MODE_LIGHT,
		DAY_MODE_DARK,
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		DAY_MODE_RED,
#endif
#ifdef ENABLE_WEATHER_INFO
		DAY_MODE_MORNING,
		DAY_MODE_EVENING,
		DAY_MODE_RAIN,
#endif
	};

	typedef struct SVIEWDISTANCESET
	{
		float m_fFogStart;
		float m_fFogEnd;
		float m_fFarClip;
		D3DXVECTOR3 m_v3SkyBoxScale;
	} TVIEWDISTANCESET;

public:
	CPythonBackground();
	~CPythonBackground();
	CLASS_DELETE_COPYMOVE(CPythonBackground);

	void Initialize();

	void Destroy();
	void Create();

	void GlobalPositionToLocalPosition(LONG & rGlobalX, LONG & rGlobalY) const;
	void LocalPositionToGlobalPosition(LONG & rLocalX, LONG & rLocalY) const;

	void EnableTerrainOnlyForHeight();
	bool SetSplatLimit(int iSplatNum);
	bool SetVisiblePart(int eMapOutDoorPart, bool isVisible);
#ifndef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	bool SetShadowLevel(int eLevel);
	void RefreshShadowLevel();
#endif
	void SelectViewDistanceNum(int eNum);
	void SetViewDistanceSet(int eNum, float fFarClip);
	float GetFarClip();

	uint32_t GetRenderShadowTime() const;
	void GetDistanceSetInfo(int * peNum, float * pfStart, float * pfEnd, float * pfFarClip) const;

	bool GetPickingPoint(D3DXVECTOR3 * v3IntersectPt);
	bool GetPickingPointWithRay(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt);
	bool GetPickingPointWithRayOnlyTerrain(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt);
	BOOL GetLightDirection(D3DXVECTOR3 & rv3LightDirection) const;

	void Update(float fCenterX, float fCenterY, float fCenterZ);

	void CreateCharacterShadowTexture();
	void ReleaseCharacterShadowTexture();
	void Render();
	void RenderSnow();
#ifdef ENABLE_ENVIRONMENT_RAIN
	void RenderRain();
#endif
	void RenderPCBlocker();
	void RenderCollision();
	void RenderCharacterShadowToTexture();
	void RenderSky();
	void RenderCloud();
	void RenderWater();
	void RenderEffect();
	void RenderBeforeLensFlare();
	void RenderAfterLensFlare();

	bool CheckAdvancing(CInstanceBase * pInstance);

	void SetCharacterDirLight();
	void SetBackgroundDirLight();

	void ChangeToDay();
	void ChangeToNight();
	void EnableSnowEnvironment();
	void DisableSnowEnvironment();
	void SetXMaxTree(int iGrade);

#ifdef ENABLE_ENVIRONMENT_RAIN
	void EnableRainEnvironment();
	void DisableRainEnvironment();
#endif

	void ClearGuildArea();
	void RegisterGuildArea(int isx, int isy, int iex, int iey);

	void CreateTargetEffect(uint32_t dwID, uint32_t dwChrVID);
	void CreateTargetEffect(uint32_t dwID, long lx, long ly);
	void DeleteTargetEffect(uint32_t dwID);

	void CreateSpecialEffect(uint32_t dwID, float fx, float fy, float fz, const char * c_szFileName);
	void DeleteSpecialEffect(uint32_t dwID);

#ifdef ENABLE_BATTLE_FIELD
	bool IsBattleFieldMap();
#endif

#ifdef ENABLE_12ZI
	bool IsReviveTargetMap();
#endif

	void Warp(uint32_t dwX, uint32_t dwY);

	void VisibleGuildArea();
	void DisableGuildArea();

	void RegisterDungeonMapName(const char * c_szMapName);
	TMapInfo * GlobalPositionToMapInfo(uint32_t dwGlobalX, uint32_t dwGlobalY);
	const char * GetWarpMapName() const;

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	void CreatePrivateShopPos(uint32_t dwID, uint32_t dwChrVID);
	void DeletePrivateShopPos(uint32_t dwID, uint32_t dwChrVID);
	void CreateSpecialShopPos(uint32_t dwID, float fx, float fy, float fz, const char* c_szFileName);
	void DeleteSpecialShopPos(uint32_t dwID);
#endif

#ifdef ENABLE_FOG_FIX
	void RenderFogFix();
#endif

protected:
	void __CreateProperty();
	bool __IsSame(std::set<int> & rleft, std::set<int> & rright) const;

protected:
	std::string m_strMapName;

private:
	CSnowEnvironment m_SnowEnvironment;
#ifdef ENABLE_ENVIRONMENT_RAIN
	CRainEnvironment m_RainEnvironment;
#endif

	int m_iDayMode;
	int m_iXMasTreeGrade;

#ifndef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	int m_eShadowLevel;
#endif
	int m_eViewDistanceNum;

	BOOL m_bVisibleGuildArea;

	uint32_t m_dwRenderShadowTime;

	uint32_t m_dwBaseX;
	uint32_t m_dwBaseY;

	TVIEWDISTANCESET m_ViewDistanceSet[NUM_DISTANCE_SET];

	std::set<int> m_kSet_iShowingPortalID;
	std::set<std::string> m_kSet_strDungeonMapName;
	std::map<uint32_t, uint32_t> m_kMap_dwTargetID_dwChrID;
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	std::map<uint32_t, uint32_t> m_kMapShop_dwTargetID_dwChrID;
#endif

	struct SReserveTargetEffect
	{
		int ilx;
		int ily;
	};
	std::map<uint32_t, SReserveTargetEffect> m_kMap_dwID_kReserveTargetEffect;

	struct FFindWarpMapName
	{
		uint32_t m_dwX, m_dwY;

		FFindWarpMapName(uint32_t dwX, uint32_t dwY)
		{
			m_dwX = dwX;
			m_dwY = dwY;
		}
		bool operator()(TMapInfo & rMapInfo)
		{
			if (m_dwX < rMapInfo.m_dwBaseX || m_dwX >= rMapInfo.m_dwEndX || m_dwY < rMapInfo.m_dwBaseY || m_dwY >= rMapInfo.m_dwEndY)
				return false;
			return true;
		}
	};

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	private:
		BOOL m_bNightMode;
		BOOL m_bSnowMode;
#	ifdef ENABLE_ENVIRONMENT_RAIN
		BOOL m_bRainMode;
#	endif
		BOOL m_bSnowTextureMode;
		BOOL m_bXMasEvent;

	public:
		bool IsBoomMap(const char* c_szMapName);
		bool IsXMasMap(const char* c_szMapName);
		int GetDayMode();

		void SetXMasShowEvent(BOOL bXMasEvent);
		BOOL IsXMasShowEvent();

		void SetNightModeOption(BOOL bNightMode);
		BOOL IsNightModeOption();

		void SetSnowModeOption(BOOL bSnowMode, bool bStartup = false);
		BOOL IsSnowModeOption();

#	ifdef ENABLE_ENVIRONMENT_RAIN
		void SetRainModeOption(BOOL bRainMode, bool bStartup = false);
		BOOL IsRainModeOption();
#	endif

		bool EnableSnowTextureMode(BOOL bSnowTextureMode);
		void SetSnowTextureModeOption(BOOL bSnowTextureMode);
		BOOL IsSnowTextureModeOption();
#endif

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	private:
		int m_eShadowTargetLevel;
		int m_eShadowQualityLevel;

	public:
		float GetShadowDistance();
		bool SetShadowTargetLevel(int eLevel);
		void RefreshShadowTargetLevel();
		bool SetShadowQualityLevel(int eLevel);
		void RefreshShadowQualityLevel();
#endif
};

#endif // !defined(AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_)
