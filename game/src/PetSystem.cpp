#include "stdafx.h"
#include "config.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "PetSystem.h"
#include "../../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "config.h"
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	#include "AchievementSystem.h"
#endif

extern bool IS_BLOCKED_PET_SUMMON_MAP(int map_index);	//@jeex006

EVENTINFO(petsystem_event_info)
{
	CPetSystem* pPetSystem;
};

EVENTFUNC(petsystem_update_event)
{
	petsystem_event_info* info = dynamic_cast<petsystem_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	CPetSystem*	pPetSystem = info->pPetSystem;

	if (nullptr == pPetSystem)
		return 0;


	pPetSystem->Update(0);

	return PASSES_PER_SEC(1) / 4;
}

const float PET_COUNT_LIMIT = 3;

///////////////////////////////////////////////////////////////////////////////////////
//  CPetActor
///////////////////////////////////////////////////////////////////////////////////////
CPetActor::CPetActor(LPCHARACTER owner, uint32_t vnum, uint32_t options)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwOptions = options;
	m_dwLastActionTime = 0;

	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;

	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;

#ifdef PET_AUTO_PICKUP
	m_pickup_item = nullptr;
	m_is_pickup = false;
	pickTime = 0;
#endif
}

CPetActor::~CPetActor()
{
	this->Unsummon();

	m_pkOwner = 0;
#ifdef PET_AUTO_PICKUP
	m_pickup_item = nullptr;
	m_is_pickup = false;
	pickTime = 0;
#endif
}

#ifdef ENABLE_PET_SYSTEM
void CPetActor::SetName()
#else
void CPetActor::SetName(const char* name)
#endif
{
	std::string petName = m_pkOwner->GetName();

#ifndef ENABLE_PET_SYSTEM
	if (0 != m_pkOwner &&
		0 == name &&
		0 != m_pkOwner->GetName())
	{
		petName += "'s Pet";
	}
	else
		petName += name;
#endif

	if (true == IsSummoned())
		m_pkChar->SetName(petName);

	m_name = petName;
}

bool CPetActor::Mount()
{
	if (0 == m_pkOwner)
		return false;

	if (true == HasOption(EPetOption_Mountable))
		m_pkOwner->MountVnum(m_dwVnum);

	return m_pkOwner->GetMountVnum() == m_dwVnum;
}

void CPetActor::Unmount()
{
	if (0 == m_pkOwner)
		return;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
}

void CPetActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
#ifdef ENABLE_ACHIEVEMENT_SYSTEM
		if (m_pkOwner->GetQuestFlag("pet.summon_time") > 0)
		{
			LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(m_dwSummonItemVID);
			if (pSummonItem) {
				CAchievementSystem::Instance().OnSummon(m_pkOwner, achievements::ETaskTypes::TYPE_SUMMON_PET, pSummonItem->GetVnum(), get_global_time() - m_pkOwner->GetQuestFlag("pet.summon_time"), false);
				m_pkOwner->SetQuestFlag("pet.summon_time", 0);
			}
		}
#endif

#ifndef ENABLE_PET_SYSTEM
		this->ClearBuff();
#endif
		this->SetSummonItem(nullptr);
#ifdef PET_AUTO_PICKUP
		this->SetPickupItem(nullptr);
		this->SetPickup(false);
		this->SetPickupTime(0);
#endif
		if (nullptr != m_pkOwner)
			m_pkOwner->ComputePoints();

		if (nullptr != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);

		m_pkChar = 0;
		m_dwVID = 0;
	}
}

bool CPetSystem::IsActivePet()
{
	bool state = false;

	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;
		if (petActor != 0)
		{
			if (petActor->IsSummoned())
			{
				state = true;
				break;
			}
		}
	}

	return state;
}

