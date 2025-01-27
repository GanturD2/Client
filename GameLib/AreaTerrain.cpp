#include "StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"

#include "../EterLib/ResourceManager.h"
#include "../EterLib/StateManager.h"
#include "../EterPack/EterPackManager.h"

#include "AreaTerrain.h"
#include "MapOutdoor.h"

CDynamicPool<CTerrain> CTerrain::ms_kPool;

void CTerrain::DestroySystem()
{
	ms_kPool.Destroy();
}

CTerrain * CTerrain::New()
{
	return ms_kPool.Alloc();
}

void CTerrain::Delete(CTerrain * pkTerrain)
{
	pkTerrain->Clear();
	ms_kPool.Free(pkTerrain);
}

CTerrain::CTerrain()
{
	msl::refill(m_lpAlphaTexture);
	m_lpMarkedTexture = {};
	Initialize();
}

CTerrain::~CTerrain()
{
	DeallocateMarkedSplats();
	RAW_DeallocateSplats();
	Clear();
}

void CTerrain::SetMapOutDoor(CMapOutdoor * pOwnerOutdoorMap)
{
	m_pOwnerOutdoorMap = pOwnerOutdoorMap;
}

void CTerrain::Clear()
{
	DeallocateMarkedSplats();
	CTerrainImpl::Clear();
	Initialize();
}

bool CTerrain::Initialize()
{
	SetReady(false);
	m_strName = "";
	m_wX = m_wY = 0xFFFF;
	m_bReady = false;
	m_bMarked = false;

	for (uint8_t byY = 0; byY < PATCH_YCOUNT; ++byY)
		for (uint8_t byX = 0; byX < PATCH_XCOUNT; ++byX)
			m_TerrainPatchList[byY * PATCH_XCOUNT + byX].Clear();

	return true;
}

void CTerrain::LoadMiniMapTexture(const char * c_pchMiniMapFileName)
{
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	m_MiniMapGraphicImageInstance.Destroy();
#endif

	uint32_t dwStart = ELTimer_GetMSec();
	auto * pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(c_pchMiniMapFileName));
	m_MiniMapGraphicImageInstance.SetImagePointer(pImage);

	if (!m_MiniMapGraphicImageInstance.GetTexturePointer()->IsEmpty())
	{
		m_lpMiniMapTexture = m_MiniMapGraphicImageInstance.GetTexturePointer()->GetD3DTexture();
		Tracef("CTerrain::LoadMiniMapTexture %d ms\n", ELTimer_GetMSec() - dwStart);
	}
	else
	{
		Tracef(" CTerrain::LoadMiniMapTexture - MiniMapTexture Error");
		m_lpMiniMapTexture = nullptr;
	}
}

void CTerrain::LoadShadowTexture(const char * ShadowFileName)
{
	uint32_t dwStart = ELTimer_GetMSec();
	auto * pImage = msl::inherit_cast<CGraphicImage *>(CResourceManager::Instance().GetResourcePointer(ShadowFileName));
	m_ShadowGraphicImageInstance.SetImagePointer(pImage);

	if (!m_ShadowGraphicImageInstance.GetTexturePointer()->IsEmpty())
		m_lpShadowTexture = m_ShadowGraphicImageInstance.GetTexturePointer()->GetD3DTexture();
	else
	{
		TraceError(" CTerrain::LoadShadowTexture - ShadowTexture is Empty");
		m_lpShadowTexture = nullptr;
	}
	Tracef("CTerrain::LoadShadowTexture %d ms\n", ELTimer_GetMSec() - dwStart);
}

