#include "StdAfx.h"
#include "PythonApplication.h"
#include "ProcessScanner.h"
#include "PythonExceptionSender.h"
#include "resource.h"
#include "Version.h"

#ifdef _DEBUG
#	include <crtdbg.h>
#endif

#include "../EterPack/EterPackManager.h"
#include "../EterLib/Util.h"
#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif
#include "../EterBase/CPostIt.h"

#if defined(EVENT_HANDLER_MASTER)
#include "EventHandler.h"
#endif

extern "C"
{
	extern int32_t _fltused;
	volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;
	__declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int32_t AmdPowerXpressRequestHighPerformance = 1;
};

extern "C"
{
	FILE __iob_func[3] = { *stdin,*stdout,*stderr };
}

#pragma comment(linker, "/NODEFAULTLIB:libci.lib")

#pragma comment(lib, "version.lib")

#ifdef _DEBUG
#	pragma comment(lib, "python27.lib") // _d (need to recompile it from python source)
#else
#	pragma comment(lib, "python27.lib")
#endif

#pragma comment(lib, "imagehlp.lib")
#pragma comment(lib, "devil.lib")

#if GrannyProductMinorVersion == 4
#	pragma comment(lib, "granny2.4.0.10.lib")
#elif GrannyProductMinorVersion == 7
#	pragma comment(lib, "granny2.7.0.30.lib")
#elif GrannyProductMinorVersion == 8
#	pragma comment(lib, "granny2.8.49.0.lib")
#elif GrannyProductMinorVersion == 9
#	ifdef BUILDING_GRANNY_STATIC
#		pragma comment(lib, "granny2.9.12.0_static.lib")
#	else
#		pragma comment(lib, "granny2.9.12.0.lib")
#	endif
#elif GrannyProductMinorVersion == 11
#	ifdef BUILDING_GRANNY_STATIC
#		pragma comment(lib, "granny2.11.8.0_static.lib")
#	else
#		pragma comment(lib, "granny2.11.8.0.lib")
#	endif
#else
#	error "unknown granny version"
#endif

#pragma comment(lib, "mss32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "oldnames.lib")
#pragma comment(lib, "SpeedTreeRT.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dmoguids.lib")
#ifdef CEF_BROWSER
#pragma comment (lib, "libcef.lib")
#ifdef _DEBUG
#pragma comment (lib, "libcef_dll_wrapperd.lib")
#else
#pragma comment (lib, "libcef_dll_wrapper.lib")
#endif
#endif

#include <cstdlib>
#include <cryptopp/cryptoppLibLink.h>

// #define __USE_CYTHON__
#ifdef __USE_CYTHON__
// don't include these two files .h .cpp if you're implementing cython via .pyd
#	include "../CRootLib/Workplace/RootLib/PythonrootlibManager.h"
// it would be better including such file in the project, but this is easier at this moment:
#	include "../CRootLib/Workplace/RootLib/PythonrootlibManager.cpp"
#endif

// #define __USE_EXTRA_CYTHON__
#ifdef __USE_EXTRA_CYTHON__
// don't include these two files .h .cpp if you're implementing cython via .pyd
#	include "../CUIScriptLib/Workplace/UiScriptLib/PythonuiscriptlibManager.h"
// it would be better including such file in the project, but this is easier at this moment:
#	include "../CUIScriptLib/Workplace/UiScriptLib/PythonuiscriptlibManager.cpp"
#endif

static const char * sc_apszPythonLibraryFilenames[] = {"UserDict.pyc", "__future__.pyc", "copy_reg.pyc", "linecache.pyc",
													   "ntpath.pyc",   "os.pyc",         "site.pyc",     "stat.pyc",
													   "string.pyc",   "traceback.pyc",  "types.pyc"};

#ifdef ENABLE_PYLIB_CHECK
#	define PRINT_LEVEL 0
#	define PRINTME(level, ...)   \
		if (PRINT_LEVEL >= level) \
			TraceError(__VA_ARGS__);
#	define PYFOLD "./lib"
// #define PYFORCE

typedef struct PyLibFiles_s
{
	std::string sFileName;
	size_t stSize;
	uint32_t dwCRC32;
} PyLibFiles_t;

