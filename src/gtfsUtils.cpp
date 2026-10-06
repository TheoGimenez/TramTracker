/*
 * File: /src/gtfsUtils.cpp
 * Project: ESP32 TramTracker
 * Created Date: 02-10-2026, 2:41:49 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 4:14:53 pm
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
