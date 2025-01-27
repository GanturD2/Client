#pragma once

//#define CACHE_DEFORMED_VERTEX
#include "../EterLib/GrpImage.h"
#include "../EterLib/GrpCollisionObject.h"

#include "Model.h"
#include "Motion.h"

class CGrannyModelInstance : public CGraphicCollisionObject
{
public:
	enum
	{
		ANIFPS_MIN = 30,
		ANIFPS_MAX = 120
	};

public:
	static void DestroySystem();

	static CGrannyModelInstance * New();
	static void Delete(CGrannyModelInstance * pkInst);

	static CDynamicPool<CGrannyModelInstance> ms_kPool;

public:
	struct FCreateDeviceObjects
	{
		void operator()(CGrannyModelInstance * pModelInstance) const { pModelInstance->CreateDeviceObjects(); }
	};

	struct FDestroyDeviceObjects
	{
		void operator()(CGrannyModelInstance * pModelInstance) const { pModelInstance->DestroyDeviceObjects(); }
	};

public:
	CGrannyModelInstance();
	~CGrannyModelInstance() override;

	bool IsEmpty() const;
	void Clear();

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	// Update & Render
	void Update(uint32_t dwAniFPS);
	void UpdateLocalTime(float fElapsedTime);
	void UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed) const;

	void UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float fLocalTime);
	void DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix);
	void Deform(const D3DXMATRIX * c_pWorldMatrix);

	// FIXME : 현재는 하드웨어의 한계로 2장의 텍스춰로 제한이 되어있는 상태이기에 이런
	//         불안정한 아키텍춰가 가능하지만, 궁극적인 방향은 (모델 텍스춰 전부) + (효과용 텍스춰)
	//         이런식의 자동 셋팅이 이뤄져야 되지 않나 생각합니다. - [levites]
	// NOTE : 내부에 if문을 포함 시키기 보다는 조금은 번거롭지만 이렇게 함수 콜 자체를 분리
	//        시키는 것이 퍼포먼스 적인 측면에서는 더 나은 것 같습니다. - [levites]
	// NOTE : 건물은 무조건 OneTexture. 캐릭터는 경우에 따라 TwoTexture.
	void RenderWithOneTexture();
	void RenderWithTwoTexture();
	void BlendRenderWithOneTexture();
	void BlendRenderWithTwoTexture();
	void RenderWithoutTexture();

	// Model
	CGrannyModel * GetModel() const;
	void SetMaterialImagePointer(const char * c_szImageName, CGraphicImage * pImage);
	void SetMaterialData(const char * c_szImageName, const SMaterialData & c_rkMaterialData);
	void SetSpecularInfo(const char * c_szMtrlName, BOOL bEnable, float fPower);

	void SetMainModelPointer(CGrannyModel * pModel, CGraphicVertexBuffer * pkSharedDeformableVertexBuffer);
	void SetLinkedModelPointer(CGrannyModel * pkModel, CGraphicVertexBuffer * pkSharedDeformableVertexBuffer,
							   CGrannyModelInstance ** ppkSkeletonInst);

	// Motion
	void SetMotionPointer(const CGrannyMotion * pMotion, float blendTime = 0.0f, int loopCount = 0, float speedRatio = 1.0f);
	void ChangeMotionPointer(const CGrannyMotion * pMotion, int loopCount = 0, float speedRatio = 1.0f);
	void SetMotionAtEnd() const;
	bool IsMotionPlaying() const;

	void CopyMotion(CGrannyModelInstance * pModelInstance, bool bIsFreeSourceControl = false);

	// Time
	void SetLocalTime(float fLocalTime);
	int ResetLocalTime();
	float GetLocalTime() const;
	float GetNextTime();

	// WORK
	uint32_t GetDeformableVertexCount() const;
	uint32_t GetVertexCount() const;

	// END_OF_WORK

	// Bone & Attaching
	float * GetBoneMatrixPointer(int iBone) const;
	float * GetCompositeBoneMatrixPointer(int iBone) const;
	bool GetMeshMatrixPointer(int iMesh, const D3DXMATRIX ** c_ppMatrix) const;
	bool GetBoneIndexByName(const char * c_szBoneName, int * pBoneIndex) const;
	void SetParentModelInstance(const CGrannyModelInstance * c_pParentModelInstance, const char * c_szBoneName);
	void SetParentModelInstance(const CGrannyModelInstance * c_pParentModelInstance, int iBone);

	// Collision Detection
	bool Intersect(const D3DXMATRIX * c_pMatrix, float * pu, float * pv, float * pt);
	static void MakeBoundBox(TBoundBox * pBoundBox, const float * mat, const float * OBBMin, const float * OBBMax, D3DXVECTOR3 * vtMin,
							 D3DXVECTOR3 * vtMax);
	void GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);

	// Reload Texture
	static void ReloadTexture();


