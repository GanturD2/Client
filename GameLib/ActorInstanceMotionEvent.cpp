#include "StdAfx.h"
#include "../EffectLib/EffectManager.h"
#include "../MilesLib/SoundManager.h"

#include "ActorInstance.h"
#include "FlyingObjectManager.h"
#include "FlyingInstance.h"
#include "GameEventManager.h"

#include "FlyHandler.h"
#include "GameLibDefines.h"

void CActorInstance::MotionEventProcess()
{
	if (!m_pkCurRaceMotionData)
		return;

	for (uint32_t i = 0; i < m_pkCurRaceMotionData->GetMotionEventDataCount(); ++i)
	{
		const CRaceMotionData::TMotionEventData * c_pData;
		if (!m_pkCurRaceMotionData->GetMotionEventDataPointer(i, &c_pData))
			continue;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
		if (m_kCurMotNode.uSkill == 0)
			MotionEventProcess(m_kCurMotNode.dwcurFrame, i, c_pData, GetSkillColorByEffectID(m_kCurMotNode.dwMotionKey));
		else
			MotionEventProcess(m_kCurMotNode.dwcurFrame, i, c_pData, m_dwSkillColor[m_kCurMotNode.uSkill]);
#else
		MotionEventProcess(m_kCurMotNode.dwcurFrame, i, c_pData);
#endif
	}
}

