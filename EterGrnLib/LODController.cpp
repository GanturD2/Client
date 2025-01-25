#include "StdAfx.h"
#include "LODController.h"

#include <ctime>

static float LODHEIGHT_ACTOR = 500.0f;
static float LODDISTANCE_ACTOR = 5000.0f;
static float LODDISTANCE_BUILDING = 25000.0f;

static const float LOD_APPLY_MIN = 500.0f;

bool ms_isMinLODModeEnable = false;

enum
{
	SHARED_VB_500 = 0,
	SHARED_VB_1000 = 1,
	SHARED_VB_1500 = 2,
	SHARED_VB_2000 = 3,
	SHARED_VB_2500 = 4,
	SHARED_VB_3000 = 5,
	SHARED_VB_3500 = 6,
	SHARED_VB_4000 = 7,
	SHARED_VB_NUM = 9
};

static std::vector<CGraphicVertexBuffer *> gs_vbs[SHARED_VB_NUM];

static CGraphicVertexBuffer gs_emptyVB;

static CGraphicVertexBuffer * __AllocDeformVertexBuffer(unsigned deformableVertexCount)
{
	if (deformableVertexCount == 0)
		return &gs_emptyVB;

	unsigned capacity = (((deformableVertexCount - 1) / 500) + 1) * 500;
	unsigned index = (deformableVertexCount - 1) / 500;
	if (index < SHARED_VB_NUM)
	{
		std::vector<CGraphicVertexBuffer *> & vbs = gs_vbs[index];
		if (!vbs.empty())
		{
			CGraphicVertexBuffer * pkRetVB = vbs.back();
			vbs.pop_back();
			return pkRetVB;
		}
	}


	static time_t base = time(nullptr);

	auto pkNewVB = new CGraphicVertexBuffer;

	if (!pkNewVB->Create(capacity, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED))
		TraceError("NEW_ERROR %8d: %d(%d)", time(nullptr) - base, capacity, deformableVertexCount);

	return pkNewVB;
}

void __FreeDeformVertexBuffer(CGraphicVertexBuffer * pkDelVB)
{
	if (pkDelVB)
	{
		if (pkDelVB == &gs_emptyVB)
			return;

		unsigned index = (pkDelVB->GetVertexCount() - 1) / 500;
		if (index < SHARED_VB_NUM)
			gs_vbs[index].emplace_back(pkDelVB);
		else
		{
			pkDelVB->Destroy();
			delete pkDelVB;
		}
	}
}

void __ReserveSharedVertexBuffers(unsigned index, unsigned count)
{
	NANOBEGIN
	if (index >= SHARED_VB_NUM)
		return;

	unsigned capacity = (index + 1) * 500;

	for (unsigned i = 0; i != count; ++i)
	{
		auto pkNewVB = new CGraphicVertexBuffer;
		pkNewVB->Create(capacity, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED);
		gs_vbs[index].emplace_back(pkNewVB);
	}
	NANOEND
}

void GrannyCreateSharedDeformBuffer()
{
	__ReserveSharedVertexBuffers(SHARED_VB_500, 40);
	__ReserveSharedVertexBuffers(SHARED_VB_1000, 20);
	__ReserveSharedVertexBuffers(SHARED_VB_1500, 20);
	__ReserveSharedVertexBuffers(SHARED_VB_2000, 40);
	__ReserveSharedVertexBuffers(SHARED_VB_3000, 20);
}

void GrannyDestroySharedDeformBuffer()
{
#ifdef _DEBUG
	TraceError("granny_shared_vbs:");
#endif
	for (int i = 0; i != SHARED_VB_NUM; ++i)
	{
		std::vector<CGraphicVertexBuffer *> & vbs = gs_vbs[i];
#ifdef _DEBUG
		TraceError("\t%d: %d", i, vbs.size());
#endif

		std::vector<CGraphicVertexBuffer *>::iterator v;
		for (v = vbs.begin(); v != vbs.end(); ++v)
		{
			CGraphicVertexBuffer * pkEachVB = (*v);
			pkEachVB->Destroy();
			delete pkEachVB;
		}
		vbs.clear();
	}
}

void CGrannyLODController::SetMinLODMode(bool isEnable)
{
	ms_isMinLODModeEnable = isEnable;
}

