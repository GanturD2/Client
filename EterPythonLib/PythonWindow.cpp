#include "StdAfx.h"
#include "../EterBase/CRC32.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonWindowManager.h"
#ifdef ENABLE_RENDER_TARGET
#	include "../EterLib/CRenderTargetManager.h"
#endif
#ifdef ENABLE_INGAME_WIKI
#	include "../eterLib/CWikiRenderTargetManager.h"
#endif

#ifdef ENABLE_FISHING_RENEWAL
#include <stdlib.h>
float listFishLeft[MAX_FISHING_WAYS][3] =
{
					{56.0f, 36.0f, -46.11f},
					{28.3f, 125.0f, -106.95f},
					{24.3f, 79.0f, -80.08f},
					{52.3f, 182.0f, -141.68f},
					{85.0f, 93.0f, -82.47f},
					{202.7f, 125.7f, -68.11f},
};

float listFishRight[MAX_FISHING_WAYS][3] =
{
					{244.0f, 170.0f, 122.62f},
					{223.0f, 113.0f, 97.73f},
					{230.0f, 177.0f, 120.44f},
					{167.3f, 89.7f, 45.28f},
					{222.0f, 141.7f, 112.08f},
					{253.0f, 152.0f, 116.17f},
};
#endif

BOOL g_bOutlineBoxEnable = FALSE;

namespace UI
{
CWindow::CWindow(PyObject * ppyObject)
	: m_x(0),
	  m_y(0),
	  m_lWidth(0),
	  m_lHeight(0),
	  m_bMovable(false),
	  m_bShow(false),
	  m_dwFlag(0),
	  m_poHandler(ppyObject),
	  m_pParent(nullptr),
	  m_isUpdatingChildren(FALSE)
#ifdef ENABLE_INGAME_WIKI
	, m_isInsideRender(false)
#endif
{
#ifdef _DEBUG
	static uint32_t DEBUG_dwGlobalCounter = 0;
	DEBUG_dwCounter = DEBUG_dwGlobalCounter++;

	m_strName = "!!debug";
#endif
	//assert(m_poHandler != nullptr);
	m_HorizontalAlign = EHorizontalAlign::HORIZONTAL_ALIGN_LEFT;
	m_VerticalAlign = EVerticalAlign::VERTICAL_ALIGN_TOP;
	m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
	m_limitBiasRect.bottom = m_limitBiasRect.left = m_limitBiasRect.right = m_limitBiasRect.top = 0;
#ifdef ENABLE_OFFICAL_FEATURES
	m_bWindowType = WINDOW_TYPE_WINDOW;
#endif
#ifdef ENABLE_INGAME_WIKI
	memset(&m_renderBox, 0, sizeof(m_renderBox));
#endif
}

CWindow::~CWindow() = default;

uint32_t CWindow::Type()
{
	static uint32_t s_dwType = GetCRC32("CWindow", strlen("CWindow"));
	return (s_dwType);
}

BOOL CWindow::IsType(uint32_t dwType)
{
	return OnIsType(dwType);
}

BOOL CWindow::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return TRUE;

	return FALSE;
}

struct FClear
{
	void operator()(CWindow * pWin) const { pWin->Clear(); }
};

void CWindow::Clear()
{
	// FIXME : Children을 즉시 Delete하지는 않는다.
	//         어차피 Python쪽에서 Destroy가 하나씩 다시 호출 될 것이므로..
	//         하지만 만약을 위해 링크는 끊어 놓는다.
	//         더 좋은 형태는 있는가? - [levites]
	std::for_each(m_pChildList.begin(), m_pChildList.end(), FClear());
	m_pChildList.clear();

	m_pParent = nullptr;
	DestroyHandle();
	Hide();
}

void CWindow::DestroyHandle()
{
	m_poHandler = nullptr;
}

void CWindow::Show()
{
	m_bShow = true;
}

void CWindow::Hide()
{
	m_bShow = false;
}

#ifdef ENABLE_INGAME_WIKI
void CWindow::OnHideWithChilds()
{
	OnHide();
	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::OnHideWithChilds));
}

void CWindow::OnHide()
{
	PyCallClassMemberFunc(m_poHandler, "OnHide", BuildEmptyTuple());
}
#endif

bool CWindow::IsRendering()
{
	if (!IsShow())
		return false;

	if (!m_pParent)
		return true;

	return m_pParent->IsRendering();
}

void CWindow::__RemoveReserveChildren()
{
	if (m_pReserveChildList.empty())
		return;

	for (auto & it : m_pReserveChildList)
		m_pChildList.remove(it);
	m_pReserveChildList.clear();
}

void CWindow::Update()
{
	if (!IsShow())
		return;

	__RemoveReserveChildren();

	OnUpdate();

	m_isUpdatingChildren = TRUE;
	for (auto it = m_pChildList.begin(); it != m_pChildList.end();)
	{
		auto it_next = it;
		++it_next;
		(*it)->Update();
		it = it_next;
	}
	m_isUpdatingChildren = FALSE;
}

#ifdef ENABLE_INGAME_WIKI
bool CWindow::IsShow()
{
	if (!m_bShow)
		return false;

	if (m_isInsideRender)
		if (m_renderBox.left + m_renderBox.right >= m_lWidth || m_renderBox.top + m_renderBox.bottom >= m_lHeight)
			return false;

	return true;
}
#endif

void CWindow::Render()
{
	if (!IsShow())
		return;

	OnRender();

	if (g_bOutlineBoxEnable)
	{
		CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f);
		CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::Render));
#ifdef ENABLE_INGAME_WIKI
	OnAfterRender();
#endif
}

void CWindow::BeginRender()
{
	if (!m_poHandler)
		return;

	if (!IsShow())
		return;

	static PyObject* poFuncName_OnUpdate = PyString_InternFromString("BeginRender");

	PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());
}

void CWindow::EndRender()
{
	if (!m_poHandler)
		return;

	if (!IsShow())
		return;

	static PyObject* poFuncName_OnUpdate = PyString_InternFromString("EndRender");

	PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());
}

void CWindow::OnUpdate()
{
	if (!m_poHandler)
		return;

	if (!IsShow())
		return;

	static PyObject * poFuncName_OnUpdate = PyString_InternFromString("OnUpdate");

	//PyCallClassMemberFunc(m_poHandler, "OnUpdate", BuildEmptyTuple());
	PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());
}

void CWindow::OnRender()
{
	if (!m_poHandler)
		return;

	if (!IsShow())
		return;

	//PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
}

#ifdef ENABLE_INGAME_WIKI
void CWindow::OnAfterRender()
{
	if (!m_poHandler)
		return;

	if (!IsShow())
		return;

	PyCallClassMemberFunc(m_poHandler, "OnAfterRender", BuildEmptyTuple());
}
#endif

void CWindow::SetName(const char * c_szName)
{
	m_strName = c_szName;
}

void CWindow::SetSize(long width, long height)
{
	m_lWidth = width;
	m_lHeight = height;

	m_rect.right = m_rect.left + m_lWidth;
	m_rect.bottom = m_rect.top + m_lHeight;

#ifdef ENABLE_INGAME_WIKI
	if (m_isInsideRender)
		UpdateRenderBoxRecursive();
#endif
}

void CWindow::SetHorizontalAlign(uint32_t dwAlign)
{
	m_HorizontalAlign = static_cast<EHorizontalAlign>(dwAlign);
	UpdateRect();
}

void CWindow::SetVerticalAlign(uint32_t dwAlign)
{
	m_VerticalAlign = static_cast<EVerticalAlign>(dwAlign);
	UpdateRect();
}

void CWindow::SetPosition(long x, long y)
{
	m_x = x;
	m_y = y;

	UpdateRect();

#ifdef ENABLE_INGAME_WIKI
	if (m_isInsideRender)
		UpdateRenderBoxRecursive();
#endif
}

#ifdef ENABLE_INGAME_WIKI
void CWindow::UpdateRenderBoxRecursive()
{
	UpdateRenderBox();
	for (auto it = m_pChildList.begin(); it != m_pChildList.end(); ++it)
		(*it)->UpdateRenderBoxRecursive();
}
#endif

void CWindow::GetPosition(long * plx, long * ply) const
{
	*plx = m_x;
	*ply = m_y;
}

long CWindow::UpdateRect()
{
	m_rect.top = m_y;
	if (m_pParent)
	{
		switch (m_VerticalAlign)
		{
		case VERTICAL_ALIGN_BOTTOM:
			m_rect.top = m_pParent->GetHeight() - m_rect.top;
			break;
		case VERTICAL_ALIGN_CENTER:
			m_rect.top = (m_pParent->GetHeight() - GetHeight()) / 2 + m_rect.top;
			break;
		}
		m_rect.top += m_pParent->m_rect.top;
	}
	m_rect.bottom = m_rect.top + m_lHeight;

#if defined(_USE_CPP_RTL_FLIP)
	if (m_pParent == nullptr)
	{
		m_rect.left = m_x;
		m_rect.right = m_rect.left + m_lWidth;
	}
	else
	{
		if (m_pParent->IsFlag(UI::CWindow::FLAG_RTL) == true)
		{
			m_rect.left = m_pParent->GetWidth() - m_lWidth - m_x;
			switch (m_HorizontalAlign)
			{
			case HORIZONTAL_ALIGN_RIGHT:
				m_rect.left = -m_x;
				break;
			case HORIZONTAL_ALIGN_CENTER:
				m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() - m_x;
				break;
			}
			m_rect.left += m_pParent->m_rect.left;
			m_rect.right = m_rect.left + m_lWidth;
		}
		else
		{
			m_rect.left = m_x;
			switch (m_HorizontalAlign)
			{
			case HORIZONTAL_ALIGN_RIGHT:
				m_rect.left = m_pParent->GetWidth() - m_rect.left;
				break;
			case HORIZONTAL_ALIGN_CENTER:
				m_rect.left = (m_pParent->GetWidth() - GetWidth()) / 2 + m_rect.left;
				break;
			}
			m_rect.left += m_pParent->m_rect.left;
			m_rect.right = m_rect.left + m_lWidth;
		}
	}
#else
	m_rect.left = m_x;
	if (m_pParent)
	{
		switch (m_HorizontalAlign)
		{
		case HORIZONTAL_ALIGN_RIGHT:
			m_rect.left = abs(m_pParent->GetWidth()) - m_rect.left;
			break;
		case HORIZONTAL_ALIGN_CENTER:
			m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() / 2 + m_rect.left;
			break;
		}
		m_rect.left += 0L < m_pParent->GetWidth() ? m_pParent->m_rect.left : m_pParent->m_rect.right + abs(m_pParent->GetWidth());
	}
	m_rect.right = m_rect.left + m_lWidth;
#endif
	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

	OnChangePosition();

	return 1;
}

