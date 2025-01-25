#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "AbstractPlayer.h"
#include "Packet.h"

#include "../EterLib/Camera.h"
#ifdef ENABLE_MINI_GAME_YUTNORI
#	include "PythonYutnoriManager.h"
#endif
#ifdef ENABLE_AUTO_SYSTEM
# include "PythonPlayer.h"
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Frame Process

int CHAR_STAGE_VIEW_BOUND = 200 * 100;

struct FCharacterManagerCharacterInstanceUpdate
{
	void operator()(const std::pair<uint32_t, CInstanceBase *> & cr_Pair) const { cr_Pair.second->Update(); }
};

void CPythonCharacterManager::AdjustCollisionWithOtherObjects(CActorInstance * pInst)
{
	if (!pInst->IsPC())
		return;

	CPythonCharacterManager & rkChrMgr = Instance();
	for (CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase * pkInstEach = *i;
		CActorInstance * rkActorEach = pkInstEach->GetGraphicThingInstancePtr();

		if (rkActorEach == pInst)
			continue;

		if (rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy()
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
			|| rkActorEach->IsShop()
#endif
#ifdef ENABLE_PET_MONSTER_TYPE
			|| rkActorEach->IsGrowthPet()
#endif
			)
			continue;

		if (pInst->TestPhysicsBlendingCollision(*rkActorEach))
		{
			// NOTE : 일단 기존위치로 원복
			// TODO : 향후 조금더 잘 처리한다면 physic movement거리를 steping해서 iteration처리해야 함.
			TPixelPosition curPos;
			pInst->GetPixelPosition(&curPos);
			pInst->SetBlendingPosition(curPos);
			//Tracef("!!!!!! Collision Adjusted\n");
			break;
		}
	}
}


void CPythonCharacterManager::EnableSortRendering(bool isEnable) const {}

