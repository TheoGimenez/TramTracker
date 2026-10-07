/*
 * File: /src/gtfsUtils.cpp
 * Project: ESP32 TramTracker
 * Created Date: 02-10-2026, 2:41:49 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 9:55:05 am
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
pbNbrToStr(const uint32_t id, char* dest, const size_t capacity)
{
    assert(dest != nullptr);
    // memset(dest, 0, capacity);

    const uint32_t written = snprintf(dest, capacity, "%" PRIu32, id);
    dest[written < 0 ? 0 : written] = '\0';

    return dest;
}

static bool
feedHeaderDecoder(pb_istream_t* stream, const pb_field_iter_t*, void**)
{
    light_transit_rt_FeedHeader feedHeader = light_transit_rt_FeedHeader_init_default;

    if (!pb_decode(stream,light_transit_rt_FeedHeader_fields, &feedHeader))
    {
        LOG_PRINTF("FeedHeader decode failed: %s, bytes_left=%u\n",
                   PB_GET_ERROR(stream),
                   static_cast<unsigned>(stream->bytes_left))
        return false;
    }

    // If dataset incrementaity fetching isn't define to full, ignore all stream.
    if (
        !feedHeader.has_incrementality ||
        feedHeader.incrementality != light_transit_rt_FeedHeader_Incrementality_FULL_DATASET)
    {
        return false;
    }

    return true;
}

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
