#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceManager.h"
#include "ItemManager.h"
#include "RaceData.h"

#include "../EterLib/ResourceManager.h"
#include "../EterGrnLib/Util.h"
#ifdef ENABLE_SKILL_COLOR_SYSTEM
#	include "../UserInterface/InstanceBase.h"
#	include "../UserInterface/PythonSkill.h"
#endif

uint32_t CActorInstance::GetVirtualID()
{
	return m_dwSelfVID;
}

void CActorInstance::SetVirtualID(uint32_t dwVID)
{
	m_dwSelfVID = dwVID;
}

void CActorInstance::UpdateAttribute()
{
	if (!m_pAttributeInstance)
		return;

	if (!m_bNeedUpdateCollision)
		return;

	m_bNeedUpdateCollision = FALSE;

	const CStaticCollisionDataVector & c_rkVec_ColliData = m_pAttributeInstance->GetObjectPointer()->GetCollisionDataVector();
	UpdateCollisionData(&c_rkVec_ColliData);

	m_pAttributeInstance->RefreshObject(GetTransform());
	UpdateHeightInstance(m_pAttributeInstance);

	//BOOL isHeightData = m_pAttributeInstance->IsEmpty();
}

void CActorInstance::__CreateAttributeInstance(CAttributeData * pData)
{
	m_pAttributeInstance = CAttributeInstance::New();
	m_pAttributeInstance->Clear();
	m_pAttributeInstance->SetObjectPointer(pData);
	if (pData->IsEmpty())
	{
		m_pAttributeInstance->Clear();
		CAttributeInstance::Delete(m_pAttributeInstance);
	}
}

uint32_t CActorInstance::GetRace()
{
	return m_eRace;
}

bool CActorInstance::SetRace(uint32_t eRace)
{
	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(eRace, &pRaceData))
	{
		m_eRace = 0;
		m_pkCurRaceData = nullptr;
		return false;
	}

	m_eRace = eRace;
	m_pkCurRaceData = pRaceData;

	CAttributeData * pAttributeData = pRaceData->GetAttributeDataPtr();
	if (pAttributeData)
		__CreateAttributeInstance(pAttributeData);

	msl::refill(m_adwPartItemID);

	// Setup Graphic ResourceData
	__ClearAttachingEffect();

	CGraphicThingInstance::Clear();

	if (IsPC())
	{
		CGraphicThingInstance::ReserveModelThing(CRaceData::PART_MAX_NUM);
		CGraphicThingInstance::ReserveModelInstance(CRaceData::PART_MAX_NUM);
	}
	else
	{
		CGraphicThingInstance::ReserveModelThing(1);
		CGraphicThingInstance::ReserveModelInstance(1);
	}


	CRaceData::TMotionModeDataIterator itor;

	if (pRaceData->CreateMotionModeIterator(itor))
	{
		do
		{
			uint16_t wMotionMode = itor->first;
			CRaceData::TMotionModeData * pMotionModeData = itor->second;

			auto itorMotion = pMotionModeData->MotionVectorMap.begin();
			for (; itorMotion != pMotionModeData->MotionVectorMap.end(); ++itorMotion)
			{
				uint16_t wMotionIndex = itorMotion->first;
				const CRaceData::TMotionVector & c_rMotionVector = itorMotion->second;
				CRaceData::TMotionVector::const_iterator it;
				uint32_t i;
				for (i = 0, it = c_rMotionVector.begin(); it != c_rMotionVector.end(); ++i, ++it)
				{
					uint32_t dwMotionKey = MAKE_RANDOM_MOTION_KEY(wMotionMode, wMotionIndex, i);
					CGraphicThingInstance::RegisterMotionThing(dwMotionKey, it->pMotion);
				}
			}
		} while (pRaceData->NextMotionModeIterator(itor));
	}

	return true;
}

void CActorInstance::SetHair(uint32_t eHair)
{
	m_eHair = eHair;

	CRaceData * pRaceData;

	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
		return;

	CRaceData::SHair * pkHair = pRaceData->FindHair(eHair);
	if (pkHair)
	{
		if (!pkHair->m_stModelFileName.empty())
		{
			auto * pkHairThing =
				msl::inherit_cast<CGraphicThing *>(CResourceManager::Instance().GetResourcePointer(pkHair->m_stModelFileName.c_str()));
			RegisterModelThing(CRaceData::PART_HAIR, pkHairThing);
			SetModelInstance(CRaceData::PART_HAIR, CRaceData::PART_HAIR, 0, CRaceData::PART_MAIN);
		}

		const std::vector<CRaceData::SSkin> & c_rkVct_kSkin = pkHair->m_kVct_kSkin;
		std::vector<CRaceData::SSkin>::const_iterator i;
		for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i)
		{
			const CRaceData::SSkin & c_rkSkinItem = *i;

			CResource * pkRes = CResourceManager::Instance().GetResourcePointer(c_rkSkinItem.m_stDstFileName.c_str());

			if (pkRes)
				SetMaterialImagePointer(CRaceData::PART_HAIR, c_rkSkinItem.m_stSrcFileName.c_str(),
										msl::inherit_cast<CGraphicImage *>(pkRes));
		}
	}
}


