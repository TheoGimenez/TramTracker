/*
 * File: route.cpp
 * Project: ESP32 TramTracker
 * File Created: Wednesday, 30th September 2026 9:23:44, pm
 * Author: Nathan Fain
 * -----
 * Last Modified: Wednesday, October 7th 2026, 12:01:46 pm
 * Modified By: Theo Gimenez
 * -----
 * Description: 
 */

#include "stopUtils.h"
//
#include <cstring>
#include <stdio.h>
#include <stdint.h>
//
#include "generated_stops.h"

const char*
get_stop_name(const uint32_t stopId)
{
    // binary search from the array.
    size_t left  = 0;
    size_t right = gtfs::k_stop_count;

    while (left < right)
    {
        size_t middle = left + (right - left) / 2;

        if (gtfs::k_stops[middle].id == stopId)
        {
            return gtfs::k_stop_names + gtfs::k_stops[middle].name_offset;
        }

        if (gtfs::k_stops[middle].id < stopId)
            left = middle + 1;
        else
            right = middle;
    }

    return nullptr;
}

// can't differ bus stop from tram stop or multiple stop platform
// in case of multiple stop id that share a name, will return the first id
// (lowest value)
uint32_t
get_stop_id(const char* stopName, const size_t stopNameLength)
{ 
    size_t idx = 0;

    if (stopName == nullptr) return 0;

    // algo in case of stopName is assured to be in the table.
    while (idx != gtfs::k_stop_count)
    {
        const char* tmp = gtfs::k_stop_names + gtfs::k_stops[idx].name_offset;
    
        if(
            memcmp(stopName, tmp, stopNameLength) == 0
            && tmp[stopNameLength] == 0)
        {
            return gtfs::k_stops[idx].id;
        }
        ++idx;
    }   
    return 0; 
}

uint32_t
get_stop_id(const char* stopName)
{
    return get_stop_id(stopName, std::strlen(stopName));
}
