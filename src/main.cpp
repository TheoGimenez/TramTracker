/*
 * Filename: /src/main.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Sunday, October 18th 2026, 6:08:22 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include <chrono>
//
#include <Arduino.h>
#include <esp_sntp.h> // use of time_t
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

    //! 
    const char* k_ssid = "TramTracker_Hotspot";
    const char* k_pswd = "tram123";
}
//
namespace app
{
    DisplayController display;   // LED Matrix Controller
    WebServer         webserver; // Webserver to control the display
    // Selected Stop that default value set in include/default_config.h
    selected_stop_s   selected_stop = {0};
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

void stop_selection_callback(const String &selectedRouteName, const String &selectedStopName)
{
    resolve_selected_stop(selectedRouteName.c_str(), selectedStopName.c_str(), app::selected_stop);

    app::display.setTemporaryText(String("Arret :") + selectedStopName + " selectionne ...");
    app::display.setDisplayToTmpText(true, 1);

    //! for debug purpose, fetching should be on another thread.
    const auto next_arrivals = fetch_next_time_arrival(app::selected_stop.route, app::selected_stop.ids, getTime());
    
    String led_text = selectedStopName + " " + selectedStopName;
    if (next_arrivals.has_value())
    {
        const auto &times = next_arrivals.value().next_arrival_times;

        if (times[0])
        {
            led_text += ": " "SENS_A" "-> " + String(times[0] / 60) + ":" + String(times[0] % 60) + ". / ";
        }
        else
        {
            led_text += ": " "SENS_A" "-> " "Pas de Service.";
        }
        if (times[1])
        {
            led_text += ": " "SENS_B" "-> " + String(times[1] / 60) + ":" + String(times[1] % 60) + ".";
        }
        else
        {
            led_text += ": " "SENS_B" "-> " "Pas de Service.";
        }
    }
    else
    {
        led_text += ": " "SENS_A" "-> " "Pas de Service." ": SENS_B" ": " "Pas de Service.";
    }
    app::display.setMainText(led_text);
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
LOG_PRINTF("IP=[%s]",WiFi.localIP().toString())
    app::display.setTemporaryText(WiFi.localIP().toString());
    //
    //* sync time:
    configTzTime(TZ_Europe_Paris, ntp_server);
//! add a getTime() == 0 -> display error on matrix.
    //
    //* Init webserver
    app::webserver.begin(stop_selection_callback);
    app::webserver.setHomePageToStopSelection();
    //
    //* Init network client
    initialize_network();
    //
    //* Init protobuff
    // const auto next_arrivals = fetch_next_time_arrival(app::selected_stop.route, app::selected_stop.ids, getTime());
}

void loop()
{
    app::display.updateDisplay();

    if (digitalRead(hardware::k_boot_button) == LOW)
    {
        app::display.setTemporaryText(WiFi.localIP().toString());
        app::display.setDisplayToTmpText(true, 2);
        delay(100);
    }
}