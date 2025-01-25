#pragma once

#include "../EterLib/FuncObject.h"
#include "../EterLib/NetStream.h"
#include "../EterLib/NetPacketHeaderMap.h"

#include "InsultChecker.h"

#include "Packet.h"
#ifdef ENABLE_SWITCHBOT
#	include "PythonSwitchbot.h"
#endif

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
public:
	enum
	{
		SERVER_COMMAND_LOG_OUT = 0,
		SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
		SERVER_COMMAND_QUIT = 2,

		MAX_ACCOUNT_PLAYER
	};

	enum
	{
		ERROR_NONE,
		ERROR_UNKNOWN,
		ERROR_CONNECT_MARK_SERVER,
		ERROR_LOAD_MARK,
		ERROR_MARK_WIDTH,
		ERROR_MARK_HEIGHT,

		// MARK_BUG_FIX
		ERROR_MARK_UPLOAD_NEED_RECONNECT,
		ERROR_MARK_CHECK_NEED_RECONNECT,
		// END_OF_MARK_BUG_FIX
	};

	enum
	{
		ACCOUNT_CHARACTER_SLOT_ID,
		ACCOUNT_CHARACTER_SLOT_NAME,
		ACCOUNT_CHARACTER_SLOT_RACE,
		ACCOUNT_CHARACTER_SLOT_LEVEL,
		ACCOUNT_CHARACTER_SLOT_STR,
		ACCOUNT_CHARACTER_SLOT_DEX,
		ACCOUNT_CHARACTER_SLOT_HTH,
		ACCOUNT_CHARACTER_SLOT_INT,
		ACCOUNT_CHARACTER_SLOT_PLAYTIME,
		ACCOUNT_CHARACTER_SLOT_FORM,
		ACCOUNT_CHARACTER_SLOT_ADDR,
		ACCOUNT_CHARACTER_SLOT_PORT,
		ACCOUNT_CHARACTER_SLOT_GUILD_ID,
		ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
		ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
		ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		ACCOUNT_CHARACTER_SLOT_ACCE,
#endif
#ifdef ENABLE_AURA_SYSTEM
		ACCOUNT_CHARACTER_SLOT_AURA,
#endif
#ifdef ENABLE_YOHARA_SYSTEM
		ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL,
		ACCOUNT_CHARACTER_SLOT_SUNGMA_STR,
		ACCOUNT_CHARACTER_SLOT_SUNGMA_HP,
		ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE,
		ACCOUNT_CHARACTER_SLOT_SUNGMA_IMMUNE,
#endif
#ifdef NEW_SELECT_CHARACTER
		ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME,
#endif
	};

	enum
	{
		PHASE_WINDOW_LOGO,
		PHASE_WINDOW_LOGIN,
		PHASE_WINDOW_SELECT,
		PHASE_WINDOW_CREATE,
		PHASE_WINDOW_LOAD,
		PHASE_WINDOW_GAME,
		PHASE_WINDOW_EMPIRE,
		PHASE_WINDOW_NUM
	};

public:
	CPythonNetworkStream();
	~CPythonNetworkStream();
	CLASS_DELETE_COPYMOVE(CPythonNetworkStream);

	bool SendSpecial(int nLen, void * pvBuf);

	void StartGame();
	void Warp(LONG lGlobalX, LONG lGlobalY);

	void NotifyHack(const char * c_szMsg);
	void SetWaitFlag();

	void SendEmoticon(uint32_t eEmoticon);

	void ExitApplication();
	void ExitGame();
	void LogOutGame();
	void AbsoluteExitGame();
	void AbsoluteExitApplication() const;

	void EnableChatInsultFilter(bool isEnable);
	bool IsChatInsultIn(const char * c_szMsg);
	bool IsInsultIn(const char * c_szMsg);

	uint32_t GetGuildID() const;

	uint32_t UploadMark(const char * c_szImageFileName) const;
	uint32_t UploadSymbol(const char * c_szImageFileName) const;

	bool LoadInsultList(const char * c_szInsultListFileName);
	bool LoadConvertTable(uint32_t dwEmpireID, const char * c_szFileName);

	uint32_t GetAccountCharacterSlotDatau(uint32_t iSlot, uint32_t eType);
	const char * GetAccountCharacterSlotDataz(uint32_t iSlot, uint32_t eType);

	// SUPPORT_BGM
	const char * GetFieldMusicFileName() const;
	float GetFieldMusicVolume() const;
	// END_OF_SUPPORT_BGM

#ifdef NEW_SELECT_CHARACTER
	const char* GetLoginID();
#endif

	bool IsSelectedEmpire() const;

	void ToggleGameDebugInfo();

	void SetMarkServer(const char * c_szAddr, uint32_t uPort);
	void ConnectLoginServer(const char * c_szAddr, uint32_t uPort);
	void ConnectGameServer(uint32_t iChrSlot);

	void SetLoginInfo(const char * c_szID, const char * c_szPassword);
	void SetLoginKey(uint32_t dwLoginKey);
	void ClearLoginInfo(void);

	void SetHandler(PyObject * poHandler);
	void SetPhaseWindow(uint32_t ePhaseWnd, PyObject * poPhaseWnd);
	void ClearPhaseWindow(uint32_t ePhaseWnd, const PyObject * poPhaseWnd);
	void SetServerCommandParserWindow(PyObject * poWnd);

	bool SendSyncPositionElementPacket(uint32_t dwVictimVID, uint32_t dwVictimX, uint32_t dwVictimY);

	bool SendAttackPacket(uint32_t uMotAttack, uint32_t dwVIDVictim);
	bool SendCharacterStatePacket(const TPixelPosition & c_rkPPosDst, float fDstRot, uint32_t eFunc, uint32_t uArg);
	bool SendUseSkillPacket(uint32_t dwSkillIndex, uint32_t dwTargetVID = 0);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	bool SendSkillColorPacket(uint8_t bSkillSlot, uint32_t dwColor1, uint32_t dwColor2, uint32_t dwColor3, uint32_t dwColor4, uint32_t dwColor5);
