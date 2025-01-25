#pragma once

#include "../EterBase/Utils.h"
#if defined(ENABLE_RENDER_TARGET) || defined(ENABLE_INGAME_WIKI)
#	include <cstdint>
#endif

#ifdef ENABLE_OFFICAL_FEATURES
enum WindowTypes
{
	WINDOW_TYPE_WINDOW,
	WINDOW_TYPE_TEXTLINE,
	WINDOW_TYPE_MAX_NUM
};
#endif

namespace UI
{
class CWindow
{
public:
	using TWindowContainer = std::list<CWindow *>;

	static uint32_t Type();
	BOOL IsType(uint32_t dwType);

	enum EHorizontalAlign
	{
		HORIZONTAL_ALIGN_LEFT = 0,
		HORIZONTAL_ALIGN_CENTER = 1,
		HORIZONTAL_ALIGN_RIGHT = 2
	};

	enum EVerticalAlign
	{
		VERTICAL_ALIGN_TOP = 0,
		VERTICAL_ALIGN_CENTER = 1,
		VERTICAL_ALIGN_BOTTOM = 2
	};

	enum EFlags
	{
		FLAG_MOVABLE = (1 << 0), // 움직일 수 있는 창
		FLAG_LIMIT = (1 << 1), // 창이 화면을 벗어나지 않음
		FLAG_SNAP = (1 << 2), // 스냅 될 수 있는 창
		FLAG_DRAGABLE = (1 << 3),
		FLAG_ATTACH = (1 << 4), // 완전히 부모에 붙어 있는 창 (For Drag / ex. ScriptWindow)
		FLAG_RESTRICT_X = (1 << 5), // 좌우 이동 제한
		FLAG_RESTRICT_Y = (1 << 6), // 상하 이동 제한
		FLAG_NOT_CAPTURE = (1 << 7),
		FLAG_FLOAT = (1 << 8), // 공중에 떠있어서 순서 재배치가 되는 창
		FLAG_NOT_PICK = (1 << 9), // 마우스에 의해 Pick되지 않는 창
		FLAG_IGNORE_SIZE = (1 << 10),
		FLAG_RTL = (1 << 11), // Right-to-left
	};

public:
	explicit CWindow(PyObject * ppyObject);
	virtual ~CWindow();

	void AddChild(CWindow * pWin);

	void Clear();
	void DestroyHandle();
	void Update();
	void Render();
	void			BeginRender();
	void			EndRender();

	void SetName(const char * c_szName);
	const char * GetName() const { return m_strName.c_str(); }
	void SetSize(long width, long height);
	long GetWidth() const { return m_lWidth; }
	long GetHeight() const { return m_lHeight; }

	void SetHorizontalAlign(uint32_t dwAlign);
	void SetVerticalAlign(uint32_t dwAlign);
	void SetPosition(long x, long y);
	void GetPosition(long * plx, long * ply) const;
	long GetPositionX() const { return m_x; }
	long GetPositionY() const { return m_y; }
	RECT & GetRect() { return m_rect; }
	void GetLocalPosition(long & rlx, long & rly) const;
	void GetMouseLocalPosition(long & rlx, long & rly) const;
	long UpdateRect();

	RECT & GetLimitBias() { return m_limitBiasRect; }
	void SetLimitBias(long l, long r, long t, long b) { m_limitBiasRect.left = l, m_limitBiasRect.right = r, m_limitBiasRect.top = t, m_limitBiasRect.bottom = b; }

	void Show();
	void Hide();
#ifdef ENABLE_INGAME_WIKI
	virtual	bool	IsShow();
	void			OnHideWithChilds();
	void			OnHide();
#else
	bool			IsShow() { return m_bShow; }
#endif
	bool IsRendering();

	bool HasParent() const { return m_pParent ? true : false; }
	bool HasChild() const { return m_pChildList.empty() ? false : true; }
	int GetChildCount() const { return m_pChildList.size(); }

	CWindow * GetRoot();
	CWindow * GetParent() const;
#ifdef ENABLE_INGAME_WIKI
	bool			IsChild(CWindow* pWin, bool bCheckRecursive = false);
#else
	bool			IsChild(CWindow* pWin);
#endif
	void DeleteChild(CWindow * pWin);
	void SetTop(CWindow * pWin);

