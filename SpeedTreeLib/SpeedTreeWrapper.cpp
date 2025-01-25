#pragma warning(disable : 4786)

#include "StdAfx.h"
#include <cstdlib>
#include <cstdio>

#include "../eterBase/Debug.h"
#include "../eterBase/Timer.h"
#include "../eterBase/Filename.h"
#include "../eterLib/ResourceManager.h"
#include "../eterLib/Camera.h"
#include "../eterLib/StateManager.h"
#include "SpeedTreeConfig.h"
#include "CSpeedTreeDirectX.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"

unsigned int                    CSpeedTreeWrapper::m_unNumWrappersActive = 0;
bool							CSpeedTreeWrapper::ms_bSelfShadowOn = true;
LPDIRECT3DVERTEXSHADER9			CSpeedTreeWrapper::ms_lpBranchVertexShader = NULL;
LPDIRECT3DVERTEXSHADER9			CSpeedTreeWrapper::ms_lpLeafVertexShader = NULL;
#define AGBR2ARGB(dwColor) (dwColor & 0xff00ff00) + ((dwColor & 0x00ff0000) >> 16) + ((dwColor & 0x000000ff) << 16)
CSpeedTreeWrapper::CSpeedTreeWrapper() :
    m_pSpeedTree(new CSpeedTreeRT),
    m_bIsInstance(false),
    m_pInstanceOf(NULL),
    m_pGeometryCache(NULL),
    m_usNumLeafLods(0),
    m_unNumFrondLods(0),
    m_pBranchIndexCounts(NULL),
    m_pBranchIndexBuffer(NULL),
    m_pBranchVertexBuffer(NULL),
    m_pFrondIndexCounts(NULL),
    m_pFrondIndexBuffers(NULL),
    m_pFrondVertexBuffer(NULL),
    m_pLeafVertexBuffer(NULL),
    m_pLeavesUpdatedByCpu(NULL),
    m_unBranchVertexCount(0),
    m_unFrondVertexCount(0),
    m_pTextureInfo(NULL)
{
    m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    m_unNumWrappersActive++;
}

void CSpeedTreeWrapper::OnRenderPCBlocker()
{
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

    const uint32_t dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
    const uint32_t dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
    const uint32_t dwAlphaBlendEnable = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
    STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
    STATEMANAGER.SetVertexShader(ms_lpBranchVertexShader);
    {
        LPDIRECT3DTEXTURE9 lpd3dTexture;
        if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
            STATEMANAGER.SetTexture(0, lpd3dTexture);

        if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
        {
            STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));
            STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
            RenderBranches();
        }
    }

    STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    {
        if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0 &&
            m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel > -1 &&
            m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
        {
            if (!m_CompositeImageInstance.IsEmpty())
                STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

            STATEMANAGER.SetStreamSource(0, m_pFrondVertexBuffer, sizeof(SFVFBranchVertex));
            STATEMANAGER.SetIndices(m_pFrondIndexBuffers[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel], 0);
            RenderFronds();
        }
    }
    {
        STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
        STATEMANAGER.SetVertexShader(ms_lpLeafVertexShader);

        if (!m_CompositeImageInstance.IsEmpty())
            STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

        RenderLeaves();
        EndLeafForTreeType();
    }

    STATEMANAGER.SetVertexShader(NULL);
    STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
    RenderBillboards();

    STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
    STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlendEnable);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
}

void CSpeedTreeWrapper::OnRender()
{
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
    STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
    STATEMANAGER.SetVertexShader(ms_lpBranchVertexShader);

    SetupBranchForTreeType();
    RenderBranches();

    STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
    STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    SetupFrondForTreeType();
    RenderFronds();

    STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
    STATEMANAGER.SetVertexShader(ms_lpLeafVertexShader);

    SetupLeafForTreeType();
    RenderLeaves();
    EndLeafForTreeType();

    STATEMANAGER.SetVertexShader(NULL);
    STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
    RenderBillboards();

    STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
    STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
    STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}

uint32_t CSpeedTreeWrapper::GetCollisionObjectCount()
{
    assert(m_pSpeedTree);
    return m_pSpeedTree->GetCollisionObjectCount();
}

void CSpeedTreeWrapper::GetCollisionObject(uint32_t nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
    assert(m_pSpeedTree);
    m_pSpeedTree->GetCollisionObject(nIndex, eType, pPosition, pDimensions);
}

