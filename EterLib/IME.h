#pragma once

#include <imm.h>

#pragma comment(lib, "imm32.lib")

#include "DIMM.h"

class IIMEEventSink
{
public:
	virtual bool OnWM_CHAR(WPARAM wParam, LPARAM lParam) = 0;
	virtual void OnUpdate() = 0;

	virtual void OnChangeCodePage() = 0;

	virtual void OnOpenCandidateList() = 0;
	virtual void OnCloseCandidateList() = 0;

	virtual void OnOpenReadingWnd() = 0;
	virtual void OnCloseReadingWnd() = 0;
};

class CIME
{
public:
	enum
	{
		IMEREADING_MAXLEN = 128,
		IMESTR_MAXLEN = 1024,
		IMECANDIDATE_MAXLEN = 32768,
		MAX_CANDLIST = 10,
		MAX_CANDIDATE_LENGTH = 256
	};

public:
	CIME();
	virtual ~CIME();

	bool Initialize(HWND hWnd);
	void Uninitialize(void);

	static void Clear();

	void SetMax(int iMax);
	void SetUserMax(int iMax);
	void SetText(const char * szText, int len);
	int GetText(std::string & rstrText, bool addCodePage = false);
	const char * GetCodePageText();
	int GetCodePage();

	// Candidate List
	int GetCandidateCount();
	int GetCandidatePageCount();
	int GetCandidate(uint32_t index, std::string & rstrText);
	int GetCandidateSelection();

	// Reading Information
	int GetReading(std::string & rstrText);
	int GetReadingError();

	void SetInputMode(uint32_t dwMode);
	uint32_t GetInputMode();

	bool IsIMEEnabled();
	void EnableIME(bool bEnable = true);
	void DisableIME();

	void EnableCaptureInput();
	void DisableCaptureInput();
	bool IsCaptureEnabled();

	void SetNumberMode();
	void SetStringMode();
	bool __IsWritable(wchar_t key);
	void AddExceptKey(wchar_t key);
	void ClearExceptKey();

	void PasteTextFromClipBoard();
	void EnablePaste(bool bFlag);
	void PasteString(const char * str);
	static void FinalizeString(bool bSend = false);

	void UseDefaultIME();

	static int GetCurPos();
	static int GetCompLen();
	static int GetULBegin();
	static int GetULEnd();

	static void CloseCandidateList();
	static void CloseReadingInformation();
	static void ChangeInputLanguage();
	static void ChangeInputLanguageWorker();

	LRESULT WMInputLanguage(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WMStartComposition(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WMComposition(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WMEndComposition(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WMNotify(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WMChar(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);

protected:
	void IncCurPos();
	void DecCurPos();
	void SetCurPos(int offset);
	void DelCurPos();

protected:
	static void CheckInputLocale();
	static void CheckToggleState();
	static void SetSupportLevel(uint32_t dwImeLevel);

	void InsertString(wchar_t * wString, int iSize);

	void OnChar(wchar_t c);

	uint32_t GetCodePageFromLang(LANGID langid);
	void ResultProcess(HIMC hImc);
	void CompositionProcessBuilding(HIMC hImc);
	void CompositionProcess(HIMC hImc);
	void AttributeProcess(HIMC hImc);
	void CandidateProcess(HIMC hImc);
	void ReadingProcess(HIMC hImc);

	bool IsMax(const wchar_t * wInput, int len);

	uint32_t GetImeId(uint32_t uIndex = 0);
	bool GetReadingWindowOrientation();
	static void SetupImeApi();

	static INPUTCONTEXT *(WINAPI * _ImmLockIMC)(HIMC);
	static BOOL(WINAPI * _ImmUnlockIMC)(HIMC);
	static LPVOID(WINAPI * _ImmLockIMCC)(HIMCC);
	static BOOL(WINAPI * _ImmUnlockIMCC)(HIMCC);

	static uint32_t(WINAPI * _GetReadingString)(HIMC, uint32_t, LPWSTR, PINT, BOOL *, PUINT);
	static BOOL(WINAPI * _ShowReadingWindow)(HIMC, BOOL);

protected:
	HIMC m_hOrgIMC;
	int m_max;
	int m_userMax;

	BOOL m_bOnlyNumberMode;

	std::vector<wchar_t> m_exceptKey;

	bool m_bEnablePaste;
	bool m_bUseDefaultIME;


public:
	static bool ms_bInitialized;
	static bool ms_bDisableIMECompletely;
	static bool ms_bUILessMode;
	static bool ms_bImeEnabled;
	static bool ms_bCaptureInput;
	static bool ms_bChineseIME;
	static bool ms_bUseIMMCandidate;

	static HWND ms_hWnd;
	static HKL ms_hklCurrent;
	static char ms_szKeyboardLayout[KL_NAMELENGTH + 1];
	static OSVERSIONINFOA ms_stOSVI;

	static HINSTANCE ms_hImm32Dll;
	static HINSTANCE ms_hCurrentImeDll;
	static uint32_t ms_dwImeState;

	static uint32_t ms_adwId[2];

	// IME Level
	static uint32_t ms_dwIMELevel;
	static uint32_t ms_dwIMELevelSaved;

	// Candidate List
	static bool ms_bCandidateList;
	static uint32_t ms_dwCandidateCount;
	static bool ms_bVerticalCandidate;
	static int ms_iCandListIndexBase;
	static WCHAR ms_wszCandidate[CIME::MAX_CANDLIST][MAX_CANDIDATE_LENGTH];
	static uint32_t ms_dwCandidateSelection;
	static uint32_t ms_dwCandidatePageSize;

	// Reading Information
	static bool ms_bReadingInformation;
	static int ms_iReadingError;
	static bool ms_bHorizontalReading;
	static std::vector<wchar_t> ms_wstrReading;

	// Indicator
	static wchar_t * ms_wszCurrentIndicator;

	static IIMEEventSink * ms_pEvent;

	wchar_t m_wszComposition[IMESTR_MAXLEN];
	static wchar_t m_wText[IMESTR_MAXLEN];

	static int ms_compLen;
	static int ms_curpos;
	static int ms_lastpos;
	static int ms_ulbegin;
	static int ms_ulend;

	static uint32_t ms_uOutputCodePage;
	static uint32_t ms_uInputCodePage;
};
