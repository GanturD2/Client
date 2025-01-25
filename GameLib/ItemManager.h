#pragma once

#include "ItemData.h"
#ifdef ENABLE_DS_SET
#	include "DragonSoulTable.h"
#endif

class CItemManager : public CSingleton<CItemManager>
{
public:
	enum EItemDescCol
	{
		ITEMDESC_COL_VNUM,
		ITEMDESC_COL_NAME,
		ITEMDESC_COL_DESC,
		ITEMDESC_COL_SUMM,
		ITEMDESC_COL_NUM
	};

#ifdef ENABLE_SCALE_SYSTEM
	enum EItemScaleCol
	{
		ITEM_SCALE_COL_VNUM,
		ITEM_SCALE_COL_JOB,
		ITEM_SCALE_COL_SEX,
		ITEM_SCALE_COL_MESH_SCALE_X,
		ITEM_SCALE_COL_MESH_SCALE_Y,
		ITEM_SCALE_COL_MESH_SCALE_Z,
		ITEM_SCALE_COL_PARTICLE_SCALE,
		ITEM_SCALE_COL_NUM
	};
#endif

public:
	typedef std::map<uint32_t, CItemData *> TItemMap;
	typedef std::map<std::string, CItemData *> TItemNameMap;

#ifdef ENABLE_INGAME_WIKI
public:
	typedef std::vector<CItemData*> TItemVec;
	typedef std::vector<uint32_t> TItemNumVec;

public:
	void WikiAddVnumToBlacklist(uint32_t vnum)
	{
		const auto it = m_ItemMap.find(vnum);
		if (it != m_ItemMap.end())
			it->second->SetBlacklisted(true);
	};

	TItemNumVec* WikiGetLastItems()
	{
		return &m_tempItemVec;
	}

	bool							CanIncrSelectedItemRefineLevel();
	bool							CanIncrItemRefineLevel(uint32_t itemVnum);
	bool							CanLoadWikiItem(uint32_t dwVnum);
	uint32_t							GetWikiItemStartRefineVnum(uint32_t dwVnum);
	std::string						GetWikiItemBaseRefineName(uint32_t dwVnum);
	size_t							WikiLoadClassItems(uint8_t classType, uint32_t raceFilter);
	std::tuple<const char*, int>	SelectByNamePart(const char * namePart);

protected:
	TItemNumVec m_tempItemVec;

private:
	bool IsFilteredAntiflag(CItemData* itemData, uint32_t raceFilter);
#endif

public:
	CItemManager();
	virtual ~CItemManager();

	void Destroy();

	BOOL SelectItemData(uint32_t dwIndex);
	CItemData * GetSelectedItemDataPointer();

	BOOL GetItemDataPointer(uint32_t dwItemID, CItemData ** ppItemData);

	/////
	bool LoadItemDesc(const char * c_szFileName);
	bool LoadItemList(const char * c_szFileName);
	bool LoadItemTable(const char * c_szFileName);
	CItemData * MakeItemData(uint32_t dwIndex);
	TItemMap GetItems() const { return m_ItemMap; }	//ENABLE_SEARCH_ITEM_DROP_ON_MOB || ENABLE_PRIVATESHOP_SEARCH_SYSTEM

#ifdef ENABLE_SCALE_SYSTEM
	bool LoadItemScale(const char* szItemScale);
#endif

#ifdef ENABLE_DS_SET
public:
	bool LoadDragonSoulTable(const char* c_szFileName);
	float GetDSSetWeight(uint8_t bDSType, uint8_t bDSGrade) const;
	int GetDSBasicApplyCount(uint8_t bDSType, uint8_t bDSGrade) const;
	int GetDSBasicApplyValue(uint8_t bDSType, uint16_t wApplyType) const;
	int GetDSAdditionalApplyValue(uint8_t bDSType, uint16_t wApplyType) const;

protected:
	CDragonSoulTable* m_pDSTable;
#endif

#ifdef ENABLE_SHINING_SYSTEM
public:
	bool LoadShiningTable(const char* c_szFileName);
#endif

protected:
	TItemMap m_ItemMap;
	std::vector<CItemData *> m_vec_ItemRange;
	CItemData * m_pSelectedItemData;
};
