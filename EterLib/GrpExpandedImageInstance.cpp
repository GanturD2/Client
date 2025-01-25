#include "StdAfx.h"
#include "../EterBase/CRC32.h"
#include "GrpExpandedImageInstance.h"
#include "StateManager.h"

CDynamicPool<CGraphicExpandedImageInstance> CGraphicExpandedImageInstance::ms_kPool;

void CGraphicExpandedImageInstance::CreateSystem(uint32_t uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicExpandedImageInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicExpandedImageInstance * CGraphicExpandedImageInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicExpandedImageInstance::Delete(CGraphicExpandedImageInstance * pkImgInst)
{
	pkImgInst->Destroy();
	ms_kPool.Free(pkImgInst);
}

void CGraphicExpandedImageInstance::OnRender()
{
	CGraphicImage * pImage = m_roImage.GetPointer();
	CGraphicTexture * pTexture = pImage->GetTexturePointer();

	const RECT & c_rRect = pImage->GetRectReference();
	float texReverseWidth = 1.0f / float(pTexture->GetWidth());
	float texReverseHeight = 1.0f / float(pTexture->GetHeight());
#ifdef ENABLE_INGAME_WIKI
	float su1 = (c_rRect.left + m_renderBox.left - m_RenderingRect.left_top - m_TextureRenderingRect.left) * texReverseWidth;
	float su2 = (c_rRect.left + m_renderBox.left - m_RenderingRect.left_bottom - m_TextureRenderingRect.left) * texReverseWidth;
	float sv1 = (c_rRect.top + m_renderBox.top - m_RenderingRect.top_left - m_TextureRenderingRect.top) * texReverseHeight;
	float sv2 = (c_rRect.top + m_renderBox.top - m_RenderingRect.top_right - m_TextureRenderingRect.top) * texReverseHeight;
	float eu1 = (c_rRect.left + m_RenderingRect.right_top + m_TextureRenderingRect.right + (c_rRect.right - c_rRect.left - m_TextureRenderingRect.left) - m_renderBox.right) * texReverseWidth;
	float eu2 = (c_rRect.left + m_RenderingRect.right_bottom + m_TextureRenderingRect.right + (c_rRect.right - c_rRect.left - m_TextureRenderingRect.left) - m_renderBox.right) * texReverseWidth;
	float ev1 = (c_rRect.top + m_RenderingRect.bottom_left + m_TextureRenderingRect.bottom + (c_rRect.bottom - c_rRect.top - m_TextureRenderingRect.top) - m_renderBox.bottom) * texReverseHeight;
	float ev2 = (c_rRect.top + m_RenderingRect.bottom_right + m_TextureRenderingRect.bottom + (c_rRect.bottom - c_rRect.top - m_TextureRenderingRect.top) - m_renderBox.bottom) * texReverseHeight;
#else
	float su = (c_rRect.left - m_RenderingRect.left) * texReverseWidth;
	float sv = (c_rRect.top - m_RenderingRect.top) * texReverseHeight;
	float eu = (c_rRect.left + m_RenderingRect.right + (c_rRect.right - c_rRect.left)) * texReverseWidth;
	float ev = (c_rRect.top + m_RenderingRect.bottom + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;
#endif

	TPDTVertex vertices[4];
	vertices[0].position.x	= m_v2Position.x-0.5f;
	vertices[0].position.y	= m_v2Position.y-0.5f;
	vertices[0].position.z	= m_fDepth;
#ifdef ENABLE_INGAME_WIKI
	vertices[0].texCoord	= TTextureCoordinate(su1, sv1);
#else
	vertices[0].texCoord	= TTextureCoordinate(su, sv);
#endif
	vertices[0].diffuse		= m_DiffuseColor;

	vertices[1].position.x	= m_v2Position.x-0.5f;
	vertices[1].position.y	= m_v2Position.y-0.5f;
	vertices[1].position.z	= m_fDepth;
#ifdef ENABLE_INGAME_WIKI
	vertices[1].texCoord	= TTextureCoordinate(eu1, sv2);
#else
	vertices[1].texCoord	= TTextureCoordinate(eu, sv);
#endif
	vertices[1].diffuse		= m_DiffuseColor;

	vertices[2].position.x	= m_v2Position.x-0.5f;
	vertices[2].position.y	= m_v2Position.y-0.5f;
	vertices[2].position.z	= m_fDepth;
#ifdef ENABLE_INGAME_WIKI
	vertices[2].texCoord	= TTextureCoordinate(su2, ev1);
#else
	vertices[2].texCoord	= TTextureCoordinate(su, ev);
#endif
	vertices[2].diffuse		= m_DiffuseColor;

	vertices[3].position.x	= m_v2Position.x-0.5f;
	vertices[3].position.y	= m_v2Position.y-0.5f;
	vertices[3].position.z	= m_fDepth;
#ifdef ENABLE_INGAME_WIKI
	vertices[3].texCoord	= TTextureCoordinate(eu2, ev2);
#else
	vertices[3].texCoord	= TTextureCoordinate(eu, ev);
#endif
	vertices[3].diffuse		= m_DiffuseColor;

	if (0.0f == m_fRotation)
	{
		float fimgWidth = float(pImage->GetWidth()) * m_v2Scale.x;
		float fimgHeight = float(pImage->GetHeight()) * m_v2Scale.y;

#ifdef ENABLE_INGAME_WIKI
		vertices[0].position.x -= m_RenderingRect.left_top - m_renderBox.left;
		vertices[0].position.y -= m_RenderingRect.top_left - m_renderBox.top;
		vertices[1].position.x += fimgWidth + m_RenderingRect.right_top - m_renderBox.right;
		vertices[1].position.y -= m_RenderingRect.top_right - m_renderBox.top;
		vertices[2].position.x -= m_RenderingRect.left_bottom - m_renderBox.left;
		vertices[2].position.y += fimgHeight + m_RenderingRect.bottom_left - m_renderBox.bottom;
		vertices[3].position.x += fimgWidth + m_RenderingRect.right_bottom - m_renderBox.right;
		vertices[3].position.y += fimgHeight + m_RenderingRect.bottom_right - m_renderBox.bottom;
#else
		vertices[0].position.x -= m_RenderingRect.left;
		vertices[0].position.y -= m_RenderingRect.top;
		vertices[1].position.x += fimgWidth + m_RenderingRect.right;
		vertices[1].position.y -= m_RenderingRect.top;
		vertices[2].position.x -= m_RenderingRect.left;
		vertices[2].position.y += fimgHeight + m_RenderingRect.bottom;
		vertices[3].position.x += fimgWidth + m_RenderingRect.right;
		vertices[3].position.y += fimgHeight + m_RenderingRect.bottom;
#endif

		if ((0.0f < m_v2Scale.x && 0.0f > m_v2Scale.y) || (0.0f > m_v2Scale.x && 0.0f < m_v2Scale.y))
			STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

#ifdef ENABLE_OFFICAL_FEATURES
		if (m_bLeftRightReverse)
		{
#ifdef ENABLE_INGAME_WIKI
			vertices[0].texCoord = TTextureCoordinate(eu1, sv1);
			vertices[1].texCoord = TTextureCoordinate(su1, sv1);
			vertices[2].texCoord = TTextureCoordinate(eu1, ev1);
			vertices[3].texCoord = TTextureCoordinate(su1, ev1);
#else
			vertices[0].texCoord = TTextureCoordinate(eu, sv);
			vertices[1].texCoord = TTextureCoordinate(su, sv);
			vertices[2].texCoord = TTextureCoordinate(eu, ev);
			vertices[3].texCoord = TTextureCoordinate(su, ev);
#endif
		}
#endif
	}
	else
	{
		float fimgHalfWidth = float(pImage->GetWidth()) / 2.0f * m_v2Scale.x;
		float fimgHalfHeight = float(pImage->GetHeight()) / 2.0f * m_v2Scale.y;

		for (auto & vertice : vertices)
		{
			vertice.position.x += m_v2Origin.x;
			vertice.position.y += m_v2Origin.y;
		}

		float fRadian = D3DXToRadian(m_fRotation);
		vertices[0].position.x += (-fimgHalfWidth * cosf(fRadian)) - (-fimgHalfHeight * sinf(fRadian));
		vertices[0].position.y += (-fimgHalfWidth * sinf(fRadian)) + (-fimgHalfHeight * cosf(fRadian));
		vertices[1].position.x += (+fimgHalfWidth * cosf(fRadian)) - (-fimgHalfHeight * sinf(fRadian));
		vertices[1].position.y += (+fimgHalfWidth * sinf(fRadian)) + (-fimgHalfHeight * cosf(fRadian));
		vertices[2].position.x += (-fimgHalfWidth * cosf(fRadian)) - (+fimgHalfHeight * sinf(fRadian));
		vertices[2].position.y += (-fimgHalfWidth * sinf(fRadian)) + (+fimgHalfHeight * cosf(fRadian));
		vertices[3].position.x += (+fimgHalfWidth * cosf(fRadian)) - (+fimgHalfHeight * sinf(fRadian));
		vertices[3].position.y += (+fimgHalfWidth * sinf(fRadian)) + (+fimgHalfHeight * cosf(fRadian));
	}

	switch (m_iRenderingMode)
	{
		case RENDERING_MODE_SCREEN:
		case RENDERING_MODE_COLOR_DODGE:
			STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
			STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case RENDERING_MODE_MODULATE:
			STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			break;
	}

	// 2004.11.18.myevan.ctrl+alt+del 반복 사용시 튕기는 문제
	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
		STATEMANAGER.SetTexture(1, nullptr);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
	//STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16, vertices, sizeof(TPDTVertex));
	/////////////////////////////////////////////////////////////

	switch (m_iRenderingMode)
	{
		case RENDERING_MODE_SCREEN:
		case RENDERING_MODE_COLOR_DODGE:
		case RENDERING_MODE_MODULATE:
			STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
			STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
			break;
	}
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
}

void CGraphicExpandedImageInstance::SetDepth(float fDepth)
{
	m_fDepth = fDepth;
}

void CGraphicExpandedImageInstance::SetOrigin()
{
	SetOrigin(float(GetWidth()) / 2.0f, float(GetHeight()) / 2.0f);
}

void CGraphicExpandedImageInstance::SetOrigin(float fx, float fy)
{
	m_v2Origin.x = fx;
	m_v2Origin.y = fy;
}

void CGraphicExpandedImageInstance::SetRotation(float fRotation)
{
	m_fRotation = fRotation;
}

void CGraphicExpandedImageInstance::SetScale(float fx, float fy)
{
	m_v2Scale.x = fx;
	m_v2Scale.y = fy;
}

void CGraphicExpandedImageInstance::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
	if (IsEmpty())
		return;

#ifdef ENABLE_INGAME_WIKI
	SetExpandedRenderingRect(fLeft, fLeft, fTop, fTop, fRight, fRight, fBottom, fBottom);
#else
	auto fWidth = float(GetWidth());
	auto fHeight = float(GetHeight());

	m_RenderingRect.left = fWidth * fLeft;
	m_RenderingRect.top = fHeight * fTop;
	m_RenderingRect.right = fWidth * fRight;
	m_RenderingRect.bottom = fHeight * fBottom;
#endif
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
void CGraphicExpandedImageInstance::SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom)
{
	if (IsEmpty())
		return;

#ifdef ENABLE_INGAME_WIKI
	SetExpandedRenderingRect(fLeft, fLeft, fTop, fTop, fRight, fRight, fBottom, fBottom);
#else
	float fWidth = float(GetWidth() * m_v2Scale.x);
	float fHeight = float(GetHeight() * m_v2Scale.y);

	m_RenderingRect.left = fWidth * fLeft;
	m_RenderingRect.top = fHeight * fTop;
	m_RenderingRect.right = fWidth * fRight;
	m_RenderingRect.bottom = fHeight * fBottom;
#endif
}
#endif

void CGraphicExpandedImageInstance::SetRenderingMode(int iMode)
{
	m_iRenderingMode = iMode;
}

#ifdef ENABLE_12ZI
void CGraphicExpandedImageInstance::RenderCoolTime(float fCoolTime)
{
	if (IsEmpty())
		return;

	assert(!IsEmpty());

	OnRenderCoolTime(fCoolTime);
}

void CGraphicExpandedImageInstance::OnRenderCoolTime(float fCoolTime)
{
	if (fCoolTime >= 1.0f)
		fCoolTime = 1.0f;

	CGraphicImage * pImage = m_roImage.GetPointer();
	CGraphicTexture * pTexture = pImage->GetTexturePointer();

	const RECT& c_rRect = pImage->GetRectReference();
	float texReverseWidth = 1.0f / float(pTexture->GetWidth());
	float texReverseHeight = 1.0f / float(pTexture->GetHeight());

#ifdef ENABLE_INGAME_WIKI
	float su = (c_rRect.left + m_renderBox.left - m_RenderingRect.left_top - m_TextureRenderingRect.left) * texReverseWidth;
	float sv = (c_rRect.top + m_renderBox.top - m_RenderingRect.top_left - m_TextureRenderingRect.top) * texReverseHeight;
	float eu = (c_rRect.left + m_RenderingRect.right_top + m_TextureRenderingRect.right + (c_rRect.right - c_rRect.left - m_TextureRenderingRect.left) - m_renderBox.right) * texReverseWidth;
	float ev = (c_rRect.top + m_RenderingRect.bottom_left + m_TextureRenderingRect.bottom + (c_rRect.bottom - c_rRect.top - m_TextureRenderingRect.top) - m_renderBox.bottom) * texReverseHeight;
#else
	float su = (c_rRect.left - m_RenderingRect.left) * texReverseWidth;
	float sv = (c_rRect.top - m_RenderingRect.top) * texReverseHeight;
	float eu = (c_rRect.right + m_RenderingRect.right) * texReverseWidth;
	float ev = (c_rRect.bottom + m_RenderingRect.bottom) * texReverseHeight;
#endif

	float fimgWidth = c_rRect.right - c_rRect.left;
	float fimgHeight = c_rRect.bottom - c_rRect.top;
	float fimgWidthHalf = fimgWidth * 0.5f;
	float fimgHeightHalf = fimgHeight * 0.5f;

	float fxCenter = m_v2Position.x - 0.5f + fimgWidthHalf;
	float fyCenter = m_v2Position.y - 0.5f + fimgHeightHalf;

	if (fCoolTime < 1.0f)
	{
		if (fCoolTime < 0.0)
			fCoolTime = 0.0;

		const int c_iTriangleCountPerBox = 8;
		D3DXVECTOR2 v2BoxPos[c_iTriangleCountPerBox] =
		{
			D3DXVECTOR2(-1.0f, -1.0f),
			D3DXVECTOR2(-1.0f,  0.0f),
			D3DXVECTOR2(-1.0f, +1.0f),
			D3DXVECTOR2( 0.0f, +1.0f),
			D3DXVECTOR2(+1.0f, +1.0f),
			D3DXVECTOR2(+1.0f,  0.0f),
			D3DXVECTOR2(+1.0f, -1.0f),
			D3DXVECTOR2( 0.0f, -1.0f),
		};

		D3DXVECTOR2 v2TexPos[c_iTriangleCountPerBox] =
		{
			D3DXVECTOR2(su,  sv),
			D3DXVECTOR2(su, 0.5f * (ev + sv)),
			D3DXVECTOR2(su,  ev),
			D3DXVECTOR2(0.5f * (su + eu),  ev),
			D3DXVECTOR2(eu,  ev),
			D3DXVECTOR2(eu, 0.5f * (ev + sv)),
			D3DXVECTOR2(eu,  sv),
			D3DXVECTOR2(0.5f * (su + eu),  sv),
		};

		int iTriCount = int(8.0f - 8.0f * fCoolTime);
		float fLastPercentage = (8.0f - 8.0f * fCoolTime) - iTriCount;

		std::vector<TPDTVertex> vertices;
		TPDTVertex vertex;
		vertex.position = TPosition(fxCenter, fyCenter, m_fDepth);
		vertex.texCoord = TTextureCoordinate(0.5f * (su + eu), 0.5f * (ev + sv));
		vertex.diffuse = m_DiffuseColor;
		vertices.emplace_back(vertex);

		vertex.position = TPosition(fxCenter, fyCenter - fimgHeightHalf, m_fDepth);
		vertex.texCoord = TTextureCoordinate(0.5f * (su + eu), sv);
		vertex.diffuse = m_DiffuseColor;
		vertices.emplace_back(vertex);

		if (iTriCount > 0)
		{
			for (int j = 0; j < iTriCount; ++j)
			{
				vertex.position = TPosition(fxCenter + (v2BoxPos[j].x * fimgWidthHalf),
											fyCenter + (v2BoxPos[j].y * fimgHeightHalf),
											m_fDepth);
				vertex.texCoord = TTextureCoordinate(v2TexPos[j & (c_iTriangleCountPerBox - 1)].x,
													 v2TexPos[j & (c_iTriangleCountPerBox - 1)].y);
				vertex.diffuse = m_DiffuseColor;
				vertices.emplace_back(vertex);
			}
		}

		if (fLastPercentage > 0.0f)
		{
			D3DXVECTOR2 * pv2Pos;
			D3DXVECTOR2 * pv2LastPos;
			assert((iTriCount+8)%8>=0&&(iTriCount+8)%8<8);
			assert((iTriCount+7)%8>=0&&(iTriCount+7)%8<8);
			pv2LastPos=&v2BoxPos[(iTriCount+8)%8];
			pv2Pos=&v2BoxPos[(iTriCount+7)%8];
			float fxShit = (pv2LastPos->x - pv2Pos->x) * fLastPercentage + pv2Pos->x;
			float fyShit = (pv2LastPos->y - pv2Pos->y) * fLastPercentage + pv2Pos->y;
			vertex.position = TPosition(fimgWidthHalf * fxShit + fxCenter + 0.5f,
										fimgHeightHalf * fyShit + fyCenter + 0.5f,
										m_fDepth);
			vertex.texCoord = TTextureCoordinate(su + 0.5f * (eu - su) + fxShit * 0.5f * (eu - su),
												 sv + 0.5f * (ev - sv) + fyShit * 0.5f * (ev - sv));

			vertex.diffuse = m_DiffuseColor;
			vertices.emplace_back(vertex);
			++iTriCount;
		}

		if (vertices.empty())
			return;

		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

		switch (m_iRenderingMode)
		{
			case RENDERING_MODE_SCREEN:
			case RENDERING_MODE_COLOR_DODGE:
				STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
				STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				break;
			case RENDERING_MODE_MODULATE:
				STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
				break;
		}

		if (CGraphicBase::SetPDTStream(&vertices[0], vertices.size()))
		{
			CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_TRI);
			STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
			STATEMANAGER.SetTexture(1, nullptr);
			STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
			STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
		}
		switch (m_iRenderingMode)
		{
			case RENDERING_MODE_SCREEN:
			case RENDERING_MODE_COLOR_DODGE:
			case RENDERING_MODE_MODULATE:
				STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
				STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
				break;
		}
		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
}
#endif

#ifdef ENABLE_OFFICAL_FEATURES
void CGraphicExpandedImageInstance::LeftRightReverse()
{
	m_bLeftRightReverse = true;
}
#endif

#ifdef ENABLE_INGAME_WIKI
void CGraphicExpandedImageInstance::SetExpandedRenderingRect(float fLeftTop, float fLeftBottom, float fTopLeft, float fTopRight, float fRightTop, float fRightBottom, float fBottomLeft, float fBottomRight)
{
	if (IsEmpty())
		return;

	float fWidth = float(GetWidth() * m_v2Scale.x);
	float fHeight = float(GetHeight() * m_v2Scale.y);
	//float fWidth = float(GetWidth());
	//float fHeight = float(GetHeight());

	m_RenderingRect.left_top = fWidth * fLeftTop;
	m_RenderingRect.left_bottom = fWidth * fLeftBottom;
	m_RenderingRect.top_left = fHeight * fTopLeft;
	m_RenderingRect.top_right = fHeight * fTopRight;
	m_RenderingRect.right_top = fWidth * fRightTop;
	m_RenderingRect.right_bottom = fWidth * fRightBottom;
	m_RenderingRect.bottom_left = fHeight * fBottomLeft;
	m_RenderingRect.bottom_right = fHeight * fBottomRight;
}

void CGraphicExpandedImageInstance::iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom)
{
	if (IsEmpty())
		return;

	iSetExpandedRenderingRect(iLeft, iLeft, iTop, iTop, iRight, iRight, iBottom, iBottom);
}