void CGrannyLODController::SetMaterialImagePointer(const char * c_szImageName, CGraphicImage * pImage)
{
	std::deque<CGrannyModelInstance *>::iterator i;
	for (i = m_que_pkModelInst.begin(); i != m_que_pkModelInst.end(); ++i)
	{
		CGrannyModelInstance * pkModelInst = (*i);
		pkModelInst->SetMaterialImagePointer(c_szImageName, pImage);
	}
}

void CGrannyLODController::SetMaterialData(const char * c_szImageName, const SMaterialData & c_rkMaterialData)
{
	std::deque<CGrannyModelInstance *>::iterator i;
	for (i = m_que_pkModelInst.begin(); i != m_que_pkModelInst.end(); ++i)
	{
		CGrannyModelInstance * pkModelInst = (*i);
		pkModelInst->SetMaterialData(c_szImageName, c_rkMaterialData);
	}
}

void CGrannyLODController::SetSpecularInfo(const char * c_szMtrlName, BOOL bEnable, float fPower)
{
	std::deque<CGrannyModelInstance *>::iterator i;
	for (i = m_que_pkModelInst.begin(); i != m_que_pkModelInst.end(); ++i)
	{
		CGrannyModelInstance * pkModelInst = (*i);
		pkModelInst->SetSpecularInfo(c_szMtrlName, bEnable, fPower);
	}
}

CGrannyLODController::CGrannyLODController() = default;

CGrannyLODController::~CGrannyLODController()
{
	__FreeDeformVertexBuffer(m_pkSharedDeformableVertexBuffer);

	Clear();
}

void CGrannyLODController::Clear()
{
	if (m_pAttachedParentModel)
		m_pAttachedParentModel->DetachModelInstance(this);

	m_pCurrentModelInstance = nullptr;
	m_pAttachedParentModel = nullptr;

	std::for_each(m_que_pkModelInst.begin(), m_que_pkModelInst.end(), CGrannyModelInstance::Delete);
	m_que_pkModelInst.clear();

	for (auto & rData : m_AttachedModelDataVector)
		rData.pkLODController->m_pAttachedParentModel = nullptr;

	m_AttachedModelDataVector.clear();
}

void CGrannyLODController::AddModel(CGraphicThing * pThing, int iSrcModel, CGrannyLODController * pSkelLODController)
{
	if (!pThing)
		return;

	if (pSkelLODController && pSkelLODController->m_que_pkModelInst.empty())
	{
		assert(!"EMPTY SKELETON(CANNON LINK)");
		return;
	}

	assert(pThing->GetReferenceCount() >= 1);

	pThing->AddReference();

	if (pThing->GetModelCount() <= iSrcModel)
	{
		pThing->Release();
		return;
	}
	CGrannyModel * pModel = pThing->GetModelPointer(iSrcModel);
	if (!pModel)
	{
		pThing->Release();
		return;
	}

	CGrannyModelInstance * pModelInstance = CGrannyModelInstance::New();

	__ReserveSharedDeformableVertexBuffer(pModel->GetDeformVertexCount());

	if (pSkelLODController)
		pModelInstance->SetLinkedModelPointer(pModel, m_pkSharedDeformableVertexBuffer, &pSkelLODController->m_pCurrentModelInstance);
	else
		pModelInstance->SetLinkedModelPointer(pModel, m_pkSharedDeformableVertexBuffer, nullptr);

	// END_OF_WORK

	if (!m_pCurrentModelInstance)
	{
		m_pCurrentModelInstance = pModelInstance;
		pModelInstance->DeformNoSkin(&ms_matIdentity);

		D3DXVECTOR3 vtMin, vtMax;
		pModelInstance->GetBoundBox(&vtMin, &vtMax);

		float fSize = 0.0f;
		fSize = fMAX(fSize, fabs(vtMin.x - vtMax.x));
		fSize = fMAX(fSize, fabs(vtMin.y - vtMax.y));
		fSize = fMAX(fSize, fabs(vtMin.z - vtMax.z));

		if (fSize < LODHEIGHT_ACTOR)
			SetLODLimits(0.0f, LODDISTANCE_ACTOR);
		else
			//
			SetLODLimits(0.0f, LODDISTANCE_BUILDING);
	}
	else
	{
		// FIXME : CModelInstance::m_pgrnWorldPose�� Update���� ����ϴµ�,
		//         Deform�� ���� ������ nullptr �Դϴ�. ������ ���� �ٲ��� ������.. - [levites]
		pModelInstance->DeformNoSkin(&ms_matIdentity);
	}

	pThing->Release();

	m_que_pkModelInst.push_front(pModelInstance);
}


