#include "StdAfx.h"
#include "resource.h"
#include "PythonApplication.h"
#include "../EterLib/Camera.h"
#include "Locale.h"
#include "../GameLib/GameLibDefines.h"

extern D3DXCOLOR g_fSpecularColor;
extern BOOL bVisibleNotice = true;
extern BOOL bTestServerFlag = FALSE;
extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = 0;

PyObject* appShowWebPage(PyObject* poSelf, PyObject* poArgs)
{
	char* szWebPage;
	if (!PyTuple_GetString(poArgs, 0, &szWebPage))
		return Py_BuildException();

	PyObject* poRect = PyTuple_GetItem(poArgs, 1);
	if (!PyTuple_Check(poRect))
		return Py_BuildException();

	RECT rcWebPage;
	rcWebPage.left = PyInt_AsLong(PyTuple_GetItem(poRect, 0));
	rcWebPage.top = PyInt_AsLong(PyTuple_GetItem(poRect, 1));
	rcWebPage.right = PyInt_AsLong(PyTuple_GetItem(poRect, 2));
	rcWebPage.bottom = PyInt_AsLong(PyTuple_GetItem(poRect, 3));

	CPythonApplication::Instance().ShowWebPage(szWebPage, rcWebPage);
	return Py_BuildNone();
}

PyObject* appMoveWebPage(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poRect = PyTuple_GetItem(poArgs, 0);
	if (!PyTuple_Check(poRect))
		return Py_BuildException();

	RECT rcWebPage;
	rcWebPage.left = PyInt_AsLong(PyTuple_GetItem(poRect, 0));
	rcWebPage.top = PyInt_AsLong(PyTuple_GetItem(poRect, 1));
	rcWebPage.right = PyInt_AsLong(PyTuple_GetItem(poRect, 2));
	rcWebPage.bottom = PyInt_AsLong(PyTuple_GetItem(poRect, 3));

	CPythonApplication::Instance().MoveWebPage(rcWebPage);
	return Py_BuildNone();
}

PyObject* appHideWebPage(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().HideWebPage();
	return Py_BuildNone();
}

PyObject* appIsWebPageMode(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().IsWebPageMode());
}

PyObject* appEnablePerformanceTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}

/////////////////////////////////////////////////////

extern BOOL HAIR_COLOR_ENABLE;
extern BOOL USE_ARMOR_SPECULAR;
extern BOOL USE_WEAPON_SPECULAR;
extern BOOL SKILL_EFFECT_UPGRADE_ENABLE;
extern BOOL RIDE_HORSE_ENABLE;
extern double g_specularSpd;

// TEXTTAIL_LIVINGTIME_CONTROL
extern void TextTail_SetLivingTime(long livingTime);

PyObject* appSetTextTailLivingTime(PyObject* poSelf, PyObject* poArgs)
{
	float livingTime;
	if (!PyTuple_GetFloat(poArgs, 0, &livingTime))
		return Py_BuildException();

	TextTail_SetLivingTime(livingTime * 1000);

	return Py_BuildNone();
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

PyObject* appSetHairColorEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	HAIR_COLOR_ENABLE = nEnable;

	return Py_BuildNone();
}

PyObject* appSetArmorSpecularEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	USE_ARMOR_SPECULAR = nEnable;

	return Py_BuildNone();
}

PyObject* appSetWeaponSpecularEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	USE_WEAPON_SPECULAR = nEnable;

	return Py_BuildNone();
}

PyObject* appSetSkillEffectUpgradeEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	SKILL_EFFECT_UPGRADE_ENABLE = nEnable;

	return Py_BuildNone();
}

PyObject* SetTwoHandedWeaponAttSpeedDecreaseValue(PyObject* poSelf, PyObject* poArgs)
{
	int iValue;
	if (!PyTuple_GetInteger(poArgs, 0, &iValue))
		return Py_BuildException();

	TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = iValue;

	return Py_BuildNone();
}

PyObject* appSetRideHorseEnable(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	RIDE_HORSE_ENABLE = nEnable;

	return Py_BuildNone();
}

PyObject* appSetCameraMaxDistance(PyObject* poSelf, PyObject* poArgs)
{
	float fMax;
	if (!PyTuple_GetFloat(poArgs, 0, &fMax))
		return Py_BuildException();

	CCamera::SetCameraMaxDistance(fMax);
	return Py_BuildNone();
}

PyObject* appSetControlFP(PyObject* poSelf, PyObject* poArgs)
{
	_controlfp(_PC_24, _MCW_PC);
	return Py_BuildNone();
}

PyObject* appSetSpecularSpeed(PyObject* poSelf, PyObject* poArgs)
{
	float fSpeed;
	if (!PyTuple_GetFloat(poArgs, 0, &fSpeed))
		return Py_BuildException();

	g_specularSpd = fSpeed;

	return Py_BuildNone();
}

PyObject* appSetMinFog(PyObject* poSelf, PyObject* poArgs)
{
	float fMinFog;
	if (!PyTuple_GetFloat(poArgs, 0, &fMinFog))
		return Py_BuildException();

	CPythonApplication::Instance().SetMinFog(fMinFog);
	return Py_BuildNone();
}

PyObject* appSetFrameSkip(PyObject* poSelf, PyObject* poArgs)
{
	int nFrameSkip;
	if (!PyTuple_GetInteger(poArgs, 0, &nFrameSkip))
		return Py_BuildException();

	CPythonApplication::Instance().SetFrameSkip(nFrameSkip ? true : false);
	return Py_BuildNone();
}

// LOCALE

PyObject* appForceSetLocale(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	char* szLocalePath;
	if (!PyTuple_GetString(poArgs, 1, &szLocalePath))
		return Py_BuildException();

	LocaleService_ForceSetLocale(szName, szLocalePath);

	return Py_BuildNone();
}

PyObject* appGetLocaleServiceName(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", LocaleService_GetName());
}

//
bool LoadLocaleData(const char* localePath);

PyObject* appSetCHEONMA(PyObject* poSelf, PyObject* poArgs)
{
	int enable;
	if (!PyTuple_GetInteger(poArgs, 0, &enable))
		return Py_BuildException();

	LocaleService_SetCHEONMA(enable ? true : false);
	return Py_BuildNone();
}

PyObject* appIsCHEONMA(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", LocaleService_IsCHEONMA());
}

#include "../EterBase/tea.h"

PyObject* appLoadLocaleAddr(PyObject* poSelf, PyObject* poArgs)
{
	char* addrPath;
	if (!PyTuple_GetString(poArgs, 0, &addrPath))
		return Py_BuildException();

	msl::file_ptr fPtr(addrPath, "rb");
	if (!fPtr)
		return Py_BuildException();

	auto size = fPtr.size();
	auto* enc = static_cast<char*>(_alloca(size));
	fPtr.read(enc, size);

	static const unsigned char key[16] = { 0x82, 0x1b, 0x34, 0xae, 0x12, 0x3b, 0xfb, 0x17, 0xd7, 0x2c, 0x39, 0xae, 0x41, 0x98, 0xf1, 0x63 };

	auto* buf = static_cast<char*>(_alloca(size));
	tea_decrypt(reinterpret_cast<uint32_t*>(buf), reinterpret_cast<const uint32_t*>(enc), reinterpret_cast<const uint32_t*>(key), size);

	unsigned int retSize = *reinterpret_cast<unsigned int*>(buf);
	char* ret = buf + sizeof(unsigned int);
	return Py_BuildValue("s#", ret, retSize);
}

PyObject* appLoadLocaleData(PyObject* poSelf, PyObject* poArgs)
{
	char* localePath;
	if (!PyTuple_GetString(poArgs, 0, &localePath))
		return Py_BuildException();

	return Py_BuildValue("i", LoadLocaleData(localePath));
}

PyObject* appGetLocaleName(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", LocaleService_GetLocaleName());
}

PyObject* appGetLocalePath(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", LocaleService_GetLocalePath());
}
// END_OF_LOCALE

PyObject* appGetDefaultCodePage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", LocaleService_GetCodePage());
}

#include <IL/il.h>

PyObject* appGetImageInfo(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	BOOL canLoad = FALSE;
	ILuint uWidth = 0;
	ILuint uHeight = 0;

	ILuint uImg;
	ilGenImages(1, &uImg);
	ilBindImage(uImg);
	if (ilLoad(IL_TYPE_UNKNOWN, szFileName))
	{
		canLoad = TRUE;
		uWidth = ilGetInteger(IL_IMAGE_WIDTH);
		uHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	}

	ilDeleteImages(1, &uImg);

	return Py_BuildValue("iii", canLoad, uWidth, uHeight);
}

#include "../EterPack/EterPackManager.h"

PyObject* appIsExistFile(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	bool isExist = CEterPackManager::Instance().isExist(szFileName);

	return Py_BuildValue("i", isExist);
}

PyObject* appGetFileList(PyObject* poSelf, PyObject* poArgs)
{
	char* szFilter;
	if (!PyTuple_GetString(poArgs, 0, &szFilter))
		return Py_BuildException();

	PyObject* poList = PyList_New(0);

	WIN32_FIND_DATA wfd{};

	HANDLE hFind = FindFirstFile(szFilter, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			PyObject* poFileName = PyString_FromString(wfd.cFileName);
			PyList_Append(poList, poFileName);
		} while (FindNextFile(hFind, &wfd));


		FindClose(hFind);
	}

	return poList;
}


PyObject* appUpdateGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().UpdateGame();
	return Py_BuildNone();
}

PyObject* appRenderGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().RenderGame();
	return Py_BuildNone();
}


PyObject* appSetMouseHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	CPythonApplication::Instance().SetMouseHandler(poHandler);
	return Py_BuildNone();
}

PyObject* appCreate(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 1, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 2, &height))
		return Py_BuildException();

	int Windowed;
	if (!PyTuple_GetInteger(poArgs, 3, &Windowed))
		return Py_BuildException();

	CPythonApplication& rkApp = CPythonApplication::Instance();
	if (!rkApp.Create(poSelf, szName, width, height, Windowed))
	{
		//return Py_BuildNone();
		return nullptr;
	}

	return Py_BuildNone();
}

PyObject* appLoop(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().Loop();

	return Py_BuildNone();
}

PyObject* appGetInfo(PyObject* poSelf, PyObject* poArgs)
{
	int nInfo;
	if (!PyTuple_GetInteger(poArgs, 0, &nInfo))
		return Py_BuildException();

	std::string stInfo;
	CPythonApplication::Instance().GetInfo(nInfo, &stInfo);
	return Py_BuildValue("s", stInfo.c_str());
}

PyObject* appProcess(PyObject* poSelf, PyObject* poArgs)
{
	if (CPythonApplication::Instance().Process())
		return Py_BuildValue("i", 1);

	return Py_BuildValue("i", 0);
}

PyObject* appAbort(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().Abort();
	return Py_BuildNone();
}

PyObject* appExit(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().Exit();
	return Py_BuildNone();
}

PyObject* appSetCamera(PyObject* poSelf, PyObject* poArgs)
{
	float Distance;
	if (!PyTuple_GetFloat(poArgs, 0, &Distance))
		return Py_BuildException();

	float Pitch;
	if (!PyTuple_GetFloat(poArgs, 1, &Pitch))
		return Py_BuildException();

	float Rotation;
	if (!PyTuple_GetFloat(poArgs, 2, &Rotation))
		return Py_BuildException();

	float fDestinationHeight;
	if (!PyTuple_GetFloat(poArgs, 3, &fDestinationHeight))
		return Py_BuildException();

	CPythonApplication::Instance().SetCamera(Distance, Pitch, Rotation, fDestinationHeight);
	return Py_BuildNone();
}

PyObject* appGetCamera(PyObject* poSelf, PyObject* poArgs)
{
	float Distance, Pitch, Rotation, DestinationHeight;
	CPythonApplication::Instance().GetCamera(&Distance, &Pitch, &Rotation, &DestinationHeight);

	return Py_BuildValue("ffff", Distance, Pitch, Rotation, DestinationHeight);
}

PyObject* appGetCameraPitch(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetPitch());
}

PyObject* appGetCameraRotation(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetRotation());
}

PyObject* appGetTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetGlobalTime());
}

PyObject* appGetGlobalTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetServerTime());
}

PyObject* appGetGlobalTimeStamp(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetServerTimeStamp());
}

PyObject* appGetUpdateFPS(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetUpdateFPS());
}

PyObject* appGetRenderFPS(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetRenderFPS());
}

PyObject* appRotateCamera(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().RotateCamera(iDirection);
	return Py_BuildNone();
}

PyObject* appPitchCamera(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().PitchCamera(iDirection);
	return Py_BuildNone();
}

PyObject* appZoomCamera(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().ZoomCamera(iDirection);
	return Py_BuildNone();
}

PyObject* appMovieRotateCamera(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().MovieRotateCamera(iDirection);
	return Py_BuildNone();
}

PyObject* appMoviePitchCamera(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().MoviePitchCamera(iDirection);
	return Py_BuildNone();
}

