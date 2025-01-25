#pragma once

#include "AbstractCharacterManager.h"
#include "InstanceBase.h"
#include "../GameLib/PhysicsObject.h"

class CPythonCharacterManager : public CSingleton<CPythonCharacterManager>, public IAbstractCharacterManager, public IObjectManager
{
public:
	// Character List
	typedef std::list<CInstanceBase *> TCharacterInstanceList;
	typedef std::map<uint32_t, CInstanceBase *> TCharacterInstanceMap;

	class CharacterIterator;

public:
	CPythonCharacterManager();
	~CPythonCharacterManager();
	CLASS_DELETE_COPYMOVE(CPythonCharacterManager);

	void AdjustCollisionWithOtherObjects(CActorInstance * pInst) override;

	void EnableSortRendering(bool isEnable) const;

	bool IsRegisteredVID(uint32_t dwVID);
	bool IsAliveVID(uint32_t dwVID);
	bool IsDeadVID(uint32_t dwVID);
	bool IsCacheMode() const;

	bool OLD_GetPickedInstanceVID(uint32_t * pdwPickedActorID) const;
	CInstanceBase * OLD_GetPickedInstancePtr() const;
	D3DXVECTOR2 & OLD_GetPickedInstPosReference();

	CInstanceBase * FindClickableInstancePtr() const;

	void InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	void RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
	void ChangeGVG(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);

	void GetInfo(std::string * pstInfo) const;

	void ClearMainInstance();
	bool SetMainInstance(uint32_t dwVID);
	CInstanceBase * GetMainInstancePtr() const;

	void SCRIPT_SetAffect(uint32_t dwVID, uint32_t eState, BOOL isVisible);
	void SetEmoticon(uint32_t dwVID, uint32_t eState);
	bool IsPossibleEmoticon(uint32_t dwVID);
	void ShowPointEffect(uint32_t ePoint, uint32_t dwVID);
	bool RegisterPointEffect(uint32_t ePoint, const char * c_szFileName);

	// System
	void Destroy() override;

	void DeleteAllInstances();

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	void Update();
	void Deform();
	void Render();
	void RenderShadowMainInstance() const;
	void RenderShadowAllInstances();
	void RenderCollision();
	void RenderCollisionNew();

	// Create/Delete Instance
	CInstanceBase * CreateInstance(const CInstanceBase::SCreateData & c_rkCreateData);
	CInstanceBase * RegisterInstance(uint32_t VirtualID);

	void DeleteInstance(uint32_t dwDelVID);
	void DeleteInstanceByFade(uint32_t dwVID);
	void DeleteVehicleInstance(uint32_t VirtualID);

	void DestroyAliveInstanceMap();
	void DestroyDeadInstanceList();

	CharacterIterator CharacterInstanceBegin() { return CharacterIterator(m_kAliveInstMap.begin()); }

	CharacterIterator CharacterInstanceEnd() { return CharacterIterator(m_kAliveInstMap.end()); }

	// Access Instance
	void SelectInstance(uint32_t VirtualID);
	CInstanceBase * GetSelectedInstancePtr() const;

	CInstanceBase * GetInstancePtr(uint32_t VirtualID) override;
	CInstanceBase * GetInstancePtrByName(const char * name);

	// Pick
	int PickAll();
	CInstanceBase * GetCloseInstance(CInstanceBase * pInstance);

	// Refresh TextTail
	void RefreshAllPCTextTail();
	void RefreshAllGuildMark();

#ifdef ENABLE_PARTY_MATCH
	void SetPartyMatchOff(bool activeState) { m_PartyMatchActiveState = activeState; }
	bool GetPartyMatchOff() { return m_PartyMatchActiveState; }
#endif

protected:
	void UpdateTransform();
	void UpdateDeleting();

#ifdef ENABLE_TAB_TARGETING
public:
	CInstanceBase *						GetTabNextTargetPointer(CInstanceBase * pkInstMain);
	const uint32_t							GetTabNextTargetVectorIndex() { return m_adwVectorIndexTabNextTarget; }
	void								ResetTabNextTargetVectorIndex() { m_adwVectorIndexTabNextTarget = -1; }

protected:
	uint32_t								m_adwVectorIndexTabNextTarget;
#endif

#ifdef ENABLE_PARTY_MATCH
	bool m_PartyMatchActiveState;
#endif

#ifdef ENABLE_AUTO_SYSTEM
public:
	void SetAutoOnOff(bool activeState) { m_AutoActiveState = activeState; }
	bool GetAutoOnOff() { return m_AutoActiveState; }

protected:
	bool m_AutoActiveState;
#endif

protected:
	void __Initialize();

	void __DeleteBlendOutInstance(CInstanceBase * pkInstDel);

	void __OLD_Pick();
	void __NEW_Pick();

	void __UpdateSortPickedActorList();
	void __UpdatePickedActorList();
	void __SortPickedActorList();

	void __RenderSortedAliveActorList();
	void __RenderSortedDeadActorList();

protected:
	CInstanceBase * m_pkInstMain;
	CInstanceBase * m_pkInstPick;
	CInstanceBase * m_pkInstBind;
	D3DXVECTOR2 m_v2PickedInstProjPos;

	TCharacterInstanceMap m_kAliveInstMap;
	TCharacterInstanceList m_kDeadInstList;

	std::vector<CInstanceBase *> m_kVct_pkInstPicked;

	uint32_t m_adwPointEffect[POINT_MAX_NUM];

public:
	class CharacterIterator
	{
	public:
		CharacterIterator() = default;
		CharacterIterator(const TCharacterInstanceMap::iterator & it) : m_it(it) {}

		CInstanceBase * operator*() const { return m_it->second; }

		CharacterIterator & operator++()
		{
			++m_it;
			return *this;
		}

		CharacterIterator operator++(int)
		{
			CharacterIterator new_it = *this;
			++(*this);
			return new_it;
		}

		CharacterIterator & operator=(const CharacterIterator & rhs)
		{
			m_it = rhs.m_it;
			return (*this);
		}

		bool operator==(const CharacterIterator & rhs) const { return m_it == rhs.m_it; }

		bool operator!=(const CharacterIterator & rhs) const { return m_it != rhs.m_it; }

	private:
		TCharacterInstanceMap::iterator m_it;
	};
#ifdef ENABLE_AUTO_SYSTEM
	public:
		//CInstanceBase* OtomatikHedefBul(CInstanceBase* pkInstMain);
		CInstanceBase* AutoHuntingGetMob(CInstanceBase* pkInstMain, uint32_t dwVID, bool lock = false);
#endif
};
