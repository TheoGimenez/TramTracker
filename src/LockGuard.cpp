/*
 * File: /src/LockGuard.cpp
 * Project: ESP32 TramTracker
 * Created Date: 25-10-2026, 11:42:47 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Sunday, October 25th 2026, 11:44:15 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "LockGuard.h"

LockGuard::LockGuard(SemaphoreHandle_t mutex, const uint32_t maxTimeout) : m_mutex{mutex}
{
    m_isLock = xSemaphoreTake(m_mutex, maxTimeout);
}

LockGuard::~LockGuard()
{
    if (isLock())
    {
        xSemaphoreGive(m_mutex);
    }
}

bool
LockGuard::isLock(void) const
{
    return (m_isLock == pdTRUE);
}