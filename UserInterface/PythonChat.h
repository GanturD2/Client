#pragma once

#include "Packet.h"
#include "AbstractChat.h"

class CWhisper
{
public:
	typedef struct SChatLine
	{
		CGraphicTextInstance Instance;

		SChatLine() = default;
		~SChatLine() { Instance.Destroy(); }
		CLASS_DELETE_COPYMOVE(SChatLine);

		static void DestroySystem();

		static SChatLine * New();
		static void Delete(SChatLine * pkChatLine);

		static CDynamicPool<SChatLine> ms_kPool;
	} TChatLine;

	using TChatLineDeque = std::deque<TChatLine *>;
	using TChatLineList = std::list<TChatLine *>;

public:
	CWhisper();
	~CWhisper();
	CLASS_DELETE_COPYMOVE(CWhisper);

	void Destroy();

	void SetPosition(float fPosition);
	void SetBoxSize(float fWidth, float fHeight);
	void AppendChat(int iType, const char * c_szChat);
	void Render(float fx, float fy);

protected:
	void __Initialize();
	void __ArrangeChat();

protected:
	float m_fLineStep;
	float m_fWidth;
	float m_fHeight;
	float m_fcurPosition;

	TChatLineDeque m_ChatLineDeque;
	TChatLineList m_ShowingChatLineList;

public:
	static void DestroySystem();

	static CWhisper * New();
	static void Delete(CWhisper * pkWhisper);

	static CDynamicPool<CWhisper> ms_kPool;
};

class CPythonChat : public CSingleton<CPythonChat>, public IAbstractChat
{
public:
	enum EWhisperType
	{
		WHISPER_TYPE_CHAT = 0,
		WHISPER_TYPE_NOT_EXIST = 1,
		WHISPER_TYPE_TARGET_BLOCKED = 2,
		WHISPER_TYPE_SENDER_BLOCKED = 3,
		WHISPER_TYPE_ERROR = 4,
		WHISPER_TYPE_GM = 5,
		WHISPER_TYPE_SYSTEM = 0xFF
	};

	enum EHyperLink
	{
		HYPER_LINK_ITEM_KEYWORD,
		HYPER_LINK_ITEM_VNUM,
#ifdef ENABLE_SET_ITEM
		HYPER_LINK_ITEM_PRE_SET_VALUE,
#endif
		HYPER_LINK_ITEM_FLAGS,
		HYPER_LINK_ITEM_SOCKET0,
		HYPER_LINK_ITEM_SOCKET1,
		HYPER_LINK_ITEM_SOCKET2,
#ifdef ENABLE_PROTO_RENEWAL
		HYPER_LINK_ITEM_SOCKET3,
		HYPER_LINK_ITEM_SOCKET4,
		HYPER_LINK_ITEM_SOCKET5,
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		HYPER_LINK_ITEM_CHANGE_ITEM_VNUM,
#endif
#ifdef ENABLE_REFINE_ELEMENT
		HYPER_LINK_ITEM_GRADE_VALUE, // NOT OFFICIAL
		HYPER_LINK_ITEM_REFINE_ELEMENT_APPLY_TYPE,
		HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE0,
		HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE1,
		HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE2,
		HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE0,
		HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE1,
		HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE2,
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0,
		HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0,
		HYPER_LINK_ITEM_APPLY_RANDOM_TYPE1,
		HYPER_LINK_ITEM_APPLY_RANDOM_VALUE1,
		HYPER_LINK_ITEM_APPLY_RANDOM_TYPE2,
		HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2,
		//HYPER_LINK_ITEM_APPLY_RANDOM_TYPE3,
		//HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3,
#endif
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE0,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE0,
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE1,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE1,
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE2,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE2,
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE3,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE3,
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE4,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE4,
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE5,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE5,
		HYPER_LINK_ITEM_ATTRIBUTE_TYPE6,
		HYPER_LINK_ITEM_ATTRIBUTE_VALUE6,

#ifdef ENABLE_YOHARA_SYSTEM
		HYPER_LINK_ITEM_RANDOM_VALUE0,
		HYPER_LINK_ITEM_RANDOM_VALUE1,
		HYPER_LINK_ITEM_RANDOM_VALUE2,
		HYPER_LINK_ITEM_RANDOM_VALUE3,
#endif