#endif
	bool SendTargetPacket(uint32_t dwVID);

	bool SendCharacterPositionPacket(uint8_t iPosition);

	bool SendItemUsePacket(TItemPos pos);
	bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
	bool SendItemDropPacket(TItemPos pos, uint32_t elk);
	bool SendItemDropPacketNew(TItemPos pos, uint32_t elk, uint32_t count);
#ifdef ENABLE_DESTROY_SYSTEM
	bool SendItemDestroyPacket(TItemPos pos, uint32_t elk, uint32_t count);
#endif
	bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, uint8_t num);
	bool SendItemPickUpPacket(uint32_t vid);

#ifdef ENABLE_PARTY_MATCH
	bool PartyMatch(int index, uint8_t setting);
	bool RecvPartyMatch();
#endif

#ifdef ENABLE_INGAME_WIKI
	bool SendWikiRequestInfo(uint64_t retID, uint32_t vnum, uint8_t isMob);
#endif
	bool SendQuickSlotAddPacket(uint8_t wpos, uint8_t type, uint8_t pos);
	bool SendQuickSlotDelPacket(uint8_t pos);
	bool SendQuickSlotMovePacket(uint8_t pos, uint8_t change_pos);

	// PointReset 개 임시
	bool SendPointResetPacket();

	// Shop
	bool SendShopEndPacket();
#ifdef ENABLE_SPECIAL_INVENTORY
	bool SendShopBuyPacket(uint16_t bPos);
	bool SendShopSellPacket(uint16_t bySlot);
#else
	bool SendShopBuyPacket(uint8_t bPos);
	bool SendShopSellPacket(uint8_t bySlot);
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
	bool SendShopSellPacketNew(uint16_t wSlot, uint8_t byCount);
#else
	bool SendShopSellPacketNew(uint8_t wSlot, uint8_t byCount);
#endif

	// Exchange
	bool SendExchangeStartPacket(uint32_t vid);
	bool SendExchangeItemAddPacket(TItemPos ItemPos, uint8_t byDisplayPos);
#ifdef ENABLE_CHEQUE_SYSTEM
	bool SendExchangeElkAddPacket(uint32_t elk, uint32_t cheque);
#else
	bool SendExchangeElkAddPacket(uint32_t elk);
#endif
	bool SendExchangeItemDelPacket(uint8_t pos) const;
	bool SendExchangeAcceptPacket();
	bool SendExchangeExitPacket();

	// Quest
	bool SendScriptAnswerPacket(int iAnswer);
	bool SendScriptButtonPacket(unsigned int iIndex);
	bool SendAnswerMakeGuildPacket(const char * c_szName);
	bool SendQuestInputStringPacket(const char * c_szString);
#ifdef ENABLE_OX_RENEWAL
	bool SendQuestInputStringLongPacket(const char * c_szString);
#endif
	bool SendQuestConfirmPacket(uint8_t byAnswer, uint32_t dwPID);

	// Event
	bool SendOnClickPacket(uint32_t vid);

	// MINI GAME EVENTS //
#ifdef ENABLE_EVENT_MANAGER
	bool SendRequestEventQuest(const char* c_szName);
	bool SendRequestEventData(int iMonth);
	bool RecvEventInformation();
	bool RecvEventReload();
	bool RecvEventKWScore();
#endif
#ifdef ENABLE_MINI_GAME_OKEY_NORMAL
	bool SendMiniGameRumiStart(int safemode);
	bool SendMiniGameRumiExit();
	bool SendMiniGameRumiDeckCardClick();
	bool SendMiniGameRumiHandCardClick(int index);
	bool SendMiniGameRumiFieldCardClick(int index);
	bool SendMiniGameRumiDestroy(int index);
	bool RecvMiniGameOkeyCardPacket();
#endif
#ifdef ENABLE_MONSTER_BACK
	bool SendAttendanceGetReward();
	bool RecvAttendanceEvent();
	bool RecvAttendanceEventInfo();
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
	bool RecvHitCountInfo();
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
	bool SendMiniGameCatchKingStart(int ibetNumber);
	bool SendMiniGameCatchKingDeckCardClick();
	bool SendMiniGameCatchKingFieldCardClick(int icardNumber);
	bool SendMiniGameCatchKingReward();
	bool RecvMiniGameCatchKingPacket();
#endif
#ifdef ENABLE_METINSTONE_RAIN_EVENT
	bool RecvStoneEvent();
#endif
#ifdef ENABLE_FISH_EVENT
	bool SendFishBoxUse(uint8_t bWindow, uint16_t wCell);
	bool SendFishShapeAdd(uint8_t bPos);
	bool RecvFishEventInfo();
#endif
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
	bool SoulRoulette(const uint8_t option);
	bool RecvSoulRoulette();
#endif
#ifdef ENABLE_MINI_GAME_BNW
	bool SendMiniGameBNW(uint8_t bSubHeader, uint8_t bSubArgument);
	bool RecvMiniGameBNWPacket();
#endif
#ifdef ENABLE_MINI_GAME_FINDM
	bool SendMiniGameFindM(uint8_t bSubHeader, uint8_t bSubArgument);
	bool RecvMiniGameFindMPacket();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	bool SendMiniGameYutNori(uint8_t bSubHeader, uint8_t bSubArgument);
	bool RecvMiniGameYutNoriSetYut();
	bool RecvMiniGameYutNoriPacket();
#endif
#ifdef ENABLE_FLOWER_EVENT
	bool SendFlowerEventExchange(uint8_t id);
