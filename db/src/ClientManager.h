// vim:ts=8 sw=4
#ifndef __INC_CLIENTMANAGER_H__
#define __INC_CLIENTMANAGER_H__

#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cstring>

#include "../../common/stl.h"
#include "../../common/building.h"

#include "Peer.h"
#include "DBManager.h"
#include "LoginData.h"

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
# include "Shop.h"
#endif

class CPlayerTableCache;
class CItemCache;
class CItemPriceListTableCache;
#ifdef ENABLE_SKILL_COLOR_SYSTEM
class CSKillColorCache;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
class CShopCache;
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
#include <unordered_map>
class CAchievementCache;
#endif

class CPacketInfo
{
public:
	void Add(int header);
	void Reset();

	std::map<int, int> m_map_info;
};

size_t CreatePlayerSaveQuery(char* pszQuery, size_t querySize, TPlayerTable* pkTab);

class CClientManager : public CNetBase, public singleton<CClientManager>
{
public:
	typedef std::list<CPeer*> TPeerList;
	typedef std::unordered_map<uint32_t, CPlayerTableCache*> TPlayerTableCacheMap;
	typedef std::unordered_map<uint32_t, CItemCache*> TItemCacheMap;
	typedef std::unordered_set<CItemCache*, std::hash<CItemCache*> > TItemCacheSet;
	typedef std::unordered_map<uint32_t, TItemCacheSet*> TItemCacheSetPtrMap;
	typedef std::unordered_map<uint32_t, CItemPriceListTableCache*> TItemPriceListCacheMap;
	typedef std::unordered_map<int16_t, uint8_t> TChannelStatusMap;
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	typedef std::unordered_map<uint32_t, CSKillColorCache*> TSkillColorCacheMap;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	typedef std::unordered_map<uint32_t, CShopCache*> TShopCacheMap;
#endif
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	typedef std::unordered_map<uint32_t, CAchievementCache*> TAchievementCacheMap;
#endif

	// MYSHOP_PRICE_LIST
	/// Item Price Information List Request Information
	/**
	* first: Peer handle
	* second: ID of the requested player
	*/
	typedef std::pair<uint32_t, uint32_t> TItemPricelistReqInfo;
	// END_OF_MYSHOP_PRICE_LIST

	class ClientHandleInfo
	{
	public:
		uint32_t dwHandle;
		uint32_t account_id;
		uint32_t player_id;
		uint8_t account_index;
		char login[LOGIN_MAX_LEN + 1];
		char safebox_password[SAFEBOX_PASSWORD_MAX_LEN + 1];
		char ip[MAX_HOST_LENGTH + 1];

		TAccountTable* pAccountTable;
		TSafeboxTable* pSafebox;

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		TPlayerAchievementInfo pAchievementTable;
		ClientHandleInfo(DWORD argHandle,
			DWORD dwPID,
			DWORD accountId,
			TPlayerAchievementInfo& pAchTable) {
			dwHandle = argHandle;
			pSafebox = NULL;
			pAccountTable = NULL;
			player_id = dwPID;
			account_id = accountId;
			pAchievementTable = std::move(pAchTable);
		};
#endif

		ClientHandleInfo(uint32_t argHandle, uint32_t dwPID = 0)
		{
			dwHandle = argHandle;
			pSafebox = nullptr;
			pAccountTable = nullptr;
			player_id = dwPID;
		};
		//Constructor for German gift function
		ClientHandleInfo(uint32_t argHandle, uint32_t dwPID, uint32_t accountId)
		{
			dwHandle = argHandle;
			pSafebox = nullptr;
			pAccountTable = nullptr;
			player_id = dwPID;
			account_id = accountId;
		};

		~ClientHandleInfo()
		{
			if (pSafebox)
			{
				delete pSafebox;
				pSafebox = nullptr;
			}
		}
	};

public:
	CClientManager();
	~CClientManager();

	bool Initialize();
	time_t GetCurrentTime() noexcept;

	void MainLoop();
	void Quit() noexcept;

	void GetPeerP2PHostNames(std::string& peerHostNames);
	void SetTablePostfix(const char* c_pszTablePostfix);
	void SetPlayerIDStart(int iIDStart) noexcept;
	int GetPlayerIDStart() noexcept { return m_iPlayerIDStart; }

