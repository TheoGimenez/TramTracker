/*
 * Filename: /src/main.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Thursday, October 15th 2026, 2:04:59 am
 * Modified By: Theo Gimenez
 * -----
 */

#include <Arduino.h>
#include <chrono>
#include <esp_sntp.h> // use of time_t
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <time.h>
#include <WiFi.h>
//
#include "DisplayController.h"
#include "generated_app_config.h"
#include "generated_stops.h"
#include "networkUtils.h"
#include "stopUtils.h"
#include "tripUpdate.h"
#include "utils.h"
#include "WebServer.h"
#include "default_config.h"

namespace hardware
{
    constexpr uint8_t k_esp_led = 2;     // esp integrated blue led
    constexpr uint8_t k_boot_button = 0; // esp boot button
} // namespace hardware
namespace
{	
#   define TZ_Europe_Paris PSTR("CET-1CEST,M3.5.0,M10.5.0/3")
    // const char*     ntp_timezone        = TZ_Europe_Paris "CET-1CEST,M3.5.0,M10.5.0/3";
    const char*     ntp_server          = "pool.ntp.org";
    constexpr long  gmt_offset_sec      = 3600;
    constexpr int   daylight_offset_sec = 3600;
}
//
namespace app
{
    DisplayController display;   // LED Matrix Controller
    WebServer         webserver; // Webserver to control the display
    // Selected Stop that default value set in include/default_config.h
    selected_stop_s   selected_stop = {{0}, TRAM_TRACKER_DEFAULT_ROUTE_NAME, TRAM_TRACKER_DEFAULT_STOP_NAME};
}

time_t getTime()
{
    time_t    now;
    struct tm time_info;
    
    if (!getLocalTime(&time_info))
    {
        return 0;
    }
    time(&now);
    return now;
}

void setup()
{
#   ifdef TRAM_DEBUG
    Serial.begin(115200);
#   endif
    pinMode(hardware::k_esp_led, OUTPUT);
    pinMode(hardware::k_boot_button, INPUT_PULLDOWN);
    //
    //* Init display
    app::display.begin();
    //
    //* Init wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(app::k_wifi_ssid, app::k_wifi_password);
    WiFi.setAutoReconnect(true);
    while (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
    app::display.setTemporaryText(WiFi.localIP().toString());
    //* sync time:
    configTzTime(TZ_Europe_Paris, ntp_server);

    //* Init webserver
    app::webserver.begin([](String selectedLine)
                         {
        app::display.setMainText(selectedLine);
        app::display.setDisplayToMainText();
    });
    //
    //* Init network client
    initialize_network();

    //* Init selected stop (debug purpose)
    set_stop_ids(app::selected_stop);
    set_stop_name(app::selected_stop);

    //
    //* Init protobuff
    const auto next_arrivals = fetch_next_time_arrival(app::selected_stop.route, app::selected_stop.ids, getTime());

    if (next_arrivals.has_value())
    {
        LOG_PRINT("NEXT ARRIVAL=[")
        LOG_PRINT(next_arrivals.value().next_arrival_times[0])
        LOG_PRINT("], [")
        LOG_PRINT(next_arrivals.value().next_arrival_times[1])
        LOG_PRINTLN("]")
    }
    else
    {
        LOG_PRINTLN("no parsing")
    }

    
    // app::display.setMainText("Aucun stop selectionner : connectez vous sur " String(WiFi.localIP().toString()));
    // app::display.setDisplayToMainText(true, 0);
}

void loop()
{
    app::display.updateDisplay();

    if (digitalRead(hardware::k_boot_button) == LOW)
    {
        app::display.setDisplayToTmpText(true, 2);
        delay(100);
    }
}