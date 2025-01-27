#include "StdAfx.h"

#include "FlyingData.h"
#include "FlyingInstance.h"
#include "FlyingObjectManager.h"

#include "ActorInstance.h"

CFlyingManager::CFlyingManager()
{
	m_pMapManager = nullptr;
}

CFlyingManager::~CFlyingManager()
{
	Destroy();
}

void CFlyingManager::__DestroyFlyingInstanceList()
{
	TFlyingInstanceList::iterator i;
	for (i = m_kLst_pkFlyInst.begin(); i != m_kLst_pkFlyInst.end(); ++i)
	{
		CFlyingInstance * pkFlyInst = *i;
		CFlyingInstance::Delete(pkFlyInst);
	}
	m_kLst_pkFlyInst.clear();
}

void CFlyingManager::__DestroyFlyingDataMap()
{
	TFlyingDataMap::iterator i;
	for (i = m_kMap_pkFlyData.begin(); i != m_kMap_pkFlyData.end(); ++i)
	{
		CFlyingData * pkFlyData = i->second;
		CFlyingData::Delete(pkFlyData);
	}
	m_kMap_pkFlyData.clear();
}


void CFlyingManager::DeleteAllInstances()
{
	__DestroyFlyingInstanceList();
}

void CFlyingManager::Destroy()
{
	__DestroyFlyingInstanceList();
	__DestroyFlyingDataMap();

	m_pMapManager = nullptr;
}

bool CFlyingManager::RegisterFlyingData(const char * c_szFilename)
{
	std::string s;
	StringPath(c_szFilename, s);
	uint32_t dwRetCRC = GetCaseCRC32(s.c_str(), s.size());

	if (m_kMap_pkFlyData.find(dwRetCRC) != m_kMap_pkFlyData.end())
		return false;

	CFlyingData * pFlyingData = CFlyingData::New();
	if (!pFlyingData->LoadScriptFile(c_szFilename))
	{
		Tracenf("CEffectManager::RegisterFlyingData %s - Failed to load flying data file", c_szFilename);
		CFlyingData::Delete(pFlyingData);
		return false;
	}

	m_kMap_pkFlyData.emplace(dwRetCRC, pFlyingData);
	return true;
}

bool CFlyingManager::RegisterFlyingData(const char * c_szFilename, uint32_t & r_dwRetCRC)
{
	std::string s;
	StringPath(c_szFilename, s);
	r_dwRetCRC = GetCaseCRC32(s.c_str(), s.size());

	if (m_kMap_pkFlyData.find(r_dwRetCRC) != m_kMap_pkFlyData.end())
	{
		TraceError("CFlyingManager::RegisterFlyingData - Already exists flying data named [%s]", c_szFilename);
		return false;
	}

	CFlyingData * pFlyingData = CFlyingData::New();
	if (!pFlyingData->LoadScriptFile(c_szFilename))
	{
		TraceError("CEffectManager::RegisterFlyingData %s - Failed to load flying data file", c_szFilename);
		CFlyingData::Delete(pFlyingData);
		return false;
	}

	m_kMap_pkFlyData.emplace(r_dwRetCRC, pFlyingData);
	return true;
}

#ifdef ENABLE_SKILL_COLOR_SYSTEM
CFlyingInstance* CFlyingManager::CreateFlyingInstanceFlyTarget(const uint32_t dwID,
	const D3DXVECTOR3& v3StartPosition,
	const CFlyTarget& cr_FlyTarget,
	bool canAttack,
	uint32_t* dwSkillColor)
#else
CFlyingInstance * CFlyingManager::CreateFlyingInstanceFlyTarget(const uint32_t dwID, const D3DXVECTOR3 & v3StartPosition,
																const CFlyTarget & cr_FlyTarget, bool canAttack)
#endif
{
	if (m_kMap_pkFlyData.find(dwID) == m_kMap_pkFlyData.end())
	{
		//TraceError("CFlyingManager::CreateFlyingInstanceFlyTarget - No data with CRC [%d]", dwID);
		return nullptr;
	}

	CFlyingInstance * pFlyingInstance = CFlyingInstance::New();
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	if (dwSkillColor != nullptr)
		pFlyingInstance->SetSkillColor(dwSkillColor);
#endif
	pFlyingInstance->Create(m_kMap_pkFlyData[dwID], v3StartPosition, cr_FlyTarget, canAttack);

	m_kLst_pkFlyInst.emplace_back(pFlyingInstance);

	pFlyingInstance->ID = m_IDCounter++;
	return pFlyingInstance;
}

