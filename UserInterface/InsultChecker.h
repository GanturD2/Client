#pragma once

class CInsultChecker
{
public:
	CInsultChecker & GetSingleton() const;

public:
	CInsultChecker();
	virtual ~CInsultChecker();
	CLASS_DELETE_COPYMOVE(CInsultChecker);

	void Clear();

	void AppendInsult(const std::string & c_rstInsult);
	bool IsInsultIn(const char * c_szLine, uint32_t uLineLen);
	void FilterInsult(char * szLine, uint32_t uLineLen);

private:
	bool __GetInsultLength(const char * c_szWord, uint32_t * puInsultLen);
	bool __IsInsult(const char * c_szWord);

private:
	std::list<std::string> m_kList_stInsult;
};
