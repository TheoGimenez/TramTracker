/*
 * File: /src/networkUtils.h
 * Project: ESP32 TramTracker
 * Created Date: 17-09-2026, 3:46:10 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 12:08:21 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef NETWORK_UTILS_CPP_
#define NETWORK_UTILS_CPP_

#include <stdint.h>
#include <cstdlib>

void     initializeNetwork  (void);
int      openGTFSConnection (void);
void     closeGTFSConnection(void);
bool     readResponseBytes  (uint8_t* buffer, size_t count);
void     sendRequest        (const char *endpoint);
uint32_t getContentLength   (void);

#endif // NETWORK_UTILS_CPP_