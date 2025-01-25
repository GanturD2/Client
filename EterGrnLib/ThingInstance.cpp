#include "StdAfx.h"
#include "../EterBase/Debug.h"
#include "../EterLib/Camera.h"
#include "../EterBase/Timer.h"

//#include "../UserInterface/PythonSystem.h"
#include "ThingInstance.h"
#include "Thing.h"
#include "ModelInstance.h"
#ifdef ENABLE_SCALE_SYSTEM
#	include "../GameLib/GameType.h"
#	include "../GameLib/RaceData.h"
#endif

#define LODControllerLambda [&](auto & elem) { f(elem.get()); }

CDynamicPool<CGraphicThingInstance> CGraphicThingInstance::ms_kPool;

CGraphicThing * CGraphicThingInstance::GetBaseThingPtr()
{
	if (m_modelThingSetVector.empty())
		return nullptr;

	TModelThingSet & rkModelThingSet = m_modelThingSetVector[0];
	if (rkModelThingSet.m_pLODThingRefVector.empty())
		return nullptr;

	auto & proThing = rkModelThingSet.m_pLODThingRefVector[0];
	if (!proThing)
		return nullptr;

	CGraphicThing::TRef roThing = *proThing;
	return roThing.GetPointer();
}

bool CGraphicThingInstance::LessRenderOrder(CGraphicThingInstance * pkThingInst)
{
	return (GetBaseThingPtr() < pkThingInst->GetBaseThingPtr());
}

void CGraphicThingInstance::CreateSystem(uint32_t uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicThingInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicThingInstance * CGraphicThingInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicThingInstance::Delete(CGraphicThingInstance * pkThingInst)
{
	pkThingInst->Clear();
	ms_kPool.Free(pkThingInst);
}

void CGraphicThingInstance::SetMotionAtEnd()
{
	CGrannyLODController::FEndStopMotionPointer f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

bool CGraphicThingInstance::Picking(const D3DXVECTOR3 & v, const D3DXVECTOR3 & dir, float & out_x, float & out_y) const
{
	if (!m_pHeightAttributeInstance)
		return false;
	return m_pHeightAttributeInstance->Picking(v, dir, out_x, out_y);
}


void CGraphicThingInstance::OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector)
{
	assert(pscdVector);
	for (const auto & it : *pscdVector)
		AddCollision(&it, &GetTransform());
}

void CGraphicThingInstance::OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance)
{
	assert(pAttributeInstance);
	SetHeightInstance(pAttributeInstance);
}

bool CGraphicThingInstance::OnGetObjectHeight(float fX, float fY, float * pfHeight)
{
	return m_pHeightAttributeInstance && m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight);
}

void CGraphicThingInstance::BuildBoundingSphere()
{
	D3DXVECTOR3 v3Min, v3Max;
	GetBoundBox(0, &v3Min, &v3Max);
	m_v3Center = (v3Min + v3Max) * 0.5f;
	D3DXVECTOR3 vDelta = (v3Max - v3Min);

	m_fRadius = D3DXVec3Length(&vDelta) * 0.5f + 50.0f; // extra length for attached objects
}

bool CGraphicThingInstance::GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius)
{
	if (m_fRadius <= 0)
	{
		BuildBoundingSphere();

		fRadius = m_fRadius;
		v3Center = m_v3Center;
	}
	else
	{
		fRadius = m_fRadius;
		v3Center = m_v3Center;
	}

	D3DXVec3TransformCoord(&v3Center, &v3Center, &GetTransform());
	return true;
}

void CGraphicThingInstance::BuildBoundingAABB()
{
	D3DXVECTOR3 v3Min, v3Max;
	GetBoundBox(0, &v3Min, &v3Max);
	m_v3Center = (v3Min + v3Max) * 0.5f;
	m_v3Min = v3Min;
	m_v3Max = v3Max;
}

bool CGraphicThingInstance::GetBoundingAABB(D3DXVECTOR3 & v3Min, D3DXVECTOR3 & v3Max)
{
	BuildBoundingAABB();

	v3Min = m_v3Min;
	v3Max = m_v3Max;

	D3DXVec3TransformCoord(&m_v3Center, &m_v3Center, &GetTransform());
	return true;
}