	bool IsIn(long x, long y) const;
	bool IsIn() const;
	CWindow * PickWindow(long x, long y);
	CWindow * PickTopWindow(long x, long y); // NOTE : Children으로 내려가지 않고 상위에서만
	//        체크 하는 특화된 함수

	void __RemoveReserveChildren();

	void AddFlag(uint32_t flag) { SET_BIT(m_dwFlag, flag); }
	void RemoveFlag(uint32_t flag) { REMOVE_BIT(m_dwFlag, flag); }
	bool IsFlag(uint32_t flag) const { return (m_dwFlag & flag) ? true : false; }
#ifdef ENABLE_INGAME_WIKI
	void			SetInsideRender(BOOL flag);
	void			GetRenderBox(RECT* box);
	void			UpdateTextLineRenderBox();
	void			UpdateRenderBox();
	void			UpdateRenderBoxRecursive();
#endif
	/////////////////////////////////////

	virtual void OnRender();
#ifdef ENABLE_INGAME_WIKI
	virtual void	OnAfterRender();
	virtual void	OnUpdateRenderBox() {}
#endif
	virtual void OnUpdate();
	virtual void OnChangePosition() {}

	virtual void OnSetFocus();
	virtual void OnKillFocus();

	virtual void OnMouseDrag(long lx, long ly);
	virtual void OnMouseOverIn();
	virtual void OnMouseOverOut();
	virtual void OnMouseOver();
	virtual void OnDrop();
	virtual void OnTop();
	virtual void OnIMEUpdate();

	virtual void OnMoveWindow(long lx, long ly);

	///////////////////////////////////////

	BOOL RunIMETabEvent();
	BOOL RunIMEReturnEvent();
	BOOL RunIMEKeyDownEvent(int ikey);

	CWindow * RunKeyDownEvent(int ikey);
	BOOL RunKeyUpEvent(int ikey);
	BOOL RunPressEscapeKeyEvent();
	BOOL RunPressExitKeyEvent();

	virtual BOOL OnIMETabEvent();
	virtual BOOL OnIMEReturnEvent();
	virtual BOOL OnIMEKeyDownEvent(int ikey);

	virtual BOOL OnIMEChangeCodePage();
	virtual BOOL OnIMEOpenCandidateListEvent();
	virtual BOOL OnIMECloseCandidateListEvent();
	virtual BOOL OnIMEOpenReadingWndEvent();
	virtual BOOL OnIMECloseReadingWndEvent();

	virtual BOOL OnMouseLeftButtonDown();
	virtual BOOL OnMouseLeftButtonUp();
	virtual BOOL OnMouseLeftButtonDoubleClick();
	virtual BOOL OnMouseRightButtonDown();
	virtual BOOL OnMouseRightButtonUp();
	virtual BOOL OnMouseRightButtonDoubleClick();
	virtual BOOL OnMouseMiddleButtonDown();
	virtual BOOL OnMouseMiddleButtonUp();
#ifdef ENABLE_RUN_MOUSE_WHEEL_EVENT
	virtual BOOL RunMouseWheelEvent(long nLen);
#endif

	virtual BOOL OnKeyDown(int ikey);
	virtual BOOL OnKeyUp(int ikey);
	virtual BOOL OnPressEscapeKey();
	virtual BOOL OnPressExitKey();
	///////////////////////////////////////

	virtual void SetColor(uint32_t dwColor) {}
	virtual BOOL OnIsType(uint32_t dwType);
	/////////////////////////////////////

	virtual BOOL IsWindow() { return TRUE; }
	/////////////////////////////////////

#ifdef ENABLE_INGAME_WIKI
public:
	virtual void	iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom);
	virtual void	SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
	virtual int		GetRenderingWidth();
	virtual int		GetRenderingHeight();
	void			ResetRenderingRect(bool bCallEvent = true);

private:
	virtual void	OnSetRenderingRect();
#endif

protected:
	std::string m_strName;

	EHorizontalAlign m_HorizontalAlign;
	EVerticalAlign m_VerticalAlign;
	long m_x, m_y; // X,Y 상대좌표
	long m_lWidth, m_lHeight; // 크기
	RECT m_rect; // Global 좌표
	RECT m_limitBiasRect; // limit bias 값

#ifdef ENABLE_INGAME_WIKI
	RECT				m_renderingRect;
#endif
	bool m_bMovable;
	bool m_bShow;

	uint32_t m_dwFlag;
#ifdef ENABLE_OFFICAL_FEATURES
	uint8_t m_bWindowType;
#endif