const PyLibFiles_t PyLibFilesTable[] = {
#	if PY_VERSION_HEX >= 0x02070000 && PY_VERSION_HEX < 0x02080000
	{PYFOLD "/abc.pyc", 6187, 3834771731},
	{PYFOLD "/bisect.pyc", 3236, 3116899751},
	{PYFOLD "/codecs.pyc", 36978, 2928014693},
	{PYFOLD "/collections.pyc", 26172, 385366131},
	{PYFOLD "/copy.pyc", 13208, 1091298715},
	{PYFOLD "/copy_reg.pyc", 5157, 536292604},
	{PYFOLD "/encodings/aliases.pyc", 8803, 3888310600},
	{PYFOLD "/encodings/cp949.pyc", 2009, 1824094431},
	{PYFOLD "/encodings/__init__.pyc", 4510, 2926961588},
	{PYFOLD "/fnmatch.pyc", 3732, 4270526278},
	{PYFOLD "/functools.pyc", 6193, 3257285433},
	{PYFOLD "/genericpath.pyc", 3303, 1652596334},
	{PYFOLD "/heapq.pyc", 13896, 2948659214},
	{PYFOLD "/keyword.pyc", 2169, 2178546341},
	{PYFOLD "/linecache.pyc", 3235, 4048207604},
	{PYFOLD "/locale.pyc", 49841, 4114662314},
	{PYFOLD "/ntpath.pyc", 11961, 2765879465},
	{PYFOLD "/os.pyc", 25769, 911432770},
	{PYFOLD "/pyexpat.pyd", 127488, 2778492911},
	{PYFOLD "/pyexpat_d.pyd", 194560, 2589182738},
	{PYFOLD "/re.pyc", 13178, 1671609387},
	{PYFOLD "/shutil.pyc", 19273, 1873281015},
	{PYFOLD "/site.pyc", 20019, 3897044925},
	{PYFOLD "/sre_compile.pyc", 11107, 1620746411},
	{PYFOLD "/sre_constants.pyc", 6108, 3900811275},
	{PYFOLD "/sre_parse.pyc", 19244, 1459430047},
	{PYFOLD "/stat.pyc", 2791, 1375966108},
	{PYFOLD "/string.pyc", 19656, 1066063587},
	{PYFOLD "/sysconfig.pyc", 17571, 1529083148},
	{PYFOLD "/traceback.pyc", 11703, 3768933732},
	{PYFOLD "/types.pyc", 2530, 920695307},
	{PYFOLD "/UserDict.pyc", 9000, 1431875928},
	{PYFOLD "/warnings.pyc", 13232, 3752454002},
	{PYFOLD "/weakref.pyc", 16037, 2124701469},
	{PYFOLD "/xml/dom/domreg.pyc", 3506, 2127674645},
	{PYFOLD "/xml/dom/expatbuilder.pyc", 36698, 316034696},
	{PYFOLD "/xml/dom/minicompat.pyc", 4144, 747596376},
	{PYFOLD "/xml/dom/minidom.pyc", 74704, 1543233763},
	{PYFOLD "/xml/dom/nodefilter.pyc", 1243, 3805409468},
	{PYFOLD "/xml/dom/xmlbuilder.pyc", 18659, 4118801318},
	{PYFOLD "/xml/dom/__init__.pyc", 7337, 343751384},
	{PYFOLD "/xml/parsers/expat.pyc", 326, 2425747752},
	{PYFOLD "/xml/parsers/__init__.pyc", 353, 1691127318},
	{PYFOLD "/xml/__init__.pyc", 1117, 3531597556},
	{PYFOLD "/_abcoll.pyc", 22339, 2365844594},
	{PYFOLD "/_locale.pyc", 49841, 4114662314},
	{PYFOLD "/_weakrefset.pyc", 10490, 1576811346},
	{PYFOLD "/__future__.pyc", 4431, 2857792867},
#	elif PY_VERSION_HEX == 0x020203f0
#	else
#		error "unknown python version"
#	endif
};

bool checkPyLibDir(const std::string szDirName)
{
	bool HasHack = false;

	char szDirNamePath[MAX_PATH];
	sprintf(szDirNamePath, "%s\\*", szDirName.c_str());

	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(szDirNamePath, &f);

	if (h == INVALID_HANDLE_VALUE)
		return HasHack;

	do
	{
		if (HasHack)
			break;
		const char * name = f.cFileName;

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char filePath[MAX_PATH];
			sprintf(filePath, "%s%s%s", szDirName.c_str(), "\\", name);
			PRINTME(1, "sub %s", filePath);
			checkPyLibDir(filePath);
		}
		else
		{
			// start processing file
			PRINTME(1, "starting %s", name);
			std::string sName(name);
			std::string sPathName(szDirName + "/" + name);
			// change \\ to /
			std::replace(sPathName.begin(), sPathName.end(), '\\', '/');
			PRINTME(1, "path %s", sPathName.c_str());
			// lower file name
			std::transform(sName.begin(), sName.end(), sName.begin(), tolower);
			{
				PRINTME(1, "verify %s", sName.c_str());
				bool isPyLibFound = false;
				for (const auto & elem : PyLibFilesTable)
				{
					if (!elem.sFileName.compare(sPathName))
					{
						PRINTME(1, "found %s==%s", elem.sFileName.c_str(), sName.c_str());
						const uint32_t dwCrc32 = GetFileCRC32(sPathName.c_str());
						// assert(dwCrc32);
						const uint32_t dwFileSize = f.nFileSizeLow;
						if (elem.stSize != dwFileSize)
						{
							PRINTME(1, "wrong size %u==%u", elem.stSize, dwFileSize);
							HasHack = true;
							PRINTME(0, "wrong size %u for %s", dwFileSize, sPathName.c_str());
							return HasHack;
						}
						if (elem.dwCRC32 != dwCrc32)
						{
							PRINTME(1, "wrong crc32 %u==%u", elem.dwCRC32, dwCrc32);
							HasHack = true;
							PRINTME(0, "wrong crc32 %u for %s", dwCrc32, sPathName.c_str());
							return HasHack;
						}
						PRINTME(1, "right size %u==%u", elem.stSize, dwFileSize);
						PRINTME(1, "right crc32 %u==%u", elem.dwCRC32, dwCrc32);
						PRINTME(2, "{ \"%s\", %u, %u},", sPathName.c_str(), dwFileSize, dwCrc32);
						isPyLibFound = true;
						break;
					}
				}
				// block ambiguous pyc/d files
				if (!isPyLibFound)
				{
					PRINTME(1, "not found %s", sName.c_str());
#	ifdef PYFORCE
					HasHack = true;
					PRINTME(0, "ambiguous file for %s", sPathName.c_str());
					return HasHack;
#	endif
				}
				PRINTME(1, "skipping file(%s) hack(%u) found(%u)", sName.c_str(), HasHack, isPyLibFound);
			}
		}

	} while (FindNextFile(h, &f));
	FindClose(h);
	return HasHack;
}

