/*
 * File: /src/tripUpdate.h
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:37:14 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 3:37:20 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef TRIP_UPDATE_H_
#define TRIP_UPDATE_H_

#include <cstdint>
#include <esp_sntp.h>

time_t fetchNextTimeArrival(const uint32_t stopId, const uint32_t routeId);

#endif // TRIP_UPDATE_H_