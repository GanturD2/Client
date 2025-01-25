#include "StdAfx.h"
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_RENDER_TARGET
#include "CRenderTarget.h"
#include "ResourceManager.h"

#include "../EterBase/CRC32.h"
#include "../EterPythonLib/PythonGraphic.h"

#include "../EterLib/Camera.h"
#include "../EterLib/CRenderTargetManager.h"

#include "../GameLib/GameType.h"
#include "../GameLib/MapType.h"
#include "../GameLib/ItemData.h"
#include "../GameLib/ActorInstance.h"
#include "../GameLib/ItemManager.h"

#include "../UserInterface/InstanceBase.h"

CRenderTarget::CRenderTarget(const uint32_t width, const uint32_t height) :
	m_pModel(nullptr),
	m_background(nullptr),
	m_modelRotation(0.0f),
	m_fEyeY(0.0f),
	m_fTargetY(0.0f),
	m_fTargetHeight(0.0f),
	m_fZoomY(0.0f),
	m_visible(false)
{
	auto pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, D3DFMT_X8R8G8B8, D3DFMT_D16))
	{
		delete pTex;
		TraceError("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture %dx%d", width, height);
		throw std::runtime_error("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture");
	}

	m_renderTargetTexture = std::unique_ptr<CGraphicRenderTargetTexture>(pTex);
}

CRenderTarget::~CRenderTarget()
{
	/*m_pModel = nullptr;
	m_background = nullptr;
	m_modelRotation = 0;
	m_fEyeY = 0.0f;
	m_fTargetY = 0.0f;
	m_fTargetHeight = 0.0f;
	m_fZoomY = 0.0f;
	m_visible = false;*/
}

void CRenderTarget::SetVisibility(bool isShow)
{
	m_visible = isShow;
}

void CRenderTarget::RenderTexture() const
{
	m_renderTargetTexture->Render();
}

void CRenderTarget::SetRenderingRect(RECT* rect) const
{
	m_renderTargetTexture->SetRenderingRect(rect);
}

void CRenderTarget::CreateTextures() const
{
	m_renderTargetTexture->CreateTextures();
}

void CRenderTarget::ReleaseTextures() const
{
	m_renderTargetTexture->ReleaseTextures();
}

void CRenderTarget::SelectModel(const uint32_t index)
{
	if (index == 0)
	{
		//delete m_pModel;
		m_pModel.reset();
		return;
	}

	CInstanceBase::SCreateData kCreateData{};
	kCreateData.m_bType = CActorInstance::TYPE_NPC; // Dynamic Type
	kCreateData.m_dwRace = index;
#ifdef ENABLE_RENDER_TARGET_EFFECT
	kCreateData.m_isRenderTarget = true;
#endif

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
			m_pModel.reset();

		return;
	}

	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0.0f, 0.0f, 0.0f));
#ifndef ENABLE_RENDER_TARGET_EFFECT
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
#endif
	m_modelRotation = 0.0f;

	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);

	CActorInstance& rkActor = m_pModel->GetGraphicThingInstanceRef();
	const float fHeight = rkActor.GetHeight();
	m_fEyeY = fHeight;
	m_fTargetY = 0.0f;
	m_fTargetHeight = m_fEyeY * 8.9f / 140.0f;
	m_fZoomY = 0.0f;

	auto& camera_manager = CCameraManager::Instance();
	camera_manager.SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(m_fEyeY / 2.0f);
	camera_manager.ResetToPreviousCamera();
}

void CRenderTarget::SelectModelNPC(const uint32_t index)
{
	if (index == 0)
	{
		//delete m_pModel;
		m_pModel.reset();
		return;
	}

	CInstanceBase::SCreateData kCreateData{};

	kCreateData.m_bType = CActorInstance::TYPE_NPC; // Dynamic Type
	kCreateData.m_dwRace = index;
#ifdef ENABLE_RENDER_TARGET_EFFECT
	kCreateData.m_isRenderTarget = true; 
#endif
	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
			m_pModel.reset();

		return;
	}

	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0, 0, 0));
#ifndef ENABLE_RENDER_TARGET_EFFECT
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
#endif	
	m_modelRotation = 0.0f;
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);
/*#ifdef ENABLE_SCALE_SYSTEM
	float scale = std::max(m_pModel->GetGraphicThingInstanceRef().GetWidth(), m_pModel->GetGraphicThingInstanceRef().GetHeight());

	if (scale > 190)
	{

		m_pModel->GetGraphicThingInstancePtr()->SetScaleWorld(190.0f / scale, 190.0f / scale, 190.0f / scale);
	}
#endif*/
	auto& camera_manager = CCameraManager::Instance();
	camera_manager.SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();
}

