#include "StdAfx.h"
#include "PythonApplication.h"

#ifdef ENABLE_RENDER_TARGET
PyObject* renderTargetSelectModel(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	int modelIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->SelectModel(modelIndex);

	return Py_BuildNone();
}

PyObject* renderTargetSelectModelNPC(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	int modelIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->SelectModelNPC(modelIndex);

	return Py_BuildNone();
}

PyObject* renderTargetSelectModelPC(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	int race = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &race))
		return Py_BadArgument();

	int vnum = 0;
	if (!PyTuple_GetInteger(poArgs, 2, &vnum))
		return Py_BadArgument();

	int weapon = 0;
	if (!PyTuple_GetInteger(poArgs, 3, &weapon))
		return Py_BadArgument();

	int armor = 0;
	if (!PyTuple_GetInteger(poArgs, 4, &armor))
		return Py_BadArgument();

	int hair = 0;
	if (!PyTuple_GetInteger(poArgs, 5, &hair))
		return Py_BadArgument();

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	int acce = 0;
	if (!PyTuple_GetInteger(poArgs, 6, &acce))
		return Py_BadArgument();
#endif

#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined (ENABLE_AURA_SYSTEM)
	int aura = 0;
	if (!PyTuple_GetInteger(poArgs, 7, &aura))
		return Py_BadArgument();
#endif

#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined (ENABLE_AURA_SYSTEM)
	CRenderTargetManager::Instance().GetRenderTarget(index)->SelectModelPC(race, vnum, weapon, armor, hair, acce, aura);
#else
	CRenderTargetManager::Instance().GetRenderTarget(index)->SelectModelPC(race, vnum, weapon, armor, hair);
#endif

	return Py_BuildNone();
}

PyObject* renderTargetSetVisibility(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	bool isShow = false;
	if (!PyTuple_GetBoolean(poArgs, 1, &isShow))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->SetVisibility(isShow);

	return Py_BuildNone();
}

PyObject* renderTargetSetBackground(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	char * szPathName;
	if (!PyTuple_GetString(poArgs, 1, &szPathName))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->CreateBackground(
		szPathName, CPythonApplication::Instance().GetWidth(),
		CPythonApplication::Instance().GetHeight());
	return Py_BuildNone();
}

PyObject* renderTargetSetZoom(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t byRenderIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &byRenderIndex))
		return Py_BadArgument();

	bool bZoom = true;
	if (!PyTuple_GetBoolean(poArgs, 1, &bZoom))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(byRenderIndex)->SetZoom(bZoom);
	return Py_BuildNone();
}

PyObject* renderTargetChangeArmor(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	int modelIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->ChangeArmor(modelIndex);

	return Py_BuildNone();
}

PyObject* renderTargetChangeWeapon(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	int modelIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->ChangeWeapon(modelIndex);

	return Py_BuildNone();
}

PyObject* renderTargetChangeHair(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index = 0;
	if (PyTuple_GetByte(poArgs, 0, &index))
		return Py_BadArgument();

	int modelIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex))
		return Py_BadArgument();

	CRenderTargetManager::Instance().GetRenderTarget(index)->ChangeHair(modelIndex);

	return Py_BuildNone();
}

void initRenderTarget() {
	static PyMethodDef s_methods[] =
	{
		{ "SelectModel", renderTargetSelectModel, METH_VARARGS },
		{ "SelectModelNPC", renderTargetSelectModelNPC, METH_VARARGS },
		{ "SelectModelPC", renderTargetSelectModelPC, METH_VARARGS },

		{ "SetVisibility", renderTargetSetVisibility, METH_VARARGS },
		{ "SetBackground", renderTargetSetBackground, METH_VARARGS },
		{ "SetZoom", renderTargetSetZoom, METH_VARARGS },

		{ "ChangeArmor", renderTargetChangeArmor, METH_VARARGS },
		{ "ChangeWeapon", renderTargetChangeWeapon, METH_VARARGS },
		{ "ChangeHair", renderTargetChangeHair, METH_VARARGS },

		{nullptr, nullptr, 0 },
	};

	PyObject* poModule = Py_InitModule("renderTarget", s_methods);
	PyModule_AddIntConstant(poModule, "MODEL_NONE",				CRenderTargetManager::MODEL_NONE);
	PyModule_AddIntConstant(poModule, "MODEL_RENDER",			CRenderTargetManager::MODEL_RENDER);
	PyModule_AddIntConstant(poModule, "MODEL_PRIVATE_SHOP",		CRenderTargetManager::MODEL_PRIVATE_SHOP);
}
#endif
