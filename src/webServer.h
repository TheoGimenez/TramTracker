/*
 * File: /src/WebServer.h
 * Project: ESP32 TramTracker
 * Created Date: 15-09-2026, 10:16:53 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Sunday, October 18th 2026, 6:10:34 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_WEB_SERVER_H_
#define SRC_WEB_SERVER_H_

#include <functional>
//
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//
#include "stopSelection.html.h"

namespace web
{
    // inline constexpr const char* k_stop_selection_html = /include/stopSelection.html.h;
    inline constexpr const char* k_wifi_html = nullptr;
    inline constexpr const char* k_route_param_intput = "route";
    inline constexpr const char* k_stop_param_input = "stop";
} // namespace web

class WebServer
{
    using Callback = std::function<void(const String&, const String&)>;

    enum PageRenderer {
        WIFI_CONFIG,
        STOP_CONFIG
    };

    AsyncWebServer m_server;
    PageRenderer   m_pageToRender;
    Callback       m_onUpdateCallback;

public:
        WebServer(void);
public:
    void begin                  (Callback           onUpdateCallback);

    void setHomePageToWifiConfig    (void);
    void setHomePageToStopSelection(void);
private:
    void _homePageHandler       (AsyncWebServerRequest* request);
    void _onSelectHandler       (AsyncWebServerRequest* request);
    void _onStopHandler         (AsyncWebServerRequest* request);

    void _routeSelectorCallback (AsyncWebServerRequest* request);
    void _wifiConfigCallback    (AsyncWebServerRequest* request);

};

#endif /* SRC_WEB_SERVER_H_ */