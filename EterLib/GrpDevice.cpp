#include "StdAfx.h"
#include "GrpDevice.h"
#include "../EterBase/Stl.h"
#include "../EterBase/Debug.h"

bool GRAPHICS_CAPS_CAN_NOT_DRAW_LINE = false;
bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = false;
bool GRAPHICS_CAPS_HALF_SIZE_IMAGE = false;
bool GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = false;
bool GRAPHICS_CAPS_SOFTWARE_TILING = false;

D3DPRESENT_PARAMETERS g_kD3DPP;
bool g_isBrowserMode = false;
RECT g_rcBrowser;

CGraphicDevice::CGraphicDevice() : m_uBackBufferCount(0)
{
	__Initialize();
}

CGraphicDevice::~CGraphicDevice()
{
	Destroy();
}

void CGraphicDevice::__Initialize()
{
	ms_iD3DAdapterInfo = D3DADAPTER_DEFAULT;
	ms_iD3DDevInfo = D3DADAPTER_DEFAULT;
	ms_iD3DModeInfo = D3DADAPTER_DEFAULT;

	ms_lpd3d = nullptr;
	ms_lpd3dDevice = nullptr;
	ms_lpd3dMatStack = nullptr;

	ms_dwWavingEndTime = 0;
	ms_dwFlashingEndTime = 0;

	m_pStateManager = nullptr;

	__InitializeDefaultIndexBufferList();
	__InitializePDTVertexBufferList();
}

void CGraphicDevice::RegisterWarningString(uint32_t uiMsg, const char * c_szString)
{
	m_kMap_strWarningMessage[uiMsg] = c_szString;
}

void CGraphicDevice::__WarningMessage(HWND hWnd, uint32_t uiMsg)
{
	if (m_kMap_strWarningMessage.end() == m_kMap_strWarningMessage.find(uiMsg))
		return;
	MessageBox(hWnd, m_kMap_strWarningMessage[uiMsg].c_str(), "Warning", MB_OK | MB_TOPMOST);
}

void CGraphicDevice::MoveWebBrowserRect(const RECT & c_rcWebPage)
{
	g_rcBrowser = c_rcWebPage;
}

void CGraphicDevice::EnableWebBrowserMode(const RECT & c_rcWebPage)
{
	if (!ms_lpd3dDevice)
		return;

	D3DPRESENT_PARAMETERS & rkD3DPP = ms_d3dPresentParameter;

	g_isBrowserMode = true;

	if (D3DSWAPEFFECT_COPY == rkD3DPP.SwapEffect)
		return;

	g_kD3DPP = rkD3DPP;
	g_rcBrowser = c_rcWebPage;

	rkD3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
	rkD3DPP.BackBufferCount = 0;
	rkD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	IDirect3DDevice9& rkD3DDev = *ms_lpd3dDevice;
	HRESULT hr = rkD3DDev.Reset(&rkD3DPP);
	if (FAILED(hr))
		return;

	STATEMANAGER.SetDefaultState();
}

void CGraphicDevice::DisableWebBrowserMode()
{
	if (!ms_lpd3dDevice)
		return;

	D3DPRESENT_PARAMETERS & rkD3DPP = ms_d3dPresentParameter;

	g_isBrowserMode = false;

	rkD3DPP = g_kD3DPP;

	IDirect3DDevice9 & rkD3DDev = *ms_lpd3dDevice;
	HRESULT hr = rkD3DDev.Reset(&rkD3DPP);
	if (FAILED(hr))
		return;

	STATEMANAGER.SetDefaultState();
}

bool CGraphicDevice::ResizeBackBuffer(uint32_t uWidth, uint32_t uHeight)
{
	if (!ms_lpd3dDevice)
		return false;

	D3DPRESENT_PARAMETERS & rkD3DPP = ms_d3dPresentParameter;
	if (rkD3DPP.Windowed)
	{
		if (rkD3DPP.BackBufferWidth != uWidth || rkD3DPP.BackBufferHeight != uHeight)
		{
			rkD3DPP.BackBufferWidth = uWidth;
			rkD3DPP.BackBufferHeight = uHeight;

			IDirect3DDevice9 & rkD3DDev = *ms_lpd3dDevice;

			HRESULT hr = rkD3DDev.Reset(&rkD3DPP);
			if (FAILED(hr))
				return false;

			STATEMANAGER.SetDefaultState();
		}
	}

	return true;
}

uint32_t CGraphicDevice::CreatePNTStreamVertexShader()
{
	assert(ms_lpd3dDevice != nullptr);
	return D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
}

