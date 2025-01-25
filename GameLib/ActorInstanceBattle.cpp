#include "StdAfx.h"
#include "../EffectLib/EffectManager.h"
#include "../MilesLib/SoundManager.h"

#include "ActorInstance.h"
#include "RaceData.h"
#include "GameLibDefines.h"
#ifdef ENABLE_CSHIELD
#	include "../UserInterface/CShield.h"
#endif

void CActorInstance::SetBattleHitEffect(uint32_t dwID)
{
	m_dwBattleHitEffectID = dwID;
}

void CActorInstance::SetBattleAttachEffect(uint32_t dwID)
{
	m_dwBattleAttachEffectID = dwID;
}

bool CActorInstance::CanAct()
{
	if (IsDead())
		return false;

	if (IsStun())
		return false;

	if (IsParalysis())
		return false;

	if (IsFaint())
		return false;

	if (IsSleep())
		return false;

	return true;
}

bool CActorInstance::CanUseSkill()
{
	if (!CanAct())
		return false;

	uint32_t dwCurMotionIndex = __GetCurrentMotionIndex();

	// Locked during attack
	switch (dwCurMotionIndex)
	{
		case CRaceMotionData::NAME_FISHING_THROW:
		case CRaceMotionData::NAME_FISHING_WAIT:
		case CRaceMotionData::NAME_FISHING_STOP:
		case CRaceMotionData::NAME_FISHING_REACT:
		case CRaceMotionData::NAME_FISHING_CATCH:
		case CRaceMotionData::NAME_FISHING_FAIL:
			return TRUE;
			break;
	}

	// Locked during using skill
	if (IsUsingSkill())
	{
		if (m_pkCurRaceMotionData->IsCancelEnableSkill())
			return TRUE;

		return FALSE;
	}

	return true;
}

bool CActorInstance::CanMove()
{
	if (!CanAct())
		return false;

	if (isLock())
		return false;

	return true;
}

bool CActorInstance::CanAttack()
{
	if (!CanAct())
		return false;

	if (IsUsingSkill())
	{
		if (!CanCancelSkill())
			return false;
	}

	return true;
}

bool CActorInstance::CanFishing()
{
	if (!CanAct())
		return false;

	if (IsUsingSkill())
		return false;

	switch (__GetCurrentMotionIndex())
	{
		case CRaceMotionData::NAME_WAIT:
		case CRaceMotionData::NAME_WALK:
		case CRaceMotionData::NAME_RUN:
			break;
		default:
			return false;
			break;
	}

	return true;
}

BOOL CActorInstance::IsClickableDistanceDestInstance(CActorInstance & rkInstDst, float fDistance)
{
	TPixelPosition kPPosSrc;
	GetPixelPosition(&kPPosSrc);

	D3DXVECTOR3 kD3DVct3Src(kPPosSrc);

	TCollisionPointInstanceList & rkLstkDefPtInst = rkInstDst.m_DefendingPointInstanceList;
	TCollisionPointInstanceList::iterator i;

	for (i = rkLstkDefPtInst.begin(); i != rkLstkDefPtInst.end(); ++i)
	{
		CDynamicSphereInstanceVector & rkVctkDefSphere = (*i).SphereInstanceVector;

		CDynamicSphereInstanceVector::iterator j;
		for (j = rkVctkDefSphere.begin(); j != rkVctkDefSphere.end(); ++j)
		{
			CDynamicSphereInstance & rkSphere = (*j);

			const auto vv = D3DXVECTOR3(rkSphere.v3Position - kD3DVct3Src);
			float fMovDistance = D3DXVec3Length(&vv);
			float fAtkDistance = rkSphere.fRadius + fDistance;

			if (fAtkDistance > fMovDistance)
				return TRUE;
		}
	}

	return FALSE;
}

void CActorInstance::InputNormalAttackCommand(float fDirRot)
{
	if (!__CanInputNormalAttackCommand())
		return;

	m_fAtkDirRot = fDirRot;
	NormalAttack(m_fAtkDirRot);
}