void CActorInstance::SetShape(uint32_t eShape, float fSpecular)
{
	m_eShape = eShape;

	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
		return;

	CRaceData::SShape * pkShape = pRaceData->FindShape(eShape);
	if (pkShape)
	{
		CResourceManager & rkResMgr = CResourceManager::Instance();

		if (pkShape->m_stModelFileName.empty())
		{
			CGraphicThing * pModelThing = pRaceData->GetBaseModelThing();
			RegisterModelThing(0, pModelThing);
		}
		else
		{
			auto * pModelThing = msl::inherit_cast<CGraphicThing *>(rkResMgr.GetResourcePointer(pkShape->m_stModelFileName.c_str()));
			RegisterModelThing(0, pModelThing);
		}

		{
			std::string stLODModelFileName;

			char szLODModelFileNameEnd[256];
			for (uint32_t uLODIndex = 1; uLODIndex <= 3; ++uLODIndex)
			{
				sprintf(szLODModelFileNameEnd, "_lod_%.2d.gr2", uLODIndex);
				stLODModelFileName = CFileNameHelper::NoExtension(pkShape->m_stModelFileName) + szLODModelFileNameEnd;
				if (!rkResMgr.IsFileExist(stLODModelFileName.c_str()))
					break;

				auto * pLODModelThing = msl::inherit_cast<CGraphicThing *>(rkResMgr.GetResourcePointer(stLODModelFileName.c_str()));
				if (!pLODModelThing)
					break;

				RegisterLODThing(0, pLODModelThing);
			}
		}

		SetModelInstance(0, 0, 0);

		const std::vector<CRaceData::SSkin> & c_rkVct_kSkin = pkShape->m_kVct_kSkin;
		std::vector<CRaceData::SSkin>::const_iterator i;
		for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i)
		{
			const CRaceData::SSkin & c_rkSkinItem = *i;

			CResource * pkRes = CResourceManager::Instance().GetResourcePointer(c_rkSkinItem.m_stDstFileName.c_str());

			if (pkRes)
			{
				if (fSpecular > 0.0f)
				{
					SMaterialData kMaterialData;
					kMaterialData.pImage = msl::inherit_cast<CGraphicImage *>(pkRes);
					kMaterialData.isSpecularEnable = TRUE;
					kMaterialData.fSpecularPower = fSpecular;
					kMaterialData.bSphereMapIndex = 0;
					SetMaterialData(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(), kMaterialData);
				}
				else
					SetMaterialImagePointer(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(),
											msl::inherit_cast<CGraphicImage *>(pkRes));
			}
		}
	}
	else
	{
		if (pRaceData->IsTree())
			__CreateTree(pRaceData->GetTreeFileName());
		else
		{
			CGraphicThing * pModelThing = pRaceData->GetBaseModelThing();
			RegisterModelThing(0, pModelThing);

			CGraphicThing * pLODModelThing = pRaceData->GetLODModelThing();
			RegisterLODThing(0, pLODModelThing);

			SetModelInstance(0, 0, 0);
		}
	}

	// Attaching Objects
	for (uint32_t i = 0; i < pRaceData->GetAttachingDataCount(); ++i)
	{
		const NRaceData::TAttachingData * c_pAttachingData;
		if (!pRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
			continue;

		switch (c_pAttachingData->dwType)
		{
			case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
				if (c_pAttachingData->isAttaching)
					AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(), c_pAttachingData->pEffectData->strFileName.c_str());
				else
					AttachEffectByName(0, nullptr, c_pAttachingData->pEffectData->strFileName.c_str());
				break;
		}
	}
}