void CGrannyLODController::__ReserveSharedDeformableVertexBuffer(uint32_t deformableVertexCount)
{
	if (m_pkSharedDeformableVertexBuffer && m_pkSharedDeformableVertexBuffer->GetVertexCount() >= deformableVertexCount)
		return;

	__FreeDeformVertexBuffer(m_pkSharedDeformableVertexBuffer);

	m_pkSharedDeformableVertexBuffer = __AllocDeformVertexBuffer(deformableVertexCount);
}

void CGrannyLODController::AttachModelInstance(CGrannyLODController * pSrcLODController, const char * c_szBoneName)
{
	CGrannyModelInstance * pSrcInstance = pSrcLODController->GetModelInstance();
	if (!pSrcInstance)
		return;

	CGrannyModelInstance * pDestInstance = GetModelInstance();
	if (pDestInstance)
		pSrcInstance->SetParentModelInstance(pDestInstance, c_szBoneName);

	if (!pSrcLODController->GetModelInstance())
		return;

	// Link Parent Data
	pSrcLODController->m_pAttachedParentModel = this;

	// Link Child Data
	auto itor = m_AttachedModelDataVector.begin();
	for (; m_AttachedModelDataVector.end() != itor;)
	{
		TAttachingModelData & rData = *itor;
		if (pSrcLODController == rData.pkLODController)
			itor = m_AttachedModelDataVector.erase(itor);
		else
			++itor;
	}

	TAttachingModelData AttachingModelData;
	AttachingModelData.pkLODController = pSrcLODController;
	AttachingModelData.strBoneName = c_szBoneName;
	m_AttachedModelDataVector.emplace_back(AttachingModelData);
}

void CGrannyLODController::DetachModelInstance(CGrannyLODController * pSrcLODController)
{
	CGrannyModelInstance * pSrcInstance = pSrcLODController->GetModelInstance();
	if (!pSrcInstance)
		return;

	CGrannyModelInstance * pDestInstance = GetModelInstance();
	if (pDestInstance)
		pSrcInstance->SetParentModelInstance(nullptr, 0);

	// Unlink Child Data
	auto itor = m_AttachedModelDataVector.begin();
	for (; m_AttachedModelDataVector.end() != itor;)
	{
		TAttachingModelData & rData = *itor;
		if (pSrcLODController == rData.pkLODController)
			itor = m_AttachedModelDataVector.erase(itor);
		else
			++itor;
	}

	// Unlink Parent Data
	pSrcLODController->m_pAttachedParentModel = nullptr;
}

void CGrannyLODController::SetLODLimits(float /*fNearLOD*/, float fFarLOD)
{
	m_fLODDistance = fFarLOD;
}

void CGrannyLODController::SetLODLevel(uint8_t bLodLevel)
{
	assert(m_que_pkModelInst.size() > 0);

	if (!m_que_pkModelInst.empty())
		m_bLODLevel = static_cast<uint8_t>(MIN(m_que_pkModelInst.size() - 1, bLodLevel));
}

void CGrannyLODController::CreateDeviceObjects()
{
	std::for_each(m_que_pkModelInst.begin(), m_que_pkModelInst.end(), CGrannyModelInstance::FCreateDeviceObjects());
}

void CGrannyLODController::DestroyDeviceObjects()
{
	std::for_each(m_que_pkModelInst.begin(), m_que_pkModelInst.end(), CGrannyModelInstance::FDestroyDeviceObjects());
}

