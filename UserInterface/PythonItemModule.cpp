#include "StdAfx.h"
#include "PythonItem.h"

#include "../GameLib/ItemManager.h"
#include "../GameLib/GameLibDefines.h"
#include "InstanceBase.h"
#include "AbstractApplication.h"

extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;

PyObject * itemSetUseSoundFileName(PyObject * poSelf, PyObject * poArgs)
{
	int iUseSound;
	if (!PyTuple_GetInteger(poArgs, 0, &iUseSound))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonItem & rkItem = CPythonItem::Instance();
	rkItem.SetUseSoundFileName(iUseSound, szFileName);
	return Py_BuildNone();
}

PyObject * itemSetDropSoundFileName(PyObject * poSelf, PyObject * poArgs)
{
	int iDropSound;
	if (!PyTuple_GetInteger(poArgs, 0, &iDropSound))
		return Py_BadArgument();

	char * szFileName;
	if (!PyTuple_GetString(poArgs, 1, &szFileName))
		return Py_BadArgument();

	CPythonItem & rkItem = CPythonItem::Instance();
	rkItem.SetDropSoundFileName(iDropSound, szFileName);
	return Py_BuildNone();
}

PyObject * itemSelectItem(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BadArgument();

	if (!CItemManager::Instance().SelectItemData(iIndex))
	{
		TraceError("Cannot find item by %d", iIndex);
		CItemManager::Instance().SelectItemData(60001);
	}

	return Py_BuildNone();
}

PyObject* itemIsWeddingItem(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	switch (PyTuple_Size(poArgs))
	{
	case 0:
	{
		CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
		if (!pItemData)
			return Py_BuildException("No item selected!");
		iItemIndex = pItemData->GetIndex();
		break;
	}
	case 1:
	{
		if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
			return Py_BuildException();
		break;
	}
	default:
		return Py_BuildException();
	}
	//if ((dwItemIndex == 50201 || dwItemIndex == 50202) || (dwItemIndex >= 11901 && dwItemIndex <= 11914))
	//	return Py_BuildValue("i", TRUE);

	switch (iItemIndex)
	{
		case CItemData::WEDDING_TUXEDO1:
		case CItemData::WEDDING_TUXEDO2:
		case CItemData::WEDDING_TUXEDO3:
		case CItemData::WEDDING_TUXEDO4:
		case CItemData::WEDDING_BRIDE_DRESS1:
		case CItemData::WEDDING_BRIDE_DRESS2:
		case CItemData::WEDDING_BRIDE_DRESS3:
		case CItemData::WEDDING_BRIDE_DRESS4:
		case CItemData::WEDDING_BOUQUET1:
		case CItemData::WEDDING_BOUQUET2:
			return Py_BuildValue("i", TRUE);
			break;
	}

	return Py_BuildValue("i", FALSE);
}

#ifdef ENABLE_ATTR_6TH_7TH
PyObject* itemGetAttr67MaterialVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->IsAttr67MaterialVnum());
}
#endif

PyObject* itemGetItemNameByVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	if (!CItemManager::Instance().SelectItemData(iItemIndex))
	{
		TraceError("Cannot find item by vnum %lu", iItemIndex);
		return Py_BuildValue("s", "");
	}

	return Py_BuildValue("s", CItemManager::Instance().GetSelectedItemDataPointer()->GetName());
}

PyObject * itemGetItemName(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("s", pItemData->GetName());
}

PyObject * itemGetCurrentItemVnum(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetIndex());
}

PyObject * itemGetRefinedVnum(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetRefinedVnum());
}

PyObject * itemGetItemDescription(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("s", pItemData->GetDescription());
}

PyObject * itemGetItemSummary(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("s", pItemData->GetSummary());
}

PyObject * itemGetIconImage(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetIconImage());
}

PyObject * itemGetIconImageFileName(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	CGraphicSubImage * pImage = pItemData->GetIconImage();
	if (!pImage)
		return Py_BuildValue("s", "Noname");

	return Py_BuildValue("s", pImage->GetFileName());
}

PyObject * itemGetItemSize(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("(ii)", 1, pItemData->GetSize());
}

PyObject * itemGetItemType(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetType());
}

PyObject * itemGetItemSubType(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetSubType());
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
PyObject* itemIsAcceScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if ((pItemData->GetType() != CItemData::ITEM_TYPE_USE)
		|| (pItemData->GetSubType() != CItemData::USE_SPECIAL)
		|| (pItemData->GetIndex() != 90000 && pItemData->GetIndex() != 39046))
		return Py_BuildValue("i", FALSE);
	else
		return Py_BuildValue("i", TRUE);
}

PyObject* itemIsWedddingItem(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	if (iItemIndex == 50201 || iItemIndex == 50202)
		return Py_BuildValue("i", 1);
	if (iItemIndex < 11901 || iItemIndex > 11914)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", 1);
}
#endif

#ifdef ENABLE_SET_ITEM
PyObject* itemIsSetItemScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if ((pItemData->GetType() != CItemData::ITEM_TYPE_USE) || (pItemData->GetSubType() != CItemData::USE_SPECIAL) || (pItemData->GetIndex() != 39115))
		return Py_BuildValue("i", FALSE);
	else
		return Py_BuildValue("i", TRUE);
}
#endif

#ifdef ENABLE_PROTO_RENEWAL
PyObject* itemGetApplyValue(PyObject* poSelf, PyObject* poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemApply ItemApply;
	if (!pItemData->GetApply(iValueIndex, &ItemApply))
		return Py_BuildException();

	return Py_BuildValue("i", ItemApply.lValue);
}

PyObject* itemGetMaskType(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetMaskType());
}

PyObject* itemGetMaskSubType(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetMaskSubType());
}

PyObject* itemGetVnum(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetIndex());
}
#endif

PyObject * itemGetIBuyItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetIBuyItemPrice());
}

PyObject * itemGetISellItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetISellItemPrice());
}


PyObject * itemIsAntiFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsAntiFlag(iFlag));
}

PyObject * itemIsFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsFlag(iFlag));
}

PyObject * itemIsWearableFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsWearableFlag(iFlag));
}

PyObject * itemIs1GoldItem(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->IsFlag(CItemData::ITEM_FLAG_COUNT_PER_1GOLD));
}

PyObject * itemGetLimit(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemLimit ItemLimit;
	if (!pItemData->GetLimit(iValueIndex, &ItemLimit))
		return Py_BuildException();

	return Py_BuildValue("ii", ItemLimit.bType, ItemLimit.lValue);
}

PyObject * itemGetAffect(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemApply ItemApply;
	if (!pItemData->GetApply(iValueIndex, &ItemApply))
		return Py_BuildException();

	if ((CItemData::APPLY_ATT_SPEED == ItemApply.wType) && (CItemData::ITEM_TYPE_WEAPON == pItemData->GetType()) &&
		(CItemData::WEAPON_TWO_HANDED == pItemData->GetSubType()))
		ItemApply.lValue -= TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE;

	return Py_BuildValue("ii", ItemApply.wType, ItemApply.lValue);
}

#ifdef ENABLE_DETAILS_UI
PyObject * itemGetApplyPoint(PyObject* poSelf, PyObject* poArgs)
{
	int applyType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &applyType))
		return Py_BadArgument();

	return Py_BuildValue("i", ApplyTypeToPointType((uint16_t)applyType));	//@fixme436
}
/*PyObject * itemGetApplyPoint(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex; 
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex)) 
		return Py_BadArgument(); 

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer(); 
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CItemData::TItemApply ItemApply;
	if (!pItemData->GetApply(iValueIndex, &ItemApply))
		return Py_BuildException();

	return Py_BuildValue("ii", ItemApply.bType, ItemApply.lValue); }*/
#endif

PyObject * itemGetValue(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetValue(iValueIndex));
}

PyObject * itemGetSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iValueIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iValueIndex))
		return Py_BadArgument();

	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->GetSocket(iValueIndex));
}

PyObject * itemGetIconInstance(PyObject * poSelf, PyObject * poArgs)
{
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	CGraphicSubImage * pImage = pItemData->GetIconImage();
	if (!pImage)
		return Py_BuildException("Cannot get icon image by %d", pItemData->GetIndex());

	CGraphicImageInstance * pImageInstance = CGraphicImageInstance::New();
	pImageInstance->SetImagePointer(pImage);

	return Py_BuildValue("i", pImageInstance);
}

PyObject * itemDeleteIconInstance(PyObject * poSelf, PyObject * poArgs)
{
	int iHandle;
	if (!PyTuple_GetInteger(poArgs, 0, &iHandle))
		return Py_BadArgument();

	CGraphicImageInstance::Delete((CGraphicImageInstance *) iHandle);

	return Py_BuildNone();
}

PyObject * itemIsEquipmentVID(PyObject * poSelf, PyObject * poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Not yet select item data");

	return Py_BuildValue("i", pItemData->IsEquipment());
}

// 2005.05.20.myevan.통합 USE_TYPE 체크
PyObject * itemGetUseType(PyObject * poSelf, PyObject * poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	return Py_BuildValue("s", pItemData->GetUseTypeString());
}

PyObject * itemIsRefineScroll(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (pItemData->GetType() != CItemData::ITEM_TYPE_USE)
		return Py_BuildValue("i", FALSE);

	switch (pItemData->GetSubType())
	{
		case CItemData::USE_TUNING:
			return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsDetachScroll(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	int iType = pItemData->GetType();
	int iSubType = pItemData->GetSubType();
	if (iType == CItemData::ITEM_TYPE_USE)
		if (iSubType == CItemData::USE_DETACHMENT)
			return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * itemCanAddToQuickSlotItem(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

#ifdef ENABLE_PROTO_RENEWAL
	if (pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_QUICKSLOT))
		return Py_BuildValue("i", FALSE);
#endif

#ifdef ENABLE_SOUL_SYSTEM
	if (CItemData::ITEM_TYPE_SOUL == pItemData->GetType())
		return Py_BuildValue("i", TRUE);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (CItemData::ITEM_TYPE_PET == pItemData->GetType() && CItemData::PET_UPBRINGING == pItemData->GetSubType())
		return Py_BuildValue("i", TRUE);
#endif

	if (CItemData::ITEM_TYPE_USE == pItemData->GetType() || CItemData::ITEM_TYPE_QUEST == pItemData->GetType())
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsKey(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_TREASURE_KEY == pItemData->GetType())
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * itemIsMetin(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (CItemData::ITEM_TYPE_METIN == pItemData->GetType())
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject * itemRender(PyObject * poSelf, PyObject * poArgs)
{
	CPythonItem::Instance().Render();
	return Py_BuildNone();
}

PyObject * itemUpdate(PyObject * poSelf, PyObject * poArgs)
{
	IAbstractApplication & rkApp = IAbstractApplication::GetSingleton();

	POINT ptMouse;
	rkApp.GetMousePosition(&ptMouse);

	CPythonItem::Instance().Update(ptMouse);
	return Py_BuildNone();
}

PyObject * itemCreateItem(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BadArgument();
	int iVirtualNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iVirtualNumber))
		return Py_BadArgument();

	float x;
	if (!PyTuple_GetFloat(poArgs, 2, &x))
		return Py_BadArgument();
	float y;
	if (!PyTuple_GetFloat(poArgs, 3, &y))
		return Py_BadArgument();
	float z;
	if (!PyTuple_GetFloat(poArgs, 4, &z))
		return Py_BadArgument();

	bool bDrop = true;
	PyTuple_GetBoolean(poArgs, 5, &bDrop);

	CPythonItem::Instance().CreateItem(iVirtualID, iVirtualNumber, x, y, z, bDrop);

	return Py_BuildNone();
}

PyObject * itemDeleteItem(PyObject * poSelf, PyObject * poArgs)
{
	int iVirtualID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
		return Py_BadArgument();

	CPythonItem::Instance().DeleteItem(iVirtualID);
	return Py_BuildNone();
}

PyObject * itemPick(PyObject * poSelf, PyObject * poArgs)
{
	uint32_t dwItemID;
	if (CPythonItem::Instance().GetPickedItemID(&dwItemID))
		return Py_BuildValue("i", dwItemID);

	return Py_BuildValue("i", -1);
}

PyObject * itemLoadItemTable(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BadArgument();

	CItemManager::Instance().LoadItemTable(szFileName);
	return Py_BuildNone();
}

//ENABLE_SEARCH_ITEM_DROP_ON_MOB
PyObject* itemGetItemNames(PyObject* poSelf, PyObject* poArgs)
{
	CItemManager::TItemMap m_ItemMap = CItemManager::Instance().GetItems();
	CItemManager::TItemMap::iterator f = m_ItemMap.begin();
	PyObject* dict = PyTuple_New(m_ItemMap.size());
	int i = 0;
	while (m_ItemMap.end() != f)
	{
		PyObject* item = PyDict_New();
		PyDict_SetItemString(item, "vnum", Py_BuildValue("i", f->second->GetIndex()));
		PyDict_SetItemString(item, "name", Py_BuildValue("s", f->second->GetName()));
		PyTuple_SetItem(dict, i++, item);
		f++;
	}
	return dict;
}

#ifdef ENABLE_DS_SET
PyObject* itemGetDSSetWeight(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &bDSType))
		return Py_BadArgument();

	uint8_t bDSGrade = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &bDSGrade))
		return Py_BadArgument();

	return Py_BuildValue("f", CItemManager::Instance().GetDSSetWeight(bDSType, bDSGrade));
}

