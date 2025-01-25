#pragma once

namespace UI
{
class CWindow;

class CWindowManager : public CSingleton<CWindowManager>
{
public:
	typedef std::map<std::string, CWindow *> TLayerContainer;
	using TWindowContainer = std::list<CWindow *>;
	typedef std::map<int, CWindow *> TKeyCaptureWindowMap;

public:
	CWindowManager();
	~CWindowManager() override;

	void Destroy();

	float GetAspect() const;
	void SetScreenSize(long lWidth, long lHeight);
	void SetResolution(int hres, int vres);

	void GetResolution(long & rx, long & ry) const
	{
		rx = m_iHres;
		ry = m_iVres;
	}

	void SetMouseHandler(PyObject * poMouseHandler);
	long GetScreenWidth() const { return m_lWidth; }
	long GetScreenHeight() const { return m_lHeight; }
	void GetMousePosition(long & rx, long & ry) const;
	BOOL IsDragging() const;

	CWindow * GetLockWindow() const { return m_pLockWindow; }
	CWindow * GetPointWindow() const { return m_pPointWindow; }
	bool IsFocus() const { return (m_pActiveWindow || m_pLockWindow); }
	bool IsFocusWindow(const CWindow * pWindow) const { return pWindow == m_pActiveWindow; }

	static void SetParent(CWindow * pWindow, CWindow * pParentWindow);
	void SetPickAlways(CWindow * pWindow);

	enum
	{
		WT_NORMAL,
		WT_SLOT,
		WT_GRIDSLOT,
		WT_TEXTLINE,
		WT_MARKBOX,
		WT_IMAGEBOX,
		WT_EXP_IMAGEBOX,
		WT_ANI_IMAGEBOX,
		WT_BUTTON,
		WT_RATIOBUTTON,
		WT_TOGGLEBUTTON,
		WT_DRAGBUTTON,
		WT_BOX,
		WT_BAR,
		WT_LINE,
		WT_BAR3D,
		WT_NUMLINE,
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_MINI_GAME_YUTNORI)
		WT_MOVE_IMAGEBOX,
		WT_MOVE_TEXTLINE,
		WT_MOVE_SCALEIMAGEBOX,
#endif
#ifdef ENABLE_FISHING_RENEWAL
		WT_MOVE_ANIIMAGEBOX,
#endif
	};

	CWindow * RegisterWindow(PyObject * po, const char * c_szLayer);
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_MINI_GAME_YUTNORI)
	CWindow * RegisterMoveImageBox(PyObject * po, const char * c_szLayer);
	CWindow * RegisterMoveTextLine(PyObject* po, const char* c_szLayer);
	CWindow * RegisterMoveScaleImageBox(PyObject* po, const char* c_szLayer);
#endif
	CWindow * RegisterTypeWindow(PyObject * po, uint32_t dwWndType, const char * c_szLayer);
	CWindow * RegisterSlotWindow(PyObject * po, const char * c_szLayer);
	CWindow * RegisterGridSlotWindow(PyObject * po, const char * c_szLayer);
	CWindow * RegisterTextLine(PyObject * po, const char * c_szLayer);
	CWindow * RegisterMarkBox(PyObject * po, const char * c_szLayer);
	CWindow * RegisterImageBox(PyObject * po, const char * c_szLayer);
	CWindow * RegisterExpandedImageBox(PyObject * po, const char * c_szLayer);
	CWindow * RegisterAniImageBox(PyObject * po, const char * c_szLayer);
	CWindow * RegisterButton(PyObject * po, const char * c_szLayer);
	CWindow * RegisterRadioButton(PyObject * po, const char * c_szLayer);
	CWindow * RegisterToggleButton(PyObject * po, const char * c_szLayer);
	CWindow * RegisterDragButton(PyObject * po, const char * c_szLayer);
#ifdef ENABLE_RENDER_TARGET
	CWindow *	RegisterRenderTarget(PyObject * po, const char * c_szLayer);
#endif
#ifdef ENABLE_INGAME_WIKI
	CWindow *	RegisterWikiRenderTarget(PyObject* po, const char* c_szLayer);
#endif
	CWindow * RegisterBox(PyObject * po, const char * c_szLayer);
	CWindow * RegisterBar(PyObject * po, const char * c_szLayer);
	CWindow * RegisterLine(PyObject * po, const char * c_szLayer);
	CWindow * RegisterBar3D(PyObject * po, const char * c_szLayer);
	CWindow * RegisterNumberLine(PyObject * po, const char * c_szLayer);
