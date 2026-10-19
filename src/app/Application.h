/*
 * File: /src/app/Application.h
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:31 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 1:30:24 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_APP_APPLICATION_H_
#define SRC_APP_APPLICATION_H_

#include <string>
//
// #include <Arduino.h>
#include <WiFi.h>
//
#include "debugLog.h"
#include "DisplayController.h"
#include "generated_app_config.h"
#include "../realtime/rtArrivalService.h"
#include "../transit/transitCatalog.h"
#include "transitType.h"
#include "../realtime/rtHttpClient.h"
#include "utils.h"
#include "WebServer.h"

#define TZ_Europe_Paris PSTR("CET-1CEST,M3.5.0,M10.5.0/3")

namespace {
constexpr const char* k_ntp_timezone = TZ_Europe_Paris;
constexpr const char* k_ntp_server   = "pool.ntp.org";
constexpr const char* k_text_no_service = "Pas de service.";

namespace hardware {
    constexpr uint8_t k_esp_led = 2;     // esp integrated blue led
    constexpr uint8_t k_boot_button = 0; // esp boot button
} // namespace hardware
} // namespace

class Application
{
    DisplayController m_display;        // LED matrix controller
    WebServer         m_webServer;
    // TransitCatalog    m_transitCatalog;
    stop_selection_s  m_selectedStop    = {0};

public:
    void   begin               (void);
    void   update              (void);

private:
    void   _initializeHarware  (void);
    void   _initializeNetwork  (void);
    void   _initializeWebServer(void);
    
    bool   _onStopSelection    (const String& routeName, const String& stopName);
    void   _getStopNextArrivals(void);
};

#endif // SRC_APP_APPLICATION_H_