#ifdef ENABLE_QUIVER_SYSTEM
CFlyingInstance * CFlyingManager::CreateIndexedFlyingInstanceFlyTarget(const uint32_t dwIndex,
	const D3DXVECTOR3 & v3StartPosition,
	const CFlyTarget & cr_FlyTarget)
{
	if (m_kMap_dwIndexFlyData.end() == m_kMap_dwIndexFlyData.find(dwIndex))
	{
		TraceError("CFlyingManager::CreateIndexedFlyingInstanceFlyTarget(dwIndex=%d) - Not registered index", dwIndex);
		return nullptr;
	}

	TIndexFlyData& rIndexFlyData = m_kMap_dwIndexFlyData[dwIndex];
	return CreateFlyingInstanceFlyTarget(rIndexFlyData.dwCRC,
		D3DXVECTOR3(v3StartPosition.x, v3StartPosition.y, v3StartPosition.z),
		cr_FlyTarget,
		true);
}
#endif

void CFlyingManager::Update()
{
	auto i = m_kLst_pkFlyInst.begin();

	while (i != m_kLst_pkFlyInst.end())
	{
		CFlyingInstance * pkFlyInst = *i;
		if (!pkFlyInst->Update())
		{
			CFlyingInstance::Delete(pkFlyInst);
			i = m_kLst_pkFlyInst.erase(i);
		}
		else
			++i;
	}
}

void CFlyingManager::Render()
{
	std::for_each(m_kLst_pkFlyInst.begin(), m_kLst_pkFlyInst.end(), std::mem_fn(&CFlyingInstance::Render));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

bool CFlyingManager::RegisterIndexedFlyData(uint32_t dwIndex, uint8_t byType, const char * c_szFileName)
{
	uint32_t dwCRC;
	if (!RegisterFlyingData(c_szFileName, dwCRC))
	{
		TraceError("CFlyingManager::RegisterIndexFlyData(dwIndex=%d, c_szFileName=%s) - Failed to load flying data file", dwIndex,
				   c_szFileName);
		return false;
	}

	TIndexFlyData IndexFlyData;
	IndexFlyData.byType = byType;
	IndexFlyData.dwCRC = dwCRC;
	m_kMap_dwIndexFlyData.emplace(dwIndex, IndexFlyData);

	return true;
}

void CFlyingManager::CreateIndexedFly(uint32_t dwIndex, CActorInstance * pStartActor, CActorInstance * pEndActor)
{
	if (m_kMap_dwIndexFlyData.end() == m_kMap_dwIndexFlyData.find(dwIndex))
	{
		TraceError("CFlyingManager::CreateIndexedFly(dwIndex=%d) - Not registered index", dwIndex);
		return;
	}

	TPixelPosition posStart;
	pStartActor->GetPixelPosition(&posStart);

	TIndexFlyData & rIndexFlyData = m_kMap_dwIndexFlyData[dwIndex];
	switch (rIndexFlyData.byType)
	{
		case INDEX_FLY_TYPE_NORMAL:
		{
#ifdef ENABLE_GRAPHIC_ON_OFF
			if (CPythonGraphicOnOff::Instance().GetEffectOnOffLevel() >= CPythonGraphicOnOff::EFFECT_ONOFF_LEVEL_NONE)
				return;
#endif

#ifdef ENABLE_SKILL_COLOR_SYSTEM
			CreateFlyingInstanceFlyTarget(rIndexFlyData.dwCRC,
				D3DXVECTOR3(posStart.x, posStart.y, posStart.z),
				pEndActor,
				false,
				pStartActor->GetSkillColorByEffectID(dwIndex));
#else
			CreateFlyingInstanceFlyTarget(rIndexFlyData.dwCRC, D3DXVECTOR3(posStart.x, posStart.y, posStart.z), pEndActor, false);
#endif
			break;
		}
		case INDEX_FLY_TYPE_FIRE_CRACKER:
		{
			float fRot = fmod(pStartActor->GetRotation() - 90.0f + 360.0f, 360.0f) + frandom(-30.0f, 30.0f);

			float fDistance = frandom(2000.0f, 5000.0f);
			float fxRand = fDistance * cosf(D3DXToRadian(fRot));
			float fyRand = fDistance * sinf(D3DXToRadian(fRot));
			float fzRand = frandom(1000.0f, 2500.0f);

			CreateFlyingInstanceFlyTarget(rIndexFlyData.dwCRC, D3DXVECTOR3(posStart.x, posStart.y, posStart.z + 200),
										  D3DXVECTOR3(posStart.x + fxRand, posStart.y + fyRand, posStart.z + fzRand), false);
			break;
		}
		case INDEX_FLY_TYPE_AUTO_FIRE:
		{
#ifdef ENABLE_SKILL_COLOR_SYSTEM
			CreateFlyingInstanceFlyTarget(rIndexFlyData.dwCRC,
				D3DXVECTOR3(posStart.x, posStart.y, posStart.z + 100.0f),
				pEndActor,
				false,
				pStartActor->GetSkillColorByEffectID(dwIndex));
#else
			CreateFlyingInstanceFlyTarget(rIndexFlyData.dwCRC, D3DXVECTOR3(posStart.x, posStart.y, posStart.z + 100.0f), pEndActor, false);
#endif
			break;
		}
	}
}
