#include "StdAfx.h"
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_INGAME_WIKI
#include "../EterLib/Camera.h"
#include "../EterLib/CWikiRenderTargetManager.h"
#include "../EterPythonLib/PythonGraphic.h"
#include "../EterBase/CRC32.h"
#include "../GameLib/GameType.h"
#include "../GameLib/MapType.h"
#include "../GameLib/ItemData.h"
#include "../GameLib/ActorInstance.h"
#include "../UserInterface/InstanceBase.h"

#include "ResourceManager.h"
#include "CWikiRenderTarget.h"

CWikiRenderTarget::~CWikiRenderTarget() {}
CWikiRenderTarget::CWikiRenderTarget(const uint32_t width, const uint32_t height) :
																		m_renderTargetTexture(nullptr),
																		m_pModel(nullptr),
																		m_modelRotation(0),
																		m_visible(false),
																		m_v3Eye(0.0f, 0.0f, 0.0f),
																		m_v3Target(0.0f, 0.0f, 0.0f),
																		m_v3Up(0.0f, 0.0f, 1.0f)
{
	auto pTex = new CGraphicWikiRenderTargetTexture;
	
	if (!pTex->Create(width, height, D3DFMT_X8R8G8B8, D3DFMT_D16)) {
		TraceError("CWikiRenderTarget::CWikiRenderTarget: Could not create CGraphicRenderTargetTexture %dx%d", width, height);
		delete pTex;
		
		throw std::runtime_error("CWikiRenderTarget::CWikiRenderTarget: Could not create CGraphicRenderTargetTexture");
	}
	
	m_renderTargetTexture = std::unique_ptr<CGraphicWikiRenderTargetTexture>(pTex);
}

/*----------------------------
--------PUBLIC CLASS FUNCTIONS
-----------------------------*/

void CWikiRenderTarget::CreateTextures() const
{
	if (!m_renderTargetTexture)
		return;
	
	m_renderTargetTexture->CreateTextures();
}

void CWikiRenderTarget::ReleaseTextures() const
{
	if (!m_renderTargetTexture)
		return;
	
	m_renderTargetTexture->ReleaseTextures();
}

void CWikiRenderTarget::RenderTexture() const
{
	if (!m_renderTargetTexture)
		return;
	
	m_renderTargetTexture->Render();
}

void CWikiRenderTarget::SetRenderingRect(RECT* rect) const
{
	if (!m_renderTargetTexture)
		return;
	
	m_renderTargetTexture->SetRenderingRect(rect);
}

void CWikiRenderTarget::SetRenderingBox(RECT* renderBox) const
{
	if (!m_renderTargetTexture)
		return;
	
	m_renderTargetTexture->SetRenderingBox(renderBox);
}

void CWikiRenderTarget::SelectModel(const uint32_t model_vnum)
{
	CInstanceBase::SCreateData kCreateData{};
	
	kCreateData.m_bType = CActorInstance::TYPE_PC;
	kCreateData.m_dwRace = model_vnum;
	
	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
			m_pModel.reset();
		return;
	}
	
	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0,0,0));
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetRotation(0.0f);
	
	float fTargetHeight = 110.0f;
	
	m_v3Eye.x = 0.0f;
	m_v3Eye.y = -1000.0f;
	m_v3Eye.z = 600.0f;
	
	m_v3Target.x = 0.0f;
	m_v3Target.y = 0.0f;
	m_v3Target.z = 95.0f;
	
	if (model_vnum > (NRaceData::JOB_MAX_NUM * 2))
	{
		auto ptrGraphicInstance = m_pModel->GetGraphicThingInstancePtr();
		if (ptrGraphicInstance) {
			float fRaceHeight = ptrGraphicInstance->GetHeight();
			fTargetHeight = fRaceHeight / 2.0f;
			
			m_v3Eye.y = -(fRaceHeight * 8.9f);
			m_v3Eye.z = 0.0f;
			
			m_v3Target.z = fRaceHeight/2.0f;
		}
	}
	
	auto& camera_manager = CCameraManager::Instance();
	camera_manager.SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(fTargetHeight);
	camera_manager.ResetToPreviousCamera();
	
	m_modelRotation = 0.0f;
}

void CWikiRenderTarget::UpdateModel()
{
	if (!m_visible || !m_pModel)
		return;
	
	if (m_modelRotation < 360.0f)
		m_modelRotation += 1.0f;
	else
		m_modelRotation = 0.0f;
	
	m_pModel->SetRotation(m_modelRotation);
	m_pModel->Transform();
	m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CWikiRenderTarget::DeformModel() const
{
	if (!m_visible)
		return;

	if (m_pModel)
		m_pModel->Deform();
}

void CWikiRenderTarget::RenderModel() const
{
	if (!m_visible)
		return;
	
	auto& python_graphic = CPythonGraphic::Instance();
	auto& camera_manager = CCameraManager::Instance();
	auto& state_manager = CStateManager::Instance();
	auto& rectRender = *m_renderTargetTexture->GetRenderingRect();
	
	if (!m_pModel)
		return;
	
	m_renderTargetTexture->SetRenderTarget();
	m_renderTargetTexture->Clear();
	
	python_graphic.ClearDepthBuffer();
	
	const auto fov = python_graphic.GetFOV();
	const auto aspect = python_graphic.GetAspect();
	const auto near_y = python_graphic.GetNear();
	const auto far_y = python_graphic.GetFar();
	
	const auto width = static_cast<float>(rectRender.right - rectRender.left);
	const auto height = static_cast<float>(rectRender.bottom - rectRender.top);
	
	state_manager.SetRenderState(D3DRS_FOGENABLE, FALSE);
	
	python_graphic.SetViewport(0.0f, 0.0f, width, height);
	python_graphic.PushState();
	
	camera_manager.SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	if (!camera_manager.GetCurrentCamera()->IsDraging())
		camera_manager.GetCurrentCamera()->SetViewParams(m_v3Eye, m_v3Target, m_v3Up);
	
	python_graphic.UpdateViewMatrix();
	python_graphic.SetPerspective(10.0f, width / height, 100.0f, 15000.0f);
	
	m_pModel->Render();
	m_pModel->GetGraphicThingInstanceRef().WikiRenderAllAttachingModuleEffect();
	
	camera_manager.ResetToPreviousCamera();
	
	python_graphic.RestoreViewport();
	python_graphic.PopState();
	python_graphic.SetPerspective(fov, aspect, near_y, far_y);
	
	m_renderTargetTexture->ResetRenderTarget();
	
	state_manager.SetRenderState(D3DRS_FOGENABLE, TRUE);
}

void CWikiRenderTarget::SetArmor(uint32_t vnum)
{
	if (!m_pModel)
		return;
	
	m_pModel->ChangeArmor(vnum);
}

void CWikiRenderTarget::SetWeapon(uint32_t vnum)
{
	if (!m_pModel)
		return;
	
	m_pModel->ChangeWeapon(vnum);
}

void CWikiRenderTarget::SetHair(uint32_t vnum)
{
	if (!m_pModel)
		return;
	
	m_pModel->ChangeHair(vnum);
}

void CWikiRenderTarget::SetModelV3Eye(float x, float y, float z)
{
	m_v3Eye.x = x;
	m_v3Eye.y = y;
	m_v3Eye.z = z;
}

void CWikiRenderTarget::SetModelV3Target(float x, float y, float z)
{
	m_v3Target.x = x;
	m_v3Target.y = y;
	m_v3Target.z = z;
}
#endif
