#pragma once

#include "../EterLib/MSApplication.h"
#include "../EterLib/Input.h"
#include "../EterLib/GrpDevice.h"
#include "../EterLib/NetDevice.h"
#include "../EterLib/GrpLightManager.h"
#include "../EffectLib/EffectManager.h"
#include "../GameLib/RaceManager.h"
#include "../GameLib/ItemManager.h"
#include "../GameLib/FlyingObjectManager.h"
#include "../GameLib/GameEventManager.h"
#include "../MilesLib/SoundManager.h"

#include "PythonEventManager.h"
#include "PythonPlayer.h"
#include "PythonNonPlayer.h"
#include "PythonMiniMap.h"
#include "PythonIME.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonChat.h"
#include "PythonTextTail.h"
#include "PythonSkill.h"
#include "PythonSystem.h"
//#include "PythonNetworkDatagram.h"
#include "PythonNetworkStream.h"
#include "PythonCharacterManager.h"
#include "PythonQuest.h"
#include "PythonMessenger.h"
#include "PythonSafeBox.h"
#include "PythonGuild.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"

#include "AccountConnector.h"

#include "ServerStateChecker.h"
#include "AbstractApplication.h"
#include "MovieMan.h"
#ifdef ENABLE_RENDER_TARGET
#	include "../eterLib/CRenderTarget.h"
#	include "../eterLib/CRenderTargetManager.h"
#endif
#ifdef ENABLE_CUBE_RENEWAL
#	include "PythonCubeRenewal.h"
#endif
#ifdef ENABLE_RANKING_SYSTEM
#	include "PythonRanking.h"
#endif
#ifdef ENABLE_INGAME_WIKI
#	include "PythonWikiRenderTarget.h"
#endif
#ifdef ENABLE_MAILBOX
#	include "PythonMailBox.h"
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
#	include "PythonGuildBank.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "PythonGameEventManager.h"
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
#	include "PythonDungeonInfo.h"
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
#	include "PythonYutnoriManager.h"
#endif
#ifdef ENABLE_WEATHER_INFO
#	include "WeatherInfo.h"
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#	include "PythonLocale.h"
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#	include "PythonAchievement.h"
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
#	include "PythonBiologManager.h"
#endif
#ifdef ENABLE_CSHIELD
#	include "CShield.h"
#endif

class CPythonApplication : public CMSApplication, public CInputKeyboard, public IAbstractApplication
{
public:
	enum EDeviceState
	{
		DEVICE_STATE_FALSE,
		DEVICE_STATE_SKIP,
		DEVICE_STATE_OK
	};

#ifdef ENABLE_OFFICAL_FEATURES
	enum EVersion
	{
		VERSION_NUMBER = 49,
	};

	enum ELoginInfo
	{
		LOGIN_TYPE_NONE,
		LOGIN_TYPE_STEAM,
		LOGIN_TYPE_TNT
	};
#endif

#ifdef ENABLE_DELETE_FAILURE_TYPE
	enum EDeleteTypes
	{
		DELETE_FAILURE_NORMAL,
		DELETE_FAILURE_HAVE_SEALED_ITEM,
		DELETE_FAILURE_PRIVATE_CODE_ERROR,
		DELETE_FAILURE_LIMITE_LEVEL_HIGHER,
		DELETE_FAILURE_LIMITE_LEVEL_LOWER,
		DELETE_FAILURE_REMAIN_TIME,
		DELETE_FAILURE_GUILD_MEMBER,
		DELETE_FAILURE_MARRIAGE,
		DELETE_FAILURE_LAST_CHAR_SAFEBOX,
		DELETE_FAILURE_ATTR67,
		DELETE_FAILURE_PREMIUM_PRIVATE_SHOP
	};
#endif

	enum ECursorMode
	{
		CURSOR_MODE_HARDWARE,
		CURSOR_MODE_SOFTWARE
	};

	enum ECursorShape
	{
		CURSOR_SHAPE_NORMAL,
		CURSOR_SHAPE_ATTACK,
		CURSOR_SHAPE_TARGET,
		CURSOR_SHAPE_TALK,
		CURSOR_SHAPE_CANT_GO,
		CURSOR_SHAPE_PICK,

