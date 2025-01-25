#ifndef __INC_ITEM_MANAGER__
#define __INC_ITEM_MANAGER__

#ifdef ENABLE_INGAME_WIKI
# include <memory>
#endif

// special_item_group.txt
class CSpecialAttrGroup
{
public:
	CSpecialAttrGroup(uint32_t vnum)
		: m_dwVnum(vnum)
	{}

#ifdef ENABLE_EXTENDED_RELOAD
	void Clear()
	{
		if (!m_vecAttrs.empty())
			m_vecAttrs.clear();
	}
#endif

	struct CSpecialAttrInfo
	{
		CSpecialAttrInfo(uint32_t _apply_type, uint32_t _apply_value)
			: apply_type(_apply_type), apply_value(_apply_value)
		{}
		uint32_t apply_type;
		uint32_t apply_value;

	};
	uint32_t m_dwVnum;
	std::string m_stEffectFileName;
	std::vector<CSpecialAttrInfo> m_vecAttrs;
};

class CSpecialItemGroup
{
public:
	enum EGiveType
	{
		NONE,
		GOLD,
		EXP,
		MOB,
		SLOW,
		DRAIN_HP,
		POISON,
		MOB_GROUP,
#ifdef ENABLE_WOLFMAN_CHARACTER
		BLEEDING,
#endif
	};

	enum ESIGType { NORMAL, PCT, QUEST, SPECIAL };

	struct CSpecialItemInfo
	{
		uint32_t vnum;
		int count;
		int rare;

		CSpecialItemInfo(uint32_t _vnum, int _count, int _rare)
			: vnum(_vnum), count(_count), rare(_rare)
		{}
	};

	CSpecialItemGroup(uint32_t vnum, uint8_t type = 0)
		: m_dwVnum(vnum), m_bType(type)
	{}

	void AddItem(uint32_t vnum, int count, int prob, int rare)
	{
		if (!prob)
			return;
		if (!m_vecProbs.empty())
			prob += m_vecProbs.back();
		m_vecProbs.emplace_back(prob);
		m_vecItems.emplace_back(CSpecialItemInfo(vnum, count, rare));
	}

	bool IsEmpty() const
	{
		return m_vecProbs.empty();
	}

	int GetMultiIndex(std::vector <int>& idx_vec) const
	{
		idx_vec.clear();
		if (m_bType == PCT)
		{
			int count = 0;
			if (number(1, 100) <= m_vecProbs[0])
			{
				idx_vec.emplace_back(0);
				count++;
			}
			for (uint i = 1; i < m_vecProbs.size(); i++)
			{
				if (number(1, 100) <= m_vecProbs[i] - m_vecProbs[i - 1])
				{
					idx_vec.emplace_back(i);
					count++;
				}
			}
			return count;
		}
		else
		{
			idx_vec.emplace_back(GetOneIndex());
			return 1;
		}
	}

	int GetOneIndex() const
	{
		const int n = number(1, m_vecProbs.back());
		const itertype(m_vecProbs) it = lower_bound(m_vecProbs.begin(), m_vecProbs.end(), n);
		return std::distance(m_vecProbs.begin(), it);
	}

	int GetVnum(int idx) const
	{
		return m_vecItems[idx].vnum;
	}

	int GetCount(int idx) const
	{
		return m_vecItems[idx].count;
	}

	int GetRarePct(int idx) const
	{
		return m_vecItems[idx].rare;
	}

	bool Contains(uint32_t dwVnum) const
	{
		for (uint32_t i = 0; i < m_vecItems.size(); i++)
		{
			if (m_vecItems[i].vnum == dwVnum)
				return true;
		}
		return false;
	}

	uint32_t GetAttrVnum(uint32_t dwVnum) const
	{
		if (CSpecialItemGroup::SPECIAL != m_bType)
			return 0;
		for (auto it : m_vecItems)
		{
			if (it.vnum == dwVnum)
			{
				return it.count;
			}
		}
		return 0;
	}

	int GetGroupSize() const
	{
		return m_vecProbs.size();
	}

#ifdef ENABLE_EXTENDED_RELOAD
	void Clear()
	{
		if (!m_vecProbs.empty())
			m_vecProbs.clear();

		if (!m_vecItems.empty())
			m_vecItems.clear();
	}
#endif

	uint32_t m_dwVnum;
	uint8_t m_bType;
	std::vector<int> m_vecProbs;
	std::vector<CSpecialItemInfo> m_vecItems; // vnum, count
};

class CMobItemGroup
{
public:
	struct SMobItemGroupInfo
	{
		uint32_t dwItemVnum;
		int iCount;
		int iRarePct;

