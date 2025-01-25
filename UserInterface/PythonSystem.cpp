#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonApplication.h"

#define DEFAULT_VALUE_ALWAYS_SHOW_NAME true

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
	m_poInterfaceHandler = poHandler;
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = nullptr;
}

void CPythonSystem::SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight)
{
	m_WindowStatus[iIndex].isVisible = iVisible;
	m_WindowStatus[iIndex].isMinimized = iMinimized;
	m_WindowStatus[iIndex].ixPosition = ix;
	m_WindowStatus[iIndex].iyPosition = iy;
	m_WindowStatus[iIndex].iHeight = iHeight;
}

void CPythonSystem::GetDisplaySettings()
{
	msl::refill(m_ResolutionList);
	m_ResolutionCount = 0;

	LPDIRECT3D9 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

	lpD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_WHQL_LEVEL, &d3dAdapterIdentifier);
	lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddmDesktop);

	// 이 어뎁터가 가지고 있는 디스플래이 모드갯수를 나열한다..
	const uint32_t dwNumAdapterModes = lpD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_UNKNOWN);
	for (uint32_t iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
		lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_UNKNOWN, iMode, &DisplayMode);
		uint32_t bpp = 0;

		// 800 600 이상만 걸러낸다.
		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		// 일단 16bbp 와 32bbp만 취급하자.
		// 16bbp만 처리하게끔 했음 - [levites]
		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp || m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;

			// 프리퀀시만 다르므로 프리퀀시만 셋팅해준다.
			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			// 새로운 거니까 추가해주자.
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width = DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height = DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp = bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0] = DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count = 1;

				++m_ResolutionCount;
			}
		}
	}
}

