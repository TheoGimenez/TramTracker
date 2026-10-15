/*
 * File: /src/tripUpdate.cpp
 * Project: ESP32 TramTracker
 * Created Date: 06-10-2026, 2:36:09 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Thursday, October 15th 2026, 1:57:32 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "tripUpdate.h"
//
#include <assert.h>
#include <cstdint>
#include <cstring>
#include <optional>
//
#include <pb.h>
#include <pb_decode.h>
#include <gtfs-realtime-light.pb.h>
//
#include "contexts.hpp"
#include "generated_app_config.h"
#include "gtfsUtils.h"
#include "networkUtils.h"
#include "stopUtils.h"
#include "utils.h"

namespace {
    constexpr uint8_t dir_A = 0;
    constexpr uint8_t dir_B = 1;
}

static bool same_string(const char* s1, const char* s2)
{
    return (strcmp(s1, s2) == 0);
}

static bool
stop_time_update_decoder(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    context::stop_time_update_s&               context = *static_cast<context::stop_time_update_s*>(*arg);
    light_transit_rt_TripUpdate_StopTimeUpdate stop_update = light_transit_rt_TripUpdate_StopTimeUpdate_init_default;

    if (!pb_decode(stream, light_transit_rt_TripUpdate_StopTimeUpdate_fields, &stop_update))
    {
        LOG_PRINTF("STOPTIMEUPDATE decode failed: %s, bytes_left=%u\n",
                   PB_GET_ERROR(stream),
                   static_cast<unsigned>(stream->bytes_left))
        return false;
    }

    if (!stop_update.has_stop_id)
    {
        return true;
    }
    if (same_string(stop_update.stop_id, context.feed.stop_ids[dir_A]))
    {
        context.direction = dir_A;
    }
    else if (same_string(stop_update.stop_id, context.feed.stop_ids[dir_B]))
    {
        context.direction = dir_B;
    }
    else
    {
        return true;
    }

    int64_t arrival = 0;
    if (stop_update.has_arrival && stop_update.arrival.has_time)
    {
        arrival = stop_update.arrival.time - context.feed.now;
        if (stop_update.arrival.has_delay)
        {
            arrival += stop_update.arrival.delay;
        }
    }
    else if (stop_update.has_departure && stop_update.departure.has_time)
    {
        arrival = stop_update.departure.time - context.feed.now;
        if (stop_update.departure.has_delay)
        {
            arrival += stop_update.departure.delay;
        }
    }

    if (arrival > 0 && (context.remaining_seconds == 0 || arrival < context.remaining_seconds))
    {
        context.remaining_seconds = arrival;
    }

    return true;
}

static bool
trip_update_decoder(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    context::feed_s&            context             = *static_cast<context::feed_s*>(*arg);
    context::stop_time_update_s stop_update_context = {context, 0, 0};
    light_transit_rt_FeedEntity entity              = light_transit_rt_FeedEntity_init_default;

    entity.trip_update.stop_time_update.funcs.decode = &stop_time_update_decoder;
    entity.trip_update.stop_time_update.arg          = &stop_update_context;

    if (!pb_decode(stream, light_transit_rt_FeedEntity_fields, &entity))
    {
        LOG_PRINTF("Entity decode failed: %s, bytes_left=%u\n", PB_GET_ERROR(stream), static_cast<unsigned>(stream->bytes_left))
        return false;
    }

    if (!entity.has_trip_update)
    {
        return true;
    }

    const auto& trip = entity.trip_update.trip;
    //* Route comparaison ensure that a earliest arrival found in stopTimeUpdate of a given stop is not another shared route.
    if (!trip.has_route_id || !same_string(trip.route_id, context.route))
    {
        return true;
    }

    const auto dir = stop_update_context.direction;
    // Only merge after the route check; keep the earliest valid result across entities.
    if (stop_update_context.remaining_seconds > 0
        && (context.next_arrivals[dir] == 0 || stop_update_context.remaining_seconds < context.next_arrivals[dir]))
    {
        context.next_arrivals[dir] = stop_update_context.remaining_seconds;
    }

    return true;
}

std::optional<next_arrival_s>
fetch_next_time_arrival(const char* route, const uint32_t stopIds[2], const time_t now)
{
    uint32_t        content_length = 0;
    context::feed_s context        = {stopIds, route, static_cast<int64_t>(now), {0}};

    if (route == nullptr || (stopIds[0] == 0 && stopIds[1] == 0))
    {
        return std::nullopt;
    }

    if (!open_gtfs_connection())
    {
        LOG_PRINTLN("Cannot establish connection")
        return std::nullopt;
    }
    else
    {
        LOG_PRINTLN("Connection established")
    }

    send_request(app::k_gtfs_trip_update_endpoint);
    content_length = get_content_length();

    if (content_length != 0)
    {
        if (!gtfs::decode_feed(content_length, &trip_update_decoder, static_cast<void *>(&context)))
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

    return (context.next_arrivals[dir_A] + context.next_arrivals[dir_B])
        ? std::optional<next_arrival_s>{
            {time_t(context.next_arrivals[dir_A]/* - now*/), // substraction to `time_t now` donne in StopTimeUpdate callback
             time_t(context.next_arrivals[dir_B]/* - now*/)}}// substraction to `time_t now` donne in StopTimeUpdate callback
        : std::nullopt;
}