void CSpeedTreeWrapper::OnUpdateCollisionData(const CStaticCollisionDataVector*)
{
    D3DXMATRIX mat;
    D3DXMatrixTranslation(&mat, m_afPos[0], m_afPos[1], m_afPos[2]);
    for (uint32_t i = 0; i < GetCollisionObjectCount(); ++i)
    {
        CSpeedTreeRT::ECollisionObjectType ObjectType;
        CStaticCollisionData CollisionData;

        GetCollisionObject(i, ObjectType, (float*)&CollisionData.v3Position, CollisionData.fDimensions);

        if (ObjectType == CSpeedTreeRT::CO_BOX)
            continue;

        switch (ObjectType)
        {
        case CSpeedTreeRT::CO_SPHERE:
            CollisionData.dwType = COLLISION_TYPE_SPHERE;
            CollisionData.fDimensions[0] = CollisionData.fDimensions[0];
            break;

        case CSpeedTreeRT::CO_CYLINDER:
            CollisionData.dwType = COLLISION_TYPE_CYLINDER;
            CollisionData.fDimensions[0] = CollisionData.fDimensions[0];
            CollisionData.fDimensions[1] = CollisionData.fDimensions[1];
            break;
        }
        AddCollision(&CollisionData, &mat);
    }
}

bool CSpeedTreeWrapper::GetBoundingSphere(D3DXVECTOR3& v3Center, float& fRadius)
{
    float fX, fY, fZ;
    fX = m_afBoundingBox[3] - m_afBoundingBox[0];
    fY = m_afBoundingBox[4] - m_afBoundingBox[1];
    fZ = m_afBoundingBox[5] - m_afBoundingBox[2];
    v3Center.x = 0.0f;
    v3Center.y = 0.0f;
    v3Center.z = fZ * 0.5f;
    fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f * 0.9f;
    const D3DXVECTOR3 vec = m_pSpeedTree->GetTreePosition();
    v3Center += vec;
    return true;
}

void CSpeedTreeWrapper::CalculateBBox()
{
    float fX, fY, fZ;

    fX = m_afBoundingBox[3] - m_afBoundingBox[0];
    fY = m_afBoundingBox[4] - m_afBoundingBox[1];
    fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

    m_v3BBoxMin.x = -fX / 2.0f;
    m_v3BBoxMin.y = -fY / 2.0f;
    m_v3BBoxMin.z = 0.0f;
    m_v3BBoxMax.x = fX / 2.0f;
    m_v3BBoxMax.y = fY / 2.0f;
    m_v3BBoxMax.z = fZ;

    m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
    m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
    m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
    m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
    m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

    const D3DXMATRIX& c_rmatTransform = GetTransform();

    for (uint32_t i = 0; i < 8; ++i)
    {
        D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
        if (0 == i)
        {
            m_v3TBBoxMin.x = m_v4TBBox[i].x;
            m_v3TBBoxMin.y = m_v4TBBox[i].y;
            m_v3TBBoxMin.z = m_v4TBBox[i].z;
            m_v3TBBoxMax.x = m_v4TBBox[i].x;
            m_v3TBBoxMax.y = m_v4TBBox[i].y;
            m_v3TBBoxMax.z = m_v4TBBox[i].z;
        }
        else
        {
            if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
                m_v3TBBoxMin.x = m_v4TBBox[i].x;
            if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
                m_v3TBBoxMax.x = m_v4TBBox[i].x;
            if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
                m_v3TBBoxMin.y = m_v4TBBox[i].y;
            if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
                m_v3TBBoxMax.y = m_v4TBBox[i].y;
            if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
                m_v3TBBoxMin.z = m_v4TBBox[i].z;
            if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
                m_v3TBBoxMax.z = m_v4TBBox[i].z;
        }
    }
}

CSpeedTreeWrapper::~CSpeedTreeWrapper()
{
    if (!m_bIsInstance)
    {
        if (m_unBranchVertexCount > 0)
        {
            SAFE_RELEASE(m_pBranchVertexBuffer);
            SAFE_RELEASE(m_pBranchIndexBuffer);
            SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
        }

        if (m_unFrondVertexCount > 0)
        {
            SAFE_RELEASE(m_pFrondVertexBuffer);
            for (unsigned int i = 0; i < m_unNumFrondLods; ++i)
                if (m_pFrondIndexCounts[i] > 0)
                    SAFE_RELEASE(m_pFrondIndexBuffers[i]);
            SAFE_DELETE_ARRAY(m_pFrondIndexBuffers);
            SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
        }

        for (unsigned short i = 0; i < m_usNumLeafLods; ++i)
        {
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);

            if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
                SAFE_RELEASE(m_pLeafVertexBuffer[i]);
        }

        SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);
        SAFE_DELETE_ARRAY(m_pLeafVertexBuffer);

        SAFE_DELETE(m_pTextureInfo);
        SAFE_DELETE(m_pGeometryCache);
    }
    SAFE_DELETE(m_pSpeedTree);

    --m_unNumWrappersActive;


    Clear();
}

bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, const uint8_t* c_pbBlock, unsigned int uiBlockSize, uint32_t nSeed, float fSize, float fSizeVariance)
{
    bool bSuccess = false;

#ifdef WRAPPER_FLIP_T_TEXCOORD
    m_pSpeedTree->SetTextureFlip(true);
#endif

    if (!m_pSpeedTree->LoadTree(c_pbBlock, uiBlockSize))
    {
        if (!m_pSpeedTree->LoadTree(pszSptFile))
        {
            TraceError("SpeedTreeRT Error: %s", CSpeedTreeRT::GetCurrentError());
            return false;
        }
    }

    m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
    m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
    m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);

#ifdef WRAPPER_USE_NO_WIND
    m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
    m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
    m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
#endif

    m_pSpeedTree->SetNumLeafRockingGroups(1);

    // override the size, if necessary
    if (fSize >= 0.0f && fSizeVariance >= 0.0f)
        m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

    // generate tree geometry
    if (m_pSpeedTree->Compute(NULL, nSeed))
    {
        // get the dimensions
        m_pSpeedTree->GetBoundingBox(m_afBoundingBox);

        // make the leaves rock in the wind
        m_pSpeedTree->SetLeafRockingState(true);

        // billboard setup
        CSpeedTreeRT::SetDropToBillboard(true);

        // query & set materials
        m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial());
        m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial());
        m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial());

        // adjust lod distances
        const float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];

        // query textures
        m_pTextureInfo = new CSpeedTreeRT::STextures;
        m_pSpeedTree->GetTextures(*m_pTextureInfo);

        // load branch textures
        auto vs = std::string(pszSptFile);
        LoadTexture((CFileNameHelper::GetPath(vs) + CFileNameHelper::NoExtension(std::string(m_pTextureInfo->m_pBranchTextureFilename)) + ".dds").c_str(), m_BranchImageInstance);

        if (m_pTextureInfo->m_pSelfShadowFilename != NULL)
            LoadTexture((CFileNameHelper::GetPath(vs) + CFileNameHelper::NoExtension(std::string(m_pTextureInfo->m_pSelfShadowFilename)) + ".dds").c_str(), m_ShadowImageInstance);

        if (m_pTextureInfo->m_pCompositeFilename)
            LoadTexture((CFileNameHelper::GetPath(vs) + CFileNameHelper::NoExtension(std::string(m_pTextureInfo->m_pCompositeFilename)) + ".dds").c_str(), m_CompositeImageInstance);
  
        // setup the index and vertex buffers
        SetupBuffers();

        // everything appeared to go well
        bSuccess = true;
    }
    else // tree failed to compute
        fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError());

    return bSuccess;
}