	int GetPlayerDeleteLevelLimit() noexcept { return m_iPlayerDeleteLevelLimit; }

	void SetChinaEventServer(bool flag) noexcept { m_bChinaEventServer = flag; }
	bool IsChinaEventServer() noexcept { return m_bChinaEventServer; }

	uint32_t GetUserCount() noexcept; // Returns the number of connected users.

	void SendAllGuildSkillRechargePacket();
	void SendTime();

	CPlayerTableCache* GetPlayerCache(uint32_t id);
	void PutPlayerCache(TPlayerTable* pNew);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	CShopCache* GetShopCache(uint32_t pid);
	void PutShopCache(TPlayerShopTableCache pNew);
#endif

	void CreateItemCacheSet(uint32_t dwID);
	TItemCacheSet* GetItemCacheSet(uint32_t dwID);
	void FlushItemCacheSet(uint32_t dwID);

	CItemCache* GetItemCache(uint32_t id);
	void PutItemCache(TPlayerItem* pNew, bool bSkipQuery = false);
	bool DeleteItemCache(uint32_t id);

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	CSKillColorCache* GetSkillColorCache(uint32_t id);
	void PutSkillColorCache(const TSkillColor* pNew);
	void UpdateSkillColorCache();
#endif

	void UpdatePlayerCache();
	void UpdateItemCache();

	// MYSHOP_PRICE_LIST
	/// Get the price information list cache.
	/**
	* @param [in] dwID Owner of price information list. (Player ID)
	* @return Pointer to the price list cache
	*/
	CItemPriceListTableCache* GetItemPriceListCache(uint32_t dwID);

	/// Put the price information list cache.
	/**
	* @param [in] pItemPriceList List of item price information to put in cache
	*
	* If the cache already exists, replace it instead of Update
	*/
	void PutItemPriceListCache(const TItemPriceListTable* pItemPriceList);


	/// Flush the item price information list cache for which the flush time has expired, and then delete it from the cache.
	void UpdateItemPriceListCache(void);
	// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void UpdateShopCache();
#endif

	void SendGuildSkillUsable(uint32_t guild_id, uint32_t dwSkillVnum, bool bUsable);

	void SetCacheFlushCountLimit(int iLimit) noexcept;

	template <class Func>
	Func for_each_peer(Func f);

	CPeer* GetAnyPeer() noexcept;

	void ForwardPacket(uint8_t header, const void* data, int size, uint8_t bChannel = 0, CPeer* except = nullptr);

	void SendNotice(const char* c_pszFormat, ...);

	// @fixme203 directly GetCommand instead of strcpy
	char* GetCommand(char* str, char* command) noexcept; //Function to get command from German gift function
	void ItemAward(CPeer* peer, const char* login); //German gift function

#ifdef ENABLE_GROWTH_PET_SYSTEM
	TItemTable* GetItemTableByVnum(uint32_t dwVnum);
#endif

protected:
	void Destroy();

private:
	bool InitializeTables();
	bool InitializeShopTable();
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	bool InitializeGemShopTable();
#endif
#ifdef ENABLE_MAILBOX
	bool InitializeMailBoxTable();
#endif
	bool InitializeMobTable();
	bool InitializeItemTable();
	bool InitializeQuestItemTable();
	bool InitializeSkillTable();
	bool InitializeRefineTable();
	bool InitializeBanwordTable();
	bool InitializeItemAttrTable();
	bool InitializeItemRareTable();
	bool InitializeLandTable();
	bool InitializeObjectProto();
	bool InitializeObjectTable();
	bool InitializeMonarch();
#ifdef ENABLE_EVENT_MANAGER
	bool InitializeEventTable();
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	bool InitializeBiologMissions();
	bool InitializeBiologRewards();
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	bool InitializeSungmaTable();
#endif

	// Reflect mob_proto and item_proto read from mob_proto.txt and item_proto.txt to real db.
	// Even if item_proto and mob_proto are not reflected in the db, there is no problem with the game running,
	// A problem occurs because the operation tool reads and writes item_proto and mob_proto of db.
	bool MirrorMobTableIntoDB();
	bool MirrorItemTableIntoDB();

