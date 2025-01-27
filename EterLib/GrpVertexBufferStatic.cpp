#include "StdAfx.h"
#include "GrpVertexBufferStatic.h"

bool CStaticVertexBuffer::Create(int vtxCount, uint32_t fvf, bool /*isManaged*/)
{
	// 무조건 MANAGED 모드
	return CGraphicVertexBuffer::Create(vtxCount, fvf, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED);
}

CStaticVertexBuffer::CStaticVertexBuffer() = default;

CStaticVertexBuffer::~CStaticVertexBuffer() = default;