void CWindow::GetLocalPosition(long & rlx, long & rly) const
{
	rlx = rlx - m_rect.left;
	rly = rly - m_rect.top;
}

void CWindow::GetMouseLocalPosition(long & rlx, long & rly) const
{
#ifdef ENABLE_OFFICAL_FEATURES
	CWindowManager::Instance().GetMousePosition(rlx, rly);

	if (GetDefaultCodePage() == CP_ARABIC && m_bWindowType == WindowTypes::WINDOW_TYPE_TEXTLINE)
	{
		rlx = m_rect.right - rlx;
		rly -= m_rect.top;
}
	else
	{
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}
#else
	CWindowManager::Instance().GetMousePosition(rlx, rly);
	rlx = rlx - m_rect.left;
	rly = rly - m_rect.top;
#endif
}

void CWindow::AddChild(CWindow * pWin)
{
	m_pChildList.emplace_back(pWin);
	pWin->m_pParent = this;

#ifdef ENABLE_INGAME_WIKI
	if (m_isInsideRender && !pWin->m_isInsideRender)
		pWin->SetInsideRender(m_isInsideRender);
#endif
}

#ifdef ENABLE_INGAME_WIKI
void CWindow::SetInsideRender(BOOL flag)
{
	if (!m_pParent || m_isInsideRender && m_pParent->m_isInsideRender)
		return;

	if (m_isInsideRender && flag)
		return;

	m_isInsideRender = flag;
	UpdateRenderBox();
	for (auto it = m_pChildList.begin(); it != m_pChildList.end(); ++it)
		(*it)->SetInsideRender(m_isInsideRender);
}

void CWindow::GetRenderBox(RECT* box)
{
	memcpy(box, &m_renderBox, sizeof(RECT));
}

void CWindow::UpdateTextLineRenderBox()
{
	int width, height;
	((CTextLine*)this)->GetTextSize(&width, &height);

	const int pWidth = m_pParent->GetWidth();
	const int pHeight = m_pParent->GetHeight();

	if (m_x - m_pParent->m_renderBox.left < 0)
		m_renderBox.left = -m_x + m_pParent->m_renderBox.left;
	else
		m_renderBox.left = 0;

	if (m_y - m_pParent->m_renderBox.top < 0)
		m_renderBox.top = -m_y + m_pParent->m_renderBox.top;
	else
		m_renderBox.top = 0;

	if (m_x + width > pWidth - m_pParent->m_renderBox.right)
		m_renderBox.right = m_x + width - pWidth + m_pParent->m_renderBox.right;
	else
		m_renderBox.right = 0;

	if (m_y + height > pHeight - m_pParent->m_renderBox.bottom)
		m_renderBox.bottom = m_y + height - pHeight + m_pParent->m_renderBox.bottom;
	else
		m_renderBox.bottom = 0;
}

void CWindow::UpdateRenderBox()
{
	if (!m_isInsideRender || !m_pParent)
		memset(&m_renderBox, 0, sizeof(m_renderBox));
	else
	{
		const int width = m_lWidth;
		const int height = m_lHeight;
		const int pWidth = m_pParent->GetWidth();
		const int pHeight = m_pParent->GetHeight();

		if (m_x - m_pParent->m_renderBox.left < 0)
			m_renderBox.left = -m_x + m_pParent->m_renderBox.left;
		else
			m_renderBox.left = 0;

		if (m_y - m_pParent->m_renderBox.top < 0)
			m_renderBox.top = -m_y + m_pParent->m_renderBox.top;
		else
			m_renderBox.top = 0;

		if (m_x + width > pWidth - m_pParent->m_renderBox.right)
			m_renderBox.right = m_x + width - pWidth + m_pParent->m_renderBox.right;
		else
			m_renderBox.right = 0;

		if (m_y + height > pHeight - m_pParent->m_renderBox.bottom)
			m_renderBox.bottom = m_y + height - pHeight + m_pParent->m_renderBox.bottom;
		else
			m_renderBox.bottom = 0;
	}

	OnUpdateRenderBox();
}
#endif

CWindow * CWindow::GetRoot()
{
	if (m_pParent)
		if (m_pParent->IsWindow())
			return m_pParent->GetRoot();

	return this;
}

CWindow * CWindow::GetParent() const
{
	return m_pParent;
}

#ifdef ENABLE_INGAME_WIKI
bool CWindow::IsChild(CWindow* pWin, bool bCheckRecursive)
#else
bool CWindow::IsChild(CWindow* pWin)
#endif
{
	auto itor = m_pChildList.begin();

	while (itor != m_pChildList.end())
	{
		if (*itor == pWin)
			return true;

#ifdef ENABLE_INGAME_WIKI
		if (bCheckRecursive)
			if ((*itor)->IsChild(pWin, true))
				return true;
#endif

		++itor;
	}

	return false;
}

void CWindow::DeleteChild(CWindow * pWin)
{
	if (m_isUpdatingChildren)
		m_pReserveChildList.emplace_back(pWin);
	else
		m_pChildList.remove(pWin);
}

void CWindow::SetTop(CWindow * pWin)
{
	if (!pWin->IsFlag(FLAG_FLOAT))
		return;

	auto itor = std::find(m_pChildList.begin(), m_pChildList.end(), pWin);
	if (m_pChildList.end() != itor)
	{
		m_pChildList.emplace_back(*itor);
		m_pChildList.erase(itor);

		pWin->OnTop();
	}
	else
		TraceError(" CWindow::SetTop - Failed to find child window\n");
}

void CWindow::OnMouseDrag(long lx, long ly)
{
	PyCallClassMemberFunc(m_poHandler, "OnMouseDrag", Py_BuildValue("(ii)", lx, ly));
}

void CWindow::OnMoveWindow(long lx, long ly)
{
	PyCallClassMemberFunc(m_poHandler, "OnMoveWindow", Py_BuildValue("(ii)", lx, ly));
}

void CWindow::OnSetFocus()
{
	//PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
	PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
}

void CWindow::OnKillFocus()
{
	PyCallClassMemberFunc(m_poHandler, "OnKillFocus", BuildEmptyTuple());
}

void CWindow::OnMouseOverIn()
{
	PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
}

void CWindow::OnMouseOverOut()
{
	PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
}

void CWindow::OnMouseOver() {}

void CWindow::OnDrop()
{
	PyCallClassMemberFunc(m_poHandler, "OnDrop", BuildEmptyTuple());
}

void CWindow::OnTop()
{
	PyCallClassMemberFunc(m_poHandler, "OnTop", BuildEmptyTuple());
}

void CWindow::OnIMEUpdate()
{
	PyCallClassMemberFunc(m_poHandler, "OnIMEUpdate", BuildEmptyTuple());
}

BOOL CWindow::RunIMETabEvent()
{
	if (!IsRendering())
		return FALSE;

	if (OnIMETabEvent())
		return TRUE;

	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->RunIMETabEvent())
			return TRUE;
	}

	return FALSE;
}

BOOL CWindow::RunIMEReturnEvent()
{
	if (!IsRendering())
		return FALSE;

	if (OnIMEReturnEvent())
		return TRUE;

	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->RunIMEReturnEvent())
			return TRUE;
	}

	return FALSE;
}

BOOL CWindow::RunIMEKeyDownEvent(int ikey)
{
	if (!IsRendering())
		return FALSE;

	if (OnIMEKeyDownEvent(ikey))
		return TRUE;

	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->RunIMEKeyDownEvent(ikey))
			return TRUE;
	}

	return FALSE;
}

CWindow * CWindow::RunKeyDownEvent(int ikey)
{
	if (OnKeyDown(ikey))
		return this;

	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->IsShow())
		{
			CWindow * pProcessedWindow = pWindow->RunKeyDownEvent(ikey);
			if (nullptr != pProcessedWindow)
				return pProcessedWindow;
		}
	}

	return nullptr;
}

BOOL CWindow::RunKeyUpEvent(int ikey)
{
	if (OnKeyUp(ikey))
		return TRUE;

	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->IsShow())
		{
			if (pWindow->RunKeyUpEvent(ikey))
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CWindow::RunPressEscapeKeyEvent()
{
	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->IsShow())
			if (pWindow->RunPressEscapeKeyEvent())
				return TRUE;
	}

	if (OnPressEscapeKey())
		return TRUE;

	return FALSE;
}