bool CActorInstance::InputComboAttackCommand(float fDirRot)
{
	m_fAtkDirRot = fDirRot;

	if (m_isPreInput)
		return false;

	/////////////////////////////////////////////////////////////////////////////////

	// Process Input
	if (0 == m_dwcurComboIndex)
	{
		__RunNextCombo();
		return true;
	}
	else if (m_pkCurRaceMotionData->IsComboInputTimeData())
	{
		float fElapsedTime = GetAttackingElapsedTime();

		if (fElapsedTime > m_pkCurRaceMotionData->GetComboInputEndTime())
		{
			//Tracen("ÀÔ·Â ÇÑ°è ½Ã°£ Áö³²");
			if (IsBowMode())
				m_isNextPreInput = TRUE;
			return false;
		}

		if (fElapsedTime > m_pkCurRaceMotionData->GetNextComboTime()) // ÄÞº¸ ¹ßµ¿ ½Ã°£ ÀÌ ÈÄ¶ó¸é
		{
			//Tracen("´ÙÀ½ ÄÞº¸ µ¿ÀÛ");
			// args : BlendingTime
			__RunNextCombo();
			return true;
		}
		else if (fElapsedTime > m_pkCurRaceMotionData->GetComboInputStartTime())
		{
			//Tracen("¼± ÀÔ·Â ¼³Á¤");
			m_isPreInput = TRUE;
			return false;
		}
	}
	else
	{
		float fElapsedTime = GetAttackingElapsedTime();
		if (fElapsedTime > m_pkCurRaceMotionData->GetMotionDuration() * 0.9f)
		{
			//Tracen("´ÙÀ½ ÄÞº¸ µ¿ÀÛ");
			// args : BlendingTime
			__RunNextCombo();
			return true;
		}
	}
	// Process Input

	return false;
}

void CActorInstance::ComboProcess()
{
	// If combo is on action
	if (0 != m_dwcurComboIndex)
	{
		if (!m_pkCurRaceMotionData)
		{
			Tracef("Attacking motion data is nullptr! : %d\n", m_dwcurComboIndex);
			__ClearCombo();
			return;
		}

		float fElapsedTime = GetAttackingElapsedTime();

		// Process PreInput
		if (m_isPreInput)
		{
			//Tracenf("¼±ÀÔ·Â %f ´ÙÀ½ÄÞº¸½Ã°£ %f", fElapsedTime, m_pkCurRaceMotionData->GetNextComboTime());
			if (fElapsedTime > m_pkCurRaceMotionData->GetNextComboTime())
			{
				__RunNextCombo();
				m_isPreInput = FALSE;

				return;
			}
		}
	}
	else
	{
		m_isPreInput = FALSE;

		if (!IsUsingSkill())
			if (m_isNextPreInput)
			{
				__RunNextCombo();
				m_isNextPreInput = FALSE;
			}
	}
}

