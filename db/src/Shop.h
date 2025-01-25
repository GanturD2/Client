#ifndef __INC_METIN_II_DB_SHOP_H__
#define __INC_METIN_II_DB_SHOP_H__

#include "../../common/CommonDefines.h"

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP

#include <memory>
#include "Peer.h"

class Shop
{
public:
	Shop();
	void Initialize();

	bool HasPeer() const { return m_linkedPeer != nullptr; };
	void BindPeer(CPeer* peer) { m_linkedPeer = peer; };
	CPeer* GetPeer() const { return m_linkedPeer; };

	void SetTable(const TPlayerShopTable &newTab) { m_table = newTab; }
	TPlayerShopTable* GetTable() { return &m_table; }

	void SetItem(int index, TShopItemTable table);

	// Info getters
	uint32_t GetOwnerPID() const { return m_table.pid; }
	std::string GetOwnerName() const { return m_table.playerName; }

	void SetClosed(bool status) { m_table.closed = status; }
	bool IsClosed() const { return m_table.closed; }
	bool HasItems() const;

	void SetOfflineMinutesLeft(int minutes) { m_offlineMinutesLeft = minutes; }
	int GetOfflineMinutesLeft() { return m_offlineMinutesLeft; }

	void AlterGoldStash(uint32_t price, bool bAdd);
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	void AlterChequeStash(uint32_t cheque, bool bAdd);
#endif

	void SetGoldStash(uint32_t stash) { m_goldStash = m2::minmax<uint32_t>(0, stash, GOLD_MAX); }
	uint32_t GetGoldStash() const { return m_goldStash; };

#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	void SetChequeStash(uint32_t cheque_stash) { m_chequeStash = m2::minmax<uint32_t>(0, cheque_stash, CHEQUE_MAX); }
	uint32_t GetChequeStash() const { return m_chequeStash; };
#endif

	TPlayerShopTableCache GetCacheTable();
	void SetName(std::string& newName) { strlcpy(m_table.shopName, newName.c_str(), sizeof(m_table.shopName)); }

	bool IsOwnerOnline() const { return m_isOwnerOnline; }
	void SetOwnerOnline(bool value) { m_isOwnerOnline = value; }

	bool RemoveItem(int display_pos);
	int FindItem(int display_pos);

private:
	TPlayerShopTable m_table;
	uint32_t m_goldStash;
#ifdef ENABLE_CHEQUE_SYSTEM	//OFFLINE_SHOP_CHEQUE
	uint32_t m_chequeStash;
#endif
	int m_offlineMinutesLeft;

	CPeer* m_linkedPeer;
	bool m_isOwnerOnline;
};

typedef std::shared_ptr<Shop> spShop;

#endif
#endif
