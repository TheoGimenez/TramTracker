/*
 * File: /src/reatltime/rtArrivalService.h
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:37:14 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 2:06:03 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_REALTIME_RT_ARRIVAL_SERVICE_H_
#define SRC_REALTIME_RT_ARRIVAL_SERVICE_H_

#include <cstdint>
#include <optional>
//
#include <esp_sntp.h> // use of time_t
//
#include "GtfsDecoder.h"
#include "rtHttpClient.h"
#include "../app/transitType.h"

namespace rt
{

    std::optional<arrival_update_s> fetch_next_arrival(const char *route, const uint32_t stopIds[2], const time_t now);

} // namespace realtime

#endif // SRC_REALTIME_RT_ARRIVAL_SERVICE_H_