void CActorInstance::__RunNextCombo()
{
#if defined(ENABLE_CHECK_ATTACKSPEED_HACK) && !defined(_DEBUG)
	if (!CShield::Instance().CheckAttackspeed(m_fAtkSpd, m_fReachScale, IsTwoHandMode(), CRaceMotionData::MODE_HORSE_TWOHAND_SWORD == GetMotionMode()))
	{
		hackFound = TRUE;
		CShield::Instance().Close();
	}
#endif

	++m_dwcurComboIndex;
	///////////////////////////

	uint16_t wComboIndex = m_dwcurComboIndex;
	uint16_t wComboType = __GetCurrentComboType();

	if (wComboIndex == 0)
	{
		TraceError("CActorInstance::__RunNextCombo(wComboType=%d, wComboIndex=%d)", wComboType, wComboIndex);
		return;
	}

	uint32_t dwComboArrayIndex = wComboIndex - 1;

	CRaceData::TComboData * pComboData;

	if (!m_pkCurRaceData->GetComboDataPointer(m_wcurMotionMode, wComboType, &pComboData))
	{
		TraceError("CActorInstance::__RunNextCombo(wComboType=%d, wComboIndex=%d) - "
				   "m_pkCurRaceData->GetComboDataPointer(m_wcurMotionMode=%d, &pComboData) == nullptr",
				   wComboType, wComboIndex, m_wcurMotionMode);
		return;
	}

	if (dwComboArrayIndex >= pComboData->ComboIndexVector.size())
	{
		TraceError("CActorInstance::__RunNextCombo(wComboType=%d, wComboIndex=%d) - (dwComboArrayIndex=%d) >= "
				   "(pComboData->ComboIndexVector.size()=%d)",
				   wComboType, wComboIndex, dwComboArrayIndex, pComboData->ComboIndexVector.size());
		return;
	}

	uint16_t wcurComboMotionIndex = pComboData->ComboIndexVector[dwComboArrayIndex];
	ComboAttack(wcurComboMotionIndex, m_fAtkDirRot, 0.1f);
	////////////////////////////////

	if (m_dwcurComboIndex == pComboData->ComboIndexVector.size())
		__OnEndCombo();
}

void CActorInstance::__OnEndCombo()
{
	if (__IsMountingHorse())
		m_dwcurComboIndex = 1;
}

void CActorInstance::__ClearCombo()
{
	m_dwcurComboIndex = 0;
	m_isPreInput = FALSE;
	m_pkCurRaceMotionData = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CActorInstance::isAttacking()
{
	if (isNormalAttacking())
		return TRUE;

	if (isComboAttacking())
		return TRUE;

	if (IsSplashAttacking())
		return TRUE;

	return FALSE;
}

BOOL CActorInstance::isValidAttacking()
{
	if (!m_pkCurRaceMotionData)
		return FALSE;

	if (!m_pkCurRaceMotionData->isAttackingMotion())
		return FALSE;

	const NRaceData::TMotionAttackData * c_pData = m_pkCurRaceMotionData->GetMotionAttackDataPointer();
	float fElapsedTime = GetAttackingElapsedTime();
	auto itor = c_pData->HitDataContainer.begin();
	for (; itor != c_pData->HitDataContainer.end(); ++itor)
	{
		const NRaceData::THitData & c_rHitData = *itor;
		if (fElapsedTime > c_rHitData.fAttackStartTime && fElapsedTime < c_rHitData.fAttackEndTime)
			return TRUE;
	}

	return TRUE;
}

BOOL CActorInstance::CanCheckAttacking()
{
	if (isAttacking())
		return true;

	return false;
}

bool CActorInstance::__IsInSplashTime()
{
	if (m_kSplashArea.fDisappearingTime > GetLocalTime())
		return true;

	return false;
}

BOOL CActorInstance::isNormalAttacking()
{
	if (!m_pkCurRaceMotionData)
		return FALSE;

	if (!m_pkCurRaceMotionData->isAttackingMotion())
		return FALSE;

	const NRaceData::TMotionAttackData * c_pData = m_pkCurRaceMotionData->GetMotionAttackDataPointer();
	if (NRaceData::MOTION_TYPE_NORMAL != c_pData->iMotionType)
		return FALSE;

	return TRUE;
}

BOOL CActorInstance::isComboAttacking()
{
	if (!m_pkCurRaceMotionData)
		return FALSE;

	if (!m_pkCurRaceMotionData->isAttackingMotion())
		return FALSE;

	const NRaceData::TMotionAttackData * c_pData = m_pkCurRaceMotionData->GetMotionAttackDataPointer();
	if (NRaceData::MOTION_TYPE_COMBO != c_pData->iMotionType)
		return FALSE;

	return TRUE;
}

BOOL CActorInstance::IsSplashAttacking()
{
	if (!m_pkCurRaceMotionData)
		return FALSE;

	if (m_pkCurRaceMotionData->HasSplashMotionEvent())
		return TRUE;

	return FALSE;
}

BOOL CActorInstance::__IsMovingSkill(uint16_t wSkillNumber)
{
	enum
	{
		HORSE_DASH_SKILL_NUMBER = 137
	};

	return HORSE_DASH_SKILL_NUMBER == wSkillNumber;
}

BOOL CActorInstance::IsActEmotion()
{
	uint32_t dwCurMotionIndex = __GetCurrentMotionIndex();
	switch (dwCurMotionIndex)
	{
		case CRaceMotionData::NAME_FRENCH_KISS_START + 0:
		case CRaceMotionData::NAME_FRENCH_KISS_START + 1:
		case CRaceMotionData::NAME_FRENCH_KISS_START + 2:
		case CRaceMotionData::NAME_FRENCH_KISS_START + 3:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case CRaceMotionData::NAME_FRENCH_KISS_START + 4:
#endif
		case CRaceMotionData::NAME_KISS_START + 0:
		case CRaceMotionData::NAME_KISS_START + 1:
		case CRaceMotionData::NAME_KISS_START + 2:
		case CRaceMotionData::NAME_KISS_START + 3:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case CRaceMotionData::NAME_KISS_START + 4:
#endif
			return TRUE;
			break;
	}

	return FALSE;
}

BOOL CActorInstance::IsUsingMovingSkill()
{
	return __IsMovingSkill(m_kCurMotNode.uSkill);
}

uint32_t CActorInstance::GetComboIndex()
{
	return m_dwcurComboIndex;
}

float CActorInstance::GetAttackingElapsedTime()
{
	return (GetLocalTime() - m_kCurMotNode.fStartTime) * m_kCurMotNode.fSpeedRatio;
//	return (GetLocalTime() - m_kCurMotNode.fStartTime) * __GetAttackSpeed();
}

bool CActorInstance::__CanInputNormalAttackCommand()
{
	if (IsWaiting())
		return true;

	if (isNormalAttacking())
	{
		float fElapsedTime = GetAttackingElapsedTime();

		if (fElapsedTime > m_pkCurRaceMotionData->GetMotionDuration() * 0.9f)
			return true;
	}

	return false;
}

BOOL CActorInstance::NormalAttack(float fDirRot, float fBlendTime)
{
	uint16_t wMotionIndex;
	if (!m_pkCurRaceData->GetNormalAttackIndex(m_wcurMotionMode, &wMotionIndex))
		return FALSE;

	BlendRotation(fDirRot, fBlendTime);
	SetAdvancingRotation(fDirRot);
	InterceptOnceMotion(wMotionIndex, 0.1f, 0, __GetAttackSpeed());

	__OnAttack(wMotionIndex);

	NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());

	return TRUE;
}

