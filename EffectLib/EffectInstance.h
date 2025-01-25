#pragma once

#include "../EterLib/GrpObjectInstance.h"
#include "../EterLib/Pool.h"
#include "../MilesLib/Type.h"

#include "EffectElementBaseInstance.h"
#include "EffectData.h"
#include "EffectMeshInstance.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"

class CEffectInstance : public CGraphicObjectInstance
{
public:
	using TEffectElementInstanceVector = std::vector<CEffectElementBaseInstance *>;

	enum
	{
		ID = EFFECT_OBJECT
	};

	int GetType() const override { return ID; }

	bool GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius) override;

	static void DestroySystem();

	static CEffectInstance * New();
	static void Delete(CEffectInstance * pkEftInst);

	static void ResetRenderingEffectCount();
	static int GetRenderingEffectCount();

public:
	CEffectInstance();
	~CEffectInstance() override;

	bool LessRenderOrder(CEffectInstance * pkEftInst) const;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void SetEffectDataPointer(CEffectData* pEffectData, uint32_t* dwSkillColor = nullptr, uint32_t EffectID = 0);
#else
	void SetEffectDataPointer(CEffectData * pEffectData);
#endif

	void Clear() override;
	BOOL isAlive() const;
	void SetActive();
	void SetDeactive();
	void SetGlobalMatrix(const D3DXMATRIX & c_rmatGlobal);
	void UpdateSound();
	void OnUpdate() override;
	void OnRender() override;
	void OnBlendRender() override {} // Not used
	void OnRenderToShadowMap() override {} // Not used
	void OnRenderShadow() override {} // Not used
	void OnRenderPCBlocker() override {} // Not used
#ifdef ENABLE_RENDER_TARGET_EFFECT
	void SetSpecialRender() noexcept;
	bool IsSpecialRender();
#endif

protected:
	void __Initialize();

	void __SetParticleData(CParticleSystemData * pData);
	void __SetMeshData(CEffectMeshScript * pMesh);
	void __SetLightData(CLightData * pData);

	void OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector) override {} // Not used
	void OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance) override {}

	bool OnGetObjectHeight(float fX, float fY, float * pfHeight) override { return false; }

protected:
	BOOL m_isAlive{};
	uint32_t m_dwFrame{};
	D3DXMATRIX m_matGlobal;

	CEffectData * m_pkEftData{};

	std::vector<CParticleSystemInstance *> m_ParticleInstanceVector;
	std::vector<CEffectMeshInstance *> m_MeshInstanceVector;
	std::vector<CLightInstance *> m_LightInstanceVector;

	NSound::TSoundInstanceVector * m_pSoundInstanceVector{};

	float m_fBoundingSphereRadius{};
	D3DXVECTOR3 m_v3BoundingSpherePosition;

	float m_fLastTime{};
#if defined(ENABLE_RENDER_TARGET_EFFECT) || defined(ENABLE_INGAME_WIKI)
	bool m_isSpecialRender;
#endif

public:
	static CDynamicPool<CEffectInstance> ms_kPool;
	static int ms_iRenderingEffectCount;

#ifdef ENABLE_INGAME_WIKI
public:
	void SetWikiIgnoreFrustum(bool flag)
	{
		m_isSpecialRender = flag;

		const FWikiEffectFrustum f2(m_isSpecialRender);
		std::for_each(m_ParticleInstanceVector.begin(), m_ParticleInstanceVector.end(), f2);
		std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(), f2);
		std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(), f2);
	}

protected:
	struct FWikiEffectFrustum
	{
		FWikiEffectFrustum(bool igno) noexcept : wikiIgnoreFrustum(igno) {}

		void operator () (CEffectElementBaseInstance* pInstance) noexcept
		{
			if (pInstance)
				pInstance->SetWikiIgnoreFrustum(wikiIgnoreFrustum);
		}

		bool wikiIgnoreFrustum;
	};
#endif

#ifdef ENABLE_SCALE_SYSTEM
protected:
	float m_fParticleScale;
	D3DXVECTOR3 m_v3MeshScale;

public:
	void SetParticleScale(float fParticleScale) { m_fParticleScale = fParticleScale; }
	float GetParticleScale() { return m_fParticleScale; }
	void SetMeshScale(D3DXVECTOR3 rv3MeshScale) { m_v3MeshScale = rv3MeshScale; }
	D3DXVECTOR3 GetMeshScale() { return m_v3MeshScale; }
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
protected:
	uint32_t m_dwEffectID;
	bool m_isShow;

public:
	void SetEffectID(uint32_t dwEffectID) { m_dwEffectID = dwEffectID; }
	uint32_t GetEffectID() const { return m_dwEffectID; }

	void Show() { m_isShow = true; }
	void Hide() { m_isShow = false; }
	bool IsShow() const { return m_isShow; }
#endif
#ifdef ENABLE_RENDERING_ONLY_IN_AREA
	const D3DXMATRIX& GetGlobalMatrix() const { return m_matGlobal; };
#endif
};
