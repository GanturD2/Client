#include "StdAfx.h"
#include "../EterLib/GrpMath.h"
#include "../GameLib/ItemManager.h"
#include "../EffectLib/EffectManager.h"
#include "PythonBackground.h"

#include "PythonItem.h"
#include "PythonTextTail.h"
#ifdef ENABLE_EXTENDED_CONFIGS
#	include "PythonSystem.h"
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
#	include "../EterPythonLib/PythonGraphicOnOff.h"
#endif

const float c_fDropStartHeight = 100.0f;
const float c_fDropTime = 0.5f;

std::string CPythonItem::TGroundItemInstance::ms_astDropSoundFileName[DROPSOUND_NUM];

void CPythonItem::GetInfo(std::string * pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Item: Inst %u, Pool %u", m_GroundItemInstanceMap.size(), m_GroundItemInstancePool.GetCapacity());

	pstInfo->append(szInfo);
}

void CPythonItem::TGroundItemInstance::Clear()
{
	stOwnership = "";
	ThingInstance.Clear();
	CEffectManager::Instance().DestroyEffectInstance(dwEffectInstanceIndex);
}

void CPythonItem::TGroundItemInstance::__PlayDropSound(uint32_t eItemType, const D3DXVECTOR3 & c_rv3Pos)
{
	if (eItemType >= DROPSOUND_NUM)
		return;

	CSoundManager::Instance().PlaySound3D(c_rv3Pos.x, c_rv3Pos.y, c_rv3Pos.z, ms_astDropSoundFileName[eItemType].c_str());
}

bool CPythonItem::TGroundItemInstance::Update()
{
	if (bAnimEnded)
		return false;
	if (dwEndTime < CTimer::Instance().GetCurrentMillisecond())
	{
		ThingInstance.SetRotationQuaternion(qEnd);

		/*D3DXVECTOR3 v3Adjust = -v3Center;
		D3DXMATRIX mat;
		D3DXMatrixRotationYawPitchRoll(&mat,
		D3DXToRadian(rEnd.y),
		D3DXToRadian(rEnd.x),
		D3DXToRadian(rEnd.z));
		D3DXVec3TransformCoord(&v3Adjust,&v3Adjust,&mat);*/

		D3DXQUATERNION qAdjust(-v3Center.x, -v3Center.y, -v3Center.z, 0.0f);
		D3DXQUATERNION qc;
		D3DXQuaternionConjugate(&qc, &qEnd);
		D3DXQuaternionMultiply(&qAdjust, &qAdjust, &qEnd);
		D3DXQuaternionMultiply(&qAdjust, &qc, &qAdjust);

		ThingInstance.SetPosition(v3EndPosition.x + qAdjust.x, v3EndPosition.y + qAdjust.y, v3EndPosition.z + qAdjust.z);
		//ThingInstance.Update();
		bAnimEnded = true;

		__PlayDropSound(eDropSoundType, v3EndPosition);
	}
	else
	{
		uint32_t time = CTimer::Instance().GetCurrentMillisecond() - dwStartTime;
		uint32_t etime = dwEndTime - CTimer::Instance().GetCurrentMillisecond();
		float rate = time * 1.0f / (dwEndTime - dwStartTime);

		D3DXVECTOR3 v3NewPosition = v3EndPosition; // = rate*(v3EndPosition - v3StartPosition) + v3StartPosition;
		v3NewPosition.z += 100 - 100 * rate * (3 * rate - 2); //-100*(rate-1)*(3*rate+2);

		D3DXQUATERNION q;
		D3DXQuaternionRotationAxis(&q, &v3RotationAxis, etime * 0.03f * (-1 + rate * (3 * rate - 2)));
		//ThingInstance.SetRotation(rEnd.y + etime*rStart.y, rEnd.x + etime*rStart.x, rEnd.z + etime*rStart.z);
		D3DXQuaternionMultiply(&q, &qEnd, &q);

		ThingInstance.SetRotationQuaternion(q);
		D3DXQUATERNION qAdjust(-v3Center.x, -v3Center.y, -v3Center.z, 0.0f);
		D3DXQUATERNION qc;
		D3DXQuaternionConjugate(&qc, &q);
		D3DXQuaternionMultiply(&qAdjust, &qAdjust, &q);
		D3DXQuaternionMultiply(&qAdjust, &qc, &qAdjust);

		ThingInstance.SetPosition(v3NewPosition.x + qAdjust.x, v3NewPosition.y + qAdjust.y, v3NewPosition.z + qAdjust.z);

		/*D3DXVECTOR3 v3Adjust = -v3Center;
		D3DXMATRIX mat;
		D3DXMatrixRotationYawPitchRoll(&mat,
		D3DXToRadian(rEnd.y + etime*rStart.y),
		D3DXToRadian(rEnd.x + etime*rStart.x),
		D3DXToRadian(rEnd.z + etime*rStart.z));

		D3DXVec3TransformCoord(&v3Adjust,&v3Adjust,&mat);
		//Tracef("%f %f %f\n",v3Adjust.x,v3Adjust.y,v3Adjust.z);
		v3NewPosition += v3Adjust;
		ThingInstance.SetPosition(v3NewPosition.x, v3NewPosition.y, v3NewPosition.z);*/
	}
	ThingInstance.Transform();
	ThingInstance.Deform();
	return !bAnimEnded;
}

