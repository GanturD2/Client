/******************************************************************************

  Copyright (C) 1999, 2000 NVIDIA Corporation

  This file is provided without support, instruction, or implied warranty of any
  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
  not liable under any circumstances for any damages or loss whatsoever arising
  from the use or inability to use this file or items derived from it.

    Comments:

      A simple class to manage rendering state.  Created as a singleton.
	  Create it as a static global, or with new.  It doesn't matter as long as it is created
	  before you use the CStateManager::GetSingleton() API to get a reference to it.

	  Call it with STATEMANAGER.SetRenderState(...)
	  Call it with STATEMANAGER.SetTextureStageState(...), etc.

	  Call the 'Save' versions of the function if you want to deviate from the current state.
	  Call the 'Restore' version to retrieve the last Save.

	  There are two levels of caching:
	  - All Sets/Saves/Restores are tracked for redundancy.  This reduces the size of the batch to
	  be flushed
	  - The flush function is called before rendering, and only copies state that is
	  different from the current chip state.

  If you get an assert it is probably because an API call failed.

  See NVLink for a good example of how this class is used.

  Don't be afraid of the vector being used to track the flush batch.  It will grow as big as
  it needs to be and then stop, so it shouldn't be reallocated.

  The state manager holds a reference to the d3d device.

  - cmaughan@nvidia.com

******************************************************************************/

#ifndef __CSTATEMANAGER_H
#define __CSTATEMANAGER_H

#include <d3d9/d3d9.h>
#include <d3d9/d3dx9.h>

#include <vector>

#include "../eterBase/Singleton.h"

