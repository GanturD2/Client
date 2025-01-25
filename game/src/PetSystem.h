#ifndef __HEADER_PET_SYSTEM__
#define __HEADER_PET_SYSTEM__


class CHARACTER;

// TODO: 펫으로서의 능력치? 라던가 친밀도, 배고픔 기타등등... 수치
struct SPetAbility
{
};

/**
*/
class CPetActor //: public CHARACTER
{
public:
	enum EPetOptions
	{
		EPetOption_Followable = 1 << 0,
		EPetOption_Mountable = 1 << 1,
		EPetOption_Summonable = 1 << 2,
		EPetOption_Combatable = 1 << 3,
	};


protected:
	friend class CPetSystem;

	CPetActor(LPCHARACTER owner, uint32_t vnum, uint32_t options = EPetOption_Followable | EPetOption_Summonable);
	// CPetActor(LPCHARACTER owner, uint32_t vnum, const SPetAbility& petAbility, uint32_t options = EPetOption_Followable | EPetOption_Summonable);

	virtual ~CPetActor();

	virtual bool Update(uint32_t deltaTime);

protected:
	virtual bool _UpdateFollowAI(); ///< 주인을 따라다니는 AI 처리
	virtual bool _UpdatAloneActionAI(float fMinDist, float fMaxDist); ///< 주인 근처에서 혼자 노는 AI 처리

	/// @TODO
	//virtual bool _UpdateCombatAI();

private:
	bool Follow(float fMinDistance = 50.f);

public:
	LPCHARACTER GetCharacter() const { return m_pkChar; }
	LPCHARACTER GetOwner() const { return m_pkOwner; }
	uint32_t GetVID() const { return m_dwVID; }
	uint32_t GetVnum() const { return m_dwVnum; }

	bool HasOption(EPetOptions option) const { return m_dwOptions & option; }

#ifdef ENABLE_PET_SYSTEM
	void SetName();
#else
	void SetName(const char* petName);
#endif

	bool Mount();
	void Unmount();

#ifdef ENABLE_PET_SYSTEM
	uint32_t Summon(LPITEM pSummonItem, bool bSpawnFar = false);
#else
	uint32_t Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar = false);
#endif
	void Unsummon();

	bool IsSummoned() const { return 0 != m_pkChar; }
	void SetSummonItem(LPITEM pItem);
	uint32_t GetSummonItemVID() { return m_dwSummonItemVID; }

	void GiveBuff();
	void ClearBuff();

#ifdef PET_AUTO_PICKUP
	bool CheckPetPickup(time_t time);
	void PickUpItems(int range);
	void BringItem();
	void SetPickup(bool is_pickup) { m_is_pickup = is_pickup; }
	bool IsPickup() { return m_is_pickup; }
	void SetPickupItem(LPITEM item) { m_pickup_item = item; }
	LPITEM GetPickupItem() { return m_pickup_item; }
	void SetPickupTime(time_t zTime) { pickTime = zTime; }
	time_t GetPickupTime() { return pickTime; }
#endif

private:
	uint32_t m_dwVnum;
	uint32_t m_dwVID;
	uint32_t m_dwOptions;
	uint32_t m_dwLastActionTime;
	uint32_t m_dwSummonItemVID;
	uint32_t m_dwSummonItemVnum;

	int16_t m_originalMoveSpeed;

	std::string m_name;

	LPCHARACTER m_pkChar; // Instance of pet(CHARACTER)
	LPCHARACTER m_pkOwner;

#ifdef PET_AUTO_PICKUP
	bool m_is_pickup;
	LPITEM m_pickup_item;
	time_t pickTime;
#endif

	// SPetAbility m_petAbility; // 능력치
};

/**
*/
class CPetSystem
{
public:
	typedef std::unordered_map<uint32_t, CPetActor*> TPetActorMap; /// <VNUM, PetActor> map. (한 캐릭터가 같은 vnum의 펫을 여러개 가질 일이 있을까..??)

public:
	CPetSystem(LPCHARACTER owner);
	virtual ~CPetSystem();

	CPetActor* GetByVID(uint32_t vid) const;
	CPetActor* GetByVnum(uint32_t vnum) const;

	bool Update(uint32_t deltaTime);
	void Destroy();

	size_t CountSummoned() const; ///< 현재 소환된(실체화 된 캐릭터가 있는) 펫의 개수

public:
	void SetUpdatePeriod(uint32_t ms);

#ifdef ENABLE_PET_SYSTEM
	CPetActor* Summon(uint32_t mobVnum, LPITEM pSummonItem, bool bSpawnFar, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);
#else
	CPetActor* Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);
#endif

	void Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);
	void Unsummon(CPetActor* petActor, bool bDeleteFromList = false);

	// TODO: 진짜 펫 시스템이 들어갈 때 구현. (캐릭터가 보유한 펫의 정보를 추가할 때 라던가...)
	CPetActor* AddPet(uint32_t mobVnum, const char* petName, const SPetAbility& ability, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable | CPetActor::EPetOption_Combatable);

	void DeletePet(uint32_t mobVnum);
	void DeletePet(CPetActor* petActor);
	void RefreshBuff();

	bool IsActivePet();

private:
	TPetActorMap m_petActorMap;
	LPCHARACTER m_pkOwner; ///< 펫 시스템의 Owner
	uint32_t m_dwUpdatePeriod; ///< 업데이트 주기 (ms단위)
	uint32_t m_dwLastUpdateTime;
	LPEVENT m_pkPetSystemUpdateEvent;
};

/**
// Summon Pet
CPetSystem* petSystem = mainChar->GetPetSystem();
CPetActor* petActor = petSystem->Summon(~~~);

uint32_t petVID = petActor->GetVID();
if (0 == petActor)
{
ERROR_LOG(...)
};


// Unsummon Pet
petSystem->Unsummon(petVID);

// Mount Pet
petActor->Mount()..


CPetActor::Update(...)
{
// AI : Follow, actions, etc...
}

*/

#endif //__HEADER_PET_SYSTEM__
