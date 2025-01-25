#include "StdAfx.h"
#include "../EterLib/Camera.h"
#include "../EterLib/AttributeInstance.h"
#include "../GameLib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"
#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#include "ProcessScanner.h"
#ifdef ENABLE_SWITCHBOT
#	include "PythonSwitchbot.h"
#endif

#if defined(EVENT_HANDLER_MASTER)
#include "EventHandler.h"
#endif

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd = 0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication()
{
	CTimer::Instance().UseCustomTime();
	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd = 0.0f;

	m_dwFaceAccCount = 0;
	m_dwFaceAccTime = 0;

	m_dwFaceSpdSum = 0;
	m_dwFaceSpdCount = 0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication() = default;

void CPythonApplication::GetMousePosition(POINT * ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog) const
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable = false;
	else
		m_isFrameSkipDisable = true;
}

void CPythonApplication::NotifyHack(const char * c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(uint32_t eInfo, std::string * pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort() const
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PostQuitMessage(0);
}

void CPythonApplication::Exit() const
{
	PostQuitMessage(0);
}

void CPythonApplication::RenderGame()
{
#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.RenderBackgrounds();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.RenderBackground();
#endif
	float fAspect = m_kWndMgr.GetAspect();
	float fFarClip = m_pyBackground.GetFarClip();

	m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);

	CCullingManager::Instance().Process();

	m_kChrMgr.Deform();
	m_kEftMgr.Update();
#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.DeformModels();
#endif
#ifdef ENABLE_INGAME_WIKI
	m_pyWikiModelViewManager.DeformModels();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.DeformModel();
#endif

	m_pyBackground.RenderCharacterShadowToTexture();

	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

/*#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.RenderBackgrounds();
#endif*/

	{
		long lx, ly;
		m_kWndMgr.GetMousePosition(lx, ly);
		m_pyGraphic.SetCursorPosition(lx, ly);
	}

	m_pyBackground.RenderSky();

	m_pyBackground.RenderBeforeLensFlare();

	m_pyBackground.RenderCloud();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();

	m_pyBackground.SetCharacterDirLight();
	m_kChrMgr.Render();
#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.RenderModels();
#endif
#ifdef ENABLE_INGAME_WIKI
	m_pyWikiModelViewManager.RenderModels();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.RenderModel();
#endif

	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	m_pyBackground.RenderSnow();
#ifdef ENABLE_ENVIRONMENT_RAIN
	m_pyBackground.RenderRain();
#endif
	m_pyBackground.RenderEffect();

	m_pyBackground.EndEnvironment();

	m_kEftMgr.Render();
	m_pyItem.Render();
	m_FlyingManager.Render();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();

	m_pyBackground.RenderAfterLensFlare();
}

void CPythonApplication::UpdateGame()
{
	uint32_t t1 = ELTimer_GetMSec();
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

	uint32_t t2 = ELTimer_GetMSec();

	//!@# Alt+Tab 중 SetTransfor 에서 튕김 현상 해결을 위해 - [levites]
	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

		s.SetPerspective(30.0f, fAspect, 100.0f, fFarClip);
		s.BuildViewFrustum();
	}

#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.UpdateModels();
#endif
#ifdef ENABLE_INGAME_WIKI
	m_pyWikiModelViewManager.UpdateModels();
#endif

	uint32_t t3 = ELTimer_GetMSec();
	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	uint32_t t4 = ELTimer_GetMSec();
	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	uint32_t t5 = ELTimer_GetMSec();
	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	uint32_t t6 = ELTimer_GetMSec();
	m_kChrMgr.Update();
	uint32_t t7 = ELTimer_GetMSec();
	m_kEftMgr.UpdateSound();

#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.UpdateModel();
#endif

	uint32_t t8 = ELTimer_GetMSec();
	m_FlyingManager.Update();
	uint32_t t9 = ELTimer_GetMSec();
	m_pyItem.Update(ptMouse);
	uint32_t t10 = ELTimer_GetMSec();
	m_pyPlayer.Update();
	uint32_t t11 = ELTimer_GetMSec();

	// NOTE : Update 동안 위치 값이 바뀌므로 다시 얻어 옵니다 - [levites]
	//        이 부분 때문에 메인 케릭터의 Sound가 이전 위치에서 플레이 되는 현상이 있었음.
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	uint32_t t12 = ELTimer_GetMSec();
}