PyObject* appMovieZoomCamera(PyObject* poSelf, PyObject* poArgs)
{
	int iDirection;
	if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
		return Py_BuildException();
	CPythonApplication::Instance().MovieZoomCamera(iDirection);
	return Py_BuildNone();
}

PyObject* appMovieResetCamera(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().MovieResetCamera();
	return Py_BuildNone();
}

PyObject* appGetFaceSpeed(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonApplication::Instance().GetFaceSpeed());
}

PyObject* appGetRenderTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("fi", CPythonApplication::Instance().GetAveRenderTime(), CPythonApplication::Instance().GetCurRenderTime());
}

PyObject* appGetUpdateTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetCurUpdateTime());
}

PyObject* appGetLoad(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetLoad());
}
PyObject* appGetFaceCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetFaceCount());
}

PyObject* appGetAvaiableTextureMememory(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CGraphicBase::GetAvailableTextureMemory());
}

PyObject* appSetFPS(PyObject* poSelf, PyObject* poArgs)
{
	int iFPS;
	if (!PyTuple_GetInteger(poArgs, 0, &iFPS))
		return Py_BuildException();

	CPythonApplication::Instance().SetFPS(iFPS);

	return Py_BuildNone();
}

PyObject* appSetGlobalCenterPosition(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	CPythonApplication::Instance().SetGlobalCenterPosition(x, y);
	return Py_BuildNone();
}


PyObject* appSetCenterPosition(PyObject* poSelf, PyObject* poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();

	float fy;
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();

	float fz;
	if (!PyTuple_GetFloat(poArgs, 2, &fz))
		return Py_BuildException();

	CPythonApplication::Instance().SetCenterPosition(fx, -fy, fz);
	return Py_BuildNone();
}

PyObject* appGetCursorPosition(PyObject* poSelf, PyObject* poArgs)
{
	long lx, ly;
	UI::CWindowManager& rkWndMgr = UI::CWindowManager::Instance();
	rkWndMgr.GetMousePosition(lx, ly);

	return Py_BuildValue("ii", lx, ly);
}

PyObject* appRunPythonFile(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	bool ret = CPythonLauncher::Instance().RunFile(szFileName);
	return Py_BuildValue("i", ret);
}

PyObject* appIsPressed(PyObject* poSelf, PyObject* poArgs)
{
	int iKey;
	if (!PyTuple_GetInteger(poArgs, 0, &iKey))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonApplication::Instance().IsPressed(iKey));
}

PyObject* appSetCursor(PyObject* poSelf, PyObject* poArgs)
{
	/*
		char * szName;
		if (!PyTuple_GetString(poArgs, 0, &szName))
			return Py_BuildException();

		if (!CPythonApplication::Instance().SetHardwareCursor(szName))
			return Py_BuildException("Wrong Cursor Name [%s]", szName);
	*/
	int iCursorNum;
	if (!PyTuple_GetInteger(poArgs, 0, &iCursorNum))
		return Py_BuildException();

	if (!CPythonApplication::Instance().SetCursorNum(iCursorNum))
		return Py_BuildException("Wrong Cursor Name [%d]", iCursorNum);

	return Py_BuildNone();
}

PyObject* appGetCursor(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::Instance().GetCursorNum());
}

PyObject* appShowCursor(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().SetCursorVisible(TRUE);

	return Py_BuildNone();
}

PyObject* appHideCursor(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().SetCursorVisible(FALSE);

	return Py_BuildNone();
}

PyObject* appIsShowCursor(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", TRUE == CPythonApplication::Instance().GetCursorVisible());
}

PyObject* appIsLiarCursorOn(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", TRUE == CPythonApplication::Instance().GetLiarCursorOn());
}

PyObject* appSetSoftwareCursor(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
	return Py_BuildNone();
}

PyObject* appSetHardwareCursor(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	return Py_BuildNone();
}

PyObject* appSetConnectData(PyObject* poSelf, PyObject* poArgs)
{
	char* szIP;
	if (!PyTuple_GetString(poArgs, 0, &szIP))
		return Py_BuildException();

	int iPort;
	if (!PyTuple_GetInteger(poArgs, 1, &iPort))
		return Py_BuildException();

	CPythonApplication::Instance().SetConnectData(szIP, iPort);

	return Py_BuildNone();
}

PyObject* appGetConnectData(PyObject* poSelf, PyObject* poArgs)
{
	std::string strIP;
	int iPort;

	CPythonApplication::Instance().GetConnectData(strIP, iPort);

	return Py_BuildValue("si", strIP.c_str(), iPort);
}

PyObject* appGetRandom(PyObject* poSelf, PyObject* poArgs)
{
	int from;
	if (!PyTuple_GetInteger(poArgs, 0, &from))
		return Py_BuildException();

	int to;
	if (!PyTuple_GetInteger(poArgs, 1, &to))
		return Py_BuildException();

	if (from > to)
	{
		int tmp = from;
		from = to;
		to = tmp;
	}

	return Py_BuildValue("i", random_range(from, to));
}

PyObject* appGetRotatingDirection(PyObject* poSelf, PyObject* poArgs)
{
	float fSource;
	if (!PyTuple_GetFloat(poArgs, 0, &fSource))
		return Py_BuildException();
	float fTarget;
	if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
		return Py_BuildException();

	return Py_BuildValue("i", GetRotatingDirection(fSource, fTarget));
}

PyObject* appGetDegreeDifference(PyObject* poSelf, PyObject* poArgs)
{
	float fSource;
	if (!PyTuple_GetFloat(poArgs, 0, &fSource))
		return Py_BuildException();
	float fTarget;
	if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
		return Py_BuildException();

	return Py_BuildValue("f", GetDegreeDifference(fSource, fTarget));
}

PyObject* appSleep(PyObject* poSelf, PyObject* poArgs)
{
	int iTime;
	if (!PyTuple_GetInteger(poArgs, 0, &iTime))
		return Py_BuildException();

	Sleep(iTime);

	return Py_BuildNone();
}

PyObject* appSetDefaultFontName(PyObject* poSelf, PyObject* poArgs)
{
	char* szFontName;
	if (!PyTuple_GetString(poArgs, 0, &szFontName))
		return Py_BuildException();

	// DEFAULT_FONT
	DefaultFont_SetName(szFontName);
	// END_OF_DEFAULT_FONT

	return Py_BuildNone();
}

PyObject* appSetGuildSymbolPath(PyObject* poSelf, PyObject* poArgs)
{
	char* szPathName;
	if (!PyTuple_GetString(poArgs, 0, &szPathName))
		return Py_BuildException();

	SetGuildSymbolPath(szPathName);

	return Py_BuildNone();
}

PyObject* appEnableSpecialCameraMode(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().EnableSpecialCameraMode();
	return Py_BuildNone();
}

PyObject* appSetCameraSpeed(PyObject* poSelf, PyObject* poArgs)
{
	int iPercentage;
	if (!PyTuple_GetInteger(poArgs, 0, &iPercentage))
		return Py_BuildException();

	CPythonApplication::Instance().SetCameraSpeed(iPercentage);

	CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (pCamera)
		pCamera->SetResistance(float(iPercentage) / 100.0f);
	return Py_BuildNone();
}

PyObject* appIsFileExist(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	return Py_BuildValue("i", -1 != _access(szFileName, 0));
}

PyObject* appSetCameraSetting(PyObject* poSelf, PyObject* poArgs)
{
	int ix;
	if (!PyTuple_GetInteger(poArgs, 0, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 1, &iy))
		return Py_BuildException();
	int iz;
	if (!PyTuple_GetInteger(poArgs, 2, &iz))
		return Py_BuildException();

	int iZoom;
	if (!PyTuple_GetInteger(poArgs, 3, &iZoom))
		return Py_BuildException();
	int iRotation;
	if (!PyTuple_GetInteger(poArgs, 4, &iRotation))
		return Py_BuildException();
	int iPitch;
	if (!PyTuple_GetInteger(poArgs, 5, &iPitch))
		return Py_BuildException();

	CPythonApplication::SCameraSetting CameraSetting;
	ZeroMemory(&CameraSetting, sizeof(CameraSetting));
	CameraSetting.v3CenterPosition.x = float(ix);
	CameraSetting.v3CenterPosition.y = float(iy);
	CameraSetting.v3CenterPosition.z = float(iz);
	CameraSetting.fZoom = float(iZoom);
	CameraSetting.fRotation = float(iRotation);
	CameraSetting.fPitch = float(iPitch);
	CPythonApplication::Instance().SetEventCamera(CameraSetting);
	return Py_BuildNone();
}

PyObject* appSaveCameraSetting(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonApplication::Instance().SaveCameraSetting(szFileName);
	return Py_BuildNone();
}

PyObject* appLoadCameraSetting(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	bool bResult = CPythonApplication::Instance().LoadCameraSetting(szFileName);
	return Py_BuildValue("i", bResult);
}

PyObject* appSetDefaultCamera(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().SetDefaultCamera();
	return Py_BuildNone();
}

PyObject* appSetSightRange(PyObject* poSelf, PyObject* poArgs)
{
	int iRange;
	if (!PyTuple_GetInteger(poArgs, 0, &iRange))
		return Py_BuildException();

	CPythonApplication::Instance().SetForceSightRange(iRange);
	return Py_BuildNone();
}

extern int g_iAccumulationTime;

PyObject* apptestGetAccumulationTime(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", g_iAccumulationTime);
}

PyObject* apptestResetAccumulationTime(PyObject* poSelf, PyObject* poArgs)
{
	g_iAccumulationTime = 0;
	return Py_BuildNone();
}

PyObject* apptestSetSpecularColor(PyObject* poSelf, PyObject* poArgs)
{
	float fr;
	if (!PyTuple_GetFloat(poArgs, 0, &fr))
		return Py_BuildException();
	float fg;
	if (!PyTuple_GetFloat(poArgs, 1, &fg))
		return Py_BuildException();
	float fb;
	if (!PyTuple_GetFloat(poArgs, 2, &fb))
		return Py_BuildException();
	g_fSpecularColor = D3DXCOLOR(fr, fg, fb, 1.0f);
	return Py_BuildNone();
}

PyObject* appSetVisibleNotice(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();
	bVisibleNotice = iFlag;
	return Py_BuildNone();
}

PyObject* appIsVisibleNotice(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", bVisibleNotice);
}

PyObject* appEnableTestServerFlag(PyObject* poSelf, PyObject* poArgs)
{
	bTestServerFlag = TRUE;
	return Py_BuildNone();
}

PyObject* appIsEnableTestServerFlag(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", bTestServerFlag);
}

class CTextLineLoader
{
public:
	CTextLineLoader(const char* c_szFileName)
	{
		const VOID* pvData;
		CMappedFile kFile;
		if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
			return;

		m_kTextFileLoader.Bind(kFile.Size(), pvData);
	}

	uint32_t GetLineCount() const { return m_kTextFileLoader.GetLineCount(); }

	const char* GetLine(uint32_t dwIndex)
	{
		if (dwIndex >= GetLineCount())
			return "";

		return m_kTextFileLoader.GetLineString(dwIndex).c_str();
	}

protected:
	CMemoryTextFileLoader m_kTextFileLoader;
};

PyObject* appOpenTextFile(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	auto* pTextLineLoader = new CTextLineLoader(szFileName);
	return Py_BuildValue("i", (int)pTextLineLoader);
}

PyObject* appCloseTextFile(PyObject* poSelf, PyObject* poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BuildException();

	auto* pTextFileLoader = (CTextLineLoader*)iHandle;
	delete pTextFileLoader;

	return Py_BuildNone();
}

PyObject* appGetTextFileLineCount(PyObject* poSelf, PyObject* poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BuildException();

	auto* pTextFileLoader = (CTextLineLoader*)iHandle;
	return Py_BuildValue("i", pTextFileLoader->GetLineCount());
}

PyObject* appGetTextFileLine(PyObject* poSelf, PyObject* poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BuildException();
	int iLineIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iLineIndex))
		return Py_BuildException();

	auto* pTextFileLoader = (CTextLineLoader*)iHandle;
	return Py_BuildValue("s", pTextFileLoader->GetLine(iLineIndex));
}

PyObject* appSetGuildMarkPath(PyObject* poSelf, PyObject* poArgs)
{
	char* path;
	if (!PyTuple_GetString(poArgs, 0, &path))
		return Py_BuildException();

	char newPath[256];
	char* ext = strstr(path, ".tga");

	if (ext)
	{
		int extPos = ext - path;
		strncpy(newPath, path, extPos);
		newPath[extPos] = '\0';
	}
	else
		strncpy(newPath, path, sizeof(newPath) - 1);

	CGuildMarkManager::Instance().SetMarkPathPrefix(newPath);
	return Py_BuildNone();
}

PyObject* appIsDevStage(PyObject* poSelf, PyObject* poArgs)
{
	int nIsDevelopmentStage = 0;
#if defined(LOCALE_SERVICE_STAGE_DEVELOPMENT)
	nIsDevelopmentStage = 1;
#endif
	return Py_BuildValue("i", nIsDevelopmentStage);
}

