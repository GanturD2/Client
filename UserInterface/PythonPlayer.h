#pragma once

#include "AbstractPlayer.h"
#include "Packet.h"
#include "PythonSkill.h"

class CInstanceBase;

/*
 *	메인 캐릭터 (자신이 조정하는 캐릭터) 가 가진 정보들을 관리한다.
 *
 * 2003-01-12 Levites	본래는 CPythonCharacter가 가지고 있었지만 규모가 너무 커져 버린데다
 *						위치도 애매해서 따로 분리
 * 2003-07-19 Levites	메인 캐릭터의 이동 처리 CharacterInstance에서 떼어다 붙임
 *						기존의 데이타 보존의 역할에서 완벽한 메인 플레이어 제어 클래스로
 *						탈바꿈 함.
 */

enum
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};

class CPythonPlayer : public CSingleton<CPythonPlayer>, public IAbstractPlayer
{
public:
	enum
	{
		CATEGORY_NONE = 0,
		CATEGORY_ACTIVE = 1,
		CATEGORY_PASSIVE = 2,
		CATEGORY_MAX_NUM = 3,

		STATUS_INDEX_ST = 1,
		STATUS_INDEX_DX = 2,
		STATUS_INDEX_IQ = 3,
		STATUS_INDEX_HT = 4
	};

	enum
	{
		MBT_LEFT,
		MBT_RIGHT,
		MBT_MIDDLE,
		MBT_NUM
	};

	enum
	{
		MBF_SMART,
		MBF_MOVE,
		MBF_CAMERA,
		MBF_ATTACK,
		MBF_SKILL,
		MBF_AUTO
	};

	enum
	{
		MBS_CLICK,
		MBS_PRESS
	};

	enum EMode
	{
		MODE_NONE,
		MODE_CLICK_POSITION,
		MODE_CLICK_ITEM,
		MODE_CLICK_ACTOR,
		MODE_USE_SKILL
	};

	enum EEffect
	{
		EFFECT_PICK,
		EFFECT_NUM
	};

	enum EMetinSocketType
	{
		METIN_SOCKET_TYPE_NONE,
		METIN_SOCKET_TYPE_SILVER,
		METIN_SOCKET_TYPE_GOLD
	};

#ifdef ENABLE_PARTY_MATCH
	enum EPartyMatchSetting
	{
		PARTY_MATCH_REQUIRED_ITEM_MAX = 3
	};

	enum EPacketGCPartyMatchSubHeader
	{
		PARTY_MATCH_SEARCH,
		PARTY_MATCH_CANCEL,
	};

	enum EPartyMatchMsg
	{
		PARTY_MATCH_INFO,
		PARTY_MATCH_FAIL,
		PARTY_MATCH_SUCCESS,
		PARTY_MATCH_START,
		PARTY_MATCH_CANCEL_SUCCESS,
		PARTY_MATCH_FAIL_NO_ITEM,
		PARTY_MATCH_FAIL_LEVEL,
		PARTY_MATCH_FAIL_NOT_LEADER,
		PARTY_MATCH_FAIL_MEMBER_NOT_CONDITION,
		PARTY_MATCH_FAIL_NONE_MAP_INDEX,
		PARTY_MATCH_FAIL_IMPOSSIBLE_MAP,
		PARTY_MATCH_HOLD,
		PARTY_MATCH_FAIL_FULL_MEMBER,
	};
#endif

	typedef struct SSkillInstance
	{
		uint32_t dwIndex;
		int iType;
		int iGrade;
		int iLevel;
		float fcurEfficientPercentage;
		float fnextEfficientPercentage;

		float fCoolTime;
		float fLastUsedTime;
		BOOL bActive;
	} TSkillInstance;

	enum EKeyBoard_UD
	{
		KEYBOARD_UD_NONE,
		KEYBOARD_UD_UP,
		KEYBOARD_UD_DOWN
	};

	enum EKeyBoard_LR
	{
		KEYBOARD_LR_NONE,
		KEYBOARD_LR_LEFT,
		KEYBOARD_LR_RIGHT
	};

	enum
	{
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT
	};

	typedef struct SPlayerStatus
	{
		TItemData aInventoryItem[c_Inventory_Slot_Count]; // 180 = 4*45
		TItemData aEquipmentItem[c_Equipment_Slot_Count]; // 44 = 32 + 2*6
		TItemData aBeltInventoryItem[c_Belt_Inventory_Slot_Count]; // 16 = 4*4
		TItemData aDSItem[c_DragonSoul_Inventory_Count]; // 1152 = 6*6*32
#ifdef ENABLE_ATTR_6TH_7TH
		TItemData aNPCItem[1];
#endif
#ifdef ENABLE_SWITCHBOT
		TItemData aSwitchbotItem[SWITCHBOT_SLOT_COUNT];
#endif
#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
		TItemData aAdditionalEquipmentItem_1[ADDITIONAL_EQUIPMENT_MAX_SLOT];
#endif
		TQuickSlot aQuickSlot[QUICKSLOT_MAX_NUM];
		TSkillInstance aSkill[SKILL_MAX_NUM];
		long m_alPoint[POINT_MAX_NUM];
		long lQuickPageIndex;

		void SetPoint(uint32_t ePoint, long lPoint);
		long GetPoint(uint32_t ePoint);
#ifdef ENABLE_AUTO_SYSTEM
		TAutoSlot aAutoSlot[AUTO_POSITINO_SLOT_MAX];
#endif
	} TPlayerStatus;

	typedef struct SPartyMemberInfo
	{
		SPartyMemberInfo(uint32_t _dwPID, const char * c_szName) : dwVID(0), dwPID(_dwPID), strName(c_szName) {}

		uint32_t dwVID;
		uint32_t dwPID;
		std::string strName;
		uint8_t byState;
		uint8_t byHPPercentage;
		int16_t sAffects[PARTY_AFFECT_SLOT_MAX_NUM];
	} TPartyMemberInfo;

	enum EPartyRole
	{
		PARTY_ROLE_NORMAL,
		PARTY_ROLE_LEADER,
		PARTY_ROLE_ATTACKER,
		PARTY_ROLE_TANKER,
		PARTY_ROLE_BUFFER,
		PARTY_ROLE_SKILL_MASTER,
		PARTY_ROLE_BERSERKER,
		PARTY_ROLE_DEFENDER,
		PARTY_ROLE_MAX_NUM
	};

