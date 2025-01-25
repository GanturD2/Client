#include "StdAfx.h"

#ifdef CEF_BROWSER
#include "CefClientApp.h"

#include "CefClientHandler.h"
#include "CefClientV8ExtensionHandler.h"
#include <cef/wrapper/cef_helpers.h>

ClientApp::ClientApp()
{
}

void ClientApp::OnWebKitInitialized()
{
	const std::string app_code =
		"var app;"
		"if (!app)"
		"	app = {};"
		"(function() {"
		"	app.ChangeTextInJS = function(text) {"
		"		native function ChangeTextInJS();"
		"		return ChangeTextInJS(text);"
		"	};"
		"})();";

	CefRegisterExtension("v8/app", app_code, new ClientV8ExtensionHandler(this));
}
#endif
