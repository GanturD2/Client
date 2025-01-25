#include "StdAfx.h"
#include "../EterPack/EterPackManager.h"
#include "../EterBase/tea.h"

// CHINA_CRYPT_KEY
uint32_t g_adwEncryptKey[4];
uint32_t g_adwDecryptKey[4];

#include "AccountConnector.h"

inline const uint8_t * GetKey_20050304Myevan()
{
	volatile static uint32_t s_adwKey[1938];

	volatile uint32_t seed = 1491971513;
	for (uint32_t i = 0; i < uint8_t(seed); i++)
	{
		seed ^= 2148941891;
		seed += 3592385981;
		s_adwKey[i] = seed;
	}

	return (const uint8_t *) s_adwKey;
}

void CAccountConnector::__BuildClientKey_20050304Myevan() const
{
	const uint8_t * c_pszKey = GetKey_20050304Myevan();
	memcpy(g_adwEncryptKey, c_pszKey + 157, 16);

	for (auto & i : g_adwEncryptKey)
		i = random();

	tea_encrypt((uint32_t *) g_adwDecryptKey, (const uint32_t *) g_adwEncryptKey, reinterpret_cast<const uint32_t *>(c_pszKey + 37), 16);
}
// END_OF_CHINA_CRYPT_KEY

PyObject * packExist(PyObject * poSelf, PyObject * poArgs)
{
	char * strFileName;

	if (!PyTuple_GetString(poArgs, 0, &strFileName))
		return Py_BuildException();

	return Py_BuildValue("i", CEterPackManager::Instance().isExist(strFileName) ? 1 : 0);
}

#ifdef ENABLE_PACK_IMPORT_MODULE
#	include <msl/utils.h>

static PyObject* packImportModule(PyObject* poSelf, PyObject* poArgs)
{
	char* str;
	char* filename;
	int mode = 0; // 0:"exec"
	int dont_inherit = 0;
	int supplied_flags = 0;
	PyCompilerFlags cf;
	PyObject* result = NULL, * tmp = NULL;
	Py_ssize_t length;
	int start[] = { Py_file_input, Py_eval_input, Py_single_input };

	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	cf.cf_flags = supplied_flags;
	PyEval_MergeCompilerFlags(&cf);

	CMappedFile file;
	const void* pData = nullptr;
	if (CEterPackManager::Instance().Get(file, filename, &pData))
	{
		/* Copy to NUL-terminated buffer. */
		tmp = PyString_FromStringAndSize((const char*)pData, file.Size());
		if (tmp == NULL)
			return NULL;
		str = PyString_AS_STRING(tmp);
		length = PyString_GET_SIZE(tmp);
	}
	else
	{
		std::string msg = "pack.importModule() filename not found: ";
		msg += filename;
		PyErr_SetString(PyExc_TypeError, msg.c_str());
		goto cleanup;
	}

	if ((size_t)length != strlen(str))
	{
		PyErr_SetString(PyExc_TypeError, "pack.importModule() expected string without null bytes");
		goto cleanup;
	}

	result = Py_CompileStringFlags(str, filename, start[mode], &cf);
cleanup:
	Py_XDECREF(tmp);
	return result;
}

#	include "../GameLib/RaceManager.h"
static PyObject* packLoadNpcList(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadNpcList(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		uint32_t vnum = atoi(kTokenVector[0].c_str());
		if (vnum)
			CRaceManager::Instance().RegisterRaceName(vnum, kTokenVector[1].c_str());
		else if (kTokenVector.size() >= 3)
			CRaceManager::Instance().RegisterRaceSrcName(kTokenVector[1].c_str(), kTokenVector[2].c_str());
		else
			TraceError("packLoadNpcList(c_szFileName=%s) - Line Error %s %s", filename, kTokenVector[0].c_str(), kTokenVector[1].c_str());
	}
	return Py_BuildNone();
}

static PyObject* packLoadLocaleInterface(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	PyObject* localeDict;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadLocaleInterface(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2)
			continue;

		PyDict_SetItemString(localeDict, kTokenVector[0].c_str(), PyString_FromString(kTokenVector[1].c_str()));
	}
	return Py_BuildNone();
}

static PyObject* packLoadLocaleGame(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	PyObject* localeDict;
	PyObject* funcDict;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &localeDict))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 2, &funcDict))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadLocaleGame(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 2) // skip empty or incomplete
			continue;

		PyObject* itemValue = nullptr;

		// if 3 tabs, key0=func[type2](value1)
		if (kTokenVector.size() > 2 && !kTokenVector[2].empty())
		{
			PyObject* funcType = PyDict_GetItemString(funcDict, kTokenVector[2].c_str());
			if (funcType)
				itemValue = PyObject_CallFunction(funcType, "s#", kTokenVector[1].c_str(), kTokenVector[1].size());
			else
			{
				TraceError("packLoadLocaleGame(c_szFileName=%s, funcDict=%s) - Tag Error %d", filename, kTokenVector[2].c_str(),
					kTokenVector.size());
				return Py_BuildException();
			}
		}
		else
			itemValue = PyString_FromString(kTokenVector[1].c_str());

		// if 2 tabs, key0=value1
		PyDict_SetItemString(localeDict, kTokenVector[0].c_str(), itemValue);
	}
	return Py_BuildNone();
}