	enum
	{
		SKILL_NORMAL,
		SKILL_MASTER,
		SKILL_GRAND_MASTER,
		SKILL_PERFECT_MASTER
	};

	struct SAutoPotionInfo
	{
		SAutoPotionInfo() = default;

		bool bActivated{ false };
		long currentAmount{ 0 };
		long totalAmount{ 0 };
		long inventorySlotIndex;
	};

	enum EAutoPotionType
	{
		AUTO_POTION_TYPE_HP = 0,
		AUTO_POTION_TYPE_SP = 1,
#ifdef ENABLE_ANTI_EXP_RING
		AUTO_POTION_TYPE_EXP = 2,
#endif
		AUTO_POTION_TYPE_NUM
	};

public:
	CPythonPlayer();
	~CPythonPlayer();
	CLASS_DELETE_COPYMOVE(CPythonPlayer);

	void PickCloseMoney();
	void PickCloseItem();

	void SetGameWindow(PyObject* ppyObject);

	void SetObserverMode(bool isEnable) override;
	bool IsObserverMode() const;

	void SetQuickCameraMode(bool isEnable);

	void SetAttackKeyState(bool isPress);

	void NEW_GetMainActorPosition(TPixelPosition* pkPPosActor) const;

	bool RegisterEffect(uint32_t dwEID, const char* c_szFileName, bool isCache);

	bool NEW_SetMouseState(int eMBT, int eMBS);
	bool NEW_SetMouseFunc(int eMBT, int eMBF);
	int NEW_GetMouseFunc(int eMBT);
	void NEW_SetMouseMiddleButtonState(int eMBState);

	void NEW_SetAutoCameraRotationSpeed(float fRotSpd);
	void NEW_ResetCameraRotation() const;

	void NEW_SetSingleDirKeyState(int eDirKey, bool isPress);
	void NEW_SetSingleDIKKeyState(int eDIKKey, bool isPress);
	void NEW_SetMultiDirKeyState(bool isLeft, bool isRight, bool isUp, bool isDown);

	void NEW_Attack();
	void NEW_Fishing();
	bool NEW_CancelFishing();

	void NEW_LookAtFocusActor();
	bool NEW_IsAttackableDistanceFocusActor();


	bool NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
	bool NEW_MoveToMousePickedDirection();
	bool NEW_MoveToMouseScreenDirection();
	bool NEW_MoveToDirection(float fDirRot);
	void NEW_Stop();


	// Reserved
	bool NEW_IsEmptyReservedDelayTime(float fElapsedTime); // 네이밍 교정 논의 필요 - [levites]


	// Dungeon
	void SetDungeonDestinationPosition(int ix, int iy);
	void AlarmHaveToGo();


	CInstanceBase* NEW_FindActorPtr(uint32_t dwVID) const;
	CInstanceBase* NEW_GetMainActorPtr() override;

	// flying target set
	void Clear();
	void ClearSkillDict(); // 없어지거나 ClearGame 쪽으로 포함될 함수
	void NEW_ClearSkillData(bool bAll = false);

	void Update();

	// Play Time
	uint32_t GetPlayTime() const;
	void SetPlayTime(uint32_t dwPlayTime);

	// System
	void SetMainCharacterIndex(int iIndex) override;

	uint32_t GetMainCharacterIndex() override;
	bool IsMainCharacterIndex(uint32_t dwIndex) override;
	uint32_t GetGuildID();
	void NotifyDeletingCharacterInstance(uint32_t dwVID) override;
	void NotifyCharacterDead(uint32_t dwVID) override;
	void NotifyCharacterUpdate(uint32_t dwVID) override;
	void NotifyDeadMainCharacter();
	void NotifyChangePKMode() override;
#ifdef ENABLE_SKILL_COOLTIME_UPDATE
	void	ResetSkillCoolTimes();
#endif


	// Player Status
	const char* GetName() override;
	void SetName(const char* name);

	void SetRace(uint32_t dwRace) override;
	uint32_t GetRace() const;

	void SetWeaponPower(uint32_t dwMinPower, uint32_t dwMaxPower, uint32_t dwMinMagicPower, uint32_t dwMaxMagicPower,
		uint32_t dwAddPower) override;
	void SetStatus(uint32_t dwType, long lValue);
	int GetStatus(uint32_t dwType) override;

#ifdef ENABLE_SOULBIND_SYSTEM
	bool	SealItem(TItemPos Cell, long nSealDate);
	bool	UnSealItem(TItemPos Cell, long nSealDate);
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
protected:
	// need optimization (vector)
	uint32_t CombinationSlot[3];
public:
	uint8_t GetItemMagicPctBySlot(TItemPos Cell);

	void SetItemCombinationWindowActivedItemSlot(uint32_t iSlot, uint32_t iIndex);
	uint32_t GetConbWindowSlotByAttachedInvenSlot(uint32_t dwType);
#endif

	// Item
	void MoveItemData(TItemPos SrcCell, TItemPos DstCell);
	void SetItemData(TItemPos Cell, const TItemData& c_rkItemInst) override;
	const TItemData* GetItemData(TItemPos Cell) const;
	void SetItemCount(TItemPos Cell, uint8_t byCount) override;
#ifdef ENABLE_GIVE_BASIC_ITEM
	void SetBasicItem(TItemPos Cell, bool is_basic);
	bool IsBasicItem(TItemPos Cell);
#endif
#ifdef ENABLE_SET_ITEM
	void	SetItemSetValue(TItemPos Cell, uint8_t set_value);
	uint8_t	GetItemSetValue(TItemPos Cell);
#endif
	void SetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex, uint32_t dwMetinNumber) override;
	void SetItemAttribute(TItemPos Cell, uint32_t dwAttrIndex, uint16_t wType, int16_t sValue) override;	//@fixme436
#ifdef ENABLE_YOHARA_SYSTEM
	void SetSungmaAttribute(TItemPos Cell, uint32_t dwSungmaAttrIndex, uint16_t wType, int16_t sValue);	//@fixme436

	void SetRandomDefaultAttrs(TItemPos Cell, uint32_t dwRandomValueIndex, uint32_t dwRandomValueNumber);
	uint32_t GetRandomDefaultAttrs(TItemPos Cell, uint32_t dwRandomValueIndex) const;

	void GetItemApplyRandom(TItemPos Cell, uint32_t dwSungmaAttrIndex, uint16_t* pwType, int16_t* psValue);	//@fixme436
