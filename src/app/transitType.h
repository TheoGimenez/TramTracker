/*
 * File: /src/app/transitType.h
 * Project: ESP32 TramTracker
 * Created Date: 18-10-2026, 9:56:18 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Monday, October 19th 2026, 11:49:47 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_APP_TRANSIT_TYPE_H_
#define SRC_APP_TRANSIT_TYPE_H_

#include <cstddef>
#include <cstdint>
//
#include <esp_sntp.h> // use of time_t

inline constexpr uint8_t k_dir_A = 0;
inline constexpr uint8_t k_dir_B = 1;
inline constexpr size_t  k_direction_count = 2;

struct stop_selection_s
{
    uint32_t    ids[k_direction_count]; // if one field have 0, it's a one sided direction stop.
    const char* route;
    const char* name;
};

struct arrival_update_s
{
    // RealtimeStatus status{RealtimeStatus::invalid_feed};
    time_t seconds_until_arrivals[k_direction_count]{};
};

#endif // SRC_APP_TRANSIT_TYPE_H_