#endif
	// END MINI GAME EVENTS //

	// Fly
	bool SendFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition & kPPosTarget);
	bool SendAddFlyTargetingPacket(uint32_t dwTargetVID, const TPixelPosition & kPPosTarget);
	bool SendShootPacket(uint32_t uSkill);

	// Command
	bool ClientCommand(const char * c_szCommand) const;
	void ServerCommand(char * c_szCommand);

	// Emoticon
	void RegisterEmoticonString(const char * pcEmoticonString);

	// Party
	bool SendPartyInvitePacket(uint32_t dwVID);
	bool SendPartyInviteAnswerPacket(uint32_t dwLeaderVID, uint8_t byAnswer);
	bool SendPartyRemovePacket(uint32_t dwPID);
	bool SendPartySetStatePacket(uint32_t dwVID, uint8_t byState, uint8_t byFlag);
	bool SendPartyUseSkillPacket(uint8_t bySkillIndex, uint32_t dwVID);
	bool SendPartyParameterPacket(uint8_t byDistributeMode);

	// SafeBox
#ifdef ENABLE_SAFEBOX_MONEY
	bool SendSafeBoxMoneyPacket(uint8_t byState, uint32_t dwMoney);
#endif
	bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, uint8_t bySafeBoxPos);
	bool SendSafeBoxCheckoutPacket(uint8_t bySafeBoxPos, TItemPos InventoryPos);
	bool SendSafeBoxItemMovePacket(uint8_t bySourcePos, uint8_t byTargetPos, uint8_t byCount);

	// Mall
	bool SendMallCheckoutPacket(uint8_t byMallPos, TItemPos InventoryPos);

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	bool SendGuildstorageCheckinPacket(TItemPos InventoryPos, uint8_t byGuildstoragePos);
	bool SendGuildstorageCheckoutPacket(uint8_t byGuildstoragePos, TItemPos InventoryPos);
#endif

	// Guild
	bool SendGuildAddMemberPacket(uint32_t dwVID);
	bool SendGuildRemoveMemberPacket(uint32_t dwPID);
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
	bool SendGuildVoteChangeMaster(uint32_t dwPID);
	bool SendGuildVoteLandAbndon(int iVID);
#endif
	bool SendGuildChangeGradeNamePacket(uint8_t byGradeNumber, const char * c_szName);
	bool SendGuildChangeGradeAuthorityPacket(uint8_t byGradeNumber, uint8_t byAuthority);
	bool SendGuildOfferPacket(uint32_t dwExperience);
	bool SendGuildPostCommentPacket(const char * c_szMessage);
	bool SendGuildDeleteCommentPacket(uint32_t dwIndex);
	bool SendGuildRefreshCommentsPacket(uint32_t dwHighestIndex);
	bool SendGuildChangeMemberGradePacket(uint32_t dwPID, uint8_t byGrade);
	bool SendGuildUseSkillPacket(uint32_t dwSkillID, uint32_t dwTargetVID);
	bool SendGuildChangeMemberGeneralPacket(uint32_t dwPID, uint8_t byFlag);
	bool SendGuildInvitePacket(uint32_t dwVID);
	bool SendGuildInviteAnswerPacket(uint32_t dwGuildID, uint8_t byAnswer);
	bool SendGuildChargeGSPPacket(uint32_t dwMoney);
	bool SendGuildDepositMoneyPacket(uint32_t dwMoney);
	bool SendGuildWithdrawMoneyPacket(uint32_t dwMoney);

#ifdef ENABLE_MOVE_CHANNEL
	bool SendMoveChannelPacket(const uint8_t bChannelIndex);
#endif

	// Mall
	bool RecvMallOpenPacket();
	bool RecvMallItemSetPacket();
	bool RecvMallItemDelPacket();
#ifdef ENABLE_AUTO_SYSTEM
	void SendAutoCoolTime(int slotIndex, int iValue);
#endif

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	bool RecvGuildstorageOpenPacket();
	bool RecvGuildstorageItemSetPacket();
	bool RecvGuildstorageItemDelPacket();
#endif

#ifdef ENABLE_GUILDBANK_LOG
	void GuildBankInfoOpen();
	bool SendGuildBankInfoOpen();
#endif
#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
	void GuildDonateInfoOpen();
#endif

	// Lover
	bool RecvLoverInfoPacket();
	bool RecvLovePointUpdatePacket();

	// Dig
	bool RecvDigMotionPacket();

	// Fishing
	bool SendFishingPacket(int iRotation);
#ifdef ENABLE_FISHING_RENEWAL
	bool SendFishingPacketNew(int r, int i);
	bool RecvFishingNew();
#endif
	bool SendGiveItemPacket(uint32_t dwTargetVID, TItemPos ItemPos, int iItemCount);

	// Private Shop
	bool SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock);
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	// Private Shop Search
	bool SendPrivateShopSearchInfo(uint8_t bRace, uint8_t bMaskType, uint8_t bMaskSubType,
		uint8_t bMinRefine, uint8_t bMaxRefine, uint8_t bMinLevel, uint8_t bMaxLevel, int iMinGold, int iMaxGold, char* itemName
#	ifdef ENABLE_CHEQUE_SYSTEM
		, uint32_t dwMinCheque, uint32_t dwMaxCheque
#	endif	
	);
	void SendPrivateShopSearchInfoSub(int iPage);
	bool SendPrivateShopSearchBuyItem(uint32_t shopVid, uint8_t shopItemPos);
	bool SendPrivateShopSearchClose();

	bool RecvPrivateShopSearchSet();
	bool RecvPrivateShopSearchOpen();
#endif

	// Refine
	bool SendRefinePacket(uint16_t wPos, uint8_t byType);	//@fixme425
	bool SendSelectItemPacket(uint32_t dwItemPos);

	// Client Version
	bool SendClientVersionPacket();

	// CRC Report
	bool __SendCRCReportPacket() const;

	// 용홍석 강화
	bool SendDragonSoulRefinePacket(uint8_t bRefineType, const TItemPos * pos);

	// Handshake
	bool RecvHandshakePacket();
	bool RecvHandshakeOKPacket();

	bool RecvHybridCryptKeyPacket();
	bool RecvHybridCryptSDBPacket();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
	bool RecvKeyAgreementPacket();
	bool RecvKeyAgreementCompletedPacket();