		SMobItemGroupInfo(uint32_t dwItemVnum, int iCount, int iRarePct)
			: dwItemVnum(dwItemVnum),
			iCount(iCount),
			iRarePct(iRarePct)
		{
		}
	};

	CMobItemGroup(uint32_t dwMobVnum, int iKillDrop, const std::string& r_stName)
		:
		m_dwMobVnum(dwMobVnum),
		m_iKillDrop(iKillDrop),
		m_stName(r_stName)
	{
	}

	int GetKillPerDrop() const
	{
		return m_iKillDrop;
	}

	void AddItem(uint32_t dwItemVnum, int iCount, int iPartPct, int iRarePct)
	{
		if (!m_vecProbs.empty())
			iPartPct += m_vecProbs.back();
		m_vecProbs.emplace_back(iPartPct);
		m_vecItems.emplace_back(SMobItemGroupInfo(dwItemVnum, iCount, iRarePct));
	}

	// MOB_DROP_ITEM_BUG_FIX
	bool IsEmpty() const
	{
		return m_vecProbs.empty();
	}

	int GetOneIndex() const
	{
		const int n = number(1, m_vecProbs.back());
		const itertype(m_vecProbs) it = lower_bound(m_vecProbs.begin(), m_vecProbs.end(), n);
		return std::distance(m_vecProbs.begin(), it);
	}
	// END_OF_MOB_DROP_ITEM_BUG_FIX

	const SMobItemGroupInfo& GetOne() const
	{
		return m_vecItems[GetOneIndex()];
	}

private:
	uint32_t m_dwMobVnum;
	int m_iKillDrop;
	std::string m_stName;
	std::vector<int> m_vecProbs;
	std::vector<SMobItemGroupInfo> m_vecItems;
};

class CDropItemGroup
{
	struct SDropItemGroupInfo
	{
		uint32_t dwVnum;
		uint32_t dwPct;
		int iCount;

		SDropItemGroupInfo(uint32_t dwVnum, uint32_t dwPct, int iCount)
			: dwVnum(dwVnum), dwPct(dwPct), iCount(iCount)
		{}
	};

public:
	CDropItemGroup(uint32_t dwVnum, uint32_t dwMobVnum, const std::string& r_stName)
		:
		m_dwVnum(dwVnum),
		m_dwMobVnum(dwMobVnum),
		m_stName(r_stName)
	{
	}

	const std::vector<SDropItemGroupInfo>& GetVector()
	{
		return m_vec_items;
	}

	void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
	{
		m_vec_items.emplace_back(SDropItemGroupInfo(dwItemVnum, dwPct, iCount));
	}

private:
	uint32_t m_dwVnum;
	uint32_t m_dwMobVnum;
	std::string m_stName;
	std::vector<SDropItemGroupInfo> m_vec_items;
};

class CLevelItemGroup
{
	struct SLevelItemGroupInfo
	{
		uint32_t dwVNum;
		uint32_t dwPct;
		int iCount;

		SLevelItemGroupInfo(uint32_t dwVnum, uint32_t dwPct, int iCount)
			: dwVNum(dwVnum), dwPct(dwPct), iCount(iCount)
		{ }
	};

private:
	uint32_t m_dwLevelLimit;
	std::string m_stName;
	std::vector<SLevelItemGroupInfo> m_vec_items;

public:
	CLevelItemGroup(uint32_t dwLevelLimit)
		: m_dwLevelLimit(dwLevelLimit)
	{}

	uint32_t GetLevelLimit() { return m_dwLevelLimit; }

	void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
	{
		m_vec_items.emplace_back(SLevelItemGroupInfo(dwItemVnum, dwPct, iCount));
	}

	const std::vector<SLevelItemGroupInfo>& GetVector()
	{
		return m_vec_items;
	}
};

#ifdef MOB_DROP_ITEM_RENEWAL
class CMapIndexItemGroup
{
	struct SMapIndexItemGroup
	{
		uint32_t dwVNum;
		uint32_t dwPct;
		int iCount;

		SMapIndexItemGroup(uint32_t dwVnum, uint32_t dwPct, int iCount)
			: dwVNum(dwVnum), dwPct(dwPct), iCount(iCount)
		{ }
	};

private:
	uint32_t m_dwMapIndex;
	std::string m_stName;
	std::vector<SMapIndexItemGroup> m_vec_items;

public:
	CMapIndexItemGroup(uint32_t dwMapIndex)
		: m_dwMapIndex(dwMapIndex)
	{}

	uint32_t GetMapIndex() { return m_dwMapIndex; }

