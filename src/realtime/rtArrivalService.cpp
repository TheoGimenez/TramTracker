/*
 * File: /src/realtime/rtArrivalService.cpp
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:36:09 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 1:48:23 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "rtArrivalService.h"


std::optional<arrival_update_s>
rt::fetch_next_arrival(const char* route, const uint32_t stopIds[2], const time_t now)
{
    uint32_t        content_length = 0;
    // context::feed_s context        = {stopIds, route, static_cast<int64_t>(now), {0}};
    Gtfs::Decoder decoder = {Gtfs::context::feed_s{stopIds, route, static_cast<int64_t>(now), {0}}};

    if (route == nullptr || (stopIds[0] == 0 && stopIds[1] == 0))
    {
        return std::nullopt;
    }

    if (!rt::open_gtfs_connection())
    {
        LOG_PRINTLN("Cannot establish connection")
        return std::nullopt;
    }
    else
    {
        LOG_PRINTLN("Connection established")
    }

    rt::send_request(env::k_gtfs_trip_update_endpoint);
    content_length = rt::get_content_length();

    if (content_length != 0)
    {
        if (!decoder.decode(content_length))
        {
            LOG_PRINTLN("DECODE FAILED");
            rt::close_gtfs_connection();
            return std::nullopt;
        }
    }
    else
    {
        LOG_PRINTLN("No content-length")
    }

    //
    rt::close_gtfs_connection();

    return decoder.getArrivalUpdate();
}