BOOL CWindow::RunPressExitKeyEvent()
{
	for (auto itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
	{
		CWindow * pWindow = *itor;

		if (pWindow->RunPressExitKeyEvent())
			return TRUE;

		if (pWindow->IsShow())
			if (pWindow->OnPressExitKey())
				return TRUE;
	}

	return FALSE;
}

BOOL CWindow::OnMouseLeftButtonDown()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnMouseLeftButtonUp()
{
	PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonUp", BuildEmptyTuple());
	return TRUE; // NOTE : ButtonUp은 예외로 무조건 TRUE
}

BOOL CWindow::OnMouseLeftButtonDoubleClick()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnMouseRightButtonDown()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnMouseRightButtonUp()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnMouseRightButtonDoubleClick()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnMouseMiddleButtonDown()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnMouseMiddleButtonUp()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

#ifdef ENABLE_RUN_MOUSE_WHEEL_EVENT
BOOL CWindow::RunMouseWheelEvent(long nLen)
{
	bool bValue = false;
	if (PyCallClassMemberFunc(m_poHandler, "OnRunMouseWheel", Py_BuildValue("(l)", nLen), &bValue))
		return bValue;

	return bValue;
}
#endif

BOOL CWindow::OnIMETabEvent()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMETab", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMEReturnEvent()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMEReturn", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMEKeyDownEvent(int ikey)
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMEKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMEChangeCodePage()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMEChangeCodePage", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMEOpenCandidateListEvent()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMECloseCandidateListEvent()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMEOpenReadingWndEvent()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnIMECloseReadingWndEvent()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnKeyDown(int ikey)
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnKeyUp(int ikey)
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnKeyUp", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnPressEscapeKey()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnPressEscapeKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

BOOL CWindow::OnPressExitKey()
{
	long lValue;
	if (PyCallClassMemberFunc(m_poHandler, "OnPressExitKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

	return FALSE;
}

/////

bool CWindow::IsIn(long x, long y) const
{
	if (x >= m_rect.left && x <= m_rect.right)
		if (y >= m_rect.top && y <= m_rect.bottom)
			return true;

	return false;
}

bool CWindow::IsIn() const
{
	long lx, ly;
	CWindowManager::Instance().GetMousePosition(lx, ly);

	return IsIn(lx, ly);
}

CWindow * CWindow::PickWindow(long x, long y)
{
	auto ritor = m_pChildList.rbegin();
	for (; ritor != m_pChildList.rend(); ++ritor)
	{
		CWindow * pWin = *ritor;
		if (pWin->IsShow())
		{
			if (!pWin->IsFlag(FLAG_IGNORE_SIZE))
			{
				if (!pWin->IsIn(x, y))
				{
					if (0L <= pWin->GetWidth())
						continue;
				}
			}

			CWindow * pResult = pWin->PickWindow(x, y);
			if (pResult)
				return pResult;
		}
	}

	if (IsFlag(FLAG_NOT_PICK))
		return nullptr;

	return (this);
}

CWindow * CWindow::PickTopWindow(long x, long y)
{
	auto ritor = m_pChildList.rbegin();
	for (; ritor != m_pChildList.rend(); ++ritor)
	{
		CWindow * pWin = *ritor;
		if (pWin->IsShow())
		{
			if (pWin->IsIn(x, y))
			{
				if (!pWin->IsFlag(FLAG_NOT_PICK))
					return pWin;
			}
		}
	}

	return nullptr;
}

#ifdef ENABLE_INGAME_WIKI
void CWindow::iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom)
{
	m_renderingRect.left = iLeft;
	m_renderingRect.top = iTop;
	m_renderingRect.right = iRight;
	m_renderingRect.bottom = iBottom;
	OnSetRenderingRect();
}

void CWindow::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
	float fWidth = float(GetWidth());
	float fHeight = float(GetHeight());
	if (IsType(CTextLine::Type()))
	{
		int iWidth, iHeight;
		((CTextLine*)this)->GetTextSize(&iWidth, &iHeight);
		fWidth = float(iWidth);
		fHeight = float(iHeight);
	}

	iSetRenderingRect(fWidth * fLeft, fHeight * fTop, fWidth * fRight, fHeight * fBottom);
}

int CWindow::GetRenderingWidth()
{
	return std::max<size_t>(0, GetWidth() + m_renderingRect.right + m_renderingRect.left);
}

int CWindow::GetRenderingHeight()
{
	return std::max<size_t>(0, GetHeight() + m_renderingRect.bottom + m_renderingRect.top);
}

void CWindow::ResetRenderingRect(bool bCallEvent)
{
	m_renderingRect.bottom = m_renderingRect.left = m_renderingRect.right = m_renderingRect.top = 0;

	if (bCallEvent)
		OnSetRenderingRect();
}

void CWindow::OnSetRenderingRect() {}
#endif

#ifdef ENABLE_RENDER_TARGET
CUiRenderTarget::CUiRenderTarget(PyObject* ppyObject) : CWindow(ppyObject)
{
	m_dwIndex = -1;
}
CUiRenderTarget::~CUiRenderTarget() = default;

bool CUiRenderTarget::SetRenderTarget(uint8_t index)
{
	if (!CRenderTargetManager::Instance().GetRenderTarget(index))
	{
		if (!CRenderTargetManager::Instance().CreateRenderTarget(index, GetWidth(), GetHeight()))
		{
			TraceError("CRenderTargetManager could not create the texture. w %d h %d", GetWidth(), GetHeight());
			return false;
		}
	}
	m_dwIndex = index;

	UpdateRect();
	return true;
}

void CUiRenderTarget::OnRender()
{
	auto target = CRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
	if (!target)
	{
		TraceError("SetRenderingRect -> target empty!");
		return;
	}

	target->SetRenderingRect(&m_rect);

	target->RenderTexture();
}
#endif

#ifdef ENABLE_INGAME_WIKI
CUiWikiRenderTarget::~CUiWikiRenderTarget() = default;
CUiWikiRenderTarget::CUiWikiRenderTarget(PyObject * ppyObject) :
	CWindow(ppyObject),
	m_dwIndex(-1) {}

/*----------------------------
--------PUBLIC CLASS FUNCTIONS
-----------------------------*/
bool CUiWikiRenderTarget::SetWikiRenderTargetModule(int iRenderTargetModule)
{
	if (!CWikiRenderTargetManager::Instance().GetRenderTarget(iRenderTargetModule))
	{
		if (!CWikiRenderTargetManager::Instance().CreateRenderTarget(iRenderTargetModule, GetWidth(), GetHeight()))
		{
			TraceError("CWikiRenderTargetManager could not create the texture. w %d h %d", GetWidth(), GetHeight());
			return false;
		}
	}

	m_dwIndex = iRenderTargetModule;

	UpdateRect();
	return true;
}

void CUiWikiRenderTarget::OnUpdateRenderBox()
{
	if (m_dwIndex == -1 /*(CPythonWikiRenderTarget::START_MODULE)*/)
		return;

	auto target = CWikiRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
	if (!target)
		return;

	target->SetRenderingBox(&m_renderBox);
}

/*----------------------------
-----PROTECTED CLASS FUNCTIONS
-----------------------------*/
void CUiWikiRenderTarget::OnRender()
{
	if (m_dwIndex == -1 /*(CPythonWikiRenderTarget::START_MODULE)*/)
		return;

	auto target = CWikiRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
	if (!target)
		return;

	target->SetRenderingRect(&m_rect);
	target->RenderTexture();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CBox::CBox(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000) {}
CBox::~CBox() = default;

void CBox::SetColor(uint32_t dwColor)
{
	m_dwColor = dwColor;
}

void CBox::OnRender()
{
	CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
	CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CBar::CBar(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000) {}
CBar::~CBar() = default;

void CBar::SetColor(uint32_t dwColor)
{
	m_dwColor = dwColor;
}

void CBar::OnRender()
{
	CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
#ifdef ENABLE_INGAME_WIKI
	CPythonGraphic::Instance().RenderBar2d(m_rect.left + m_renderBox.left, m_rect.top + m_renderBox.top, m_rect.right - m_renderBox.right, m_rect.bottom - m_renderBox.bottom);
#else
	CPythonGraphic::Instance().RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CLine::CLine(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000) {}
CLine::~CLine() = default;

void CLine::SetColor(uint32_t dwColor)
{
	m_dwColor = dwColor;
}

void CLine::OnRender()
{
	CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();
	rkpyGraphic.SetDiffuseColor(m_dwColor);
	rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

uint32_t CBar3D::Type()
{
	static uint32_t s_dwType = GetCRC32("CBar3D", strlen("CBar3D"));
	return (s_dwType);
}

CBar3D::CBar3D(PyObject * ppyObject) : CWindow(ppyObject)
{
	m_dwLeftColor = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	m_dwRightColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
	m_dwCenterColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
}
CBar3D::~CBar3D() = default;

void CBar3D::SetColor(uint32_t dwLeft, uint32_t dwRight, uint32_t dwCenter)
{
	m_dwLeftColor = dwLeft;
	m_dwRightColor = dwRight;
	m_dwCenterColor = dwCenter;
}

void CBar3D::OnRender()
{
	CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();

	rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
	rkpyGraphic.RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

	rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
	rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.top);
	rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom);

	rkpyGraphic.SetDiffuseColor(m_dwRightColor);
	rkpyGraphic.RenderLine2d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom);
	rkpyGraphic.RenderLine2d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CTextLine::CTextLine(PyObject * ppyObject) : CWindow(ppyObject)
{
	m_TextInstance.SetColor(0.78f, 0.78f, 0.78f);
	m_TextInstance.SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
	m_TextInstance.SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_TOP);
#ifdef ENABLE_OFFICAL_FEATURES
	m_bWindowType = WINDOW_TYPE_TEXTLINE;
#endif
}
CTextLine::~CTextLine()
{
	m_TextInstance.Destroy();
}

#ifdef ENABLE_INGAME_WIKI
uint32_t CTextLine::Type()
{
	static uint32_t s_dwType = GetCRC32("CTextLine", strlen("CTextLine"));
	return (s_dwType);
}
#endif

void CTextLine::SetMax(int iMax)
{
	m_TextInstance.SetMax(iMax);
}
void CTextLine::SetHorizontalAlign(int iType)
{
	m_TextInstance.SetHorizonalAlign(iType);
}
void CTextLine::SetVerticalAlign(int iType)
{
	m_TextInstance.SetVerticalAlign(iType);
}
void CTextLine::SetSecret(BOOL bFlag)
{
	m_TextInstance.SetSecret(bFlag != 0);
}
void CTextLine::SetOutline(BOOL bFlag)
{
	m_TextInstance.SetOutline(bFlag != 0);
}
void CTextLine::SetFeather(BOOL bFlag)
{
	m_TextInstance.SetFeather(bFlag != 0);
}
void CTextLine::SetMultiLine(BOOL bFlag)
{
	m_TextInstance.SetMultiLine(bFlag != 0);
}
void CTextLine::SetFontName(const char * c_szFontName)
{
	std::string stFontName = c_szFontName;
	stFontName += ".fnt";

	CResourceManager & rkResMgr = CResourceManager::Instance();
	CResource * pkRes = rkResMgr.GetTypeResourcePointer(stFontName.c_str());
	auto * pkResFont = msl::inherit_cast<CGraphicText *>(pkRes);
	m_TextInstance.SetTextPointer(pkResFont);
}
void CTextLine::SetFontColor(uint32_t dwColor)
{
	m_TextInstance.SetColor(dwColor);
}
void CTextLine::SetLimitWidth(float fWidth)
{
	m_TextInstance.SetLimitWidth(fWidth);
}
void CTextLine::SetText(const char * c_szText)
{
	OnSetText(c_szText);
}
void CTextLine::GetTextSize(int * pnWidth, int * pnHeight)
{
	m_TextInstance.GetTextSize(pnWidth, pnHeight);
}
const char * CTextLine::GetText()
{
	return m_TextInstance.GetValueStringReference().c_str();
}
#ifdef ENABLE_SUNG_MAHI_TOWER
void CTextLine::GetCharSize(short* sWidth)
{
	m_TextInstance.GetCharacterWidth(sWidth);
}
#endif
#ifdef WJ_MULTI_TEXTLINE
void CTextLine::SetLineHeight(uint16_t iLineHeight)
{
	m_TextInstance.SetLineHeight(iLineHeight);
}
uint16_t CTextLine::GetLineHeight()
{
	return m_TextInstance.GetLineHeight();
}

uint16_t CTextLine::GetTextLineCount()
{
	return m_TextInstance.GetTextLineCount();
}
void CTextLine::DisableEnterToken()
{
	m_TextInstance.DisableEnterToken();
}
#endif
void CTextLine::ShowCursor()
{
	m_TextInstance.ShowCursor();
}
bool CTextLine::IsShowCursor()
{
	return m_TextInstance.IsShowCursor();
}
void CTextLine::HideCursor()
{
	m_TextInstance.HideCursor();
}
int CTextLine::GetCursorPosition()
{
	long lx, ly;
	GetMouseLocalPosition(lx, ly);
	return m_TextInstance.PixelPositionToCharacterPosition(lx);
}

void CTextLine::OnSetText(const char * c_szText)
{
	m_TextInstance.SetValue(c_szText);
	m_TextInstance.Update();

#ifdef ENABLE_INGAME_WIKI
	if (m_isInsideRender)
		UpdateRenderBoxRecursive();
#endif
}

#ifdef ENABLE_INGAME_WIKI
bool CTextLine::IsShow()
{
	if (!m_bShow)
		return false;

	if (m_isInsideRender)
	{
		int cW, cH;
		GetTextSize(&cW, &cH);
		if (m_renderBox.left + m_renderBox.right >= cW || m_renderBox.top + m_renderBox.bottom >= cH)
			return false;
	}

	return true;
}
#endif

void CTextLine::OnUpdate()
{
	if (IsShow())
		m_TextInstance.Update();
}
void CTextLine::OnRender()
{
	if (IsShow())
		m_TextInstance.Render();
}

#ifdef ENABLE_INGAME_WIKI
int CTextLine::GetRenderingWidth()
{
	int iTextWidth, iTextHeight;
	GetTextSize(&iTextWidth, &iTextHeight);

	return iTextWidth + m_renderingRect.right + m_renderingRect.left;
}

int CTextLine::GetRenderingHeight()
{
	int iTextWidth, iTextHeight;
	GetTextSize(&iTextWidth, &iTextHeight);

	return iTextHeight + m_renderingRect.bottom + m_renderingRect.top;
}

void CTextLine::OnSetRenderingRect()
{
	int iTextWidth, iTextHeight;
	GetTextSize(&iTextWidth, &iTextHeight);

	m_TextInstance.iSetRenderingRect(m_renderingRect.left, -m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
}
#endif

void CTextLine::OnChangePosition()
{
	// FOR_ARABIC_ALIGN
	//if (m_TextInstance.GetHorizontalAlign() == CGraphicTextInstance::HORIZONTAL_ALIGN_ARABIC)
	if (GetDefaultCodePage() == CP_ARABIC)
		m_TextInstance.SetPosition(m_rect.right, m_rect.top);
	else
		m_TextInstance.SetPosition(m_rect.left, m_rect.top);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CNumberLine::CNumberLine(PyObject * ppyObject) : CWindow(ppyObject)
{
	m_strPath = "d:/ymir work/ui/game/taskbar/";
	m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
	m_dwWidthSummary = 0;
}
CNumberLine::CNumberLine(CWindow * pParent) : CWindow(nullptr)
{
	m_strPath = "d:/ymir work/ui/game/taskbar/";
	m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
	m_dwWidthSummary = 0;

	m_pParent = pParent;
}
CNumberLine::~CNumberLine()
{
	ClearNumber();
}

void CNumberLine::SetPath(const char * c_szPath)
{
	m_strPath = c_szPath;
}
void CNumberLine::SetHorizontalAlign(int iType)
{
	m_iHorizontalAlign = iType;
}
void CNumberLine::SetNumber(const char * c_szNumber)
{
	if (c_szNumber == m_strNumber)
		return;

	ClearNumber();

	m_strNumber = c_szNumber;

	for (auto & cChar : m_strNumber)
	{
		std::string strImageFileName;

		if (':' == cChar)
			strImageFileName = m_strPath + "colon.sub";
		else if ('?' == cChar)
			strImageFileName = m_strPath + "questionmark.sub";
		else if ('/' == cChar)
			strImageFileName = m_strPath + "slash.sub";
		else if ('%' == cChar)
			strImageFileName = m_strPath + "percent.sub";
		else if ('+' == cChar)
		{
/*#ifdef ENABLE_PLUS_ICON_ITEM
			strImageFileName = m_strPath + "infobtn.tga";
#else*/
			strImageFileName = m_strPath + "plus.sub";
/*#endif*/
		}
		else if ('m' == cChar)
			strImageFileName = m_strPath + "m.sub";
		else if ('g' == cChar)
			strImageFileName = m_strPath + "g.sub";
		else if ('p' == cChar)
			strImageFileName = m_strPath + "p.sub";
		else if (cChar >= '0' && cChar <= '9')
		{
			strImageFileName = m_strPath;
			strImageFileName += cChar;
			strImageFileName += ".sub";
		}
		else
			continue;

		if (!CResourceManager::Instance().IsFileExist(strImageFileName.c_str()))
			continue;

		auto * pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(strImageFileName.c_str()));

		CGraphicImageInstance * pInstance = CGraphicImageInstance::New();
		pInstance->SetImagePointer(pImage);
		m_ImageInstanceVector.emplace_back(pInstance);
#ifdef ENABLE_12ZI
		if (m_pParent)
			OnChangePosition();
#endif

		m_dwWidthSummary += pInstance->GetWidth();
	}
}

void CNumberLine::ClearNumber()
{
	m_ImageInstanceVector.clear();
	m_dwWidthSummary = 0;
	m_strNumber = "";
}

void CNumberLine::OnRender()
{
	for (auto & i : m_ImageInstanceVector)
		i->Render();
}

void CNumberLine::OnChangePosition()
{
	int ix = m_x;
	int iy = m_y;

	if (m_pParent)
	{
		ix = m_rect.left;
		iy = m_rect.top;
	}

	switch (m_iHorizontalAlign)
	{
	case HORIZONTAL_ALIGN_LEFT:
		break;
	case HORIZONTAL_ALIGN_CENTER:
		ix -= int(m_dwWidthSummary) / 2;
		break;
	case HORIZONTAL_ALIGN_RIGHT:
		ix -= int(m_dwWidthSummary);
		break;
	}

	for (auto & i : m_ImageInstanceVector)
	{
		i->SetPosition(ix, iy);
		ix += i->GetWidth();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CImageBox::CImageBox(PyObject * ppyObject) : CWindow(ppyObject)
#ifdef ENABLE_12ZI
	, m_fCoolTime(0.0f), m_fCoolTimeStart(0.0f)
#endif
{
	m_pImageInstance = nullptr;
}
CImageBox::~CImageBox()
{
	CImageBox::OnDestroyInstance();
}

void CImageBox::OnCreateInstance()
{
	OnDestroyInstance();

	m_pImageInstance = CGraphicImageInstance::New();
}
void CImageBox::OnDestroyInstance()
{
	if (m_pImageInstance)
	{
		CGraphicImageInstance::Delete(m_pImageInstance);
		m_pImageInstance = nullptr;
	}
}

BOOL CImageBox::LoadImage(const char * c_szFileName)
{
	if (!c_szFileName[0])
		return FALSE;

	OnCreateInstance();

	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource)
		return FALSE;
	if (!pResource->IsType(CGraphicImage::Type()))
		return FALSE;

	m_pImageInstance->SetImagePointer(msl::inherit_cast<CGraphicImage *>(pResource));
	if (m_pImageInstance->IsEmpty())
		return FALSE;

	SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
	UpdateRect();

	return TRUE;
}

#if defined(ENABLE_IMAGE_SCALE) || defined(ENABLE_GROWTH_PET_SYSTEM)
void CImageBox::SetScale(float sx, float sy)
{
	if (!m_pImageInstance)
		return;

	((CGraphicImageInstance*)m_pImageInstance)->SetScale(sx, sy);
	CWindow::SetSize(long(float(GetWidth()) * sx), long(float(GetHeight()) * sy));
}
#endif

void CImageBox::SetDiffuseColor(float fr, float fg, float fb, float fa) const
{
	if (!m_pImageInstance)
		return;

	m_pImageInstance->SetDiffuseColor(fr, fg, fb, fa);
}

int CImageBox::GetWidth() const
{
	if (!m_pImageInstance)
		return 0;

	return m_pImageInstance->GetWidth();
}

int CImageBox::GetHeight() const
{
	if (!m_pImageInstance)
		return 0;

	return m_pImageInstance->GetHeight();
}

void CImageBox::OnUpdate() {}
void CImageBox::OnRender()
{
	if (!m_pImageInstance)
		return;

#ifdef ENABLE_12ZI
	if (IsShow())
	{
		if (m_fCoolTime == 0.0f)
			m_pImageInstance->Render();
		else
			m_pImageInstance->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
	}
#else
	if (IsShow())
		m_pImageInstance->Render();
#endif
}

#ifdef ENABLE_OFFICAL_FEATURES
void CImageBox::LeftRightReverse()
{
	if (!m_pImageInstance)
		return;

	m_pImageInstance->LeftRightReverse();
}
#endif

void CImageBox::OnChangePosition()
{
	if (!m_pImageInstance)
		return;

	m_pImageInstance->SetPosition(m_rect.left, m_rect.top);
}

#ifdef ENABLE_12ZI
void CImageBox::SetCoolTime(float fCoolTime)
{
	m_fCoolTime = fCoolTime;
}

void CImageBox::SetCoolTimeStart(float fCoolTimeStart)
{
	m_fCoolTimeStart = fCoolTimeStart;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// MarkBox - 마크 출력용 UI 윈도우
///////////////////////////////////////////////////////////////////////////////////////////////
CMarkBox::CMarkBox(PyObject * ppyObject) : CWindow(ppyObject)
{
	m_pMarkInstance = nullptr;
}

CMarkBox::~CMarkBox()
{
	CMarkBox::OnDestroyInstance();
}

void CMarkBox::OnCreateInstance()
{
	OnDestroyInstance();
	m_pMarkInstance = CGraphicMarkInstance::New();
}

void CMarkBox::OnDestroyInstance()
{
	if (m_pMarkInstance)
	{
		CGraphicMarkInstance::Delete(m_pMarkInstance);
		m_pMarkInstance = nullptr;
	}
}

void CMarkBox::LoadImage(const char * c_szFilename)
{
	OnCreateInstance();

	m_pMarkInstance->SetImageFileName(c_szFilename);
	m_pMarkInstance->Load();
	SetSize(m_pMarkInstance->GetWidth(), m_pMarkInstance->GetHeight());

	UpdateRect();
}

void CMarkBox::SetScale(FLOAT fScale) const
{
	if (!m_pMarkInstance)
		return;

	m_pMarkInstance->SetScale(fScale);
}

void CMarkBox::SetIndex(uint32_t uIndex) const
{
	if (!m_pMarkInstance)
		return;

	m_pMarkInstance->SetIndex(uIndex);
}

void CMarkBox::SetDiffuseColor(float fr, float fg, float fb, float fa) const
{
	if (!m_pMarkInstance)
		return;

	m_pMarkInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CMarkBox::OnUpdate() {}

void CMarkBox::OnRender()
{
	if (!m_pMarkInstance)
		return;

	if (IsShow())
		m_pMarkInstance->Render();
}

void CMarkBox::OnChangePosition()
{
	if (!m_pMarkInstance)
		return;

	m_pMarkInstance->SetPosition(m_rect.left, m_rect.top);
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

uint32_t CExpandedImageBox::Type()
{
	static uint32_t s_dwType = GetCRC32("CExpandedImageBox", strlen("CExpandedImageBox"));
	return (s_dwType);
}

BOOL CExpandedImageBox::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return TRUE;

	return FALSE;
}

CExpandedImageBox::CExpandedImageBox(PyObject * ppyObject) : CImageBox(ppyObject) {}
CExpandedImageBox::~CExpandedImageBox()
{
	CExpandedImageBox::OnDestroyInstance();
}

void CExpandedImageBox::OnCreateInstance()
{
	OnDestroyInstance();

	m_pImageInstance = CGraphicExpandedImageInstance::New();
}
void CExpandedImageBox::OnDestroyInstance()
{
	if (m_pImageInstance)
	{
		CGraphicExpandedImageInstance::Delete(msl::inherit_cast<CGraphicExpandedImageInstance *>(m_pImageInstance));
		m_pImageInstance = nullptr;
	}
}

void CExpandedImageBox::SetScale(float fx, float fy)
{
	if (!m_pImageInstance)
		return;

	msl::inherit_cast<CGraphicExpandedImageInstance *>(m_pImageInstance)->SetScale(fx, fy);
	SetSize(long(float(GetWidth()) * fx), long(float(GetHeight()) * fy));
}
void CExpandedImageBox::SetOrigin(float fx, float fy) const
{
	if (!m_pImageInstance)
		return;

	msl::inherit_cast<CGraphicExpandedImageInstance *>(m_pImageInstance)->SetOrigin(fx, fy);
}
void CExpandedImageBox::SetRotation(float fRotation) const
{
	if (!m_pImageInstance)
		return;

	msl::inherit_cast<CGraphicExpandedImageInstance *>(m_pImageInstance)->SetRotation(fRotation);
}
void CExpandedImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom) const
{
	if (!m_pImageInstance)
		return;

	msl::inherit_cast<CGraphicExpandedImageInstance *>(m_pImageInstance)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CExpandedImageBox::SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom)
{
	if (!m_pImageInstance)
		return;

	((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
}
#endif

#if defined(ENABLE_IMAGE_CLIP_RECT) || defined(ENABLE_BATTLE_PASS_SYSTEM)
void CExpandedImageBox::SetImageClipRect(float fLeft, float fTop, float fRight, float fBottom, bool bIsVertical)
{
	if (!m_pImageInstance)
		return;

	const RECT& c_rRect = GetRect();

	const float fDifLeft = (c_rRect.left < fLeft) ? -(float(fLeft - c_rRect.left) / float(GetWidth())) : 0.0f;
	const float fDifTop = (c_rRect.top < fTop) ? -(float(fTop - c_rRect.top) / float(GetHeight())) : 0.0f;
	const float fDifRight = (c_rRect.right > fRight) ? -(float(c_rRect.right - fRight) / float(GetWidth())) : 0.0f;
	const float fDifBottom = (c_rRect.bottom > fBottom) ? -(float(c_rRect.bottom - fBottom) / float(GetHeight())) : 0.0f;

	if (bIsVertical)
		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fDifTop, fRight, fDifBottom);
	else
		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fDifLeft, fDifTop, fDifRight, fDifBottom);
}
#endif

void CExpandedImageBox::SetRenderingMode(int iMode) const
{
	msl::inherit_cast<CGraphicExpandedImageInstance *>(m_pImageInstance)->SetRenderingMode(iMode);
}

#ifdef ENABLE_INGAME_WIKI
int CExpandedImageBox::GetRenderingWidth()
{
	return CWindow::GetWidth() + m_renderingRect.right + m_renderingRect.left;
}

int CExpandedImageBox::GetRenderingHeight()
{
	return CWindow::GetHeight() + m_renderingRect.bottom + m_renderingRect.top;
}

void CExpandedImageBox::OnSetRenderingRect()
{
	if (!m_pImageInstance)
		return;

	((CGraphicExpandedImageInstance*)m_pImageInstance)->iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
}

void CExpandedImageBox::SetExpandedRenderingRect(float fLeftTop, float fLeftBottom, float fTopLeft, float fTopRight, float fRightTop, float fRightBottom, float fBottomLeft, float fBottomRight)
{
	if (!m_pImageInstance)
		return;

	((CGraphicExpandedImageInstance*)m_pImageInstance)->SetExpandedRenderingRect(fLeftTop, fLeftBottom, fTopLeft, fTopRight, fRightTop, fRightBottom, fBottomLeft, fBottomRight);
}

void CExpandedImageBox::SetTextureRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
	if (!m_pImageInstance)
		return;

	((CGraphicExpandedImageInstance*)m_pImageInstance)->SetTextureRenderingRect(fLeft, fTop, fRight, fBottom);
}

uint32_t CExpandedImageBox::GetPixelColor(uint32_t x, uint32_t y)
{
	return ((CGraphicExpandedImageInstance*)m_pImageInstance)->GetPixelColor(x, y);
}

void CExpandedImageBox::OnUpdateRenderBox()
{
	if (!m_pImageInstance)
		return;

	((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderBox(m_renderBox);
}
#endif
void CExpandedImageBox::OnUpdate() {}
void CExpandedImageBox::OnRender()
{
	if (!m_pImageInstance)
		return;

#ifdef ENABLE_12ZI
	if (IsShow())
	{
		if (m_fCoolTime == 0.0f)
			m_pImageInstance->Render();
		else
			((CGraphicExpandedImageInstance*)m_pImageInstance)->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
	}
#else
	if (IsShow())
		m_pImageInstance->Render();
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

uint32_t CAniImageBox::Type()
{
	static uint32_t s_dwType = GetCRC32("CAniImageBox", strlen("CAniImageBox"));
	return (s_dwType);
}

BOOL CAniImageBox::OnIsType(uint32_t dwType)
{
	if (Type() == dwType)
		return TRUE;

	return FALSE;
}

CAniImageBox::CAniImageBox(PyObject * ppyObject) : CWindow(ppyObject), m_bycurDelay(0), m_byDelay(4), m_bycurIndex(0)
#ifdef ENABLE_FISHING_RENEWAL
, m_RotationProcess(false)
#endif
{
	m_ImageVector.clear();
}
CAniImageBox::~CAniImageBox()
{
	for_each(m_ImageVector.begin(), m_ImageVector.end(), CGraphicExpandedImageInstance::DeleteExpandedImageInstance);
}

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
struct FSetDiffuseColor
{
	float fr, fg, fb, fa;
	void operator () (CGraphicExpandedImageInstance* pInstance)
	{
		pInstance->SetDiffuseColor(fr, fg, fb, fa);
	}
};

void CAniImageBox::SetDiffuseColor(float fR, float fG, float fB, float fA)
{
	FSetDiffuseColor setDiffuseColor;
	setDiffuseColor.fr = fR;
	setDiffuseColor.fg = fG;
	setDiffuseColor.fb = fB;
	setDiffuseColor.fa = fA;
	for_each(m_ImageVector.begin(), m_ImageVector.end(), setDiffuseColor);
}

void CAniImageBox::SetDiffuseColorByType(uint8_t byColorType)
{
	float fr = 1.0f, fg = 1.0f, fb = 1.0f, fa = 1.0f;
	switch (byColorType)
	{
		case COLOR_TYPE_ORANGE:
			fr = 1.0f; fg = 0.34509805f; fb = 0.035294119f; fa = 0.5f;
			break;
		case COLOR_TYPE_WHITE:
			fr = 1.0f; fg = 1.0f; fb = 1.0f; fa = 0.5f;
			break;
		case COLOR_TYPE_RED:
			fr = 1.0f; fg = 0.0f; fb = 0.0f; fa = 0.5f;
			break;
		case COLOR_TYPE_GREEN:
			fr = 0.0f; fg = 1.0f; fb = 0.0f; fa = 0.5f;
			break;
		case COLOR_TYPE_YELLOW:
			fr = 1.0f; fg = 1.0f; fb = 0.0f; fa = 0.5f;
			break;
		case COLOR_TYPE_SKY:
			fr = 0.0f; fg = 1.0f; fb = 1.0f; fa = 0.5f;
			break;
		case COLOR_TYPE_PINK:
			fr = 1.0f; fg = 0.0f; fb = 1.0f; fa = 0.5f;
			break;
		case COLOR_TYPE_BLUE:
			fr = 0.0f; fg = 0.0f; fb = 1.0f; fa = 0.5f;
		case COLOR_TYPE_PURPLE:
			fr = 1.0f; fg = 0.2f; fb = 1.0f; fa = 0.5f;
			break;

		default:
			return;
			break;
	}

	SetDiffuseColor(fr, fg, fb, fa);
}
#endif

void CAniImageBox::SetDelay(int iDelay)
{
	m_byDelay = iDelay;
}

void CAniImageBox::AppendImage(const char * c_szFileName)
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource->IsType(CGraphicImage::Type()))
		return;

	CGraphicExpandedImageInstance * pImageInstance = CGraphicExpandedImageInstance::New();

	pImageInstance->SetImagePointer(msl::inherit_cast<CGraphicImage *>(pResource));
	if (pImageInstance->IsEmpty())
	{
		CGraphicExpandedImageInstance::Delete(pImageInstance);
		return;
	}

	m_ImageVector.emplace_back(pImageInstance);

	m_bycurIndex = msl::truncate_cast<uint8_t>(rand() % m_ImageVector.size());
}

struct FSetRenderingRect
{
	float fLeft, fTop, fRight, fBottom;
	void operator()(CGraphicExpandedImageInstance * pInstance) const { pInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom); }
};
void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
	FSetRenderingRect setRenderingRect;
	setRenderingRect.fLeft = fLeft;
	setRenderingRect.fTop = fTop;
	setRenderingRect.fRight = fRight;
	setRenderingRect.fBottom = fBottom;
	for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingRect);
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
struct FSetRenderingRectWithScale
{
	float fLeft, fTop, fRight, fBottom;
	void operator () (CGraphicExpandedImageInstance* pInstance)
	{
		pInstance->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
	}
};
void CAniImageBox::SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom)
{
	FSetRenderingRectWithScale setRenderingRect;
	setRenderingRect.fLeft = fLeft;
	setRenderingRect.fTop = fTop;
	setRenderingRect.fRight = fRight;
	setRenderingRect.fBottom = fBottom;
	for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingRect);
}
#endif

struct FSetRenderingMode
{
	int iMode;
	void operator()(CGraphicExpandedImageInstance * pInstance) const { pInstance->SetRenderingMode(iMode); }
};
void CAniImageBox::SetRenderingMode(int iMode)
{
	FSetRenderingMode setRenderingMode;
	setRenderingMode.iMode = iMode;
	for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingMode);
}