void CPythonCharacterManager::InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	CInstanceBase::InsertPVPKey(dwVIDSrc, dwVIDDst);

	CInstanceBase * pkInstSrc = GetInstancePtr(dwVIDSrc);
	if (pkInstSrc)
		pkInstSrc->RefreshTextTail();

	CInstanceBase * pkInstDst = GetInstancePtr(dwVIDDst);
	if (pkInstDst)
		pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	CInstanceBase::RemovePVPKey(dwVIDSrc, dwVIDDst);

	CInstanceBase * pkInstSrc = GetInstancePtr(dwVIDSrc);
	if (pkInstSrc)
		pkInstSrc->RefreshTextTail();

	CInstanceBase * pkInstDst = GetInstancePtr(dwVIDDst);
	if (pkInstDst)
		pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::ChangeGVG(uint32_t dwSrcGuildID, uint32_t dwDstGuildID)
{
	for (auto & itor : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = itor.second;

		uint32_t dwInstanceGuildID = pInstance->GetGuildID();
		if (dwSrcGuildID == dwInstanceGuildID || dwDstGuildID == dwInstanceGuildID)
			pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::ClearMainInstance()
{
	m_pkInstMain = nullptr;
}

bool CPythonCharacterManager::SetMainInstance(uint32_t dwVID)
{
	m_pkInstMain = GetInstancePtr(dwVID);

	if (!m_pkInstMain)
		return false;

	return true;
}

CInstanceBase * CPythonCharacterManager::GetMainInstancePtr() const
{
	return m_pkInstMain;
}

void CPythonCharacterManager::GetInfo(std::string * pstInfo) const
{
	pstInfo->append("Actor: ");

	CInstanceBase::GetInfo(pstInfo);

	char szInfo[256];
	sprintf(szInfo, "Container - Live %u, Dead %u", m_kAliveInstMap.size(), m_kDeadInstList.size());
	pstInfo->append(szInfo);
}


bool CPythonCharacterManager::IsCacheMode() const
{
	static bool s_isOldCacheMode = false;

	bool isCacheMode = s_isOldCacheMode;
	if (s_isOldCacheMode)
	{
		if (m_kAliveInstMap.size() < 30)
			isCacheMode = false;
	}
	else
	{
		if (m_kAliveInstMap.size() > 40)
			isCacheMode = true;
	}
	s_isOldCacheMode = isCacheMode;

	return isCacheMode;
}

void CPythonCharacterManager::Update()
{
	CInstanceBase::ResetPerformanceCounter();

	CInstanceBase * pkInstMain = GetMainInstancePtr();
	uint32_t dwDeadInstCount = 0;
	uint32_t dwForceVisibleInstCount = 0;

	auto i = m_kAliveInstMap.begin();
	while (m_kAliveInstMap.end() != i)
	{
		auto c = i++;

		CInstanceBase * pkInstEach = c->second;
		pkInstEach->Update();

		if (pkInstMain)
		{
			//@fixme417
			/*if (pkInstEach->IsForceVisible())
			{
				dwForceVisibleInstCount++;
				continue;
			}*/

			auto nDistance = int(pkInstEach->NEW_GetDistanceFromDestInstance(*pkInstMain));
			if (nDistance > CHAR_STAGE_VIEW_BOUND + 10)
			{
				__DeleteBlendOutInstance(pkInstEach);
				m_kAliveInstMap.erase(c);
				dwDeadInstCount++;
			}
		}
	}
	UpdateTransform();

	UpdateDeleting();

	__NEW_Pick();
}

void CPythonCharacterManager::ShowPointEffect(uint32_t ePoint, uint32_t dwVID)
{
	CInstanceBase * pkInstSel = (dwVID == 0xffffffff) ? GetMainInstancePtr() : GetInstancePtr(dwVID);

	if (!pkInstSel)
		return;

	switch (ePoint)
	{
		case POINT_LEVEL:
#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_CONQUEROR_LEVEL:
#endif
			pkInstSel->LevelUp();
			break;
		case POINT_LEVEL_STEP:
#ifdef ENABLE_YOHARA_SYSTEM
		case POINT_CONQUEROR_LEVEL_STEP:
#endif
			pkInstSel->SkillUp();
			break;
	}
}

bool CPythonCharacterManager::RegisterPointEffect(uint32_t ePoint, const char * c_szFileName)
{
	if (ePoint >= POINT_MAX_NUM)
		return false;

	CEffectManager & rkEftMgr = CEffectManager::Instance();
	rkEftMgr.RegisterEffect2(c_szFileName, &m_adwPointEffect[ePoint]);

	return true;
}

void CPythonCharacterManager::UpdateTransform()
{
	CInstanceBase * pMainInstance = GetMainInstancePtr();
	if (pMainInstance)
	{
		CPythonBackground & rkBG = CPythonBackground::Instance();
		for (auto & i : m_kAliveInstMap)
		{
			CInstanceBase * pSrcInstance = i.second;

			pSrcInstance->CheckAdvancing();

			// 2004.08.02.myevan.IsAttacked 일 경우 죽었을때도 체크하므로,
			// 실질적으로 거리가 변경되는 IsPushing일때만 체크하도록 한다
			if (pSrcInstance->IsPushing())
				rkBG.CheckAdvancing(pSrcInstance);
		}

#ifdef __MOVIE_MODE__
		if (!m_pkInstMain->IsMovieMode())
			rkBG.CheckAdvancing(m_pkInstMain);
#else
		rkBG.CheckAdvancing(m_pkInstMain);
#endif
	}

	{
		for (auto & itor : m_kAliveInstMap)
		{
			CInstanceBase * pInstance = itor.second;
			pInstance->Transform();
		}
	}
}
void CPythonCharacterManager::UpdateDeleting()
{
	auto itor = m_kDeadInstList.begin();
	for (; itor != m_kDeadInstList.end();)
	{
		CInstanceBase * pInstance = *itor;

#ifdef ENABLE_GRAPHIC_ON_OFF
		if (pInstance->IsShowActor() && pInstance->UpdateDeleting())
#else
		if (pInstance->UpdateDeleting())
#endif
			++itor;
		else
		{
			CInstanceBase::Delete(pInstance);
			itor = m_kDeadInstList.erase(itor);
		}
	}
}

struct FCharacterManagerCharacterInstanceDeform
{
	void operator()(const std::pair<uint32_t, CInstanceBase *> & cr_Pair) const
	{
		cr_Pair.second->Deform();
		//pInstance->Update();
	}
};
struct FCharacterManagerCharacterInstanceListDeform
{
	void operator()(CInstanceBase * pInstance) const { pInstance->Deform(); }
};

void CPythonCharacterManager::Deform()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceDeform());
	std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(), FCharacterManagerCharacterInstanceListDeform());
}


