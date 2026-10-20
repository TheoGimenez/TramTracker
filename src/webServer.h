/*
 * File: /src/WebServer.h
 * Project: ESP32 TramTracker
 * Created Date: 15-09-2026, 10:16:53 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 2:00:07 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_WEB_SERVER_H_
#define SRC_WEB_SERVER_H_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <set>
//
#include <WString.h> //! STRING
#include <cJSON.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//
#include "generated_app_config.h"
#include "stopSelection.html.h"
#include "transit/transitCatalog.h"

namespace web
{
    // inline constexpr const char* k_stop_selection_html = /include/stopSelection.html.h;
    inline constexpr const char* k_wifi_html = nullptr;
    inline constexpr const char* k_route_param_input = "route";
    inline constexpr const char* k_stop_param_input = "stop";
} // namespace web

class WebServer
{
public:
    using Callback = std::function<bool(const String&, const String&)>;

    enum DefaultHomePage : uint8_t
    {
        SET_WIFI_PAGE,
        SELECT_STOP_PAGE,
        page_count // do not use
    };

private:
    AsyncWebServer  m_server;
    DefaultHomePage m_pageToRender;
    Callback        m_onUpdateCallback;
    // const char      m_pages[page_count];

public:
        WebServer(void);
public:
    void begin                  (Callback           onUpdateCallback);
    void setHomePage            (const DefaultHomePage p, Callback callback);
private:
    void _homePageHandler       (AsyncWebServerRequest* request);
    void _onSelectHandler       (AsyncWebServerRequest* request);
    void _onStopHandler         (AsyncWebServerRequest* request);

    void _routeSelectorCallback (AsyncWebServerRequest* request);
    void _wifiConfigCallback    (AsyncWebServerRequest* request);

};

#endif /* SRC_WEB_SERVER_H_ */