PyObject* itemGetDSBasicApplyCount(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &bDSType))
		return Py_BadArgument();
	uint8_t bDSGrade = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &bDSGrade))
		return Py_BadArgument();

	return Py_BuildValue("i", CItemManager::Instance().GetDSBasicApplyCount(bDSType, bDSGrade));
}

PyObject* itemGetDSBasicApplyValue(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &bDSType))
		return Py_BadArgument();
	uint8_t bApplyType;
	if (!PyTuple_GetInteger(poArgs, 1, &bApplyType))
		return Py_BadArgument();

	return Py_BuildValue("i", CItemManager::Instance().GetDSBasicApplyValue(bDSType, bApplyType));
}

PyObject* itemGetDSAdditionalApplyValue(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bDSType;
	if (!PyTuple_GetInteger(poArgs, 0, &bDSType))
		return Py_BadArgument();
	uint8_t bApplyType;
	if (!PyTuple_GetInteger(poArgs, 1, &bApplyType))
		return Py_BadArgument();

	return Py_BuildValue("i", CItemManager::Instance().GetDSAdditionalApplyValue(bDSType, bApplyType));
}
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
PyObject* itemIsSealScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	int iType = pItemData->GetType();
	int iSubType = pItemData->GetSubType();

	if (iType == CItemData::ITEM_TYPE_USE && iSubType == CItemData::USE_BIND)
		return Py_BuildValue("i", TRUE);
	else if (iType == CItemData::ITEM_TYPE_USE && iSubType == CItemData::USE_UNBIND)
		return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}

PyObject* itemGetUnlimitedSealDate(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", -1);
}
#endif

#ifdef ENABLE_QUIVER_SYSTEM
PyObject * itemGetSocketCountByVnum(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iItemIndex, &pItemData))
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pItemData->GetSocketCount());
}
#endif

#ifdef ENABLE_SOUL_SYSTEM
PyObject * itemIsSoulItem(PyObject * poSelf, PyObject * poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if (pItemData->GetType() == CItemData::ITEM_TYPE_SOUL)
		return Py_BuildValue("i", true);

	return Py_BuildValue("i", false);
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
PyObject* itemIsElement(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	int iType = pItemData->GetType();
	int iSubType = pItemData->GetSubType();
	if (iType == CItemData::ITEM_TYPE_USE)
		if (iSubType == CItemData::USE_ELEMENT_UPGRADE || iSubType == CItemData::USE_ELEMENT_DOWNGRADE || iSubType == CItemData::USE_ELEMENT_CHANGE)
			return Py_BuildValue("i", TRUE);

	return Py_BuildValue("i", FALSE);
}
#endif

#ifdef ENABLE_COMPARE_TOOLTIP
static const std::map<uint8_t, std::map<uint8_t, uint8_t>> m_mapItemSlotIndex =
{
	{
		CItemData::ITEM_TYPE_ARMOR,
		{
			{ CItemData::ARMOR_BODY, c_Equipment_Body },
			{ CItemData::ARMOR_HEAD, c_Equipment_Head },
			{ CItemData::ARMOR_SHIELD, c_Equipment_Shield },
			{ CItemData::ARMOR_WRIST, c_Equipment_Wrist },
			{ CItemData::ARMOR_FOOTS, c_Equipment_Shoes },
			{ CItemData::ARMOR_NECK, c_Equipment_Neck },
			{ CItemData::ARMOR_EAR, c_Equipment_Ear },
			#if defined(ENABLE_PENDANT)
			{ CItemData::ARMOR_PENDANT, c_Equipment_Pendant },
			#endif
		}
	},

	{
		CItemData::ITEM_TYPE_COSTUME,
		{
			{ CItemData::COSTUME_BODY, c_Costume_Slot_Body },
			{ CItemData::COSTUME_HAIR, c_Costume_Slot_Hair },
			#if defined(ENABLE_MOUNT_COSTUME_SYSTEM)
			{ CItemData::COSTUME_MOUNT, c_Costume_Slot_Mount },
			#endif	
			#if defined(ENABLE_ACCE_COSTUME_SYSTEM)
			{ CItemData::COSTUME_ACCE, c_Costume_Slot_Acce },
			#endif
			#if defined(ENABLE_WEAPON_COSTUME_SYSTEM)
			{ CItemData::COSTUME_WEAPON, c_Costume_Slot_Weapon },
			#endif
		}
	},
};

PyObject* itemGetCompareIndex(PyObject* poSelf, PyObject* poArgs)
{
	int iVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &iVnum))
		return Py_BadArgument();

	CItemData * pItemData;
	CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData);

	const auto& bItemType = pItemData->GetType();
	const auto& bItemSubType = pItemData->GetSubType();

	switch (bItemType)
	{
		case CItemData::ITEM_TYPE_WEAPON:
			return Py_BuildValue("i", c_Equipment_Weapon);
		case CItemData::ITEM_TYPE_BELT:
			return Py_BuildValue("i", c_Equipment_Belt);
		default:
			const auto& itType = m_mapItemSlotIndex.find(bItemType);
			if (itType != m_mapItemSlotIndex.end())
			{
				const auto& itSubType = itType->second.find(bItemSubType);
				if (itSubType != itType->second.end())
					return Py_BuildValue("i", itSubType->second);
			}
	};

	return Py_BuildValue("i", 0);
}
#endif

#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_BELT_INVENTORY)
PyObject* itemIsItemUsedForDragonSoul(PyObject* poSelf, PyObject* poArgs)
{
	int iItemIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemIndex))
		return Py_BadArgument();

	uint8_t byWndType;
	if (!PyTuple_GetByte(poArgs, 1, &byWndType))
		byWndType = 0;

	CItemManager::Instance().SelectItemData(iItemIndex);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	if ((pItemData->GetType() == CItemData::ITEM_TYPE_USE) &&
		(pItemData->GetSubType() == CItemData::USE_TIME_CHARGE_PER ||
			pItemData->GetSubType() == CItemData::USE_TIME_CHARGE_FIX ||
			(pItemData->GetSubType() == 33 && byWndType == DRAGON_SOUL_INVENTORY)))
		return Py_BuildValue("i", TRUE);

	if (pItemData->GetType() == CItemData::ITEM_TYPE_EXTRACT)
	{
		if (byWndType == INVENTORY)
		{
			if (pItemData->GetSubType() == CItemData::EXTRACT_DRAGON_SOUL)
				return Py_BuildValue("i", TRUE);
		}
		else if (byWndType == DRAGON_SOUL_INVENTORY)
		{
			if (pItemData->GetSubType() == CItemData::EXTRACT_DRAGON_HEART)
				return Py_BuildValue("i", TRUE);
		}
		else
			return Py_BuildValue("i", TRUE);
	}

	return Py_BuildValue("i", FALSE);
}
#endif

#ifdef ENABLE_INGAME_WIKI
PyObject* itemGetItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetIndex());
}

PyObject* itemGetItemRefinedVnum(PyObject* poSelf, PyObject* poArgs)
{
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("no selected item data");

	return Py_BuildValue("i", pItemData->GetRefinedVnum());
}

PyObject* itemGetItemDataByNamePart(PyObject* poSelf, PyObject* poArgs)
{
	char* nameStart;
	if (!PyTuple_GetString(poArgs, 0, &nameStart))
		return Py_BadArgument();

	std::tuple<const char*, int> itemData = CItemManager::Instance().SelectByNamePart(nameStart);
	return Py_BuildValue("(si)", std::get<0>(itemData), std::get<1>(itemData));
}
#endif

#ifdef ENABLE_FLOWER_EVENT
PyObject* itemGetPointApply(PyObject* poSelf, PyObject* poArgs)
{
	uint16_t wPointType = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &wPointType))
		return Py_BadArgument();

	const uint16_t wApplyOn = PointTypeToApplyType(wPointType);

	return Py_BuildValue("i", wApplyOn);
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject* itemIsPossibleChangeLookLeft(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	return Py_BuildValue("b", CPythonItem::Instance().CanAddChangeLookItem(pItemData, nullptr));
}

PyObject* itemIsPossibleChangeLookRight(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	int iOtherItemVID;
	if (!PyTuple_GetInteger(poArgs, 1, &iOtherItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	CItemManager::Instance().SelectItemData(iOtherItemVID);
	CItemData* pOtherItemData = CItemManager::Instance().GetSelectedItemDataPointer();

	return Py_BuildValue("b", CPythonItem::Instance().CanAddChangeLookItem(pItemData, pOtherItemData));
}

PyObject* itemIsChangeLookFreePassYangItem(PyObject* poSelf, PyObject* poArgs)
{
	int dwVirtualNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &dwVirtualNumber))
		return Py_BadArgument();

	return Py_BuildValue("b", CPythonItem::Instance().CanAddChangeLookFreeItem(dwVirtualNumber));
}

PyObject* itemIsChangeLookClearScroll(PyObject* poSelf, PyObject* poArgs)
{
	int iItemVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemVID))
		return Py_BadArgument();

	CItemManager::Instance().SelectItemData(iItemVID);
	CItemData* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return Py_BuildException("Can't find select item data");

	return Py_BuildValue("b", CPythonItem::Instance().IsChangeLookClearScrollItem(pItemData->GetIndex()));
}
#endif

#if defined(ENABLE_AUTO_SYSTEM) || defined(ENABLE_FLOWER_EVENT)
#include "PythonPlayer.h"

PyObject * itemCheckAffect(PyObject* poSelf, PyObject* poArgs)
{
	int dwType;
	uint16_t wPointType;

	switch (PyTuple_Size(poArgs))
	{
		case 2:
			if (!PyTuple_GetInteger(poArgs, 0, &dwType))
				return Py_BadArgument();
			if (!PyTuple_GetInteger(poArgs, 1, &wPointType))
				return Py_BadArgument();
			break;

		default:
			return Py_BuildException();
	}

	const int iAffIndex = CPythonPlayer::Instance().GetAffectDataIndex(dwType, wPointType);
	return Py_BuildValue("b", iAffIndex != -1);
}

PyObject* itemGetAffectDuration(PyObject* poSelf, PyObject* poArgs)
{
	int dwType;
	if (!PyTuple_GetInteger(poArgs, 0, &dwType))
		return Py_BadArgument();

	return Py_BuildValue("b", CPythonPlayer::Instance().GetAffectDuration(dwType));
}
#endif