#ifdef ENABLE_PET_SYSTEM
uint32_t CPetActor::Summon(LPITEM pSummonItem, bool bSpawnFar)
#else
uint32_t CPetActor::Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar)
#endif
{
	if (IS_BLOCKED_PET_SUMMON_MAP(m_pkOwner->GetMapIndex())) {	//@jeex006
		m_pkOwner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("YOU_CANNOT_SUMMON_A_PET_HERE"));
		return 0;
	}

	long x = m_pkOwner->GetX();
	long y = m_pkOwner->GetY();
	long z = m_pkOwner->GetZ();

	if (true == bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (0 != m_pkChar)
	{
		m_pkChar->Show (m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}

	m_pkChar = CHARACTER_MANAGER::Instance().SpawnMob(
		m_dwVnum,
		m_pkOwner->GetMapIndex(),
		x, y, z,
		false, (int)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CPetSystem::Summon] Failed to summon the pet. (vnum: %d)", m_dwVnum);
		return 0;
	}

	m_pkChar->SetPet();

//	m_pkOwner->DetailLog();
//	m_pkChar->DetailLog();

	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();

#ifdef ENABLE_PET_SYSTEM
	this->SetName();
#else
	this->SetName(petName);
#endif

	this->SetSummonItem(pSummonItem);
	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);

#ifdef ENABLE_ACHIEVEMENT_SYSTEM
	if (pSummonItem)
	{
		CAchievementSystem::Instance().OnSummon(m_pkOwner, achievements::ETaskTypes::TYPE_SUMMON_PET, pSummonItem->GetVnum(), 0, false);
		m_pkOwner->SetQuestFlag("pet.summon_time", get_global_time());
	}
#endif

	return m_dwVID;
}

bool CPetActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	const float fDist = fnumber(fMinDist, fMaxDist);
	const float r = fnumber (0.0f, 359.0f);
	const float dest_x = GetOwner()->GetX() + fDist * cos(r);
	const float dest_y = GetOwner()->GetY() + fDist * sin(r);

	//m_pkChar->SetRotation(number(0, 359));

	//GetDeltaByDegree(m_pkChar->GetRotation(), fDist, &fx, &fy);

	//if (!(SECTREE_MANAGER::Instance().IsMovablePosition(m_pkChar->GetMapIndex(), m_pkChar->GetX() + (int) fx, m_pkChar->GetY() + (int) fy)
	//	&& SECTREE_MANAGER::Instance().IsMovablePosition(m_pkChar->GetMapIndex(), m_pkChar->GetX() + (int) fx/2, m_pkChar->GetY() + (int) fy/2)))
	//	return true;

	m_pkChar->SetNowWalking(true);

	//if (m_pkChar->Goto(m_pkChar->GetX() + (int) fx, m_pkChar->GetY() + (int) fy))
	//	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

// char_state.cpp StateHorse
bool CPetActor::_UpdateFollowAI()
{
	if (0 == m_pkChar->m_pkMobData)
	{
		//sys_err("[CPetActor::_UpdateFollowAI] m_pkChar->m_pkMobData is nullptr");
		return false;
	}

	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}

	const float	START_FOLLOW_DISTANCE = 300.0f;
	const float	START_RUN_DISTANCE = 900.0f;

	const float	RESPAWN_DISTANCE = 4500.0f;
	const float	APPROACH = 200.0f;

	bool bRun = false;

	const uint32_t currentTime = get_dword_time();

	const long ownerX = m_pkOwner->GetX();
	const long ownerY = m_pkOwner->GetY();
	const long charX = m_pkChar->GetX();
	const long charY = m_pkChar->GetY();

	const float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		const float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		const float fx = -APPROACH * cos(fOwnerRot);
		const float fy = -APPROACH * sin(fOwnerRot);
		if (m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			return true;
		}
	}

	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if (fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}

		m_pkChar->SetNowWalking(!bRun);

		Follow(APPROACH);

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	//else
	//{
	//	if (fabs(m_pkChar->GetRotation() - GetDegreeFromPositionXY(charX, charY, ownerX, ownerX)) > 10.f || fabs(m_pkChar->GetRotation() - GetDegreeFromPositionXY(charX, charY, ownerX, ownerX)) < 350.f)
	//	{
	//		m_pkChar->Follow(m_pkOwner, APPROACH);
	//		m_pkChar->SetLastAttacked(currentTime);
	//		m_dwLastActionTime = currentTime;
	//	}
	//}
	else
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	//else if (currentTime - m_dwLastActionTime > number(5000, 12000))
	//{
	//	this->_UpdatAloneActionAI(START_FOLLOW_DISTANCE / 2, START_FOLLOW_DISTANCE);
	//}

	return true;
}