bool CTerrain::LoadShadowMap(const char * c_pszFileName)
{
	uint32_t dwStart = ELTimer_GetMSec();
	Tracef("LoadShadowMap %s ", c_pszFileName);

	CMappedFile file;
	LPCVOID c_pvData;

	if (!CEterPackManager::Instance().Get(file, c_pszFileName, &c_pvData))
	{
		TraceError(" CTerrain::LoadShadowMap - %s OPEN ERROR", c_pszFileName);
		return false;
	}

	uint32_t dwShadowMapSize = sizeof(uint16_t) * 256 * 256;

	if (file.Size() != dwShadowMapSize)
	{
		TraceError(" CTerrain::LoadShadowMap - %s SIZE ERROR", c_pszFileName);
		return false;
	}

	memcpy(m_awShadowMap, c_pvData, dwShadowMapSize);

	Tracef("%d ms\n", ELTimer_GetMSec() - dwStart);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Seamless용 새로운 함수들...
//////////////////////////////////////////////////////////////////////////

void CTerrain::CopySettingFromGlobalSetting()
{
	m_lViewRadius = m_pOwnerOutdoorMap->GetViewRadius();
	m_fHeightScale = m_pOwnerOutdoorMap->GetHeightScale();
}

uint16_t CTerrain::WE_GetHeightMapValue(int16_t sX, int16_t sY)
{
	if (sX >= -1 && sY >= -1 && sX < HEIGHTMAP_RAW_XSIZE - 1 && sY < HEIGHTMAP_RAW_YSIZE - 1)
		return GetHeightMapValue(sX, sY);

	uint8_t byTerrainNum;
	if (!m_pOwnerOutdoorMap->GetTerrainNumFromCoord(m_wX, m_wY, &byTerrainNum))
	{
		Tracef("CTerrain::WE_GetHeightMapValue : Can't Get TerrainNum from Coord %d, %d", m_wX, m_wY);
		byTerrainNum = 4;
	}

	int16_t sTerrainCouuntX, sTerrainCouuntY;
	m_pOwnerOutdoorMap->GetTerrainCount(&sTerrainCouuntX, &sTerrainCouuntY);

	CTerrain * pTerrain = nullptr;

	if (sY < -1)
	{
		if (m_wY <= 0)
		{
			if (sX < -1)
			{
				if (m_wX <= 0)
					return GetHeightMapValue(-1, -1);
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 1, &pTerrain))
						return GetHeightMapValue(-1, -1);
					else
						return pTerrain->GetHeightMapValue(sX + XSIZE, -1);
				}
			}
			else if (sX >= HEIGHTMAP_RAW_XSIZE - 1)
			{
				if (m_wX >= sTerrainCouuntX - 1)
					return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, -1);
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 1, &pTerrain))
						return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, -1);
					else
						return pTerrain->GetHeightMapValue(sX - XSIZE, -1);
				}
			}
			else
				return GetHeightMapValue(sX, -1);
		}
		else
		{
			if (sX < -1)
			{
				if (m_wX <= 0)
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 3, &pTerrain))
						return GetHeightMapValue(-1, -1);
					else
						return pTerrain->GetHeightMapValue(-1, sY + YSIZE);
				}
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 4, &pTerrain))
						return GetHeightMapValue(-1, -1);
					else
						return pTerrain->GetHeightMapValue(sX + XSIZE, sY + YSIZE);
				}
			}
			else if (sX >= HEIGHTMAP_RAW_XSIZE - 1)
			{
				if (m_wX >= sTerrainCouuntX)
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 3, &pTerrain))
						return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, -1);
					else
						return pTerrain->GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, sY + YSIZE);
				}
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 2, &pTerrain))
						return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, -1);
					else
						return pTerrain->GetHeightMapValue(sX - XSIZE, sY + YSIZE);
				}
			}
			else
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 3, &pTerrain))
					return GetHeightMapValue(sX, -1);
				else
					return pTerrain->GetHeightMapValue(sX, sY + YSIZE);
			}
		}
	}
	else if (sY >= HEIGHTMAP_RAW_YSIZE - 1)
	{
		if (m_wY >= sTerrainCouuntY - 1)
		{
			if (sX < -1)
			{
				if (m_wX <= 0)
					return GetHeightMapValue(-1, HEIGHTMAP_RAW_XSIZE - 1);
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 1, &pTerrain))
						return GetHeightMapValue(-1, HEIGHTMAP_RAW_XSIZE - 1);
					else
						return pTerrain->GetHeightMapValue(sX + XSIZE, HEIGHTMAP_RAW_YSIZE - 1);
				}
			}
			else if (sX >= HEIGHTMAP_RAW_XSIZE - 1)
			{
				if (m_wX >= sTerrainCouuntX - 1)
					return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, HEIGHTMAP_RAW_YSIZE - 1);
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 1, &pTerrain))
						return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, HEIGHTMAP_RAW_YSIZE - 1);
					else
						return pTerrain->GetHeightMapValue(sX - XSIZE, HEIGHTMAP_RAW_YSIZE - 1);
				}
			}
			else
				return GetHeightMapValue(sX, HEIGHTMAP_RAW_YSIZE - 1);
		}
		else
		{
			if (sX < -1)
			{
				if (m_wX <= 0)
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 3, &pTerrain))
						return GetHeightMapValue(-1, HEIGHTMAP_RAW_YSIZE - 1);
					else
						return pTerrain->GetHeightMapValue(-1, sY - YSIZE);
				}
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 2, &pTerrain))
						return GetHeightMapValue(-1, HEIGHTMAP_RAW_XSIZE - 1);
					else
						return pTerrain->GetHeightMapValue(sX + XSIZE, sY - YSIZE);
				}
			}
			else if (sX >= HEIGHTMAP_RAW_XSIZE - 1)
			{
				if (m_wX >= sTerrainCouuntX - 1)
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 3, &pTerrain))
						return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, HEIGHTMAP_RAW_YSIZE - 1);
					else
						return pTerrain->GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, sY - YSIZE);
				}
				else
				{
					if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 4, &pTerrain))
						return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, HEIGHTMAP_RAW_YSIZE - 1);
					else
						return pTerrain->GetHeightMapValue(sX - XSIZE, sY - YSIZE);
				}
			}
			else
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 3, &pTerrain))
					return GetHeightMapValue(sX, HEIGHTMAP_RAW_YSIZE - 1);
				else
					return pTerrain->GetHeightMapValue(sX, sY - YSIZE);
			}
		}
	}
	else
	{
		if (sX < -1)
		{
			if (m_wX <= 0)
				return GetHeightMapValue(-1, sY);
			else
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum - 1, &pTerrain))
					return GetHeightMapValue(-1, sY);
				else
					return pTerrain->GetHeightMapValue(sX + XSIZE, sY);
			}
		}
		else if (sX >= HEIGHTMAP_RAW_XSIZE - 1)
		{
			if (m_wX >= sTerrainCouuntX - 1)
				return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, sY);
			else
			{
				if (!m_pOwnerOutdoorMap->GetTerrainPointer(byTerrainNum + 1, &pTerrain))
					return GetHeightMapValue(HEIGHTMAP_RAW_XSIZE - 1, sY);
				else
					return pTerrain->GetHeightMapValue(sX - XSIZE, sY);
			}
		}
		else
			return GetHeightMapValue(sX, sY);
	}
}

