#include "StdAfx.h"
#include "../EterLib/StateManager.h"

#include "ActorInstance.h"

#ifdef ENABLE_RENDER_TARGET_EFFECT
#	include "../EffectLib/EffectManager.h"
#endif

bool CActorInstance::ms_isDirLine = false;

bool CActorInstance::IsDirLine()
{
	return ms_isDirLine;
}

void CActorInstance::ShowDirectionLine(bool isVisible)
{
	ms_isDirLine = isVisible;
}

void CActorInstance::SetMaterialColor(uint32_t dwColor)
{
	if (m_pkHorse)
		m_pkHorse->SetMaterialColor(dwColor);

	m_dwMtrlColor &= 0xff000000;
	m_dwMtrlColor |= (dwColor & 0x00ffffff);
}

void CActorInstance::SetMaterialAlpha(uint32_t dwAlpha)
{
	m_dwMtrlAlpha = dwAlpha;
}


void CActorInstance::OnRender()
{
	D3DMATERIAL9 kMtrl;
	STATEMANAGER.GetMaterial(&kMtrl);

	kMtrl.Diffuse = D3DXCOLOR(m_dwMtrlColor);
	STATEMANAGER.SetMaterial(&kMtrl);

	// 현재는 이렇게.. 최종적인 형태는 Diffuse와 Blend의 분리로..
	// 아니면 이런 형태로 가되 Texture & State Sorting 지원으로.. - [levites]
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	switch (m_iRenderMode)
	{
		case RENDER_MODE_NORMAL:
			BeginDiffuseRender();
			RenderWithOneTexture();
			EndDiffuseRender();
			BeginOpacityRender();
			BlendRenderWithOneTexture();
			EndOpacityRender();
			break;
		case RENDER_MODE_BLEND:
			if (m_fAlphaValue == 1.0f)
			{
				BeginDiffuseRender();
				RenderWithOneTexture();
				EndDiffuseRender();
				BeginOpacityRender();
				BlendRenderWithOneTexture();
				EndOpacityRender();
			}
			else if (m_fAlphaValue > 0.0f)
			{
				BeginBlendRender();
				RenderWithOneTexture();
				BlendRenderWithOneTexture();
				EndBlendRender();
			}
			break;
		case RENDER_MODE_ADD:
			BeginAddRender();
			RenderWithOneTexture();
			BlendRenderWithOneTexture();
			EndAddRender();
			break;
		case RENDER_MODE_MODULATE:
			BeginModulateRender();
			RenderWithOneTexture();
			BlendRenderWithOneTexture();
			EndModulateRender();
			break;
	}

	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

	kMtrl.Diffuse = D3DXCOLOR(0xffffffff);
	STATEMANAGER.SetMaterial(&kMtrl);

	if (ms_isDirLine)
	{
		D3DXVECTOR3 kD3DVt3Cur(m_x, m_y, m_z);

		D3DXVECTOR3 kD3DVt3LookDir(0.0f, -1.0f, 0.0f);
		D3DXMATRIX kD3DMatLook;
		D3DXMatrixRotationZ(&kD3DMatLook, D3DXToRadian(GetRotation()));
		D3DXVec3TransformCoord(&kD3DVt3LookDir, &kD3DVt3LookDir, &kD3DMatLook);
		D3DXVec3Scale(&kD3DVt3LookDir, &kD3DVt3LookDir, 200.0f);
		D3DXVec3Add(&kD3DVt3LookDir, &kD3DVt3LookDir, &kD3DVt3Cur);

		D3DXVECTOR3 kD3DVt3AdvDir(0.0f, -1.0f, 0.0f);
		D3DXMATRIX kD3DMatAdv;
		D3DXMatrixRotationZ(&kD3DMatAdv, D3DXToRadian(GetAdvancingRotation()));
		D3DXVec3TransformCoord(&kD3DVt3AdvDir, &kD3DVt3AdvDir, &kD3DMatAdv);
		D3DXVec3Scale(&kD3DVt3AdvDir, &kD3DVt3AdvDir, 200.0f);
		D3DXVec3Add(&kD3DVt3AdvDir, &kD3DVt3AdvDir, &kD3DVt3Cur);

		static CScreen s_kScreen;

		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

		s_kScreen.SetDiffuseColor(1.0f, 1.0f, 0.0f);
		s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, kD3DVt3Cur.z, kD3DVt3AdvDir.x, kD3DVt3AdvDir.y, kD3DVt3AdvDir.z);

		s_kScreen.SetDiffuseColor(0.0f, 1.0f, 1.0f);
		s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, kD3DVt3Cur.z, kD3DVt3LookDir.x, kD3DVt3LookDir.y, kD3DVt3LookDir.z);

		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);

		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
		STATEMANAGER.RestoreFVF();
	}