void initItem()
{
	static PyMethodDef s_methods[] = {{"SetUseSoundFileName", itemSetUseSoundFileName, METH_VARARGS},
									  {"SetDropSoundFileName", itemSetDropSoundFileName, METH_VARARGS},
									  {"SelectItem", itemSelectItem, METH_VARARGS},

									  {"GetItemName", itemGetItemName, METH_VARARGS},
		{ "IsWeddingItem",					itemIsWeddingItem,						METH_VARARGS },
#ifdef ENABLE_ATTR_6TH_7TH
		{ "GetAttr67MaterialVnum",			itemGetAttr67MaterialVnum,				METH_VARARGS},
#endif
		{ "GetItemNameByVnum",				itemGetItemNameByVnum,					METH_VARARGS },
									  {"GetItemDescription", itemGetItemDescription, METH_VARARGS},
									  {"GetItemSummary", itemGetItemSummary, METH_VARARGS},
									  {"GetIconImage", itemGetIconImage, METH_VARARGS},
									  {"GetIconImageFileName", itemGetIconImageFileName, METH_VARARGS},
									  {"GetItemSize", itemGetItemSize, METH_VARARGS},
									  {"GetItemType", itemGetItemType, METH_VARARGS},
									  {"GetItemSubType", itemGetItemSubType, METH_VARARGS},
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		{ "IsAcceScroll",					itemIsAcceScroll,						METH_VARARGS },
		{ "IsWedddingItem",					itemIsWedddingItem,						METH_VARARGS },
#endif
#ifdef ENABLE_SET_ITEM
		{ "IsSetItemScroll",					itemIsSetItemScroll,						METH_VARARGS },
#endif
#ifdef ENABLE_PROTO_RENEWAL
		{ "GetVnum",						itemGetVnum,							METH_VARARGS },
		{ "GetApplyValue",					itemGetApplyValue,						METH_VARARGS },
		{ "GetMaskType",					itemGetMaskType,						METH_VARARGS },
		{ "GetMaskSubType",					itemGetMaskSubType,						METH_VARARGS },
#endif
									  {"GetIBuyItemPrice", itemGetIBuyItemPrice, METH_VARARGS},
									  {"GetISellItemPrice", itemGetISellItemPrice, METH_VARARGS},
									  {"IsAntiFlag", itemIsAntiFlag, METH_VARARGS},
									  {"IsFlag", itemIsFlag, METH_VARARGS},
									  {"IsWearableFlag", itemIsWearableFlag, METH_VARARGS},
									  {"Is1GoldItem", itemIs1GoldItem, METH_VARARGS},
									  {"GetLimit", itemGetLimit, METH_VARARGS},
									  {"GetAffect", itemGetAffect, METH_VARARGS},
#ifdef ENABLE_DETAILS_UI
		{ "GetApplyPoint",					itemGetApplyPoint,						METH_VARARGS },
#endif
									  {"GetValue", itemGetValue, METH_VARARGS},
									  {"GetSocket", itemGetSocket, METH_VARARGS},
									  {"GetIconInstance", itemGetIconInstance, METH_VARARGS},
									  {"GetUseType", itemGetUseType, METH_VARARGS},
									  {"DeleteIconInstance", itemDeleteIconInstance, METH_VARARGS},
									  {"IsEquipmentVID", itemIsEquipmentVID, METH_VARARGS},
									  {"IsRefineScroll", itemIsRefineScroll, METH_VARARGS},
									  {"IsDetachScroll", itemIsDetachScroll, METH_VARARGS},
									  {"IsKey", itemIsKey, METH_VARARGS},
									  {"IsMetin", itemIsMetin, METH_VARARGS},
									  {"CanAddToQuickSlotItem", itemCanAddToQuickSlotItem, METH_VARARGS},

									  {"Update", itemUpdate, METH_VARARGS},
									  {"Render", itemRender, METH_VARARGS},
									  {"CreateItem", itemCreateItem, METH_VARARGS},
									  {"DeleteItem", itemDeleteItem, METH_VARARGS},
									  {"Pick", itemPick, METH_VARARGS},

									  {"LoadItemTable", itemLoadItemTable, METH_VARARGS},

		{ "GetNames",						itemGetItemNames,						METH_VARARGS },

#ifdef ENABLE_DS_SET
		{ "GetDSSetWeight",					itemGetDSSetWeight,						METH_VARARGS },
		{ "GetDSBasicApplyCount",			itemGetDSBasicApplyCount,				METH_VARARGS },
		{ "GetDSBasicApplyValue",			itemGetDSBasicApplyValue,				METH_VARARGS },
		{ "GetDSAdditionalApplyValue",		itemGetDSAdditionalApplyValue,			METH_VARARGS },
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
		{ "IsSealScroll",					itemIsSealScroll,						METH_VARARGS },
		{ "GetUnlimitedSealDate",			itemGetUnlimitedSealDate,				METH_VARARGS },
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		{ "GetSocketCountByVnum",			itemGetSocketCountByVnum,				METH_VARARGS },
#endif

		{ "GetVnum",						itemGetCurrentItemVnum,					METH_VARARGS },
		{ "GetRefinedVnum",					itemGetRefinedVnum,						METH_VARARGS },

#ifdef ENABLE_SOUL_SYSTEM
		{ "IsSoulItem",						itemIsSoulItem,							METH_VARARGS },
#endif

#ifdef ENABLE_REFINE_ELEMENT
		{ "IsElement",						itemIsElement,							METH_VARARGS },
#endif

#ifdef ENABLE_COMPARE_TOOLTIP
		{ "GetCompareIndex",				itemGetCompareIndex,					METH_VARARGS },
#endif

#if defined(ENABLE_EXTEND_INVEN_SYSTEM) || defined(ENABLE_BELT_INVENTORY)
		{ "IsItemUsedForDragonSoul",		itemIsItemUsedForDragonSoul,			METH_VARARGS },
#endif

#ifdef ENABLE_INGAME_WIKI
		{ "GetItemVnum",					itemGetItemVnum,						METH_VARARGS },
		{ "GetItemRefinedVnum",				itemGetItemRefinedVnum,					METH_VARARGS },
		{ "GetItemDataByNamePart",			itemGetItemDataByNamePart,				METH_VARARGS },
#endif

#if defined(ENABLE_AUTO_SYSTEM) || defined(ENABLE_FLOWER_EVENT)
									  { "CheckAffect",			itemCheckAffect,				METH_VARARGS },
									  { "GetAffectDuration",			itemGetAffectDuration,				METH_VARARGS },
#endif
#ifdef ENABLE_FLOWER_EVENT
									  {"GetPointApply", itemGetPointApply, METH_VARARGS},
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "IsPossibleChangeLookLeft",		itemIsPossibleChangeLookLeft,			METH_VARARGS },
		{ "IsPossibleChangeLookRight",		itemIsPossibleChangeLookRight,			METH_VARARGS },
		{ "IsChangeLookFreePassYangItem",	itemIsChangeLookFreePassYangItem,		METH_VARARGS },
		{ "IsChangeLookClearScroll",		itemIsChangeLookClearScroll,			METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("item", s_methods);

	PyModule_AddIntConstant(poModule, "USESOUND_ACCESSORY",			CPythonItem::USESOUND_ACCESSORY);
	PyModule_AddIntConstant(poModule, "USESOUND_ARMOR",				CPythonItem::USESOUND_ARMOR);
	PyModule_AddIntConstant(poModule, "USESOUND_BOW",				CPythonItem::USESOUND_BOW);
	PyModule_AddIntConstant(poModule, "USESOUND_DEFAULT",			CPythonItem::USESOUND_DEFAULT);
	PyModule_AddIntConstant(poModule, "USESOUND_WEAPON",			CPythonItem::USESOUND_WEAPON);
	PyModule_AddIntConstant(poModule, "USESOUND_POTION",			CPythonItem::USESOUND_POTION);
	PyModule_AddIntConstant(poModule, "USESOUND_PORTAL",			CPythonItem::USESOUND_PORTAL);

	PyModule_AddIntConstant(poModule, "DROPSOUND_ACCESSORY",		CPythonItem::DROPSOUND_ACCESSORY);
	PyModule_AddIntConstant(poModule, "DROPSOUND_ARMOR",			CPythonItem::DROPSOUND_ARMOR);
	PyModule_AddIntConstant(poModule, "DROPSOUND_BOW",				CPythonItem::DROPSOUND_BOW);
	PyModule_AddIntConstant(poModule, "DROPSOUND_DEFAULT",			CPythonItem::DROPSOUND_DEFAULT);
	PyModule_AddIntConstant(poModule, "DROPSOUND_WEAPON",			CPythonItem::DROPSOUND_WEAPON);

	PyModule_AddIntConstant(poModule, "EQUIPMENT_COUNT",			c_Equipment_Count);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_HEAD",				c_Equipment_Head);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_BODY",				c_Equipment_Body);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_WEAPON",			c_Equipment_Weapon);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_WRIST",			c_Equipment_Wrist);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SHOES",			c_Equipment_Shoes);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_NECK",				c_Equipment_Neck);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_EAR",				c_Equipment_Ear);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE1",			c_Equipment_Unique1);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_UNIQUE2",			c_Equipment_Unique2);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_ARROW",			c_Equipment_Arrow);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_SHIELD",			c_Equipment_Shield);

	//ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "EQUIPMENT_RING1",			c_Equipment_Ring1);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_RING2",			c_Equipment_Ring2);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_BELT",				c_Equipment_Belt);
	PyModule_AddIntConstant(poModule, "EQUIPMENT_PENDANT",			c_Equipment_Pendant);
#ifdef ENABLE_GLOVE_SYSTEM
	PyModule_AddIntConstant(poModule, "EQUIPMENT_GLOVE",			c_Equipment_Glove);
#endif
#	if defined(ENABLE_PROTO_RENEWAL_CUSTOM) && defined(ENABLE_PET_SYSTEM)
	PyModule_AddIntConstant(poModule, "EQUIPMENT_STANDARD_PET",		c_Equipment_Standard_Pet);
#	endif
	//END_ENABLE_NEW_EQUIPMENT_SYSTEM

	PyModule_AddIntConstant(poModule, "ITEM_TYPE_NONE",				CItemData::ITEM_TYPE_NONE);			//0
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_WEAPON",			CItemData::ITEM_TYPE_WEAPON);		//1
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ARMOR",			CItemData::ITEM_TYPE_ARMOR);		//2
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_USE",				CItemData::ITEM_TYPE_USE);			//3
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_AUTOUSE",			CItemData::ITEM_TYPE_AUTOUSE);		//4
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_MATERIAL",			CItemData::ITEM_TYPE_MATERIAL);		//5
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL",			CItemData::ITEM_TYPE_SPECIAL);		//6
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TOOL",				CItemData::ITEM_TYPE_TOOL);			//7
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_LOTTERY",			CItemData::ITEM_TYPE_LOTTERY);		//8
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ELK",				CItemData::ITEM_TYPE_ELK);			//9
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_METIN",			CItemData::ITEM_TYPE_METIN);		//10
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CONTAINER",		CItemData::ITEM_TYPE_CONTAINER);	//11
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_FISH",				CItemData::ITEM_TYPE_FISH);			//12
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_ROD",				CItemData::ITEM_TYPE_ROD);			//13
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_RESOURCE",			CItemData::ITEM_TYPE_RESOURCE);		//14
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_CAMPFIRE",			CItemData::ITEM_TYPE_CAMPFIRE);		//15
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_UNIQUE",			CItemData::ITEM_TYPE_UNIQUE);		//16
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLBOOK",		CItemData::ITEM_TYPE_SKILLBOOK);	//17
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_QUEST",			CItemData::ITEM_TYPE_QUEST);		//18
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_POLYMORPH",		CItemData::ITEM_TYPE_POLYMORPH);	//19
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_BOX",		CItemData::ITEM_TYPE_TREASURE_BOX);	//20
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TREASURE_KEY",		CItemData::ITEM_TYPE_TREASURE_KEY);	//21
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLFORGET",		CItemData::ITEM_TYPE_SKILLFORGET);	//22
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_GIFTBOX",			CItemData::ITEM_TYPE_GIFTBOX);		//23
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PICK",				CItemData::ITEM_TYPE_PICK);			//24
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_HAIR",				CItemData::ITEM_TYPE_HAIR);			//25
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_TOTEM",			CItemData::ITEM_TYPE_TOTEM);		//26
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_BLEND",			CItemData::ITEM_TYPE_BLEND);		//27
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_COSTUME",			CItemData::ITEM_TYPE_COSTUME);		//28
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_DS",				CItemData::ITEM_TYPE_DS);			//29
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SPECIAL_DS",		CItemData::ITEM_TYPE_SPECIAL_DS);	//30
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_EXTRACT",			CItemData::ITEM_TYPE_EXTRACT);		//31
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SECONDARY_COIN",	CItemData::ITEM_TYPE_SECONDARY_COIN);	//32
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_RING",				CItemData::ITEM_TYPE_RING);			//33
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_BELT",				CItemData::ITEM_TYPE_BELT);			//34
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PET",				CItemData::ITEM_TYPE_PET);			//35
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_MEDIUM",			CItemData::ITEM_TYPE_MEDIUM);		//36	ENABLE_MOVE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_GACHA",			CItemData::ITEM_TYPE_GACHA);		//37	ENABLE_BATTLE_FIELD
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_SOUL",				CItemData::ITEM_TYPE_SOUL);			//38	ENABLE_SOUL_SYSTEM
	PyModule_AddIntConstant(poModule, "ITEM_TYPE_PASSIVE",			CItemData::ITEM_TYPE_PASSIVE);		//38	ENABLE_PASSIVE_SYSTEM

	// Item Sub Type
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_BODY",			CItemData::COSTUME_BODY);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_HAIR",			CItemData::COSTUME_HAIR);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_MOUNT",			CItemData::COSTUME_MOUNT);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_ACCE",			CItemData::COSTUME_ACCE);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_WEAPON",		CItemData::COSTUME_WEAPON);
	PyModule_AddIntConstant(poModule, "COSTUME_TYPE_AURA",			CItemData::COSTUME_AURA);

	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_START",			c_Costume_Slot_Start);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_COUNT",			c_Costume_Slot_Count);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_BODY",			c_Costume_Slot_Body);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_HAIR",			c_Costume_Slot_Hair);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_MOUNT",			c_Costume_Slot_Mount);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_ACCE",			c_Costume_Slot_Acce);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_WEAPON",		c_Costume_Slot_Weapon);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_AURA",			c_Costume_Slot_Aura);
	PyModule_AddIntConstant(poModule, "COSTUME_SLOT_END",			c_Costume_Slot_End);

	//ENABLE_NEW_EQUIPMENT_SYSTEM
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_START",	c_Belt_Inventory_Slot_Start);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_COUNT",	c_Belt_Inventory_Slot_Count);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_END",	c_Belt_Inventory_Slot_End);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_WIDTH",	c_Belt_Inventory_Width);
	PyModule_AddIntConstant(poModule, "BELT_INVENTORY_SLOT_HEIGHT",	c_Belt_Inventory_Height);
	//END_ENABLE_NEW_EQUIPMENT_SYSTEM

	PyModule_AddIntConstant(poModule, "WEAPON_SWORD",				CItemData::WEAPON_SWORD);
	PyModule_AddIntConstant(poModule, "WEAPON_DAGGER",				CItemData::WEAPON_DAGGER);
	PyModule_AddIntConstant(poModule, "WEAPON_BOW",					CItemData::WEAPON_BOW);
	PyModule_AddIntConstant(poModule, "WEAPON_TWO_HANDED",			CItemData::WEAPON_TWO_HANDED);
	PyModule_AddIntConstant(poModule, "WEAPON_BELL",				CItemData::WEAPON_BELL);
	PyModule_AddIntConstant(poModule, "WEAPON_FAN",					CItemData::WEAPON_FAN);
	PyModule_AddIntConstant(poModule, "WEAPON_ARROW",				CItemData::WEAPON_ARROW);
	PyModule_AddIntConstant(poModule, "WEAPON_MOUNT_SPEAR",			CItemData::WEAPON_MOUNT_SPEAR);
	PyModule_AddIntConstant(poModule, "WEAPON_CLAW",				CItemData::WEAPON_CLAW);
	PyModule_AddIntConstant(poModule, "WEAPON_QUIVER",				CItemData::WEAPON_QUIVER);
	PyModule_AddIntConstant(poModule, "WEAPON_NUM_TYPES",			CItemData::WEAPON_NUM_TYPES);

	PyModule_AddIntConstant(poModule, "ARMOR_BODY",					CItemData::ARMOR_BODY);
	PyModule_AddIntConstant(poModule, "ARMOR_HEAD",					CItemData::ARMOR_HEAD);
	PyModule_AddIntConstant(poModule, "ARMOR_SHIELD",				CItemData::ARMOR_SHIELD);
	PyModule_AddIntConstant(poModule, "ARMOR_WRIST",				CItemData::ARMOR_WRIST);
	PyModule_AddIntConstant(poModule, "ARMOR_FOOTS",				CItemData::ARMOR_FOOTS);
	PyModule_AddIntConstant(poModule, "ARMOR_NECK",					CItemData::ARMOR_NECK);
	PyModule_AddIntConstant(poModule, "ARMOR_EAR",					CItemData::ARMOR_EAR);
	PyModule_AddIntConstant(poModule, "ARMOR_PENDANT",				CItemData::ARMOR_PENDANT);	// New
