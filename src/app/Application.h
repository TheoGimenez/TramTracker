/*
 * File: /src/app/Application.h
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 10:04:31 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Saturday, October 31st 2026, 11:18:55 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_APP_APPLICATION_H_
#define SRC_APP_APPLICATION_H_

#include <cstddef>
#include <cstdint>
#include <string>
//
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
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

namespace Application {
    void begin(void);
} // Application

#endif // SRC_APP_APPLICATION_H_