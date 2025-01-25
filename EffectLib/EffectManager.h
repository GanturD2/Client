#pragma once

#include "EffectInstance.h"

class CEffectManager : public CScreen, public CSingleton<CEffectManager>
{
public:
	enum EEffectType
	{
		EFFECT_TYPE_NONE = 0,
		EFFECT_TYPE_PARTICLE = 1,
		EFFECT_TYPE_ANIMATION_TEXTURE = 2,
		EFFECT_TYPE_MESH = 3,
		EFFECT_TYPE_SIMPLE_LIGHT = 4,

		EFFECT_TYPE_MAX_NUM = 4
	};

	typedef std::map<uint32_t, CEffectData *> TEffectDataMap;
	typedef std::map<uint32_t, CEffectInstance *> TEffectInstanceMap;

public:
	CEffectManager();
	~CEffectManager() override;

	void Destroy();

	void UpdateSound();
	void Update();
	void Render();
#ifdef ENABLE_INGAME_WIKI
	void WikiModuleRenderOneEffect(uint32_t id);
#endif

	void GetInfo(std::string * pstInfo) const;

	bool IsAliveEffect(uint32_t dwInstanceIndex);

	// Register
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	BOOL RegisterEffect(const char* c_szFileName, bool isExistDelete = false, bool isNeedCache = false, const char* name = nullptr);
	BOOL RegisterEffect2(const char* c_szFileName, uint32_t* pdwRetCRC, bool isNeedCache = false, const char* name = nullptr);
#else
	BOOL RegisterEffect(const char * c_szFileName,bool isExistDelete=false,bool isNeedCache=false);
	BOOL RegisterEffect2(const char * c_szFileName, uint32_t* pdwRetCRC, bool isNeedCache=false);
#endif

	void DeleteAllInstances();

	// Usage
#ifdef ENABLE_SKILL_COLOR_SYSTEM
#	ifdef ENABLE_SCALE_SYSTEM
	int CreateEffect(uint32_t dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, float fParticleScale = 1.0f, uint32_t* dwSkillColor = nullptr);
	int CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, float fParticleScale = 1.0f);
	void CreateEffectInstance(uint32_t dwInstanceIndex, uint32_t dwID, float fParticleScale = 1.0f, const D3DXVECTOR3* c_pv3MeshScale = nullptr, uint32_t* dwSkillColor = nullptr);
#	else
	int CreateEffect(uint32_t dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, uint32_t* dwSkillColor = nullptr);
	int CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation);
	void CreateEffectInstance(uint32_t dwInstanceIndex, uint32_t dwID, uint32_t* dwSkillColor = nullptr);
#	endif
#else
#	ifdef ENABLE_SCALE_SYSTEM
	int CreateEffect(uint32_t dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, float fParticleScale = 1.0f);
	int CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, float fParticleScale = 1.0f);
	void CreateEffectInstance(uint32_t dwInstanceIndex, uint32_t dwID, float fParticleScale = 1.0f, const D3DXVECTOR3* c_pv3MeshScale = nullptr);
#	else
	int CreateEffect(uint32_t dwID, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation);
	int CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation);
	void CreateEffectInstance(uint32_t dwInstanceIndex, uint32_t dwID);
#	endif
#endif

	BOOL SelectEffectInstance(uint32_t dwInstanceIndex);
	bool DestroyEffectInstance(uint32_t dwInstanceIndex);
	void DeactiveEffectInstance(uint32_t dwInstanceIndex);

#ifdef ENABLE_RENDER_TARGET_EFFECT
	void SetSpecialRenderEffect(uint32_t dwID);
	void RenderSelectedEffect();
#endif

	void SetEffectTextures(uint32_t dwID, std::vector<std::string> textures);
	void SetEffectInstancePosition(const D3DXVECTOR3 & c_rv3Position) const;
	void SetEffectInstanceRotation(const D3DXVECTOR3 & c_rv3Rotation) const;
	void SetEffectInstanceGlobalMatrix(const D3DXMATRIX & c_rmatGlobal) const;

	void ShowEffect() const;
	void HideEffect() const;

	// Temporary function
	uint32_t GetRandomEffect();
	int GetEmptyIndex();
	bool GetEffectData(uint32_t dwID, CEffectData ** ppEffect);
	bool GetEffectData(uint32_t dwID, const CEffectData ** c_ppEffect);

	// Area에 직접 찍는 Effect용 함수... EffectInstance의 Pointer를 반환한다.
	// EffectManager 내부 EffectInstanceMap을 이용하지 않는다.
	void CreateUnsafeEffectInstance(uint32_t dwEffectDataID, CEffectInstance ** ppEffectInstance);
	static bool DestroyUnsafeEffectInstance(CEffectInstance * pEffectInstance);

protected:
	void __Initialize();

	void __DestroyEffectInstanceMap();
	void __DestroyEffectCacheMap();
	void __DestroyEffectDataMap();

protected:
	bool m_isDisableSortRendering{};
	TEffectDataMap m_kEftDataMap;
	TEffectInstanceMap m_kEftInstMap;
	TEffectInstanceMap m_kEftCacheMap;

	CEffectInstance * m_pSelectedEffectInstance{};

#ifdef ENABLE_GRAPHIC_ON_OFF
protected:
	uint32_t m_dwDropItemEffectID;

public:
	void SetDropItemEffectID(uint32_t dwDropItemEffectID) { m_dwDropItemEffectID = dwDropItemEffectID; }
#endif
};