bool CTerrain::GetNormal(int ix, int iy, D3DXVECTOR3 * pv3Normal)
{
	long lMapWidth = XSIZE * CELLSCALE;
	long lMapHeight = YSIZE * CELLSCALE;
	while (ix < 0)
		ix += lMapWidth;

	while (iy < 0)
		iy += lMapHeight;

	while (ix > lMapWidth)
		ix -= lMapWidth;

	while (iy > lMapHeight)
		iy -= lMapHeight;

	ix /= CELLSCALE;
	iy /= CELLSCALE;

	D3DXVECTOR3 v3Noraml;
	auto * n = (char *) &m_acNormalMap[(iy * NORMALMAP_XSIZE + ix) * 3];
	pv3Normal->x = -((float) *n++) * 0.007874016f;
	pv3Normal->y = ((float) *n++) * 0.007874016f;
	pv3Normal->z = ((float) *n++) * 0.007874016f;

	return true;
}
// Returns the height of the terrain at the given world coordinate
float CTerrain::GetHeight(int x, int y)
{
	//if (0 == CELLSCALE)
	//return 0.0f;

	x -= m_wX * XSIZE * CELLSCALE;
	y -= m_wY * YSIZE * CELLSCALE;

	if (x < 0 || y < 0 || x > XSIZE * CELLSCALE || y > XSIZE * CELLSCALE)
		return 0.0f;

	long xdist; /* x mod size of tile */
	long ydist; /* y mod size of tile */
	float xslope, yslope; /* slopes of heights between vertices */

	float h1, h2, h3;
	long x2, y2;
	float ooscale;

	/* Find out the distance relative to the top left vertex of a tile */
	xdist = x % CELLSCALE;
	ydist = y % CELLSCALE;

	/* Convert into pixel coordinates */
	ooscale = 1.0f / ((float) CELLSCALE);
	x /= CELLSCALE;
	y /= CELLSCALE;

	x2 = x;
	y2 = y;
	/* Get the height and color of the pixel at the top left corner */
	h1 = (float) GetHeightMapValue(x2, y2) * m_fHeightScale;

	/* Get the height and color of the pixel at the bottom right corner */
	x2 = x + 1;
	y2 = y + 1;

	h2 = (float) GetHeightMapValue(x2, y2) * m_fHeightScale;

	/* Left triangle */
	if (xdist <= ydist)
	{
		x2 = x;
		y2 = y + 1;

		h3 = (float) GetHeightMapValue(x2, y2) * m_fHeightScale;

		/* Get the height of the pixel at the bottom left corner */
		xslope = (h2 - h3) * ooscale;
		yslope = (h3 - h1) * ooscale;

		return (h1 + (xdist * xslope + ydist * yslope));
	}

	/* Right triangle */
	x2 = x + 1;
	y2 = y;

	h3 = (float) GetHeightMapValue(x2, y2) * m_fHeightScale;

	/* Get the height of the pixel at the top right corner */
	xslope = (h3 - h1) * ooscale;
	yslope = (h2 - h3) * ooscale;

	return (h1 + (xdist * xslope + ydist * yslope));
}

//////////////////////////////////////////////////////////////////////////
// HeightMapCoord -> TileMapCoord

void CTerrain::CalculateNormal(long x, long y)
{
	D3DXVECTOR3 normal;

	normal.x = -m_fHeightScale * ((float) GetHeightMapValue((x - 1), y) - (float) GetHeightMapValue((x + 1), y));
	normal.y = -m_fHeightScale * ((float) GetHeightMapValue(x, (y - 1)) - (float) GetHeightMapValue(x, (y + 1)));

	normal.z = 2.0f * CELLSCALE;
	normal *= 127.0f / D3DXVec3Length(&normal);

	int ix, iy, iz;
	PR_FLOAT_TO_INT(normal.x, ix);
	PR_FLOAT_TO_INT(normal.y, iy);
	PR_FLOAT_TO_INT(normal.z, iz);

	auto * n = (char *) &m_acNormalMap[(y * NORMALMAP_XSIZE + x) * 3];

	*n++ = (char) ix;
	*n++ = (char) iy;
	*n++ = (char) iz;
}

bool CTerrain::RAW_LoadTileMap(const char * c_pszFileName, bool bBGLoading)
{
	CTerrainImpl::RAW_LoadTileMap(c_pszFileName);
	uint32_t dwStart = ELTimer_GetMSec();
	RAW_AllocateSplats(bBGLoading);
	Tracef("CTerrain::RAW_AllocateSplats %d\n", ELTimer_GetMSec() - dwStart);
	return true;
}

bool CTerrain::LoadHeightMap(const char * c_pszFileName)
{
	CTerrainImpl::LoadHeightMap(c_pszFileName);
	uint32_t dwStart = ELTimer_GetMSec();
	for (uint16_t y = 0; y < NORMALMAP_YSIZE; ++y)
		for (uint16_t x = 0; x < NORMALMAP_XSIZE; ++x)
			CalculateNormal(x, y);

	Tracef("LoadHeightMap::CalculateNormal %d ms\n", ELTimer_GetMSec() - dwStart);
	return true;
}

bool CTerrain::LoadAttrMap(const char * c_pszFileName)
{
	return CTerrainImpl::LoadAttrMap(c_pszFileName);
}

bool CTerrain::isAttrOn(uint16_t wCoordX, uint16_t wCoordY, uint8_t byAttrFlag)
{
	if (wCoordX >= ATTRMAP_XSIZE || wCoordY >= ATTRMAP_YSIZE)
	{
		Tracef("CTerrain::isAttrOn Coordiante Error! Return false... Input Coord - X : %d, Y : %d ( Limit X : %d, Y : %d)", wCoordX,
			   wCoordY, ATTRMAP_XSIZE, ATTRMAP_YSIZE);
		return false;
	}

	uint8_t byMapAttr = m_abyAttrMap[wCoordY * ATTRMAP_XSIZE + wCoordX];

	if (byAttrFlag < 16)
		return (byMapAttr & byAttrFlag) ? true : false;
	else
	{
		if (byAttrFlag / 16 == byMapAttr / 16)
			return true;
		else
			return false;
	}
}

uint8_t CTerrain::GetAttr(uint16_t wCoordX, uint16_t wCoordY)
{
	if (wCoordX >= ATTRMAP_XSIZE || wCoordY >= ATTRMAP_YSIZE)
	{
		Tracef("CTerrain::GetAttr Coordiante Error! Return 0... Input Coord - X : %d, Y : %d ( Limit X : %d, Y : %d)", wCoordX, wCoordY,
			   ATTRMAP_XSIZE, ATTRMAP_YSIZE);
		return 0;
	}

	return m_abyAttrMap[wCoordY * ATTRMAP_XSIZE + wCoordX];
}

void CTerrain::GetWaterHeight(uint8_t byWaterNum, long * plWaterHeight)
{
	if (byWaterNum > m_byNumWater)
	{
		Tracef("CTerrain::GetWaterHeight WaterNum %d(Total Num %d) ERROR!", byWaterNum, m_byNumWater);
		return;
	}
	*plWaterHeight = m_lWaterHeight[byWaterNum];
}

