/*
 * File: /src/gtfsUtils.h
 * Project: ESP32 TramTracker
 * Created Date: 02-10-2026, 12:55:11 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 4:15:02 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef GTFS_UTILS_H_
#define GTFS_UTILS_H_
//
#include <stdint.h>
//
#include <pb.h>
//

namespace gtfs
{
    using EntityDecoder = bool (*)(pb_istream_t *, const pb_field_iter_t *, void **);
}

char* pbNbrToStr (const uint32_t id, char *dest, const size_t capacity);
bool  parseGTFS(const size_t   content_length, gtfs::EntityDecoder entityDecoder, void* decoderArgs);

#endif // GTFS_UTILS_H_