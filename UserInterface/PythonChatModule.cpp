#include "StdAfx.h"
#include "PythonChat.h"
#include "PythonItem.h"
#include "../GameLib/ItemManager.h"

PyObject * chatSetChatColor(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	int r;
	if (!PyTuple_GetInteger(poArgs, 1, &r))
		return Py_BuildException();

	int g;
	if (!PyTuple_GetInteger(poArgs, 2, &g))
		return Py_BuildException();

	int b;
	if (!PyTuple_GetInteger(poArgs, 3, &b))
		return Py_BuildException();

	CPythonChat::Instance().SetChatColor(iType, r, g, b);
	return Py_BuildNone();
}

PyObject * chatClear(PyObject * poSelf, PyObject * poArgs)
{
	CPythonChat::Instance().Destroy();
	return Py_BuildNone();
}

PyObject * chatClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonChat::Instance().Close();
	return Py_BuildNone();
}

PyObject * chatCreateChatSet(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().CreateChatSet(iID));
}

PyObject * chatUpdate(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().Update(iID);
	return Py_BuildNone();
}

PyObject * chatRender(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().Render(iID);
	return Py_BuildNone();
}

PyObject * chatSetBoardState(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iState;
	if (!PyTuple_GetInteger(poArgs, 1, &iState))
		return Py_BuildException();

	CPythonChat::Instance().SetBoardState(iID, iState);

	return Py_BuildNone();
}

PyObject * chatSetPosition(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int ix;
	if (!PyTuple_GetInteger(poArgs, 1, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 2, &iy))
		return Py_BuildException();

	CPythonChat::Instance().SetPosition(iID, ix, iy);

	return Py_BuildNone();
}

PyObject * chatSetHeight(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
		return Py_BuildException();

	CPythonChat::Instance().SetHeight(iID, iHeight);

	return Py_BuildNone();
}

PyObject * chatSetStep(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iStep;
	if (!PyTuple_GetInteger(poArgs, 1, &iStep))
		return Py_BuildException();

	CPythonChat::Instance().SetStep(iID, iStep);

	return Py_BuildNone();
}

PyObject * chatToggleChatMode(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().ToggleChatMode(iID, iType);

	return Py_BuildNone();
}

PyObject * chatEnableChatMode(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().EnableChatMode(iID, iType);

	return Py_BuildNone();
}

PyObject * chatDisableChatMode(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().DisableChatMode(iID, iType);

	return Py_BuildNone();
}

PyObject * chatSetEndPos(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	float fPos;
	if (!PyTuple_GetFloat(poArgs, 1, &fPos))
		return Py_BuildException();

	CPythonChat::Instance().SetEndPos(iID, fPos);

	return Py_BuildNone();
}

PyObject * chatGetLineCount(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().GetLineCount(iID));
}

PyObject * chatGetVisibleLineCount(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().GetVisibleLineCount(iID));
}

PyObject * chatGetLineStep(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().GetLineStep(iID));
}

PyObject * chatAppendChat(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	char * szChat;
	if (!PyTuple_GetString(poArgs, 1, &szChat))
		return Py_BuildException();

	CPythonChat::Instance().AppendChat(iType, szChat);

	return Py_BuildNone();
}

PyObject * chatAppendChatWithDelay(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	char * szChat;
	if (!PyTuple_GetString(poArgs, 1, &szChat))
		return Py_BuildException();

	int iDelay;
	if (!PyTuple_GetInteger(poArgs, 2, &iDelay))
		return Py_BuildException();

	CPythonChat::Instance().AppendChatWithDelay(iType, szChat, iDelay);

	return Py_BuildNone();
}

PyObject * chatArrangeShowingChat(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().ArrangeShowingChat(iID);

	return Py_BuildNone();
}

PyObject * chatIgnoreCharacter(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().IgnoreCharacter(szName);

	return Py_BuildNone();
}

PyObject * chatIsIgnoreCharacter(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().IsIgnoreCharacter(szName);

	return Py_BuildNone();
}

PyObject * chatCreateWhisper(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().CreateWhisper(szName);

	return Py_BuildNone();
}

PyObject * chatAppendWhisper(PyObject * poSelf, PyObject * poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	char * szChat;
	if (!PyTuple_GetString(poArgs, 2, &szChat))
		return Py_BuildException();

	CPythonChat::Instance().AppendWhisper(iType, szName, szChat);
	return Py_BuildNone();
}