bool CPythonCharacterManager::OLD_GetPickedInstanceVID(uint32_t * pdwPickedActorID) const
{
	if (!m_pkInstPick)
		return false;

	*pdwPickedActorID = m_pkInstPick->GetVirtualID();
	return true;
}

CInstanceBase * CPythonCharacterManager::OLD_GetPickedInstancePtr() const
{
	return m_pkInstPick;
}

D3DXVECTOR2 & CPythonCharacterManager::OLD_GetPickedInstPosReference()
{
	return m_v2PickedInstProjPos;
}

bool CPythonCharacterManager::IsRegisteredVID(uint32_t dwVID)
{
	if (m_kAliveInstMap.end() == m_kAliveInstMap.find(dwVID))
		return false;

	return true;
}

bool CPythonCharacterManager::IsAliveVID(uint32_t dwVID)
{
	return m_kAliveInstMap.find(dwVID) != m_kAliveInstMap.end();
}

bool CPythonCharacterManager::IsDeadVID(uint32_t dwVID)
{
	for (auto & f : m_kDeadInstList)
	{
		if (f->GetVirtualID() == dwVID)
			return true;
	}

	return false;
}

struct LessCharacterInstancePtrRenderOrder
{
	bool operator()(CInstanceBase * pkLeft, CInstanceBase * pkRight) const { return pkLeft->LessRenderOrder(pkRight); }
};

struct FCharacterManagerCharacterInstanceRender
{
	void operator()(const std::pair<uint32_t, CInstanceBase *> & cr_Pair) const
	{
		cr_Pair.second->Render();
		cr_Pair.second->RenderTrace();
	}
};
struct FCharacterInstanceRender
{
	void operator()(CInstanceBase * pInstance) const { pInstance->Render(); }
};
struct FCharacterInstanceRenderTrace
{
	void operator()(CInstanceBase * pInstance) const { pInstance->RenderTrace(); }
};


void CPythonCharacterManager::__RenderSortedAliveActorList()
{
	static std::vector<CInstanceBase *> s_kVct_pkInstAliveSort;
	s_kVct_pkInstAliveSort.clear();

	TCharacterInstanceMap & rkMap_pkInstAlive = m_kAliveInstMap;
	for (auto & i : rkMap_pkInstAlive)
#ifdef ENABLE_GRAPHIC_ON_OFF
		if (i.second->IsShowActor())
			s_kVct_pkInstAliveSort.emplace_back(i.second);
#else
		s_kVct_pkInstAliveSort.emplace_back(i.second);
#endif

	std::sort(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), LessCharacterInstancePtrRenderOrder());
	std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), FCharacterInstanceRender());
	std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(), FCharacterInstanceRenderTrace());
}

void CPythonCharacterManager::__RenderSortedDeadActorList()
{
	static std::vector<CInstanceBase *> s_kVct_pkInstDeadSort;
	s_kVct_pkInstDeadSort.clear();

	TCharacterInstanceList & rkLst_pkInstDead = m_kDeadInstList;
	for (auto & i : rkLst_pkInstDead)
		s_kVct_pkInstDeadSort.emplace_back(i);

	std::sort(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), LessCharacterInstancePtrRenderOrder());
	std::for_each(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(), FCharacterInstanceRender());
}