void CPythonItem::Update(const POINT & c_rkPtMouse)
{
	auto itor = m_GroundItemInstanceMap.begin();
	for (; itor != m_GroundItemInstanceMap.end(); ++itor)
		itor->second->Update();

	m_dwPickedItemID = __Pick(c_rkPtMouse);
}

void CPythonItem::Render()
{
	CPythonGraphic::Instance().SetDiffuseOperation();
	auto itor = m_GroundItemInstanceMap.begin();
	for (; itor != m_GroundItemInstanceMap.end(); ++itor)
	{
		CGraphicThingInstance & rInstance = itor->second->ThingInstance;
		//rInstance.Update();
		rInstance.Render();
		rInstance.BlendRender();
	}

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (!CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EX_CURSOR))
		return;
#endif
}

void CPythonItem::SetUseSoundFileName(uint32_t eItemType, const std::string & c_rstFileName)
{
	if (eItemType >= USESOUND_NUM)
		return;

	//Tracenf("SetUseSoundFile %d : %s", eItemType, c_rstFileName.c_str());

	m_astUseSoundFileName[eItemType] = c_rstFileName;
}

void CPythonItem::SetDropSoundFileName(uint32_t eItemType, const std::string & c_rstFileName) const
{
	if (eItemType >= DROPSOUND_NUM)
		return;

	Tracenf("SetDropSoundFile %d : %s", eItemType, c_rstFileName.c_str());

	SGroundItemInstance::ms_astDropSoundFileName[eItemType] = c_rstFileName;
}

void CPythonItem::PlayUseSound(uint32_t dwItemID)
{
	//CItemManager& rkItemMgr=CItemManager::Instance();

	CItemData * pkItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pkItemData))
		return;

	uint32_t eItemType = __GetUseSoundType(*pkItemData);
	if (eItemType == USESOUND_NONE)
		return;
	if (eItemType >= USESOUND_NUM)
		return;

	CSoundManager::Instance().PlaySound2D(m_astUseSoundFileName[eItemType].c_str());
}


void CPythonItem::PlayDropSound(uint32_t dwItemID) const
{
	//CItemManager& rkItemMgr=CItemManager::Instance();

	CItemData * pkItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pkItemData))
		return;

	uint32_t eItemType = __GetDropSoundType(*pkItemData);
	if (eItemType >= DROPSOUND_NUM)
		return;

	CSoundManager::Instance().PlaySound2D(SGroundItemInstance::ms_astDropSoundFileName[eItemType].c_str());
}

void CPythonItem::PlayUsePotionSound() const
{
	CSoundManager::Instance().PlaySound2D(m_astUseSoundFileName[USESOUND_POTION].c_str());
}

