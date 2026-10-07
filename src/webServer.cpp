/*
 * File: /src/webServer.cpp
 * Project: ESP32 TramTracker
 * Created Date: 15-09-2026, 10:16:53 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 11:47:22 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "webServer.h"
#include "generated_app_config.h"

WebServer::WebServer(void) : m_server{app::k_web_server_port}
{}

void
WebServer::begin(std::function<void(const String &)> callback)
{
    m_onUpdateCallback = callback;

    m_server.on("/", HTTP_GET, [this](auto *r){ _homePageHandler(r); });
    m_server.on("/update", HTTP_GET, [this](auto *r){ _onUpdateHandler(r); });

    // start server
    m_server.begin();
}

// private
void
WebServer::_homePageHandler(AsyncWebServerRequest *request)
{
    return request->send(200, "text/html", web::k_index_html);
}

void
WebServer::_onUpdateHandler(AsyncWebServerRequest *request)
{
    if (request->hasParam(web::k_tram_selector_param_intput))
    {
        String selected_line = request->getParam(web::k_tram_selector_param_intput)->value();
        m_onUpdateCallback(selected_line);
    }
    request->send(200, "text/plain", "OK");
}
