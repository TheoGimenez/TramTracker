/*
 * File: /src/transitCatalog.cpp
 * Project: ESP32 TramTracker
 * File Created: Wednesday, 30th September 2026 9:23:44, pm
 * Author: Nathan Fain
 * -----
 * Last Modified: Tuesday, October 20th 2026, 12:31:23 am
 * Modified By: Theo Gimenez
 * -----
 */

#include "transitCatalog.h"

// Resolve the user-selected route and stop name into the two TAM direction-specific stop IDs.
std::optional<stop_selection_s>
transit::resolve_selected_stop(const char* routeShortName, const char* stopName)
{
    uint8_t          route_index   = 1; // begin index to 1, 0 is reserved to "unknow" route
    stop_selection_s resolved_stop = {0};

    while (route_index < gtfs::k_route_count)
    {
        const char* route_name = gtfs::k_routes[route_index].name();
        if (same_string(route_name, routeShortName))
        {
            resolved_stop.route = route_name;
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

        const gtfs::Stop& stop = gtfs::k_stops[link.stop_index];
        if (same_string(stop.name(), stopName))
        {
            resolved_stop.name = stop.name();
            resolved_stop.ids[link.direction_id] = stop.id;

            if (resolved_stop.ids[0] != 0 && resolved_stop.ids[1] != 0)
            {
                return std::optional<stop_selection_s>{resolved_stop};
            }
        }

    }
    
    // some stop deserve only one direction
    if (resolved_stop.ids[0] + resolved_stop.ids[1] != 0)
    {
        return std::optional<stop_selection_s>{resolved_stop};
    }

    return std::nullopt;
}

std::vector<String>
transit::get_routes_list(void)
{
    std::vector<String> routes;

    routes.reserve(gtfs::k_route_count);
    // beggin index to 1, 0 is reserved for "unknown"
    for (size_t i = 1; i < gtfs::k_route_count; ++i)
    {
        routes.push_back(String(gtfs::k_routes[i].name()));
    }

    return routes;
}

uint8_t
transit::get_route_idx(const String& route)
{
    size_t idx = gtfs::k_route_count;

    do
    {
        --idx;
        if (route.equals(gtfs::k_routes[idx].name()))
        {
            break;
        }
    } while (idx > 0); // idx 0 reserved for unknow

    return idx;
}

std::set<String>
transit::get_stops_list(const uint8_t route_idx)
{
    std::set<String> stops;


    for (size_t i = 0; i < gtfs::k_stop_route_count; ++i)
    {
        const auto& link = gtfs::k_stop_routes[i];
    
        if (link.route_index != route_idx)
        {
            continue;
        }
        
        stops.insert(gtfs::k_stops[link.stop_index].name());
    }
    return stops;
}

void
transit::forEachRouteName(NameVisitor visitor, void *context)
{
    // iterate on each route name exept index 0 reserved for "unknow"
    for (size_t i = gtfs::k_route_count; i > 0; --i)
    {
        if (visitor(gtfs::k_routes[i].name(), context))
        {
            return;
        }
    }
}

void
transit::forEachOrderedRouteName(NameVisitor visitor, void *context)
{
    // iterate on each route name exept index 0 reserved for "unknow"
    for (size_t i = 1; i < gtfs::k_route_count; ++i)
    {
        if (visitor(gtfs::k_routes[i].name(), context))
        {
            return;
        }
    }
}