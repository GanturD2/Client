#pragma once

#include "Type.h"
#include "../EterBase/Random.h"
#include "../EterLib/Pool.h"
class CParticleInstance;

namespace NEffectUpdateDecorator
{
class CDecoratorData
{
public:
	float fTime;
	float fElapsedTime;
	CParticleInstance * pInstance;

	CDecoratorData(float fTime, float fElapsedTime, CParticleInstance * pInstance)
		: fTime(fTime), fElapsedTime(fElapsedTime), pInstance(pInstance)
	{
	}
};

class CBaseDecorator
{
	friend class CParticleSystemData;

public:
	CBaseDecorator() = default;
	virtual ~CBaseDecorator() = default;
	CLASS_DELETE_COPYMOVE(CBaseDecorator);

	void Excute(const CDecoratorData & d)
	{
		CBaseDecorator * pd = this;
		while (pd)
		{
			CBaseDecorator * pNextDecorator = pd->m_NextDecorator;
			pd->__Excute(d);
			pd = pNextDecorator;
		}
	}

	CBaseDecorator * AddChainFront(CBaseDecorator * pd)
	{
		pd->m_NextDecorator = this;
		m_PrevDecorator = pd;
		return pd;
	}

	void DeleteThis() const
	{
		if (m_NextDecorator)
			m_NextDecorator->DeleteThis();
		delete this;
	}

	CBaseDecorator * Clone(CParticleInstance * pFirstInstance, CParticleInstance * pInstance)
	{
		CBaseDecorator * pNewDecorator = __Clone(pFirstInstance, pInstance);
		CBaseDecorator * pSrc = this;
		CBaseDecorator * pDest = pNewDecorator;
		while (pSrc->m_NextDecorator)
		{
			pDest->m_NextDecorator = pSrc->m_NextDecorator->__Clone(pFirstInstance, pInstance);
			pDest->m_NextDecorator->m_PrevDecorator = pDest;

			pSrc = pSrc->m_NextDecorator;
			pDest = pDest->m_NextDecorator;
		}
		return pNewDecorator;
	}

protected:
	virtual void __Excute(const CDecoratorData & d) = 0;
	virtual CBaseDecorator * __Clone(CParticleInstance * pFirstInstance, CParticleInstance * pInstance) = 0;

	void RemoveMe() const
	{
		m_PrevDecorator->m_NextDecorator = m_NextDecorator;
		m_NextDecorator->m_PrevDecorator = m_PrevDecorator;
		delete this;
	}

	CBaseDecorator * m_NextDecorator{nullptr};
	CBaseDecorator * m_PrevDecorator{nullptr};
};

class CHeaderDecorator : public CBaseDecorator, public CPooledObject<CHeaderDecorator>
{
public:
	CHeaderDecorator() = default;
	~CHeaderDecorator() = default;
	CLASS_DELETE_COPYMOVE(CHeaderDecorator);

protected:
	void __Excute(const CDecoratorData &) override {}
	CBaseDecorator * __Clone(CParticleInstance *, CParticleInstance *) override { return new CHeaderDecorator; }
};

class CNullDecorator : public CBaseDecorator, public CPooledObject<CNullDecorator>
{
public:
	CNullDecorator() = default;
	~CNullDecorator() = default;
	CLASS_DELETE_COPYMOVE(CNullDecorator);

protected:
	void __Excute(const CDecoratorData & d) override {}

