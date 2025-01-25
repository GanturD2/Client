#include "StdAfx.h"
#include "../EffectLib/EffectManager.h"

#include "ActorInstance.h"
#include "ItemData.h"
#include "ItemManager.h"
#include "RaceData.h"
#include "WeaponTrace.h"
#include "GameLibDefines.h"
#ifdef ENABLE_GRAPHIC_ON_OFF
#	include "../UserInterface/InstanceBase.h"
#endif

BOOL USE_WEAPON_SPECULAR = TRUE;

uint32_t CActorInstance::AttachSmokeEffect(uint32_t eSmoke)
{
	if (!m_pkCurRaceData)
		return 0;

	uint32_t dwSmokeEffectID = m_pkCurRaceData->GetSmokeEffectID(eSmoke);

	return AttachEffectByID(0, m_pkCurRaceData->GetSmokeBone().c_str(), dwSmokeEffectID);
}

bool CActorInstance::__IsLeftHandWeapon(uint32_t type)
{
	if (CItemData::WEAPON_DAGGER == type || (CItemData::WEAPON_FAN == type && __IsMountingHorse()))
		return true;
	else if (CItemData::WEAPON_BOW == type)
		return true;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (CItemData::WEAPON_CLAW == type)
		return true;
#endif
	else
		return false;
}

bool CActorInstance::__IsRightHandWeapon(uint32_t type)
{
	if (CItemData::WEAPON_DAGGER == type || (CItemData::WEAPON_FAN == type && __IsMountingHorse()))
		return true;
	else if (CItemData::WEAPON_BOW == type)
		return false;
#ifdef ENABLE_WOLFMAN_CHARACTER
	else if (CItemData::WEAPON_CLAW == type)
		return true;
#endif
	else
		return true;
}

bool CActorInstance::__IsWeaponTrace(uint32_t weaponType)
{
	switch (weaponType)
	{
		case CItemData::WEAPON_BELL:
		case CItemData::WEAPON_FAN:
		case CItemData::WEAPON_BOW:
			return false;

		default:
			return true;
	}
}

void CActorInstance::AttachWeapon(uint32_t dwItemIndex, uint32_t dwParentPartIndex, uint32_t dwPartIndex)
{
	if (dwPartIndex >= CRaceData::PART_MAX_NUM)
		return;

	m_adwPartItemID[dwPartIndex] = dwItemIndex;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemIndex, &pItemData))
	{
		RegisterModelThing(dwPartIndex, nullptr);
		SetModelInstance(dwPartIndex, dwPartIndex, 0);

		RegisterModelThing(CRaceData::PART_WEAPON_LEFT, nullptr);
		SetModelInstance(CRaceData::PART_WEAPON_LEFT, CRaceData::PART_WEAPON_LEFT, 0);

		RefreshActorInstance();
		return;
	}

	__DestroyWeaponTrace();

	if (__IsRightHandWeapon(pItemData->GetWeaponType()))
		AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON, pItemData);
	if (__IsLeftHandWeapon(pItemData->GetWeaponType()))
		AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON_LEFT, pItemData);
}

BOOL CActorInstance::GetAttachingBoneName(uint32_t dwPartIndex, const char ** c_pszBoneName)
{
	return m_pkCurRaceData->GetAttachingBoneName(dwPartIndex, c_pszBoneName);
}

void CActorInstance::AttachWeapon(uint32_t dwParentPartIndex, uint32_t dwPartIndex, CItemData * pItemData)
{
	//	assert(m_pkCurRaceData);
	if (!pItemData)
		return;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(AUTODETECT_LYCAN_RODNPICK_BONE)
	const char * szBoneName;
	if ((GetRace() == 8) && (pItemData->GetType() == CItemData::ITEM_TYPE_ROD || pItemData->GetType() == CItemData::ITEM_TYPE_PICK))
		szBoneName = "equip_right";
	else if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
		return;
#else
	const char * szBoneName;
	if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
		return;
#endif

	if (CRaceData::PART_WEAPON_LEFT == dwPartIndex)
		RegisterModelThing(dwPartIndex, pItemData->GetSubModelThing());
	else
		RegisterModelThing(dwPartIndex, pItemData->GetModelThing());

	for (uint32_t i = 0; i < pItemData->GetLODModelThingCount(); ++i)
	{
		CGraphicThing * pThing;

		if (!pItemData->GetLODModelThingPointer(i, &pThing))
			continue;

		RegisterLODThing(dwPartIndex, pThing);
	}

	SetModelInstance(dwPartIndex, dwPartIndex, 0);
	AttachModelInstance(dwParentPartIndex, szBoneName, dwPartIndex);

	if (USE_WEAPON_SPECULAR)
	{
		SMaterialData kMaterialData;
		kMaterialData.pImage = nullptr;
		kMaterialData.isSpecularEnable = TRUE;
		kMaterialData.fSpecularPower = pItemData->GetSpecularPowerf();
		kMaterialData.bSphereMapIndex = 1;
		SetMaterialData(dwPartIndex, nullptr, kMaterialData);
	}

	// Weapon Trace
	if (__IsWeaponTrace(pItemData->GetWeaponType()))
	{
		CWeaponTrace * pWeaponTrace = CWeaponTrace::New();
		pWeaponTrace->SetWeaponInstance(this, dwPartIndex, szBoneName);
		m_WeaponTraceVector.emplace_back(pWeaponTrace);
	}
}