int CPythonSystem::GetResolutionCount() const
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index) const
{
	if (index >= m_ResolutionCount)
		return 0;

	return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT uint32_t * width, OUT uint32_t * height, OUT uint32_t * bpp) const
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT uint32_t * frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int CPythonSystem::GetResolutionIndex(uint32_t width, uint32_t height, uint32_t bit) const
{
	uint32_t re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int CPythonSystem::GetFrequencyIndex(int res_index, uint32_t frequency)
{
	uint32_t re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

uint32_t CPythonSystem::GetWidth() const
{
	return m_Config.width;
}

uint32_t CPythonSystem::GetHeight() const
{
	return m_Config.height;
}
uint32_t CPythonSystem::GetBPP() const
{
	return m_Config.bpp;
}
uint32_t CPythonSystem::GetFrequency() const
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard() const
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor() const
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume() const
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume() const
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

int CPythonSystem::GetDistance() const
{
	return m_Config.iDistance;
}

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
int CPythonSystem::GetShadowTargetLevel()
{
	return m_Config.iShadowTargetLevel;
}

void CPythonSystem::SetShadowTargetLevel(unsigned int level)
{
	m_Config.iShadowTargetLevel = MINMAX(CPythonBackground::SHADOW_NONE, level, CPythonBackground::SHADOW_ALL);
	CPythonBackground::Instance().RefreshShadowTargetLevel();
}

int CPythonSystem::GetShadowQualityLevel()
{
	return m_Config.iShadowQualityLevel;
}

void CPythonSystem::SetShadowQualityLevel(unsigned int level)
{
	m_Config.iShadowQualityLevel = MINMAX(CPythonBackground::SHADOW_BAD, level, CPythonBackground::SHADOW_GOOD);
	CPythonBackground::Instance().RefreshShadowQualityLevel();
}
#else
int CPythonSystem::GetShadowLevel() const
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::Instance().RefreshShadowLevel();
}
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
void CPythonSystem::SetNightModeOption(int iOpt)
{
	m_Config.bNightMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetNightModeOption()
{
	return m_Config.bNightMode;
}

void CPythonSystem::SetSnowModeOption(int iOpt)
{
	m_Config.bSnowMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowModeOption()
{
	return m_Config.bSnowMode;
}

void CPythonSystem::SetSnowTextureModeOption(int iOpt)
{
	m_Config.bSnowTextureMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowTextureModeOption()
{
	return m_Config.bSnowTextureMode;
}

# ifdef ENABLE_ENVIRONMENT_RAIN
void CPythonSystem::SetRainModeOption(int iOpt)
{
	m_Config.bRainMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetRainModeOption()
{
	return m_Config.bRainMode;
}
# endif
#endif

#ifdef ENABLE_DICE_SYSTEM
void CPythonSystem::SetDiceChatShow(int iFlag)
{
	m_Config.bDiceFlag = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsDiceChatShow()
{
	return m_Config.bDiceFlag;
}
#endif

int CPythonSystem::IsSaveID() const
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID() const
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling() const
{
	return m_Config.is_object_culling;
}

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;

	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(const TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	m_Config = {};

	m_Config.width = 1024;
	m_Config.height = 768;
	m_Config.bpp = 32;

#if defined(LOCALE_SERVICE_WE_JAPAN)
	m_Config.bWindowed = true;
#else
	m_Config.bWindowed = false;
#endif

	m_Config.is_software_cursor = false;
	m_Config.is_object_culling = true;
	m_Config.iDistance = 3;

	m_Config.gamma = 3;
	m_Config.music_volume = 1.0f;
	m_Config.voice_volume = 5;

	m_Config.bDecompressDDS = false;
	m_Config.bSoftwareTiling = 0;
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	m_Config.iShadowTargetLevel = CPythonBackground::SHADOW_NONE;
	m_Config.iShadowQualityLevel = CPythonBackground::SHADOW_BAD;
#else
	m_Config.iShadowLevel = 3;
#endif
	m_Config.bViewChat = true;
	m_Config.bAlwaysShowName = DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage = true;
	m_Config.bShowSalesText = true;
	m_Config.bFogMode = false;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	m_Config.bPrivateShopTextTail = 0;
#endif
#ifdef WJ_SHOW_MOB_INFO
	m_Config.bShowMobLevel = true;
	m_Config.bShowMobAIFlag = true;
#endif
#ifdef ENABLE_STRUCTURE_VIEW_MODE
	m_Config.bShowDecoMode = true;
#endif
	m_Config.bItemHighlight = true;
#ifdef ENABLE_EXTENDED_CONFIGS
	m_Config.bShowMoneyLog = true;
	m_Config.bCollectEquipment = true;
#endif
#ifdef ENABLE_REFINE_RENEWAL
	m_Config.bRefineStatus	= true;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	m_Config.bNightMode = true;
	m_Config.bSnowMode = true;
	m_Config.bSnowTextureMode = true;
# ifdef ENABLE_ENVIRONMENT_RAIN
	m_Config.bRainMode = true;
# endif
#endif
#ifdef ENABLE_DICE_SYSTEM
	m_Config.bDiceFlag = true;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	m_Config.bShowPlayerLanguage = true;
	m_Config.bShowChatLanguage = true;
	m_Config.bShowWhisperLanguage = true;
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	m_Config.bShowEquipmentView = true;
#endif
}

bool CPythonSystem::IsWindowed() const
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat() const
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName() const
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowDamage() const
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText() const
{
	return m_Config.bShowSalesText;
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
bool CPythonSystem::GetShopTextTailLevel()
{
	return m_Config.bPrivateShopTextTail;
}

void CPythonSystem::SetShopTextTailLevel(int iFlag)
{
	m_Config.bPrivateShopTextTail = iFlag;
}
#endif

bool CPythonSystem::IsUsingItemHighlight()
{
	return m_Config.bItemHighlight;
}

void CPythonSystem::SetUsingItemHighlight(int iFlag)
{
	m_Config.bItemHighlight = iFlag == 1 ? true : false;
}

#ifdef ENABLE_EXTENDED_CONFIGS
bool CPythonSystem::IsShowMoneyLog()
{
	return m_Config.bShowMoneyLog;
}

void CPythonSystem::SetShowMoneyLog(int iFlag)
{
	m_Config.bShowMoneyLog = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_REFINE_RENEWAL
bool CPythonSystem::IsRefineStatusShow()
{
	return m_Config.bRefineStatus;
}

void CPythonSystem::SetRefineStatus(int iFlag)
{
	m_Config.bRefineStatus = iFlag == 1 ? true : false;
}
#endif

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

void CPythonSystem::SetFogMode(int iFlag)
{
	m_Config.bFogMode = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsFogMode()
{
	return m_Config.bFogMode;
}

#ifdef WJ_SHOW_MOB_INFO
void CPythonSystem::SetShowMobLevel(int iOpt)
{
	m_Config.bShowMobLevel = iOpt == 1 ? true : false;
}

bool CPythonSystem::IsShowMobLevel() const
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobAIFlag(int iOpt)
{
	m_Config.bShowMobAIFlag = iOpt == 1 ? true : false;
}

bool CPythonSystem::IsShowMobAIFlag() const
{
	return m_Config.bShowMobAIFlag;
}
#endif

#ifdef ENABLE_STRUCTURE_VIEW_MODE
bool CPythonSystem::GetStructureViewMode()
{
	return m_Config.bShowDecoMode;
}

void CPythonSystem::SetStructureViewMode(int iFlag)
{
	m_Config.bShowDecoMode = iFlag == 1 ? true : false;
}
#endif

bool CPythonSystem::IsAutoTiling() const
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling = 1;
	else
		m_Config.bSoftwareTiling = 2;
}

bool CPythonSystem::IsSoftwareTiling() const
{
	if (m_Config.bSoftwareTiling == 1)
		return true;

	return false;
}

bool CPythonSystem::IsUseDefaultIME() const
{
	return m_Config.bUseDefaultIME;
}

bool CPythonSystem::LoadConfig()
{
	msl::file_ptr fPtr("metin2.cfg", "rt");

	if (!fPtr)
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fPtr.get()))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width = atoi(value);
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height = atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp = atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME"))
		{
			if (strchr(value, '.') == nullptr) // Old compatiability
			{
				m_Config.music_volume = pow(10.0f, (-1.0f + ((static_cast<float>(atoi(value))) / 5.0f)));
				if (atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			}
			else
				m_Config.music_volume = atof(value);
		}
		else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = static_cast<char>(atoi(value));
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "PRE_LOADING_DELAY_TIME"))
			g_iLoadingDelayTime = atoi(value);
		else if (!stricmp(command, "WINDOWED"))
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
		else if (!stricmp(command, "SHADOW_TARGET_LEVEL"))
			m_Config.iShadowTargetLevel = atoi(value);
		else if (!stricmp(command, "SHADOW_QUALITY_LEVEL"))
			m_Config.iShadowQualityLevel = atoi(value);
#else
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
#endif
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "FOG_MODE_ON"))
			m_Config.bFogMode = atoi(value) == 1 ? true : false;
#ifdef WJ_SHOW_MOB_INFO
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_STRUCTURE_VIEW_MODE
		else if (!stricmp(command, "SHOW_DECOMODE"))
			m_Config.bShowDecoMode = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		else if (!stricmp(command, "SHOW_PREM_SALESTEXT"))
			m_Config.bPrivateShopTextTail = atoi(value);
#endif
		else if (!stricmp(command, "ITEM_HIGHLIGHT"))
			m_Config.bItemHighlight = atoi(value) == 1 ? true : false;
#ifdef ENABLE_EXTENDED_CONFIGS
		else if (!stricmp(command, "SHOW_MONEY_LOG"))
			m_Config.bShowMoneyLog = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "COLLECT_EQUIPMENT"))
			m_Config.bCollectEquipment = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_REFINE_RENEWAL
		else if (!stricmp(command, "SHOW_REFINE_DIALOG"))
			m_Config.bRefineStatus = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		else if (!stricmp(command, "NIGHT_MODE_ON"))
			m_Config.bNightMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_MODE_ON"))
			m_Config.bSnowMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_TEXTURE_MODE"))
			m_Config.bSnowTextureMode = atoi(value) == 1 ? true : false;
# ifdef ENABLE_ENVIRONMENT_RAIN
		else if (!stricmp(command, "RAIN_MODE_ON"))
			m_Config.bRainMode = atoi(value) == 1 ? true : false;
# endif
#endif
#ifdef ENABLE_DICE_SYSTEM
		else if (!stricmp(command, "DICE"))
			m_Config.bDiceFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		else if (!stricmp(command, "SHOW_PLAYER_LANGUAGE"))
			m_Config.bShowPlayerLanguage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_CHAT_LANGUAGE"))
			m_Config.bShowChatLanguage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_WHISPER_LANGUAGE"))
			m_Config.bShowWhisperLanguage = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		else if (!stricmp(command, "SHOW_EQUIPMENT_VIEW"))
			m_Config.bShowEquipmentView = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
		else if (!stricmp(command, "EFFECT_LEVEL"))
		{
			CPythonGraphicOnOff::Instance().SetEffectOnOffLevel(atoi(value));
		}
		else if (!stricmp(command, "PRIVATE_SHOP_LEVEL"))
		{
			CPythonGraphicOnOff::Instance().SetPrivateShopOnOffLevel(atoi(value));
		}
		else if (!stricmp(command, "DROP_ITEM_LEVEL"))
		{
			CPythonGraphicOnOff::Instance().SetItemDropOnOffLevel(atoi(value));
		}
		else if (!stricmp(command, "PET_STATUS"))
		{
			CPythonGraphicOnOff::Instance().SetPetOnOffStatus(atoi(value));
		}
		else if (!stricmp(command, "NPC_STATUS"))
		{
			CPythonGraphicOnOff::Instance().SetNPCNameOnOffStatus(atoi(value));
		}
#endif
	}

	if (m_Config.bWindowed)
	{
		const unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		const unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (m_Config.width >= screen_width)
			m_Config.width = screen_width;
		if (m_Config.height >= screen_height)
			m_Config.height = screen_height;
	}

	m_OldConfig = m_Config;
	return true;
}