#ifdef ENABLE_GLOVE_SYSTEM
	PyModule_AddIntConstant(poModule, "ARMOR_GLOVE",				CItemData::ARMOR_GLOVE);
#endif

	PyModule_AddIntConstant(poModule, "USE_POTION",					CItemData::USE_POTION);					// 0
	PyModule_AddIntConstant(poModule, "USE_TALISMAN",				CItemData::USE_TALISMAN);				// 1
	PyModule_AddIntConstant(poModule, "USE_TUNING",					CItemData::USE_TUNING);					// 2
	PyModule_AddIntConstant(poModule, "USE_MOVE",					CItemData::USE_MOVE);					// 3
	PyModule_AddIntConstant(poModule, "USE_TREASURE_BOX",			CItemData::USE_TREASURE_BOX);			// 4
	PyModule_AddIntConstant(poModule, "USE_MONEYBAG",				CItemData::USE_MONEYBAG);				// 5
	PyModule_AddIntConstant(poModule, "USE_BAIT",					CItemData::USE_BAIT);					// 6
	PyModule_AddIntConstant(poModule, "USE_ABILITY_UP",				CItemData::USE_ABILITY_UP);				// 7
	PyModule_AddIntConstant(poModule, "USE_AFFECT",					CItemData::USE_AFFECT);					// 8
	PyModule_AddIntConstant(poModule, "USE_CREATE_STONE",			CItemData::USE_CREATE_STONE);			// 9
	PyModule_AddIntConstant(poModule, "USE_SPECIAL",				CItemData::USE_SPECIAL);				// 10
	PyModule_AddIntConstant(poModule, "USE_POTION_NODELAY",			CItemData::USE_POTION_NODELAY);			// 11
	PyModule_AddIntConstant(poModule, "USE_CLEAR",					CItemData::USE_CLEAR);					// 12
	PyModule_AddIntConstant(poModule, "USE_INVISIBILITY",			CItemData::USE_INVISIBILITY);			// 13
	PyModule_AddIntConstant(poModule, "USE_DETACHMENT",				CItemData::USE_DETACHMENT);				// 14
	PyModule_AddIntConstant(poModule, "USE_BUCKET",					CItemData::USE_BUCKET);					// 15
	PyModule_AddIntConstant(poModule, "USE_POTION_CONTINUE",		CItemData::USE_POTION_CONTINUE);		// 16
	PyModule_AddIntConstant(poModule, "USE_CLEAN_SOCKET",			CItemData::USE_CLEAN_SOCKET);			// 17
	PyModule_AddIntConstant(poModule, "USE_CHANGE_ATTRIBUTE",		CItemData::USE_CHANGE_ATTRIBUTE);		// 18
	PyModule_AddIntConstant(poModule, "USE_ADD_ATTRIBUTE",			CItemData::USE_ADD_ATTRIBUTE);			// 19
	PyModule_AddIntConstant(poModule, "USE_ADD_ACCESSORY_SOCKET",	CItemData::USE_ADD_ACCESSORY_SOCKET);	// 20
	PyModule_AddIntConstant(poModule, "USE_PUT_INTO_ACCESSORY_SOCKET",CItemData::USE_PUT_INTO_ACCESSORY_SOCKET);// 21
	PyModule_AddIntConstant(poModule, "USE_ADD_ATTRIBUTE2",			CItemData::USE_ADD_ATTRIBUTE2);			// 22
	PyModule_AddIntConstant(poModule, "USE_RECIPE",					CItemData::USE_RECIPE);					// 23
	PyModule_AddIntConstant(poModule, "USE_CHANGE_ATTRIBUTE2",		CItemData::USE_CHANGE_ATTRIBUTE2);		// 24
	PyModule_AddIntConstant(poModule, "USE_BIND",					CItemData::USE_BIND);					// 25
	PyModule_AddIntConstant(poModule, "USE_UNBIND",					CItemData::USE_UNBIND);					// 26
	PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_PER",		CItemData::USE_TIME_CHARGE_PER);		// 27
	PyModule_AddIntConstant(poModule, "USE_TIME_CHARGE_FIX",		CItemData::USE_TIME_CHARGE_FIX);		// 28
	PyModule_AddIntConstant(poModule, "USE_PUT_INTO_BELT_SOCKET",	CItemData::USE_PUT_INTO_BELT_SOCKET);	// 29
	PyModule_AddIntConstant(poModule, "USE_PUT_INTO_RING_SOCKET",	CItemData::USE_PUT_INTO_RING_SOCKET);	// 30
	PyModule_AddIntConstant(poModule, "USE_CHANGE_COSTUME_ATTR",	CItemData::USE_CHANGE_COSTUME_ATTR);	// 31
	PyModule_AddIntConstant(poModule, "USE_RESET_COSTUME_ATTR",		CItemData::USE_RESET_COSTUME_ATTR);		// 32
#ifdef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "USE_UNK_33",					CItemData::USE_UNK_33);					// 33
	PyModule_AddIntConstant(poModule, "USE_SELECT_ATTRIBUTE",		CItemData::USE_SELECT_ATTRIBUTE);		// 34	ENABLE_CHANGED_ATTR
	PyModule_AddIntConstant(poModule, "USE_FLOWER",					CItemData::USE_FLOWER);					// 35	ENABLE_FLOWER_EVENT
	PyModule_AddIntConstant(poModule, "USE_EMOTION_PACK",			CItemData::USE_EMOTION_PACK);			// 36	ENABLE_EXPRESSING_EMOTION
#	ifdef ENABLE_REFINE_ELEMENT
	PyModule_AddIntConstant(poModule, "USE_ELEMENT_UPGRADE",		CItemData::USE_ELEMENT_UPGRADE);		// 37
	PyModule_AddIntConstant(poModule, "USE_ELEMENT_DOWNGRADE",		CItemData::USE_ELEMENT_DOWNGRADE);		// 38
	PyModule_AddIntConstant(poModule, "USE_ELEMENT_CHANGE",			CItemData::USE_ELEMENT_CHANGE);			// 39
#	endif
	PyModule_AddIntConstant(poModule, "USE_CALL",					CItemData::USE_CALL);					// 40
	PyModule_AddIntConstant(poModule, "USE_POTION_TOWER",			CItemData::USE_POTION_TOWER);			// 41
	PyModule_AddIntConstant(poModule, "USE_POTION_NODELAY_TOWER",	CItemData::USE_POTION_NODELAY_TOWER);	// 42
	PyModule_AddIntConstant(poModule, "USE_REMOVE_AFFECT",			CItemData::USE_REMOVE_AFFECT);			// 43
	PyModule_AddIntConstant(poModule, "USE_EMOTION_TOWER",			CItemData::USE_EMOTION_TOWER);			// 44
	PyModule_AddIntConstant(poModule, "USE_SECRET_DUNGEON_SCROLL",	CItemData::USE_SECRET_DUNGEON_SCROLL);	// 45	
#endif

	// Custom
	PyModule_AddIntConstant(poModule, "USE_AGGREGATE_MONSTER", CItemData::USE_AGGREGATE_MONSTER);	
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
	PyModule_AddIntConstant(poModule, "USE_PUT_INTO_AURA_SOCKET",	CItemData::USE_PUT_INTO_AURA_SOCKET);
#endif

	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_NORMAL",	CItemData::MATERIAL_DS_REFINE_NORMAL);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_BLESSED",	CItemData::MATERIAL_DS_REFINE_BLESSED);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_REFINE_HOLLY",	CItemData::MATERIAL_DS_REFINE_HOLLY);
	PyModule_AddIntConstant(poModule, "MATERIAL_DS_CHANGE_ATTR",	CItemData::MATERIAL_DS_CHANGE_ATTR);
#ifdef ENABLE_PASSIVE_SYSTEM
	PyModule_AddIntConstant(poModule, "MATERIAL_PASSIVE_WEAPON",	CItemData::MATERIAL_PASSIVE_WEAPON);
	PyModule_AddIntConstant(poModule, "MATERIAL_PASSIVE_ARMOR",		CItemData::MATERIAL_PASSIVE_ARMOR);
	PyModule_AddIntConstant(poModule, "MATERIAL_PASSIVE_ACCE",		CItemData::MATERIAL_PASSIVE_ACCE);
	PyModule_AddIntConstant(poModule, "MATERIAL_PASSIVE_ELEMENT",	CItemData::MATERIAL_PASSIVE_ELEMENT);
#endif

	PyModule_AddIntConstant(poModule, "METIN_NORMAL",				CItemData::METIN_NORMAL);
	PyModule_AddIntConstant(poModule, "METIN_GOLD",					CItemData::METIN_GOLD);
#if defined(ENABLE_PROTO_RENEWAL) && defined(ENABLE_YOHARA_SYSTEM)
	PyModule_AddIntConstant(poModule, "METIN_SUNGMA",				CItemData::METIN_SUNGMA);
#endif

	PyModule_AddIntConstant(poModule, "LIMIT_NONE",					CItemData::LIMIT_NONE);
	PyModule_AddIntConstant(poModule, "LIMIT_LEVEL",				CItemData::LIMIT_LEVEL);
	PyModule_AddIntConstant(poModule, "LIMIT_STR",					CItemData::LIMIT_STR);
	PyModule_AddIntConstant(poModule, "LIMIT_DEX",					CItemData::LIMIT_DEX);
	PyModule_AddIntConstant(poModule, "LIMIT_INT",					CItemData::LIMIT_INT);
	PyModule_AddIntConstant(poModule, "LIMIT_CON",					CItemData::LIMIT_CON);
#ifndef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "LIMIT_PCBANG",				CItemData::LIMIT_PCBANG);
#endif
	PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME",			CItemData::LIMIT_REAL_TIME);
	PyModule_AddIntConstant(poModule, "LIMIT_REAL_TIME_START_FIRST_USE",	CItemData::LIMIT_REAL_TIME_START_FIRST_USE);
	PyModule_AddIntConstant(poModule, "LIMIT_TIMER_BASED_ON_WEAR",	CItemData::LIMIT_TIMER_BASED_ON_WEAR);
#ifdef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "LIMIT_NEWWORLD_LEVEL",		CItemData::LIMIT_NEWWORLD_LEVEL);
	PyModule_AddIntConstant(poModule, "LIMIT_DURATION",				CItemData::LIMIT_DURATION);
