#ifndef __INC_METIN_II_GAME_SECTREE_MANAGER_H__
#define __INC_METIN_II_GAME_SECTREE_MANAGER_H__

#include "sectree.h"


typedef struct SMapRegion
{
	int index;
	int sx, sy, ex, ey;
	PIXEL_POSITION posSpawn;

	bool bEmpireSpawnDifferent;
	PIXEL_POSITION posEmpire[3];

	std::string strMapName;
} TMapRegion;

struct TAreaInfo
{
	int sx, sy, ex, ey, dir;
	TAreaInfo(int sx, int sy, int ex, int ey, int dir)
		: sx(sx), sy(sy), ex(ex), ey(ey), dir(dir)
	{}
};

struct npc_info
{
	uint8_t bType;
	const char* name;
	uint32_t dwMobVnum;
	long x, y;
	npc_info(uint8_t bType, const char* name, uint32_t dwMobVnum, long x, long y)
		: bType(bType), name(name), dwMobVnum(dwMobVnum), x(x), y(y)
	{}
};

typedef std::map<std::string, TAreaInfo> TAreaMap;

typedef struct SSetting
{
	int iIndex;
	int iCellScale;
	int iBaseX;
	int iBaseY;
	int iWidth;
	int iHeight;

	PIXEL_POSITION posSpawn;
} TMapSetting;

class SECTREE_MAP
{
public:
	typedef std::map<uint32_t, LPSECTREE> MapType;

	SECTREE_MAP();
	SECTREE_MAP(SECTREE_MAP& r);
	virtual ~SECTREE_MAP();

	bool Add(uint32_t key, LPSECTREE sectree) { return map_.insert(MapType::value_type(key, sectree)).second; }

	LPSECTREE Find(uint32_t dwPackage);
	LPSECTREE Find(uint32_t x, uint32_t y);
	void Build();

	TMapSetting m_setting{};

	template< typename Func >
	void for_each(Func& rfunc)
	{
		// <Factor> Using snapshot copy to avoid side-effects
		FCollectEntity collector;
		std::map<uint32_t, LPSECTREE>::iterator it = map_.begin();
		for (; it != map_.end(); ++it)
		{
			LPSECTREE sectree = it->second;
			sectree->for_each_entity(collector);
		}
		collector.ForEach(rfunc);
		/*
		std::map<uint32_t,LPSECTREE>::iterator i = map_.begin();
		for (; i != map_.end(); ++i )
		{
			LPSECTREE pSec = i->second;
			pSec->for_each_entity( rfunc );
		}
		*/
	}

	void DumpAllToSysErr()
	{
		SECTREE_MAP::MapType::iterator i;
		for (i = map_.begin(); i != map_.end(); ++i)
		{
			sys_err("SECTREE %x(%u, %u)", i->first, i->first & 0xffff, i->first >> 16);
		}
	}

private:
	MapType map_;
};

enum EAttrRegionMode
{
	ATTR_REGION_MODE_SET,
	ATTR_REGION_MODE_REMOVE,
	ATTR_REGION_MODE_CHECK,
};

class SECTREE_MANAGER : public singleton<SECTREE_MANAGER>
{
public:
	SECTREE_MANAGER();
	virtual ~SECTREE_MANAGER();

	LPSECTREE_MAP GetMap(long lMapIndex);
	LPSECTREE Get(uint32_t dwIndex, uint32_t package);
	LPSECTREE Get(uint32_t dwIndex, uint32_t x, uint32_t y);

