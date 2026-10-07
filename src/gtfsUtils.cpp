/*
 * File: /src/gtfsUtils.cpp
 * Project: ESP32 TramTracker
 * Created Date: 02-10-2026, 2:41:49 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 10:55:30 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "gtfsUtils.h"
//
#include <assert.h>
#include <cstdio>
#include <inttypes.h> // requiered for PRIu32
//
#include <gtfs-realtime-light.pb.h>
#include <pb_decode.h>
//
#include "networkUtils.h"
#include "utils.h"


char*
gtfs::pb_nbr_to_str(const uint32_t id, char* dest, const size_t capacity)
{
    assert((dest != nullptr && (dest[capacity-1] == 0 || dest[capacity-1] != 0)));

    // doesn't test if dest a the right size, that the responsability of the fct user.
    const uint32_t written = snprintf(dest, capacity, "%" PRIu32, id);
    dest[written < 0 ? 0 : written] = '\0';

    return dest;
}

static bool
feed_header_decoder(pb_istream_t* stream, const pb_field_iter_t*, void**)
{
    light_transit_rt_FeedHeader feed_header = light_transit_rt_FeedHeader_init_default;

    if (!pb_decode(stream,light_transit_rt_FeedHeader_fields, &feed_header))
    {
        LOG_PRINTF("FeedHeader decode failed: %s, bytes_left=%u\n",
                   PB_GET_ERROR(stream),
                   static_cast<unsigned>(stream->bytes_left))
        return false;
    }

    // If dataset incrementaity fetching isn't define to full, ignore all stream.
    if (!feed_header.has_incrementality ||
        feed_header.incrementality != light_transit_rt_FeedHeader_Incrementality_FULL_DATASET)
    {
        return false;
    }
    return true;
}

// Network read wrapper to separate network and Protobuf logics.
static bool
nanopb_read_callback(pb_istream_t* stream, pb_byte_t* buffer, size_t count)
{
    (void)stream;
    return read_response_bytes(buffer, count);
}

bool
gtfs::decode_feed(const size_t contentLength, gtfs::EntityDecoder entityDecoder, void* decoderArg)
{
    pb_istream_t                 stream = {&nanopb_read_callback, nullptr, contentLength};
    light_transit_rt_FeedMessage feed   = light_transit_rt_FeedMessage_init_default;

    feed.entity.funcs.decode = entityDecoder;
    feed.entity.arg          = decoderArg;

    if (!pb_decode(&stream, light_transit_rt_FeedMessage_fields, &feed))
    {
        LOG_PRINTF("Decode Failing: %s\n", PB_GET_ERROR(&stream))
        return false;
    }
    return true;
}