void CPythonCharacterManager::Render()
{
	STATEMANAGER.SetTexture(0, nullptr);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetTexture(1, nullptr);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


	__RenderSortedAliveActorList();
	__RenderSortedDeadActorList();

	CInstanceBase * pkPickedInst = OLD_GetPickedInstancePtr();
	if (pkPickedInst)
	{
		const D3DXVECTOR3 & c_rv3Position = pkPickedInst->GetGraphicThingInstanceRef().GetPosition();
		CPythonGraphic::Instance().ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z, &m_v2PickedInstProjPos.x,
												   &m_v2PickedInstProjPos.y);
	}
}

void CPythonCharacterManager::RenderShadowMainInstance() const
{
	CInstanceBase * pkInstMain = GetMainInstancePtr();
	if (pkInstMain)
		pkInstMain->RenderToShadowMap();
}

struct FCharacterManagerCharacterInstanceRenderToShadowMap
{
	void operator()(const std::pair<uint32_t, CInstanceBase *> & cr_Pair) const
	{
#ifdef ENABLE_GRAPHIC_ON_OFF
		if (cr_Pair.second->IsShowActor())
			cr_Pair.second->RenderToShadowMap();
#else
		cr_Pair.second->RenderToShadowMap();
#endif
	}
};

void CPythonCharacterManager::RenderShadowAllInstances()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderToShadowMap());
}

struct FCharacterManagerCharacterInstanceRenderCollision
{
	void operator()(const std::pair<uint32_t, CInstanceBase *> & cr_Pair) const
	{
#ifdef ENABLE_GRAPHIC_ON_OFF
		if (cr_Pair.second->IsShowActor())
			cr_Pair.second->RenderCollision();
#else
		cr_Pair.second->RenderCollision();
#endif
	}
};

void CPythonCharacterManager::RenderCollision()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderCollision());
}

struct FCharacterManagerCharacterInstanceRenderCollisionNew
{
	void operator()(const std::pair<uint32_t, CInstanceBase*>& cr_Pair) const
	{
		cr_Pair.second->RenderCollisionNew();
	}
};

void CPythonCharacterManager::RenderCollisionNew()
{
	std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), FCharacterManagerCharacterInstanceRenderCollisionNew());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Managing Process

CInstanceBase * CPythonCharacterManager::CreateInstance(const CInstanceBase::SCreateData & c_rkCreateData)
{
	CInstanceBase * pCharacterInstance = RegisterInstance(c_rkCreateData.m_dwVID);
	if (!pCharacterInstance)
	{
		TraceError("CPythonCharacterManager::CreateInstance: VID[%d] - ALREADY EXIST\n", c_rkCreateData.m_dwVID); // @fixme010
		return nullptr;
	}

	if (!pCharacterInstance->Create(c_rkCreateData))
	{
		TraceError("CPythonCharacterManager::CreateInstance VID[%d] Race[%d]", c_rkCreateData.m_dwVID, c_rkCreateData.m_dwRace);
		DeleteInstance(c_rkCreateData.m_dwVID);
		return nullptr;
	}

	if (c_rkCreateData.m_isMain)
		SelectInstance(c_rkCreateData.m_dwVID);

	return (pCharacterInstance);
}

CInstanceBase * CPythonCharacterManager::RegisterInstance(uint32_t VirtualID)
{
	auto itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() != itor)
		return nullptr;

	CInstanceBase * pCharacterInstance = CInstanceBase::New();
	m_kAliveInstMap.emplace(VirtualID, pCharacterInstance);

	return (pCharacterInstance);
}