PyObject * chatRenderWhisper(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();

	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	CWhisper * pWhisper;
	if (CPythonChat::Instance().GetWhisper(szName, &pWhisper))
		pWhisper->Render(fx, fy);

	return Py_BuildNone();
}

PyObject * chatSetWhisperBoxSize(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	float fWidth;
	if (!PyTuple_GetFloat(poArgs, 1, &fWidth))
		return Py_BuildException();

	float fHeight;
	if (!PyTuple_GetFloat(poArgs, 2, &fHeight))
		return Py_BuildException();

	CWhisper * pWhisper;
	if (CPythonChat::Instance().GetWhisper(szName, &pWhisper))
		pWhisper->SetBoxSize(fWidth, fHeight);

	return Py_BuildNone();
}

PyObject * chatSetWhisperPosition(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	float fPosition;
	if (!PyTuple_GetFloat(poArgs, 1, &fPosition))
		return Py_BuildException();

	CWhisper * pWhisper;
	if (CPythonChat::Instance().GetWhisper(szName, &pWhisper))
		pWhisper->SetPosition(fPosition);

	return Py_BuildNone();
}

PyObject * chatClearWhisper(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().ClearWhisper(szName);

	return Py_BuildNone();
}

PyObject * chatInitWhisper(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poInterface;
	if (!PyTuple_GetObject(poArgs, 0, &poInterface))
		return Py_BuildException();

	CPythonChat::Instance().InitWhisper(poInterface);
	return Py_BuildNone();
}

PyObject * chatGetLinkFromHyperlink(PyObject * poSelf, PyObject * poArgs)
{
	char * szHyperlink;

	if (!PyTuple_GetString(poArgs, 0, &szHyperlink))
		return Py_BuildException();

	std::string stHyperlink(szHyperlink);
	std::vector<std::string> results;

	split_string(stHyperlink, ":", results, false);

	// item:vnum:flag:socket0:socket1:socket2
	if ("item" == results[0])
	{
#ifdef ENABLE_PROTO_RENEWAL
		if (results.size() < CPythonChat::HYPER_LINK_ITEM_SOCKET5 + 1)
#else
		if (results.size() < CPythonChat::HYPER_LINK_ITEM_SOCKET2 + 1)
#endif
		{
			return Py_BuildValue("s", "");
		}

		CItemData * pItemData = nullptr;

		if (CItemManager::Instance().GetItemDataPointer(htoi(results[1].c_str()), &pItemData))
		{
			char buf[1024] = {0};
			char itemlink[256];
			bool isAttr = false;

			int len = snprintf(itemlink, sizeof(itemlink),
				"item"
				":%x"    // HYPER_LINK_ITEM_VNUM
#ifdef ENABLE_SET_ITEM
				":%d"    // HYPER_LINK_ITEM_PRE_SET_VALUE
#endif
				":%x"    // HYPER_LINK_ITEM_FLAGS
				":%x"    // HYPER_LINK_ITEM_SOCKET0
				":%x"    // HYPER_LINK_ITEM_SOCKET1
				":%x",    // HYPER_LINK_ITEM_SOCKET2
				htoi(results[CPythonChat::HYPER_LINK_ITEM_VNUM].c_str()),
#ifdef ENABLE_SET_ITEM
				htoi(results[CPythonChat::HYPER_LINK_ITEM_PRE_SET_VALUE].c_str()),
#endif
				htoi(results[CPythonChat::HYPER_LINK_ITEM_FLAGS].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET0].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET1].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET2].c_str()));

#if (defined(ENABLE_GLOVE_SYSTEM) && defined(ENABLE_YOHARA_SYSTEM)) || (defined(ENABLE_PROTO_RENEWAL))
			len += snprintf(itemlink + len, sizeof(itemlink) - len,
				":%x"    // HYPER_LINK_ITEM_SOCKET3
				":%x"    // HYPER_LINK_ITEM_SOCKET4
				":%x",    // HYPER_LINK_ITEM_SOCKET5
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET3].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET4].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET5].c_str()));
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", htoi(results[CPythonChat::HYPER_LINK_ITEM_CHANGE_ITEM_VNUM].c_str())); // HYPER_LINK_ITEM_CHANGE_ITEM_VNUM
#endif

