#pragma once

#include "GrpImageInstance.h"

#ifdef ENABLE_INGAME_WIKI
typedef struct tagExpandedRECT
{
	LONG	left_top;
	LONG	left_bottom;
	LONG	top_left;
	LONG	top_right;
	LONG	right_top;
	LONG	right_bottom;
	LONG	bottom_left;
	LONG	bottom_right;
} ExpandedRECT, * PEXPANDEDRECT;
#endif

class CGraphicExpandedImageInstance : public CGraphicImageInstance
{
public:
	static uint32_t Type();
	static void DeleteExpandedImageInstance(CGraphicExpandedImageInstance * pkInstance)
	{
		pkInstance->Destroy();
		ms_kPool.Free(pkInstance);
	}

	enum ERenderingMode
	{
		RENDERING_MODE_NORMAL,
		RENDERING_MODE_SCREEN,
		RENDERING_MODE_COLOR_DODGE,
		RENDERING_MODE_MODULATE
	};

public:
	CGraphicExpandedImageInstance();
	virtual ~CGraphicExpandedImageInstance();

	void Destroy();

	void SetDepth(float fDepth);
	void SetOrigin();
	void SetOrigin(float fx, float fy);
	void SetRotation(float fRotation);
	void SetScale(float fx, float fy);
	void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
#ifdef ENABLE_GROWTH_PET_SYSTEM
	void SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom);
#endif
	void SetRenderingMode(int iMode);
#ifdef ENABLE_12ZI
	void RenderCoolTime(float fCoolTime);
#endif
#ifdef ENABLE_INGAME_WIKI
	void SetRenderBox(RECT& renderBox);
	uint32_t GetPixelColor(uint32_t x, uint32_t y);
	int GetRenderWidth();
	int GetRenderHeight();
	void SetExpandedRenderingRect(float fLeftTop, float fLeftBottom, float fTopLeft, float fTopRight, float fRightTop, float fRightBottom, float fBottomLeft, float fBottomRight);
	void iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom);
	void iSetExpandedRenderingRect(int iLeftTop, int iLeftBottom, int iTopLeft, int iTopRight, int iRightTop, int iRightBottom, int iBottomLeft, int iBottomRight);
	void SetTextureRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
#endif
#ifdef ENABLE_OFFICAL_FEATURES
	void LeftRightReverse();
#endif

protected:
	void Initialize();

	void OnRender();
	void OnSetImagePointer();
#ifdef ENABLE_12ZI
	void OnRenderCoolTime(float fCoolTime);
#endif

	BOOL OnIsType(uint32_t dwType);

#ifdef ENABLE_INGAME_WIKI
private:
	void SaveColorMap();
#endif

protected:
	float m_fDepth;
	D3DXVECTOR2 m_v2Origin;
	D3DXVECTOR2 m_v2Scale;
	float m_fRotation;
#ifdef ENABLE_INGAME_WIKI
	ExpandedRECT m_RenderingRect;
	RECT m_TextureRenderingRect;
	RECT m_renderBox;
	uint32_t* m_pColorMap;
#else
	RECT m_RenderingRect;
#endif
	int m_iRenderingMode;
#ifdef ENABLE_OFFICAL_FEATURES
	bool m_bLeftRightReverse;
#endif

public:
	static void CreateSystem(uint32_t uCapacity);
	static void DestroySystem();

	static CGraphicExpandedImageInstance * New();
	static void Delete(CGraphicExpandedImageInstance * pkImgInst);

	static CDynamicPool<CGraphicExpandedImageInstance> ms_kPool;
};