void CPythonCharacterManager::DeleteInstance(uint32_t dwDelVID)
{
	auto itor = m_kAliveInstMap.find(dwDelVID);

	if (m_kAliveInstMap.end() == itor)
	{
		Tracef("DeleteCharacterInstance: no vid by %d\n", dwDelVID);
		return;
	}

	CInstanceBase * pkInstDel = itor->second;

	if (pkInstDel == m_pkInstBind)
		m_pkInstBind = nullptr;

	if (pkInstDel == m_pkInstMain)
		m_pkInstMain = nullptr;

	if (pkInstDel == m_pkInstPick)
		m_pkInstPick = nullptr;

	CInstanceBase::Delete(pkInstDel);

	m_kAliveInstMap.erase(itor);
}

void CPythonCharacterManager::__DeleteBlendOutInstance(CInstanceBase * pkInstDel)
{
	pkInstDel->DeleteBlendOut();
	m_kDeadInstList.emplace_back(pkInstDel);

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyCharacterDead(pkInstDel->GetVirtualID());
}

void CPythonCharacterManager::DeleteInstanceByFade(uint32_t dwVID)
{
	auto f = m_kAliveInstMap.find(dwVID);
	if (m_kAliveInstMap.end() == f)
		return;
	__DeleteBlendOutInstance(f->second);
	m_kAliveInstMap.erase(f);
}

void CPythonCharacterManager::SelectInstance(uint32_t VirtualID)
{
	auto itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() == itor)
	{
		Tracef("SelectCharacterInstance: no vid by %d\n", VirtualID);
		return;
	}

	m_pkInstBind = itor->second;
}

#ifdef ENABLE_TAB_TARGETING
CInstanceBase * CPythonCharacterManager::GetTabNextTargetPointer(CInstanceBase * pkInstMain)
{
	if (!pkInstMain)
	{
		ResetTabNextTargetVectorIndex();
		return nullptr;
	}

	struct FCharacterManagerInstanceTarget
	{
		CInstanceBase * pkInstMain;
		FCharacterManagerInstanceTarget(CInstanceBase * pInstance) : pkInstMain(pInstance) {}

		inline void operator () (const std::pair<uint32_t, CInstanceBase *>& itor)
		{
			const auto pkInstTarget = itor.second;
			if (!pkInstTarget || pkInstTarget == pkInstMain || !pkInstTarget->IsEnemy())
				return;

			const auto fRadiusDistance = pkInstMain->GetDistance(pkInstTarget);
			if (fRadiusDistance < 1500.0f)
				m_vecTargetInstance.emplace_back(pkInstTarget);
		}

		std::vector<CInstanceBase *> m_vecTargetInstance;
	};

	FCharacterManagerInstanceTarget f(pkInstMain);
	f = std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(), f);

	const auto kTargetCount = f.m_vecTargetInstance.size();
	if (kTargetCount == 0)
	{
		ResetTabNextTargetVectorIndex();
		return nullptr;
	}

	if (GetTabNextTargetVectorIndex() >= kTargetCount - 1)
		ResetTabNextTargetVectorIndex();

	return f.m_vecTargetInstance.at(++m_adwVectorIndexTabNextTarget);
}
#endif

CInstanceBase * CPythonCharacterManager::GetInstancePtr(uint32_t VirtualID)
{
	auto itor = m_kAliveInstMap.find(VirtualID);

	if (m_kAliveInstMap.end() == itor)
		return nullptr;

	return itor->second;
}

CInstanceBase * CPythonCharacterManager::GetInstancePtrByName(const char * name)
{
	for (auto & itor : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = itor.second;

		if (!strcmp(pInstance->GetNameString(), name))
			return pInstance;
	}

	return nullptr;
}

CInstanceBase * CPythonCharacterManager::GetSelectedInstancePtr() const
{
	return m_pkInstBind;
}

CInstanceBase * CPythonCharacterManager::FindClickableInstancePtr() const
{
	return nullptr;
}

