/*
 * File: /src/LockGuard.h
 * Project: ESP32 TramTracker
 * Created Date: 25-10-2026, 12:36:04 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:41:26 pm
 * Modified By: Theo Gimenez
 * -----
 */

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
//
#include "utils.h"


class LockGuard
{
    SemaphoreHandle_t m_mutex;
    BaseType_t        m_isLock;

public:
    LockGuard           (SemaphoreHandle_t mutex, const uint32_t maxTimeout = portMAX_DELAY);
    LockGuard           (const LockGuard&)  = delete;
    LockGuard           (const LockGuard&&) = delete;
    LockGuard& operator=(const LockGuard&)  = delete;
    ~LockGuard();

public:
    bool isLock         (void) const;
};
