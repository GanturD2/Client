#pragma once
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_INGAME_WIKI
#include <unordered_map>
#include <cstdint>
#include <memory>
#include "../EterBase/Singleton.h"
#include "CWikiRenderTarget.h"

class CWikiRenderTargetManager : public CSingleton<CWikiRenderTargetManager>
{
	public:
		CWikiRenderTargetManager();
		virtual ~CWikiRenderTargetManager();
		void InitializeData() { m_renderTargets.clear(); }
	
	public:
		std::shared_ptr<CWikiRenderTarget>	GetRenderTarget(int module_id);
		bool								CreateRenderTarget(int module_id, int width, int height);
		void								DeleteRenderTarget(int module_id);
		
		void								CreateRenderTargetTextures();
		void								ReleaseRenderTargetTextures();
		
		void								DeformModels();
		void								UpdateModels();
		void								RenderModels();
	
	protected:
		std::unordered_map<int, std::shared_ptr<CWikiRenderTarget>>	m_renderTargets;
};
#endif
