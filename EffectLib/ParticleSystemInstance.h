#pragma once

#include "EffectElementBaseInstance.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"

#include "../EterLib/GrpScreen.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/GrpImageInstance.h"
#include "EmitterProperty.h"

class CParticleSystemInstance : public CEffectElementBaseInstance
{
public:
	static void DestroySystem();

	static CParticleSystemInstance * New();
	static void Delete(CParticleSystemInstance * pkPSInst);

	static CDynamicPool<CParticleSystemInstance> ms_kPool;

public:
	template <typename T> void ForEachParticleRendering(T & FunObj)
	{
		for (uint32_t dwFrameIndex = 0; dwFrameIndex < m_kVct_pkImgInst.size(); dwFrameIndex++)
		{
			STATEMANAGER.SetTexture(0, m_kVct_pkImgInst[dwFrameIndex]->GetTextureReference().GetD3DTexture());
			TParticleInstanceList::iterator itor = m_ParticleInstanceListVector[dwFrameIndex].begin();
			for (; itor != m_ParticleInstanceListVector[dwFrameIndex].end(); ++itor)
			{
					if (!InFrustum(*itor)
#ifdef ENABLE_RENDER_TARGET_EFFECT
					&& !m_isSpecialRender
#endif
					)
					return;
				FunObj(*itor);
			}
		}
	}

	CParticleSystemInstance();
	~CParticleSystemInstance() override;

	void OnSetDataPointer(CEffectElementBase * pElement) override;

	void CreateParticles(float fElapsedTime);

	bool InFrustum(const CParticleInstance * pInstance) const
	{
		if (m_pParticleProperty->m_bAttachFlag)
			return CScreen::GetFrustum().ViewVolumeTest(Vector3d(pInstance->m_v3Position.x + mc_pmatLocal->_41,
																 pInstance->m_v3Position.y + mc_pmatLocal->_42,
																 pInstance->m_v3Position.z + mc_pmatLocal->_43),
														pInstance->GetRadiusApproximation()) != VS_OUTSIDE;
		return CScreen::GetFrustum().ViewVolumeTest(
				   Vector3d(pInstance->m_v3Position.x, pInstance->m_v3Position.y, pInstance->m_v3Position.z),
				   pInstance->GetRadiusApproximation()) != VS_OUTSIDE;
	}

	uint32_t GetEmissionCount() const;
#ifdef ENABLE_RENDER_TARGET_EFFECT
	bool IsSpecialRender() const; 
	void SetSpecialRender();
#endif

protected:
	void OnInitialize() override;
	void OnDestroy() override;

	bool OnUpdate(float fElapsedTime) override;
	void OnRender() override;

protected:
	float m_fEmissionResidue{};

	uint32_t m_dwCurrentEmissionCount{};
	int m_iLoopCount{};

	using TParticleInstanceList = std::list<CParticleInstance *>;
	using TParticleInstanceListVector = std::vector<TParticleInstanceList>;
	TParticleInstanceListVector m_ParticleInstanceListVector;

	using TImageInstanceVector = std::vector<CGraphicImageInstance *>;
	TImageInstanceVector m_kVct_pkImgInst;

	CParticleSystemData * m_pData{};

	CParticleProperty * m_pParticleProperty{};
	CEmitterProperty * m_pEmitterProperty{};
#ifdef ENABLE_RENDER_TARGET_EFFECT
	bool m_isSpecialRender; 
#endif
};