void CSpeedTreeWrapper::SetupBuffers(void)
{
    if (m_pGeometryCache == NULL)
        m_pGeometryCache = new CSpeedTreeRT::SGeometry;
    m_pSpeedTree->GetGeometry(*m_pGeometryCache);
    SetupBranchBuffers();
    SetupFrondBuffers();
    SetupLeafBuffers();
}

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
    CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
    m_unBranchVertexCount = pBranches->m_usVertexCount;

    if (m_unBranchVertexCount > 1)
    {
        SFVFBranchVertex* pVertexBuffer = NULL;
        ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pBranchVertexBuffer, NULL);
        m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        for (unsigned int i = 0; i < m_unBranchVertexCount; ++i)
        {
            memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));
            pVertexBuffer->m_dwDiffuseColor = AGBR2ARGB(pBranches->m_pColors[i]);
            pVertexBuffer->m_fShadowCoords[0] = pBranches->m_pTexCoords1[i * 2];
            pVertexBuffer->m_fShadowCoords[1] = pBranches->m_pTexCoords1[i * 2 + 1];
            pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
            pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];
            ++pVertexBuffer;
        }
        m_pBranchVertexBuffer->Unlock();

        const unsigned int unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels();
        m_pBranchIndexCounts = new unsigned short[unNumLodLevels];
        for (unsigned int i = 0; i < unNumLodLevels; ++i)
        {
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, (short)i);
            if (pBranches->m_usNumStrips > 0)
                m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
            else
                m_pBranchIndexCounts[i] = 0;
        }
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);
        ms_lpd3dDevice->CreateIndexBuffer(m_pBranchIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pBranchIndexBuffer, NULL);
        unsigned short* pIndexBuffer = NULL;
        m_pBranchIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
        memcpy(pIndexBuffer, pBranches->m_pStrips[0], pBranches->m_pStripLengths[0] * sizeof(unsigned short));
        m_pBranchIndexBuffer->Unlock();
    }
}

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
    CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
    m_unFrondVertexCount = pFronds->m_usVertexCount;

    if (m_unFrondVertexCount > 1)
    {
        SFVFBranchVertex* pVertexBuffer = NULL;
        ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pFrondVertexBuffer, NULL);
        m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);

        for (unsigned short i = 0; i < m_unFrondVertexCount; ++i)
        {
            memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));
            pVertexBuffer->m_dwDiffuseColor = AGBR2ARGB(pFronds->m_pColors[i]);
            pVertexBuffer->m_fShadowCoords[0] = pFronds->m_pTexCoords1[i * 2];
            pVertexBuffer->m_fShadowCoords[1] = pFronds->m_pTexCoords1[i * 2 + 1];
            pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
            pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];
            ++pVertexBuffer;
        }
        m_pFrondVertexBuffer->Unlock();
        m_unNumFrondLods = m_pSpeedTree->GetNumFrondLodLevels();
        m_pFrondIndexCounts = new unsigned short[m_unNumFrondLods];
        m_pFrondIndexBuffers = new LPDIRECT3DINDEXBUFFER9[m_unNumFrondLods];

        for (unsigned short i = 0; i < m_unNumFrondLods; ++i)
        {
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, i);
            if (pFronds->m_usNumStrips > 0)
                m_pFrondIndexCounts[i] = pFronds->m_pStripLengths[0];
            else
                m_pFrondIndexCounts[i] = 0;

            if (m_pFrondIndexCounts[i] > 0)
            {
                ms_lpd3dDevice->CreateIndexBuffer(m_pFrondIndexCounts[i] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pFrondIndexBuffers[i], NULL);
                unsigned short* pIndexBuffer = NULL;
                m_pFrondIndexBuffers[i]->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
                memcpy(pIndexBuffer, pFronds->m_pStrips[0], m_pFrondIndexCounts[i] * sizeof(unsigned short));
                m_pFrondIndexBuffers[i]->Unlock();
            }
        }
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
    }
}

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
    const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };
    m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels();
    m_pLeafVertexBuffer = new LPDIRECT3DVERTEXBUFFER9[m_usNumLeafLods];
    m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods];

    for (unsigned int unLod = 0; unLod < m_usNumLeafLods; ++unLod)
    {
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, unLod);
        m_pLeavesUpdatedByCpu[unLod] = false;
        const unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;
        if (usLeafCount < 1)
            continue;

        SFVFLeafVertex* pVertexBuffer = NULL;
        ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_SYSTEMMEM, &m_pLeafVertexBuffer[unLod], NULL);
        m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
        SFVFLeafVertex* pVertex = pVertexBuffer;
        for (unsigned int unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
        {
            const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
            for (unsigned int unVert = 0; unVert < 6; ++unVert)
            {
                memcpy(pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));
                pVertex->m_dwDiffuseColor = AGBR2ARGB(pLeaf->m_pColors[unLeaf]);
                memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));
                ++pVertex;
            }

        }
        m_pLeafVertexBuffer[unLod]->Unlock();
    }
}

void CSpeedTreeWrapper::Advance(void){}

CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void)
{
    CSpeedTreeWrapper* pInstance = new CSpeedTreeWrapper;
    pInstance->m_bIsInstance = true;
    pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance();
    if (pInstance->m_pSpeedTree)
    {
        pInstance->m_cBranchMaterial = m_cBranchMaterial;
        pInstance->m_cLeafMaterial = m_cLeafMaterial;
        pInstance->m_cFrondMaterial = m_cFrondMaterial;
        pInstance->m_CompositeImageInstance.SetImagePointer(m_CompositeImageInstance.GetGraphicImagePointer());
        pInstance->m_BranchImageInstance.SetImagePointer(m_BranchImageInstance.GetGraphicImagePointer());
        if (!m_ShadowImageInstance.IsEmpty())
            pInstance->m_ShadowImageInstance.SetImagePointer(m_ShadowImageInstance.GetGraphicImagePointer());
        pInstance->m_pTextureInfo = m_pTextureInfo;
        pInstance->m_pGeometryCache = m_pGeometryCache;
        pInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
        pInstance->m_pBranchIndexCounts = m_pBranchIndexCounts;
        pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
        pInstance->m_unBranchVertexCount = m_unBranchVertexCount;
        pInstance->m_pFrondIndexBuffers = m_pFrondIndexBuffers;
        pInstance->m_unNumFrondLods = m_unNumFrondLods;
        pInstance->m_pFrondIndexCounts = m_pFrondIndexCounts;
        pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
        pInstance->m_unFrondVertexCount = m_unFrondVertexCount;
        pInstance->m_pLeafVertexBuffer = m_pLeafVertexBuffer;
        pInstance->m_usNumLeafLods = m_usNumLeafLods;
        pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;
        memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
        memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
        pInstance->m_pInstanceOf = this;
        m_vInstances.push_back(pInstance);
    }
    else
    {
        fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError());
        delete pInstance;
        pInstance = NULL;
    }

    return pInstance;
}

CSpeedTreeWrapper** CSpeedTreeWrapper::GetInstances(unsigned int& nCount)
{
    nCount = m_vInstances.size();
    return nCount ? &(m_vInstances[0]) : NULL;
}

void CSpeedTreeWrapper::DeleteInstance(CSpeedTreeWrapper* pInstance)
{
    std::vector<CSpeedTreeWrapper*>::iterator itor = m_vInstances.begin();

    while (itor != m_vInstances.end())
    {
        if (*itor == pInstance)
        {
            delete pInstance;
            itor = m_vInstances.erase(itor);
        }
        else
            ++itor;
    }
}

void CSpeedTreeWrapper::SetupBranchForTreeType(void) const
{
    LPDIRECT3DTEXTURE9 lpd3dTexture;
    if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
        STATEMANAGER.SetTexture(0, lpd3dTexture);

    if (ms_bSelfShadowOn && (lpd3dTexture = m_ShadowImageInstance.GetTextureReference().GetD3DTexture()))
        STATEMANAGER.SetTexture(1, lpd3dTexture);
    else
        STATEMANAGER.SetTexture(1, NULL);

    if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
    {
        STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));
        STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
    }
}

void CSpeedTreeWrapper::RenderBranches(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

    if (m_pGeometryCache->m_fBranchAlphaTestValue > 0.0f)
    {
        PositionTree();
        STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, uint32_t(m_pGeometryCache->m_fBranchAlphaTestValue));
        if (m_pBranchIndexCounts &&
            m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel > -1 &&
            m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
        {
            ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
            STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
        }
    }
}

void CSpeedTreeWrapper::SetupFrondForTreeType(void) const
{
    LPDIRECT3DTEXTURE9 lpd3dTexture;

    if ((lpd3dTexture = m_CompositeImageInstance.GetTextureReference().GetD3DTexture()))
        STATEMANAGER.SetTexture(0, lpd3dTexture);

    if ((lpd3dTexture = m_ShadowImageInstance.GetTextureReference().GetD3DTexture()))
        STATEMANAGER.SetTexture(1, lpd3dTexture);

    if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
    {
        STATEMANAGER.SetStreamSource(0, m_pFrondVertexBuffer, sizeof(SFVFBranchVertex));
    }
}

void CSpeedTreeWrapper::RenderFronds(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

    if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
    {
        PositionTree();
        STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, uint32_t(m_pGeometryCache->m_fFrondAlphaTestValue));
        if (m_pFrondIndexCounts &&
            m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel > -1 &&
            m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
        {
            STATEMANAGER.SetIndices(m_pFrondIndexBuffers[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel], 0);

            ms_faceCount += m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;
            STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2);
        }
    }
}

void CSpeedTreeWrapper::SetupLeafForTreeType(void) const
{
    if (!m_CompositeImageInstance.IsEmpty())
        STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
    STATEMANAGER.SetTexture(1, NULL);
}

void CSpeedTreeWrapper::RenderLeaves(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);