uint32_t CGraphicDevice::CreatePNT2StreamVertexShader()
{
	assert(ms_lpd3dDevice != nullptr);
	return D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_TEX2;
}

CGraphicDevice::EDeviceState CGraphicDevice::GetDeviceState()
{
	if (!ms_lpd3dDevice)
		return DEVICESTATE_NULL;

	HRESULT hr;

	if (FAILED(hr = ms_lpd3dDevice->TestCooperativeLevel()))
	{
		if (D3DERR_DEVICELOST == hr)
			return DEVICESTATE_BROKEN;

		if (D3DERR_DEVICENOTRESET == hr)
			return DEVICESTATE_NEEDS_RESET;

		return DEVICESTATE_BROKEN;
	}

	return DEVICESTATE_OK;
}

auto CGraphicDevice::LostDevice() -> void {
	__DestroyPDTVertexBufferList();

	//TraceError("Successfully removed device data!");
}

auto CGraphicDevice::ResetDevice() -> void {

	m_pStateManager->SetDefaultState();
	__CreatePDTVertexBufferList();
	//TraceError("Successfully restored device data!");
}

bool CGraphicDevice::Reset()
{
	LostDevice();
	HRESULT hr;

	if (FAILED(hr = ms_lpd3dDevice->Reset(&ms_d3dPresentParameter)))
	{
		TraceError("Device reset failed?");
		return false;
	}

	ResetDevice();

	return true;
}

static LPDIRECT3DSURFACE9 s_lpStencil;
static uint32_t s_MaxTextureWidth, s_MaxTextureHeight;

LPDIRECT3D9 CGraphicDevice::GetDirectx9()
{
	return ms_lpd3d;
}

LPDIRECT3DDEVICE9 CGraphicDevice::GetDevice()
{
	return ms_lpd3dDevice;
}

BOOL EL3D_ConfirmDevice(D3DCAPS9 & rkD3DCaps, uint32_t uBehavior, D3DFORMAT /*eD3DFmt*/)
{
	// PUREDEVICE는 GetTransform / GetViewport 등이 되지 않는다.
	if (uBehavior & D3DCREATE_PUREDEVICE)
		return FALSE;

	if (uBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		// DirectionalLight
		if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS))
			return FALSE;

		// PositionalLight
		if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS))
			return FALSE;

		// Software T&L Support - ATI NOT SUPPORT CLIP, USE DIRECTX SOFTWARE PROCESSING CLIPPING
		if (GRAPHICS_CAPS_SOFTWARE_TILING)
		{
			if (!(rkD3DCaps.PrimitiveMiscCaps & D3DPMISCCAPS_CLIPTLVERTS))
				return FALSE;
		}
		else
		{
			// Shadow/Terrain
			if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN))
				return FALSE;
		}
	}

	s_MaxTextureWidth = rkD3DCaps.MaxTextureWidth;
	s_MaxTextureHeight = rkD3DCaps.MaxTextureHeight;

	return TRUE;
}

uint32_t GetMaxTextureWidth()
{
	return s_MaxTextureWidth;
}

uint32_t GetMaxTextureHeight()
{
	return s_MaxTextureHeight;
}

bool CGraphicDevice::__IsInDriverBlackList(D3D_CAdapterInfo & rkD3DAdapterInfo)
{
	D3DADAPTER_IDENTIFIER9 & rkD3DAdapterIdentifier = rkD3DAdapterInfo.GetIdentifier();

	char szSrcDriver[256];
	strncpy(szSrcDriver, rkD3DAdapterIdentifier.Driver, sizeof(szSrcDriver) - 1);
	uint32_t dwSrcHighVersion = rkD3DAdapterIdentifier.DriverVersion.QuadPart >> 32;
	uint32_t dwSrcLowVersion = rkD3DAdapterIdentifier.DriverVersion.QuadPart & 0xffffffff;

	bool ret = false;

	msl::file_ptr fPtr("grpblk.txt", "r");
	if (fPtr)
	{
		uint32_t dwChkHighVersion;
		uint32_t dwChkLowVersion;

		char szChkDriver[256];

		char szLine[256];
		while (fgets(szLine, sizeof(szLine) - 1, fPtr.get()))
		{
			sscanf(szLine, "%s %x %x", szChkDriver, &dwChkHighVersion, &dwChkLowVersion);

			if (strcmp(szSrcDriver, szChkDriver) == 0)
				if (dwSrcHighVersion == dwChkHighVersion)
					if (dwSrcLowVersion == dwChkLowVersion)
					{
						ret = true;
						break;
					}

			szLine[0] = '\0';
		}
	}

	return ret;
}

