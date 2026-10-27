/*
 * File: /src/app/Application.h
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:31 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:31:58 pm
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

#include "../transit/transitCatalog.h"

#define TZ_Europe_Paris PSTR("CET-1CEST,M3.5.0,M10.5.0/3")

namespace {
constexpr const char* k_ntp_timezone    = TZ_Europe_Paris;
constexpr const char* k_ntp_server      = "pool.ntp.org";
constexpr const char* k_text_no_service = "Pas de service.";

namespace hardware {
    constexpr uint8_t k_esp_led = 2;     // esp integrated blue led
    constexpr uint8_t k_boot_button = 0; // esp boot button
} // namespace hardware
} // namespace

class Application
{
    DisplayContr      m_display;        // LED matrix controller
    web::AsyncServer  m_webServer;
    // TransitCatalog    m_transitCatalog;
    // stop_selection_s  m_selectedStop{transit::resolve_selected_stop("3", "Place Carnot").value()};
    stop_selection_s  m_selected_stop;
    SemaphoreHandle_t m_stop_mutex;
public:
                Application               ();

public:
// Application():m_display{}, m_webServer{}, m_selectedStop{transit::resolve_selected_stop("3", "Place Carnot").value()} {}
    void        begin                     (void);
    void        updateDisplay             (void);
    void        updateArrivals            (void);

private:
    void        _initializeHarware        (void);
    void        _initializeNetwork        (void);
    void        _initializeWebServer      (void);

    bool        _onStopSelection          (const char* routeName, const char* stopName);
    void        _stopNextArrivalsService  (void);
    std::string _formatNextArrivalToString(const time_t remainingSecs[2]);
};

#endif // SRC_APP_APPLICATION_H_