#endif
	uint32_t GetItemIndex(TItemPos Cell) override;
	uint32_t GetItemFlags(TItemPos Cell) override;
#ifdef ENABLE_SPECIAL_INVENTORY
	int GetSpecialInventoryTypeByGlobalSlot(int iSlot);
	std::tuple<int, int> GetSpecialInventoryRange(uint8_t invenType);
#endif
	uint32_t GetItemAntiFlags(TItemPos Cell);
	uint8_t GetItemTypeBySlot(TItemPos Cell);
	uint8_t GetItemSubTypeBySlot(TItemPos Cell);
	uint32_t GetItemCount(TItemPos Cell) override;
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
	uint32_t GetItemCountByVnum(uint32_t dwVnum, bool bIgnoreSetValue);
#else
	uint32_t GetItemCountByVnum(uint32_t dwVnum);
#endif
	uint32_t GetItemMetinSocket(TItemPos Cell, uint32_t dwMetinSocketIndex) const;
	void GetItemAttribute(TItemPos Cell, uint32_t dwAttrSlotIndex, uint16_t* pwType, int16_t* psValue) const;	//@fixme436
#ifdef ENABLE_CHANGED_ATTR
	void	GetItemChangedAttribute(uint32_t dwAttrSlotIndex, uint16_t* pbyType, int16_t* psValue);	//@fixme436
	void	SetSelectAttr(const TPlayerItemAttribute* attr);
#endif
	void SendClickItemPacket(uint32_t dwIID);

#ifdef ENABLE_REFINE_ELEMENT
	void 	SetElement(TItemPos Cell, uint8_t value_grade_element, uint8_t type_element);
	void 	SetElementAttack(TItemPos Cell, uint32_t attack_element_index, uint32_t attack_element);
	void 	SetElementValue(TItemPos Cell, uint32_t elements_value_bonus_index, int16_t elements_value_bonus);
	uint8_t 	GetElementGrade(TItemPos Cell);
	uint32_t 	GetElementAttack(TItemPos Cell, uint32_t attack_element_index);
	uint8_t 	GetElementType(TItemPos Cell);
	int16_t 	GetElementValue(TItemPos Cell, uint32_t elements_value_bonus_index);
#endif

	void RequestAddLocalQuickSlot(uint32_t dwLocalSlotIndex, uint32_t dwWndType, uint32_t dwWndItemPos) const;
	void RequestAddToEmptyLocalQuickSlot(uint32_t dwWndType, uint32_t dwWndItemPos);
	void RequestMoveGlobalQuickSlotToLocalQuickSlot(uint32_t dwGlobalSrcSlotIndex, uint32_t dwLocalDstSlotIndex) const;
	void RequestDeleteGlobalQuickSlot(uint32_t dwGlobalSlotIndex) const;
	void RequestUseLocalQuickSlot(uint32_t dwLocalSlotIndex);
	uint32_t LocalQuickSlotIndexToGlobalQuickSlotIndex(uint32_t dwLocalSlotIndex) const;

	void GetGlobalQuickSlotData(uint32_t dwGlobalSlotIndex, uint32_t* pdwWndType, uint32_t* pdwWndItemPos);
	void GetLocalQuickSlotData(uint32_t dwSlotPos, uint32_t* pdwWndType, uint32_t* pdwWndItemPos);
	void RemoveQuickSlotByValue(int iType, int iPosition) const;

	char IsItem(TItemPos Cell) const;

	//ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventorySlot(TItemPos Cell) const;
	//END_ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsInventorySlot(TItemPos Cell) const;
	bool IsEquipmentSlot(TItemPos Cell) const;
	bool IsEquipItemInSlot(TItemPos Cell) override;

	// Quickslot
	int GetQuickPage() const;
	void SetQuickPage(int nQuickPageIndex);
	void AddQuickSlot(int QuickSlotIndex, char IconType, char IconPosition) override;
	void DeleteQuickSlot(int QuickSlotIndex) override;
	void MoveQuickSlot(int Source, int Target) override;


	// Skill
	void SetSkill(uint32_t dwSlotIndex, uint32_t dwSkillIndex);
	bool GetSkillSlotIndex(uint32_t dwSkillIndex, uint32_t* pdwSlotIndex);
	int GetSkillIndex(uint32_t dwSlotIndex) const;
	int GetSkillGrade(uint32_t dwSlotIndex) const;
	int GetSkillLevel(uint32_t dwSlotIndex) const;
	float GetSkillCurrentEfficientPercentage(uint32_t dwSlotIndex) const;
	float GetSkillNextEfficientPercentage(uint32_t dwSlotIndex) const;
	void SetSkillLevel(uint32_t dwSlotIndex, uint32_t dwSkillLevel);
	void SetSkillLevel_(uint32_t dwSkillIndex, uint32_t dwSkillGrade, uint32_t dwSkillLevel);
	BOOL IsToggleSkill(uint32_t dwSlotIndex) const;
	void ClickSkillSlot(uint32_t dwSlotIndex);
	void ChangeCurrentSkillNumberOnly(uint32_t dwSlotIndex);
	bool FindSkillSlotIndexBySkillIndex(uint32_t dwSkillIndex, uint32_t* pdwSkillSlotIndex);

	float GetSkillCoolTime(uint32_t dwSlotIndex) const;
	float GetSkillElapsedCoolTime(uint32_t dwSlotIndex) const;
	BOOL IsSkillActive(uint32_t dwSlotIndex) const;
	BOOL IsSkillCoolTime(uint32_t dwSlotIndex);
	void UseGuildSkill(uint32_t dwSkillSlotIndex);
	bool AffectIndexToSkillSlotIndex(uint32_t uAffect, uint32_t* pdwSkillSlotIndex);
	bool AffectIndexToSkillIndex(uint32_t dwAffectIndex, uint32_t* pdwSkillIndex);

	void SetAffect(uint32_t uAffect);
	void ResetAffect(uint32_t uAffect);
	void ClearAffects() const;


	// Target
	void SetTarget(uint32_t dwVID, BOOL bForceChange = TRUE) override;
	void OpenCharacterMenu(uint32_t dwVictimActorID) const;
	uint32_t GetTargetVID() const;

	// Party
	void ExitParty();
	void AppendPartyMember(uint32_t dwPID, const char* c_szName);
	void LinkPartyMember(uint32_t dwPID, uint32_t dwVID);
	void UnlinkPartyMember(uint32_t dwPID);
	void UpdatePartyMemberInfo(uint32_t dwPID, uint8_t byState, uint8_t byHPPercentage);
	void UpdatePartyMemberAffect(uint32_t dwPID, uint8_t byAffectSlotIndex, int16_t sAffectNumber);
	void RemovePartyMember(uint32_t dwPID);
	bool IsPartyMemberByVID(uint32_t dwVID) override;
	bool IsPartyMemberByName(const char* c_szName);
	bool GetPartyMemberPtr(uint32_t dwPID, TPartyMemberInfo** ppPartyMemberInfo);
	bool PartyMemberPIDToVID(uint32_t dwPID, uint32_t* pdwVID);
	bool PartyMemberVIDToPID(uint32_t dwVID, uint32_t* pdwPID);
	bool IsSamePartyMember(uint32_t dwVID1, uint32_t dwVID2) override;


	// Fight
	void RememberChallengeInstance(uint32_t dwVID);
	void RememberRevengeInstance(uint32_t dwVID);
	void RememberCantFightInstance(uint32_t dwVID);
	void ForgetInstance(uint32_t dwVID);
	bool IsChallengeInstance(uint32_t dwVID);
	bool IsRevengeInstance(uint32_t dwVID);
	bool IsCantFightInstance(uint32_t dwVID);


	// Private Shop
	void OpenPrivateShop();
	void ClosePrivateShop();
	bool IsOpenPrivateShop() const;

