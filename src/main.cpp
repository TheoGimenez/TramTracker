/*
 * Filename: /home/theo/Documents/PlatformIO/Projects/TramTracker/src/main.cpp
 * Path: /home/theo/Documents/PlatformIO/Projects/TramTracker/src
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 *
 * Copyright (c) 2026 Your Company
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
#include "route.h"
#include "tripUpdate.h"
#include "utils.h"
#include "webServer.h"

namespace hardware
{
    constexpr uint8_t k_esp_led = 2;     // esp integrated blue led
    constexpr uint8_t k_boot_button = 0; // esp boot button
} // namespace hardware

//
namespace app
{
    DisplayController display; // LED Matrix Controller
    WebServer webserver;       // Webserver to control the display
    
    struct
    {
        uint32_t    id              = 0;
        uint32_t    routeId         = 0;
        time_t      nextArrivalTime = 0;
        time_t      NextArrivalDate = 0;
        char* const fullName        = nullptr;
    } selectedStop;
}

// void setSelectedStop(const uint16_t stopId, const uint16_t routeId) //!prototype
// {
    // app::context.selectedStop.id       = stopId;
    // app::context.selectedStop.routeId  = routeId;
    // app::context.selectedStop.fullName = getStopName(stopId);
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
    WiFi.begin(app::kWifiSsid, app::kWifiPassword);
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
    initializeNetwork();
    //* Setup selected stop
    app::selectedStop.id = getStopId("Place Carnot"); // place carnot
    app::selectedStop.routeId = 3; // Ligne 3
    //
    //* Setup protobuff
    // first call
    // time_t nextArrival =
    auto nextArrival = fetchNextTimeArrival(app::selectedStop.id, app::selectedStop.routeId);
    if (nextArrival.has_value())
    {
        LOG_PRINT("NEXT ARRIVAL=")
        LOG_PRINTLN(nextArrival.value())
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