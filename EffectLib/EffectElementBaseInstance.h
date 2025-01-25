#pragma once

#include "EffectElementBase.h"

class CEffectElementBaseInstance
{
public:
	CEffectElementBaseInstance() = default;
	virtual ~CEffectElementBaseInstance() = default;

	void SetDataPointer(CEffectElementBase * pElement);

	void Initialize();
	void Destroy();

	void SetLocalMatrixPointer(const D3DXMATRIX * c_pMatrix);
	bool Update(float fElapsedTime);
	void Render();

	bool isActive() const;
	void SetActive();
	void SetDeactive();

#ifdef ENABLE_INGAME_WIKI
public:
	void SetWikiIgnoreFrustum(bool flag) { m_isSpecialRender = flag; }

protected:
	bool m_isSpecialRender;
#endif

protected:
	virtual void OnSetDataPointer(CEffectElementBase * pElement) = 0;

	virtual void OnInitialize() = 0;
	virtual void OnDestroy() = 0;

	virtual bool OnUpdate(float fElapsedTime) = 0;
	virtual void OnRender() = 0;

protected:
	const D3DXMATRIX * mc_pmatLocal{};

	bool m_isActive{};

	float m_fLocalTime{};
	uint32_t m_dwStartTime{};
	float m_fElapsedTime{};
	float m_fRemainingTime{};
	bool m_bStart{};

private:
	CEffectElementBase * m_pBase{};

#ifdef ENABLE_SCALE_SYSTEM
public:
	void 		SetParticleScale(float fParticleScale);
	void 		SetMeshScale(D3DXVECTOR3 rv3MeshScale);

protected:
	float		m_fParticleScale;
	D3DXVECTOR3	m_v3MeshScale;
#endif
};