void CAniImageBox::ResetFrame()
{
	m_bycurIndex = 0;
}

#ifdef ENABLE_FISHING_RENEWAL
void CAniImageBox::SetRotation(float fRotation)
{
	const int iSize = m_ImageVector.size();
	if (iSize > 0)
	{
		m_RotationProcess = true;
		for (int i = 0; i < iSize; i++)
			m_ImageVector[i]->SetRotation(fRotation);
	}

	m_RotationProcess = false;
}
#endif

void CAniImageBox::OnUpdate()
{
#ifdef ENABLE_FISHING_RENEWAL
	if (m_RotationProcess)
		return;
#endif

	++m_bycurDelay;
	if (m_bycurDelay < m_byDelay)
		return;

	m_bycurDelay = 0;

	++m_bycurIndex;
	if (m_bycurIndex >= m_ImageVector.size())
	{
		m_bycurIndex = 0;

		OnEndFrame();
	}
}
void CAniImageBox::OnRender()
{
#ifdef ENABLE_FISHING_RENEWAL
	if (m_RotationProcess)
		return;
#endif

	if (m_bycurIndex < m_ImageVector.size())
	{
		CGraphicExpandedImageInstance * pImage = m_ImageVector[m_bycurIndex];
		pImage->Render();
	}
}