void CPythonApplication::SkipRenderBuffering(uint32_t dwSleepMSec)
{
	m_dwBufSleepSkipTime = ELTimer_GetMSec() + dwSleepMSec;
}

bool CPythonApplication::Process()
{
	ELTimer_SetFrameMSec();

	uint32_t dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static uint32_t s_dwUpdateFrameCount = 0;
	static uint32_t s_dwRenderFrameCount = 0;
	static uint32_t s_dwFaceCount = 0;
	static uint32_t s_uiLoad = 0;
	static uint32_t s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS = s_dwUpdateFrameCount;
		m_dwRenderFPS = s_dwRenderFrameCount;
		m_dwLoad = s_uiLoad;

		m_dwFaceCount = s_dwFaceCount / std::max(1u, s_dwRenderFrameCount);

		s_dwCheckTime = ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;

#if defined(EVENT_HANDLER_MASTER)
		//EventHandler::Instance().DeleteProccess();
		EventHandler::Instance().Proccess();
#endif
	}

	// Update Time
	static BOOL s_bFrameSkip = false;
	static uint32_t s_uiNextFrameTime = ELTimer_GetMSec();

	CTimer & rkTimer = CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	uint32_t uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime; //17 - 1초당 60fps기준.

	uint32_t updatestart = ELTimer_GetMSec();
	// Network I/O
	m_pyNetworkStream.Process();

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);
	}
	//////////////////////
	//!@# Alt+Tab 중 SetTransfor 에서 튕김 현상 해결을 위해 - [levites]
	//if (m_isActivateWnd)
	__UpdateCamera();
	// Update Game Playing
	CResourceManager::Instance().Update();
	OnCameraUpdate();
	OnMouseUpdate();
	OnUIUpdate();

	//Update
	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	uint32_t dwCurrentTime = ELTimer_GetMSec();
	BOOL bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = (static_cast<float>(dt) / static_cast<float>(uiFrameTime)) * uiFrameTime;

		if (dt >= 500)
		{
			s_uiNextFrameTime += nAdjustTime;
			printf("FrameSkip Adjusting... %d\n", nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}

		if (!m_isFrameSkipDisable)
			s_bFrameSkip = true;

		bCurrentLateUpdate = TRUE;
	}

	if (m_isMinimizedWnd)
		CEffectManager::Instance().Update();

	if (m_isFrameSkipDisable && !m_isMinimizedWnd)
		s_bFrameSkip = false;

	if (!s_bFrameSkip)
	{
		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		uint32_t dwRenderStartTime = ELTimer_GetMSec();

		bool canRender = true;

		if (m_isMinimizedWnd)
			canRender = false;
		else
		{
			if (DEVICE_STATE_OK != CheckDeviceState())
				canRender = false;
		}

		if (!IsActive())
			SkipRenderBuffering(3000);

		if (!canRender)
			SkipRenderBuffering(3000);
		else
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin())
			{
				m_pyGraphic.ClearDepthBuffer();

#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();
				/////////////////////

				m_pyGraphic.End();

				m_pyGraphic.Show();

				uint32_t dwRenderEndTime = ELTimer_GetMSec();

				static uint32_t s_dwRenderCheckTime = dwRenderEndTime;
				static uint32_t s_dwRenderRangeTime = 0;
				static uint32_t s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;
				s_dwRenderRangeTime += m_dwCurRenderTime;
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime - s_dwRenderCheckTime > 1000)
				{
					m_fAveRenderTime = float(double(s_dwRenderRangeTime) / double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime = ELTimer_GetMSec();
					s_dwRenderRangeTime = 0;
					s_dwRenderRangeFrame = 0;
				}

				uint32_t dwCurFaceCount = m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						uint32_t dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{
							if (dwBufRenderTime > 58)
								dwBufRenderTime = 64;
							else if (dwBufRenderTime > 42)
								dwBufRenderTime = 48;
							else if (dwBufRenderTime > 26)
								dwBufRenderTime = 32;
							else if (dwBufRenderTime > 10)
								dwBufRenderTime = 16;
							else
								dwBufRenderTime = 8;
						}

						m_fAveRenderTime = s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd = (m_dwFaceAccCount / m_dwFaceAccTime);

					// 거리 자동 조절
					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio = 0.3f;
						s_fAveRenderTime =
							(s_fAveRenderTime * (100.0f - fRatio) + std::max<float>(16.0f, m_dwCurRenderTime) * fRatio) / 100.0f;


						float fFar = 25600.0f;
						float fNear = MIN_FOG;
						auto dbAvePow = double(1000.0f / s_fAveRenderTime);
						double dbMaxPow = 60.0;
						float fDistance = std::max<float>(fNear + (fFar - fNear) * (dbAvePow) / dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
					// 거리 강제 설정시
					else
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
				}
				else
				{
					// 10000 폴리곤 보다 적을때는 가장 멀리 보이게 한다
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	int rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate)
	{
		s_uiLoad -= rest; // 쉰 시간은 로드에서 뺀다..
		Sleep(rest);
	}

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject * poMouseHandler)
{
	m_poMouseHandler = poMouseHandler;
}