		CURSOR_SHAPE_FISH,	//ENABLE_FISHING_RENEWAL

		CURSOR_SHAPE_DOOR,
		CURSOR_SHAPE_CHAIR,
		CURSOR_SHAPE_MAGIC, // Magic
		CURSOR_SHAPE_BUY, // Buy
		CURSOR_SHAPE_SELL, // Sell

		CURSOR_SHAPE_CAMERA_ROTATE, // Camera Rotate
		CURSOR_SHAPE_HSIZE, // Horizontal Size
		CURSOR_SHAPE_VSIZE, // Vertical Size
		CURSOR_SHAPE_HVSIZE, // Horizontal & Vertical Size

		CURSOR_SHAPE_COUNT,

		// 안정적인 네이밍 변환을 위한 임시 enumerate
		NORMAL = CURSOR_SHAPE_NORMAL,
		ATTACK = CURSOR_SHAPE_ATTACK,
		TARGET = CURSOR_SHAPE_TARGET,
		CAMERA_ROTATE = CURSOR_SHAPE_CAMERA_ROTATE,
		CURSOR_COUNT = CURSOR_SHAPE_COUNT
	};

	enum EInfo
	{
		INFO_ACTOR,
		INFO_EFFECT,
		INFO_ITEM,
		INFO_TEXTTAIL
	};

	enum ECameraControlDirection
	{
		CAMERA_TO_POSITIVE = 1,
		CAMERA_TO_NEGITIVE = -1,
		CAMERA_STOP = 0
	};

	enum
	{
		CAMERA_MODE_NORMAL = 0,
		CAMERA_MODE_STAND = 1,
		CAMERA_MODE_BLEND = 2,

		EVENT_CAMERA_NUMBER = 101
	};

	enum ERenderTarget
	{
		RENDER_TARGET_UNKNOWN,
		RENDER_TARGET_MODEL_PREVIEW,
#ifdef ENABLE_MINI_GAME_YUTNORI
		RENDER_TARGET_YUTNORI,
#endif
	};

	struct SCameraSpeed
	{
		float m_fUpDir{0.0f};
		float m_fViewDir{0.0f};
		float m_fCrossDir{0.0f};

		SCameraSpeed() = default;
	};

public:
	CPythonApplication();
	~CPythonApplication();
	CLASS_DELETE_COPYMOVE(CPythonApplication);

public:
	void ShowWebPage(const char * c_szURL, const RECT & c_rcWebPage);
	void MoveWebPage(const RECT & c_rcWebPage);
	void HideWebPage();

	bool IsWebPageMode() const;

public:
	void NotifyHack(const char * c_szFormat, ...);
	void GetInfo(uint32_t eInfo, std::string * pstInfo);
	void GetMousePosition(POINT * ppt) override;

	static CPythonApplication & Instance()
	{
		assert(ms_pInstance != nullptr);
		return *ms_pInstance;
	}

	void Loop();
	void Destroy();
	void Clear();
	void Exit() const;
	void Abort() const;

	void SetMinFog(float fMinFog) const;
	void SetFrameSkip(bool isEnable);
	void SkipRenderBuffering(uint32_t dwSleepMSec) override;

	bool Create(PyObject * poSelf, const char * c_szName, int width, int height, int Windowed);
	bool CreateDevice(int width, int height, int Windowed, int bit = 32, int frequency = 0);

	void UpdateGame();
	void RenderGame();

	bool Process();

	void UpdateClientRect();

	bool CreateCursors();
	void DestroyCursors();

	void SafeSetCapture() const;
	void SafeReleaseCapture() const;

	BOOL SetCursorNum(int iCursorNum);
	void SetCursorVisible(BOOL bFlag, bool bLiarCursorOn = false);
	BOOL GetCursorVisible() const;
	bool GetLiarCursorOn() const;
	void SetCursorMode(int iMode);
	int GetCursorMode() const;
	int GetCursorNum() const { return m_iCursorNum; }

	void SetMouseHandler(PyObject * poMouseHandler);

	int GetWidth() const;
	int GetHeight() const;

