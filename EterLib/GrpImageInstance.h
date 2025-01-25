#pragma once

#include "GrpImage.h"
#include "GrpIndexBuffer.h"
#include "GrpVertexBufferDynamic.h"
#include "Pool.h"
#include "../UserInterface/Locale_inc.h"

class CGraphicImageInstance
{
public:
	static uint32_t Type();
	BOOL IsType(uint32_t dwType);

public:
	CGraphicImageInstance();
	virtual ~CGraphicImageInstance();

	void Destroy();

	void Render();

	void SetDiffuseColor(float fr, float fg, float fb, float fa);
	void SetPosition(float fx, float fy);

	void SetImagePointer(CGraphicImage * pImage);
	void ReloadImagePointer(CGraphicImage * pImage);
#ifdef ENABLE_12ZI
	void RenderCoolTime(float fCoolTime);
#endif
	bool IsEmpty() const;

	int GetWidth();
	int GetHeight();

	CGraphicTexture * GetTexturePointer();
	const CGraphicTexture & GetTextureReference() const;
	CGraphicImage * GetGraphicImagePointer();

	bool operator==(const CGraphicImageInstance & rhs) const;

#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MINI_GAME_YUTNORI)
	void SetScale(float fx, float fy);
	void SetScale(D3DXVECTOR2 v2Scale);
	const D3DXVECTOR2& GetScale() const;
	void SetScalePercent(uint8_t byPercent);
	void SetScalePivotCenter(bool bScalePivotCenter);
	void LeftRightReverse();
#endif

protected:
	void Initialize();

	virtual void OnRender();
	virtual void OnSetImagePointer();
#ifdef ENABLE_12ZI
	virtual void OnRenderCoolTime(float fCoolTime);
#endif

	virtual BOOL OnIsType(uint32_t dwType);

protected:
	D3DXCOLOR m_DiffuseColor;
	D3DXVECTOR2 m_v2Position;
#if defined(ENABLE_OFFICAL_FEATURES) || defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_MINI_GAME_YUTNORI)
	D3DXVECTOR2 m_v2Scale;
	bool m_bScalePivotCenter;
	bool m_bLeftRightReverse;
#endif
	CGraphicImage::TRef m_roImage;

public:
	static void CreateSystem(uint32_t uCapacity);
	static void DestroySystem();

	static CGraphicImageInstance * New();
	static void Delete(CGraphicImageInstance * pkImgInst);

	static CDynamicPool<CGraphicImageInstance> ms_kPool;
};
