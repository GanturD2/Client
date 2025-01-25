#include "StdAfx.h"
#include "ActorInstance.h"
#include "AreaTerrain.h"
#include "RaceData.h"
#include "../SpeedTreeLib/CSpeedTreeDirectX.h"
#include "../SpeedTreeLib/SpeedTreeWrapper.h"
#include "GameLibDefines.h"

enum
{
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_MAX_NUM = 9,
#else
	MAIN_RACE_MAX_NUM = 8,
#endif
};

void CActorInstance::INSTANCEBASE_Deform()
{
	Deform();
	TraceProcess();
}

void CActorInstance::INSTANCEBASE_Transform()
{
	if (m_pkHorse)
	{
		m_pkHorse->INSTANCEBASE_Transform();

		m_x = m_pkHorse->NEW_GetCurPixelPositionRef().x;
		m_y = -m_pkHorse->NEW_GetCurPixelPositionRef().y;
		m_z = m_pkHorse->NEW_GetCurPixelPositionRef().z;
		m_bNeedUpdateCollision = TRUE;
	}

	Update();
	TransformProcess();
	Transform();
	UpdatePointInstance();
	ShakeProcess();
	UpdateBoundingSphere();
	UpdateAttribute();
}

void CActorInstance::OnUpdate()
{
	if (!IsParalysis())
		CGraphicThingInstance::OnUpdate();

	UpdateAttachingInstances();

	__BlendAlpha_Update();

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (m_eRace >= 34000 && m_eRace <= 34999)
		m_bShowActor = CPythonGraphicOnOff::Instance().IsPetOnOffStatus();

	if (!m_isMain && HasPrivateShopSign())
		m_bShowActor = CPythonGraphicOnOff::Instance().IsPrivateShopOnOffLevel(m_dwDistanceFromMainCharacter);

	if (!m_bShowActor)
		return HideAllAttachingEffect();

	if (CPythonGraphicOnOff::Instance().IsEffectOnOffLevel(GetActorType(), FALSE != m_isMain, GetRank()) && (!m_bIsInvisibleActor || m_isMain))
	{
		if (!m_bShowEffects)
		{
			ShowAllAttachingEffect();
			m_bShowEffects = true;
		}
	}
	else if (m_bShowEffects)
	{
		HideAllAttachingEffect();
		m_bShowEffects = false;
	}
#endif
}

IBackground & CActorInstance::GetBackground()
{
	return IBackground::Instance();
}

void CActorInstance::SetMainInstance()
{
	m_isMain = true;
}

void CActorInstance::SetParalysis(bool isParalysis)
{
	m_isParalysis = isParalysis;
}

void CActorInstance::SetFaint(bool isFaint)
{
	m_isFaint = isFaint;
}

void CActorInstance::SetSleep(bool isSleep)
{
	m_isSleep = isSleep;

	Stop();
}

void CActorInstance::SetResistFallen(bool isResistFallen)
{
	m_isResistFallen = isResistFallen;
}

void CActorInstance::SetReachScale(float fScale)
{
	m_fReachScale = fScale;
}

float CActorInstance::__GetReachScale()
{
	return m_fReachScale;
}

float CActorInstance::__GetAttackSpeed()
{
	return m_fAtkSpd;
}

uint16_t CActorInstance::__GetCurrentComboType()
{
	if (IsBowMode())
		return 0;
	if (IsHandMode())
		return 0;
	if (__IsMountingHorse())
		return 0;

	return m_wcurComboType;
}

void CActorInstance::SetComboType(uint16_t wComboType)
{
	m_wcurComboType = wComboType;
}

void CActorInstance::SetAttackSpeed(float fAtkSpd)
{
	m_fAtkSpd = fAtkSpd;
}

void CActorInstance::SetMoveSpeed(float fMovSpd)
{
	if (m_fMovSpd == fMovSpd)
		return;

	m_fMovSpd = fMovSpd;

	if (__IsMoveMotion())
	{
		Stop();
		Move();
	}
}

