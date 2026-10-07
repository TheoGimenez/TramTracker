/*
 * File: /src/networkUtils.h
 * Project: ESP32 TramTracker
 * Created Date: 17-09-2026, 3:46:10 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Wednesday, October 7th 2026, 11:51:32 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_NETWORK_UTILS_H_
#define SRC_NETWORK_UTILS_H_

#include <stdint.h>
#include <cstdlib>

void     initialize_network   (void);
int      open_gtfs_connection (void);
void     close_gtfs_connection(void);
bool     read_response_bytes  (uint8_t* buffer, size_t count);
void     send_request         (const char *endpoint);
uint32_t get_content_length   (void);

#endif // SRC_NETWORK_UTILS_H_