	void AddPeer(socket_t fd);
	void RemovePeer(CPeer* pPeer);
	CPeer* GetPeer(IDENT ident);

	int AnalyzeQueryResult(SQLMsg* msg);
	int AnalyzeErrorMsg(CPeer* peer, SQLMsg* msg);

	int Process();

	void ProcessPackets(CPeer* peer);

	CLoginData* GetLoginData(uint32_t dwKey);
	CLoginData* GetLoginDataByLogin(const char* c_pszLogin);
	CLoginData* GetLoginDataByAID(uint32_t dwAID);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	CLoginData* GetLoginDataByPID(uint32_t dwPID);
#endif

	void InsertLoginData(CLoginData* pkLD);
	void DeleteLoginData(CLoginData* pkLD);

	bool InsertLogonAccount(const char* c_pszLogin, uint32_t dwHandle, const char* c_pszIP);
	bool DeleteLogonAccount(const char* c_pszLogin, uint32_t dwHandle);
	bool FindLogonAccount(const char* c_pszLogin);

	void GuildCreate(CPeer* peer, uint32_t dwGuildID);
	void GuildSkillUpdate(CPeer* peer, const TPacketGuildSkillUpdate* p);
	void GuildExpUpdate(CPeer* peer, const TPacketGuildExpUpdate* p);
	void GuildAddMember(CPeer* peer, const TPacketGDGuildAddMember* p);
	void GuildChangeGrade(CPeer* peer, const TPacketGuild* p);
	void GuildRemoveMember(CPeer* peer, const TPacketGuild* p);
	void GuildChangeMemberData(CPeer* peer, const TPacketGuildChangeMemberData* p);
	void GuildDisband(CPeer* peer, const TPacketGuild* p);
	void GuildWar(CPeer* peer, TPacketGuildWar* p);
	void GuildWarScore(CPeer* peer, const TPacketGuildWarScore* p);
	void GuildChangeLadderPoint(const TPacketGuildLadderPoint* p);
	void GuildUseSkill(const TPacketGuildUseSkill* p);
	void GuildDepositMoney(const TPacketGDGuildMoney* p);
#ifdef ENABLE_USE_MONEY_FROM_GUILD
	void GuildWithdrawMoney(const TPacketGDGuildMoney* p);
#else
	void GuildWithdrawMoney(CPeer* peer, const TPacketGDGuildMoney* p);
#endif
	void GuildWithdrawMoneyGiveReply(const TPacketGDGuildMoneyWithdrawGiveReply* p);
	void GuildWarBet(const TPacketGDGuildWarBet* p);
	void GuildChangeMaster(const TPacketChangeGuildMaster* p);

	void SetGuildWarEndTime(uint32_t guild_id1, uint32_t guild_id2, time_t tEndTime);

	void QUERY_BOOT(CPeer* peer, TPacketGDBoot* p);

	void QUERY_LOGIN(CPeer* peer, uint32_t dwHandle, SLoginPacket* data);
	void QUERY_LOGOUT(CPeer* peer, uint32_t dwHandle, const char*);

	void RESULT_LOGIN(CPeer* peer, SQLMsg* msg);

	void QUERY_PLAYER_LOAD(CPeer* peer, uint32_t dwHandle, const TPlayerLoadPacket*);
	void RESULT_COMPOSITE_PLAYER(CPeer* peer, SQLMsg* pMsg, uint32_t dwQID);
	void RESULT_PLAYER_LOAD(CPeer* peer, MYSQL_RES* pRes, const ClientHandleInfo* pkInfo);
	void RESULT_ITEM_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwPID);
	void RESULT_QUEST_LOAD(CPeer* pkPeer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwPID);
	// @fixme402 (RESULT_AFFECT_LOAD +dwRealPID)
	void RESULT_AFFECT_LOAD(CPeer* pkPeer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwRealPID);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void QUERY_SKILL_COLOR_LOAD(CPeer* peer, uint32_t dwHandle, TPlayerLoadPacket* packet);
	void RESULT_SKILL_COLOR_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle);
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	void RESULT_EXT_BATTLE_PASS_LOAD(CPeer* peer, MYSQL_RES* pRes, uint32_t dwHandle, uint32_t dwRealPID);
	void QUERY_SAVE_EXT_BATTLE_PASS(CPeer* peer, uint32_t dwHandle, TPlayerExtBattlePassMission* battlePass);
