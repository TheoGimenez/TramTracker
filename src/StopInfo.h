/*
 * File: /src/StopInfo.h
 * Project: ESP32 TramTracker
 * Created Date: 30-09-2026, 12:10:32 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 10:36:37 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef STOP_INFO_H_
#define STOP_INFO_H_

#include <cstdint>
//
#include "StopSelection.h"
#include "route.h"

namespace app
{
    class StopInfo
    {
    public:
        char    stopId[8];   // authorize maximum 9 999 999 stop id.
        char    routeId[16]; // autorize maximum 999 999 999 999 999 route id.
        char    tripId[32];
        int64_t nextArrivalUnix = 0;

    public:
        StopInfo() =delete;
        ~StopInfo() = default;
        StopInfo(const StopSelection &selectedStop) : stopId{0}, routeId{0}, tripId{0}, nextArrivalUnix{0}
        {
            pbNbrToStr(selectedStop.id, stopId, sizeof(stopId));
            pbNbrToStr(selectedStop.routeId, routeId, sizeof(routeId));
        }

        // uint64_t NextArrivalDate;
    };
}

#endif // STOP_INFO_H_