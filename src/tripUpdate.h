/*
 * File: /src/tripUpdate.h
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:37:14 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 11:51:11 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_TRIP_UPDATE_H_
#define SRC_TRIP_UPDATE_H_

#include <cstdint>
#include <optional>
#include <esp_sntp.h>

std::optional<time_t> fetch_next_time_arrival(const uint32_t stopId, const uint32_t routeId);

#endif // SRC_TRIP_UPDATE_H_