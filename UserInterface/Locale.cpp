#include "StdAfx.h"
#include "Locale.h"
#include "PythonApplication.h"
#include "resource.h"
#include "../EterBase/CRC32.h"
#include "../EterPack/EterPackManager.h"
#include "../EterLocale/Japanese.h"
#include <windowsx.h>

const char * LSS_YMIR = "YMIR";
const char * LSS_JAPAN = "JAPAN";
const char * LSS_ENGLISH = "ENGLISH";
const char * LSS_HONGKONG = "HONGKONG";
const char * LSS_TAIWAN = "TAIWAN";
const char * LSS_NEWCIBN = "NEWCIBN";
const char * LSS_EUROPE = "EUROPE";
const char * LSS_GLOBAL = "GLOBAL";

static bool IS_CHEONMA = false;

#ifndef LSS_SECURITY_KEY
#ifdef _IMPROVED_PACKET_ENCRYPTION_
#	define LSS_SECURITY_KEY	"testtesttesttest"	// Enable this, when _IMPROVED_PACKET_ENCRYPTION_ is active [EterBase\ServiceDefs.h] ~ [GameSource -> common/service.h]
#else
#	define LSS_SECURITY_KEY    "1234abcd5678efgh"	// Enable this, when _IMPROVED_PACKET_ENCRYPTION_ is disabled
#endif
#endif

std::string __SECURITY_KEY_STRING__ = LSS_SECURITY_KEY;

char MULTI_LOCALE_SERVICE[256] = "YMIR";
char MULTI_LOCALE_PATH[256] = "locale/ymir";
char MULTI_LOCALE_NAME[256] = "ymir";
int MULTI_LOCALE_CODE = 949;
int MULTI_LOCALE_REPORT_PORT = 10000;

void LocaleService_LoadConfig(const char * fileName)
{
	NANOBEGIN

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	bool bMultiLocale = false;
	const char* szMultiLocale = "loca.cfg";
	if (_access(szMultiLocale, 0) == 0)
	{
		bMultiLocale = true;
		fileName = szMultiLocale;
	}
#endif

	msl::file_ptr fPtr(fileName, "rt");
	if (fPtr)
	{
		char line[256];
		char name[256];
		int code;
		int id;
		if (fgets(line, sizeof(line) - 1, fPtr.get()))
		{
			line[sizeof(line) - 1] = '\0';
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			if (bMultiLocale)
				sscanf(line, "%d %s", &code, name);
			else
			{
				sscanf(line, "%d %d %s", &id, &code, name);
				MULTI_LOCALE_REPORT_PORT = id;
			}
#else
			sscanf(line, "%d %d %s", &id, &code, name);

			MULTI_LOCALE_REPORT_PORT = id;
#endif
			MULTI_LOCALE_CODE = code;
			strcpy(MULTI_LOCALE_NAME, name);
			sprintf(MULTI_LOCALE_PATH, "locale/country/%s", MULTI_LOCALE_NAME);	//@custom016
		}
	}
	NANOEND
}

unsigned LocaleService_GetLastExp(int level)
{
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_GUILD_LEVEL)
	static const int GUILD_LEVEL_MAX = 40;
#else
	static const int GUILD_LEVEL_MAX = 20;
#endif

	static uint32_t INTERNATIONAL_GUILDEXP_LIST[GUILD_LEVEL_MAX + 1] =
	{
		0,			// 0
		6000UL,		// 1
		18000UL,	// 2
		36000UL,	// 3
		64000UL,	// 4
		94000UL,	// 5
		130000UL,	// 6
		172000UL,	// 7
		220000UL,	// 8
		274000UL,	// 9
		334000UL,	// 10
		400000UL,	// 11
		600000UL,	// 12
		840000UL,	// 13
		1120000UL,	// 14
		1440000UL,	// 15
		1800000UL,	// 16
		2600000UL,	// 17
		3200000UL,	// 18
		4000000UL,	// 19
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_GUILD_LEVEL)
		45000UL,
		47250UL,
		49500UL,
		51750UL,
		54000UL,
		56250UL,
		58500UL,
		60750UL,
		63000UL,
		65250UL, // 29
		67500UL,
		69750UL,
		72000UL,
		74250UL,
		76500UL,
		78750UL,
		81000UL,
		83250UL,
		85500UL,
		87750UL, // 39
#endif
		16800000UL	// 20
	};

	if (level < 0 && level >= GUILD_LEVEL_MAX)
		return 0;

	return INTERNATIONAL_GUILDEXP_LIST[level];
}

