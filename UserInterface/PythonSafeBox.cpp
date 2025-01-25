#include "StdAfx.h"
#include "PythonSafeBox.h"

void CPythonSafeBox::OpenSafeBox(int iSize)
{
	m_ItemInstanceVector.clear();
	m_ItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

	for (auto & rInstance : m_ItemInstanceVector)
		ZeroMemory(&rInstance, sizeof(rInstance));
}

void CPythonSafeBox::SetItemData(uint32_t dwSlotIndex, const TItemData & rItemInstance)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::SetItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	m_ItemInstanceVector[dwSlotIndex] = rItemInstance;
}

void CPythonSafeBox::DelItemData(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::DelItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	TItemData & rInstance = m_ItemInstanceVector[dwSlotIndex];
	ZeroMemory(&rInstance, sizeof(rInstance));
}

#ifdef ENABLE_SAFEBOX_MONEY
void CPythonSafeBox::SetMoney(uint32_t dwMoney)
{
	m_dwMoney = dwMoney;
}

uint32_t CPythonSafeBox::GetMoney() const
{
	return m_dwMoney;
}
#endif

int CPythonSafeBox::GetCurrentSafeBoxSize() const
{
	return m_ItemInstanceVector.size();
}

BOOL CPythonSafeBox::GetSlotItemID(uint32_t dwSlotIndex, uint32_t * pdwItemID)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetSlotItemID(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*pdwItemID = m_ItemInstanceVector[dwSlotIndex].vnum;

	return TRUE;
}

BOOL CPythonSafeBox::GetItemDataPtr(uint32_t dwSlotIndex, TItemData ** ppInstance)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*ppInstance = &m_ItemInstanceVector[dwSlotIndex];

	return TRUE;
}

void CPythonSafeBox::OpenMall(int iSize)
{
	m_MallItemInstanceVector.clear();
	m_MallItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

	for (auto & rInstance : m_MallItemInstanceVector)
		ZeroMemory(&rInstance, sizeof(rInstance));
}

void CPythonSafeBox::SetMallItemData(uint32_t dwSlotIndex, const TItemData & rItemData)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::SetMallItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	m_MallItemInstanceVector[dwSlotIndex] = rItemData;
}

void CPythonSafeBox::DelMallItemData(uint32_t dwSlotIndex)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::DelMallItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	TItemData & rInstance = m_MallItemInstanceVector[dwSlotIndex];
	ZeroMemory(&rInstance, sizeof(rInstance));
}

BOOL CPythonSafeBox::GetMallItemDataPtr(uint32_t dwSlotIndex, TItemData ** ppInstance)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*ppInstance = &m_MallItemInstanceVector[dwSlotIndex];

	return TRUE;
}

BOOL CPythonSafeBox::GetSlotMallItemID(uint32_t dwSlotIndex, uint32_t * pdwItemID)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*pdwItemID = m_MallItemInstanceVector[dwSlotIndex].vnum;

	return TRUE;
}

uint32_t CPythonSafeBox::GetMallSize() const
{
	return m_MallItemInstanceVector.size();
}

CPythonSafeBox::CPythonSafeBox()
{
	isOpen = 0;
#ifdef ENABLE_SAFEBOX_MONEY
	m_dwMoney = 0;
#endif
}

CPythonSafeBox::~CPythonSafeBox() = default;

PyObject * safeboxGetCurrentSafeboxSize(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().GetCurrentSafeBoxSize());
}

PyObject * safeboxGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject * safeboxGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->count);
}

PyObject * safeboxGetItemFlags(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->flags);
}

PyObject * safeboxGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	TItemData * pItemData;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject * safeboxGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		TItemData * pItemData;
		if (CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].wType, pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

#ifdef ENABLE_SAFEBOX_MONEY
PyObject * safeboxGetMoney(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().GetMoney());
}
#endif

PyObject * safeboxGetMallItemID(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject * safeboxGetMallItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->count);
}

PyObject * safeboxGetMallItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	TItemData * pItemData;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject * safeboxGetMallItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		TItemData * pItemData;
		if (CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].wType, pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject * safeboxGetMallSize(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().GetMallSize());
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject* safeboxGetItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	TItemData* pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->dwTransmutationVnum);
}

PyObject* safeboxGetMallItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	TItemData* pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->dwTransmutationVnum);
}
#endif

PyObject * safeboxIsOpen(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().IsOpen());
}

PyObject * safeboxOpen(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSafeBox::Instance().IsOpen(true);
	return Py_BuildNone();
}
PyObject * safeboxClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSafeBox::Instance().IsOpen(false);
	return Py_BuildNone();
}