#endif
	PyModule_AddIntConstant(poModule, "LIMIT_TYPE_MAX_NUM",			CItemData::LIMIT_MAX_NUM);
	PyModule_AddIntConstant(poModule, "LIMIT_MAX_NUM",				CItemData::ITEM_LIMIT_MAX_NUM);

	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_FEMALE",		CItemData::ITEM_ANTIFLAG_FEMALE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MALE",			CItemData::ITEM_ANTIFLAG_MALE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WARRIOR",		CItemData::ITEM_ANTIFLAG_WARRIOR);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ASSASSIN",		CItemData::ITEM_ANTIFLAG_ASSASSIN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SURA",			CItemData::ITEM_ANTIFLAG_SURA);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SHAMAN",		CItemData::ITEM_ANTIFLAG_SHAMAN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GET",			CItemData::ITEM_ANTIFLAG_GET);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_DROP",			CItemData::ITEM_ANTIFLAG_DROP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SELL",			CItemData::ITEM_ANTIFLAG_SELL);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_A",		CItemData::ITEM_ANTIFLAG_EMPIRE_A);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_B",		CItemData::ITEM_ANTIFLAG_EMPIRE_B);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_EMPIRE_R",		CItemData::ITEM_ANTIFLAG_EMPIRE_R);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAVE",			CItemData::ITEM_ANTIFLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_GIVE",			CItemData::ITEM_ANTIFLAG_GIVE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PKDROP",		CItemData::ITEM_ANTIFLAG_PKDROP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_STACK",		CItemData::ITEM_ANTIFLAG_STACK);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_MYSHOP",		CItemData::ITEM_ANTIFLAG_MYSHOP);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_SAFEBOX",		CItemData::ITEM_ANTIFLAG_SAFEBOX);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_WOLFMAN",		CItemData::ITEM_ANTIFLAG_WOLFMAN);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_QUICKSLOT",	CItemData::ITEM_ANTIFLAG_QUICKSLOT);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_CHANGELOOK",	CItemData::ITEM_ANTIFLAG_CHANGELOOK);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_REINFORCE",	CItemData::ITEM_ANTIFLAG_REINFORCE);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ENCHANT",		CItemData::ITEM_ANTIFLAG_ENCHANT);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ENERGY",		CItemData::ITEM_ANTIFLAG_ENERGY);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_PETFEED",		CItemData::ITEM_ANTIFLAG_PETFEED);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_APPLY",		CItemData::ITEM_ANTIFLAG_APPLY);
	PyModule_AddIntConstant(poModule, "ITEM_ANTIFLAG_ACCE",			CItemData::ITEM_ANTIFLAG_ACCE);

	PyModule_AddIntConstant(poModule, "ITEM_FLAG_REFINEABLE",		CItemData::ITEM_FLAG_REFINEABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_SAVE",				CItemData::ITEM_FLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_STACKABLE",		CItemData::ITEM_FLAG_STACKABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_COUNT_PER_1GOLD",	CItemData::ITEM_FLAG_COUNT_PER_1GOLD);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_SLOW_QUERY",		CItemData::ITEM_FLAG_SLOW_QUERY);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_UNIQUE",			CItemData::ITEM_FLAG_UNIQUE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_MAKECOUNT",		CItemData::ITEM_FLAG_MAKECOUNT);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_IRREMOVABLE",		CItemData::ITEM_FLAG_IRREMOVABLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_CONFIRM_WHEN_USE",	CItemData::ITEM_FLAG_CONFIRM_WHEN_USE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_QUEST_USE",		CItemData::ITEM_FLAG_QUEST_USE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_QUEST_USE_MULTIPLE", CItemData::ITEM_FLAG_QUEST_USE_MULTIPLE);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_LOG",				CItemData::ITEM_FLAG_LOG);
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_APPLICABLE",		CItemData::ITEM_FLAG_APPLICABLE);	// UNUSED
	PyModule_AddIntConstant(poModule, "ITEM_FLAG_GROUP_DMG_WEAPON",	CItemData::ITEM_FLAG_GROUP_DMG_WEAPON);	//ENABLE_GROUP_DAMAGE_WEAPON_EFFECT

	PyModule_AddIntConstant(poModule, "ANTIFLAG_FEMALE",			CItemData::ITEM_ANTIFLAG_FEMALE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MALE",				CItemData::ITEM_ANTIFLAG_MALE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_WARRIOR",			CItemData::ITEM_ANTIFLAG_WARRIOR);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ASSASSIN",			CItemData::ITEM_ANTIFLAG_ASSASSIN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SURA",				CItemData::ITEM_ANTIFLAG_SURA);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SHAMAN",			CItemData::ITEM_ANTIFLAG_SHAMAN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_GET",				CItemData::ITEM_ANTIFLAG_GET);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_DROP",				CItemData::ITEM_ANTIFLAG_DROP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SELL",				CItemData::ITEM_ANTIFLAG_SELL);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_A",			CItemData::ITEM_ANTIFLAG_EMPIRE_A);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_B",			CItemData::ITEM_ANTIFLAG_EMPIRE_B);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_EMPIRE_R",			CItemData::ITEM_ANTIFLAG_EMPIRE_R);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SAVE",				CItemData::ITEM_ANTIFLAG_SAVE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_GIVE",				CItemData::ITEM_ANTIFLAG_GIVE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_PKDROP",			CItemData::ITEM_ANTIFLAG_PKDROP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_STACK",				CItemData::ITEM_ANTIFLAG_STACK);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_MYSHOP",			CItemData::ITEM_ANTIFLAG_MYSHOP);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_SAFEBOX",			CItemData::ITEM_ANTIFLAG_SAFEBOX);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_WOLFMAN",			CItemData::ITEM_ANTIFLAG_WOLFMAN);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_QUICKSLOT",			CItemData::ITEM_ANTIFLAG_QUICKSLOT);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_CHANGELOOK",		CItemData::ITEM_ANTIFLAG_CHANGELOOK);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_REINFORCE",			CItemData::ITEM_ANTIFLAG_REINFORCE);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ENCHANT",			CItemData::ITEM_ANTIFLAG_ENCHANT);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ENERGY",			CItemData::ITEM_ANTIFLAG_ENERGY);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_PETFEED",			CItemData::ITEM_ANTIFLAG_PETFEED);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_APPLY",				CItemData::ITEM_ANTIFLAG_APPLY);
	PyModule_AddIntConstant(poModule, "ANTIFLAG_ACCE",				CItemData::ITEM_ANTIFLAG_ACCE);

	PyModule_AddIntConstant(poModule, "WEARABLE_BODY",				CItemData::WEARABLE_BODY);
	PyModule_AddIntConstant(poModule, "WEARABLE_HEAD",				CItemData::WEARABLE_HEAD);
	PyModule_AddIntConstant(poModule, "WEARABLE_FOOTS",				CItemData::WEARABLE_FOOTS);
	PyModule_AddIntConstant(poModule, "WEARABLE_WRIST",				CItemData::WEARABLE_WRIST);
	PyModule_AddIntConstant(poModule, "WEARABLE_WEAPON",			CItemData::WEARABLE_WEAPON);
	PyModule_AddIntConstant(poModule, "WEARABLE_NECK",				CItemData::WEARABLE_NECK);
	PyModule_AddIntConstant(poModule, "WEARABLE_EAR",				CItemData::WEARABLE_EAR);
	PyModule_AddIntConstant(poModule, "WEARABLE_UNIQUE",			CItemData::WEARABLE_UNIQUE);
	PyModule_AddIntConstant(poModule, "WEARABLE_SHIELD",			CItemData::WEARABLE_SHIELD);
	PyModule_AddIntConstant(poModule, "WEARABLE_ARROW",				CItemData::WEARABLE_ARROW);
	PyModule_AddIntConstant(poModule, "WEARABLE_PENDANT",			CItemData::WEARABLE_PENDANT);
#ifdef ENABLE_GLOVE_SYSTEM
	PyModule_AddIntConstant(poModule, "WEARABLE_GLOVE",				CItemData::WEARABLE_GLOVE);
#endif

	PyModule_AddIntConstant(poModule, "ITEM_APPLY_MAX_NUM",			CItemData::ITEM_APPLY_MAX_NUM);
	PyModule_AddIntConstant(poModule, "ITEM_SOCKET_MAX_NUM",		CItemData::ITEM_SOCKET_MAX_NUM);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "PET_NAME_MIN_SIZE",			CItemData::PET_NAME_MIN_SIZE);
	PyModule_AddIntConstant(poModule, "PET_NAME_MAX_SIZE",			CItemData::PET_NAME_MAX_SIZE);
	PyModule_AddIntConstant(poModule, "PET_HATCHING_MONEY",			CItemData::PET_HATCHING_MONEY);

	PyModule_AddIntConstant(poModule, "PET_EGG",					CItemData::PET_EGG);
	PyModule_AddIntConstant(poModule, "PET_UPBRINGING",				CItemData::PET_UPBRINGING);
	PyModule_AddIntConstant(poModule, "PET_BAG",					CItemData::PET_BAG);
	PyModule_AddIntConstant(poModule, "PET_FEEDSTUFF",				CItemData::PET_FEEDSTUFF);
	PyModule_AddIntConstant(poModule, "PET_SKILL",					CItemData::PET_SKILL);
	PyModule_AddIntConstant(poModule, "PET_SKILL_DEL_BOOK",			CItemData::PET_SKILL_DEL_BOOK);
	PyModule_AddIntConstant(poModule, "PET_NAME_CHANGE",			CItemData::PET_NAME_CHANGE);
	PyModule_AddIntConstant(poModule, "PET_EXPFOOD",				CItemData::PET_EXPFOOD);
	PyModule_AddIntConstant(poModule, "PET_SKILL_ALL_DEL_BOOK",		CItemData::PET_SKILL_ALL_DEL_BOOK);
	PyModule_AddIntConstant(poModule, "PET_EXPFOOD_PER",			CItemData::PET_EXPFOOD_PER);
	PyModule_AddIntConstant(poModule, "PET_ITEM_TYPE",				CItemData::PET_ITEM_TYPE);
	PyModule_AddIntConstant(poModule, "PET_ATTR_CHANGE",			CItemData::PET_ATTR_CHANGE);
	PyModule_AddIntConstant(poModule, "PET_PAY",					CItemData::PET_PAY);
	PyModule_AddIntConstant(poModule, "PET_PREMIUM_FEEDSTUFF",		CItemData::PET_PREMIUM_FEEDSTUFF);

	//CheckItemPet
	PyModule_AddIntConstant(poModule, "PET_EGG_USE_TRUE",						CItemData::PET_EGG_USE_TRUE);
	PyModule_AddIntConstant(poModule, "PET_EGG_USE_FAILED_BECAUSE_TRADING",		CItemData::PET_EGG_USE_FAILED_BECAUSE_TRADING);
	PyModule_AddIntConstant(poModule, "PET_EGG_USE_FAILED_BECAUSE_SHOP_OPEN",	CItemData::PET_EGG_USE_FAILED_BECAUSE_SHOP_OPEN);
	PyModule_AddIntConstant(poModule, "PET_EGG_USE_FAILED_BECAUSE_MALL_OPEN",	CItemData::PET_EGG_USE_FAILED_BECAUSE_MALL_OPEN);
	PyModule_AddIntConstant(poModule, "PET_EGG_USE_FAILED_BECAUSE_SAFEBOX_OPEN",CItemData::PET_EGG_USE_FAILED_BECAUSE_SAFEBOX_OPEN);

	//Results
	PyModule_AddIntConstant(poModule, "EGG_USE_SUCCESS",			CItemData::EGG_USE_SUCCESS);
	PyModule_AddIntConstant(poModule, "EGG_USE_FAILED_BECAUSE_NAME",CItemData::EGG_USE_FAILED_BECAUSE_NAME);
	PyModule_AddIntConstant(poModule, "EGG_USE_FAILED_TIMEOVER",	CItemData::EGG_USE_FAILED_TIMEOVER);

	PyModule_AddIntConstant(poModule, "NAME_CHANGE_USE_SUCCESS",				CItemData::NAME_CHANGE_USE_SUCCESS);
	PyModule_AddIntConstant(poModule, "NAME_CHANGE_USE_FAILED_BECAUSE_NAME",	CItemData::NAME_CHANGE_USE_FAILED_BECAUSE_NAME);
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	PyModule_AddIntConstant(poModule, "E_SEAL_DATE_DEFAULT_TIMESTAMP", CItemData::SEAL_DATE_DEFAULT_TIMESTAMP);
#endif
#ifdef ENABLE_SOUL_SYSTEM
	PyModule_AddIntConstant(poModule, "RED_SOUL",					CItemData::SOUL_RED);
	PyModule_AddIntConstant(poModule, "BLUE_SOUL",					CItemData::SOUL_BLUE);
