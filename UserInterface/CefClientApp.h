#ifndef __INC_CEF_CLIENT_APP_H__
#define __INC_CEF_CLIENT_APP_H__

#ifdef CEF_BROWSER
#include <cef/cef_app.h>
#include <cef/cef_client.h>

class ClientApp : public CefApp, public CefRenderProcessHandler
{
public:
	ClientApp();

	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
	{
		return this;
	}

	void OnWebKitInitialized() OVERRIDE;

	IMPLEMENT_REFCOUNTING(ClientApp);
};
#endif

#endif // __INC_CEF_CLIENT_APP_H__
