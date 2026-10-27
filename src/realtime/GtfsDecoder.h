/*
 * File: /src/realtime/GtfsDecoder.h
 * Project: ESP32 TramTracker
 * Created Date: 19-10-2026, 9:44:51 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:13:55 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_REALTIME_GTFS_DECODER_H
#define SRC_REALTIME_GTFS_DECODER_H

//!
#include "debug.hpp"
//!
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
//
#include <esp_sntp.h> // use of time_t
#include <pb.h>
#include <pb_decode.h>
#include <gtfs-realtime-light.pb.h>
//
#include "debugLog.h"
#include "rtHttpClient.h"
#include "../app/transitType.h"
#include "utils.h"

using Decoder = bool (*)(pb_istream_t *, const pb_field_iter_t *, void **);

namespace Gtfs {
namespace context {
    
    // inline char* nbr_to_buff(const uint32_t val, char* dest, const size_t capacity);

#   define STOP_ID_MAX_STRING_LEN sizeof(light_transit_rt_TripUpdate_StopTimeUpdate::stop_id)
    class StopIdBuffers
    {
        char m_ids[k_direction_count][STOP_ID_MAX_STRING_LEN];

    public:
        StopIdBuffers(const uint32_t ids[k_direction_count]) : m_ids{{0}, {0}}
        {
            // proto::nbr_to_buff(ids[k_dir_A], m_ids[k_dir_A], STOP_ID_MAX_STRING_LEN);
            // proto::nbr_to_buff(ids[k_dir_B], m_ids[k_dir_B], STOP_ID_MAX_STRING_LEN);

            {
                auto [ptr, ec] = std::to_chars(m_ids[k_dir_A], m_ids[k_dir_A] + STOP_ID_MAX_STRING_LEN, ids[k_dir_A]);
                *ptr = '\0';
            }
            {
                auto [ptr, ec] = std::to_chars(m_ids[k_dir_B], m_ids[k_dir_B] + STOP_ID_MAX_STRING_LEN, ids[k_dir_B]);
                *ptr = '\0';
            }
        }
    public:
        const char *operator[](const size_t n) const { return m_ids[n]; }
    };

    struct feed_s
    {
        const StopIdBuffers stop_ids;
        const char* const   route;
        const int64_t       now;
        int64_t             next_arrivals[k_direction_count];
    };

    struct stop_time_update_s
    {
        const feed_s& feed;
        int64_t       remaining_seconds;
        int8_t        direction;
    };
} // namespace context
    
    class Decoder
    {
        context::feed_s m_context;
        
    public:
                                        Decoder         (context::feed_s feed);
        
    public:
        bool                            decode          (const size_t contentLength);
        std::optional<arrival_update_s> getArrivalUpdate(void) const;
    };
} // namespace Gtfs

#endif// SRC_REALTIME_GTFS_DECODER_H