	PyObject * m_poHandler;

	CWindow * m_pParent;
	TWindowContainer m_pChildList;

	BOOL m_isUpdatingChildren;
#ifdef ENABLE_INGAME_WIKI
	BOOL				m_isInsideRender;
	RECT				m_renderBox;
#endif
	TWindowContainer m_pReserveChildList;

#ifdef _DEBUG
public:
	uint32_t DEBUG_dwCounter;
#endif
};

class CLayer : public CWindow
{
public:
	explicit CLayer(PyObject * ppyObject) : CWindow(ppyObject) {}
	~CLayer() override = default;

	BOOL IsWindow() override { return FALSE; }
};

#ifdef ENABLE_RENDER_TARGET
class CUiRenderTarget : public CWindow
{
public:
	CUiRenderTarget(PyObject* ppyObject);
	virtual ~CUiRenderTarget();

	bool SetRenderTarget(uint8_t index);

protected:
	uint32_t m_dwIndex;
	void OnRender();
};
#endif

#ifdef ENABLE_INGAME_WIKI
class CUiWikiRenderTarget : public CWindow
{
public:
	CUiWikiRenderTarget(PyObject * ppyObject);
	virtual ~CUiWikiRenderTarget();

public:
	bool	SetWikiRenderTargetModule(int iRenderTargetModule);
	void	OnUpdateRenderBox();

protected:
	void	OnRender();

protected:
	uint32_t	m_dwIndex;
};
#endif

class CBox : public CWindow
{
public:
	explicit CBox(PyObject * ppyObject);
	~CBox() override;

	void SetColor(uint32_t dwColor) override;

protected:
	void OnRender() override;

protected:
	uint32_t m_dwColor;
};

class CBar : public CWindow
{
public:
	explicit CBar(PyObject * ppyObject);
	~CBar() override;

	void SetColor(uint32_t dwColor) override;

protected:
	void OnRender() override;

protected:
	uint32_t m_dwColor;
};

class CLine : public CWindow
{
public:
	CLine(PyObject * ppyObject);
	~CLine() override;

	void SetColor(uint32_t dwColor) override;

protected:
	void OnRender() override;

protected:
	uint32_t m_dwColor;
};

class CBar3D : public CWindow
{
public:
	static uint32_t Type();

public:
	CBar3D(PyObject * ppyObject);
	~CBar3D() override;

	void SetColor(uint32_t dwLeft, uint32_t dwRight, uint32_t dwCenter);

protected:
	void OnRender() override;

protected:
	uint32_t m_dwLeftColor;
	uint32_t m_dwRightColor;
	uint32_t m_dwCenterColor;
};

// Text
class CTextLine : public CWindow
{
#ifdef ENABLE_INGAME_WIKI
public:
	static uint32_t Type();
#endif

public:
	CTextLine(PyObject * ppyObject);
	~CTextLine() override;

	void SetMax(int iMax);
	void SetHorizontalAlign(int iType);
	void SetVerticalAlign(int iType);
	void SetSecret(BOOL bFlag);
	void SetOutline(BOOL bFlag);
	void SetFeather(BOOL bFlag);
	void SetMultiLine(BOOL bFlag);
	void SetFontName(const char * c_szFontName);
	void SetFontColor(uint32_t dwColor);
	void SetLimitWidth(float fWidth);

	void ShowCursor();
	bool IsShowCursor();
	void HideCursor();
	int GetCursorPosition();

	void SetText(const char * c_szText);
	const char * GetText();

	void GetTextSize(int * pnWidth, int * pnHeight);
#ifdef ENABLE_SUNG_MAHI_TOWER
	void GetCharSize(short* sWidth);
#endif

#ifdef ENABLE_INGAME_WIKI
	bool IsShow();
	int GetRenderingWidth();
	int GetRenderingHeight();
	void OnSetRenderingRect();
#endif

#ifdef WJ_MULTI_TEXTLINE
	void SetLineHeight(uint16_t iLineHeight);
	uint16_t GetLineHeight();
	uint16_t GetTextLineCount();
	void DisableEnterToken();
#endif

protected:
	void OnUpdate() override;
	void OnRender() override;
	void OnChangePosition() override;

