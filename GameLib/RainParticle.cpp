#include "StdAfx.h"

#ifdef ENABLE_ENVIRONMENT_RAIN
#include "RainParticle.h"

const float c_fRainDistance = 70000.0f;

std::vector<CRainParticle *> CRainParticle::ms_kVct_RainParticlePool;

void CRainParticle::SetCameraVertex(const D3DXVECTOR3 & rv3Up, const D3DXVECTOR3 & rv3Cross)
{
	m_v3Up = rv3Up * m_fHalfWidth;
	m_v3Cross = rv3Cross * m_fHalfHeight;
}

bool CRainParticle::IsActivate()
{
	return m_bActivate;
}

void CRainParticle::Update(float fElapsedTime, const D3DXVECTOR3 & c_rv3Pos)
{
	m_v3Position += m_v3Velocity * fElapsedTime;

	if (m_v3Position.z < c_rv3Pos.z - 500.0f)
		m_bActivate = false;
	else if (abs(m_v3Position.x - c_rv3Pos.x) > c_fRainDistance)
		m_bActivate = false;
	else if (abs(m_v3Position.y - c_rv3Pos.y) > c_fRainDistance)
		m_bActivate = false;
}

void CRainParticle::GetVerticies(SParticleVertex & rv3Vertex1, SParticleVertex & rv3Vertex2, SParticleVertex & rv3Vertex3,
								 SParticleVertex & rv3Vertex4)
{
	rv3Vertex1.v3Pos = m_v3Position - m_v3Cross - m_v3Up;
	rv3Vertex1.u = 0.0f;
	rv3Vertex1.v = 0.0f;

	rv3Vertex2.v3Pos = m_v3Position + m_v3Cross - m_v3Up;
	rv3Vertex2.u = 1.0f;
	rv3Vertex2.v = 0.0f;

	rv3Vertex3.v3Pos = m_v3Position - m_v3Cross + m_v3Up;
	rv3Vertex3.u = 0.0f;
	rv3Vertex3.v = 1.0f;

	rv3Vertex4.v3Pos = m_v3Position + m_v3Cross + m_v3Up;
	rv3Vertex4.u = 1.0f;
	rv3Vertex4.v = 1.0f;
}

void CRainParticle::Init(const D3DXVECTOR3 & c_rv3Pos)
{
	float fRot = frandom(0.0f, 36000.0f) / 100.0f;
	float fDistance = frandom(0.0f, c_fRainDistance) / 10.0f;

	m_v3Position.x = c_rv3Pos.x + fDistance * sin((double) D3DXToRadian(fRot));
	m_v3Position.y = c_rv3Pos.y + fDistance * cos((double) D3DXToRadian(fRot));
	m_v3Position.z = c_rv3Pos.z + frandom(1500.0f, 2000.0f);
	m_v3Velocity.x = 0.0f;
	m_v3Velocity.y = 0.0f;
	m_v3Velocity.z = frandom(-50.0f, -200.0f);
	m_fHalfWidth = frandom(2.0f, 7.0f);
	m_fHalfHeight = m_fHalfWidth;
	m_bActivate = true;
	m_bChangedSize = false;

	m_fPeriod = frandom(1.5f, 5.0f);
	m_fcurRadian = frandom(-1.6f, 1.6f);
	m_fAmplitude = frandom(1.0f, 3.0f);
}

CRainParticle * CRainParticle::New()
{
	if (ms_kVct_RainParticlePool.empty())
		return new CRainParticle;

	CRainParticle * pParticle = ms_kVct_RainParticlePool.back();
	ms_kVct_RainParticlePool.pop_back();
	return pParticle;
}

void CRainParticle::Delete(CRainParticle * pRainParticle)
{
	ms_kVct_RainParticlePool.emplace_back(pRainParticle);
}

void CRainParticle::DestroyPool()
{
	stl_wipe(ms_kVct_RainParticlePool);
}

CRainParticle::CRainParticle() = default;
CRainParticle::~CRainParticle() = default;
#endif
