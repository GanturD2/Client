#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
public:
	enum EWindow
	{
		WINDOW_STATUS,
		WINDOW_INVENTORY,
		WINDOW_ABILITY,
		WINDOW_SOCIETY,
		WINDOW_JOURNAL,
		WINDOW_COMMAND,

		WINDOW_QUICK,
		WINDOW_GAUGE,
		WINDOW_MINIMAP,
		WINDOW_CHAT,

		WINDOW_MAX_NUM
	};

	enum
	{
		FREQUENCY_MAX_NUM = 30,
		RESOLUTION_MAX_NUM = 100
	};

	typedef struct SResolution
	{
		uint32_t width;
		uint32_t height;
		uint32_t bpp; // bits per pixel (high-color = 16bpp, true-color = 32bpp)

		uint32_t frequency[20];
		uint8_t frequency_count;
	} TResolution;

	typedef struct SWindowStatus
	{
		int isVisible;
		int isMinimized;

		int ixPosition;
		int iyPosition;
		int iHeight;
	} TWindowStatus;

	typedef struct SConfig
	{
		uint32_t width;
		uint32_t height;
		uint32_t bpp;
		uint32_t frequency;

		bool is_software_cursor;
		bool is_object_culling;
		int iDistance;
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
		int				iShadowTargetLevel;
		int				iShadowQualityLevel;
#else
		int				iShadowLevel;
#endif

		FLOAT music_volume;
		uint8_t voice_volume;

		int gamma;

		int isSaveID;
		char SaveID[20];

		bool bWindowed;
		bool bDecompressDDS;
		bool bNoSoundCard;
		bool bUseDefaultIME;
		uint8_t bSoftwareTiling;
		bool bViewChat;
		bool bAlwaysShowName;
		bool bShowDamage;
		bool bShowSalesText;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		uint8_t			bPrivateShopTextTail;
#endif
		bool			bFogMode;
		bool			bItemHighlight;
#ifdef ENABLE_EXTENDED_CONFIGS
		bool			bShowMoneyLog;
		bool			bCollectEquipment;
#endif
#ifdef ENABLE_REFINE_RENEWAL
		bool			bRefineStatus;
		bool			bRefineStatusFlag;
#endif
#ifdef WJ_SHOW_MOB_INFO
		bool bShowMobLevel;
		bool bShowMobAIFlag;
#endif
#ifdef ENABLE_STRUCTURE_VIEW_MODE
		bool			bShowDecoMode;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		bool			bNightMode;
		bool			bSnowMode;
		bool			bSnowTextureMode;
# ifdef ENABLE_ENVIRONMENT_RAIN
		bool			bRainMode;
# endif
#endif
#ifdef ENABLE_DICE_SYSTEM
		bool			bDiceFlag;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		bool			bShowPlayerLanguage;
		bool			bShowChatLanguage;
		bool			bShowWhisperLanguage;
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		bool			bShowEquipmentView;
#endif
	} TConfig;

public:
	CPythonSystem();
	~CPythonSystem();
	CLASS_DELETE_COPYMOVE(CPythonSystem);

	void Clear();
	void SetInterfaceHandler(PyObject * poHandler);
	void DestroyInterfaceHandler();

	// Config
	void SetDefaultConfig();
	bool LoadConfig();
	bool SaveConfig();
	void ApplyConfig();
	void SetConfig(const TConfig * pNewConfig);
	TConfig * GetConfig();
	void ChangeSystem() const;

	// Interface
	bool LoadInterfaceStatus();
	void SaveInterfaceStatus() const;
	bool isInterfaceConfig() const;
	const TWindowStatus & GetWindowStatusReference(int iIndex) const;

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	uint32_t GetBPP() const;
	uint32_t GetFrequency() const;
	bool IsSoftwareCursor() const;
	bool IsWindowed() const;
	bool IsViewChat() const;
	bool IsAlwaysShowName() const;
	bool IsShowDamage() const;
	bool IsShowSalesText() const;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	bool							GetShopTextTailLevel();
#endif
	bool IsUseDefaultIME() const;
	bool IsNoSoundCard() const;
	bool IsAutoTiling() const;
	bool IsSoftwareTiling() const;
#ifdef ENABLE_EXTENDED_CONFIGS
	bool							IsShowMoneyLog();
	bool							IsCollectEquipment()								{ return m_Config.bCollectEquipment; }
