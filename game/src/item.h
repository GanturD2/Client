#ifndef __INC_METIN_II_GAME_ITEM_H__
#define __INC_METIN_II_GAME_ITEM_H__

#include "entity.h"

class CItem : public CEntity
{
protected:
	// override methods from ENTITY class
	virtual void EncodeInsertPacket(LPENTITY entity);
	virtual void EncodeRemovePacket(LPENTITY entity);

public:
	CItem(uint32_t dwVnum);
	virtual ~CItem();

	int GetLevelLimit();

	bool CheckItemUseLevel(int nLevel);

	long FindApplyValue(uint16_t wApplyType); //@fixme532

	bool IsStackable() noexcept { return (GetFlag() & ITEM_FLAG_STACKABLE) ? true : false; }

	void Initialize();
	void Destroy();

	void Save();

	void SetWindow(uint8_t b) noexcept { m_bWindow = b; }
	uint8_t GetWindow() noexcept { return m_bWindow; }

	void SetID(uint32_t id) noexcept { m_dwID = id; }
	uint32_t GetID() noexcept { return m_dwID; }

	void SetProto(const TItemTable* table);
	TItemTable const* GetProto() noexcept { return m_pProto; }

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
	uint8_t GetLevel() const noexcept { return m_pProto ? (m_pProto->aLimits[0].bType == LIMIT_LEVEL ? static_cast<uint8_t>(m_pProto->aLimits[0].lValue) : 0) : 0; }
	std::string GetNameString() { return m_pProto ? std::string(m_pProto->szLocaleName) : ""; }
#endif

#ifdef ENABLE_PROTO_RENEWAL
	int GetShopBuyPrice();
	int GetShopSellPrice();
#else
	int GetGold();
	int GetShopBuyPrice();
#endif

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	const char* GetName();
#else
	const char* GetName() { return m_pProto ? m_pProto->szLocaleName : NULL; }
#endif

	const char* GetBaseName() noexcept { return m_pProto ? m_pProto->szName : nullptr; }
	uint8_t GetSize() noexcept { return m_pProto ? m_pProto->bSize : 0; }

	void SetFlag(long flag) noexcept { m_lFlag = flag; }
	long GetFlag() noexcept { return m_lFlag; }

	void AddFlag(long bit);
	void RemoveFlag(long bit);

	uint32_t GetWearFlag() noexcept { return m_pProto ? m_pProto->dwWearFlags : 0; }
	uint32_t GetAntiFlag() noexcept { return m_pProto ? m_pProto->dwAntiFlags : 0; }
	uint32_t GetImmuneFlag() noexcept { return m_pProto ? m_pProto->dwImmuneFlag : 0; }

	void SetVID(uint32_t vid) noexcept { m_dwVID = vid; }
	uint32_t GetVID() noexcept { return m_dwVID; }

	bool SetCount(uint32_t count);
	uint32_t GetCount();

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	void SetChangeLookVnum(uint32_t blVnum);
	uint32_t GetChangeLookVnum() const;
#endif
#ifdef ENABLE_GIVE_BASIC_ITEM
protected:
	bool is_basic;
public:
	bool IsBasicItem() const { return is_basic; }
	void SetBasic(bool b);
#endif

#ifdef ENABLE_SET_ITEM
protected:
	uint8_t set_value;

public:
	uint8_t GetItemSetValue() const { return set_value; }
	void SetItemSetValue(uint8_t iSet);
#endif

	/* Comments on GetVnum and GetOriginalVnum
	* GetVnum is Masked Vnum. By using this, the actual Vnum of the item is 10, but it can act as if the Vnum is 20.
	* Masking value is defined in ori_to_new.txt.
	* GetOriginalVnum is an item-specific Vnum, and this Vnum should be used when logging, sending item information to the client, and saving.
	*/
	uint32_t GetVnum() const noexcept { return m_dwMaskVnum ? m_dwMaskVnum : m_dwVnum; }
	uint32_t GetOriginalVnum() const noexcept { return m_dwVnum; }
	uint8_t GetType() const noexcept { return m_pProto ? m_pProto->bType : 0; }
	uint8_t GetSubType() const noexcept { return m_pProto ? m_pProto->bSubType : 0; }
#ifdef ENABLE_PROTO_RENEWAL
	uint8_t GetMaskType() const noexcept { return m_pProto ? m_pProto->bMaskType : 0; }
	uint8_t GetMaskSubType() const noexcept { return m_pProto ? m_pProto->bMaskSubType : 0; }
#endif
	uint8_t GetLimitType(uint32_t idx) const noexcept { return m_pProto ? m_pProto->aLimits[idx].bType : 0; }
	long GetLimitValue(uint32_t idx) const noexcept { return m_pProto ? m_pProto->aLimits[idx].lValue : 0; }