#endif

#ifdef ENABLE_CUBE_RENEWAL
	bool SendCubeRefinePacket(int vnum, int multiplier, int indexImprove, int itemReq[5]);
	bool SendCubeRenewalClosePacket();
	bool RecvCubeRenewalPacket();
#endif

#ifdef ENABLE_TICKET_SYSTEM
	bool RecvTicketSystemPacket();
#endif

#ifdef ENABLE_BATTLE_FIELD
	bool RecvBattleZoneInfo();
#endif

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
	// Won Exchange
	bool SendWonExchangeSellPacket(uint16_t wValue);
	bool SendWonExchangeBuyPacket(uint16_t wValue);
#endif

	// ETC
	uint32_t GetMainActorVID() const;
	uint32_t GetMainActorRace() const;
	uint32_t GetMainActorEmpire() const;
	uint32_t GetMainActorSkillGroup() const;
	void SetEmpireID(uint32_t dwEmpireID);
	uint32_t GetEmpireID() const;
	void __TEST_SetSkillGroupFake(int iIndex);

#ifdef ENABLE_GROWTH_PET_SYSTEM
	bool SendPetWindowType(uint8_t pet_window_type);
	bool SendPetHatchingWindowPacket(bool bisOpenPetHatchingWindow);
	bool SendPetHatchingPacket(char* sGrowthPetName, uint8_t bPos);
	bool SendPetLearnSkillPacket(uint8_t bSkillBookSlotIndex, uint8_t bSkillBookInvenIndex);
	bool SendPetSkillUpgradePacket(uint8_t bslotPos, uint8_t bslotIndex);
	bool SendPetSkillUpgradeRequestPacket(uint8_t bslotPos, uint8_t bslotIndex);
	bool SendPetFeedPacket(uint8_t feedIndex, PyObject* poFeedItemsList, PyObject* poFeedItemsCount);
	bool SendPetDeleteSkill(uint8_t bSkillBookDelSlotIndex, uint8_t bSkillBookDelInvenIndex);
	bool SendPetDeleteAllSkill(uint8_t bPetSkillAllDelBookIndex);
	bool SendPetNameChange(char* sPetName, uint8_t bItemWindowType, uint16_t bItemWindowCell, uint8_t bPetWindowType, uint16_t bPetWindowCell);
#	ifdef ENABLE_PET_ATTR_DETERMINE
	bool SendPetAttrDetermine();
	bool SendChangePetPacket(uint8_t bSkillBookDebPetSlotIndexlSlotIndex, uint8_t bMaterialSlotIndex);
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	bool SendPetRevive(TItemPos upBringingPos, const std::vector<std::pair<uint16_t, uint16_t>>& itemVec);
#	endif
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	bool SendAcceClosePacket();
	bool SendAcceRefinePacket(uint8_t bAcceWindow, uint8_t bSlotAcce, uint8_t bSlotMaterial);
	bool RecvAcceRefinePacket();
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
	bool SendItemCombinationPacket(uint8_t bSlotMedium, uint8_t bSlotBase, uint8_t bSlotMaterial);
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	bool SendExtendInvenRequest(uint8_t bStepIndex, uint8_t bWindow);
	bool SendExtendInvenUpgrade(uint8_t bWindow);
# else
	bool SendExtendInvenRequest(uint8_t bStepIndex);
	bool SendExtendInvenUpgrade();
# endif

	bool RecvExtendInvenInfo();
	bool RecvExtendInvenResult();
#endif

#ifdef ENABLE_REFINE_ELEMENT
	bool 	RecvElementsSpellPacket();
	bool 	SendElementsSpellClose();
	bool 	SendElementsSpellAdd(int pos);
	bool 	SendElementsSpellChange(int pos, int type_select);
#endif

#ifdef ENABLE_ATTR_6TH_7TH
	bool SendSkillBookCombinationPacket(PyObject* cell, uint8_t bIndexComb);
	bool SendRegistFragmentPacket(uint8_t bRegistSlot);
	bool SendAttr67ClosePacket(uint8_t bIndexComb);
	bool SendAttr67AddPacket(uint8_t bRegistSlot, uint8_t bFragmentCount, uint16_t wCellAdditive, uint8_t bCountAdditive);
	bool RecvAttr67Packet();
#endif

#ifdef NEW_SELECT_CHARACTER
	bool RecvCharacterInformation();
#endif

	//////////////////////////////////////////////////////////////////////////
	// Phase 관련
	//////////////////////////////////////////////////////////////////////////
public:
	void SetOffLinePhase();
	void SetHandShakePhase();
	void SetLoginPhase();
	void SetSelectPhase();
	void SetLoadingPhase();
	void SetGamePhase();
	void ClosePhase() const;

	// Login Phase
	bool SendLoginPacket(const char * c_szName, const char * c_szPassword);
	bool SendLoginPacketNew(const char * c_szName, const char * c_szPassword);
	bool SendChinaMatrixCardPacket(const char * c_szMatrixCardString);
	bool SendRunupMatrixAnswerPacket(const char * c_szMatrixCardString);
	bool SendNEWCIBNPasspodAnswerPacket(const char * answer);

	bool SendEnterGame();

	// Select Phase
	bool SendSelectEmpirePacket(uint32_t dwEmpireID);
	bool SendSelectCharacterPacket(uint8_t Index);
	bool SendChangeNamePacket(uint8_t index, const char * name);
	bool SendCreateCharacterPacket(uint8_t index, const char * name, uint8_t job, uint8_t shape, uint8_t byCON, uint8_t byINT,
								   uint8_t bySTR, uint8_t byDEX);
	bool SendDestroyCharacterPacket(uint8_t index, const char * szPrivateCode);

	// Main Game Phase
	bool SendChatPacket(const char * c_szChat, uint8_t byType = CHAT_TYPE_TALKING);
	bool SendWhisperPacket(const char * name, const char * c_szChat);
	bool SendMessengerAddByVIDPacket(uint32_t vid);
	bool SendMessengerAddByNamePacket(const char * c_szName);
	bool SendMessengerRemovePacket(const char * c_szKey, const char * c_szName);
