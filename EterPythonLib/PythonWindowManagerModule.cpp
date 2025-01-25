#include "StdAfx.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonGridSlotWindow.h"

bool PyTuple_GetWindow(PyObject* poArgs, int pos, UI::CWindow** ppRetWindow)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, pos, &iHandle))
		return false;
	if (!iHandle)
		return false;

	*ppRetWindow = reinterpret_cast<UI::CWindow*>(iHandle);
	return true;
}

PyObject* wndMgrGetAspect(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", UI::CWindowManager::Instance().GetAspect());
}

PyObject* wndMgrOnceIgnoreMouseLeftButtonUpEvent(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindowManager::Instance().OnceIgnoreMouseLeftButtonUpEvent();
	return Py_BuildNone();
}

PyObject* wndMgrSetMouseHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetMouseHandler(poHandler);
	return Py_BuildNone();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///// Register /////
// Window
PyObject* wndMgrRegister(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindowManager& kWndMgr = UI::CWindowManager::Instance();
	UI::CWindow* pWindow = kWndMgr.RegisterWindow(po, szLayer);
	if (!pWindow)
		return Py_BuildException();

	return Py_BuildValue("i", pWindow);
}

// SlotWindow
PyObject* wndMgrRegisterSlotWindow(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterSlotWindow(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// GridSlotWindow
PyObject* wndMgrRegisterGridSlotWindow(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterGridSlotWindow(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// TextLine
PyObject* wndMgrRegisterTextLine(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterTextLine(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// MarkBox
PyObject* wndMgrRegisterMarkBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterMarkBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// ImageBox
PyObject* wndMgrRegisterImageBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterImageBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// ExpandedImageBox
PyObject* wndMgrRegisterExpandedImageBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterExpandedImageBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// AniImageBox
PyObject* wndMgrRegisterAniImageBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterAniImageBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// RegisterButton
PyObject* wndMgrRegisterButton(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterButton(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// RadioButton
PyObject* wndMgrRegisterRadioButton(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterRadioButton(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// ToggleButton
PyObject* wndMgrRegisterToggleButton(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterToggleButton(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// DragButton
PyObject* wndMgrRegisterDragButton(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterDragButton(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// Box
PyObject* wndMgrRegisterBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

#ifdef ENABLE_RENDER_TARGET
PyObject* wndMgrRegisterRenderTarget(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterRenderTarget(po, szLayer);
	return Py_BuildValue("i", pWindow);
}
#endif

// Bar
PyObject* wndMgrRegisterBar(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterBar(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// Line
PyObject* wndMgrRegisterLine(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterLine(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// Slot
PyObject* wndMgrRegisterBar3D(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterBar3D(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// NumberLine
PyObject* wndMgrRegisterNumberLine(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterNumberLine(po, szLayer);
	return Py_BuildValue("i", pWindow);
}
///// Register /////
/////////////////////////////////////////////////////////////////////////////////////////////////

PyObject* wndMgrDestroy(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	UI::CWindowManager::Instance().DestroyWindow(pWin);
	return Py_BuildNone();
}

PyObject* wndMgrIsFocus(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", pWindow == UI::CWindowManager::Instance().GetActivateWindow());
}

PyObject* wndMgrSetFocus(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	UI::CWindowManager::Instance().ActivateWindow(pWin);
	return Py_BuildNone();
}

PyObject* wndMgrKillFocus(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (PyTuple_GetWindow(poArgs, 0, &pWin))
	{
		if (pWin == UI::CWindowManager::Instance().GetActivateWindow())
			UI::CWindowManager::Instance().DeactivateWindow();
	}
	else
		UI::CWindowManager::Instance().DeactivateWindow();

	return Py_BuildNone();
}

PyObject* wndMgrLock(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	UI::CWindowManager::Instance().LockWindow(pWin);
	return Py_BuildNone();
}

PyObject* wndMgrUnlock(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindowManager::Instance().UnlockWindow();
	return Py_BuildNone();
}

PyObject* wndMgrSetName(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	char* szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	pWin->SetName(szName);
	return Py_BuildNone();
}

PyObject* wndMgrSetTop(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetTop(pWin);
	return Py_BuildNone();
}

PyObject* wndMgrShow(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	pWin->Show();
	return Py_BuildNone();
}

PyObject* wndMgrHide(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	pWin->Hide();
	return Py_BuildNone();
}

PyObject* wndMgrIsShow(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	return Py_BuildValue("i", pWin->IsShow());
}

PyObject* wndMgrIsRTL(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	return Py_BuildValue("i", pWin->IsFlag(UI::CWindow::FLAG_RTL));
}

PyObject* wndMgrSetScreenSize(PyObject* poSelf, PyObject* poArgs)
{
	int width;
	if (!PyTuple_GetInteger(poArgs, 0, &width))
		return Py_BuildException();
	int height;
	if (!PyTuple_GetInteger(poArgs, 1, &height))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetScreenSize(width, height);
	return Py_BuildNone();
}

PyObject* wndMgrSetParent(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	UI::CWindow* pParentWin;
	if (!PyTuple_GetWindow(poArgs, 1, &pParentWin))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetParent(pWin, pParentWin);
	return Py_BuildNone();
}

PyObject* wndMgrSetPickAlways(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetPickAlways(pWin);
	return Py_BuildNone();
}

PyObject* wndMgrSetWndSize(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int width;
	if (!PyTuple_GetInteger(poArgs, 1, &width))
		return Py_BuildException();
	int height;
	if (!PyTuple_GetInteger(poArgs, 2, &height))
		return Py_BuildException();

	pWin->SetSize(width, height);
	return Py_BuildNone();
}

PyObject* wndMgrSetWndPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int x;
	if (!PyTuple_GetInteger(poArgs, 1, &x))
		return Py_BuildException();
	int y;
	if (!PyTuple_GetInteger(poArgs, 2, &y))
		return Py_BuildException();

	pWin->SetPosition(x, y);
	return Py_BuildNone();
}

PyObject* wndMgrGetName(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	return Py_BuildValue("s", pWin->GetName());
}

PyObject* wndMgrGetWndWidth(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	return Py_BuildValue("i", pWin->GetWidth());
}

PyObject* wndMgrGetWndHeight(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	return Py_BuildValue("i", pWin->GetHeight());
}

PyObject* wndMgrGetWndLocalPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	long lx, ly;
	pWin->GetPosition(&lx, &ly);

	return Py_BuildValue("ii", lx, ly);
}

PyObject* wndMgrGetWndGlobalPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	RECT& rRect = pWindow->GetRect();
	return Py_BuildValue("ii", rRect.left, rRect.top);
}

PyObject* wndMgrGetWindowRect(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	RECT& rRect = pWin->GetRect();
	return Py_BuildValue("iiii", rRect.left, rRect.top, rRect.right - rRect.left, rRect.bottom - rRect.top);
}

PyObject* wndMgrSetWindowHorizontalAlign(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iAlign;
	if (!PyTuple_GetInteger(poArgs, 1, &iAlign))
		return Py_BuildException();

	pWin->SetHorizontalAlign(iAlign);

	return Py_BuildNone();
}

PyObject* wndMgrSetWindowVerticalAlign(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iAlign;
	if (!PyTuple_GetInteger(poArgs, 1, &iAlign))
		return Py_BuildException();

	pWin->SetVerticalAlign(iAlign);

	return Py_BuildNone();
}

PyObject* wndMgrIsIn(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

#ifdef ENABLE_INGAME_WIKI
	bool bCheckChilds;
	if (!PyTuple_GetBoolean(poArgs, 1, &bCheckChilds))
		bCheckChilds = false;

	UI::CWindow* pWinPoint = UI::CWindowManager::Instance().GetPointWindow();
	if (pWin == pWinPoint)
		return Py_BuildValue("b", true);

	if (bCheckChilds && pWinPoint)
	{
		if (pWin->IsChild(pWinPoint, true))
			return Py_BuildValue("b", true);
	}

	return Py_BuildValue("b", false);
#else
	return Py_BuildValue("i", pWin == UI::CWindowManager::Instance().GetPointWindow());
#endif
}

PyObject* wndMgrGetMouseLocalPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	long lx, ly;
	pWin->GetMouseLocalPosition(lx, ly);
	return Py_BuildValue("ii", lx, ly);
}

PyObject* wndMgrGetHyperlink(PyObject* poSelf, PyObject* poArgs)
{
	char retBuf[1024];
	int retLen = CGraphicTextInstance::Hyperlink_GetText(retBuf, sizeof(retBuf) - 1);
	retBuf[retLen] = '\0';

	return Py_BuildValue("s#", retBuf, retLen);
}

PyObject* wndMgrGetScreenWidth(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", UI::CWindowManager::Instance().GetScreenWidth());
}

PyObject* wndMgrGetScreenHeight(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", UI::CWindowManager::Instance().GetScreenHeight());
}

PyObject* wndMgrGetMousePosition(PyObject* poSelf, PyObject* poArgs)
{
	long lx, ly;
	UI::CWindowManager::Instance().GetMousePosition(lx, ly);
	return Py_BuildValue("ii", lx, ly);
}

PyObject* wndMgrIsDragging(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", UI::CWindowManager::Instance().IsDragging());
}

PyObject* wndMgrIsPickedWindow(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	UI::CWindow* pPickedWin = UI::CWindowManager::Instance().GetPointWindow();
	return Py_BuildValue("i", pWin == pPickedWin ? 1 : 0);
}

PyObject* wndMgrGetChildCount(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	return Py_BuildValue("i", pWin->GetChildCount());
}

PyObject* wndMgrAddFlag(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	char* pszFlag;
	if (!PyTuple_GetString(poArgs, 1, &pszFlag))
		return Py_BuildException();

	if (pszFlag && *pszFlag)
	{
		if (!stricmp(pszFlag, "movable"))
			pWin->AddFlag(UI::CWindow::FLAG_MOVABLE);
		else if (!stricmp(pszFlag, "limit"))
			pWin->AddFlag(UI::CWindow::FLAG_LIMIT);
		else if (!stricmp(pszFlag, "dragable"))
			pWin->AddFlag(UI::CWindow::FLAG_DRAGABLE);
		else if (!stricmp(pszFlag, "attach"))
			pWin->AddFlag(UI::CWindow::FLAG_ATTACH);
		else if (!stricmp(pszFlag, "restrict_x"))
			pWin->AddFlag(UI::CWindow::FLAG_RESTRICT_X);
		else if (!stricmp(pszFlag, "restrict_y"))
			pWin->AddFlag(UI::CWindow::FLAG_RESTRICT_Y);
		else if (!stricmp(pszFlag, "float"))
			pWin->AddFlag(UI::CWindow::FLAG_FLOAT);
		else if (!stricmp(pszFlag, "not_pick"))
			pWin->AddFlag(UI::CWindow::FLAG_NOT_PICK);
		else if (!stricmp(pszFlag, "ignore_size"))
			pWin->AddFlag(UI::CWindow::FLAG_IGNORE_SIZE);
		else if (!stricmp(pszFlag, "rtl"))
			pWin->AddFlag(UI::CWindow::FLAG_RTL);
		else if (!stricmp(pszFlag, "ltr"))
			pWin->RemoveFlag(UI::CWindow::FLAG_RTL);
#ifdef ENABLE_MAILBOX
		else if (!stricmp(pszFlag, "not_capture"))
			pWin->AddFlag(UI::CWindow::FLAG_NOT_CAPTURE);
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		else if (!stricmp(pszFlag, "not_move"))
			pWin->RemoveFlag(UI::CWindow::FLAG_MOVABLE);
#endif

		else
			TraceError("Unknown window flag %s", pszFlag);
	}

	return Py_BuildNone();
}

PyObject* wndMgrSetLimitBias(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int l;
	if (!PyTuple_GetInteger(poArgs, 1, &l))
		return Py_BuildException();
	int r;
	if (!PyTuple_GetInteger(poArgs, 2, &r))
		return Py_BuildException();
	int t;
	if (!PyTuple_GetInteger(poArgs, 3, &t))
		return Py_BuildException();
	int b;
	if (!PyTuple_GetInteger(poArgs, 4, &b))
		return Py_BuildException();

	pWin->SetLimitBias(l, r, t, b);
	return Py_BuildNone();
}

PyObject* wndMgrUpdateRect(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	pWin->UpdateRect();
	return Py_BuildNone();
}

PyObject* wndMgrAppendSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BuildException();

	int ixPosition;
	if (!PyTuple_GetInteger(poArgs, 2, &ixPosition))
		return Py_BuildException();

	int iyPosition;
	if (!PyTuple_GetInteger(poArgs, 3, &iyPosition))
		return Py_BuildException();

	int ixCellSize;
	if (!PyTuple_GetInteger(poArgs, 4, &ixCellSize))
		return Py_BuildException();

	int iyCellSize;
	if (!PyTuple_GetInteger(poArgs, 5, &iyCellSize))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->AppendSlot(iIndex, ixPosition, iyPosition, ixCellSize, iyCellSize);

	return Py_BuildNone();
}

PyObject* wndMgrArrangeSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iStartIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iStartIndex))
		return Py_BuildException();

	int ixCellCount;
	if (!PyTuple_GetInteger(poArgs, 2, &ixCellCount))
		return Py_BuildException();

	int iyCellCount;
	if (!PyTuple_GetInteger(poArgs, 3, &iyCellCount))
		return Py_BuildException();

	int ixCellSize;
	if (!PyTuple_GetInteger(poArgs, 4, &ixCellSize))
		return Py_BuildException();

	int iyCellSize;
	if (!PyTuple_GetInteger(poArgs, 5, &iyCellSize))
		return Py_BuildException();

	int ixBlank;
	if (!PyTuple_GetInteger(poArgs, 6, &ixBlank))
		return Py_BuildException();

	int iyBlank;
	if (!PyTuple_GetInteger(poArgs, 7, &iyBlank))
		return Py_BuildException();

	if (!pWin->IsType(UI::CGridSlotWindow::Type()))
	{
		TraceError("wndMgr.ArrangeSlot : not a grid window");
		return Py_BuildException();
	}

	auto* pGridSlotWin = msl::inherit_cast<UI::CGridSlotWindow*>(pWin);
	pGridSlotWin->ArrangeGridSlot(iStartIndex, ixCellCount, iyCellCount, ixCellSize, iyCellSize, ixBlank, iyBlank);

	return Py_BuildNone();
}

PyObject* wndMgrSetSlotBaseImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	float fr;
	if (!PyTuple_GetFloat(poArgs, 2, &fr))
		return Py_BuildException();
	float fg;
	if (!PyTuple_GetFloat(poArgs, 3, &fg))
		return Py_BuildException();
	float fb;
	if (!PyTuple_GetFloat(poArgs, 4, &fb))
		return Py_BuildException();
	float fa;
	if (!PyTuple_GetFloat(poArgs, 5, &fa))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
	{
		TraceError("wndMgr.ArrangeSlot : not a slot window");
		return Py_BuildException();
	}

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlotBaseImage(szFileName, fr, fg, fb, fa);

	return Py_BuildNone();
}

PyObject* wndMgrSetCoverButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	char* szUpImageName;
	if (!PyTuple_GetString(poArgs, 2, &szUpImageName))
		return Py_BuildException();
	char* szOverImageName;
	if (!PyTuple_GetString(poArgs, 3, &szOverImageName))
		return Py_BuildException();
	char* szDownImageName;
	if (!PyTuple_GetString(poArgs, 4, &szDownImageName))
		return Py_BuildException();
	char* szDisableImageName;
	if (!PyTuple_GetString(poArgs, 5, &szDisableImageName))
		return Py_BuildException();

	int iLeftButtonEnable;
	if (!PyTuple_GetInteger(poArgs, 6, &iLeftButtonEnable))
		return Py_BuildException();
	int iRightButtonEnable;
	if (!PyTuple_GetInteger(poArgs, 7, &iRightButtonEnable))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->SetCoverButton(iSlotIndex, szUpImageName, szOverImageName, szDownImageName, szDisableImageName, iLeftButtonEnable, iRightButtonEnable
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
		, pSlotWin->GetScaleX(), pSlotWin->GetScaleY()
#endif
	);

	return Py_BuildNone();
}

PyObject* wndMgrEnableCoverButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->EnableCoverButton(iSlotIndex);

	return Py_BuildNone();
}

PyObject* wndMgrDisableCoverButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->DisableCoverButton(iSlotIndex);

	return Py_BuildNone();
}

PyObject* wndMgrIsDisableCoverButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	return Py_BuildValue("i", pSlotWin->IsDisableCoverButton(iSlotIndex));
}

PyObject* wndMgrSetAlwaysRenderCoverButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotIndex;
	bool bAlwaysRender = false;

	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!PyTuple_GetBoolean(poArgs, 2, &bAlwaysRender))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->SetAlwaysRenderCoverButton(iSlotIndex, bAlwaysRender);

	return Py_BuildNone();
}

PyObject* wndMgrAppendSlotButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	char* szUpImageName;
	if (!PyTuple_GetString(poArgs, 1, &szUpImageName))
		return Py_BuildException();
	char* szOverImageName;
	if (!PyTuple_GetString(poArgs, 2, &szOverImageName))
		return Py_BuildException();
	char* szDownImageName;
	if (!PyTuple_GetString(poArgs, 3, &szDownImageName))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->AppendSlotButton(szUpImageName, szOverImageName, szDownImageName);

	return Py_BuildNone();
}

PyObject* wndMgrAppendRequirementSignImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	char* szImageName;
	if (!PyTuple_GetString(poArgs, 1, &szImageName))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->AppendRequirementSignImage(szImageName);

	return Py_BuildNone();
}

PyObject* wndMgrShowSlotButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->ShowSlotButton(iSlotNumber);

	return Py_BuildNone();
}

PyObject* wndMgrHideAllSlotButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	auto pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->HideAllSlotButton();

	return Py_BuildNone();
}

PyObject* wndMgrShowRequirementSign(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->ShowRequirementSign(iSlotNumber);

	return Py_BuildNone();
}

PyObject* wndMgrHideRequirementSign(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWindow);
	pSlotWin->HideRequirementSign(iSlotNumber);

	return Py_BuildNone();
}