void CGrannyLODController::RenderWithOneTexture() const
{
	assert(m_pCurrentModelInstance != nullptr);

//#define CHECK_LOD
#ifdef CHECK_LOD
	if (m_que_pkModelInst.size() > 0 && m_pCurrentModelInstance == m_que_pkModelInst[0])
		m_pCurrentModelInstance->RenderWithoutTexture();

	if (m_que_pkModelInst.size() > 1 && m_pCurrentModelInstance == m_que_pkModelInst[1])
		m_pCurrentModelInstance->RenderWithOneTexture();

	if (m_que_pkModelInst.size() > 2 && m_pCurrentModelInstance == m_que_pkModelInst[2])
		m_pCurrentModelInstance->RenderWithOneTexture();

	if (m_que_pkModelInst.size() > 3 && m_pCurrentModelInstance == m_que_pkModelInst[3])
		m_pCurrentModelInstance->RenderWithOneTexture();

#else
	m_pCurrentModelInstance->RenderWithOneTexture();
#endif
}

void CGrannyLODController::BlendRenderWithOneTexture() const
{
	assert(m_pCurrentModelInstance != nullptr);
	m_pCurrentModelInstance->BlendRenderWithOneTexture();
}

void CGrannyLODController::RenderWithTwoTexture() const
{
	assert(m_pCurrentModelInstance != nullptr);
	m_pCurrentModelInstance->RenderWithTwoTexture();
}

void CGrannyLODController::BlendRenderWithTwoTexture() const
{
	assert(m_pCurrentModelInstance != nullptr);
	m_pCurrentModelInstance->BlendRenderWithTwoTexture();
}

void CGrannyLODController::Update(float fElapsedTime, float fDistanceFromCenter, float fDistanceFromCamera)
{
	UpdateLODLevel(fDistanceFromCenter, fDistanceFromCamera);
	UpdateTime(fElapsedTime);
}

void CGrannyLODController::UpdateLODLevel(float fDistanceFromCenter, float fDistanceFromCamera)
{
	if (m_que_pkModelInst.size() <= 1)
		return;

	assert(m_pCurrentModelInstance != nullptr);


	if (fDistanceFromCenter > LOD_APPLY_MIN) // �߽� LOD ���� ���
	{
		// ī�޶���� �־��� ���� fLODRate�� �۾�����
		// 3�� LOD�� ������.. ���� �հ� 0, ����� ���� ���ڰ� Ŀ����

		// 100fps 50fps 33fps 25fps 20fps
		// 10ms 20ms 30ms 40ms 50ms
		float fLODFactor = fMINMAX(0.0f, (m_fLODDistance - fDistanceFromCamera), m_fLODDistance);

		if (m_fLODDistance > 0.0f)
			m_dwLODAniFPS =
				static_cast<uint32_t>((CGrannyModelInstance::ANIFPS_MAX - CGrannyModelInstance::ANIFPS_MIN) * fLODFactor / m_fLODDistance +
									  CGrannyModelInstance::ANIFPS_MIN);
		else
			m_dwLODAniFPS = CGrannyModelInstance::ANIFPS_MIN;

		assert(m_dwLODAniFPS > 0);
		m_dwLODAniFPS /= 10;
		m_dwLODAniFPS *= 10;

		float fLODStep = m_fLODDistance / m_que_pkModelInst.size();
		auto bLODLevel = uint8_t(fLODFactor / fLODStep);

		if (m_fLODDistance <= 5000.0f)
		{
			if (fDistanceFromCamera < 500.0f)
				bLODLevel = 0;
			else if (fDistanceFromCamera < 1500.0f)
				bLODLevel = 1;
			else if (fDistanceFromCamera < 2500.0f)
				bLODLevel = 2;
			else
				bLODLevel = 3;

			bLODLevel = static_cast<uint8_t>(m_que_pkModelInst.size() - std::min<size_t>(bLODLevel, m_que_pkModelInst.size()) - 1);
		}

		if (ms_isMinLODModeEnable)
			bLODLevel = 0;

		SetLODLevel(bLODLevel);

		if (m_pCurrentModelInstance != m_que_pkModelInst[m_bLODLevel])
			SetCurrentModelInstance(m_que_pkModelInst[m_bLODLevel]);
	}
	else
	{
		m_dwLODAniFPS = CGrannyModelInstance::ANIFPS_MAX;

		if (!m_que_pkModelInst.empty())
		{
			if (m_pCurrentModelInstance != m_que_pkModelInst.back())
				SetCurrentModelInstance(m_que_pkModelInst.back());
		}
	}
}

void CGrannyLODController::UpdateTime(float fElapsedTime) const
{
	assert(m_pCurrentModelInstance != nullptr);

	m_pCurrentModelInstance->Update(m_dwLODAniFPS);

	m_pCurrentModelInstance->UpdateLocalTime(fElapsedTime);
}

