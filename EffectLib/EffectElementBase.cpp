#include "StdAfx.h"
#include "EffectElementBase.h"


void CEffectElementBase::GetPosition(float fTime, D3DXVECTOR3 & rPosition)
{
	if (m_TimeEventTablePosition.empty())
	{
		rPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		return;
	}
	if (m_TimeEventTablePosition.size() == 1)
	{
		rPosition = m_TimeEventTablePosition[0].m_vecPosition;
		return;
	}
	if (m_TimeEventTablePosition.front().m_fTime > fTime)
	{
		rPosition = m_TimeEventTablePosition.front().m_vecPosition;
		return;
	}
	if (m_TimeEventTablePosition.back().m_fTime < fTime)
	{
		rPosition = m_TimeEventTablePosition.back().m_vecPosition;
		return;
	}

	const auto result = std::lower_bound(m_TimeEventTablePosition.begin(), m_TimeEventTablePosition.end(), fTime);

	TEffectPosition & rEffectPosition = *result;
	auto rPrev = result;
	if (m_TimeEventTablePosition.begin() != result)
		rPrev = result - 1;
	else
	{
		rPosition = result->m_vecPosition;
		return;
	}
	TEffectPosition & rPrevEffectPosition = *rPrev;
	const auto iMovingType = rPrevEffectPosition.m_iMovingType;

	if (MOVING_TYPE_DIRECT == iMovingType)
	{
		const auto Head = fabs(rEffectPosition.m_fTime - fTime) / fabs(rEffectPosition.m_fTime - rPrevEffectPosition.m_fTime);
		const auto Tail = 1.0f - fabs(rEffectPosition.m_fTime - fTime) / fabs(rEffectPosition.m_fTime - rPrevEffectPosition.m_fTime);
		rPosition = (rPrevEffectPosition.m_vecPosition * Head) + (rEffectPosition.m_vecPosition * Tail);
	}
	else if (MOVING_TYPE_BEZIER_CURVE == iMovingType)
	{
		const auto ft = (fTime - rPrevEffectPosition.m_fTime) / (rEffectPosition.m_fTime - rPrevEffectPosition.m_fTime);

		rPosition = rPrevEffectPosition.m_vecPosition * (1.0f - ft) * (1.0f - ft) +
			(rPrevEffectPosition.m_vecPosition + rPrevEffectPosition.m_vecControlPoint) * (1.0f - ft) * ft * 2 +
			rEffectPosition.m_vecPosition * ft * ft;
	}
}

bool CEffectElementBase::isData()
{
	return OnIsData();
}

void CEffectElementBase::Clear()
{
	m_fStartTime = 0.0f;

	OnClear();
}

BOOL CEffectElementBase::LoadScript(CTextFileLoader & rTextFileLoader)
{
	CTokenVector * pTokenVector;
	if (!rTextFileLoader.GetTokenFloat("starttime", &m_fStartTime))
		m_fStartTime = 0.0f;
	if (rTextFileLoader.GetTokenVector("timeeventposition", &pTokenVector))
	{
		m_TimeEventTablePosition.clear();

		uint32_t dwIndex = 0;
		for (uint32_t i = 0; i < pTokenVector->size(); ++dwIndex)
		{
			TEffectPosition EffectPosition;
			EffectPosition.m_fTime = atof(pTokenVector->at(i++).c_str());
			if (pTokenVector->at(i) == "MOVING_TYPE_BEZIER_CURVE")
			{
				i++;

				EffectPosition.m_iMovingType = MOVING_TYPE_BEZIER_CURVE;

				EffectPosition.m_vecPosition.x = atof(pTokenVector->at(i++).c_str());
				EffectPosition.m_vecPosition.y = atof(pTokenVector->at(i++).c_str());
				EffectPosition.m_vecPosition.z = atof(pTokenVector->at(i++).c_str());

				EffectPosition.m_vecControlPoint.x = atof(pTokenVector->at(i++).c_str());
				EffectPosition.m_vecControlPoint.y = atof(pTokenVector->at(i++).c_str());
				EffectPosition.m_vecControlPoint.z = atof(pTokenVector->at(i++).c_str());
			}
			else if (pTokenVector->at(i) == "MOVING_TYPE_DIRECT")
			{
				i++;

				EffectPosition.m_iMovingType = MOVING_TYPE_DIRECT;

				EffectPosition.m_vecPosition.x = atof(pTokenVector->at(i++).c_str());
				EffectPosition.m_vecPosition.y = atof(pTokenVector->at(i++).c_str());
				EffectPosition.m_vecPosition.z = atof(pTokenVector->at(i++).c_str());

				EffectPosition.m_vecControlPoint = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
			}
			else
				return FALSE;

			m_TimeEventTablePosition.emplace_back(EffectPosition);
		}
	}

	return OnLoadScript(rTextFileLoader);
}

float CEffectElementBase::GetStartTime() const
{
	return m_fStartTime;
}

CEffectElementBase::CEffectElementBase()
{
	m_fStartTime = 0.0f;
}
