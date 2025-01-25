#ifndef __HEADER_PET_SYSTEM__
#define __HEADER_PET_SYSTEM__


class CHARACTER;

// TODO: �����μ��� �ɷ�ġ? ����� ģ�е�, ����� ��Ÿ���... ��ġ
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
	virtual bool _UpdateFollowAI(); ///< ������ ����ٴϴ� AI ó��
	virtual bool _UpdatAloneActionAI(float fMinDist, float fMaxDist); ///< ���� ��ó���� ȥ�� ��� AI ó��

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

	// SPetAbility m_petAbility; // �ɷ�ġ
};

/**
*/
class CPetSystem
{
public:
	typedef std::unordered_map<uint32_t, CPetActor*> TPetActorMap; /// <VNUM, PetActor> map. (�� ĳ���Ͱ� ���� vnum�� ���� ������ ���� ���� ������..??)

public:
	CPetSystem(LPCHARACTER owner);
	virtual ~CPetSystem();

	CPetActor* GetByVID(uint32_t vid) const;
	CPetActor* GetByVnum(uint32_t vnum) const;

	bool Update(uint32_t deltaTime);
	void Destroy();

	size_t CountSummoned() const; ///< ���� ��ȯ��(��üȭ �� ĳ���Ͱ� �ִ�) ���� ����

public:
	void SetUpdatePeriod(uint32_t ms);

#ifdef ENABLE_PET_SYSTEM
	CPetActor* Summon(uint32_t mobVnum, LPITEM pSummonItem, bool bSpawnFar, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);
#else
	CPetActor* Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable);
#endif

	void Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);
	void Unsummon(CPetActor* petActor, bool bDeleteFromList = false);

	// TODO: ��¥ �� �ý����� �� �� ����. (ĳ���Ͱ� ������ ���� ������ �߰��� �� �����...)
	CPetActor* AddPet(uint32_t mobVnum, const char* petName, const SPetAbility& ability, uint32_t options = CPetActor::EPetOption_Followable | CPetActor::EPetOption_Summonable | CPetActor::EPetOption_Combatable);

	void DeletePet(uint32_t mobVnum);
	void DeletePet(CPetActor* petActor);
	void RefreshBuff();

	bool IsActivePet();

private:
	TPetActorMap m_petActorMap;
	LPCHARACTER m_pkOwner; ///< �� �ý����� Owner
	uint32_t m_dwUpdatePeriod; ///< ������Ʈ �ֱ� (ms����)
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
