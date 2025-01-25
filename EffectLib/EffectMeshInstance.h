#pragma once

#include "../EterLib/GrpImageInstance.h"
#include "EffectElementBaseInstance.h"
#include "FrameController.h"
#include "EffectMesh.h"

class CEffectMeshInstance : public CEffectElementBaseInstance
{
public:
	// NOTE : Mesh 단위 텍스춰 데이타의 인스턴스이다.
	typedef struct STextureInstance
	{
		CFrameController TextureFrameController;
		std::vector<CGraphicImageInstance *> TextureInstanceVector;
	} TTextureInstance;

public:
	CEffectMeshInstance();
	~CEffectMeshInstance() override;

public:
	static void DestroySystem();

	static CEffectMeshInstance * New();
	static void Delete(CEffectMeshInstance * pkMeshInstance);

	static CDynamicPool<CEffectMeshInstance> ms_kPool;

protected:
	void OnSetDataPointer(CEffectElementBase * pElement) override;

	void OnInitialize() override;
	void OnDestroy() override;

	bool OnUpdate(float fElapsedTime) override;
	void OnRender() override;

	BOOL isActive();

protected:
	CEffectMeshScript * m_pMeshScript{};
	CEffectMesh * m_pEffectMesh{};

	CFrameController m_MeshFrameController;
	std::vector<TTextureInstance> m_TextureInstanceVector;

	CEffectMesh::TRef m_roMesh;
};