	void SetGlobalCenterPosition(LONG x, LONG y) const;
	void SetCenterPosition(float fx, float fy, float fz) override;
	void GetCenterPosition(TPixelPosition * pPixelPosition) const;
	void SetCamera(float Distance, float Pitch, float Rotation, float fDestinationHeight);
	void GetCamera(float * Distance, float * Pitch, float * Rotation, float * DestinationHeight) const;
	void RotateCamera(int iDirection);
	void PitchCamera(int iDirection);
	void ZoomCamera(int iDirection);
	void MovieRotateCamera(int iDirection);
	void MoviePitchCamera(int iDirection);
	void MovieZoomCamera(int iDirection);
	void MovieResetCamera();
	void SetViewDirCameraSpeed(float fSpeed);
	void SetCrossDirCameraSpeed(float fSpeed);
	void SetUpDirCameraSpeed(float fSpeed);
	float GetRotation() const;
	float GetPitch() const;
#ifdef ENABLE_MULTIFARM_BLOCK
	void MultiFarmBlockIcon(uint8_t bStatus);
#endif

	void SetFPS(int iFPS);
	void SetServerTime(time_t tTime) override;
	time_t GetServerTime() const;
	time_t GetServerTimeStamp() const;
	float GetGlobalTime() override;
	float GetGlobalElapsedTime() override;

	float GetFaceSpeed() const { return m_fFaceSpd; }
	float GetAveRenderTime() const { return m_fAveRenderTime; }
	uint32_t GetCurRenderTime() const { return m_dwCurRenderTime; }
	uint32_t GetCurUpdateTime() const { return m_dwCurUpdateTime; }
	uint32_t GetUpdateFPS() const { return m_dwUpdateFPS; }
	uint32_t GetRenderFPS() const { return m_dwRenderFPS; }
	uint32_t GetLoad() const { return m_dwLoad; }
	uint32_t GetFaceCount() const { return m_dwFaceCount; }

	void SetConnectData(const char * c_szIP, int iPort);
	void GetConnectData(std::string & rstIP, int & riPort) const;

	void RunIMEUpdate() override;
	void RunIMETabEvent() override;
	void RunIMEReturnEvent() override;
	void RunPressExitKey() const;

	void RunIMEChangeCodePage() override;
	void RunIMEOpenCandidateListEvent() override;
	void RunIMECloseCandidateListEvent() override;
	void RunIMEOpenReadingWndEvent() override;
	void RunIMECloseReadingWndEvent() override;

	void EnableSpecialCameraMode();
	void SetCameraSpeed(int iPercentage);

	bool IsLockCurrentCamera() const;
	void SetEventCamera(const SCameraSetting & c_rCameraSetting) override;
	void BlendEventCamera(const SCameraSetting & c_rCameraSetting, float fBlendTime) override;
	void SetDefaultCamera() override;

	void SetCameraSetting(const SCameraSetting & c_rCameraSetting);
	void GetCameraSetting(SCameraSetting * pCameraSetting) const;
	void SaveCameraSetting(const char * c_szFileName) const;
	bool LoadCameraSetting(const char * c_szFileName);

#ifdef ENABLE_REVERSED_FUNCTIONS_EX
	void CpyDefaultCamera(const SCameraSetting& c_rCameraSetting);
	bool GetDefaultCameraSetting(SCameraSetting * pCameraSetting) const;
#endif

	void SetForceSightRange(int iRange);
#ifdef ENABLE_FLASH_APLICATION
	void FlashApplication();	//@fixme403
#endif

public:
	int OnLogoOpen(const char * szName);
	int OnLogoUpdate();
	void OnLogoRender() const;
	void OnLogoClose();

protected:
	IGraphBuilder * m_pGraphBuilder; // Graph Builder
	IBaseFilter * m_pFilterSG; // Sample Grabber 필터
	ISampleGrabber * m_pSampleGrabber; // 영상 이미지 캡처를 위한 샘플 그래버
	IMediaControl * m_pMediaCtrl; // Media Control
	IMediaEventEx * m_pMediaEvent; // Media Event
	IVideoWindow * m_pVideoWnd; // Video Window
	IBasicVideo * m_pBasicVideo;
	std::vector<uint8_t> m_pCaptureBuffer; // 영상 이미지를 캡처한 버퍼
	std::unique_ptr<CGraphicImageTexture> m_pLogoTex; // 출력할 텍스쳐
	bool m_bLogoError; // 영상 읽기 상태
	bool m_bLogoPlay;