struct FChangePosition
{
	float fx, fy;
	void operator()(CGraphicExpandedImageInstance * pInstance) const { pInstance->SetPosition(fx, fy); }
};

void CAniImageBox::OnChangePosition()
{
	FChangePosition changePosition;
	changePosition.fx = m_rect.left;
	changePosition.fy = m_rect.top;
	for_each(m_ImageVector.begin(), m_ImageVector.end(), changePosition);
}

void CAniImageBox::OnEndFrame()
{
	PyCallClassMemberFunc(m_poHandler, "OnEndFrame", BuildEmptyTuple());
}

#ifdef ENABLE_FISHING_RENEWAL
	uint32_t CFishBox::Type()
	{
		static uint32_t s_dwType = GetCRC32("CFishBox", strlen("CFishBox"));
		return (s_dwType);
	}

	BOOL CFishBox::OnIsType(uint32_t dwType)
	{
		if (CFishBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CFishBox::CFishBox(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_v2SrcPos(0.0f, 0.0f),
			m_v2DstPos(0.0f, 0.0f),
			m_v2NextPos(0.0f, 0.0f),
			m_v2Direction(0.0f, 0.0f),
			m_v2NextDistance(0.0f, 0.0f),
			m_fDistance(0.0f),
			m_fMoveSpeed(3.0f),
			m_bIsMove(false),
			m_left(false),
			m_right(false),
			m_lastRandom(0),
			m_direction(0) {
	}

	CFishBox::~CFishBox() {}

	void CFishBox::OnEndMove()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple());
		SetRandomPosition();
	}

	void CFishBox::SetRandomPosition()
	{
		m_v2Direction.x = 0.0f;
		m_v2Direction.y = 0.0f;

		if (m_left == false)
		{
			m_v2DstPos.x = listFishLeft[m_lastRandom][0];
			m_v2DstPos.y = listFishLeft[m_lastRandom][1];
			m_direction = listFishLeft[m_lastRandom][2];
		}
		else
		{
			m_v2DstPos.x = listFishRight[m_lastRandom][0];
			m_v2DstPos.y = listFishRight[m_lastRandom][1];
			m_direction = listFishRight[m_lastRandom][2];
		}

		D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);
		m_fDistance = (m_v2Direction.x*m_v2Direction.x + m_v2Direction.y*m_v2Direction.y);
		D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

		m_bIsMove = true;
	}

	void CFishBox::MoveStart()
	{
		m_lastRandom = 0;
		m_bIsMove = false;
		m_left = false;
		m_right = false;
		m_v2SrcPos.x = m_rect.left;
		m_v2SrcPos.y = m_rect.top;
		SetRandomPosition();
	}

	void CFishBox::MoveStop()
	{
		m_bIsMove = false;
	}

	bool CFishBox::GetMove() {
		return m_bIsMove;
	}

	void CFishBox::GetPosition(int * x, int * y)
	{
		*x = m_v2NextPos.x;
		*y = m_v2NextPos.y;
	}

	void CFishBox::OnUpdate()
	{
		if (IsShow() && GetMove())
		{
			const auto vv1 = m_v2Direction * m_fMoveSpeed;
			D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &vv1);
			D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

			const float fNextDistance = (m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
			if (fNextDistance >= m_fDistance)
			{
				if (m_left == false)
				{
					m_left = true;
				}
				else if (m_right == false)
				{
					m_right = true;

					int r = 0;
					bool same = true;
					while (same)
					{
						r = rand() % MAX_FISHING_WAYS;
						if (r != m_lastRandom)
						{
							same = false;
							m_lastRandom = r;
						}
					}

					m_left = false;
					m_right = false;
				}

				m_v2SrcPos = m_v2NextPos;
				MoveStop();
				OnEndMove();
				SetRandomPosition();
				return;
			}

			SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			PyCallClassMemberFunc(m_poHandler, "OnRotation", Py_BuildValue("(f)", m_direction));
		}
	}