uint32_t CPythonItem::__GetDropSoundType(const CItemData & c_rkItemData) const
{
	switch (c_rkItemData.GetType())
	{
		case CItemData::ITEM_TYPE_WEAPON:
			switch (c_rkItemData.GetWeaponType())
			{
				case CItemData::WEAPON_BOW:
					return DROPSOUND_BOW;

				case CItemData::WEAPON_ARROW:
#ifdef ENABLE_QUIVER_SYSTEM
				case CItemData::WEAPON_QUIVER:
#endif
					return DROPSOUND_DEFAULT;

				default:
					return DROPSOUND_WEAPON;
			}
		case CItemData::ITEM_TYPE_ARMOR:
			switch (c_rkItemData.GetSubType())
			{
				case CItemData::ARMOR_NECK:
				case CItemData::ARMOR_EAR:
					return DROPSOUND_ACCESSORY;

				case CItemData::ARMOR_BODY:
					return DROPSOUND_ARMOR;

				default:
					return DROPSOUND_DEFAULT;
			}

		default:
			return DROPSOUND_DEFAULT;
	}
}


uint32_t CPythonItem::__GetUseSoundType(const CItemData & c_rkItemData) const
{
	switch (c_rkItemData.GetType())
	{
	case CItemData::ITEM_TYPE_WEAPON:
			switch (c_rkItemData.GetWeaponType())
			{
				case CItemData::WEAPON_BOW:
					return USESOUND_BOW;
				case CItemData::WEAPON_ARROW:
#ifdef ENABLE_QUIVER_SYSTEM
				case CItemData::WEAPON_QUIVER:
#endif
					return USESOUND_DEFAULT;
				default:
					return USESOUND_WEAPON;
			}
		case CItemData::ITEM_TYPE_ARMOR:
			switch (c_rkItemData.GetSubType())
			{
				case CItemData::ARMOR_NECK:
				case CItemData::ARMOR_EAR:
					return USESOUND_ACCESSORY;
				case CItemData::ARMOR_BODY:
					return USESOUND_ARMOR;
				default:
					return USESOUND_DEFAULT;
			}
		case CItemData::ITEM_TYPE_USE:
			switch (c_rkItemData.GetSubType())
			{
				case CItemData::USE_ABILITY_UP:
					return USESOUND_POTION;
				case CItemData::USE_POTION:
					return USESOUND_NONE;
				case CItemData::USE_TALISMAN:
					return USESOUND_PORTAL;
				default:
					return USESOUND_DEFAULT;
			}

		default:
			return USESOUND_DEFAULT;
	}
}

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
#include "PythonNonPlayer.h"
#include "PythonSkill.h"
void CPythonItem::CreateItem(uint32_t dwVirtualID, uint32_t dwVirtualNumber, float x, float y, float z, bool bDrop, long alSockets[ITEM_SOCKET_SLOT_MAX_NUM], TPlayerItemAttribute aAttrs[ITEM_ATTRIBUTE_SLOT_MAX_NUM])
#else
void CPythonItem::CreateItem(uint32_t dwVirtualID, uint32_t dwVirtualNumber, float x, float y, float z, bool bDrop)
#endif
{
	//CItemManager& rkItemMgr=CItemManager::Instance();

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwVirtualNumber, &pItemData))
		return;

	CGraphicThing * pItemModel = pItemData->GetDropModelThing();

	TGroundItemInstance * pGroundItemInstance = m_GroundItemInstancePool.Alloc();
	pGroundItemInstance->dwVirtualNumber = dwVirtualNumber;

	bool bStabGround = false;

	if (bDrop)
	{
		z = CPythonBackground::Instance().GetHeight(x, y) + 10.0f;

#ifdef ENABLE_QUIVER_SYSTEM
		if (pItemData->GetType() == CItemData::ITEM_TYPE_WEAPON && pItemData->GetWeaponType() == CItemData::WEAPON_QUIVER)
			bStabGround = true;
#endif

		bStabGround = false;
		pGroundItemInstance->bAnimEnded = false;
	}
	else
		pGroundItemInstance->bAnimEnded = true;

	{
		// attaching effect
		CEffectManager & rem = CEffectManager::Instance();
		pGroundItemInstance->dwEffectInstanceIndex = rem.CreateEffect(m_dwDropItemEffectID, D3DXVECTOR3(x, -y, z), D3DXVECTOR3(0, 0, 0));

		pGroundItemInstance->eDropSoundType = __GetDropSoundType(*pItemData);
	}


	D3DXVECTOR3 normal;
	if (!CPythonBackground::Instance().GetNormal(int(x), int(y), &normal))
		normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	pGroundItemInstance->ThingInstance.Clear();
	pGroundItemInstance->ThingInstance.ReserveModelThing(1);
	pGroundItemInstance->ThingInstance.ReserveModelInstance(1);
	pGroundItemInstance->ThingInstance.RegisterModelThing(0, pItemModel);
	pGroundItemInstance->ThingInstance.SetModelInstance(0, 0, 0);
	if (bDrop)
	{
		pGroundItemInstance->v3EndPosition = D3DXVECTOR3(x, -y, z);
		pGroundItemInstance->ThingInstance.SetPosition(0, 0, 0);
	}
	else
		pGroundItemInstance->ThingInstance.SetPosition(x, -y, z);

	pGroundItemInstance->ThingInstance.Update();
	pGroundItemInstance->ThingInstance.Transform();
	pGroundItemInstance->ThingInstance.Deform();

	if (bDrop)
	{
		D3DXVECTOR3 vMin, vMax;
		pGroundItemInstance->ThingInstance.GetBoundBox(&vMin, &vMax);
		pGroundItemInstance->v3Center = (vMin + vMax) * 0.5f;

		/*std::array<std::pair<float, int>, 3> f = {std::make_pair(vMax.x - vMin.x, 0), std::make_pair(vMax.y - vMin.y, 1), std::make_pair(vMax.z - vMin.z, 2)};
		std::sort(f.begin(), f.end());*/
		std::pair<float, int> f[3] = { std::make_pair(vMax.x - vMin.x, 0), std::make_pair(vMax.y - vMin.y, 1), std::make_pair(vMax.z - vMin.z, 2) };
		std::sort(f, f + 3);

		D3DXVECTOR3 rEnd;

		if (bStabGround)
		{
			// »ÏÁ·
			if (f[2].second == 0) // axis x
			{
				rEnd.y = 90.0f + frandom(-15.0f, 15.0f);
				rEnd.x = frandom(0.0f, 360.0f);
				rEnd.z = frandom(-15.0f, 15.0f);
			}
			else if (f[2].second == 1) // axis y
			{
				rEnd.y = frandom(0.0f, 360.0f);
				rEnd.x = frandom(-15.0f, 15.0f);
				rEnd.z = 180.0f + frandom(-15.0f, 15.0f);
			}
			else // axis z
			{
				rEnd.y = 180.0f + frandom(-15.0f, 15.0f);
				rEnd.x = 0.0f + frandom(-15.0f, 15.0f);
				rEnd.z = frandom(0.0f, 360.0f);
			}
		}
		else
		{
			// ³ÐÀû
			// ¶¥ÀÇ ³ë¸»ÀÇ ¿µÇâÀ» ¹ÞÀ» °Í
			if (f[0].second == 0)
			{
				// y,z = by normal
				pGroundItemInstance->qEnd = RotationArc(D3DXVECTOR3((static_cast<float>(random() % 2)) * 2 - 1 + frandom(-0.1f, 0.1f),
																	0 + frandom(-0.1f, 0.1f), 0 + frandom(-0.1f, 0.1f)),
														D3DXVECTOR3(0, 0, 1) /*normal*/);
			}
			else if (f[0].second == 1)
			{
				pGroundItemInstance->qEnd =
					RotationArc(D3DXVECTOR3(0 + frandom(-0.1f, 0.1f), (static_cast<float>(random() % 2)) * 2 - 1 + frandom(-0.1f, 0.1f),
											0 + frandom(-0.1f, 0.1f)),
								D3DXVECTOR3(0, 0, 1) /*normal*/);
			}
			else
			{
				pGroundItemInstance->qEnd = RotationArc(D3DXVECTOR3(0 + frandom(-0.1f, 0.1f), 0 + frandom(-0.1f, 0.1f),
																	(static_cast<float>(random() % 2)) * 2 - 1 + frandom(-0.1f, 0.1f)),
														D3DXVECTOR3(0, 0, 1) /*normal*/);
			}
		}
		float rot = frandom(0, 2 * 3.1415926535f);
		D3DXQUATERNION q(0, 0, cosf(rot), sinf(rot));
		D3DXQuaternionMultiply(&pGroundItemInstance->qEnd, &pGroundItemInstance->qEnd, &q);
		q = RotationArc(D3DXVECTOR3(0, 0, 1), normal);
		D3DXQuaternionMultiply(&pGroundItemInstance->qEnd, &pGroundItemInstance->qEnd, &q);

		pGroundItemInstance->dwStartTime = CTimer::Instance().GetCurrentMillisecond();
		pGroundItemInstance->dwEndTime = pGroundItemInstance->dwStartTime + 300;
		pGroundItemInstance->v3RotationAxis.x = sinf(rot + 0); //frandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);
		pGroundItemInstance->v3RotationAxis.y = cosf(rot + 0); //frandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);
		pGroundItemInstance->v3RotationAxis.z = 0; //frandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);

		D3DXVECTOR3 v3Adjust = -pGroundItemInstance->v3Center;
		D3DXMATRIX mat;
		D3DXMatrixRotationQuaternion(&mat, &pGroundItemInstance->qEnd);

		D3DXVec3TransformCoord(&v3Adjust, &v3Adjust, &mat);
	}

	pGroundItemInstance->ThingInstance.Show();

	m_GroundItemInstanceMap.emplace(dwVirtualID, pGroundItemInstance);

	CPythonTextTail & rkTextTail = CPythonTextTail::Instance();

