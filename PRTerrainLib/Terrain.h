// Terrain.h: interface for the CTerrainImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TERRAIN_H__1C9108E1_69A7_4392_9B68_0E6CD4E1BFBA__INCLUDED_)
#	define AFX_TERRAIN_H__1C9108E1_69A7_4392_9B68_0E6CD4E1BFBA__INCLUDED_

#	if _MSC_VER > 1000
#		pragma once
#	endif // _MSC_VER > 1000

#	include "../eterImageLib/TGAImage.h"

#	include "TextureSet.h"
#	include "TerrainType.h"

class CTerrainImpl
{
public:
	enum
	{
		XSIZE = TERRAIN_SIZE,
		YSIZE = TERRAIN_SIZE,
		HEIGHTMAP_XSIZE = XSIZE + 1,
		HEIGHTMAP_YSIZE = YSIZE + 1,
		HEIGHTMAP_RAW_XSIZE = XSIZE + 3,
		HEIGHTMAP_RAW_YSIZE = YSIZE + 3,
		ATTRMAP_XSIZE = XSIZE * 2,
		ATTRMAP_YSIZE = YSIZE * 2,
		ATTRMAP_ZCOUNT = 8,
		TILEMAP_XSIZE = XSIZE * 2,
		TILEMAP_YSIZE = YSIZE * 2,
		TILEMAP_RAW_XSIZE = XSIZE * 2 + 2,
		TILEMAP_RAW_YSIZE = YSIZE * 2 + 2,
		WATERMAP_XSIZE = XSIZE,
		WATERMAP_YSIZE = YSIZE,
		NORMALMAP_XSIZE = XSIZE + 1,
		NORMALMAP_YSIZE = YSIZE + 1,
		SHADOWMAP_XSIZE = XSIZE * 2,
		SHADOWMAP_YSIZE = YSIZE * 2,
		SPLATALPHA_RAW_XSIZE = XSIZE * 2 + 2,
		SPLATALPHA_RAW_YSIZE = YSIZE * 2 + 2,
		SPLATALPHA_ZCOUNT = 7,
		HEIGHT_TILE_XRATIO = TILEMAP_XSIZE / XSIZE,
		HEIGHT_TILE_YRATIO = TILEMAP_XSIZE / XSIZE,

		PATCH_XSIZE = TERRAIN_PATCHSIZE,
		PATCH_YSIZE = TERRAIN_PATCHSIZE,
		PATCH_TILE_XSIZE = PATCH_XSIZE * HEIGHT_TILE_XRATIO,
		PATCH_TILE_YSIZE = PATCH_YSIZE * HEIGHT_TILE_YRATIO,
		PATCH_XCOUNT = TERRAIN_PATCHCOUNT,
		PATCH_YCOUNT = TERRAIN_PATCHCOUNT,

		CELLSCALE = 200,
		CELLSCALE_IN_METER = CELLSCALE / 100,
		HALF_CELLSCALE = CELLSCALE / 2,
		HALF_CELLSCALE_IN_METER = HALF_CELLSCALE / 100,
		TERRAIN_XSIZE = XSIZE * CELLSCALE,
		TERRAIN_YSIZE = YSIZE * CELLSCALE
	};

	enum
	{
		MAX_ATTRIBUTE_NUM = 8,
		MAX_WATER_NUM = 255
	};

	enum
	{
		ATTRIBUTE_BLOCK = (1 << 0),
		ATTRIBUTE_WATER = (1 << 1),
		ATTRIBUTE_BANPK = (1 << 2)
	};

public:
	static void SetTextureSet(CTextureSet * pTextureSet);
	static CTextureSet * GetTextureSet(); // 무조건 성공해야 함
	static void SetSnowTextureSet(CTextureSet * pTextureSet);
	static CTextureSet * GetSnowTextureSet();

protected:
	static CTextureSet * ms_pTextureSet;
	static CTextureSet * ms_pSnowTextureSet;

public:
	CTerrainImpl();
	virtual ~CTerrainImpl();

	TTerrainSplatPatch & GetTerrainSplatPatch() { return m_TerrainSplatPatch; }
	uint32_t GetNumTextures() { return ms_pTextureSet->GetTextureCount(); }
	TTerrainTexture & GetTexture(const long & c_rlTextureNum) { return ms_pTextureSet->GetTexture(c_rlTextureNum); }
	uint32_t					GetNumSnowTextures() { return ms_pSnowTextureSet->GetTextureCount(); }
	TTerrainTexture &		GetSnowTexture(const long & c_rlTextureNum) { return ms_pSnowTextureSet->GetTexture(c_rlTextureNum); }
	// 		int						GetHeightTileMapRatio()					{ return m_iHeightTileRatio; }

	bool LoadWaterMap(const char * c_szFileName);
	bool LoadWaterMapFile(const char * c_szFileName);

	LPDIRECT3DTEXTURE9 GetShadowTexture() { return m_lpShadowTexture; }

	uint32_t GetShadowMapColor(float fx, float fy);

protected:
	void Initialize();
	virtual void Clear();

	void LoadTextures();
	bool LoadHeightMap(const char * c_szFileName);
	bool RAW_LoadTileMap(const char * c_szFileName);
	bool LoadAttrMap(const char * c_szFileName);
#	ifdef WORLD_EDITOR
public:
#	endif
	__forceinline uint16_t GetHeightMapValue(int16_t sx, int16_t sy);

protected:
	LPDIRECT3DTEXTURE9 m_lpAlphaTexture[MAXTERRAINTEXTURES];
	uint16_t m_awRawHeightMap[HEIGHTMAP_RAW_YSIZE * HEIGHTMAP_RAW_XSIZE];

	uint8_t m_abyTileMap[TILEMAP_RAW_YSIZE * TILEMAP_RAW_XSIZE];
	uint8_t m_abyAttrMap[ATTRMAP_YSIZE * ATTRMAP_XSIZE];
	uint8_t m_abyWaterMap[WATERMAP_YSIZE * WATERMAP_XSIZE];
	CHAR m_acNormalMap[NORMALMAP_YSIZE * NORMALMAP_XSIZE * 3];

	TGA_HEADER m_HeightMapHeader;

	uint16_t m_wTileMapVersion;

	long m_lViewRadius;
	float m_fHeightScale;

	TTerrainSplatPatch m_TerrainSplatPatch;

	//////////////////////////////////////////////////////////////////////////
	//Water Map
	uint8_t m_byNumWater;
	long m_lWaterHeight[MAX_WATER_NUM + 1];
	//Water Map
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// TileMap과 HeightMap 비율
	// 		int						m_iHeightTileRatio;

	//////////////////////////////////////////////////////////////////////////
	// Shadow Map
	LPDIRECT3DTEXTURE9 m_lpShadowTexture;
	uint16_t m_awShadowMap[SHADOWMAP_YSIZE * SHADOWMAP_XSIZE]; // 16bit R5 G6 B5

protected:
	long m_lSplatTilesX;
	long m_lSplatTilesY;
};


__forceinline uint16_t CTerrainImpl::GetHeightMapValue(int16_t sx, int16_t sy)
{
	return m_awRawHeightMap[(sy + 1) * HEIGHTMAP_RAW_XSIZE + sx + 1];
}

#endif // !defined(AFX_TERRAIN_H__1C9108E1_69A7_4392_9B68_0E6CD4E1BFBA__INCLUDED_)