int CGraphicDevice::Create(HWND hWnd, int iHres, int iVres, bool Windowed, int /*iBit*/, int iReflashRate)
{
	int iRet = CREATE_OK;

	Destroy();

	ms_iWidth = iHres;
	ms_iHeight = iVres;

	ms_hWnd = hWnd;
	ms_hDC = GetDC(hWnd);
	ms_lpd3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!ms_lpd3d)
		return CREATE_NO_DIRECTX;

	if (!ms_kD3DDetector.Build(*ms_lpd3d, EL3D_ConfirmDevice))
		return CREATE_ENUM;

#ifdef ENABLE_D3D_DETECTION
	if (!ms_kD3DDetector.Find(800, 600, 32, TRUE, &ms_iD3DModeInfo, &ms_iD3DDevInfo, &ms_iD3DAdapterInfo))
		if (!ms_kD3DDetector.Find(iHres, iVres, 32, TRUE, &ms_iD3DModeInfo, &ms_iD3DDevInfo, &ms_iD3DAdapterInfo))
			return CREATE_DETECT;
#endif

	std::string stDevList;
	ms_kD3DDetector.GetString(&stDevList);

	D3D_CAdapterInfo * pkD3DAdapterInfo = ms_kD3DDetector.GetD3DAdapterInfop(ms_iD3DAdapterInfo);
	if (!pkD3DAdapterInfo)
	{
		Tracenf("adapter %d is EMPTY", ms_iD3DAdapterInfo);
		return CREATE_DETECT;
	}

	if (__IsInDriverBlackList(*pkD3DAdapterInfo))
	{
		iRet |= CREATE_BAD_DRIVER;
		__WarningMessage(hWnd, CREATE_BAD_DRIVER);
	}

	D3D_SModeInfo * pkD3DModeInfo = pkD3DAdapterInfo->GetD3DModeInfop(ms_iD3DDevInfo, ms_iD3DModeInfo);
	if (!pkD3DModeInfo)
	{
		Tracenf("device %d, mode %d is EMPTY", ms_iD3DDevInfo, ms_iD3DModeInfo);
		return CREATE_DETECT;
	}

	D3DADAPTER_IDENTIFIER9 & rkD3DAdapterId = pkD3DAdapterInfo->GetIdentifier();
	if (Windowed && strnicmp(rkD3DAdapterId.Driver, "3dfx", 4) == 0 && 22 == pkD3DAdapterInfo->GetDesktopD3DDisplayModer().Format)
		return CREATE_FORMAT;

	if (pkD3DModeInfo->m_dwD3DBehavior == D3DCREATE_SOFTWARE_VERTEXPROCESSING)
	{
		iRet |= CREATE_NO_TNL;
	}

	std::string stModeInfo;
	pkD3DModeInfo->GetString(&stModeInfo);

	int ErrorCorrection = 0;

