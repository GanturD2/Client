#ifndef __INC_CEF_CLIENT_HANDLER_H__
#define __INC_CEF_CLIENT_HANDLER_H__

#ifdef CEF_BROWSER
#include <cef/cef_render_process_handler.h>
#include <cef/cef_client.h>
#include <cef/cef_v8.h>
#include <cef/cef_browser.h>

class ClientHandler : public CefClient, public CefLifeSpanHandler
{
public:
	ClientHandler();

	CefRefPtr<CefBrowser> GetBrowser()
	{
		return m_Browser;
	}

	CefWindowHandle GetBrowserHwnd()
	{
		return m_BrowserHwnd;
	}

	// CefClient methods
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
	{
		return this;
	}

	// Virutal on CefLifeSpanHandler
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

protected:
	// The child browser window
	CefRefPtr<CefBrowser> m_Browser;

	// The child browser window handle
	CefWindowHandle m_BrowserHwnd;

	// /
	// Macro that provides a reference counting implementation for classes extending
	// CefBase.
	// /
	IMPLEMENT_REFCOUNTING(ClientHandler);
};
#endif

#endif __INC_CEF_CLIENT_HANDLER_H__
