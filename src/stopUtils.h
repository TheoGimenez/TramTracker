/*
 * File: /src/stopUtils.h
 * Project: ESP32 TramTracker
 * Created Date: 18-09-2026, 10:20:38 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Thursday, October 15th 2026, 2:13:44 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_STOP_UTILS_H_
#define SRC_STOP_UTILS_H_

#include <array>
#include <stdint.h>
#include <stddef.h>
//
#include <esp_sntp.h> // use of time_t

struct selected_stop_s
{
    uint32_t    ids[2]; // if one field have 0, it's a one sided direction stop.
    const char* route;
    const char* name;
};

struct next_arrival_s
{
    time_t next_arrival_times[2]; // store in seconds
};

void                    set_stop_ids          (selected_stop_s& selector);
void                    set_stop_name         (selected_stop_s& selector);
const char*             get_stop_name         (const uint32_t   stopId);

// std::array<uint32_t, 2> get_stop_id           (const uint16_t   routeId, const char* stopName, const size_t stopNameLength);
// std::array<uint32_t, 2> get_stop_id           (const uint16_t   routeId, const char* stopName);
// inline constexpr const char*  get_route_name        (const uint16_t   routeId);

#endif // SRC_STOP_UTILS_H_
