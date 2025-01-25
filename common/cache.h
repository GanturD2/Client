#ifndef __INC_COMMON_CACHE_H__
#define __INC_COMMON_CACHE_H__

template <typename T> class cache
{
public:
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	cache(bool bMemSet = true)
#else
	cache()
#endif
		: m_bNeedQuery(false), m_expireTime(600), m_lastUpdateTime(0)
	{
		m_lastFlushTime = time(0);
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (bMemSet)
#endif
			memset(&m_data, 0, sizeof(m_data));
	}

	T* Get(bool bUpdateTime = true)
	{
		if (bUpdateTime)
			m_lastUpdateTime = time(0);

		return &m_data;
	}

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	void Put(T& pNew, bool bSkipQuery = false)
	{
		m_data = std::move(pNew);
		m_lastUpdateTime = time(0);

		if (!bSkipQuery)
			m_bNeedQuery = true;
	}
#endif

	void Put(T* pNew, bool bSkipQuery = false)
	{
		thecore_memcpy(&m_data, pNew, sizeof(T));
		m_lastUpdateTime = time(0);

		if (!bSkipQuery)
			m_bNeedQuery = true;
	}

	bool CheckFlushTimeout()
	{
		if (m_bNeedQuery && time(0) - m_lastFlushTime > m_expireTime)
			return true;

		return false;
	}

	bool CheckTimeout()
	{
		if (time(0) - m_lastUpdateTime > m_expireTime)
			return true;

		return false;
	}

	void Flush()
	{
		if (!m_bNeedQuery)
			return;

		OnFlush();
		m_bNeedQuery = false;
		m_lastFlushTime = time(0); //(performance) When an object of a class is created, the constructors of all member variables are called consecutively in the order the variables are declared, even if you don't explicitly write them to the initialization list. You could avoid assigning 'm_lastFlushTime' a value by passing the value to the constructor in the initialization list. [useInitializationList]
	}

	virtual void OnFlush() = 0;


protected:
	T m_data;
	bool m_bNeedQuery;
	time_t m_expireTime;
	time_t m_lastUpdateTime;
	time_t m_lastFlushTime;
};

#endif