void CActorInstance::DettachEffect(uint32_t dwEID)
{
	auto i = m_AttachingEffectList.begin();

	while (i != m_AttachingEffectList.end())
	{
		TAttachingEffect & rkAttEft = (*i);

		if (rkAttEft.dwEffectIndex == dwEID)
		{
			i = m_AttachingEffectList.erase(i);
			CEffectManager::Instance().DestroyEffectInstance(dwEID);
		}
		else
			++i;
	}
}

uint32_t CActorInstance::AttachEffectByName(uint32_t dwParentPartIndex, const char * c_pszBoneName, const char * c_pszEffectName)
{
	std::string str;
	uint32_t dwCRC;
	StringPath(c_pszEffectName, str);
	dwCRC = GetCaseCRC32(str.c_str(), str.length());

	return AttachEffectByID(dwParentPartIndex, c_pszBoneName, dwCRC);
}

void CActorInstance::ChangePart(uint32_t dwPartIndex, uint32_t dwItemIndex)
{
	m_adwPartItemID[dwPartIndex] = dwItemIndex;
}

uint32_t CActorInstance::AttachEffectByID(uint32_t dwParentPartIndex, const char * c_pszBoneName, uint32_t dwEffectID, const D3DXVECTOR3 * c_pv3Position
#ifdef ENABLE_SCALE_SYSTEM
	, float fParticleScale, const D3DXVECTOR3* c_pv3MeshScale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	, uint32_t* dwSkillColor
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
	, uint32_t dwAffectIndex, BOOL bIsSpecial
#endif
)
{
	TAttachingEffect ae;
	ae.iLifeType = EFFECT_LIFE_INFINITE;
	ae.dwEndTime = 0;
	ae.dwModelIndex = dwParentPartIndex;
	ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	ae.isAttaching = TRUE;
#ifdef ENABLE_GRAPHIC_ON_OFF
	ae.dwAffectIndex = dwAffectIndex;
	ae.bSpecial = bIsSpecial;
#endif

	if (c_pv3Position
#ifdef ENABLE_RENDER_TARGET_EFFECT
		&& !IsRenderTarget()
#endif	
	)
		D3DXMatrixTranslation(&ae.matTranslation, c_pv3Position->x, c_pv3Position->y, c_pv3Position->z);
	else
		D3DXMatrixIdentity(&ae.matTranslation);
	CEffectManager & rkEftMgr = CEffectManager::Instance();

#ifdef ENABLE_SCALE_SYSTEM
	if (c_pv3MeshScale)
#endif
	{
		rkEftMgr.CreateEffectInstance(ae.dwEffectIndex, dwEffectID
#ifdef ENABLE_SCALE_SYSTEM
			, fParticleScale, c_pv3MeshScale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			, dwSkillColor
#endif
		);
	}
#ifdef ENABLE_SCALE_SYSTEM
	else
	{
#	ifdef ENABLE_SKILL_COLOR_SYSTEM
		rkEftMgr.CreateEffectInstance(ae.dwEffectIndex, dwEffectID, 1.0f, nullptr, dwSkillColor);
#	else
		rkEftMgr.CreateEffectInstance(ae.dwEffectIndex, dwEffectID, 1.0f, nullptr);
#	endif
	}
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!m_bShowEffects && !bIsSpecial && dwAffectIndex != CInstanceBase::EFFECT_SELECT && dwAffectIndex != CInstanceBase::EFFECT_TARGET)
	{
		rkEftMgr.SelectEffectInstance(ae.dwEffectIndex);
		rkEftMgr.HideEffect();
	}