#endif

	//BONUS_LIST
	PyModule_AddIntConstant(poModule, "APPLY_NONE",						CItemData::APPLY_NONE);							// 0 [POINT_NONE]
	PyModule_AddIntConstant(poModule, "APPLY_MAX_HP",					CItemData::APPLY_MAX_HP);						// 1
	PyModule_AddIntConstant(poModule, "APPLY_MAX_SP",					CItemData::APPLY_MAX_SP);						// 2
	PyModule_AddIntConstant(poModule, "APPLY_CON",						CItemData::APPLY_CON);							// 3
	PyModule_AddIntConstant(poModule, "APPLY_INT",						CItemData::APPLY_INT);							// 4
	PyModule_AddIntConstant(poModule, "APPLY_STR",						CItemData::APPLY_STR);							// 5
	PyModule_AddIntConstant(poModule, "APPLY_DEX",						CItemData::APPLY_DEX);							// 6
	PyModule_AddIntConstant(poModule, "APPLY_ATT_SPEED",				CItemData::APPLY_ATT_SPEED);					// 7
	PyModule_AddIntConstant(poModule, "APPLY_MOV_SPEED",				CItemData::APPLY_MOV_SPEED);					// 8
	PyModule_AddIntConstant(poModule, "APPLY_CAST_SPEED",				CItemData::APPLY_CAST_SPEED);					// 9
	PyModule_AddIntConstant(poModule, "APPLY_HP_REGEN",					CItemData::APPLY_HP_REGEN);						// 10
	PyModule_AddIntConstant(poModule, "APPLY_SP_REGEN",					CItemData::APPLY_SP_REGEN);						// 11
	PyModule_AddIntConstant(poModule, "APPLY_POISON_PCT",				CItemData::APPLY_POISON_PCT);					// 12
	PyModule_AddIntConstant(poModule, "APPLY_STUN_PCT", 				CItemData::APPLY_STUN_PCT);						// 13
	PyModule_AddIntConstant(poModule, "APPLY_SLOW_PCT", 				CItemData::APPLY_SLOW_PCT);						// 14
	PyModule_AddIntConstant(poModule, "APPLY_CRITICAL_PCT",				CItemData::APPLY_CRITICAL_PCT);					// 15
	PyModule_AddIntConstant(poModule, "APPLY_PENETRATE_PCT",			CItemData::APPLY_PENETRATE_PCT);				// 16
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_HUMAN",			CItemData::APPLY_ATTBONUS_HUMAN);				// 17
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ANIMAL",			CItemData::APPLY_ATTBONUS_ANIMAL);				// 18
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ORC",				CItemData::APPLY_ATTBONUS_ORC);					// 19
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MILGYO",			CItemData::APPLY_ATTBONUS_MILGYO);				// 20
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_UNDEAD",			CItemData::APPLY_ATTBONUS_UNDEAD);				// 21
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DEVIL", 			CItemData::APPLY_ATTBONUS_DEVIL);				// 22
	PyModule_AddIntConstant(poModule, "APPLY_STEAL_HP",					CItemData::APPLY_STEAL_HP);						// 23
	PyModule_AddIntConstant(poModule, "APPLY_STEAL_SP",					CItemData::APPLY_STEAL_SP);						// 24
	PyModule_AddIntConstant(poModule, "APPLY_MANA_BURN_PCT",			CItemData::APPLY_MANA_BURN_PCT);				// 25
	PyModule_AddIntConstant(poModule, "APPLY_DAMAGE_SP_RECOVER",		CItemData::APPLY_DAMAGE_SP_RECOVER);			// 26
	PyModule_AddIntConstant(poModule, "APPLY_BLOCK",					CItemData::APPLY_BLOCK);						// 27
	PyModule_AddIntConstant(poModule, "APPLY_DODGE",					CItemData::APPLY_DODGE);						// 28
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SWORD",				CItemData::APPLY_RESIST_SWORD);					// 29
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_TWOHAND",			CItemData::APPLY_RESIST_TWOHAND);				// 30
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_DAGGER",			CItemData::APPLY_RESIST_DAGGER);				// 31
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_BELL",				CItemData::APPLY_RESIST_BELL);					// 32
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_FAN",				CItemData::APPLY_RESIST_FAN);					// 33
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_BOW", 				CItemData::APPLY_RESIST_BOW);					// 34
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_FIRE", 				CItemData::APPLY_RESIST_FIRE);					// 35
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ELEC", 				CItemData::APPLY_RESIST_ELEC);					// 36
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC", 			CItemData::APPLY_RESIST_MAGIC);					// 37
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WIND",				CItemData::APPLY_RESIST_WIND);					// 38
	PyModule_AddIntConstant(poModule, "APPLY_REFLECT_MELEE",			CItemData::APPLY_REFLECT_MELEE);				// 39
	PyModule_AddIntConstant(poModule, "APPLY_REFLECT_CURSE",			CItemData::APPLY_REFLECT_CURSE);				// 40
	PyModule_AddIntConstant(poModule, "APPLY_POISON_REDUCE",			CItemData::APPLY_POISON_REDUCE);				// 41
	PyModule_AddIntConstant(poModule, "APPLY_KILL_SP_RECOVER",			CItemData::APPLY_KILL_SP_RECOVER);				// 42
	PyModule_AddIntConstant(poModule, "APPLY_EXP_DOUBLE_BONUS",			CItemData::APPLY_EXP_DOUBLE_BONUS);				// 43
	PyModule_AddIntConstant(poModule, "APPLY_GOLD_DOUBLE_BONUS",		CItemData::APPLY_GOLD_DOUBLE_BONUS);			// 44
	PyModule_AddIntConstant(poModule, "APPLY_ITEM_DROP_BONUS",			CItemData::APPLY_ITEM_DROP_BONUS);				// 45
	PyModule_AddIntConstant(poModule, "APPLY_POTION_BONUS",				CItemData::APPLY_POTION_BONUS);					// 46
	PyModule_AddIntConstant(poModule, "APPLY_KILL_HP_RECOVER",			CItemData::APPLY_KILL_HP_RECOVER);				// 47
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_STUN",				CItemData::APPLY_IMMUNE_STUN);					// 48
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_SLOW",				CItemData::APPLY_IMMUNE_SLOW);					// 49
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_FALL",				CItemData::APPLY_IMMUNE_FALL);					// 50
	PyModule_AddIntConstant(poModule, "APPLY_SKILL",					CItemData::APPLY_SKILL);						// 51 [unused - POINT_NONE]
	PyModule_AddIntConstant(poModule, "APPLY_BOW_DISTANCE", 			CItemData::APPLY_BOW_DISTANCE);					// 52
	PyModule_AddIntConstant(poModule, "APPLY_ATT_GRADE_BONUS",			CItemData::APPLY_ATT_GRADE_BONUS);				// 53
	PyModule_AddIntConstant(poModule, "APPLY_DEF_GRADE_BONUS",			CItemData::APPLY_DEF_GRADE_BONUS);				// 54
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATT_GRADE",			CItemData::APPLY_MAGIC_ATT_GRADE);				// 55
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_DEF_GRADE",			CItemData::APPLY_MAGIC_DEF_GRADE);				// 56
	PyModule_AddIntConstant(poModule, "APPLY_CURSE_PCT",				CItemData::APPLY_CURSE_PCT);					// 56
	PyModule_AddIntConstant(poModule, "APPLY_MAX_STAMINA",				CItemData::APPLY_MAX_STAMINA);					// 58
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WARRIOR",			CItemData::APPLY_ATT_BONUS_TO_WARRIOR);			// 59
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_ASSASSIN",		CItemData::APPLY_ATT_BONUS_TO_ASSASSIN);		// 60
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SURA",			CItemData::APPLY_ATT_BONUS_TO_SURA);			// 61
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SHAMAN",			CItemData::APPLY_ATT_BONUS_TO_SHAMAN);			// 62
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_MONSTER",			CItemData::APPLY_ATT_BONUS_TO_MONSTER);			// 63
	PyModule_AddIntConstant(poModule, "APPLY_MALL_ATTBONUS",			CItemData::APPLY_MALL_ATTBONUS);				// 64
	PyModule_AddIntConstant(poModule, "APPLY_MALL_DEFBONUS",			CItemData::APPLY_MALL_DEFBONUS);				// 65
	PyModule_AddIntConstant(poModule, "APPLY_MALL_EXPBONUS",			CItemData::APPLY_MALL_EXPBONUS);				// 66
	PyModule_AddIntConstant(poModule, "APPLY_MALL_ITEMBONUS",			CItemData::APPLY_MALL_ITEMBONUS);				// 67
	PyModule_AddIntConstant(poModule, "APPLY_MALL_GOLDBONUS",			CItemData::APPLY_MALL_GOLDBONUS);				// 68
	PyModule_AddIntConstant(poModule, "APPLY_MAX_HP_PCT",				CItemData::APPLY_MAX_HP_PCT);					// 69
	PyModule_AddIntConstant(poModule, "APPLY_MAX_SP_PCT",				CItemData::APPLY_MAX_SP_PCT);					// 70
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BONUS",		CItemData::APPLY_SKILL_DAMAGE_BONUS);			// 71
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DAMAGE_BONUS",	CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS);		// 72
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DEFEND_BONUS",		CItemData::APPLY_SKILL_DEFEND_BONUS);			// 73
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DEFEND_BONUS",	CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS);		// 74
	PyModule_AddIntConstant(poModule, "APPLY_PC_BANG_EXP_BONUS",		CItemData::APPLY_PC_BANG_EXP_BONUS);			// 75
	PyModule_AddIntConstant(poModule, "APPLY_PC_BANG_DROP_BONUS",		CItemData::APPLY_PC_BANG_DROP_BONUS);			// 76
	PyModule_AddIntConstant(poModule, "APPLY_EXTRACT_HP_PCT",			CItemData::APPLY_EXTRACT_HP_PCT);				// 77
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WARRIOR",			CItemData::APPLY_RESIST_WARRIOR );				// 78
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ASSASSIN",			CItemData::APPLY_RESIST_ASSASSIN );				// 79
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SURA",				CItemData::APPLY_RESIST_SURA );					// 80
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_SHAMAN",			CItemData::APPLY_RESIST_SHAMAN );				// 81
	PyModule_AddIntConstant(poModule, "APPLY_ENERGY",					CItemData::APPLY_ENERGY );						// 82
	PyModule_AddIntConstant(poModule, "APPLY_DEF_GRADE",				CItemData::APPLY_DEF_GRADE);					// 83
	PyModule_AddIntConstant(poModule, "APPLY_COSTUME_ATTR_BONUS",		CItemData::APPLY_COSTUME_ATTR_BONUS );			// 84
	PyModule_AddIntConstant(poModule, "APPLY_MAGIC_ATTBONUS_PER",		CItemData::APPLY_MAGIC_ATTBONUS_PER );			// 85
	PyModule_AddIntConstant(poModule, "APPLY_MELEE_MAGIC_ATTBONUS_PER",	CItemData::APPLY_MELEE_MAGIC_ATTBONUS_PER );	// 86
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_ICE",				CItemData::APPLY_RESIST_ICE );					// 87
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_EARTH",				CItemData::APPLY_RESIST_EARTH );				// 88
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_DARK",				CItemData::APPLY_RESIST_DARK );					// 89
	PyModule_AddIntConstant(poModule, "APPLY_ANTI_CRITICAL_PCT",		CItemData::APPLY_ANTI_CRITICAL_PCT );			// 90
	PyModule_AddIntConstant(poModule, "APPLY_ANTI_PENETRATE_PCT",		CItemData::APPLY_ANTI_PENETRATE_PCT );			// 91
	PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_REDUCE",			CItemData::APPLY_BLEEDING_REDUCE );				// 92
	PyModule_AddIntConstant(poModule, "APPLY_BLEEDING_PCT",				CItemData::APPLY_BLEEDING_PCT );				// 93
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_WOLFMAN",			CItemData::APPLY_ATT_BONUS_TO_WOLFMAN);			// 94
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_WOLFMAN",			CItemData::APPLY_RESIST_WOLFMAN );				// 95
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_CLAW",				CItemData::APPLY_RESIST_CLAW );					// 96
	PyModule_AddIntConstant(poModule, "APPLY_ACCEDRAIN_RATE",			CItemData::APPLY_ACCEDRAIN_RATE);				// 97
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MAGIC_REDUCTION",	CItemData::APPLY_RESIST_MAGIC_REDUCTION);		// 98
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ELECT",			CItemData::APPLY_ENCHANT_ELECT);				// 99
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_FIRE",				CItemData::APPLY_ENCHANT_FIRE);					// 100
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_ICE",				CItemData::APPLY_ENCHANT_ICE);					// 101
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_WIND",				CItemData::APPLY_ENCHANT_WIND);					// 102
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_EARTH",			CItemData::APPLY_ENCHANT_EARTH);				// 103
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_DARK",				CItemData::APPLY_ENCHANT_DARK);					// 104
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CZ",				CItemData::APPLY_ATTBONUS_CZ);					// 105
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_INSECT",			CItemData::APPLY_ATTBONUS_INSECT);				// 106
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DESERT",			CItemData::APPLY_ATTBONUS_DESERT);				// 107
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_SWORD",			CItemData::APPLY_ATTBONUS_SWORD);				// 108
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_TWOHAND",			CItemData::APPLY_ATTBONUS_TWOHAND);				// 109
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_DAGGER",			CItemData::APPLY_ATTBONUS_DAGGER);				// 110
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BELL",			CItemData::APPLY_ATTBONUS_BELL);				// 111
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_FAN",				CItemData::APPLY_ATTBONUS_FAN);					// 112
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_BOW",				CItemData::APPLY_ATTBONUS_BOW);					// 113
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_CLAW",			CItemData::APPLY_ATTBONUS_CLAW);				// 114
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_HUMAN",				CItemData::APPLY_RESIST_HUMAN);					// 115
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_MOUNT_FALL",		CItemData::APPLY_RESIST_MOUNT_FALL);			// 116
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_FIST",				CItemData::APPLY_RESIST_FIST);					// 117
	PyModule_AddIntConstant(poModule, "APPLY_MOUNT",					CItemData::APPLY_MOUNT);						// 118
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_SAMYEON", CItemData::APPLY_SKILL_DAMAGE_SAMYEON);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_TANHWAN", CItemData::APPLY_SKILL_DAMAGE_TANHWAN);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_PALBANG", CItemData::APPLY_SKILL_DAMAGE_PALBANG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_GIGONGCHAM", CItemData::APPLY_SKILL_DAMAGE_GIGONGCHAM);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_GYOKSAN", CItemData::APPLY_SKILL_DAMAGE_GYOKSAN);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_GEOMPUNG", CItemData::APPLY_SKILL_DAMAGE_GEOMPUNG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_AMSEOP", CItemData::APPLY_SKILL_DAMAGE_AMSEOP);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_GUNGSIN", CItemData::APPLY_SKILL_DAMAGE_GUNGSIN);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_CHARYUN", CItemData::APPLY_SKILL_DAMAGE_CHARYUN);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_SANGONG", CItemData::APPLY_SKILL_DAMAGE_SANGONG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_YEONSA", CItemData::APPLY_SKILL_DAMAGE_YEONSA);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_KWANKYEOK", CItemData::APPLY_SKILL_DAMAGE_KWANKYEOK);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_GIGUNG", CItemData::APPLY_SKILL_DAMAGE_GIGUNG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_HWAJO", CItemData::APPLY_SKILL_DAMAGE_HWAJO);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_SWAERYUNG", CItemData::APPLY_SKILL_DAMAGE_SWAERYUNG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_YONGKWON", CItemData::APPLY_SKILL_DAMAGE_YONGKWON);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_PABEOB", CItemData::APPLY_SKILL_DAMAGE_PABEOB);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_MARYUNG", CItemData::APPLY_SKILL_DAMAGE_MARYUNG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_HWAYEOMPOK", CItemData::APPLY_SKILL_DAMAGE_HWAYEOMPOK);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_MAHWAN", CItemData::APPLY_SKILL_DAMAGE_MAHWAN);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BIPABU", CItemData::APPLY_SKILL_DAMAGE_BIPABU);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_YONGBI", CItemData::APPLY_SKILL_DAMAGE_YONGBI);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_PAERYONG", CItemData::APPLY_SKILL_DAMAGE_PAERYONG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_NOEJEON", CItemData::APPLY_SKILL_DAMAGE_NOEJEON);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BYEURAK", CItemData::APPLY_SKILL_DAMAGE_BYEURAK);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_CHAIN", CItemData::APPLY_SKILL_DAMAGE_CHAIN);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_CHAYEOL", CItemData::APPLY_SKILL_DAMAGE_CHAYEOL);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_SALPOONG", CItemData::APPLY_SKILL_DAMAGE_SALPOONG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_GONGDAB", CItemData::APPLY_SKILL_DAMAGE_GONGDAB);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_PASWAE", CItemData::APPLY_SKILL_DAMAGE_PASWAE);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE", CItemData::APPLY_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DEFEND_BONUS_BOSS_OR_MORE", CItemData::APPLY_SKILL_DEFEND_BONUS_BOSS_OR_MORE);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE", CItemData::APPLY_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DAMAGE_BONUS_BOSS_OR_MORE", CItemData::APPLY_SKILL_DAMAGE_BONUS_BOSS_OR_MORE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_ENCHANT_FIRE", CItemData::APPLY_HIT_BUFF_ENCHANT_FIRE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_ENCHANT_ICE", CItemData::APPLY_HIT_BUFF_ENCHANT_ICE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_ENCHANT_ELEC", CItemData::APPLY_HIT_BUFF_ENCHANT_ELEC);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_ENCHANT_WIND", CItemData::APPLY_HIT_BUFF_ENCHANT_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_ENCHANT_DARK", CItemData::APPLY_HIT_BUFF_ENCHANT_DARK);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_ENCHANT_EARTH", CItemData::APPLY_HIT_BUFF_ENCHANT_EARTH);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_RESIST_FIRE", CItemData::APPLY_HIT_BUFF_RESIST_FIRE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_RESIST_ICE", CItemData::APPLY_HIT_BUFF_RESIST_ICE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_RESIST_ELEC", CItemData::APPLY_HIT_BUFF_RESIST_ELEC);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_RESIST_WIND", CItemData::APPLY_HIT_BUFF_RESIST_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_RESIST_DARK", CItemData::APPLY_HIT_BUFF_RESIST_DARK);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_RESIST_EARTH", CItemData::APPLY_HIT_BUFF_RESIST_EARTH);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHEONGRANG_MOV_SPEED", CItemData::APPLY_USE_SKILL_CHEONGRANG_MOV_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHEONGRANG_CASTING_SPEED", CItemData::APPLY_USE_SKILL_CHEONGRANG_CASTING_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHAYEOL_CRITICAL_PCT", CItemData::APPLY_USE_SKILL_CHAYEOL_CRITICAL_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SANGONG_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_SANGONG_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GIGUNG_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_GIGUNG_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_JEOKRANG_DEF_BONUS", CItemData::APPLY_USE_SKILL_JEOKRANG_DEF_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GWIGEOM_DEF_BONUS", CItemData::APPLY_USE_SKILL_GWIGEOM_DEF_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_TERROR_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_TERROR_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_MUYEONG_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_MUYEONG_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_MANASHILED_CASTING_SPEED", CItemData::APPLY_USE_SKILL_MANASHILED_CASTING_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_HOSIN_DEF_BONUS", CItemData::APPLY_USE_SKILL_HOSIN_DEF_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GICHEON_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_GICHEON_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_JEONGEOP_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_JEUNGRYEOK_DEF_BONUS", CItemData::APPLY_USE_SKILL_JEUNGRYEOK_DEF_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_GIHYEOL_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHUNKEON_CASTING_SPEED", CItemData::APPLY_USE_SKILL_CHUNKEON_CASTING_SPEED);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS", CItemData::APPLY_USE_SKILL_NOEGEOM_ATT_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DURATION_INCREASE_EUNHYUNG", CItemData::APPLY_SKILL_DURATION_INCREASE_EUNHYUNG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DURATION_INCREASE_GYEONGGONG", CItemData::APPLY_SKILL_DURATION_INCREASE_GYEONGGONG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DURATION_INCREASE_GEOMKYUNG", CItemData::APPLY_SKILL_DURATION_INCREASE_GEOMKYUNG);
	PyModule_AddIntConstant(poModule, "APPLY_SKILL_DURATION_INCREASE_JEOKRANG", CItemData::APPLY_SKILL_DURATION_INCREASE_JEOKRANG);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_PALBANG_HP_ABSORB", CItemData::APPLY_USE_SKILL_PALBANG_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_AMSEOP_HP_ABSORB", CItemData::APPLY_USE_SKILL_AMSEOP_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_YEONSA_HP_ABSORB", CItemData::APPLY_USE_SKILL_YEONSA_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_YONGBI_HP_ABSORB", CItemData::APPLY_USE_SKILL_YONGBI_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHAIN_HP_ABSORB", CItemData::APPLY_USE_SKILL_CHAIN_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_PASWAE_SP_ABSORB", CItemData::APPLY_USE_SKILL_PASWAE_SP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GIGONGCHAM_STUN", CItemData::APPLY_USE_SKILL_GIGONGCHAM_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHARYUN_STUN", CItemData::APPLY_USE_SKILL_CHARYUN_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_PABEOB_STUN", CItemData::APPLY_USE_SKILL_PABEOB_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_MAHWAN_STUN", CItemData::APPLY_USE_SKILL_MAHWAN_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GONGDAB_STUN", CItemData::APPLY_USE_SKILL_GONGDAB_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SAMYEON_STUN", CItemData::APPLY_USE_SKILL_SAMYEON_STUN);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GYOKSAN_KNOCKBACK", CItemData::APPLY_USE_SKILL_GYOKSAN_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SEOMJEON_KNOCKBACK", CItemData::APPLY_USE_SKILL_SEOMJEON_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SWAERYUNG_KNOCKBACK", CItemData::APPLY_USE_SKILL_SWAERYUNG_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_HWAYEOMPOK_KNOCKBACK", CItemData::APPLY_USE_SKILL_HWAYEOMPOK_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GONGDAB_KNOCKBACK", CItemData::APPLY_USE_SKILL_GONGDAB_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_KWANKYEOK_KNOCKBACK", CItemData::APPLY_USE_SKILL_KWANKYEOK_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER", CItemData::APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_10PER);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_STONE", CItemData::APPLY_ATTBONUS_STONE);
	PyModule_AddIntConstant(poModule, "APPLY_DAMAGE_HP_RECOVERY", CItemData::APPLY_DAMAGE_HP_RECOVERY);
	PyModule_AddIntConstant(poModule, "APPLY_DAMAGE_SP_RECOVERY", CItemData::APPLY_DAMAGE_SP_RECOVERY);
	PyModule_AddIntConstant(poModule, "APPLY_ALIGNMENT_DAMAGE_BONUS", CItemData::APPLY_ALIGNMENT_DAMAGE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_NORMAL_DAMAGE_GUARD", CItemData::APPLY_NORMAL_DAMAGE_GUARD);
	PyModule_AddIntConstant(poModule, "APPLY_MORE_THEN_HP90_DAMAGE_REDUCE", CItemData::APPLY_MORE_THEN_HP90_DAMAGE_REDUCE);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_TUSOK_HP_ABSORB", CItemData::APPLY_USE_SKILL_TUSOK_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_PAERYONG_HP_ABSORB", CItemData::APPLY_USE_SKILL_PAERYONG_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_BYEURAK_HP_ABSORB", CItemData::APPLY_USE_SKILL_BYEURAK_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_FIRST_ATTRIBUTE_BONUS", CItemData::APPLY_FIRST_ATTRIBUTE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_SECOND_ATTRIBUTE_BONUS", CItemData::APPLY_SECOND_ATTRIBUTE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_THIRD_ATTRIBUTE_BONUS", CItemData::APPLY_THIRD_ATTRIBUTE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_FOURTH_ATTRIBUTE_BONUS", CItemData::APPLY_FOURTH_ATTRIBUTE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_FIFTH_ATTRIBUTE_BONUS", CItemData::APPLY_FIFTH_ATTRIBUTE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_SAMYEON_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_GEOMPUNG_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_GUNGSIN_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_KWANKYEOK_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_YONGKWON_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_MARYUNG_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_BIPABU_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_NOEJEON_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_SALPOONG_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER", CItemData::APPLY_USE_SKILL_PASWAE_NEXT_COOLTIME_DECREASE_20PER);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_CHAYEOL_HP_ABSORB", CItemData::APPLY_USE_SKILL_CHAYEOL_HP_ABSORB);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_STR", CItemData::APPLY_SUNGMA_STR);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_HP", CItemData::APPLY_SUNGMA_HP);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_MOVE", CItemData::APPLY_SUNGMA_MOVE);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_IMMUNE", CItemData::APPLY_SUNGMA_IMMUNE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_PCT", CItemData::APPLY_HIT_PCT);
	PyModule_AddIntConstant(poModule, "APPLY_RANDOM", CItemData::APPLY_RANDOM);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_HUMAN", CItemData::APPLY_ATTBONUS_PER_HUMAN);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_ANIMAL", CItemData::APPLY_ATTBONUS_PER_ANIMAL);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_ORC", CItemData::APPLY_ATTBONUS_PER_ORC);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_MILGYO", CItemData::APPLY_ATTBONUS_PER_MILGYO);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_UNDEAD", CItemData::APPLY_ATTBONUS_PER_UNDEAD);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_DEVIL", CItemData::APPLY_ATTBONUS_PER_DEVIL);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_PER_ELECT", CItemData::APPLY_ENCHANT_PER_ELECT);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_PER_FIRE", CItemData::APPLY_ENCHANT_PER_FIRE);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_PER_ICE", CItemData::APPLY_ENCHANT_PER_ICE);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_PER_WIND", CItemData::APPLY_ENCHANT_PER_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_PER_EARTH", CItemData::APPLY_ENCHANT_PER_EARTH);
	PyModule_AddIntConstant(poModule, "APPLY_ENCHANT_PER_DARK", CItemData::APPLY_ENCHANT_PER_DARK);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_CZ", CItemData::APPLY_ATTBONUS_PER_CZ);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_INSECT", CItemData::APPLY_ATTBONUS_PER_INSECT);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_DESERT", CItemData::APPLY_ATTBONUS_PER_DESERT);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_STONE", CItemData::APPLY_ATTBONUS_PER_STONE);
	PyModule_AddIntConstant(poModule, "APPLY_ATTBONUS_PER_MONSTER", CItemData::APPLY_ATTBONUS_PER_MONSTER);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_HUMAN", CItemData::APPLY_RESIST_PER_HUMAN);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_ICE", CItemData::APPLY_RESIST_PER_ICE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_DARK", CItemData::APPLY_RESIST_PER_DARK);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_EARTH", CItemData::APPLY_RESIST_PER_EARTH);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_FIRE", CItemData::APPLY_RESIST_PER_FIRE);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_ELEC", CItemData::APPLY_RESIST_PER_ELEC);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_MAGIC", CItemData::APPLY_RESIST_PER_MAGIC);
	PyModule_AddIntConstant(poModule, "APPLY_RESIST_PER_WIND", CItemData::APPLY_RESIST_PER_WIND);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_SUNGMA_STR", CItemData::APPLY_HIT_BUFF_SUNGMA_STR);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_SUNGMA_MOVE", CItemData::APPLY_HIT_BUFF_SUNGMA_MOVE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_SUNGMA_HP", CItemData::APPLY_HIT_BUFF_SUNGMA_HP);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_BUFF_SUNGMA_IMMUNE", CItemData::APPLY_HIT_BUFF_SUNGMA_IMMUNE);
	PyModule_AddIntConstant(poModule, "APPLY_MOUNT_MELEE_MAGIC_ATTBONUS_PER", CItemData::APPLY_MOUNT_MELEE_MAGIC_ATTBONUS_PER);
	PyModule_AddIntConstant(poModule, "APPLY_DISMOUNT_MOVE_SPEED_BONUS_PER", CItemData::APPLY_DISMOUNT_MOVE_SPEED_BONUS_PER);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_AUTO_HP_RECOVERY", CItemData::APPLY_HIT_AUTO_HP_RECOVERY);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_AUTO_SP_RECOVERY", CItemData::APPLY_HIT_AUTO_SP_RECOVERY);
	PyModule_AddIntConstant(poModule, "APPLY_USE_SKILL_COOLTIME_DECREASE_ALL", CItemData::APPLY_USE_SKILL_COOLTIME_DECREASE_ALL);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_STONE_ATTBONUS_STONE", CItemData::APPLY_HIT_STONE_ATTBONUS_STONE);
	PyModule_AddIntConstant(poModule, "APPLY_HIT_STONE_DEF_GRADE_BONUS", CItemData::APPLY_HIT_STONE_DEF_GRADE_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_KILL_BOSS_ITEM_BONUS", CItemData::APPLY_KILL_BOSS_ITEM_BONUS);
	PyModule_AddIntConstant(poModule, "APPLY_MOB_HIT_MOB_AGGRESSIVE", CItemData::APPLY_MOB_HIT_MOB_AGGRESSIVE);
	PyModule_AddIntConstant(poModule, "APPLY_NO_DEATH_AND_HP_RECOVERY30", CItemData::APPLY_NO_DEATH_AND_HP_RECOVERY30);
	PyModule_AddIntConstant(poModule, "APPLY_AUTO_PICKUP", CItemData::APPLY_AUTO_PICKUP);
	PyModule_AddIntConstant(poModule, "APPLY_MOUNT_NO_KNOCKBACK", CItemData::APPLY_MOUNT_NO_KNOCKBACK);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_PER_STR", CItemData::APPLY_SUNGMA_PER_STR);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_PER_HP", CItemData::APPLY_SUNGMA_PER_HP);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_PER_MOVE", CItemData::APPLY_SUNGMA_PER_MOVE);
	PyModule_AddIntConstant(poModule, "APPLY_SUNGMA_PER_IMMUNE", CItemData::APPLY_SUNGMA_PER_IMMUNE);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_POISON100", CItemData::APPLY_IMMUNE_POISON100);
	PyModule_AddIntConstant(poModule, "APPLY_IMMUNE_BLEEDING100", CItemData::APPLY_IMMUNE_BLEEDING100);
	PyModule_AddIntConstant(poModule, "APPLY_MONSTER_DEFEND_BONUS", CItemData::APPLY_MONSTER_DEFEND_BONUS);