#ifdef ENABLE_MESSENGER_BLOCK
	bool SendMessengerBlockAddByVIDPacket(uint32_t vid);
	bool SendMessengerBlockAddByNamePacket(const char* c_szName);
	bool SendMessengerBlockRemovePacket(const char* c_szKey, const char* c_szName);
	bool SendMessengerBlockRemoveByVIDPacket(uint32_t vid);
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	bool SendGemShopBuy(uint8_t bPos);
	bool SendSlotAdd();
	bool SendRequestRefresh();
	bool SendSelectmetinstone(uint32_t dwItemPos);
#endif

#ifdef ENABLE_TICKET_SYSTEM
	bool SendTicketPacketOpen(int szAction, int szMode, const char* szTicketID);
	bool SendTicketPacketCreate(const char* szTitle, const char* szContent, int szPriority);
	bool SendTicketPacketReply(const char* szTicketID, const char* szReply);
	bool SendTicketPacketAdmin(int szAction, const char* szTicketID, const char* szName, const char* szReason);
	bool SendTicketPacketAdminChangePage(int iStartPage);
#endif

#ifdef ENABLE_MAILBOX
	bool RecvMailboxProcess();
	bool RecvMailbox();
	bool RecvMailboxAddData();
	bool RecvMailboxAll();
	bool RecvMailboxUnread();
	bool SendPostDelete(const uint8_t Index);
	bool SendPostGetItems(const uint8_t Index);
	bool SendPostWriteConfirm(const char* szName);
	bool SendPostWrite(const char* szName, const char* szTitle, const char* szMessage, const TItemPos& pos, const int iYang, const int iWon);
	bool SendMailBoxClose();
	bool SendPostAllDelete();
	bool SendPostAllGetItems();
	bool RequestPostAddData(const uint8_t ButtonIndex, const uint8_t DataIndex);
#endif

protected:
	bool OnProcess() override; // State들을 실제로 실행한다.
	void OffLinePhase();
	void HandShakePhase();
	void LoginPhase();
	void SelectPhase();
	void LoadingPhase();
	void GamePhase();

	bool __IsNotPing() const;

	void __DownloadMark() const;
	void __DownloadSymbol(const std::vector<uint32_t> & c_rkVec_dwGuildID) const;

	void __PlayInventoryItemUseSound(TItemPos uSlotPos) const;
	void __PlayInventoryItemDropSound(TItemPos uSlotPos) const;
	void __PlaySafeBoxItemDropSound(uint32_t uSlotPos) const;
	void __PlayMallItemDropSound(uint32_t uSlotPos) const;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void __PlayGuildstorageItemDropSound(uint32_t uSlotPos);
#endif

	bool __CanActMainInstance() const;

	enum REFRESH_WINDOW_TYPE
	{
		RefreshStatus = (1 << 0),
		RefreshAlignmentWindow = (1 << 1),
		RefreshCharacterWindow = (1 << 2),
		RefreshEquipmentWindow = (1 << 3),
		RefreshInventoryWindow = (1 << 4),
		RefreshExchangeWindow = (1 << 5),
		RefreshSkillWindow = (1 << 6),
		RefreshSafeboxWindow = (1 << 7),
		RefreshMessengerWindow = (1 << 8),
		RefreshGuildWindowInfoPage = (1 << 9),
		RefreshGuildWindowBoardPage = (1 << 10),
		RefreshGuildWindowMemberPage = (1 << 11),
		RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
		RefreshGuildWindowSkillPage = (1 << 13),
		RefreshGuildWindowGradePage = (1 << 14),
		RefreshTargetBoard = (1 << 15),
		RefreshMallWindow = (1 << 16),
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		RefreshGuildstorageWindow = (1 << 17),
#endif
	};

	void __RefreshStatus();
	void __RefreshAlignmentWindow();
	void __RefreshCharacterWindow();
	void __RefreshEquipmentWindow();
	void __RefreshInventoryWindow();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void __RefreshShopInfoWindow();
#endif
	void __RefreshExchangeWindow();
	void __RefreshSkillWindow();
	void __RefreshSafeboxWindow();
	void __RefreshMessengerWindow();
	void __RefreshGuildWindowInfoPage();
	void __RefreshGuildWindowBoardPage();
	void __RefreshGuildWindowMemberPage();
	void __RefreshGuildWindowMemberPageGradeComboBox();
	void __RefreshGuildWindowSkillPage();
	void __RefreshGuildWindowGradePage();
	void __RefreshTargetBoardByVID(uint32_t dwVID);
	void __RefreshTargetBoardByName(const char * c_szName);
	void __RefreshTargetBoard();
	void __RefreshMallWindow();
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	void __RefreshShopSearchWindow();
#endif
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	void __RefreshGuildstorageWindow();
#endif

	bool __SendHack(const char * c_szMsg);