	int m_nLeft, m_nRight, m_nTop, m_nBottom;


protected:
	LRESULT WindowProcedure(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam) override;

	void OnCameraUpdate();

	void OnUIUpdate() const;
	void OnUIRender() const;

	void OnMouseUpdate() const;
	void OnMouseRender() const;

	void OnMouseWheel(int nLen) const;
	void OnMouseMove(int x, int y);
	void OnMouseMiddleButtonDown(int x, int y);
	void OnMouseMiddleButtonUp(int x, int y);
	void OnMouseLeftButtonDown(int x, int y) const;
	void OnMouseLeftButtonUp(int x, int y) const;
	void OnMouseLeftButtonDoubleClick(int x, int y) const;
	void OnMouseRightButtonDown(int x, int y) const;
	void OnMouseRightButtonUp(int x, int y) const;
	void OnSizeChange(int width, int height) const;
	void OnKeyDown(int iIndex) override;
	void OnKeyUp(int iIndex) override;
	void OnIMEKeyDown(int iIndex) const;

	int32_t CheckDeviceState();

	BOOL __IsContinuousChangeTypeCursor(int iCursorNum) const;

	void __UpdateCamera();

	void __SetFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP) const;
	void __MinimizeFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight) const;
	void __ResetCamera(); // @fixme019

protected:
	CTimer m_timer;

	CLightManager m_LightManager;
	CSoundManager m_SoundManager;
	CFlyingManager m_FlyingManager;
	CRaceManager m_RaceManager;
	CGameEventManager m_GameEventManager;
	CItemManager m_kItemMgr;
	CMovieMan m_MovieManager;

	UI::CWindowManager m_kWndMgr;
	CEffectManager m_kEftMgr;
	CPythonCharacterManager m_kChrMgr;

	CServerStateChecker m_kServerStateChecker;
	CPythonGraphic m_pyGraphic;
#ifdef ENABLE_GRAPHIC_ON_OFF
	CPythonGraphicOnOff			m_pyGraphicOnOff;
#endif
	CPythonNetworkStream m_pyNetworkStream;
	//CPythonNetworkDatagram		m_pyNetworkDatagram;
	CPythonPlayer m_pyPlayer;
	CPythonIME m_pyIme;
	CPythonItem m_pyItem;
	CPythonShop m_pyShop;
	CPythonExchange m_pyExchange;
	CPythonChat m_pyChat;
	CPythonTextTail m_pyTextTail;
	CPythonNonPlayer m_pyNonPlayer;
	CPythonMiniMap m_pyMiniMap;
	CPythonEventManager m_pyEventManager;
	CPythonBackground m_pyBackground;
	CPythonSkill m_pySkill;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	CPythonSkillPet				m_pySkillPet;
#endif
	CPythonResource m_pyRes;
	CPythonQuest m_pyQuest;
	CPythonMessenger m_pyManager;
	CPythonSafeBox m_pySafeBox;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	CPythonGuildBank m_pyGuildBank;
#endif
	CPythonGuild m_pyGuild;
#ifdef ENABLE_SWITCHBOT
	CPythonSwitchbot			m_pySwitchbot;
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	CPythonDungeonInfo m_pyDungeonInfo;
#endif
	CGuildMarkManager m_kGuildMarkManager;
	CGuildMarkDownloader m_kGuildMarkDownloader;
	CGuildMarkUploader m_kGuildMarkUploader;
	CAccountConnector m_kAccountConnector;

	CGraphicDevice m_grpDevice;
	CNetworkDevice m_netDevice;

	CPythonSystem m_pySystem;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	CPythonLocale m_pyLocale;
#endif

#ifdef ENABLE_RENDER_TARGET
	CRenderTargetManager		m_kRenderTargetManager;