int LocaleService_GetSkillPower(unsigned level)
{
	static const unsigned SKILL_POWER_NUM = 50;

	if (level >= SKILL_POWER_NUM)
		return 0;

	// 0 5 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 40 50 52 54 56 58 60 63 66 69 72 82 85 88 91 94 98 102 106 110 115 125 125 125 125 125
	static unsigned INTERNATIONAL_SKILL_POWERS[SKILL_POWER_NUM] = {0,   5,  6,  8,  10, 12,  14,  16,  18,  20,
																   22,  24, 26, 28, 30, 32,  34,  36,  38,  40,
																   50, // master
																   52,  54, 56, 58, 60, 63,  66,  69,  72,
																   82, // grand_master
																   85,  88, 91, 94, 98, 102, 106, 110, 115,
																   125, // perfect_master
																   125};
	return INTERNATIONAL_SKILL_POWERS[level];
}

const char * LocaleService_GetSecurityKey()
{
	return __SECURITY_KEY_STRING__.c_str();
}

// CHEONMA
void LocaleService_SetCHEONMA(bool isEnable)
{
	IS_CHEONMA = isEnable;
}

bool LocaleService_IsCHEONMA()
{
	return LocaleService_IsYMIR();
}
// END_OF_CHEONMA

#if defined(LOCALE_SERVICE_EUROPE) || defined(LOCALE_SERVICE_BRAZIL) || defined(LOCALE_SERVICE_CANADA) ||     \
	defined(LOCALE_SERVICE_SINGAPORE) || defined(LOCALE_SERVICE_VIETNAM) || defined(LOCALE_SERVICE_TAIWAN) || \
	defined(LOCALE_SERVICE_NEWCIBN)
#	define _LSS_USE_LOCALE_CFG 1
#	define _LSS_SERVICE_NAME LSS_EUROPE
#elif defined(LOCALE_SERVICE_ITALY)
#	define _LSS_SERVICE_NAME LSS_ITALY
#	define _LSS_SERVICE_CODEPAGE CP_LATIN
#	define _LSS_SERVICE_LOCALE_NAME "it"
#	define _LSS_SERVICE_LOCALE_PATH "locale/it"
#elif defined(LOCALE_SERVICE_ENGLISH)
#	define _LSS_SERVICE_NAME LSS_ENGLISH
#	define _LSS_SERVICE_CODEPAGE CP_LATIN
#	define _LSS_SERVICE_LOCALE_NAME "english"
#	define _LSS_SERVICE_LOCALE_PATH "locale/english"
#elif defined(LOCALE_SERVICE_JAPAN)
#	define _LSS_SERVICE_NAME LSS_JAPAN
#	define _LSS_SERVICE_CODEPAGE CP_JAPANESE
#	define _LSS_SERVICE_LOCALE_NAME "japan"
#	define _LSS_SERVICE_LOCALE_PATH "locale/japan"
#elif defined(LOCALE_SERVICE_YMIR)
#	define _LSS_SERVICE_NAME LSS_YMIR
#	define _LSS_SERVICE_CODEPAGE CP_HANGUL
#	define _LSS_SERVICE_LOCALE_NAME "ymir"
#	define _LSS_SERVICE_LOCALE_PATH "locale/ymir"
#elif defined(LOCALE_SERVICE_HONGKONG)
#	define _LSS_SERVICE_NAME LSS_HONGKONG
#	define _LSS_SERVICE_CODEPAGE CP_CHINESE_TRAD
#	define _LSS_SERVICE_LOCALE_NAME "hongkong"
#	define _LSS_SERVICE_LOCALE_PATH "locale/hongkong"
#elif defined(LOCALE_SERVICE_TAIWAN)
#	define _LSS_SERVICE_NAME LSS_TAIWAN
#	define _LSS_SERVICE_CODEPAGE CP_CHINESE_TRAD
#	define _LSS_SERVICE_LOCALE_NAME "taiwan"
#	define _LSS_SERVICE_LOCALE_PATH "locale/taiwan"
#elif defined(LOCALE_SERVICE_NEWCIBN)
#	define _LSS_SERVICE_NAME LSS_NEWCIBN
#	define _LSS_SERVICE_CODEPAGE CP_CHINESE_SIMPLE
#	define _LSS_SERVICE_LOCALE_NAME "newcibn"
#	define _LSS_SERVICE_LOCALE_PATH "locale/newcibn"
#endif

