/*
 * Filename: /src/main.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 12:07:07 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include <Arduino.h>
#include <esp_sntp.h> // use of time_t
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
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

//
namespace app
{
    DisplayController display;   // LED Matrix Controller
    WebServer         webserver; // Webserver to control the display
    
    // anonym struct that only used here
    struct
    {
        uint32_t    id                   = 0;
        uint32_t    route_id             = 0;
        time_t      next_arrival_time[2] = {0}; // next arrival time for the 2 direction (0 or 1)
        char* const fullName             = nullptr;
    } selectedStop;
}

// void set_selected_stop(const uint16_t stopId, const uint16_t routeId) //!prototype
// {
    // app::context.selectedStop.id       = stopId;
    // app::context.selectedStop.route_id = routeId;
    // app::context.selectedStop.fullName = get_stop_name(stopId);
// }

void setup()
{
#   ifdef TRAM_DEBUG
    Serial.begin(115200);
#   endif
    pinMode(hardware::k_esp_led, OUTPUT);
    pinMode(hardware::k_boot_button, INPUT_PULLDOWN);
    //
    //* Setup display
    app::display.begin();
    //
    //* Setup wifi
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
    //
    //* Setup webserver
    app::webserver.begin([](String selectedLine)
                         {
        app::display.setMainText(selectedLine);
        app::display.setDisplayToMainText();
    });
    //
    //* Setup network client
    initialize_network();
    //* Setup selected stop
    app::selectedStop.id = get_stop_id("Place Carnot"); // place carnot
    app::selectedStop.route_id = 3; // Ligne 3
    //
    //* Setup protobuff
    // first call
    // time_t nextArrival =
    auto next_arrival = fetch_next_time_arrival(app::selectedStop.id, app::selectedStop.route_id);
    if (next_arrival.has_value())
    {
        LOG_PRINT("NEXT ARRIVAL=")
        LOG_PRINTLN(next_arrival.value())
    }
    else
    {
        LOG_PRINTLN("no parsing")
    }
    // String time((nexArrival - now()) * 1000 / 60); 
    // app::display.setMainText(app::selectedStop.fullName + String(" dans " + time + "min"));
    
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