void CGrannyLODController::SetCurrentModelInstance(CGrannyModelInstance * pgrnModelInstance)
{
	// Copy Motion
	pgrnModelInstance->CopyMotion(m_pCurrentModelInstance, true);
	m_pCurrentModelInstance = pgrnModelInstance;

	// Change children attaching link
	RefreshAttachedModelInstance();

	// Change parent attaching link
	if (m_pAttachedParentModel)
		m_pAttachedParentModel->RefreshAttachedModelInstance();
}

void CGrannyLODController::RefreshAttachedModelInstance()
{
	if (!m_pCurrentModelInstance)
		return;

	for (uint32_t i = 0; i < m_AttachedModelDataVector.size(); ++i)
	{
		TAttachingModelData & rModelData = m_AttachedModelDataVector[i];

		CGrannyModelInstance * pSrcInstance = rModelData.pkLODController->GetModelInstance();
		if (!pSrcInstance)
		{
			Tracenf("CGrannyLODController::RefreshAttachedModelInstance : "
					"m_AttachedModelDataVector[%d]->pkLODController->GetModelIntance()==nullptr",
					i);
			continue;
		}

		pSrcInstance->SetParentModelInstance(m_pCurrentModelInstance, rModelData.strBoneName.c_str());
	}
}

void CGrannyLODController::UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float fElapsedTime) const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->UpdateSkeleton(c_pWorldMatrix, fElapsedTime);
}

void CGrannyLODController::DeformAll(const D3DXMATRIX * c_pWorldMatrix)
{
	std::deque<CGrannyModelInstance *>::iterator i;
	for (i = m_que_pkModelInst.begin(); i != m_que_pkModelInst.end(); ++i)
	{
		CGrannyModelInstance * pkModelInst = (*i);
		pkModelInst->Deform(c_pWorldMatrix);
	}
}

void CGrannyLODController::DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix) const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->DeformNoSkin(c_pWorldMatrix);
}

void CGrannyLODController::Deform(const D3DXMATRIX * c_pWorldMatrix) const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->Deform(c_pWorldMatrix);
}

void CGrannyLODController::RenderToShadowMap() const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->RenderWithoutTexture();
}

void CGrannyLODController::RenderShadow() const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->RenderWithOneTexture();
}

void CGrannyLODController::ReloadTexture() const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->ReloadTexture();
}

void CGrannyLODController::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax) const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->GetBoundBox(vtMin, vtMax);
}

bool CGrannyLODController::Intersect(const D3DXMATRIX * c_pMatrix, float * u, float * v, float * t) const
{
	if (!m_pCurrentModelInstance)
		return false;
	return m_pCurrentModelInstance->Intersect(c_pMatrix, u, v, t);
}

void CGrannyLODController::SetLocalTime(float fLocalTime) const
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->SetLocalTime(fLocalTime);
}

void CGrannyLODController::ResetLocalTime() const
{
	assert(m_pCurrentModelInstance != nullptr);
	m_pCurrentModelInstance->ResetLocalTime();
}

void CGrannyLODController::SetMotionPointer(const CGrannyMotion * c_pMotion, float fBlendTime, int iLoopCount, float speedRatio) const
{
	assert(m_pCurrentModelInstance != nullptr);
	m_pCurrentModelInstance->SetMotionPointer(c_pMotion, fBlendTime, iLoopCount, speedRatio);
}

void CGrannyLODController::ChangeMotionPointer(const CGrannyMotion * c_pMotion, int iLoopCount, float speedRatio) const
{
	assert(m_pCurrentModelInstance != nullptr);
	m_pCurrentModelInstance->ChangeMotionPointer(c_pMotion, iLoopCount, speedRatio);
}

void CGrannyLODController::SetMotionAtEnd()
{
	if (m_pCurrentModelInstance)
		m_pCurrentModelInstance->SetMotionAtEnd();
}

BOOL CGrannyLODController::isModelInstance() const
{
	if (!m_pCurrentModelInstance)
		return FALSE;
	return TRUE;
}

CGrannyModelInstance * CGrannyLODController::GetModelInstance() const
{
	return m_pCurrentModelInstance;
}