	long GetValue(uint32_t idx);

	uint8_t GetMagicPct() const noexcept { return m_pProto ? m_pProto->bAlterToMagicItemPct : 0; }

	void SetCell(LPCHARACTER ch, uint16_t pos) noexcept { m_pOwner = ch, m_wCell = pos; }
	uint16_t GetCell() noexcept { return m_wCell; }

#ifdef ENABLE_PROTO_RENEWAL
	uint16_t GetApplyType(int i) noexcept { return m_pProto ? m_pProto->aApplies[i].wType : 0; } //@fixme532
	long GetApplyValue(int i) noexcept { return m_pProto ? m_pProto->aApplies[i].lValue : 0; }
#endif

	LPITEM RemoveFromCharacter();
	bool AddToCharacter(LPCHARACTER ch, TItemPos Cell);
	LPCHARACTER GetOwner() noexcept { return m_pOwner; }

	LPITEM RemoveFromGround();
	bool AddToGround(long lMapIndex, const PIXEL_POSITION& pos, bool skipOwnerCheck = false);

	int FindEquipCell(LPCHARACTER ch, int bCandidateCell = -1);
	bool IsEquipped() const noexcept { return m_bEquipped; }
	bool EquipTo(LPCHARACTER ch, uint8_t bWearCell);
	bool IsEquipable() const;

	bool CanUsedBy(LPCHARACTER ch);

	bool DistanceValid(LPCHARACTER ch);

	void UpdatePacket();
	void UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use* packet);

	void SetExchanging(bool isOn = true);
	bool IsExchanging() noexcept { return m_bExchanging; }

	bool IsTwohanded();

	bool IsPolymorphItem();

#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	long FirstAttribute(long attrVal, int i);
#endif

	void ModifyPoints(bool bAdd);

	bool CreateSocket(uint8_t bSlot, uint8_t bGold);
	const long* GetSockets() noexcept { return &m_alSockets[0]; }
	long GetSocket(int i) noexcept { return m_alSockets[i]; }

	void SetSockets(const long* al);
	void SetSocket(int i, long v, bool bLog = true);

	int GetSocketCount();
	bool AddSocket();

	const TPlayerItemAttribute* GetAttributes() noexcept { return m_aAttr; }
	const TPlayerItemAttribute& GetAttribute(int i) noexcept { return m_aAttr[i]; }

	uint16_t GetAttributeType(int i) noexcept { return m_aAttr[i].wType; } //@fixme532
	int16_t GetAttributeValue(int i) noexcept { return m_aAttr[i].sValue; }

#ifdef ENABLE_YOHARA_SYSTEM
	// Random Attr
	const TPlayerItemApplyRandom* GetApplysRandom() { return m_aApplyRandom; }
	const TPlayerItemApplyRandom& GetApplyRandom(int i) { return m_aApplyRandom[i]; }

	uint16_t GetApplyRandomType(int i) { return m_aApplyRandom[i].wType; } //@fixme532
	int16_t GetApplyRandomValue(int i) { return m_aApplyRandom[i].sValue; }
	int16_t GetApplyRandomRow(int i) { return m_aApplyRandom[i].row; }

	uint16_t GetSungmaUniqueRandomSocket();
	bool GetApplyRandomData(uint32_t vnum);
	bool ItemHasApplyRandom();

	// Random Atk
	void SetRandomDefaultAttrs(const uint16_t* c_al);
	void SetRandomDefaultAttr(int i, uint16_t v);

	const uint16_t* GetRandomDefaultAttrs() { return &m_alRandomValues[0]; }
	uint16_t GetRandomDefaultAttr(int i) { return m_alRandomValues[i]; }

	bool ItemHasRandomDefaultAttr();

	// Defaultfunctions
	int GetRandomAttrCount();
	void ClearRandomAttribute();
	void SetRandomAttrs(const TPlayerItemApplyRandom* c_pAttribute);
	void CopyApplyRandomTo(LPITEM pItem);

	// Refine
	void SetItemApplyRandom(int i, uint16_t wType, int16_t sValue, int8_t bRow); //@fixme532
	void PutRandomAttributeWithLevel(uint16_t bLevel); //@fixme532
	void AddRandomAttr(uint16_t wApply, uint16_t bLevel); //@fixme532
	bool HasRandomAttr(uint16_t wApply);