void CGraphicThingInstance::CalculateBBox()
{
	GetBoundBox(&m_v3BBoxMin, &m_v3BBoxMax);

	m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const D3DXMATRIX & c_rmatTransform = GetTransform();

	for (uint32_t i = 0; i < 8; ++i)
	{
		D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
		if (0 == i)
		{
			m_v3TBBoxMin.x = m_v4TBBox[i].x;
			m_v3TBBoxMin.y = m_v4TBBox[i].y;
			m_v3TBBoxMin.z = m_v4TBBox[i].z;
			m_v3TBBoxMax.x = m_v4TBBox[i].x;
			m_v3TBBoxMax.y = m_v4TBBox[i].y;
			m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
		else
		{
			if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}
}

bool CGraphicThingInstance::CreateDeviceObjects()
{
	CGrannyLODController::FCreateDeviceObjects f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
	return true;
}

void CGraphicThingInstance::DestroyDeviceObjects()
{
	CGrannyLODController::FDestroyDeviceObjects f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

bool CGraphicThingInstance::CheckModelInstanceIndex(int iModelInstance) const
{
	if (iModelInstance < 0)
		return false;

	int max = m_LODControllerVector.size();

	return iModelInstance < max;
}

bool CGraphicThingInstance::CheckModelThingIndex(int iModelThing) const
{
	if (iModelThing < 0)
		return false;

	int max = m_modelThingSetVector.size();

	return iModelThing < max;
}

bool CGraphicThingInstance::CheckMotionThingIndex(uint32_t dwMotionKey)
{
	auto itor = m_roMotionThingMap.find(dwMotionKey);

	return m_roMotionThingMap.end() != itor;
}

bool CGraphicThingInstance::GetMotionThingPointer(uint32_t dwKey, CGraphicThing ** ppMotion)
{
	if (!CheckMotionThingIndex(dwKey))
		return false;

	*ppMotion = m_roMotionThingMap[dwKey]->GetPointer();
	return true;
}

bool CGraphicThingInstance::IsMotionThing() const
{
	return !m_roMotionThingMap.empty();
}

void CGraphicThingInstance::ReserveModelInstance(int iCount)
{
	m_LODControllerVector.clear();
	m_LODControllerVector.resize(iCount);
	for (auto & elem : m_LODControllerVector)
		elem = std::make_unique<CGrannyLODController>();
}

void CGraphicThingInstance::ReserveModelThing(int iCount)
{
	m_modelThingSetVector.resize(iCount);
}

bool CGraphicThingInstance::FindBoneIndex(int iModelInstance, const char * c_szBoneName, int * iRetBone)
{
	assert(CheckModelInstanceIndex(iModelInstance));

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[iModelInstance]->GetModelInstance();

	if (!pModelInstance)
		return false;

	return pModelInstance->GetBoneIndexByName(c_szBoneName, iRetBone);
}

void CGraphicThingInstance::AttachModelInstance(int iDstModelInstance, const char * c_szBoneName, int iSrcModelInstance)
{
	if (!CheckModelInstanceIndex(iSrcModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)",
				   iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)",
				   iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}

	CGrannyLODController * pSrcLODController = m_LODControllerVector[iSrcModelInstance].get();
	CGrannyLODController * pDstLODController = m_LODControllerVector[iDstModelInstance].get();
	pDstLODController->AttachModelInstance(pSrcLODController, c_szBoneName);
}

void CGraphicThingInstance::AttachModelInstance(int iDstModelInstance, const char * c_szBoneName, CGraphicThingInstance & rSrcInstance,
												int iSrcModelInstance)
{
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)",
				   iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}
	if (!rSrcInstance.CheckModelInstanceIndex(iSrcModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)",
				   iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}

	CGrannyLODController * pDstLODController = m_LODControllerVector[iDstModelInstance].get();
	CGrannyLODController * pSrcLODController = rSrcInstance.m_LODControllerVector[iSrcModelInstance].get();
	pDstLODController->AttachModelInstance(pSrcLODController, c_szBoneName);
}