bool CTerrain::GetWaterHeight(uint16_t wCoordX, uint16_t wCoordY, long * plWaterHeight)
{
	uint8_t byWaterNum = *(m_abyWaterMap + (wCoordY * WATERMAP_XSIZE) + wCoordX);
	if (byWaterNum > m_byNumWater)
	{
		Tracef("CTerrain::GetWaterHeight (X %d, Y %d) ERROR!", wCoordX, wCoordY, m_byNumWater);
		return false;
	}
	*plWaterHeight = m_lWaterHeight[byWaterNum] / 2;

	return true;
}

void CTerrain::RAW_DeallocateSplats(bool bBGLoading)
{
	for (uint32_t i = 1; i < GetTextureSet()->GetTextureCount(); ++i)
	{
		TTerainSplat & rSplat = m_TerrainSplatPatch.Splats[i];

		if (m_lpAlphaTexture[i])
		{
			ULONG ulRef;
			do
			{
				ulRef = m_lpAlphaTexture[i]->Release();
			} while (ulRef > 0);
		}

		rSplat.pd3dTexture = m_lpAlphaTexture[i] = nullptr;
	}

	m_TerrainSplatPatch = {};
}

void CTerrain::RAW_AllocateSplats(bool bBGLoading)
{
	RAW_DeallocateSplats(bBGLoading);
	uint32_t dwTexCount = GetTextureSet()->GetTextureCount();

	m_TerrainSplatPatch.m_bNeedsUpdate = true;

	for (uint32_t t = 0; t < dwTexCount; ++t)
		m_TerrainSplatPatch.Splats[t].NeedsUpdate = 1;

	RAW_CountTiles();

	// 	if ( WAIT_OBJECT_0 == LockDataWrite() )
	RAW_GenerateSplat(bBGLoading);
	// 	UnlockDataWrite();

	m_TerrainSplatPatch.m_bNeedsUpdate = false;
}

void CTerrain::RAW_CountTiles()
{
	for (long y = 0; y < TILEMAP_RAW_YSIZE; ++y)
	{
		long lPatchIndexY = std::min<long>(std::max<long>((y - 1) / PATCH_TILE_YSIZE, 0), PATCH_YCOUNT - 1);
		for (long x = 0; x < TILEMAP_RAW_XSIZE; ++x)
		{
			long lPatchIndexX = std::min<long>(std::max<long>((x - 1) / (PATCH_TILE_XSIZE), 0), PATCH_XCOUNT - 1);
			uint8_t tilenum = m_abyTileMap[y * TILEMAP_RAW_XSIZE + x];

			++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + lPatchIndexX][tilenum];

			if (0 == y % PATCH_TILE_YSIZE && 0 != y && (TILEMAP_RAW_YSIZE - 2) != y)
			{
				++m_TerrainSplatPatch
					  .PatchTileCount[std::min<int>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT + lPatchIndexX][tilenum];
				if (0 == x % PATCH_TILE_XSIZE && 0 != x && (TILEMAP_RAW_XSIZE - 2) != x)
				{
					++m_TerrainSplatPatch
						  .PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<int>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][tilenum];
					++m_TerrainSplatPatch.PatchTileCount[std::min<int>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT +
														 std::min<int>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][tilenum];
				}
				else if (1 == x % PATCH_TILE_XSIZE && (TILEMAP_RAW_XSIZE - 1) != x && 1 != x)
				{
					++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max(0l, lPatchIndexX - 1)][tilenum];
					++m_TerrainSplatPatch.PatchTileCount[std::min<int>(PATCH_YCOUNT - 1, lPatchIndexY + 1) * PATCH_XCOUNT +
														 std::max(0l, lPatchIndexX - 1)][tilenum];
				}
			}
			else if (1 == y % PATCH_TILE_YSIZE && (TILEMAP_RAW_YSIZE - 1) != y && 1 != y)
			{
				++m_TerrainSplatPatch.PatchTileCount[std::max(0l, lPatchIndexY - 1) * PATCH_XCOUNT + lPatchIndexX][tilenum];
				if (0 == x % PATCH_TILE_XSIZE && 0 != x && (TILEMAP_RAW_XSIZE - 2) != x)
				{
					++m_TerrainSplatPatch
						  .PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<int>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][tilenum];
					++m_TerrainSplatPatch.PatchTileCount[std::max(0l, lPatchIndexY - 1) * PATCH_XCOUNT +
														 std::min<int>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][tilenum];
				}
				else if (1 == x % PATCH_TILE_XSIZE && (TILEMAP_RAW_XSIZE - 1) != x && 1 != x)
				{
					++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max(0l, lPatchIndexX - 1)][tilenum];
					++m_TerrainSplatPatch
						  .PatchTileCount[std::max(0l, lPatchIndexY - 1) * PATCH_XCOUNT + std::max(0l, lPatchIndexX - 1)][tilenum];
				}
			}
			else
			{
				if (0 == x % PATCH_TILE_XSIZE && 0 != x && (TILEMAP_RAW_XSIZE - 2) != x)
					++m_TerrainSplatPatch
						  .PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::min<int>(PATCH_XCOUNT - 1, lPatchIndexX + 1)][tilenum];
				else if (1 == x % PATCH_TILE_XSIZE && (TILEMAP_RAW_XSIZE - 1) != x && 1 != x)
					++m_TerrainSplatPatch.PatchTileCount[lPatchIndexY * PATCH_XCOUNT + std::max(0l, lPatchIndexX - 1)][tilenum];
			}

			++m_TerrainSplatPatch.TileCount[tilenum];
		}
	}
}