void CActorInstance::SetFishingPosition(D3DXVECTOR3 & rv3Position)
{
	m_v3FishingPosition = rv3Position;
}

void CActorInstance::Move()
{
	if (m_isWalking)
		SetLoopMotion(CRaceMotionData::NAME_WALK, 0.15f, m_fMovSpd);
	else
		SetLoopMotion(CRaceMotionData::NAME_RUN, 0.15f, m_fMovSpd);
}

void CActorInstance::Stop(float fBlendingTime)
{
	__ClearMotion();
	SetLoopMotion(CRaceMotionData::NAME_WAIT, fBlendingTime);
}

void CActorInstance::SetOwner(uint32_t dwOwnerVID)
{
	m_fOwnerBaseTime = GetLocalTime();
	m_dwOwnerVID = dwOwnerVID;
}

bool CActorInstance::GetActorFallState()
{
	return m_bIsFall;
}

void CActorInstance::SetActorFallState(bool bIsFall)
{
	m_bIsFall = bIsFall;
}

void CActorInstance::SetActorType(uint32_t eType)
{
	m_eActorType = eType;
}

uint32_t CActorInstance::GetActorType() const
{
	return m_eActorType;
}

bool CActorInstance::IsHandMode()
{
	if (CRaceMotionData::MODE_GENERAL == GetMotionMode())
		return true;

	if (CRaceMotionData::MODE_HORSE == GetMotionMode())
		return true;

	return false;
}

bool CActorInstance::IsTwoHandMode()
{
	if (CRaceMotionData::MODE_TWOHAND_SWORD == GetMotionMode())
		return true;

	return false;
}

bool CActorInstance::IsBowMode()
{
	if (CRaceMotionData::MODE_BOW == GetMotionMode())
		return true;

	if (CRaceMotionData::MODE_HORSE_BOW == GetMotionMode())
		return true;

	return false;
}

bool CActorInstance::IsPoly()
{
	if (TYPE_POLY == m_eActorType)
		return true;

	if (TYPE_PC == m_eActorType)
		if (m_eRace >= MAIN_RACE_MAX_NUM)
			return TRUE;

	return false;
}