bool CPythonSystem::SaveConfig()
{
	msl::file_ptr fPtr("metin2.cfg", "wt");

	if (!fPtr)
		return false;

	fprintf(fPtr.get(),
			"WIDTH						%u\n"
			"HEIGHT						%u\n"
			"BPP						%u\n"
			"FREQUENCY					%u\n"
			"SOFTWARE_CURSOR			%d\n"
			"OBJECT_CULLING				%d\n"
			"VISIBILITY					%d\n"
			"MUSIC_VOLUME				%.3f\n"
			"VOICE_VOLUME				%d\n"
			"GAMMA						%d\n"
#ifdef ENABLE_EXTENDED_CONFIGS
			"SHOW_MONEY_LOG				%d\n"
#endif
			"IS_SAVE_ID					%d\n"
			"SAVE_ID					%s\n"
			"PRE_LOADING_DELAY_TIME		%d\n"
			"DECOMPRESSED_TEXTURE		%d\n",
			m_Config.width,
			m_Config.height,
			m_Config.bpp,
			m_Config.frequency,
			m_Config.is_software_cursor,
			m_Config.is_object_culling,
			m_Config.iDistance,
			m_Config.music_volume,
			m_Config.voice_volume,
			m_Config.gamma,
#ifdef ENABLE_EXTENDED_CONFIGS
			m_Config.bShowMoneyLog,
#endif
			m_Config.isSaveID,
			m_Config.SaveID,
			g_iLoadingDelayTime,
			m_Config.bDecompressDDS);

	if (m_Config.bWindowed == 1)
		fprintf(fPtr.get(), "WINDOWED				%d\n", m_Config.bWindowed);
	if (m_Config.bViewChat == 0)
		fprintf(fPtr.get(), "VIEW_CHAT				%d\n", m_Config.bViewChat);
	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fPtr.get(), "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);
	if (m_Config.bShowDamage == 0)
		fprintf(fPtr.get(), "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);
	if (m_Config.bShowSalesText == 0)
		fprintf(fPtr.get(), "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);
	if (m_Config.bFogMode == 0)
		fprintf(fPtr.get(), "FOG_MODE_ON				%d\n", m_Config.bFogMode);
#ifdef WJ_SHOW_MOB_INFO
	if (m_Config.bShowMobLevel == 0)
		fprintf(fPtr.get(), "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
	if (m_Config.bShowMobAIFlag == 0)
		fprintf(fPtr.get(), "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAIFlag);
#endif
#ifdef ENABLE_STRUCTURE_VIEW_MODE
	if (m_Config.bShowDecoMode == 0)
		fprintf(fPtr.get(), "SHOW_DECOMODE				%d\n", m_Config.bShowDecoMode);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	if (m_Config.bPrivateShopTextTail == 0)
		fprintf(fPtr.get(), "SHOW_PREM_SALESTEXT		%d\n", m_Config.bPrivateShopTextTail);
#endif
	fprintf(fPtr.get(), "ITEM_HIGHLIGHT				%d\n", m_Config.bItemHighlight);
#ifdef ENABLE_EXTENDED_CONFIGS
	fprintf(fPtr.get(), "COLLECT_EQUIPMENT			%d\n", m_Config.bCollectEquipment);
#endif
#ifdef ENABLE_REFINE_RENEWAL
	fprintf(fPtr.get(), "SHOW_REFINE_DIALOG			%d\n", m_Config.bRefineStatus);
#endif
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	fprintf(fPtr.get(), "SHADOW_TARGET_LEVEL		%d\n", m_Config.iShadowTargetLevel);
	fprintf(fPtr.get(), "SHADOW_QUALITY_LEVEL		%d\n", m_Config.iShadowQualityLevel);
#else
	fprintf(fPtr.get(), "SHADOW_LEVEL				%d\n", m_Config.iShadowLevel);
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	fprintf(fPtr.get(), "NIGHT_MODE_ON				%d\n", m_Config.bNightMode);
	fprintf(fPtr.get(), "SNOW_MODE_ON				%d\n", m_Config.bSnowMode);
	fprintf(fPtr.get(), "SNOW_TEXTURE_MODE			%d\n", m_Config.bSnowTextureMode);
# ifdef ENABLE_ENVIRONMENT_RAIN
	fprintf(fPtr.get(), "RAIN_MODE_ON				%d\n", m_Config.bRainMode);
# endif
#endif
#ifdef ENABLE_DICE_SYSTEM
	fprintf(fPtr.get(), "DICE						%d\n", m_Config.bDiceFlag);
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	fprintf(fPtr.get(), "SHOW_PLAYER_LANGUAGE			%d\n", m_Config.bShowPlayerLanguage);
	fprintf(fPtr.get(), "SHOW_CHAT_LANGUAGE				%d\n", m_Config.bShowChatLanguage);
	fprintf(fPtr.get(), "SHOW_WHISPER_LANGUAGE			%d\n", m_Config.bShowWhisperLanguage);
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	fprintf(fPtr.get(), "SHOW_EQUIPMENT_VIEW			%d\n", m_Config.bShowEquipmentView);
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
	fprintf(fPtr.get(), "EFFECT_LEVEL				%d\n", CPythonGraphicOnOff::Instance().GetEffectOnOffLevel());
	fprintf(fPtr.get(), "PRIVATE_SHOP_LEVEL			%d\n", CPythonGraphicOnOff::Instance().GetPrivateShopOnOffLevel());
	fprintf(fPtr.get(), "DROP_ITEM_LEVEL				%d\n", CPythonGraphicOnOff::Instance().GetItemDropOnOffLevel());
	fprintf(fPtr.get(), "PET_STATUS					%d\n", CPythonGraphicOnOff::Instance().GetPetOnOffStatus());
	fprintf(fPtr.get(), "NPC_NAME_STATUS				%d\n", CPythonGraphicOnOff::Instance().GetNPCNameOnOffStatus());
#endif

	fprintf(fPtr.get(), "USE_DEFAULT_IME		%d\n", m_Config.bUseDefaultIME);
	fprintf(fPtr.get(), "SOFTWARE_TILING		%d\n", m_Config.bSoftwareTiling);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	fprintf(fPtr.get(), "SHADOW_TARGET_LEVEL		%d\n", m_Config.iShadowTargetLevel);
	fprintf(fPtr.get(), "SHADOW_QUALITY_LEVEL		%d\n", m_Config.iShadowQualityLevel);
#else
	fprintf(fPtr.get(), "SHADOW_LEVEL			%d\n", m_Config.iShadowLevel);
#endif
	fprintf(fPtr.get(), "\n");
	return true;
}

bool CPythonSystem::LoadInterfaceStatus()
{
	msl::file_ptr fPtr("interface.cfg", "rb");

	if (!fPtr.get())
		return false;

	fread(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, fPtr.get());
	return true;
}

void CPythonSystem::SaveInterfaceStatus() const
{
	if (!m_poInterfaceHandler)
		return;

	PyCallClassMemberFunc(m_poInterfaceHandler, "OnSaveInterfaceStatus", Py_BuildValue("()"));

	msl::file_ptr fPtr("interface.cfg", "wb");
	if (!fPtr.get())
	{
		TraceError("Cannot open interface.cfg");
		return;
	}

	fwrite(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, fPtr.get());
}

bool CPythonSystem::isInterfaceConfig() const
{
	return m_isInterfaceConfig;
}

const CPythonSystem::TWindowStatus & CPythonSystem::GetWindowStatusReference(int iIndex) const
{
	return m_WindowStatus[iIndex];
}

void CPythonSystem::ApplyConfig() // 이전 설정과 현재 설정을 비교해서 바뀐 설정을 적용 한다.
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;

		switch (m_Config.gamma)
		{
		case 0:
			val = 0.4f;
			break;
		case 1:
			val = 0.7f;
			break;
		case 2:
			val = 1.0f;
			break;
		case 3:
			val = 1.2f;
			break;
		case 4:
			val = 1.4f;
			break;
		}

		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}

	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem() const
{
	CSoundManager & rkSndMgr = CSoundManager::Instance();
	rkSndMgr.SetMusicVolume(m_Config.music_volume);
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	CPythonBackground& rkBG = CPythonBackground::Instance();
	rkBG.SetNightModeOption(m_Config.bNightMode);
	rkBG.SetSnowModeOption(m_Config.bSnowMode, true);
	rkBG.SetSnowTextureModeOption(m_Config.bSnowTextureMode);
# ifdef ENABLE_ENVIRONMENT_RAIN
	rkBG.SetRainModeOption(m_Config.bRainMode, true);
# endif
#endif
}

void CPythonSystem::Clear()
{
	SetInterfaceHandler(nullptr);
}

CPythonSystem::CPythonSystem()
{
	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();

	if (LoadInterfaceStatus())
		m_isInterfaceConfig = true;
	else
		m_isInterfaceConfig = false;
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler == nullptr && "CPythonSystem MUST CLEAR!");
}

#ifdef ENABLE_HWID_BAN
#include <iomanip>
#include "picosha2.h"
#include "smbios.cpp"
TCHAR* registry_read(LPCTSTR subkey, LPCTSTR name, unsigned long type)
{
	HKEY key;
	TCHAR value[255];
	unsigned long value_length = 255;
	RegOpenKey(HKEY_LOCAL_MACHINE, subkey, &key);
	RegQueryValueEx(key, name, nullptr, &type, (LPBYTE)&value, &value_length);
	RegCloseKey(key);
	return value;
}

uint32_t CPythonSystem::getVolumeHash() {
	CHAR windowsDirectory[MAX_PATH];
	CHAR volumeName[8] = { 0 };
	unsigned long serialNum = 0;

	if (!GetWindowsDirectory(windowsDirectory, sizeof(windowsDirectory)))
		windowsDirectory[0] = L'C';

	volumeName[0] = windowsDirectory[0];
	volumeName[1] = ':';
	volumeName[2] = '\\';
	volumeName[3] = '\0';

	GetVolumeInformation(volumeName, nullptr, 0, &serialNum, 0, nullptr, nullptr, 0);

	return serialNum;
}

const char* CPythonSystem::getCpuInfos() {
	SYSTEM_INFO kSystemInfo;
	GetSystemInfo(&kSystemInfo);

	std::string stTemp;
	char szNum[15 + 1];
#define AddNumber(num) _itoa_s(num, szNum, 10), stTemp += szNum
	AddNumber(kSystemInfo.wProcessorArchitecture);
	AddNumber(kSystemInfo.dwNumberOfProcessors);
	AddNumber(kSystemInfo.dwProcessorType);
	AddNumber(kSystemInfo.wProcessorLevel);
	AddNumber(kSystemInfo.wProcessorRevision);
	AddNumber(IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_CHANNELS_ENABLED));
	AddNumber(IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE));
	AddNumber(IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED));
	AddNumber(IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_PAE_ENABLED));
	AddNumber(IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE));
	AddNumber(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE));