	template< typename Func >
	void for_each(int iMapIndex, Func& rfunc)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::Instance().GetMap(iMapIndex);
		if (pSecMap)
			pSecMap->for_each(rfunc);
	}

	int LoadSettingFile(long lIndex, const char* c_pszSettingFileName, TMapSetting& r_setting);
	bool LoadMapRegion(const char* c_pszFileName, TMapSetting& r_Setting, const char* c_pszMapName);
	int Build(const char* c_pszListFileName, const char* c_pszBasePath);
	LPSECTREE_MAP BuildSectreeFromSetting(TMapSetting& r_setting);
	bool LoadAttribute(LPSECTREE_MAP pkMapSectree, const char* c_pszFileName, TMapSetting& r_setting);
	void LoadDungeon(int iIndex, const char* c_pszFileName);
	bool GetValidLocation(long lMapIndex, long x, long y, long& r_lValidMapIndex, PIXEL_POSITION& r_pos, uint8_t empire = 0);
	bool GetSpawnPosition(long x, long y, PIXEL_POSITION& r_pos);
	bool GetSpawnPositionByMapIndex(long lMapIndex, PIXEL_POSITION& r_pos);
	bool GetRecallPositionByEmpire(int iMapIndex, uint8_t bEmpire, PIXEL_POSITION& r_pos);

	const TMapRegion* GetMapRegion(long lMapIndex);
	int GetMapIndex(long x, long y);
	const TMapRegion* FindRegionByPartialName(const char* szMapName);

	bool GetMapBasePosition(long x, long y, PIXEL_POSITION& r_pos);
	bool GetMapBasePositionByMapIndex(long lMapIndex, PIXEL_POSITION& r_pos);
	bool GetMovablePosition(long lMapIndex, long x, long y, PIXEL_POSITION& pos);
	bool IsMovablePosition(long lMapIndex, long x, long y);
	bool GetCenterPositionOfMap(long lMapIndex, PIXEL_POSITION& r_pos);
	bool GetRandomLocation(long lMapIndex, PIXEL_POSITION& r_pos, uint32_t dwCurrentX = 0, uint32_t dwCurrentY = 0, int iMaxDistance = 0);

	long CreatePrivateMap(long lMapIndex); // returns new private map index, returns 0 when fail
	void DestroyPrivateMap(long lMapIndex);

	TAreaMap& GetDungeonArea(long lMapIndex);
	void SendNPCPosition(LPCHARACTER ch);
	void InsertNPCPosition(long lMapIndex, uint8_t bType, const char* szName, uint32_t dwMobVnum, long x, long y);

	uint8_t GetEmpireFromMapIndex(long lMapIndex);

	void PurgeMonstersInMap(long lMapIndex);
	void PurgeStonesInMap(long lMapIndex);
	void PurgeNPCsInMap(long lMapIndex);
	size_t GetMonsterCountInMap(long lMapIndex);
	size_t GetMonsterCountInMap(long lMpaIndex, uint32_t dwVnum
#ifdef ENABLE_GUILD_DRAGONLAIR_SYSTEM
		, bool bForceVnum = false
#endif
	);

	/// Executes specific processing for Sectree's Attribute for the area.
	/**
	* @param [in] lMapIndex Map index to apply
	* @param [in] lStartX The leftmost coordinate of the rectangular area.
	* @param [in] lStartY The top coordinate of the rectangle area.
	* @param [in] lEndX Rightmost coordinate of the rectangular area
	* @param [in] lEndY The coordinates of the bottom of the rectangle
	* @param [in] lRotate The angle to rotate about the region.
	* @param [in] dwAttr Attribute to apply
	* @param [in] Type to be processed for mode Attribute
	*/
	bool ForAttrRegion(long lMapIndex, long lStartX, long lStartY, long lEndX, long lEndY, long lRotate, uint32_t dwAttr, EAttrRegionMode mode);

	bool SaveAttributeToImage(int lMapIndex, const char* c_pszFileName, LPSECTREE_MAP pMapSrc = nullptr);

private:

	/// Execute specific processing for Sectree's Attribute in the rectangular area.
	/**
	* @param [in] lMapIndex Map index to apply
	* @param [in] Coordinates of the leftmost Cell of the lCX rectangular area
	* @param [in] lCY The coordinates of the uppermost cell of the rectangular area
	* @param [in] LCW Rectangular area Cell unit width
	* @param [in] Cell unit height of lCH rectangular area
	* @param [in] lRotate Angle to rotate (perpendicular)
	* @param [in] dwAttr Attribute to apply
	* @param [in] Type to be processed for mode Attribute
	*/
	bool ForAttrRegionRightAngle(long lMapIndex, long lCX, long lCY, long lCW, long lCH, long lRotate, uint32_t dwAttr, EAttrRegionMode mode);

	/// Specific processing is performed for Sectree's Attribute for a non-rectangular area.
	/**
	* @param [in] lMapIndex Map index to apply
	* @param [in] Coordinates of the leftmost Cell of the lCX rectangular area
	* @param [in] lCY The coordinates of the uppermost cell of the rectangular area
	* @param [in] LCW Rectangular area Cell unit width
	* @param [in] Cell unit height of lCH rectangular area
	* @param [in] lRotate Angle to rotate (non-perpendicular angle)
	* @param [in] dwAttr Attribute to apply
	* @param [in] Type to be processed for mode Attribute
	*/
	bool ForAttrRegionFreeAngle(long lMapIndex, long lCX, long lCY, long lCW, long lCH, long lRotate, uint32_t dwAttr, EAttrRegionMode mode);

	/// Execute specific processing for the attribute of one Cell.
	/**
	* @param [in] lMapIndex Map index to apply
	* @param [in] X coordinate of cell to apply lCX
	* @param [in] Y coordinate of cell to apply lCY
	* @param [in] dwAttr Attribute to apply
	* @param [in] Type to be processed for mode Attribute
	*/
	bool ForAttrRegionCell(long lMapIndex, long lCX, long lCY, uint32_t dwAttr, EAttrRegionMode mode);

	static uint16_t current_sectree_version;
	std::map<uint32_t, LPSECTREE_MAP> m_map_pkSectree;
	std::map<int, TAreaMap> m_map_pkArea;
	std::vector<TMapRegion> m_vec_mapRegion;
	std::map<uint32_t, std::vector<npc_info> > m_mapNPCPosition;

	// <Factor> Circular private map indexing
	typedef std::unordered_map<long, int> PrivateIndexMapType;
	PrivateIndexMapType next_private_index_map_;
};

#endif
