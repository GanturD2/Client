#include "StdAfx.h"
#include "../EterBase/Random.h"
#include "../EterLib/StateManager.h"
#include "EffectManager.h"
#ifdef ENABLE_GRAPHIC_ON_OFF
#	include "../EterPythonLib/PythonGraphicOnOff.h"
#endif


void CEffectManager::GetInfo(std::string * pstInfo) const
{
	char szInfo[256];

	sprintf(szInfo, "Effect: Inst - ED %u, EI %u Pool - PSI %u, MI %u, LI %u, PI %u, EI %u, ED %u, PSD %u, EM %u, LD %u",
			m_kEftDataMap.size(), m_kEftInstMap.size(), CParticleSystemInstance::ms_kPool.GetCapacity(),
			CEffectMeshInstance::ms_kPool.GetCapacity(), CLightInstance::ms_kPool.GetCapacity(), CParticleInstance::ms_kPool.GetCapacity(),
			CEffectInstance::ms_kPool.GetCapacity(), CEffectData::ms_kPool.GetCapacity(), CParticleSystemData::ms_kPool.GetCapacity(),
			CEffectMeshScript::ms_kPool.GetCapacity(), CLightData::ms_kPool.GetCapacity());
	pstInfo->append(szInfo);
}

void CEffectManager::UpdateSound()
{
	for (auto & itor : m_kEftInstMap)
	{
		CEffectInstance * pEffectInstance = itor.second;

		pEffectInstance->UpdateSound();
	}
}

bool CEffectManager::IsAliveEffect(uint32_t dwInstanceIndex)
{
	auto f = m_kEftInstMap.find(dwInstanceIndex);
	if (m_kEftInstMap.end() == f)
		return false;

	return f->second->isAlive() != 0;
}

void CEffectManager::Update()
{
	// 2004. 3. 1. myevan. 이펙트 모니터링 하는 코드
	/*
	if (GetAsyncKeyState(VK_F9))
	{
		Tracenf("CEffectManager::m_EffectInstancePool %d", m_EffectInstancePool.GetCapacity());
		Tracenf("CEffectManager::m_EffectDataPool %d", m_EffectDataPool.GetCapacity());
		Tracenf("CEffectInstance::ms_LightInstancePool %d", CEffectInstance::ms_LightInstancePool.GetCapacity());
		Tracenf("CEffectInstance::ms_MeshInstancePool %d", CEffectInstance::ms_MeshInstancePool.GetCapacity());
		Tracenf("CEffectInstance::ms_ParticleSystemInstancePool %d", CEffectInstance::ms_ParticleSystemInstancePool.GetCapacity());
		Tracenf("CParticleInstance::ms_ParticleInstancePool %d", CParticleInstance::ms_kPool.GetCapacity());
		Tracenf("CRayParticleInstance::ms_RayParticleInstancePool %d", CRayParticleInstance::ms_kPool.GetCapacity());
		Tracen("---------------------------------------------");
	}
	*/

	for (auto itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end();)
	{
		CEffectInstance * pEffectInstance = itor->second;

#ifdef ENABLE_GRAPHIC_ON_OFF
		if (!CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT)
			&& pEffectInstance->GetEffectID() == m_dwDropItemEffectID
			|| (pEffectInstance->Update(), pEffectInstance->isAlive()))
		{
			++itor;
		}
		else
		{
			itor = m_kEftInstMap.erase(itor);
			CEffectInstance::Delete(pEffectInstance);
		}
#else
		pEffectInstance->Update(/*fElapsedTime*/);

		if (!pEffectInstance->isAlive())
		{
			itor = m_kEftInstMap.erase(itor);

			CEffectInstance::Delete(pEffectInstance);
		}
		else
			++itor;
#endif
	}
}


struct CEffectManager_LessEffectInstancePtrRenderOrder
{
	bool operator()(CEffectInstance * pkLeft, CEffectInstance * pkRight) const { return pkLeft->LessRenderOrder(pkRight); }
};

struct CEffectManager_FEffectInstanceRender
{
	void operator()(CEffectInstance * pkEftInst) const
	{
#ifdef ENABLE_RENDER_TARGET_EFFECT
		if (pkEftInst->IsSpecialRender())
			return;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
		if (pkEftInst->IsShow())
			pkEftInst->Render();
#else
		pkEftInst->Render();
#endif
	}
};

