
#ifndef ROUTE_H_
#define ROUTE_H_

#include <unordered_map>
#include <set>
#include <utility>

const char* getStopName (const uint32_t stop_id);
uint32_t    getStopId   (const char*    stopName, const size_t stopNameLength);
uint32_t    getStopId   (const char*    stopName);

#endif // ROUTE_H_