#endif
#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (IsRenderTarget())
		rkEftMgr.SetSpecialRenderEffect(ae.dwEffectIndex);
#endif

	if (c_pszBoneName)
	{
		int iBoneIndex;

		if (!FindBoneIndex(dwParentPartIndex, c_pszBoneName, &iBoneIndex))
		{
			ae.iBoneIndex = -1;
			//Tracef("Cannot get Bone Index\n");
			//assert(false && "Cannot get Bone Index");
		}
		else
			ae.iBoneIndex = iBoneIndex;
	}
	else
		ae.iBoneIndex = -1;

	m_AttachingEffectList.emplace_back(ae);

	return ae.dwEffectIndex;
}

void CActorInstance::RefreshActorInstance()
{
	if (!m_pkCurRaceData)
	{
		TraceError("void CActorInstance::RefreshActorInstance() - m_pkCurRaceData=nullptr");
		return;
	}

	// This is Temporary place before making the weapon detection system
	// Setup Collison Detection Data
	m_BodyPointInstanceList.clear();
	//m_AttackingPointInstanceList.clear();
	m_DefendingPointInstanceList.clear();

	// Base
	for (uint32_t i = 0; i < m_pkCurRaceData->GetAttachingDataCount(); ++i)
	{
		const NRaceData::TAttachingData * c_pAttachingData;

		if (!m_pkCurRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
			continue;

		switch (c_pAttachingData->dwType)
		{
			case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
			{
				const NRaceData::TCollisionData * c_pCollisionData = c_pAttachingData->pCollisionData;

				TCollisionPointInstance PointInstance;
				if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
					continue;

				if (!CreateCollisionInstancePiece(CRaceData::PART_MAIN, c_pAttachingData, &PointInstance))
					continue;

				switch (c_pCollisionData->iCollisionType)
				{
					case NRaceData::COLLISION_TYPE_ATTACKING:
						//m_AttackingPointInstanceList.emplace_back(PointInstance);
						break;
					case NRaceData::COLLISION_TYPE_DEFENDING:
						m_DefendingPointInstanceList.emplace_back(PointInstance);
						break;
					case NRaceData::COLLISION_TYPE_BODY:
						m_BodyPointInstanceList.emplace_back(PointInstance);
						break;
				}
			}
			break;

			case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
//				if (!m_bEffectInitialized)
//				{
//					uint32_t dwCRC;
//					StringPath(c_pAttachingData->pEffectData->strFileName);
//					dwCRC = GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(),c_pAttachingData->pEffectData->strFileName.length());
//
//					TAttachingEffect ae;
//					ae.iLifeType = EFFECT_LIFE_INFINITE;
//					ae.dwEndTime = 0;
//					ae.dwModelIndex = 0;
//					ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
//					ae.isAttaching = TRUE;
//					CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex, dwCRC);
//
//					if (c_pAttachingData->isAttaching)
//					{
//						int iBoneIndex;
//						if (!FindBoneIndex(0,c_pAttachingData->strAttachingBoneName.c_str(), &iBoneIndex))
//						{
//							Tracef("Cannot get Bone Index\n");
//							assert(false/*Cannot get Bone Index*/);
//						}
//
//						ae.iBoneIndex = iBoneIndex;
//					}
//					else
//					{
//						ae.iBoneIndex = -1;
//					}
//
//					m_AttachingEffectList.emplace_back(ae);
//				}

#ifdef ENABLE_GRAPHIC_ON_OFF
				if (!m_bEffectInitialized)
				{
#endif
					if (c_pAttachingData->isAttaching)
						AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(), c_pAttachingData->pEffectData->strFileName.c_str());
					else
						AttachEffectByName(0, nullptr, c_pAttachingData->pEffectData->strFileName.c_str());
#ifdef ENABLE_GRAPHIC_ON_OFF
				}
#endif
				break;

			case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
				break;

			default:
				assert(false /*NOT_IMPLEMENTED*/);
				break;
		}
	}

	for (uint32_t j = 0; j < CRaceData::PART_MAX_NUM; ++j)
	{
		if (0 == m_adwPartItemID[j])
			continue;

		CItemData * pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(m_adwPartItemID[j], &pItemData))
			return;

		for (uint32_t k = 0; k < pItemData->GetAttachingDataCount(); ++k)
		{
			const NRaceData::TAttachingData * c_pAttachingData;

			if (!pItemData->GetAttachingDataPointer(k, &c_pAttachingData))
				continue;

			switch (c_pAttachingData->dwType)
			{
				case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
					{
						const NRaceData::TCollisionData * c_pCollisionData = c_pAttachingData->pCollisionData;

						TCollisionPointInstance PointInstance;
						if (NRaceData::COLLISION_TYPE_ATTACKING == c_pCollisionData->iCollisionType)
							continue;
						if (!CreateCollisionInstancePiece(j, c_pAttachingData, &PointInstance))
							continue;

						switch (c_pCollisionData->iCollisionType)
						{
							case NRaceData::COLLISION_TYPE_ATTACKING:
								//m_AttackingPointInstanceList.emplace_back(PointInstance);
								break;
							case NRaceData::COLLISION_TYPE_DEFENDING:
								m_DefendingPointInstanceList.emplace_back(PointInstance);
								break;
							case NRaceData::COLLISION_TYPE_BODY:
								m_BodyPointInstanceList.emplace_back(PointInstance);
								break;
						}
					}
					break;

				case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
					if (!m_bEffectInitialized)
					{
						uint32_t dwCRC;
						StringPath(c_pAttachingData->pEffectData->strFileName);
						dwCRC = GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(), c_pAttachingData->pEffectData->strFileName.length());

						TAttachingEffect ae;
						ae.iLifeType = EFFECT_LIFE_INFINITE;
						ae.dwEndTime = 0;
						ae.dwModelIndex = j;
						ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
						ae.isAttaching = TRUE;
#ifdef ENABLE_GRAPHIC_ON_OFF
						ae.bSpecial = FALSE;
						ae.dwAffectIndex = dwCRC;
#endif
						CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex, dwCRC);

#ifdef ENABLE_RENDER_TARGET_EFFECT
						if (IsRenderTarget())
							CEffectManager::Instance().SetSpecialRenderEffect(ae.dwEffectIndex);
#endif

						int iBoneIndex;
						if (!FindBoneIndex(j, c_pAttachingData->strAttachingBoneName.c_str(), &iBoneIndex))
						{
							Tracef("Cannot get Bone Index\n");
							assert(false /*Cannot get Bone Index*/);
						}
						Tracef("Creating %p %d %d\n", this, j, k);

						ae.iBoneIndex = iBoneIndex;

						m_AttachingEffectList.emplace_back(ae);
					}
					break;

				case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
					break;

				default:
					assert(false /*NOT_IMPLEMENTED*/);
					break;
			}
		}
	}

	m_bEffectInitialized = true;