void CEffectManager::Render()
{
	STATEMANAGER.SetTexture(0, nullptr);
	STATEMANAGER.SetTexture(1, nullptr);

	if (m_isDisableSortRendering)
	{
		for (auto itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end();)
		{
			CEffectInstance * pEffectInstance = itor->second;
#ifdef ENABLE_RENDER_TARGET_EFFECT
			if (pEffectInstance->IsSpecialRender()) 
			{
				++itor; 
				continue; 
			}
#endif	
#ifdef ENABLE_GRAPHIC_ON_OFF
			if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT)
				|| pEffectInstance->GetEffectID() != m_dwDropItemEffectID)
				pEffectInstance->Render();
#else
			pEffectInstance->Render();
#endif
			++itor;
		}
	}
	else
	{
		static std::vector<CEffectInstance *> s_kVct_pkEftInstSort;
		s_kVct_pkEftInstSort.clear();

		TEffectInstanceMap & rkMap_pkEftInstSrc = m_kEftInstMap;
		for (auto & i : rkMap_pkEftInstSrc)
#ifdef ENABLE_GRAPHIC_ON_OFF
		{
			CEffectInstance* pEffectInstance = i.second;
			if (CPythonGraphicOnOff::Instance().IsItemDropOnOffLevel(CPythonGraphicOnOff::ITEM_DROP_ONOFF_LEVEL_HIDE_EFFECT)
				|| pEffectInstance->GetEffectID() != m_dwDropItemEffectID)
				s_kVct_pkEftInstSort.emplace_back(i.second);
		}
#else
			s_kVct_pkEftInstSort.emplace_back(i.second);
#endif

		std::sort(s_kVct_pkEftInstSort.begin(), s_kVct_pkEftInstSort.end(), CEffectManager_LessEffectInstancePtrRenderOrder());
		std::for_each(s_kVct_pkEftInstSort.begin(), s_kVct_pkEftInstSort.end(), CEffectManager_FEffectInstanceRender());
	}
}

#ifdef ENABLE_INGAME_WIKI
void CEffectManager::WikiModuleRenderOneEffect(uint32_t id)
{
	STATEMANAGER.SetTexture(0, nullptr);
	STATEMANAGER.SetTexture(1, nullptr);

	const auto& pEffectInstance = m_kEftInstMap.find(id);

	if (pEffectInstance != m_kEftInstMap.end())
	{
		pEffectInstance->second->SetWikiIgnoreFrustum(true);
		pEffectInstance->second->Show();
		pEffectInstance->second->Render();
	}
	else
		TraceError("!RenderOne, not found");
}
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
BOOL CEffectManager::RegisterEffect(const char* c_szFileName, bool isExistDelete, bool isNeedCache, const char* name)
#else
BOOL CEffectManager::RegisterEffect(const char * c_szFileName,bool isExistDelete,bool isNeedCache)
#endif
{
	std::string strFileName;
	StringPath(c_szFileName, strFileName);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t dwCRC = GetCaseCRC32(strFileName.c_str(), strFileName.length(), name);
#else
	uint32_t dwCRC = GetCaseCRC32(strFileName.c_str(), strFileName.length());
#endif

	auto itor = m_kEftDataMap.find(dwCRC);
	if (m_kEftDataMap.end() != itor)
	{
		if (isExistDelete)
		{
			CEffectData * pkEftData = itor->second;
			CEffectData::Delete(pkEftData);
			m_kEftDataMap.erase(itor);
		}
		else
		{
			//TraceError("CEffectManager::RegisterEffect - m_kEftDataMap.find [%s] Already Exist", c_szFileName);
			return TRUE;
		}
	}

	CEffectData * pkEftData = CEffectData::New();

	if (!pkEftData->LoadScript(c_szFileName))
	{
		TraceError("CEffectManager::RegisterEffect - LoadScript(%s) Error", c_szFileName);
		CEffectData::Delete(pkEftData);
		return FALSE;
	}

	m_kEftDataMap.emplace(dwCRC, pkEftData);

	if (isNeedCache)
	{
		if (m_kEftCacheMap.find(dwCRC) == m_kEftCacheMap.end())
		{
			CEffectInstance * pkNewEftInst = CEffectInstance::New();
			pkNewEftInst->SetEffectDataPointer(pkEftData);
			m_kEftCacheMap.emplace(dwCRC, pkNewEftInst);
		}
	}

	return TRUE;
}

// CEffectData 를 포인터형으로 리턴하게 하고..
// CEffectData에서 CRC를 얻을수 있게 한다
#ifdef ENABLE_SKILL_COLOR_SYSTEM
BOOL CEffectManager::RegisterEffect2(const char* c_szFileName, uint32_t* pdwRetCRC, bool isNeedCache, const char* name)
#else
BOOL CEffectManager::RegisterEffect2(const char * c_szFileName, uint32_t* pdwRetCRC, bool isNeedCache)
#endif
{
	std::string strFileName;
	StringPath(c_szFileName, strFileName);
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	uint32_t dwCRC = GetCaseCRC32(strFileName.c_str(), strFileName.length(), name);
#else
	uint32_t dwCRC = GetCaseCRC32(strFileName.c_str(), strFileName.length());
#endif
	*pdwRetCRC = dwCRC;

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	return RegisterEffect(c_szFileName, false, isNeedCache, name);
#else
	return RegisterEffect(c_szFileName,false,isNeedCache);
#endif
}