void CTerrain::RAW_GenerateSplat(bool bBGLoading)
{
	if (!m_TerrainSplatPatch.m_bNeedsUpdate)
		return;

	m_TerrainSplatPatch.m_bNeedsUpdate = false;

	uint8_t abyAlphaMap[SPLATALPHA_RAW_XSIZE * SPLATALPHA_RAW_YSIZE];
	uint8_t * aptr;

	for (uint32_t i = 1; i < GetTextureSet()->GetTextureCount(); ++i)
	{
		TTerainSplat & rSplat = m_TerrainSplatPatch.Splats[i];

		if (rSplat.NeedsUpdate)
		{
			if (m_TerrainSplatPatch.TileCount[i] > 0)
			{
				if (rSplat.Active) // We already have an alpha map which needs to be updated
				{
					if (m_lpAlphaTexture[i])
					{
						ULONG ulRef;
						do
						{
							ulRef = m_lpAlphaTexture[i]->Release();
							if (ulRef > 0)
								TraceError(" CTerrain::RAW_GenerateSplat - TileCount > 0 : Alpha Texture Release(%d) ERROR", ulRef);
						} while (ulRef > 0);
					}

					rSplat.pd3dTexture = m_lpAlphaTexture[i] = nullptr;
				}

				rSplat.Active = 1;
				rSplat.NeedsUpdate = 0;

				aptr = abyAlphaMap;

				for (long y = 0; y < SPLATALPHA_RAW_YSIZE; ++y)
				{
					for (long x = 0; x < SPLATALPHA_RAW_XSIZE; ++x)
					{
						long lTileMapOffset = y * TILEMAP_RAW_XSIZE + x;

						uint8_t byTileNum = m_abyTileMap[lTileMapOffset];
						if (byTileNum == i)
							*aptr = 0xFF;
						else if (byTileNum > i)
						{
							uint8_t byTileTL, byTileTR, byTileBL, byTileBR, byTileT, byTileB, byTileL, byTileR;

							if (x > 0 && y > 0)
								byTileTL = m_abyTileMap[lTileMapOffset - TILEMAP_RAW_YSIZE - 1];
							else
								byTileTL = 0;
							if (x < (SPLATALPHA_RAW_XSIZE - 1) && y > 0)
								byTileTR = m_abyTileMap[lTileMapOffset - TILEMAP_RAW_YSIZE + 1];
							else
								byTileTR = 0;
							if (x > 0 && y < (SPLATALPHA_RAW_YSIZE - 1))
								byTileBL = m_abyTileMap[lTileMapOffset + TILEMAP_RAW_YSIZE - 1];
							else
								byTileBL = 0;
							if (x < (SPLATALPHA_RAW_XSIZE - 1) && y < (SPLATALPHA_RAW_YSIZE - 1))
								byTileBR = m_abyTileMap[lTileMapOffset + TILEMAP_RAW_YSIZE + 1];
							else
								byTileBR = 0;
							if (y > 0)
								byTileT = m_abyTileMap[lTileMapOffset - TILEMAP_RAW_YSIZE];
							else
								byTileT = 0;
							if (y < (SPLATALPHA_RAW_YSIZE - 1))
								byTileB = m_abyTileMap[lTileMapOffset + TILEMAP_RAW_YSIZE];
							else
								byTileB = 0;
							if (x > 0)
								byTileL = m_abyTileMap[lTileMapOffset - 1];
							else
								byTileL = 0;
							if (x < (SPLATALPHA_RAW_XSIZE - 1))
								byTileR = m_abyTileMap[lTileMapOffset + 1];
							else
								byTileR = 0;

							if (byTileTL == i || byTileTR == i || byTileBL == i || byTileBR == i || byTileT == i || byTileB == i ||
								byTileL == i || byTileR == i)
								*aptr = 0xFF;
							else
								*aptr = 0x00;
						}
						else
							*aptr = 0x00;

						++aptr;
					}
				}


				rSplat.pd3dTexture = AddTexture32(i, abyAlphaMap, SPLATALPHA_RAW_XSIZE, SPLATALPHA_RAW_YSIZE);
			}
			else
			{
				if (rSplat.Active)
				{
					if (m_lpAlphaTexture[i])
					{
						ULONG ulRef;
						do
						{
							ulRef = m_lpAlphaTexture[i]->Release();
							if (ulRef > 0)
								TraceError(" CTerrain::RAW_GenerateSplat - TileDount 0 : Alpha Texture Release(%d) ERROR", ulRef);
						} while (ulRef > 0);
					}

					rSplat.pd3dTexture = m_lpAlphaTexture[i] = nullptr;
				}
				rSplat.NeedsUpdate = 0;
				rSplat.Active = 0;
			}
		}
	}
}