	void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
	{
		m_vec_items.emplace_back(SMapIndexItemGroup(dwItemVnum, dwPct, iCount));
	}

	const std::vector<SMapIndexItemGroup>& GetVector()
	{
		return m_vec_items;
	}
};
#endif

class CBuyerThiefGlovesItemGroup
{
	struct SThiefGroupInfo
	{
		uint32_t dwVnum;
		uint32_t dwPct;
		int iCount;

		SThiefGroupInfo(uint32_t dwVnum, uint32_t dwPct, int iCount)
			: dwVnum(dwVnum), dwPct(dwPct), iCount(iCount)
		{}
	};

public:
	CBuyerThiefGlovesItemGroup(uint32_t dwVnum, uint32_t dwMobVnum, const std::string& r_stName)
		:
		m_dwVnum(dwVnum),
		m_dwMobVnum(dwMobVnum),
		m_stName(r_stName)
	{
	}

	const std::vector<SThiefGroupInfo>& GetVector()
	{
		return m_vec_items;
	}

	void AddItem(uint32_t dwItemVnum, uint32_t dwPct, int iCount)
	{
		m_vec_items.emplace_back(SThiefGroupInfo(dwItemVnum, dwPct, iCount));
	}

private:
	uint32_t m_dwVnum;
	uint32_t m_dwMobVnum;
	std::string m_stName;
	std::vector<SThiefGroupInfo> m_vec_items;
};

class ITEM;

class ITEM_MANAGER : public singleton<ITEM_MANAGER>
{
public:
	ITEM_MANAGER();
	virtual ~ITEM_MANAGER();

	bool Initialize(TItemTable* table, int size);
	void Destroy();
	void Update();
	void GracefulShutdown();

#ifdef ENABLE_INGAME_WIKI
	uint32_t GetWikiItemStartRefineVnum(uint32_t dwVnum);
	std::string GetWikiItemBaseRefineName(uint32_t dwVnum);
	int GetWikiMaxRefineLevel(uint32_t dwVnum);

	TWikiInfoTable* GetItemWikiInfo(uint32_t vnum);
	bool GetWikiRefineInfo(uint32_t vnum, std::vector<TWikiRefineInfo>& wikiVec);
	bool GetWikiChestInfo(uint32_t dwItemVnum, std::vector<CSpecialItemGroup::CSpecialItemInfo>& vec_item);
	std::vector<TWikiItemOriginInfo>& GetItemOrigin(uint32_t vnum) { return m_itemOriginMap[vnum]; }
#endif

	uint32_t GetNewID();
	bool SetMaxItemID(TItemIDRangeTable range);
	bool SetMaxSpareItemID(TItemIDRangeTable range);

	void DelayedSave(LPITEM item);
	void FlushDelayedSave(LPITEM item);
	void SaveSingleItem(LPITEM item);

	LPITEM CreateItem(uint32_t vnum, uint32_t count = 1, uint32_t dwID = 0, bool bTryMagic = false, int iRarePct = -1, bool bSkipSave = false);
	void DestroyItem(LPITEM item);
	void RemoveItem(LPITEM item, const char* c_pszReason = nullptr);

	LPITEM Find(uint32_t id);
	LPITEM FindByVID(uint32_t vid);
	TItemTable* GetTable(uint32_t vnum);
	bool GetVnum(const char* c_pszName, uint32_t& r_dwVnum);
	bool GetVnumByOriginalName(const char* c_pszName, uint32_t& r_dwVnum);

	bool GetDropPct(LPCHARACTER pkChr, LPCHARACTER pkKiller, OUT int& iDeltaPercent, OUT int& iRandRange);
#ifdef ENABLE_NEWSTUFF
	bool GetDropPctCustom(LPCHARACTER pkChr, LPCHARACTER pkKiller, OUT int& iDeltaPercent, OUT int& iRandRange);
#endif
	bool CreateDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item);
#ifdef ENABLE_SEND_TARGET_INFO
	bool CreateDropItemVector(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item);
#endif

	bool ReadCommonDropItemFile(const char* c_pszFileName);
	bool ReadEtcDropItemFile(const char* c_pszFileName);
	bool ReadDropItemGroup(const char* c_pszFileName);
	bool ReadMonsterDropItemGroup(const char* c_pszFileName);
	bool ReadSpecialDropItemFile(const char* c_pszFileName);
#ifdef ENABLE_EXTENDED_RELOAD
	bool ReloadMobDropItemGroup(const char* c_pszFileName);
	bool ReloadSpecialItemGroup(const char* c_pszFileName);