#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined (ENABLE_AURA_SYSTEM)
void CRenderTarget::SelectModelPC(const uint32_t race, uint32_t index, uint32_t weapon, uint32_t armor, uint32_t hair, uint32_t acce, uint32_t aura)
#else
void CRenderTarget::SelectModelPC(const uint32_t race, uint32_t index, uint32_t weapon, uint32_t armor, uint32_t hair)
#endif
{
	if (index == 0)
	{
		//delete m_pModel;
		m_pModel.reset();
		return;
	}

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(index, &pItemData))
		return;

	CInstanceBase::SCreateData kCreateData;
	ZeroMemory(&kCreateData, sizeof(kCreateData));
#ifdef ENABLE_RENDER_TARGET_EFFECT
	kCreateData.m_isRenderTarget = true;
#endif
	kCreateData.m_bType = CActorInstance::TYPE_PC;

	uint32_t result = RaceMatchesAntiflag(race, pItemData->GetIndex());

	kCreateData.m_dwRace = result;

	uint32_t itemType = pItemData->GetType();
	uint32_t itemSubType = pItemData->GetSubType();

	if (weapon && ItemMatchesRace(result, weapon) && (weapon != index)) {
		kCreateData.m_dwWeapon = weapon;
	}
	if (armor && ItemMatchesRace(result, armor) && (armor != index)) {
		kCreateData.m_dwArmor = armor;
	}
	if (hair && ItemMatchesRace(result, hair) && (hair != index)) {
		CItemData * pItemData_hair;
		if (CItemManager::Instance().GetItemDataPointer(hair, &pItemData_hair))
			kCreateData.m_dwHair = pItemData_hair->GetValue(3);
	}
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	if (acce && ItemMatchesRace(result, acce) && (acce != index)) {
		kCreateData.m_dwAcce = acce;
	}
#endif

#ifdef ENABLE_AURA_SYSTEM
	if (acce && ItemMatchesRace(result, aura) && (aura != index)) {
		kCreateData.m_dwAura = aura;
	}
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if ((CItemData::ITEM_TYPE_WEAPON == itemType) || (CItemData::ITEM_TYPE_COSTUME == itemType && CItemData::COSTUME_WEAPON == itemSubType))
		kCreateData.m_dwWeapon = index;
	else if ((CItemData::ITEM_TYPE_ARMOR == itemType && CItemData::ARMOR_BODY == itemSubType) || (CItemData::ITEM_TYPE_COSTUME == itemType && CItemData::COSTUME_BODY == itemSubType))
		kCreateData.m_dwArmor = index;
	else if ((CItemData::ITEM_TYPE_HAIR == itemType) || (CItemData::ITEM_TYPE_COSTUME == itemType && CItemData::COSTUME_HAIR == itemSubType))
		kCreateData.m_dwHair = pItemData->GetValue(3);
#else
	if (CItemData::ITEM_TYPE_WEAPON == itemType)
		kCreateData.m_dwWeapon = index;
	else if (CItemData::ITEM_TYPE_ARMOR == itemType && CItemData::ARMOR_BODY == itemSubType)
		kCreateData.m_dwArmor = index;
	else if ((CItemData::ITEM_TYPE_HAIR == itemType))
		kCreateData.m_dwHair = pItemData->GetValue(3);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	else if (CItemData::ITEM_TYPE_COSTUME == itemType && CItemData::COSTUME_ACCE == itemSubType)
		kCreateData.m_dwAcce = index;
#endif

#ifdef ENABLE_AURA_SYSTEM
	else if (CItemData::ITEM_TYPE_COSTUME == itemType && CItemData::COSTUME_AURA == itemSubType)
		kCreateData.m_dwAura = index;
#endif

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
			m_pModel.reset();

		return;
	}

	m_pModel = std::move(model);

	m_pModel->NEW_SetPixelPosition(TPixelPosition(0.0f, 0.0f, 0.0f));
#ifndef ENABLE_RENDER_TARGET_EFFECT
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
#endif
	m_modelRotation = 0.0f;

	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);

	CActorInstance& rkActor = m_pModel->GetGraphicThingInstanceRef();
	const float fHeight = rkActor.GetHeight();
	m_fEyeY = fHeight;
	m_fTargetY = 0.0f;
	m_fTargetHeight = m_fEyeY * 8.9f / 140.0f;
	m_fZoomY = 0.0f;

	auto& camera_manager = CCameraManager::Instance();
	camera_manager.SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(m_fEyeY / 2.0f);
	camera_manager.ResetToPreviousCamera();
}

