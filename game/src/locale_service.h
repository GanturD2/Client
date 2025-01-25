#ifndef __LOCALE_SERVCICE__
#define __LOCALE_SERVCICE__

bool LocaleService_Init(const std::string& c_rstServiceName);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
void LocaleService_LoadItemNameFile();
void LocaleService_LoadMobNameFile();
void LocaleService_LoadSkillNameFile();
#else
void LocaleService_LoadLocaleStringFile();
#endif
void LocaleService_LoadEmpireTextConvertTables();
void LocaleService_TransferDefaultSetting();
const std::string& LocaleService_GetBasePath();
const std::string& LocaleService_GetMapPath();
const std::string& LocaleService_GetQuestPath();

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

enum eLocalization
{
	LC_NOSET = 0,

	LC_YMIR,
	LC_JAPAN,
	LC_ENGLISH,
	LC_HONGKONG,
	LC_NEWCIBN,
	LC_GERMANY,
	LC_KOREA,
	LC_FRANCE,
	LC_ITALY,
	LC_SPAIN,
	LC_UK,
	LC_TURKEY,
	LC_POLAND,
	LC_PORTUGAL,
	LC_CANADA,
	LC_BRAZIL,
	LC_GREEK,
	LC_RUSSIA,
	LC_DENMARK,
	LC_BULGARIA,
	LC_CROATIA,
	LC_MEXICO,
	LC_ARABIA,
	LC_CZECH,
	LC_ROMANIA,
	LC_HUNGARY,
	LC_NETHERLANDS,
	LC_SINGAPORE,
	LC_VIETNAM,
	LC_THAILAND,
	LC_USA,
	LC_WE_KOREA, ///< World Edition version for korea
	LC_TAIWAN,
//#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	LC_EUROPE,
//#endif

	LC_MAX_VALUE
};

eLocalization LC_GetLocalType();

bool LC_IsLocale(const eLocalization t);
bool LC_IsYMIR();
bool LC_IsJapan();
bool LC_IsEnglish();
bool LC_IsHongKong();
bool LC_IsNewCIBN();
bool LC_IsGermany();
bool LC_IsKorea();
bool LC_IsEurope();
bool LC_IsCanada();
bool LC_IsBrazil();
bool LC_IsSingapore();
bool LC_IsVietnam();
bool LC_IsThailand();
bool LC_IsWE_Korea();
bool LC_IsTaiwan();

bool LC_IsWorldEdition();

#endif