bool CPetActor::Update(uint32_t deltaTime)
{
	bool bResult = true;

#ifdef ENABLE_PET_SYSTEM
	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead())
		|| nullptr == ITEM_MANAGER::Instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::Instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		const LPITEM item = ITEM_MANAGER::Instance().FindByVID(m_dwSummonItemVID);
		if (!item)
			return false;

		if (!item->IsEquipped())	//@fixme438: Fix pet unsummoning while it's still equipped.
		//@fixme454
		m_pkOwner->SetQuestFlag("pet_system.pet_vnum", 0);
		m_pkOwner->SetQuestFlag("pet_item_id.pet_vnum", 0);
		//@end_fixme454
		this->Unsummon();
		return true;
	}

	const LPITEM item2 = ITEM_MANAGER::Instance().FindByVID(m_dwSummonItemVID);
	if (!item2)
		return false;

	if (this->IsSummoned() && !item2->IsEquipped())	//@fixme438: Fix pet is summoned while item is unequiped.
	{
		//@fixme454
		m_pkOwner->SetQuestFlag("pet_system.pet_vnum", 0);
		m_pkOwner->SetQuestFlag("pet_item_id.pet_vnum", 0);
		//@end_fixme454
		this->Unsummon();
		return true;
	}
#else
	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead())
		|| nullptr == ITEM_MANAGER::Instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::Instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		//@fixme454
		m_pkOwner->SetQuestFlag("pet_system.pet_vnum", 0);
		m_pkOwner->SetQuestFlag("pet_item_id.pet_vnum", 0);
		//@end_fixme454
		this->Unsummon();
		return true;
	}
#endif

	if (this->IsSummoned() && HasOption(EPetOption_Followable))
		bResult = bResult && this->_UpdateFollowAI();

#ifdef PET_AUTO_PICKUP
	if (this->IsSummoned() && this->m_pkChar->GetRaceNum() == 34055)	//Bruce
	{
		this->CheckPetPickup(get_global_time());
	}
#endif

	return bResult;
}