uint32_t CRenderTarget::RaceMatchesAntiflag(uint32_t byRace, uint32_t dwItemVnum)
{
	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemVnum, &pItemData))
		return byRace;

	const auto byJob = static_cast<uint8_t>(RaceToJob(byRace));
	const auto bySex = static_cast<uint8_t>(RaceToSex(byRace));

	// Basic
	if ((byJob == NRaceData::JOB_WARRIOR) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WARRIOR)))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_WARRIOR_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_WARRIOR_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_WARRIOR_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_WARRIOR_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_WARRIOR_M : CRaceData::RACE_WARRIOR_W;
	}
	else if ((byJob == NRaceData::JOB_ASSASSIN) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_ASSASSIN)))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_ASSASSIN_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_ASSASSIN_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_ASSASSIN_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_ASSASSIN_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_ASSASSIN_M : CRaceData::RACE_ASSASSIN_W;
	}
	else if ((byJob == NRaceData::JOB_SURA) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SURA)))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SURA_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SURA_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SURA_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SURA_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_SURA_M : CRaceData::RACE_SURA_W;
	}
	else if ((byJob == NRaceData::JOB_SHAMAN) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SHAMAN)))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SHAMAN_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SHAMAN_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SHAMAN_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SHAMAN_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_SHAMAN_M : CRaceData::RACE_SHAMAN_W;
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if ((byJob == NRaceData::JOB_WOLFMAN) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WOLFMAN)))
	{
		return CRaceData::RACE_WOLFMAN_M;
	}
#endif

	// Failover
	if (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WARRIOR))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_WARRIOR_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_WARRIOR_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_WARRIOR_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_WARRIOR_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_WARRIOR_M : CRaceData::RACE_WARRIOR_W;
	}
	else if (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_ASSASSIN))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_ASSASSIN_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_ASSASSIN_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_ASSASSIN_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_ASSASSIN_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_ASSASSIN_M : CRaceData::RACE_ASSASSIN_W;
	}
	else if (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SURA))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SURA_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SURA_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SURA_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SURA_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_SURA_M : CRaceData::RACE_SURA_W;
	}
	else if (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SHAMAN))
	{
		if (bySex == NRaceData::SEX_MALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SHAMAN_M;

		if (bySex == NRaceData::SEX_FEMALE && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SHAMAN_W;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return CRaceData::RACE_SHAMAN_M;

		if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return CRaceData::RACE_SHAMAN_W;

		return (bySex == NRaceData::SEX_MALE) ? CRaceData::RACE_SHAMAN_M : CRaceData::RACE_SHAMAN_W;
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WOLFMAN))
	{
		return CRaceData::RACE_WOLFMAN_M;
	}
#endif

	return byRace;
}

bool CRenderTarget::ItemMatchesRace(uint32_t race, uint32_t vnum)
{
	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(vnum, &pItemData))
		return false;

	uint32_t Job = RaceToJob(race);
	uint32_t Sex = RaceToSex(race);	//male 1

	//basic
	if ((Job == NRaceData::JOB_WARRIOR) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WARRIOR)))
	{
		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return true;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return false;

		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return false;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return true;

		return true;
	}
	else if ((Job == NRaceData::JOB_ASSASSIN) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_ASSASSIN)))
	{
		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return true;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return false;

		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return false;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return true;

		return true;
	}
	else if ((Job == NRaceData::JOB_SURA) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SURA)))
	{
		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return true;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return false;

		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return false;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return true;

		return true;
	}
	else if ((Job == NRaceData::JOB_SHAMAN) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_SHAMAN)))
	{
		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return true;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_FEMALE))
			return false;

		if (Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return false;

		if (!Sex && pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_MALE))
			return true;

		return true;
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if ((Job == NRaceData::JOB_WOLFMAN) && (!pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_WOLFMAN)))
	{
		return true;
	}
#endif

	return false;
}

bool CRenderTarget::CreateBackground(const char* imgPath, uint32_t width, uint32_t height)
{
	if (m_background)
		return false;

	m_background = std::make_unique<CGraphicImageInstance>();

	const auto graphic_image = dynamic_cast<CGraphicImage*>(CResourceManager::Instance().GetResourcePointer(imgPath));
	if (!graphic_image)
	{
		m_background.reset();
		return false;
	}

	m_background->SetImagePointer(graphic_image);
#ifdef ENABLE_IMAGE_SCALE
	m_background->SetScale(static_cast<float>(width) / graphic_image->GetWidth(), static_cast<float>(height) / graphic_image->GetHeight());
#endif

	return true;
}

