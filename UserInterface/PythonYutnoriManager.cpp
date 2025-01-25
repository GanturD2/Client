#include "StdAfx.h"

#ifdef ENABLE_MINI_GAME_YUTNORI
#include "PythonApplication.h"
#include "PythonYutnoriManager.h"
#include "../eterLib/Camera.h"
#include "../eterLib/GrpRenderTargetTexture.h"
#include "../eterLib/CRenderTargetManager.h"
#include "../eterPythonLib/PythonGraphic.h"
#include "../gameLib/ActorInstance.h"

CPythonYutnoriManager::CPythonYutnoriManager():
	m_pModelInstance(NULL),
	m_fModelHeight(0.0f),
	m_bShow(false),
	m_bIsDeleting(false),
	m_fStartDelete(0.0f),
	m_fAlphaValue(0.0f)
{
}

CPythonYutnoriManager::~CPythonYutnoriManager()
{
	Destroy();
}

void CPythonYutnoriManager::Initialize()
{
	m_pModelInstance = NULL;
	m_fModelHeight = 0.0f;
	m_bShow = false;
	
	m_bIsDeleting = false;
	m_fStartDelete = 0.0f;
	m_fAlphaValue = 0.0f;
}

void CPythonYutnoriManager::Destroy()
{
	m_bShow = false;
	
	m_bIsDeleting = false;
	m_fStartDelete = 0.0f;
	m_fAlphaValue = 0.0f;
	
	m_fModelHeight = 0.0f;

	if(m_pModelInstance)
	{
		CInstanceBase::Delete(m_pModelInstance);
	}
	
	m_pModelInstance = NULL;
}

void CPythonYutnoriManager::ClearInstances()
{
	if(m_pModelInstance)
	{
		CInstanceBase::Delete(m_pModelInstance);
	}
	
	m_bShow = false;
	
	m_bIsDeleting = false;
	m_fStartDelete = 0.0f;
	m_fAlphaValue = 0.0f;
	
	m_pModelInstance = NULL;
}

void CPythonYutnoriManager::SetShow(bool bShow)
{
	m_bShow = bShow;
	
	CRenderTargetManager::Instance().SetDiffuseColor(CPythonApplication::RENDER_TARGET_YUTNORI, 1.0f, 1.0f, 1.0f, 1.0f);
	m_fAlphaValue = 1.0f;
}

void CPythonYutnoriManager::ChangeMotion(uint8_t bMotion)
{
	if(bMotion < 0 || bMotion > 5)
		return;
	
	uint8_t bRealMotion = 0;
	
	switch(bMotion)
	{
		case 0:
			bRealMotion = CRaceMotionData::NAME_WAIT;
			break;
			
		case 1:
			bRealMotion = CRaceMotionData::NAME_STAND_UP;
			break;

		case 2:
			bRealMotion = CRaceMotionData::NAME_DEAD;
			break;

		case 3:
			bRealMotion = CRaceMotionData::NAME_NORMAL_ATTACK;
			break;

		case 4:
			bRealMotion = CRaceMotionData::NAME_SPAWN;
			break;	

		case 5:
			bRealMotion = CRaceMotionData::NAME_SPECIAL_1;
			break;
	}
	
	if(m_pModelInstance)
		m_pModelInstance->Refresh(bRealMotion, false);
	
	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();
	m_bIsDeleting = true;
	m_fStartDelete = rApp.GetGlobalTime();
	m_fAlphaValue = 1.0f;
}

bool CPythonYutnoriManager::CreateModelInstance()
{
	if(m_pModelInstance)
		return true;

	CInstanceBase::SCreateData kCreateData;
	memset(&kCreateData, 0, sizeof(kCreateData));
	
	kCreateData.m_bType = CActorInstance::TYPE_OBJECT;
	kCreateData.m_dwRace = 20505;

	CInstanceBase * pModel = CInstanceBase::New();
	if (!pModel->Create(kCreateData))
	{
		if (pModel)
			CInstanceBase::Delete(pModel);
		return false;
	}

	m_pModelInstance = pModel;
	
	if (!m_pModelInstance)
	{
		TraceError("CPythonYutnoriManager::CreateModelInstance failed.");
		return false;
	}
	
	m_pModelInstance->Refresh(CRaceMotionData::NAME_WAIT, false);
	m_pModelInstance->SetLODLimits(0, 100.0f);
	m_pModelInstance->SetAlwaysRender(true);
	m_pModelInstance->SetRotation(0.0f);
	m_pModelInstance->NEW_SetPixelPosition(TPixelPosition(0.0f, 0.0f, 0.0f));

	m_fModelHeight = m_pModelInstance->GetGraphicThingInstanceRef().GetHeight();

	CCameraManager::Instance().SetCurrentCamera(CCameraManager::YUTNORI_CAMERA);
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	pCam->SetTargetHeight(m_fModelHeight / 2.0f);
	CCameraManager::Instance().ResetToPreviousCamera();

	return true;
}