#ifdef ENABLE_GRAPHIC_ON_OFF
	m_bShowEffects = true;
#endif
}

void CActorInstance::SetWeaponTraceTexture(const char * szTextureName)
{
	std::vector<CWeaponTrace *>::iterator it;
	for (it = m_WeaponTraceVector.begin(); it != m_WeaponTraceVector.end(); ++it)
		(*it)->SetTexture(szTextureName);
}

void CActorInstance::UseTextureWeaponTrace()
{
	for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(), std::mem_fn(&CWeaponTrace::UseTexture));
}

void CActorInstance::UseAlphaWeaponTrace()
{
	for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(), std::mem_fn(&CWeaponTrace::UseAlpha));
}

void CActorInstance::UpdateAttachingInstances()
{
	CEffectManager & rkEftMgr = CEffectManager::Instance();

	std::list<TAttachingEffect>::iterator it;
	uint32_t dwCurrentTime = CTimer::Instance().GetCurrentMillisecond();
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();)
	{
		if (EFFECT_LIFE_WITH_MOTION == it->iLifeType)
		{
			++it;
			continue;
		}

		if ((EFFECT_LIFE_NORMAL == it->iLifeType && it->dwEndTime < dwCurrentTime) || !rkEftMgr.IsAliveEffect(it->dwEffectIndex))
		{
			rkEftMgr.DestroyEffectInstance(it->dwEffectIndex);
			it = m_AttachingEffectList.erase(it);
		}
		else
		{
			if (it->isAttaching)
			{
				rkEftMgr.SelectEffectInstance(it->dwEffectIndex);

				if (it->iBoneIndex == -1)
				{
					D3DXMATRIX matTransform;
					matTransform = it->matTranslation;
					matTransform *= m_worldMatrix;
					rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
				}
				else
				{
					D3DXMATRIX * pBoneMat;
					if (GetBoneMatrix(it->dwModelIndex, it->iBoneIndex, &pBoneMat))
					{
						D3DXMATRIX matTransform;
						matTransform = *pBoneMat;
						matTransform *= it->matTranslation;
						matTransform *= m_worldMatrix;
						rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
					}
					else
					{
						//TraceError("GetBoneMatrix(modelIndex(%d), boneIndex(%d)).NOT_FOUND_BONE",
						//	it->dwModelIndex, it->iBoneIndex);
					}
				}
			}

			++it;
		}
	}
}