void CRenderTarget::RenderBackground() const
{
	if (!m_visible || !m_background)
		return;

	m_renderTargetTexture->SetRenderTarget();

	CGraphicRenderTargetTexture::Clear();
	CPythonGraphic::Instance().SetInterfaceRenderState();

	m_background->Render();
	m_renderTargetTexture->ResetRenderTarget();
}

void CRenderTarget::UpdateModel()
{
	if (!m_visible || !m_pModel)
		return;

	if (m_modelRotation < 360.0f)
		m_modelRotation += 1.0f;
	else
		m_modelRotation = 0.0f;

	m_pModel->SetRotation(m_modelRotation);
	m_pModel->Transform();
	CActorInstance& rkModelActor = m_pModel->GetGraphicThingInstanceRef();
	rkModelActor.RotationProcess();
}

void CRenderTarget::DeformModel() const
{
	if (!m_visible)
		return;

	if (m_pModel)
		m_pModel->Deform();
}

void CRenderTarget::RenderModel() const
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

	if (!m_background)
		m_renderTargetTexture->Clear();

	python_graphic.ClearDepthBuffer();

	const auto fov = python_graphic.GetFOV();
	const auto aspect = python_graphic.GetAspect();
	const auto near_y = python_graphic.GetNear();
	const auto far_y = python_graphic.GetFar();

	const auto width = static_cast<float>(rectRender.right - rectRender.left);
	const auto height = static_cast<float>(rectRender.bottom - rectRender.top);

	const BOOL bIsFog = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	state_manager.SetRenderState(D3DRS_FOGENABLE, FALSE);

	python_graphic.SetViewport(0.0f, 0.0f, width, height);
	python_graphic.PushState();

	CActorInstance& rkActor = m_pModel->GetGraphicThingInstanceRef();
	const float fActorHeight = rkActor.GetHeight();

	const D3DXVECTOR3 v3Eye(0.0f, -(m_fEyeY * 8.9f + m_fZoomY), 0.0f); // x, y z
	const D3DXVECTOR3 v3Target(0.0f, m_fTargetY, fActorHeight / 2); // x, y, z
	const D3DXVECTOR3 v3Up(0.0f, 0.0f, 1.0f);

	camera_manager.SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	camera_manager.GetCurrentCamera()->SetViewParams(v3Eye, v3Target, v3Up);

	python_graphic.UpdateViewMatrix();
	python_graphic.SetPerspective(10.0f, width / height, 100.0f, 15000.0f);

	m_pModel->Render();

	camera_manager.ResetToPreviousCamera();
	python_graphic.RestoreViewport();
	python_graphic.PopState();
	python_graphic.SetPerspective(fov, aspect, near_y, far_y);
	m_renderTargetTexture->ResetRenderTarget();
	state_manager.SetRenderState(D3DRS_FOGENABLE, bIsFog);
}

void CRenderTarget::SetZoom(bool bZoom) noexcept
{
	if (!m_pModel)
		return;

	if (bZoom)
	{
		m_fZoomY = m_fZoomY - m_fTargetHeight;
		const float v3 = -(m_fEyeY * 8.9f - m_fEyeY * 3.0f);
		m_fZoomY = fmax(v3, m_fZoomY);
	}
	else
	{
		m_fZoomY = m_fZoomY + m_fTargetHeight;
		const float v6 = 14000.0f - m_fEyeY * 8.9f;
		const float v7 = m_fEyeY * 8.9f + m_fEyeY * 5.0f;
		m_fZoomY = fmin(m_fZoomY, v6);
		m_fZoomY = fmin(m_fZoomY, v7);
	}
}

void CRenderTarget::SetMotion(uint32_t motion)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->Refresh(motion, true);
	m_pModel->SetLoopMotion(motion);
}

void CRenderTarget::SetRotation(bool rotation)
{
	m_modelRotation = rotation;
}

void CRenderTarget::ChangeArmor(uint32_t vnum)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->SetArmor(vnum);
}

void CRenderTarget::ChangeWeapon(uint32_t vnum)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->SetWeapon(vnum);
}

void CRenderTarget::ChangeHair(uint32_t vnum)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->SetHair(vnum);
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
void CRenderTarget::ChangeAcce(uint32_t vnum)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->SetAcce(vnum);
}
#endif

#ifdef ENABLE_AURA_SYSTEM
void CRenderTarget::ChangeAura(uint32_t vnum)
{
	if (!m_visible || !m_pModel)
		return;

	m_pModel->SetAura(vnum);
}
#endif

#endif