BOOL CActorInstance::ComboAttack(uint32_t dwMotionIndex, float fDirRot, float fBlendTime)
{
	BlendRotation(fDirRot, fBlendTime);
	SetAdvancingRotation(fDirRot);

	InterceptOnceMotion(dwMotionIndex, fBlendTime, 0, __GetAttackSpeed());

	__OnAttack(dwMotionIndex);

	NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());

	return TRUE;
}

void CActorInstance::__ProcessMotionEventAttackSuccess(uint32_t dwMotionKey, uint8_t byEventIndex, CActorInstance & rVictim)
{
	CRaceMotionData * pMotionData;

	if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &pMotionData))
		return;

	if (byEventIndex >= pMotionData->GetMotionEventDataCount())
		return;

	const CRaceMotionData::TMotionAttackingEventData * pMotionEventData;
	if (!pMotionData->GetMotionAttackingEventDataPointer(byEventIndex, &pMotionEventData))
		return;

	const D3DXVECTOR3 & c_rv3VictimPos = rVictim.GetPositionVectorRef();
	__ProcessDataAttackSuccess(pMotionEventData->AttackData, rVictim, c_rv3VictimPos);
}


void CActorInstance::__ProcessMotionAttackSuccess(uint32_t dwMotionKey, CActorInstance & rVictim)
{
	CRaceMotionData * c_pMotionData;

	if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &c_pMotionData))
		return;

	const D3DXVECTOR3 & c_rv3VictimPos = rVictim.GetPositionVectorRef();
	__ProcessDataAttackSuccess(c_pMotionData->GetMotionAttackDataReference(), rVictim, c_rv3VictimPos);
}