#if defined(ENABLE_KEYCHANGE_SYSTEM)
	// Keyboard Controls
public:
	void OpenKeyChangeWindow();
	void IsOpenKeySettingWindow(BOOL bSet) { m_isOpenKeySettingWindow = bSet; };
	void KeySetting(int iKey, int iKeyFunction) { m_keySettingMap[iKey] = iKeyFunction; }
	void KeySettingClear() { m_keySettingMap.clear(); }
	void OnKeyDown(int iKey);
	void OnKeyUp(int iKey);
	using KeySettingMap = std::map<int, int>;
private:
	int m_iKeyBuffer;
	BOOL m_isOpenKeySettingWindow;
	KeySettingMap m_keySettingMap;
	bool m_bAutoRun;
public:
#endif

	// Stamina
	void StartStaminaConsume(uint32_t dwConsumePerSec, uint32_t dwCurrentStamina) override;
	void StopStaminaConsume(uint32_t dwCurrentStamina) override;


	// PK Mode
	uint32_t GetPKMode();


	// Mobile
	void SetMobileFlag(BOOL bFlag) override;
	BOOL HasMobilePhoneNumber() const;


	// Combo
	void SetComboSkillFlag(BOOL bFlag) override;


	// System
	void SetMovableGroundDistance(float fDistance);


	// Emotion
	void ActEmotion(uint32_t dwEmotionID);
	void StartEmotionProcess() override;
	void EndEmotionProcess() override;


#ifdef ENABLE_ACCE_COSTUME_SYSTEM
protected:
	bool m_isAcceRefineWindowOpen;
	int m_iAcceRefineWindowType;
	int m_iAcceActivedItemSlot[3];

public:
	void SetAcceRefineWindowOpen(bool isAcceRefineWindowOpen) { m_isAcceRefineWindowOpen = isAcceRefineWindowOpen; };
	bool GetAcceRefineWindowOpen() { return m_isAcceRefineWindowOpen; };

	void SetAcceRefineWindowType(int iAcceRefineWindowType) { m_iAcceRefineWindowType = iAcceRefineWindowType; };
	int GetAcceRefineWindowType() { return m_iAcceRefineWindowType; };

	void RefreshAcceWindow();
	void ClearAcceActivedItemSlot(int iAcceSlot);
	void SetAcceActivedItemSlot(int iAcceSlot, int iSlotIndex);
	int GetAcceActivedItemSlot(int iAcceSlot) { return m_iAcceActivedItemSlot[iAcceSlot]; };
#endif

#ifdef ENABLE_BATTLE_FIELD
protected:
	//Battle Field
	bool bBattleFieldIsButtonFlush;
	bool bBattleFieldIsOpen;
	bool bBattleFieldIsEnabled;
	bool bBattleFieldIsEventOpen;
	bool bBattleFieldIsEventEnable;

	int iBattleFieldStartTime;
	int iBattleFieldEndTime;

public:
	void SetBattleButtonFlush(bool bIsFlush);
	bool IsBattleButtonFlush() const;

	void SetBattleFieldOpen(bool bIsOpen);
	bool IsBattleFieldOpen() const;

	void SetBattleFieldEventOpen(bool bIsEventOpen);
	bool IsBattleFieldEventOpen() const;
	bool GetBattleFieldEventEnable() const;

	void SetBattleFieldInfo(bool bIsOpen);
	void SetBattleFieldEventInfo(bool bIsEventEnable, int iStartTime, int iEndTime);
	bool GetBattleFieldEnable() const;
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
public:
	void ClearGemShopItemVector() noexcept { m_GemItemsMap.clear(); }
	void SetGemShopItemData(uint8_t slotIndex, const TGemShopItem& rItemInstance);
	bool GetGemShopItemData(uint8_t slotIndex, const TGemShopItem** ppGemItemInfo);

	void SetGemShopRefreshTime(int refreshTime) noexcept { m_pRefreshTime = refreshTime; }
	int GetGemShopRefreshTime() noexcept { return m_pRefreshTime; }

	void SetOpenGemShopWindow(bool isOpen);
	bool IsOpenGemShopWindow();

	void SetGemShopOpenSlotCount(uint8_t count) noexcept { bGemShopOpenSlotCount = count; };
	uint8_t GetGemShopOpenSlotCount() noexcept { return bGemShopOpenSlotCount; };

	void SetGemShopOpenSlotItemCount(uint8_t slot, uint8_t count) noexcept { bGemShopOpenSlotItemCount[slot] = count; };
	uint8_t GetGemShopOpenSlotItemCount(uint8_t slot) noexcept { return bGemShopOpenSlotItemCount[slot]; };

	void SetGemShopSlotAdd(uint8_t slotIndex, uint8_t status);

