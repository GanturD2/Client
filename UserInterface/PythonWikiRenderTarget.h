#pragma once
#include "StdAfx.h"

#ifdef ENABLE_INGAME_WIKI
#include "../EterPythonLib/PythonWindow.h"
#include "../eterLib/CWikiRenderTargetManager.h"
#include "../eterLib/GrpWikiRenderTargetTexture.h"

class CPythonWikiRenderTarget : public CSingleton<CPythonWikiRenderTarget>
{
	public:
		CPythonWikiRenderTarget();
		virtual ~CPythonWikiRenderTarget();
	
	public:
		const static	int DELETE_PARM = -1;
		const static	int START_MODULE = 1;
		
		typedef std::vector<std::tuple<int, std::shared_ptr<UI::CUiWikiRenderTarget>>> TWikiRenderTargetModules;
		
		int		GetFreeID();
		void		RegisterRenderModule(int module_id, int module_wnd);
		
		void		ManageModelViewVisibility(int module_id, bool flag);
		
		void		ShowModelViewManager(bool flag) { _bCanRenderModules = flag; }
		bool		CanRenderWikiModules() const;
		
		void		SetModelViewModel(int module_id, int module_vnum);
		void		SetWeaponModel(int module_id, int weapon_vnum);
		void		SetModelForm(int module_id, int main_vnum);
		void		SetModelHair(int module_id, int hair_vnum);
		void		SetModelV3Eye(int module_id, float x, float y, float z);
		void		SetModelV3Target(int module_id, float x, float y, float z);
	
	protected:
		bool									_InitializeWindow(int module_id, UI::CUiWikiRenderTarget* handle_window);
		std::shared_ptr<CWikiRenderTarget>		_GetRenderTargetHandle(int module_id);
	
	private:
		TWikiRenderTargetModules				_RenderWikiModules;
		bool									_bCanRenderModules;
};
#endif
