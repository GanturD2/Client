#pragma once

#ifdef ENABLE_ENVIRONMENT_RAIN
#include "../EterLib/GrpScreen.h"

class CRainParticle;

class CRainEnvironment : public CScreen
{
public:
	CRainEnvironment();
	virtual ~CRainEnvironment();

	bool Create();
	void Destroy();

	void Enable();
	void Disable();

	void Update(const D3DXVECTOR3 & c_rv3Pos);
	void Deform();
	void Render();

protected:
	void __Initialize();
	bool __CreateGeometry();

protected:
	LPDIRECT3DSURFACE9 m_lpOldSurface;
	LPDIRECT3DSURFACE9 m_lpOldDepthStencilSurface;

	LPDIRECT3DTEXTURE9 m_lpRainTexture;
	LPDIRECT3DSURFACE9 m_lpRainRenderTargetSurface;
	LPDIRECT3DSURFACE9 m_lpRainDepthSurface;

	LPDIRECT3DTEXTURE9 m_lpAccumTexture;
	LPDIRECT3DSURFACE9 m_lpAccumRenderTargetSurface;
	LPDIRECT3DSURFACE9 m_lpAccumDepthSurface;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	LPDIRECT3DINDEXBUFFER9 m_pIB;

	D3DXVECTOR3 m_v3Center;

	CGraphicImageInstance * m_pImageInstance;
	std::vector<CRainParticle *> m_kVct_pkParticleRain;

	uint32_t m_dwParticleMaxNum;
	float m_dwElapsedTime;

	BOOL m_bRainEnable;
};
#endif