#ifdef ENABLE_EXTENDED_ITEMNAME_ON_GROUND
	static char szItemName[128];
	ZeroMemory(szItemName, sizeof(szItemName));
	int len = 0;
	switch (pItemData->GetType())
	{
		case CItemData::ITEM_TYPE_POLYMORPH:
		{
			const char* c_szTmp;
			CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
			rkNonPlayer.GetName(alSockets[0], &c_szTmp);
			len += snprintf(szItemName, sizeof(szItemName), "%s", c_szTmp);
			break;
		}

		case CItemData::ITEM_TYPE_SKILLBOOK:
		case CItemData::ITEM_TYPE_SKILLFORGET:
		{
			const uint32_t dwSkillVnum = (dwVirtualNumber == 50300 || dwVirtualNumber == 70037) ? alSockets[0] : 0;
			CPythonSkill::SSkillData * c_pSkillData;
			if ((dwSkillVnum != 0) && CPythonSkill::Instance().GetSkillData(dwSkillVnum, &c_pSkillData))
				len += snprintf(szItemName, sizeof(szItemName), "%s", c_pSkillData->GradeData[0].strName.c_str());

			break;
		}
	}

	len += snprintf(szItemName + len, sizeof(szItemName) - len, (len>0)?" %s":"%s", pItemData->GetName());

	bool bHasAttr = false;
	for (size_t i = 0; i < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++i)
	{
		if (aAttrs[i].wType != 0 && aAttrs[i].sValue != 0)
		{
			bHasAttr = true;
			break;
		}
	}
	rkTextTail.RegisterItemTextTail(
		dwVirtualID,
		szItemName,
		&pGroundItemInstance->ThingInstance,
		bHasAttr);
