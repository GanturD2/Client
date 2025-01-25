#pragma once

#include "../EterLib/parser.h"

#include <string>
#include <map>

class CPythonEventManager : public CSingleton<CPythonEventManager>
{
public:
	typedef struct STextLine
	{
		int ixLocal, iyLocal;
		CGraphicTextInstance * pInstance;
	} TTextLine;

	using TScriptTextLineList = std::list<TTextLine>;
	typedef std::map<int, std::string> TEventAnswerMap;

	enum
	{
		EVENT_POSITION_START = 0,
		EVENT_POSITION_END = 1,

#ifdef NEW_SELECT_CHARACTER
		BOX_VISIBLE_LINE_COUNT = 15
#else
		BOX_VISIBLE_LINE_COUNT = 5
#endif
	};

	enum EButtonType
	{
		BUTTON_TYPE_NEXT,
		BUTTON_TYPE_DONE,
		BUTTON_TYPE_CANCEL
	};

	enum EEventType
	{
		EVENT_TYPE_LETTER,
		EVENT_TYPE_COLOR,
		EVENT_TYPE_DELAY,
		EVENT_TYPE_ENTER,
		EVENT_TYPE_WAIT,
		EVENT_TYPE_CLEAR,
		EVENT_TYPE_QUESTION,
		EVENT_TYPE_NEXT,
		EVENT_TYPE_DONE,

		EVENT_TYPE_LEFT_IMAGE,
		EVENT_TYPE_TOP_IMAGE,
		EVENT_TYPE_BACKGROUND_IMAGE,
		EVENT_TYPE_IMAGE,

		EVENT_TYPE_ADD_MAP_SIGNAL,
		EVENT_TYPE_CLEAR_MAP_SIGNAL,
		EVENT_TYPE_SET_MESSAGE_POSITION,
		EVENT_TYPE_ADJUST_MESSAGE_POSITION,
		EVENT_TYPE_SET_CENTER_MAP_POSITION,

		EVENT_TYPE_QUEST_BUTTON,

		// HIDE_QUEST_LETTER
		EVENT_TYPE_QUEST_BUTTON_CLOSE,
		// END_OF_HIDE_QUEST_LETTER

		EVENT_TYPE_SLEEP,
		EVENT_TYPE_SET_CAMERA,
		EVENT_TYPE_BLEND_CAMERA,
		EVENT_TYPE_RESTORE_CAMERA,
		EVENT_TYPE_FADE_OUT,
		EVENT_TYPE_FADE_IN,
		EVENT_TYPE_WHITE_OUT,
		EVENT_TYPE_WHITE_IN,
		EVENT_TYPE_CLEAR_TEXT,
		EVENT_TYPE_TEXT_HORIZONTAL_ALIGN_CENTER,
		EVENT_TYPE_TITLE_IMAGE,

		EVENT_TYPE_RUN_CINEMA,
		EVENT_TYPE_DUNGEON_RESULT,

		EVENT_TYPE_ITEM_NAME,
		EVENT_TYPE_MONSTER_NAME,

		EVENT_TYPE_COLOR256,
		EVENT_TYPE_WINDOW_SIZE,

		EVENT_TYPE_INPUT,
		EVENT_TYPE_CONFIRM_WAIT,
		EVENT_TYPE_END_CONFIRM_WAIT,

		EVENT_TYPE_INSERT_IMAGE,