protected:
	// Common
	bool RecvErrorPacket(int header);
	bool RecvPingPacket();
	bool RecvDefaultPacket(int header);
	bool RecvPhasePacket();

	// Login Phase
	bool __RecvLoginSuccessPacket4();
	bool __RecvLoginFailurePacket();
	bool __RecvEmpirePacket();

	// Select Phase
	bool __RecvPlayerCreateSuccessPacket();
	bool __RecvPlayerCreateFailurePacket();
	bool __RecvPlayerDestroySuccessPacket();
	bool __RecvPlayerDestroyFailurePacket();
	bool __RecvPlayerPoints();
	bool __RecvChangeName();

	// Loading Phase
	bool RecvMainCharacter();
	bool RecvMainCharacter3_BGM();
	bool RecvMainCharacter4_BGM_VOL();

	void __SetFieldMusicFileName(const char * musicName) const;
	void __SetFieldMusicFileInfo(const char * musicName, float vol) const;
	// END_OF_SUPPORT_BGM

	// Main Game Phase
	bool RecvWarpPacket();
	bool RecvPVPPacket();
	bool RecvDuelStartPacket();

	bool RecvCharacterAppendPacket();
	bool RecvCharacterAdditionalInfo();
	bool RecvCharacterUpdatePacket();
	bool RecvCharacterDeletePacket();
	bool RecvChatPacket();
	bool RecvOwnerShipPacket();
	bool RecvSyncPositionPacket();
	bool RecvWhisperPacket();
	bool RecvPointChange(); // Alarm to python

	bool RecvStunPacket();
	bool RecvDeadPacket();
	bool RecvCharacterMovePacket();

	bool RecvItemDelPacket(); // Alarm to python
	bool RecvItemSetPacket(); // Alarm to python
	bool RecvItemUsePacket(); // Alarm to python
	bool RecvItemUpdatePacket(); // Alarm to python
	bool RecvItemGroundAddPacket();
	bool RecvItemGroundDelPacket();
	bool RecvItemOwnership();

	bool RecvQuickSlotAddPacket(); // Alarm to python
	bool RecvQuickSlotDelPacket(); // Alarm to python
	bool RecvQuickSlotMovePacket(); // Alarm to python

	bool RecvCharacterPositionPacket();
	bool RecvMotionPacket();

	bool RecvShopPacket();
	bool RecvShopSignPacket();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	bool RecvShopItemData();
	bool RecvMyShopSignPacket();
	bool RecvShopStashSync();
	bool RecvShopOfflineTimeSync();
	bool RecvShopPositionSync();
#endif
	bool RecvExchangePacket();

	// Quest
	bool RecvScriptPacket();
	bool RecvQuestInfoPacket();
	bool RecvQuestConfirmPacket();
	bool RecvRequestMakeGuild();

	// Skill
	bool RecvSkillLevel();

	// Target
	bool RecvTargetPacket();
	bool RecvViewEquipPacket();
	bool RecvDamageInfoPacket();

#ifdef ENABLE_SEND_TARGET_INFO
	bool RecvTargetInfoPacket();

	public:
		bool SendTargetInfoLoadPacket(uint32_t dwVID);

	protected:
#endif

	// Fly
	bool RecvCreateFlyPacket();
	bool RecvFlyTargetingPacket();
	bool RecvAddFlyTargetingPacket();

	// Messenger
	bool RecvMessenger();

	// Guild
	bool RecvGuild();

	// Party
	bool RecvPartyInvite();
	bool RecvPartyAdd();
	bool RecvPartyUpdate();
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	bool RecvPartyPositionInfo();
#endif
	bool RecvPartyRemove();
	bool RecvPartyLink();
	bool RecvPartyUnlink();
	bool RecvPartyParameter();

	// SafeBox
	bool RecvSafeBoxSetPacket();
	bool RecvSafeBoxDelPacket();
	bool RecvSafeBoxWrongPasswordPacket();
	bool RecvSafeBoxSizePacket();
#ifdef ENABLE_SAFEBOX_MONEY
	bool RecvSafeBoxMoneyChangePacket();
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
	bool RecvGemShopOpen();
	bool RecvRefreshGemShopWIndow();
	bool RecvGemShopSlotBuy();
	bool RecvGemShopSlotAdd();
#endif

	// Fishing
	bool RecvFishing();

	// Dungeon
	bool RecvDungeon();

	// Time
	bool RecvTimePacket();

	// WalkMode
	bool RecvWalkModePacket();

	// ChangeSkillGroup
	bool RecvChangeSkillGroupPacket();

	// Refine
	bool RecvRefineInformationPacket();

	// Use Potion
	bool RecvSpecialEffect();
#ifdef ENABLE_12ZI
	bool RecvSpecialZodiacEffect();
#endif

	// 서버에서 지정한 이팩트 발동 패킷.
	bool RecvSpecificEffect();

	// 용혼석 관련
	bool RecvDragonSoulRefine();

#ifdef ENABLE_INGAME_WIKI
	bool RecvWikiPacket();
#endif

	// MiniMap Info
	bool RecvNPCList();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	bool RecvSHOPList();
#endif
	bool RecvLandPacket();
	bool RecvTargetCreatePacket();
	bool RecvTargetUpdatePacket();
	bool RecvTargetDeletePacket();

	// Affect
	bool RecvAffectAddPacket();
	bool RecvAffectRemovePacket();

	// Channel
	bool RecvChannelPacket();

#ifdef ENABLE_GROWTH_PET_SYSTEM
	bool RecvGrowthPetPacket();
	bool RecvGrowthPetInfoPacket();
	bool RecvGrowthPetHatching();
	bool RecvGrowthPetSkillUpgradeRequest();
	bool RecvGrowthPetFeedItemResult();
	bool RecvGrowthPetNameChangeResult();
#	ifdef ENABLE_PET_ATTR_DETERMINE
	bool RecvGrowthPetAttrDetermineResult();
	bool RecvGrowthPetAttrChangeResult();
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
	bool RecvGrowthPetReviveItemResult();
#	endif
#endif

#ifdef ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM
	bool RecvMeleyLairRankingTimeInfo();
	bool RecvMeleyLairRankingTimeResult();
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
	bool RecvItemSealPacket();
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
public:
	bool SendRemoveFromMyShop(int slot, TItemPos target);
#	ifdef ENABLE_CHEQUE_SYSTEM
	bool SendAddToMyShop(TItemPos from, int target, uint32_t price, uint32_t cheque);
