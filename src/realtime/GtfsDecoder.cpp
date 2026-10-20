/*
 * File: /src/realtime/GtfsDecoder.cpp
 * Project: ESP32 TramTracker
 * Created Date: 19-10-2026, 9:58:03 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 1:48:38 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "GtfsDecoder.h"

using namespace Gtfs;

static bool
stop_time_update_decoder(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
{
    // if (arg == nullptr || *arg == nullptr)
    // {
    //     LOG_PRINTLN("STOPTIMEUPDATE decoder has no context")
    //     return false;
    // }

    context::stop_time_update_s&               context     = *static_cast<context::stop_time_update_s*>(*arg);
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
    if (same_string(stop_update.stop_id, context.feed.stop_ids[k_dir_A]))
    {
        context.direction = k_dir_A;
    }
    else if (same_string(stop_update.stop_id, context.feed.stop_ids[k_dir_B]))
    {
        context.direction = k_dir_B;
    }
    else
    {
        return true;
    }

    int64_t arrival = 0;
    if (stop_update.has_arrival && stop_update.arrival.has_time)
    {
        arrival = stop_update.arrival.time - context.feed.now;
    }
    else if (stop_update.has_departure && stop_update.departure.has_time)
    {
        arrival = stop_update.departure.time - context.feed.now;
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
    if (arg == nullptr || *arg == nullptr)
    {
        LOG_PRINTLN("TripUpdate decoder has no context")
        return false;
    }

    context::feed_s&            context             = *static_cast<context::feed_s*>(*arg);
    context::stop_time_update_s stop_update_context = {context, int64_t(0), int8_t(0)};
    light_transit_rt_FeedEntity entity              = light_transit_rt_FeedEntity_init_default;

    entity.trip_update.stop_time_update.funcs.decode = &stop_time_update_decoder;
    entity.trip_update.stop_time_update.arg          = &stop_update_context;

    if (!pb_decode_ex(stream, light_transit_rt_FeedEntity_fields, &entity, PB_DECODE_NOINIT))
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

// bool
// GtfsDecoder::feed_header_decoder(pb_istream_t* stream, const pb_field_iter_t*, void**)
// {
//     light_transit_rt_FeedHeader feed_header = light_transit_rt_FeedHeader_init_default;

//     if (!pb_decode(stream,light_transit_rt_FeedHeader_fields, &feed_header))
//     {
//         LOG_PRINTF("FeedHeader decode failed: %s, bytes_left=%u\n",
//                    PB_GET_ERROR(stream),
//                    static_cast<unsigned>(stream->bytes_left))
//         return false;
//     }

//     // If dataset incrementaity fetching isn't define to full, ignore all stream.
//     if (!feed_header.has_incrementality ||
//         feed_header.incrementality != light_transit_rt_FeedHeader_Incrementality_FULL_DATASET)
//     {
//         return false;
//     }
//     return true;
// }

// static bool entityDecoder(pb_istream_t* stream, const pb_field_iter_t*, void** arg)
// {
//     return trip_update_decoder(stream, nullptr, arg);
// }

// Network read wrapper to separate network and Protobuf logics.
static bool nanopb_read_callback(pb_istream_t* stream, pb_byte_t* buffer, size_t count)
{
    (void)stream;
    return rt::read_response_bytes(buffer, count);
}

Gtfs::Decoder::Decoder(context::feed_s feed) : m_context{feed}{}

bool
Gtfs::Decoder::decode(const size_t contentLength)
{
    pb_istream_t                 stream = {&nanopb_read_callback, nullptr, contentLength};
    light_transit_rt_FeedMessage feed   = light_transit_rt_FeedMessage_init_default;

    feed.entity.funcs.decode = trip_update_decoder;
    feed.entity.arg          = &m_context;

    if (!pb_decode(&stream, light_transit_rt_FeedMessage_fields, &feed))
    {
        LOG_PRINTF("Decode Failing: %s\n", PB_GET_ERROR(&stream))
        return false;
    }

    return true;
}


std::optional<arrival_update_s>
Gtfs::Decoder::getArrivalUpdate(void) const
{

    if (m_context.next_arrivals[k_dir_A] + m_context.next_arrivals[k_dir_B])
    {
        return std::optional<arrival_update_s>{{time_t(m_context.next_arrivals[k_dir_A]), time_t(m_context.next_arrivals[k_dir_B])}};
    }

    return std::nullopt;
}
