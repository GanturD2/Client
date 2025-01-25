#include "StdAfx.h"
#include "PythonCubeRenewal.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

#ifdef ENABLE_CUBE_RENEWAL
#include "../GameLib/ItemManager.h"
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <vector>

#define contain(x) if (line.find(x) != std::string::npos)
#define econtain(x) else if (line.find(x) != std::string::npos)

void split(const std::string& s, char delim, std::vector<std::string>* elems) {
	(*elems).clear();
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		(*elems).push_back(item);
	}
}

bool CPythonCubeRenewal::FN_check_cube_data(CUBE_DATA* cube_data)
{
	size_t i = 0;

	size_t dwEndIndex = 0;

	dwEndIndex = cube_data->npc_vnum.size();

	for (i = 0; i < dwEndIndex; ++i)
	{
		if (cube_data->npc_vnum[i] == 0)
			return false;
	}

	dwEndIndex = cube_data->item.size();

	for (i = 0; i < dwEndIndex; ++i)
	{
		if (cube_data->item[i].vnum == 0)
			return false;

		if (cube_data->item[i].count == 0)
			return false;
	}

	dwEndIndex = cube_data->reward.size();

	for (i = 0; i < dwEndIndex; ++i)
	{
		if (cube_data->reward[i].vnum == 0)
			return false;

		if (cube_data->reward[i].count == 0)
			return false;
	}

	return true;
}

#include "../EterPack/EterPackManager.h"
bool CPythonCubeRenewal::LoadFile(const char* szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
	if (!CEterPackManager::Instance().Get(kFile, szFileName, &pvData)) {
		TraceError("file not found %s", szFileName);
		return false;
	}

	// clear data file;
	s_cube_proto = {};

	const auto* c_pcBuf = static_cast<const char*>(pvData);

	std::stringstream buffer;

	buffer << c_pcBuf;

	int pos = 0;
	std::string abc = "";
	while (pos < kFile.Size())
	{
		const char c = c_pcBuf[pos++];
		abc += c;
	}
	std::string line;

	CUBE_DATA* cube_data = nullptr;
	CUBE_VALUE cube_value = { 0, 0 };
	
	int iLastVnum = 0;
	
	std::vector<std::string> vecOut = {};
	while (std::getline(buffer, line, '\n')) {
		contain("section")
		{
			cube_data = new CUBE_DATA;
			cube_data->gold = 0;
		}
		econtain("npc")
		{
			split(line, '\t', &vecOut);
			cube_data->npc_vnum.push_back(std::stoi(vecOut[1]));
		}
		econtain("item")
		{
			split(line, '\t', &vecOut);
			cube_value.vnum = std::stoi(vecOut[1]);
			cube_value.count = std::stoi(vecOut[2]);

			cube_data->item.push_back(cube_value);
		}
		econtain("reward")
		{
			split(line, '\t', &vecOut);
			cube_value.vnum = std::stoi(vecOut[1]);
			cube_value.count = std::stoi(vecOut[2]);

			iLastVnum = cube_value.vnum;

			cube_data->reward.push_back(cube_value);
		}
		econtain("percent") {
			split(line, '\t', &vecOut);
			cube_data->percent = std::stoi(vecOut[1]);
		}
		econtain("gold") {
			split(line, '\t', &vecOut);
			cube_data->gold = std::stoi(vecOut[1]);
		}
		econtain("category") {
			split(line, '\t', &vecOut);

			if (vecOut.size() != 0)
			{
				cube_data->category = vecOut[1];
			}
		}
		econtain("not_remove") {
			split(line, '\t', &vecOut);
			cube_data->not_remove = std::stoi(vecOut[1]);
		}
		econtain("set_value") {
			split(line, '\t', &vecOut);
			cube_data->set_value = std::stoi(vecOut[1]);
		}
		econtain("gem") {
			split(line, '\t', &vecOut);
			cube_data->gem = std::stoi(vecOut[1]);
		}
		econtain("allow_copy") {
			split(line, '\t', &vecOut);
			cube_data->allow_copy = std::stoi(vecOut[1]) == 1;
		}
		econtain("end")
		{
			if (!FN_check_cube_data(cube_data))
			{
				delete cube_data;
				continue;
			}
			
			//TraceError("reward_item: %d, at category %s", iLastVnum, cube_data->category.c_str());
			s_cube_proto.push_back(cube_data);
		}
	}

	return true;
}

CPythonCubeRenewal::CPythonCubeRenewal() : m_CubeRenewalHandler(nullptr)
{
	s_cube_proto = {};
}

CPythonCubeRenewal::~CPythonCubeRenewal()
{
	s_cube_proto = {};
}
static const char* szItemName(uint32_t dwVnum)
{
	if (!CItemManager::Instance().SelectItemData(dwVnum))
	{
		TraceError("Cannot find item by szItemName(%lu)", dwVnum);
		return "";
	}

	return CItemManager::Instance().GetSelectedItemDataPointer()->GetName();
}