#ifdef ENABLE_SCALE_SYSTEM
int CEffectManager::CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation, float fParticleScale)
#else
int CEffectManager::CreateEffect(const char* c_szFileName, const D3DXVECTOR3& c_rv3Position, const D3DXVECTOR3& c_rv3Rotation)
#endif
{
	uint32_t dwID = GetCaseCRC32(c_szFileName, strlen(c_szFileName));
#ifdef ENABLE_SCALE_SYSTEM
	return CreateEffect(dwID, c_rv3Position, c_rv3Rotation, fParticleScale);
#else
	return CreateEffect(dwID, c_rv3Position, c_rv3Rotation);
#endif
}

int CEffectManager::CreateEffect(uint32_t dwID, const D3DXVECTOR3 & c_rv3Position, const D3DXVECTOR3 & c_rv3Rotation
#ifdef ENABLE_SCALE_SYSTEM
	, float fParticleScale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	, uint32_t* dwSkillColor
#endif
)
{
	int iInstanceIndex = GetEmptyIndex();

	CreateEffectInstance(iInstanceIndex, dwID
#ifdef ENABLE_SCALE_SYSTEM
		, fParticleScale, 0
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
		, dwSkillColor
#endif
	);

	SelectEffectInstance(iInstanceIndex);
	D3DXMATRIX mat;
	D3DXMatrixRotationYawPitchRoll(&mat, D3DXToRadian(c_rv3Rotation.x), D3DXToRadian(c_rv3Rotation.y), D3DXToRadian(c_rv3Rotation.z));
	mat._41 = c_rv3Position.x;
	mat._42 = c_rv3Position.y;
	mat._43 = c_rv3Position.z;
	SetEffectInstanceGlobalMatrix(mat);

	return iInstanceIndex;
}

void CEffectManager::CreateEffectInstance(uint32_t dwInstanceIndex, uint32_t dwID
#ifdef ENABLE_SCALE_SYSTEM
	, float fParticleScale, const D3DXVECTOR3* c_pv3MeshScale
#endif
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	, uint32_t* dwSkillColor
#endif
)
{
	if (!dwID)
		return;

	CEffectData * pEffect;
	if (!GetEffectData(dwID, &pEffect))
	{
		Tracef("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwID);
		return;
	}

	CEffectInstance * pEffectInstance = CEffectInstance::New();
#ifdef ENABLE_SCALE_SYSTEM
	pEffectInstance->SetParticleScale(fParticleScale);
	if (c_pv3MeshScale)
		pEffectInstance->SetMeshScale(*c_pv3MeshScale);
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	pEffectInstance->SetEffectDataPointer(pEffect, dwSkillColor, dwID);
#else
	pEffectInstance->SetEffectDataPointer(pEffect);
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
	pEffectInstance->SetEffectID(dwID);
#endif

	m_kEftInstMap.emplace(dwInstanceIndex, pEffectInstance);
}

bool CEffectManager::DestroyEffectInstance(uint32_t dwInstanceIndex)
{
	auto itor = m_kEftInstMap.find(dwInstanceIndex);

	if (itor == m_kEftInstMap.end())
		return false;

	CEffectInstance * pEffectInstance = itor->second;

	m_kEftInstMap.erase(itor);

	CEffectInstance::Delete(pEffectInstance);

	return true;
}

void CEffectManager::DeactiveEffectInstance(uint32_t dwInstanceIndex)
{
	auto itor = m_kEftInstMap.find(dwInstanceIndex);

	if (itor == m_kEftInstMap.end())
		return;

	CEffectInstance * pEffectInstance = itor->second;
	pEffectInstance->SetDeactive();
}

#ifdef ENABLE_RENDER_TARGET_EFFECT
void CEffectManager::RenderSelectedEffect()
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->Render();
}

void CEffectManager::SetSpecialRenderEffect(uint32_t dwID)
{
	auto itor = m_kEftInstMap.find(dwID);

	if (m_kEftInstMap.end() == itor)
		return;

	if (!itor->second)
		return;

	itor->second->SetSpecialRender();
}
#endif