#	else
	bool SendAddToMyShop(TItemPos from, int target, uint32_t price);
#	endif
	bool CloseMyShop();
	bool OpenMyShop();
#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_EXPIRE
	bool ReopenMyShop();
#	endif
	bool WithdrawMyShopMoney(uint32_t goldAmount, uint8_t chequeAmount);
	bool RenameMyShop(char* newName);

#ifdef ENABLE_MYSHOP_DECO
public:
	bool SendMyShopDecoState(uint8_t bState);
	bool SendMyShopDecoSet(uint8_t bType, uint32_t dwPolyVNum);
#endif
#endif

#ifdef ENABLE_DUNGEON_INFO_SYSTEM
public:
	bool SendDungeonInfo(uint8_t bAction, uint8_t bIndex = 0, uint8_t bRankType = 0);

protected:
	bool RecvDungeonInfo();
	bool RecvDungeonRanking();
#endif

protected:
	// 이모티콘
	bool ParseEmoticon(const char * pChatMsg, uint32_t * pdwEmoticon);

	// 파이썬으로 보내는 콜들
	void OnConnectFailure() override;
	void OnScriptEventStart(int iSkin, int iIndex);
	void HideQuestWindows();	//@fixme419

	void OnRemoteDisconnect() override;
	void OnDisconnect() override;

	void SetGameOnline();
	void SetGameOffline();
	BOOL IsGameOnline() const;

protected:
	bool CheckPacket(TPacketHeader * pRetHeader);

	void __InitializeGamePhase();
	void __InitializeMarkAuth();
	void __GlobalPositionToLocalPosition(LONG & rGlobalX, LONG & rGlobalY) const;
	void __LocalPositionToGlobalPosition(LONG & rLocalX, LONG & rLocalY) const;

	bool __IsPlayerAttacking() const;
	bool __IsEquipItemInSlot(TItemPos uSlotPos) const;

	void __ShowMapName(LONG lLocalX, LONG lLocalY);

	void __LeaveOfflinePhase() {}
	void __LeaveHandshakePhase() {}
	void __LeaveLoginPhase() {}
	void __LeaveSelectPhase() {}
	void __LeaveLoadingPhase() {}
	void __LeaveGamePhase();

	void __ClearNetworkActorManager() const;

	void __ClearSelectCharacterData();

	void __ConvertEmpireText(uint32_t dwEmpireID, char * szText);

	void __RecvCharacterAppendPacket(const SNetworkActorData * pkNetActorData);
	void __RecvCharacterUpdatePacket(const SNetworkUpdateActorData * pkNetUpdateActorData);

	void __FilterInsult(char * szLine, uint32_t uLineLen);

	void __SetGuildID(uint32_t id);

#ifdef ENABLE_BATTLE_PASS_SYSTEM
public:
	bool SendExtBattlePassAction(uint8_t bAction);
	bool SendExtBattlePassPremium(bool premium);

protected:
	bool RecvExtBattlePassOpenPacket();
	bool RecvExtBattlePassGeneralInfoPacket();
	bool RecvExtBattlePassMissionInfoPacket();
	bool RecvExtBattlePassMissionUpdatePacket();
	bool RecvExtBattlePassRankingPacket();
#endif

protected:
	TPacketGCHandshake m_HandshakeData;
	uint32_t m_dwChangingPhaseTime;
	uint32_t m_dwBindupRetryCount;
	uint32_t m_dwMainActorVID;
	uint32_t m_dwMainActorRace;
	uint32_t m_dwMainActorEmpire;
	uint32_t m_dwMainActorSkillGroup;
	BOOL m_isGameOnline;
	BOOL m_isStartGame;

	uint32_t m_dwGuildID;
	uint32_t m_dwEmpireID;

	struct SServerTimeSync
	{
		uint32_t m_dwChangeServerTime;
		uint32_t m_dwChangeClientTime;
	} m_kServerTimeSync;

	void __ServerTimeSync_Initialize();
	//uint32_t m_dwBaseServerTime;
	//uint32_t m_dwBaseClientTime;

	uint32_t m_dwLastGamePingTime;

	std::string m_stID;
	std::string m_stPassword;
	std::string m_strLastCommand;
	std::string m_strPhase;
	uint32_t m_dwLoginKey;
	BOOL m_isWaitLoginKey;

	std::string m_stMarkIP;

	CFuncObject<CPythonNetworkStream> m_phaseProcessFunc;
	CFuncObject<CPythonNetworkStream> m_phaseLeaveFunc;

	PyObject * m_poHandler;
	PyObject * m_apoPhaseWnd[PHASE_WINDOW_NUM];
	PyObject * m_poSerCommandParserWnd;
#ifdef ENABLE_FISHING_RENEWAL
	bool m_phaseWindowGame;
#endif

	TSimplePlayerInformation m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT];
	uint32_t m_adwGuildID[PLAYER_PER_ACCOUNT];
	std::string m_astrGuildName[PLAYER_PER_ACCOUNT];
	bool m_bSimplePlayerInfo;

	CRef<CNetworkActorManager> m_rokNetActorMgr;

	bool m_isRefreshStatus;
	bool m_isRefreshCharacterWnd;
	bool m_isRefreshEquipmentWnd;
	bool m_isRefreshInventoryWnd;
	bool m_isRefreshExchangeWnd;
	bool m_isRefreshSkillWnd;
	bool m_isRefreshSafeboxWnd;
	bool m_isRefreshMallWnd;
	bool m_isRefreshMessengerWnd;
	bool m_isRefreshGuildWndInfoPage;
	bool m_isRefreshGuildWndBoardPage;
	bool m_isRefreshGuildWndMemberPage;
	bool m_isRefreshGuildWndMemberPageGradeComboBox;
	bool m_isRefreshGuildWndSkillPage;
	bool m_isRefreshGuildWndGradePage;