#endif
	//END_BONUS_LIST

#ifdef ENABLE_GUILDRENEWAL_SYSTEM
	PyModule_AddIntConstant(poModule, "GUILD_SLOT_START_INDEX",			210);
#endif

#ifdef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_NONE",						CItemData::MASK_ITEM_TYPE_NONE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_MOUNT_PET",				CItemData::MASK_ITEM_TYPE_MOUNT_PET);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_EQUIPMENT_WEAPON",			CItemData::MASK_ITEM_TYPE_EQUIPMENT_WEAPON);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_EQUIPMENT_ARMOR",			CItemData::MASK_ITEM_TYPE_EQUIPMENT_ARMOR);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_EQUIPMENT_JEWELRY",		CItemData::MASK_ITEM_TYPE_EQUIPMENT_JEWELRY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_TUNING",					CItemData::MASK_ITEM_TYPE_TUNING);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_POTION",					CItemData::MASK_ITEM_TYPE_POTION);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_FISHING_PICK",				CItemData::MASK_ITEM_TYPE_FISHING_PICK);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_DRAGON_STONE",				CItemData::MASK_ITEM_TYPE_DRAGON_STONE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_COSTUMES",					CItemData::MASK_ITEM_TYPE_COSTUMES);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_SKILL",					CItemData::MASK_ITEM_TYPE_SKILL);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_UNIQUE",					CItemData::MASK_ITEM_TYPE_UNIQUE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_ETC",						CItemData::MASK_ITEM_TYPE_ETC);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_TYPE_MAX",						CItemData::MASK_ITEM_TYPE_MAX);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_MOUNT_PET_MOUNT",				CItemData::MASK_ITEM_SUBTYPE_MOUNT_PET_MOUNT);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_MOUNT_PET_CHARGED_PET",			CItemData::MASK_ITEM_SUBTYPE_MOUNT_PET_CHARGED_PET);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_MOUNT_PET_FREE_PET",			CItemData::MASK_ITEM_SUBTYPE_MOUNT_PET_FREE_PET);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_MOUNT_PET_EGG",					CItemData::MASK_ITEM_SUBTYPE_MOUNT_PET_EGG);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_SWORD",		CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_SWORD);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_DAGGER",		CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_DAGGER);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BOW",			CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BOW);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_TWO_HANDED",	CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_TWO_HANDED);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BELL",			CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_BELL);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_CLAW",			CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_CLAW);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_FAN",			CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_FAN);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_MOUNT_SPEAR",	CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_MOUNT_SPEAR);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_ARROW",		CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_ARROW);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_WEAPON_WEAPON_QUIVER",		CItemData::MASK_ITEM_SUBTYPE_WEAPON_WEAPON_QUIVER);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ARMOR_ARMOR_BODY",			CItemData::MASK_ITEM_SUBTYPE_ARMOR_ARMOR_BODY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ARMOR_ARMOR_HEAD",			CItemData::MASK_ITEM_SUBTYPE_ARMOR_ARMOR_HEAD);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ARMOR_ARMOR_SHIELD",		CItemData::MASK_ITEM_SUBTYPE_ARMOR_ARMOR_SHIELD);
#ifdef ENABLE_GLOVE_SYSTEM
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ARMOR_ARMOR_GLOVE",		CItemData::MASK_ITEM_SUBTYPE_ARMOR_ARMOR_GLOVE);
#endif

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_WRIST",	CItemData::MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_WRIST);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_FOOTS",	CItemData::MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_FOOTS);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_NECK",	CItemData::MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_NECK);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_EAR",	CItemData::MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_EAR);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_JEWELRY_ITEM_BELT",	CItemData::MASK_ITEM_SUBTYPE_JEWELRY_ITEM_BELT);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_PENDANT",CItemData::MASK_ITEM_SUBTYPE_JEWELRY_ARMOR_PENDANT);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_TUNING_RESOURCE",				CItemData::MASK_ITEM_SUBTYPE_TUNING_RESOURCE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_TUNING_STONE",					CItemData::MASK_ITEM_SUBTYPE_TUNING_STONE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_TUNING_ETC",					CItemData::MASK_ITEM_SUBTYPE_TUNING_ETC);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_POTION_ABILITY",				CItemData::MASK_ITEM_SUBTYPE_POTION_ABILITY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_POTION_HAIRDYE",				CItemData::MASK_ITEM_SUBTYPE_POTION_HAIRDYE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_POTION_ETC",					CItemData::MASK_ITEM_SUBTYPE_POTION_ETC);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_FISHING_PICK_FISHING_POLE",		CItemData::MASK_ITEM_SUBTYPE_FISHING_PICK_FISHING_POLE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_FISHING_PICK_EQUIPMENT_PICK",	CItemData::MASK_ITEM_SUBTYPE_FISHING_PICK_EQUIPMENT_PICK);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_FISHING_PICK_FOOD",				CItemData::MASK_ITEM_SUBTYPE_FISHING_PICK_FOOD);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_FISHING_PICK_STONE",			CItemData::MASK_ITEM_SUBTYPE_FISHING_PICK_STONE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_FISHING_PICK_ETC",				CItemData::MASK_ITEM_SUBTYPE_FISHING_PICK_ETC);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_DIAMOND",	CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_DIAMOND);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_RUBY",		CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_RUBY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_JADE",		CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_JADE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_SAPPHIRE",	CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_SAPPHIRE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_GARNET",	CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_GARNET);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_ONYX",		CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_DRAGON_ONYX);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_DRAGON_STONE_ETC",				CItemData::MASK_ITEM_SUBTYPE_DRAGON_STONE_ETC);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_WEAPON",		CItemData::MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_WEAPON);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_BODY",			CItemData::MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_BODY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_HAIR",			CItemData::MASK_ITEM_SUBTYPE_COSTUMES_COSTUME_HAIR);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_COSTUMES_SASH",					CItemData::MASK_ITEM_SUBTYPE_COSTUMES_SASH);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_COSTUMES_ETC",					CItemData::MASK_ITEM_SUBTYPE_COSTUMES_ETC);
#ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_COSTUMES_AURA",				CItemData::MASK_ITEM_SUBTYPE_COSTUMES_AURA);
#endif

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_SKILL_PAHAE",					CItemData::MASK_ITEM_SUBTYPE_SKILL_PAHAE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK",				CItemData::MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_SKILL_BOOK_OF_OBLIVION",		CItemData::MASK_ITEM_SUBTYPE_SKILL_BOOK_OF_OBLIVION);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_SKILL_ETC",					CItemData::MASK_ITEM_SUBTYPE_SKILL_ETC);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK_HORSE",		CItemData::MASK_ITEM_SUBTYPE_SKILL_SKILL_BOOK_HORSE);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_UNIQUE_ABILITY",				CItemData::MASK_ITEM_SUBTYPE_UNIQUE_ABILITY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_UNIQUE_ETC",					CItemData::MASK_ITEM_SUBTYPE_UNIQUE_ETC);

	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_GIFTBOX",					CItemData::MASK_ITEM_SUBTYPE_ETC_GIFTBOX);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_MATRIMORY",					CItemData::MASK_ITEM_SUBTYPE_ETC_MATRIMORY);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_EVENT",						CItemData::MASK_ITEM_SUBTYPE_ETC_EVENT);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_SEAL",						CItemData::MASK_ITEM_SUBTYPE_ETC_SEAL);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_PARTI",						CItemData::MASK_ITEM_SUBTYPE_ETC_PARTI);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_POLYMORPH",					CItemData::MASK_ITEM_SUBTYPE_ETC_POLYMORPH);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_RECIPE",					CItemData::MASK_ITEM_SUBTYPE_ETC_RECIPE);
	PyModule_AddIntConstant(poModule, "MASK_ITEM_SUBTYPE_ETC_ETC",						CItemData::MASK_ITEM_SUBTYPE_ETC_ETC);
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
	PyModule_AddIntConstant(poModule, "MEDIUM_MOVE_COSTUME_ATTR",				CItemData::MEDIUM_MOVE_COSTUME_ATTR);