uint32_t CActorInstance::__GetOwnerVID()
{
	return m_dwOwnerVID;
}

float CActorInstance::__GetOwnerTime()
{
	return GetLocalTime() - m_fOwnerBaseTime;
}

bool IS_HUGE_RACE(unsigned int vnum)
{
	switch (vnum)
	{
		case 691:	// Oberork
		case 1091:	// Dämonenkönig
		case 1092:	// Stolzer Dämonenkönig
		case 1093:	// Sensenmann
		case 1094:	// Gemeiner Dämonenkönig
		case 1095:	// Blauer Tod
		case 1901:	// Neunschwanz
		case 2091:	// Königinnenspinne
		case 2092:	// Spinnenbaroness
		case 2094:	// Spinnenbaron
		case 2291:	// Roter Drache
		case 2491:	// Hauptmann Yonghan
		case 2492:	// General Yonghan
		case 2493:	// Beran-Setaou
		case 2496:	// unused
		case 2591:	// Tartaros
		case 2598:	// Azrael
		case 3190:	// Arges
		case 3191:	// Polyphemos
		case 3290:	// Rakshasa
		case 3391:	// Lemurische Leibgarde
		case 3390:	// Lemuren - Fürst
		case 3490:	// General Kappa
		case 3491:	// Triton
		case 3590:	// Knochengesicht
		case 3591:	// Roter Häuptling
		case 3595:	// Brutales Knochengesicht
		case 3596:	// Brutaler Roter Häuptling
		case 3690:	// General Lobster
		case 3790:	// Gargoyle
		case 3791:	// König Wobba
		case 3890:	// Kapitän Shrack
		case 3891:	// Der Große Oger
		case 3911:	// Aku - Boku
		case 3912:	// Arboretum
		case 3913:	// Sycomor
		case 3960:	// Hydra1
		case 3961:	// Hydra2
		case 3962:	// Hydra3
		case 4101:	// Endzeit - Wächter Zi
		case 4121:	// Endzeit - Wächter Zi
		case 6191:	// Nemere
		case 6091:	// Razador
			return true;
	}

#ifdef ENABLE_12ZI
	if (vnum >= 2750 && vnum <= 2862)
		return true;
#endif

	return false;
}

bool CActorInstance::__CanPushDestActor(CActorInstance & rkActorDst)
{
	if (rkActorDst.IsBuilding())
		return false;

	if (rkActorDst.IsDoor())
		return false;

	if (rkActorDst.IsStone())
		return false;

	if (rkActorDst.IsNPC())
		return false;

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	if (rkActorDst.IsShop())
		return false;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (rkActorDst.IsGrowthPet())
		return false;
#endif

#ifdef ENABLE_PROTO_RENEWAL
	if (rkActorDst.IsPetPay())
		return false;

	if (rkActorDst.IsHorse())
		return false;
#endif

	if (IS_HUGE_RACE(rkActorDst.GetRace()) || rkActorDst.GetActorFallState())
		return false;

	if (rkActorDst.IsStun())
		return true;

	if (rkActorDst.__GetOwnerVID() != GetVirtualID())
		return false;

	if (rkActorDst.__GetOwnerTime() > 3.0f)
		return false;

	return true;
}

bool IS_PARTY_HUNTING_RACE(unsigned int vnum)
{
	return true;

	/*
	if (vnum < 8)
		return true;

	if (vnum >= 8000 && vnum <= 8112)
		return true;

	if (vnum >= 2400 && vnum <  5000)
		return true;

	return false;
	*/
}