void CGraphicThingInstance::DetachModelInstance(int iDstModelInstance, CGraphicThingInstance & rSrcInstance, int iSrcModelInstance)
{
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, iSrcModelInstance=%d)", iDstModelInstance,
				   iSrcModelInstance);
		return;
	}
	if (!rSrcInstance.CheckModelInstanceIndex(iSrcModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, iSrcModelInstance=%d)", iDstModelInstance,
				   iSrcModelInstance);
		return;
	}

	CGrannyLODController * pDstLODController = m_LODControllerVector[iDstModelInstance].get();
	CGrannyLODController * pSrcLODController = rSrcInstance.m_LODControllerVector[iSrcModelInstance].get();
	pDstLODController->DetachModelInstance(pSrcLODController);
}

bool CGraphicThingInstance::GetBonePosition(int iModelIndex, int iBoneIndex, float * pfx, float * pfy, float * pfz)
{
	assert(CheckModelInstanceIndex(iModelIndex));

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[iModelIndex]->GetModelInstance();

	if (!pModelInstance)
		return false;

	const float * pfMatrix = pModelInstance->GetBoneMatrixPointer(iBoneIndex);

	*pfx = pfMatrix[12];
	*pfy = pfMatrix[13];
	*pfz = pfMatrix[14];
	return true;
}
//iSkelInstance �� ������ �⺻ ���� Link(���� �ٴ°�)��Ű��,
//������ �⺻ ���� attach(��ǥ�� ������ ���°�) �˴ϴ�.
bool CGraphicThingInstance::SetModelInstance(int iDstModelInstance, int iSrcModelThing, int iSrcModel, int iSkelInstance)
{
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::SetModelInstance(iDstModelInstance=%d, pModelThing=%d, iSrcModel=%d)\n", iDstModelInstance,
				   iSrcModelThing, iSrcModel);
		return false;
	}
	if (!CheckModelThingIndex(iSrcModelThing))
	{
		TraceError("CGraphicThingInstance::SetModelInstance(iDstModelInstance=%d, pModelThing=%d, iSrcModel=%d)\n", iDstModelInstance,
				   iSrcModelThing, iSrcModel);
		return false;
	}

	CGrannyLODController * pController = m_LODControllerVector[iDstModelInstance].get();
	if (!pController)
		return false;

	// HAIR_LINK
	CGrannyLODController * pSkelController = nullptr;
	if (iSkelInstance != DONTUSEVALUE)
	{
		if (!CheckModelInstanceIndex(iSkelInstance))
		{
			TraceError("CGraphicThingInstance::SetModelInstanceByOtherSkeletonInstance(iSkelInstance=%d, iDstModelInstance=%d, "
					   "pModelThing=%d, iSrcModel=%d)\n",
					   iSkelInstance, iDstModelInstance, iSrcModelThing, iSrcModel);
			return false;
		}
		pSkelController = m_LODControllerVector[iSkelInstance].get();
		if (!pSkelController)
			return false;
	}
	// END_OF_HAIR_LINK

	TModelThingSet & rModelThingSet = m_modelThingSetVector[iSrcModelThing];

	pController->Clear();


	for (auto & i : rModelThingSet.m_pLODThingRefVector)
	{
		if (i->IsNull())
			return false;

		pController->AddModel(i->GetPointer(), iSrcModel, pSkelController);
	}
	return true;
}

void CGraphicThingInstance::SetMaterialImagePointer(uint32_t ePart, const char * c_szImageName, CGraphicImage * pImage)
{
	if (ePart >= m_LODControllerVector.size())
	{
		TraceError(
			"CGraphicThingInstance::SetMaterialImagePointer(ePart(%d)<uPartCount(%d), c_szImageName=%s, pImage=%s) - ePart OUT OF RANGE",
			ePart, m_LODControllerVector.size(), c_szImageName, pImage->GetFileName());

		return;
	}

	if (!m_LODControllerVector[ePart])
	{
		TraceError("CGraphicThingInstance::SetMaterialImagePointer(ePart(%d), c_szImageName=%s, pImage=%s) - ePart Data is nullptr", ePart,
				   m_LODControllerVector.size(), c_szImageName, pImage->GetFileName());

		return;
	}

	m_LODControllerVector[ePart]->SetMaterialImagePointer(c_szImageName, pImage);
}