bool __CheckPyLibFiles()
{
	PRINTME(1, "__CheckPyLibFiles processing " PYFOLD);
	if (checkPyLibDir(PYFOLD))
		return false;
	return true;
}
#endif

#ifdef ENABLE_MILES_CHECK
#	include <algorithm>
#	include "../EterBase/Filename.h"
// #include "../EterBase/CRC32.h"
#	define PRINT_LEVEL 0
#	define PRINTME(level, ...)   \
		if (PRINT_LEVEL >= level) \
			TraceError(__VA_ARGS__);

typedef struct MilesFiles_s
{
	std::string sFileName;
	size_t stSize;
	uint32_t dwCRC32;
} MilesFiles_t;

typedef struct MilesExten_s
{
	std::string ExtName;
	bool IsUni;
} MilesExten_t;

const MilesExten_t MilesExtenTable[] = {{"dll", false}, {"asi", true}, {"flt", true}, {"m3d", true}, {"mix", true}};

const MilesFiles_t MilesFilesTable[] = {{"mss32.dll", 349696, 1817711331},  {"mssa3d.m3d", 83456, 1812642892},
										{"mssds3d.m3d", 70656, 2704596484}, {"mssdsp.flt", 93696, 3364819387},
										{"mssdx7.m3d", 80896, 236402185},   {"msseax.m3d", 103424, 3195814903},
										{"mssmp3.asi", 125952, 1219814613}, {"mssrsx.m3d", 354816, 550946743},
										{"msssoft.m3d", 67072, 4284421368}, {"mssvoice.asi", 197120, 1407967464}};

bool checkMilesDir(const std::string szDirName)
{
	bool HasHack = false;

	char szDirNamePath[MAX_PATH];
	sprintf(szDirNamePath, "%s\\*", szDirName.c_str());

	WIN32_FIND_DATA f;
	HANDLE h = FindFirstFile(szDirNamePath, &f);

	if (h == INVALID_HANDLE_VALUE)
		return HasHack;

	do
	{
		if (HasHack)
			break;
		const char * name = f.cFileName;

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;

		if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// do nothing if it's a folder
		}
		else
		{
			// start processing file
			PRINTME(1, "starting %s", name);
			std::string sName(name);
			std::string sPathName(szDirName + "/" + name);
			PRINTME(1, "path %s", sPathName.c_str());
			// lower file name
			std::transform(sName.begin(), sName.end(), sName.begin(), tolower);
			// file or symlink; check for asi flt m3d mix exploit
			std::string sNameExt = CFileNameHelper::GetExtension(sName);
			PRINTME(1, "ext %s", sNameExt.c_str());
			// workaround instead of std::find
			bool isMilesFile = false, isMilesUnique = false;
			for (const auto & elem : MilesExtenTable)
			{
				PRINTME(1, "is %s ? %s", elem.ExtName.c_str(), sNameExt.c_str());
				if (!sNameExt.compare(0, elem.ExtName.length(), elem.ExtName))
				{
					isMilesFile = true;
					isMilesUnique = elem.IsUni;
				}
			}
			if (isMilesFile)
			{
				PRINTME(1, "verify %s -> %s", sName.c_str(), sNameExt.c_str());
				bool isMilesFound = false;
				for (const auto & elem : MilesFilesTable)
				{
					if (!elem.sFileName.compare(sName))
					{
						PRINTME(1, "found %s==%s", elem.sFileName.c_str(), sName.c_str());
						const uint32_t dwCrc32 = GetFileCRC32(sPathName.c_str());
						// assert(dwCrc32);
						const uint32_t dwFileSize = f.nFileSizeLow;
						if (elem.stSize != dwFileSize)
						{
							PRINTME(1, "wrong size %u==%u", elem.stSize, dwFileSize);
							HasHack = true;
							PRINTME(0, "wrong size %u for %s", dwFileSize, sPathName.c_str());
							return HasHack;
						}
						if (elem.dwCRC32 != dwCrc32)
						{
							PRINTME(1, "wrong crc32 %u==%u", elem.dwCRC32, dwCrc32);
							HasHack = true;
							PRINTME(0, "wrong crc32 %u for %s", dwCrc32, sPathName.c_str());
							return HasHack;
						}
						PRINTME(1, "right size %u==%u", elem.stSize, dwFileSize);
						PRINTME(1, "right crc32 %u==%u", elem.dwCRC32, dwCrc32);
						isMilesFound = true;
						break;
					}
				}
				// only mss32.dll is checked, and there's no need to check the others
				if (!isMilesFound && isMilesUnique)
				{
					PRINTME(1, "not found %s", sName.c_str());
					HasHack = true;
					PRINTME(0, "ambiguous file for %s", sPathName.c_str());
					return HasHack;
				}
				PRINTME(1, "skipping file(%s) hack(%u) found(%u) uni(%u)", sName.c_str(), HasHack, isMilesFound, isMilesUnique);
			}
		}

	} while (FindNextFile(h, &f));
	FindClose(h);
	return HasHack;
}

