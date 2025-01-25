#include "StdAfx.h"
#include "PythonMessenger.h"

void CPythonMessenger::RemoveFriend(const char * c_szKey)
{
	m_FriendNameMap.erase(c_szKey);
}

void CPythonMessenger::OnFriendLogin(const char * c_szKey /*, const char * c_szName*/)
{
	m_FriendNameMap.emplace(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnFriendLogout(const char * c_szKey)
{
	m_FriendNameMap.emplace(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

#ifdef ENABLE_GM_MESSENGER_LIST
void CPythonMessenger::OnGMLogin(const char* c_szKey)
{
	m_GMNameMap.emplace(c_szKey);
	PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_GM, c_szKey));
}

void CPythonMessenger::OnGMLogout(const char* c_szKey)
{
	m_GMNameMap.emplace(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_GM, c_szKey));
}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
void CPythonMessenger::OnBlockLogin(const char* c_szKey)
{
	m_BlockNameMap.emplace(c_szKey);
	PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_BLOCK, c_szKey));
}

void CPythonMessenger::OnBlockLogout(const char* c_szKey)
{
	m_BlockNameMap.emplace(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_BLOCK, c_szKey));
}

BOOL CPythonMessenger::IsBlockByKey(const char* c_szKey)
{
	return (m_BlockNameMap.end() != m_BlockNameMap.find(c_szKey));
}

BOOL CPythonMessenger::IsBlockFriendByName(const char* c_szName)
{
	return IsBlockByKey(c_szName);
}

void CPythonMessenger::RemoveBlock(const char* c_szKey)
{
	m_BlockNameMap.erase(c_szKey);
	PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveList", Py_BuildValue("(is)", MESSENGER_GROUP_INDEX_BLOCK, c_szKey));
}
#endif

void CPythonMessenger::SetMobile(const char * c_szKey, uint8_t byState)
{
	m_FriendNameMap.emplace(c_szKey);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnMobile", Py_BuildValue("(isi)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey, byState));
}

BOOL CPythonMessenger::IsFriendByKey(const char * c_szKey)
{
	return m_FriendNameMap.end() != m_FriendNameMap.find(c_szKey);
}

BOOL CPythonMessenger::IsFriendByName(const char * c_szName)
{
	return IsFriendByKey(c_szName);
}

void CPythonMessenger::AppendGuildMember(const char * c_szName)
{
	if (m_GuildMemberStateMap.end() != m_GuildMemberStateMap.find(c_szName))
		return;

	LogoutGuildMember(c_szName);
}

void CPythonMessenger::RemoveGuildMember(const char * c_szName)
{
	m_GuildMemberStateMap.erase(c_szName);

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveList", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RemoveAllGuildMember()
{
	m_GuildMemberStateMap.clear();

	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveAllList", Py_BuildValue("(i)", MESSENGER_GRUOP_INDEX_GUILD));
}

void CPythonMessenger::LoginGuildMember(const char * c_szName)
{
	m_GuildMemberStateMap[c_szName] = 1;
	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::LogoutGuildMember(const char * c_szName)
{
	m_GuildMemberStateMap[c_szName] = 0;
	if (m_poMessengerHandler)
		PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout", Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RefreshGuildMember()
{
	for (auto & itor : m_GuildMemberStateMap)
	{
		if (itor.second)
			PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin",
								  Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, (itor.first).c_str()));
		else
			PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout",
								  Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, (itor.first).c_str()));
	}
}

void CPythonMessenger::Destroy()
{
	m_FriendNameMap.clear();
	m_GuildMemberStateMap.clear();
}

void CPythonMessenger::SetMessengerHandler(PyObject * poHandler)
{
	m_poMessengerHandler = poHandler;
}

#if defined(ENABLE_MESSENGER_GET_FRIEND_NAMES) || defined(ENABLE_MAILBOX)
const CPythonMessenger::TFriendNameMap CPythonMessenger::GetFriendNames()
{
	return m_FriendNameMap;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////

PyObject * messengerRemoveFriend(PyObject * poSelf, PyObject * poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonMessenger::Instance().RemoveFriend(szKey);
	return Py_BuildNone();
}

PyObject * messengerIsFriendByName(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonMessenger::Instance().IsFriendByName(szName));
}

#ifdef ENABLE_MESSENGER_BLOCK
PyObject* messengerIsBlockFriendByName(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonMessenger::Instance().IsBlockFriendByName(szName));
}
#endif

PyObject * messengerDestroy(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMessenger::Instance().Destroy();
	return Py_BuildNone();
}

PyObject * messengerRefreshGuildMember(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMessenger::Instance().RefreshGuildMember();
	return Py_BuildNone();
}

#if defined(ENABLE_MESSENGER_GET_FRIEND_NAMES) || defined(ENABLE_MAILBOX)
PyObject* messengerGetFriendNames(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonMessenger::TFriendNameMap friendNameMap = CPythonMessenger::Instance().GetFriendNames();
	PyObject * pyTupleFriendNames = PyTuple_New(friendNameMap.size());

	int iPos = 0;
	for (CPythonMessenger::TFriendNameMap::iterator itor = friendNameMap.begin(); itor != friendNameMap.end(); ++itor, ++iPos)
		PyTuple_SetItem(pyTupleFriendNames, iPos, Py_BuildValue("s", (*itor).c_str()));

	return pyTupleFriendNames;
}
#endif

PyObject * messengerSetMessengerHandler(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poEventHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poEventHandler))
		return Py_BuildException();

	CPythonMessenger::Instance().SetMessengerHandler(poEventHandler);
	return Py_BuildNone();
}

void initMessenger()
{
	static PyMethodDef s_methods[] = {{"RemoveFriend", messengerRemoveFriend, METH_VARARGS},
#if defined(ENABLE_MESSENGER_GET_FRIEND_NAMES) || defined(ENABLE_MAILBOX)
		{ "GetFriendNames",				messengerGetFriendNames,			METH_VARARGS },
#endif
									  {"IsFriendByName", messengerIsFriendByName, METH_VARARGS},
									  {"Destroy", messengerDestroy, METH_VARARGS},
									  {"RefreshGuildMember", messengerRefreshGuildMember, METH_VARARGS},
									  {"SetMessengerHandler", messengerSetMessengerHandler, METH_VARARGS},
#ifdef ENABLE_MESSENGER_BLOCK
		{ "IsBlockFriendByName",		messengerIsBlockFriendByName,		METH_VARARGS },
#endif
									  {nullptr, nullptr, 0}};

	Py_InitModule("messenger", s_methods);
}
