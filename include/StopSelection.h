/*
 * File: /include/StopSelection.h
 * Project: ESP32 TramTracker
 * Created Date: 30-09-2026, 12:10:32 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 6th 2026, 11:42:20 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef STOP_SELECTION_H_
#define STOP_SELECTION_H_

#include <cstdint>

namespace app
{
    struct StopSelection
    {
    public:
        uint32_t id;
        uint32_t routeId;
        // char s_id[8];       // authorize maximum 9 999 999 stop id.
        // char s_routeId[16]; // authorize maximum 999 999 999 999 999 route id.

        char *const fullName; // pointer to the name in /include/generated_stops.h

        // uint64_t NextArrivalDate;
    };

    inline StopSelection selectedStop = {0};
        // uint64_t NextArrivalDate;
}

#endif // STOP_SELECTION_H_