#ifdef ENABLE_REFINE_ELEMENT
			len += snprintf(itemlink + len, sizeof(itemlink) - len,
				":%x"	// HYPER_LINK_ITEM_GRADE_VALUE --> NOT OFFICIAL
				":%x"	// HYPER_LINK_ITEM_REFINE_ELEMENT_APPLY_TYPE
				":%x"	// HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE0
				":%x"	// HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE1
				":%x"	// HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE2
				":%x"	// HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE0
				":%x"	// HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE1
				":%x",	// HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE2
				htoi(results[CPythonChat::HYPER_LINK_ITEM_GRADE_VALUE].c_str()), // NOT OFFICIAL
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_APPLY_TYPE].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE0].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE1].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE2].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE0].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE1].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE2].c_str()));
#endif

#ifdef ENABLE_YOHARA_SYSTEM
			for (int s = CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0; s <= CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2; s += 2)
			{
				len += snprintf(itemlink + len, sizeof(itemlink) - len,
					":%x" // HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0~3
					":%lld", // HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0~3
					htoi(results[s].c_str()), atoi(results[s + 1].c_str()));
			}
#endif

			if (results.size() >= CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE1)
			{
				for (int i = CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE0; i < results.size(); i += 2)
				{
					len += snprintf(itemlink + len, sizeof(itemlink) - len,
						":%x" // HYPER_LINK_ITEM_ATTRIBUTE_TYPE0~6
						":%lld", // HYPER_LINK_ITEM_ATTRIBUTE_VALUE0~6
						htoi(results[i].c_str()), atoi(results[i + 1].c_str()));

					isAttr = true;
				}
			}

#ifdef ENABLE_YOHARA_SYSTEM
			len += snprintf(itemlink + len, sizeof(itemlink) - len,
				":%x"	// HYPER_LINK_ITEM_RANDOM_VALUE0
				":%x"	// HYPER_LINK_ITEM_RANDOM_VALUE1
				":%x"	// HYPER_LINK_ITEM_RANDOM_VALUE2
				":%x",	// HYPER_LINK_ITEM_RANDOM_VALUE3
				htoi(results[CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE0].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE1].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE2].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE3].c_str()));
#endif

			if (isAttr)
				snprintf(buf, sizeof(buf), "|cffffc700|H%s|h[%s]|h|r", itemlink, pItemData->GetName());
			else
				snprintf(buf, sizeof(buf), "|cfff1e6c0|H%s|h[%s]|h|r", itemlink, pItemData->GetName());

			return Py_BuildValue("s", buf);
		}
	}

	return Py_BuildValue("s", "");
}

#ifdef ENABLE_CHAT_SETTINGS
PyObject* chatDeleteChatSet(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().DeleteChatSet(iID);
	return Py_BuildNone();
}
#endif
#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
PyObject* chatEnableCountryMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().EnableCountryMode(iID, iType);

	return Py_BuildNone();
}
PyObject* chatDisableCountryMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().DisableCountryMode(iID, iType);

	return Py_BuildNone();
}
#endif