#endif

	void SetAttributes(const TPlayerItemAttribute* c_pAttribute);

	int FindAttribute(uint16_t wType); //@fixme532
	bool RemoveAttributeAt(int index);
	bool RemoveAttributeType(uint16_t wType); //@fixme532

	bool HasAttr(uint16_t wApply); //@fixme532
	bool HasApply(uint16_t wApply); //@fixme532
	bool HasRareAttr(uint16_t wApply); //@fixme532

	void SetDestroyEvent(LPEVENT pkEvent);
	void StartDestroyEvent(int iSec = ITEM_FLOOR_TIME);

	uint32_t GetRefinedVnum() noexcept { return m_pProto ? m_pProto->dwRefinedVnum : 0; }
	uint32_t GetRefineFromVnum();
	int GetRefineLevel();
#ifdef ENABLE_ANNOUNCEMENT_REFINE_SUCCES
	int GetItemGrade();
#endif

	void SetSkipSave(bool b) noexcept { m_bSkipSave = b; }
	bool GetSkipSave() noexcept { return m_bSkipSave; }

	bool IsOwnership(LPCHARACTER ch);
	void SetOwnership(LPCHARACTER ch, int iSec = 10);
	void SetOwnershipEvent(LPEVENT pkEvent);

	void SetLastOwnerPID(uint32_t pid) noexcept { m_dwLastOwnerPID = pid; }
	uint32_t GetLastOwnerPID() noexcept { return m_dwLastOwnerPID; }

	int GetAttributeSetIndex(); // Returns which index of the specified array to be attached to is used.
	void AlterToMagicItem();
	void AlterToSocketItem(int iSocketCount);

	uint16_t GetRefineSet() noexcept { return m_pProto ? m_pProto->wRefineSet : 0; }

	void StartUniqueExpireEvent();
	void SetUniqueExpireEvent(LPEVENT pkEvent);

	void StartTimerBasedOnWearExpireEvent();
	void SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent);

#ifdef ENABLE_SOUL_SYSTEM
	void StartSoulTimeUseEvent();
	void SetSoulKeepTime();
#endif

	void StartRealTimeExpireEvent();
	bool IsRealTimeItem();

	void StopUniqueExpireEvent();
	void StopTimerBasedOnWearExpireEvent();
	void StopAccessorySocketExpireEvent();

	// For now, it only works properly for REAL_TIME and TIMER_BASED_ON_WEAR items.
	int GetDuration();

	int GetAttributeCount();
	void ClearAttribute();
	void ClearAllAttribute();
	void ChangeAttribute(const int* aiChangeProb = nullptr);
	void ChangeAttributeValue();
	void AddAttribute();
	void AddAttribute(uint16_t wType, int16_t sValue); //@fixme532
#ifdef ENABLE_CHANGED_ATTR
	void GetSelectAttr(TPlayerItemAttribute(&arr)[ITEM_ATTRIBUTE_MAX_NUM]);
