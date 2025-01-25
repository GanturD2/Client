#pragma once
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_RENDER_TARGET
#include <memory>
#include "GrpRenderTargetTexture.h"

class CInstanceBase;
class CGraphicImageInstance;

class CRenderTarget
{
	using TCharacterInstanceMap = std::map<uint32_t, CInstanceBase*>;
	
	public:
		CRenderTarget(uint32_t width, uint32_t height);
		~CRenderTarget();

		void SetVisibility(bool isShow);
		void RenderTexture() const;
		void SetRenderingRect(RECT* rect) const;

		void SelectModel(uint32_t index);
		void SelectModelNPC(uint32_t index);
#if defined(ENABLE_ACCE_COSTUME_SYSTEM) || defined (ENABLE_AURA_SYSTEM)
		void SelectModelPC(const uint32_t race, uint32_t index, uint32_t weapon, uint32_t armor, uint32_t hair, uint32_t acce, uint32_t aura);
#else
		void SelectModelPC(const uint32_t race, uint32_t index, uint32_t weapon, uint32_t armor, uint32_t hair);
#endif
		bool ItemMatchesRace(uint32_t race, uint32_t vnum);
		uint32_t RaceMatchesAntiflag(uint32_t byRace, uint32_t dwItemVnum);

		bool CreateBackground(const char* imgPath, uint32_t width, uint32_t height);
		void RenderBackground() const;
		void UpdateModel();
		void DeformModel() const;
		void RenderModel() const;
		void SetZoom(bool bZoom) noexcept;

		void SetMotion(uint32_t motion);
		void SetRotation(bool rotation);
		void ChangeArmor(uint32_t vnum);
		void ChangeWeapon(uint32_t vnum);
		void ChangeHair(uint32_t vnum);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		void ChangeAcce(uint32_t vnum);
#endif
#ifdef ENABLE_AURA_SYSTEM
		void ChangeAura(uint32_t vnum);
#endif

		void CreateTextures() const;
		void ReleaseTextures() const;
	
	private:
		std::unique_ptr<CInstanceBase> m_pModel; 
		std::unique_ptr<CGraphicImageInstance> m_background;
		std::unique_ptr<CGraphicRenderTargetTexture> m_renderTargetTexture;
		float m_modelRotation;
		float m_fEyeY;
		float m_fTargetY;
		float m_fTargetHeight;
		float m_fZoomY;
		bool m_visible;
};
#endif
