#include "StdAfx.h"

PyObject * profilerPush(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildNone();
}

PyObject * profilerPop(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildNone();
}

void initProfiler()
{
	static PyMethodDef s_methods[] = {{"Push", profilerPush, METH_VARARGS},
									  {"Pop", profilerPop, METH_VARARGS},

									  {nullptr, nullptr, 0}};

	Py_InitModule("profiler", s_methods);
}