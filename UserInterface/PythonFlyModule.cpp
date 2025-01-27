#include "StdAfx.h"
#include "../GameLib/FlyingObjectManager.h"

PyObject * flyUpdate(PyObject * poSelf, PyObject * poArgs)
{
	CFlyingManager::Instance().Update();
	return Py_BuildNone();
}

PyObject * flyRender(PyObject * poSelf, PyObject * poArgs)
{
	CFlyingManager::Instance().Render();
	return Py_BuildNone();
}

void initfly()
{
	static PyMethodDef s_methods[] = {{"Update", flyUpdate, METH_VARARGS},
									  {"Render", flyRender, METH_VARARGS},

									  {nullptr, nullptr, 0}};

	Py_InitModule("fly", s_methods);
}