#ifndef __INC_METIN_II_GAME_INPUT_PROCESSOR__
#define __INC_METIN_II_GAME_INPUT_PROCESSOR__

#include "packet_info.h"

enum
{
	INPROC_CLOSE,
	INPROC_HANDSHAKE,
	INPROC_LOGIN,
	INPROC_MAIN,
	INPROC_DEAD,
	INPROC_DB,
	INPROC_UDP,
	INPROC_P2P,
	INPROC_AUTH,
};

void LoginFailure(LPDESC d, const char* c_pszStatus);

extern void SendShout(const char* szText, uint8_t bEmpire
#ifdef ENABLE_MESSENGER_BLOCK
	, const char* c_szName
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	, uint8_t bLanguage
#endif
#ifdef ENABLE_EMPIRE_FLAG
	, bool bFlag
#endif
);

class CInputProcessor
{
public:
	CInputProcessor();
#ifdef ENABLE_HAND_SHAKE_LIMIT
	virtual ~CInputProcessor();

	void SetBlockHostName(const char* c_szpHostName);
	int IsBlockHostName(const char* c_szpHostName, bool bClear = false);
#else
	virtual ~CInputProcessor() {};
#endif

	virtual bool Process(LPDESC d, const void* c_pvOrig, int iBytes, int& r_iBytesProceed);
	virtual uint8_t GetType() = 0;

	void BindPacketInfo(CPacketInfo* pPacketInfo);
	void Pong(LPDESC d);
	void Handshake(LPDESC d, const char* c_pData);
	void Version(LPCHARACTER ch, const char* c_pData);

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData) = 0;

	CPacketInfo* m_pPacketInfo;
	int m_iBufferLeft;

#ifdef ENABLE_HAND_SHAKE_LIMIT
	std::vector<const char*> m_vecLastBlockHostName;
	uint32_t m_dwHandShakePulse;
#endif

	CPacketInfoCG m_packetInfoCG;
};

class CInputClose : public CInputProcessor
{
public:
	virtual uint8_t GetType() { return INPROC_CLOSE; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData) { return m_iBufferLeft; }
};

class CInputHandshake : public CInputProcessor
{
public:
	CInputHandshake();
	virtual ~CInputHandshake();

