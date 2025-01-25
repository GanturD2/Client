#include "StdAfx.h"

#ifdef ENABLE_BIOLOG_SYSTEM
#include "PythonBiologManager.h"
#include "PythonNetworkStream.h"

PyObject* biologManagerGetCurrentLevel(PyObject* poSelf, PyObject* poArgs)
{
	const auto dwLevel = CPythonBiologManager::Instance().Biolog_GetCurrentLevel();
	return Py_BuildValue("i", dwLevel);
}

PyObject* biologManagerGetCurrentItem(PyObject* poSelf, PyObject* poArgs)
{
	const auto iItem = CPythonBiologManager::Instance().Biolog_GetCurrentItem();
	return Py_BuildValue("i", iItem);
}

PyObject* biologManagerGetCurrentCount(PyObject* poSelf, PyObject* poArgs)
{
	const auto iGiven = CPythonBiologManager::Instance().Biolog_GetCurrentGivenItems();
	const auto iCount = CPythonBiologManager::Instance().Biolog_GetCurrentRequiredCount();
	return Py_BuildValue("ii", iGiven, iCount);
}

PyObject* biologManagerGetCurrentCooldown(PyObject* poSelf, PyObject* poArgs)
{
	const auto iGCooldown = CPythonBiologManager::Instance().Biolog_GetCurrentGlobalCooldown();
	const auto iCooldown = CPythonBiologManager::Instance().Biolog_GetCurrentCooldown();
	return Py_BuildValue("ii", iGCooldown, iCooldown);
}

PyObject* biologManagerGetCooldownReminder(PyObject* poSelf, PyObject* poArgs)
{
	const auto bReminder = CPythonBiologManager::Instance().Biolog_GetCurrentCooldownReminder();
	return Py_BuildValue("b", bReminder);
}

PyObject* biologManagerGetCurrentRewardBonus(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	const auto iType = CPythonBiologManager::Instance().Biolog_GetCurrentRewardType(index);
	const auto iValue = CPythonBiologManager::Instance().Biolog_GetCurrentRewardValue(index);

	return Py_BuildValue("ii", iType, iValue);
}

PyObject* biologManagerGetCurrentRewardItem(PyObject* poSelf, PyObject* poArgs)
{
	const auto iItem = CPythonBiologManager::Instance().Biolog_GetCurrentRewardItem();
	const auto iItemCount = CPythonBiologManager::Instance().Biolog_GetCurrentRewardItemCount();

	return Py_BuildValue("ii", iItem, iItemCount);
}

PyObject* biologSendPacket(PyObject* poSelf, PyObject* poArgs)
{
	int bSubHeader = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &bSubHeader))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendBiologManagerAction(bSubHeader);

	return Py_BuildNone();
}

PyObject* biologSendPacketItem(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendBiologManagerAction(BIOLOG_MANAGER_SUBHEADER_CG_SEND);

	return Py_BuildNone();
}

PyObject* biologSendPacketReminder(PyObject* poSelf, PyObject* poArgs)
{
	bool bReminder;
	if (!PyTuple_GetBoolean(poArgs, 0, &bReminder))
		bReminder = false;

	CPythonNetworkStream::Instance().SendBiologManagerAction(BIOLOG_MANAGER_SUBHEADER_CG_TIMER);
	CPythonNetworkStream::Instance().Send(sizeof(bReminder), &bReminder);

	return Py_BuildNone();
}

PyObject* biologManagerHasSubMission(PyObject* poSelf, PyObject* poArgs)
{
	const bool bHasSubMission = CPythonBiologManager::Instance().Biolog_HasSubMission();
	return Py_BuildValue("b", bHasSubMission);
}

PyObject* biologManagerGetNpcVnum(PyObject* poSelf, PyObject* poArgs)
{
	const uint32_t dwGetNpcVnum = CPythonBiologManager::Instance().Biolog_GetNpcVnum();
	return Py_BuildValue("i", dwGetNpcVnum);
}

PyObject* biologManagerGetCurrentSubItem(PyObject* poSelf, PyObject* poArgs)
{
	const uint32_t dwSubItem = CPythonBiologManager::Instance().Biolog_GetCurrentSubItem();
	return Py_BuildValue("i", dwSubItem);
}

void initBiologManager() noexcept
{
	static PyMethodDef s_methods[] = { {"SendPacket", biologSendPacket, METH_VARARGS},
									  {"SendPacketItem", biologSendPacketItem, METH_VARARGS},
									  {"SendPacketReminder", biologSendPacketReminder, METH_VARARGS},
									  {"GetInfoLevel", biologManagerGetCurrentLevel, METH_VARARGS},
									  {"GetInfoItem", biologManagerGetCurrentItem, METH_VARARGS},
									  {"GetInfoCount", biologManagerGetCurrentCount, METH_VARARGS},
									  {"GetInfoCooldown", biologManagerGetCurrentCooldown, METH_VARARGS},
									  {"GetInfoReminder", biologManagerGetCooldownReminder, METH_VARARGS},
									  {"GetInfoRewardBonus", biologManagerGetCurrentRewardBonus, METH_VARARGS},
									  {"GetInfoRewardItem", biologManagerGetCurrentRewardItem, METH_VARARGS},
									  {"HasSubMission", biologManagerHasSubMission, METH_VARARGS},
									  {"GetNpcVnum", biologManagerGetNpcVnum, METH_VARARGS},
									  {"GetInfoSubItem", biologManagerGetCurrentSubItem, METH_VARARGS},

									  {nullptr, nullptr, 0} };

	PyObject* poModule = Py_InitModule("biologmgr", s_methods);

	PyModule_AddIntConstant(poModule, "BIOLOG_BONUS_LENGTH", MAX_BONUSES_LENGTH);
}
#endif