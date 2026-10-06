/*
 * File: /src/gtfsUtils.cpp
 * Project: ESP32 TramTracker
 * Created Date: 02-10-2026, 2:41:49 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 11:49:37 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "gtfsUtils.h"
//
#include <stdint.h>
//
#include <gtfs-realtime-light.pb.h>
#include <pb_decode.h>
//
#include "generated_app_config.h"
#include "networkUtils.h"
#include "StopInfo.h"
#include "StopSelection.h"
#include "utils.h"

namespace gtfs
{
    static inline uint64_t nextTimeArrival = 0;
}

struct StopTimeUpdateDecodeContext
{
    const char* stopId;
    int64_t earliestArrival;
};

static bool
stopTimeUpdateCallback(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    auto& context = *static_cast<StopTimeUpdateDecodeContext*>(*arg);
    light_transit_rt_TripUpdate_StopTimeUpdate stopUpdate =
        light_transit_rt_TripUpdate_StopTimeUpdate_init_default;

    if (!pb_decode(stream, light_transit_rt_TripUpdate_StopTimeUpdate_fields, &stopUpdate))
    {
        return false;
    }

    if (!stopUpdate.has_stop_id || strcmp(stopUpdate.stop_id, context.stopId) != 0)
    {
        return true;
    }

    int64_t arrival = 0;
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
    light_transit_rt_FeedEntity entity = light_transit_rt_FeedEntity_init_default;
    app::StopInfo& gtfsContext = * static_cast<app::StopInfo*>(*arg);
    StopTimeUpdateDecodeContext stopUpdateContext{gtfsContext.stopId, 0};
    entity.trip_update.stop_time_update.funcs.decode = &stopTimeUpdateCallback;
    entity.trip_update.stop_time_update.arg = &stopUpdateContext;

    if (!pb_decode(stream, light_transit_rt_FeedEntity_fields, &entity))
    {
        // LOG_PRINTLN("Entity decode failed.")
        return false;
    }

    if (!entity.has_trip_update)
    {
        return true;
    }

    const auto& tripUpdate = entity.trip_update;
    const auto& trip = tripUpdate.trip;

    if (gtfsContext.stopId != nullptr && gtfsContext.routeId[0] != '\0' &&
        (!trip.has_direction_id || strcmp(trip.route_id, gtfsContext.routeId) != 0))
    {
        return true;
    }

    if (stopUpdateContext.earliestArrival > 0 &&
        (gtfsContext.nextArrivalUnix == 0 ||
            stopUpdateContext.earliestArrival < gtfsContext.nextArrivalUnix))
    {
        gtfsContext.nextArrivalUnix = stopUpdateContext.earliestArrival;
    }

    return true;
}

uint64_t
fetchNextArrivalHour(const app::StopSelection& selectedStop, uint64_t now)
{
    uint32_t content_length = 0;
    app::StopInfo gtfsContext{selectedStop};

    if (!openGTFSConnection())
    {
        LOG_PRINTLN("Cannot establish connection")
        return 0;
    } else {
        LOG_PRINTLN("Connection established")
    }

    sendRequest(app::kGtfsTripUpdateEndpoint);
    content_length = getContentLength();

    if (content_length != 0)
    {
        bool ok = parseGTFS(content_length, &tripUpdateEntityCallback, static_cast<void*>(&gtfsContext));
        // if (ok) {
        //     gtfsContext.nextArrivalUnix = 
        // }
    }
    else
    {
        LOG_PRINTLN("No content-length")
    }

    //
    closeGTFSConnection();
    return gtfsContext.nextArrivalUnix;
}

// private

// Network read wrapper to separate network and Protobuf logics.
static bool
nanopbReadCallback(pb_istream_t* stream, pb_byte_t* buffer, size_t count)
{
    (void)stream;
    return readResponseBytes(buffer, count);
}

bool
parseGTFS(const size_t content_length, gtfs::EntityDecoder entityDecoder, void* decoderArg)
{
    pb_istream_t stream = {&nanopbReadCallback, nullptr, content_length};

    light_transit_rt_FeedMessage feed = light_transit_rt_FeedMessage_init_default;
    feed.entity.funcs.decode = entityDecoder;
    feed.entity.arg = decoderArg;

    LOG_PRINTLN("BEGIN DECODING...")
    if (!pb_decode(&stream, light_transit_rt_FeedMessage_fields, &feed))
    {
        LOG_PRINTF("Decode Failing: %s\n", PB_GET_ERROR(&stream))
        return false;
    }
    LOG_PRINTLN("Decode succeed")
    return true;
}