void CActorInstance::__ProcessDataAttackSuccess(const NRaceData::TAttackData & c_rAttackData, CActorInstance & rVictim, const D3DXVECTOR3 & c_rv3Position, uint32_t uiSkill, BOOL isSendPacket)
{
	if (NRaceData::HIT_TYPE_NONE == c_rAttackData.iHittingType)
		return;

	InsertDelay(c_rAttackData.fStiffenTime);

	if (__CanPushDestActor(rVictim) && c_rAttackData.fExternalForce > 0.0f)
	{
		__PushCircle(rVictim);

		// VICTIM_COLLISION_TEST
		const D3DXVECTOR3 & kVictimPos = rVictim.GetPosition();
		rVictim.m_PhysicsObject.IncreaseExternalForce(kVictimPos, c_rAttackData.fExternalForce); //*nForceRatio/100.0f);

		// VICTIM_COLLISION_TEST_END
	}

	// Invisible Time
	if (IS_PARTY_HUNTING_RACE(rVictim.GetRace()))
	{
		if (uiSkill)
			rVictim.m_fInvisibleTime = CTimer::Instance().GetCurrentSecond() + c_rAttackData.fInvisibleTime;

		if (m_isMain)
			rVictim.m_fInvisibleTime = CTimer::Instance().GetCurrentSecond() + c_rAttackData.fInvisibleTime;
	}
	else
		rVictim.m_fInvisibleTime = CTimer::Instance().GetCurrentSecond() + c_rAttackData.fInvisibleTime;

	// Stiffen Time
	rVictim.InsertDelay(c_rAttackData.fStiffenTime);

	// Hit Effect
	D3DXVECTOR3 vec3Effect(rVictim.m_x, rVictim.m_y, rVictim.m_z);

	if (IS_HUGE_RACE(rVictim.GetRace()))
		vec3Effect = c_rv3Position;

	const D3DXVECTOR3 & v3Pos = GetPosition();

	float fHeight = D3DXToDegree(atan2(-vec3Effect.x + v3Pos.x, +vec3Effect.y - v3Pos.y));

	if (rVictim.IsBuilding() || rVictim.IsDoor())
	{
		D3DXVECTOR3 vec3Delta = vec3Effect - v3Pos;
		D3DXVec3Normalize(&vec3Delta, &vec3Delta);
		vec3Delta *= 30.0f;

		CEffectManager & rkEftMgr = CEffectManager::Instance();
		if (m_dwBattleHitEffectID)
			rkEftMgr.CreateEffect(m_dwBattleHitEffectID, v3Pos + vec3Delta, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	}
#ifdef ENABLE_GRAPHIC_ON_OFF
	else if (m_bShowEffects)
#else
	else
#endif
	{
		CEffectManager & rkEftMgr = CEffectManager::Instance();
		if (m_dwBattleHitEffectID)
			rkEftMgr.CreateEffect(m_dwBattleHitEffectID, vec3Effect, D3DXVECTOR3(0.0f, 0.0f, fHeight));
		if (m_dwBattleAttachEffectID)
			rVictim.AttachEffectByID(0, nullptr, m_dwBattleAttachEffectID);
	}

	if (rVictim.IsBuilding())
	{
		// 2004.08.03.ºôµùÀÇ °æ¿ì Èçµé¸®¸é ÀÌ»óÇÏ´Ù
	}
	else if (rVictim.IsStone() || rVictim.IsDoor())
		__HitStone(rVictim);
	else
	{
		///////////
		// Motion
		if (NRaceData::HIT_TYPE_GOOD == c_rAttackData.iHittingType || rVictim.IsResistFallen())
			__HitGood(rVictim);
		else if (NRaceData::HIT_TYPE_GREAT == c_rAttackData.iHittingType)
			__HitGreate(rVictim);
		else
			TraceError("ProcessSucceedingAttacking: Unknown AttackingData.iHittingType %d", c_rAttackData.iHittingType);
	}

	__OnHit(uiSkill, rVictim, isSendPacket);
}

void CActorInstance::OnShootDamage()
{
	if (IsStun())
		Die();
	else
	{
		__Shake(100);

		if (!isLock() && !__IsKnockDownMotion() && !__IsStandUpMotion())
		{
			if (InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE))
				PushLoopMotion(CRaceMotionData::NAME_WAIT);
		}
	}
}