#ifdef ENABLE_GUILDSTORAGE_SYSTEM
	bool m_isRefreshGuildstorageWnd;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	bool m_isRefreshShopInfoWnd;
#endif
#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	bool m_isRefreshShopSearchWnd;
#endif

	// Emoticon
	std::vector<std::string> m_EmoticonStringVector;

	struct STextConvertTable
	{
		char acUpper[26];
		char acLower[26];
		uint8_t aacHan[5000][2];
	} m_aTextConvTable[3];


	struct SMarkAuth
	{
		CNetworkAddress m_kNetAddr;
		uint32_t m_dwHandle;
		uint32_t m_dwRandomKey;
	} m_kMarkAuth;


	uint32_t m_dwSelectedCharacterIndex;

	CInsultChecker m_kInsultChecker;

	bool m_isEnableChatInsultFilter;
	bool m_bComboSkillFlag;

	std::deque<std::string> m_kQue_stHack;

private:
	struct SDirectEnterMode
	{
		bool m_isSet;
		uint32_t m_dwChrSlotIndex;
	} m_kDirectEnterMode;

#ifdef ENABLE_PARTY_MATCH
	struct PartyMatchInfo
	{
		int limit_level;
		std::vector<std::pair<int, int>> items;
	};
	std::unordered_map<int, std::shared_ptr<PartyMatchInfo>> m_PartyMatch;
#endif

	void __DirectEnterMode_Initialize();
	void __DirectEnterMode_Set(uint32_t uChrSlotIndex);
	bool __DirectEnterMode_IsSet() const;

public:
	uint32_t EXPORT_GetBettingGuildWarValue(const char * c_szValueName) const;
#ifdef ENABLE_PARTY_MATCH
	bool LoadPartyMatchInfo(const char* FileName);
	const decltype(m_PartyMatch)& GetPartyMatchInfo() const { return m_PartyMatch; };
#endif

private:
	struct SBettingGuildWar
	{
		uint32_t m_dwBettingMoney;
		uint32_t m_dwObserverCount;
	} m_kBettingGuildWar;

	CInstanceBase * m_pInstTarget;

	void __BettingGuildWar_Initialize();
	void __BettingGuildWar_SetObserverCount(uint32_t uObserverCount);
	void __BettingGuildWar_SetBettingMoney(uint32_t uBettingMoney);

#ifdef ENABLE_AURA_SYSTEM
protected:
	bool RecvAuraPacket();

public:
	bool SendAuraRefineCheckIn(TItemPos InventoryCell, TItemPos AuraCell, uint8_t byAuraRefineWindowType);
	bool SendAuraRefineCheckOut(TItemPos AuraCell, uint8_t byAuraRefineWindowType);
	bool SendAuraRefineAccept(uint8_t byAuraRefineWindowType);
	bool SendAuraRefineCancel();
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
protected:
	bool RecvBiologManager();

public:
	bool SendBiologManagerAction(uint8_t bSubHeader);
#endif

#ifdef ENABLE_HUNTING_SYSTEM
public:
	bool SendHuntingAction(uint8_t bAction, uint32_t dValue);

protected:
	bool RecvHuntingOpenWindowMain();
	bool RecvHuntingOpenWindowSelect();
	bool RecvHuntingOpenWindowReward();
	bool RecvHuntingUpdate();
	bool RecvHuntingRandomItems();
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
protected:
	bool RecvLottoOpenWindows();
	bool RecvLottoBasicInfo();
	bool RecvLottoTicketInfo();
	bool RecvLottoRankingJackpotInfo();
	bool RecvLottoRankingMoneyInfo();

public:
	bool SendOpenLottoWindow();
	bool SendOpenRankingWindow();
	bool SendLottoBuyTicket(int slot, int num1, int num2, int num3, int num4);
	bool SendLottoDeleteTicket(int slot);
	bool SendLottoReciveMoney(int slot);
	bool SendLottoPickMoney(long long amount);
#endif

#ifdef ENABLE_SWITCHBOT
public:
	bool RecvSwitchbotPacket();

	bool SendSwitchbotStartPacket(uint8_t slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives);
	bool SendSwitchbotStopPacket(uint8_t slot);
#endif

#ifdef ENABLE_LOADING_TIP
public:
	bool LoadLoadingTipList(const char* c_szLoadingTipListFileName);
	bool LoadLoadingTipVnum(const char* c_szLoadingTipVnumFileName);
	const char* GetLoadingTipVnum(long lMapIndex);

	void SetMapTarget(long lMapTarget) { m_lMapTarget = lMapTarget; };
	uint8_t GetMapTarget() { return m_lMapTarget; };

protected:
	std::map<uint32_t, std::vector<uint32_t>> m_kMap_dwMapIndex_vecTipList;
	std::map<uint32_t, std::string> m_kMap_dwID_strTipString;

	long m_lMapTarget;
#endif

#ifdef ENABLE_HWID_BAN
public:
	bool SendHwidBanPacket(int mode, const char* c_szPlayer, const char* c_szReason);
#endif

#ifdef ENABLE_CHANGED_ATTR
public:
	bool SendChagedItemAttributePacket(const bool bNew, const TItemPos& pos);
protected:
	bool RecvSelectAttr();
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
public:
	bool SendChangeLookCheckInPacket(const TItemPos& pos, const uint8_t bSlotType);
	bool SendChangeLookCheckOutPacket(const uint8_t bSlotType);
	bool SendChangeLookCheckInFreeYangItemPacket(const TItemPos& pos);
	bool SendChangeLookCheckOutFreeYangItemPacket();
	bool SendChangeLookAcceptPacket();
	bool SendChangeLookCancelPacket();
	bool RecvChangeLookPacket();
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
public:
	bool SendLanguageChangePacket(uint8_t bLanguage);
	bool SendTargetLanguageRequestPacket(const char* c_szName);

protected:
	bool RecvTargetLanguageResult();
#endif
};