#undef AddNumber

	return stTemp.c_str();
}

const char* CPythonSystem::getMachineName() {
	static char computerName[1024];
	unsigned long size = 1024;
	GetComputerName(computerName, &size);
	return &(computerName[0]);
}

const char* CPythonSystem::getBiosDate()
{
	static char buf[1024];
	strncpy(buf, registry_read("HARDWARE\\DESCRIPTION\\System\\BIOS", "BIOSReleaseDate", REG_SZ), sizeof(buf));
	return buf;
}

const char* CPythonSystem::getMainboardName()
{
	static char buf[1024];
	strncpy(buf, registry_read("HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardProduct", REG_SZ), sizeof(buf));
	return buf;
}

const char* CPythonSystem::getGPUName()
{
	static char buf[1024];
	IDirect3D9* d8object = Direct3DCreate9(D3D_SDK_VERSION);
	const unsigned int adaptercount = d8object->GetAdapterCount();
	D3DADAPTER_IDENTIFIER9* adapters = new D3DADAPTER_IDENTIFIER9[sizeof(adaptercount)];

	for (unsigned int i = 0; i < adaptercount; i++) {
		d8object->GetAdapterIdentifier(i, 0, &(adapters[i]));
	}

	strncpy(buf, adapters->Description, sizeof(buf));
	return buf;
}

