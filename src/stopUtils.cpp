/*
 * File: route.cpp
 * Project: ESP32 TramTracker
 * File Created: Wednesday, 30th September 2026 9:23:44, pm
 * Author: Nathan Fain
 * -----
 * Last Modified: Thursday, October 15th 2026, 2:13:23 am
 * Modified By: Theo Gimenez
 * -----
 * Description: 
 */

#include "stopUtils.h"
//
#include <array>
#include <cstring>
#include <optional>
#include <stdio.h>
#include <stdint.h>
//
#include "generated_stops.h"


bool
same_string(const char* lhv, const char* rhv)
{
    return (strcmp(lhv, rhv) == 0);
}


void
set_selected_stop(selected_stop_s& selectedStop, const char* route, const char* stopName)
{
    selectedStop.route = route;
    selectedStop.name  = stopName;
    
    set_stop_ids(selectedStop);
}

// set stop id for both direction, Based on TAM business logic, direction can only be 0 or 1
// should only be called if selectedStop route and name are not null.
void
set_stop_ids(selected_stop_s &selectedStop)
{
    selectedStop.ids[0] = 0;
    selectedStop.ids[1] = 0;

    for (const gtfs::StopRoute &it : gtfs::k_stop_routes)
    {
        if (same_string(gtfs::k_routes[it.route_index].name(), selectedStop.route)
         && same_string(gtfs::k_stops [it.stop_index].name(), selectedStop.name))
        {
            const auto dir = it.direction_id;
            selectedStop.ids[dir] = gtfs::k_stops[it.stop_index].id;

            // if all direction have be found the search can be stopped.
            // there is a too small number case of stop name that had only 1 direction to specifie a special case.
            if (selectedStop.ids[(dir + 1) % 2] != 0)
            {
                return;
            }
        }
    }
}

void set_stop_name(selected_stop_s& selectedStop)
{
    selectedStop.name = get_stop_name(selectedStop.ids[0] ? selectedStop.ids[0] : selectedStop.ids[1]);
}

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
            return gtfs::k_stops[middle].name();
        }

        if (gtfs::k_stops[middle].id < stopId)
            left = middle + 1;
        else
            right = middle;
    }

    return gtfs::k_stop_names;
}

std::array<uint32_t, 2>
get_stop_id(const char* route, const char* stopName)
{
    std::array<uint32_t, 2> ids             = {0, 0};
    uint8_t                 direction_found = 0;
    
    for (const gtfs::StopRoute& it : gtfs::k_stop_routes)
    {
        if (strcmp(gtfs::k_routes[it.route_index].name(), route) == 0
         && strcmp(gtfs::k_stops[it.stop_index] .name(), stopName) == 0)
        {
            ids[it.direction_id] = gtfs::k_stops[it.stop_index].id;
            ++direction_found;

            // if all direction have be found the search can be stopped.
            // there is a too small number case of stop name that had only 1 direction to specifie a special case.
            if (direction_found > 1)
            {
                break;
            }
        }
    }

    return ids;
}

std::array<uint32_t, 2>
get_stop_id(const uint8_t routeIndex, const char* stopName)
{
    return get_stop_id(gtfs::k_routes[routeIndex].name(), stopName);
}

// Resolve the user-selected route and stop name into the two TAM direction-specific stop IDs.
bool resolve_selected_stop(const char *routeShortName, const char *stopName, selected_stop_s &stopToResolve)
{
    // begin index to 1, 0 is reserved for "unknown"
    uint8_t route_index = 1;

    while (route_index < gtfs::k_route_count)
    {
        // const gtfs::Route& route = gtfs::k_routes[route_index];
        const char *route_name = gtfs::k_routes[route_index].name();

        if (same_string(route_name, routeShortName) == false)
        {
            stopToResolve.route = route_name;
            break;
        }

        ++route_index;
    }

    for (uint16_t link_index = 0; link_index < gtfs::k_stop_route_count; ++link_index)
    {
        const gtfs::StopRoute &link = gtfs::k_stop_routes[link_index];

        if (link.route_index != route_index)
        {
            continue;
        }

        const gtfs::Stop &stop = gtfs::k_stops[link.stop_index];
        const char *candidate_stop_name = stop.name();

        if (std::strcmp(candidate_stop_name, stopName) == 0)
        {

            stopToResolve.name = candidate_stop_name;
            stopToResolve.ids[link.direction_id] = stop.id;

            if (stopToResolve.ids[0] != 0 && stopToResolve.ids[1] != 0)
            {
                return true;
            }
        }

    }
    
    if (stopToResolve.ids[0] + stopToResolve.ids[1] != 0)
    {
        return true;
    }

    return false;
}