		HYPER_LINK_ITEM_MAX,
	};

#ifdef ENABLE_GROWTH_PET_SYSTEM
	enum EGrowthPetHyperLink
	{
		HYPER_LINK_ITEM_PET_KEYWORD,
		HYPER_LINK_ITEM_PET_VNUM,
		HYPER_LINK_ITEM_PET_PRE_SET_VALUE,
		HYPER_LINK_ITEM_PET_FLAGS,
		HYPER_LINK_ITEM_PET_SOCKET0,
		HYPER_LINK_ITEM_PET_SOCKET1,
		HYPER_LINK_ITEM_PET_SOCKET2,
		HYPER_LINK_ITEM_PET_SOCKET3,
		HYPER_LINK_ITEM_PET_SOCKET4,
		HYPER_LINK_ITEM_PET_SOCKET5,
		HYPER_LINK_ITEM_PET_LEVEL,
		HYPER_LINK_ITEM_PET_AGE,
		HYPER_LINK_ITEM_PET_EVOL_LEVEL,
		HYPER_LINK_ITEM_PET_HP,
		HYPER_LINK_ITEM_PET_DEF,
		HYPER_LINK_ITEM_PET_SP,
		HYPER_LINK_ITEM_PET_LIFE,
		HYPER_LINK_ITEM_PET_LIFE_MAX,
		HYPER_LINK_ITEM_PET_SKILL_COUNT,
		HYPER_LINK_ITEM_PET_BAG_PET_VNUM,
		HYPER_LINK_ITEM_PET_NICKNAME,
		HYPER_LINK_ITEM_PET_SKILL0_VNUM,
		HYPER_LINK_ITEM_PET_SKILL0_LEVEL,
		HYPER_LINK_ITEM_PET_SKILL1_VNUM,
		HYPER_LINK_ITEM_PET_SKILL1_LEVEL,
		HYPER_LINK_ITEM_PET_SKILL2_VNUM,
		HYPER_LINK_ITEM_PET_SKILL2_LEVEL,
		HYPER_LINK_ITEM_PET_MAX,
	};
#endif

	enum EBoardState
	{
		BOARD_STATE_VIEW,
		BOARD_STATE_EDIT,
		BOARD_STATE_LOG
	};

	enum
	{
		CHAT_LINE_MAX_NUM = 300,
		CHAT_LINE_COLOR_ARRAY_MAX_NUM = 3
	};

	typedef struct SChatLine
	{
		int iType;
		float fAppendedTime;
		D3DXCOLOR aColor[CHAT_LINE_COLOR_ARRAY_MAX_NUM];
		CGraphicTextInstance Instance;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CGraphicImageInstance* pLanguageImageInstance;
#endif
#ifdef ENABLE_EMPIRE_FLAG
		CGraphicImageInstance* pEmpireImageInstance;
#endif

		SChatLine();
		virtual ~SChatLine();
		CLASS_DELETE_COPYMOVE(SChatLine);

#ifdef ENABLE_CHAT_SETTINGS
		std::string text;
# ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		uint8_t countryIndex;
# endif
# ifdef ENABLE_EMPIRE_FLAG
		uint8_t empireIndex;
# endif
#endif

		void SetColor(uint32_t dwID, uint32_t dwColor);
		void SetColorAll(uint32_t dwColor);
		D3DXCOLOR & GetColorRef(uint32_t dwID);
		static void DestroySystem();

		static SChatLine * New();
		static void Delete(SChatLine * pkChatLine);

		static CDynamicPool<SChatLine> ms_kPool;
	} TChatLine;

	typedef struct SWaitChat
	{
		int iType;
		std::string strChat;

		uint32_t dwAppendingTime;
	} TWaitChat;

	using TChatLineDeque = std::deque<TChatLine *>;
	using TChatLineList = std::list<TChatLine *>;

	typedef std::map<std::string, CWhisper *> TWhisperMap;
	using TIgnoreCharacterSet = std::set<std::string>;
	using TWaitChatList = std::list<TWaitChat>;