	virtual void OnSetText(const char * c_szText);

#ifdef ENABLE_INGAME_WIKI
	void OnUpdateRenderBox() {
		UpdateTextLineRenderBox();
		m_TextInstance.SetRenderBox(m_renderBox);
	}
#endif

protected:
	CGraphicTextInstance m_TextInstance;
#ifdef ENABLE_OFFICAL_FEATURES
	uint8_t m_bWindowType;
#endif
};

class CNumberLine : public CWindow
{
public:
	explicit CNumberLine(PyObject * ppyObject);
	explicit CNumberLine(CWindow * pParent);
	~CNumberLine() override;

	void SetPath(const char * c_szPath);
	void SetHorizontalAlign(int iType);
	void SetNumber(const char * c_szNumber);

protected:
	void ClearNumber();
	void OnRender() override;
	void OnChangePosition() override;

protected:
	std::string m_strPath;
	std::string m_strNumber;
	std::vector<CGraphicImageInstance *> m_ImageInstanceVector;

	int m_iHorizontalAlign;
	uint32_t m_dwWidthSummary;
};

// Image
class CImageBox : public CWindow
{
public:
	explicit CImageBox(PyObject * ppyObject);
	~CImageBox() override;

#ifdef ENABLE_INGAME_WIKI
	void UnloadImage()
	{
		OnDestroyInstance();
		SetSize(GetWidth(), GetHeight());
		UpdateRect();
	}
#endif

	BOOL LoadImage(const char * c_szFileName);
	void SetDiffuseColor(float fr, float fg, float fb, float fa) const;

	int GetWidth() const;
	int GetHeight() const;

#if defined(ENABLE_IMAGE_SCALE) || defined(ENABLE_GROWTH_PET_SYSTEM)
	void SetScale(float sx, float sy);
#endif

#ifdef ENABLE_12ZI
	void SetCoolTime(float fCoolTime);
	void SetCoolTimeStart(float fCoolTimeStart);
#endif
#ifdef ENABLE_OFFICAL_FEATURES
	void LeftRightReverse();
#endif

protected:
	virtual void OnCreateInstance();
	virtual void OnDestroyInstance();

	void OnUpdate() override;
	void OnRender() override;
	void OnChangePosition() override;
#ifdef ENABLE_12ZI
	float m_fCoolTime;
	float m_fCoolTimeStart;
#endif

protected:
	CGraphicImageInstance * m_pImageInstance;
};
class CMarkBox : public CWindow
{
public:
	explicit CMarkBox(PyObject * ppyObject);
	~CMarkBox() override;

	void LoadImage(const char * c_szFilename);
	void SetDiffuseColor(float fr, float fg, float fb, float fa) const;
	void SetIndex(uint32_t uIndex) const;
	void SetScale(FLOAT fScale) const;

protected:
	virtual void OnCreateInstance();
	virtual void OnDestroyInstance();

	void OnUpdate() override;
	void OnRender() override;
	void OnChangePosition() override;

protected:
	CGraphicMarkInstance * m_pMarkInstance;
};
class CExpandedImageBox : public CImageBox
{
public:
	static uint32_t Type();

public:
	explicit CExpandedImageBox(PyObject * ppyObject);
	~CExpandedImageBox() override;

	void SetScale(float fx, float fy);
	void SetOrigin(float fx, float fy) const;
	void SetRotation(float fRotation) const;
	void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom) const;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	void SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom);
#endif
#if defined(ENABLE_IMAGE_CLIP_RECT) || defined(ENABLE_BATTLE_PASS_SYSTEM)
	void SetImageClipRect(float fLeft, float fTop, float fRight, float fBottom, bool bIsVertical = false);
#endif
	void SetRenderingMode(int iMode) const;
#ifdef ENABLE_INGAME_WIKI
	int GetRenderingWidth();
	int GetRenderingHeight();
	void OnSetRenderingRect();
	void SetExpandedRenderingRect(float fLeftTop, float fLeftBottom, float fTopLeft, float fTopRight, float fRightTop, float fRightBottom, float fBottomLeft, float fBottomRight);
	void SetTextureRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
	uint32_t GetPixelColor(uint32_t x, uint32_t y);
#endif

protected:
	void OnCreateInstance() override;
	void OnDestroyInstance() override;
#ifdef ENABLE_INGAME_WIKI
	void OnUpdateRenderBox();
#endif

	void OnUpdate() override;
	void OnRender() override;

	BOOL OnIsType(uint32_t dwType) override;
};
class CAniImageBox : public CWindow
{
public:
	static uint32_t Type();

public:
	explicit CAniImageBox(PyObject * ppyObject);
	~CAniImageBox() override;

