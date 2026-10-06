/*
 * File: /include/utils.h
 * Project: ESP32 TramTracker
 * Created Date: 18-09-2026, 4:33:19 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 12:10:05 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef UTILS_H_
    #define UTILS_H_

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

#endif // UTILS_H_