int32_t CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
		case CGraphicDevice::DEVICESTATE_NULL:
			return DEVICE_STATE_FALSE;

		case CGraphicDevice::DEVICESTATE_BROKEN:
			return DEVICE_STATE_SKIP;

		case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
			m_pyBackground.ReleaseCharacterShadowTexture();
#ifdef ENABLE_INGAME_WIKI
			CWikiRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif
			CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
			if (!m_grpDevice.Reset())
				return DEVICE_STATE_SKIP;

			break;

		default:
			break;
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
{
	m_grpDevice.InitBackBufferCount(2);
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_BAD_DRIVER,
									  ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER, "WARN_BAD_DRIVER"));
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_NO_TNL, ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL, "WARN_NO_TNL"));

	int iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit, frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		//PyErr_SetString(PyExc_RuntimeError, "DirectX 8.1 or greater required to run game");
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		//PyErr_SetString(PyExc_RuntimeError, "GraphicDevice create failed");
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
				CGrannyLODController::SetMinLODMode(true);
			return true;
		}

		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{
	while (true)
	{
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;

			m_dwLastIdleTime = ELTimer_GetMSec();
		}
	}
}

// SUPPORT_NEW_KOREA_SERVER
bool LoadLocaleData(const char * localePath)
{
	NANOBEGIN
	CPythonNonPlayer & rkNPCMgr = CPythonNonPlayer::Instance();
	CItemManager & rkItemMgr = CItemManager::Instance();
	CPythonSkill & rkSkillMgr = CPythonSkill::Instance();
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	CPythonSkillPet& rkSkillPetMgr	= CPythonSkillPet::Instance();
#endif
#ifdef ENABLE_CUBE_RENEWAL
	CPythonCubeRenewal& rkCube = CPythonCubeRenewal::Instance();
	char szCubeFileName[256];
#endif
#ifdef ENABLE_EVENT_MANAGER
	InGameEventManager& rkEventMgr = InGameEventManager::Instance();
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	CPythonLocale& rkLocale = CPythonLocale::Instance();
#endif
	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
#ifdef ENABLE_DS_SET
	char szDragonSoulTable[256];
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	char szSkillPetFileName[256];
#endif
#ifdef ENABLE_SCALE_SYSTEM
	char szItemScale[256];
#endif
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
#ifdef ENABLE_PARTY_MATCH
	char szPartyMatch[256];
#endif
#ifdef ENABLE_EVENT_MANAGER
	char szRewardList[256];
#endif

	//@custom016
	const char* localePathCommon = "locale/common";
#ifdef ENABLE_CUBE_RENEWAL
	snprintf(szCubeFileName, sizeof(szCubeFileName), "%s/cube.txt", localePathCommon);
#endif
	snprintf(szItemList, sizeof(szItemList), "%s/item_list.txt", localePathCommon);
#ifdef ENABLE_DS_SET
	snprintf(szDragonSoulTable, sizeof(szDragonSoulTable), "%s/dragon_soul_table.txt", localePathCommon);
#endif
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName), "%s/SkillTable.txt", localePathCommon);

	snprintf(szItemProto,	sizeof(szItemProto),	"%s/item_proto",	localePathCommon);
	snprintf(szItemDesc,	sizeof(szItemDesc),		"%s/itemdesc.txt",	localePath);
