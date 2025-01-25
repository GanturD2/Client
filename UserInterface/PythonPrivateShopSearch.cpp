#include "stdafx.h"

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
#include "PythonPrivateShopSearch.h"

CPythonPrivateShopSearch::CPythonPrivateShopSearch()
{
}

CPythonPrivateShopSearch::~CPythonPrivateShopSearch()
{
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CPythonPrivateShopSearch::AddItemData(uint32_t vid, uint32_t price, uint32_t cheque, const TSearchItemData& rItemData)
#else
void CPythonPrivateShopSearch::AddItemData(uint32_t vid, uint32_t price, const TSearchItemData& rItemData)
#endif
{
	m_ItemInstanceVector.emplace_back(rItemData);
}

void CPythonPrivateShopSearch::ClearItemData()
{
	m_ItemInstanceVector.clear();
}

uint32_t CPythonPrivateShopSearch::GetItemDataPtr(uint32_t index, TSearchItemData** ppInstance)
{
	*ppInstance = &m_ItemInstanceVector.at(index);
	return 1;
}

PyObject* privateShopSearchGetSearchItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();

	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	CPythonPrivateShopSearch::TSearchItemData* pItemData;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject* privateShopSearchGetSearchItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		CPythonPrivateShopSearch::TSearchItemData* pItemData;
		if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].wType, pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* privateShopSearchGetItemCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPrivateShopSearch::Instance().GetItemDataCount());
}

PyObject* privateShopSearchGetSearchItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData * pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->count);
}

PyObject* privateShopSearchGetSearchItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->price);
}

#ifdef ENABLE_CHEQUE_SYSTEM
PyObject* privateShopSearchGetSearchItemCheque(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->cheque);
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM //SEARCHSHOP_CHANGELOOK
PyObject* privateShopSearchGetSearchItemLook(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData * pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->dwTransmutationVnum);
}
#endif

#ifdef ENABLE_REFINE_ELEMENT //SEARCHSHOP_ELEMENT
PyObject* privateShopSearchGetElement(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	if (pInstance->grade_element > 0)
		return Py_BuildValue("iiii", pInstance->grade_element, pInstance->attack_element[pInstance->grade_element - 1], pInstance->element_type_bonus, pInstance->elements_value_bonus[pInstance->grade_element - 1]);

	return Py_BuildValue("iiii", 0, 0, 0, 0);
}
#endif

#ifdef ENABLE_SUNGMA_ATTRIBUTE
PyObject* privateShopSearchGetSearchItemItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < APPLY_RANDOM_SLOT_MAX_NUM)
	{
		CPythonPrivateShopSearch::TSearchItemData* pItemData;
		if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aApplyRandom[iAttrSlotIndex].wType, pItemData->aApplyRandom[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}
#endif

PyObject* privateShopSearchGetSearchItemShopVID(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vid);
}

PyObject* privateShopSearchGetSearchItemPos(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->Cell.cell);
}

PyObject* privateShopSearchGetSearchItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	CPythonPrivateShopSearch::TSearchItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject* privateShopSearchClearSearchItems(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPrivateShopSearch::Instance().ClearItemData();
	return Py_BuildNone();
}

void initprivateShopSearch()
{
	static PyMethodDef s_methods[] = {{"GetItemCount", privateShopSearchGetItemCount, METH_VARARGS},
									  {"GetSearchItemCount", privateShopSearchGetSearchItemCount, METH_VARARGS},
									  {"GetSearchItemPrice", privateShopSearchGetSearchItemPrice, METH_VARARGS},
#ifdef ENABLE_CHEQUE_SYSTEM
									  {"GetSearchItemCheque", privateShopSearchGetSearchItemCheque, METH_VARARGS},
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM //SEARCHSHOP_CHANGELOOK
									  {"GetSearchItemLook", privateShopSearchGetSearchItemLook, METH_VARARGS},
#endif
#ifdef ENABLE_REFINE_ELEMENT //SEARCHSHOP_ELEMENT
									  {"GetElement", privateShopSearchGetElement, METH_VARARGS},
#endif
#ifdef ENABLE_SUNGMA_ATTRIBUTE
									  {"GetSearchItemItemApplyRandom", privateShopSearchGetSearchItemItemApplyRandom, METH_VARARGS},
#endif
									  {"GetSearchItemVnum", privateShopSearchGetSearchItemVnum, METH_VARARGS},
									  {"GetSearchItemShopVID", privateShopSearchGetSearchItemShopVID, METH_VARARGS},
									  {"ClearSearchItems", privateShopSearchClearSearchItems, METH_VARARGS},
									  {"GetSearchItemMetinSocket", privateShopSearchGetSearchItemMetinSocket, METH_VARARGS},
									  {"GetSearchItemAttribute", privateShopSearchGetSearchItemAttribute, METH_VARARGS},
									  {"GetSearchItemPos", privateShopSearchGetSearchItemPos, METH_VARARGS},
									  {nullptr, nullptr, 0} };

	PyObject * poModule = Py_InitModule("privateShopSearch", s_methods);
}
#endif
