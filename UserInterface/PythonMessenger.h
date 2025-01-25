#pragma once

class CPythonMessenger : public CSingleton<CPythonMessenger>
{
public:
	using TFriendNameMap = std::set<std::string>;
	typedef std::map<std::string, uint8_t> TGuildMemberStateMap;
#ifdef ENABLE_GM_MESSENGER_LIST
	typedef std::set<std::string> TGMNameMap;
#endif
#ifdef ENABLE_MESSENGER_BLOCK
	typedef std::set<std::string> TBlockNameMap;
#endif

	enum EMessengerGroupIndex
	{
		MESSENGER_GRUOP_INDEX_FRIEND,
		MESSENGER_GRUOP_INDEX_GUILD,
#ifdef ENABLE_GM_MESSENGER_LIST
		MESSENGER_GROUP_INDEX_GM,
#endif
#ifdef ENABLE_MESSENGER_BLOCK
		MESSENGER_GROUP_INDEX_BLOCK,
#endif
	};

public:
	CPythonMessenger() = default;
	~CPythonMessenger() = default;
	CLASS_DELETE_COPYMOVE(CPythonMessenger);

	void Destroy();

	// Friend
	void RemoveFriend(const char * c_szKey);
#if defined(ENABLE_MESSENGER_GET_FRIEND_NAMES) || defined(ENABLE_MAILBOX)
	const TFriendNameMap GetFriendNames();
#endif
#ifdef ENABLE_MESSENGER_BLOCK
	void RemoveBlock(const char* c_szKey);
	void OnBlockLogin(const char* c_szKey);
	void OnBlockLogout(const char* c_szKey);
	BOOL IsBlockByKey(const char* c_szKey);
	BOOL IsBlockFriendByName(const char* c_szName);
#endif

	void OnFriendLogin(const char * c_szKey);
	void OnFriendLogout(const char * c_szKey);
	void SetMobile(const char * c_szKey, uint8_t byState);
	BOOL IsFriendByKey(const char * c_szKey);
	BOOL IsFriendByName(const char * c_szName);

#ifdef ENABLE_GM_MESSENGER_LIST
	void OnGMLogin(const char* c_szKey);
	void OnGMLogout(const char* c_szKey);
#endif

	// Guild
	void AppendGuildMember(const char * c_szName);
	void RemoveGuildMember(const char * c_szName);
	void RemoveAllGuildMember();
	void LoginGuildMember(const char * c_szName);
	void LogoutGuildMember(const char * c_szName);
	void RefreshGuildMember();

	void SetMessengerHandler(PyObject * poHandler);

protected:
	TFriendNameMap m_FriendNameMap;
#ifdef ENABLE_GM_MESSENGER_LIST
	TGMNameMap m_GMNameMap;
#endif
#ifdef ENABLE_MESSENGER_BLOCK
	TBlockNameMap m_BlockNameMap;
#endif
	TGuildMemberStateMap m_GuildMemberStateMap;

private:
	PyObject * m_poMessengerHandler{nullptr};
};