PyObject* wndMgrRefreshSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->RefreshSlot();

	return Py_BuildNone();
}

PyObject* wndMgrSetUseMode(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetUseMode(iFlag);

	return Py_BuildNone();
}

PyObject* wndMgrSetUsableItem(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetUsableItem(iFlag);

	return Py_BuildNone();
}

PyObject* wndMgrClearSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->ClearSlot(iSlotIndex);

	return Py_BuildNone();
}

PyObject* wndMgrClearAllSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->ClearAllSlot();

	return Py_BuildNone();
}

PyObject* wndMgrHasSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);

	return Py_BuildValue("i", pSlotWin->HasSlot(iSlotIndex));
}

PyObject* wndMgrSetSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iItemIndex))
		return Py_BuildException();

	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 3, &iWidth))
		return Py_BuildException();

	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 4, &iHeight))
		return Py_BuildException();

	int iImageHandle;
	if (!PyTuple_GetInteger(poArgs, 5, &iImageHandle))
		return Py_BuildException();

	D3DXCOLOR diffuseColor;
	PyObject* pTuple;
	if (!PyTuple_GetObject(poArgs, 6, &pTuple))
	{
		diffuseColor = D3DXCOLOR(1.0, 1.0, 1.0, 1.0);
		//return Py_BuildException();
	}
	else
		// get diffuse color from pTuple
	{
		if (PyTuple_Size(pTuple) != 4)
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 0, &diffuseColor.r))
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 1, &diffuseColor.g))
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 2, &diffuseColor.b))
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 3, &diffuseColor.a))
			return Py_BuildException();
	}

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlot(iSlotIndex, iItemIndex, iWidth, iHeight, (CGraphicImage*)iImageHandle, diffuseColor
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
		, pSlotWin->GetScaleX(), pSlotWin->GetScaleY()
#endif
	);

	return Py_BuildNone();
}

#ifdef ENABLE_FISH_EVENT
PyObject* wndMgrSetFishSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iItemIndex))
		return Py_BuildException();

	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 3, &iWidth))
		return Py_BuildException();

	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 4, &iHeight))
		return Py_BuildException();

	int iImageHandle;
	if (!PyTuple_GetInteger(poArgs, 5, &iImageHandle))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetFishSlot(iSlotIndex, iItemIndex, iWidth, iHeight, (CGraphicImage*)iImageHandle);

	return Py_BuildNone();
}
#endif

PyObject* wndMgrSetSlotCount(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int iCount;
	if (!PyTuple_GetInteger(poArgs, 2, &iCount))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlotCount(iSlotIndex, iCount);

	return Py_BuildNone();
}

PyObject* wndMgrSetSlotCountNew(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int iGrade;
	if (!PyTuple_GetInteger(poArgs, 2, &iGrade))
		return Py_BuildException();

	int iCount;
	if (!PyTuple_GetInteger(poArgs, 3, &iCount))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlotCountNew(iSlotIndex, iGrade, iCount);

	return Py_BuildNone();
}

