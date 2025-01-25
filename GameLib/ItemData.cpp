#include "StdAfx.h"
#include "../EterLib/ResourceManager.h"

#include "ItemData.h"
#ifdef ENABLE_AURA_SYSTEM
#	include "../EffectLib/EffectManager.h"
#endif

CDynamicPool<CItemData> CItemData::ms_kPool;


CItemData * CItemData::New()
{
	return ms_kPool.Alloc();
}

void CItemData::Delete(CItemData * pkItemData)
{
	pkItemData->Clear();
	ms_kPool.Free(pkItemData);
}

void CItemData::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicThing * CItemData::GetModelThing()
{
	return m_pModelThing;
}

CGraphicThing * CItemData::GetSubModelThing()
{
	if (m_pSubModelThing)
		return m_pSubModelThing;
	else
		return m_pModelThing;
}

CGraphicThing * CItemData::GetDropModelThing()
{
	return m_pDropModelThing;
}

CGraphicSubImage * CItemData::GetIconImage()
{
	if (m_pIconImage == nullptr && m_strIconFileName.empty() == false)
		__SetIconImage(m_strIconFileName.c_str());
	return m_pIconImage;
}

uint32_t CItemData::GetLODModelThingCount()
{
	return m_pLODModelThingVector.size();
}

BOOL CItemData::GetLODModelThingPointer(uint32_t dwIndex, CGraphicThing ** ppModelThing)
{
	if (dwIndex >= m_pLODModelThingVector.size())
		return FALSE;

	*ppModelThing = m_pLODModelThingVector[dwIndex];

	return TRUE;
}

uint32_t CItemData::GetAttachingDataCount()
{
	return m_AttachingDataVector.size();
}

BOOL CItemData::GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA != m_AttachingDataVector[dwIndex].dwType)
		return FALSE;

	*c_ppAttachingData = &m_AttachingDataVector[dwIndex];
	return TRUE;
}

BOOL CItemData::GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData)
{
	if (dwIndex >= GetAttachingDataCount())
		return FALSE;

	*c_ppAttachingData = &m_AttachingDataVector[dwIndex];
	return TRUE;
}

void CItemData::SetSummary(const std::string & c_rstSumm)
{
	m_strSummary = c_rstSumm;
}

void CItemData::SetDescription(const std::string & c_rstDesc)
{
	m_strDescription = c_rstDesc;
}

void CItemData::SetDefaultItemData(const char * c_szIconFileName, const char * c_szModelFileName)
{
	if (c_szModelFileName)
	{
		m_strModelFileName = c_szModelFileName;
		m_strDropModelFileName = c_szModelFileName;
	}
	else
	{
		m_strModelFileName = "";
		m_strDropModelFileName = "d:/ymir work/item/etc/item_bag.gr2";
	}
	m_strIconFileName = c_szIconFileName;

	m_strSubModelFileName = "";
	m_strDescription = "";
	m_strSummary = "";
	msl::refill(m_ItemTable.alSockets);

	__LoadFiles();
}

void CItemData::__LoadFiles()
{
	// Model File Name
	if (!m_strModelFileName.empty())
		m_pModelThing = msl::inherit_cast<CGraphicThing *>(CResourceManager::Instance().GetResourcePointer(m_strModelFileName.c_str()));

	if (!m_strSubModelFileName.empty())
		m_pSubModelThing =
			msl::inherit_cast<CGraphicThing *>(CResourceManager::Instance().GetResourcePointer(m_strSubModelFileName.c_str()));

	if (!m_strDropModelFileName.empty())
		m_pDropModelThing =
			msl::inherit_cast<CGraphicThing *>(CResourceManager::Instance().GetResourcePointer(m_strDropModelFileName.c_str()));


	if (!m_strLODModelFileNameVector.empty())
	{
		m_pLODModelThingVector.clear();
		m_pLODModelThingVector.resize(m_strLODModelFileNameVector.size());

		for (uint32_t i = 0; i < m_strLODModelFileNameVector.size(); ++i)
		{
			const std::string & c_rstrLODModelFileName = m_strLODModelFileNameVector[i];
			m_pLODModelThingVector[i] =
				msl::inherit_cast<CGraphicThing *>(CResourceManager::Instance().GetResourcePointer(c_rstrLODModelFileName.c_str()));
		}
	}
}