RETRY:
	ZeroMemory(&ms_d3dPresentParameter, sizeof(ms_d3dPresentParameter));

	ms_d3dPresentParameter.Windowed = Windowed;
	ms_d3dPresentParameter.BackBufferWidth = iHres;
	ms_d3dPresentParameter.BackBufferHeight = iVres;
	ms_d3dPresentParameter.hDeviceWindow = hWnd;
	ms_d3dPresentParameter.BackBufferCount = m_uBackBufferCount;
	ms_d3dPresentParameter.SwapEffect = D3DSWAPEFFECT_DISCARD;

	if (Windowed)
		ms_d3dPresentParameter.BackBufferFormat = pkD3DAdapterInfo->GetDesktopD3DDisplayModer().Format;
	else
	{
		ms_d3dPresentParameter.BackBufferFormat = pkD3DModeInfo->m_eD3DFmtPixel;
		ms_d3dPresentParameter.FullScreen_RefreshRateInHz = iReflashRate;
	}

	ms_d3dPresentParameter.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	ms_d3dPresentParameter.EnableAutoDepthStencil = TRUE;
	ms_d3dPresentParameter.AutoDepthStencilFormat = pkD3DModeInfo->m_eD3DFmtDepthStencil;

	ms_dwD3DBehavior = pkD3DModeInfo->m_dwD3DBehavior;

	if (FAILED(ms_hLastResult = ms_lpd3d->CreateDevice(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, hWnd,
													   // 2004. 1. 9 myevan 버텍스 프로세싱 방식 자동 선택 추가
													   pkD3DModeInfo->m_dwD3DBehavior, &ms_d3dPresentParameter, &ms_lpd3dDevice)))
	{
		switch (ms_hLastResult)
		{
			case D3DERR_INVALIDCALL:
				Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_INVALIDCALL\nThe method call is invalid. For example, a method's parameter "
					   "may have an invalid value.");
				break;
			case D3DERR_NOTAVAILABLE:
				Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_NOTAVAILABLE\nThis device does not support the queried technique. ");
				break;
			case D3DERR_OUTOFVIDEOMEMORY:
				Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_OUTOFVIDEOMEMORY\nDirect3D does not have enough display memory to perform "
					   "the operation");
				break;
			default:
				Tracenf("IDirect3DDevice.CreateDevice - ERROR %d", ms_hLastResult);
				break;
		}

		if (ErrorCorrection)
			return CREATE_DEVICE;

		// 2004. 1. 9 myevan 큰의미 없는 코드인듯.. 에러나면 표시하고 종료하자
		iReflashRate = 0;
		++ErrorCorrection;
		iRet = CREATE_REFRESHRATE;
		goto RETRY;
	}

	// Check DXT Support Info
	if (ms_lpd3d->CheckDeviceFormat(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, ms_d3dPresentParameter.BackBufferFormat, 0, D3DRTYPE_TEXTURE,
									D3DFMT_DXT1) == D3DERR_NOTAVAILABLE)
		ms_bSupportDXT = false;

	if (ms_lpd3d->CheckDeviceFormat(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, ms_d3dPresentParameter.BackBufferFormat, 0, D3DRTYPE_TEXTURE,
									D3DFMT_DXT3) == D3DERR_NOTAVAILABLE)
		ms_bSupportDXT = false;

	if (ms_lpd3d->CheckDeviceFormat(ms_iD3DAdapterInfo, D3DDEVTYPE_HAL, ms_d3dPresentParameter.BackBufferFormat, 0, D3DRTYPE_TEXTURE,
									D3DFMT_DXT5) == D3DERR_NOTAVAILABLE)
		ms_bSupportDXT = false;

	if (FAILED((ms_hLastResult = ms_lpd3dDevice->GetDeviceCaps(&ms_d3dCaps))))
	{
		Tracenf("IDirect3DDevice.GetDeviceCaps - ERROR %d", ms_hLastResult);
		return CREATE_GET_DEVICE_CAPS2;
	}

	if (!Windowed)
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, iHres, iVres, SWP_SHOWWINDOW);

	ms_lpd3dDevice->GetViewport(&ms_Viewport);

	m_pStateManager = new CStateManager(ms_lpd3dDevice);
	m_pStateManager->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	D3DXCreateMatrixStack(0, &ms_lpd3dMatStack);
	ms_lpd3dMatStack->LoadIdentity();

	ms_pntVS = CreatePNTStreamVertexShader();
	ms_pnt2VS = CreatePNT2StreamVertexShader();

	D3DXMatrixIdentity(&ms_matIdentity);
	D3DXMatrixIdentity(&ms_matView);
	D3DXMatrixIdentity(&ms_matProj);
	D3DXMatrixIdentity(&ms_matInverseView);
	D3DXMatrixIdentity(&ms_matInverseViewYAxis);
	D3DXMatrixIdentity(&ms_matScreen0);
	D3DXMatrixIdentity(&ms_matScreen1);
	D3DXMatrixIdentity(&ms_matScreen2);

	ms_matScreen0._11 = 1;
	ms_matScreen0._22 = -1;

	ms_matScreen1._41 = 1;
	ms_matScreen1._42 = 1;

	ms_matScreen2._11 = (float) iHres / 2;
	ms_matScreen2._22 = (float) iVres / 2;

	D3DXCreateSphere(ms_lpd3dDevice, 1.0f, 32, 32, &ms_lpSphereMesh, nullptr);
	D3DXCreateCylinder(ms_lpd3dDevice, 1.0f, 1.0f, 1.0f, 8, 8, &ms_lpCylinderMesh, nullptr);

	ms_lpd3dDevice->Clear(0L, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);

	if (!__CreateDefaultIndexBufferList())
		return false;

	if (!__CreatePDTVertexBufferList())
		return false;

	uint32_t dwTexMemSize = GetAvailableTextureMemory();

	if (dwTexMemSize < 64 * 1024 * 1024)
		ms_isLowTextureMemory = true;
	else
		ms_isLowTextureMemory = false;

	if (dwTexMemSize > 100 * 1024 * 1024)
		ms_isHighTextureMemory = true;
	else
		ms_isHighTextureMemory = false;

	if (ms_d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER)
		GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = false;
	else
		GRAPHICS_CAPS_CAN_NOT_TEXTURE_ADDRESS_BORDER = true;

	if (strnicmp(rkD3DAdapterId.Driver, "SIS", 3) == 0)
	{
		GRAPHICS_CAPS_CAN_NOT_DRAW_LINE = true;
		GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = true;
		GRAPHICS_CAPS_HALF_SIZE_IMAGE = true;
		ms_isLowTextureMemory = true;
	}
	else if (strnicmp(rkD3DAdapterId.Driver, "3dfx", 4) == 0)
	{
		GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW = true;
		GRAPHICS_CAPS_HALF_SIZE_IMAGE = true;
		ms_isLowTextureMemory = true;
	}

	return (iRet);
}