void CActorInstance::SoundEventProcess(BOOL bCheckFrequency)
{
	if (!m_pkCurRaceMotionData)
		return;

	CSoundManager & rkSndMgr = CSoundManager::Instance();
	const NSound::TSoundInstanceVector * c_pkVct_kSndInst = m_pkCurRaceMotionData->GetSoundInstanceVectorPointer();
	rkSndMgr.UpdateSoundInstance(m_x, m_y, m_z, m_kCurMotNode.dwcurFrame, c_pkVct_kSndInst, bCheckFrequency);
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CActorInstance::MotionEventProcess(uint32_t dwcurFrame, int iIndex, const CRaceMotionData::TMotionEventData* c_pData, uint32_t* dwSkillColor)
#else
void CActorInstance::MotionEventProcess(uint32_t dwcurFrame, int iIndex, const CRaceMotionData::TMotionEventData * c_pData)
#endif
{
	if (c_pData->dwFrame != dwcurFrame)
		return;

	switch (c_pData->iType)
	{
		case CRaceMotionData::MOTION_EVENT_TYPE_EFFECT:
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			ProcessMotionEventEffectEvent(c_pData, dwSkillColor);
#else
			ProcessMotionEventEffectEvent(c_pData);
#endif
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_EFFECT_TO_TARGET:
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			ProcessMotionEventEffectToTargetEvent(c_pData, dwSkillColor);
#else
			ProcessMotionEventEffectToTargetEvent(c_pData);
#endif
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_SCREEN_WAVING:
			CGameEventManager::Instance().ProcessEventScreenWaving(this,
																   msl::inherit_cast<const CRaceMotionData::TScreenWavingEventData *>(c_pData));
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_SPECIAL_ATTACKING:
			ProcessMotionEventSpecialAttacking(iIndex, c_pData);
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_SOUND:
			ProcessMotionEventSound(c_pData);
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_FLY:
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			ProcessMotionEventFly(c_pData, dwSkillColor);
#else
			ProcessMotionEventFly(c_pData);
#endif
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_CHARACTER_SHOW:
			__ShowEvent();
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_CHARACTER_HIDE:
			__HideEvent();
			break;

		case CRaceMotionData::MOTION_EVENT_TYPE_WARP:
#ifndef WORLD_EDITOR
			ProcessMotionEventWarp(c_pData);
#endif
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER
		case CRaceMotionData::MOTION_EVENT_TYPE_UNK11:
#	ifndef WORLD_EDITOR
			ProcessMotionEventUnk11(dwcurFrame, iIndex, c_pData);
#	endif
			break;
		case CRaceMotionData::MOTION_EVENT_TYPE_UNK12:
#	ifndef WORLD_EDITOR
			ProcessMotionEventUnk12(dwcurFrame, iIndex, c_pData);
#	endif
			break;
#endif
	}
}

void CActorInstance::__ShowEvent()
{
	m_isHiding = FALSE;
	RestoreRenderMode();
	SetAlphaValue(1.0f);
}

void CActorInstance::__HideEvent()
{
	m_isHiding = TRUE;
	SetBlendRenderMode();
	SetAlphaValue(0.0f);
}

BOOL CActorInstance::__IsHiding()
{
	return m_isHiding;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CActorInstance::ProcessMotionEventEffectEvent(const CRaceMotionData::TMotionEventData* c_pData, uint32_t* dwSkillColor)
#else
void CActorInstance::ProcessMotionEventEffectEvent(const CRaceMotionData::TMotionEventData * c_pData)
#endif
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!m_bShowEffects || !m_bShowActor)
		return;
#endif

	if (CRaceMotionData::MOTION_EVENT_TYPE_EFFECT != c_pData->iType)
		return;

	const auto * c_pEffectData = msl::inherit_cast<const CRaceMotionData::TMotionEffectEventData *>(c_pData);
#ifdef ENABLE_SCALE_SYSTEM
	const D3DXVECTOR3 c_v3Scale = GetScale();
#endif

	if (c_pEffectData->isIndependent)
	{
		int iIndex = CEffectManager::Instance().CreateEffect(c_pEffectData->dwEffectIndex, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f)
#ifdef ENABLE_SCALE_SYSTEM
			, c_v3Scale.x
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			, dwSkillColor
#endif
		);

		D3DXMATRIX matLocalPosition;
		D3DXMatrixTranslation(&matLocalPosition, c_pEffectData->v3EffectPosition.x, c_pEffectData->v3EffectPosition.y,
							  c_pEffectData->v3EffectPosition.z);

		D3DXMATRIX matWorld;
		matWorld = matLocalPosition;
		matWorld *= m_worldMatrix;

		CEffectManager::Instance().SelectEffectInstance(iIndex);
		CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matWorld);
		return;
	}

	if (c_pEffectData->isAttaching)
	{
		if (c_pEffectData->isFollowing)
		{
			AttachEffectByID(0, c_pEffectData->strAttachingBoneName.c_str(), c_pEffectData->dwEffectIndex,
				&c_pEffectData->v3EffectPosition
#ifdef ENABLE_SCALE_SYSTEM
				, c_v3Scale.x, & c_v3Scale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				, dwSkillColor
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
				, 0, FALSE
#endif
			);
		}
		else
		{
			int iBoneIndex;
			uint32_t dwPartIndex = 0;
			if (FindBoneIndex(dwPartIndex, c_pEffectData->strAttachingBoneName.c_str(), &iBoneIndex))
			{
				D3DXMATRIX * pBoneMat;
				GetBoneMatrix(dwPartIndex, iBoneIndex, &pBoneMat);

				D3DXMATRIX matLocalPosition;
				D3DXMatrixTranslation(&matLocalPosition, c_pEffectData->v3EffectPosition.x, c_pEffectData->v3EffectPosition.y,
									  c_pEffectData->v3EffectPosition.z);

				/////////////////////////////////////////////////////////////////////
				D3DXMATRIX matWorld;
				matWorld = *pBoneMat;
				matWorld *= matLocalPosition;
				matWorld *= m_worldMatrix;
				/////////////////////////////////////////////////////////////////////

				int iIndex = CEffectManager::Instance().CreateEffect(c_pEffectData->dwEffectIndex, c_pEffectData->v3EffectPosition,
					D3DXVECTOR3(0.0f, 0.0f, 0.0f)
#ifdef ENABLE_SCALE_SYSTEM
					, 1.0f
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
					, dwSkillColor
#endif
				);

				CEffectManager::Instance().SelectEffectInstance(iIndex);
				CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matWorld);
			}
		}
	}
	else
		AttachEffectByID(0, nullptr, c_pEffectData->dwEffectIndex, &c_pEffectData->v3EffectPosition
#ifdef ENABLE_SCALE_SYSTEM
			, c_v3Scale.x, & c_v3Scale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			, dwSkillColor
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
			, 0, FALSE
#endif
		);
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CActorInstance::ProcessMotionEventEffectToTargetEvent(const CRaceMotionData::TMotionEventData* c_pData, uint32_t* dwSkillColor)
#else
void CActorInstance::ProcessMotionEventEffectToTargetEvent(const CRaceMotionData::TMotionEventData * c_pData)
#endif
{
#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!m_bShowEffects || !m_bShowActor)
		return;
#endif

	if (CRaceMotionData::MOTION_EVENT_TYPE_EFFECT_TO_TARGET != c_pData->iType)
		return;

	const auto * c_pEffectToTargetData = msl::inherit_cast<const CRaceMotionData::TMotionEffectToTargetEventData *>(c_pData);

	if (c_pEffectToTargetData->isFishingEffect)
	{
		CEffectManager & rkEftMgr = CEffectManager::Instance();

		if (-1 != m_iFishingEffectID)
			rkEftMgr.DeactiveEffectInstance(m_iFishingEffectID);

		m_iFishingEffectID =
			rkEftMgr.CreateEffect(c_pEffectToTargetData->dwEffectIndex, m_v3FishingPosition, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	}
	else
	{
		if (!m_kFlyTarget.IsValidTarget())
			return;

		if (c_pEffectToTargetData->isFollowing && IsFlyTargetObject())
		{
			auto * pTargetInstance = msl::inherit_cast<CActorInstance *>(m_kFlyTarget.GetFlyTarget());
			D3DXVECTOR3 v3Position(c_pEffectToTargetData->v3EffectPosition.x, c_pEffectToTargetData->v3EffectPosition.y,
								   c_pEffectToTargetData->v3EffectPosition.z);
			pTargetInstance->AttachEffectByID(0, nullptr, c_pEffectToTargetData->dwEffectIndex, &v3Position
#ifdef ENABLE_SCALE_SYSTEM
				, 1.0f, (const D3DXVECTOR3*)nullptr
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				, dwSkillColor
#endif
			);
		}
		else
		{
			const D3DXVECTOR3 & c_rv3FlyTarget = m_kFlyTarget.GetFlyTargetPosition();
			D3DXVECTOR3 v3Position(c_rv3FlyTarget.x + c_pEffectToTargetData->v3EffectPosition.x,
								   c_rv3FlyTarget.y + c_pEffectToTargetData->v3EffectPosition.y,
								   c_rv3FlyTarget.z + c_pEffectToTargetData->v3EffectPosition.z);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
#	ifdef ENABLE_SCALE_SYSTEM
			CEffectManager::Instance().CreateEffect(c_pEffectToTargetData->dwEffectIndex, v3Position, D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1.0f, dwSkillColor);
#	else
			CEffectManager::Instance().CreateEffect(c_pEffectToTargetData->dwEffectIndex, v3Position, D3DXVECTOR3(0.0f, 0.0f, 0.0f), dwSkillColor);
#	endif
#else
#	ifdef ENABLE_SCALE_SYSTEM
			CEffectManager::Instance().CreateEffect(c_pEffectToTargetData->dwEffectIndex, v3Position, D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1.0f);
#	else
			CEffectManager::Instance().CreateEffect(c_pEffectToTargetData->dwEffectIndex, v3Position, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
#	endif
#endif
		}
	}
}

void CActorInstance::ProcessMotionEventSpecialAttacking(int iMotionEventIndex, const CRaceMotionData::TMotionEventData * c_pData)
{
	if (CRaceMotionData::MOTION_EVENT_TYPE_SPECIAL_ATTACKING != c_pData->iType)
		return;

	const auto * c_pAttackingData = msl::inherit_cast<const CRaceMotionData::TMotionAttackingEventData *>(c_pData);

	float fRadian = D3DXToRadian(270.0f + 360.0f - GetRotation());
	m_kSplashArea.isEnableHitProcess = c_pAttackingData->isEnableHitProcess;
	m_kSplashArea.uSkill = m_kCurMotNode.uSkill;
	m_kSplashArea.MotionKey = m_kCurMotNode.dwMotionKey;
	m_kSplashArea.fDisappearingTime = GetLocalTime() + c_pAttackingData->fDurationTime;
	m_kSplashArea.c_pAttackingEvent = c_pAttackingData;
	m_kSplashArea.HittedInstanceMap.clear();

	m_kSplashArea.SphereInstanceVector.clear();
	m_kSplashArea.SphereInstanceVector.resize(c_pAttackingData->CollisionData.SphereDataVector.size());
	for (uint32_t i = 0; i < c_pAttackingData->CollisionData.SphereDataVector.size(); ++i)
	{
		const TSphereData & c_rSphereData = c_pAttackingData->CollisionData.SphereDataVector[i].GetAttribute();
		CDynamicSphereInstance & rSphereInstance = m_kSplashArea.SphereInstanceVector[i];

		rSphereInstance.fRadius = c_rSphereData.fRadius;
		//rSphereInstance.v3Advance = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		rSphereInstance.v3Position.x = m_x + c_rSphereData.v3Position.x * sinf(fRadian) + c_rSphereData.v3Position.y * cosf(fRadian);
		rSphereInstance.v3Position.y = m_y + c_rSphereData.v3Position.x * cosf(fRadian) - c_rSphereData.v3Position.y * sinf(fRadian);
		rSphereInstance.v3Position.z = m_z + c_rSphereData.v3Position.z;
		rSphereInstance.v3LastPosition = rSphereInstance.v3Position;
	}
}

void CActorInstance::ProcessMotionEventSound(const CRaceMotionData::TMotionEventData * c_pData)
{
	if (CRaceMotionData::MOTION_EVENT_TYPE_SOUND != c_pData->iType)
		return;

	const auto * c_pSoundData = msl::inherit_cast<const CRaceMotionData::TMotionSoundEventData *>(c_pData);

	Tracenf("PLAY SOUND: %s", c_pSoundData->strSoundFileName.c_str());
	CSoundManager::Instance().PlaySound3D(m_x, m_y, m_z, c_pSoundData->strSoundFileName.c_str());
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
void CActorInstance::ProcessMotionEventFly(const CRaceMotionData::TMotionEventData* c_pData, uint32_t* dwSkillColor)
#else
void CActorInstance::ProcessMotionEventFly(const CRaceMotionData::TMotionEventData * c_pData)
#endif
{
	if (CRaceMotionData::MOTION_EVENT_TYPE_FLY != c_pData->iType)
		return;

	const auto * c_pFlyData = msl::inherit_cast<const CRaceMotionData::TMotionFlyEventData *>(c_pData);

	if (m_kFlyTarget.IsValidTarget())
	{
		CFlyingManager & rfm = CFlyingManager::Instance();

		D3DXVECTOR3 v3Start(c_pFlyData->v3FlyPosition);
		v3Start += m_v3Position;

		if (c_pFlyData->isAttaching)
		{
			D3DXMATRIX * pBoneMat;
			int iBoneIndex;
			uint32_t dwPartIndex = 0;

			if (FindBoneIndex(dwPartIndex, c_pFlyData->strAttachingBoneName.c_str(), &iBoneIndex))
				if (GetBoneMatrix(dwPartIndex, iBoneIndex, &pBoneMat))
				{
					v3Start.x += pBoneMat->_41;
					v3Start.y += pBoneMat->_42;
					v3Start.z += pBoneMat->_43;
				}
		}

#ifdef ENABLE_QUIVER_SYSTEM
		CFlyingInstance * pInstance = nullptr;
		if (m_eRace == CRaceData::RACE_ASSASSIN_W || m_eRace == CRaceData::RACE_ASSASSIN_M)
		{
			CRaceMotionData* pRaceMotionData = m_pkCurRaceMotionData;
			if (!pRaceMotionData)
				return;

			const NRaceData::TMotionAttackData * c_pData = m_pkCurRaceMotionData->GetMotionAttackDataPointer();
			if (NRaceData::MOTION_TYPE_NORMAL == c_pData->iMotionType)
			{
				if (IsEqippedQuiver())
					pInstance = rfm.CreateIndexedFlyingInstanceFlyTarget(GetQuiverEffectID(), v3Start, m_kFlyTarget);
				else
#ifdef ENABLE_SKILL_COLOR_SYSTEM
					pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true, dwSkillColor);
#else
					pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
#endif
			}
			else if (__IsMountingHorse())
			{
				if (IsEqippedQuiver())
					pInstance = rfm.CreateIndexedFlyingInstanceFlyTarget(GetQuiverEffectID(), v3Start, m_kFlyTarget);
				else
#ifdef ENABLE_SKILL_COLOR_SYSTEM
					pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true, dwSkillColor);
#else
					pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
#endif
			}
			else
#ifdef ENABLE_SKILL_COLOR_SYSTEM
				pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true, dwSkillColor);
#else
				pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
#endif
		}
		else
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true, dwSkillColor);
#else
			pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex, v3Start, m_kFlyTarget, true);