bool CPetActor::Follow(float fMinDistance)
{
	if (!m_pkOwner || !m_pkChar)
		return false;

	const float fDist = DISTANCE_SQRT(m_pkOwner->GetX() - m_pkChar->GetX(), m_pkOwner->GetY() - m_pkChar->GetY());
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(m_pkOwner->GetX(), m_pkOwner->GetY());

	float fx, fy;

	const float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);

	if (!m_pkChar->Goto((int)(m_pkChar->GetX() +fx+0.5f), (int)(m_pkChar->GetY() +fy+0.5f)) )
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CPetActor::SetSummonItem(LPITEM pItem)
{
	if (nullptr == pItem)
	{
		//@fixme430
		LPITEM pSummonItem = ITEM_MANAGER::Instance().FindByVID(m_dwSummonItemVID);
		if (nullptr != pSummonItem)
			pSummonItem->SetSocket(1, FALSE);
		// @end_fixme430

		m_dwSummonItemVID = 0;
		m_dwSummonItemVnum = 0;
#ifdef PET_AUTO_PICKUP
		this->SetPickupItem(nullptr);
		this->SetPickup(false);
		this->SetPickupTime(0);
#endif
		return;
	}

	pItem->SetSocket(1, TRUE);	//@fixme430
	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

bool __PetCheckBuff(const CPetActor* pPetActor)
{
	bool bMustHaveBuff = true;

	if (!pPetActor)
		bMustHaveBuff = false;

	switch (pPetActor->GetVnum())
	{
		case 34004:
		case 34009:
			if (nullptr == pPetActor->GetOwner()->GetDungeon())
				bMustHaveBuff = false;
		default:
			break;
	}
	return bMustHaveBuff;
}

void CPetActor::GiveBuff()
{
	if (!__PetCheckBuff(this))
		return;

	const LPITEM item = ITEM_MANAGER::Instance().FindByVID(m_dwSummonItemVID);
	if (nullptr != item)
		item->ModifyPoints(true);
	return;
}

void CPetActor::ClearBuff()
{
	if (nullptr == m_pkOwner)
		return;
	const TItemTable* item_proto = ITEM_MANAGER::Instance().GetTable(m_dwSummonItemVnum);
	if (nullptr == item_proto)
		return;
	if (!__PetCheckBuff(this)) // @fixme129
		return;
	for (int i = 0; i < ITEM_APPLY_MAX_NUM; i++)
	{
		if (item_proto->aApplies[i].wType == APPLY_NONE)	//@jeex015
			continue;
		m_pkOwner->ApplyPoint(item_proto->aApplies[i].wType, -item_proto->aApplies[i].lValue);	//@jeex015
	}

	return;
}

#ifdef PET_AUTO_PICKUP
bool CPetActor::CheckPetPickup(time_t time)
{
	if (!GetPickupTime())
	{
		SetPickupTime(time);
		//m_pkOwner->ChatPacket(7, "gobale time set %d", pickTime);
	}
	else
	{
		if (GetPickupTime() + 10 < time)
		{
			//m_pkOwner->ChatPacket(7, "Start Pickup Timer %d", time);
			PickUpItems(900); // 900 = RANGE
			BringItem();
			return true;
		}
	}

	return true;
}

struct PetPickUpItemStruct
{
	CPetActor* pet;
	int range;
	PetPickUpItemStruct(CPetActor* p, int r)
	{
		pet = p;
		range = r;
	}

	void operator()(LPENTITY pEnt)
	{
		if (!pet->GetOwner() || !pet->GetCharacter())
			return;

		if (pet->IsPickup())
			return;

		if (pEnt->IsType(ENTITY_ITEM) == true)
		{
			LPITEM item = (LPITEM)pEnt;
			LPCHARACTER player = pet->GetOwner();

			if (!item || !player)
				return;

			if (!item->GetSectree() || !item->IsOwnership(player))
				return;

			const int iDist = DISTANCE_APPROX(item->GetX() - player->GetX(), player->GetY() - player->GetY());

			if (iDist > range)
				return;

			pet->SetPickup(true);
			pet->SetPickupItem(item);
		}

	}
};

void CPetActor::PickUpItems(int range)
{
	if (IsPickup())
		return;

	const long map = m_pkChar->GetMapIndex();
	const PIXEL_POSITION m = m_pkChar->GetXYZ();
	LPSECTREE tree = SECTREE_MANAGER::Instance().Get(map, m.x, m.y);
	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d", m.x, m.y);
		return;
	}

	PetPickUpItemStruct f(this, range);
	tree->ForEachAround(f);
}

void CPetActor::BringItem()
{
	if (!IsPickup())
		return;

	if (GetPickupItem() == nullptr)
		return;

	if (!m_pkOwner || !m_pkChar)
		return;

	LPITEM item = GetPickupItem();
	if (item == nullptr)
		return;

	if (item)
	{
		constexpr float fMinDistance = 20.0f;
		const float fDist = DISTANCE_SQRT(item->GetX() - m_pkChar->GetX(), item->GetY() - m_pkChar->GetY());
		if (fDist <= 250.0)
		{
			SetPickup(false);
			m_pkOwner->PickupItemByPet(item->GetVID());
			SetPickupItem(nullptr);
			m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);
			return;
		}
		m_pkChar->SetRotationToXY(item->GetX(), item->GetY());
		float fx, fy;

		const float fDistToGo = fDist - fMinDistance;
		GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);

		if (!m_pkChar->Goto(static_cast<long>(m_pkChar->GetX() + fx + 0.5f), static_cast<long>(m_pkChar->GetY() + fy + 0.5f)))
			return;

		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////////////
//  CPetSystem
///////////////////////////////////////////////////////////////////////////////////////
CPetSystem::CPetSystem(LPCHARACTER owner)
{
//	assert(0 != owner && "[CPetSystem::CPetSystem] Invalid owner");

	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CPetSystem::~CPetSystem()
{
	Destroy();
}

void CPetSystem::Destroy()
{
	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor)
		{
			delete petActor;
		}
	}
	event_cancel(&m_pkPetSystemUpdateEvent);
	m_petActorMap.clear();
}