	typedef struct SChatSet
	{
		int m_ix;
		int m_iy;
		int m_iHeight;
		int m_iStep;
		float m_fEndPos;

		int m_iBoardState;
		std::vector<int> m_iMode;

		TChatLineList m_ShowingChatLineList;

		bool CheckMode(uint32_t dwType)
		{
			if (dwType >= m_iMode.size())
				return false;

			return m_iMode[dwType] ? true : false;
		}

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
		std::vector<int> m_iCountryMode;
		bool CheckCountryMode(uint32_t dwType)
		{
			if (dwType >= m_iCountryMode.size())
				return false;
			return m_iCountryMode[dwType] ? true : false;
		}
#endif

		SChatSet()
		{
			m_iBoardState = BOARD_STATE_VIEW;

			m_ix = 0;
			m_iy = 0;
			m_fEndPos = 1.0f;
			m_iHeight = 0;
			m_iStep = 15;

			m_iMode.clear();
			m_iMode.resize(ms_iChatModeSize, 1);

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
			m_iCountryMode.clear();
			m_iCountryMode.resize(15, 1);
#endif
		}

		static int ms_iChatModeSize;
	} TChatSet;

	typedef std::map<int, TChatSet> TChatSetMap;

public:
	CPythonChat();
	~CPythonChat();
	CLASS_DELETE_COPYMOVE(CPythonChat);

	void SetChatColor(uint32_t eType, uint32_t r, uint32_t g, uint32_t b);

	void Destroy();
	void Close();

	int CreateChatSet(uint32_t dwID);
	void Update(uint32_t dwID);
	void Render(uint32_t dwID);
	void RenderWhisper(const char * c_szName, float fx, float fy);

	void SetBoardState(uint32_t dwID, int iState);
	void SetPosition(uint32_t dwID, int ix, int iy);
	void SetHeight(uint32_t dwID, int iHeight);
	void SetStep(uint32_t dwID, int iStep);
	void ToggleChatMode(uint32_t dwID, int iMode);
	void EnableChatMode(uint32_t dwID, int iMode);
	void DisableChatMode(uint32_t dwID, int iMode);
	void SetEndPos(uint32_t dwID, float fPos);

#ifdef ENABLE_CHAT_SETTINGS
	void DeleteChatSet(uint32_t dwID);
#endif
#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
	void EnableCountryMode(uint32_t dwID, int iMode);
	void DisableCountryMode(uint32_t dwID, int iMode);
#endif

	int GetVisibleLineCount(uint32_t dwID);
	int GetEditableLineCount(uint32_t dwID);
	int GetLineCount(uint32_t dwID);
	int GetLineStep(uint32_t dwID);

	// Chat
	void AppendChat(int iType, const char* c_szChat);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void AppendChat(int iType, uint8_t bLanguage,
#	ifdef ENABLE_EMPIRE_FLAG
		uint8_t bEmpire,
#	endif
		const char* c_szChat);
#endif

	void AppendChatWithDelay(int iType, const char * c_szChat, int iDelay);
	void ArrangeShowingChat(uint32_t dwID);

	// Ignore
	void IgnoreCharacter(const char * c_szName);
	BOOL IsIgnoreCharacter(const char * c_szName);

	// Whisper
	CWhisper * CreateWhisper(const char * c_szName);
	void AppendWhisper(int iType, const char * c_szName, const char * c_szChat);
	void ClearWhisper(const char * c_szName);
	BOOL GetWhisper(const char * c_szName, CWhisper ** ppWhisper);
	void InitWhisper(PyObject * ppyObject);

protected:
	void __Initialize();
	void __DestroyWhisperMap();

	TChatLineList * GetChatLineListPtr(uint32_t dwID);
	TChatSet * GetChatSetPtr(uint32_t dwID);

	void UpdateViewMode(uint32_t dwID);
	void UpdateEditMode(uint32_t dwID);
	void UpdateLogMode(uint32_t dwID);

	uint32_t GetChatColor(int iType) const;

protected:
	TChatLineDeque m_ChatLineDeque;
#ifdef ENABLE_CHAT_SETTINGS
	std::map<uint8_t, TChatLineDeque> m_ChatLineNewDeque;
#endif
	TChatLineList m_ShowingChatLineList;
	TChatSetMap m_ChatSetMap;
	TWhisperMap m_WhisperMap;
	TIgnoreCharacterSet m_IgnoreCharacterSet;
	TWaitChatList m_WaitChatList;

	D3DXCOLOR m_akD3DXClrChat[CHAT_TYPE_MAX_NUM];
};
