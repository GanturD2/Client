#pragma once

#include "GrpDetector.h"
#include "Ray.h"
#include <vector>
#include "../UserInterface/Locale_inc.h"

void PixelPositionToD3DXVECTOR3(const D3DXVECTOR3 & c_rkPPosSrc, D3DXVECTOR3 * pv3Dst);
void D3DXVECTOR3ToPixelPosition(const D3DXVECTOR3 & c_rv3Src, D3DXVECTOR3 * pv3Dst);

class CGraphicTexture;

typedef uint16_t TIndex;

typedef struct SFace
{
	TIndex indices[3];
} TFace;

typedef D3DXVECTOR3 TPosition;

typedef D3DXVECTOR3 TNormal;

typedef D3DXVECTOR2 TTextureCoordinate;

typedef uint32_t TDiffuse;
typedef uint32_t TAmbient;
typedef uint32_t TSpecular;

typedef union UDepth {
	float f;
	long l;
	uint32_t dw;
} TDepth;

typedef struct SVertex
{
	float x, y, z;
	uint32_t color;
	float u, v;
} TVertex;

struct STVertex
{
	float x, y, z, rhw;
};

struct SPVertex
{
	float x, y, z;
};

typedef struct SPDVertex
{
	float x, y, z;
	uint32_t color;
} TPDVertex;

struct SPDTVertexRaw
{
	float px, py, pz;
	uint32_t diffuse;
	float u, v;
};

typedef struct SPTVertex
{
	TPosition position;
	TTextureCoordinate texCoord;
} TPTVertex;

typedef struct SPDTVertex
{
	TPosition position;
	TDiffuse diffuse;
	TTextureCoordinate texCoord;
} TPDTVertex;

typedef struct SPNTVertex
{
	TPosition position;
	TNormal normal;
	TTextureCoordinate texCoord;
} TPNTVertex;

typedef struct SPNT2Vertex
{
	TPosition position;
	TNormal normal;
	TTextureCoordinate texCoord;
	TTextureCoordinate texCoord2;
} TPNT2Vertex;

typedef struct SPDT2Vertex
{
	TPosition position;
	uint32_t diffuse;
	TTextureCoordinate texCoord;
	TTextureCoordinate texCoord2;
} TPDT2Vertex;

typedef struct SNameInfo
{
	uint32_t name;
	TDepth depth;
} TNameInfo;

typedef struct SBoundBox
{
	float sx, sy, sz;
	float ex, ey, ez;
	int meshIndex;
	int boneIndex;
} TBoundBox;

const uint16_t c_FillRectIndices[6] = {0, 2, 1, 2, 3, 1};

/*
enum EIndexCount
{
	LINE_INDEX_COUNT = 2,
	TRIANGLE_INDEX_COUNT = 2*3,
	RECTANGLE_INDEX_COUNT = 2*4,
	CUBE_INDEX_COUNT = 2*4*3,
	FILLED_TRIANGLE_INDEX_COUNT = 3,
	FILLED_RECTANGLE_INDEX_COUNT = 3*2,
	FILLED_CUBE_INDEX_COUNT = 3*2*6
};
*/

class CGraphicBase
{
public:
	static uint32_t GetAvailableTextureMemory();
	static const D3DXMATRIX & GetViewMatrix();
	static const D3DXMATRIX & GetIdentityMatrix();

	enum
	{
		DEFAULT_IB_LINE,
		DEFAULT_IB_LINE_TRI,
		DEFAULT_IB_LINE_RECT,
		DEFAULT_IB_LINE_CUBE,
		DEFAULT_IB_FILL_TRI,
		DEFAULT_IB_FILL_RECT,
		DEFAULT_IB_FILL_CUBE,
		DEFAULT_IB_NUM
	};

public:
	CGraphicBase();
	virtual ~CGraphicBase();

	void SetSimpleCamera(float x, float y, float z, float pitch, float roll);
	void SetEyeCamera(float xEye, float yEye, float zEye, float xCenter, float yCenter, float zCenter, float xUp, float yUp, float zUp);
	void SetAroundCamera(float distance, float pitch, float roll, float lookAtZ = 0.0f);
	void SetPositionCamera(float fx, float fy, float fz, float distance, float pitch, float roll);
	void MoveCamera(float fdeltax, float fdeltay, float fdeltaz);

	void GetTargetPosition(float * px, float * py, float * pz);
	void GetCameraPosition(float * px, float * py, float * pz);
	void SetOrtho2D(float hres, float vres, float zres);
	void SetOrtho3D(float hres, float vres, float zmin, float zmax);
	void SetPerspective(float fov, float aspect, float nearz, float farz);
#if defined(ENABLE_RENDER_TARGET) || defined(ENABLE_INGAME_WIKI)
	float		GetFOV() const noexcept;
	float		GetAspect() const noexcept;
	float		GetNear() const noexcept;
	float		GetFar() const noexcept;
#else
	float		GetFOV();
#endif
	void GetClipPlane(float * fNearY, float * fFarY)
	{
		*fNearY = ms_fNearY;
		*fFarY = ms_fFarY;
	}

	////////////////////////////////////////////////////////////////////////
	void PushMatrix();

	void MultMatrix(const D3DXMATRIX * pMat);
	void MultMatrixLocal(const D3DXMATRIX * pMat);

