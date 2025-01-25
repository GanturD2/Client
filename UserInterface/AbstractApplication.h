#pragma once

#include "AbstractSingleton.h"

class IAbstractApplication : public TAbstractSingleton<IAbstractApplication>
{
public:
	struct SCameraPos
	{
		float m_fUpDir{0.0f};
		float m_fViewDir{0.0f};
		float m_fCrossDir{0.0f};

		SCameraPos() = default;
	};

	struct SCameraSetting
	{
		D3DXVECTOR3 v3CenterPosition{0.0f, 0.0f, 0.0f};
		SCameraPos kCmrPos{};
		float fRotation{0.0f};
		float fPitch{0.0f};
		float fZoom{0.0f};

		SCameraSetting() = default;
	};

public:
	IAbstractApplication() = default;
	~IAbstractApplication() = default;

	virtual void GetMousePosition(POINT * ppt) = 0;
	virtual float GetGlobalTime() = 0;
	virtual float GetGlobalElapsedTime() = 0;

	virtual void SkipRenderBuffering(uint32_t dwSleepMSec) = 0;
	virtual void SetServerTime(time_t tTime) = 0;
	virtual void SetCenterPosition(float fx, float fy, float fz) = 0;

	virtual void SetEventCamera(const SCameraSetting & c_rCameraSetting) = 0;
	virtual void BlendEventCamera(const SCameraSetting & c_rCameraSetting, float fBlendTime) = 0;
	virtual void SetDefaultCamera() = 0;
#ifdef ENABLE_REVERSED_FUNCTIONS_EX
	virtual void CpyDefaultCamera(const SCameraSetting& c_rCameraSetting) = 0;
#endif

	virtual void RunIMEUpdate() = 0;
	virtual void RunIMETabEvent() = 0;
	virtual void RunIMEReturnEvent() = 0;

	virtual void RunIMEChangeCodePage() = 0;
	virtual void RunIMEOpenCandidateListEvent() = 0;
	virtual void RunIMECloseCandidateListEvent() = 0;
	virtual void RunIMEOpenReadingWndEvent() = 0;
	virtual void RunIMECloseReadingWndEvent() = 0;
};