#if defined(_LSS_USE_LOCALE_CFG)
#	if defined(_LSS_SERVICE_NAME)
const char * LocaleService_GetName()
{
	return _LSS_SERVICE_NAME;
}
#	else
const char * LocaleService_GetName()
{
	return MULTI_LOCALE_SERVICE;
}
#	endif
unsigned int LocaleService_GetCodePage()
{
	return MULTI_LOCALE_CODE;
}
const char * LocaleService_GetLocaleName()
{
	return MULTI_LOCALE_NAME;
}
const char * LocaleService_GetLocalePath()
{
	return MULTI_LOCALE_PATH;
}
#elif defined(_LSS_SERVICE_NAME)
const char * LocaleService_GetName()
{
	return _LSS_SERVICE_NAME;
}
unsigned int LocaleService_GetCodePage()
{
	return _LSS_SERVICE_CODEPAGE;
}
const char * LocaleService_GetLocaleName()
{
	return _LSS_SERVICE_LOCALE_NAME;
}
const char * LocaleService_GetLocalePath()
{
	return _LSS_SERVICE_LOCALE_PATH;
}
#endif

void LocaleService_ForceSetLocale(const char * name, const char * localePath)
{
	strcpy(MULTI_LOCALE_NAME, name);
	strcpy(MULTI_LOCALE_PATH, localePath);

	// 기존 천마 서버로 접속시에는 security key 변경 (WE 버전 클라로 천마서버 접속하기 위함)
	if (0 == stricmp(name, "ymir"))
		__SECURITY_KEY_STRING__ = "testtesttesttest";
	if (0 == stricmp(name, "we_korea"))
		__SECURITY_KEY_STRING__ = "1234abcd5678efgh";
}

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
uint8_t LocaleService_GetLocaleID()
{
	const char* c_szLocale = LocaleService_GetLocaleName();
	if (strcmp(c_szLocale, "en") == 0)
		return LANGUAGE_EN;
	else if (strcmp(c_szLocale, "ae") == 0)
		return LANGUAGE_AE;
	else if (strcmp(c_szLocale, "cz") == 0)
		return LANGUAGE_CZ;
	else if (strcmp(c_szLocale, "dk") == 0)
		return LANGUAGE_DK;
	else if (strcmp(c_szLocale, "fr") == 0)
		return LANGUAGE_FR;
	else if (strcmp(c_szLocale, "gr") == 0)
		return LANGUAGE_GR;
	else if (strcmp(c_szLocale, "nl") == 0)
		return LANGUAGE_NL;
	else if (strcmp(c_szLocale, "pl") == 0)
		return LANGUAGE_PL;
	else if (strcmp(c_szLocale, "hu") == 0)
		return LANGUAGE_HU;
	else if (strcmp(c_szLocale, "de") == 0)
		return LANGUAGE_DE;
	else if (strcmp(c_szLocale, "it") == 0)
		return LANGUAGE_IT;
	else if (strcmp(c_szLocale, "ru") == 0)
		return LANGUAGE_RU;
	else if (strcmp(c_szLocale, "pt") == 0)
		return LANGUAGE_PT;
	else if (strcmp(c_szLocale, "ro") == 0)
		return LANGUAGE_RO;
	else if (strcmp(c_szLocale, "es") == 0)
		return LANGUAGE_ES;
	else if (strcmp(c_szLocale, "tr") == 0)
		return LANGUAGE_TR;
	else
		return LANGUAGE_NONE;
}

///////////////////////////////////

typedef struct SLangauge
{
	uint8_t		bIndex;
	uint16_t	sCodePage;
	std::string	strLocale;
}TLanguage;

TLanguage aLanguage[LANGUAGE_MAX_NUM] =
{
	{ LANGUAGE_NONE,	0,		""   },
	{ LANGUAGE_EN,		1252,	"en" },
	{ LANGUAGE_AE,		1256,	"ae" },
	{ LANGUAGE_CZ,		1250,	"cz" },
	{ LANGUAGE_DK,		1252,	"dk" },
	{ LANGUAGE_FR,		1252,	"fr" },
	{ LANGUAGE_GR,		1253,	"gr" },
	{ LANGUAGE_NL,		1252,	"nl" },
	{ LANGUAGE_PL,		1250,	"pl" },
	{ LANGUAGE_HU,		1250,	"hu" },
	{ LANGUAGE_DE,		1252,	"de" },
	{ LANGUAGE_IT,		1252,	"it" },
	{ LANGUAGE_RU,		1251,	"ru" },
	{ LANGUAGE_PT,		1252,	"pt" },
	{ LANGUAGE_RO,		1250,	"ro" },
	{ LANGUAGE_ES,		1252,	"es" },
	{ LANGUAGE_TR,		1254,	"tr" },
};