#ifdef ENABLE_GROWTH_PET_SYSTEM
	snprintf(szSkillPetFileName, sizeof(szSkillPetFileName), "%s/pet_skill.txt", localePath);
#endif
#ifdef ENABLE_SCALE_SYSTEM
	snprintf(szItemScale, sizeof(szItemScale), "%s/item_scale.txt", localePathCommon);
#endif
#ifdef ENABLE_EVENT_MANAGER
	snprintf(szRewardList, sizeof(szRewardList), "%s/event_reward_list.txt", localePathCommon);
#endif
	snprintf(szMobProto,	sizeof(szMobProto),	"%s/mob_proto",		localePathCommon);
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName), "%s/SkillDesc.txt", localePath);
	snprintf(szInsultList,	sizeof(szInsultList),	"%s/insult.txt", localePath);

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	rkSkillPetMgr.Destroy();
#endif

	if (!rkItemMgr.LoadItemList(szItemList))
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);

#ifdef ENABLE_CUBE_RENEWAL
	// snprintf(szCubeFileName, sizeof(szCubeFileName, "%s/cube.txt", localePathCommon);
	if (!rkCube.LoadFile(szCubeFileName))
	{
		TraceError("LoadLocaleData - LoadFile(%s) Error", szCubeFileName);
		return false;
	}
#endif

#ifdef ENABLE_DS_SET
	if (!rkItemMgr.LoadDragonSoulTable(szDragonSoulTable))
		Tracenf("LoadLocaleData - LoadDragonSoulTable(%s) Error", szDragonSoulTable);
#endif

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szMobProto);
		return false;
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (!rkSkillPetMgr.RegisterSkillPet(szSkillPetFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillPet(%s) Error", szSkillPetFileName);
		return false;
	}
#endif

#ifdef ENABLE_SCALE_SYSTEM
	if (!rkItemMgr.LoadItemScale(szItemScale))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemScale);
	}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	char szLocaleTextString[256];
	snprintf(szLocaleTextString, sizeof(szLocaleTextString), "%s/locale_string.txt", localePath);
	if (!rkLocale.LoadLocaleString(szLocaleTextString))
	{
		TraceError("LoadLocaleData - LoadLocaleString(%s) Error", szLocaleTextString);
		return false;
	}

	char szLocaleQuestString[256];
	snprintf(szLocaleQuestString, sizeof(szLocaleQuestString), "%s/locale_quest.txt", localePath);
	if (!rkLocale.LoadQuestLocaleString(szLocaleQuestString))
	{
		TraceError("LoadLocaleData - LoadQuestLocaleString(%s) Error", szLocaleQuestString);
		return false;
	}

	char szLocaleQuizString[256];
	snprintf(szLocaleQuizString, sizeof(szLocaleQuizString), "%s/locale_quiz.txt", localePath);
	if (!rkLocale.LoadOXQuizLocaleString(szLocaleQuizString))
	{
		TraceError("LoadLocaleData - LoadOXQuizLocaleString(%s) Error", szLocaleQuizString);
		return false;
	}
#endif

	if (!rkNetStream.LoadInsultList(szInsultList))
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);

	if (LocaleService_IsYMIR())
	{
		char szEmpireTextConvFile[256];
		for (uint32_t dwEmpireID = 1; dwEmpireID <= 3; ++dwEmpireID)
		{
			sprintf(szEmpireTextConvFile, "%s/lang%u.cvt", localePath, dwEmpireID);
			if (!rkNetStream.LoadConvertTable(dwEmpireID, szEmpireTextConvFile))
				TraceError("LoadLocaleData - CPythonNetworkStream::LoadConvertTable(%d, %s) FAILURE", dwEmpireID, szEmpireTextConvFile);
		}
	}