bool __CheckMilesFiles()
{
	PRINTME(1, "__CheckMilesFiles processing . and .\\miles");
	if (checkMilesDir(".") || checkMilesDir(".\\miles"))
		return false;
	return true;
}
#endif

#ifdef ENABLE_CRASHRPT
#	include "CrashRpt.h"
#	pragma comment(lib, "CrashRpt1500.lib")

int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO * pInfo)
{
	// The application has crashed!

	// Close the log file here
	// OpenLogFile
	return CR_CB_DODEFAULT;
}

void CrashRptUnregister()
{
	// Uninitialize CrashRpt before exiting the main function
	crUninstall();
}

void CrashRptInit()
{
	// Define CrashRpt configuration parameters
	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);
	info.pszAppName = "Desperadoz2 - The Darkness";
	info.pszAppVersion = VER_FILE_VERSION_STR;
	info.pszEmailSubject = "Desperadoz2 - The Darkness - Error Report";
	info.pszEmailTo = "report@desperadoz2.de";
	info.pszUrl = "https://desperadoz2.de/tools/crashrpt.php";
	info.uPriorities[CR_HTTP] = 3; // First try send report over HTTP
	info.uPriorities[CR_SMTP] = 2; // Second try send report over SMTP
	info.uPriorities[CR_SMAPI] = 1; // Third try send report over Simple MAPI
	// Install all available exception handlers
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
	// Restart the app on crash
	info.dwFlags |= CR_INST_APP_RESTART;
	info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
	info.pszRestartCmdLine = "/restart";
	// Define the Privacy Policy URL
	//info.pszPrivacyPolicyURL = "http://myapp.com/privacypolicy.html";

	// Install crash reporting
	int nResult = crInstall(&info);
	if (nResult != 0)
	{
		// Something goes wrong. Get error message.
		char szErrorMsg[512] = "";
		crGetLastErrorMsg(szErrorMsg, 512);
		TraceError("%s\n", szErrorMsg);
	}

	// Set crash callback function
	crSetCrashCallback(CrashCallback, NULL);
	// Add our log file to the error report
	crAddFile2("syserr.txt", NULL, "Log File", CR_AF_MAKE_FILE_COPY);
	// We want the screenshot of the entire desktop is to be added on crash
	crAddScreenshot2(CR_AS_VIRTUAL_SCREEN, CR_AV_QUALITY_LOW);
}
#endif

char gs_szErrorString[512] = "";

void ApplicationSetErrorString(const char * szErrorString) noexcept
{
	strcpy(gs_szErrorString, szErrorString);
}

bool CheckPythonLibraryFilenames()
{
	for (const auto & elem : sc_apszPythonLibraryFilenames)
	{
		std::string stFilename = "lib\\"s + elem;

		if (_access(stFilename.c_str(), 0) != 0)
			return false;

		MoveFile(stFilename.c_str(), stFilename.c_str());
	}

	return true;
}

struct ApplicationStringTable
{
	HINSTANCE m_hInstance;
	std::map<uint32_t, std::string> m_kMap_dwID_stLocale;
} gs_kAppStrTable;

void ApplicationStringTable_Initialize(HINSTANCE hInstance)
{
	gs_kAppStrTable.m_hInstance = hInstance;
}

const std::string & ApplicationStringTable_GetString(uint32_t dwID, LPCSTR szKey)
{
	char szBuffer[512];
	char szIniFileName[256];
	char szLocale[256];

	::GetCurrentDirectory(sizeof(szIniFileName), szIniFileName);
	if (szIniFileName[lstrlen(szIniFileName) - 1] != '\\')
		strcat(szIniFileName, "\\");
	strcat(szIniFileName, "Project-X.dat");

	strcpy(szLocale, LocaleService_GetLocalePath());
	if (strnicmp(szLocale, "locale/", strlen("locale/")) == 0)
		strcpy(szLocale, LocaleService_GetLocalePath() + strlen("locale/"));
	::GetPrivateProfileString(szLocale, szKey, nullptr, szBuffer, sizeof(szBuffer) - 1, szIniFileName);
	if (szBuffer[0] == '\0')
		LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer) - 1);
	if (szBuffer[0] == '\0')
		::GetPrivateProfileString("en", szKey, nullptr, szBuffer, sizeof(szBuffer) - 1, szIniFileName);
	if (szBuffer[0] == '\0')
		strcpy(szBuffer, szKey);

	std::string & rstLocale = gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale = szBuffer;

	return rstLocale;
}