#endif
	void SetSoftwareTiling(bool isEnable);
	void SetViewChatFlag(int iFlag);
	void SetAlwaysShowNameFlag(int iFlag);
	void SetShowDamageFlag(int iFlag);
	void SetShowSalesTextFlag(int iFlag);
	void							SetFogMode(int iFlag);
	bool							IsFogMode();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
	void							SetShopTextTailLevel(int iFlag);
#endif
	bool							IsUsingItemHighlight();
	void							SetUsingItemHighlight(int iFlag);
#ifdef ENABLE_EXTENDED_CONFIGS
	void							SetShowMoneyLog(int iFlag);
	void							SetCollectEquipment(bool iFlag)						{ m_Config.bCollectEquipment = iFlag; }
#endif
#ifdef ENABLE_REFINE_RENEWAL
	bool							IsRefineStatusShow();
	void							SetRefineStatus(int iFlag);
#endif
#ifdef WJ_SHOW_MOB_INFO
	void SetShowMobAIFlag(int iFlag);
	bool IsShowMobAIFlag() const;
	void SetShowMobLevel(int iFlag);
	bool IsShowMobLevel() const;
#endif
#ifdef ENABLE_STRUCTURE_VIEW_MODE
	bool							GetStructureViewMode();
	void							SetStructureViewMode(int iFlag);
#endif

	// Window
	void SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight);

	// SaveID
	int IsSaveID() const;
	const char * GetSaveID() const;
	void SetSaveID(int iValue, const char * c_szSaveID);

	/// Display
	void GetDisplaySettings();

	int GetResolutionCount() const;
	int GetFrequencyCount(int index) const;
	bool GetResolution(int index, OUT uint32_t * width, OUT uint32_t * height, OUT uint32_t * bpp) const;
	bool GetFrequency(int index, int freq_index, OUT uint32_t * frequncy);
	int GetResolutionIndex(uint32_t width, uint32_t height, uint32_t bit) const;
	int GetFrequencyIndex(int res_index, uint32_t frequency);
	bool isViewCulling() const;

	// Sound
	float GetMusicVolume() const;
	int GetSoundVolume() const;
	void SetMusicVolume(float fVolume);
	void SetSoundVolumef(float fVolume);

	int GetDistance() const;
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	int								GetShadowTargetLevel();
	void							SetShadowTargetLevel(unsigned int level);
	int								GetShadowQualityLevel();
	void							SetShadowQualityLevel(unsigned int level);
#else
	int GetShadowLevel() const;
	void SetShadowLevel(unsigned int level);
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	void							SetNightModeOption(int iOpt);
	bool							GetNightModeOption();
	void							SetSnowModeOption(int iOpt);
	bool							GetSnowModeOption();
	void							SetSnowTextureModeOption(int iOpt);
	bool							GetSnowTextureModeOption();
# ifdef ENABLE_ENVIRONMENT_RAIN
	void							SetRainModeOption(int iOpt);
	bool							GetRainModeOption();
# endif
#endif

#ifdef ENABLE_DICE_SYSTEM
	void							SetDiceChatShow(int iFlag);
	bool							IsDiceChatShow();
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void SetShowPlayerLanguage(int iFlag) { m_Config.bShowPlayerLanguage = iFlag == 1 ? true : false; };
	bool IsShowPlayerLanguage() { return m_Config.bShowPlayerLanguage; };

	void SetShowChatLanguage(int iFlag) { m_Config.bShowChatLanguage = iFlag == 1 ? true : false; };
	bool IsShowChatLanguage() { return m_Config.bShowChatLanguage; };

	void SetShowWhisperLanguage(int iFlag) { m_Config.bShowWhisperLanguage = iFlag == 1 ? true : false; };
	bool IsShowWhisperLanguage() { return m_Config.bShowWhisperLanguage; };
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	void SetEquipmentView(int iFlag) { m_Config.bShowEquipmentView = iFlag == 1 ? true : false; };
	bool GetEquipmentView() { return m_Config.bShowEquipmentView; };
#endif

#ifdef ENABLE_HWID_BAN
	uint32_t getVolumeHash();
	const char* getCpuInfos();
	const char* getMachineName();
	const char* getBiosDate();
	const char* getMainboardName();
	const char* getGPUName();
	const char* GetHWID();
	const char* generateHash(const std::string& bytes);
#endif

protected:
	TResolution m_ResolutionList[RESOLUTION_MAX_NUM]{};
	int m_ResolutionCount{};

	TConfig m_Config{};
	TConfig m_OldConfig{};

	bool m_isInterfaceConfig{};
	PyObject * m_poInterfaceHandler{nullptr};
	TWindowStatus m_WindowStatus[WINDOW_MAX_NUM]{};
};
