/*
 * File: /src/WebServer.cpp
 * Project: ESP32 TramTracker
 * Created Date: 15-09-2026, 10:16:53 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:27:14 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "WebServer.h"

namespace web {

AsyncServer::AsyncServer(void) : m_server{env::k_web_server_port}, m_pageToRender{DefaultHomePage::SELECT_STOP_PAGE}
{}

void
AsyncServer::begin(Callback callback)
{
m_onUpdateCallback = callback;

    m_server.on("/",       HTTP_GET,  [this](auto*r ){ _homePageHandler(r); });
    m_server.on("/stops",  HTTP_GET,  [this](auto*r ){ _onStopHandler(r); });
    m_server.on("/select", HTTP_POST, [this](auto*r ){ _onSelectHandler(r); });

    m_server.begin();
}


void
AsyncServer::setHomePage(const DefaultHomePage p, Callback callback)
{
    m_pageToRender     = p;
    m_onUpdateCallback = callback;
}

// private
static String
stop_selector_process_template(const String& var)
{
    if (var == "ROUTES")
    {
        String options("");

        transit::forEachOrderedRouteName([](const char* route, void* options)->bool {
            auto& opt = *static_cast<String*>(options);

            opt.concat("<option value=\"");
            opt.concat(route);
            opt.concat("\">");
            opt.concat(route);
            opt.concat("</option>\n");

            return false;
        }, &options);

        return options;
    }

    return asyncsrv::emptyString;
}

void
AsyncServer::_homePageHandler(AsyncWebServerRequest* request)
{
    if (m_pageToRender == DefaultHomePage::SELECT_STOP_PAGE)
    {
        return request->send(200, "text/html", web::k_stop_selection_html, &stop_selector_process_template);
    }
    else // m_pageToRender == DefaultHomePage::SET_WIFI_PAGE
    {
        // return request->send(200, "text/html", web::k_wifi_html);
    }
}

void
AsyncServer::_onSelectHandler(AsyncWebServerRequest* request)
{
    if (m_pageToRender == DefaultHomePage::SELECT_STOP_PAGE)
    {
        _routeSelectorCallback(request);
    }
    else // m_pageToRender == DefaultHomePage::SET_WIFI_PAGE
    {
        _wifiConfigCallback(request);
    }
}

void AsyncServer::_onStopHandler(AsyncWebServerRequest *request)
{
    if (request->hasParam(web::k_route_param_input))
    {
        String     selected_route = request->getParam(web::k_route_param_input)->value();
        const auto route_idx      = transit::get_route_idx(selected_route.c_str());

        if (route_idx) // route idx 0 is for unknown
        {
            std::set<std::string> stops = transit::get_stops_list(route_idx);

            if (stops.size())
            {
                cJSON *json_array = cJSON_CreateArray();

                for (const auto &s : stops)
                {
                    cJSON_AddItemToArray(json_array, cJSON_CreateString(s.c_str()));
                }
                char *json_str = cJSON_PrintUnformatted(json_array);

                request->send(200, "application/json; charset=utf-8", json_str);

                cJSON_free(json_str);
                cJSON_Delete(json_array);

                return;
            }
        }
    }
    return request->send(400, "text/plain", "Bad Request");
}

void
AsyncServer::_routeSelectorCallback(AsyncWebServerRequest* request)
{
    if (request->hasParam(web::k_route_param_input) && request->hasParam(web::k_stop_param_input))
    {
        const String selected_route = request->getParam(web::k_route_param_input)->value();
        const String selected_stop  = request->getParam(web::k_stop_param_input)->value();

        if (!m_onUpdateCallback(selected_route.c_str(), selected_stop.c_str()))
        {
            return request->send(400, "text/plain", "Bad request: Incorrect value");
        }

        return request->send(200, "text/plain", "OK");
    }

    return request->send(400, "text/plain", "Bad request: Missing parameters");
}

void
AsyncServer::_wifiConfigCallback(AsyncWebServerRequest* request)
{
}

} // namespace web