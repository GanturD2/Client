#include "StdAfx.h"

#ifdef ENABLE_ENVIRONMENT_RAIN
#include "RainEnvironment.h"

#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/ResourceManager.h"
#include "RainParticle.h"

void CRainEnvironment::Enable()
{
	if (!m_bRainEnable)
		Create();

	m_bRainEnable = TRUE;
}

void CRainEnvironment::Disable()
{
	m_bRainEnable = FALSE;
}

void CRainEnvironment::Update(const D3DXVECTOR3 & c_rv3Pos)
{
	if (!m_bRainEnable)
	{
		if (m_kVct_pkParticleRain.empty())
			return;
	}

	m_v3Center = c_rv3Pos;
}

void CRainEnvironment::Deform()
{
	if (!m_bRainEnable)
	{
		if (m_kVct_pkParticleRain.empty())
			return;
	}

	const D3DXVECTOR3 & c_rv3Pos = m_v3Center;

	static long s_lLastTime = CTimer::Instance().GetCurrentMillisecond();
	long lcurTime = CTimer::Instance().GetCurrentMillisecond();
	float fElapsedTime = float(lcurTime - s_lLastTime) / m_dwElapsedTime;
	s_lLastTime = lcurTime;

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	const D3DXVECTOR3 & c_rv3View = pCamera->GetView();

	D3DXVECTOR3 v3ChangedPos = c_rv3View * 3500.0f + c_rv3Pos;
	v3ChangedPos.z = c_rv3Pos.z;

	auto itor = m_kVct_pkParticleRain.begin();
	for (; itor != m_kVct_pkParticleRain.end();)
	{
		CRainParticle* pRain = *itor;
		pRain->Update(fElapsedTime, v3ChangedPos);

		if (!pRain->IsActivate())
		{
			CRainParticle::Delete(pRain);

			itor = m_kVct_pkParticleRain.erase(itor);
		}
		else
			++itor;
	}

	if (m_bRainEnable)
	{
		for (int p = 0; p < std::min(10u, m_dwParticleMaxNum - m_kVct_pkParticleRain.size()); ++p)
		{
			CRainParticle* pRainParticle = CRainParticle::New();
			pRainParticle->Init(v3ChangedPos);
			m_kVct_pkParticleRain.emplace_back(pRainParticle);
		}
	}
}

void CRainEnvironment::Render()
{
	if (!m_bRainEnable)
	{
		if (m_kVct_pkParticleRain.empty())
			return;
	}

	uint32_t dwParticleCount = std::min(m_dwParticleMaxNum, m_kVct_pkParticleRain.size());

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	const D3DXVECTOR3 & c_rv3Up = pCamera->GetUp();
	const D3DXVECTOR3 & c_rv3Cross = pCamera->GetCross();

	SParticleVertex * pv3Verticies;
	if (SUCCEEDED(m_pVB->Lock(0, sizeof(SParticleVertex) * dwParticleCount * 4, (void **) &pv3Verticies, D3DLOCK_DISCARD)))
	{
		int i = 0;
		auto itor = m_kVct_pkParticleRain.begin();
		for (; i < dwParticleCount && itor != m_kVct_pkParticleRain.end(); ++i, ++itor)
		{
			CRainParticle* pRain = *itor;
			pRain->SetCameraVertex(c_rv3Up, c_rv3Cross);
			pRain->GetVerticies(pv3Verticies[i * 4 + 0], pv3Verticies[i * 4 + 1], pv3Verticies[i * 4 + 2], pv3Verticies[i * 4 + 3]);
		}
		m_pVB->Unlock();
	}

	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTexture(1, nullptr);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_pImageInstance->GetGraphicImagePointer()->GetTextureReference().SetTextureStage(0);
	STATEMANAGER.SetIndices(m_pIB, 0);
	STATEMANAGER.SetStreamSource(0, m_pVB, sizeof(SParticleVertex));
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
	STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwParticleCount * 4, 0, dwParticleCount * 2);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

bool CRainEnvironment::__CreateGeometry()
{
	if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(sizeof(SParticleVertex) * m_dwParticleMaxNum * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_SYSTEMMEM, &m_pVB, NULL)))
		return false;

	if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(sizeof(uint16_t) * m_dwParticleMaxNum * 6, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIB, NULL)))
		return false;

	uint16_t * dstIndices;
	if (FAILED(m_pIB->Lock(0, sizeof(uint16_t) * m_dwParticleMaxNum * 6, (VOID **) &dstIndices, 0)))
		return false;

	const uint16_t c_awFillRectIndices[6] = {0, 2, 1, 2, 3, 1};
	for (int i = 0; i < m_dwParticleMaxNum; ++i)
	{
		for (int j = 0; j < 6; ++j)
			dstIndices[i * 6 + j] = i * 4 + c_awFillRectIndices[j];
	}

	m_pIB->Unlock();
	return true;
}

bool CRainEnvironment::Create()
{
	Destroy();

	if (!__CreateGeometry())
		return false;

	auto * pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/rain.dds"));
	m_pImageInstance = CGraphicImageInstance::New();
	m_pImageInstance->SetImagePointer(pImage);

	return true;
}

void CRainEnvironment::Destroy()
{
	SAFE_RELEASE(m_lpRainTexture);
	SAFE_RELEASE(m_lpRainRenderTargetSurface);
	SAFE_RELEASE(m_lpRainDepthSurface);
	SAFE_RELEASE(m_lpAccumTexture);
	SAFE_RELEASE(m_lpAccumRenderTargetSurface);
	SAFE_RELEASE(m_lpAccumDepthSurface);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);

	stl_wipe(m_kVct_pkParticleRain);
	CRainParticle::DestroyPool();

	if (m_pImageInstance)
	{
		CGraphicImageInstance::Delete(m_pImageInstance);
		m_pImageInstance = nullptr;
	}

	__Initialize();
}

void CRainEnvironment::__Initialize()
{
	m_bRainEnable = FALSE;
	m_lpRainTexture = nullptr;
	m_lpRainRenderTargetSurface = nullptr;
	m_lpRainDepthSurface = nullptr;
	m_lpAccumTexture = nullptr;
	m_lpAccumRenderTargetSurface = nullptr;
	m_lpAccumDepthSurface = nullptr;
	m_pVB = nullptr;
	m_pIB = nullptr;
	m_pImageInstance = nullptr;

	m_kVct_pkParticleRain.reserve(m_dwParticleMaxNum);
}

CRainEnvironment::CRainEnvironment()
{
	m_dwParticleMaxNum = 800000;	// Raindrops count [default: 3000]
	m_dwElapsedTime = 250.0f;		// Falling speed [default: 1000.0f]
	__Initialize();
}

CRainEnvironment::~CRainEnvironment()
{
	Destroy();
}
#endif