#include <webServer.h>
#include "generated_app_config.h"

WebServer::WebServer(void) : server(app::kWebServerPort) {}

void WebServer::begin(std::function<void(const String &)> callback)
{
    m_onUpdateCallback = callback;

    server.on("/", HTTP_GET, [this](auto *r){ _homePageHandler(r); });
    server.on("/update", HTTP_GET, [this](auto *r){ _onUpdateHandler(r); });

    // start server
    server.begin();
}

void WebServer::_homePageHandler(AsyncWebServerRequest *request)
{
    return request->send(200, "text/html", INDEX_HTML);
}

void WebServer::_onUpdateHandler(AsyncWebServerRequest *request)
{
    if (request->hasParam(TRAM_SELECTOR_PARAM_INPUT))
    {
        String selectedLine = request->getParam(TRAM_SELECTOR_PARAM_INPUT)->value();
        m_onUpdateCallback(selectedLine);
    }
    request->send(200, "text/plain", "OK");
}