void CGraphicExpandedImageInstance::iSetExpandedRenderingRect(int iLeftTop, int iLeftBottom, int iTopLeft, int iTopRight, int iRightTop, int iRightBottom, int iBottomLeft, int iBottomRight)
{
	if (IsEmpty())
		return;

	m_RenderingRect.left_top = iLeftTop;
	m_RenderingRect.left_bottom = iLeftBottom;
	m_RenderingRect.top_left = iTopLeft;
	m_RenderingRect.top_right = iTopRight;
	m_RenderingRect.right_top = iRightTop;
	m_RenderingRect.right_bottom = iRightBottom;
	m_RenderingRect.bottom_left = iBottomLeft;
	m_RenderingRect.bottom_right = iBottomRight;
}

void CGraphicExpandedImageInstance::SetTextureRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
	if (IsEmpty())
		return;

	float fWidth = float(GetWidth());
	float fHeight = float(GetHeight());

	m_TextureRenderingRect.left = fWidth * fLeft;
	m_TextureRenderingRect.top = fHeight * fTop;
	m_TextureRenderingRect.right = fWidth * fRight;
	m_TextureRenderingRect.bottom = fHeight * fBottom;
}


int CGraphicExpandedImageInstance::GetRenderWidth()
{
	return GetWidth() * m_v2Scale.x;
}