#endif

	// PLAYER_INDEX_CREATE_BUG_FIX
	void RESULT_PLAYER_INDEX_CREATE(CPeer* pkPeer, SQLMsg* msg);
	// END_PLAYER_INDEX_CREATE_BUG_FIX

	// MYSHOP_PRICE_LIST
	/// Result processing for price information load query
	/**
	* @param peer Pointer to the peer object of the game server requesting price information
	* @param pMsg Pointer to object received as result of query
	*
	* The loaded price information list is stored in the cache and the list is sent to the peer.
	*/
	void RESULT_PRICELIST_LOAD(CPeer* peer, SQLMsg* pMsg);

	/// Result processing for load query to update price information
	/**
	* @param pMsg Pointer to object received as result of query
	*
	* It creates a price information list cache with the loaded information and updates it with the updated price information.
	*/
	void RESULT_PRICELIST_LOAD_FOR_UPDATE(SQLMsg* pMsg);
	// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void RESULT_LOAD_PLAYER_SHOP_ITEMS(SQLMsg* pMsg);
#endif
	void QUERY_PLAYER_SAVE(CPeer* peer, uint32_t dwHandle, TPlayerTable*);

	void __QUERY_PLAYER_CREATE(CPeer* peer, uint32_t dwHandle, TPlayerCreatePacket*);
	void __QUERY_PLAYER_DELETE(CPeer* peer, uint32_t dwHandle, TPlayerDeletePacket*);
	void __RESULT_PLAYER_DELETE(CPeer* peer, SQLMsg* msg);

	void QUERY_PLAYER_COUNT(CPeer* pkPeer, const TPlayerCountPacket*);

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void QUERY_SKILL_COLOR_SAVE(const char* c_pData);
#endif
	void QUERY_ITEM_SAVE(CPeer* pkPeer, const char* c_pData);
	void QUERY_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData);
#ifdef ENABLE_GROWTH_PET_SYSTEM
	void QUERY_PET_ITEM_DESTROY(CPeer* pkPeer, const char* c_pData);
#endif
	void QUERY_ITEM_FLUSH(CPeer* pkPeer, const char* c_pData);

	void QUERY_QUEST_SAVE(CPeer* pkPeer, TQuestTable*, uint32_t dwLen);
	void QUERY_ADD_AFFECT(CPeer* pkPeer, const TPacketGDAddAffect* p);
	void QUERY_REMOVE_AFFECT(CPeer* pkPeer, const TPacketGDRemoveAffect* p);

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void QUERY_SAFEBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxLoadPacket*, int bMall);
#else
	void QUERY_SAFEBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxLoadPacket*, bool bMall);
#endif
#ifdef ENABLE_SAFEBOX_MONEY
	void QUERY_SAFEBOX_SAVE(CPeer* pkPeer, const TSafeboxTable* pTable);
#endif
	void QUERY_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxChangeSizePacket* p);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void QUERY_GUILDSTORAGE_CHANGE_SIZE(CPeer* pkPeer, uint32_t dwHandle, TGuildstorageChangeSizePacket* p);
#endif
	void QUERY_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, uint32_t dwHandle, const TSafeboxChangePasswordPacket* p);

	void RESULT_SAFEBOX_LOAD(CPeer* pkPeer, SQLMsg* msg);
	void RESULT_SAFEBOX_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void RESULT_GUILDSTORAGE_CHANGE_SIZE(CPeer* pkPeer, SQLMsg* msg);
#endif
	void RESULT_SAFEBOX_CHANGE_PASSWORD(CPeer* pkPeer, SQLMsg* msg);
	void RESULT_SAFEBOX_CHANGE_PASSWORD_SECOND(CPeer* pkPeer, SQLMsg* msg);

#ifdef ENABLE_MOVE_CHANNEL
	void QUERY_MOVE_CHANNEL(CPeer* pkPeer, uint32_t dwHandle, TMoveChannel*);