PyObject* appIsTestStage(PyObject* poSelf, PyObject* poArgs)
{
	int nIsTestStage = 0;
#if defined(LOCALE_SERVICE_STAGE_TEST)
	nIsTestStage = 1;
#endif
	return Py_BuildValue("i", nIsTestStage);
}

PyObject* appIsLiveStage(PyObject* poSelf, PyObject* poArgs)
{
	int nIsLiveStage = 0;
#if !defined(LOCALE_SERVICE_STAGE_TEST) && !defined(LOCALE_SERVICE_STAGE_DEVELOPMENT)
	nIsLiveStage = 1;
#endif
	return Py_BuildValue("i", nIsLiveStage);
}

PyObject* appLogoOpen(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	int nIsSuccess = 1; //CPythonApplication::Instance().OnLogoOpen(szName);
	CMovieMan::Instance().PlayLogo(szName);

	return Py_BuildValue("i", nIsSuccess);
}

PyObject* appLogoUpdate(PyObject* poSelf, PyObject* poArgs)
{
	int nIsRun = 0; //CPythonApplication::Instance().OnLogoUpdate();
	return Py_BuildValue("i", nIsRun);
}

PyObject* appLogoRender(PyObject* poSelf, PyObject* poArgs)
{
	//CPythonApplication::Instance().OnLogoRender();
	return Py_BuildNone();
}

PyObject* appLogoClose(PyObject* poSelf, PyObject* poArgs)
{
	//CPythonApplication::Instance().OnLogoClose();
	return Py_BuildNone();
}

#ifdef ENABLE_FLASH_APLICATION
PyObject* appFlashApplication(PyObject* poSelf, PyObject* poArgs)	//@fixme403
{
	CPythonApplication::Instance().FlashApplication();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
PyObject* appUpdateLanguageLocale(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bLanguage;
	if (!PyTuple_GetInteger(poArgs, 0, &bLanguage))
		return Py_BuildException();

	UpdateLanguageLocale(bLanguage);
	return Py_BuildNone();
}

///////////////////////////////////
PyObject* appGetLanguageIconPath(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bLanguage;
	if (!PyTuple_GetInteger(poArgs, 0, &bLanguage))
		return Py_BuildException();

	return Py_BuildValue("s", GetLanguageIconPath(bLanguage).c_str());
}
#endif

#ifdef ENABLE_MAP_LOCATION_APP_NAME
PyObject* appSetText(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonApplication& rkApp = CPythonApplication::Instance();
	rkApp.SetText(szName);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* appGetLoginType(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::LOGIN_TYPE_NONE);
}

PyObject* appVERSION(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonApplication::VERSION_NUMBER);
}
#endif

PyObject* appCHEQUE_INPUT_LIMIT(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CHEQUE_MAX);
}

#ifdef ENABLE_RENDER_LOGIN_EFFECTS
PyObject* appRenderEffects(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().RenderEffects();
	return Py_BuildNone();
}