void CPythonYutnoriManager::RenderBackground()
{
	if (!m_bShow)
		return;

	RECT rectRender;
	if (!CRenderTargetManager::Instance().GetRenderTargetRect(CPythonApplication::RENDER_TARGET_YUTNORI, rectRender))
		return;

	if (!CRenderTargetManager::Instance().ChangeRenderTarget(CPythonApplication::RENDER_TARGET_YUTNORI))
		return;

	CRenderTargetManager::Instance().ClearRenderTarget();
	CPythonGraphic::Instance().SetInterfaceRenderState();
	
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.7f);
	CPythonGraphic::Instance().RenderBar2d(0.0f, 0.0f, (float)CPythonApplication::Instance().GetWidth(), (float)CPythonApplication::Instance().GetHeight());

	CRenderTargetManager::Instance().ResetRenderTarget();
}

void CPythonYutnoriManager::DeformModel()
{
	if (!m_bShow)
		return;

	if (m_pModelInstance)
		m_pModelInstance->Deform();
}

void CPythonYutnoriManager::UpdateModel()
{
	if (!m_bShow)
		return;

	if (!m_pModelInstance)
		return;

	m_pModelInstance->Transform();
	CActorInstance& rkModelActor = m_pModelInstance->GetGraphicThingInstanceRef();
	rkModelActor.RotationProcess();
	
	UpdateDeleting();
}

void CPythonYutnoriManager::UpdateDeleting()
{
	if(!m_bIsDeleting)
		return;
	
	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();
	
	float fcurTime = rApp.GetGlobalTime();
	float fElapsedTime = (fcurTime - m_fStartDelete);
	
	if(fElapsedTime >= 1.0f)
	{
		float fAlpha = m_fAlphaValue - (rApp.GetGlobalElapsedTime() * 0.75f);
	
		CRenderTargetManager::Instance().SetDiffuseColor(CPythonApplication::RENDER_TARGET_YUTNORI, 1.0f, 1.0f, 1.0f, fAlpha);
		m_fAlphaValue = fAlpha;
	
		if (fAlpha < 0.0f)
		{
			m_bShow = false;
			
			m_bIsDeleting = false;
			m_fAlphaValue = 0.0f;
			
			CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
			rkNetStream.RecvMiniGameYutNoriSetYut();
		}
	}
}

void CPythonYutnoriManager::RenderModel()
{
	if (!m_bShow)
		return;

	RECT rectRender;
	if (!CRenderTargetManager::Instance().GetRenderTargetRect(CPythonApplication::RENDER_TARGET_YUTNORI, rectRender))
		return;

	if (!CRenderTargetManager::Instance().ChangeRenderTarget(CPythonApplication::RENDER_TARGET_YUTNORI))
		return;

	if (!m_pModelInstance)
	{
		CRenderTargetManager::Instance().ResetRenderTarget();
		return;
	}

	CPythonGraphic::Instance().ClearDepthBuffer();

	float fFov = CPythonGraphic::Instance().GetFOV();
	float fAspect = CPythonGraphic::Instance().GetAspect();
	float fNearY = CPythonGraphic::Instance().GetNear();
	float fFarY = CPythonGraphic::Instance().GetFar();

	float fWidth = static_cast<float>(rectRender.right) - static_cast<float>(rectRender.left);
	float fHeight = static_cast<float>(rectRender.bottom) - static_cast<float>(rectRender.top);

	BOOL bIsFog = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, 0);
	
	CCameraManager::Instance().SetCurrentCamera(CCameraManager::YUTNORI_CAMERA);
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	if (!pCam)
	{
		CRenderTargetManager::Instance().ResetRenderTarget();
		return;
	}
	
	CPythonGraphic::Instance().PushState();

	D3DXVECTOR3 v3Eye(0.0f, 0.0f, 650.0f);
	D3DXVECTOR3 v3Target (0.0f, 5.0f, 0.0f);
	D3DXVECTOR3 v3Up (0.0f, 1.0f, 0.0f);

	pCam->SetViewParams(v3Eye, v3Target, v3Up);
	pCam->Move(D3DXVECTOR3(pCam->GetTarget().x, pCam->GetTarget().y, pCam->GetTarget().y + pCam->GetTargetHeight()));

	CPythonGraphic::Instance().UpdateViewMatrix();
	CPythonGraphic::Instance().SetPerspective(10.0f, fWidth / fHeight, 100.0f, 15000.0f);

	m_pModelInstance->Render();
	
	CCameraManager::Instance().ResetToPreviousCamera();
	CPythonGraphic::Instance().PopState();
	CPythonGraphic::Instance().SetPerspective(fFov, fAspect, fNearY, fFarY);
	CRenderTargetManager::Instance().ResetRenderTarget();
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, bIsFog);
}
#endif