	virtual uint8_t GetType() { return INPROC_HANDSHAKE; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

protected:
	void GuildMarkLogin(LPDESC d, const char* c_pData);

	CPacketInfo* m_pMainPacketInfo;
};

class CInputLogin : public CInputProcessor
{
public:
	virtual uint8_t GetType() { return INPROC_LOGIN; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

protected:
	void Login(LPDESC d, const char* data);
	void LoginByKey(LPDESC d, const char* data);

	void CharacterSelect(LPDESC d, const char* data);
	void CharacterCreate(LPDESC d, const char* data);
	void CharacterDelete(LPDESC d, const char* data);
	void Entergame(LPDESC d, const char* data);
	void Empire(LPDESC d, const char* c_pData);
	void GuildMarkCRCList(LPDESC d, const char* c_pData);
	// MARK_BUG_FIX
	void GuildMarkIDXList(LPDESC d, const char* c_pData);
	// END_OF_MARK_BUG_FIX
	void GuildMarkUpload(LPDESC d, const char* c_pData);
	int GuildSymbolUpload(LPDESC d, const char* c_pData, size_t uiBytes);
	void GuildSymbolCRC(LPDESC d, const char* c_pData);
	void ChangeName(LPDESC d, const char* data);
};

class CInputMain : public CInputProcessor
{
public:
	virtual uint8_t GetType() { return INPROC_MAIN; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

protected:
	void Attack(LPCHARACTER ch, const uint8_t header, const char* data);

	int Whisper(LPCHARACTER ch, const char* data, size_t uiBytes);
#ifdef ENABLE_DUNGEON_INFO_SYSTEM
	void DungeonInfo(LPCHARACTER ch, const char* c_pData);
#endif
	int Chat(LPCHARACTER ch, const char* data, size_t uiBytes);
	void ItemUse(LPCHARACTER ch, const char* data);
	void ItemDrop(LPCHARACTER ch, const char* data);
	void ItemDrop2(LPCHARACTER ch, const char* data);
#ifdef ENABLE_DESTROY_SYSTEM
	void ItemDestroy(LPCHARACTER ch, const char* data);
#endif
	void ItemMove(LPCHARACTER ch, const char* data);
	void ItemPickup(LPCHARACTER ch, const char* data);
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	void SoulRoulette(LPCHARACTER ch, const char* data);
#endif
	void ItemToItem(LPCHARACTER ch, const char* pcData);
	void QuickslotAdd(LPCHARACTER ch, const char* data);
	void QuickslotDelete(LPCHARACTER ch, const char* data);
	void QuickslotSwap(LPCHARACTER ch, const char* data);
#ifdef ENABLE_SPECIAL_INVENTORY
	int Shop(LPCHARACTER ch, const char* data);
#else
	int Shop(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
	void OnClick(LPCHARACTER ch, const char* data);
	void Exchange(LPCHARACTER ch, const char* data);
	void Position(LPCHARACTER ch, const char* data);
	void Move(LPCHARACTER ch, const char* data);
	int SyncPosition(LPCHARACTER ch, const char* data, size_t uiBytes);
	void FlyTarget(LPCHARACTER ch, const char* pcData, uint8_t bHeader);
	void UseSkill(LPCHARACTER ch, const char* pcData);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void SetSkillColor(LPCHARACTER ch, const char* pcData);
#endif

	void ScriptAnswer(LPCHARACTER ch, const void* pvData);
	void ScriptButton(LPCHARACTER ch, const void* pvData);
	void ScriptSelectItem(LPCHARACTER ch, const void* pvData);

	void QuestInputString(LPCHARACTER ch, const void* pvData);
#ifdef ENABLE_OX_RENEWAL
	void QuestInputLongString(LPCHARACTER ch, const void* pvData);
#endif
	void QuestConfirm(LPCHARACTER ch, const void* pvData);
	void Target(LPCHARACTER ch, const char* pcData);
	void Warp(LPCHARACTER ch, const char* pcData);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void SafeboxCheckin(LPCHARACTER ch, const char* c_pData, int bMall);
	void SafeboxCheckout(LPCHARACTER ch, const char* c_pData, int bMall);
#else
	void SafeboxCheckin(LPCHARACTER ch, const char* c_pData);
	void SafeboxCheckout(LPCHARACTER ch, const char* c_pData, bool bMall);
#endif
	void SafeboxItemMove(LPCHARACTER ch, const char* data);
	int Messenger(LPCHARACTER ch, const char* c_pData, size_t uiBytes);

#ifdef ENABLE_HUNTING_SYSTEM
	int ReciveHuntingAction(LPCHARACTER ch, const char* c_pData);
#endif

#ifdef ENABLE_MOVE_CHANNEL
	void MoveChannel(LPCHARACTER ch, const char* c_pData);
#endif

	void PartyInvite(LPCHARACTER ch, const char* c_pData);
	void PartyInviteAnswer(LPCHARACTER ch, const char* c_pData);
	void PartyRemove(LPCHARACTER ch, const char* c_pData);
	void PartySetState(LPCHARACTER ch, const char* c_pData);
	void PartyUseSkill(LPCHARACTER ch, const char* c_pData);
	void PartyParameter(LPCHARACTER ch, const char* c_pData);

#ifdef ENABLE_SAFEBOX_MONEY
	void SafeboxMoney(LPCHARACTER ch, const char* c_pData);
#endif

#ifdef ENABLE_INGAME_WIKI
	void Wikipedia(const LPCHARACTER& ch, const char* c_pData);
#endif

	int Guild(LPCHARACTER ch, const char* data, size_t uiBytes);
	void AnswerMakeGuild(LPCHARACTER ch, const char* c_pData);

	void Fishing(LPCHARACTER ch, const char* c_pData);
	void ItemGive(LPCHARACTER ch, const char* c_pData);
	void Hack(LPCHARACTER ch, const char* c_pData);
	int MyShop(LPCHARACTER ch, const char* c_pData, size_t uiBytes);

	void Refine(LPCHARACTER ch, const char* c_pData);
#ifdef ENABLE_HWID_BAN
	void HWIDManager(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_TICKET_SYSTEM
	int TicketSystem(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
	void Roulette(LPCHARACTER ch, const char* c_pData);
#ifdef ENABLE_SEND_TARGET_INFO
	void TargetInfoLoad(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_SWITCHBOT
	int Switchbot(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef ENABLE_CUBE_RENEWAL
	int CubeRenewalSend(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef ENABLE_ATTR_6TH_7TH
	void Attr67Send(LPCHARACTER ch, const char* data);
#endif
#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
	void WonExchange(LPCHARACTER ch, const char* pcData);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void RemoveMyShopItem(LPCHARACTER ch, const char* c_pData);
	void AddMyShopItem(LPCHARACTER ch, const char* c_pData);
	void ClosePlayerShop(LPCHARACTER ch);
	void OpenPlayerShop(LPCHARACTER ch);
# ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TIME
	void ReopenPlayerShop(LPCHARACTER ch);
# endif
	void WithdrawShopStash(LPCHARACTER ch, const TPacketCGShopWithdraw* pack);
	void RenameShop(LPCHARACTER ch, TPacketGCShopRename* pack);

#ifdef ENABLE_MYSHOP_DECO
	void MyShopDecoState(LPCHARACTER ch, const char* c_pData);
	void MyShopDecoAdd(LPCHARACTER ch, const char* c_pData);
#endif
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	void PrivateShopSearchSearch(LPCHARACTER ch, const char* data);
	void PrivateShopSearchBuy(LPCHARACTER ch, const char* data);
	void PrivateShopSearchClose(LPCHARACTER ch, const char* data);
#endif
#ifdef ENABLE_REFINE_ELEMENT
	void ElementsSpell(LPCHARACTER ch, const char* data);
#endif
#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
	void ExtendInvenRequest(LPCHARACTER ch, const char* c_pData);
	void ExtendInvenUpgrade(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_MOVE_COSTUME_ATTR
	void MoveCostumeAttrRequest(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	void AcceCloseRequest(LPCHARACTER ch, const char* c_pData);
	void AcceRefineRequest(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_AURA_SYSTEM
	int Aura(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	void Transmutation(LPCHARACTER ch, const char* c_pData);
#endif
#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	int GemShop(LPCHARACTER ch, const char* data, size_t uiBytes);
	void ScriptSelectItemEx(LPCHARACTER ch, const void* pvData);
#endif
#ifdef ENABLE_CHANGED_ATTR
	void ItemSelectAttr(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_MAILBOX
	void MailboxWrite(LPCHARACTER ch, const char* data);
	void MailboxConfirm(LPCHARACTER ch, const char* data);
	void MailboxProcess(LPCHARACTER ch, const char* c_pData);
#endif
#if defined(ENABLE_MINI_GAME_OKEY_NORMAL) || defined(ENABLE_MINI_GAME_CATCH_KING)
	int MiniGameOkeyCard(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef ENABLE_FISH_EVENT
	int FishEvent(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	void LanguageChange(LPCHARACTER ch, const char* c_pData);
	void TargetLanguageRequest(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_PARTY_MATCH
	void PartyMatch(LPCHARACTER ch, const char* data);
#endif
#ifdef ENABLE_EVENT_MANAGER
	void RequestEventQuest(LPCHARACTER ch, const void* pvData);
	void RequestEventData(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	int ReciveExtBattlePassActions(LPCHARACTER ch, const char* data, size_t uiBytes);
	int ReciveExtBattlePassPremium(LPCHARACTER ch, const char* data, size_t uiBytes);
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
public:
	int GrowthPet(LPCHARACTER ch, const char* data, size_t uiBytes);

	void GrowthPetHatching(LPCHARACTER ch, const char* c_pData);
	void GrowthPetLearnSkill(LPCHARACTER ch, const char* c_pData);
	void GrowthPetSkillUpgradeRequest(LPCHARACTER ch, const char* c_pData);
	void GrowthPetSkillUpgrade(LPCHARACTER ch, const char* c_pData);
	void GrowthPetFeedRequest(LPCHARACTER ch, const char* c_pData);
	void GrowthPetDeleteSkillRequest(LPCHARACTER ch, const char* c_pData);
	void GrowthPetDeleteAllSkillRequest(LPCHARACTER ch, const char* c_pData);
	void GrowthPetNameChangeRequest(LPCHARACTER ch, const char* c_pData);
#	ifdef ENABLE_PET_ATTR_DETERMINE
	void GrowthPetAttrDetermineRequest(LPCHARACTER ch, const char* c_pData);
	void GrowthPetAttrChangeRequest(LPCHARACTER ch, const char* c_pData);
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	void GrowthPetReviveRequest(LPCHARACTER ch, const char* c_pData);
#	endif
#endif
#ifdef ENABLE_FLOWER_EVENT
	void FlowerEventSend(LPCHARACTER ch, const char* data);
#endif
#ifdef ENABLE_BIOLOG_SYSTEM
	int BiologManager(LPCHARACTER ch, const char* c_pData, size_t uiBytes);
#endif
#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
	void LottoOpenWindow(LPCHARACTER ch, const char* c_pData);
	void LottoBuyTicket(LPCHARACTER ch, const char* c_pData);
	void LottoTicketOptions(LPCHARACTER ch, const char* c_pData);
	void LottoPickMoney(LPCHARACTER ch, const char* c_pData);
#endif
#ifdef ENABLE_FISHING_RENEWAL
	void FishingNew(LPCHARACTER ch, const char* c_pData);
#endif
};

class CInputDead : public CInputMain
{
public:
	virtual uint8_t GetType() { return INPROC_DEAD; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);
};

class CInputDB : public CInputProcessor
{
public:
	virtual bool Process(LPDESC d, const void* c_pvOrig, int iBytes, int& r_iBytesProceed);
	virtual uint8_t GetType() { return INPROC_DB; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

protected:
	void MapLocations(const char* c_pData);
	void LoginSuccess(uint32_t dwHandle, const char* data);
	void PlayerCreateFailure(LPDESC d, uint8_t bType); // 0 = general failure 1 = already exists
	void PlayerDeleteSuccess(LPDESC d, const char* data);
#ifdef ENABLE_DELETE_FAILURE_TYPE
	void PlayerDeleteFail(LPDESC d, const char* c_pData);
#else
	void PlayerDeleteFail(LPDESC d);
#endif
	void PlayerLoad(LPDESC d, const char* data);
	void PlayerCreateSuccess(LPDESC d, const char* data);
	void Boot(const char* data);
	void QuestLoad(LPDESC d, const char* c_pData);
	void SafeboxLoad(LPDESC d, const char* c_pData);
	void SafeboxChangeSize(LPDESC d, const char* c_pData);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void GuildstorageChangeSize(LPDESC d, const char* c_pData);
#endif
	void SafeboxWrongPassword(LPDESC d);
	void SafeboxChangePasswordAnswer(LPDESC d, const char* c_pData);
	void MallLoad(LPDESC d, const char* c_pData);
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void GuildstorageLoad(LPDESC d, const char* c_pData);
#endif
#ifdef ENABLE_MOVE_CHANNEL
	void MoveChannelRespond(LPDESC d, const char* c_pData);
#endif
	void EmpireSelect(LPDESC d, const char* c_pData);
	void P2P(const char* c_pData);
	void ItemLoad(LPDESC d, const char* c_pData);
	void AffectLoad(LPDESC d, const char* c_pData);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	void SkillColorLoad(LPDESC d, const char* c_pData);
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void MyShopInfoLoad(LPDESC d, const char* c_pData);
	void CloseShop(const char* c_pData);
#endif
	void GuildLoad(const char* c_pData);
	void GuildSkillUpdate(const char* c_pData);
	void GuildSkillRecharge();
	void GuildExpUpdate(const char* c_pData);
	void GuildAddMember(const char* c_pData);
	void GuildRemoveMember(const char* c_pData);
	void GuildChangeGrade(const char* c_pData);
	void GuildChangeMemberData(const char* c_pData);
	void GuildDisband(const char* c_pData);
	void GuildLadder(const char* c_pData);
	void GuildWar(const char* c_pData);
	void GuildWarScore(const char* c_pData);
	void GuildSkillUsableChange(const char* c_pData);
	void GuildMoneyChange(const char* c_pData);
#ifndef ENABLE_USE_MONEY_FROM_GUILD
	void GuildWithdrawMoney(const char* c_pData);
#endif
	void GuildWarReserveAdd(TGuildWarReserve* p);
	void GuildWarReserveUpdate(TGuildWarReserve* p);
	void GuildWarReserveDelete(uint32_t dwID);
	void GuildWarBet(TPacketGDGuildWarBet* p);
	void GuildChangeMaster(TPacketChangeGuildMaster* p);

	void LoginAlready(LPDESC d, const char* c_pData);

	void PartyCreate(const char* c_pData);
	void PartyDelete(const char* c_pData);
	void PartyAdd(const char* c_pData);
	void PartyRemove(const char* c_pData);
	void PartyStateChange(const char* c_pData);
	void PartySetMemberLevel(const char* c_pData);

	void Time(const char* c_pData);

	void ReloadProto(const char* c_pData);
	void ChangeName(LPDESC d, const char* data);

	void AuthLogin(LPDESC d, const char* c_pData);
	void ItemAward(const char* c_pData);

	void ChangeEmpirePriv(const char* c_pData);
	void ChangeGuildPriv(const char* c_pData);
	void ChangeCharacterPriv(const char* c_pData);

	void MoneyLog(const char* c_pData);

	void SetEventFlag(const char* c_pData);

	void CreateObject(const char* c_pData);
	void DeleteObject(const char* c_pData);
	void UpdateLand(const char* c_pData);

	void Notice(const char* c_pData);

	void MarriageAdd(TPacketMarriageAdd* p);
	void MarriageUpdate(TPacketMarriageUpdate* p);
	void MarriageRemove(TPacketMarriageRemove* p);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void SpawnPlayerShops(const char* c_pData);
	void SpinPlayerShopTimer(const char* c_pData);
	void StopPlayerShopTimer(const char* c_pData);
#endif

	void WeddingRequest(TPacketWeddingRequest* p);
	void WeddingReady(TPacketWeddingReady* p);
	void WeddingStart(TPacketWeddingStart* p);
	void WeddingEnd(TPacketWeddingEnd* p);

	void TakeMonarchMoney(LPDESC d, const char* data);
	void AddMonarchMoney(LPDESC d, const char* data);
	void DecMonarchMoney(LPDESC d, const char* data);
	void SetMonarch(LPDESC d, const char* data);

	void ChangeMonarchLord(TPacketChangeMonarchLordACK* data);
	void UpdateMonarchInfo(TMonarchInfo* data);
	void AddBlockCountryIp(TPacketBlockCountryIp* data);
	void BlockException(TPacketBlockException* data);

	// MYSHOP_PRICE_LIST
	/// Response packet (HEADER_DG_MYSHOP_PRICELIST_RES) processing function to request for item price information list
	/**
	* @param d The descriptor of the player who requested the item price information list
	* @param p Pointer to packet data
	*/
	void MyshopPricelistRes(LPDESC d, const TPacketMyshopPricelistHeader* p);
	// END_OF_MYSHOP_PRICE_LIST
	//
	//RELOAD_ADMIN
	void ReloadAdmin(const char* c_pData);
	//END_RELOAD_ADMIN

	void DetailLog(const TPacketNeedLoginLogInfo* info);
	// German Futures Function Test
	void ItemAwardInformer(TPacketItemAwardInfromer* data);

	void RespondChannelStatus(LPDESC desc, const char* pcData);

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void ReceiveShopSaleInfo(const char* c_pData);
	void WithdrawGoldResult(LPDESC desc, TPacketGoldWithdrawResult* p);
	void MyShopClose(const char* c_pData);
	void UpdateMyShopName(const char* c_pData);
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
	void GuildDungeon(const char* c_pData);
	void GuildDungeonCD(const char* c_pData);
#endif

#ifdef ENABLE_MAILBOX
	void MailBoxRespondLoad(LPDESC d, const char* c_pData);
	void MailBoxRespondName(LPDESC d, const char* c_pData);
	void MailBoxRespondUnreadData(LPDESC d, const char* c_pData);
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
	void ExtBattlePassLoad(LPDESC d, const char* c_pData);
#endif

#ifdef ENABLE_EVENT_MANAGER
	void EventNotification(const char* c_pData);
	void ReloadEvent(const char* c_pData);
#endif

#ifdef ENABLE_WEATHER_INFO
	void ReloadWeatherInfo(TPacketDGWeatherInfo* p);
#endif

protected:
	uint32_t m_dwHandle;
#ifdef ENABLE_WEATHER_INFO
	uint8_t byRainTimer;
	uint8_t byRainState;
#endif
};

class CInputUDP : public CInputProcessor
{
public:
	CInputUDP();
	virtual bool Process(LPDESC d, const void* c_pvOrig, int iBytes, int& r_iBytesProceed);

	virtual uint8_t GetType() { return INPROC_UDP; }
	void SetSockAddr(struct sockaddr_in& rSockAddr) { m_SockAddr = rSockAddr; };

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

protected:
	void Handshake(LPDESC lpDesc, const char* c_pData);
	void StateChecker(const char* c_pData);

protected:
	struct sockaddr_in m_SockAddr;
	CPacketInfoUDP m_packetInfoUDP;
};

class CInputP2P : public CInputProcessor
{
public:
	CInputP2P();
	virtual uint8_t GetType() { return INPROC_P2P; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

public:
	void Setup(LPDESC d, const char* c_pData);
	void Login(LPDESC d, const char* c_pData);
	void Logout(LPDESC d, const char* c_pData);
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void StartShopOffline(LPDESC d, const char* c_pData);
	void EndShopOffline(LPDESC d, const char* c_pData);
#endif
	int Relay(LPDESC d, const char* c_pData, size_t uiBytes);
	int Notice(LPDESC d, const char* c_pData, size_t uiBytes, bool bBigFont = false);
	int MonarchNotice(LPDESC d, const char* c_pData, size_t uiBytes);
	int MonarchTransfer(LPDESC d, const char* c_pData);
	int Guild(LPDESC d, const char* c_pData, size_t uiBytes);
	void Shout(const char* c_pData);
	void Disconnect(const char* c_pData);
	void MessengerAdd(const char* c_pData);
	void MessengerRemove(const char* c_pData);
#ifdef ENABLE_MESSENGER_BLOCK
	void MessengerBlockAdd(const char* c_pData);
	void MessengerBlockRemove(const char* c_pData);
#endif
	//void MessengerMobile(const char* c_pData);
	void FindPosition(LPDESC d, const char* c_pData);
	void WarpCharacter(const char* c_pData);
	void GuildWarZoneMapIndex(const char* c_pData);
	void Transfer(const char* c_pData);
	void XmasWarpSanta(const char* c_pData);
	void XmasWarpSantaReply(const char* c_pData);
	void LoginPing(LPDESC d, const char* c_pData);
	void BlockChat(const char* c_pData);
	void IamAwake(LPDESC d, const char* c_pData);
#ifdef ENABLE_MULTI_FARM_BLOCK
	void MultiFarm(const char* c_pData);
#endif
#ifdef ENABLE_SWITCHBOT
	void Switchbot(LPDESC d, const char* c_pData);
#endif
#ifdef ENABLE_BATTLE_FIELD
	int Command(LPDESC d, const char* c_pData, size_t uiBytes);
#endif
#ifdef ENABLE_RANKING_SYSTEM
	void LoadRanking(const char* c_pData);
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	void StoneEvent(LPDESC d, const char* c_pData);
#endif
#ifdef ENABLE_WORLD_BOSS
	void WorldBoss(LPDESC d, const char* c_pData);
#endif
	int BigNotice(LPDESC d, const char* c_pData, size_t uiBytes);
#ifdef ENABLE_EVENT_MANAGER
	void Event(const char* c_pData);
	void HideAndSeekEvent(const char* c_pData);
#endif
#ifdef ENABLE_BATTLE_PASS_SYSTEM
	void BattlePassEvent(const char* c_pData);
#endif

protected:
	CPacketInfoGG m_packetInfoGG;
};

class CInputAuth : public CInputProcessor
{
public:
	CInputAuth();
	virtual uint8_t GetType() { return INPROC_AUTH; }

protected:
	virtual int Analyze(LPDESC d, uint8_t bHeader, const char* c_pData);

public:
	void Login(LPDESC d, const char* c_pData);
};

#endif /* __INC_METIN_II_GAME_INPUT_PROCESSOR__ */
