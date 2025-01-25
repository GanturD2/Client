#include "StdAfx.h"
#include "../EterBase/Debug.h"
#include "ModelInstance.h"
#include "Model.h"


void CGrannyModelInstance::Update(uint32_t dwAniFPS)
{
	if (!dwAniFPS)
		return;

	const auto c_dwCurUpdateFrame = static_cast<uint32_t>(GetLocalTime() * ANIFPS_MAX);
	const uint32_t ANIFPS_STEP = ANIFPS_MAX / dwAniFPS;
	if (c_dwCurUpdateFrame > ANIFPS_STEP && c_dwCurUpdateFrame / ANIFPS_STEP == m_dwOldUpdateFrame / ANIFPS_STEP)
		return;

	m_dwOldUpdateFrame = c_dwCurUpdateFrame;

	GrannyFreeCompletedModelControls(m_pgrnModelInstance);
	GrannySetModelClock(m_pgrnModelInstance, GetLocalTime());
}

void CGrannyModelInstance::UpdateLocalTime(float fElapsedTime)
{
	m_fSecondsElapsed = fElapsedTime;
	m_fLocalTime += fElapsedTime;
}

void CGrannyModelInstance::UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed) const
{
	if (!m_pgrnModelInstance)
	{
		TraceError("CGrannyModelIstance::UpdateTransform - m_pgrnModelInstance = nullptr");
		return;
	}
#if GrannyProductMinorVersion == 4
	GrannyUpdateModelMatrix(m_pgrnModelInstance, fSecondsElapsed, reinterpret_cast<const float *>(pMatrix),
							reinterpret_cast<float *>(pMatrix));
#elif GrannyProductMinorVersion == 11 || GrannyProductMinorVersion == 9 || GrannyProductMinorVersion == 8 || GrannyProductMinorVersion == 7
	GrannyUpdateModelMatrix(m_pgrnModelInstance, fSecondsElapsed, reinterpret_cast<const float *>(pMatrix),
							reinterpret_cast<float *>(pMatrix), false);
#else
#	error "unknown granny version"
#endif
}

void CGrannyModelInstance::Deform(const D3DXMATRIX * c_pWorldMatrix)
{
	if (IsEmpty())
		return;

	UpdateWorldPose();
	UpdateWorldMatrices(c_pWorldMatrix);

	if (m_pModel->CanDeformPNTVertices())
	{
		// WORK
		CGraphicVertexBuffer & rkDeformableVertexBuffer = __GetDeformableVertexBufferRef();
		TPNTVertex * pntVertices;
		if (rkDeformableVertexBuffer.LockRange(m_pModel->GetDeformVertexCount(), reinterpret_cast<void **>(&pntVertices)))
		{
			DeformPNTVertices(pntVertices);
			rkDeformableVertexBuffer.Unlock();
		}
		else
			TraceError("GRANNY DEFORM DYNAMIC BUFFER LOCK ERROR");
		// END_OF_WORK
	}
}

//////////////////////////////////////////////////////
class CGrannyLocalPose
{
public:
	CGrannyLocalPose()
	{
		m_pgrnLocalPose = nullptr;
		m_boneCount = 0;
	}

	virtual ~CGrannyLocalPose()
	{
		if (m_pgrnLocalPose)
			GrannyFreeLocalPose(m_pgrnLocalPose);
	}

	granny_local_pose * Get(int boneCount)
	{
		if (m_pgrnLocalPose)
		{
			if (m_boneCount >= boneCount)
				return m_pgrnLocalPose;

			GrannyFreeLocalPose(m_pgrnLocalPose);
		}

		m_boneCount = boneCount;
		m_pgrnLocalPose = GrannyNewLocalPose(m_boneCount);
		return m_pgrnLocalPose;
	}

private:
	granny_local_pose * m_pgrnLocalPose;
	int m_boneCount;
};
//////////////////////////////////////////////////////

void CGrannyModelInstance::UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float /*fLocalTime*/)
{
	UpdateWorldPose();
	UpdateWorldMatrices(c_pWorldMatrix);
}

void CGrannyModelInstance::UpdateWorldPose()
{
	if (m_ppkSkeletonInst)
		if (*m_ppkSkeletonInst != this)
			return;

	static CGrannyLocalPose s_SharedLocalPose;

	granny_skeleton * pgrnSkeleton = GrannyGetSourceSkeleton(m_pgrnModelInstance);
	granny_local_pose * pgrnLocalPose = s_SharedLocalPose.Get(pgrnSkeleton->BoneCount);

	const float * pAttachBoneMatrix = (mc_pParentInstance) ? mc_pParentInstance->GetBoneMatrixPointer(m_iParentBoneIndex) : nullptr;

	GrannySampleModelAnimationsAccelerated(m_pgrnModelInstance, pgrnSkeleton->BoneCount, pAttachBoneMatrix, pgrnLocalPose,
										   __GetWorldPosePtr());
	GrannyFreeCompletedModelControls(m_pgrnModelInstance);
}

void CGrannyModelInstance::UpdateWorldMatrices(const D3DXMATRIX * c_pWorldMatrix)
{
	// NO_MESH_BUG_FIX
	if (m_meshMatrices.empty())
		return;
	// END_OF_NO_MESH_BUG_FIX

	assert(m_pModel != nullptr);
	assert(ms_lpd3dMatStack != nullptr);

	int meshCount = m_pModel->GetMeshCount();

	granny_matrix_4x4 * pgrnMatCompositeBuffer = GrannyGetWorldPoseComposite4x4Array(__GetWorldPosePtr());
	auto * boneMatrices = reinterpret_cast<D3DXMATRIX *>(pgrnMatCompositeBuffer);

	for (int i = 0; i < meshCount; ++i)
	{
		auto & rWorldMatrix = m_meshMatrices[i];

		const CGrannyMesh * pMesh = m_pModel->GetMeshPointer(i);

		// WORK

#if GrannyProductMinorVersion == 4
		int * boneIndices = __GetMeshBoneIndices(i);
#elif GrannyProductMinorVersion == 11 || GrannyProductMinorVersion == 9 || GrannyProductMinorVersion == 8 || GrannyProductMinorVersion == 7
		const granny_int32x * boneIndices = __GetMeshBoneIndices(i);
#else
#	error "unknown granny version"
#endif
		// END_OF_WORK

		if (pMesh->CanDeformPNTVertices())
			rWorldMatrix = *c_pWorldMatrix;
		else
		{
#if GrannyProductMinorVersion == 4
			int iBone = *boneIndices;
#elif GrannyProductMinorVersion == 11 || GrannyProductMinorVersion == 9 || GrannyProductMinorVersion == 8 || GrannyProductMinorVersion == 7
			const granny_int32x iBone = *boneIndices;
#else
#	error "unknown granny version"
#endif
			D3DXMatrixMultiply(&rWorldMatrix, &boneMatrices[iBone], c_pWorldMatrix);
		}
	}

#ifdef _TEST
	TEST_matWorld = *c_pWorldMatrix;
#endif
}

void CGrannyModelInstance::DeformPNTVertices(void * pvDest) const
{
	assert(m_pModel != nullptr);
	assert(m_pModel->CanDeformPNTVertices());

	// WORK
	m_pModel->DeformPNTVertices(pvDest, reinterpret_cast<D3DXMATRIX *>(GrannyGetWorldPoseComposite4x4Array(__GetWorldPosePtr())),
								m_vct_pgrnMeshBinding);
	// END_OF_WORK
}