uint16_t GetLanguageCodePage(uint8_t bLanguage)
{
	if (bLanguage > LANGUAGE_MAX_NUM)
		return 0;

	return aLanguage[bLanguage].sCodePage;
}

std::string GetLanguageLocale(uint8_t bLanguage)
{
	if (bLanguage > LANGUAGE_MAX_NUM)
		return "";

	return aLanguage[bLanguage].strLocale;
}

std::string GetLanguageIconPath(uint8_t bLanguage)
{
	if (bLanguage > LANGUAGE_MAX_NUM)
		return "";

	std::string c_szLocale = GetLanguageLocale(bLanguage);
	if (c_szLocale.empty())
		return "";

	char szFilePath[256];
	sprintf(szFilePath, "d:/ymir work/ui/intro/login/server_flag_%s.sub", c_szLocale.c_str());

	return std::string(szFilePath);
}

void UpdateLanguageLocale(uint8_t bLanguage)
{
	MULTI_LOCALE_CODE = GetLanguageCodePage(bLanguage);
	strcpy(MULTI_LOCALE_NAME, GetLanguageLocale(bLanguage).c_str());
	sprintf(MULTI_LOCALE_PATH, "locale/%s", MULTI_LOCALE_NAME);

	if (FILE* fp = fopen("loca.cfg", "wt"))
	{
		fprintf(fp, "%d %s\n", MULTI_LOCALE_CODE, MULTI_LOCALE_NAME);
		fclose(fp);
	}
	else
	{
		if (FILE* fp2 = fopen("locale.cfg", "wt"))
		{
			fprintf(fp, "%d %s\n", MULTI_LOCALE_CODE, MULTI_LOCALE_NAME);
			fclose(fp);
		}
	}
}
#endif

#ifdef ENABLE_EMPIRE_FLAG
std::string GetEmpireIconPath(uint8_t bEmpire)
{
	if (bEmpire > 3)
		return "";

	std::string c_szLocale = "";
	if (bEmpire == 1)
		c_szLocale = "shinsoo";
	else if (bEmpire == 2)
		c_szLocale = "chunjo";
	else if (bEmpire == 3)
		c_szLocale = "jinno";

	char szFilePath[256];
	sprintf(szFilePath, "d:/ymir work/ui/game/flag/empire/%s.tga", c_szLocale.c_str());

	return std::string(szFilePath);
}
#endif

bool LocaleService_IsYMIR()
{
	return (stricmp(LocaleService_GetName(), LSS_YMIR) == 0) || (stricmp(LocaleService_GetLocaleName(), "ymir") == 0);
}
bool LocaleService_IsJAPAN()
{
	return (stricmp(LocaleService_GetName(), LSS_JAPAN) == 0) || (stricmp(LocaleService_GetLocaleName(), "japan") == 0);
}
bool LocaleService_IsENGLISH()
{
	return (stricmp(LocaleService_GetName(), LSS_ENGLISH) == 0);
}
bool LocaleService_IsEUROPE()
{
	return (stricmp(LocaleService_GetName(), LSS_EUROPE) == 0);
}
bool LocaleService_IsHONGKONG()
{
	return (stricmp(LocaleService_GetName(), LSS_HONGKONG) == 0);
}
bool LocaleService_IsTAIWAN()
{
	return (stricmp(LocaleService_GetName(), LSS_TAIWAN) == 0);
}
bool LocaleService_IsNEWCIBN()
{
	return (stricmp(LocaleService_GetName(), LSS_NEWCIBN) == 0);
}

#if defined(LOCALE_SERVICE_WE_JAPAN)
BOOL LocaleService_IsLeadByte(const char chByte)
{
	return ShiftJIS_IsLeadByte(chByte);
}

int LocaleService_StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength)
{
	return ShiftJIS_StringCompareCI(szStringLeft, szStringRight, sizeLength);
}
#else
BOOL LocaleService_IsLeadByte(const char chByte)
{
	return ((static_cast<unsigned char>(chByte)) & 0x80) != 0;
}

int LocaleService_StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength)
{
	return strnicmp(szStringLeft, szStringRight, sizeLength);
}
#endif