void CPythonCharacterManager::__UpdateSortPickedActorList()
{
	__UpdatePickedActorList();
	__SortPickedActorList();
}

void CPythonCharacterManager::__UpdatePickedActorList()
{
	m_kVct_pkInstPicked.clear();

	for (auto & i : m_kAliveInstMap)
	{
		CInstanceBase * pkInstEach = i.second;
		// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
		if (pkInstEach->CanPickInstance())
		{
			if (pkInstEach->IsDead())
			{
				if (pkInstEach->IntersectBoundingBox())
					m_kVct_pkInstPicked.emplace_back(pkInstEach);
			}
			else
			{
				if (pkInstEach->IntersectDefendingSphere())
					m_kVct_pkInstPicked.emplace_back(pkInstEach);
			}
		}
	}
}

struct CInstanceBase_SLessCameraDistance
{
	TPixelPosition m_kPPosEye;

	bool operator()(CInstanceBase * pkInstLeft, CInstanceBase * pkInstRight) const
	{
		int nLeftDeadPoint = pkInstLeft->IsDead();
		int nRightDeadPoint = pkInstRight->IsDead();

		if (nLeftDeadPoint < nRightDeadPoint)
			return true;

		if (pkInstLeft->CalculateDistanceSq3d(m_kPPosEye) < pkInstRight->CalculateDistanceSq3d(m_kPPosEye))
			return true;

		return false;
	}
};

void CPythonCharacterManager::__SortPickedActorList()
{
	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	const D3DXVECTOR3 & c_rv3EyePos = pCamera->GetEye();

	CInstanceBase_SLessCameraDistance kLess;
	kLess.m_kPPosEye = TPixelPosition(+c_rv3EyePos.x, -c_rv3EyePos.y, +c_rv3EyePos.z);

	std::sort(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), kLess);
}

void CPythonCharacterManager::__NEW_Pick()
{
	__UpdateSortPickedActorList();

	CInstanceBase * pkInstMain = GetMainInstancePtr();

#ifdef __MOVIE_MODE
	if (pkInstMain)
		if (pkInstMain->IsMovieMode())
		{
			if (m_pkInstPick)
				m_pkInstPick->OnUnselected();
			return;
		}
#endif

	// 정밀한 체크
	{
		for (auto pkInstEach : m_kVct_pkInstPicked)
		{
			if (pkInstEach != pkInstMain && pkInstEach->IntersectBoundingBox())
			{
				if (m_pkInstPick)
				{
					if (m_pkInstPick != pkInstEach)
						m_pkInstPick->OnUnselected();
				}

				if (pkInstEach->CanPickInstance())
				{
					m_pkInstPick = pkInstEach;
					m_pkInstPick->OnSelected();
					return;
				}
			}
		}
	}

	// 못찾겠으면 걍 순서대로
	{
		for (auto pkInstEach : m_kVct_pkInstPicked)
		{
			if (pkInstEach != pkInstMain)
			{
				if (m_pkInstPick)
				{
					if (m_pkInstPick != pkInstEach)
						m_pkInstPick->OnUnselected();
				}

				if (pkInstEach->CanPickInstance())
				{
					m_pkInstPick = pkInstEach;
					m_pkInstPick->OnSelected();
					return;
				}
			}
		}
	}

	if (pkInstMain)
	{
		if (pkInstMain->CanPickInstance())
		{
			if (m_kVct_pkInstPicked.end() != std::find(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), pkInstMain))
			{
				if (m_pkInstPick)
					if (m_pkInstPick != pkInstMain)
						m_pkInstPick->OnUnselected();

				m_pkInstPick = pkInstMain;
				m_pkInstPick->OnSelected();
				return;
			}
		}
	}

	if (m_pkInstPick)
	{
		m_pkInstPick->OnUnselected();
		m_pkInstPick = nullptr;
	}
}

