#pragma once

#include "../eterBase/Singleton.h"

class CPythonLauncher : public CSingleton<CPythonLauncher>
{
public:
	CPythonLauncher();
	virtual ~CPythonLauncher();

	void Clear();

	bool Create(const char * c_szProgramName = "eter.python");
	void SetTraceFunc(int (*pFunc)(PyObject * obj, PyFrameObject * f, int what, PyObject * arg));
	bool RunLine(const char * c_szSrc);
	bool RunFile(const char * c_szFileName);
	bool RunMemoryTextFile(const char * c_szFileName, uint32_t uFileSize, const VOID * c_pvFileData);
	bool RunCompiledFile(const char * c_szFileName);
	const char * GetError();

protected:
	PyObject * m_poModule;
	PyObject * m_poDic;
};