void CGraphicThingInstance::SetMaterialData(uint32_t ePart, const char * c_szImageName, SMaterialData kMaterialData)
{
	if (ePart >= m_LODControllerVector.size())
	{
		TraceError("CGraphicThingInstance::SetMaterialData(ePart(%d)<uPartCount(%d)) - ePart OUT OF RANGE", ePart,
				   m_LODControllerVector.size());

		return;
	}

	if (!m_LODControllerVector[ePart])
	{
		TraceError("CGraphicThingInstance::SetMaterialData(ePart(%d)) - ePart Data is nullptr", ePart, m_LODControllerVector.size());

		return;
	}

	m_LODControllerVector[ePart]->SetMaterialData(c_szImageName, kMaterialData);
}

void CGraphicThingInstance::SetSpecularInfo(uint32_t ePart, const char * c_szMtrlName, BOOL bEnable, float fPower)
{
	if (ePart >= m_LODControllerVector.size())
	{
		TraceError("CGraphicThingInstance::SetSpecularInfo(ePart(%d)<uPartCount(%d)) - ePart OUT OF RANGE", ePart,
				   m_LODControllerVector.size());

		return;
	}

	if (!m_LODControllerVector[ePart])
	{
		TraceError("CGraphicThingInstance::SetSpecularInfo(ePart(%d)) - ePart Data is nullptr", ePart, m_LODControllerVector.size());

		return;
	}

	m_LODControllerVector[ePart]->SetSpecularInfo(c_szMtrlName, bEnable, fPower);
}

bool CGraphicThingInstance::SetMotion(uint32_t dwMotionKey, float blendTime, int loopCount, float speedRatio)
{
	if (!CheckMotionThingIndex(dwMotionKey))
		return false;

	auto itor = m_roMotionThingMap.find(dwMotionKey);
	auto & proMotionThing = itor->second;
	CGraphicThing * pMotionThing = proMotionThing->GetPointer();

	if (!pMotionThing)
		return false;

	if (!pMotionThing->CheckMotionIndex(0))
		return false;

	CGrannyLODController::FSetMotionPointer f{};
	f.m_pMotion = pMotionThing->GetMotionPointer(0);
	f.m_blendTime = blendTime;
	f.m_loopCount = loopCount;
	f.m_speedRatio = speedRatio;

	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
	return true;
}

bool CGraphicThingInstance::ChangeMotion(uint32_t dwMotionKey, int loopCount, float speedRatio)
{
	if (!CheckMotionThingIndex(dwMotionKey))
		return false;

	auto itor = m_roMotionThingMap.find(dwMotionKey);
	auto & proMotionThing = itor->second;
	CGraphicThing * pMotionThing = proMotionThing->GetPointer();

	if (!pMotionThing)
		return false;

	if (!pMotionThing->CheckMotionIndex(0))
		return false;

	CGrannyLODController::FChangeMotionPointer f{};
	f.m_pMotion = pMotionThing->GetMotionPointer(0);
	f.m_loopCount = loopCount;
	f.m_speedRatio = speedRatio;

	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
	return true;
}