#endif

	void QUERY_EMPIRE_SELECT(CPeer* pkPeer, uint32_t dwHandle, const TEmpireSelectPacket* p);
	void QUERY_SETUP(CPeer* pkPeer, uint32_t dwHandle, const char* c_pData);

	void SendPartyOnSetup(CPeer* peer);

	void QUERY_HIGHSCORE_REGISTER(CPeer* peer, TPacketGDHighscore* data);
	void RESULT_HIGHSCORE_REGISTER(CPeer* pkPeer, SQLMsg* msg);

	void QUERY_FLUSH_CACHE(CPeer* pkPeer, const char* c_pData);

	void QUERY_PARTY_CREATE(CPeer* peer, TPacketPartyCreate* p);
	void QUERY_PARTY_DELETE(CPeer* peer, TPacketPartyDelete* p);
	void QUERY_PARTY_ADD(CPeer* peer, TPacketPartyAdd* p);
	void QUERY_PARTY_REMOVE(CPeer* peer, TPacketPartyRemove* p);
	void QUERY_PARTY_STATE_CHANGE(CPeer* peer, TPacketPartyStateChange* p);
	void QUERY_PARTY_SET_MEMBER_LEVEL(CPeer* peer, TPacketPartySetMemberLevel* p);

	void QUERY_RELOAD_PROTO();

	void QUERY_CHANGE_NAME(CPeer* peer, uint32_t dwHandle, TPacketGDChangeName* p);
	void GetPlayerFromRes(TPlayerTable* player_table, MYSQL_RES* res);

	void QUERY_SMS(CPeer* pkPeer, TPacketGDSMS* p);
	void QUERY_LOGIN_KEY(CPeer* pkPeer, TPacketGDLoginKey* p);

	void AddGuildPriv(const TPacketGiveGuildPriv* p);
	void AddEmpirePriv(const TPacketGiveEmpirePriv* p);
	void AddCharacterPriv(const TPacketGiveCharacterPriv* p);

	void MoneyLog(const TPacketMoneyLog* p);

	void QUERY_AUTH_LOGIN(CPeer* pkPeer, uint32_t dwHandle, TPacketGDAuthLogin* p);

	void QUERY_LOGIN_BY_KEY(CPeer* pkPeer, uint32_t dwHandle, TPacketGDLoginByKey* p);
	void RESULT_LOGIN_BY_KEY(CPeer* peer, SQLMsg* msg);

	void ChargeCash(const TRequestChargeCash* p);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void SendShops(CPeer* peer);
	void SendSingleShop(uint32_t pid);
	void PlayerLoginEvent(uint32_t pid);
# ifdef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
	void PlayerSetShopTime(uint32_t pid);
# endif
#endif

#ifdef ENABLE_YOHARA_SYSTEM
	void SungmaPremium(CPeer* peer, uint32_t dwHandle, uint32_t dwAccID);
#endif

	void LoadEventFlag();
	void SetEventFlag(TPacketSetEventFlag* p);
	void SendEventFlagsOnSetup(CPeer* peer);

	void MarriageAdd(TPacketMarriageAdd* p);
	void MarriageUpdate(const TPacketMarriageUpdate* p);
	void MarriageRemove(const TPacketMarriageRemove* p);

	void WeddingRequest(const TPacketWeddingRequest* p);
	void WeddingReady(const TPacketWeddingReady* p);
	void WeddingEnd(const TPacketWeddingEnd* p);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void ShopSaleResult(const char* data);
	void ShopRemove(const char* data);
#endif

#ifdef ENABLE_MAILBOX
	bool GET_MAIL(const char* name, const uint8_t index, SMailBoxTable** mail);
	void QUERY_MAILBOX_LOAD(CPeer* pkPeer, uint32_t dwHandle, TMailBox*);
	void QUERY_MAILBOX_CHECK_NAME(CPeer* pkPeer, uint32_t dwHandle, TMailBox*);
	void QUERY_MAILBOX_WRITE(CPeer* pkPeer, uint32_t dwHandle, TMailBoxTable*);
	void QUERY_MAILBOX_DELETE(CPeer* pkPeer, uint32_t dwHandle, TMailBox*);
	void QUERY_MAILBOX_CONFIRM(CPeer* pkPeer, uint32_t dwHandle, TMailBox*);
	void QUERY_MAILBOX_GET(CPeer* pkPeer, uint32_t dwHandle, TMailBox*);
	void QUERY_MAILBOX_UNREAD(CPeer* pkPeer, uint32_t dwHandle, TMailBox*);
	void MAILBOX_BACKUP();