void CActorInstance::__Shake(uint32_t dwDuration)
{
	uint32_t dwCurTime = ELTimer_GetMSec();
	m_dwShakeTime = dwCurTime + dwDuration;
}

void CActorInstance::ShakeProcess()
{
	if (m_dwShakeTime)
	{
		D3DXVECTOR3 v3Pos(0.0f, 0.0f, 0.0f);

		uint32_t dwCurTime = ELTimer_GetMSec();

		if (m_dwShakeTime < dwCurTime)
			m_dwShakeTime = 0;
		else
		{
			int nShakeSize = 10;

			switch (rand() % 2)
			{
				case 0:
					v3Pos.x += rand() % nShakeSize;
					break;
				case 1:
					v3Pos.x -= rand() % nShakeSize;
					break;
			}

			switch (rand() % 2)
			{
				case 0:
					v3Pos.y += rand() % nShakeSize;
					break;
				case 1:
					v3Pos.y -= rand() % nShakeSize;
					break;
			}

			switch (rand() % 2)
			{
				case 0:
					v3Pos.z += rand() % nShakeSize;
					break;
				case 1:
					v3Pos.z -= rand() % nShakeSize;
					break;
			}
		}

		m_worldMatrix._41 += v3Pos.x;
		m_worldMatrix._42 += v3Pos.y;
		m_worldMatrix._43 += v3Pos.z;
	}
}

void CActorInstance::__HitStone(CActorInstance & rVictim)
{
	if (rVictim.IsStun())
		rVictim.Die();
	else
		rVictim.__Shake(100);
}

void CActorInstance::__HitGood(CActorInstance & rVictim)
{
	if (rVictim.IsKnockDown())
		return;

	if (rVictim.IsStun())
		rVictim.Die();
	else
	{
		rVictim.__Shake(100);

		if (!rVictim.isLock())
		{
			float fRotRad = D3DXToRadian(GetRotation());
			float fVictimRotRad = D3DXToRadian(rVictim.GetRotation());

			D3DXVECTOR2 v2Normal(sin(fRotRad), cos(fRotRad));
			D3DXVECTOR2 v2VictimNormal(sin(fVictimRotRad), cos(fVictimRotRad));

			D3DXVec2Normalize(&v2Normal, &v2Normal);
			D3DXVec2Normalize(&v2VictimNormal, &v2VictimNormal);

			float fScalar = D3DXVec2Dot(&v2Normal, &v2VictimNormal);

			if (fScalar < 0.0f)
			{
				if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE))
					rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
			}
			else
			{
				if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_BACK))
					rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
				else if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE))
					rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
			}
		}
	}
}

void CActorInstance::__HitGreate(CActorInstance & rVictim)
{
	// DISABLE_KNOCKDOWN_ATTACK
	if (rVictim.IsKnockDown())
		return;
	if (rVictim.__IsStandUpMotion())
		return;
	// END_OF_DISABLE_KNOCKDOWN_ATTACK

	float fRotRad = D3DXToRadian(GetRotation());
	float fVictimRotRad = D3DXToRadian(rVictim.GetRotation());

	D3DXVECTOR2 v2Normal(sin(fRotRad), cos(fRotRad));
	D3DXVECTOR2 v2VictimNormal(sin(fVictimRotRad), cos(fVictimRotRad));

	D3DXVec2Normalize(&v2Normal, &v2Normal);
	D3DXVec2Normalize(&v2VictimNormal, &v2VictimNormal);

	float fScalar = D3DXVec2Dot(&v2Normal, &v2VictimNormal);

	rVictim.__Shake(100);

	if (rVictim.IsUsingSkill())
		return;

	if (rVictim.IsStun())
	{
		if (fScalar < 0.0f)
			rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING);
		else
		{
			if (!rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING_BACK))
				rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING);
		}

		rVictim.m_isRealDead = true;
	}
	else
	{
		if (fScalar < 0.0f)
		{
			if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING))
			{
				rVictim.PushOnceMotion(CRaceMotionData::NAME_STAND_UP);
				rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
			}
		}
		else
		{
			if (!rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING_BACK))
			{
				if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING))
				{
					rVictim.PushOnceMotion(CRaceMotionData::NAME_STAND_UP);
					rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
				}
			}
			else
			{
				rVictim.PushOnceMotion(CRaceMotionData::NAME_STAND_UP_BACK);
				rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
			}
		}
	}
}

