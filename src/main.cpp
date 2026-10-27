/*
 * Filename: /src/main.cpp
 * Project: ESP32 TramTracker
 * Created Date: Thursday, January 1st 1970, 1:00:00 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:40:24 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include "app/Application.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace {
    Application app;

#   ifdef TRAM_DEBUG
    constexpr uint16_t    k_page_size     = 256 * 4; // For case Of Print
#   else
    constexpr uint16_t    k_page_size     = 256;
#   endif
    constexpr uint32_t    k_arrival_service_period = 45000; // 45 seconds.
} // namespace

static void gtfsService(void *)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        app.updateArrivals();

        xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(k_arrival_service_period));
    }
}

static void displayService(void*)
{
    for (;;)
    {
        app.updateDisplay();

        utils::task_delay(1);
    }
}

void setup()
{
    INIT_LOG

    app.begin();

    xTaskCreatePinnedToCore(
        gtfsService,
        "nextArrivalService",
        k_page_size * 32, // 8Ko stackDepth if use arduino / espressif value in Byte if use pure freeRtos value in Word (4 bytes)
        nullptr,
        1, // task_priority
        nullptr,
        1 // use core 1 to set More
    ); //

    xTaskCreatePinnedToCore(
        displayService,
        "StopDisplayService",
        k_page_size * 8, // 2Ko stackDepth if use arduino / espressif
        nullptr,
        1, // task_priority
        nullptr,
        1 // use core 1 to set More
    );
}

void loop(void){}
