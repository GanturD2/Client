#pragma once
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_INGAME_WIKI
#include <cstdint>
#include <memory>
#include "GrpWikiRenderTargetTexture.h"

class CInstanceBase;
class CGraphicImageInstance;

class CWikiRenderTarget
{
	using TCharacterInstanceMap = std::map<uint32_t, CInstanceBase*>;
	
	public:
		CWikiRenderTarget(uint32_t width, uint32_t height);
		virtual ~CWikiRenderTarget();
	
	public:
		void		SetVisibility(bool flag) { m_visible = flag; }
		
		void		CreateTextures() const;
		void		ReleaseTextures() const;
		void		RenderTexture() const;
		
		void		SetRenderingRect(RECT* rect) const;
		void		SetRenderingBox(RECT* renderBox) const;
		
		void		SelectModel(uint32_t model_vnum);
		void		UpdateModel();
		void		DeformModel() const;
		void		RenderModel() const;
		
		void		SetWeapon(uint32_t dwVnum);
		void		SetArmor(uint32_t vnum);
		void		SetHair(uint32_t vnum);
		void		SetModelV3Eye(float x, float y, float z);
		void		SetModelV3Target(float x, float y, float z);
	
	private:
		std::unique_ptr<CInstanceBase>						m_pModel;
		std::unique_ptr<CGraphicWikiRenderTargetTexture>	m_renderTargetTexture;
		float												m_modelRotation;
		bool												m_visible;
		D3DXVECTOR3											m_v3Eye;
		D3DXVECTOR3											m_v3Target;
		D3DXVECTOR3											m_v3Up;
};
#endif
