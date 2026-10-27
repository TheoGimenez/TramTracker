/*
 * File: /src/realtime/rtHttpClient.h
 * Project: ESP32 TramTracker
 * Created Date: 19-10-2026, 9:27:16 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:15:30 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_REALTIME_RT_HTTP_CLIENT_H_
#define SRC_REALTIME_RT_HTTP_CLIENT_H_

#include <cstddef>
#include <cstdint>
//
#include <WiFiClientSecure.h>
//
#include "debugLog.h"
#include "generated_app_config.h"
#include "utils.h"

namespace rt {

void     initialize_network   (void);
int      open_gtfs_connection (void);
void     close_gtfs_connection(void);
bool     read_response_bytes  (uint8_t* buffer, size_t count);
void     send_request         (const char *endpoint);
uint32_t get_content_length   (void);

} // namespace rt

#endif // SRC_REALTIME_RT_HTTP_CLIENT_H_