#else
	rkTextTail.RegisterItemTextTail(
		dwVirtualID,
		pItemData->GetName(),
		&pGroundItemInstance->ThingInstance);
#endif
}

void CPythonItem::SetOwnership(uint32_t dwVID, const char * c_pszName)
{
	auto itor = m_GroundItemInstanceMap.find(dwVID);

	if (m_GroundItemInstanceMap.end() == itor)
		return;

	TGroundItemInstance * pGroundItemInstance = itor->second;
	pGroundItemInstance->stOwnership.assign(c_pszName);

	CPythonTextTail & rkTextTail = CPythonTextTail::Instance();
	rkTextTail.SetItemTextTailOwner(dwVID, c_pszName);
}

bool CPythonItem::GetOwnership(uint32_t dwVID, const char ** c_pszName)
{
	auto itor = m_GroundItemInstanceMap.find(dwVID);

	if (m_GroundItemInstanceMap.end() == itor)
		return false;

	TGroundItemInstance * pGroundItemInstance = itor->second;
	*c_pszName = pGroundItemInstance->stOwnership.c_str();

	return true;
}

void CPythonItem::DeleteAllItems()
{
	CPythonTextTail & rkTextTail = CPythonTextTail::Instance();

	for (auto & i : m_GroundItemInstanceMap)
	{
		TGroundItemInstance * pGroundItemInst = i.second;
		rkTextTail.DeleteItemTextTail(i.first);
		pGroundItemInst->Clear();
		m_GroundItemInstancePool.Free(pGroundItemInst);
	}
	m_GroundItemInstanceMap.clear();
}