protected:
	std::map<uint8_t, TGemShopItem> m_GemItemsMap;
	int m_pRefreshTime;
	bool m_isOpenGemShopWindow;

	uint8_t bGemShopOpenSlotCount;
	uint8_t bGemShopOpenSlotItemCount[GEM_SHOP_HOST_ITEM_MAX_NUM];
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	// Extend Inventory System
protected:
	uint8_t m_bExtendInvenStage;
	uint8_t m_bExtendInvenMax;
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t m_bExtendSpecialInvenStage[3];
	uint16_t m_bExtendSpecialInvenMax[3];
# endif

public:
	void SetExtendInvenStage(uint8_t bExtendInvenStage) noexcept { m_bExtendInvenStage = bExtendInvenStage; };
	uint8_t GetExtendInvenStage() noexcept { return m_bExtendInvenStage; };

	void SetExtendInvenMax(uint8_t bExtendInvenMax) noexcept { m_bExtendInvenMax = bExtendInvenMax; };
	uint8_t GetExtendInvenMax() noexcept { return m_bExtendInvenMax; };

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	void SetExtendSpecialInvenStage(uint8_t bExtendInvenStage, uint8_t bWIndow) noexcept { m_bExtendSpecialInvenStage[bWIndow] = bExtendInvenStage; };
	uint8_t GetExtendSpecialInvenStage(uint8_t bWIndow) noexcept { return m_bExtendSpecialInvenStage[bWIndow]; };

	void SetExtendSpecialInvenMax(uint16_t bExtendInvenMax, uint8_t bWIndow) noexcept { m_bExtendSpecialInvenMax[bWIndow] = bExtendInvenMax; };
	uint16_t GetExtendSpecialInvenMax(uint8_t bWIndow) noexcept { return m_bExtendSpecialInvenMax[bWIndow]; };
# endif
#endif

public:
	// Function Only For Console System
	BOOL __ToggleCoolTime();
	BOOL __ToggleLevelLimit();

	__inline const SAutoPotionInfo& GetAutoPotionInfo(int type) const { return m_kAutoPotionInfo[type]; }
	__inline SAutoPotionInfo& GetAutoPotionInfo(int type) { return m_kAutoPotionInfo[type]; }
	__inline void SetAutoPotionInfo(int type, const SAutoPotionInfo& info) { m_kAutoPotionInfo[type] = info; }

protected:
	TQuickSlot& __RefLocalQuickSlot(int SlotIndex);
	TQuickSlot& __RefGlobalQuickSlot(int SlotIndex);


	uint32_t __GetLevelAtk();
	uint32_t __GetStatAtk();
	uint32_t __GetWeaponAtk(uint32_t dwWeaponPower) const;
	uint32_t __GetTotalAtk(uint32_t dwWeaponPower, uint32_t dwRefineBonus);
	uint32_t __GetRaceStat();
	uint32_t __GetHitRate();
	uint32_t __GetEvadeRate();

	void __UpdateBattleStatus();

#ifdef ENABLE_REVERSED_FUNCTIONS
public:
#endif
	void __DeactivateSkillSlot(uint32_t dwSlotIndex);

protected:
	void __ActivateSkillSlot(uint32_t dwSlotIndex);

	void __OnPressSmart(CInstanceBase& rkInstMain, bool isAuto);
	void __OnClickSmart(CInstanceBase& rkInstMain, bool isAuto);

	void __OnPressItem(CInstanceBase& rkInstMain, uint32_t dwPickedItemID);
	void __OnPressActor(CInstanceBase& rkInstMain, uint32_t dwPickedActorID, bool isAuto);
	void __OnPressGround(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);
	void __OnPressScreen(CInstanceBase& rkInstMain);

	void __OnClickActor(CInstanceBase& rkInstMain, uint32_t dwPickedActorID, bool isAuto);
	void __OnClickItem(CInstanceBase& rkInstMain, uint32_t dwPickedItemID) const;
	void __OnClickGround(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);

	bool __IsMovableGroundDistance(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround) const;

	bool __GetPickedActorPtr(CInstanceBase** ppkInstPicked) const;

	bool __GetPickedActorID(uint32_t* pdwActorID) const;
	bool __GetPickedItemID(uint32_t* pdwItemID) const;
	bool __GetPickedGroundPos(TPixelPosition* pkPPosPicked) const;

	void __ClearReservedAction();
	void __ReserveClickItem(uint32_t dwItemID);
	void __ReserveClickActor(uint32_t dwActorID);
	void __ReserveClickGround(const TPixelPosition& c_rkPPosPickedGround);
	void __ReserveUseSkill(uint32_t dwActorID, uint32_t dwSkillSlotIndex, uint32_t dwRange);

	void __ReserveProcess_ClickActor();

	void __ShowPickedEffect(const TPixelPosition& c_rkPPosPickedGround);
	void __SendClickActorPacket(CInstanceBase& rkInstVictim);

	void __ClearAutoAttackTargetActorID();
	void __SetAutoAttackTargetActorID(uint32_t dwVID);

	void NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst);

	void NEW_SetMouseSmartState(int eMBS, bool isAuto);
	void NEW_SetMouseMoveState(int eMBS);
	void NEW_SetMouseCameraState(int eMBS);
	void NEW_GetMouseDirRotation(float fScrX, float fScrY, float* pfDirRot) const;
	void NEW_GetMultiKeyDirRotation(bool isLeft, bool isRight, bool isUp, bool isDown, float* pfDirRot) const;

	float GetDegreeFromDirection(int iUD, int iLR) const;
	float GetDegreeFromPosition(int ix, int iy, int iHalfWidth, int iHalfHeight) const;

	bool CheckCategory(int iCategory);
	bool CheckAbilitySlot(int iSlotIndex);

	void RefreshKeyWalkingDirection();
	void NEW_RefreshMouseWalkingDirection();


	// Instances
	void RefreshInstances();

	bool __CanShot(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget);
	bool __CanUseSkill();

	bool __CanMove();

	bool __CanAttack();
	bool __CanChangeTarget();

	bool __CheckSkillUsable(uint32_t dwSlotIndex);
	void __UseCurrentSkill();
	void __UseChargeSkill(uint32_t dwSkillSlotIndex);
	bool __UseSkill(uint32_t dwSlotIndex);
	bool __CheckSpecialSkill(uint32_t dwSkillIndex);

	bool __CheckRestSkillCoolTime(uint32_t dwSlotIndex) const;
	bool __CheckShortLife(const TSkillInstance& rkSkillInst, const CPythonSkill::TSkillData& rkSkillData);
	bool __CheckShortMana(const TSkillInstance& rkSkillInst, const CPythonSkill::TSkillData& rkSkillData);
	bool __CheckShortArrow(const TSkillInstance& rkSkillInst, const CPythonSkill::TSkillData& rkSkillData);
	bool __CheckDashAffect(const CInstanceBase& rkInstMain) const;

	void __SendUseSkill(uint32_t dwSkillSlotIndex, uint32_t dwTargetVID);
	void __RunCoolTime(uint32_t dwSkillSlotIndex);

	uint8_t __GetSkillType(uint32_t dwSkillSlotIndex);

	bool __IsReservedUseSkill(uint32_t dwSkillSlotIndex) const;
	bool __IsMeleeSkill(CPythonSkill::TSkillData& rkSkillData);
	bool __IsChargeSkill(CPythonSkill::TSkillData& rkSkillData);
	uint32_t __GetSkillTargetRange(const CPythonSkill::TSkillData& rkSkillData);
	bool __SearchNearTarget();
	bool __IsUsingChargeSkill();

	bool __ProcessEnemySkillTargetRange(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget, CPythonSkill::TSkillData& rkSkillData,
		uint32_t dwSkillSlotIndex);


	// Item
	bool __HasEnoughArrow();
	bool __HasItem(uint32_t dwItemID);
	uint32_t __GetPickableDistance();


	// Target
	CInstanceBase* __GetTargetActorPtr() const;
	void __ClearTarget();
	uint32_t __GetTargetVID() const;
	void __SetTargetVID(uint32_t dwVID);
	bool __IsSameTargetVID(uint32_t dwVID) const;
	bool __IsTarget() const;
	bool __ChangeTargetToPickedInstance();

	CInstanceBase* __GetSkillTargetInstancePtr(const CPythonSkill::TSkillData& rkSkillData) const;
	CInstanceBase* __GetAliveTargetInstancePtr() const;
	CInstanceBase* __GetDeadTargetInstancePtr() const;

	BOOL __IsRightButtonSkillMode();


	// Update
	void __Update_AutoAttack();
	void __Update_NotifyGuildAreaEvent();


	// Emotion
	BOOL __IsProcessingEmotion() const;