static bool IsStackableItem(uint32_t dwVnum)
{
	if (!CItemManager::Instance().SelectItemData(dwVnum))
	{
		TraceError("Cannot find item by IsStackableItem(%lu)", dwVnum);
		return false;
	}

	auto ptr = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!ptr)
		return false;
	auto info = ptr->GetTable();
	if (!info)
		return false;
	return (info->dwFlags & CItemData::ITEM_FLAG_STACKABLE) && !IS_SET(info->dwAntiFlags, CItemData::ITEM_ANTIFLAG_STACK);
}

PyObject* cuberenewalSendRefine(PyObject* poSelf, PyObject* poArgs)
{
	int vnum;
	if (!PyTuple_GetInteger(poArgs, 0, &vnum))
		return Py_BuildException();

	int multiplier;
	if (!PyTuple_GetInteger(poArgs, 1, &multiplier))
		return Py_BuildException();

	int indexImprove;
	if (!PyTuple_GetInteger(poArgs, 2, &indexImprove))
		return Py_BuildException();

	int totalItemReq;
	if (!PyTuple_GetInteger(poArgs, 3, &totalItemReq)) {
		TraceError("err parsing arg3");
		return Py_BuildException();
	}

	TraceError("totalItemReq %d", totalItemReq);

	int itemReq[5] = { 0, 0, 0, 0, 0 };
	for (int i = 0; i < totalItemReq; ++i) {
		if (!PyTuple_GetInteger(poArgs, 4 + i, &itemReq[i])) {
			TraceError("err parsing");
			return Py_BuildException();
		}
		else {
			TraceError("pos %d vnum %d", i, itemReq[i]);
		}
	}

	for (int r = 0; r < totalItemReq; ++r)
		TraceError("itemReq %d", itemReq[r]);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendCubeRefinePacket(vnum, multiplier, indexImprove, itemReq);
	return Py_BuildNone();
}

PyObject* cuberenewalGetInfo(PyObject* poSelf, PyObject* poArgs)
{
	// const int32_t npcVnum = 20091;
	int npcVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &npcVnum))
		return Py_BuildException();

	PyObject* poList = PyList_New(0);
	for (const auto& data : CPythonCubeRenewal::Instance().GetDataVector()) {
		for (const auto& npc : data->npc_vnum) {
			if (npc == npcVnum) {
				PyObject* dictInfo = PyDict_New();
				PyDict_SetItemString(dictInfo, "npcVnum", Py_BuildValue("i", npc));

				int itemCount = 0;
				for (const auto& item : data->item) {
					PyObject* itemList = PyList_New(0);
					PyList_Append(itemList, Py_BuildValue("i", item.vnum));
					PyList_Append(itemList, Py_BuildValue("i", item.count));
					std::string key = "item";
					key += std::to_string(itemCount);
					PyDict_SetItemString(dictInfo, key.c_str(), itemList);
					++itemCount;
				}
				PyObject* rewList = PyList_New(0);
				PyList_Append(rewList, Py_BuildValue("i", data->reward[0].vnum));
				PyList_Append(rewList, Py_BuildValue("i", data->reward[0].count));
				PyDict_SetItemString(dictInfo, "reward", rewList);
				PyDict_SetItemString(dictInfo, "percent", Py_BuildValue("i", data->percent));
				PyDict_SetItemString(dictInfo, "gold", Py_BuildValue("i", data->gold));
				PyDict_SetItemString(dictInfo, "gem", Py_BuildValue("i", data->gem));
				PyDict_SetItemString(dictInfo, "category", Py_BuildValue("s", data->category.c_str()));
				PyDict_SetItemString(dictInfo, "set_value", Py_BuildValue("i", data->set_value));
				PyDict_SetItemString(dictInfo, "can_stack", Py_BuildValue("i", IsStackableItem(data->reward[0].vnum) == 1));
				PyDict_SetItemString(dictInfo, "allow_copy", Py_BuildValue("i", data->allow_copy));



				//if (strlen(szName) <= 0) 
				std::string stName = "";
				{
					std::string stNamef = szItemName(data->reward[0].vnum);
					

					for (int i = 0; i < stNamef.length(); ++i)
					{
						if (stNamef[i] == '+')
							break;

						stName += stNamef[i];
					}

					stName = stName.c_str();

				//	TraceError("vnum: %d, name: %s", data->reward[0].vnum, stName.c_str());
				}

				PyDict_SetItemString(dictInfo, "name", Py_BuildValue("s", stName.c_str()));

				PyList_Append(poList, dictInfo);
			}
		}
	}

	return poList;
}

PyObject* cuberenewalSendClosePacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendCubeRenewalClosePacket();
	return Py_BuildNone();
}

void intcuberenewal()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetInfo",						cuberenewalGetInfo,				METH_VARARGS },
		{ "SendRefine",						cuberenewalSendRefine,			METH_VARARGS },
		{ "SendClosePacket",				cuberenewalSendClosePacket,		METH_VARARGS },
		{ nullptr,							nullptr,						0 },
	};

	PyObject* poModule = Py_InitModule("cuberenewal", s_methods);
}
#endif