protected:
	void __Initialize();

	void __DestroyModelInstance();
	void __DestroyMeshMatrices();
	void __DestroyDynamicVertexBuffer();


	void __CreateModelInstance();
	void __CreateMeshMatrices();
	void __CreateDynamicVertexBuffer();

	// WORK
	void __DestroyWorldPose();
	void __CreateWorldPose(CGrannyModelInstance * pkSkeletonInst);

	bool __CreateMeshBindingVector(CGrannyModelInstance * pkDstModelInst);
	void __DestroyMeshBindingVector();

#if GrannyProductMinorVersion == 4
	int * __GetMeshBoneIndices(unsigned int iMeshBinding) const;
#elif GrannyProductMinorVersion == 11 || GrannyProductMinorVersion == 9 || GrannyProductMinorVersion == 8 || GrannyProductMinorVersion == 7
	const granny_int32x * __GetMeshBoneIndices(unsigned int iMeshBinding) const;
#else
#	error "unknown granny version"
#endif

	bool __IsDeformableVertexBuffer() const;
	void __SetSharedDeformableVertexBuffer(CGraphicVertexBuffer * pkSharedDeformableVertexBuffer);

	IDirect3DVertexBuffer9* __GetDeformableD3DVertexBufferPtr();
	CGraphicVertexBuffer & __GetDeformableVertexBufferRef();

	granny_world_pose * __GetWorldPosePtr() const;
	// END_OF_WORK


	// Update & Render
	void UpdateWorldPose();
	void UpdateWorldMatrices(const D3DXMATRIX * c_pWorldMatrix);
	void DeformPNTVertices(void * pvDest) const;

	void RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);
	void RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType);
	void RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType) const;

protected:
	// Static Data
	CGrannyModel * m_pModel;

	// Granny Data
	granny_model_instance * m_pgrnModelInstance;

	//granny_world_pose *				m_pgrnWorldPose;		// 현재 월드 포즈 포인터

	granny_control * m_pgrnCtrl;
	granny_animation * m_pgrnAni;

	// Meshes' Transform Data
	std::vector<D3DXMATRIX> m_meshMatrices;


	// Attaching Data
	const CGrannyModelInstance * mc_pParentInstance;
	int m_iParentBoneIndex;

	// Game Data
	float m_fLocalTime;
	float m_fSecondsElapsed;

	uint32_t m_dwOldUpdateFrame;

	CGrannyMaterialPalette m_kMtrlPal;

	// WORK
	granny_world_pose * m_pgrnWorldPoseReal; // 실제 메모리는 여기에 할당
	std::vector<granny_mesh_binding *> m_vct_pgrnMeshBinding;

	// Dynamic Vertex Buffer
	CGraphicVertexBuffer * m_pkSharedDeformableVertexBuffer;
	CGraphicVertexBuffer m_kLocalDeformableVertexBuffer;
	bool m_isDeformableVertexBuffer;
	// END_OF_WORK

	// TEST
	CGrannyModelInstance ** m_ppkSkeletonInst;
	// END_OF_TEST
public:
	bool HaveBlendThing() const { return m_pModel->HaveBlendThing(); }
};