	void SetDelay(int iDelay);
	void AppendImage(const char * c_szFileName);
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
	void SetScale(float fx, float fy);
#endif
	void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
#ifdef ENABLE_GROWTH_PET_SYSTEM
	void SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom);
#endif
	void SetRenderingMode(int iMode);
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	void SetDiffuseColor(float fR, float fG, float fB, float fA);
	void SetDiffuseColorByType(uint8_t byColorType);
#endif
	void ResetFrame();
#ifdef ENABLE_FISHING_RENEWAL
	void SetRotation(float fRotation);
#endif

protected:
	void OnUpdate() override;
	void OnRender() override;
	void OnChangePosition() override;
	virtual void OnEndFrame();

	BOOL OnIsType(uint32_t dwType) override;

protected:
	uint8_t m_bycurDelay;
	uint8_t m_byDelay;
	uint8_t m_bycurIndex;
#ifdef ENABLE_FISHING_RENEWAL
	bool m_RotationProcess;
#endif
	std::vector<CGraphicExpandedImageInstance *> m_ImageVector;
};

#ifdef ENABLE_FISHING_RENEWAL
class CFishBox : public CWindow
{
public:
	static uint32_t Type();

public:
	CFishBox(PyObject* ppyObject);
	virtual ~CFishBox();

	bool GetMove();
	void MoveStart();
	void MoveStop();
	void SetRandomPosition();
	void GetPosition(int* x, int* y);
	void RegisterAni(CAniImageBox* ani);

protected:
	void OnUpdate();
	virtual void OnEndMove();

	BOOL OnIsType(uint32_t dwType);

	D3DXVECTOR2 m_v2SrcPos, m_v2DstPos, m_v2NextPos, m_v2Direction, m_v2NextDistance;
	float m_fDistance, m_fMoveSpeed;
	bool m_bIsMove;
	bool m_left, m_right;
	uint8_t m_lastRandom;
	float m_direction;
};
#endif

// Button
class CButton : public CWindow
{
public:
	explicit CButton(PyObject * ppyObject);
	~CButton() override;

	BOOL SetUpVisual(const char * c_szFileName);
	BOOL SetOverVisual(const char * c_szFileName);
	BOOL SetDownVisual(const char * c_szFileName);
	BOOL SetDisableVisual(const char * c_szFileName);

	const char * GetUpVisualFileName();
	const char * GetOverVisualFileName();
	const char * GetDownVisualFileName();

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MINI_GAME_YUTNORI)
	void SetScale(float fx, float fy);
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	long GetImageWidth();
	long GetImageHeight();
#endif

#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MAILBOX)
	void EnableFlash();
	void DisableFlash();
#ifdef ENABLE_12ZI
	void SetAlwaysToolTip(bool func);
#endif
#endif

	void Flash();
	void Enable();
	void Disable();

	void SetUp();
	void Up();
	void Over();
	void Down();

	BOOL IsDisable() const;
	BOOL IsPressed() const;
#ifdef ENABLE_INGAME_WIKI
	void OnSetRenderingRect();
#endif
#ifdef ENABLE_OFFICAL_FEATURES
	void LeftRightReverse();
#endif

protected:
	void OnUpdate() override;
	void OnRender() override;
	void OnChangePosition() override;

	BOOL OnMouseLeftButtonDown() override;
	BOOL OnMouseLeftButtonDoubleClick() override;
	BOOL OnMouseLeftButtonUp() override;
	void OnMouseOverIn() override;
	void OnMouseOverOut() override;

	BOOL IsEnable() const;

#ifdef ENABLE_INGAME_WIKI
	void SetCurrentVisual(CGraphicExpandedImageInstance* pVisual);
#else
	void SetCurrentVisual(CGraphicImageInstance * pVisual);
#endif

protected:
	BOOL m_bEnable;
	BOOL m_isPressed;
	BOOL m_isFlash;
#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MAILBOX)
	BOOL m_isEnableFlash;
#ifdef ENABLE_12ZI
	BOOL m_isAlwaysTooltip;
#endif
#endif

#ifdef ENABLE_INGAME_WIKI
	CGraphicExpandedImageInstance* m_pcurVisual;
	CGraphicExpandedImageInstance m_upVisual;
	CGraphicExpandedImageInstance m_overVisual;
	CGraphicExpandedImageInstance m_downVisual;
	CGraphicExpandedImageInstance m_disableVisual;
