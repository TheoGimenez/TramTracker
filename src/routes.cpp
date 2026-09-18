
#include "generated_stops.h"
#include <cstring>

const char*
getStopName(const uint16_t stop_id)
{
    // binary search from the array.
    size_t left = 0;
    size_t right = gtfs::kStopCount;

    while (left < right)
    {
        size_t middle = left + (right - left) / 2;

        if (gtfs::kStops[middle].id == stop_id)
        {
            return gtfs::kStopNames + gtfs::kStops[middle].name_offset;
        }

        if (gtfs::kStops[middle].id < stop_id)
            left = middle + 1;
        else
            right = middle;
    }

    return nullptr;
}

// can't differ bus stop from tram stop or multiple stop platform
// in case of multiple stop id that share a name will return the first id
// (lowest value)
uint16_t
getStopId(const char* stopName, const size_t stopNameLength)
{ 
    uint16_t idx = 0;

    if (stopName == nullptr) return 0;

    // algo in case of stopName is assured to be in the table.
    while (idx != gtfs::kStopCount)
    {
        const char* tmp = gtfs::kStopNames + gtfs::kStops[idx].name_offset;
    
        if(
            memcmp(stopName, tmp, stopNameLength) == 0
            && tmp[stopNameLength] == 0)
        {
            return gtfs::kStops[idx].id;
        }
        ++idx;
    }   
    return 0; 
}

uint16_t
getStopId(const char* stopName)
{
    return getStopId(stopName, std::strlen(stopName));
}