#endif

	// convert name -> vnum special_item_group.txt
	bool ConvSpecialDropItemFile();
	// convert name -> vnum special_item_group.txt

	uint32_t GetRefineFromVnum(uint32_t dwVnum);

	static void CopyAllAttrTo(LPITEM pkOldItem, LPITEM pkNewItem);

	const CSpecialItemGroup* GetSpecialItemGroup(uint32_t dwVnum);
	const CSpecialAttrGroup* GetSpecialAttrGroup(uint32_t dwVnum);

	const std::vector<TItemTable>& GetTable() { return m_vec_prototype; }

	// CHECK_UNIQUE_GROUP
	int GetSpecialGroupFromItem(uint32_t dwVnum) const { const itertype(m_ItemToSpecialGroup) it = m_ItemToSpecialGroup.find(dwVnum); return (it == m_ItemToSpecialGroup.end()) ? 0 : it->second; }
	// END_OF_CHECK_UNIQUE_GROUP

protected:
	int RealNumber(uint32_t vnum);
	void CreateQuestDropItem(LPCHARACTER pkChr, LPCHARACTER pkKiller, std::vector<LPITEM>& vec_item, int iDeltaPercent, int iRandRange);
#ifdef ENABLE_EVENT_MANAGER
	void DropEventItem(const CHARACTER& pkKiller, const CHARACTER& pkChr, ITEM_MANAGER& itemMgr, std::vector<LPITEM>& vec_item,
		uint32_t dwVnum, uint8_t iLevelDiff, uint8_t bDropType, int iChanceMax, int iDeltaPercent, int iRandRange);
#endif

protected:
	typedef std::map<uint32_t, LPITEM> ITEM_VID_MAP;

	std::vector<TItemTable> m_vec_prototype;
	std::vector<TItemTable*> m_vec_item_vnum_range_info;
	std::map<uint32_t, uint32_t> m_map_ItemRefineFrom;
	int m_iTopOfTable;

#ifdef ENABLE_INGAME_WIKI
	std::map<uint32_t, std::unique_ptr<TWikiInfoTable>> m_wikiInfoMap;
	std::map<uint32_t, std::vector<TWikiItemOriginInfo>> m_itemOriginMap;
#endif

	ITEM_VID_MAP m_VIDMap;
	uint32_t m_dwVIDCount;
	uint32_t m_dwCurrentID;
	TItemIDRangeTable m_ItemIDRange;
	TItemIDRangeTable m_ItemIDSpareRange;

	std::unordered_set<LPITEM> m_set_pkItemForDelayedSave;
	std::map<uint32_t, LPITEM> m_map_pkItemByID;
	std::map<uint32_t, uint32_t> m_map_dwEtcItemDropProb;
	std::map<uint32_t, CDropItemGroup*> m_map_pkDropItemGroup;
	std::map<uint32_t, CSpecialItemGroup*> m_map_pkSpecialItemGroup;
	std::map<uint32_t, CSpecialItemGroup*> m_map_pkQuestItemGroup;
	std::map<uint32_t, CSpecialAttrGroup*> m_map_pkSpecialAttrGroup;
	std::map<uint32_t, CMobItemGroup*> m_map_pkMobItemGroup;
	std::map<uint32_t, CLevelItemGroup*> m_map_pkLevelItemGroup;
#ifdef MOB_DROP_ITEM_RENEWAL
	std::map<uint32_t, CMapIndexItemGroup*> m_map_pkMapIndexItemGroup;
#endif
	std::map<uint32_t, CBuyerThiefGlovesItemGroup*> m_map_pkGloveItemGroup;

	// CHECK_UNIQUE_GROUP
	std::map<uint32_t, int> m_ItemToSpecialGroup;
	// END_OF_CHECK_UNIQUE_GROUP

private:
	typedef std::map <uint32_t, uint32_t> TMapDW2DW;
	TMapDW2DW m_map_new_to_ori;

public:
	uint32_t GetMaskVnum(uint32_t dwVnum);
	std::map<uint32_t, TItemTable> m_map_vid;
	std::map<uint32_t, TItemTable>& GetVIDMap() { return m_map_vid; }
	std::vector<TItemTable>& GetVecProto() { return m_vec_prototype; }

	const static int MAX_NORM_ATTR_NUM = ITEM_ATTRIBUTE_NORM_NUM;
	const static int MAX_RARE_ATTR_NUM = ITEM_ATTRIBUTE_RARE_NUM;

	bool ReadItemVnumMaskTable(const char* c_pszFileName);
};

#define M2_DESTROY_ITEM(ptr) ITEM_MANAGER::Instance().DestroyItem(ptr)

#endif