#if !defined WRAPPER_USE_NO_WIND || defined WRAPPER_USE_CPU_LEAF_PLACEMENT
    for (unsigned int i = 0; i < 2; ++i)
    {
        const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (i == 0) ? &m_pGeometryCache->m_sLeaves0 : &m_pGeometryCache->m_sLeaves1;
        int unLod = pLeaf->m_nDiscreteLodLevel;

        if (pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
        {
            SFVFLeafVertex* pVertex = NULL;
            m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertex), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
            for (unsigned int unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
            {
                const D3DXVECTOR3 vecCenter(&(pLeaf->m_pCenterCoords[unLeaf * 3]));
                const D3DXVECTOR3 vec0(&pLeaf->m_pLeafMapCoords[unLeaf][0]);
                const D3DXVECTOR3 vec1(&pLeaf->m_pLeafMapCoords[unLeaf][4]);
                const D3DXVECTOR3 vec2(&pLeaf->m_pLeafMapCoords[unLeaf][8]);
                const D3DXVECTOR3 vec3(&pLeaf->m_pLeafMapCoords[unLeaf][12]);

                (pVertex++)->m_vPosition = vecCenter + vec0;
                (pVertex++)->m_vPosition = vecCenter + vec1;
                (pVertex++)->m_vPosition = vecCenter + vec2;
                (pVertex++)->m_vPosition = vecCenter + vec0;
                (pVertex++)->m_vPosition = vecCenter + vec2;
                (pVertex++)->m_vPosition = vecCenter + vec3;
            }
            m_pLeafVertexBuffer[unLod]->Unlock();
        }
#endif
    }

    PositionTree();
    for (unsigned int unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
    {
        const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (unLeafLevel == 0) ?
            &m_pGeometryCache->m_sLeaves0 : pLeaf = &m_pGeometryCache->m_sLeaves1;
    
        int unLod = pLeaf->m_nDiscreteLodLevel;
        if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
        {
            STATEMANAGER.SetStreamSource(0, m_pLeafVertexBuffer[unLod], sizeof(SFVFLeafVertex));
            STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, uint32_t(pLeaf->m_fAlphaTestValue));
    
            ms_faceCount += pLeaf->m_usLeafCount * 2;
            STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, pLeaf->m_usLeafCount * 2);
        }
    }
}

void CSpeedTreeWrapper::EndLeafForTreeType(void){}

void CSpeedTreeWrapper::RenderBillboards(void) const
{
#ifdef WRAPPER_BILLBOARD_MODE
    PositionTree();
    struct SBillboardVertex
    {
        float fX, fY, fZ;
        uint32_t dColor;
        float fU, fV;
    };

    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);

    if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
    {
        const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
        const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
        SBillboardVertex sVertex[4] =
        {
            { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
            { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
            { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
            { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
        };
        STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, uint32_t(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));

        ms_faceCount += 2;
        STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
    }

    if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
    {
        const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
        const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
        SBillboardVertex sVertex[4] =
        {
            { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
            { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
            { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
            { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
        };
        STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, uint32_t(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));

        ms_faceCount += 2;
        STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
    }
#endif
}

void CSpeedTreeWrapper::CleanUpMemory(void)
{
    if (!m_bIsInstance)
        m_pSpeedTree->DeleteTransientData();
}

void CSpeedTreeWrapper::PositionTree(void) const
{
    D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition();
    D3DXMATRIX matTranslation;
    D3DXMatrixIdentity(&matTranslation);
    D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);
    STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);
    D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
    STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}

bool CSpeedTreeWrapper::LoadTexture(const char* pFilename, CGraphicImageInstance& rImage)
{
    CResource* pResource = CResourceManager::Instance().GetResourcePointer(pFilename);
    rImage.SetImagePointer(static_cast<CGraphicImage*>(pResource));

    if (rImage.IsEmpty())
        return false;

    return true;
}

void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
    const float afUsefulConstants[] =
    {
        m_pSpeedTree->GetLeafLightingAdjustment(), 0.0f, 0.0f, 0.0f
    };
    STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

    const float afMaterial[] =
    {
        pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
        pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
    };
    STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Material, afMaterial, 2);
}

void CSpeedTreeWrapper::SetPosition(float x, float y, float z)
{
    m_afPos[0] = x;
    m_afPos[1] = y;
    m_afPos[2] = z;

    m_pSpeedTree->SetTreePosition(x, y, z);
    CGraphicObjectInstance::SetPosition(x, y, z);
}
