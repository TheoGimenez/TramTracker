/*
 * File: /include/utils.h
 * Project: ESP32 TramTracker
 * Created Date: 18-09-2026, 4:33:19 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 11:52:25 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef INCLUDE_UTILS_H_
    #define INCLUDE_UTILS_H_

    #ifdef TRAM_DEBUG
        #include <Arduino.h>
        #define LOG_PRINT(...) Serial.print(__VA_ARGS__);
        #define LOG_PRINTLN(...) Serial.println(__VA_ARGS__);
        #define LOG_PRINTF(...) Serial.printf(__VA_ARGS__);
    #else
        #define LOG_PRINT(...)
        #define LOG_PRINTLN(...)
        #define LOG_PRINTF(...)
    #endif

#endif // INCLUDE_UTILS_H_