int CGraphicExpandedImageInstance::GetRenderHeight()
{
	return GetHeight() * m_v2Scale.y;
}

void CGraphicExpandedImageInstance::SaveColorMap()
{
	if (m_pColorMap)
		delete[] m_pColorMap;

	if (GetWidth() == 0 || GetHeight() == 0)
		return;

	CGraphicImage* pImage = m_roImage.GetPointer();
	CGraphicTexture* pTexture = pImage->GetTexturePointer();

	D3DLOCKED_RECT lockedRect;
	HRESULT hr = pTexture->GetD3DTexture()->LockRect(0, &lockedRect, nullptr, 0);
	if (hr != D3D_OK)
	{
		TraceError("Could not save color map (result %u)", hr);
		return;
	}

	m_pColorMap = new uint32_t[GetWidth() * GetHeight()];

	// read colors
	for (uint32_t y = 0; y < GetHeight(); ++y)
	{
		for (uint32_t x = 0; x < GetWidth(); ++x)
		{
			uint32_t dwIndex = x * 4 + y * lockedRect.Pitch;
			m_pColorMap[y * GetWidth() + x] = *(uint32_t*)(&((uint8_t*)lockedRect.pBits)[dwIndex]);
		}
	}

	pTexture->GetD3DTexture()->UnlockRect(0);
}