#endif
#ifdef ENABLE_INGAME_WIKI
	CGraphicWikiRenderTargetTexture		m_pyWikiRenderTargetTexture;
	CWikiRenderTargetManager			m_pyWikiModelViewManager;
	CPythonWikiRenderTarget				m_pyWikiRenderTarget;
#endif
#ifdef ENABLE_CUBE_RENEWAL
	CPythonCubeRenewal 			m_pyCubeRenewal;
#endif
#ifdef ENABLE_RANKING_SYSTEM
	CPythonRanking				m_pyRanking;
#endif
#ifdef ENABLE_EVENT_MANAGER
	InGameEventManager m_pyInGameEventMgr;
#endif
#ifdef ENABLE_MAILBOX
	CPythonMailBox					m_pyMailBox;
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	CPythonYutnoriManager		m_kYutNoriManager;
#endif
#ifdef ENABLE_WEATHER_INFO
	IWeatherInfo m_pyWeather;
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	CAchievementSystem			m_achievement;
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	CPythonBiologManager m_pyBiolog;
#endif
#ifdef ENABLE_CSHIELD
	CShield m_pyCShield;
#endif

	PyObject * m_poMouseHandler{nullptr};
	D3DXVECTOR3 m_v3CenterPosition;

	unsigned int m_iFPS;
	float m_fAveRenderTime{0.0f};
	uint32_t m_dwCurRenderTime;
	uint32_t m_dwCurUpdateTime;
	uint32_t m_dwLoad;
	uint32_t m_dwWidth;
	uint32_t m_dwHeight;

protected:
	// Time
	uint32_t m_dwLastIdleTime{0};
	uint32_t m_dwStartLocalTime;
	time_t m_tServerTime;
	time_t m_tLocalStartTime;
	float m_fGlobalTime{0.0f};
	float m_fGlobalElapsedTime{0.0f};

	/////////////////////////////////////////////////////////////
	// Camera
	SCameraSetting m_DefaultCameraSetting;
	SCameraSetting m_kEventCameraSetting;

	int m_iCameraMode;
	float m_fBlendCameraStartTime;
	float m_fBlendCameraBlendTime;
	SCameraSetting m_kEndBlendCameraSetting;

	float m_fRotationSpeed;
	float m_fPitchSpeed;
	float m_fZoomSpeed;
	float m_fCameraRotateSpeed;
	float m_fCameraPitchSpeed;
	float m_fCameraZoomSpeed;

	SCameraPos m_kCmrPos;
	SCameraSpeed m_kCmrSpd;

	BOOL m_isSpecialCameraMode;
	// Camera
	/////////////////////////////////////////////////////////////

	float m_fFaceSpd;
	uint32_t m_dwFaceSpdSum;
	uint32_t m_dwFaceSpdCount;

	uint32_t m_dwFaceAccCount;
	uint32_t m_dwFaceAccTime;

	uint32_t m_dwUpdateFPS{0};
	uint32_t m_dwRenderFPS{0};
	uint32_t m_dwFaceCount{0};

	uint32_t m_dwLButtonDownTime{0};
	uint32_t m_dwLButtonUpTime;

	typedef std::map<int, HANDLE> TCursorHandleMap;
	TCursorHandleMap m_CursorHandleMap;
	HANDLE m_hCurrentCursor;

	BOOL m_bCursorVisible{TRUE};
	bool m_bLiarCursorOn{false};
	int m_iCursorMode{CURSOR_MODE_HARDWARE};
	bool m_isWindowed{false};
	bool m_isFrameSkipDisable{false};

	// Connect Data
	std::string m_strIP;
	int m_iPort;

	static CPythonApplication * ms_pInstance;

	bool m_isMinimizedWnd;
	bool m_isActivateWnd;
	BOOL m_isWindowFullScreenEnable;

	uint32_t m_dwStickyKeysFlag;
	uint32_t m_dwBufSleepSkipTime;
	int m_iForceSightRange;

protected:
	int m_iCursorNum;
	int m_iContinuousCursorNum;

#ifdef ENABLE_RENDER_LOGIN_EFFECTS
public:
	void RenderEffects();
	void UpdateEffects();
#endif
};
