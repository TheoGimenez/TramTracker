/*
 * File: /src/tripUpdate.cpp
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:36:09 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 10:11:59 am
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
public:
    FeedDecodeContext(const uint32_t _stopId, const uint32_t _routeId) :
        stopId{0},
        routeId{0},
        nextArrival{0}
    {
        pbNbrToStr(_stopId, stopId, sizeof(stopId));
        pbNbrToStr(_routeId, routeId, sizeof(routeId));
    }
};

struct StopTimeUpdateDecodeContext
{
    const char* stopId;
    int64_t earliestArrival;
};

static bool
stopTimeUpdateCallback(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    StopTimeUpdateDecodeContext&               context    = *static_cast<StopTimeUpdateDecodeContext*>(*arg);
    light_transit_rt_TripUpdate_StopTimeUpdate stopUpdate = light_transit_rt_TripUpdate_StopTimeUpdate_init_default;
    int64_t                                    arrival;

    if (!pb_decode(stream, light_transit_rt_TripUpdate_StopTimeUpdate_fields, &stopUpdate))
    {
        LOG_PRINTF("STOPTIMEUPDATE decode failed: %s, bytes_left=%u\n",
                   PB_GET_ERROR(stream),
                   static_cast<unsigned>(stream->bytes_left))
        return false;
    }
    
    if (!stopUpdate.has_stop_id || strcmp(stopUpdate.stop_id, context.stopId) != 0)
    {
        return true;
    }

    arrival = 0;
    if (stopUpdate.has_arrival && stopUpdate.arrival.has_time)
    {
        arrival = stopUpdate.arrival.time;
    }
    else if (stopUpdate.has_departure && stopUpdate.departure.has_time)
    {
        arrival = stopUpdate.departure.time;
    }

    if (arrival > 0 &&
        (context.earliestArrival == 0 || arrival < context.earliestArrival))
    {
        context.earliestArrival = arrival;
    }

    return true;
}

static bool
tripUpdateEntityCallback(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    FeedDecodeContext&          context           = *static_cast<FeedDecodeContext*>(*arg);
    StopTimeUpdateDecodeContext stopUpdateContext = {context.stopId, 0};
    light_transit_rt_FeedEntity entity            = light_transit_rt_FeedEntity_init_default;

    entity.trip_update.stop_time_update.funcs.decode = &stopTimeUpdateCallback;
    entity.trip_update.stop_time_update.arg = &stopUpdateContext;

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

    const auto& tripUpdate = entity.trip_update;
    const auto& trip = tripUpdate.trip;

    if (!trip.has_route_id || strcmp(trip.route_id, context.routeId) != 0)
    {
        return true;
    }
    // Only merge after the route check; keep the earliest valid result across entities.
    if (stopUpdateContext.earliestArrival > 0 &&
        (context.nextArrival == 0 || stopUpdateContext.earliestArrival < context.nextArrival))
    {
        context.nextArrival = stopUpdateContext.earliestArrival;
    }

    return true;
}

std::optional<time_t>
fetchNextTimeArrival(const uint32_t stopId, const uint32_t routeId)
{
    assert((stopId != 0 && routeId != 0));
    uint32_t content_length = 0;
    FeedDecodeContext context{stopId, routeId};
    std::optional<time_t> optional_return;

    if (!openGTFSConnection())
    {
        LOG_PRINTLN("Cannot establish connection")
        return 0;
    }
    else
    {
        LOG_PRINTLN("Connection established")
    }

    sendRequest(app::kGtfsTripUpdateEndpoint);
    content_length = getContentLength();

    if (content_length != 0)
    {
        if (!parseGTFS(content_length, &tripUpdateEntityCallback, static_cast<void *>(&context)))
        {
            closeGTFSConnection();
            return std::nullopt;
        }
    }
    else
    {
        LOG_PRINTLN("No content-length")
    }

    //
    closeGTFSConnection();
    //! return time_t(context.nextArrival - time::now());
    return std::optional<time_t>{context.nextArrival};
}