#endif

	// MYSHOP_PRICE_LIST
	// Personal store price information

	/// Item price information list update packet (HEADER_GD_MYSHOP_PRICELIST_UPDATE) processing function
	/**
	* @param [in] Pointer to pPacket packet data
	*/
	void MyshopPricelistUpdate(const TItemPriceListTable* pPacket); // @fixme403 (TPacketMyshopPricelistHeader to TItemPriceListTable)

	/// Item price information list request packet (HEADER_GD_MYSHOP_PRICELIST_REQ) processing function
	/**
	* @param Pointer of the peer object of the game server that sent the peer packet
	* @param [in] dwHandle Handle of peer requesting price information
	* @param [in] dwPlayerID ID of the player who requested the price information list
	*/
	void MyshopPricelistRequest(CPeer* peer, uint32_t dwHandle, uint32_t dwPlayerID);
	// END_OF_MYSHOP_PRICE_LIST

	// Building
	void CreateObject(const TPacketGDCreateObject* p);
	void DeleteObject(uint32_t dwID);
	void UpdateLand(const uint32_t* pdw);

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void LanguageChange(CPeer* pPeer, const char* c_szData);
#endif

	// BLOCK_CHAT
	void BlockChat(TPacketBlockChat* p);
	// END_OF_BLOCK_CHAT

private:
	int m_looping;
	socket_t m_fdAccept; // Socket to be connected
	TPeerList m_peerList;

	CPeer* m_pkAuthPeer;

	// LoginKey, LoginData pair
	typedef std::unordered_map<uint32_t, CLoginData*> TLoginDataByLoginKey;
	TLoginDataByLoginKey m_map_pkLoginData;

	// Login LoginData pair
	typedef std::unordered_map<std::string, CLoginData*> TLoginDataByLogin;
	TLoginDataByLogin m_map_pkLoginDataByLogin;

	// AccountID LoginData pair
	typedef std::unordered_map<uint32_t, CLoginData*> TLoginDataByAID;
	TLoginDataByAID m_map_pkLoginDataByAID;

	// Login Login Data pair (Account that is actually logged in)
	typedef std::unordered_map<std::string, CLoginData*> TLogonAccountMap;
	TLogonAccountMap m_map_kLogonAccount;

	int m_iPlayerIDStart;
	int m_iPlayerDeleteLevelLimit;
	int m_iPlayerDeleteLevelLimitLower;
	bool m_bChinaEventServer;

	std::vector<TMobTable> m_vec_mobTable;
	std::vector<TItemTable> m_vec_itemTable;
	std::map<uint32_t, TItemTable*> m_map_itemTableByVnum;

	int m_iShopTableSize;
	TShopTable* m_pShopTable;

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	int m_iGemShopTableSize;
	TGemShopTable* m_pGemShopTable;
#endif

	int m_iRefineTableSize;
	TRefineTable* m_pRefineTable;

	std::vector<TSkillTable> m_vec_skillTable;
	std::vector<TBanwordTable> m_vec_banwordTable;
	std::vector<TItemAttrTable> m_vec_itemAttrTable;
	std::vector<TItemAttrTable> m_vec_itemRareTable;
	std::vector<building::TLand> m_vec_kLandTable;
	std::vector<building::TObjectProto> m_vec_kObjectProto;
	std::map<uint32_t, building::TObject*> m_map_pkObjectTable;
