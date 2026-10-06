/*
 * File: /src/route.h
 * Project: ESP32 TramTracker
 * Created Date: 18-09-2026, 10:20:38 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 10:22:56 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef ROUTE_H_
#define ROUTE_H_

#include <stdint.h>
#include <stddef.h>

const char* getStopName (const uint32_t stop_id);
uint32_t    getStopId   (const char*    stopName, const size_t stopNameLength);
uint32_t    getStopId   (const char*    stopName);
char*       pbNbrToStr  (uint32_t       id, char* dest, const size_t capacity);

#endif // ROUTE_H_
