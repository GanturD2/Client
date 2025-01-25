#pragma once

#include "../EterLib/GrpTextInstance.h"
#include "../EterLib/GrpMarkInstance.h"
#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/GrpExpandedImageInstance.h"

#include "../EterGrnLib/ThingInstance.h"

class CPythonGraphic : public CScreen, public CSingleton<CPythonGraphic>
{
public:
	CPythonGraphic();
	~CPythonGraphic() override;

	static void Destroy();

	void PushState();
	void PopState();

	static LPDIRECT3D9 GetD3D();

	float GetOrthoDepth() const;
	void SetInterfaceRenderState() const;
	static void SetGameRenderState();

	void SetCursorPosition(int x, int y);

	static void SetOmniLight();

	void SetViewport(float fx, float fy, float fWidth, float fHeight);
	void RestoreViewport() const;

	long GenerateColor(float r, float g, float b, float a);
	void RenderDownButton(float sx, float sy, float ex, float ey);
	void RenderUpButton(float sx, float sy, float ex, float ey);

	void RenderImage(CGraphicImageInstance * pImageInstance, float x, float y);
	void RenderAlphaImage(CGraphicImageInstance * pImageInstance, float x, float y, float aLeft, float aRight) const;
	void RenderCoolTimeBoxColor(float fxCenter, float fyCenter, float fRadius, float fTime, float fColorR, float fColorG, float fColorB, float fColorA);
	void RenderCoolTimeBox(float fxCenter, float fyCenter, float fRadius, float fTime
#ifdef ENABLE_GROWTH_PET_SYSTEM
		, float fr = 0.0f, float fg = 0.0f, float fb = 0.0f, float fa = 0.5f
#endif
	) const;
	void RenderCoolTimeBoxInverse(float fxCenter, float fyCenter, float fRadius, float fTime);

	static bool SaveJPEG(const char * pszFileName, LPBYTE pbyBuffer, uint32_t uWidth, uint32_t uHeight);
	static bool SaveScreenShot(const char * c_pszFileName);

	static uint32_t GetAvailableMemory();
	static void SetGamma(float fGammaFactor = 1.0f);

protected:
	typedef struct SState
	{
		D3DXMATRIX matView;
		D3DXMATRIX matProj;
	} TState;

	uint32_t m_lightColor;
	uint32_t m_darkColor;

protected:
	std::stack<TState> m_stateStack;

	D3DXMATRIX m_SaveWorldMatrix;

	CCullingManager m_CullingManager;

	D3DVIEWPORT9 m_backupViewport;

	float m_fOrthoDepth;
};