void CPythonCharacterManager::__OLD_Pick()
{
	for (auto & itor : m_kAliveInstMap)
	{
		CInstanceBase * pkInstEach = itor.second;

		if (pkInstEach == m_pkInstMain)
			continue;

		if (pkInstEach->IntersectDefendingSphere())
		{
			if (m_pkInstPick)
			{
				if (m_pkInstPick != pkInstEach)
					m_pkInstPick->OnUnselected();
			}

			m_pkInstPick = pkInstEach;
			m_pkInstPick->OnSelected();

			return;
		}
	}

	if (m_pkInstPick)
	{
		m_pkInstPick->OnUnselected();
		m_pkInstPick = nullptr;
	}
}

int CPythonCharacterManager::PickAll()
{
	for (auto & itor : m_kAliveInstMap)
	{
		CInstanceBase * pInstance = itor.second;

		if (pInstance->IntersectDefendingSphere())
			return pInstance->GetVirtualID();
	}

	return -1;
}

CInstanceBase * CPythonCharacterManager::GetCloseInstance(CInstanceBase * pInstance)
{
	float fMinDistance = 10000.0f;
	CInstanceBase * pCloseInstance = nullptr;

	auto itor = m_kAliveInstMap.begin();
	for (; itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase * pTargetInstance = itor->second;

		if (pTargetInstance == pInstance)
			continue;

		uint32_t dwVirtualNumber = pTargetInstance->GetVirtualNumber();
		if (CPythonNonPlayer::ON_CLICK_EVENT_BATTLE != CPythonNonPlayer::Instance().GetEventType(dwVirtualNumber))
			continue;

		float fDistance = pInstance->GetDistance(pTargetInstance);
		if (fDistance < fMinDistance)
		{
			fMinDistance = fDistance;
			pCloseInstance = pTargetInstance;
		}
	}

	return pCloseInstance;
}

void CPythonCharacterManager::RefreshAllPCTextTail()
{
	CharacterIterator itor = CharacterInstanceBegin();
	CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;
		if (!pInstance->IsPC())
			continue;

		pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::RefreshAllGuildMark()
{
	CharacterIterator itor = CharacterInstanceBegin();
	CharacterIterator itorEnd = CharacterInstanceEnd();
	for (; itor != itorEnd; ++itor)
	{
		CInstanceBase * pInstance = *itor;
		if (!pInstance->IsPC())
			continue;

#ifdef ENABLE_SHOW_GUILD_LEADER
		pInstance->ChangeGuild(pInstance->GetGuildID(), pInstance->GetNewIsGuildName());
#else
		pInstance->ChangeGuild(pInstance->GetGuildID());
#endif
		pInstance->RefreshTextTail();
	}
}

void CPythonCharacterManager::DeleteAllInstances()
{
	DestroyAliveInstanceMap();
	DestroyDeadInstanceList();
}


void CPythonCharacterManager::DestroyAliveInstanceMap()
{
	for (auto & i : m_kAliveInstMap)
		CInstanceBase::Delete(i.second);

	m_kAliveInstMap.clear();
}

void CPythonCharacterManager::DestroyDeadInstanceList()
{
	std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(), CInstanceBase::Delete);
	m_kDeadInstList.clear();
}

void CPythonCharacterManager::Destroy()
{
#ifdef ENABLE_MINI_GAME_YUTNORI
	CPythonYutnoriManager::Instance().ClearInstances();
#endif
	DeleteAllInstances();

	CInstanceBase::DestroySystem();

	__Initialize();
}

void CPythonCharacterManager::__Initialize()
{
	msl::refill(m_adwPointEffect);
	m_pkInstMain = nullptr;
	m_pkInstBind = nullptr;
	m_pkInstPick = nullptr;
	m_v2PickedInstProjPos = D3DXVECTOR2(0.0f, 0.0f);
#ifdef ENABLE_TAB_TARGETING
	ResetTabNextTargetVectorIndex();
#endif
}

CPythonCharacterManager::CPythonCharacterManager()
{
	__Initialize();
}