#else
	CGraphicImageInstance * m_pcurVisual;
	CGraphicImageInstance m_upVisual;
	CGraphicImageInstance m_overVisual;
	CGraphicImageInstance m_downVisual;
	CGraphicImageInstance m_disableVisual;
#endif

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
	BOOL m_bScale = FALSE;
	float m_pScaleX = 1.0;
	float m_pScaleY = 1.0;
#endif
};
class CRadioButton : public CButton
{
public:
	explicit CRadioButton(PyObject * ppyObject);
	~CRadioButton() override;

protected:
	BOOL OnMouseLeftButtonDown() override;
	BOOL OnMouseLeftButtonUp() override;
	void OnMouseOverIn() override;
	void OnMouseOverOut() override;
};
class CToggleButton : public CButton
{
public:
	explicit CToggleButton(PyObject * ppyObject);
	~CToggleButton() override;

protected:
	BOOL OnMouseLeftButtonDown() override;
	BOOL OnMouseLeftButtonUp() override;
	void OnMouseOverIn() override;
	void OnMouseOverOut() override;
};
class CDragButton : public CButton
{
public:
	explicit CDragButton(PyObject * ppyObject);
	~CDragButton() override;

	void SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight);

protected:
	void OnChangePosition() override;
	void OnMouseOverIn() override;
	void OnMouseOverOut() override;

protected:
	RECT m_restrictArea;
};

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_MINI_GAME_YUTNORI)
class CMoveTextLine : public CTextLine
{
public:
	explicit CMoveTextLine(PyObject * ppyObject);
	~CMoveTextLine() override;

public:
	static uint32_t Type();

	void SetMoveSpeed(float fSpeed);
	void SetMovePosition(float fDstX, float fDstY);
	bool GetMove();
	void MoveStart();
	void MoveStop();

protected:
	void OnUpdate();
	void OnRender();
	void OnEndMove();
	void OnChangePosition();

	BOOL OnIsType(uint32_t dwType);

	D3DXVECTOR2 m_v2SrcPos, m_v2DstPos, m_v2NextPos, m_v2Direction, m_v2NextDistance;
	float m_fDistance, m_fMoveSpeed;
	bool m_bIsMove;
};

class CMoveImageBox : public CImageBox
{
public:
	explicit CMoveImageBox(PyObject * ppyObject);
	~CMoveImageBox() override;

	static uint32_t Type();

	void SetMoveSpeed(float fSpeed);
	void SetMovePosition(float fDstX, float fDstY);
	bool GetMove();
	void MoveStart();
	void MoveStop();

	void SetMoveAll(bool bFlag) { m_bMoveAll = bFlag; }

protected:
	virtual void OnCreateInstance();
	virtual void OnDestroyInstance();

	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnEndMove();

	BOOL OnIsType(uint32_t dwType);

	D3DXVECTOR2 m_v2SrcPos, m_v2DstPos, m_v2NextPos, m_v2Direction, m_v2NextDistance;
	float m_fDistance, m_fMoveSpeed;
	bool m_bIsMove;
	bool m_bMoveAll;
};

class CMoveScaleImageBox : public CMoveImageBox
{
public:
	explicit CMoveScaleImageBox(PyObject * ppyObject);
	~CMoveScaleImageBox() override;

	static uint32_t Type();

	void SetMaxScale(float fMaxScale);
	void SetMaxScaleRate(float fMaxScaleRate);
	void SetScalePivotCenter(bool bScalePivotCenter);

protected:
	virtual void OnCreateInstance();
	virtual void OnDestroyInstance();

	virtual void OnUpdate();

	BOOL OnIsType(uint32_t dwType);

	float m_fMaxScale, m_fMaxScaleRate, m_fScaleDistance, m_fAdditionalScale;
	D3DXVECTOR2 m_v2CurScale;
};
#endif
} // namespace UI

extern BOOL g_bOutlineBoxEnable;

#ifdef ENABLE_FISHING_RENEWAL
#ifndef MAX_FISHING_WAYS
#define MAX_FISHING_WAYS 6
#endif
extern float listFishLeft[MAX_FISHING_WAYS][3];
extern float listFishRight[MAX_FISHING_WAYS][3];
#endif
