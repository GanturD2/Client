// vim:ts=8 sw=4
#ifndef __INC_DB_CACHE_H__
#define __INC_DB_CACHE_H__

#include "../../common/cache.h"

#ifdef ENABLE_SKILL_COLOR_SYSTEM
class CSKillColorCache : public cache<TSkillColor>
{
public:
	CSKillColorCache();
	virtual ~CSKillColorCache();

	virtual void OnFlush();
};
#endif

class CItemCache : public cache<TPlayerItem>
{
public:
	CItemCache();
	virtual ~CItemCache();

	void Delete();
	virtual void OnFlush();
};

class CPlayerTableCache : public cache<TPlayerTable>
{
public:
	CPlayerTableCache();
	virtual ~CPlayerTableCache();

	virtual void OnFlush();

	uint32_t GetLastUpdateTime() { return m_lastUpdateTime; }
};

// MYSHOP_PRICE_LIST
/**
* @class CItemPriceListTableCache
* @brief ���λ����� ������ �������� ����Ʈ�� ���� ĳ�� class
* @version 05/06/10 Bang2ni - First release.
*/
class CItemPriceListTableCache : public cache< TItemPriceListTable >
{
public:

	/// Constructor
	/**
	* ĳ�� ���� �ð��� �����Ѵ�.
	*/
	CItemPriceListTableCache(void);
	virtual ~CItemPriceListTableCache();

	/// ����Ʈ ����
	/**
	* @param [in] pUpdateList ������ ����Ʈ
	*
	* ĳ�õ� ���������� �����Ѵ�.
	* �������� ����Ʈ�� ���� á�� ��� ������ ĳ�̵� �������� �ڿ��� ���� �����Ѵ�.
	*/
	void UpdateList(const TItemPriceListTable* pUpdateList);

	/// ���������� DB �� ����Ѵ�.
	virtual void OnFlush(void);

private:

	static const int s_nMinFlushSec; ///< Minimum cache expire time
};
// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
class CShopCache : public cache<TPlayerShopTableCache>
{
public:
	CShopCache();
	virtual ~CShopCache();

	virtual void OnFlush();

	uint32_t GetLastUpdateTime() { return m_lastUpdateTime; }
};
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
class CAchievementCache : public cache<TPlayerAchievementInfo>
{
public:
	CAchievementCache(bool bMemSet = true);
	virtual ~CAchievementCache();
	virtual void OnFlush();
};
#endif

#endif