#define CHECK_D3DAPI(a)   \
	{                     \
		HRESULT hr = (a); \
                          \
		if (hr != S_OK)   \
			assert(!#a);  \
	}

static const uint32_t STATEMANAGER_MAX_RENDERSTATES = 256;
static const uint32_t STATEMANAGER_MAX_TEXTURESTATES = 128;
static const uint32_t STATEMANAGER_MAX_SAMPLERSTATES = 128;
static const uint32_t STATEMANAGER_MAX_STAGES = 8;
static const uint32_t STATEMANAGER_MAX_VCONSTANTS = 96;
static const uint32_t STATEMANAGER_MAX_PCONSTANTS = 8;
static const uint32_t STATEMANAGER_MAX_TRANSFORMSTATES = 300; // World1 lives way up there...
static const uint32_t STATEMANAGER_MAX_STREAMS = 16;

class CStreamData
{
public:
	CStreamData(LPDIRECT3DVERTEXBUFFER9 pStreamData = nullptr, uint32_t Stride = 0) : m_lpStreamData(pStreamData), m_Stride(Stride) {}

	bool operator==(const CStreamData & rhs) const { return ((m_lpStreamData == rhs.m_lpStreamData) && (m_Stride == rhs.m_Stride)); }

	LPDIRECT3DVERTEXBUFFER9 m_lpStreamData;
	uint32_t m_Stride;
};

class CIndexData
{
public:
	CIndexData(LPDIRECT3DINDEXBUFFER9 pIndexData = nullptr, uint32_t BaseVertexIndex = 0)
		: m_lpIndexData(pIndexData), m_BaseVertexIndex(BaseVertexIndex)
	{
	}

	bool operator==(const CIndexData & rhs) const
	{
		return ((m_lpIndexData == rhs.m_lpIndexData) && (m_BaseVertexIndex == rhs.m_BaseVertexIndex));
	}

	LPDIRECT3DINDEXBUFFER9 m_lpIndexData;
	uint32_t m_BaseVertexIndex;
};

// State types managed by the class
typedef enum eStateType
{
	STATE_MATERIAL = 0,
	STATE_RENDER,
	STATE_TEXTURE,
	STATE_TEXTURESTAGE,
	STATE_VSHADER,
	STATE_PSHADER,
	STATE_TRANSFORM,
	STATE_VCONSTANT,
	STATE_PCONSTANT,
	STATE_STREAM,
	STATE_INDEX
} eStateType;

class CStateID
{
public:
	CStateID(eStateType Type, uint32_t dwValue0 = 0, uint32_t dwValue1 = 0) : m_Type(Type), m_dwValue0(dwValue0), m_dwValue1(dwValue1) {}

	CStateID(eStateType Type, uint32_t dwStage, D3DTEXTURESTAGESTATETYPE StageType)
		: m_Type(Type), m_dwStage(dwStage), m_TextureStageStateType(StageType)
	{
	}

	CStateID(eStateType Type, D3DRENDERSTATETYPE RenderType) : m_Type(Type), m_RenderStateType(RenderType) {}

	eStateType m_Type;

	union {
		uint32_t m_dwValue0;
		uint32_t m_dwStage;
		D3DRENDERSTATETYPE m_RenderStateType;
		D3DTRANSFORMSTATETYPE m_TransformStateType;
	};

	union {
		uint32_t m_dwValue1;
		D3DTEXTURESTAGESTATETYPE m_TextureStageStateType;
	};
};

typedef std::vector<CStateID> TStateID;

class CStateManagerState
{
public:
	CStateManagerState() {}

	void ResetState()
	{
		for (auto i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
			m_RenderStates[i] = 0x7FFFFFFF;

		for (auto i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
			for (auto y = 0; y < STATEMANAGER_MAX_TEXTURESTATES; ++y)
				m_TextureStates[i][y] = 0x7FFFFFFF;
		for (auto i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
			for (auto y = 0; y < STATEMANAGER_MAX_SAMPLERSTATES; ++y)
				m_SamplerStates[i][y] = 0x7FFFFFFF;

		for (auto i = 0; i < STATEMANAGER_MAX_STREAMS; i++)
			m_StreamData[i] = CStreamData();

		m_IndexData = CIndexData();

		for (auto i = 0; i < STATEMANAGER_MAX_STAGES; i++)
			m_Textures[i] = nullptr;

		// Matrices and constants are not cached, just restored.  It's silly to check all the
		// data elements (by which time the driver could have been sent them).
		for (auto i = 0; i < STATEMANAGER_MAX_TRANSFORMSTATES; i++)
			D3DXMatrixIdentity(&m_Matrices[i]);

		for (auto i = 0; i < STATEMANAGER_MAX_VCONSTANTS; i++)
			m_VertexShaderConstants[i] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

		for (auto i = 0; i < STATEMANAGER_MAX_PCONSTANTS; i++)
			m_PixelShaderConstants[i] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

		m_dwFVF = D3DFVF_XYZ;
		m_dwVertexShader = NULL;
		m_dwPixelShader = NULL;

		ZeroMemory(&m_Matrices, sizeof(D3DXMATRIX) * STATEMANAGER_MAX_TRANSFORMSTATES);
	}

	// Renderstates
	uint32_t m_RenderStates[STATEMANAGER_MAX_RENDERSTATES];

	// Texture stage states
	uint32_t m_TextureStates[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_TEXTURESTATES];

	// Texture sampler states
	uint32_t m_SamplerStates[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_SAMPLERSTATES];

	// Vertex shader constants
	D3DXVECTOR4 m_VertexShaderConstants[STATEMANAGER_MAX_VCONSTANTS];

	// Pixel shader constants
	D3DXVECTOR4 m_PixelShaderConstants[STATEMANAGER_MAX_PCONSTANTS];

	// Textures
	LPDIRECT3DBASETEXTURE9 m_Textures[STATEMANAGER_MAX_STAGES];

	// Shaders
	LPDIRECT3DVERTEXSHADER9 m_dwVertexShader;
	LPDIRECT3DVERTEXDECLARATION9 m_dwVertexDeclaration;
	LPDIRECT3DPIXELSHADER9 m_dwPixelShader;

	uint32_t m_dwFVF;

	D3DXMATRIX m_Matrices[STATEMANAGER_MAX_TRANSFORMSTATES];

	D3DMATERIAL9 m_D3DMaterial;

	CStreamData m_StreamData[STATEMANAGER_MAX_STREAMS];
	CIndexData m_IndexData;
};

class CStateManager : public CSingleton<CStateManager>
{
public:
	CStateManager(LPDIRECT3DDEVICE9 lpDevice);
	virtual ~CStateManager();

	void SetDefaultState();
	void Restore();

	bool BeginScene();
	void EndScene();

	// Material
	void SaveMaterial();
	void SaveMaterial(const D3DMATERIAL9* pMaterial);
	void SetMaterial(const D3DMATERIAL9* pMaterial);
	void GetMaterial(D3DMATERIAL9* pMaterial);
	void SetLight(uint32_t index, CONST D3DLIGHT9* pLight);
	void GetLight(uint32_t index, D3DLIGHT9* pLight);
	void RestoreMaterial();

	// Renderstates
	void SaveRenderState(D3DRENDERSTATETYPE Type, uint32_t dwValue);
	void RestoreRenderState(D3DRENDERSTATETYPE Type);
	void SetRenderState(D3DRENDERSTATETYPE Type, uint32_t Value);
	void GetRenderState(D3DRENDERSTATETYPE Type, uint32_t * pdwValue);

	// Textures
	void SaveTexture(uint32_t dwStage, LPDIRECT3DBASETEXTURE9 pTexture);
	void RestoreTexture(uint32_t dwStage);
	void SetTexture(uint32_t dwStage, LPDIRECT3DBASETEXTURE9 pTexture);
	void GetTexture(uint32_t dwStage, LPDIRECT3DBASETEXTURE9* ppTexture);

	// Texture stage states
	void SaveTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type, uint32_t dwValue);
	void RestoreTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type);
	void SetTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type, uint32_t dwValue);
	void GetTextureStageState(uint32_t dwStage, D3DTEXTURESTAGESTATETYPE Type, uint32_t* pdwValue);
	void SaveSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type, uint32_t dwValue);
	void RestoreSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type);
	void SetSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type, uint32_t dwValue);
	void GetSamplerState(uint32_t dwStage, D3DSAMPLERSTATETYPE Type, uint32_t* pdwValue);

	void SetBestFiltering(uint32_t dwStage); // if possible set anisotropy filtering, or use trilinear

	// Vertex Shader
	void SaveVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader);
	void SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader);
	void GetVertexShader(LPDIRECT3DVERTEXSHADER9* pdwShader);
	void SaveVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader);
	void RestoreVertexDeclaration();
	void SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader);
	void GetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9* pdwShader);
	void RestoreVertexShader();

	// Pixel Shader
	void SavePixelShader(LPDIRECT3DPIXELSHADER9 lpShader);
	void SetPixelShader(LPDIRECT3DPIXELSHADER9 lpShader);
	void GetPixelShader(LPDIRECT3DPIXELSHADER9* lppShader);

	void SaveFVF(uint32_t dwFVF);
	void RestoreFVF();
	void SetFVF(uint32_t dwFVF);
	void GetFVF(uint32_t* dwFVF);
	void RestorePixelShader();

	// *** These states are cached, but not protected from multiple sends of the same value.
	// Transform
	void SaveTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX * pMatrix);
	void RestoreTransform(D3DTRANSFORMSTATETYPE Type);

	// Don't cache-check the transform.  To much to do
	void SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX * pMatrix);
	void GetTransform(D3DTRANSFORMSTATETYPE Type, D3DMATRIX * pMatrix);

	// SetVertexShaderConstant
	void RestoreVertexShaderConstant(uint32_t uiRegister, uint32_t uiConstantCount);
	// SetPixelShaderConstant
	void RestorePixelShaderConstant(uint32_t uiRegister, uint32_t uiConstantCount);
	void SetVertexShaderConstant(uint32_t dwRegister, CONST void* pConstantData, uint32_t dwConstantCount);
	void SaveVertexShaderConstant(uint32_t dwRegister, CONST void * pConstantData, uint32_t dwConstantCount);
	
	// SetPixelShaderConstant
	void SavePixelShaderConstant(uint32_t dwRegister, CONST void * pConstantData, uint32_t dwConstantCount);
	void SetPixelShaderConstant(uint32_t dwRegister, CONST void * pConstantData, uint32_t dwConstantCount);

	void SaveStreamSource(uint32_t StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, uint32_t Stride);
	void RestoreStreamSource(uint32_t StreamNumber);
	void SetStreamSource(uint32_t StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, uint32_t Stride);

	void SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, uint32_t BaseVertexIndex);
	void RestoreIndices();
	void SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, uint32_t BaseVertexIndex);

	HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount);
	HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, uint32_t PrimitiveCount, const void * pVertexStreamZeroData,
							uint32_t VertexStreamZeroStride);
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, uint32_t minIndex, uint32_t NumVertices, uint32_t startIndex,
								 uint32_t primCount, INT baseVertexIndex = 0);
	HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertexIndices,
								   uint32_t PrimitiveCount, CONST void * pIndexData, D3DFORMAT IndexDataFormat,
								   CONST void * pVertexStreamZeroData, uint32_t VertexStreamZeroStride);

	// Codes For Debug
	uint32_t GetRenderState(D3DRENDERSTATETYPE Type);

private:
	void SetDevice(LPDIRECT3DDEVICE9 lpDevice);

private:
	CStateManagerState m_ChipState;
	CStateManagerState m_CurrentState;
	CStateManagerState m_CopyState;
	TStateID m_DirtyStates;
	bool m_bForce;
	bool m_bScene;
	uint32_t m_dwBestMinFilter;
	uint32_t m_dwBestMagFilter;
	LPDIRECT3DDEVICE9 m_lpD3DDev;

#ifdef _DEBUG
	// Saving Flag
	BOOL m_bRenderStateSavingFlag[STATEMANAGER_MAX_RENDERSTATES];
	BOOL m_bTextureStageStateSavingFlag[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_TEXTURESTATES];
	BOOL m_bSamplerStateSavingFlag[STATEMANAGER_MAX_STAGES][STATEMANAGER_MAX_SAMPLERSTATES];
	BOOL m_bTransformSavingFlag[STATEMANAGER_MAX_TRANSFORMSTATES];
#endif _DEBUG
};

#define STATEMANAGER (CStateManager::Instance())

#endif __CSTATEMANAGER_H