PyObject* appUpdateEffects(PyObject* poSelf, PyObject* poArgs)
{
	CPythonApplication::Instance().UpdateEffects();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MYSHOP_DECO
PyObject* appMyShopDecoBGCreate(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
#	include "PythonYutnoriManager.h"
PyObject* appYutnoriCreate(PyObject* poSelf, PyObject* poArgs)
{
	CPythonYutnoriManager::Instance().Initialize();
	CPythonYutnoriManager::Instance().CreateModelInstance();

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MULTIFARM_BLOCK
PyObject* appSetMultiFarmExeIcon(PyObject* poSelf, PyObject* poArgs)
{
	int iStatus;
	if (!PyTuple_GetInteger(poArgs, 0, &iStatus))
		return Py_BuildException();

	CPythonApplication::Instance().MultiFarmBlockIcon(iStatus);
	return Py_BuildNone();
}
#endif

void initapp()
{
	static PyMethodDef s_methods[] = { {"IsDevStage", appIsDevStage, METH_VARARGS},
									  {"IsTestStage", appIsTestStage, METH_VARARGS},
									  {"IsLiveStage", appIsLiveStage, METH_VARARGS},

		// TEXTTAIL_LIVINGTIME_CONTROL
		{"SetTextTailLivingTime", appSetTextTailLivingTime, METH_VARARGS},
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		{"EnablePerformanceTime", appEnablePerformanceTime, METH_VARARGS},
		{"SetHairColorEnable", appSetHairColorEnable, METH_VARARGS},

		{"SetArmorSpecularEnable", appSetArmorSpecularEnable, METH_VARARGS},
		{"SetWeaponSpecularEnable", appSetWeaponSpecularEnable, METH_VARARGS},
		{"SetSkillEffectUpgradeEnable", appSetSkillEffectUpgradeEnable, METH_VARARGS},
		{"SetTwoHandedWeaponAttSpeedDecreaseValue", SetTwoHandedWeaponAttSpeedDecreaseValue, METH_VARARGS},
		{"SetRideHorseEnable", appSetRideHorseEnable, METH_VARARGS},

		{"SetCameraMaxDistance", appSetCameraMaxDistance, METH_VARARGS},
		{"SetMinFog", appSetMinFog, METH_VARARGS},
		{"SetFrameSkip", appSetFrameSkip, METH_VARARGS},
		{"GetImageInfo", appGetImageInfo, METH_VARARGS},
		{"GetInfo", appGetInfo, METH_VARARGS},
		{"UpdateGame", appUpdateGame, METH_VARARGS},
		{"RenderGame", appRenderGame, METH_VARARGS},
		{"Loop", appLoop, METH_VARARGS},
		{"Create", appCreate, METH_VARARGS},
		{"Process", appProcess, METH_VARARGS},
		{"Exit", appExit, METH_VARARGS},
		{"Abort", appAbort, METH_VARARGS},
		{"SetMouseHandler", appSetMouseHandler, METH_VARARGS},
		{"IsExistFile", appIsExistFile, METH_VARARGS},
		{"GetFileList", appGetFileList, METH_VARARGS},

		{"SetCamera", appSetCamera, METH_VARARGS},
		{"GetCamera", appGetCamera, METH_VARARGS},
		{"GetCameraPitch", appGetCameraPitch, METH_VARARGS},
		{"GetCameraRotation", appGetCameraRotation, METH_VARARGS},
		{"GetTime", appGetTime, METH_VARARGS},
		{"GetGlobalTime", appGetGlobalTime, METH_VARARGS},
		{"GetGlobalTimeStamp", appGetGlobalTimeStamp, METH_VARARGS},
		{"GetUpdateFPS", appGetUpdateFPS, METH_VARARGS},
		{"GetRenderFPS", appGetRenderFPS, METH_VARARGS},
		{"RotateCamera", appRotateCamera, METH_VARARGS},
		{"PitchCamera", appPitchCamera, METH_VARARGS},
		{"ZoomCamera", appZoomCamera, METH_VARARGS},
		{"MovieRotateCamera", appMovieRotateCamera, METH_VARARGS},
		{"MoviePitchCamera", appMoviePitchCamera, METH_VARARGS},
		{"MovieZoomCamera", appMovieZoomCamera, METH_VARARGS},
		{"MovieResetCamera", appMovieResetCamera, METH_VARARGS},

		{"GetAvailableTextureMemory", appGetAvaiableTextureMememory, METH_VARARGS},
		{"GetRenderTime", appGetRenderTime, METH_VARARGS},
		{"GetUpdateTime", appGetUpdateTime, METH_VARARGS},
		{"GetLoad", appGetLoad, METH_VARARGS},
		{"GetFaceSpeed", appGetFaceSpeed, METH_VARARGS},
		{"GetFaceCount", appGetFaceCount, METH_VARARGS},
		{"SetFPS", appSetFPS, METH_VARARGS},
		{"SetGlobalCenterPosition", appSetGlobalCenterPosition, METH_VARARGS},
		{"SetCenterPosition", appSetCenterPosition, METH_VARARGS},
		{"GetCursorPosition", appGetCursorPosition, METH_VARARGS},

		{"GetRandom", appGetRandom, METH_VARARGS},
		{"RunPythonFile", appRunPythonFile, METH_VARARGS},
		{"IsWebPageMode", appIsWebPageMode, METH_VARARGS},
		{"ShowWebPage", appShowWebPage, METH_VARARGS},
		{"MoveWebPage", appMoveWebPage, METH_VARARGS},
		{"HideWebPage", appHideWebPage, METH_VARARGS},
		{"IsPressed", appIsPressed, METH_VARARGS},
		{"SetCursor", appSetCursor, METH_VARARGS},
		{"GetCursor", appGetCursor, METH_VARARGS},
		{"ShowCursor", appShowCursor, METH_VARARGS},
		{"HideCursor", appHideCursor, METH_VARARGS},
		{"IsShowCursor", appIsShowCursor, METH_VARARGS},
		{"IsLiarCursorOn", appIsLiarCursorOn, METH_VARARGS},
		{"SetSoftwareCursor", appSetSoftwareCursor, METH_VARARGS},
		{"SetHardwareCursor", appSetHardwareCursor, METH_VARARGS},

		{"SetConnectData", appSetConnectData, METH_VARARGS},
		{"GetConnectData", appGetConnectData, METH_VARARGS},

		{"GetRotatingDirection", appGetRotatingDirection, METH_VARARGS},
		{"GetDegreeDifference", appGetDegreeDifference, METH_VARARGS},
		{"Sleep", appSleep, METH_VARARGS},
		{"SetDefaultFontName", appSetDefaultFontName, METH_VARARGS},
		{"SetGuildSymbolPath", appSetGuildSymbolPath, METH_VARARGS},

		{"EnableSpecialCameraMode", appEnableSpecialCameraMode, METH_VARARGS},
		{"SetCameraSpeed", appSetCameraSpeed, METH_VARARGS},

		{"SaveCameraSetting", appSaveCameraSetting, METH_VARARGS},
		{"LoadCameraSetting", appLoadCameraSetting, METH_VARARGS},
		{"SetDefaultCamera", appSetDefaultCamera, METH_VARARGS},
		{"SetCameraSetting", appSetCameraSetting, METH_VARARGS},

		{"SetSightRange", appSetSightRange, METH_VARARGS},

		{"IsFileExist", appIsFileExist, METH_VARARGS},
		{"OpenTextFile", appOpenTextFile, METH_VARARGS},
		{"CloseTextFile", appCloseTextFile, METH_VARARGS},
		{"GetTextFileLineCount", appGetTextFileLineCount, METH_VARARGS},
		{"GetTextFileLine", appGetTextFileLine, METH_VARARGS},

		// LOCALE
		{"GetLocaleServiceName", appGetLocaleServiceName, METH_VARARGS},
		{"GetLocaleName", appGetLocaleName, METH_VARARGS},
		{"GetLocalePath", appGetLocalePath, METH_VARARGS},
		{"ForceSetLocale", appForceSetLocale, METH_VARARGS},
		// END_OF_LOCALE

		// CHEONMA
		{"LoadLocaleAddr", appLoadLocaleAddr, METH_VARARGS},
		{"LoadLocaleData", appLoadLocaleData, METH_VARARGS},
		{"SetCHEONMA", appSetCHEONMA, METH_VARARGS},
		{"IsCHEONMA", appIsCHEONMA, METH_VARARGS},
		// END_OF_CHEONMA

		{"GetDefaultCodePage", appGetDefaultCodePage, METH_VARARGS},
		{"SetControlFP", appSetControlFP, METH_VARARGS},
		{"SetSpecularSpeed", appSetSpecularSpeed, METH_VARARGS},

		{"testGetAccumulationTime", apptestGetAccumulationTime, METH_VARARGS},
		{"testResetAccumulationTime", apptestResetAccumulationTime, METH_VARARGS},
		{"testSetSpecularColor", apptestSetSpecularColor, METH_VARARGS},

		{"SetVisibleNotice", appSetVisibleNotice, METH_VARARGS},
		{"IsVisibleNotice", appIsVisibleNotice, METH_VARARGS},
		{"EnableTestServerFlag", appEnableTestServerFlag, METH_VARARGS},
		{"IsEnableTestServerFlag", appIsEnableTestServerFlag, METH_VARARGS},

		{"SetGuildMarkPath", appSetGuildMarkPath, METH_VARARGS},

		{"OnLogoUpdate", appLogoUpdate, METH_VARARGS},
		{"OnLogoRender", appLogoRender, METH_VARARGS},
		{"OnLogoOpen", appLogoOpen, METH_VARARGS},
		{"OnLogoClose", appLogoClose, METH_VARARGS},
#ifdef ENABLE_FLASH_APLICATION
									  { "FlashApplication", appFlashApplication, METH_VARARGS },	//@fixme403
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
									  { "UpdateLanguageLocale",		appUpdateLanguageLocale,			METH_VARARGS },
		///////////////////////////////////
		{ "GetLanguageIconPath",		appGetLanguageIconPath,			METH_VARARGS },
#endif
#ifdef ENABLE_MAP_LOCATION_APP_NAME
		{ "SetText",					appSetText,						METH_VARARGS },
#endif
#ifdef ENABLE_OFFICAL_FEATURES
		{ "GetLoginType", appGetLoginType, METH_VARARGS },
		{ "VERSION", appVERSION, METH_VARARGS },
#endif
#ifdef ENABLE_RENDER_LOGIN_EFFECTS
		{ "RenderEffects",				appRenderEffects,				METH_VARARGS },
		{ "UpdateEffects",				appUpdateEffects,				METH_VARARGS },
#endif
		{ "CHEQUE_INPUT_LIMIT", appCHEQUE_INPUT_LIMIT, METH_VARARGS },

#ifdef ENABLE_MYSHOP_DECO
									  { "MyShopDecoBGCreate", appMyShopDecoBGCreate, METH_VARARGS },
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
									  { "YutnoriCreate", appYutnoriCreate, METH_VARARGS },
#endif

#ifdef ENABLE_MULTIFARM_BLOCK
									  { "SetMultiFarmExeIcon", 				appSetMultiFarmExeIcon,					METH_VARARGS },
#endif

									  {nullptr, nullptr} };

	PyObject* poModule = Py_InitModule("app", s_methods);
	PyModule_AddIntConstant(poModule, "INFO_ITEM", CPythonApplication::INFO_ITEM);
	PyModule_AddIntConstant(poModule, "INFO_ACTOR", CPythonApplication::INFO_ACTOR);
	PyModule_AddIntConstant(poModule, "INFO_EFFECT", CPythonApplication::INFO_EFFECT);
	PyModule_AddIntConstant(poModule, "INFO_TEXTTAIL", CPythonApplication::INFO_TEXTTAIL);

	PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_SAME", DEGREE_DIRECTION_SAME);
	PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_RIGHT", DEGREE_DIRECTION_RIGHT);
	PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_LEFT", DEGREE_DIRECTION_LEFT);

	PyModule_AddIntConstant(poModule, "VK_LEFT", VK_LEFT);
	PyModule_AddIntConstant(poModule, "VK_RIGHT", VK_RIGHT);
	PyModule_AddIntConstant(poModule, "VK_UP", VK_UP);
	PyModule_AddIntConstant(poModule, "VK_DOWN", VK_DOWN);
	PyModule_AddIntConstant(poModule, "VK_HOME", VK_HOME);
	PyModule_AddIntConstant(poModule, "VK_END", VK_END);
	PyModule_AddIntConstant(poModule, "VK_DELETE", VK_DELETE);

	PyModule_AddIntConstant(poModule, "DIK_ESCAPE", DIK_ESCAPE);
	PyModule_AddIntConstant(poModule, "DIK_ESC", DIK_ESCAPE); // 편의를 위해
	PyModule_AddIntConstant(poModule, "DIK_1", DIK_1);
	PyModule_AddIntConstant(poModule, "DIK_2", DIK_2);
	PyModule_AddIntConstant(poModule, "DIK_3", DIK_3);
	PyModule_AddIntConstant(poModule, "DIK_4", DIK_4);
	PyModule_AddIntConstant(poModule, "DIK_5", DIK_5);
	PyModule_AddIntConstant(poModule, "DIK_6", DIK_6);
	PyModule_AddIntConstant(poModule, "DIK_7", DIK_7);
	PyModule_AddIntConstant(poModule, "DIK_8", DIK_8);
	PyModule_AddIntConstant(poModule, "DIK_9", DIK_9);
	PyModule_AddIntConstant(poModule, "DIK_0", DIK_0);
	PyModule_AddIntConstant(poModule, "DIK_MINUS", DIK_MINUS); /* - on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_EQUALS", DIK_EQUALS);
	PyModule_AddIntConstant(poModule, "DIK_BACK", DIK_BACK); /* backspace */
	PyModule_AddIntConstant(poModule, "DIK_TAB", DIK_TAB);
	PyModule_AddIntConstant(poModule, "DIK_Q", DIK_Q);
	PyModule_AddIntConstant(poModule, "DIK_W", DIK_W);
	PyModule_AddIntConstant(poModule, "DIK_E", DIK_E);
	PyModule_AddIntConstant(poModule, "DIK_R", DIK_R);
	PyModule_AddIntConstant(poModule, "DIK_T", DIK_T);
	PyModule_AddIntConstant(poModule, "DIK_Y", DIK_Y);
	PyModule_AddIntConstant(poModule, "DIK_U", DIK_U);
	PyModule_AddIntConstant(poModule, "DIK_I", DIK_I);
	PyModule_AddIntConstant(poModule, "DIK_O", DIK_O);
	PyModule_AddIntConstant(poModule, "DIK_P", DIK_P);
	PyModule_AddIntConstant(poModule, "DIK_LBRACKET", DIK_LBRACKET);
	PyModule_AddIntConstant(poModule, "DIK_RBRACKET", DIK_RBRACKET);
	PyModule_AddIntConstant(poModule, "DIK_RETURN", DIK_RETURN); /* Enter on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_LCONTROL", DIK_LCONTROL);
	PyModule_AddIntConstant(poModule, "DIK_A", DIK_A);
	PyModule_AddIntConstant(poModule, "DIK_S", DIK_S);
	PyModule_AddIntConstant(poModule, "DIK_D", DIK_D);
	PyModule_AddIntConstant(poModule, "DIK_F", DIK_F);
	PyModule_AddIntConstant(poModule, "DIK_G", DIK_G);
	PyModule_AddIntConstant(poModule, "DIK_H", DIK_H);
	PyModule_AddIntConstant(poModule, "DIK_J", DIK_J);
	PyModule_AddIntConstant(poModule, "DIK_K", DIK_K);
	PyModule_AddIntConstant(poModule, "DIK_L", DIK_L);
	PyModule_AddIntConstant(poModule, "DIK_SEMICOLON", DIK_SEMICOLON);
	PyModule_AddIntConstant(poModule, "DIK_APOSTROPHE", DIK_APOSTROPHE);
	PyModule_AddIntConstant(poModule, "DIK_GRAVE", DIK_GRAVE); /* accent grave */
	PyModule_AddIntConstant(poModule, "DIK_LSHIFT", DIK_LSHIFT);
	PyModule_AddIntConstant(poModule, "DIK_BACKSLASH", DIK_BACKSLASH);
	PyModule_AddIntConstant(poModule, "DIK_Z", DIK_Z);
	PyModule_AddIntConstant(poModule, "DIK_X", DIK_X);
	PyModule_AddIntConstant(poModule, "DIK_C", DIK_C);
	PyModule_AddIntConstant(poModule, "DIK_V", DIK_V);
	PyModule_AddIntConstant(poModule, "DIK_B", DIK_B);
	PyModule_AddIntConstant(poModule, "DIK_N", DIK_N);
	PyModule_AddIntConstant(poModule, "DIK_M", DIK_M);
	PyModule_AddIntConstant(poModule, "DIK_COMMA", DIK_COMMA);
	PyModule_AddIntConstant(poModule, "DIK_PERIOD", DIK_PERIOD); /* . on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_SLASH", DIK_SLASH); /* / on main keyboard */
	PyModule_AddIntConstant(poModule, "DIK_RSHIFT", DIK_RSHIFT);
	PyModule_AddIntConstant(poModule, "DIK_MULTIPLY", DIK_MULTIPLY); /* * on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_LALT", DIK_LMENU); /* left Alt */
	PyModule_AddIntConstant(poModule, "DIK_SPACE", DIK_SPACE);
	PyModule_AddIntConstant(poModule, "DIK_CAPITAL", DIK_CAPITAL);
	PyModule_AddIntConstant(poModule, "DIK_F1", DIK_F1);
	PyModule_AddIntConstant(poModule, "DIK_F2", DIK_F2);
	PyModule_AddIntConstant(poModule, "DIK_F3", DIK_F3);
	PyModule_AddIntConstant(poModule, "DIK_F4", DIK_F4);
	PyModule_AddIntConstant(poModule, "DIK_F5", DIK_F5);
	PyModule_AddIntConstant(poModule, "DIK_F6", DIK_F6);
	PyModule_AddIntConstant(poModule, "DIK_F7", DIK_F7);
	PyModule_AddIntConstant(poModule, "DIK_F8", DIK_F8);
	PyModule_AddIntConstant(poModule, "DIK_F9", DIK_F9);
	PyModule_AddIntConstant(poModule, "DIK_F10", DIK_F10);
	PyModule_AddIntConstant(poModule, "DIK_NUMLOCK", DIK_NUMLOCK);
	PyModule_AddIntConstant(poModule, "DIK_SCROLL", DIK_SCROLL); /* Scroll Lock */
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD7", DIK_NUMPAD7);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD8", DIK_NUMPAD8);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD9", DIK_NUMPAD9);
	PyModule_AddIntConstant(poModule, "DIK_SUBTRACT", DIK_SUBTRACT); /* - on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD4", DIK_NUMPAD4);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD5", DIK_NUMPAD5);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD6", DIK_NUMPAD6);
	PyModule_AddIntConstant(poModule, "DIK_ADD", DIK_ADD); /* + on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD1", DIK_NUMPAD1);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD2", DIK_NUMPAD2);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD3", DIK_NUMPAD3);
	PyModule_AddIntConstant(poModule, "DIK_NUMPAD0", DIK_NUMPAD0);
	PyModule_AddIntConstant(poModule, "DIK_DECIMAL", DIK_DECIMAL); /* . on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_F11", DIK_F11);
	PyModule_AddIntConstant(poModule, "DIK_F12", DIK_F12);
	PyModule_AddIntConstant(poModule, "DIK_NEXTTRACK", DIK_NEXTTRACK); /* Next Track */
	PyModule_AddIntConstant(poModule, "DIK_NUMPADENTER", DIK_NUMPADENTER); /* Enter on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_RCONTROL", DIK_RCONTROL);
	PyModule_AddIntConstant(poModule, "DIK_MUTE", DIK_MUTE); /* Mute */
	PyModule_AddIntConstant(poModule, "DIK_CALCULATOR", DIK_CALCULATOR); /* Calculator */
	PyModule_AddIntConstant(poModule, "DIK_PLAYPAUSE", DIK_PLAYPAUSE); /* Play / Pause */
	PyModule_AddIntConstant(poModule, "DIK_MEDIASTOP", DIK_MEDIASTOP); /* Media Stop */
	PyModule_AddIntConstant(poModule, "DIK_VOLUMEDOWN", DIK_VOLUMEDOWN); /* Volume - */
	PyModule_AddIntConstant(poModule, "DIK_VOLUMEUP", DIK_VOLUMEUP); /* Volume + */
	PyModule_AddIntConstant(poModule, "DIK_WEBHOME", DIK_WEBHOME); /* Web home */
	PyModule_AddIntConstant(poModule, "DIK_NUMPADCOMMA", DIK_NUMPADCOMMA); /* , on numeric keypad (NEC PC98) */
	PyModule_AddIntConstant(poModule, "DIK_DIVIDE", DIK_DIVIDE); /* / on numeric keypad */
	PyModule_AddIntConstant(poModule, "DIK_SYSRQ", DIK_SYSRQ);
	PyModule_AddIntConstant(poModule, "DIK_RALT", DIK_RMENU); /* right Alt */
	PyModule_AddIntConstant(poModule, "DIK_PAUSE", DIK_PAUSE); /* Pause */
	PyModule_AddIntConstant(poModule, "DIK_HOME", DIK_HOME); /* Home on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_UP", DIK_UP); /* UpArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_PGUP", DIK_PRIOR); /* PgUp on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_LEFT", DIK_LEFT); /* LeftArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_RIGHT", DIK_RIGHT); /* RightArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_END", DIK_END); /* End on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_DOWN", DIK_DOWN); /* DownArrow on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_PGDN", DIK_NEXT); /* PgDn on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_INSERT", DIK_INSERT); /* Insert on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_DELETE", DIK_DELETE); /* Delete on arrow keypad */
	PyModule_AddIntConstant(poModule, "DIK_LWIN", DIK_LWIN); /* Left Windows key */
	PyModule_AddIntConstant(poModule, "DIK_RWIN", DIK_RWIN); /* Right Windows key */
	PyModule_AddIntConstant(poModule, "DIK_APPS", DIK_APPS); /* AppMenu key */

	// Cursor
	PyModule_AddIntConstant(poModule, "NORMAL", CPythonApplication::CURSOR_SHAPE_NORMAL);
	PyModule_AddIntConstant(poModule, "ATTACK", CPythonApplication::CURSOR_SHAPE_ATTACK);
	PyModule_AddIntConstant(poModule, "TARGET", CPythonApplication::CURSOR_SHAPE_TARGET);
	PyModule_AddIntConstant(poModule, "TALK", CPythonApplication::CURSOR_SHAPE_TALK);
	PyModule_AddIntConstant(poModule, "CANT_GO", CPythonApplication::CURSOR_SHAPE_CANT_GO);
	PyModule_AddIntConstant(poModule, "PICK", CPythonApplication::CURSOR_SHAPE_PICK);
	PyModule_AddIntConstant(poModule, "FISH", CPythonApplication::CURSOR_SHAPE_FISH);	//ENABLE_FISHING_RENEWAL

	PyModule_AddIntConstant(poModule, "DOOR", CPythonApplication::CURSOR_SHAPE_DOOR);
	PyModule_AddIntConstant(poModule, "CHAIR", CPythonApplication::CURSOR_SHAPE_CHAIR);
	PyModule_AddIntConstant(poModule, "MAGIC", CPythonApplication::CURSOR_SHAPE_MAGIC);
	PyModule_AddIntConstant(poModule, "BUY", CPythonApplication::CURSOR_SHAPE_BUY);
	PyModule_AddIntConstant(poModule, "SELL", CPythonApplication::CURSOR_SHAPE_SELL);

	PyModule_AddIntConstant(poModule, "CAMERA_ROTATE", CPythonApplication::CURSOR_SHAPE_CAMERA_ROTATE);
	PyModule_AddIntConstant(poModule, "HSIZE", CPythonApplication::CURSOR_SHAPE_HSIZE);
	PyModule_AddIntConstant(poModule, "VSIZE", CPythonApplication::CURSOR_SHAPE_VSIZE);
	PyModule_AddIntConstant(poModule, "HVSIZE", CPythonApplication::CURSOR_SHAPE_HVSIZE);

	PyModule_AddIntConstant(poModule, "CAMERA_TO_POSITIVE", CPythonApplication::CAMERA_TO_POSITIVE);
	PyModule_AddIntConstant(poModule, "CAMERA_TO_NEGATIVE", CPythonApplication::CAMERA_TO_NEGITIVE);
	PyModule_AddIntConstant(poModule, "CAMERA_STOP", CPythonApplication::CAMERA_STOP);

#ifdef ENABLE_OFFICAL_FEATURES
	PyModule_AddIntConstant(poModule, "LOGIN_TYPE_NONE", CPythonApplication::LOGIN_TYPE_NONE);
	PyModule_AddIntConstant(poModule, "LOGIN_TYPE_STEAM", CPythonApplication::LOGIN_TYPE_STEAM);
	PyModule_AddIntConstant(poModule, "LOGIN_TYPE_TNT", CPythonApplication::LOGIN_TYPE_TNT);
#endif

#ifdef ENABLE_DELETE_FAILURE_TYPE
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_NORMAL", CPythonApplication::DELETE_FAILURE_NORMAL);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_HAVE_SEALED_ITEM", CPythonApplication::DELETE_FAILURE_HAVE_SEALED_ITEM);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_PRIVATE_CODE_ERROR", CPythonApplication::DELETE_FAILURE_PRIVATE_CODE_ERROR);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_LIMITE_LEVEL_HIGHER", CPythonApplication::DELETE_FAILURE_LIMITE_LEVEL_HIGHER);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_LIMITE_LEVEL_LOWER", CPythonApplication::DELETE_FAILURE_LIMITE_LEVEL_LOWER);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_REMAIN_TIME", CPythonApplication::DELETE_FAILURE_REMAIN_TIME);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_GUILD_MEMBER", CPythonApplication::DELETE_FAILURE_GUILD_MEMBER);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_MARRIAGE", CPythonApplication::DELETE_FAILURE_MARRIAGE);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_LAST_CHAR_SAFEBOX", CPythonApplication::DELETE_FAILURE_LAST_CHAR_SAFEBOX);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_ATTR67", CPythonApplication::DELETE_FAILURE_ATTR67);
	PyModule_AddIntConstant(poModule, "DELETE_FAILURE_PREMIUM_PRIVATE_SHOP", CPythonApplication::DELETE_FAILURE_PREMIUM_PRIVATE_SHOP);