void CPythonItem::DeleteItem(uint32_t dwVirtualID)
{
	auto itor = m_GroundItemInstanceMap.find(dwVirtualID);
	if (m_GroundItemInstanceMap.end() == itor)
		return;

	TGroundItemInstance * pGroundItemInstance = itor->second;
	pGroundItemInstance->Clear();
	m_GroundItemInstancePool.Free(pGroundItemInstance);
	m_GroundItemInstanceMap.erase(itor);

	// Text Tail
	CPythonTextTail::Instance().DeleteItemTextTail(dwVirtualID);
}


bool CPythonItem::GetCloseMoney(const TPixelPosition & c_rPixelPosition, uint32_t * pdwItemID, uint32_t dwDistance)
{
	uint32_t dwCloseItemID = 0;
	uint32_t dwCloseItemDistance = 1000 * 1000;

	for (auto & i : m_GroundItemInstanceMap)
	{
		TGroundItemInstance * pInstance = i.second;

		if (pInstance->dwVirtualNumber != VNUM_MONEY)
			continue;

		auto dwxDistance = uint32_t(c_rPixelPosition.x) - uint32_t(pInstance->v3EndPosition.x); // @fixme022
		auto dwyDistance = uint32_t(c_rPixelPosition.y) - uint32_t(-pInstance->v3EndPosition.y); // @fixme022
		auto dwDistance = dwxDistance * dwxDistance + dwyDistance * dwyDistance;

		if (dwDistance < dwCloseItemDistance)
		{
			dwCloseItemID = i.first;
			dwCloseItemDistance = dwDistance;
		}
	}

	if (dwCloseItemDistance > float(dwDistance) * float(dwDistance))
		return false;

	*pdwItemID = dwCloseItemID;

	return true;
}

bool CPythonItem::GetCloseItem(const TPixelPosition & c_rPixelPosition, uint32_t * pdwItemID, uint32_t dwDistance)
{
	uint32_t dwCloseItemID = 0;
	uint32_t dwCloseItemDistance = 1000 * 1000;

	for (auto & i : m_GroundItemInstanceMap)
	{
#ifdef ENABLE_EXTENDED_CONFIGS
		//Dont collect armor/weapons if enabled option
		if (CPythonSystem::Instance().IsCollectEquipment() == false)
		{
			CItemData * pItemData;
			if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(i.first), &pItemData))
				continue;

			if (pItemData->GetType() == CItemData::ITEM_TYPE_WEAPON || pItemData->GetType() == CItemData::ITEM_TYPE_ARMOR)
				continue;
		}
		//End of system