#ifdef ENABLE_EVENT_MANAGER
	std::vector<TEventTable> m_vec_eventTable;
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	std::vector<TBiologMissionsProto> m_vec_BiologMissions;
	std::vector<TBiologRewardsProto> m_vec_BiologRewards;
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	std::vector<TSungmaTable> m_vec_sungmaTable;
#endif

	bool m_bShutdowned;

	TPlayerTableCacheMap m_map_playerCache; // Player id key

	TItemCacheMap m_map_itemCache; // Item id is key
	TItemCacheSetPtrMap m_map_pkItemCacheSetPtr; // Player id is key, what item cache does this player have?
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	TSkillColorCacheMap m_map_SkillColorCache;
#endif

	// MYSHOP_PRICE_LIST
	/// List of item price information by player map. key: player ID, value: price information list cache
	TItemPriceListCacheMap m_mapItemPriceListCache; ///< List of item price information by player
	// END_OF_MYSHOP_PRICE_LIST

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	TShopCacheMap m_map_shopCache; // key is owners's pid
#endif

	TChannelStatusMap m_mChannelStatus;

	struct TPartyInfo
	{
		uint8_t bRole;
		uint8_t bLevel;

		TPartyInfo() noexcept : bRole(0), bLevel(0)
		{
		}
	};

	typedef std::map<uint32_t, TPartyInfo> TPartyMember;
	typedef std::map<uint32_t, TPartyMember> TPartyMap;
	typedef std::map<uint8_t, TPartyMap> TPartyChannelMap;
	TPartyChannelMap m_map_pkChannelParty;

	typedef std::map<std::string, long> TEventFlagMap;
	TEventFlagMap m_map_lEventFlag;

#ifdef ENABLE_MAILBOX
	typedef std::vector<TMailBoxTable> MailVec;
	typedef std::map<std::string, MailVec> MailBoxMap;
	MailBoxMap m_map_mailbox;
#endif

	uint8_t m_bLastHeader;
	int m_iCacheFlushCount;
	int m_iCacheFlushCountLimit;
#ifdef ENABLE_MAILBOX
	int m_iMailBoxBackupSec;
#endif

private:
	TItemIDRangeTable m_itemRange;

public:
	bool InitializeNowItemID();
	uint32_t GetItemID() noexcept;
	uint32_t GainItemID() noexcept;
	TItemIDRangeTable GetItemRange() { return m_itemRange; }

	//BOOT_LOCALIZATION
public:
	/* Local information initialization
	**/
	bool InitializeLocalization();

private:
	std::vector<tLocale> m_vec_Locale;
	//END_BOOT_LOCALIZATION
	//ADMIN_MANAGER

	bool __GetAdminInfo(const char* szIP, std::vector<tAdminInfo>& rAdminVec);
	bool __GetHostInfo(std::vector<std::string>& rIPVec);
	//END_ADMIN_MANAGER


	//RELOAD_ADMIN
	void ReloadAdmin(CPeer* peer, TPacketReloadAdmin* p);
	//END_RELOAD_ADMIN
	void BreakMarriage(CPeer* peer, const char* data);

	struct TLogoutPlayer
	{
		uint32_t pid;
		time_t time;

		bool operator < (const TLogoutPlayer& r)
		{
			return (pid < r.pid);
		}
	};

	typedef std::unordered_map<uint32_t, TLogoutPlayer*> TLogoutPlayerMap;
	TLogoutPlayerMap m_map_logout;

	void InsertLogoutPlayer(uint32_t pid);
	void DeleteLogoutPlayer(uint32_t pid);
	void UpdateLogoutPlayer();
	void UpdateItemCacheSet(uint32_t pid);

	void FlushPlayerCacheSet(uint32_t pid);

	//MONARCH
	void Election(CPeer* peer, uint32_t dwHandle, const char* p);
	void Candidacy(CPeer* peer, uint32_t dwHandle, const char* p);
	void AddMonarchMoney(const CPeer* peer, uint32_t dwHandle, const char* p);
	void TakeMonarchMoney(CPeer* peer, uint32_t dwHandle, const char* p);
	void ComeToVote(CPeer* peer, uint32_t dwHandle, const char* p);
	void RMCandidacy(CPeer* peer, uint32_t dwHandle, const char* p);
	void SetMonarch(CPeer* peer, uint32_t dwHandle, const char* p);
	void RMMonarch(CPeer* peer, uint32_t dwHandle, const char* p);

	void DecMonarchMoney(const CPeer* peer, uint32_t dwHandle, const char* p);
	void ChangeMonarchLord(CPeer* peer, uint32_t dwHandle, TPacketChangeMonarchLord* info);
	//END_MONARCH

	void BlockException(TPacketBlockException* data);

	void SendSpareItemIDRange(CPeer* peer);

	void UpdateHorseName(TPacketUpdateHorseName* data, CPeer* peer);
	void AckHorseName(uint32_t dwPID, CPeer* peer);
	void DeleteLoginKey(const TPacketDC* data);
	void ResetLastPlayerID(const TPacketNeedLoginLogInfo* data);

	//delete gift notify icon
	void DeleteAwardId(const TPacketDeleteAwardID* data);
	void UpdateChannelStatus(TChannelStatus* pData);
	void RequestChannelStatus(CPeer* peer, uint32_t dwHandle);
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	void GuildDungeon(const TPacketGDGuildDungeon* sPacket);
	void GuildDungeonGD(const TPacketGDGuildDungeonCD* sPacket);
