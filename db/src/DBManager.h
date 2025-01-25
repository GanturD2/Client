// vim:ts=8 sw=4
#ifndef __INC_METIN2_DB_DBMANAGER_H__
#define __INC_METIN2_DB_DBMANAGER_H__

// The purpose of the DB connection class is... to connect to DB, send a query, and get the result.
// handle everything
// Code by Kobung programmer Anoa~ = _=)b
#include <mysql/mysql.h>

#include "../../libsql/AsyncSQL.h"
#include <memory>

#define SQL_SAFE_LENGTH(size) (size * 2 + 1)
#define QUERY_SAFE_LENGTH(size) (1024 + SQL_SAFE_LENGTH(size))

class CQueryInfo
{
public:
	int iType;
	uint32_t dwIdent;
	void* pvData;
};

enum eSQL_SLOT
{
	SQL_PLAYER,
	SQL_ACCOUNT,
	SQL_COMMON,
	SQL_HOTBACKUP,
	SQL_MAX_NUM,
};

class CDBManager : public singleton<CDBManager>
{
protected:
	void Initialize();
	void Destroy();

public:
	CDBManager();
	virtual ~CDBManager();

	void Clear();
	void Quit();

	int Connect(int iSlot, const char* host, int port, const char* dbname, const char* user, const char* pass);

	void ReturnQuery(const char* c_pszQuery, int iType, uint32_t dwIdent, void* pvData, int iSlot = SQL_PLAYER);
	void AsyncQuery(const char* c_pszQuery, int iSlot = SQL_PLAYER);
	std::unique_ptr<SQLMsg> DirectQuery(const char* c_pszQuery, int iSlot = SQL_PLAYER);

	SQLMsg* PopResult();
	SQLMsg* PopResult(eSQL_SLOT slot);

	unsigned long EscapeString(void* to, const void* from, unsigned long length, int iSlot = SQL_PLAYER);

	uint32_t CountReturnQuery(int i) { return m_mainSQL[i] ? m_mainSQL[i]->CountQuery() : 0; }
	uint32_t CountReturnResult(int i) { return m_mainSQL[i] ? m_mainSQL[i]->CountResult() : 0; }
	uint32_t CountReturnQueryFinished(int i) { return m_mainSQL[i] ? m_mainSQL[i]->CountQueryFinished() : 0; }
	uint32_t CountReturnCopiedQuery(int i) { return m_mainSQL[i] ? m_mainSQL[i]->GetCopiedQueryCount() : 0; }

	uint32_t CountAsyncQuery(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->CountQuery() : 0; }
	uint32_t CountAsyncResult(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->CountResult() : 0; }
	uint32_t CountAsyncQueryFinished(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->CountQueryFinished() : 0; }
	uint32_t CountAsyncCopiedQuery(int i) { return m_asyncSQL[i] ? m_asyncSQL[i]->GetCopiedQueryCount() : 0; }

	void ResetCounter()
	{
		for (int i = 0; i < SQL_MAX_NUM; ++i)
		{
			if (m_mainSQL[i])
			{
				m_mainSQL[i]->ResetQueryFinished();
				m_mainSQL[i]->ResetCopiedQueryCount();
			}

			if (m_asyncSQL[i])
			{
				m_asyncSQL[i]->ResetQueryFinished();
				m_asyncSQL[i]->ResetCopiedQueryCount();
			}
		}
	}

private:
	std::unique_ptr<CAsyncSQL2> m_mainSQL[SQL_MAX_NUM];
	std::unique_ptr<CAsyncSQL2> m_directSQL[SQL_MAX_NUM];
	std::unique_ptr<CAsyncSQL2> m_asyncSQL[SQL_MAX_NUM];

	int m_quit; // looping flag

	//CHARSET
public:
	void SetLocale(const char* szLocale);
	void QueryLocaleSet();
private:

	//END_CHARSET
};

#endif