#endif

		TGroundItemInstance * pInstance = i.second;

		auto dwxDistance = uint32_t(c_rPixelPosition.x - pInstance->v3EndPosition.x);
		auto dwyDistance = uint32_t(c_rPixelPosition.y - (-pInstance->v3EndPosition.y));
		auto dwDistance = uint32_t(dwxDistance * dwxDistance + dwyDistance * dwyDistance);

		if (dwxDistance * dwxDistance + dwyDistance * dwyDistance < dwCloseItemDistance)
		{
			dwCloseItemID = i.first;
			dwCloseItemDistance = dwDistance;
		}
	}

	if (dwCloseItemDistance > float(dwDistance) * float(dwDistance))
		return false;

	*pdwItemID = dwCloseItemID;

	return true;
}

BOOL CPythonItem::GetGroundItemPosition(uint32_t dwVirtualID, TPixelPosition * pPosition)
{
	auto itor = m_GroundItemInstanceMap.find(dwVirtualID);
	if (m_GroundItemInstanceMap.end() == itor)
		return FALSE;

	TGroundItemInstance * pInstance = itor->second;

	const D3DXVECTOR3 & rkD3DVct3 = pInstance->ThingInstance.GetPosition();

	pPosition->x = +rkD3DVct3.x;
	pPosition->y = -rkD3DVct3.y;
	pPosition->z = +rkD3DVct3.z;

	return TRUE;
}

uint32_t CPythonItem::__Pick(const POINT & c_rkPtMouse)
{
	float fu, fv, ft;

#ifdef ENABLE_GRAPHIC_ON_OFF
	CPythonTextTail& rkTextTailMgr = CPythonTextTail::Instance();
	int iPickID = rkTextTailMgr.Pick(c_rkPtMouse.x, c_rkPtMouse.y);
	if (iPickID < 0)
	{
		TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.begin();
		for (; itor != m_GroundItemInstanceMap.end(); ++itor)
		{
			TGroundItemInstance* pInstance = itor->second;

			if (pInstance->ThingInstance.Intersect(&fu, &fv, &ft))
			{
				return itor->first;
			}
		}
	}

	return iPickID;
#else
	auto itor = m_GroundItemInstanceMap.begin();
	for (; itor != m_GroundItemInstanceMap.end(); ++itor)
	{
		TGroundItemInstance * pInstance = itor->second;

		if (pInstance->ThingInstance.Intersect(&fu, &fv, &ft))
			return itor->first;
	}

	CPythonTextTail & rkTextTailMgr = CPythonTextTail::Instance();
	return rkTextTailMgr.Pick(c_rkPtMouse.x, c_rkPtMouse.y);
#endif
}

bool CPythonItem::GetPickedItemID(uint32_t * pdwPickedItemID) const
{
	if (INVALID_ID == m_dwPickedItemID)
		return false;

	*pdwPickedItemID = m_dwPickedItemID;
	return true;
}

uint32_t CPythonItem::GetVirtualNumberOfGroundItem(uint32_t dwVID)
{
	auto itor = m_GroundItemInstanceMap.find(dwVID);

	if (itor == m_GroundItemInstanceMap.end())
		return 0;

	return itor->second->dwVirtualNumber;
}

void CPythonItem::BuildNoGradeNameData(int iType) const
{
	/*
	CMapIterator<std::string, CItemData *> itor = CItemManager::Instance().GetItemNameMapIterator();

	m_NoGradeNameItemData.clear();
	m_NoGradeNameItemData.reserve(1024);

	while (++itor)
	{
		CItemData * pItemData = *itor;
		if (iType == pItemData->GetType())
			m_NoGradeNameItemData.emplace_back(pItemData);
	}
	*/
}

uint32_t CPythonItem::GetNoGradeNameDataCount() const
{
	return m_NoGradeNameItemData.size();
}