CPythonCharacterManager::~CPythonCharacterManager()
{
	Destroy();
}

#ifdef ENABLE_AUTO_SYSTEM
CInstanceBase* CPythonCharacterManager::AutoHuntingGetMob(CInstanceBase* pkInstMain, uint32_t dwVID, bool lock)
{
	if (!pkInstMain)
		return nullptr;

	float passDistance = 0.0f;

	CInstanceBase* minInstance = NULL;

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();

	if (!lock && dwVID != 0)
	{
		CInstanceBase* pInstanceTarget = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
		if (pInstanceTarget)
			passDistance = pkInstMain->GetDistance(pInstanceTarget) + 200.0f;
	}

	for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); ++itor)
	{
		CInstanceBase* iTarget = itor->second;
		if (!iTarget || iTarget == pkInstMain /*|| (!iTarget->IsEnemy() && !iTarget->IsStone())*/ || iTarget->IsDead() || iTarget->GetVirtualID() == dwVID)
			continue;

		const float fDistance0 = iTarget->NEW_GetDistanceFromDestPixelPosition(rkPlayer.AutoHuntGetStartLocation());
		if (rkPlayer.GetAutoRangeOnOff() && abs(fDistance0) >= AUTO_MAX_FOCUS_DISTANCE)
			continue;

		/**********Is there a place to walk between me and the target?**********/

		bool obstacleVarDur = false;
		TPixelPosition targetPos; iTarget->NEW_GetPixelPosition(&targetPos);
		const D3DXVECTOR3& c_rv3Src = pkInstMain->GetGraphicThingInstanceRef().GetPosition();
		const D3DXVECTOR3 c_v3Dst = D3DXVECTOR3(targetPos.x, -targetPos.y, c_rv3Src.z);
		const D3DXVECTOR3 c_v3Delta = c_v3Dst - c_rv3Src;
		const int cycleNumber = 100;
		const D3DXVECTOR3 inc = c_v3Delta / cycleNumber;
		D3DXVECTOR3 v3Movement(0.0f, 0.0f, 0.0f);
		IPhysicsWorld* pWorld = IPhysicsWorld::GetPhysicsWorld();

		if (!pWorld)
			obstacleVarDur = true;

		for (int i = 0; i < cycleNumber; ++i)
		{
			if (pWorld->isPhysicalCollision(c_rv3Src + v3Movement))
				obstacleVarDur = true;

			v3Movement += inc;
		}

		if (obstacleVarDur)
			continue;

		/*if (pkInstMain->CheckAdvancing())
			CPythonChat::Instance().AppendChat(1, "CheckAdvancing");*/

		/**********Is there a place to walk between me and the target?**********/

		const float fDistance = pkInstMain->GetDistance(iTarget);

		if (passDistance != 0.0f && fDistance < passDistance)
			continue;

		if (lock && dwVID != 0)
		{
			CInstanceBase* pInstanceTarget = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
			if (pInstanceTarget && dwVID == iTarget->GetVirtualID())
				continue;
		}

		else if (minInstance != NULL)
		{
			if (rkPlayer.GetAutoRangeOnOff())
			{
				const float fDistance2 = pkInstMain->GetDistance(minInstance);
				if (fDistance < fDistance2 && fDistance < 250.0f)
					minInstance = iTarget;
				else if (abs(fDistance0) < AUTO_MAX_FOCUS_DISTANCE && abs(fDistance0) < abs(minInstance->NEW_GetDistanceFromDestPixelPosition(rkPlayer.AutoHuntGetStartLocation())))
					minInstance = iTarget;
			}
			else
			{
				const float fDistance2 = pkInstMain->GetDistance(minInstance);
				if (fDistance < fDistance2)
					minInstance = iTarget;
			}
		}
		else if (minInstance == NULL)
			minInstance = iTarget;
	}

	return minInstance ? minInstance : nullptr;
}
#endif