PyObject* wndMgrSetSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	float fCoolTime;
	if (!PyTuple_GetFloat(poArgs, 2, &fCoolTime))
		return Py_BuildException();

	float fElapsedTime = 0.0f;
	PyTuple_GetFloat(poArgs, 3, &fElapsedTime);

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlotCoolTime(iSlotIndex, fCoolTime, fElapsedTime);

	return Py_BuildNone();
}

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* wndMgrStoreSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iKey;
	if (!PyTuple_GetInteger(poArgs, 1, &iKey))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iSlotIndex))
		return Py_BuildException();

	float fCoolTime;
	if (!PyTuple_GetFloat(poArgs, 3, &fCoolTime))
		return Py_BuildException();

	float fElapsedTime = 0.0f;
	PyTuple_GetFloat(poArgs, 4, &fElapsedTime);

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->StoreSlotCoolTime(iKey, iSlotIndex, fCoolTime, fElapsedTime);

	return Py_BuildNone();
}

PyObject* wndMgrRestoreSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iKey;
	if (!PyTuple_GetInteger(poArgs, 1, &iKey))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->RestoreSlotCoolTime(iKey);

	return Py_BuildNone();
}
#endif

#ifdef SIGN_FOR_PLUS_ITEMS
PyObject* wndMgrAppendPlusOnSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int ixPlusPosition;
	if (!PyTuple_GetInteger(poArgs, 2, &ixPlusPosition))
		return Py_BuildException();

	int iyPlusPosition;
	if (!PyTuple_GetInteger(poArgs, 3, &iyPlusPosition))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->AppendPlusOnSlot(iSlotIndex, ixPlusPosition, iyPlusPosition);

	return Py_BuildNone();
}
#endif

PyObject* wndMgrSetToggleSlot(PyObject* poSelf, PyObject* poArgs)
{
	assert(!"wndMgrSetToggleSlot - Don't use such function");
	return Py_BuildNone();
}

PyObject* wndMgrActivateSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	int iColorType;
	if (!PyTuple_GetInteger(poArgs, 2, &iColorType))
		iColorType = UI::COLOR_TYPE_WHITE;

	if (!pWin->IsType(UI::CSlotWindow::Type()) || (iColorType < UI::COLOR_TYPE_ORANGE || iColorType >= UI::COLOR_TYPE_MAX))
		return Py_BuildException();
#else
	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();
#endif

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->ActivateSlot(iSlotIndex
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
		, iColorType
#endif
	);
	return Py_BuildNone();
}

PyObject* wndMgrDeactivateSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->DeactivateSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrEnableSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->EnableSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrDisableSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->DisableSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrShowSlotBaseImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->ShowSlotBaseImage(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrHideSlotBaseImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->HideSlotBaseImage(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrSetSlotType(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlotType(iType);

	return Py_BuildNone();
}

PyObject* wndMgrSetSlotStyle(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iStyle;
	if (!PyTuple_GetInteger(poArgs, 1, &iStyle))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SetSlotStyle(iStyle);

	return Py_BuildNone();
}

PyObject* wndMgrSelectSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->SelectSlot(iIndex);

	return Py_BuildNone();
}

PyObject* wndMgrClearSelected(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->ClearSelected();

	return Py_BuildNone();
}

PyObject* wndMgrGetSelectedSlotCount(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	return Py_BuildValue("i", pSlotWin->GetSelectedSlotCount());
}

PyObject* wndMgrGetSelectedSlotNumber(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	return Py_BuildValue("i", pSlotWin->GetSelectedSlotNumber(iSlotNumber));
}

PyObject* wndMgrIsSelectedSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotNumber))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	return Py_BuildValue("i", pSlotWin->isSelectedSlot(iSlotNumber));
}

PyObject* wndMgrGetSlotCount(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	return Py_BuildValue("i", pSlotWin->GetSlotCount());
}

PyObject* wndMgrLockSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->LockSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrUnlockSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	pSlotWin->UnlockSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndBarSetColor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iColor;
	if (!PyTuple_GetInteger(poArgs, 1, &iColor))
		return Py_BuildException();

	if (pWindow->IsType(UI::CBar3D::Type()))
	{
		int iLeftColor = iColor;

		int iRightColor;
		if (!PyTuple_GetInteger(poArgs, 2, &iRightColor))
			return Py_BuildException();
		int iCenterColor;
		if (!PyTuple_GetInteger(poArgs, 3, &iCenterColor))
			return Py_BuildException();

		(msl::inherit_cast<UI::CBar3D*>(pWindow))->SetColor(iLeftColor, iRightColor, iCenterColor);
	}
	else
		(pWindow)->SetColor(iColor);
	return Py_BuildNone();
}

PyObject* wndMgrAttachIcon(PyObject* poSelf, PyObject* poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BuildException();
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 2, &iSlotNumber))
		return Py_BuildException();
	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 3, &iWidth))
		return Py_BuildException();
	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 4, &iHeight))
		return Py_BuildException();

	UI::CWindowManager::Instance().AttachIcon(iType, iIndex, iSlotNumber, iWidth, iHeight);
	return Py_BuildNone();
}

PyObject* wndMgrDeattachIcon(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindowManager::Instance().DeattachIcon();
	return Py_BuildNone();
}

PyObject* wndMgrSetAttachingFlag(PyObject* poSelf, PyObject* poArgs)
{
	BOOL bFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &bFlag))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetAttachingFlag(bFlag);
	return Py_BuildNone();
}

// Text
PyObject* wndTextSetMax(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iMax;
	if (!PyTuple_GetInteger(poArgs, 1, &iMax))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetMax(iMax);
	return Py_BuildNone();
}
PyObject* wndTextSetHorizontalAlign(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetHorizontalAlign(iType);
	return Py_BuildNone();
}
PyObject* wndTextSetVerticalAlign(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetVerticalAlign(iType);
	return Py_BuildNone();
}
PyObject* wndTextSetSecret(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetSecret(iFlag);
	return Py_BuildNone();
}
PyObject* wndTextSetOutline(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetOutline(iFlag);
	return Py_BuildNone();
}
PyObject* wndTextSetFeather(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetFeather(iFlag);
	return Py_BuildNone();
}
PyObject* wndTextSetMultiLine(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetMultiLine(iFlag);
	return Py_BuildNone();
}
PyObject* wndTextSetFontName(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFontName;
	if (!PyTuple_GetString(poArgs, 1, &szFontName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetFontName(szFontName);
	return Py_BuildNone();
}
PyObject* wndTextSetFontColor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (2 == PyTuple_Size(poArgs))
	{
		int iColor;
		if (!PyTuple_GetInteger(poArgs, 1, &iColor))
			return Py_BuildException();
		(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetFontColor(iColor);
	}
	else if (4 == PyTuple_Size(poArgs))
	{
		float fr;
		if (!PyTuple_GetFloat(poArgs, 1, &fr))
			return Py_BuildException();
		float fg;
		if (!PyTuple_GetFloat(poArgs, 2, &fg))
			return Py_BuildException();
		float fb;
		if (!PyTuple_GetFloat(poArgs, 3, &fb))
			return Py_BuildException();
		float fa = 1.0f;

		uint8_t argb[4] = { static_cast<uint8_t>(255.0f * fb), static_cast<uint8_t>(255.0f * fg), static_cast<uint8_t>(255.0f * fr),
						   static_cast<uint8_t>(255.0f * fa) };
		(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetFontColor(*(reinterpret_cast<uint32_t*>(argb)));
	}
	else
		return Py_BuildException();

	return Py_BuildNone();
}
PyObject* wndTextSetLimitWidth(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fWidth;
	if (!PyTuple_GetFloat(poArgs, 1, &fWidth))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetLimitWidth(fWidth);
	return Py_BuildNone();
}
PyObject* wndTextSetText(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szText;
	if (!PyTuple_GetString(poArgs, 1, &szText))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->SetText(szText);
	return Py_BuildNone();
}
PyObject* wndTextGetText(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("s", (msl::inherit_cast<UI::CTextLine*>(pWindow))->GetText());
}

PyObject* wndTextGetTextSize(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int nWidth;
	int nHeight;

	auto* pkTextLine = msl::inherit_cast<UI::CTextLine*>(pWindow);
	pkTextLine->GetTextSize(&nWidth, &nHeight);

	return Py_BuildValue("(ii)", nWidth, nHeight);
}


PyObject* wndTextShowCursor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->ShowCursor();
	return Py_BuildNone();
}
PyObject* wndTextHideCursor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CTextLine*>(pWindow))->HideCursor();
	return Py_BuildNone();
}
PyObject* wndTextGetCursorPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", (msl::inherit_cast<UI::CTextLine*>(pWindow))->GetCursorPosition());
}

#ifdef ENABLE_SUNG_MAHI_TOWER
PyObject* wndTextGetCharSize(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	short sWidth;

	UI::CTextLine* pkTextLine = (UI::CTextLine*)pWindow;
	pkTextLine->GetCharSize(&sWidth);

	return Py_BuildValue("i", sWidth);
}
#endif

PyObject* wndNumberSetNumber(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szNumber;
	if (!PyTuple_GetString(poArgs, 1, &szNumber))
		return Py_BuildException();

	(msl::inherit_cast<UI::CNumberLine*>(pWindow))->SetNumber(szNumber);

	return Py_BuildNone();
}

PyObject* wndNumberSetNumberHorizontalAlignCenter(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CNumberLine*>(pWindow))->SetHorizontalAlign(UI::CWindow::HORIZONTAL_ALIGN_CENTER);

	return Py_BuildNone();
}

PyObject* wndNumberSetNumberHorizontalAlignRight(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CNumberLine*>(pWindow))->SetHorizontalAlign(UI::CWindow::HORIZONTAL_ALIGN_RIGHT);

	return Py_BuildNone();
}

PyObject* wndNumberSetPath(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szPath;
	if (!PyTuple_GetString(poArgs, 1, &szPath))
		return Py_BuildException();

	(msl::inherit_cast<UI::CNumberLine*>(pWindow))->SetPath(szPath);

	return Py_BuildNone();
}

PyObject* wndMarkBox_SetImageFilename(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CMarkBox*>(pWindow))->LoadImage(szFileName);
	return Py_BuildNone();
}

PyObject* wndMarkBox_SetImage(PyObject* poSelf, PyObject* poArgs)
{
	// 아무것도 하지 않음
	return Py_BuildNone();
}

PyObject* wndMarkBox_Load(PyObject* poSelf, PyObject* poArgs)
{
	// 아무것도 하지 않음
	return Py_BuildNone();
}

PyObject* wndMarkBox_SetIndex(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int nIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &nIndex))
		return Py_BuildException();

	(msl::inherit_cast<UI::CMarkBox*>(pWindow))->SetIndex(nIndex);
	return Py_BuildNone();
}

PyObject* wndMarkBox_SetScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fScale;
	if (!PyTuple_GetFloat(poArgs, 1, &fScale))
		return Py_BuildException();

	(msl::inherit_cast<UI::CMarkBox*>(pWindow))->SetScale(fScale);
	return Py_BuildNone();
}