bool CPetSystem::Update(uint32_t deltaTime)
{
	bool bResult = true;

	const uint32_t currentTime = get_dword_time();

	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;

	std::vector <CPetActor*> v_garbageActor;

	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor && petActor->IsSummoned())
		{
			const LPCHARACTER& pPet = petActor->GetCharacter();
			if (!pPet)
				return false;

			if (nullptr == CHARACTER_MANAGER::Instance().Find(pPet->GetVID()))
			{
				v_garbageActor.emplace_back(petActor);
			}
			else
			{
				bResult = bResult && petActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CPetActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); ++it)	//@fixme541
		DeletePet(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CPetSystem::DeletePet(uint32_t mobVnum)
{
	const auto iter = m_petActorMap.find(mobVnum);

	if (m_petActorMap.end() == iter)
	{
		sys_err("[CPetSystem::DeletePet] Can't find pet on my list (VNUM: %d)", mobVnum);
		return;
	}

	CPetActor* petActor = iter->second;

	if (0 == petActor)
		sys_err("[CPetSystem::DeletePet] Null Pointer (petActor)");
	else
		delete petActor;

	m_petActorMap.erase(iter);
}

void CPetSystem::DeletePet(CPetActor* petActor)
{
	for (TPetActorMap::iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		if (iter->second == petActor)
		{
			delete petActor;
			m_petActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CPetSystem::DeletePet] Can't find petActor(0x%x) on my list(size: %d) ", petActor, m_petActorMap.size());
}

void CPetSystem::Unsummon(uint32_t vnum, bool bDeleteFromList)
{
	CPetActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CPetSystem::GetByVnum(%d)] Null Pointer (petActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeletePet(actor);

	bool bActive = false;
	for (TPetActorMap::iterator it = m_petActorMap.begin(); it != m_petActorMap.end(); ++it)	//@fixme541
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pkPetSystemUpdateEvent);
		m_pkPetSystemUpdateEvent = nullptr;
	}
}

#ifdef ENABLE_PET_SYSTEM
CPetActor* CPetSystem::Summon(uint32_t mobVnum, LPITEM pSummonItem, bool bSpawnFar, uint32_t options)
#else
CPetActor* CPetSystem::Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options)
#endif
{
	CPetActor* petActor = this->GetByVnum(mobVnum);

	if (0 == petActor)
	{
		petActor = M2_NEW CPetActor(m_pkOwner, mobVnum, options);
		m_petActorMap.insert(std::make_pair(mobVnum, petActor));
	}

#ifdef ENABLE_NEWSTUFF
#	ifdef ENABLE_PET_SYSTEM
	uint32_t petVID = petActor->Summon(pSummonItem, bSpawnFar);
#	else
	uint32_t petVID = petActor->Summon(petName, pSummonItem, bSpawnFar);
#	endif
	if (!petVID)
		sys_err("[CPetSystem::Summon(%d)] Null Pointer (petVID)", pSummonItem);
#endif

	if (nullptr == m_pkPetSystemUpdateEvent)
	{
		petsystem_event_info* info = AllocEventInfo<petsystem_event_info>();

		info->pPetSystem = this;

		m_pkPetSystemUpdateEvent = event_create(petsystem_update_event, info, PASSES_PER_SEC(1) / 4);	// 0.25ÃÊ
	}

	return petActor;
}

CPetActor* CPetSystem::GetByVID(uint32_t vid) const
{
	CPetActor* petActor = 0;

	bool bFound = false;

	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		petActor = iter->second;

		if (0 == petActor)
		{
			sys_err("[CPetSystem::GetByVID(%d)] Null Pointer (petActor)", vid);
			continue;
		}

		bFound = petActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? petActor : 0;
}

CPetActor* CPetSystem::GetByVnum(uint32_t vnum) const
{
	CPetActor* petActor = 0;

	TPetActorMap::const_iterator iter = m_petActorMap.find(vnum);

	if (m_petActorMap.end() != iter)
		petActor = iter->second;

	return petActor;
}

size_t CPetSystem::CountSummoned() const
{
	size_t count = 0;

	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor)
		{
			if (petActor->IsSummoned())
				++count;
		}
	}

	return count;
}

void CPetSystem::RefreshBuff()
{
	for (TPetActorMap::const_iterator iter = m_petActorMap.begin(); iter != m_petActorMap.end(); ++iter)
	{
		CPetActor* petActor = iter->second;

		if (0 != petActor)
		{
			if (petActor->IsSummoned())
			{
				petActor->GiveBuff();
			}
		}
	}
}
