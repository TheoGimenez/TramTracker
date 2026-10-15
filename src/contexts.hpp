/*
 * File: /src/contexts.hpp
 * Project: ESP32 TramTracker
 * Created Date: 15-10-2026, 12:09:41 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Thursday, October 15th 2026, 2:00:32 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_CONTEXTS_HPP_
#define SRC_CONTEXTS_HPP_

#include <cstddef>
#include <cstdint>
//
#include <gtfs-realtime-light.pb.h>
//
#include "gtfsUtils.h"

#define STOP_ID_MAX_STRING_LEN sizeof(light_transit_rt_TripUpdate_StopTimeUpdate::stop_id)

namespace context
{

class StopIdBuffers
{
    char m_ids[2][STOP_ID_MAX_STRING_LEN];
public:
    StopIdBuffers(const uint32_t ids[2]) : m_ids{{0}, {0}}
    {
        gtfs::pb_nbr_to_str(ids[0], m_ids[0], STOP_ID_MAX_STRING_LEN);
        gtfs::pb_nbr_to_str(ids[1], m_ids[1], STOP_ID_MAX_STRING_LEN);
    }
    const char* operator[](const size_t n) const {return m_ids[n];}
};

struct feed_s
{
    const StopIdBuffers stop_ids;
    const char* const   route;
    const int64_t       now;
    int64_t             next_arrivals[2];
};

struct stop_time_update_s
{
    const feed_s& feed;
    int64_t       remaining_seconds;
    int8_t        direction;
};

} // namespace context

#endif // SRC_CONTEXTS_HPP_