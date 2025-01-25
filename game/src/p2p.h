
#ifndef P2P_MANAGER_H_
#define P2P_MANAGER_H_

#include <unordered_map>

#include "input.h"
#include "../../common/stl.h"

typedef struct _CCI
{
	uint32_t dwPID;
	long lMapIndex;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	uint8_t bEmpire;
	uint8_t bChannel;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	uint8_t bLanguage;
#endif
	char login[LOGIN_MAX_LEN + 1];
	int16_t level;

	LPDESC pkDesc;
} CCI;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
typedef struct _ShopCCI
{
	uint32_t dwPID;
	long lMapIndex;
	uint8_t bEmpire;
	uint8_t bChannel;
	LPDESC pkDesc;
} ShopCCI;

#include <iostream>
#include <memory>
typedef std::shared_ptr<ShopCCI> spShopCCI;
#endif

class P2P_MANAGER : public singleton<P2P_MANAGER>
{
public:
	P2P_MANAGER();
	~P2P_MANAGER();

	void RegisterAcceptor(LPDESC d);
	void UnregisterAcceptor(LPDESC d);

	void RegisterConnector(LPDESC d);
	void UnregisterConnector(LPDESC d);

	void EraseUserByDesc(LPDESC d);

	void FlushOutput();

	void Boot(LPDESC d);

	void Send(const void* c_pvData, int iSize, LPDESC except = nullptr);

	void Login(LPDESC d, const TPacketGGLogin* p);
	void Logout(const char* c_pszName);

	CCI* Find(const char* c_pszName);
	CCI* FindByPID(uint32_t pid);
	CCI* PickRandomCCI();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	spShopCCI FindShop(uint32_t pid);
#endif

	int GetCount();
	int GetEmpireUserCount(int idx);
	int GetDescCount();
	void GetP2PHostNames(std::string& hostNames);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void StartShopOffline(LPDESC d, const TPacketGGShopStartOffline* p);
	void EndShopOffline(uint32_t pid);
	int GetForeignShopCount();
#endif

	int CountPeerPlayers() const { return m_map_pkCCI.size(); }

private:
	void Logout(CCI* pkCCI);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void EndShopOffline(spShopCCI shopCCI);
#endif

	CInputProcessor* m_pkInputProcessor;
	int m_iHandleCount;

	typedef std::unordered_map<std::string, CCI*, stringhash> TCCIMap;
	typedef std::unordered_map<uint32_t, CCI*> TPIDCCIMap;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	typedef std::unordered_map<uint32_t, spShopCCI> TShopCCIMap;
#endif

	std::unordered_set<LPDESC> m_set_pkPeers;
	TCCIMap m_map_pkCCI;
	TPIDCCIMap m_map_dwPID_pkCCI;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	TShopCCIMap m_map_shopCCI;
#endif
	int m_aiEmpireUserCount[EMPIRE_MAX_NUM];
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	int m_aiEmpireShopCount[EMPIRE_MAX_NUM];
#endif
};

#endif /* P2P_MANAGER_H_ */