const std::string & ApplicationStringTable_GetString(uint32_t dwID)
{
	char szBuffer[512];

	LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer) - 1);
	std::string & rstLocale = gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale = szBuffer;

	return rstLocale;
}

const char * ApplicationStringTable_GetStringz(uint32_t dwID, LPCSTR szKey)
{
	return ApplicationStringTable_GetString(dwID, szKey).c_str();
}

const char * ApplicationStringTable_GetStringz(uint32_t dwID)
{
	return ApplicationStringTable_GetString(dwID).c_str();
}

////////////////////////////////////////////

int Setup(LPSTR lpCmdLine); // Internal function forward

void setGrannyLogCallback();

#ifdef ENABLE_PACK_TYPE_DIO
#  ifdef _WIN32
#      ifdef _DEBUG
#          pragma comment(lib, "liblz4_static-Debug.lib")
#          pragma comment(lib, "libPackTypeDio-Debug.lib")
#      else
#          pragma comment(lib, "liblz4_static-Release.lib")
#          pragma comment(lib, "libPackTypeDio-Release.lib")
#      endif
#  endif
#endif

#ifdef ENABLE_NO_INDEX_FILE
using vecIndex_t = std::vector<std::vector<std::string>>;
const vecIndex_t vecIndex =
{
	{"*", "metin2_patch_battlefied"},
	{"*", "BGM"},
	{"*", "icon"},
	{"*", "locale"},
	{"root/uiscript", "root"},
	{"*", "maps"},
	{"*", "property"},
	{"*", "season1"},
	{"*", "season2"},
	{"*", "sound"},
	{"*", "textureset"},
	{"*", "uiscript"},
	{"*", "yw_bin"},
	{"*", "yw_effect"},
	{"*", "yw_environment"},
	{"*", "yw_guild"},
	{"*", "yw_item"},
	{"*", "yw_monster"},
	{"*", "yw_monster2"},
	{"*", "yw_npc"},
	{"*", "yw_npc2"},
	{"*", "yw_npc_mount"},
	{"*", "yw_npc_pet"},
	{"*", "yw_pc"},
	{"*", "yw_pc2"},
	{"*", "yw_pc3"},
	{"*", "yw_special"},
	{"*", "yw_terrainmaps"},
	{"*", "yw_tree"},
	{"*", "yw_ui"},
	{"*", "yw_uiloading"},
	{"*", "yw_zone"},
	{"*", "ymir_work"},
	{"locale/common/", "locale"},
	{"locale/effect/", "locale"},
	{"locale/icon/", "locale"},
	{"locale/map/", "locale"},
	{"locale/questtimer/", "locale"},
	{"locale/ui/", "locale"},
	{"locale/country/de/", "locale"},
	{"locale/country/en/", "locale"},
	{"locale/country/es/", "locale"},
	{"locale/country/fr/", "locale"},
	{"locale/country/it/", "locale"},
	{"locale/country/pt/", "locale"},
	{"locale/country/ro/", "locale"},
	{"locale/country/pl/", "locale"},
	{"locale/country/tr/", "locale"},
};

bool PackInitialize(const char* c_pszFolder)
{
	if (_access(c_pszFolder, 0) != 0)
		return true;

	std::string stFolder(c_pszFolder);
	stFolder += "/";

	const bool bPackFirst = true;

#	if defined(USE_RELATIVE_PATH)
	CEterPackManager::Instance().SetRelativePathMode();
#	endif
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);

	CSoundData::SetPackMode();

	std::string strPackName, strTexCachePackName;
	for (auto& elem : vecIndex)
	{
		const std::string& c_rstFolder = elem[0];
		const std::string& c_rstName = elem[1];

		strPackName = stFolder + c_rstName;
		strTexCachePackName = strPackName + "_texcache";

		CEterPackManager::Instance().RegisterPack(strPackName.c_str(), c_rstFolder.c_str());
		CEterPackManager::Instance().RegisterPack(strTexCachePackName.c_str(), c_rstFolder.c_str());
	}

	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root")).c_str());
	NANOEND
		return true;
}
#else
bool PackInitialize(const char * c_pszFolder)
{
	NANOBEGIN
	if (_access(c_pszFolder, 0) != 0)
		return true;

	std::string stFolder(c_pszFolder);
	stFolder += "/";

	std::string stFileName(stFolder);
	stFileName += "Index";

	CMappedFile file;
	LPCVOID pvData;

	if (!file.Create(stFileName.c_str(), &pvData, 0, 0))
	{
		LogBoxf("FATAL ERROR! File not exist: %s", stFileName.c_str());
		TraceError("FATAL ERROR! File not exist: %s", stFileName.c_str());
		return true;
	}

	CMemoryTextFileLoader TextLoader;
	TextLoader.Bind(file.Size(), pvData);

	bool bPackFirst = TRUE;

	const std::string & strPackType = TextLoader.GetLineString(0);

	if (strPackType.compare("FILE") && strPackType.compare("PACK"))
	{
		TraceError("Pack/Index has invalid syntax. First line must be 'PACK' or 'FILE'");
		return false;
	}

#ifdef NDEBUG // @warme601 _DISTRIBUTE -> NDEBUG
	Tracef("Note: PackFirst mode enabled. [pack]\n");
#else
	bPackFirst = FALSE;
	Tracef("Note: PackFirst mode not enabled. [file]\n");
#endif

	CTextFileLoader::SetCacheMode();
#if defined(USE_RELATIVE_PATH)
	CEterPackManager::Instance().SetRelativePathMode();
#endif
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);

	CSoundData::SetPackMode(); // Miles 파일 콜백을 셋팅

	std::string strPackName, strTexCachePackName;
	for (uint32_t i = 1; i < TextLoader.GetLineCount() - 1; i += 2)
	{
		const std::string & c_rstFolder = TextLoader.GetLineString(i);
		const std::string & c_rstName = TextLoader.GetLineString(i + 1);

		strPackName = stFolder + c_rstName;
		strTexCachePackName = strPackName + "_texcache";

		CEterPackManager::Instance().RegisterPack(strPackName.c_str(), c_rstFolder.c_str());
		CEterPackManager::Instance().RegisterPack(strTexCachePackName.c_str(), c_rstFolder.c_str());
	}

	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root")).c_str());
	NANOEND
	return true;
}
#endif