bool CActorInstance::IsPC()
{
	if (TYPE_PC == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsNPC()
{
	if (TYPE_NPC == m_eActorType)
		return true;

	return false;
}

#ifdef ENABLE_PROTO_RENEWAL
bool CActorInstance::IsPetPay()
{
	if (TYPE_PET_PAY == m_eActorType)
		return true;

	return false;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
bool CActorInstance::IsGrowthPet()
{
	if (TYPE_PET == m_eActorType)
		return true;

	return false;
}
#endif

#if defined(ENABLE_PREMIUM_PRIVATE_SHOP) && defined(ENABLE_PROTO_RENEWAL)
bool CActorInstance::IsShop()
{
	return (m_eActorType == TYPE_SHOP);
}
#else
bool CActorInstance::IsShop()
{
	const std::vector<uint16_t> ShopVnums = {
		30000,
	};

	for (auto it : ShopVnums) {
		if (GetRace() == it)
			return true;
	}

	return false;
}
#endif

bool CActorInstance::IsEnemy()
{
	if (TYPE_ENEMY == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsStone()
{
	if (TYPE_STONE == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsWarp()
{
	if (TYPE_WARP == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsGoto()
{
	if (TYPE_GOTO == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsBuilding()
{
	if (TYPE_BUILDING == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsDoor()
{
	if (TYPE_DOOR == m_eActorType)
		return true;

	return false;
}

bool CActorInstance::IsObject()
{
	if (TYPE_OBJECT == m_eActorType)
		return true;

	return false;
}

#ifdef ENABLE_PROTO_RENEWAL
bool CActorInstance::IsHorse()
{
	return (m_eActorType == TYPE_HORSE);
}
#endif

uint32_t CActorInstance::GetRank()
{
	return m_dwRank;
}

void CActorInstance::SetRank(uint32_t rank)
{
	m_dwRank = rank;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CActorInstance::ChangeSkillColor(const uint32_t* dwSkillColor)
{
	memcpy(m_dwSkillColor, dwSkillColor, sizeof(m_dwSkillColor));
}
#endif

void CActorInstance::DestroySystem() {}

void CActorInstance::DieEnd()
{
	Die();

	CGraphicThingInstance::SetMotionAtEnd();
}

void CActorInstance::Die()
{
	if (m_isRealDead)
		return;

	if (__IsMoveMotion())
		Stop();

	SetAdvancingRotation(GetRotation());

	if (IsStone())
		InterceptOnceMotion(CRaceMotionData::NAME_DEAD);
	else
	{
		if (!__IsDieMotion())
			InterceptOnceMotion(CRaceMotionData::NAME_DEAD);
	}

	m_isRealDead = TRUE;
}

BOOL CActorInstance::IsSleep()
{
	return m_isSleep;
}

BOOL CActorInstance::IsParalysis()
{
	return m_isParalysis;
}

BOOL CActorInstance::IsFaint()
{
	return m_isFaint;
}

BOOL CActorInstance::IsResistFallen()
{
	return m_isResistFallen;
}

BOOL CActorInstance::IsMoving()
{
	return __IsMoveMotion();
}

BOOL CActorInstance::IsWaiting()
{
	return __IsWaitMotion();
}

BOOL CActorInstance::IsDead()
{
	return m_isRealDead;
}

BOOL CActorInstance::IsKnockDown()
{
	return __IsKnockDownMotion();
}

BOOL CActorInstance::IsDamage()
{
	return __IsDamageMotion();
}

BOOL CActorInstance::IsAttacked()
{
	if (IsPushing())
		return TRUE;

	if (__IsDamageMotion())
		return TRUE;

	if (__IsKnockDownMotion())
		return TRUE;

	if (__IsDieMotion())
		return TRUE;

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Process
void CActorInstance::PhysicsProcess()
{
	m_PhysicsObject.Update(m_fSecondElapsed);
	AddMovement(m_PhysicsObject.GetXMovement(), m_PhysicsObject.GetYMovement(), 0.0f);
}

void CActorInstance::__AccumulationMovement(float fRot)
{
	if (CRaceMotionData::NAME_WAIT == __GetCurrentMotionIndex())
		return;

	D3DXMATRIX s_matRotationZ;
	D3DXMatrixRotationZ(&s_matRotationZ, D3DXToRadian(fRot));
	UpdateTransform(&s_matRotationZ, GetAverageSecondElapsed());
#ifdef UCAN_ADAM
	AddMovement(s_matRotationZ._41, s_matRotationZ._42, s_matRotationZ._43 + ucanAdam_z);
#else
	AddMovement(s_matRotationZ._41, s_matRotationZ._42, s_matRotationZ._43);
#endif
}

void CActorInstance::AccumulationMovement()
{
	if (m_pkTree)
		return;

	if (m_pkHorse)
	{
		m_pkHorse->__AccumulationMovement(m_fcurRotation);
		return;
	}

	__AccumulationMovement(m_fAdvancingRotation);
}

void CActorInstance::TransformProcess()
{
	if (!IsParalysis())
	{
		m_x += m_v3Movement.x;
		m_y += m_v3Movement.y;
		m_z += m_v3Movement.z;
	}

	__InitializeMovement();

	SetPosition(m_x, m_y, m_z);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Process

void CActorInstance::OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector)
{
	assert(pscdVector);
	CStaticCollisionDataVector::const_iterator it;
	for (it = pscdVector->begin(); it != pscdVector->end(); ++it)
	{
		const CStaticCollisionData & c_rColliData = *it;
		const D3DXMATRIX & c_rMatrix = GetTransform();
		AddCollision(&c_rColliData, &c_rMatrix);
	}
}

void CActorInstance::OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance)
{
	assert(pAttributeInstance);
	SetHeightInstance(pAttributeInstance);
}

bool CActorInstance::OnGetObjectHeight(float fX, float fY, float * pfHeight)
{
	if (!m_pHeightAttributeInstance)
		return false;

	if (TYPE_BUILDING != GetType())
		return false;

	return m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight) == 1 ? true : false;
}

//////////////////////////////////////////////////////////////////
// Battle
void CActorInstance::Revive()
{
	m_isSleep = FALSE;
	m_isParalysis = FALSE;
	m_isFaint = FALSE;
	m_isRealDead = FALSE;
	m_isStun = FALSE;
	m_isWalking = FALSE;
	m_isMain = FALSE;
	m_isResistFallen = FALSE;

	__InitializeCollisionData();
}

BOOL CActorInstance::IsStun()
{
	return m_isStun;
}

void CActorInstance::Stun()
{
	m_isStun = TRUE;
}

void CActorInstance::SetWalkMode()
{
	m_isWalking = TRUE;
	if (CRaceMotionData::NAME_RUN == GET_MOTION_INDEX(m_kCurMotNode.dwMotionKey))
		SetLoopMotion(CRaceMotionData::NAME_WALK, 0.15f, m_fMovSpd);
}

void CActorInstance::SetRunMode()
{
	m_isWalking = FALSE;
	if (CRaceMotionData::NAME_WALK == GET_MOTION_INDEX(m_kCurMotNode.dwMotionKey))
		SetLoopMotion(CRaceMotionData::NAME_RUN, 0.15f, m_fMovSpd);
}

MOTION_KEY CActorInstance::GetNormalAttackIndex()
{
	uint16_t wMotionIndex;
	m_pkCurRaceData->GetNormalAttackIndex(GetMotionMode(), &wMotionIndex);

	return MAKE_MOTION_KEY(GetMotionMode(), wMotionIndex);
}

//////////////////////////////////////////////////////////////////
// Movement
void CActorInstance::__InitializeMovement()
{
	m_v3Movement.x = 0.0f;
	m_v3Movement.y = 0.0f;
	m_v3Movement.z = 0.0f;
}

void CActorInstance::AddMovement(float fx, float fy, float fz)
{
	m_v3Movement.x += fx;
	m_v3Movement.y += fy;
	m_v3Movement.z += fz;
}

#ifdef UCAN_ADAM
void CActorInstance::UcanAdam(float z)
{
	if (m_pkHorse)
	{
		if ((m_pkHorse->ucanAdam_z + z) < 0.0f)
			return;

		if ((m_pkHorse->ucanAdam_z + z) == 0.0f)
			DisableSkipCollision();
		else if ((m_pkHorse->ucanAdam_z + z) > 0.0f)
			EnableSkipCollision();

		switch (m_pkHorse->GetRace())
		{
		case 20252:		// Flug Mounts Mob ID aus der Mop_proto Vnum
		case 20263:
		case 20264:
		case 20269:
		case 20270:
		case 20271:
		case 20272:
			m_pkHorse->ucanAdam_z += z;
			break;
		default:
			break;
		}

		if (CRaceMotionData::NAME_WAIT == __GetCurrentMotionIndex())
			m_pkHorse->AddMovement(0.0f, 0.0f, m_pkHorse->ucanAdam_z);
	}
}
#endif

const float gc_fActorSlideMoveSpeed = 5.0f;

void CActorInstance::AdjustDynamicCollisionMovement(const CActorInstance * c_pActorInstance)
{
	if (m_pkHorse)
	{
		m_pkHorse->AdjustDynamicCollisionMovement(c_pActorInstance);
		return;
	}

#ifdef UCAN_ADAM
	if (ucanAdam_z > 0.0f)
		return;
#endif

	if (isAttacking())
		return;

	uint32_t uActorType = c_pActorInstance->GetActorType();
	if ((uActorType == TYPE_BUILDING) || (uActorType == TYPE_OBJECT) || (uActorType == TYPE_DOOR) || (uActorType == TYPE_STONE))
		BlockMovement();
	else
	{
		float move_length = D3DXVec3Length(&m_v3Movement);
		if (move_length > gc_fActorSlideMoveSpeed)
			m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;

		auto itMain = m_BodyPointInstanceList.begin();
		for (; itMain != m_BodyPointInstanceList.end(); ++itMain)
		{
			CDynamicSphereInstanceVector & c_rMainSphereVector = (*itMain).SphereInstanceVector;
			for (auto & c_rMainSphere : c_rMainSphereVector)
			{
				auto itOpp = c_pActorInstance->m_BodyPointInstanceList.begin();
				for (; itOpp != c_pActorInstance->m_BodyPointInstanceList.end(); ++itOpp)
				{
					CSphereCollisionInstance s;
					s.GetAttribute().fRadius = itOpp->SphereInstanceVector[0].fRadius;
					s.GetAttribute().v3Position = itOpp->SphereInstanceVector[0].v3Position;
					D3DXVECTOR3 v3Delta = s.GetCollisionMovementAdjust(c_rMainSphere);
					m_v3Movement += v3Delta;
					c_rMainSphere.v3Position += v3Delta;

					if (v3Delta.x != 0.0f || v3Delta.y != 0.0f || v3Delta.z != 0.0f)
					{
						move_length = D3DXVec3Length(&m_v3Movement);
						if (move_length > gc_fActorSlideMoveSpeed)
						{
							m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;
							c_rMainSphere.v3Position = c_rMainSphere.v3LastPosition;
							c_rMainSphere.v3Position += m_v3Movement;
						}
					}
				}
			}
		}
	}
}


void CActorInstance::__AdjustCollisionMovement(const CGraphicObjectInstance * c_pGraphicObjectInstance)
{
	if (m_pkHorse)
	{
		m_pkHorse->__AdjustCollisionMovement(c_pGraphicObjectInstance);
		return;
	}

#ifdef UCAN_ADAM
	if (ucanAdam_z > 0.0f)
		return;
#endif

	// Body´Â ÇÏ³ªÀÓÀ» °¡Á¤ÇÕ´Ï´Ù.

	if (m_v3Movement.x == 0.0f && m_v3Movement.y == 0.0f && m_v3Movement.z == 0.0f)
		return;

	float move_length = D3DXVec3Length(&m_v3Movement);
	if (move_length > gc_fActorSlideMoveSpeed)
		m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;

	auto itMain = m_BodyPointInstanceList.begin();
	for (; itMain != m_BodyPointInstanceList.end(); ++itMain)
	{
		CDynamicSphereInstanceVector & c_rMainSphereVector = (*itMain).SphereInstanceVector;
		for (auto & c_rMainSphere : c_rMainSphereVector)
		{
			D3DXVECTOR3 v3Delta = c_pGraphicObjectInstance->GetCollisionMovementAdjust(c_rMainSphere);
			m_v3Movement += v3Delta;
			c_rMainSphere.v3Position += v3Delta;

			if (v3Delta.x != 0.0f || v3Delta.y != 0.0f || v3Delta.z != 0.0f)
			{
				move_length = D3DXVec3Length(&m_v3Movement);
				if (move_length > gc_fActorSlideMoveSpeed)
				{
					m_v3Movement *= gc_fActorSlideMoveSpeed / move_length;
					c_rMainSphere.v3Position = c_rMainSphere.v3LastPosition;
					c_rMainSphere.v3Position += m_v3Movement;
				}
			}
		}
	}
}

BOOL CActorInstance::IsMovement()
{
	if (m_pkHorse)
		if (m_pkHorse->IsMovement())
			return TRUE;

	if (0.0f != m_v3Movement.x)
		return TRUE;
	if (0.0f != m_v3Movement.y)
		return TRUE;
	if (0.0f != m_v3Movement.z)
		return TRUE;

	return FALSE;
}

float CActorInstance::GetWidth()
{
	return CGraphicThingInstance::GetWidth();
}


#ifdef ENABLE_SCALE_SYSTEM_TEXTAIL
#include "RaceManager.h"
float CActorInstance::GetHeight()
{
	if (IsPC())
	{
		return CGraphicThingInstance::GetHeight();
	}

	uint32_t dwRace = GetRace();
	float fRaceHeight = CGraphicThingInstance::GetHeight();
	if (fRaceHeight == 0.0f)
	{
		fRaceHeight = CGraphicThingInstance::GetHeight(dwRace);
		CRaceManager::Instance().SetRaceHeight(dwRace, fRaceHeight);
	}

	return fRaceHeight;
}
#else
float CActorInstance::GetHeight()
{
	return CGraphicThingInstance::GetHeight();
}
#endif

bool CActorInstance::IntersectDefendingSphere()
{
	for (auto & it : m_DefendingPointInstanceList)
	{
		CDynamicSphereInstanceVector & rSphereInstanceVector = it.SphereInstanceVector;
		for (auto & rInstance : rSphereInstanceVector)
		{
			D3DXVECTOR3 v3SpherePosition = rInstance.v3Position;
			float fRadius = rInstance.fRadius;

			D3DXVECTOR3 v3Orig;
			D3DXVECTOR3 v3Dir;
			float fRange;
			ms_Ray.GetStartPoint(&v3Orig);
			ms_Ray.GetDirection(&v3Dir, &fRange);

			D3DXVECTOR3 v3Distance = v3Orig - v3SpherePosition;
			float b = D3DXVec3Dot(&v3Dir, &v3Distance);
			float c = D3DXVec3Dot(&v3Distance, &v3Distance) - fRadius * fRadius;

			if (b * b - c >= 0)
				return true;
		}
	}
	return false;
}

#ifdef ENABLE_GRAPHIC_ON_OFF
bool CActorInstance::IsShowEffects()
{
	return m_bShowEffects;
}

bool CActorInstance::IsShowActor()
{
	return m_bShowActor;
}

bool CActorInstance::IsMainInstance() const
{
	return TRUE == m_isMain;
}
#endif

bool CActorInstance::__IsMountingHorse()
{
	return nullptr != m_pkHorse;
}

void CActorInstance::MountHorse(CActorInstance * pkHorse)
{
	m_pkHorse = pkHorse;

	if (m_pkHorse)
	{
		m_pkHorse->SetCurPixelPosition(NEW_GetCurPixelPositionRef());
		m_pkHorse->SetRotation(GetRotation());
		m_pkHorse->SetAdvancingRotation(GetRotation());
	}
}

void CActorInstance::__CreateTree(const char * c_szFileName)
{
	__DestroyTree();

	CSpeedTreeForest& rkForest = CSpeedTreeForest::Instance();
	m_pkTree = rkForest.CreateInstance(m_x, m_y, m_z, GetCaseCRC32(c_szFileName, strlen(c_szFileName)), c_szFileName);
	m_pkTree->SetPosition(m_x, m_y, m_z);
	m_pkTree->UpdateBoundingSphere();
	m_pkTree->UpdateCollisionData();
}

void CActorInstance::__DestroyTree()
{
	if (!m_pkTree)
		return;

	CSpeedTreeForest::Instance().DeleteInstance(m_pkTree);
}

void CActorInstance::__SetTreePosition(float fx, float fy, float fz)
{
	if (!m_pkTree)
		return;
	if (m_x == fx && m_y == fy && m_z == fz)
		return;

	m_pkTree->SetPosition(fx, fy, fz);
	m_pkTree->UpdateBoundingSphere();
	m_pkTree->UpdateCollisionData();
}

void CActorInstance::ClearAttachingEffect()
{
	__ClearAttachingEffect();
}

void CActorInstance::Destroy()
{
	ClearFlyTargeter();

	m_HitDataMap.clear();
	m_MotionDeque.clear();

	if (m_pAttributeInstance)
	{
		m_pAttributeInstance->Clear();
		CAttributeInstance::Delete(m_pAttributeInstance);
		m_pAttributeInstance = nullptr;
	}

	__ClearAttachingEffect();

	CGraphicThingInstance::Clear();

	__DestroyWeaponTrace();
	__DestroyTree();


	__Initialize();
}

void CActorInstance::__InitializeRotationData()
{
	m_fAtkDirRot = 0.0f;
	m_fcurRotation = 0.0f;
	m_rotBegin = 0.0f;
	m_rotEnd = 0.0f;
	m_rotEndTime = 0.0f;
	m_rotBeginTime = 0.0f;
	m_rotBlendTime = 0.0f;
	m_fAdvancingRotation = 0.0f;
	m_rotX = 0.0f;
	m_rotY = 0.0f;
#ifdef ENABLE_GRAPHIC_ON_OFF
	m_bRank = 0;
	m_bShowEffects = true;
	m_bIsInvisibleActor = false;
	m_bHasPrivateShopSign = false;
	m_bShowActor = true;
	m_dwDistanceFromMainCharacter = 0;
#endif
}

void CActorInstance::__InitializeStateData()
{
	m_bEffectInitialized = false;

	m_isPreInput = FALSE;
	m_isNextPreInput = FALSE;

	m_isSleep = FALSE;
	m_isParalysis = FALSE;
	m_isFaint = FALSE;
	m_isRealDead = FALSE;
	m_isWalking = FALSE;
	m_isMain = FALSE;
	m_isStun = FALSE;
	m_isHiding = FALSE;
	m_isResistFallen = FALSE;

	m_iRenderMode = RENDER_MODE_NORMAL;
	m_fAlphaValue = 0.0f;
	m_AddColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

	m_dwMtrlColor = 0xffffffff;
	m_dwMtrlAlpha = 0xff000000;

	m_dwBattleHitEffectID = 0;
	m_dwBattleAttachEffectID = 0;
}

void CActorInstance::__InitializeMotionData()
{
	m_wcurMotionMode = CRaceMotionData::MODE_GENERAL;
	m_wcurComboType = 0;

	m_fReachScale = 1.0f;
	m_fMovSpd = 1.0f;
	m_fAtkSpd = 1.0f;

	m_fInvisibleTime = 0.0f;

	m_kSplashArea.isEnableHitProcess = TRUE;
	m_kSplashArea.uSkill = 0;
	m_kSplashArea.MotionKey = 0;
	m_kSplashArea.fDisappearingTime = 0.0f;
	m_kSplashArea.SphereInstanceVector.clear();
	m_kSplashArea.HittedInstanceMap.clear();

	m_kCurMotNode = {};

	__ClearCombo();
}

void CActorInstance::__Initialize()
{
#ifdef UCAN_ADAM
	ucanAdam_z = 0.0f;
#endif
	m_pkCurRaceMotionData = nullptr;
	m_pkCurRaceData = nullptr;
	m_pkHorse = nullptr;
	m_pkTree = nullptr;

	m_fOwnerBaseTime = 0.0f;

	m_eActorType = TYPE_PC;
	m_bIsFall = false;
	m_eRace = 0;
	m_dwRank = 0;

	m_eShape = 0;
	m_eHair = 0;

	m_dwSelfVID = 0;
	m_dwOwnerVID = 0;

	m_pkEventHandler = nullptr;

	m_PhysicsObject.Initialize();

	m_pAttributeInstance = nullptr;

	m_pFlyEventHandler = nullptr;

	m_v3FishingPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_iFishingEffectID = -1;

	m_pkHorse = nullptr;

	__InitializePositionData();
	__InitializeRotationData();
	__InitializeMotionData();
	__InitializeStateData();
	__InitializeCollisionData();

	__BlendAlpha_Initialize();

	ClearFlyTargeter();
#ifdef ENABLE_RENDER_TARGET_EFFECT
	m_isRenderTarget = false; 
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	m_bIsQuiverEquipped = false;
	m_dwQuiverEffectID = 0;
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	memset(m_dwSkillColor, 0, sizeof(m_dwSkillColor));
#endif
#ifdef ENABLE_AUTO_SYSTEM
	auto_affect = false;
#endif
}

CActorInstance::CActorInstance()
{
	__Initialize();
	m_PhysicsObject.SetActorInstance(this);
}

CActorInstance::~CActorInstance()
{
	Destroy();
}