void CActorInstance::ChangeMaterial(const char * c_szFileName)
{
	CRaceData * pRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
		return;

	CRaceData::SShape * pkShape = pRaceData->FindShape(m_eShape);
	if (!pkShape)
		return;

	const std::vector<CRaceData::SSkin> & c_rkVct_kSkin = pkShape->m_kVct_kSkin;
	if (c_rkVct_kSkin.empty())
		return;

	auto i = c_rkVct_kSkin.begin();
	const CRaceData::SSkin & c_rkSkinItem = *i;

	std::string dstFileName = "d:/ymir work/npc/guild_symbol/guild_symbol.dds";
	dstFileName = c_szFileName;

	CResource * pkRes = CResourceManager::Instance().GetResourcePointer(dstFileName.c_str());
	if (!pkRes)
		return;

	SetMaterialImagePointer(c_rkSkinItem.m_ePart, c_rkSkinItem.m_stSrcFileName.c_str(), msl::inherit_cast<CGraphicImage *>(pkRes));
}
/*
void CActorInstance::SetPart(uint32_t dwPartIndex, uint32_t dwItemID)
{
	if (dwPartIndex>=CRaceData::PART_MAX_NUM)
		return;

	if (!m_pkCurRaceData)
	{
		assert(m_pkCurRaceData);
		return;
	}

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pItemData))
		return;

	RegisterModelThing(dwPartIndex, pItemData->GetModelThing());
	for (uint32_t i = 0; i < pItemData->GetLODModelThingCount(); ++i)
	{
		CGraphicThing * pThing;
		if (!pItemData->GetLODModelThingPointer(i, &pThing))
			continue;

		RegisterLODThing(dwPartIndex, pThing);
	}
	SetModelInstance(dwPartIndex, dwPartIndex, 0);

	m_adwPartItemID[dwPartIndex] = dwItemID;
}
*/

#ifdef ENABLE_SKILL_COLOR_SYSTEM
uint32_t * CActorInstance::GetSkillColorByEffectID(uint32_t id)
{
	switch (id)
	{
		case 14: // FLY_CHAIN_LIGHTNING
			return m_dwSkillColor[108];
			break;

		case 16: //FLY_SKILL_MUYEONG
			return m_dwSkillColor[78];
			break;
			/////////////////////////////////////////////
		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_CHEONGEUN:
		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_FALLEN_CHEONGEUN:
			return m_dwSkillColor[19];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GYEONGGONG:
			return m_dwSkillColor[49];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GWIGEOM:
			return m_dwSkillColor[63];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GONGPO:
			return m_dwSkillColor[64];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_JUMAGAP:
			return m_dwSkillColor[65];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_HOSIN:
			return m_dwSkillColor[94];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_BOHO:
			return m_dwSkillColor[95];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_KWAESOK:
			return m_dwSkillColor[110];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_HEUKSIN:
			return m_dwSkillColor[79];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_MUYEONG:
			return m_dwSkillColor[78];

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_FIRE:
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_GICHEON:
			return m_dwSkillColor[96];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_JEUNGRYEOK:
			return m_dwSkillColor[111];
			break;

		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_PABEOP:
			return m_dwSkillColor[66];
			break;

#ifdef ENABLE_NINETH_SKILL
		case CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_CHEONUN:
			return m_dwSkillColor[182];
			break;
#endif

			/////////////////////////////////////////////
		case CInstanceBase::EFFECT_WEAPON + CInstanceBase::WEAPON_ONEHAND:
		case CInstanceBase::EFFECT_WEAPON + CInstanceBase::WEAPON_TWOHAND:
			return m_dwSkillColor[4];
			break;
	}

	uint32_t motion_index = GET_MOTION_INDEX(id);
	for (int i = 0; i < CPythonSkill::SKILL_EFFECT_COUNT; ++i)
	{
		for (int x = 0; x < MAX_SKILL_COUNT; ++x)
		{
			if ((motion_index == ((CRaceMotionData::NAME_SKILL + x + 1) + (i * 25))) || (motion_index == ((CRaceMotionData::NAME_SKILL + 15 + x + 1) + (i * 25))))
				return m_dwSkillColor[(motion_index - CRaceMotionData::NAME_SKILL) - (i * 25)];
		}
	}
	return nullptr;
}
#endif

uint32_t CActorInstance::GetPartItemID(uint32_t dwPartIndex)
{
	if (dwPartIndex >= CRaceData::PART_MAX_NUM)
	{
		TraceError("CActorInstance::GetPartIndex(dwPartIndex=%d/CRaceData::PART_MAX_NUM=%d)", dwPartIndex, CRaceData::PART_MAX_NUM);
		return 0;
	}

	return m_adwPartItemID[dwPartIndex];
}

void CActorInstance::SetSpecularInfo(BOOL bEnable, int iPart, float fAlpha)
{
	CRaceData * pkRaceData;
	if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pkRaceData))
		return;

	CRaceData::SShape * pkShape = pkRaceData->FindShape(m_eShape);
	if (pkShape->m_kVct_kSkin.empty())
		return;

	std::string filename = pkShape->m_kVct_kSkin[0].m_stSrcFileName;
	CFileNameHelper::ChangeDosPath(filename);

	CGraphicThingInstance::SetSpecularInfo(iPart, filename.c_str(), bEnable, fAlpha);
}

void CActorInstance::SetSpecularInfoForce(BOOL bEnable, int iPart, float fAlpha)
{
	CGraphicThingInstance::SetSpecularInfo(iPart, nullptr, bEnable, fAlpha);
}