bool RunMainScript(CPythonLauncher & pyLauncher, const char * lpCmdLine)
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	/////////////////////////////////////////////
	initapp();
	initsystemSetting();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initProfiler();
	initEvent();
	initeffect();
	initfly();
	initsnd();
#ifdef ENABLE_EVENT_MANAGER
	initeventmgr();
#endif
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	initguildbank();
#endif
	initguild();
	initServerStateChecker();
#ifdef ENABLE_SWITCHBOT
	initSwitchbot();
#endif
#ifdef ENABLE_RENDER_TARGET
	initRenderTarget();
#endif
#ifdef ENABLE_CUBE_RENEWAL
	intcuberenewal();
#endif
#ifdef ENABLE_RANKING_SYSTEM
	initRanking();
#endif
#ifdef ENABLE_INGAME_WIKI
	initWiki();
#endif
#ifdef ENABLE_TICKET_SYSTEM
	initTicket();
#endif
#ifdef ENABLE_MAILBOX
	initmailbox();
#endif
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	initDungeonInfo();
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	InitAchievementModule();
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	initBiologManager();
#endif

#ifdef __USE_CYTHON__
	// don't add this line if you're implementing cython via .pyd:
	initrootlibManager();
#endif
#ifdef __USE_EXTRA_CYTHON__
	// don't add this line if you're implementing cython via .pyd:
	inituiscriptlibManager();
#endif

	NANOBEGIN

	PythonRegisterCommandLine(lpCmdLine);
	if (!PythonRun(lpCmdLine, pyLauncher))
		return false;

	NANOEND
	return true;
}

bool PythonRun(const char * lpCmdLine, CPythonLauncher & pyLauncher)
{
	std::vector<std::string> stVec;
	SplitLine(lpCmdLine, " ", &stVec);

#ifdef __USE_CYTHON__
	if (!pyLauncher.RunLine("import rootlib\nrootlib.moduleImport('system')"))
#else
	if (!pyLauncher.RunFile("system.py"))
#endif
	{
		TraceError("RunMain Error");
		return false;
	}
	return true;
}

void PythonRegisterCommandLine(const char * lpCmdLine)
{
	PyObject * builtins = PyImport_ImportModule("__builtin__");
#ifdef NDEBUG // @warme601 _DISTRIBUTE -> NDEBUG
	PyModule_AddIntConstant(builtins, "__DEBUG__", 0);
#else
	PyModule_AddIntConstant(builtins, "__DEBUG__", 1);
#endif
#ifdef __USE_CYTHON__
	PyModule_AddIntConstant(builtins, "__USE_CYTHON__", 1);
#else
	PyModule_AddIntConstant(builtins, "__USE_CYTHON__", 0);
#endif
#ifdef __USE_EXTRA_CYTHON__
	PyModule_AddIntConstant(builtins, "__USE_EXTRA_CYTHON__", 1);
#else
	PyModule_AddIntConstant(builtins, "__USE_EXTRA_CYTHON__", 0);
#endif

	// RegisterCommandLine
	{
		std::string stRegisterCmdLine;

		const char * loginMark = "-cs";
		const char * loginMark_NonEncode = "-ncs";
		const char * seperator = " ";

		std::string stCmdLine;
		const int CmdSize = 3;
		std::vector<std::string> stVec;
		SplitLine(lpCmdLine, seperator, &stVec);
		if (CmdSize == stVec.size() && stVec[0] == loginMark)
		{
			char buf[MAX_PATH]; //TODO 아래 함수 string 형태로 수정
			base64_decode(stVec[2].c_str(), buf);
			stVec[2] = buf;
			string_join(seperator, stVec, &stCmdLine);
		}
		else if (CmdSize <= stVec.size() && stVec[0] == loginMark_NonEncode)
		{
			stVec[0] = loginMark;
			string_join(" ", stVec, &stCmdLine);
		}
		else
			stCmdLine = lpCmdLine;

		PyModule_AddStringConstant(builtins, "__COMMAND_LINE__", stCmdLine.c_str());
	}
}