PyObject* wndMarkBox_SetDiffuseColor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fr;
	if (!PyTuple_GetFloat(poArgs, 1, &fr))
		return Py_BuildException();
	float fg;
	if (!PyTuple_GetFloat(poArgs, 2, &fg))
		return Py_BuildException();
	float fb;
	if (!PyTuple_GetFloat(poArgs, 3, &fb))
		return Py_BuildException();
	float fa;
	if (!PyTuple_GetFloat(poArgs, 4, &fa))
		return Py_BuildException();

	(msl::inherit_cast<UI::CMarkBox*>(pWindow))->SetDiffuseColor(fr, fg, fb, fa);

	return Py_BuildNone();
}


PyObject* wndImageLoadImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	if (!(msl::inherit_cast<UI::CImageBox*>(pWindow))->LoadImage(szFileName))
		return Py_BuildException("Failed to load image (filename: %s)", szFileName);

	return Py_BuildNone();
}

PyObject* wndImageSetDiffuseColor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fr;
	if (!PyTuple_GetFloat(poArgs, 1, &fr))
		return Py_BuildException();
	float fg;
	if (!PyTuple_GetFloat(poArgs, 2, &fg))
		return Py_BuildException();
	float fb;
	if (!PyTuple_GetFloat(poArgs, 3, &fb))
		return Py_BuildException();
	float fa;
	if (!PyTuple_GetFloat(poArgs, 4, &fa))
		return Py_BuildException();

	(msl::inherit_cast<UI::CImageBox*>(pWindow))->SetDiffuseColor(fr, fg, fb, fa);

	return Py_BuildNone();
}

PyObject* wndImageGetWidth(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", (msl::inherit_cast<UI::CImageBox*>(pWindow))->GetWidth());
}

PyObject* wndImageGetHeight(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", (msl::inherit_cast<UI::CImageBox*>(pWindow))->GetHeight());
}

PyObject* wndImageSetScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	(msl::inherit_cast<UI::CExpandedImageBox*>(pWindow))->SetScale(fx, fy);

	return Py_BuildNone();
}
PyObject* wndImageSetOrigin(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	(msl::inherit_cast<UI::CExpandedImageBox*>(pWindow))->SetOrigin(fx, fy);

	return Py_BuildNone();
}
PyObject* wndImageSetRotation(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fRotation;
	if (!PyTuple_GetFloat(poArgs, 1, &fRotation))
		return Py_BuildException();

	(msl::inherit_cast<UI::CExpandedImageBox*>(pWindow))->SetRotation(fRotation);

	return Py_BuildNone();
}
PyObject* wndImageSetRenderingRect(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fLeft;
	if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
		return Py_BuildException();
	float fTop;
	if (!PyTuple_GetFloat(poArgs, 2, &fTop))
		return Py_BuildException();
	float fRight;
	if (!PyTuple_GetFloat(poArgs, 3, &fRight))
		return Py_BuildException();
	float fBottom;
	if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
		return Py_BuildException();

	if (pWindow->IsType(UI::CExpandedImageBox::Type()))
		(msl::inherit_cast<UI::CExpandedImageBox*>(pWindow))->SetRenderingRect(fLeft, fTop, fRight, fBottom);
	else if (pWindow->IsType(UI::CAniImageBox::Type()))
		(msl::inherit_cast<UI::CAniImageBox*>(pWindow))->SetRenderingRect(fLeft, fTop, fRight, fBottom);

	return Py_BuildNone();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* wndImageSetRenderingRectWithScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fLeft;
	if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
		return Py_BuildException();
	float fTop;
	if (!PyTuple_GetFloat(poArgs, 2, &fTop))
		return Py_BuildException();
	float fRight;
	if (!PyTuple_GetFloat(poArgs, 3, &fRight))
		return Py_BuildException();
	float fBottom;
	if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
		return Py_BuildException();

	if (pWindow->IsType(UI::CExpandedImageBox::Type()))
		((UI::CExpandedImageBox*)pWindow)->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);
	else if (pWindow->IsType(UI::CAniImageBox::Type()))
		((UI::CAniImageBox*)pWindow)->SetRenderingRectWithScale(fLeft, fTop, fRight, fBottom);

	return Py_BuildNone();
}
#endif

#if defined(ENABLE_IMAGE_CLIP_RECT) || defined(ENABLE_BATTLE_PASS_SYSTEM)
PyObject* wndSetClipRect(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fLeft;
	if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
		return Py_BuildException();
	float fTop;
	if (!PyTuple_GetFloat(poArgs, 2, &fTop))
		return Py_BuildException();
	float fRight;
	if (!PyTuple_GetFloat(poArgs, 3, &fRight))
		return Py_BuildException();
	float fBottom;
	if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
		return Py_BuildException();
	int isVertical;
	if (!PyTuple_GetInteger(poArgs, 5, &isVertical))
		return Py_BuildException();

	if (pWindow->IsType(UI::CExpandedImageBox::Type()))
		((UI::CExpandedImageBox*)pWindow)->SetImageClipRect(fLeft, fTop, fRight, fBottom, isVertical ? true : false);

	return Py_BuildNone();
}
#endif

PyObject* wndImageSetRenderingMode(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 1, &iMode))
		return Py_BuildException();

	if (pWindow->IsType(UI::CExpandedImageBox::Type()))
		(msl::inherit_cast<UI::CExpandedImageBox*>(pWindow))->SetRenderingMode(iMode);

	return Py_BuildNone();
}
PyObject* wndImageSetDelay(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fDelay;
	if (!PyTuple_GetFloat(poArgs, 1, &fDelay))
		return Py_BuildException();

	(msl::inherit_cast<UI::CAniImageBox*>(pWindow))->SetDelay(fDelay);

	return Py_BuildNone();
}
PyObject* wndImageAppendImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CAniImageBox*>(pWindow))->AppendImage(szFileName);

	return Py_BuildNone();
}


#ifdef ENABLE_OFFICAL_FEATURES
PyObject* wndImageResetFrame(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CAniImageBox*)pWindow)->ResetFrame();

	return Py_BuildNone();
}

PyObject* wndMgrGetSlotLocalPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
	{
#ifdef _DEBUG
		TraceError("wndMgr.GetSlotLocalPosition : not a slot window");
#endif
		return Py_BuildException();
	}

	int iX = 0, iY = 0;
	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->GetSlotLocalPosition((uint32_t)iSlotIndex, &iX, &iY);
	return Py_BuildValue("ii", iX, iY);
}

PyObject* wndMgrGetSlotGlobalPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
	{
#ifdef _DEBUG
		TraceError("wndMgr.GetSlotGlobalPosition : not a slot window");
#endif
		return Py_BuildException();
	}

	int iX = 0, iY = 0;
	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->GetSlotGlobalPosition((uint32_t)iSlotIndex, &iX, &iY);
	return Py_BuildValue("ii", iX, iY);
}
#endif

PyObject* wndButtonSetUpVisual(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->SetUpVisual(szFileName);

	return Py_BuildNone();
}
PyObject* wndButtonSetOverVisual(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->SetOverVisual(szFileName);

	return Py_BuildNone();
}
PyObject* wndButtonSetDownVisual(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->SetDownVisual(szFileName);

	return Py_BuildNone();
}
PyObject* wndButtonSetDisableVisual(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->SetDisableVisual(szFileName);

	return Py_BuildNone();
}
PyObject* wndButtonGetUpVisualFileName(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("s", (msl::inherit_cast<UI::CButton*>(pWindow))->GetUpVisualFileName());
}
PyObject* wndButtonGetOverVisualFileName(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("s", (msl::inherit_cast<UI::CButton*>(pWindow))->GetOverVisualFileName());
}
PyObject* wndButtonGetDownVisualFileName(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("s", (msl::inherit_cast<UI::CButton*>(pWindow))->GetDownVisualFileName());
}
PyObject* wndButtonFlash(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->Flash();

	return Py_BuildNone();
}
PyObject* wndButtonEnable(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->Enable();

	return Py_BuildNone();
}
PyObject* wndButtonDisable(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->Disable();

	return Py_BuildNone();
}
PyObject* wndButtonDown(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->Down();

	return Py_BuildNone();
}
PyObject* wndButtonSetUp(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	(msl::inherit_cast<UI::CButton*>(pWindow))->SetUp();

	return Py_BuildNone();
}

#ifdef ENABLE_MINI_GAME_YUTNORI
PyObject* wndButtonSetScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();

	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	((UI::CButton*)pWindow)->SetScale(fx, fy);

	return Py_BuildNone();
}

PyObject* wndMgrGetButtonImageWidth(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", ((UI::CButton*)pWindow)->GetImageWidth());
}

PyObject* wndMgrGetButtonImageHeight(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", ((UI::CButton*)pWindow)->GetImageHeight());
}
#endif

PyObject* wndButtonSetRestrictMovementArea(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int ix;
	if (!PyTuple_GetInteger(poArgs, 1, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 2, &iy))
		return Py_BuildException();
	int iwidth;
	if (!PyTuple_GetInteger(poArgs, 3, &iwidth))
		return Py_BuildException();
	int iheight;
	if (!PyTuple_GetInteger(poArgs, 4, &iheight))
		return Py_BuildException();

	(msl::inherit_cast<UI::CDragButton*>(pWindow))->SetRestrictMovementArea(ix, iy, iwidth, iheight);

	return Py_BuildNone();
}

PyObject* wndButtonIsDown(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", (msl::inherit_cast<UI::CButton*>(pWindow))->IsPressed());
}

#ifdef ENABLE_FISH_EVENT
PyObject* wndMgrSetPickedAreaRender(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	if (!pWin->IsType(UI::CGridSlotWindow::Type()))
	{
		TraceError("wndMgr.SetPickedAreaRender : not a grid window");
		return Py_BuildException();
	}

	UI::CGridSlotWindow* pGridSlotWin = (UI::CGridSlotWindow*)pWin;
	pGridSlotWin->SetPickedAreaRender(iFlag ? true : false);

	return Py_BuildNone();
}

PyObject* wndMgrDeleteCoverButton(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWindow;
	pSlotWin->DeleteCoverButton(iSlotIndex);

	return Py_BuildNone();
}

PyObject* wndMgrSetDisableDeattach(PyObject* poSelf, PyObject* poArgs)
{
	BOOL bFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &bFlag))
		return Py_BuildException();

	UI::CWindowManager::Instance().SetDisableDeattach(bFlag ? true : false);
	return Py_BuildNone();
}
#endif

extern BOOL g_bOutlineBoxEnable;
extern BOOL g_bShowOverInWindowName;

#ifdef NEW_SELECT_CHARACTER
PyObject* wndButtonOver(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	g_bShowOverInWindowName = iFlag;

	return Py_BuildNone();
}
#endif

PyObject* wndMgrSetOutlineFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	g_bOutlineBoxEnable = iFlag;

	return Py_BuildNone();
}

PyObject* wndMgrShowOverInWindowName(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	g_bShowOverInWindowName = iFlag;

	return Py_BuildNone();
}