void CGraphicDevice::__InitializePDTVertexBufferList()
{
	for (auto & i : ms_alpd3dPDTVB)
		i = nullptr;
}

void CGraphicDevice::__DestroyPDTVertexBufferList()
{
	for (auto & i : ms_alpd3dPDTVB)
	{
		if (i)
		{
			i->Release();
			i = nullptr;
		}
	}
}

bool CGraphicDevice::__CreatePDTVertexBufferList()
{
	for (auto & i : ms_alpd3dPDTVB)
	{
		if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(sizeof(TPDTVertex) * PDT_VERTEX_NUM, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
													  D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_SYSTEMMEM, &i, NULL)))
			return false;
	}
	return true;
}

void CGraphicDevice::__InitializeDefaultIndexBufferList()
{
	for (auto & i : ms_alpd3dDefIB)
		i = nullptr;
}

void CGraphicDevice::__DestroyDefaultIndexBufferList()
{
	for (auto & i : ms_alpd3dDefIB)
		if (i)
		{
			i->Release();
			i = nullptr;
		}
}

bool CGraphicDevice::__CreateDefaultIndexBuffer(uint32_t eDefIB, uint32_t uIdxCount, const uint16_t * c_awIndices)
{
	assert(ms_alpd3dDefIB[eDefIB] == nullptr);

	if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(sizeof(uint16_t) * uIdxCount, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED,
												 &ms_alpd3dDefIB[eDefIB], NULL)))
		return false;

	uint16_t * dstIndices;
	if (FAILED(ms_alpd3dDefIB[eDefIB]->Lock(0, 0, (void **) &dstIndices, 0)))
		return false;

	memcpy(dstIndices, c_awIndices, sizeof(uint16_t) * uIdxCount);

	ms_alpd3dDefIB[eDefIB]->Unlock();

	return true;
}

bool CGraphicDevice::__CreateDefaultIndexBufferList()
{
	static const uint16_t c_awLineIndices[2] = {0, 1};
	static const uint16_t c_awLineTriIndices[6] = {0, 1, 0, 2, 1, 2};
	static const uint16_t c_awLineRectIndices[8] = {0, 1, 0, 2, 1, 3, 2, 3};
	static const uint16_t c_awLineCubeIndices[24] = {0, 1, 0, 2, 1, 3, 2, 3, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7};
	static const uint16_t c_awFillTriIndices[3] = {0, 1, 2};
	static const uint16_t c_awFillRectIndices[6] = {0, 2, 1, 2, 3, 1};
	static const uint16_t c_awFillCubeIndices[36] = {0, 1, 2, 1, 3, 2, 2, 0, 6, 0, 4, 6, 0, 1, 4, 1, 5, 4,
													 1, 3, 5, 3, 7, 5, 3, 2, 7, 2, 6, 7, 4, 5, 6, 5, 7, 6};

	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE, 2, c_awLineIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_TRI, 6, c_awLineTriIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_RECT, 8, c_awLineRectIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_CUBE, 24, c_awLineCubeIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_TRI, 3, c_awFillTriIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_RECT, 6, c_awFillRectIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE, 36, c_awFillCubeIndices))
		return false;

	return true;
}

void CGraphicDevice::InitBackBufferCount(uint32_t uBackBufferCount)
{
	m_uBackBufferCount = uBackBufferCount;
}

void CGraphicDevice::Destroy()
{
	__DestroyPDTVertexBufferList();
	__DestroyDefaultIndexBufferList();

	if (ms_hDC)
	{
		ReleaseDC(ms_hWnd, ms_hDC);
		ms_hDC = nullptr;
	}

	if (ms_pntVS)
	{
		ms_pntVS = 0;
	}

	if (ms_pnt2VS)
	{
		ms_pnt2VS = 0;
	}

	safe_release(ms_lpSphereMesh);
	safe_release(ms_lpCylinderMesh);

	safe_release(ms_lpd3dMatStack);
	safe_release(ms_lpd3dDevice);
	safe_release(ms_lpd3d);

	if (m_pStateManager)
	{
		delete m_pStateManager;
		m_pStateManager = nullptr;
	}

	__Initialize();
}