LPDIRECT3DTEXTURE9 CTerrain::AddTexture32(uint8_t byImageNum, uint8_t * pbyImage, long lTextureWidth, long lTextureHeight)
{
	assert(nullptr == m_lpAlphaTexture[byImageNum]);

	if (m_lpAlphaTexture[byImageNum])
		m_lpAlphaTexture[byImageNum]->Release();

	m_lpAlphaTexture[byImageNum] = nullptr;

	HRESULT hr;
	D3DFORMAT format;

	if (ms_bSupportDXT)
		format = D3DFMT_A8R8G8B8;
	else
		format = D3DFMT_A4R4G4B4;


	bool bResizedAndSuccess = false;

	IDirect3DTexture9 * pkTex = nullptr;

	uint32_t uiNewWidth = 256;
	uint32_t uiNewHeight = 256;
	hr = ms_lpd3dDevice->CreateTexture(uiNewWidth, uiNewHeight, 5, 0, format, D3DPOOL_MANAGED, &pkTex, NULL);
	if (FAILED(hr))
	{
		TraceError("CTerrain::AddTexture32 - CreateTexture Error");
		return nullptr;
	}


	uint8_t abResizeImage[256 * 256];
	{
		uint8_t * pbDstPixel = abResizeImage;
		uint8_t * pbSrcPixel;
		uint8_t * abCurLine = pbyImage;
		for (uint32_t y = 0; y < 256; ++y, abCurLine += 258)
		{
			for (uint32_t x = 0; x < 256; ++x)
			{
				pbSrcPixel = abCurLine + x;
				*pbDstPixel++ = (((pbSrcPixel[0] + pbSrcPixel[1] + pbSrcPixel[2] + pbSrcPixel[258] + pbSrcPixel[260] + pbSrcPixel[258 * 2] +
								   pbSrcPixel[258 * 2 + 1] + pbSrcPixel[258 * 2 + 2]) >>
								  3) +
								 pbSrcPixel[259]) >>
					1;
			}
		}

		D3DLOCKED_RECT d3dlr;
		hr = pkTex->LockRect(0, &d3dlr, nullptr, 0);
		if (FAILED(hr))
		{
			pkTex->Release();
			return nullptr;
		}

		if (ms_bSupportDXT)
			PutImage32(abResizeImage, (uint8_t *) d3dlr.pBits, 256, d3dlr.Pitch, 256, 256, bResizedAndSuccess);
		else
			PutImage16(abResizeImage, (uint8_t *) d3dlr.pBits, 256, d3dlr.Pitch, 256, 256, bResizedAndSuccess);

		pkTex->UnlockRect(0);
	}

	uint8_t abResizeImage2[128 * 128];

	uint8_t * pbSrcBuffer = abResizeImage;
	uint8_t * pbDstBuffer = abResizeImage2;

	uint32_t uSrcSize = 256;

	for (uint32_t uMipMapLevel = 1; uMipMapLevel != pkTex->GetLevelCount(); ++uMipMapLevel)
	{
		uint32_t uDstSize = uSrcSize >> 1;

		uint8_t * pbDstPixel = pbDstBuffer;
		uint8_t * pbSrcPixel;
		uint8_t * abCurLine = pbSrcBuffer;
		for (uint32_t y = 0; y != uSrcSize; y += 2, abCurLine += uSrcSize * 2)
		{
			for (uint32_t x = 0; x != uSrcSize; x += 2)
			{
				pbSrcPixel = abCurLine + x;
				*pbDstPixel++ = (pbSrcPixel[0] + pbSrcPixel[1] + pbSrcPixel[uSrcSize + 0] + pbSrcPixel[uSrcSize + 1]) >> 2;
			}
		}

		D3DLOCKED_RECT d3dlr;

		hr = pkTex->LockRect(uMipMapLevel, &d3dlr, nullptr, 0);
		if (FAILED(hr))
			continue;

		if (ms_bSupportDXT)
			PutImage32(pbDstBuffer, (uint8_t *) d3dlr.pBits, uDstSize, d3dlr.Pitch, uDstSize, uDstSize, bResizedAndSuccess);
		else
			PutImage16(pbDstBuffer, (uint8_t *) d3dlr.pBits, uDstSize, d3dlr.Pitch, uDstSize, uDstSize, bResizedAndSuccess);

		hr = pkTex->UnlockRect(uMipMapLevel);

		std::swap(pbSrcBuffer, pbDstBuffer);
		uSrcSize = uDstSize;
	}

	m_lpAlphaTexture[byImageNum] = pkTex;

	return pkTex;
}

void CTerrain::PutImage32(const uint8_t * src, uint8_t * dst, long src_pitch, long dst_pitch, long texturewidth, long textureheight,
						  bool bResize)
{
	for (int y = 0; y < textureheight; ++y)
	{
		for (int x = 0; x < texturewidth; ++x)
		{
			uint32_t packed_pixel = src[x] << 24;
			*((uint32_t *) (dst + x * 4)) = packed_pixel;
		}

		dst += dst_pitch;
		src += src_pitch;
	}
}

void CTerrain::PutImage16(const uint8_t * src, uint8_t * dst, long src_pitch, long dst_pitch, long texturewidth, long textureheight,
						  bool bResize)
{
	for (int y = 0; y < textureheight; ++y)
	{
		for (int x = 0; x < texturewidth; ++x)
		{
			uint16_t packed_pixel = src[x] << 8;
			//& 연산 한번이 아깝다
			//uint16_t packed_pixel = (src[x]&0xF0) << 8;
			*((uint16_t *) (dst + x * 2)) = packed_pixel;
		}

		dst += dst_pitch;
		src += src_pitch;
	}
}

void CTerrain::SetCoordinate(uint16_t wCoordX, uint16_t wCoordY)
{
	m_wX = wCoordX;
	m_wY = wCoordY;
}

void CTerrain::CalculateTerrainPatch()
{
	for (uint8_t byPatchNumY = 0; byPatchNumY < PATCH_YCOUNT; ++byPatchNumY)
		for (uint8_t byPatchNumX = 0; byPatchNumX < PATCH_XCOUNT; ++byPatchNumX)
			_CalculateTerrainPatch(byPatchNumX, byPatchNumY);
}

CTerrainPatch * CTerrain::GetTerrainPatchPtr(uint8_t byPatchNumX, uint8_t byPatchNumY)
{
	if (byPatchNumX < 0 || byPatchNumX >= PATCH_XCOUNT || byPatchNumY < 0 || byPatchNumY >= PATCH_YCOUNT)
		return nullptr;

	return &m_TerrainPatchList[byPatchNumY * PATCH_XCOUNT + byPatchNumX];
}