#ifdef ENABLE_LOADING_TIP
	char szLoadingTipList[256], szLoadingTipVnum[256];
	snprintf(szLoadingTipList, sizeof(szLoadingTipList), "%s/loading_tip_list.txt", localePathCommon);	//@custom016
	//snprintf(szLoadingTipList, sizeof(szLoadingTipList), "%s/loading_tip_list.txt", localePath);
	snprintf(szLoadingTipVnum, sizeof(szLoadingTipVnum), "%s/loading_tip_vnum.txt", localePath);
	if (!rkNetStream.LoadLoadingTipList(szLoadingTipList))
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadLoadingTipList(%s)", szLoadingTipList);

	if (!rkNetStream.LoadLoadingTipVnum(szLoadingTipVnum))
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadLoadingTipVnum(%s)", szLoadingTipVnum);
#endif

#ifdef ENABLE_PARTY_MATCH
	snprintf(szPartyMatch, sizeof(szPartyMatch), "%s/partymatch_info.txt", localePathCommon);
	rkNetStream.LoadPartyMatchInfo(szPartyMatch);
#endif

#ifdef ENABLE_EVENT_MANAGER
	if (!rkEventMgr.LoadEventRewardList(szRewardList))
	{
		TraceError("LoadLocaleData - LoadEventRewardList(%s) Error", szRewardList);
		return false;
	}
#endif

#ifdef ENABLE_SHINING_SYSTEM
	char szShiningTable[256];
	snprintf(szShiningTable, sizeof(szShiningTable), "%s/shiningtable.txt", localePathCommon);
	if (!rkItemMgr.LoadShiningTable(szShiningTable))
	{
		Tracenf("LoadLocaleData - LoadShiningTable(%s) Error", szShiningTable);
	}
#endif

	NANOEND
	return true;
}
// END_OF_SUPPORT_NEW_KOREA_SERVER

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject * poSelf, const char * c_szName, int width, int height, int Windowed)
{
	NANOBEGIN
	Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	if (FindWindow(nullptr, c_szName))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	uint32_t WindowMode = __GetWindowMode(Windowed ? true : false);
	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_METIN2)), IDC_CURSOR_NORMAL))
	{
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
		CPythonIME::Instance().UseDefaultIME();

	// 풀스크린 모드이고
	// 디폴트 IME 를 사용하거나 유럽 버전이면
	// 윈도우 풀스크린 모드를 사용한다
	if (!m_pySystem.IsWindowed() && (m_pySystem.IsUseDefaultIME() || LocaleService_IsEUROPE()))
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;

			if (bAnotherWindow)
			{
				RECT rc;

				GetClientRect(&rc);

				int windowWidth = rc.right - rc.left;
				int windowHeight = (rc.bottom - rc.top);

				SetPosition(GetScreenWidth() - windowWidth, GetScreenHeight() - 60 - windowHeight);
			}
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

	NANOEND
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Cursor
	if (!CreateCursors())
	{
		TraceError("CMSWindow::Cursors Create Error");
		SET_EXCEPTION("CREATE_CURSOR");
		return false;
	}

	if (!m_pySystem.IsNoSoundCard())
	{
		// Sound
		if (!m_SoundManager.Create())
		{
			// NOTE : 중국측의 요청으로 생략
			//		LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_SOUND_DEVICE));
		}
	}

	extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

	if (!m_pySystem.IsAutoTiling())
		GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();

	// Device
	if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
		return false;

	GrannyCreateSharedDeformBuffer();

	if (m_pySystem.IsAutoTiling())
	{
		if (m_grpDevice.IsFastTNL())
			m_pyBackground.ReserveSoftwareTilingEnable(false);
		else
			m_pyBackground.ReserveSoftwareTilingEnable(true);
	}
	else
		m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());

	SetVisibleMode(true);

	if (m_isWindowFullScreenEnable) //m_pySystem.IsUseDefaultIME() && !m_pySystem.IsWindowed())
		SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);

	if (!InitializeKeyboard(GetWindowHandle()))
		return false;

	m_pySystem.GetDisplaySettings();

	// Mouse
	if (m_pySystem.IsSoftwareCursor())
		SetCursorMode(CURSOR_MODE_SOFTWARE);
	else
		SetCursorMode(CURSOR_MODE_HARDWARE);

	// Network
	if (!m_netDevice.Create())
	{
		TraceError("NetDevice::Create failed");
		SET_EXCEPTION("CREATE_NETWORK");
		return false;
	}

	if (!m_grpDevice.IsFastTNL())
		CGrannyLODController::SetMinLODMode(true);

	m_pyItem.Create();

	// Other Modules
	DefaultFont_Startup();

	CPythonIME::Instance().Create(GetWindowHandle());
	CPythonIME::Instance().SetText("", 0);
	CPythonTextTail::Instance().Initialize();

	// Light Manager
	m_LightManager.Initialize();

	CGraphicImageInstance::CreateSystem(32);

	// 백업
	STICKYKEYS sStickKeys{};
	sStickKeys.cbSize = sizeof(sStickKeys);
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);
	m_dwStickyKeysFlag = sStickKeys.dwFlags;

	// 설정
	sStickKeys.dwFlags &= ~(SKF_AVAILABLE | SKF_HOTKEYACTIVE);
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);

	// SphereMap
	CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
	CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
	return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y) const
{
	CPythonBackground & rkBG = CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition) const
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}


