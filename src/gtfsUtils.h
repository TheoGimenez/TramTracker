/*
 * File: /src/gtfsUtils.h
 * Project: ESP32 TramTracker
 * Created Date: 02-10-2026, 12:55:11 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Thursday, October 15th 2026, 1:01:50 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_GTFS_UTILS_H_
#define SRC_GTFS_UTILS_H_
//
#include <stdint.h>
//
#include <pb.h>
//

namespace gtfs
{
    using EntityDecoder = bool (*)(pb_istream_t *, const pb_field_iter_t *, void **);
    
    char* pb_nbr_to_str (const uint32_t id, char *dest, const size_t capacity);
    bool  decode_feed   (const size_t   content_length, gtfs::EntityDecoder entityDecoder, void* decoderArgs);
}

#endif // SRC_GTFS_UTILS_H_