CItemData * CPythonItem::GetNoGradeNameDataPtr(uint32_t dwIndex)
{
	if (dwIndex >= m_NoGradeNameItemData.size())
		return nullptr;

	return m_NoGradeNameItemData[dwIndex];
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
#include "PythonPlayer.h"

static bool _CanAddTransmutationItem(const CItemData* target)
{
	const bool bType = CPythonPlayer::Instance().GetChangeLookWindowType();
	const uint8_t bItemType = target->GetType();
	const uint8_t bItemSubType = target->GetSubType();
	const uint32_t dwIndex = target->GetIndex();

	switch (static_cast<ETRANSMUTATIONTYPE>(bType))
	{
		case ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_MOUNT:
		{
			if (bItemType == CItemData::EItemType::ITEM_TYPE_QUEST && (dwIndex == 50051 || dwIndex == 50052 || dwIndex == 50053))
				return true;

			if (bItemType == CItemData::EItemType::ITEM_TYPE_COSTUME && bItemSubType == CItemData::ECostumeSubTypes::COSTUME_MOUNT)
				return true;
		}
		break;

		case ETRANSMUTATIONTYPE::TRANSMUTATION_TYPE_ITEM:
		{
			if (bItemType == CItemData::EItemType::ITEM_TYPE_WEAPON && bItemSubType != CItemData::EWeaponSubTypes::WEAPON_ARROW)
				return true;

			if (bItemType == CItemData::EItemType::ITEM_TYPE_ARMOR && bItemSubType == CItemData::EArmorSubTypes::ARMOR_BODY)
				return true;

			if (bItemType == CItemData::EItemType::ITEM_TYPE_COSTUME && bItemSubType == CItemData::ECostumeSubTypes::COSTUME_BODY)
				return true;
		}
		break;

		default:
			break;
	}

	return false;
}

static bool _CheckOtherTransmutationItem(const CItemData* target, const CItemData* material)
{
	if (target == material)
		return false;

	if (material->GetIndex() == target->GetIndex())
		return false;

	if (target->GetIndex() >= 50051 && target->GetIndex() <= 50053)
	{
		if (material->GetType() == CItemData::EItemType::ITEM_TYPE_COSTUME && material->GetSubType() != CItemData::ECostumeSubTypes::COSTUME_MOUNT)
			return false;
	}
	else
	{
		if (material->GetType() != target->GetType())
			return false;

		if (material->GetSubType() != target->GetSubType())
			return false;

		if (material->GetAntiFlags() != target->GetAntiFlags())
			return false;
	}

	return true;
}

bool CPythonItem::CanAddChangeLookItem(const CItemData* item, const CItemData* other_item) const
{
	if (item == nullptr)
		return false;

	if (other_item)
	{
		if (!_CheckOtherTransmutationItem(item, other_item))
			return false;
	}

	else if (!_CanAddTransmutationItem(item))
		return false;

	return true;
}

bool CPythonItem::CanAddChangeLookFreeItem(const uint32_t dwVnum) const
{
	return dwVnum == static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_TICKET_1)
		|| dwVnum == static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_TICKET_2);
}

bool CPythonItem::IsChangeLookClearScrollItem(const uint32_t dwVnum) const
{
	return dwVnum == static_cast<uint32_t>(ETRANSMUTATIONSETTINGS::TRANSMUTATION_CLEAR_SCROLL);
}
#endif

void CPythonItem::Destroy()
{
	DeleteAllItems();
	m_GroundItemInstancePool.Clear();
}

void CPythonItem::Create()
{
	CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/etc/dropitem/dropitem.mse", &m_dwDropItemEffectID);
#ifdef ENABLE_GRAPHIC_ON_OFF
	CEffectManager::Instance().SetDropItemEffectID(m_dwDropItemEffectID);
#endif
}

CPythonItem::CPythonItem()
{
	m_GroundItemInstancePool.SetName("CDynamicPool<TGroundItemInstance>");
	m_dwPickedItemID = INVALID_ID;
}

CPythonItem::~CPythonItem()
{
	assert(m_GroundItemInstanceMap.empty());
}