#ifdef ENABLE_REFINE_ELEMENT
PyObject* safeboxGetElements(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData* pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	if (pInstance->grade_element > 0)
	{
		return Py_BuildValue("iiii", pInstance->grade_element, pInstance->attack_element[pInstance->grade_element - 1], pInstance->element_type_bonus, pInstance->elements_value_bonus[pInstance->grade_element - 1]);
	}

	return Py_BuildValue("iiii", 0, 0, 0, 0);
}
#endif

#ifdef ENABLE_YOHARA_SYSTEM
PyObject* safeboxGetItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < APPLY_RANDOM_SLOT_MAX_NUM)
	{
		TItemData* pItemData;
		if (CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aApplyRandom[iAttrSlotIndex].wType, pItemData->aApplyRandom[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* safeboxGetMallItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < APPLY_RANDOM_SLOT_MAX_NUM)
	{
		TItemData* pItemData;
		if (CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aApplyRandom[iAttrSlotIndex].wType, pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* safeboxGetRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iRandomValueIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
		return Py_BadArgument();

	if (iRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM)
		return Py_BuildException();

	TItemData* pItemData;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alRandomValues[iRandomValueIndex]);
}

PyObject* safeboxGetMallRandomValue(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iRandomValueIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iRandomValueIndex))
		return Py_BadArgument();

	if (iRandomValueIndex >= ITEM_RANDOM_VALUES_MAX_NUM)
		return Py_BuildException();

	TItemData* pItemData;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alRandomValues[iRandomValueIndex]);
}
#endif

#ifdef ENABLE_SET_ITEM
PyObject* safeboxGetItemSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();

	TItemData* pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iPos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->set_value);
}
#endif

void initsafebox()
{
	static PyMethodDef s_methods[] = {// SafeBox
									  {"GetCurrentSafeboxSize", safeboxGetCurrentSafeboxSize, METH_VARARGS},
									  {"GetItemID", safeboxGetItemID, METH_VARARGS},
									  {"GetItemCount", safeboxGetItemCount, METH_VARARGS},
									  {"GetItemFlags", safeboxGetItemFlags, METH_VARARGS},
									  {"GetItemMetinSocket", safeboxGetItemMetinSocket, METH_VARARGS},
									  {"GetItemAttribute", safeboxGetItemAttribute, METH_VARARGS},
#ifdef ENABLE_SAFEBOX_MONEY
									  {"GetMoney", safeboxGetMoney, METH_VARARGS},
#endif

									  // Mall
									  {"GetMallItemID", safeboxGetMallItemID, METH_VARARGS},
									  {"GetMallItemCount", safeboxGetMallItemCount, METH_VARARGS},
									  {"GetMallItemMetinSocket", safeboxGetMallItemMetinSocket, METH_VARARGS},
									  {"GetMallItemAttribute", safeboxGetMallItemAttribute, METH_VARARGS},
									  {"GetMallSize", safeboxGetMallSize, METH_VARARGS},

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
									  {"GetItemChangeLookVnum",		safeboxGetItemChangeLookVnum,			METH_VARARGS },
									  {"GetMallItemChangeLookVnum",	safeboxGetMallItemChangeLookVnum,		METH_VARARGS },
#endif

									  {"Open",						safeboxOpen,							METH_VARARGS },
									  {"Close",						safeboxClose,							METH_VARARGS },
									  {"IsOpen",						safeboxIsOpen,							METH_VARARGS },

#ifdef ENABLE_REFINE_ELEMENT
									  {"GetElements",				safeboxGetElements,						METH_VARARGS },
#endif

#ifdef ENABLE_YOHARA_SYSTEM
									  {"GetItemApplyRandom", safeboxGetItemApplyRandom, METH_VARARGS},
									  {"GetMallItemApplyRandom", safeboxGetItemApplyRandom, METH_VARARGS},

									  {"GetRandomValue", safeboxGetRandomValue, METH_VARARGS},
									  {"GetMallRandomValue", safeboxGetMallItemApplyRandom, METH_VARARGS},
#endif
#ifdef ENABLE_SET_ITEM
									  {"GetItemSetValue", safeboxGetItemSetValue, METH_VARARGS},
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("safebox", s_methods);
	PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_X_COUNT", CPythonSafeBox::SAFEBOX_SLOT_X_COUNT);
	PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_Y_COUNT", CPythonSafeBox::SAFEBOX_SLOT_Y_COUNT);
	PyModule_AddIntConstant(poModule, "SAFEBOX_PAGE_SIZE", CPythonSafeBox::SAFEBOX_PAGE_SIZE);
}