void CGraphicThingInstance::SetEndStopMotion()
{
	CGrannyLODController::FEndStopMotionPointer f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::RegisterModelThing(int iModelThing, CGraphicThing * pModelThing)
{
	if (!CheckModelThingIndex(iModelThing))
	{
		TraceError("CGraphicThingInstance::RegisterModelThing(iModelThing=%d, pModelThing=%s)\n", iModelThing, pModelThing->GetFileName());
		return;
	}

	m_modelThingSetVector[iModelThing].Clear();

	if (pModelThing)
		RegisterLODThing(iModelThing, pModelThing);
}

void CGraphicThingInstance::RegisterLODThing(int iModelThing, CGraphicThing * pModelThing)
{
	assert(CheckModelThingIndex(iModelThing));
	auto pModelRef = std::make_unique<CGraphicThing::TRef>();
	pModelRef->SetPointer(pModelThing);
	m_modelThingSetVector[iModelThing].m_pLODThingRefVector.emplace_back(std::move(pModelRef));
}

void CGraphicThingInstance::RegisterMotionThing(uint32_t dwMotionKey, CGraphicThing * pMotionThing)
{
	auto pMotionRef = std::make_unique<CGraphicThing::TRef>();
	pMotionRef->SetPointer(pMotionThing);
	m_roMotionThingMap.emplace(dwMotionKey, std::move(pMotionRef));
}

void CGraphicThingInstance::ResetLocalTime()
{
	m_fLastLocalTime = 0.0f;
	m_fLocalTime = 0.0f;

	CGrannyLODController::FResetLocalTime f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::InsertDelay(float fDelay)
{
	m_fDelay = fDelay;
}

float CGraphicThingInstance::GetLastLocalTime() const
{
	return m_fLastLocalTime;
}

float CGraphicThingInstance::GetLocalTime() const
{
	return m_fLocalTime;
}

float CGraphicThingInstance::GetSecondElapsed() const
{
	return m_fSecondElapsed;
}

float CGraphicThingInstance::GetAverageSecondElapsed() const
{
	return m_fAverageSecondElapsed;
}

bool CGraphicThingInstance::Intersect(float * pu, float * pv, float * pt)
{
	if (!isShow())
		return false;
	if (!m_bUpdated)
		return false;

	if (m_LODControllerVector.empty())
		return false;

	return m_LODControllerVector[0]->Intersect(&GetTransform(), pu, pv, pt);
}

void CGraphicThingInstance::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	vtMin->x = vtMin->y = vtMin->z = 100000.0f;
	vtMax->x = vtMax->y = vtMax->z = -100000.0f;
	CGrannyLODController::FBoundBox f(vtMin, vtMax);
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

BOOL CGraphicThingInstance::GetBoundBox(uint32_t dwModelInstanceIndex, D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	if (!CheckModelInstanceIndex(dwModelInstanceIndex))
		return FALSE;

	vtMin->x = vtMin->y = vtMin->z = 100000.0f;
	vtMax->x = vtMax->y = vtMax->z = -100000.0f;

	CGrannyLODController * pController = m_LODControllerVector[dwModelInstanceIndex].get();
	if (!pController->isModelInstance())
		return FALSE;

	CGrannyModelInstance * pModelInstance = pController->GetModelInstance();
	pModelInstance->GetBoundBox(vtMin, vtMax);
	return TRUE;
}

BOOL CGraphicThingInstance::GetBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, D3DXMATRIX ** ppMatrix)
{
	if (!CheckModelInstanceIndex(dwModelInstanceIndex))
		return FALSE;

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[dwModelInstanceIndex]->GetModelInstance();
	if (!pModelInstance)
		return FALSE;

	*ppMatrix = reinterpret_cast<D3DXMATRIX *>(pModelInstance->GetBoneMatrixPointer(dwBoneIndex));
	if (!*ppMatrix)
		return FALSE;

	return TRUE;
}

BOOL CGraphicThingInstance::GetCompositeBoneMatrix(uint32_t dwModelInstanceIndex, uint32_t dwBoneIndex, D3DXMATRIX ** ppMatrix)
{
	if (!CheckModelInstanceIndex(dwModelInstanceIndex))
		return FALSE;

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[dwModelInstanceIndex]->GetModelInstance();
	if (!pModelInstance)
		return FALSE;

	*ppMatrix = reinterpret_cast<D3DXMATRIX *>(pModelInstance->GetCompositeBoneMatrixPointer(dwBoneIndex));

	return TRUE;
}

void CGraphicThingInstance::UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed, int iModelInstanceIndex)
{
	int nLODCount = m_LODControllerVector.size();
	if (iModelInstanceIndex >= nLODCount)
		return;

	CGrannyLODController * pkLODCtrl = m_LODControllerVector[iModelInstanceIndex].get();
	if (!pkLODCtrl)
		return;

	CGrannyModelInstance * pModelInstance = pkLODCtrl->GetModelInstance();
	if (!pModelInstance)
		return;

	pModelInstance->UpdateTransform(pMatrix, fSecondsElapsed);
}

