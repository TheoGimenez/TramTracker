/*
 * File: /src/stopUtils.h
 * Project: ESP32 TramTracker
 * Created Date: 18-09-2026, 10:20:38 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 12:02:05 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_STOP_UTILS_H_
#define SRC_STOP_UTILS_H_

#include <stdint.h>
#include <stddef.h>

const char* get_stop_name (const uint32_t stopId);
uint32_t    get_stop_id   (const char*    stopName, const size_t stopNameLength);
uint32_t    get_stop_id   (const char*    stopName);

#endif // SRC_STOP_UTILS_H_
