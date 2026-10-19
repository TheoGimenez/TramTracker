/*
 * File: /src/utils.h
 * Project: ESP32 TramTracker
 * Created Date: 19-10-2026, 6:03:00 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Tuesday, October 20th 2026, 1:54:53 am
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include <charconv>
#include <ctime> // struct tm
#include <cstring>
#include <stdlib.h>
//
#include "esp_sntp.h" // use of time_t
#include <WString.h>//! STRING

// Syntax sugar Magic macro that to send method in off-class function. shorter to read than std::bind_front
#define WRAP_METHOD(func) [this](auto&&... args) -> decltype(auto) { \
    return this->func(std::forward<decltype(args)>(args)...);    \
}

bool   same_string(const char* s1, const char* s2);
time_t get_time   (void);
void   configure_tz_time(const char* tz, const char* server1, const char* server2 = nullptr, const char* server3 = nullptr);

template<typename T>
inline void append_num(String& str, T val)
{
    char s[16];
    if (auto [ptr, ec] = std::to_chars(s, s + sizeof(16), val); ec == std::errc())
    {
        str.concat(s, ptr - s);
    }
}


#endif //SRC_UTILS_H_