#ifdef ENABLE_FISHING_RENEWAL
	CWindow* RegisterFishBox(PyObject* po, const char* c_szLayer);
#endif

	void DestroyWindow(CWindow * pWin);
	void NotifyDestroyWindow(CWindow * pWindow);

	// Attaching Icon
	BOOL IsAttaching() const;
	uint32_t GetAttachingType() const;
	uint32_t GetAttachingIndex() const;
	uint32_t GetAttachingSlotNumber() const;
	void GetAttachingIconSize(uint8_t * pbyWidth, uint8_t * pbyHeight) const;
	void AttachIcon(uint32_t dwType, uint32_t dwIndex, uint32_t dwSlotNumber, uint8_t byWidth, uint8_t byHeight);
	void DeattachIcon();
	void SetAttachingFlag(BOOL bFlag);
	// Attaching Icon

#ifdef ENABLE_FISH_EVENT
	void SetDisableDeattach(bool bFlag) { m_bIsDisableDettach = bFlag; }
	bool IsDisableDeattach() { return m_bIsDisableDettach; }
#endif

	void OnceIgnoreMouseLeftButtonUpEvent();
	void LockWindow(CWindow * pWin);
	void UnlockWindow();

	void ActivateWindow(CWindow * pWin);
	void DeactivateWindow();
	CWindow * GetActivateWindow() const;
	bool		IsFocusedWindow(CWindow * pWindow);
	static void SetTop(CWindow * pWin);
	void SetTopUIWindow();
	void ResetCapture();

	void Update();
	void Render() const;

	void RunMouseMove(long x, long y);
	void RunMouseLeftButtonDown(long x, long y);
	void RunMouseLeftButtonUp(long x, long y);
	void RunMouseLeftButtonDoubleClick(long x, long y);
	void RunMouseRightButtonDown(long x, long y);
	void RunMouseRightButtonUp(long x, long y);
	void RunMouseRightButtonDoubleClick(long x, long y);
	void RunMouseMiddleButtonDown(long x, long y);
	void RunMouseMiddleButtonUp(long x, long y);
#ifdef ENABLE_RUN_MOUSE_WHEEL_EVENT
	bool RunMouseWheelEvent(long nLen);
#endif

	void RunIMEUpdate() const;
	void RunIMETabEvent();
	void RunIMEReturnEvent() const;
	void RunIMEKeyDown(int vkey) const;
	void RunChangeCodePage() const;
	void RunOpenCandidate() const;
	void RunCloseCandidate() const;
	void RunOpenReading() const;
	void RunCloseReading() const;

	void RunKeyDown(int vkey);
	void RunKeyUp(int vkey);
	void RunPressEscapeKey() const;
	void RunPressExitKey() const;

private:
	void SetMousePosition(long x, long y);
	CWindow * __PickWindow(long x, long y);

	static CWindow * __NewWindow(PyObject * po, uint32_t dwWndType);
	void __ClearReserveDeleteWindowList();

private:
	long m_lWidth;
	long m_lHeight;

	int m_iVres;
	int m_iHres;

	long m_lMouseX, m_lMouseY;
	long m_lDragX, m_lDragY;
	long m_lPickedX, m_lPickedY;

	BOOL m_bOnceIgnoreMouseLeftButtonUpEventFlag;
	int m_iIgnoreEndTime;
#ifdef ENABLE_FISH_EVENT
	bool m_bIsDisableDettach;
#endif

	// Attaching Icon
	PyObject * m_poMouseHandler;
	BOOL m_bAttachingFlag;
	uint32_t m_dwAttachingType;
	uint32_t m_dwAttachingIndex;
	uint32_t m_dwAttachingSlotNumber;
	uint8_t m_byAttachingIconWidth;
	uint8_t m_byAttachingIconHeight;
	// Attaching Icon

	CWindow * m_pActiveWindow;
	TWindowContainer m_ActiveWindowList;
	CWindow * m_pLockWindow;
	TWindowContainer m_LockWindowList;
	CWindow * m_pPointWindow;
	CWindow * m_pLeftCaptureWindow;
	CWindow * m_pRightCaptureWindow;
	CWindow * m_pMiddleCaptureWindow;
	TKeyCaptureWindowMap m_KeyCaptureWindowMap;
	TWindowContainer m_ReserveDeleteWindowList;
	TWindowContainer m_PickAlwaysWindowList;

	CWindow * m_pRootWindow;
	TWindowContainer m_LayerWindowList;
	TLayerContainer m_LayerWindowMap;
};

PyObject * BuildEmptyTuple();
} // namespace UI