protected:
	PyObject* m_ppyGameWindow;

	// Client Player Data
	std::map<uint32_t, uint32_t> m_skillSlotDict;
	std::string m_stName;
	uint32_t m_dwMainCharacterIndex;
	uint32_t m_dwRace;
	uint32_t m_dwWeaponMinPower;
	uint32_t m_dwWeaponMaxPower;
	uint32_t m_dwWeaponMinMagicPower;
	uint32_t m_dwWeaponMaxMagicPower;
	uint32_t m_dwWeaponAddPower;

	// Todo
	uint32_t m_dwSendingTargetVID;
	float m_fTargetUpdateTime;

	// Attack
	uint32_t m_dwAutoAttackTargetVID;

	// NEW_Move
	EMode m_eReservedMode;
	float m_fReservedDelayTime;

	float m_fMovDirRot;

	bool m_isUp;
	bool m_isDown;
	bool m_isLeft;
	bool m_isRight;
	bool m_isAtkKey;
	bool m_isDirKey;
	bool m_isCmrRot;
	bool m_isSmtMov;
	bool m_isDirMov;

	float m_fCmrRotSpd;

	TPlayerStatus m_playerStatus;

	uint32_t m_iComboOld;
	uint32_t m_dwVIDReserved;
	uint32_t m_dwIIDReserved;

	uint32_t m_dwcurSkillSlotIndex;
	uint32_t m_dwSkillSlotIndexReserved;
	uint32_t m_dwSkillRangeReserved;

	TPixelPosition m_kPPosInstPrev;
	TPixelPosition m_kPPosReserved;

	// Emotion
	BOOL m_bisProcessingEmotion;

	// Dungeon
	BOOL m_isDestPosition;
	int m_ixDestPos;
	int m_iyDestPos;
	int m_iLastAlarmTime;

	// Party
	std::map<uint32_t, TPartyMemberInfo> m_PartyMemberMap;

	// PVP
	std::set<uint32_t> m_ChallengeInstanceSet;
	std::set<uint32_t> m_RevengeInstanceSet;
	std::set<uint32_t> m_CantFightInstanceSet;

	// Private Shop
	bool m_isOpenPrivateShop;

	// Observer mode
	bool m_isObserverMode;

	// Stamina
	BOOL m_isConsumingStamina;
	float m_fCurrentStamina;
	float m_fConsumeStaminaPerSec;

	// Guild
	uint32_t m_inGuildAreaID;

	// Mobile
	BOOL m_bMobileFlag;

	// System
	BOOL m_sysIsCoolTime;
	BOOL m_sysIsLevelLimit;

protected:
	// Game Cursor Data
	TPixelPosition m_MovingCursorPosition;
	float m_fMovingCursorSettingTime;
	uint32_t m_adwEffect[EFFECT_NUM];
#ifdef ENABLE_CHANGED_ATTR
	TPlayerItemAttribute	m_SelectAttrArr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#endif

	uint32_t m_dwVIDPicked;
	uint32_t m_dwIIDPicked;
	int m_aeMBFButton[MBT_NUM];

	uint32_t m_dwTargetVID;
	uint32_t m_dwTargetEndTime;
	uint32_t m_dwPlayTime;

	SAutoPotionInfo m_kAutoPotionInfo[AUTO_POTION_TYPE_NUM];

protected:
	float MOVABLE_GROUND_DISTANCE;

private:
	std::map<uint32_t, uint32_t> m_kMap_dwAffectIndexToSkillIndex;

#if defined(ENABLE_AUTO_SYSTEM) || defined(ENABLE_FLOWER_EVENT)
protected:
	typedef std::vector<TPacketAffectElement> TAffectDataVector;
	TAffectDataVector m_mapAffectData;

public:
	void AddAffect(uint32_t dwType, TPacketAffectElement kElem);
	void RemoveAffect(uint32_t dwType, uint16_t wApplyOn);
	int GetAffectDataIndex(uint32_t dwType, uint16_t wApplyOn);
	TPacketAffectElement GetAffectData(uint32_t dwType, uint16_t wApplyOn);
	int GetAffectDuration(uint32_t dwType);
#endif