#endif

	// Application Defines
	//------SECURITY------>
#ifdef ENABLE_DEBUG_GUI
	PyModule_AddIntConstant(poModule, "ENABLE_DEBUG_GUI", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DEBUG_GUI", false);
#endif




	//-----PROTO_STRUCT---->
#ifdef ENABLE_PROTO_RENEWAL_CUSTOM
	PyModule_AddIntConstant(poModule, "ENABLE_PROTO_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PROTO_RENEWAL", false);
#endif

#if defined(ENABLE_PROTO_RENEWAL_CUSTOM) && defined(ENABLE_WOLFMAN_CHARACTER)
	PyModule_AddIntConstant(poModule, "ENABLE_PROTO_RENEWAL_CUSTOM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PROTO_RENEWAL_CUSTOM", false);
#endif




	//------CHARACTERS------>
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	PyModule_AddIntConstant(poModule, "ENABLE_PLAYER_PER_ACCOUNT5", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PLAYER_PER_ACCOUNT5", false);
#endif
	PyModule_AddIntConstant(poModule, "PLAYER_PER_ACCOUNT", PLAYER_PER_ACCOUNT);

#ifdef ENABLE_WOLFMAN_CHARACTER
	PyModule_AddIntConstant(poModule, "ENABLE_WOLFMAN_CHARACTER", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WOLFMAN_CHARACTER", false);
#endif





	//--------SYSTEMS------->
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	PyModule_AddIntConstant(poModule, "WJ_ENABLE_PICKUP_ITEM_EFFECT", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_ENABLE_PICKUP_ITEM_EFFECT", false);
#endif

#ifdef WJ_SHOW_MOB_INFO
	PyModule_AddIntConstant(poModule, "WJ_SHOW_MOB_INFO", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_SHOW_MOB_INFO", false);
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
	PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_ENABLE_TRADABLE_ICON", false);
#endif

#ifdef ENABLE_INVENTORY_SORT
	PyModule_AddIntConstant(poModule, "ENABLE_INVENTORY_SORT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_INVENTORY_SORT", false);
#endif



	/*---Main Systems*///--->
#ifdef ENABLE_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_PET_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "ENABLE_ACTIVE_PET_EFFECT", false);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PET_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_ACTIVE_PET_EFFECT", true);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SYSTEM", true);
#	ifdef ENABLE_PET_ATTR_DETERMINE
	PyModule_AddIntConstant(poModule, "ENABLE_PET_ATTR_DETERMINE", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_PET_ATTR_DETERMINE", false);
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	PyModule_AddIntConstant(poModule, "ENABLE_PET_PRIMIUM_FEEDSTUFF", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_PET_PRIMIUM_FEEDSTUFF", false);
#	endif
#	ifdef ENABLE_GROWTH_PET_HATCHING_MONEY_CHANGE
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_HATCHING_MONEY_CHANGE", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_HATCHING_MONEY_CHANGE", false);
#	endif
#	ifdef ENABLE_GROWTH_PET_SKILL_DEL
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SKILL_DEL", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SKILL_DEL", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SKILL_DEL", false);
#	ifndef ENABLE_PET_ATTR_DETERMINE
	PyModule_AddIntConstant(poModule, "ENABLE_PET_ATTR_DETERMINE", false);
#	endif
#	ifndef ENABLE_PET_PRIMIUM_FEEDSTUFF
	PyModule_AddIntConstant(poModule, "ENABLE_PET_PRIMIUM_FEEDSTUFF", false);
#	endif
#	ifndef ENABLE_GROWTH_PET_HATCHING_MONEY_CHANGE
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_HATCHING_MONEY_CHANGE", false);
#	endif
#	ifndef ENABLE_GROWTH_PET_SKILL_DEL
	PyModule_AddIntConstant(poModule, "ENABLE_GROWTH_PET_SKILL_DEL", false);
#	endif
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_COSTUME_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "ACCE_MAX_DRAINRATE", 25);
#ifdef WJ_SHOW_ACCE_ONOFF
	PyModule_AddIntConstant(poModule, "WJ_SHOW_ACCE_ONOFF", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_SHOW_ACCE_ONOFF", false);
#endif

#ifdef ENABLE_ACCE_SECOND_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SECOND_COSTUME_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SECOND_COSTUME_SYSTEM", false);
#endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_COSTUME_SYSTEM", false);

#ifndef ENABLE_ACCE_SECOND_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ACCE_SECOND_COSTUME_SYSTEM", false);
#endif
#endif

#ifdef ENABLE_DRAGON_SOUL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "ENABLE_DS_PASSWORD", false);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_DS_PASSWORD", false);
#endif

#ifdef ENABLE_DS_GRADE_MYTH
	PyModule_AddIntConstant(poModule, "ENABLE_DS_GRADE_MYTH", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DS_GRADE_MYTH", false);
#endif

#ifdef ENABLE_DS_CHANGE_ATTR
	PyModule_AddIntConstant(poModule, "ENABLE_DS_CHANGE_ATTR", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DS_CHANGE_ATTR", false);
#endif

#ifdef ENABLE_ENERGY_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM", false);
#endif

#ifdef ENABLE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM", false);
#endif

#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_COSTUME_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_COSTUME_SYSTEM", false);
#endif

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_WEAPON_COSTUME_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WEAPON_COSTUME_SYSTEM", false);
#endif

#ifdef ENABLE_USE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "ENABLE_USE_COSTUME_ATTR", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_USE_COSTUME_ATTR", false);
#endif

#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_MAGIC_REDUCTION_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MAGIC_REDUCTION_SYSTEM", false);
#endif

#ifdef ENABLE_GACHA_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GACHA_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GACHA_SYSTEM", false);
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_SYSTEM", true);
	//PyModule_AddIntConstant(poModule, "CHEQUE_INPUT_LIMIT", 3);
#	ifdef ENABLE_CHEQUE_EXCHANGE_WINDOW
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_EXCHANGE_WINDOW", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_EXCHANGE_WINDOW", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_SYSTEM", false);
#	ifndef ENABLE_CHEQUE_EXCHANGE_WINDOW
	PyModule_AddIntConstant(poModule, "ENABLE_CHEQUE_EXCHANGE_WINDOW", false);
#	endif
#endif

#ifdef ENABLE_GEM_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GEM_SYSTEM", true);
#	ifdef ENABLE_GEM_SHOP
	PyModule_AddIntConstant(poModule, "ENABLE_GEM_SHOP", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GEM_SHOP", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GEM_SYSTEM", false);
#	ifndef ENABLE_GEM_SHOP
	PyModule_AddIntConstant(poModule, "ENABLE_GEM_SHOP", false);
#	endif
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SOULBIND_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SOULBIND_SYSTEM", false);
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_ITEM_SYSTEM", true);

#	ifdef ENABLE_CHANGE_LOOK_MOUNT
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_MOUNT", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_MOUNT", false);
#	endif
#	ifdef ENABLE_CHANGE_LOOK_HAIR
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_HAIR", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_HAIR", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_ITEM_SYSTEM", false);

#	ifndef ENABLE_CHANGE_LOOK_MOUNT
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_MOUNT", false);
#	endif
#	ifndef ENABLE_CHANGE_LOOK_HAIR
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGE_LOOK_HAIR", false);
#	endif
#endif

#ifdef ENABLE_VIP_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_VIP_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_VIP_SYSTEM", false);
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	PyModule_AddIntConstant(poModule, "ENABLE_PREMIUM_PRIVATE_SHOP", true);

#	ifdef ENABLE_OPEN_SHOP_WITHOUT_BAG
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_WITHOUT_BAG", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_WITHOUT_BAG", false);
#	endif

#	ifdef ENABLE_OPEN_SHOP_ONLY_IN_MARKET
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_ONLY_IN_MARKET", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_ONLY_IN_MARKET", false);
#	endif

#	ifdef ENABLE_OPEN_SHOP_WITH_PASSWORD
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_WITH_PASSWORD", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_WITH_PASSWORD", false);
#	endif

#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	PyModule_AddIntConstant(poModule, "ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL", false);
#	endif

#	ifdef ENABLE_NEARBY_SHOPS_DISPLAYED
	PyModule_AddIntConstant(poModule, "ENABLE_NEARBY_SHOPS_DISPLAYED", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_NEARBY_SHOPS_DISPLAYED", false);
#	endif

#	ifdef ENABLE_ONE_PASS_PER_LOGIN
	PyModule_AddIntConstant(poModule, "ENABLE_ONE_PASS_PER_LOGIN", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_ONE_PASS_PER_LOGIN", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PREMIUM_PRIVATE_SHOP", false);

#	ifndef ENABLE_OPEN_SHOP_WITHOUT_BAG
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_WITHOUT_BAG", false);
#	endif

#	ifndef ENABLE_OPEN_SHOP_ONLY_IN_MARKET
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_ONLY_IN_MARKET", false);
#	endif

#	ifndef ENABLE_OPEN_SHOP_WITH_PASSWORD
	PyModule_AddIntConstant(poModule, "ENABLE_OPEN_SHOP_WITH_PASSWORD", false);
#	endif

#	ifndef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	PyModule_AddIntConstant(poModule, "ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL", false);
#	endif

#	ifndef ENABLE_NEARBY_SHOPS_DISPLAYED
	PyModule_AddIntConstant(poModule, "ENABLE_NEARBY_SHOPS_DISPLAYED", false);
#	endif

#	ifndef ENABLE_ONE_PASS_PER_LOGIN
	PyModule_AddIntConstant(poModule, "ENABLE_ONE_PASS_PER_LOGIN", false);
#	endif
#endif

	/*Secundary Systems*///->
#ifdef ENABLE_QUIVER_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_QUIVER_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_QUIVER_SYSTEM", false);
#endif

#ifdef ENABLE_REFINE_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_RENEWAL", false);
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DUNGEON_INFO_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DUNGEON_INFO_SYSTEM", false);
#endif

#ifdef ENABLE_EXPRESSING_EMOTION
	PyModule_AddIntConstant(poModule, "ENABLE_EXPRESSING_EMOTION", true);
	//PyModule_AddIntConstant(poModule, "SPECIAL_ACTION_START_INDEX", 71);
	PyModule_AddIntConstant(poModule, "SPECIAL_ACTION_START_INDEX", 60);
	PyModule_AddIntConstant(poModule, "SPECIAL_ACTION_END_INDEX", 77);

	PyModule_AddIntConstant(poModule, "SLOT_EMOTION_START", 60);
	PyModule_AddIntConstant(poModule, "SLOT_EMOTION_END", 77);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXPRESSING_EMOTION", false);
	PyModule_AddIntConstant(poModule, "SPECIAL_ACTION_START_INDEX", 71);
	PyModule_AddIntConstant(poModule, "SLOT_EMOTION_START", 60);
	PyModule_AddIntConstant(poModule, "SLOT_EMOTION_END", 77);
#endif

#ifdef ENABLE_OX_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_OX_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_OX_RENEWAL", false);
#endif





	/*-----Dungeons*///----->
#ifdef ENABLE_DAWNMIST_DUNGEON
	PyModule_AddIntConstant(poModule, "ENABLE_DAWNMIST_DUNGEON", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DAWNMIST_DUNGEON", false);
#endif

#ifdef ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_FIELD", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_FIELD", false);
#endif

#ifdef ENABLE_12ZI
	PyModule_AddIntConstant(poModule, "ENABLE_12ZI", true);
#ifdef ENABLE_CHAT_MISSION_ALTERNATIVE
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_MISSION_ALTERNATIVE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_MISSION_ALTERNATIVE", false);
#endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_12ZI", false);
#ifdef ENABLE_CHAT_MISSION_ALTERNATIVE
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_MISSION_ALTERNATIVE", false);
#endif
#endif

#ifdef ENABLE_DEFENSE_WAVE
	PyModule_AddIntConstant(poModule, "ENABLE_DEFENSE_WAVE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DEFENSE_WAVE", false);
#endif

#ifdef ENABLE_QUEEN_NETHIS
	PyModule_AddIntConstant(poModule, "ENABLE_QUEEN_NETHIS", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_QUEEN_NETHIS", false);
#endif





	//--------OPTIONS------->
#ifdef NEW_SELECT_CHARACTER
	PyModule_AddIntConstant(poModule, "NEW_SELECT_CHARACTER", true);
#else
	PyModule_AddIntConstant(poModule, "NEW_SELECT_CHARACTER", false);
#endif




	/*---Main Options*///--->
#ifdef ENABLE_DICE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DICE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DICE_SYSTEM", false);
#endif

#ifdef ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL
	PyModule_AddIntConstant(poModule, "ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PARTY_EXP_DISTRIBUTION_EQUAL", false);
#endif

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM", false);
#endif

#ifdef NON_CHECK_DRAGON_SOUL_QUALIFIED
	PyModule_AddIntConstant(poModule, "NON_CHECK_DRAGON_SOUL_QUALIFIED", true);
#else
	PyModule_AddIntConstant(poModule, "NON_CHECK_DRAGON_SOUL_QUALIFIED", false);
#endif

#ifdef ENABLE_SOCKET_STRING3
	PyModule_AddIntConstant(poModule, "ENABLE_SOCKET_STRING3", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SOCKET_STRING3", false);
#endif

#ifdef ENABLE_EMOJI_TEXTLINE
	PyModule_AddIntConstant(poModule, "ENABLE_EMOJI_TEXTLINE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EMOJI_TEXTLINE", false);
#endif

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	PyModule_AddIntConstant(poModule, "WJ_SHOW_PARTY_ON_MINIMAP", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_SHOW_PARTY_ON_MINIMAP", false);
#endif

#ifdef ENABLE_AFK_MODE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_AFK_MODE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AFK_MODE_SYSTEM", false);
#endif

#ifdef ENABLE_AFTERDEATH_SHIELD
	PyModule_AddIntConstant(poModule, "ENABLE_AFTERDEATH_SHIELD", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AFTERDEATH_SHIELD", false);
#endif

#ifdef ENABLE_TARGET_AFFECT_SHOWER
	PyModule_AddIntConstant(poModule, "ENABLE_TARGET_AFFECT_SHOWER", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_TARGET_AFFECT_SHOWER", false);
#endif

#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
	PyModule_AddIntConstant(poModule, "ENABLE_VIEW_TARGET_DECIMAL_HP", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_VIEW_TARGET_DECIMAL_HP", false);
#endif

#ifdef ENABLE_SEND_TARGET_INFO
	PyModule_AddIntConstant(poModule, "ENABLE_SEND_TARGET_INFO", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SEND_TARGET_INFO", false);
#endif

#ifdef ENABLE_GIVE_BASIC_ITEM
	PyModule_AddIntConstant(poModule, "ENABLE_GIVE_BASIC_ITEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GIVE_BASIC_ITEM", false);
#endif

#ifdef ENABLE_DESTROY_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_DESTROY_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DESTROY_SYSTEM", false);
#endif

#ifdef ENABLE_DETAILS_UI
	PyModule_AddIntConstant(poModule, "ENABLE_DETAILS_UI", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DETAILS_UI", false);
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_KEYCHANGE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_KEYCHANGE_SYSTEM", false);
#endif

#ifdef ENABLE_TARGET_SELECT_COLOR
	PyModule_AddIntConstant(poModule, "ENABLE_TARGET_SELECT_COLOR", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_TARGET_SELECT_COLOR", false);
#endif

#ifdef ENABLE_LEVEL_IN_TRADE
	PyModule_AddIntConstant(poModule, "ENABLE_LEVEL_IN_TRADE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LEVEL_IN_TRADE", false);
#endif

#ifdef ENABLE_MONARCH_AFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_MONARCH_AFFECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MONARCH_AFFECT", false);
#endif

#ifdef ENABLE_MESSENGER_GET_FRIEND_NAMES
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_GET_FRIEND_NAMES", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_GET_FRIEND_NAMES", false);
#endif

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDRENEWAL_SYSTEM", true);
#	ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SECOND_GUILDRENEWAL_SYSTEM", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_SECOND_GUILDRENEWAL_SYSTEM", false);
#	endif

#	ifdef ENABLE_EXTENDED_GUILD_LEVEL
	PyModule_AddIntConstant(poModule, "ENABLE_EXTENDED_GUILD_LEVEL", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_EXTENDED_GUILD_LEVEL", false);
#	endif

#	ifdef ENABLE_MEDAL_OF_HONOR
	PyModule_AddIntConstant(poModule, "ENABLE_MEDAL_OF_HONOR", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_MEDAL_OF_HONOR", false);
#	endif
#	ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DONATE_ATTENDANCE", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DONATE_ATTENDANCE", false);
#	endif


#	ifdef ENABLE_GUILD_WAR_SCORE
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_WAR_SCORE", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_WAR_SCORE", false);
#	endif
#	ifdef ENABLE_GUILD_LAND_INFO
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_LAND_INFO", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_LAND_INFO", false);
#	endif
#	ifdef ENABLE_GUILDBANK_LOG
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDBANK_LOG", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDBANK_LOG", false);
#	endif

#	ifdef ENABLE_NEW_WORLD_GUILDRENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_WORLD_GUILDRENEWAL", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_WORLD_GUILDRENEWAL", false);
#	endif

#	ifdef ENABLE_GUILD_REQUEST
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_REQUEST", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_REQUEST", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDRENEWAL_SYSTEM", false);
#	ifndef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SECOND_GUILDRENEWAL_SYSTEM", false);
#	endif

#	ifndef ENABLE_EXTENDED_GUILD_LEVEL
	PyModule_AddIntConstant(poModule, "ENABLE_EXTENDED_GUILD_LEVEL", false);
#	endif

#	ifndef ENABLE_MEDAL_OF_HONOR
	PyModule_AddIntConstant(poModule, "ENABLE_MEDAL_OF_HONOR", false);
#	endif
#	ifndef ENABLE_GUILD_DONATE_ATTENDANCE
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DONATE_ATTENDANCE", false);
#	endif

#	ifndef ENABLE_GUILD_WAR_SCORE
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_WAR_SCORE", false);
#	endif
#	ifndef ENABLE_GUILD_LAND_INFO
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_LAND_INFO", false);
#	endif
#	ifndef ENABLE_GUILDBANK_LOG
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDBANK_LOG", false);
#	endif

#	ifndef ENABLE_NEW_WORLD_GUILDRENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_WORLD_GUILDRENEWAL", false);
#	endif

#	ifndef ENABLE_GUILD_REQUEST
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_REQUEST", false);
#	endif
#endif

#ifdef LOGIN_COUNT_DOWN_UI_MODIFY
	PyModule_AddIntConstant(poModule, "LOGIN_COUNT_DOWN_UI_MODIFY", true);
#else
	PyModule_AddIntConstant(poModule, "LOGIN_COUNT_DOWN_UI_MODIFY", false);
#endif

#ifdef SIGN_FOR_PLUS_ITEMS
	PyModule_AddIntConstant(poModule, "SIGN_FOR_PLUS_ITEMS", true);
#else
	PyModule_AddIntConstant(poModule, "SIGN_FOR_PLUS_ITEMS", false);
#endif

#ifdef ENABLE_PENDANT
	PyModule_AddIntConstant(poModule, "ENABLE_PENDANT", true);
#ifdef ENABLE_ELEMENT_ADD
	PyModule_AddIntConstant(poModule, "ENABLE_ELEMENT_ADD", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ELEMENT_ADD", false);
#endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PENDANT", false);
#ifndef ENABLE_ELEMENT_ADD
	PyModule_AddIntConstant(poModule, "ENABLE_ELEMENT_ADD", false);
#endif
#endif

#ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_AURA_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "AURA_MAX_SOCKETRATE", 25);

#ifdef ENABLE_AURA_BOOST
	PyModule_AddIntConstant(poModule, "ENABLE_AURA_BOOST", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AURA_BOOST", false);
#endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AURA_SYSTEM", false);

#ifndef ENABLE_AURA_BOOST
	PyModule_AddIntConstant(poModule, "ENABLE_AURA_BOOST", false);
#endif
#endif


	/*Secundary Options*///->
#ifdef ENABLE_RENEWAL_SHOPEX
	PyModule_AddIntConstant(poModule, "ENABLE_RENEWAL_SHOPEX", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RENEWAL_SHOPEX", false);
#endif

#ifdef ENABLE_SLOT_WINDOW_EX
	PyModule_AddIntConstant(poModule, "ENABLE_SLOT_WINDOW_EX", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SLOT_WINDOW_EX", false);
#endif

#ifdef ENABLE_ANTI_EXP_RING
	PyModule_AddIntConstant(poModule, "ENABLE_ANTI_EXP_RING", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ANTI_EXP_RING", false);
#endif

#ifdef ENABLE_CUBE_PERCENT_INFO
	PyModule_AddIntConstant(poModule, "ENABLE_CUBE_PERCENT_INFO", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CUBE_PERCENT_INFO", false);
#endif

#ifdef ENABLE_AFFECT_POLYMORPH_REMOVE
	PyModule_AddIntConstant(poModule, "ENABLE_AFFECT_POLYMORPH_REMOVE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AFFECT_POLYMORPH_REMOVE", false);
#endif

#ifdef ENABLE_AFFECT_BUFF_REMOVE
	PyModule_AddIntConstant(poModule, "ENABLE_AFFECT_BUFF_REMOVE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AFFECT_BUFF_REMOVE", false);
#endif

#ifdef ENABLE_GROUP_DAMAGE_WEAPON_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_GROUP_DAMAGE_WEAPON_EFFECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GROUP_DAMAGE_WEAPON_EFFECT", false);
#endif

#ifdef ENABLE_EXTENDED_CONFIGS
	PyModule_AddIntConstant(poModule, "ENABLE_EXTENDED_CONFIGS", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXTENDED_CONFIGS", false);
#endif

#ifdef ENABLE_RUN_MOUSE_WHEEL_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_RUN_MOUSE_WHEEL_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RUN_MOUSE_WHEEL_EVENT", false);
#endif

#ifdef ENABLE_MOVE_CHANNEL
	PyModule_AddIntConstant(poModule, "ENABLE_MOVE_CHANNEL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOVE_CHANNEL", false);
#endif

#ifdef ENABLE_LOADING_TIP
	PyModule_AddIntConstant(poModule, "ENABLE_LOADING_TIP", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LOADING_TIP", false);
#endif

#ifdef ENABLE_NEW_SHINING_EFFEKT
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_SHINING_EFFEKT", true);
#	ifdef ENABLE_LVL115_ARMOR_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_LVL115_ARMOR_EFFECT", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_LVL115_ARMOR_EFFECT", false);
#	endif
#	ifdef USE_BODY_COSTUME_WITH_EFFECT
	PyModule_AddIntConstant(poModule, "USE_BODY_COSTUME_WITH_EFFECT", true);
#	else
	PyModule_AddIntConstant(poModule, "USE_BODY_COSTUME_WITH_EFFECT", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_SHINING_EFFEKT", false);
#	ifdef ENABLE_LVL115_ARMOR_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_LVL115_ARMOR_EFFECT", false);
#	endif
#	ifdef USE_BODY_COSTUME_WITH_EFFECT
	PyModule_AddIntConstant(poModule, "USE_BODY_COSTUME_WITH_EFFECT", false);
#	endif
#endif

#ifdef ENABLE_RENDER_TARGET
	PyModule_AddIntConstant(poModule, "ENABLE_RENDER_TARGET", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RENDER_TARGET", false);
#endif

#ifdef ENABLE_SOUL_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SOUL_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SOUL_SYSTEM", false);
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
	PyModule_AddIntConstant(poModule, "ENABLE_SPECIAL_INVENTORY", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SPECIAL_INVENTORY", false);
#endif

#ifdef ENABLE_QUEST_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_QUEST_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_QUEST_RENEWAL", false);
#endif

#ifdef ENABLE_SWITCHBOT
	PyModule_AddIntConstant(poModule, "ENABLE_SWITCHBOT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SWITCHBOT", false);
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_LANGUAGE_SYSTEM", 1);

	PyModule_AddIntConstant(poModule, "LANGUAGE_NONE", LANGUAGE_NONE);
	PyModule_AddIntConstant(poModule, "LANGUAGE_EN", LANGUAGE_EN);
	PyModule_AddIntConstant(poModule, "LANGUAGE_AE", LANGUAGE_AE);
	PyModule_AddIntConstant(poModule, "LANGUAGE_CZ", LANGUAGE_CZ);
	PyModule_AddIntConstant(poModule, "LANGUAGE_DK", LANGUAGE_DK);
	PyModule_AddIntConstant(poModule, "LANGUAGE_FR", LANGUAGE_FR);
	PyModule_AddIntConstant(poModule, "LANGUAGE_GR", LANGUAGE_GR);
	PyModule_AddIntConstant(poModule, "LANGUAGE_NL", LANGUAGE_NL);
	PyModule_AddIntConstant(poModule, "LANGUAGE_PL", LANGUAGE_PL);
	PyModule_AddIntConstant(poModule, "LANGUAGE_HU", LANGUAGE_HU);
	PyModule_AddIntConstant(poModule, "LANGUAGE_DE", LANGUAGE_DE);
	PyModule_AddIntConstant(poModule, "LANGUAGE_IT", LANGUAGE_IT);
	PyModule_AddIntConstant(poModule, "LANGUAGE_RU", LANGUAGE_RU);
	PyModule_AddIntConstant(poModule, "LANGUAGE_PT", LANGUAGE_PT);
	PyModule_AddIntConstant(poModule, "LANGUAGE_RO", LANGUAGE_RO);
	PyModule_AddIntConstant(poModule, "LANGUAGE_ES", LANGUAGE_ES);
	PyModule_AddIntConstant(poModule, "LANGUAGE_TR", LANGUAGE_TR);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MULTI_LANGUAGE_SYSTEM", 0);
#endif

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDSTORAGE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILDSTORAGE_SYSTEM", false);
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_COLOR_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_COLOR_SYSTEM", false);
#endif

#ifdef ENABLE_MAP_LOCATION_APP_NAME
	PyModule_AddIntConstant(poModule, "ENABLE_MAP_LOCATION_APP_NAME", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MAP_LOCATION_APP_NAME", false);
#endif

#ifdef ENABLE_CUBE_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_CUBE_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CUBE_RENEWAL", false);
#endif

#ifdef ENABLE_ATTR_6TH_7TH
	PyModule_AddIntConstant(poModule, "ENABLE_ATTR_6TH_7TH", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ATTR_6TH_7TH", false);
#endif

#ifdef ENABLE_SKILLBOOK_COMBINATION
	PyModule_AddIntConstant(poModule, "ENABLE_SKILLBOOK_COMBINATION", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SKILLBOOK_COMBINATION", false);
#endif

#ifdef WJ_MULTI_TEXTLINE
	PyModule_AddIntConstant(poModule, "WJ_MULTI_TEXTLINE", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_MULTI_TEXTLINE", false);
#endif

#ifdef ENABLE_SKILL_COOLTIME_UPDATE
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_COOLTIME_UPDATE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SKILL_COOLTIME_UPDATE", false);
#endif

#ifdef ENABLE_IMAGE_SCALE
	PyModule_AddIntConstant(poModule, "ENABLE_IMAGE_SCALE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_IMAGE_SCALE", false);
#endif

#ifdef ENABLE_ATLAS_SCALE
	PyModule_AddIntConstant(poModule, "ENABLE_ATLAS_SCALE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ATLAS_SCALE", false);
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_PRIVATESHOP_SEARCH_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PRIVATESHOP_SEARCH_SYSTEM", false);
#endif

#ifdef ENABLE_DS_SET
	PyModule_AddIntConstant(poModule, "ENABLE_DS_SET", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DS_SET", false);
#endif

#ifdef ENABLE_SERVER_SELECT_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_SERVER_SELECT_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SERVER_SELECT_RENEWAL", false);
#endif

#ifdef ENABLE_CHANNEL_LIST
	PyModule_AddIntConstant(poModule, "ENABLE_CHANNEL_LIST", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANNEL_LIST", false);
#endif

#ifdef ENABLE_GUILD_MARK_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_MARK_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_MARK_RENEWAL", false);
#endif

#ifdef ENABLE_RANKING_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_RANKING_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RANKING_SYSTEM", false);
#endif

#ifdef ENABLE_RANKING_SYSTEM_PARTY
	PyModule_AddIntConstant(poModule, "ENABLE_RANKING_SYSTEM_PARTY", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RANKING_SYSTEM_PARTY", false);
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DRAGONLAIR_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DRAGONLAIR_SYSTEM", false);
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM", false);
#endif

#ifdef ENABLE_REFINE_ELEMENT
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_ELEMENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_ELEMENT", false);
#endif

#ifdef ENABLE_INGAME_WIKI
	PyModule_AddIntConstant(poModule, "ENABLE_INGAME_WIKI", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_INGAME_WIKI", false);
#endif

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	PyModule_AddIntConstant(poModule, "ENABLE_ATTR_6TH_7TH_EXTEND", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ATTR_6TH_7TH_EXTEND", false);
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_YOHARA_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_YOHARA_SYSTEM", false);
#endif

#ifdef ENABLE_PASSIVE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_PASSIVE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PASSIVE_SYSTEM", false);
#endif

#ifdef ENABLE_NINETH_SKILL
	PyModule_AddIntConstant(poModule, "ENABLE_NINETH_SKILL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NINETH_SKILL", false);
#endif

#ifdef ENABLE_GLOVE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_GLOVE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GLOVE_SYSTEM", false);
#endif

#ifdef ENABLE_COMPARE_TOOLTIP
	PyModule_AddIntConstant(poModule, "ENABLE_COMPARE_TOOLTIP", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_COMPARE_TOOLTIP", false);
#endif

#ifdef ENABLE_SAFEBOX_IMPROVING
	PyModule_AddIntConstant(poModule, "ENABLE_SAFEBOX_IMPROVING", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SAFEBOX_IMPROVING", false);
#endif

#ifdef ENABLE_SAFEBOX_MONEY
	PyModule_AddIntConstant(poModule, "ENABLE_SAFEBOX_MONEY", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SAFEBOX_MONEY", false);
#endif

#ifdef ENABLE_WORLD_BOSS
	PyModule_AddIntConstant(poModule, "ENABLE_WORLD_BOSS", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WORLD_BOSS", false);
#endif

#ifdef ENABLE_SUNG_MAHI_TOWER
	PyModule_AddIntConstant(poModule, "ENABLE_SUNG_MAHI_TOWER", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SUNG_MAHI_TOWER", false);
#endif

#ifdef ENABLE_BELT_INVENTORY_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_BELT_INVENTORY_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BELT_INVENTORY_RENEWAL", false);
#endif

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// OFFICAL_MODULES
#ifdef WJ_NEW_USER_CARE
	PyModule_AddIntConstant(poModule, "WJ_NEW_USER_CARE", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_NEW_USER_CARE", false);
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_INVEN_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_INVEN_SYSTEM", false);
#endif

#ifdef ENABLE_DELETE_FAILURE_TYPE
	PyModule_AddIntConstant(poModule, "ENABLE_DELETE_FAILURE_TYPE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DELETE_FAILURE_TYPE", false);
#endif

#ifdef ENABLE_FOG_FIX
	PyModule_AddIntConstant(poModule, "ENABLE_FOG_FIX", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FOG_FIX", false);
#endif

#ifdef ENABLE_DELETE_FAILURE_TYPE_ADD
	PyModule_AddIntConstant(poModule, "ENABLE_DELETE_FAILURE_TYPE_ADD", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DELETE_FAILURE_TYPE_ADD", false);
#endif

#ifdef ENABLE_REFINE_MSG_ADD
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_MSG_ADD", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_REFINE_MSG_ADD", false);
#endif

#ifdef ENABLE_678TH_SKILL
	PyModule_AddIntConstant(poModule, "ENABLE_678TH_SKILL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_678TH_SKILL", false);
#endif

#ifdef ENABLE_DS_TYPE_CHANGE
	PyModule_AddIntConstant(poModule, "ENABLE_DS_TYPE_CHANGE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DS_TYPE_CHANGE", false);
#endif

#ifdef ENABLE_DSS_KEY_SELECT
	PyModule_AddIntConstant(poModule, "ENABLE_DSS_KEY_SELECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DSS_KEY_SELECT", false);
#endif

#ifdef ENABLE_NEW_USER_CARE
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_USER_CARE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_NEW_USER_CARE", false);
#endif

#ifdef ENABLE_BALANCE_IMPROVING
	PyModule_AddIntConstant(poModule, "ENABLE_BALANCE_IMPROVING", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BALANCE_IMPROVING", false);
#endif

#ifdef ENABLE_HELP_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_HELP_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HELP_RENEWAL", false);
#endif

#ifdef ENABLE_BOSS_BOX
	PyModule_AddIntConstant(poModule, "ENABLE_BOSS_BOX", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BOSS_BOX", false);
#endif

#ifdef POPUPDIALOG_MODIFY
	PyModule_AddIntConstant(poModule, "POPUPDIALOG_MODIFY", true);
#else
	PyModule_AddIntConstant(poModule, "POPUPDIALOG_MODIFY", false);
#endif

#ifdef ENABLE_PICK_ROD_REFINE_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_PICK_ROD_REFINE_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PICK_ROD_REFINE_RENEWAL", false);
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_SLOT_COVER_IMAGE_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SLOT_COVER_IMAGE_SYSTEM", false);
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "ENABLE_MOVE_COSTUME_ATTR", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOVE_COSTUME_ATTR", false);
#endif

#ifdef ENABLE_MESSENGER_LOCK_FIX
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_LOCK_FIX", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_LOCK_FIX", false);
#endif

#ifdef ENABLE_PARTY_MATCH
	PyModule_AddIntConstant(poModule, "ENABLE_PARTY_MATCH", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PARTY_MATCH", false);
#endif

	//CUSTOM
#ifdef ENABLE_VIEW_ITEM_VNUM
	PyModule_AddIntConstant(poModule, "ENABLE_VIEW_ITEM_VNUM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_VIEW_ITEM_VNUM", false);
#endif

#ifdef ENABLE_ANTIFLAG_TOOLTIP
	PyModule_AddIntConstant(poModule, "ENABLE_ANTIFLAG_TOOLTIP", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ANTIFLAG_TOOLTIP", false);
#endif

#ifdef ENABLE_RACE_HEIGHT
	PyModule_AddIntConstant(poModule, "ENABLE_RACE_HEIGHT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RACE_HEIGHT", false);
#endif

#ifdef ENABLE_BLOCK_COMMAND_PLAYER
	PyModule_AddIntConstant(poModule, "ENABLE_BLOCK_COMMAND_PLAYER", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BLOCK_COMMAND_PLAYER", false);
#endif

#ifdef ENABLE_EXPANDED_MONEY_TASKBAR
	PyModule_AddIntConstant(poModule, "ENABLE_EXPANDED_MONEY_TASKBAR", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXPANDED_MONEY_TASKBAR", false);
#endif

#ifdef ENABLE_FLASH_APLICATION
	PyModule_AddIntConstant(poModule, "ENABLE_FLASH_APLICATION", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FLASH_APLICATION", false);
#endif

#ifdef ENABLE_SKIP_MOVIE
	PyModule_AddIntConstant(poModule, "ENABLE_SKIP_MOVIE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SKIP_MOVIE", false);
#endif

#ifdef ENABLE_PLUS_ICON_ITEM
	PyModule_AddIntConstant(poModule, "ENABLE_PLUS_ICON_ITEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PLUS_ICON_ITEM", false);
#endif

#ifdef ENABLE_DS_REFINE_WINDOW
	PyModule_AddIntConstant(poModule, "ENABLE_DS_REFINE_WINDOW", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DS_REFINE_WINDOW", false);
#endif

#ifdef ENABLE_DISABLE_SOFTWARE_TILING
	PyModule_AddIntConstant(poModule, "ENABLE_DISABLE_SOFTWARE_TILING", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DISABLE_SOFTWARE_TILING", false);
#endif

#ifdef ENABLE_DS_ACTIVE_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_DS_ACTIVE_EFFECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_DS_ACTIVE_EFFECT", false);
#endif

#ifdef ENABLE_DS_ACTIVE_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_RINGS_IN_COSTUME_WINDOW", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RINGS_IN_COSTUME_WINDOW", false);
#endif

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	PyModule_AddIntConstant(poModule, "ENABLE_SHADOW_RENDER_QUALITY_OPTION", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SHADOW_RENDER_QUALITY_OPTION", false);
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_EFFECT_OPTION", false);
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
	PyModule_AddIntConstant(poModule, "ENABLE_GRAPHIC_ON_OFF", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GRAPHIC_ON_OFF", false);
#endif

#ifdef DISABLE_LINK_FROM_HYPERLINK
	PyModule_AddIntConstant(poModule, "DISABLE_LINK_FROM_HYPERLINK", true);
#else
	PyModule_AddIntConstant(poModule, "DISABLE_LINK_FROM_HYPERLINK", false);
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_BLOCK", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MESSENGER_BLOCK", false);
#endif

#ifdef ENABLE_GM_MESSENGER_LIST
	PyModule_AddIntConstant(poModule, "ENABLE_GM_MESSENGER_LIST", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_GM_MESSENGER_LIST", false);
#endif

#ifdef ENABLE_RENDER_LOGIN_EFFECTS
	PyModule_AddIntConstant(poModule, "ENABLE_RENDER_LOGIN_EFFECTS", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_RENDER_LOGIN_EFFECTS", false);
#endif

#ifdef ENABLE_STRUCTURE_VIEW_MODE
	PyModule_AddIntConstant(poModule, "ENABLE_STRUCTURE_VIEW_MODE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_STRUCTURE_VIEW_MODE", false);
#endif

#ifdef ENABLE_MYSHOP_DECO
	PyModule_AddIntConstant(poModule, "ENABLE_MYSHOP_DECO", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MYSHOP_DECO", false);
#endif

#ifdef ENABLE_STONE_OF_BLESS
	PyModule_AddIntConstant(poModule, "ENABLE_STONE_OF_BLESS", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_STONE_OF_BLESS", false);
#endif

#ifdef ENABLE_HWID_BAN
	PyModule_AddIntConstant(poModule, "ENABLE_HWID_BAN", true);

#	ifdef ENABLE_MULTIFARM_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MULTIFARM_BLOCK", true);
#	else
	PyModule_AddIntConstant(poModule, "ENABLE_MULTIFARM_BLOCK", false);
#	endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HWID_BAN", false);

#	ifndef ENABLE_MULTIFARM_BLOCK
	PyModule_AddIntConstant(poModule, "ENABLE_MULTIFARM_BLOCK", false);
#	endif
#endif

#ifdef ENABLE_LVL96_WEAPON_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_LVL96_WEAPON_EFFECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LVL96_WEAPON_EFFECT", false);
#endif

#ifdef ENABLE_LVL96_ARMOR_EFFECT
	PyModule_AddIntConstant(poModule, "ENABLE_LVL96_ARMOR_EFFECT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LVL96_ARMOR_EFFECT", false);
#endif

#ifdef ENABLE_CHANGED_ATTR
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGED_ATTR", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHANGED_ATTR", false);
#endif

#ifdef ENABLE_SET_ITEM
	PyModule_AddIntConstant(poModule, "ENABLE_SET_ITEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SET_ITEM", false);
#endif

#ifdef WJ_SHOW_NPC_QUEST_NAME
	PyModule_AddIntConstant(poModule, "WJ_SHOW_NPC_QUEST_NAME", true);
#else
	PyModule_AddIntConstant(poModule, "WJ_SHOW_NPC_QUEST_NAME", false);
#endif

#ifdef ENABLE_MULTIPLE_CHEST_OPEN
	PyModule_AddIntConstant(poModule, "ENABLE_MULTIPLE_CHEST_OPEN", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MULTIPLE_CHEST_OPEN", false);
#endif

#ifdef ENABLE_VIEW_EQUIP_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_VIEW_EQUIP_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_VIEW_EQUIP_RENEWAL", false);
#endif

#ifdef ENABLE_CHAT_SETTINGS
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_SETTINGS", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_SETTINGS", false);
#endif

#ifdef ENABLE_CHAT_SETTINGS_EXTEND
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_SETTINGS_EXTEND", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_CHAT_SETTINGS_EXTEND", false);
#endif

#ifdef ENABLE_AUTO_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "ENABLE_IMPROVED_AUTOMATIC_HUNTING_SYSTEM", true);
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_AFFECT_DISPLAY", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_IMPROVED_AUTOMATIC_HUNTING_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_AFFECT_DISPLAY", false);
#endif
	PyModule_AddIntConstant(poModule, "ENABLE_AUTO_SYSTEM_OFFICAL", false);

#ifdef ENABLE_TICKET_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_TICKET_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_TICKET_SYSTEM", false);
#endif

#ifdef ENABLE_MAILBOX
	PyModule_AddIntConstant(poModule, "ENABLE_MAILBOX", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MAILBOX", false);
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_PASS_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BATTLE_PASS_SYSTEM", false);
#endif

	PyModule_AddIntConstant(poModule, "ENABLE_IMAGE_CLIP_RECT", true);

#ifdef ENABLE_FISHING_RENEWAL
	PyModule_AddIntConstant(poModule, "ENABLE_FISHING_RENEWAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FISHING_RENEWAL", false);
#endif

#ifdef ENABLE_WEATHER_INFO
	PyModule_AddIntConstant(poModule, "ENABLE_WEATHER_INFO", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WEATHER_INFO", false);
#endif

#ifdef ENABLE_ENVIRONMENT_RAIN
	PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_RAIN", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ENVIRONMENT_RAIN", false);
#endif

#ifdef ENABLE_HIDE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_HIDE_COSTUME_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HIDE_COSTUME_SYSTEM", false);
#endif

#ifdef ENABLE_PATCHNOTE_WINDOW
	PyModule_AddIntConstant(poModule, "ENABLE_PATCHNOTE_WINDOW", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PATCHNOTE_WINDOW", false);
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_ACHIEVEMENT_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ACHIEVEMENT_SYSTEM", false);
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_BIOLOG_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_BIOLOG_SYSTEM", false);
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV", false);
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	PyModule_AddIntConstant(poModule, "ENABLE_ADDITIONAL_EQUIPMENT_PAGE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ADDITIONAL_EQUIPMENT_PAGE", false);
#endif

#ifdef ENABLE_HUNTING_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_HUNTING_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_HUNTING_SYSTEM", false);
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_WORLD_LOTTERY_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WORLD_LOTTERY_SYSTEM", false);
#endif

#ifdef ENABLE_ELEMENTAL_WORLD
	PyModule_AddIntConstant(poModule, "ENABLE_ELEMENTAL_WORLD", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ELEMENTAL_WORLD", false);
#endif

#ifdef ENABLE_LOOTING_SYSTEM
	PyModule_AddIntConstant(poModule, "ENABLE_LOOTING_SYSTEM", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LOOTING_SYSTEM", false);
#endif

#ifdef ENABLE_PACK_IMPORT_MODULE
	PyModule_AddIntConstant(poModule, "ENABLE_PACK_IMPORT_MODULE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_PACK_IMPORT_MODULE", false);
#endif
	// End_Application Defines

	// EVENTS
#ifdef ENABLE_EVENT_MANAGER
	PyModule_AddIntConstant(poModule, "ENABLE_EVENT_MANAGER", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EVENT_MANAGER", false);
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_OKEY_NORMAL", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_OKEY_NORMAL", false);
#endif

	//ENABLE_EVENT_BANNER_FLAG

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	PyModule_AddIntConstant(poModule, "ENABLE_SUMMER_EVENT_ROULETTE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SUMMER_EVENT_ROULETTE", false);
#endif

#ifdef ENABLE_FISH_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_FISH_EVENT", true);

	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_1", FISH_EVENT_SHAPE_1);
	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_2", FISH_EVENT_SHAPE_2);
	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_3", FISH_EVENT_SHAPE_3);
	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_4", FISH_EVENT_SHAPE_4);
	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_5", FISH_EVENT_SHAPE_5);
	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_6", FISH_EVENT_SHAPE_6);
	PyModule_AddIntConstant(poModule, "FISH_EVENT_SHAPE_7", FISH_EVENT_SHAPE_7);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FISH_EVENT", false);
#endif

#ifdef ENABLE_MONSTER_BACK
	PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_BACK", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_BACK", false);
#endif

#ifdef ENABLE_10TH_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_10TH_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_10TH_EVENT", false);
#endif

#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	PyModule_AddIntConstant(poModule, "ENABLE_ACCUMULATE_DAMAGE_DISPLAY", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_ACCUMULATE_DAMAGE_DISPLAY", false);
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_CATCH_KING", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_CATCH_KING", false);
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_YUTNORI", true);

	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_YUTNORI_RENDER", true);
	PyModule_AddIntConstant(poModule, "RENDER_TARGET_YUTNORI", CPythonApplication::RENDER_TARGET_YUTNORI);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_YUTNORI", false);
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_YUTNORI_RENDER", false);
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_METINSTONE_RAIN_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_METINSTONE_RAIN_EVENT", false);
#endif

#ifdef ENABLE_SOCCER_BALL_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_SOCCER_BALL_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SOCCER_BALL_EVENT", false);
#endif

#ifdef ENABLE_WORD_GAME_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_WORD_GAME_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_WORD_GAME_EVENT", false);
#endif

#ifdef ENABLE_LUCKY_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_LUCKY_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_LUCKY_EVENT", false);
#endif

#ifdef ENABLE_MONSTER_BACK
	PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_BACK", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MONSTER_BACK", false);
#endif

#ifdef ENABLE_EASTER_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_EASTER_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_EASTER_EVENT", false);
#endif

#ifdef ENABLE_SUMMER_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_SUMMER_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SUMMER_EVENT", false);
#endif

#ifdef ENABLE_2017_RAMADAN
	PyModule_AddIntConstant(poModule, "ENABLE_2017_RAMADAN", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_2017_RAMADAN", false);
#endif



#ifdef ENABLE_2016_VALENTINE
	PyModule_AddIntConstant(poModule, "ENABLE_2016_VALENTINE", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_2016_VALENTINE", false);
#endif

#ifdef ENABLE_FLOWER_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_FLOWER_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_FLOWER_EVENT", false);
#endif

#ifdef ENABLE_MINI_GAME_FINDM
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_FINDM", true);

# ifdef ENABLE_MINI_GAME_FINDM_HINT
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_FINDM_HINT", true);
# else
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_FINDM_HINT", false);
# endif
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_FINDM", false);

# ifndef ENABLE_MINI_GAME_FINDM_HINT
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_FINDM_HINT", false);
# endif
#endif

#ifdef ENABLE_MINI_GAME_BNW
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_BNW", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MINI_GAME_BNW", false);
#endif

#ifdef ENABLE_METINSTONE_RAIN_EVENT
	PyModule_AddIntConstant(poModule, "ENABLE_METINSTONE_RAIN_EVENT", true);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_METINSTONE_RAIN_EVENT", false);
#endif
	// END_EVENTS

	PyModule_AddIntConstant(poModule, "ENABLE_EVENT_SYSTEM", false);
	PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_ATTR_RENEWAL_SECOND", false);
	PyModule_AddIntConstant(poModule, "ENABLE_EXTEND_MALLBOX", false);

#ifdef __UNIMPLEMENTED__
	PyModule_AddIntConstant(poModule, "__UNIMPLEMENTED__", true);
#else
	PyModule_AddIntConstant(poModule, "__UNIMPLEMENTED__", false);
#endif

#ifdef USE_OPENID
	PyModule_AddIntConstant(poModule, "USE_OPENID", 1);
	if (openid_test)
		PyModule_AddIntConstant(poModule, "OPENID_TEST", true);
	else
		PyModule_AddIntConstant(poModule, "OPENID_TEST", false);
#else
	PyModule_AddIntConstant(poModule, "USE_OPENID", 0);
	PyModule_AddIntConstant(poModule, "OPENID_TEST", 0);
#endif /* USE_OPENID */
}
