#pragma once

#ifdef ENABLE_INGAME_WIKI
#include "GrpBase.h"
#include "GrpTexture.h"

class CGraphicWikiRenderTargetTexture : public CGraphicTexture
{
	public:
		CGraphicWikiRenderTargetTexture();
		virtual	~CGraphicWikiRenderTargetTexture();
	
	public:
		bool				Create(int width, int height, D3DFORMAT texFormat, D3DFORMAT depthFormat);
		
		void				CreateTextures();
		bool				CreateRenderTexture(int width, int height, D3DFORMAT format);
		void				ReleaseTextures();
		LPDIRECT3DTEXTURE9	GetD3DRenderTargetTexture() const;
		
		bool				CreateRenderDepthStencil(int width, int height, D3DFORMAT format);
		
		void				SetRenderTarget();
		void				ResetRenderTarget();
		
		void				SetRenderingRect(RECT* rect);
		RECT*				GetRenderingRect();
		void				SetRenderingBox(RECT* renderBox);
		RECT*				GetRenderingBox();
		
		static void			Clear();
		void				Render() const;
	
	protected:
		void Reset();
	
	protected:
		LPDIRECT3DTEXTURE9	m_lpd3dRenderTexture{};
		LPDIRECT3DSURFACE9	m_lpd3dRenderTargetSurface{};
		LPDIRECT3DSURFACE9	m_lpd3dDepthSurface{};
		LPDIRECT3DSURFACE9	m_lpd3dOriginalRenderTarget{};
		LPDIRECT3DSURFACE9	m_lpd3dOldDepthBufferSurface{};
		
		D3DFORMAT			m_d3dFormat;
		D3DFORMAT			m_depthStencilFormat;
		
		RECT				m_renderRect{};
		RECT				m_renderBox{};
};
#endif