#endif
#ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "RESOURCE_AURA",				CItemData::RESOURCE_AURA);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_NONE",			CItemData::AURA_GRADE_NONE);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_ORDINARY",		CItemData::AURA_GRADE_ORDINARY);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_SIMPLE",			CItemData::AURA_GRADE_SIMPLE);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_NOBLE",			CItemData::AURA_GRADE_NOBLE);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_SPARKLING",		CItemData::AURA_GRADE_SPARKLING);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_MAGNIFICENT",		CItemData::AURA_GRADE_MAGNIFICENT);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_RADIANT",			CItemData::AURA_GRADE_RADIANT);
	PyModule_AddIntConstant(poModule, "AURA_GRADE_MAX_NUM",			CItemData::AURA_GRADE_MAX_NUM);
	PyModule_AddIntConstant(poModule, "AURA_BOOST_ITEM_VNUM_BASE",	CItemData::AURA_BOOST_ITEM_VNUM_BASE);
	PyModule_AddIntConstant(poModule, "ITEM_AURA_BOOST_ERASER",		CItemData::ITEM_AURA_BOOST_ERASER);
	PyModule_AddIntConstant(poModule, "ITEM_AURA_BOOST_WEAK",		CItemData::ITEM_AURA_BOOST_WEAK);
	PyModule_AddIntConstant(poModule, "ITEM_AURA_BOOST_NORMAL",		CItemData::ITEM_AURA_BOOST_NORMAL);
	PyModule_AddIntConstant(poModule, "ITEM_AURA_BOOST_STRONG",		CItemData::ITEM_AURA_BOOST_STRONG);
	PyModule_AddIntConstant(poModule, "ITEM_AURA_BOOST_ULTIMATE",	CItemData::ITEM_AURA_BOOST_ULTIMATE);
	PyModule_AddIntConstant(poModule, "ITEM_AURA_BOOST_MAX",		CItemData::ITEM_AURA_BOOST_MAX);
#endif
#ifdef ENABLE_PICK_ROD_REFINE_RENEWAL
	PyModule_AddIntConstant(poModule, "ITEM_PICK_MAX_LEVEL",					CItemData::ITEM_PICK_MAX_LEVEL);
	PyModule_AddIntConstant(poModule, "ITEM_ROD_MAX_LEVEL",						CItemData::ITEM_ROD_MAX_LEVEL);
#endif
#ifdef ENABLE_PASSIVE_SYSTEM
	PyModule_AddIntConstant(poModule, "PASSIVE_JOB",				CItemData::PASSIVE_JOB);
#endif
}