#endif
#else
		CFlyingInstance * pInstance = rfm.CreateFlyingInstanceFlyTarget(c_pFlyData->dwFlyIndex,v3Start,m_kFlyTarget,true);
#endif
		if (pInstance)
		{
			pInstance->SetEventHandler(m_pFlyEventHandler);
			pInstance->SetOwner(this);
#ifdef ENABLE_GRAPHIC_ON_OFF
			pInstance->SetOwner2(this);
#endif
			pInstance->SetSkillIndex(m_kCurMotNode.uSkill);
		}

		if (m_pFlyEventHandler)
			m_pFlyEventHandler->OnShoot(m_kCurMotNode.uSkill);
	}
	else
	{
		//TraceError("ActorInstance::ProcessMotionEventFly No Target");
	}
}

void CActorInstance::ProcessMotionEventWarp(const CRaceMotionData::TMotionEventData * c_pData)
{
	if (CRaceMotionData::MOTION_EVENT_TYPE_WARP != c_pData->iType)
		return;

	// FIXME : TMotionWarpEventData로 뺄 변수 - [levites]
	static const float sc_fDistanceFromTarget = 270.0f;

	if (m_kFlyTarget.IsValidTarget())
	{
		D3DXVECTOR3 v3MainPosition(m_x, m_y, m_z);
		const D3DXVECTOR3 & c_rv3TargetPosition = __GetFlyTargetPosition();

		D3DXVECTOR3 v3Distance = c_rv3TargetPosition - v3MainPosition;
		D3DXVec3Normalize(&v3Distance, &v3Distance);
		TPixelPosition DestPixelPosition = c_rv3TargetPosition - (v3Distance * sc_fDistanceFromTarget);

		// 2004.07.05.myevan.궁신탄영 맵에 끼이는 문제해결. 목표위치가 이동 못하는 곳일 경우 이동하지 않는다
		IBackground & rkBG = GetBackground();
		if (!rkBG.IsBlock(DestPixelPosition.x, -DestPixelPosition.y))
			SetPixelPosition(DestPixelPosition);

		LookAt(c_rv3TargetPosition.x, c_rv3TargetPosition.y);

		__OnWarp();
	}
	else
	{
		//TraceError("ActorInstance::ProcessMotionEventFly No Target");
	}
}