	CBaseDecorator * __Clone(CParticleInstance *, CParticleInstance *) override { return new CNullDecorator; }
};

template <class T> class CTimeEventDecorator : public CBaseDecorator, public CPooledObject<CTimeEventDecorator<T>>
{
public:
	using TTimeEventType = CTimeEvent<T>;
	using TTimeEventContainerType = std::vector<TTimeEventType>;

	explicit CTimeEventDecorator(const TTimeEventContainerType & TimeEventContainer, T * pValue = nullptr)
		: it_start(TimeEventContainer.begin()),
		  it_end(TimeEventContainer.end()),
		  it_cur(TimeEventContainer.begin()),
		  it_next(TimeEventContainer.begin()),
		  pData(pValue)
	{
		if (it_start == it_end)
			*pValue = T();
		else
			++it_next;
	}

	~CTimeEventDecorator() {}

	void SetData(T * pValue) { pData = pValue; }

protected:
	CTimeEventDecorator(CTimeEventDecorator<T> & ted, CParticleInstance * pFirstInstance, CParticleInstance * pInstance)
		: it_start(ted.it_start),
		  it_end(ted.it_end),
		  it_cur(ted.it_cur),
		  it_next(ted.it_next),
		  pData((T *) ((unsigned char *) ted.pData - (uint32_t) pFirstInstance + (uint32_t) pInstance))
	{
		if (it_start == it_end)
			*pData = T();
	}

	CBaseDecorator * __Clone(CParticleInstance * pFirstInstance, CParticleInstance * pInstance)
	{
		return new CTimeEventDecorator(*this, pFirstInstance, pInstance);
	}

	void __Excute(const CDecoratorData & d)
	{
		if (it_start == it_end)
			RemoveMe();
		else if (it_cur->m_fTime > d.fTime)
			*pData = it_cur->m_Value;
		else
		{
			while (it_next != it_end && it_next->m_fTime <= d.fTime)
				++it_cur, ++it_next;
			if (it_next == it_end)
			{
				// setting value
				*pData = it_cur->m_Value;

				RemoveMe();
			}
			else
			{
				float length = it_next->m_fTime - it_cur->m_fTime;
				*pData = it_cur->m_Value * (1 - (d.fTime - it_cur->m_fTime) / length);
				*pData += it_next->m_Value * ((d.fTime - it_cur->m_fTime) / length);
			}
		}
	}

	typename TTimeEventContainerType::const_iterator it_start;
	typename TTimeEventContainerType::const_iterator it_end;
	typename TTimeEventContainerType::const_iterator it_cur;
	typename TTimeEventContainerType::const_iterator it_next;
	T * pData;
};

using CScaleValueDecorator = CTimeEventDecorator<float>;
using CColorValueDecorator = CTimeEventDecorator<float>;
using CColorAllDecorator = CTimeEventDecorator<DWORDCOLOR>;
using CAirResistanceValueDecorator = CTimeEventDecorator<float>;
using CGravityValueDecorator = CTimeEventDecorator<float>;
using CRotationSpeedValueDecorator = CTimeEventDecorator<float>;

class CTextureAnimationCWDecorator : public CBaseDecorator, public CPooledObject<CTextureAnimationCWDecorator>
{
public:
	CTextureAnimationCWDecorator(float fFrameTime, uint32_t n, uint8_t * pIdx)
		: n(n), fLastFrameTime(fFrameTime), fFrameTime(fFrameTime), pIdx(pIdx)
	{
	}
	~CTextureAnimationCWDecorator() = default;
	CLASS_DELETE_COPYMOVE(CTextureAnimationCWDecorator);

protected:
	CBaseDecorator * __Clone(CParticleInstance * pfi, CParticleInstance * pi) override
	{
		return new CTextureAnimationCWDecorator(
			fFrameTime, n, static_cast<uint8_t *>(reinterpret_cast<unsigned char *>(pi) + (pIdx - reinterpret_cast<uint8_t *>(pfi))));
	}

	void __Excute(const CDecoratorData & d) override
	{
		fLastFrameTime -= d.fElapsedTime;
		while (fLastFrameTime < 0.0f)
		{
			fLastFrameTime += fFrameTime;
			if (++(*pIdx) >= n)
				*pIdx = 0;
		}
	}

