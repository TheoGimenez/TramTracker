/*
 * File: /src/tripUpdate.h
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:37:14 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Monday, October 12th 2026, 6:56:01 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_TRIP_UPDATE_H_
#define SRC_TRIP_UPDATE_H_

#include <cstdint>
#include <esp_sntp.h>
#include <optional>
#include <time.h>
//
#include "stopUtils.h"

std::optional<next_arrival_s> fetch_next_time_arrival(const char* route, const uint32_t stopIds[2], const time_t now);

#endif // SRC_TRIP_UPDATE_H_