void CActorInstance::SetBlendingPosition(const TPixelPosition & c_rPosition, float fBlendingTime)
{
	//return;
	TPixelPosition Position;

	Position.x = c_rPosition.x - m_x;
	Position.y = c_rPosition.y - m_y;
	Position.z = 0;

	m_PhysicsObject.SetLastPosition(Position, fBlendingTime);
}

void CActorInstance::ResetBlendingPosition()
{
	m_PhysicsObject.Initialize();
}

void CActorInstance::GetBlendingPosition(TPixelPosition * pPosition)
{
	if (m_PhysicsObject.isBlending())
	{
		m_PhysicsObject.GetLastPosition(pPosition);
		pPosition->x += m_x;
		pPosition->y += m_y;
		pPosition->z += m_z;
	}
	else
	{
		pPosition->x = m_x;
		pPosition->y = m_y;
		pPosition->z = m_z;
	}
}

void CActorInstance::__PushCircle(CActorInstance & rVictim)
{
	const TPixelPosition & c_rkPPosAtk = NEW_GetAtkPixelPositionRef();

	D3DXVECTOR3 v3SrcPos(c_rkPPosAtk.x, -c_rkPPosAtk.y, c_rkPPosAtk.z);

	const D3DXVECTOR3 & c_rv3SrcPos = v3SrcPos;
	const D3DXVECTOR3 & c_rv3DstPos = rVictim.GetPosition();

	D3DXVECTOR3 v3Direction;
	v3Direction.x = c_rv3DstPos.x - c_rv3SrcPos.x;
	v3Direction.y = c_rv3DstPos.y - c_rv3SrcPos.y;
	v3Direction.z = 0.0f;
	D3DXVec3Normalize(&v3Direction, &v3Direction);

	rVictim.__SetFallingDirection(v3Direction.x, v3Direction.y);
}

void CActorInstance::__PushDirect(CActorInstance & rVictim)
{
	D3DXVECTOR3 v3Direction;
	v3Direction.x = cosf(D3DXToRadian(m_fcurRotation + 270.0f));
	v3Direction.y = sinf(D3DXToRadian(m_fcurRotation + 270.0f));
	v3Direction.z = 0.0f;

	rVictim.__SetFallingDirection(v3Direction.x, v3Direction.y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CActorInstance::__isInvisible()
{
	if (IsDead())
		return true;

	if (CTimer::Instance().GetCurrentSecond() >= m_fInvisibleTime)
		return false;

	return true;
}

void CActorInstance::__SetFallingDirection(float fx, float fy)
{
	m_PhysicsObject.SetDirection(D3DXVECTOR3(fx, fy, 0.0f));
}

//@fixme427
float CActorInstance::__GetInvisibleTimeAdjust(const uint32_t uiSkill, const NRaceData::TAttackData& c_rAttackData)
{
	static const int shamanw = 3, shamanm = 7;

	if ((GetRace() != shamanw && GetRace() != shamanm) || uiSkill != 0 || m_fAtkSpd < 1.3)
		return 0.0f;

	const auto scale = (m_fAtkSpd - 1.3) / 1.3;
	const auto inv = c_rAttackData.fInvisibleTime * 0.5;
	return inv * scale;
}