void initChat()
{
	static PyMethodDef s_methods[] = {{"SetChatColor", chatSetChatColor, METH_VARARGS},
									  {"Clear", chatClear, METH_VARARGS},
									  {"Close", chatClose, METH_VARARGS},

									  {"CreateChatSet", chatCreateChatSet, METH_VARARGS},
									  {"Update", chatUpdate, METH_VARARGS},
									  {"Render", chatRender, METH_VARARGS},

									  {"SetBoardState", chatSetBoardState, METH_VARARGS},
									  {"SetPosition", chatSetPosition, METH_VARARGS},
									  {"SetHeight", chatSetHeight, METH_VARARGS},
									  {"SetStep", chatSetStep, METH_VARARGS},
									  {"ToggleChatMode", chatToggleChatMode, METH_VARARGS},
									  {"EnableChatMode", chatEnableChatMode, METH_VARARGS},
									  {"DisableChatMode", chatDisableChatMode, METH_VARARGS},
									  {"SetEndPos", chatSetEndPos, METH_VARARGS},

									  {"GetLineCount", chatGetLineCount, METH_VARARGS},
									  {"GetVisibleLineCount", chatGetVisibleLineCount, METH_VARARGS},
									  {"GetLineStep", chatGetLineStep, METH_VARARGS},

									  // Chat
									  {"AppendChat", chatAppendChat, METH_VARARGS},
									  {"AppendChatWithDelay", chatAppendChatWithDelay, METH_VARARGS},
									  {"ArrangeShowingChat", chatArrangeShowingChat, METH_VARARGS},

									  // Ignore
									  {"IgnoreCharacter", chatIgnoreCharacter, METH_VARARGS},
									  {"IsIgnoreCharacter", chatIsIgnoreCharacter, METH_VARARGS},

									  // Whisper
									  {"CreateWhisper", chatCreateWhisper, METH_VARARGS},
									  {"AppendWhisper", chatAppendWhisper, METH_VARARGS},
									  {"RenderWhisper", chatRenderWhisper, METH_VARARGS},
									  {"SetWhisperBoxSize", chatSetWhisperBoxSize, METH_VARARGS},
									  {"SetWhisperPosition", chatSetWhisperPosition, METH_VARARGS},
									  {"ClearWhisper", chatClearWhisper, METH_VARARGS},
									  {"InitWhisper", chatInitWhisper, METH_VARARGS},

									  // Link
									  {"GetLinkFromHyperlink", chatGetLinkFromHyperlink, METH_VARARGS},

#ifdef ENABLE_CHAT_SETTINGS
										{ "DeleteChatSet",			chatDeleteChatSet,			METH_VARARGS },
#endif
#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
										{ "EnableCountryMode",	chatEnableCountryMode,	METH_VARARGS },
										{ "DisableCountryMode",	chatDisableCountryMode,	METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("chatm2g", s_methods);	//@fixme430

	PyModule_AddIntConstant(poModule, "CHAT_TYPE_TALKING",			CHAT_TYPE_TALKING);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_INFO",				CHAT_TYPE_INFO);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_NOTICE",			CHAT_TYPE_NOTICE);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_PARTY",			CHAT_TYPE_PARTY);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_GUILD",			CHAT_TYPE_GUILD);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_COMMAND",			CHAT_TYPE_COMMAND);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_SHOUT",			CHAT_TYPE_SHOUT);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_WHISPER",			CHAT_TYPE_WHISPER);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_BIG_NOTICE",		CHAT_TYPE_BIG_NOTICE);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_MONARCH_NOTICE",	CHAT_TYPE_MONARCH_NOTICE);
#ifdef ENABLE_OX_RENEWAL
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_CONTROL_NOTICE",	CHAT_TYPE_CONTROL_NOTICE);
#endif
#ifdef ENABLE_DICE_SYSTEM
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_DICE_INFO",		CHAT_TYPE_DICE_INFO);
#endif
#ifdef ENABLE_12ZI
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_MISSION",			CHAT_TYPE_MISSION);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_SUB_MISSION",		CHAT_TYPE_SUB_MISSION);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_CLEAR_MISSION",	CHAT_TYPE_CLEAR_MISSION);
#endif
#ifdef ENABLE_CHAT_SETTINGS_EXTEND
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_EXP_INFO",			CHAT_TYPE_EXP_INFO);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_ITEM_INFO",		CHAT_TYPE_ITEM_INFO);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_MONEY_INFO",		CHAT_TYPE_MONEY_INFO);
#endif

	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_CHAT",		CPythonChat::WHISPER_TYPE_CHAT);
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_SYSTEM",	CPythonChat::WHISPER_TYPE_SYSTEM);
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_GM",		CPythonChat::WHISPER_TYPE_GM);

	PyModule_AddIntConstant(poModule, "BOARD_STATE_VIEW",		CPythonChat::BOARD_STATE_VIEW);
	PyModule_AddIntConstant(poModule, "BOARD_STATE_EDIT",		CPythonChat::BOARD_STATE_EDIT);
	PyModule_AddIntConstant(poModule, "BOARD_STATE_LOG",		CPythonChat::BOARD_STATE_LOG);

	PyModule_AddIntConstant(poModule, "CHAT_SET_CHAT_WINDOW",	0);
	PyModule_AddIntConstant(poModule, "CHAT_SET_LOG_WINDOW",	1);

	// HyperLink
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_KEYWORD", CPythonChat::HYPER_LINK_ITEM_KEYWORD);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_VNUM", CPythonChat::HYPER_LINK_ITEM_VNUM);
#ifdef __UNKNOWN__
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PRE_SET_VALUE", CPythonChat::HYPER_LINK_ITEM_PRE_SET_VALUE);
#endif
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_FLAGS", CPythonChat::HYPER_LINK_ITEM_FLAGS);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET0", CPythonChat::HYPER_LINK_ITEM_SOCKET0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET1", CPythonChat::HYPER_LINK_ITEM_SOCKET1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET2", CPythonChat::HYPER_LINK_ITEM_SOCKET2);
#ifdef ENABLE_PROTO_RENEWAL
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET3", CPythonChat::HYPER_LINK_ITEM_SOCKET3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET4", CPythonChat::HYPER_LINK_ITEM_SOCKET4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET5", CPythonChat::HYPER_LINK_ITEM_SOCKET5);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_CHANGE_ITEM_VNUM", CPythonChat::HYPER_LINK_ITEM_CHANGE_ITEM_VNUM);
#endif
#ifdef ENABLE_REFINE_ELEMENT
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_GRADE_VALUE", CPythonChat::HYPER_LINK_ITEM_GRADE_VALUE); // NOT OFFICIAL
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_APPLY_TYPE", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_APPLY_TYPE);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE0", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE1", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE2", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_VALUE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE0", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE1", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE2", CPythonChat::HYPER_LINK_ITEM_REFINE_ELEMENT_BONUS_VALUE2);
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE1", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE1", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE2", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2);
	//PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE3", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE3);
	//PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_RANDOM_VALUE0", CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_RANDOM_VALUE1", CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_RANDOM_VALUE2", CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_RANDOM_VALUE3", CPythonChat::HYPER_LINK_ITEM_RANDOM_VALUE3);