#ifdef ENABLE_PENDANT
	uint32_t GetElementByVID(uint32_t dwVID);
#endif
#ifdef ENABLE_OFFICAL_FEATURES
	bool IsPoly();

protected:
	bool m_isOpenSafeBox; // Safebox
	bool m_isOpenMall; // Mall

public:
	// Safebox
	void SetOpenSafeBox(bool isOpen) noexcept { m_isOpenSafeBox = isOpen; };
	bool IsOpenSafeBox() const noexcept { return m_isOpenSafeBox; };

	// Mall
	void SetOpenMall(bool isOpen) noexcept { m_isOpenMall = isOpen; };
	bool IsOpenMall() const noexcept { return m_isOpenMall; };
#endif

#ifdef ENABLE_AURA_SYSTEM
public:
	enum EItemAuraSockets
	{
		ITEM_SOCKET_AURA_DRAIN_ITEM_VNUM,
		ITEM_SOCKET_AURA_CURRENT_LEVEL,
#	ifdef ENABLE_AURA_BOOST
		ITEM_SOCKET_AURA_BOOST,
#	endif
	};

	enum EItemAuraMaterialValues
	{
		ITEM_VALUE_AURA_MATERIAL_EXP,
	};

#	ifdef ENABLE_AURA_BOOST
	enum EItemAuraBoostValues
	{
		ITEM_VALUE_AURA_BOOST_PERCENT,
		ITEM_VALUE_AURA_BOOST_TIME,
		ITEM_VALUE_AURA_BOOST_UNLIMITED,
	};
#	endif

private:
	typedef struct SAuraRefineInfo
	{
		uint8_t bAuraRefineInfoLevel;
		uint8_t bAuraRefineInfoExpPercent;
	} TAuraRefineInfo;

	std::vector<TItemData> m_AuraItemInstanceVector;

protected:
	bool m_bAuraWindowOpen;
	uint8_t m_bOpenedAuraWindowType;
	TItemPos m_AuraRefineActivatedCell[AURA_SLOT_MAX];
	TAuraRefineInfo m_bAuraRefineInfo[AURA_REFINE_INFO_SLOT_MAX];

	void __ClearAuraRefineWindow();

public:
	void SetAuraRefineWindowOpen(uint8_t wndType);
	uint8_t GetAuraRefineWindowType() const noexcept { return m_bOpenedAuraWindowType; };

	bool IsAuraRefineWindowOpen() const noexcept { return m_bAuraWindowOpen; }
	bool IsAuraRefineWindowEmpty();

	void SetAuraRefineInfo(uint8_t bAuraRefineInfoSlot, uint8_t bAuraRefineInfoLevel, uint8_t bAuraRefineInfoExpPercent);
	uint8_t GetAuraRefineInfoLevel(uint8_t bAuraRefineInfoSlot);
	uint8_t GetAuraRefineInfoExpPct(uint8_t bAuraRefineInfoSlot);

	void SetAuraItemData(uint8_t bSlotIndex, const TItemData& rItemInstance);
	void DelAuraItemData(uint8_t bSlotIndex);

	uint8_t FineMoveAuraItemSlot();
	uint8_t GetAuraCurrentItemSlotCount();

	BOOL GetAuraItemDataPtr(uint8_t bSlotIndex, TItemData** ppInstance);

	void SetActivatedAuraSlot(uint8_t bSlotIndex, TItemPos ItemCell);
	uint8_t FindActivatedAuraSlot(TItemPos ItemCell);
	TItemPos FindUsingAuraSlot(uint8_t bSlotIndex);
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
public:
	bool	GetPickedActorID(uint32_t* pdwActorID);
	bool	GetPickedItemID(uint32_t* pdwItemID);
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
public:
	void SetShopSearchItemData(uint32_t real_position, TSearchItemData rItemData);
	TSearchItemData GetShopSearchItemData(uint32_t index);

	void ClearShopSearchData();

	void ShopSearchChangePage(int iPage);
	int GetShopSearchPage() const noexcept { return iShopSearchPage; }
	uint32_t GetShopSearchResultCount() noexcept { return m_ItemInstanceVector.size(); }
	uint32_t GetShopSearchResultMaxCount() noexcept { return m_ItemInstanceVector.size(); }
protected:
	std::unordered_map<uint32_t, TSearchItemData> m_ItemInstanceVector;
	int iShopSearchPage;
#endif

#ifdef ENABLE_MYSHOP_DECO
public:
	void SetMyShopDecoShow(bool status) noexcept { m_bMyShopDecoShow = status; }
	bool GetMyShopDecoOpen() const noexcept { return m_bMyShopDecoShow; }
protected:
	bool m_bMyShopDecoShow;
#endif

#ifdef ENABLE_CUBE_RENEWAL
protected:
	bool m_isOpenCubeRenewal;

public:
	// Safebox
	void SetOpenCubeRenewal(bool isOpen) { m_isOpenCubeRenewal = isOpen; };
	bool IsOpenCubeRenewal() const { return m_isOpenCubeRenewal; };
#endif

#ifdef ENABLE_AUTO_SYSTEM
public:
	TAutoSlot& AutoSlotData(int iSlotIndex);

	void SetAutoSlotCoolTime(int otoAvSlotIndex, uint32_t dSuresi);
	void GetAutoSlotIndex(uint32_t dwSlotPos, uint32_t* dwVnum, uint32_t* fillingTime);
	void SetLastAttackTime(uint32_t dwTime) { findTargetMs = dwTime; }

	void SetAutoSkillSlotIndex(int iSlotIndex, uint32_t dwIndex);
	void SetAutoPositionSlotIndex(int iSlotIndex, uint32_t dwIndex);

	int CheckSkillSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime);
	int CheckPositionSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime);

	bool AutoStatus() { return autoStatus; }
	void StartAuto(bool gelenDurum) { autoStart = gelenDurum; }
	bool CanStartAuto() { return autoStart; }

	void SetAutoPause(bool val) { autoPause = val; }
	bool GetAutoPause() { return autoPause; }

	TPixelPosition AutoHuntGetStartLocation() { return autohuntStartLocation; }

	void AutoHuntTimer(bool beceriSlot);
	void UseAutoSkill(uint32_t dwSlotIndex, long iTime, int slotIndex);
	void AutoReserveUse();/*0 red 1 blue*/
	bool AutoBarrierCheck(CInstanceBase* pTarget, CInstanceBase* pInstance);
	void AutoSlotControl(uint32_t slotIndex);

	void ClearAutoSKillSlot();
	void ClearAutoPositionSlot(int autoSlotIndex = AUTO_POSITINO_SLOT_MAX, bool manual = false);
	void ClearAutoAllSlot();

	void AutoStartOnOff(bool gelenDurum);
	void AutoAttackOnOff(bool gelenDurum) { autoAttackOnOff = gelenDurum; }
	bool GetAutoAttackOnOff() { return autoAttackOnOff; }

	void AutoSkillOnOff(bool gelenDurum) { autoSkillOnOff = gelenDurum; }
	bool GetAutoSkillOnOff() { return autoSkillOnOff; }

	void AutoPositionOnOff(bool gelenDurum) { autoPositionsOnOff = gelenDurum; }
	bool GetAutoPositionOnOff() { return autoPositionsOnOff; }

	void AutoRangeOnOff(bool gelenDurum) { autoRangeOnOff = gelenDurum; }
	bool GetAutoRangeOnOff() { return autoRangeOnOff; }

	void SetAutoRestart(bool gelenDurum) { AutoRestart = gelenDurum; }
	bool GetAutoRestart() { return AutoRestart; }

