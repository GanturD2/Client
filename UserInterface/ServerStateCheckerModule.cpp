#include "StdAfx.h"
#include "ServerStateChecker.h"

PyObject * ServerStateCheckerCreate(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poWnd;
	if (!PyTuple_GetObject(poArgs, 0, &poWnd))
		return Py_BadArgument();

	CServerStateChecker::Instance().Create(poWnd);
	return Py_BuildNone();
}

PyObject * ServerStateCheckerUpdate(PyObject * poSelf, PyObject * poArgs)
{
	CServerStateChecker::Instance().Update();
	return Py_BuildNone();
}

PyObject * ServerStateCheckerRequest(PyObject * poSelf, PyObject * poArgs)
{
	CServerStateChecker::Instance().Request();
	return Py_BuildNone();
}

PyObject * ServerStateCheckerAddChannel(PyObject * poSelf, PyObject * poArgs)
{
	int nServerIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &nServerIndex))
		return Py_BuildException();

	char * szAddr;
	if (!PyTuple_GetString(poArgs, 1, &szAddr))
		return Py_BuildException();

	int nPort;
	if (!PyTuple_GetInteger(poArgs, 2, &nPort))
		return Py_BuildException();

	CServerStateChecker::Instance().AddChannel(nServerIndex, szAddr, nPort);
	return Py_BuildNone();
}

PyObject * ServerStateCheckerInitialize(PyObject * poSelf, PyObject * poArgs)
{
	CServerStateChecker::Instance().Initialize();
	return Py_BuildNone();
}

#ifdef ENABLE_OFFICAL_FEATURES
PyObject* ServerStateCheckerSetAuth(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}
#endif

void initServerStateChecker()
{
	static PyMethodDef s_methods[] = {
		{"Create", ServerStateCheckerCreate, METH_VARARGS},
		{"Update", ServerStateCheckerUpdate, METH_VARARGS},
		{"Request", ServerStateCheckerRequest, METH_VARARGS},
		{"AddChannel", ServerStateCheckerAddChannel, METH_VARARGS},
		{"Initialize", ServerStateCheckerInitialize, METH_VARARGS},
#ifdef ENABLE_OFFICAL_FEATURES
		{"SetAuth",		ServerStateCheckerSetAuth,		METH_VARARGS},
#endif
	{nullptr, nullptr, 0}};

	Py_InitModule("ServerStateChecker", s_methods);
}