		EVENT_TYPE_SELECT_ITEM,
		EVENT_TYPE_IMAGE_SHOWTOOLTIP_BY_CELL,
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
		EVENT_TYPE_SELECT_ITEM_EX,
#endif
#ifdef ENABLE_REVERSED_FUNCTIONS_EX
		EVENT_TYPE_CLOSE_CINEMA,
		EVENT_TYPE_SET_KEYBOARD_MOUSE_LOCK,
		EVENT_TYPE_RESTORE_CAMERA_EXCEPT_POSITION,
#endif
	};

private:
	std::map<std::string, int> EventTypeMap;

public:
	typedef struct SEventSet
	{
		int ix, iy;
		int iWidth;
		int iyLocal;

		// State
		bool isLock;

		long lLastDelayTime;

		int iCurrentLetter;

		D3DXCOLOR CurrentColor;
		std::string strCurrentLine;

		CGraphicTextInstance * pCurrentTextLine;
		TScriptTextLineList ScriptTextLineList;

		BOOL isConfirmWait;
		CGraphicTextInstance * pConfirmTimeTextLine;
		int iConfirmEndTime;

		// Group Data
		script::Group ScriptGroup;

		// Static Data
		char szFileName[32 + 1];

		int iVisibleStartLine;
		int iVisibleLineCount;

		int iAdjustLine;

		D3DXCOLOR DiffuseColor;
		long lWaitingTime;
		int iRestrictedCharacterCount;

		int nAnswer;

		bool isTextCenterMode;
		bool isWaitFlag;

		int iTotalLineCount;

		PyObject * poEventHandler;

		SEventSet() = default;
		virtual ~SEventSet() = default;
		CLASS_DELETE_COPYMOVE(SEventSet);
	} TEventSet;

	using TEventSetVector = std::vector<TEventSet *>;


public:
	CPythonEventManager();
	~CPythonEventManager();
	CLASS_DELETE_COPYMOVE(CPythonEventManager);

	void Destroy();

	int RegisterEventSet(const char * c_szFileName);
	int RegisterEventSetFromString(const std::string & strScript);
	void ClearEventSeti(int iIndex);
	void __ClearEventSetp(TEventSet * pEventSet);

	void SetEventHandler(int iIndex, PyObject * poEventHandler);
	void SetRestrictedCount(int iIndex, int iCount);

	int GetEventSetLocalYPosition(int iIndex);
	void AddEventSetLocalYPosition(int iIndex, int iAddValue);
	void InsertText(int iIndex, const char * c_szText, int iX_pos = 0);

	void UpdateEventSet(int iIndex, int ix, int iy);
	void RenderEventSet(int iIndex);
	void SetEventSetWidth(int iIndex, int iWidth);

	void Skip(int iIndex);
	bool IsWait(int iIndex);
	void EndEventProcess(int iIndex);

	void SelectAnswer(int iIndex, int iAnswer) const;
	void SetVisibleStartLine(int iIndex, int iStartLine);
	int GetVisibleStartLine(int iIndex);
	int GetLineCount(int iIndex);
	void SetVisibleLineCount(int iIndex, int iLineCount);
	void SetFontColor(float iIndex, float fR, float fG, float fB, float fA);
	int GetLineHeight(int iIndex);
	void SetYPosition(int iIndex, int iY);
	int GetProcessedLineCount(int iIndex);
	void AllProcessEventSet(int iIndex);
	int GetTotalLineCount(int iIndex);

	void SetInterfaceWindow(PyObject * poInterface);
	void SetLeftTimeString(const char * c_szString);

protected:
	void __InitEventSet(TEventSet & rEventSet);
	void __InsertLine(TEventSet & rEventSet, BOOL isCenter = FALSE, int iX_pos = 0);
	void __AddSpace(TEventSet & rEventSet, int iSpace) const;

	uint32_t GetEmptyEventSetSlot();

	bool CheckEventSetIndex(int iIndex) const;

	bool ProcessEventSet(TEventSet * pEventSet);

	void ClearLine(TEventSet * pEventSet);
	void RefreshLinePosition(TEventSet * pEventSet) const;
	bool GetScriptEventIndex(const char * c_szName, int * pEventPosition, int * pEventType);

	void MakeQuestion(TEventSet * pEventSet, script::TArgList & rArgumentList) const;
	void MakeNextButton(TEventSet * pEventSet, int iButtonType) const;

protected:
	TEventSetVector m_EventSetVector;
	BOOL m_isQuestConfirmWait;

private:
	CDynamicPool<TEventSet> m_EventSetPool;
	CDynamicPool<CGraphicTextInstance> m_ScriptTextLinePool;
	PyObject * m_poInterface{nullptr};
	std::string m_strLeftTimeString;

#ifdef ENABLE_REVERSED_FUNCTIONS_EX
	bool gs_bEnableMouseClickEvents;
	bool gs_bEnableMouseWheelEvents;
#endif
};