protected:
	void UpdateAuto();

	bool autoStatus;
	bool autoStart;
	bool autoPause;
	uint32_t findTargetMs;
	TPixelPosition autohuntStartLocation;

	long zLastSec;
	long kpLastMs;
	long mpLastMs;
	bool auto_HP;
	bool autp_MP;
	long affect_control;

	bool autoAttackOnOff;
	bool autoSkillOnOff;
	bool autoPositionsOnOff;
	bool autoRangeOnOff;
	bool AutoRestart;
#endif

#ifdef ENABLE_MONSTER_BACK
public:
	void 	SetRewardItem(uint32_t bDay, uint32_t dwVnum, uint32_t dwCount);
	std::vector<TRewardItem>& GetRewardVec() { return m_rewardItems; }

protected:
	std::vector<TRewardItem> m_rewardItems;
#endif
#ifdef ENABLE_ACCUMULATE_DAMAGE_DISPLAY
public:
	void		SetAccumulateDamage(uint32_t dwVid, uint32_t dwCount);
	uint32_t 	GetAccumulateDamage(uint32_t dwVid);

protected:
	std::vector<THitCountInfo> m_hitCount;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
public:
	void ClearChangeLook(const bool bClearEffect = false);

	void SetItemTransmutationVnum(TItemPos& Cell, const uint32_t blVnum);
	uint32_t GetItemTransmutationVnum(TItemPos Cell) const;

	void SetChangeLookWindowOpen(const bool blOpen);
	bool GetChangeLookWindowOpen() const;
	void SetChangeLookWindowType(const bool blType);
	bool GetChangeLookWindowType() const;

	void SetChangeLookItemData(TSubPacketGCChangeLookItemSet& data);
	TSubPacketGCChangeLookItemSet* GetChangeLookItemData(const uint8_t bSlotType);
	void DelChangeLookItemData(const uint8_t bSlotType, const bool bClearEffect = false);

	void SetChangeLookFreeItemData(TSubPacketGCChangeLookItemSet& data);
	TSubPacketGCChangeLookItemSet* GetChangeLookFreeItemData();
	void DelChangeLookFreeItemData(const bool bClearEffect = false);

protected:
	bool bIsChangeLookWindowOpen;
	bool bChangeLookWindowType;
	TSubPacketGCChangeLookItemSet m_ChangeLookSlot[static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX)];
	TSubPacketGCChangeLookItemSet m_ChangeLookFreeYangItemSlot;
#endif

#ifdef ENABLE_TAB_TARGETING
public:
	void SelectNearTarget();
#endif

#if defined(ENABLE_KEYCHANGE_SYSTEM) && defined(ENABLE_TAB_TARGETING)
public:
	void SetWarState(const bool bWarState) { m_bWarState = bWarState; };
	bool GetWarState() const { return m_bWarState; }
#endif
protected:
	bool m_bWarState;


#ifdef ENABLE_SECOND_GUILDRENEWAL_SYSTEM
protected:
	bool m_bParaState;

public:
	void SetParalysis(bool bParaState) { m_bParaState = bParaState; };
	bool GetParalysis() const { return m_bWarState; }
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
#ifdef ENABLE_PET_ATTR_DETERMINE
protected:
	uint32_t PetAttrChangeSlot[3];
#endif

protected:
	bool m_isOpenPetHatchingWindow;
	bool m_isOpenPetFeedWindow;

public:
	// Growth Pet System
	void SetOpenPetHatchingWindow(bool isOpen);
	bool IsOpenPetHatchingWindow();

	void SetOpenPetFeedWindow(bool isOpen);
	bool IsOpenPetFeedWindow();

#ifdef ENABLE_PET_ATTR_DETERMINE
	// Pet Attr Determine
	void SetItemAttrChangeWindowActivedItemSlot(uint32_t iSlot, uint32_t iIndex);
	uint32_t GetAttrChangeWindowSlotByAttachedInvenSlot(uint32_t dwType);
#endif

public:
	void SetPetInfo(TGrowthPetInfo pet_info);
	TGrowthPetInfo GetPetInfo(uint32_t pet_id);

	void SetActivePetItemID(uint32_t dwActivePetItemID) noexcept { m_dwActivePetItemID = dwActivePetItemID; };
	uint32_t GetActivePetItemID() const noexcept { return m_dwActivePetItemID; };

	void SetActivePetItemVnum(uint32_t dwActivePetItemVnum) noexcept { m_dwActivePetItemVnum = dwActivePetItemVnum; };
	uint32_t GetActivePetItemVnum() const noexcept { return m_dwActivePetItemVnum; };

protected:
	std::unordered_map<uint32_t, TGrowthPetInfo> m_GrowthPetInfo;
	uint32_t m_dwActivePetItemID;
	uint32_t m_dwActivePetItemVnum;
#endif
};

extern const int c_iFastestSendingCount;
extern const int c_iSlowestSendingCount;
extern const float c_fFastestSendingDelay;
extern const float c_fSlowestSendingDelay;
extern const float c_fRotatingStepTime;

extern const float c_fComboDistance;
extern const float c_fPickupDistance;
extern const float c_fClickDistance;