#ifdef ENABLE_SLOT_WINDOW_EX
PyObject* wndMgrIsActivatedSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);
	return Py_BuildValue("i", pSlotWin->IsActivatedSlot(iSlotIndex));
}

PyObject* wndMgrGetSlotCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	auto* pSlotWin = msl::inherit_cast<UI::CSlotWindow*>(pWin);

	float fElapsedTime = 0.0f;
	float fCoolTime = pSlotWin->GetSlotCoolTime(iSlotIndex, &fElapsedTime);
	return Py_BuildValue("ff", fCoolTime, fElapsedTime);
}
#endif


#ifdef WJ_ENABLE_TRADABLE_ICON
PyObject* wndMgrSetCanMouseEventSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWnd;
	if (!PyTuple_GetWindow(poArgs, 0, &pWnd))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWnd->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWnd = (UI::CSlotWindow*)pWnd;
	pSlotWnd->SetCanMouseEventSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrSetCantMouseEventSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWnd;
	if (!PyTuple_GetWindow(poArgs, 0, &pWnd))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWnd->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWnd = (UI::CSlotWindow*)pWnd;
	pSlotWnd->SetCantMouseEventSlot(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrSetUsableSlotOnTopWnd(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWnd;
	if (!PyTuple_GetWindow(poArgs, 0, &pWnd))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWnd->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWnd = (UI::CSlotWindow*)pWnd;
	pSlotWnd->SetUsableSlotOnTopWnd(iSlotIndex);
	return Py_BuildNone();
}

PyObject* wndMgrSetUnusableSlotOnTopWnd(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWnd;
	if (!PyTuple_GetWindow(poArgs, 0, &pWnd))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	if (!pWnd->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWnd = (UI::CSlotWindow*)pWnd;
	pSlotWnd->SetUnusableSlotOnTopWnd(iSlotIndex);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* wndSetMoveSpeed(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fSpeed;
	if (!PyTuple_GetFloat(poArgs, 1, &fSpeed))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->SetMoveSpeed(fSpeed);
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->SetMoveSpeed(fSpeed);

	return Py_BuildNone();
}

PyObject* wndSetMovePosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fDstX(0.0f), fDstY(0.0f);
	if (!PyTuple_GetFloat(poArgs, 1, &fDstX))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 2, &fDstY))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->SetMovePosition(fDstX, fDstY);
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->SetMovePosition(fDstX, fDstY);

	return Py_BuildNone();
}

PyObject* wndMoveStart(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->MoveStart();
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->MoveStart();

	return Py_BuildNone();
}

PyObject* wndMoveStop(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveImageBox*)pWindow)->MoveStop();
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		((UI::CMoveTextLine*)pWindow)->MoveStop();

	return Py_BuildNone();
}

PyObject* wndGetMove(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveImageBox::Type()) || pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		return Py_BuildValue("i", ((UI::CMoveImageBox*)pWindow)->GetMove());
	else if (pWindow->IsType(UI::CMoveTextLine::Type()))
		return Py_BuildValue("i", ((UI::CMoveTextLine*)pWindow)->GetMove());
	else
		return Py_BuildValue("i", 0);
}

PyObject* wndSetMaxScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fMaxScale = 1.0f;
	if (!PyTuple_GetFloat(poArgs, 1, &fMaxScale))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveScaleImageBox*)pWindow)->SetMaxScale(fMaxScale);

	return Py_BuildNone();
}

PyObject* wndSetMaxScaleRate(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fMaxScaleRate = 1.0f;
	if (!PyTuple_GetFloat(poArgs, 1, &fMaxScaleRate))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveScaleImageBox*)pWindow)->SetMaxScaleRate(fMaxScaleRate);

	return Py_BuildNone();
}

PyObject* wndSetScalePivotCenter(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	bool bScalePivotCenter = false;
	if (!PyTuple_GetBoolean(poArgs, 1, &bScalePivotCenter))
		return Py_BuildException();

	if (pWindow->IsType(UI::CMoveScaleImageBox::Type()))
		((UI::CMoveScaleImageBox*)pWindow)->SetScalePivotCenter(bScalePivotCenter);

	return Py_BuildNone();
}
#endif

#ifdef WJ_MULTI_TEXTLINE
PyObject* wndMgrSetLineHeight(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int lineHeight;
	if (!PyTuple_GetInteger(poArgs, 1, &lineHeight))
		return Py_BuildException();

	((UI::CTextLine*)pWindow)->SetLineHeight(lineHeight);
	return Py_BuildNone();
}

PyObject* wndMgrGetLineHeight(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", ((UI::CTextLine*)pWindow)->GetLineHeight());
}

PyObject* wndMgrGetTextLineCount(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	return Py_BuildValue("i", ((UI::CTextLine*)pWindow)->GetTextLineCount());
}

PyObject* wndMgrDisableEnterToken(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CTextLine*)pWindow)->DisableEnterToken();
	return Py_BuildNone();
}
#endif

#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MAILBOX)
PyObject* wndButtonEnableFlash(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CButton*)pWindow)->EnableFlash();

	return Py_BuildNone();
}

PyObject* wndButtonDisableFlash(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CButton*)pWindow)->DisableFlash();

	return Py_BuildNone();
}
#endif

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
PyObject* wndSetAniImgScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	((UI::CAniImageBox*)pWindow)->SetScale(fx, fy);

	return Py_BuildNone();
}

PyObject* wndMgrSetSlotScale(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	if (!pWindow->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWindow;
	pSlotWin->SetScale(fx, fy);

	return Py_BuildNone();
}
#endif

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
PyObject* wndMgrSetSlotDiffuseColor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();
	int iColorType;
	if (!PyTuple_GetInteger(poArgs, 2, &iColorType))
		return Py_BuildException();

	if (!pWindow->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWindow;
	pSlotWin->SetSlotDiffuseColor(iSlotIndex, iColorType);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* wndMgrSetSlotCoolTimeColor(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	float fr;
	if (!PyTuple_GetFloat(poArgs, 2, &fr))
		return Py_BuildException();

	float fg;
	if (!PyTuple_GetFloat(poArgs, 3, &fg))
		return Py_BuildException();

	float fb;
	if (!PyTuple_GetFloat(poArgs, 4, &fb))
		return Py_BuildException();

	float fa;
	if (!PyTuple_GetFloat(poArgs, 5, &fa))
		return Py_BuildException();

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->SetSlotCoolTimeColor(iSlotIndex, fr, fg, fb, fa);

	return Py_BuildNone();
}

PyObject* wndMgrSetSlotCoolTimeInverse(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	float fCoolTime;
	if (!PyTuple_GetFloat(poArgs, 2, &fCoolTime))
		return Py_BuildException();

	float fElapsedTime = 0.0f;
	PyTuple_GetFloat(poArgs, 3, &fElapsedTime);

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->SetSlotCoolTimeInverse(iSlotIndex, fCoolTime, fElapsedTime);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_RENDER_TARGET
PyObject* wndRenderTargetSetRenderTarget(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	int index;
	if (!PyTuple_GetInteger(poArgs, 1, &index))
		return Py_BuildException();

	if (pWindow->IsType(UI::CUiRenderTarget::Type()))
	{
		auto* pRenderTarget = msl::inherit_cast<UI::CUiRenderTarget*>(pWindow);
		if (pRenderTarget)
			pRenderTarget->SetRenderTarget(index);
	}

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_12ZI
PyObject* wndButtonSetAlwaysToolTip(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	bool alwaystooltip = false;
	if (!PyTuple_GetBoolean(poArgs, 1, &alwaystooltip))
		return Py_BuildException();

	((UI::CButton*)pWindow)->SetAlwaysToolTip(alwaystooltip);

	return Py_BuildNone();
}

PyObject* wndImageSetCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fCoolTime;
	if (!PyTuple_GetFloat(poArgs, 1, &fCoolTime))
		return Py_BuildException();

	((UI::CImageBox*)pWindow)->SetCoolTime(fCoolTime);
	return Py_BuildNone();
}

PyObject* wndImageSetStartCoolTime(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();
	float fCoolTimeStart;
	if (!PyTuple_GetFloat(poArgs, 1, &fCoolTimeStart))
		return Py_BuildException();

	((UI::CImageBox*)pWindow)->SetCoolTimeStart(fCoolTimeStart);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
PyObject* wndMgrSetSlotCoverImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	char* szFileName;
	if (!PyTuple_GetString(poArgs, 2, &szFileName))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->SetSlotCoverImage(iSlotIndex, szFileName);

	return Py_BuildNone();
}

PyObject* wndMgrEnableSlotCoverImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int iOnOff;
	if (!PyTuple_GetInteger(poArgs, 2, &iOnOff))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->EnableSlotCoverImage(iSlotIndex, iOnOff);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PLUS_ICON_ITEM
PyObject* wndMgrGetValue(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t slot;
	char* Number;

	UI::CWindow* pWin;

	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &slot))
		return Py_BuildException();

	if (!PyTuple_GetString(poArgs, 2, &Number))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->GetValue(slot, Number);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
PyObject* wndIsDisable(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	const BOOL IsDisable = dynamic_cast<UI::CButton*>(pWindow)->IsDisable();
	return Py_BuildValue("i", IsDisable);
}
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
PyObject* wndMgrGetRealMousePosition(PyObject* poSelf, PyObject* poArgs)
{
	POINT kCursorPos;
	GetCursorPos(&kCursorPos);
	return Py_BuildValue("ii", kCursorPos.x, kCursorPos.y);
}

PyObject* wndMgrGetColorAtPosition(PyObject* poSelf, PyObject* poArgs)
{
	int iX;
	if (!PyTuple_GetInteger(poArgs, 0, &iX))
		return Py_BadArgument();

	int iY;
	if (!PyTuple_GetInteger(poArgs, 1, &iY))
		return Py_BadArgument();

	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 2, &pWin))
		pWin = nullptr;

	const POINT ptZero = { 0, 0 };
	HMONITOR hWndMonitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO kInfo;
	ZeroMemory(&kInfo, sizeof(kInfo));
	kInfo.cbSize = sizeof(kInfo);
	GetMonitorInfo(hWndMonitor, &kInfo);

	HDC pkDC = GetDC(nullptr);
	COLORREF rkColor = GetPixel(pkDC, iX * GetDeviceCaps(pkDC, DESKTOPHORZRES) / GetDeviceCaps(pkDC, HORZRES),
		iY * GetDeviceCaps(pkDC, DESKTOPVERTRES) / GetDeviceCaps(pkDC, VERTRES));
	ReleaseDC(nullptr, pkDC);

	uint8_t bRed = GetRValue(rkColor);
	uint8_t bGreen = GetGValue(rkColor);
	uint8_t bBlue = GetBValue(rkColor);

	return Py_BuildValue("iii", bRed, bGreen, bBlue);
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* wndRegisterMoveImageBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();

	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterMoveImageBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// MoveTextLine
PyObject* wndRegisterMoveTextLine(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterMoveTextLine(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

// MoveScaleImageBox
PyObject* wndRegisterMoveScaleImageBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();
	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterMoveScaleImageBox(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

PyObject* wndButtonLeftRightReverse(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CButton*)pWindow)->LeftRightReverse();
	return Py_BuildNone();
}

PyObject* wndImageLeftRightReverse(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CImageBox*)pWindow)->LeftRightReverse();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_INGAME_WIKI
PyObject* wndMgrRegisterWikiRenderTarget(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* po;
	if (!PyTuple_GetObject(poArgs, 0, &po))
		return Py_BuildException();

	char* szLayer;
	if (!PyTuple_GetString(poArgs, 1, &szLayer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterWikiRenderTarget(po, szLayer);
	return Py_BuildValue("i", pWindow);
}

PyObject* wndMgrSetInsideRender(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	bool val;
	if (!PyTuple_GetBoolean(poArgs, 1, &val))
		return Py_BuildException();

	pWin->SetInsideRender(val);
	return Py_BuildNone();
}

PyObject* wndSetRenderingRect(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float fLeft;
	if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
		return Py_BuildException();

	float fTop;
	if (!PyTuple_GetFloat(poArgs, 2, &fTop))
		return Py_BuildException();

	float fRight;
	if (!PyTuple_GetFloat(poArgs, 3, &fRight))
		return Py_BuildException();

	float fBottom;
	if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
		return Py_BuildException();

	pWindow->SetRenderingRect(fLeft, fTop, fRight, fBottom);
	return Py_BuildNone();
}

PyObject* wndMgrGetRenderBox(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	RECT val;
	pWin->GetRenderBox(&val);
	return Py_BuildValue("iiii", val.left, val.top, val.right, val.bottom);
}

PyObject* wndMgrSetWikiRenderTarget(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int renderWindow = -1;
	if (!PyTuple_GetInteger(poArgs, 1, &renderWindow))
		return Py_BuildException();

	((UI::CUiWikiRenderTarget*)pWin)->SetWikiRenderTargetModule(renderWindow);
	return Py_BuildNone();
}

PyObject* wndImageUnloadImage(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	((UI::CImageBox*)pWindow)->UnloadImage();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
PyObject* wndMgrSetCardSlot(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWin;
	if (!PyTuple_GetWindow(poArgs, 0, &pWin))
		return Py_BuildException();

	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSlotIndex))
		return Py_BuildException();

	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iItemIndex))
		return Py_BuildException();

	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 3, &iWidth))
		return Py_BuildException();

	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 4, &iHeight))
		return Py_BuildException();

	char* c_szFileName;
	if (!PyTuple_GetString(poArgs, 5, &c_szFileName))
		return Py_BuildException();

	D3DXCOLOR diffuseColor;
	PyObject* pTuple;
	if (!PyTuple_GetObject(poArgs, 6, &pTuple))
	{
		diffuseColor = D3DXCOLOR(1.0, 1.0, 1.0, 1.0);
		//return Py_BuildException();
	}
	else
		// get diffuse color from pTuple
	{
		if (PyTuple_Size(pTuple) != 4)
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 0, &diffuseColor.r))
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 1, &diffuseColor.g))
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 2, &diffuseColor.b))
			return Py_BuildException();
		if (!PyTuple_GetFloat(pTuple, 3, &diffuseColor.a))
			return Py_BuildException();
	}

	if (!pWin->IsType(UI::CSlotWindow::Type()))
		return Py_BuildException();

	UI::CSlotWindow* pSlotWin = (UI::CSlotWindow*)pWin;
	pSlotWin->SetCardSlot(iSlotIndex, iItemIndex, iWidth, iHeight, c_szFileName, diffuseColor);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_FISHING_RENEWAL