#ifdef ENABLE_WOLFMAN_CHARACTER
void CActorInstance::ProcessMotionEventUnk11(uint32_t dwcurFrame, int iIndex,
											 const CRaceMotionData::TMotionEventData * c_pData) // AniSpeed ON
{
	if (CRaceMotionData::MOTION_EVENT_TYPE_UNK11 != c_pData->iType)
		return;

	const auto * c_pUnk11TargetData = msl::inherit_cast<const CRaceMotionData::TMotionUnk11EventData *>(c_pData);

	// workaround
	{
		static const float sc_fDistanceFromTarget = 270.0f;

		if (m_kFlyTarget.IsValidTarget())
		{
			D3DXVECTOR3 v3MainPosition(m_x, m_y, m_z);
			const D3DXVECTOR3 & c_rv3TargetPosition = __GetFlyTargetPosition();

			D3DXVECTOR3 v3Distance = c_rv3TargetPosition - v3MainPosition;
			D3DXVec3Normalize(&v3Distance, &v3Distance);
			TPixelPosition DestPixelPosition =
				c_rv3TargetPosition - (v3Distance * (sc_fDistanceFromTarget + c_pUnk11TargetData->iAniSpeed));

			// 2004.07.05.myevan.궁신탄영 맵에 끼이는 문제해결. 목표위치가 이동 못하는 곳일 경우 이동하지 않는다
			IBackground & rkBG = GetBackground();
			if (!rkBG.IsBlock(DestPixelPosition.x, -DestPixelPosition.y))
				SetPixelPosition(DestPixelPosition);

			LookAt(c_rv3TargetPosition.x, c_rv3TargetPosition.y);

			__OnWarp();
		}
	}
}

void CActorInstance::ProcessMotionEventUnk12(uint32_t dwcurFrame, int iIndex,
											 const CRaceMotionData::TMotionEventData * c_pData) // AniSpeed OFF
{
	if (CRaceMotionData::MOTION_EVENT_TYPE_UNK12 != c_pData->iType)
		return;

	const auto * c_pUnk12TargetData = msl::inherit_cast<const CRaceMotionData::TMotionUnk12EventData *>(c_pData);

	// workaround
	{
	}
}
#endif