void CTerrain::_CalculateTerrainPatch(uint8_t byPatchNumX, uint8_t byPatchNumY)
{
	if (!m_awRawHeightMap || !m_acNormalMap || !m_abyWaterMap)
		return;

	uint32_t dwPatchNum = byPatchNumY * PATCH_XCOUNT + byPatchNumX;

	CTerrainPatch & rkTerrainPatch = m_TerrainPatchList[dwPatchNum];
	if (!rkTerrainPatch.NeedUpdate())
		return;

	const float fOpaqueWaterDepth = m_pOwnerOutdoorMap->GetOpaqueWaterDepth();
	const float fOOOpaqueWaterDepth = 1.0f / fOpaqueWaterDepth;
	const float fTransparentWaterDepth = 0.8f * fOpaqueWaterDepth;

	rkTerrainPatch.Clear();

	HardwareTransformPatch_SSourceVertex akSrcTerrainVertex[CTerrainPatch::TERRAIN_VERTEX_COUNT];
	SWaterVertex akSrcWaterVertex[PATCH_XSIZE * PATCH_YSIZE * 6];

	uint32_t dwNormalWidth = CTerrainImpl::NORMALMAP_XSIZE * 3;
	uint32_t dwStartX = byPatchNumX * PATCH_XSIZE;
	uint32_t dwStartY = byPatchNumY * PATCH_YSIZE;

	uint16_t * wOrigRawHeightPtr = m_awRawHeightMap + ((dwStartY + 1) * HEIGHTMAP_RAW_XSIZE) + dwStartX + 1;
	char * chOrigNormalPtr = m_acNormalMap + (dwStartY * dwNormalWidth) + dwStartX * 3;
	uint8_t * byOrigWaterPtr = m_abyWaterMap + (dwStartY * WATERMAP_XSIZE) + dwStartX;

	float fX, fY, fOrigX, fOrigY;
	fOrigX = fX = (float) (m_wX * XSIZE * CELLSCALE) + (float) (dwStartX * CELLSCALE);
	fOrigY = fY = (float) (m_wY * YSIZE * CELLSCALE) + (float) (dwStartY * CELLSCALE);

	rkTerrainPatch.SetMinX(fX);
	rkTerrainPatch.SetMaxX(fX + (float) (PATCH_XSIZE * CELLSCALE));
	rkTerrainPatch.SetMinY(fY);
	rkTerrainPatch.SetMaxY(fY + (float) (PATCH_YSIZE * CELLSCALE));

	float fMinZ = 999999.0f;
	float fMaxZ = -999999.0f;
	uint16_t wNumPlainType = 0;
	uint16_t wNumHillType = 0;
	uint16_t wNumCliffType = 0;

	bool bWaterExist = false;

	SWaterVertex * lpWaterVertex = akSrcWaterVertex;
	uint32_t uWaterVertexCount = 0;

	HardwareTransformPatch_SSourceVertex * lpTerrainVertex = akSrcTerrainVertex;
	uint32_t uTerrainVertexCount = 0;

	D3DXVECTOR3 kNormal;
	D3DXVECTOR3 kPosition;
	for (uint32_t dwY = dwStartY; dwY <= dwStartY + PATCH_YSIZE; ++dwY)
	{
		uint16_t * pwRawHeight = wOrigRawHeightPtr;
		char * pchNormal = chOrigNormalPtr;
		uint8_t * pbyWater = byOrigWaterPtr;
		fX = fOrigX;

		for (uint32_t dwX = dwStartX; dwX <= dwStartX + PATCH_XSIZE; ++dwX)
		{
			uint16_t hgt = (*pwRawHeight++);

			kNormal.x = -(*pchNormal++) * 0.0078740f;
			kNormal.y = (*pchNormal++) * 0.0078740f;
			kNormal.z = (*pchNormal++) * 0.0078740f;

			kPosition.x = +fX;
			kPosition.y = -fY;
			kPosition.z = (float) (hgt) *m_fHeightScale;
			lpTerrainVertex->kPosition = kPosition;
			lpTerrainVertex->kNormal = kNormal;

			if (0.5f > kNormal.z) // 수평으로 부터 30도 이하 각으로  기울어져 있다. Cliff type으로 정의
				++wNumCliffType;
			else if (0.8660254f > kNormal.z) // 수평으로 부터 60도 이하 각으로  기울어져 있다. Hill type으로 정의
				++wNumHillType;
			else // 그 이상은 plain 타입
				++wNumPlainType;

			if (kPosition.z > fMaxZ)
				fMaxZ = kPosition.z;
			if (kPosition.z < fMinZ)
				fMinZ = kPosition.z;

			if (0 <= dwX && 0 <= dwY && XSIZE > dwX && YSIZE > dwY && (dwStartX + PATCH_XSIZE) != dwX && (dwStartY + PATCH_YSIZE) != dwY)
			{
				uint8_t byNumWater = (*pbyWater++);

				if (byNumWater != 0xFF)
				{
					long lWaterHeight = m_lWaterHeight[byNumWater];
					if (-1 != lWaterHeight)
					{
						auto fWaterTerrainHeightDifference0 = (float) (lWaterHeight - (long) hgt);
						if (fWaterTerrainHeightDifference0 >= fTransparentWaterDepth)
							fWaterTerrainHeightDifference0 = fTransparentWaterDepth;
						if (fWaterTerrainHeightDifference0 <= 0.0f)
							fWaterTerrainHeightDifference0 = 0.0f;

						auto fWaterTerrainHeightDifference1 =
							(float) (lWaterHeight - (long) (*(pwRawHeight + CTerrainImpl::HEIGHTMAP_RAW_XSIZE - 1)));
						if (fWaterTerrainHeightDifference1 >= fTransparentWaterDepth)
							fWaterTerrainHeightDifference1 = fTransparentWaterDepth;
						if (fWaterTerrainHeightDifference1 <= 0.0f)
							fWaterTerrainHeightDifference1 = 0.0f;

						auto fWaterTerrainHeightDifference2 = (float) (lWaterHeight - (long) (*(pwRawHeight)));
						if (fWaterTerrainHeightDifference2 >= fTransparentWaterDepth)
							fWaterTerrainHeightDifference2 = fTransparentWaterDepth;
						if (fWaterTerrainHeightDifference2 <= 0.0f)
							fWaterTerrainHeightDifference2 = 0.0f;

						auto fWaterTerrainHeightDifference3 =
							(float) (lWaterHeight - (long) (*(pwRawHeight + CTerrainImpl::HEIGHTMAP_RAW_XSIZE)));
						if (fWaterTerrainHeightDifference3 >= fTransparentWaterDepth)
							fWaterTerrainHeightDifference3 = fTransparentWaterDepth;
						if (fWaterTerrainHeightDifference3 <= 0.0f)
							fWaterTerrainHeightDifference3 = 0.0f;

						uint32_t dwAlpha0;
						uint32_t dwAlpha1;
						uint32_t dwAlpha2;
						uint32_t dwAlpha3;

						PR_FLOAT_TO_INT(fWaterTerrainHeightDifference0 * fOOOpaqueWaterDepth * 255.0f, dwAlpha0);
						PR_FLOAT_TO_INT(fWaterTerrainHeightDifference1 * fOOOpaqueWaterDepth * 255.0f, dwAlpha1);
						PR_FLOAT_TO_INT(fWaterTerrainHeightDifference2 * fOOOpaqueWaterDepth * 255.0f, dwAlpha2);
						PR_FLOAT_TO_INT(fWaterTerrainHeightDifference3 * fOOOpaqueWaterDepth * 255.0f, dwAlpha3);

						uint32_t dwAlphaKey = (dwAlpha0 << 24) | (dwAlpha1 << 16) | (dwAlpha2 << 8) | dwAlpha3;
						if (dwAlphaKey != 0)
						{
							assert(lpWaterVertex < akSrcWaterVertex + PATCH_XSIZE * PATCH_YSIZE * 6);
							lpWaterVertex->x = fX;
							lpWaterVertex->y = -fY;
							lpWaterVertex->z = (float) lWaterHeight * m_fHeightScale;
							lpWaterVertex->dwDiffuse = ((dwAlpha0 << 24) & 0xFF000000) | 0x000000FF; // 0x000F939B
							lpWaterVertex++;

							lpWaterVertex->x = fX;
							lpWaterVertex->y = -fY - float(CELLSCALE);
							lpWaterVertex->z = (float) lWaterHeight * m_fHeightScale;
							lpWaterVertex->dwDiffuse = ((dwAlpha1 << 24) & 0xFF000000) | 0x00FFFFFF;
							lpWaterVertex++;

							lpWaterVertex->x = fX + float(CELLSCALE);
							lpWaterVertex->y = -fY;
							lpWaterVertex->z = (float) lWaterHeight * m_fHeightScale;
							lpWaterVertex->dwDiffuse = ((dwAlpha2 << 24) & 0xFF000000) | 0x00FFFFFF;
							lpWaterVertex++;

							lpWaterVertex->x = fX + float(CELLSCALE);
							lpWaterVertex->y = -fY;
							lpWaterVertex->z = (float) lWaterHeight * m_fHeightScale;
							lpWaterVertex->dwDiffuse = ((dwAlpha2 << 24) & 0xFF000000) | 0x00FFFFFF;
							lpWaterVertex++;

							lpWaterVertex->x = fX;
							lpWaterVertex->y = -fY - float(CELLSCALE);
							lpWaterVertex->z = (float) lWaterHeight * m_fHeightScale;
							lpWaterVertex->dwDiffuse = ((dwAlpha1 << 24) & 0xFF000000) | 0x00FFFFFF;
							lpWaterVertex++;

							lpWaterVertex->x = fX + float(CELLSCALE);
							lpWaterVertex->y = -fY - float(CELLSCALE);
							lpWaterVertex->z = (float) lWaterHeight * m_fHeightScale;
							lpWaterVertex->dwDiffuse = ((dwAlpha3 << 24) & 0xFF0000FF) | 0x00FFFFFF;
							lpWaterVertex++;

							uWaterVertexCount += 6;
							bWaterExist = true;
						}
					}
				}
			}

			++lpTerrainVertex;
			++uTerrainVertexCount;
			fX += float(CELLSCALE);
		}

		wOrigRawHeightPtr += CTerrainImpl::HEIGHTMAP_RAW_XSIZE;
		chOrigNormalPtr += dwNormalWidth;
		byOrigWaterPtr += CTerrainImpl::XSIZE;
		fY += float(CELLSCALE);
	}

	if (wNumPlainType <= std::max(wNumHillType, wNumCliffType))
	{
		if (wNumCliffType <= wNumHillType)
			rkTerrainPatch.SetType(CTerrainPatch::PATCH_TYPE_HILL);
		else
			rkTerrainPatch.SetType(CTerrainPatch::PATCH_TYPE_CLIFF);
	}

	rkTerrainPatch.SetWaterExist(bWaterExist);

	rkTerrainPatch.SetMinZ(fMinZ);
	rkTerrainPatch.SetMaxZ(fMaxZ);

	assert((PATCH_XSIZE + 1) * (PATCH_YSIZE + 1) == uTerrainVertexCount);
	rkTerrainPatch.BuildTerrainVertexBuffer(akSrcTerrainVertex);

	if (bWaterExist)
		rkTerrainPatch.BuildWaterVertexBuffer(akSrcWaterVertex, uWaterVertexCount);

	rkTerrainPatch.NeedUpdate(false);
}