#ifdef ENABLE_SCALE_SYSTEM
void CGraphicThingInstance::RecalcAccePositionMatrixFromBoneMatrix()
{
	if (m_LODControllerVector.empty())
		return;

	CGrannyModelInstance* pModelInstance = m_LODControllerVector[0]->GetModelInstance();
	if (!pModelInstance)
		return;

	int iBoneIndex = 0;
	pModelInstance->GetBoneIndexByName("Bip01 Spine2", &iBoneIndex);
	const D3DXMATRIX* cmatBoneMatrix = (D3DXMATRIX*)pModelInstance->GetBoneMatrixPointer(iBoneIndex);
	if (cmatBoneMatrix)
	{
		D3DXVECTOR3 v3ScaleCenter = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		float fAccePosX = cmatBoneMatrix->_41;
		float fAccePosY = cmatBoneMatrix->_42;
		float fAccePosZ = cmatBoneMatrix->_43;
		D3DXMatrixIdentity(&m_matScale);
		if (m_bAttachedAcceRace)
		{
			v3ScaleCenter.x = fAccePosX;
			v3ScaleCenter.y = fAccePosY;
		}
		else
		{
			v3ScaleCenter.x = fAccePosX - 18.0f;
			v3ScaleCenter.y = -40.0f;
		}
		v3ScaleCenter.z = fAccePosZ;
		D3DXQUATERNION qRot = D3DXQUATERNION(0.0f, 0.0f, 0.0f, 0.0f);
		D3DXMatrixTransformation(&m_matScale, &v3ScaleCenter, &qRot, &m_v3ScaleAcce, nullptr, nullptr, nullptr);
	}

	D3DXMATRIX matTemp;
	D3DXMatrixMultiply(&matTemp, &m_matScaleWorld, &m_matScale);
	m_matAbsoluteTrans = matTemp * m_mRotation;
	m_matAbsoluteTrans._41 += m_v3Position.x;
	m_matAbsoluteTrans._42 += m_v3Position.y;
	m_matAbsoluteTrans._43 += m_v3Position.z;
}
#endif

#ifdef ENABLE_SCALE_SYSTEM
void CGraphicThingInstance::DeformAll()
{
	m_bUpdated = true;

	for (std::vector<CGrannyLODController*>::size_type i = 0; i != m_LODControllerVector.size(); i++)
	{
		CGrannyLODController* pkLOD = m_LODControllerVector[i].get();
		if (pkLOD && pkLOD->isModelInstance())
		{
			if (i == CRaceData::PART_ACCE)
			{
				RecalcAccePositionMatrixFromBoneMatrix();
				pkLOD->DeformAll(&m_matAbsoluteTrans);
			}
			else
				pkLOD->DeformAll(&m_worldMatrix);
		}
	}
}

void CGraphicThingInstance::DeformNoSkin()
{
	m_bUpdated = true;

	for (std::vector<CGrannyLODController*>::size_type i = 0; i != m_LODControllerVector.size(); i++)
	{
		CGrannyLODController* pkLOD = m_LODControllerVector[i].get();
		if (pkLOD && pkLOD->isModelInstance())
		{
			if (i == CRaceData::PART_ACCE)
			{
				RecalcAccePositionMatrixFromBoneMatrix();
				pkLOD->DeformNoSkin(&m_matAbsoluteTrans);
			}
			else
				pkLOD->DeformNoSkin(&m_worldMatrix);
		}
	}
}