#endif

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
struct FSetScale
{
	float fx, fy;
	void operator () (CGraphicExpandedImageInstance* pInstance)
	{
		pInstance->SetScale(fx, fy);
	}
};

void CAniImageBox::SetScale(float fx, float fy)
{
	FSetScale setScale;
	setScale.fx = fx;
	setScale.fy = fy;
	for_each(m_ImageVector.begin(), m_ImageVector.end(), setScale);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CButton::CButton(PyObject * ppyObject) : CWindow(ppyObject),
									m_bEnable(TRUE),
									m_isPressed(FALSE),
									m_isFlash(FALSE),
#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM)
									m_isEnableFlash(FALSE),
#endif
#ifdef ENABLE_12ZI
									m_isAlwaysTooltip(FALSE), // Missing reverse
#endif
									m_pcurVisual(nullptr)

{
	AddFlag(FLAG_NOT_CAPTURE);
}
CButton::~CButton() = default;

BOOL CButton::SetUpVisual(const char * c_szFileName)
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource->IsType(CGraphicImage::Type()))
		return FALSE;

	m_upVisual.SetImagePointer(msl::inherit_cast<CGraphicImage *>(pResource));
	if (m_upVisual.IsEmpty())
		return FALSE;

	if (m_bScale)
		m_upVisual.SetScale(m_pScaleX, m_pScaleY);

	SetSize(m_upVisual.GetWidth()
		* m_pScaleX
		, m_upVisual.GetHeight()
		* m_pScaleY
	);
	//
	SetCurrentVisual(&m_upVisual);
	//

	return TRUE;
}
BOOL CButton::SetOverVisual(const char * c_szFileName)
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource->IsType(CGraphicImage::Type()))
		return FALSE;

	m_overVisual.SetImagePointer(msl::inherit_cast<CGraphicImage *>(pResource));
	if (m_overVisual.IsEmpty())
		return FALSE;

	if (m_bScale)
		m_overVisual.SetScale(m_pScaleX, m_pScaleY);

	SetSize(m_overVisual.GetWidth()
		* m_pScaleX
		, m_overVisual.GetHeight()
		* m_pScaleY
	);

	return TRUE;
}
BOOL CButton::SetDownVisual(const char * c_szFileName)
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource->IsType(CGraphicImage::Type()))
		return FALSE;

	m_downVisual.SetImagePointer(msl::inherit_cast<CGraphicImage *>(pResource));
	if (m_downVisual.IsEmpty())
		return FALSE;

	if (m_bScale)
		m_downVisual.SetScale(m_pScaleX, m_pScaleY);

	SetSize(m_downVisual.GetWidth()
		* m_pScaleX
		, m_downVisual.GetHeight()
		* m_pScaleY
	);

	return TRUE;
}
BOOL CButton::SetDisableVisual(const char * c_szFileName)
{
	CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
	if (!pResource->IsType(CGraphicImage::Type()))
		return FALSE;

	m_disableVisual.SetImagePointer(msl::inherit_cast<CGraphicImage *>(pResource));
	if (m_downVisual.IsEmpty())
		return FALSE;

	if (m_bScale)
		m_disableVisual.SetScale(m_pScaleX, m_pScaleY);

	SetSize(m_disableVisual.GetWidth()
		* m_pScaleX
		, m_disableVisual.GetHeight()
		* m_pScaleY
	);

	return TRUE;
}