PyObject* wndMgrRegisterFishBox(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* p;
	if (!PyTuple_GetObject(poArgs, 0, &p))
		return Py_BuildException();

	char* layer;
	if (!PyTuple_GetString(poArgs, 1, &layer))
		return Py_BuildException();

	UI::CWindow* pWindow = UI::CWindowManager::Instance().RegisterFishBox(p, layer);
	return Py_BuildValue("i", pWindow);
}

PyObject* wndMgrFishMoveStart(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CFishBox::Type()))
		((UI::CFishBox*)pWindow)->MoveStart();

	return Py_BuildNone();
}

PyObject* wndMgrFishMoveStop(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CFishBox::Type()))
		((UI::CFishBox*)pWindow)->MoveStop();

	return Py_BuildNone();
}

PyObject* wndMgrFishGetMove(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CFishBox::Type()))
		return Py_BuildValue("i", ((UI::CFishBox*)pWindow)->GetMove());
	else
		return Py_BuildValue("i", 0);
}

PyObject* wndMgrFishGetPosition(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	if (pWindow->IsType(UI::CFishBox::Type())) {
		int x, y;
		((UI::CFishBox*)pWindow)->GetPosition(&x, &y);
		return Py_BuildValue("ii", x, y);
	}
	else {
		return Py_BuildValue("ii", 0, 0);
	}
}

PyObject* wndMgrAniSetRotation(PyObject* poSelf, PyObject* poArgs)
{
	UI::CWindow* pWindow;
	if (!PyTuple_GetWindow(poArgs, 0, &pWindow))
		return Py_BuildException();

	float r;
	if (!PyTuple_GetFloat(poArgs, 1, &r))
		return Py_BuildException();

	if (pWindow->IsType(UI::CAniImageBox::Type())) {
		((UI::CAniImageBox*)pWindow)->SetRotation(r);
	}

	return Py_BuildNone();
}
#endif

