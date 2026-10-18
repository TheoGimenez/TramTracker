/*
 * File: /src/WebServer.cpp
 * Project: ESP32 TramTracker
 * Created Date: 15-09-2026, 10:16:53 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Sunday, October 18th 2026, 5:45:21 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "WebServer.h"
//
#include <set>
//
#include <Arduino.h>
#include <cJSON.h>
#include <esp_log.h>
//
#include "generated_app_config.h"
#include "stopUtils.h"
#include "utils.h"

WebServer::WebServer(void) : m_server{app::k_web_server_port}, m_pageToRender{PageRenderer::STOP_CONFIG}
{}

void
WebServer::begin(std::function<void(const String &, const String&)> callback)
{
    m_onUpdateCallback = callback;

    m_server.on("/", HTTP_GET, [this](auto *r){ _homePageHandler(r); });
    
    m_server.on("/stops", HTTP_GET, [this](auto *r){ _onStopHandler(r); });
    m_server.on("/select", HTTP_POST, [this](auto *r){ _onSelectHandler(r); });

    // start server
    m_server.begin();
}

void
WebServer::setHomePageToWifiConfig(void)
{
    m_pageToRender = PageRenderer::WIFI_CONFIG;
}

void
WebServer::setHomePageToStopSelection(void)
{
    m_pageToRender = PageRenderer::STOP_CONFIG;
}

// private
static String
stop_selector_process_template(const String &var)
{Serial.printf("Template demandé : [%s]\n", var.c_str());
    if (var == "ROUTES")
    {
        String options("");
        const auto &routes = get_routes_list();
        for (const String &route : routes)
        {
            options += "<option value=\"" + route + "\">";
            options += route;
            options += "</option>\n";
        }

        return options;
    }

    return asyncsrv::emptyString;
}

void
WebServer::_homePageHandler(AsyncWebServerRequest* request)
{
    if (m_pageToRender == PageRenderer::STOP_CONFIG)
    {
        return request->send(200, "text/html", web::k_stop_selection_html, &stop_selector_process_template);
    }
    else // m_pageToRender == PageRenderer::WIFI_CONFIG
    {
        // return request->send(200, "text/html", web::k_wifi_html);
    }
}

void
WebServer::_onSelectHandler(AsyncWebServerRequest* request)
{
    if (m_pageToRender == PageRenderer::STOP_CONFIG)
    {
        _routeSelectorCallback(request);
    }
    else // m_pageToRender == PageRenderer::WIFI_CONFIG
    {
        _wifiConfigCallback(request);
    }
}

void WebServer::_onStopHandler(AsyncWebServerRequest *request)
{
    if (request->hasParam(web::k_route_param_intput))
    {
        String selected_route = request->getParam(web::k_route_param_intput)->value();

        const auto route_idx = get_route_idx(selected_route);
        if (route_idx) // route idx 0 is for unknown
        {
            std::set<String> stops = get_stops_list_from_route(route_idx);

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

void    // _onSelectHandler
WebServer::_routeSelectorCallback(AsyncWebServerRequest* request)
{
    if (request->hasParam(web::k_route_param_intput) && request->hasParam(web::k_stop_param_input))
    {
        String selected_route = request->getParam(web::k_route_param_intput)->value();
        String selected_line = request->getParam(web::k_stop_param_input)->value();

        m_onUpdateCallback(selected_route, selected_line);
        
        return request->send(200, "text/plain", "OK");
    }

    return request->send(400, "text/plain", "Bad request: Missing parameters");
}

void
WebServer::_wifiConfigCallback(AsyncWebServerRequest* request)
{
}

// Legacy callback that use json.
/*
void WebServer::_onSelectHandler(AsyncWebServerRequest *request, uint8_t* data, size_t len, size_t index, size_t total)
{
    //! Nota bene for futur me that read here: if the request if big, the data are comming chunked.
    //! if json < 2ko there a high probability that all data are on chunk 'index' 0.
    if (index != 0 || len != total)
    {
        return request->send(413, "text/plain", "Payload Too Large or chunked");
    }

    char* json_buff = (char*) malloc(len + 1);
    if (json_buff == NULL)
    {
        return request->send(500, "text/plain", "Out of memory");
    }

    memcpy(json_buff, data, len);
    json_buff[len] = '\0';

    cJSON* root = cJSON_Parse(json_buff);

    free(json_buff);

    if (root == NULL)
    {
        return request->send(500, "text/plain", "Bad Request: invalid JSON");
    }

    if (cJSON_IsObject(root))
    {
        cJSON* route_item = cJSON_GetObjectItem(root, web::k_route_param_intput);
        cJSON* stop_item = cJSON_GetObjectItem(root, web::k_stop_param_input);

        if (cJSON_IsString(route_item) && cJSON_IsString(stop_item))
        {
            String route = route_item->valuestring;
            String stop = stop_item->valuestring;

            m_onUpdateCallback(route, stop);

            cJSON_Delete(root);
            return request->send(200, "text/plain", "OK");
        }
    }
    cJSON_Delete(root);
    return request->send(400, "text/plain", "Bad Request");
}
*/