#endif

	void ApplyAddon(int iAddonType);

	int GetSpecialGroup() const;
	bool IsSameSpecialGroup(const LPITEM item) const;

	// ACCESSORY_REFINE
	// Add sockets via mines to accessories
	bool IsAccessoryForSocket();

	int GetAccessorySocketGrade();
	int GetAccessorySocketMaxGrade();
	int GetAccessorySocketDownGradeTime();

	void SetAccessorySocketGrade(int iGrade);
	void SetAccessorySocketMaxGrade(int iMaxGrade);
	void SetAccessorySocketDownGradeTime(uint32_t time);

	void AccessorySocketDegrade();

	// Timer running when an accessory is out of an item (copper, etc.)
	void StartAccessorySocketExpireEvent();
	void SetAccessorySocketExpireEvent(LPEVENT pkEvent);

	bool CanPutInto(LPITEM item);
	// END_OF_ACCESSORY_REFINE

	void CopyAttributeTo(LPITEM pItem);
	void CopySocketTo(LPITEM pItem);

	int GetRareAttrCount();
	bool AddRareAttribute();
	bool ChangeRareAttribute();

	void AttrLog();

	void Lock(bool f) noexcept { m_isLocked = f; }
	bool isLocked() const noexcept { return m_isLocked; }

#ifdef ENABLE_REFINE_ELEMENT
	void SetElement(uint8_t value_grade_element, const uint32_t* attack_element, uint8_t element_type_bonus, const int16_t* elements_value_bonus);
	void SetElementNew(uint8_t value_grade_element, uint32_t attack_element, uint8_t element_type_bonus, int16_t elements_value_bonus);
	uint8_t GetElementGrade() noexcept { return m_grade_element; }
	void DeleteAllElement(uint8_t value_grade_element);
	void ChangeElement(uint8_t element_type_bonus);

	const uint32_t* GetElementAttacks() noexcept { return &m_attack_element[0]; }
	uint32_t GetElementAttack(int index) noexcept { return m_attack_element[index]; }

	uint8_t GetElementsType() noexcept { return m_element_type_bonus; }
	const int16_t* GetElementsValues() noexcept { return &m_elements_value_bonus[0]; }
	int16_t GetElementsValue(int index) noexcept { return m_elements_value_bonus[index]; }
#endif

public:
	void SetAttribute(int i, uint16_t wType, int16_t sValue); //@fixme532
	void SetForceAttribute(int i, uint16_t wType, int16_t sValue); //@fixme532

protected:
	bool EquipEx(bool is_equip);
	bool Unequip();

	void AddAttr(uint16_t wApply, uint8_t bLevel); //@fixme532
	void PutAttributeWithLevel(uint8_t bLevel); //@fixme532
	void PutAttribute(const int* aiAttrPercentTable);

public:
	void AddRareAttribute2(const int* aiAttrPercentTable = nullptr);
protected:
	void AddRareAttr(uint16_t wApply, uint8_t bLevel); //@fixme532
	void PutRareAttribute(const int* aiAttrPercentTable);
	void PutRareAttributeWithLevel(uint8_t bLevel); //@fixme532

protected:
	friend class CInputDB;
	bool OnAfterCreatedItem(); // A function called after an item has been completely created (loaded) with all information on the server.

public:
	bool IsRideItem();
	bool IsRamadanRing();

#ifdef ENABLE_MOUNT_PROTO_AFFECT_SYSTEM
	uint32_t GetMountVnum();
#else
	void ClearMountAttributeAndAffect();
	bool IsNewMountItem();
#endif

	/*
	* It is the same as the existing cash item in Germany, but because it makes an exchangeable cash item,
	* To the original item, new items with only the exchange prohibition flag deleted were allocated to the new item band.
	* The problem is that the new item has to have the same effect as the original item,
	* It is based on server, client, and vnum
	* We ran into an unfortunate situation where we had to put all new vnums on the server.
	* So, if it is an item of the new vnum, when running on the server, it rotates by changing it to the original item vnum,
	* When saving, change it to the original vnum.
	*/

	// Mask vnum is for items whose vnum is changed for some reason (ex. the above situation).
	void SetMaskVnum(uint32_t vnum) noexcept { m_dwMaskVnum = vnum; }
	uint32_t GetMaskVnum() noexcept { return m_dwMaskVnum; }
	bool IsMaskedItem() noexcept { return m_dwMaskVnum != 0; }

	// Dragon Soul Stone
	bool IsDragonSoul();
	int GiveMoreTime_Per(float fPercent);
	int GiveMoreTime_Fix(uint32_t dwTime);

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
	void RefreshAdditionalEquipmentItems(LPCHARACTER ch, uint8_t cell, bool add);
	bool EquipToDB(LPCHARACTER ch, uint8_t bWearCell);