	uint32_t n;
	float fLastFrameTime;
	float fFrameTime;
	uint8_t * pIdx;
};

class CTextureAnimationCCWDecorator : public CBaseDecorator, public CPooledObject<CTextureAnimationCCWDecorator>
{
public:
	CTextureAnimationCCWDecorator(float fFrameTime, uint8_t n, uint8_t * pIdx)
		: n(n), fLastFrameTime(fFrameTime), fFrameTime(fFrameTime), pIdx(pIdx)
	{
	}
	~CTextureAnimationCCWDecorator() = default;
	CLASS_DELETE_COPYMOVE(CTextureAnimationCCWDecorator);

protected:
	CBaseDecorator * __Clone(CParticleInstance * pfi, CParticleInstance * pi) override
	{
		return new CTextureAnimationCCWDecorator(
			fFrameTime, n, static_cast<uint8_t *>(reinterpret_cast<unsigned char *>(pi) + (pIdx - reinterpret_cast<uint8_t *>(pfi))));
	}

	void __Excute(const CDecoratorData & d) override
	{
		fLastFrameTime -= d.fElapsedTime;
		while (fLastFrameTime < 0.0f)
		{
			fLastFrameTime += fFrameTime;

			if (--(*pIdx) >= n && n != 0) // Because variable is unsigned..
				*pIdx = static_cast<uint8_t>(n - 1);
		}
	}

	uint8_t n;
	float fLastFrameTime;
	float fFrameTime;
	uint8_t * pIdx;
};

class CTextureAnimationRandomDecorator : public CBaseDecorator, public CPooledObject<CTextureAnimationRandomDecorator>
{
public:
	CTextureAnimationRandomDecorator(float fFrameTime, uint8_t n, uint8_t * pIdx)
		: n(n), fLastFrameTime(fFrameTime), fFrameTime(fFrameTime), pIdx(pIdx)
	{
	}
	~CTextureAnimationRandomDecorator() = default;
	CLASS_DELETE_COPYMOVE(CTextureAnimationRandomDecorator);

protected:
	CBaseDecorator * __Clone(CParticleInstance * pfi, CParticleInstance * pi) override
	{
		return new CTextureAnimationRandomDecorator(
			fFrameTime, n, static_cast<uint8_t *>(reinterpret_cast<unsigned char *>(pi) + (pIdx - reinterpret_cast<uint8_t *>(pfi))));
	}

	void __Excute(const CDecoratorData & d) override
	{
		fLastFrameTime -= d.fElapsedTime;
		if (fLastFrameTime < 0.0f && n != 0)
			*pIdx = static_cast<uint8_t>(random_range(0, n - 1));
		while (fLastFrameTime < 0.0f)
			fLastFrameTime += fFrameTime;
	}

	uint8_t n;
	float fLastFrameTime;
	float fFrameTime;
	uint8_t * pIdx;
};

class CAirResistanceDecorator : public CBaseDecorator, public CPooledObject<CAirResistanceDecorator>
{
public:
	CAirResistanceDecorator() = default;
	~CAirResistanceDecorator() override = default;

protected:
	void __Excute(const CDecoratorData & d) override;
	CBaseDecorator * __Clone(CParticleInstance * pfi, CParticleInstance * pi) override;
};

class CGravityDecorator : public CBaseDecorator, public CPooledObject<CGravityDecorator>
{
public:
	CGravityDecorator() = default;
	~CGravityDecorator() override = default;

protected:
	void __Excute(const CDecoratorData & d) override;
	CBaseDecorator * __Clone(CParticleInstance * pfi, CParticleInstance * pi) override;
};

class CRotationDecorator : public CBaseDecorator, public CPooledObject<CRotationDecorator>
{
public:
	CRotationDecorator() = default;
	~CRotationDecorator() override = default;

protected:
	void __Excute(const CDecoratorData & d) override;
	CBaseDecorator * __Clone(CParticleInstance * pfi, CParticleInstance * pi) override;
};
} // namespace NEffectUpdateDecorator
