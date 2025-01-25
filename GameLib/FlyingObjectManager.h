#pragma once

#include "FlyTarget.h"

#include <set>

class CFlyingInstance;
class CFlyingData;
class CMapManager;
class CActorInstance;

class CFlyingManager : public CSingleton<CFlyingManager>
{
public:
	enum EIndexFlyType
	{
		INDEX_FLY_TYPE_NORMAL,
		INDEX_FLY_TYPE_FIRE_CRACKER,
		INDEX_FLY_TYPE_AUTO_FIRE
	};

public:
	CFlyingManager();
	virtual ~CFlyingManager();

	void Destroy();

	void DeleteAllInstances();

	bool RegisterFlyingData(const char * c_szFilename);
	bool RegisterFlyingData(const char * c_szFilename, uint32_t & r_dwRetCRC);

#ifdef ENABLE_SKILL_COLOR_SYSTEM
	CFlyingInstance* CreateFlyingInstanceFlyTarget(const uint32_t dwID, const D3DXVECTOR3& v3StartPosition, const CFlyTarget& cr_FlyTarget, bool canAttack, uint32_t* dwSkillColor = nullptr);
#else
	CFlyingInstance * CreateFlyingInstanceFlyTarget(const uint32_t dwID, const D3DXVECTOR3 & v3StartPosition,
													const CFlyTarget & cr_FlyTarget, bool canAttack);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	CFlyingInstance * CreateIndexedFlyingInstanceFlyTarget(const uint32_t dwIndex, const D3DXVECTOR3& v3StartPosition, const CFlyTarget& cr_FlyTarget);
#endif

	void Update();
	void Render();

	void SetMapManagerPtr(CMapManager * pMapManager) { m_pMapManager = pMapManager; }
	CMapManager * GetMapManagerPtr() { return m_pMapManager; }

public: // Controlled by Server
	bool RegisterIndexedFlyData(uint32_t dwIndex, uint8_t byType, const char * c_szFileName);
	void CreateIndexedFly(uint32_t dwIndex, CActorInstance * pStartActor, CActorInstance * pEndActor);

private:
	void __DestroyFlyingInstanceList();
	void __DestroyFlyingDataMap();

	typedef std::map<uint32_t, CFlyingData *> TFlyingDataMap;
	typedef std::list<CFlyingInstance *> TFlyingInstanceList;

	typedef struct SIndexFlyData
	{
		uint8_t byType;
		uint32_t dwCRC;
	} TIndexFlyData;
	typedef std::map<uint32_t, TIndexFlyData> TIndexFlyDataMap;

	TFlyingDataMap m_kMap_pkFlyData;
	TFlyingInstanceList m_kLst_pkFlyInst;
	TIndexFlyDataMap m_kMap_dwIndexFlyData;

	CMapManager * m_pMapManager;

	uint32_t m_IDCounter;
};