#else
PyObject * packGet(PyObject * poSelf, PyObject * poArgs)
{
	char * strFileName;

	if (!PyTuple_GetString(poArgs, 0, &strFileName))
		return Py_BuildException();

	// 파이썬에서 읽어드리는 패킹 파일은 python 파일과 txt 파일에 한정한다
	const char * pcExt = strrchr(strFileName, '.');
	if (pcExt) // 확장자가 있고
	{
#ifdef ENABLE_PACK_GET_CHECK
		if ((stricmp(pcExt, ".py") == 0) || (stricmp(pcExt, ".pyc") == 0) || (stricmp(pcExt, ".txt") == 0)
#	ifdef ENABLE_ACHIEVEMENT_SYSTEM
			|| (stricmp(pcExt, ".xml") == 0)
#	endif
			)
#else
		if (1)
#endif
		{
			CMappedFile file;
			const void * pData = nullptr;

			if (CEterPackManager::Instance().Get(file, strFileName, &pData))
				return Py_BuildValue("s#", pData, file.Size());
		}
	}

	return Py_BuildException();
}
#endif

#ifdef ENABLE_MYSHOP_DECO
/*static PyObject* packLoadShopDeco(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadShopDeco(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");

	PyObject* dict = PyTuple_New(lines.size());
	int i = 0;
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() == 0 || kTokenVector[0].empty())
			continue;

		PyObject* deco = PyDict_New();
		const uint8_t type = atoi(kTokenVector[0].c_str());
		if (type == 1)
			PyDict_SetItemString(deco, "model", Py_BuildValue("sss", kTokenVector[0].c_str(), kTokenVector[1].c_str(), kTokenVector[2].c_str()));
		else if (type == 2)
			PyDict_SetItemString(deco, "model", Py_BuildValue("ssss", kTokenVector[0].c_str(), kTokenVector[1].c_str(), kTokenVector[2].c_str(), kTokenVector[3].c_str()));

		PyTuple_SetItem(dict, i++, deco);
	}

	return dict;
}*/
#endif

#ifdef ENABLE_RACE_HEIGHT
/*#include "../GameLib/RaceManager.h"
static PyObject* packLoadRaceHeight(PyObject* poSelf, PyObject* poArgs)
{
	char* filename;
	if (!PyTuple_GetString(poArgs, 0, &filename))
		return Py_BuildException();

	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, filename, &pvData))
	{
		TraceError("packLoadRaceHeight(c_szFileName=%s) - Load Error", filename);
		return Py_BuildException();
	}

	std::string fileData((const char*)pvData, kFile.Size());
	const auto& lines = msl::string_split_any(fileData, "\r\n");
	for (const auto& line : lines)
	{
		auto& kTokenVector = msl::string_split(line, "\t");
		for (auto& token : kTokenVector)
			msl::trim_in_place(token);

		if (kTokenVector.size() < 3)
			continue;

		const uint32_t vnum = atoi(kTokenVector[0].c_str());
		const float height = static_cast<float>(atof(kTokenVector[1].c_str()));
		if (vnum && height)
			CRaceManager::Instance().SetRaceHeight(vnum, height);
		else
			TraceError("packLoadRaceHeight(c_szFileName=%s) - Line Error %s %s", filename, kTokenVector[0].c_str(), kTokenVector[1].c_str());
	}
	return Py_BuildNone();
}*/
#endif

void initpack()
{
	static PyMethodDef s_methods[] = {
		{"Exist", packExist, METH_VARARGS},
#ifdef ENABLE_PACK_IMPORT_MODULE
		{"importModule", packImportModule, METH_VARARGS},
		{"loadLocaleGame", packLoadLocaleGame, METH_VARARGS},
		{"loadLocaleInterface", packLoadLocaleInterface, METH_VARARGS},
		{"loadNpcList", packLoadNpcList, METH_VARARGS},
#else
		{"Get", packGet, METH_VARARGS},
#endif
#ifdef ENABLE_MYSHOP_DECO
	//	{"loadShopDeco", packLoadShopDeco, METH_VARARGS},
#endif
#ifdef ENABLE_RACE_HEIGHT
	//	{"loadRaceHeight", packLoadRaceHeight, METH_VARARGS},
#endif
		{nullptr, nullptr}};

	Py_InitModule("pack", s_methods);
}