#if defined(ENABLE_IE11_UPDATE)
void SetInternetRegKey()
{
	LONG status;
	HKEY hKey;

	status = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION"), 0, KEY_ALL_ACCESS, &hKey);
	if ((status == ERROR_SUCCESS) && (hKey != nullptr))
	{
		uint32_t standard = 11000;
		uint32_t version;
		unsigned long size = sizeof(version);
		unsigned long type = REG_DWORD;
		status = RegQueryValueEx(hKey, TEXT("YOURCLIENTEXECUTABLEFILE.EXTENSION"), nullptr, &type, (uint8_t*)&version, &size);
		if (status != ERROR_SUCCESS)
		{
			status = RegSetValueEx(hKey, TEXT("YOURCLIENTEXECUTABLEFILE.EXTENSION"), 0, REG_DWORD, (uint8_t*)&standard, sizeof(standard));
			if (status != ERROR_SUCCESS)
			{

			}
		}
		RegCloseKey(hKey);
	}
}
#endif

bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
#ifdef ENABLE_CRASHRPT
	// CrThreadAutoInstallHelper cr_thread_install_helper(); unneccessary
	CrashRptInit();
#endif
#ifdef LOCALE_SERVICE_YMIR
	extern bool g_isScreenShotKey;
	g_isScreenShotKey = true;
#endif

#if defined(ENABLE_IE11_UPDATE)
	SetInternetRegKey();
#endif

	uint32_t dwRandSeed = time(nullptr) + uint32_t(GetCurrentProcess());
	srandom(dwRandSeed);
	srand(random());

#if defined(EVENT_HANDLER_MASTER)
	EventHandler EventHandle;
#endif

	SetLogLevel(1);

	ilInit();

	if (!Setup(lpCmdLine))
		return false;

#ifdef _DEBUG
	OpenConsoleWindow();
	OpenLogFile(true); // true == uses syserr.txt and log.txt
#else
	OpenLogFile(false); // false == uses syserr.txt only
#endif

	static CLZO lzo;
	static CEterPackManager EterPackManager;

	if (!PackInitialize("pack"))
	{
		LogBox("Pack Initialization failed. Check log.txt file..");
		return false;
	}

#ifdef ENABLE_PYLIB_CHECK
	if (!__CheckPyLibFiles())
		return false;
#endif
#ifdef ENABLE_MILES_CHECK
	if (!__CheckMilesFiles())
		return false;
#endif
	auto app = std::make_unique<CPythonApplication>();
	app->Initialize(hInstance);

	bool ret = false;
	{
		CPythonLauncher pyLauncher;
		CPythonExceptionSender pyExceptionSender;
		SetExceptionSender(&pyExceptionSender);

		if (pyLauncher.Create())
		{
			ret = RunMainScript(pyLauncher, lpCmdLine); //게임 실행중엔 함수가 끝나지 않는다.
		}

		//게임 종료시.
		app->Clear();

		timeEndPeriod(1);
		pyLauncher.Clear();
	}

	app->Destroy();
	return ret;
}

HANDLE CreateMetin2GameMutex()
{
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength				= sizeof(sa);
	sa.lpSecurityDescriptor	= nullptr;
	sa.bInheritHandle		= FALSE;

	return CreateMutex(&sa, FALSE, "Metin2GameMutex");
}

void DestroyMetin2GameMutex(HANDLE hMutex)
{
	if (hMutex)
	{
		ReleaseMutex(hMutex);
		hMutex = nullptr;
	}
}

void __ErrorPythonLibraryIsNotExist()
{
	LogBoxf("FATAL ERROR!! Python Library file not exist!");
}

bool __IsTimeStampOption(const LPSTR lpCmdLine)
{
	const char * TIMESTAMP = "/timestamp";
	return (strncmp(lpCmdLine, TIMESTAMP, strlen(TIMESTAMP)) == 0);
}