void CActorInstance::ShowAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
		CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
		CEffectManager::Instance().ShowEffect();
	}
}

void CActorInstance::HideAllAttachingEffect()
{
	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
	{
#ifdef ENABLE_GRAPHIC_ON_OFF
		if (FALSE == it->bSpecial)
		{
			if (CPythonGraphicOnOff::Instance().GetEffectOnOffLevel() < CPythonGraphicOnOff::EFFECT_ONOFF_LEVEL_NONE)
			{
				if (it->dwAffectIndex != CInstanceBase::EFFECT_SELECT && it->dwAffectIndex != CInstanceBase::EFFECT_TARGET)
				{
					CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
					CEffectManager::Instance().HideEffect();
				}
			}
			else
			{
				CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
				CEffectManager::Instance().HideEffect();
			}
		}
#else
		CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
		CEffectManager::Instance().HideEffect();
#endif
	}
}

void CActorInstance::__ClearAttachingEffect()
{
	m_bEffectInitialized = false;

	std::list<TAttachingEffect>::iterator it;
	for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end(); ++it)
		CEffectManager::Instance().DestroyEffectInstance(it->dwEffectIndex);
	m_AttachingEffectList.clear();
}

#ifdef ENABLE_INGAME_WIKI
void CActorInstance::WikiRenderAllAttachingModuleEffect()
{
	for (const auto& it : m_AttachingEffectList)
		CEffectManager::Instance().WikiModuleRenderOneEffect(it.dwEffectIndex);
}
#endif

#ifdef ENABLE_QUIVER_SYSTEM
void CActorInstance::SetQuiverEquipped(bool bEquipped)
{
	m_bIsQuiverEquipped = bEquipped;
}

void CActorInstance::SetQuiverEffectID(uint32_t dwEffectID)
{
	m_dwQuiverEffectID = dwEffectID;
}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
void CActorInstance::AttachAcce(uint32_t dwParentPartIndex, uint32_t dwPartIndex, CItemData* pItemData)
{
	if (!pItemData)
		return;

	if (CRaceData::PART_ACCE == dwPartIndex)
		RegisterModelThing(CRaceData::PART_ACCE, pItemData->GetSubModelThing());
	else
		RegisterModelThing(dwPartIndex, pItemData->GetModelThing());

	for (uint32_t i = 0; i < pItemData->GetLODModelThingCount(); ++i)
	{
		CGraphicThing* pThing;
		if (!pItemData->GetLODModelThingPointer(i, &pThing))
			continue;

		RegisterLODThing(dwPartIndex, pThing);
	}

	SetModelInstance(dwPartIndex, dwPartIndex, 0);
	AttachModelInstance(dwParentPartIndex, "Bip01 Spine2", dwPartIndex);

	SMaterialData kMaterialData;
	kMaterialData.pImage = nullptr;
	kMaterialData.isSpecularEnable = true;
	kMaterialData.fSpecularPower = pItemData->GetSpecularPowerf();
	kMaterialData.bSphereMapIndex = 1;
	SetMaterialData(dwPartIndex, nullptr, kMaterialData);
}

void CActorInstance::AttachAcce(uint32_t dwItemIndex, uint32_t dwParentPartIndex, uint32_t dwPartIndex)
{
	if (dwPartIndex >= CRaceData::PART_MAX_NUM)
		return;

	m_adwPartItemID[dwPartIndex] = dwItemIndex;

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemIndex, &pItemData))
	{
		RegisterModelThing(dwPartIndex, nullptr);
		SetModelInstance(dwPartIndex, dwPartIndex, 0);

		RegisterModelThing(CRaceData::PART_ACCE, nullptr);
		SetModelInstance(CRaceData::PART_ACCE, CRaceData::PART_ACCE, 0);

		RefreshActorInstance();
		return;
	}

	AttachAcce(dwParentPartIndex, CRaceData::PART_ACCE, pItemData);
}
#endif