const char * CButton::GetUpVisualFileName()
{
	return m_upVisual.GetGraphicImagePointer()->GetFileName();
}
const char * CButton::GetOverVisualFileName()
{
	return m_overVisual.GetGraphicImagePointer()->GetFileName();
}
const char * CButton::GetDownVisualFileName()
{
	return m_downVisual.GetGraphicImagePointer()->GetFileName();
}

void CButton::Flash()
{
	m_isFlash = TRUE;
}

#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MAILBOX)
void CButton::EnableFlash()
{
	m_isEnableFlash = TRUE;
}

void CButton::DisableFlash()
{
	m_isEnableFlash = FALSE;
}

#ifdef ENABLE_12ZI
void CButton::SetAlwaysToolTip(bool func)
{
	m_isAlwaysTooltip = func;
}
#endif
#endif

void CButton::Enable()
{
	SetUp();
	m_bEnable = TRUE;
}

void CButton::Disable()
{
	m_bEnable = FALSE;
	if (!m_disableVisual.IsEmpty())
		SetCurrentVisual(&m_disableVisual);
}

BOOL CButton::IsDisable() const
{
	return m_bEnable;
}

void CButton::SetUp()
{
	SetCurrentVisual(&m_upVisual);
	m_isPressed = FALSE;
}
void CButton::Up()
{
	if (IsIn())
		SetCurrentVisual(&m_overVisual);
	else
		SetCurrentVisual(&m_upVisual);

	PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
}
void CButton::Over()
{
	SetCurrentVisual(&m_overVisual);
}
void CButton::Down()
{
	m_isPressed = TRUE;
	SetCurrentVisual(&m_downVisual);
	PyCallClassMemberFunc(m_poHandler, "DownEvent", BuildEmptyTuple());
}

void CButton::OnUpdate() {}
void CButton::OnRender()
{
#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MAILBOX)
	if (!IsShow())
		return;

	if (m_pcurVisual)
	{
		if (m_isFlash && !IsIn())
		{
			if (int(timeGetTime() / 500) % 2)
				return;
		}
		else if (m_isEnableFlash && !IsIn())
		{
			if (int(timeGetTime() / 500) % 2)
			{
				SetCurrentVisual(&m_overVisual);
				m_pcurVisual->Render();
				return;
			}
			else
			{
				SetCurrentVisual(m_isPressed ? &m_downVisual : &m_upVisual);
				m_pcurVisual->Render();
				return;
			}
		}
		m_pcurVisual->Render();
	}

	PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
#else
	if (!IsShow())
		return;

	if (m_pcurVisual)
	{
		if (m_isFlash)
			if (!IsIn())
				if (int(timeGetTime() / 500) % 2)
					return;

		m_pcurVisual->Render();
	}

	PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
#endif
}
void CButton::OnChangePosition()
{
	if (m_pcurVisual)
		m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
}

BOOL CButton::OnMouseLeftButtonDown()
{
	if (!IsEnable())
		return TRUE;

	m_isPressed = TRUE;
	Down();

	return TRUE;
}
BOOL CButton::OnMouseLeftButtonDoubleClick()
{
	if (!IsEnable())
		return TRUE;

	OnMouseLeftButtonDown();

	return TRUE;
}
BOOL CButton::OnMouseLeftButtonUp()
{
	if (!IsEnable())
		return TRUE;
	if (!IsPressed())
		return TRUE;

	m_isPressed = FALSE;
	Up();

	return TRUE;
}
void CButton::OnMouseOverIn()
{
	if (!IsEnable())
		return;

	Over();
	PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
#if defined(ENABLE_DETAILS_UI) || defined(ENABLE_ACHIEVEMENT_SYSTEM)
	PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
#endif
}
void CButton::OnMouseOverOut()
{
	if (!IsEnable())
		return;

	SetUp();
	PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
#if defined(ENABLE_DETAILS_UI) || defined(ENABLE_ACHIEVEMENT_SYSTEM)
	PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
#endif
}

#ifdef ENABLE_INGAME_WIKI
void CButton::SetCurrentVisual(CGraphicExpandedImageInstance* pVisual)
#else
void CButton::SetCurrentVisual(CGraphicImageInstance * pVisual)
#endif
{
	m_pcurVisual = pVisual;
	m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

#ifdef ENABLE_INGAME_WIKI
	if (m_pcurVisual == &m_upVisual)
		PyCallClassMemberFunc(m_poHandler, "OnSetUpVisual", BuildEmptyTuple());
	else if (m_pcurVisual == &m_overVisual)
		PyCallClassMemberFunc(m_poHandler, "OnSetOverVisual", BuildEmptyTuple());
	else if (m_pcurVisual == &m_downVisual)
		PyCallClassMemberFunc(m_poHandler, "OnSetDownVisual", BuildEmptyTuple());
	else if (m_pcurVisual == &m_disableVisual)
		PyCallClassMemberFunc(m_poHandler, "OnSetDisableVisual", BuildEmptyTuple());
#endif
}

BOOL CButton::IsEnable() const
{
	return m_bEnable;
}

BOOL CButton::IsPressed() const
{
	return m_isPressed;
}

#ifdef ENABLE_INGAME_WIKI
void CButton::OnSetRenderingRect()
{
	m_upVisual.iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
	m_overVisual.iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
	m_downVisual.iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
	m_disableVisual.iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
}
#endif

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
void CButton::SetScale(float fx, float fy)
{
	if (fx < 0.0)
		fx = 1.0;
	if (fy < 0.0)
		fy = 1.0;

	m_bScale = TRUE;
	m_pScaleX = fx;
	m_pScaleY = fy;
}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
/*void CButton::SetScale(float fx, float fy)
{
	if (!m_upVisual.IsEmpty())
		m_upVisual.SetScale(fx, fy);

	if (!m_overVisual.IsEmpty())
		m_overVisual.SetScale(fx, fy);

	if (!m_downVisual.IsEmpty())
		m_downVisual.SetScale(fx, fy);

	if (!m_disableVisual.IsEmpty())
		m_disableVisual.SetScale(fx, fy);
}*/

long CButton::GetImageWidth()
{
	if (!m_upVisual.IsEmpty())
		return m_upVisual.GetWidth();

	if (!m_overVisual.IsEmpty())
		return m_overVisual.GetWidth();

	if (!m_downVisual.IsEmpty())
		return m_downVisual.GetWidth();

	if (!m_disableVisual.IsEmpty())
		return m_disableVisual.GetWidth();

	return 0;
}

long CButton::GetImageHeight()
{
	if (!m_upVisual.IsEmpty())
		return m_upVisual.GetHeight();

	if (!m_overVisual.IsEmpty())
		return m_overVisual.GetHeight();

	if (!m_downVisual.IsEmpty())
		return m_downVisual.GetHeight();

	if (!m_disableVisual.IsEmpty())
		return m_disableVisual.GetHeight();

	return 0;
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
void CButton::LeftRightReverse()
{
	m_upVisual.LeftRightReverse();
	m_overVisual.LeftRightReverse();
	m_downVisual.LeftRightReverse();
	m_disableVisual.LeftRightReverse();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CRadioButton::CRadioButton(PyObject * ppyObject) : CButton(ppyObject) {}
CRadioButton::~CRadioButton() = default;

BOOL CRadioButton::OnMouseLeftButtonDown()
{
	if (!IsEnable())
		return TRUE;

	if (!m_isPressed)
	{
		Down();
		PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
	}

	return TRUE;
}
BOOL CRadioButton::OnMouseLeftButtonUp()
{
	return TRUE;
}
void CRadioButton::OnMouseOverIn()
{
	if (!IsEnable())
		return;

	if (!m_isPressed)
		SetCurrentVisual(&m_overVisual);

	PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
}
void CRadioButton::OnMouseOverOut()
{
	if (!IsEnable())
		return;

	if (!m_isPressed)
		SetCurrentVisual(&m_upVisual);

	PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CToggleButton::CToggleButton(PyObject * ppyObject) : CButton(ppyObject) {}
CToggleButton::~CToggleButton() = default;

BOOL CToggleButton::OnMouseLeftButtonDown()
{
	if (!IsEnable())
		return TRUE;

	if (m_isPressed)
	{
		SetUp();
		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);
		PyCallClassMemberFunc(m_poHandler, "OnToggleUp", BuildEmptyTuple());
	}
	else
	{
		Down();
		PyCallClassMemberFunc(m_poHandler, "OnToggleDown", BuildEmptyTuple());
	}

	return TRUE;
}
BOOL CToggleButton::OnMouseLeftButtonUp()
{
	return TRUE;
}

void CToggleButton::OnMouseOverIn()
{
	if (!IsEnable())
		return;

	if (!m_isPressed)
		SetCurrentVisual(&m_overVisual);

	PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
}
void CToggleButton::OnMouseOverOut()
{
	if (!IsEnable())
		return;

	if (!m_isPressed)
		SetCurrentVisual(&m_upVisual);

	PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

CDragButton::CDragButton(PyObject * ppyObject) : CButton(ppyObject)
{
	RemoveFlag(FLAG_NOT_CAPTURE);
	m_restrictArea.left = 0;
	m_restrictArea.top = 0;
	m_restrictArea.right = CWindowManager::Instance().GetScreenWidth();
	m_restrictArea.bottom = CWindowManager::Instance().GetScreenHeight();
}
CDragButton::~CDragButton() = default;

void CDragButton::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
{
	m_restrictArea.left = ix;
	m_restrictArea.top = iy;
	m_restrictArea.right = ix + iwidth;
	m_restrictArea.bottom = iy + iheight;
}

void CDragButton::OnChangePosition()
{
	m_x = std::max(m_x, m_restrictArea.left);
	m_y = std::max(m_y, m_restrictArea.top);
	m_x = std::min(m_x, std::max(0l, m_restrictArea.right - m_lWidth));
	m_y = std::min(m_y, std::max(0l, m_restrictArea.bottom - m_lHeight));

	m_rect.left = m_x;
	m_rect.top = m_y;

	if (m_pParent)
	{
		const RECT & c_rRect = m_pParent->GetRect();
		m_rect.left += c_rRect.left;
		m_rect.top += c_rRect.top;
	}

	m_rect.right = m_rect.left + m_lWidth;
	m_rect.bottom = m_rect.top + m_lHeight;

	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

	if (m_pcurVisual)
		m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

	if (IsPressed())
		PyCallClassMemberFunc(m_poHandler, "OnMove", BuildEmptyTuple());
}

void CDragButton::OnMouseOverIn()
{
	if (!IsEnable())
		return;

	CButton::OnMouseOverIn();
	PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
}

void CDragButton::OnMouseOverOut()
{
	if (!IsEnable())
		return;

	CButton::OnMouseOverIn();
	PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
}

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_MINI_GAME_YUTNORI)
/// CMoveTextLine
CMoveTextLine::CMoveTextLine(PyObject * ppyObject) :
	CTextLine(ppyObject),
	m_v2SrcPos(0.0f, 0.0f),
	m_v2DstPos(0.0f, 0.0f),
	m_v2NextPos(0.0f, 0.0f),
	m_v2Direction(0.0f, 0.0f),
	m_v2NextDistance(0.0f, 0.0f),
	m_fDistance(0.0f),
	m_fMoveSpeed(10.0f),
	m_bIsMove(false)
{
}

CMoveTextLine::~CMoveTextLine()
{
	m_TextInstance.Destroy();
}

uint32_t CMoveTextLine::Type()
{
	static uint32_t s_dwType = GetCRC32("CMoveTextLine", strlen("CMoveTextLine"));
	return (s_dwType);
}

BOOL CMoveTextLine::OnIsType(uint32_t dwType)
{
	if (CMoveTextLine::Type() == dwType)
		return TRUE;

	return FALSE;
}

void CMoveTextLine::SetMoveSpeed(float fSpeed)
{
	m_fMoveSpeed = fSpeed;
}

bool CMoveTextLine::GetMove()
{
	return m_bIsMove;
}

void CMoveTextLine::MoveStart()
{
	m_bIsMove = true;
	m_v2NextPos = m_v2SrcPos;
}

void CMoveTextLine::MoveStop()
{
	m_bIsMove = false;
}

void CMoveTextLine::OnEndMove()
{
	PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple());
}

void CMoveTextLine::OnChangePosition()
{
	m_TextInstance.SetPosition((GetDefaultCodePage() == CP_1256) ? m_rect.right : m_rect.left, m_rect.top);
}

void CMoveTextLine::SetMovePosition(float fDstX, float fDstY)
{
	if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
	{
		m_v2SrcPos.x = m_rect.left;
		m_v2SrcPos.y = m_rect.top;

		m_v2DstPos.x = fDstX;
		m_v2DstPos.y = fDstY;

		D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);

		m_fDistance = (m_v2Direction.y*m_v2Direction.y + m_v2Direction.x*m_v2Direction.x);
		D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

		if (m_v2SrcPos != m_v2NextPos)
		{
			const float fDist = sqrtf(m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
			m_v2NextPos = m_v2Direction * fDist;
			m_TextInstance.SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		}
	}
}

void CMoveTextLine::OnUpdate()
{
	if (IsShow() && GetMove())
	{
		const auto vv1 = m_v2Direction * m_fMoveSpeed;
		D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &vv1);
		D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

		const float fNextDistance = m_v2NextDistance.y * m_v2NextDistance.y + m_v2NextDistance.x * m_v2NextDistance.x;
		if (fNextDistance >= m_fDistance)
		{
			m_v2NextPos = m_v2DstPos;
			MoveStop();
			OnEndMove();
		}

		m_TextInstance.SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		m_TextInstance.Update();
	}
}