uint32_t CGraphicExpandedImageInstance::GetPixelColor(uint32_t x, uint32_t y)
{
	if (!m_pColorMap)
		SaveColorMap();

	return m_pColorMap[y * GetWidth() + x];
}
#endif

uint32_t CGraphicExpandedImageInstance::Type()
{
	static uint32_t s_dwType = GetCRC32("CGraphicExpandedImageInstance", strlen("CGraphicExpandedImageInstance"));
	return (s_dwType);
}

void CGraphicExpandedImageInstance::OnSetImagePointer()
{
	if (IsEmpty())
		return;

#ifdef ENABLE_INGAME_WIKI
	if (m_pColorMap)
	{
		delete[] m_pColorMap;
		m_pColorMap = nullptr;
	}
#endif

	SetOrigin(float(GetWidth()) / 2.0f, float(GetHeight()) / 2.0f);
}

BOOL CGraphicExpandedImageInstance::OnIsType(uint32_t dwType)
{
	if (CGraphicExpandedImageInstance::Type() == dwType)
		return TRUE;

	return CGraphicImageInstance::IsType(dwType);
}

void CGraphicExpandedImageInstance::Initialize()
{
	m_iRenderingMode = RENDERING_MODE_NORMAL;
	m_fDepth = 0.0f;
	m_v2Origin.x = m_v2Origin.y = 0.0f;
	m_v2Scale.x = m_v2Scale.y = 1.0f;
	m_fRotation = 0.0f;
#ifdef ENABLE_INGAME_WIKI
	m_RenderingRect = {};
	memset(&m_TextureRenderingRect, 0, sizeof(RECT));
	m_pColorMap = nullptr;

	memset(&m_renderBox, 0, sizeof(m_renderBox));
#else
	m_RenderingRect = {};
#endif
#ifdef ENABLE_OFFICAL_FEATURES
	m_bLeftRightReverse = false;
#endif
}

#ifdef ENABLE_INGAME_WIKI
void CGraphicExpandedImageInstance::SetRenderBox(RECT& renderBox)
{
	memcpy(&m_renderBox, &renderBox, sizeof(m_renderBox));
}
#endif

void CGraphicExpandedImageInstance::Destroy()
{
	CGraphicImageInstance::Destroy();

#ifdef ENABLE_INGAME_WIKI
	if (m_pColorMap)
		delete[] m_pColorMap;
#endif

	Initialize();
}

CGraphicExpandedImageInstance::CGraphicExpandedImageInstance()
{
	Initialize();
}

CGraphicExpandedImageInstance::~CGraphicExpandedImageInstance()
{
	Destroy();
}