#ifdef ENABLE_RENDER_TARGET_EFFECT
	if (IsRenderTarget())
	{
		auto& rkEftMgr = CEffectManager::Instance(); 
		for (auto& it : m_AttachingEffectList)
		{
			rkEftMgr.SelectEffectInstance(it.dwEffectIndex);
			rkEftMgr.RenderSelectedEffect();
		}
	}
#endif
#ifdef ENABLE_RENDER_LOGIN_EFFECTS
	if (IsLoginRender())
	{
		auto& rkEftMgr = CEffectManager::Instance();
		for (auto& it : m_AttachingEffectList)
		{
			rkEftMgr.SelectEffectInstance(it.dwEffectIndex);
			rkEftMgr.RenderSelectedEffect();
		}
	}
#endif
}

void CActorInstance::BeginDiffuseRender()
{
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CActorInstance::EndDiffuseRender()
{
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CActorInstance::BeginOpacityRender()
{
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
}

void CActorInstance::EndOpacityRender()
{
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
}

void CActorInstance::BeginBlendRender()
{
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(1.0f, 1.0f, 1.0f, m_fAlphaValue));
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
}

void CActorInstance::EndBlendRender()
{
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}

void CActorInstance::BeginAddRender()
{
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_AddColor);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CActorInstance::EndAddRender()
{
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CActorInstance::RestoreRenderMode()
{
	// NOTE : This is temporary code. I wanna convert this code to that restore the mode to
	//        model's default setting which had has as like specular or normal. - [levites]
	m_iRenderMode = RENDER_MODE_NORMAL;
	if (m_kBlendAlpha.m_isBlending)
		m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
}


void CActorInstance::SetAddRenderMode()
{
	m_iRenderMode = RENDER_MODE_ADD;
	if (m_kBlendAlpha.m_isBlending)
		m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
}

void CActorInstance::SetRenderMode(int iRenderMode)
{
	m_iRenderMode = iRenderMode;
	if (m_kBlendAlpha.m_isBlending)
		m_kBlendAlpha.m_iOldRenderMode = iRenderMode;
}

void CActorInstance::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_AddColor = c_rColor;
	m_AddColor.a = 1.0f;
}

void CActorInstance::BeginModulateRender()
{
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_AddColor);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CActorInstance::EndModulateRender()
{
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CActorInstance::SetModulateRenderMode()
{
	m_iRenderMode = RENDER_MODE_MODULATE;
	if (m_kBlendAlpha.m_isBlending)
		m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
}

void CActorInstance::RenderCollisionData()
{
	static CScreen s_Screen;

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (m_pAttributeInstance)
	{
		for (uint32_t col = 0; col < GetCollisionInstanceCount(); ++col)
		{
			CBaseCollisionInstance * pInstance = GetCollisionInstanceData(col);
			pInstance->Render();
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_ZENABLE, FALSE);
	s_Screen.SetColorOperation();
	s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);	//Red
	TCollisionPointInstanceList::iterator itor;
	/*itor = m_AttackingPointInstanceList.begin();
	for (; itor != m_AttackingPointInstanceList.end(); ++itor)
	{
		const TCollisionPointInstance & c_rInstance = *itor;
		for (uint32_t i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
		{
			const CDynamicSphereInstance & c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
			s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x,
									c_rSphereInstance.v3Position.y,
									c_rSphereInstance.v3Position.z,
									c_rSphereInstance.fRadius);
		}
	}*/
	s_Screen.SetDiffuseColor(1.0f, (isShow()) ? 1.0f : 0.0f, 0.0f);	//Red
	D3DXVECTOR3 center;
	float r;
	GetBoundingSphere(center, r);
	s_Screen.RenderCircle3d(center.x, center.y, center.z, r);

	s_Screen.SetDiffuseColor(0.0f, 0.0f, 1.0f);	//Blue
	itor = m_DefendingPointInstanceList.begin();
	for (; itor != m_DefendingPointInstanceList.end(); ++itor)
	{
		const TCollisionPointInstance & c_rInstance = *itor;
		for (const auto & c_rSphereInstance : c_rInstance.SphereInstanceVector)
		{
			s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x, c_rSphereInstance.v3Position.y, c_rSphereInstance.v3Position.z,
									c_rSphereInstance.fRadius);
		}
	}

	s_Screen.SetDiffuseColor(0.0f, 1.0f, 0.0f);	//Green
	itor = m_BodyPointInstanceList.begin();
	for (; itor != m_BodyPointInstanceList.end(); ++itor)
	{
		const TCollisionPointInstance & c_rInstance = *itor;
		for (const auto & c_rSphereInstance : c_rInstance.SphereInstanceVector)
		{
			s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x, c_rSphereInstance.v3Position.y, c_rSphereInstance.v3Position.z,
									c_rSphereInstance.fRadius);
		}
	}

	s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
	//	if (m_SplashArea.fDisappearingTime > GetLocalTime())
	{
		auto itor = m_kSplashArea.SphereInstanceVector.begin();
		for (; itor != m_kSplashArea.SphereInstanceVector.end(); ++itor)
		{
			const CDynamicSphereInstance & c_rInstance = *itor;
			s_Screen.RenderCircle3d(c_rInstance.v3Position.x, c_rInstance.v3Position.y, c_rInstance.v3Position.z, c_rInstance.fRadius);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_ZENABLE, TRUE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
}

void CActorInstance::RenderCollisionDataNew()
{
	static CScreen s_Screen;
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);

	STATEMANAGER.GetTransform(D3DTS_WORLD, &matWorld);

	CRaceMotionData* pMotionData;
	if (!m_pkCurRaceData->GetMotionDataPointer(m_kCurMotNode.dwMotionKey, &pMotionData))
		return;

	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xffff0000);

	s_Screen.SetColorOperation();
	s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);

	float zTest = 0.0f;

	//if (m_pkHorse)
	//{
	//	int boneIndex;
	//	if (m_pkHorse->FindBoneIndex(0, "saddle", &boneIndex))
	//	{
	//		D3DXMATRIX* pBoneMat;
	//		if (m_pkHorse->GetBoneMatrix(0, boneIndex, &pBoneMat))
	//			//zTest = pBoneMat->_43;
	//			zTest = 100.0f;
	//	}
	//}

	float c = cosf(D3DXToRadian(GetRotation()));
	float s = sinf(D3DXToRadian(GetRotation()));

	if (pMotionData->isAttackingMotion())
	{
		const NRaceData::TMotionAttackData& c_rMotionAttackData = pMotionData->GetMotionAttackDataReference();

		for (uint32_t m_dwcurHitDataIndex = 0; m_dwcurHitDataIndex < c_rMotionAttackData.HitDataContainer.size(); ++m_dwcurHitDataIndex)
		{
			const NRaceData::THitData& c_rHitData = c_rMotionAttackData.HitDataContainer[m_dwcurHitDataIndex];
			if (!c_rHitData.mapHitPosition.empty())
			{
				NRaceData::THitTimePositionMap::const_iterator it;
				//it = c_rHitData.mapHitPosition.find(m_currMotionIndex);
				for (it = c_rHitData.mapHitPosition.begin(); it != c_rHitData.mapHitPosition.end(); ++it)
					//if (it != c_rHitData.mapHitPosition.end())
				{
					D3DXVECTOR3 v3DstEnd;
					D3DXVECTOR3 v3DstBegin;
					{
						D3DXVECTOR3 v3SrcDir = it->second.v3Position - it->second.v3LastPosition;
						v3SrcDir *= __GetReachScale();

						const D3DXVECTOR3& v3Src = it->second.v3LastPosition + v3SrcDir;
						v3DstBegin = it->second.v3Position;
						v3DstBegin.x = v3Src.x * c - v3Src.y * s;
						v3DstBegin.y = v3Src.x * s + v3Src.y * c;
						v3DstBegin += GetPosition();
						v3DstBegin.z += zTest;
					}
					{
						const D3DXVECTOR3& v3Src = it->second.v3LastPosition;
						v3DstEnd = it->second.v3LastPosition;
						v3DstEnd.x = v3Src.x * c - v3Src.y * s;
						v3DstEnd.y = v3Src.x * s + v3Src.y * c;
						v3DstEnd += GetPosition();
						v3DstEnd.z += zTest;
					}

					s_Screen.RenderSphere(0,
						v3DstBegin.x,
						v3DstBegin.y,
						v3DstBegin.z,
						20.0f,
						D3DFILL_WIREFRAME);
					s_Screen.RenderSphere(0,
						v3DstEnd.x,
						v3DstEnd.y,
						v3DstEnd.z,
						20.0f,
						D3DFILL_WIREFRAME);

					/*D3DXVECTOR3 v3SrcDir = it->second.v3Position - it->second.v3LastPosition;
					v3SrcDir *= __GetReachScale();

					const D3DXVECTOR3& v3Src = it->second.v3LastPosition + v3SrcDir;
					s_Screen.RenderLine3d(it->second.v3LastPosition.x,
						it->second.v3LastPosition.y,
						it->second.v3LastPosition.z + zTest,
						v3Src.x,
						v3Src.y,
						v3Src.z + zTest);*/
				}
			}
		}
	}

	s_Screen.SetColorOperation();
	s_Screen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff00ff00);

	for (auto itor = m_DefendingPointInstanceList.begin(); itor != m_DefendingPointInstanceList.end(); ++itor)
	{
		const CDynamicSphereInstanceVector& c_DefendingSphereVector = itor->SphereInstanceVector;
		CDynamicSphereInstanceVector::const_iterator dsit;

		for (dsit = c_DefendingSphereVector.begin(); dsit != c_DefendingSphereVector.end(); ++dsit)
		{
			float zScale = GetScale().z;
			D3DXVECTOR3 newPos = dsit->v3Position - GetPosition();
			if (zScale > 1.0f)
				newPos = dsit->v3Position + newPos * sqrt(zScale);
			else if (zScale > 0.0f && zScale < 1.0f)
				newPos = dsit->v3Position - newPos * sqrt(zScale);
			else
				newPos = dsit->v3Position;

			s_Screen.RenderSphere(0, dsit->v3Position.x,
				dsit->v3Position.y,
				dsit->v3Position.z,
				dsit->fRadius, D3DFILL_WIREFRAME);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff0000ff);

	for (auto itor = m_BodyPointInstanceList.begin(); itor != m_BodyPointInstanceList.end(); ++itor)
	{
		const CDynamicSphereInstanceVector& c_DefendingSphereVector = itor->SphereInstanceVector;
		CDynamicSphereInstanceVector::const_iterator dsit;

		for (dsit = c_DefendingSphereVector.begin(); dsit != c_DefendingSphereVector.end(); ++dsit)
		{
			float zScale = GetScale().z;
			D3DXVECTOR3 newPos = dsit->v3Position - GetPosition();
			if (zScale > 1.0f)
				newPos = dsit->v3Position + newPos * sqrt(zScale);
			else if (zScale > 0.0f && zScale < 1.0f)
				newPos = dsit->v3Position - newPos * sqrt(zScale);
			else
				newPos = dsit->v3Position;

			s_Screen.RenderSphere(0, dsit->v3LastPosition.x,
				dsit->v3LastPosition.y,
				dsit->v3LastPosition.z,
				dsit->fRadius, D3DFILL_WIREFRAME);
		}
	}
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffFFFFff);
	s_Screen.RenderSphere(0, GetPosition().x, GetPosition().y, GetPosition().z, 300.0f * GetScale().z, D3DFILL_WIREFRAME);

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
	STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
}

void CActorInstance::RenderToShadowMap()
{
	if (RENDER_MODE_BLEND == m_iRenderMode)
		if (GetAlphaValue() < 0.5f)
			return;

	CGraphicThingInstance::RenderToShadowMap();

	if (m_pkHorse)
		m_pkHorse->RenderToShadowMap();
}