	void Translate(float x, float y, float z);
	void Rotate(float degree, float x, float y, float z);
	void RotateLocal(float degree, float x, float y, float z);
	void RotateYawPitchRollLocal(float fYaw, float fPitch, float fRoll);
	void Scale(float x, float y, float z);
	void PopMatrix();
	void LoadMatrix(const D3DXMATRIX & c_rSrcMatrix);
	void GetMatrix(D3DXMATRIX * pRetMatrix) const;
	const D3DXMATRIX * GetMatrixPointer() const;

	// Special Routine
	void GetSphereMatrix(D3DXMATRIX * pMatrix, float fValue = 0.1f);

	////////////////////////////////////////////////////////////////////////
	void InitScreenEffect();
	void SetScreenEffectWaving(float fDuringTime, int iPower);
#ifdef ENABLE_EARTHQUAKE_EVENT
	void SetScreenEffectWavingEnd();
#endif
	void SetScreenEffectFlashing(float fDuringTime, const D3DXCOLOR & c_rColor);

	////////////////////////////////////////////////////////////////////////
	uint32_t GetColor(float r, float g, float b, float a = 1.0f);

	uint32_t GetFaceCount();
	void ResetFaceCount();
	HRESULT GetLastResult();

	void UpdateProjMatrix();
	void UpdateViewMatrix();

	void SetViewport(uint32_t dwX, uint32_t dwY, uint32_t dwWidth, uint32_t dwHeight, float fMinZ, float fMaxZ);
	static void GetBackBufferSize(uint32_t * puWidth, uint32_t * puHeight);
	static bool IsTLVertexClipping();
	static bool IsFastTNL();
	static bool IsLowTextureMemory();
	static bool IsHighTextureMemory();

	static void SetDefaultIndexBuffer(uint32_t eDefIB);
	static bool SetPDTStream(SPDTVertexRaw * pSrcVertices, uint32_t uVtxCount);
	static bool SetPDTStream(SPDTVertex * pVertices, uint32_t uVtxCount);

protected:
	static D3DXMATRIX ms_matIdentity;

	static D3DXMATRIX ms_matView;
	static D3DXMATRIX ms_matProj;
	static D3DXMATRIX ms_matInverseView;
	static D3DXMATRIX ms_matInverseViewYAxis;

	static D3DXMATRIX ms_matWorld;
	static D3DXMATRIX ms_matWorldView;

protected:
	//void		UpdatePrePipeLineMatrix();
	void UpdatePipeLineMatrix();

protected:
	// 각종 D3DX Mesh 들 (컬루젼 데이터 등을 표시활 때 쓴다)
	static LPD3DXMESH ms_lpSphereMesh;
	static LPD3DXMESH ms_lpCylinderMesh;

protected:
	static HRESULT ms_hLastResult;

	static int ms_iWidth;
	static int ms_iHeight;

	static uint32_t ms_iD3DAdapterInfo;
	static uint32_t ms_iD3DDevInfo;
	static uint32_t ms_iD3DModeInfo;
	static D3D_CDisplayModeAutoDetector ms_kD3DDetector;

	static HWND ms_hWnd;
	static HDC ms_hDC;
	static LPDIRECT3D9 ms_lpd3d;
	static LPDIRECT3DDEVICE9 ms_lpd3dDevice;
	static ID3DXMatrixStack * ms_lpd3dMatStack;
	static D3DVIEWPORT9 ms_Viewport;

	static uint32_t ms_faceCount;
	static D3DCAPS9 ms_d3dCaps;
	static D3DPRESENT_PARAMETERS ms_d3dPresentParameter;

	static uint32_t ms_dwD3DBehavior;
	static uint32_t ms_ptVS;
	static uint32_t ms_pntVS;
	static uint32_t ms_pnt2VS;

	static D3DXMATRIX ms_matScreen0;
	static D3DXMATRIX ms_matScreen1;
	static D3DXMATRIX ms_matScreen2;
	//static D3DXMATRIX				ms_matPrePipeLine;

	static D3DXVECTOR3 ms_vtPickRayOrig;
	static D3DXVECTOR3 ms_vtPickRayDir;

	static float ms_fFieldOfView;
	static float ms_fAspect;
	static float ms_fNearY;
	static float ms_fFarY;

	// 2004.11.18.myevan.DynamicVertexBuffer로 교체
	/*
		static std::vector<TIndex>		ms_lineIdxVector;
		static std::vector<TIndex>		ms_lineTriIdxVector;
		static std::vector<TIndex>		ms_lineRectIdxVector;
		static std::vector<TIndex>		ms_lineCubeIdxVector;

		static std::vector<TIndex>		ms_fillTriIdxVector;
		static std::vector<TIndex>		ms_fillRectIdxVector;
		static std::vector<TIndex>		ms_fillCubeIdxVector;
		*/

	// Screen Effect - Waving, Flashing and so on..
	static uint32_t ms_dwWavingEndTime;
	static int ms_iWavingPower;
	static uint32_t ms_dwFlashingEndTime;
	static D3DXCOLOR ms_FlashingColor;

	// Terrain picking용 Ray... CCamera 이용하는 버전.. 기존의 Ray와 통합 필요...
	static CRay ms_Ray;

	//
	static bool ms_bSupportDXT;
	static bool ms_isLowTextureMemory;
	static bool ms_isHighTextureMemory;

	enum
	{
		PDT_VERTEX_NUM = 16,
		PDT_VERTEXBUFFER_NUM = 100
	};


	static LPDIRECT3DVERTEXBUFFER9 ms_alpd3dPDTVB[PDT_VERTEXBUFFER_NUM];
	static LPDIRECT3DINDEXBUFFER9 ms_alpd3dDefIB[DEFAULT_IB_NUM];
};