#define ENABLE_LOAD_ALTER_ITEMICON
void CItemData::__SetIconImage(const char * c_szFileName)
{
	if (!CResourceManager::Instance().IsFileExist(c_szFileName))
	{
		TraceError("%s not found. CItemData::__SetIconImage", c_szFileName);
		m_pIconImage = nullptr;
#ifdef ENABLE_LOAD_ALTER_ITEMICON
		static const char * c_szAlterIconImage = "icon/item/27995.tga";
		if (CResourceManager::Instance().IsFileExist(c_szAlterIconImage))
			m_pIconImage = msl::inherit_cast<CGraphicSubImage *>(CResourceManager::Instance().GetResourcePointer(c_szAlterIconImage));
#endif
	}
	else if (m_pIconImage == nullptr)
		m_pIconImage = msl::inherit_cast<CGraphicSubImage *>(CResourceManager::Instance().GetResourcePointer(c_szFileName));
}

void CItemData::SetItemTableData(TItemTable * pItemTable)
{
	memcpy(&m_ItemTable, pItemTable, sizeof(TItemTable));
}

const CItemData::TItemTable * CItemData::GetTable() const
{
	return &m_ItemTable;
}

uint32_t CItemData::GetIndex() const
{
	return m_ItemTable.dwVnum;
}

uint32_t CItemData::GetRefinedVnum() const
{
	return m_ItemTable.dwRefinedVnum;
}

const char * CItemData::GetName() const
{
	return m_ItemTable.szLocaleName;
}

const char * CItemData::GetDescription() const
{
	return m_strDescription.c_str();
}

const char * CItemData::GetSummary() const
{
	return m_strSummary.c_str();
}


uint8_t CItemData::GetType() const
{
	return m_ItemTable.bType;
}

uint8_t CItemData::GetSubType() const
{
	return m_ItemTable.bSubType;
}

#ifdef ENABLE_MOVE_COSTUME_ATTR
uint8_t CItemData::GetMagicPct() const
{
	return m_ItemTable.bAlterToMagicItemPct;
}
#endif

#ifdef ENABLE_PROTO_RENEWAL
uint8_t CItemData::GetMaskType() const
{
	return m_ItemTable.bMaskType;
}

