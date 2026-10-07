/*
 * File: /src/tripUpdate.cpp
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:36:09 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 11:23:31 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "tripUpdate.h"
//
#include <assert.h>
#include <cstdint>
#include <optional>
//
#include <pb.h>
#include <pb_decode.h>
#include <gtfs-realtime-light.pb.h>
//
#include "generated_app_config.h"
#include "gtfsUtils.h"
#include "networkUtils.h"
#include "utils.h"

struct FeedDecodeContext
{
    char    stopId[16];  // after finding stop that ad long id (9) `S_0:00215` set buffer to 16 from 8
    char    routeId[16]; // 
    int64_t nextArrival; // unix time from epoch (01/01/1970)

    FeedDecodeContext(const uint32_t _stopId, const uint32_t _routeId) :
        stopId{0},
        routeId{0},
        nextArrival{0}
    {
        gtfs::pb_nbr_to_str(_stopId, stopId, sizeof(stopId));
        gtfs::pb_nbr_to_str(_routeId, routeId, sizeof(routeId));
    }
};

struct stop_time_update_decode_context
{
    const char* stopId;
    int64_t earliestArrival;
};

static bool
stop_time_update_callback(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    stop_time_update_decode_context&           context     = *static_cast<stop_time_update_decode_context*>(*arg);
    light_transit_rt_TripUpdate_StopTimeUpdate stop_update = light_transit_rt_TripUpdate_StopTimeUpdate_init_default;
    int64_t                                    arrival;

    if (!pb_decode(stream, light_transit_rt_TripUpdate_StopTimeUpdate_fields, &stop_update))
    {
        LOG_PRINTF("STOPTIMEUPDATE decode failed: %s, bytes_left=%u\n",
                   PB_GET_ERROR(stream),
                   static_cast<unsigned>(stream->bytes_left))
        return false;
    }
    
    if (!stop_update.has_stop_id || strcmp(stop_update.stop_id, context.stopId) != 0)
    {
        return true;
    }

    arrival = 0;
    if (stop_update.has_arrival && stop_update.arrival.has_time)
    {
        arrival = stop_update.arrival.time;
    }
    else if (stop_update.has_departure && stop_update.departure.has_time)
    {
        arrival = stop_update.departure.time;
    }

    if (arrival > 0 &&
        (context.earliestArrival == 0 || arrival < context.earliestArrival))
    {
        context.earliestArrival = arrival;
    }

    return true;
}

static bool
trip_update_entity_callback(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    FeedDecodeContext&              context             = *static_cast<FeedDecodeContext*>(*arg);
    stop_time_update_decode_context stop_update_context = {context.stopId, 0};
    light_transit_rt_FeedEntity     entity              = light_transit_rt_FeedEntity_init_default;

    entity.trip_update.stop_time_update.funcs.decode = &stop_time_update_callback;
    entity.trip_update.stop_time_update.arg          = &stop_update_context;

        if (!pb_decode(stream, light_transit_rt_FeedEntity_fields, &entity))
    {
        LOG_PRINTF("Entity decode failed: %s, bytes_left=%u\n",
                   PB_GET_ERROR(stream),
                   static_cast<unsigned>(stream->bytes_left))
        return false;
    }

    if (!entity.has_trip_update)
    {
        return true;
    }

    const auto& trip_update = entity.trip_update;
    const auto& trip = trip_update.trip;

    if (!trip.has_route_id || strcmp(trip.route_id, context.routeId) != 0)
    {
        return true;
    }
    // Only merge after the route check; keep the earliest valid result across entities.
    if (stop_update_context.earliestArrival > 0
        && (context.nextArrival == 0 || stop_update_context.earliestArrival < context.nextArrival))
    {
        context.nextArrival = stop_update_context.earliestArrival;
    }

    return true;
}

std::optional<time_t>
fetch_next_time_arrival(const uint32_t stopId, const uint32_t routeId)
{
    assert((stopId != 0 && routeId != 0));

    uint32_t          content_length = 0;
    FeedDecodeContext context        = {stopId, routeId};

    if (!open_gtfs_connection())
    {
        LOG_PRINTLN("Cannot establish connection")
        return 0;
    }
    else
    {
        LOG_PRINTLN("Connection established")
    }

    send_request(app::k_gtfs_trip_update_endpoint);
    content_length = get_content_length();

    if (content_length != 0)
    {
        if (!gtfs::decode_feed(content_length, &trip_update_entity_callback, static_cast<void *>(&context)))
        {
            close_gtfs_connection();
            return std::nullopt;
        }
    }
    else
    {
        LOG_PRINTLN("No content-length")
    }

    //
    close_gtfs_connection();
    //! return time_t(context.nextArrival - time::now());
    return std::optional<time_t>{context.nextArrival};
}