#endif
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE0", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE0", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE1", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE1", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE2", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE2", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE3", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE3", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE4", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE4", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE5", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE5);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE5", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE5);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE6", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE6);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE6", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE6);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_MAX", CPythonChat::HYPER_LINK_ITEM_MAX);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_KEYWORD", CPythonChat::HYPER_LINK_ITEM_PET_KEYWORD);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_VNUM", CPythonChat::HYPER_LINK_ITEM_PET_VNUM);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_PRE_SET_VALUE", CPythonChat::HYPER_LINK_ITEM_PET_PRE_SET_VALUE);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_FLAGS", CPythonChat::HYPER_LINK_ITEM_PET_FLAGS);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SOCKET0", CPythonChat::HYPER_LINK_ITEM_PET_SOCKET0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SOCKET1", CPythonChat::HYPER_LINK_ITEM_PET_SOCKET1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SOCKET2", CPythonChat::HYPER_LINK_ITEM_PET_SOCKET2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SOCKET3", CPythonChat::HYPER_LINK_ITEM_PET_SOCKET3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SOCKET4", CPythonChat::HYPER_LINK_ITEM_PET_SOCKET4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SOCKET5", CPythonChat::HYPER_LINK_ITEM_PET_SOCKET5);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_LEVEL", CPythonChat::HYPER_LINK_ITEM_PET_LEVEL);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_AGE", CPythonChat::HYPER_LINK_ITEM_PET_AGE);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_EVOL_LEVEL", CPythonChat::HYPER_LINK_ITEM_PET_EVOL_LEVEL);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_HP", CPythonChat::HYPER_LINK_ITEM_PET_HP);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_DEF", CPythonChat::HYPER_LINK_ITEM_PET_DEF);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SP", CPythonChat::HYPER_LINK_ITEM_PET_SP);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_LIFE", CPythonChat::HYPER_LINK_ITEM_PET_LIFE);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_LIFE_MAX", CPythonChat::HYPER_LINK_ITEM_PET_LIFE_MAX);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL_COUNT", CPythonChat::HYPER_LINK_ITEM_PET_SKILL_COUNT);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_BAG_PET_VNUM", CPythonChat::HYPER_LINK_ITEM_PET_BAG_PET_VNUM);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_NICKNAME", CPythonChat::HYPER_LINK_ITEM_PET_NICKNAME);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL0_VNUM", CPythonChat::HYPER_LINK_ITEM_PET_SKILL0_VNUM);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL0_LEVEL", CPythonChat::HYPER_LINK_ITEM_PET_SKILL0_LEVEL);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL1_VNUM", CPythonChat::HYPER_LINK_ITEM_PET_SKILL1_VNUM);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL1_LEVEL", CPythonChat::HYPER_LINK_ITEM_PET_SKILL1_LEVEL);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL2_VNUM", CPythonChat::HYPER_LINK_ITEM_PET_SKILL2_VNUM);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_SKILL2_LEVEL", CPythonChat::HYPER_LINK_ITEM_PET_SKILL2_LEVEL);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PET_MAX", CPythonChat::HYPER_LINK_ITEM_PET_MAX);
#endif
}