uint8_t CItemData::GetMaskSubType() const
{
	return m_ItemTable.bMaskSubType;
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
uint32_t CItemData::GetAntiFlags() const
{
	return m_ItemTable.dwAntiFlags;
}
#endif

#define DEF_STR(x) #x

const char * CItemData::GetUseTypeString() const
{
	if (GetType() != CItemData::ITEM_TYPE_USE)
		return "NOT_USE_TYPE";

	switch (GetSubType())
	{
		case USE_TUNING:									// 2
			return DEF_STR(USE_TUNING);
		case USE_DETACHMENT:								// 14
			return DEF_STR(USE_DETACHMENT);
		case USE_CLEAN_SOCKET:								// 17
			return DEF_STR(USE_CLEAN_SOCKET);
		case USE_CHANGE_ATTRIBUTE:							// 18
			return DEF_STR(USE_CHANGE_ATTRIBUTE);
#ifdef ENABLE_ATTR_6TH_7TH
		case USE_CHANGE_ATTRIBUTE2:
			return DEF_STR(USE_CHANGE_ATTRIBUTE2);
#endif
		case USE_ADD_ATTRIBUTE:								// 19
			return DEF_STR(USE_ADD_ATTRIBUTE);
		case USE_ADD_ACCESSORY_SOCKET:						// 20
			return DEF_STR(USE_ADD_ACCESSORY_SOCKET);
		case USE_PUT_INTO_ACCESSORY_SOCKET:					// 21
			return DEF_STR(USE_PUT_INTO_ACCESSORY_SOCKET);
		case USE_ADD_ATTRIBUTE2:							// 22
			return DEF_STR(USE_ADD_ATTRIBUTE2);
		case USE_BIND:										// 25
			return DEF_STR(USE_BIND);
		case USE_UNBIND:									// 26
			return DEF_STR(USE_UNBIND);
		case USE_PUT_INTO_BELT_SOCKET:						// 29
			return DEF_STR(USE_PUT_INTO_BELT_SOCKET);
		case USE_PUT_INTO_RING_SOCKET:						// 30
			return DEF_STR(USE_PUT_INTO_RING_SOCKET);
#ifdef ENABLE_USE_COSTUME_ATTR
		case USE_CHANGE_COSTUME_ATTR:						// 31
			return DEF_STR(USE_CHANGE_COSTUME_ATTR);
		case USE_RESET_COSTUME_ATTR:						// 32
			return DEF_STR(USE_RESET_COSTUME_ATTR);
#endif
#ifdef ENABLE_CHANGED_ATTR
		case USE_SELECT_ATTRIBUTE:							// 34
			return DEF_STR(USE_SELECT_ATTRIBUTE);
#endif
#ifdef ENABLE_FLOWER_EVENT
		case USE_FLOWER:									// 35
			return DEF_STR(USE_FLOWER);
#endif
#ifdef ENABLE_EXPRESSING_EMOTION
		case USE_EMOTION_PACK:								// 36
			return DEF_STR(USE_EMOTION_PACK);
#endif
#ifdef ENABLE_REFINE_ELEMENT
		case USE_ELEMENT_UPGRADE:							// 37
			return DEF_STR(USE_ELEMENT_UPGRADE);
		case USE_ELEMENT_DOWNGRADE:							// 38
			return DEF_STR(USE_ELEMENT_DOWNGRADE);
		case USE_ELEMENT_CHANGE:							// 39
			return DEF_STR(USE_ELEMENT_CHANGE);
#endif
#ifdef ENABLE_PROTO_RENEWAL
		case USE_CALL:										// 40
			return DEF_STR(USE_CALL);
		case USE_POTION_TOWER:								// 41
			return DEF_STR(USE_POTION_TOWER);
		case USE_POTION_NODELAY_TOWER:						// 42
			return DEF_STR(USE_POTION_NODELAY_TOWER);
		case USE_REMOVE_AFFECT:								// 43
			return DEF_STR(USE_REMOVE_AFFECT);
		case USE_EMOTION_TOWER:								// 44
			return DEF_STR(USE_EMOTION_TOWER);
		case USE_SECRET_DUNGEON_SCROLL:						// 45
			return DEF_STR(USE_SECRET_DUNGEON_SCROLL);
#endif

		// Custom
		case USE_AGGREGATE_MONSTER:
			return DEF_STR(USE_AGGREGATE_MONSTER);
#if defined(ENABLE_AURA_SYSTEM) && defined(ENABLE_AURA_BOOST)
		case USE_PUT_INTO_AURA_SOCKET:
			return DEF_STR(USE_PUT_INTO_AURA_SOCKET);
#endif
	}
	return "USE_UNKNOWN_TYPE";
}


uint32_t CItemData::GetWeaponType() const
{
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	if (GetType() == CItemData::ITEM_TYPE_COSTUME && GetSubType() == CItemData::COSTUME_WEAPON)
		return GetValue(3);
#endif
	return m_ItemTable.bSubType;
}

uint8_t CItemData::GetSize() const
{
	return m_ItemTable.bSize;
}

BOOL CItemData::IsAntiFlag(uint32_t dwFlag) const
{
	return (dwFlag & m_ItemTable.dwAntiFlags) != 0;
}

BOOL CItemData::IsFlag(uint32_t dwFlag) const
{
	return (dwFlag & m_ItemTable.dwFlags) != 0;
}

BOOL CItemData::IsWearableFlag(uint32_t dwFlag) const
{
	return (dwFlag & m_ItemTable.dwWearFlags) != 0;
}

BOOL CItemData::HasNextGrade() const
{
	return 0 != m_ItemTable.dwRefinedVnum;
}

uint32_t CItemData::GetWearFlags() const
{
	return m_ItemTable.dwWearFlags;
}

uint32_t CItemData::GetIBuyItemPrice() const
{
	return m_ItemTable.dwIBuyItemPrice;
}

uint32_t CItemData::GetISellItemPrice() const
{
	return m_ItemTable.dwISellItemPrice;
}


BOOL CItemData::GetLimit(uint8_t byIndex, TItemLimit * pItemLimit) const
{
	if (byIndex >= ITEM_LIMIT_MAX_NUM)
	{
		assert(byIndex < ITEM_LIMIT_MAX_NUM);
		return FALSE;
	}

	*pItemLimit = m_ItemTable.aLimits[byIndex];

	return TRUE;
}

BOOL CItemData::GetApply(uint8_t byIndex, TItemApply * pItemApply) const
{
	if (byIndex >= ITEM_APPLY_MAX_NUM)
	{
		assert(byIndex < ITEM_APPLY_MAX_NUM);
		return FALSE;
	}

	*pItemApply = m_ItemTable.aApplies[byIndex];
	return TRUE;
}

long CItemData::GetValue(uint8_t byIndex) const
{
	if (byIndex >= ITEM_VALUES_MAX_NUM)
	{
		assert(byIndex < ITEM_VALUES_MAX_NUM);
		return 0;
	}

	return m_ItemTable.alValues[byIndex];
}

long CItemData::SetSocket(uint8_t byIndex, uint32_t value)
{
	if (byIndex >= ITEM_SOCKET_MAX_NUM)
	{
		assert(byIndex < ITEM_SOCKET_MAX_NUM);
		return -1;
	}

	return m_ItemTable.alSockets[byIndex] = value;
}

long CItemData::GetSocket(uint8_t byIndex) const
{
	if (byIndex >= ITEM_SOCKET_MAX_NUM)
	{
		assert(byIndex < ITEM_SOCKET_MAX_NUM);
		return -1;
	}

	return m_ItemTable.alSockets[byIndex];
}

//서버와 동일 서버 함수 변경시 같이 변경!!(이후에 합친다)
//SocketCount = 1 이면 초급무기
//SocketCount = 2 이면 중급무기
//SocketCount = 3 이면 고급무기
int CItemData::GetSocketCount() const
{
	return m_ItemTable.bGainSocketPct;
}

uint32_t CItemData::GetIconNumber() const
{
	return m_ItemTable.dwVnum;
}

uint32_t CItemData::GetSpecularPoweru() const
{
	return m_ItemTable.bSpecular;
}

float CItemData::GetSpecularPowerf() const
{
	uint32_t uSpecularPower = GetSpecularPoweru();

	return float(uSpecularPower) / 100.0f;
}

//refine 값은 아이템번호 끝자리와 일치한다-_-(테이블이용으로 바꿀 예정)
uint32_t CItemData::GetRefine() const
{
	return GetIndex() % 10;
}

BOOL CItemData::IsEquipment() const
{
	switch (GetType())
	{
		case ITEM_TYPE_WEAPON:
		case ITEM_TYPE_ARMOR:
		case ITEM_TYPE_BELT:
		case ITEM_TYPE_RING:
		case ITEM_TYPE_COSTUME:
			return TRUE;
			break;
	}

	return FALSE;
}

#ifdef ENABLE_SOULBIND_SYSTEM
BOOL CItemData::CanSealItem() const
{
	switch (GetType())
	{
		case ITEM_TYPE_WEAPON:
			if (GetSubType() == WEAPON_ARROW/* || GetSubType() >= WEAPON_NUM_TYPES*/)
				return FALSE;
			else
				return TRUE;
			break;

		case ITEM_TYPE_ARMOR:
		case ITEM_TYPE_RING:
		case ITEM_TYPE_BELT:
			return TRUE;
			break;

		case ITEM_TYPE_COSTUME:
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			return (GetSubType() != COSTUME_MOUNT);
#else
			return TRUE;/*(GetSubType() == COSTUME_BODY || GetSubType() == COSTUME_HAIR)*/
#endif
			break;
	}

	return FALSE;
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH
BOOL CItemData::IsAttr67MaterialVnum() const
{
	if (GetIndex() >= 39070 && GetIndex() <= 39108)
		return TRUE;

	return FALSE;
}
#endif

void CItemData::Clear()
{
	m_strSummary = "";
	m_strModelFileName = "";
	m_strSubModelFileName = "";
	m_strDropModelFileName = "";
	m_strIconFileName = "";
	m_strLODModelFileNameVector.clear();

	m_pModelThing = nullptr;
	m_pSubModelThing = nullptr;
	m_pDropModelThing = nullptr;
	m_pIconImage = nullptr;
	m_pLODModelThingVector.clear();
#ifdef ENABLE_INGAME_WIKI
	m_isValidImage = true;
	m_wikiInfo.isSet = false;
	m_wikiInfo.hasData = false;
	m_isBlacklisted = false;
#endif

	m_ItemTable = {};
#ifdef ENABLE_AURA_SYSTEM
	m_dwAuraEffectID = 0;
#endif
#ifdef ENABLE_SCALE_SYSTEM
	memset(&m_ItemScaleTable, 0, sizeof(m_ItemScaleTable));
#endif
#ifdef ENABLE_SHINING_SYSTEM
	memset(&m_ItemShiningTable, 0, sizeof(m_ItemShiningTable));
#endif
}

CItemData::CItemData()
{
	Clear();
}

CItemData::~CItemData() = default;

#ifdef ENABLE_SOULBIND_SYSTEM
bool CItemData::IsSealAbleItem() const
{
	if (GetType() == ITEM_TYPE_WEAPON)
		return GetSubType() != WEAPON_ARROW;

	if (GetType() == ITEM_TYPE_RING)
		return true;

	if (GetType() == ITEM_TYPE_DS)
		return true;

	if (GetType() == ITEM_TYPE_COSTUME)
		return true;

	if (GetType() == ITEM_TYPE_QUEST)
		return true;

	return GetType() == ITEM_TYPE_ARMOR || GetType() == ITEM_TYPE_BELT;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
void CItemData::SetAuraEffectID(const char* szAuraEffectPath)
{
	if (szAuraEffectPath)
		CEffectManager::Instance().RegisterEffect2(szAuraEffectPath, &m_dwAuraEffectID);
}
#endif

#ifdef ENABLE_SCALE_SYSTEM
float CItemData::GetItemParticleScale(uint8_t byJob, uint8_t bySex)
{
	return m_ItemScaleTable.fParticleScale[bySex][byJob];
}

void CItemData::SetItemTableScaleData(uint8_t byJob, uint8_t bySex, float fMeshScaleX, float fMeshScaleY, float fMeshScaleZ, float fParticleScale)
{
	m_ItemScaleTable.v3MeshScale[bySex][byJob].x = fMeshScaleX;
	m_ItemScaleTable.v3MeshScale[bySex][byJob].y = fMeshScaleY;
	m_ItemScaleTable.v3MeshScale[bySex][byJob].z = fMeshScaleZ;
	m_ItemScaleTable.fParticleScale[bySex][byJob] = fParticleScale;
}

D3DXVECTOR3& CItemData::GetItemScaleVector(uint8_t byJob, uint8_t bySex)
{
	return m_ItemScaleTable.v3MeshScale[bySex][byJob];
}
#endif

#ifdef ENABLE_SHINING_SYSTEM
bool CItemData::TItemShiningTable::Any() const
{
	for (int i = 0; i < CItemData::ITEM_SHINING_MAX_COUNT; i++)
	{
		if (strcmp(szShinings[i], ""))
			return true;
	}

	return false;
}
#endif