void CEffectManager::CreateUnsafeEffectInstance(uint32_t dwEffectDataID, CEffectInstance ** ppEffectInstance)
{
	CEffectData * pEffect;
	if (!GetEffectData(dwEffectDataID, &pEffect))
	{
		Tracef("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwEffectDataID);
		return;
	}

	CEffectInstance * pkEftInstNew = CEffectInstance::New();
	pkEftInstNew->SetEffectDataPointer(pEffect);

	*ppEffectInstance = pkEftInstNew;
}

bool CEffectManager::DestroyUnsafeEffectInstance(CEffectInstance * pEffectInstance)
{
	if (!pEffectInstance)
		return false;

	CEffectInstance::Delete(pEffectInstance);

	return true;
}

BOOL CEffectManager::SelectEffectInstance(uint32_t dwInstanceIndex)
{
	auto itor = m_kEftInstMap.find(dwInstanceIndex);

	m_pSelectedEffectInstance = nullptr;

	if (m_kEftInstMap.end() == itor)
		return FALSE;

	m_pSelectedEffectInstance = itor->second;

	return TRUE;
}

void CEffectManager::SetEffectTextures(uint32_t dwID, std::vector<std::string> textures)
{
	CEffectData * pEffectData;
	if (!GetEffectData(dwID, &pEffectData))
	{
		Tracef("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwID);
		return;
	}

	for (uint32_t i = 0; i < textures.size(); i++)
	{
		CParticleSystemData * pParticle = pEffectData->GetParticlePointer(i);
		pParticle->ChangeTexture(textures.at(i).c_str());
	}
}

void CEffectManager::SetEffectInstancePosition(const D3DXVECTOR3 & c_rv3Position) const
{
	if (!m_pSelectedEffectInstance)
	{
		//		assert(!"Instance to use is not yet set!");
		return;
	}

	m_pSelectedEffectInstance->SetPosition(c_rv3Position);
}

void CEffectManager::SetEffectInstanceRotation(const D3DXVECTOR3 & c_rv3Rotation) const
{
	if (!m_pSelectedEffectInstance)
	{
		//		assert(!"Instance to use is not yet set!");
		return;
	}

	m_pSelectedEffectInstance->SetRotation(c_rv3Rotation.x, c_rv3Rotation.y, c_rv3Rotation.z);
}

void CEffectManager::SetEffectInstanceGlobalMatrix(const D3DXMATRIX & c_rmatGlobal) const
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->SetGlobalMatrix(c_rmatGlobal);
}

void CEffectManager::ShowEffect() const
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->Show();
}

void CEffectManager::HideEffect() const
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->Hide();
}

bool CEffectManager::GetEffectData(uint32_t dwID, CEffectData ** ppEffect)
{
	auto itor = m_kEftDataMap.find(dwID);

	if (itor == m_kEftDataMap.end())
		return false;

	*ppEffect = itor->second;

	return true;
}

bool CEffectManager::GetEffectData(uint32_t dwID, const CEffectData ** c_ppEffect)
{
	auto itor = m_kEftDataMap.find(dwID);

	if (itor == m_kEftDataMap.end())
		return false;

	*c_ppEffect = itor->second;

	return true;
}

uint32_t CEffectManager::GetRandomEffect()
{
	const int iIndex = random() % m_kEftDataMap.size();

	auto itor = m_kEftDataMap.begin();
	for (int i = 0; i < iIndex; ++i, ++itor)
		continue;

	return itor->first;
}

int CEffectManager::GetEmptyIndex()
{
	static int iMaxIndex = 1;

	if (iMaxIndex > 2100000000)
		iMaxIndex = 1;

	int iNextIndex = iMaxIndex++;
	while (m_kEftInstMap.find(iNextIndex) != m_kEftInstMap.end())
		iNextIndex++;

	return iNextIndex;
}

void CEffectManager::DeleteAllInstances()
{
	__DestroyEffectInstanceMap();
}

void CEffectManager::__DestroyEffectInstanceMap()
{
	for (auto & i : m_kEftInstMap)
	{
		CEffectInstance * pkEftInst = i.second;
		CEffectInstance::Delete(pkEftInst);
	}

	m_kEftInstMap.clear();
}

void CEffectManager::__DestroyEffectCacheMap()
{
	for (auto & i : m_kEftCacheMap)
	{
		CEffectInstance * pkEftInst = i.second;
		CEffectInstance::Delete(pkEftInst);
	}

	m_kEftCacheMap.clear();
}

void CEffectManager::__DestroyEffectDataMap()
{
	for (auto & i : m_kEftDataMap)
	{
		CEffectData * pData = i.second;
		CEffectData::Delete(pData);
	}

	m_kEftDataMap.clear();
}

void CEffectManager::Destroy()
{
	__DestroyEffectInstanceMap();
	__DestroyEffectCacheMap();
	__DestroyEffectDataMap();

	__Initialize();
}

void CEffectManager::__Initialize()
{
	m_pSelectedEffectInstance = nullptr;
	m_isDisableSortRendering = false;
#ifdef ENABLE_GRAPHIC_ON_OFF
	m_dwDropItemEffectID = 0;
#endif
}

CEffectManager::CEffectManager()
{
	__Initialize();
}

CEffectManager::~CEffectManager()
{
	Destroy();
}

// just for map effect