void __PrintTimeStamp()
{
#ifdef _DEBUG
	LogBoxf("METIN2 BINARY DEBUG VERSION %s ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);

#else
	LogBoxf("METIN2 BINARY DISTRIBUTE VERSION %s ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
#endif
}

bool __IsLocaleOption(const LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--locale") == 0);
}

bool __IsLocaleVersion(const LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--perforce-revision") == 0);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ApplicationStringTable_Initialize(hInstance);

	LocaleService_LoadConfig("locale.cfg");
	SetDefaultCodePage(LocaleService_GetCodePage());

	bool bQuit = false;
	int nArgc = 0;
	PCHAR * szArgv = CommandLineToArgv(lpCmdLine, &nArgc);

	for (int i = 0; i < nArgc; i++)
	{
		if (szArgv[i] == nullptr)
			continue;
		if (__IsLocaleVersion(szArgv[i])) // #0000829: [M2EU] 버전 파일이 항상 생기지 않도록 수정
		{
			char szModuleName[MAX_PATH];
			char szVersionPath[MAX_PATH];
			GetModuleFileName(nullptr, szModuleName, sizeof(szModuleName));
			sprintf(szVersionPath, "%s.version", szModuleName);
			msl::file_ptr fPtr(szVersionPath, "wt");
			if (fPtr)
				fprintf(fPtr.get(), "r%d\n", METIN2_GET_VERSION());
			bQuit = true;
		}
		else if (__IsLocaleOption(szArgv[i]))
		{
			msl::file_ptr fPtr("locale.txt", "wt");
			fprintf(fPtr.get(), "service[%s] code_page[%d]", LocaleService_GetName(), LocaleService_GetCodePage());
			bQuit = true;
		}
		else if (__IsTimeStampOption(szArgv[i]))
		{
			__PrintTimeStamp();
			bQuit = true;
		}
		else if ((strcmp(szArgv[i], "--force-set-locale") == 0))
		{
			// locale 설정엔 인자가 두 개 더 필요함 (로케일 명칭, 데이터 경로)
			if (nArgc <= i + 2)
			{
				MessageBox(nullptr, "Invalid arguments", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
				goto Clean;
			}

			const char * localeName = szArgv[++i];
			const char * localePath = szArgv[++i];

			LocaleService_ForceSetLocale(localeName, localePath);
		}
	}

	if (bQuit)
		goto Clean;

#if defined(NEEDED_COMMAND_ARGUMENT)
	// 옵션이 없으면 비정상 실행으로 간주, 프로그램 종료
	if (strstr(lpCmdLine, NEEDED_COMMAND_ARGUMENT) == 0)
	{
		MessageBox(nullptr, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"),
				   ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
		goto Clean;
	}
#endif

#if defined(NEEDED_COMMAND_CLIPBOARD)
	{
		CHAR szSecKey[256];
		CPostIt cPostIt("VOLUME1");

		if (cPostIt.Get("SEC_KEY", szSecKey, sizeof(szSecKey)) == FALSE)
		{
			MessageBox(nullptr, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"),
					   ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
		}
		if (strstr(szSecKey, NEEDED_COMMAND_CLIPBOARD) == 0)
		{
			MessageBox(nullptr, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"),
					   ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
		}
		cPostIt.Empty();
	}
#endif

#ifdef CEF_BROWSER
	CefWebBrowser_Startup(hInstance);
#else
	WebBrowser_Startup(hInstance);
#endif

#ifndef ENABLE_PYLIB_CHECK
	if (!CheckPythonLibraryFilenames())
	{
		__ErrorPythonLibraryIsNotExist();
		goto Clean;
	}
#endif
	Main(hInstance, lpCmdLine);

#ifdef CEF_BROWSER
	CefWebBrowser_Cleanup();
#else
	WebBrowser_Cleanup();
#endif

	CoUninitialize();

	if (gs_szErrorString[0])
		MessageBox(nullptr, gs_szErrorString, ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);

Clean:
	SAFE_FREE_GLOBAL(szArgv);

	return 0;
}

#if GrannyProductMinorVersion == 4 || GrannyProductMinorVersion == 7
static void GrannyError(granny_log_message_type Type, granny_log_message_origin Origin, char const * Error, void * UserData)
{
	TraceError("GRANNY: %s", Error);
}
#elif GrannyProductMinorVersion == 11 || GrannyProductMinorVersion == 9 || GrannyProductMinorVersion == 8
static void GrannyError(granny_log_message_type Type, granny_log_message_origin Origin, char const * File, granny_int32x Line,
						char const * Error, void * UserData)
{
	//Origin==GrannyFileReadingLogMessage for granny run-time tag& revision warning (Type==GrannyWarningLogMessage)
	//Origin==GrannyControlLogMessage for miss track_group on static models as weapons warning (Type==GrannyWarningLogMessage)
	//Origin==GrannyMeshBindingLogMessage for miss bone ToSkeleton on new ymir models error (Type==GrannyErrorLogMessage)
	// if (Type == GrannyWarningLogMessage)
	if (Origin == GrannyFileReadingLogMessage || Origin == GrannyControlLogMessage || Origin == GrannyMeshBindingLogMessage)
		return;
#ifndef HIDE_GRANNY_SYSERR
	TraceError("GRANNY: %s(%d): ERROR: %s --- [%d] %s --- [%d] %s", File, Line, Error, Type, GrannyGetLogMessageTypeString(Type), Origin,
		GrannyGetLogMessageOriginString(Origin));
#endif
}
#else
#	error "unknown granny version"
#endif

int Setup(LPSTR lpCmdLine)
{
	/*
	 *	타이머 정밀도를 올린다.
	 */
	TIMECAPS tc;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		return 0;

	uint32_t wTimerRes = MINMAX(tc.wPeriodMin, 1, tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);

	setGrannyLogCallback();
	return 1;
}

void setGrannyLogCallback()
{
	granny_log_callback Callback;
	Callback.Function = GrannyError;
	Callback.UserData = nullptr;
	GrannySetLogCallback(&Callback);
}