void initwndMgr()
{
	static PyMethodDef s_methods[] = {// WindowManager
									  {"SetMouseHandler", wndMgrSetMouseHandler, METH_VARARGS},
									  {"SetScreenSize", wndMgrSetScreenSize, METH_VARARGS},
									  {"GetScreenWidth", wndMgrGetScreenWidth, METH_VARARGS},
									  {"GetScreenHeight", wndMgrGetScreenHeight, METH_VARARGS},
									  {"AttachIcon", wndMgrAttachIcon, METH_VARARGS},
									  {"DeattachIcon", wndMgrDeattachIcon, METH_VARARGS},
									  {"SetAttachingFlag", wndMgrSetAttachingFlag, METH_VARARGS},
									  {"GetAspect", wndMgrGetAspect, METH_VARARGS},
									  {"GetHyperlink", wndMgrGetHyperlink, METH_VARARGS},
									  {"OnceIgnoreMouseLeftButtonUpEvent", wndMgrOnceIgnoreMouseLeftButtonUpEvent, METH_VARARGS},

									  // Window
									  {"Register", wndMgrRegister, METH_VARARGS},
									  {"RegisterSlotWindow", wndMgrRegisterSlotWindow, METH_VARARGS},
									  {"RegisterGridSlotWindow", wndMgrRegisterGridSlotWindow, METH_VARARGS},
									  {"RegisterTextLine", wndMgrRegisterTextLine, METH_VARARGS},
									  {"RegisterMarkBox", wndMgrRegisterMarkBox, METH_VARARGS},
									  {"RegisterImageBox", wndMgrRegisterImageBox, METH_VARARGS},
									  {"RegisterExpandedImageBox", wndMgrRegisterExpandedImageBox, METH_VARARGS},
									  {"RegisterAniImageBox", wndMgrRegisterAniImageBox, METH_VARARGS},
									  {"RegisterButton", wndMgrRegisterButton, METH_VARARGS},
									  {"RegisterRadioButton", wndMgrRegisterRadioButton, METH_VARARGS},
									  {"RegisterToggleButton", wndMgrRegisterToggleButton, METH_VARARGS},
									  {"RegisterDragButton", wndMgrRegisterDragButton, METH_VARARGS},
									  {"RegisterBox", wndMgrRegisterBox, METH_VARARGS},
									  {"RegisterBar", wndMgrRegisterBar, METH_VARARGS},
									  {"RegisterLine", wndMgrRegisterLine, METH_VARARGS},
									  {"RegisterBar3D", wndMgrRegisterBar3D, METH_VARARGS},
									  {"RegisterNumberLine", wndMgrRegisterNumberLine, METH_VARARGS},
									  {"Destroy", wndMgrDestroy, METH_VARARGS},
									  {"AddFlag", wndMgrAddFlag, METH_VARARGS},
									  {"IsRTL", wndMgrIsRTL, METH_VARARGS},

									  // Base Window
									  {"SetName", wndMgrSetName, METH_VARARGS},
									  {"GetName", wndMgrGetName, METH_VARARGS},

									  {"SetTop", wndMgrSetTop, METH_VARARGS},
									  {"Show", wndMgrShow, METH_VARARGS},
									  {"Hide", wndMgrHide, METH_VARARGS},
									  {"IsShow", wndMgrIsShow, METH_VARARGS},
									  {"SetParent", wndMgrSetParent, METH_VARARGS},
									  {"SetPickAlways", wndMgrSetPickAlways, METH_VARARGS},

									  {"IsFocus", wndMgrIsFocus, METH_VARARGS},
									  {"SetFocus", wndMgrSetFocus, METH_VARARGS},
									  {"KillFocus", wndMgrKillFocus, METH_VARARGS},
									  {"Lock", wndMgrLock, METH_VARARGS},
									  {"Unlock", wndMgrUnlock, METH_VARARGS},

									  {"SetWindowSize", wndMgrSetWndSize, METH_VARARGS},
									  {"SetWindowPosition", wndMgrSetWndPosition, METH_VARARGS},
									  {"GetWindowWidth", wndMgrGetWndWidth, METH_VARARGS},
									  {"GetWindowHeight", wndMgrGetWndHeight, METH_VARARGS},
									  {"GetWindowLocalPosition", wndMgrGetWndLocalPosition, METH_VARARGS},
									  {"GetWindowGlobalPosition", wndMgrGetWndGlobalPosition, METH_VARARGS},
									  {"GetWindowRect", wndMgrGetWindowRect, METH_VARARGS},
									  {"SetWindowHorizontalAlign", wndMgrSetWindowHorizontalAlign, METH_VARARGS},
									  {"SetWindowVerticalAlign", wndMgrSetWindowVerticalAlign, METH_VARARGS},

									  {"GetChildCount", wndMgrGetChildCount, METH_VARARGS},

									  {"IsPickedWindow", wndMgrIsPickedWindow, METH_VARARGS},
									  {"IsIn", wndMgrIsIn, METH_VARARGS},
									  {"GetMouseLocalPosition", wndMgrGetMouseLocalPosition, METH_VARARGS},
									  {"GetMousePosition", wndMgrGetMousePosition, METH_VARARGS},
									  {"IsDragging", wndMgrIsDragging, METH_VARARGS},

									  {"SetLimitBias", wndMgrSetLimitBias, METH_VARARGS},

									  {"UpdateRect", wndMgrUpdateRect, METH_VARARGS},

									  // Slot Window
									  {"AppendSlot", wndMgrAppendSlot, METH_VARARGS},
									  {"ArrangeSlot", wndMgrArrangeSlot, METH_VARARGS},
									  {"ClearSlot", wndMgrClearSlot, METH_VARARGS},
									  {"ClearAllSlot", wndMgrClearAllSlot, METH_VARARGS},
									  {"HasSlot", wndMgrHasSlot, METH_VARARGS},
									  {"SetSlot", wndMgrSetSlot, METH_VARARGS},
#ifdef ENABLE_FISH_EVENT
									  {"SetFishSlot", wndMgrSetFishSlot, METH_VARARGS},
#endif
									  {"SetSlotCount", wndMgrSetSlotCount, METH_VARARGS},
									  {"SetSlotCountNew", wndMgrSetSlotCountNew, METH_VARARGS},
									  {"SetSlotCoolTime", wndMgrSetSlotCoolTime, METH_VARARGS},
#ifdef ENABLE_OFFICAL_FEATURES
		{ "StoreSlotCoolTime",			wndMgrStoreSlotCoolTime,			METH_VARARGS },
		{ "RestoreSlotCoolTime",		wndMgrRestoreSlotCoolTime,			METH_VARARGS },
#endif
									  {"SetToggleSlot", wndMgrSetToggleSlot, METH_VARARGS},
									  {"ActivateSlot", wndMgrActivateSlot, METH_VARARGS},
									  {"DeactivateSlot", wndMgrDeactivateSlot, METH_VARARGS},
									  {"EnableSlot", wndMgrEnableSlot, METH_VARARGS},
									  {"DisableSlot", wndMgrDisableSlot, METH_VARARGS},
									  {"ShowSlotBaseImage", wndMgrShowSlotBaseImage, METH_VARARGS},
									  {"HideSlotBaseImage", wndMgrHideSlotBaseImage, METH_VARARGS},
									  {"SetSlotType", wndMgrSetSlotType, METH_VARARGS},
									  {"SetSlotStyle", wndMgrSetSlotStyle, METH_VARARGS},
									  {"SetSlotBaseImage", wndMgrSetSlotBaseImage, METH_VARARGS},

									  {"SetCoverButton", wndMgrSetCoverButton, METH_VARARGS},
									  {"EnableCoverButton", wndMgrEnableCoverButton, METH_VARARGS},
									  {"DisableCoverButton", wndMgrDisableCoverButton, METH_VARARGS},
									  {"IsDisableCoverButton", wndMgrIsDisableCoverButton, METH_VARARGS},
									  {"SetAlwaysRenderCoverButton", wndMgrSetAlwaysRenderCoverButton, METH_VARARGS},

									  {"AppendSlotButton", wndMgrAppendSlotButton, METH_VARARGS},
									  {"AppendRequirementSignImage", wndMgrAppendRequirementSignImage, METH_VARARGS},
									  {"ShowSlotButton", wndMgrShowSlotButton, METH_VARARGS},
									  {"HideAllSlotButton", wndMgrHideAllSlotButton, METH_VARARGS},
									  {"ShowRequirementSign", wndMgrShowRequirementSign, METH_VARARGS},
									  {"HideRequirementSign", wndMgrHideRequirementSign, METH_VARARGS},
									  {"RefreshSlot", wndMgrRefreshSlot, METH_VARARGS},
									  {"SetUseMode", wndMgrSetUseMode, METH_VARARGS},
									  {"SetUsableItem", wndMgrSetUsableItem, METH_VARARGS},

									  {"SelectSlot", wndMgrSelectSlot, METH_VARARGS},
									  {"ClearSelected", wndMgrClearSelected, METH_VARARGS},
									  {"GetSelectedSlotCount", wndMgrGetSelectedSlotCount, METH_VARARGS},
									  {"GetSelectedSlotNumber", wndMgrGetSelectedSlotNumber, METH_VARARGS},
									  {"IsSelectedSlot", wndMgrIsSelectedSlot, METH_VARARGS},
									  {"GetSlotCount", wndMgrGetSlotCount, METH_VARARGS},
									  {"LockSlot", wndMgrLockSlot, METH_VARARGS},
									  {"UnlockSlot", wndMgrUnlockSlot, METH_VARARGS},

									  // Bar
									  {"SetColor", wndBarSetColor, METH_VARARGS},

									  // TextLine
									  {"SetMax", wndTextSetMax, METH_VARARGS},
									  {"SetHorizontalAlign", wndTextSetHorizontalAlign, METH_VARARGS},
									  {"SetVerticalAlign", wndTextSetVerticalAlign, METH_VARARGS},
									  {"SetSecret", wndTextSetSecret, METH_VARARGS},
									  {"SetOutline", wndTextSetOutline, METH_VARARGS},
									  {"SetFeather", wndTextSetFeather, METH_VARARGS},
									  {"SetMultiLine", wndTextSetMultiLine, METH_VARARGS},
									  {"SetText", wndTextSetText, METH_VARARGS},
									  {"SetFontName", wndTextSetFontName, METH_VARARGS},
									  {"SetFontColor", wndTextSetFontColor, METH_VARARGS},
									  {"SetLimitWidth", wndTextSetLimitWidth, METH_VARARGS},
									  {"GetText", wndTextGetText, METH_VARARGS},
									  {"GetTextSize", wndTextGetTextSize, METH_VARARGS},
									  {"ShowCursor", wndTextShowCursor, METH_VARARGS},
									  {"HideCursor", wndTextHideCursor, METH_VARARGS},
									  {"GetCursorPosition", wndTextGetCursorPosition, METH_VARARGS},

#ifdef ENABLE_SUNG_MAHI_TOWER
									  { "GetCharSize",				wndTextGetCharSize,					METH_VARARGS },
#endif

									  // NumberLine
									  {"SetNumber", wndNumberSetNumber, METH_VARARGS},
									  {"SetNumberHorizontalAlignCenter", wndNumberSetNumberHorizontalAlignCenter, METH_VARARGS},
									  {"SetNumberHorizontalAlignRight", wndNumberSetNumberHorizontalAlignRight, METH_VARARGS},
									  {"SetPath", wndNumberSetPath, METH_VARARGS},

									  // ImageBox
									  {"MarkBox_SetImage", wndMarkBox_SetImage, METH_VARARGS},
									  {"MarkBox_SetImageFilename", wndMarkBox_SetImageFilename, METH_VARARGS},
									  {"MarkBox_Load", wndMarkBox_Load, METH_VARARGS},
									  {"MarkBox_SetIndex", wndMarkBox_SetIndex, METH_VARARGS},
									  {"MarkBox_SetScale", wndMarkBox_SetScale, METH_VARARGS},
									  {"MarkBox_SetDiffuseColor", wndMarkBox_SetDiffuseColor, METH_VARARGS},

									  // ImageBox
									  {"LoadImage", wndImageLoadImage, METH_VARARGS},
									  {"SetDiffuseColor", wndImageSetDiffuseColor, METH_VARARGS},
									  {"GetWidth", wndImageGetWidth, METH_VARARGS},
									  {"GetHeight", wndImageGetHeight, METH_VARARGS},

									  // ExpandedImageBox
									  {"SetScale", wndImageSetScale, METH_VARARGS},
									  {"SetOrigin", wndImageSetOrigin, METH_VARARGS},
									  {"SetRotation", wndImageSetRotation, METH_VARARGS},
									  {"SetRenderingRect", wndImageSetRenderingRect, METH_VARARGS},
#ifdef ENABLE_GROWTH_PET_SYSTEM
		{ "SetRenderingRectWithScale",	wndImageSetRenderingRectWithScale,	METH_VARARGS },
#endif
#if defined(ENABLE_IMAGE_CLIP_RECT) || defined(ENABLE_BATTLE_PASS_SYSTEM)
									  { "SetClipRect", wndSetClipRect, METH_VARARGS },
#endif
									  {"SetRenderingMode", wndImageSetRenderingMode, METH_VARARGS},

									  // AniImageBox
									  {"SetDelay", wndImageSetDelay, METH_VARARGS},
									  {"AppendImage", wndImageAppendImage, METH_VARARGS},
#ifdef ENABLE_OFFICAL_FEATURES
		{ "ResetFrame",					wndImageResetFrame,					METH_VARARGS },
		{ "GetSlotLocalPosition",		wndMgrGetSlotLocalPosition,			METH_VARARGS },
		{ "GetSlotGlobalPosition",		wndMgrGetSlotGlobalPosition,		METH_VARARGS },
#endif

		// Button
		{"SetUpVisual", wndButtonSetUpVisual, METH_VARARGS},
		{"SetOverVisual", wndButtonSetOverVisual, METH_VARARGS},
		{"SetDownVisual", wndButtonSetDownVisual, METH_VARARGS},
		{"SetDisableVisual", wndButtonSetDisableVisual, METH_VARARGS},
		{"GetUpVisualFileName", wndButtonGetUpVisualFileName, METH_VARARGS},
		{"GetOverVisualFileName", wndButtonGetOverVisualFileName, METH_VARARGS},
		{"GetDownVisualFileName", wndButtonGetDownVisualFileName, METH_VARARGS},
		{"Flash", wndButtonFlash, METH_VARARGS},
		{"Enable", wndButtonEnable, METH_VARARGS},
		{"Disable", wndButtonDisable, METH_VARARGS},
#ifdef NEW_SELECT_CHARACTER
		{ "Over",						wndButtonOver,						METH_VARARGS },
#endif
									  {"Down", wndButtonDown, METH_VARARGS},
									  {"SetUp", wndButtonSetUp, METH_VARARGS},
									  {"IsDown", wndButtonIsDown, METH_VARARGS},

#ifdef ENABLE_MINI_GAME_YUTNORI
									  { "SetButtonScale",				wndButtonSetScale,					METH_VARARGS },
									  { "GetButtonImageWidth",		wndMgrGetButtonImageWidth,			METH_VARARGS },
									  { "GetButtonImageHeight",		wndMgrGetButtonImageHeight,			METH_VARARGS },
#endif

#ifdef ENABLE_FISH_EVENT
									  { "SetPickedAreaRender",		wndMgrSetPickedAreaRender,			METH_VARARGS },
									  { "DeleteCoverButton",			wndMgrDeleteCoverButton,			METH_VARARGS },
									  { "SetDisableDeattach",			wndMgrSetDisableDeattach,			METH_VARARGS },
#endif

									  // DragButton
									  {"SetRestrictMovementArea", wndButtonSetRestrictMovementArea, METH_VARARGS},

									  // For Debug
									  {"SetOutlineFlag", wndMgrSetOutlineFlag, METH_VARARGS},
									  {"ShowOverInWindowName", wndMgrShowOverInWindowName, METH_VARARGS},

#ifdef ENABLE_SLOT_WINDOW_EX
									  {"IsActivatedSlot", wndMgrIsActivatedSlot, METH_VARARGS},
									  {"GetSlotCoolTime", wndMgrGetSlotCoolTime, METH_VARARGS},
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
		{ "SetCanMouseEventSlot",		wndMgrSetCanMouseEventSlot,			METH_VARARGS },
		{ "SetCantMouseEventSlot",		wndMgrSetCantMouseEventSlot,		METH_VARARGS },
		{ "SetUsableSlotOnTopWnd",		wndMgrSetUsableSlotOnTopWnd,		METH_VARARGS },
		{ "SetUnusableSlotOnTopWnd",	wndMgrSetUnusableSlotOnTopWnd,		METH_VARARGS },
#endif

#ifdef ENABLE_OFFICAL_FEATURES
		{ "SetMoveSpeed", wndSetMoveSpeed, METH_VARARGS},
		{ "SetMovePosition", wndSetMovePosition, METH_VARARGS},
		{ "MoveStart", wndMoveStart, METH_VARARGS},
		{ "MoveStop", wndMoveStop, METH_VARARGS},
		{ "GetMove", wndGetMove, METH_VARARGS},

		{ "SetMaxScale", wndSetMaxScale, METH_VARARGS},
		{ "SetMaxScaleRate", wndSetMaxScaleRate, METH_VARARGS},
		{ "SetScalePivotCenter", wndSetScalePivotCenter, METH_VARARGS},
#endif

#ifdef WJ_MULTI_TEXTLINE
		{ "SetLineHeight", wndMgrSetLineHeight, METH_VARARGS },
		{ "GetLineHeight", wndMgrGetLineHeight, METH_VARARGS },

		{ "GetTextLineCount", wndMgrGetTextLineCount, METH_VARARGS },
		{ "DisableEnterToken", wndMgrDisableEnterToken, METH_VARARGS },
#endif

#if defined(ENABLE_12ZI) || defined(ENABLE_BATTLE_FIELD) || defined(ENABLE_GROWTH_PET_SYSTEM)
		{ "EnableFlash", wndButtonEnableFlash, METH_VARARGS },
		{ "DisableFlash", wndButtonDisableFlash, METH_VARARGS },
#endif

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM)
		{ "SetAniImgScale", wndSetAniImgScale, METH_VARARGS},
		{ "SetSlotScale", wndMgrSetSlotScale, METH_VARARGS},
#endif
#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
		{ "SetSlotDiffuseColor", wndMgrSetSlotDiffuseColor, METH_VARARGS },
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		{ "SetSlotCoolTimeColor", wndMgrSetSlotCoolTimeColor, METH_VARARGS },
		{ "SetSlotCoolTimeInverse",		wndMgrSetSlotCoolTimeInverse,		METH_VARARGS },
#endif

#ifdef ENABLE_RENDER_TARGET
		{ "RegisterRenderTarget",		wndMgrRegisterRenderTarget,			METH_VARARGS },
		{ "SetRenderTarget",			wndRenderTargetSetRenderTarget,		METH_VARARGS },
#endif

#ifdef ENABLE_12ZI
		{ "SetCoolTimeImageBox",		wndImageSetCoolTime,				METH_VARARGS },
		{ "SetStartCoolTimeImageBox",	wndImageSetStartCoolTime,			METH_VARARGS },
		{ "SetAlwaysToolTip", wndButtonSetAlwaysToolTip, METH_VARARGS },
#endif

#ifdef ENABLE_SLOT_COVER_IMAGE_SYSTEM
		{ "SetSlotCoverImage", wndMgrSetSlotCoverImage, METH_VARARGS },
		{ "EnableSlotCoverImage", wndMgrEnableSlotCoverImage, METH_VARARGS },
#endif

#ifdef ENABLE_PLUS_ICON_ITEM
		{ "GetValue", wndMgrGetValue, METH_VARARGS },
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
		{ "GetRealMousePosition",		wndMgrGetRealMousePosition,			METH_VARARGS },
		{ "GetColorAtPosition",			wndMgrGetColorAtPosition,			METH_VARARGS },
#endif

#ifdef SIGN_FOR_PLUS_ITEMS
		{ "AppendPlusOnSlot",			wndMgrAppendPlusOnSlot,				METH_VARARGS },
#endif



#ifdef ENABLE_OFFICAL_FEATURES
		{ "RegisterMoveImageBox",	wndRegisterMoveImageBox,			METH_VARARGS },
									  { "RegisterMoveTextLine",		wndRegisterMoveTextLine,			METH_VARARGS },
									  { "RegisterMoveScaleImageBox",	wndRegisterMoveScaleImageBox,	METH_VARARGS },
									  { "LeftRightReverseImageBox",	wndImageLeftRightReverse,			METH_VARARGS },
									  { "LeftRightReverse",			wndButtonLeftRightReverse,			METH_VARARGS },
#endif

#ifdef ENABLE_INGAME_WIKI
		{ "RegisterWikiRenderTarget",	wndMgrRegisterWikiRenderTarget,		METH_VARARGS },
		{ "SetInsideRender",			wndMgrSetInsideRender,				METH_VARARGS },
		{ "SetRenderingRectWiki",		wndSetRenderingRect,				METH_VARARGS },
		{ "GetRenderBox",				wndMgrGetRenderBox,					METH_VARARGS },
		{ "SetWikiRenderTarget",		wndMgrSetWikiRenderTarget,			METH_VARARGS },
		{ "UnloadImage",				wndImageUnloadImage,				METH_VARARGS },
		{ "IsShowCursor",				wndTextShowCursor,					METH_VARARGS },
#endif

#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
									  { "SetCardSlot",				wndMgrSetCardSlot,					METH_VARARGS },
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
									  { "IsDIsable", wndIsDisable, METH_VARARGS },
#endif

#ifdef ENABLE_FISHING_RENEWAL
		{ "RegisterFishBox", wndMgrRegisterFishBox, METH_VARARGS },
		{ "FishMoveStart", wndMgrFishMoveStart, METH_VARARGS },
		{ "FishMoveStop", wndMgrFishMoveStop, METH_VARARGS },
		{ "FishGetMove", wndMgrFishGetMove, METH_VARARGS },
		{ "FishGetPosition", wndMgrFishGetPosition, METH_VARARGS },
		{ "AniSetRotation", wndMgrAniSetRotation, METH_VARARGS },
#endif

									  {nullptr, nullptr} };

	PyObject* poModule = Py_InitModule("wndMgr", s_methods);

	// 하나의 딕셔너리에 너무 많은 Function이 포함 되는 것 같아 이런식으로 딕셔너리를 나누는 것을 고려 중 - [levites]
	//	PyObject * poMgrModule = Py_InitModule("wndMgr", s_methods);
	//	PyObject * poTextModule = Py_InitModule("wndText", s_methods);
	//	PyObject * poSlotModule = Py_InitModule("wndSlot", s_methods);

	PyModule_AddIntConstant(poModule, "SLOT_STYLE_NONE", UI::SLOT_STYLE_NONE);
	PyModule_AddIntConstant(poModule, "SLOT_STYLE_PICK_UP", UI::SLOT_STYLE_PICK_UP);
	PyModule_AddIntConstant(poModule, "SLOT_STYLE_SELECT", UI::SLOT_STYLE_SELECT);

	PyModule_AddIntConstant(poModule, "TEXT_HORIZONTAL_ALIGN_LEFT", CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
	PyModule_AddIntConstant(poModule, "TEXT_HORIZONTAL_ALIGN_RIGHT", CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
	PyModule_AddIntConstant(poModule, "TEXT_HORIZONTAL_ALIGN_CENTER", CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	PyModule_AddIntConstant(poModule, "TEXT_VERTICAL_ALIGN_TOP", CGraphicTextInstance::VERTICAL_ALIGN_TOP);
	PyModule_AddIntConstant(poModule, "TEXT_VERTICAL_ALIGN_BOTTOM", CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	PyModule_AddIntConstant(poModule, "TEXT_VERTICAL_ALIGN_CENTER", CGraphicTextInstance::VERTICAL_ALIGN_CENTER);

	PyModule_AddIntConstant(poModule, "HORIZONTAL_ALIGN_LEFT", UI::CWindow::HORIZONTAL_ALIGN_LEFT);
	PyModule_AddIntConstant(poModule, "HORIZONTAL_ALIGN_CENTER", UI::CWindow::HORIZONTAL_ALIGN_CENTER);
	PyModule_AddIntConstant(poModule, "HORIZONTAL_ALIGN_RIGHT", UI::CWindow::HORIZONTAL_ALIGN_RIGHT);
	PyModule_AddIntConstant(poModule, "VERTICAL_ALIGN_TOP", UI::CWindow::VERTICAL_ALIGN_TOP);
	PyModule_AddIntConstant(poModule, "VERTICAL_ALIGN_CENTER", UI::CWindow::VERTICAL_ALIGN_CENTER);
	PyModule_AddIntConstant(poModule, "VERTICAL_ALIGN_BOTTOM", UI::CWindow::VERTICAL_ALIGN_BOTTOM);

#ifdef WJ_ENABLE_PICKUP_ITEM_EFFECT
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_ORANGE", UI::COLOR_TYPE_ORANGE);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_WHITE", UI::COLOR_TYPE_WHITE);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_RED", UI::COLOR_TYPE_RED);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_GREEN", UI::COLOR_TYPE_GREEN);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_YELLOW", UI::COLOR_TYPE_YELLOW);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_SKY", UI::COLOR_TYPE_SKY);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_PINK", UI::COLOR_TYPE_PINK);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_BLUE", UI::COLOR_TYPE_BLUE);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_PURPLE", UI::COLOR_TYPE_PURPLE);
	PyModule_AddIntConstant(poModule, "COLOR_TYPE_MAX", UI::COLOR_TYPE_MAX);

#	ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "HILIGHTSLOT_ACCE", UI::HILIGHTSLOT_ACCE);
#	endif
#	ifdef ENABLE_CHANGE_LOOK_SYSTEM
	PyModule_AddIntConstant(poModule, "HILIGHTSLOT_CHANGE_LOOK", UI::HILIGHTSLOT_CHANGE_LOOK);
#	endif
#	ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "HILIGHTSLOT_AURA", UI::HILIGHTSLOT_AURA);
#	endif
	PyModule_AddIntConstant(poModule, "HILIGHTSLOT_CUBE", UI::HILIGHTSLOT_CUBE);
#endif

	PyModule_AddIntConstant(poModule, "RENDERING_MODE_MODULATE", CGraphicExpandedImageInstance::RENDERING_MODE_MODULATE);
}
