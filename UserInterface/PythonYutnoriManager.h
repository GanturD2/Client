#pragma once

#ifdef ENABLE_MINI_GAME_YUTNORI
#include "InstanceBase.h"

class CPythonYutnoriManager : public CSingleton<CPythonYutnoriManager>
{
public:
		CPythonYutnoriManager();
		virtual ~CPythonYutnoriManager();

		void Initialize();
		void Destroy();
		
		void ClearInstances();
		bool CreateModelInstance();
		
		void RenderBackground();
		void DeformModel();
		void RenderModel();
		void UpdateModel();
		void UpdateDeleting();

		void SetShow(bool bShow);
		bool IsShow() { return m_bShow; }
		
		void ChangeMotion(uint8_t bMotion);

private:
	CInstanceBase* m_pModelInstance;
	CGraphicImageInstance* m_pBackgroundImage;
		
	bool m_bShow;
	float m_fModelHeight;
		
	bool m_bIsDeleting;
	float m_fStartDelete;
	float m_fAlphaValue;
};
#endif