void CMoveTextLine::OnRender()
{
	if (IsShow())
		m_TextInstance.Render();
}

/// CMoveImageBox
CMoveImageBox::CMoveImageBox(PyObject * ppyObject) :
	CImageBox(ppyObject),
	m_v2SrcPos(0.0f, 0.0f),
	m_v2DstPos(0.0f, 0.0f),
	m_v2NextPos(0.0f, 0.0f),
	m_v2Direction(0.0f, 0.0f),
	m_v2NextDistance(0.0f, 0.0f),
	m_fDistance(0.0f),
	m_fMoveSpeed(10.0f),
	m_bIsMove(false)
{
}

CMoveImageBox::~CMoveImageBox()
{
	OnDestroyInstance();
}

void CMoveImageBox::OnCreateInstance()
{
	OnDestroyInstance();

	m_pImageInstance = CGraphicImageInstance::New();
}

void CMoveImageBox::OnDestroyInstance()
{
	if (m_pImageInstance)
	{
		CGraphicImageInstance::Delete(m_pImageInstance);
		m_pImageInstance = nullptr;
	}
}

uint32_t CMoveImageBox::Type()
{
	static uint32_t s_dwType = GetCRC32("CMoveImageBox", strlen("CMoveImageBox"));
	return (s_dwType);
}

BOOL CMoveImageBox::OnIsType(uint32_t dwType)
{
	if (CMoveImageBox::Type() == dwType)
		return TRUE;

	return FALSE;
}

void CMoveImageBox::OnEndMove()
{
	PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple());
}

void CMoveImageBox::SetMovePosition(float fDstX, float fDstY)
{
	if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
	{
		m_v2SrcPos.x = m_rect.left;
		m_v2SrcPos.y = m_rect.top;

		m_v2DstPos.x = fDstX;
		m_v2DstPos.y = fDstY;

		D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);

		m_fDistance = (m_v2Direction.x*m_v2Direction.x + m_v2Direction.y*m_v2Direction.y);

		D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

		if (m_pImageInstance && m_v2SrcPos != m_v2NextPos)
		{
			const float fDist = sqrtf(m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);

			m_v2NextPos = m_v2Direction * fDist;
			m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		}
	}
}

void CMoveImageBox::SetMoveSpeed(float fSpeed)
{
	m_fMoveSpeed = fSpeed;
}

void CMoveImageBox::MoveStart()
{
	m_bIsMove = true;
	m_v2NextPos = m_v2SrcPos;
}

void CMoveImageBox::MoveStop()
{
	m_bIsMove = false;
}

bool CMoveImageBox::GetMove()
{
	return m_bIsMove;
}

void CMoveImageBox::OnUpdate()
{
	if (!m_pImageInstance)
		return;

	if (IsShow() && GetMove())
	{
		const auto vv1 = m_v2Direction * m_fMoveSpeed;
		D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &vv1);
		D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

		const float fNextDistance = (m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
		if (fNextDistance >= m_fDistance)
		{
			m_v2NextPos = m_v2DstPos;
			MoveStop();
			OnEndMove();
		}

		m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
	}
}

void CMoveImageBox::OnRender()
{
	if (!m_pImageInstance)
		return;

	if (IsShow())
		m_pImageInstance->Render();
}

/// CMoveScaleImageBox
CMoveScaleImageBox::CMoveScaleImageBox(PyObject * ppyObject) :
	CMoveImageBox(ppyObject),
	m_fMaxScale(1.0f),
	m_fMaxScaleRate(1.0f),
	m_fScaleDistance(0.0f),
	m_fAdditionalScale(0.0f),
	m_v2CurScale(1.0f, 1.0f)
{
}

CMoveScaleImageBox::~CMoveScaleImageBox()
{
	OnDestroyInstance();
}

void CMoveScaleImageBox::OnCreateInstance()
{
	OnDestroyInstance();

	m_pImageInstance = CGraphicImageInstance::New();
}

void CMoveScaleImageBox::OnDestroyInstance()
{
	if (m_pImageInstance)
	{
		CGraphicImageInstance::Delete(m_pImageInstance);
		m_pImageInstance = nullptr;
	}
}

uint32_t CMoveScaleImageBox::Type()
{
	static uint32_t s_dwType = GetCRC32("CMoveScaleImageBox", strlen("CMoveScaleImageBox"));
	return (s_dwType);
}

BOOL CMoveScaleImageBox::OnIsType(uint32_t dwType)
{
	if (CMoveScaleImageBox::Type() == dwType)
		return TRUE;

	return FALSE;
}

void CMoveScaleImageBox::SetMaxScale(float fMaxScale)
{
	m_fMaxScale = fMaxScale;
}

void CMoveScaleImageBox::SetMaxScaleRate(float fMaxScaleRate)
{
	m_fMaxScaleRate = fMaxScaleRate;
	const float fDistanceRate = m_fDistance * fMaxScaleRate;
	m_fScaleDistance = fDistanceRate;
	m_v2CurScale = m_pImageInstance->GetScale();
	const float fDiffScale = m_fMaxScale - m_v2CurScale.x;
	m_fAdditionalScale = fDiffScale / (sqrtf(fDistanceRate) / m_fMoveSpeed);
}

void CMoveScaleImageBox::SetScalePivotCenter(bool bScalePivotCenter)
{
	if (m_pImageInstance)
		m_pImageInstance->SetScalePivotCenter(bScalePivotCenter);
}

void CMoveScaleImageBox::OnUpdate()
{
	if (!m_pImageInstance)
		return;

	if (IsShow() && GetMove())
	{
		const auto vv1 = m_v2Direction * m_fMoveSpeed;
		D3DXVec2Add(&m_v2NextPos, &m_v2NextPos, &vv1);
		D3DXVec2Subtract(&m_v2NextDistance, &m_v2NextPos, &m_v2SrcPos);

		const float fNextDistance = (m_v2NextDistance.x*m_v2NextDistance.x + m_v2NextDistance.y*m_v2NextDistance.y);
		if (m_fScaleDistance < fNextDistance)
			m_fAdditionalScale *= -1.0f;

		D3DXVECTOR2 v2NewScale;
		const auto vv2 = D3DXVECTOR2(m_fAdditionalScale, m_fAdditionalScale);
		D3DXVec2Add(&v2NewScale, &m_pImageInstance->GetScale(), &vv2);
		if (m_fMaxScale < v2NewScale.x)
			v2NewScale = D3DXVECTOR2(m_fMaxScale, m_fMaxScale);

		if (m_v2CurScale.x > v2NewScale.x)
			v2NewScale = m_v2CurScale;

		m_pImageInstance->SetScale(v2NewScale);

		if (fNextDistance >= m_fDistance)
		{
			m_pImageInstance->SetScale(m_v2CurScale);
			m_v2NextPos = m_v2DstPos;
			MoveStop();
			OnEndMove();
		}

		m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
	}
}
#endif

} // namespace UI
