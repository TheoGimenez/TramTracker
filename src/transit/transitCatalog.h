/*
 * File: /src/transit/transitCatalog.h
 * Project: ESP32 TramTracker
 * Created Date: 18-09-2026, 10:20:38 am
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 27th 2026, 10:18:14 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_TRANSIT_TRANSIT_CATALOG_H_
#define SRC_TRANSIT_TRANSIT_CATALOG_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <string.h>
//
#include "generated_stops.h"
#include "../app/transitType.h"
#include "utils.h"

namespace transit
{
    using NameVisitor = bool (*)(const char *name, void *context);

    std::vector<std::string>        get_routes_list        (void);
    uint8_t                         get_route_idx          (const char* route);
    uint8_t                         get_route_idx          (const std::string &route);
    std::set<std::string>           get_stops_list         (const uint8_t route_idx);
    std::optional<stop_selection_s> resolve_selected_stop  (const char *routeShortName, const char *stopName);
    //
    void                            forEachRouteName       (NameVisitor visitor, void *context);
    void                            forEachOrderedRouteName(NameVisitor visitor, void *context);
} // namespace transitCatalog
#endif // SRC_TRANSIT_TRANSIT_CATALOG_H_