void CGraphicThingInstance::OnDeform()
{
	m_bUpdated = true;

	for (std::vector<CGrannyLODController*>::size_type i = 0; i != m_LODControllerVector.size(); i++)
	{
		CGrannyLODController* pkLOD = m_LODControllerVector[i].get();
		if (pkLOD && pkLOD->isModelInstance())
		{
			if (i == CRaceData::PART_ACCE)
			{
				RecalcAccePositionMatrixFromBoneMatrix();
				pkLOD->Deform(&m_matAbsoluteTrans);
			}
			else
				pkLOD->Deform(&m_worldMatrix);
		}
	}
}
#else
void CGraphicThingInstance::DeformAll()
{
	m_bUpdated = true;

	CGrannyLODController::FDeformAll f{};
	f.mc_pWorldMatrix = &m_worldMatrix;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::DeformNoSkin()
{
	m_bUpdated = true;

	for (auto & pkLOD : m_LODControllerVector)
	{
		if (pkLOD->isModelInstance())
			pkLOD->DeformNoSkin(&m_worldMatrix);
	}
}

void CGraphicThingInstance::OnDeform()
{
	m_bUpdated = true;

	CGrannyLODController::FDeform f{};
	f.mc_pWorldMatrix = &m_worldMatrix;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}
#endif

void CGraphicThingInstance::__SetLocalTime(float fLocalTime)
{
	m_fLastLocalTime = m_fLocalTime;
	m_fLocalTime = fLocalTime;

	CGrannyLODController::FSetLocalTime f{};
	f.fLocalTime = fLocalTime;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::UpdateLODLevel()
{
	CCamera * pcurCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pcurCamera)
	{
		TraceError("CGraphicThingInstance::UpdateLODLevel - GetCurrentCamera() == nullptr");
		return;
	}

	const D3DXVECTOR3 & c_rv3TargetPosition = pcurCamera->GetTarget();
	const D3DXVECTOR3 & c_rv3CameraPosition = pcurCamera->GetEye();
	const D3DXVECTOR3 & c_v3Position = GetPosition();

	// NOTE : �߽����κ����� �Ÿ� ��꿡 z�� ���̴� ������� �ʴ´�. - [levites]
	CGrannyLODController::FUpdateLODLevel f{};
	f.fDistanceFromCenter = sqrtf((c_rv3TargetPosition.x - c_v3Position.x) * (c_rv3TargetPosition.x - c_v3Position.x) +
								  (c_rv3TargetPosition.y - c_v3Position.y) * (c_rv3TargetPosition.y - c_v3Position.y));
	f.fDistanceFromCamera = sqrtf((c_rv3CameraPosition.x - c_v3Position.x) * (c_rv3CameraPosition.x - c_v3Position.x) +
								  (c_rv3CameraPosition.y - c_v3Position.y) * (c_rv3CameraPosition.y - c_v3Position.y) +
								  (c_rv3CameraPosition.z - c_v3Position.z) * (c_rv3CameraPosition.z - c_v3Position.z));

	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::UpdateTime()
{
	m_fSecondElapsed = CTimer::Instance().GetElapsedSecond();

	if (m_fDelay > m_fSecondElapsed)
	{
		m_fDelay -= m_fSecondElapsed;
		m_fSecondElapsed = 0.0f;
	}
	else
	{
		m_fSecondElapsed -= m_fDelay;
		m_fDelay = 0.0f;
	}

	m_fLastLocalTime = m_fLocalTime;
	m_fLocalTime += m_fSecondElapsed;
	m_fAverageSecondElapsed = m_fAverageSecondElapsed + (m_fSecondElapsed - m_fAverageSecondElapsed) / 4.0f;

	CGrannyLODController::FUpdateTime f{};
	f.fElapsedTime = m_fSecondElapsed;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::OnUpdate()
{
	UpdateLODLevel();
	UpdateTime();
}

void CGraphicThingInstance::OnRender()
{
	RenderWithOneTexture();
}

void CGraphicThingInstance::OnBlendRender()
{
	BlendRenderWithOneTexture();
}

void CGraphicThingInstance::RenderWithOneTexture()
{
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderWithOneTexture f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::BlendRenderWithOneTexture()
{
	if (!m_bUpdated)
		return;

	CGrannyLODController::FBlendRenderWithOneTexture f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::RenderWithTwoTexture()
{
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderWithTwoTexture f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::BlendRenderWithTwoTexture()
{
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderWithTwoTexture f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::OnRenderToShadowMap()
{
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderToShadowMap f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::OnRenderShadow()
{
	CGrannyLODController::FRenderShadow f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

void CGraphicThingInstance::OnRenderPCBlocker()
{
	CGrannyLODController::FRenderWithOneTexture f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

uint32_t CGraphicThingInstance::GetLODControllerCount() const
{
	return m_LODControllerVector.size();
}

CGrannyLODController * CGraphicThingInstance::GetLODControllerPointer(uint32_t dwModelIndex) const
{
	assert(dwModelIndex < m_LODControllerVector.size());
	return m_LODControllerVector[dwModelIndex].get();
}

CGrannyLODController * CGraphicThingInstance::GetLODControllerPointer(uint32_t dwModelIndex)
{
	assert(dwModelIndex < m_LODControllerVector.size());
	return m_LODControllerVector[dwModelIndex].get();
}

uint8_t CGraphicThingInstance::GetLODLevel(uint32_t dwModelInstance)
{
	assert(dwModelInstance < m_LODControllerVector.size());
	return (m_LODControllerVector[dwModelInstance]->GetLODLevel());
}

float CGraphicThingInstance::GetWidth()
{
	if (m_LODControllerVector.empty())
		return 0.0f;

	CGrannyModelInstance* pModelInstance = m_LODControllerVector[0]->GetModelInstance();
	if (!pModelInstance)
		return 0.0f;

	D3DXVECTOR3 vtMin, vtMax;
	pModelInstance->GetBoundBox(&vtMin, &vtMax);

	return fabs(vtMin.x - vtMax.x);
}

#ifdef ENABLE_SCALE_SYSTEM_TEXTAIL
float CGraphicThingInstance::GetHeight(bool bScaleX)
#else
float CGraphicThingInstance::GetHeight()
#endif
{
	if (m_LODControllerVector.empty())
		return 0.0f;

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[0]->GetModelInstance();
	if (!pModelInstance)
		return 0.0f;

#ifdef ENABLE_SCALE_SYSTEM_TEXTAIL
	D3DXVECTOR3 vtMin, vtMax, vtScale = GetScale();
#else
	D3DXVECTOR3 vtMin, vtMax;
#endif
	pModelInstance->GetBoundBox(&vtMin, &vtMax);

#ifdef ENABLE_SCALE_SYSTEM_TEXTAIL
	if (bScaleX)
		return vtScale.x * (vtMax.x - vtMin.x);

	return vtScale.z * (vtMax.z - vtMin.z);
#else
	return fabs(vtMin.z - vtMax.z);
#endif
}

void CGraphicThingInstance::ReloadTexture()
{
	CGrannyLODController::FReloadTexture f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), LODControllerLambda);
}

bool CGraphicThingInstance::HaveBlendThing()
{
	for (auto & i : m_LODControllerVector)
	{
		if (i->HaveBlendThing())
			return true;
	}
	return false;
}


void CGraphicThingInstance::OnClear()
{
	m_LODControllerVector.clear();
	m_roMotionThingMap.clear();
	for (auto & d : m_modelThingSetVector)
		d.Clear();
}

void CGraphicThingInstance::OnInitialize()
{
	m_bUpdated = false;
	m_fLastLocalTime = 0.0f;
	m_fLocalTime = 0.0f;
	m_fDelay = 0.0;
	m_fSecondElapsed = 0.0f;
	m_fAverageSecondElapsed = 0.03f;
	m_fRadius = -1.0f;
	m_v3Center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	ResetLocalTime();
#ifdef ENABLE_RENDER_LOGIN_EFFECTS
	m_bIsLoginRender = false;
#endif
}

CGraphicThingInstance::CGraphicThingInstance()
{
	Initialize();
}

CGraphicThingInstance::~CGraphicThingInstance() = default;
