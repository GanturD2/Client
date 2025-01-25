#include "StdAfx.h"
#include "EffectElementBaseInstance.h"

bool CEffectElementBaseInstance::Update(float fElapsedTime)
{
	if (m_bStart)
	{
		m_fElapsedTime = fElapsedTime;
		m_fLocalTime += fElapsedTime;

		return OnUpdate(fElapsedTime);
	}
	m_fRemainingTime -= fElapsedTime;
	if (m_fRemainingTime <= 0.0f)
		m_bStart = true;
	return true;
}

void CEffectElementBaseInstance::Render()
{
	if (!m_bStart)
		return;

	assert(mc_pmatLocal);

	OnRender();
}

void CEffectElementBaseInstance::SetLocalMatrixPointer(const D3DXMATRIX * c_pMatrix)
{
	mc_pmatLocal = c_pMatrix;
}

void CEffectElementBaseInstance::SetDataPointer(CEffectElementBase * pElement)
{
	m_pBase = pElement;

	m_dwStartTime = CTimer::Instance().GetCurrentMillisecond();

	//////////////////////////////////////////////////////////////////////////
	//add by ipkn, start time management

	m_fRemainingTime = pElement->GetStartTime();
	if (m_fRemainingTime <= 0.0f)
		m_bStart = true;
	else
		m_bStart = false;

	//////////////////////////////////////////////////////////////////////////

	OnSetDataPointer(pElement);
}

#ifdef ENABLE_SCALE_SYSTEM
void CEffectElementBaseInstance::SetParticleScale(float fParticleScale)
{
	m_fParticleScale = fParticleScale;
}

void CEffectElementBaseInstance::SetMeshScale(D3DXVECTOR3 rv3MeshScale)
{
	m_v3MeshScale = rv3MeshScale;
}
#endif

bool CEffectElementBaseInstance::isActive() const
{
	return m_isActive;
}

void CEffectElementBaseInstance::SetActive()
{
	m_isActive = true;
}

void CEffectElementBaseInstance::SetDeactive()
{
	m_isActive = false;
}

void CEffectElementBaseInstance::Initialize()
{
	mc_pmatLocal = nullptr;

	m_isActive = true;

	m_fLocalTime = 0.0f;
	m_dwStartTime = 0;
	m_fElapsedTime = 0.0f;

	m_bStart = false;
	m_fRemainingTime = 0.0f;
#ifdef ENABLE_INGAME_WIKI
	m_isSpecialRender = false;
#endif
#ifdef ENABLE_SCALE_SYSTEM
	m_fParticleScale = 1.0f;
	m_v3MeshScale.x = m_v3MeshScale.y = m_v3MeshScale.z = 1.0f;
#endif
	OnInitialize();
}

void CEffectElementBaseInstance::Destroy()
{
	OnDestroy();
	Initialize();
}
