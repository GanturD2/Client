#pragma once

class CGraphicRatioInstance
{
public:
	CGraphicRatioInstance();
	virtual ~CGraphicRatioInstance();

	void Clear();

	void SetRatioReference(const float & c_rRatio);
	void BlendRatioReference(uint32_t blendTime, const float & c_rDstRatio);

	void Update();

	const float & GetCurrentRatioReference() const;

protected:
	uint32_t GetTime();

protected:
	float m_curRatio;
	float m_srcRatio;
	float m_dstRatio;

	uint32_t m_baseTime;
	uint32_t m_blendTime;
};