#endif

#ifdef ENABLE_SPECIAL_INVENTORY
	int GetSpecialInventoryType();
#endif

#ifdef ENABLE_SEALBIND_SYSTEM
	long m_sealDate;
	long GetSealDate() noexcept { return m_sealDate; }

	void SetSealDate(long sealDate) noexcept { (-1 != sealDate && time(0) > sealDate) ? m_sealDate = 0 : m_sealDate = sealDate; }
	bool IsSealed();
	bool IsUnlimitedSealDate() noexcept { return m_sealDate == -1; }
	bool IsSealAble() const;
#endif

	bool IsSocketModifyingItem() {
		return GetType() == ITEM_USE && (GetSubType() == USE_PUT_INTO_BELT_SOCKET
			|| GetSubType() == USE_PUT_INTO_RING_SOCKET
			|| GetSubType() == USE_PUT_INTO_ACCESSORY_SOCKET
			|| GetSubType() == USE_ADD_ACCESSORY_SOCKET
			|| GetSubType() == USE_CLEAN_SOCKET);
	}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	long DrainedValue(long value);
	float GetDrainPercentage();
	TItemTable* GetAbsorbedItem();
#endif

	// Weapon
	bool IsWeapon() { return GetType() == ITEM_WEAPON; }
	bool IsSword() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_SWORD; }
	bool IsDagger() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_DAGGER; }
	bool IsBow() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BOW; }
	bool IsTwoHandSword() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_TWO_HANDED; }
	bool IsBell() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BELL; }
	bool IsFan() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_FAN; }
	bool IsArrow() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_ARROW; }
	bool IsMountSpear() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_MOUNT_SPEAR; }
	bool IsClaw() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_CLAW; }
#ifdef ENABLE_QUIVER_SYSTEM
	bool IsQuiver() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_QUIVER; }
#endif
	bool IsBouquet() { return GetType() == ITEM_WEAPON && GetSubType() == WEAPON_BOUQUET; }

	// Armor
	bool IsArmor() { return GetType() == ITEM_ARMOR; }
	bool IsArmorBody() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_BODY; }
	bool IsHelmet() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_HEAD; }
	bool IsShield() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_SHIELD; }
	bool IsWrist() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_WRIST; }
	bool IsShoe() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_FOOTS; }
	bool IsNecklace() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_NECK; }
	bool IsEarRing() { return GetType() == ITEM_ARMOR && GetSubType() == ARMOR_EAR; }

	bool IsEquipment() { return GetType() == ITEM_WEAPON || GetType() == ITEM_ARMOR; }
	bool IsRing() { return GetType() == ITEM_RING; }
#ifdef ENABLE_PET_SYSTEM
	bool IsPetItem() { return GetType() == ITEM_PET && GetSubType() == PET_PAY; }
#else
	bool IsPetItem() { return GetVnum() <= 53001 && GetVnum() <= 53021; }
#endif
#if defined (ENABLE_CHANGE_LOOK_SYSTEM) && defined(ENABLE_CHANGE_LOOK_MOUNT)
	bool IsHorseSummonItem() { return GetVnum() >= 50051 && GetVnum() <= 50053; }
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	bool IsPetUpbringing() { return GetType() == ITEM_PET && GetSubType() == PET_UPBRINGING; }
	bool IsPetBag() { return GetType() == ITEM_PET && GetSubType() == PET_BAG; }
#endif
	bool IsCostume() { return GetType() == ITEM_COSTUME; }
	bool IsCostumeMount() { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_MOUNT; }
	bool IsCostumeHair() { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_HAIR; }
	bool IsCostumeBody() { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_BODY; }
	bool IsCostumeAcce() { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_ACCE; }
	bool IsCostumeAura() { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_AURA; }
	bool IsCostumeWeapon() { return GetType() == ITEM_COSTUME && GetSubType() == COSTUME_WEAPON; }
	bool IsOldHair() { return GetVnum() >= 74001 && GetVnum() <= 75620; }
	bool IsCostumeModifyItem() { return GetType() == ITEM_USE && (GetSubType() == USE_CHANGE_COSTUME_ATTR || GetSubType() == USE_RESET_COSTUME_ATTR); }
	bool IsBelt() { return GetType() == ITEM_BELT; }
