#ifndef __INC_METIN_II_DB_MANAGER_H__
#define __INC_METIN_II_DB_MANAGER_H__

#include "../../libsql/AsyncSQL.h"

enum
{
	QUERY_TYPE_RETURN = 1,
	QUERY_TYPE_FUNCTION = 2,
	QUERY_TYPE_AFTER_FUNCTION = 3,
};

enum
{
	QID_SAFEBOX_SIZE,
	QID_DB_STRING,
	QID_AUTH_LOGIN,
	QID_LOTTO,
	QID_HIGHSCORE_REGISTER,
	QID_HIGHSCORE_SHOW,

	// BLOCK_CHAT
	QID_BLOCK_CHAT_LIST,
	// END_OF_BLOCK_CHAT

	// PROTECT_CHILD_FOR_NEWCIBN
	QID_PROTECT_CHILD,
	// END_PROTECT_CHILD_FOR_NEWCIBN

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	QID_GUILDSTORAGE_SIZE,
#endif
};

typedef struct SUseTime
{
	uint32_t dwLoginKey;
	char szLogin[LOGIN_MAX_LEN + 1];
	uint32_t dwUseSec;
	char szIP[MAX_HOST_LENGTH + 1];
} TUseTime;

class CQueryInfo
{
public:
	int iQueryType;
};

class CReturnQueryInfo : public CQueryInfo
{
public:
	int iType;
	uint32_t dwIdent;
	void* pvData;
};

class CFuncQueryInfo : public CQueryInfo
{
public:
	std::function<void(SQLMsg*)> f;
};

class CFuncAfterQueryInfo : public CQueryInfo
{
public:
	std::function<void()> f;
};

class CLoginData;


class DBManager : public singleton<DBManager>
{
public:
	DBManager();
	virtual ~DBManager();

	bool IsConnected();

	bool Connect(const char* host, const int port, const char* user, const char* pwd, const char* db);
	void Query(const char* c_pszFormat, ...);

	std::unique_ptr<SQLMsg> DirectQuery(const char* c_pszFormat, ...);
	void ReturnQuery(int iType, uint32_t dwIdent, void* pvData, const char* c_pszFormat, ...);

	void Process();
	void AnalyzeReturnQuery(SQLMsg* pmsg);

	void SendMoneyLog(uint8_t type, uint32_t vnum, int gold);

	void LoginPrepare(LPDESC d, uint32_t* pdwClientKey, int* paiPremiumTimes = nullptr);
	void SendAuthLogin(LPDESC d);
	void SendLoginPing(const char* c_pszLogin);

	void InsertLoginData(CLoginData* pkLD);
	void DeleteLoginData(CLoginData* pkLD);
	CLoginData* GetLoginData(uint32_t dwKey);

	uint32_t CountQuery() { return m_sql.CountQuery(); }
	uint32_t CountQueryResult() { return m_sql.CountResult(); }
	void ResetQueryResult() { m_sql.ResetQueryFinished(); }

	// BLOCK EXCEPTION
	void RequestBlockException(const char* login, int cmd);
	// BLOCK EXCEPTION

	void LoadDBString();
	const std::string& GetDBString(const std::string& key);
	const std::vector<std::string>& GetGreetMessage();

	void FuncQuery(std::function<void(SQLMsg*)> f, const char* c_pszFormat, ...); // The result is called as the argument (SQLMsg *) and freed by itself.
	void FuncAfterQuery(std::function<void()> f, const char* c_pszFormat, ...); // f is called when done void f(void) form

	size_t EscapeString(char* dst, size_t dstSize, const char* src, size_t srcSize);

private:
	SQLMsg* PopResult();

	CAsyncSQL m_sql;
	CAsyncSQL m_sql_direct;
	bool m_bIsConnect;

	std::map<std::string, std::string> m_map_dbstring;
	std::vector<std::string> m_vec_GreetMessage;
	std::map<uint32_t, CLoginData*> m_map_pkLoginData;
	std::vector<TUseTime> m_vec_kUseTime;
};

////////////////////////////////////////////////////////////////
typedef struct SHighscoreRegisterQueryInfo
{
	char szBoard[20 + 1];
	uint32_t dwPID;
	int iValue;
	bool bOrder;
} THighscoreRegisterQueryInfo;

// ACCOUNT_DB
class AccountDB : public singleton<AccountDB>
{
public:
	AccountDB();

	bool IsConnected();
	bool Connect(const char* host, const int port, const char* user, const char* pwd, const char* db);
	bool ConnectAsync(const char* host, const int port, const char* user, const char* pwd, const char* db, const char* locale);

	std::unique_ptr<SQLMsg> DirectQuery(const char* query);
	void ReturnQuery(int iType, uint32_t dwIdent, void* pvData, const char* c_pszFormat, ...);
	void AsyncQuery(const char* query);

	void SetLocale(const std::string& stLocale);

	void Process();

private:
	SQLMsg* PopResult();
	void AnalyzeReturnQuery(SQLMsg* pMsg);

	CAsyncSQL2 m_sql_direct;
	CAsyncSQL2 m_sql;
	bool m_IsConnect;

};
//END_ACCOUNT_DB

#endif