const char* CPythonSystem::GetHWID()
{
	static const unsigned long TargetLength = 64;

	std::stringstream stream;

#ifdef HDD_COMPONENT
	stream << getVolumeHash();
#endif
#ifdef CPU_COMPONENT
	stream << getCpuInfos();
#endif
#ifdef PC_NAME_COMPONENT
	stream << getMachineName();
#endif
#ifdef BIOS_DATE_COMPONENT
	stream << getBiosDate();
#endif
#ifdef MAINBOARD_NAME_COMPONENT
	stream << getMainboardName();
#endif
#ifdef GPU_NAME_COMPONENT
	stream << getGPUName();
#endif

	auto string = stream.str();

	while (string.size() < TargetLength) {
		string = string + string;
	}

	if (string.size() > TargetLength) {
		string = string.substr(0, TargetLength);
	}

	return generateHash(string);
}

const char* CPythonSystem::generateHash(const std::string& bytes) {
	static char s_szHWIDBuffer[HWID_MAX_LEN];
	static char chars[] = "0123456789ABCDEF";
	std::stringstream stream;

	const auto size = bytes.size();
	for (unsigned long i = 0; i < size; ++i)
	{
		const unsigned char ch = ~((unsigned char)((int16_t)bytes[i] +
			(int16_t)bytes[(i + 1) % size] +
			(int16_t)bytes[(i + 2) % size] +
			(int16_t)bytes[(i + 3) % size])) * (i + 1);

		stream << chars[(ch >> 4) & 0x0F] << chars[ch & 0x0F];
	}

	strncpy(s_szHWIDBuffer, stream.str().c_str(), sizeof(s_szHWIDBuffer));

	return s_szHWIDBuffer;
}
#endif