#ifdef ENABLE_PASSIVE_SYSTEM
	bool IsPassive() { return GetType() == ITEM_PASSIVE; }
#endif

#ifdef ENABLE_VIP_SYSTEM
	bool IsVipItem() { return GetVnum() == 38001; }
#endif

	bool CanStackWith(LPITEM other);
	static bool IsSameItem(CItem a, CItem b);

private:
	TItemTable const* m_pProto;

	uint32_t m_dwVnum;
	LPCHARACTER m_pOwner;

	uint8_t m_bWindow;
	uint32_t m_dwID;
	bool m_bEquipped;
	uint32_t m_dwVID;
	uint16_t m_wCell;
	uint32_t m_dwCount;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	uint32_t m_dwTransmutationVnum;
#endif
	long m_lFlag;
	uint32_t m_dwLastOwnerPID;

	bool m_bExchanging;

	long m_alSockets[ITEM_SOCKET_MAX_NUM];
	TPlayerItemAttribute m_aAttr[ITEM_ATTRIBUTE_MAX_NUM];
#ifdef ENABLE_YOHARA_SYSTEM
	TPlayerItemApplyRandom m_aApplyRandom[APPLY_RANDOM_SLOT_MAX_NUM];
	uint16_t m_alRandomValues[ITEM_RANDOM_VALUES_MAX_NUM];
#endif

	LPEVENT m_pkDestroyEvent;
	LPEVENT m_pkExpireEvent;
	LPEVENT m_pkUniqueExpireEvent;
	LPEVENT m_pkTimerBasedOnWearExpireEvent;
	LPEVENT m_pkRealTimeExpireEvent;
#ifdef ENABLE_SOUL_SYSTEM
	LPEVENT m_pkSoulTimeUseEvent;
#endif
#ifdef ENABLE_CHANGE_LOOK_MOUNT
	LPEVENT m_pkChangeLookExpireEvent;
#endif
	LPEVENT m_pkAccessorySocketExpireEvent;
	LPEVENT m_pkOwnershipEvent;

	uint32_t m_dwOwnershipPID;

	bool m_bSkipSave;

	bool m_isLocked;

	uint32_t m_dwMaskVnum;
	uint32_t m_dwSIGVnum;

#ifdef ENABLE_REFINE_ELEMENT
	uint8_t m_grade_element;
	uint32_t m_attack_element[MAX_ELEMENTS_SPELL];
	uint8_t m_element_type_bonus;
	int16_t m_elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif

#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
private:
	LPEVENT m_pkAuraBoostSocketExpireEvent;

public:
	bool IsAuraBoosterForSocket();

	void StartAuraBoosterSocketExpireEvent();
	void StopAuraBoosterSocketExpireEvent();
	void SetAuraBoosterSocketExpireEvent(LPEVENT pkEvent);
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
private:
	TGrowthPetInfo m_dwPetInfo{ 0 };

public:
	const TGrowthPetInfo GetGrowthPetItemInfo() noexcept { return m_dwPetInfo; }
	void SetGrowthPetItemInfo(TGrowthPetInfo petInfo) noexcept { m_dwPetInfo = petInfo; }
#endif

public:
	void SetSIGVnum(uint32_t dwSIG) noexcept { m_dwSIGVnum = dwSIG; }
	uint32_t GetSIGVnum() const noexcept { return m_dwSIGVnum; }

#ifdef ENABLE_CHANGE_LOOK_MOUNT
public:
	void StartChangeLookExpireEvent();
	void StopChangeLookExpireEvent();
	bool IsExpireTimeItem();
	long GetRealExpireTime();
#endif
};

EVENTINFO(item_event_info)
{
	LPITEM item;
	char szOwnerName[CHARACTER_NAME_MAX_LEN];

	item_event_info()
		: item(0)
	{
		::memset(szOwnerName, 0, CHARACTER_NAME_MAX_LEN);
	}
};

EVENTINFO(item_vid_event_info)
{
	uint32_t item_vid;

	item_vid_event_info()
		: item_vid(0)
	{
	}
};

#endif