void CTerrain::AllocateMarkedSplats(uint8_t * pbyAlphaMap)
{
	TTerainSplat & rAttrSplat = m_MarkedSplatPatch.Splats[0];
	HRESULT hr;

	if (m_lpMarkedTexture)
	{
		ULONG ulRef;
		do
		{
			ulRef = m_lpMarkedTexture->Release();
		} while (ulRef > 0);
	}

	do
	{
		hr = ms_lpd3dDevice->CreateTexture(ATTRMAP_XSIZE, ATTRMAP_YSIZE, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_lpMarkedTexture, NULL);
	} while (FAILED(hr));

	D3DLOCKED_RECT d3dlr;
	do
	{
		hr = m_lpMarkedTexture->LockRect(0, &d3dlr, nullptr, 0);
	} while (FAILED(hr));

	PutImage32(pbyAlphaMap, (uint8_t *) d3dlr.pBits, ATTRMAP_XSIZE, d3dlr.Pitch, ATTRMAP_XSIZE, ATTRMAP_YSIZE);

	do
	{
		hr = m_lpMarkedTexture->UnlockRect(0);
	} while (FAILED(hr));

	rAttrSplat.pd3dTexture = m_lpMarkedTexture;
	m_bMarked = true;
}

void CTerrain::DeallocateMarkedSplats()
{
	TTerainSplat & rSplat = m_MarkedSplatPatch.Splats[0];
	if (m_lpMarkedTexture)
	{
		ULONG ulRef;
		do
		{
			ulRef = m_lpMarkedTexture->Release();
		} while (ulRef > 0);
	}

	rSplat.pd3dTexture = nullptr;
	m_lpMarkedTexture = nullptr;
	m_bMarked = FALSE;

	m_MarkedSplatPatch = {};
}
