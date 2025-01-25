#pragma once

#include "Locale_inc.h"

bool LocaleService_IsYMIR();
bool LocaleService_IsJAPAN();
bool LocaleService_IsENGLISH();
bool LocaleService_IsHONGKONG();
bool LocaleService_IsTAIWAN();
bool LocaleService_IsNEWCIBN();
bool LocaleService_IsEUROPE();
bool LocaleService_IsWorldEdition();

unsigned LocaleService_GetCodePage();
const char * LocaleService_GetName();
const char * LocaleService_GetLocaleName();
const char * LocaleService_GetLocalePath();
const char * LocaleService_GetSecurityKey();
BOOL LocaleService_IsLeadByte(char chByte);
int LocaleService_StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength);

void LocaleService_ForceSetLocale(const char * name, const char * localePath);
void LocaleService_LoadConfig(const char * fileName);
unsigned LocaleService_GetLastExp(int level);
int LocaleService_GetSkillPower(unsigned level);

// CHEONMA
void LocaleService_SetCHEONMA(bool isEnable);
bool LocaleService_IsCHEONMA();
// END_OF_CHEONMA

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
uint8_t LocaleService_GetLocaleID();

///////////////////////////////////

enum ELanguages
{
	LANGUAGE_NONE,
	LANGUAGE_EN,
	LANGUAGE_AE,
	LANGUAGE_CZ,
	LANGUAGE_DK,
	LANGUAGE_FR,
	LANGUAGE_GR,
	LANGUAGE_NL,
	LANGUAGE_PL,
	LANGUAGE_HU,
	LANGUAGE_DE,
	LANGUAGE_IT,
	LANGUAGE_RU,
	LANGUAGE_PT,
	LANGUAGE_RO,
	LANGUAGE_ES,
	LANGUAGE_TR,

	LANGUAGE_MAX_NUM,
	LANGUAGE_DEFAULT = LANGUAGE_EN,
};

uint16_t GetLanguageCodePage(uint8_t bLanguage);
std::string GetLanguageLocale(uint8_t bLanguage);
std::string GetLanguageIconPath(uint8_t bLanguage);
void UpdateLanguageLocale(uint8_t bLanguage);
#endif

#ifdef ENABLE_EMPIRE_FLAG
std::string GetEmpireIconPath(uint8_t bEmpire);
#endif