void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = tTime;
	m_tLocalStartTime = time(nullptr);
}

time_t CPythonApplication::GetServerTime() const
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

// 2005.03.28 - MALL 아이템에 들어있는 시간의 단위가 서버에서 time(0) 으로 만들어지는
//              값이기 때문에 단위를 맞추기 위해 시간 관련 처리를 별도로 추가
time_t CPythonApplication::GetServerTimeStamp() const
{
	return (time(nullptr) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth() const
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight() const
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort) const
{
	rstIP = m_strIP;
	riPort = m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

#ifdef ENABLE_MULTIFARM_BLOCK
void CPythonApplication::MultiFarmBlockIcon(uint8_t bStatus)
{
	HICON exeIcon = bStatus ? LoadIcon(ms_hInstance, MAKEINTRESOURCE(IDI_METIN2)) : LoadIcon(ms_hInstance, MAKEINTRESOURCE(BLOCK_METIN2));
	SendMessage(GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)exeIcon);
}
#endif

#ifdef ENABLE_FLASH_APLICATION
#include <Windows.h>
void CPythonApplication::FlashApplication()	//@fixme403
{
	HWND hWnd = GetWindowHandle();
	FLASHWINFO fi;
	fi.cbSize = sizeof(FLASHWINFO);
	fi.hwnd = hWnd;
	fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
	fi.uCount = 0;
	fi.dwTimeout = 0;
	FlashWindowEx(&fi);
}
#endif

void CPythonApplication::Destroy()
{
#ifdef CEF_BROWSER
	CefWebBrowser_Destroy();
#else
	WebBrowser_Destroy();
#endif

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();
#ifdef ENABLE_RENDER_TARGET
	m_kRenderTargetManager.Destroy();
#endif
#ifdef ENABLE_INGAME_WIKI
	m_pyWikiModelViewManager.InitializeData();
#endif

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.Destroy();
#endif

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();

#ifdef ENABLE_GRAPHIC_ON_OFF
	m_pyGraphicOnOff.Destroy();
#endif

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();


	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	// FIXME : 만들어져 있지 않음 - [levites]
	//CSpeedTreeForestDirectX8::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys{};
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0);
}

#ifdef ENABLE_RENDER_LOGIN_EFFECTS
void CPythonApplication::RenderEffects()
{
	CCullingManager::Instance().Process();
	m_kEftMgr.Render();
}

void CPythonApplication::UpdateEffects()
{
	m_kEftMgr.Update();
}
#endif