#endif

#ifdef ENABLE_EVENT_MANAGER
	void UpdateEventStatus(uint32_t dwID);
	void EventNotification(TPacketSetEventFlag* p);
	void QUERY_RELOAD_EVENT();
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
# ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
	void PlayerLogoutEvent(uint32_t pid);
# endif

	// SHOPS
	void RESULT_LOAD_SHOPS(SQLMsg* msg);
	void ProcessShopPacket(CPeer* peer, uint32_t dwHandle, const char* data);

	void SaveShop(CPeer* peer, TPlayerShopTable* data);
	void SaveShopFlush(CPeer* peer, TPlayerShopTable* data);
	spShop GetShop(uint32_t pid);
# ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
	void CancelOpenShopOffline(uint32_t pid);
# endif
	void LoadPlayerShop(CPeer* peer, TPlayerShopTable* p);

	void WithdrawShopGold(CPeer* peer, uint32_t playerHandle, const char* data);
	void RollbackWithdrawShopGold(CPeer* peer, uint32_t playerHandle, const char* data);

	void SendMyShopInfo(CPeer* peer, uint32_t dwHandle, uint32_t pid);
# ifndef ENABLE_SHOP_NO_SPEND_MIN_IF_ONLINE
	void NotifyShopLogout(uint32_t pid);
# else
	void NotifyShopTime(uint32_t pid);
# endif
	void UpdateOfflineTime(const char* data);
	void RequestCloseShop(spShop shop, bool isDeleteShop);
	void RequestCloseShop(const char* data);
	void RenameShop(const char* data);

public:
	void UnbindPeer(const CPeer* peer);
#endif

#ifdef ENABLE_PROTO_FROM_DB
public:
	bool InitializeMobTableFromDB();
	bool InitializeItemTableFromDB();
protected:
	bool bIsProtoReadFromDB;
#endif

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
protected:
	void RESULT_ACHIEVEMENT_LOAD(CPeer* pkPeer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID, TPlayerAchievementInfo& pAchTable);
	void RESULT_ACHIEVEMENT_LOAD_ACHIEVEMENTS(CPeer* pkPeer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID, TPlayerAchievementInfo& pAchTable);
	void RESULT_ACHIEVEMENT_LOAD_TASKS(CPeer* pkPeer, MYSQL_RES* pRes, DWORD dwHandle, DWORD dwPID, TPlayerAchievementInfo& pAchTable);
	void QUERY_PLAYER_ACHIEVEMENTS_LOAD(CPeer* peer, DWORD dwHandle, TPlayerLoadPacket* packet);

	void RecvAchievementPacket(CPeer* peer, DWORD dwHandle, const char* c_pData);

	void UpdateAchievementsCache();
	void UpdateAchievementsRanking();

protected:
	TAchievementCacheMap m_map_AchievementCache;
	std::vector<TAchievementRanking> m_dwAchievementRanking;
#endif
};

template<class Func>
Func CClientManager::for_each_peer(Func f)
{
	TPeerList::iterator it;
	for (it = m_peerList.begin(); it != m_peerList.end(); ++it)
	{
		f(*it);
	}
	return f;
}
#endif
